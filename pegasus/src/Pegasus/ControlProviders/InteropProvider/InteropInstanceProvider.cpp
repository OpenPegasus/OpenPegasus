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


///////////////////////////////////////////////////////////////////////////////
//  Interop Provider - This provider services those classes from the
//  DMTF Interop schema in an implementation compliant with the SMI-S v1.1
//  Server Profile.
//
//  Please see PG_ServerProfile20.mof in the directory
//  $(PEGASUS_ROOT)/Schemas/Pegasus/InterOp/VER20 for retails regarding the
//  classes supported by this control provider.
//
//  Interop forces all creates to the PEGASUS_NAMESPACENAME_INTEROP
//  namespace. There is a test on each operation that returns
//  the Invalid Class CIMDError
//  This is a control provider and as such uses the Tracer functions
//  for data and function traces.  Since we do not expect high volume
//  use we added a number of traces to help diagnostics.
///////////////////////////////////////////////////////////////////////////////


#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/PegasusVersion.h>
#include <Pegasus/Common/Pegasus_inl.h>

#include <cctype>
#include <iostream>

#include "InteropProvider.h"
#include "InteropProviderUtils.h"
#include "InteropConstants.h"

PEGASUS_USING_STD;
PEGASUS_NAMESPACE_BEGIN

// This Mutex serializes access to the instance change CIM requests. Keeps from
// mixing instance creates, modifications, and deletes. This keeps the provider
// from simultaneously execute creates, modifications, and deletes of instances
// While these operations are largely protected by the locking mechanisms of
// the repository this mutex guarantees that the provider will not
// simultaneously execute the instance change operations.
Mutex changeControlMutex;

/*****************************************************************************
 *
 * Implementation of InstanceProvider createInstance method.
 *
 *****************************************************************************/
void InteropProvider::createInstance(
    const OperationContext & context,
    const CIMObjectPath & instanceReference,
    const CIMInstance & clientInstance,
    ObjectPathResponseHandler & handler)
{
    PEG_METHOD_ENTER(TRC_CONTROLPROVIDER, "InteropProvider::createInstance()");
    initProvider();
    AutoMutex autoMut(changeControlMutex);

    PEG_TRACE((TRC_CONTROLPROVIDER, Tracer::LEVEL4,
        "%s createInstance. InstanceReference= %s",
        thisProvider,
        (const char *) instanceReference.toString().getCString()));

    // test for legal namespace for this provider. Exception if not
    //namespaceSupported(instanceReference);
    // NOTE: Above is commented out because the routing tables will always
    // do the right thing and that's the only way requests get here.

    handler.processing();

    const CIMName & instClassName = instanceReference.getClassName();
    TARGET_CLASS classEnum;

    classEnum = translateClassInput(instClassName);

    // The object path returned to the client invoking the operation
    CIMObjectPath newInstanceReference;

    //
    // The InteropProvider accepts CreateInstance requests for the
    // CIM_Namespace class, but it treats the instance as a PG_Namespace
    // instance and this operation will return an object path referencing
    // the resulting PG_Namespace object.
    //
    if (classEnum == PG_NAMESPACE || classEnum == CIM_NAMESPACE)
    {
        newInstanceReference = createNamespace(clientInstance);
    }
    else if(classEnum == PG_PROVIDERPROFILECAPABILITIES)
    {
        newInstanceReference = createProviderProfileCapabilityInstance(
            clientInstance,
            context);
    }
    else   // Invalid class for the create functions.
    {
        PEG_METHOD_EXIT();
        MessageLoaderParms mparms(
            "ControlProviders.InteropProvider.CREATE_INSTANCE_NOT_ALLOWED",
            "Create instance operation not allowed by Interop Provider for "
                "class $0.",
            instClassName.getString());
        throw CIMNotSupportedException(mparms);
    }

    // Deliver object path to client for accessing the new instance.
    handler.deliver(newInstanceReference);

    // complete processing the request
    handler.complete();

    PEG_METHOD_EXIT();
}

/*****************************************************************************
 *
 * Implementation of InstanceProvider deleteInstance method.
 *
 *****************************************************************************/
void InteropProvider::deleteInstance(
    const OperationContext & context,
    const CIMObjectPath & instanceName,
    ResponseHandler & handler)
{
    PEG_METHOD_ENTER(TRC_CONTROLPROVIDER, "InteropProvider::deleteInstance");

    initProvider();

    const CIMName instClassName = instanceName.getClassName();

    AutoMutex autoMut(changeControlMutex);

    PEG_TRACE((TRC_CONTROLPROVIDER, Tracer::LEVEL4,
        "%s deleteInstance. instanceName= %s",
        thisProvider,
        (const char *) instanceName.toString().getCString()));

    if(instClassName == PEGASUS_CLASSNAME_PGNAMESPACE)
    {
        handler.processing();
        deleteNamespace(instanceName);
        handler.complete();

        PEG_METHOD_EXIT();
        return;
    }
    else if(instClassName == PEGASUS_CLASSNAME_PG_PROVIDERPROFILECAPABILITIES)
    {
        handler.processing();
        deleteProviderProfileCapabilityInstance(instanceName, context);
        handler.complete();
        PEG_METHOD_EXIT();
        return;
    }

    MessageLoaderParms mparms(
        "ControlProviders.InteropProvider.DELETE_INSTANCE_NOT_ALLOWED",
        "Delete instance operation not allowed by Interop Provider for "
            "class $0.",
        instClassName.getString());
    throw CIMNotSupportedException(mparms);
}


/*****************************************************************************
 *
 * Implementation of InstanceProvider getInstance method.
 *
 *****************************************************************************/
void InteropProvider::getInstance(
    const OperationContext & context,
    const CIMObjectPath & instanceName,
    const Boolean includeQualifiers,
    const Boolean includeClassOrigin,
    const CIMPropertyList & propertyList,
    InstanceResponseHandler & handler)
{
    PEG_METHOD_ENTER(TRC_CONTROLPROVIDER, "InteropProvider::getInstance");

    initProvider();

    // test for legal namespace for this provider. Exception if not
    //namespaceSupported(instanceName);
    // NOTE: Above is commented out because the routing tables will always
    // do the right thing and that's the only way requests get here.

    handler.processing();

    CIMInstance myInstance = localGetInstance(
        context,
        instanceName,
        propertyList);

    handler.deliver(myInstance);

    handler.complete();

    PEG_METHOD_EXIT();
}


/*****************************************************************************
 *
 * Implementation of InstanceProvider enumerateInstances method.
 *
 *****************************************************************************/
void InteropProvider::enumerateInstances(
    const OperationContext & context,
    const CIMObjectPath & ref,
    const Boolean includeQualifiers,
    const Boolean includeClassOrigin,
    const CIMPropertyList& propertyList,
    InstanceResponseHandler & handler)
{
    PEG_METHOD_ENTER(TRC_CONTROLPROVIDER,
        "InteropProvider::enumerateInstances()");

    initProvider();

    // test for legal namespace for this provider. Exception if not
    //namespaceSupported(ref);
    // NOTE: Above is commented out because the routing tables will always
    // do the right thing and that's the only way requests get here.
    handler.processing();

    // Call the internal enumerateInstances to generate instances of defined
    // class.  This expects the instances to be returned complete including
    // complete path.
    handler.deliver(localEnumerateInstances(context, ref, propertyList));
    handler.complete();
    PEG_METHOD_EXIT();
}


/*****************************************************************************
 *
 * Implementation of InstanceProvider modifyInstance method.
 *
 *****************************************************************************/
void InteropProvider::modifyInstance(
    const OperationContext & context,
    const CIMObjectPath & instanceReference,
    const CIMInstance& modifiedIns,
    const Boolean includeQualifiers,
    const CIMPropertyList& propertyList,
    ResponseHandler & handler)
{
    PEG_METHOD_ENTER(TRC_CONTROLPROVIDER,
        "InteropProvider::modifyInstance");

    initProvider();

    AutoMutex autoMut(changeControlMutex);

    PEG_TRACE((TRC_CONTROLPROVIDER, Tracer::LEVEL4,
        "%s modifyInstance. instanceReference= %s, includeQualifiers= %s, "
            "PropertyList= %s",
        thisProvider,
        (const char *) (instanceReference.toString().getCString()),
        boolToString(includeQualifiers),
        (const char *)propertyList.toString().getCString()));

    // test for legal namespace for this provider. Exception if not
    //namespaceSupported(instanceReference);
    // NOTE: Above is commented out because the routing tables will always
    // do the right thing and that's the only way requests get here.

    CIMName className =  instanceReference.getClassName();

    // begin processing the request
    handler.processing();

    if (className.equal(PEGASUS_CLASSNAME_PG_OBJECTMANAGER))
    {
        modifyObjectManagerInstance(context, instanceReference,modifiedIns,
            includeQualifiers, propertyList);
    }
    else
    {
        throw CIMNotSupportedException("Delete instance of class " +
          className.getString());
    }

    handler.complete();
    PEG_METHOD_EXIT();
    return;
}

/*****************************************************************************
 *
 * Implementation of InstanceProvider enumerateInstanceNames method
 *
 *****************************************************************************/
void InteropProvider::enumerateInstanceNames(
    const OperationContext & context,
    const CIMObjectPath & classReference,
    ObjectPathResponseHandler & handler)
{
    PEG_METHOD_ENTER(TRC_CONTROLPROVIDER,
        "InteropProvider::enumerateInstanceNames()");

    initProvider();

    PEG_TRACE((TRC_CONTROLPROVIDER, Tracer::LEVEL4,
        "%s enumerateInstanceNames. classReference= %s",
        thisProvider,
        (const char *) classReference.toString().getCString()));

    // test for legal namespace for this provider. Exception if not
    // namespaceSupported(classReference);
    // NOTE: Above is commented out because the routing tables will always
    // do the right thing and that's the only way requests get here.

    // begin processing the request
    handler.processing();

    // Utilize the local enumeration method to retrieve the instances and
    // extract the instance names.
    Array<CIMInstance> instances = localEnumerateInstances(
        context,
        classReference,
        CIMPropertyList());

    for (Uint32 i = 0 ; i < instances.size() ; i++)
    {
        handler.deliver(instances[i].getPath());
    }

    handler.complete();
    PEG_METHOD_EXIT();
}

PEGASUS_NAMESPACE_END
// END_OF_FILE
