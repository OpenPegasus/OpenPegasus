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
#include <Pegasus/Common/PegasusVersion.h>
#include <Pegasus/Common/Tracer.h>
#include <Pegasus/Server/CIMOperationRequestDispatcher.h>

#include "CIMExportRequestDispatcher.h"

PEGASUS_USING_STD;

PEGASUS_NAMESPACE_BEGIN

CIMExportRequestDispatcher::CIMExportRequestDispatcher()
   : Base(PEGASUS_QUEUENAME_EXPORTREQDISPATCHER)
{
   PEG_METHOD_ENTER(TRC_EXP_REQUEST_DISP,
      "CIMExportRequestDispatcher::CIMExportRequestDispatcher");

   PEG_METHOD_EXIT();
}

CIMExportRequestDispatcher::~CIMExportRequestDispatcher()
{
   PEG_METHOD_ENTER(TRC_EXP_REQUEST_DISP,
      "CIMExportRequestDispatcher::~CIMExportRequestDispatcher");

   PEG_METHOD_EXIT();
}

void CIMExportRequestDispatcher::_handle_async_request(AsyncRequest *req)
{
   PEG_METHOD_ENTER(TRC_EXP_REQUEST_DISP,
      "CIMExportRequestDispatcher::_handle_async_request");

    PEGASUS_ASSERT(req != 0 && req->op != 0);

    if (req->getType() == ASYNC_CIMSERVICE_STOP)
    {
        handle_CimServiceStop(static_cast<CimServiceStop *>(req));
    }
    else if (req->getType() == ASYNC_ASYNC_LEGACY_OP_START)
    {
        Message *legacy =
            (static_cast<AsyncLegacyOperationStart *>(req)->get_action());
        if (legacy->getType() == CIM_EXPORT_INDICATION_REQUEST_MESSAGE)
        {
            Message* legacy_response = _handleExportIndicationRequest(
                (CIMExportIndicationRequestMessage*) legacy);
            // constructor puts itself into a linked list, DO NOT remove the new
            new AsyncLegacyOperationResult(req->op, legacy_response);

            _complete_op_node(req->op);
            delete legacy;
        }
        else
        {
            PEG_TRACE((TRC_DISCARDED_DATA, Tracer::LEVEL1,
                "CIMExportRequestDispatcher::_handle_async_request got "
                    "unexpected legacy message type '%s'",
                MessageTypeToString(legacy->getType())));
            _make_response(req, async_results::CIM_NAK);
            delete legacy;
        }
    }
    else
    {
        Base::_handle_async_request(req);
    }
    PEG_METHOD_EXIT();
}

void CIMExportRequestDispatcher::handleEnqueue(Message* message)
{
    PEG_METHOD_ENTER(TRC_EXP_REQUEST_DISP,
        "CIMExportRequestDispatcher::handleEnqueue");

    PEGASUS_ASSERT(message != 0);

    switch (message->getType())
    {
        case CIM_EXPORT_INDICATION_REQUEST_MESSAGE:
        {
            CIMExportIndicationResponseMessage* response =
                _handleExportIndicationRequest(
                    (CIMExportIndicationRequestMessage*) message);

            PEG_TRACE((
                TRC_HTTP,
                Tracer::LEVEL4,
                "_CIMExportRequestDispatcher::handleEnqueue(message) - "
                    "message->getCloseConnect() returned %d",
                message->getCloseConnect()));

            response->setCloseConnect(message->getCloseConnect());

            MessageQueue* queue = MessageQueue::lookup(response->dest);
            PEGASUS_ASSERT(queue != 0);

            queue->enqueue(response);
            break;
        }

        default:
            PEGASUS_UNREACHABLE(PEGASUS_ASSERT(0);)
            break;
    }
    delete message;

    PEG_METHOD_EXIT();
}

void CIMExportRequestDispatcher::handleEnqueue()
{
    PEG_METHOD_ENTER(TRC_EXP_REQUEST_DISP,
        "CIMExportRequestDispatcher::handleEnqueue");

    // It is important to handle the enqueued message on a separate thread,
    // because this method is likely to be processing on a central (Monitor)
    // thread and handling the message will likely include a call to a
    // provider.  The thread is launched here rather than at a lower level
    // because async messages are handled differently and the
    // _handleExportIndicationRequest message does not have enough context
    // to manage the difference.

    ThreadStatus rtn = PEGASUS_THREAD_OK;
    while ((rtn = _thread_pool->allocate_and_awaken(
                      (void *)this,
                      CIMExportRequestDispatcher::_handleEnqueueOnThread)) !=
               PEGASUS_THREAD_OK)
    {
        if (rtn != PEGASUS_THREAD_INSUFFICIENT_RESOURCES)
        {
            PEG_TRACE((TRC_DISCARDED_DATA, Tracer::LEVEL1,
                "Could not allocate thread for %s.",
                getQueueName()));
            break;
        }

        Threads::yield();
    }

    PEG_METHOD_EXIT();
}

// Note: This method should not throw an exception.  It is used as a thread
// entry point, and any exceptions thrown are ignored.
ThreadReturnType PEGASUS_THREAD_CDECL
CIMExportRequestDispatcher::_handleEnqueueOnThread(void* arg)
{
    PEG_METHOD_ENTER(TRC_EXP_REQUEST_DISP,
        "CIMExportRequestDispatcher::_handleEnqueueOnThread");

    PEGASUS_ASSERT(arg != 0);

    CIMExportRequestDispatcher* dispatcher =
        reinterpret_cast<CIMExportRequestDispatcher*>(arg);
    PEGASUS_ASSERT(dispatcher != 0);

    Message* message = dispatcher->dequeue();
    if (message)
    {
        dispatcher->handleEnqueue(message);
    }

    PEG_METHOD_EXIT();
    return ThreadReturnType(0);
}

CIMExportIndicationResponseMessage*
CIMExportRequestDispatcher::_handleExportIndicationRequest(
    CIMExportIndicationRequestMessage* request)
{
    PEG_METHOD_ENTER(TRC_EXP_REQUEST_DISP,
        "CIMExportRequestDispatcher::_handleExportIndicationRequest");

    OperationContext context;

    CIMException cimException;

    Uint32 serviceId =find_service_qid(PEGASUS_QUEUENAME_PROVIDERMANAGER_CPP);

    PEG_TRACE ((TRC_INDICATION_RECEIPT, Tracer::LEVEL4,
        "%s Indication %s received in export server for destination %s",
         (const char*)(request->indicationInstance.getClassName().getString().
         getCString()),
         (const char*)(request->messageId.getCString()),
         (const char*)(request->destinationPath.getCString())));
    AsyncOpNode* op = this->get_op();

    AsyncLegacyOperationStart * asyncRequest =
        new AsyncLegacyOperationStart(
            op,
            serviceId,
            new CIMExportIndicationRequestMessage(*request));

    asyncRequest->dest = serviceId;

    //SendAsync(op,
    //          serviceIds[0],
    //          CIMExportRequestDispatcher::_forwardRequestCallback,
    //          this,
    //          (void *)request->queueIds.top());
    AsyncReply *asyncReply = SendWait(asyncRequest);

    CIMExportIndicationResponseMessage* response =
        reinterpret_cast<CIMExportIndicationResponseMessage *>(
            (static_cast<AsyncLegacyOperationResult *>(
                asyncReply))->get_result());
    response->dest = request->queueIds.top();

    delete asyncReply;    // Recipient deletes request
    this->return_op(op);

    PEG_METHOD_EXIT();
    return response;
}

PEGASUS_NAMESPACE_END
