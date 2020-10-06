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
#include "CIMOMSampleProvider.h"
#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/String.h>
#include <Pegasus/Common/Constants.h>
#include <Pegasus/Common/CIMDateTime.h>
#include <Pegasus/Common/OperationContextInternal.h>
#include <Pegasus/Common/InternalException.h>

PEGASUS_NAMESPACE_BEGIN
PEGASUS_USING_STD;

CIMOMSampleProvider::CIMOMSampleProvider(void)
{
}

CIMOMSampleProvider::~CIMOMSampleProvider(void)
{
}

void CIMOMSampleProvider::initialize(CIMOMHandle & cimom)
{
    cout <<" CIMOMSampleProvider::initialize" << endl;

    // create some instances

    CIMInstance instance1("CIMOMSample");
    CIMObjectPath reference1("CIMOMSample.Id=187");

    instance1.addProperty(CIMProperty("Id", Uint16(100)));

    _instances.append(instance1);
    _instanceNames.append(reference1);

}

void CIMOMSampleProvider::terminate(void)
{
}

void CIMOMSampleProvider::getInstance(
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
    //cout << "className(Method: getInstance)" <<className;

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

void CIMOMSampleProvider::enumerateInstances(
    const OperationContext & context,
    const CIMObjectPath & classReference,
    const Boolean includeQualifiers,
    const Boolean includeClassOrigin,
    const CIMPropertyList & propertyList,
    InstanceResponseHandler & handler)
{
    OperationContext localContext;
    cout << "CIMOMSampleProvider::enumerateInstances" << endl;
    // announce operation processing.
    handler.processing();

    // OperationContext Identity related statements. What will happen
    // if we change the User identity at this point ?
    //
    // Update from TechCall: This seems to be a problem which is a major
    // security issue at this point of time in Pegasus and also a bugzilla
    // record exists for this problem. Not changing as of now.

    String user("subodh");
    cout << " User (Initialized here statically) = " << user;

    IdentityContainer container = context.get(IdentityContainer::NAME);
    cout << " User in context passed to the method (container.getUserName) = "
         << container.getUserName()<< endl;

    // Changing the UserIdentity in OperationContext
    localContext.insert(IdentityContainer(user));
    container = localContext.get(IdentityContainer::NAME);

    cout << " User in localContext (container.getUserName) = "
         << container.getUserName()<< endl;

    // begin processing the request
    handler.processing();

    if (String::equal(user, container.getUserName()))
    {
        for(Uint32 i = 0, n = _instances.size(); i < n; i++)
        // deliver instance
            handler.deliver(_instances[i]);
    }
    else
    {
        cout << "User should be " << user << "but its "
             << container.getUserName() << endl;
        throw(CIM_ERR_INVALID_PARAMETER);
        }
    // complete processing the request
    handler.complete();
}

void CIMOMSampleProvider::enumerateInstanceNames(
    const OperationContext & context,
    const CIMObjectPath & classReference,
    ObjectPathResponseHandler & handler)
{
    cout << "CIMOMSampleProvider::enumerateInstanceNames" << endl;
    // begin processing the request

    CIMName clName = classReference.getClassName();
    handler.processing();

    for(Uint32 i = 0, n = _instances.size(); i < n; i++)
        // deliver reference
        handler.deliver(_instanceNames[i]);

    // complete processing the request
    handler.complete();
}

void CIMOMSampleProvider::modifyInstance(
    const OperationContext & context,
    const CIMObjectPath & instanceReference,
    const CIMInstance & instanceObject,
    const Boolean includeQualifiers,
    const CIMPropertyList & propertyList,
    ResponseHandler & handler)
{
    cout << "CIMOMSampleProvider::modifyInstance" << endl;
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

void CIMOMSampleProvider::createInstance(
    const OperationContext & context,
    const CIMObjectPath & instanceReference,
    const CIMInstance & instanceObject,
    ObjectPathResponseHandler & handler)
{
    cout << "CIMOMSampleProvider::createInstance" << endl;
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

void CIMOMSampleProvider::deleteInstance(
    const OperationContext & context,
    const CIMObjectPath & instanceReference,
    ResponseHandler & handler)
{
    cout << "CIMOMSampleProvider::deleteInstance" << endl;
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
