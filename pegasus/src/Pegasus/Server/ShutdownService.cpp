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


/////////////////////////////////////////////////////////////////////////////
//  ShutdownService
/////////////////////////////////////////////////////////////////////////////

#include <Pegasus/Common/Config.h>
#include <Pegasus/Server/ShutdownExceptions.h>
#include <Pegasus/Server/CIMServerState.h>
#include <Pegasus/Server/ShutdownService.h>
#include <Pegasus/Common/XmlWriter.h>
#include <Pegasus/Common/Message.h>
#include <Pegasus/Common/CimomMessage.h>
#include <Pegasus/Common/CIMMessage.h>
#include <Pegasus/Common/MessageQueueService.h>
#include <Pegasus/Common/Tracer.h>

#if defined(PEGASUS_OS_TYPE_UNIX)
# include <sys/types.h>
#endif

PEGASUS_USING_STD;

PEGASUS_NAMESPACE_BEGIN

/**
    The constant representing the shutdown timeout property name
*/
static String SHUTDOWN_TIMEOUT_PROPERTY = "shutdownTimeout";

/**
    Initialize ShutdownService instance
*/
ShutdownService* ShutdownService::_instance = 0;

/**
    Initialize all other class variables
*/
CIMServer* ShutdownService::_cimserver = 0;
Uint32 ShutdownService::_shutdownTimeout = 0;
ModuleController* ShutdownService::_controller = 0;

/** Constructor. */
ShutdownService::ShutdownService(CIMServer* cimserver)
{
    _cimserver = cimserver;

    //
    // get module controller
    //
    _controller = ModuleController::getModuleController();
}

/**
    Terminates the shutdown service
*/
void ShutdownService::destroy()
{
    delete _instance;
    _instance = 0;
}

/** Destructor. */
ShutdownService::~ShutdownService()
{
}

/**
   return a pointer to the ShutdownService instance.
*/
ShutdownService* ShutdownService::getInstance(CIMServer* cimserver)
{
    if (!_instance)
    {
        _instance = new ShutdownService(cimserver);
    }
    return _instance;
}

/**
    The shutdown method to be called by the ShutdownProvider to
    process a shutdown request from the cimcli client.
*/
void ShutdownService::shutdown(
    Boolean force,
    Uint32 timeout,
    Boolean requestPending)
{
    PEG_METHOD_ENTER(TRC_SHUTDOWN, "ShutdownService::shutdown");

    _shutdownTimeout = timeout;

    try
    {
        //
        // set CIMServer state to TERMINATING
        //
        _cimserver->setState(CIMServerState::TERMINATING);

        PEG_TRACE_CSTRING(
            TRC_SHUTDOWN,
            Tracer::LEVEL4,
            "ShutdownService::shutdown - CIM server state set to "
                "CIMServerState::TERMINATING");

        //
        // Tell the CIMServer to stop accepting new client connection requests.
        //
        _cimserver->stopClientConnection();

        PEG_TRACE_CSTRING(
            TRC_SHUTDOWN,
            Tracer::LEVEL4,
            "ShutdownService::shutdown - No longer accepting new client "
                "connection requests.");

        //
        // Determine if there are any outstanding CIM operation requests
        // (take into account that one of the request is the shutdown request).
        //
        waitUntilNoMoreRequests(requestPending);

        //
        // Send a shutdown signal to the CIMServer. CIMServer itself will take
        // care of shutting down the CimomServices and deleting them. In other
        // words, _DO_ _NOT_ call 'shutdownCimomServices' from a provider.
        //
        _cimserver->shutdown();

        PEG_TRACE_CSTRING(TRC_SHUTDOWN, Tracer::LEVEL4,
            "ShutdownService::shutdown - CIMServer instructed to shut down.");
    }
    catch (Exception& e)
    {
        PEG_TRACE((TRC_SHUTDOWN, Tracer::LEVEL2,
            "Error occurred during CIMServer shutdown: %s",
            (const char*)e.getMessage().getCString()));
    }
    catch (...)
    {
        PEG_TRACE_CSTRING(TRC_SHUTDOWN, Tracer::LEVEL2,
            "Unexpected error occurred during CIMServer shutdown. ");
    }

    PEG_METHOD_EXIT();
}

/**********************************************************/
/*  private methods                                       */
/**********************************************************/

void ShutdownService::shutdownCimomServices()
{
    PEG_METHOD_ENTER(TRC_SHUTDOWN, "ShutdownService::shutdownCimomServices");

    //
    // Shutdown the Indication Service
    //
    _sendShutdownRequestToService(PEGASUS_QUEUENAME_INDICATIONSERVICE);

    // Shutdown the Indication Handler Service
    _sendShutdownRequestToService(PEGASUS_QUEUENAME_INDHANDLERMANAGER);

    // PEGASUS_QUEUENAME_EXPORTRESPENCODER
    _sendShutdownRequestToService(PEGASUS_QUEUENAME_EXPORTRESPENCODER);

    //
    // shutdown  CIM Export Request Dispatcher Service
    //
    _sendShutdownRequestToService(PEGASUS_QUEUENAME_EXPORTREQDISPATCHER);

    //
    // shutdown  CIM Operation Request Dispatcher Service
    //
    _sendShutdownRequestToService(PEGASUS_QUEUENAME_OPREQDISPATCHER);

    // shutdown CIM Provider Manager
    _sendShutdownRequestToService(PEGASUS_QUEUENAME_PROVIDERMANAGER_CPP);

    // shutdown ModuleController also called ControlService.
    _sendShutdownRequestToService(PEGASUS_QUEUENAME_CONTROLSERVICE);

    PEG_METHOD_EXIT();
}

void ShutdownService::_sendShutdownRequestToService(const char* serviceName)
{
    MessageQueue *queue = MessageQueue::lookup(serviceName);
    Uint32 _queueId;
    if (queue)
    {
        _queueId =  queue->getQueueId();
    }
    else
    {
        // service not found, just return
        return;
    }
    // send a Stop (this is a legacy message that in some of the MQS does
    // termination of its internal stuff. Then follow it with a Stop (to
    // open up its incoming queue), and then with a AsyncIoClose
    // which closes the incoming queue.

    // All of these messages MUST be sequential. Do not use SendForget or
    // SendAsync as those are asynchronous and their receipt is guaranteed
    // to be undeterministic and possibly out of sequence (which is something
    // we do not want).

    CimServiceStop stop_message(
        NULL,
        _queueId);

    AutoPtr<AsyncReply> StopAsyncReply(
        _controller->ClientSendWait(_queueId, &stop_message));

    CimServiceStart start_message(
        NULL,
        _queueId);

    AutoPtr <AsyncReply> StartAsyncReply(
        _controller->ClientSendWait(_queueId, &start_message));

    AsyncIoClose close_request(
        NULL,
        _queueId);

    AutoPtr <AsyncReply> CloseAsyncReply(
        _controller->ClientSendWait(_queueId, &close_request));
}

void ShutdownService::shutdownProviders()
{
    _shutdownProviders(true);
    _shutdownProviders(false);
}

void ShutdownService::_shutdownProviders(Boolean controlProviders)
{
    PEG_METHOD_ENTER(TRC_SHUTDOWN, "ShutdownService::_shutdownProviders");

    //
    // get provider manager service or control service
    //
    MessageQueue* queue =
        controlProviders ?
            MessageQueue::lookup(PEGASUS_QUEUENAME_CONTROLSERVICE) :
            MessageQueue::lookup(PEGASUS_QUEUENAME_PROVIDERMANAGER_CPP);

    if (queue == 0)
    {
        PEG_METHOD_EXIT();
        return;
    }

    MessageQueueService* _service = dynamic_cast<MessageQueueService*>(queue);
    PEGASUS_ASSERT(_service != 0);
    Uint32 _queueId = _service->getQueueId();

    //
    // create stop all providers request
    //
    CIMStopAllProvidersRequestMessage* stopRequest =
        new CIMStopAllProvidersRequestMessage(
            XmlWriter::getNextMessageId(),
            QueueIdStack(_queueId),
            _shutdownTimeout);

    //
    // create async request message
    //
    AsyncRequest *asyncRequest;

    if (controlProviders)
    {
        asyncRequest = new AsyncModuleOperationStart(
           0,
           _queueId,
           String(),
           stopRequest);
    }
    else
    {
        asyncRequest = new AsyncLegacyOperationStart(
            0,
            _queueId,
            stopRequest);
    }


    // Use SendWait, which is serialized and waits. Do not use asynchronous
    // callback as the response might be received _after_ the provider or
    // this service has been deleted.

    AsyncReply* asyncReply =
        _controller->ClientSendWait(_queueId, asyncRequest);


    MessageType msgType = asyncReply->getType();
    PEGASUS_ASSERT((msgType == ASYNC_ASYNC_LEGACY_OP_RESULT) ||
        (msgType == ASYNC_ASYNC_MODULE_OP_RESULT));

    CIMStopAllProvidersResponseMessage *response;

    if (msgType == ASYNC_ASYNC_LEGACY_OP_RESULT)
    {
        response = reinterpret_cast<CIMStopAllProvidersResponseMessage *>(
            (static_cast<AsyncLegacyOperationResult *>(
                asyncReply))->get_result());
    }
    else
    {
        response = reinterpret_cast<CIMStopAllProvidersResponseMessage*>(
            (static_cast<AsyncModuleOperationResult *>(
                asyncReply))->get_result());
    }

    if (response->cimException.getCode() != CIM_ERR_SUCCESS)
    {
        Logger::put_l(Logger::ERROR_LOG, System::CIMSERVER, Logger::SEVERE,
            MessageLoaderParms(
                "Server.ShutdownService.CIM_PROVIDER_SHUTDOWN",
                "A provider shutdown exception has occurred: $0",
                response->cimException.getMessage()));
    }

    delete asyncRequest;
    delete asyncReply;
    delete response;

    PEG_METHOD_EXIT();
}

void ShutdownService::waitUntilNoMoreRequests(Boolean requestPending)
{
    Uint32 waitTime = _shutdownTimeout;    // maximum wait time in seconds
    const Uint32 waitInterval = 1;         // one second wait interval
    Uint32 requestCount;

    //
    // Loop and wait one second until either there is no more requests
    // or until timeout expires.
    //
    while (waitTime > 0)
    {
        requestCount = _cimserver->getOutstandingRequestCount();
        if (requestCount <= (requestPending ? 1 : 0))
        {
            break;
        }

        PEG_TRACE((
            TRC_SHUTDOWN,
            Tracer::LEVEL4,
            "ShutdownService waiting for outstanding CIM operations to "
                "complete.  Request count: %d",
            requestCount));
        Threads::sleep(waitInterval * 1000);
        waitTime -= waitInterval;
    }

    PEG_TRACE((
        TRC_SHUTDOWN,
        Tracer::LEVEL4,
        "ShutdownService::shutdown - All outstanding CIM operations "
            "complete = %s",
        ((_cimserver->getOutstandingRequestCount()) <=
         (requestPending ? 1 : 0)) ? "true" : "false"));
}

PEGASUS_NAMESPACE_END
