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
// Author:  Melvin, IBM (msolomon@in.ibm.com) for PEP# 241
//
// Modified By:
//
//%/////////////////////////////////////////////////////////////////////////////

#include "../TestStressTestClient.h"

PEGASUS_USING_PEGASUS;
PEGASUS_USING_STD;

class TestWrapperStressClient : public TestStressTestClient
{
};

Boolean quit = false;
Boolean nextCheck = false;
String errorInfo;
char thisClient[] = "WrapperStressClient";

/**
    Signal handler for SIGALARM.
    @param signum - the alarm identifier
 */
void endTest(int signum)
{
    cout<<"Recieved interupt signal SIGINT!\n"<<endl;
    quit = true;
}

/**
    MAIN
*/
int main(int argc, char** argv)
{
    OptionManager om;
    TestWrapperStressClient wc;
    pid_t clientPid = getpid();
    Uint32 validArg = 0;
    Boolean verboseTest;

    //
    // Varriables needed for loging and status checking.
    //
    String clientId;
    String pidFile;
    String clientLog, stopClient;
    char pid_str[15];
    int status = CLIENT_UNKNOWN;

    //
    // Number of times the client command succeeded.
    //
    Uint32 successCount = 0;

    //
    // Number of iterations after which logErrorPercentage() is called.
    //
    Uint32 iteration = 0;

    //
    // Total number of times the client command was executed.
    //
    Uint32 totalCount = 0;

    //
    // Variables needed for Command operation.
    //
    String command;

    String options;
    String help;

    try
    {
        struct OptionRow *newOptionsTable = 0;
        Uint32 newOptionsCount;

        struct OptionRow cOptionTable[] =
        {
            {"clientname", "", true, Option::STRING, 0, 0, "clientname",
                "Client name" },

            {"options", "", true, Option::STRING, 0, 0, "options",
                "Corresponding Client program's options" }
        };

        Uint32 cOptionCount = sizeof(cOptionTable) / sizeof(cOptionTable[0]);
        newOptionsCount = cOptionCount;

        try
        {
            //
            // Generate new option table for this client using OptionManager
            //
            newOptionsTable = wc.generateClientOptions(
                                  cOptionTable,
                                  cOptionCount,
                                  newOptionsCount);
            validArg = wc.GetOptions(
                           om,
                           argc,
                           argv,
                           newOptionsTable,
                           newOptionsCount);
        }
        catch (Exception& e)
        {
            cerr << argv[0] << ": " << e.getMessage() << endl;
            exit(1);
        }
        catch (...)
        {
            cerr << argv[0] << ": Error in Options operations "<< endl;
            exit(1);
        }

        verboseTest = om.isTrue("verbose");

        om.lookupValue("clientid", clientId);

        om.lookupValue("pidfile", pidFile);

        om.lookupValue("clientlog", clientLog);

        om.lookupValue("clientname", command);

        om.lookupValue("options", options);

        om.lookupValue("help", help);
    } /** end of option Try block. */
    catch (Exception& e)
    {
        cerr << argv[0] << ": " << e.getMessage() <<endl;
        exit(1);
    }
    catch (...)
    {
        cerr << argv[0] << ": Unknown Error gathering options "
             << "in Wrapper Client " << endl;
        exit(1);
    }

    /** Checking whether the user asked for HELP Info...
    if (om.valueEquals("help", "true"))
    {
        String header = "Usage ";
        header.append(argv[0]);
        header.append(" -parameters -clientName [clientName]
        header.append(" -options [options] -clientid [clientId] ");
        header.append(" -pidfile [pidFile] -clientlog [clientLog]");
        String trailer = "Assumes localhost:5988 if host not specified";
        trailer.append("\nHost may be of the form name or name:port");
        trailer.append("\nPort 5988 assumed if port number missing.");
        om.printOptionsHelpTxt(header, trailer);
        exit(0);
    }
    */

    try
    {
        if (options.getCString())
        {
            command.append(" " + options);
        }
        if (verboseTest)
        {
            errorInfo.append("client command :  ");
            errorInfo.append(command);
            wc.errorLog(clientPid, clientLog, errorInfo);
            errorInfo.clear();
        }

        //
        // Signal Handling - SIGINT.
        //
        signal(SIGINT, endTest);

        //
        // Timer Start.
        //
        wc.startTime();

        wc.logInfo(clientId, clientPid, status, pidFile);
        sprintf(pid_str, "%d", clientPid);

        stopClient = String::EMPTY;
        stopClient.append(FileSystem::extractFilePath(pidFile));
        stopClient.append("STOP_");
        stopClient.append(pid_str);

        //
        // This loop executes till the client gets stop signal from
        // controller.
        //
        while (!quit)
        {
            if (FileSystem::exists(stopClient))
            {
                if (verboseTest)
                {
                    String mes("Ending client. ");
                    wc.errorLog(clientPid, clientLog, mes);
                }
                break;
            }

                if (!verboseTest)
                {
#ifdef PEGASUS_OS_TYPE_WINDOWS
                    freopen("nul","w",stdout);
#else
                    FILE * file = freopen("/dev/null","w",stdout);
                    if (0 == file)
                    {
                        // ignore possible error, not having stdout redirected
                        // to /dev/null doesn't hurt the test
                        continue;
                    }
#endif
                }

            int i = system(command.getCString());

            iteration++;
            totalCount++;
            switch (i)
            {
                case 0:
                    if (status != CLIENT_PASS)
                    {
                        status = CLIENT_PASS;
                        wc.logInfo(clientId, clientPid, status, pidFile);
                    }
                    successCount++;
                    break;

                case 1:
                    status = CLIENT_UNKNOWN;
                    wc.logInfo(clientId, clientPid, status, pidFile);
                    break;

                default:
                    status = CLIENT_FAIL;
                    wc.logInfo(clientId, clientPid, status, pidFile);
                    break;
            }

            nextCheck = wc.checkTime();
            if (nextCheck)
            {
                wc.logInfo(clientId, clientPid, status, pidFile);
                nextCheck = false;
            }

            //
            // If verbose is set, log success percentage for every 100
            // iterations.  If verbose is not set, log success percentage
            // for every 1000 iterations.
            //
            if (verboseTest)
            {
                if (iteration == 100)
                {
                    wc.logErrorPercentage(
                        successCount,
                        totalCount,
                        clientPid,
                        clientLog,
                        thisClient);
                    iteration = 0;
                }
            }
            else
            {
                if (iteration == 1000)
                {
                    wc.logErrorPercentage(
                        successCount,
                        totalCount,
                        clientPid,
                        clientLog,
                        thisClient);
                    iteration = 0;
                }
            }
        } /** end of while(!quit). */
    } /** end of command execution try block. */
    catch (Exception &exp)
    {
        String expStr("Exception in WrapperClient causing it to exit: ");
        expStr.append(exp.getMessage());
        wc.errorLog(clientPid, clientLog, expStr);

        if (verboseTest)
        {
            cerr << expStr.getCString() << endl;
        }
    }
    catch (...)
    {
        String expStr("General Exception in WrapperClient causing it to exit");
        wc.errorLog(clientPid, clientLog, expStr);

        if (verboseTest)
        {
            cerr << expStr.getCString() << endl;
        }
    }

//
// second delay before shutdown.
//
#ifndef PEGASUS_OS_TYPE_WINDOWS
    sleep(1);
#else
    Sleep(1000);
#endif

    if(FileSystem::exists(stopClient))
    {
        //
        // Remove STOP file here.
        //
        FileSystem::removeFile(stopClient);
    }
    if (verboseTest)
    {
        errorInfo.append(
            "++++ TestWrapperStressClient Terminated Normally +++++");
        wc.errorLog(clientPid, clientLog, errorInfo);
        errorInfo.clear();
    }
    return 0;
}
