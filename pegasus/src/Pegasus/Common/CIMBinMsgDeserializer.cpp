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

#include <Pegasus/Common/XmlReader.h>
#include <Pegasus/Common/OperationContextInternal.h>
#include <Pegasus/Common/System.h>
#include <Pegasus/Common/Tracer.h>

#include "CIMBinMsgDeserializer.h"

PEGASUS_NAMESPACE_BEGIN

CIMMessage* CIMBinMsgDeserializer::deserialize(
    CIMBuffer& in,
    size_t size)
{
    PEG_METHOD_ENTER(TRC_DISPATCHER, "CIMBinMsgDeserializer::deserialize");

    if (size == 0)
        return 0;

    CIMMessage* msg = 0;
    String typeString;
    OperationContext operationContext;
    bool present;

    // [messageId]

    String messageID;

    if (!in.getString(messageID))
        return 0;

    // [binaryRequest]

    Boolean binaryRequest;

    if (!in.getBoolean(binaryRequest))
        return 0;

    // [binaryResponse]

    Boolean binaryResponse;

    if (!in.getBoolean(binaryResponse))
        return 0;

    Boolean internalOperation;

    if (!in.getBoolean(internalOperation))
        return 0;
    // [type]

    MessageType type;
    {
        Uint32 tmp;

        if (!in.getUint32(tmp))
            return 0;

        type = MessageType(tmp);
    }

#ifndef PEGASUS_DISABLE_PERFINST

    // [serverStartTimeMicroseconds]
    Uint64 serverStartTimeMicroseconds;

    if (!in.getUint64(serverStartTimeMicroseconds))
        return 0;

    // [providerTimeMicroseconds]
    Uint64 providerTimeMicroseconds;

    if (!in.getUint64(providerTimeMicroseconds))
        return 0;

#endif

    // [isComplete]

    Boolean isComplete;

    if (!in.getBoolean(isComplete))
        return 0;

    // [index]

    Uint32 index;

    if (!in.getUint32(index))
        return 0;

    // [operationContext]

    if (!_getOperationContext(in, operationContext))
        return 0;

    // [CIMRequestMessage]

    if (!in.getPresent(present))
        return 0;

    if (present)
    {
        if (!(msg = _getRequestMessage(in, type)))
            return 0;
    }

    // [CIMResponseMessage]

    if (!in.getPresent(present))
        return 0;

    if (present)
    {
        if (!(msg = _getResponseMessage(in, type, binaryResponse)))
            return 0;
    }

    // Initialize the messge:

    msg->messageId = messageID;
    msg->binaryRequest = binaryRequest;
    msg->binaryResponse = binaryResponse;
#ifndef PEGASUS_DISABLE_PERFINST
    msg->setServerStartTime(serverStartTimeMicroseconds);
    msg->setProviderTime(providerTimeMicroseconds);
#endif
    msg->setComplete(isComplete);
    msg->setIndex(index);
    msg->operationContext = operationContext;
    msg->internalOperation = internalOperation;

    PEG_TRACE((TRC_DISPATCHER,  Tracer::LEVEL4,
        "Deserialize MessageId=%s type=%s binaryReq=%s"
                       " binaryResp=%s iscomplete=%s internal=%s",
        (const char*)msg->messageId.getCString(),
        MessageTypeToString(msg->getType()),
        boolToString(msg->binaryRequest),
        boolToString(msg->binaryResponse),
        boolToString(msg->isComplete()),
        boolToString(msg->internalOperation)
        ));

    PEGASUS_DEBUG_ASSERT(msg->valid());
    PEG_METHOD_EXIT();
    return msg;
}

CIMRequestMessage* CIMBinMsgDeserializer::_getRequestMessage(
    CIMBuffer& in,
    MessageType type)
{
    CIMRequestMessage* msg = 0;
    XmlEntry entry;
    QueueIdStack queueIdStack;
    Boolean present;

    // [queueIdStack]

    _getQueueIdStack(in, queueIdStack);

    // [CIMOperationRequestMessage]

    if (!in.getPresent(present))
        return 0;

    if (present)
    {
        // [userInfo]

        String authType;
        String userName;

        if (!_getUserInfo(in, authType, userName))
            return 0;

        // [nameSpace]

        CIMNamespaceName nameSpace;

        if (!_getNamespaceName(in, nameSpace))
            return 0;

        // [className]

        CIMName className;

        if (!_getName(in, className))
            return 0;

        // [providerType]

        Uint32 providerType;

        if (!in.getUint32(providerType))
            return 0;

        // [message]

        CIMOperationRequestMessage* oreq = 0;

        switch (type)
        {
            case CIM_GET_INSTANCE_REQUEST_MESSAGE:
                oreq = _getGetInstanceRequestMessage(in);
                break;
            case CIM_DELETE_INSTANCE_REQUEST_MESSAGE:
                oreq = _getDeleteInstanceRequestMessage(in);
                break;
            case CIM_CREATE_INSTANCE_REQUEST_MESSAGE:
                oreq = _getCreateInstanceRequestMessage(in);
                break;
            case CIM_MODIFY_INSTANCE_REQUEST_MESSAGE:
                oreq = _getModifyInstanceRequestMessage(in);
                break;
            case CIM_ENUMERATE_INSTANCES_REQUEST_MESSAGE:
                oreq = _getEnumerateInstancesRequestMessage(in);
                break;
            case CIM_ENUMERATE_INSTANCE_NAMES_REQUEST_MESSAGE:
                oreq = _getEnumerateInstanceNamesRequestMessage();
                break;
            case CIM_EXEC_QUERY_REQUEST_MESSAGE:
                oreq = _getExecQueryRequestMessage(in);
                break;
            case CIM_GET_PROPERTY_REQUEST_MESSAGE:
                oreq = _getGetPropertyRequestMessage(in);
                break;
            case CIM_SET_PROPERTY_REQUEST_MESSAGE:
                oreq = _getSetPropertyRequestMessage(in);
                break;
            case CIM_ASSOCIATORS_REQUEST_MESSAGE:
                oreq = _getAssociatorsRequestMessage(in);
                break;
            case CIM_ASSOCIATOR_NAMES_REQUEST_MESSAGE:
                oreq = _getAssociatorNamesRequestMessage(in);
                break;
            case CIM_REFERENCES_REQUEST_MESSAGE:
                oreq = _getReferencesRequestMessage(in);
                break;
            case CIM_REFERENCE_NAMES_REQUEST_MESSAGE:
                oreq = _getReferenceNamesRequestMessage(in);
                break;
            case CIM_INVOKE_METHOD_REQUEST_MESSAGE:
                oreq = _getInvokeMethodRequestMessage(in);
                break;
            default:
                PEGASUS_UNREACHABLE(PEGASUS_ASSERT(0);)
                break;
        }

        if (!oreq)
            return 0;

        oreq->authType = authType;
        oreq->userName = userName;
        oreq->nameSpace = nameSpace;
        oreq->className = className;
        oreq->providerType = providerType;
        msg = oreq;
    }

    // [CIMIndicationRequestMessage]

    if (!in.getPresent(present))
        return 0;

    if (present)
    {
        // [userInfo]

        String authType;
        String userName;

        if (!_getUserInfo(in, authType, userName))
        {
        }

        // [message]

        CIMIndicationRequestMessage* ireq = 0;

        switch (type)
        {
            case CIM_CREATE_SUBSCRIPTION_REQUEST_MESSAGE:
                ireq = _getCreateSubscriptionRequestMessage(in);
                break;
            case CIM_MODIFY_SUBSCRIPTION_REQUEST_MESSAGE:
                ireq = _getModifySubscriptionRequestMessage(in);
                break;
            case CIM_DELETE_SUBSCRIPTION_REQUEST_MESSAGE:
                ireq = _getDeleteSubscriptionRequestMessage(in);
                break;
            default:
                PEGASUS_UNREACHABLE(PEGASUS_ASSERT(0);)
                break;
        }

        if (!ireq)
            return 0;

        // Initialize the message:

        ireq->authType = authType;
        ireq->userName = userName;
        msg = ireq;
    }

    // [other]

    if (!in.getPresent(present))
        return 0;

    if (present)
    {
        switch (type)
        {
            case CIM_EXPORT_INDICATION_REQUEST_MESSAGE:
                msg = _getExportIndicationRequestMessage(in);
                break;
            case CIM_PROCESS_INDICATION_REQUEST_MESSAGE:
                msg = _getProcessIndicationRequestMessage(in);
                break;
            case CIM_DISABLE_MODULE_REQUEST_MESSAGE:
                msg = _getDisableModuleRequestMessage(in);
                break;
            case CIM_ENABLE_MODULE_REQUEST_MESSAGE:
                msg = _getEnableModuleRequestMessage(in);
                break;
            case CIM_STOP_ALL_PROVIDERS_REQUEST_MESSAGE:
                msg = _getStopAllProvidersRequestMessage(in);
                break;
            case CIM_INITIALIZE_PROVIDER_AGENT_REQUEST_MESSAGE:
                msg =
                    _getInitializeProviderAgentRequestMessage(in);
                break;
            case CIM_NOTIFY_CONFIG_CHANGE_REQUEST_MESSAGE:
                msg = _getNotifyConfigChangeRequestMessage(in);
                break;
            case CIM_SUBSCRIPTION_INIT_COMPLETE_REQUEST_MESSAGE:
                msg = _getSubscriptionInitCompleteRequestMessage();
                break;
            case CIM_INDICATION_SERVICE_DISABLED_REQUEST_MESSAGE:
                msg = _getIndicationServiceDisabledRequestMessage();
                break;
            case PROVAGT_GET_SCMOCLASS_REQUEST_MESSAGE:
                msg = _getProvAgtGetScmoClassRequestMessage(in);
                break;


            default:
                PEGASUS_UNREACHABLE(PEGASUS_ASSERT(0);)
                break;
        }

        if (!msg)
            return 0;
    }

    msg->queueIds = queueIdStack;

    return msg;
}

CIMResponseMessage* CIMBinMsgDeserializer::_getResponseMessage(
    CIMBuffer& in,
    MessageType type,
    bool binaryResponse)
{
    CIMResponseMessage* msg = 0;
    QueueIdStack queueIdStack;
    CIMException cimException;

    // [queueIdStack]

    if (!_getQueueIdStack(in, queueIdStack))
        return 0;

    // [cimException]
    if (!_getException(in, cimException))
        return 0;

    // [message]

    switch (type)
    {
        case CIM_GET_INSTANCE_RESPONSE_MESSAGE:
            msg = _getGetInstanceResponseMessage(in, binaryResponse);
            break;
        case CIM_DELETE_INSTANCE_RESPONSE_MESSAGE:
            msg = _getDeleteInstanceResponseMessage();
            break;
        case CIM_CREATE_INSTANCE_RESPONSE_MESSAGE:
            msg = _getCreateInstanceResponseMessage(in);
            break;
        case CIM_MODIFY_INSTANCE_RESPONSE_MESSAGE:
            msg = _getModifyInstanceResponseMessage();
            break;
        case CIM_ENUMERATE_INSTANCES_RESPONSE_MESSAGE:
            msg = _getEnumerateInstancesResponseMessage(in, binaryResponse);
            break;
        case CIM_ENUMERATE_INSTANCE_NAMES_RESPONSE_MESSAGE:
            msg = _getEnumerateInstanceNamesResponseMessage(in);
            break;
        case CIM_EXEC_QUERY_RESPONSE_MESSAGE:
            msg = _getExecQueryResponseMessage(in, binaryResponse);
            break;
        case CIM_GET_PROPERTY_RESPONSE_MESSAGE:
            msg = _getGetPropertyResponseMessage(in);
            break;
        case CIM_SET_PROPERTY_RESPONSE_MESSAGE:
            msg = _getSetPropertyResponseMessage();
            break;
        case CIM_ASSOCIATORS_RESPONSE_MESSAGE:
            msg = _getAssociatorsResponseMessage(in, binaryResponse);
            break;
        case CIM_ASSOCIATOR_NAMES_RESPONSE_MESSAGE:
            msg = _getAssociatorNamesResponseMessage(in);
            break;
        case CIM_REFERENCES_RESPONSE_MESSAGE:
            msg = _getReferencesResponseMessage(in, binaryResponse);
            break;
        case CIM_REFERENCE_NAMES_RESPONSE_MESSAGE:
            msg = _getReferenceNamesResponseMessage(in);
            break;
        case CIM_INVOKE_METHOD_RESPONSE_MESSAGE:
            msg = _getInvokeMethodResponseMessage(in);
            break;
        case CIM_CREATE_SUBSCRIPTION_RESPONSE_MESSAGE:
            msg = _getCreateSubscriptionResponseMessage();
            break;
        case CIM_MODIFY_SUBSCRIPTION_RESPONSE_MESSAGE:
            msg = _getModifySubscriptionResponseMessage();
            break;
        case CIM_DELETE_SUBSCRIPTION_RESPONSE_MESSAGE:
            msg = _getDeleteSubscriptionResponseMessage();
            break;
        case CIM_EXPORT_INDICATION_RESPONSE_MESSAGE:
            msg = _getExportIndicationResponseMessage();
            break;
        case CIM_PROCESS_INDICATION_RESPONSE_MESSAGE:
            msg = _getProcessIndicationResponseMessage();
            break;
        case CIM_DISABLE_MODULE_RESPONSE_MESSAGE:
            msg = _getDisableModuleResponseMessage(in);
            break;
        case CIM_ENABLE_MODULE_RESPONSE_MESSAGE:
            msg = _getEnableModuleResponseMessage(in);
            break;
        case CIM_STOP_ALL_PROVIDERS_RESPONSE_MESSAGE:
            msg = _getStopAllProvidersResponseMessage();
            break;
        case CIM_INITIALIZE_PROVIDER_AGENT_RESPONSE_MESSAGE:
            msg =
                _getInitializeProviderAgentResponseMessage();
            break;
        case CIM_NOTIFY_CONFIG_CHANGE_RESPONSE_MESSAGE:
            msg = _getNotifyConfigChangeResponseMessage();
            break;
        case CIM_SUBSCRIPTION_INIT_COMPLETE_RESPONSE_MESSAGE:
            msg = _getSubscriptionInitCompleteResponseMessage();
            break;
        case CIM_INDICATION_SERVICE_DISABLED_RESPONSE_MESSAGE:
            msg = _getIndicationServiceDisabledResponseMessage();
            break;
        case PROVAGT_GET_SCMOCLASS_RESPONSE_MESSAGE:
            msg = _getProvAgtGetScmoClassResponseMessage(in);
            break;

        default:
            PEGASUS_UNREACHABLE(PEGASUS_ASSERT(0);)
            break;
    }

    if (!msg)
        return 0;

    msg->queueIds = queueIdStack;
    msg->cimException = cimException;

    return msg;
}

Boolean CIMBinMsgDeserializer::_getUserInfo(
    CIMBuffer& in,
    String& authType,
    String& userName)
{
    if (!in.getString(authType))
        return false;

    if (!in.getString(userName))
        return false;

    return true;
}

Boolean CIMBinMsgDeserializer::_getQueueIdStack(
    CIMBuffer& in,
    QueueIdStack& queueIdStack)
{
    Uint32 size;

    if (!in.getUint32(size))
        return false;

    for (Uint32 i = 0; i < size; i++)
    {
        Uint32 tmp;

        if (!in.getUint32(tmp))
            return false;

        queueIdStack.push(tmp);
    }

    return true;
}

Boolean CIMBinMsgDeserializer::_getOperationContext(
    CIMBuffer& in,
    OperationContext& operationContext)
{
    operationContext.clear();

    XmlEntry entry;
    bool present;

    // [IdentityContainer]

    if (!in.getPresent(present))
        return false;

    if (present)
    {
        String userName;

        if (!in.getString(userName))
            return false;

        operationContext.insert(IdentityContainer(userName));
    }

    // [SubscriptionInstanceContainer]

    if (!in.getPresent(present))
        return false;

    if (present)
    {
        CIMInstance ci;

        if (!_getInstance(in, ci))
            return false;

        operationContext.insert(SubscriptionInstanceContainer(ci));
    }

    // [SubscriptionFilterConditionContainer]

    if (!in.getPresent(present))
        return false;

    if (present)
    {
        String filterCondition;
        String queryLanguage;

        if (!in.getString(filterCondition) || !in.getString(queryLanguage))
            return false;

        operationContext.insert(SubscriptionFilterConditionContainer(
            filterCondition, queryLanguage));
    }

    // [SubscriptionFilterQueryContainer]

    if (!in.getPresent(present))
        return false;

    if (present)
    {
        String filterQuery;
        String queryLanguage;
        CIMNamespaceName nameSpace;

        if (!in.getString(filterQuery) || !in.getString(queryLanguage))
            return false;

        if (!_getNamespaceName(in, nameSpace))
            return false;

        operationContext.insert(SubscriptionFilterQueryContainer(
            filterQuery, queryLanguage, nameSpace));
    }

    // [SubscriptionInstanceNamesContainer]

    if (!in.getPresent(present))
        return false;

    if (present)
    {
        Array<CIMObjectPath> cops;

        if (!in.getObjectPathA(cops))
            return false;

        operationContext.insert(SubscriptionInstanceNamesContainer(cops));
    }

    // [TimeoutContainer]

    if (!in.getPresent(present))
        return false;

    if (present)
    {
        Uint32 timeout;

        if (!in.getUint32(timeout))
            return false;

        operationContext.insert(TimeoutContainer(timeout));
    }

    // [AcceptLanguageListContainer]

    if (!in.getPresent(present))
        return false;

    if (present)
    {
        AcceptLanguageList acceptLanguages;

        if (!_getAcceptLanguageList(in, acceptLanguages))
            return false;

        operationContext.insert(AcceptLanguageListContainer(acceptLanguages));
    }

    // [ContentLanguageListContainer]

    if (!in.getPresent(present))
        return false;

    if (present)
    {
        ContentLanguageList list;

        if (!_getContentLanguageList(in, list))
            return false;

        operationContext.insert(ContentLanguageListContainer(list));
    }

    // [SnmpTrapOidContainer]

    if (!in.getPresent(present))
        return false;

    if (present)
    {
        String snmpTrapOid;

        if (!in.getString(snmpTrapOid))
            return false;

        operationContext.insert(SnmpTrapOidContainer(snmpTrapOid));
    }

    // [LocaleContainer]

    if (!in.getPresent(present))
        return false;

    if (present)
    {
        String languageId;

        if (!in.getString(languageId))
            return false;

        operationContext.insert(LocaleContainer(languageId));
    }

    // [ProviderIdContainer]

    if (!in.getPresent(present))
        return false;

    if (present)
    {
        CIMInstance module;
        CIMInstance provider;
        Boolean isRemoteNameSpace;
        String remoteInfo;
        String provMgrPath;

        if (!_getInstance(in, module))
            return false;

        if (!_getInstance(in, provider))
            return false;

        if (!in.getBoolean(isRemoteNameSpace))
            return false;

        if (!in.getString(remoteInfo))
            return false;

        if (!in.getString(provMgrPath))
            return false;

        ProviderIdContainer pidc(
            module, provider, isRemoteNameSpace, remoteInfo);

        pidc.setProvMgrPath(provMgrPath);

        operationContext.insert(pidc);
    }

    // [CachedClassDefinitionContainer]

    if (!in.getPresent(present))
        return false;

    if (present)
    {
        CIMClass cc;

        if (!in.getClass(cc))
            return false;

        operationContext.insert(CachedClassDefinitionContainer(cc));
    }

    // [UserRoleContainer]

    if (!in.getPresent(present))
        return false;

    if (present)
    {
        String userRole;

        if (!in.getString(userRole))
            return false;

        operationContext.insert(UserRoleContainer(userRole));
    }

    return true;
}

Boolean CIMBinMsgDeserializer::_getContentLanguageList(
    CIMBuffer& in,
    ContentLanguageList& contentLanguages)
{
    contentLanguages.clear();

    Uint32 size;

    if (!in.getUint32(size))
        return false;

    for (Uint32 i = 0; i < size; i++)
    {
        String tmp;

        if (!in.getString(tmp))
            return false;

        contentLanguages.append(LanguageTag(tmp));
    }

    return true;
}

Boolean CIMBinMsgDeserializer::_getAcceptLanguageList(
    CIMBuffer& in,
    AcceptLanguageList& acceptLanguages)
{
    acceptLanguages.clear();

    Uint32 size;

    if (!in.getUint32(size))
        return false;

    for (Uint32 i = 0; i < size; i++)
    {
        String languageTag;
        Real32 qualityValue;

        if (!in.getString(languageTag) || !in.getReal32(qualityValue))
            return false;

        acceptLanguages.insert(LanguageTag(languageTag), qualityValue);
    }

    return true;
}

Boolean CIMBinMsgDeserializer::_getException(
    CIMBuffer& in,
    CIMException& cimException)
{
    Uint32 statusCode;
    String message;
    String cimMessage;
    String file;
    Uint32 line;
    ContentLanguageList contentLanguages;

    if (!in.getUint32(statusCode))
        return false;

    if (!in.getString(message))
        return false;

    if (!in.getString(cimMessage))
        return false;

    if (!in.getString(file))
        return false;

    if (!in.getUint32(line))
        return false;

    if (!_getContentLanguageList(in, contentLanguages))
        return false;

    TraceableCIMException e = TraceableCIMException(contentLanguages,
        CIMStatusCode(statusCode), message, file, line);
    e.setCIMMessage(cimMessage);
    cimException = e;

    return true;
}

Boolean CIMBinMsgDeserializer::_getPropertyList(
    CIMBuffer& in,
    CIMPropertyList& propertyList)
{
    return in.getPropertyList(propertyList);
}

Boolean CIMBinMsgDeserializer::_getObjectPath(
    CIMBuffer& in,
    CIMObjectPath& cop)
{
    return in.getObjectPath(cop);
}

Boolean CIMBinMsgDeserializer::_getInstance(
    CIMBuffer& in,
    CIMInstance& ci)
{
    return in.getInstance(ci);
}

Boolean CIMBinMsgDeserializer::_getNamespaceName(
    CIMBuffer& in,
    CIMNamespaceName& cimNamespaceName)
{
    return in.getNamespaceName(cimNamespaceName);
}

Boolean CIMBinMsgDeserializer::_getName(
    CIMBuffer& in,
    CIMName& cn)
{
    return in.getName(cn);
}

Boolean CIMBinMsgDeserializer::_getObject(
    CIMBuffer& in,
    CIMObject& object)
{
    return in.getObject(object);
}

Boolean CIMBinMsgDeserializer::_getParamValue(
    CIMBuffer& in,
    CIMParamValue& pv)
{
    return in.getParamValue(pv);
}

CIMGetInstanceRequestMessage*
CIMBinMsgDeserializer::_getGetInstanceRequestMessage(CIMBuffer& in)
{
    CIMObjectPath instanceName;
    Boolean includeQualifiers;
    Boolean includeClassOrigin;
    CIMPropertyList propertyList;

    if (!in.getObjectPath(instanceName))
        return 0;

    if (!in.getBoolean(includeQualifiers))
        return 0;

    if (!in.getBoolean(includeClassOrigin))
        return 0;

    if (!in.getPropertyList(propertyList))
        return 0;

    return new CIMGetInstanceRequestMessage(
        String::EMPTY,
        CIMNamespaceName(),
        instanceName,
        includeQualifiers,
        includeClassOrigin,
        propertyList,
        QueueIdStack());
}

CIMDeleteInstanceRequestMessage*
CIMBinMsgDeserializer::_getDeleteInstanceRequestMessage(
    CIMBuffer& in)
{
    CIMObjectPath instanceName;

    if (!_getObjectPath(in, instanceName))
        return 0;

    return new CIMDeleteInstanceRequestMessage(
        String::EMPTY,
        CIMNamespaceName(),
        instanceName,
        QueueIdStack());
}

CIMCreateInstanceRequestMessage*
CIMBinMsgDeserializer::_getCreateInstanceRequestMessage(
    CIMBuffer& in)
{
    CIMInstance newInstance;

    if (!_getInstance(in, newInstance))
        return 0;

    return new CIMCreateInstanceRequestMessage(
        String::EMPTY,
        CIMNamespaceName(),
        newInstance,
        QueueIdStack());
}

CIMModifyInstanceRequestMessage*
CIMBinMsgDeserializer::_getModifyInstanceRequestMessage(
    CIMBuffer& in)
{
    CIMInstance modifiedInstance;
    Boolean includeQualifiers;
    CIMPropertyList propertyList;

    if (!_getInstance(in, modifiedInstance))
        return 0;

    if (!in.getBoolean(includeQualifiers))
        return 0;

    if (!_getPropertyList(in, propertyList))
        return 0;

    return new CIMModifyInstanceRequestMessage(
        String::EMPTY,
        CIMNamespaceName(),
        modifiedInstance,
        includeQualifiers,
        propertyList,
        QueueIdStack());
}

CIMEnumerateInstancesRequestMessage*
CIMBinMsgDeserializer::_getEnumerateInstancesRequestMessage(
    CIMBuffer& in)
{
    CIMObjectPath instanceName;
    Boolean deepInheritance;
    Boolean includeQualifiers;
    Boolean includeClassOrigin;
    CIMPropertyList propertyList;

    if (!in.getBoolean(deepInheritance))
        return 0;

    if (!in.getBoolean(includeQualifiers))
        return 0;

    if (!in.getBoolean(includeClassOrigin))
        return 0;

    if (!_getPropertyList(in, propertyList))
        return 0;

    return new CIMEnumerateInstancesRequestMessage(
        String::EMPTY,
        CIMNamespaceName(),
        CIMName(),
        deepInheritance,
        includeQualifiers,
        includeClassOrigin,
        propertyList,
        QueueIdStack());
}

CIMEnumerateInstanceNamesRequestMessage*
CIMBinMsgDeserializer::_getEnumerateInstanceNamesRequestMessage()
{
    return new CIMEnumerateInstanceNamesRequestMessage(
        String::EMPTY,
        CIMNamespaceName(),
        CIMName(),
        QueueIdStack());
}

CIMExecQueryRequestMessage*
CIMBinMsgDeserializer::_getExecQueryRequestMessage(
    CIMBuffer& in)
{
    String queryLanguage;
    String query;

    if (!in.getString(queryLanguage) || !in.getString(query))
        return 0;

    return new CIMExecQueryRequestMessage(
        String::EMPTY,
        CIMNamespaceName(),
        queryLanguage,
        query,
        QueueIdStack());
}

CIMAssociatorsRequestMessage*
CIMBinMsgDeserializer::_getAssociatorsRequestMessage(
    CIMBuffer& in)
{
    CIMObjectPath objectName;
    CIMName assocClass;
    CIMName resultClass;
    String role;
    String resultRole;
    Boolean includeQualifiers;
    Boolean includeClassOrigin;
    CIMPropertyList propertyList;

    if (!_getObjectPath(in, objectName))
        return 0;

    if (!_getName(in, assocClass))
        return 0;

    if (!_getName(in, resultClass))
        return 0;

    if (!in.getString(role))
        return 0;

    if (!in.getString(resultRole))
        return 0;

    if (!in.getBoolean(includeQualifiers))
        return 0;

    if (!in.getBoolean(includeClassOrigin))
        return 0;

    if (!_getPropertyList(in, propertyList))
        return 0;

    return new CIMAssociatorsRequestMessage(
        String::EMPTY,
        CIMNamespaceName(),
        objectName,
        assocClass,
        resultClass,
        role,
        resultRole,
        includeQualifiers,
        includeClassOrigin,
        propertyList,
        QueueIdStack());
}

CIMAssociatorNamesRequestMessage*
CIMBinMsgDeserializer::_getAssociatorNamesRequestMessage(
    CIMBuffer& in)
{
    CIMObjectPath objectName;
    CIMName assocClass;
    CIMName resultClass;
    String role;
    String resultRole;

    if (!_getObjectPath(in, objectName))
        return 0;

    if (!_getName(in, assocClass))
        return 0;

    if (!_getName(in, resultClass))
        return 0;

    if (!in.getString(role))
        return 0;

    if (!in.getString(resultRole))
        return 0;

    return new CIMAssociatorNamesRequestMessage(
        String::EMPTY,
        CIMNamespaceName(),
        objectName,
        assocClass,
        resultClass,
        role,
        resultRole,
        QueueIdStack());
}

CIMReferencesRequestMessage*
CIMBinMsgDeserializer::_getReferencesRequestMessage(CIMBuffer& in)
{
    CIMObjectPath objectName;
    CIMName resultClass;
    String role;
    Boolean includeQualifiers;
    Boolean includeClassOrigin;
    CIMPropertyList propertyList;

    if (!_getObjectPath(in, objectName))
        return 0;

    if (!_getName(in, resultClass))
        return 0;

    if (!in.getString(role))
        return 0;

    if (!in.getBoolean(includeQualifiers))
        return 0;

    if (!in.getBoolean(includeClassOrigin))
        return 0;

    if (!_getPropertyList(in, propertyList))
        return 0;

    return new CIMReferencesRequestMessage(
        String::EMPTY,
        CIMNamespaceName(),
        objectName,
        resultClass,
        role,
        includeQualifiers,
        includeClassOrigin,
        propertyList,
        QueueIdStack());
}

CIMReferenceNamesRequestMessage*
CIMBinMsgDeserializer::_getReferenceNamesRequestMessage(
    CIMBuffer& in)
{
    CIMObjectPath objectName;
    CIMName resultClass;
    String role;

    if (!_getObjectPath(in, objectName))
        return 0;

    if (!_getName(in, resultClass))
        return 0;

    if (!in.getString(role))
        return 0;

    return new CIMReferenceNamesRequestMessage(
        String::EMPTY,
        CIMNamespaceName(),
        objectName,
        resultClass,
        role,
        QueueIdStack());
}

CIMGetPropertyRequestMessage*
CIMBinMsgDeserializer::_getGetPropertyRequestMessage(
    CIMBuffer& in)
{
    CIMObjectPath instanceName;
    CIMName propertyName;

    if (!_getObjectPath(in, instanceName))
        return 0;

    if (!_getName(in, propertyName))
        return 0;

    return new CIMGetPropertyRequestMessage(
        String::EMPTY,
        CIMNamespaceName(),
        instanceName,
        propertyName,
        QueueIdStack());
}

CIMSetPropertyRequestMessage*
CIMBinMsgDeserializer::_getSetPropertyRequestMessage(
    CIMBuffer& in)
{
    CIMObjectPath instanceName;
    CIMParamValue newValue;

    if (!_getObjectPath(in, instanceName))
        return 0;

    if (!_getParamValue(in, newValue))
        return 0;

    return new CIMSetPropertyRequestMessage(
        String::EMPTY,
        CIMNamespaceName(),
        instanceName,
        newValue.getParameterName(),
        newValue.getValue(),
        QueueIdStack());
}

CIMInvokeMethodRequestMessage*
CIMBinMsgDeserializer::_getInvokeMethodRequestMessage(
    CIMBuffer& in)
{
    XmlEntry entry;
    CIMObjectPath instanceName;
    CIMName methodName;
    Array<CIMParamValue> inParameters;

    if (!_getObjectPath(in, instanceName))
        return 0;

    if (!_getName(in, methodName))
        return 0;

    if (!in.getParamValueA(inParameters))
        return 0;

    return new CIMInvokeMethodRequestMessage(
        String::EMPTY,
        CIMNamespaceName(),
        instanceName,
        methodName,
        inParameters,
        QueueIdStack());
}

CIMCreateSubscriptionRequestMessage*
CIMBinMsgDeserializer::_getCreateSubscriptionRequestMessage(
    CIMBuffer& in)
{
    XmlEntry entry;
    CIMNamespaceName nameSpace;
    CIMInstance subscriptionInstance;
    Array<CIMName> classNames;
    CIMPropertyList propertyList;
    Uint16 repeatNotificationPolicy;
    String query;

    if (!_getNamespaceName(in, nameSpace))
        return 0;

    if (!_getInstance(in, subscriptionInstance))
        return 0;

    if (!in.getNameA(classNames))
        return 0;

    if (!_getPropertyList(in, propertyList))
        return 0;

    if (!in.getUint16(repeatNotificationPolicy))
        return 0;

    if (!in.getString(query))
        return 0;

    return new CIMCreateSubscriptionRequestMessage(
        String::EMPTY,
        nameSpace,
        subscriptionInstance,
        classNames,
        propertyList,
        repeatNotificationPolicy,
        query,
        QueueIdStack());
}

CIMModifySubscriptionRequestMessage*
CIMBinMsgDeserializer::_getModifySubscriptionRequestMessage(
    CIMBuffer& in)
{
    XmlEntry entry;
    CIMNamespaceName nameSpace;
    CIMInstance subscriptionInstance;
    Array<CIMName> classNames;
    CIMPropertyList propertyList;
    Uint16 repeatNotificationPolicy;
    String query;

    if (!_getNamespaceName(in, nameSpace))
        return 0;

    if (!_getInstance(in, subscriptionInstance))
        return 0;

    if (!in.getNameA(classNames))
        return 0;

    if (!_getPropertyList(in, propertyList))
        return 0;

    if (!in.getUint16(repeatNotificationPolicy))
        return 0;

    if (!in.getString(query))
        return 0;

    return new CIMModifySubscriptionRequestMessage(
        String::EMPTY,
        nameSpace,
        subscriptionInstance,
        classNames,
        propertyList,
        repeatNotificationPolicy,
        query,
        QueueIdStack());
}

CIMDeleteSubscriptionRequestMessage*
CIMBinMsgDeserializer::_getDeleteSubscriptionRequestMessage(
    CIMBuffer& in)
{
    XmlEntry entry;
    CIMNamespaceName nameSpace;
    CIMInstance subscriptionInstance;
    Array<CIMName> classNames;

    if (!_getNamespaceName(in, nameSpace))
        return 0;

    if (!_getInstance(in, subscriptionInstance))
        return 0;

    if (!in.getNameA(classNames))
        return 0;

    return new CIMDeleteSubscriptionRequestMessage(
        String::EMPTY,
        nameSpace,
        subscriptionInstance,
        classNames,
        QueueIdStack());
}

CIMExportIndicationRequestMessage*
CIMBinMsgDeserializer::_getExportIndicationRequestMessage(
    CIMBuffer& in)
{
    String authType;
    String userName;
    String destinationPath;
    CIMInstance indicationInstance;

    if (!_getUserInfo(in, authType, userName))
        return 0;

    if (!in.getString(destinationPath))
        return 0;

    if (!_getInstance(in, indicationInstance))
        return 0;

    return new CIMExportIndicationRequestMessage(
        String::EMPTY,
        destinationPath,
        indicationInstance,
        QueueIdStack(),
        authType,
        userName);
}

CIMProcessIndicationRequestMessage*
CIMBinMsgDeserializer::_getProcessIndicationRequestMessage(
    CIMBuffer& in)
{
    XmlEntry entry;
    CIMNamespaceName nameSpace;
    CIMInstance indicationInstance;
    Array<CIMObjectPath> subscriptionInstanceNames;
    CIMInstance provider;
    Uint32 timeoutMilliSec;

    if (!_getNamespaceName(in, nameSpace))
        return 0;

    if (!_getInstance(in, indicationInstance))
        return 0;

    if (!in.getObjectPathA(subscriptionInstanceNames))
        return 0;

    if (!_getInstance(in, provider))
        return 0;

    if (!in.getUint32(timeoutMilliSec))
        return 0;

    return new CIMProcessIndicationRequestMessage(
        String::EMPTY,
        nameSpace,
        indicationInstance,
        subscriptionInstanceNames,
        provider,
        QueueIdStack(),
        timeoutMilliSec);
}

CIMDisableModuleRequestMessage*
CIMBinMsgDeserializer::_getDisableModuleRequestMessage(
    CIMBuffer& in)
{
    XmlEntry entry;
    String authType;
    String userName;
    CIMInstance providerModule;
    Array<CIMInstance> providers;
    Boolean disableProviderOnly;
    Array<Boolean> indicationProviders;

    if (!_getUserInfo(in, authType, userName))
        return 0;

    if (!_getInstance(in, providerModule))
        return 0;

    if (!in.getInstanceA(providers))
        return 0;

    if (!in.getBoolean(disableProviderOnly))
        return 0;

    if (!in.getBooleanA(indicationProviders))
        return 0;

    return new CIMDisableModuleRequestMessage(
        String::EMPTY,
        providerModule,
        providers,
        disableProviderOnly,
        indicationProviders,
        QueueIdStack(),
        authType,
        userName);
}

CIMEnableModuleRequestMessage*
CIMBinMsgDeserializer::_getEnableModuleRequestMessage(
    CIMBuffer& in)
{
    String authType;
    String userName;
    CIMInstance providerModule;

    if (!_getUserInfo(in, authType, userName))
        return 0;

    if (!_getInstance(in, providerModule))
        return 0;

    return new CIMEnableModuleRequestMessage(
        String::EMPTY,
        providerModule,
        QueueIdStack(),
        authType,
        userName);
}

CIMStopAllProvidersRequestMessage*
CIMBinMsgDeserializer::_getStopAllProvidersRequestMessage(
    CIMBuffer& in)
{
    Uint32 shutdownTimeout;

    if (!in.getUint32(shutdownTimeout))
        return 0;

    return new CIMStopAllProvidersRequestMessage(
        String::EMPTY,
        QueueIdStack(),
        shutdownTimeout);
}

CIMInitializeProviderAgentRequestMessage*
CIMBinMsgDeserializer::_getInitializeProviderAgentRequestMessage(
    CIMBuffer& in)
{
    XmlEntry entry;
    String pegasusHome;
    typedef Pair<String,String> ConfigPair;
    Array<ConfigPair> configProperties;
    Uint32 size;
    Boolean bindVerbose;
    Boolean subscriptionInitComplete;

    if (!in.getString(pegasusHome))
        return 0;

    if (!in.getUint32(size))
        return 0;

    for (Uint32 i = 0; i < size; i++)
    {
        String first;
        String second;

        if (!in.getString(first) || !in.getString(second))
            return 0;

        configProperties.append(ConfigPair(first, second));
    }

    if (!in.getBoolean(bindVerbose))
        return 0;

    if (!in.getBoolean(subscriptionInitComplete))
        return 0;

    return new CIMInitializeProviderAgentRequestMessage(
        String::EMPTY,
        pegasusHome,
        configProperties,
        bindVerbose,
        subscriptionInitComplete,
        QueueIdStack());
}

CIMNotifyConfigChangeRequestMessage*
CIMBinMsgDeserializer::_getNotifyConfigChangeRequestMessage(
    CIMBuffer& in)
{
    String propertyName;
    String newPropertyValue;
    Boolean currentValueModified;

    if (!in.getString(propertyName))
        return 0;

    if (!in.getString(newPropertyValue))
        return 0;

    if (!in.getBoolean(currentValueModified))
        return 0;

    return new CIMNotifyConfigChangeRequestMessage(
        String::EMPTY,
        propertyName,
        newPropertyValue,
        currentValueModified,
        QueueIdStack());
}

CIMIndicationServiceDisabledRequestMessage*
CIMBinMsgDeserializer::_getIndicationServiceDisabledRequestMessage()
{
    return new CIMIndicationServiceDisabledRequestMessage(
        String(),
        QueueIdStack());
}

CIMSubscriptionInitCompleteRequestMessage*
CIMBinMsgDeserializer::_getSubscriptionInitCompleteRequestMessage()
{
    return new CIMSubscriptionInitCompleteRequestMessage(
        String::EMPTY,
        QueueIdStack());
}

ProvAgtGetScmoClassRequestMessage*
CIMBinMsgDeserializer::_getProvAgtGetScmoClassRequestMessage(
    CIMBuffer& in)
{
    CIMName className;
    CIMNamespaceName nsName;
    String messageID;

    if (!in.getString(messageID))
        return 0;

    if (!in.getNamespaceName(nsName))
        return 0;

    if (!in.getName(className))
        return 0;

    return new ProvAgtGetScmoClassRequestMessage(
        messageID,
        nsName,
        className,
        QueueIdStack());
}

CIMGetInstanceResponseMessage*
CIMBinMsgDeserializer::_getGetInstanceResponseMessage(
    CIMBuffer& in,
    bool binaryResponse)
{
    CIMGetInstanceResponseMessage* msg = new CIMGetInstanceResponseMessage(
        String::EMPTY,
        CIMException(),
        QueueIdStack());

    CIMResponseData& responseData = msg->getResponseData();

    if (binaryResponse)
    {
        if (!responseData.setBinary(in))
        {
            delete(msg);
            return 0;
        }
    }
    else
    {
        if (!responseData.setXml(in))
        {
            delete(msg);
            return 0;
        }
    }

    return msg;
}

CIMDeleteInstanceResponseMessage*
CIMBinMsgDeserializer::_getDeleteInstanceResponseMessage()
{
    return new CIMDeleteInstanceResponseMessage(
        String::EMPTY,
        CIMException(),
        QueueIdStack());
}

CIMCreateInstanceResponseMessage*
CIMBinMsgDeserializer::_getCreateInstanceResponseMessage(
    CIMBuffer& in)
{
    CIMObjectPath instanceName;

    if (!_getObjectPath(in, instanceName))
        return 0;

    return new CIMCreateInstanceResponseMessage(
        String::EMPTY,
        CIMException(),
        QueueIdStack(),
        instanceName);
}

CIMModifyInstanceResponseMessage*
CIMBinMsgDeserializer::_getModifyInstanceResponseMessage()
{
    return new CIMModifyInstanceResponseMessage(
        String::EMPTY,
        CIMException(),
        QueueIdStack());
}

CIMEnumerateInstancesResponseMessage*
CIMBinMsgDeserializer::_getEnumerateInstancesResponseMessage(
    CIMBuffer& in,
    bool binaryResponse)
{
    CIMEnumerateInstancesResponseMessage* msg;

    msg = new CIMEnumerateInstancesResponseMessage(String::EMPTY,
        CIMException(), QueueIdStack());

    CIMResponseData& responseData = msg->getResponseData();

    if (binaryResponse)
    {
        if (!responseData.setBinary(in))
        {
            delete(msg);
            return 0;
        }
        return msg;
    }
    else
    {
        if (!responseData.setXml(in))
        {
            delete(msg);
            return 0;
        }
        return msg;
    }
}

CIMEnumerateInstanceNamesResponseMessage*
CIMBinMsgDeserializer::_getEnumerateInstanceNamesResponseMessage(
    CIMBuffer& in)
{
    CIMEnumerateInstanceNamesResponseMessage* msg;

    msg = new CIMEnumerateInstanceNamesResponseMessage(String::EMPTY,
                                                       CIMException(),
                                                       QueueIdStack());

    CIMResponseData& responseData = msg->getResponseData();

    if (!responseData.setBinary(in))
    {
        delete(msg);
        return 0;
    }

    return msg;
}

CIMExecQueryResponseMessage*
CIMBinMsgDeserializer::_getExecQueryResponseMessage(
    CIMBuffer& in,
    bool binaryResponse)
{
    CIMExecQueryResponseMessage* msg;

    msg = new CIMExecQueryResponseMessage(String::EMPTY,
        CIMException(), QueueIdStack());

    CIMResponseData& responseData = msg->getResponseData();

    if (binaryResponse)
    {
        if (!responseData.setBinary(in))
        {
            delete(msg);
            return 0;
        }
    }
    else
    {
        if (!responseData.setXml(in))
        {
            delete(msg);
            return 0;
        }
    }

    return msg;
}

CIMAssociatorsResponseMessage*
CIMBinMsgDeserializer::_getAssociatorsResponseMessage(
    CIMBuffer& in,
    bool binaryResponse)
{
    CIMAssociatorsResponseMessage* msg;

    msg = new CIMAssociatorsResponseMessage(String::EMPTY,
        CIMException(), QueueIdStack());

    CIMResponseData& responseData = msg->getResponseData();

    if (binaryResponse)
    {
        if (!responseData.setBinary(in))
        {
            delete(msg);
            return 0;
        }
    }
    else
    {
        if (!responseData.setXml(in))
        {
            delete(msg);
            return 0;
        }
    }

    return msg;
}

CIMAssociatorNamesResponseMessage*
CIMBinMsgDeserializer::_getAssociatorNamesResponseMessage(
    CIMBuffer& in)
{
    CIMAssociatorNamesResponseMessage* msg;

    msg = new CIMAssociatorNamesResponseMessage(String::EMPTY,
        CIMException(), QueueIdStack());

    CIMResponseData& responseData = msg->getResponseData();

    if (!responseData.setBinary(in))
    {
        delete(msg);
        return 0;
    }

    return msg;
}

CIMReferencesResponseMessage*
CIMBinMsgDeserializer::_getReferencesResponseMessage(
    CIMBuffer& in,
    bool binaryResponse)
{
    CIMReferencesResponseMessage* msg;

    msg = new CIMReferencesResponseMessage(String::EMPTY,
        CIMException(), QueueIdStack());

    CIMResponseData& responseData = msg->getResponseData();

    if (binaryResponse)
    {
        if (!responseData.setBinary(in))
        {
            delete(msg);
            return 0;
        }
    }
    else
    {
        if (!responseData.setXml(in))
        {
            delete(msg);
            return 0;
        }
    }

    return msg;
}

CIMReferenceNamesResponseMessage*
CIMBinMsgDeserializer::_getReferenceNamesResponseMessage(
    CIMBuffer& in)
{
    CIMReferenceNamesResponseMessage* msg;

    msg = new CIMReferenceNamesResponseMessage(String::EMPTY,
        CIMException(), QueueIdStack());

    CIMResponseData& responseData = msg->getResponseData();

    if (!responseData.setBinary(in))
    {
        delete(msg);
        return 0;
    }

    return msg;
}

CIMGetPropertyResponseMessage*
CIMBinMsgDeserializer::_getGetPropertyResponseMessage(
    CIMBuffer& in)
{
    CIMParamValue value;

    if (!_getParamValue(in, value))
        return 0;

    return new CIMGetPropertyResponseMessage(
        String::EMPTY,
        CIMException(),
        QueueIdStack(),
        value.getValue());
}

CIMSetPropertyResponseMessage*
CIMBinMsgDeserializer::_getSetPropertyResponseMessage()
{
    return new CIMSetPropertyResponseMessage(
        String::EMPTY,
        CIMException(),
        QueueIdStack());
}

CIMInvokeMethodResponseMessage*
CIMBinMsgDeserializer::_getInvokeMethodResponseMessage(
    CIMBuffer& in)
{
    XmlEntry entry;
    CIMParamValue genericParamValue;
    CIMParamValue retValue;
    CIMName methodName;
    Array<CIMParamValue> outParameters;

    if (!_getParamValue(in, retValue))
        return 0;

    if (!in.getParamValueA(outParameters))
        return 0;

    if (!_getName(in, methodName))
        return 0;

    return new CIMInvokeMethodResponseMessage(
        String::EMPTY,
        CIMException(),
        QueueIdStack(),
        retValue.getValue(),
        outParameters,
        methodName);
}

CIMCreateSubscriptionResponseMessage*
CIMBinMsgDeserializer::_getCreateSubscriptionResponseMessage()
{
    return new CIMCreateSubscriptionResponseMessage(
        String::EMPTY,
        CIMException(),
        QueueIdStack());
}

CIMModifySubscriptionResponseMessage*
CIMBinMsgDeserializer::_getModifySubscriptionResponseMessage()
{
    return new CIMModifySubscriptionResponseMessage(
        String::EMPTY,
        CIMException(),
        QueueIdStack());
}

CIMDeleteSubscriptionResponseMessage*
CIMBinMsgDeserializer::_getDeleteSubscriptionResponseMessage()
{
    return new CIMDeleteSubscriptionResponseMessage(
        String::EMPTY,
        CIMException(),
        QueueIdStack());
}

CIMExportIndicationResponseMessage*
CIMBinMsgDeserializer::_getExportIndicationResponseMessage()
{
    return new CIMExportIndicationResponseMessage(
        String::EMPTY,
        CIMException(),
        QueueIdStack());
}

CIMProcessIndicationResponseMessage*
CIMBinMsgDeserializer::_getProcessIndicationResponseMessage()
{
    return new CIMProcessIndicationResponseMessage(
        String::EMPTY,
        CIMException(),
        QueueIdStack());
}

CIMDisableModuleResponseMessage*
CIMBinMsgDeserializer::_getDisableModuleResponseMessage(
    CIMBuffer& in)
{
    XmlEntry entry;
    CIMValue genericValue;
    Array<Uint16> operationalStatus;

    if (!in.getUint16A(operationalStatus))
        return 0;

    return new CIMDisableModuleResponseMessage(
        String::EMPTY,
        CIMException(),
        QueueIdStack(),
        operationalStatus);
}

CIMEnableModuleResponseMessage*
CIMBinMsgDeserializer::_getEnableModuleResponseMessage(
    CIMBuffer& in)
{
    XmlEntry entry;
    CIMValue genericValue;
    Array<Uint16> operationalStatus;

    if (!in.getUint16A(operationalStatus))
        return 0;

    return new CIMEnableModuleResponseMessage(
        String::EMPTY,
        CIMException(),
        QueueIdStack(),
        operationalStatus);
}

CIMStopAllProvidersResponseMessage*
CIMBinMsgDeserializer::_getStopAllProvidersResponseMessage()
{
    return new CIMStopAllProvidersResponseMessage(
        String::EMPTY,
        CIMException(),
        QueueIdStack());
}

CIMInitializeProviderAgentResponseMessage*
CIMBinMsgDeserializer::_getInitializeProviderAgentResponseMessage()
{
    return new CIMInitializeProviderAgentResponseMessage(
        String::EMPTY,
        CIMException(),
        QueueIdStack());
}

CIMNotifyConfigChangeResponseMessage*
CIMBinMsgDeserializer::_getNotifyConfigChangeResponseMessage()
{
    return new CIMNotifyConfigChangeResponseMessage(
        String::EMPTY,
        CIMException(),
        QueueIdStack());
}

CIMSubscriptionInitCompleteResponseMessage*
CIMBinMsgDeserializer::_getSubscriptionInitCompleteResponseMessage()
{
    return new CIMSubscriptionInitCompleteResponseMessage(
        String::EMPTY,
        CIMException(),
        QueueIdStack());
}

CIMIndicationServiceDisabledResponseMessage*
CIMBinMsgDeserializer::_getIndicationServiceDisabledResponseMessage()
{
    return new CIMIndicationServiceDisabledResponseMessage(
        String(),
        CIMException(),
        QueueIdStack());
}

ProvAgtGetScmoClassResponseMessage*
CIMBinMsgDeserializer::_getProvAgtGetScmoClassResponseMessage(
    CIMBuffer& in)
{
    SCMOClass theClass("","");
    String messageID;

    if (!in.getString(messageID))
        return 0;

    if (!in.getSCMOClass(theClass))
        return 0;

    return new ProvAgtGetScmoClassResponseMessage(
        messageID,
        CIMException(),
        QueueIdStack(),
        theClass);
}

PEGASUS_NAMESPACE_END
