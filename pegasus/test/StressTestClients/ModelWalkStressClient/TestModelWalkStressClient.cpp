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
// Author:  Aruran, IBM (aruran.shanmug@in.ibm.com)
//
// Modified By:
//
//%/////////////////////////////////////////////////////////////////////////////

#include "../TestStressTestClient.h"

PEGASUS_USING_PEGASUS;
PEGASUS_USING_STD;

Boolean verboseTest = false;

Boolean quit = false;
Boolean nextCheck = false;
int globalStatus = CLIENT_UNKNOWN;
char clientName[] = "ModelWalkStressClient";

String errorInfo;

/**
     Signal handler for SIGALARM.
     @param   signum - the alarm identifier.
*/
void endTest(int signum)
{
    PEGASUS_STD(cout) << "\nRecieved interrupt signal SIGINT!\n"
                      << PEGASUS_STD(endl);
    quit = true;
}

class TestModelWalkStressClient:public TestStressTestClient
{
};

/**
    GetNameSpaces
    If no nameSpace is supplied on the command line, this method looks into
    the root and test nameSpaces for the _Namespace class. The names of the
    instances of that class are retrued by this method.
*/
Array<CIMNamespaceName> getNameSpaces(
    TestModelWalkStressClient &tmsc,
    CIMClient* client,
    OptionManager &om,
    pid_t clientPid,
    String& clientLog,
    String &clientId,
    int status,
    String &pidFile)
{
    Array<CIMNamespaceName> topNamespaceNames;
    Array<CIMNamespaceName> returnNamespaces;
    String tmpNamespace;

    om.lookupValue("namespace",tmpNamespace);
    if (tmpNamespace != String::EMPTY)
    {
        returnNamespaces.append(CIMNamespaceName (tmpNamespace));
    }
    else
    {
        //
        // Get all namespaces for display using the __Namespaces function.
        //
        CIMName className = "__NameSpace";

        //
        // We have to append any new top level root namespace if created in
        // repository.
        //
        topNamespaceNames.append("root");
        topNamespaceNames.append("test");

        Uint32 start = 0;
        Uint32 end = topNamespaceNames.size();

        //
        // for all new elements in the output array.
        //
        for (Uint32 range = start; range < end; range ++)
        {
            //
            // Get the next increment in naming for all name element in
            // the array.
            //
            Array<CIMInstance> instances = client->enumerateInstances(
                topNamespaceNames[range], className);

            if (status != CLIENT_PASS)
            {
                status = CLIENT_PASS;
                tmsc.logInfo(clientId, clientPid, status, pidFile);
            }
            for (Uint32 i = 0 ; i < instances.size(); i++)
            {
                Uint32 pos;
                //
                // if we find the property and it is a string, use it.
                //
                if ((pos = instances[i].findProperty("name")) != PEG_NOT_FOUND)
                {
                    CIMValue value;
                    String namespaceComponent;
                    value = instances[i].getProperty(pos).getValue();
                    if (value.getType() == CIMTYPE_STRING)
                    {
                        value.get(namespaceComponent);
                        String ns = topNamespaceNames[range].getString();
                        ns.append("/");
                        ns.append(namespaceComponent);
                        returnNamespaces.append(ns);
                    }
                }
            }
        }
    }//else block ends here...

    if (verboseTest)
    {
        errorInfo.clear();
        errorInfo.append("+++++ Successfully Enumerated all Namespaces that ");
        errorInfo.append("have a _NameSpace instance defined for them in the");
        errorInfo.append(" root of test namespaces +++++");
        tmsc.errorLog(clientPid, clientLog, errorInfo);
        errorInfo.clear();
    }
    return returnNamespaces;
}
/**
    EnumerateAllQualifiers
    This method enumerates all the qualifiers in each of the nameSpaces
    of the "nameSpacesArray"
*/
static void enumerateAllQualifiers(
    TestModelWalkStressClient &tmsc,
    CIMClient* client,
    Array<CIMNamespaceName> nameSpacesArray,
    pid_t clientPid,
    String& clientLog,
    String &clientId,
    int status,
    String &pidFile)
{
    Array<CIMQualifierDecl> qualifierDecls;
    qualifierDecls.clear();

    for (Uint32 i=0; i < nameSpacesArray.size();i++)
    {
        try
        {
            qualifierDecls = client->enumerateQualifiers(nameSpacesArray[i]);

            if (status != CLIENT_PASS)
            {
                status = CLIENT_PASS;
                tmsc.logInfo(clientId, clientPid, status, pidFile);
            }
        }
        //
        // Following exceptions will be caught in the Main method.
        //
        catch (CIMException &e)
        {
            errorInfo.clear();
            errorInfo.append("CIMException in enumerateQualifiers : ");
            errorInfo.append(e.getMessage());
            CIMException cimExp(e.getCode(), errorInfo);
            throw cimExp;
        }
        catch (Exception &e)
        {
            errorInfo.clear();
            errorInfo.append("Exception in enumerateQualifiers : ");
            errorInfo.append(e.getMessage());
            Exception exp(errorInfo);
            throw exp;
        }
        catch (...)
        {
            Exception exp("General Exception in enumerateQualifiers ");
            throw exp;
        }
    }

    if (verboseTest)
    {
        errorInfo.clear();
        errorInfo.append("+++++ Successfully Enumerated Qualifiers for ");
        errorInfo.append("Namespaces +++++");
        tmsc.errorLog(clientPid, clientLog, errorInfo);
        errorInfo.clear();
    }
}
/**
    EnumerateReferenceNames
    This method enumerates the references to each instance in the array
    "cimInstances" for the "nameSpace"
*/
static void enumerateReferenceNames(
    TestModelWalkStressClient &tmsc,
    CIMClient* client,
    Array<CIMInstance> cimNInstances,
    CIMNamespaceName nameSpace,
    CIMName referenceClass,
    pid_t clientPid,
    String& clientLog,
    String &clientId,
    int status,
    String &pidFile)
{
    String role = String::EMPTY;
    Array<CIMObjectPath> resultObjectPaths;

    for (Uint32 i=0; i < cimNInstances.size(); i++)
    {
        try
        {
            resultObjectPaths = client->referenceNames(
                nameSpace, cimNInstances[i].getPath(),
                referenceClass, role);

            if (status != CLIENT_PASS)
            {
                status = CLIENT_PASS;
                tmsc.logInfo(clientId, clientPid, status, pidFile);
            }
        }
        //
        // Following exceptions will be caught in the Main method.
        //
        catch (CIMException &e)
        {
            errorInfo.clear();
            errorInfo.append("CIMException in enumerateReferenceNames : ");
            errorInfo.append(e.getMessage());
            CIMException cimExp(e.getCode(), errorInfo);
            throw cimExp;
        }
        catch (Exception &e)
        {
            errorInfo.clear();
            errorInfo.append("Exception in enumerateReferenceNames : ");
            errorInfo.append(e.getMessage());
            Exception exp(errorInfo);
            throw exp;
        }
        catch (...)
        {
            Exception exp("General Exception in enumerateReferenceNames ");
            throw exp;
        }
    }
}
/**
    EnumerateAssociatorNames
    This method enumerates the associators to each instance in the array
    "cimInstances" for the "nameSpace"
*/
static void enumerateAssociatorNames(
    TestModelWalkStressClient &tmsc,
    CIMClient* client,
    Array<CIMInstance> cimNInstances,
    CIMNamespaceName nameSpace,
    CIMName assocClass,
    pid_t clientPid,
    String& clientLog,
    String &clientId,
    int status,
    String &pidFile)
{
    CIMName resultClass = CIMName();
    String role = String::EMPTY;
    String resultRole = String::EMPTY;
    Array<CIMObjectPath> resultObjectPaths;

    for (Uint32 i=0; i < cimNInstances.size(); i++)
    {
        try
        {
            resultObjectPaths = client->associatorNames(
                nameSpace, cimNInstances[i].getPath(),
                assocClass, resultClass, role, resultRole);

            if (status != CLIENT_PASS)
            {
                status = CLIENT_PASS;
                tmsc.logInfo(clientId, clientPid, status, pidFile);
            }
        }
        //
        // Following exceptions will be caught in the Main method.
        //
        catch (CIMException &e)
        {
            errorInfo.clear();
            errorInfo.append("CIMException in enumerateAssociatorNames : ");
            errorInfo.append(e.getMessage());
            CIMException cimExp(e.getCode(), errorInfo);
            throw cimExp;
        }
        catch (Exception &e)
        {
            errorInfo.clear();
            errorInfo.append("Exception in enumerateAssociatorNames : ");
            errorInfo.append(e.getMessage());
            Exception exp(errorInfo);
            throw exp;
        }
        catch (...)
        {
            Exception exp("General Exception in enumerateAssociatorNames ");
            throw exp;
        }
    }
}
/**
    EnumerateInstanceRelatedInfo
    This method enumerates instances, referances (by way of subrotine) and
    associators (by way of subrotine.Exceptions are caught, modified and
    re-thrown so that generic exception handling can be used in Main
*/
static void enumerateInstanceRelatedInfo(
    TestModelWalkStressClient &tmsc,
    CIMClient *client,
    Array<CIMName> classNames,
    CIMNamespaceName nameSpace,
    pid_t clientPid,
    String& clientLog,
    String &clientId,
    int status,
    String &pidFile )
{
    Boolean deepInheritance = true;
    Boolean localOnly = true;
    Boolean includeQualifiers = false;
    Boolean includeClassOrigin = false;
    Array<CIMInstance> cimNInstances;

    for (Uint32 i=0; i < classNames.size(); i++)
    {
        try
        {
            cimNInstances = client->enumerateInstances(
                nameSpace, classNames[i], deepInheritance,
                localOnly, includeQualifiers, includeClassOrigin);

            if (status != CLIENT_PASS)
            {
                status = CLIENT_PASS;
                tmsc.logInfo(clientId, clientPid, status, pidFile);
            }
        }
        //
        // Following exceptions will be caught in the Main method.
        //
        catch (CIMException &e)
        {
            errorInfo.clear();
            errorInfo.append("CIMException thrown from ");
            errorInfo.append("enumerateInstanceRelatedInfo, class = ");
            errorInfo.append(classNames[i].getString());
            errorInfo.append("  ");
            errorInfo.append(e.getMessage());
            CIMException cimExp(e.getCode(), errorInfo);
            throw cimExp;
        }
        catch (Exception &e)
        {
            errorInfo.clear();
            errorInfo.append("Exception thrown from ");
            errorInfo.append("enumerateInstanceRelatedInfo, class = ");
            errorInfo.append(classNames[i].getString());
            errorInfo.append("  ");
            errorInfo.append(e.getMessage());
            Exception exp(errorInfo);
            throw exp;
        }
        catch (...)
        {
            Exception exp("General Exception in EnumerateInstanceRelatedInfo ");
            throw exp;
        }

        enumerateReferenceNames (tmsc, client, cimNInstances, nameSpace,
            CIMName(), clientPid, clientLog, clientId, status, pidFile);
        enumerateAssociatorNames (tmsc, client, cimNInstances, nameSpace,
            CIMName(), clientPid, clientLog, clientId, status, pidFile);

        if (verboseTest)
        {
            errorInfo.clear();
            errorInfo.append("+++++ Successfully Done Instance Operations ");
            errorInfo.append("on class = ");
            errorInfo.append(classNames[i].getString());
            tmsc.errorLog(clientPid, clientLog, errorInfo);
            errorInfo.clear();
        }
    }

}
/**
    EnumerateClassRelatedInfo
    This method enumerates classes and instances (by way of subrotine)
*/
static void enumerateClassRelatedInfo(
    TestModelWalkStressClient &tmsc,
    CIMClient* client,
    OptionManager &om,
    Array<CIMNamespaceName> nameSpacesArray,
    pid_t clientPid,
    String& clientLog,
    String &clientId,
    int status,
    String &pidFile)
{
    Array<CIMName> classNames;
    String tmpClassName;

    om.lookupValue("classname",tmpClassName);
    if (tmpClassName != String::EMPTY)
    {
        classNames.append(CIMName(tmpClassName));
        enumerateInstanceRelatedInfo(tmsc, client, classNames,
            *nameSpacesArray.getData(), clientPid, clientLog, clientId,
            status, pidFile);
    }
    else
    {
        Boolean deepInheritance = true;
        CIMName className;
        for (Uint32 i=0; i < nameSpacesArray.size();i++)
        {
            classNames = client->enumerateClassNames(nameSpacesArray[i],
                className, deepInheritance);

            if (status != CLIENT_PASS)
            {
                status = CLIENT_PASS;
                tmsc.logInfo(clientId, clientPid, status, pidFile);
            }
            enumerateInstanceRelatedInfo(tmsc, client, classNames,
                nameSpacesArray[i], clientPid, clientLog, clientId, status,
                pidFile );
        }
    }
      if (verboseTest)
    {
        errorInfo.clear();
        errorInfo.append("+++++ Successfully Enumerated classes ");
        errorInfo.append("in Namespaces +++++");
        tmsc.errorLog(clientPid, clientLog, errorInfo);
        errorInfo.clear();
    }
}
/**
    MAIN
*/

int main(int argc, char** argv)
{
    OptionManager om;
    TestModelWalkStressClient tmsc;
    Uint32 validArg = 0;
    struct OptionRow *newOptionsTable = 0;
    Uint32 newOptionsCount = 0;
    Uint32 cOptionCount =  0;

    //
    // Variables need to connect to server.
    //
    Boolean useSSL;
    String host;
    Uint32 portNumber = 0;
    String portStr;
    String userName;
    String password;
    String help;
    Boolean connectedToHost = false;

    //
    // Variables needed to do loging and status checking.
    //
    String pidFile;
    String clientId;
    pid_t clientPid;
    String clientLog, stopClient;
    char pid_str[15];

    //
    // Number of times the command succeeded.
    //
    Uint32 successCount=0;

    //
    // Number of iterations after which logErrorPercentage() is called.
    //
    Uint32 iteration=0;

    //
    // Total number of times the command was executed.
    //
    Uint32 totalCount=0;

    //
    // timeout.
    //
    Uint32 timeout = 30000;

    //
    // This try block includes all the options gathering function.
    //
    try
    {
        //
        // client specific options if any.
        //
        struct OptionRow *cOptionTable = 0;
        newOptionsCount = cOptionCount;

        try
        {
            //
            // Generate new option table for this client using
            // the OptionManager.
            //

            newOptionsTable = tmsc.generateClientOptions(cOptionTable,
                cOptionCount,newOptionsCount);
            validArg = tmsc.GetOptions(om, argc, argv, newOptionsTable,
                newOptionsCount);
        }
        catch (Exception& e)
        {
            PEGASUS_STD(cerr) << argv[0] << ": " << e.getMessage()
                              << PEGASUS_STD(endl);
            exit(1);
        }
        catch (...)
        {
            PEGASUS_STD(cerr) << argv[0] << ": Error in Options operation "
                              << PEGASUS_STD(endl);
            exit(1);
        }

        verboseTest = om.isTrue("verbose");

        om.lookupValue("username", userName);

        om.lookupValue("password", password);

        om.lookupIntegerValue("timeout",timeout);

        useSSL =  om.isTrue("ssl");

        om.lookupValue("hostname",host);
        om.lookupValue("port",portStr);

        if (portStr.getCString())
        {
            sscanf (portStr.getCString (), "%u", &portNumber);
        }

        //
        // Setting default ports.
        //
        if (!portNumber)
        {
            if (useSSL)
            {
                portNumber = 5989;
            }
            else
            {
                portNumber = 5988;
            }
        }

        //
        // default host is localhost.
        //
        if (host == String::EMPTY)
        {
            host = String("localhost");
        }

        //
        // Signal Handling - SIGINT
        //
        signal(SIGINT, endTest);

        om.lookupValue("clientid", clientId);

        om.lookupValue("pidfile", pidFile);

        clientPid = getpid();

        om.lookupValue("clientlog", clientLog);

        om.lookupValue("help", help);

    }// end of option Try block
    catch (Exception& e)
    {
        PEGASUS_STD(cerr) << argv[0] << ": " << e.getMessage()
                          << PEGASUS_STD(endl);
        exit(1);
    }
    catch (...)
    {
        PEGASUS_STD(cerr) << argv[0] << ": Unknown Error gathering options "
                          << PEGASUS_STD(endl);
        exit(1);
    }

    /* Checking whether the user asked for HELP Info...
    if (om.valueEquals("help", "true"))
    {
        String header = "Usage ";
        header.append(argv[0]);
        header.append(" -parameters -n [namespace] -c [classname] ");
        header.append(" -clientId [clientId] -pidFile [pidFile] ");
        header.append(" -clientLog [clientLog]");
        String trailer = "Assumes localhost:5988 if host not specified";
        trailer.append("\nHost may be of the form name or name:port");
        trailer.append("\nPort 5988 assumed if port number missing.");
        om.printOptionsHelpTxt(header, trailer);
        exit(0);
    } */

    try
    {
        //
        // Timer Start.
        //
        tmsc.startTime();

        tmsc.logInfo(clientId, clientPid, globalStatus, pidFile);

        //
        // connect the client.
        //
        CIMClient* client = new CIMClient();

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
                    tmsc.errorLog(clientPid, clientLog, mes);
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

            if (!connectedToHost)
            {
                try
                {
                    tmsc.connectClient( client, host, portNumber, userName,
                        password, useSSL, timeout, verboseTest);
                    connectedToHost = true;

                   //
                   // Client has successfully connected to server.
                   // update status if previously not Success.
                   //
                   if (globalStatus != CLIENT_PASS)
                   {
                       globalStatus = CLIENT_PASS;
                       tmsc.logInfo(clientId, clientPid, globalStatus, pidFile);
                   }
                }
                catch (CannotConnectException)
                {
                    globalStatus = CLIENT_UNKNOWN;
                    tmsc.logInfo(clientId, clientPid, globalStatus, pidFile);
                    connectedToHost = false;
                }
                catch (CIMException &e)
                {
                    globalStatus = CLIENT_UNKNOWN;
                    tmsc.logInfo(clientId, clientPid, globalStatus, pidFile);
                    connectedToHost = false;
                    String mes(e.getMessage());
                    tmsc.errorLog(clientPid, clientLog, mes);
                }
                catch (Exception &e)
                {
                    globalStatus = CLIENT_UNKNOWN;
                    tmsc.logInfo(clientId, clientPid, globalStatus, pidFile);
                    connectedToHost = false;
                    String mes(e.getMessage());
                    tmsc.errorLog(clientPid, clientLog, mes);
                }
                catch (...)
                {
                    globalStatus = CLIENT_UNKNOWN;
                    tmsc.logInfo(clientId, clientPid, globalStatus, pidFile);
                    connectedToHost = false;
                    String mes("Error connencting to server in ModleWalk");
                    mes.append(" client ");
                    tmsc.errorLog(clientPid, clientLog,mes);
                }
            }/* end of if (!connectedToHost). */
            iteration++;
            totalCount++;
            if (connectedToHost)
            {
                try
                {
                    Array<CIMNamespaceName> nameSpacesArray;

                    //
                    // Enumerate all the namespaces here.
                    //
                    nameSpacesArray = getNameSpaces(
                                          tmsc,
                                          client,
                                          om,
                                          clientPid,
                                          clientLog,
                                          clientId,
                                          globalStatus,
                                          pidFile);

                    //
                    // Enumerate all qualifiers in the namespaces.
                    //
                    enumerateAllQualifiers(
                        tmsc,
                        client,
                        nameSpacesArray,
                        clientPid,
                        clientLog,
                        clientId,
                        globalStatus,
                        pidFile);

                    //
                    // Enumerate all the class related info here.
                    //
                    enumerateClassRelatedInfo(
                        tmsc,
                        client,
                        om,
                        nameSpacesArray,
                        clientPid,
                        clientLog,
                        clientId,
                        globalStatus,
                        pidFile);

                    successCount++;
                }
                //
                // This specail catch block in needed so that we will know if
                // the connenction was lost. We then connect on the next time
                // through the loop.
                //

                catch (CannotConnectException)
                {
                    globalStatus = CLIENT_UNKNOWN;
                    tmsc.logInfo(clientId, clientPid, globalStatus, pidFile);
                    connectedToHost = false;
                }
                catch (CIMException &cimE)
                {
                    globalStatus = CLIENT_FAIL;
                    tmsc.logInfo(clientId, clientPid, globalStatus, pidFile);
                    String mes(cimE.getMessage());
                    tmsc.errorLog(clientPid, clientLog, mes);
                }
                catch (Exception &exp)
                {
                    globalStatus = CLIENT_FAIL;
                    tmsc.logInfo(clientId, clientPid, globalStatus, pidFile);
                    String mes(exp.getMessage());
                    tmsc.errorLog(clientPid, clientLog, mes);
                }
                catch (...)
                {
                    globalStatus = CLIENT_FAIL;
                    tmsc.logInfo(clientId, clientPid, globalStatus, pidFile);
                    String mes("Unknown Error during ModelWalk Execution");
                    tmsc.errorLog(clientPid, clientLog, mes);
                }

                nextCheck = tmsc.checkTime();
                if (nextCheck)
                {
                    tmsc.logInfo(clientId, clientPid, globalStatus, pidFile);
                    nextCheck = false;
                }

                //
                // If verbose is set, log success percentage for every 100
                // iterations. If verbose is not set, log success percentage
                // for every 10000 iterations.
                //
                if (verboseTest)
                {
                    if (iteration == 100)
                    {
                        tmsc.logErrorPercentage(
                            successCount,
                            totalCount,
                            clientPid,
                            clientLog,
                            clientName);
                        iteration = 0;
                    }
                }
                else
                {
                    if (iteration == 1000)
                    {
                        tmsc.logErrorPercentage(
                            successCount,
                            totalCount,
                            clientPid,
                            clientLog,
                            clientName);
                        iteration = 0;
                    }
                }
            }
        } // end of while (!quit).
    }
    catch (Exception &exp)
    {
        String expStr("Exception in ModelWalk client, causing it to exit: ");
        expStr.append(exp.getMessage());
        tmsc.errorLog(clientPid, clientLog, expStr);

        if (verboseTest)
        {
            PEGASUS_STD(cerr) << expStr.getCString() << PEGASUS_STD(endl);
        }
    }
    catch (...)
    {
        String expStr("General Exception in ModelWalk, causing it to exit");
        tmsc.errorLog(clientPid, clientLog, expStr);

        if (verboseTest)
        {
            PEGASUS_STD(cerr) << expStr.getCString() << PEGASUS_STD(endl);
        }
    }

//
// One second delay before shutdown.
//
#ifndef PEGASUS_OS_TYPE_WINDOWS
    sleep(1);
#else
    Sleep(1000);
#endif
    if (FileSystem::exists(stopClient))
    {
        //
        // Remove STOP file here.
        //
        FileSystem::removeFile(stopClient);
    }

    if (verboseTest)
    {
        errorInfo.clear();
        errorInfo.append("+++++ TestModelWalkStressClient Terminated ");
        errorInfo.append("Normally +++++");
        tmsc.errorLog(clientPid, clientLog, errorInfo);
        errorInfo.clear();
    }
    return 0;
}
