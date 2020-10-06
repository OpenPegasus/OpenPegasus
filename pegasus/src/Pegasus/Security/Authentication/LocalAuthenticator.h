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

#ifndef Pegasus_LocalAuthenticator_h
#define Pegasus_LocalAuthenticator_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/String.h>
#include <Pegasus/Common/AuthenticationInfo.h>
#include <Pegasus/Security/Authentication/Linkage.h>
#include <Pegasus/Security/Authentication/AuthenticationStatus.h>

PEGASUS_NAMESPACE_BEGIN


/**
    This is an abstract class that the local authenticators will extend and
    provide the implementation. Sample implementation can be seen in
    SecureLocalAuthenticator.cpp
*/
class PEGASUS_SECURITY_LINKAGE LocalAuthenticator
{
public:

    /** constructor. */
    LocalAuthenticator() { };

    /** destructor. */
    virtual ~LocalAuthenticator() { };

    /** Verify the authentication of the requesting user.
        @param userName String containing the user name
        @param secretReceived String containing the authentication secret
        sent by the client.
        @param secretKept String containing the authentication secret that
        was sent to client as part of the challenge.
        @return AuthenticationStatus holding http status code and error detail
    */
    virtual AuthenticationStatus authenticate(
        const String& userName,
        const String& secretReceived,
        const String& secretKept) = 0;

    /** Construct and return the Peagaus Local authentication challenge header
        @param authType String containing the authentication type
        @param userName String containing the user name
        @param filePath String to store the authentication file path
        @param secret String to store the authentication secret
        @return A string containing the authentication challenge header.
    */
    virtual String getAuthResponseHeader(
        const String& authType,
        const String& userName,
        String& filePath,
        String& secret) = 0;

    /**
        Verify whether the user is valid.
        @param userName String containing the user name
        @param authInfo reference to AuthenticationInfo object that holds the
        authentication information for the given connection.
        @return AuthenticationStatus holding http status code and error detail
    */
    virtual AuthenticationStatus validateUser(
        const String& userName,
        AuthenticationInfo* authInfo) = 0;
};


PEGASUS_NAMESPACE_END

#endif /* Pegasus_LocalAuthenticator_h */
