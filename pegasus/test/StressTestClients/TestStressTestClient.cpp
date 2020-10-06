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

#include "TestStressTestClient.h"

PEGASUS_USING_PEGASUS;
PEGASUS_USING_STD;

/** Constructor - Initializes the time variables and defines the Options
    Table.
*/
TestStressTestClient::TestStressTestClient()
{
    startMilliseconds = 0;
    nowMilliseconds = 0;
    nextCheckupInMillisecs = 0;
    static struct OptionRow testOptionsTable[] =  {
        {"namespace", "", false, Option::STRING, 0, 0, "namespace",
                        "specifies namespace to be used for the test"},

        {"classname", "", false, Option::STRING, 0, 0, "classname",
                        "specifies classname to be used for the test"},

        {"verbose", "false", false, Option::BOOLEAN, 0, 0, "verbose",
                                  "If set, outputs extra information "},

        {"help", "false", false, Option::BOOLEAN, 0, 0, "help",
                      "Prints help message with command line options "},

#ifdef PEGASUS_HAS_SSL
        {"ssl", "false", false, Option::BOOLEAN, 0, 0, "ssl", "use SSL"},
#endif

        {"username", "", false, Option::STRING, 0, 0, "username",
                                               "Specifies user name"},

        {"password", "", false, Option::STRING, 0, 0, "password",
                                                "Specifies password"},

        {"port", "", false, Option::STRING, 0, 0, "port",
                                               "Port number on host"},

        {"clientid", "", true, Option::STRING, 0, 0, "clientid",
                               "Client identification by controller"},

        {"pidfile", "", true, Option::STRING, 0, 0, "pidfile",
                                           "Client process log file"},

        {"clientlog", "", true, Option::STRING, 0, 0, "clientlog",
                                             "Client error log file"},

        {"hostname", "", false, Option::STRING, 0, 0, "hostname",
                                                         "Host name"}
    };
    optionCount = sizeof(testOptionsTable) / sizeof(testOptionsTable[0]);
    optionsTable = testOptionsTable;
 }
/**
    OPTION MANAGEMENT
*/

/** GetOptions function - This function sets up the options from
    testOptionsTable which is initialized through constructor
    using the option manager.
    const char* optionName;
    const char* defaultValue;
    int required;
    Option::Type type;
    char** domain;
    Uint32 domainSize;
    const char* commandLineOptionName;
    const char* optionHelpMessage;
*/
int TestStressTestClient::GetOptions(
    OptionManager& om,
    int& argc,
    char** argv,
    OptionRow* newOptionsTable, Uint32 cOptionCount)
{
    char **argvv;
    int counter = 0;
    String argument = String::EMPTY;


    //
    // om.registerOptions(newOptionsTable, (const)cOptionCount);
    //
    om.registerOptions(newOptionsTable, cOptionCount);
    argvv = argv;

    //
    // Following section is introduced to ignore  options not
    // required by a client.
    //
    for (int i = 1; i < argc; i++)
    {
        argument = String::EMPTY;
        const char* arg = argv[i];

        //
        // Check for - option.
        //
        if (*arg == '-')
        {
            //
            // Look for the option.
            //
            argument.append(arg + 1);
            const Option* option = om.lookupOption(argument);

            //
            // Get the option argument if any.
            //
            if (option)
            {
                argvv[++counter]=argv[i];
                if (option->getType() != Option::BOOLEAN)
                {
                    if (i + 1 != argc)
                    {
                        argvv[++counter] = argv[++i];
                    }
                }
            }
        }
    }
    ++counter;
    om.mergeCommandLine(counter, argvv);
    om.checkRequiredOptions();
    return counter;
}

/** This method is used by clients to register client specific required
    options to the option table. All these options are taken as mandatory one.
*/
OptionRow *TestStressTestClient::generateClientOptions(
    OptionRow* clientOptionsTable,
    Uint32 clientOptionCount,
    Uint32& totalOptions)
{

    Uint32 currOptionCount = 0;
    static struct OptionRow *newOptionsTable = 0;

    totalOptions = optionCount + clientOptionCount;

    newOptionsTable = new struct OptionRow[totalOptions];

    for (; currOptionCount < optionCount; currOptionCount++)
    {
        newOptionsTable[currOptionCount] = optionsTable[currOptionCount];
    }
    for (Uint32 i =0; i < clientOptionCount; i++)
    {
        newOptionsTable[currOptionCount] = clientOptionsTable[i];
        currOptionCount++;
    }
    return newOptionsTable;
}

/** This method is used by the clients to log information which are required
    for controller reference. It logs the information with Client ID and
    status of the client in the PID File log file.
*/
void TestStressTestClient::logInfo(
    String clientId,
    pid_t clientPid,
    int clientStatus,
    String &pidFile)
{
    char pid_str[15];
    char status_str[15];
    char time_str[32];

#ifdef PEGASUS_OS_TYPE_WINDOWS
    int offset = 2;
#else
    int offset = 1;
#endif

    //
    //  Get current time for time stamp
    //
    nowMilliseconds = TimeValue::getCurrentTime().toMilliseconds();

    sprintf(pid_str, "%d", clientPid);
    sprintf(status_str, "%d", clientStatus);
    sprintf(time_str,  "%" PEGASUS_64BIT_CONVERSION_WIDTH "u",
        nowMilliseconds);

    fstream pFile(pidFile.getCString(),ios::in|ios::out);

    Boolean addClient= false;

    if (!!pFile)
    {
        String line;
        while(!pFile.eof() && GetLine(pFile, line))
        {
            String subLine;

            Uint32 indx=line.find(':');
            if (indx != PEG_NOT_FOUND)
            {
                subLine = line.subString(0, indx);
            }

            if (String::compare(subLine, clientId) == 0)
            {
                long pos;
                addClient = true;
                pos = (long)pFile.tellp();
                pFile.seekp(pos - line.size()-offset);
                String newLine = clientId;
                newLine.append("::");
                newLine.append(pid_str);
                newLine.append("::");
                newLine.append(status_str);
                newLine.append("::");
                newLine.append(time_str);

                Sint32 jSize = line.size() - newLine.size();
                CString newLineCString = newLine.getCString();
                pFile.write(newLineCString, strlen(newLineCString));
                for (Sint32 i = 0; i < jSize; i++)
                {
                    pFile.write(" ",1);
                }
                break;
            }
        }
        if(!addClient)
        {
            pFile.close();
            fstream newPidFile(pidFile.getCString(),ios::out|ios::app);
            newPidFile<<clientId<<"::"<<clientPid<<"::"<<clientStatus<<"::"
                 <<time_str<<"\n";
        }
    }
    pFile.close();
}

/** This method is used to take the client process start time.*/
void TestStressTestClient::startTime()
{
    startMilliseconds = TimeValue::getCurrentTime().toMilliseconds();
    nowMilliseconds = startMilliseconds;
}

/** This method is used to check the time stamp for logging information about
    the success or failure.
*/
Boolean TestStressTestClient::checkTime()
{
    nowMilliseconds = TimeValue::getCurrentTime().toMilliseconds();

    if (nowMilliseconds >= nextCheckupInMillisecs)
    {
        nextCheckupInMillisecs = (Uint64)convertmin2millisecs(CHECKUP_INTERVAL)
            + nowMilliseconds;
        return true;
    }
    return false;
}

/** This method is used to log the information about the client's success or
    failure percentage at a specific interval of time.
*/
void TestStressTestClient::logErrorPercentage(
    Uint32 successCount,
    Uint32 totalCount,
    pid_t clientPid,
    String &clientLog,
    char client[])
{
    Uint32 successPercentage, errorPercentage;
    successPercentage = (successCount/totalCount)*100;
    errorPercentage = 100 - successPercentage;

    //
    // loging details here
    //
    ofstream errorLog_File(clientLog.getCString(), ios::app);
    errorLog_File<<client<<" PID#"<<clientPid<<" ran "<<totalCount
                 <<" times with a "<<errorPercentage<<"% failure"<<"\n";
    errorLog_File.close();
}

/** This method is used to log the informations of client logs to the client
    log file.
*/
void TestStressTestClient::errorLog(
    pid_t clientPid,
    String &clientLog,
    String &message)
{
    //
    // loging details here .
    //
    ofstream errorLog_File(clientLog.getCString(), ios::app);
    errorLog_File<<" PID#"<<clientPid<<"::"<<message<<"\n";
    errorLog_File.close();
}

/** This method handles the SSLCertificate verification part. */
static Boolean verifyCertificate(SSLCertificateInfo &certInfo)
{
    //
    // Add code to handle server certificate verification.
    //
    return true;
}

/** This method is used by the clients to connect to the server. If useSSL is
    true then an SSL connection will be atemped with the userName and passWord
    that is passed in. Otherwise connection will be established using
    localhost. All parameters are required.
*/
void TestStressTestClient::connectClient(
    CIMClient *client,
    String host,
    Uint32 portNumber,
    String userName,
    String password,
    Boolean useSSL,
    Uint32 timeout,
    Boolean verboseTest)
{
    if (useSSL)
    {
#ifdef PEGASUS_HAS_SSL
        //
        // Get environment variables.
        //
        const char* pegasusHome = getenv("PEGASUS_HOME");

        String trustpath = FileSystem::getAbsolutePath(
            pegasusHome, PEGASUS_SSLCLIENT_CERTIFICATEFILE);

        String randFile = String::EMPTY;

#ifdef PEGASUS_SSL_RANDOMFILE
        randFile = FileSystem::getAbsolutePath(
                       pegasusHome,
                       PEGASUS_SSLCLIENT_RANDOMFILE);
#endif

        SSLContext sslContext(
            trustpath, verifyCertificate, randFile);
        if (verboseTest)
        {
            cout << "connecting to " << host << ":"
                 << portNumber << " using SSL"
                 << endl;
        }
        client->connect (host, portNumber, sslContext, userName, password);
#else
        PEGASUS_TEST_ASSERT(false);
#endif
    } /* useSSL. */
    else
    {
        if (verboseTest)
        {
            cout << "Connecting to " << host << ":" << portNumber
                 << endl;
        }
        client->connect (host, portNumber, userName, password);
    }
    if (verboseTest)
    {
        cout << "Client Connected" << endl;
    }
}
