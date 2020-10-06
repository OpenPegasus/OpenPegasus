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

#ifndef Pegasus_IndicationRouter_h
#define Pegasus_IndicationRouter_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/Linkage.h>
#include <Pegasus/Common/Mutex.h>
#include <Pegasus/Common/String.h>
#include <Pegasus/Common/Exception.h>
#include <Pegasus/Common/HashTable.h>
#include <Pegasus/Common/Semaphore.h>

PEGASUS_NAMESPACE_BEGIN


class CIMProcessIndicationRequestMessage;
class CIMProcessIndicationResponseMessage;

/*
 * IndicationRouter is used for routing the indications to the indication
 * service. If indication ordering is enabled, delivery routine waits until
 * indication is enqueued onto the destination queue in correct order.
 * This class used by the ProviderManagerService , ModuleController and
 * ProviderAgent to route the indications.
 */

class PEGASUS_COMMON_LINKAGE IndicationRouter
{
public:
    struct DeliveryStatusEntry
    {
        Semaphore semaphore;
        DeliveryStatusEntry():semaphore(0) {}
    };

    IndicationRouter(
        CIMProcessIndicationRequestMessage *request,
        void (*router)(CIMProcessIndicationRequestMessage*));

    ~IndicationRouter();

    /**
     * Delivers the indication and waits for its enqueue status if the
     * indication ordering is enabled.
     */
    void deliverAndWaitForStatus();

    /**
     * Notifies the waiter that the indication delivery is completed to
     * the HandlerService in correct order.
     */
    static void notify(CIMProcessIndicationResponseMessage *response);

private:
    static Mutex _statusMutex;
    typedef HashTable<
                String,
                DeliveryStatusEntry*,
                EqualFunc<String>,
                HashFunc<String> > DeliveryStatusTable;

    static DeliveryStatusTable _statusTable;
    CIMProcessIndicationRequestMessage *_request;
    void (*_deliveryRoutine)(CIMProcessIndicationRequestMessage*);
    DeliveryStatusEntry *_entry;
};

/**
 * DeliveryStatusAggregator is used to aggregate the handle indication
 * responses. DeliveryStatusAggregator is created by the IndicationService
 * if the indication orrdering is enabled.
 */
class PEGASUS_COMMON_LINKAGE DeliveryStatusAggregator
{
public:
    DeliveryStatusAggregator(
        const String &origMessageId_,
        Uint32 responseQid_,
        const String &oopAgentName_,
        Boolean waitUntilDelivered_);

    // Verifies if the ExpectedResponseCount matches the CurrentResponseCount
    // and delivers the CIMProcessIndicationResponseMessage to the waiter.
    void complete();

    // Increments the ExpectedResponseCount for each matched subscription.
    void incExpectedResponseCount();

    // Sets the flag to indicate the setting the ExpectedResponseCount is
    // completed.
    void expectedResponseCountSetDone();

    ~DeliveryStatusAggregator(){}
    String origMessageId;
    Uint32 responseQid;
    String oopAgentName;
    // If this flag is set to true, CIMProcessIndicationResponseMessage will
    // be sent only after the indication is delivered.
    Boolean waitUntilDelivered;
private:
    void _sendDeliveryStausResponse();
    DeliveryStatusAggregator(
        const DeliveryStatusAggregator &x);
    DeliveryStatusAggregator& operator=(
        const DeliveryStatusAggregator &x);
    Uint32 _expectedResponseCount;
    Uint32 _currentResponseCount;
    Boolean _expectedResponseCountSetDone;
    Mutex _responseCountMutex;
};

PEGASUS_NAMESPACE_END

#endif /* Pegasus_IndicationRouter_h */
