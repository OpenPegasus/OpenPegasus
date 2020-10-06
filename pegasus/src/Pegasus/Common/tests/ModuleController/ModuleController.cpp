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


#include <Pegasus/Common/Config.h>
#include <sys/types.h>
#if defined(PEGASUS_PLATFORM_WIN64_IA64_MSVC) || \
    defined(PEGASUS_PLATFORM_WIN64_X86_64_MSVC) || \
    defined(PEGASUS_PLATFORM_WIN32_IX86_MSVC)
#else
#include <unistd.h>
#endif
#include <Pegasus/Common/PegasusAssert.h>
#include <Pegasus/Common/Threads.h>
#include <iostream>
#include <stdio.h>
#include <string.h>
#include <Pegasus/Common/ModuleController.h>
#include <Pegasus/Common/CIMMessage.h>

PEGASUS_USING_STD;
PEGASUS_USING_PEGASUS;

#define CONTROLLER_NAME "ControlService"
#define TESTSERVICE_NAME "TestService"

class TestService;
class TestModuleMessageHandler;

AtomicInt responsesReceived;
ModuleController *controlService;
TestService *testService;
Array<TestModuleMessageHandler*> registeredModules;
const Uint32 REQUESTS_ISSUED = 50;

class TestResponseMessage : public CIMResponseMessage
{
public:
    TestResponseMessage(
        const String& messageId_,
        const CIMException& cimException_,
        Uint32 id_,
        const QueueIdStack &queueIds_)
    : CIMResponseMessage(DUMMY_MESSAGE,
        messageId_,
        cimException_,
        queueIds_),
      id(id_)
    {
    }
    Uint32 id;
};

class TestRequestMessage : public CIMRequestMessage
{
public:
    TestRequestMessage(
        const String& messageId_,
        Uint32 id_,
        const QueueIdStack &queueIds_)
    : CIMRequestMessage(DUMMY_MESSAGE,
        messageId_,
        queueIds_),
      id(id_)
    {
    }
    CIMResponseMessage *buildResponse() const
    {
        TestResponseMessage *response = new TestResponseMessage(
            messageId,
            CIMException(),
            id,
            queueIds.copyAndPop());
        return response;
    }
    Uint32 id;
};


class TestService : public MessageQueueService
{
public:
    typedef MessageQueueService Base;
    TestService(const char *name);
    virtual ~TestService();
    static void _testServiceCallback(
        AsyncOpNode* op,
        MessageQueue* q,
        void* userParameter);
    virtual void handleEnqueue() {}
    virtual void handleEnqueue(Message *msg) {}
private:
    TestService();
    TestService(const TestService &);
    TestService & operator =(const TestService &);
};

TestService::TestService(const char *name)
         : Base(name)
{
}

TestService::~TestService()
{
}

void TestService::_testServiceCallback(
    AsyncOpNode* op,
    MessageQueue* q,
    void* userParameter)
{
    TestService* service =
        static_cast<TestService*>(q);

    AsyncRequest* asyncRequest = static_cast<AsyncRequest*>(op->getRequest());
    AsyncReply* asyncReply = static_cast<AsyncReply*>(op->removeResponse());
    TestResponseMessage* response = 0;
    MessageType msgType = asyncReply->getType();
    if (msgType == ASYNC_ASYNC_MODULE_OP_RESULT)
    {
        response = reinterpret_cast<TestResponseMessage*>(
            (static_cast<AsyncModuleOperationResult*>(asyncReply))->
                get_result());
    }
    else
    {
        PEGASUS_TEST_ASSERT(0);
    }
    delete asyncReply;
    op->removeRequest();
    delete asyncRequest;
    service->return_op(op);
    delete response;
    responsesReceived++;
}

class TestModuleMessageHandler
{
public:
    TestModuleMessageHandler(const String moduleName): moduleName(moduleName)
    {
    }
    ~TestModuleMessageHandler()
    {
    }
    Message* processMessage(Message *msg);
private:
    String moduleName;
};

Message* TestModuleMessageHandler::processMessage(Message *msg)
{
    CIMRequestMessage *request = dynamic_cast<CIMRequestMessage*>(msg);
    PEGASUS_TEST_ASSERT(request);
    CIMResponseMessage* response = 0;
    if (msg->getType() ==  DUMMY_MESSAGE)
    {
        response = request->buildResponse();
    }
    return response;
}

static Message* receiveMessageCallback(
    Message* message,
    void* instance)
{
    TestModuleMessageHandler* mh =
        reinterpret_cast<TestModuleMessageHandler*>(instance);

    return mh->processMessage(message);
}


void testRegisterModule(const char *moduleName)
{
    // Register the module with control Service.
    TestModuleMessageHandler *module =
        new TestModuleMessageHandler(moduleName);
    registeredModules.append(module);
    controlService->register_module(
        moduleName,
        module,
        receiveMessageCallback);

    // Try registering the same module, should get exception.
    try
    {
        controlService->register_module(
            moduleName,
            module,
            receiveMessageCallback);
        PEGASUS_TEST_ASSERT(0);
    }
    catch(AlreadyExistsException&)
    {
    }
}

void _testModuleController()
{
    Uint32 cqid = controlService->getQueueId();
    Uint32 tqid = testService->getQueueId();

    String testModule1("testmodule1");
    String testModule2("testmodule2");
    for (Uint32 i = 1; i <= REQUESTS_ISSUED ; ++i)
    {
        TestRequestMessage *request = new TestRequestMessage(
            String::EMPTY,
            i,
            QueueIdStack(cqid,tqid));

        AsyncOpNode *op = controlService->get_op();

        AsyncModuleOperationStart* mReq = new AsyncModuleOperationStart(
            op,
            cqid,
            i % 2 ? testModule1 : testModule2,
            request);

        PEGASUS_TEST_ASSERT(0 != mReq);

        testService->SendAsync(
            op,
            cqid,
            TestService::_testServiceCallback,
            testService,
            0);
    }
}

int main()
{
    controlService = new ModuleController(CONTROLLER_NAME);
    testService = new TestService(TESTSERVICE_NAME);
    cout << "controller queue id " << controlService->getQueueId() << endl;
    cout << "test service queue id " << testService->getQueueId() << endl;

    cout << "testing module registration " << endl;
    testRegisterModule("testmodule1");
    testRegisterModule("testmodule2");


    cout << "testing module message dispatch " << endl;
    _testModuleController();

    // wait until we get all responses.
    while (REQUESTS_ISSUED != responsesReceived.get())
    {
        Threads::sleep(10);
    }

    delete controlService;
    delete testService;

    cout << "deleting modules " << endl;
    for (Uint32 i = 0, n = registeredModules.size() ; i < n ; ++i)
    {
        delete registeredModules[i];
    }

    return 0;
}

