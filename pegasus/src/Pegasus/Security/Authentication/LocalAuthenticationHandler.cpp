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

#include <Pegasus/Common/AuditLogger.h>
#include <Pegasus/Common/Logger.h>
#include <Pegasus/Common/Tracer.h>

#include "SecureLocalAuthenticator.h"
#include "LocalAuthenticationHandler.h"

#ifdef PEGASUS_ZOS_SECURITY
// This include file will not be provided in the OpenGroup CVS for now.
// Do NOT try to include it in your compile
# include <Pegasus/Common/safCheckzOS_inline.h>
#endif

PEGASUS_USING_STD;

PEGASUS_NAMESPACE_BEGIN


LocalAuthenticationHandler::LocalAuthenticationHandler()
{
    PEG_METHOD_ENTER(TRC_AUTHENTICATION,
       "LocalAuthenticationHandler::LocalAuthenticationHandler()");

    _localAuthenticator.reset(
        (LocalAuthenticator*) new SecureLocalAuthenticator());

    PEG_METHOD_EXIT();
}

LocalAuthenticationHandler::~LocalAuthenticationHandler()
{
    PEG_METHOD_ENTER(TRC_AUTHENTICATION,
        "LocalAuthenticationHandler::~LocalAuthenticationHandler()");

    PEG_METHOD_EXIT();
}

AuthenticationStatus LocalAuthenticationHandler::authenticate(
    const String& authHeader,
    AuthenticationInfo* authInfo)
{
    PEG_METHOD_ENTER(TRC_AUTHENTICATION,
        "LocalAuthenticationHandler::authenticate()");

    // Look for ':' seperator
    Uint32 colon1 = authHeader.find(':');

    if (colon1 == PEG_NOT_FOUND)
    {
        PEG_METHOD_EXIT();
        return AuthenticationStatus(AUTHSC_UNAUTHORIZED);
    }

    String userName = authHeader.subString(0, colon1);

    // Look for another ':' seperator
    Uint32 colon2 = authHeader.find(colon1 + 1, ':');

    String filePath;

    String secretReceived;

    if (colon2 == PEG_NOT_FOUND)
    {
        filePath = String::EMPTY;

        secretReceived = authHeader.subString(colon1 + 1);
    }
    else
    {
        filePath = authHeader.subString(colon1 + 1, (colon2 - colon1 - 1));

        secretReceived = authHeader.subString(colon2 + 1);
    }

    //
    // Check for the expected file path in the authentication header
    //
    if (filePath != authInfo->getLocalAuthFilePath())
    {
        PEG_METHOD_EXIT();
        return AuthenticationStatus(AUTHSC_UNAUTHORIZED);
    }

    //
    // Check if the authentication information is present
    //
    if (secretReceived.size() == 0 || userName.size() == 0)
    {
        PEG_METHOD_EXIT();
        return AuthenticationStatus(AUTHSC_UNAUTHORIZED);
    }

    String authenticatedUsername = authInfo->getAuthenticatedUser();

    //
    // If this connection has been previously authenticated then ensure
    // the username passed with the current request matches the
    // username previously authenticated.
    //
    if (authenticatedUsername.size() != 0 &&
        userName != authenticatedUsername)
    {
        PEG_METHOD_EXIT();
        return AuthenticationStatus(AUTHSC_UNAUTHORIZED);
    }

    //
    // Check if the user is a valid system user
    //
    if (!System::isSystemUser(userName.getCString()))
    {
        PEG_METHOD_EXIT();
        return AuthenticationStatus(AUTHSC_UNAUTHORIZED);
    }

    // Check if the user is authorized to CIMSERV
#ifdef PEGASUS_ZOS_SECURITY
    if (!CheckProfileCIMSERVclassWBEM(userName, __READ_RESOURCE))
    {
        Logger::put_l(Logger::STANDARD_LOG, ZOS_SECURITY_NAME, Logger::WARNING,
            MessageLoaderParms(
                "Security.Authentication.LocalAuthenticationHandler."
                    "NOREAD_CIMSERV_ACCESS.PEGASUS_OS_ZOS",
                "Request UserID $0 doesn't have READ permission "
                    "to profile CIMSERV CL(WBEM).",
                userName));
        return AuthenticationStatus(AUTHSC_UNAUTHORIZED);
    }
#endif

    // It is not necessary to check remote privileged user access for local
    // connections; set the flag to "check done"
    authInfo->setRemotePrivilegedUserAccessChecked();

    // Authenticate
    AuthenticationStatus authStatus = _localAuthenticator->authenticate(
        filePath, secretReceived, authInfo->getLocalAuthSecret());

    if (authStatus.isSuccess())
    {
        authInfo->setAuthenticatedUser(userName);
        // For Privilege Separation, remember the secret on subsequent requests
        authInfo->setLocalAuthSecret(secretReceived);
    }
    else
    {
        // log a failed authentication
        Logger::put_l(
            Logger::STANDARD_LOG, System::CIMSERVER, Logger::INFORMATION,
            MessageLoaderParms(
                "Security.Authentication.LocalAuthenticationHandler."
                    "LOCAL_AUTHENTICATION_FAILURE",
                "Local Authentication failed for user $0 from client "
                "IP address $1.",userName,authInfo->getIpAddress()));
    }

    PEG_AUDIT_LOG(logLocalAuthentication(userName, authStatus.isSuccess()));

    PEG_METHOD_EXIT();

    return authStatus;
}

AuthenticationStatus LocalAuthenticationHandler::validateUser(
    const String& userName,
    AuthenticationInfo* authInfo)
{
    return _localAuthenticator->validateUser(userName,authInfo);
}

String LocalAuthenticationHandler::getAuthResponseHeader(
    const String& authType,
    const String& userName,
    AuthenticationInfo* authInfo)
{
    PEG_METHOD_ENTER(TRC_AUTHENTICATION,
        "LocalAuthenticationHandler::getAuthResponseHeader()");

    String secret;
    String filePath;
    String authResp;

    //
    // Check if the user is a valid system user
    //
    if (!System::isSystemUser(userName.getCString()))
    {
        PEG_METHOD_EXIT();
        return authResp;
    }

    authResp = _localAuthenticator->getAuthResponseHeader(
        authType, userName, filePath, secret);

    authInfo->setLocalAuthFilePath(filePath);
    authInfo->setLocalAuthSecret(secret);

    PEG_METHOD_EXIT();

    return authResp;
}

PEGASUS_NAMESPACE_END
