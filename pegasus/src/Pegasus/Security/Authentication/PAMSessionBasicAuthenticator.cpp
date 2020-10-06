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

#include <Pegasus/Common/Tracer.h>
#include "PAMSessionBasicAuthenticator.h"
#include "PAMSession.h"
#include "pam_rcToAuthStatus.h"

PEGASUS_USING_STD;

PEGASUS_NAMESPACE_BEGIN

PAMSessionBasicAuthenticator::PAMSessionBasicAuthenticator()
{
    PEG_METHOD_ENTER(TRC_AUTHENTICATION,
        "PAMSessionBasicAuthenticator::PAMSessionBasicAuthenticator()");

    // Build Authentication parameter realm required for Basic Challenge
    // e.g. realm="HostName"

    _realm.assign("realm=");
    _realm.append(Char16('"'));
    _realm.append(System::getHostName());
    _realm.append(Char16('"'));

    PEG_METHOD_EXIT();
}

PAMSessionBasicAuthenticator::~PAMSessionBasicAuthenticator()
{
    PEG_METHOD_ENTER(TRC_AUTHENTICATION,
        "PAMSessionBasicAuthenticator::~PAMSessionBasicAuthenticator()");

    PEG_METHOD_EXIT();
}

AuthenticationStatus PAMSessionBasicAuthenticator::authenticate(
    const String& userName,
    const String& password,
    AuthenticationInfo* authInfo)
{

    PEG_METHOD_ENTER(TRC_AUTHENTICATION,
        "PAMSessionBasicAuthenticator::authenticate()");

    int pamRC = _PAMAuthenticate(
        userName.getCString(),
        password.getCString(),
        authInfo);
    
    AuthenticationStatus authStatus = _getAuthStatusFromPAM_RC(pamRC);

    // in case of an expired password, store user authenticated password
    if (authStatus.isPasswordExpired())
    {
        authInfo->setAuthenticatedPassword(password);
        authInfo->setAuthenticatedUser(userName);
        authInfo->setExpiredPassword(true);
    }
    
    PEG_METHOD_EXIT();
    return authStatus;
}

AuthenticationStatus PAMSessionBasicAuthenticator::validateUser(
    const String& userName,
    AuthenticationInfo* authInfo)
{
    PEG_METHOD_ENTER(TRC_AUTHENTICATION,
        "PAMSessionBasicAuthenticator::validateUser()");

    int pamRC = _PAMValidateUser(userName.getCString(), authInfo);
    AuthenticationStatus authStatus = _getAuthStatusFromPAM_RC(pamRC);
    
    PEG_METHOD_EXIT();
    return authStatus;
}


String PAMSessionBasicAuthenticator::getAuthResponseHeader()
{
    PEG_METHOD_ENTER(TRC_AUTHENTICATION,
        "PAMSessionBasicAuthenticator::getAuthResponseHeader()");

    // Build response header: WWW-Authenticate: Basic realm="HostName"

    String responseHeader = "WWW-Authenticate: Basic ";
    responseHeader.append(_realm);

    PEG_METHOD_EXIT();
    return responseHeader;
}

AuthenticationStatus PAMSessionBasicAuthenticator::updateExpiredPassword(
        const String& userName,
        const String& oldPass,
        const String& newPass,
        const String& ipAddress)
{
    PEG_METHOD_ENTER(TRC_AUTHENTICATION,
        "PAMSessionBasicAuthenticator::updateExpiredPassword()");

    int pamRC = _PAMUpdateExpiredPassword(
        userName.getCString(),
        oldPass.getCString(),
        newPass.getCString(),
        ipAddress.getCString());

    AuthenticationStatus authStatus = _getAuthStatusFromPAM_RC(pamRC);
    
    PEG_METHOD_EXIT();
    return authStatus;
}


PEGASUS_NAMESPACE_END
