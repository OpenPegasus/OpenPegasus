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

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/Tracer.h>
#include "AuthenticationInfoRep.h"
#include <Pegasus/Common/SSLContext.h>
#include <Pegasus/Common/FileSystem.h>
#include <Pegasus/Common/Executor.h>

PEGASUS_USING_STD;

PEGASUS_NAMESPACE_BEGIN

const String AuthenticationInfoRep::AUTH_TYPE_SSL = "SSL";
const String AuthenticationInfoRep::AUTH_TYPE_ZOS_LOCAL_DOMIAN_SOCKET = "LDS";
const String AuthenticationInfoRep::AUTH_TYPE_ZOS_ATTLS = "ATTLS";
const String AuthenticationInfoRep::AUTH_TYPE_COOKIE = "COOKIE";

AuthenticationInfoRep::AuthenticationInfoRep()
    : _connectionAuthenticated(false),
      _wasRemotePrivilegedUserAccessChecked(false),
      _authHandle(),
      _isExpiredPassword(false)
#ifdef PEGASUS_ENABLE_SESSION_COOKIES
      ,_cookie()
#endif
{
    PEG_METHOD_ENTER(
        TRC_AUTHENTICATION, "AuthenticationInfoRep::AuthenticationInfoRep");
#ifdef PEGASUS_NEGOTIATE_AUTHENTICATION
      _session.reset(new NegotiateServerSession());
#endif
    PEG_METHOD_EXIT();
}

AuthenticationInfoRep::~AuthenticationInfoRep()
{
    PEG_METHOD_ENTER(
        TRC_AUTHENTICATION, "AuthenticationInfoRep::~AuthenticationInfoRep");

    // initiate the deletion of _localAuthFilePath.
    if(FileSystem::exists(_localAuthFilePath))
    {
        // No response was received from the local client for the 
        // authentication challenge. Hence deleting the file here.

        // Use executor, if present.
        if (Executor::detectExecutor() == 0)
        {
            Executor::removeFile(_localAuthFilePath.getCString());
        }
        else
        {
            FileSystem::removeFile(_localAuthFilePath);
        }
    }

    PEG_METHOD_EXIT();
}

void AuthenticationInfoRep::setConnectionAuthenticated(
    Boolean connectionAuthenticated)
{
    PEG_METHOD_ENTER(TRC_AUTHENTICATION,
        "AuthenticationInfoRep::setConnectionAuthenticated");

    _connectionAuthenticated = connectionAuthenticated;

    PEG_METHOD_EXIT();
}

#ifdef PEGASUS_OS_ZOS

    // The connection user is for z/OS only.
    // On z/OS Unix Local Domain Sockets and sockets
    // protected by AT-TLS are able to get the user ID of
    // the connected user.
    // This information is needed for later authentication
    // steps.

void AuthenticationInfoRep::setConnectionUser(const String& userName)
{
    PEG_METHOD_ENTER(
        TRC_AUTHENTICATION, "AuthenticationInfoRep::setConnectionUser()");

    _connectionUser = userName;

    PEG_METHOD_EXIT();
}
#endif

void AuthenticationInfoRep::setAuthenticatedUser(const String& userName)
{
    PEG_METHOD_ENTER(
        TRC_AUTHENTICATION, "AuthenticationInfoRep::setAuthenticatedUser");

    _authUser = userName;

    PEG_METHOD_EXIT();
}

void AuthenticationInfoRep::setAuthenticatedPassword(const String& password)
{
    PEG_METHOD_ENTER(
        TRC_AUTHENTICATION, "AuthenticationInfoRep::setAuthenticatedPassword");

    _authPassword = password;

    PEG_METHOD_EXIT();
}

void AuthenticationInfoRep::setLocalAuthFilePath(const String& filePath)
{
    PEG_METHOD_ENTER(
        TRC_AUTHENTICATION, "AuthenticationInfoRep::setLocalAuthFilePath");

    _localAuthFilePath = filePath;

    PEG_METHOD_EXIT();
}

void AuthenticationInfoRep::setLocalAuthSecret(const String& secret)
{
    PEG_METHOD_ENTER(
        TRC_AUTHENTICATION, "AuthenticationInfoRep::setLocalAuthSecret");

    _localAuthSecret = secret;

    PEG_METHOD_EXIT();
}

void AuthenticationInfoRep::setAuthType(const String& authType)
{
    PEG_METHOD_ENTER(
        TRC_AUTHENTICATION, "AuthenticationInfoRep::setAuthType");

    _authType = authType;

    PEG_METHOD_EXIT();
}

void AuthenticationInfoRep::setClientCertificateChain(
    Array<SSLCertificateInfo*> clientCertificate)
{
    PEG_METHOD_ENTER(TRC_AUTHENTICATION,
        "AuthenticationInfoRep::setClientCertificateChain");

    _clientCertificate = clientCertificate;

    PEG_METHOD_EXIT();
}

PEGASUS_NAMESPACE_END
