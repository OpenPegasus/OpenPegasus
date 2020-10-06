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
#include "SoapResponse.h"
#include "WsmWriter.h"

PEGASUS_USING_STD;

PEGASUS_NAMESPACE_BEGIN

SoapResponse::SoapResponse(WsmResponse* response)
{
    String action;
    ContentLanguageList lang;

    _maxEnvelopeSize = response->getMaxEnvelopeSize();
    _queueId = response->getQueueId();
    _httpCloseConnect = response->getHttpCloseConnect();

    WsmWriter::appendSoapEnvelopeStart(_envStart);
    WsmWriter::appendSoapEnvelopeEnd(_envEnd);
    WsmWriter::appendSoapHeaderStart(_hdrStart);
    WsmWriter::appendSoapHeaderEnd(_hdrEnd);
    WsmWriter::appendSoapBodyStart(_bodyStart);
    WsmWriter::appendSoapBodyEnd(_bodyEnd);

    switch(response->getOperationType())
    {
        case WS_TRANSFER_GET:
            action = WSM_ACTION_GET_RESPONSE;
            break;

        case WS_TRANSFER_PUT:
            action = WSM_ACTION_PUT_RESPONSE;
            break;

        case WS_TRANSFER_CREATE:
            action = WSM_ACTION_CREATE_RESPONSE;
            break;

        case WS_SUBSCRIPTION_CREATE:
            action = WSM_ACTION_SUBSCRIBE_RESPONSE;
            break;

        case WS_TRANSFER_DELETE:
            action = WSM_ACTION_DELETE_RESPONSE;
            break;
         
        case WS_SUBSCRIPTION_DELETE:
            action = WSM_ACTION_UNSUBSCRIBE_RESPONSE;
            break;    
     
        case WS_ENUMERATION_ENUMERATE:
            action = WSM_ACTION_ENUMERATE_RESPONSE;
            break;

        case WS_ENUMERATION_PULL:
            action = WSM_ACTION_PULL_RESPONSE;
            break;

        case WS_ENUMERATION_RELEASE:
            action = WSM_ACTION_RELEASE_RESPONSE;
            break;

        case WSM_FAULT:
            action = ((WsmFaultResponse*) response)->getFault().getAction();
            WsmWriter::appendSoapHeader(_hdrContent,
                action, response->getMessageId(), response->getRelatesTo());
            WsmWriter::appendWsmFaultBody(_bodyContent,
                ((WsmFaultResponse*) response)->getFault());
            break;

        case SOAP_FAULT:
            action = String(WsmNamespaces::supportedNamespaces[
                WsmNamespaces::WS_ADDRESSING].extendedName) + String("/fault");
            WsmWriter::appendSoapFaultHeaders(_hdrContent,
                ((SoapFaultResponse*) response)->getFault(),
                action, response->getMessageId(), response->getRelatesTo());
            WsmWriter::appendSoapFaultBody(_bodyContent,
                ((SoapFaultResponse*) response)->getFault());
            break;

        case WS_INVOKE:
        {
            WsInvokeResponse* rsp = (WsInvokeResponse*)response;

            // Get root of resource URI.
            String root = WsmUtils::getRootResourceUri(rsp->resourceUri);

            // Build the action.
            action = root;
            action.append('/');
            action.append(rsp->className);
            action.append('/');
            action.append(rsp->methodName);
            break;
        }

        default:
            PEGASUS_UNREACHABLE(PEGASUS_ASSERT(0);)
    }

    WsmWriter::appendHTTPResponseHeader(_httpHeader, action,
       response->getHttpMethod(), response->getOmitXMLProcessingInstruction(),
       response->getContentLanguages(),
       response->getOperationType() == WSM_FAULT || 
           response->getOperationType() == SOAP_FAULT,0);

    // Make sure that fault response fits within MaxEnvelopeSize
    if (response->getOperationType() == WSM_FAULT || 
        response->getOperationType() == SOAP_FAULT)
    {
        if (_maxEnvelopeSize && getEnvelopeSize() > _maxEnvelopeSize)
        {
            _bodyContent.clear();
            WsmFault fault(WsmFault::wsman_EncodingLimit,
                MessageLoaderParms(
                    "WsmServer.WsmResponseEncoder.FAULT_MAX_ENV_SIZE_EXCEEDED",
                    "Fault response could not be encoded within requested "
                    "envelope size limits."),
                WSMAN_FAULTDETAIL_MAXENVELOPESIZE);
            WsmWriter::appendWsmFaultBody(_bodyContent, fault);
        }
    }
    else
    {
        WsmWriter::appendSoapHeader(_hdrContent,
            action, response->getMessageId(), response->getRelatesTo());
    }
}

Buffer SoapResponse::getResponseContent()
{
    Buffer out(WSM_MIN_MAXENVELOPESIZE_VALUE);
    out << _httpHeader << _envStart << _hdrStart << _hdrContent
        << _hdrEnd << _bodyStart << _bodyHeader << _bodyContent
        << _bodyTrailer << _bodyEnd << _envEnd;
    return out;
}

Boolean SoapResponse::appendHeader(Buffer& buf)
{
    if (_maxEnvelopeSize &&
        getEnvelopeSize() + buf.size() > _maxEnvelopeSize)
    {
        return false;
    }
    _hdrContent << buf;
    return true;
}

Boolean SoapResponse::appendBodyContent(Buffer& buf)
{
    if (_maxEnvelopeSize &&
        getEnvelopeSize() + buf.size() > _maxEnvelopeSize)
    {
        return false;
    }
    _bodyContent << buf;
    return true;
}

Boolean SoapResponse::appendBodyHeader(Buffer& buf)
{
    if (_maxEnvelopeSize &&
        getEnvelopeSize() + buf.size() > _maxEnvelopeSize)
    {
        return false;
    }
    _bodyHeader << buf;
    return true;
}

Boolean SoapResponse::appendBodyTrailer(Buffer& buf)
{
    if (_maxEnvelopeSize &&
        getEnvelopeSize() + buf.size() > _maxEnvelopeSize)
    {
        return false;
    }
    _bodyTrailer << buf;
    return true;
}

PEGASUS_NAMESPACE_END
