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

#include <Pegasus/Common/IndicationRouter.h>
#include <Pegasus/Common/ModuleController.h>
#include <Pegasus/Common/Exception.h>
#include <Pegasus/Common/MessageQueueService.h>

PEGASUS_NAMESPACE_BEGIN

Mutex IndicationRouter::_statusMutex;
IndicationRouter::DeliveryStatusTable IndicationRouter::_statusTable;

IndicationRouter::IndicationRouter(
    CIMProcessIndicationRequestMessage *request,
    void (*deliveryRoutine)(CIMProcessIndicationRequestMessage*))
        : _request(request), _deliveryRoutine(deliveryRoutine), _entry(0)
{
}

void IndicationRouter::deliverAndWaitForStatus()
{
    String uniqueMessageId;

#ifdef PEGASUS_ENABLE_INDICATION_ORDERING
    // Wait only if this indication is not coming from OOP provider.
    Uint32 timeoutMilliSec = _request->timeoutMilliSec;

    if (!_request->oopAgentName.size())
    {
        char messagePtrString[20];
        sprintf(messagePtrString, "%p", this);
        uniqueMessageId = messagePtrString;
        _request->messageId = uniqueMessageId;

        AutoMutex mtx(_statusMutex);
        _entry =  new DeliveryStatusEntry;
        PEGASUS_FCT_EXECUTE_AND_ASSERT(
            true,
            _statusTable.insert(uniqueMessageId, _entry));
    }

    _deliveryRoutine(_request);

    if (_entry)
    {
        // Maximum wait time is equals to SequenceIdentifierLifeTime
        // = 10 * DeliveryRetryAttempts * DeliveryRetryInterval
        Uint32 maxWaitTimeMilliSec = 10 * 3 * 20 * 1000;
        if (timeoutMilliSec == 0 )
        {
            // Minimum wait time is equals to default
            // DeliveryRetryAttempts * DeliveryRetryInterval
            timeoutMilliSec = 3 * 20 * 1000;
        }
        else if (timeoutMilliSec > maxWaitTimeMilliSec)
        {
            timeoutMilliSec = maxWaitTimeMilliSec;
        }
        _entry->semaphore.time_wait(timeoutMilliSec);
        AutoMutex mtx(_statusMutex);
        _statusTable.remove(uniqueMessageId);
    }
#else
    _deliveryRoutine(_request);
#endif
}

void IndicationRouter::notify(CIMProcessIndicationResponseMessage *response)
{
#ifdef PEGASUS_ENABLE_INDICATION_ORDERING
    DeliveryStatusEntry *entry;
    AutoMutex mtx(_statusMutex);
    if (_statusTable.lookup(response->messageId, entry))
    {
        entry->semaphore.signal();
    }
    delete response;
#else
    // We should not reach here if indication ordering is not enabled.
    PEGASUS_ASSERT(false);
#endif
}

IndicationRouter::~IndicationRouter()
{
    delete _entry;
}

DeliveryStatusAggregator:: DeliveryStatusAggregator(
    const String &origMessageId_,
    Uint32 responseQid_,
    const String &oopAgentName_,
    Boolean waitUntilDelivered_):
        origMessageId(origMessageId_),
        responseQid(responseQid_),
        oopAgentName(oopAgentName_),
        waitUntilDelivered(waitUntilDelivered_),
        _expectedResponseCount(0),
        _currentResponseCount(0),
        _expectedResponseCountSetDone(false)
{
}

void  DeliveryStatusAggregator::complete()
{
    {
        AutoMutex mtx(_responseCountMutex);
        _currentResponseCount++;
        if (!_expectedResponseCountSetDone ||
            _expectedResponseCount != _currentResponseCount)
        {
            return;
        }
    }
    // Don't acquire _responseCountMutex while calling the
    // _sendDeliveryStausResponse. This method deletes this
    // object.
    _sendDeliveryStausResponse();
}

void DeliveryStatusAggregator::incExpectedResponseCount()
{
    AutoMutex mtx(_responseCountMutex);
    _expectedResponseCount++;
}

void DeliveryStatusAggregator::expectedResponseCountSetDone()
{
    {
        AutoMutex mtx(_responseCountMutex);
        _expectedResponseCountSetDone = true;
        if (_expectedResponseCount != _currentResponseCount)
        {
            return;
        }
    }
    // Don't acquire _responseCountMutex while calling the
    // _sendDeliveryStausResponse. This method deletes this
    // object.
    _sendDeliveryStausResponse();
}

void DeliveryStatusAggregator::_sendDeliveryStausResponse()
{
    CIMProcessIndicationResponseMessage *response =
         new CIMProcessIndicationResponseMessage(
             origMessageId,
             CIMException(),
             QueueIdStack(responseQid),
             oopAgentName);
    response->dest = responseQid;
    MessageQueueService::SendForget(response);
    delete this;
}

PEGASUS_NAMESPACE_END
