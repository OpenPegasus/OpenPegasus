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

#ifndef Pegasus_Network_h
#define Pegasus_Network_h

#include <Pegasus/Common/Config.h>

//==============================================================================
//
// Network.h
//
//     This file includes network-related system-header files. Please include
//     this file directly rather than including system headers directly. If
//     special inclusions are necessary for any platform, please add them to
//     this file rather than other files. The reason for this file is to limit
//     platform-specific conditional compilation expressions to only a few
//     well-known header files.
//
//==============================================================================

//------------------------------------------------------------------------------
//
// PEGASUS_OS_TYPE_WINDOWS network system header files
//
//------------------------------------------------------------------------------

#ifdef PEGASUS_OS_TYPE_WINDOWS
#   ifdef FD_SETSIZE
#       ifndef PEGASUS_WMIMAPPER
#           error "<Pegasus/Common/Network.h>: FD_SETSIZE is already defined. \
             This file must be included prior to any header file that defines \
             FD_SETSIZE, such as <windows.h>, <winsock.h>, or <winsock2.h>."
#       else
#           undef FD_SETSIZE
#       endif
#   endif
#   define FD_SETSIZE 1024
#   include <windows.h>
#   ifndef _WINSOCKAPI_
#       include <winsock2.h>
#   endif
#   include <wincrypt.h>
#   include <ws2tcpip.h>
#endif

#    ifdef PEGASUS_HAS_GETIFADDRS
#       include <ifaddrs.h>
#    endif

//------------------------------------------------------------------------------
//
// PEGASUS_OS_TYPE_UNIX or PEGASUS_OS_VMS network system header files.
//
//------------------------------------------------------------------------------

#if defined(PEGASUS_OS_TYPE_UNIX) || defined (PEGASUS_OS_VMS)

#   include <errno.h>
#   include <sys/types.h>
#   include <fcntl.h>
#   include <netdb.h>
#   include <netinet/in.h>
#   include <arpa/inet.h>
#   include <sys/socket.h>
#   include <sys/time.h>
#   include <sys/ioctl.h>
#   ifndef PEGASUS_DISABLE_LOCAL_DOMAIN_SOCKET
#       include <unistd.h>
#       include <sys/un.h>
#   endif
#   include <unistd.h>
#   ifdef PEGASUS_OS_ZOS
#       ifndef TCP_NODELAY
#           define TCP_NODELAY 1
#       endif
#   else
#       include <netinet/tcp.h>
#   endif
//------------------------------------------------------------------------------
// 
// if_nametoindex() in libC on HP_UX does not support name mangling.
// Therefore, have "C" linkage for if.h on HP-UX.
// 
//------------------------------------------------------------------------------
#   ifdef PEGASUS_OS_HPUX
extern "C"
{
#       include <net/if.h>
}
#   else
#       include <net/if.h>
#   endif
#endif

//------------------------------------------------------------------------------
//
// PEGASUS_RETRY_SYSTEM_CALL()
//
//     This macro repeats the given system call until it returns something
//     other than EINTR.
//
//------------------------------------------------------------------------------

#ifdef PEGASUS_OS_TYPE_WINDOWS
#   define PEGASUS_RETRY_SYSTEM_CALL(EXPR, RESULT) RESULT = EXPR
#else
#   define PEGASUS_RETRY_SYSTEM_CALL(EXPR, RESULT) \
        while (((RESULT = (EXPR)) == -1) && (errno == EINTR))
#endif

//------------------------------------------------------------------------------
//
// PEGASUS_SOCKET_ERROR
//
//------------------------------------------------------------------------------

#ifdef PEGASUS_OS_TYPE_WINDOWS
#   define PEGASUS_SOCKET_ERROR SOCKET_ERROR
#else
#   define PEGASUS_SOCKET_ERROR (-1)
#endif

//------------------------------------------------------------------------------
//
// PEGASUS_NETWORK_TCPIP_STOPPED
//
// This return code indicates that the transpor layer is
// stopped and the socket is invalid. The socket must created again.
//
//------------------------------------------------------------------------------

#ifdef PEGASUS_OS_ZOS
#   define PEGASUS_NETWORK_TCPIP_STOPPED EIO
#else
#   define PEGASUS_NETWORK_TCPIP_STOPPED 0
#endif

//------------------------------------------------------------------------------
//
// PEGASUS_NETWORK_TRYAGAIN
//
// This return code indicates that the network function
// should be tried again by the program.
//
//------------------------------------------------------------------------------

#if !defined(PEGASUS_OS_TYPE_WINDOWS)
#   define PEGASUS_NETWORK_TRYAGAIN EAGAIN
#else
#   define PEGASUS_NETWORK_TRYAGAIN 0
#endif

//------------------------------------------------------------------------------
//
// PEGASUS_NETWORK_EINPROGRESS
//
// This return code indicates that the network function
// is in progress. The application should try select or poll and
// check for successful completion.
//
//------------------------------------------------------------------------------

#if !defined(PEGASUS_OS_TYPE_WINDOWS)
#   define PEGASUS_NETWORK_EINPROGRESS EINPROGRESS
#else
#   define PEGASUS_NETWORK_EINPROGRESS WSAEWOULDBLOCK
#endif

////////////////////////////////////////////////////////////////////////////////
//
// getSocketError()
//
////////////////////////////////////////////////////////////////////////////////

static inline int getSocketError()
{
#ifdef PEGASUS_OS_TYPE_WINDOWS
    return WSAGetLastError();
#else
    return errno;
#endif
}

//------------------------------------------------------------------------------
//
// PEGASUS_INVALID_SOCKET
//
//------------------------------------------------------------------------------

#ifdef PEGASUS_OS_TYPE_WINDOWS
#   define PEGASUS_INVALID_SOCKET INVALID_SOCKET
#else
#   define PEGASUS_INVALID_SOCKET (-1)
#endif

//------------------------------------------------------------------------------
//
//
// PEGASUS_INVALID_ADDRESS_FAMILY
//
//------------------------------------------------------------------------------

#   ifdef PEGASUS_OS_TYPE_WINDOWS
#      define PEGASUS_INVALID_ADDRESS_FAMILY WSAEAFNOSUPPORT
#   elif defined(PEGASUS_OS_HPUX)
#      define PEGASUS_INVALID_ADDRESS_FAMILY EPROTONOSUPPORT
#   else
#      define PEGASUS_INVALID_ADDRESS_FAMILY EAFNOSUPPORT
#   endif

//------------------------------------------------------------------------------
//
// SocketHandle
//
//------------------------------------------------------------------------------

#ifdef PEGASUS_OS_TYPE_WINDOWS
typedef SOCKET SocketHandle;
#else
typedef int SocketHandle;
#endif

//------------------------------------------------------------------------------
//
// SocketLength
//
//------------------------------------------------------------------------------

#if defined(PEGASUS_OS_SOLARIS) || \
    defined(PEGASUS_PLATFORM_TRU64_ALPHA_DECCXX) || \
    defined(PEGASUS_PLATFORM_WIN64_IA64_MSVC) || \
    defined(PEGASUS_PLATFORM_WIN64_X86_64_MSVC) || \
    defined(PEGASUS_PLATFORM_WIN32_IX86_MSVC)
    typedef int SocketLength;
#elif defined(PEGASUS_PLATFORM_VMS_ALPHA_DECCXX) || \
    defined(PEGASUS_PLATFORM_VMS_IA64_DECCXX)
    typedef size_t SocketLength;
#elif defined(PEGASUS_OS_ZOS)
    typedef socklen_t SocketLength;
#elif defined(PEGASUS_PLATFORM_HPUX_IA64_ACC) && \
    !defined(_XOPEN_SOURCE_EXTENDED)
    typedef int SocketLength;
#elif defined(PEGASUS_PLATFORM_HPUX_IA64_GNU) && \
    !defined(_XOPEN_SOURCE_EXTENDED)
    typedef int SocketLength;
#elif defined(PEGASUS_PLATFORM_HPUX_PARISC_ACC) && \
    !defined(_XOPEN_SOURCE_EXTENDED)
    typedef int SocketLength;
#elif defined(PEGASUS_PLATFORM_HPUX_PARISC_GNU) && \
    !defined(_XOPEN_SOURCE_EXTENDED)
    typedef int SocketLength;
#else
    typedef socklen_t SocketLength;
#endif

#endif  /* Pegasus_Network_h */
