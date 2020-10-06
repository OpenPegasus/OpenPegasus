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
//
// This sample provider is both an Instance and Lifecycle Indication provider.
//
// BE SURE YOU CAREFULLY READ THE FILE README.LIFECYCLEINDICATIONS.HTM IN THIS
// DIRECTORY TO MAKE SURE YOU UNDERSTAND CURRENT USE OF LIFECYCLE INDICATIONS!!
//
// Note: If this provider were expanded to also be a MethodProvider, then you
// may want to consider also generating a lifecycle indication for
// InstMethodCall_for_Sample_LifecycleIndicationProviderClass (a subclass of
// CIM_InstMethodCall). See LifecycleIndicationProviderR.mof.

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/CIMDateTime.h>
#include <Pegasus/Provider/CIMInstanceProvider.h>
#include <Pegasus/Provider/CIMIndicationProvider.h>
#include <Pegasus/General/MofWriter.h>

#include "LifecycleIndicationProvider.h"

#include <iostream>

PEGASUS_USING_PEGASUS;
PEGASUS_USING_STD;

LifecycleIndicationProvider::LifecycleIndicationProvider(void)
{
}

LifecycleIndicationProvider::~LifecycleIndicationProvider(void)
{
}

void LifecycleIndicationProvider::initialize(CIMOMHandle & cimom)
{
//  cout << "LifecycleIndicationProvider::initialize()" << endl;

    // save cimom handle
    _cimom = cimom;
    _indication_handler = 0;
    _lifecycle_indications_enabled = false;
    _numSubscriptions = 0;
    _nextUID = 0;
}

void LifecycleIndicationProvider::terminate(void)
{
//  cout << "LifecycleIndicationProvider::terminate()" << endl;
    delete this;
}

void LifecycleIndicationProvider::getInstance(
    const OperationContext & context,
    const CIMObjectPath & instanceReference,
    const Boolean includeQualifiers,
    const Boolean includeClassOrigin,
    const CIMPropertyList & propertyList,
    InstanceResponseHandler & handler)
{
//  cout << "LifecycleIndicationProvider::getInstance()" << endl;
    handler.processing();

    for(Uint32 i = 0, n = _instances.size(); i < n; i++)
    {
        if(instanceReference == _instances[i].getPath())
        {
//          cout << "delivering " << _instances[i].getPath().toString() << endl;

            handler.deliver(_instances[i]);

            break;
        }
    }
    // It's not shown here, but your implementation of getInstance() might want
    // to generate a lifecycle indication for
    // InstRead_for_Sample_LifecycleIndicationProviderClass, for the instance
    // that is returned. See LifecycleIndicationProviderR.mof.

    handler.complete();
}

void LifecycleIndicationProvider::enumerateInstances(
    const OperationContext & context,
    const CIMObjectPath & ref,
    const Boolean includeQualifiers,
    const Boolean includeClassOrigin,
    const CIMPropertyList & propertyList,
    InstanceResponseHandler & handler)
{
//  cout << "LifecycleIndicationProvider::enumerateInstances()" << endl;
    handler.processing();

    for(Uint32 i = 0, n = _instances.size(); i < n; i++)
    {
//      cout << "delivering " << _instances[i].getPath().toString() << endl;

        handler.deliver(_instances[i]);
    }
    // Do you consider enumerateInstances() to be the "read" of an instance?
    // If so, then you might want to generate a lifecycle indication here for
    // InstRead_for_Sample_LifecycleIndicationProviderClass, for each instance
    // that is returned. See LifecycleIndicationProviderR.mof.

    handler.complete();
}

void LifecycleIndicationProvider::enumerateInstanceNames(
    const OperationContext & context,
    const CIMObjectPath & classReference,
    ObjectPathResponseHandler & handler)
{
//  cout << "LifecycleIndicationProvider::enumerateInstanceNames()" << endl;
    handler.processing();

    for(Uint32 i = 0, n = _instances.size(); i < n; i++)
    {
//      cout << "delivering " << _instances[i].getPath().toString() << endl;

        handler.deliver(_instances[i].getPath());
    }
    // Do you consider enumerateInstanceNames() to be the "read" of an instance?
    // If so, then you might want to generate a lifecycle indication here for
    // InstRead_for_Sample_LifecycleIndicationProviderClass, for each instance
    // whose name is returned. See LifecycleIndicationProviderR.mof.

    handler.complete();
}

void LifecycleIndicationProvider::modifyInstance(
    const OperationContext & context,
    const CIMObjectPath & instanceReference,
    const CIMInstance & instanceObject,
    const Boolean includeQualifiers,
    const CIMPropertyList & propertyList,
    ResponseHandler & handler)
{
    throw CIMException(CIM_ERR_NOT_SUPPORTED);
    // When this is supported, it should generate a lifecycle indication of
    // InstModification_for_Sample_LifecycleIndicationProviderClass for the
    // instance that is modified. Remember that this indication has *two*
    // embedded objects, both "before" (PreviousInstance) and
    // "after" (SourceInstance) snapshots of the instance that is modified.
    // See LifecycleIndicationProviderR.mof.
}

void LifecycleIndicationProvider::createInstance(
    const OperationContext & context,
    const CIMObjectPath & instanceReference,
    const CIMInstance & instanceObject,
    ObjectPathResponseHandler & handler)
{
//  cout << "LifecycleIndicationProvider::createInstance()" << endl;
    // Validate the class name
    if(!instanceObject.getClassName().equal(
           "Sample_LifecycleIndicationProviderClass"))
    {
        throw CIMNotSupportedException(
            instanceObject.getClassName().getString());
    }

    // Find the key property
    Uint32 idIndex = instanceObject.findProperty("uniqueId");

    if(idIndex == PEG_NOT_FOUND)
    {
        throw CIMInvalidParameterException("Missing key value");
    }

    CIMInstance cimInstance = instanceObject.clone();

    // Create the new instance name
    CIMValue idValue = instanceObject.getProperty(idIndex).getValue();
    Array<CIMKeyBinding> keys;
    keys.append(CIMKeyBinding("uniqueId", idValue));

    CIMObjectPath instanceName =
        CIMObjectPath(
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

    // If there is at least one subscription active for the lifecycle indication
    // InstCreation_for_Sample_LifecycleIndicationProviderClass, then generate
    // that indication here, embedding the newly-created instance as
    // the SourceInstance property. See LifecycleIndicationProviderR.mof.
    if (_lifecycle_indications_enabled)
    {
        CIMInstance indicationInstance(
            CIMName(
                "InstCreation_for_Sample_LifecycleIndicationProviderClass"));
        CIMObjectPath path;
        path.setNameSpace("root/SampleProvider");
        path.setClassName(
            "InstCreation_for_Sample_LifecycleIndicationProviderClass");
        indicationInstance.setPath(path);

        char buffer[32];
        sprintf(buffer, "%d", _nextUID++);
        indicationInstance.addProperty
            (CIMProperty ("IndicationIdentifier",String(buffer)));

        CIMDateTime currentDateTime = CIMDateTime::getCurrentDateTime ();
        indicationInstance.addProperty
            (CIMProperty ("IndicationTime", currentDateTime));

        indicationInstance.addProperty
            (CIMProperty ("SourceInstance",CIMObject(cimInstance)));

        _indication_handler->deliver (indicationInstance);

//      cout << "LifecycleIndicationProvider::createInstance() sent "
//                  "InstCreation_for_Sample_LifecycleIndicationProviderClass"
//           << endl;
    }
}

void LifecycleIndicationProvider::deleteInstance(
    const OperationContext & context,
    const CIMObjectPath & instanceReference,
    ResponseHandler & handler)
{
//  cout << "LifecycleIndicationProvider::deleteInstance()" << endl;
    CIMInstance localInstance;
    // convert a potential fully qualified reference into a local reference
    // (class name and keys only).
    CIMObjectPath localReference =
        CIMObjectPath(
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
            localInstance = _instances[i];
            // remove instance from the array
            _instances.remove(i);

            break;
        }
    }

    // complete processing the request
    handler.complete();

    // If there is at least one subscription active for the lifecycle indication
    // InstDeletion_for_Sample_LifecycleIndicationProviderClass, then generate
    // that indication here, embedding the just-deleted instance as the
    // SourceInstance property. See LifecycleIndicationProviderR.mof.
    if (_lifecycle_indications_enabled)
    {
        CIMInstance indicationInstance(
            CIMName(
                "InstDeletion_for_Sample_LifecycleIndicationProviderClass"));
        CIMObjectPath path;
        path.setNameSpace("root/SampleProvider");
        path.setClassName(
            "InstDeletion_for_Sample_LifecycleIndicationProviderClass");
        indicationInstance.setPath(path);

        char buffer[32];
        sprintf(buffer, "%d", _nextUID++);
        indicationInstance.addProperty
            (CIMProperty ("IndicationIdentifier",String(buffer)));

        CIMDateTime currentDateTime = CIMDateTime::getCurrentDateTime ();
        indicationInstance.addProperty
            (CIMProperty ("IndicationTime", currentDateTime));

        // Before we send the Lifecycle Indication for the delete of this
        // instance, change the "lastOp" property value to "deleteInstance".
        Uint32 ix = localInstance.findProperty(CIMName("lastOp"));
        if (ix != PEG_NOT_FOUND)
        {
            localInstance.removeProperty(ix);
            localInstance.addProperty(
                CIMProperty(
                    CIMName("lastOp"),
                    String("deleteInstance")));
        }

        indicationInstance.addProperty
            (CIMProperty ("SourceInstance",CIMObject(localInstance)));

        _indication_handler->deliver (indicationInstance);

//      cout << "LifecycleIndicationProvider::deleteInstance() sent "
//                  "InstDeletion_for_Sample_LifecycleIndicationProviderClass"
//           << endl;
    }
}

void LifecycleIndicationProvider::enableIndications (
    IndicationResponseHandler & handler)
{
    /*
       This method is called when the first subscription is received for one of
       the lifecycle indications that is supported by this provider.
       Note that is the reason we subclass the CIM Lifecycle Indications
       (eg. CIM_InstCreation), so that this provider is only enabled for the
       specific indications associated with the resource it handles,
       not for *any* Lifecycle Indication subscription.
       You may wish to use this technique, or if the proliferation of subclasses
       is a concern, then this provider could interrogate the subscription
       in one of the methods below, to see if the resource handled by this
       provider is included in the subscription.
    **/
//  cout << "LifecycleIndicationProvider::enableIndications()" << endl;
    _lifecycle_indications_enabled = true;
    _indication_handler = &handler;
}

void LifecycleIndicationProvider::disableIndications (void)
{
//  cout << "LifecycleIndicationProvider::disableIndications()" << endl;
    _lifecycle_indications_enabled = false;
    _indication_handler->complete();
    _indication_handler = 0;
}

void LifecycleIndicationProvider::createSubscription (
    const OperationContext & context,
    const CIMObjectPath & subscriptionName,
    const Array <CIMObjectPath> & classNames,
    const CIMPropertyList & propertyList,
    const Uint16 repeatNotificationPolicy)
{
//  cout << "LifecycleIndicationProvider::createSubscription()" << endl;
    _numSubscriptions++;
}

void LifecycleIndicationProvider::modifySubscription (
    const OperationContext & context,
    const CIMObjectPath & subscriptionName,
    const Array <CIMObjectPath> & classNames,
    const CIMPropertyList & propertyList,
    const Uint16 repeatNotificationPolicy)
{
//  cout << "LifecycleIndicationProvider::modifySubscription()" << endl;
}

void LifecycleIndicationProvider::deleteSubscription (
    const OperationContext & context,
    const CIMObjectPath & subscriptionName,
    const Array <CIMObjectPath> & classNames)
{
//  cout << "LifecycleIndicationProvider::deleteSubscription()" << endl;
    _numSubscriptions--;
}

