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
#include <Pegasus/Common/System.h>
#include <Pegasus/Common/Base64.h>
#include <Pegasus/Common/AuthenticationInfo.h>
#include <Pegasus/Config/ConfigManager.h>
#include <Pegasus/Security/Authentication/LocalAuthenticationHandler.h>

//
// Enable debug messages
//
//#define DEBUG 1


PEGASUS_USING_PEGASUS;

PEGASUS_USING_STD;

static Boolean verbose;

String authType = "Local";

String testUser = System::getEffectiveUserName();

String secret;

String filePath;

AuthenticationInfo* authInfo = 0;

void testAuthHeader()
{
    LocalAuthenticationHandler  localAuthHandler;

    String respHeader =
        localAuthHandler.getAuthResponseHeader(authType, testUser, authInfo);

#ifdef DEBUG
    if (verbose)
        cout << "respHeader= " << respHeader << endl;
#endif

    secret = authInfo->getLocalAuthSecret();

    PEGASUS_TEST_ASSERT(respHeader.size() != 0);

    Uint32 startQuote = respHeader.find(0, '"');
    PEGASUS_TEST_ASSERT(startQuote != PEG_NOT_FOUND);

    Uint32 endQuote = respHeader.find(startQuote + 1, '"');
    PEGASUS_TEST_ASSERT(startQuote != PEG_NOT_FOUND);

    filePath =
        respHeader.subString(startQuote + 1, (endQuote - startQuote - 1));

    PEGASUS_TEST_ASSERT(filePath.size() != 0);
}

//
// Test with invalid userPass
//
void testAuthenticationFailure_1()
{
    String authHeader;
    Boolean authenticated;
    // initialize with success to ensure failure is detected
    AuthenticationStatus authStatus(AUTHSC_SUCCESS);

    LocalAuthenticationHandler  localAuthHandler;

    //
    // Test with invalid auth header
    //
    authHeader = testUser;
    authHeader.append(filePath);
    authHeader.append(secret);

    authStatus = localAuthHandler.authenticate(authHeader, authInfo);
    authenticated = authStatus.isSuccess();

    if (verbose)
    {
        cout << "authHeader: " << authHeader << endl;
        cout << "Authentication of user " + testUser + " returned with: ";
        cout << authenticated << endl;
    }
    PEGASUS_TEST_ASSERT(!authenticated);
}

//
// Test with invalid system user
//
void testAuthenticationFailure_2()
{
    String authHeader;
    Boolean authenticated;
    // initialize with success to ensure failure is detected
    AuthenticationStatus authStatus(AUTHSC_SUCCESS);

    LocalAuthenticationHandler  localAuthHandler;

    //
    // Test with invalid auth header
    //
    authHeader = testUser;
    authHeader.append(filePath);

    authStatus = localAuthHandler.authenticate(authHeader, authInfo);
    authenticated = authStatus.isSuccess();

    if (verbose)
    {
        cout << "authHeader: " << authHeader << endl;
        cout << "Authentication of user " + testUser + " returned with: ";
        cout << authenticated << endl;
    }

    PEGASUS_TEST_ASSERT(!authenticated);
}

//
// Test with invalid password
//
void testAuthenticationFailure_3()
{
    String authHeader;
    Boolean authenticated;
    // initialize with success to ensure failure is detected
    AuthenticationStatus authStatus(AUTHSC_SUCCESS);

    LocalAuthenticationHandler  localAuthHandler;

    authHeader = testUser;
    authHeader.append(":");
    authHeader.append(filePath);
    authHeader.append(":");

    authStatus = localAuthHandler.authenticate(authHeader, authInfo);
    authenticated = authStatus.isSuccess();

    if (verbose)
    {
        cout << "authHeader: " << authHeader << endl;
        cout << "Authentication of user " + testUser + " returned with: ";
        cout << authenticated << endl;
    }

    PEGASUS_TEST_ASSERT(!authenticated);
}

//
// Test with invalid CIM user or invalid password
//
void testAuthenticationFailure_4()
{
    String authHeader;
    Boolean authenticated;
    // initialize with success to ensure failure is detected
    AuthenticationStatus authStatus(AUTHSC_SUCCESS);

    LocalAuthenticationHandler  localAuthHandler;

    authHeader = testUser;
    authHeader.append(":");
    authHeader.append(filePath);
    authHeader.append(":");
    authHeader.append("asd442394asd");

    authStatus = localAuthHandler.authenticate(authHeader, authInfo);
    authenticated = authStatus.isSuccess();

    if (verbose)
    {
        cout << "authHeader: " << authHeader << endl;
        cout << "Authentication of user " + testUser + " returned with: ";
        cout << authenticated << endl;
    }

    PEGASUS_TEST_ASSERT(!authenticated);
}

//
// Test with valid user name and password
//
void testAuthenticationSuccess()
{
    String authHeader;
    // initialize with success to ensure failure is detected
    AuthenticationStatus authStatus(AUTHSC_SUCCESS);

    LocalAuthenticationHandler  localAuthHandler;

    authHeader = testUser;
    authHeader.append(":");
    authHeader.append(filePath);
    authHeader.append(":");
    authHeader.append(secret);

    authInfo->setLocalAuthFilePath(filePath);
    authStatus = localAuthHandler.authenticate(authHeader, authInfo);
    Boolean authenticated = authStatus.isSuccess();
    authInfo->setLocalAuthFilePath(String::EMPTY);

    if (verbose)
    {
        cout << "authHeader: " << authHeader << endl;
        cout << "User " + testUser + " authenticated ";
        cout << ((authenticated) ? "successfully.":"failed..") << endl;
    }

    PEGASUS_TEST_ASSERT(authenticated);
}

////////////////////////////////////////////////////////////////////

int main(int, char** argv)
{
    verbose = (getenv ("PEGASUS_TEST_VERBOSE")) ? true : false;
    if (verbose) cout << argv[0] << ": started" << endl;

#if defined(PEGASUS_OS_TYPE_UNIX)

    try
    {
#ifdef DEBUG
        Tracer::setTraceFile("/tmp/trace");
        Tracer::setTraceComponents("all");
        verbose = true;
#endif

        ConfigManager* configManager = ConfigManager::getInstance();
        PEGASUS_TEST_ASSERT(0 != configManager);

        const char* path = getenv("PEGASUS_HOME");
        String pegHome = path;

        if (pegHome.size())
            ConfigManager::setPegasusHome(pegHome);

        if (verbose)
            cout << "Peg Home : " << ConfigManager::getPegasusHome() << endl;
        authInfo = new AuthenticationInfo(true);

        if (verbose) cout << "Doing testAuthHeader()...." << endl;
        testAuthHeader();

        if (verbose) cout << "Doing testAuthenticationFailure_1()...." << endl;
        testAuthenticationFailure_1();

        if (verbose) cout << "Doing testAuthenticationFailure_2()...." << endl;
        testAuthenticationFailure_2();

        if (verbose) cout << "Doing testAuthenticationFailure_3()...." << endl;
        testAuthenticationFailure_3();

        if (verbose) cout << "Doing testAuthenticationFailure_4()...." << endl;
        testAuthenticationFailure_4();

        if (verbose) cout << "Doing testAuthenticationSuccess()...." << endl;
        testAuthenticationSuccess();

    }
    catch(Exception& e)
    {
        cout << argv[0] << "Exception: " << e.getMessage() << endl;
        PEGASUS_TEST_ASSERT(0);
    }

    delete authInfo;
#endif

    cout << argv[0] << " +++++ passed all tests" << endl;

    return 0;
}
