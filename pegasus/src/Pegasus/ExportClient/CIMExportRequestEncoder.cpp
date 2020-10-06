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
#include <Pegasus/Common/XmlWriter.h>
#include <Pegasus/Common/HTTPMessage.h>
#include <Pegasus/Common/AcceptLanguageList.h>
#include <Pegasus/Common/ContentLanguageList.h>
#include "CIMExportRequestEncoder.h"

PEGASUS_USING_STD;

PEGASUS_NAMESPACE_BEGIN

CIMExportRequestEncoder::CIMExportRequestEncoder(
    MessageQueue* outputQueue,
    const String& hostName,
    ClientAuthenticator* authenticator)
    : MessageQueue(PEGASUS_QUEUENAME_EXPORTREQENCODER),
      _outputQueue(outputQueue),
      _hostName(hostName.getCString()),
      _authenticator(authenticator)
{
    PEG_METHOD_ENTER(TRC_EXPORT_CLIENT,
        "CIMExportRequestEncoder::CIMExportRequestEncoder()");
    PEG_METHOD_EXIT();
}

CIMExportRequestEncoder::~CIMExportRequestEncoder()
{
    PEG_METHOD_ENTER(TRC_EXPORT_CLIENT,
        "CIMExportRequestEncoder::~CIMExportRequestEncoder()");
    _authenticator.release();
    PEG_METHOD_EXIT();
}

void CIMExportRequestEncoder::handleEnqueue()
{
    PEG_METHOD_ENTER(TRC_EXPORT_CLIENT,
        "CIMExportRequestEncoder::handleEnqueue()");
    Message* message = dequeue();

    PEGASUS_ASSERT(message != 0);

    _authenticator->setRequestMessage(message);

    switch (message->getType())
    {
        case CIM_EXPORT_INDICATION_REQUEST_MESSAGE:
            _encodeExportIndicationRequest(
                (CIMExportIndicationRequestMessage*)message);
            break;
        default:
            PEGASUS_UNREACHABLE(PEGASUS_ASSERT(0);)
            break;
    }

    //ATTN: Do not delete the message here.
    //
    // ClientAuthenticator needs this message for resending the request on
    // authentication challenge from the server. The message is deleted in
    // the decoder after receiving the valid response from the server.
    //
    //delete message;
    PEG_METHOD_EXIT();
}

void CIMExportRequestEncoder::_encodeExportIndicationRequest(
    CIMExportIndicationRequestMessage* message)
{
    PEG_METHOD_ENTER(TRC_EXPORT_CLIENT,
        "CIMExportRequestEncoder::_encodeExportIndicationRequest()");
    Buffer params;

    XmlWriter::appendInstanceEParameter(
        params, "NewIndication", message->indicationInstance);

    // Note:  Accept-Language will not be set in the request
    // We will accept the default language of the export server.
    Buffer buffer = XmlWriter::formatSimpleEMethodReqMessage(
        message->destinationPath.getCString(),
        _hostName,
        CIMName("ExportIndication"),
        message->messageId,
        message->getHttpMethod(),
        _authenticator->buildRequestAuthHeader(),
        AcceptLanguageList(),
        ((ContentLanguageListContainer)message->operationContext.get(
            ContentLanguageListContainer::NAME)).getLanguages(),
        params);

    HTTPMessage* httpMessage = new HTTPMessage(buffer);
    PEG_TRACE_CSTRING(TRC_XML_IO, Tracer::LEVEL4,
        httpMessage->message.getData());

    _outputQueue->enqueue(httpMessage);
    PEG_METHOD_EXIT();
}

PEGASUS_NAMESPACE_END
