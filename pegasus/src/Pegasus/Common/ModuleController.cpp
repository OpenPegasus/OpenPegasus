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

#include "ModuleController.h"
#include <Pegasus/Common/MessageLoader.h>
#include <Pegasus/Common/InternalException.h>

PEGASUS_NAMESPACE_BEGIN

PEGASUS_USING_STD;

RegisteredModuleHandle::RegisteredModuleHandle(
    const String& name,
    void* module_address,
    Message* (*receive_message)(Message *, void *))
    : _name(name),
      _module_address(module_address),
      _module_receive_message(receive_message)
{
    PEGASUS_ASSERT(_module_receive_message != 0);
}

RegisteredModuleHandle::~RegisteredModuleHandle()
{
}

const String & RegisteredModuleHandle::get_name() const
{
    return _name;
}

Message* RegisteredModuleHandle::_receive_message(Message* msg)
{
    return _module_receive_message(msg, _module_address);
}

ModuleController::ModuleController(const char* name)
    : Base(name),
      _modules()
{
}

ModuleController::~ModuleController()
{
    RegisteredModuleHandle* module;

    try
    {
        module = _modules.remove_front();
        while (module)
        {
            delete module;
            module = _modules.remove_front();
        }
    }
    catch (...)
    {
    }
}

void ModuleController::register_module(
    const String& module_name,
    void* module_address,
    Message* (*receive_message)(Message *, void *))
{
    RegisteredModuleHandle *module;
    // see if the module already exists in this controller.
    RegisteredModulesList::AutoLock lock(_modules);
    module = _modules.front();
    while (module != NULL)
    {
        if (module->get_name() == module_name)
        {
            MessageLoaderParms parms(
                "Common.ModuleController.MODULE",
                "module \"$0\"",
                module_name);
            throw AlreadyExistsException(parms);
        }
        module = _modules.next_of(module);
    }
    // the module does not exist, go ahead and create it.
    module = new RegisteredModuleHandle(
        module_name,
        module_address,
        receive_message);
    _modules.insert_back(module);
}

void ModuleController::handleEnqueue(Message *message)
{
    if (message->getType() == CIM_PROCESS_INDICATION_RESPONSE_MESSAGE)
    {
        CIMProcessIndicationResponseMessage *response = 
            (CIMProcessIndicationResponseMessage*)message;
        PEGASUS_ASSERT(!response->oopAgentName.size());
        IndicationRouter::notify(response);
        return;
    }
    PEGASUS_ASSERT(false);
}

void ModuleController::_handle_async_request(AsyncRequest* rq)
{
    if (rq->getType() == ASYNC_ASYNC_MODULE_OP_START)
    {
        // find the target modules
        RegisteredModuleHandle* target;
        Message* module_result = NULL;

        CIMRequestMessage *request =
            static_cast<CIMRequestMessage*>(
                static_cast<AsyncModuleOperationStart *>(rq)->_act);

        MessageType reqType = request->getType();
        if (reqType == CIM_STOP_ALL_PROVIDERS_REQUEST_MESSAGE ||
            reqType == CIM_SUBSCRIPTION_INIT_COMPLETE_REQUEST_MESSAGE ||
            reqType == CIM_INDICATION_SERVICE_DISABLED_REQUEST_MESSAGE)
        {
            // Note: Since control providers are not loaded/unloaded
            // dynamically, and the requests processed here arrives after
            // registering the all control providers, acquiring the lock
            // on modules list is not required.
            // Process the request on each control provider by iterating
            // through modules list. Since called functions  returns null
            // responses for these requests, build the single response after
            //  the request is processed with all control providers.
            target = _modules.front();
            while (target != NULL)
            {
                module_result = target->_receive_message(
                    static_cast<AsyncModuleOperationStart *>(rq)->_act);
                target = _modules.next_of(target);
            }
            module_result = request->buildResponse();
            // Do NOT remove the new operator !!!
            // The constructor of AsyncModuleOperationResult does use its this
            // pointer to put itself into a linked list
            //
            // see _res->put_async(this); in AsyncModuleOperationResult()
            // in file pegasus/src/Pegasus/Common/CimomMessage.cpp
            new AsyncModuleOperationResult(
                rq->op,
                async_results::OK,
                static_cast<AsyncModuleOperationStart *>(rq)->_target_module,
                module_result);
            _complete_op_node(rq->op);
            return;
        }

        {
            RegisteredModulesList::AutoLock lock(_modules);
            target = _modules.front();
            while (target != NULL)
            {
                if (target->get_name() ==
                        static_cast<AsyncModuleOperationStart *>(rq)->
                            _target_module)
                {
                    break;
                }

                target = _modules.next_of(target);
            }
        }

        if (target)
        {
            // ATTN: This statement was taken out of the _module_lock block
            // above because that caused all requests to control providers to
            // be serialized.  There is now a risk that the control provider
            // module may be deleted after the lookup and before this call.
            // See Bugzilla 3120.
            module_result = target->_receive_message(
                static_cast<AsyncModuleOperationStart *>(rq)->_act);
        }

        if (module_result == NULL)
        {
            module_result = new AsyncReply(
                ASYNC_REPLY,
                MessageMask::ha_async | MessageMask::ha_reply,
                rq->op,
                async_results::CIM_NAK);
        }

        // Do NOT remove the new operator !!!
        // The constructor of AsyncModuleOperationResult does use its this
        // pointer to put itself into a linked list
        //
        // see _res->put_async(this); in AsyncModuleOperationResult constructor
        // in file pegasus/src/Pegasus/Common/CimomMessage.cpp
        new AsyncModuleOperationResult(
            rq->op,
            async_results::OK,
            static_cast<AsyncModuleOperationStart *>(rq)->_target_module,
            module_result);
        _complete_op_node(rq->op);
    }
    else
        Base::_handle_async_request(rq);
}

void ModuleController::_handle_async_callback(AsyncOpNode* op)
{
    Base::_handle_async_callback(op);
}

ModuleController* ModuleController::getModuleController()
{
    MessageQueue* messageQueue =
        MessageQueue::lookup(PEGASUS_QUEUENAME_CONTROLSERVICE);
    PEGASUS_ASSERT(messageQueue != 0);

    MessageQueueService* service =
        dynamic_cast<MessageQueueService*>(messageQueue);
    PEGASUS_ASSERT(service != 0);

    return static_cast<ModuleController*>(service);
}

AsyncReply* ModuleController::ClientSendWait(
    Uint32 destination_q,
    AsyncRequest* request)
{
    request->dest = destination_q;
    AsyncReply* reply = Base::SendWait(request);
    return reply;
}

Boolean ModuleController::ClientSendForget(
    Uint32 destination_q,
    AsyncRequest* message)
{
    message->dest = destination_q;
    return SendForget(message);
}

void ModuleController::_indicationDeliveryRoutine(
    CIMProcessIndicationRequestMessage* request)
{
    if (!request->operationContext.contains(AcceptLanguageListContainer::NAME))
    {
        request->operationContext.insert(
            AcceptLanguageListContainer(AcceptLanguageList()));
    }

    ModuleController *mc = getModuleController();

    Uint32 _indicationServiceQueueId = mc->find_service_qid(
        PEGASUS_QUEUENAME_INDICATIONSERVICE);

    request->queueIds = QueueIdStack(
        mc->getQueueId(), _indicationServiceQueueId);

    AsyncLegacyOperationStart * asyncRequest =
        new AsyncLegacyOperationStart(
        0,
        _indicationServiceQueueId,
        request);

    mc->SendForget(asyncRequest);
}

void ModuleController::indicationCallback(
    CIMProcessIndicationRequestMessage* request)
{
    IndicationRouter router =
        IndicationRouter(request, _indicationDeliveryRoutine);

    router.deliverAndWaitForStatus();
}

PEGASUS_NAMESPACE_END
