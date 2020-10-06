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
 *  Originated: December 20, 2001
 *    Original Author: Mike Day md@soft-hackle.net
 *                                mdday@us.ibm.com
 *
 *
 *  Copyright (c) 2001 - 2004  IBM
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

/* make pattern matching work with a length parameter, like strncmp(a,b) */
/* <<< Wed Aug  7 20:37:13 2002 mdd >>> */

/* make all direct calls static (hidden), public calls to be exposed
   through client->() */
/* << Mon Sep 16 14:00:36 2002 mdd >> */

#include "slp_client.h"
#include "slp_utils.h"

/*********************************************************************/
/*********************************************************************/


/***************** #define SLP_CLIENT_DEBUG 1 *********************/
//#define SLP_CLIENT_DEBUG 1

#ifdef SLP_CLIENT_DEBUG

/*
** DEBUG_PRINT_CONTROL values
*/
#define DEBUG_ENTER  1
#define DEBUG_EXIT   2
#define DEBUG_LEVEL1 4
#define DEBUG_LEVEL2 8
#define DEBUG_LEVEL3 16
#define DEBUG_LEVEL4 32

#define ALL DEBUG_ENTER | DEBUG_EXIT | DEBUG_LEVEL1 | DEBUG_LEVEL2 \
    | DEBUG_LEVEL3 | DEBUG_LEVEL4

/*
** DEBUG_PRINT_CONTROL is set to the control what is printed
*/
int DEBUG_PRINT_CONTROL = ALL;

/* int DEBUG_PRINT_CONTROL = 0; */

/*
** DEBUG_PRINT (DC, FS, AS)
**
** args:
**    DC = Debug Control
**    FS = Format String
**    AS = Arg String
**
** examples:
**  DEBUG_PRINT((DEBUG_ENTER, "function name"));
**  DEBUG_PRINT((DEBUG_ENTER, "function name arg1=%d arg2=%s", arg1, arg2));
**  DEBUG_PRINT((DEBUG_EXIT, "%s rtn=%d", func_name, rtn));
**  DEBUG_PRINT((DEBUG_LEVEL1, "%s v1=0x%0x, v1=%d", func_name, v1, v1));
**
**
*/

#define fdout stdout

static void _debug_print(int dc, const char* format, ...)
{
    va_list ap;
    va_start(ap, format);


    if (DEBUG_PRINT_CONTROL & dc)
    {
        fprintf(fdout, "SLP_CLIENT: ");

        if (DEBUG_ENTER & dc)
        {
            fprintf(fdout, " ENTER -- ");
        }
        else if (DEBUG_EXIT & dc)
        {
            fprintf(fdout, " EXIT  -- ");
        }
        else
        {
            fprintf(fdout, " DATA  -- ");
        }

        vfprintf(fdout, format, ap);

        fprintf(fdout, "\n");
    }

    va_end(ap);
    return;
}

#define DEBUG_PRINT(ARGS) _debug_print ARGS

#else /* SLP_CLIENT_DEBUG */

#define DEBUG_PRINT(ARGS)

#endif /* SLP_CLIENT_DEBUG */

/*********************************************************************/

#ifdef PEGASUS_OS_ZOS

#include <ctest.h>

//
// This functions has been duplicated from
// src/Pegasus/Common/PegasusAssertZOS.cpp to solve
// a circular build dependency.
// The library pegcommon can not be build prior pegslp_client which would
// be necessary to get this function.
//

void __pegasus_assert_zOS(const char* file, int line, const char* cond)
{
    // a buffer to compose the messages
    char msgBuffer[1024];

    sprintf(msgBuffer,"PEGASUS_ASSERT: Assertation \'%s\' failed",cond);
    fprintf(stderr,"\n%s in file %s ,line %d\n",msgBuffer,file,line);

    // generate stacktace
    ctrace(msgBuffer);

    // flush trace buffers has been leftout because the slp client does not use
    // tracing.

    // If env vars are set, a SYSM dump is generated.
    kill(getpid(),SIGDUMP);
}

#endif

// Bug 6545 added the LSLP_REUSEADDR macro because SO_REUSEADDR behaves
// differently on AIX versus other POSIX platforms.
#if defined(PEGASUS_PLATFORM_AIX_RS_IBMCXX)
#    define LSLP_REUSEADDR SO_REUSEPORT
#else
#    define LSLP_REUSEADDR SO_REUSEADDR
#endif

#if defined(PEGASUS_PLATFORM_WIN64_IA64_MSVC) || \
    defined(PEGASUS_PLATFORM_WIN64_X86_64_MSVC) || \
    defined(PEGASUS_PLATFORM_WIN32_IX86_MSVC)
int _winsock_count = 0;
WSADATA _wsa_data ;
#   include <time.h>
#endif

struct da_list *alloc_da_node(BOOL head)
{
    struct da_list *node = (struct da_list *)calloc(1, sizeof(struct da_list));
    DEBUG_PRINT((DEBUG_ENTER, "decode_attr_rply "));
    if (node != NULL && head == TRUE)
    {
        node->isHead = TRUE;
        node->next = node->prev = node;
    }
    return node;
}

struct da_list *da_node_exists(struct da_list *head, const void *key)
{

    if (head != NULL && _LSLP_IS_HEAD(head) && key != NULL)
    {
        struct da_list *temp = head->next;
        while (!_LSLP_IS_HEAD(temp))
        {
            if (! strcmp(temp->url, (const char *)key))
            {
                return temp;
            }
            temp = temp->next;
        }
    }
    return NULL;
}

void free_da_list_members(struct da_list *da)
{
    PEGASUS_ASSERT( ! _LSLP_IS_HEAD(da));
    if (da->url != NULL)
    {
        free(da->url);
    }
    if (da->scope != NULL)
    {
        free(da->scope);
    }
    if (da->attr != NULL)
    {
        free(da->attr);
    }
    if (da->spi != NULL)
    {
        free(da->spi);
    }
    if (da->auth != NULL)
    {
        free(da->auth);
    }
}

void free_da_list_node(struct da_list *da)
{
    PEGASUS_ASSERT( ! _LSLP_IS_HEAD(da));
    free_da_list_members(da);
    free(da);
}

/*
    Checks the address type in URL based on addr family.
*/
BOOL _slp_check_url_addr(const char *url, int af, void *url_bin_addr)
{
    char *p, *q, *r;
    BOOL match = FALSE;

    if (!url || !(p = strdup(url)))
    {
        return FALSE;
    }
    r = p;
    while (*p && *p != '/')
    {
        p++;
    }
    if (*p == '/' && *(p + 1) == '/')
    {
        p += 2;
        q = p;
       while(*p && *p != '/' && *p != ';' && *p != ']')
       {
           p++;
       }
       if (*q == '[' && *p == ']')
       {
           q++;
       }
       *p = 0;
       if (af == AF_INET)
       {
           if ((match = slp_is_valid_ip4_addr(q)) && url_bin_addr)
           {
               slp_pton(AF_INET, q, &url_bin_addr);
           }
       }
#ifdef PEGASUS_ENABLE_IPV6
       else if (af == AF_INET6)
       {
           if ((match = slp_is_valid_ip6_addr(q)) && url_bin_addr)
           {
               slp_pton(AF_INET6, q, &url_bin_addr);
           }
       }
#endif
    }
    free(r);
    return match;
}

/*** effectively reallocates *list -- FREES MEMORY ***/
static int _slp_get_local_interface(struct slp_if_addr **list, int af)
{
#if defined(PEGASUS_PLATFORM_WIN64_IA64_MSVC) || \
    defined(PEGASUS_PLATFORM_WIN64_X86_64_MSVC) || \
    defined(PEGASUS_PLATFORM_WIN32_IX86_MSVC)
    int errcode;
    int buf_size = 256;
#endif

    SOCKETD sock;
    int interfaces = 0;

    DEBUG_PRINT((DEBUG_ENTER, "_slp_get_local_interfaces "));

    if (list == NULL)
    {
        return 0;
    }

    if (*list != NULL)
    {
        free( *list );
    }
    *list = (struct slp_if_addr*)malloc(sizeof(struct slp_if_addr));
    (*list)->af = AF_UNSPEC;

#ifdef PEGASUS_ENABLE_IPV6
    struct slp_if_addr *ifp;
    if (af == AF_INET6)
    {
        if (!slp_is_ip6_stack_active())
        {
            return 0;
        }
#ifdef PEGASUS_HAS_GETIFADDRS
        {
            struct ifaddrs *array, *addrs;
            if (0 > getifaddrs(&array))
            {
                return 0;
            }
            for( addrs = array; addrs != NULL; addrs = addrs->ifa_next)
            {
                if(addrs->ifa_addr && (addrs->ifa_addr->sa_family == AF_INET6)
                    && (addrs->ifa_flags & IFF_UP))
                {
                    interfaces++;
                }
            }
            free(*list);
            *list  = (struct slp_if_addr *)
                calloc(interfaces + 2, sizeof(struct slp_if_addr));
            ifp = *list;
            for( addrs = array; addrs != NULL; addrs = addrs->ifa_next)
            {
                if(addrs->ifa_addr && (addrs->ifa_addr->sa_family == AF_INET6)
                    && (addrs->ifa_flags & IFF_UP))
                {
                    ifp->af = AF_INET6;
                    ifp->ip6_addr =
                        ((struct sockaddr_in6 *)addrs->ifa_addr)->sin6_addr;
                    ifp++;
                }
            }
            ifp->af = AF_UNSPEC;
            freeifaddrs(array);
        }
#endif // PEGASUS_HAS_GETIFADDRS
#ifdef PEGASUS_OS_ZOS
        if (-1 < (sock = _LSLP_SOCKET(AF_INET6, SOCK_DGRAM, 0)))
        {
            __net_ifconf6header_t ifConfHeader;
            __net_ifconf6entry_t *pifConfEntries;
            int interface_counter;

            memset(&ifConfHeader,0,sizeof(__net_ifconf6header_t));
            if (-1 == ioctl(sock, SIOCGIFCONF6, &ifConfHeader))
            {
                _LSLP_CLOSESOCKET(sock);
                DEBUG_PRINT((DEBUG_EXIT,
                    "_slp_get_local_interfaces: "
                        "zOS can not get IPV6 interfaces: %s",
                    strerror(errno)));
                return 0;
            }

            // Allocate the buffer for the entries.
            ifConfHeader.__nif6h_buffer=
                (char *)calloc(ifConfHeader.__nif6h_entries,
                    ifConfHeader.__nif6h_entrylen);

            ifConfHeader.__nif6h_buflen= ifConfHeader.__nif6h_entries *
                               ifConfHeader.__nif6h_entrylen;

            DEBUG_PRINT((DEBUG_LEVEL1,
               "_slp_get_local_interfaces: "
                   "There are %d of interface entries.",
               ifConfHeader.__nif6h_entries));

            if (-1 == ioctl(sock, SIOCGIFCONF6, &ifConfHeader))
            {
                _LSLP_CLOSESOCKET(sock);
                free(ifConfHeader.__nif6h_buffer);
                DEBUG_PRINT((DEBUG_EXIT,
                    "_slp_get_local_interfaces: "
                        "zOS can not get IPV6 interfaces entries: %s",
                    strerror(errno)));
                return 0;
            }

            pifConfEntries=(__net_ifconf6entry_t *)ifConfHeader.__nif6h_buffer;
            interfaces=ifConfHeader.__nif6h_entries;

            // now store the addresses
            free(*list);
            *list  = (struct slp_if_addr *)
                calloc(interfaces + 2, sizeof(struct slp_if_addr));
            ifp = *list;
            for (int i = 0 ; i < ifConfHeader.__nif6h_entries; i++)
            {
                if (!slp_is_loop_back(
                        AF_INET6,
                        &pifConfEntries[i].__nif6e_addr.sin6_addr))
                {
                    char buff[PEGASUS_INET6_ADDRSTR_LEN];
                    ifp->af = AF_INET6;
                    ifp->ip6_addr = pifConfEntries[i].__nif6e_addr.sin6_addr;
                    DEBUG_PRINT((DEBUG_LEVEL1,
                        "_slp_get_local_interfaces: IPV6 %s",
                        inet_ntop(
                            ifp->af,
                            &(ifp->ip6_addr),
                            buff ,
                            PEGASUS_INET6_ADDRSTR_LEN)));
                    ifp++;
                }
                else
                {
                    //a interface was a loop back
                    interfaces--;
                }
            }

            ifp->af = AF_UNSPEC; // list terminate

            free(ifConfHeader.__nif6h_buffer);
            _LSLP_CLOSESOCKET(sock);
        } // opened the socket

#endif // PEGASUS_OS_ZOS
        DEBUG_PRINT((DEBUG_EXIT,
            "_slp_get_local_interfaces: IPV6 interfaces %d.",
            interfaces));
        return(interfaces);
    }
#endif // PEGASUS_ENABLE_IPV6

    if (!slp_is_ip4_stack_active())
    {
        return 0;
    }

#if defined(PEGASUS_PLATFORM_WIN64_IA64_MSVC) || \
    defined(PEGASUS_PLATFORM_WIN64_X86_64_MSVC) || \
    defined(PEGASUS_PLATFORM_WIN32_IX86_MSVC)

    if (INVALID_SOCKET != (sock = WSASocket(af, SOCK_RAW,
        0, NULL, 0, 0)))
    {
        int bytes_returned;
        char output_buf[2048];

        if (0 == (errcode = WSAIoctl(sock, SIO_ADDRESS_LIST_QUERY, NULL, 0,
            output_buf, buf_size, (LPDWORD)&bytes_returned, NULL,
            NULL)))
        {

            SOCKET_ADDRESS_LIST *addr_list;
            SOCKET_ADDRESS *addr;
            struct slp_if_addr *ifp;
            SOCKADDR *sin;
            void *p;

            addr_list = (SOCKET_ADDRESS_LIST *)output_buf;
            free(*list);
            *list = (struct slp_if_addr *) malloc(
                sizeof(struct slp_if_addr)* (addr_list->iAddressCount + 1));
            addr = addr_list->Address;

            for (interfaces = 0, ifp = *list,
                sin = (SOCKADDR*) addr->lpSockaddr ;
                interfaces < addr_list->iAddressCount;
                interfaces++ , ifp++)
            {
                if (af == AF_INET)
                {
                    ifp->ip4_addr = ((struct sockaddr_in*)sin)->sin_addr;
                }
#ifdef PEGASUS_ENABLE_IPV6
                else
                {
                    ifp->ip6_addr = ((struct sockaddr_in6*)sin)->sin6_addr;
                }
#endif
                ifp->af = af;
                addr++;
                sin = (SOCKADDR*)addr->lpSockaddr;
            }

            ifp->af = AF_UNSPEC; // list terminate
        }
        else
        {
            errcode = WSAGetLastError();
        }
        _LSLP_CLOSESOCKET(sock);
    }

#else
    if (-1 < (sock = _LSLP_SOCKET(AF_INET, SOCK_DGRAM, 0)))
    {
        struct ifconf conf;
        struct slp_if_addr *this_addr;
        int interface_counter;

        conf.ifc_buf = (char *)calloc(128, sizeof(struct ifreq));
        conf.ifc_len = 128 * sizeof(struct ifreq) ;
        if (-1 < ioctl(sock, SIOCGIFCONF, &conf))
        {
            // count the interfaces
            struct ifreq *r = conf.ifc_req;
            SOCKADDR_IN *addr ;
            addr = (SOCKADDR_IN *)&r->ifr_addr;
            while (addr->sin_addr.s_addr != 0)
            {
                interfaces++;
                r++;
                addr = (SOCKADDR_IN *)&r->ifr_addr;
            }

            // now store the addresses
            interface_counter = interfaces;
            free(*list);
            *list  = (struct slp_if_addr *)
                calloc(interfaces + 2, sizeof(struct slp_if_addr));
            this_addr = *list;
            r = conf.ifc_req;
            addr = (SOCKADDR_IN *)&r->ifr_addr;
            while (interface_counter-- &&  addr->sin_addr.s_addr != 0)
            {
                this_addr->ip4_addr.s_addr = addr->sin_addr.s_addr;
                this_addr->af = AF_INET;
                r++;
                this_addr++;
                addr = (SOCKADDR_IN *)&r->ifr_addr;
            }
            this_addr->af = AF_UNSPEC; // list terminate
        } // did the ioctl
        free(conf.ifc_buf);
        _LSLP_CLOSESOCKET(sock);
    } // opened the socket

#endif
    DEBUG_PRINT((DEBUG_EXIT, "_slp_get_local_interfaces:ok "));
    return(interfaces);
}

void slp_get_local_interfaces(struct slp_client *client)
{
    _slp_get_local_interface(&client->_local_addr_list[0], AF_INET);
#ifdef PEGASUS_ENABLE_IPV6
    _slp_get_local_interface(&client->_local_addr_list[1], AF_INET6);
#endif
}

BOOL slp_join_multicast(SOCKETD sock, struct slp_if_addr addr)
{
#ifdef PEGASUS_ENABLE_IPV6
    struct ipv6_mreq group;
#endif
    struct ip_mreq mreq;
    DEBUG_PRINT((DEBUG_ENTER, "slp_join_multicast "));

    // don't join on the loopback interface
    if (addr.af == AF_INET)
    {
        if (!slp_is_ip4_stack_active() ||
            slp_is_loop_back(AF_INET, &addr.ip4_addr))
        {
            DEBUG_PRINT((DEBUG_EXIT, "slp_join_multicast ip4:err2 "));
            return(FALSE);
        }
    }
    else
    {
#ifdef PEGASUS_ENABLE_IPV6
        PEGASUS_ASSERT(addr.af == AF_INET6);
        if (!slp_is_ip6_stack_active() ||
            slp_is_loop_back(AF_INET6, &addr.ip6_addr))
        {
            DEBUG_PRINT((DEBUG_EXIT, "slp_join_multicast ip6:err2 "));
            return(FALSE);
        }
#endif
    }

    if (addr.af == AF_INET)
    {
        // Join ip4 multicast adress for SVRLOC , SVRLOC-DA and Service
        // location type.
        mreq.imr_multiaddr.s_addr = inet_addr("239.255.255.253");
        mreq.imr_interface.s_addr = addr.ip4_addr.s_addr;

        if (SOCKET_ERROR == setsockopt(sock,IPPROTO_IP, IP_ADD_MEMBERSHIP,
            (const char *)&mreq, sizeof(mreq)))
        {
            DEBUG_PRINT((DEBUG_EXIT, "slp_join_multicast ip4:err3 "));
            return(FALSE);
        }
    }
#ifdef PEGASUS_ENABLE_IPV6
    else
    {
        // Join multicast groups SVRLOC and SVRLOC-DA for ip6 interfaces.
        // Refer RFC 3111.
        group.ipv6mr_interface = 0; // default interface

        // SVRLOC Link Local
        slp_pton(AF_INET6, SLP_MC_LINK_SVRLOC, &group.ipv6mr_multiaddr);
        setsockopt(sock, IPPROTO_IPV6, IPV6_JOIN_GROUP,
                 (char *)&group, sizeof(group));

        // SVRLOC Site Local
        slp_pton(AF_INET6, SLP_MC_SITE_SVRLOC, &group.ipv6mr_multiaddr);
        setsockopt(sock, IPPROTO_IPV6, IPV6_JOIN_GROUP,
                 (char *)&group, sizeof(group));

        // SVRLOC-DA Link Local
        slp_pton(AF_INET6, SLP_MC_LINK_SVRLOC_DA, &group.ipv6mr_multiaddr);
        setsockopt(sock, IPPROTO_IPV6, IPV6_JOIN_GROUP,
                 (char *)&group, sizeof(group));

        // SVRLOC-DA Site Local
        slp_pton(AF_INET6, SLP_MC_SITE_SVRLOC_DA, &group.ipv6mr_multiaddr);
        setsockopt(sock, IPPROTO_IPV6, IPV6_JOIN_GROUP,
                 (char *)&group, sizeof(group));
    }
#endif

    DEBUG_PRINT((DEBUG_EXIT, "slp_join_multicast:ok "));
    return(TRUE);    //jeb
}

#ifdef PEGASUS_ENABLE_IPV6
void slp_join_ip6_service_type_multicast_group(struct slp_client *client,
    const char *srv_type)
{
    struct ipv6_mreq group;
    memset ( &group, 0, sizeof(group));
    unsigned long hash;
    char buff[PEGASUS_INET6_ADDRSTR_LEN];
    SOCKETD sock = client->_rcv_sock[1];

    if (!srv_type || sock == INVALID_SOCKET)
    {
        return;
    }
    // Refer RFC 3111 sec 4.1 for computing the SLP service location hash
    hash = 1000 + slp_hash(srv_type, (unsigned long)strlen(srv_type));

   // Join on Link local scope
    sprintf(buff,"FF02::1:%lu",hash);
    slp_pton(AF_INET6, buff, &group.ipv6mr_multiaddr);
    setsockopt(sock, IPPROTO_IPV6, IPV6_JOIN_GROUP,
        (char *)&group, sizeof(group));

    // Join on SITE Local scope
    sprintf(buff,"FF05::1:%lu",hash);
    slp_pton(AF_INET6, buff, &group.ipv6mr_multiaddr);
    setsockopt(sock, IPPROTO_IPV6, IPV6_JOIN_GROUP,
        (char *)&group, sizeof(group));

}
#endif

int slp_join_multicast_all(SOCKETD sock, int af)
{

    struct slp_if_addr *list = NULL , *lptr = NULL;
    int interface_counter;

    int num_interfaces = _slp_get_local_interface(&list, af);
    DEBUG_PRINT((DEBUG_ENTER, "slp_join_multicast_all "));
    interface_counter = num_interfaces;

    lptr = list;
    while (interface_counter-- && lptr->af != AF_UNSPEC)
    {
        slp_join_multicast(sock, *lptr);
        lptr++;
    }
    free(list);
    DEBUG_PRINT((DEBUG_EXIT, "slp_join_multicast_all "));
    return(num_interfaces);
}

/*
    Creates and binds the socket based on address family.
*/
static int _slp_create_bind_socket(SOCKETD *sock, int af, int port,
    void *addr, BOOL reuse)
{
#ifdef PEGASUS_ENABLE_IPV6
    SOCKADDR_IN6 ip6;
#endif
    SOCKADDR_IN ip4;
    int err = 1;
    int brc = 0;

    *sock  = _LSLP_SOCKET(af, SOCK_DGRAM, 0);
    DEBUG_PRINT((DEBUG_ENTER, "_slp_create_bind_socket"));

    if (*sock == INVALID_SOCKET)
    {
        DEBUG_PRINT((DEBUG_EXIT, "_slp_create_bind_socket:err1"));
        return -1;
    }
    if (reuse)
    {
        _LSLP_SETSOCKOPT(*sock, SOL_SOCKET, LSLP_REUSEADDR,
            (const char *)&err, sizeof(err));
    }

    if (af == AF_INET)
    {
        ip4.sin_family = af;
        ip4.sin_port = htons(port);
        ip4.sin_addr.s_addr  =
            addr ? ((struct in_addr*)addr)->s_addr : INADDR_ANY;
        brc =  _LSLP_BIND(*sock, (struct sockaddr *)&ip4, sizeof(ip4));
    }
#ifdef PEGASUS_ENABLE_IPV6
    else
    {
        PEGASUS_ASSERT(af == AF_INET6);

        // To be able to bind the IPV6 socket to in6addr_any in parallel to
        // the IPV4 socket, the IPV6_V6ONLY option has to be set.
#ifdef PEGASUS_OS_TYPE_WINDOWS
#ifdef PEGASUS_HAVE_WINDOWS_IPV6ONLY
        _LSLP_SETSOCKOPT(*sock, IPPROTO_IPV6, IPV6_V6ONLY,
            (const char *)&err, sizeof(err));
#endif
#else // PEGASUS_OS_TYPE_WINDOWS
        _LSLP_SETSOCKOPT(*sock, IPPROTO_IPV6, IPV6_V6ONLY,
            (const char *)&err, sizeof(err));
#endif

        memset (&ip6, 0, sizeof(ip6));
        ip6.sin6_family = af;
        ip6.sin6_port = htons(port);
        ip6.sin6_addr = addr ? *((struct in6_addr*)addr) : in6addr_any;
        _LSLP_BIND(*sock, (struct sockaddr *)&ip6, sizeof(ip6));
    }
#endif

    DEBUG_PRINT((DEBUG_EXIT, "_slp_create_bind_socket"));

    return brc == SOCKET_ERROR ? -1 : 0;
}

static SOCKETD _slp_open_listen_sock(int af, int port)
{
    SOCKETD sock = INVALID_SOCKET;

    DEBUG_PRINT((DEBUG_ENTER, "_slp_open_listen_sock %d",af));

    if (_slp_create_bind_socket(&sock, af, port, 0, TRUE) == 0)
    {
        slp_join_multicast_all(sock, af);
    }

    DEBUG_PRINT((DEBUG_EXIT, "_slp_open_listen_sock "));

    return(sock);
}

void slp_open_listen_socks(struct slp_client *client)
{
    int i;
    for (i = 0; i < 2; ++i)
    {
        if (client->_rcv_sock[i] != INVALID_SOCKET)
        {
            _LSLP_CLOSESOCKET(client->_rcv_sock[i]);
        }
    }

    if (client->_ip4_stack_active)
    {
        client->_rcv_sock[0] = _slp_open_listen_sock(
            AF_INET,
            client->_target_port);
    }

#ifdef PEGASUS_ENABLE_IPV6
    if (client->_ip6_stack_active)
    {
        client->_rcv_sock[1] = _slp_open_listen_sock(
            AF_INET6,
            client->_target_port);
    }
#endif
}

void prepare_pr_buf(struct slp_client *client, const char *address)
{
    if (address == NULL || client == NULL)
    {
        return;;
    }
    if (client->_pr_buf_len > LSLP_MTU)
    {
        printf("Memory allocation failed in file %s at Line number %d\n",
            __FILE__, __LINE__);
        exit(1);
    }

    if (client->_pr_buf_len > 0 && client->_pr_buf_len < (LSLP_MTU - 2))
    {
        client->_pr_buf[client->_pr_buf_len - 1] = ',';
    }

    do
    {
        client->_pr_buf[client->_pr_buf_len] = *address;
        address++;
        (client->_pr_buf_len)++;
    }while ((*address != 0x00) && (client->_pr_buf_len < LSLP_MTU - 1));

    (client->_pr_buf_len)++;
}

/** attn need to role change to getflags line into nucleus **/
void make_srv_ack(
    struct slp_client *client,
    SOCKADDR *remote,
    char response,
    int16 code)
{
    char *bptr;
    struct slp_if_addr local_address;
    size_t addr_len;
    int idx;

    if (TRUE == ( ((_LSLP_GETFLAGS( client->_rcv_buf )) &
        (LSLP_FLAGS_MCAST) ) ? FALSE : TRUE ))
    {
        SOCKETD sock;  //jeb

        memset(client->_msg_buf, 0x00, LSLP_MTU);
        bptr = client->_msg_buf;
        _LSLP_SETVERSION(bptr, LSLP_PROTO_VER);
        _LSLP_SETFUNCTION(bptr, response);

        _LSLP_SETFLAGS(bptr, 0);
        _LSLP_SETXID(bptr, _LSLP_GETXID(client->_rcv_buf));
        _LSLP_SETLAN(bptr, LSLP_EN_US, LSLP_EN_US_LEN);
        bptr += _LSLP_HDRLEN(bptr) ;
        _LSLP_SETSHORT(bptr, code, 0 );
        bptr += 2;
        _LSLP_SETLENGTH(client->_msg_buf, bptr - client->_msg_buf);

        if (remote->sa_family == AF_INET)
        {
            addr_len = sizeof(SOCKADDR_IN);
            idx = 0;
        }
#ifdef PEGASUS_ENABLE_IPV6
        else
        {
            addr_len = sizeof(SOCKADDR_IN6);
            idx = 1;
        }
#endif
        if (client->_local_addr_any)
        {
            local_address.af = remote->sa_family;
            if (local_address.af == AF_INET)
            {
                local_address.ip4_addr.s_addr = INADDR_ANY;
            }
#ifdef PEGASUS_ENABLE_IPV6
            else
            {
                local_address.ip6_addr = in6addr_any;
            }
#endif
        }
        else
        {
            local_address = client->_local_addr;
        }

#ifdef PEGASUS_OS_ZOS
        struct slp_if_addr* ptr_addr;
        ptr_addr = client->_local_addr_list[idx];
        while (ptr_addr->af != AF_UNSPEC)
        {
            local_address = *ptr_addr;
#endif
            if (-1 != _slp_create_bind_socket(
                &sock,
                local_address.af,
                client->_target_port,
                &local_address.ip4_addr,
                TRUE))
            {
                _LSLP_SENDTO(sock, client->_msg_buf,
                    _LSLP_GETLENGTH(client->_msg_buf), 0,
                    remote, addr_len);
            }
            _LSLP_CLOSESOCKET(sock);
#ifdef PEGASUS_OS_ZOS
            ptr_addr++;
        } // end of while loop around all local network interfaces
#endif
    }
}


BOOL prepare_query(
    struct slp_client *client,
    uint16 xid,
    const char *service_type,
    const char *scopes,
    const char *predicate  )
{
    int16 len, total_len, buf_len;
    char *bptr, *bptrSave;

    DEBUG_PRINT((DEBUG_ENTER, "prepare_query "));

    if (xid != client->_xid)
    {
        /* this is a new request */
        memset(client->_pr_buf, 0x00, LSLP_MTU);
        client->_pr_buf_len = 0;
        client->_xid = xid;
    }
    memset(client->_msg_buf, 0x00, LSLP_MTU);
    bptr = client->_msg_buf;
    _LSLP_SETVERSION(bptr, LSLP_PROTO_VER);
    _LSLP_SETFUNCTION(bptr, LSLP_SRVRQST);
    /* we don't know the length yet */
    _LSLP_SETFLAGS(bptr, 0);
    _LSLP_SETXID(bptr, xid);
    _LSLP_SETLAN(bptr, LSLP_EN_US, LSLP_EN_US_LEN);
    bptr += ( total_len = _LSLP_HDRLEN(bptr) ) ;

    if (client->_pr_buf_len + total_len < LSLP_MTU)
    {
        /* set the pr list length */
        _LSLP_SETSHORT(bptr, (len = client->_pr_buf_len), 0);
        if (len)
        {
            memcpy(bptr + 2, client->_pr_buf, len);
        }
        total_len += ( 2 + len );
        bptr += (2 + len);

        if (service_type == NULL)
        {
            len = DA_SRVTYPELEN;
        }
        else
        {
            len = (int16)strlen(service_type) ;
        }

        if (total_len + 2 + len < LSLP_MTU)
        {
            /* set the service type string length */
            _LSLP_SETSHORT(bptr, len, 0);

            if (service_type != NULL)
            {
                memcpy(bptr + 2, service_type, len);
            }
            else
            {
                memcpy(bptr + 2, DA_SRVTYPE, len);
            }

            total_len += (2 + len);
            bptr += (2 + len);

            /* set the scope len and scope type, advance the buffer */

            if (scopes == NULL)
            {
                len = DA_SCOPELEN;
            }
            else
            {
                len = (int16)strlen(scopes);
            }

            if (total_len + 2 + len < LSLP_MTU)
            {
                _LSLP_SETSHORT(bptr, len, 0);
                if (scopes != NULL)
                {
                    memcpy(bptr + 2, scopes, len);
                }
                else
                {
                    memcpy(bptr + 2, DA_SCOPE, DA_SCOPELEN);
                }

                total_len += ( 2 + len);
                bptr += (2 + len);

                /* stuff the predicate if there is one  */
                if (predicate == NULL)
                {
                    len = 0;
                }
                else
                {
                    len = (int16)strlen(predicate) ;
                }

                if (total_len + 2 + len < LSLP_MTU)
                {
                    _LSLP_SETSHORT(bptr, len, 0);
                    if (predicate != NULL)
                    {
                        memcpy(bptr + 2, predicate, len);
                    }
                    total_len += (2 + len);
                    bptrSave = (bptr += (2 + len));
                    /* <<< bug 1732 Thu Jun 10 08:51:03 2004 mdd >>> force the
                       spi string to be zero length*/
                    /* stuff the spi */
                    buf_len = LSLP_MTU - total_len;
                    _LSLP_SETSHORT(bptr, 0x0000, 0);
                    bptr += 2;
                    total_len += 2;


                    /*       lslpStuffSPIList(&bptr, &buf_len, client->_spi); */

                    /* read back the length of the spi */
                    /*       total_len += (2 + _LSLP_GETSHORT(bptrSave, 0)); */


                    /* <<< Thu Jun 10 08:51:03 2004 mdd >>>  force the spi
                       string to be zero length*/

                    PEGASUS_ASSERT(total_len == bptr - client->_msg_buf);
                    /*  always add an attr extension to an srvrq if there is
                        room */
                    if (total_len + 9 <= LSLP_MTU)
                    {
                        _LSLP_SETNEXTEXT(client->_msg_buf, total_len);
                        _LSLP_SETSHORT(bptr, 0x0002, 0);
                        _LSLP_SET3BYTES(bptr, 0x00000000, 2);
                        _LSLP_SETSHORT(bptr, 0x0000, 5);
                        _LSLP_SETSHORT(bptr, 0x0000, 7);
                        _LSLP_SETBYTE(bptr, 0x00, 9);
                        total_len += 10;
                    }
                    /* now go back and set the length for the entire message */
                    _LSLP_SETLENGTH(client->_msg_buf, total_len );
                    DEBUG_PRINT((DEBUG_EXIT, "prepare_query:ok "));
                    return(TRUE);

                } /*  room for predicate  */
            } /* room for the scope  */
        } /* room for the service type  */
    } /* room for the pr list  */
    DEBUG_PRINT((DEBUG_EXIT, "prepare_query:err "));
    return(FALSE);
}


// <<< Sat Jul 24 16:25:44 2004 mdd >>> attr request


/*       0                   1                   2                   3 */
/*       0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 */
/*      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ */
/*      |       Service Location header (function = AttrRqst = 6)       | */
/*      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ */
/*      |       length of PRList        |        <PRList> String        \ */
/*      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ */
/*      |         length of URL         |              URL              \ */
/*      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ */
/*      |    length of <scope-list>     |      <scope-list> string      \ */
/*      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ */
/*      |  length of <tag-list> string  |       <tag-list> string       \ */
/*      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ */
/*      |   length of <SLP SPI> string  |        <SLP SPI> string       \ */
/*      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ */

BOOL prepare_attr_query(
    struct slp_client *client,
    uint16 xid,
    const char *url,
    const char *scopes,
    const char *tags)
{
    int16 len, total_len;
    char *bptr;
    const char *scopeptr;
    static char default_scope[] = "DEFAULT";

    DEBUG_PRINT((DEBUG_ENTER, "prepare_attr_query "));

    if (url == NULL)
    {
        return FALSE;
    }
    /* first off, check to see if this is a retry or a new request */
    if (xid != client->_xid)
    {
        /* this is a new request, not a retry */
        /* reset the previous responder buffer */
        memset(client->_pr_buf, 0x00, LSLP_MTU);
        client->_pr_buf_len = 0;
        client->_xid = xid;
    }

    /* reset the client's send buffer and initialize the buffer pointer */
    memset(client->_msg_buf, 0x00, LSLP_MTU);
    bptr = client->_msg_buf;

    /* initialize the SLP Header */
    _LSLP_SETVERSION(bptr, LSLP_PROTO_VER);
    _LSLP_SETFUNCTION(bptr, LSLP_ATTRREQ);

    /* skip the length field until we know the actual length of the message */

    _LSLP_SETFLAGS(bptr, 0);
    _LSLP_SETXID(bptr, xid);
    _LSLP_SETLAN(bptr, LSLP_EN_US, LSLP_EN_US_LEN);

    /* adjust the buffer pointer forward and initialize our length counter */
    bptr += ( total_len = _LSLP_HDRLEN(bptr) );

    /* make sure the pr buffer will not cause an overflow and
       set the pr list length */
    if (client->_pr_buf_len + total_len < LSLP_MTU)
    {
        _LSLP_SETSHORT(bptr, (len = client->_pr_buf_len), 0 );
        /* if there is a pr list, copy it into the msg buffer */
        if (len)
        {
            memcpy(bptr + 2, client->_pr_buf, len);
        }
        /* adjust our counter and buffer pointer */
        total_len += ( 2 + len );
        bptr += ( 2 + len );

        /* if there is room, set the url length and copy the url */
        len = (int16) strlen(url);
        if (total_len + 2 + len < LSLP_MTU)
        {
            _LSLP_SETSHORT(bptr, len, 0 );
            if (len)
            {
                memcpy(bptr + 2, url, len);
            }
            /* adjust counter and buffer pointer */
            total_len += (2 + len);
            bptr += (2 + len);

            /* if there is room, set the scope list length and copy
               the scope list */
            if (scopes == NULL)
            {
                scopeptr = default_scope;
            }
            else
            {
                scopeptr = scopes;
            }

            len = (int16) strlen(scopeptr);

            if (total_len + 2 + len < LSLP_MTU)
            {
                _LSLP_SETSHORT(bptr, len, 0 );

                if (len)
                {
                    memcpy(bptr + 2, scopeptr, len);
                }
                total_len += (2 + len);
                bptr += (2 + len);

                /* if there is room, set the tag list length and copy
                   the tag list */
                if (tags != NULL)
                {
                    len = (int16) strlen(tags);
                }
                else
                {
                    len = 0;
                }

                if (total_len + 2 + len < LSLP_MTU)
                {
                    _LSLP_SETSHORT(bptr + 2, len, 0);
                    if (len)
                    {
                        memcpy(bptr + 2, tags, len);
                    }
                    total_len += ( 2 + len );
                    bptr += ( 2 + len );

                    /* leave the spi length at zero - spi will be deprecated
                       in next version of SLP */
                    /* length needs to account for the spi string length */
                    total_len += 2;

                    /* now go back and set the length for the entire message*/
                    _LSLP_SETLENGTH(client->_msg_buf, total_len );
                    DEBUG_PRINT((DEBUG_EXIT, "prepare_attr_query:ok "));
                    return(TRUE);

                } /* if tags list fits */
            } /* if scopes fit */
        } /* if the url fits */
    } /* if pr buffer fits */

    DEBUG_PRINT((DEBUG_EXIT, "prepare_attr_query:err "));
    return FALSE;
}


lslpMsg *get_response( struct slp_client *client, lslpMsg *head)
{
    PEGASUS_ASSERT(head != NULL && _LSLP_IS_HEAD(head));
    if (_LSLP_IS_EMPTY(&(client->replies)))
    {
        return NULL;
    }
    _LSLP_LINK_HEAD(head, &(client->replies));
    // make sure both head and replies point to same thing
    client->replies = *head;
    return head;
}

/*
    Sets the local interface address in the client which is used to bind the
    local socket while sending the UDP datagrams.
*/
static void _slp_set_local_addr(struct slp_client *client, int af)
{
    if (client->_local_addr_any)
    {
        client->_local_addr.af = af;
        if (af == AF_INET)
        {
            client->_local_addr.ip4_addr.s_addr = INADDR_ANY;
        }
#ifdef PEGASUS_ENABLE_IPV6
        else
        {
            client->_local_addr.ip6_addr = in6addr_any;
        }
#endif
    }
}

/*
    Checks where we can converge/local-try the request by taking the address
    family and target address.
*/
static BOOL _slp_can_make_request(struct slp_client *client, int af,
    const char *addr)
{
    // Check if we have direct target_addr
    if (af == AF_UNSPEC)
    {
        if (client->_target_addr_any)
        {
            return FALSE;
        }
        af = client->_target_addr.af;
        _slp_set_local_addr(client, af);
    }
    else
    {
        if (!client->_target_addr_any || (!client->_local_addr_any &&
            client->_local_addr.af != af))
        {
            return FALSE;
        }
        _slp_set_local_addr(client, af);
        client->_target_addr.af = af;
        slp_pton(af, addr, &client->_target_addr.ip4_addr);
    }

    // Make sure that stack is active for the address family.
    if ((af == AF_INET && !client->_ip4_stack_active)
#ifdef PEGASUS_ENABLE_IPV6
        || (af == AF_INET6 && !client->_ip6_stack_active)
#endif
        )
    {
        return FALSE;
    }

    return TRUE;
}

int find_das(
    struct slp_client *client,
    const char *predicate,
    const char *scopes)
{
    converge_srv_req(client, NULL, predicate, scopes);
    time(&(client->_last_da_cycle));
    if (! _LSLP_IS_EMPTY(&(client->das)))
    {
        client->_use_das = TRUE;
    }
    else
    {
        client->_use_das = FALSE;
    }
    return((int) client->_use_das );
}


/* this request MUST be retried <_convergence> times on EACH interface */
/* regardless of how many responses we have received  */
/* it can be VERY time consuming but is the most thorough  */
/* discovery method  */
static void _slp_converge_srv_req(
    struct slp_client *client,
    const char *type,
    const char *predicate,
    const char *scopes,
    int xid)
{
    struct slp_if_addr *p_addr, local_addr_save;
    uint16 convergence;
    int idx;
    int af = client->_target_addr.af;

    DEBUG_PRINT((DEBUG_ENTER, "converge_srv_req "));

    local_addr_save = client->_local_addr;

    idx = af == AF_INET ? 0 : 1;
    p_addr = client->_local_addr_list[idx];
    PEGASUS_ASSERT(p_addr);

    do
    {
        // Don't converge on loop-back address, we shall try local requaet
        // later.
        if (slp_is_loop_back(af, &p_addr->ip4_addr))
        {
            p_addr++;
            continue;
        }
        client->_local_addr = *p_addr;
        convergence = client->_convergence;

        if (convergence == 0)
        {
            convergence = 1;
        }

        if (prepare_query( client, client->_xid + xid, type, scopes, predicate))
        {
            _LSLP_SETFLAGS(client->_msg_buf, LSLP_FLAGS_MCAST);
            send_rcv_udp( client );
        }

        while (--convergence > 0)
        {
            if (prepare_query( client, client->_xid, type, scopes, predicate))
            {
                _LSLP_SETFLAGS(client->_msg_buf, LSLP_FLAGS_MCAST);
                send_rcv_udp( client );
            }
        }
        p_addr++;
    }while (p_addr->af != AF_UNSPEC);


    client->_local_addr = local_addr_save;

    DEBUG_PRINT((DEBUG_EXIT, "converge_srv_req "));
    return ;
}

void converge_srv_req(
    struct slp_client *client,
    const char *type,
    const char *predicate,
    const char *scopes)
{
    int xid = 1;
#ifdef PEGASUS_ENABLE_IPV6
    unsigned long hash;
    char addr[PEGASUS_INET6_ADDRSTR_LEN];
#endif
    if (_slp_can_make_request(client, AF_UNSPEC, 0))
    {
         _slp_converge_srv_req(client, type, predicate, scopes, xid);
    }
    else
    {
        // Try with ip4 first
        if (_slp_can_make_request(client, AF_INET, "239.255.255.253"))
        {
            _slp_converge_srv_req(client, type, predicate, scopes, xid);
            xid = 0;
        }
#ifdef PEGASUS_ENABLE_IPV6
        if (type)
        {
            hash = 1000 + slp_hash(type, (unsigned long)strlen(type));
            // Try on Link local scope
            sprintf(addr,"FF02::1:%lu",hash);
            if (_slp_can_make_request(client, AF_INET6, addr))
            {
                _slp_converge_srv_req(client, type, predicate, scopes, xid);
                xid = 0;
            }
            // Try on Site local scope
            sprintf(addr,"FF05::1:%lu",hash);
            if (_slp_can_make_request(client, AF_INET6, addr))
            {
                _slp_converge_srv_req(client, type, predicate, scopes, xid);
            }
        }
        else
        {
            if (_slp_can_make_request(client, AF_INET6, SLP_MC_LINK_SVRLOC_DA))
            {
                _slp_converge_srv_req(client, type, predicate, scopes, xid);
                xid = 0;
            }
            if (_slp_can_make_request(client, AF_INET6, SLP_MC_SITE_SVRLOC_DA))
            {
                _slp_converge_srv_req(client, type, predicate, scopes, xid);
            }
        }
#endif
    }

    // always try a local request
    local_srv_req(client, type, predicate, scopes);
}

/*
    Sets local and target addresses of slp_client from remote address.
*/
static void _slp_fill_local_target_addr(
    struct slp_client *client,
    SOCKADDR *addr,
    uint16 port)
{
    if (addr->sa_family == AF_INET)
    {
        client->_local_addr.ip4_addr.s_addr = INADDR_ANY;
        client->_local_addr.af = AF_INET;
        if (addr)
        {
            client->_target_addr.ip4_addr = ((SOCKADDR_IN*)addr)->sin_addr;
        }
        else
        {
          client->_target_addr.ip4_addr.s_addr = inet_addr("127.0.0.1");
        }
        client->_target_addr.af = AF_INET;
        client->_target_port =
            addr ? ntohs(((SOCKADDR_IN*)addr)->sin_port) : port;
    }
#ifdef PEGASUS_ENABLE_IPV6
    else
    {
        client->_local_addr.ip6_addr = in6addr_any;
        client->_local_addr.af = AF_INET6;
        if (addr)
        {
            client->_target_addr.ip6_addr =
                ((SOCKADDR_IN6*)addr)->sin6_addr;
        }
        else
        {
            slp_pton(AF_INET6, "::1",
                &client->_target_addr.ip6_addr);
        }
        client->_target_addr.af = AF_INET6;
        client->_target_port =
            addr ? ntohs(((SOCKADDR_IN6*)addr)->sin6_port) : port;
    }
#endif
}

// this request will be retried MAX <_retries> times
// but will always end when the first response is received
// This request is best when using a directory agent
void unicast_srv_req(
    struct slp_client *client,
    const char *type,
    const char *predicate,
    const char *scopes,
    SOCKADDR *addr)
{
    struct slp_if_addr target_addr_save, local_addr_save;
    uint16 target_port_save;
    struct timeval tv_save;
    int retries ;

    DEBUG_PRINT((DEBUG_ENTER, "unicast_srv_req "));

    target_addr_save = client->_target_addr;
    local_addr_save = client->_local_addr;
    target_port_save = client->_target_port;

    tv_save.tv_sec = client->_tv.tv_sec;
    client->_tv.tv_sec = 1;

    _slp_fill_local_target_addr(client, addr, 0);

    retries = client->_retries;

    srv_req(client, type, predicate, scopes, FALSE);

    while (retries && _LSLP_IS_EMPTY(&(client->replies)))
    {
        srv_req(client, type, predicate, scopes, FALSE);
        retries--;
    }

    client->_target_addr = target_addr_save;
    client->_local_addr = local_addr_save;
    client->_target_port = target_port_save;
    client->_tv.tv_sec = tv_save.tv_sec;

    DEBUG_PRINT((DEBUG_EXIT, "unicast_srv_req "));
    return;
}

// this request is targeted to the loopback interface,
// and has a tiny wait timer. It should be resolved quickly.
// It will never be retried.
void local_srv_req(
    struct slp_client *client,
    const char *type,
    const char *predicate,
    const char *scopes)
{
    struct slp_if_addr target_addr_save, local_addr_save;
    uint16 target_port_save;
    struct timeval tv_save;
    BOOL local_any_save, target_any_save;

    DEBUG_PRINT((DEBUG_ENTER, "local_srv_req "));

    target_addr_save = client->_target_addr;
    target_port_save = client->_target_port;
    local_addr_save = client->_local_addr;
    local_any_save = client->_local_addr_any;
    target_any_save = client->_target_addr_any;

    tv_save.tv_sec = client->_tv.tv_sec;
    tv_save.tv_usec = client->_tv.tv_usec;
    client->_tv.tv_sec = 0;
    client->_tv.tv_usec = 10000;

    client->_local_addr_any = client->_target_addr_any = TRUE;

    // Try first on ip4 interface. This should get all ip4 URLs.
    if (_slp_can_make_request(client, AF_INET, "127.0.0.1"))
    {
        srv_req(client, type, predicate, scopes, TRUE);
    }

    // Try on ip6 interface. This should get all ip6 URLs.
#ifdef PEGASUS_ENABLE_IPV6
    if (_slp_can_make_request(client, AF_INET6, "::1"))
    {
        srv_req(client, type, predicate, scopes, TRUE);
    }
#endif

    client->_target_addr = target_addr_save;
    client->_target_port = target_port_save;
    client->_local_addr = local_addr_save;
    client->_local_addr_any = local_any_save;
    client->_target_addr_any = target_any_save;

    client->_tv.tv_sec = tv_save.tv_sec;
    client->_tv.tv_usec = tv_save.tv_usec;

    DEBUG_PRINT((DEBUG_EXIT, "local_srv_req "));
    return;
}

// workhorse request function
void srv_req(
    struct slp_client *client,
    const char *type,
    const char *predicate,
    const char *scopes,
    BOOL retry )
{
    DEBUG_PRINT((DEBUG_ENTER, "srv_req "));
    if ((TRUE == prepare_query(client,
        (retry == TRUE) ? client->_xid : client->_xid + 1,
        type,
        scopes,
        predicate)))
    {
        PEGASUS_ASSERT(client->_target_addr.af != AF_UNSPEC);

        if ( (client->_target_addr.af == AF_INET &&
           (client->_target_addr.ip4_addr.s_addr == _LSLP_MCAST ||
           client->_target_addr.ip4_addr.s_addr == _LSLP_LOCAL_BCAST))
#ifdef PEGASUS_ENABLE_IPV6
            || (client->_target_addr.af == AF_INET6 &&
               IN6_IS_ADDR_MULTICAST(&client->_target_addr.ip6_addr))
#endif
            )
        {
            _LSLP_SETFLAGS(client->_msg_buf, LSLP_FLAGS_MCAST);
        }
        send_rcv_udp( client ) ;
    } /* prepared query  */
    DEBUG_PRINT((DEBUG_EXIT, "srv_req "));
    return ;
}

/* this request MUST be retried <_convergence> times on EACH interface  */
/* regardless of how many responses we have received  */
/* it can be VERY time consuming but is the most thorough  */
/* discovery method  */
static void _slp_converge_attr_req(
    struct slp_client *client,
    const char *url,
    const char *scopes,
    const char *tags,
    int xid)
{

    struct slp_if_addr  *p_addr;
    struct slp_if_addr old_target_addr, old_local_addr;
    uint16 convergence;
    int idx;
    int af = client->_target_addr.af;

    DEBUG_PRINT((DEBUG_ENTER, "converge_attr_req "));
    PEGASUS_ASSERT(af != AF_UNSPEC);

    idx = af == AF_INET ? 0 : 1;

    old_target_addr = client->_target_addr;
    old_local_addr = client->_local_addr;

    p_addr = client->_local_addr_list[idx];
    do
    {
        if (slp_is_loop_back(af, &p_addr->ip4_addr))
        {
            p_addr++;
            continue;
        }
        client->_local_addr = *p_addr;
        convergence = client->_convergence;

        if (convergence == 0)
        {
            convergence = 1;
        }

        if (prepare_attr_query( client, client->_xid + xid, url, scopes, tags))
        {
            _LSLP_SETFLAGS(client->_msg_buf, LSLP_FLAGS_MCAST) ;
            send_rcv_udp( client );
        }

        while (--convergence > 0)
        {
            if (prepare_attr_query( client, client->_xid, url, scopes, tags))
            {
                _LSLP_SETFLAGS(client->_msg_buf, LSLP_FLAGS_MCAST) ;
                send_rcv_udp( client );
            }
        }
        p_addr++;
    }while (p_addr->af != AF_UNSPEC);



    client->_target_addr = old_target_addr;
    client->_local_addr = old_local_addr;

    DEBUG_PRINT((DEBUG_EXIT, "converge_attr_req "));
    return ;

}


void converge_attr_req(
    struct slp_client *client,
    const char *url,
    const char *scopes,
    const char *tags)
{
    int xid = 1;

    if (_slp_can_make_request(client, AF_UNSPEC, 0))
    {
         _slp_converge_attr_req(client, url, scopes, tags, xid);
    }
    else
    {
        // Try with ip4 first
        if (_slp_can_make_request(client, AF_INET, "239.255.255.253") &&
            _slp_check_url_addr(url, AF_INET, 0))
        {
             _slp_converge_attr_req(client, url, scopes, tags, xid);
             xid = 0;
        }
#ifdef PEGASUS_ENABLE_IPV6
        if (_slp_can_make_request(client, AF_INET6, SLP_MC_LINK_SVRLOC) &&
            _slp_check_url_addr(url, AF_INET6, 0))
        {
             _slp_converge_attr_req(client, url, scopes, tags, xid);
             xid = 0;
        }
        if (_slp_can_make_request(client, AF_INET6, SLP_MC_SITE_SVRLOC) &&
            _slp_check_url_addr(url, AF_INET6, 0))
        {
             _slp_converge_attr_req(client, url, scopes, tags, xid);
        }
#endif
    }
    // always try a local request
    local_attr_req(client, url, scopes, tags);
}

// this request will be retried MAX <_retries> times
// but will always end when the first response is received
// This request is best when using a directory agent
void unicast_attr_req(
    struct slp_client *client,
    const char *url,
    const char *scopes,
    const char *tags,
    SOCKADDR *addr)
{

    struct slp_if_addr target_addr_save, local_addr_save;
    uint16 target_port_save;
    struct timeval tv_save;
    int retries ;

    DEBUG_PRINT((DEBUG_ENTER, "unicast_attr_req "));

    target_addr_save = client->_target_addr;
    local_addr_save = client->_local_addr;
    target_port_save = client->_target_port;

    tv_save.tv_sec = client->_tv.tv_sec;
    client->_tv.tv_sec = 1;

    _slp_fill_local_target_addr(client, addr, 0);

    retries = client->_retries;

    attr_req(client, url, scopes, tags, FALSE) ;

    while (retries && _LSLP_IS_EMPTY(&(client->replies)))
    {
        attr_req(client, url, scopes, tags, FALSE);
        retries--;
    }
    client->_target_addr = target_addr_save;
    client->_local_addr = local_addr_save;
    client->_target_port = target_port_save;
    client->_tv.tv_sec = tv_save.tv_sec;

    DEBUG_PRINT((DEBUG_EXIT, "unicast_attr_req "));
    return;
}



// targeted to the loopback interface
void local_attr_req(
    struct slp_client *client,
    const char *url,
    const char *scopes,
    const char *tags )
{
    struct slp_if_addr target_addr_save, local_addr_save;
    uint16 target_port_save;
    BOOL local_any_save, target_any_save;

    struct timeval tv_save;

    DEBUG_PRINT((DEBUG_ENTER, "local_attr_req "));

    target_addr_save = client->_target_addr;
    target_port_save = client->_target_port;
    local_addr_save = client->_local_addr;
    local_any_save = client->_local_addr_any;
    target_any_save = client->_target_addr_any;

    tv_save.tv_sec = client->_tv.tv_sec;
    tv_save.tv_usec = client->_tv.tv_usec;
    client->_tv.tv_sec = 0;
    client->_tv.tv_usec = 10000;

    client->_local_addr_any = client->_target_addr_any = TRUE;

    if (_slp_can_make_request(client, AF_INET, "127.0.0.1") &&
        _slp_check_url_addr(url, AF_INET, 0))
    {
        attr_req(client, url, scopes, tags, TRUE) ;
    }
#ifdef PEGASUS_ENABLE_IPV6
    else if (_slp_can_make_request(client, AF_INET6, "::1") &&
        _slp_check_url_addr(url, AF_INET6, 0))
    {
        attr_req(client, url, scopes, tags, TRUE) ;
    }
#endif


    client->_target_addr = target_addr_save;
    client->_target_port = target_port_save;
    client->_local_addr = local_addr_save;
    client->_local_addr_any = local_any_save;
    client->_target_addr_any = target_any_save;

    client->_tv.tv_sec = tv_save.tv_sec;
    client->_tv.tv_usec = tv_save.tv_usec;

    DEBUG_PRINT((DEBUG_EXIT, "local_attr_req "));
    return;

}



// <<< Sat Jul 24 16:22:01 2004 mdd >>> attr request
// workhorse attr request function
void attr_req(
    struct slp_client *client,
    const char *url,
    const char *scopes,
    const char *tags,
    BOOL retry)
{

    DEBUG_PRINT((DEBUG_ENTER, "attr_req "));
    if (TRUE == prepare_attr_query(
        client,
        (retry == TRUE)? client->_xid : client->_xid+1,
        url,
        scopes,
        tags))
    {
        // check for the multicast address and set the mcast flag if necessary

        if ( (client->_target_addr.af == AF_INET &&
           (client->_target_addr.ip4_addr.s_addr == _LSLP_MCAST ||
           client->_target_addr.ip4_addr.s_addr == _LSLP_LOCAL_BCAST))
#ifdef PEGASUS_ENABLE_IPV6
            || (client->_target_addr.af == AF_INET6 &&
               IN6_IS_ADDR_MULTICAST(&client->_target_addr.ip6_addr))
#endif
            )
        {
            _LSLP_SETFLAGS(client->_msg_buf, LSLP_FLAGS_MCAST) ;
        }

        send_rcv_udp( client );
    }

    DEBUG_PRINT((DEBUG_EXIT, "attr_req "));
    return;
}



/*       0                   1                   2                   3 */
/*       0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 */
/*      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ */
/*      |       Service Location header (function = AttrRply = 7)       | */
/*      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ */
/*      |         Error Code            |      length of <attr-list>    | */
/*      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ */
/*      |                         <attr-list>                           \ */
/*      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ */
/*      |# of AttrAuths |  Attribute Authentication Block (if present)  \ */
/*      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ */

void decode_attr_rply(struct slp_client *client)
{

    char *bptr;
    lslpMsg *reply;

    int32 total_len, purported_len;

    DEBUG_PRINT((DEBUG_ENTER, "decode_attr_rply "));

    bptr = client->_rcv_buf;
    purported_len = _LSLP_GETLENGTH(bptr);

    /* marshall the header data */
    reply = alloc_slp_msg(FALSE);
    if (reply == NULL)
    {
        printf("Memory allocation failed in file %s at Line number %d\n",
            __FILE__, __LINE__);
        exit(1);
    }
    reply->hdr.ver = _LSLP_GETVERSION(bptr);
    reply->type = (msgTypes) (reply->hdr.msgid = _LSLP_GETFUNCTION(bptr));
    reply->hdr.len = purported_len;
    reply->hdr.flags = _LSLP_GETFLAGS(bptr);
    reply->hdr.nextExt = _LSLP_GETNEXTEXT(bptr);
    reply->hdr.xid = _LSLP_GETXID(bptr);


    reply->hdr.langLen = _LSLP_GETLANLEN(bptr);
    memcpy(reply->hdr.lang, bptr + LSLP_LAN_LEN + 2,
        (_LSLP_MIN(reply->hdr.langLen, 19)));
    bptr += (total_len = _LSLP_HDRLEN(bptr));
    if (total_len < purported_len)
    {

        /* process the attr rply */
        reply->hdr.errCode = reply->msg.attrRep.errCode =
            _LSLP_GETSHORT(bptr, 0);
        reply->msg.attrRep.attrListLen = _LSLP_GETSHORT(bptr, 2);
        if (reply->msg.attrRep.attrListLen + total_len < purported_len)
        {

            total_len += 4;
            bptr += 4;
            reply->msg.attrRep.attrList =
                (char *)calloc(1, reply->msg.attrRep.attrListLen + 1);
            if (reply->msg.attrRep.attrList == NULL)
            {
                printf("Memory allocation failed in file %s at Line number"
                    " %d\n", __FILE__, __LINE__);
                free(reply);
                exit(1);
            }
            memcpy(reply->msg.attrRep.attrList,
                bptr, reply->msg.attrRep.attrListLen);

            /* ignore auth blocks, they will be deprecated in next
               version of protocol */
        } /* attr list len checks out */
    } /* hdr len checks out */
    /* link the response to the client */
    _LSLP_INSERT(reply, &(client->replies));

    DEBUG_PRINT((DEBUG_EXIT, "decode_attr_rply "));
}


/* failsafe against malformed messages. */
/* if it sees a message it does not like, it simply returns */
/* bad message is silently discarded. */


void message_sanity_check(struct slp_client *client)
{
    DEBUG_PRINT((DEBUG_ENTER, "message_sanity_check "));
    if (_LSLP_GETLENGTH(client->_rcv_buf) > LSLP_MTU)
    {
        DEBUG_PRINT((DEBUG_EXIT, "message_sanity_check:err "));
        return;
    }

    if (_LSLP_GETVERSION(client->_rcv_buf) != LSLP_PROTO_VER)
    {
        DEBUG_PRINT((DEBUG_EXIT, "message_sanity_check:err2 "));
        return;
    }

    if (_LSLP_GETFUNCTION(client->_rcv_buf) < LSLP_SRVRQST ||
        _LSLP_GETFUNCTION(client->_rcv_buf) > LSLP_SAADVERT)
    {
        DEBUG_PRINT((DEBUG_EXIT, "message_sanity_check:err3 "));
        return;
    }

    DEBUG_PRINT((DEBUG_EXIT, "message_sanity_check:ok "));
    return;

}

/*
    Returns ip addr in text form that is present in remote addr.
*/
static char *_slp_get_text_ip(SOCKADDR *remote)
{
    static char buff[PEGASUS_INET6_ADDRSTR_LEN];
    int af = remote->sa_family;
    void *addr = 0;

    if (af == AF_INET)
    {
        addr = &((SOCKADDR_IN*)remote)->sin_addr;
    }
#ifdef PEGASUS_ENABLE_IPV6
    else
    {
        addr = &((SOCKADDR_IN6*)remote)->sin6_addr;
    }
#endif
    slp_ntop(af, addr, buff, sizeof(buff));

    return buff;
}

void decode_msg(struct slp_client *client, SOCKADDR *remote)
{
    char response;

    DEBUG_PRINT((DEBUG_ENTER, "decode_msg "));
    message_sanity_check(client);

    char function = _LSLP_GETFUNCTION( client->_rcv_buf );
    if (client->_xid == _LSLP_GETXID( client->_rcv_buf))
    {
        if (function == LSLP_SRVRPLY ||
            function == LSLP_ATTRRPLY ||
            function == LSLP_SRVTYPERPLY)
        {
            prepare_pr_buf(client, _slp_get_text_ip(remote));
        }
    }

    // <<< Fri Dec 21 15:47:06 2001 mdd >>>
    // increment the correct function counters
    switch (function)
    {
        case LSLP_DAADVERT:
            decode_daadvert( client, remote );
            DEBUG_PRINT((DEBUG_EXIT, "decode_msg %s", "LSLP_DAADVERT "));
            return;

        case LSLP_SRVRQST:
            decode_srvreq( client, remote );
            DEBUG_PRINT((DEBUG_EXIT, "decode_msg %s", "LSLP_SRVRQST "));
            return;

        case LSLP_SRVRPLY:
            decode_srvrply( client );
            DEBUG_PRINT((DEBUG_EXIT, "decode_msg %s", "LSLP_SRVRPLY "));
            return;

        case LSLP_SRVACK:
            DEBUG_PRINT((DEBUG_EXIT, "decode_msg %s", "LSLP_SRVACK "));
            return;

        case LSLP_ATTRREQ:
            decode_attrreq( client, remote );
            DEBUG_PRINT((DEBUG_EXIT, "decode_msg %s", "LSLP_ATTRREQ "));
            return;

        case LSLP_ATTRRPLY:
            decode_attr_rply( client );
            DEBUG_PRINT((DEBUG_EXIT, "decode_msg %s", "LSLP_ATTRRPLY "));
            return;

        case LSLP_SRVTYPERQST:
            response = LSLP_SRVTYPERPLY;
            break;

        case LSLP_SRVREG:
            decode_srvreg(client, remote);
            DEBUG_PRINT((DEBUG_EXIT, "decode_msg %s", "LSLP_SRVREG "));
            return;

        case LSLP_SRVDEREG:

        default:
            response = LSLP_SRVACK;
            break;
    }
    make_srv_ack(client, remote, response, LSLP_MSG_NOT_SUPPORTED);

    DEBUG_PRINT((DEBUG_EXIT, "decode_msg "));
    return;
}



/* this is a hack. but it will be consistent with the changes I envision */
/* for auth blocks and authenticated url entries */
void decode_srvreg(struct slp_client *client, SOCKADDR *remote)
{
    char *bptr, *url_string, *attr_string, *type_string, *scopes;
    uint16 lifetime ;
    int32 total_len, purported_len;
    BOOL mcast;
    int16 str_len;

    DEBUG_PRINT((DEBUG_ENTER, "decode_srvreg "));

    mcast = ( ((_LSLP_GETFLAGS( client->_rcv_buf )) &
        (LSLP_FLAGS_MCAST) ) ? TRUE : FALSE ) ;
    bptr = client->_rcv_buf;
    purported_len = _LSLP_GETLENGTH(bptr);

    bptr += (total_len = _LSLP_HDRLEN(bptr));
    if (purported_len < LSLP_MTU && (total_len < purported_len))
    {
        lslpURL *decoded_url;
        int16 diff, err;
        diff = purported_len - total_len;

        /* decode the url entry */
        if (NULL != (decoded_url = lslpUnstuffURL(&bptr, &diff, &err)))
        {
            url_string = decoded_url->url;
            lifetime = (uint16)decoded_url->lifetime;

            /* adjust pointers and size variables */
            /* bptr already adjusted by call to lslpUnstuffURL */
            total_len += ((purported_len - total_len) - diff);

            /* decode the service type string */
            if ((total_len + 2 + (str_len = _LSLP_GETSHORT(bptr, 0)))
                < purported_len)
            {
                if (NULL != (type_string = (char *)malloc(str_len + 1)))
                {
                    memcpy(type_string, bptr + 2, str_len);
                    *(type_string + str_len) = 0x00;

                    /* adjust pointers and size variables */
                    bptr += (2 + str_len);
                    total_len += (2 + str_len);

                    /* decode the scope list */
                    if ((total_len + 2 + (str_len = _LSLP_GETSHORT(bptr, 0)))
                        < purported_len)
                    {
                        if (NULL != (scopes = (char *)malloc(str_len + 1)))
                        {
                            memcpy(scopes, bptr + 2, str_len);
                            *(scopes + str_len) = 0x00;

                            /* adjust pointers and size variables */
                            bptr += (2 + str_len);
                            total_len += (2 + str_len);

                            /* decode the attr list */
                            if ((total_len + 2 + (str_len =
                                _LSLP_GETSHORT(bptr, 0))) < purported_len)
                            {
                                if (NULL != (attr_string =
                                    (char *)malloc(str_len + 1)))
                                {
                                    memcpy(attr_string, bptr + 2, str_len);
                                    *(attr_string + str_len) = 0x00;

                                    /* adjust pointers and size variables */
                                    bptr += (2 + str_len);
                                    total_len += (2 + str_len);

                                    /* decode the auth block list */
                                    /* lslpUnstuffAuthList( ) */
                                    __srv_reg_local(
                                        client,
                                        url_string,
                                        attr_string,
                                        type_string,
                                        scopes,
                                        lifetime);
                                    make_srv_ack(
                                        client,
                                        remote,
                                        LSLP_SRVACK,
                                        0);
                                    free(attr_string);
                                    free(scopes);
                                    free(type_string);
                                    lslpFreeURL(decoded_url);
                                    DEBUG_PRINT((
                                        DEBUG_EXIT,
                                        "decode_srvreg:1 "));
                                    return;
                                } /* malloced attr_string */
                            } /* attr string sanity check */
                            free(scopes);
                        } /* malloced scopes string */
                    } /* scope list sanity check */
                    free(type_string);
                } /* malloced srv type string */
            } /* srv type sanity check */
            lslpFreeURL(decoded_url);
            make_srv_ack(client, remote, LSLP_SRVACK, LSLP_INTERNAL_ERROR);
            DEBUG_PRINT((DEBUG_EXIT, "decode_srvreg:2 "));
            return;
        } /* decoded the url entry */
    } /* initial length sanity check OK */
    make_srv_ack(client, remote, LSLP_SRVACK, LSLP_PARSE_ERROR);

    DEBUG_PRINT((DEBUG_EXIT, "decode_srvreg:3 "));
    return;
}


void decode_srvrply(struct slp_client *client)
{
    char *bptr, *xtptr, *xtn_limit;
    lslpMsg *reply;

    int16 err, count, buf_len;
    int32 total_len, purported_len;

    DEBUG_PRINT((DEBUG_ENTER, "decode_srvrply "));

    xtn_limit = 0;
    bptr = client->_rcv_buf;
    purported_len = _LSLP_GETLENGTH(bptr);

    /* marshall the header data */
    reply = alloc_slp_msg(FALSE);
    if (reply == NULL)
    {
        printf("Memory allocation failed in file %s at Line number %d\n",
            __FILE__, __LINE__);
        exit(1);
    }
    reply->hdr.ver = _LSLP_GETVERSION(bptr);
    reply->hdr.msgid = _LSLP_GETFUNCTION(bptr);
    reply->type = (msgTypes) reply->hdr.msgid;
    reply->hdr.len = purported_len;
    reply->hdr.flags = _LSLP_GETFLAGS(bptr);
    reply->hdr.nextExt = _LSLP_GETNEXTEXT(bptr);
    reply->hdr.xid = _LSLP_GETXID(bptr);

    if ((reply->hdr.nextExt != 0x00000000) &&
        (reply->hdr.nextExt < reply->hdr.len))
    {
        xtptr = client->_rcv_buf + reply->hdr.nextExt;
        xtn_limit = xtptr + reply->hdr.len;
    }
    else
    {
        xtptr = NULL;
    }

    reply->hdr.langLen = _LSLP_GETLANLEN(bptr);
    memcpy(reply->hdr.lang, bptr + LSLP_LAN_LEN + 2,
        (_LSLP_MIN(reply->hdr.langLen, 19)));
    bptr += (total_len = _LSLP_HDRLEN(bptr));

    if (total_len < purported_len)
    {
        int32 next_ext = 0;

        /* process the srvrply */
        reply->hdr.errCode = reply->msg.srvRply.errCode =
            _LSLP_GETSHORT(bptr, 0);
        reply->msg.srvRply.urlCount = (count = _LSLP_GETSHORT(bptr, 2));
        bptr += 4;
        total_len += 4;
        if (count)
        {
            lslpURL *url;
            if (NULL == (reply->msg.srvRply.urlList = lslpAllocURLList()))
            {
                printf("Memory allocation failed in file %s at Line "
                    "number %d\n", __FILE__, __LINE__);
                free(reply);
                exit(1);
            }
            if (NULL != xtptr)
            {
                if (NULL == (reply->msg.srvRply.attr_list =
                    lslpAllocAtomList()))
                {
                    printf("Memory allocation failed in file %s at Line "
                        "number %d\n", __FILE__, __LINE__);
                    free(reply);
                    exit(1);
                }
            }

            buf_len = LSLP_MTU - total_len;
            /* get the urls */
            while (count-- && buf_len > 0)
            {
                url = lslpUnstuffURL(&bptr, &buf_len, &err);
                if (NULL != url)
                {
                    reply->msg.srvRply.urlLen = url->len;
                    _LSLP_INSERT(url, reply->msg.srvRply.urlList);
                }
            }

            /* get the attributes if they are present */

/*        0                   1                   2                   3 */
/*        0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 */
/*       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ */
/*       |      Extension ID = 0x0002    |     Next Extension Offset     | */
/*       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ */
/*       | Offset, contd.|      Service URL Length       |  Service URL  / */
/*       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ */
/*       |     Attribute List Length     |         Attribute List        / */
/*       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ */
/*       |# of AttrAuths |(if present) Attribute Authentication Blocks.../ */
/*       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ */

            while ((NULL != xtptr) && (xtptr + 9 < xtn_limit))
            {
                next_ext = _LSLP_GET3BYTES(xtptr, 2);
                if (0x0002 == _LSLP_GETSHORT(xtptr, 0))
                {

                    // BOOL lslp_pattern_match(const char *string,
                    //  const char *pattern, BOOL case_sensitive)
                    /* find the correct url to associate this attribute
                       extension. */
                    lslpURL *rply_url = reply->msg.srvRply.urlList;
                    if (rply_url != 0 && ! _LSLP_IS_HEAD(rply_url->next))
                    {
                        char *url_buf;
                        int16 url_len = _LSLP_GETSHORT(xtptr, 5);
                        rply_url = rply_url->next;
                        url_buf = (char *)calloc(1, url_len + 1);
                        if (url_buf == 0)
                        {
                            printf("Memory allocation failed in file %s at "
                                "Line number %d\n", __FILE__, __LINE__);
                            free(reply);
                            exit(1);
                        }
                        memcpy(url_buf, xtptr + 7, url_len);
                        while (! _LSLP_IS_HEAD(rply_url))
                        {
                            if (TRUE == lslp_pattern_match2(
                                url_buf, rply_url->url, FALSE))
                            {
                                /* this is the correct url to associate with
                                   the next attribute */
                                int16 attr_len;
                                int32 attr_offset = 7 +
                                    _LSLP_GETSHORT(xtptr, 5);
                                attr_len = _LSLP_GETSHORT(xtptr, attr_offset);
                                attr_offset += 2;
                                if ((xtptr + attr_offset + attr_len) <
                                    xtn_limit)
                                {
                                    lslpAtomList *temp = lslpAllocAtom();
                                    if (temp != NULL)
                                    {
                                        temp->str =
                                            (char *)malloc(attr_len + 1);
                                        if (temp->str != NULL)
                                        {
                                            memcpy(temp->str, xtptr +
                                                attr_offset, attr_len);
                                            temp->str[attr_len] = 0x00;
                                            /* allocate an attr list head */
                                            if (rply_url->attrs == NULL)
                                            {
                                                rply_url->attrs =
                                                    lslpAllocAtomList();
                                            }

                                            if (rply_url->attrs != NULL)
                                            {
                                                _LSLP_INSERT(
                                                    temp,
                                                    rply_url->attrs);
                                            }
                                        }
                                        else
                                        {
                                            lslpFreeAtom(temp);
                                        }
                                    } /* alloced atom */
                                } /* sanity check */
                            } /* if we found the right url */
                            else
                            {
                            }
                            rply_url = rply_url->next;
                        } /* while traversing urls in the reply */
                        free(url_buf);
                    } /* if the reply contains urls */
                } /* correct extension */
                if (next_ext == 0)
                {
                    break;
                }
                xtptr = client->_rcv_buf + next_ext;
            } /* traversing extensions */
        }
        /* link the response to the client */
        _LSLP_INSERT(reply, &(client->replies));
    }
    DEBUG_PRINT((DEBUG_EXIT, "decode_srvrply "));
    return;
}

void decode_daadvert(struct slp_client *client, SOCKADDR *remote)
{
    char *bptr = 0;
    int16 str_len;
    int32 total_len, purported_len;

    DEBUG_PRINT((DEBUG_ENTER, "decode_daadvert "));

    bptr = client->_rcv_buf;
    purported_len = _LSLP_GETLENGTH(bptr);
    bptr += (total_len = _LSLP_HDRLEN(bptr));
    if (total_len < purported_len)
    {
        struct da_list *adv = alloc_da_node(FALSE);
        if (adv == NULL)
        {
            printf("Memory allocation failed in file %s at Line number %d\n",
                __FILE__, __LINE__);
            exit(1);
        }
        adv->function = LSLP_DAADVERT;
        adv->err = _LSLP_GETSHORT(bptr, 0);
        adv->stateless_boot = _LSLP_GETLONG(bptr, 2);
        total_len += (8 + (str_len = _LSLP_GETSHORT(bptr, 6)));

        if (total_len < purported_len)
        {
            /* decode and capture the url  - note: this is a string,
               not a url-entry structure */
            if (NULL == (adv->url = (char *)malloc(str_len + 1)))
            {
                printf("Memory allocation failed in file %s at Line number "
                    "%d\n", __FILE__, __LINE__);
                exit(1);
            }
            memcpy(adv->url, bptr + 8, str_len);
            *((adv->url) + str_len) = 0x00;
            /* advance the pointer past the url string */
            bptr += (str_len + 8);
            total_len += (2 + (str_len = _LSLP_GETSHORT(bptr, 0)));
            if (total_len < purported_len)
            {
                if (str_len > 0)
                {
                    if (NULL == (adv->scope = (char *)malloc(str_len + 1)))
                    {
                        printf("Memory allocation failed in file %s at Line"
                            " number %d\n", __FILE__, __LINE__);
                        exit(1);
                    }
                    memcpy(adv->scope, bptr + 2, str_len);
                    *((adv->scope) + str_len) = 0x00;
                }
                /* advance the pointer past the scope string  */
                bptr += (str_len + 2);
                total_len += (2 + (str_len = _LSLP_GETSHORT(bptr, 0)));
                if (total_len < purported_len)
                {
                    if (str_len > 0)
                    {
                        if (NULL == (adv->attr = (char *)malloc(str_len + 1)))
                        {
                            printf("Memory allocation failed in file %s at "
                                "Line number %d\n", __FILE__, __LINE__);
                            exit(1);
                        }
                        memcpy(adv->attr, bptr + 2, str_len);
                        *((adv->attr) + str_len) = 0x00;
                    }
                    /* advance the pointer past the attr string */
                    bptr += (str_len + 2);
                    total_len += (2 + (str_len = _LSLP_GETSHORT(bptr, 0)));
                    if (total_len < purported_len)
                    {
                        struct da_list * exists;

                        if (str_len > 0)
                        {
                            if (NULL == (adv->spi = (char*)malloc(str_len + 1)))
                            {
                                printf("Memory allocation failed in file %s at"
                                    " Line number %d\n", __FILE__, __LINE__);
                                exit(1);
                            }
                            memcpy(adv->spi, bptr + 2, str_len);
                            *((adv->spi) + str_len) = 0x00;
                        } /*  if there is an spi  */

                        /* advance the pointer past the spi string  */
                        bptr += (str_len + 2);
                        adv->auth_blocks = _LSLP_GETBYTE(bptr, 0);

                        // if we already know about this da, remove the
                        // existing entry from our cache and insert this new
                        // entry maybe the stateless boot field changed or the
                        // da supports new scopes, etc.
                        exists  = da_node_exists(&(client->das),  adv->url);
                        if (NULL != exists)
                        {
                            _LSLP_UNLINK(exists);
                            free_da_list_node(exists);
                        }
                        /* need code here to handle authenticated urls */
                        strcpy(&(adv->remote[0]), _slp_get_text_ip(remote));
                        _LSLP_INSERT(adv, &(client->das))
                        DEBUG_PRINT((DEBUG_EXIT, "decode_daadvert:1 "));
                        
                        //Returning here as we need the da
                        return;

                    } /*  spi length field is consistent with hdr */
                } /* attr length field is consistent with hdr */
            } /*  scope length field is consistent with hdr */
        }
        free_da_list_node(adv);
    }
    DEBUG_PRINT((DEBUG_EXIT, "decode_daadvert:2 "));
    return;
}


void decode_srvreq(struct slp_client *client, SOCKADDR *remote)
{

    char *bptr, *bptrSave;
    int32 total_len, purported_len;
    BOOL mcast;
    struct slp_if_addr local_address;
    struct lslp_srv_rply_out *rp_out = NULL;
    struct lslp_srv_req *rq = NULL;
    int16 str_len, buf_len, err = LSLP_PARSE_ERROR;
    size_t addr_len;
    int idx;

    DEBUG_PRINT((DEBUG_ENTER, "decode_srvreq "));

    mcast = ( ((_LSLP_GETFLAGS( client->_rcv_buf )) &
        (LSLP_FLAGS_MCAST) ) ? TRUE : FALSE   ) ;
    bptr = client->_rcv_buf;
    purported_len = _LSLP_GETLENGTH(bptr);

    bptr += (total_len = _LSLP_HDRLEN(bptr));
    if (purported_len < LSLP_MTU && (total_len < purported_len))
    {
        if (! _LSLP_IS_EMPTY((lslpSrvRegList *)client->regs))
        {
            // advance past the slp v2 header
            // get the previous responder list
            str_len = _LSLP_GETSHORT(bptr, 0);
            if ((str_len + total_len + 2) < purported_len)
            {
                if (FALSE == slp_previous_responder(
                    client, (str_len ? bptr + 2 : NULL ), remote->sa_family))
                {
                    rq = (struct lslp_srv_req *)calloc(
                        1, sizeof(struct lslp_srv_req));
                    if (NULL != rq)
                    {
                        bptr += 2 + str_len;
                        total_len += 2 + str_len;
                        // extract the service type string
                        rq->srvcTypeLen = (str_len = _LSLP_GETSHORT(bptr, 0));
                        if (str_len && (str_len + total_len + 2 <
                            purported_len))
                        {
                            rq->srvcType = (char *)malloc(str_len + 1);
                            if (rq->srvcType == NULL)
                            {
                                free(rq);
                                DEBUG_PRINT((DEBUG_EXIT, "decode_srvreq:1 "));
                                return ;
                            }
                            memcpy(rq->srvcType, bptr + 2, str_len);
                            *(rq->srvcType + str_len) = 0x00;
                            bptr += 2 + str_len;
                            total_len += 2 + str_len;
                            bptrSave = bptr;
                            buf_len = LSLP_MTU - total_len;
                            rq->scopeList =
                                lslpUnstuffScopeList(&bptr, &buf_len, &err);
                            total_len += _LSLP_GETSHORT(bptrSave, 0);
                            if (TRUE == lslp_scope_intersection(
                                client->_scopes, rq->scopeList))
                            {
                                /* continue marshalling data */

                                if (total_len < LSLP_MTU - 2)
                                {
                                    /* get the predicate */
                                    rq->predicateLen =
                                        (str_len = _LSLP_GETSHORT(bptr, 0));
                                    if (str_len && (str_len + total_len + 2)
                                        < LSLP_MTU)
                                    {
                                        rq->predicate =
                                            (char *)malloc(str_len + 1);
                                        if (rq->predicate != NULL)
                                        {
                                            memcpy(
                                                rq->predicate,
                                                bptr + 2,
                                                str_len);
                                            *(rq->predicate + str_len) = 0x00;
                                        }
                                    } /* predicate */

                                    bptr += str_len + 2;
                                    total_len += str_len + 2;
                                    /* get the spi list */
                                    buf_len = LSLP_MTU - total_len ;
                                    rq->spiList = lslpUnstuffSPIList(
                                        &bptr,
                                        &buf_len,
                                        &err);

                                    /* check for an extension */
                                    rq->next_ext =
                                        lslp_get_next_ext(client->_rcv_buf);
                                    if (rq->next_ext)
                                    {
                                        /* derive the offset limit for the
                                           next extension and do a sanity
                                           check */
                                        rq->ext_limit = (char *)(
                                            purported_len + client->_rcv_buf);
                                        if (rq->ext_limit < rq->next_ext)
                                        {
                                            rq->next_ext = NULL;
                                        }
                                    }
                                    /* initialize the scratch buffer before
                                       processing the srvrq that way we can
                                       know if there are extensions if the
                                       first 5 bytes changed */
                                    memset(client->_scratch, 0x00, LSLP_MTU);

                                    _LSLP_SETSHORT(client->_scratch,0x0000,0);
                                    _LSLP_SET3BYTES(
                                        client->_scratch,
                                        0x00000000,
                                        2);
                                    rp_out = _lslpProcessSrvReq(client, rq, 0,
                                        remote);
                                } /* sanity check on remaining buffer */
                            } /* my scopes intersect */
                        } /* service type sanity check */
                    } /* allocated req struct */
                } /* not a previous responder */
            } /* pr list sanity check */
        } /* we have regs */

        if (mcast == FALSE || (rp_out != NULL && rp_out->urlCount > 0 ))
        {
            SOCKETD sock; //jeb
            char *extptr, *next_extptr, *next_extptr_save;
            int32 ext_offset;

            // we need to respond to this message
            _LSLP_SETVERSION(client->_msg_buf, LSLP_PROTO_VER);
            _LSLP_SETFUNCTION(client->_msg_buf, LSLP_SRVRPLY);
            // skip the length for now
            _LSLP_SETFLAGS(client->_msg_buf, 0);

            _LSLP_SETNEXTEXT(client->_msg_buf, 0);
            _LSLP_SETXID( client->_msg_buf, (_LSLP_GETXID(client->_rcv_buf)));
            _LSLP_SETLAN(client->_msg_buf, LSLP_EN_US, LSLP_EN_US_LEN);
            total_len = _LSLP_HDRLEN(client->_msg_buf);
            bptr = client->_msg_buf + total_len;
            if (rp_out != NULL)
            {
                if (rp_out->urlLen < (LSLP_MTU - total_len))
                {
                    memcpy(bptr, rp_out->urlList, rp_out->urlLen);
                    bptr += rp_out->urlLen;
                    total_len += rp_out->urlLen;
                }
                else
                {
                    _LSLP_SETSHORT( bptr, 0, 0 );
                    _LSLP_SETFLAGS(client->_msg_buf, LSLP_FLAGS_OVERFLOW);
                    total_len += 2;
                    bptr += 2;
                    /* clear the extension ptr, we don't have room for it. */
                    rq->next_ext = NULL;
                }
            }
            else
            {
                /* set the error code */
                _LSLP_SETSHORT(bptr, 0, 0);
                /* set the url count */
                _LSLP_SETSHORT(bptr, 0, 2);
                bptr += 4;
                total_len += 4;
            }
            extptr = client->_scratch;
            next_extptr_save = next_extptr = client->_msg_buf + LSLP_NEXT_EX;
            ext_offset = (int32)(bptr - client->_msg_buf);

            if (0x0002 == (_LSLP_GETSHORT(extptr, 0)))
            {
                /* set the next extension field in the header */
                _LSLP_SET3BYTES(next_extptr, ext_offset, 0);
                while (0x0002 == (_LSLP_GETSHORT(extptr, 0)))
                {
                    /* check see if it fits */
                    int16 ext_len = 5;
                    /* accumulate url len */
                    ext_len += (2 + _LSLP_GETSHORT(extptr, ext_len));
                    /* accumulate attr len */
                    ext_len += (2 + _LSLP_GETSHORT(extptr, ext_len));
                    /* add the auths byte */
                    ext_len++;
                    if ((ext_len + total_len) < LSLP_MTU)
                    {
                        /* set the next extension pointer to be the third byte
                           of the first extension */
                        next_extptr = client->_msg_buf + ext_offset + 2;

                        /* copy the extension */
                        memcpy(client->_msg_buf + ext_offset, extptr, ext_len);
                        bptr += ext_len;
                        total_len += ext_len;

                        /* set the offset of the next extension */
                        _LSLP_SET3BYTES(
                            (next_extptr_save = next_extptr),
                            ext_offset + ext_len,
                            0);
                        extptr += ext_len;

                        /* adjust the pointers */
                        ext_offset += ext_len;
                        next_extptr = client->_msg_buf + ext_offset + 2;

                        /* minimum length of attr extension is 10 bytes -
                           see if we should continue */
                        if (total_len + 10 >= LSLP_MTU)
                        {
                            /* no room, set next ext field in this extension
                               to zero */
                            _LSLP_SET3BYTES(next_extptr_save, 0x00000000, 0);
                            /* if there is another extension, set the
                               truncated flag */
                            if (0x0002 == (_LSLP_GETSHORT(extptr, 0)))
                            {
                                _LSLP_SETFLAGS(
                                    client->_msg_buf,
                                    LSLP_FLAGS_OVERFLOW);
                            }
                            break;
                        } /* no more room */
                    }
                    else
                    {
                        _LSLP_SET3BYTES(next_extptr_save, 0x00000000, 0);
                    }
                } /* while there are extensions */
                /* terminate the last extension */
                _LSLP_SET3BYTES(next_extptr_save, 0x00000000, 0);
            } /* if there is at least one extension */

            // ok, now we can set the length
            _LSLP_SETLENGTH(client->_msg_buf, total_len );

            _LSLP_SETSHORT(client->_scratch, 0x0000, 0);
            _LSLP_SET3BYTES(client->_scratch, 0x00000000, 2);
            // client->_msg_buf is stuffed with the service reply. now we need
            // to allocate a socket and send it back to the requesting node

            if (remote->sa_family == AF_INET)
            {
                addr_len = sizeof(SOCKADDR_IN);
                idx = 0;
            }
#ifdef PEGASUS_ENABLE_IPV6
            else
            {
                addr_len = sizeof(SOCKADDR_IN6);
                idx = 1;
            }
#endif

            if (client->_local_addr_any)
            {
                local_address.af = remote->sa_family;
                if (local_address.af == AF_INET)
                {
                    local_address.ip4_addr.s_addr = INADDR_ANY;
                }
#ifdef PEGASUS_ENABLE_IPV6
                else
                {
                    local_address.ip6_addr = in6addr_any;
                }
#endif
            }
            else
            {
                PEGASUS_ASSERT(remote->sa_family == client->_local_addr.af);
                local_address = client->_local_addr;
            }

#ifdef PEGASUS_OS_ZOS
            struct slp_if_addr *ptr_addr;
            ptr_addr = client->_local_addr_list[idx];

            while (ptr_addr->af != AF_UNSPEC)
            {
                local_address = *ptr_addr;
#endif

                if (-1 != _slp_create_bind_socket(
                    &sock,
                    local_address.af,
                    client->_target_port,
                    &local_address.ip4_addr,
                    TRUE))
                {
                    if (mcast == TRUE)
                    {
                        _LSLP_SLEEP(rand() % 30);
                    }
                    _LSLP_SENDTO(
                        sock,
                        client->_msg_buf,
                        total_len,
                        0,
                        remote,
                        addr_len);
                }
                _LSLP_CLOSESOCKET(sock);
#ifdef PEGASUS_OS_ZOS
                ptr_addr++;
            } // end of while loop around all local network interfaces
#endif
        } // we must respond to this request

        /* free resources */
        if (rq != NULL)
        {
            if (rq->prList != NULL)
            {
                free(rq->prList);
            }
            if (rq->srvcType != NULL)
            {
                free(rq->srvcType);
            }
            if (rq->scopeList != NULL)
            {
                lslpFreeScopeList(rq->scopeList);
            }
            if (rq->predicate != NULL)
            {
                free(rq->predicate);
            }
            if (rq->spiList != NULL)
            {
                lslpFreeSPIList(rq->spiList);
            }
            free(rq);
        }
        if (rp_out != NULL)
        {
            if (rp_out->urlList != NULL)
            {
                free(rp_out->urlList);
            }
            free(rp_out);
        }
    } /* header sanity check */
    DEBUG_PRINT((DEBUG_EXIT, "decode_srvreq "));
}

BOOL  srv_reg(
    struct slp_client *client,
    const char *url,
    const char *attributes,
    const char *service_type,
    const char *scopes,
    int16 lifetime)
{
    int32 len;
    int16 str_len, buf_len;
    char *bptr;
    lslpURL *url_entry = NULL;

    DEBUG_PRINT((DEBUG_ENTER, "srvreq "));

    /* this is always a new request */
    memset( client->_pr_buf, 0x00, LSLP_MTU);
    client->_pr_buf_len = 0;
    client->_xid++;

    memset(client->_msg_buf, 0x00, LSLP_MTU);
    bptr = client->_msg_buf;
    _LSLP_SETVERSION(bptr, LSLP_PROTO_VER);
    _LSLP_SETFUNCTION(bptr, LSLP_SRVREG);
    /* we don't know the length yet */
    _LSLP_SETXID(bptr, client->_xid);
    _LSLP_SETLAN(bptr, LSLP_EN_US, LSLP_EN_US_LEN);
    /* Set FRESH flag, we don't support incremental registrations */
    _LSLP_SETFLAGS(bptr, LSLP_FLAGS_FRESH);
    bptr += (len =  _LSLP_HDRLEN(bptr));

    /* construct a url-entry  */

    url_entry = (lslpURL *)calloc(1, sizeof(lslpURL));
    if (url_entry == NULL)
    {
        DEBUG_PRINT((DEBUG_EXIT, "srvreq:err "));
        return FALSE;
    }
    url_entry->lifetime = lifetime + time(NULL);
    url_entry->len = (uint16)strlen(url);
    url_entry->url = strdup(url);
    url_entry->auths = 0;
    buf_len = LSLP_MTU - len;

    if (TRUE == lslpStuffURL(&bptr, &buf_len, url_entry))
    {
        len = LSLP_MTU - buf_len;
        /* stuff the service type string */

        /* stuff the service type  */
        str_len = (int16)strlen(service_type);
        if (len + 2 + str_len < LSLP_MTU)
        {
            int retries;
            _LSLP_SETSHORT(bptr, str_len, 0);
            memcpy(bptr + 2, service_type, str_len);
            bptr += (2 + str_len);
            len += (2 + str_len);
            /* stuff the scope list if there is one */
            if (scopes == NULL)
            {
                str_len = 0;
            }
            else
            {
                str_len = (int16)strlen(scopes);
            }
            if (len + 2 + str_len < LSLP_MTU)
            {
                _LSLP_SETSHORT(bptr, str_len, 0);
                if (str_len)
                {
                    memcpy(bptr + 2, scopes, str_len);
                }
                len += (2 + str_len);
                bptr += (2 + str_len);
                /* stuff the attribute string if there is one */
                if (attributes == NULL)
                {
                    str_len = 0;
                }
                else
                {
                    str_len = (int16)strlen(attributes);
                }
                if (len + 2 + str_len < LSLP_MTU)
                {
                    _LSLP_SETSHORT(bptr, str_len, 0);
                    if (str_len)
                    {
                        memcpy(bptr + 2, attributes, str_len);
                    }

                    len += ( 2 + str_len);
                    bptr += (2 + str_len);

                    /* <<< Thu Jun 10 09:03:11 2004 mdd >>> force no
                       authentication blocks */

                    /* no attribute auths for now */
                    if (len + 1 < LSLP_MTU)
                    {
                        _LSLP_SETBYTE(bptr, 0x00, 0);
                    }
                    len += 1;
                    /* set the length field in the header */
                    _LSLP_SETLENGTH( client->_msg_buf, len);

                    retries = client->_retries;
                    while (--retries)
                    {
                        if (TRUE == send_rcv_udp(client))
                        {
                            if (LSLP_SRVACK ==
                                _LSLP_GETFUNCTION(client->_rcv_buf))
                            {
                                if (0x0000 == _LSLP_GETSHORT(
                                    client->_rcv_buf,
                                    (_LSLP_HDRLEN( client->_rcv_buf ))))
                                {
                                    memset(client->_msg_buf, 0x00, LSLP_MTU);
                                    if (url_entry != NULL)
                                    {
                                        lslpFreeURL(url_entry);
                                    }
                                    DEBUG_PRINT((DEBUG_EXIT, "srvreq:ok "));
                                    return(TRUE);
                                }
                            }
                        } // received a response
                    } // retrying the unicast
                } /* attribute string fits into buffer */
            } /* scope string fits into buffer  */
        } /* service type fits into buffer  */
    } /* url fits into buffer  */
    memset( client->_msg_buf, 0x00, LSLP_MTU);
    if (url_entry != NULL)
    {
        lslpFreeURL(url_entry);
    }
    DEBUG_PRINT((DEBUG_EXIT, "srvreq:err2 "));
    return(FALSE);
}


BOOL send_rcv_udp(struct slp_client *client)
{
#ifdef PEGASUS_ENABLE_IPV6
    SOCKADDR_IN6 ip6;
#endif
    SOCKADDR_IN ip4;
    SOCKETD sock;               //jeb
    void * target;
    BOOL ccode = FALSE;
    int af;
    size_t addr_len;
    int err;

    DEBUG_PRINT((DEBUG_ENTER, "send_rcv_udp "));

    af = client->_target_addr.af;

    if (-1 != _slp_create_bind_socket(&sock, af, 0,
        &client->_local_addr.ip4_addr, TRUE))
    {
            int bcast =
                ((_LSLP_GETFLAGS(client->_msg_buf)) & LSLP_FLAGS_MCAST) ? 1:0;
            if (bcast)
            {
#ifdef PEGASUS_OS_ZOS
                int ttl = 0;
                int sockopt = 0;
                unsigned char my_ttl = 128;
                ttl = _LSLP_SET_TTL(sock, my_ttl);
                // Broadcast messages are not supported by IPv6.
                if (af == AF_INET)
                {
                    sockopt = _LSLP_SETSOCKOPT(
                        sock,
                        SOL_SOCKET,
                        SO_BROADCAST,
                        (const char *) &bcast,
                        sizeof(bcast));
                }
                if ((SOCKET_ERROR == ttl ) || (SOCKET_ERROR == sockopt))
                {
#else
                if (af == AF_INET && (SOCKET_ERROR ==
                      _LSLP_SET_TTL(sock, client->_ttl) ||
                    SOCKET_ERROR == _LSLP_SETSOCKOPT(
                    sock,
                    SOL_SOCKET,
                    SO_BROADCAST,
                    (const char *)&bcast,
                    sizeof(bcast))))
                {
#endif
                    _LSLP_CLOSESOCKET(sock);
                    DEBUG_PRINT((DEBUG_EXIT, "send_rcv_udp:err "));
                    return(FALSE);
                }

                if (af == AF_INET &&
                    client->_local_addr.ip4_addr.s_addr != INADDR_ANY)
                {
                    struct in_addr ma;
                    memset(&ma, 0x00, sizeof(ma));
                    ma.s_addr = client->_local_addr.ip4_addr.s_addr;
                    if ((SOCKET_ERROR == _LSLP_SETSOCKOPT(
                        sock,
                        IPPROTO_IP,
                        IP_MULTICAST_IF,
                        (const char *)&ma,
                        sizeof(ma))))
                    {
                        _LSLP_CLOSESOCKET(sock);
                        DEBUG_PRINT((DEBUG_EXIT, "send_rcv_udp:err2 "));
                        return(FALSE);
                    }
                }
                else
                {
                  /* ATTN: implement for IPv6 */
                }
            }
            if (af == AF_INET)
            {
                ip4.sin_family = AF_INET;
                ip4.sin_port = htons(client->_target_port);
                ip4.sin_addr = client->_target_addr.ip4_addr;
                addr_len = sizeof(ip4);
                target = &ip4;
            }
#ifdef PEGASUS_ENABLE_IPV6
            else
            {
                addr_len = sizeof(ip6);
                memset(&ip6, 0, addr_len);
                ip6.sin6_family = AF_INET6;
                ip6.sin6_port = htons(client->_target_port);
                ip6.sin6_addr = client->_target_addr.ip6_addr;
                target = &ip6;
            }
#endif
            if (SOCKET_ERROR == (err = _LSLP_SENDTO(
                sock,
                client->_msg_buf,
                _LSLP_GETLENGTH(client->_msg_buf),
                0,
                target,
                addr_len)))
            {
                _LSLP_CLOSESOCKET(sock);
                DEBUG_PRINT((DEBUG_EXIT, "send_rcv_udp "));
                return(FALSE);
            } /* oops - error sending data */

            while (0 < __service_listener(client, sock))
            {
                ccode = TRUE;
            }
            _LSLP_CLOSESOCKET(sock);
    }

    DEBUG_PRINT(
        (DEBUG_EXIT, "send_rcv_udp:%s",
        (ccode == FALSE ? "err3" : "ok")));

    return(ccode);
}

int32 service_listener(
    struct slp_client *client,
    SOCKETD extra_sock,
    lslpMsg *list)
{
    int32 ccode = 0;

//    DEBUG_PRINT((DEBUG_ENTER, "service_listener "));

    if (list != NULL)
    {
        list->isHead = TRUE;
        list->prev = list->next = list;

        ccode = __service_listener(client, extra_sock);

        //fills the list with replies and
        //should be free by caller of this function
        get_response(client, list);
    }

//    DEBUG_PRINT((DEBUG_EXIT, "service_listener ccode = %d", ccode));
    return ccode;
}


int32 __service_listener(
    struct slp_client *client,
    SOCKETD extra_sock ) //jeb
{
    struct timeval tv;
    int32 err;
    SOCKETD max_sock = -1;

    LSLP_FD_SET fds;
    _LSLP_FD_ZERO(&fds);

//    DEBUG_PRINT((DEBUG_ENTER, "__service_listener "));

    if (client->_rcv_sock[0] != INVALID_SOCKET)
    {
        _LSLP_FD_SET(client->_rcv_sock[0], &fds);
        max_sock = client->_rcv_sock[0];
    }
    if (client->_rcv_sock[1] != INVALID_SOCKET)
    {
        _LSLP_FD_SET(client->_rcv_sock[1], &fds);
        if (max_sock < client->_rcv_sock[1])
        {
            max_sock = client->_rcv_sock[1];
        }
    }

    if (extra_sock)
    {
        _LSLP_FD_SET( extra_sock, &fds);
        if (max_sock < extra_sock)
        {
            max_sock = extra_sock;
        }

    }

    do
    {
        tv.tv_sec = client->_tv.tv_sec;
        tv.tv_usec = client->_tv.tv_usec;
        err = _LSLP_SELECT(max_sock + 1, &fds, NULL, NULL, &tv);
    }while ((err < 0 ) && (errno == EINTR));


    if ( 0 < err )
    {
#ifdef PEGASUS_ENABLE_IPV6
        SOCKADDR_STORAGE remote;
#else
        SOCKADDR_IN remote;
#endif
        int size = sizeof(remote);
        int i;
        for (i = 0; i < 2 ; ++i)
        {
            if (client->_rcv_sock[i] != INVALID_SOCKET &&
                _LSLP_FD_ISSET(client->_rcv_sock[i], &fds))
            {
                err = _LSLP_RECV_FROM(
                    client->_rcv_sock[i],
                    client->_rcv_buf,
                    LSLP_MTU,
                    0,
                    &remote,
                    &size);
                decode_msg(client, (SOCKADDR*)&remote);
            }
        }

        if (extra_sock && _LSLP_FD_ISSET(extra_sock, &fds))
        {
            err = _LSLP_RECV_FROM(
                extra_sock,
                client->_rcv_buf,
                LSLP_MTU,
                0,
                &remote,
                &size);
            decode_msg(client, (SOCKADDR*)&remote);
        }
    }

    if (err == SOCKET_ERROR)
    {
        // our interfaces could be disconnected or we could be a laptop that
        // just got pulled from the network, etc.
        slp_get_local_interfaces(client);
        slp_open_listen_socks(client);
#ifdef PEGASUS_ENABLE_IPV6
        slp_join_ip6_service_type_multicast_group(client, client->_srv_type);
#endif
    }

//    DEBUG_PRINT((DEBUG_EXIT, "__service_listener err = %d", err));
    return err;
}

int srv_reg_all(
    struct slp_client *client,
    const char *url,
    const char *attributes,
    const char *service_type,
    const char *scopes,
    int16 lifetime)
{
    int convergence_save;
    int af = 0;

    // keep track of how many times we register
    int registrations = 0;
    DEBUG_PRINT((DEBUG_ENTER, "srv_reg_all "));

    PEGASUS_ASSERT(client != NULL && url != NULL && attributes != NULL &&
        service_type != NULL && scopes != NULL);
    if (client == NULL || url == NULL || attributes == NULL ||
        service_type == NULL || scopes == NULL)
    {
        DEBUG_PRINT((DEBUG_EXIT, "srv_reg_all:err "));
        return(0);
    }

    // save convergence parameter
    convergence_save = client->_convergence;
    client->_convergence = 0;

    if (client->_use_das == TRUE)
    {
        find_das(client, NULL, scopes);
        if (! _LSLP_IS_EMPTY(&(client->das)))
        {
            struct da_list *da = client->das.next;
            while (! _LSLP_IS_HEAD(da))
            {
                if (slp_is_valid_ip4_addr(da->remote))
                {
                    af = AF_INET;
                    client->_target_addr.ip4_addr.s_addr =
                        inet_addr(da->remote);
                }
#ifdef PEGASUS_ENABLE_IPV6
                else if (slp_is_valid_ip6_addr(da->remote))
                {
                    af = AF_INET6;
                    slp_pton(AF_INET6,da->remote,
                        &client->_target_addr.ip6_addr);
                }
#endif
                else
                {
                    PEGASUS_ASSERT(0);
                }

                if (client->_local_addr_any)
                {
                    client->_local_addr.af = af;
                    if (af == AF_INET)
                    {
                        client->_local_addr.ip4_addr.s_addr = INADDR_ANY;
                    }
#ifdef PEGASUS_ENABLE_IPV6
                    else
                    {
                        client->_local_addr.ip6_addr = in6addr_any;
                    }
#endif
                }
                if (client->_target_addr.af == client->_local_addr.af &&
                    TRUE == srv_reg(
                    client,
                    url,
                    attributes,
                    service_type,
                    scopes,
                    lifetime))
                {
                    registrations++;
                }
                da = da->next;;
            }
        }
    }
    // restore parameters
    client->_convergence = convergence_save;

    af = client->_target_addr.af;

    if (client->_local_addr_any)
    {
        client->_local_addr.af = af;
        if (af == AF_INET)
        {
            client->_local_addr.ip4_addr.s_addr = INADDR_ANY;
        }
#ifdef PEGASUS_ENABLE_IPV6
        else
        {
            client->_local_addr.ip6_addr = in6addr_any;
        }
#endif
    }

    /* do a unicast srvreg if we have the target set */
    if ( client->_target_addr.af == client->_local_addr.af &&
         !((af == AF_INET &&
         client->_target_addr.ip4_addr.s_addr == _LSLP_MCAST)
#ifdef PEGASUS_ENABLE_IPV6
         || (af == AF_INET6 &&
               IN6_IS_ADDR_MULTICAST(&client->_target_addr.ip6_addr))
#endif
         ))
    {
        if (TRUE == srv_reg(
            client,
            url,
            attributes,
            service_type,
            scopes,
            lifetime))
        {
            registrations++;
        }
    }

    DEBUG_PRINT((DEBUG_EXIT, "srv_reg_all:ok "));
    return(registrations);
}

int srv_reg_local(
    struct slp_client *client,
    const char *url,
    const char *attributes,
    const char *service_type,
    const char *scopes,
    uint16 lifetime)
{
    int count = 0;

    struct slp_if_addr target_addr_save, local_addr_save;
    int convergence_save;
    BOOL local_any_save, target_any_save;
    DEBUG_PRINT((DEBUG_ENTER, "srv_reg_local "));
    PEGASUS_ASSERT(client != NULL && url != NULL && attributes != NULL &&
        service_type != NULL && scopes != NULL);

    if (client == NULL || url == NULL || attributes == NULL ||
        service_type == NULL || scopes == NULL)
    {
        DEBUG_PRINT((DEBUG_EXIT, "srv_reg_local:err "));
        return 0;
    }

    target_addr_save = client->_target_addr;
    local_addr_save = client->_local_addr;
    convergence_save = client->_convergence;
    client->_convergence = 0;
    local_any_save = client->_local_addr_any;
    target_any_save = client->_target_addr_any;

    client->_local_addr_any = client->_target_addr_any = TRUE;

    if (_slp_can_make_request(client, AF_INET, "127.0.0.1") &&
        _slp_check_url_addr(url, AF_INET, 0))
    {
        if (TRUE == srv_reg(client, url, attributes, service_type,
            scopes, lifetime))
        {
           count = 1;
        }
    }
#ifdef PEGASUS_ENABLE_IPV6
    else if (_slp_can_make_request(client, AF_INET6, "::1") &&
        _slp_check_url_addr(url, AF_INET6, 0))
    {
        if (TRUE == srv_reg(client, url, attributes, service_type,
            scopes, lifetime))
        {
           count = 1;
        }
    }
#endif

    client->_convergence = convergence_save;
    client->_target_addr = target_addr_save;
    client->_local_addr = local_addr_save;
    client->_local_addr_any = local_any_save;
    client->_target_addr_any = target_any_save;

    DEBUG_PRINT((DEBUG_ENTER, "srv_reg_local count=%d", count));
    return count;
}

/* Note: this is how srv_reg_local will look with authentication support */

/* void srv_reg_local(struct slp_client *client,  */
/*            const char *url,  */
/*            int url_auth_count, */
/*            void *url_auth_blob, */
/*            const char *attributes,  */
/*            const char *service_type, */
/*            const char *scopes, */
/*            uint16 lifetime,  */
/*            int attr_auth_count,  */
/*            void *attr_auth_blob) */
/* { */
/* } */

void __srv_reg_local (
    struct slp_client *client,
    const char *url,
    const char *attributes,
    const char *service_type,
    const char *scopes,
    uint16 lifetime)   //jeb int16 to uint16
{

    char *url_copy;
    lslpSrvRegList *reg;

    DEBUG_PRINT((DEBUG_ENTER, "__srv_reg_local "));

    PEGASUS_ASSERT(client != NULL && url != NULL && attributes != NULL && \
        service_type != NULL && scopes != NULL);

    url_copy = strdup(url);
    if (url_copy == NULL)
    {
        printf("Memory allocation failed in file %s at Line number %d\n",
            __FILE__, __LINE__);
        exit(1);
    }

    // Join ip6 service location multicast group if it is new service here.
#ifdef PEGASUS_ENABLE_IPV6
     slp_join_ip6_service_type_multicast_group(client, service_type);
#endif

    /* update the attributes if an existing registration */
    reg   = client->regs->next;
    while (! _LSLP_IS_HEAD(reg))
    {
        if (0 == lslp_string_compare(url_copy, reg->url->url))
        {
            free(url_copy);
            reg->directoryTime = lifetime + time(NULL);
            reg->url->lifetime = lifetime + time(NULL);
            if (reg->attrList != NULL)
            {
                lslpFreeAttrList(reg->attrList, LSLP_DESTRUCTOR_DYNAMIC);
            }
            reg->attrList = _lslpDecodeAttrString((char *)attributes);
            DEBUG_PRINT((DEBUG_EXIT, "__srv_reg_local %s", "1 "));
            return;
        }
        reg = reg->next;
    }
    reg = (lslpSrvRegList *)calloc(1, sizeof(lslpSrvRegList));
    if (reg != NULL)
    {
        int16 len;

        if (scopes == NULL)
        {
            free(reg);
            DEBUG_PRINT((DEBUG_ENTER, "srv_reg_local %s", "2 "));
            return;
        }
        if (NULL == (reg->url = lslpAllocURL()))
        {
            printf("Memory allocation failed in file %s at Line number %d\n",
                __FILE__, __LINE__);
            exit(1);
        }
        reg->directoryTime = lifetime + time(NULL);
        reg->url->atomized = _lslpDecodeURLs(&url_copy, 1);
        reg->url->url = url_copy;
        reg->url->lifetime = lifetime + time(NULL);
        reg->url->auths = 0;
        reg->url->len = (uint16)strlen(url_copy);
        reg->srvType = strdup(service_type);
        len = (int16)strlen(scopes) + 1;
        reg->scopeList  = lslpScopeStringToList(scopes, len);
        len = (int16)strlen(attributes);
        reg->attrList  = _lslpDecodeAttrString((char *)attributes);
        _LSLP_INSERT(reg, (lslpSrvRegList *)client->regs);
    }
    DEBUG_PRINT((DEBUG_EXIT, "srv_reg_local %s", "3 "));
    return;
}

BOOL slp_previous_responder(struct slp_client *client, char *pr_list, int af)
{

    char *a, *s = NULL;
    struct slp_if_addr *list_addr;
    if (pr_list == NULL || 0 == strlen(pr_list))
    {
        return(FALSE);
    }

    a = _LSLP_STRTOK(pr_list, ",", &s);
    while (NULL != a)
    {
        if (af == AF_INET && slp_is_valid_ip4_addr(a))
        {
            list_addr = client->_local_addr_list[0];
            while (list_addr->af != AF_UNSPEC)
            {
                if (list_addr->ip4_addr.s_addr == inet_addr(a))
                {
                    return(TRUE);
                }
                list_addr++;
            }
        }
#ifdef PEGASUS_ENABLE_IPV6
        else if(af == AF_INET6 && slp_is_valid_ip6_addr(a))
        {
            struct in6_addr t;
            list_addr = client->_local_addr_list[1];
            while (list_addr->af != AF_UNSPEC)
            {
                slp_pton(AF_INET6, a, &t);
                if (slp_addr_equal(AF_INET6, &list_addr->ip6_addr, &t))
                {
                    return(TRUE);
                }
                list_addr++;
            }

        }
#endif
        else
        {
            return FALSE;
        }
        a = _LSLP_STRTOK(NULL, ",", &s);
    }
    return(FALSE);
}

struct slp_client *create_slp_client(
    const char *target_addr,
    const char *local_interface,
    uint16 target_port,
    const char *spi,
    const char *scopes,
    BOOL should_listen,
    BOOL use_das,
    const char *srv_type)
{
    int16 len;
    struct slp_client *client;
    int target_af = AF_UNSPEC;
    int local_af = AF_UNSPEC;

    DEBUG_PRINT((DEBUG_ENTER, "create_slp_client "));

    if (spi == NULL || scopes == NULL)
    {
        DEBUG_PRINT((DEBUG_EXIT, "create_slp_client:err "));
        return NULL;
    }

    if (target_addr)
    {
        if (slp_is_valid_ip4_addr(target_addr))
        {
            target_af = AF_INET;
        }
#ifdef PEGASUS_ENABLE_IPV6
        else if ((slp_is_valid_ip6_addr(target_addr)))
        {
            target_af = AF_INET6;
        }
#endif
        else
        {
            DEBUG_PRINT((DEBUG_EXIT, "create_slp_client:err2 "));
            return NULL;
        }
    }

    if (local_interface)
    {
        if (slp_is_valid_ip4_addr(local_interface))
        {
            local_af = AF_INET;
        }
#ifdef PEGASUS_ENABLE_IPV6
        else if ((slp_is_valid_ip6_addr(local_interface)))
        {
            local_af = AF_INET6;
        }
#endif
        else
        {
            DEBUG_PRINT((DEBUG_EXIT, "create_slp_client:err3 "));
            return NULL;
        }
    }

    // Don't allow having local_interface and target_addr of different address
    // types.
    if (local_af != AF_UNSPEC && target_af != AF_UNSPEC)
    {
        // We can not have ipv4 interface sending datagrams to ip6 interface or
        // vice-versa.
        if (local_af != target_af)
        {
            DEBUG_PRINT((DEBUG_EXIT, "create_slp_client:err4 "));
            return NULL;
        }
    }

#ifdef PEGASUS_OS_TYPE_WINDOWS
    {
        int err = WindowsStartNetwork();
        if (err != 0)
        {
            DEBUG_PRINT((DEBUG_EXIT,
                "create_slp_client: WSAStartup() failure, errno %i", err));
            return NULL;
        }
    }
#endif

    client = (struct slp_client *)calloc(1, sizeof(struct slp_client)); //jeb
    if (client == NULL)
    {
        DEBUG_PRINT((DEBUG_EXIT, "create_slp_client:err5 "));
        return NULL;
    }

    /* initialize the random number generator for randomizing the
       timing of multicast responses */
    srand(time(NULL));

    /****** DEBUGGING ****** << Fri Nov 21 10:34:54 2003 mdd >> */
    client->_pr_buf = (char* )calloc(LSLP_MTU, sizeof(char));
    client->_msg_buf = (char* )calloc(LSLP_MTU, sizeof(char));
    client->_rcv_buf = (char* )calloc(LSLP_MTU, sizeof(char));
    client->_scratch = (char* )calloc(LSLP_MTU, sizeof(char));
    client->_err_buf = (char* )calloc(255, sizeof(char));

    client->_target_addr_any = target_af == AF_UNSPEC ? TRUE : FALSE;
    client->_local_addr_any = local_af == AF_UNSPEC ? TRUE : FALSE;

    client->_buf_len = LSLP_MTU;
    client->_version = 1;
    client->_xid = 1;
    client->_target_port = target_port;

    client->_ip4_stack_active = slp_is_ip4_stack_active();

#ifdef PEGASUS_ENABLE_IPV6
    client->_ip6_stack_active = slp_is_ip6_stack_active();
#endif

    if (target_addr == NULL)
    {
        client->_target_addr_any = TRUE;
    }
    else
    {
        client->_target_addr.af = target_af;
        if (target_af == AF_INET)
        {
            client->_target_addr.ip4_addr.s_addr = inet_addr(target_addr);
        }
#ifdef PEGASUS_ENABLE_IPV6
        else
        {
            slp_pton(target_af, target_addr, &client->_target_addr.ip4_addr);
        }
#endif
    }

    if (local_interface == NULL)
    {
        client->_local_addr_any = TRUE;
    }
    else
    {
        client->_local_addr.af = local_af;
        if (local_af == AF_INET)
        {
            client->_local_addr.ip4_addr.s_addr = inet_addr(local_interface);
        }
#ifdef PEGASUS_ENABLE_IPV6
        else
        {
            slp_pton(local_af, local_interface, &client->_local_addr.ip6_addr);
        }
#endif
    }

    if (spi == NULL)
    {
        free(client->_pr_buf);
        free(client->_msg_buf);
        free(client->_rcv_buf);
        free(client->_scratch);
        free(client->_err_buf);
        free(client);
        DEBUG_PRINT((DEBUG_EXIT, "create_slp_client:err3 "));
        return NULL;
    }

    len = (int16)strlen(spi) + 1;
    client->_spi = lslpScopeStringToList(spi, len);

    if (scopes == NULL)
    {
        free(client->_pr_buf);
        free(client->_msg_buf);
        free(client->_rcv_buf);
        free(client->_scratch);
        free(client->_err_buf);
        free(client);
        DEBUG_PRINT((DEBUG_EXIT, "create_slp_client:err4 "));
        return NULL;
    }

    len = (int16)strlen(scopes) + 1;
    client->_scopes = lslpScopeStringToList(scopes, len);
    client->_tv.tv_usec = 200000;     //.2 sec

    client->_use_das = use_das;
    client->_retries = 3;
    client->_ttl = 255;
    client->_convergence = 5;

    client->das.isHead = TRUE;
    client->das.next = client->das.prev = &(client->das);

    client->replies.isHead = TRUE;
    client->replies.next = client->replies.prev = &(client->replies);

    client->regs = (lslpSrvRegHead*)malloc(sizeof(lslpSrvRegHead));
    client->regs->isHead = TRUE;
    client->regs->next = client->regs->prev = (lslpSrvRegList *)client->regs;

    client->_local_addr_list[0] = client->_local_addr_list[1] = NULL;

    // Get both ip4 and ip6 interfaces
    slp_get_local_interfaces(client);

    /* see if we can use a local DA. If not, open up the listen socket */
    client->_rcv_sock[0] = client->_rcv_sock[1] = INVALID_SOCKET;

    if (should_listen == TRUE)
    {
        slp_open_listen_socks(client);
#ifdef PEGASUS_ENABLE_IPV6
        if (srv_type)
        {
            client->_srv_type = (char*)malloc(strlen(srv_type) + 1);
            strcpy(client->_srv_type, srv_type);
            slp_join_ip6_service_type_multicast_group(client, srv_type);
        }
#endif
    }

    if (client->_use_das == TRUE)
    {
        local_srv_req(client, NULL, NULL, "DEFAULT");
        if (! _LSLP_IS_EMPTY(&(client->das)))
        {
            _LSLP_CLOSESOCKET(client->_rcv_sock[0]);
            _LSLP_CLOSESOCKET(client->_rcv_sock[1]);
            client->_rcv_sock[0] = client->_rcv_sock[1] = INVALID_SOCKET;
            client->_use_das = TRUE;
            client->_da_target_port = client->_target_port;
            if (client->_ip4_stack_active)
            {
                client->_da_target_addr.ip4_addr.s_addr =
                    inet_addr("127.0.0.1");
                client->_da_target_addr.af = AF_INET;
            }
#ifdef PEGASUS_ENABLE_IPV6
            else
            {
                slp_pton(AF_INET6, "::1", &client->_da_target_addr.ip6_addr);
                client->_da_target_addr.af = AF_INET6;
            }
#endif
        }
    }


    /* field and record separators */
    client->_use_separators = FALSE;
    client->_fs = '\0';
    client->_rs = '\0';

    client->get_response = get_response;
    client->find_das = find_das;
    client->converge_srv_req = converge_srv_req;
    client->unicast_srv_req = unicast_srv_req;
    client->local_srv_req = local_srv_req;
    client->srv_req = srv_req;

    client->converge_attr_req = converge_attr_req;
    client->unicast_attr_req = unicast_attr_req;
    client->local_attr_req = local_attr_req;
    client->attr_req = attr_req;
    client->decode_attr_rply = decode_attr_rply;

    client->srv_reg = srv_reg;
    client->srv_reg_all = srv_reg_all;
    client->srv_reg_local = srv_reg_local;
    client->service_listener = service_listener;
    client->prepare_pr_buf = prepare_pr_buf;
    client->decode_msg = decode_msg;
    client->decode_srvreq = decode_srvreq;
    client->decode_srvrply = decode_srvrply;
    client->decode_attrreq = decode_attrreq;

    client->decode_daadvert = decode_daadvert;
    client->send_rcv_udp = send_rcv_udp;
    client->slp_previous_responder = slp_previous_responder;

    DEBUG_PRINT((DEBUG_EXIT, "create_slp_client:ok "));
    return client;
}

void destroy_slp_client(struct slp_client *client)
{
    DEBUG_PRINT((DEBUG_ENTER, "destroy_slp_client "));
    if (client == NULL)
    {
        DEBUG_PRINT((DEBUG_EXIT, "destroy_slp_client:err "));
        return;
    }

    _LSLP_CLOSESOCKET(client->_rcv_sock[0]);
    _LSLP_CLOSESOCKET(client->_rcv_sock[1]);

    _LSLP_FREE_DEINIT(client->_local_addr_list[0]);
    _LSLP_FREE_DEINIT(client->_local_addr_list[1]);

    lslpFreeScopeList((lslpScopeList *)(client->_spi));
    lslpFreeScopeList(client->_scopes);
    _LSLP_FREE_DEINIT(client->_crypto_context);

    //Freeing memory allocated during client creation.
    free(client->_pr_buf);
    free(client->_msg_buf);
    free(client->_rcv_buf);
    free(client->_scratch);
    free(client->_err_buf);
    free(client->_srv_type);

    //Freeing memory allocated for regs in __srv_reg_local
    lslpFreeSrvRegList(client->regs);
    free(client->regs);
    free(client);
    DEBUG_PRINT((DEBUG_EXIT, "destroy_slp_client:ok "));
    return;
}

/* returns a length-preceded escaped string compliant with an slp opaque */
/* value length is encoded as two-byte value at the beginning of the string */
/* size of returned buffer will be ((3 * length) + 3 + 2) */
char *encode_opaque(void *buffer, int16 length)
{
    static char transcode[16] =
    {
        '0', '1', '2', '3', '4', '5', '6', '7',
        '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'
    };

    char *bptr, *srcptr, *buf;
    int32 encoded_length;

    if (buffer == NULL || length == 0)
    {
        return(NULL);
    }

    encoded_length = (length * 3) + 5;
    /* see if the encoded length will overflow the max size of an slp string*/
    if (0xffff0000 & encoded_length)
    {
        return NULL;
    }

    buf = (char *) malloc(encoded_length);
    if (buf == NULL)
    {
        return NULL;
    }

    bptr = buf;

    /* encode the length */
    _LSLP_SETSHORT(bptr, (int16)encoded_length, 0);
    bptr += 2;
    srcptr = (char *)buffer;
    *bptr = 0x5c; *(bptr + 1) = 0x66; *(bptr + 2) = 0x66;
    bptr += 3;

    while (length)
    {
        *bptr = 0x5c;
        *(bptr + 1) = transcode[((*srcptr) & 0xf0) >> 4];
        *(bptr + 2) = transcode[(*srcptr) & 0x0f];
        bptr += 3;
        srcptr++;
        length--;
    }
    return buf;
}

/* returns an opaque buffer. size of opaque buffer will be */
/* ((size of escaped opaque string - 2) / 3) - 1  */
void *decode_opaque(char *buffer)
{

    int16 encoded_length, alloc_length;
    char *bptr, *srcptr, *retptr;

    if (buffer == NULL)
    {
        return NULL;
    }

    srcptr = buffer;
    retptr = bptr = NULL;
    /* get the length */
    encoded_length = _LSLP_GETSHORT(srcptr, 0);
    if (0 == encoded_length || 0 > encoded_length)
    {
        return NULL;
    }

    alloc_length = (encoded_length / 3) - 1;

    srcptr += 2;
    encoded_length -= 2;

    /* check the header */
    if (*srcptr == 0x5c)
    {
        if ((*(srcptr + 1) == 0x46) || (*(srcptr + 1) == 0x66))
        {
            if ((*(srcptr + 2) == 0x46) || (*(srcptr + 2) == 0x66))
            {
                retptr = (bptr = (char *) malloc(alloc_length));
                if (bptr == NULL)
                {
                    return NULL;
                }
                /* adjust the encoded length to reflect that we consumed
                   the header */
                encoded_length -= 3;
                srcptr += 3;

                while (encoded_length && alloc_length)
                {
                    char accumulator = 0;
                    if (*srcptr == 0x5c)
                    {
                        /* encode the high nibble */
                        if (*(srcptr + 1) < 0x3a && *(srcptr + 1) > 0x2f)
                        {
                            /* asci decimal char */
                            accumulator = (*(srcptr + 1) - 0x30) * 0x10;
                        }
                        else if (*(srcptr + 1) < 0x47 && *(srcptr + 1) > 0x40)
                        {
                            accumulator = (*(srcptr + 1) - 0x37) * 0x10;
                        }
                        else if (*(srcptr + 1) < 0x67 && *(srcptr + 1) > 0x60)
                        {
                            accumulator = (*(srcptr + 1) - 0x57) * 0x10;
                        }

                        /* encode the low nibble */
                        if (*(srcptr + 2) < 0x3a && *(srcptr + 2) > 0x2f)
                        {
                            /* asci decimal char */
                            accumulator += *(srcptr + 2) - 0x30;
                        }
                        else if (*(srcptr + 2) < 0x47 && *(srcptr + 2) > 0x40)
                        {
                            accumulator += *(srcptr + 2) - 0x37;
                        }
                        else if (*(srcptr + 2) < 0x67 && *(srcptr + 2) > 0x60)
                        {
                            accumulator += *(srcptr + 2) - 0x57;
                        }
                        /* store the byte */
                        *bptr = accumulator;
                    }
                    else
                        break;

                    /* update pointers and counters */
                    srcptr += 3;
                    bptr++;
                    encoded_length -= 3;
                    alloc_length--;
                }
                if (alloc_length || encoded_length)
                {
                    free(retptr);
                    retptr = NULL;
                }
            }
        }
    }
    return(void *)retptr;
}

/* right now we don't support the attr request unless
   it specifies a complete url. */

void decode_attrreq(struct slp_client *client, SOCKADDR *remote)
{
    char *bptr;
    lslpURL *url;
    lslpScopeList *scopes;
    lslpAttrList *attr_tags, *attr_return;
    lslpSrvRegList *regs;
    size_t addr_len;
    int idx;
    int16 str_len, buf_len, err = 0, parse_err;
    int32 total_len, purported_len;
    struct slp_if_addr local_address;

    DEBUG_PRINT((DEBUG_ENTER, "decode_attrreq "));

    /* read the length from the slp header */
    bptr = client->_rcv_buf;
    purported_len = _LSLP_GETLENGTH(bptr);
    if (purported_len > LSLP_MTU)
    {
        DEBUG_PRINT((DEBUG_EXIT, "decode_attrreq:err "));
        return;
    }

    bptr += (total_len = _LSLP_HDRLEN(bptr));

    /* use total_len as a running count of the msg length, ensure it is */
    /* consistent with the purported length to protect against overflow */
    if (total_len < purported_len)
    {
        /* get the length of the PR list and look at it */
        str_len = _LSLP_GETSHORT(bptr, 0);
        if ((str_len + total_len + 2) < purported_len)
        {
            if (FALSE == slp_previous_responder(
                client,
                (str_len ? bptr + 2 : NULL), remote->sa_family))
            {
                bptr += str_len + 2;
                total_len += str_len + 2;

                /* decode the url */
                str_len = _LSLP_GETSHORT(bptr, 0);
                bptr += 2;

                if ((str_len + total_len + 2) < purported_len)
                {
                    if (NULL != (url = lslpAllocURL()))
                    {
                        if (NULL != (url->url =
                            (char *)calloc(str_len + 1, sizeof(char))))
                        {
                            memcpy(url->url, bptr, str_len);
                            *(bptr + str_len) = 0x00;
                            url->atomized =
                                _lslpDecodeURLs( (char **)&(url->url), 1);
                            bptr += str_len + 2;
                            total_len += str_len + 2;
                            buf_len = LSLP_MTU - total_len;
                            str_len = _LSLP_GETSHORT(bptr, 0);
                            scopes = lslpUnstuffScopeList(
                                &bptr,
                                &buf_len,
                                &parse_err);
                            total_len += str_len + 2;
                            /* see if we even support the scopes */
                            //   if( NULL != url->atomized && TRUE ==
                            //lslp_scope_intersection(client->_scopes,scopes)){
                            if (lslp_scope_intersection(
                                client->_scopes,
                                scopes))
                            {
                                /* decode the attribute tags */
                                //jeb str_len =
                                char *bptrSave; SOCKETD sock;
                                attr_tags = lslpUnstuffAttr(
                                    &bptr,
                                    &buf_len,
                                    &parse_err);
                                /* see if we have url match */
                                total_len += str_len + 2;
                                regs = client->regs->next;
                                attr_return = NULL;

                                while (! _LSLP_IS_HEAD(regs))
                                {
                                    if (! lslp_string_compare(
                                        regs->url->url,
                                        url->url))
                                    {
                                        /* we have a hit on the url, see if
                                           the scopes intersect */
                                        if (TRUE == lslp_scope_intersection(
                                            regs->scopeList,
                                            scopes))
                                        {
                                            attr_return = regs->attrList;
                                            err = 0;
                                            break;
                                        }
                                    }
                                    regs = regs->next;
                                }

                                /* stuff the attribute reply */

                                _LSLP_SETVERSION(
                                    client->_msg_buf,
                                    LSLP_PROTO_VER);
                                _LSLP_SETFUNCTION(
                                    client->_msg_buf,
                                    LSLP_ATTRRPLY);
                                // skip the length for now
                                _LSLP_SETFLAGS(client->_msg_buf, 0);
                                _LSLP_SETNEXTEXT(client->_msg_buf, 0);
                                _LSLP_SETXID(
                                    client->_msg_buf,
                                    (_LSLP_GETXID(client->_rcv_buf)));
                                _LSLP_SETLAN(
                                    client->_msg_buf,
                                    LSLP_EN_US, LSLP_EN_US_LEN);
                                total_len = _LSLP_HDRLEN(client->_msg_buf);

                                bptr = client->_msg_buf + total_len;
                                _LSLP_SETSHORT(bptr, err, 0);
                                bptr += 2;
                                total_len += 2;

                                buf_len = LSLP_MTU - (total_len + 1);
                                bptrSave = bptr;

                                /* stuff the attr list */

                                if (FALSE == lslpStuffAttrList(
                                    &bptr,
                                    &buf_len,
                                    attr_return,
                                    attr_tags))
                                {
                                    /* overflow, set the flag, stuff a zero
                                       attr list */
                                    _LSLP_SETFLAGS(
                                        client->_msg_buf,
                                        LSLP_FLAGS_OVERFLOW);
                                    buf_len = LSLP_MTU - total_len;
                                    bptr = bptrSave;
                                    lslpStuffAttrList(
                                        &bptr,
                                        &buf_len,
                                        NULL,
                                        NULL);
                                }
                                /* for now don't support auth blocks */

                                _LSLP_SETBYTE(bptr, 0, 0);

                                /* to calculate the length, re-read the attr
                                   list length */
                                total_len +=(2 + _LSLP_GETSHORT(bptrSave, 0));
                                /* add the length of the auth block (zero plus
                                   the number of auths) */
                                total_len += 1;

                                // ok, now we can set the length
                                _LSLP_SETLENGTH(client->_msg_buf, total_len);

                                /* client->_msg_buf is stuffed with the attr
                                   reply. now we need to allocate a socket and
                                   send it back to the requesting node  */

                                /* only send the response if there is an
                                   attribute or if this is a unicast */
                                if (remote->sa_family == AF_INET)
                                {
                                    addr_len = sizeof(SOCKADDR_IN);
                                    idx = 0;
                                }
#ifdef PEGASUS_ENABLE_IPV6
                                else
                                {
                                    addr_len = sizeof(SOCKADDR_IN6);
                                    idx = 1;
                                }
#endif

                                if (client->_local_addr_any)
                                {
                                    local_address.af = remote->sa_family;
                                    if (local_address.af == AF_INET)
                                    {
                                        local_address.ip4_addr.s_addr =
                                            INADDR_ANY;
                                    }
#ifdef PEGASUS_ENABLE_IPV6
                                    else
                                    {
                                        local_address.ip6_addr = in6addr_any;
                                    }
#endif
                                }
                                else
                                {
                                    PEGASUS_ASSERT(remote->sa_family ==
                                        client->_local_addr.af);
                                    local_address = client->_local_addr;
                                }

#ifdef PEGASUS_OS_ZOS
                                struct slp_if_addr* ptr_addr;
                                ptr_addr = client->_local_addr_list[idx];
                                while (ptr_addr->af != AF_UNSPEC)
                                {
                                        local_address = *ptr_addr;
#endif
                                    if (-1 != _slp_create_bind_socket(
                                        &sock,
                                        local_address.af,
                                        client->_target_port,
                                        &local_address.ip4_addr,
                                        TRUE))
                                    {
                                            _LSLP_SENDTO(
                                                sock,
                                                client->_msg_buf,
                                                total_len,
                                                0,
                                                (remote),
                                                addr_len);
                                    }
                                    _LSLP_CLOSESOCKET(sock);

#ifdef PEGASUS_OS_ZOS
                                    ptr_addr++;
                                    // end of while loop around all local
                                    // network interfaces
                                }
#endif
                                if (attr_tags)
                                {
                                    lslpFreeAttrList(attr_tags, TRUE);
                                }
                            } /* scopes intersect */
                            if (scopes)
                            {
                                lslpFreeScopeList(scopes);
                            }
                        } /* url string allocated */
                        if (url)
                        {
                            lslpFreeURL(url);
                        }
                    } /* url allocated */
                }
            } /* not on the pr list */
        }
    }
    DEBUG_PRINT((DEBUG_EXIT, "decode_attrreq:ok "));
}


/* folds extra whitespace out of the string - in place */
/* destroys parameter ! */
char *lslp_foldString(char *s)
{
    char *src, *dst;
    PEGASUS_ASSERT(s != 0);
    src = s;
    while (isspace(*src))
    {
        src++;
    }

    if (*src == 0x00)
    {
        /* return a string of exactly one space  */
        *s = 0x20;
        *(s + 1) = 0x00;
        return(s);
    }

    dst = s;
    while (*src != 0x00)
    {
        *dst = *src;
        dst++;
        src++;
        if (isspace(*src))
        {
            *dst = *src;
            dst++;
            src++;
            while (isspace(*src))
            {
                src++;
            }
        }
    }
    *dst = 0x00;
    if (isspace(*(dst - 1)))
    {
        *(dst - 1) = 0x00;
    }
    return(s);
}

BOOL lslpStuffAttrList(
    char **buf,
    int16 *len,
    lslpAttrList *list,
    lslpAttrList *include)
{

    lslpAttrList *attrs, *included;
    int16 attrLen = 0, lenSave;
    char *bptr;
    BOOL ccode = FALSE, hit = TRUE;
    PEGASUS_ASSERT(buf != NULL);
    PEGASUS_ASSERT(len != NULL && *len > 3);

    DEBUG_PRINT((DEBUG_ENTER, "lslpStuffAttrList "));

    /*   PEGASUS_ASSERT(list != NULL); */
    if (buf == NULL || len == NULL || *len < 3)
    {
        return(FALSE);
    }
    /* always return TRUE on an empty list so we can continue to build the */
    /* msg buffer - an empty list is not an error! */
    if (list == NULL || _LSLP_IS_EMPTY(list))
    {
        _LSLP_SETSHORT((*buf), 0, 0);
        (*buf) += 2;
        (*len) -= 2;
        DEBUG_PRINT((DEBUG_EXIT, "lslpStuffAttrList:ok "));
        return(TRUE);
    }
    /* attr list */
    lenSave = *len;
    attrs = list->next;
    bptr = *buf;

    /* <<< Fri May 14 17:13:22 2004 mdd >>>
       on some platforms memset must be called with a 4-byte aligned
       address as the buffer */
    /*  memset(bptr, 0x00, *len); */
    (*buf) += 2;     /* reserve space for the attrlist length short */
    (*len) -= 2;
    while (! _LSLP_IS_HEAD(attrs) && attrLen + 1 < *len)
    {
        PEGASUS_ASSERT(attrs->type != head);
        if (include != NULL && _LSLP_IS_HEAD(include) &&
            (!_LSLP_IS_EMPTY(include)))
        {
            included = include->next;
            hit = FALSE;

            while (! _LSLP_IS_HEAD(included))
            {
                if (! lslp_string_compare(attrs->name, included->name))
                {
                    hit = TRUE;
                    break;
                }
                included = included->next;
            }
        }

        if (hit == FALSE)
        {
            attrs = attrs->next;
            continue;
        }

        if (attrLen + (int16)strlen(attrs->name) + 3 < *len)
        {
            /* << Wed Jun  9 14:07:54 2004 mdd >> properly encode
               multi-valued attributes */
            if (_LSLP_IS_HEAD(attrs->prev) ||
                lslp_string_compare(attrs->prev->name, attrs->name))
            {

                if (attrs->type != tag)
                {
                    **buf = '(';
                    (*buf)++;
                    attrLen++;
                    (*len)--;
                }

                strcpy(*buf, attrs->name);
                (*buf) += strlen(attrs->name);
                attrLen += (int16)strlen(attrs->name);
                (*len) -= (int16)strlen(attrs->name);

                if (attrs->type != tag)
                {
                    **buf = '=';
                    (*buf)++;
                    attrLen++;
                    (*len)--;
                }
            } /* if not a multi-val */
            switch (attrs->type)
            {
                case tag:
                    ccode = TRUE;
                    break;

                case string:
                    if (attrLen + (int16)strlen(attrs->val.stringVal) + 2 <
                        *len)
                    {
                        strcpy(*buf, (attrs->val.stringVal));
                        (*buf) += strlen(attrs->val.stringVal);
                        attrLen +=  (int16)strlen(attrs->val.stringVal);
                        (*len) -= (int16)strlen(attrs->val.stringVal);
                        ccode = TRUE;
                    }
                    else
                    {
                        ccode = FALSE;
                    }
                    break;

                case integer:
                    if (attrLen + 33 + 2 < *len)
                    {
                        _itoa(attrs->val.intVal, *buf, 10);
                        attrLen += (int16)strlen(*buf);
                        (*buf) += strlen(*buf);
                        (*len) -= (int16)strlen(*buf);
                        ccode = TRUE;
                    }
                    else
                    {
                        ccode = FALSE;
                    }
                    break;

                case bool_type:
                    if (attrLen + 6 + 2 < *len)
                    {
                        if (attrs->val.boolVal)
                        {
                            strcpy(*buf, "TRUE");
                        }
                        else
                        {
                            strcpy(*buf, "FALSE");
                        }
                        attrLen += (int16)strlen(*buf);
                        (*buf) += strlen(*buf);
                        (*len) -= (int16)strlen(*buf);
                        ccode = TRUE;
                    }
                    else
                    {
                        ccode = FALSE;
                    }
                    break;

                case opaque:
                    {
                        int16 opLen;
                        opLen =
                            (_LSLP_GETSHORT(((char *)attrs->val.opaqueVal),0));
                        if (attrLen + opLen + 3 < *len)
                        {
                            memcpy(
                                *buf,
                                (((char*)attrs->val.opaqueVal)+2),
                                opLen);
                            (*buf) += opLen;
                            attrLen += opLen;
                            (*len) -= opLen;
                            ccode = TRUE;
                        }
                        else
                        {
                            ccode = FALSE;
                        }
                        break;
                    }
                default:
                    ccode = FALSE;
                    break;
            }
            if (ccode == TRUE && attrLen + 2 < *len && attrs->type != tag)
            {
                /* << Wed Jun  9 14:07:54 2004 mdd >>
                   properly encode multi-valued attributes */
                if (_LSLP_IS_HEAD(attrs->next) ||
                    lslp_string_compare(attrs->next->name, attrs->name))
                {
                    **buf = ')';
                    (*buf)++;
                    attrLen++;
                    (*len)--;
                }
            }
            if (ccode == TRUE && !_LSLP_IS_HEAD(attrs->next) && attrLen + 1 <
                *len)
            {
                **buf = ',';
                (*buf)++;
                attrLen++;
                (*len)--;
            }
        }  /* if room for the attr name */
        else
        {
            ccode = FALSE;
            break;
        }
        attrs = attrs->next;
    } /* while we are traversing the attr list */

    /* check for a trailing comma, which may end up in the buffer if we are */
    /* selecting attributes based upon a tag list */
    if (*buf && *(*buf - 1) == ',')
    {
        *(*buf - 1) = 0x00;
        attrLen -= 1;
    }

    /* set the length short */
    if (ccode == TRUE)
    {
        _LSLP_SETSHORT(bptr, attrLen, 0);
    }
    else
    {
        (*buf) = bptr;
        (*len) = lenSave;
        memset(*buf, 0x00, *len);
    }
    DEBUG_PRINT((DEBUG_EXIT, "lslpStuffAttrList:ok "));
    return(ccode);
}


lslpAttrList *lslpUnstuffAttr(char **buf, int16 *len, int16 *err)
{
    int16 tempLen;
    lslpAttrList *temp = NULL;
    PEGASUS_ASSERT(buf != NULL && *buf != NULL);
    PEGASUS_ASSERT(len != NULL);
    PEGASUS_ASSERT(err != NULL);
    *err = 0;

    DEBUG_PRINT((DEBUG_ENTER, "lslpUnstuffAttr "));

    tempLen = _LSLP_GETSHORT(*buf, 0);
    if (tempLen > 0)
    {
        (*buf) += sizeof(int16);
        (*len) -= sizeof(int16);
        if (*len >= tempLen)
        {
            if (NULL != (temp = _lslpDecodeAttrString(*buf)))
            {
                (*buf) += tempLen;
                (*len) -= tempLen;
            }
            else
            {
                *err = LSLP_PARSE_ERROR;
            }
        }
        else
        {
            *err = LSLP_INTERNAL_ERROR;
        }
    }
    DEBUG_PRINT((DEBUG_EXIT, "lslpStuffAttrList:ok "));
    return(temp);
}



lslpURL *lslpAllocURL()
{
    lslpURL *url = (lslpURL *)calloc(1, sizeof(lslpURL));

    return(url);
}

lslpURL *lslpAllocURLList()
{
    lslpURL *head;
    if (NULL != (head = lslpAllocURL()))
    {
        head->next = head->prev = head;
        head->isHead = TRUE;
        return(head);
    }
    return(NULL);
}

/* url MUST be unlinked from list */
void lslpFreeURL(lslpURL *url)
{
    PEGASUS_ASSERT(url != NULL);
    if (url->url != NULL)
    {
        free(url->url);
    }

    if (url->authBlocks != NULL)
    {
        lslpFreeAuthList(url->authBlocks);
    }

    if (url->atomized != NULL)
    {
        lslpFreeAtomizedURLList(url->atomized, TRUE);
    }

    if (url->attrs)
    {
        lslpFreeAtomList(url->attrs, LSLP_DESTRUCTOR_DYNAMIC);
    }

    free(url);
    return;
}

void lslpFreeURLList(lslpURL *list)
{
    lslpURL *temp;

    PEGASUS_ASSERT(list != NULL);
    PEGASUS_ASSERT(_LSLP_IS_HEAD(list));
    while (! (_LSLP_IS_EMPTY(list)))
    {
        temp = list->next;
        _LSLP_UNLINK(temp);
        lslpFreeURL(temp);
    }

    lslpFreeURL(list);
    return;
}


/* for protected scopes, we need to change stuff URL so that it */
/* only stuffs url auth blocks that match spis in the srv req */


/* this routine may return FALSE AND alter the buffer and length */
BOOL  lslpStuffURL(char **buf, int16 *len, lslpURL *url)
{
    PEGASUS_ASSERT((buf != NULL) && (*buf != NULL));
    PEGASUS_ASSERT((len != NULL) && (*len > 8));
    PEGASUS_ASSERT((url != NULL) && (! _LSLP_IS_HEAD(url)));

    DEBUG_PRINT((DEBUG_ENTER, "lslpStuffURL "));

    if (_LSLP_IS_HEAD(url))
    {
        return(FALSE);
    }

    memset(*buf, 0x00, *len);
    /* advanced past the reserved portion */
    (*buf) += sizeof(char);
    _LSLP_SETSHORT(*buf, url->lifetime - time(NULL), 0);
    (*buf) += sizeof(int16);
    url->len = (uint16)strlen(url->url);
    /* url->url is a null terminated string,
       but we only stuff the non-null bytes */
    _LSLP_SETSHORT(*buf, url->len, 0);

    (*buf) += sizeof(int16);
    (*len) -= 5;
    if (*len < url->len - 1)
    {
        return(FALSE);
    }

    memcpy(*buf, url->url, url->len);
    (*buf) += url->len;
    (*len) -= url->len;
    DEBUG_PRINT((DEBUG_EXIT, "lslpStuffURL "));
    return(lslpStuffAuthList(buf, len, url->authBlocks));
}

/* this routine may  return FALSE AND alter the buffer and length */
BOOL lslpStuffURLList(char **buf, int16 *len, lslpURL *list)
{
    BOOL ccode = TRUE;
    PEGASUS_ASSERT((buf != NULL) && (*buf != NULL));
    PEGASUS_ASSERT((len != NULL) && (*len > 8));
    PEGASUS_ASSERT((list != NULL) && (_LSLP_IS_HEAD(list)));

    DEBUG_PRINT((DEBUG_ENTER, "lslpStuffURLList "));

    if (! _LSLP_IS_HEAD(list))
    {
        return(FALSE);
    }

    while ((ccode == TRUE) && (! _LSLP_IS_HEAD(list->next)))
    {
        list = list->next;
        ccode = lslpStuffURL(buf, len, list);
    }
    DEBUG_PRINT((DEBUG_EXIT, "lslpStuffURLList:ok "));
    return(ccode);
}

lslpURL *lslpUnstuffURL(char **buf, int16 *len, int16 *err)
{

    char *tempurl;
    lslpURL *temp;
    PEGASUS_ASSERT(buf != NULL && *buf != NULL);
    PEGASUS_ASSERT(len != NULL && *len > 8);
    PEGASUS_ASSERT(err != NULL);
    *err = 0;

    DEBUG_PRINT((DEBUG_ENTER, "lslpUnstuffURL "));

    if (NULL != (temp = lslpAllocURL()))
    {
        temp->lifetime = _LSLP_GETSHORT((*buf), 1);
        temp->len = _LSLP_GETSHORT((*buf), 3);
        (*buf) += 5;
        (*len) -= 5;
        if (*len >= temp->len)
        {
            if (NULL != (temp->url = (char *)calloc(1, temp->len + 1)))
            {
                memcpy(temp->url, *buf, temp->len);
                *((temp->url) + temp->len) = 0x00;
                tempurl = temp->url;
                temp->atomized = _lslpDecodeURLs((char **)&tempurl, 1);
                (*buf) += temp->len;
                (*len) -= temp->len;
                if (temp->atomized != NULL)
                {
                    temp->authBlocks = lslpUnstuffAuthList(buf, len, err);
                }
            }
            else
            {
                *err = LSLP_INTERNAL_ERROR;
            }
        }
        else
        {
            *err = LSLP_PARSE_ERROR;
        }
    }
    else
    {
        *err = LSLP_INTERNAL_ERROR;
    }

    if (*err != 0 && temp != NULL)
    {
        lslpFreeURL(temp);
        temp = NULL;
    }
    DEBUG_PRINT((DEBUG_EXIT, "lslpUnstuffURL:ok "));
    return(temp);
}


void lslpFreeFilterTree(lslpLDAPFilter *root)
{
    PEGASUS_ASSERT(root != NULL);
    if (! _LSLP_IS_EMPTY( &(root->children)))
    {
        lslpFreeFilterTree((lslpLDAPFilter *)root->children.next);
    }

    if (! (_LSLP_IS_HEAD(root->next)) && (! _LSLP_IS_EMPTY(root->next)))
    {
        lslpFreeFilterTree(root->next);
    }

    if (root->attrs.next != NULL)
    {
        while (! (_LSLP_IS_EMPTY(&(root->attrs))))
        {
            lslpAttrList *attrs = root->attrs.next;
            _LSLP_UNLINK(attrs);
            lslpFreeAttr(attrs);
        }
    }
}


BOOL lslpEvaluateOperation(int compare_result, int operation)
{
    switch (operation)
    {
        case expr_eq:
            if (compare_result == 0)    /*  a == b */
            {
                return(TRUE);
            }
            break;

        case expr_gt:
            if (compare_result >= 0)    /*  a >= b  */
            {
                return(TRUE);
            }
            break;

        case expr_lt:            /* a <= b  */
            if (compare_result <= 0)
            {
                return(TRUE);
            }
            break;

        case expr_present:
        case expr_approx:
        default:
            return(TRUE);
            break;
    }
    return(FALSE);
}

/* evaluates attr values, not names */
BOOL lslpEvaluateAttributes(
    const lslpAttrList *filter,
    const lslpAttrList *registered,
    int op)
{
    PEGASUS_ASSERT(filter != NULL &&
        registered != NULL &&
        (! _LSLP_IS_HEAD(filter)) &&
        (! _LSLP_IS_HEAD(registered))) ;
    /* first ensure they are the same type  */
    switch (filter->type)
    {
        case opaque:
        case string:
            if (registered->type != string && registered->type != opaque)
            {
                return(FALSE);
            }

            if (registered->type == opaque || filter->type == opaque)
            {
                return(lslpEvaluateOperation(
                    memcmp(registered->val.stringVal,
                    filter->val.stringVal,
                    _LSLP_MIN(registered->attr_len,
                    (int32)strlen(filter->val.stringVal))), op));
            }
            else
            {
                if (TRUE == lslp_pattern_match(registered->val.stringVal,
                    filter->val.stringVal,
                    FALSE))
                {
                    return(lslpEvaluateOperation(0, op));
                }
                else
                {
                    return(lslpEvaluateOperation(1, op));
                }
            }
            break;

        case integer:
            return(
                lslpEvaluateOperation(
                filter->val.intVal - registered->val.intVal,
                op));

        case tag:   /* equivalent to a presence test  */
            return(TRUE);

        case bool_type:
            if ((filter->val.boolVal != 0) && (registered->val.boolVal != 0))
            {
                return(TRUE);
            }
            if ((filter->val.boolVal == 0) && (registered->val.boolVal == 0))
            {
                return(TRUE);
            }
            break;
        default:
            break;
    }
    return(FALSE);
}

/* filter is a filter tree, attrs is ptr to an attr listhead */

BOOL lslpEvaluateFilterTree(lslpLDAPFilter *filter, const lslpAttrList *attrs)
{
    PEGASUS_ASSERT(filter != NULL);
    PEGASUS_ASSERT(attrs != NULL);
    PEGASUS_ASSERT(! _LSLP_IS_HEAD(filter));
    if (filter == NULL || (_LSLP_IS_HEAD(filter)) || attrs == NULL)
    {
        return FALSE;
    }

    if (! _LSLP_IS_HEAD(filter->children.next))
    {
        lslpEvaluateFilterTree((lslpLDAPFilter *)filter->children.next, attrs);
    }

    if (! (_LSLP_IS_HEAD(filter->next)) && (! _LSLP_IS_EMPTY(filter->next)))
    {
        lslpEvaluateFilterTree(filter->next, attrs);
    }

    if (filter->_operator == ldap_and ||
        filter->_operator == ldap_or ||
        filter->_operator == ldap_not)
    {
        /* evaluate ldap logical operators by evaluating filter->children
           as a list of filters */
        lslpLDAPFilter *child_list = (lslpLDAPFilter *)filter->children.next;
        /* initialize  the filter's logical value to TRUE */
        if (filter->_operator == ldap_or)
        {
            filter->logical_value = FALSE;
        }
        else
        {
            filter->logical_value = TRUE;
        }

        while (! _LSLP_IS_HEAD(child_list))
        {
            if (child_list->logical_value == TRUE)
            {
                if (filter->_operator == ldap_or)
                {
                    filter->logical_value = TRUE;
                    break;
                }

                if (filter->_operator == ldap_not)
                {
                    filter->logical_value = FALSE;
                    break;
                }
                /* for an & operator keep going  */
            }
            else
            {
                /* child is FALSE */
                if (filter->_operator == ldap_and)
                {
                    filter->logical_value = FALSE;
                    break;
                }
            }
            child_list = child_list->next;
        }
    }
    else
    {
        /* find the first matching attribute and set the logical value */
        filter->logical_value = FALSE;
        if (! _LSLP_IS_HEAD(filter->attrs.next))
        {

            /* need to continue to search until the attr list end to support
               multi-valued attributes */

            attrs = attrs->next;
            while (! _LSLP_IS_HEAD(attrs))
            {
                /* advance to a matching attribute name if it exists */
                while ((! _LSLP_IS_HEAD(attrs )) &&
                    (FALSE  == lslp_pattern_match2(filter->attrs.next->name,
                    attrs->name, FALSE)))
                {
                    attrs = attrs->next ;
                }
                /* either we have traversed the list or found the first
                   matching attribute */
                if (! _LSLP_IS_HEAD(attrs))
                {
                    /* we found the first matching attribute,
                       now do the comparison */
                    if (filter->_operator == expr_present ||
                        filter->_operator == expr_approx)
                    {
                        filter->logical_value = TRUE;
                    }
                    else
                    {
                        filter->logical_value =
                            lslpEvaluateAttributes(
                            filter->attrs.next,
                            attrs,
                            filter->_operator);
                    }
                }
                else
                {
                    break;
                } /* did not find a matching attribute */

                /* if we found a match, exit the comparison loop */
                if (filter->logical_value == TRUE)
                {
                    break;
                }
                /* no match, continue searching attributes. */
                attrs = attrs->next;
            }
        }
    }
    return(filter->logical_value);
}

lslpScopeList *lslpScopeStringToList(const char *s, int16 len)
{
    char *p, *saveP, *t;
    lslpScopeList *h, *temp;
    if (s == NULL)
    {
        return(lslpAllocScopeList());
    }

    if (NULL != (h  = lslpAllocScopeList()))
    {
        saveP = (p = (char *)malloc(len + 1));
        if (p == NULL)
        {
            return NULL;
        }
        memcpy(p, s, len);
        *(p + len) = 0x00;

        while (NULL != (t = strchr(p, ',')))
        {
            *t = 0x00;
            t++;
            p = lslp_foldString(p);
            if ((lslp_islegal_scope(p)) && (NULL != (temp = lslpAllocScope())))
            {
                if (NULL != (temp->scope = strdup(p)))
                {
                    _LSLP_INSERT(temp, h);
                }
            }
            p = t;
        }
        p = lslp_foldString(p);
        if ((lslp_islegal_scope(p)) && (NULL != (temp = lslpAllocScope())))
        {
            if (NULL != (temp->scope = strdup(p)))
            {
                _LSLP_INSERT(temp, h);
            }
        }
        free(saveP);
    }
    return(h);
}

BOOL lslpStuffScopeList(char **buf, int16 *len, lslpScopeList *list)
{
    lslpScopeList *scopes;
    int16 scopeLen = 0, lenSave;
    char *bptr;
    BOOL ccode = FALSE;
    PEGASUS_ASSERT(buf != NULL);
    PEGASUS_ASSERT(len != NULL && *len > 3);
    PEGASUS_ASSERT(list != NULL);

    if (buf == NULL || len == NULL || *len < 3 || list == NULL)
    {
        return(FALSE);
    }

    /* always return TRUE on an empty list so we can continue to build the */
    /* msg buffer - an empty list is not an error! */
    if (_LSLP_IS_EMPTY(list))
    {
        return(TRUE);
    }

    /* scope list */
    lenSave = *len;
    scopes = list->next;
    bptr = *buf;
    memset(*buf, 0x00, *len);
    (*buf) += 2;     /* reserve space for the scopelist length short */
    (*len) -= 2;
    while (! _LSLP_IS_HEAD(scopes) && scopeLen + 1 < *len)
    {
        if (scopeLen + (int16)strlen(scopes->scope) < *len)
        {
            ccode = TRUE;
            strcpy(*buf, scopes->scope);
            (*buf) += strlen(scopes->scope);
            scopeLen += (int16)strlen(scopes->scope) ;
            if (! _LSLP_IS_HEAD(scopes->next))
            {
                **buf = ',';
                (*buf)++;
                scopeLen++;
            }
        }  /* if room for the attr name */
        else
        {
            ccode = FALSE;
            break;
        }
        scopes = scopes->next;
    } /* while we are traversing the attr list */
    /* set the length short */
    if (ccode == TRUE)
    {
        //      **buf = 0x00;
        //      (*buf)++;
        //      scopeLen++;
        //      << Thu May 30 16:18:57 2002 mdd >>
        (*len) -= scopeLen;
        _LSLP_SETSHORT(bptr, scopeLen, 0);
    }
    else
    {
        *len = lenSave;
        *buf = bptr;
        memset(*buf, 0x00, *len);
    }
    return(ccode);

}


lslpScopeList *lslpUnstuffScopeList(char **buf, int16 *len, int16 *err)
{
    int16 tempLen;
    lslpScopeList *temp = NULL;
    PEGASUS_ASSERT(buf != NULL && *buf != NULL);
    PEGASUS_ASSERT(len != NULL);
    PEGASUS_ASSERT(err != NULL);
    *err = 0;
    tempLen = _LSLP_GETSHORT(*buf, 0);
    (*buf += 2);
    (*len -= 2);
    if (tempLen != 0)
    {
        if (tempLen <= *len)
        {
            if (NULL != (temp = lslpScopeStringToList(*buf, tempLen)))
            {
                (*buf) += tempLen;
                (*len) -= tempLen;
            }
            else
            {
                *err = LSLP_INTERNAL_ERROR;
            }
        }
        else
        {
            *err = LSLP_PARSE_ERROR;
        }
    }
    return(temp);
}


lslpSPIList *lslpUnstuffSPIList(char **buf, int16 *len, int16 *err)
{
    return((lslpSPIList *)lslpUnstuffScopeList(buf, len, err));
}


BOOL lslpStuffSPIList(char **buf, int16 *len, lslpSPIList *list)
{
    return(lslpStuffScopeList(buf, len, (lslpScopeList *)list));
}


BOOL lslp_scope_intersection(lslpScopeList *a, lslpScopeList *b)
{
    if ((a == NULL) || (b == NULL))
    {
        return(TRUE);
    }

    PEGASUS_ASSERT(_LSLP_IS_HEAD(a));
    PEGASUS_ASSERT(_LSLP_IS_HEAD(b));
    if ((_LSLP_IS_EMPTY(a)) || (_LSLP_IS_EMPTY(b)))
    {
        return(FALSE);
    }

    while (! (_LSLP_IS_HEAD(a->next)))
    {
        a = a->next;
        while (!(_LSLP_IS_HEAD(b->next)))
        {
            b = b->next;
            PEGASUS_ASSERT((a->scope != NULL) && (b->scope != NULL));
#ifndef PEGASUS_OS_ZOS
            if (! strcasecmp(a->scope, b->scope))
            {
#else
            int alength_of_string = strlen(a->scope);
            int blength_of_string = strlen(a->scope);

            if (alength_of_string > blength_of_string)
            {
                blength_of_string = alength_of_string;
            }
            if (! strncasecmp(a->scope, b->scope, blength_of_string))
            {
#endif
                return(TRUE);
            }
        }
        /* reset b */
        b = b->next;
    }
    return(FALSE);
}

/* caseless compare that folds whitespace */
int lslp_string_compare(char *s1, char *s2)
{
    PEGASUS_ASSERT(s1 != NULL);
    PEGASUS_ASSERT(s2 != NULL);
    lslp_foldString(s1);
    lslp_foldString(s2);
    if (TRUE == lslp_pattern_match2(s1, s2, FALSE))
    {
        return 0;
    }
    return -1;
}

/* return 1 if char is legal for scope value, 0 otherwise */
int lslp_isscope(int c)
{
    int i;
    static char scope_reserved[] =
    { '(', ')', ',', 0x5c, '!', '<',
        '=', '>', '~', ';', '*', '+', 0x7f};

    if (! _LSLP_ISASCII(c))
    {
        return 0;
    }

    if (c < 0x20)
    {
        return(0);
    }

    for (i = 0; i < 13; i++)
    {
        if (c == scope_reserved[i])
        {
            return(0);
        }
    }
    return(1);
}

BOOL lslp_islegal_scope(char *s)
{
    char *temp;
    PEGASUS_ASSERT(s != NULL);
    if (! strlen(s))
    {
        return(FALSE);
    }
    temp = s;
    while (*temp != 0x00)
    {
        if (! lslp_isscope(*temp))
        {
            return(FALSE);
        }
        temp++;
    }
    return(TRUE);
}

BOOL lslp_srvtype_match(const char *s, char *r)
{
    char *sp;
    PEGASUS_ASSERT(s != NULL);
    PEGASUS_ASSERT(r != NULL);
    /* check to see if search is for an abstract service type */
    sp = (char *)r;
    while (*sp != 0x00)
    {
        sp++;
    }
    /* if this is a search for an abstract service type, use a wildcard */
    /* to collect all concrete registrations under this type */
    if (*(sp - 1) == ':')
    {
        *(sp - 1) = '*';
    }
    /* check for a dangling semicolon and kill it  */
    else if (*(sp - 1) == ';')
    {
        *(sp - 1) = 0x00;
    }

    return(lslp_pattern_match(s, r, FALSE));
}


lslpScopeList *lslpAllocScope()
{
    return((lslpScopeList *)calloc(1, sizeof(lslpScopeList)));
}

lslpScopeList *lslpAllocScopeList()
{
    lslpScopeList *head;
    if (NULL != (head = lslpAllocScope()))
    {
        head->next = head->prev = head;
        head->isHead = TRUE;
    }
    return(head);
}

void lslpFreeScope(lslpScopeList *s)
{
    PEGASUS_ASSERT(s != NULL);
    if (s->scope != NULL)
    {
        free(s->scope);
    }
    free(s);
}

void lslpFreeScopeList(lslpScopeList *head)
{
    lslpScopeList *temp;
    PEGASUS_ASSERT(head != NULL);
    PEGASUS_ASSERT(_LSLP_IS_HEAD(head));
    while (! (_LSLP_IS_EMPTY(head)))
    {
        temp = head->next;
        _LSLP_UNLINK(temp);
        lslpFreeScope(temp);
    }
    lslpFreeScope(head);
    return;
}

lslpSrvRegList *lslpAllocSrvReg()
{
    lslpSrvRegList *l;
    if (NULL != (l = (lslpSrvRegList *)calloc(1, sizeof(lslpSrvRegList))))
    {
        if (NULL != (l->url = lslpAllocURL()))
        {
            if (NULL != (l->scopeList = lslpAllocScopeList()))
            {
                if (NULL != (l->attrList = lslpAllocAttrList()))
                {
                    if (NULL != (l->authList = lslpAllocAuthList()))
                    {
                        return(l);
                    }
                    lslpFreeAttrList(l->attrList, TRUE);
                }
                lslpFreeScopeList(l->scopeList);
            }
            lslpFreeURL(l->url);
        }
        free(l);
    }
    return(NULL);
}

lslpSrvRegHead *lslpAllocSrvRegList()
{
    lslpSrvRegHead *head;
    if (NULL != (head = (lslpSrvRegHead *)calloc(1, sizeof(lslpSrvRegHead))))
    {
        head->next = head->prev = (struct lslp_srv_reg_instance *)head;
        head->isHead = TRUE;
        return(head);
    }
    return(NULL);
}

void lslpFreeSrvReg(lslpSrvRegList *s)
{
    PEGASUS_ASSERT(s != NULL);
    if (s->url != NULL)
    {
        lslpFreeURL(s->url);
    }
    if (s->srvType != NULL)
    {
        free(s->srvType);
    }
    if (s->scopeList != NULL)
    {
        lslpFreeScopeList(s->scopeList);
    }
    if (s->attrList != NULL)
    {
        lslpFreeAttrList(s->attrList, TRUE);
    }
    if (s->authList != NULL)
    {
        lslpFreeAuthList(s->authList);
    }
    free(s);
}

void lslpFreeSrvRegList(lslpSrvRegHead *head)
{
    lslpSrvRegList *temp;
    PEGASUS_ASSERT(head != NULL);
    PEGASUS_ASSERT(_LSLP_IS_HEAD(head));
    while (!  (_LSLP_IS_EMPTY((struct lslp_srv_reg_instance *)head)))
    {
        temp = head->next;
        _LSLP_UNLINK(temp);
        lslpFreeSrvReg(temp);
    }
}


lslpAuthBlock *lslpAllocAuthBlock()
{
    return((lslpAuthBlock *)calloc(1, sizeof(lslpAuthBlock)));
}

lslpAuthBlock *lslpAllocAuthList()
{
    lslpAuthBlock *head;
    if (NULL != (head = lslpAllocAuthBlock()))
    {
        head->next = head->prev = head;
        head->isHead = TRUE;
        return(head);
    }
    return(NULL);
}


/* NOTE: be CERTAIN block is not linked to a list !!! */
void lslpFreeAuthBlock(lslpAuthBlock *auth)
{
    if (auth->spi != NULL)
    {
        free(auth->spi);
    }
    if (auth->block != NULL)
    {
        free(auth->block);
    }
    free(auth);
    return;
}

void lslpFreeAuthList(lslpAuthBlock *list)
{
    lslpAuthBlock *temp;

    PEGASUS_ASSERT(list != NULL);
    PEGASUS_ASSERT(_LSLP_IS_HEAD(list));
    while (! (_LSLP_IS_EMPTY(list)))
    {
        temp = list->next;
        _LSLP_UNLINK(temp);
        lslpFreeAuthBlock(temp);
    }
    lslpFreeAuthBlock(list);
    return;
}

/* for protected scopes, we need to change this routine (or add another one)
   that uses an SPI list as an input and stuffs only auth only stuffs
   auth blocks that match spis in the input list */

/* this routine will return FALSE AND alter the buffer and length */
BOOL lslpStuffAuthList(char **buf, int16 *len, lslpAuthBlock *list)
{
    char *bptr;
    int16 numAuths = 0;
    lslpAuthBlock *auths;
    BOOL ccode = TRUE;
    PEGASUS_ASSERT(buf != NULL);
    PEGASUS_ASSERT(len != NULL);
    /*   PEGASUS_ASSERT(list != NULL); */
    if (buf == NULL || *buf == NULL || len == NULL)
    {
        return(FALSE);
    }
    /* always return true on an empty list so we can continue to build the */
    /* msg buffer - an empty list is not an error! */
    bptr = *buf;
    _LSLP_SETBYTE(bptr, numAuths, 0);

    (*buf)+= 1;
    (*len) -= 1;
    if (list == NULL || _LSLP_IS_EMPTY(list))
    {
        return(TRUE);
    }
    /* auth list */
    auths  = list->next;
    memset(*buf, 0x00, *len);
    while (! _LSLP_IS_HEAD(auths) && auths->len <= *len)
    {
        /* store the auth bsd */
        _LSLP_SETSHORT(*buf, auths->descriptor, 0);
        /* store the the bsd size */
        _LSLP_SETSHORT(*buf, auths->len, 2);
        /* store the timestamp */
        _LSLP_SETLONG(*buf, auths->timestamp, 4);
        /* store spi string length */
        _LSLP_SETSHORT(*buf, auths->spiLen, 8);
        PEGASUS_ASSERT(strlen(auths->spi) + 1 == auths->spiLen );
        /* store the spi string */
        strcpy((*buf) + 10, auths->spi);
        /* store the authentication block */
        if (auths->block != NULL && (auths->len - (auths->spiLen + 10) ) > 0)
        {
            memcpy(
                ((*buf) + 10 + auths->spiLen),
                auths->block,
                (auths->len - (auths->spiLen + 10)));
        }
        (*buf) += auths->len;
        (*len) -= auths->len;
        numAuths++;
        auths = auths->next;
    } /* while we are traversing the attr list */
    if (! _LSLP_IS_HEAD(auths))
    {
        /* we terminated the loop before copying all the auth blocks */
        ccode = FALSE;
    }
    else
    {
        _LSLP_SETBYTE(bptr, numAuths, 0);
    }
    return(ccode);
}

lslpAuthBlock *lslpUnstuffAuthList(char **buf, int16 *len, int16 *err)
{
    int16 tempLen;
    lslpAuthBlock *list = NULL, *temp;
    PEGASUS_ASSERT(buf != NULL && *buf != NULL);
    PEGASUS_ASSERT(len != NULL);
    PEGASUS_ASSERT(err != NULL);
    *err = 0;
    tempLen = _LSLP_GETBYTE(*buf, 0);     /* get the number of auth-blocks */
    (*buf) += 1;                            /* advance to the auth blocks */
    (*len) -= 1;
    if ((tempLen == 0) || (NULL != (list = lslpAllocAuthList())))
    {
        if (list != NULL)
        {
            while ((tempLen > 0) && (*len > 10) && (*err == 0))
            {
                if (NULL != (temp = lslpAllocAuthBlock()))
                {
                    /* get the block structure descriptor */
                    temp->descriptor = _LSLP_GETSHORT(*buf, 0);
                    (*buf) += 2;            /* advance to the block length */
                    temp->len = _LSLP_GETSHORT(*buf, 0);/* get block length */
                    (*buf) += 2;            /* advance to the timestamp */
                    temp->timestamp = _LSLP_GETLONG(*buf, 0);/* get timestamp*/
                    (*buf) += 4;            /* advance to the spi length */
                    temp->spiLen = _LSLP_GETSHORT(*buf, 0);/* get spi length*/
                    (*buf) += 2;            /* advance to the spi */
                    *len -= 10;
                    if (*len >= (temp->spiLen))
                    {
#if defined( PEGASUS_OS_ZOS ) || defined(PEGASUS_OS_SOLARIS)
                        if (NULL !=
                            (temp->spi =
                            (char*)calloc(temp->spiLen + 1,sizeof(char))))
                        {
#else
                        if (NULL !=
                            (temp->spi =
                            (char *)calloc(temp->spiLen + 1,sizeof(char))))
                        {
#endif
                            memcpy(temp->spi, *buf, temp->spiLen);/* copy spi*/
                            (*buf) += temp->spiLen;/* advance to next block */
                            (*len) -= temp->spiLen;
                            if (*len >= (temp->len - (10 + temp->spiLen)))
                            {
                                if (NULL != (temp->block =
#if defined( PEGASUS_OS_ZOS ) || defined( PEGASUS_OS_SOLARIS)
                                    (char *)calloc(
                                    (temp->len - (10 + temp->spiLen)) + 1,
#else
                                    (char *)calloc(
                                    (temp->len - (10 + temp->spiLen)) + 1,
#endif
                                    sizeof(char))))
                                {
                                    memcpy(
                                        temp->block,
                                        *buf,
                                        (temp->len - (10 + temp->spiLen)));
                                    /* insert the auth block into the list */
                                    _LSLP_INSERT(temp, list);
                                    (*buf) +=
                                        (temp->len - (10 + temp->spiLen));
                                    (*len) -=
                                        (temp->len - (10 + temp->spiLen));
                                    temp = NULL;
                                }
                                else
                                {
                                    /*if we alloced the auth block buffer*/
                                    *err = LSLP_INTERNAL_ERROR;
                                }
                            }
                            else
                            {
                                *err = LSLP_PARSE_ERROR;
                            }
                        }
                        else
                        {
                            *err = LSLP_INTERNAL_ERROR;
                        }/* if we alloced the spi buffer*/
                    }
                    else
                    {
                        *err = LSLP_PARSE_ERROR;
                    }
                }
                else
                {
                    *err = LSLP_INTERNAL_ERROR;
                } /* if we alloced the auth block */
                tempLen--; /* decrement the number of auth blocks */
            } /* while there is room and there are auth blocks to process */
        }
        else
        {
            if (tempLen == 0)
            {
                *err = 0;
            }
            else
            {
                *err = LSLP_INTERNAL_ERROR;
            }
        }
    }
    if (*err != 0 && list != NULL)
    {
        lslpFreeAuthList(list);
        list = NULL;
    }
    return(list);
}


uint32 lslpCheckSum(char *s, int16 l)
{
    uint32 c;
    BOOL pad = 0;
    uint16 a = 0, b = 0, *p;
    PEGASUS_ASSERT(s != NULL);
    if (l == 1)
    {
        return(0);
    }

    PEGASUS_ASSERT(l > 1);
    if (l & 0x01)
    {
        pad = TRUE;
    }

    p = (uint16 *)s;

    while (l > 1)
    {
        a = (a + *p++) & 0xFFFF;
        b = (b + a) & 0xFFFF;
        l -= 2;
    }
    /* "pad" the string with a zero word */
    if (pad == TRUE)
    {
        b = (b + a) & 0xFFFF;
    }
    /* return the value as a dword with containing two shorts in */
    /* network order -- ab */
    _LSLP_SETSHORT((uint8 *)&c, a, 0);
    _LSLP_SETSHORT((uint8 *)&c, a, 2);
    return(c);
}


lslpHdr * lslpAllocHdr()
{
    return((lslpHdr *)calloc(1, sizeof(lslpHdr)));
}

void lslpFreeHdr(lslpHdr *hdr)
{
    PEGASUS_ASSERT(hdr != NULL);
    if (hdr->data != NULL)
    {
        free(hdr->data);
    }
    free(hdr);
}

void lslpDestroySrvReq(struct lslp_srv_req *r, char flag)
{
    PEGASUS_ASSERT(r != NULL);
    if (r->prList != NULL)
    {
        free(r->prList);
    }
    if (r->srvcType != NULL)
    {
        free(r->srvcType);
    }
    if (r->scopeList != NULL)
    {
        lslpFreeScopeList(r->scopeList);
    }
    if (r->predicate != NULL)
    {
        free(r->predicate);
    }
    if (r->spiList != NULL)
    {
        lslpFreeSPIList(r->spiList);
    }
    if (flag == LSLP_DESTRUCTOR_DYNAMIC)
    {
        free(r);
    }
}

void lslpDestroySrvRply(struct lslp_srv_rply *r, char flag)
{
    PEGASUS_ASSERT(r != NULL);
    if (r->urlList != NULL)
    {
        lslpFreeURLList(r->urlList);
    }
    if (r->attr_list != NULL)
    {
        lslpFreeAtomList(r->attr_list, LSLP_DESTRUCTOR_DYNAMIC);
    }
    if (flag == LSLP_DESTRUCTOR_DYNAMIC)
    {
        free(r);
    }
}

void lslpDestroySrvReg(struct lslp_srv_reg *r, char flag)
{
    PEGASUS_ASSERT(r != NULL);
    if (r->url != NULL)
    {
        lslpFreeURLList(r->url);
    }
    if (r->srvType != NULL)
    {
        free(r->srvType);
    }
    if (r->scopeList != NULL)
    {
        lslpFreeScopeList(r->scopeList);
    }
    if (r->attrList != NULL)
    {
        lslpFreeAttrList(r->attrList, TRUE);
    }
    if (r->attrAuthList != NULL)
    {
        lslpFreeAuthList(r->attrAuthList);
    }
    if (flag == LSLP_DESTRUCTOR_DYNAMIC)
    {
        free(r);
    }
}

void lslpDestroySrvAck(struct lslp_srv_ack *r, char flag)
{
    PEGASUS_ASSERT(r != NULL);
    if (flag == LSLP_DESTRUCTOR_DYNAMIC)
    {
        free(r);
    }
}

void lslpDestroyDAAdvert(struct lslp_da_advert *r, char flag)
{
    PEGASUS_ASSERT(r != NULL);
    if (r->url != NULL)
    {
        free(r->url);
    }
    if (r->scopeList != NULL)
    {
        free(r->scopeList);
    }
    if (r->attrList != NULL)
    {
        free(r->attrList);
    }
    if (r->spiList != NULL)
    {
        free(r->spiList);
    }
    if (r->authBlocks != NULL)
    {
        lslpFreeAuthList(r->authBlocks);
    }
    if (flag == LSLP_DESTRUCTOR_DYNAMIC)
    {
        free(r);
    }
}

void lslpDestroySAAdvert(struct lslp_sa_advert *r, char flag)
{
    PEGASUS_ASSERT(r != NULL);
    if (r->url != NULL)
    {
        free(r->url);
    }
    if (r->scopeList != NULL)
    {
        free(r->scopeList);
    }
    if (r->attrList != NULL)
    {
        free(r->attrList);
    }
    if (r->authBlocks != NULL)
    {
        lslpFreeAuthList(r->authBlocks);
    }
    if (flag == LSLP_DESTRUCTOR_DYNAMIC)
    {
        free(r);
    }
}

void lslpDestroySrvTypeReq(struct lslp_srvtype_req *r, char flag)
{
    PEGASUS_ASSERT(r != NULL);
    if (r->prList != NULL)
    {
        free(r->prList);
    }
    if (r->nameAuth != NULL)
    {
        free(r->nameAuth);
    }
    if (r->scopeList != NULL)
    {
        lslpFreeScopeList(r->scopeList);
    }
    if (flag == LSLP_DESTRUCTOR_DYNAMIC)
    {
        free(r);
    }
    return;
}

void lslpDestroySrvTypeReply(struct lslp_srvtype_rep *r, char flag)
{
    PEGASUS_ASSERT(r != NULL);
    if (r->srvTypeList != NULL)
    {
        free(r->srvTypeList);
    }
    if (flag == LSLP_DESTRUCTOR_DYNAMIC)
    {
        free(r);
    }
    return;
}

void lslpDestroyAttrReq(struct lslp_attr_req *r, char flag)
{
    PEGASUS_ASSERT(r != NULL);
    if (r->prList != NULL)
    {
        free(r->prList);
    }
    if (r->url != NULL)
    {
        free(r->url);
    }
    if (r->scopeList != NULL)
    {
        free(r->scopeList);
    }
    if (r->tagList != NULL)
    {
        free(r->tagList);
    }
    if (r->spiList != NULL)
    {
        lslpFreeSPIList(r->spiList);
    }
    if (flag == LSLP_DESTRUCTOR_DYNAMIC)
    {
        free(r);
    }
    return;
}

void lslpDestroyAttrReply(struct lslp_attr_rep *r, char flag)
{
    PEGASUS_ASSERT(r != NULL);
    if (r->attrList != NULL)
    {
        free(r->attrList);
    }
    if (r->authBlocks != NULL)
    {
        lslpFreeAuthList(r->authBlocks);
    }
    if (flag == LSLP_DESTRUCTOR_DYNAMIC)
    {
        free(r);
    }
    return;
}

void lslpDestroySrvDeReg(struct lslp_srv_dereg *r, char flag)
{
    PEGASUS_ASSERT(r != NULL);
    if (r->scopeList != NULL)
    {
        free(r->scopeList);
    }
    if (r->urlList != NULL)
    {
        lslpFreeURLList(r->urlList);
    }
    if (r->tagList != NULL)
    {
        free(r->tagList);
    }
    if (flag == LSLP_DESTRUCTOR_DYNAMIC)
    {
        free(r);
    }
    return;
}


lslpMsg *alloc_slp_msg(BOOL head)
{
    lslpMsg *ret = (lslpMsg *)calloc(1, sizeof(lslpMsg));
    if (ret != NULL)
    {
        ret->dynamic = LSLP_DESTRUCTOR_DYNAMIC;
        if (head == TRUE)
        {
            ret->next = ret->prev = ret;
            ret->isHead = TRUE;
        }
    }
    return ret;
}


void lslpDestroySLPMsg(lslpMsg *msg)
{
    PEGASUS_ASSERT(msg != NULL);
    switch (msg->type)
    {
        case srvReq:
            lslpDestroySrvReq(&(msg->msg.srvReq), LSLP_DESTRUCTOR_STATIC);
            break;

        case srvRply:
            lslpDestroySrvRply(&(msg->msg.srvRply), LSLP_DESTRUCTOR_STATIC);
            break;

        case srvReg:
            lslpDestroySrvReg(&(msg->msg.srvReg), LSLP_DESTRUCTOR_STATIC);
            break;

        case srvAck:
            lslpDestroySrvAck(&(msg->msg.srvAck), LSLP_DESTRUCTOR_STATIC);
            break;

        case daAdvert:
            lslpDestroyDAAdvert(&(msg->msg.daAdvert), LSLP_DESTRUCTOR_STATIC);
            break;

        case saAdvert:
            lslpDestroySAAdvert(&(msg->msg.saAdvert), LSLP_DESTRUCTOR_STATIC);
            break;

        case srvTypeReq:
            lslpDestroySrvTypeReq(
                &(msg->msg.srvTypeReq),
                LSLP_DESTRUCTOR_STATIC);
            break;

        case srvTypeRep:
            lslpDestroySrvTypeReply(
                &(msg->msg.srvTypeRep),
                LSLP_DESTRUCTOR_STATIC);
            break;

        case attrReq:
            lslpDestroyAttrReq(&(msg->msg.attrReq), LSLP_DESTRUCTOR_STATIC);
            break;

        case attrRep:
            lslpDestroyAttrReply(&(msg->msg.attrRep), LSLP_DESTRUCTOR_STATIC);
            break;

        case srvDereg:
            lslpDestroySrvDeReg(&(msg->msg.srvDereg), LSLP_DESTRUCTOR_STATIC);
            break;

        default:
            break;
    }
    if (msg->dynamic == LSLP_DESTRUCTOR_DYNAMIC)
    {
        free(msg);
    }
    return;
}

/* a is an attribute list, while b is a string representation of an
   ldap filter */
BOOL lslp_predicate_match(lslpAttrList *a, char *b)
{
    BOOL ccode;
    lslpLDAPFilter *ftree;
    if (a == NULL)
    {
        return FALSE;
    }
    PEGASUS_ASSERT(_LSLP_IS_HEAD(a));
    if (b == NULL || ! strlen(b))
    {
        return(TRUE);        /*  no predicate - aways tests TRUE  */
    }
    if (NULL != (ftree = _lslpDecodeLDAPFilter(b)))
    {
        ccode = lslpEvaluateFilterTree(ftree, a);
        lslpFreeFilterTree(ftree);
        return(ccode);
    }
    return(FALSE);
}

struct lslp_srv_rply_out *_lslpProcessSrvReq(
    struct slp_client *client,
    struct lslp_srv_req *msg,
    int16 errCode,
    SOCKADDR *remote)
{
    char *buf;
    int16 bufLen, avail;
    lslpSrvRegList *reg;
    int16 ext_offset;
    char *extptr, *next_extptr;
    BOOL pile_up_attrs = FALSE;
    char url_addr[PEGASUS_IN6_ADDR_SIZE];

    struct lslp_srv_rply_out *temp_rply =
        (struct lslp_srv_rply_out *)calloc(
        1,
        sizeof(struct lslp_srv_rply_out));

    DEBUG_PRINT((DEBUG_ENTER, "_lslpProcessSrvReq "));

    PEGASUS_ASSERT(msg != NULL);
    if (msg != NULL && temp_rply != NULL)
    {
        if ((temp_rply->errCode = errCode) == 0)
        {
            buf = (char *)calloc(LSLP_MTU, sizeof(char));
            if (buf != NULL)
            {
                bufLen = LSLP_MTU;
                temp_rply->urlList = buf;
                buf += 4;
                avail = LSLP_MTU - 4;

                /* check for the attr list extension */
                /* remember to subtract 5 bytes from the limit for each
                   extension we traverse */


/*    The format of the Attribute List Extension is as follows: */

/*       0                   1                   2                   3 */
/*       0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 */
/*      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ */
/*      |      Extension ID = 0x0002    |     Next Extension Offset     | */
/*      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ */
/*      | Offset, contd.|      Service URL Length       |  Service URL  / */
/*      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ */
/*      |     Attribute List Length     |         Attribute List        / */
/*      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ */
/*      |# of AttrAuths |(if present) Attribute Authentication Blocks.../ */
/*      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ */

/*    The Extension ID is 0x0002. */




                if (msg->next_ext != NULL)
                {
                    /* we are looking for extension ID 0x0002 */
                    extptr = msg->next_ext;
                    while (extptr && extptr < msg->ext_limit)
                    {
                        char * extptr_save = extptr;
                        if (0x0002 == _LSLP_GETSHORT(extptr, 0))
                        {
                            /* make sure it doesn't overflow */
                            if (extptr + 9 > msg->ext_limit)
                            {
                                break;
                            }
                            /* got our extension */
                            /* do a sanity check offset five should be 0x0000,
                               offset 7 should be 0x0000*/
                            if ((0x0000 == _LSLP_GETSHORT(extptr, 5)) &&
                                (0x0000 == _LSLP_GETSHORT(extptr, 7)))
                            {
                                /* ok, accumulate attribute data */
                                pile_up_attrs = TRUE;
                            }
                            break;
                        }
                        /* see if there are any more extensions */
                        extptr += _LSLP_GET3BYTES(extptr, 2);
                        if (extptr == extptr_save)
                        {
                            break;
                        }
                    } /* while traversing extensions */
                } /* if there is an extension */


                reg = client->regs->next;
                extptr = client->_scratch;
                next_extptr = extptr + 2;
                ext_offset = 0;
                while (! _LSLP_IS_HEAD(reg))
                {
                    if (time(NULL) > reg->directoryTime)
                    {
                        /* the lifetime for this registration has expired */
                        /* unlink and free it */
                        lslpSrvRegList *temp_reg = reg;
                        reg = reg->next;
                        _LSLP_UNLINK(temp_reg);
                        lslpFreeSrvReg(temp_reg);
                        continue;
                    }
                    // Match the remote address family. Send ip4 URLs to ip4
                    // remote addrs and ip6 URLs to ip6 remote addrs.
                    if (!_slp_check_url_addr(reg->url->url, remote->sa_family,
                        url_addr))
                    {
                        reg = reg->next;
                        continue;
                    }

                    // Determine whether we have correct ip6 SCOPES.
/*
                               Request Source Address Scope
                          +------------+------------+---------+
                          | Link-Local | Site-Local | Global  |
            +-------------+------------+------------+---------+
   Service  | Link-Local  |  Respond   |    Drop    |   Drop  |
   Address  +-------------+------------+------------+---------+
   Scope    | Site-Local  |  Respond   |   Respond  |   Drop  |
            +-------------+------------+------------+---------+
            | Global      |  Respond   |   Respond  | Respond |
            +-------------+------------+------------+---------+

                       Out-of-Scope Rules
*/

#ifdef PEGASUS_ENABLE_IPV6
                    if (remote->sa_family == AF_INET6)
                    {
                        struct in6_addr src_addr =
                            ((SOCKADDR_IN6*)remote)->sin6_addr;
                        // Site local source address
                        if (IN6_IS_ADDR_SITELOCAL(&src_addr))
                        {
                            // Drop if Link local.
                            if (IN6_IS_ADDR_LINKLOCAL(
                                (struct in6_addr*)&url_addr))
                            {
                                reg = reg->next;
                                continue;
                            }
                        }
                        // Global source address
                        else if (!IN6_IS_ADDR_SITELOCAL(&src_addr) &&
                            !IN6_IS_ADDR_LINKLOCAL(&src_addr))
                        {
                            // Drop if Link or Site local.
                            if (IN6_IS_ADDR_SITELOCAL(
                                   (struct in6_addr*)&url_addr) ||
                                IN6_IS_ADDR_LINKLOCAL(
                                    (struct in6_addr*)&url_addr))
                            {
                                reg = reg->next;
                                continue;
                            }
                        }
                    }
#endif
                    if (TRUE == lslp_srvtype_match(
                        reg->srvType,
                        msg->srvcType))
                    {
                        /* we need to check for the special case - of a
                           srvreq with service type directory-agent */
                        /* - it is allowed to omit scopes */
                        if ((msg->scopeList == NULL) ||
                            (_LSLP_IS_EMPTY(msg->scopeList) &&
                            (! strncasecmp(
                            msg->srvcType,
                            DA_SRVTYPE,
                            DA_SRVTYPELEN))) ||
                            (TRUE == lslp_scope_intersection(
                            reg->scopeList,
                            msg->scopeList)))
                        {
                            if (TRUE == lslp_predicate_match(
                                reg->attrList,
                                msg->predicate))
                            {
                                /* we have a match for protected scopes,
                                   we need to change stuff URL so that it
                                   only stuffs url auth blocks that match spis
                                   in the srv req */
                                if (TRUE == lslpStuffURL(
                                    &buf,
                                    &avail, reg->url))
                                {
                                    if (TRUE == pile_up_attrs &&
                                        ((reg->url->len + ext_offset + 2) <
                                        LSLP_MTU))
                                    {
                                        /* use the scratch buffer as temporary
                                           storage for the attribute extensions
                                           go back and set the length of the
                                           next extension in prev extension */
                                        _LSLP_SET3BYTES(
                                            next_extptr,
                                            ext_offset, 0);
                                        next_extptr = extptr + 2;
                                        /* set the extension id */
                                        _LSLP_SETSHORT(extptr, 0x0002, 0);
                                        /* init the next ext to zero */
                                        _LSLP_SET3BYTES(
                                            next_extptr,
                                            0x00000000,
                                            0);
                                        /* length of url string */
                                        _LSLP_SETSHORT(
                                            extptr,
                                            reg->url->len,
                                            5);
                                        memcpy(
                                            extptr + 7,
                                            reg->url->url,
                                            reg->url->len);
                                        ext_offset += (7 + reg->url->len);
                                        extptr += (7 + reg->url->len);
                                        /* attr list length */
                                        if (ext_offset + 2 +
                                            reg->attrList->attr_string_len <
                                            LSLP_MTU)
                                        {
                                            _LSLP_SETSHORT(
                                                extptr,
                                                reg->attrList->attr_string_len,
                                                0);
                                            memcpy(
                                                extptr + 2,
                                                reg->attrList->attr_string,
                                                reg->attrList->attr_string_len
                                                );
                                            ext_offset +=
                                                (2 +
                                                reg->attrList->attr_string_len
                                                );
                                            extptr +=
                                                (2 +
                                                reg->attrList->attr_string_len
                                                );
                                            if (ext_offset + 1 < LSLP_MTU)
                                            {
                                                /* set the number of attr auths
                                                   to zero */
                                                //jeb attrib set to 1??
                                                //_LSLP_SETBYTE(extptr, 1, 0);
                                                _LSLP_SETBYTE(extptr, 0, 0);
                                                extptr += 1;
                                                ext_offset += 1;
                                            }
                                            else
                                            {
                                                pile_up_attrs = FALSE;
                                                _LSLP_SETSHORT(
                                                    client->_scratch,
                                                    0x0000,
                                                    0);
                                            }
                                        }
                                        else
                                        {
                                            pile_up_attrs = FALSE;
                                            _LSLP_SETSHORT(
                                                client->_scratch,
                                                0x0000,
                                                0);
                                        }
                                    }
                                    else
                                    {
                                        pile_up_attrs = FALSE;
                                        _LSLP_SETSHORT(
                                            client->_scratch,
                                            0x0000,
                                            0);
                                    }
                                    temp_rply->urlCount++;
                                }
                                else
                                {
                                    temp_rply->errCode = LSLP_INTERNAL_ERROR;
                                    return(temp_rply);
                                }
                            } /* if we have a match */
                        } /* if we found a scope intersection */
                    } /* if the types match */
                    reg = reg->next;
                } /* while we are traversing the reg list for this da */
                /* stuff the error code and url count */
                _LSLP_SETSHORT(temp_rply->urlList, temp_rply->errCode, 0);
                _LSLP_SETSHORT(temp_rply->urlList, temp_rply->urlCount, 2);
                /* resize to the actual size needed */
                temp_rply->urlLen = (uint16)(buf - temp_rply->urlList) ;
            } /* if we alloced our buffer */
        } /* if we need to look for matches */
        else
        {
            if (NULL != (temp_rply->urlList = (char *)calloc(8, sizeof(char))))
            {
                _LSLP_SETSHORT(temp_rply->urlList, temp_rply->errCode, 0);
            }
        } /* error reply */
        if (temp_rply->urlList == NULL)
        {
            free(temp_rply);
            temp_rply = NULL;
        }
    } /* if we alloced our reply structure */
    return(temp_rply);
}


#define lslp_to_lower(c)  (((c) > 0x40 && (c) < 0x5b) ? ((c) + 0x20) : (c))
/* based upon TclStringCaseMatch */
#define MAX_RECURSION  10
static BOOL _lslp_pattern_match(
    const char *string,
    const char *pattern,
    BOOL case_sensitive,
    BOOL eb)
{

    char s, p;
    static int recursion_level;
    if (recursion_level == MAX_RECURSION)
    {
        return FALSE;
    }
    recursion_level++;

    PEGASUS_ASSERT(string != NULL && pattern != NULL);

    while (1)
    {
        p = *pattern;
        if (p == '\0')
        {
            recursion_level--;
            if (*string == '\0')  //jeb
            {
                return TRUE;
            }
            else
            {
                return FALSE;
            }
            //jeb       return (*string == '\0');
        }

        if ((*string == '\0') && (p != '*'))
        {
            recursion_level--;
            return FALSE;
        }

        if (p == '*')
        {
            while (*(++pattern) == '*')
            {
                ;
            }

            p = *pattern;
            if (p == '\0')
            {
                recursion_level--;
                return TRUE;
            }
            while (1)
            {
                if ((!eb && p != '[') && (p != '?') && (p != '\\'))
                {
                    /* advance the string until there is a match */
                    while (*string)
                    {
                        s = *string;
                        if (case_sensitive == FALSE)
                        {
                            p = lslp_to_lower(p);
                            s = lslp_to_lower(s);
                        }
                        if (s == p)
                        {
                            break;
                        }
                        string++;
                    }
                }
                if (TRUE ==
                    _lslp_pattern_match(string, pattern, case_sensitive, eb))
                {
                    recursion_level--;
                    return TRUE;
                }
                if (*string == '\0')
                {
                    recursion_level--;
                    return FALSE;
                }
                string++;
            }
        }
        if (p == '?')
        {
            pattern++;
            string++;
            continue;
        }

        if (!eb && p == '[')
        {
            char start, end;
            pattern++;
            s = *(string++);
            if (case_sensitive == FALSE)
            {
                s = lslp_to_lower(s);
            }
            while (1)
            {
                if ((!eb && *pattern == ']') || *pattern == '\0')
                {
                    recursion_level--;
                    return FALSE ;
                }
                start = *(pattern++);
                if (case_sensitive == FALSE)
                {
                    start = lslp_to_lower(start);
                }
                if (*pattern == '-')
                {
                    pattern++;
                    if (*pattern == '\0')
                    {
                        recursion_level--;
                        return FALSE;
                    }
                    end = *(pattern++);
                    if (case_sensitive == FALSE)
                    {
                        end = lslp_to_lower(end);
                    }
                    if (((start <= s) && (s <= end )) ||
                        ((end <= s) && (s <= start)))
                    {
                        break;
                    }
                }
                else if (start == s)
                {
                    break;
                }
            }
            while (!eb && *pattern != ']')
            {
                if (*pattern == '\0')
                {
                    pattern--;
                    break;
                }
                pattern++;
            }
            pattern++;
            continue;
        }

        if (p == '\\')
        {
            pattern++;
            if (*pattern == '\0')
            {
                recursion_level--;
                return FALSE;
                //jeb    return 0;
            }
        }
        s = *(string++);
        p = *(pattern++);
        if (case_sensitive == FALSE)
        {
            p = lslp_to_lower(p);
            s = lslp_to_lower(s);
        }
        if (s == p)
        {
            continue;
        }
        recursion_level--;
        return FALSE;
    }
}

BOOL lslp_pattern_match(
    const char *string,
    const char *pattern,
    BOOL case_sensitive)
{
    return _lslp_pattern_match(string, pattern, case_sensitive, FALSE);
}

/*
     Matches the pattern by treating brackets([]) as part regualr expression
     expansion and also not treating them as  part of reaular expressions.
     Used for IPv6 addrss URLs.
*/
BOOL lslp_pattern_match2(
    const char *string,
    const char *pattern,
    BOOL case_sensitive)
{
    if ( TRUE == _lslp_pattern_match(string, pattern, case_sensitive, TRUE))
    {
        return TRUE;
    }
    return _lslp_pattern_match(string, pattern, case_sensitive, FALSE);
}

char * lslp_get_next_ext(char *hdr_buf)
{
    int32 len;
    PEGASUS_ASSERT(hdr_buf != NULL);
    if (0 != _LSLP_GETNEXTEXT(hdr_buf))
    {
        len = _LSLP_GETLENGTH(hdr_buf);
        if (len > _LSLP_GETNEXTEXT(hdr_buf))
        {
            return( hdr_buf + _LSLP_GETNEXTEXT(hdr_buf));
        }
    }
    return NULL;
}


/** uses a newline as the field separator, two consecutive newlines
    as the record separator **/
void lslp_print_srv_rply(lslpMsg *srvrply)
{

    lslpURL *url_list;
    if (srvrply != NULL && srvrply->type == srvRply)
    {
        /* output errCode, urlCount, urlLen */
        printf("%d\n%d\n%d\n", srvrply->msg.srvRply.errCode,
            srvrply->msg.srvRply.urlCount,
            srvrply->msg.srvRply.urlLen );
        if ((NULL != srvrply->msg.srvRply.urlList) &&
            (! _LSLP_IS_EMPTY( srvrply->msg.srvRply.urlList)))
        {

            url_list = srvrply->msg.srvRply.urlList->next;
            while (! _LSLP_IS_HEAD(url_list))
            {

                if (NULL != url_list->url)
                {
                    printf("URL: %s\n", url_list->url);
                }
                else
                {
                    printf("URL: \n");
                }

                /* see if there are attributes to print */
                if (NULL != url_list->attrs &&
                    ! _LSLP_IS_HEAD(url_list->attrs->next))
                {
                    lslpAtomList *attrs = url_list->attrs->next;
                    while (! _LSLP_IS_HEAD(attrs))
                    {
                        printf("ATTR: %s\n", attrs->str);
                        attrs = attrs->next;
                    } /* while traversing attr list */
                } /* if attr list */
                url_list = url_list->next;

            } /* while traversing url list */
        } /* if there are urls to print */
        /* print the record separator */
        printf("\n\n");
    }
}

/* outputs information for machine consumption */
void lslp_print_srv_rply_parse(lslpMsg *srvrply, char fs, char rs)
{
    BOOL dont_print_extra_rs = FALSE;

    lslpURL *url_list;
    if (srvrply != NULL && srvrply->type == srvRply)
    {
        /* output errCode, urlCount, urlLen */
        printf("%d%c%d%c%d%c",
            srvrply->msg.srvRply.errCode,
            fs,
            srvrply->msg.srvRply.urlCount,
            fs,
            srvrply->msg.srvRply.urlLen,
            fs);
        if ((NULL != srvrply->msg.srvRply.urlList) &&
            (! _LSLP_IS_EMPTY( srvrply->msg.srvRply.urlList)))
        {

            url_list = srvrply->msg.srvRply.urlList->next;
            while (! _LSLP_IS_HEAD(url_list))
            {

                if (NULL != url_list->url)
                {
                    printf("%s%c", url_list->url, fs);
                }
                else
                {
                    printf("%c", fs);
                }

                /* see if there are attributes to print */
                if (NULL != url_list->attrs &&
                    ! _LSLP_IS_HEAD(url_list->attrs->next))
                {
                    lslpAtomList *attrs = url_list->attrs->next;
                    while (! _LSLP_IS_HEAD(attrs) && attrs->str &&
                        strlen(attrs->str))
                    {
                        printf("%s", attrs->str);
                        attrs = attrs->next;
                        if (! _LSLP_IS_HEAD(attrs) && attrs->str &&
                            strlen(attrs->str))
                        {
                            printf("%c", fs);
                        }
                    } /* while traversing attr list */
                }
                else
                {
                    /* if no attr list, print the record separator  */
                    printf("%c", rs);
                    dont_print_extra_rs = TRUE;
                }

                url_list = url_list->next;
                /* if there is another url, print a record separator
                   also print the first four fields so the consuming program's
                   parser doesn't get upset */
                if (! _LSLP_IS_HEAD(url_list) && dont_print_extra_rs == FALSE)
                {

                    printf("%c", rs);
                    printf("%d%c%d%c%d%c",
                        srvrply->msg.srvRply.errCode,
                        fs,
                        srvrply->msg.srvRply.urlCount,
                        fs,
                        srvrply->msg.srvRply.urlLen,
                        fs);
                }

            } /* while traversing url list */

        } /* if there are urls to print */
        /* print the record separator */
        printf("%c", rs);
    }
}

/** uses a newline as the field separator, two consecutive newlines
    as the record separator **/
SLP_STORAGE_DECL void lslp_print_attr_rply(lslpMsg *attrrply)
{
    if (attrrply != NULL && attrrply->type == attrRep)
    {

        /* output the err, attr len, attr string */
        printf("error code: %d\nattribute length: %d\n%s\n\n",
            attrrply->msg.attrRep.errCode,
            attrrply->msg.attrRep.attrListLen,
            attrrply->msg.attrRep.attrList);
    }
    else
    {
        printf("no correctly formatted responses\n\n");
    }
}


SLP_STORAGE_DECL void lslp_print_attr_rply_parse(
    lslpMsg *attrrply,
    char fs,
    char rs)
{
    if (attrrply != NULL && attrrply->type == attrRep)
    {

        /* output the err, attr len, attr string */
        printf("%d%c%d%c%s%c",
            attrrply->msg.attrRep.errCode, fs,
            attrrply->msg.attrRep.attrListLen,fs,
            attrrply->msg.attrRep.attrList, rs);
    }
}



/* string must not be null and must start with "service:" */
SLP_STORAGE_DECL BOOL test_service_type(char *type)
{
    char * temp;
    BOOL ccode;

    if (type == NULL || ! strlen(type))
    {
        return FALSE;
    }
    temp = strdup(type);
    if (temp == NULL)
    {
        printf("Memory allocation failed in file %s at Line number %d\n",
            __FILE__, __LINE__);
        exit(1);
    }
    ccode = lslp_pattern_match( "service:*", temp, FALSE);
    free(temp);
    return ccode;
}


SLP_STORAGE_DECL BOOL test_service_type_reg(char *type)
{
    char * temp;
    BOOL ccode;

    if (type == NULL || ! strlen(type))
    {
        return FALSE;
    }
    temp = strdup(type);
    if (temp == NULL)
    {
        printf("Memory allocation failed in file %s at Line number %d\n",
            __FILE__, __LINE__);
        exit(1);
    }
    ccode = lslp_pattern_match(temp, "service:*", FALSE);
    free(temp);
    return ccode;
}


SLP_STORAGE_DECL BOOL test_url(char *url)
{

    char *temp;
    lslpAtomizedURL *aurl;

    if (url == NULL)
    {
        return FALSE;
    }
    temp = strdup(url);
    if (temp == NULL)
    {
        printf("Memory allocation failed in file %s at Line number %d\n",
            __FILE__, __LINE__);
        exit(1);
    }
    aurl = _lslpDecodeURLs(&temp, 1);
    free(temp);
    if (aurl != NULL)
    {
        lslpFreeAtomizedURLList(aurl, TRUE);
        return TRUE;
    }
    return FALSE;
}

SLP_STORAGE_DECL BOOL test_attribute(char *attr)
{
    char *temp;
    lslpAttrList *attr_list;

    if (attr == NULL)
    {
        return FALSE;
    }
    if (! strlen(attr))
    {
        return TRUE;
    }

    temp = strdup(attr);
    if (temp == NULL)
    {
        printf("Memory allocation failed in file %s at Line number %d\n",
            __FILE__, __LINE__);
        exit(1);
    }
    attr_list = _lslpDecodeAttrString(temp);
    free(temp);

    if (attr_list != NULL)
    {
        lslpFreeAttrList(attr_list, TRUE);
        return TRUE;
    }
    return FALSE;
}

SLP_STORAGE_DECL BOOL test_scopes(char *scopes)
{
    BOOL ccode;
    char *temp;

    if (scopes == 0)
    {
        return FALSE;
    }

    if (! strlen(scopes))
    {
        return TRUE;
    }

    temp = strdup(scopes);
    if (temp == 0)
    {
        printf("Memory allocation failed in file %s at Line number %d\n",
            __FILE__, __LINE__);
        exit(1);
    }
    ccode = lslp_islegal_scope(temp);
    free(temp);

    return ccode;
}

SLP_STORAGE_DECL BOOL test_predicate(char *predicate)
{
    char * temp;
    lslpLDAPFilter *filter;

    if (predicate == 0)
    {
        return FALSE;
    }
    if (! strlen(predicate))
    {
        return TRUE;
    }

    temp = strdup(predicate);
    if (temp == 0)
    {
        printf("Memory allocation failed in file %s at Line number %d\n",
            __FILE__, __LINE__);
        exit(1);
    }
    filter = _lslpDecodeLDAPFilter(temp);
    free(temp);
    if (filter != NULL)
    {
        lslpFreeFilter(filter);
        return TRUE;
    }
    return FALSE;
}


SLP_STORAGE_DECL uint32 test_srv_reg(
    char *type,
    char *url,
    char *attr,
    char *scopes)
{

    /* it is legal for the scope string and attribute list to be empty */
    if (type == NULL || 0 == strlen(type) ||
        FALSE == test_service_type_reg(type))
    {
        return 1;
    }
    if (url == NULL || 0 == strlen(url) || FALSE == test_url(url))
    {
        return 2;
    }
    if (attr != NULL && FALSE == test_attribute(attr))
    {
        return 3;
    }
    if (scopes != NULL && FALSE == test_scopes(scopes))
    {
        return 4;
    }
    return 0;
}



SLP_STORAGE_DECL uint32 test_query(
    char *type,
    char *predicate,
    char *scopes)
{
    /* it is legal for predicate and scopes to be empty */

    if (type == NULL || 0 == strlen(type) || FALSE == test_service_type(type))
    {
        return 1;
    }

    if (predicate != NULL && FALSE == test_predicate(predicate))
    {
        return 2;
    }

    if (scopes != NULL && FALSE == test_scopes(scopes))
    {
        return 3;
    }

    return 0;
}



#ifdef INCLUDE_TEST_CODE

/* , (serial-number=78-HNRX5)" */
/* , (fru=model 8657\\-a)" */
/*                 "name=chassis_4_b205g134),
                   (uuid=\\ff\0d\0a\25\56\0d\0a\25\56\0d\0a\25\56)" \ */
/*                 "(ip-address=9.37.98.90)" */



#define MM_URL "service:management-hardware.IBM:management-module://" \
"chassis_one:4456/%20;%20hello"
#define MM_URL2 "service:management-hardware.IBM:management-module://" \
    "chassis_two:4456/%20;%20hello"
#define MM_URL3 "service:management-hardware.IBM:management-module://" \
    "chassis_three:4456/%20;%20hello"
#define MM_ATTR "management-module, (mangement-module_test =     )"
#define MM_ATTR2 "(enclosure-serial-number=\\ff\\04\\05), management-module"
#define MM_ATTR3 "(enclosure-serial-number=\\ff\\04\\05), management-module," \
        "(nothing=one)"
#define MM_SRV_TYPE "service:management-hardware.IBM:management-module"
#define MM_SCOPES "DEFAULT"
#define MM_LIFETIME 0xff
#define MM_PRED "(management-module =*)"
#define MM_PRED2 "management-module, (enclosure-serial-number=\\ff\\04\\05)"


    int32 opaque_buf[] = {
    0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
    0x09, 0x0a, 0xfe0b, 0xcdac, 0x0d, 0x0e, 0x10
};
int16 opaque_len = 15 * sizeof(int32);

#define NUM_STRINGS_TO_MATCH 8
char *patterns[] = {
    "hello, world",
    "hEllo, wOrld",
    "Custer is a h?ll of a d**n*ed fo*\\; -- Phil Sheridan",
    "See Mug, See",
    "management-hardware.[ih][ba][ml]",
    "The telephone is [hlr]eap [bds]ig medicine -- Crazy Horse",
    "Young kids today are [A-z]ool",
    "Young kids today are [A-z]ool"
};

char *strings[] = {
    "hello, world",
    "hello, world",
    "custer is a hill of a dinged fog; -- Phil Sheridan",
    "see mug, see",
    "management-Hardware.HAL",
    "the telephone is leap dig medicine -- Crazy Horse",
    "young kids today are cool",
    "young kids today are fool"
};


int main(int argc, char **argv)
{
    BOOL ccode;

    lslpMsg *replies, rply;
    int i ;
    time_t now, last;

    struct slp_client *client = create_slp_client(
        NULL,
        NULL,
        DEFAULT_SLP_PORT,
        "DSA",
        "DEFAULT, TEST SCOPE",
        TRUE,
        TRUE,
        0);

    if (client != NULL)
    {
        char *encoded, *test_buf;
        void *decoded;
        int16 encoded_len;
        printf("Testing opaque encode/decode\n");

        encoded = encode_opaque(opaque_buf, opaque_len);
        decoded = decode_opaque(encoded);
        if (memcmp(decoded, opaque_buf, opaque_len))
        {
            printf("opaque decoder produced mismatch\n");
        }

        encoded_len = _LSLP_GETSHORT(encoded, 0);
        test_buf = (char *)calloc(encoded_len, sizeof(char));
        if (test_buf != NULL)
        {
            memcpy(test_buf, encoded + 2, encoded_len - 2);
            printf("%s\n", test_buf);
            free(test_buf);
        }

        free(encoded);
        free(decoded);

        for (i = 0; i < NUM_STRINGS_TO_MATCH ; i++)
        {
            if (TRUE == lslp_pattern_match(strings[i], patterns[i], FALSE))
            {
                printf("%s MATCHES %s (no case)\n", strings[i], patterns[i]);
            }
            else
            {
                printf("%s DOES NOT MATCH %s (no case)\n",
                    strings[i], patterns[i]);
            }
        }

        for (i = 0; i < NUM_STRINGS_TO_MATCH ; i++)
        {
            if (TRUE == lslp_pattern_match(strings[i], patterns[i], TRUE))
            {
                printf("%s MATCHES %s \n", strings[i], patterns[i]);
            }
            else
            {
                printf("%s DOES NOT MATCH %s \n", strings[i], patterns[i]);
            }
        }
        client->srv_reg_local(
            client,
            "service:service-agent:\\9.37.98.90",
            "(service-type = )", "service-agent",
            MM_SCOPES, MM_LIFETIME);

        ccode = client->srv_reg_all(
            client,
            MM_URL,
            MM_ATTR,
            MM_SRV_TYPE,
            MM_SCOPES,
            MM_LIFETIME);
        printf("registered with %d agents.\n", ccode );

        ccode = client->srv_reg_all(
            client,
            MM_URL2,
            MM_ATTR2,
            MM_SRV_TYPE,
            MM_SCOPES,
            MM_LIFETIME);
        printf("registered with %d agents.\n", ccode );

        ccode = client->srv_reg_all(
            client,
            MM_URL3,
            MM_ATTR3,
            MM_SRV_TYPE,
            MM_SCOPES,
            MM_LIFETIME);
        printf("registered with %d agents.\n", ccode );

        client->converge_srv_req(client, MM_SRV_TYPE, MM_PRED, MM_SCOPES);

        client->converge_srv_req(
            client,
            "service-agent",
            "(service-type =*)",
            MM_SCOPES);

        replies = &rply;

        client->service_listener(client, 0, replies);
        if (_LSLP_IS_HEAD(replies->next))
        {
            printf("Recieved 0 Service Replies\n");
        }
        else
        {
            replies = replies->next;
            while (! _LSLP_IS_HEAD(replies))
            {
                if (replies->type == srvRply)
                {
                    printf("SRVRPLY: err_code %d url_count %d, url_len %d \n",
                        replies->msg.srvRply.errCode,
                        replies->msg.srvRply.urlCount,
                        replies->msg.srvRply.urlLen);
                    if (NULL != replies->msg.srvRply.urlList)
                    {
                        if (! _LSLP_IS_EMPTY(replies->msg.srvRply.urlList))
                        {
                            lslpURL *url_list =
                                replies->msg.srvRply.urlList->next;
                            while (! _LSLP_IS_HEAD(url_list))
                            {
                                printf("\t%s\n", url_list->url);
                                if ((NULL != replies->msg.srvRply.attr_list) &&
                                    (! _LSLP_IS_EMPTY(
                                    replies->msg.srvRply.attr_list)))
                                {
                                    lslpAtomList *temp =
                                        replies->msg.srvRply.attr_list->next;
                                    if (! _LSLP_IS_HEAD(temp))
                                    {
                                        if (temp->str != NULL)
                                        {
                                            printf("ATTRS: %s\n", temp->str);
                                            _LSLP_UNLINK(temp);
                                            lslpFreeAtom(temp);
                                        }
                                    }
                                }
                                url_list = url_list->next;
                            }
                        }
                        else
                        {
                            printf("no URLs\n");
                        } /* url list not empty */
                    }
                    else
                    {
                        printf("no URLs\n");
                    } /* url list  not NULL */
                } /* srvrply */
                replies    = replies->next;
            }    /* traversing replies */
        }
    }

    now = (last = time(NULL));

    while (1)
    {
        lslpMsg msg;
        _LSLP_SLEEP(100);
        client->service_listener(client, 0, &msg);
        now = time(NULL);
        if (now - last > ( MM_LIFETIME - 1))
        {
            client->srv_reg_local(
                client,
                MM_URL2,
                MM_ATTR2,
                MM_SRV_TYPE,
                MM_SCOPES,
                MM_LIFETIME);
            last = time(NULL);
        }
    }
    return 1;
}


#endif //test code
