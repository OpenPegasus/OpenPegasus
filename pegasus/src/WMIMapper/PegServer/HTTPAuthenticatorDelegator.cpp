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

#include <Pegasus/Common/Constants.h>
#include <Pegasus/Common/HTTPAcceptor.h>
#include <Pegasus/Common/HTTPConnection.h>
#include <Pegasus/Common/HTTPMessage.h>
#include <Pegasus/Common/XmlWriter.h>
#include "HTTPAuthenticatorDelegator.h"

PEGASUS_USING_STD;

PEGASUS_NAMESPACE_BEGIN


HTTPAuthenticatorDelegator::HTTPAuthenticatorDelegator(
    Uint32 cimOperationMessageQueueId,
    Uint32 cimExportMessageQueueId,
    CIMRepository* repository)
    : Base(PEGASUS_QUEUENAME_HTTPAUTHDELEGATOR, MessageQueue::getNextQueueId()),
      _cimOperationMessageQueueId(cimOperationMessageQueueId),
      _cimExportMessageQueueId(cimExportMessageQueueId),
      _wsmanOperationMessageQueueId(PEG_NOT_FOUND),
      _repository(repository)
{
    PEG_METHOD_ENTER(TRC_HTTP,
        "HTTPAuthenticatorDelegator::HTTPAuthenticatorDelegator");

    _authenticationManager.reset(new AuthenticationManager());

    PEG_METHOD_EXIT();
}

HTTPAuthenticatorDelegator::~HTTPAuthenticatorDelegator()
{
    PEG_METHOD_ENTER(TRC_HTTP,
        "HTTPAuthenticatorDelegator::~HTTPAuthenticatorDelegator");


    PEG_METHOD_EXIT();
}

void HTTPAuthenticatorDelegator::enqueue(Message* message)
{
    handleEnqueue(message);
}

void HTTPAuthenticatorDelegator::_sendResponse(
    Uint32 queueId,
    Buffer& message,
    Boolean closeConnect)
{
    PEG_METHOD_ENTER(TRC_HTTP,
        "HTTPAuthenticatorDelegator::_sendResponse");

    MessageQueue* queue = MessageQueue::lookup(queueId);

    if (queue)
    {
        AutoPtr<HTTPMessage> httpMessage(new HTTPMessage(message));
        httpMessage->dest = queue->getQueueId();

        queue->enqueue(httpMessage.release());
    }

    PEG_METHOD_EXIT();
}


void HTTPAuthenticatorDelegator::_sendChallenge(
    Uint32 queueId,
    const String& authResponse,
    Boolean closeConnect)
{
    PEG_METHOD_ENTER(TRC_HTTP,
        "HTTPAuthenticatorDelegator::_sendChallenge");

    //
    // build unauthorized (401) response message
    //

    Buffer message;
    XmlWriter::appendUnauthorizedResponseHeader(message, authResponse);

    _sendResponse(queueId, message, false);

    PEG_METHOD_EXIT();
}

void HTTPAuthenticatorDelegator::_sendHttpError(
    Uint32 queueId,
    const String& status,
    const String& cimError,
    const String& pegasusError,
    Boolean closeConnect)
{
    PEG_METHOD_ENTER(TRC_HTTP,
        "HTTPAuthenticatorDelegator::_sendHttpError");

    //
    // build error response message
    //

    Buffer message;
    message = XmlWriter::formatHttpErrorRspMessage(
        status,
        cimError,
        pegasusError);

    _sendResponse(queueId, message, false);

    PEG_METHOD_EXIT();
}


void HTTPAuthenticatorDelegator::handleEnqueue(Message *message)
{
    PEG_METHOD_ENTER(TRC_HTTP,
        "HTTPAuthenticatorDelegator::handleEnqueue");

    if (!message)
    {
        PEG_METHOD_EXIT();
        return;
    }

    // Flag indicating whether the message should be deleted after handling.
    // This should be set to false by handleHTTPMessage when the message is
    // passed as is to another queue.
    Boolean deleteMessage = true;

    if (message->getType() == HTTP_MESSAGE)
    {
        handleHTTPMessage((HTTPMessage*)message, deleteMessage);
    }

    if (deleteMessage)
    {
        delete message;
    }

    PEG_METHOD_EXIT();
}

void HTTPAuthenticatorDelegator::handleEnqueue()
{
    PEG_METHOD_ENTER(TRC_HTTP,
        "HTTPAuthenticatorDelegator::handleEnqueue");

    Message* message = dequeue();
    if(message)
       handleEnqueue(message);

    PEG_METHOD_EXIT();
}

void HTTPAuthenticatorDelegator::handleHTTPMessage(
    HTTPMessage* httpMessage,
    Boolean & deleteMessage)
{
    PEG_METHOD_ENTER(TRC_HTTP,
        "HTTPAuthenticatorDelegator::handleHTTPMessage");

    Boolean authenticated = false;
    deleteMessage = true;

    // ATTN-RK-P3-20020408: This check probably shouldn't be necessary, but
    // we're getting an empty message when the client closes the connection
    if (httpMessage->message.size() == 0)
    {
        // The message is empty; just drop it
        PEG_METHOD_EXIT();
        return;
    }

    //
    // get the configured authentication flag
    //
    // WMI MAPPER SPECIFIC: AUTHENTICATION ALWAYS ENABLED:
    Boolean enableAuthentication = true;
    /* NORMAL METHOD OF LOOKING UP AUTHENTICATION CONFIGURATION:
    Boolean enableAuthentication = ConfigManager::parseBooleanValue(
        ConfigManager::getInstance()->getCurrentValue("enableAuthentication"));
    */

    //
    // Save queueId:
    //
    Uint32 queueId = httpMessage->queueId;

    //
    // Parse the HTTP message:
    //
    String startLine;
    Array<HTTPHeader> headers;
    Uint32 contentLength;

    httpMessage->parse(startLine, headers, contentLength);

    //
    // Parse the request line:
    //
    String methodName;
    String requestUri;
    String httpVersion;
    HttpMethod httpMethod = HTTP_METHOD__POST;

    HTTPMessage::parseRequestLine(
        startLine, methodName, requestUri, httpVersion);

    //
    //  Set HTTP method for the request
    //
    if (methodName == "M-POST")
    {
        httpMethod = HTTP_METHOD_M_POST;
    }

    if (methodName != "M-POST" && methodName != "POST")
    {
        // Only POST and M-POST are implemented by this server
        _sendHttpError(queueId,
                       HTTP_STATUS_NOTIMPLEMENTED);
    }
    else if ((httpMethod == HTTP_METHOD_M_POST) &&
             (httpVersion == "HTTP/1.0"))
    {
        //
        //  M-POST method is not valid with version 1.0
        //
        _sendHttpError(queueId,
                       HTTP_STATUS_BADREQUEST);
    }
    else
    {
        //
        // Process M-POST and POST messages:
        //
        PEG_TRACE_CSTRING(
            TRC_HTTP,
            Tracer::LEVEL3,
            "HTTPAuthenticatorDelegator - M-POST/POST processing start");

        //
        // Search for Authorization header:
        //
        String authorization;

        //
        // Do not require authentication for indication delivery
        //
        const char* cimExport;
        if (enableAuthentication &&
            HTTPMessage::lookupHeader(
                headers, "CIMExport", cimExport, true))
        {
            enableAuthentication = false;
        }

        if ( HTTPMessage::lookupHeader(
             headers, "PegasusAuthorization", authorization, false) &&
             enableAuthentication
           )
        {
            try
            {
                //
                // Do pegasus/local authentication
                //
                authenticated =
                    _authenticationManager->performPegasusAuthentication(
                        authorization,
                        httpMessage->authInfo);

                if (!authenticated)
                {
                    String authChallenge;
                    String authResp;

                    authResp =
                        _authenticationManager->getPegasusAuthResponseHeader(
                            authorization,
                            httpMessage->authInfo);

                    if (!String::equal(authResp, String::EMPTY))
                    {
                        _sendChallenge(queueId, authResp, false);
                    }
                    else
                    {
                        _sendHttpError(queueId,
                                       HTTP_STATUS_BADREQUEST,
                                       String::EMPTY,
                                       "Authorization header error");
                    }

                    PEG_METHOD_EXIT();
                    return;
                }
            }
            catch (CannotOpenFile &cof)
            {
                _sendHttpError(queueId,
                               HTTP_STATUS_INTERNALSERVERERROR);
                PEG_METHOD_EXIT();
                return;

            }
        }

        if ( HTTPMessage::lookupHeader(
             headers, "Authorization", authorization, false) &&
             enableAuthentication
           )
        {
            //
            // Do http authentication if not authenticated already
            //
            if (!authenticated)
            {
                authenticated =
                    _authenticationManager->performHttpAuthentication(
                        authorization,
                        httpMessage->authInfo);

                if (!authenticated)
                {
                    //ATTN: the number of challenges get sent for a
                    //      request on a connection can be pre-set.
                    String authResp =
                        _authenticationManager->getHttpAuthResponseHeader();

                    if (!String::equal(authResp, String::EMPTY))
                    {
                        _sendChallenge(queueId, authResp, false);
                    }
                    else
                    {
                        _sendHttpError(queueId,
                                       HTTP_STATUS_BADREQUEST,
                                       String::EMPTY,
                                       "Authorization header error");
                    }

                    PEG_METHOD_EXIT();
                    return;
                }
            }  // first not authenticated check
        }  // "Authorization" header check


        if ( authenticated || !enableAuthentication )
        {
            //
            // Search for "CIMOperation" header:
            //
            const char* cimOperation;

            if (HTTPMessage::lookupHeader(
                    headers, "CIMOperation", cimOperation, true))
            {
                PEG_TRACE((
                    TRC_HTTP,
                    Tracer::LEVEL3,
                    "HTTPAuthenticatorDelegator - CIMOperation: %s",
                    cimOperation));

                MessageQueue* queue =
                    MessageQueue::lookup(_cimOperationMessageQueueId);

                if (queue)
                {
                   httpMessage->dest = queue->getQueueId();

                   try
                     {
                       queue->enqueue(httpMessage);
                     }
                   catch(exception & e)
                     {
                       delete httpMessage;
                       _sendHttpError(queueId,
                                      HTTP_STATUS_REQUEST_TOO_LARGE);
                       PEG_METHOD_EXIT();
                       deleteMessage = false;
                       return;
                     }
                 deleteMessage = false;
                }
            }
            else if (HTTPMessage::lookupHeader(
                         headers, "CIMExport", cimOperation, true))
            {
                PEG_TRACE((
                    TRC_HTTP,
                    Tracer::LEVEL3,
                    "HTTPAuthenticatorDelegator - CIMExport: $0",
                    cimOperation));

                MessageQueue* queue =
                    MessageQueue::lookup(_cimExportMessageQueueId);

                if (queue)
                {
                    httpMessage->dest = queue->getQueueId();

                    queue->enqueue(httpMessage);
                    deleteMessage = false;
                }
            }
            else
            {
                // We don't recognize this request message type

                // The Specification for CIM Operations over HTTP reads:
                //
                //     3.3.4. CIMOperation
                //
                //     If a CIM Server receives a CIM Operation request without
                //     this [CIMOperation] header, it MUST NOT process it as if
                //     it were a CIM Operation Request.  The status code
                //     returned by the CIM Server in response to such a request
                //     is outside of the scope of this specification.
                //
                //     3.3.5. CIMExport
                //
                //     If a CIM Listener receives a CIM Export request without
                //     this [CIMExport] header, it MUST NOT process it.  The
                //     status code returned by the CIM Listener in response to
                //     such a request is outside of the scope of this
                //     specification.
                //
                // The author has chosen to send a 400 Bad Request error, but
                // without the CIMError header since this request must not be
                // processed as a CIM request.

                _sendHttpError(queueId,
                               HTTP_STATUS_BADREQUEST);
                PEG_METHOD_EXIT();
                return;
            } // bad request
        } // authenticated and enableAuthentication check
        else
        {  // client not authenticated; send challenge
            String authResp =
                _authenticationManager->getHttpAuthResponseHeader();

            if (!String::equal(authResp, String::EMPTY))
            {
                _sendChallenge(queueId, authResp, false);
            }
            else
            {
                _sendHttpError(queueId,
                               HTTP_STATUS_BADREQUEST,
                               String::EMPTY,
                               "Authorization header error");
            }
        }
    } // M-POST and POST processing

    PEG_METHOD_EXIT();
}

PEGASUS_NAMESPACE_END
