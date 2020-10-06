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


#include "async_callback.h"
#include <Pegasus/Common/Tracer.h>
PEGASUS_USING_STD;
PEGASUS_USING_PEGASUS;


static Boolean verbose;

async_start::async_start (AsyncOpNode * op, Uint32 start_q, Uint32 completion_q,
                          Message * op_data):Base (
      op,
      start_q, op_data)
{

}

async_complete::async_complete (const async_start & start,
                                Uint32 result, Message * result_data):
Base (start.op, result),
_result_data (result_data)
{
}


Message *
async_complete::get_result_data (void)
{
  Message *ret = _result_data;
  _result_data = 0;
  ret->put_async (0);
  return ret;
}


AtomicInt test_async_queue::msg_count (0);

test_async_queue::test_async_queue (ROLE role):Base (
            (role == CLIENT) ? "client" : "server"),
            _die_now (0), _role (role)
{

}

void
test_async_queue::_handle_async_request (AsyncRequest * rq)
{
  if ((rq->getType () == ASYNC_ASYNC_OP_START) ||
      (rq->getType () == ASYNC_ASYNC_LEGACY_OP_START))
    {
     try {
      PEGASUS_TEST_ASSERT (_role == SERVER);
      Message *response_data =
        new Message (CIM_GET_INSTANCE_RESPONSE_MESSAGE);

      // DO NOT remove the new, the constructor is putting the object itself
      // into a linked list
      new async_complete (static_cast < async_start & >(*rq),
                            async_results::OK,
                            response_data);
      _complete_op_node (rq->op);
    } catch (const PEGASUS_STD(bad_alloc) &)
    {
        cerr <<" Out of memory!" << endl;
    }
    }
  else if (rq->getType () == ASYNC_CIMSERVICE_STOP)
    {
      PEGASUS_TEST_ASSERT (_role == SERVER);
      _handle_stop ((CimServiceStop *) rq);
    }
  else
    Base::_handle_async_request (rq);
}


void
test_async_queue::_handle_async_callback (AsyncOpNode * op)
{
  PEGASUS_TEST_ASSERT (_role == CLIENT);

  PEGASUS_TEST_ASSERT (op->getState() & ASYNC_OPSTATE_COMPLETE);

  Base::_handle_async_callback (op);
}

void
test_async_queue::async_handleEnqueue (AsyncOpNode * op,
                                       MessageQueue * q, void *parm)
{

  async_start *rq = static_cast < async_start * >(op->removeRequest());
  PEGASUS_TEST_ASSERT(rq != 0);

  async_complete *rp = static_cast < async_complete * >(op->removeResponse());
  PEGASUS_TEST_ASSERT(rp != 0);

  if ((rq->getType () == ASYNC_ASYNC_OP_START) &&
      (rp->getType () == ASYNC_ASYNC_OP_RESULT))
    {
      Message *cim_rq = rq->get_action ();
      Message *cim_rp = rp->get_result_data ();

      PEGASUS_TEST_ASSERT (cim_rq->getType () ==
                           CIM_GET_INSTANCE_REQUEST_MESSAGE);
      PEGASUS_TEST_ASSERT (cim_rp->getType () ==
                      CIM_GET_INSTANCE_RESPONSE_MESSAGE);
      test_async_queue::msg_count++;

      delete cim_rp;
      delete cim_rq;
      delete rp;
      delete rq;
      delete op;
    }
}


void
test_async_queue::async_handleSafeEnqueue (Message * msg,
                                           void *handle, void *parameter)
{
  test_async_queue::msg_count++;
  delete msg;
}

void
test_async_queue::_handle_stop (CimServiceStop * stop)
{
  try {
  AsyncReply *resp = new AsyncReply (ASYNC_REPLY,
                                     0,
                                     stop->op,
                                     async_results::CIM_SERVICE_STOPPED);
  _completeAsyncResponse (stop, resp);
  _die_now = 1;
  } catch (const PEGASUS_STD(bad_alloc) &) {
    cerr << "Out of memory in _handle_stop. Continuing tests .. " << endl;
  }
}

ThreadReturnType PEGASUS_THREAD_CDECL client_func (void *parm);
ThreadReturnType PEGASUS_THREAD_CDECL server_func (void *parm);

int main (int, char **argv)
{
  verbose = getenv ("PEGASUS_TEST_VERBOSE") ? true : false;

#ifdef PEGASUS_DEBUG
  if (verbose)
  {
    Tracer::setTraceLevel(4);
    Tracer::setTraceComponents("DiscardedData");
    Tracer::setTraceFile("async_callback.trc");
  }
#endif
  try
  {
    Thread client (client_func, (void *) 0, false);

    Thread server (server_func, (void *) 0, false);

    server.run ();
    client.run ();


    client.join ();
    server.join ();
  } catch (const Exception & e)
  {
    cout << "Exception: " << e.getMessage () << endl;

  }
  cout << argv[0] << " +++++ passed all tests" << endl;

  return (0);
}


ThreadReturnType PEGASUS_THREAD_CDECL
client_func (void *parm)
{
    test_async_queue *client = new test_async_queue (test_async_queue::CLIENT);

    // find the server
    MessageQueue *serverQueue = 0;
    while (serverQueue == 0)
    {
        serverQueue = MessageQueue::lookup("server");
        // It is a good idea to yield to other threads. You should do this,
        // but this test-case stresses situations in which does not happen.
        //Threads::yield ();
    }

    if (verbose)
    {
        cout << "testing low-level async send " << endl;
    }

    Uint32 requestCount = 0;
    while (requestCount < 10000)
    {
        // The problem on multi-processor machines is that if we make it
        // continue on sending the messages, and the MessageQueueService
        // does not get to pick up the messages, the machine can crawl to
        // halt with about 300-400 threads and ever-continuing number of
        // them created. This is an evil stress test so lets leave it behind.
        try
        {
            Message *cim_rq = new Message (CIM_GET_INSTANCE_REQUEST_MESSAGE);

            AsyncOpNode *op = client->get_op();
            // DO NOT remove the new, the constructor is putting the object
            // itself into a linked list
            new AsyncOperationStart(
                op,
                serverQueue->getQueueId(),
                cim_rq);
            client->SendAsync(
                op,
                serverQueue->getQueueId(),
                test_async_queue::async_handleEnqueue,
                client,
                (void *) 0);
        }
        catch (const PEGASUS_STD(bad_alloc) &)
        {
            cerr << "Out of memory! Continuing tests." << endl;
            continue;
        }
        requestCount++;

        // You really ought to allow other threads to their job (like picking
        // up all of these messages, but we want to stress test unfair
        // circumstances.
        //Threads::yield ();
    }

    if (verbose)
    {
        cout << "Waiting until all messages are flushed." << endl;
    }
    while (test_async_queue::msg_count.get() != requestCount)
    {
        if (verbose)
        {
            if (test_async_queue::msg_count.get() % (requestCount/10) == 0)
            {
                cout << test_async_queue::msg_count.get() / (requestCount/100)
                    << "% complete" << endl;
            }
        }
        Threads::yield();
    }

    if (verbose)
    {
        cout << "Waiting until all messages are flushed. " << endl;
    }
    while (test_async_queue::msg_count.get() != requestCount)
    {
        if (verbose)
        {
            if (test_async_queue::msg_count.get() % (requestCount/10) == 0)
            {
                cout << test_async_queue::msg_count.get() / (requestCount/100)
                    << "% complete" << endl;
            }
        }
        Threads::yield();
    }

    if (verbose)
    {
        cout << "sending stop to server " << endl;
    }
    try
    {
        CimServiceStop *stop = new CimServiceStop(
            0,
            serverQueue->getQueueId());

        AsyncMessage *reply = client->SendWait (stop);
        delete stop;
        delete reply;
    }
    catch (const PEGASUS_STD(bad_alloc) &)
    {
        cerr <<" Out of memory! Continuing tests." << endl;
    }

    // wait for the server to shut down
    while (serverQueue)
    {
        serverQueue = MessageQueue::lookup("server");
        Threads::yield ();
    }

    if (verbose)
    {
        cout << "shutting down client" << endl;
    }

    delete client;
    return ThreadReturnType(0);
}


ThreadReturnType PEGASUS_THREAD_CDECL
server_func (void *parm)
{
  test_async_queue *server = new test_async_queue (test_async_queue::SERVER);

  while (server->_die_now.get () < 1)
    {
      Threads::yield ();
    }
  if (verbose)
    cout << "server shutting down" << endl;

  delete server;

  return ThreadReturnType(0);
}
