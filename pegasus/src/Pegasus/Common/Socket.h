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

#ifndef Pegasus_Socket_h
#define Pegasus_Socket_h

#include <cctype>
#include <Pegasus/Common/Network.h>
#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/Linkage.h>

#ifdef PEGASUS_OS_ZOS
#   include <string.h>  // added by rk for memcpy
#endif

PEGASUS_NAMESPACE_BEGIN

class Socket
{
public:

    /**
        Connects the specified socket to the specified address.  If the socket
        is non-blocking, the connect attempt will time out after the specified
        interval.

        @param socket The socket to connect.
        @param address The address to which to connect the socket
        @param addressLength The length of the sockaddr buffer in which the
            address is specified.
        @param timeoutMilliseconds The number of milliseconds after which the
            connect attempt should time out, if the socket is non-blocking.
        @return True if the connect attempt is successful, false otherwise.
    */
    static Boolean timedConnect(
        SocketHandle socket,
        sockaddr* address,
        int addressLength,
        Uint32 timeoutMilliseconds);

    static Sint32 read(SocketHandle socket, void* ptr, Uint32 size);

    static Sint32 peek(SocketHandle socket, void* ptr, Uint32 size);

    static Sint32 write(SocketHandle socket, const void* ptr, Uint32 size);

    static Sint32 timedWrite(SocketHandle socket,
                             const void* ptr,
                             Uint32 size,
                             Uint32 socketWriteTimeout);

    /**
        Closes a specified socket.  If successful, the socket handle is set to
        PEGASUS_INVALID_SOCKET.
        @param socket The SocketHandle to close.
    */
    static void close(SocketHandle& socket);

    static void disableBlocking(SocketHandle socket);

    static void initializeInterface();

    static void uninitializeInterface();

    static SocketHandle createSocket(int domain, int type, int protocol);

private:
    Socket();
};

// Use with AutoPtr to automatically close a socket on error
struct CloseSocketHandle
{
    void operator()(SocketHandle* ptr)
    {
        if (ptr)
        {
            Socket::close(*ptr);
        }
    }
};

PEGASUS_NAMESPACE_END

#endif /* Pegasus_Socket_h */
