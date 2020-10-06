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

#include <Pegasus/Common/Executor.h>
#include <Pegasus/Config/ConfigManager.h>
#include <Pegasus/Common/Tracer.h>
#include "PAMBasicAuthenticator.h"
#include "pam_rcToAuthStatus.h"

PEGASUS_USING_STD;

PEGASUS_NAMESPACE_BEGIN

PAMBasicAuthenticator::PAMBasicAuthenticator()
{
    PEG_METHOD_ENTER(TRC_AUTHENTICATION,
        "PAMBasicAuthenticator::PAMBasicAuthenticator()");

    // Build Authentication parameter realm required for Basic Challenge
    // e.g. realm="HostName"

    _realm.assign("realm=");
    _realm.append(Char16('"'));
    _realm.append(System::getHostName());
    _realm.append(Char16('"'));

    PEG_METHOD_EXIT();
}

PAMBasicAuthenticator::~PAMBasicAuthenticator()
{
    PEG_METHOD_ENTER(TRC_AUTHENTICATION,
        "PAMBasicAuthenticator::~PAMBasicAuthenticator()");

    PEG_METHOD_EXIT();
}

AuthenticationStatus PAMBasicAuthenticator::authenticate(
    const String& userName,
    const String& password,
    AuthenticationInfo* authInfo)
{
    PEG_METHOD_ENTER(TRC_AUTHENTICATION,
        "PAMBasicAuthenticator::authenticate()");

    int pam_rc =
        Executor::authenticatePassword(
            userName.getCString(),
            password.getCString());

    // return code of -1 will be translated to AUTHSC_UNAUTHORIZED
    AuthenticationStatus authStatus = _getAuthStatusFromPAM_RC(pam_rc);
    PEG_METHOD_EXIT();
    return authStatus;
}

AuthenticationStatus PAMBasicAuthenticator::validateUser(
    const String& userName,
    AuthenticationInfo* authInfo)
{
    PEG_METHOD_ENTER(TRC_AUTHENTICATION,
        "PAMBasicAuthenticator::validateUser()");

    int pam_rc = Executor::validateUser(userName.getCString());
    // return code of -1 will be translated to AUTHSC_UNAUTHORIZED
    AuthenticationStatus authStatus = _getAuthStatusFromPAM_RC(pam_rc);
    PEG_METHOD_EXIT();
    return authStatus;
}


String PAMBasicAuthenticator::getAuthResponseHeader()
{
    PEG_METHOD_ENTER(TRC_AUTHENTICATION,
        "PAMBasicAuthenticator::getAuthResponseHeader()");

    // Build response header: WWW-Authenticate: Basic realm="HostName"

    String responseHeader = "WWW-Authenticate: Basic ";
    responseHeader.append(_realm);

    PEG_METHOD_EXIT();
    return responseHeader;
}

PEGASUS_NAMESPACE_END
