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

#include "Cimom.h"

#include <iostream>
#include <Pegasus/Common/Constants.h>
#include <Pegasus/Common/Tracer.h>
#include <Pegasus/Common/MessageLoader.h>
#include <Pegasus/Common/AutoPtr.h>

PEGASUS_USING_STD;

PEGASUS_NAMESPACE_BEGIN

cimom::RegisteredServicesTable cimom::_registeredServicesTable;
Mutex cimom::_registeredServicesTableLock;

PEGASUS_TEMPLATE_SPECIALIZATION struct HashFunc<void*>
{
    static Uint32 hash(void* x)
    {
        return Uint32((((unsigned long)x) >> 3)) + 13;
    }
};

Boolean cimom::registerCIMService(MessageQueueService *service)
{
    AutoMutex mtx(_registeredServicesTableLock);
    if (_registeredServicesTable.insert(service, false))
    {
        return true;
    }
    return false;
}

Boolean cimom::deregisterCIMService(MessageQueueService *service)
{
    for(;;)
    {
        {
            AutoMutex mtx(_registeredServicesTableLock);
            Boolean monitoring;
            if (!_registeredServicesTable.lookup(service, monitoring))
            {
                return false;
            }
            if (!monitoring)
            {
                _registeredServicesTable.remove(service);
                break;
            }
        }
        Threads::yield();
    }

    return true;
}

Boolean cimom::_monitorCIMService(MessageQueueService *service)
{
    AutoMutex mtx(_registeredServicesTableLock);
    Boolean *monitoring;
    if (!_registeredServicesTable.lookupReference(service, monitoring))
    {
        return false;
    }
    PEGASUS_ASSERT (*monitoring == false);
    return *monitoring = true;
}

void cimom::_releaseCIMService(MessageQueueService *service)
{
    AutoMutex mtx(_registeredServicesTableLock);
    Boolean *monitoring=0;
    if (!_registeredServicesTable.lookupReference(service, monitoring))
    {
        PEGASUS_ASSERT(0);
    }
    PEGASUS_ASSERT (*monitoring == true);
    *monitoring = false;
}

Boolean cimom::route_async(AsyncOpNode *op)
{
    if (_die.get() > 0)
        return false;

    if (_routed_queue_shutdown.get() > 0)
        return false;

    return _routed_ops.enqueue(op);
}

ThreadReturnType PEGASUS_THREAD_CDECL cimom::_routing_proc(void *parm)
{
    Thread* myself = reinterpret_cast<Thread *>(parm);
    cimom* dispatcher = reinterpret_cast<cimom *>(myself->get_parm());
    AsyncOpNode *op = 0;

    try
    {
        while (dispatcher->_die.get() == 0)
        {
            op = dispatcher->_routed_ops.dequeue_wait();

            if (op == 0)
            {
                break;
            }
            else
            {
                MessageQueue *dest_q = op->_op_dest;

                // See if we are the destination.
                if (dest_q == _global_this)
                {
                    dispatcher->_handle_cimom_op(op);
                    continue;
                }

                MessageQueueService *dest_svc =
                    dynamic_cast<MessageQueueService *>(dest_q);
                PEGASUS_ASSERT(dest_svc);

                Boolean accepted = false;
                if (dispatcher->_monitorCIMService(dest_svc))
                {
                    accepted = dest_svc->accept_async(op);
                    dispatcher->_releaseCIMService(dest_svc);
                }
                if (accepted == false)
                {
                    // Send NAK to requesting service.
                    try
                    {
                        _make_response(
                            op->_request.get(),
                            async_results::CIM_NAK);
                    }
                    catch(...)
                    {
                        // May be bad_alloc caused _make_response to fail,
                        // delete op.
                        PEG_TRACE_CSTRING(
                            TRC_MESSAGEQUEUESERVICE,Tracer::LEVEL1,
                            "cimom::_make_response failed");
                        _global_this->cache_op(op);
                    }
                }
            }
        }
    }
    // Note: We should never get exception and we are not expecting it.
    catch(const Exception &e)
    {
        PEG_TRACE((TRC_MESSAGEQUEUESERVICE,Tracer::LEVEL1,
            "Exception caught in cimom::_routing_proc : %s",
                (const char*)e.getMessage().getCString()));
    }
    catch(const exception &e)
    {
        PEG_TRACE((TRC_MESSAGEQUEUESERVICE,Tracer::LEVEL1,
            "Exception caught in cimom::_routing_proc : %s", e.what()));
    }
    catch(...)
    {
        PEG_TRACE_CSTRING(TRC_MESSAGEQUEUESERVICE,Tracer::LEVEL1,
            "Unknown Exception caught in cimom::_routing_proc");
    }

    return 0;
}


cimom::cimom()
    : MessageQueue(PEGASUS_QUEUENAME_METADISPATCHER),
      _routed_ops(),
      _routing_thread(_routing_proc, this, false),
      _die(0),
      _routed_queue_shutdown(0)
{
    _global_this = this;

    ThreadStatus tr = PEGASUS_THREAD_OK;
    while ((tr = _routing_thread.run()) != PEGASUS_THREAD_OK)
    {
        if (tr == PEGASUS_THREAD_INSUFFICIENT_RESOURCES)
            Threads::yield();
        else
            throw Exception(
                MessageLoaderParms("Common.Cimom.NOT_ENOUGH_THREADS",
                    "Cannot allocate thread for Cimom class"));
    }
}


cimom::~cimom()
{
    PEGASUS_ASSERT(_routed_queue_shutdown.get() == 0);

    AsyncIoClose *msg = new AsyncIoClose(
        0,
        getQueueId());

    msg->op = get_cached_op();
    msg->op->_flags = ASYNC_OPFLAGS_FIRE_AND_FORGET;
    msg->op->_op_dest = _global_this;
    msg->op->_request.reset(msg);

    PEGASUS_FCT_EXECUTE_AND_ASSERT(true,_routed_ops.enqueue(msg->op));

    _routing_thread.join();

    PEGASUS_ASSERT(_routed_queue_shutdown.get());
    PEGASUS_ASSERT(_die.get());
}

void cimom::_make_response(Message *req, Uint32 code)
{
    if (!(req->getMask() & MessageMask::ha_async))
    {
        // legacy message, just delete
        delete req;
        return;
    }

    AsyncOpNode *op = static_cast<AsyncRequest *>(req)->op;

    // If the state is complete means requesting service was unable to take the
    // response, nothing we can do here but delete op.
    if (op->_state == ASYNC_OPSTATE_COMPLETE)
    {
        _global_this->cache_op(op);
        return;
    }

    Uint32 flags = op->_flags;

    if (flags == ASYNC_OPFLAGS_FIRE_AND_FORGET)
    {
        _global_this->cache_op(static_cast<AsyncRequest *>(req)->op);
        return;
    }

    AutoPtr<AsyncReply> reply;
    reply.reset(new AsyncReply(
        ASYNC_REPLY,
        0,
        (static_cast<AsyncRequest *>(req))->op,
        code));

    _completeAsyncResponse(
        static_cast<AsyncRequest*>(req),
        reply.get());
    reply.release();
}

void cimom::_completeAsyncResponse(
    AsyncRequest *request,
    AsyncReply *reply)
{
    PEG_METHOD_ENTER(TRC_MESSAGEQUEUESERVICE, "cimom::_completeAsyncResponse");
    PEGASUS_ASSERT(request != 0);

    AsyncOpNode *op = request->op;
    if (op->_flags == ASYNC_OPFLAGS_CALLBACK)
    {
        if (reply != 0)
        {
            op->_response.reset(reply);
        }
        _complete_op_node(op);
        return;
    }
    else if (op->_flags == ASYNC_OPFLAGS_FIRE_AND_FORGET)
    {
        PEGASUS_ASSERT(op->_state == ASYNC_OPSTATE_UNKNOWN);
        _global_this->cache_op(op);
    }
    else
    {
        PEGASUS_ASSERT (op->_flags == ASYNC_OPFLAGS_PSEUDO_CALLBACK);
        PEGASUS_ASSERT(op->_state == ASYNC_OPSTATE_UNKNOWN);
        op->_state = ASYNC_OPSTATE_COMPLETE;
        op->_client_sem.signal();
    }
    PEG_METHOD_EXIT();
}

cimom *cimom::_global_this;

void cimom::_complete_op_node(
    AsyncOpNode *op)
{
    Uint32 flags = op->_flags;

    PEGASUS_ASSERT(op->_state == ASYNC_OPSTATE_UNKNOWN);

    op->_state = ASYNC_OPSTATE_COMPLETE;
    if (flags ==  ASYNC_OPFLAGS_FIRE_AND_FORGET )
    {
        _global_this->cache_op(op);
        return;
    }

    if (flags ==  ASYNC_OPFLAGS_PSEUDO_CALLBACK)
    {
        op->_client_sem.signal();
        return;
    }

    PEGASUS_ASSERT(flags == ASYNC_OPFLAGS_CALLBACK);

    // send this node to the response queue
    op->_op_dest = op->_callback_response_q;
    _global_this->route_async(op);
}


void cimom::handleEnqueue()
{
    //ATTN: We should never get legacy messages directly.
    PEGASUS_ASSERT(0);
}


void cimom::_handle_cimom_op(AsyncOpNode *op)
{
    Message* msg = op->getRequest();

    // We handle only one message at present.
    PEGASUS_ASSERT( msg->getType() ==  ASYNC_IOCLOSE);
    _global_this->_routed_queue_shutdown = 1;
    _make_response(msg, async_results::OK);
    // All services are shutdown, empty out the queue
    for(;;)
    {
        AsyncOpNode* operation = 0;
        try
        {
            operation = _global_this->_routed_ops.dequeue();
            if (operation)
            {
                _global_this->cache_op(operation);
            }
            else
            {
                break;
            }
        }
        catch (...)
        {
             break;
        }
    }
    // shutdown the AsyncQueue
    _global_this->_routed_ops.close();
    // exit the routing thread.
    _die++;
}

AsyncOpNode* cimom::get_cached_op()
{
    AutoPtr<AsyncOpNode> op(new AsyncOpNode());

    op->_state = ASYNC_OPSTATE_UNKNOWN;
    op->_flags = ASYNC_OPFLAGS_UNKNOWN;

    return op.release();
}

void cimom::cache_op(AsyncOpNode* op)
{
    delete op;
}

PEGASUS_NAMESPACE_END
