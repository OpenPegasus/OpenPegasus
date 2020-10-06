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

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/Constants.h>
#include <Pegasus/Common/PegasusVersion.h>
#include <Service/ServerRunStatus.h>
#include <Service/ServerShutdownClient.h>

#ifdef PEGASUS_ENABLE_PRIVILEGE_SEPARATION
# define PEGASUS_PROCESS_NAME "cimservermain"
#else
# define PEGASUS_PROCESS_NAME "cimserver"
#endif

PEGASUS_USING_PEGASUS;
PEGASUS_USING_STD;

ServerRunStatus _serverRunStatus(
    PEGASUS_PROCESS_NAME, PEGASUS_CIMSERVER_START_FILE);

int main(int argc, char** argv)
{
    MessageLoader::_useProcessLocale = true;
    MessageLoader::setPegasusMsgHomeRelative(argv[0]);

    // Check arguments.

    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s shutdown-timeout-in-seconds\n", argv[0]);
        fprintf(stderr,
            "%s is an internal program.  Please do not invoke it directly.\n",
            argv[0]);
        exit(1);
    }

    // Extract timeout argument.

    char* end;
    Uint32 timeout = (Uint32)strtoul(argv[1], &end, 0);

    if (*end != '\0' || timeout == 0)
    {
        fprintf(stderr,
            "%s: bad timeout argument: \"%s\"\n", argv[0], argv[1]);
        exit(1);
    }

    // Shutdown.

    try
    {
        ServerShutdownClient serverShutdownClient(&_serverRunStatus);
        serverShutdownClient.shutdown(timeout);

        MessageLoaderParms parms(
            "src.Server.cimserver.SERVER_STOPPED",
            "CIM Server stopped.");
        cout << MessageLoader::getMessage(parms) << endl;
    }
    catch (Exception& e)
    {
        cout << e.getMessage() << endl;
        exit(1);
    }

    return 0;
}
