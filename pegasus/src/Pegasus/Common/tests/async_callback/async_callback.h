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
//%////////////////////////////////////////////////////////////////////////////

#ifndef Pegasus_async_callback_h
#define Pegasus_async_callback_h

#include <Pegasus/Common/Config.h>
#include <sys/types.h>
#if !defined(PEGASUS_OS_TYPE_WINDOWS)
#include <unistd.h>
#endif
#include <Pegasus/Common/PegasusAssert.h>
#include <iostream>
#include <stdio.h>
#include <string.h>

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/InternalException.h>
#include <Pegasus/Common/MessageQueue.h>
#include <Pegasus/Common/Thread.h>
#include <Pegasus/Common/Array.h>
#include <Pegasus/Common/AsyncOpNode.h>
#include <Pegasus/Common/CimomMessage.h>
#include <Pegasus/Common/Cimom.h>
#include <Pegasus/Common/MessageQueueService.h>

PEGASUS_NAMESPACE_BEGIN


class test_async_queue;

class async_start : public AsyncOperationStart
{
   public:
      typedef AsyncOperationStart Base;

      async_start(AsyncOpNode *op,
          Uint32 start_q,
          Uint32 completion_q,
          Message *op_data);

      virtual ~async_start(void) { }


   private:
      friend class test_async_queue;
};


class async_complete: public AsyncOperationResult
{   public:
      typedef AsyncOperationResult Base;

      async_complete(const async_start & start_op,
             Uint32 result,
             Message *result_data);

      virtual ~async_complete(void) {    delete _result_data; }

      Message *get_result_data(void);

   private:
      Message *_result_data;
      friend class test_async_queue;
};


class test_async_queue : public MessageQueueService
{
   public:
      typedef MessageQueueService Base;

      enum ROLE
      {
     CLIENT,
     SERVER
      };

      test_async_queue(ROLE role);
      virtual ~test_async_queue(void) { }

      // pure virtuals that will go away eventually
      void handleEnqueue(void)
      {
     Message *msg = dequeue();
     if(msg)
        handleEnqueue(msg);
      }

      void handleEnqueue(Message *msg)
      {
     delete msg;
      }

      // static callback function
      static void async_handleEnqueue(AsyncOpNode *op, MessageQueue *, void *);
      static void async_handleSafeEnqueue(Message *, void *, void *);

      static AtomicInt msg_count;
      AtomicInt _die_now;
   protected:
      virtual void _handle_async_request(AsyncRequest *req);
      virtual void _handle_async_callback(AsyncOpNode *operation);


   private:
      test_async_queue(void);
      void _handle_stop(CimServiceStop *stop);

      ROLE _role;
};



PEGASUS_NAMESPACE_END

#endif // Pegasus_async_callback_h
