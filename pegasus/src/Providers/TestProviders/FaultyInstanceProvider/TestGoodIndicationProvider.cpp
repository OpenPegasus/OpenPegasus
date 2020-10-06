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
//%///////////////////////////////////////////////////////////////////////

#include <Pegasus/Common/PegasusAssert.h>

#include "TestGoodIndicationProvider.h"

PEGASUS_USING_STD;

PEGASUS_USING_PEGASUS;

TestGoodIndicationProvider::TestGoodIndicationProvider() throw ()
{
}

TestGoodIndicationProvider::~TestGoodIndicationProvider() throw ()
{
}

void TestGoodIndicationProvider::initialize(CIMOMHandle& cimom)
{
  _cimom = cimom;
}

void TestGoodIndicationProvider::terminate()
{
    delete this;
}

void TestGoodIndicationProvider::enableIndications(
    IndicationResponseHandler& handler)
{
}

void TestGoodIndicationProvider::disableIndications()
{
}

void TestGoodIndicationProvider::createSubscription(
    const OperationContext& context,
    const CIMObjectPath& subscriptionName,
    const Array <CIMObjectPath>& classNames,
    const CIMPropertyList& propertyList,
    const Uint16 repeatNotificationPolicy)
{
    // do nothing
}

void TestGoodIndicationProvider::modifySubscription(
    const OperationContext& context,
    const CIMObjectPath& subscriptionName,
    const Array <CIMObjectPath>& classNames,
    const CIMPropertyList& propertyList,
    const Uint16 repeatNotificationPolicy)
{
    throw CIMException(CIM_ERR_NOT_SUPPORTED);    
}

void TestGoodIndicationProvider::deleteSubscription(
    const OperationContext& context,
    const CIMObjectPath& subscriptionName,
    const Array <CIMObjectPath>& classNames)
{
}
