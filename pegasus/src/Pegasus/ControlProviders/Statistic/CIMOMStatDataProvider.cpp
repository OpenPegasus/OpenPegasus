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

#include "CIMOMStatDataProvider.h"
#include <Pegasus/Common/PegasusVersion.h>
#include <Pegasus/Common/Print.h>

PEGASUS_USING_STD;
PEGASUS_NAMESPACE_BEGIN

CIMOMStatDataProvider::CIMOMStatDataProvider()
{
    sData = StatisticalData::current();
}

// Build a single instance key property value.  The value consists of
// the StatisticalData name prepended to the StatisticalData index.
String CIMOMStatDataProvider::buildKey(Uint16 type)
{
    String key;
    char buffer[32];
    sprintf(buffer, "%u", type);
    return String(sData->getRequestName(type) + buffer);
}

CIMObjectPath CIMOMStatDataProvider::buildObjectPath(Uint16 type)
{
        return CIMObjectPath(
            "CIM_CIMOMStatisticalData.InstanceID=\""+ buildKey(type) +"\"");
}

CIMOMStatDataProvider::~CIMOMStatDataProvider()
{
}

void CIMOMStatDataProvider::getInstance(
    const OperationContext & context,
    const CIMObjectPath & instanceReference,
    const Boolean includeQualifiers,
    const Boolean includeClassOrigin,
    const CIMPropertyList & propertyList,
    InstanceResponseHandler & handler)
{
    CIMObjectPath localReference = CIMObjectPath(
        String::EMPTY,
        CIMNamespaceName(),
        instanceReference.getClassName(),
        instanceReference.getKeyBindings());

    // begin processing the request
    handler.processing();

    // find the correct instance and build it
    for (Uint16 i = StatisticalData::GET_CLASS;
           i < StatisticalData::NUMBER_OF_TYPES; i++)
    {
        if (localReference == buildObjectPath(i))
        {
            // deliver requested instance
            handler.deliver(buildInstance(
               (StatisticalData::StatRequestType)i,
               instanceReference));
            break;
        }
    }

    // complete processing the request
    handler.complete();
}

void CIMOMStatDataProvider::enumerateInstances(
    const OperationContext & context,
    const CIMObjectPath & classReference,
    const Boolean includeQualifiers,
    const Boolean includeClassOrigin,
    const CIMPropertyList & propertyList,
    InstanceResponseHandler & handler)
{
    // begin processing the request
    handler.processing();

    // Loop throuh complete StatisticalData table building instances
    // Start at GET_CLASS because lower groups are not part of data
    for (StatisticalData::StatRequestType i = StatisticalData::GET_CLASS;
          i < StatisticalData::NUMBER_OF_TYPES;
          i = StatisticalData::StatRequestType(i + 1))
    {
        // deliver instance
        handler.deliver(buildInstance(i,classReference));
    }

    // complete processing the request
    handler.complete();
}

void CIMOMStatDataProvider::enumerateInstanceNames(
    const OperationContext & context,
    const CIMObjectPath & classReference,
    ObjectPathResponseHandler & handler)
{
    // begin processing the request
    handler.processing();

    // Enumerate over the whole enum set
    for (StatisticalData::StatRequestType i = StatisticalData::GET_CLASS;
          i < StatisticalData::NUMBER_OF_TYPES;
          i = StatisticalData::StatRequestType(i + 1))
    {
        // deliver reference
        handler.deliver(buildObjectPath(i));
    }

    // complete processing the request
    handler.complete();
}

void CIMOMStatDataProvider::modifyInstance(
    const OperationContext & context,
    const CIMObjectPath & instanceReference,
    const CIMInstance & instanceObject,
    const Boolean includeQualifiers,
    const CIMPropertyList & propertyList,
    ResponseHandler & handler)
{
    throw CIMNotSupportedException("StatisticalData::modifyInstance");
}

void CIMOMStatDataProvider::createInstance(
    const OperationContext & context,
    const CIMObjectPath & instanceReference,
    const CIMInstance & instanceObject,
    ObjectPathResponseHandler & handler)
{
    throw CIMNotSupportedException("StatisticalData::createInstance");
}

void CIMOMStatDataProvider::deleteInstance(
    const OperationContext & context,
    const CIMObjectPath & instanceReference,
    ResponseHandler & handler)
{
    throw CIMNotSupportedException("StatisticalData::deleteInstance");
}

CIMInstance CIMOMStatDataProvider::buildInstance(
    StatisticalData::StatRequestType type,
    CIMObjectPath cimRef)
{
    checkObjectManager();

    CIMDateTime cimom_time = CIMDateTime((sData->cimomTime[type]), true);
    CIMDateTime provider_time = CIMDateTime((sData->providerTime[type]), true);

    CIMInstance requestedInstance("CIM_CIMOMStatisticalData");

    requestedInstance.addProperty(CIMProperty("InstanceID",
        CIMValue(buildKey(type))));

    // If StatRequestType > the types defined in the mof class
    // set OperationType to other "Other" add the OtherOperationType property.
    if (type > StatisticalData::ENUMERATE_QUALIFIERS)
    {
        requestedInstance.addProperty(CIMProperty("OperationType",
            CIMValue(Uint16(1))));
        requestedInstance.addProperty(CIMProperty("OtherOperationType",
            CIMValue(sData->getRequestName(type))));
        // Test Diagnostic Output
        // cout << "Set other property " << sData->getRequestName(type) << endl;
    }
    else
    {
        Uint16 mof_type = getValueMapType(type);
        requestedInstance.addProperty(CIMProperty("OperationType",
            CIMValue(mof_type)));
    }

    requestedInstance.addProperty(CIMProperty("NumberOfOperations",
        CIMValue((Uint64)sData->numCalls[type])));
    requestedInstance.addProperty(CIMProperty("CimomElapsedTime",
        CIMValue(cimom_time)));
    requestedInstance.addProperty(CIMProperty("ProviderElapsedTime",
        CIMValue(provider_time)));
    requestedInstance.addProperty(CIMProperty("RequestSize",
        CIMValue((Uint64)sData->requestSize[type])));
    requestedInstance.addProperty(CIMProperty("ResponseSize",
        CIMValue((Uint64)sData->responseSize[type])));
    requestedInstance.addProperty( CIMProperty("Description",
        CIMValue(String("CIMOM performance statistics for CIM request "))));
    requestedInstance.addProperty(CIMProperty("Caption",
        CIMValue(String("CIMOM performance statistics for CIM request"))));

    requestedInstance.setPath(buildObjectPath(type));

    return requestedInstance;
}

void CIMOMStatDataProvider::checkObjectManager()
{

    if (!sData->copyGSD)
    {
       //set all values to 0 if CIM_ObjectManager is False

        for (Uint16 i=0; i<StatisticalData::length; i++)
        {
            sData->numCalls[i] = 0;
            sData->cimomTime[i] = 0;
            sData->providerTime[i] = 0;
            sData->responseSize[i] = 0;
            sData->requestSize[i] = 0;
        }
    }
}


// This conversion makes make the OperationType attribute of the
// CIM_CIMOMStatisticalData instances agree with DMTF spec.
// The CIM_StatisticalData class specifys type 0 as "unknown"
// and 1 as "other"
//
// The internal message types are subject to change so the symb0lic
// enumerated values are used within a select statement rather than
// a one dimensional array that is simply indexed to determine the
// output type.
// Returns the ValueMap type corresponding to each StatisticalData
// type.

Uint16 CIMOMStatDataProvider::getValueMapType(
   StatisticalData::StatRequestType type)
{
    Uint16 outType;

    switch (type)
    {
        case StatisticalData::GET_CLASS:
            outType= 3 ;
            break;

        case StatisticalData::GET_INSTANCE:
            outType= 4;
            break;

        case StatisticalData::INDICATION_DELIVERY:
            outType= 26;
            break;

        case StatisticalData::DELETE_CLASS:
            outType= 5;
            break;

        case StatisticalData::DELETE_INSTANCE:
            outType= 6;
            break;

        case StatisticalData::CREATE_CLASS:
            outType= 7;
            break;

        case StatisticalData::CREATE_INSTANCE:
            outType= 8;
            break;

        case StatisticalData::MODIFY_CLASS:
            outType= 9;
            break;

        case StatisticalData::MODIFY_INSTANCE:
            outType= 10;
            break;

        case StatisticalData::ENUMERATE_CLASSES:
            outType= 11;
            break;

        case StatisticalData::ENUMERATE_CLASS_NAMES:
            outType= 12;
            break;

        case StatisticalData::ENUMERATE_INSTANCES:
            outType= 13;
            break;

        case StatisticalData::ENUMERATE_INSTANCE_NAMES:
            outType= 14;
            break;

        case StatisticalData::EXEC_QUERY:
            outType= 15;
            break;

        case StatisticalData::ASSOCIATORS:
            outType= 16;
            break;

        case StatisticalData::ASSOCIATOR_NAMES:
            outType= 17;
            break;

        case StatisticalData::REFERENCES:
            outType= 18;
            break;

        case StatisticalData::REFERENCE_NAMES:
            outType= 19;
            break;

        case StatisticalData::GET_PROPERTY:
            outType= 20;
            break;

        case StatisticalData::SET_PROPERTY:
            outType= 21;
            break;

        case StatisticalData::GET_QUALIFIER:
            outType= 22;
            break;

        case StatisticalData::SET_QUALIFIER:
            outType= 23;
            break;

        case StatisticalData::DELETE_QUALIFIER:
            outType= 24;
            break;

        case StatisticalData::ENUMERATE_QUALIFIERS:
            outType= 25;
            break;

        // This converts to Other type. (OperationType = 1)
        case StatisticalData::INVOKE_METHOD:
            outType= 1;
            break;

        default:
            // This type is unknown so output "Unknown"
            outType=0;
            break;
    }
    // test diagnostic output
    //cout << "Proivder type mapper " << type << " returns" << outType << endl;

    return outType;
}

PEGASUS_NAMESPACE_END

