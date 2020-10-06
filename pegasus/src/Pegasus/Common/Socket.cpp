//%LICENSE////////////////////////////////////////////////////////////////
//
// Licensed to The Open Group (TOG) under one or more contributor license
// agreements.  Refer to the OpenPegasusNOTICE.txt file distributed with
// this work for additional information regarding copyright ownership.
// Each contributor licenses this file to you under the OpenPegasus Open
// Source License; you may not use this file except in compliance with the
// License.
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
// IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
// CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
// TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
// SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//
//////////////////////////////////////////////////////////////////////////
//
//%/////////////////////////////////////////////////////////////////////////////

#include "Socket.h"
#include "Network.h"
#include <cctype>
#include <cstring>
#include <Pegasus/Common/Sharable.h>
#include <Pegasus/Common/Logger.h>
#include <Pegasus/Common/System.h>
#include <Pegasus/Common/Tracer.h>
#include <Pegasus/Common/Threads.h>
#include <Pegasus/Common/Mutex.h>

PEGASUS_NAMESPACE_BEGIN

#ifdef PEGASUS_OS_TYPE_WINDOWS
static Uint32 _socketInterfaceRefCount = 0;
static Mutex _socketInterfaceRefCountLock;
#endif

Boolean Socket::timedConnect(
    SocketHandle socket,
    sockaddr* address,
    int addressLength,
    Uint32 timeoutMilliseconds)
{
    int connectResult;
#ifdef PEGASUS_OS_TYPE_WINDOWS
    connectResult = ::connect(socket, address, addressLength);
#else
    Uint32 maxConnectAttempts = 100;
    // Retry the connect() until it succeeds or it fails with an error other
    // than EINTR, EAGAIN (for Linux), or ECONNREFUSED (for HP-UX and z/OS).
    while (((connectResult = ::connect(socket, address, addressLength)) == -1)
           && (maxConnectAttempts-- > 0)
           && ((errno == EINTR) || (errno == EAGAIN) ||
               (errno == ECONNREFUSED)))
    {
        Threads::sleep(1);
    }
#endif

    if (connectResult == 0)
    {
        return true;
    }

    if (getSocketError() == PEGASUS_NETWORK_EINPROGRESS)
    {
        PEG_TRACE((TRC_HTTP, Tracer::LEVEL4,
            "Connection to server in progress.  Waiting up to %u milliseconds "
                "for the socket to become connected.",
            timeoutMilliseconds));

        fd_set fdwrite;
        FD_ZERO(&fdwrite);
        FD_SET(socket, &fdwrite);
        struct timeval timeoutValue =
            { timeoutMilliseconds/1000, timeoutMilliseconds%1000*1000 };
        int selectResult = -1;

#ifdef PEGASUS_OS_TYPE_WINDOWS
        PEGASUS_RETRY_SYSTEM_CALL(
            select(FD_SETSIZE, NULL, &fdwrite, &fdwrite, &timeoutValue),
            selectResult);
#else
        PEGASUS_RETRY_SYSTEM_CALL(
            select(FD_SETSIZE, NULL, &fdwrite, NULL, &timeoutValue),
            selectResult);
#endif
        if (selectResult == 0)
        {
            PEG_TRACE_CSTRING(TRC_HTTP, Tracer::LEVEL1,
                "select() timed out waiting for the socket connection to be "
                    "established.");
            return false;
        }
        else if (selectResult > 0)
        {
            int optval;
            SocketLength optlen = sizeof(int);
            getsockopt(socket, SOL_SOCKET, SO_ERROR, (char*)&optval, &optlen);
            if (optval == 0)
            {
                PEG_TRACE_CSTRING(TRC_HTTP, Tracer::LEVEL4,
                    "Connection with server established.");
                return true;
            }
            else
            {
                PEG_TRACE((TRC_HTTP, Tracer::LEVEL1,
                    "Did not connect, getsockopt() returned optval = %d",
                    optval));
                return false;
            }
        }
        else
        {
            PEG_TRACE((TRC_HTTP, Tracer::LEVEL1,
                "select() returned error code %d",
                getSocketError()));
            return false;
        }
    }

    PEG_TRACE((TRC_HTTP, Tracer::LEVEL1,
        "connect() returned error code %d",
        getSocketError()));
    return false;
}

Sint32 Socket::read(SocketHandle socket, void* ptr, Uint32 size)
{
#ifdef PEGASUS_OS_TYPE_WINDOWS
    return ::recv(socket, (char*)ptr, size, 0);
#else
    int status;
    PEGASUS_RETRY_SYSTEM_CALL(::read(socket, (char*)ptr, size), status);
    return status;
#endif
}

Sint32 Socket::peek(SocketHandle socket, void* ptr, Uint32 size)
{
    #ifdef PEGASUS_OS_TYPE_WINDOWS
    return ::recv(socket, (char*)ptr, size, MSG_PEEK);
    #else
    int status;
    PEGASUS_RETRY_SYSTEM_CALL(::recv(socket, (char*)ptr, size, MSG_PEEK),
            status);
    return status;
    #endif
}

Sint32 Socket::write(SocketHandle socket, const void* ptr, Uint32 size)
{
#ifdef PEGASUS_OS_TYPE_WINDOWS
    return ::send(socket, (const char*)ptr, size, 0);
#else
    int status;
    PEGASUS_RETRY_SYSTEM_CALL(::write(socket, (char*)ptr, size), status);
    return status;
#endif
}

Sint32 Socket::timedWrite(
    SocketHandle socket,
    const void* ptr,
    Uint32 size,
    Uint32 socketWriteTimeout)
{
    Sint32 bytesWritten = 0;
    Sint32 totalBytesWritten = 0;
    Boolean socketTimedOut = false;
    int selreturn = 0;
    while (1)
    {
#ifdef PEGASUS_OS_TYPE_WINDOWS
        PEGASUS_RETRY_SYSTEM_CALL(
            ::send(socket, (const char*)ptr, size, 0), bytesWritten);
#else
        PEGASUS_RETRY_SYSTEM_CALL(
            ::write(socket, (char*)ptr, size), bytesWritten);
#endif
        // Some data written this cycle ?
        // Add it to the total amount of written data.
        if (bytesWritten > 0)
        {
            totalBytesWritten += bytesWritten;
            socketTimedOut = false;
        }

        // All data written ? return amount of data written
        if ((Uint32)bytesWritten == size)
        {
            return totalBytesWritten;
        }
        // If data has been written partially, we resume writing data
        // this also accounts for the case of a signal interrupt
        // (i.e. errno = EINTR)
        if (bytesWritten > 0)
        {
            size -= bytesWritten;
            ptr = (void *)((char *)ptr + bytesWritten);
            continue;
        }
        // Something went wrong
        if (bytesWritten == PEGASUS_SOCKET_ERROR)
        {
            // if we already waited for the socket to get ready, bail out
            if (socketTimedOut) return bytesWritten;
#ifdef PEGASUS_OS_TYPE_WINDOWS
            if (WSAGetLastError() == WSAEWOULDBLOCK)
#else
            if (errno == EAGAIN || errno == EWOULDBLOCK)
#endif
            {
                fd_set fdwrite;
                 // max. timeout seconds waiting for the socket to get ready
                struct timeval tv = { socketWriteTimeout, 0 };
                FD_ZERO(&fdwrite);
                FD_SET(socket, &fdwrite);
                selreturn = select(FD_SETSIZE, NULL, &fdwrite, NULL, &tv);
                if (selreturn == 0) socketTimedOut = true; // ran out of time
                continue;
            }
            return bytesWritten;
        }
    }
}

void Socket::close(SocketHandle& socket)
{
    if (socket != PEGASUS_INVALID_SOCKET)
    {
#ifdef PEGASUS_OS_TYPE_WINDOWS
        if (!closesocket(socket))
        {
            socket = PEGASUS_INVALID_SOCKET;
        }
#else
        int status;
        PEGASUS_RETRY_SYSTEM_CALL(::close(socket), status);

        if (status == 0)
        {
            socket = PEGASUS_INVALID_SOCKET;
        }
#endif
    }
}

void Socket::disableBlocking(SocketHandle socket)
{
#ifdef PEGASUS_OS_TYPE_WINDOWS
    unsigned long flag = 1;    // Use "flag = 0" to enable blocking
    ioctlsocket(socket, FIONBIO, &flag);
#elif PEGASUS_OS_VMS
    int flag=1;                // Use "flag = 0" to enable blocking
    ioctl(socket, FIONBIO, &flag);
#else
    int flags = fcntl(socket, F_GETFL, 0);
    flags |= O_NONBLOCK;    // Use "flags &= ~O_NONBLOCK" to enable blocking
    fcntl(socket, F_SETFL, flags);
#endif
}

void Socket::initializeInterface()
{
#ifdef PEGASUS_OS_TYPE_WINDOWS
    AutoMutex mtx(_socketInterfaceRefCountLock);
    if (_socketInterfaceRefCount == 0)
    {
        WSADATA tmp;

        int err = WSAStartup(0x202, &tmp);
        if (err != 0)
        {
            throw Exception(MessageLoaderParms(
                "Common.Socket.WSASTARTUP_FAILED.WINDOWS",
                "WSAStartup failed with error $0.",
                err));
        }
    }

    _socketInterfaceRefCount++;
#endif
}

void Socket::uninitializeInterface()
{
#ifdef PEGASUS_OS_TYPE_WINDOWS
    AutoMutex mtx(_socketInterfaceRefCountLock);
    _socketInterfaceRefCount--;

    if (_socketInterfaceRefCount == 0)
        WSACleanup();
#endif
}

//------------------------------------------------------------------------------
//
// _setTCPNoDelay()
//
//------------------------------------------------------------------------------

inline void _setTCPNoDelay(SocketHandle socket)
{
    // This function disables "Nagle's Algorithm" also known as "the TCP delay
    // algorithm", which causes read operations to obtain whatever data is
    // already in the input queue and then wait a little longer to see if
    // more data arrives. This algorithm optimizes the case in which data is
    // sent in only one direction but severely impairs performance of round
    // trip servers. Disabling TCP delay is a standard technique for round
    // trip servers.

   int opt = 1;
   setsockopt(socket, IPPROTO_TCP, TCP_NODELAY, (char*)&opt, sizeof(opt));
}

#ifdef PEGASUS_OS_ZOS
inline void _setInformIfNewTCPIP(SocketHandle socket)
{
   // This function enables the notification of the CIM Server that a new
   // TCPIP transport layer is active. This is needed to be aware of a
   // restart of the transport layer. When this option is in effect,
   // the accetp(), select(), and read() request will receive an errno=EIO.
   // Once this happens, the socket should be closed and create a new.

   int NewTcpipOn = 1;
   setibmsockopt(
       socket,
       SOL_SOCKET,
       SO_EioIfNewTP,
       (char*)&NewTcpipOn,
       sizeof(NewTcpipOn));
}
#else
inline void _setInformIfNewTCPIP(SocketHandle)
{
}
#endif


SocketHandle Socket::createSocket(int domain, int type, int protocol)
{
    SocketHandle newSocket;

    if (domain == AF_UNIX)
    {
        return socket(domain,type,protocol);
    }

#ifdef PEGASUS_OS_ZOS
    bool sendTcpipMsg = true;
#endif

    while (1)
    {
        newSocket = socket(domain,type,protocol);

        if ((newSocket != PEGASUS_INVALID_SOCKET) ||
            (getSocketError() != PEGASUS_NETWORK_TRYAGAIN))
        {
            break;
        }

#ifdef PEGASUS_OS_ZOS
        // The program should wait for transport layer to become ready.

        if (sendTcpipMsg)
        {
            Logger::put_l(
                Logger::STANDARD_LOG, System::CIMSERVER, Logger::INFORMATION,
                MessageLoaderParms(
                    "Common.Socket.WAIT_FOR_TCPIP",
                    "TCP/IP temporary unavailable."));
            sendTcpipMsg = false;
        }

        System::sleep(30);
#endif
    } // wait for the transport layer become ready.

    // Is the socket in an unrecoverable error ?
    if (newSocket == PEGASUS_INVALID_SOCKET)
    {
        // return immediate
        return PEGASUS_INVALID_SOCKET;
    }
    else
    {
        // set aditional socket options
        _setTCPNoDelay(newSocket);
        _setInformIfNewTCPIP(newSocket);

        return newSocket;
    }
}

PEGASUS_NAMESPACE_END
