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

#include "MessageQueueService.h"
#include <Pegasus/Common/Tracer.h>
#include <Pegasus/Common/MessageLoader.h>

PEGASUS_USING_STD;

PEGASUS_NAMESPACE_BEGIN

cimom *MessageQueueService::_meta_dispatcher = 0;
AtomicInt MessageQueueService::_service_count(0);
Mutex MessageQueueService::_meta_dispatcher_mutex;

static struct timeval deallocateWait = {300, 0};

ThreadPool *MessageQueueService::_thread_pool = 0;

MessageQueueService::PollingList* MessageQueueService::_polling_list;
Mutex MessageQueueService::_polling_list_mutex;

Thread* MessageQueueService::_polling_thread = 0;

ThreadPool *MessageQueueService::get_thread_pool()
{
   return _thread_pool;
}

//
// MAX_THREADS_PER_SVC_QUEUE
//
// JR Wunderlich Jun 6, 2005
//

#define MAX_THREADS_PER_SVC_QUEUE_LIMIT 5000
#define MAX_THREADS_PER_SVC_QUEUE_DEFAULT 5

#ifndef MAX_THREADS_PER_SVC_QUEUE
# define MAX_THREADS_PER_SVC_QUEUE MAX_THREADS_PER_SVC_QUEUE_DEFAULT
#endif

Uint32 max_threads_per_svc_queue;

ThreadReturnType PEGASUS_THREAD_CDECL MessageQueueService::polling_routine(
    void* parm)
{
    Thread *myself = reinterpret_cast<Thread *>(parm);
    MessageQueueService::PollingList *list =
        reinterpret_cast<MessageQueueService::PollingList*>(myself->get_parm());

    try
    {
        while (_stop_polling.get()  == 0)
        {
            _polling_sem.wait();

            if (_stop_polling.get() != 0)
            {
                break;
            }

            // The polling_routine thread must hold the lock on the
            // _polling_list while processing incoming messages.
            // This lock is used to give this thread ownership of
            // services on the _polling_routine list.

            // This is necessary to avoid confict with other threads
            // processing the _polling_list
            // (e.g., MessageQueueServer::~MessageQueueService).

            _polling_list_mutex.lock();
            MessageQueueService *service = list->front();
            ThreadStatus rtn = PEGASUS_THREAD_OK;
            while (service != NULL)
            {
                if ((service->_incoming.count() > 0) &&
                    (service->_die.get() == 0) &&
                    (service->_threads.get() < max_threads_per_svc_queue))
                {
                    // The _threads count is used to track the
                    // number of active threads that have been allocated
                    // to process messages for this service.

                    // The _threads count MUST be incremented while
                    // the polling_routine owns the _polling_thread
                    // lock and has ownership of the service object.

                    service->_threads++;
                    rtn = _thread_pool->allocate_and_awaken(
                        service, _req_proc, &_polling_sem);
                    // if no more threads available, break from processing loop
                    if (rtn != PEGASUS_THREAD_OK )
                    {
                        service->_threads--;
                        PEG_TRACE((TRC_MESSAGEQUEUESERVICE, Tracer::LEVEL1,
                            "Could not allocate thread for %s.  Queue has %d "
                                "messages waiting and %d threads servicing."
                                "Skipping the service for right now. ",
                            service->getQueueName(),
                            service->_incoming.count(),
                            service->_threads.get()));

                        Threads::yield();
                        break;
                    }
                }
                service = list->next_of(service);
            }
            _polling_list_mutex.unlock();
        }
    }
    catch(const Exception &e)
    {
        PEG_TRACE((TRC_MESSAGEQUEUESERVICE,Tracer::LEVEL1,
            "Exception caught in MessageQueueService::polling_routine : %s",
                (const char*)e.getMessage().getCString()));
    }
    catch(const exception &e)
    {
        PEG_TRACE((TRC_MESSAGEQUEUESERVICE,Tracer::LEVEL1,
            "Exception caught in MessageQueueService::polling_routine : %s",
                e.what()));
    }
    catch(...)
    {
        PEG_TRACE_CSTRING(TRC_MESSAGEQUEUESERVICE,Tracer::LEVEL1,
            "Unknown Exception caught in MessageQueueService::polling_routine");
    }

    PEGASUS_ASSERT(_stop_polling.get());

    return ThreadReturnType(0);
}


Semaphore MessageQueueService::_polling_sem(0);
AtomicInt MessageQueueService::_stop_polling(0);


MessageQueueService::MessageQueueService(
    const char* name)
    : Base(name),
      _die(0),
      _threads(0),
      _incoming(),
      _incoming_queue_shutdown(0)
{
    _isRunning = true;

    max_threads_per_svc_queue = MAX_THREADS_PER_SVC_QUEUE;

    // if requested thread max is out of range, then set to
    // MAX_THREADS_PER_SVC_QUEUE_LIMIT

    if ((max_threads_per_svc_queue < 1) ||
        (max_threads_per_svc_queue > MAX_THREADS_PER_SVC_QUEUE_LIMIT))
    {
        max_threads_per_svc_queue = MAX_THREADS_PER_SVC_QUEUE_LIMIT;
    }

    PEG_TRACE((TRC_MESSAGEQUEUESERVICE, Tracer::LEVEL3,
       "max_threads_per_svc_queue set to %u.", max_threads_per_svc_queue));

    AutoMutex autoMut(_meta_dispatcher_mutex);

    if (_meta_dispatcher == 0)
    {
        _stop_polling = 0;
        PEGASUS_ASSERT(_service_count.get() == 0);
        _meta_dispatcher = new cimom();

        //  _thread_pool = new ThreadPool(initial_cnt, "MessageQueueService",
        //   minimum_cnt, maximum_cnt, deallocateWait);
        //
        _thread_pool =
            new ThreadPool(0, "MessageQueueService", 0, 0, deallocateWait);
    }
    _service_count++;

    // Add to the polling list
    if (!_polling_list)
    {
        _polling_list = new PollingList;
    }
    _polling_list->insert_back(this);
   _meta_dispatcher->registerCIMService(this);
}


MessageQueueService::~MessageQueueService()
{

    // Close incoming queue.
    if (_incoming_queue_shutdown.get() == 0)
    {
        AsyncIoClose *msg = new AsyncIoClose(
            0,
            _queueId);

        SendForget(msg);
        // Wait until our queue has been shutdown.
        while (_incoming_queue_shutdown.get() == 0)
        {
            Threads::yield();
        }
    }

    // die now.
    _die = 1;

    _meta_dispatcher->deregisterCIMService(this);

    // Wait until all threads processing the messages
    // for this service have completed.
    while (_threads.get() > 0)
    {
        Threads::yield();
    }


    // The polling_routine locks the _polling_list while
    // processing the incoming messages for services on the
    // list.  Deleting the service from the _polling_list
    // prior to processing, avoids synchronization issues
    // with the _polling_routine.
    _removeFromPollingList(this);

    {
        AutoMutex autoMut(_meta_dispatcher_mutex);

        _service_count--;
        // If we are last service to die, delete metadispatcher.
        if (_service_count.get() == 0)
        {
            _stop_polling++;
            _polling_sem.signal();
            if (_polling_thread)
            {
                _polling_thread->join();
                delete _polling_thread;
                _polling_thread = 0;
            }
            delete _meta_dispatcher;
            _meta_dispatcher = 0;

            delete _thread_pool;
            _thread_pool = 0;
        }
    }

    // Clean up any extra stuff on the queue.
    AsyncOpNode* op = 0;
    while ((op = _incoming.dequeue()))
    {
        delete op;
    }
}

void MessageQueueService::enqueue(Message* msg)
{
    PEG_METHOD_ENTER(TRC_MESSAGEQUEUESERVICE, "MessageQueueService::enqueue()");

    Base::enqueue(msg);

    PEG_METHOD_EXIT();
}


ThreadReturnType PEGASUS_THREAD_CDECL MessageQueueService::_req_proc(
    void* parm)
{
    MessageQueueService* service =
        reinterpret_cast<MessageQueueService*>(parm);
    PEGASUS_ASSERT(service != 0);
    try
    {
        if (service->_die.get() != 0)
        {
            service->_threads--;
            return 0;
        }
        // pull messages off the incoming queue and dispatch them. then
        // check pending messages that are non-blocking
        AsyncOpNode *operation = 0;

        // many operations may have been queued.
        do
        {
            operation = service->_incoming.dequeue();

            if (operation)
            {
               service->_handle_incoming_operation(operation);
            }
        } while (operation && !service->_incoming_queue_shutdown.get());
    }
    catch (const Exception& e)
    {
        PEG_TRACE((TRC_DISCARDED_DATA, Tracer::LEVEL1,
            "Caught exception: \"%s\".  Exiting _req_proc.",
            (const char*)e.getMessage().getCString()));
    }
    catch (...)
    {
        PEG_TRACE_CSTRING(TRC_DISCARDED_DATA, Tracer::LEVEL1,
            "Caught unrecognized exception.  Exiting _req_proc.");
    }
    service->_threads--;
    return 0;
}

// callback function is responsible for cleaning up all resources
// including op, op->_callback_node, and op->_callback_ptr
void MessageQueueService::_handle_async_callback(AsyncOpNode* op)
{
    PEGASUS_ASSERT(op->_flags == ASYNC_OPFLAGS_CALLBACK);
    // note that _callback_node may be different from op
    // op->_callback_response_q is a "this" pointer we can use for
    // static callback methods
    op->_async_callback(
        op->_callback_node, op->_callback_response_q, op->_callback_ptr);
}


void MessageQueueService::_handle_incoming_operation(AsyncOpNode* operation)
{
    if (operation != 0)
    {
        Message *rq = operation->_request.get();

// optimization <<< Thu Mar  7 21:04:05 2002 mdd >>>
// move this to the bottom of the loop when the majority of
// messages become async messages.

        // divert legacy messages to handleEnqueue
        if ((rq != 0) && (!(rq->getMask() & MessageMask::ha_async)))
        {
            operation->_request.release();
            // delete the op node
            return_op(operation);
            handleEnqueue(rq);
            return;
        }

        if ((operation->_flags & ASYNC_OPFLAGS_CALLBACK) &&
            (operation->_state & ASYNC_OPSTATE_COMPLETE))
        {
            _handle_async_callback(operation);
        }
        else
        {
            PEGASUS_ASSERT(rq != 0);
            _handle_async_request(static_cast<AsyncRequest *>(rq));
        }
    }
    return;
}

void MessageQueueService::_handle_async_request(AsyncRequest *req)
{
    MessageType type = req->getType();
    if (type == ASYNC_IOCLOSE)
    {
        handle_AsyncIoClose(static_cast<AsyncIoClose*>(req));
    }
    else if (type == ASYNC_CIMSERVICE_START)
    {
        handle_CimServiceStart(static_cast<CimServiceStart *>(req));
    }
    else if (type == ASYNC_CIMSERVICE_STOP)
    {
        handle_CimServiceStop(static_cast<CimServiceStop *>(req));
    }
    else
    {
        // we don't handle this request message
        _make_response(req, async_results::CIM_NAK);
    }
}

Boolean MessageQueueService::_enqueueResponse(
    Message* request,
    Message* response)
{
    PEG_METHOD_ENTER(TRC_MESSAGEQUEUESERVICE,
        "MessageQueueService::_enqueueResponse");

    if (request->getMask() & MessageMask::ha_async)
    {
        if (response->getMask() & MessageMask::ha_async)
        {
            _completeAsyncResponse(
                static_cast<AsyncRequest *>(request),
                static_cast<AsyncReply *>(response));

            PEG_METHOD_EXIT();
            return true;
        }
    }

    AsyncRequest* asyncRequest =
        static_cast<AsyncRequest*>(request->get_async());

    if (asyncRequest != 0)
    {
        PEGASUS_ASSERT(asyncRequest->getMask() &
            (MessageMask::ha_async | MessageMask::ha_request));

        AsyncOpNode* op = asyncRequest->op;

        // the legacy request is going to be deleted by its handler
        // remove it from the op node

        static_cast<AsyncLegacyOperationStart *>(asyncRequest)->get_action();

        AsyncLegacyOperationResult *async_result =
            new AsyncLegacyOperationResult(
                op,
                response);
        _completeAsyncResponse(
            asyncRequest,
            async_result);

        PEG_METHOD_EXIT();
        return true;
    }

    // ensure that the destination queue is in response->dest
    PEG_METHOD_EXIT();
    return SendForget(response);
}

void MessageQueueService::_make_response(Message* req, Uint32 code)
{
    cimom::_make_response(req, code);
}

void MessageQueueService::_completeAsyncResponse(
    AsyncRequest* request,
    AsyncReply* reply)
{
    PEG_METHOD_ENTER(TRC_MESSAGEQUEUESERVICE,
        "MessageQueueService::_completeAsyncResponse");

    cimom::_completeAsyncResponse(request, reply);

    PEG_METHOD_EXIT();
}


void MessageQueueService::_complete_op_node(
    AsyncOpNode* op)
{
    cimom::_complete_op_node(op);
}


Boolean MessageQueueService::accept_async(AsyncOpNode* op)
{
    if (!_isRunning)
    {
        // Don't accept any messages other than start.
        if (op->_request.get()->getType() != ASYNC_CIMSERVICE_START)
        {
            return false;
        }
    }

    if (_incoming_queue_shutdown.get() > 0)
        return false;

    if (_polling_thread == NULL)
    {
        PEGASUS_ASSERT(_polling_list);
        _polling_thread = new Thread(
            polling_routine,
            reinterpret_cast<void *>(_polling_list),
            false);
        ThreadStatus tr = PEGASUS_THREAD_OK;
        while ( (tr =_polling_thread->run()) != PEGASUS_THREAD_OK)
        {
            if (tr == PEGASUS_THREAD_INSUFFICIENT_RESOURCES)
                Threads::yield();
            else
                throw Exception(MessageLoaderParms(
                    "Common.MessageQueueService.NOT_ENOUGH_THREAD",
                    "Could not allocate thread for the polling thread."));
        }
    }
    if (_die.get() == 0)
    {
        if (_incoming.enqueue(op))
        {
            _polling_sem.signal();
            return true;
        }
    }
    return false;
}

void MessageQueueService::handle_AsyncIoClose(AsyncIoClose *req)
{
    MessageQueueService *service =
        static_cast<MessageQueueService*>(req->op->_op_dest);

#ifdef MESSAGEQUEUESERVICE_DEBUG
    PEGASUS_STD(cout) << service->getQueueName() <<
        " Received AsyncIoClose " << PEGASUS_STD(endl);
#endif
    // set the closing flag, don't accept any more messages
    service->_incoming_queue_shutdown = 1;
    //Wait until no more threads are running in the service except ourself.
    //Note that if we don't  wait for all threads to terminate here, deletion
    //of this service later may destruct derived class members while still
    //running threads in this service trying to access them.
    while (service->_threads.get() > 1)
    {
        Threads::yield();
        Threads::sleep(50);
    }
    // respond to this message. this is fire and forget, so we
    // don't need to delete anything.
    // this takes care of two problems that were being found
    // << Thu Oct  9 10:52:48 2003 mdd >>
    _make_response(req, async_results::OK);
}

void MessageQueueService::handle_CimServiceStart(CimServiceStart* req)
{
#ifdef MESSAGEQUEUESERVICE_DEBUG
    PEGASUS_STD(cout) << getQueueName() << "received START" <<
        PEGASUS_STD(endl);
#endif
    PEGASUS_ASSERT(!_isRunning);
    _isRunning = true;
    _make_response(req, async_results::OK);
}

void MessageQueueService::handle_CimServiceStop(CimServiceStop* req)
{
#ifdef MESSAGEQUEUESERVICE_DEBUG
    PEGASUS_STD(cout) << getQueueName() << "received STOP" << PEGASUS_STD(endl);
#endif
    PEGASUS_ASSERT(_isRunning);
    _isRunning = false;
    _make_response(req, async_results::CIM_SERVICE_STOPPED);
}

AsyncOpNode* MessageQueueService::get_op()
{
   AsyncOpNode* op = new AsyncOpNode();

   op->_state = ASYNC_OPSTATE_UNKNOWN;
   op->_flags = ASYNC_OPFLAGS_UNKNOWN;

   return op;
}

void MessageQueueService::return_op(AsyncOpNode* op)
{
    delete op;
}


Boolean MessageQueueService::SendAsync(
    AsyncOpNode* op,
    Uint32 destination,
    void (*callback)(AsyncOpNode*, MessageQueue*, void*),
    MessageQueue* callback_response_q,
    void* callback_ptr)
{
    return _sendAsync(
        op,
        destination,
        callback,
        callback_response_q,
        callback_ptr,
        ASYNC_OPFLAGS_CALLBACK);

}

Boolean MessageQueueService::_sendAsync(
    AsyncOpNode* op,
    Uint32 destination,
    void (*callback)(AsyncOpNode*, MessageQueue*, void*),
    MessageQueue* callback_response_q,
    void* callback_ptr,
    Uint32 flags)
{
    PEGASUS_ASSERT(op != 0);
    PEGASUS_ASSERT((callback == 0) ==
        (flags == ASYNC_OPFLAGS_PSEUDO_CALLBACK));

    // destination of this message
    op->_op_dest = MessageQueue::lookup(destination);
    if (op->_op_dest == 0)
    {
        return false;
    }
    op->_flags = flags;
    // initialize the callback data
    // callback function to be executed by recpt. of response
    op->_async_callback = callback;
    // the op node
    op->_callback_node = op;
    // the queue that will receive the response
    op->_callback_response_q = callback_response_q;
    // user data for callback
    op->_callback_ptr = callback_ptr;

    return  _meta_dispatcher->route_async(op);
}

Boolean MessageQueueService::SendForget(Message* msg)
{
    AsyncOpNode* op = 0;
    Uint32 mask = msg->getMask();

    if (mask & MessageMask::ha_async)
    {
        op = (static_cast<AsyncMessage *>(msg))->op;
    }

    if (op == 0)
    {
        op = get_op();
        op->_request.reset(msg);
        if (mask & MessageMask::ha_async)
        {
            (static_cast<AsyncMessage *>(msg))->op = op;
        }
    }

    PEGASUS_ASSERT(op->_flags == ASYNC_OPFLAGS_UNKNOWN);
    PEGASUS_ASSERT(op->_state == ASYNC_OPSTATE_UNKNOWN);
    op->_op_dest = MessageQueue::lookup(msg->dest);
    if (op->_op_dest == 0)
    {
        return_op(op);
        return false;
    }

    op->_flags = ASYNC_OPFLAGS_FIRE_AND_FORGET;

    // now see if the meta dispatcher will take it
    return  _meta_dispatcher->route_async(op);
}


AsyncReply *MessageQueueService::SendWait(AsyncRequest* request)
{
    if (request == 0)
        return 0;

    Boolean destroy_op = false;

    if (request->op == 0)
    {
        request->op = get_op();
        request->op->_request.reset(request);
        destroy_op = true;
    }

    PEGASUS_ASSERT(request->op->_flags == ASYNC_OPFLAGS_UNKNOWN);
    PEGASUS_ASSERT(request->op->_state == ASYNC_OPSTATE_UNKNOWN);

    _sendAsync(
        request->op,
        request->dest,
        0,
        0,
        (void *)0,
        ASYNC_OPFLAGS_PSEUDO_CALLBACK);

    request->op->_client_sem.wait();

    AsyncReply* rpl = static_cast<AsyncReply *>(request->op->removeResponse());
    rpl->op = 0;

    if (destroy_op == true)
    {
        request->op->_request.release();
        return_op(request->op);
        request->op = 0;
    }
    return rpl;
}

Uint32 MessageQueueService::find_service_qid(const char* name)
{
    MessageQueue* queue = MessageQueue::lookup(name);
    PEGASUS_ASSERT(queue);
    return queue->getQueueId();
}

void MessageQueueService::_removeFromPollingList(MessageQueueService *service)
{
    _polling_list_mutex.lock();
    _polling_list->remove(service);
    _polling_list_mutex.unlock();
}

PEGASUS_NAMESPACE_END
