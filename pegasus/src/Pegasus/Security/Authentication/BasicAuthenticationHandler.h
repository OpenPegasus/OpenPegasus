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

#ifndef Pegasus_BasicAuthenticationHandler_h
#define Pegasus_BasicAuthenticationHandler_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/String.h>

#include "Authenticator.h"
#include "BasicAuthenticator.h"

#include <Pegasus/Security/Authentication/Linkage.h>


PEGASUS_NAMESPACE_BEGIN

/**
    Implements the AuthenticationHandler for Basic authentication
*/

static const char BASIC_AUTHENTICATION_FAILED_KEY [] =
    "Security.Authentication.BasicAuthenticationHandler."
        "BASIC_AUTHENTICATION_FAILURE";

static const char BASIC_AUTHENTICATION_FAILED [] =
    "Authentication failed for user $0 from client IP address $1.";

class PEGASUS_SECURITY_LINKAGE BasicAuthenticationHandler
    : public Authenticator
{
public:

    /** Constructors  */
    BasicAuthenticationHandler();

    /** Destructor  */
    ~BasicAuthenticationHandler();

    /** Verify the authentication of the user passed in the authorization
        header.
        @param authHeader String containing the Authorization header
        @param authInfo Reference to AuthenticationInfo object
        @return AuthenticationStatus holding http status code and error detail
    */
    AuthenticationStatus authenticate(
        const String& authHeader,
        AuthenticationInfo* authInfo);

    /** Construct and return the Basic authentication challenge header
        @param authType An optional string containing the HTTP authentication
        type
        @param userName An optional string containing the user name
        (Parameters are not used to generate the Basic authentication challenge
         header, hence they are optional)
        @return A string containing the authentication challenge header.
    */
    String getAuthResponseHeader(
        const String& authType = String::EMPTY,
        const String& userName = String::EMPTY,
        AuthenticationInfo* authInfo = 0);

    AuthenticationStatus validateUser(
        const String& userName,
        AuthenticationInfo* authInfo);


private:

    BasicAuthenticator* _basicAuthenticator;
};

PEGASUS_NAMESPACE_END

#endif /* Pegasus_BasicAuthenticationHandler_h*/
