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

#include <stdio.h>
#include <iostream>
#include "LargeDataProvider.h"
#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/Constants.h>
#include <Pegasus/Common/CIMDateTime.h>

PEGASUS_USING_STD;
#ifndef NO_OF_INSTANCES
#define NO_OF_INSTANCES 100
#endif

PEGASUS_NAMESPACE_BEGIN

LargeDataProvider::LargeDataProvider(void)
{
}

LargeDataProvider::~LargeDataProvider(void)
{
}

void LargeDataProvider::initialize(CIMOMHandle & cimom)
{
    cout << "-----------------------------" << endl;
    cout << "LargeDataProvider::initialize" << endl;
    cout << "-----------------------------" << endl;
}

void LargeDataProvider::terminate(void)
{
    // Supporting Large data handling.
}

void LargeDataProvider::getInstance(
    const OperationContext & context,
    const CIMObjectPath & instanceReference,
    const Boolean includeQualifiers,
    const Boolean includeClassOrigin,
    const CIMPropertyList & propertyList,
    InstanceResponseHandler & handler)
{
    cout << "------------------------------" << endl;
    cout << "LargeDataProvider::getInstance" << endl;
    cout << "------------------------------" << endl;
    // convert a potential fully qualified reference into a local reference
    // (class name and keys only).
    CIMObjectPath localReference = CIMObjectPath(
        String(),
        String(),
        instanceReference.getClassName(),
        instanceReference.getKeyBindings());

    // begin processing the request
    handler.processing();

    // instance index corresponds to reference index
    for(Uint32 i = 0, n = _instances.size(); i < n; i++)
    {
        if(localReference == _instanceNames[i])
        {
            // deliver requested instance
            handler.deliver(_instances[i]);
            break;
        }
    }
    // complete processing the request
    handler.complete();
}

void LargeDataProvider::enumerateInstances(
    const OperationContext & context,
    const CIMObjectPath & classReference,
    const Boolean includeQualifiers,
    const Boolean includeClassOrigin,
    const CIMPropertyList & propertyList,
    InstanceResponseHandler & handler)
{
    char buffer[NO_OF_INSTANCES];
    CIMInstance Instances[NO_OF_INSTANCES];
    CIMObjectPath References[NO_OF_INSTANCES];

    cout << "-------------------------------------" << endl;
    cout << "LargeDataProvider::enumerateInstances" << endl;
    cout << "-------------------------------------" << endl;

    // announce operation processing.
    handler.processing();

    // creating some instances in a loop for generating a large amount of data
    // The number of instances is controlled by the macro var. NO_OF_INSTANCES.
    // TODO:: Need to fix the way in which the value of this variable is passed

    cout << "Number of Instances = " << NO_OF_INSTANCES << endl;
    for (Uint32 i = 0; i < NO_OF_INSTANCES; i++)
    {
        sprintf(buffer, "%d", i);

        Instances[i] = CIMInstance("SampleClass");
        References[i] = CIMObjectPath("SampleClass.Id="+String(buffer));

        Instances[i].addProperty(CIMProperty("Id", Uint16(i)));
        Instances[i].addProperty(CIMProperty("Message", String(buffer)));
        Instances[i].addProperty(CIMProperty("ReqType", String("Local")));
        Instances[i].addProperty(CIMProperty("RequestNumber", Uint16(i+10)));
        Instances[i].addProperty(CIMProperty("TimeSpent", Uint16(i+2)));
        Instances[i].addProperty(CIMProperty("TimeIdeal", Uint16(100)));
        Instances[i].addProperty(
            CIMProperty("Performance", String("OK \00><\00")));
        Instances[i].addProperty(CIMProperty("EndPoint",
                            String("Instance "+String(buffer)+" Ends")));

        _instances.append(Instances[i]);
        _instanceNames.append(References[i]);
    }

    for(Uint32 i = 0; i < NO_OF_INSTANCES; i++)
        // deliver instance
        handler.deliver(_instances[i]);

    // complete processing the request
    handler.complete();
}

void LargeDataProvider::enumerateInstanceNames(
    const OperationContext & context,
    const CIMObjectPath & classReference,
    ObjectPathResponseHandler & handler)
{
    cout << "-----------------------------------------" << endl;
    cout << "LargeDataProvider::enumerateInstanceNames" << endl;
    cout << "-----------------------------------------" << endl;
    // begin processing the request
    handler.processing();
    cout << "_instances.size = " << _instances.size() << endl;
    for(Uint32 i = 0; i < _instances.size(); i++)
        // deliver references
        handler.deliver(_instanceNames[i]);

    // complete processing the request
    handler.complete();
}

void LargeDataProvider::modifyInstance(
    const OperationContext & context,
    const CIMObjectPath & instanceReference,
    const CIMInstance & instanceObject,
    const Boolean includeQualifiers,
    const CIMPropertyList & propertyList,
    ResponseHandler & handler)
{
    cout << "---------------------------------" << endl;
    cout << "LargeDataProvider::modifyInstance" << endl;
    cout << "---------------------------------" << endl;
    // convert a potential fully qualified reference into a local reference
    // (class name and keys only).
    CIMObjectPath localReference = CIMObjectPath(
        String(),
        String(),
        instanceReference.getClassName(),
        instanceReference.getKeyBindings());

    // begin processing the request
    handler.processing();

    // instance index corresponds to reference index
    for(Uint32 i = 0, n = _instances.size(); i < n; i++)
    {
        if(localReference == _instanceNames[i])
        {
            // overwrite existing instance
            _instances[i] = instanceObject;
            break;
        }
    }

    // complete processing the request
    handler.complete();
}

void LargeDataProvider::createInstance(
    const OperationContext & context,
    const CIMObjectPath & instanceReference,
    const CIMInstance & instanceObject,
    ObjectPathResponseHandler & handler)
{
    cout << "---------------------------------" << endl;
    cout << "LargeDataProvider::createInstance" << endl;
    cout << "---------------------------------" << endl;

    // convert a potential fully qualified reference into a local reference
    // (class name and keys only).
    CIMObjectPath localReference = CIMObjectPath(
        String(),
        String(),
        instanceReference.getClassName(),
        instanceReference.getKeyBindings());

    // instance index corresponds to reference index
    for(Uint32 i = 0, n = _instanceNames.size(); i < n; i++)
    {
        if(localReference == _instanceNames[i])
        {
            throw CIMObjectAlreadyExistsException(
                                  localReference.toString());
        }
    }

    // begin processing the request
    handler.processing();

    // add the new instance to the array
    _instances.append(instanceObject);
    _instanceNames.append(instanceReference);

    // deliver the new instance
    handler.deliver(_instanceNames[_instanceNames.size() - 1]);

    // complete processing the request
    handler.complete();
}

void LargeDataProvider::deleteInstance(
    const OperationContext & context,
    const CIMObjectPath & instanceReference,
    ResponseHandler & handler)
{
    cout << "---------------------------------" << endl;
    cout << "LargeDataProvider::deleteInstance" << endl;
    cout << "---------------------------------" << endl;
    // convert a potential fully qualified reference into a local reference
    // (class name and keys only).
    CIMObjectPath localReference = CIMObjectPath(
        String(),
        String(),
        instanceReference.getClassName(),
        instanceReference.getKeyBindings());

    // begin processing the request
    handler.processing();

    // instance index corresponds to reference index
    for(Uint32 i = 0, n = _instances.size(); i < n; i++)
    {
        if(localReference == _instanceNames[i])
        {
            // save the instance locally
            CIMInstance cimInstance(_instances[i]);

            // remove instance from the array
            _instances.remove(i);
            _instanceNames.remove(i);

            // exit loop
            break;
        }
    }

    // complete processing the request
    handler.complete();
}

PEGASUS_NAMESPACE_END
