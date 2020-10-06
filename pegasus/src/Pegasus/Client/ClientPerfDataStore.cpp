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


#include "ClientPerfDataStore.h"
#include <Pegasus/Common/XmlWriter.h>

PEGASUS_USING_STD;

PEGASUS_NAMESPACE_BEGIN

ClientPerfDataStore::ClientPerfDataStore()
{
   _classRegistered = false;
}

void ClientPerfDataStore::reset()
{
    _operationType = CIMOPTYPE_INVOKE_METHOD;
    _serverTimeKnown = false;
    _errorCondition = false;
    _serverTime = 0;
    _networkStartTime = TimeValue();
    _networkEndTime = TimeValue();
    _requestSize = 0;
    _responseSize = 0;
    _messID = "";

}

ClientOpPerformanceData ClientPerfDataStore::createPerfDataStruct()
{
    ClientOpPerformanceData _ClientOpPerfData_obj;
    _ClientOpPerfData_obj.roundTripTime =
        _networkEndTime.toMicroseconds() - _networkStartTime.toMicroseconds();
    _ClientOpPerfData_obj.operationType = _operationType;
    _ClientOpPerfData_obj.requestSize = _requestSize;
    _ClientOpPerfData_obj.responseSize = _responseSize;
    _ClientOpPerfData_obj.serverTimeKnown = _serverTimeKnown;
    if (_serverTimeKnown)
    {
        _ClientOpPerfData_obj.serverTime = _serverTime;
    }
    return _ClientOpPerfData_obj;
}

void ClientPerfDataStore::setServerTime(Uint32 time)
{
    _serverTime = time;
    _serverTimeKnown = true;
}

void ClientPerfDataStore::setResponseSize(Uint32 size)
{
    _responseSize = size;
}

void ClientPerfDataStore::setRequestSize(Uint32 size)
{
    _requestSize = size;
}

void ClientPerfDataStore::setStartNetworkTime()
{
    _networkStartTime = TimeValue::getCurrentTime();
}

void ClientPerfDataStore::setEndNetworkTime(TimeValue time)
{
    _networkEndTime = time;
}

void ClientPerfDataStore::setServerTimeKnown(Boolean bol)
{
    _serverTimeKnown = bol;
}

void ClientPerfDataStore::setMessageID(String messageID)
{
    _messID = messageID;
}

void ClientPerfDataStore::setOperationType(MessageType type)
{
    _operationType = Message::convertMessageTypetoCIMOpType(type);
}


Boolean ClientPerfDataStore::checkMessageIDandType(
    const String& messageID,
    MessageType type)
{
    if (_messID != messageID)
    {
        _errorCondition = true;
        return false;
    }

    if (_operationType != Message::convertMessageTypetoCIMOpType(type))
    {
        _errorCondition = true;
        return false;
    }

    return true;
}

String ClientPerfDataStore::toString() const
{
    Buffer out;
    out << " operation type = " << (Uint32)_operationType << "\r\n";
    out << " network start time = "
        << CIMValue(_networkStartTime.toMilliseconds()).toString()
        << "\r\n";
    out << " network end time = "
        << CIMValue(_networkEndTime.toMilliseconds()).toString()
        << "\r\n";
    out << " number of request bytes = " << _requestSize << "\r\n";
    out << " number of response bytes = " << _responseSize << "\r\n";
    out << "message ID = " << _messID << "\r\n";

    if (_errorCondition)
    {
        out << "the error condition is true " << "\r\n";
    }
    else
    {
        out << "the error condition is false" << "\r\n";
    }

    if (_classRegistered)
    {
        out << "there is a class registered" << "\r\n";
    }
    else
    {
        out << "no class is registered" << "\r\n";
    }

    if (_serverTimeKnown)
    {
        out << "_serverTimeKnown is true" << "\r\n";
        out << "_serverTime = " << _serverTime << "\r\n";
    }
    else
    {
        out << "_serverTimeKnown is false" << "\r\n";
    }

    return (String(out.getData(), out.size()));
}

Boolean ClientPerfDataStore::getStatError() const
{
    return _errorCondition;
}

void ClientPerfDataStore::setClassRegistered(Boolean bol)
{
    _classRegistered = bol;
}

Boolean ClientPerfDataStore::isClassRegistered() const
{
    return _classRegistered;
}

PEGASUS_NAMESPACE_END
