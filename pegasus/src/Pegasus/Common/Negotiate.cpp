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
#include "Executor.h"
#include "Negotiate.h"
#include "Tracer.h"
#include "Base64.h"

PEGASUS_NAMESPACE_BEGIN

static const char NEGOTIATE_GET_NAME_FAILED_KEY [] =
    "Common.Negotiate NEGOTIATE_GET_NAME_FAILURE";

static const char NEGOTIATE_GET_NAME_FAILED[] =
    "Cannot read user name for Negotiate request.";

static const char NEGOTIATE_GET_NAME_RELEASE_FAILED_KEY [] =
    "Common.Negotiate NEGOTIATE_GET_NAME_RELEASE_FAILURE";

static const char NEGOTIATE_GET_NAME_RELEASE_FAILED[] =
    "Cannot release user name for Negotiate request.";

static const char NEGOTIATE_GET_NAME_SUCCESS_KEY [] =
    "Common.Negotiate NEGOTIATE_GET_NAME_RELEASE_SUCCESS";

static const char NEGOTIATE_GET_NAME_SUCCESS[] =
    "Got user name $0 from Negotiate request.";

static const char NEGOTIATE_SERVICE_NAME[] = "cimom@";

static gss_OID_desc gss_mech_spnego = {
        6,
        (char *)"\x2b\x06\x01\x05\x05\x02"
};

String getNegotiateError(uint32_t major, uint32_t minor)
{
    gss_buffer_desc text;
    uint32_t maj, min;
    uint32_t msg_ctx = 0;

    bool first = true;
    String msg;
    do {
        maj = gss_display_status(&min, major, GSS_C_GSS_CODE, GSS_C_NO_OID,
                &msg_ctx, &text);
        if (maj != GSS_S_COMPLETE)
            return msg;
        if (!first)
            msg.append(", ");
        msg.append((const char*)text.value, text.length);
        first = false;
    } while (msg_ctx != 0);

    do {
        maj = gss_display_status(&min, minor, GSS_C_MECH_CODE, GSS_C_NO_OID,
                &msg_ctx, &text);
        if (maj != GSS_S_COMPLETE)
            return msg;
        if (!first)
            msg.append(", ");
        msg.append((const char*)text.value, text.length);
        first = false;
    } while (msg_ctx != 0);

    return msg;
}

NegotiateServerSession::NegotiateServerSession()
    : _challenge(String::EMPTY)
{
    PEG_METHOD_ENTER(
        TRC_AUTHENTICATION, "NegotiateServerSession::NegotiateServerSession");

    _ctx = GSS_C_NO_CONTEXT;

    PEG_METHOD_EXIT();
}

NegotiateServerSession::~NegotiateServerSession()
{
    PEG_METHOD_ENTER(
        TRC_AUTHENTICATION, "NegotiateServerSession::~NegotiateServerSession");
    uint32_t min;
    gss_delete_sec_context(&min, &_ctx, GSS_C_NO_BUFFER);
    PEG_METHOD_EXIT();
}

NegotiateAuthenticationStatus NegotiateServerSession::authenticate(
            const String &authorization,
            String &userName,
            Boolean mapToLocalName)
{
    PEG_METHOD_ENTER(
        TRC_AUTHENTICATION, "NegotiateServerSession::authenticate");

    // decode the authorization data
    Buffer data;
    data.append((const char*) authorization.getCString(), authorization.size());
    Buffer decodedData = Base64::decode( data );

    // collect GSSAPI input and output arguments
    uint32_t flags = 0, maj, min = 0, unused;
    gss_name_t client = GSS_C_NO_NAME;
    gss_buffer_desc output = GSS_C_EMPTY_BUFFER;
    gss_buffer_desc input = GSS_C_EMPTY_BUFFER;
    gss_OID mech_type;
    input.value = decodedData.getContentPtr();
    input.length = decodedData.size();

    maj = gss_accept_sec_context(
                        &min,                   // minor error code
                        &_ctx,                  // gssapi context
                        GSS_C_NO_CREDENTIAL,    // server's credentials
                        &input,                 // token from WWW-Authenticate:
                        GSS_C_NO_CHANNEL_BINDINGS, // input channel bindings
                        &client,                // out: client's name
                        &mech_type,             // out: authentication type
                        &output,                // out: token for Authorization:
                        &flags,                 // out: flags
                        NULL,                   // out: context lifetime
                        NULL);                  // out: delegated credentials

    userName = parseUserName(client, mech_type, mapToLocalName);
    gss_release_name(&unused, &client);

    NegotiateAuthenticationStatus status;

    if (GSS_ERROR(maj))
    {
        PEG_TRACE((TRC_AUTHENTICATION, Tracer::LEVEL3,
                "NegotiateServerSession::authenticate:"\
                " GSSAPI failed, reseting authentication."));

        // reset the GSSAPI context, in case the user tries it again
        gss_delete_sec_context(&unused, &_ctx, GSS_C_NO_BUFFER);
        _ctx = GSS_C_NO_CONTEXT;
        // no challenge in the next HTTP response
        _challenge = String::EMPTY;

        // TODO: log the gssapi error message
        status = NEGOTIATE_FAILED;
    }
    else
    {
        // no error so far
        // remember the challenge to send (already base64-encoded)
        if (output.length > 0)
        {
            Buffer outputBuf((const char*) output.value, output.length);
            Buffer encodedBuf = Base64::encode(outputBuf);
            _challenge.assign(encodedBuf.getData(), encodedBuf.size());
            gss_release_buffer(&min, &output);
        }
        else
        {
            // no challenge in the next HTTP response
            _challenge = String::EMPTY;
        }

        if (maj == GSS_S_COMPLETE)
        {
            PEG_TRACE((TRC_AUTHENTICATION, Tracer::LEVEL3,
                    "NegotiateServerSession::authenticate:"\
                    " GSSAPI finished successfully."));
            status = NEGOTIATE_SUCCESS;
        }
        else
        {
            PEG_TRACE((TRC_AUTHENTICATION, Tracer::LEVEL3,
                    "NegotiateServerSession::authenticate:"\
                    " GSSAPI continues."));
            status = NEGOTIATE_CONTINUE;
        }
    }

    // GSSAPI does not do any access control, do it now
    if (status == NEGOTIATE_SUCCESS)
    {
        int ret = Executor::validateUser((const char*) userName.getCString());
        if (ret != 0)
        {
            PEG_TRACE((TRC_AUTHENTICATION, Tracer::LEVEL3,
                    "NegotiateServerSession::authenticate:"\
                    " authorization failed."));

            // reset the GSSAPI context, in case the user tries it again
            gss_delete_sec_context(&unused, &_ctx, GSS_C_NO_BUFFER);
            _ctx = GSS_C_NO_CONTEXT;
            // no challenge in the next HTTP response
            _challenge = String::EMPTY;
            status = NEGOTIATE_FAILED;
            // TODO: log access error
        }
    }

    PEG_METHOD_EXIT();
    return status;
}

String NegotiateServerSession::parseUserName(gss_name_t client,
        gss_OID mech_type, Boolean mapToLocalName)
{
    PEG_METHOD_ENTER(
         TRC_AUTHENTICATION, "NegotiateServerSession::parseUserName");

    uint32_t maj, min;
    gss_buffer_desc output = GSS_C_EMPTY_BUFFER;
    gss_OID oid;

    if (mapToLocalName)
        maj = gss_localname(&min, client, mech_type, &output);
    else
        maj = gss_display_name(&min, client, &output, &oid);
    if (maj != GSS_S_COMPLETE)
    {
        Logger::put_l(Logger::STANDARD_LOG, System::CIMSERVER,
            Logger::INFORMATION,
            MessageLoaderParms(
                    NEGOTIATE_GET_NAME_FAILED_KEY,
                    NEGOTIATE_GET_NAME_FAILED));
        PEG_METHOD_EXIT();
        return String::EMPTY;
    }

    String userName((const char*) output.value, (unsigned int) output.length);

    Logger::put_l(Logger::STANDARD_LOG, System::CIMSERVER,
        Logger::TRACE,
        MessageLoaderParms(
                NEGOTIATE_GET_NAME_SUCCESS_KEY,
                NEGOTIATE_GET_NAME_SUCCESS, userName));

    maj = gss_release_buffer(&min, &output);
    if (maj != GSS_S_COMPLETE)
    {
        Logger::put_l(Logger::STANDARD_LOG, System::CIMSERVER,
            Logger::INFORMATION,
            MessageLoaderParms(
                    NEGOTIATE_GET_NAME_RELEASE_FAILED_KEY,
                    NEGOTIATE_GET_NAME_RELEASE_FAILED));
    }

    PEG_METHOD_EXIT();
    return userName;
}






NegotiateClientSession::NegotiateClientSession(String hostname)
    : _challenge(String::EMPTY)
{
    PEG_METHOD_ENTER(
        TRC_AUTHENTICATION, "NegotiateClientSession::NegotiateClientSession");

    _ctx = GSS_C_NO_CONTEXT;

    // translate hostname to gss_name_t (cimom@<hostname>)
    gss_buffer_desc input;
    Buffer buf(NEGOTIATE_SERVICE_NAME, sizeof(NEGOTIATE_SERVICE_NAME)-1);
    buf.append(hostname.getCString(), hostname.size()+1);
    input.value = buf.getContentPtr();
    input.length = buf.size();

    uint32_t min=0, maj;
    maj = gss_import_name(&min, &input, GSS_C_NT_HOSTBASED_SERVICE, &_service);
    if (GSS_ERROR(maj))
    {
        String msg = getNegotiateError(maj, min);
        MessageLoaderParms parms(
            "Common.Negotiate."
                "NEGOTIATE_CLIENT_SESSION_FAILED_TO_INITIALIZE",
            "Client authentication handler for Negotiate failed to "
                "initialize properly: %s.", (const char *) msg.getCString());
        Logger::put_l(Logger::ERROR_LOG, System::CIMSERVER, Logger::SEVERE,
            parms);
        throw Exception(parms);
    }
    PEG_METHOD_EXIT();
}

NegotiateClientSession::~NegotiateClientSession()
{
    PEG_METHOD_ENTER(
        TRC_AUTHENTICATION, "NegotiateClientSession::~NegotiateClientSession");

    uint32_t min;
    gss_delete_sec_context(&min, &_ctx, GSS_C_NO_BUFFER);
    gss_release_name(&min, &_service);

    PEG_METHOD_EXIT();
}

/**
 * Return <data> for 'WWW-Authenticate: Negotiate <data>' header,
 * already base64 encoded.
 * @return The data for WWW-Authenticate header.
 */
String NegotiateClientSession::buildRequestAuthData()
{
    PEG_METHOD_ENTER(
        TRC_AUTHENTICATION, "NegotiateClientSession::buildRequestAuthData");

    // decode previous challenge from base64

    gss_buffer_desc input = GSS_C_EMPTY_BUFFER;
    if (_challenge.size() > 0)
    {
        Buffer data;
        data.append((const char*) _challenge.getCString(), _challenge.size());
        Buffer decodedData = Base64::decode( data );
        input.value = decodedData.getContentPtr();
        input.length = decodedData.size();
    }

    // collect GSSAPI input and output arguments
    uint32_t flags = 0, maj, min = 0;
    gss_buffer_desc output = GSS_C_EMPTY_BUFFER;

    maj = gss_init_sec_context(
            &min,                       // minor error code
            GSS_C_NO_CREDENTIAL,        // client credentials
            &_ctx,                      // gssapi context
            _service,                   // remote service name
            &gss_mech_spnego,           // requested authentication mechanism
            GSS_C_REPLAY_FLAG,          // requested flags
            0,                          // requested token lifetime
            GSS_C_NO_CHANNEL_BINDINGS,  // input channel bindings
            &input,                     // token from 401 Unauthorized response
            NULL,                       // out: actual authentication mechanism
            &output,                    // out: token for WWW-Authenticate:
            &flags,                     // out: output flags
            NULL);                      // out: actual token lifetime

    String authentication;
    if (GSS_ERROR(maj))
    {
        // reset the GSSAPI context and challenge
        _challenge = String::EMPTY;

        String msg = getNegotiateError(maj, min);
        PEG_TRACE((TRC_AUTHENTICATION, Tracer::LEVEL2,
                "NegotiateClientSession::buildRequestAuthData GSSAPI error %s",
                (const char *) msg.getCString()));
        gss_delete_sec_context(&min, &_ctx, GSS_C_NO_BUFFER);
        _ctx = GSS_C_NO_CONTEXT;
    }
    else
    {
        // no error so far
        if (output.length > 0)
        {
            Buffer outputBuf((const char*) output.value, output.length);
            Buffer encodedBuf = Base64::encode(outputBuf);
            authentication.assign(encodedBuf.getData(), encodedBuf.size());
            gss_release_buffer(&min, &output);
            PEG_TRACE((TRC_AUTHENTICATION, Tracer::LEVEL2,
                    "NegotiateClientSession::buildRequestAuthData"
                    " sending response %s\n",
                    (const char*) authentication.getCString()));
        }
    }

    PEG_METHOD_EXIT();
    return authentication;
}

PEGASUS_NAMESPACE_END


