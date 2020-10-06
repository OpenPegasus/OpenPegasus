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

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/CIMDateTime.h>
#include "ProviderLifecycleIndicationTestProvider.h"

PEGASUS_USING_STD;
PEGASUS_USING_PEGASUS;

static Uint32 _numSubscriptions = 0;

extern "C" PEGASUS_EXPORT CIMProvider* PegasusCreateProvider(
    const String& providerName)
{
    if (String::equalNoCase(
        providerName,
        "TestProviderLifecycleIndicationProvider"))
    {
        return new ProviderLifecycleIndicationTestProvider();
    }

    return 0;
}

ProviderLifecycleIndicationTestProvider::
    ProviderLifecycleIndicationTestProvider() throw()
{
}

ProviderLifecycleIndicationTestProvider::
    ~ProviderLifecycleIndicationTestProvider() throw()
{
}

void ProviderLifecycleIndicationTestProvider::initialize(CIMOMHandle& cimom)
{
}

void ProviderLifecycleIndicationTestProvider::terminate()
{
    delete this;
}

void ProviderLifecycleIndicationTestProvider::enableIndications(
    IndicationResponseHandler& handler)
{
}

void ProviderLifecycleIndicationTestProvider::disableIndications()
{
}

void ProviderLifecycleIndicationTestProvider::createSubscription(
    const OperationContext& context,
    const CIMObjectPath& subscriptionName,
    const Array <CIMObjectPath>& classNames,
    const CIMPropertyList& propertyList,
    const Uint16 repeatNotificationPolicy)
{
    _numSubscriptions++;
}

void ProviderLifecycleIndicationTestProvider::modifySubscription(
    const OperationContext& context,
    const CIMObjectPath& subscriptionName,
    const Array <CIMObjectPath>& classNames,
    const CIMPropertyList& propertyList,
    const Uint16 repeatNotificationPolicy)
{
}

void ProviderLifecycleIndicationTestProvider::deleteSubscription(
    const OperationContext& context,
    const CIMObjectPath& subscriptionName,
    const Array <CIMObjectPath>& classNames)
{
    _numSubscriptions--;
}

void ProviderLifecycleIndicationTestProvider::invokeMethod(
    const OperationContext& context,
    const CIMObjectPath& objectReference,
    const CIMName& methodName,
    const Array<CIMParamValue>& inParameters,
    MethodResultResponseHandler& handler)
{
    handler.processing();
    if (methodName.equal("terminate"))
    {
        exit(-1);
    }
    else if (methodName.equal("getSubscriptionCount"))
    {
        handler.deliver(_numSubscriptions);
    }
    else
    {
         handler.deliver(Uint32(0));
    }
    handler.complete();
}
