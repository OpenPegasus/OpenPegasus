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

#include <cctype>
#include <cstdio>
#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/HTTPConnection.h>
#include <Pegasus/Common/HTTPMessage.h>
#include <Pegasus/Common/Tracer.h>
#include <Pegasus/Common/AutoPtr.h>
#include <Pegasus/Common/MessageLoader.h>

#include "WsmConstants.h"
#include "WsmReader.h"
#include "WsmWriter.h"
#include "WsmResponseEncoder.h"
#include "WsmToCimRequestMapper.h"
#include "SoapResponse.h"
#include "CimToWsmResponseMapper.h"

PEGASUS_USING_STD;

PEGASUS_NAMESPACE_BEGIN

WsmResponseEncoder::WsmResponseEncoder()
{
}

WsmResponseEncoder::~WsmResponseEncoder()
{
}

void WsmResponseEncoder::_sendResponse(SoapResponse* response)
{
    PEG_METHOD_ENTER(TRC_WSMSERVER, "WsmResponseEncoder::sendResponse");
    PEG_TRACE((TRC_WSMSERVER, Tracer::LEVEL3,
        "WsmResponseEncoder::sendResponse()"));

    if (!response)
    {
        PEG_METHOD_EXIT();
        return;
    }

    Uint32 queueId = response->getQueueId();
    Boolean httpCloseConnect = response->getHttpCloseConnect();

    PEG_TRACE((TRC_WSMSERVER, Tracer::LEVEL4,
        "WsmResponseEncoder::sendResponse()- "
            "response->getHttpCloseConnect() returned %d",
        httpCloseConnect));

    MessageQueue* queue = MessageQueue::lookup(queueId);
    if (!queue)
    {
        PEG_TRACE((TRC_DISCARDED_DATA, Tracer::LEVEL1,
            "ERROR: non-existent queueId = %u, response not sent.", queueId));
        PEG_METHOD_EXIT();
        return;
    }
    PEGASUS_ASSERT(dynamic_cast<HTTPConnection*>(queue) != 0);

    Buffer message = response->getResponseContent();

    // Note: WS-Management responses are never sent in chunks, so there is no
    // need to check dynamic_cast<HTTPConnection*>(queue)->isChunkRequested().
    // HTTPMessage::isComplete() defaults to true, and we leave it that way.

    AutoPtr<HTTPMessage> httpMessage(new HTTPMessage(message));

    httpMessage->setCloseConnect(httpCloseConnect);
    queue->enqueue(httpMessage.release());

    PEG_METHOD_EXIT();
}

void WsmResponseEncoder::_sendUnreportableSuccess(WsmResponse* response)
{
    // DSP0226 R6.2-2:  If the mustUnderstand attribute is set to
    // "true", the service shall comply with the request.  If the
    // response would exceed the maximum size, the service should
    // return a wsman:EncodingLimit fault.  Because a service might
    // execute the operation prior to knowing the response size, the
    // service should undo any effects of the operation before
    // issuing the fault.  If the operation cannot be reversed (such
    // as a destructive wxf:Put or wxf:Delete, or a wxf:Create), the
    // service shall indicate that the operation succeeded in the
    // wsman:EncodingLimit fault with the following detail code:
    //     http://schemas.dmtf.org/wbem/wsman/1/wsman/faultDetail/
    //         UnreportableSuccess

    WsmFault fault(WsmFault::wsman_EncodingLimit,
        MessageLoaderParms(
            "WsmServer.WsmResponseEncoder.UNREPORTABLE_SUCCESS",
            "Success response could not be encoded within "
            "requested envelope size limits."),
        WSMAN_FAULTDETAIL_UNREPORTABLESUCCESS);
    WsmFaultResponse faultResponse(
        response->getRelatesTo(),
        response->getQueueId(),
        response->getHttpMethod(),
        response->getHttpCloseConnect(),
        response->getOmitXMLProcessingInstruction(),
        fault);

    SoapResponse soapResponse(&faultResponse);
    _sendResponse(&soapResponse);
}

SoapResponse* WsmResponseEncoder::_buildEncodingLimitFault(
    WsmResponse* response)
{
    WsmFault fault(WsmFault::wsman_EncodingLimit,
        MessageLoaderParms(
            "WsmServer.WsmResponseEncoder.MAX_ENV_SIZE_EXCEEDED",
            "Response could not be encoded within requested "
            "envelope size limits."),
        WSMAN_FAULTDETAIL_MAXENVELOPESIZE);
    WsmFaultResponse faultResponse(
        response->getRelatesTo(),
        response->getQueueId(),
        response->getHttpMethod(),
        response->getHttpCloseConnect(),
        response->getOmitXMLProcessingInstruction(),
        fault);

    return new SoapResponse(&faultResponse);
}

void WsmResponseEncoder::enqueue(WsmResponse* response)
{
    PEG_METHOD_ENTER(TRC_WSMSERVER, "WsmResponseEncoder::enqueue()");
    PEGASUS_ASSERT(response);

    PEG_TRACE((TRC_WSMSERVER, Tracer::LEVEL4,
        "WsmResponseEncoder::enqueue()- "
            "response->getHttpCloseConnect() returned %d",
        response->getHttpCloseConnect()));

    try
    {
        switch (response->getOperationType())
        {
            case WS_TRANSFER_GET:
                _encodeWxfGetResponse((WxfGetResponse*) response);
                break;

            case WS_TRANSFER_PUT:
                _encodeWxfPutResponse((WxfPutResponse*) response);
                break;

            case WS_TRANSFER_CREATE:
                _encodeWxfCreateResponse((WxfCreateResponse*) response);
                break;

            case WS_SUBSCRIPTION_CREATE:
                _encodeWxfSubCreateResponse((WxfSubCreateResponse*) response);
                break;

            case WS_TRANSFER_DELETE:
                _encodeWxfDeleteResponse((WxfDeleteResponse*) response);
                break;
           
            case WS_SUBSCRIPTION_DELETE:
                _encodeWxfSubDeleteResponse((WxfSubDeleteResponse*) response);
                break;

            case WS_ENUMERATION_RELEASE:
                _encodeWsenReleaseResponse((WsenReleaseResponse*) response);
                break;

            case WSM_FAULT:
                _encodeWsmFaultResponse((WsmFaultResponse*) response);
                break;

            case SOAP_FAULT:
                _encodeSoapFaultResponse((SoapFaultResponse*) response);
                break;

            case WS_INVOKE:
                _encodeWsInvokeResponse((WsInvokeResponse*)response);
                break;

            case WS_ENUMERATION_ENUMERATE:
            case WS_ENUMERATION_PULL:
                // These cases are handled specially to allow for the message
                // contents to be tuned according to the MaxEnvelopeSize value.
            default:
                // Unexpected message type
                PEGASUS_UNREACHABLE(PEGASUS_ASSERT(0);)
                break;
        }
    }
    catch (PEGASUS_STD(bad_alloc)&)
    {
       MessageLoaderParms parms(
            "WsmServer.WsmResponseEncoder.OUT_OF_MEMORY",
            "A System error has occurred. Please retry the "
                "WS-Management operation at a later time.");

        Logger::put_l(
            Logger::ERROR_LOG, System::CIMSERVER, Logger::SEVERE, parms);

        MessageQueue* queue = MessageQueue::lookup(response->getQueueId());
        HTTPConnection* httpQueue = dynamic_cast<HTTPConnection*>(queue);
        PEGASUS_ASSERT(httpQueue);

        // Handle the internal server error on this connection.
        httpQueue->handleInternalServerError(0, true);
    }

    PEG_METHOD_EXIT();
}

void WsmResponseEncoder::_encodeWxfGetResponse(WxfGetResponse* response)
{
    SoapResponse soapResponse(response);
    Buffer body;
    WsmWriter::appendInstanceElement(body, response->getResourceUri(),
        response->getInstance(), PEGASUS_INSTANCE_NS, false);
    if (soapResponse.appendBodyContent(body))
    {
        _sendResponse(&soapResponse);
    }
    else
    {
        _sendUnreportableSuccess(response);
    }
}

void WsmResponseEncoder::_encodeWxfPutResponse(WxfPutResponse* response)
{
    SoapResponse soapResponse(response);
    Buffer headers;

    // DSP0226 R6.5-1:  A service receiving a message that contains the
    // wsman:RequestEPR header block should return a response that contains
    // a wsman:RequestedEPR header block.  This block contains the most recent
    // EPR of the resource being accessed or a status code if the service
    // cannot determine or return the EPR.  This EPR reflects any identity
    // changes that may have occurred as a result of the current operation, as
    // set forth in the following behavior.  The header block in the
    // corresponding response message has the following format:
    //     <wsman:RequestedEPR...>
    //       [ <wsa:EndpointReference>
    //          wsa:EndpointReferenceType
    //       </wsa:EndpointReference> |
    //       <wsman:EPRInvalid/> |
    //       <wsman:EPRUnknown/> ]
    //     </wsman:RequestedEPR>
    if (response->getRequestedEPR())
    {
        WsmWriter::appendStartTag(
            headers, WsmNamespaces::WS_MAN, STRLIT("RequestedEPR"));
        WsmWriter::appendStartTag(
            headers,
            WsmNamespaces::WS_ADDRESSING, STRLIT("EndpointReference"));
        WsmWriter::appendEPRElement(headers, response->getEPR());
        WsmWriter::appendEndTag(
            headers,
            WsmNamespaces::WS_ADDRESSING, STRLIT("EndpointReference"));
        WsmWriter::appendEndTag(
            headers, WsmNamespaces::WS_MAN, STRLIT("RequestedEPR"));
    }

    if (soapResponse.appendHeader(headers))
    {
        _sendResponse(&soapResponse);
    }
    else
    {
        _sendUnreportableSuccess(response);
    }
}

void WsmResponseEncoder::_encodeWxfCreateResponse(WxfCreateResponse* response)
{
    SoapResponse soapResponse(response);
    Buffer body;

    WsmWriter::appendStartTag(
        body, WsmNamespaces::WS_TRANSFER, STRLIT("ResourceCreated"));
    WsmWriter::appendEPRElement(body, response->getEPR());
    WsmWriter::appendEndTag(
        body, WsmNamespaces::WS_TRANSFER, STRLIT("ResourceCreated"));

    if (soapResponse.appendBodyContent(body))
    {
        _sendResponse(&soapResponse);
    }
    else
    {
        _sendUnreportableSuccess(response);
    }
}

void WsmResponseEncoder::_encodeWxfSubCreateResponse(
    WxfSubCreateResponse* response)
{
    SoapResponse soapResponse(response);
    Buffer body;
    WsmEndpointReference epr = response->getEPR();

    WsmWriter::appendStartTag(
        body, WsmNamespaces::WS_EVENTING, STRLIT("SubscribeResponse"));
    WsmWriter::appendStartTag(
        body, WsmNamespaces::WS_EVENTING, STRLIT("SubscriptionManager"));

    WsmWriter::appendTagValue( body, 
        WsmNamespaces::WS_EVENTING, 
        STRLIT("Address"), 
        epr.address);

    WsmWriter::appendStartTag(
        body, WsmNamespaces::WS_EVENTING, STRLIT("ReferenceParameters"));

    // This is the identifier of the Subscription. Subscription name is 
    // the subscribe request's messageID(without the uuid part) which 
    // is same as relates to field of the response.
    String subId = response->getRelatesTo().subString(PEGASUS_WS_UUID_LENGTH);
    WsmWriter::appendTagValue( body, 
        WsmNamespaces::WS_EVENTING, 
        STRLIT("Identifier"), 
        subId);

    WsmWriter::appendEndTag(
        body, WsmNamespaces::WS_EVENTING, STRLIT("ReferenceParameters"));
    WsmWriter::appendEndTag(
        body, WsmNamespaces::WS_EVENTING, STRLIT("SubscriptionManager"));

    CIMDateTime dt;
    String dat;

    response->getSubscriptionDuration(dt);

    if(WsmUtils::toMicroSecondString(dt) != "0")
    {
        CimToWsmResponseMapper Map;
        Map.convertCimToWsmDatetime(dt, dat);
        /** 
            Get the expires field. This is as duration in the 
            IndicationSubscription instance, with the units as microseconds. 
            We get it as a string, then convert it to Uint64, then convert 
            that to the CIMDatetime interval format. That is converted to the
            WsmDatetime format, which is something like P1DT1M1S.
        */
        WsmWriter::appendTagValue( body, 
            WsmNamespaces::WS_EVENTING, 
            STRLIT("Expires"),
            dat);
    }
    WsmWriter::appendEndTag(
        body, WsmNamespaces::WS_EVENTING, STRLIT("SubscribeResponse"));

    if (soapResponse.appendBodyContent(body))
    {
        _sendResponse(&soapResponse);
    }
    else
    {
        _sendUnreportableSuccess(response);
    }
}

void WsmResponseEncoder::_encodeWxfDeleteResponse(WxfDeleteResponse* response)
{
    SoapResponse soapResponse(response);
    _sendResponse(&soapResponse);
}

void WsmResponseEncoder::_encodeWxfSubDeleteResponse(
    WxfSubDeleteResponse* response)
{
    SoapResponse soapResponse(response);
    _sendResponse(&soapResponse);
}


SoapResponse* WsmResponseEncoder::encodeWsenEnumerateResponse(
    WsenEnumerateResponse* response,
    Uint32& numDataItemsEncoded)
{
    PEG_METHOD_ENTER(TRC_WSMSERVER,
        "WsmResponseEncoder::encodeWsenEnumerateResponse");

    AutoPtr<SoapResponse> soapResponse(new SoapResponse(response));
    Buffer headers;

    if (response->requestedItemCount())
    {
        WsmWriter::appendStartTag(
            headers, WsmNamespaces::WS_MAN, STRLIT("TotalItemsCountEstimate"));
        WsmWriter::append(headers, response->getItemCount());
        WsmWriter::appendEndTag(
            headers, WsmNamespaces::WS_MAN, STRLIT("TotalItemsCountEstimate"));
    }

    if (!_encodeEnumerationData(
            *soapResponse.get(),
            headers,
            WS_ENUMERATION_ENUMERATE,
            response->getEnumerationContext(),
            response->isComplete(),
            response->getEnumerationData(),
            numDataItemsEncoded,
            response->getResourceUri()))
    {
        soapResponse.reset(_buildEncodingLimitFault(response));
    }

    PEG_METHOD_EXIT();
    return soapResponse.release();
}

SoapResponse* WsmResponseEncoder::encodeWsenPullResponse(
    WsenPullResponse* response,
    Uint32& numDataItemsEncoded)
{
    AutoPtr<SoapResponse> soapResponse(new SoapResponse(response));
    Buffer headers;

    if (!_encodeEnumerationData(
            *soapResponse.get(),
            headers,
            WS_ENUMERATION_PULL,
            response->getEnumerationContext(),
            response->isComplete(),
            response->getEnumerationData(),
            numDataItemsEncoded,
            response->getResourceUri()))
    {
        soapResponse.reset(_buildEncodingLimitFault(response));
    }

    return soapResponse.release();
}

Boolean WsmResponseEncoder::_encodeEnumerationData(
    SoapResponse& soapResponse,
    Buffer& headers,
    WsmOperationType operation,
    Uint64 contextId,
    Boolean isComplete,
    WsenEnumerationData& data,
    Uint32& numDataItemsEncoded,
    const String& resourceUri)
{
    PEG_METHOD_ENTER(TRC_WSMSERVER,
        "WsmResponseEncoder::_encodeEnumerationData");
    Buffer bodyHeader, bodyTrailer;

    PEGASUS_ASSERT(operation == WS_ENUMERATION_ENUMERATE ||
        operation == WS_ENUMERATION_PULL);

    numDataItemsEncoded = 0;

    WsmWriter::appendStartTag(
        bodyHeader, WsmNamespaces::WS_ENUMERATION,
        operation == WS_ENUMERATION_ENUMERATE ?
            STRLIT("EnumerateResponse") : STRLIT("PullResponse"));

    // Include an EnumerationContext in the response.  If this response
    // completes the enumeration, this element will be modified/removed below.
    Uint32 ecPos = bodyHeader.size();
    WsmWriter::appendStartTag(
        bodyHeader, WsmNamespaces::WS_ENUMERATION,
        STRLIT("EnumerationContext"));
    WsmWriter::append(bodyHeader, contextId);
    WsmWriter::appendEndTag(
        bodyHeader, WsmNamespaces::WS_ENUMERATION,
        STRLIT("EnumerationContext"));
    Uint32 ecSize = bodyHeader.size() - ecPos;

    PEG_TRACE((TRC_WSMSERVER, Tracer::LEVEL4,
               "Encoder data size %u ",data.getSize()));

    if (data.getSize() > 0)
    {
        WsmWriter::appendStartTag(
            bodyHeader,
            operation == WS_ENUMERATION_ENUMERATE ?
                WsmNamespaces::WS_MAN : WsmNamespaces::WS_ENUMERATION,
            STRLIT("Items"));
        WsmWriter::appendEndTag(
            bodyTrailer,
            operation == WS_ENUMERATION_ENUMERATE ?
                WsmNamespaces::WS_MAN : WsmNamespaces::WS_ENUMERATION,
            STRLIT("Items"));
    }

    Uint32 eosPos = bodyTrailer.size();
    Uint32 eosSize = 0;
    if (isComplete)
    {
        // Write an EndOfSequence element with the expectation that all the
        // elements fit within MaxEnvelopeSize.  This element will be removed
        // below if the assumption proves untrue.  This element is written
        // up front before all the response data was included, because adding
        // the EndOfSequence element later might push the response size past
        // the MaxEnvelopeSize.
        WsmWriter::appendEmptyTag(
            bodyTrailer,
            operation == WS_ENUMERATION_ENUMERATE ?
                WsmNamespaces::WS_MAN : WsmNamespaces::WS_ENUMERATION,
            STRLIT("EndOfSequence"));
        eosSize = bodyTrailer.size() - eosPos;
    }

    WsmWriter::appendEndTag(
        bodyTrailer, WsmNamespaces::WS_ENUMERATION,
        operation == WS_ENUMERATION_ENUMERATE ?
            STRLIT("EnumerateResponse") : STRLIT("PullResponse"));

    // Fault the request if it can't be encoded within the limits
    if (!soapResponse.appendHeader(headers) ||
        !soapResponse.appendBodyHeader(bodyHeader) ||
        !soapResponse.appendBodyTrailer(bodyTrailer))
    {
        return false;
    }

    // Now add the list of items
    Uint32 i = 0;

    if (data.enumerationMode == WSEN_EM_OBJECT)
    {
        for (i = 0; i < data.instances.size(); i++)
        {
            Buffer body;

            if (data.polymorphismMode == WSMB_PM_EXCLUDE_SUBCLASS_PROPERTIES)
            {
                // The response does not contain the subclass properties, but
                // the class name is still that of the subclass.
                // Replace it here.
                data.instances[i].setClassName(
                    WsmToCimRequestMapper::convertResourceUriToClassName(
                        data.classUri).getString());
            }

            WsmWriter::appendInstanceElement(body, resourceUri,
                data.instances[i], PEGASUS_INSTANCE_NS, false);

            if (!soapResponse.appendBodyContent(body))
            {
                break;
            }
        }
    }
    else if (data.enumerationMode == WSEN_EM_EPR)
    {
        for (i = 0; i < data.eprs.size(); i++)
        {
            Buffer body;

            WsmWriter::appendStartTag(
                body,
                WsmNamespaces::WS_ADDRESSING,
                STRLIT("EndpointReference"));
            WsmWriter::appendEPRElement(body, data.eprs[i]);
            WsmWriter::appendEndTag(
                body,
                WsmNamespaces::WS_ADDRESSING,
                STRLIT("EndpointReference"));
            if (!soapResponse.appendBodyContent(body))
            {
                break;
            }
        }
    }
    else if (data.enumerationMode == WSEN_EM_OBJECT_AND_EPR)
    {
        for (i = 0; i < data.instances.size(); i++)
        {
            Buffer body;

            WsmWriter::appendStartTag(
                body,
                WsmNamespaces::WS_MAN,
                STRLIT("Item"));

            if (data.polymorphismMode == WSMB_PM_EXCLUDE_SUBCLASS_PROPERTIES)
            {
                // The response does not contain the subclass properties, but
                // the class name is still that of the subclass.
                // Replace it here.
                data.instances[i].setClassName(
                    WsmToCimRequestMapper::convertResourceUriToClassName(
                        data.classUri).getString());
            }

            WsmWriter::appendInstanceElement(body, resourceUri,
                data.instances[i], PEGASUS_INSTANCE_NS, false);

            WsmWriter::appendStartTag(
                body,
                WsmNamespaces::WS_ADDRESSING,
                STRLIT("EndpointReference"));
            WsmWriter::appendEPRElement(body, data.eprs[i]);
            WsmWriter::appendEndTag(
                body,
                WsmNamespaces::WS_ADDRESSING,
                STRLIT("EndpointReference"));

            WsmWriter::appendEndTag(
                body,
                WsmNamespaces::WS_MAN,
                STRLIT("Item"));

            if (!soapResponse.appendBodyContent(body))
            {
                break;
            }
        }
    }
    else
    {
        PEGASUS_UNREACHABLE(PEGASUS_ASSERT(0);)
    }

    numDataItemsEncoded = i;

    // If the list is not empty, but none of the items have been successfully
    // added to the soapResponse, fault the request because it cannot be
    // encoded within the specified limits.
    if (data.getSize() > 0 && numDataItemsEncoded == 0)
    {
        return false;
    }

    if (isComplete)
    {
        if (data.getSize() > numDataItemsEncoded)
        {
            // The request is complete but could not be encoded within
            // MaxEnvelopeSize.  Clear EndOfSequence tag.
            soapResponse.getBodyTrailer().remove(eosPos, eosSize);
        }
        else
        {
            // All the enumeration results were written.  Update the
            // EnumerationContext element.
            if (operation == WS_ENUMERATION_ENUMERATE)
            {
                // DSP0226 R8.2.3-5:  A conformant service that supports
                // optimized enumeration and has not returned all items of the
                // enumeration sequence in the wsen:EnumerateResponse message
                // shall return a wsen:EnumerationContext element that is
                // initialized such that a subsequent wsen:Pull message will
                // return the set of items after those returned in the
                // wsen:EnumerateResponse. If all items of the enumeration
                // sequence have been returned in the wsen:EnumerateResponse
                // message, the service should return an empty
                // wsen:EnumerationContext element and shall return the
                // wsman:EndOfSequence element in the response.
                Buffer emptyEc(50);
                WsmWriter::appendEmptyTag(
                    emptyEc, WsmNamespaces::WS_ENUMERATION,
                    STRLIT("EnumerationContext"));
                soapResponse.getBodyHeader().remove(ecPos, ecSize);
                soapResponse.getBodyHeader().insert(
                    ecPos, emptyEc.getData(), emptyEc.size());
            }
            else
            {
                // DSP0226 R8.4-8:  If the wsen:EndOfSequence marker occurs in
                // the wsen:PullResponse message, the wsen:EnumerationContext
                // element shall be omitted, as the enumeration has completed.
                // The client cannot subsequently issue a wsen:Release message.
                soapResponse.getBodyHeader().remove(ecPos, ecSize);
            }
        }
    }
    PEG_METHOD_EXIT();
    return true;
}

void WsmResponseEncoder::_encodeWsenReleaseResponse(
    WsenReleaseResponse* response)
{
    SoapResponse soapResponse(response);
    _sendResponse(&soapResponse);
}

void WsmResponseEncoder::_encodeWsmFaultResponse(WsmFaultResponse* response)
{
    SoapResponse soapResponse(response);
    _sendResponse(&soapResponse);
}

void WsmResponseEncoder::_encodeSoapFaultResponse(SoapFaultResponse* response)
{
    SoapResponse soapResponse(response);
    _sendResponse(&soapResponse);
}

void WsmResponseEncoder::_encodeWsInvokeResponse(
    WsInvokeResponse* response)
{
    SoapResponse* soapResponse = new SoapResponse(response);

    Buffer body;
    WsmWriter::appendInvokeOutputElement(
        body,
        response->resourceUri,
        response->className,
        response->methodName,
        response->instance,
        PEGASUS_INVOKE_NS);

    if (soapResponse->appendBodyContent(body))
    {
        _sendResponse(soapResponse);
    }
    else
    {
        delete soapResponse;
        _sendUnreportableSuccess(response);
    }
}

PEGASUS_NAMESPACE_END
