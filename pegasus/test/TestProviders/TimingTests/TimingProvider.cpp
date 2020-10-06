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
//%////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <unistd.h>
#include <iostream>
#include <Pegasus/Common/String.h>
#include <Pegasus/Common/InternalException.h>
#include "TimingProvider.h"

PEGASUS_NAMESPACE_BEGIN
PEGASUS_USING_STD;

#define PEGASUS_USE_DEPRECATED_INTERFACES
TimingProvider::TimingProvider(void)
{
}

TimingProvider::~TimingProvider(void)
{
}

void TimingProvider::initialize(CIMOMHandle & cimom)
{
    cout <<" TimingProvider::initialize" << endl;

    // Create Instances for TimeOne Class
    CIMInstance instance1("TimeOne");
    CIMObjectPath reference1("TimeOne.Id=1");

    instance1.addProperty(CIMProperty("Id", Uint8(1)));   // key
    instance1.addProperty(CIMProperty("Message", String("Class One I1")));

    _instances.append(instance1);
    _instanceNames.append(reference1);


    CIMInstance instance2("TimeOne");
    CIMObjectPath reference2("TimeOne.Id=2");

    instance2.addProperty(CIMProperty("Id", Uint8(2)));   // key
    instance2.addProperty(CIMProperty("Message", String("Class One I2")));

    _instances.append(instance2);
    _instanceNames.append(reference2);

    // Create Instances for TimeTwo Class
     CIMInstance instance_1("TimeTwo");
     CIMObjectPath reference_1("TimeTwo.Srno=22");

    instance_1.addProperty(CIMProperty("Srno", Uint8(1)));   // key
    instance_1.addProperty(CIMProperty("TimeVar", Uint16(1111)));
    _instances_2.append(instance_1);
    _instanceNames_2.append(reference_1);

    CIMInstance instance_2("TimeTwo");
    CIMObjectPath reference_2("TimeTwo.Srno=2");

    instance_2.addProperty(CIMProperty("Srno", Uint8(2)));   // key
    instance_2.addProperty(CIMProperty("TimeVar", Uint16(2222)));

    _instances_2.append(instance_2);
    _instanceNames_2.append(reference_2);
}

void TimingProvider::terminate(void)
{
    cout <<" TimingProvider::terminate" << endl;
}

void TimingProvider::getInstance(
    const OperationContext & context,
    const CIMObjectPath & instanceReference,
    const Boolean includeQualifiers,
    const Boolean includeClassOrigin,
    const CIMPropertyList & propertyList,
    InstanceResponseHandler & handler)
{
    // convert a potential fully qualified reference into a local reference
    // (class name and keys only).
    CIMObjectPath localReference = CIMObjectPath(
        String(),
        String(),
        instanceReference.getClassName(),
        instanceReference.getKeyBindings());

    CIMName className = instanceReference.getClassName();

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

void TimingProvider::enumerateInstances(
    const OperationContext & context,
    const CIMObjectPath & classReference,
    const Boolean includeQualifiers,
    const Boolean includeClassOrigin,
    const CIMPropertyList & propertyList,
    InstanceResponseHandler & handler)
{
    cout <<"TimingProvider::enumerateInstances" << endl;
    CIMName cn = classReference.getClassName();

    String className = cn.getString();
    sleep(15);
    // cout << "className = " << className << endl;
    // Create Instances for TimeOne Class
    if (String::equalNoCase(className, "TimeOne"))
    {
        sleep(5);
        // announce operation processing.
        handler.processing();
        for(Uint32 i = 0, n = _instances.size(); i < n; i++)
            // deliver instance
            handler.deliver(_instances[i]);
    }
    // Create Instances for SecondClass
    if (String::equalNoCase(className, "TimeTwo"))
    {
        handler.processing();
        sleep(5);

        for(Uint32 i = 0, n = _instances_2.size(); i < n; i++)
            // deliver instance
            handler.deliver(_instances_2[i]);
    }
    // complete processing the request
    handler.complete();
}

void TimingProvider::enumerateInstanceNames(
    const OperationContext & context,
    const CIMObjectPath & classReference,
    ObjectPathResponseHandler & handler)
{
    // begin processing the request
    handler.processing();
    cout <<"TimingProvider::enumerateInstanceNames" << endl;
    CIMName clName = classReference.getClassName();

    sleep(10);
    if (clName == "TimeOne")
    {
        for(Uint32 i = 0, n = _instances.size(); i < n; i++)
            // deliver reference
            handler.deliver(_instanceNames[i]);
    }
    else if (clName == "TimeTwo")
    {
        for(Uint32 i = 0, n = _instances_2.size(); i < n; i++)
            // deliver reference
            handler.deliver(_instanceNames_2[i]);
    }
    // complete processing the request
    cout <<"TimingProvider::enumerateInstanceNames" << endl;
    handler.complete();
}

void TimingProvider::modifyInstance(
    const OperationContext & context,
    const CIMObjectPath & instanceReference,
    const CIMInstance & instanceObject,
    const Boolean includeQualifiers,
    const CIMPropertyList & propertyList,
    ResponseHandler & handler)
{
    // convert a potential fully qualified reference into a local reference
    // (class name and keys only).
    CIMObjectPath localReference = CIMObjectPath(
        String(),
        String(),
        instanceReference.getClassName(),
        instanceReference.getKeyBindings());
    cout <<"TimingProvider::modifyInstance" << endl;
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

void TimingProvider::createInstance(
    const OperationContext & context,
    const CIMObjectPath & instanceReference,
    const CIMInstance & instanceObject,
    ObjectPathResponseHandler & handler)
{
    // convert a potential fully qualified reference into a local reference
    // (class name and keys only).
    CIMObjectPath localReference = CIMObjectPath(
        String(),
        String(),
        instanceReference.getClassName(),
        instanceReference.getKeyBindings());

    cout <<"TimingProvider::createInstance" << endl;
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

    cout <<"TimingProvider::createInstance" << endl;
    // deliver the new instance
    handler.deliver(_instanceNames[_instanceNames.size() - 1]);

    // complete processing the request
    handler.complete();
}

void TimingProvider::deleteInstance(
    const OperationContext & context,
    const CIMObjectPath & instanceReference,
    ResponseHandler & handler)
{
    // convert a potential fully qualified reference into a local reference
    // (class name and keys only).
    CIMObjectPath localReference = CIMObjectPath(
        String(),
        String(),
        instanceReference.getClassName(),
        instanceReference.getKeyBindings());

    // begin processing the request
    handler.processing();

    cout <<"TimingProvider::deleteInstance" << endl;
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

    cout <<"TimingProvider::deleteInstance" << endl;
    // complete processing the request
    handler.complete();
}

PEGASUS_NAMESPACE_END
