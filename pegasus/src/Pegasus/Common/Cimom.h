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

#ifndef Pegasus_Cimom_h
#define Pegasus_Cimom_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/InternalException.h>
#include <Pegasus/Common/MessageQueue.h>
#include <Pegasus/Common/AsyncQueue.h>
#include <Pegasus/Common/Thread.h>
#include <Pegasus/Common/ArrayInternal.h>
#include <Pegasus/Common/AsyncOpNode.h>
#include <Pegasus/Common/CimomMessage.h>
#include <Pegasus/Common/MessageQueueService.h>
//#include <Pegasus/Server/CIMOperationRequestDispatcher.h>
//#include <Pegasus/Server/CIMOperationResponseEncoder.h>
//#include <Pegasus/Server/CIMOperationRequestDecoder.h>
#include <Pegasus/Common/Linkage.h>
#include <Pegasus/Common/List.h>
#include <Pegasus/Common/Mutex.h>
#include <Pegasus/Common/Time.h>
#include <Pegasus/Common/HashTable.h>

PEGASUS_NAMESPACE_BEGIN

class MessageQueueService;

class PEGASUS_COMMON_LINKAGE cimom : public MessageQueue
{
public:
    cimom();
    virtual ~cimom();

    AsyncOpNode* get_cached_op();
    void cache_op(AsyncOpNode* op);

    Boolean registerCIMService(MessageQueueService *service);
    Boolean deregisterCIMService(MessageQueueService *service);

    typedef HashTable <MessageQueueService*, Boolean, EqualFunc <void*>,
        HashFunc <void*> > RegisteredServicesTable;

protected:
      static void _make_response(Message* req, Uint32 code);
      static void _completeAsyncResponse(
          AsyncRequest* request,
          AsyncReply* reply);
      static void _complete_op_node(AsyncOpNode* op);

private:
    AsyncQueue<AsyncOpNode> _routed_ops;

    static ThreadReturnType PEGASUS_THREAD_CDECL _routing_proc(void*);

    Thread _routing_thread;

    void _handle_cimom_op(AsyncOpNode* op);

    virtual void handleEnqueue();

    Boolean route_async(AsyncOpNode* operation);

    inline Boolean _monitorCIMService(MessageQueueService *service);
    inline void _releaseCIMService(MessageQueueService *service);

    AtomicInt _die;
    AtomicInt _routed_queue_shutdown;

    static cimom *_global_this;


    static RegisteredServicesTable _registeredServicesTable;
    static Mutex _registeredServicesTableLock;

    friend class MessageQueueService;
};

PEGASUS_NAMESPACE_END

#endif // Pegasus_Cimom_h
