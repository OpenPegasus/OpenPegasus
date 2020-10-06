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

#ifndef Pegasus_CIMBinMsgDeserializer_h
#define Pegasus_CIMBinMsgDeserializer_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/CIMMessage.h>
#include <Pegasus/Common/CIMBuffer.h>

PEGASUS_NAMESPACE_BEGIN

/** This class converts binary messages into CIMMessage objects. Please see
    comments for The CIMBuffer class for more information about goals of the
    the serialization/deserialization scheme.
*/
class PEGASUS_COMMON_LINKAGE CIMBinMsgDeserializer
{
public:

    static CIMMessage* deserialize(CIMBuffer& in, size_t size);

private:

    PEGASUS_HIDDEN_LINKAGE
    static CIMRequestMessage* _getRequestMessage(
        CIMBuffer& in,
        MessageType type);

    PEGASUS_HIDDEN_LINKAGE
    static CIMResponseMessage* _getResponseMessage(
        CIMBuffer& in,
        MessageType type,
        bool binaryResponse);

    PEGASUS_HIDDEN_LINKAGE
    static Boolean _getUserInfo(
        CIMBuffer& in,
        String& authType,
        String& userName);

    PEGASUS_HIDDEN_LINKAGE
    static Boolean _getQueueIdStack(
        CIMBuffer& in,
        QueueIdStack& queueIdStack);

    PEGASUS_HIDDEN_LINKAGE
    static Boolean _getOperationContext(
        CIMBuffer& in,
        OperationContext& operationContext);

    PEGASUS_HIDDEN_LINKAGE
    static Boolean _getContentLanguageList(
        CIMBuffer& in,
        ContentLanguageList& contentLanguages);

    PEGASUS_HIDDEN_LINKAGE
    static Boolean _getAcceptLanguageList(
        CIMBuffer& in,
        AcceptLanguageList& acceptLanguages);

    PEGASUS_HIDDEN_LINKAGE
    static Boolean _getException(
        CIMBuffer& in,
        CIMException& cimException);

    PEGASUS_HIDDEN_LINKAGE
    static Boolean _getPropertyList(
        CIMBuffer& in,
        CIMPropertyList& propertyList);

    PEGASUS_HIDDEN_LINKAGE
    static Boolean _getObjectPath(
        CIMBuffer& in,
        CIMObjectPath& cimObjectPath);

    PEGASUS_HIDDEN_LINKAGE
    static Boolean _getInstance(
        CIMBuffer& in,
        CIMInstance& cimInstance);

    PEGASUS_HIDDEN_LINKAGE
    static Boolean _getNamespaceName(
        CIMBuffer& in,
        CIMNamespaceName& cimNamespaceName);

    PEGASUS_HIDDEN_LINKAGE
    static Boolean _getName(
        CIMBuffer& in,
        CIMName& cimName);

    PEGASUS_HIDDEN_LINKAGE
    static Boolean _getObject(
        CIMBuffer& in,
        CIMObject& object);

    PEGASUS_HIDDEN_LINKAGE
    static Boolean _getParamValue(
        CIMBuffer& in,
        CIMParamValue& paramValue);

    PEGASUS_HIDDEN_LINKAGE
    static CIMGetInstanceRequestMessage*
        _getGetInstanceRequestMessage(CIMBuffer& in);

    PEGASUS_HIDDEN_LINKAGE
    static CIMDeleteInstanceRequestMessage*
        _getDeleteInstanceRequestMessage(CIMBuffer& in);

    PEGASUS_HIDDEN_LINKAGE
    static CIMCreateInstanceRequestMessage*
        _getCreateInstanceRequestMessage(CIMBuffer& in);

    PEGASUS_HIDDEN_LINKAGE
    static CIMModifyInstanceRequestMessage*
        _getModifyInstanceRequestMessage(CIMBuffer& in);

    PEGASUS_HIDDEN_LINKAGE
    static CIMEnumerateInstancesRequestMessage*
        _getEnumerateInstancesRequestMessage(CIMBuffer& in);

    PEGASUS_HIDDEN_LINKAGE
    static CIMEnumerateInstanceNamesRequestMessage*
        _getEnumerateInstanceNamesRequestMessage();

    PEGASUS_HIDDEN_LINKAGE
    static CIMExecQueryRequestMessage*
        _getExecQueryRequestMessage(CIMBuffer& in);

    PEGASUS_HIDDEN_LINKAGE
    static CIMAssociatorsRequestMessage*
        _getAssociatorsRequestMessage(CIMBuffer& in);

    PEGASUS_HIDDEN_LINKAGE
    static CIMAssociatorNamesRequestMessage*
        _getAssociatorNamesRequestMessage(CIMBuffer& in);

    PEGASUS_HIDDEN_LINKAGE
    static CIMReferencesRequestMessage*
        _getReferencesRequestMessage(CIMBuffer& in);

    PEGASUS_HIDDEN_LINKAGE
    static CIMReferenceNamesRequestMessage*
        _getReferenceNamesRequestMessage(CIMBuffer& in);

    PEGASUS_HIDDEN_LINKAGE
    static CIMGetPropertyRequestMessage*
        _getGetPropertyRequestMessage(CIMBuffer& in);

    PEGASUS_HIDDEN_LINKAGE
    static CIMSetPropertyRequestMessage*
        _getSetPropertyRequestMessage(CIMBuffer& in);

    PEGASUS_HIDDEN_LINKAGE
    static CIMInvokeMethodRequestMessage*
        _getInvokeMethodRequestMessage(CIMBuffer& in);

    PEGASUS_HIDDEN_LINKAGE
    static CIMInitializeProviderAgentRequestMessage*
        _getInitializeProviderAgentRequestMessage(CIMBuffer& in);

    static CIMCreateSubscriptionRequestMessage*
        _getCreateSubscriptionRequestMessage(CIMBuffer& in);

    PEGASUS_HIDDEN_LINKAGE
    static CIMModifySubscriptionRequestMessage*
        _getModifySubscriptionRequestMessage(CIMBuffer& in);

    PEGASUS_HIDDEN_LINKAGE
    static CIMDeleteSubscriptionRequestMessage*
        _getDeleteSubscriptionRequestMessage(CIMBuffer& in);

    PEGASUS_HIDDEN_LINKAGE
    static CIMExportIndicationRequestMessage*
        _getExportIndicationRequestMessage(CIMBuffer& in);

    PEGASUS_HIDDEN_LINKAGE
    static CIMProcessIndicationRequestMessage*
        _getProcessIndicationRequestMessage(CIMBuffer& in);

    PEGASUS_HIDDEN_LINKAGE
    static CIMDisableModuleRequestMessage*
        _getDisableModuleRequestMessage(CIMBuffer& in);

    PEGASUS_HIDDEN_LINKAGE
    static CIMEnableModuleRequestMessage*
        _getEnableModuleRequestMessage(CIMBuffer& in);

    PEGASUS_HIDDEN_LINKAGE
    static CIMStopAllProvidersRequestMessage*
        _getStopAllProvidersRequestMessage(CIMBuffer& in);

    PEGASUS_HIDDEN_LINKAGE
    static CIMNotifyConfigChangeRequestMessage*
        _getNotifyConfigChangeRequestMessage(CIMBuffer& in);

    PEGASUS_HIDDEN_LINKAGE
    static CIMSubscriptionInitCompleteRequestMessage*
        _getSubscriptionInitCompleteRequestMessage();

    PEGASUS_HIDDEN_LINKAGE
    static CIMIndicationServiceDisabledRequestMessage*
        _getIndicationServiceDisabledRequestMessage();

    PEGASUS_HIDDEN_LINKAGE
    static ProvAgtGetScmoClassRequestMessage*
        _getProvAgtGetScmoClassRequestMessage(CIMBuffer& in);

    PEGASUS_HIDDEN_LINKAGE
    static CIMGetInstanceResponseMessage* _getGetInstanceResponseMessage(
        CIMBuffer& in, bool binaryResponse);

    PEGASUS_HIDDEN_LINKAGE
    static CIMDeleteInstanceResponseMessage*
        _getDeleteInstanceResponseMessage();

    PEGASUS_HIDDEN_LINKAGE
    static CIMCreateInstanceResponseMessage*
        _getCreateInstanceResponseMessage(CIMBuffer& in);

    PEGASUS_HIDDEN_LINKAGE
    static CIMModifyInstanceResponseMessage*
        _getModifyInstanceResponseMessage();

    PEGASUS_HIDDEN_LINKAGE
    static CIMEnumerateInstancesResponseMessage*
        _getEnumerateInstancesResponseMessage(
            CIMBuffer& in, bool binaryResponse);

    PEGASUS_HIDDEN_LINKAGE
    static CIMEnumerateInstanceNamesResponseMessage*
        _getEnumerateInstanceNamesResponseMessage(CIMBuffer& in);

    PEGASUS_HIDDEN_LINKAGE
    static CIMExecQueryResponseMessage*
        _getExecQueryResponseMessage(CIMBuffer& in, bool binaryResponse);

    PEGASUS_HIDDEN_LINKAGE
    static CIMAssociatorsResponseMessage*
        _getAssociatorsResponseMessage(CIMBuffer& in, bool binaryResponse);

    PEGASUS_HIDDEN_LINKAGE
    static CIMAssociatorNamesResponseMessage*
        _getAssociatorNamesResponseMessage(CIMBuffer& in);

    PEGASUS_HIDDEN_LINKAGE
    static CIMReferencesResponseMessage*
        _getReferencesResponseMessage(CIMBuffer& in, bool binaryResponse);

    PEGASUS_HIDDEN_LINKAGE
    static CIMReferenceNamesResponseMessage*
        _getReferenceNamesResponseMessage(CIMBuffer& in);

    PEGASUS_HIDDEN_LINKAGE
    static CIMGetPropertyResponseMessage*
        _getGetPropertyResponseMessage(CIMBuffer& in);

    PEGASUS_HIDDEN_LINKAGE
    static CIMSetPropertyResponseMessage*
        _getSetPropertyResponseMessage();

    PEGASUS_HIDDEN_LINKAGE
    static CIMInvokeMethodResponseMessage*
        _getInvokeMethodResponseMessage(CIMBuffer& in);

    PEGASUS_HIDDEN_LINKAGE
    static CIMInitializeProviderAgentResponseMessage*
        _getInitializeProviderAgentResponseMessage();

    PEGASUS_HIDDEN_LINKAGE
    static CIMCreateSubscriptionResponseMessage*
        _getCreateSubscriptionResponseMessage();

    PEGASUS_HIDDEN_LINKAGE
    static CIMModifySubscriptionResponseMessage*
        _getModifySubscriptionResponseMessage();

    PEGASUS_HIDDEN_LINKAGE
    static CIMDeleteSubscriptionResponseMessage*
        _getDeleteSubscriptionResponseMessage();

    PEGASUS_HIDDEN_LINKAGE
    static CIMExportIndicationResponseMessage*
        _getExportIndicationResponseMessage();

    PEGASUS_HIDDEN_LINKAGE
    static CIMProcessIndicationResponseMessage*
        _getProcessIndicationResponseMessage();

    PEGASUS_HIDDEN_LINKAGE
    static CIMDisableModuleResponseMessage*
        _getDisableModuleResponseMessage(CIMBuffer& in);

    PEGASUS_HIDDEN_LINKAGE
    static CIMEnableModuleResponseMessage*
        _getEnableModuleResponseMessage(CIMBuffer& in);

    PEGASUS_HIDDEN_LINKAGE
    static CIMStopAllProvidersResponseMessage*
        _getStopAllProvidersResponseMessage();

    PEGASUS_HIDDEN_LINKAGE
    static CIMNotifyConfigChangeResponseMessage*
        _getNotifyConfigChangeResponseMessage();

    PEGASUS_HIDDEN_LINKAGE
    static CIMSubscriptionInitCompleteResponseMessage*
        _getSubscriptionInitCompleteResponseMessage();

    PEGASUS_HIDDEN_LINKAGE
    static CIMIndicationServiceDisabledResponseMessage*
        _getIndicationServiceDisabledResponseMessage();

    PEGASUS_HIDDEN_LINKAGE
    static ProvAgtGetScmoClassResponseMessage*
        _getProvAgtGetScmoClassResponseMessage(CIMBuffer& in);


};

PEGASUS_NAMESPACE_END

#endif /* Pegasus_CIMBinMsgDeserializer_h */
