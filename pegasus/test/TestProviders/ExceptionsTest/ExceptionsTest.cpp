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
// Author: <Subodh Soni> (<ssubodh@in.ibm.com>)
//
// Modified By:
//
//%/////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <iostream>
#include "ExceptionsTest.h"
#include <Pegasus/Common/InternalException.h>

PEGASUS_NAMESPACE_BEGIN
PEGASUS_USING_STD;
// The following macro is enabled/ disabled by defining in the Makefile.

#ifdef TEST_EXCEPTIONS
#define PEGASUS_EXCEPTION       PEGASUS_CIM_EXCEPTION(CIM_ERR_FAILED,   \
                                        "ExceptionsTest Exception")
#define CIM_EXCEPTION           CIMException(CIM_ERR_FAILED, "Test Provider")
#else
#define PEGASUS_EXCEPTION
#define CIM_EXCEPTION
#endif

// Update the value of the following macro for enabling/ disabling exceptions.
//
#define EXCEPTION_TYPE      1
#define CheckExceptionType(X)   {                   \
                cout <<" Exception Type  " << X << endl;\
                if (X == 1) throw PEGASUS_EXCEPTION;    \
                if (X == 2) throw CIM_EXCEPTION;    \
                }

ExceptionsTest::ExceptionsTest(void)
{
}

ExceptionsTest::~ExceptionsTest(void)
{
}

void ExceptionsTest::initialize(CIMOMHandle & cimom)
{

    cout <<" ExceptionsTest::initialize" << endl;

    CheckExceptionType(EXCEPTION_TYPE);

    // create default instances for FirstClass
    CIMInstance instance1("FirstClass");
    CIMObjectPath reference1("FirstClass.Id=1");

    instance1.addProperty(CIMProperty("Id", Uint8(1)));   // key
    instance1.addProperty(CIMProperty("Message", String("Subodh.....")));

    _instances.append(instance1);
    _instanceNames.append(reference1);

    CheckExceptionType(EXCEPTION_TYPE);

    CIMInstance instance2("FirstClass");
    CIMObjectPath reference2("FirstClass.Id=2");

    instance2.addProperty(CIMProperty("Id", Uint8(2)));   // key
    instance2.addProperty(CIMProperty("Message", String("Soni....")));

    _instances.append(instance2);
    _instanceNames.append(reference2);

    CheckExceptionType(EXCEPTION_TYPE);

    // create default instances for SecondClass
    CIMInstance instance_1("SecondClass");
    CIMObjectPath reference_1("SecondClass.Srno=11");

    instance_1.addProperty(CIMProperty("Srno", Uint8(1)));   // key
    instance_1.addProperty(
        CIMProperty("Name", String("Class Two Instance 1.....")));

    _instances_second.append(instance_1);
    _instanceNames_second.append(reference_1);

    CheckExceptionType(EXCEPTION_TYPE);

    CIMInstance instance_2("SecondClass");
    CIMObjectPath reference_2("SecondClass.Srno=2");

    instance_2.addProperty(CIMProperty("Srno", Uint8(2)));   // key
    instance_2.addProperty(
        CIMProperty("Name", String("Class Two Instance 2....")));

    _instances_second.append(instance_2);
    _instanceNames_second.append(reference_2);

    CheckExceptionType(EXCEPTION_TYPE);
}

void ExceptionsTest::terminate(void)
{
}

void ExceptionsTest::getInstance(
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

    cout <<"Calling Exception ExceptionsTest::getInstance 1" << endl;
    CheckExceptionType(EXCEPTION_TYPE);

    if (className == CIMName("FirstClass"))
    {
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
    }
    // complete processing the request
    handler.complete();
    cout <<"Calling Exception ExceptionsTest::getInstance 2" << endl;
    CheckExceptionType(EXCEPTION_TYPE);
}

void ExceptionsTest::enumerateInstances(
    const OperationContext & context,
    const CIMObjectPath & classReference,
    const Boolean includeQualifiers,
    const Boolean includeClassOrigin,
    const CIMPropertyList & propertyList,
    InstanceResponseHandler & handler)
{
    cout <<"Calling Exception ExceptionsTest::enumerateInstances 1" << endl;
    CheckExceptionType(EXCEPTION_TYPE);

    CIMName clName = classReference.getClassName();
    String className = clName.getString();

    if (String::equalNoCase(className, "FirstClass"))
    {
        // announce operation processing.
        handler.processing();
        for(Uint32 i = 0, n = _instances.size(); i < n; i++)
            // deliver instance
            handler.deliver(_instances[i]);
    }
    if (String::equalNoCase(className, "SecondClass"))
    {
        handler.processing();
        for(Uint32 i = 0, n = _instances_second.size(); i < n; i++)
            // deliver instance
            handler.deliver(_instances_second[i]);
    }
    if (String::equalNoCase(className, "ThirdClass"))
    {
        handler.processing();
        for(Uint32 i = 0, n = _instances_third.size(); i < n; i++)
            // deliver instance
            handler.deliver(_instances_third[i]);
    }
    cout <<"Calling Exception ExceptionsTest::enumerateInstances 2" << endl;
    CheckExceptionType(EXCEPTION_TYPE);

    // complete processing the request
    handler.complete();
}

void ExceptionsTest::enumerateInstanceNames(
    const OperationContext & context,
    const CIMObjectPath & classReference,
    ObjectPathResponseHandler & handler)
{
    // begin processing the request
    handler.processing();

    cout <<"Calling Exception ExceptionsTest::enumerateInstanceNames 1" << endl;
    CheckExceptionType(EXCEPTION_TYPE);
    CIMName clName = classReference.getClassName();
    String className = clName.getString();

    if (className == "FirstClass")
    {
        for(Uint32 i = 0, n = _instances.size(); i < n; i++)
            // deliver reference
            handler.deliver(_instanceNames[i]);
    }
    else if (className == "SecondClass")
    {
        for(Uint32 i = 0, n = _instances_second.size(); i < n; i++)
            // deliver reference
            handler.deliver(_instanceNames_second[i]);
    }
    else if (className == "ThirdClass")
    {
        for(Uint32 i = 0, n = _instances_third.size(); i < n; i++)
            // deliver reference
            handler.deliver(_instanceNames_third[i]);

    }
    // complete processing the request
    cout <<"Calling Exception ExceptionsTest::enumerateInstanceNames 2" << endl;
    CheckExceptionType(EXCEPTION_TYPE);
    handler.complete();
}

void ExceptionsTest::modifyInstance(
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

    cout <<"Calling Exception ExceptionsTest::modifyInstance 1" << endl;
    CheckExceptionType(EXCEPTION_TYPE);
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
    cout <<"Calling Exception ExceptionsTest::modifyInstance 2" << endl;
    CheckExceptionType(EXCEPTION_TYPE);
}

void ExceptionsTest::createInstance(
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

    cout <<"Calling Exception ExceptionsTest::createInstance 1" << endl;
    CheckExceptionType(EXCEPTION_TYPE);
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

    cout <<"Calling Exception ExceptionsTest::createInstance 2" << endl;
    CheckExceptionType(EXCEPTION_TYPE);
    // deliver the new instance
    handler.deliver(_instanceNames[_instanceNames.size() - 1]);

    // complete processing the request
    handler.complete();
}

void ExceptionsTest::deleteInstance(
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

    cout <<"Calling Exception ExceptionsTest::deleteInstance 1" << endl;
    CheckExceptionType(EXCEPTION_TYPE);
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

    cout <<"Calling Exception ExceptionsTest::deleteInstance 2" << endl;
    CheckExceptionType(EXCEPTION_TYPE);
    // complete processing the request
    handler.complete();
}

PEGASUS_NAMESPACE_END
