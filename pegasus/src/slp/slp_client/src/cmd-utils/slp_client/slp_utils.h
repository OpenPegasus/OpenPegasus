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

#ifndef Pegasus_SLP_Utils_h
#define Pegasus_SLP_Utils_h

#include <Pegasus/Common/Config.h>

#ifndef NUCLEUS
#include "lslp-common-defs.h"
#include "lslp.h"
#endif

#ifdef PEGASUS_OS_VMS
#include <netinet/in6.h>
#endif

#   ifdef PEGASUS_OS_TYPE_WINDOWS
#      define SLP_INVALID_ADDRESS_FAMILY WSAEAFNOSUPPORT
#   elif defined(PEGASUS_OS_HPUX)
#      define SLP_INVALID_ADDRESS_FAMILY EPROTONOSUPPORT
#   else
#      define SLP_INVALID_ADDRESS_FAMILY EAFNOSUPPORT
#   endif

#ifdef PEGASUS_OS_TYPE_WINDOWS
#   define SLP_INVALID_SOCKET INVALID_SOCKET
#else
#   define SLP_INVALID_SOCKET (-1)
#endif

#ifdef PEGASUS_ENABLE_IPV6

#define SOCKADDR_IN6 struct sockaddr_in6
#define SOCKADDR_STORAGE struct sockaddr_storage

#ifdef PEGASUS_HAS_GETIFADDRS
#include <net/if.h>
#include <ifaddrs.h> // for getifaddrs() function
#endif

#endif

#ifdef SLP_LIB_IMPORT
# define SLP_STORAGE_DECL PEGASUS_IMPORT
#else
# define SLP_STORAGE_DECL PEGASUS_EXPORT
#endif

#ifdef INET_ADDRSTRLEN
#define PEGASUS_INET_ADDRSTR_LEN INET_ADDRSTRLEN
#else
#define PEGASUS_INET_ADDRSTR_LEN 16
#endif

#ifdef INET6_ADDRSTRLEN
#define PEGASUS_INET6_ADDRSTR_LEN INET6_ADDRSTRLEN
#else
#define PEGASUS_INET6_ADDRSTR_LEN 46
#endif

#ifdef IN6ADDR_LOOPBACK_INIT
#define PEGASUS_IPV6_LOOPBACK_INIT IN6ADDR_LOOPBACK_INIT
#else
#define PEGASUS_IPV6_LOOPBACK_INIT {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1}
#endif


#ifdef INADDR_LOOPBACK
#define PEGASUS_IPV4_LOOPBACK_INIT INADDR_LOOPBACK
#else
#define PEGASUS_IPV4_LOOPBACK_INIT 0x7F000001
#endif

#define PEGASUS_IPV4_LOOPBACK_RANGE_START 0x7F000000
#define PEGASUS_IPV4_LOOPBACK_RANGE_END 0x7FFFFFFF

#ifdef PEGASUS_ENABLE_IPV6
#define PEGASUS_IN6_ADDR_SIZE (sizeof (struct in6_addr))
#else
#define PEGASUS_IN6_ADDR_SIZE 0x10
#endif

#ifdef    __cplusplus
extern "C"
{
#endif

/**
    SLP service Name
*/
#define SLP_SERVICE_NAME  "svrloc"

/**
    Default SLP Port Number
*/
#define DEFAULT_SLP_PORT  427

/**
    Verifies given IPv4 Address and returns true if it is valid.
*/
SLP_STORAGE_DECL BOOL slp_is_valid_ip4_addr(const char *ipv4_addr);

/**
    Verifies given IPv6 Address (without brackets) and returns
    true if it is valid.
*/
SLP_STORAGE_DECL BOOL slp_is_valid_ip6_addr(const char *ip6_addr);

/**
    Verifies given hostName and returns true if it is valid.
*/
SLP_STORAGE_DECL BOOL slp_is_valid_host_name(const char *host);

/**
    The slp_ntop (inet_ntop()) function shall convert a numeric
    address into a text string suitable for presentation. The af argument
    shall specify the family of the address.
*/
SLP_STORAGE_DECL const char *slp_ntop(int af, const void *src,
    char *dst, int size);

/*
    The  slp_pton (inet_pton())  function  shall convert an
    address in its standard text presentation form into its numeric
    binary form.
*/
SLP_STORAGE_DECL int slp_pton(int af, const char *src, void *dst);

/**
    Checks whether the two addresses in binary form are equal based on
    address family. af can be AF_INET or AF_INET6.
*/
SLP_STORAGE_DECL BOOL slp_addr_equal(int af, void *p1, void *p2);

SLP_STORAGE_DECL BOOL slp_is_loop_back(int af, void *addr);

SLP_STORAGE_DECL BOOL slp_is_loop_back_addr(char *addr);

// See RFC 3111 sec 4.1
unsigned long slp_hash(const char *pc, unsigned int len);

SLP_STORAGE_DECL BOOL slp_is_ip4_stack_active();
SLP_STORAGE_DECL BOOL slp_is_ip6_stack_active();

#ifdef __cplusplus
}
#endif

#endif //Pegasus_SLP_Utils_h
