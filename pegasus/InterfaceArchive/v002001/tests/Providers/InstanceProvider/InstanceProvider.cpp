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
/* NOCHKSRC */

#include "InstanceProvider.h"

PEGASUS_NAMESPACE_BEGIN

InstanceProvider::InstanceProvider(void)
{
}

InstanceProvider::~InstanceProvider(void)
{
}

void InstanceProvider::initialize(CIMOMHandle & cimom)
{
	// create default instances
	CIMInstance instance1("Sample_InstanceProviderClass");
        instance1.setPath(CIMObjectPath("Sample_InstanceProviderClass.Identifier=1"));

	instance1.addProperty(CIMProperty("Identifier", Uint8(1)));   // key
	instance1.addProperty(CIMProperty("Message", String("Hello World")));

	_instances.append(instance1);

	CIMInstance instance2("Sample_InstanceProviderClass");
        instance2.setPath(CIMObjectPath("Sample_InstanceProviderClass.Identifier=2"));

	instance2.addProperty(CIMProperty("Identifier", Uint8(2)));   // key
	instance2.addProperty(CIMProperty("Message", String("Yo Planet")));

	_instances.append(instance2);

	CIMInstance instance3("Sample_InstanceProviderClass");
        instance3.setPath(CIMObjectPath("Sample_InstanceProviderClass.Identifier=3"));

	instance3.addProperty(CIMProperty("Identifier", Uint8(3)));   // key
	instance3.addProperty(CIMProperty("Message", String("Hey Earth")));

	_instances.append(instance3);
}

void InstanceProvider::terminate(void)
{
	delete this;
}

void InstanceProvider::getInstance(
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
		CIMNamespaceName(),
		instanceReference.getClassName(),
		instanceReference.getKeyBindings());

	// begin processing the request
	handler.processing();

	// instance index corresponds to reference index
	for(Uint32 i = 0, n = _instances.size(); i < n; i++)
	{
		if(localReference == _instances[i].getPath())
		{
			// deliver requested instance
			handler.deliver(_instances[i]);

			break;
		}
	}

	// complete processing the request
	handler.complete();
}

void InstanceProvider::enumerateInstances(
	const OperationContext & context,
	const CIMObjectPath & classReference,
	const Boolean includeQualifiers,
	const Boolean includeClassOrigin,
	const CIMPropertyList & propertyList,
	InstanceResponseHandler & handler)
{
	// begin processing the request
	handler.processing();

	for(Uint32 i = 0, n = _instances.size(); i < n; i++)
	{
		// deliver instance
		handler.deliver(_instances[i]);
	}

	// complete processing the request
	handler.complete();
}

void InstanceProvider::enumerateInstanceNames(
	const OperationContext & context,
	const CIMObjectPath & classReference,
	ObjectPathResponseHandler & handler)
{
	// begin processing the request
	handler.processing();

	for(Uint32 i = 0, n = _instances.size(); i < n; i++)
	{
		// deliver reference
		handler.deliver(_instances[i].getPath());
	}

	// complete processing the request
	handler.complete();
}

void InstanceProvider::modifyInstance(
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
		CIMNamespaceName(),
		instanceReference.getClassName(),
		instanceReference.getKeyBindings());
	
	// begin processing the request
	handler.processing();

	// instance index corresponds to reference index
	for(Uint32 i = 0, n = _instances.size(); i < n; i++)
	{
		if(localReference == _instances[i].getPath())
		{
                       CIMInstance cimInstance = instanceObject.clone();

                       CIMObjectPath instanceName = CIMObjectPath(
                               String(),
                               CIMNamespaceName(),
                               instanceReference.getClassName(),
                               instanceReference.getKeyBindings());

                        cimInstance.setPath(instanceName);

			// overwrite existing instance
			_instances[i] = instanceObject;
			
			break;
		}
	}
	
	// complete processing the request
	handler.complete();
}

void InstanceProvider::createInstance(
    const OperationContext & context,
    const CIMObjectPath & instanceReference,
    const CIMInstance & instanceObject,
    ObjectPathResponseHandler & handler)
{
    // Validate the class name
    if (!instanceObject.getClassName().equal("Sample_InstanceProviderClass"))
    {
        throw CIMNotSupportedException(
            instanceObject.getClassName().getString());
    }

    // Find the key property
    Uint32 idIndex = instanceObject.findProperty("Identifier");

    if (idIndex == PEG_NOT_FOUND)
    {
        throw CIMInvalidParameterException("Missing key value");
    }

    CIMInstance cimInstance = instanceObject.clone();

    // Create the new instance name
    CIMValue idValue = instanceObject.getProperty(idIndex).getValue();
    Array<CIMKeyBinding> keys;
    keys.append(CIMKeyBinding("Identifier", idValue));
    CIMObjectPath instanceName = CIMObjectPath(
        String(),
        CIMNamespaceName(),
        instanceObject.getClassName(),
        keys);

    cimInstance.setPath(instanceName);
    
    // Determine whether this instance already exists
    for(Uint32 i = 0, n = _instances.size(); i < n; i++)
    {
        if(instanceName == _instances[i].getPath())
        {
            throw CIMObjectAlreadyExistsException(instanceName.toString());
        }
    }

    // begin processing the request
    handler.processing();

    // add the new instance to the array
    _instances.append(cimInstance);

    // deliver the new instance name
    handler.deliver(instanceName);

    // complete processing the request
    handler.complete();
}

void InstanceProvider::deleteInstance(
	const OperationContext & context,
	const CIMObjectPath & instanceReference,
	ResponseHandler & handler)
{
	// convert a potential fully qualified reference into a local reference
	// (class name and keys only).
	CIMObjectPath localReference = CIMObjectPath(
		String(),
		CIMNamespaceName(),
		instanceReference.getClassName(),
		instanceReference.getKeyBindings());
	
	// begin processing the request
	handler.processing();

	// instance index corresponds to reference index
	for(Uint32 i = 0, n = _instances.size(); i < n; i++)
	{
		if(localReference == _instances[i].getPath())
		{
			// remove instance from the array
			_instances.remove(i);

			break;
		}
	}
	
	// complete processing the request
	handler.complete();
}

PEGASUS_NAMESPACE_END

