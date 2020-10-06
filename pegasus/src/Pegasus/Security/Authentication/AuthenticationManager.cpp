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

#include <Pegasus/Common/System.h>
#include <Pegasus/Common/XmlWriter.h>
#include <Pegasus/Common/Tracer.h>
#include <Pegasus/Common/PegasusVersion.h>
#include <Pegasus/Common/HTTPMessage.h>

#include <Pegasus/Config/ConfigManager.h>

#include "LocalAuthenticationHandler.h"
#include "BasicAuthenticationHandler.h"
#include "AuthenticationManager.h"

#include <Pegasus/Common/AutoPtr.h>

#ifdef PEGASUS_NEGOTIATE_AUTHENTICATION
#include "NegotiateAuthenticationHandler.h"
#endif

PEGASUS_USING_STD;

PEGASUS_NAMESPACE_BEGIN

//
// Constructor
//
AuthenticationManager::AuthenticationManager()
{
    PEG_METHOD_ENTER(
        TRC_AUTHENTICATION, "AuthenticationManager::AuthenticationManager()");

    //
    // get authentication handlers
    //
    _localAuthHandler = _getLocalAuthHandler();

    _httpAuthHandler = _getHttpAuthHandler();

    PEG_METHOD_EXIT();
}

//
// Destructor
//
AuthenticationManager::~AuthenticationManager()
{
    PEG_METHOD_ENTER(
        TRC_AUTHENTICATION, "AuthenticationManager::~AuthenticationManager()");

    //
    // delete authentication handlers
    //
    delete _localAuthHandler;
    delete _httpAuthHandler;

    PEG_METHOD_EXIT();
}

Boolean AuthenticationManager::isRemotePrivilegedUserAccessAllowed(
        String & userName)
{
    //
    // Reject access if the user is privileged and remote privileged user
    // access is not enabled.
    //
    if (!ConfigManager::parseBooleanValue(ConfigManager::getInstance()->
            getCurrentValue("enableRemotePrivilegedUserAccess"))
        && System::isPrivilegedUser(userName))
    {
        Logger::put_l(
            Logger::STANDARD_LOG, System::CIMSERVER, Logger::INFORMATION,
            MessageLoaderParms(
                "Security.Authentication.BasicAuthenticationHandler."
                    "PRIVILEGED_ACCESS_DISABLED",
                "Authentication failed for user '$0' because "
                    "enableRemotePrivilegedUserAccess is not set to 'true'.",
                userName));
        return false;
    }
    return true;
}

//
// Perform http authentication
//
AuthenticationStatus AuthenticationManager::performHttpAuthentication(
    const String& authHeader,
    AuthenticationInfo* authInfo)
{
    PEG_METHOD_ENTER(TRC_AUTHENTICATION,
        "AuthenticationManager::performHttpAuthentication()");

    String authType;
    String cookie;

    //
    // Parse the HTTP authentication header for authentication information
    //
    if ( !HTTPMessage::parseHttpAuthHeader(authHeader, authType, cookie) )
    {
        PEG_TRACE((
            TRC_DISCARDED_DATA,
            Tracer::LEVEL1,
            "HTTPAuthentication failed. "
                "Malformed HTTP authentication header: %s",
            (const char*)authHeader.getCString()));
        PEG_METHOD_EXIT();
        return AuthenticationStatus(AUTHSC_UNAUTHORIZED);
    }

    AuthenticationStatus authStatus(AUTHSC_UNAUTHORIZED);

    //
    // Check the authenticationinformation and do the authentication
    //
    if ( String::equalNoCase(authType, "Basic") &&
         String::equal(_httpAuthType, "Basic") )
    {
        authStatus = _httpAuthHandler->authenticate(cookie, authInfo);
    }
#ifdef PEGASUS_NEGOTIATE_AUTHENTICATION
    else if ( String::equalNoCase(authType, "Negotiate") &&
              String::equal(_httpAuthType, "Negotiate") )
    {
        authStatus = _httpAuthHandler->authenticate(cookie, authInfo);
    }
#endif
    // FUTURE: Add code to check for "Digest" when digest
    // authentication is implemented.

    if ( authStatus.isSuccess() )
    {
        authInfo->setAuthType(authType);
    }

    PEG_METHOD_EXIT();

    return authStatus;
}

//
// Perform pegasus sepcific local authentication
//
AuthenticationStatus AuthenticationManager::performPegasusAuthentication(
    const String& authHeader,
    AuthenticationInfo* authInfo)
{
    PEG_METHOD_ENTER(TRC_AUTHENTICATION,
        "AuthenticationManager::performPegasusAuthentication()");

    AuthenticationStatus authStatus(AUTHSC_UNAUTHORIZED);

    String authType;
    String userName;
    String cookie;

    //
    // Parse the pegasus authentication header authentication information
    //
    if ( !HTTPMessage::parseLocalAuthHeader(authHeader,
              authType, userName, cookie) )
    {
        PEG_TRACE((
            TRC_DISCARDED_DATA,
            Tracer::LEVEL1,
            "PegasusAuthentication failed. "
                "Malformed Pegasus authentication header: %s",
            (const char*)authHeader.getCString()));
        PEG_METHOD_EXIT();
        return AuthenticationStatus(AUTHSC_UNAUTHORIZED);
    }

    // The HTTPAuthenticatorDelegator ensures only local authentication
    // requests get here.
    PEGASUS_ASSERT(authType == "Local");

    authStatus = _localAuthHandler->authenticate(cookie, authInfo);

    if ( authStatus.isSuccess() )
    {
        authInfo->setAuthType(authType);
    }

    PEG_METHOD_EXIT();

    return authStatus;
}

//
// Validate user.
//
AuthenticationStatus AuthenticationManager::validateUserForHttpAuth(
    const String& userName,
    AuthenticationInfo* authInfo)
{
    return _httpAuthHandler->validateUser(userName,authInfo);
}

//
// Get pegasus/local authentication response header
//
String AuthenticationManager::getPegasusAuthResponseHeader(
    const String& authHeader,
    AuthenticationInfo* authInfo)
{
    PEG_METHOD_ENTER(TRC_AUTHENTICATION,
        "AuthenticationManager::getPegasusAuthResponseHeader()");

    String respHeader;

    String authType;
    String userName;
    String cookie;

    //
    // Parse the pegasus authentication header authentication information
    //
    if ( !HTTPMessage::parseLocalAuthHeader(authHeader,
              authType, userName, cookie) )
    {
        PEG_METHOD_EXIT();
        return respHeader;
    }

    //
    // User name can not be empty
    //
    if (String::equal(userName, String::EMPTY))
    {
        PEG_METHOD_EXIT();
        return respHeader;
    }

    respHeader =
        _localAuthHandler->getAuthResponseHeader(authType, userName, authInfo);

    PEG_METHOD_EXIT();
    return respHeader;

}

//
// Get HTTP authentication response header
//
#ifdef PEGASUS_NEGOTIATE_AUTHENTICATION
String AuthenticationManager::getHttpAuthResponseHeader(
    AuthenticationInfo* authInfo)
#else
String AuthenticationManager::getHttpAuthResponseHeader()
#endif
{
    PEG_METHOD_ENTER(TRC_AUTHENTICATION,
        "AuthenticationManager::getHttpAuthResponseHeader()");

#ifdef PEGASUS_NEGOTIATE_AUTHENTICATION
    String respHeader = _httpAuthHandler->getAuthResponseHeader(
        String::EMPTY, String::EMPTY, authInfo);
#else
    String respHeader = _httpAuthHandler->getAuthResponseHeader();
#endif
    PEG_METHOD_EXIT();
    return respHeader;
}

//
// Get local authentication handler
//
Authenticator* AuthenticationManager::_getLocalAuthHandler()
{
    PEG_METHOD_ENTER(
        TRC_AUTHENTICATION, "AuthenticationManager::_getLocalAuthHandler()");

    PEG_METHOD_EXIT();
    //
    // create and return a local authentication handler.
    //
    return new LocalAuthenticationHandler();
}


//
// Get Http authentication handler
//
Authenticator* AuthenticationManager::_getHttpAuthHandler()
{
    PEG_METHOD_ENTER(
        TRC_AUTHENTICATION, "AuthenticationManager::_getHttpAuthHandler()");
    AutoPtr<Authenticator> handler;

    //
    // get the configured authentication type
    //
    AutoPtr<ConfigManager> configManager(ConfigManager::getInstance());

    _httpAuthType = configManager->getCurrentValue("httpAuthType");
    configManager.release();
    //
    // create a authentication handler.
    //
    if ( String::equal(_httpAuthType, "Basic") )
    {
        handler.reset((Authenticator* ) new BasicAuthenticationHandler( ));
    }
#ifdef PEGASUS_NEGOTIATE_AUTHENTICATION
    if ( String::equal(_httpAuthType, "Negotiate") )
    {
        handler.reset((Authenticator* ) new NegotiateAuthenticationHandler( ));
    }
#endif
    // FUTURE: uncomment these line when Digest authentication
    // is implemented.
    //
    //else if (String::equal(_httpAuthType, "Digest"))
    //{
    //    handler = (Authenticator* ) new DigestAuthenticationHandler( );
    //}
    else
    {
        //
        // This should never happen. Gets here only if Security Config
        // property owner has not validated the configured http auth type.
        //
        PEGASUS_UNREACHABLE(PEGASUS_ASSERT(0);)
    }

    PEG_METHOD_EXIT();
    return handler.release();
}

PEGASUS_NAMESPACE_END
