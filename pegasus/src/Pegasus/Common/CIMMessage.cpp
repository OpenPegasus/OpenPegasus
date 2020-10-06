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

#include <Pegasus/Common/AutoPtr.h>
#include <Pegasus/Common/StatisticalData.h>
#include "CIMMessage.h"
#include "XmlWriter.h"
#include "CIMResponseData.h"

PEGASUS_USING_STD;

PEGASUS_NAMESPACE_BEGIN

#define PEGASUS_ARRAY_T ArraySint8
# include "ArrayImpl.h"
#undef PEGASUS_ARRAY_T

void CIMResponseMessage::syncAttributes(const CIMRequestMessage* request)
{
    // Propagate request attributes to the response, as necessary
    setMask(request->getMask());
    setHttpMethod(request->getHttpMethod());
    setCloseConnect(request->getCloseConnect());
#ifndef PEGASUS_DISABLE_PERFINST
    setServerStartTime(request->getServerStartTime());
#endif
    binaryRequest = request->binaryRequest;
    binaryResponse = request->binaryResponse;
    internalOperation = request->internalOperation;
}

CIMResponseMessage* CIMGetClassRequestMessage::buildResponse() const
{
    AutoPtr<CIMGetClassResponseMessage> response(
        new CIMGetClassResponseMessage(
            messageId,
            CIMException(),
            queueIds.copyAndPop(),
            CIMClass()));
    response->syncAttributes(this);
    return response.release();
}

CIMResponseMessage* CIMGetInstanceRequestMessage::buildResponse() const
{
    AutoPtr<CIMGetInstanceResponseMessage> response(
        new CIMGetInstanceResponseMessage(
            messageId,
            CIMException(),
            queueIds.copyAndPop()));
    CIMResponseData & rspData = response->getResponseData();
    rspData.setRequestProperties(
        includeQualifiers,
        includeClassOrigin,
        propertyList);
    response->syncAttributes(this);
    return response.release();
}

CIMResponseMessage* CIMExportIndicationRequestMessage::buildResponse() const
{
    AutoPtr<CIMExportIndicationResponseMessage> response(
        new CIMExportIndicationResponseMessage(
            messageId,
            CIMException(),
            queueIds.copyAndPop()));
    response->syncAttributes(this);
    return response.release();
}

CIMResponseMessage* CIMDeleteClassRequestMessage::buildResponse() const
{
    AutoPtr<CIMDeleteClassResponseMessage> response(
        new CIMDeleteClassResponseMessage(
            messageId,
            CIMException(),
            queueIds.copyAndPop()));
    response->syncAttributes(this);
    return response.release();
}

CIMResponseMessage* CIMDeleteInstanceRequestMessage::buildResponse() const
{
    AutoPtr<CIMDeleteInstanceResponseMessage> response(
        new CIMDeleteInstanceResponseMessage(
            messageId,
            CIMException(),
            queueIds.copyAndPop()));
    response->syncAttributes(this);
    return response.release();
}

CIMResponseMessage* CIMCreateClassRequestMessage::buildResponse() const
{
    AutoPtr<CIMCreateClassResponseMessage> response(
        new CIMCreateClassResponseMessage(
            messageId,
            CIMException(),
            queueIds.copyAndPop()));
    response->syncAttributes(this);
    return response.release();
}

CIMResponseMessage* CIMCreateInstanceRequestMessage::buildResponse() const
{
    AutoPtr<CIMCreateInstanceResponseMessage> response(
        new CIMCreateInstanceResponseMessage(
            messageId,
            CIMException(),
            queueIds.copyAndPop(),
            CIMObjectPath()));
    response->syncAttributes(this);
    return response.release();
}

CIMResponseMessage* CIMModifyClassRequestMessage::buildResponse() const
{
    AutoPtr<CIMModifyClassResponseMessage> response(
        new CIMModifyClassResponseMessage(
            messageId,
            CIMException(),
            queueIds.copyAndPop()));
    response->syncAttributes(this);
    return response.release();
}

CIMResponseMessage* CIMModifyInstanceRequestMessage::buildResponse() const
{
    AutoPtr<CIMModifyInstanceResponseMessage> response(
        new CIMModifyInstanceResponseMessage(
            messageId,
            CIMException(),
            queueIds.copyAndPop()));
    response->syncAttributes(this);
    return response.release();
}

CIMResponseMessage* CIMEnumerateClassesRequestMessage::buildResponse() const
{
    AutoPtr<CIMEnumerateClassesResponseMessage> response(
        new CIMEnumerateClassesResponseMessage(
            messageId,
            CIMException(),
            queueIds.copyAndPop(),
            Array<CIMClass>()));
    response->syncAttributes(this);
    return response.release();
}

CIMResponseMessage* CIMEnumerateClassNamesRequestMessage::buildResponse() const
{
    AutoPtr<CIMEnumerateClassNamesResponseMessage> response(
        new CIMEnumerateClassNamesResponseMessage(
            messageId,
            CIMException(),
            queueIds.copyAndPop(),
            Array<CIMName>()));
    response->syncAttributes(this);
    return response.release();
}

CIMResponseMessage* CIMEnumerateInstancesRequestMessage::buildResponse() const
{
    AutoPtr<CIMEnumerateInstancesResponseMessage> response(
        new CIMEnumerateInstancesResponseMessage(
            messageId,
            CIMException(),
            queueIds.copyAndPop()));
    CIMResponseData & rspData = response->getResponseData();
    rspData.setRequestProperties(
        includeQualifiers,
        includeClassOrigin,
        propertyList);
    response->syncAttributes(this);
    return response.release();
}

CIMResponseMessage*
    CIMEnumerateInstanceNamesRequestMessage::buildResponse() const
{
    AutoPtr<CIMEnumerateInstanceNamesResponseMessage> response(
        new CIMEnumerateInstanceNamesResponseMessage(
            messageId,
            CIMException(),
            queueIds.copyAndPop()));
    response->syncAttributes(this);
    return response.release();
}

CIMResponseMessage* CIMExecQueryRequestMessage::buildResponse() const
{
    AutoPtr<CIMExecQueryResponseMessage> response(
        new CIMExecQueryResponseMessage(
            messageId,
            CIMException(),
            queueIds.copyAndPop()));
    response->syncAttributes(this);
    return response.release();
}

CIMResponseMessage* CIMAssociatorsRequestMessage::buildResponse() const
{
    AutoPtr<CIMAssociatorsResponseMessage> response(
        new CIMAssociatorsResponseMessage(
            messageId,
            CIMException(),
            queueIds.copyAndPop()));
    CIMResponseData & rspData = response->getResponseData();
    rspData.setRequestProperties(
        includeQualifiers,
        includeClassOrigin,
        propertyList);
    rspData.setIsClassOperation(isClassRequest);
    response->syncAttributes(this);
    return response.release();
}

CIMResponseMessage* CIMAssociatorNamesRequestMessage::buildResponse() const
{
    AutoPtr<CIMAssociatorNamesResponseMessage> response(
        new CIMAssociatorNamesResponseMessage(
            messageId,
            CIMException(),
            queueIds.copyAndPop()));
    CIMResponseData & rspData = response->getResponseData();
    rspData.setIsClassOperation(isClassRequest);
    response->syncAttributes(this);
    return response.release();
}

CIMResponseMessage* CIMReferencesRequestMessage::buildResponse() const
{
    AutoPtr<CIMReferencesResponseMessage> response(
        new CIMReferencesResponseMessage(
            messageId,
            CIMException(),
            queueIds.copyAndPop()));
    CIMResponseData & rspData = response->getResponseData();
    rspData.setRequestProperties(
        includeQualifiers,
        includeClassOrigin,
        propertyList);
    rspData.setIsClassOperation(isClassRequest);
    response->syncAttributes(this);
    return response.release();
}

CIMResponseMessage* CIMReferenceNamesRequestMessage::buildResponse() const
{
    AutoPtr<CIMReferenceNamesResponseMessage> response(
        new CIMReferenceNamesResponseMessage(
            messageId,
            CIMException(),
            queueIds.copyAndPop()));
    CIMResponseData & rspData = response->getResponseData();
    rspData.setIsClassOperation(isClassRequest);
    response->syncAttributes(this);
    return response.release();
}

CIMResponseMessage* CIMGetPropertyRequestMessage::buildResponse() const
{
    AutoPtr<CIMGetPropertyResponseMessage> response(
        new CIMGetPropertyResponseMessage(
            messageId,
            CIMException(),
            queueIds.copyAndPop(),
            CIMValue()));
    response->syncAttributes(this);
    return response.release();
}

CIMResponseMessage* CIMSetPropertyRequestMessage::buildResponse() const
{
    AutoPtr<CIMSetPropertyResponseMessage> response(
        new CIMSetPropertyResponseMessage(
            messageId,
            CIMException(),
            queueIds.copyAndPop()));
    response->syncAttributes(this);
    return response.release();
}

CIMResponseMessage* CIMGetQualifierRequestMessage::buildResponse() const
{
    AutoPtr<CIMGetQualifierResponseMessage> response(
        new CIMGetQualifierResponseMessage(
            messageId,
            CIMException(),
            queueIds.copyAndPop(),
            CIMQualifierDecl()));
    response->syncAttributes(this);
    return response.release();
}

CIMResponseMessage* CIMSetQualifierRequestMessage::buildResponse() const
{
    AutoPtr<CIMSetQualifierResponseMessage> response(
        new CIMSetQualifierResponseMessage(
            messageId,
            CIMException(),
            queueIds.copyAndPop()));
    response->syncAttributes(this);
    return response.release();
}

CIMResponseMessage* CIMDeleteQualifierRequestMessage::buildResponse() const
{
    AutoPtr<CIMDeleteQualifierResponseMessage> response(
        new CIMDeleteQualifierResponseMessage(
            messageId,
            CIMException(),
            queueIds.copyAndPop()));
    response->syncAttributes(this);
    return response.release();
}

CIMResponseMessage* CIMEnumerateQualifiersRequestMessage::buildResponse() const
{
    AutoPtr<CIMEnumerateQualifiersResponseMessage> response(
        new CIMEnumerateQualifiersResponseMessage(
            messageId,
            CIMException(),
            queueIds.copyAndPop(),
            Array<CIMQualifierDecl>()));
    response->syncAttributes(this);
    return response.release();
}

CIMResponseMessage* CIMInvokeMethodRequestMessage::buildResponse() const
{
    AutoPtr<CIMInvokeMethodResponseMessage> response(
        new CIMInvokeMethodResponseMessage(
            messageId,
            CIMException(),
            queueIds.copyAndPop(),
            CIMValue(),
            Array<CIMParamValue>(),
            methodName));
    response->syncAttributes(this);
    return response.release();
}

CIMResponseMessage* CIMProcessIndicationRequestMessage::buildResponse() const
{
    AutoPtr<CIMProcessIndicationResponseMessage> response(
        new CIMProcessIndicationResponseMessage(
            messageId,
            CIMException(),
            queueIds.copyAndPop()));
    response->syncAttributes(this);
    return response.release();
}

CIMResponseMessage*
    CIMNotifyProviderRegistrationRequestMessage::buildResponse() const
{
    AutoPtr<CIMNotifyProviderRegistrationResponseMessage> response(
        new CIMNotifyProviderRegistrationResponseMessage(
            messageId,
            CIMException(),
            queueIds.copyAndPop()));
    response->syncAttributes(this);
    return response.release();
}

CIMResponseMessage*
    CIMNotifyProviderTerminationRequestMessage::buildResponse() const
{
    AutoPtr<CIMNotifyProviderTerminationResponseMessage> response(
        new CIMNotifyProviderTerminationResponseMessage(
            messageId,
            CIMException(),
            queueIds.copyAndPop()));
    response->syncAttributes(this);
    return response.release();
}

CIMResponseMessage* CIMHandleIndicationRequestMessage::buildResponse() const
{
    AutoPtr<CIMHandleIndicationResponseMessage> response(
        new CIMHandleIndicationResponseMessage(
            messageId,
            CIMException(),
            queueIds.copyAndPop()));
    response->syncAttributes(this);
    return response.release();
}

CIMResponseMessage* CIMCreateSubscriptionRequestMessage::buildResponse() const
{
    AutoPtr<CIMCreateSubscriptionResponseMessage> response(
        new CIMCreateSubscriptionResponseMessage(
            messageId,
            CIMException(),
            queueIds.copyAndPop()));
    response->syncAttributes(this);
    return response.release();
}

CIMResponseMessage* CIMModifySubscriptionRequestMessage::buildResponse() const
{
    AutoPtr<CIMModifySubscriptionResponseMessage> response(
        new CIMModifySubscriptionResponseMessage(
            messageId,
            CIMException(),
            queueIds.copyAndPop()));
    response->syncAttributes(this);
    return response.release();
}

CIMResponseMessage* CIMDeleteSubscriptionRequestMessage::buildResponse() const
{
    AutoPtr<CIMDeleteSubscriptionResponseMessage> response(
        new CIMDeleteSubscriptionResponseMessage(
            messageId,
            CIMException(),
            queueIds.copyAndPop()));
    response->syncAttributes(this);
    return response.release();
}

CIMResponseMessage*
    CIMSubscriptionInitCompleteRequestMessage::buildResponse() const
{
    AutoPtr<CIMSubscriptionInitCompleteResponseMessage> response(
        new CIMSubscriptionInitCompleteResponseMessage(
            messageId,
            CIMException(),
            queueIds.copyAndPop()));
    response->syncAttributes(this);
    return response.release();
}

CIMResponseMessage*
    CIMIndicationServiceDisabledRequestMessage::buildResponse() const
{
    AutoPtr<CIMIndicationServiceDisabledResponseMessage> response(
        new CIMIndicationServiceDisabledResponseMessage(
            messageId,
            CIMException(),
            queueIds.copyAndPop()));
    response->syncAttributes(this);
    return response.release();
}

CIMResponseMessage* CIMDisableModuleRequestMessage::buildResponse() const
{
    AutoPtr<CIMDisableModuleResponseMessage> response(
        new CIMDisableModuleResponseMessage(
            messageId,
            CIMException(),
            queueIds.copyAndPop(),
            Array<Uint16>()));
    response->syncAttributes(this);
    return response.release();
}

CIMResponseMessage* CIMEnableModuleRequestMessage::buildResponse() const
{
    AutoPtr<CIMEnableModuleResponseMessage> response(
        new CIMEnableModuleResponseMessage(
            messageId,
            CIMException(),
            queueIds.copyAndPop(),
            Array<Uint16>()));
    response->syncAttributes(this);
    return response.release();
}

CIMResponseMessage* CIMNotifyProviderEnableRequestMessage::buildResponse() const
{
    AutoPtr<CIMNotifyProviderEnableResponseMessage> response(
        new CIMNotifyProviderEnableResponseMessage(
            messageId,
            CIMException(),
            queueIds.copyAndPop()));
    response->syncAttributes(this);
    return response.release();
}

CIMResponseMessage* CIMNotifyProviderFailRequestMessage::buildResponse() const
{
    AutoPtr<CIMNotifyProviderFailResponseMessage> response(
        new CIMNotifyProviderFailResponseMessage(
            messageId,
            CIMException(),
            queueIds.copyAndPop()));
    response->syncAttributes(this);
    return response.release();
}

CIMResponseMessage* CIMStopAllProvidersRequestMessage::buildResponse() const
{
    AutoPtr<CIMStopAllProvidersResponseMessage> response(
        new CIMStopAllProvidersResponseMessage(
            messageId,
            CIMException(),
            queueIds.copyAndPop()));
    response->syncAttributes(this);
    return response.release();
}

CIMResponseMessage*
    CIMInitializeProviderAgentRequestMessage::buildResponse() const
{
    AutoPtr<CIMInitializeProviderAgentResponseMessage> response(
        new CIMInitializeProviderAgentResponseMessage(
            messageId,
            CIMException(),
            queueIds.copyAndPop()));
    response->syncAttributes(this);
    return response.release();
}

CIMResponseMessage* CIMNotifyConfigChangeRequestMessage::buildResponse() const
{
    AutoPtr<CIMNotifyConfigChangeResponseMessage> response(
        new CIMNotifyConfigChangeResponseMessage(
            messageId,
            CIMException(),
            queueIds.copyAndPop()));
    response->syncAttributes(this);
    return response.release();
}
//EXP_PULL_BEGIN
CIMResponseMessage*
    CIMOpenEnumerateInstancesRequestMessage::buildResponse() const
{
    AutoPtr<CIMOpenEnumerateInstancesResponseMessage> response(
        new CIMOpenEnumerateInstancesResponseMessage(
            messageId,
            CIMException(),
            queueIds.copyAndPop()));
    CIMResponseData & rspData = response->getResponseData();
    rspData.setRequestProperties(
        false,
        includeClassOrigin,
        propertyList);
    response->syncAttributes(this);
    return response.release();
}

CIMResponseMessage*
    CIMOpenEnumerateInstancePathsRequestMessage::buildResponse() const
{
    AutoPtr<CIMOpenEnumerateInstancePathsResponseMessage> response(
        new CIMOpenEnumerateInstancePathsResponseMessage(
            messageId,
            CIMException(),
            queueIds.copyAndPop()));
    response->syncAttributes(this);
    return response.release();
}

CIMResponseMessage*
    CIMOpenReferenceInstancesRequestMessage::buildResponse() const
{
    AutoPtr<CIMOpenReferenceInstancesResponseMessage> response(
        new CIMOpenReferenceInstancesResponseMessage(
            messageId,
            CIMException(),
            queueIds.copyAndPop()));
    CIMResponseData & rspData = response->getResponseData();
    rspData.setRequestProperties(
        false,
        includeClassOrigin,
        propertyList);
    response->syncAttributes(this);
    return response.release();
}

CIMResponseMessage*
    CIMOpenReferenceInstancePathsRequestMessage::buildResponse() const
{
    AutoPtr<CIMOpenReferenceInstancePathsResponseMessage> response(
        new CIMOpenReferenceInstancePathsResponseMessage(
            messageId,
            CIMException(),
            queueIds.copyAndPop()));
    response->syncAttributes(this);
    return response.release();
}

CIMResponseMessage*
    CIMOpenAssociatorInstancesRequestMessage::buildResponse() const
{
    AutoPtr<CIMOpenAssociatorInstancesResponseMessage> response(
        new CIMOpenAssociatorInstancesResponseMessage(
            messageId,
            CIMException(),
            queueIds.copyAndPop()));
    CIMResponseData & rspData = response->getResponseData();
    rspData.setRequestProperties(
        false,
        includeClassOrigin,
        propertyList);
    response->syncAttributes(this);
    return response.release();
}

CIMResponseMessage*
    CIMOpenAssociatorInstancePathsRequestMessage::buildResponse() const
{
    AutoPtr<CIMOpenAssociatorInstancePathsResponseMessage> response(
        new CIMOpenAssociatorInstancePathsResponseMessage(
            messageId,
            CIMException(),
            queueIds.copyAndPop()));
    response->syncAttributes(this);
    return response.release();
}

CIMResponseMessage*
    CIMPullInstancesWithPathRequestMessage::buildResponse() const
{
    AutoPtr<CIMPullInstancesWithPathResponseMessage> response(
        new CIMPullInstancesWithPathResponseMessage(
            messageId,
            CIMException(),
            queueIds.copyAndPop(),
            false,
            enumerationContext));
    response->syncAttributes(this);
    return response.release();
}

CIMResponseMessage* CIMPullInstancePathsRequestMessage::buildResponse() const
{
    AutoPtr<CIMPullInstancePathsResponseMessage> response(
        new CIMPullInstancePathsResponseMessage(
            messageId,
            CIMException(),
            queueIds.copyAndPop(),
            false,
            enumerationContext));
    response->syncAttributes(this);
    return response.release();
}

CIMResponseMessage*
    CIMPullInstancesRequestMessage::buildResponse() const
{
    AutoPtr<CIMPullInstancesResponseMessage> response(
        new CIMPullInstancesResponseMessage(
            messageId,
            CIMException(),
            queueIds.copyAndPop(),
            false,
            enumerationContext));
    response->syncAttributes(this);
    return response.release();
}

CIMResponseMessage* CIMCloseEnumerationRequestMessage::buildResponse() const
{
    AutoPtr<CIMCloseEnumerationResponseMessage> response(
        new CIMCloseEnumerationResponseMessage(
            messageId,
            CIMException(),
            queueIds.copyAndPop()));
    response->syncAttributes(this);
    return response.release();
}


CIMResponseMessage* CIMEnumerationCountRequestMessage::buildResponse() const
{
    AutoPtr<CIMEnumerationCountResponseMessage> response(
        new CIMEnumerationCountResponseMessage(
            messageId,
            CIMException(),
            queueIds.copyAndPop(),
            0));
    response->syncAttributes(this);
    return response.release();
}

CIMResponseMessage*
    CIMOpenQueryInstancesRequestMessage::buildResponse() const
{
    AutoPtr<CIMOpenQueryInstancesResponseMessage> response(
        new CIMOpenQueryInstancesResponseMessage(
            messageId,
            CIMException(),
            CIMClass(),           // queryResultClass
            queueIds.copyAndPop()));
    CIMResponseData & rspData = response->getResponseData();
    // KS_TODO Maybe not required. Wait for complete OpenQuery Implementation
    rspData.setRequestProperties(
        false,
        false,
        CIMPropertyList());
    response->syncAttributes(this);
    return response.release();
}
//EXP_PULL_END

CIMResponseMessage*
    CIMNotifySubscriptionNotActiveRequestMessage::buildResponse() const
{
    AutoPtr<CIMNotifySubscriptionNotActiveResponseMessage> response(
        new CIMNotifySubscriptionNotActiveResponseMessage(
            messageId,
            CIMException(),
            queueIds.copyAndPop()));
    response->syncAttributes(this);
    return response.release();
}

CIMResponseMessage*
    CIMNotifyListenerNotActiveRequestMessage::buildResponse() const
{
    AutoPtr<CIMNotifyListenerNotActiveResponseMessage> response(
        new CIMNotifyListenerNotActiveResponseMessage(
            messageId,
            CIMException(),
            queueIds.copyAndPop()));
    response->syncAttributes(this);
    return response.release();
}

CIMResponseMessage* ProvAgtGetScmoClassRequestMessage::buildResponse() const
{
    AutoPtr<ProvAgtGetScmoClassResponseMessage> response(
        new ProvAgtGetScmoClassResponseMessage(
            messageId,
            CIMException(),
            queueIds.copyAndPop(),
            SCMOClass("","")));
    response->syncAttributes(this);
    return response.release();
}

CIMMessage::CIMMessage(
    MessageType type,
    const String& messageId_)
    : Message(type),
      messageId(messageId_),
      _languageContextThreadId(Threads::self())
#ifndef PEGASUS_DISABLE_PERFINST
      ,_serverStartTimeMicroseconds(0),
      _providerTimeMicroseconds(0),
      _totalServerTimeMicroseconds(0)
#endif
{
    operationContext.insert(
        AcceptLanguageListContainer(AcceptLanguageList()));
    operationContext.insert(
        ContentLanguageListContainer(ContentLanguageList()));

    binaryRequest = false;
    binaryResponse = false;
    internalOperation = false;
}

#ifndef PEGASUS_DISABLE_PERFINST
void CIMMessage::endServer()
{
    PEGASUS_ASSERT(_serverStartTimeMicroseconds != 0);

    _totalServerTimeMicroseconds =
        TimeValue::getCurrentTime().toMicroseconds() -
            _serverStartTimeMicroseconds;

    Uint64 serverTimeMicroseconds =
        _totalServerTimeMicroseconds - _providerTimeMicroseconds;

    MessageType msgType = getType();
    StatisticalData::current()->addToValue(serverTimeMicroseconds,
        msgType,
        StatisticalData::PEGASUS_STATDATA_SERVER);

    StatisticalData::current()->addToValue(_providerTimeMicroseconds,
        msgType,
        StatisticalData::PEGASUS_STATDATA_PROVIDER);

    /* This adds the number of bytes read to the total.the request size
       value must be stored (requSize) and passed to the StatisticalData
       object at the end of processingm otherwise it will be the ONLY value
       that is passed to the client which reports the current state of the
       object, not the previous (one command ago) state */

    StatisticalData::current()->addToValue(
        StatisticalData::current()->requSize,
        msgType,
        StatisticalData::PEGASUS_STATDATA_BYTES_READ);
}
#endif

CIMRequestMessage::CIMRequestMessage(
    MessageType type_,
    const String& messageId_,
    const QueueIdStack& queueIds_)
    : CIMMessage(type_, messageId_), queueIds(queueIds_)
{
}

CIMResponseMessage::CIMResponseMessage(
    MessageType type_,
    const String& messageId_,
    const CIMException& cimException_,
    const QueueIdStack& queueIds_,
    Boolean isAsyncResponsePending_)
    :
    CIMMessage(type_, messageId_),
    queueIds(queueIds_),
    cimException(cimException_),
    isAsyncResponsePending(isAsyncResponsePending_)
{
}

CIMOperationRequestMessage::CIMOperationRequestMessage(
    MessageType type_,
    const String& messageId_,
    const QueueIdStack& queueIds_,
    const String& authType_,
    const String& userName_,
    const CIMNamespaceName& nameSpace_,
    const CIMName& className_,
    Uint32 providerType_)
    :
    CIMRequestMessage(type_, messageId_, queueIds_),
    authType(authType_),
    userName(userName_),
    nameSpace(nameSpace_),
    className(className_),
    providerType(providerType_)
{
}

CIMIndicationRequestMessage::CIMIndicationRequestMessage(
        MessageType type_,
        const String & messageId_,
        const QueueIdStack& queueIds_,
        const String& authType_,
        const String& userName_)
:
    CIMRequestMessage(type_, messageId_, queueIds_),
    authType(authType_),
    userName(userName_)
{
}

CIMGetClassRequestMessage::CIMGetClassRequestMessage(
        const String& messageId_,
        const CIMNamespaceName& nameSpace_,
        const CIMName& className_,
        Boolean localOnly_,
        Boolean includeQualifiers_,
        Boolean includeClassOrigin_,
        const CIMPropertyList& propertyList_,
        const QueueIdStack& queueIds_,
        const String& authType_,
        const String& userName_)
: CIMOperationRequestMessage(CIM_GET_CLASS_REQUEST_MESSAGE,
        messageId_, queueIds_,
        authType_, userName_,
        nameSpace_,className_,
        TYPE_CLASS),
    localOnly(localOnly_),
    includeQualifiers(includeQualifiers_),
    includeClassOrigin(includeClassOrigin_),
    propertyList(propertyList_)
{
}

CIMGetInstanceRequestMessage::CIMGetInstanceRequestMessage(
        const String& messageId_,
        const CIMNamespaceName& nameSpace_,
        const CIMObjectPath& instanceName_,
        Boolean includeQualifiers_,
        Boolean includeClassOrigin_,
        const CIMPropertyList& propertyList_,
        const QueueIdStack& queueIds_,
        const String& authType_,
        const String& userName_)
: CIMOperationRequestMessage(
        CIM_GET_INSTANCE_REQUEST_MESSAGE, messageId_, queueIds_,
        authType_, userName_,
        nameSpace_,instanceName_.getClassName()),
    instanceName(instanceName_),
    localOnly(false),
    includeQualifiers(includeQualifiers_),
    includeClassOrigin(includeClassOrigin_),
    propertyList(propertyList_)
{
}

CIMExportIndicationRequestMessage::CIMExportIndicationRequestMessage(
        const String& messageId_,
        const String& destinationPath_,
        const CIMInstance& indicationInstance_,
        const QueueIdStack& queueIds_,
        const String& authType_ ,
        const String& userName_ )
: CIMRequestMessage(
        CIM_EXPORT_INDICATION_REQUEST_MESSAGE, messageId_, queueIds_),
    destinationPath(destinationPath_),
    indicationInstance(indicationInstance_),
    authType(authType_),
    userName(userName_)
{
}

CIMDeleteClassRequestMessage::CIMDeleteClassRequestMessage(
        const String& messageId_,
        const CIMNamespaceName& nameSpace_,
        const CIMName& className_,
        const QueueIdStack& queueIds_,
        const String& authType_ ,
        const String& userName_ )
: CIMOperationRequestMessage(
        CIM_DELETE_CLASS_REQUEST_MESSAGE, messageId_, queueIds_,
        authType_, userName_,
        nameSpace_,className_,
        TYPE_CLASS)
{
}

CIMDeleteInstanceRequestMessage::CIMDeleteInstanceRequestMessage(
        const String& messageId_,
        const CIMNamespaceName& nameSpace_,
        const CIMObjectPath& instanceName_,
        const QueueIdStack& queueIds_,
        const String& authType_ ,
        const String& userName_ )
: CIMOperationRequestMessage(
        CIM_DELETE_INSTANCE_REQUEST_MESSAGE, messageId_, queueIds_,
        authType_, userName_,
        nameSpace_,instanceName_.getClassName()),
    instanceName(instanceName_)
{
}
CIMCreateClassRequestMessage::CIMCreateClassRequestMessage(
        const String& messageId_,
        const CIMNamespaceName& nameSpace_,
        const CIMClass& newClass_,
        const QueueIdStack& queueIds_,
        const String& authType_ ,
        const String& userName_ )
: CIMOperationRequestMessage(
        CIM_CREATE_CLASS_REQUEST_MESSAGE, messageId_, queueIds_,
        authType_, userName_,
        nameSpace_,newClass_.getClassName(),
        TYPE_CLASS),
    newClass(newClass_)
{
}
CIMCreateInstanceRequestMessage::CIMCreateInstanceRequestMessage(
        const String& messageId_,
        const CIMNamespaceName& nameSpace_,
        const CIMInstance& newInstance_,
        const QueueIdStack& queueIds_,
        const String& authType_ ,
        const String& userName_ )
: CIMOperationRequestMessage(
        CIM_CREATE_INSTANCE_REQUEST_MESSAGE, messageId_, queueIds_,
        authType_, userName_,
        nameSpace_,newInstance_.getClassName()),
    newInstance(newInstance_)
{
}
CIMModifyClassRequestMessage::CIMModifyClassRequestMessage(
        const String& messageId_,
        const CIMNamespaceName& nameSpace_,
        const CIMClass& modifiedClass_,
        const QueueIdStack& queueIds_,
        const String& authType_ ,
        const String& userName_ )
: CIMOperationRequestMessage(
        CIM_MODIFY_CLASS_REQUEST_MESSAGE, messageId_, queueIds_,
        authType_, userName_,
        nameSpace_,modifiedClass_.getClassName(),
        TYPE_CLASS),
    modifiedClass(modifiedClass_)
{
}
CIMModifyInstanceRequestMessage::CIMModifyInstanceRequestMessage(
        const String& messageId_,
        const CIMNamespaceName& nameSpace_,
        const CIMInstance& modifiedInstance_,
        Boolean includeQualifiers_,
        const CIMPropertyList& propertyList_,
        const QueueIdStack& queueIds_,
        const String& authType_ ,
        const String& userName_ )
: CIMOperationRequestMessage(
        CIM_MODIFY_INSTANCE_REQUEST_MESSAGE, messageId_, queueIds_,
        authType_, userName_,
        nameSpace_,modifiedInstance_.getClassName()),
    modifiedInstance(modifiedInstance_),
    includeQualifiers(includeQualifiers_),
    propertyList(propertyList_)
{
}
CIMEnumerateClassesRequestMessage::CIMEnumerateClassesRequestMessage(
        const String& messageId_,
        const CIMNamespaceName& nameSpace_,
        const CIMName& className_,
        Boolean deepInheritance_,
        Boolean localOnly_,
        Boolean includeQualifiers_,
        Boolean includeClassOrigin_,
        const QueueIdStack& queueIds_,
        const String& authType_ ,
        const String& userName_ )
: CIMOperationRequestMessage(
        CIM_ENUMERATE_CLASSES_REQUEST_MESSAGE, messageId_, queueIds_,
        authType_, userName_,
        nameSpace_,className_,
        TYPE_CLASS),
    deepInheritance(deepInheritance_),
    localOnly(localOnly_),
    includeQualifiers(includeQualifiers_),
    includeClassOrigin(includeClassOrigin_)
{
}

CIMEnumerateClassNamesRequestMessage::CIMEnumerateClassNamesRequestMessage(
        const String& messageId_,
        const CIMNamespaceName& nameSpace_,
        const CIMName& className_,
        Boolean deepInheritance_,
        const QueueIdStack& queueIds_,
        const String& authType_ ,
        const String& userName_ )
: CIMOperationRequestMessage(
        CIM_ENUMERATE_CLASS_NAMES_REQUEST_MESSAGE, messageId_, queueIds_,
        authType_, userName_,
        nameSpace_,className_,
        TYPE_CLASS),
    deepInheritance(deepInheritance_)
{
}

CIMEnumerateInstancesRequestMessage::CIMEnumerateInstancesRequestMessage(
        const String& messageId_,
        const CIMNamespaceName& nameSpace_,
        const CIMName& className_,
        Boolean deepInheritance_,
        Boolean includeQualifiers_,
        Boolean includeClassOrigin_,
        const CIMPropertyList& propertyList_,
        const QueueIdStack& queueIds_,
        const String& authType_ ,
        const String& userName_ )
: CIMOperationRequestMessage(
        CIM_ENUMERATE_INSTANCES_REQUEST_MESSAGE, messageId_, queueIds_,
        authType_, userName_,
        nameSpace_,className_),
    deepInheritance(deepInheritance_),
    localOnly(false),
    includeQualifiers(includeQualifiers_),
    includeClassOrigin(includeClassOrigin_),
    propertyList(propertyList_)
{
}

CIMEnumerateInstanceNamesRequestMessage::
CIMEnumerateInstanceNamesRequestMessage(
        const String& messageId_,
        const CIMNamespaceName& nameSpace_,
        const CIMName& className_,
        const QueueIdStack& queueIds_,
        const String& authType_ ,
        const String& userName_ )
: CIMOperationRequestMessage(
        CIM_ENUMERATE_INSTANCE_NAMES_REQUEST_MESSAGE, messageId_, queueIds_,
        authType_, userName_,
        nameSpace_, className_)
{
}

CIMExecQueryRequestMessage::CIMExecQueryRequestMessage(
        const String& messageId_,
        const CIMNamespaceName& nameSpace_,
        const String& queryLanguage_,
        const String& query_,
        const QueueIdStack& queueIds_,
        const String& authType_ ,
        const String& userName_ )
: CIMOperationRequestMessage(CIM_EXEC_QUERY_REQUEST_MESSAGE,
        messageId_, queueIds_,
        authType_, userName_,
        nameSpace_,CIMName(),
        TYPE_QUERY),
    queryLanguage(queryLanguage_),
    query(query_)
{
}

CIMAssociatorsRequestMessage::CIMAssociatorsRequestMessage(
        const String& messageId_,
        const CIMNamespaceName& nameSpace_,
        const CIMObjectPath& objectName_,
        const CIMName& assocClass_,
        const CIMName& resultClass_,
        const String& role_,
        const String& resultRole_,
        Boolean includeQualifiers_,
        Boolean includeClassOrigin_,
        const CIMPropertyList& propertyList_,
        const QueueIdStack& queueIds_,
        Boolean isClassRequest_,
        const String& authType_ ,
        const String& userName_ )
: CIMOperationRequestMessage(
        CIM_ASSOCIATORS_REQUEST_MESSAGE, messageId_, queueIds_,
        authType_, userName_,
        nameSpace_,objectName_.getClassName(),
        TYPE_ASSOCIATION),
    objectName(objectName_),
    assocClass(assocClass_),
    resultClass(resultClass_),
    role(role_),
    resultRole(resultRole_),
    includeQualifiers(includeQualifiers_),
    includeClassOrigin(includeClassOrigin_),
    propertyList(propertyList_),
    isClassRequest(isClassRequest_)
{
}

CIMAssociatorNamesRequestMessage::CIMAssociatorNamesRequestMessage(
        const String& messageId_,
        const CIMNamespaceName& nameSpace_,
        const CIMObjectPath& objectName_,
        const CIMName& assocClass_,
        const CIMName& resultClass_,
        const String& role_,
        const String& resultRole_,
        const QueueIdStack& queueIds_,
        Boolean isClassRequest_,
        const String& authType_ ,
        const String& userName_ )
: CIMOperationRequestMessage(
        CIM_ASSOCIATOR_NAMES_REQUEST_MESSAGE, messageId_, queueIds_,
        authType_, userName_,
        nameSpace_,objectName_.getClassName(),
        TYPE_ASSOCIATION),
    objectName(objectName_),
    assocClass(assocClass_),
    resultClass(resultClass_),
    role(role_),
    resultRole(resultRole_),
    isClassRequest(isClassRequest_)
{
}
CIMReferencesRequestMessage::CIMReferencesRequestMessage(
        const String& messageId_,
        const CIMNamespaceName& nameSpace_,
        const CIMObjectPath& objectName_,
        const CIMName& resultClass_,
        const String& role_,
        Boolean includeQualifiers_,
        Boolean includeClassOrigin_,
        const CIMPropertyList& propertyList_,
        const QueueIdStack& queueIds_,
        Boolean isClassRequest_,
        const String& authType_ ,
        const String& userName_ )
: CIMOperationRequestMessage(
        CIM_REFERENCES_REQUEST_MESSAGE, messageId_, queueIds_,
        authType_, userName_,
        nameSpace_,objectName_.getClassName(),
        TYPE_ASSOCIATION),
    objectName(objectName_),
    resultClass(resultClass_),
    role(role_),
    includeQualifiers(includeQualifiers_),
    includeClassOrigin(includeClassOrigin_),
    propertyList(propertyList_),
    isClassRequest(isClassRequest_)
{
}
CIMReferenceNamesRequestMessage::CIMReferenceNamesRequestMessage(
        const String& messageId_,
        const CIMNamespaceName& nameSpace_,
        const CIMObjectPath& objectName_,
        const CIMName& resultClass_,
        const String& role_,
        const QueueIdStack& queueIds_,
        Boolean isClassRequest_,
        const String& authType_ ,
        const String& userName_ )
: CIMOperationRequestMessage(
        CIM_REFERENCE_NAMES_REQUEST_MESSAGE, messageId_, queueIds_,
        authType_, userName_,
        nameSpace_,objectName_.getClassName(),
        TYPE_ASSOCIATION),
    objectName(objectName_),
    resultClass(resultClass_),
    role(role_),
    isClassRequest(isClassRequest_)
{
}

CIMGetPropertyRequestMessage::CIMGetPropertyRequestMessage(
        const String& messageId_,
        const CIMNamespaceName& nameSpace_,
        const CIMObjectPath& instanceName_,
        const CIMName& propertyName_,
        const QueueIdStack& queueIds_,
        const String& authType_ ,
        const String& userName_ )
: CIMOperationRequestMessage(
        CIM_GET_PROPERTY_REQUEST_MESSAGE, messageId_, queueIds_,
        authType_, userName_,
        nameSpace_,instanceName_.getClassName()),
    instanceName(instanceName_),
    propertyName(propertyName_)
{
}

CIMSetPropertyRequestMessage::CIMSetPropertyRequestMessage(
        const String& messageId_,
        const CIMNamespaceName& nameSpace_,
        const CIMObjectPath& instanceName_,
        const CIMName& propertyName_,
        const CIMValue& newValue_,
        const QueueIdStack& queueIds_,
        const String& authType_ ,
        const String& userName_ )
: CIMOperationRequestMessage(
        CIM_SET_PROPERTY_REQUEST_MESSAGE, messageId_, queueIds_,
        authType_, userName_,
        nameSpace_,instanceName_.getClassName()),
    instanceName(instanceName_),
    propertyName(propertyName_),
    newValue(newValue_)
{
}

CIMGetQualifierRequestMessage::CIMGetQualifierRequestMessage(
        const String& messageId_,
        const CIMNamespaceName& nameSpace_,
        const CIMName& qualifierName_,
        const QueueIdStack& queueIds_,
        const String& authType_ ,
        const String& userName_ )
: CIMOperationRequestMessage(
        CIM_GET_QUALIFIER_REQUEST_MESSAGE, messageId_, queueIds_,
        authType_, userName_,
        nameSpace_,CIMName(),
        TYPE_CLASS),
    qualifierName(qualifierName_)
{
}

CIMSetQualifierRequestMessage::CIMSetQualifierRequestMessage(
        const String& messageId_,
        const CIMNamespaceName& nameSpace_,
        const CIMQualifierDecl& qualifierDeclaration_,
        const QueueIdStack& queueIds_,
        const String& authType_ ,
        const String& userName_ )
: CIMOperationRequestMessage(
        CIM_SET_QUALIFIER_REQUEST_MESSAGE, messageId_, queueIds_,
        authType_, userName_,
        nameSpace_,CIMName(),
        TYPE_CLASS),
    qualifierDeclaration(qualifierDeclaration_)
{
}

CIMDeleteQualifierRequestMessage::CIMDeleteQualifierRequestMessage(
        const String& messageId_,
        const CIMNamespaceName& nameSpace_,
        const CIMName& qualifierName_,
        const QueueIdStack& queueIds_,
        const String& authType_ ,
        const String& userName_ )
: CIMOperationRequestMessage(
        CIM_DELETE_QUALIFIER_REQUEST_MESSAGE, messageId_, queueIds_,
        authType_, userName_,
        nameSpace_,CIMName(),
        TYPE_CLASS),
    qualifierName(qualifierName_)
{
}
CIMEnumerateQualifiersRequestMessage::CIMEnumerateQualifiersRequestMessage(
        const String& messageId_,
        const CIMNamespaceName& nameSpace_,
        const QueueIdStack& queueIds_,
        const String& authType_ ,
        const String& userName_ )
: CIMOperationRequestMessage(
        CIM_ENUMERATE_QUALIFIERS_REQUEST_MESSAGE, messageId_, queueIds_,
        authType_, userName_,
        nameSpace_,CIMName(),
        TYPE_CLASS)
{
}

CIMInvokeMethodRequestMessage::CIMInvokeMethodRequestMessage(
        const String& messageId_,
        const CIMNamespaceName& nameSpace_,
        const CIMObjectPath& instanceName_,
        const CIMName& methodName_,
        const Array<CIMParamValue>& inParameters_,
        const QueueIdStack& queueIds_,
        const String& authType_ ,
        const String& userName_ )
: CIMOperationRequestMessage(
        CIM_INVOKE_METHOD_REQUEST_MESSAGE, messageId_, queueIds_,
        authType_, userName_,
        nameSpace_,instanceName_.getClassName(),
        TYPE_METHOD),
    instanceName(instanceName_),
    methodName(methodName_),
    inParameters(inParameters_)
{
}

// EXP_PULL_BEGIN

// Open and Pull Message Constructors

CIMOpenOperationRequestMessage::CIMOpenOperationRequestMessage(
    MessageType type_,
    const String& messageId_,
    const CIMNamespaceName& nameSpace_,
    const CIMName& className,
    const String& filterQueryLanguage_,
    const String& filterQuery_,
    const Uint32Arg& operationTimeout_,
    Boolean continueOnError_,
    Uint32 maxObjectCount_,
    Uint32 providerType_,
    const QueueIdStack& queueIds_,
    const String& authType_,
    const String& userName_)
: CIMOperationRequestMessage(
        type_, messageId_, queueIds_, authType_, userName_,
        nameSpace_, className, providerType_),
    filterQueryLanguage(filterQueryLanguage_),
    filterQuery(filterQuery_),
    operationTimeout(operationTimeout_),
    continueOnError(continueOnError_),
    maxObjectCount(maxObjectCount_)
{
}

CIMOpenEnumerateInstancesRequestMessage::
CIMOpenEnumerateInstancesRequestMessage(
    const String& messageId_,
    const CIMNamespaceName& nameSpace_,
    const CIMName& className_,
    const Boolean deepInheritance_,
    const Boolean includeClassOrigin_,
    const CIMPropertyList& propertyList_,
    const String& filterQueryLanguage_,
    const String& filterQuery_,
    const Uint32Arg& operationTimeout_,
    Boolean continueOnError_,
    Uint32 maxObjectCount_,
    const QueueIdStack& queueIds_,
    const String& authType_,
    const String& userName_)
: CIMOpenOperationRequestMessage(
    CIM_OPEN_ENUMERATE_INSTANCES_REQUEST_MESSAGE, messageId_, nameSpace_,
    className_,
    filterQueryLanguage_,
    filterQuery_,
    operationTimeout_,
    continueOnError_,
    maxObjectCount_,
    TYPE_INSTANCE,
    queueIds_,authType_, userName_),

    deepInheritance(deepInheritance_),
    includeClassOrigin(includeClassOrigin_),
    propertyList(propertyList_)
{
}

CIMOpenEnumerateInstancePathsRequestMessage::
CIMOpenEnumerateInstancePathsRequestMessage(
    const String& messageId_,
    const CIMNamespaceName& nameSpace_,
    const CIMName& className_,
    const String& filterQueryLanguage_,
    const String& filterQuery_,
    const Uint32Arg& operationTimeout_,
    Boolean continueOnError_,
    Uint32 maxObjectCount_,
    const QueueIdStack& queueIds_,
    const String& authType_,
    const String& userName_)
: CIMOpenOperationRequestMessage(
    CIM_OPEN_ENUMERATE_INSTANCE_PATHS_REQUEST_MESSAGE, messageId_,
    nameSpace_,
    className_,
    filterQueryLanguage_,
    filterQuery_,
    operationTimeout_,
    continueOnError_,
    maxObjectCount_,
    TYPE_INSTANCE,
    queueIds_,authType_, userName_)
{
}

CIMOpenReferenceInstancesRequestMessage::
CIMOpenReferenceInstancesRequestMessage(
    const String& messageId_,
    const CIMNamespaceName& nameSpace_,
    const CIMObjectPath& objectName_,
    const CIMName& resultClass_,
    const String& role_,
    Boolean includeClassOrigin_,
    const CIMPropertyList& propertyList_,
    const String& filterQueryLanguage_,
    const String& filterQuery_,
    const Uint32Arg& operationTimeout_,
    Boolean continueOnError_,
    Uint32 maxObjectCount_,
    const QueueIdStack& queueIds_,
    const String& authType_,
    const String& userName_)
: CIMOpenOperationRequestMessage(
    CIM_OPEN_REFERENCE_INSTANCES_REQUEST_MESSAGE, messageId_, nameSpace_,
    objectName_.getClassName(), filterQueryLanguage_, filterQuery_,
    operationTimeout_, continueOnError_, maxObjectCount_,
    TYPE_ASSOCIATION, queueIds_,authType_, userName_),

    objectName(objectName_),
    resultClass(resultClass_),
    role(role_),
    includeClassOrigin(includeClassOrigin_),
    propertyList(propertyList_)
{
}

CIMOpenReferenceInstancePathsRequestMessage::
CIMOpenReferenceInstancePathsRequestMessage(
    const String& messageId_,
    const CIMNamespaceName& nameSpace_,
    const CIMObjectPath& objectName_,
    const CIMName& resultClass_,
    const String& role_,
    const String& filterQueryLanguage_,
    const String& filterQuery_,
    const Uint32Arg& operationTimeout_,
    Boolean continueOnError_,
    Uint32 maxObjectCount_,
    const QueueIdStack& queueIds_,
    const String& authType_,
    const String& userName_)
: CIMOpenOperationRequestMessage(
    CIM_OPEN_REFERENCE_INSTANCE_PATHS_REQUEST_MESSAGE, messageId_,
    nameSpace_, objectName_.getClassName(), filterQueryLanguage_,
    filterQuery_, operationTimeout_, continueOnError_, maxObjectCount_,
    TYPE_ASSOCIATION, queueIds_,authType_, userName_),

    objectName(objectName_),
    resultClass(resultClass_),
    role(role_)
{
}

CIMOpenAssociatorInstancesRequestMessage::
CIMOpenAssociatorInstancesRequestMessage(
    const String& messageId_,
    const CIMNamespaceName& nameSpace_,
    const CIMObjectPath& objectName_,
    const CIMName& assocClass_,
    const CIMName& resultClass_,
    const String& role_,
    const String& resultRole_,
    const Boolean includeClassOrigin_,
    const CIMPropertyList& propertyList_,
    const String& filterQueryLanguage_,
    const String& filterQuery_,
    const Uint32Arg& operationTimeout_,
    Boolean continueOnError_,
    Uint32 maxObjectCount_,
    const QueueIdStack& queueIds_,
    const String& authType_,
    const String& userName_)
: CIMOpenOperationRequestMessage(
    CIM_OPEN_ASSOCIATOR_INSTANCES_REQUEST_MESSAGE,
    messageId_, nameSpace_, objectName_.getClassName(),
    filterQueryLanguage_, filterQuery_, operationTimeout_, continueOnError_,
    maxObjectCount_, TYPE_ASSOCIATION, queueIds_,authType_, userName_),

    objectName(objectName_),
    assocClass(assocClass_),
    resultClass(resultClass_),
    role(role_),
    resultRole(resultRole_),
    includeClassOrigin(includeClassOrigin_),
    propertyList(propertyList_)
{
}

CIMOpenAssociatorInstancePathsRequestMessage::
CIMOpenAssociatorInstancePathsRequestMessage(
    const String& messageId_,
    const CIMNamespaceName& nameSpace_,
    const CIMObjectPath& objectName_,
    const CIMName& assocClass_,
    const CIMName& resultClass_,
    const String& role_,
    const String& resultRole_,
    const String& filterQueryLanguage_,
    const String& filterQuery_,
    const Uint32Arg& operationTimeout_,
    Boolean continueOnError_,
    Uint32 maxObjectCount_,
    const QueueIdStack& queueIds_,
    const String& authType_,
    const String& userName_)
: CIMOpenOperationRequestMessage(
    CIM_OPEN_ASSOCIATOR_INSTANCE_PATHS_REQUEST_MESSAGE,
    messageId_, nameSpace_, objectName_.getClassName(),
    filterQueryLanguage_, filterQuery_, operationTimeout_, continueOnError_,
    maxObjectCount_, TYPE_ASSOCIATION, queueIds_,authType_, userName_),

    objectName(objectName_),
    assocClass(assocClass_),
    resultClass(resultClass_),
    role(role_),
    resultRole(resultRole_)
{
}

CIMOpenQueryInstancesRequestMessage::
CIMOpenQueryInstancesRequestMessage(
    const String& messageId_,
    const CIMNamespaceName& nameSpace_,
    const String& queryLanguage_,
    const String& query_,
    Boolean returnQueryResultClass_,
    const Uint32Arg& operationTimeout_,
    Boolean continueOnError_,
    Uint32 maxObjectCount_,
    const QueueIdStack& queueIds_,
    const String& authType_,
    const String& userName_)
: CIMOpenOperationRequestMessage(
    CIM_OPEN_QUERY_INSTANCES_REQUEST_MESSAGE,
    messageId_, nameSpace_, CIMName(),
    queryLanguage_, query_, operationTimeout_, continueOnError_,
    maxObjectCount_, TYPE_QUERY, queueIds_,authType_, userName_),

    returnQueryResultClass(returnQueryResultClass_),
    queryLanguage(queryLanguage_),
    query(query_)
{
}

CIMPullOperationRequestMessage::CIMPullOperationRequestMessage(
    MessageType type_,
    const String& messageId_,
    const CIMNamespaceName& nameSpace_,
    const String& enumerationContext_,
    Uint32 maxObjectCount_,
    const QueueIdStack& queueIds_,
    const String& authType_,
    const String& userName_)
: CIMOperationRequestMessage(type_, messageId_, queueIds_, authType_,
    userName_, nameSpace_, CIMName()),

    enumerationContext(enumerationContext_),
    maxObjectCount(maxObjectCount_)
{
}

CIMPullInstancesWithPathRequestMessage::CIMPullInstancesWithPathRequestMessage(
    const String& messageId_,
    const CIMNamespaceName& nameSpace_,
    const String& enumerationContext_,
    Uint32 maxObjectCount_,
    const QueueIdStack& queueIds_,
    const String& authType_,
    const String& userName_)
: CIMPullOperationRequestMessage(
    CIM_PULL_INSTANCES_WITH_PATH_REQUEST_MESSAGE, messageId_, nameSpace_,
        enumerationContext_, maxObjectCount_, queueIds_,
        authType_, userName_)
{
}

CIMPullInstancePathsRequestMessage::CIMPullInstancePathsRequestMessage(
    const String& messageId_,
    const CIMNamespaceName& nameSpace_,
    const String& enumerationContext_,
    Uint32 maxObjectCount_,
    const QueueIdStack& queueIds_,
    const String& authType_,
    const String& userName_)
: CIMPullOperationRequestMessage(
    CIM_PULL_INSTANCE_PATHS_REQUEST_MESSAGE, messageId_, nameSpace_,
        enumerationContext_, maxObjectCount_, queueIds_,
        authType_, userName_)
{
}

CIMPullInstancesRequestMessage::CIMPullInstancesRequestMessage(
    const String& messageId_,
    const CIMNamespaceName& nameSpace_,
    const String& enumerationContext_,
    Uint32 maxObjectCount_,
    const QueueIdStack& queueIds_,
    const String& authType_,
    const String& userName_)
: CIMPullOperationRequestMessage(
    CIM_PULL_INSTANCES_REQUEST_MESSAGE, messageId_, nameSpace_,
        enumerationContext_, maxObjectCount_, queueIds_,
        authType_, userName_)
{
}

CIMCloseEnumerationRequestMessage::CIMCloseEnumerationRequestMessage(
    const String& messageId_,
    const CIMNamespaceName& nameSpace_,
    const String& enumerationContext_,
    const QueueIdStack& queueIds_,
    const String& authType_,
    const String& userName_)
: CIMOperationRequestMessage(
    CIM_CLOSE_ENUMERATION_REQUEST_MESSAGE, messageId_, queueIds_,
        authType_, userName_, nameSpace_, CIMName()),
    enumerationContext(enumerationContext_)
{
}

CIMEnumerationCountRequestMessage::CIMEnumerationCountRequestMessage(
    const String& messageId_,
    const CIMNamespaceName& nameSpace_,
    const String& enumerationContext_,
    const QueueIdStack& queueIds_,
    const String& authType_,
    const String& userName_)
: CIMOperationRequestMessage(
    CIM_ENUMERATION_COUNT_REQUEST_MESSAGE, messageId_, queueIds_,
        authType_, userName_,
        nameSpace_,CIMName()),
    enumerationContext(enumerationContext_)
{
}

// EXP_PULL_END

CIMProcessIndicationRequestMessage::CIMProcessIndicationRequestMessage(
        const String & messageId_,
        const CIMNamespaceName & nameSpace_,
        const CIMInstance& indicationInstance_,
        const Array<CIMObjectPath> & subscriptionInstanceNames_,
        const CIMInstance & provider_,
        const QueueIdStack& queueIds_,
        Uint32 timeoutMilliSec_,
        String oopAgentName_ )
: CIMRequestMessage(
        CIM_PROCESS_INDICATION_REQUEST_MESSAGE, messageId_, queueIds_),
    nameSpace (nameSpace_),
    indicationInstance(indicationInstance_),
    subscriptionInstanceNames(subscriptionInstanceNames_),
    provider(provider_),
    timeoutMilliSec(timeoutMilliSec_),
    oopAgentName(oopAgentName_)
{
}
CIMNotifyProviderRegistrationRequestMessage::
CIMNotifyProviderRegistrationRequestMessage(
        const String & messageId_,
        const Operation operation_,
        const CIMName & className_,
        const Array <CIMNamespaceName> & newNamespaces_,
        const Array <CIMNamespaceName> & oldNamespaces_,
        const CIMPropertyList & newPropertyNames_,
        const CIMPropertyList & oldPropertyNames_,
        const QueueIdStack& queueIds_)
: CIMRequestMessage(
        CIM_NOTIFY_PROVIDER_REGISTRATION_REQUEST_MESSAGE,
        messageId_, queueIds_),
    className (className_),
    newNamespaces (newNamespaces_),
    oldNamespaces (oldNamespaces_),
    newPropertyNames (newPropertyNames_),
    oldPropertyNames (oldPropertyNames_),
    operation(operation_)
{
}
CIMNotifyProviderTerminationRequestMessage::
CIMNotifyProviderTerminationRequestMessage(
        const String & messageId_,
        const Array <CIMInstance> & providers_,
        const QueueIdStack& queueIds_)
: CIMRequestMessage(
        CIM_NOTIFY_PROVIDER_TERMINATION_REQUEST_MESSAGE,
        messageId_, queueIds_),
    providers (providers_)
{
}

CIMHandleIndicationRequestMessage::CIMHandleIndicationRequestMessage(
        const String& messageId_,
        const CIMNamespaceName & nameSpace_,
        const CIMInstance& handlerInstance_,
        const CIMInstance& indicationInstance_,
        const CIMInstance& subscriptionInstance_,
        const QueueIdStack& queueIds_,
        const String& authType_ ,
        const String& userName_ )
: CIMRequestMessage(
        CIM_HANDLE_INDICATION_REQUEST_MESSAGE, messageId_, queueIds_),
    nameSpace(nameSpace_),
    handlerInstance(handlerInstance_),
    indicationInstance(indicationInstance_),
    subscriptionInstance(subscriptionInstance_),
    authType(authType_),
    userName(userName_),
    deliveryStatusAggregator(0)
{
}

CIMCreateSubscriptionRequestMessage::CIMCreateSubscriptionRequestMessage(
        const String& messageId_,
        const CIMNamespaceName & nameSpace_,
        const CIMInstance & subscriptionInstance_,
        const Array<CIMName> & classNames_,
        const CIMPropertyList & propertyList_,
        const Uint16 repeatNotificationPolicy_,
        const String & query_,
        const QueueIdStack& queueIds_,
        const String & authType_ ,
        const String & userName_ )
: CIMIndicationRequestMessage(
        CIM_CREATE_SUBSCRIPTION_REQUEST_MESSAGE,
        messageId_,
        queueIds_,
        authType_,
        userName_),
    nameSpace (nameSpace_),
    subscriptionInstance(subscriptionInstance_),
    classNames(classNames_),
    propertyList (propertyList_),
    repeatNotificationPolicy (repeatNotificationPolicy_),
    query (query_)
{
}
CIMModifySubscriptionRequestMessage::CIMModifySubscriptionRequestMessage(
        const String& messageId_,
        const CIMNamespaceName & nameSpace_,
        const CIMInstance & subscriptionInstance_,
        const Array<CIMName> & classNames_,
        const CIMPropertyList & propertyList_,
        const Uint16 repeatNotificationPolicy_,
        const String & query_,
        const QueueIdStack& queueIds_,
        const String & authType_ ,
        const String & userName_ )
: CIMIndicationRequestMessage(
        CIM_MODIFY_SUBSCRIPTION_REQUEST_MESSAGE,
        messageId_,
        queueIds_,
        authType_,
        userName_),
    nameSpace(nameSpace_),
    subscriptionInstance(subscriptionInstance_),
    classNames(classNames_),
    propertyList (propertyList_),
    repeatNotificationPolicy (repeatNotificationPolicy_),
    query (query_)
{
}

CIMDeleteSubscriptionRequestMessage::CIMDeleteSubscriptionRequestMessage(
        const String& messageId_,
        const CIMNamespaceName & nameSpace_,
        const CIMInstance & subscriptionInstance_,
        const Array<CIMName> & classNames_,
        const QueueIdStack& queueIds_,
        const String& authType_ ,
        const String& userName_ )
: CIMIndicationRequestMessage(
        CIM_DELETE_SUBSCRIPTION_REQUEST_MESSAGE,
        messageId_,
        queueIds_,
        authType_,
        userName_),
    nameSpace(nameSpace_),
    subscriptionInstance(subscriptionInstance_),
    classNames(classNames_)
{
}
CIMSubscriptionInitCompleteRequestMessage::
CIMSubscriptionInitCompleteRequestMessage(
        const String & messageId_,
        const QueueIdStack & queueIds_)
    : CIMRequestMessage
      (CIM_SUBSCRIPTION_INIT_COMPLETE_REQUEST_MESSAGE,
       messageId_,
       queueIds_)
{
}
CIMIndicationServiceDisabledRequestMessage::
CIMIndicationServiceDisabledRequestMessage(
        const String & messageId_,
        const QueueIdStack & queueIds_)
    : CIMRequestMessage
      (CIM_INDICATION_SERVICE_DISABLED_REQUEST_MESSAGE,
       messageId_,
       queueIds_)
{
}
CIMDisableModuleRequestMessage::
CIMDisableModuleRequestMessage(
        const String& messageId_,
        const CIMInstance& providerModule_,
        const Array<CIMInstance>& providers_,
        Boolean disableProviderOnly_,
        const Array<Boolean>& indicationProviders_,
        const QueueIdStack& queueIds_,
        const String& authType_ ,
        const String& userName_ )
: CIMRequestMessage(
        CIM_DISABLE_MODULE_REQUEST_MESSAGE,
        messageId_,
        queueIds_),
    providerModule(providerModule_),
    providers(providers_),
    disableProviderOnly(disableProviderOnly_),
    indicationProviders(indicationProviders_),
    authType(authType_),
    userName(userName_)
{
}
CIMEnableModuleRequestMessage::CIMEnableModuleRequestMessage(
        const String& messageId_,
        const CIMInstance& providerModule_,
        const QueueIdStack& queueIds_,
        const String& authType_ ,
        const String& userName_ )
: CIMRequestMessage(
        CIM_ENABLE_MODULE_REQUEST_MESSAGE,
        messageId_,
        queueIds_),
    providerModule(providerModule_),
    authType(authType_),
    userName(userName_)
{
}
CIMNotifyProviderEnableRequestMessage::CIMNotifyProviderEnableRequestMessage(
        const String & messageId_,
        const Array <CIMInstance> & capInstances_,
        const QueueIdStack& queueIds_)
: CIMRequestMessage(
        CIM_NOTIFY_PROVIDER_ENABLE_REQUEST_MESSAGE,
        messageId_,
        queueIds_),
    capInstances(capInstances_)
{
}
CIMNotifyProviderFailRequestMessage::CIMNotifyProviderFailRequestMessage(
        const String & messageId_,
        const String & moduleName_,
        const String & userName_,
        const QueueIdStack& queueIds_)
: CIMRequestMessage(
        CIM_NOTIFY_PROVIDER_FAIL_REQUEST_MESSAGE,
        messageId_,
        queueIds_),
    moduleName(moduleName_),
    userName(userName_)
{
}

CIMStopAllProvidersRequestMessage::CIMStopAllProvidersRequestMessage(
        const String& messageId_,
        const QueueIdStack& queueIds_,
        Uint32 shutdownTimeout_ )
: CIMRequestMessage(
        CIM_STOP_ALL_PROVIDERS_REQUEST_MESSAGE,
        messageId_,
        queueIds_),
    shutdownTimeout(shutdownTimeout_)
{
}
CIMInitializeProviderAgentRequestMessage::
CIMInitializeProviderAgentRequestMessage(
        const String & messageId_,
        const String& pegasusHome_,
        const Array<Pair<String, String> >& configProperties_,
        Boolean bindVerbose_,
        Boolean subscriptionInitComplete_,
        const QueueIdStack& queueIds_)
: CIMRequestMessage(
        CIM_INITIALIZE_PROVIDER_AGENT_REQUEST_MESSAGE,
        messageId_,
        queueIds_),
    pegasusHome(pegasusHome_),
    configProperties(configProperties_),
    bindVerbose(bindVerbose_),
    subscriptionInitComplete(subscriptionInitComplete_)
{
}

CIMNotifyConfigChangeRequestMessage::
CIMNotifyConfigChangeRequestMessage(
        const String & messageId_,
        const String & propertyName_,
        const String & newPropertyValue_,
        Boolean currentValueModified_, // false - planned value modified
        const QueueIdStack& queueIds_)
: CIMRequestMessage(
        CIM_NOTIFY_CONFIG_CHANGE_REQUEST_MESSAGE,
        messageId_,
        queueIds_),
    propertyName(propertyName_),
    newPropertyValue(newPropertyValue_),
    currentValueModified(currentValueModified_)
{
}
ProvAgtGetScmoClassRequestMessage::ProvAgtGetScmoClassRequestMessage(
        const String& messageId_,
        const CIMNamespaceName& nameSpace_,
        const CIMName& className_,
        const QueueIdStack& queueIds_)
: CIMRequestMessage(
        PROVAGT_GET_SCMOCLASS_REQUEST_MESSAGE,
        messageId_,
        queueIds_),
    nameSpace(nameSpace_),
    className(className_)
{
}

CIMNotifySubscriptionNotActiveRequestMessage::
CIMNotifySubscriptionNotActiveRequestMessage(
        const String & messageId_,
        const CIMObjectPath &subscriptionName_,
        const QueueIdStack& queueIds_)
: CIMRequestMessage(
        CIM_NOTIFY_SUBSCRIPTION_NOT_ACTIVE_REQUEST_MESSAGE,
        messageId_, queueIds_),
    subscriptionName(subscriptionName_)
{
}

CIMNotifyListenerNotActiveRequestMessage::
CIMNotifyListenerNotActiveRequestMessage(
        const String & messageId_,
        const CIMObjectPath &handlerName_,
        const QueueIdStack& queueIds_)
: CIMRequestMessage(
        CIM_NOTIFY_LISTENER_NOT_ACTIVE_REQUEST_MESSAGE,
        messageId_, queueIds_),
    handlerName(handlerName_)
{
}


CIMResponseDataMessage::CIMResponseDataMessage(
        MessageType type_,
        const String& messageId_,
        const CIMException& cimException_,
        const QueueIdStack& queueIds_,
        CIMResponseData::ResponseDataContent rspContent_,
        Boolean isAsyncResponsePending)
: CIMResponseMessage(
        type_,
        messageId_,
        cimException_,
        queueIds_,
        isAsyncResponsePending),
    _responseData(rspContent_)
{
}

CIMGetClassResponseMessage:: CIMGetClassResponseMessage(
        const String& messageId_,
        const CIMException& cimException_,
        const QueueIdStack& queueIds_,
        const CIMClass& cimClass_)
: CIMResponseMessage(CIM_GET_CLASS_RESPONSE_MESSAGE,
        messageId_, cimException_, queueIds_),
    cimClass(cimClass_)
{
}

CIMGetInstanceResponseMessage::CIMGetInstanceResponseMessage(
        const String& messageId_,
        const CIMException& cimException_,
        const QueueIdStack& queueIds_)
: CIMResponseDataMessage(CIM_GET_INSTANCE_RESPONSE_MESSAGE,
        messageId_, cimException_, queueIds_, CIMResponseData::RESP_INSTANCE)
{
}

CIMExportIndicationResponseMessage::CIMExportIndicationResponseMessage(
        const String& messageId_,
        const CIMException& cimException_,
        const QueueIdStack& queueIds_)
: CIMResponseMessage(CIM_EXPORT_INDICATION_RESPONSE_MESSAGE,
        messageId_, cimException_, queueIds_)
{
}
CIMDeleteClassResponseMessage::CIMDeleteClassResponseMessage(
        const String& messageId_,
        const CIMException& cimException_,
        const QueueIdStack& queueIds_)
: CIMResponseMessage(CIM_DELETE_CLASS_RESPONSE_MESSAGE,
        messageId_, cimException_, queueIds_)
{
}

CIMDeleteInstanceResponseMessage::CIMDeleteInstanceResponseMessage(
        const String& messageId_,
        const CIMException& cimException_,
        const QueueIdStack& queueIds_)
: CIMResponseMessage(CIM_DELETE_INSTANCE_RESPONSE_MESSAGE,
        messageId_, cimException_, queueIds_)
{
}
CIMCreateClassResponseMessage::CIMCreateClassResponseMessage(
        const String& messageId_,
        const CIMException& cimException_,
        const QueueIdStack& queueIds_)
: CIMResponseMessage(CIM_CREATE_CLASS_RESPONSE_MESSAGE,
        messageId_, cimException_, queueIds_)
{
}

CIMCreateInstanceResponseMessage::CIMCreateInstanceResponseMessage(
        const String& messageId_,
        const CIMException& cimException_,
        const QueueIdStack& queueIds_,
        const CIMObjectPath& instanceName_)
: CIMResponseMessage(CIM_CREATE_INSTANCE_RESPONSE_MESSAGE,
        messageId_, cimException_, queueIds_),
    instanceName(instanceName_)
{
}

CIMModifyClassResponseMessage::CIMModifyClassResponseMessage(
        const String& messageId_,
        const CIMException& cimException_,
        const QueueIdStack& queueIds_)
: CIMResponseMessage(CIM_MODIFY_CLASS_RESPONSE_MESSAGE,
        messageId_, cimException_, queueIds_)
{
}

CIMModifyInstanceResponseMessage::CIMModifyInstanceResponseMessage(
        const String& messageId_,
        const CIMException& cimException_,
        const QueueIdStack& queueIds_)
: CIMResponseMessage(CIM_MODIFY_INSTANCE_RESPONSE_MESSAGE,
        messageId_, cimException_, queueIds_)
{
}
CIMEnumerateClassesResponseMessage::CIMEnumerateClassesResponseMessage(
        const String& messageId_,
        const CIMException& cimException_,
        const QueueIdStack& queueIds_,
        const Array<CIMClass>& cimClasses_)
: CIMResponseMessage(CIM_ENUMERATE_CLASSES_RESPONSE_MESSAGE,
        messageId_, cimException_, queueIds_),
    cimClasses(cimClasses_)
{
}

CIMEnumerateClassNamesResponseMessage::CIMEnumerateClassNamesResponseMessage(
        const String& messageId_,
        const CIMException& cimException_,
        const QueueIdStack& queueIds_,
        const Array<CIMName>& classNames_)
: CIMResponseMessage(CIM_ENUMERATE_CLASS_NAMES_RESPONSE_MESSAGE,
        messageId_, cimException_, queueIds_),
    classNames(classNames_)
{
}
CIMEnumerateInstancesResponseMessage::CIMEnumerateInstancesResponseMessage(
        const String& messageId_,
        const CIMException& cimException_,
        const QueueIdStack& queueIds_)
: CIMResponseDataMessage(
        CIM_ENUMERATE_INSTANCES_RESPONSE_MESSAGE,
        messageId_, cimException_, queueIds_, CIMResponseData::RESP_INSTANCES)
{
}
CIMEnumerateInstanceNamesResponseMessage::
CIMEnumerateInstanceNamesResponseMessage(
        const String& messageId_,
        const CIMException& cimException_,
        const QueueIdStack& queueIds_)
: CIMResponseDataMessage(CIM_ENUMERATE_INSTANCE_NAMES_RESPONSE_MESSAGE,
        messageId_, cimException_, queueIds_, CIMResponseData::RESP_INSTNAMES)
{
}

CIMExecQueryResponseMessage::CIMExecQueryResponseMessage(
        const String& messageId_,
        const CIMException& cimException_,
        const QueueIdStack& queueIds_)
: CIMResponseDataMessage(CIM_EXEC_QUERY_RESPONSE_MESSAGE,
        messageId_, cimException_, queueIds_, CIMResponseData::RESP_OBJECTS)
{
}
CIMAssociatorsResponseMessage::CIMAssociatorsResponseMessage(
        const String& messageId_,
        const CIMException& cimException_,
        const QueueIdStack& queueIds_)
: CIMResponseDataMessage(CIM_ASSOCIATORS_RESPONSE_MESSAGE,
        messageId_, cimException_, queueIds_, CIMResponseData::RESP_OBJECTS)
{
}

CIMAssociatorNamesResponseMessage::CIMAssociatorNamesResponseMessage(
        const String& messageId_,
        const CIMException& cimException_,
        const QueueIdStack& queueIds_)
: CIMResponseDataMessage(CIM_ASSOCIATOR_NAMES_RESPONSE_MESSAGE,
        messageId_, cimException_, queueIds_, CIMResponseData::RESP_OBJECTPATHS)
{
}

CIMReferencesResponseMessage::CIMReferencesResponseMessage(
        const String& messageId_,
        const CIMException& cimException_,
        const QueueIdStack& queueIds_)
: CIMResponseDataMessage(CIM_REFERENCES_RESPONSE_MESSAGE,
        messageId_, cimException_, queueIds_, CIMResponseData::RESP_OBJECTS)
{
}

CIMReferenceNamesResponseMessage::CIMReferenceNamesResponseMessage(
        const String& messageId_,
        const CIMException& cimException_,
        const QueueIdStack& queueIds_)
: CIMResponseDataMessage(CIM_REFERENCE_NAMES_RESPONSE_MESSAGE,
        messageId_, cimException_, queueIds_, CIMResponseData::RESP_OBJECTPATHS)
{
}

CIMGetPropertyResponseMessage::CIMGetPropertyResponseMessage(
        const String& messageId_,
        const CIMException& cimException_,
        const QueueIdStack& queueIds_,
        const CIMValue& value_)
: CIMResponseMessage(CIM_GET_PROPERTY_RESPONSE_MESSAGE,
        messageId_, cimException_, queueIds_),
    value(value_)
{
}

CIMSetPropertyResponseMessage::CIMSetPropertyResponseMessage(
        const String& messageId_,
        const CIMException& cimException_,
        const QueueIdStack& queueIds_)
: CIMResponseMessage(CIM_SET_PROPERTY_RESPONSE_MESSAGE,
        messageId_, cimException_, queueIds_)
{
}

CIMGetQualifierResponseMessage::CIMGetQualifierResponseMessage(
        const String& messageId_,
        const CIMException& cimException_,
        const QueueIdStack& queueIds_,
        const CIMQualifierDecl& cimQualifierDecl_)
: CIMResponseMessage(CIM_GET_QUALIFIER_RESPONSE_MESSAGE,
        messageId_, cimException_, queueIds_),
    cimQualifierDecl(cimQualifierDecl_)
{
}

CIMSetQualifierResponseMessage::CIMSetQualifierResponseMessage(
        const String& messageId_,
        const CIMException& cimException_,
        const QueueIdStack& queueIds_)
:
    CIMResponseMessage(CIM_SET_QUALIFIER_RESPONSE_MESSAGE,
            messageId_, cimException_, queueIds_)
{
}

CIMDeleteQualifierResponseMessage::CIMDeleteQualifierResponseMessage(
        const String& messageId_,
        const CIMException& cimException_,
        const QueueIdStack& queueIds_)
:
    CIMResponseMessage(CIM_DELETE_QUALIFIER_RESPONSE_MESSAGE,
            messageId_, cimException_, queueIds_)
{
}

CIMEnumerateQualifiersResponseMessage::CIMEnumerateQualifiersResponseMessage(
        const String& messageId_,
        const CIMException& cimException_,
        const QueueIdStack& queueIds_,
        const Array<CIMQualifierDecl>& qualifierDeclarations_)
: CIMResponseMessage(CIM_ENUMERATE_QUALIFIERS_RESPONSE_MESSAGE,
        messageId_, cimException_, queueIds_),
    qualifierDeclarations(qualifierDeclarations_)
{
}

CIMInvokeMethodResponseMessage::CIMInvokeMethodResponseMessage(
        const String& messageId_,
        const CIMException& cimException_,
        const QueueIdStack& queueIds_,
        const CIMValue& retValue_,
        const Array<CIMParamValue>& outParameters_,
        const CIMName& methodName_)
: CIMResponseMessage(CIM_INVOKE_METHOD_RESPONSE_MESSAGE,
        messageId_, cimException_, queueIds_),
    retValue(retValue_),
    outParameters(outParameters_),
    methodName(methodName_)
{
}

CIMProcessIndicationResponseMessage::CIMProcessIndicationResponseMessage(
        const String& messageId_,
        const CIMException& cimException_,
        const QueueIdStack& queueIds_,
        String oopAgentName_,
        CIMInstance subscription_)
: CIMResponseMessage(CIM_PROCESS_INDICATION_RESPONSE_MESSAGE,
        messageId_, cimException_, queueIds_),
    oopAgentName(oopAgentName_),
    subscription(subscription_)
{
}

CIMNotifyProviderRegistrationResponseMessage::
CIMNotifyProviderRegistrationResponseMessage(
        const String& messageId_,
        const CIMException& cimException_,
        const QueueIdStack& queueIds_)
: CIMResponseMessage(CIM_NOTIFY_PROVIDER_REGISTRATION_RESPONSE_MESSAGE,
        messageId_, cimException_, queueIds_)
{
}

CIMNotifyProviderTerminationResponseMessage::
CIMNotifyProviderTerminationResponseMessage(
        const String& messageId_,
        const CIMException& cimException_,
        const QueueIdStack& queueIds_)
: CIMResponseMessage(CIM_NOTIFY_PROVIDER_TERMINATION_RESPONSE_MESSAGE,
        messageId_, cimException_, queueIds_)
{
}

CIMHandleIndicationResponseMessage::CIMHandleIndicationResponseMessage(
        const String& messageId_,
        const CIMException& cimException_,
        const QueueIdStack& queueIds_)
: CIMResponseMessage(CIM_HANDLE_INDICATION_RESPONSE_MESSAGE,
        messageId_, cimException_, queueIds_)
{
}

CIMCreateSubscriptionResponseMessage::CIMCreateSubscriptionResponseMessage(
        const String& messageId_,
        const CIMException& cimException_,
        const QueueIdStack& queueIds_)
: CIMResponseMessage(
        CIM_CREATE_SUBSCRIPTION_RESPONSE_MESSAGE,
        messageId_,
        cimException_,
        queueIds_)
{
}

CIMModifySubscriptionResponseMessage::CIMModifySubscriptionResponseMessage(
        const String& messageId_,
        const CIMException& cimException_,
        const QueueIdStack& queueIds_)
: CIMResponseMessage(
        CIM_MODIFY_SUBSCRIPTION_RESPONSE_MESSAGE,
        messageId_,
        cimException_,
        queueIds_)
{
}

CIMDeleteSubscriptionResponseMessage::CIMDeleteSubscriptionResponseMessage(
        const String& messageId_,
        const CIMException& cimException_,
        const QueueIdStack& queueIds_)
: CIMResponseMessage(
        CIM_DELETE_SUBSCRIPTION_RESPONSE_MESSAGE,
        messageId_,
        cimException_,
        queueIds_)
{
}

CIMSubscriptionInitCompleteResponseMessage::
    CIMSubscriptionInitCompleteResponseMessage
(const String & messageId_,
 const CIMException & cimException_,
 const QueueIdStack & queueIds_)
    : CIMResponseMessage
      (CIM_SUBSCRIPTION_INIT_COMPLETE_RESPONSE_MESSAGE,
       messageId_,
       cimException_,
       queueIds_)
{
}

CIMIndicationServiceDisabledResponseMessage::
    CIMIndicationServiceDisabledResponseMessage
(const String & messageId_,
 const CIMException & cimException_,
 const QueueIdStack & queueIds_)
    : CIMResponseMessage
      (CIM_INDICATION_SERVICE_DISABLED_RESPONSE_MESSAGE,
       messageId_,
       cimException_,
       queueIds_)
{
}

CIMDisableModuleResponseMessage::CIMDisableModuleResponseMessage(
        const String& messageId_,
        const CIMException& cimException_,
        const QueueIdStack& queueIds_,
        const Array<Uint16>& operationalStatus_)
: CIMResponseMessage(CIM_DISABLE_MODULE_RESPONSE_MESSAGE,
        messageId_, cimException_, queueIds_),
    operationalStatus(operationalStatus_)
{
}

CIMEnableModuleResponseMessage::CIMEnableModuleResponseMessage(
        const String& messageId_,
        const CIMException& cimException_,
        const QueueIdStack& queueIds_,
        const Array<Uint16>& operationalStatus_)
: CIMResponseMessage(CIM_ENABLE_MODULE_RESPONSE_MESSAGE,
        messageId_, cimException_, queueIds_),
    operationalStatus(operationalStatus_)
{
}

CIMNotifyProviderEnableResponseMessage::CIMNotifyProviderEnableResponseMessage(
        const String& messageId_,
        const CIMException& cimException_,
        const QueueIdStack& queueIds_)
: CIMResponseMessage(CIM_NOTIFY_PROVIDER_ENABLE_RESPONSE_MESSAGE,
        messageId_, cimException_, queueIds_)
{
}

CIMNotifyProviderFailResponseMessage::CIMNotifyProviderFailResponseMessage(
        const String& messageId_,
        const CIMException& cimException_,
        const QueueIdStack& queueIds_)
: CIMResponseMessage(CIM_NOTIFY_PROVIDER_FAIL_RESPONSE_MESSAGE,
        messageId_, cimException_, queueIds_)
{
}

CIMStopAllProvidersResponseMessage::CIMStopAllProvidersResponseMessage(
        const String& messageId_,
        const CIMException& cimException_,
        const QueueIdStack& queueIds_)
: CIMResponseMessage(CIM_STOP_ALL_PROVIDERS_RESPONSE_MESSAGE,
        messageId_, cimException_, queueIds_)
{
}

CIMInitializeProviderAgentResponseMessage::
    CIMInitializeProviderAgentResponseMessage(
        const String& messageId_,
        const CIMException& cimException_,
        const QueueIdStack& queueIds_)
: CIMResponseMessage(CIM_INITIALIZE_PROVIDER_AGENT_RESPONSE_MESSAGE,
        messageId_, cimException_, queueIds_)
{
}

CIMNotifyConfigChangeResponseMessage::CIMNotifyConfigChangeResponseMessage(
        const String& messageId_,
        const CIMException& cimException_,
        const QueueIdStack& queueIds_)
    : CIMResponseMessage(CIM_NOTIFY_CONFIG_CHANGE_RESPONSE_MESSAGE,
        messageId_, cimException_, queueIds_)
{
}

ProvAgtGetScmoClassResponseMessage::ProvAgtGetScmoClassResponseMessage(
        const String& messageId_,
        const CIMException& cimException_,
        const QueueIdStack& queueIds_,
        const SCMOClass& scmoClass_)
: CIMResponseMessage(PROVAGT_GET_SCMOCLASS_RESPONSE_MESSAGE,
        messageId_, cimException_, queueIds_),
    scmoClass(scmoClass_)
{
}

CIMNotifySubscriptionNotActiveResponseMessage::
    CIMNotifySubscriptionNotActiveResponseMessage(
        const String& messageId_,
        const CIMException& cimException_,
        const QueueIdStack& queueIds_)
    : CIMResponseMessage(CIM_NOTIFY_SUBSCRIPTION_NOT_ACTIVE_RESPONSE_MESSAGE,
        messageId_, cimException_, queueIds_)
{
}

CIMNotifyListenerNotActiveResponseMessage::
    CIMNotifyListenerNotActiveResponseMessage(
        const String& messageId_,
        const CIMException& cimException_,
        const QueueIdStack& queueIds_)
    : CIMResponseMessage(CIM_NOTIFY_LISTENER_NOT_ACTIVE_RESPONSE_MESSAGE,
        messageId_, cimException_, queueIds_)
{
}

// EXP_PULL_BEGIN
CIMOpenOrPullResponseDataMessage::CIMOpenOrPullResponseDataMessage(
    MessageType type_,
    const String& messageId_,
    const CIMException& cimException_,
    const QueueIdStack& queueIds_,
    CIMResponseData::ResponseDataContent rspContent_,
    const Boolean endOfSequence_,
    const String& enumerationContext_
    )
: CIMResponseDataMessage(type_,
    messageId_, cimException_, queueIds_, rspContent_, isAsyncResponsePending),
    endOfSequence(endOfSequence_),
    enumerationContext(enumerationContext_)
{
}

CIMOpenEnumerateInstancesResponseMessage::
    CIMOpenEnumerateInstancesResponseMessage(
        const String& messageId_,
        const CIMException& cimException_,
        const QueueIdStack& queueIds_,
        const Boolean endOfSequence_,
        const String& enumerationContext_
        )
    : CIMOpenOrPullResponseDataMessage(
        CIM_OPEN_ENUMERATE_INSTANCES_RESPONSE_MESSAGE,
        messageId_, cimException_, queueIds_, CIMResponseData::RESP_INSTANCES,
        endOfSequence_, enumerationContext_)
    {
    }

CIMOpenEnumerateInstancePathsResponseMessage::
    CIMOpenEnumerateInstancePathsResponseMessage(
        const String& messageId_,
        const CIMException& cimException_,
        const QueueIdStack& queueIds_,
        const Boolean endOfSequence_,
        const String& enumerationContext_
        )
    : CIMOpenOrPullResponseDataMessage(
        CIM_OPEN_ENUMERATE_INSTANCE_PATHS_RESPONSE_MESSAGE,
        messageId_, cimException_, queueIds_,CIMResponseData::RESP_INSTNAMES,
        endOfSequence_, enumerationContext_)
    {
    }

CIMOpenReferenceInstancesResponseMessage::
    CIMOpenReferenceInstancesResponseMessage(
        const String& messageId_,
        const CIMException& cimException_,
        const QueueIdStack& queueIds_,
        const Boolean endOfSequence_,
        const String& enumerationContext_
        )
    : CIMOpenOrPullResponseDataMessage(
        CIM_OPEN_REFERENCE_INSTANCES_RESPONSE_MESSAGE,
        messageId_, cimException_, queueIds_, CIMResponseData::RESP_OBJECTS,
        endOfSequence_, enumerationContext_)
    {
    }

CIMOpenReferenceInstancePathsResponseMessage::
    CIMOpenReferenceInstancePathsResponseMessage(
        const String& messageId_,
        const CIMException& cimException_,
        const QueueIdStack& queueIds_,
        const Boolean endOfSequence_,
        const String& enumerationContext_
        )
    : CIMOpenOrPullResponseDataMessage(
        CIM_OPEN_REFERENCE_INSTANCE_PATHS_RESPONSE_MESSAGE,
        messageId_, cimException_, queueIds_,CIMResponseData::RESP_OBJECTPATHS,
        endOfSequence_, enumerationContext_)
    {
    }

CIMOpenAssociatorInstancesResponseMessage::
    CIMOpenAssociatorInstancesResponseMessage(
        const String& messageId_,
        const CIMException& cimException_,
        const QueueIdStack& queueIds_,
        const Boolean endOfSequence_,
        const String& enumerationContext_
        )
    : CIMOpenOrPullResponseDataMessage(
        CIM_OPEN_ASSOCIATOR_INSTANCES_RESPONSE_MESSAGE,
        messageId_, cimException_, queueIds_,CIMResponseData::RESP_OBJECTS,
        endOfSequence_, enumerationContext_)
    {
    }

CIMOpenAssociatorInstancePathsResponseMessage::
    CIMOpenAssociatorInstancePathsResponseMessage(
        const String& messageId_,
        const CIMException& cimException_,
        const QueueIdStack& queueIds_,
        const Boolean endOfSequence_ ,
        const String& enumerationContext_
        )
    :CIMOpenOrPullResponseDataMessage(
        CIM_OPEN_ASSOCIATOR_INSTANCE_PATHS_RESPONSE_MESSAGE,
        messageId_, cimException_, queueIds_,CIMResponseData::RESP_OBJECTPATHS,
        endOfSequence_, enumerationContext_)
    {
    }

CIMPullInstancesWithPathResponseMessage::
    CIMPullInstancesWithPathResponseMessage(
        const String& messageId_,
        const CIMException& cimException_,
        const QueueIdStack& queueIds_,
        const Boolean endOfSequence_,
        const String& enumerationContext_
        )
    : CIMOpenOrPullResponseDataMessage(
        CIM_PULL_INSTANCES_WITH_PATH_RESPONSE_MESSAGE,
        messageId_, cimException_, queueIds_,CIMResponseData::RESP_INSTANCES,
        endOfSequence_, enumerationContext_)
    {
    }

CIMPullInstancePathsResponseMessage::
    CIMPullInstancePathsResponseMessage(
        const String& messageId_,
        const CIMException& cimException_,
        const QueueIdStack& queueIds_,
        const Boolean endOfSequence_,
        const String& enumerationContext_)
    : CIMOpenOrPullResponseDataMessage(CIM_PULL_INSTANCE_PATHS_RESPONSE_MESSAGE,
        messageId_, cimException_, queueIds_,CIMResponseData::RESP_OBJECTPATHS,
        endOfSequence_, enumerationContext_)
    {
    }

CIMPullInstancesResponseMessage::
    CIMPullInstancesResponseMessage(
        const String& messageId_,
        const CIMException& cimException_,
        const QueueIdStack& queueIds_,
        const Boolean endOfSequence_,
        const String& enumerationContext_
        )
    : CIMOpenOrPullResponseDataMessage(CIM_PULL_INSTANCES_RESPONSE_MESSAGE,
        messageId_, cimException_, queueIds_,CIMResponseData::RESP_INSTANCES,
        endOfSequence_, enumerationContext_)
    {
    }

CIMCloseEnumerationResponseMessage::
    CIMCloseEnumerationResponseMessage(
        const String& messageId_,
        const CIMException& cimException_,
        const QueueIdStack& queueIds_)
    : CIMResponseMessage(CIM_CLOSE_ENUMERATION_RESPONSE_MESSAGE,
        messageId_, cimException_, queueIds_)
    {
    }

CIMOpenQueryInstancesResponseMessage::
    CIMOpenQueryInstancesResponseMessage(
        const String& messageId_,
        const CIMException& cimException_,
        const CIMClass& queryResultClass_,
        const QueueIdStack& queueIds_,
        Boolean endOfSequence_,
        const String& enumerationContext_)
    : CIMOpenOrPullResponseDataMessage(
        CIM_OPEN_QUERY_INSTANCES_RESPONSE_MESSAGE,
        messageId_, cimException_, queueIds_,CIMResponseData::RESP_INSTANCES,
        endOfSequence_, enumerationContext_),
        queryResultClass(queryResultClass_)
    {
    }

CIMEnumerationCountResponseMessage::
    CIMEnumerationCountResponseMessage(
        const String& messageId_,
        const CIMException& cimException_,
        const QueueIdStack& queueIds_,
        const Uint64Arg& count_ )
    : CIMResponseMessage(CIM_ENUMERATION_COUNT_RESPONSE_MESSAGE,
        messageId_, cimException_, queueIds_),
        count(count_)
    {
    }

//EXP_PULL_END

PEGASUS_NAMESPACE_END
