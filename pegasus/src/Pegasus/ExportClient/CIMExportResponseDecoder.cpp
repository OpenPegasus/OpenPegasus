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
#include <iostream>
#include <Pegasus/Common/Constants.h>
#include <Pegasus/Common/XmlParser.h>
#include <Pegasus/Common/XmlReader.h>
#include <Pegasus/Common/System.h>
#include <Pegasus/Common/XmlWriter.h>
#include <Pegasus/Common/HTTPMessage.h>
#include <Pegasus/Common/CIMMessage.h>
#include <Pegasus/Common/Exception.h>
#include "CIMExportResponseDecoder.h"

// l10n
#include <Pegasus/Common/MessageLoader.h>

PEGASUS_USING_STD;

PEGASUS_NAMESPACE_BEGIN

CIMExportResponseDecoder::CIMExportResponseDecoder(
    MessageQueue* outputQueue,
    MessageQueue* encoderQueue,
    ClientAuthenticator* authenticator)
    : MessageQueue(PEGASUS_QUEUENAME_EXPORTRESPDECODER),
      _outputQueue(outputQueue),
      _encoderQueue(encoderQueue),
      _authenticator(authenticator)
{
    PEG_METHOD_ENTER(TRC_EXPORT_CLIENT,
        "CIMExportResponseDecoder::CIMExportResponseDecoder()");
    PEG_METHOD_EXIT();
}

CIMExportResponseDecoder::~CIMExportResponseDecoder()
{
    PEG_METHOD_ENTER(TRC_EXPORT_CLIENT,
        "CIMExportResponseDecoder::~CIMExportResponseDecoder()");
    _outputQueue.release();
    _encoderQueue.release();
    _authenticator.release();
    PEG_METHOD_EXIT();
}

void CIMExportResponseDecoder::setEncoderQueue(MessageQueue* encoderQueue)
{
    PEG_METHOD_ENTER(TRC_EXPORT_CLIENT,
        "CIMExportResponseDecoder::setEncoderQueue()");
    _encoderQueue.release();
    _encoderQueue.reset(encoderQueue);
    PEG_METHOD_EXIT();
}

void CIMExportResponseDecoder::handleEnqueue()
{
    PEG_METHOD_ENTER(TRC_EXPORT_CLIENT,
        "CIMExportResponseDecoder::handleEnqueue()");
    Message* message = dequeue();

    PEGASUS_ASSERT(message != 0);

    switch (message->getType())
    {
        case HTTP_MESSAGE:
        {
            HTTPMessage* httpMessage = (HTTPMessage*)message;
            _handleHTTPMessage(httpMessage);
            break;
        }

        default:
            PEGASUS_UNREACHABLE(PEGASUS_ASSERT(0);)
            break;
    }

    delete message;
    PEG_METHOD_EXIT();
}

void CIMExportResponseDecoder::_handleHTTPMessage(HTTPMessage* httpMessage)
{
    PEG_METHOD_ENTER(TRC_EXPORT_CLIENT,
        "CIMExportResponseDecoder::_handleHTTPMessage()");

    //
    //  Parse the HTTP CIM Export response message
    //
    ClientExceptionMessage* exceptionMessage;
    char* content;
    Array<HTTPHeader> headers;
    Uint32 contentLength;
    Uint32 statusCode;
    String reasonPhrase;
    Boolean cimReconnect;
    Boolean valid;
    HTTPExportResponseDecoder::parseHTTPHeaders(httpMessage, exceptionMessage,
        headers, contentLength, statusCode, reasonPhrase, cimReconnect, valid);

    //
    //  Return exception on any parse errors from the HTTP export response
    //  message
    //
    if (!valid)
    {
        _outputQueue->enqueue(exceptionMessage);
        PEG_METHOD_EXIT();
        return;
    }

    try
    {
        if (_authenticator->checkResponseHeaderForChallenge(headers))
        {
            //
            // Get the original request, put that in the encoder's queue for
            // re-sending with authentication challenge response.
            //

            Message* reqMessage = _authenticator->releaseRequestMessage();

            if (cimReconnect == true)
            {
                reqMessage->setCloseConnect(cimReconnect);
                _outputQueue->enqueue(reqMessage);
            }
            else
            {
                _encoderQueue->enqueue(reqMessage);
            }

            PEG_METHOD_EXIT();
            return;
        }
        else
        {
            //
            // Received a valid/error response from the server.
            // We do not need the original request message anymore, hence
            // delete the request message by getting the handle from the
            // ClientAuthenticator.
            //
            Message* reqMessage = _authenticator->releaseRequestMessage();
            delete reqMessage;
        }
    }
    catch(InvalidAuthHeader& e)
    {
        AutoPtr<CIMClientMalformedHTTPException> malformedHTTPException(
            new CIMClientMalformedHTTPException(e.getMessage()));
        AutoPtr<ClientExceptionMessage> response(
            new ClientExceptionMessage(malformedHTTPException.get()));

        malformedHTTPException.release();

        response->setCloseConnect(cimReconnect);
        _outputQueue->enqueue(response.release());
        PEG_METHOD_EXIT();
        return;
    }

    //
    //  Validate the HTTP headers in the export response message
    //
    HTTPExportResponseDecoder::validateHTTPHeaders(httpMessage, headers,
        contentLength, statusCode, cimReconnect, reasonPhrase, content,
        exceptionMessage, valid);

    //
    //  Return exception on any errors in the HTTP headers in the export
    //  response message
    //
    if (!valid)
    {
        _outputQueue->enqueue(exceptionMessage);
        PEG_METHOD_EXIT();
        return;
    }

    //
    //  Decode the export response message
    //
    Message* responseMessage;
    HTTPExportResponseDecoder::decodeExportResponse(content, cimReconnect,
        responseMessage);
    _outputQueue->enqueue(responseMessage);

    PEG_METHOD_EXIT();
}

PEGASUS_NAMESPACE_END
