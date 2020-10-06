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
#include <Pegasus/Common/Base64.h>
#include <Pegasus/Common/AuthenticationInfo.h>
#include <Pegasus/Config/ConfigManager.h>
#include <Pegasus/Repository/CIMRepository.h>
#include <Pegasus/Security/UserManager/UserManager.h>
#include <Pegasus/Security/Authentication/BasicAuthenticationHandler.h>

//
// Enable debug messages
//
//#define DEBUG 1


PEGASUS_USING_PEGASUS;

PEGASUS_USING_STD;

static Boolean verbose;

String authType = "Basic";

String testUser = System::getEffectiveUserName();

String guestUser = "guest";

String guestPassword = "guest";

String invalidUser = "xyz123ww";

String invalidPassword = "xyz123ww";

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

void testAuthHeader()
{
    BasicAuthenticationHandler  basicAuthHandler;

    String respHeader = basicAuthHandler.getAuthResponseHeader();

    if (verbose) cout << "Challenge Header = " << respHeader << endl;

    PEGASUS_TEST_ASSERT(respHeader.size() != 0);
}

//
// Test with invalid userPass (with no ':' separator)
//
void testAuthenticationFailure_1()
{
    String authHeader;
    Boolean authenticated;
    // initialize with success to ensure failure is detected
    AuthenticationStatus authStatus(AUTHSC_SUCCESS);

    BasicAuthenticationHandler  basicAuthHandler;

    AuthenticationInfo* authInfo = new AuthenticationInfo(true);

    //
    // Test with invalid user password string
    //
    String userPass = testUser;
    userPass.append(guestPassword);

    authHeader.append(encodeUserPass(userPass));

    authStatus = basicAuthHandler.authenticate(authHeader, authInfo);
    authenticated = authStatus.isSuccess();

    if (verbose)
    {
        cout << "Authentication of user " + testUser + " returned with: ";
        cout << authenticated << endl;
    }

    delete authInfo;

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

    BasicAuthenticationHandler  basicAuthHandler;

    AuthenticationInfo* authInfo = new AuthenticationInfo(true);

    //
    // Test with invalid password
    //
    String userPass = invalidUser;
    userPass.append(":");
    userPass.append(guestPassword);

    authHeader.append(encodeUserPass(userPass));

    authStatus = basicAuthHandler.authenticate(authHeader, authInfo);
    authenticated = authStatus.isSuccess();

    if (verbose)
    {
        cout << "Authentication of invalidUser "+invalidUser+" returned with: ";
        cout << authenticated << endl;
    }

    delete authInfo;

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

    BasicAuthenticationHandler  basicAuthHandler;

    AuthenticationInfo* authInfo = new AuthenticationInfo(true);

    String userPass = guestUser;
    userPass.append(":");
    userPass.append(invalidPassword);

    authHeader.append(encodeUserPass(userPass));

    authStatus = basicAuthHandler.authenticate(authHeader, authInfo);
    authenticated = authStatus.isSuccess();

    if (verbose)
    {
        cout << "Authentication of user " + testUser + " returned with: ";
        cout << authenticated << endl;
    }

    delete authInfo;

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

    BasicAuthenticationHandler  basicAuthHandler;

    AuthenticationInfo* authInfo = new AuthenticationInfo(true);

    String userPass = invalidUser;
    userPass.append(":");
    userPass.append(invalidPassword);

    authHeader.append(encodeUserPass(userPass));

    authStatus = basicAuthHandler.authenticate(authHeader, authInfo);
    authenticated = authStatus.isSuccess();

    if (verbose)
    {
        cout << "Authentication of user " + testUser + " returned with: ";
        cout << authenticated << endl;
    }

    delete authInfo;

    PEGASUS_TEST_ASSERT(!authenticated);
}

//
// Test with valid user name and password
// (Assuming there is a valid CIM user 'guest' with password 'guest')
//
void testAuthenticationSuccess()
{
    String authHeader;
    // initialize with success to ensure failure is detected
    AuthenticationStatus authStatus(AUTHSC_SUCCESS);

    BasicAuthenticationHandler  basicAuthHandler;

    AuthenticationInfo* authInfo = new AuthenticationInfo(true);

    String userPass = guestUser;
    userPass.append(":");
    userPass.append(guestPassword);

    authHeader.append(encodeUserPass(userPass));

    Boolean authenticated;

    authStatus = basicAuthHandler.authenticate(authHeader, authInfo);
    authenticated = authStatus.isSuccess();

    if (verbose)
    {
        cout << "Authentication of guestUser " + guestUser + " returned with: ";
        cout << authenticated << endl;
    }

    delete authInfo;

    //PEGASUS_TEST_ASSERT(authenticated);
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
        Tracer::setTraceFile("./Authentication.trc");
        Tracer::setTraceComponents("all");
        Tracer::setTraceLevel(Tracer::LEVEL4);
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

        if (verbose)
            cout << "Doing testAuthHeader()...." << endl;

        // -- Create a test repository:

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
