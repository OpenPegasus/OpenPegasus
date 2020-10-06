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

#ifndef Pegasus_AuthenticationInfo_h
#define Pegasus_AuthenticationInfo_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/InternalException.h>
#include <Pegasus/Common/Tracer.h>
#include <Pegasus/Common/AuthenticationInfoRep.h>
#include <Pegasus/Common/Linkage.h>
#include <Pegasus/Common/SSLContext.h>


#ifdef PEGASUS_NEGOTIATE_AUTHENTICATION
// TODO: write proper CIMKerberosSecurityAssociation with opaque types
#include <Pegasus/Common/Negotiate.h>
#endif //PEGASUS_NEGOTIATE_AUTHENTICATION

PEGASUS_NAMESPACE_BEGIN

/**
    This class keeps the authentication information of a connection
    persistent until the connection is destroyed.

    The HTTPConnection object creates a AuthenticationInfo object on a new
    socket connection and includes this object reference in the HTTPMessage
    that gets passed to the Delegator and in turn to the AuthenticationManager.
    The AuthenticationManager and the related authentication classes use the
    AuthenticationInfo to store and access the persistent authentication
    information for a connection.
*/
class PEGASUS_COMMON_LINKAGE AuthenticationInfo
{
public:

    /** Constructor - Creates an uninitialized new AuthenticationInfo
        object representing an AuthenticationInfo class. The class object
        created by this constructor can only be used in an operation such as the
        copy constructor.  It cannot be used to do method calls such as
        getAuthType, since it is uninitialized.

        Use one of the other constructors to create an initiated new
        AuthenticationInfo class object. Throws an exception
        "unitialized handle" if this unitialized handle is used for
        method calls.
    */
    AuthenticationInfo() : _rep(0)
    {
        PEG_METHOD_ENTER(
            TRC_AUTHENTICATION, "AuthenticationInfo::AuthenticationInfo");


        PEG_METHOD_EXIT();
    }

    /** Creates and instantiates a AuthenticationInfo from another
        AuthenticationInfo instance
        @return pointer to the new AuthenticationInfo instance
    */
    AuthenticationInfo(const AuthenticationInfo& x)
    {
        PEG_METHOD_ENTER(
            TRC_AUTHENTICATION, "AuthenticationInfo::AuthenticationInfo");

        Inc(_rep = x._rep);

        PEG_METHOD_EXIT();
    }

    /** Assignment operator */
    AuthenticationInfo& operator=(const AuthenticationInfo& x)
    {
        PEG_METHOD_ENTER(
            TRC_AUTHENTICATION, "AuthenticationInfo::AuthenticationInfo");

        if (x._rep != _rep)
        {
            Dec(_rep);
            Inc(_rep = x._rep);
        }

        PEG_METHOD_EXIT();
        return *this;
    }

    /** Constructor - Instantiates a AuthenticationInfo object.
    @param flag - used only to distinguish from the default constructor.
    */
    AuthenticationInfo(Boolean)
    {
        PEG_METHOD_ENTER(
            TRC_AUTHENTICATION, "AuthenticationInfo::AuthenticationInfo");

        _rep = new AuthenticationInfoRep();

        PEG_METHOD_EXIT();
    }

    /** Destructor  */
    ~AuthenticationInfo()
    {
        PEG_METHOD_ENTER(
            TRC_AUTHENTICATION, "AuthenticationInfo::~AuthenticationInfo");

        Dec(_rep);

        PEG_METHOD_EXIT();
    }

    /** Sets the connection authentication status of the request to the
        status specified.
        @param status - the new authentication status
    */
    void   setConnectionAuthenticated(Boolean status)
    {
        CheckRep(_rep);
        _rep->setConnectionAuthenticated(status);
    }

    /** Get the previously authenticated user name
        @return the authenticated user name
    */
    String getAuthenticatedUser() const
    {
        CheckRep(_rep);
        return _rep->getAuthenticatedUser();
    }

    /** Sets the authenticated user name
        @param userName - string containing the authenticated user name
    */
    void   setAuthenticatedUser(const String& userName)
    {
        CheckRep(_rep);
        _rep->setAuthenticatedUser(userName);
    }

#ifdef PEGASUS_OS_ZOS

    /** The connection user is for z/OS only.
        On z/OS Unix Local Domain Sockets and sockets
        protected by AT-TLS are able to get the user ID of
        the connected user.
        This information is needed for later authentication
        steps.
     */

    /** Get the connection user name
        @return the connection user name
    */
    String getConnectionUser() const
    {
        CheckRep(_rep);
        return _rep->getConnectionUser();
    }

    /** Sets the connection user name
        @param userName - string containing the user name
                           provided by the connection
    */
    void   setConnectionUser(const String& userName)
    {
        CheckRep(_rep);
        _rep->setConnectionUser(userName);
    }

#endif

    /** Get the previously authenticated password
        @return the authenticated password
    */
    String getAuthenticatedPassword() const
    {
        CheckRep(_rep);
        return _rep->getAuthenticatedPassword();
    }

    /** Sets the authenticated password
        @param password - string containing the authenticated password
    */
    void   setAuthenticatedPassword(const String& password)
    {
        CheckRep(_rep);
        _rep->setAuthenticatedPassword(password);
    }

    /** Get the local authentication file path that was sent to client
        @return string containing the authentication file path
    */
    String getLocalAuthFilePath() const
    {
        CheckRep(_rep);
        return _rep->getLocalAuthFilePath();
    }

    /** Set the local authentication file path to the specified file path
        @param filePath String containing the authentication file path
    */
    void setLocalAuthFilePath(const String& filePath)
    {
        CheckRep(_rep);
        _rep->setLocalAuthFilePath(filePath);
    }

    /** Get the local authentication secret that was sent to client
        @return string containing the authentication secret
    */
    String getLocalAuthSecret() const
    {
        CheckRep(_rep);
        return _rep->getLocalAuthSecret();
    }

    /** Set the local authentication secret to the specified secret
        @param secret - string containing the authentication secret
    */
    void   setLocalAuthSecret(const String& secret)
    {
        CheckRep(_rep);
        _rep->setLocalAuthSecret(secret);
    }

    /** Is the request authenticated
    */
    /** Returns the authentication status of the current connection.
        @return true if the connection was authenticated, false otherwise
    */
    Boolean isConnectionAuthenticated() const
    {
        CheckRep(_rep);
        return _rep->isConnectionAuthenticated();
    }

    /** Set the authentication type to the specified type
        @param string containing the authentication type
    */
    void   setAuthType(const String& authType)
    {
        CheckRep(_rep);
        _rep->setAuthType(authType);
    }

    /** Get the authentication type of the connection
        @return string containing the authentication type
    */
    String getAuthType() const
    {
        CheckRep(_rep);
        return _rep->getAuthType();
    }

    /**
        Set the IP address to the specified IP address
        @param string containing the IP address
    */
    void setIpAddress(const String& ipAddress)
    {
        CheckRep(_rep);
        _rep->setIpAddress(ipAddress);
    }

    /**
        Get the IP address of the connection

        NOTE: The IP address is for debug use only.
        It should not be used for authentication purposes.

        @return string containing the IP address
    */
    String getIpAddress() const
    {
        CheckRep(_rep);
        return _rep->getIpAddress();
    }

#ifdef PEGASUS_NEGOTIATE_AUTHENTICATION
    /** Get GSSAPI context for this connection. */
    SharedPtr<NegotiateServerSession> getNegotiateSession()
    {
        CheckRep(_rep);
        return _rep->getNegotiateSession();
    }
#endif //PEGASUS_NEGOTIATE_AUTHENTICATION

    Array<SSLCertificateInfo*> getClientCertificateChain()
    {
        CheckRep(_rep);
        return _rep->getClientCertificateChain();
    }

    void setClientCertificateChain(Array<SSLCertificateInfo*> clientCertificate)
    {
        CheckRep(_rep);
        _rep->setClientCertificateChain(clientCertificate);
    }

    /** Set flag to show that isRemotePrivilegedUserAccess check has been done
        this function should only be used by OpenPegasus AuthenticationHandlers
    */
    void setRemotePrivilegedUserAccessChecked()
    {
        CheckRep(_rep);
        _rep->setRemotePrivilegedUserAccessChecked();
    }

    /** Indicates whether the isRemotePrivilegedUserAccess check has been
        performed.  This method should only be used by OpenPegasus
        AuthenticationHandlers
    */
    Boolean getRemotePrivilegedUserAccessChecked()
    {
        CheckRep(_rep);
        return _rep->getRemotePrivilegedUserAccessChecked();
    }

    void setAuthHandle(const AuthHandle & authHandle)
    {
        CheckRep(_rep);
        _rep->setAuthHandle(authHandle);
    }

    AuthHandle getAuthHandle()
    {
        CheckRep(_rep);
        return _rep->getAuthHandle();
    }

    void setUserRole(const String & userRole)
    {
        CheckRep(_rep);
        _rep->setUserRole(userRole);
    }

    String getUserRole()
    {
        CheckRep(_rep);
        return _rep->getUserRole();
    }

    void setExpiredPassword(Boolean status)
    {
        CheckRep(_rep);
        _rep->setExpiredPassword(status);
    }

    Boolean isExpiredPassword() const
    {
        CheckRep(_rep);
        return _rep->isExpiredPassword();
    }

#ifdef PEGASUS_ENABLE_SESSION_COOKIES
    /**
     * Value of Cookie: header to send in the next response
     */
    void setCookie(const String &value)
    {
        CheckRep(_rep);
        _rep->setCookie(value);
    }

    String getCookie() const
    {
        CheckRep(_rep);
        return _rep->getCookie();
    }
#endif

private:

    AuthenticationInfo(AuthenticationInfoRep* rep) : _rep(rep)
    {

    }

    AuthenticationInfoRep* _rep;
};

PEGASUS_NAMESPACE_END

#endif   /* Pegasus_AuthenticationInfo_h*/
