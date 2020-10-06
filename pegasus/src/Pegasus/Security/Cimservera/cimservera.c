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
/*
//
//%/////////////////////////////////////////////////////////////////////////////
*/

#include <Executor/PAMAuth.h>
#include <Executor/Socket.h>
#include <Pegasus/Common/PegasusVersion.h>
#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

static void Exit(int status)
{
    exit(status);
}

int main(int argc, char* argv[])
{
    int sock;
    CimserveraRequest request;
    CimserveraResponse response;

    /* Open syslog: */

    openlog("cimservera", LOG_PID, LOG_AUTH);

    /* Check arguments. */

    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <socket-number>\n", argv[0]);
        Exit(1);
    }

    /* Open socket stream. */

    {
        char* end;
        long x = strtoul(argv[1], &end, 10);

        if (*end != '\0')
        {
            fprintf(stderr, "%s : bad socket argument: %s\n", argv[0], argv[1]);
            Exit(1);
        }

        sock = (int)x;
    }

    /* Wait on request. */

    if (RecvBlock(sock, &request, sizeof(request)) != sizeof(request))
    {
        close(sock);
        Exit(1);
    }

    /* Handle request (authenticate or validateUser) */

    if (strcmp(request.arg0, "authenticate") == 0)
    {
        int status = PAMAuthenticateInProcess(request.arg1, request.arg2);

        response.status = status;

        if (SendBlock(sock, &response, sizeof(response)) != sizeof(response))
        {
            close(sock);
            Exit(1);
        }

        close(sock);
        Exit(status == 0 ? 0 : 1);
    }
    else if (strcmp(request.arg0, "validateUser") == 0)
    {
        int status = PAMValidateUserInProcess(request.arg1);

        response.status = status;

        if (SendBlock(sock, &response, sizeof(response)) != sizeof(response))
        {
            close(sock);
            Exit(1);
        }

        close(sock);
        Exit(status == 0 ? 0 : 1);
    }

    syslog(LOG_WARNING, "invalid request");
    close(sock);
    Exit(1);
    return 1;
}
