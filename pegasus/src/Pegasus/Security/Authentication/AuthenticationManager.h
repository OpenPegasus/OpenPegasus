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

#ifndef Pegasus_AuthenticationManager_h
#define Pegasus_AuthenticationManager_h

#include <Pegasus/Common/Config.h>

#include "Authenticator.h"

#include <Pegasus/Security/Authentication/Linkage.h>

#ifdef PEGASUS_NEGOTIATE_AUTHENTICATION
#include <Pegasus/Common/AuthenticationInfo.h>
#endif

PEGASUS_NAMESPACE_BEGIN

/** This class implements the HTTP authentication and Pegasus Local
    authentication mecahnism. It provides methods to perform authentication
    and to generate authentication challenge headers.
*/

class PEGASUS_SECURITY_LINKAGE AuthenticationManager
{
public:

    /**Constructor */
    AuthenticationManager();

    /**Destructor */
    ~AuthenticationManager();

    /** Authenticates the requests from HTTP connections.
        @param authHeader String containing the Authorization header
        @param authInfo Reference to AuthenticationInfo object that holds the
        authentication information for the given connection.
        @return AuthenticationStatus holding http status code and error detail
    */
    AuthenticationStatus performHttpAuthentication(
        const String& authHeader,
        AuthenticationInfo* authInfo);

    /** Authenticates the requests from Local connections.
        @param authHeader String containing the Authorization header
        @param authInfo Reference to AuthenticationInfo object that holds the
        authentication information for the given connection.
        @return AuthenticationStatus holding http status code and error detail
    */
    AuthenticationStatus performPegasusAuthentication(
        const String& authHeader,
        AuthenticationInfo* authInfo);

    /** Validates whether the user is a valid user for requests
        from HTTP connections.
        @param  userName  name of the user
        @param authInfo Reference to AuthenticationInfo object that holds the
        authentication information for the given connection.
        @return AuthenticationStatus holding http status code and error detail
    */
    AuthenticationStatus validateUserForHttpAuth(
        const String& userName,
        AuthenticationInfo* authInfo);

    /** Constructs the Pegasus Local authentication challenge header.
        @param authHeader String containing the Authorization header
        @param authInfo reference to AuthenticationInfo object that holds the
        authentication information for the given connection.
        @return String containing the authentication challenge
    */
    String getPegasusAuthResponseHeader(
        const String& authHeader,
        AuthenticationInfo* authInfo);

    /** Constructs the HTTP authentication challenge header.
        @return String containing the authentication challenge
    */
#ifdef PEGASUS_NEGOTIATE_AUTHENTICATION
    String getHttpAuthResponseHeader(
        AuthenticationInfo* authInfo = 0);
#else
    String getHttpAuthResponseHeader();
#endif

    static Boolean isRemotePrivilegedUserAccessAllowed( String & userName);

private:

    Authenticator* _getLocalAuthHandler();

    Authenticator* _getHttpAuthHandler();

    Authenticator* _localAuthHandler;
    Authenticator* _httpAuthHandler;
    String         _httpAuthType;

};

PEGASUS_NAMESPACE_END

#endif /* Pegasus_AuthenticationManager_h */

