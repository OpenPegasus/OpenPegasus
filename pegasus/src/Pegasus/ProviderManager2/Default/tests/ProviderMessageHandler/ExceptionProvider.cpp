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

#include <exception>

#include "ExceptionProvider.h"

PEGASUS_USING_STD;

PEGASUS_NAMESPACE_BEGIN

class TestCxxException : public exception
{
    virtual const char* what() const throw()
    {
        return "test C++ exception";
    }
};

ExceptionProvider::ExceptionProvider()
{
}

ExceptionProvider::~ExceptionProvider()
{
}

void ExceptionProvider::initialize(CIMOMHandle& cimom)
{
}

void ExceptionProvider::terminate()
{
    delete this;
}

void _throwException(const String& exceptionType)
{
    if (exceptionType == "CIM_Exception")
    {
        throw CIMException(CIM_ERR_NOT_FOUND, "test cim exception");
    }
    else if (exceptionType == "Regular_Exception")
    {
        throw Exception("test regular exception");
    }
    else if (exceptionType == "Cxx_Exception")
    {
        throw TestCxxException();
    }
    else
    {
        throw Uint32(1);
    }
}

void ExceptionProvider::getInstance(
    const OperationContext& context,
    const CIMObjectPath& instanceReference,
    const Boolean includeQualifiers,
    const Boolean includeClassOrigin,
    const CIMPropertyList& propertyList,
    InstanceResponseHandler& handler)
{
    _throwException(instanceReference.getClassName().getString());
}

void ExceptionProvider::enumerateInstances(
    const OperationContext& context,
    const CIMObjectPath& classReference,
    const Boolean includeQualifiers,
    const Boolean includeClassOrigin,
    const CIMPropertyList& propertyList,
    InstanceResponseHandler& handler)
{
    _throwException(classReference.getClassName().getString());
}

void ExceptionProvider::enumerateInstanceNames(
    const OperationContext& context,
    const CIMObjectPath& classReference,
    ObjectPathResponseHandler& handler)
{
    _throwException(classReference.getClassName().getString());
}

void ExceptionProvider::modifyInstance(
    const OperationContext& context,
    const CIMObjectPath& instanceReference,
    const CIMInstance& instanceObject,
    const Boolean includeQualifiers,
    const CIMPropertyList& propertyList,
    ResponseHandler& handler)
{
    _throwException(instanceReference.getClassName().getString());
}

void ExceptionProvider::createInstance(
    const OperationContext& context,
    const CIMObjectPath& instanceReference,
    const CIMInstance& instanceObject,
    ObjectPathResponseHandler& handler)
{
    _throwException(instanceReference.getClassName().getString());
}

void ExceptionProvider::deleteInstance(
    const OperationContext& context,
    const CIMObjectPath& instanceReference,
    ResponseHandler& handler)
{
    _throwException(instanceReference.getClassName().getString());
}

void ExceptionProvider::execQuery(
    const OperationContext& context,
    const CIMObjectPath& nameSpaceAndClass,
    const QueryExpression& query,
    InstanceResponseHandler& handler)
{
    _throwException(nameSpaceAndClass.getClassName().getString());
}


void ExceptionProvider::associators(
    const OperationContext& context,
    const CIMObjectPath& objectName,
    const CIMName& associationClass,
    const CIMName& resultClass,
    const String& role,
    const String& resultRole,
    const Boolean includeQualifiers,
    const Boolean includeClassOrigin,
    const CIMPropertyList& propertyList,
    ObjectResponseHandler& handler)
{
    _throwException(objectName.getClassName().getString());
}

void ExceptionProvider::associatorNames(
    const OperationContext& context,
    const CIMObjectPath& objectName,
    const CIMName& associationClass,
    const CIMName& resultClass,
    const String& role,
    const String& resultRole,
    ObjectPathResponseHandler& handler)
{
    _throwException(objectName.getClassName().getString());
}

void ExceptionProvider::references(
    const OperationContext& context,
    const CIMObjectPath& objectName,
    const CIMName& resultClass,
    const String& role,
    const Boolean includeQualifiers,
    const Boolean includeClassOrigin,
    const CIMPropertyList& propertyList,
    ObjectResponseHandler& handler)
{
    _throwException(objectName.getClassName().getString());
}

void ExceptionProvider::referenceNames(
    const OperationContext& context,
    const CIMObjectPath& objectName,
    const CIMName& resultClass,
    const String& role,
    ObjectPathResponseHandler& handler)
{
    _throwException(objectName.getClassName().getString());
}

void ExceptionProvider::invokeMethod(
    const OperationContext& context,
    const CIMObjectPath& objectReference,
    const CIMName& methodName,
    const Array<CIMParamValue>& inParameters,
    MethodResultResponseHandler& handler)
{
    _throwException(objectReference.getClassName().getString());
}

void ExceptionProvider::enableIndications(IndicationResponseHandler& handler)
{
}

void ExceptionProvider::disableIndications()
{
}

void ExceptionProvider::createSubscription(
    const OperationContext& context,
    const CIMObjectPath& subscriptionName,
    const Array<CIMObjectPath>& classNames,
    const CIMPropertyList& propertyList,
    const Uint16 repeatNotificationPolicy)
{
    _throwException(subscriptionName.getClassName().getString());
}

void ExceptionProvider::modifySubscription(
    const OperationContext& context,
    const CIMObjectPath& subscriptionName,
    const Array<CIMObjectPath>& classNames,
    const CIMPropertyList& propertyList,
    const Uint16 repeatNotificationPolicy)
{
    _throwException(subscriptionName.getClassName().getString());
}

void ExceptionProvider::deleteSubscription(
    const OperationContext& context,
    const CIMObjectPath& subscriptionName,
    const Array<CIMObjectPath>& classNames)
{
    _throwException(subscriptionName.getClassName().getString());
}

// CIMIndicationConsumerProvider interface
void ExceptionProvider::consumeIndication(
    const OperationContext& context,
    const String& destinationPath,
    const CIMInstance& indication)
{
    _throwException(destinationPath);
}

PEGASUS_NAMESPACE_END
