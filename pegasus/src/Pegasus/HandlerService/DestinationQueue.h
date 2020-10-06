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

#ifndef Pegasus_DestinationQueue_h
#define Pegasus_DestinationQueue_h

#include <Pegasus/Common/List.h>
#include <Pegasus/Common/String.h>
#include <Pegasus/Common/CIMInstance.h>
#include <Pegasus/Common/OperationContext.h>
#include <Pegasus/Common/Mutex.h>
#include <Pegasus/Common/CIMMessage.h>
#include <Pegasus/Common/IndicationRouter.h>

#include <Pegasus/HandlerService/Linkage.h>
#include <Pegasus/HandlerService/IndicationHandlerConstants.h>
#include <Pegasus/Handler/CIMHandler.h>

PEGASUS_NAMESPACE_BEGIN

class DestinationQueue;
/**
    This class is used to store the indication related information in the
    DestinationQueue
*/

class PEGASUS_HANDLER_SERVICE_LINKAGE IndicationInfo : public Linkable
{
public:
    IndicationInfo(
        const CIMInstance &indication_,
        const CIMInstance &subscription_,
        const OperationContext &context_,
        const String &nameSpace_,
        DestinationQueue *queue_,
        DeliveryStatusAggregator *deliveryStatusAggregator_):
            indication(indication_),
            subscription(subscription_),
            context(context_),
            nameSpace(nameSpace_),
            queue(queue_),
            deliveryStatusAggregator(deliveryStatusAggregator_),
            deliveryRetryAttemptsMade(0)
    {
    }

    ~IndicationInfo()
    {
        if (deliveryStatusAggregator)
        {
            deliveryStatusAggregator->complete();
        }
    }
    
    CIMInstance indication;
    CIMInstance subscription;
    OperationContext context;
    String nameSpace;
    DestinationQueue *queue;
    DeliveryStatusAggregator *deliveryStatusAggregator;
    Uint16 deliveryRetryAttemptsMade;
    Uint64 arrivalTimeUsec;
    Uint64 lastDeliveryRetryTimeUsec;
};

/**
    The DestinationQueue class holds the indications to be delivered to the
    destination in the form of IndicationInfo.
*/

class PEGASUS_HANDLER_SERVICE_LINKAGE DestinationQueue
{
public:
    // Structure to hold the queue statistics information
    struct QueueInfo
    {
        CIMObjectPath handlerName;
        Uint64 queueCreationTimeUsec;
        String sequenceContext;
        Sint64 nextSequenceNumber;
        Uint32 maxQueueLength;
        Uint32 sequenceIdentifierLifetimeSeconds;
        Uint32 size;
        Uint64 queueFullDroppedIndications;
        Uint64 lifetimeExpiredIndications;
        Uint64 retryAttemptsExceededIndications;
        Uint64 subscriptionDisableDroppedIndications;
        Uint64 lastSuccessfulDeliveryTimeUsec;
    };

    // Reasons for discarding the indications.
    enum
    {
        LISTENER_NOT_ACTIVE,
        SUBSCRIPTION_NOT_ACTIVE,
        DESTINATIONQUEUE_FULL,
        SIL_EXPIRED, //SequenceIdentifierLifetime expired.
        DRA_EXCEEDED, //DeliveryRetryAttempts exceeded.
        CIMSERVER_SHUTDOWN
     };

     // LastDelivery status for the DestinationQueue.
     enum
     {
        PENDING,
        FAIL,
        SUCCESS,
    };

    DestinationQueue(const CIMInstance &handler);
    ~DestinationQueue();

    Sint64 getSequenceNumber();

    String getSequenceContext()
    {
        return _sequenceContext;
    }

    CIMInstance& getHandler()
    {
        return _handler;
    }

    static Uint32 getDeliveryRetryIntervalSeconds()
    {
        return Uint32(_minDeliveryRetryIntervalUsec / 1000000);
    }
   
    static void setDeliveryRetryAttempts(Uint16 DeliveryRetryAttempts);

    static void setminDeliveryRetryInterval(Uint32 minDeliveryRetryInterval);

    void enqueue(
        CIMHandleIndicationRequestMessage *message);

    /**
        Cleanup all the indications from the DestinationQueue. This method
        should be called on the DestionationQueue when listener has been
        deleted.
    */
    void cleanup();

    /**
        Cleanup all the indications from the DestinationQueue. This method
        should be called on the DestionationQueue during the CIMServer shutdown.
    */
    void shutdown();

    /**
        Updates the lastDeliveryRetry status of the queue to SUCCESS and also
        updates the last successful delivery retry time.
    */
    void updateDeliveryRetrySuccess(IndicationInfo *message);

    /**
        Updates the lastDeliveryRetry status of the queue to FAIL.
        DeliveryRetryAttempts of indication is incremented by one and enqueued
        onto the queue if

        1. DeliveryRetryAttempts for the indication is not exceeded.
        2. Queue is not already full.

        Indication is dicarded and traces if the any of the above conditions
        are met.
    */
    void updateDeliveryRetryFailure(
        IndicationInfo *message, const CIMException &e);

    /**
        Deletes all the matched indications for the corresponding subscription.
    */
    void deleteMatchedIndications(const CIMObjectPath &subscriptionPath);

    /**
        Gets the next eligible indication for the delivery. This method gets
        the eligible indications in the following way.

        1. If no indications in the queue or indication is already being retried
           for delivery from the queue , return NULL.
        2. Delete all the sequence-identfier-lifetime expired indications.
           Return NULL if there are no indications.
        3. Verify and return if the DeliveryRetryInterval has excedded for the
           indication at the front of the queue.
        4. Calculate the next indication's DeliveryRetryInterval expiration time
           and return as part of output-arg (nextIndDRIExpTimeUsec).
        5. timeNowUsec(Current time in usec) is input-arg passed to this method
           for performance reasons.
    */
    IndicationInfo* getNextIndicationForDelivery(
        Uint64 &timeNowUsec, Uint64 &nextIndDRIExpTimeUsec);

    void getInfo(QueueInfo &qinfo);

    IndicationExportConnection** getConnectionPtr()
    {
        return &_connection;
    }

private:
    void _cleanup(int reasonCode);
    CIMInstance _getInstance(const CIMName &className);
    void _initIndicationServiceProperties();
    void _initObjectManagerProperties();
    void _waitForNonPendingDeliveryStatus();
    String _getSequenceContext(const CIMInstance &indication);
    Sint64 _getSequenceNumber(const CIMInstance &indication);
    /**
        Logs the discarded indication with the reason. SequenceContext
        and SequenceNumber properties of indication are traced.
    */
    void _logDiscardedIndication(
        Uint32 reasonCode,
        const CIMInstance &indication,
        const String &message =  String());

    CIMInstance _handler;
    IndicationExportConnection *_connection;
    List<IndicationInfo,NullLock> _queue;
    Mutex _queueMutex;
    Uint32 _lastDeliveryRetryStatus;
    String _sequenceContext;
    Sint64 _sequenceNumber;
    Boolean _calcMaxQueueSize;

    static Uint16 _maxDeliveryRetryAttempts;
    static Uint64 _minDeliveryRetryIntervalUsec;
    static Uint64 _sequenceIdentifierLifetimeUsec;
    static Uint64 _minSubscriptionRemovalTimeIntervalUsec;
    static Uint64 _serverStartupTimeUsec;
    static Boolean _initialized;
    static String _indicationServiceName;
    static String _objectManagerName;
    static Uint32    _indicationServiceQid;

    struct IndDiscardedReasonMsgs
    {
        const char *key;
        const char *msg;
    };

    static IndDiscardedReasonMsgs indDiscardedReasonMsgs[];

    Uint64 _queueFullDroppedIndications;
    Uint64 _lifetimeExpiredIndications;
    Uint64 _retryAttemptsExceededIndications;
    Uint64 _subscriptionDeleteDroppedIndications;
    Uint64 _lastSuccessfulDeliveryTimeUsec;
    Uint64 _queueCreationTimeUsec;
    Uint32 _maxIndicationDeliveryQueueSize;
};

PEGASUS_NAMESPACE_END

#endif // Pegasus_DestinationQueue_h
