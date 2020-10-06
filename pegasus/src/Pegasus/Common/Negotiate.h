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
//%////////////////////////////////////////////////////////////////////////////

#ifndef Pegasus_Negotiate_h
#define Pegasus_Negotiate_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/Linkage.h>
#include <Pegasus/Common/String.h>

#include <gssapi/gssapi.h>
#include <gssapi/gssapi_ext.h>

PEGASUS_NAMESPACE_BEGIN

enum NegotiateAuthenticationStatus
{
    /** User is authenticated. */
    NEGOTIATE_SUCCESS,
    /** Authentication is in progress, 401 Unauthorized with a challenge
     *  should be sent. */
    NEGOTIATE_CONTINUE,
    /** User failed to authenticate. */
    NEGOTIATE_FAILED
};

/**
 * This class keeps status of ongoing GSSAPI/Negotiate authentication.
 * It processes Authorization: header from HTTP requests and helps to compose
 * WWW-Authentication header in responses.
 */
class PEGASUS_COMMON_LINKAGE NegotiateServerSession
{
public:
    NegotiateServerSession();
    virtual ~NegotiateServerSession();

    /**
     * Perform authentication of Authorization: Negotiate <data>' header.
     * @param authorization - The <data> from Authorization: header, base64
     * encoded.
     * @param userName - output parameter, authenticated user name or empty
     * string, if the name is not known yet.
     * @param mapToLocalName - value of mapToLocal configuration option
     * @return Status of the authentication process.
     */
    NegotiateAuthenticationStatus authenticate(
            const String &authorization,
            String &userName,
            Boolean mapToLocalName);

    /**
     * Get challenge for next WWW-Authenticate: Negotiate <challenge>.
     * The challenge is already encoded in base64.
     */
    String getChallenge()
    {
        return _challenge;
    }


private:
    /** GSSAPI context */
    gss_ctx_id_t _ctx;
    /** Challenge to be sent in next 401 Unauthorized or 200 OK response.*/
    String   _challenge;

    /**
     * Parse user name out of gss_name_t.
     */
    String parseUserName(gss_name_t client, gss_OID mech_type,
            Boolean mapToLocalName);


};

/**
 * This class keeps status of ongoing GSSAPI/Negotiate authentication.
 * It processes WWW-Authenticate: header from HTTP responses and helps to
 * compose Authroization header in requests.
 */
class PEGASUS_COMMON_LINKAGE NegotiateClientSession
{
public:
    /**
     * Create new client session.
     * @param hostname - Fully qualified domain name of the server, it's needed
     * to compose correct service identity.
     */
    NegotiateClientSession(String hostname);
    virtual ~NegotiateClientSession();

    /**
     * Return <data> for 'WWW-Authenticate: Negotiate <data>' header,
     * already base64 encoded.
     * @return The data for WWW-Authenticate header.
     */
    String buildRequestAuthData();

    /**
     * Parse Authorization: Negotiate <challenge> header.
     * @param challenge - the challenge, base64 encoded.
     */
    void parseChallenge(const String &challenge)
    {
        _challenge = challenge;
    }

private:
    /** GSSAPI context */
    gss_ctx_id_t _ctx;
    /** Data from the last Authorization: Negotiate <challenge> header,
     *  base64 encoded.*/
    String   _challenge;

    /** Remote service name. */
    gss_name_t _service;
};

PEGASUS_NAMESPACE_END

#endif //Pegasus_Negotiate_h
