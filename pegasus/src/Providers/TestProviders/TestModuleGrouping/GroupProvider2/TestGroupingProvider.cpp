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

#include "TestGroupingProvider.h"
#include "../GroupUtilLib/groupUtilLib.h"

PEGASUS_NAMESPACE_BEGIN

TestGroupingProvider2::TestGroupingProvider2()
{
}

TestGroupingProvider2::~TestGroupingProvider2()
{
}

void TestGroupingProvider2::initialize(CIMOMHandle& cimom)
{
    _cimom = cimom;
    _subscriptionCount = 0;
}

void TestGroupingProvider2::terminate()
{
    delete this;
}

void TestGroupingProvider2::invokeMethod(
    const OperationContext& context,
    const CIMObjectPath& objectReference,
    const CIMName& methodName,
    const Array<CIMParamValue>& inParameters,
    MethodResultResponseHandler& handler)
{
    if (!objectReference.getClassName().equal("Test_GroupingClass2"))
    {
        throw CIMNotSupportedException(
            objectReference.getClassName().getString());
    }

    handler.processing();
    if (methodName.equal("getNextIdentifier"))
    {
        handler.deliver(CIMValue(Uint32(getNextIdentifier())));
    }
    else if (methodName.equal("getSubscriptionCount"))
    {
         handler.deliver(CIMValue(_subscriptionCount));
    }

    handler.complete();
}

void TestGroupingProvider2::enableIndications (
    IndicationResponseHandler & handler)
{
}

void TestGroupingProvider2::disableIndications ()
{
}

void TestGroupingProvider2::createSubscription (
    const OperationContext & context,
    const CIMObjectPath & subscriptionName,
    const Array <CIMObjectPath> & classNames,
    const CIMPropertyList & propertyList,
    const Uint16 repeatNotificationPolicy)
{
    _subscriptionCount++;
}

void TestGroupingProvider2::modifySubscription (
    const OperationContext & context,
    const CIMObjectPath & subscriptionName,
    const Array <CIMObjectPath> & classNames,
    const CIMPropertyList & propertyList,
    const Uint16 repeatNotificationPolicy)
{
}


void TestGroupingProvider2::deleteSubscription (
    const OperationContext & context,
    const CIMObjectPath & subscriptionName,
    const Array <CIMObjectPath> & classNames)
{
    _subscriptionCount--;
}

PEGASUS_NAMESPACE_END
