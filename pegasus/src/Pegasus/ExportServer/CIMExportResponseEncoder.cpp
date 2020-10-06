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
#include <cctype>
#include <cstdio>
#include <Pegasus/Common/Constants.h>
#include <Pegasus/Common/XmlParser.h>
#include <Pegasus/Common/XmlReader.h>
#include <Pegasus/Common/XmlWriter.h>
#include <Pegasus/Common/HTTPMessage.h>
#include <Pegasus/Common/Logger.h>
#include <Pegasus/Common/Tracer.h>
#include <Pegasus/Common/ContentLanguageList.h>
#include "CIMExportResponseEncoder.h"

PEGASUS_USING_STD;

PEGASUS_NAMESPACE_BEGIN

CIMExportResponseEncoder::CIMExportResponseEncoder()
    : Base(PEGASUS_QUEUENAME_EXPORTRESPENCODER)
{
}

CIMExportResponseEncoder::~CIMExportResponseEncoder()
{
}

void CIMExportResponseEncoder::sendResponse(
    Uint32 queueId,
    Buffer& message,
    Boolean closeConnect)
{
    MessageQueue* queue = MessageQueue::lookup(queueId);

    if (queue)
    {
        HTTPMessage* httpMessage = new HTTPMessage(message);

        httpMessage->setCloseConnect(closeConnect);

        queue->enqueue(httpMessage);
    }
    else
    {
        PEG_TRACE((TRC_DISCARDED_DATA, Tracer::LEVEL1,
            "Invalid queueId = %i, response not sent.", queueId));
    }
}

void CIMExportResponseEncoder::sendEMethodError(
    Uint32 queueId,
    HttpMethod httpMethod,
    const String& messageId,
    const String& eMethodName,
    const CIMException& cimException,
    Boolean closeConnect)
{
    Buffer message;
    message = XmlWriter::formatSimpleEMethodErrorRspMessage(
        eMethodName,
        messageId,
        httpMethod,
        cimException);

    sendResponse(queueId, message,closeConnect);
}

void CIMExportResponseEncoder::sendEMethodError(
    CIMResponseMessage* response,
    const String& cimMethodName,
    Boolean closeConnect)
{
    Uint32 queueId = response->queueIds.top();
    response->queueIds.pop();

    sendEMethodError(
        queueId,
        response->getHttpMethod(),
        response->messageId,
        cimMethodName,
        response->cimException,
        closeConnect);
}

void CIMExportResponseEncoder::handleEnqueue(Message *message)
{
    PEGASUS_ASSERT(message != 0);

    switch (message->getType())
    {
        case CIM_EXPORT_INDICATION_RESPONSE_MESSAGE:
            encodeExportIndicationResponse(
                (CIMExportIndicationResponseMessage*)message);
            break;

        default:
            PEGASUS_UNREACHABLE(PEGASUS_ASSERT(0);)
            break;
    }

    delete message;
}


void CIMExportResponseEncoder::handleEnqueue()
{
    Message* message = dequeue();
    if (message)
        handleEnqueue(message);
}

void CIMExportResponseEncoder::encodeExportIndicationResponse(
    CIMExportIndicationResponseMessage* response)
{
    Boolean closeConnect = response->getCloseConnect();
    PEG_TRACE((
        TRC_HTTP,
        Tracer::LEVEL4,
        "CIMExportResponseEncoder::handleEnqueue() - "
            "response>getCloseConnect() returned %d",
        response->getCloseConnect()));

    if (response->cimException.getCode() != CIM_ERR_SUCCESS)
    {
        sendEMethodError(response, "ExportIndication",closeConnect);
        return;
    }


    Buffer body;

    // Note: Content-Language will not be set in the response.
    // Export responses are sent in the default language of the
    // ExportServer.
    Buffer message = XmlWriter::formatSimpleEMethodRspMessage(
       CIMName ("ExportIndication"), response->messageId,
       response->getHttpMethod(),
       ContentLanguageList(),
       body);

    sendResponse(response->queueIds.top(), message,closeConnect);
}

PEGASUS_NAMESPACE_END
