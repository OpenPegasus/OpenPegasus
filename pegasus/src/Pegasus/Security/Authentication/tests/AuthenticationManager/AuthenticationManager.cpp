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
#include <Pegasus/Common/FileSystem.h>
#include <Pegasus/Common/AuthenticationInfo.h>
#include <Pegasus/Common/Base64.h>
#include <Pegasus/Config/ConfigManager.h>
#include <Pegasus/Security/UserManager/UserManager.h>
#include <Pegasus/Security/Authentication/AuthenticationManager.h>

//
// Enable debug messages
//
//#define DEBUG 1


PEGASUS_USING_STD;

PEGASUS_USING_PEGASUS;

static Boolean verbose;

String testUser = System::getEffectiveUserName();

String guestUser = "guest";

String guestPassword = "guest";

String localHeader = "Local ";

String basicHeader = "Basic ";



String encodeUserPass(const String& userPass)
{
    //
    // copy userPass string content to Uint8 array for encoding
    //
    Buffer userPassArray;

    Uint32 userPassLength = userPass.size();

    userPassArray.reserveCapacity( userPassLength );
    userPassArray.clear();

    for( Uint32 i = 0; i < userPassLength; i++ )
    {
        userPassArray.append( (Uint8)userPass[i] );
    }

    //
    // base64 encode the user name and password
    //
    Buffer encodedArray;

    encodedArray = Base64::encode( userPassArray );

    String encodedStr =
        String( encodedArray.getData(), encodedArray.size() );

    if (verbose) cout << "userPass: " << userPass << endl;
    if (verbose) cout << "Encoded userPass: " << encodedStr << endl;

    return (encodedStr);
}

//
// Test HTTP Auth header creation
//
void testHttpAuthHeader()
{
    AuthenticationManager  authManager;

    String respHeader = authManager.getHttpAuthResponseHeader();

    if (verbose) cout << "Challenge Header = " << respHeader << endl;

    PEGASUS_TEST_ASSERT(respHeader.size() != 0);
}

//
// Test Local Auth header creation
//
void testLocalAuthHeader_1()
{
    String authHeader;

    AuthenticationManager  authManager;

    AuthenticationInfo* authInfo = new AuthenticationInfo(true);

    // Test invalid header
    String respHeader =
        authManager.getPegasusAuthResponseHeader(testUser, authInfo);

    if (verbose) cout << "RespHeader: " << respHeader << endl;

    delete authInfo;

    Uint32 startQuote = respHeader.find(0, '"');
    PEGASUS_TEST_ASSERT(startQuote == PEG_NOT_FOUND);
}

//
// Test Local Auth header creation
//
void testLocalAuthHeader_2()
{
    String authHeader;

    AuthenticationManager  authManager;

    AuthenticationInfo* authInfo = new AuthenticationInfo(true);

    // Test invalid header
    authHeader.append(localHeader);

    String respHeader =
        authManager.getPegasusAuthResponseHeader(authHeader, authInfo);

    if (verbose) cout << "RespHeader: " << respHeader << endl;

    delete authInfo;

    Uint32 startQuote = respHeader.find(0, '"');
    PEGASUS_TEST_ASSERT(startQuote == PEG_NOT_FOUND);
}

//
// Test Local Auth header creation
//
void testLocalAuthHeader_3()
{
    String authHeader;

    AuthenticationManager  authManager;

    AuthenticationInfo* authInfo = new AuthenticationInfo(true);

    // Test invalid header
    authHeader.append(localHeader);
    authHeader.append("\"\"");

    String respHeader =
        authManager.getPegasusAuthResponseHeader(authHeader, authInfo);

    if (verbose) cout << "RespHeader: " << respHeader << endl;

    delete authInfo;

    Uint32 startQuote = respHeader.find(0, '"');
    PEGASUS_TEST_ASSERT(startQuote == PEG_NOT_FOUND);
}

//
// Test local authentication
//
void testLocalAuthSuccess()
{
    String authHeader;

    AuthenticationManager  authManager;

    AuthenticationInfo* authInfo = new AuthenticationInfo(true);

    // Test valid header
    authHeader.append(localHeader);
    authHeader.append("\"");
    authHeader.append(testUser);
    authHeader.append("\"");

    String respHeader =
        authManager.getPegasusAuthResponseHeader(authHeader, authInfo);

    if (verbose) cout << "RespHeader: " << respHeader << endl;

    Uint32 startQuote = respHeader.find(0, '"');
    PEGASUS_TEST_ASSERT(startQuote != PEG_NOT_FOUND);

    Uint32 endQuote = respHeader.find(startQuote + 1, '"');
    PEGASUS_TEST_ASSERT(endQuote != PEG_NOT_FOUND);

    String filePath = respHeader.subString(
        startQuote + 1, (endQuote - startQuote - 1));
    PEGASUS_TEST_ASSERT(filePath.size() != 0);

    authHeader.clear();
    authHeader.append(localHeader);
    authHeader.append("\"");
    authHeader.append(testUser);
    authHeader.append(":");
    authHeader.append(filePath);
    authHeader.append(":");
    authHeader.append(authInfo->getLocalAuthSecret());
    authHeader.append("\"");

    if (verbose) cout << "Local Resp AuthHeader: " << authHeader << endl;

    Boolean authenticated;
    // test case looks for success, initialize with failure
    AuthenticationStatus authStatus(AUTHSC_UNAUTHORIZED);


    authStatus = authManager.performPegasusAuthentication(authHeader, authInfo);
    authenticated = authStatus.isSuccess();

    //
    // remove the auth file created for this user request
    //
    if (FileSystem::exists(filePath))
    {
        FileSystem::removeFile(filePath);
    }
    if (verbose)
    {
        cout << "Local Authentication of User " + testUser + " returned with: ";
        cout << authenticated << endl;
    }

    delete authInfo;

    PEGASUS_TEST_ASSERT(authenticated);
}


//
// Test HTTP Basic with valid user name and password
//
void testBasicAuthSuccess()
{
    String authHeader;

    AuthenticationManager  authManager;

    AuthenticationInfo* authInfo = new AuthenticationInfo(true);

    String userPass = guestUser;
    userPass.append(":");
    userPass.append(guestPassword);

    authHeader.append(basicHeader);
    authHeader.append(encodeUserPass(userPass));

    Boolean authenticated;
    // test case looks for success, initialize with failure
    AuthenticationStatus authStatus(AUTHSC_UNAUTHORIZED);

    authStatus = authManager.performHttpAuthentication(authHeader, authInfo);
    authenticated = authStatus.isSuccess();

    if (verbose)
    {
        cout << "Authentication of user " + guestUser + " returned with: ";
        cout << authenticated << endl;
    }

    delete authInfo;

    //PEGASUS_TEST_ASSERT(authenticated);
}

////////////////////////////////////////////////////////////////////

int main(int, char** argv)
{
    verbose = (getenv ("PEGASUS_TEST_VERBOSE")) ? true : false;
    if (verbose)
        cout << argv[0] << ": started" << endl;

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

        if(pegHome.size())
            ConfigManager::setPegasusHome(pegHome);

        if (verbose)
            cout << "Peg Home : " << ConfigManager::getPegasusHome() << endl;

        if (verbose) cout << "Doing testHttpAuthHeader()...." << endl;

        // -- Create a repository:
        const char* tmpDir = getenv ("PEGASUS_TMP");
        String repositoryPath;
        if (tmpDir == NULL)
        {
            repositoryPath = ".";
        }
        else
        {
            repositoryPath = tmpDir;
        }
        repositoryPath.append("/repository");

        FileSystem::removeDirectoryHier(repositoryPath);

        CIMRepository* repository = new CIMRepository(repositoryPath);

        // -- Create a UserManager object:

#ifndef PEGASUS_PAM_AUTHENTICATION
        UserManager* userManager = UserManager::getInstance(repository);
        PEGASUS_TEST_ASSERT(0 != userManager);
#endif

        testHttpAuthHeader();

        if (verbose) cout << "Doing testLocalAuthHeader_1()...." << endl;
        testLocalAuthHeader_1();

        if (verbose) cout << "Doing testLocalAuthHeader_2()...." << endl;
        testLocalAuthHeader_2();

        if (verbose) cout << "Doing testLocalAuthHeader_3()...." << endl;
        testLocalAuthHeader_3();

        if (verbose) cout << "Doing testLocalAuthSuccess()...." << endl;
        testLocalAuthSuccess();

        if (verbose) cout << "Doing testBasicAuthSuccess()...." << endl;
        testBasicAuthSuccess();

#ifndef PEGASUS_PAM_AUTHENTICATION
        UserManager::destroy();
#endif
        delete repository;
        FileSystem::removeDirectoryHier(repositoryPath);
    }
    catch(Exception& e)
    {
        cout << argv[0] << " Exception: " << e.getMessage() << endl;
        PEGASUS_TEST_ASSERT(0);
    }

#endif

    cout << argv[0] << " +++++ passed all tests" << endl;

    return 0;
}
