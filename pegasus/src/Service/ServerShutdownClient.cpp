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
#include <Pegasus/Common/Logger.h>
#include <Pegasus/Common/System.h>
#include <Pegasus/Common/Tracer.h>
#include <Pegasus/Client/CIMClient.h>
#include <Service/ServerShutdownClient.h>

PEGASUS_USING_STD;

PEGASUS_NAMESPACE_BEGIN

ServerShutdownClient::ServerShutdownClient(
    ServerRunStatus* serverRunStatus)
    : _serverRunStatus(serverRunStatus)
{
}

ServerShutdownClient::~ServerShutdownClient()
{
}

//
//  Waits until either the CIM Server has terminated, or the shutdown timeout
//  has expired.  If the shutdown timeout has expired, and the CIM Server is
//  still running, kills the cimserver process.
//
void ServerShutdownClient::_waitForTerminationOrTimeout(Uint32 maxWaitTime)
{
    //
    //  If the CIM Server is still running, and the shutdown timeout has not
    //  expired, loop and wait one second until either the CIM Server has
    //  terminated, or the shutdown timeout has expired
    //
    Boolean running = _serverRunStatus->isServerRunning();
    while (running && (maxWaitTime > 0))
    {
        System::sleep(1);
        running = _serverRunStatus->isServerRunning();
        maxWaitTime--;
    }

    //
    //  If the shutdown timeout has expired, and the CIM Server is still
    //  running, kill the cimserver process
    //
    if (running)
    {
        Boolean wasKilled = _serverRunStatus->kill();

#if defined(PEGASUS_OS_HPUX) || defined(PEGASUS_PLATFORM_LINUX_GENERIC_GNU) \
|| defined(PEGASUS_OS_ZOS) || defined(PEGASUS_OS_SOLARIS) \
|| defined (PEGASUS_OS_VMS)
        if (wasKilled)
        {
            MessageLoaderParms parms(
                "src.Server.cimserver.TIMEOUT_EXPIRED_SERVER_KILLED",
                "Shutdown timeout expired.  Forced shutdown initiated.");
            Logger::put_l(Logger::ERROR_LOG, System::CIMSERVER, Logger::SEVERE,
                parms);
            cout << MessageLoader::getMessage(parms) << endl;
            exit (0);
        }
#endif
    }
}

void ServerShutdownClient::shutdown(Uint32 timeoutValue)
{
    //
    // Create CIMClient object
    //
    CIMClient client;

    //
    // Get local host name
    //
    String hostStr = System::getHostName();

    //
    // open connection to CIMOM
    //
    try
    {
        client.connectLocal();

        //
        // set client timeout to 2 seconds
        //
        client.setTimeout(2000);
    }
    catch(Exception&)
    {
        MessageLoaderParms parms(
            "src.Server.cimserver.UNABLE_CONNECT_SERVER_MAY_NOT_BE_RUNNING",
            "Unable to connect to CIM Server.  CIM Server may not be running.");
        PEGASUS_STD(cerr) << MessageLoader::getMessage(parms) <<
            PEGASUS_STD(endl);
        exit(1);
    }

    try
    {
        //
        // construct CIMObjectPath
        //
        String referenceStr = "//";
        referenceStr.append(hostStr);
        referenceStr.append("/");
        referenceStr.append(PEGASUS_NAMESPACENAME_SHUTDOWN.getString());
        referenceStr.append(":");
        referenceStr.append(PEGASUS_CLASSNAME_SHUTDOWN.getString());
        CIMObjectPath reference(referenceStr);

        //
        // issue the invokeMethod request on the shutdown method
        //
        Array<CIMParamValue> inParams;
        Array<CIMParamValue> outParams;

        // set force option to true for now
        inParams.append(CIMParamValue("force",
            CIMValue(Boolean(true))));

        inParams.append(CIMParamValue("timeout",
            CIMValue(Uint32(timeoutValue))));

        CIMValue retValue = client.invokeMethod(
            PEGASUS_NAMESPACENAME_SHUTDOWN,
            reference,
            "shutdown",
            inParams,
            outParams);
    }
    catch(CIMException& e)
    {
        //l10n - TODO
        MessageLoaderParms shutdownFailedMsgParms(
            "src.Server.cimserver.SHUTDOWN_FAILED",
            "Error in server shutdown: ");
        PEGASUS_STD(cerr) << MessageLoader::getMessage(shutdownFailedMsgParms);
        if (e.getCode() == CIM_ERR_INVALID_NAMESPACE)
        {
            //
            // Repository may be empty.
            //
            Logger::put_l(Logger::ERROR_LOG, System::CIMSERVER, Logger::SEVERE,
                MessageLoaderParms(
                    "src.Server.cimserver.SHUTDOWN_FAILED_REPOSITORY_EMPTY",
                    "Error in server shutdown: The repository may be empty."));
            MessageLoaderParms parms(
                "src.Server.cimserver.REPOSITORY_EMPTY",
                "The repository may be empty.");
            PEGASUS_STD(cerr) << MessageLoader::getMessage(parms) <<
                PEGASUS_STD(endl);
        }
        else
        {
            Logger::put_l(Logger::ERROR_LOG, System::CIMSERVER, Logger::SEVERE,
                MessageLoaderParms(
                    "src.Server.cimserver.SHUTDOWN_FAILED",
                    "Error in server shutdown: $0", e.getMessage()));
            PEGASUS_STD(cerr) << e.getMessage() << PEGASUS_STD(endl);
        }

        // Kill the cimserver process
        if (_serverRunStatus->kill())
        {
            MessageLoaderParms parms(
                "src.Server.cimserver.SERVER_FORCED_SHUTDOWN",
                "Forced shutdown initiated.");
            Logger::put_l(Logger::ERROR_LOG, System::CIMSERVER, Logger::SEVERE,
                parms);
            PEGASUS_STD(cerr) << MessageLoader::getMessage(parms) <<
                PEGASUS_STD(endl);
        }
        exit(1);
    }
    catch(Exception&)
    {
        //
        // This may mean that the CIM Server has terminated, causing this
        // client to get a "Empty HTTP response message" exception.  It may
        // also mean that the CIM Server is taking longer than 2 seconds
        // (client timeout value) to terminate, causing this client to
        // timeout with a "connection timeout" exception.
        //
        //  Wait until either the CIM Server has terminated, or the shutdown
        //  timeout has expired.  If the timeout has expired and the CIM Server
        //  is still running, kill the cimserver process.
        //
        _waitForTerminationOrTimeout(timeoutValue - 2);
        return;
    }

    //
    //  InvokeMethod succeeded.
    //  Wait until either the CIM Server has terminated, or the shutdown
    //  timeout has expired.  If the timeout has expired and the CIM Server
    //  is still running, kill the cimserver process.
    //
    _waitForTerminationOrTimeout(timeoutValue);
}

PEGASUS_NAMESPACE_END
