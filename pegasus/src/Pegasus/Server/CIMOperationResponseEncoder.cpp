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
#include <Pegasus/Common/CIMBuffer.h>
#include <cctype>
#include <cstdio>
#include <Pegasus/Common/HTTPConnection.h>
#include <Pegasus/Common/BinaryCodec.h>
#include <Pegasus/Common/XmlParser.h>
#include <Pegasus/Common/XmlReader.h>
#include <Pegasus/Common/XmlWriter.h>
#include <Pegasus/Common/HTTPMessage.h>
#include <Pegasus/Common/Logger.h>
#include <Pegasus/Common/Tracer.h>
#include <Pegasus/Common/StatisticalData.h>
#include <Pegasus/Common/AutoPtr.h>
#include <Pegasus/Common/MessageLoader.h>
#include "CIMOperationResponseEncoder.h"

PEGASUS_USING_STD;

PEGASUS_NAMESPACE_BEGIN

CIMOperationResponseEncoder::CIMOperationResponseEncoder()
    : Base(PEGASUS_QUEUENAME_OPRESPENCODER)
{
}

CIMOperationResponseEncoder::~CIMOperationResponseEncoder()
{
}


//==============================================================================
//
// CIMOperationResponseEncoder::sendResponse()
//
//     This function is called once for every chunk comprising the inner part
//     of the HTTP payload. This is true whether chunking is enabled or not.
//     The "bodygiven" parameter contains all or part of the inner response
//     body. For example, in the case of the enumerate-instances XML response,
//     each "bodygiven" contains a complete named-instance as shown below.
//
//         <VALUE.NAMEDINSTANCE>
//         ...
//         <VALUE.NAMEDINSTANCE>
//
//     In the case of the get-class XML response, bodygiven contains the
//     entire class. Sometimes bodygiven is null, probably indicating that
//     one of the responding threads returned an empty response (for example,
//     a provider may return zero instances).
//
//     This function wraps the inner payload with the following elements:
//
//         1. HTTP status line.
//         2. HTTP headers.
//         3. Payload header.
//         4. Payload footer.
//
//     In the case of an enumerate-instances XML response, the payload header
//     contains all the XML leading up to the first XML chunk. For example:
//
//         <?xml version="1.0" encoding="utf-8" ?>
//         <CIM CIMVERSION="2.0" DTDVERSION="2.0">
//         <MESSAGE ID="1000" PROTOCOLVERSION="1.0">
//         <SIMPLERSP>
//         <IMETHODRESPONSE NAME="EnumerateInstances">
//         <IRETURNVALUE>
//
//     The payload footer then would just contain the closing tags for these:
//
//         </IRETURNVALUE>
//         </IMETHODRESPONSE>
//         </MESSAGE>
//         </SIMPLERSP>
//         </CIM>
//
//==============================================================================

/* Sent a response with a Buffer containing the body. This is based on
   sending response body (bodygiven).  This call is used by the non-pull
   operations.
*/

void CIMOperationResponseEncoder::sendResponse(
    CIMResponseMessage* response,
    const String& name,
    Boolean isImplicit,
    Buffer* bodygiven)
{
    Buffer bodyParams;
    sendResponsePull(response, name, isImplicit, &bodyParams, bodygiven);
}

/*
    Pull operation version of SendResponse.  This adds one additional
    parameter (bodyParamsIn) that contains the parameters.  This is because
    the parameters are added only on the the final segment of a chunk
*/
void CIMOperationResponseEncoder::sendResponsePull(
    CIMResponseMessage* response,
    const String& name,
    Boolean isImplicit,
    Buffer* bodyParams,
    Buffer* bodygiven)
{
    PEG_METHOD_ENTER(TRC_DISPATCHER,
        "CIMOperationResponseEncoder::sendResponse");
    PEG_TRACE((TRC_HTTP, Tracer::LEVEL4,
        "name = %s",
        (const char*)name.getCString()));

    if (! response)
    {
        PEG_METHOD_EXIT();
        return;
    }

    Uint32 queueId = response->queueIds.top();

    Boolean closeConnect = response->getCloseConnect();
    PEG_TRACE((
        TRC_HTTP,
        Tracer::LEVEL4,
        "CIMOperationResponseEncoder::sendResponse()- "
            "response->getCloseConnect() returned %d",
        closeConnect));

    MessageQueue* queue = MessageQueue::lookup(queueId);

    if (!queue)
    {
        PEG_TRACE((TRC_DISCARDED_DATA, Tracer::LEVEL1,
            "ERROR: non-existent queueId = %u, response not sent.", queueId));
        PEG_METHOD_EXIT();
        return;
    }

    HttpMethod httpMethod = response->getHttpMethod();
    String& messageId = response->messageId;
    CIMException& cimException = response->cimException;
    Buffer message;

    // Note: the language is ALWAYS passed empty to the xml formatters because
    // it is HTTPConnection that needs to make the decision of whether to add
    // the languages to the HTTP message.
    ContentLanguageList contentLanguage;

    CIMName cimName(name);
    Uint32 messageIndex = response->getIndex();
    Boolean isFirst = messageIndex == 0 ? true : false;
    Boolean isLast = response->isComplete();
    Buffer bodylocal;
    Buffer& body = bodygiven ? *bodygiven : bodylocal;

    Buffer& bodyParamsBuf = bodyParams ? *bodyParams : bodylocal;

    // STAT_SERVEREND sets the getTotalServerTime() value in the message class
    STAT_SERVEREND

#ifndef PEGASUS_DISABLE_PERFINST
    Uint64 serverTime = response->getTotalServerTime();
#else
    Uint64 serverTime = 0;
#endif

    Buffer (*formatResponse)(
        const CIMName& iMethodName,
        const String& messageId,
        HttpMethod httpMethod,
        const ContentLanguageList& httpContentLanguages,
        const Buffer& bodyParams,
        const Buffer& body,
        Uint64 serverResponseTime,
        Boolean isFirst,
        Boolean isLast);

    Buffer (*formatError)(
        const CIMName& methodName,
        const String& messageId,
        HttpMethod httpMethod,
        const CIMException& cimException);

    if (isImplicit == false)
    {
        formatResponse = XmlWriter::formatSimpleMethodRspMessage;
        formatError = XmlWriter::formatSimpleMethodErrorRspMessage;
    }
    else
    {
        formatError = XmlWriter::formatSimpleIMethodErrorRspMessage;

        if (response->binaryResponse)
        {
            formatResponse = BinaryCodec::formatSimpleIMethodRspMessage;
        }
        else
        {
            formatResponse = XmlWriter::formatSimpleIMethodRspMessage;
        }
    }

    if (cimException.getCode() != CIM_ERR_SUCCESS)
    {
        HTTPConnection* httpQueue = dynamic_cast<HTTPConnection*>(queue);
        Boolean isChunkRequest = false;
        Boolean isFirstError = true;

        // Note:  The WMI Mapper may use a non-HTTPConnection queue here.
        if (httpQueue)
        {
            isChunkRequest = httpQueue->isChunkRequested();
            isFirstError =
                (httpQueue->cimException.getCode() == CIM_ERR_SUCCESS);
        }

        // only process the FIRST error
        if (isFirstError)
        {
            // NOTE: even if this error occurs in the middle, HTTPConnection
            // will flush the entire queued message and reformat.
            if (isChunkRequest == false)
            {
                message =
                    formatError(name, messageId, httpMethod, cimException);
            }

            // uri encode the error (for the http header) only when it is
            // non-chunking or the first error with chunking
            if (isChunkRequest == false ||
                (isChunkRequest == true && isFirst == true))
            {
                String msg =
                    TraceableCIMException(cimException).getDescription();
                String uriEncodedMsg = XmlWriter::encodeURICharacters(msg);
                CIMException cimExceptionUri(
                    cimException.getCode(), uriEncodedMsg);
                cimExceptionUri.setContentLanguages(
                    cimException.getContentLanguages());
                cimException = cimExceptionUri;
            }
        } // if first error in response stream

        // never put the error in chunked response (because it will end up in
        // the trailer), so just use the non-error response formatter to send
        // more data

        if (isChunkRequest == true)
        {
            message = formatResponse(
                cimName,
                messageId,
                httpMethod,
                contentLanguage,
                bodyParamsBuf,
                body,
                serverTime,
                isFirst,
                isLast);
        }
    }
    else
    {
        // else non-error condition
        try
        {
            message = formatResponse(
                cimName,
                messageId,
                httpMethod,
                contentLanguage,
                bodyParamsBuf,
                body,
                serverTime,
                isFirst,
                isLast);
        }
        catch (PEGASUS_STD(bad_alloc)&)
        {
            MessageLoaderParms parms(
                "Server.CIMOperationResponseEncoder.OUT_OF_MEMORY",
                "A System error has occurred. Please retry the CIM Operation "
                    "at a later time.");

            Logger::put_l(
                Logger::ERROR_LOG, System::CIMSERVER, Logger::WARNING,
                parms);

            cimException = PEGASUS_CIM_EXCEPTION_L(CIM_ERR_FAILED, parms);

            // try again with new error and no body
            body.clear();
            sendResponse(response, name, isImplicit);
            PEG_METHOD_EXIT();
            return;
        }

        STAT_BYTESSENT
    }

    AutoPtr<HTTPMessage> httpMessage(
        new HTTPMessage(message, 0, &cimException));
    httpMessage->setComplete(isLast);
    httpMessage->setIndex(messageIndex);
    httpMessage->binaryResponse = response->binaryResponse;

    if (cimException.getCode() != CIM_ERR_SUCCESS)
    {
        httpMessage->contentLanguages = cimException.getContentLanguages();
    }
    else
    {
        const OperationContext::Container& container =
            response->operationContext.get(ContentLanguageListContainer::NAME);
        const ContentLanguageListContainer& listContainer =
            *dynamic_cast<const ContentLanguageListContainer*>(&container);
        contentLanguage = listContainer.getLanguages();
        httpMessage->contentLanguages = contentLanguage;
    }

    httpMessage->setCloseConnect(closeConnect);

    queue->enqueue(httpMessage.release());

    PEG_METHOD_EXIT();
}

void CIMOperationResponseEncoder::enqueue(Message* message)
{
    try
    {
        handleEnqueue(message);
    }
    catch(PEGASUS_STD(bad_alloc)&)
    {
        MessageLoaderParms parms(
            "Server.CIMOperationResponseEncoder.OUT_OF_MEMORY",
            "A System error has occurred. Please retry the CIM Operation "
                "at a later time.");

        Logger::put_l(
            Logger::ERROR_LOG, System::CIMSERVER, Logger::SEVERE, parms);

        CIMResponseMessage* response =
            dynamic_cast<CIMResponseMessage*>(message);
        Uint32 queueId = response->queueIds.top();
        MessageQueue* queue = MessageQueue::lookup(queueId);
        HTTPConnection* httpQueue = dynamic_cast<HTTPConnection*>(queue);
        PEGASUS_ASSERT(httpQueue);

        // Handle internal error on this connection.
        httpQueue->handleInternalServerError(
            response->getIndex(), response->isComplete());

        delete message;
    }
}

void CIMOperationResponseEncoder::handleEnqueue(Message* message)
{
    PEG_METHOD_ENTER(TRC_DISPATCHER,
        "CIMOperationResponseEncoder::handleEnqueue()");

    if (!message)
    {
        PEG_METHOD_EXIT();
        return;
    }

    CIMResponseMessage* response = dynamic_cast<CIMResponseMessage*>(message);
    PEGASUS_ASSERT(response);

    response->updateThreadLanguages();

    PEG_TRACE((
        TRC_HTTP,
        Tracer::LEVEL4,
        "CIMOperationResponseEncoder::handleEnque()- "
            "message->getCloseConnect() returned %d",
        message->getCloseConnect()));

    // Handle binary messages up front:
    {
        CIMResponseMessage* msg = dynamic_cast<CIMResponseMessage*>(message);

        if (msg && msg->binaryResponse)
        {
            if (msg->cimException.getCode() == CIM_ERR_SUCCESS)
            {
                Buffer body;
                CIMName name;

                if (BinaryCodec::encodeResponseBody(body, msg, name))
                {
                    sendResponse(msg, name.getString(), true, &body);
                    delete msg;
                    PEG_METHOD_EXIT();
                    return;
                }
            }
        }
    }

    switch (message->getType())
    {
        case CIM_GET_CLASS_RESPONSE_MESSAGE:
            encodeGetClassResponse(
                (CIMGetClassResponseMessage*)message);
            break;

        case CIM_GET_INSTANCE_RESPONSE_MESSAGE:
            encodeGetInstanceResponse(
                (CIMGetInstanceResponseMessage*)message);
            break;

        case CIM_DELETE_CLASS_RESPONSE_MESSAGE:
            encodeDeleteClassResponse(
                (CIMDeleteClassResponseMessage*)message);
            break;

        case CIM_DELETE_INSTANCE_RESPONSE_MESSAGE:
            encodeDeleteInstanceResponse(
                (CIMDeleteInstanceResponseMessage*)message);
            break;

        case CIM_CREATE_CLASS_RESPONSE_MESSAGE:
            encodeCreateClassResponse(
                (CIMCreateClassResponseMessage*)message);
            break;

        case CIM_CREATE_INSTANCE_RESPONSE_MESSAGE:
            encodeCreateInstanceResponse(
                (CIMCreateInstanceResponseMessage*)message);
            break;

        case CIM_MODIFY_CLASS_RESPONSE_MESSAGE:
            encodeModifyClassResponse(
                (CIMModifyClassResponseMessage*)message);
            break;

        case CIM_MODIFY_INSTANCE_RESPONSE_MESSAGE:
            encodeModifyInstanceResponse(
                (CIMModifyInstanceResponseMessage*)message);
            break;

        case CIM_ENUMERATE_CLASSES_RESPONSE_MESSAGE:
            encodeEnumerateClassesResponse(
                (CIMEnumerateClassesResponseMessage*)message);
            break;

        case CIM_ENUMERATE_CLASS_NAMES_RESPONSE_MESSAGE:
            encodeEnumerateClassNamesResponse(
                (CIMEnumerateClassNamesResponseMessage*)message);
            break;

        case CIM_ENUMERATE_INSTANCES_RESPONSE_MESSAGE:
            encodeEnumerateInstancesResponse(
                (CIMEnumerateInstancesResponseMessage*)message);
            break;

        case CIM_ENUMERATE_INSTANCE_NAMES_RESPONSE_MESSAGE:
            encodeEnumerateInstanceNamesResponse(
                (CIMEnumerateInstanceNamesResponseMessage*)message);
            break;

        case CIM_EXEC_QUERY_RESPONSE_MESSAGE:
            encodeExecQueryResponse(
                (CIMExecQueryResponseMessage*)message);
            break;

        case CIM_ASSOCIATORS_RESPONSE_MESSAGE:
            encodeAssociatorsResponse(
                (CIMAssociatorsResponseMessage*)message);
            break;

        case CIM_ASSOCIATOR_NAMES_RESPONSE_MESSAGE:
            encodeAssociatorNamesResponse(
                (CIMAssociatorNamesResponseMessage*)message);
            break;

        case CIM_REFERENCES_RESPONSE_MESSAGE:
            encodeReferencesResponse(
                (CIMReferencesResponseMessage*)message);
            break;

        case CIM_REFERENCE_NAMES_RESPONSE_MESSAGE:
            encodeReferenceNamesResponse(
                (CIMReferenceNamesResponseMessage*)message);
            break;

        case CIM_GET_PROPERTY_RESPONSE_MESSAGE:
            encodeGetPropertyResponse(
                (CIMGetPropertyResponseMessage*)message);
            break;

        case CIM_SET_PROPERTY_RESPONSE_MESSAGE:
            encodeSetPropertyResponse(
                (CIMSetPropertyResponseMessage*)message);
            break;

        case CIM_GET_QUALIFIER_RESPONSE_MESSAGE:
            encodeGetQualifierResponse(
                (CIMGetQualifierResponseMessage*)message);
            break;

        case CIM_SET_QUALIFIER_RESPONSE_MESSAGE:
            encodeSetQualifierResponse(
                (CIMSetQualifierResponseMessage*)message);
            break;

        case CIM_DELETE_QUALIFIER_RESPONSE_MESSAGE:
            encodeDeleteQualifierResponse(
                (CIMDeleteQualifierResponseMessage*)message);
            break;

        case CIM_ENUMERATE_QUALIFIERS_RESPONSE_MESSAGE:
            encodeEnumerateQualifiersResponse(
                (CIMEnumerateQualifiersResponseMessage*)message);
            break;

        case CIM_INVOKE_METHOD_RESPONSE_MESSAGE:
            encodeInvokeMethodResponse(
                (CIMInvokeMethodResponseMessage*)message);
            break;
//EXP_PULL_BEGIN
        case CIM_OPEN_ENUMERATE_INSTANCES_RESPONSE_MESSAGE:
            encodeOpenEnumerateInstancesResponse(
                (CIMOpenEnumerateInstancesResponseMessage*)message);
            break;

        case CIM_OPEN_ENUMERATE_INSTANCE_PATHS_RESPONSE_MESSAGE:
            encodeOpenEnumerateInstancePathsResponse(
                (CIMOpenEnumerateInstancePathsResponseMessage*)message);
            break;

        case CIM_OPEN_REFERENCE_INSTANCES_RESPONSE_MESSAGE:
            encodeOpenReferenceInstancesResponse(
                (CIMOpenReferenceInstancesResponseMessage*)message);
            break;

        case CIM_OPEN_REFERENCE_INSTANCE_PATHS_RESPONSE_MESSAGE:
            encodeOpenReferenceInstancePathsResponse(
                (CIMOpenReferenceInstancePathsResponseMessage*)message);
            break;

        case CIM_OPEN_ASSOCIATOR_INSTANCES_RESPONSE_MESSAGE:
            encodeOpenAssociatorInstancesResponse(
                (CIMOpenAssociatorInstancesResponseMessage*)message);
            break;

        case CIM_OPEN_ASSOCIATOR_INSTANCE_PATHS_RESPONSE_MESSAGE:
            encodeOpenAssociatorInstancePathsResponse(
                (CIMOpenAssociatorInstancePathsResponseMessage*)message);
            break;

        case CIM_PULL_INSTANCE_PATHS_RESPONSE_MESSAGE:
            encodePullInstancePathsResponse(
                (CIMPullInstancePathsResponseMessage*)message);
            break;

        case CIM_PULL_INSTANCES_WITH_PATH_RESPONSE_MESSAGE:
            encodePullInstancesWithPathResponse(
                (CIMPullInstancesWithPathResponseMessage*)message);
            break;

        case CIM_PULL_INSTANCES_RESPONSE_MESSAGE:
            encodePullInstancesResponse(
                (CIMPullInstancesResponseMessage*)message);
            break;

        case CIM_CLOSE_ENUMERATION_RESPONSE_MESSAGE:
            encodeCloseEnumerationResponse(
                (CIMCloseEnumerationResponseMessage*)message);
            break;

        case CIM_ENUMERATION_COUNT_RESPONSE_MESSAGE:
            encodeEnumerationCountResponse(
                (CIMEnumerationCountResponseMessage*)message);
            break;

        case CIM_OPEN_QUERY_INSTANCES_RESPONSE_MESSAGE:
            encodeOpenQueryInstancesResponse(
                (CIMOpenQueryInstancesResponseMessage*)message);
            break;
//EXP_PULL_END
        default:
            // Unexpected message type
            PEGASUS_UNREACHABLE(PEGASUS_ASSERT(0);)
            break;
    }

    delete message;

    PEG_METHOD_EXIT();
    return;
}

void CIMOperationResponseEncoder::handleEnqueue()
{
    Message* message = dequeue();
    if (message)
        handleEnqueue(message);
}

void CIMOperationResponseEncoder::encodeCreateClassResponse(
    CIMCreateClassResponseMessage* response)
{
    sendResponse(response, "CreateClass", true);
}

void CIMOperationResponseEncoder::encodeGetClassResponse(
    CIMGetClassResponseMessage* response)
{
    Buffer body;
    if (response->cimException.getCode() == CIM_ERR_SUCCESS)
        XmlWriter::appendClassElement(body, response->cimClass);
    sendResponse(response, "GetClass", true, &body);
}

void CIMOperationResponseEncoder::encodeModifyClassResponse(
    CIMModifyClassResponseMessage* response)
{
    sendResponse(response, "ModifyClass", true);
}

void CIMOperationResponseEncoder::encodeEnumerateClassNamesResponse(
    CIMEnumerateClassNamesResponseMessage* response)
{
    Buffer body;
    if (response->cimException.getCode() == CIM_ERR_SUCCESS)
        for (Uint32 i = 0, n = response->classNames.size(); i < n; i++)
            XmlWriter::appendClassNameElement(body, response->classNames[i]);
    sendResponse(response, "EnumerateClassNames", true, &body);
}

void CIMOperationResponseEncoder::encodeEnumerateClassesResponse(
    CIMEnumerateClassesResponseMessage* response)
{
    Buffer body;
    if (response->cimException.getCode() == CIM_ERR_SUCCESS)
        for (Uint32 i = 0, n= response->cimClasses.size(); i < n; i++)
            XmlWriter::appendClassElement(body, response->cimClasses[i]);
    sendResponse(response, "EnumerateClasses", true, &body);
}

void CIMOperationResponseEncoder::encodeDeleteClassResponse(
    CIMDeleteClassResponseMessage* response)
{
    sendResponse(response, "DeleteClass", true);
}

void CIMOperationResponseEncoder::encodeCreateInstanceResponse(
    CIMCreateInstanceResponseMessage* response)
{
    Buffer body;
    if (response->cimException.getCode() == CIM_ERR_SUCCESS)
        XmlWriter::appendInstanceNameElement(body, response->instanceName);
    sendResponse(response, "CreateInstance", true, &body);
}

void CIMOperationResponseEncoder::encodeGetInstanceResponse(
    CIMGetInstanceResponseMessage* response)
{
    Buffer body;
    if (response->cimException.getCode() == CIM_ERR_SUCCESS)
    {
        response->getResponseData().encodeXmlResponse(body, false);
    }
    sendResponse(response, "GetInstance", true, &body);
}

void CIMOperationResponseEncoder::encodeModifyInstanceResponse(
    CIMModifyInstanceResponseMessage* response)
{
    sendResponse(response, "ModifyInstance", true);
}

void CIMOperationResponseEncoder::encodeEnumerateInstancesResponse(
    CIMEnumerateInstancesResponseMessage* response)
{
    Buffer body;

    if (response->cimException.getCode() == CIM_ERR_SUCCESS)
    {
        response->getResponseData().encodeXmlResponse(body, false);
    }

    sendResponse(response, "EnumerateInstances", true, &body);
}

//EXP_PULL_BEGIN
/*
    Apply the EndOfSequence and EnumerationContext parameters to the
    supplied buffer. These parameters are standard on most open
    and pull responses.
    // FUTURE TODO: Since the common response message is
    // CIMOpenOrPullResponseDataMessage we could change this to just
    // pass the response message.
*/
void _appendOpenOrPullResponseParameters(Buffer& rtnParamBody,
    Boolean endOfSequence, String& enumerationContext)
{
    // Insert EndOfSequence. PerDSP0200 this is required output
    // parameter
    XmlWriter::appendBooleanParameter(rtnParamBody, "EndOfSequence",
        endOfSequence);

    // Insert EnumerationContext parameter. Per DSP0200 this is a required
    // parameter but may be NULL value if endOfSequence = true
    XmlWriter::appendStringParameter(rtnParamBody, "EnumerationContext",
        (endOfSequence? String::EMPTY : enumerationContext) );
}

void CIMOperationResponseEncoder::encodeOpenEnumerateInstancesResponse(
    CIMOpenEnumerateInstancesResponseMessage* response)
{
    Buffer body;
    Buffer rtnParamBody;

    if (response->cimException.getCode() == CIM_ERR_SUCCESS)
    {
        // Encode response objects with indication that this is pull
        // operation.
        response->getResponseData().encodeXmlResponse(body, true);

        // Add return elements, endOfSequence and context
        _appendOpenOrPullResponseParameters(rtnParamBody,
            response->endOfSequence, response->enumerationContext);
    }

    sendResponsePull(response, "OpenEnumerateInstances",
        true, &rtnParamBody, &body);
}

void CIMOperationResponseEncoder::encodeOpenEnumerateInstancePathsResponse(
    CIMOpenEnumerateInstancePathsResponseMessage* response)
{
    Buffer body;
    Buffer rtnParamBody;

    if (response->cimException.getCode() == CIM_ERR_SUCCESS)
    {
        response->getResponseData().encodeXmlResponse(body, true);

        // Add return elements, endOfSequence and Enumerationontext
        _appendOpenOrPullResponseParameters(rtnParamBody,
            response->endOfSequence, response->enumerationContext);
    }

    sendResponsePull(response, "OpenEnumerateInstancePaths",
        true, &rtnParamBody, &body);
}

void CIMOperationResponseEncoder::encodeOpenReferenceInstancesResponse(
    CIMOpenReferenceInstancesResponseMessage* response)
{
    Buffer body;
    Buffer rtnParamBody;

    if (response->cimException.getCode() == CIM_ERR_SUCCESS)
    {
        response->getResponseData().encodeXmlResponse(body, true);

        // Add return elements, endOfSequence and context
        _appendOpenOrPullResponseParameters(rtnParamBody,
            response->endOfSequence, response->enumerationContext);
    }


    sendResponsePull(response, "OpenReferenceInstances", true,
        &rtnParamBody, &body);
}

void CIMOperationResponseEncoder::encodeOpenReferenceInstancePathsResponse(
    CIMOpenReferenceInstancePathsResponseMessage* response)
{
    Buffer body;
    Buffer rtnParamBody;

    if (response->cimException.getCode() == CIM_ERR_SUCCESS)
    {
        response->getResponseData().encodeXmlResponse(body, true);

        // Add return elements, endOfSequence and context
        _appendOpenOrPullResponseParameters(rtnParamBody,
            response->endOfSequence, response->enumerationContext);
    }

    sendResponsePull(response, "OpenReferenceInstancePaths", true,
        &rtnParamBody, &body);
}

void CIMOperationResponseEncoder::encodeOpenAssociatorInstancesResponse(
    CIMOpenAssociatorInstancesResponseMessage* response)
{
    Buffer body;
    Buffer rtnParamBody;

    if (response->cimException.getCode() == CIM_ERR_SUCCESS)
    {
        response->getResponseData().encodeXmlResponse(body, true);

        // Add return elements, endOfSequence and context
        _appendOpenOrPullResponseParameters(rtnParamBody,
            response->endOfSequence, response->enumerationContext);
    }

    sendResponsePull(response, "OpenAssociatorInstances", true,
                     &rtnParamBody, &body);
}

void CIMOperationResponseEncoder::encodeOpenAssociatorInstancePathsResponse(
    CIMOpenAssociatorInstancePathsResponseMessage* response)
{
    Buffer body;
    Buffer rtnParamBody;

    if (response->cimException.getCode() == CIM_ERR_SUCCESS)
    {
        response->getResponseData().encodeXmlResponse(body, true);

        // Add return elements, endOfSequence and context
        _appendOpenOrPullResponseParameters(rtnParamBody,
            response->endOfSequence, response->enumerationContext);
    }

    sendResponsePull(response, "OpenAssociatorInstancePaths", true,
                     &rtnParamBody, &body);
}

void CIMOperationResponseEncoder::encodePullInstancesWithPathResponse(
    CIMPullInstancesWithPathResponseMessage* response)
{
    Buffer body;
    Buffer rtnParamBody;

    if (response->cimException.getCode() == CIM_ERR_SUCCESS)
    {
        response->getResponseData().encodeXmlResponse(body, true);

        // Add return elements, endOfSequence and context
        _appendOpenOrPullResponseParameters(rtnParamBody,
            response->endOfSequence, response->enumerationContext);
    }

    sendResponsePull(response, "PullInstancesWithPath", true,
                     &rtnParamBody, &body);
}

void CIMOperationResponseEncoder::encodePullInstancePathsResponse(
    CIMPullInstancePathsResponseMessage* response)
{
    Buffer body;
    Buffer rtnParamBody;

    if (response->cimException.getCode() == CIM_ERR_SUCCESS)
    {
        response->getResponseData().encodeXmlResponse(body, true);

        // Add return parameters-endOfSequence and context
        _appendOpenOrPullResponseParameters(rtnParamBody,
            response->endOfSequence, response->enumerationContext);
    }

    sendResponsePull(response, "PullInstancePaths", true, &rtnParamBody,
                     &body);
}

void CIMOperationResponseEncoder::encodePullInstancesResponse(
    CIMPullInstancesResponseMessage* response)
{
    Buffer body;
    Buffer rtnParamBody;

    if (response->cimException.getCode() == CIM_ERR_SUCCESS)
    {
        // encode as pull and also encode only the instance.
        // if Content type is Object, encode as instance
        response->getResponseData().encodeXmlResponse(body, true, true);

        // Add return elements, endOfSequence and context
        _appendOpenOrPullResponseParameters(rtnParamBody,
            response->endOfSequence, response->enumerationContext);
    }

    sendResponsePull(response, "PullInstances", true,
                     &rtnParamBody, &body);
}

void CIMOperationResponseEncoder::encodeCloseEnumerationResponse(
    CIMCloseEnumerationResponseMessage* response)
{
    Buffer body;
    Buffer rtnParamBody;

    sendResponsePull(response, "CloseEnumeration", true, &rtnParamBody,
                     &body);
}

void CIMOperationResponseEncoder::encodeEnumerationCountResponse(
    CIMEnumerationCountResponseMessage* response)
{
    Buffer body;
    Buffer rtnParamBody;

    XmlWriter::appendUint64ReturnValue(rtnParamBody, "Count",
        response->count);

    sendResponsePull(response, "EnumerationCount", true, &rtnParamBody, &body);
}

void CIMOperationResponseEncoder::encodeOpenQueryInstancesResponse(
    CIMOpenQueryInstancesResponseMessage* response)
{
    Buffer body;
    Buffer rtnParamBody;

    if (response->cimException.getCode() == CIM_ERR_SUCCESS)
    {
        // Encode response objects with indication that this is pull
        // operation and that we encode only the instance
        // If objectType is object, encode as instance
        response->getResponseData().encodeXmlResponse(body, true, true);
    }

    // KS_FUTURE implement the addition of the queryClassResult
    // Add return elements, endOfSequence and context
    _appendOpenOrPullResponseParameters(rtnParamBody,
        response->endOfSequence, response->enumerationContext);

    sendResponsePull(response, "OpenQueryInstances",
        true, &rtnParamBody, &body);
}
//EXP_PULL_END

void CIMOperationResponseEncoder::encodeEnumerateInstanceNamesResponse(
    CIMEnumerateInstanceNamesResponseMessage* response)
{
    Buffer body;

    if (response->cimException.getCode() == CIM_ERR_SUCCESS)
    {
        response->getResponseData().encodeXmlResponse(body, false);
    }
    sendResponse(response, "EnumerateInstanceNames", true, &body);
}

void CIMOperationResponseEncoder::encodeDeleteInstanceResponse(
    CIMDeleteInstanceResponseMessage* response)
{
    sendResponse(response, "DeleteInstance", true);
}

void CIMOperationResponseEncoder::encodeGetPropertyResponse(
    CIMGetPropertyResponseMessage* response)
{
    Buffer body;
    if (response->cimException.getCode() == CIM_ERR_SUCCESS)
        XmlWriter::appendValueElement(body, response->value);
    sendResponse(response, "GetProperty", true, &body);
}

void CIMOperationResponseEncoder::encodeSetPropertyResponse(
    CIMSetPropertyResponseMessage* response)
{
    sendResponse(response, "SetProperty", true);
}

void CIMOperationResponseEncoder::encodeSetQualifierResponse(
    CIMSetQualifierResponseMessage* response)
{
    sendResponse(response, "SetQualifier", true);
}

void CIMOperationResponseEncoder::encodeGetQualifierResponse(
    CIMGetQualifierResponseMessage* response)
{
    Buffer body;
    if (response->cimException.getCode() == CIM_ERR_SUCCESS)
        XmlWriter::appendQualifierDeclElement(body, response->cimQualifierDecl);
    sendResponse(response, "GetQualifier", true, &body);
}

void CIMOperationResponseEncoder::encodeEnumerateQualifiersResponse(
    CIMEnumerateQualifiersResponseMessage* response)
{
    Buffer body;
    if (response->cimException.getCode() == CIM_ERR_SUCCESS)
        for (Uint32 i = 0, n = response->qualifierDeclarations.size();
             i < n; i++)
            XmlWriter::appendQualifierDeclElement(
                body, response->qualifierDeclarations[i]);
    sendResponse(response, "EnumerateQualifiers", true, &body);
}

void CIMOperationResponseEncoder::encodeDeleteQualifierResponse(
    CIMDeleteQualifierResponseMessage* response)
{
    sendResponse(response, "DeleteQualifier", true);
}

void CIMOperationResponseEncoder::encodeReferenceNamesResponse(
    CIMReferenceNamesResponseMessage* response)
{
    Buffer body;
    if (response->cimException.getCode() == CIM_ERR_SUCCESS)
    {
        response->getResponseData().encodeXmlResponse(body, false);
    }
    sendResponse(response, "ReferenceNames", true, &body);
}

void CIMOperationResponseEncoder::encodeReferencesResponse(
    CIMReferencesResponseMessage* response)
{
    Buffer body;
    if (response->cimException.getCode() == CIM_ERR_SUCCESS)
    {
        response->getResponseData().encodeXmlResponse(body, false);
    }
    sendResponse(response, "References", true, &body);
}

void CIMOperationResponseEncoder::encodeAssociatorNamesResponse(
    CIMAssociatorNamesResponseMessage* response)
{
    Buffer body;
    if (response->cimException.getCode() == CIM_ERR_SUCCESS)
    {
        response->getResponseData().encodeXmlResponse(body, false);
    }
    sendResponse(response, "AssociatorNames", true, &body);
}

void CIMOperationResponseEncoder::encodeAssociatorsResponse(
    CIMAssociatorsResponseMessage* response)
{
    Buffer body;
    if (response->cimException.getCode() == CIM_ERR_SUCCESS)
    {
        response->getResponseData().encodeXmlResponse(body, false);
    }
    sendResponse(response, "Associators", true, &body);
}

void CIMOperationResponseEncoder::encodeExecQueryResponse(
    CIMExecQueryResponseMessage* response)
{
    Buffer body;
    if (response->cimException.getCode() == CIM_ERR_SUCCESS)
    {
        response->getResponseData().encodeXmlResponse(body, false);
    }
    sendResponse(response, "ExecQuery", true, &body);
}

void CIMOperationResponseEncoder::encodeInvokeMethodResponse(
    CIMInvokeMethodResponseMessage* response)
{
    Buffer body;

    // ATTN-RK-P3-20020219: Who's job is it to make sure the return value is
    // not an array?
    // Only add the return value if it is not null

    if (response->cimException.getCode() == CIM_ERR_SUCCESS)
    {
        if (!response->retValue.isNull())
            XmlWriter::appendReturnValueElement(body, response->retValue);

        for (Uint32 i=0, n = response->outParameters.size(); i < n; i++)
            XmlWriter::appendParamValueElement(
                body, response->outParameters[i]);
    }
    sendResponse(response, response->methodName.getString(), false, &body);
}

PEGASUS_NAMESPACE_END
