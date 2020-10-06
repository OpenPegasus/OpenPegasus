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
#include <Pegasus/Common/Constants.h>
#include <Pegasus/Common/CIMDateTime.h>

#include "SNMPIndicationProvider.h"

PEGASUS_USING_PEGASUS;

SNMPIndicationProvider::SNMPIndicationProvider() throw()
{
}

SNMPIndicationProvider::~SNMPIndicationProvider() throw()
{
}

void SNMPIndicationProvider::initialize(CIMOMHandle& cimom)
{
    // save cimom handle
    _cimom = cimom;
}

void SNMPIndicationProvider::terminate()
{
    delete this;
}

void SNMPIndicationProvider::enableIndications(
    IndicationResponseHandler& handler)
{
    handler.processing();

    //Indication
    CIMInstance cimInstance(
        PEGASUS_NAMESPACENAME_INTEROP.getString() +
            ":cycHostAdapterDiscovered");

    cimInstance.addProperty(CIMProperty("cycHostAdapterNumber", Uint32(101)));
    cimInstance.addProperty(
        CIMProperty("cycHostAdapterID", String("HP-NU-123")));
    cimInstance.addProperty(CIMProperty("cycManagerID", String("HP-NU-456")));

    CIMIndication cimIndication(cimInstance);

    handler.deliver(cimIndication);
}

void SNMPIndicationProvider::disableIndications()
{
    //handler.complete();
}

void SNMPIndicationProvider::createSubscription(
    const OperationContext& context,
    const CIMObjectPath& subscriptionName,
    const Array<CIMObjectPath>& classNames,
    const CIMPropertyList& propertyList,
    const Uint16 repeatNotificationPolicy)
{
}

void SNMPIndicationProvider::modifySubscription(
    const OperationContext& context,
    const CIMObjectPath& subscriptionName,
    const Array<CIMObjectPath>& classNames,
    const CIMPropertyList& propertyList,
    const Uint16 repeatNotificationPolicy)
{
}

void SNMPIndicationProvider::deleteSubscription(
    const OperationContext& context,
    const CIMObjectPath& subscriptionName,
    const Array<CIMObjectPath>& classNames)
{
}
