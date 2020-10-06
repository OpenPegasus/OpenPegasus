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

#include "ProviderManagerService.h"

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/PegasusVersion.h>
#include <Pegasus/Common/Constants.h>
#include <Pegasus/Common/CIMMessage.h>
#include <Pegasus/Common/CimomMessage.h>
#include <Pegasus/Common/Thread.h>
#include <Pegasus/Common/Tracer.h>
#include <Pegasus/Common/Logger.h>
#include <Pegasus/Common/AutoPtr.h>
#include <Pegasus/Common/Constants.h>
#include <Pegasus/Common/StatisticalData.h>
#include <Pegasus/Common/StringConversion.h>
#include <Pegasus/Common/ModuleController.h>

#include <Pegasus/Config/ConfigManager.h>

#include <Pegasus/ProviderManagerRouter/BasicProviderManagerRouter.h>
#include <Pegasus/ProviderManagerRouter/OOPProviderManagerRouter.h>

#include <Pegasus/Server/ProviderRegistrationManager/ProviderManagerMap.h>

#ifdef PEGASUS_ZOS_SECURITY
// This include file will not be provided in the OpenGroup CVS for now.
// Do NOT try to include it in your compile
#include <Pegasus/ProviderManager2/ProviderManagerzOS_inline.h>
#endif

PEGASUS_NAMESPACE_BEGIN

const String PG_PROVMODULE_GROUPNAME_CIMSERVER = "CIMServer";

ProviderManagerService* ProviderManagerService::providerManagerService=NULL;
Boolean ProviderManagerService::_allProvidersStopped = false;
Uint32 ProviderManagerService::_indicationServiceQueueId = PEG_NOT_FOUND;
ProviderRegistrationManager*
    ProviderManagerService::_providerRegistrationManager;

/**
    Hashtable for the failed provider modules. This table maintains the
    failure count for each provider module.
*/
static HashTable <String, Uint32, EqualFunc <String>,
    HashFunc <String> > _failedProviderModuleTable;

static Mutex _failedProviderModuleTableMutex;

//
// This method is called when the provider module is failed and
// maxFailedProviderModuleRestarts config value is specified.
//
void  ProviderManagerService::_invokeProviderModuleStartMethod(
    const CIMObjectPath &ref)
{
    try
    {
        ModuleController *controller = ModuleController::getModuleController();
        PEGASUS_ASSERT(controller);

        CIMInvokeMethodRequestMessage* request =
            new CIMInvokeMethodRequestMessage(
                XmlWriter::getNextMessageId(),
                PEGASUS_NAMESPACENAME_INTEROP,
                ref,
                "Start",
                Array<CIMParamValue>(),
                QueueIdStack(controller->getQueueId()));

        request->operationContext.insert(
            IdentityContainer(String()));

        AsyncModuleOperationStart* moduleControllerRequest =
            new AsyncModuleOperationStart(
                0,
                controller->getQueueId(),
                PEGASUS_MODULENAME_PROVREGPROVIDER,
                request);
        // We are not intersted in the response.
        controller->SendForget(moduleControllerRequest);
    }
    catch(const Exception &e)
    {
        PEG_TRACE((TRC_PROVIDERMANAGER,Tracer::LEVEL1,
            "Exception caught while invoking PG_ProviderModule.start"
                " method: %s",
        (const char*)e.getMessage().getCString()));
    }
    catch(...)
    {
        PEG_TRACE_CSTRING(TRC_PROVIDERMANAGER,Tracer::LEVEL1,
            "Unknown exception caught while invoking PG_ProviderModule.start"
                " method");
    }
}

ProviderManagerService::ProviderManagerService(
        ProviderRegistrationManager * providerRegistrationManager,
        CIMRepository * repository,
        ProviderManager* (*createDefaultProviderManagerCallback)())
    : MessageQueueService(PEGASUS_QUEUENAME_PROVIDERMANAGER_CPP)
{
    providerManagerService=this;
    _repository=repository;

    _providerRegistrationManager = providerRegistrationManager;

    _idleTimeCleanupBusy = 0;

    _forceProviderProcesses = ConfigManager::parseBooleanValue(
        ConfigManager::getInstance()->getCurrentValue(
            "forceProviderProcesses"));

    _oopProviderManagerRouter = new OOPProviderManagerRouter(
        indicationCallback,
        responseChunkCallback,
        providerModuleGroupFailureCallback,
        asyncResponseCallback);

    _basicProviderManagerRouter = new BasicProviderManagerRouter(
        indicationCallback,
        responseChunkCallback,
        createDefaultProviderManagerCallback);
}

ProviderManagerService::~ProviderManagerService(void)
{
    delete _basicProviderManagerRouter;
    delete _oopProviderManagerRouter;
    providerManagerService=NULL;
}

void ProviderManagerService::handleEnqueue(void)
{
    Message * message = dequeue();

    handleEnqueue(message);
}

void ProviderManagerService::_handleIndicationDeliveryResponse(Message *message)
{
    CIMProcessIndicationResponseMessage *response =
        (CIMProcessIndicationResponseMessage*)message;
    if (response->oopAgentName.size())
    {
        _oopProviderManagerRouter->processMessage(response);
    }
    else
    {
        IndicationRouter::notify(response);
    }
}

void ProviderManagerService::handleEnqueue(Message * message)
{
    PEGASUS_ASSERT(message != 0);

    if (message->getType() == CIM_PROCESS_INDICATION_RESPONSE_MESSAGE)
    {
        _handleIndicationDeliveryResponse(message);
        return;
    }

    AsyncLegacyOperationStart* asyncRequest =
        static_cast<AsyncLegacyOperationStart*>(message->get_async());

    if (asyncRequest == 0)
    {
        asyncRequest = new AsyncLegacyOperationStart(
            0,
            this->getQueueId(),
            message);
    }

    _handle_async_request(asyncRequest);
}

void ProviderManagerService::_handle_async_request(AsyncRequest * request)
{
    PEG_METHOD_ENTER(TRC_PROVIDERMANAGER,
        "ProviderManagerService::_handle_async_request");

    PEGASUS_ASSERT((request != 0) && (request->op != 0));

    if (request->getType() == ASYNC_ASYNC_LEGACY_OP_START)
    {
        _incomingQueue.insert_back(request->op);
        ThreadStatus rtn = PEGASUS_THREAD_OK;
        while ((rtn = _thread_pool->allocate_and_awaken(
                          (void *)this,
                          ProviderManagerService::handleCimOperation)) !=
                   PEGASUS_THREAD_OK)
        {
            if (rtn == PEGASUS_THREAD_INSUFFICIENT_RESOURCES)
                Threads::yield();
            else
            {
                PEG_TRACE((
                    TRC_PROVIDERMANAGER,
                    Tracer::LEVEL1,
                    "Could not allocate thread for %s.",
                    getQueueName()));
                break;
           }
        }
    }
    else
    {
        // pass all other operations to the default handler
        MessageQueueService::_handle_async_request(request);
    }

    PEG_METHOD_EXIT();

    return;
}

// Note: This method should not throw an exception.  It is used as a thread
// entry point, and any exceptions thrown are ignored.
ThreadReturnType PEGASUS_THREAD_CDECL
ProviderManagerService::handleCimOperation(void* arg)
{
    PEG_METHOD_ENTER(TRC_PROVIDERMANAGER,
        "ProviderManagerService::handleCimOperation");

    PEGASUS_ASSERT(arg != 0);

    // get the service from argument
    ProviderManagerService* service =
        reinterpret_cast<ProviderManagerService *>(arg);
    PEGASUS_ASSERT(service != 0);

    try
    {
        AsyncOpNode* op = service->_incomingQueue.remove_front();
        PEGASUS_ASSERT(op != 0);

        AsyncRequest* request =
            static_cast<AsyncRequest*>(op->getRequest());
        PEGASUS_ASSERT(request != 0);
        PEGASUS_ASSERT(request->getType() == ASYNC_ASYNC_LEGACY_OP_START);

        Message* legacy =
           static_cast<AsyncLegacyOperationStart *>(request)->get_action();
        static_cast<AsyncLegacyOperationStart *>(request)->put_action(legacy);

        // Set the client's requested language into this service thread.
        // This will allow functions in this service to return messages
        // in the correct language.
        CIMMessage* msg = dynamic_cast<CIMMessage *>(legacy);
        PEGASUS_ASSERT(msg != 0);

        Thread::setLanguages(
            ((AcceptLanguageListContainer)msg->operationContext.get(
                AcceptLanguageListContainer::NAME)).getLanguages());

        service->handleCimRequest(op, legacy);
    }
    catch (const Exception& e)
    {
        PEG_TRACE((TRC_DISCARDED_DATA, Tracer::LEVEL1,
            "Unexpected exception in handleCimOperation: %s",
            (const char*)e.getMessage().getCString()));
    }
    catch (...)
    {
        PEG_TRACE_CSTRING(TRC_DISCARDED_DATA, Tracer::LEVEL1,
            "Unexpected exception in handleCimOperation.");
    }

    PEG_METHOD_EXIT();

    return ThreadReturnType(0);
}

void ProviderManagerService::handleCimRequest(
    AsyncOpNode * op,
    Message * message)
{
    PEG_METHOD_ENTER(TRC_PROVIDERMANAGER,
        "ProviderManagerService::handleCimRequest");

    CIMRequestMessage * request = dynamic_cast<CIMRequestMessage *>(message);
    PEGASUS_ASSERT(request != 0);

    AutoPtr<Message> response;

    try
    {
        if (request->getType() == CIM_EXPORT_INDICATION_REQUEST_MESSAGE)
        {
            //
            // Get a ProviderIdContainer for ExportIndicationRequestMessage.
            // Note: This can be removed when the CIMExportRequestDispatcher
            // is updated to add the ProviderIdContainer to the message.
            //
            CIMInstance providerModule;
            CIMInstance provider;
            const CIMExportIndicationRequestMessage* expRequest =
                dynamic_cast<const CIMExportIndicationRequestMessage*>(request);
            PEGASUS_ASSERT(expRequest != 0);
            if (_providerRegistrationManager->lookupIndicationConsumer(
                    expRequest->destinationPath, provider, providerModule))
            {
                request->operationContext.insert(
                    ProviderIdContainer(providerModule, provider));
                response.reset(_processMessage(request));
            }
            else
            {
                CIMResponseMessage* cimResponse = request->buildResponse();
                cimResponse->cimException = PEGASUS_CIM_EXCEPTION(
                    CIM_ERR_NOT_SUPPORTED, String::EMPTY);
                response.reset(cimResponse);
            }
        }
        else if ((dynamic_cast<CIMOperationRequestMessage*>(request) != 0) ||
            (dynamic_cast<CIMIndicationRequestMessage*>(request) != 0) ||
            (request->getType() == CIM_EXPORT_INDICATION_REQUEST_MESSAGE))
        {
            // Handle CIMOperationRequestMessage,
            // CIMExportIndicationRequestMessage,
            // and CIMIndicationRequestMessage.
            // (These should be blocked when the provider module is disabled.)

            //
            // Get the provider module instance to check for a disabled module
            //
            CIMInstance providerModule;

            // The provider ID container is added to the OperationContext
            // by the CIMOperationRequestDispatcher for all CIM operation
            // requests to providers, so it does not need to be added again.
            ProviderIdContainer pidc =
                request->operationContext.get(ProviderIdContainer::NAME);
            providerModule = pidc.getModule();

            //
            // Check if the target provider is disabled
            //
            Boolean moduleDisabled = false;
            Uint32 pos =
                providerModule.findProperty(CIMName("OperationalStatus"));
            PEGASUS_ASSERT(pos != PEG_NOT_FOUND);
            Array<Uint16> operationalStatus;
            providerModule.getProperty(pos).getValue().get(operationalStatus);

            for (Uint32 i = 0; i < operationalStatus.size(); i++)
            {
                if ((operationalStatus[i] == CIM_MSE_OPSTATUS_VALUE_STOPPED) ||
                    (operationalStatus[i] == CIM_MSE_OPSTATUS_VALUE_STOPPING))
                {
                    moduleDisabled = true;
                    break;
                }
            }

            if (moduleDisabled)
            {
                //
                // Send a "provider blocked" response
                //
                CIMResponseMessage* cimResponse = request->buildResponse();
                cimResponse->cimException = PEGASUS_CIM_EXCEPTION_L(
                    CIM_ERR_NOT_SUPPORTED,
                    MessageLoaderParms(
                        "ProviderManager.ProviderManagerService."
                            "PROVIDER_BLOCKED",
                        "provider blocked."));
                response.reset(cimResponse);
            }
            else
            {
                //
                // Forward the request to the appropriate ProviderManagerRouter
                //
                response.reset(_processMessage(request));
            }
        }
        else if (request->getType() == CIM_ENABLE_MODULE_REQUEST_MESSAGE)
        {
            // Handle CIMEnableModuleRequestMessage
            CIMEnableModuleRequestMessage * emReq =
                dynamic_cast<CIMEnableModuleRequestMessage*>(request);

            CIMInstance providerModule = emReq->providerModule;

            // Forward the request to the ProviderManager
            response.reset(_processMessage(request));

            // If successful, update provider module status to OK
            // ATTN: Use CIMEnableModuleResponseMessage operationalStatus?
            CIMEnableModuleResponseMessage * emResp =
                dynamic_cast<CIMEnableModuleResponseMessage*>(response.get());
            // If the provider is not loaded then update the provider status in
            // this thread or else the response thread will call
            // asyncResponseCallback  which will update the provider status.
            if(!emResp->isAsyncResponsePending)
            {
                _updateModuleStatusToEnabled(emResp, providerModule);
            }
        }
        else if (request->getType() == CIM_DISABLE_MODULE_REQUEST_MESSAGE)
        {
            // Handle CIMDisableModuleRequestMessage
            CIMDisableModuleRequestMessage * dmReq =
                dynamic_cast<CIMDisableModuleRequestMessage*>(request);

            CIMInstance providerModule = dmReq->providerModule;
            Boolean updateModuleStatus = !dmReq->disableProviderOnly;

            //
            //  On issuing a disable request, append Stopping status
            //  Do not remove existing status
            //
            if (updateModuleStatus)
            {
                Array<Uint16> removeStatus;
                Array<Uint16> appendStatus;
                appendStatus.append (CIM_MSE_OPSTATUS_VALUE_STOPPING);
                _updateProviderModuleStatus(
                    providerModule, removeStatus, appendStatus);

                String providerModuleName;
                Uint32 pos = providerModule.findProperty(
                    PEGASUS_PROPERTYNAME_NAME);
                PEGASUS_ASSERT(pos != PEG_NOT_FOUND);
                providerModule.getProperty(pos).getValue().get(
                    providerModuleName);
                // Remove from failedProviderModuleTable.
                AutoMutex mtx(_failedProviderModuleTableMutex);
                _failedProviderModuleTable.remove(providerModuleName);
            }

            // Forward the request to the ProviderManager
            response.reset(_processMessage(request));

            // If the provider is not initialized then update the status of the
            // provider in this thread since there will be no response from
            // any provider.
            CIMDisableModuleResponseMessage * dmResp =
                dynamic_cast<CIMDisableModuleResponseMessage*>(response.get());
            if(!dmResp->isAsyncResponsePending)
            {
                if (updateModuleStatus)
                {
                    _updateModuleStatusToDisabled(dmResp,providerModule);
                }
            }
        }
        else
        {
            response.reset(_processMessage(request));
        }
    }
    catch (Exception& e)
    {
        CIMResponseMessage* cimResponse = request->buildResponse();
        cimResponse->cimException =
            CIMException(CIM_ERR_FAILED, e.getMessage());
        response.reset(cimResponse);
    }
    catch (PEGASUS_STD(exception)& e)
    {
        CIMResponseMessage* cimResponse = request->buildResponse();
        cimResponse->cimException = CIMException(CIM_ERR_FAILED, e.what());
        response.reset(cimResponse);
    }
    catch (...)
    {
        CIMResponseMessage* cimResponse = request->buildResponse();
        cimResponse->cimException = CIMException(CIM_ERR_FAILED, String::EMPTY);
        response.reset(cimResponse);
    }

    // all responses will be handled by the asyncResponseCallback
    // Certain requests like disable and enable module will be processed
    // in this thread if the module is not loaded yet.
    CIMResponseMessage *cimResponse = dynamic_cast<CIMResponseMessage*>(
        response.get());

    if(!cimResponse->isAsyncResponsePending)
    {
        // constructor of object is putting itself into a linked list
        // DO NOT remove the new operator
        new AsyncLegacyOperationResult(
            op,
            response.release());

        _complete_op_node(op);
    }

    PEG_METHOD_EXIT();
}

void ProviderManagerService::responseChunkCallback(
    CIMRequestMessage* request,
    CIMResponseMessage* response)
{
    PEG_METHOD_ENTER(TRC_PROVIDERMANAGER,
        "ProviderManagerService::responseChunkCallback");

    try
    {
        // only incomplete messages are processed because the caller ends up
        // sending the complete() stage
        PEGASUS_ASSERT(response->isComplete() == false);

        AsyncLegacyOperationStart *requestAsync =
            dynamic_cast<AsyncLegacyOperationStart *>(request->get_async());
        request->put_async(requestAsync);  // Put it back for the next chunk
        PEGASUS_ASSERT(requestAsync);
        AsyncOpNode *op = requestAsync->op;
        PEGASUS_ASSERT(op);
        PEGASUS_ASSERT(!response->get_async());
        response->put_async(new AsyncLegacyOperationResult(op, response));

        // set the destination
        op->_op_dest = op->_callback_response_q;

        MessageQueueService *service =
            dynamic_cast<MessageQueueService *>(op->_callback_response_q);

        PEGASUS_ASSERT(service);

        // the last chunk MUST be sent last, so use execute the callback
        // not all chunks are going through the dispatcher's chunk
        // resequencer, so this must be a synchronous call here
        // After the call is done, response and asyncResponse are now invalid
        // as they have been sent and deleted externally

        op->_async_callback(op, service, op->_callback_ptr);
    }
    catch (Exception &e)
    {
        PEG_TRACE((TRC_DISCARDED_DATA, Tracer::LEVEL1,
            "Exception in ProviderManagerService::responseChunkCallback: %s"
            ". Chunk not delivered.",(const char*)e.getMessage().getCString()));
    }
    catch (...)
    {
        PEG_TRACE_CSTRING(TRC_DISCARDED_DATA, Tracer::LEVEL1,
            "Unknown exception in ProviderManagerService::"
            "responseChunkCallback. Chunk not delivered.");
    }

    PEG_METHOD_EXIT();
}


void ProviderManagerService::asyncResponseCallback(
    CIMRequestMessage* request,
    CIMResponseMessage* response)
{
    PEG_METHOD_ENTER(TRC_PROVIDERMANAGER,
        "ProviderManagerService::asyncResponseCallback");

    AsyncLegacyOperationStart *requestAsync =
       dynamic_cast<AsyncLegacyOperationStart *>(request->get_async());
    request->put_async(requestAsync);
    PEGASUS_ASSERT(requestAsync);

    AsyncOpNode *op = requestAsync->op;
    PEGASUS_ASSERT(op);

    try
    {
        // Only complete responses for async responses are handled
        // here.
        PEGASUS_ASSERT(response->isComplete() == true);

        if(request->operationContext.contains(
            AcceptLanguageListContainer::NAME))
        {
            Thread::setLanguages(
                ((AcceptLanguageListContainer)request->operationContext.get(
                AcceptLanguageListContainer::NAME)).getLanguages());
        }
        else
        {
                Thread::setLanguages(AcceptLanguageList());
        }

        if(request->getType() == CIM_STOP_ALL_PROVIDERS_REQUEST_MESSAGE)
        {
            _allProvidersStopped = true;
        }
        else if(request->getType() == CIM_ENABLE_MODULE_REQUEST_MESSAGE)
        {
            // Handle CIMEnableModuleRequestMessage
            CIMEnableModuleRequestMessage * emReq =
                dynamic_cast<CIMEnableModuleRequestMessage*>(request);

            CIMInstance providerModule = emReq->providerModule;

            // If successful, update provider module status to OK
            // ATTN: Use CIMEnableModuleResponseMessage operationalStatus?
            CIMEnableModuleResponseMessage * emResp =
                dynamic_cast<CIMEnableModuleResponseMessage*>(response);
            providerManagerService->_updateModuleStatusToEnabled(
                emResp,providerModule);
        }
        else if (request->getType() == CIM_DISABLE_MODULE_REQUEST_MESSAGE)
        {
            // Handle CIMDisableModuleRequestMessage
            CIMDisableModuleRequestMessage * dmReq =
                dynamic_cast<CIMDisableModuleRequestMessage*>(request);

            CIMInstance providerModule = dmReq->providerModule;
            Boolean updateModuleStatus = !dmReq->disableProviderOnly;

            // Update provider module status based on success or failure
            if (updateModuleStatus)
            {
                CIMDisableModuleResponseMessage * dmResp =
                    dynamic_cast<CIMDisableModuleResponseMessage*>(
                        response);
                providerManagerService->_updateModuleStatusToDisabled(
                    dmResp,providerModule);
            }
        }
    }
    catch (Exception &e)
    {
        response->cimException =
            CIMException(CIM_ERR_FAILED, e.getMessage());
    }
    catch (PEGASUS_STD(exception)& e)
    {
        response->cimException = CIMException(CIM_ERR_FAILED, e.what());
    }
    catch (...)
    {
        response->cimException = CIMException(CIM_ERR_FAILED, String());
    }

    // constructor of object is putting itself into a linked list
    // DO NOT remove the new operator
    new AsyncLegacyOperationResult(op, response);

    providerManagerService->_complete_op_node(op);

    PEG_METHOD_EXIT();
}

Message* ProviderManagerService::_processMessage(CIMRequestMessage* request)
{
    Message* response = 0;

    // Add CachedClassDefinitionContainer here needed by CMPIProviderManager
    // to correct mismatches between Embedded Instances/Objects for InvokeMethod
    // requests.
#if defined (PEGASUS_ENABLE_CMPI_PROVIDER_MANAGER)
    if (request->getType() == CIM_INVOKE_METHOD_REQUEST_MESSAGE)
    {
        CIMOperationRequestMessage *reqMsg =
            dynamic_cast<CIMOperationRequestMessage*>(request);
        PEGASUS_ASSERT(reqMsg);
        ProviderIdContainer pidc = (ProviderIdContainer)
           reqMsg->operationContext.get(ProviderIdContainer::NAME);
        CIMInstance providerModule = pidc.getModule();
        String interfaceType;
        CIMValue itValue = providerModule.getProperty(
            providerModule.findProperty("InterfaceType")).getValue();
        itValue.get(interfaceType);
        if (interfaceType == "CMPI")
        {
            CIMConstClass cls = _repository->getFullConstClass(
                reqMsg->nameSpace,
               reqMsg->className);
            reqMsg->operationContext.insert(
                CachedClassDefinitionContainer(cls));
        }
    }
 #endif

    if ((request->getType() == CIM_STOP_ALL_PROVIDERS_REQUEST_MESSAGE) ||
        (request->getType() ==
            CIM_SUBSCRIPTION_INIT_COMPLETE_REQUEST_MESSAGE) ||
        (request->getType() ==
            CIM_INDICATION_SERVICE_DISABLED_REQUEST_MESSAGE) ||
        (request->getType() == CIM_NOTIFY_CONFIG_CHANGE_REQUEST_MESSAGE))
    {
        if (_basicProviderManagerRouter)
        {
            response = _basicProviderManagerRouter->processMessage(request);
        }

        if (_oopProviderManagerRouter)
        {
            // Note: These responses do not contain interesting data, so just
            // use the last one.
            delete response;

            response = _oopProviderManagerRouter->processMessage(request);
        }
    }
    else
    {
        CIMInstance providerModule;
        Uint16 bitness = PG_PROVMODULE_BITNESS_DEFAULT;

        if (request->getType() == CIM_ENABLE_MODULE_REQUEST_MESSAGE)
        {
            CIMEnableModuleRequestMessage* emReq =
                dynamic_cast<CIMEnableModuleRequestMessage*>(request);
            PEGASUS_ASSERT(emReq != 0);
            providerModule = emReq->providerModule;
        }
        else if (request->getType() == CIM_DISABLE_MODULE_REQUEST_MESSAGE)
        {
            CIMDisableModuleRequestMessage* dmReq =
                dynamic_cast<CIMDisableModuleRequestMessage*>(request);
            PEGASUS_ASSERT(dmReq != 0);
            providerModule = dmReq->providerModule;
        }
        else
        {
            ProviderIdContainer pidc =
                request->operationContext.get(ProviderIdContainer::NAME);
            providerModule = pidc.getModule();

            String interfaceType;
            String interfaceVersion;
            CIMValue itValue = providerModule.getProperty(
                providerModule.findProperty("InterfaceType")).getValue();
            CIMValue ivValue = providerModule.getProperty(
                providerModule.findProperty("InterfaceVersion")).getValue();
            itValue.get(interfaceType);
            ivValue.get(interfaceVersion);

            Uint32  idx = providerModule.findProperty(
                PEGASUS_PROPERTYNAME_MODULE_BITNESS);

            if (idx != PEG_NOT_FOUND)
            {
                CIMValue value = providerModule.getProperty(idx).getValue();
                if (!value.isNull())
                {
                    value.get(bitness);
                }
            }

            String provMgrPath;

            if (!ProviderManagerMap::instance().getProvMgrPathForIfcType(
                    interfaceType, interfaceVersion, bitness, provMgrPath))
            {
                MessageLoaderParms parms(
                    "ProviderManager.ProviderManagerService."
                        "PROVIDERMANAGER_LOOKUP_FAILED",
                    "Provider interface type \"$0\" version \"$1\" is not "
                        "recognized.",
                    interfaceType,
                    interfaceVersion);
                Logger::put_l(
                    Logger::ERROR_LOG, System::CIMSERVER, Logger::SEVERE,
                    parms);

                CIMResponseMessage* cimResponse = request->buildResponse();
                cimResponse->cimException = PEGASUS_CIM_EXCEPTION_L(
                    CIM_ERR_FAILED, parms);
                return cimResponse;
            }

            pidc.setProvMgrPath(provMgrPath);

            request->operationContext.set(pidc);

#ifdef PEGASUS_ZOS_SECURITY
            if (request->getType() != CIM_EXPORT_INDICATION_REQUEST_MESSAGE)
            {
                // this is a z/OS only function
                // the function checks user authorization
                // based on CIM operation versus provider profile
                // Input: request and Provider ID Container
                //Return: failure: a response message for the client
                //        success: NULL
                response = checkSAFProviderProfile(request, pidc);
                if (response != NULL)
                {
                    return response;
                }
            }
#endif
        }

        Uint16 userContext = PEGASUS_DEFAULT_PROV_USERCTXT;
        Uint32 pos = providerModule.findProperty(
            PEGASUS_PROPERTYNAME_MODULE_USERCONTEXT);
        if (pos != PEG_NOT_FOUND)
        {
            providerModule.getProperty(pos).getValue().get(userContext);
        }

        String moduleGroupName;
        Uint32 idx = providerModule.findProperty(
            PEGASUS_PROPERTYNAME_MODULE_MODULEGROUPNAME);
        if (idx != PEG_NOT_FOUND)
        {
            providerModule.getProperty(idx).getValue().get(moduleGroupName);
        }

        // Load proxy-provider into CIMServer, in case of remote namespace
        // requests. (ie through _basicProviderManagerRouter). -V 3913
#ifdef PEGASUS_ENABLE_REMOTE_CMPI
        if ((dynamic_cast<CIMOperationRequestMessage*>(request) != 0) ||
            (request->getType() == CIM_EXPORT_INDICATION_REQUEST_MESSAGE))
        {
            ProviderIdContainer pidc1 =
            request->operationContext.get(ProviderIdContainer::NAME);
            if (pidc1.isRemoteNameSpace() )
            {
                PEG_TRACE_CSTRING(TRC_PROVIDERMANAGER, Tracer::LEVEL4,
                                "Processing Remote NameSpace request ");
                response = _basicProviderManagerRouter->processMessage(request);
                return response;
            }
        }
#endif
        // Forward the request to the appropriate ProviderManagerRouter, based
        // on the CIM Server configuration and the UserContext setting.

        if ( (_forceProviderProcesses &&
                   (moduleGroupName != PG_PROVMODULE_GROUPNAME_CIMSERVER))||
                      bitness == PG_PROVMODULE_BITNESS_32
#if !defined(PEGASUS_DISABLE_PROV_USERCTXT) && !defined(PEGASUS_OS_ZOS)
            || (userContext == PG_PROVMODULE_USERCTXT_REQUESTOR)
            || (userContext == PG_PROVMODULE_USERCTXT_DESIGNATED)
            || ((userContext == PG_PROVMODULE_USERCTXT_PRIVILEGED) &&
                !System::isPrivilegedUser(System::getEffectiveUserName()))
#endif
           )
        {
            response = _oopProviderManagerRouter->processMessage(request);
        }
        else
        {
            response = _basicProviderManagerRouter->processMessage(request);
        }
    }

    return response;
}
// idleTimeCleanup calls a separate thread to do the cleanup because it
// could be a long running function.
void ProviderManagerService::idleTimeCleanup()
{
    PEG_METHOD_ENTER(TRC_PROVIDERMANAGER,
        "ProviderManagerService::idleTimeCleanup");

    // Ensure that only one _idleTimeCleanupHandler thread runs at a time
    _idleTimeCleanupBusy++;
    if (_idleTimeCleanupBusy.get() != 1)
    {
        _idleTimeCleanupBusy--;
        PEG_METHOD_EXIT();
        return;
    }

    //
    // Start an idle time cleanup thread.
    //
    // The call has a second purpose, to clean up a dead enumeration.
    // In that case the contextId input would be non-empty

    if (_thread_pool->allocate_and_awaken((void*)this,
            ProviderManagerService::_idleTimeCleanupHandler) !=
                PEGASUS_THREAD_OK)
    {
        PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL1,
            "Could not allocate thread for %s to cleanup idle providers \
                and request.",
            getQueueName()));

        // If we fail to allocate a thread, don't retry now.
        _idleTimeCleanupBusy--;
        PEG_METHOD_EXIT();
        return;
    }

    // Note: _idleTimeCleanupBusy is decremented in
    // _idleTimeCleanupHandler

    PEG_METHOD_EXIT();
}

/*
    enumerationContextCleanup does not call a separate thread to execute
    because is is short running and is NOT run directly off the monitor
    thread but off the enumerationContextTableTimeout thread.
*/
bool ProviderManagerService::enumerationContextCleanup(const String& contextId)
{
    PEG_METHOD_ENTER(TRC_PROVIDERMANAGER,
        "ProviderManagerService::enumerationContextCleanup");
    bool rtn = true;
    // Ensure that only one _idleTimeCleanupHandler thread runs at a time
    _idleTimeCleanupBusy++;
    if (_idleTimeCleanupBusy.get() != 1)
    {
        _idleTimeCleanupBusy--;
        rtn = false;
    }
    if (_oopProviderManagerRouter)
    {
        try
        {
            // call the oop providerManager. If the enumerationContextId
            // string not empty this will be call to do enum context
            // cleanup. Else it is call to do idleTimeCleanup.
            _oopProviderManagerRouter->enumerationContextCleanup(
                contextId);
        }
        catch (...)
        {
            // Ignore errors
            PEG_TRACE_CSTRING(TRC_PROVIDERMANAGER, Tracer::LEVEL2,
                "Unexpected exception from "
                    "OOPProviderManagerRouter::enumerationContextCleanup");
            rtn = false;
        }
    }

        _idleTimeCleanupBusy--;

        PEG_METHOD_EXIT();
        return rtn;
}

ThreadReturnType PEGASUS_THREAD_CDECL
ProviderManagerService::_idleTimeCleanupHandler(void* arg) throw()
{
    ProviderManagerService* myself =
        reinterpret_cast<ProviderManagerService*>(arg);

    try
    {
        PEG_METHOD_ENTER(TRC_PROVIDERMANAGER,
            "ProviderManagerService::_idleTimeCleanupHandler");

        if (myself->_basicProviderManagerRouter)
        {
            try
            {
                myself->_basicProviderManagerRouter->idleTimeCleanup();
            }
            catch (...)
            {
                // Ignore errors
                PEG_TRACE_CSTRING(TRC_PROVIDERMANAGER, Tracer::LEVEL2,
                    "Unexpected exception from "
                        "BasicProviderManagerRouter::idleTimeCleanup");
            }
        }

        if (myself->_oopProviderManagerRouter)
        {
            try
            {
                myself->_oopProviderManagerRouter->idleTimeCleanup();
            }
            catch (...)
            {
                // Ignore errors
                PEG_TRACE_CSTRING(TRC_PROVIDERMANAGER, Tracer::LEVEL2,
                    "Unexpected exception from "
                        "OOPProviderManagerRouter::idleTimeCleanup");
            }
        }

        myself->_idleTimeCleanupBusy--;
        PEG_METHOD_EXIT();
    }
    catch (...)
    {
        // Ignore errors
        PEG_TRACE_CSTRING(TRC_PROVIDERMANAGER, Tracer::LEVEL2,
            "Unexpected exception in _idleTimeCleanupHandler");

        myself->_idleTimeCleanupBusy--;
    }

    return ThreadReturnType(0);
}

void ProviderManagerService::_updateModuleStatusToEnabled(
    CIMEnableModuleResponseMessage *emResp,
    CIMInstance &providerModule)
{
    if (emResp->cimException.getCode() == CIM_ERR_SUCCESS)
    {
        //
        //  On a successful enable, remove Stopped status and
        //  append OK status
        //
        Array<Uint16> removeStatus;
        Array<Uint16> appendStatus;
        removeStatus.append (CIM_MSE_OPSTATUS_VALUE_STOPPED);
        appendStatus.append (CIM_MSE_OPSTATUS_VALUE_OK);
        _updateProviderModuleStatus(
            providerModule, removeStatus, appendStatus);
       String moduleName;
       providerModule.getProperty(
           providerModule.findProperty("Name")).getValue().get(moduleName);
       AutoMutex mtx(_failedProviderModuleTableMutex);
       if (!_failedProviderModuleTable.contains(moduleName))
       {
            _providerRegistrationManager->sendPMInstAlert(
                providerModule,
                PM_ENABLED);
       }
    }
}

void ProviderManagerService::_updateModuleStatusToDisabled(
    CIMDisableModuleResponseMessage *dmResp,
    CIMInstance &providerModule)
{
    // Update provider module status based on success or failure
    if (dmResp->cimException.getCode() != CIM_ERR_SUCCESS)
    {
        //
        //  On an unsuccessful disable, remove Stopping status
        //
        Array<Uint16> removeStatus;
        Array<Uint16> appendStatus;
        removeStatus.append (CIM_MSE_OPSTATUS_VALUE_STOPPING);
        _updateProviderModuleStatus(
            providerModule, removeStatus, appendStatus);
    }
    else
    {
        // Disable may or may not have been successful,
        // depending on whether there are outstanding requests.
        // Remove Stopping status
        // Append status, if any, from disable module response
        Array<Uint16> removeStatus;
        Array<Uint16> appendStatus;
        removeStatus.append (CIM_MSE_OPSTATUS_VALUE_STOPPING);
        if (dmResp->operationalStatus.size() > 0)
        {
            //
            //  On a successful disable, remove an OK or
            // a Degraded status, if present
            //
            if (dmResp->operationalStatus[
                dmResp->operationalStatus.size()-1] ==
                CIM_MSE_OPSTATUS_VALUE_STOPPED)
            {
                removeStatus.append (CIM_MSE_OPSTATUS_VALUE_OK);
                removeStatus.append
                    (CIM_MSE_OPSTATUS_VALUE_DEGRADED);
            }
            appendStatus.append (dmResp->operationalStatus[
                dmResp->operationalStatus.size()-1]);
        }
        _updateProviderModuleStatus(
            providerModule, removeStatus, appendStatus);
        _providerRegistrationManager->sendPMInstAlert(
            providerModule,
            PM_DISABLED);
    }
}

// Updates the providerModule instance and the ProviderRegistrationManager
//
// This method is used to update the provider module status when the module is
// disabled or enabled.  If a Degraded status has been set (appended) to the
// OperationalStatus, it is cleared (removed) when the module is disabled or
// enabled.
//
void ProviderManagerService::_updateProviderModuleStatus(
    CIMInstance& providerModule,
    const Array<Uint16>& removeStatus,
    const Array<Uint16>& appendStatus)
{
    PEG_METHOD_ENTER(TRC_PROVIDERMANAGER,
        "ProviderManagerService::_updateProviderModuleStatus");

    Array<Uint16> operationalStatus;
    String providerModuleName;

    Uint32 pos = providerModule.findProperty(PEGASUS_PROPERTYNAME_NAME);
    PEGASUS_ASSERT(pos != PEG_NOT_FOUND);
    providerModule.getProperty(pos).getValue().get(providerModuleName);

    //
    // get operational status
    //
    pos = providerModule.findProperty(CIMName("OperationalStatus"));
    PEGASUS_ASSERT(pos != PEG_NOT_FOUND);
    CIMProperty operationalStatusProperty = providerModule.getProperty(pos);

    if (_providerRegistrationManager->updateProviderModuleStatus(
        providerModuleName, removeStatus, appendStatus, operationalStatus) ==
        false)
    {
        throw PEGASUS_CIM_EXCEPTION_L(
            CIM_ERR_FAILED,
            MessageLoaderParms(
                "ProviderManager.ProviderManagerService."
                    "SET_MODULE_STATUS_FAILED",
                "Failed to update the provider module status."));
    }

    operationalStatusProperty.setValue(CIMValue(operationalStatus));

    PEG_METHOD_EXIT();
}

void ProviderManagerService::indicationCallback(
    CIMProcessIndicationRequestMessage* request)
{
    IndicationRouter router =
        IndicationRouter(request, _indicationDeliveryRoutine);

    router.deliverAndWaitForStatus();
}

void ProviderManagerService::_indicationDeliveryRoutine(
    CIMProcessIndicationRequestMessage* request)
{
    if (request->operationContext.contains(AcceptLanguageListContainer::NAME))
    {
        AcceptLanguageListContainer cntr =
            request->operationContext.get(AcceptLanguageListContainer::NAME);
    }
    else
    {
        request->operationContext.insert(
            AcceptLanguageListContainer(AcceptLanguageList()));
    }

    if (_indicationServiceQueueId == PEG_NOT_FOUND)
    {
        _indicationServiceQueueId = providerManagerService->find_service_qid(
            PEGASUS_QUEUENAME_INDICATIONSERVICE);
    }

    request->queueIds = QueueIdStack(
        providerManagerService->getQueueId(),
        _indicationServiceQueueId);

    AsyncLegacyOperationStart * asyncRequest =
        new AsyncLegacyOperationStart(
        0,
        _indicationServiceQueueId,
        request);

    providerManagerService->SendForget(asyncRequest);
}

void ProviderManagerService::providerModuleGroupFailureCallback(
     const String &groupName,
     const String & userName,
     Uint16 userContext,
     Boolean isGroup)
{
    PEG_METHOD_ENTER (TRC_PROVIDERMANAGER,
        "ProviderManagerService::providerModuleFailureCallback");

    Array<String> moduleNames;

    // If this agent is servicing the group of modules, get all related
    // provider module names.
    if (isGroup)
    {
        _providerRegistrationManager->getProviderModuleNamesForGroup(
            groupName, moduleNames);
    }
    else
    {
        moduleNames.append(groupName);
    }

    for (Uint32 i = 0, n = moduleNames.size() ; i < n ; ++i)
    {
        _reconcileProviderModuleFailure(
            moduleNames[i],
            userName,
            userContext);
    }

    PEG_METHOD_EXIT();
}

void ProviderManagerService::_reconcileProviderModuleFailure(
     const String & moduleName,
     const String & userName,
     Uint16 userContext)
{
    PEG_METHOD_ENTER (TRC_PROVIDERMANAGER,
        "ProviderManagerService::_reconcileProviderModuleFailure");

    if (userContext == PG_PROVMODULE_USERCTXT_REQUESTOR)
    {
        Logger::put_l(
            Logger::STANDARD_LOG, System::CIMSERVER, Logger::WARNING,
            MessageLoaderParms(
                "ProviderManager.ProviderManagerService."
                    "OOP_PROVIDER_MODULE_USER_CTXT_FAILURE_DETECTED",
                "A failure was detected in provider module $0 with "
                    "user context $1.",
                moduleName, userName));
    }
    else  //  not requestor context
    {
        Logger::put_l(
            Logger::STANDARD_LOG, System::CIMSERVER, Logger::WARNING,
            MessageLoaderParms(
                "ProviderManager.ProviderManagerService."
                    "OOP_PROVIDER_MODULE_FAILURE_DETECTED",
                "A failure was detected in provider module $0.",
                moduleName));
    }

    //
    // Send Notify Provider Fail request message to Indication Service.
    // (After a CIMStopAllProvidersRequestMessage is processed, the
    // IndicationService may be destructed, so it cannot be used then.)
    //

    if (_indicationServiceQueueId == PEG_NOT_FOUND)
    {
        MessageQueue* indicationServiceQueue =
            lookup(PEGASUS_QUEUENAME_INDICATIONSERVICE);

        if (indicationServiceQueue)
        {
            _indicationServiceQueueId = indicationServiceQueue->getQueueId();
        }
    }

    if (!_allProvidersStopped && (_indicationServiceQueueId != PEG_NOT_FOUND))
    {
        //
        //  Create Notify Provider Fail request message
        //
        CIMNotifyProviderFailRequestMessage* request =
            new CIMNotifyProviderFailRequestMessage(
                XmlWriter::getNextMessageId(),
                moduleName,
                userName,
                QueueIdStack());

        request->queueIds = QueueIdStack(
            _indicationServiceQueueId, providerManagerService->getQueueId());

        AsyncLegacyOperationStart asyncRequest(
            0,
            _indicationServiceQueueId,
            request);

        AutoPtr <AsyncReply> asyncReply(
            providerManagerService->SendWait(&asyncRequest));

        AutoPtr <CIMNotifyProviderFailResponseMessage> response(
            reinterpret_cast<CIMNotifyProviderFailResponseMessage *>(
                (dynamic_cast<AsyncLegacyOperationResult *>(
                    asyncReply.get()))->get_result()));

        if (response->cimException.getCode () != CIM_ERR_SUCCESS)
        {
            PEG_TRACE((TRC_DISCARDED_DATA, Tracer::LEVEL2,
                "Unexpected exception in providerModuleFailureCallback: %s",
                (const char*)response->cimException.getMessage().getCString()));
        }
        else
        {
            CIMInstance providerModule;
            CIMKeyBinding keyBinding(
                _PROPERTY_PROVIDERMODULE_NAME,
                moduleName,
                CIMKeyBinding::STRING);

            Array<CIMKeyBinding> kbArray;
            kbArray.append(keyBinding);

            CIMObjectPath modulePath(
                "",
                PEGASUS_NAMESPACENAME_INTEROP,
                PEGASUS_CLASSNAME_PROVIDERMODULE,
                kbArray);

            //
            //  Successful response
            //  Examine result to see if any subscriptions were affected
            //
            if (response->numSubscriptionsAffected > 0)
            {
                //
                //  Subscriptions were affected
                //  Update the provider module status to Degraded
                //
                try
                {

                    Boolean startProviderModule = false;
                    // Get the maxFailedProviderModuleRestarts value. Note that
                    // this is a dynamic property.
                    Uint64 maxFailedProviderModuleRestarts = 0;
                    Uint32 moduleFailureCount  = 1;
                    String value =
                        ConfigManager::getInstance()->getCurrentValue(
                            "maxFailedProviderModuleRestarts");
                    StringConversion::decimalStringToUint64(value.getCString(),
                        maxFailedProviderModuleRestarts);

                    if (maxFailedProviderModuleRestarts)
                    {
                        startProviderModule = true;
                        Uint32 *failedCount;

                        AutoMutex mtx(_failedProviderModuleTableMutex);
                        if (_failedProviderModuleTable.lookupReference(
                            moduleName, failedCount))
                        {
                            if ((moduleFailureCount = ++(*failedCount)) >
                                maxFailedProviderModuleRestarts)
                            {
                                startProviderModule = false;
                                _failedProviderModuleTable.remove(moduleName);
                            }
                        }
                        else
                        {
                            _failedProviderModuleTable.insert(
                                moduleName, moduleFailureCount);
                        }
                    }

                    providerModule =
                        providerManagerService->_providerRegistrationManager->
                            getInstance(
                                modulePath, false, false, CIMPropertyList());

                    Array<Uint16> removeStatus;
                    Array<Uint16> appendStatus;

                    removeStatus.append(CIM_MSE_OPSTATUS_VALUE_OK);

                    // If the provider module needs to be restarted set the
                    // module OpertionalStatus to the STOPPED and send the
                    // module start request.
                    appendStatus.append(startProviderModule ?
                        CIM_MSE_OPSTATUS_VALUE_STOPPED :
                        CIM_MSE_OPSTATUS_VALUE_DEGRADED);

                    providerManagerService->_updateProviderModuleStatus(
                        providerModule, removeStatus, appendStatus);

                    if (startProviderModule)
                    {
                        _invokeProviderModuleStartMethod(modulePath);
                        _providerRegistrationManager->sendPMInstAlert(
                            providerModule,
                            PM_FAILED_RESTARTED);
                       //
                       // Log a information message since provider module
                       // is restarted automatically
                       //
                       Logger::put_l(
                           Logger::STANDARD_LOG,
                           System::CIMSERVER,
                           Logger::INFORMATION,
                           MessageLoaderParms(
                               "ProviderManager.OOPProviderManagerRouter."
                                  "OOP_PROVIDER_MODULE_RESTARTED_AFTER_FAILURE",
                               "The indication providers in the module"
                                  " $0 have been restarted with subscriptions"
                                  " enabled after $1 failure(s). After $2"
                                  " such attempts the provider will not be"
                                  " restarted automatically with subscriptions"
                                  " enabled. To ensure these providers continue"
                                  " to service active subscriptions please fix"
                                  " the problem in the provider.",
                                moduleName,
                                moduleFailureCount,
                                Uint32(maxFailedProviderModuleRestarts)));
                    }
                    else
                    {
                        _providerRegistrationManager->sendPMInstAlert(
                            providerModule,
                            PM_DEGRADED);
                        //
                        //  Log a warning message since subscriptions
                        //  were affected
                        //
                        Logger::put_l(
                            Logger::STANDARD_LOG,
                            System::CIMSERVER,
                            Logger::WARNING,
                            MessageLoaderParms(
                                "ProviderManager.ProviderManagerService."
                                   "OOP_PROVIDER_MODULE_SUBSCRIPTIONS_AFFECTED",
                                 "The generation of indications by providers"
                                     " in module $0 may be affected. To ensure"
                                     " these providers are serving active "
                                     "subscriptions, disable and then re-enable"
                                     " this module using the cimprovider "
                                     "command.",
                                moduleName));
                    }

                }
                catch (const Exception & e)
                {
                    PEG_TRACE((TRC_DISCARDED_DATA, Tracer::LEVEL1,
                        "Failed to update provider module status: %s",
                        (const char*)e.getMessage().getCString()));
                }
            }
            else
            {
                providerModule = providerManagerService->
                    _providerRegistrationManager->getInstance(
                        modulePath,
                        false,
                        false,
                        CIMPropertyList());
                _providerRegistrationManager->sendPMInstAlert(
                    providerModule,
                    PM_FAILED);
            }
        }
    }

    PEG_METHOD_EXIT();
}

PEGASUS_NAMESPACE_END
