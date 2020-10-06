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

#ifndef Pegasus_LocalAuthenticationHandler_h
#define Pegasus_LocalAuthenticationHandler_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/String.h>
#include <Pegasus/Common/AutoPtr.h>

#include "LocalAuthenticator.h"
#include "Authenticator.h"

#include <Pegasus/Security/Authentication/Linkage.h>

PEGASUS_NAMESPACE_BEGIN

/**
    This class implements the AuthenticationHandler for Pegasus Local
    authentication.  It extends the Authenticator and provides the
    implementation.
*/

class PEGASUS_SECURITY_LINKAGE LocalAuthenticationHandler
    : public Authenticator
{
public:

    /** Constructors  */
    LocalAuthenticationHandler();

    /** Destructor  */
    ~LocalAuthenticationHandler();

    /** Verify the authentication of the user passed in the authorization
        header.
        @param authHeader String containing the Authorization header
        @param authInfo Reference to AuthenticationInfo object
        @return AuthenticationStatus holding http status code and error detail
    */
    AuthenticationStatus authenticate(
        const String& authHeader,
        AuthenticationInfo* authInfo);

    /** Construct and return the Pegasus Local authentication challenge header
        @param authType String containing the HTTP authentication type
        @param userName String containing the user name
        @param authInfo Reference to AuthenticationInfo object
        @return A string containing the authentication challenge header.
    */
    String getAuthResponseHeader(
        const String& authType = String::EMPTY,
        const String& userName = String::EMPTY,
        AuthenticationInfo* authInfo = 0);

    /**
        Verify whether the user is valid.
        @param userName String containing the user name
        @param authInfo reference to AuthenticationInfo object that holds the
        authentication information for the given connection.
        @return AuthenticationStatus holding http status code and error detail
    */
    AuthenticationStatus validateUser(
        const String& userName,
        AuthenticationInfo* authInfo = 0);

private:

    AutoPtr<LocalAuthenticator> _localAuthenticator;
};

PEGASUS_NAMESPACE_END

#endif /* Pegasus_LocalAuthenticationHandler_h*/
