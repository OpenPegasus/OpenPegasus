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

#ifndef Pegasus_AuthenticationInfoRep_h
#define Pegasus_AuthenticationInfoRep_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/ArrayInternal.h>
#include <Pegasus/Common/String.h>
#include <Pegasus/Common/Sharable.h>
#include <Pegasus/Common/Linkage.h>
#include <Pegasus/Common/SSLContext.h>
#include <Pegasus/Common/AuthHandle.h>


#ifdef PEGASUS_NEGOTIATE_AUTHENTICATION
#include <Pegasus/Common/Negotiate.h>
#endif //PEGASUS_NEGOTIATE_AUTHENTICATION

PEGASUS_NAMESPACE_BEGIN

class AuthenticationInfo;

/**
    This class keeps the authentication information of a connection
    persistent until the connection is destroyed.
*/
class PEGASUS_COMMON_LINKAGE AuthenticationInfoRep :  public Sharable
{
public:

    //ATTN: we should be using an enumeration for the authtype instead of a
    //string.
    //In the AuthenticationManager, the authtype is set to Basic, Digest, etc
    //We also need to be able to check whether the type is SSL, so I'm adding a
    //string here to make it less arbitrary.  PEP165
    static const String AUTH_TYPE_SSL;
    static const String AUTH_TYPE_ZOS_LOCAL_DOMIAN_SOCKET;
    static const String AUTH_TYPE_ZOS_ATTLS;
    static const String AUTH_TYPE_COOKIE;

    AuthenticationInfoRep();

    ~AuthenticationInfoRep();

    void   setConnectionAuthenticated(Boolean connectionAuthenticated);

    String getAuthenticatedUser() const
    {
        return _authUser;
    }

    void   setAuthenticatedUser(const String& userName);

#ifdef PEGASUS_OS_ZOS

    // The connection user is for z/OS only.
    // On z/OS Unix Local Domain Sockets and sockets
    // protected by AT-TLS are able to get the user ID of
    // the connected user.
    // This information is needed for later authentication
    // steps.

    String getConnectionUser() const
    {        return _connectionUser;
    }

    void   setConnectionUser(const String& userName);

#endif

    String getAuthenticatedPassword() const
    {
        return _authPassword;
    }

    void   setAuthenticatedPassword(const String& password);

    String getLocalAuthFilePath() const
    {
        return _localAuthFilePath;
    }

    void setLocalAuthFilePath(const String& filePath);

    String getLocalAuthSecret() const
    {
        return _localAuthSecret;
    }

    void   setLocalAuthSecret(const String& secret);

    Boolean isConnectionAuthenticated() const
    {
        return _connectionAuthenticated;
    }

    String getAuthType() const
    {
        return _authType;
    }

    void   setAuthType(const String& authType);

    void setIpAddress(const String& ipAddress)
    {
        _ipAddress = ipAddress;
    }

    String getIpAddress()
    {
        return _ipAddress;
    }

#ifdef PEGASUS_NEGOTIATE_AUTHENTICATION
    /** Get GSSAPI context for this connection. */
    SharedPtr<NegotiateServerSession> getNegotiateSession()
    {
        return _session;
    }
#endif //PEGASUS_NEGOTIATE_AUTHENTICATION

    Array<SSLCertificateInfo*> getClientCertificateChain()
    {
        return _clientCertificate;
    }

    void setClientCertificateChain(
        Array<SSLCertificateInfo*> clientCertificate);

    void setRemotePrivilegedUserAccessChecked()
    {
        _wasRemotePrivilegedUserAccessChecked = true;
    }

    Boolean getRemotePrivilegedUserAccessChecked()
    {
        return _wasRemotePrivilegedUserAccessChecked;
    }

    void setAuthHandle(const AuthHandle& authHandle)
    {
        _authHandle = authHandle;
    }

    AuthHandle getAuthHandle()
    {
        return _authHandle;
    }

    void setUserRole(const String& userRole)
    {
        _userRole = userRole;
    }

    String getUserRole()
    {
        return _userRole;
    }

    void setExpiredPassword(Boolean status)
    {
        _isExpiredPassword = status;
    }

    Boolean isExpiredPassword() const
    {
        return _isExpiredPassword;
    }

#ifdef PEGASUS_ENABLE_SESSION_COOKIES
    void setCookie(const String &value)
    {
        _cookie = value;
    }

    String getCookie() const
    {
        return _cookie;
    }
#endif

private:

    /** Default Copy Constructor and assignment operator  */
    AuthenticationInfoRep(const AuthenticationInfoRep& x);
    AuthenticationInfoRep& operator=(const AuthenticationInfoRep& x);

    String  _authUser;
    String  _authPassword;
    String  _localAuthSecret;
    String  _localAuthFilePath;
#ifdef PEGASUS_OS_ZOS
    String  _connectionUser;
#endif
    String  _authType;
    Boolean _connectionAuthenticated;
    String  _ipAddress;

#ifdef PEGASUS_NEGOTIATE_AUTHENTICATION
    SharedPtr<NegotiateServerSession> _session;
#endif //PEGASUS_NEGOTIATE_AUTHENTICATION
    
    Boolean _wasRemotePrivilegedUserAccessChecked;

    Array<SSLCertificateInfo*> _clientCertificate;

    AuthHandle _authHandle;
    String _userRole;
    Boolean _isExpiredPassword;
#ifdef PEGASUS_ENABLE_SESSION_COOKIES
    String  _cookie;
#endif
};

PEGASUS_NAMESPACE_END

#endif   /* Pegasus_AuthenticationInfoRep_h*/
