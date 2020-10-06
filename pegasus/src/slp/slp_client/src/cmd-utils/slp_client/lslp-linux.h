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
 *  Description:
 *
 *  Originated: March 15, 2000
 *  Original Author: Mike Day md@soft-hackle.net
 *                                mdd@us.ibm.com
 *
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

#ifndef _LSLP_LINUX_INCLUDE_
#define _LSLP_LINUX_INCLUDE_

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include <sys/time.h>
#include <pthread.h>
#ifndef PEGASUS_OS_ZOS
# include <semaphore.h>
#endif
#ifdef PEGASUS_OS_ZOS
# include <netdb.h>
# include <strings.h>
#endif
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netdb.h>
#include <errno.h>
#include <Pegasus/Common/PegasusAssert.h>
#include <memory.h>
#include <string.h>
#include <ctype.h>
#include <net/if.h>
typedef unsigned char uint8;
typedef uint8 byte;
typedef short int16;
typedef unsigned short uint16;
typedef int int32;
typedef unsigned int uint32;
typedef long long int64;
typedef unsigned long long uint64;
typedef uint32 BOOL;
#ifdef __cplusplus
extern "C" {
#endif
#ifdef PEGASUS_OS_SOLARIS
# include "lslp-solaris.h"
#endif
#ifdef PEGASUS_OS_HPUX
# include "lslp-hpux.h"
#endif

void  num_to_ascii(uint32 val, char *buf, int32 radix, BOOL is_neg);

typedef int SOCKETD;

#ifdef PEGASUS_OS_ZOS
# define _LSLP_SLEEP(m) \
  { \
      if (m) \
      { \
          if (m <= 1000) \
          { \
              usleep(1); \
          } \
          else \
          { \
              sleep( m / 1000000000); \
              usleep((m % 1000000000) / 1000); \
          } \
      } \
  }
#else
# define _LSLP_SLEEP(m) \
   { \
       if (m) \
       { \
           struct timespec wait_time , actual_time; \
           wait_time.tv_sec = (m / 1000); ; \
           wait_time.tv_nsec = (((m % 1000) * 1000) * 1000);  \
           nanosleep(&wait_time, &actual_time); \
       } \
   }
#endif

#define INVALID_SOCKET -1
#define SOCKET_ERROR -1

/** void *(*start)(void *), ustacksize, void *arg  **/

#ifndef PEGASUS_OS_ZOS
# define _LSLP_STRTOK(n, d, s) strtok_r((n), (d), (s))
#else
# define _LSLP_STRTOK(n, d, s) strtok((n), (d) )
#endif

#define SOCKADDR_IN struct sockaddr_in
#define SOCKADDR struct sockaddr
#define SOCKET int

#ifndef TRUE
# define TRUE 1
#endif
#ifndef FALSE
# define FALSE 0
#endif

#define _itoa(v, b, r) num_to_ascii((uint32)(v), (b), (r), (((r) == 10) \
    && ((int32)(v) < 0)))

#define _LSLP_CLOSESOCKET close
#ifdef PEGASUS_OS_ZOS
 SOCKETD _lslp_socket(int domain, int type, int protocol);
# define _LSLP_SOCKET(a, b, c) _lslp_socket((int)(a), (int)(b), (int)(c))
#else
# define _LSLP_SOCKET(a, b, c) socket((int)(a), (int)(b), (int)(c))
#endif
#define _LSLP_BIND(a, b, c) bind((int)(a), (const struct sockaddr *)(b),\
    (socklen_t)(c))
#ifndef _LSLP_SENDTO
# define _LSLP_SENDTO(a, b, c, d, e, f) \
    sendto((int)(a), (const void *)(b), (size_t)(c), (int)(d), \
    (const struct sockaddr *)(e), (socklen_t)(f))
#endif
#ifndef _LSLP_RECV_FROM
# define _LSLP_RECV_FROM(a, b, c, d, e, f) \
    recvfrom((int)(a), (void *)(b), (size_t)(c), (int)(d), \
    (struct sockaddr *)(e), (socklen_t *)(f))
#endif
#ifndef _LSLP_SETSOCKOPT
# define _LSLP_SETSOCKOPT(a, b, c, d, e) \
    setsockopt((int)(a), (int)(b), (int)(c), (const void *)(d), (socklen_t)(e))
#endif
#define _LSLP_SET_TTL(s, t)  setsockopt((s), IPPROTO_IP, IP_MULTICAST_TTL, \
    (const char *)&(t), sizeof((t)))

#define LSLP_FD_SET fd_set

#define _LSLP_SELECT(a, b, c, d, e) \
    select((int)(a), (fd_set *)(b), (fd_set *)(c), (fd_set *)(d),\
    (struct timeval *)(e))
#define _LSLP_FD_ISSET(a, b) FD_ISSET((int)(a), (fd_set *)(b))
#define _LSLP_FD_SET(a, b) FD_SET((int)(a), (fd_set *)(b))
#define _LSLP_FD_ZERO(a) FD_ZERO((fd_set *)(a))

#define LSLP_MTU 4096

/* ascii and char tests and conversions */

#define _LSLP_ISASCII(a) isascii(a)

#ifdef __cplusplus
}
#endif

#endif /* _LSLP_LINUX_INCLUDE_ */
