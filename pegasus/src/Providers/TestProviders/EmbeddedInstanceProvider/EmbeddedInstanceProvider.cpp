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
#include <Pegasus/Common/CIMDateTime.h>
#include <Pegasus/Common/System.h>

#include "EmbeddedInstanceProvider.h"

PEGASUS_USING_STD;

PEGASUS_USING_PEGASUS;


EmbeddedInstanceProvider::EmbeddedInstanceProvider()
    : indicationHandler(0),
      errorInstance(0),
      indicationInstance(0),
      STATIC_REPOSITORY("test/EmbeddedInstance/Static")
{
}

EmbeddedInstanceProvider::~EmbeddedInstanceProvider()
{
}

void EmbeddedInstanceProvider::initialize(CIMOMHandle& cimomHandle)
{
    cimom = cimomHandle;
}

void EmbeddedInstanceProvider::terminate()
{
    delete this;
}

void EmbeddedInstanceProvider::enableIndications(
    IndicationResponseHandler& handler)
{
    indicationHandler = &handler;
}

void EmbeddedInstanceProvider::disableIndications()
{
    indicationHandler = 0;
}

void EmbeddedInstanceProvider::createSubscription(
    const OperationContext& context,
    const CIMObjectPath& subscriptionName,
    const Array <CIMObjectPath>& classNames,
    const CIMPropertyList& propertyList,
    const Uint16 repeatNotificationPolicy)
{
    // I'm only expecting to receive one indication, and this is just
    // a test provider
}

void EmbeddedInstanceProvider::modifySubscription(
    const OperationContext& context,
    const CIMObjectPath& subscriptionName,
    const Array <CIMObjectPath>& classNames,
    const CIMPropertyList& propertyList,
    const Uint16 repeatNotificationPolicy)
{
    // I'm only expecting to receive one indication, and this is just
    // a test provider
}

void EmbeddedInstanceProvider::deleteSubscription(
    const OperationContext& context,
    const CIMObjectPath& subscriptionName,
    const Array <CIMObjectPath>& classNames)
{
    // I'm only expecting to receive one indication, and this is just
    // a test provider
}

/**
    When the "PropagateError" method is invoked, this function
    will read a CIM_Error embedded instance from the inParameters and use
    that instance to create an InstMethodIndication populating the Error[]
    property with the CIM_Error parameter and errorInstance stored in this
    class (if it's been created already). The output parameter of this method
    will contain the embedded instance received as input, and the newly
    created InstMethodIndication will be sent by the provider to any
    registered listeners.
*/
void EmbeddedInstanceProvider::invokeMethod(
        const OperationContext& context,
        const CIMObjectPath& objectReference,
        const CIMName& methodName,
        const Array<CIMParamValue>& inParameters,
        MethodResultResponseHandler& handler)
{
    // This should start sending indications with the stored Job instance
    // embedded in the InstMethodIndication
    handler.processing();
    if (!methodName.equal(CIMName("PropagateError")))
        throw CIMException(CIM_ERR_METHOD_NOT_AVAILABLE);

    if (inParameters.size() != 1)
    {
        throw CIMException(
            CIM_ERR_INVALID_PARAMETER, "Did not receive exactly 1 parameter");
    }

    CIMParamValue errorParam = inParameters[0];
    if (!String::equal(errorParam.getParameterName(), String("error")))
    {
        throw CIMException(
            CIM_ERR_INVALID_PARAMETER, "Did not find \"error\" parameter");
    }

    CIMInstance errorParamInst;
    errorParam.getValue().get(errorParamInst);
    if (errorParamInst.getPath().getClassName() != "PG_EmbeddedError")
    {
        throw CIMException(CIM_ERR_INVALID_PARAMETER);
    }
    errorParamInst.setPath(errorInstance->getPath());
    Array<CIMName> propList;
    for (unsigned int i = 0, n = errorParamInst.getPropertyCount(); i < n; i++)
    {
        propList.append(errorParamInst.getProperty(i).getName());
    }
    CIMInstance tmpErrorInstance(errorInstance->clone());
    if (!tmpErrorInstance.identical(errorParamInst))
    {
        throw Exception("Did not receive expected ErrorInstance");
    }

    // Build new indication instance
    CIMObjectPath indicationPath(System::getHostName(),
        CIMNamespaceName("test/EmbeddedInstance/Dynamic"),
        CIMName("PG_InstMethodIndication"));
    indicationInstance.reset(new CIMInstance("PG_InstMethodIndication"));
    indicationInstance->setPath(indicationPath);
    indicationInstance->addProperty(CIMProperty("MethodName",
        CIMValue(String("PropagateError"))));
    indicationInstance->addProperty(CIMProperty("PreCall",
        CIMValue(Boolean(false))));
    indicationInstance->addProperty(CIMProperty("SourceInstance",
        CIMValue(CIMObject(*errorInstance))));
    Array<CIMInstance> errorInstances;
    errorInstances.append(*errorInstance);
    errorInstances.append(*errorInstance);
    indicationInstance->addProperty(CIMProperty("Error",
        CIMValue(errorInstances)));

    handler.deliverParamValue(CIMParamValue(String("sameError"),
        CIMValue(errorParamInst)));
    handler.deliver(CIMValue(Uint32(1)));
    handler.complete();

    indicationHandler->deliver(*indicationInstance);
}

/**
    Retrieves either the stored errorInstance or indicationInstance from
    both the class and the static repository, performs a comparison, and
    returns the instance if they are the same.
*/
void EmbeddedInstanceProvider::getInstance(
    const OperationContext& context,
    const CIMObjectPath& ref,
    const Boolean includeQualifiers,
    const Boolean includeClassOrigin,
    const CIMPropertyList& propertyList,
    InstanceResponseHandler& handler)
{
    handler.processing();
    CIMInstance retInst;
    if (ref.getClassName().equal(CIMName("PG_EmbeddedError")))
        retInst = errorInstance->clone();
    else if (ref.getClassName().equal(CIMName("PG_InstMethodIndication")))
        retInst = indicationInstance->clone();

    handler.deliver(retInst);
    handler.complete();
}

/**
    Will send a number of copies of either the stored errorInstance
    or indicationInstance.
*/
void EmbeddedInstanceProvider::enumerateInstances(
    const OperationContext& context,
    const CIMObjectPath& ref,
    const Boolean includeQualifiers,
    const Boolean includeClassOrigin,
    const CIMPropertyList& propertyList,
    InstanceResponseHandler& handler)
{
    handler.processing();
    CIMInstance retInst;
    if (ref.getClassName().equal(CIMName("PG_EmbeddedError")))
        retInst = errorInstance->clone();
    else if (ref.getClassName().equal(CIMName("PG_InstMethodIndication")))
        retInst = indicationInstance->clone();

    for (int i = 0; i < 5; i++)
        handler.deliver(retInst);
    handler.complete();
}

/**
    Returns the instance names of the errorInstance or indicationInstance
    stored in this class.
*/
void EmbeddedInstanceProvider::enumerateInstanceNames(
    const OperationContext& context,
    const CIMObjectPath& ref,
    ObjectPathResponseHandler& handler)
{
    handler.processing();
    if (ref.getClassName().equal(CIMName("PG_EmbeddedError")))
    {
        CIMObjectPath errorInstancePath = errorInstance->getPath();
        handler.deliver(errorInstancePath);
        handler.deliver(errorInstancePath);
        handler.deliver(errorInstancePath);
        handler.deliver(errorInstancePath);
        handler.deliver(errorInstancePath);
    }
    else if (ref.getClassName().equal(CIMName("PG_InstMethodIndication")))
    {
        CIMObjectPath indicationInstancePath = indicationInstance->getPath();
        handler.deliver(indicationInstancePath);
        handler.deliver(indicationInstancePath);
        handler.deliver(indicationInstancePath);
        handler.deliver(indicationInstancePath);
        handler.deliver(indicationInstancePath);
    }
    handler.complete();
}

/**
    TBD?
*/
void EmbeddedInstanceProvider::modifyInstance(
    const OperationContext& context,
    const CIMObjectPath& ref,
    const CIMInstance& obj,
    const Boolean includeQualifiers,
    const CIMPropertyList& propertyList,
    ResponseHandler& handler)
{
    throw CIMException(CIM_ERR_NOT_SUPPORTED);
//    handler.processing();
//    handler.complete();
}

/**
    Stores a copy of the provided instance locally in the errorInstance
    class property, and also creates a copy of the instance in the static
    repository namespace. The instance is then read back out of the repository
    and compared against the one stored by the class.
*/
void EmbeddedInstanceProvider::createInstance(
    const OperationContext& context,
    const CIMObjectPath& ref,
    const CIMInstance& obj,
    ObjectPathResponseHandler& handler)
{
    handler.processing();
    errorInstance.reset(new CIMInstance(obj));
    Array<CIMName> propNameList;
    propNameList.append(CIMName("errorKey"));
    propNameList.append(CIMName("EmbeddedInst"));
    CIMPropertyList propList(propNameList);
    CIMClass objClass = cimom.getClass(
        context,
        STATIC_REPOSITORY,
        obj.getClassName(),
        false,
        true,
        false,
        propList);
    CIMObjectPath objPath = obj.buildPath(objClass);
    errorInstance->setPath(objPath);

    repositoryPath = cimom.createInstance(
        context, STATIC_REPOSITORY, *errorInstance);

    CIMInstance repositoryInstance = cimom.getInstance(
        context,
        STATIC_REPOSITORY,
        repositoryPath,
        false,
        false,
        false,
        propList);
    repositoryInstance.setPath(repositoryPath);
    
    if (!errorInstance->identical(repositoryInstance))
    {
        throw Exception(
            "Repository instance and local instance for embedded error not "
                "identical");
    }

    handler.deliver(objPath);
    handler.complete();
}

/**
  * Removes the specified instance from the repository. The provider
  * then tries to retrieve the instance from the repository which should fail.
  */
void EmbeddedInstanceProvider::deleteInstance(
    const OperationContext& context,
    const CIMObjectPath& ref,
    ResponseHandler& handler)
{
    handler.processing();
    if (ref.getClassName().equal(CIMName("PG_EmbeddedError")))
    {
      cimom.deleteInstance(
          context,
          STATIC_REPOSITORY,
          repositoryPath);
    }
    handler.complete();
}
