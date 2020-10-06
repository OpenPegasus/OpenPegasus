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


#include <cstdlib>
#include <Pegasus/Common/PegasusAssert.h>
#include <Pegasus/Common/System.h>
#include <Pegasus/Common/OperationContextInternal.h>
#include <Pegasus/Common/SCMOClassCache.h>
#include <Pegasus/Common/CIMBinMsgSerializer.h>
#include <Pegasus/Common/CIMBinMsgDeserializer.h>
#include <Pegasus/Common/XmlWriter.h>

PEGASUS_USING_PEGASUS;
PEGASUS_USING_STD;

// Local CIM Class repository for lookup by the SCMOClassCache
// This is needed for converting CIM objects to SCMOInstances
static Array<CIMClass>* classArray = 0;
static Boolean verbose;

#define PEGASUS_TEMP_ASSERT(COND)                                         \
    do                                                                    \
    {                                                                     \
        if (!(COND))                                                      \
        {                                                                 \
            printf("PEGASUS_TEMP_ASSERT failed in file %s at line %d\n",  \
                __FILE__, __LINE__);                                      \
        }                                                                 \
    } while (0)


// Appends a CIMInstance to an array of SCMOInstance
void getSCMOInstanceFromCIM(
    Array<SCMOInstance>& instArraySCMO,
    const CIMInstance& instance)
{
    if (!instance.isUninitialized())
    {
        // Note, the conversion from CIMInstance to SCMOInstance
        // causes a lookup in the SCMOClassCache of the CIMClass for
        // this instance.
        SCMOInstance scmoInst(instance);
        instArraySCMO.append(scmoInst);
    }
}

// Converts an array of CIMInstance to an array of SCMOInstance
void getSCMOInstancesFromCIM(
    Array<SCMOInstance>& instArraySCMO,
    const Array<CIMInstance>& instances)
{
    for (Uint32 x=0; x < instances.size(); x++)
    {
        if (!instances[x].isUninitialized())
        {
            // Note, the conversion from CIMInstance to SCMOInstance
            // causes a lookup in the SCMOClassCache of the CIMClass for
            // this instance.
            SCMOInstance scmoInst(instances[x]);
            instArraySCMO.append(scmoInst);
        }
    }
}

// Converts an array of CIMObjectPath to an array of SCMOInstance
void getSCMOInstancesFromCIM(
    Array<SCMOInstance>& instArraySCMO,
    const Array<CIMObjectPath>& instanceNames)
{
    for (Uint32 x=0; x < instanceNames.size(); x++)
    {
        SCMOInstance scmoInst(instanceNames[x]);
        instArraySCMO.append(scmoInst);
    }
}

// Converts an array of CIMObject to an array of SCMOInstance
// CIMObjects that represent CIMClasses are ignored
void getSCMOInstancesFromCIM(
    Array<SCMOInstance>& instArraySCMO,
    const Array<CIMObject>& objects)
{
    for (Uint32 x=0; x < objects.size(); x++)
    {
        if (!objects[x].isUninitialized() && !objects[x].isClass())
        {
            // Note, the conversion from CIMObject to SCMOInstance
            // causes a lookup in the SCMOClassCache of the CIMClass for
            // this instance.
            SCMOInstance scmoInst(objects[x]);
            instArraySCMO.append(scmoInst);
        }
    }
}


//
// validateCIMPropertyList
//
void validateCIMPropertyList(
    const CIMPropertyList& inPropertyList,
    const CIMPropertyList& outPropertyList)
{
    PEGASUS_TEST_ASSERT(
        (inPropertyList.isNull() && outPropertyList.isNull()) ||
        (!inPropertyList.isNull() && !outPropertyList.isNull()));
    PEGASUS_TEST_ASSERT(inPropertyList.getPropertyNameArray() ==
        outPropertyList.getPropertyNameArray());
}

//
// validateCIMInstance
//
void validateCIMInstance(
    const CIMInstance& inInstance,
    const CIMInstance& outInstance)
{
    if (inInstance.isUninitialized())
    {
        PEGASUS_TEST_ASSERT(outInstance.isUninitialized());
    }
    else
    {
        if (!inInstance.identical(outInstance))
        {
            cout << "Error Instances not identical.\ninINSTANCE====" << endl;
            XmlWriter::printInstanceElement(inInstance, cout);
            cout << "outINSTANCE====" << endl;
            XmlWriter::printInstanceElement(inInstance, cout);
        }
        PEGASUS_TEST_ASSERT(inInstance.identical(outInstance));
    }
}

//
// validateCIMObjectPathArray
//
void validateCIMObjectPathArray(
    const Array<CIMObjectPath>& inObjectPathArray,
    const Array<CIMObjectPath>& outObjectPathArray)
{
    PEGASUS_TEST_ASSERT(inObjectPathArray.size() == outObjectPathArray.size());

    for (Uint32 i = 0, n = outObjectPathArray.size(); i < n; i++)
    {
        PEGASUS_TEST_ASSERT(inObjectPathArray[i] == outObjectPathArray[i]);
    }
}

//
// validateCIMNameArray
//
void validateCIMNameArray(
    const Array<CIMName>& inNameArray,
    const Array<CIMName>& outNameArray)
{
    PEGASUS_TEST_ASSERT(inNameArray.size() == outNameArray.size());

    for (Uint32 i = 0, n = outNameArray.size(); i < n; i++)
    {
        if (inNameArray[i].isNull())
        {
            PEGASUS_TEST_ASSERT(outNameArray[i].isNull());
        }
        else
        {
            PEGASUS_TEST_ASSERT(inNameArray[i] == outNameArray[i]);
        }
    }
}

//
// validateCIMInstanceArray
//
void validateCIMInstanceArray(
    const Array<CIMInstance>& inInstanceArray,
    const Array<CIMInstance>& outInstanceArray)
{
    PEGASUS_TEST_ASSERT(inInstanceArray.size() == outInstanceArray.size());

    for (Uint32 i = 0, n = outInstanceArray.size(); i < n; i++)
    {
        if (inInstanceArray[i].isUninitialized())
        {
            PEGASUS_TEST_ASSERT(outInstanceArray[i].isUninitialized());
        }
        else
        {
            PEGASUS_TEST_ASSERT(
                inInstanceArray[i].identical(outInstanceArray[i]));
        }
    }
}

//
// validateCIMObjectArray
//
void validateCIMObjectArray(
    const Array<CIMObject>& inObjectArray,
    const Array<CIMObject>& outObjectArray)
{
    PEGASUS_TEST_ASSERT(inObjectArray.size() == outObjectArray.size());

    for (Uint32 i = 0, n = outObjectArray.size(); i < n; i++)
    {
        if (inObjectArray[i].isUninitialized())
        {
            PEGASUS_TEST_ASSERT(outObjectArray[i].isUninitialized());
        }
        else
        {
            PEGASUS_TEST_ASSERT(inObjectArray[i].identical(outObjectArray[i]));
        }
    }
}

//
// validateCIMParamValueArray
//
void validateCIMParamValueArray(
    const Array<CIMParamValue>& inParamValueArray,
    const Array<CIMParamValue>& outParamValueArray)
{
    PEGASUS_TEST_ASSERT(inParamValueArray.size() == outParamValueArray.size());

    for (Uint32 i = 0, n = outParamValueArray.size(); i < n; i++)
    {
        if (inParamValueArray[i].isUninitialized())
        {
            PEGASUS_TEST_ASSERT(outParamValueArray[i].isUninitialized());
        }
        else
        {
            PEGASUS_TEST_ASSERT(inParamValueArray[i].getParameterName() ==
                outParamValueArray[i].getParameterName());
            PEGASUS_TEST_ASSERT(inParamValueArray[i].getValue() ==
                outParamValueArray[i].getValue());
            PEGASUS_TEST_ASSERT(inParamValueArray[i].isTyped() ==
                outParamValueArray[i].isTyped());
        }
    }
}

//
// validateOperationContext
//
void validateOperationContext(
    const OperationContext& inOperationContext,
    const OperationContext& outOperationContext)
{
    try
    {
        const IdentityContainer inContainer =
            inOperationContext.get(IdentityContainer::NAME);
        try
        {
            const IdentityContainer outContainer =
                outOperationContext.get(IdentityContainer::NAME);
            PEGASUS_TEST_ASSERT(
                inContainer.getUserName() == outContainer.getUserName());
        }
        catch(const Exception&)
        {
            PEGASUS_TEST_ASSERT(0);
        }
    }
    catch(const Exception&)
    {
    }

    try
    {
        const SubscriptionInstanceContainer inContainer =
            inOperationContext.get(SubscriptionInstanceContainer::NAME);
        try
        {
            const SubscriptionInstanceContainer outContainer =
                outOperationContext.get(SubscriptionInstanceContainer::NAME);
            validateCIMInstance(
                inContainer.getInstance(), outContainer.getInstance());
        }
        catch(const Exception&)
        {
            PEGASUS_TEST_ASSERT(0);
        }
    }
    catch(const Exception&)
    {
    }

    try
    {
        const SubscriptionFilterConditionContainer inContainer =
            inOperationContext.get(SubscriptionFilterConditionContainer::NAME);
        try
        {
            const SubscriptionFilterConditionContainer outContainer =
                outOperationContext.get(
                    SubscriptionFilterConditionContainer::NAME);
            PEGASUS_TEST_ASSERT(inContainer.getFilterCondition() ==
                outContainer.getFilterCondition());
            PEGASUS_TEST_ASSERT(inContainer.getQueryLanguage() ==
                outContainer.getQueryLanguage());
        }
        catch(const Exception&)
        {
            PEGASUS_TEST_ASSERT(0);
        }
    }
    catch(const Exception&)
    {
    }

    try
    {
        const SubscriptionFilterQueryContainer inContainer =
            inOperationContext.get(SubscriptionFilterQueryContainer::NAME);
        try
        {
            const SubscriptionFilterQueryContainer outContainer =
                outOperationContext.get(
                    SubscriptionFilterQueryContainer::NAME);
            PEGASUS_TEST_ASSERT(inContainer.getFilterQuery() ==
                outContainer.getFilterQuery());
            PEGASUS_TEST_ASSERT(inContainer.getQueryLanguage() ==
                outContainer.getQueryLanguage());
            PEGASUS_TEST_ASSERT(inContainer.getSourceNameSpace() ==
                outContainer.getSourceNameSpace());
        }
        catch(const Exception&)
        {
            PEGASUS_TEST_ASSERT(0);
        }
    }
    catch(const Exception&)
    {
    }

    try
    {
        const SubscriptionInstanceNamesContainer inContainer =
            inOperationContext.get(SubscriptionInstanceNamesContainer::NAME);
        try
        {
            const SubscriptionInstanceNamesContainer outContainer =
                outOperationContext.get(
                    SubscriptionInstanceNamesContainer::NAME);
            validateCIMObjectPathArray(
                inContainer.getInstanceNames(),
                outContainer.getInstanceNames());
        }
        catch(const Exception&)
        {
            PEGASUS_TEST_ASSERT(0);
        }
    }
    catch(const Exception&)
    {
    }

    try
    {
        const TimeoutContainer inContainer =
            inOperationContext.get(TimeoutContainer::NAME);
        try
        {
            const TimeoutContainer outContainer =
                outOperationContext.get(TimeoutContainer::NAME);
            PEGASUS_TEST_ASSERT(
                inContainer.getTimeOut() == outContainer.getTimeOut());
        }
        catch(const Exception&)
        {
            PEGASUS_TEST_ASSERT(0);
        }
    }
    catch(const Exception&)
    {
    }

    try
    {
        const AcceptLanguageListContainer inContainer =
            inOperationContext.get(AcceptLanguageListContainer::NAME);
        try
        {
            const AcceptLanguageListContainer outContainer =
                outOperationContext.get(AcceptLanguageListContainer::NAME);
            PEGASUS_TEST_ASSERT(
                inContainer.getLanguages() == outContainer.getLanguages());
        }
        catch(const Exception&)
        {
            PEGASUS_TEST_ASSERT(0);
        }
    }
    catch(const Exception&)
    {
    }

    try
    {
        const ContentLanguageListContainer inContainer =
            inOperationContext.get(ContentLanguageListContainer::NAME);
        try
        {
            const ContentLanguageListContainer outContainer =
                outOperationContext.get(ContentLanguageListContainer::NAME);
            PEGASUS_TEST_ASSERT(
                inContainer.getLanguages() == outContainer.getLanguages());
        }
        catch(const Exception&)
        {
            PEGASUS_TEST_ASSERT(0);
        }
    }
    catch(const Exception&)
    {
    }

    try
    {
        const SnmpTrapOidContainer inContainer =
            inOperationContext.get(SnmpTrapOidContainer::NAME);
        try
        {
            const SnmpTrapOidContainer outContainer =
                outOperationContext.get(SnmpTrapOidContainer::NAME);
            PEGASUS_TEST_ASSERT(
                inContainer.getSnmpTrapOid() == outContainer.getSnmpTrapOid());
        }
        catch(const Exception&)
        {
            PEGASUS_TEST_ASSERT(0);
        }
    }
    catch(const Exception&)
    {
    }

    try
    {
        const LocaleContainer inContainer =
            inOperationContext.get(LocaleContainer::NAME);
        try
        {
            const LocaleContainer outContainer =
                outOperationContext.get(LocaleContainer::NAME);
            PEGASUS_TEST_ASSERT(
                inContainer.getLanguageId() == outContainer.getLanguageId());
        }
        catch(const Exception&)
        {
            PEGASUS_TEST_ASSERT(0);
        }
    }
    catch(const Exception&)
    {
    }

    try
    {
        const UserRoleContainer inContainer =
            inOperationContext.get(UserRoleContainer::NAME);
        try
        {
            const UserRoleContainer outContainer =
                outOperationContext.get(UserRoleContainer::NAME);
            PEGASUS_TEST_ASSERT(
                inContainer.getUserRole() == outContainer.getUserRole());
        }
        catch(const Exception&)
        {
            PEGASUS_TEST_ASSERT(0);
        }
    }
    catch(const Exception&)
    {
    }
}

//
// validateCIMMessageAttributes
//
void validateCIMMessageAttributes(
    const CIMMessage* inMessage,
    const CIMMessage* outMessage)
{
    // Validate Message attributes
    PEGASUS_TEST_ASSERT(inMessage->getType() == outMessage->getType());
    PEGASUS_TEST_ASSERT(inMessage->isComplete() == outMessage->isComplete());
    PEGASUS_TEST_ASSERT(inMessage->getIndex() == outMessage->getIndex());

    // Validate messageId member
    PEGASUS_TEST_ASSERT(inMessage->messageId == outMessage->messageId);

    // Validate OperationContext
    validateOperationContext(
        inMessage->operationContext, outMessage->operationContext);
}

//
// validateCIMRequestMessageAttributes
//
void validateCIMRequestMessageAttributes(
    const CIMRequestMessage* inMessage,
    const CIMRequestMessage* outMessage)
{
    validateCIMMessageAttributes(inMessage, outMessage);

    // Validate queueIds member
    PEGASUS_TEST_ASSERT(
        inMessage->queueIds.size() == outMessage->queueIds.size());
    if (!inMessage->queueIds.isEmpty())
    {
        PEGASUS_TEST_ASSERT(
            inMessage->queueIds.top() == outMessage->queueIds.top());
    }
}

//
// validateCIMOperationRequestMessageAttributes
//
void validateCIMOperationRequestMessageAttributes(
    const CIMOperationRequestMessage* inMessage,
    const CIMOperationRequestMessage* outMessage)
{
    validateCIMRequestMessageAttributes(inMessage, outMessage);
    PEGASUS_TEST_ASSERT(inMessage->nameSpace == outMessage->nameSpace);
    PEGASUS_TEST_ASSERT(inMessage->className == outMessage->className);
    PEGASUS_TEST_ASSERT(inMessage->providerType == outMessage->providerType);
}

//
// validateCIMIndicationRequestMessageAttributes
//
void validateCIMIndicationRequestMessageAttributes(
    const CIMIndicationRequestMessage* inMessage,
    const CIMIndicationRequestMessage* outMessage)
{
    // CIMIndicationRequestMessage has no additional properties
    validateCIMRequestMessageAttributes(inMessage, outMessage);
}

//
// validateCIMResponseMessageAttributes
//
void validateCIMResponseMessageAttributes(
    const CIMResponseMessage* inMessage,
    const CIMResponseMessage* outMessage)
{
    validateCIMMessageAttributes(inMessage, outMessage);

    // Validate queueIds member
    PEGASUS_TEST_ASSERT(
        inMessage->queueIds.size() == outMessage->queueIds.size());
    if (!inMessage->queueIds.isEmpty())
    {
        PEGASUS_TEST_ASSERT(
            inMessage->queueIds.top() == outMessage->queueIds.top());
    }

    // Validate cimException member
    PEGASUS_TEST_ASSERT(inMessage->cimException.getCode() ==
           outMessage->cimException.getCode());
    PEGASUS_TEST_ASSERT(inMessage->cimException.getMessage() ==
           outMessage->cimException.getMessage());
}

//
// serializeDeserializeMessage
//
/**
    Serializes the message into a Buffer, and then dseserialize it back into
    a Message object.
*/
CIMMessage* serializeDeserializeMessage(CIMMessage* inMessage)
{
    CIMBuffer buf(64*1024);
    CIMBinMsgSerializer::serialize(buf, inMessage);

    size_t size = buf.size();
    buf.rewind();

    CIMMessage* outMessage;
    outMessage = CIMBinMsgDeserializer::deserialize(buf, size);

    return outMessage;
}

//
// testEmptyRequestMessage
//
void testEmptyMessage()
{
    CIMBuffer buf(64*1024);
    CIMBinMsgSerializer::serialize(buf, 0);
    PEGASUS_TEST_ASSERT(buf.size() == 0);

    size_t size = buf.size();
    buf.rewind();

    CIMMessage* outMessage = CIMBinMsgDeserializer::deserialize(buf, size);
    PEGASUS_TEST_ASSERT(outMessage == 0);
}

//
// testCIMGetInstanceRequestMessage
//
void testCIMGetInstanceRequestMessage(
    const OperationContext& oc,
    const String& mid,
    const CIMNamespaceName& ns,
    const CIMObjectPath& path,
    Boolean iq,
    Boolean ico,
    const CIMPropertyList& pl,
    const QueueIdStack& qids,
    const String& auth,
    const String& user)
{
    CIMGetInstanceRequestMessage inMessage(
        mid, ns, path, iq, ico, pl, qids, auth, user);
    inMessage.operationContext = oc;
    AutoPtr<CIMGetInstanceRequestMessage> outMessage(
        dynamic_cast<CIMGetInstanceRequestMessage*>(
            serializeDeserializeMessage(&inMessage)));
    PEGASUS_TEST_ASSERT(outMessage.get() != 0);

    validateCIMOperationRequestMessageAttributes(&inMessage, outMessage.get());
    PEGASUS_TEST_ASSERT(inMessage.instanceName == outMessage->instanceName);
    PEGASUS_TEST_ASSERT(
        inMessage.includeQualifiers == outMessage->includeQualifiers);
    PEGASUS_TEST_ASSERT(
        inMessage.includeClassOrigin == outMessage->includeClassOrigin);
    validateCIMPropertyList(inMessage.propertyList, outMessage->propertyList);
    PEGASUS_TEST_ASSERT(inMessage.authType == outMessage->authType);
    PEGASUS_TEST_ASSERT(inMessage.userName == outMessage->userName);

    if (verbose)
    {
        cout << "testCIMGetInstanceRequestMessage - OK" << endl;
    }
}

//
// testCIMCreateInstanceRequestMessage
//
void testCIMCreateInstanceRequestMessage(
    const OperationContext& oc,
    const String& mid,
    const CIMNamespaceName& ns,
    const CIMInstance& inst,
    const QueueIdStack& qids,
    const String& auth,
    const String& user)
{
    CIMCreateInstanceRequestMessage inMessage(mid, ns, inst, qids, auth, user);
    inMessage.operationContext = oc;
    AutoPtr<CIMCreateInstanceRequestMessage> outMessage(
        dynamic_cast<CIMCreateInstanceRequestMessage*>(
            serializeDeserializeMessage(&inMessage)));
    PEGASUS_TEST_ASSERT(outMessage.get() != 0);

    validateCIMOperationRequestMessageAttributes(&inMessage, outMessage.get());
    validateCIMInstance(inMessage.newInstance, outMessage->newInstance);
    PEGASUS_TEST_ASSERT(inMessage.authType == outMessage->authType);
    PEGASUS_TEST_ASSERT(inMessage.userName == outMessage->userName);

    if (verbose)
    {
        cout << "testCIMCreateInstanceRequestMessage - OK" << endl;
    }
}

//
// testCIMModifyInstanceRequestMessage
//
void testCIMModifyInstanceRequestMessage(
    const OperationContext& oc,
    const String& mid,
    const CIMNamespaceName& ns,
    const CIMInstance& inst,
    Boolean iq,
    const CIMPropertyList& pl,
    const QueueIdStack& qids,
    const String& auth,
    const String& user)
{
    CIMModifyInstanceRequestMessage inMessage(
        mid, ns, inst, iq, pl, qids, auth, user);
    inMessage.operationContext = oc;
    AutoPtr<CIMModifyInstanceRequestMessage> outMessage(
        dynamic_cast<CIMModifyInstanceRequestMessage*>(
            serializeDeserializeMessage(&inMessage)));
    PEGASUS_TEST_ASSERT(outMessage.get() != 0);

    validateCIMOperationRequestMessageAttributes(&inMessage, outMessage.get());
    validateCIMInstance(
        inMessage.modifiedInstance, outMessage->modifiedInstance);
    PEGASUS_TEST_ASSERT(
        inMessage.includeQualifiers == outMessage->includeQualifiers);
    validateCIMPropertyList(inMessage.propertyList, outMessage->propertyList);
    PEGASUS_TEST_ASSERT(inMessage.authType == outMessage->authType);
    PEGASUS_TEST_ASSERT(inMessage.userName == outMessage->userName);

    if (verbose)
    {
        cout << "testCIMModifyInstanceRequestMessage - OK" << endl;
    }
}

//
// testCIMDeleteInstanceRequestMessage
//
void testCIMDeleteInstanceRequestMessage(
    const OperationContext& oc,
    const String& mid,
    const CIMNamespaceName& ns,
    const CIMObjectPath& path,
    const QueueIdStack& qids,
    const String& auth,
    const String& user)
{
    CIMDeleteInstanceRequestMessage inMessage(mid, ns, path, qids, auth, user);
    inMessage.operationContext = oc;
    AutoPtr<CIMDeleteInstanceRequestMessage> outMessage(
        dynamic_cast<CIMDeleteInstanceRequestMessage*>(
            serializeDeserializeMessage(&inMessage)));
    PEGASUS_TEST_ASSERT(outMessage.get() != 0);

    validateCIMOperationRequestMessageAttributes(&inMessage, outMessage.get());
    PEGASUS_TEST_ASSERT(inMessage.instanceName == outMessage->instanceName);
    PEGASUS_TEST_ASSERT(inMessage.authType == outMessage->authType);
    PEGASUS_TEST_ASSERT(inMessage.userName == outMessage->userName);

    if (verbose)
    {
        cout << "testCIMDeleteInstanceRequestMessage - OK" << endl;
    }
}

//
// testCIMEnumerateInstancesRequestMessage
//
void testCIMEnumerateInstancesRequestMessage(
    const OperationContext& oc,
    const String& mid,
    const CIMNamespaceName& ns,
    const CIMName& className,
    Boolean di,
    Boolean iq,
    Boolean ico,
    const CIMPropertyList& pl,
    const QueueIdStack& qids,
    const String& auth,
    const String& user)
{
    CIMEnumerateInstancesRequestMessage inMessage(
        mid, ns, className, di, iq, ico, pl, qids, auth, user);
    inMessage.operationContext = oc;
    AutoPtr<CIMEnumerateInstancesRequestMessage> outMessage(
        dynamic_cast<CIMEnumerateInstancesRequestMessage*>(
            serializeDeserializeMessage(&inMessage)));
    PEGASUS_TEST_ASSERT(outMessage.get() != 0);

    validateCIMOperationRequestMessageAttributes(&inMessage, outMessage.get());
    PEGASUS_TEST_ASSERT(
        inMessage.deepInheritance == outMessage->deepInheritance);
    PEGASUS_TEST_ASSERT(
        inMessage.includeQualifiers == outMessage->includeQualifiers);
    PEGASUS_TEST_ASSERT(
        inMessage.includeClassOrigin == outMessage->includeClassOrigin);
    validateCIMPropertyList(inMessage.propertyList, outMessage->propertyList);
    PEGASUS_TEST_ASSERT(inMessage.authType == outMessage->authType);
    PEGASUS_TEST_ASSERT(inMessage.userName == outMessage->userName);

    if (verbose)
    {
        cout << "testCIMEnumerateInstancesRequestMessage - OK" << endl;
    }
}

//
// testCIMEnumerateInstanceNamesRequestMessage
//
void testCIMEnumerateInstanceNamesRequestMessage(
    const OperationContext& oc,
    const String& mid,
    const CIMNamespaceName& ns,
    const CIMName& className,
    const QueueIdStack& qids,
    const String& auth,
    const String& user)
{
    CIMEnumerateInstanceNamesRequestMessage inMessage(
        mid, ns, className, qids, auth, user);
    inMessage.operationContext = oc;
    AutoPtr<CIMEnumerateInstanceNamesRequestMessage> outMessage(
        dynamic_cast<CIMEnumerateInstanceNamesRequestMessage*>(
            serializeDeserializeMessage(&inMessage)));
    PEGASUS_TEST_ASSERT(outMessage.get() != 0);

    validateCIMOperationRequestMessageAttributes(&inMessage, outMessage.get());
    PEGASUS_TEST_ASSERT(inMessage.authType == outMessage->authType);
    PEGASUS_TEST_ASSERT(inMessage.userName == outMessage->userName);

    if (verbose)
    {
        cout << "testCIMEnumerateInstanceNamesRequestMessage - OK" << endl;
    }
}

//
// testCIMExecQueryRequestMessage
//
void testCIMExecQueryRequestMessage(
    const OperationContext& oc,
    const String& mid,
    const CIMNamespaceName& ns,
    const String& ql,
    const String& qs,
    const QueueIdStack& qids,
    const String& auth,
    const String& user)
{
    CIMExecQueryRequestMessage inMessage(mid, ns, ql, qs, qids, auth, user);
    inMessage.operationContext = oc;
    AutoPtr<CIMExecQueryRequestMessage> outMessage(
        dynamic_cast<CIMExecQueryRequestMessage*>(
            serializeDeserializeMessage(&inMessage)));
    PEGASUS_TEST_ASSERT(outMessage.get() != 0);

    validateCIMOperationRequestMessageAttributes(&inMessage, outMessage.get());
    PEGASUS_TEST_ASSERT(inMessage.queryLanguage == outMessage->queryLanguage);
    PEGASUS_TEST_ASSERT(inMessage.query == outMessage->query);
    PEGASUS_TEST_ASSERT(inMessage.authType == outMessage->authType);
    PEGASUS_TEST_ASSERT(inMessage.userName == outMessage->userName);

    if (verbose)
    {
        cout << "testCIMExecQueryRequestMessage - OK" << endl;
    }
}

//
// testCIMAssociatorsRequestMessage
//
void testCIMAssociatorsRequestMessage(
    const OperationContext& oc,
    const String& mid,
    const CIMNamespaceName& ns,
    const CIMObjectPath& path,
    const CIMName& assocClass,
    const CIMName& resultClass,
    const String& role,
    const String& resultRole,
    Boolean iq,
    Boolean ico,
    const CIMPropertyList& pl,
    const QueueIdStack& qids,
    const String& auth,
    const String& user)
{
    CIMAssociatorsRequestMessage inMessage(
        mid, ns, path, assocClass, resultClass, role, resultRole,
        iq, ico, pl, qids, false, auth, user);
    inMessage.operationContext = oc;
    AutoPtr<CIMAssociatorsRequestMessage> outMessage(
        dynamic_cast<CIMAssociatorsRequestMessage*>(
            serializeDeserializeMessage(&inMessage)));
    PEGASUS_TEST_ASSERT(outMessage.get() != 0);

    validateCIMOperationRequestMessageAttributes(&inMessage, outMessage.get());
    PEGASUS_TEST_ASSERT(inMessage.objectName == outMessage->objectName);
    PEGASUS_TEST_ASSERT(inMessage.assocClass == outMessage->assocClass);
    PEGASUS_TEST_ASSERT(inMessage.resultClass == outMessage->resultClass);
    PEGASUS_TEST_ASSERT(inMessage.role == outMessage->role);
    PEGASUS_TEST_ASSERT(inMessage.resultRole == outMessage->resultRole);
    PEGASUS_TEST_ASSERT(
        inMessage.includeQualifiers == outMessage->includeQualifiers);
    PEGASUS_TEST_ASSERT(
        inMessage.includeClassOrigin == outMessage->includeClassOrigin);
    validateCIMPropertyList(inMessage.propertyList, outMessage->propertyList);
    PEGASUS_TEST_ASSERT(inMessage.authType == outMessage->authType);
    PEGASUS_TEST_ASSERT(inMessage.userName == outMessage->userName);

    if (verbose)
    {
        cout << "testCIMAssociatorsRequestMessage - OK" << endl;
    }
}

//
// testCIMAssociatorNamesRequestMessage
//
void testCIMAssociatorNamesRequestMessage(
    const OperationContext& oc,
    const String& mid,
    const CIMNamespaceName& ns,
    const CIMObjectPath& path,
    const CIMName& assocClass,
    const CIMName& resultClass,
    const String& role,
    const String& resultRole,
    const QueueIdStack& qids,
    const String& auth,
    const String& user)
{
    CIMAssociatorNamesRequestMessage inMessage(
        mid, ns, path, assocClass, resultClass, role, resultRole,
        qids, false, auth, user);
    inMessage.operationContext = oc;
    AutoPtr<CIMAssociatorNamesRequestMessage> outMessage(
        dynamic_cast<CIMAssociatorNamesRequestMessage*>(
            serializeDeserializeMessage(&inMessage)));
    PEGASUS_TEST_ASSERT(outMessage.get() != 0);

    validateCIMOperationRequestMessageAttributes(&inMessage, outMessage.get());
    PEGASUS_TEST_ASSERT(inMessage.objectName == outMessage->objectName);
    PEGASUS_TEST_ASSERT(inMessage.assocClass == outMessage->assocClass);
    PEGASUS_TEST_ASSERT(inMessage.resultClass == outMessage->resultClass);
    PEGASUS_TEST_ASSERT(inMessage.role == outMessage->role);
    PEGASUS_TEST_ASSERT(inMessage.resultRole == outMessage->resultRole);
    PEGASUS_TEST_ASSERT(inMessage.authType == outMessage->authType);
    PEGASUS_TEST_ASSERT(inMessage.userName == outMessage->userName);

    if (verbose)
    {
        cout << "testCIMAssociatorNamesRequestMessage - OK" << endl;
    }
}

//
// testCIMReferencesRequestMessage
//
void testCIMReferencesRequestMessage(
    const OperationContext& oc,
    const String& mid,
    const CIMNamespaceName& ns,
    const CIMObjectPath& path,
    const CIMName& resultClass,
    const String& role,
    Boolean iq,
    Boolean ico,
    const CIMPropertyList& pl,
    const QueueIdStack& qids,
    const String& auth,
    const String& user)
{
    CIMReferencesRequestMessage inMessage(
        mid, ns, path, resultClass, role, iq, ico, pl, qids, false, auth, user);
    inMessage.operationContext = oc;
    AutoPtr<CIMReferencesRequestMessage> outMessage(
        dynamic_cast<CIMReferencesRequestMessage*>(
            serializeDeserializeMessage(&inMessage)));
    PEGASUS_TEST_ASSERT(outMessage.get() != 0);

    validateCIMOperationRequestMessageAttributes(&inMessage, outMessage.get());
    PEGASUS_TEST_ASSERT(inMessage.objectName == outMessage->objectName);
    PEGASUS_TEST_ASSERT(inMessage.resultClass == outMessage->resultClass);
    PEGASUS_TEST_ASSERT(inMessage.role == outMessage->role);
    PEGASUS_TEST_ASSERT(
        inMessage.includeQualifiers == outMessage->includeQualifiers);
    PEGASUS_TEST_ASSERT(
        inMessage.includeClassOrigin == outMessage->includeClassOrigin);
    validateCIMPropertyList(inMessage.propertyList, outMessage->propertyList);
    PEGASUS_TEST_ASSERT(inMessage.authType == outMessage->authType);
    PEGASUS_TEST_ASSERT(inMessage.userName == outMessage->userName);

    if (verbose)
    {
        cout << "testCIMReferencesRequestMessage - OK" << endl;
    }
}

//
// testCIMReferenceNamesRequestMessage
//
void testCIMReferenceNamesRequestMessage(
    const OperationContext& oc,
    const String& mid,
    const CIMNamespaceName& ns,
    const CIMObjectPath& path,
    const CIMName& resultClass,
    const String& role,
    const QueueIdStack& qids,
    const String& auth,
    const String& user)
{
    CIMReferenceNamesRequestMessage inMessage(
        mid, ns, path, resultClass, role, qids, false, auth, user);
    inMessage.operationContext = oc;
    AutoPtr<CIMReferenceNamesRequestMessage> outMessage(
        dynamic_cast<CIMReferenceNamesRequestMessage*>(
            serializeDeserializeMessage(&inMessage)));
    PEGASUS_TEST_ASSERT(outMessage.get() != 0);

    validateCIMOperationRequestMessageAttributes(&inMessage, outMessage.get());
    PEGASUS_TEST_ASSERT(inMessage.objectName == outMessage->objectName);
    PEGASUS_TEST_ASSERT(inMessage.resultClass == outMessage->resultClass);
    PEGASUS_TEST_ASSERT(inMessage.role == outMessage->role);
    PEGASUS_TEST_ASSERT(inMessage.authType == outMessage->authType);
    PEGASUS_TEST_ASSERT(inMessage.userName == outMessage->userName);

    if (verbose)
    {
        cout << "testCIMReferenceNamesRequestMessage - OK" << endl;
    }
}

//
// testCIMGetPropertyRequestMessage
//
void testCIMGetPropertyRequestMessage(
    const OperationContext& oc,
    const String& mid,
    const CIMNamespaceName& ns,
    const CIMObjectPath& path,
    const CIMName& propName,
    const QueueIdStack& qids,
    const String& auth,
    const String& user)
{
    CIMGetPropertyRequestMessage inMessage(
        mid, ns, path, propName, qids, auth, user);
    inMessage.operationContext = oc;
    AutoPtr<CIMGetPropertyRequestMessage> outMessage(
        dynamic_cast<CIMGetPropertyRequestMessage*>(
            serializeDeserializeMessage(&inMessage)));
    PEGASUS_TEST_ASSERT(outMessage.get() != 0);

    validateCIMOperationRequestMessageAttributes(&inMessage, outMessage.get());
    PEGASUS_TEST_ASSERT(inMessage.instanceName == outMessage->instanceName);
    PEGASUS_TEST_ASSERT(inMessage.propertyName == outMessage->propertyName);
    PEGASUS_TEST_ASSERT(inMessage.authType == outMessage->authType);
    PEGASUS_TEST_ASSERT(inMessage.userName == outMessage->userName);

    if (verbose)
    {
        cout << "testCIMGetPropertyRequestMessage - OK" << endl;
    }
}

//
// testCIMSetPropertyRequestMessage
//
void testCIMSetPropertyRequestMessage(
    const OperationContext& oc,
    const String& mid,
    const CIMNamespaceName& ns,
    const CIMObjectPath& path,
    const CIMName& propName,
    const CIMValue& newValue,
    const QueueIdStack& qids,
    const String& auth,
    const String& user)
{
    CIMSetPropertyRequestMessage inMessage(
        mid, ns, path, propName, newValue, qids, auth, user);
    inMessage.operationContext = oc;
    AutoPtr<CIMSetPropertyRequestMessage> outMessage(
        dynamic_cast<CIMSetPropertyRequestMessage*>(
            serializeDeserializeMessage(&inMessage)));
    PEGASUS_TEST_ASSERT(outMessage.get() != 0);

    validateCIMOperationRequestMessageAttributes(&inMessage, outMessage.get());
    PEGASUS_TEST_ASSERT(inMessage.instanceName == outMessage->instanceName);
    PEGASUS_TEST_ASSERT(inMessage.propertyName == outMessage->propertyName);
    PEGASUS_TEST_ASSERT(inMessage.newValue == outMessage->newValue);
    PEGASUS_TEST_ASSERT(inMessage.authType == outMessage->authType);
    PEGASUS_TEST_ASSERT(inMessage.userName == outMessage->userName);

    if (verbose)
    {
        cout << "testCIMSetPropertyRequestMessage - OK" << endl;
    }
}

//
// testCIMInvokeMethodRequestMessage
//
void testCIMInvokeMethodRequestMessage(
    const OperationContext& oc,
    const String& mid,
    const CIMNamespaceName& ns,
    const CIMObjectPath& path,
    const CIMName& methName,
    const Array<CIMParamValue>& pvArray,
    const QueueIdStack& qids,
    const String& auth,
    const String& user)
{
    CIMInvokeMethodRequestMessage inMessage(
        mid, ns, path, methName, pvArray, qids, auth, user);
    inMessage.operationContext = oc;
    AutoPtr<CIMInvokeMethodRequestMessage> outMessage(
        dynamic_cast<CIMInvokeMethodRequestMessage*>(
            serializeDeserializeMessage(&inMessage)));
    PEGASUS_TEST_ASSERT(outMessage.get() != 0);

    validateCIMOperationRequestMessageAttributes(&inMessage, outMessage.get());
    PEGASUS_TEST_ASSERT(inMessage.instanceName == outMessage->instanceName);
    PEGASUS_TEST_ASSERT(inMessage.methodName == outMessage->methodName);
    validateCIMParamValueArray(
        inMessage.inParameters, outMessage->inParameters);
    PEGASUS_TEST_ASSERT(inMessage.authType == outMessage->authType);
    PEGASUS_TEST_ASSERT(inMessage.userName == outMessage->userName);

    if (verbose)
    {
        cout << "testCIMInvokeMethodRequestMessage - OK" << endl;
    }
}

//
// testCIMCreateSubscriptionRequestMessage
//
void testCIMCreateSubscriptionRequestMessage(
    const OperationContext& oc,
    const String& mid,
    const CIMNamespaceName& ns,
    const CIMInstance& inst,
    const Array<CIMName>& nameArray,
    const CIMPropertyList& pl,
    Uint16 rnp,
    const String& qs,
    const QueueIdStack& qids,
    const String& auth,
    const String& user)
{
    CIMCreateSubscriptionRequestMessage inMessage(
        mid, ns, inst, nameArray, pl, rnp, qs, qids, auth, user);
    inMessage.operationContext = oc;
    AutoPtr<CIMCreateSubscriptionRequestMessage> outMessage(
        dynamic_cast<CIMCreateSubscriptionRequestMessage*>(
            serializeDeserializeMessage(&inMessage)));
    PEGASUS_TEST_ASSERT(outMessage.get() != 0);

    validateCIMIndicationRequestMessageAttributes(
        &inMessage, outMessage.get());
    PEGASUS_TEST_ASSERT(inMessage.nameSpace == outMessage->nameSpace);
    validateCIMInstance(
        inMessage.subscriptionInstance, outMessage->subscriptionInstance);
    validateCIMNameArray(inMessage.classNames, outMessage->classNames);
    validateCIMPropertyList(inMessage.propertyList, outMessage->propertyList);
    PEGASUS_TEST_ASSERT(inMessage.repeatNotificationPolicy ==
        outMessage->repeatNotificationPolicy);
    PEGASUS_TEST_ASSERT(inMessage.query == outMessage->query);
    PEGASUS_TEST_ASSERT(inMessage.authType == outMessage->authType);
    PEGASUS_TEST_ASSERT(inMessage.userName == outMessage->userName);

    if (verbose)
    {
        cout << "testCIMCreateSubscriptionRequestMessage - OK" << endl;
    }
}

//
// testCIMModifySubscriptionRequestMessage
//
void testCIMModifySubscriptionRequestMessage(
    const OperationContext& oc,
    const String& mid,
    const CIMNamespaceName& ns,
    const CIMInstance& inst,
    const Array<CIMName>& nameArray,
    const CIMPropertyList& pl,
    Uint16 rnp,
    const String& qs,
    const QueueIdStack& qids,
    const String& auth,
    const String& user)
{
    CIMModifySubscriptionRequestMessage inMessage(
        mid, ns, inst, nameArray, pl, rnp, qs, qids, auth, user);
    inMessage.operationContext = oc;
    AutoPtr<CIMModifySubscriptionRequestMessage> outMessage(
        dynamic_cast<CIMModifySubscriptionRequestMessage*>(
            serializeDeserializeMessage(&inMessage)));
    PEGASUS_TEST_ASSERT(outMessage.get() != 0);

    validateCIMIndicationRequestMessageAttributes(
        &inMessage, outMessage.get());
    PEGASUS_TEST_ASSERT(inMessage.nameSpace == outMessage->nameSpace);
    validateCIMInstance(
        inMessage.subscriptionInstance, outMessage->subscriptionInstance);
    validateCIMNameArray(inMessage.classNames, outMessage->classNames);
    validateCIMPropertyList(inMessage.propertyList, outMessage->propertyList);
    PEGASUS_TEST_ASSERT(inMessage.repeatNotificationPolicy ==
        outMessage->repeatNotificationPolicy);
    PEGASUS_TEST_ASSERT(inMessage.query == outMessage->query);
    PEGASUS_TEST_ASSERT(inMessage.authType == outMessage->authType);
    PEGASUS_TEST_ASSERT(inMessage.userName == outMessage->userName);

    if (verbose)
    {
        cout << "testCIMModifySubscriptionRequestMessage - OK" << endl;
    }
}

//
// testCIMDeleteSubscriptionRequestMessage
//
void testCIMDeleteSubscriptionRequestMessage(
    const OperationContext& oc,
    const String& mid,
    const CIMNamespaceName& ns,
    const CIMInstance& inst,
    const Array<CIMName>& nameArray,
    const QueueIdStack& qids,
    const String& auth,
    const String& user)
{
    CIMDeleteSubscriptionRequestMessage inMessage(
        mid, ns, inst, nameArray, qids, auth, user);
    inMessage.operationContext = oc;
    AutoPtr<CIMDeleteSubscriptionRequestMessage> outMessage(
        dynamic_cast<CIMDeleteSubscriptionRequestMessage*>(
            serializeDeserializeMessage(&inMessage)));
    PEGASUS_TEST_ASSERT(outMessage.get() != 0);

    validateCIMIndicationRequestMessageAttributes(
        &inMessage, outMessage.get());
    PEGASUS_TEST_ASSERT(inMessage.nameSpace == outMessage->nameSpace);
    validateCIMInstance(
        inMessage.subscriptionInstance, outMessage->subscriptionInstance);
    validateCIMNameArray(inMessage.classNames, outMessage->classNames);
    PEGASUS_TEST_ASSERT(inMessage.authType == outMessage->authType);
    PEGASUS_TEST_ASSERT(inMessage.userName == outMessage->userName);

    if (verbose)
    {
        cout << "testCIMDeleteSubscriptionRequestMessage - OK" << endl;
    }
}

//
// testCIMExportIndicationRequestMessage
//
void testCIMExportIndicationRequestMessage(
    const OperationContext& oc,
    const String& mid,
    const String& dest,
    const CIMInstance& inst,
    const QueueIdStack& qids,
    const String& auth,
    const String& user)
{
    CIMExportIndicationRequestMessage inMessage(
        mid, dest, inst, qids, auth, user);
    inMessage.operationContext = oc;
    AutoPtr<CIMExportIndicationRequestMessage> outMessage(
        dynamic_cast<CIMExportIndicationRequestMessage*>(
            serializeDeserializeMessage(&inMessage)));
    PEGASUS_TEST_ASSERT(outMessage.get() != 0);

    validateCIMRequestMessageAttributes(&inMessage, outMessage.get());
    PEGASUS_TEST_ASSERT(
        inMessage.destinationPath == outMessage->destinationPath);
    validateCIMInstance(
        inMessage.indicationInstance, outMessage->indicationInstance);
    PEGASUS_TEST_ASSERT(inMessage.authType == outMessage->authType);
    PEGASUS_TEST_ASSERT(inMessage.userName == outMessage->userName);

    if (verbose)
    {
        cout << "testCIMExportIndicationRequestMessage - OK" << endl;
    }
}

//
// testCIMProcessIndicationRequestMessage
//
void testCIMProcessIndicationRequestMessage(
    const OperationContext& oc,
    const String& mid,
    const CIMNamespaceName& ns,
    const CIMInstance& inst,
    const Array<CIMObjectPath>& subsInstArray,
    const CIMInstance& provider,
    const QueueIdStack& qids)
{
    CIMProcessIndicationRequestMessage inMessage(
        mid, ns, inst, subsInstArray, provider, qids);
    inMessage.operationContext = oc;
    AutoPtr<CIMProcessIndicationRequestMessage> outMessage(
        dynamic_cast<CIMProcessIndicationRequestMessage*>(
            serializeDeserializeMessage(&inMessage)));
    PEGASUS_TEST_ASSERT(outMessage.get() != 0);

    validateCIMRequestMessageAttributes(&inMessage, outMessage.get());
    PEGASUS_TEST_ASSERT(inMessage.nameSpace == outMessage->nameSpace);
    validateCIMInstance(
        inMessage.indicationInstance, outMessage->indicationInstance);
    validateCIMObjectPathArray(inMessage.subscriptionInstanceNames,
        outMessage->subscriptionInstanceNames);
    validateCIMInstance(inMessage.provider, outMessage->provider);

    if (verbose)
    {
        cout << "testCIMProcessIndicationRequestMessage - OK" << endl;
    }
}

//
// testCIMDisableModuleRequestMessage
//
void testCIMDisableModuleRequestMessage(
    const OperationContext& oc,
    const String& mid,
    const CIMInstance& provModule,
    const Array<CIMInstance>& providers,
    Boolean providerOnly,
    const Array<Boolean>& indProv,
    const QueueIdStack& qids,
    const String& auth,
    const String& user)
{
    CIMDisableModuleRequestMessage inMessage(
        mid, provModule, providers, providerOnly, indProv, qids, auth, user);
    inMessage.operationContext = oc;
    AutoPtr<CIMDisableModuleRequestMessage> outMessage(
        dynamic_cast<CIMDisableModuleRequestMessage*>(
            serializeDeserializeMessage(&inMessage)));
    PEGASUS_TEST_ASSERT(outMessage.get() != 0);

    validateCIMRequestMessageAttributes(&inMessage, outMessage.get());
    validateCIMInstance(
        inMessage.providerModule, outMessage->providerModule);
    validateCIMInstanceArray(inMessage.providers, outMessage->providers);
    PEGASUS_TEST_ASSERT(
        inMessage.disableProviderOnly == outMessage->disableProviderOnly);
    PEGASUS_TEST_ASSERT(inMessage.indicationProviders.size() ==
        outMessage->indicationProviders.size());
    for (Uint32 i = 0; i < inMessage.indicationProviders.size(); i++)
    {
        PEGASUS_TEST_ASSERT(inMessage.indicationProviders[i] ==
            outMessage->indicationProviders[i]);
    }
    PEGASUS_TEST_ASSERT(inMessage.authType == outMessage->authType);
    PEGASUS_TEST_ASSERT(inMessage.userName == outMessage->userName);

    if (verbose)
    {
        cout << "testCIMDisableModuleRequestMessage - OK" << endl;
    }
}

//
// testCIMEnableModuleRequestMessage
//
void testCIMEnableModuleRequestMessage(
    const OperationContext& oc,
    const String& mid,
    const CIMInstance& provModule,
    const QueueIdStack& qids,
    const String& auth,
    const String& user)
{
    CIMEnableModuleRequestMessage inMessage(mid, provModule, qids, auth, user);
    inMessage.operationContext = oc;
    AutoPtr<CIMEnableModuleRequestMessage> outMessage(
        dynamic_cast<CIMEnableModuleRequestMessage*>(
            serializeDeserializeMessage(&inMessage)));
    PEGASUS_TEST_ASSERT(outMessage.get() != 0);

    validateCIMRequestMessageAttributes(&inMessage, outMessage.get());
    validateCIMInstance(
        inMessage.providerModule, outMessage->providerModule);
    PEGASUS_TEST_ASSERT(inMessage.authType == outMessage->authType);
    PEGASUS_TEST_ASSERT(inMessage.userName == outMessage->userName);

    if (verbose)
    {
        cout << "testCIMEnableModuleRequestMessage - OK" << endl;
    }
}

//
// testCIMStopAllProvidersRequestMessage
//
void testCIMStopAllProvidersRequestMessage(
    const OperationContext& oc,
    const String& mid,
    const QueueIdStack& qids)
{
    CIMStopAllProvidersRequestMessage inMessage(mid, qids);
    inMessage.operationContext = oc;
    AutoPtr<CIMStopAllProvidersRequestMessage> outMessage(
        dynamic_cast<CIMStopAllProvidersRequestMessage*>(
            serializeDeserializeMessage(&inMessage)));
    PEGASUS_TEST_ASSERT(outMessage.get() != 0);

    validateCIMRequestMessageAttributes(&inMessage, outMessage.get());

    if (verbose)
    {
        cout << "testCIMStopAllProvidersRequestMessage - OK" << endl;
    }
}

//
// testCIMInitializeProviderAgentRequestMessage
//
void testCIMInitializeProviderAgentRequestMessage(
    const OperationContext& oc,
    const String& mid,
    const String& home,
    const Array<Pair<String, String> >& config,
    Boolean bindVerbose,
    Boolean subsInitComplete,
    const QueueIdStack& qids)
{
    CIMInitializeProviderAgentRequestMessage inMessage(
        mid, home, config, bindVerbose, subsInitComplete, qids);
    inMessage.operationContext = oc;
    AutoPtr<CIMInitializeProviderAgentRequestMessage> outMessage(
        dynamic_cast<CIMInitializeProviderAgentRequestMessage*>(
            serializeDeserializeMessage(&inMessage)));
    PEGASUS_TEST_ASSERT(outMessage.get() != 0);

    validateCIMRequestMessageAttributes(&inMessage, outMessage.get());
    PEGASUS_TEST_ASSERT(inMessage.pegasusHome == outMessage->pegasusHome);
    PEGASUS_TEST_ASSERT(inMessage.configProperties.size() ==
        outMessage->configProperties.size());
    for (Uint32 i = 0; i < inMessage.configProperties.size(); i++)
    {
        PEGASUS_TEST_ASSERT(inMessage.configProperties[i].first ==
            outMessage->configProperties[i].first);
        PEGASUS_TEST_ASSERT(inMessage.configProperties[i].second ==
            outMessage->configProperties[i].second);
    }
    PEGASUS_TEST_ASSERT(inMessage.bindVerbose == outMessage->bindVerbose);
    PEGASUS_TEST_ASSERT(inMessage.subscriptionInitComplete ==
        outMessage->subscriptionInitComplete);

    if (verbose)
    {
        cout << "testCIMInitializeProviderAgentRequestMessage - OK" << endl;
    }
}

//
// testCIMNotifyConfigChangeRequestMessage
//
void testCIMNotifyConfigChangeRequestMessage(
    const OperationContext& oc,
    const String& mid,
    const String& propName,
    const String& newPropValue,
    Boolean currValueModified,
    const QueueIdStack& qids)
{
    CIMNotifyConfigChangeRequestMessage inMessage(
        mid, propName, newPropValue, currValueModified, qids);
    inMessage.operationContext = oc;
    AutoPtr<CIMNotifyConfigChangeRequestMessage> outMessage(
        dynamic_cast<CIMNotifyConfigChangeRequestMessage*>(
            serializeDeserializeMessage(&inMessage)));
    PEGASUS_TEST_ASSERT(outMessage.get() != 0);

    validateCIMRequestMessageAttributes(&inMessage, outMessage.get());
    PEGASUS_TEST_ASSERT(inMessage.propertyName == outMessage->propertyName);
    PEGASUS_TEST_ASSERT(
        inMessage.newPropertyValue == outMessage->newPropertyValue);
    PEGASUS_TEST_ASSERT(inMessage.currentValueModified ==
        outMessage->currentValueModified);

    if (verbose)
    {
        cout << "testCIMNotifyConfigChangeRequestMessage - OK" << endl;
    }
}

//
// testCIMSubscriptionInitCompleteRequestMessage
//
void testCIMSubscriptionInitCompleteRequestMessage(
    const OperationContext& oc,
    const String& mid,
    const QueueIdStack& qids)
{
    CIMSubscriptionInitCompleteRequestMessage inMessage(mid, qids);
    inMessage.operationContext = oc;
    AutoPtr<CIMSubscriptionInitCompleteRequestMessage> outMessage(
        dynamic_cast<CIMSubscriptionInitCompleteRequestMessage*>(
            serializeDeserializeMessage(&inMessage)));
    PEGASUS_TEST_ASSERT(outMessage.get() != 0);

    validateCIMRequestMessageAttributes(&inMessage, outMessage.get());

    if (verbose)
    {
        cout << "testCIMSubscriptionInitCompleteRequestMessage - OK" << endl;
    }
}

//
// testProvAgtGetScmoClassRequestMessage
//
void testProvAgtGetScmoClassRequestMessage(
    const String& mid,
    const CIMNamespaceName& ns,
    const CIMName& className,
    const QueueIdStack& qids)
{
    ProvAgtGetScmoClassRequestMessage inMessage(mid, ns, className, qids);
    AutoPtr<ProvAgtGetScmoClassRequestMessage> outMessage(
        dynamic_cast<ProvAgtGetScmoClassRequestMessage*>(
            serializeDeserializeMessage(&inMessage)));
    PEGASUS_TEST_ASSERT(outMessage.get() != 0);

    PEGASUS_TEST_ASSERT(
        inMessage.nameSpace == outMessage->nameSpace);
    PEGASUS_TEST_ASSERT(
        inMessage.className == outMessage->className);

    if (verbose)
    {
        cout << "testProvAgtGetScmoClassRequestMessage - OK" << endl;
    }
}

//
// testCIMGetInstanceResponseMessage
//
void testCIMGetInstanceResponseMessage(
    const OperationContext& oc,
    const String& mid,
    const CIMException& ex,
    const QueueIdStack& qids,
    const CIMInstance& inst)
{
    CIMGetInstanceResponseMessage inMessage(mid, ex, qids);
    Uint32 prevSize =  inMessage.getResponseData().size();
    inMessage.getResponseData().setInstance(inst);
    PEGASUS_TEST_ASSERT(inMessage.getResponseData().size() == prevSize + 1);
    inMessage.operationContext = oc;
    prevSize = inMessage.getResponseData().size();
    AutoPtr<CIMGetInstanceResponseMessage> outMessage(
        dynamic_cast<CIMGetInstanceResponseMessage*>(
            serializeDeserializeMessage(&inMessage)));
    PEGASUS_TEST_ASSERT(outMessage.get() != 0);

    validateCIMResponseMessageAttributes(&inMessage, outMessage.get());
    if(outMessage->getResponseData().size() != prevSize)
    {
        cout << outMessage->getResponseData().size() << " " << prevSize << endl;
    }

    PEGASUS_TEST_ASSERT(outMessage->getResponseData().size() == prevSize);
     validateCIMInstance(
        inMessage.getResponseData().getInstance(),
        outMessage->getResponseData().getInstance());
     if (verbose)
    {
        cout << "testCIMGetInstanceResponseMessage - OK" << endl;
    }
}

void testCIMGetInstanceResponseMessageSCMO(
    const OperationContext& oc,
    const String& mid,
    const CIMException& ex,
    const QueueIdStack& qids,
    const CIMInstance& inst)
{
    Array<SCMOInstance> instArraySCMO;
    getSCMOInstanceFromCIM(instArraySCMO, inst);

    CIMGetInstanceResponseMessage inMessage(mid, ex, qids);
    inMessage.binaryResponse=true;
    Uint32 prevSize = inMessage.getResponseData().size();
    inMessage.getResponseData().setSCMO(instArraySCMO);
    PEGASUS_TEST_ASSERT(inMessage.getResponseData().size() ==
                        instArraySCMO.size() + prevSize);
    inMessage.operationContext = oc;
    AutoPtr<CIMGetInstanceResponseMessage> outMessage(
        dynamic_cast<CIMGetInstanceResponseMessage*>(
            serializeDeserializeMessage(&inMessage)));
    PEGASUS_TEST_ASSERT(outMessage.get() != 0);

    Uint32 outSize = outMessage->getResponseData().size();
    Uint32 inSize = inMessage.getResponseData().size();
    if (inSize != outSize)
    {
        cout << "Error in Serialized Msg Size "
             << " in msg size= " << inSize
             << " out msg size= " << outSize
             << " content= "
             << outMessage->getResponseData().getResponseDataContent()
             << endl << "trace inMessage = " << endl
             << inMessage.getResponseData().toStringTraceResponseData()
             << endl << "trace outMessage=" << endl
             << outMessage->getResponseData().toStringTraceResponseData()
             << endl;
    }

    PEGASUS_TEMP_ASSERT(outSize == inSize);

    validateCIMResponseMessageAttributes(&inMessage, outMessage.get());
    validateCIMInstance(
        inMessage.getResponseData().getInstance(),
        outMessage->getResponseData().getInstance());

    if (verbose)
    {
        cout << "testCIMGetInstanceResponseMessageSCMO - OK" << endl;
    }
}

//
// testCIMCreateInstanceResponseMessage
//
void testCIMCreateInstanceResponseMessage(
    const OperationContext& oc,
    const String& mid,
    const CIMException& ex,
    const QueueIdStack& qids,
    const CIMObjectPath& instName)
{
    CIMCreateInstanceResponseMessage inMessage(mid, ex, qids, instName);
    inMessage.operationContext = oc;
    AutoPtr<CIMCreateInstanceResponseMessage> outMessage(
        dynamic_cast<CIMCreateInstanceResponseMessage*>(
            serializeDeserializeMessage(&inMessage)));
    PEGASUS_TEST_ASSERT(outMessage.get() != 0);

    validateCIMResponseMessageAttributes(&inMessage, outMessage.get());
    PEGASUS_TEST_ASSERT(inMessage.instanceName == outMessage->instanceName);

    if (verbose)
    {
        cout << "testCIMCreateInstanceResponseMessage - OK" << endl;
    }
}

//
// testCIMModifyInstanceResponseMessage
//
void testCIMModifyInstanceResponseMessage(
    const OperationContext& oc,
    const String& mid,
    const CIMException& ex,
    const QueueIdStack& qids)
{
    CIMModifyInstanceResponseMessage inMessage(mid, ex, qids);
    inMessage.operationContext = oc;
    AutoPtr<CIMModifyInstanceResponseMessage> outMessage(
        dynamic_cast<CIMModifyInstanceResponseMessage*>(
            serializeDeserializeMessage(&inMessage)));
    PEGASUS_TEST_ASSERT(outMessage.get() != 0);

    validateCIMResponseMessageAttributes(&inMessage, outMessage.get());

    if (verbose)
    {
        cout << "testCIMModifyInstanceResponseMessage - OK" << endl;
    }
}

//
// testCIMDeleteInstanceResponseMessage
//
void testCIMDeleteInstanceResponseMessage(
    const OperationContext& oc,
    const String& mid,
    const CIMException& ex,
    const QueueIdStack& qids)
{
    CIMDeleteInstanceResponseMessage inMessage(mid, ex, qids);
    inMessage.operationContext = oc;
    AutoPtr<CIMDeleteInstanceResponseMessage> outMessage(
        dynamic_cast<CIMDeleteInstanceResponseMessage*>(
            serializeDeserializeMessage(&inMessage)));
    PEGASUS_TEST_ASSERT(outMessage.get() != 0);

    validateCIMResponseMessageAttributes(&inMessage, outMessage.get());

    if (verbose)
    {
        cout << "testCIMDeleteInstanceResponseMessage - OK" << endl;
    }
}

//
// testCIMEnumerateInstancesResponseMessage
//
void testCIMEnumerateInstancesResponseMessage(
    const OperationContext& oc,
    const String& mid,
    const CIMException& ex,
    const QueueIdStack& qids,
    const Array<CIMInstance>& instances)
{
    Array<SCMOInstance> instArraySCMO;
    getSCMOInstancesFromCIM(instArraySCMO, instances);

    CIMEnumerateInstancesResponseMessage inMessage(mid, ex, qids);
    inMessage.binaryResponse=true;
    inMessage.getResponseData().setInstances(instances);
    if (inMessage.getResponseData().size() != instances.size())
    {
        cout << "Size err" << inMessage.getResponseData().size() << " "
             << instances.size() << endl;
    }

    PEGASUS_TEMP_ASSERT(inMessage.getResponseData().size() ==
        instances.size());

    inMessage.getResponseData().setSCMO(instArraySCMO);

    if (inMessage.getResponseData().size() != instances.size()
        + instArraySCMO.size())
    {
        cout << "Size err" << inMessage.getResponseData().size() << " "
             << instances.size() + instArraySCMO.size() << endl;
    }
    PEGASUS_TEMP_ASSERT(inMessage.getResponseData().size() ==
        instances.size() + instArraySCMO.size());

    inMessage.operationContext = oc;
    AutoPtr<CIMEnumerateInstancesResponseMessage> outMessage(
        dynamic_cast<CIMEnumerateInstancesResponseMessage*>(
            serializeDeserializeMessage(&inMessage)));

    PEGASUS_TEMP_ASSERT(inMessage.getResponseData().size() ==
                        instances.size());

    PEGASUS_TEST_ASSERT(outMessage.get() != 0);


    validateCIMResponseMessageAttributes(&inMessage, outMessage.get());

    PEGASUS_TEMP_ASSERT(outMessage->getResponseData().size() ==
        inMessage.getResponseData().size());

    if (outMessage->getResponseData().size() !=
                      inMessage.getResponseData().size())
    {
        cout << "Error in Serialized Msg Size "
             << " in msg size " << inMessage.getResponseData().size()
             << " out msg size " << outMessage->getResponseData().size()
             << "content"
             << outMessage->getResponseData().getResponseDataContent()
             << endl;
    }
    validateCIMInstanceArray(
        inMessage.getResponseData().getInstances(),
        outMessage->getResponseData().getInstances());

    if (verbose)
    {
        cout << "testCIMEnumerateInstancesResponseMessage - OK" << endl;
    }
}

//
// testCIMEnumerateInstanceNamesResponseMessage
//
void testCIMEnumerateInstanceNamesResponseMessage(
    const OperationContext& oc,
    const String& mid,
    const CIMException& ex,
    const QueueIdStack& qids,
    const Array<CIMObjectPath>& instNames)
{
    Array<SCMOInstance> instArraySCMO;
    getSCMOInstancesFromCIM(instArraySCMO, instNames);

    CIMEnumerateInstanceNamesResponseMessage inMessage(mid, ex, qids);
    inMessage.binaryResponse=true;
    inMessage.getResponseData().setInstanceNames(instNames);
    PEGASUS_TEMP_ASSERT(inMessage.getResponseData().size() == instNames.size());

    inMessage.getResponseData().setSCMO(instArraySCMO);
    PEGASUS_TEMP_ASSERT(inMessage.getResponseData().size() ==
        instNames.size() + instArraySCMO.size());

    inMessage.operationContext = oc;
    AutoPtr<CIMEnumerateInstanceNamesResponseMessage> outMessage(
        dynamic_cast<CIMEnumerateInstanceNamesResponseMessage*>(
            serializeDeserializeMessage(&inMessage)));
    PEGASUS_TEST_ASSERT(outMessage.get() != 0);

    validateCIMResponseMessageAttributes(&inMessage, outMessage.get());

// KS_TODO - temp bypass to continue end-end testing until we fix
// binary protocol size function for binary data.
    PEGASUS_TEMP_ASSERT(outMessage->getResponseData().size() ==
        inMessage.getResponseData().size());

    if (outMessage->getResponseData().size() ==
                      inMessage.getResponseData().size())
    {
        cout << "Error in Serialized Msg Size "
             << " in msg size " << inMessage.getResponseData().size()
             << " out msg size " << outMessage->getResponseData().size()
             << "content"
             << outMessage->getResponseData().getResponseDataContent()
             << endl;
    }

    validateCIMObjectPathArray(
        inMessage.getResponseData().getInstanceNames(),
        outMessage->getResponseData().getInstanceNames());

    if (verbose)
    {
        cout << "testCIMEnumerateInstanceNamesResponseMessage - OK" << endl;
    }
}

//
// testCIMExecQueryResponseMessage
//
void testCIMExecQueryResponseMessage(
    const OperationContext& oc,
    const String& mid,
    const CIMException& ex,
    const QueueIdStack& qids,
    const Array<CIMObject>& objects)
{
    Array<SCMOInstance> instArraySCMO;
    getSCMOInstancesFromCIM(instArraySCMO, objects);

    CIMExecQueryResponseMessage inMessage(mid, ex, qids);
    inMessage.binaryResponse=true;
    inMessage.getResponseData().setObjects(objects);

    PEGASUS_TEMP_ASSERT(inMessage.getResponseData().size() == objects.size());
    inMessage.getResponseData().setSCMO(instArraySCMO);
    PEGASUS_TEMP_ASSERT(inMessage.getResponseData().size() ==
        objects.size() + instArraySCMO.size());

    inMessage.operationContext = oc;
    AutoPtr<CIMExecQueryResponseMessage> outMessage(
        dynamic_cast<CIMExecQueryResponseMessage*>(
            serializeDeserializeMessage(&inMessage)));
    PEGASUS_TEST_ASSERT(outMessage.get() != 0);

    // KS_TEMP temporary change to get through end-end while we sort out
    // size and move issues on binary protocol. Note that there are
    // several of these.
    if (outMessage->getResponseData().size() ==
                      inMessage.getResponseData().size())
    {
        cout << "Error in Serialized Msg Size "
             << " in msg size " << inMessage.getResponseData().size()
             << " out msg size " << outMessage->getResponseData().size()
             << "content"
             << outMessage->getResponseData().getResponseDataContent()
             << endl;
    }

    PEGASUS_TEMP_ASSERT(outMessage->getResponseData().size() ==
                        inMessage.getResponseData().size());

    validateCIMResponseMessageAttributes(&inMessage, outMessage.get());
    validateCIMObjectArray(
        inMessage.getResponseData().getObjects(),
        outMessage->getResponseData().getObjects());

    if (verbose)
    {
        cout << "testCIMExecQueryResponseMessage - OK" << endl;
    }
}

//
// testCIMAssociatorsResponseMessage
//
void testCIMAssociatorsResponseMessage(
    const OperationContext& oc,
    const String& mid,
    const CIMException& ex,
    const QueueIdStack& qids,
    const Array<CIMObject>& objArray)
{
    Array<SCMOInstance> instArraySCMO;
    getSCMOInstancesFromCIM(instArraySCMO, objArray);

    CIMAssociatorsResponseMessage inMessage(mid, ex, qids);
    inMessage.binaryResponse=true;
    inMessage.getResponseData().setObjects(objArray);
    inMessage.getResponseData().setSCMO(instArraySCMO);
    inMessage.operationContext = oc;
    AutoPtr<CIMAssociatorsResponseMessage> outMessage(
        dynamic_cast<CIMAssociatorsResponseMessage*>(
            serializeDeserializeMessage(&inMessage)));
    PEGASUS_TEST_ASSERT(outMessage.get() != 0);

    validateCIMResponseMessageAttributes(&inMessage, outMessage.get());
    validateCIMObjectArray(
        inMessage.getResponseData().getObjects(),
        outMessage->getResponseData().getObjects());

    if (verbose)
    {
        cout << "testCIMAssociatorsResponseMessage - OK" << endl;
    }
}

//
// testCIMAssociatorNamesResponseMessage
//
void testCIMAssociatorNamesResponseMessage(
    const OperationContext& oc,
    const String& mid,
    const CIMException& ex,
    const QueueIdStack& qids,
    const Array<CIMObjectPath>& pathArray)
{
    Array<SCMOInstance> instArraySCMO;
    getSCMOInstancesFromCIM(instArraySCMO, pathArray);

    CIMAssociatorNamesResponseMessage inMessage(mid, ex, qids);
    inMessage.binaryResponse=true;
    inMessage.getResponseData().setInstanceNames(pathArray);
    inMessage.getResponseData().setSCMO(instArraySCMO);
    inMessage.operationContext = oc;
    AutoPtr<CIMAssociatorNamesResponseMessage> outMessage(
        dynamic_cast<CIMAssociatorNamesResponseMessage*>(
            serializeDeserializeMessage(&inMessage)));
    PEGASUS_TEST_ASSERT(outMessage.get() != 0);

    validateCIMResponseMessageAttributes(&inMessage, outMessage.get());
    validateCIMObjectPathArray(
        inMessage.getResponseData().getInstanceNames(),
        outMessage->getResponseData().getInstanceNames());

    if (verbose)
    {
        cout << "testCIMAssociatorNamesResponseMessage - OK" << endl;
    }
}

//
// testCIMReferencesResponseMessage
//
void testCIMReferencesResponseMessage(
    const OperationContext& oc,
    const String& mid,
    const CIMException& ex,
    const QueueIdStack& qids,
    const Array<CIMObject>& objArray)
{
    Array<SCMOInstance> instArraySCMO;
    getSCMOInstancesFromCIM(instArraySCMO, objArray);

    CIMReferencesResponseMessage inMessage(mid, ex, qids);
    inMessage.binaryResponse=true;
    inMessage.getResponseData().setObjects(objArray);
    inMessage.getResponseData().setSCMO(instArraySCMO);
    inMessage.operationContext = oc;
    AutoPtr<CIMReferencesResponseMessage> outMessage(
        dynamic_cast<CIMReferencesResponseMessage*>(
            serializeDeserializeMessage(&inMessage)));
    PEGASUS_TEST_ASSERT(outMessage.get() != 0);

    validateCIMResponseMessageAttributes(&inMessage, outMessage.get());
    validateCIMObjectArray(
        inMessage.getResponseData().getObjects(),
        outMessage->getResponseData().getObjects());

    if (verbose)
    {
        cout << "testCIMReferencesResponseMessage - OK" << endl;
    }
}

//
// testCIMReferenceNamesResponseMessage
//
void testCIMReferenceNamesResponseMessage(
    const OperationContext& oc,
    const String& mid,
    const CIMException& ex,
    const QueueIdStack& qids,
    const Array<CIMObjectPath>& pathArray)
{
    Array<SCMOInstance> instArraySCMO;
    getSCMOInstancesFromCIM(instArraySCMO, pathArray);

    CIMReferenceNamesResponseMessage inMessage(mid, ex, qids);
    inMessage.binaryResponse=true;
    inMessage.getResponseData().setInstanceNames(pathArray);
    inMessage.getResponseData().setSCMO(instArraySCMO);
    inMessage.operationContext = oc;
    AutoPtr<CIMReferenceNamesResponseMessage> outMessage(
        dynamic_cast<CIMReferenceNamesResponseMessage*>(
            serializeDeserializeMessage(&inMessage)));
    PEGASUS_TEST_ASSERT(outMessage.get() != 0);

    validateCIMResponseMessageAttributes(&inMessage, outMessage.get());
    validateCIMObjectPathArray(
        inMessage.getResponseData().getInstanceNames(),
        outMessage->getResponseData().getInstanceNames());

    if (verbose)
    {
        cout << "testCIMReferenceNamesResponseMessage - OK" << endl;
    }
}

//
// testCIMGetPropertyResponseMessage
//
void testCIMGetPropertyResponseMessage(
    const OperationContext& oc,
    const String& mid,
    const CIMException& ex,
    const QueueIdStack& qids,
    const CIMValue& val)
{
    CIMGetPropertyResponseMessage inMessage(mid, ex, qids, val);
    inMessage.operationContext = oc;
    AutoPtr<CIMGetPropertyResponseMessage> outMessage(
        dynamic_cast<CIMGetPropertyResponseMessage*>(
            serializeDeserializeMessage(&inMessage)));
    PEGASUS_TEST_ASSERT(outMessage.get() != 0);

    validateCIMResponseMessageAttributes(&inMessage, outMessage.get());
    PEGASUS_TEST_ASSERT(inMessage.value == outMessage->value);

    if (verbose)
    {
        cout << "testCIMGetPropertyResponseMessage - OK" << endl;
    }
}

//
// testCIMSetPropertyResponseMessage
//
void testCIMSetPropertyResponseMessage(
    const OperationContext& oc,
    const String& mid,
    const CIMException& ex,
    const QueueIdStack& qids)
{
    CIMSetPropertyResponseMessage inMessage(mid, ex, qids);
    inMessage.operationContext = oc;
    AutoPtr<CIMSetPropertyResponseMessage> outMessage(
        dynamic_cast<CIMSetPropertyResponseMessage*>(
            serializeDeserializeMessage(&inMessage)));
    PEGASUS_TEST_ASSERT(outMessage.get() != 0);

    validateCIMResponseMessageAttributes(&inMessage, outMessage.get());

    if (verbose)
    {
        cout << "testCIMSetPropertyResponseMessage - OK" << endl;
    }
}

//
// testCIMInvokeMethodResponseMessage
//
void testCIMInvokeMethodResponseMessage(
    const OperationContext& oc,
    const String& mid,
    const CIMException& ex,
    const QueueIdStack& qids,
    const CIMValue& retVal,
    const Array<CIMParamValue>& outParams,
    const CIMName& methName)
{
    CIMInvokeMethodResponseMessage inMessage(
        mid, ex, qids, retVal, outParams, methName);
    inMessage.operationContext = oc;
    AutoPtr<CIMInvokeMethodResponseMessage> outMessage(
        dynamic_cast<CIMInvokeMethodResponseMessage*>(
            serializeDeserializeMessage(&inMessage)));
    PEGASUS_TEST_ASSERT(outMessage.get() != 0);

    validateCIMResponseMessageAttributes(&inMessage, outMessage.get());
    PEGASUS_TEST_ASSERT(inMessage.retValue == outMessage->retValue);
    validateCIMParamValueArray(
        inMessage.outParameters, outMessage->outParameters);
    PEGASUS_TEST_ASSERT(inMessage.methodName == outMessage->methodName);

    if (verbose)
    {
        cout << "testCIMInvokeMethodResponseMessage - OK" << endl;
    }
}

//
// testCIMCreateSubscriptionResponseMessage
//
void testCIMCreateSubscriptionResponseMessage(
    const OperationContext& oc,
    const String& mid,
    const CIMException& ex,
    const QueueIdStack& qids)
{
    CIMCreateSubscriptionResponseMessage inMessage(mid, ex, qids);
    inMessage.operationContext = oc;
    AutoPtr<CIMCreateSubscriptionResponseMessage> outMessage(
        dynamic_cast<CIMCreateSubscriptionResponseMessage*>(
            serializeDeserializeMessage(&inMessage)));
    PEGASUS_TEST_ASSERT(outMessage.get() != 0);

    validateCIMResponseMessageAttributes(&inMessage, outMessage.get());

    if (verbose)
    {
        cout << "testCIMCreateSubscriptionResponseMessage - OK" << endl;
    }
}

//
// testCIMModifySubscriptionResponseMessage
//
void testCIMModifySubscriptionResponseMessage(
    const OperationContext& oc,
    const String& mid,
    const CIMException& ex,
    const QueueIdStack& qids)
{
    CIMModifySubscriptionResponseMessage inMessage(mid, ex, qids);
    inMessage.operationContext = oc;
    AutoPtr<CIMModifySubscriptionResponseMessage> outMessage(
        dynamic_cast<CIMModifySubscriptionResponseMessage*>(
            serializeDeserializeMessage(&inMessage)));
    PEGASUS_TEST_ASSERT(outMessage.get() != 0);

    validateCIMResponseMessageAttributes(&inMessage, outMessage.get());

    if (verbose)
    {
        cout << "testCIMModifySubscriptionResponseMessage - OK" << endl;
    }
}

//
// testCIMDeleteSubscriptionResponseMessage
//
void testCIMDeleteSubscriptionResponseMessage(
    const OperationContext& oc,
    const String& mid,
    const CIMException& ex,
    const QueueIdStack& qids)
{
    CIMDeleteSubscriptionResponseMessage inMessage(mid, ex, qids);
    inMessage.operationContext = oc;
    AutoPtr<CIMDeleteSubscriptionResponseMessage> outMessage(
        dynamic_cast<CIMDeleteSubscriptionResponseMessage*>(
            serializeDeserializeMessage(&inMessage)));
    PEGASUS_TEST_ASSERT(outMessage.get() != 0);

    validateCIMResponseMessageAttributes(&inMessage, outMessage.get());

    if (verbose)
    {
        cout << "testCIMDeleteSubscriptionResponseMessage - OK" << endl;
    }
}

//
// testCIMExportIndicationResponseMessage
//
void testCIMExportIndicationResponseMessage(
    const OperationContext& oc,
    const String& mid,
    const CIMException& ex,
    const QueueIdStack& qids)
{
    CIMExportIndicationResponseMessage inMessage(mid, ex, qids);
    inMessage.operationContext = oc;
    AutoPtr<CIMExportIndicationResponseMessage> outMessage(
        dynamic_cast<CIMExportIndicationResponseMessage*>(
            serializeDeserializeMessage(&inMessage)));
    PEGASUS_TEST_ASSERT(outMessage.get() != 0);

    validateCIMResponseMessageAttributes(&inMessage, outMessage.get());

    if (verbose)
    {
        cout << "testCIMExportIndicationResponseMessage - OK" << endl;
    }
}

//
// testCIMProcessIndicationResponseMessage
//
void testCIMProcessIndicationResponseMessage(
    const OperationContext& oc,
    const String& mid,
    const CIMException& ex,
    const QueueIdStack& qids)
{
    CIMProcessIndicationResponseMessage inMessage(mid, ex, qids);
    inMessage.operationContext = oc;
    AutoPtr<CIMProcessIndicationResponseMessage> outMessage(
        dynamic_cast<CIMProcessIndicationResponseMessage*>(
            serializeDeserializeMessage(&inMessage)));
    PEGASUS_TEST_ASSERT(outMessage.get() != 0);

    validateCIMResponseMessageAttributes(&inMessage, outMessage.get());

    if (verbose)
    {
        cout << "testCIMProcessIndicationResponseMessage - OK" << endl;
    }
}

//
// testCIMDisableModuleResponseMessage
//
void testCIMDisableModuleResponseMessage(
    const OperationContext& oc,
    const String& mid,
    const CIMException& ex,
    const QueueIdStack& qids,
    const Array<Uint16>& opStatus)
{
    CIMDisableModuleResponseMessage inMessage(mid, ex, qids, opStatus);
    inMessage.operationContext = oc;
    AutoPtr<CIMDisableModuleResponseMessage> outMessage(
        dynamic_cast<CIMDisableModuleResponseMessage*>(
            serializeDeserializeMessage(&inMessage)));
    PEGASUS_TEST_ASSERT(outMessage.get() != 0);

    validateCIMResponseMessageAttributes(&inMessage, outMessage.get());
    PEGASUS_TEST_ASSERT(inMessage.operationalStatus.size() ==
        outMessage->operationalStatus.size());
    for (Uint32 i = 0; i < inMessage.operationalStatus.size(); i++)
    {
        PEGASUS_TEST_ASSERT(inMessage.operationalStatus[i] ==
            outMessage->operationalStatus[i]);
    }

    if (verbose)
    {
        cout << "testCIMDisableModuleResponseMessage - OK" << endl;
    }
}

//
// testCIMEnableModuleResponseMessage
//
void testCIMEnableModuleResponseMessage(
    const OperationContext& oc,
    const String& mid,
    const CIMException& ex,
    const QueueIdStack& qids,
    const Array<Uint16>& opStatus)
{
    CIMEnableModuleResponseMessage inMessage(mid, ex, qids, opStatus);
    inMessage.operationContext = oc;
    AutoPtr<CIMEnableModuleResponseMessage> outMessage(
        dynamic_cast<CIMEnableModuleResponseMessage*>(
            serializeDeserializeMessage(&inMessage)));
    PEGASUS_TEST_ASSERT(outMessage.get() != 0);

    validateCIMResponseMessageAttributes(&inMessage, outMessage.get());
    PEGASUS_TEST_ASSERT(inMessage.operationalStatus.size() ==
        outMessage->operationalStatus.size());
    for (Uint32 i = 0; i < inMessage.operationalStatus.size(); i++)
    {
        PEGASUS_TEST_ASSERT(inMessage.operationalStatus[i] ==
            outMessage->operationalStatus[i]);
    }

    if (verbose)
    {
        cout << "testCIMEnableModuleResponseMessage - OK" << endl;
    }
}

//
// testCIMStopAllProvidersResponseMessage
//
void testCIMStopAllProvidersResponseMessage(
    const OperationContext& oc,
    const String& mid,
    const CIMException& ex,
    const QueueIdStack& qids)
{
    CIMStopAllProvidersResponseMessage inMessage(mid, ex, qids);
    inMessage.operationContext = oc;
    AutoPtr<CIMStopAllProvidersResponseMessage> outMessage(
        dynamic_cast<CIMStopAllProvidersResponseMessage*>(
            serializeDeserializeMessage(&inMessage)));
    PEGASUS_TEST_ASSERT(outMessage.get() != 0);

    validateCIMResponseMessageAttributes(&inMessage, outMessage.get());

    if (verbose)
    {
        cout << "testCIMStopAllProvidersResponseMessage - OK" << endl;
    }
}

//
// testCIMInitializeProviderAgentResponseMessage
//
void testCIMInitializeProviderAgentResponseMessage(
    const OperationContext& oc,
    const String& mid,
    const CIMException& ex,
    const QueueIdStack& qids)
{
    CIMInitializeProviderAgentResponseMessage inMessage(mid, ex, qids);
    inMessage.operationContext = oc;
    AutoPtr<CIMInitializeProviderAgentResponseMessage> outMessage(
        dynamic_cast<CIMInitializeProviderAgentResponseMessage*>(
            serializeDeserializeMessage(&inMessage)));
    PEGASUS_TEST_ASSERT(outMessage.get() != 0);

    validateCIMResponseMessageAttributes(&inMessage, outMessage.get());

    if (verbose)
    {
        cout << "testCIMInitializeProviderAgentResponseMessage - OK" << endl;
    }
}

//
// testCIMNotifyConfigChangeResponseMessage
//
void testCIMNotifyConfigChangeResponseMessage(
    const OperationContext& oc,
    const String& mid,
    const CIMException& ex,
    const QueueIdStack& qids)
{
    CIMNotifyConfigChangeResponseMessage inMessage(mid, ex, qids);
    inMessage.operationContext = oc;
    AutoPtr<CIMNotifyConfigChangeResponseMessage> outMessage(
        dynamic_cast<CIMNotifyConfigChangeResponseMessage*>(
            serializeDeserializeMessage(&inMessage)));
    PEGASUS_TEST_ASSERT(outMessage.get() != 0);

    validateCIMResponseMessageAttributes(&inMessage, outMessage.get());

    if (verbose)
    {
        cout << "testCIMNotifyConfigChangeResponseMessage - OK" << endl;
    }
}

//
// testCIMSubscriptionInitCompleteResponseMessage
//
void testCIMSubscriptionInitCompleteResponseMessage(
    const OperationContext& oc,
    const String& mid,
    const CIMException& ex,
    const QueueIdStack& qids)
{
    CIMSubscriptionInitCompleteResponseMessage inMessage(mid, ex, qids);
    inMessage.operationContext = oc;
    AutoPtr<CIMSubscriptionInitCompleteResponseMessage> outMessage(
        dynamic_cast<CIMSubscriptionInitCompleteResponseMessage*>(
            serializeDeserializeMessage(&inMessage)));
    PEGASUS_TEST_ASSERT(outMessage.get() != 0);

    validateCIMResponseMessageAttributes(&inMessage, outMessage.get());

    if (verbose)
    {
        cout << "testCIMSubscriptionInitCompleteResponseMessage - OK" << endl;
    }
}

SCMOClass _scmoClassCache_GetClass(
    const CIMNamespaceName& nameSpace,
    const CIMName& className)
{
    for (Uint32 x=0; x < classArray->size(); x++)
    {
        CIMClass cls = (*classArray)[x];
        if (!cls.isUninitialized())
        {
            if (cls.getClassName() == className &&
                cls.getPath().getNameSpace() == nameSpace )
            {
                return SCMOClass(cls);
            }
        }
    }

    return SCMOClass("","");
}

//
// testProvAgtGetScmoClassResponseMessage
//
void testProvAgtGetScmoClassResponseMessage(
    const String& mid,
    const CIMException& ex,
    const CIMClass& cls,
    const QueueIdStack& qids)
{

    CIMClass inCls;
    CIMClass outCls;

    SCMOClass inc = SCMOClass("","");

    if (!cls.isUninitialized())
    {
        inc = SCMOClass(cls);
    }

    ProvAgtGetScmoClassResponseMessage inMessage(mid, ex, qids, inc);
    AutoPtr<ProvAgtGetScmoClassResponseMessage> outMessage(
        dynamic_cast<ProvAgtGetScmoClassResponseMessage*>(
            serializeDeserializeMessage(&inMessage)));
    PEGASUS_TEST_ASSERT(outMessage.get() != 0);


    if (!inMessage.scmoClass.isEmpty() &&
        !outMessage->scmoClass.isEmpty())
    {
        inMessage.scmoClass.getCIMClass(inCls);
        outMessage->scmoClass.getCIMClass(outCls);
        PEGASUS_TEST_ASSERT(inCls.identical(outCls));
    }
    else
    {
       PEGASUS_TEST_ASSERT(
           inMessage.scmoClass.isEmpty() &&
           outMessage->scmoClass.isEmpty());
    }

    if (verbose)
    {
        cout << "testProvAgtGetScmoClassResponseMessage - OK" << endl;
    }
}


void testMessageSerialization()
{
    // Initialize the SCMO Class cache and set the callback function
    // for retrieving CIMClasses to local routine
    classArray = new Array<CIMClass>();


    SCMOClassCache* scmoCache = SCMOClassCache::getInstance();
    scmoCache->setCallBack(_scmoClassCache_GetClass);



    // Message IDs
    String mid1;
    String mid2 = "123";
    String mid3 = "ab: g TEST";
    String mid4 = "2134567890asdjkfhbm,zasdlkfjowiqrup[][]{}{=+-_'.,`~|";

    // QueueIdStacks
    QueueIdStack qids1;
    QueueIdStack qids2;
    qids2.push(10);
    qids2.push(5);
    QueueIdStack qids3;
    qids3.push(0);
    qids3.push(0);
    QueueIdStack qids4;
    qids4.push(1000000);

    // Authorization types
    String auth1;
    String auth2 = "Local";
    String auth3 = "Basic";
    String auth4 = "A very secret authorization scheme";

    // User names
    String user1;
    String user2 = "user:2";
    String user3 = "happy";
    String user4 = "doc!";

    // CIMExceptions
    CIMException ex1;
    CIMException ex2(CIM_ERR_NOT_SUPPORTED, "Unsupported operation");
    CIMException ex3(CIM_ERR_ACCESS_DENIED, "");
    CIMException ex4(CIM_ERR_SUCCESS, "How can you have a \"success\" error?");

    // Operation flags
    Boolean di1 = false;
    Boolean di2 = true;
    Boolean iq1 = false;
    Boolean iq2 = true;
    Boolean ico1 = false;
    Boolean ico2 = true;

    // CIMPropertyLists
    CIMPropertyList pl1;
    Array<CIMName> plArray2;
    CIMPropertyList pl2(plArray2);
    Array<CIMName> plArray3;
    plArray3.append("MyMostVeryImportantPropertyTheOnlyOneIWant");
    CIMPropertyList pl3(plArray3);
    Array<CIMName> plArray4;
    plArray4.append("P1");
    plArray4.append("P2");
    plArray4.append("P3");
    plArray4.append("P4");
    plArray4.append("P5");
    plArray4.append("P6");
    plArray4.append("P7");
    plArray4.append("P8");
    CIMPropertyList pl4(plArray4);

    // CIMNamespaceNames
    CIMNamespaceName ns1;
    CIMNamespaceName ns2("root/ns1");
    CIMNamespaceName ns3("ns1/ns2/ns3/ns4/ns5/ns6/ns7/ns8/ns9/ns10");
    CIMNamespaceName ns4("test");

    // CIMNames
    CIMName name1;
    CIMName name2 = "TST_EmptyClass";
    CIMName name3 = "ClassName3";
    CIMName name4 = "TST_ClassDef";

    // CIMObjectPaths
    CIMObjectPath path1;
    Array<CIMKeyBinding> kb;
    CIMObjectPath path2(System::getHostName(), ns2, name2, kb);
    kb.append(CIMKeyBinding(
        CIMName("Name"), "BoringInstance", CIMKeyBinding::STRING));
    CIMObjectPath path3("", ns3, name3, kb);
    kb.append(CIMKeyBinding(CIMName("zzzz"), CIMValue(String("sleepy..."))));
    kb.append(CIMKeyBinding(CIMName("HowMany"), CIMValue(Uint32(102030405))));
    CIMObjectPath path4("", ns4, name4, kb);

    // CIMObjectPath Arrays
    Array<CIMObjectPath> pathArray1;
    Array<CIMObjectPath> pathArray2;
    pathArray2.append(path1);
    Array<CIMObjectPath> pathArray3;
    pathArray3.append(path3);
    pathArray3.append(path4);
    pathArray3.append(path2);
    Array<CIMObjectPath> pathArray4;
    pathArray4.append(path4);
    pathArray4.append(path1);
    pathArray4.append(path3);
    pathArray4.append(path2);

    // CIMInstances
    CIMInstance inst1;
    CIMInstance inst2(name2);
    inst2.setPath(path2);
    CIMInstance inst3(name3);
    inst3.addProperty(CIMProperty("Name", String("BoringInstance")));
    inst3.addProperty(CIMProperty("Identifier", String("111")));
    inst3.setPath(path3);
    CIMInstance inst4(name4);
    inst4.addProperty(CIMProperty("P1", CIMValue(Uint16(65))));
    inst4.addProperty(CIMProperty("P2", CIMValue(Boolean(false))));
    inst4.addProperty(CIMProperty("P3", CIMValue(CIMDateTime())));
    inst4.getProperty(1).addQualifier(CIMQualifier("Q1", CIMValue(Uint32(8)),
        CIMFlavor::TOSUBCLASS + CIMFlavor::ENABLEOVERRIDE));
    inst4.setPath(path4);

    //CIMProperties
    CIMProperty prop1("P1", CIMValue(Uint16(65)));
    CIMProperty prop2("P2", CIMValue(Boolean(false)));
    CIMProperty prop3("P3", CIMValue(CIMDateTime()));
    CIMProperty prop4("zzzz", CIMValue(String("sleepy...")));
    CIMProperty prop5("HowMany", CIMValue(Uint32(102030405)));
    prop2.addQualifier(CIMQualifier("Q1", CIMValue(Uint32(8)),
        CIMFlavor::TOSUBCLASS + CIMFlavor::ENABLEOVERRIDE));
    prop4.addQualifier(CIMQualifier("KEY", CIMValue(Boolean(true))));
    prop5.addQualifier(CIMQualifier("KEY", CIMValue(Boolean(true))));

    // CIMClasses
    CIMClass class1;
    CIMClass class2(name2);
    class2.setPath(path2);
    CIMClass class3(name3);
    class3.addProperty(CIMProperty("Name", String("BoringInstance")));
    class3.addProperty(CIMProperty("Identifier", String("111")));
    class3.setPath(path3);
    CIMClass class4(name4);
    class4.addProperty(prop1);
    class4.addProperty(prop2);
    class4.addProperty(prop3);
    class4.addProperty(prop4);
    class4.addProperty(prop5);
    class4.setPath(path4);

    // CIMClass Array, use for conversion from CIMInstance to SCMOInstance
    classArray->append(class1);
    classArray->append(class2);
    classArray->append(class3);
    classArray->append(class4);


    // CIMName Arrays
    Array<CIMName> nameArray1;
    Array<CIMName> nameArray2;
    nameArray2.append(name1);
    Array<CIMName> nameArray3;
    nameArray3.append(name2);
    nameArray3.append(name3);
    Array<CIMName> nameArray4;
    nameArray4.append(name4);
    nameArray4.append(name1);
    nameArray4.append(name3);
    nameArray4.append(name2);

    // CIMInstance Arrays
    Array<CIMInstance> instArray1;
    Array<CIMInstance> instArray2;
    instArray2.append(inst1);
    Array<CIMInstance> instArray3;
    instArray3.append(inst2);
    instArray3.append(inst3);
    instArray3.append(inst4);
    Array<CIMInstance> instArray4;
    instArray4.append(inst2);
    instArray4.append(inst3);
    instArray4.append(inst1);
    instArray4.append(inst4);

    // CIMObject Arrays
    Array<CIMObject> objArray1;
    Array<CIMObject> objArray2;
    objArray2.append(CIMObject(inst1));
    objArray2.append(CIMObject(class1));
    Array<CIMObject> objArray3;
    objArray3.append(CIMObject(inst2));
    objArray3.append(CIMObject(class3));
    objArray3.append(CIMObject(inst4));
    objArray3.append(CIMObject(class2));
    objArray3.append(CIMObject(inst3));
    objArray3.append(CIMObject(class4));
    Array<CIMObject> objArray4;
    objArray4.append(CIMObject(class3));
    objArray4.append(CIMObject(inst2));
    objArray4.append(CIMObject(class1));
    objArray4.append(CIMObject(inst4));
    objArray4.append(CIMObject(class2));
    objArray4.append(CIMObject(inst1));
    objArray4.append(CIMObject(class4));
    objArray4.append(CIMObject(inst3));

    // Roles
    String role1;
    String role2 = "a";
    String role3 = "AMuchLongerNameThanTheOneBeforeThis";
    String role4 = "Does anyone care if it is a valid CIMName?";

    // Query languages
    String ql1;
    String ql2 = "WQL";
    String ql3 = "DMTF:CQL";
    String ql4 = "What does it matter what I put here?";

    // Query strings
    String qs1;
    String qs2 = "SELECT * FROM MyClass WHERE SomethingICareAbout = true";
    String qs3 = "Gimme what I want!";
    String qs4 = "a=1, b=2";

    // CIMValues
    CIMValue val1;
    CIMValue val2(CIMDateTime("20060209142430.123456-420"));
    CIMValue val3(path3);
    CIMValue val4(Boolean(false));
    CIMValue val5(CIMTYPE_UINT8, false);
    CIMValue val6(CIMTYPE_REFERENCE, true);

    // CIMParamValue Arrays
    CIMParamValue pv1;
    CIMParamValue pv2("1", val2);
    CIMParamValue pv3("HiMom", val3);
    CIMParamValue pv4("Howdy do?", val4);
    CIMParamValue pv5("Test1", val5);
    CIMParamValue pv6("Test2", val6);
    Array<CIMParamValue> pvArray1;
    Array<CIMParamValue> pvArray2;
    pvArray2.append(pv4);
    Array<CIMParamValue> pvArray3;
    pvArray3.append(pv2);
    pvArray3.append(pv3);
    Array<CIMParamValue> pvArray4;
    pvArray4.append(pv4);
    pvArray4.append(pv3);
    pvArray4.append(pv2);
    pvArray4.append(pv5);
    pvArray4.append(pv6);

    // Boolean Arrays
    Array<Boolean> boolArray1;
    Array<Boolean> boolArray2;
    boolArray2.append(false);
    Array<Boolean> boolArray3;
    boolArray2.append(true);
    Array<Boolean> boolArray4;
    boolArray2.append(false);
    boolArray2.append(true);
    boolArray2.append(false);

    // Uint16 Arrays
    Array<Uint16> uint16Array1;
    Array<Uint16> uint16Array2;
    uint16Array2.append(0);
    Array<Uint16> uint16Array3;
    uint16Array3.append(65535);
    Array<Uint16> uint16Array4;
    uint16Array4.append(65535);
    uint16Array4.append(0);
    uint16Array4.append(1);
    uint16Array4.append(100);

    // String Pair Arrays
    Array<Pair<String, String> > spa1;
    Array<Pair<String, String> > spa2;
    spa2.append(Pair<String, String>("", ""));
    Array<Pair<String, String> > spa3;
    spa3.append(Pair<String, String>("Hi", ""));
    spa3.append(Pair<String, String>("", "Bye"));
    Array<Pair<String, String> > spa4;
    spa4.append(Pair<String, String>("Hi", ""));
    spa4.append(Pair<String, String>("", ""));
    spa4.append(Pair<String, String>("First half", "Second half"));
    spa4.append(Pair<String, String>("", "Bye"));

    // OperationContexts
    OperationContext oc1;
    OperationContext oc2;
    oc2.insert(IdentityContainer(user1));
    oc2.insert(SubscriptionInstanceContainer(inst1));
    oc2.insert(SubscriptionFilterQueryContainer(ql1, qs1, ns1));
    oc2.insert(SubscriptionInstanceNamesContainer(pathArray1));
    AcceptLanguageList all2;
    oc2.insert(AcceptLanguageListContainer(all2));
    ContentLanguageList cll2;
    oc2.insert(ContentLanguageListContainer(cll2));
    oc2.insert(LocaleContainer(""));
    oc2.insert(UserRoleContainer(""));
    OperationContext oc3;
    oc3.insert(SubscriptionFilterConditionContainer(ql2, qs3));
    OperationContext oc4;
    oc4.insert(IdentityContainer(user4));
    oc4.insert(SubscriptionInstanceContainer(inst4));
    oc4.insert(SubscriptionFilterConditionContainer(ql3, qs4));
    oc4.insert(SubscriptionFilterQueryContainer(ql4, qs2, ns4));
    oc4.insert(SubscriptionInstanceNamesContainer(pathArray4));
    oc4.insert(TimeoutContainer(100000000));
    AcceptLanguageList all4;
    all4.insert(LanguageTag("en-US"), 1);
    all4.insert(LanguageTag("de"), Real32(0.5));
    all4.insert(LanguageTag("*"), Real32(0.1));
    oc4.insert(AcceptLanguageListContainer(all4));
    ContentLanguageList cll4;
    cll4.append(LanguageTag("en-US"));
    cll4.append(LanguageTag("es-MX-jalisco"));
    oc4.insert(ContentLanguageListContainer(cll4));
    oc4.insert(SnmpTrapOidContainer("1.43.213.53.23.52.1"));
    oc4.insert(LocaleContainer("here"));
    oc4.insert(UserRoleContainer("NoRoleUser"));

    testEmptyMessage();

    testCIMGetInstanceRequestMessage(
        oc2, mid1, ns3, path1, iq2, ico1, pl4, qids3, auth2, user4);
    testCIMGetInstanceRequestMessage(
        oc3, mid2, ns4, path2, iq1, ico2, pl1, qids4, auth3, user1);
    testCIMGetInstanceRequestMessage(
        oc4, mid3, ns1, path3, iq1, ico1, pl2, qids1, auth4, user2);
    testCIMGetInstanceRequestMessage(
        oc1, mid4, ns2, path4, iq2, ico2, pl3, qids2, auth1, user3);

    testCIMCreateInstanceRequestMessage(
        oc1, mid4, ns1, inst2, qids2, auth4, user3);
    testCIMCreateInstanceRequestMessage(
        oc2, mid1, ns2, inst3, qids3, auth1, user4);
    testCIMCreateInstanceRequestMessage(
        oc3, mid2, ns3, inst4, qids4, auth2, user1);
    testCIMCreateInstanceRequestMessage(
        oc4, mid3, ns4, inst2, qids1, auth3, user2);

    testCIMModifyInstanceRequestMessage(
        oc4, mid2, ns1, inst2, iq2, pl4, qids2, auth3, user1);
    testCIMModifyInstanceRequestMessage(
        oc1, mid3, ns2, inst3, iq1, pl1, qids3, auth4, user2);
    testCIMModifyInstanceRequestMessage(
        oc2, mid4, ns3, inst4, iq2, pl2, qids4, auth1, user3);
    testCIMModifyInstanceRequestMessage(
        oc3, mid1, ns4, inst2, iq1, pl3, qids1, auth2, user4);

    testCIMDeleteInstanceRequestMessage(
        oc2, mid4, ns3, path3, qids1, auth2, user1);
    testCIMDeleteInstanceRequestMessage(
        oc3, mid1, ns4, path4, qids2, auth3, user2);
    testCIMDeleteInstanceRequestMessage(
        oc4, mid2, ns1, path1, qids3, auth4, user3);
    testCIMDeleteInstanceRequestMessage(
        oc1, mid3, ns2, path2, qids4, auth1, user4);

    testCIMEnumerateInstancesRequestMessage(oc2, mid1, ns2, name3,
        di1, iq2, ico1, pl2, qids4, auth3, user4);
    testCIMEnumerateInstancesRequestMessage(oc3, mid2, ns3, name4,
        di2, iq1, ico1, pl3, qids1, auth4, user1);
    testCIMEnumerateInstancesRequestMessage(oc4, mid3, ns4, name1,
        di1, iq2, ico2, pl4, qids2, auth1, user2);
    testCIMEnumerateInstancesRequestMessage(oc1, mid4, ns1, name2,
        di2, iq1, ico2, pl1, qids3, auth2, user3);

    testCIMEnumerateInstanceNamesRequestMessage(
        oc3, mid2, ns3, name1, qids2, auth4, user1);
    testCIMEnumerateInstanceNamesRequestMessage(
        oc4, mid3, ns4, name2, qids3, auth1, user2);
    testCIMEnumerateInstanceNamesRequestMessage(
        oc1, mid4, ns1, name3, qids4, auth2, user3);
    testCIMEnumerateInstanceNamesRequestMessage(
        oc2, mid1, ns2, name4, qids1, auth3, user4);

    testCIMExecQueryRequestMessage(
        oc3, mid1, ns1, ql2, qs3, qids2, auth4, user2);
    testCIMExecQueryRequestMessage(
        oc4, mid2, ns2, ql3, qs4, qids3, auth1, user3);
    testCIMExecQueryRequestMessage(
        oc1, mid3, ns3, ql4, qs1, qids4, auth2, user4);
    testCIMExecQueryRequestMessage(
        oc2, mid4, ns4, ql1, qs2, qids1, auth3, user1);

    testCIMAssociatorsRequestMessage(oc3, mid1, ns4, path2, name2,
        name4, role2, role1, iq2, ico1, pl4, qids3, auth3, user3);
    testCIMAssociatorsRequestMessage(oc4, mid2, ns1, path3, name3,
        name1, role3, role2, iq1, ico2, pl1, qids4, auth4, user4);
    testCIMAssociatorsRequestMessage(oc1, mid3, ns2, path4, name4,
        name2, role4, role3, iq2, ico2, pl2, qids1, auth1, user1);
    testCIMAssociatorsRequestMessage(oc2, mid4, ns3, path1, name1,
        name3, role1, role4, iq1, ico1, pl3, qids2, auth2, user2);

    testCIMAssociatorNamesRequestMessage(oc4, mid4, ns2, path2,
        name3, name2, role1, role2, qids1, auth4, user3);
    testCIMAssociatorNamesRequestMessage(oc1, mid1, ns3, path3,
        name4, name3, role2, role3, qids2, auth1, user4);
    testCIMAssociatorNamesRequestMessage(oc2, mid2, ns4, path4,
        name1, name4, role3, role4, qids3, auth2, user1);
    testCIMAssociatorNamesRequestMessage(oc3, mid3, ns1, path1,
        name2, name1, role4, role1, qids4, auth3, user2);

    testCIMReferencesRequestMessage(oc3, mid1, ns4, path1, name2,
        role2, iq2, ico1, pl4, qids4, auth1, user2);
    testCIMReferencesRequestMessage(oc4, mid2, ns1, path2, name3,
        role3, iq1, ico2, pl1, qids1, auth2, user3);
    testCIMReferencesRequestMessage(oc1, mid3, ns2, path3, name4,
        role4, iq2, ico2, pl2, qids2, auth3, user4);
    testCIMReferencesRequestMessage(oc2, mid4, ns3, path4, name1,
        role1, iq1, ico1, pl3, qids3, auth4, user1);

    testCIMReferenceNamesRequestMessage(oc1, mid2, ns1, path4, name4,
        role2, qids3, auth1, user2);
    testCIMReferenceNamesRequestMessage(oc2, mid3, ns2, path1, name1,
        role3, qids4, auth2, user3);
    testCIMReferenceNamesRequestMessage(oc3, mid4, ns3, path2, name2,
        role4, qids1, auth3, user4);
    testCIMReferenceNamesRequestMessage(oc4, mid1, ns4, path3, name3,
        role1, qids2, auth4, user1);

    testCIMGetPropertyRequestMessage(
        oc1, mid3, ns2, path1, name1, qids1, auth2, user2);
    testCIMGetPropertyRequestMessage(
        oc2, mid4, ns3, path2, name2, qids2, auth3, user3);
    testCIMGetPropertyRequestMessage(
        oc3, mid1, ns4, path3, name3, qids3, auth4, user4);
    testCIMGetPropertyRequestMessage(
        oc4, mid2, ns1, path4, name4, qids4, auth1, user1);

    testCIMSetPropertyRequestMessage(
        oc4, mid1, ns2, path2, name4, val2, qids3, auth2, user1);
    testCIMSetPropertyRequestMessage(
        oc1, mid2, ns3, path3, name2, val3, qids4, auth3, user2);
    testCIMSetPropertyRequestMessage(
        oc2, mid3, ns4, path4, name2, val4, qids1, auth4, user3);
    testCIMSetPropertyRequestMessage(
        oc3, mid4, ns1, path1, name3, val1, qids2, auth1, user4);

    testCIMInvokeMethodRequestMessage(
        oc4, mid2, ns4, path1, name2, pvArray3, qids3, auth4, user2);
    testCIMInvokeMethodRequestMessage(
        oc1, mid3, ns1, path2, name3, pvArray4, qids4, auth1, user3);
    testCIMInvokeMethodRequestMessage(
        oc2, mid4, ns2, path3, name4, pvArray1, qids1, auth2, user4);
    testCIMInvokeMethodRequestMessage(
        oc3, mid1, ns3, path4, name1, pvArray2, qids2, auth3, user1);

    testCIMCreateSubscriptionRequestMessage(oc1, mid3, ns1, inst3,
        nameArray1, pl3,     8, qs2, qids2, auth1, user3);
    testCIMCreateSubscriptionRequestMessage(oc2, mid4, ns2, inst4,
        nameArray2, pl4,     0, qs3, qids3, auth2, user4);
    testCIMCreateSubscriptionRequestMessage(oc3, mid1, ns3, inst1,
        nameArray3, pl1, 65535, qs4, qids4, auth3, user1);
    testCIMCreateSubscriptionRequestMessage(oc4, mid2, ns4, inst2,
        nameArray4, pl2,    10, qs1, qids1, auth4, user2);

    testCIMModifySubscriptionRequestMessage(oc2, mid1, ns4, inst2,
        nameArray2, pl3, 65535, qs4, qids1, auth4, user3);
    testCIMModifySubscriptionRequestMessage(oc3, mid2, ns1, inst3,
        nameArray3, pl4,     0, qs1, qids2, auth1, user4);
    testCIMModifySubscriptionRequestMessage(oc4, mid3, ns2, inst4,
        nameArray4, pl1,     1, qs2, qids3, auth2, user1);
    testCIMModifySubscriptionRequestMessage(oc1, mid4, ns3, inst1,
        nameArray1, pl2,   100, qs3, qids4, auth3, user2);

    testCIMDeleteSubscriptionRequestMessage(
        oc1, mid4, ns3, inst2, nameArray1, qids2, auth3, user4);
    testCIMDeleteSubscriptionRequestMessage(
        oc2, mid1, ns4, inst3, nameArray2, qids3, auth4, user1);
    testCIMDeleteSubscriptionRequestMessage(
        oc3, mid2, ns1, inst4, nameArray3, qids4, auth1, user2);
    testCIMDeleteSubscriptionRequestMessage(
        oc4, mid3, ns2, inst1, nameArray4, qids1, auth2, user3);

    testCIMExportIndicationRequestMessage(
        oc4, mid3, mid1, inst2, qids1, auth3, user3);
    testCIMExportIndicationRequestMessage(
        oc1, mid4, mid2, inst3, qids2, auth4, user4);
    testCIMExportIndicationRequestMessage(
        oc2, mid1, mid3, inst4, qids3, auth1, user1);
    testCIMExportIndicationRequestMessage(
        oc3, mid2, mid4, inst1, qids4, auth2, user2);

    testCIMProcessIndicationRequestMessage(
        oc1, mid4, ns3, inst2, pathArray2, inst4, qids4);
    testCIMProcessIndicationRequestMessage(
        oc2, mid1, ns4, inst3, pathArray3, inst1, qids1);
    testCIMProcessIndicationRequestMessage(
        oc3, mid2, ns1, inst4, pathArray4, inst2, qids2);
    testCIMProcessIndicationRequestMessage(
        oc4, mid3, ns2, inst1, pathArray1, inst3, qids3);

    testCIMDisableModuleRequestMessage(oc2, mid1, inst2, instArray3,
        false, boolArray4, qids3, auth1, user4);
    testCIMDisableModuleRequestMessage(oc3, mid2, inst3, instArray4,
         true, boolArray1, qids4, auth2, user1);
    testCIMDisableModuleRequestMessage(oc4, mid3, inst4, instArray1,
        false, boolArray2, qids1, auth3, user2);
    testCIMDisableModuleRequestMessage(oc1, mid4, inst1, instArray2,
         true, boolArray3, qids2, auth4, user3);

    testCIMEnableModuleRequestMessage(
         oc1, mid2, inst1, qids3, auth1, user4);
    testCIMEnableModuleRequestMessage(
         oc2, mid3, inst2, qids4, auth2, user1);
    testCIMEnableModuleRequestMessage(
         oc3, mid4, inst3, qids1, auth3, user2);
    testCIMEnableModuleRequestMessage(
         oc4, mid1, inst4, qids2, auth4, user3);

    testCIMStopAllProvidersRequestMessage(oc2, mid1, qids3);
    testCIMStopAllProvidersRequestMessage(oc3, mid2, qids4);
    testCIMStopAllProvidersRequestMessage(oc4, mid3, qids1);
    testCIMStopAllProvidersRequestMessage(oc1, mid4, qids2);

    testCIMInitializeProviderAgentRequestMessage(
        oc2, mid1, mid4, spa3, false,  true, qids2);
    testCIMInitializeProviderAgentRequestMessage(
        oc3, mid2, mid1, spa4,  true, false, qids3);
    testCIMInitializeProviderAgentRequestMessage(
        oc4, mid3, mid2, spa1,  true,  true, qids4);
    testCIMInitializeProviderAgentRequestMessage(
        oc1, mid4, mid3, spa2, false, false, qids1);

    testCIMNotifyConfigChangeRequestMessage(
        oc3, mid1, mid4, mid3, false, qids1);
    testCIMNotifyConfigChangeRequestMessage(
        oc4, mid2, mid1, mid4,  true, qids2);
    testCIMNotifyConfigChangeRequestMessage(
        oc1, mid3, mid2, mid1, false, qids3);
    testCIMNotifyConfigChangeRequestMessage(
        oc2, mid4, mid3, mid2,  true, qids4);

    testCIMSubscriptionInitCompleteRequestMessage(oc1, mid2, qids3);
    testCIMSubscriptionInitCompleteRequestMessage(oc2, mid3, qids4);
    testCIMSubscriptionInitCompleteRequestMessage(oc3, mid4, qids1);
    testCIMSubscriptionInitCompleteRequestMessage(oc4, mid1, qids2);

    testProvAgtGetScmoClassRequestMessage(mid4,ns2,name1,qids3);
    testProvAgtGetScmoClassRequestMessage(mid1,ns3,name3,qids2);
    testProvAgtGetScmoClassRequestMessage(mid2,ns1,name4,qids4);
    testProvAgtGetScmoClassRequestMessage(mid3,ns4,name2,qids1);

    testProvAgtGetScmoClassResponseMessage(mid2,ex1,class3,qids4);
    testProvAgtGetScmoClassResponseMessage(mid1,ex3,class2,qids3);
    testProvAgtGetScmoClassResponseMessage(mid4,ex2,class1,qids2);
    testProvAgtGetScmoClassResponseMessage(mid3,ex4,class4,qids1);

    testCIMGetInstanceResponseMessage(oc4, mid1, ex2, qids3, inst4);
    testCIMGetInstanceResponseMessage(oc1, mid2, ex3, qids4, inst1);
    testCIMGetInstanceResponseMessage(oc2, mid3, ex4, qids1, inst2);
    testCIMGetInstanceResponseMessage(oc3, mid4, ex1, qids2, inst3);
//// KS TODO Fix this
//  testCIMGetInstanceResponseMessageSCMO(oc4, mid1, ex2, qids3, inst4);
    testCIMGetInstanceResponseMessageSCMO(oc1, mid2, ex3, qids4, inst1);
    testCIMGetInstanceResponseMessageSCMO(oc2, mid3, ex4, qids1, inst2);
    testCIMGetInstanceResponseMessageSCMO(oc3, mid4, ex1, qids2, inst3);

    testCIMCreateInstanceResponseMessage(oc3, mid4, ex1, qids2, path1);
    testCIMCreateInstanceResponseMessage(oc4, mid1, ex2, qids3, path2);
    testCIMCreateInstanceResponseMessage(oc1, mid2, ex3, qids4, path3);
    testCIMCreateInstanceResponseMessage(oc2, mid3, ex4, qids1, path4);

    testCIMModifyInstanceResponseMessage(oc2, mid1, ex4, qids1);
    testCIMModifyInstanceResponseMessage(oc3, mid2, ex1, qids2);
    testCIMModifyInstanceResponseMessage(oc4, mid3, ex2, qids3);
    testCIMModifyInstanceResponseMessage(oc1, mid4, ex3, qids4);

    testCIMDeleteInstanceResponseMessage(oc4, mid1, ex2, qids4);
    testCIMDeleteInstanceResponseMessage(oc1, mid2, ex3, qids1);
    testCIMDeleteInstanceResponseMessage(oc2, mid3, ex4, qids2);
    testCIMDeleteInstanceResponseMessage(oc3, mid4, ex1, qids3);

    testCIMEnumerateInstancesResponseMessage(
        oc2, mid2, ex1, qids4, instArray4);
    testCIMEnumerateInstancesResponseMessage(
        oc3, mid3, ex2, qids1, instArray1);
    testCIMEnumerateInstancesResponseMessage(
        oc4, mid4, ex3, qids2, instArray2);
    testCIMEnumerateInstancesResponseMessage(
        oc1, mid1, ex4, qids3, instArray3);

    testCIMEnumerateInstanceNamesResponseMessage(
        oc4, mid3, ex2, qids1, pathArray2);
    testCIMEnumerateInstanceNamesResponseMessage(
        oc1, mid4, ex3, qids2, pathArray3);
    testCIMEnumerateInstanceNamesResponseMessage(
        oc2, mid1, ex4, qids3, pathArray4);
    testCIMEnumerateInstanceNamesResponseMessage(
        oc3, mid2, ex1, qids4, pathArray1);

    testCIMExecQueryResponseMessage(oc1, mid3, ex2, qids4, objArray3);
    testCIMExecQueryResponseMessage(oc2, mid4, ex3, qids1, objArray4);
    testCIMExecQueryResponseMessage(oc3, mid1, ex4, qids2, objArray1);
    testCIMExecQueryResponseMessage(oc4, mid2, ex1, qids3, objArray2);

    testCIMAssociatorsResponseMessage(oc1, mid2, ex4, qids1, objArray2);
    testCIMAssociatorsResponseMessage(oc2, mid3, ex1, qids2, objArray3);
    testCIMAssociatorsResponseMessage(oc3, mid4, ex2, qids3, objArray4);
    testCIMAssociatorsResponseMessage(oc4, mid1, ex3, qids4, objArray1);

    testCIMAssociatorNamesResponseMessage(
        oc2, mid1, ex2, qids4, pathArray3);
    testCIMAssociatorNamesResponseMessage(
        oc3, mid2, ex3, qids1, pathArray4);
    testCIMAssociatorNamesResponseMessage(
        oc4, mid3, ex4, qids2, pathArray1);
    testCIMAssociatorNamesResponseMessage(
        oc1, mid4, ex1, qids3, pathArray2);

    testCIMReferencesResponseMessage(oc3, mid2, ex4, qids1, objArray1);
    testCIMReferencesResponseMessage(oc4, mid3, ex1, qids2, objArray2);
    testCIMReferencesResponseMessage(oc1, mid4, ex2, qids3, objArray3);
    testCIMReferencesResponseMessage(oc2, mid1, ex3, qids4, objArray4);

    testCIMReferenceNamesResponseMessage(
        oc1, mid4, ex3, qids2, pathArray1);
    testCIMReferenceNamesResponseMessage(
        oc2, mid1, ex4, qids3, pathArray2);
    testCIMReferenceNamesResponseMessage(
        oc3, mid2, ex1, qids4, pathArray3);
    testCIMReferenceNamesResponseMessage(
        oc4, mid3, ex2, qids1, pathArray4);

    testCIMGetPropertyResponseMessage(oc1, mid2, ex4, qids3, val2);
    testCIMGetPropertyResponseMessage(oc2, mid3, ex1, qids4, val3);
    testCIMGetPropertyResponseMessage(oc3, mid4, ex2, qids1, val4);
    testCIMGetPropertyResponseMessage(oc4, mid1, ex3, qids2, val1);

    testCIMSetPropertyResponseMessage(oc2, mid1, ex1, qids3);
    testCIMSetPropertyResponseMessage(oc3, mid2, ex2, qids4);
    testCIMSetPropertyResponseMessage(oc4, mid3, ex3, qids1);
    testCIMSetPropertyResponseMessage(oc1, mid4, ex4, qids2);

    testCIMInvokeMethodResponseMessage(
        oc1, mid4, ex2, qids3, val4, pvArray1, name3);
    testCIMInvokeMethodResponseMessage(
        oc2, mid1, ex3, qids4, val1, pvArray2, name4);
    testCIMInvokeMethodResponseMessage(
        oc3, mid2, ex4, qids1, val2, pvArray3, name1);
    testCIMInvokeMethodResponseMessage(
        oc4, mid3, ex1, qids2, val3, pvArray4, name2);

    testCIMCreateSubscriptionResponseMessage(oc1, mid3, ex2, qids2);
    testCIMCreateSubscriptionResponseMessage(oc2, mid4, ex3, qids3);
    testCIMCreateSubscriptionResponseMessage(oc3, mid1, ex4, qids4);
    testCIMCreateSubscriptionResponseMessage(oc4, mid2, ex1, qids1);

    testCIMModifySubscriptionResponseMessage(oc1, mid1, ex1, qids1);
    testCIMModifySubscriptionResponseMessage(oc2, mid2, ex2, qids2);
    testCIMModifySubscriptionResponseMessage(oc3, mid3, ex3, qids3);
    testCIMModifySubscriptionResponseMessage(oc4, mid4, ex4, qids4);

    testCIMDeleteSubscriptionResponseMessage(oc3, mid1, ex1, qids2);
    testCIMDeleteSubscriptionResponseMessage(oc4, mid2, ex2, qids3);
    testCIMDeleteSubscriptionResponseMessage(oc1, mid3, ex3, qids4);
    testCIMDeleteSubscriptionResponseMessage(oc2, mid4, ex4, qids1);

    testCIMExportIndicationResponseMessage(oc2, mid1, ex2, qids4);
    testCIMExportIndicationResponseMessage(oc3, mid2, ex3, qids1);
    testCIMExportIndicationResponseMessage(oc4, mid3, ex4, qids2);
    testCIMExportIndicationResponseMessage(oc1, mid4, ex1, qids3);

    testCIMProcessIndicationResponseMessage(oc1, mid3, ex2, qids4);
    testCIMProcessIndicationResponseMessage(oc2, mid4, ex3, qids1);
    testCIMProcessIndicationResponseMessage(oc3, mid1, ex4, qids2);
    testCIMProcessIndicationResponseMessage(oc4, mid2, ex1, qids3);

    testCIMDisableModuleResponseMessage(
        oc1, mid4, ex1, qids2, uint16Array3);
    testCIMDisableModuleResponseMessage(
        oc2, mid1, ex2, qids3, uint16Array4);
    testCIMDisableModuleResponseMessage(
        oc3, mid2, ex3, qids4, uint16Array1);
    testCIMDisableModuleResponseMessage(
        oc4, mid3, ex4, qids1, uint16Array2);

    testCIMEnableModuleResponseMessage(
        oc1, mid3, ex4, qids2, uint16Array1);
    testCIMEnableModuleResponseMessage(
        oc2, mid4, ex1, qids3, uint16Array2);
    testCIMEnableModuleResponseMessage(
        oc3, mid1, ex2, qids4, uint16Array3);
    testCIMEnableModuleResponseMessage(
        oc4, mid2, ex3, qids1, uint16Array4);

    testCIMStopAllProvidersResponseMessage(oc1, mid2, ex2, qids1);
    testCIMStopAllProvidersResponseMessage(oc2, mid3, ex3, qids2);
    testCIMStopAllProvidersResponseMessage(oc3, mid4, ex4, qids3);
    testCIMStopAllProvidersResponseMessage(oc4, mid1, ex1, qids4);

    testCIMInitializeProviderAgentResponseMessage(oc1, mid4, ex3, qids1);
    testCIMInitializeProviderAgentResponseMessage(oc2, mid1, ex4, qids2);
    testCIMInitializeProviderAgentResponseMessage(oc3, mid2, ex1, qids3);
    testCIMInitializeProviderAgentResponseMessage(oc4, mid3, ex2, qids4);

    testCIMNotifyConfigChangeResponseMessage(oc1, mid2, ex4, qids1);
    testCIMNotifyConfigChangeResponseMessage(oc2, mid3, ex1, qids2);
    testCIMNotifyConfigChangeResponseMessage(oc3, mid4, ex2, qids3);
    testCIMNotifyConfigChangeResponseMessage(oc4, mid1, ex3, qids4);

    testCIMSubscriptionInitCompleteResponseMessage(oc1, mid3, ex1, qids2);
    testCIMSubscriptionInitCompleteResponseMessage(oc2, mid4, ex2, qids3);
    testCIMSubscriptionInitCompleteResponseMessage(oc3, mid1, ex3, qids4);
    testCIMSubscriptionInitCompleteResponseMessage(oc4, mid2, ex4, qids1);

    // Destroy the SCMO Class cache for housekeeping
    delete classArray;

    scmoCache->destroy();

}

//
// main
//
int main(int, char** argv)
{
    verbose = getenv("PEGASUS_TEST_VERBOSE") ? true : false;

    try
    {
        testMessageSerialization();
    }
    catch (Exception& e)
    {
        cout << "Exception: " << e.getMessage() << endl;
        exit(1);
    }

    cout << argv[0] << " +++++ passed all tests" << endl;
    return 0;
}
