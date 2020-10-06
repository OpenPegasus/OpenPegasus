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

#ifndef Pegasus_ClientAuthenticator_h
#define Pegasus_ClientAuthenticator_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/Message.h>
#include <Pegasus/Common/HTTPMessage.h>
#include <Pegasus/Client/Linkage.h>

#ifdef PEGASUS_NEGOTIATE_AUTHENTICATION
#include <Pegasus/Common/Negotiate.h>
#endif

PEGASUS_NAMESPACE_BEGIN


/** This class owns client authentication
*/
class PEGASUS_CLIENT_LINKAGE ClientAuthenticator
{
public:

    enum AuthType { NONE, BASIC, DIGEST, LOCAL, NEGOTIATE };

    /** Constuctor. */
    ClientAuthenticator();

    /** Destructor. */
    ~ClientAuthenticator();

    /** Build the request authentication header.
    */
    String buildRequestAuthHeader();

    /** Check whether the response header has authentication challenge
        (i.e, WWW-Authenticate header).
    */
    Boolean checkResponseHeaderForChallenge(Array<HTTPHeader> headers);

    /** Save the request message for resending on a challenge.
    */
    void setRequestMessage(Message* message);

    /** Get the request message saved for resending on a challenge while
        retaining memory ownership for the message.
    */
    Message* getRequestMessage();

    /** Get the request message saved for resending on a challenge and
        release memory ownership for the message.
    */
    Message* releaseRequestMessage();

    /** Clear the request buffer and other authentication informations
        from the earlier request.
    */
    void clear();

    /**
        Reset the challenge status for the connection.
    */
    void resetChallengeStatus();

    /** Set the user name
    */
    void setUserName(const String& userName);

    /** Get the user name
    */
    String getUserName();

    /** Set the upasswor
    */
    void setPassword(const String& password);

    /** Set the hostname
    */
    void setHost(const String& host);

    /** Set the authentication type
    */
    void setAuthType(AuthType type);

    /** Get the authentication type
    */
    AuthType getAuthType();

    /**
     * Parse Set-Cookie header from a HTTP response.
     */
    void parseCookie(Array<HTTPHeader> headers);

    String getCookie();

    void clearCookie();

private:

    String _buildLocalAuthResponse();

    String _getFileContent(const String& filePath);

    Boolean _parseAuthHeader(
        const char* authHeader,
        String& authType,
        String& authRealm);

    /** Parse realm name out of realm="<realm>" */
    String _parseBasicRealm(const String &challenge);

    String _getSubStringUptoMarker(
        const char** line,
        char marker);

    AutoPtr<Message> _requestMessage;

    Boolean _challengeReceived;

    String _userName;
    String _password;

    String _localAuthFile;
    String _localAuthFileContent;

    AuthType _authType;

    String _cookie;

#ifdef PEGASUS_NEGOTIATE_AUTHENTICATION
    AutoPtr<NegotiateClientSession> _session;
#endif
};

PEGASUS_NAMESPACE_END

#endif /* Pegasus_ClientAuthenticator_h */
