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
#include "IndicationStressTestProvider.h"

PEGASUS_USING_STD;
PEGASUS_USING_PEGASUS;

static IndicationResponseHandler* _indication_handler = 0;
static Boolean _enabled = false;
static Uint32 _nextUID = 1;
static Uint32 _numSubscriptions = 0;

static CIMObjectPath path;
static Array<String> correlatedIndications;
static Array<String> sourceNameSpaces;
static Uint32 _namespaceIndex = 0;

IndicationStressTestProvider::IndicationStressTestProvider() throw()
{
    sourceNameSpaces.append("test/testprovider");
    sourceNameSpaces.append("test/testindsrcns1");
    sourceNameSpaces.append("test/testindsrcns2");

    // Set the destination of indications for all threads once
    path.setClassName("IndicationStressTestClass");
}

IndicationStressTestProvider::~IndicationStressTestProvider() throw()
{
}

void IndicationStressTestProvider::initialize(CIMOMHandle& cimom)
{
    _cimom = cimom;
}

void IndicationStressTestProvider::terminate()
{
    delete this;
}

void IndicationStressTestProvider::enableIndications(
    IndicationResponseHandler& handler)
{
    _enabled = true;
    _indication_handler = &handler;
}

void IndicationStressTestProvider::disableIndications()
{
    _enabled = false;
    _indication_handler->complete();
}

void IndicationStressTestProvider::createSubscription(
    const OperationContext& context,
    const CIMObjectPath& subscriptionName,
    const Array <CIMObjectPath>& classNames,
    const CIMPropertyList& propertyList,
    const Uint16 repeatNotificationPolicy)
{
    _numSubscriptions++;
}

void IndicationStressTestProvider::modifySubscription(
    const OperationContext& context,
    const CIMObjectPath& subscriptionName,
    const Array <CIMObjectPath>& classNames,
    const CIMPropertyList& propertyList,
    const Uint16 repeatNotificationPolicy)
{
}

void IndicationStressTestProvider::deleteSubscription(
    const OperationContext& context,
    const CIMObjectPath& subscriptionName,
    const Array <CIMObjectPath>& classNames)
{
    _numSubscriptions--;

    if (_numSubscriptions == 0)
        _enabled = false;
}

void SendIndication(int seqNum, char *UIDbuffer)
{
    CIMInstance indicationInstance(CIMName("IndicationStressTestClass"));

    indicationInstance.setPath(path);

    indicationInstance.addProperty(
        CIMProperty("IndicationIdentifier", String(UIDbuffer)));

    indicationInstance.addProperty(
        CIMProperty("CorrelatedIndications", correlatedIndications));

    CIMDateTime currentDateTime = CIMDateTime::getCurrentDateTime();

    indicationInstance.addProperty(
        CIMProperty("IndicationTime", currentDateTime));

    indicationInstance.addProperty(
        CIMProperty("IndicationSequenceNumber", CIMValue(Uint64(seqNum))));

    // cout << "IndicationStressTestProvider instance = " <<
    //     ((CIMObject)indicationInstance).toString() << endl;

    _indication_handler->deliver(indicationInstance);
}

void IndicationStressTestProvider::invokeMethod(
    const OperationContext& context,
    const CIMObjectPath& objectReference,
    const CIMName& methodName,
    const Array<CIMParamValue>& inParameters,
    MethodResultResponseHandler& handler)
{
    Uint32 indicationSendCount;
    CIMIndication indicationInstance;
    char UIDbuffer[32];

    handler.processing();

    inParameters[0].getValue().get(indicationSendCount);

    path.setNameSpace(sourceNameSpaces[_namespaceIndex++]);

    if (_namespaceIndex == 3)
    {
        _namespaceIndex = 0;
    }
    if (_enabled)
    {
        handler.deliver(CIMValue(0));
    }
    handler.complete();

    if (indicationSendCount > 0)
    {
        for (Uint32 seqNum = 1; seqNum < indicationSendCount+1; seqNum++)
        {
            sprintf(UIDbuffer, "%u", _nextUID++);
            SendIndication(seqNum, UIDbuffer);
        }
    }
}
