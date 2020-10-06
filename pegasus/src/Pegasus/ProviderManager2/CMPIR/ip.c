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

/*!
    \file ip.c
    \brief General TCP/IP routines.
*/

#include "cmpir_common.h"
#include <stdio.h>
#include <stdlib.h>

#if defined PEGASUS_OS_TYPE_WINDOWS
# include <winsock2.h>
#else
# if defined PEGASUS_OS_ZOS
#  include <arpa/inet.h>
#else
#  include <error.h>
# endif
# include <strings.h>
# include <sys/socket.h>
# include <netinet/in.h>
# include <netdb.h>
# include <unistd.h>
#endif

#include <errno.h>
#include <string.h>
#include <sys/types.h>

#ifndef CMPI_VER_100
# define CMPI_VER_100
#endif

#include <Pegasus/Provider/CMPI/cmpimacs.h>
#include <Pegasus/Provider/CMPI/cmpidt.h>
#include <Pegasus/Provider/CMPI/cmpift.h>

#include "ip.h"
#include "tcpcomm.h"
#include "debug.h"

struct linger __linger =
{
    1,
    15
};

PEGASUS_IMPORT extern CMPIBrokerExtFT *CMPI_BrokerExt_Ftab;

static int _die = 0;
/****************************************************************************/

static struct hostent * _getHostByName (
    const char *hname,
    struct hostent *hbuf,
    char *tmphbuf,
    int hbuflen)
{
    struct hostent  *hptr;
    int herr=0,rc=0;

#ifdef PEGASUS_OS_ZOS
    extern int h_errno;
#endif

#if defined(PEGASUS_OS_LINUX)
    rc = gethostbyname_r (hname,hbuf,tmphbuf,hbuflen,&hptr,&herr);
#elif defined(PEGASUS_OS_SOLARIS)
    hptr = gethostbyname_r(hname,hbuf,tmphbuf,hbuflen,&herr);
#else
    hptr = gethostbyname(hname);
#endif

    if (hptr==NULL)
    {

#ifdef PEGASUS_OS_TYPE_WINDOWS
        const char* lpMsgBuf=NULL;
        DWORD winErrorCode;

        winErrorCode = GetLastError();
        FormatMessage(
            FORMAT_MESSAGE_ALLOCATE_BUFFER |
            FORMAT_MESSAGE_FROM_SYSTEM,
            NULL,
            winErrorCode,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            (LPTSTR) &lpMsgBuf,
            0, NULL );
#endif

        error_at_line (0, 0, __FILE__, __LINE__,
#ifdef PEGASUS_OS_ZOS
            strerror (h_errno));
#elif defined PEGASUS_OS_TYPE_WINDOWS
            (char *)lpMsgBuf);
#else
            hstrerror (h_errno));
#endif

    }

    return hptr;
}

#ifdef PEGASUS_OS_TYPE_WINDOWS
void winStartNetwork()
{
    WSADATA winData;
    WSAStartup ( 0x0002, &winData );
}
#endif

int open_connection ( const char * address, int port, int print_errmsg )
{
    int sockfd;
    struct sockaddr_in sin;
    struct hostent * server_host_name;
    struct hostent hbuf;
    char tempbuf[8192];
// masking unability to transform an ip-address via gethostbyname()
#ifdef PEGASUS_OS_ZOS
    extern int h_errno;
    in_addr_t broker_ip_address;
    broker_ip_address = inet_addr(address);

    if (broker_ip_address != INADDR_NONE)
    {
        // HERE COMES THE CALL TO GETHOSTBYADDR
        server_host_name = gethostbyaddr(
            &(broker_ip_address),
            sizeof(broker_ip_address),
            AF_INET);
        if (server_host_name == NULL)
        {
            if (print_errmsg == PEGASUS_PRINT_ERROR_MESSAGE)
            {
                error_at_line ( 0, 0, __FILE__, __LINE__,strerror(h_errno));
                return -1;
            }
        }
    }
    else
    {
#endif
        if ((server_host_name = _getHostByName (
            address,
            &hbuf,
            tempbuf,
            sizeof(tempbuf))) == NULL)
        {
            return -1;
        }
// masking end of if case for differing between ip-address and host
#ifdef PEGASUS_OS_ZOS
    }
#endif

    sin.sin_family = AF_INET;
    sin.sin_port = htons(port);
    sin.sin_addr.s_addr =
        ( (struct in_addr *) ( server_host_name->h_addr ) )->s_addr;

    if (( sockfd = socket (
        PF_INET,
        SOCK_STREAM,
#ifdef PEGASUS_OS_ZOS
        0 ) ) == -1)
#else
        IPPROTO_TCP ) ) == PEGASUS_CMPIR_INVALID_SOCKET)
#endif
    {
        if (print_errmsg == PEGASUS_PRINT_ERROR_MESSAGE)
        {
            error_at_line (
                0,
                errno,
                __FILE__,
                __LINE__,
                "failed to create socket");
        }
        return -1;
    }

    setsockopt (
        sockfd,
        SOL_SOCKET,
        SO_LINGER,
        &__linger,
        sizeof ( struct linger ));

    if (connect ( sockfd, (struct sockaddr *) &sin, sizeof ( sin ) ) == -1)
    {
        //invokes close(socket) on unix & closesocket(socket) on windows
        PEGASUS_CMPIR_CLOSESOCKET(sockfd);
        if (print_errmsg == PEGASUS_PRINT_ERROR_MESSAGE)
        {
            error_at_line (
                0,
                errno,
                __FILE__,
                __LINE__,
                "could not connect to %s:%d",
                address,
                port);
        }
        return -1;
    }
    return sockfd;
}

PEGASUS_EXPORT void accept_connections (
    int port,
    void (* __connection_handler) ( int ),
    int multithreaded)
{
    CMPI_THREAD_TYPE t;
    int in_socket, listen_socket;
    struct sockaddr_in sin;
    socklen_t sin_len = sizeof ( sin );
    ssize_t param;

    int ru = 1;
#ifdef PEGASUS_OS_TYPE_WINDOWS
    winStartNetwork();
#endif
    listen_socket = socket(PF_INET,SOCK_STREAM,IPPROTO_TCP);

    setsockopt (
        listen_socket,
        SOL_SOCKET,
        SO_REUSEADDR,
        (char *) &ru,
        sizeof ( ru ));

    memset(&sin,0,sin_len);
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = INADDR_ANY;
    sin.sin_port = htons ( port );

    if (bind ( listen_socket, (struct sockaddr *) &sin, sin_len ) ||
#ifdef PEGASUS_OS_ZOS
        listen ( listen_socket, 15 ))
    {
#else
        listen ( listen_socket, 0 ))
    {
#endif
        error_at_line (
            -1,
            PEGASUS_CMPIR_WSAGETLASTERROR,
            __FILE__,
            __LINE__,
            "cannot listen on port %d", port);
    }
    _die = 0;
    while (( in_socket = accept (
        listen_socket,
        (struct sockaddr *) &sin,
        &sin_len ) ) > 0)
    {
        if (_die == 1)
        {
            //invokes close(socket) on unix & closesocket(socket) on windows
            PEGASUS_CMPIR_CLOSESOCKET(in_socket);
            break;
        }
        setsockopt (
            in_socket,
            SOL_SOCKET,
            SO_LINGER,
            &__linger,
            sizeof ( struct linger ) );
        param = in_socket;
        if (multithreaded)
        {

            t = CMPI_BrokerExt_Ftab->newThread(
                (void *(PEGASUS_CMPIR_STDCALL*)(void *))__connection_handler,
                (void *) param,
                1);

        }
        else
            __connection_handler ( in_socket );
    }
    if (in_socket < 0)
    {
        error_at_line ( -1,
            errno,
            __FILE__,
            __LINE__,
            "invalid socket descriptor (%d) ",
            in_socket);
    }

    //invokes close(socket) on unix & closesocket(socket) on windows
    PEGASUS_CMPIR_CLOSESOCKET (listen_socket);
    listen_socket = 0;
    _die = 0;

}


int close_connection (int port )
{
    int socket = 0;

    _die = 1;

    // "tickle" the connection.
    socket = open_connection("127.0.0.1", port,PEGASUS_PRINT_ERROR_MESSAGE);
    if (socket)
    {
        //invokes close(socket) on unix & closesocket(socket) on windows
        PEGASUS_CMPIR_CLOSESOCKET (socket);

        while (_die == 1)
        {
            //sleep(1) on unix & Sleep(1) on windows
            PEGASUS_CMPIR_SLEEP(1);
        }
    }
    return _die;
}

void get_peer_address ( int socket, char * buf )
{
#define UC(b)   ( ( (int) b ) & 0xFF )

    struct sockaddr_in sin;
    socklen_t sinlen = sizeof ( sin );
    char * p = (char *) &sin.sin_addr;

    getpeername ( socket, (struct sockaddr *) &sin, &sinlen );
    sprintf ( buf, "%d.%d.%d.%d", UC(p[0]), UC(p[1]), UC(p[2]), UC(p[3]) );
}

/****************************************************************************/

/*** Local Variables:  ***/
/*** mode: C           ***/
/*** c-basic-offset: 8 ***/
/*** End:              ***/
