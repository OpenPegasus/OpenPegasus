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

#include "DeliveryRetryTestProvider.h"

PEGASUS_NAMESPACE_BEGIN

extern "C" PEGASUS_EXPORT CIMProvider* PegasusCreateProvider(
    const String& providerName)
{
    if(String::equalNoCase(providerName, "DeliveryRetryTestProvider"))
    {
        return(new DeliveryRetryTestProvider());
    }
    return(0);
}

void DeliveryRetryTestProvider::_sendIndication()
{
    CIMInstance indicationInstance(CIMName("DeliveryRetryTestClass"));

    indicationInstance.setPath(_path);

    CIMDateTime currentDateTime = CIMDateTime::getCurrentDateTime();

    indicationInstance.addProperty(
        CIMProperty("IndicationTime", currentDateTime));

    indicationInstance.addProperty(
        CIMProperty("sequenceId", CIMValue(Uint64(_nextIdentifier++))));

    _handler->deliver(indicationInstance);
}

DeliveryRetryTestProvider::DeliveryRetryTestProvider()
{
    _nextIdentifier = 1;
    _enabled = false;
    _path.setNameSpace("test/TestProvider");
    _path.setClassName("DeliveryRetryTestClass");
}

DeliveryRetryTestProvider::~DeliveryRetryTestProvider()
{
}

void DeliveryRetryTestProvider::initialize(CIMOMHandle& cimom)
{
    _cimom = cimom;
}

void DeliveryRetryTestProvider::terminate()
{
    delete this;
}

void DeliveryRetryTestProvider::invokeMethod(
    const OperationContext& context,
    const CIMObjectPath& objectReference,
    const CIMName& methodName,
    const Array<CIMParamValue>& inParameters,
    MethodResultResponseHandler& handler)
{
    Uint32 count = 0;

    handler.processing();

    if (!objectReference.getClassName().equal("DeliveryRetryTestClass"))
    {
        throw CIMNotSupportedException(
            objectReference.getClassName().getString());
    }

    if (methodName.equal("resetIdentifier"))
    {
        _nextIdentifier = 1;
    }
    else if (methodName.equal("generateIndications"))
    {
        inParameters[0].getValue().get(count);

        if (!_enabled)
        {
            handler.deliver(CIMValue(0));
        }
        else
        {
            for (Uint32 i = 0; i < count; ++i)
            {
                _sendIndication();
            }
            handler.deliver(CIMValue(1));
        }
    }
    else
    {
        handler.deliver(CIMValue(0));
    }
    handler.complete();
}

void DeliveryRetryTestProvider::enableIndications (
    IndicationResponseHandler & handler)
{
    _enabled = true;
    _handler = &handler;
}

void DeliveryRetryTestProvider::disableIndications ()
{
    _enabled = false;
}

void DeliveryRetryTestProvider::createSubscription (
    const OperationContext & context,
    const CIMObjectPath & subscriptionName,
    const Array <CIMObjectPath> & classNames,
    const CIMPropertyList & propertyList,
    const Uint16 repeatNotificationPolicy)
{
}

void DeliveryRetryTestProvider::modifySubscription (
    const OperationContext & context,
    const CIMObjectPath & subscriptionName,
    const Array <CIMObjectPath> & classNames,
    const CIMPropertyList & propertyList,
    const Uint16 repeatNotificationPolicy)
{
}


void DeliveryRetryTestProvider::deleteSubscription (
    const OperationContext & context,
    const CIMObjectPath & subscriptionName,
    const Array <CIMObjectPath> & classNames)
{
}

PEGASUS_NAMESPACE_END
