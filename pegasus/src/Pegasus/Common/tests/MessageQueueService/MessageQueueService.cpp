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


#include <sys/types.h>
#include <iostream>
#include <stdio.h>
#include <string.h>

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/PegasusAssert.h>
#include <Pegasus/Common/InternalException.h>
#include <Pegasus/Common/MessageQueue.h>
#include <Pegasus/Common/MessageQueueService.h>
#include <Pegasus/Common/Thread.h>
#include <Pegasus/Common/Array.h>
#include <Pegasus/Common/AsyncOpNode.h>
#include <Pegasus/Common/CimomMessage.h>
#include <Pegasus/Common/Cimom.h>

PEGASUS_USING_STD;
PEGASUS_USING_PEGASUS;

static char * verbose;

class TestRequestMessage : public AsyncRequest
{
public:
    typedef AsyncRequest Base;

    TestRequestMessage(
        AsyncOpNode *op,
        Uint32 destination,
        Uint32 response,
        const char *message)
    : Base(
        CIM_DELETE_CLASS_REQUEST_MESSAGE,
        0,
        op,
        destination),
      greeting(message)
    {
    }

    virtual ~TestRequestMessage()
    {
    }

    String greeting;
};


class TestResponseMessage : public AsyncReply
{
public:
    typedef AsyncReply Base;

    TestResponseMessage(
        AsyncOpNode *op,
        Uint32 result,
        Uint32 destination,
        const char *message)
    : Base(
        CIM_DELETE_CLASS_RESPONSE_MESSAGE,
        0,
        op,
        result),
      greeting(message)
    {
    }

    virtual ~TestResponseMessage()
    {
    }

    String greeting;
};


class MessageQueueServer : public MessageQueueService
{
public:
    typedef MessageQueueService Base;
    MessageQueueServer(const char *name)
    : Base(name),
      dienow(0)
    {
    }

    virtual ~MessageQueueServer()
    {
    }

    virtual void _handle_incoming_operation(AsyncOpNode *operation);

    virtual void handleEnqueue()
    {
        // This method is pure abstract in the superclass
        PEGASUS_TEST_ASSERT(0);
    }

    virtual void handleEnqueue(Message* msg)
    {
        // This method is pure abstract in the superclass
        PEGASUS_TEST_ASSERT(0);
    }

    void handleTestRequestMessage(AsyncRequest *msg);
    virtual void handleCimServiceStop(CimServiceStop *req);
    virtual void _handle_async_request(AsyncRequest *req);
    void handleLegacyOpStart(AsyncLegacyOperationStart *req);
    AtomicInt dienow;
};


class MessageQueueClient : public MessageQueueService
{

   public:
      typedef MessageQueueService Base;

      MessageQueueClient(const char *name)
         : Base(name),
           client_xid(1)
      {
      }

      virtual ~MessageQueueClient()
      {
      }

      virtual void handleEnqueue()
      {
          // This method is pure abstract in the superclass
          PEGASUS_TEST_ASSERT(0);
      }

      virtual void handleEnqueue(Message* msg)
      {
          // This method is pure abstract in the superclass
          PEGASUS_TEST_ASSERT(0);
      }

      void sendTestRequestMessage(const char *greeting, Uint32 qid);
      Uint32 get_qid();

      virtual void _handle_async_request(AsyncRequest *req);
      AtomicInt client_xid;
};

AtomicInt msg_count;
AtomicInt client_count;
AtomicInt legacyCount;


Uint32 MessageQueueClient::get_qid()
{
    return _queueId;
}

void MessageQueueServer::_handle_incoming_operation(AsyncOpNode *operation)
{
    if (operation != 0)
    {
        Message* rq = operation->getRequest();

        PEGASUS_TEST_ASSERT(rq != 0);
        if (rq->getMask() & MessageMask::ha_async)
        {
            _handle_async_request(static_cast<AsyncRequest *>(rq));
        }
        else
        {
            if (rq->getType() == CIM_CREATE_CLASS_REQUEST_MESSAGE)
            {
                legacyCount++;
                if (verbose)
                {
                    cout << " caught a hacked legacy message " << endl;
                }
            }
            _complete_op_node(operation);
        }
    }

    return;
}

void MessageQueueServer::_handle_async_request(AsyncRequest *req)
{
    if (req->getType() == CIM_DELETE_CLASS_REQUEST_MESSAGE)
    {
        handleTestRequestMessage(req);
    }
    else if (req->getType() == ASYNC_CIMSERVICE_STOP)
    {
        handleCimServiceStop(static_cast<CimServiceStop *>(req));
    }
    else if (req->getType() == ASYNC_ASYNC_LEGACY_OP_START)
    {
        handleLegacyOpStart(static_cast<AsyncLegacyOperationStart *>(req));
    }
    else
    {
        Base::_handle_async_request(req);
    }
}

void MessageQueueServer::handleLegacyOpStart(AsyncLegacyOperationStart *req)
{
    Message *legacy = req->get_action();

    if (verbose)
    {
        cout << " ### handling legacy messages " << endl;
    }

    AsyncReply *resp =
        new AsyncReply(
            ASYNC_REPLY,
            0,
            req->op,
            async_results::OK);
    _completeAsyncResponse(req, resp);

    if (verbose)
    {
        if (legacy != 0)
            cout << " legacy msg type: " << legacy->getType() << endl;
    }

    delete legacy;
}


void MessageQueueServer::handleTestRequestMessage(AsyncRequest *msg)
{
    if (msg->getType() == CIM_DELETE_CLASS_REQUEST_MESSAGE)
    {
        TestResponseMessage *resp = new TestResponseMessage(
            msg->op,
            async_results::OK,
            msg->dest,
            "i am a test response");
       _completeAsyncResponse(msg, resp);
   }
}

void MessageQueueServer::handleCimServiceStop(CimServiceStop *req)
{
    AsyncReply *resp =
        new AsyncReply(
            ASYNC_REPLY,
            0,
            req->op,
            async_results::CIM_SERVICE_STOPPED);

    _completeAsyncResponse(req, resp);

    if (verbose)
    {
        cout << "recieved STOP from test client" << endl;
    }

    dienow++;
}


void MessageQueueClient::_handle_async_request(AsyncRequest *req)
{
    Base::_handle_async_request(req);
}

void MessageQueueClient::sendTestRequestMessage(
    const char *greeting,
    Uint32 qid)
{
    TestRequestMessage *req =
        new TestRequestMessage(
            0,
            qid,
            _queueId,
            greeting);

    AsyncMessage *response = SendWait(req);
    if (response != 0)
    {
        msg_count++;
        delete response;
        if (verbose)
        {
            cout << " test message " << msg_count.get() << endl;
        }
    }
    delete req;
}


ThreadReturnType PEGASUS_THREAD_CDECL client_func(void *parm);
ThreadReturnType PEGASUS_THREAD_CDECL server_func(void *parm);

int main(int, char **argv)
{
    verbose = getenv("PEGASUS_TEST_VERBOSE");

    try
    {
        Thread client(client_func, (void *)&msg_count, false);
        Thread another(client_func, (void *)&msg_count, false);
        Thread a_third(client_func, (void *)&msg_count, false);

        Thread server(server_func, (void *)&msg_count, false);

        server.run();
        client.run();
        another.run();
        a_third.run();

        while (msg_count.get() < 1500)
        {
            Threads::sleep(10);
        }

        a_third.join();
        another.join();
        client.join();
        server.join();
    }
    catch (Exception& e)
    {
        cout << "Exception: " << e.getMessage() << endl;
        exit(1);
    }
    catch (...)
    {
        cout << "Caught unknown exception" << endl;
        exit(1);
    }

    if (verbose)
    {
        cout << "exiting main " << endl;
    }

    cout << argv[0] << " +++++ passed all tests" << endl;

    return 0;
}


ThreadReturnType PEGASUS_THREAD_CDECL client_func(void *parm)
{
    Thread* my_handle = reinterpret_cast<Thread *>(parm);
    AtomicInt * count = reinterpret_cast<AtomicInt *>(my_handle->get_parm());
    PEGASUS_TEST_ASSERT(0 != count);

    char name_buf[128];

    sprintf(name_buf, "test client %s", Threads::id().buffer);

    MessageQueueClient *q_client = new MessageQueueClient(name_buf);

    client_count++;
    while (client_count.get() < 3)
        Threads::yield();

    MessageQueue *serverQueue = 0;
    Array<Uint32> services;

    while (serverQueue == 0)
    {
        serverQueue = MessageQueue::lookup("test server");
        Threads::yield();
    }

    if (verbose)
    {
        cout << "found server at " << serverQueue->getQueueId() << endl;
    }

    while (msg_count.get() < 1500)
    {
        q_client->sendTestRequestMessage("i am the test client" ,
            serverQueue->getQueueId());
    }
    // now that we have sent and received all of our responses, tell
    // the server thread to stop

    AsyncMessage *reply;

    if (verbose)
    {
        cout << " sending LEGACY to test server" << endl;
    }

    Message *legacy = new Message(CIM_CREATE_CLASS_REQUEST_MESSAGE);

    AsyncLegacyOperationStart *req = new AsyncLegacyOperationStart(
        0,
        serverQueue->getQueueId(),
        legacy);
    reply = q_client->SendWait(req);
    delete req;
    delete reply;

    if (verbose)
    {
        cout << "trying SendForget " << endl;
    }

    legacy = new Message(CIM_CREATE_CLASS_REQUEST_MESSAGE);

    req = new AsyncLegacyOperationStart(
        0,
        serverQueue->getQueueId(),
        legacy);

    q_client->SendForget(req);

    legacy = new Message(CIM_CREATE_CLASS_REQUEST_MESSAGE);
    legacy->dest = serverQueue->getQueueId();

    q_client->SendForget(legacy);

    // Wait untill all legacy messages are processed.
    while (legacyCount.get() < 3)
    {
        Threads::yield();
    }

    MessageQueueService * server = static_cast<MessageQueueService *>(
        MessageQueue::lookup(serverQueue->getQueueId()));
    PEGASUS_TEST_ASSERT(0 != server);

    if (verbose)
    {
        cout << "sending STOP to test server" << endl;
    }

    CimServiceStop *stop = new CimServiceStop(
        0,
        serverQueue->getQueueId());

    reply = q_client->SendWait(stop);
    delete stop;
    delete reply;

    if (verbose)
    {
        cout << "deregistering client qid " << q_client->getQueueId() << endl;
    }

    if (verbose)
    {
        cout << " deleting client " << endl ;
    }

    delete q_client;

    if (verbose)
    {
        cout << " exiting " << endl;
    }

    return ThreadReturnType(0);
}


ThreadReturnType PEGASUS_THREAD_CDECL server_func(void *parm)
{
    MessageQueueServer *q_server = new MessageQueueServer("test server") ;

    while (q_server->dienow.get()  < 3)
    {
        Threads::yield();
    }

    if (verbose)
    {
        cout << " deleting server " << endl;
    }

    delete q_server;

    if (verbose)
    {
        cout << "exiting server " << endl;
    }

    return ThreadReturnType(0);
}
