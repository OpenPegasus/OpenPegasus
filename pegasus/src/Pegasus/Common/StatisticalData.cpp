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

#include "StatisticalData.h"
#include "Tracer.h"

PEGASUS_NAMESPACE_BEGIN


// The table on the right represents the mapping from the enumerated types
// in the CIM_CIMOMStatisticalDate class ValueMap versus the internal
// message type defined in Message.h. This conversion is performed by
// getOpType() in CIMOMStatDataProvider.cpp.
//

String StatisticalData::requestName[] =
{
                                    // Enumerated     ValueMap Value
                                    // value from     from class
                                    // Pegasus        CIM_StatisticalData
                                    // message type
                                    // -------------- -------------------
//"Unknown",                        //                 0   default
//"Other",                          //                 1   mapped
//"Batched",                        //                 2   not used
  "GetClass",                       //     1           3
  "GetInstance",                    //     2           4
  "IndicationDelivery",             //     3          26
  "DeleteClass",                    //     4           5
  "DeleteInstance",                 //     5           6
  "CreateClass",                    //     6           7
  "CreateInstance",                 //     7           8
  "ModifyClass",                    //     8           9
  "ModifyInstance",                 //     9          10
  "EnumerateClasses",               //    10          11
  "EnumerateClassNames",            //    12          12
  "EnumerateInstances",             //    13          13
  "EnumerateInstanceNames",         //    14          14
  "ExecQuery",                      //    15          15
  "Associators",                    //    16          16
  "AssociatorNames",                //    17          17
  "References",                     //    18          18
  "ReferenceNames",                 //    19          19
  "GetProperty",                    //    20          20
  "SetProperty",                    //    21          21
  "GetQualifier",                   //    22          22
  "SetQualifier",                   //    23          23
  "DeleteQualifier",                //    24          24
  "EnumerateQualifiers",            //    25          25
// Entries below this point are not part of the CIM Class and are treated
// as OtherOperationTypes in the CIM_CIMOMStatisticalData instance.
  "InvokeMethod",                    //    26          Not Present index = 26
//EXP_PULL_BEGIN
//// These are not defined in CIM_StatisticalData class and are
///  represented by the Other groping with supplementary property
  "OpenEnumerateInstances",         //    71          27
  "OpenEnumerateInstancePaths",     //    72
  "OpenAssociators",                //    73
  "OpenAssociatorPaths",           //     74
  "OpenReferences",                 //    75
  "OpenReferenceNames",             //    76
  "OpenQueryInstances",             //    77
  "PullInstancesWithPath",          //    78
  "PullInstancePaths",              //    79
  "CloseEnumeration" ,              //    80
//EXP_PULL_END
};

const Uint32 StatisticalData::length = NUMBER_OF_TYPES;

// Pointer to StatisticalData table if it exists.
StatisticalData* StatisticalData::table = NULL;

// If first call, create the statistical data array
StatisticalData* StatisticalData::current()
{
    if (table == NULL)
    {
        table = new StatisticalData();
    }
    return table;
}

// Constructor clears the statisticalData Array and sets the gatherint
// flag to zero
StatisticalData::StatisticalData()
{
    copyGSD = 0;
    clear();
}

void StatisticalData::clear()
{
    AutoMutex autoMut(_mutex);
    for (unsigned int i=0; i<StatisticalData::length; i++)
    {
        numCalls[i] = 0;
        cimomTime[i] = 0;
        providerTime[i] = 0;
        responseSize[i] = 0;
        requestSize[i] = 0;
    }
}

String StatisticalData::getRequestName(Uint16 i)
{
    return requestName[i];
}

void StatisticalData::addToValue(Sint64 value,
    MessageType msgType,
    StatDataType t)
{
    // Map MessageType to statistic type. Requires multiple tests because
    // mapping request and responses to the request types.
    Uint16 type;

    if ((msgType) >= CIM_OPEN_ENUMERATE_INSTANCES_REQUEST_MESSAGE)
    {
        type = msgType - CIM_DELETE_QUALIFIER_RESPONSE_MESSAGE;
    }
    else if (msgType >= CIM_GET_CLASS_RESPONSE_MESSAGE)
    {
        type = msgType - CIM_GET_CLASS_RESPONSE_MESSAGE;
    }
    else
    {
        type = msgType - 1;
    }

    // Test if valid statistic type
    if (type >= NUMBER_OF_TYPES)
    {
         PEG_TRACE((TRC_DISCARDED_DATA, Tracer::LEVEL2,
             "StatData: Statistical Data Discarded.  "
                 "Invalid Request Type =  %u", type));
         return;
    }
    //// Diagnostic to confirm message type conversion. Normally commented
    //// out
//  PEG_TRACE((TRC_STATISTICAL_DATA, Tracer::LEVEL4,
//   "StatisticalData::addToValue msgType %s %u. stat type %u %s",
//             MessageTypeToString(msgType),
//             msgType, type, (const char*)requestName[type].getCString() ));

    if (copyGSD)
    {
        AutoMutex autoMut(_mutex);
        switch (t)
        {
            case PEGASUS_STATDATA_SERVER:
                numCalls[type] += 1;
                cimomTime[type] += value;
                PEG_TRACE((TRC_STATISTICAL_DATA, Tracer::LEVEL4,
                    "StatData: SERVER: %s(%d): count = %"
                        PEGASUS_64BIT_CONVERSION_WIDTH "d; value = %"
                        PEGASUS_64BIT_CONVERSION_WIDTH "d; total = %"
                        PEGASUS_64BIT_CONVERSION_WIDTH "d",
                    (const char *)requestName[type].getCString(), type,
                    numCalls[type], value, cimomTime[type]));
                break;
            case PEGASUS_STATDATA_PROVIDER:
                providerTime[type] += value;
                PEG_TRACE((TRC_STATISTICAL_DATA, Tracer::LEVEL4,
                    "StatData: PROVIDER: %s(%d): count = %"
                        PEGASUS_64BIT_CONVERSION_WIDTH "d; value = %"
                        PEGASUS_64BIT_CONVERSION_WIDTH "d; total = %"
                        PEGASUS_64BIT_CONVERSION_WIDTH "d",
                    (const char *)requestName[type].getCString(), type,
                    numCalls[type], value, providerTime[type]));
                break;
        case PEGASUS_STATDATA_BYTES_SENT:
                responseSize[type] += value;
                PEG_TRACE((TRC_STATISTICAL_DATA, Tracer::LEVEL4,
                    "StatData: BYTES_SENT: %s(%d): count = %"
                        PEGASUS_64BIT_CONVERSION_WIDTH "d; value = %"
                        PEGASUS_64BIT_CONVERSION_WIDTH "d; total = %"
                        PEGASUS_64BIT_CONVERSION_WIDTH "d",
                    (const char *)requestName[type].getCString(), type,
                    numCalls[type], value, responseSize[type]));
                break;
        case PEGASUS_STATDATA_BYTES_READ:
                requestSize[type] += value;
                PEG_TRACE((TRC_STATISTICAL_DATA, Tracer::LEVEL4,
                    "StatData: BYTES_READ: %s(%d): count = %"
                        PEGASUS_64BIT_CONVERSION_WIDTH "d; value = %"
                        PEGASUS_64BIT_CONVERSION_WIDTH "d; total = %"
                        PEGASUS_64BIT_CONVERSION_WIDTH "d",
                    (const char *)requestName[type].getCString(), type,
                    numCalls[type], value, requestSize[type]));
                break;
        }
    }
}

void StatisticalData::setCopyGSD(Boolean flag)
{
    copyGSD = flag;
}

PEGASUS_NAMESPACE_END
