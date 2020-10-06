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

#ifndef Pegasus_BasicAuthenticator_h
#define Pegasus_BasicAuthenticator_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/String.h>
#include <Pegasus/Common/AuthenticationInfo.h>

#include <Pegasus/Security/Authentication/Linkage.h>
#include <Pegasus/Security/Authentication/AuthenticationStatus.h>

PEGASUS_NAMESPACE_BEGIN

/** This is an abstract class that the Basic authenticators will extend
    and provide the implementation. An implementation can be seen in
    SecureBasicAuthenticator.cpp
*/
class PEGASUS_SECURITY_LINKAGE BasicAuthenticator
{
public:

    /** constructor. */
    BasicAuthenticator() { };

    /** destructor. */
    virtual ~BasicAuthenticator() { };

    /** Verify the authentication of the requesting user.
        @param userName String containing the user name
        @param password String containing the user password
        @param authInfo AuthenticationInfo holding ALL request specific
               authentication information
        @return AuthenticationStatus holding http status code and error detail
    */
    virtual AuthenticationStatus authenticate(
        const String& userName,
        const String& password,
        AuthenticationInfo* authInfo) = 0;

    /** Construct and return the HTTP Basic authentication challenge header
        @return A string containing the authentication challenge header.
    */
    virtual String getAuthResponseHeader() = 0;

    /**
        Verify whether the user is valid.
        @param userName String containing the user name
        @param authInfo AuthenticationInfo holding ALL request specific
               authentication information
        @return AuthenticationStatus holding http status code and error detail
    */
    virtual AuthenticationStatus validateUser(
        const String& userName,
        AuthenticationInfo* authInfo) = 0;
};


PEGASUS_NAMESPACE_END

#endif /* Pegasus_BasicAuthenticator_h */
