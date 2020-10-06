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
#include <Pegasus/Common/Constants.h>
#include <Pegasus/Common/HTTPMessage.h>
#include <Pegasus/Common/AcceptLanguageList.h>
#include <Pegasus/Common/ContentLanguageList.h>
#include "WSMANExportRequestEncoder.h"
#include <Clients/wbemexec/HttpConstants.h>
#include <Pegasus/WsmServer/WsmWriter.h>

PEGASUS_USING_STD;

PEGASUS_NAMESPACE_BEGIN

WSMANExportRequestEncoder::WSMANExportRequestEncoder(
    MessageQueue* outputQueue,
    const String& hostName,
    const String& portId,
    ClientAuthenticator* authenticator)
    : MessageQueue(PEGASUS_QUEUENAME_WSMANEXPORTREQENCODER),
     _outputQueue(outputQueue),
     _hostName(hostName.getCString()),
     _portNumber(portId.getCString()),
     _authenticator(authenticator)
{
    PEG_METHOD_ENTER(TRC_EXPORT_CLIENT,
        "WSMANExportRequestEncoder::WSMANExportRequestEncoder()");
    PEG_METHOD_EXIT();
}

WSMANExportRequestEncoder::~WSMANExportRequestEncoder()
{
    PEG_METHOD_ENTER(TRC_EXPORT_CLIENT,
        "WSMANExportRequestEncoder::~WSMANExportRequestEncoder()");
    _authenticator.release();
    PEG_METHOD_EXIT();   
}

void WSMANExportRequestEncoder::handleEnqueue()
{
    PEG_METHOD_ENTER(TRC_EXPORT_CLIENT,
        "WSMANExportRequestEncoder::handleEnqueue()");
    Message* message = dequeue();

    PEGASUS_ASSERT(message != 0);

    _authenticator->setRequestMessage(message);

    switch (message->getType())
    {
        case WSMAN_EXPORT_INDICATION_REQUEST_MESSAGE:
            _encodeExportIndicationRequest(
                (WsExportIndicationRequest*)message);
            break;
        default:
            PEGASUS_UNREACHABLE(PEGASUS_ASSERT(0);)
            break;
    }

    PEG_METHOD_EXIT();
}
void WSMANExportRequestEncoder::_encodeWSMANIndication(
    WsExportIndicationRequest* message, Buffer &out)
{
    String action = WSM_ACTION_WSMAN_EVENT;
    String hostname = System::getFullyQualifiedHostName();
    String replyTo = "http://"+hostname+":5988";
    WsmWriter::appendSoapEnvelopeStart(out,message->contentLanguages);

    WsmWriter::appendSoapHeaderStart(out);
    if(_deliveryMode == Push)
    {
        WsmWriter::appendSoapHeader(
            out,
            action,
            message->messageId,
            "",
            message->destination);
    }
    else if(_deliveryMode == PushWithAck)
    {
        WsmWriter::appendSoapHeader(
            out,
            action,
            message->messageId,
            "",
            message->destination,
            replyTo,
            true);
    }
    WsmWriter::appendSoapHeaderEnd(out);
    WsmWriter::appendSoapBodyStart(out);
    WsmWriter::appendInstanceElement(
        out,
        message->url,
        message->IndicationInstance,
        PEGASUS_INSTANCE_NS,
        false);
    WsmWriter::appendSoapBodyEnd(out);
    WsmWriter::appendSoapEnvelopeEnd(out);
}
void WSMANExportRequestEncoder::_encodeExportIndicationRequest(
    WsExportIndicationRequest* message)
{
    PEG_METHOD_ENTER(TRC_EXPORT_CLIENT,
        "WSMANExportRequestEncoder::_encodeExportIndicationRequest()");

    Buffer out,buf;

    _encodeWSMANIndication(message, out);

    Buffer contentCopy,HttpHeader;

    contentCopy <<out;

    XmlParser parser ((char*) contentCopy.getData ());

    buf = WsmWriter::appendHTTPRequestHeader(
        parser,
        _hostName+":"+_portNumber,
        false,
        true,
        out,
        HttpHeader,
        message->url);

    //Form a HTTPMessage and enque it to HTTPConnecton queue.
    HTTPMessage* httpMessage = new HTTPMessage(buf);
    httpMessage->message << HTTP_SP;
    WsmWriter::addAuthHeader(httpMessage,_authenticator);
    PEG_TRACE_CSTRING(TRC_XML_IO, Tracer::LEVEL4,
        httpMessage->message.getData());
    _outputQueue->enqueue(httpMessage);

}

void WSMANExportRequestEncoder ::setDeliveryMode(deliveryMode &deliveryMode)
{
    _deliveryMode = deliveryMode;
}


PEGASUS_NAMESPACE_END

