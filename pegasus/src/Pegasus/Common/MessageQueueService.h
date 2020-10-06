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

#ifndef Pegasus_MessageQueue_Service_h
#define Pegasus_MessageQueue_Service_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/Message.h>
#include <Pegasus/Common/InternalException.h>
#include <Pegasus/Common/Thread.h>
#include <Pegasus/Common/ThreadPool.h>
#include <Pegasus/Common/AsyncOpNode.h>
#include <Pegasus/Common/Cimom.h>
#include <Pegasus/Common/Mutex.h>
#include <Pegasus/Common/CimomMessage.h>
#include <Pegasus/Common/Linkage.h>
#include <Pegasus/Common/Mutex.h>

PEGASUS_NAMESPACE_BEGIN

class message_module;
class cimom;


class PEGASUS_COMMON_LINKAGE MessageQueueService :
    public Linkable, public MessageQueue
{
public:

    typedef MessageQueue Base;

    MessageQueueService(
        const char* name);

    virtual ~MessageQueueService();

    virtual Boolean isAsync() const {  return true;  }

    virtual void enqueue(Message *);

    static AsyncReply* SendWait(AsyncRequest* request);
    Boolean SendAsync(AsyncOpNode* op,
        Uint32 destination,
        void (*callback)(AsyncOpNode*, MessageQueue*, void*),
        MessageQueue* callback_q,
        void* callback_ptr);

    static Boolean SendForget(Message* msg);

    Boolean update_service(Uint32 capabilities, Uint32 mask);
    Boolean deregister_service();

    static Uint32 find_service_qid(const char* name);
    static AsyncOpNode* get_op();
    static void return_op(AsyncOpNode* op);

    static ThreadPool* get_thread_pool();

    AtomicInt _die;
    AtomicInt _threads;
    Uint32 getIncomingCount() {return _incoming.count(); }

protected:
    virtual Boolean accept_async(AsyncOpNode* op);
    virtual void handleEnqueue() = 0;
    virtual void handleEnqueue(Message *) = 0;
    Boolean _enqueueResponse(Message *, Message *);
    virtual void _handle_incoming_operation(AsyncOpNode *);

    virtual void _handle_async_request(AsyncRequest* req);
    virtual void _handle_async_callback(AsyncOpNode* operation);
    virtual void _make_response(Message* req, Uint32 code);

    virtual void handle_AsyncIoClose(AsyncIoClose* req);
    virtual void handle_CimServiceStart(CimServiceStart* req);
    virtual void handle_CimServiceStop(CimServiceStop* req);

    void _completeAsyncResponse(
        AsyncRequest* request,
        AsyncReply* reply);
    void _complete_op_node(AsyncOpNode *);

    static cimom* _meta_dispatcher;
    static AtomicInt _service_count;
    static Mutex _meta_dispatcher_mutex;
    static ThreadPool* _thread_pool;
    Boolean _isRunning;
private:
    static Boolean _sendAsync(AsyncOpNode* op,
        Uint32 destination,
        void (*callback)(AsyncOpNode*, MessageQueue*, void*),
        MessageQueue* callback_q,
        void* callback_ptr,
        Uint32 flags);

    void _removeFromPollingList(MessageQueueService *service);

    static ThreadReturnType PEGASUS_THREAD_CDECL polling_routine(void *);

    AsyncQueue<AsyncOpNode> _incoming;
    static Thread* _polling_thread;
    static Semaphore _polling_sem;
    static AtomicInt _stop_polling;

    typedef List<MessageQueueService, NullLock> PollingList;
    static PollingList* _polling_list;
    static Mutex _polling_list_mutex;

    static ThreadReturnType PEGASUS_THREAD_CDECL _req_proc(void *);

    AtomicInt _incoming_queue_shutdown;

    friend class cimom;
};

PEGASUS_NAMESPACE_END

#endif /* Pegasus_MessageQueue_Service_h */
