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

#include <Pegasus/Common/Signal.h>
#include <Pegasus/Common/Array.h>
#include <Pegasus/Common/AutoPtr.h>
#include <Pegasus/Common/AtomicInt.h>
#include <Pegasus/Common/Tracer.h>
#include <Pegasus/Config/ConfigManager.h>
#include <Pegasus/Common/OperationContext.h>
#include <Pegasus/Common/StringConversion.h>
#include <Pegasus/ProviderManager2/Default/DefaultProviderManager.h>

#if defined(PEGASUS_OS_ZOS) && defined(PEGASUS_ZOS_SECURITY)
// This include file will not be provided in the OpenGroup CVS for now.
// Do NOT try to include it in your compile
#include <Pegasus/Common/safCheckzOS_inline.h>
#endif

#include "ProviderAgent.h"

#ifdef PEGASUS_OS_PASE
#include <ILEWrapper/ILEUtilities.h>
#endif

PEGASUS_USING_STD;

PEGASUS_NAMESPACE_BEGIN

// threadCreationFailureLogged will indicate if the thread limit related
// msg has been logged already. This will help avoid flooding the syslog/audit
// log with thread limit reached errors.
static AtomicInt threadCreationFailureLogged(0);

/////////////////////////////////////////////////////////////////////////////
//
// ProviderAgentRequest
//
/////////////////////////////////////////////////////////////////////////////

/**
    This class encapsulates the data required by a work thread to process a
    request in a Provider Agent.
 */
class ProviderAgentRequest
{
public:
    ProviderAgentRequest(ProviderAgent* agent_, CIMRequestMessage* request_)
        :agent(agent_),request(request_)
    {
    }

    ProviderAgent* agent;
    CIMRequestMessage* request;
};


/////////////////////////////////////////////////////////////////////////////
//
// ProviderAgent
//
/////////////////////////////////////////////////////////////////////////////

// Time values used in ThreadPool construction
static struct timeval deallocateWait = {300, 0};

Semaphore ProviderAgent::_scmoClassDelivered(0);
SCMOClass* ProviderAgent::_transferSCMOClass = 0;
Mutex ProviderAgent::_transferSCMOClassMutex;
String ProviderAgent::_transferSCMOClassRspMsgID;

ProviderAgent* ProviderAgent::_providerAgent = 0;

ProviderAgent::ProviderAgent(
    const String& agentId,
    AnonymousPipe* pipeFromServer,
    AnonymousPipe* pipeToServer)
  : _providerManagerRouter(_indicationCallback, _responseChunkCallback,
    DefaultProviderManager::createDefaultProviderManagerCallback),
    _threadPool(0, "ProviderAgent", 0, 0, deallocateWait)
{
    PEG_METHOD_ENTER(TRC_PROVIDERAGENT, "ProviderAgent::ProviderAgent");

    // Add the DefaultProviderManager to the router.

    _terminating = false;
    _agentId = agentId;
    _pipeFromServer = pipeFromServer;
    _pipeToServer = pipeToServer;
    _providerAgent = this;
    _isInitialised = false;
    _providersStopped = false;

    // Create a SCMOClass Cache and set call back for the repository.
    SCMOClassCache::getInstance()->setCallBack(_scmoClassCache_GetClass);

    PEG_METHOD_EXIT();
}

ProviderAgent::~ProviderAgent()
{
    PEG_METHOD_ENTER(TRC_PROVIDERAGENT, "ProviderAgent::~ProviderAgent");

    _providerAgent = 0;
    // Destroy the singleton services
    SCMOClassCache::destroy();
    if (_transferSCMOClass)
    {
       delete _transferSCMOClass;
    }

    PEG_METHOD_EXIT();
}

//get the shutdown timeout value
static Uint32 getShutdownTimeout()
{
    ConfigManager *cfgManager = ConfigManager::getInstance();
    String shutdnTimestr = cfgManager ->getCurrentValue( "shutdownTimeout");

    Uint64 shutdownTimeout = 0;

    StringConversion::decimalStringToUint64(
            shutdnTimestr.getCString(), shutdownTimeout);

    return (Uint32) shutdownTimeout;
}


void ProviderAgent::run()
{
    PEG_METHOD_ENTER(TRC_PROVIDERAGENT, "ProviderAgent::run");

    // Enable the signal handler to terminate gracefully on SIGHUP and SIGTERM
    getSigHandle()->registerHandler(PEGASUS_SIGHUP, _terminateSignalHandler);
    getSigHandle()->activate(PEGASUS_SIGHUP);
    getSigHandle()->registerHandler(PEGASUS_SIGTERM, _terminateSignalHandler);
    getSigHandle()->activate(PEGASUS_SIGTERM);
    // Restore the SIGCHLD signal behavior to its default action
    getSigHandle()->defaultAction(PEGASUS_SIGCHLD);
#ifdef PEGASUS_OS_ZOS
    // Establish handling signal send to us on USS shutdown
    getSigHandle()->registerHandler(PEGASUS_SIGDANGER, _terminateSignalHandler);
    getSigHandle()->activate(PEGASUS_SIGDANGER);
    // enable process to receive SIGDANGER on USS shutdown
    __shutdown_registration(_SDR_NOTIFY, _SDR_REGPROCESS, _SDR_SENDSIGDANGER);
#endif
#ifdef PEGASUS_OS_PASE
    // PASE environment need more signal handler
    getSigHandle()->registerHandler(SIGFPE, _synchronousSignalHandler);
    getSigHandle()->activate(SIGFPE);
    getSigHandle()->registerHandler(SIGILL, _synchronousSignalHandler);
    getSigHandle()->activate(SIGILL);
    getSigHandle()->registerHandler(SIGSEGV, _synchronousSignalHandler);
    getSigHandle()->activate(SIGSEGV);
    getSigHandle()->registerHandler(SIGIO, _asynchronousSignalHandler);
    getSigHandle()->activate(SIGIO);
#endif

    while (!_terminating)
    {
        Boolean active = true;
        try
        {
            //
            // Read and process the next request
            //
            active = _readAndProcessRequest();
        }
        catch (Exception& e)
        {
            PEG_TRACE((TRC_PROVIDERAGENT, Tracer::LEVEL1,
                "Unexpected Exception from _readAndProcessRequest(): %s",
                (const char*)e.getMessage().getCString()));
            _terminating = true;
        }
        catch (...)
        {
            PEG_TRACE_CSTRING(TRC_PROVIDERAGENT, Tracer::LEVEL1,
                "Unexpected exception from _readAndProcessRequest().");
            _terminating = true;
        }

        if (_terminating)
        {
            if (!_providersStopped)
            {
                //
                // Stop all providers
                //
                CIMStopAllProvidersRequestMessage *stopRequest =
                    new CIMStopAllProvidersRequestMessage("0", QueueIdStack(0));

                stopRequest->shutdownTimeout = getShutdownTimeout();
                _processStopAllProvidersRequest(stopRequest);
            }
        }
        else if (!active)
        {
            //
            // Stop agent process when no more providers are loaded
            try
            {
                if (!_providerManagerRouter.hasActiveProviders() &&
                    (_threadPool.runningCount() == 0))
                {
                    PEG_TRACE_CSTRING(TRC_PROVIDERAGENT, Tracer::LEVEL2,
                        "No active providers.  Exiting.");
                    _terminating = true;
                }
                else //clean up threads may still be busy
                {
                    _threadPool.cleanupIdleThreads();
                    if (!_providerManagerRouter.hasActiveProviders() &&
                              (_threadPool.runningCount() == 0))
                    {
                        _terminating = true;
                    }
                }
            }
            catch (...)
            {
                // Do not terminate the agent on this exception
                PEG_TRACE_CSTRING(TRC_PROVIDERAGENT, Tracer::LEVEL1,
                    "Unexpected exception from hasActiveProviders()");
            }
        }
    }

    // Notify the cimserver that the provider agent is exiting cleanly.
    Uint32 messageLength = 0;
    _pipeToServer->writeBuffer((const char*)&messageLength, sizeof(Uint32));

    PEG_METHOD_EXIT();
}

Boolean ProviderAgent::_readAndProcessRequest()
{
    PEG_METHOD_ENTER(TRC_PROVIDERAGENT,
        "ProviderAgent::_readAndProcessRequest");

    //
    // Read the request from CIM Server
    //
    CIMMessage* cimMessage;
    AnonymousPipe::Status readStatus = _pipeFromServer->readMessage(cimMessage);

    // Read operation was interrupted
    if (readStatus == AnonymousPipe::STATUS_INTERRUPT)
    {
        PEG_TRACE_CSTRING(TRC_PROVIDERAGENT, Tracer::LEVEL1,
            "Read operation was interrupted.");
        PEG_METHOD_EXIT();
        return false;
    }

    if (readStatus == AnonymousPipe::STATUS_CLOSED)
    {
        // The CIM Server connection is closed
        PEG_TRACE_CSTRING(TRC_PROVIDERAGENT, Tracer::LEVEL2,
            "CIMServer connection closed. Exiting.");
        _terminating = true;
        PEG_METHOD_EXIT();
        return false;
    }

    if (readStatus == AnonymousPipe::STATUS_ERROR)
    {
        PEG_TRACE_CSTRING(TRC_PROVIDERAGENT, Tracer::LEVEL1,
            "Error reading from pipe. Exiting.");
        Logger::put_l(Logger::ERROR_LOG, System::CIMSERVER, Logger::WARNING,
            MessageLoaderParms(
                "ProviderManager.ProviderAgent.ProviderAgent."
                    "CIMSERVER_COMMUNICATION_FAILED",
                "cimprovagt \"$0\" communication with CIM Server failed.  "
                    "Exiting.",
                _agentId));
        _terminating = true;
        PEG_METHOD_EXIT();
        return false;
    }

    CIMRequestMessage* request = 0;
    request = dynamic_cast<CIMRequestMessage*>(cimMessage);

    // The message was not a request message.
    if (request == 0)
    {
        // The message was not empty.
        if (0 != cimMessage )
        {
            // The message was not a "wake up" message.
            if (cimMessage->getType() == PROVAGT_GET_SCMOCLASS_RESPONSE_MESSAGE)
            {

                PEG_TRACE_CSTRING(TRC_PROVIDERAGENT, Tracer::LEVEL3,
                    "Processing a SCMOClassResponseMessage.");

                AutoPtr<ProvAgtGetScmoClassResponseMessage> response(
                    dynamic_cast<ProvAgtGetScmoClassResponseMessage*>
                        (cimMessage));

                PEGASUS_DEBUG_ASSERT(response.get());

                _processGetSCMOClassResponse(response.get());

                // The provider agent is still busy.
                PEG_METHOD_EXIT();
                return true;
           }
           else if (cimMessage->getType() ==
               CIM_PROCESS_INDICATION_RESPONSE_MESSAGE)
           {
               _handleIndicationDeliveryResponse(
                   (CIMProcessIndicationResponseMessage*)cimMessage);
               PEG_METHOD_EXIT();
               return true;
           }
        }

        // A "wake up" message means we should unload idle providers
        PEG_TRACE_CSTRING(TRC_PROVIDERAGENT, Tracer::LEVEL4,
            "Got a wake up message.");

        if (!_providerManagerRouter.hasActiveProviders())
        {
            // No active providers, so do not start an idle unload thread
            PEG_METHOD_EXIT();
            return false;
        }

        if(!_unloadIdleProviders())
        {
            PEG_TRACE_CSTRING(TRC_PROVIDERAGENT, Tracer::LEVEL3,
                "unloading can not be completed now.");
            Logger::put_l(Logger::ERROR_LOG, System::CIMSERVER, Logger::WARNING,
                MessageLoaderParms(
                    "ProviderManager.ProviderAgent.ProviderAgent."
                    "PROVIDERS_FAILED_TO_UNLOAD",
                    "Provider agent \"{0}\" failed to unload and exit within"
                    "\"{1}\" seconds during idle cleanup."
                    " It will be exited in the next idle clean up.",
                    _agentId, getShutdownTimeout() ));
        }

        PEG_METHOD_EXIT();
        return false;
    }

    PEG_TRACE((TRC_PROVIDERAGENT, Tracer::LEVEL3,
        "Received request from server with messageId %s",
        (const char*)request->messageId.getCString()));

    const AcceptLanguageListContainer acceptLang =
        request->operationContext.get(AcceptLanguageListContainer::NAME);
    Thread::setLanguages(acceptLang.getLanguages());

    // Get the ProviderIdContainer to complete the provider module instance
    // optimization.  If the provider module instance is blank (optimized
    // out), fill it in from our cache.  If it is not blank, update our
    // cache.  (See the _providerModuleCache member description.)
    if (request->operationContext.contains(ProviderIdContainer::NAME))
    {
        ProviderIdContainer pidc = request->operationContext.get(
            ProviderIdContainer::NAME);
        if (pidc.getModule().isUninitialized())
        {
            // Provider module is optimized out.  Fill it in from the cache.
           ProviderIdContainer newpidc(_providerModuleCache, pidc.getProvider(),
                pidc.isRemoteNameSpace(), pidc.getRemoteInfo());
            newpidc.setProvMgrPath(pidc.getProvMgrPath());
            request->operationContext.set(newpidc);
        }
        else
        {
            // Update the cache with the new provider module instance.
            _providerModuleCache = pidc.getModule();
        }
    }

    //
    // It never should be possible to receive a request other than "initialise"
    // before the provider agent is in state isInitialised
    //
    // Bail out.
    //
    if (!_isInitialised &&
        (request->getType() != CIM_INITIALIZE_PROVIDER_AGENT_REQUEST_MESSAGE))
    {
        Logger::put_l(Logger::ERROR_LOG, System::CIMSERVER, Logger::WARNING,
            MessageLoaderParms(
                "ProviderManager.ProviderAgent.ProviderAgent."
                    "PROVIDERAGENT_NOT_INITIALIZED",
                "cimprovagt \"$0\" was not yet initialized "
                    "prior to receiving a request message. Exiting.",
                _agentId));
        _terminating = true;
        PEG_METHOD_EXIT();
        return false;
    }

    //
    // Check for messages to be handled by the Agent itself.
    //
    if (request->getType() == CIM_INITIALIZE_PROVIDER_AGENT_REQUEST_MESSAGE)
    {
        // Process the request in this thread
        AutoPtr<CIMInitializeProviderAgentRequestMessage> ipaRequest(
            dynamic_cast<CIMInitializeProviderAgentRequestMessage*>(request));
        PEGASUS_ASSERT(ipaRequest.get() != 0);

        ConfigManager::setPegasusHome(ipaRequest->pegasusHome);
        ConfigManager* configManager = ConfigManager::getInstance();

        // Initialize the configuration properties
        for (Uint32 i = 0; i < ipaRequest->configProperties.size(); i++)
        {
            configManager->initCurrentValue(
                ipaRequest->configProperties[i].first,
                ipaRequest->configProperties[i].second);
        }

        // Set the default resource bundle directory for the MessageLoader
        MessageLoader::setPegasusMsgHome(ConfigManager::getHomedPath(
            configManager->getCurrentValue("messageDir")));

        // Set the log file directory
#if !defined(PEGASUS_USE_SYSLOGS)
        Logger::setHomeDirectory(ConfigManager::getHomedPath(
            configManager->getCurrentValue("logdir")));
#endif
        System::bindVerbose = ipaRequest->bindVerbose;

        //
        //  Set _subscriptionInitComplete from value in
        //  InitializeProviderAgent request
        //
        _providerManagerRouter.setSubscriptionInitComplete
            (ipaRequest->subscriptionInitComplete);

        PEG_TRACE_CSTRING(TRC_PROVIDERAGENT, Tracer::LEVEL2,
            "Processed the agent initialization message.");

        // Notify the cimserver that the provider agent is initialized.
        Uint32 messageLength = 0;
        _pipeToServer->writeBuffer((const char*)&messageLength, sizeof(Uint32));

#if defined(PEGASUS_OS_ZOS) && defined(PEGASUS_ZOS_SECURITY)
        // prepare and setup the thread-level security environment on z/OS
        // if security initialization fails
        startupCheckBPXServer(false);
        startupCheckMSC();
        if (!isZOSSecuritySetup())
        {
            Logger::put_l(Logger::ERROR_LOG, ZOS_SECURITY_NAME, Logger::FATAL,
                MessageLoaderParms(
                    "ProviderManager.ProviderAgent.ProviderAgent."
                        "UNINITIALIZED_SECURITY_SETUP.PEGASUS_OS_ZOS",
                    "Security environment could not be initialised. "
                        "Assume security fraud. Stopping Provider Agent."));
            exit(1);
        }
#endif
        // provider agent is initialised and ready to go
        _isInitialised = true;
    }
    else if (request->getType() == CIM_NOTIFY_CONFIG_CHANGE_REQUEST_MESSAGE)
    {
        // Process the request in this thread
        AutoPtr<CIMNotifyConfigChangeRequestMessage> notifyRequest(
            dynamic_cast<CIMNotifyConfigChangeRequestMessage*>(request));
        PEGASUS_ASSERT(notifyRequest.get() != 0);

        //
        // Update the ConfigManager with the new property value
        //
        ConfigManager* configManager = ConfigManager::getInstance();
        CIMException responseException;
        try
        {
            if (notifyRequest->currentValueModified)
            {
                String userName = ((IdentityContainer)
                    request->operationContext.get(
                        IdentityContainer::NAME)).getUserName();

                configManager->updateCurrentValue(
                    notifyRequest->propertyName,
                    notifyRequest->newPropertyValue,
                    userName,
                    0,
                    false);
            }
            else
            {
                configManager->updatePlannedValue(
                    notifyRequest->propertyName,
                    notifyRequest->newPropertyValue,
                    true);
            }
        }
        catch (Exception& e)
        {
            responseException = PEGASUS_CIM_EXCEPTION(
                CIM_ERR_FAILED, e.getMessage());
        }

        AutoPtr<CIMResponseMessage> response(notifyRequest->buildResponse());
        response->cimException = responseException;

        // Return response to CIM Server
        _writeResponse(response.get());
    }
    else if (request->getType() == CIM_DISABLE_MODULE_REQUEST_MESSAGE)
    {
        // Process the request in this thread
        AutoPtr<Message> response(_processRequest(request));
        _writeResponse(response.get());

        CIMResponseMessage * respMsg =
            dynamic_cast<CIMResponseMessage*>(response.get());

        // If DisableModule not successful, leave agent process running.
        // If there are any active providers after DisableModule request
        // successful, this agent might be servicing the group of
        // provider modules, leave agent process running.
        if (((request->getType() == CIM_DISABLE_MODULE_REQUEST_MESSAGE) &&
             (!_providerManagerRouter.hasActiveProviders()) &&
             (respMsg->cimException.getCode() == CIM_ERR_SUCCESS)))
        {
            // Operation is successful. End the agent process.
            _providersStopped = true;
            _terminating = true;
        }

        delete request;
    }
    else if (request->getType() == CIM_STOP_ALL_PROVIDERS_REQUEST_MESSAGE)
    {
        _processStopAllProvidersRequest(request);
    }
    else
    {
        // Start a new thread to process the request
        ProviderAgentRequest* agentRequest =
            new ProviderAgentRequest(this, request);
        ThreadStatus rtn = PEGASUS_THREAD_OK;

        while ((rtn = _threadPool.allocate_and_awaken(agentRequest,
                   ProviderAgent::_processRequestAndWriteResponse)) !=
               PEGASUS_THREAD_OK)
        {
            // Yield only for the following request.
            // CIM_INITIALIZE_PROVIDER_AGENT_REQUEST_MESSAGE,
            // CIM_NOTIFY_CONFIG_CHANGE_REQUEST_MESSAGE,
            // CIM_DISABLE_MODULE_REQUEST_MESSAGE,
            // CIM_STOP_ALL_PROVIDERS_REQUEST_MESSAGE,
            // CIM_SUBSCRIPTION_INIT_COMPLETE_REQUEST_MESSAGE,
            // CIM_INDICATION_SERVICE_DISABLED_REQUEST_MESSAGE,
            // CIM_EXPORT_INDICATION_REQUEST_MESSAGE
            // All the above have already been handled differently
            // except for CIM_EXPORT_INDICATION_REQUEST_MESSAGE,
            // CIM_SUBSCRIPTION_INIT_COMPLETE_REQUEST_MESSAGE and
            // CIM_INDICATION_SERVICE_DISABLED_REQUEST_MESSAGE.
            if (rtn == PEGASUS_THREAD_INSUFFICIENT_RESOURCES &&
                (request->getType() == CIM_EXPORT_INDICATION_REQUEST_MESSAGE ||
                 request->getType() ==
                     CIM_SUBSCRIPTION_INIT_COMPLETE_REQUEST_MESSAGE ||
                 request->getType() ==
                     CIM_INDICATION_SERVICE_DISABLED_REQUEST_MESSAGE))
            {
                Threads::yield();
            }
            else
            {
                PEG_TRACE_CSTRING(TRC_PROVIDERAGENT, Tracer::LEVEL1,
                    "Could not allocate thread to process agent request.");
                MessageLoaderParms msgLoaderPrms(
                    "ProviderManager.ProviderAgent.ProviderAgent."
                        "THREAD_ALLOCATION_FAILED",
                    "Failed to allocate a thread in cimprovagt \"$0\".",
                    _agentId);

                AutoPtr<CIMResponseMessage> response(request->buildResponse());
                response->cimException = PEGASUS_CIM_EXCEPTION_L(
                    CIM_ERR_FAILED,msgLoaderPrms);

                // Return response to CIM Server
                _writeResponse(response.get());

                // make an entry in syslog for this behaviour.
                if(threadCreationFailureLogged.get() == 0)
                {
                    threadCreationFailureLogged.inc();
                    Logger::put_l(Logger::STANDARD_LOG,
                        System::CIMSERVER,Logger::WARNING,msgLoaderPrms);
                }

                delete agentRequest;
                delete request;

                PEG_METHOD_EXIT();
                return true;
            }
        }

        // Control will reach here only if the thread creation was successful.
        // Hence this is the right place to reset threadCreationFailureLogged
        // to that if the thread limit is reached again it is logged.
        if(threadCreationFailureLogged.get() == 1)
        {
            threadCreationFailureLogged.dec();
        }
    }

    PEG_METHOD_EXIT();
    return true;
}

void ProviderAgent::_processStopAllProvidersRequest(CIMRequestMessage* request)
{
    PEG_METHOD_ENTER(TRC_PROVIDERAGENT,
        "ProviderAgent::_processStopAllProvidersRequest");

    ProviderAgentRequest* agentRequest =
        new ProviderAgentRequest(this, request);
    Uint32 shutdownTimeout =
        ((CIMStopAllProvidersRequestMessage*)request)->shutdownTimeout;

    if ( _threadPool.allocate_and_awaken(agentRequest,
                   ProviderAgent::_processRequestAndWriteResponse) !=
               PEGASUS_THREAD_OK)
     {
         PEG_TRACE_CSTRING(TRC_PROVIDERAGENT, Tracer::LEVEL1,
             "Could not allocate thread to process "
                 "StopAllProvidersRequest. Exiting.");

         MessageLoaderParms msgLoaderPrms(
             "ProviderManager.ProviderAgent.ProviderAgent."
                    "THREAD_ALLOCATION_FAILED",
             "Failed to allocate a thread in cimprovagt \"$0\".",
             _agentId);

         Logger::put_l(
             Logger::STANDARD_LOG,
             System::CIMSERVER,
             Logger::WARNING,
             msgLoaderPrms);
         exit(1);
     }

    // Wait until shutdownTimeout-1 seconds expires or
    // CIMStopAllprovidersRequestMessage is processed successfully.
    if (shutdownTimeout)
    {
        for (Uint32 i = 0; !_providersStopped && i < shutdownTimeout - 1 ; ++i)
        {
            Threads::yield();
            Threads::sleep(1000);
        }
    }

    // If the shutdownTimeout expired, exit from here. Providers not
    // responding to the cleanup requests will cause this agent left as
    // orphaned process.
    // If there are agent threads running exit from here.If provider
    // is not responding cimprovagt may loop forever in ThreadPool
    // destructor waiting for running threads to become idle.
    if (!_providersStopped || _threadPool.runningCount())
    {
        MessageLoaderParms msgLoaderPrms(
            "ProviderManager.ProviderAgent.ProviderAgent."
                "PROVIDERS_FAILED_TO_CLEANUP",
            "Providers in the agent \"$0\" have failed to cleanup within \"$1\""
                " seconds during the shutdown. Provider agent terminated"
                " forcibly.",
            _agentId,
            shutdownTimeout);
        Logger::put_l(
            Logger::STANDARD_LOG,
            System::CIMSERVER,
            Logger::WARNING,
            msgLoaderPrms);
        exit(1);
    }
    _terminating = true;

    PEG_METHOD_EXIT();
}

inline void _completeHostNameAndNamespace(
    CIMRequestMessage* request,
    Message* response)
{
    // if defined in request, complete HostNameAndNamespace
    MessageType msgType = request->getType();

    if (msgType == CIM_ASSOCIATORS_REQUEST_MESSAGE ||
        msgType == CIM_ASSOCIATOR_NAMES_REQUEST_MESSAGE ||
        msgType == CIM_REFERENCES_REQUEST_MESSAGE ||
        msgType == CIM_REFERENCE_NAMES_REQUEST_MESSAGE ||
        (msgType == CIM_ENUMERATE_INSTANCES_REQUEST_MESSAGE &&
                        request->internalOperation) )
    {
        // can do this cast here since we know request to be one of the four
        // association request messages
        CIMOperationRequestMessage* reqMsg=
            (CIMOperationRequestMessage*) request;
        // Can use System::getHostName() reliably here since it was initialized
        // through the ConfigManager at start of ProviderAgent.
        CIMResponseDataMessage * rspMsg= (CIMResponseDataMessage*) response;
//EXP_PULL_BEGIN
        // Required to process responses for pull operations. This is really
        // a poor place to put this but it is efficient for the moment
        //  Set the internal vs external to the response message.
        rspMsg->internalOperation = reqMsg->internalOperation;
//EXP_PULL_END

        CIMResponseData & rspData = rspMsg->getResponseData();

        rspData.completeHostNameAndNamespace(
            System::getHostName(),
            reqMsg->nameSpace);
    }
}

Message* ProviderAgent::_processRequest(CIMRequestMessage* request)
{
    PEG_METHOD_ENTER(TRC_PROVIDERAGENT, "ProviderAgent::_processRequest");

    Message* response = 0;

    try
    {
        // Forward the request to the ProviderManager
        response = _providerManagerRouter.processMessage(request);

        // For some operations complete hostname and
        // namespace before the response data is binary encoded
        _completeHostNameAndNamespace(request,response);
    }
    catch (Exception& e)
    {
        PEG_TRACE((TRC_PROVIDERAGENT, Tracer::LEVEL1,
            "Caught exception while processing request: %s",
            (const char*)e.getMessage().getCString()));
        CIMResponseMessage* cimResponse = request->buildResponse();
        cimResponse->cimException = PEGASUS_CIM_EXCEPTION(
            CIM_ERR_FAILED, e.getMessage());
        response = cimResponse;
    }
    catch (...)
    {
        PEG_TRACE_CSTRING(TRC_PROVIDERAGENT, Tracer::LEVEL1,
            "Caught exception while processing request.");
        CIMResponseMessage* cimResponse = request->buildResponse();
        cimResponse->cimException = PEGASUS_CIM_EXCEPTION(
            CIM_ERR_FAILED, String::EMPTY);
        response = cimResponse;
    }

    PEG_METHOD_EXIT();
    return response;
}

void ProviderAgent::_processGetSCMOClassResponse(
    ProvAgtGetScmoClassResponseMessage* response)
{
    PEG_METHOD_ENTER(TRC_PROVIDERAGENT,
        "ProviderAgent::_processGetSCMOClassResponse");
    //
    // The provider agent requests a SCMOClass from the server by
    // _scmoClassCache_GetClass()
    //

    {
        AutoMutex mtx(_transferSCMOClassMutex);
        if (0 != _transferSCMOClass)
        {
             PEG_TRACE_CSTRING(TRC_PROVIDERAGENT, Tracer::LEVEL1,
                 "_transferSCMOClass was not cleand up. The previous "
                     "ProvAgtGetScmoClassRequest's might have been timed-out.");
             delete  _transferSCMOClass;
             _transferSCMOClass = 0;
        }

        // Copy class and messageID from response
        _transferSCMOClass = new SCMOClass(response->scmoClass);
        _transferSCMOClassRspMsgID = response->messageId;
    }

    // signal delivery of SCMOClass to _scmoClassCache_GetClass()
    _scmoClassDelivered.signal();

    PEG_METHOD_EXIT();
}

void ProviderAgent::_writeResponse(Message* message)
{
    PEG_METHOD_ENTER(TRC_PROVIDERAGENT, "ProviderAgent::_writeResponse");

    CIMMessage* response = dynamic_cast<CIMMessage*>(message);
    PEGASUS_ASSERT(response != 0);

    //
    // Write the response message to the pipe
    //
    try
    {
        // Use Mutex to prevent concurrent writes to the same pipe
        AutoMutex pipeLock(_pipeToServerMutex);

        AnonymousPipe::Status writeStatus =
            _pipeToServer->writeMessage(response);

        if (writeStatus != AnonymousPipe::STATUS_SUCCESS)
        {
            PEG_TRACE_CSTRING(TRC_PROVIDERAGENT, Tracer::LEVEL1,
                "Error writing response to pipe.");
            Logger::put_l(Logger::ERROR_LOG, System::CIMSERVER, Logger::WARNING,
                MessageLoaderParms(
                    "ProviderManager.ProviderAgent.ProviderAgent."
                        "CIMSERVER_COMMUNICATION_FAILED",
                    "cimprovagt \"$0\" communication with CIM Server failed.  "
                        "Exiting.",
                    _agentId));
            _terminating = true;
        }
    }
    catch (...)
    {
        PEG_TRACE_CSTRING(TRC_PROVIDERAGENT, Tracer::LEVEL1,
            "Caught exception while writing response.");
        Logger::put_l(Logger::ERROR_LOG, System::CIMSERVER, Logger::WARNING,
            MessageLoaderParms(
                "ProviderManager.ProviderAgent.ProviderAgent."
                    "CIMSERVER_COMMUNICATION_FAILED",
                "cimprovagt \"$0\" communication with CIM Server failed.  "
                    "Exiting.",
                _agentId));
        _terminating = true;
    }

    PEG_METHOD_EXIT();
}

ThreadReturnType PEGASUS_THREAD_CDECL
ProviderAgent::_processRequestAndWriteResponse(void* arg)
{
    PEG_METHOD_ENTER(TRC_PROVIDERAGENT,
        "ProviderAgent::_processRequestAndWriteResponse");

    AutoPtr<ProviderAgentRequest> agentRequest(
        reinterpret_cast<ProviderAgentRequest*>(arg));
    PEGASUS_ASSERT(agentRequest.get() != 0);

    try
    {
        // Get the ProviderAgent and request message from the argument
        ProviderAgent* agent = agentRequest->agent;
        AutoPtr<CIMRequestMessage> request(agentRequest->request);

        const AcceptLanguageListContainer acceptLang =
            request->operationContext.get(AcceptLanguageListContainer::NAME);
        Thread::setLanguages(acceptLang.getLanguages());

        // Process the request
        AutoPtr<Message> response(agent->_processRequest(request.get()));

        // Write the response
        agent->_writeResponse(response.get());

        if (response->getType() == CIM_STOP_ALL_PROVIDERS_RESPONSE_MESSAGE)
        {
            agent->_providersStopped = true;
        }
    }
    catch (const Exception& e)
    {
        PEG_TRACE((TRC_DISCARDED_DATA, Tracer::LEVEL1,
            "Exiting _processRequestAndWriteResponse. Caught Exception: %s",
            (const char*)e.getMessage().getCString()));
    }
    catch (...)
    {
        PEG_TRACE_CSTRING(TRC_DISCARDED_DATA, Tracer::LEVEL1,
            "Caught unrecognized exception.  "
                "Exiting _processRequestAndWriteResponse.");
    }

    PEG_METHOD_EXIT();
    return(ThreadReturnType(0));
}

void ProviderAgent::_handleIndicationDeliveryResponse(
    CIMProcessIndicationResponseMessage *response)
{
    IndicationRouter::notify(response);
}

void ProviderAgent::_indicationCallback(
    CIMProcessIndicationRequestMessage* message)
{
    PEG_METHOD_ENTER(TRC_PROVIDERAGENT, "ProviderAgent::_indicationCallback");
    IndicationRouter router =
        IndicationRouter(message, _indicationDeliveryRoutine);
    router.deliverAndWaitForStatus();
    PEG_METHOD_EXIT();
}

void ProviderAgent::_indicationDeliveryRoutine(
    CIMProcessIndicationRequestMessage* message)
{
    // Send request back to the server to process
    _providerAgent->_writeResponse(message);

    delete message;
}

void ProviderAgent::_responseChunkCallback(
    CIMRequestMessage* request, CIMResponseMessage* response)
{
    PEG_METHOD_ENTER(
        TRC_PROVIDERAGENT, "ProviderAgent::_responseChunkCallback");

    // Send request back to the server to process
    _providerAgent->_writeResponse(response);

    delete response;

    PEG_METHOD_EXIT();
}


Boolean ProviderAgent::_unloadIdleProviders()
{
    PEG_METHOD_ENTER(TRC_PROVIDERAGENT, "ProviderAgent::_unloadIdleProviders");
    try
    {
        ThreadStatus rtn = PEGASUS_THREAD_OK;

        //Ensure that only one _unloadIdleProvidersHandler thread runs at a time
        _unloadIdleProvidersBusy++;

        if ((_unloadIdleProvidersBusy.get() == 1) &&
                ((rtn =_threadPool.allocate_and_awaken(
                    (void*)this,
                    ProviderAgent::_unloadIdleProvidersHandler))
                 == PEGASUS_THREAD_OK))
        {
            // _unloadIdleProvidersBusy is decremented in
            // _unloadIdleProvidersHandler
        }
        else
        {
            // If we fail to allocate a thread, don't retry now.
            _unloadIdleProvidersBusy--;
        }
        if (rtn != PEGASUS_THREAD_OK)
        {
            PEG_TRACE_CSTRING(TRC_PROVIDERAGENT, Tracer::LEVEL1,
                    "Could not allocate thread to unload idle providers.");
        }

        // Wait for the cleanup thread to finish
        Uint32 shutdownTimeout = getShutdownTimeout();
        while (_unloadIdleProvidersBusy.get() > 0 && shutdownTimeout > 0 )
        {
            Threads::yield();
            Threads::sleep(1000);
            shutdownTimeout--;
        }
    }
    catch ( ...)
    {
         PEG_TRACE_CSTRING(TRC_PROVIDERAGENT, Tracer::LEVEL3,
             "Unexpected exception during unload idle providers.");

         PEG_METHOD_EXIT();
         return false;

    }

    PEG_METHOD_EXIT();
    return _unloadIdleProvidersBusy.get() == 0;
}

ThreadReturnType PEGASUS_THREAD_CDECL
ProviderAgent::_unloadIdleProvidersHandler(void* arg) throw()
{
    try
    {
        PEG_METHOD_ENTER(TRC_PROVIDERAGENT,
            "ProviderAgent::unloadIdleProvidersHandler");

        ProviderAgent* myself = reinterpret_cast<ProviderAgent*>(arg);

        try
        {
            myself->_providerManagerRouter.idleTimeCleanup();
        }
        catch (...)
        {
            // Ignore errors
            PEG_TRACE_CSTRING(TRC_PROVIDERAGENT, Tracer::LEVEL2,
                "Unexpected exception in _unloadIdleProvidersHandler");
        }

        myself->_unloadIdleProvidersBusy--;
    }
    catch (...)
    {
        // Ignore errors
        try
        {
            PEG_TRACE_CSTRING(TRC_PROVIDERAGENT, Tracer::LEVEL2,
                "Unexpected exception in _unloadIdleProvidersHandler");
        }
        catch (...)
        {
        }
    }

    // PEG_METHOD_EXIT();    // Note: This statement could throw an exception
    return ThreadReturnType(0);
}

void ProviderAgent::_terminateSignalHandler(
    int s_n, PEGASUS_SIGINFO_T* s_info, void* sig)
{
    PEG_METHOD_ENTER(TRC_PROVIDERAGENT,
        "ProviderAgent::_terminateSignalHandler");

    if (_providerAgent != 0)
    {
        _providerAgent->_terminating = true;
    }

    PEG_METHOD_EXIT();
}

SCMOClass ProviderAgent::_scmoClassCache_GetClass(
    const CIMNamespaceName& nameSpace,
    const CIMName& className)
{

    PEG_METHOD_ENTER(TRC_PROVIDERAGENT,
        "ProviderAgent::_scmoClassCache_GetClass");

    String requestID = XmlWriter::getNextMessageId();
    // create message
    ProvAgtGetScmoClassRequestMessage* message =
        new ProvAgtGetScmoClassRequestMessage(
        requestID,
        nameSpace,
        className,
        QueueIdStack());

    // Send the request for the SCMOClass to the server
    _providerAgent->_writeResponse(message);

    delete message;

    SCMOClass scmoClass = SCMOClass("","");

    for(;;)
    {
        // Wait for semaphore signaled by _readAndProcessRequest()
        if (!_scmoClassDelivered.time_wait(
            PEGASUS_DEFAULT_CLIENT_TIMEOUT_MILLISECONDS))
        {
            PEG_TRACE((TRC_DISCARDED_DATA, Tracer::LEVEL1,
                "Timed-out waiting for SCMOClass for "
                    "Name Space Name '%s' Class Name '%s'",
                (const char*)nameSpace.getString().getCString(),
                (const char*)className.getString().getCString()));
            break;
        }


        AutoMutex mtx(_transferSCMOClassMutex);
        if ( 0 == _transferSCMOClass)
        {
            PEG_TRACE((TRC_DISCARDED_DATA, Tracer::LEVEL1,
                "No SCMOClass received for Name Space Name '%s' "
                    "Class Name '%s'",
                (const char*)nameSpace.getString().getCString(),
                (const char*)className.getString().getCString()));
            break;
        }

        // Verify if we have actually received the response for our
        // request. This may happen when previous requests have timed out.
        if (_transferSCMOClassRspMsgID != requestID)
        {
            delete _transferSCMOClass;
            _transferSCMOClass = 0;
            continue;
        }

        // Create a local copy.
        scmoClass = SCMOClass(*_transferSCMOClass);

        // Delete the transferred instance.
        delete _transferSCMOClass;
        _transferSCMOClass = 0;
        break;
    }
    PEG_METHOD_EXIT();

    return scmoClass;
}

//
// PASE environment for synchronousSignal and asynchronousSignal
//
#ifdef PEGASUS_OS_PASE
void ProviderAgent::_synchronousSignalHandler(
    int s_n, PEGASUS_SIGINFO_T* s_info, void* sig)
{
    static bool mark = false;

    if (mark)
        exit(1);

    mark = true;
    if (_providerAgent != 0)
    {
        _providerAgent->_terminating = true;
    }

    char fullJobName[29];
    umeGetJobName(fullJobName, true);
    Logger::put_l(Logger::ERROR_LOG, "provider agent", Logger::SEVERE,
        MessageLoaderParms(
            "ProviderManager.ProviderAgent.RECEIVE_SYN_SIGNAL.PEGASUS_OS_PASE",
            "$0 received synchronous signal: $1", fullJobName, s_n));
}

void ProviderAgent::_asynchronousSignalHandler(
    int s_n, PEGASUS_SIGINFO_T* s_info, void* sig)
{
    static bool amark = false;

    if (amark)
        exit(1);

    amark = true;
    if (_providerAgent != 0)
    {
        _providerAgent->_terminating = true;
    }

    char fullJobName[29];
    umeGetJobName(fullJobName, true);
    Logger::put_l(Logger::ERROR_LOG, "provider agent", Logger::SEVERE,
        MessageLoaderParms(
            "ProviderManager.ProviderAgent.RECEIVE_ASYN_SIGNAL.PEGASUS_OS_PASE",
            "$0 received asynchronous signal: $1", fullJobName, s_n));
}
#endif

PEGASUS_NAMESPACE_END
