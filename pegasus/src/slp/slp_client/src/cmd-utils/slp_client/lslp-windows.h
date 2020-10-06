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
/*****************************************************************************
 *  Description:   portability routines for WINDOZE
 *
 *  Originated: December 31, 1999
 *  Original Author: Mike Day md@soft-hackle.net
 *                                mdday@us.ibm.com
 *
 *  Copyright (c) 2001 - 2003  IBM
 *  Copyright (c) 2000 - 2003 Michael Day
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a
 *  copy of this software and associated documentation files (the "Software"),
 *  to deal in the Software without restriction, including without limitation
 *  the rights to use, copy, modify, merge, publish, distribute, sublicense,
 *  and/or sell copies of the Software, and to permit persons to whom the
 *  Software is furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in
 *  all copies or substantial portions of the Software.
 *
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 *  THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 *  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 *  DEALINGS IN THE SOFTWARE.
 *
 *****************************************************************************/



#ifndef _LSLP_WINDOWS_INCL_
#define _LSLP_WINDOWS_INCL_

#if defined(PEGASUS_PLATFORM_WIN64_IA64_MSVC) || \
    defined(PEGASUS_PLATFORM_WIN64_X86_64_MSVC) || \
    defined(PEGASUS_PLATFORM_WIN32_IX86_MSVC)
#ifndef _MT
# define _MT
#endif

#define WIN32_LEAN_AND_MEAN

#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <stdlib.h>
#include <process.h>    /* _beginthread, _endthread */
#include <winbase.h>
#include <tchar.h>
#include <sys/types.h>
#include <sys/timeb.h>
#include <Pegasus/Common/PegasusAssert.h>
#include <errno.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef unsigned char uint8;
typedef uint8 byte;
typedef short int16;
typedef unsigned short uint16;
typedef int int32;
typedef unsigned int uint32;
typedef __int64 int64;
typedef unsigned __int64 uint64;
typedef SOCKET SOCKETD;

typedef struct socket_address
{
    struct sockaddr_in *address;
    int address_len;
} socket_addr ;

typedef struct socket_address_list
{
    int count;
    socket_addr *list;
} socket_addr_list;

int WindowsStartNetwork();
/* int32 gettimeofday(struct timeval *tv, struct timezone *tz); */

/* WIN 32 macros for mutex semaphore */


/* void SLEEP(int32 milliseconds) */
#define _LSLP_SLEEP Sleep
#define _LSLP_SET_TTL(s, t)  setsockopt((s), IPPROTO_IP, IP_MULTICAST_TTL, \
    (const char *)&(t), sizeof((t)))

/* void *(*start)(void *), ustacksize, void *arg           */

#define _LSLP_STRTOK(n, d, s) strtok((n), (d) )

#define strncasecmp strnicmp

/* maximum path length */
#endif   /* WIN 32 */


// Some versions of Visual C++ such as Visual C++ 10 
// have the  error codes defined in errno.h,
// defining that error  in lslp-windows.h cause
// a lot of warning of redefinition .
// To avoid these warning ,undef these error 
// codes if the errors code are not matching 

#if defined (ENOTSOCK) && (ENOTSOCK != WSAENOTSOCK)
#undef ENOTSOCK
#endif 
#if defined (EADDRNOTAVAIL) && (EADDRNOTAVAIL != WSAEADDRNOTAVAIL)
#undef EADDRNOTAVAIL
#endif 
#if defined (EAFNOSUPPORT) && (EAFNOSUPPORT != WSAEAFNOSUPPORT)
#undef EAFNOSUPPORT
#endif 
#if defined (EISCONN) && (EISCONN != WSAEISCONN)
#undef EISCONN
#endif 
#if defined (ETIMEDOUT) && (ETIMEDOUT != WSAETIMEDOUT)
#undef ETIMEDOUT
#endif 
#if defined (ECONNREFUSED) && (ECONNREFUSED != WSAECONNREFUSED)
#undef ECONNREFUSED
#endif 
#if defined (ENETUNREACH) && (ENETUNREACH != WSAENETUNREACH)
#undef ENETUNREACH
#endif 
#if defined (EADDRINUSE) && (EADDRINUSE != WSAEADDRINUSE)
#undef EADDRINUSE
#endif 
#if defined (EINPROGRESS) && (EINPROGRESS != WSAEINPROGRESS)
#undef EINPROGRESS
#endif
#if defined (EALREADY) && (EALREADY != WSAEALREADY)
#undef EALREADY
#endif 
#if defined (EWOULDBLOCK) && (EWOULDBLOCK != WSAEWOULDBLOCK)
#undef EWOULDBLOCK
#endif
 
#if !defined (ENOTSOCK)
#define ENOTSOCK WSAENOTSOCK
#endif
#if !defined (EADDRNOTAVAIL)
#define EADDRNOTAVAIL WSAEADDRNOTAVAIL
#endif 
#if !defined (EAFNOSUPPORT)
#define EAFNOSUPPORT WSAEAFNOSUPPORT
#endif 
#if !defined (EISCONN)
#define EISCONN WSAEISCONN
#endif 
#if !defined (ETIMEDOUT)
#define ETIMEDOUT WSAETIMEDOUT
#endif
#if !defined (ECONNREFUSED)
#define ECONNREFUSED WSAECONNREFUSED
#endif
#if !defined (ENETUNREACH)
#define ENETUNREACH WSAENETUNREACH
#endif 
#if !defined (EADDRINUSE)
#define EADDRINUSE WSAEADDRINUSE
#endif
#if !defined (EINPROGRESS)
#define EINPROGRESS WSAEINPROGRESS
#endif
#if !defined (EALREADY)
#define EALREADY WSAEALREADY
#endif
#if !defined (EWOULDBLOCK)
#define EWOULDBLOCK WSAEWOULDBLOCK
#endif


#define _LSLP_SOCKET(a, b, c) socket(((int)(a)), ((int)(b)), ((int)(c)))
#define _LSLP_CLOSESOCKET closesocket
#define _LSLP_BIND(a, b, c) bind(((SOCKET)(a)), \
    ((const struct sockaddr *)(b)), ((int)(c)))
#define _LSLP_SENDTO(a, b, c, d, e, f) \
    sendto(((SOCKET)(a)), ((const char *)(b)), ((int)(c)), ((int)(d)), \
    ((const struct sockaddr *)(e)), ((int)(f)))
#define _LSLP_RECV_FROM(a, b, c, d, e, f) \
    recvfrom(((SOCKET)(a)), ((char *)(b)), ((int)(c)), ((int)(d)), \
    ((struct sockaddr *)(e)), ((int *)(f)))


#define _LSLP_SETSOCKOPT(a, b, c, d, e) \
    setsockopt(((SOCKET)(a)), ((int)(b)), ((int)(c)), ((const char *)(d)),\
    ((int)(e)))
#define _LSLP_SELECT(a, b, c, d, e) \
    select(((int)(a)), ((fd_set *)(b)), ((fd_set *)(c)), \
    ((fd_set *)(d)), ((const struct timeval *)(e)))
#define _LSLP_FD_ISSET(a, b)  FD_ISSET(((SOCKET)(a)), ((fd_set *)(b)))
#define _LSLP_FD_SET(a, b) FD_SET(((SOCKET)(a)), ((fd_set *)(b)))
#define _LSLP_FD_ZERO(a) FD_ZERO((fd_set *)(a))

#define LSLP_MTU 4096

#define LSLP_FD_SET fd_set
#define strcasecmp _stricmp

/* ascii and char tests and conversions */

#define _LSLP_ISASCII(a) __isascii(a)

#ifdef  __cplusplus
}
#endif

#endif /*_LSLP_WINDOWS_INCL_*/

