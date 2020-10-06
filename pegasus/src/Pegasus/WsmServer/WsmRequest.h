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

#ifndef Pegasus_WsmRequest_h
#define Pegasus_WsmRequest_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/OperationContext.h>
#include <Pegasus/Common/AcceptLanguageList.h>
#include <Pegasus/Common/ContentLanguageList.h>
#include <Pegasus/Common/ArrayInternal.h>
#include <Pegasus/Common/Message.h>
#include <Pegasus/Common/SharedPtr.h>
#include <Pegasus/WsmServer/WsmConstants.h>
#include <Pegasus/WsmServer/WsmSelectorSet.h>
#include <Pegasus/WsmServer/WsmInstance.h>
#include <Pegasus/WQL/WQLParser.h>
#include <Pegasus/WsmServer/WsmFilter.h>

PEGASUS_NAMESPACE_BEGIN

enum WsmOperationType
{
    WSM_FAULT,
    SOAP_FAULT,

    WSM_IDENTITY_IDENTIFY,

    WS_TRANSFER_GET,
    WS_TRANSFER_PUT,
    WS_TRANSFER_CREATE,
    WS_TRANSFER_DELETE,

    WS_ENUMERATION_ENUMERATE,
    WS_ENUMERATION_PULL,
    WS_ENUMERATION_RELEASE,

    /* WS-Management invoke */
    WS_INVOKE,
    WS_EXPORT_INDICATION,
    // etc.
    
    WS_SUBSCRIPTION_CREATE,
    WS_SUBSCRIPTION_DELETE
};

class WsmRequest : public Message 
{
public:

    WsmRequest(
        WsmOperationType type,
        const String& messageId_,
        MessageType msgType=DUMMY_MESSAGE)
        : Message(msgType),messageId(messageId_),
          httpMethod(HTTP_METHOD__POST),
          httpCloseConnect(false),
          omitXMLProcessingInstruction(false),
          queueId(0),
          requestEpr(false),
          maxEnvelopeSize(0),
          _type(type)
    {
    }

    virtual ~WsmRequest()
    {
    }

    WsmOperationType getType() const
    {
        return _type;
    }

    void copyRequestProperties(const AutoPtr<WsmRequest> &request)
    {
        authType = request->authType;
        userName = request->userName;
        ipAddress = request->ipAddress;
        httpMethod = request->httpMethod;
        acceptLanguages = request->acceptLanguages;
        contentLanguages = request->contentLanguages;
        httpCloseConnect = request->httpCloseConnect;
        omitXMLProcessingInstruction = 
            request->omitXMLProcessingInstruction;
        queueId = request->queueId;
        requestEpr = request->requestEpr;
        maxEnvelopeSize = request->maxEnvelopeSize;
    }

    String messageId;
    String authType;
    String userName;
    String ipAddress;
    HttpMethod httpMethod;
    AcceptLanguageList acceptLanguages;
    ContentLanguageList contentLanguages;
    Boolean httpCloseConnect;
    Boolean omitXMLProcessingInstruction;
    Uint32 queueId;
    Boolean requestEpr;
    Uint32 maxEnvelopeSize;

private:

    WsmOperationType _type;
};

class WxfGetRequest : public WsmRequest
{
public:

    WxfGetRequest(
        const String& messageId,
        const WsmEndpointReference& epr_)
        : WsmRequest(WS_TRANSFER_GET, messageId),
          epr(epr_)
    {
    }

    WsmEndpointReference epr;
};

class WxfPutRequest : public WsmRequest
{
public:

    WxfPutRequest(
        const String& messageId,
        const WsmEndpointReference& epr_,
        const WsmInstance& instance_)
        : WsmRequest(WS_TRANSFER_PUT, messageId),
          epr(epr_),
          instance(instance_)
    {
    }

    WsmEndpointReference epr;
    WsmInstance instance;
};

class WxfCreateRequest : public WsmRequest
{
public:

    WxfCreateRequest(
        const String& messageId,
        const WsmEndpointReference& epr_,
        const WsmInstance& instance_)
        : WsmRequest(WS_TRANSFER_CREATE, messageId),
          epr(epr_),
          instance(instance_)
    {
    }

    WsmEndpointReference epr;
    WsmInstance instance;
};

class WxfSubCreateRequest : public WsmRequest
{
public:

    WxfSubCreateRequest(
        const String& messageId,
        const WsmEndpointReference& epr_,
        const WsmInstance& instance_)
        : WsmRequest(WS_SUBSCRIPTION_CREATE, messageId),
          epr(epr_),
          instance(instance_)
    {
    }

    WsmEndpointReference epr;
    WsmInstance instance;
};

class WxfDeleteRequest : public WsmRequest
{
public:

    WxfDeleteRequest(
        const String& messageId,
        const WsmEndpointReference& epr_)
        : WsmRequest(WS_TRANSFER_DELETE, messageId),
          epr(epr_)
    {
    }

    WsmEndpointReference epr;
};

class WxfSubDeleteRequest : public WsmRequest
{
public:

    WxfSubDeleteRequest(
        const String& messageId,
        const WsmEndpointReference& epr_,
        String className_)
        : WsmRequest(WS_SUBSCRIPTION_DELETE, messageId),
          className(className_),
          epr(epr_)
    {
    }
    String className;
    WsmEndpointReference epr;
};

class WsenEnumerateRequest : public WsmRequest
{
public:

    WsenEnumerateRequest(
        const String& messageId,
        const WsmEndpointReference& epr_,
        const String& expiration_,
        Boolean requestItemCount_,
        Boolean optimized_,
        Uint32 maxElements_,
        WsenEnumerationMode enumerationMode_,
        WsmbPolymorphismMode polymorphismMode_,
        WsmFilter wsmFilter_
        )
        : WsmRequest(WS_ENUMERATION_ENUMERATE, messageId),
          epr(epr_),
          expiration(expiration_),
          requestItemCount(requestItemCount_),
          optimized(optimized_),
          maxElements(maxElements_),
          enumerationMode(enumerationMode_),
          polymorphismMode(polymorphismMode_),
          wsmFilter(wsmFilter_)
    {
    }

    ~WsenEnumerateRequest()
    {
    }

    WsmEndpointReference epr;
    String expiration;
    Boolean requestItemCount;
    Boolean optimized;
    Uint32 maxElements;
    WsenEnumerationMode enumerationMode;
    WsmbPolymorphismMode polymorphismMode;
    WsmFilter wsmFilter;
};

class WsenPullRequest : public WsmRequest
{
public:

    WsenPullRequest(
        const String& messageId,
        const WsmEndpointReference& epr_,
        Uint64 enumerationContext_,
        const String& maxTime_,
        Boolean requestItemCount_,
        Uint32 maxElements_,
        Uint32 maxCharacters_)
        : WsmRequest(WS_ENUMERATION_PULL, messageId),
          epr(epr_),
          enumerationContext(enumerationContext_),
          maxTime(maxTime_),
          requestItemCount(requestItemCount_),
          maxElements(maxElements_),
          maxCharacters(maxCharacters_)
    {
    }

    WsmEndpointReference epr;
    Uint64 enumerationContext;
    String maxTime;
    Boolean requestItemCount;
    Uint32 maxElements;
    Uint32 maxCharacters;
};

class WsenReleaseRequest : public WsmRequest
{
public:

    WsenReleaseRequest(
        const String& messageId,
        const WsmEndpointReference& epr_,
        Uint64 enumerationContext_)
        : WsmRequest(WS_ENUMERATION_RELEASE, messageId),
          epr(epr_),
          enumerationContext(enumerationContext_)
    {
    }

    WsmEndpointReference epr;
    Uint64 enumerationContext;
};

class WsInvokeRequest : public WsmRequest
{
public:

    WsInvokeRequest(
        const String& messageId,
        const WsmEndpointReference& epr_,
        const String& className_,
        const String& methodName_,
        const WsmInstance& instance_)
        :
        WsmRequest(WS_INVOKE, messageId),
        epr(epr_),
        className(className_),
        methodName(methodName_),
        instance(instance_)
    {
    }

    WsmEndpointReference epr;
    String className;
    String methodName;
    WsmInstance instance;
};

class WsExportIndicationRequest : public WsmRequest
{
public :
    
    WsExportIndicationRequest(
        const String& messageId,
        const String& url_,
        const String& destination_,
        const WsmInstance& instance_)
        :WsmRequest(
            WS_EXPORT_INDICATION,
            messageId,
            WSMAN_EXPORT_INDICATION_REQUEST_MESSAGE),
        url(url_),
        destination(destination_),
        IndicationInstance(instance_) 
    {
    }

    String url;
    String destination;
    WsmInstance IndicationInstance;   
    
};
PEGASUS_NAMESPACE_END

#endif /* Pegasus_WsmRequest_h */
