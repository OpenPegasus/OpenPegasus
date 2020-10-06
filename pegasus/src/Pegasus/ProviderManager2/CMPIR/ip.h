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
    \file ip.h
    \brief General TCP/IP routines.
*/
#ifndef _IP_H_
#define _IP_H_

#include "cmpir_common.h"
#include "proxy.h"

#define PEGASUS_SUPPRESS_ERROR_MESSAGE 0
#define PEGASUS_PRINT_ERROR_MESSAGE 1

static struct hostent * _getHostByName (
    const char *hname,
    struct hostent *hbuf,
    char *tmphbuf,
    int hbuflen);

PEGASUS_EXPORT void accept_connections (
    int port,
    void (* __connection_handler) ( int ),
    int multithreaded );

PEGASUS_EXPORT int open_connection (
    const char * address,
    int port,
    int print_errmsg );

int close_connection( int port );

void get_peer_address ( int socket, char * buf );

#endif

