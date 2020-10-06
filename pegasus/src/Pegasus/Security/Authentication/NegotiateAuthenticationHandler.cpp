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

#include "NegotiateAuthenticationHandler.h"
#include <Pegasus/Common/Base64.h>
#include <Pegasus/Config/ConfigManager.h>
//PEGASUS_USING_STD;

PEGASUS_NAMESPACE_BEGIN


static const char NEGOTIATE_AUTHENTICATION_FAILED_KEY [] =
    "Security.Authentication.NegotiateAuthenticationHandler."
        "BASIC_AUTHENTICATION_FAILURE";

static const char NEGOTIATE_AUTHENTICATION_FAILED[] =
    "Authentication failed for user $0 from client IP address $1.";

static const char NEGOTIATE_AUTHENTICATION_SUCCESS_KEY [] =
    "Security.Authentication.NegotiateAuthenticationHandler."
        "BASIC_AUTHENTICATION_FAILURE";

static const char NEGOTIATE_AUTHENTICATION_SUCCESS[] =
    "Authentication succeeded for user $0 from client IP address $1.";

static const char NEGOTIATE_AUTHENTICATION_CONTINUE_KEY [] =
    "Security.Authentication.NegotiateAuthenticationHandler."
        "BASIC_AUTHENTICATION_FAILURE";

static const char NEGOTIATE_AUTHENTICATION_CONTINUE[] =
    "Authentication continues for user $0 from client IP address $1.";

static const String NEGOTIATE_AUTHENTICATION_CHALLENGE_HEADER =
    "WWW-Authenticate: Negotiate ";

NegotiateAuthenticationHandler::NegotiateAuthenticationHandler()
{
    PEG_METHOD_ENTER(TRC_AUTHENTICATION,
       "NegotiateAuthenticationHandler::NegotiateAuthenticationHandler()");

    PEG_METHOD_EXIT();
}

NegotiateAuthenticationHandler::~NegotiateAuthenticationHandler()
{
    PEG_METHOD_ENTER(TRC_AUTHENTICATION,
        "NegotiateAuthenticationHandler::~NegotiateAuthenticationHandler()");

    PEG_METHOD_EXIT();
}

AuthenticationStatus NegotiateAuthenticationHandler::authenticate(
    const String& authHeader,
    AuthenticationInfo* authInfo)
{
    PEG_METHOD_ENTER(TRC_AUTHENTICATION,
        "NegotiateAuthenticationHandler::authenticate()");

    ConfigManager* configManager = ConfigManager::getInstance();
    Boolean mapToLocalName = ConfigManager::parseBooleanValue(
        configManager->getCurrentValue("mapToLocalName"));

    SharedPtr<NegotiateServerSession> session = authInfo->getNegotiateSession();
    String userName;
    NegotiateAuthenticationStatus status = session->authenticate(authHeader,
            userName, mapToLocalName);

    if (userName != String::EMPTY)
        authInfo->setAuthenticatedUser(userName);

    bool authenticated = false;
    switch(status)
    {
    case NEGOTIATE_SUCCESS:
        // authentication finished successfully, audit it
        PEG_AUDIT_LOG(logBasicAuthentication(
            userName,
            authInfo->getIpAddress(),
            true));

        Logger::put_l(Logger::STANDARD_LOG, System::CIMSERVER,
            Logger::TRACE,
            MessageLoaderParms(
                    NEGOTIATE_AUTHENTICATION_SUCCESS_KEY,
                    NEGOTIATE_AUTHENTICATION_SUCCESS,
                    userName,
                    authInfo->getIpAddress()));
        authenticated = true;
        break;

    case NEGOTIATE_CONTINUE:
        Logger::put_l(Logger::STANDARD_LOG, System::CIMSERVER,
            Logger::TRACE,
            MessageLoaderParms(
                    NEGOTIATE_AUTHENTICATION_CONTINUE_KEY,
                    NEGOTIATE_AUTHENTICATION_CONTINUE,
                    userName,
                    authInfo->getIpAddress()));
        break;

    case NEGOTIATE_FAILED:
        String userName = authInfo->getAuthenticatedUser();
        // audit the failure
        PEG_AUDIT_LOG(logBasicAuthentication(
                userName,
                authInfo->getIpAddress(),
                false));
        break;
    }

    PEG_METHOD_EXIT();
    return AuthenticationStatus(authenticated);
}

AuthenticationStatus NegotiateAuthenticationHandler::validateUser(
    const String& userName,
    AuthenticationInfo* authInfo)
{
    return true;
}

String NegotiateAuthenticationHandler::getAuthResponseHeader(
    const String& authType,
    const String& userName,
    AuthenticationInfo* authInfo)
{
    PEG_METHOD_ENTER(TRC_AUTHENTICATION,
        "NegotiateAuthenticationHandler::getAuthResponseHeader()");

    String authResp = NEGOTIATE_AUTHENTICATION_CHALLENGE_HEADER;
    SharedPtr<NegotiateServerSession> session = authInfo->getNegotiateSession();
    String challenge = session->getChallenge();
    authResp.append(challenge);
    PEG_TRACE((TRC_AUTHENTICATION, Tracer::LEVEL4,
            "getAuthResponseHeader returning: %s",
            (const char*)authResp.getCString()));

    PEG_METHOD_EXIT();
    return authResp;
}

PEGASUS_NAMESPACE_END
