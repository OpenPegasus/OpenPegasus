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
//%////////////////////////////////////////////////////////////////////////////

/**
    This file has testcases that are added to cover the functions defined in
    AuthenticationInfo class.
*/

#include <cstdlib>
#include <iostream>
#include <Pegasus/Common/PegasusAssert.h>
#include <Pegasus/Common/AuthenticationInfo.h>
#include <Pegasus/Common/String.h>

PEGASUS_USING_PEGASUS;
PEGASUS_USING_STD;

void test01()
{
    AuthenticationInfo authInfo(true) ;

    /**
        Added to cover the Function
        void AuthenticationInfo::setAuthenticatedUser(const String& userName)
    */
#define TEST_USER "My Test User"
    PEGASUS_TEST_ASSERT(authInfo.getAuthenticatedUser() == String::EMPTY);
    authInfo.setAuthenticatedUser(TEST_USER);
    String ret_setAuthenticatedUser = authInfo.getAuthenticatedUser();
    PEGASUS_TEST_ASSERT(ret_setAuthenticatedUser == TEST_USER);

    /**
        Added to cover the Function
        void AuthenticationInfo::setAuthenticatedPassword(
         const String& password)
    */
#define TEST_PASSWORD "My Test Password"
    PEGASUS_TEST_ASSERT(authInfo.getAuthenticatedPassword() == String::EMPTY);
    authInfo.setAuthenticatedPassword(TEST_PASSWORD);
    String ret_setAuthenticatedPassword = authInfo.getAuthenticatedPassword();
    PEGASUS_TEST_ASSERT(ret_setAuthenticatedPassword == TEST_PASSWORD);

    /**
        Added to cover the Function
        void AuthenticationInfo::setLocalAuthFilePath(const String& secret)
    */
#define TEST_FILEPATH "/tmp/localauthfilename.txt"
    PEGASUS_TEST_ASSERT(authInfo.getLocalAuthFilePath() == String::EMPTY);
    authInfo.setLocalAuthFilePath(TEST_FILEPATH);
    String ret_setAuthFilePath = authInfo.getLocalAuthFilePath();
    PEGASUS_TEST_ASSERT(ret_setAuthFilePath == TEST_FILEPATH);

    /**
        Added to cover the Function
        void AuthenticationInfo::setLocalAuthSecret(const String& secret)
    */
#define TEST_SECRET "My Test Secret"
    PEGASUS_TEST_ASSERT(authInfo.getLocalAuthSecret() == String::EMPTY);
    authInfo.setLocalAuthSecret(TEST_SECRET);
    String ret_setAuthSecret = authInfo.getLocalAuthSecret();
    PEGASUS_TEST_ASSERT(ret_setAuthSecret == TEST_SECRET);

    /**
        Added to cover the Function
        void AuthenticationInfo::setAuthType(const String& authType)
    */
    PEGASUS_TEST_ASSERT(authInfo.getAuthType() == String::EMPTY);
    authInfo.setAuthType(AuthenticationInfoRep::AUTH_TYPE_SSL);
    String ret_setauthtype = authInfo.getAuthType();
    PEGASUS_TEST_ASSERT(
        ret_setauthtype == AuthenticationInfoRep::AUTH_TYPE_SSL);

    /**
        Added to cover the Function
        void AuthenticationInfo::setConnectionAuthenticated(
            Boolean connectionAuthenticated)
    */
    PEGASUS_TEST_ASSERT(authInfo.isConnectionAuthenticated() == false);
    authInfo.setConnectionAuthenticated(true);
    PEGASUS_TEST_ASSERT(authInfo.isConnectionAuthenticated() == true);

    /**
        Added to cover the Function
        void AuthenticationInfo::setIpAddress(String ipAddress)
    */
#define TEST_IP "127.0.0.1"
    PEGASUS_TEST_ASSERT(authInfo.getIpAddress() == String::EMPTY);
    authInfo.setIpAddress(TEST_IP);
    PEGASUS_TEST_ASSERT(authInfo.getIpAddress() == TEST_IP);

    /**
        Added to cover the Function
        void AuthenticationInfo::setClientCertificateChain(
        Array<SSLCertificateInfo*> clientCertificate)
    */
    Array<SSLCertificateInfo*> clientCertificate;
    authInfo.setClientCertificateChain(clientCertificate);
    Array<SSLCertificateInfo*> ret_clientCertificate =
        authInfo.getClientCertificateChain();
    PEGASUS_TEST_ASSERT(clientCertificate == ret_clientCertificate);

    AuthenticationInfo authInfo1;
    AuthenticationInfo authInfo2 (authInfo);

    PEGASUS_TEST_ASSERT(authInfo2.isConnectionAuthenticated() == true);

    authInfo1 = authInfo;

    PEGASUS_TEST_ASSERT(authInfo1.isConnectionAuthenticated() == true);
}

int main(int, char *argv[] )
{
    try
    {
        test01();
    }
    catch (Exception& e)
    {
        PEGASUS_STD (cout) << "Exception: " << e.getMessage()
            << PEGASUS_STD(endl);
        exit(1);
    }

    PEGASUS_STD (cout) << argv[0] << " +++++ passed all tests"
        << PEGASUS_STD (endl);
    return 0;
}

