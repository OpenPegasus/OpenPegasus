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
#include <Pegasus/Common/PegasusAssert.h>
#include <iostream>
#include <Pegasus/Config/ConfigManager.h>
#include <Pegasus/Security/Authentication/Cookies.h>

//
// Enable debug messages
//
//#define DEBUG 1


PEGASUS_USING_PEGASUS;

PEGASUS_USING_STD;

static Boolean verbose;

static String userName = "tester";
static String validIP = "192.168.0.1";
static String invalidIP = "192.168.0.2";

static int timeout = 2;
static String strTimeout = "2";

//
// Test with valid sessionID
//
void testAuthenticationSuccess()
{
    String sessionID;
    String outUserName;

    HTTPSessionList sessions;

    sessionID = sessions.addNewSession(userName, validIP);

    // try to authenticate with the same sessionID
    bool authenticated = sessions.isAuthenticated(sessionID, validIP,
            outUserName);
    PEGASUS_TEST_ASSERT(authenticated);
    PEGASUS_TEST_ASSERT(outUserName == userName);
}

//
// Test with invalid sessionID
//
void testAuthenticationFailure_1()
{
    String sessionID;
    String outUserName;

    HTTPSessionList sessions;


    sessionID = sessions.addNewSession(userName, validIP);

    // try to authenticate with different sessionID
    sessionID.append("xxx");
    bool authenticated = sessions.isAuthenticated(sessionID, validIP,
            outUserName);
    PEGASUS_TEST_ASSERT(!authenticated);
}

//
// Test with invalid IP
//
void testAuthenticationFailure_2()
{
    String sessionID;
    String outUserName;

    HTTPSessionList sessions;

    sessionID = sessions.addNewSession(userName, validIP);

    bool authenticated = sessions.isAuthenticated(sessionID, invalidIP,
            outUserName);
    PEGASUS_TEST_ASSERT(!authenticated);
}

//
// Test with expired session
//
void testAuthenticationFailure_3()
{
    String sessionID;
    String outUserName;

    HTTPSessionList sessions;

    sessionID = sessions.addNewSession(userName, validIP);

    sleep(timeout);
    bool authenticated = sessions.isAuthenticated(sessionID, validIP,
            outUserName);
    PEGASUS_TEST_ASSERT(!authenticated);
}

//
// Test with valid sessionID, but with disabled cookies
//
void testAuthenticationFailure_4()
{
    String sessionID;
    String outUserName;

    HTTPSessionList sessions;

    sessionID = sessions.addNewSession(userName, validIP);

    // try to authenticate with the same sessionID
    bool authenticated = sessions.isAuthenticated(sessionID, validIP,
            outUserName);
    PEGASUS_TEST_ASSERT(!authenticated);
}

////////////////////////////////////////////////////////////////////

int main(int, char** argv)
{
    verbose = (getenv ("PEGASUS_TEST_VERBOSE")) ? true : false;
    if (verbose) cout << argv[0] << ": started" << endl;

    try
    {
#ifdef DEBUG
        Tracer::setTraceFile("/tmp/trace");
        Tracer::setTraceComponents("all");
        verbose = true;
#endif

        ConfigManager* configManager = ConfigManager::getInstance();
        PEGASUS_TEST_ASSERT(0 != configManager);
        configManager->useConfigFiles = false;
        configManager->initCurrentValue("httpSessionTimeout", strTimeout);

        const char* path = getenv("PEGASUS_HOME");
        String pegHome = path;

        if (pegHome.size())
            ConfigManager::setPegasusHome(pegHome);

        if (verbose)
            cout << "Peg Home : " << ConfigManager::getPegasusHome() << endl;

        if (verbose) cout << "Doing testAuthenticationFailure_1()...." << endl;
        testAuthenticationFailure_1();

        if (verbose) cout << "Doing testAuthenticationFailure_2()...." << endl;
        testAuthenticationFailure_2();

        if (verbose) cout << "Doing testAuthenticationFailure_3()...." << endl;
        testAuthenticationFailure_3();

        if (verbose) cout << "Doing testAuthenticationSuccess()...." << endl;
        testAuthenticationSuccess();

        // disable cookies
        configManager->initCurrentValue("httpSessionTimeout", "0");
        if (verbose) cout << "Doing testAuthenticationFailure_4()...." << endl;
        testAuthenticationFailure_4();
    }
    catch(Exception& e)
    {
        cout << argv[0] << "Exception: " << e.getMessage() << endl;
        PEGASUS_TEST_ASSERT(0);
    }

    cout << argv[0] << " +++++ passed all tests" << endl;

    return 0;
}
