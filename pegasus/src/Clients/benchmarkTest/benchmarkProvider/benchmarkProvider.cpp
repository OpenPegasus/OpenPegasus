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

#include "benchmarkProvider.h"
#include "../benchmarkDefinition/benchmarkDefinition.h"

PEGASUS_USING_STD;

PEGASUS_NAMESPACE_BEGIN

#define NAMESPACE CIMNamespaceName ("root/benchmark")

benchmarkDefinition test;

benchmarkProvider::benchmarkProvider(void)
{
}

benchmarkProvider::~benchmarkProvider(void)
{
}

CIMObjectPath benchmarkProvider::_buildObjectPath(
                         const CIMName& className,
                         CIMKeyBinding keyBinding)
{
    Array<CIMKeyBinding> keyBindings;
    keyBindings.append(keyBinding);
    return CIMObjectPath(String(), CIMNamespaceName(NAMESPACE),
                                className, keyBindings);
}

CIMInstance benchmarkProvider::_buildInstance(
                         const    CIMName& className,
                         Uint32   numberOfProperties,
                         Uint32   propertySize,
                         CIMValue Identifier)

{
    char propertyName[20];
    char propertyValue[100000];

   if (propertySize > 99999)
   {
      throw CIMException (CIM_ERR_INVALID_PARAMETER);
   }

    for (Uint32 i = 0; i < propertySize; i++)
    {
       propertyValue[i] = 'a';
    }
    propertyValue[propertySize] = 0;

    CIMInstance instance(className);
    instance.addProperty(CIMProperty(CIMName("Identifier"), Identifier));

    for(Uint32 i = 1; i <= numberOfProperties;  i++)
    {
       sprintf(propertyName, "Property%4.4u", i);
       instance.addProperty(
           CIMProperty(CIMName(propertyName), String(propertyValue)));
    }

    CIMObjectPath reference = _buildObjectPath(className,
               CIMKeyBinding(CIMName("Identifier"), Identifier));
    instance.setPath(reference);
    return(instance);
}

void benchmarkProvider::initialize(CIMOMHandle & cimom)
{
}

void benchmarkProvider::terminate(void)
{
}

void benchmarkProvider::getInstance(
    const OperationContext & context,
    const CIMObjectPath & instanceReference,
    const Boolean includeQualifiers,
    const Boolean includeClassOrigin,
    const CIMPropertyList & propertyList,
    InstanceResponseHandler & handler)
{
    CIMInstance _instance;
    Uint32 numberOfProperties;
    Uint32 sizeOfPropertyValue;
    Uint32 numberOfInstances;

    CIMName className = instanceReference.getClassName();
    test.getConfiguration(className, numberOfProperties,
                 sizeOfPropertyValue, numberOfInstances);

    Array<CIMKeyBinding> keyBindings = instanceReference.getKeyBindings();
    if (keyBindings.size() != 1)
    {
       throw CIMException(CIM_ERR_NOT_SUPPORTED);
    }

    // begin processing the request
    handler.processing();

    Uint32 ID;
    if (sscanf (keyBindings[0].getValue().getCString(), "%u", &ID) != 1)
    {
        throw CIMException (CIM_ERR_INVALID_PARAMETER);
    }

    _instance = _buildInstance(className, numberOfProperties,
                        sizeOfPropertyValue , CIMValue(ID));

    handler.deliver(_instance);

    // complete processing the request
    handler.complete();
}

void benchmarkProvider::enumerateInstances(
    const OperationContext & context,
    const CIMObjectPath & classReference,
    const Boolean includeQualifiers,
    const Boolean includeClassOrigin,
    const CIMPropertyList & propertyList,
    InstanceResponseHandler & handler)
{
    CIMInstance _instance;
    Uint32 numberOfProperties;
    Uint32 sizeOfPropertyValue;
    Uint32 numberOfInstances;

    CIMName className = classReference.getClassName();
    test.getConfiguration(className, numberOfProperties,
                          sizeOfPropertyValue, numberOfInstances);

    // begin processing the request
    handler.processing();

    for (Uint32 i = 1; i <= numberOfInstances; i++)
    {
       _instance = _buildInstance(className, numberOfProperties,
                        sizeOfPropertyValue , CIMValue(i));
       handler.deliver(_instance);
    }

    // complete processing the request
    handler.complete();
}

void benchmarkProvider::enumerateInstanceNames(
    const OperationContext & context,
    const CIMObjectPath & classReference,
    ObjectPathResponseHandler & handler)
{
    CIMObjectPath _instanceName;
    Uint32 numberOfProperties;
    Uint32 sizeOfPropertyValue;
    Uint32 numberOfInstances;

    CIMName className = classReference.getClassName();
    test.getConfiguration(className, numberOfProperties,
                          sizeOfPropertyValue, numberOfInstances);

    // begin processing the request
    handler.processing();

    for (Uint32 i = 1; i <= numberOfInstances; i++)
    {
       _instanceName = _buildObjectPath(className,
               CIMKeyBinding(CIMName("Identifier"), CIMValue(i)));
       handler.deliver(_instanceName);
    }

    // complete processing the request
    handler.complete();
}

void benchmarkProvider::modifyInstance(
    const OperationContext & context,
    const CIMObjectPath & instanceReference,
    const CIMInstance & instanceObject,
    const Boolean includeQualifiers,
    const CIMPropertyList & propertyList,
    ResponseHandler & handler)
{
    throw CIMException(CIM_ERR_NOT_SUPPORTED);
}

void benchmarkProvider::createInstance(
    const OperationContext & context,
    const CIMObjectPath & instanceReference,
    const CIMInstance & instanceObject,
    ObjectPathResponseHandler & handler)
{
    throw CIMException(CIM_ERR_NOT_SUPPORTED);
}

void benchmarkProvider::deleteInstance(
    const OperationContext & context,
    const CIMObjectPath & instanceReference,
    ResponseHandler & handler)
{
    throw CIMException(CIM_ERR_NOT_SUPPORTED);
}

PEGASUS_NAMESPACE_END
