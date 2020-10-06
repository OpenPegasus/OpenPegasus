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


#ifndef Pegasus_IndicationHandlerService_h
#define Pegasus_IndicationHandlerService_h

#include <sys/types.h>
#include <iostream>
#include <stdio.h>
#include <string.h>

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/AsyncOpNode.h>
#include <Pegasus/Common/MessageQueueService.h>
#include <Pegasus/Common/CIMMessage.h>
#include <Pegasus/Handler/CIMHandler.h>
#include <Pegasus/Repository/CIMRepository.h>

#include <Pegasus/HandlerService/HandlerTable.h>
#include <Pegasus/HandlerService/Linkage.h>

#ifdef PEGASUS_ENABLE_DMTF_INDICATION_PROFILE_SUPPORT
#include <Pegasus/HandlerService/DestinationQueue.h>
#endif

PEGASUS_NAMESPACE_BEGIN

class PEGASUS_HANDLER_SERVICE_LINKAGE IndicationHandlerService :
    public MessageQueueService
{
public:

    typedef MessageQueueService Base;

    IndicationHandlerService(CIMRepository* repository);

    ~IndicationHandlerService();

    virtual void _handle_async_request(AsyncRequest* req);

    virtual void handleEnqueue(Message*);

    virtual void handleEnqueue();

    AtomicInt dienow;

private:
    IndicationHandlerService();  //  unimplemented

    CIMRepository* _repository;
    
    void filterInstance(bool includeQualifiers,bool includeClassOrigin,
        const CIMPropertyList& propertyList,CIMInstance & newInstance);

    CIMHandleIndicationResponseMessage* _handleIndication(
        CIMHandleIndicationRequestMessage* request);

    HandlerTable _handlerTable;

    CIMHandler* _lookupHandlerForClass(const CIMName& className);

    Boolean _loadHandler(
        CIMHandleIndicationRequestMessage* request,
        CIMException& cimException);

    Boolean _loadHandler(
        const OperationContext& context,
        const String nameSpace,
        CIMInstance& indicationInstance,
        CIMInstance& indicationHandlerInstance,
        CIMInstance& indicationSubscriptionInstance,
        CIMException& cimException,
        IndicationExportConnection **connection);

#ifdef PEGASUS_ENABLE_DMTF_INDICATION_PROFILE_SUPPORT

    /**
        This method is called when HandlerService receives the
        CIMNotifySubscriptionNotActiveRequestMessage. Indications matching the
        subscription will be discarded from the queue and traced.
    */
    CIMNotifySubscriptionNotActiveResponseMessage*
        _handleSubscriptionNotActiveRequest(
            CIMNotifySubscriptionNotActiveRequestMessage *message);

    /**
        This method is called when HandlerService receives the
        CIMNotifySubscriptionNotActiveRequestMessage. Queue will be deleted.
    */
    CIMNotifyListenerNotActiveResponseMessage* _handleListenerNotActiveRequest(
        CIMNotifyListenerNotActiveRequestMessage *message);

    /**
        This method is called when HandlerService receives the
        CIMNotifyConfigChangeRequestMessage. Property DeliveryRetryAttempts
        & DeliveryRetryInterval get updated  
    */
    CIMNotifyConfigChangeResponseMessage*
        _handlePropertyUpdateRequest(
            CIMNotifyConfigChangeRequestMessage *message);


    /**
        This method is called to stop dispatcher thread when HandlerService
        receives the CimServiceStop request.
    */
    void _stopDispatcher();

    /**
        Tries to deliver the indication, returns true if delivery is successful
        else false.
    */
    void _deliverIndication(IndicationInfo *info);

    /**
        This method is called when indication in the form of
        CIMHandleIndicationRequestMessage arrives to HandlerService  from
        IndicationService. This method sets the sequence-identfier to the
        indication and enqueues the indication into the destination queue.
   */
    void _setSequenceIdentifierAndEnqueue(
        CIMHandleIndicationRequestMessage *message);

    /**
        Starts the dispatcher thread.
    */
    void  _startDispatcher();

    /**
        This method is called when indication delivery has failed.
        This method enqueues the indication on to the DestinationQueue.
   */
    void _destinationQueueEnqueue(
        CIMHandleIndicationRequestMessage *message);

    CIMResponseMessage*
        _handleEnumerateInstancesRequest(
            CIMEnumerateInstancesRequestMessage *message);

    CIMResponseMessage*
        _handleEnumerateInstanceNamesRequest(
            CIMEnumerateInstanceNamesRequestMessage *message);

     CIMResponseMessage*
         _handleGetInstanceRequest(
             CIMGetInstanceRequestMessage *message);

    Array<CIMInstance> _getDestinationQueues(
        const CIMObjectPath &getInstanceName,
        Boolean includeQualifiers,
        Boolean includeClassOrigin,
        const CIMPropertyList &propList);

    /**
        This method Cleanup all DestinationQueues
        Depending on flag _needDestinationQueueCleanup, this method is called
        either from IndicationHandlerService dtor or from _handleIndication 
   */
    void _destinationQueuesCleanup(); 
 
   /**
        Gets the Queue name from either subscriptionName or handlerName,
        constructed as follows.
        namespace:ClassName.Name=\"HandlerName\".
    */
    String _getQueueName(
        const CIMObjectPath &instancePath);

    typedef HashTable<
                String,
                DestinationQueue*,
                EqualFunc<String>,
                HashFunc<String> > DestinationQueueTable;

    DestinationQueueTable _destinationQueueTable;
    ReadWriteSem _destinationQueueTableLock;

    AtomicInt _deliveryThreadsRunningCount;
    AtomicInt _dispatcherThreadRunning;
    List<IndicationInfo, Mutex> _deliveryQueue;
    ThreadPool _deliveryThreadPool;
    Thread _dispatcherThread;
    AtomicInt _stopDispatcherThread;
    const Uint32 _maxDeliveryThreads;
    Uint16 _maxDeliveryRetry;
    Boolean _needDestinationQueueCleanup; 
    Semaphore _dispatcherWaitSemaphore;
    static ThreadReturnType PEGASUS_THREAD_CDECL
        _dispatcherRoutine(void *param);
    static ThreadReturnType PEGASUS_THREAD_CDECL _deliveryRoutine(void *param);
#endif
};

PEGASUS_NAMESPACE_END

#endif
