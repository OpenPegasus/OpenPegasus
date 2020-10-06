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

#ifndef Pegasus_StatisticalData_h
#define Pegasus_StatisticalData_h

#include <Pegasus/Common/Config.h>
#include <iostream>
#include <cstring>
#include <Pegasus/Common/Linkage.h>
#include <Pegasus/Common/InternalException.h>
#include <Pegasus/Common/String.h>
#include <Pegasus/Common/CIMProperty.h>
#include <Pegasus/Common/CIMInstance.h>
#include <Pegasus/Common/CIMDateTime.h>
#include <Pegasus/Common/CIMMessage.h>
#include <Pegasus/Common/Mutex.h>
#include <Pegasus/Common/Time.h>
#include <Pegasus/Common/TimeValue.h>

PEGASUS_NAMESPACE_BEGIN

#ifndef PEGASUS_DISABLE_PERFINST

#define STAT_GETSTARTTIME \
Uint64 serverStartTimeMicroseconds = \
    TimeValue::getCurrentTime().toMicroseconds();

#define STAT_SERVERSTART \
request->setServerStartTime(serverStartTimeMicroseconds);

#define STAT_SERVEREND \
response->endServer();\

#define STAT_BYTESSENT \
StatisticalData::current()->addToValue( \
    message.size(), response->getType(), \
    StatisticalData::PEGASUS_STATDATA_BYTES_SENT);

#define STAT_SERVEREND_ERROR \
response->endServer();

/*
    The request size value must be stored (requSize) and passed to the
    StatisticalData object at the end of processing otherwise it will be
    the ONLY vlaue that is passed to the client which reports the current
    state of the object, not the pevious (one command ago) state
*/

#define STAT_BYTESREAD \
StatisticalData::current()->requSize = contentLength;

#else
#define STAT_GETSTARTTIME
#define STAT_SERVERSTART
#define STAT_SERVEREND
#define STAT_SERVEREND_ERROR
#define STAT_BYTESREAD
#define STAT_BYTESSENT
#endif

class PEGASUS_COMMON_LINKAGE StatProviderTimeMeasurement
{
public:
    StatProviderTimeMeasurement(CIMMessage* message)
        : _message(message)
    {
#ifndef PEGASUS_DISABLE_PERFINST
        _startTimeMicroseconds = TimeValue::getCurrentTime().toMicroseconds();
#endif
    }

    ~StatProviderTimeMeasurement()
    {
#ifndef PEGASUS_DISABLE_PERFINST
        _message->setProviderTime(
            TimeValue::getCurrentTime().toMicroseconds() -
                _startTimeMicroseconds);
#endif
    }

private:
    StatProviderTimeMeasurement();
    StatProviderTimeMeasurement(const StatProviderTimeMeasurement&);
    StatProviderTimeMeasurement& operator=(const StatProviderTimeMeasurement&);

    CIMMessage* _message;
    Uint64 _startTimeMicroseconds;
};

class PEGASUS_COMMON_LINKAGE StatisticalData
{
public:
    // Statistical data type is Request Type - 3;
    enum StatRequestType
    {
//      UNKNOWN,
//      OTHER,        // Other type not defined
//      BATCHED,      // NOT used by Pegasus
        GET_CLASS,    // 0
        GET_INSTANCE,
        INDICATION_DELIVERY,
        DELETE_CLASS,
        DELETE_INSTANCE,
        CREATE_CLASS,
        CREATE_INSTANCE,
        MODIFY_CLASS,
        MODIFY_INSTANCE,
        ENUMERATE_CLASSES,
        ENUMERATE_CLASS_NAMES,  // 10
        ENUMERATE_INSTANCES,
        ENUMERATE_INSTANCE_NAMES,
        EXEC_QUERY,
        ASSOCIATORS,
        ASSOCIATOR_NAMES,
        REFERENCES,
        REFERENCE_NAMES,
        GET_PROPERTY,
        SET_PROPERTY,
        GET_QUALIFIER,   // 20
        SET_QUALIFIER,
        DELETE_QUALIFIER,
        ENUMERATE_QUALIFIERS,  // 23
// Types beyond the here are NOT part of the valueMap,Values defined
// in CIM_CIMOMStatisticalData.mof and must be treated as Other in
// the preperation of the instance output.
        INVOKE_METHOD,  // 24
// EXP_PULL_BEGIN
        OPEN_ENUMERATE_INSTANCES, // 25
        OPEN_ENUMERATEINSTANCE_PATHS,
        OPEN_ASSOCIATORS,
        OPEN_ASSOCIATOR_PATHS,
        OPEN_REFERENCES,
        OPEN_REFERENCE_PATHS,
        OPEN_QUERY_INSTANCES,
        PULL_INSTANCES_WITH_PATH,
        PULL_INSTANCE_PATHS,
        CLOSE_ENUMERATION,
// EXP_PULL_END
        NUMBER_OF_TYPES
    };

    enum StatDataType
    {
        PEGASUS_STATDATA_SERVER,
        PEGASUS_STATDATA_PROVIDER,
        PEGASUS_STATDATA_BYTES_SENT,
        PEGASUS_STATDATA_BYTES_READ
    };

    static const Uint32 length;

    /**
        Static function to get address of singleton StatisticalData
        instance. Creates a new object if none exists.

        @return StatisticalData*  Pointer to the object which
                contains the table of accumulated statistics.
     */
    static StatisticalData* current();

    timeval timestamp;

    // Statistics entries where statistics information is aggregated from
    // each operation.
    Sint64 numCalls[NUMBER_OF_TYPES];
    Sint64 cimomTime[NUMBER_OF_TYPES];
    Sint64 providerTime[NUMBER_OF_TYPES];
    Sint64 responseSize[NUMBER_OF_TYPES];
    Sint64 requestSize[NUMBER_OF_TYPES];
    Sint64 requSize;    //temporary storage for requestSize value
    Boolean copyGSD;

    static StatisticalData* table;

    /** Add the value parameter to the current value for the
        messagetype msgType and the StatDataType

        @param value Sint64 type to add
        @param msgType Pegasus message type (may be either request
                       or response
        @param t StatDataType stat type to which value is added
     */
    void addToValue(Sint64 value, MessageType msgType, StatDataType t);

    /** Clear the StatisticalData table entries back to zero
     */
    void clear();

    /**
        Get the name for the statistics type
     */
    String getRequestName(Uint16 i);

    /** Set the copyGSD flag that controls whether statistics are to
        be gathered and displayed into the singleton object
        @param flag
     */
    void setCopyGSD(Boolean flag);

protected:
    Mutex _mutex;

private:
    StatisticalData();
    StatisticalData(const StatisticalData&);        // Prevent copy-construction
    StatisticalData& operator=(const StatisticalData&);
    static String requestName[];
};


PEGASUS_NAMESPACE_END
#endif

