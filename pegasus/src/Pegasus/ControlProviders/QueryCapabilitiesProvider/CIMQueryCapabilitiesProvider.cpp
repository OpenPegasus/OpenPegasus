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
// PEP 193
//
// Bug#2491
//
//%/////////////////////////////////////////////////////////////////////////////

#include <Pegasus/Common/PegasusVersion.h>

#include "CIMQueryCapabilitiesProvider.h"
#include <Pegasus/Common/Tracer.h>

PEGASUS_USING_STD;
PEGASUS_NAMESPACE_BEGIN

// To add more capabilities, change the below static variables
const Uint16 CIMQueryCapabilitiesProvider::FeatureSet[] = {2,8};
const int CIMQueryCapabilitiesProvider::NUM_QUERY_CAPABILITIES = 2;

CIMQueryCapabilitiesProvider::CIMQueryCapabilitiesProvider()
{
  PEG_METHOD_ENTER(TRC_CONTROLPROVIDER,
           "CIMQueryCapabilitiesProvider::CIMQueryCapabilitiesProvider");
  PEG_METHOD_EXIT();
}

CIMQueryCapabilitiesProvider::~CIMQueryCapabilitiesProvider()
{
  PEG_METHOD_ENTER(TRC_CONTROLPROVIDER,
           "CIMQueryCapabilitiesProvider::~CIMQueryCapabilitiesProvider");
  PEG_METHOD_EXIT();
}

void CIMQueryCapabilitiesProvider::getInstance(
        const OperationContext & context,
        const CIMObjectPath & instanceReference,
        const Boolean includeQualifiers,
        const Boolean includeClassOrigin,
        const CIMPropertyList & propertyList,
        InstanceResponseHandler & handler)
{
    PEG_METHOD_ENTER(TRC_CONTROLPROVIDER,
             "CIMQueryCapabilitiesProvider::getInstance");

    Array<CIMKeyBinding> keys = instanceReference.getKeyBindings();

    //-- make sure we're the right instance
    CIMName keyName;
    String keyValue;

    if (keys.size() != NUM_KEY_PROPERTIES)
        throw CIMInvalidParameterException("Wrong number of keys");

    keyName = keys[0].getName();
    keyValue = keys[0].getValue();

    if(keyName.getString() != String(PROPERTY_NAME_INSTANCEID) )
        throw CIMInvalidParameterException("Incorrect Key");

    if(keyValue != String(INSTANCEID_VALUE))
        throw CIMObjectNotFoundException(keyValue);

    // Verify that the className = CIM_QueryCapabilities
    if (!instanceReference.getClassName().equal(
             PEGASUS_CLASSNAME_CIMQUERYCAPABILITIES))
    {
       PEG_METHOD_EXIT();
       throw CIMNotSupportedException(MessageLoaderParms(
           "ControlProviders.CIMQueryCapabilitiesProvider.NOT_SUPPORTED",
           "$0 is not supported by the CIM Query Capabilities Provider.",
           instanceReference.getClassName().getString()));
    }


    // begin processing the request
    handler.processing();

    if(instanceReference.getClassName() ==
         PEGASUS_CLASSNAME_CIMQUERYCAPABILITIES)
    {
        // deliver requested instance
        handler.deliver(buildInstance(instanceReference));
    }

    // complete processing the request
    handler.complete();
    PEG_METHOD_EXIT();
}

void CIMQueryCapabilitiesProvider::enumerateInstances(
        const OperationContext & context,
        const CIMObjectPath & classReference,
        const Boolean includeQualifiers,
        const Boolean includeClassOrigin,
        const CIMPropertyList & propertyList,
        InstanceResponseHandler & handler)
{

    PEG_METHOD_ENTER(TRC_CONTROLPROVIDER,
                     "CIMQueryCapabilitiesProvider::enumerateInstances");

    // begin processing the request
    handler.processing();

    // deliver instance
    handler.deliver(buildInstance(classReference));

    // complete processing the request
    handler.complete();
    PEG_METHOD_EXIT();
}

void CIMQueryCapabilitiesProvider::enumerateInstanceNames(
    const OperationContext & context,
    const CIMObjectPath & classReference,
    ObjectPathResponseHandler & handler)
{
    PEG_METHOD_ENTER(TRC_CONTROLPROVIDER,
                     "CIMQueryCapabilitiesProvider::enumerateInstanceNames");

    // begin processing the request
    handler.processing();

    CIMInstance inst = buildInstance(classReference);


    // deliver reference
    handler.deliver(inst.getPath());

    // complete processing the request
    handler.complete();
    PEG_METHOD_EXIT();
}

void CIMQueryCapabilitiesProvider::modifyInstance(
        const OperationContext & context,
        const CIMObjectPath & instanceReference,
        const CIMInstance & instanceObject,
        const Boolean includeQualifiers,
        const CIMPropertyList & propertyList,
        ResponseHandler & handler)
{
    throw CIMNotSupportedException(
        "CIMQueryCapabilitiesProvider::modifyInstance");
}

void CIMQueryCapabilitiesProvider::createInstance(
        const OperationContext & context,
        const CIMObjectPath & instanceReference,
        const CIMInstance & instanceObject,
        ObjectPathResponseHandler & handler)
{
    throw CIMNotSupportedException(
        "CIMQueryCapabilitiesProvider::createInstance");
}

void CIMQueryCapabilitiesProvider::deleteInstance(
        const OperationContext & context,
        const CIMObjectPath & instanceReference,
        ResponseHandler & handler)
{
    throw CIMNotSupportedException(
        "CIMQueryCapabilitiesProvider::deleteInstance");
}

CIMInstance CIMQueryCapabilitiesProvider::buildInstance(CIMObjectPath cimRef)
{
   Array<Uint16> features(NUM_QUERY_CAPABILITIES);

   getFeatureSet(features);

   CIMInstance requestedInstance("CIM_QueryCapabilities");

   requestedInstance.addProperty(CIMProperty(PROPERTY_NAME_ELEMENT_NAME,
      CIMValue(String("Query Capabilities for the Server"))));
   requestedInstance.addProperty( CIMProperty(PROPERTY_NAME_DESCRIPTION,
      CIMValue(String("The list of CQL features supported by this Server."))));
   requestedInstance.addProperty(CIMProperty(PROPERTY_NAME_CAPTION,
      CIMValue(String("Query Capabilities"))));
   requestedInstance.addProperty(CIMProperty(CIMName(PROPERTY_NAME_INSTANCEID),
     CIMValue(String(INSTANCEID_VALUE))));
   requestedInstance.addProperty(CIMProperty(CIMName(PROPERTY_NAME_CQLFEATURES),
     CIMValue(features)));

   CIMClass cimclass = _cimom.getClass(
                               OperationContext(),
                               cimRef.getNameSpace(),
                               cimRef.getClassName(),
                               false, true, false,
                               CIMPropertyList());

   CIMObjectPath instanceName = requestedInstance.buildPath(cimclass);

   instanceName.setNameSpace(cimRef.getNameSpace());
   requestedInstance.setPath(instanceName);

   return requestedInstance;
}

PEGASUS_NAMESPACE_END
