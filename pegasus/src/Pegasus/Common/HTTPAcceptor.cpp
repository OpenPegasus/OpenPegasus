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

#include "Config.h"
#include "Constants.h"
#include <iostream>

#include "Network.h"
#include "Socket.h"
#include "TLS.h"
#include "HTTPAcceptor.h"
#include "HTTPConnection.h"
#include "HostAddress.h"
#include "Tracer.h"
#include <Pegasus/Common/MessageLoader.h>

#ifdef PEGASUS_OS_PASE
# include <as400_protos.h>
# include <Pegasus/Common/PaseCcsid.h>
#endif

PEGASUS_USING_STD;

PEGASUS_NAMESPACE_BEGIN


static int _maxConnectionQueueLength = -1;

Uint32 HTTPAcceptor::_socketWriteTimeout =
    PEGASUS_DEFAULT_SOCKETWRITE_TIMEOUT_SECONDS;

#ifndef PEGASUS_INTEGERS_BOUNDARY_ALIGNED
Mutex HTTPAcceptor::_socketWriteTimeoutMutex;
#endif

////////////////////////////////////////////////////////////////////////////////
//
// HTTPAcceptorRep
//
////////////////////////////////////////////////////////////////////////////////

class HTTPAcceptorRep
{
public:
    HTTPAcceptorRep(Uint16 connectionType)
    {
        if (connectionType == HTTPAcceptor::LOCAL_CONNECTION)
        {
#ifndef PEGASUS_DISABLE_LOCAL_DOMAIN_SOCKET
            address =
                reinterpret_cast<struct sockaddr*>(new struct sockaddr_un);
            address_size = sizeof(struct sockaddr_un);
#else
            PEGASUS_UNREACHABLE(PEGASUS_ASSERT(false);)
#endif
        }
#ifdef PEGASUS_ENABLE_IPV6
        else if (connectionType == HTTPAcceptor::IPV6_CONNECTION)
        {
            address =
                reinterpret_cast<struct sockaddr*>(new struct sockaddr_in6);
            address_size = sizeof(struct sockaddr_in6);
        }
#endif
        else if (connectionType == HTTPAcceptor::IPV4_CONNECTION)
        {
            address =
                reinterpret_cast<struct sockaddr*>(new struct sockaddr_in);
            address_size = sizeof(struct sockaddr_in);
        }
        else
        {
            PEGASUS_UNREACHABLE(PEGASUS_ASSERT(false);)
    }
    }

    ~HTTPAcceptorRep()
    {
        closeSocket();
        delete address;
    }

    void closeSocket()
    {
        Socket::close(socket);
    }

    struct sockaddr* address;

    SocketLength address_size;
    Mutex _connection_mut;

    SocketHandle socket;
    Array<HTTPConnection*> connections;
};


////////////////////////////////////////////////////////////////////////////////
//
// HTTPAcceptor
//
////////////////////////////////////////////////////////////////////////////////

HTTPAcceptor::HTTPAcceptor(Monitor* monitor,
                           MessageQueue* outputMessageQueue,
                           Uint16 connectionType,
                           Uint32 portNumber,
                           SSLContext * sslcontext,
                           ReadWriteSem* sslContextObjectLock,
                           HostAddress *listenOn)
   : Base(PEGASUS_QUEUENAME_HTTPACCEPTOR),  // ATTN: Need unique names?
     _monitor(monitor),
     _outputMessageQueue(outputMessageQueue),
     _rep(0),
     _entry_index(-1),
     _connectionType(connectionType),
     _portNumber(portNumber),
     _sslcontext(sslcontext),
    _sslContextObjectLock(sslContextObjectLock),
    _listenAddress(listenOn)
{
   PEGASUS_ASSERT(!_sslcontext == !_sslContextObjectLock);
   Socket::initializeInterface();

   /*
        Platforms interpret the value of _maxConnectionQueueLength
        differently.  Some platforms interpret the value literally, while
        others multiply a fudge factor. When the server is under stress from
        multiple clients with multiple requests, toggling this number may
        prevent clients from being dropped.  Instead of hard coding the
        value, we allow an environment variable to be set which specifies a
        number greater than the maximum concurrent client connections
        possible.  If this environment var is not specified, then
        _maxConnectionQueueLength = 15.
   */

//To engage runtime backlog queue length: uncomment the following block AND
//comment out the line _maxConnectionQueueLength = 15

/*
    if (_maxConnectionQueueLength == -1)
    {
        const char* env = getenv("PEGASUS_MAX_BACKLOG_CONNECTION_QUEUE");
        if (!env)
        {
            _maxConnectionQueueLength = 15;
        }
        else
        {
            char* end = NULL;
            _maxConnectionQueueLength = strtol(env, &end, 10);
            if (*end)
                _maxConnectionQueueLength = 15;
            cout << " _maxConnectionQueueLength = " <<
                _maxConnectionQueueLength << endl;
        }
    }
*/
#ifdef PEGASUS_WMIMAPPER
    //The WMI Mapper can be used as a proxy to multiple WMI Servers.
    //If a client application simultaneously initiates connections
    //to many Windows systems, many of these connections may be routed
    //to a single WMI Mapper. A larger _maxConnectionQueueLength
    //value is required to allow these connections to be initiated
    //successfully.
    _maxConnectionQueueLength = 40;
#else
    _maxConnectionQueueLength = 15;
#endif
}

HTTPAcceptor::~HTTPAcceptor()
{
    destroyConnections();
    unbind();
    // ATTN: Is this correct in a multi-HTTPAcceptor server?
    Socket::uninitializeInterface();
}

void HTTPAcceptor::handleEnqueue(Message *message)
{
    if (!message)
       return;

    PEGASUS_ASSERT(_rep != 0);
    switch (message->getType())
    {
        case SOCKET_MESSAGE:
        {
            // If this is a connection request:
            PEGASUS_ASSERT(((SocketMessage*)message)->socket == _rep->socket);

            PEGASUS_ASSERT(
                ((SocketMessage*)message)->events & SocketMessage::READ);

            _acceptConnection();

            break;
        }

       case CLOSE_CONNECTION_MESSAGE:
       {
           CloseConnectionMessage* closeConnectionMessage =
               (CloseConnectionMessage*)message;

           AutoMutex autoMut(_rep->_connection_mut);

           for (Uint32 i = 0, n = _rep->connections.size(); i < n; i++)
           {
               HTTPConnection* connection = _rep->connections[i];
               SocketHandle socket = connection->getSocket();

               if (socket == closeConnectionMessage->socket)
               {
                   _monitor->unsolicitSocketMessages(socket);
                   _rep->connections.remove(i);
                   delete connection;
                   break;
               }
           }

           break;
       }

       default:
           PEGASUS_UNREACHABLE(PEGASUS_ASSERT(false);)
           break;
    }

    delete message;
}


void HTTPAcceptor::handleEnqueue()
{
    Message* message = dequeue();
    handleEnqueue(message);
}

void HTTPAcceptor::bind()
{
    if (_rep)
    {
        MessageLoaderParms parms("Common.HTTPAcceptor.ALREADY_BOUND",
            "HTTPAcceptor already bound");
        throw BindFailedException(parms);
    }

    _rep = new HTTPAcceptorRep(_connectionType);

    // bind address
    _bind();
}


/**
    _bind - creates a new server socket and bind socket to the port address.
    If PEGASUS_DISABLE_LOCAL_DOMAIN_SOCKET is not defined, the port number is
    ignored and a domain socket is bound.
*/
void HTTPAcceptor::_bind()
{
#ifdef PEGASUS_OS_PASE
    AutoPtr<PaseCcsid> ccsid;
#endif

    PEGASUS_ASSERT(_rep != 0);
    // Create address:
    memset(_rep->address, 0, _rep->address_size);

    if (_connectionType == LOCAL_CONNECTION)
    {
#ifndef PEGASUS_DISABLE_LOCAL_DOMAIN_SOCKET
        //
        // Make sure the local domain socket can be owned by the cimserver
        // user.  Otherwise, the bind may fail with a vague "bind failed"
        // error.
        //
#ifdef PEGASUS_OS_PASE
        // PASE domain socket needs ccsid 819
        int orig_ccsid;
        orig_ccsid = _SETCCSID(-1);
        if (orig_ccsid == -1)
        {
            PEG_TRACE_CSTRING(TRC_DISCARDED_DATA, Tracer::LEVEL1,
                    "HTTPAcceptor::_bind: Can not get current PASE CCSID.");
            orig_ccsid = 1208;
        }
        ccsid.reset(new PaseCcsid(819, orig_ccsid));
#endif
        if (System::exists(PEGASUS_LOCAL_DOMAIN_SOCKET_PATH))
        {
            if (!System::removeFile(PEGASUS_LOCAL_DOMAIN_SOCKET_PATH))
            {
                throw CannotRemoveFile(PEGASUS_LOCAL_DOMAIN_SOCKET_PATH);
            }
        }

        reinterpret_cast<struct sockaddr_un*>(_rep->address)->sun_family =
            AF_UNIX;
        strcpy(reinterpret_cast<struct sockaddr_un*>(_rep->address)->sun_path,
            PEGASUS_LOCAL_DOMAIN_SOCKET_PATH);
#else
        PEGASUS_UNREACHABLE(PEGASUS_ASSERT(false);)
#endif
    }
#ifdef PEGASUS_ENABLE_IPV6
    else if (_connectionType == IPV6_CONNECTION)
    {
        if(_listenAddress)
        {
            String hostAdd = _listenAddress->getHost();
            CString ip = hostAdd.getCString();

            struct sockaddr_in6 in6addr;
            memset(&in6addr, 0, sizeof(sockaddr_in6));
            if(_listenAddress ->isHostAddLinkLocal())
            {
                HostAddress::convertTextToBinary(AF_INET6, 
                (const char*)ip,
                &in6addr.sin6_addr);
                reinterpret_cast<struct sockaddr_in6*>(
                    _rep->address)->sin6_addr = in6addr.sin6_addr;
                reinterpret_cast<struct sockaddr_in6*>(
                    _rep->address)->sin6_scope_id = 
                        _listenAddress->getScopeID();
            }
            else
            {
                HostAddress::convertTextToBinary(AF_INET6, 
                (const char*)ip,
                &in6addr.sin6_addr);
                reinterpret_cast<struct sockaddr_in6*>(
                    _rep->address)->sin6_addr = in6addr.sin6_addr;
            }
        }
        else
        {
        reinterpret_cast<struct sockaddr_in6*>(_rep->address)->sin6_addr =
            in6addr_any;
        }
        reinterpret_cast<struct sockaddr_in6*>(_rep->address)->sin6_family =
            AF_INET6;
        reinterpret_cast<struct sockaddr_in6*>(_rep->address)->sin6_port =
            htons(_portNumber);
    }
#endif
    else if(_connectionType == IPV4_CONNECTION)
    {
        if(_listenAddress)
        {
            String hostAdd = _listenAddress->getHost();
            CString ip = hostAdd.getCString();
            struct sockaddr_in addrs;
            HostAddress::convertTextToBinary(
                AF_INET, 
                (const char*)ip,
                &addrs.sin_addr);
            reinterpret_cast<struct sockaddr_in*>(
                _rep->address)->sin_addr.s_addr = addrs.sin_addr.s_addr;
        }
        else
        {
            reinterpret_cast<struct sockaddr_in*>(
            _rep->address)->sin_addr.s_addr = INADDR_ANY;
        }
        reinterpret_cast<struct sockaddr_in*>(_rep->address)->sin_family =
            AF_INET;
        reinterpret_cast<struct sockaddr_in*>(_rep->address)->sin_port =
            htons(_portNumber);
    }
    else
    {
        PEGASUS_UNREACHABLE(PEGASUS_ASSERT(false);)
    }

    // Create socket:

    if (_connectionType == LOCAL_CONNECTION)
    {
        _rep->socket = Socket::createSocket(AF_UNIX, SOCK_STREAM, 0);
    }
#ifdef PEGASUS_ENABLE_IPV6
    else if (_connectionType == IPV6_CONNECTION)
    {
        _rep->socket = Socket::createSocket(PF_INET6, SOCK_STREAM, IPPROTO_TCP);
    }
#endif
    else if (_connectionType == IPV4_CONNECTION)
    {
        _rep->socket = Socket::createSocket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    }
    else
    {
        PEGASUS_UNREACHABLE(PEGASUS_ASSERT(false);)
    }

    if (_rep->socket < 0)
    {
        delete _rep;
        _rep = 0;
        MessageLoaderParms parms("Common.HTTPAcceptor.FAILED_CREATE_SOCKET",
            "Failed to create socket");
        throw BindFailedException(parms);
    }

    Socket::disableBlocking(_rep->socket);

// set the close-on-exec bit for this file handle.
// any unix that forks needs this bit set.
#if !defined PEGASUS_OS_TYPE_WINDOWS && !defined(PEGASUS_OS_VMS)
    int sock_flags;
    if ((sock_flags = fcntl(_rep->socket, F_GETFD, 0)) < 0)
    {
        PEG_TRACE_CSTRING(TRC_DISCARDED_DATA, Tracer::LEVEL1,
            "HTTPAcceptor::_bind: fcntl(F_GETFD) failed");
    }
    else
    {
        sock_flags |= FD_CLOEXEC;
        if (fcntl(_rep->socket, F_SETFD, sock_flags) < 0)
        {
            PEG_TRACE_CSTRING(TRC_DISCARDED_DATA, Tracer::LEVEL1,
                "HTTPAcceptor::_bind: fcntl(F_SETFD) failed");
        }
    }
#endif


    //
    // Set the socket option SO_REUSEADDR to reuse the socket address so
    // that we can rebind to a new socket using the same address when we
    // need to resume the cimom as a result of a timeout during a Shutdown
    // operation.
    //
    int opt=1;
    if (setsockopt(_rep->socket, SOL_SOCKET, SO_REUSEADDR,
            (char *)&opt, sizeof(opt)) < 0)
    {
        delete _rep;
        _rep = 0;
        MessageLoaderParms parms("Common.HTTPAcceptor.FAILED_SET_SOCKET_OPTION",
            "Failed to set socket option");
        throw BindFailedException(parms);
    }


    //
    // Bind socket to port:
    //
    if (::bind(_rep->socket, _rep->address, _rep->address_size) < 0)
    {
        MessageLoaderParms parms(
            "Common.HTTPAcceptor.FAILED_BIND_SOCKET_DETAIL",
            "Failed to bind socket on port $0: $1.",
            _portNumber, PEGASUS_SYSTEM_NETWORK_ERRORMSG_NLS);

        delete _rep;
        _rep = 0;
        throw BindFailedException(parms);
    }


    //
    // Get the actual port value used if the caller specified a port value of 0.
    //
    if (_portNumber == 0)
    {
        sockaddr_in buf;
        SocketLength bufSize = sizeof(buf);
        if (getsockname(_rep->socket, reinterpret_cast<sockaddr *>(&buf),
                &bufSize) == 0 )
        {
            _portNumber = ntohs(buf.sin_port);
        }
    }


    //
    // Change permissions on Linux local domain socket to allow writes by
    // others.
    //
#if !defined(PEGASUS_DISABLE_LOCAL_DOMAIN_SOCKET) && \
     (defined(PEGASUS_PLATFORM_LINUX_GENERIC_GNU) || \
      defined(PEGASUS_OS_ZOS) || \
      defined(PEGASUS_OS_PASE))
    if (_connectionType == LOCAL_CONNECTION)
    {
        if (::chmod(PEGASUS_LOCAL_DOMAIN_SOCKET_PATH,
                S_IRUSR | S_IWUSR | S_IXUSR |
                S_IRGRP | S_IWGRP | S_IXGRP |
                S_IROTH | S_IWOTH | S_IXOTH ) < 0 )
        {
            MessageLoaderParms parms(
                "Common.HTTPAcceptor.FAILED_SET_LDS_FILE_OPTION",
                "Failed to set permission on local domain socket $0: $1.",
                PEGASUS_LOCAL_DOMAIN_SOCKET_PATH,
                PEGASUS_SYSTEM_ERRORMSG_NLS );

            delete _rep;
            _rep = 0;
            throw BindFailedException(parms);
        }
    }
#endif

    // Set up listening on the given socket:

    //int const _maxConnectionQueueLength = 15;

    if (::listen(_rep->socket, _maxConnectionQueueLength) < 0)
    {
        MessageLoaderParms parms(
            "Common.HTTPAcceptor.FAILED_LISTEN_SOCKET",
            "Failed to listen on socket $0: $1.",
            (int)_rep->socket,PEGASUS_SYSTEM_NETWORK_ERRORMSG_NLS );

        delete _rep;
        _rep = 0;
        throw BindFailedException(parms);
    }

    // Register to receive SocketMessages on this socket:

    if (-1 == ( _entry_index = _monitor->solicitSocketMessages(
            _rep->socket,
            getQueueId(),
            MonitorEntry::TYPE_ACCEPTOR)))
    {
        delete _rep;
        _rep = 0;
        MessageLoaderParms parms(
            "Common.HTTPAcceptor.FAILED_SOLICIT_SOCKET_MESSAGES",
            "Failed to solicit socket messaeges");
        throw BindFailedException(parms);
    }
}

/**
    closeConnectionSocket - close the server listening socket to disallow
    new client connections.
*/
void HTTPAcceptor::closeConnectionSocket()
{
    if (_rep)
    {
        // unregister the socket

        // ATTN - comment out - see CIMServer::stopClientConnection()
        //_monitor->unsolicitSocketMessages(_rep->socket);

        // close the socket
        _rep->closeSocket();
        // Unlink Local Domain Socket Bug# 3312
        if (_connectionType == LOCAL_CONNECTION)
        {
#ifndef PEGASUS_DISABLE_LOCAL_DOMAIN_SOCKET
            PEG_TRACE_CSTRING(TRC_HTTP, Tracer::LEVEL3,
                "HTTPAcceptor::closeConnectionSocket Unlinking local "
                    "connection.");
            ::unlink(
                    reinterpret_cast<struct sockaddr_un*>
                        (_rep->address)->sun_path);
#else
            PEGASUS_UNREACHABLE(PEGASUS_ASSERT(false);)
#endif
        }
    }
    else
    {
        PEG_TRACE_CSTRING(TRC_HTTP, Tracer::LEVEL2,
            "HTTPAcceptor::closeConnectionSocket failure _rep is null.");
    }
}

/**
   reopenConnectionSocket - creates a new server socket.
*/
void HTTPAcceptor::reopenConnectionSocket()
{
    if (_rep)
    {
        _bind();
    }
    else
    {
        PEG_TRACE_CSTRING(TRC_DISCARDED_DATA, Tracer::LEVEL1,
            "HTTPAcceptor::reopenConnectionSocket failure _rep is null.");
    }
}


/**
   reconnectConnectionSocket - creates a new server socket.
*/
void HTTPAcceptor::reconnectConnectionSocket()
{
    if (_rep)
    {
        // unregister the socket
        _monitor->unsolicitSocketMessages(_rep->socket);
        // close the socket
        _rep->closeSocket();
        // Unlink Local Domain Socket
        if (_connectionType == LOCAL_CONNECTION)
        {
#ifndef PEGASUS_DISABLE_LOCAL_DOMAIN_SOCKET
            PEG_TRACE_CSTRING(TRC_HTTP, Tracer::LEVEL3,
                "HTTPAcceptor::reconnectConnectionSocket Unlinking local "
                    "connection." );
            ::unlink(
                    reinterpret_cast<struct sockaddr_un*>(
                        _rep->address)->sun_path);
#else
            PEGASUS_UNREACHABLE(PEGASUS_ASSERT(false);)
#endif
        }
        // open the socket
        _bind();
    }
    else
    {
        PEG_TRACE_CSTRING(TRC_DISCARDED_DATA, Tracer::LEVEL1,
            "HTTPAcceptor::reconnectConnectionSocket failure _rep is null.");
    }
}

/**
    getOutstandingRequestCount - returns the number of outstanding requests.
*/
Uint32 HTTPAcceptor::getOutstandingRequestCount() const
{
    Uint32 count = 0;
    if (_rep)
    {
        AutoMutex autoMut(_rep->_connection_mut);
        for (Uint32 i = 0, n = _rep->connections.size(); i < n; i++)
        {
            HTTPConnection* connection = _rep->connections[i];
            if (connection->isResponsePending())
            {
                count++;
            }
        }
    }
    return count;
}


/**
    getPortNumber - returns the port number used for the connection
*/
Uint32 HTTPAcceptor::getPortNumber() const
{
    return _portNumber;
}

void HTTPAcceptor::setSocketWriteTimeout(Uint32 socketWriteTimeout)
{
#ifndef PEGASUS_INTEGERS_BOUNDARY_ALIGNED
    AutoMutex lock(_socketWriteTimeoutMutex);
#endif
    _socketWriteTimeout = socketWriteTimeout;
}

void HTTPAcceptor::unbind()
{
    if (_rep)
    {
        _portNumber = 0;
        _rep->closeSocket();

        if (_connectionType == LOCAL_CONNECTION)
        {
#ifndef PEGASUS_DISABLE_LOCAL_DOMAIN_SOCKET
            ::unlink(
                    reinterpret_cast<struct sockaddr_un*>
                    (_rep->address)->sun_path);
#else
            PEGASUS_UNREACHABLE(PEGASUS_ASSERT(false);)
#endif
        }

        delete _rep;
        _rep = 0;
    }
    else
    {
        PEG_TRACE_CSTRING(TRC_HTTP, Tracer::LEVEL1,
            "HTTPAcceptor::unbind failure _rep is null." );
    }
}

void HTTPAcceptor::destroyConnections()
{
    if (_rep)
    {
        // For each connection created by this object:

        AutoMutex autoMut(_rep->_connection_mut);
        for (Uint32 i = 0, n = _rep->connections.size(); i < n; i++)
        {
            HTTPConnection* connection = _rep->connections[i];
            SocketHandle socket = connection->getSocket();

            // Unsolicit SocketMessages:

            _monitor->unsolicitSocketMessages(socket);

            // Destroy the connection (causing it to close):

            while (connection->refcount.get()) { }
            delete connection;
        }

        _rep->connections.clear();
    }
}

void HTTPAcceptor::_acceptConnection()
{
    // This function cannot be called on an invalid socket!

    PEGASUS_ASSERT(_rep != 0);

    // Accept the connection (populate the address):

    struct sockaddr* accept_address;
    SocketLength address_size;

    if (_connectionType == LOCAL_CONNECTION)
    {
#ifndef PEGASUS_DISABLE_LOCAL_DOMAIN_SOCKET
        accept_address =
            reinterpret_cast<struct sockaddr*>(new struct sockaddr_un);
        address_size = sizeof(struct sockaddr_un);
#else
        PEGASUS_UNREACHABLE(PEGASUS_ASSERT(false);)
#endif
    }
    else
    {
#ifdef PEGASUS_ENABLE_IPV6
        accept_address =
           reinterpret_cast<struct sockaddr*>
           (new struct sockaddr_storage);
        address_size = sizeof(struct sockaddr_storage);
#else
        accept_address =
            reinterpret_cast<struct sockaddr*>(new struct sockaddr_in);
        address_size = sizeof(struct sockaddr_in);
#endif
    }

    // It is not necessary to handle EINTR errors from this accept() call.
    // An EINTR error should not occur on a non-blocking socket.  If the
    // listen socket is blocking and EINTR occurs, the new socket connection
    // is not accepted here.

    // EAGAIN errors are also not handled here.  An EAGAIN error should not
    // occur after select() indicates that the listen socket is available for
    // reading.  If the accept() fails with an EAGAIN error code, a new
    // connection is not accepted here.

    SocketHandle socket = accept(_rep->socket, accept_address, &address_size);

    if (socket == PEGASUS_SOCKET_ERROR)
    {
        // the remote connection is invalid, destroy client address.
        delete accept_address;

        // TCPIP is down reconnect this acceptor
        if (getSocketError() == PEGASUS_NETWORK_TCPIP_STOPPED)
        {
            PEG_TRACE_CSTRING(TRC_DISCARDED_DATA, Tracer::LEVEL1,
                "Socket has an IO error. TCP/IP down. Try to reconnect.");

            reconnectConnectionSocket();

            return;
        }
        PEG_TRACE((
            TRC_DISCARDED_DATA,
            Tracer::LEVEL1,
            "HTTPAcceptor: accept() failed.  errno: %u",
            errno));
        return;
    }

    // Use an AutoPtr to ensure the socket handle is closed on exception
    AutoPtr<SocketHandle, CloseSocketHandle> socketPtr(&socket);

#ifndef PEGASUS_OS_TYPE_WINDOWS
    // We need to ensure that the socket number is not higher than
    // what fits into FD_SETSIZE, because we else won't be able to select on it
    // and won't ever communicate correct on that socket.
    if (socket >= FD_SETSIZE)
    {
        // the remote connection is invalid, destroy client address.
        delete accept_address;

        PEG_TRACE(
            (TRC_DISCARDED_DATA,
             Tracer::LEVEL1,
             "HTTPAcceptor out of available sockets."
                 "accept() returned too large socket number %u."
                 "Closing connection to the new client.",
             socket));

        return;
    }
#endif

    String ipAddress;

    if (_connectionType == LOCAL_CONNECTION)
    {
        ipAddress = "localhost";
    }
    else
    {
#ifdef PEGASUS_ENABLE_IPV6
        char ipBuffer[PEGASUS_INET6_ADDRSTR_LEN];
        if (System::getNameInfo(accept_address,
                address_size,
                ipBuffer,
                PEGASUS_INET6_ADDRSTR_LEN,
                0,
                0,
                NI_NUMERICHOST))
        {
            delete accept_address;
            return;
        }
        ipAddress = ipBuffer;
#else
        unsigned char* sa = reinterpret_cast<unsigned char*>(
            &reinterpret_cast<struct sockaddr_in*>(
                accept_address)->sin_addr.s_addr);
        char ipBuffer[32];
        sprintf(ipBuffer, "%u.%u.%u.%u", sa[0], sa[1], sa[2], sa[3]);
        ipAddress = ipBuffer;
#endif
    }

    delete accept_address;

// set the close on exec flag
#if !defined(PEGASUS_OS_TYPE_WINDOWS) && !defined(PEGASUS_OS_VMS)
    int sock_flags;
    if ((sock_flags = fcntl(socket, F_GETFD, 0)) < 0)
    {
        PEG_TRACE_CSTRING(TRC_DISCARDED_DATA, Tracer::LEVEL1,
            "HTTPAcceptor: fcntl(F_GETFD) failed");
    }
    else
    {
        sock_flags |= FD_CLOEXEC;
        if (fcntl(socket, F_SETFD, sock_flags) < 0)
        {
            PEG_TRACE_CSTRING(TRC_DISCARDED_DATA, Tracer::LEVEL1,
                "HTTPAcceptor: fcntl(F_SETFD) failed");
        }
    }
#endif


    PEG_TRACE((
        TRC_HTTP,
        Tracer::LEVEL3,
        "HTTPAcceptor - accept() success.  Socket: %u",
        socket));

    SharedPtr<MP_Socket> mp_socket(new MP_Socket(
        socket, _sslcontext, _sslContextObjectLock, ipAddress));
    // mp_socket now has responsibility for closing the socket handle
    socketPtr.release();

    mp_socket->disableBlocking();

    {
#ifndef PEGASUS_INTEGERS_BOUNDARY_ALIGNED
        AutoMutex lock(_socketWriteTimeoutMutex);
#endif
        mp_socket->setSocketWriteTimeout(_socketWriteTimeout);
    }

    // Perform the SSL handshake, if applicable.

    Sint32 socketAcceptStatus = mp_socket->accept();

    if (socketAcceptStatus < 0)
    {
        PEG_TRACE_CSTRING(TRC_DISCARDED_DATA, Tracer::LEVEL1,
            "HTTPAcceptor: SSL_accept() failed");
        return;
    }

    // Create a new connection and add it to the connection list:

    AutoPtr<HTTPConnection> connection(new HTTPConnection(
        _monitor,
        mp_socket,
        ipAddress,
        this,
        _outputMessageQueue));

    if (HTTPConnection::getIdleConnectionTimeout())
    {
        Time::gettimeofday(&connection->_idleStartTime);
    }

    if (socketAcceptStatus == 0)
    {
        PEG_TRACE_CSTRING(TRC_HTTP, Tracer::LEVEL1,
            "HTTPAcceptor: SSL_accept() pending");
        connection->_acceptPending = true;
        Time::gettimeofday(&connection->_acceptPendingStartTime);
    }

    // Solicit events on this new connection's socket:
    int index;

    if (-1 ==  (index = _monitor->solicitSocketMessages(
            connection->getSocket(),
            connection->getQueueId(), MonitorEntry::TYPE_CONNECTION)) )
    {
        PEG_TRACE_CSTRING(TRC_DISCARDED_DATA, Tracer::LEVEL1,
            "HTTPAcceptor::_acceptConnection: Attempt to allocate entry in "
                "_entries table failed.");
        return;
    }

    connection->_entry_index = index;
    AutoMutex autoMut(_rep->_connection_mut);
    _rep->connections.append(connection.get());
    connection.release();
}

PEGASUS_NAMESPACE_END
