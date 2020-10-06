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

#ifndef WaitForTCPIP_zOS_h
#define WaitForTCPIP_zOS_h

#include <errno.h>
#include <unistd.h>

PEGASUS_NAMESPACE_BEGIN

/**
 * Check if the TCP/IP stack is available. 
 * And if not, wait 30 seconds and try again.
 * 
 */

void startupWaitForTCPIP( void )
{

    bool sendTcpipMsg = true;
    int rc;
    char hostname[PEGASUS_MAXHOSTNAMELEN + 1];

    while(1) 
    {
        rc = 0;
        hostname[0] = 0;
        rc = gethostname(hostname, sizeof(hostname));

        if (rc == 0)
        {
            // The gethostname was successful.
            return;

        }

        // Resource Temporarily unavailable.
        if (errno == EAGAIN)
        {
            // The program should wait for transport layer to become ready.
            if (sendTcpipMsg)
            {
                Logger::put_l(
                    Logger::STANDARD_LOG,System::CIMSERVER,Logger::INFORMATION,
                    MessageLoaderParms(
                        "src.Service.WaitForTCPIP_zOS.WAIT_FOR_TCPIP",
                        "CIM server startup delayed, "
                        "waiting for TCP/IP to start."));
                sendTcpipMsg = false;
            }
            System::sleep(30);
        }
        else
        {
            Logger::put_l(
                Logger::ERROR_LOG, System::CIMSERVER, Logger::FATAL,
                MessageLoaderParms(
                    "src.Service.WaitForTCPIP_zOS.GETHOSTNAME_FAILED",
                    "Stopping CIM Server startup. "
                    "Failed to retrieve system's hostname: $0",
                    PEGASUS_SYSTEM_NETWORK_ERRORMSG_NLS));
            exit(1);
        }
    }
};


PEGASUS_NAMESPACE_END

#endif


