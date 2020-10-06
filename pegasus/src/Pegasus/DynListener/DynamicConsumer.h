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

#ifndef Pegasus_Dynamic_Consumer_h
#define Pegasus_Dynamic_Consumer_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/ArrayInternal.h>
#include <Pegasus/Common/System.h>
#include <Pegasus/Common/Thread.h>
#include <Pegasus/Common/List.h>
#include <Pegasus/Common/Mutex.h>
#include <Pegasus/DynListener/Linkage.h>
#include <Pegasus/Provider/CIMOMHandle.h>
#include "DynamicConsumerFacade.h"
#include "ConsumerModule.h"

PEGASUS_NAMESPACE_BEGIN


/** The IndicationDispatchEvent class encapsulates an event and all of the
 *  information associated with the event. The operation context, URL,
 *  and the CIM indication instance are the parameters to the
 *  consumeIndication() method* of the CIMIndicationConsumer interface.
 *  Additionally, we need to store the number of retries in order to resend
 *  indications if the consumer fails.
 */
class PEGASUS_DYNLISTENER_LINKAGE IndicationDispatchEvent : public Linkable
{
public:

    IndicationDispatchEvent();

    IndicationDispatchEvent(OperationContext context,
                            String url,
                            CIMInstance instance);

    IndicationDispatchEvent(const IndicationDispatchEvent &event);

    ~IndicationDispatchEvent();

    OperationContext getContext() const;

    String getURL() const;

    CIMInstance getIndicationInstance() const;

    Uint32 getRetries();

    void increaseRetries();

    CIMDateTime getLastAttemptTime();

    IndicationDispatchEvent& operator=(const IndicationDispatchEvent &event);

    Boolean operator==(const IndicationDispatchEvent &event) const;

private:
    OperationContext _context;
    String _url;
    CIMInstance _instance;
    AtomicInt _retries;
    CIMDateTime _lastAttemptTime;

};



/** The DynamicConsumer class represents the logical consumer extracted from a
 * consumer module. It is wrapped in a facade to stabalize the interface
 * and is directly tied to a module.
 *
 * The synchronization of these actions is left up to the caller.  For example,
 * the caller must ensure that terminate is not called while initialize
 * is executing.  The ConsumerManager uses a consumer table mutex to ensure
 * that no mutually exclusive operations occur at the same time. The exception
 * to this is the operation of the worker thread, which is signalled during
 * a shutdown operation or when a new event occurs.
 */

class PEGASUS_DYNLISTENER_LINKAGE DynamicConsumer:public DynamicConsumerFacade
{
public:

    typedef DynamicConsumerFacade Base;

    DynamicConsumer();

    DynamicConsumer(const String& name);

    DynamicConsumer(const String & name,
                    ConsumerModule* consumerModule,
                    CIMIndicationConsumerProvider* consumerRef);

    ~DynamicConsumer(void);

    using DynamicConsumerFacade::initialize;
    virtual void initialize(void);

    virtual void terminate(void);

    void enqueueEvent(IndicationDispatchEvent* event);

    void sendShutdownSignal();

    String getName(void) const;

    Boolean isInitialized(void) const;

    Boolean isLoaded(void) const;

    void waitForEventThread(void);

    Boolean isIdle(void);

    virtual void getIdleTimer(struct timeval *);

    virtual void updateIdleTimer(void);

    Uint32 getPendingIndications(void);

    void set(ConsumerModule* consumerModule,
             CIMIndicationConsumerProvider* consumerRef);

    Semaphore* getShutdownSemaphore();

    void reset();

    String toString();

protected:
    ConsumerModule* _module;

private:
    friend class ConsumerManager;

    // indication queue
    List<IndicationDispatchEvent,Mutex> _eventqueue;

    // this mutex controls the state of the consumer to ensure it is
    // not initializing, terminating, etc at the same time
    // ATTN: Do we need this?  The ConsumerManager will be controlling
    // the status of the consumers. Check back here when doing global queue
    Mutex _statusMutex;

    // physical consumer variables
    CIMIndicationConsumerProvider* getConsumer();
    ConsumerModule* getModule(void) const;

    String _name;
    String _fileName;

    // state variables
    Boolean _initialized;
    Boolean _dieNow;        //indicates we are shutting down

    // we must keep track of this ourself, since we cannot use the cimomhandle
    struct timeval _idleTime;
    Mutex _idleTimeMutex;
    Boolean _no_unload;

    //Signals the worker thread to wake up and check the queue.
    //This is signalled in the following scenarios:
    //  1) shutdown event
    //  2) event was placed in queue
    // This allows for a simulated "WaitForMultipleObjects"
    Semaphore* _check_queue;

    // Signals that the event thread is listening and can now be signalled.
    // This eliminates any synchronization issues that may occur when the
    // first event comes in or if shutdown is called right as the
    // consumer thread is being started
    Semaphore* _listeningSemaphore;

    // ATTN: For now, we must store the shutdown semaphore on the consumer,
    // in order to be able to gracefully unload it during a normal shutdown
    // OR an idle shutdown.  Pegasus's ThreadPool does not provide a way to
    // access any thread information once it is spawned; the only option is
    // to pass a blocking semaphore in, which will signal when the thread
    // completes.  Since we are using one dedicated thread per consumer
    // for now, we can store one shutdown semaphore per consumer.  We'll
    // change the implementation when we go to a global queue.

    // This is used to tell the consumer manager that the worker thread has
    // indeed stopped it's passed in from the manager during initialization and
    // can be checked to determine whether the consumer can be unloaded.  This
    // setup will change as the global queueing is set up.
    Semaphore* _shutdownSemaphore;

    // these functions are used to serialize and
    // deserialize outstanding indications
    void _loadOutstandingIndications(
             Array<IndicationDispatchEvent> indications);

    Array<IndicationDispatchEvent> _retrieveOutstandingIndications();

};

PEGASUS_NAMESPACE_END

#endif

