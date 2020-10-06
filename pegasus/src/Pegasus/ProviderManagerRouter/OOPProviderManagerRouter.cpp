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
#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/Constants.h>
#include <Pegasus/Common/AutoPtr.h>
#include <Pegasus/Common/ArrayInternal.h>
#include <Pegasus/Common/CIMMessage.h>
#include <Pegasus/Common/OperationContextInternal.h>
#include <Pegasus/Common/System.h>
#include <Pegasus/Common/AnonymousPipe.h>
#include <Pegasus/Common/Tracer.h>
#include <Pegasus/Common/Logger.h>
#include <Pegasus/Common/Thread.h>
#include <Pegasus/Common/MessageQueueService.h>
#include <Pegasus/Config/ConfigManager.h>
#include <Pegasus/Common/Executor.h>
#include <Pegasus/Common/StringConversion.h>
#include <Pegasus/Common/SCMOClassCache.h>

#if defined (PEGASUS_OS_TYPE_WINDOWS)
# include <windows.h>  // For CreateProcess()
#elif defined (PEGASUS_OS_VMS)
# include <perror.h>
# include <climsgdef.h>
# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <processes.h>
# include <unixio.h>
#else
# include <unistd.h>  // For fork(), exec(), and _exit()
# include <errno.h>
# include <sys/types.h>
# include <sys/resource.h>
# if defined(PEGASUS_HAS_SIGNALS)
#  include <sys/wait.h>
# endif
#endif

#include "OOPProviderManagerRouter.h"

PEGASUS_USING_STD;

PEGASUS_NAMESPACE_BEGIN

static String _GROUP_PREFIX = "grp:";
static String _MODULE_PREFIX = "mod:";

static struct timeval deallocateWait = {300, 0};

// This calss is used to aggregate the responses sent when a single requests can
// result in many responses and these responses need to be aggregated before a
// response is sent back to the ProviderManageService.
class RespAggCounter
{
public:
    RespAggCounter(Uint32 count):
        _expectedResponseCount(count),
        _receivedResponseCount(0)
    {
    }

    Boolean isComplete(CIMException &e)
    {
        AutoMutex mtx(_mutex);
        if (e.getCode()  != CIM_ERR_SUCCESS)
        {
            _exception = e;
        }
        _receivedResponseCount++;
        return _receivedResponseCount == _expectedResponseCount ;
    }

    CIMException getException()
    {
        return _exception;
    }

private:
    Mutex _mutex;
    Uint32 _expectedResponseCount, _receivedResponseCount ;
    CIMException _exception;
};


/////////////////////////////////////////////////////////////////////////////
// OutstandingRequestTable and OutstandingRequestEntry
/////////////////////////////////////////////////////////////////////////////

/**
    An OutstandingRequestEntry represents a request message sent to a
    Provider Agent for which no response has been received.  The request
    sender provides the message ID and a location for the response to be
    returned. When a response matching the message ID is received, the
    OutstandingRequestEntry is updated to indicate that the response
    will arrive asynchronously. This entry will be deleted
    when the response arrives.  */
class OutstandingRequestEntry
{
public:
    OutstandingRequestEntry(
        String originalMessageId_,
        CIMRequestMessage* requestMessage_,
        CIMResponseMessage*& responseMessage_,
        RespAggCounter* respAggregator_=NULL)
        : originalMessageId(originalMessageId_),
          requestMessage(requestMessage_),
          responseMessage(responseMessage_),
          respAggregator(respAggregator_)
    {
    }

    /**
        A unique value is substituted as the request messageId attribute to
        allow responses to be definitively correllated with requests.
        The original messageId value is stored here to avoid a race condition
        between the processing of a response chunk and the resetting of the
        original messageId in the request message.
     */
    String originalMessageId;
    CIMRequestMessage* requestMessage;
    CIMResponseMessage*& responseMessage;

    // The aggregator object which aggregates the responses for requests
    // like CIM_STOP_ALL_PROVIDERS_REQUEST_MESSAGE etc.
    RespAggCounter* respAggregator;
};

typedef HashTable<String, SharedPtr<OutstandingRequestEntry>, EqualFunc<String>,
    HashFunc<String> > OutstandingRequestTable;

class RetryThreadParam{
public:
    ProviderAgentContainer *pac;
    Array<CIMRequestMessage *> retryRequestArray;
};


/////////////////////////////////////////////////////////////////////////////
// ProviderAgentContainer
/////////////////////////////////////////////////////////////////////////////

class ProviderAgentContainer
{
public:
    ProviderAgentContainer(
        Uint16 bitness,
        const String & groupNameWithType,
        const String & userName,
        Uint16 userContext,
        PEGASUS_INDICATION_CALLBACK_T indicationCallback,
        PEGASUS_RESPONSE_CHUNK_CALLBACK_T responseChunkCallback,
        PEGASUS_PROVIDERMODULEGROUPFAIL_CALLBACK_T
            providerModuleGroupFailCallback,
        PEGASUS_ASYNC_RESPONSE_CALLBACK_T asyncResponseCallback,
        ThreadPool * threadPool);

    ~ProviderAgentContainer();

    Boolean isInitialized();

    String getGroupNameWithType() const;

    CIMResponseMessage* processMessage(CIMRequestMessage* request,
        RespAggCounter * respAggregator=NULL);

    void unloadIdleProviders();

    /**
        Check if the pending responses in the _outstandingRequestTable
        have active client connections. If not then create a response
        to indicate that this client connection can be closed. The
        entry for this request is also deleted from the
        _outstandingRequestTable. This function is called at regular
        intervals along with unloadIdleProviders
    */
    void cleanDisconnectedClientRequests();
    /**
        Find a pull request (internalOperation) that matches the messageId
        provided and if found, create a response to indicate that this provider
        is complete.  Also delete this request from the _outstandingRequestTable
        This function is only called from the EnumerationContextTable when
        that code concludes that a provider is "stuck" (i.e. it has not
        returned any provider responses in a reasonable time.  Probably the
        real case for this is where an OOP provider fails and is restarted
        so that the provider state is lost during the execution of an
        enumeration, assoc, etc. operation.  This differs from the original
        non-pull operations that use the cleanDisconnectedClientRequests for
        the same purpose.
    */
    void cleanClosedPullRequests(const String& messageId);
    static void setAllProvidersStopped();
    static void setSubscriptionInitComplete(Boolean value);
    void sendResponse(CIMResponseMessage *response);
private:
    //
    // Private methods
    //

    /** Unimplemented */
    ProviderAgentContainer();
    /** Unimplemented */
    ProviderAgentContainer(const ProviderAgentContainer& pa);
    /** Unimplemented */
    ProviderAgentContainer& operator=(const ProviderAgentContainer& pa);

    /**
        Start a Provider Agent process and establish a pipe connection with it.
        Note: The caller must lock the _agentMutex.
     */
    void _startAgentProcess();

    /**
        Send initialization data to the Provider Agent.
        Note: The caller must lock the _agentMutex.
     */
    void _sendInitializationData();

    /**
        Initialize the ProviderAgentContainer if it is not already
        initialized.  Initialization includes starting the Provider Agent
        process, establishing a pipe connection with it, and starting a
        thread to read response messages from the Provider Agent.

        Note: The caller must lock the _agentMutex.
     */
    void _initialize();

    /**
        Uninitialize the ProviderAgentContainer if it is initialized.
        The connection is closed and outstanding requests are completed
        with an error result.

        @param cleanShutdown Indicates whether the provider agent process
        exited cleanly.  A value of true indicates that responses have been
        sent for all requests that have been processed.  A value of false
        indicates that one or more requests may have been partially processed.
     */
    void _uninitialize(Boolean cleanShutdown);

    /**
        Performs the processMessage work, but does not retry on a transient
        error.
     */
    CIMResponseMessage* _processMessage(CIMRequestMessage* request,
        RespAggCounter *respAggregator);

    /**
        Read and process response messages from the Provider Agent until
        the connection is closed.
     */
    void _processResponses();

    void _sendResponse(CIMRequestMessage *request,
        CIMResponseMessage *response);

    static ThreadReturnType PEGASUS_THREAD_CDECL
        _responseProcessor(void* arg);

    /**
        Process the ProvAgtGetScmoClassRequestMessage and sends the
        requested SCMOClass back to the agent.
     */
    void _processGetSCMOClassRequest(
        ProvAgtGetScmoClassRequestMessage* request);

    /**
      This function will fetch the bottom most queueid from the
      QueueIdStack of the request message and check if the queue isActive().
    */
    Boolean _isClientActive(CIMRequestMessage *request_);

    /**
       This thread will retry the request if the provider goes down
       before the request is processed.
    */
    static ThreadReturnType PEGASUS_THREAD_CDECL _retryRequestHandler(
        void* arg) ;

    //
    // Private data
    //

    /**
        The _agentMutex must be locked whenever writing to the Provider
        Agent connection, accessing the _isInitialized flag, or changing
        the Provider Agent state.
     */
    Mutex _agentMutex;

    Uint16 _bitness;

    /**
        Name of the provider module or group served by this Provider Agent.
     */
    String _moduleOrGroupName;

    /**
        Name of the group with type(group or module indicator)
        served by this Provider Agent.
     */
    String _groupNameWithType;

    /**
        The user context in which this Provider Agent operates.
     */
    String _userName;

    /**
        User Context setting of the provider module served by this Provider
        Agent.
     */
    Uint16 _userContext;

    /**
        Callback function to which all generated indications are sent for
        processing.
     */
    PEGASUS_INDICATION_CALLBACK_T _indicationCallback;

    /**
        Callback function to which response chunks are sent for processing.
     */
    PEGASUS_RESPONSE_CHUNK_CALLBACK_T _responseChunkCallback;

    /**
        Callback function to be called upon detection of failure of a
        provider module.
     */
    PEGASUS_PROVIDERMODULEGROUPFAIL_CALLBACK_T _providerModuleGroupFailCallback;

    /**
        Callback function for async response.
    */
    PEGASUS_ASYNC_RESPONSE_CALLBACK_T _asyncResponseCallback;

    /**
        Indicates whether the Provider Agent is active.
     */
    Boolean _isInitialized;

    /**
        Pipe connection used to read responses from the Provider Agent.
     */
    AutoPtr<AnonymousPipe> _pipeFromAgent;
    /**
        Pipe connection used to write requests to the Provider Agent.
     */
    AutoPtr<AnonymousPipe> _pipeToAgent;

#if defined(PEGASUS_HAS_SIGNALS)
    /**
        Process ID of the active Provider Agent.
     */
    pid_t _pid;
#endif

    /**
        The _outstandingRequestTable holds an entry for each request that has
        been sent to this Provider Agent for which no response has been
        received.  Entries are added (by the writing thread) when a request
        is sent, and are removed (by the reading thread) when the response is
        received (or when it is determined that no response is forthcoming).
     */
    OutstandingRequestTable _outstandingRequestTable;
    /**
        The _outstandingRequestTableMutex must be locked whenever reading or
        updating the _outstandingRequestTable.
     */
    Mutex _outstandingRequestTableMutex;

    /**
        Holds the last provider module instance sent to the Provider Agent in
        a ProviderIdContainer.  Since the provider module instance rarely
        changes, an optimization is used to send it only when it differs from
        the last provider module instance sent.
     */
    CIMInstance _providerModuleCache;

    /**
        The number of Provider Agent processes that are currently initialized
        (active).
    */
    static Uint32 _numProviderProcesses;

    /**
        The _numProviderProcessesMutex must be locked whenever reading or
        updating the _numProviderProcesses count.
    */
    static Mutex _numProviderProcessesMutex;

    /**
        A value indicating that a request message has not been processed.
        A CIMResponseMessage pointer with this value indicates that the
        corresponding CIMRequestMessage has not been processed.  This is
        used to indicate that a provider agent exited without starting to
        process the request, and that the request should be retried.
     */
    static CIMResponseMessage* _REQUEST_NOT_PROCESSED;

    /**
        Indicates whether the Indication Service has completed initialization.

        For more information, please see the description of the
        ProviderManagerRouter::_subscriptionInitComplete member variable.
     */
    static Boolean _subscriptionInitComplete;


    /**
        OOPProviderManagerRouter ThreadPool pointer.
    */
    ThreadPool* _threadPool;

    static Boolean _allProvidersStopped;
};

Uint32 ProviderAgentContainer::_numProviderProcesses = 0;
Mutex ProviderAgentContainer::_numProviderProcessesMutex;
Boolean ProviderAgentContainer::_allProvidersStopped = false;
Boolean ProviderAgentContainer::_subscriptionInitComplete = false;

// Set this to a value that no valid CIMResponseMessage* will have.
CIMResponseMessage* ProviderAgentContainer::_REQUEST_NOT_PROCESSED =
    static_cast<CIMResponseMessage*>((void*)&_REQUEST_NOT_PROCESSED);

ProviderAgentContainer::ProviderAgentContainer(
    Uint16 bitness,
    const String & groupName,
    const String & userName,
    Uint16 userContext,
    PEGASUS_INDICATION_CALLBACK_T indicationCallback,
    PEGASUS_RESPONSE_CHUNK_CALLBACK_T responseChunkCallback,
    PEGASUS_PROVIDERMODULEGROUPFAIL_CALLBACK_T providerModuleGroupFailCallback,
    PEGASUS_ASYNC_RESPONSE_CALLBACK_T asyncResponseCallback,
    ThreadPool* threadPool)
    :
      _bitness(bitness),
      _groupNameWithType(groupName),
      _userName(userName),
      _userContext(userContext),
      _indicationCallback(indicationCallback),
      _responseChunkCallback(responseChunkCallback),
      _providerModuleGroupFailCallback(providerModuleGroupFailCallback),
      _asyncResponseCallback(asyncResponseCallback),
      _isInitialized(false),
      _threadPool(threadPool)
{
    PEG_METHOD_ENTER(TRC_PROVIDERMANAGER,
        "ProviderAgentContainer::ProviderAgentContainer");

    // Remove prefixes "grp" and "mod" and get actual module or group name.
    _moduleOrGroupName = _groupNameWithType.subString(4);

    PEG_METHOD_EXIT();
}

ProviderAgentContainer::~ProviderAgentContainer()
{
    PEG_METHOD_ENTER(TRC_PROVIDERMANAGER,
        "ProviderAgentContainer::~ProviderAgentContainer");

    // Ensure the destructor does not throw an exception
    try
    {
        if (isInitialized())
        {
            {
                AutoMutex lock(_agentMutex);
                // Check if the _pipeFromAgent is alive.
                if( _pipeFromAgent.get() != 0 )
                {
                    // Stop the responseProcessor thread by closing its
                    // connection.
                    _pipeFromAgent->closeReadHandle();
                }
            }

            // Wait for the responseProcessor thread to exit
            while (isInitialized())
            {
                Threads::yield();
            }
        }
    }
    catch (...)
    {
    }

    PEG_METHOD_EXIT();
}

void ProviderAgentContainer::setAllProvidersStopped()
{
    _allProvidersStopped = true;
}

void ProviderAgentContainer::setSubscriptionInitComplete(Boolean value)
{
    _subscriptionInitComplete = value;
}

void ProviderAgentContainer::_startAgentProcess()
{
    PEG_METHOD_ENTER(
        TRC_PROVIDERMANAGER, "ProviderAgentContainer::_startAgentProcess");

    // Start the provider agent.

    int pid;
    AnonymousPipe* readPipe;
    AnonymousPipe* writePipe;
    int status = Executor::startProviderAgent(
        (unsigned short)_bitness,
        (const char*)_moduleOrGroupName.getCString(),
        ConfigManager::getPegasusHome(),
        _userName,
        pid,
        readPipe,
        writePipe);

    if (status != 0)
    {
        PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL1,
            "Executor::startProviderAgent() failed"));
        PEG_METHOD_EXIT();
        throw Exception(MessageLoaderParms(
            "ProviderManager.OOPProviderManagerRouter.CIMPROVAGT_START_FAILED",
            "Failed to start cimprovagt \"$0\".",
            _moduleOrGroupName));
    }

# if defined(PEGASUS_HAS_SIGNALS)
    _pid = pid;
# endif

    _pipeFromAgent.reset(readPipe);
    _pipeToAgent.reset(writePipe);

    PEG_METHOD_EXIT();
}

// Note: Caller must lock _agentMutex
void ProviderAgentContainer::_sendInitializationData()
{
    PEG_METHOD_ENTER(TRC_PROVIDERMANAGER,
        "ProviderAgentContainer::_sendInitializationData");

    //
    // Gather config properties to pass to the Provider Agent
    //
    ConfigManager* configManager = ConfigManager::getInstance();
    Array<Pair<String, String> > configProperties;

    Array<String> configPropertyNames;
    configManager->getAllPropertyNames(configPropertyNames, true);
    for (Uint32 i = 0; i < configPropertyNames.size(); i++)
    {
        String configPropertyValue =
            configManager->getCurrentValue(configPropertyNames[i]);
        String configPropertyDefaultValue =
            configManager->getDefaultValue(configPropertyNames[i]);
        if (configPropertyValue != configPropertyDefaultValue)
        {
            configProperties.append(Pair<String, String>(
                configPropertyNames[i], configPropertyValue));
        }
    }

    //
    // Create a Provider Agent initialization message
    //
    AutoPtr<CIMInitializeProviderAgentRequestMessage> request(
        new CIMInitializeProviderAgentRequestMessage(
            String("0"),    // messageId
            configManager->getPegasusHome(),
            configProperties,
            System::bindVerbose,
            _subscriptionInitComplete,
            QueueIdStack()));

    //
    // Write the initialization message to the pipe
    //
    AnonymousPipe::Status writeStatus =
        _pipeToAgent->writeMessage(request.get());

    if (writeStatus != AnonymousPipe::STATUS_SUCCESS)
    {
        PEG_METHOD_EXIT();
        throw Exception(MessageLoaderParms(
            "ProviderManager.OOPProviderManagerRouter."
                "CIMPROVAGT_COMMUNICATION_FAILED",
            "Failed to communicate with cimprovagt \"$0\".",
            _moduleOrGroupName));
    }

    // Wait for a null response from the Provider Agent indicating it has
    // initialized successfully.

    CIMMessage* message;
    AnonymousPipe::Status readStatus;
    do
    {
        readStatus = _pipeFromAgent->readMessage(message);

    } while (readStatus == AnonymousPipe::STATUS_INTERRUPT);

    if (readStatus != AnonymousPipe::STATUS_SUCCESS)
    {
        PEG_METHOD_EXIT();
        throw Exception(MessageLoaderParms(
            "ProviderManager.OOPProviderManagerRouter."
                "CIMPROVAGT_COMMUNICATION_FAILED",
            "Failed to communicate with cimprovagt \"$0\".",
            _moduleOrGroupName));
    }

    PEGASUS_ASSERT(message == 0);

    PEG_METHOD_EXIT();
}

// Note: Caller must lock _agentMutex
void ProviderAgentContainer::_initialize()
{
    PEG_METHOD_ENTER(TRC_PROVIDERMANAGER,
        "ProviderAgentContainer::_initialize");

    if (_isInitialized)
    {
        PEGASUS_ASSERT(0);
        PEG_METHOD_EXIT();
        return;
    }

    //Get the current value of maxProviderProcesses
    String maxProviderProcessesString = ConfigManager::getInstance()->
        getCurrentValue("maxProviderProcesses");
    Uint64 v;
    StringConversion::decimalStringToUint64(
        maxProviderProcessesString.getCString(),
        v);
    Uint32 maxProviderProcesses = (Uint32)v;

    {
        AutoMutex lock(_numProviderProcessesMutex);

        if ((maxProviderProcesses != 0) &&
            (_numProviderProcesses >= maxProviderProcesses))
        {
            throw PEGASUS_CIM_EXCEPTION(
                CIM_ERR_FAILED,
                MessageLoaderParms(
                    "ProviderManager.OOPProviderManagerRouter."
                        "MAX_PROVIDER_PROCESSES_REACHED",
                    "The maximum number of cimprovagt processes has been "
                        "reached."));
        }
        else
        {
            _numProviderProcesses++;
        }
    }

    try
    {
        _startAgentProcess();
        _isInitialized = true;
        _sendInitializationData();

        // Start a thread to read and process responses from the Provider Agent
        ThreadStatus rtn = PEGASUS_THREAD_OK;
        while ((rtn = MessageQueueService::get_thread_pool()->
                   allocate_and_awaken(this, _responseProcessor)) !=
               PEGASUS_THREAD_OK)
        {
            if (rtn == PEGASUS_THREAD_INSUFFICIENT_RESOURCES)
            {
                Threads::yield();
            }
            else
            {
                PEG_TRACE_CSTRING(TRC_PROVIDERMANAGER, Tracer::LEVEL1,
                    "Could not allocate thread to process responses from the "
                        "provider agent.");

                throw Exception(MessageLoaderParms(
                    "ProviderManager.OOPProviderManagerRouter."
                        "CIMPROVAGT_THREAD_ALLOCATION_FAILED",
                    "Failed to allocate thread for cimprovagt \"$0\".",
                    _moduleOrGroupName));
            }
        }
    }
    catch (...)
    {
        // Closing the connection causes the agent process to exit
        _pipeToAgent.reset();
        _pipeFromAgent.reset();

#if defined(PEGASUS_HAS_SIGNALS)
        if (_isInitialized)
        {
            // Harvest the status of the agent process to prevent a zombie
            int status = Executor::reapProviderAgent(_pid);

            if (status == -1)
            {
                PEG_TRACE((TRC_DISCARDED_DATA, Tracer::LEVEL1,
                    "ProviderAgentContainer::_initialize(): "
                        "Executor::reapProviderAgent() failed"));
            }
        }
#endif

        _isInitialized = false;

        {
            AutoMutex lock(_numProviderProcessesMutex);
            _numProviderProcesses--;
        }

        PEG_METHOD_EXIT();
        throw;
    }

    PEG_METHOD_EXIT();
}

Boolean ProviderAgentContainer::isInitialized()
{
    AutoMutex lock(_agentMutex);
    return _isInitialized;
}

void ProviderAgentContainer::_uninitialize(Boolean cleanShutdown)
{
    PEG_METHOD_ENTER(TRC_PROVIDERMANAGER,
        "ProviderAgentContainer::_uninitialize");

#if defined(PEGASUS_HAS_SIGNALS)
    pid_t pid = 0;
#endif

    try
    {
        CIMException cimException;
        if (!cleanShutdown)
        {
            cimException = PEGASUS_CIM_EXCEPTION(
                CIM_ERR_FAILED,
                MessageLoaderParms(
                    "ProviderManager.OOPProviderManagerRouter."
                        "CIMPROVAGT_CONNECTION_LOST",
                    "Lost connection with cimprovagt \"$0\".",
                    _moduleOrGroupName));
        }

        AutoMutex lock(_agentMutex);

        PEGASUS_ASSERT(_isInitialized);

        // Close the connection with the Provider Agent
        _pipeFromAgent.reset();
        _pipeToAgent.reset();

        _providerModuleCache = CIMInstance();

        {
            AutoMutex lock2(_numProviderProcessesMutex);
            _numProviderProcesses--;
        }

        _isInitialized = false;

#if defined(PEGASUS_HAS_SIGNALS)
        // Save the _pid so we can use it after we've released the _agentMutex
        pid = _pid;
#endif

        // In case of a clean shutdown requests which could not be processed are
        // retried in a new thread.
        Array<CIMRequestMessage *> retryReqArray;

        //
        // Complete with null responses all outstanding requests on this
        // connection
        //
        {
            AutoMutex tableLock(_outstandingRequestTableMutex);

            for (OutstandingRequestTable::Iterator i =
                     _outstandingRequestTable.start();
                 i != 0; i++)
            {
                Boolean sendResponseNow = false;
                CIMResponseMessage *response = 0;

                MessageType msgType = i.value()->requestMessage->getType();

                // Note: Whether this agent was shutdown cleanly or not,
                // for the below requests wait until all responses are
                // received.
                if(msgType == CIM_STOP_ALL_PROVIDERS_REQUEST_MESSAGE ||
                    msgType == CIM_NOTIFY_CONFIG_CHANGE_REQUEST_MESSAGE ||
                    msgType ==
                        CIM_SUBSCRIPTION_INIT_COMPLETE_REQUEST_MESSAGE ||
                    msgType ==
                        CIM_INDICATION_SERVICE_DISABLED_REQUEST_MESSAGE ||
                    msgType == CIM_ENABLE_MODULE_REQUEST_MESSAGE ||
                    msgType == CIM_DISABLE_MODULE_REQUEST_MESSAGE)
                {
                    PEGASUS_ASSERT(i.value()->respAggregator);
                    if(i.value()->respAggregator->isComplete(cimException))
                    {
                        CIMException cimException;
                        sendResponseNow = true;

                        if (msgType == CIM_ENABLE_MODULE_REQUEST_MESSAGE ||
                            msgType == CIM_DISABLE_MODULE_REQUEST_MESSAGE)
                        {
                            CIMException e =
                                i.value()->respAggregator->getException();
                            if (e.getCode() == CIM_ERR_SUCCESS)
                            {
                                retryReqArray.append(i.value()->requestMessage);
                                sendResponseNow = false;
                            }
                            else
                            {
                                cimException = e;
                            }
                        }

                        if (sendResponseNow)
                        {
                            response =
                                i.value()->requestMessage->buildResponse();
                            response->messageId = i.value()->originalMessageId;
                            response->cimException = cimException;
                            sendResponseNow = true;
                        }
                        delete i.value()->respAggregator;
                    }
                }
                else if (msgType == CIM_DELETE_SUBSCRIPTION_REQUEST_MESSAGE)
                {
                    response = i.value()->requestMessage->buildResponse();
                    response->messageId = i.value()->originalMessageId;
                    sendResponseNow = true;
                }
                else if (cleanShutdown)
                {
                    // retry the request
                    retryReqArray.append(i.value()->requestMessage);
                }
                else
                {
                    // Requests with respAggregator set were already handled
                    // before.
                    PEGASUS_ASSERT(!i.value()->respAggregator);
                    response = i.value()->requestMessage->buildResponse();
                    response->cimException = cimException;
                    sendResponseNow = true;
                }

                if(sendResponseNow)
                {
                    PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL4,
                        "Completing messageId \"%s\" with a default response.",
                        (const char*)i.key().getCString()));

                    response->setComplete(true);
                    _asyncResponseCallback(
                        i.value()->requestMessage,
                        response);
                }
            }
            _outstandingRequestTable.clear();
        }

        if(retryReqArray.size() > 0 )
        {
            ThreadStatus rtn = PEGASUS_THREAD_OK;
            AutoPtr<RetryThreadParam> parms(new RetryThreadParam());
            parms->pac = this;
            parms->retryRequestArray = retryReqArray;

            Boolean didRetry = true;

            while((rtn = _threadPool->allocate_and_awaken(
                (void*)parms.release(),
                ProviderAgentContainer::_retryRequestHandler))
                != PEGASUS_THREAD_OK)
            {
                if(rtn == PEGASUS_THREAD_INSUFFICIENT_RESOURCES)
                {
                    Threads::yield();
                }
                else
                {
                    PEG_TRACE((TRC_PROVIDERMANAGER,
                        Tracer::LEVEL1,
                        "Could not allocate thread to retry "
                        "request in %s",
                        (const char *)_moduleOrGroupName. \
                        getCString()));
                    didRetry = false;
                }
            }

            if(!didRetry)
            {
                for(Uint32 i=0; i<retryReqArray.size(); i++)
                {
                    CIMResponseMessage *response =
                        retryReqArray[i]->buildResponse();
                    response->setComplete(true);
                    response->cimException =
                        PEGASUS_CIM_EXCEPTION(
                            CIM_ERR_FAILED,
                            MessageLoaderParms("ProviderManager."
                                "OOPProviderManagerRouter."
                                "REQUEST_RETRY_THREAD_ALLOCATION_FAILED",
                                "Failed to allocate a thread to "
                                   "retry a request in \"$0\".",
                                _moduleOrGroupName));

                    _asyncResponseCallback(
                        retryReqArray[i],
                        response);
               }
            }
        }

        //
        //  If not a clean shutdown, call the provider module failure callback
        //
        if (!cleanShutdown)
        {
            //
            // Call the provider module failure callback to communicate
            // the failure to the Provider Manager Service.  The Provider
            // Manager Service will inform the Indication Service.
            //

            // If this agent is servicing the group of modules, get all related
            // provider module names.
            Boolean isGroup = !String::compare(
                _groupNameWithType, _GROUP_PREFIX, 4);

            _providerModuleGroupFailCallback(
                _moduleOrGroupName, _userName, _userContext, isGroup);
        }
    }
    catch (...)
    {
        // We're uninitializing, so do not propagate the exception
        PEG_TRACE_CSTRING(TRC_DISCARDED_DATA, Tracer::LEVEL2,
            "Ignoring _uninitialize() exception.");
    }

#if defined(PEGASUS_HAS_SIGNALS)
    // Harvest the status of the agent process to prevent a zombie.  Do not
    // hold the _agentMutex during this operation.

    if ((pid != 0) && (Executor::reapProviderAgent(pid) == -1))
    {
        PEG_TRACE((TRC_DISCARDED_DATA, Tracer::LEVEL2,
            "ProviderAgentContainer::_uninitialize(): "
                "Executor::reapProviderAgent() failed."));
    }
#endif

    PEG_METHOD_EXIT();
}

String ProviderAgentContainer::getGroupNameWithType() const
{
    return _groupNameWithType;
}

void ProviderAgentContainer::sendResponse(CIMResponseMessage *response)
{
    AutoMutex lock(_agentMutex);

    AnonymousPipe::Status writeStatus =
        _pipeToAgent->writeMessage(response);
    if (writeStatus != AnonymousPipe::STATUS_SUCCESS)
    {
        PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL1,
            "Failed to write message to pipe.  writeStatus = %d.",
            writeStatus));
    }
    delete response;
}

CIMResponseMessage* ProviderAgentContainer::processMessage(
    CIMRequestMessage* request,RespAggCounter* respAggregator)
{
    PEG_METHOD_ENTER(TRC_PROVIDERMANAGER,
        "ProviderAgentContainer::processMessage");

    CIMResponseMessage* response;
    MessageType msgType = request->getType();

    do
    {
        response = _processMessage(request,respAggregator);

        if (response == _REQUEST_NOT_PROCESSED)
        {
            // Check for request message types that should not be retried.
            if ((msgType == CIM_STOP_ALL_PROVIDERS_REQUEST_MESSAGE) ||
                (msgType == CIM_NOTIFY_CONFIG_CHANGE_REQUEST_MESSAGE) ||
                (msgType == CIM_SUBSCRIPTION_INIT_COMPLETE_REQUEST_MESSAGE) ||
                (msgType == CIM_INDICATION_SERVICE_DISABLED_REQUEST_MESSAGE) ||
                (msgType == CIM_DELETE_SUBSCRIPTION_REQUEST_MESSAGE))
            {
                response = request->buildResponse();
                break;
            }
            else if (msgType == CIM_DISABLE_MODULE_REQUEST_MESSAGE)
            {
                response = request->buildResponse();
                CIMDisableModuleResponseMessage* dmResponse =
                    dynamic_cast<CIMDisableModuleResponseMessage*>(response);
                PEGASUS_ASSERT(dmResponse != 0);

                Array<Uint16> operationalStatus;
                operationalStatus.append(CIM_MSE_OPSTATUS_VALUE_STOPPED);
                dmResponse->operationalStatus = operationalStatus;
                break;
            }
        }
    } while (response == _REQUEST_NOT_PROCESSED);

    PEG_METHOD_EXIT();
    return response;
}

CIMResponseMessage* ProviderAgentContainer::_processMessage(
    CIMRequestMessage* request, RespAggCounter *respAggregator)
{
    PEG_METHOD_ENTER(TRC_PROVIDERMANAGER,
        "ProviderAgentContainer::_processMessage");

    CIMResponseMessage* response;
    String originalMessageId = request->messageId;

    PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL3,
        "ProviderAgentContainer, process message ID %s",
        (const char*)request->messageId.getCString()));

    // These three variables are used for the provider module optimization.
    // See the _providerModuleCache member description for more information.
    AutoPtr<ProviderIdContainer> origProviderId;
    Boolean doProviderModuleOptimization = false;
    Boolean updateProviderModuleCache = false;

    try
    {
        // The messageId attribute is used to correlate response messages
        // from the Provider Agent with request messages, so it is imperative
        // that the ID is unique for each request.  The incoming ID cannot be
        // trusted to be unique, so we substitute a unique one.  The memory
        // address of the request is used as the source of a unique piece of
        // data.  (The message ID is only required to be unique while the
        // request is outstanding.)
        char messagePtrString[20];
        sprintf(messagePtrString, "%p", request);
        String uniqueMessageId = messagePtrString;

        // Trace to relate request messageId to provider internal messageId
        PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL3,
            "ProviderAgentContainer, request message ID %s "
                "provider internal messageId %s",
            (const char*)request->messageId.getCString(),
            messagePtrString ));
        //
        // Set up the OutstandingRequestEntry for this request
        //
        SharedPtr<OutstandingRequestEntry> outstandingRequestEntry(
            new OutstandingRequestEntry(
                originalMessageId,
                    request,
                    response,
                respAggregator));

        //
        // Lock the Provider Agent Container while initializing the
        // agent and writing the request to the connection
        //
        {
            AutoMutex lock(_agentMutex);

            // Don't process any other messages if _allProvidersStopped flag
            // is set. CIMServer hangs during the shutdown if the agent is
            // started to process a request after StopAllProviders request
            // has been processed. This scenario may happen if provider
            // generates indication during the shutdwon whose destination is
            // indication consumer provider running within cimserver.
            if (_allProvidersStopped &&
                request->getType() != CIM_STOP_ALL_PROVIDERS_REQUEST_MESSAGE)
            {
                //Note: l11n is not necessary, not propagated to client.
                CIMException e = CIMException(
                    CIM_ERR_FAILED,
                    "Request not processed, CIMServer shutting down");
                if (!respAggregator || respAggregator->isComplete(e))
                {

                    PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL1,
                        "Exception: %s",
                        (const char*)e.getMessage().getCString()));
                    response = request->buildResponse();
                    delete respAggregator;
                    PEG_METHOD_EXIT();
                    return response;
                }
            }

            //
            // Initialize the Provider Agent, if necessary
            //
            if (!_isInitialized)
            {
                _initialize();
            }

            //
            // Add an entry to the OutstandingRequestTable for this request
            //
            {
                AutoMutex tableLock(_outstandingRequestTableMutex);

                _outstandingRequestTable.insert(
                    uniqueMessageId, outstandingRequestEntry);
            }

            // Get the provider module from the ProviderIdContainer to see if
            // we can optimize out the transmission of this instance to the
            // Provider Agent.  (See the _providerModuleCache description.)
            if (request->operationContext.contains(ProviderIdContainer::NAME))
            {
                ProviderIdContainer pidc = request->operationContext.get(
                    ProviderIdContainer::NAME);
                origProviderId.reset(new ProviderIdContainer(
                    pidc.getModule(), pidc.getProvider(),
                    pidc.isRemoteNameSpace(), pidc.getRemoteInfo()));
                origProviderId->setProvMgrPath(pidc.getProvMgrPath());
                if (_providerModuleCache.isUninitialized() ||
                    (!pidc.getModule().identical(_providerModuleCache)))
                {
                    // We haven't sent this provider module instance to the
                    // Provider Agent yet.  Update our cache after we send it.
                    updateProviderModuleCache = true;
                }
                else
                {
                    // Replace the provider module in the ProviderIdContainer
                    // with an uninitialized instance.  We'll need to put the
                    // original one back after the message is sent.
                    ProviderIdContainer newpidc = ProviderIdContainer(
                        CIMInstance(), pidc.getProvider(),
                        pidc.isRemoteNameSpace(), pidc.getRemoteInfo());
                    newpidc.setProvMgrPath(pidc.getProvMgrPath());
                    request->operationContext.set(newpidc);

                    doProviderModuleOptimization = true;
                }
            }

            //
            // Write the message to the pipe
            //
            try
            {
                PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL3,
                    "Sending request to agent, "
                    "messageId %s internal messageId %s",
                    (const char*)request->messageId.getCString(),
                    (const char*)uniqueMessageId.getCString()));

                request->messageId = uniqueMessageId;
                AnonymousPipe::Status writeStatus =
                    _pipeToAgent->writeMessage(request);
                request->messageId = originalMessageId;

                if (doProviderModuleOptimization)
                {
                    request->operationContext.set(*origProviderId.get());
                }

                if (writeStatus != AnonymousPipe::STATUS_SUCCESS)
                {
                    PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL1,
                        "Failed to write message to pipe.  writeStatus = %d.",
                        writeStatus));

                    request->messageId = originalMessageId;

                    if (doProviderModuleOptimization)
                    {
                        request->operationContext.set(*origProviderId.get());
                    }

                    // Remove this OutstandingRequestTable entry
                    {
                        AutoMutex tableLock(_outstandingRequestTableMutex);
                        PEGASUS_FCT_EXECUTE_AND_ASSERT(
                            true,
                            _outstandingRequestTable.remove(uniqueMessageId));
                    }

                    // A response value of _REQUEST_NOT_PROCESSED indicates
                    // that the request was not processed by the provider
                    // agent, so it can be retried safely.
                    PEG_METHOD_EXIT();
                    return _REQUEST_NOT_PROCESSED;
                }

                if (updateProviderModuleCache)
                {
                    _providerModuleCache = origProviderId->getModule();
                }

                response = request->buildResponse();
                response->isAsyncResponsePending = true;
                PEG_METHOD_EXIT();

                return response;
            }
            catch (...)
            {
                request->messageId = originalMessageId;

                if (doProviderModuleOptimization)
                {
                    request->operationContext.set(*origProviderId.get());
                }

                PEG_TRACE_CSTRING(TRC_PROVIDERMANAGER, Tracer::LEVEL1,
                    "Failed to write message to pipe.");
                // Remove the OutstandingRequestTable entry for this request
                {
                    AutoMutex tableLock(_outstandingRequestTableMutex);
                    PEGASUS_FCT_EXECUTE_AND_ASSERT(
                        true,
                        _outstandingRequestTable.remove(uniqueMessageId));
                }
                PEG_METHOD_EXIT();
                throw;
            }
        }
    }
    catch (CIMException& e)
    {
        PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL1,
            "Caught CIMException: %s",
            (const char*)e.getMessage().getCString()));
        response = request->buildResponse();
        response->cimException = e;
    }
    catch (Exception& e)
    {
        PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL1,
            "Caught Exception: %s",
            (const char*)e.getMessage().getCString()));
        response = request->buildResponse();
        response->cimException = PEGASUS_CIM_EXCEPTION(
            CIM_ERR_FAILED, e.getMessage());
    }
    catch (...)
    {
        PEG_TRACE_CSTRING(TRC_PROVIDERMANAGER, Tracer::LEVEL2,
            "Caught unknown exception");
        response = request->buildResponse();
        response->cimException = PEGASUS_CIM_EXCEPTION(
            CIM_ERR_FAILED, String());
    }

    response->messageId = originalMessageId;
    response->syncAttributes(request);

    PEG_METHOD_EXIT();
    return response;
}

void ProviderAgentContainer::unloadIdleProviders()
{
    PEG_METHOD_ENTER(TRC_PROVIDERMANAGER,
        "ProviderAgentContainer::unloadIdleProviders");

    AutoMutex lock(_agentMutex);
    if (_isInitialized)
    {
        // Send a "wake up" message to the Provider Agent.
        // Don't bother checking whether the operation is successful.
        Uint32 messageLength = 0;
        _pipeToAgent->writeBuffer((const char*)&messageLength, sizeof(Uint32));
    }

    PEG_METHOD_EXIT();
}

void ProviderAgentContainer::cleanDisconnectedClientRequests()
{
    PEG_METHOD_ENTER(TRC_PROVIDERMANAGER,
        "ProviderAgentContainer::cleanDisconnectedClientRequests");

    // Array to store the keys which need to be removed.
    Array<String> keys;

    AutoMutex tableLock(_outstandingRequestTableMutex);
    for (OutstandingRequestTable::Iterator i = _outstandingRequestTable.start();
        i != 0; i++)
    {
        // Do not execute the isClientActiveTest for internalOperations
        // these are executed on behalf of pull requests and do not
        // have a direct connection to the client. These are cleaned up
        // through the cleanClosedPullRequests function
        if (i.value()->requestMessage->internalOperation)
        {
            continue;
        }
        if (!_isClientActive(i.value()->requestMessage))
        {
            PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL1,
                "Client Active. Send setCompleteMessage %s",
                (const char*)i.value()->originalMessageId.getCString()));
            // create empty response and set isComplete to true.
            AutoPtr<CIMResponseMessage> response;
            SharedPtr<OutstandingRequestEntry> entry = i.value();
            response.reset(i.value()->requestMessage->buildResponse());
            response->setComplete(true);
            response->messageId = i.value()->originalMessageId;
            _asyncResponseCallback(
                i.value()->requestMessage,
                response.release());
            keys.append(i.key());
        }
    }

    for(Uint32 j=0; j<keys.size();j++)
    {
         _outstandingRequestTable.remove(keys[j]);
    }
    PEG_METHOD_EXIT();
}

// Clean up RequestTable entries of pull requests for which the client is no
// longer active.
//
void ProviderAgentContainer::cleanClosedPullRequests(const String& contextId)
{
    PEG_METHOD_ENTER(TRC_PROVIDERMANAGER,
        "ProviderAgentContainer::cleanClosedPullRequests");

    // Array to store the keys which need to be remvoed.
    Array<String> keys;

    // Any entry in this array is a request that is still outstanding and
    // has not completed. Sending a response with the complete flag set,
    // removes the entry from this table. This also assumes that the
    // dispatcher status on the request is NOT closed out (operationAggregate
    // and for pull operations enumerationContext).
    AutoMutex tableLock(_outstandingRequestTableMutex);
    for (OutstandingRequestTable::Iterator i = _outstandingRequestTable.start();
        i != 0; i++)
    {
        // If this is a pull operation, the messageId is actually the
        // enumeration context since that is the id inserted into the
        // messages created in the CIMOperationRequestDispatcher.cpp
        // If this message found, send the empty response
        if (i.value()->requestMessage->internalOperation)
        {
            // the enumerationContext is the original messageId for internal
            // operation requests.
            if (i.value()->originalMessageId == contextId)
            {
                PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL1,
                    "EnumerationContext cleanup."
                    " Send provider completeMessage messageId=%s",
                (const char*)i.value()->originalMessageId.getCString()));

                AutoPtr<CIMResponseMessage> response;
                SharedPtr<OutstandingRequestEntry> entry = i.value();
                response.reset(i.value()->requestMessage->buildResponse());
                // KS_TODO THIS NOT INTERNATIONALIZED
                CIMException cimException(CIM_ERR_FAILED,
                    "Provider response Timeout in OOPProviderManagerRouter");
                response->cimException = cimException;
                response->setComplete(true);
                response->messageId = i.value()->originalMessageId;
                _asyncResponseCallback(
                    i.value()->requestMessage,
                    response.release());
                keys.append(i.key());
            }
        }
    }

    for(Uint32 j=0; j<keys.size();j++)
    {
         _outstandingRequestTable.remove(keys[j]);
    }
    PEG_METHOD_EXIT();
}

void ProviderAgentContainer::_processGetSCMOClassRequest(
    ProvAgtGetScmoClassRequestMessage* request)
{
    PEG_METHOD_ENTER(TRC_PROVIDERMANAGER,
        "ProviderAgentContainer::_processGetSCMOClassRequest");

    AutoPtr<ProvAgtGetScmoClassResponseMessage> response(
        new ProvAgtGetScmoClassResponseMessage(
            request->messageId,
            CIMException(),
            QueueIdStack(),
            SCMOClass("","")));

    CString ns = request->nameSpace.getString().getCString();
    CString cn = request->className.getString().getCString();

    delete request;

    response->scmoClass = SCMOClassCache::getInstance()->getSCMOClass(
                              ns,strlen(ns),
                              cn,strlen(cn));

    //
    // Lock the Provider Agent Container and
    // writing the response to the connection
    //
    {
        AutoMutex lock(_agentMutex);

        //
        // Write the message to the pipe
        //
        try
        {

            AnonymousPipe::Status writeStatus =
                _pipeToAgent->writeMessage(response.get());

            if (writeStatus != AnonymousPipe::STATUS_SUCCESS)
            {
                PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL1,
                    "Failed to write message to pipe.  writeStatus = %d.",
                    writeStatus));

                PEG_METHOD_EXIT();
                return;
            }

        }
        catch (Exception & e)
        {
            PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL1,
                "Exception: Failed to write message to pipe. Error: %s",
                       (const char*)e.getMessage().getCString()));
            PEG_METHOD_EXIT();
            throw;
        }
        catch (...)
        {

            PEG_TRACE_CSTRING(TRC_PROVIDERMANAGER, Tracer::LEVEL1,
                "Unkonwn exception. Failed to write message to pipe.");
            PEG_METHOD_EXIT();
            throw;
        }
    }

    PEG_METHOD_EXIT();
    return;
}

Boolean ProviderAgentContainer::_isClientActive(CIMRequestMessage *request_)
{
    MessageQueue *connectionMQ = MessageQueue::lookup(request_->queueIds[0]);
    return connectionMQ->isActive();
}

// Note: This method should not throw an exception.  It is used as a thread
// entry point, and any exceptions thrown are ignored.
ThreadReturnType PEGASUS_THREAD_CDECL
ProviderAgentContainer::_retryRequestHandler(void* arg)
{
    PEG_METHOD_ENTER(TRC_PROVIDERMANAGER,
        "ProviderAgentContainer::_retryRequestHandler");

    PEGASUS_ASSERT(arg != 0);
    RetryThreadParam *threadParams=
        reinterpret_cast<RetryThreadParam *>(arg);
    Array<CIMRequestMessage *> retryRequests = threadParams->retryRequestArray;

    try
    {
        for(Uint32 i=0; i<retryRequests.size(); i++)
        {
            threadParams->pac->processMessage(retryRequests[i]);
        }
    }
    catch(Exception &e)
    {
        PEG_TRACE((TRC_DISCARDED_DATA, Tracer::LEVEL1,
            "Unexpected exception in _retryRequestHandler: %s",
            (const char*)e.getMessage().getCString()));
    }
    catch (...)
    {
        PEG_TRACE_CSTRING(TRC_DISCARDED_DATA, Tracer::LEVEL1,
            "Unexpected exception in _retryRequestHandler.");
    }
    PEG_METHOD_EXIT();

    return ThreadReturnType(0);
}


void ProviderAgentContainer::_processResponses()
{
    PEG_METHOD_ENTER(TRC_PROVIDERMANAGER,
        "ProviderAgentContainer::_processResponses");

    //
    // Process responses until the pipe is closed
    //
    while (1)
    {
        try
        {
            CIMMessage* message;

            //
            // Read a response from the Provider Agent
            //
            AnonymousPipe::Status readStatus =
                _pipeFromAgent->readMessage(message);

            // Ignore interrupts
            if (readStatus == AnonymousPipe::STATUS_INTERRUPT)
            {
                continue;
            }

            // Handle an error the same way as a closed connection
            if ((readStatus == AnonymousPipe::STATUS_ERROR) ||
                (readStatus == AnonymousPipe::STATUS_CLOSED))
            {
                _uninitialize(false);
                return;
            }

            // A null message indicates that the provider agent process has
            // finished its processing and is ready to exit.
            if (message == 0)
            {
                _uninitialize(true);
                return;
            }

            if (message->getType() == CIM_PROCESS_INDICATION_REQUEST_MESSAGE)
            {
                // Process an indication message
                CIMProcessIndicationRequestMessage* request =
                    reinterpret_cast<CIMProcessIndicationRequestMessage*>(
                        message);
                request->oopAgentName = getGroupNameWithType();
                _indicationCallback(request);
            }
            else if (message->getType()==PROVAGT_GET_SCMOCLASS_REQUEST_MESSAGE)
            {

                _processGetSCMOClassRequest(
                    reinterpret_cast<ProvAgtGetScmoClassRequestMessage*>(
                        message));
            }
            else if (!message->isComplete())
            {
                // Process an incomplete response chunk

                CIMResponseMessage* response;
                response = dynamic_cast<CIMResponseMessage*>(message);
                PEGASUS_ASSERT(response != 0);

                Boolean foundEntry = false;
                // Get the OutstandingRequestEntry for this response chunk
                SharedPtr<OutstandingRequestEntry> _outstandingRequestEntry;
                {
                    AutoMutex tableLock(_outstandingRequestTableMutex);
                    foundEntry = _outstandingRequestTable.lookup(
                        response->messageId, _outstandingRequestEntry);
                }

                if(foundEntry)
                {
                    // Put the original message ID into the response
                    response->messageId =
                        _outstandingRequestEntry->originalMessageId;

                    // Call the response chunk callback to process the chunk
                    // if the client connection is active.
                    // No need to acquire _agentMutex since this a chunk
                    // response callback. The request object will not be
                    // deleted here.
                    _responseChunkCallback(
                        _outstandingRequestEntry->requestMessage, response);
                }
                else
                {
                    PEG_TRACE((TRC_DISCARDED_DATA,Tracer::LEVEL4,
                               "The response for message id %s arrived after " \
                               "the client disconnected.",
                               (const char *)response->messageId.getCString()));
                    delete response;
                }
            }
            else
            {
                // Process a completed response
                CIMResponseMessage* response;
                response = dynamic_cast<CIMResponseMessage*>(message);
                PEGASUS_ASSERT(response != 0);

                Boolean foundEntry = false;
                // Give the response to the waiting OutstandingRequestEntry
                SharedPtr<OutstandingRequestEntry> _outstandingRequestEntry;
                {
                    AutoMutex tableLock(_outstandingRequestTableMutex);
                    foundEntry = _outstandingRequestTable.lookup(
                        response->messageId, _outstandingRequestEntry);

                    if(foundEntry)
                    {
                        // Remove the completed request from the table
                        PEGASUS_FCT_EXECUTE_AND_ASSERT(
                            true,
                            _outstandingRequestTable.remove(
                                response->messageId));
                    }
                }

                if(foundEntry)
                {
                    if(_outstandingRequestEntry->respAggregator == NULL)
                    {
                       response->messageId =
                           _outstandingRequestEntry->originalMessageId;

                       _sendResponse(_outstandingRequestEntry->requestMessage,
                           response);
                    }
                    else
                    {
                        if(_outstandingRequestEntry->respAggregator-> \
                            isComplete(response->cimException))
                        {
                            response->messageId =
                                _outstandingRequestEntry->originalMessageId;

                            _sendResponse(
                                _outstandingRequestEntry->requestMessage,
                                response);

                            // delete respAggregator pointer now
                            delete _outstandingRequestEntry->respAggregator;
                        }
                        else
                        {
                            // this is not the last response for this request.
                            // Its job is done and it can be deleted now.
                            delete response;
                        }
                    }
                }
                else
                {
                    PEG_TRACE((TRC_DISCARDED_DATA,Tracer::LEVEL4,
                        "The response for message id %s arrived after the " \
                            "client disconnected.",
                        (const char *)response->messageId.getCString()));
                    delete response;
                }
            }
        }
        catch (Exception& e)
        {
            PEG_TRACE((TRC_DISCARDED_DATA, Tracer::LEVEL2,
                "Ignoring exception: %s",
                (const char*)e.getMessage().getCString()));
        }
        catch (...)
        {
            PEG_TRACE_CSTRING(TRC_DISCARDED_DATA, Tracer::LEVEL2,
                "Ignoring exception");
        }
    }
}

void ProviderAgentContainer::_sendResponse(CIMRequestMessage *request,
    CIMResponseMessage *response)
{
    response->syncAttributes(request);
    {
       // acquire the _agentMutex to make sure that
       // _processMessage thread has finished
       // processing the request.
       AutoMutex agentLock(_agentMutex);
    }

    // Call the asyncResponseCallback to process
    // the completed response.
    _asyncResponseCallback(
        request,
        response);
}

ThreadReturnType PEGASUS_THREAD_CDECL
ProviderAgentContainer::_responseProcessor(void* arg)
{
    ProviderAgentContainer* pa =
        reinterpret_cast<ProviderAgentContainer*>(arg);

    pa->_processResponses();

    return ThreadReturnType(0);
}

/////////////////////////////////////////////////////////////////////////////
// OOPProviderManagerRouter
/////////////////////////////////////////////////////////////////////////////

OOPProviderManagerRouter::OOPProviderManagerRouter(
    PEGASUS_INDICATION_CALLBACK_T indicationCallback,
    PEGASUS_RESPONSE_CHUNK_CALLBACK_T responseChunkCallback,
    PEGASUS_PROVIDERMODULEGROUPFAIL_CALLBACK_T providerModuleGroupFailCallback,
    PEGASUS_ASYNC_RESPONSE_CALLBACK_T asyncResponseCallback)
{
    PEG_METHOD_ENTER(TRC_PROVIDERMANAGER,
        "OOPProviderManagerRouter::OOPProviderManagerRouter");

    _indicationCallback = indicationCallback;
    _responseChunkCallback = responseChunkCallback;
    _providerModuleGroupFailCallback = providerModuleGroupFailCallback;
    _asyncResponseCallback = asyncResponseCallback;
    _threadPool =
        new ThreadPool(0, "OOPProviderManagerRouter", 0, 0, deallocateWait);;
    PEG_METHOD_EXIT();
}

OOPProviderManagerRouter::~OOPProviderManagerRouter()
{
    PEG_METHOD_ENTER(TRC_PROVIDERMANAGER,
        "OOPProviderManagerRouter::~OOPProviderManagerRouter");

    try
    {
        // Clean up the ProviderAgentContainers
        AutoMutex lock(_providerAgentTableMutex);
        ProviderAgentTable::Iterator i = _providerAgentTable.start();
        for (; i != 0; i++)
        {
            delete i.value();
        }

        delete _threadPool;
    }
    catch (...) {}

    PEG_METHOD_EXIT();
}

void OOPProviderManagerRouter::_handleIndicationDeliveryResponse(
    CIMResponseMessage *response)
{
    if (response->getType() == CIM_PROCESS_INDICATION_RESPONSE_MESSAGE)
    {
         CIMProcessIndicationResponseMessage *rsp =
             (CIMProcessIndicationResponseMessage*)response;

        // Look up the Provider Agents for this module
        Array<ProviderAgentContainer*> paArray =
            _lookupProviderAgents(rsp->oopAgentName);

        for (Uint32 i = 0; i < paArray.size(); i++)
        {
            if (paArray[i]->isInitialized())
            {
                paArray[i]->sendResponse(response);
            }
        }
        return;
    }

    PEGASUS_UNREACHABLE(PEGASUS_ASSERT(false);)
}

Message* OOPProviderManagerRouter::processMessage(Message* message)
{
    PEG_METHOD_ENTER(TRC_PROVIDERMANAGER,
        "OOPProviderManagerRouter::processMessage");

    if (message->getType() == CIM_PROCESS_INDICATION_RESPONSE_MESSAGE)
    {
        _handleIndicationDeliveryResponse((CIMResponseMessage*)message);
        return 0;
    }

    CIMRequestMessage* request = dynamic_cast<CIMRequestMessage *>(message);
    PEGASUS_ASSERT(request != 0);

    AutoPtr<CIMResponseMessage> response;

    //
    // Get the provider information from the request
    //
    CIMInstance providerModule;

    if ((dynamic_cast<CIMOperationRequestMessage*>(request) != 0) ||
        (dynamic_cast<CIMIndicationRequestMessage*>(request) != 0) ||
        (request->getType() == CIM_EXPORT_INDICATION_REQUEST_MESSAGE))
    {
        // Provider information is in the OperationContext
        ProviderIdContainer pidc = (ProviderIdContainer)
            request->operationContext.get(ProviderIdContainer::NAME);
        providerModule = pidc.getModule();
    }
    else if (request->getType() == CIM_ENABLE_MODULE_REQUEST_MESSAGE)
    {
        CIMEnableModuleRequestMessage* emReq =
            dynamic_cast<CIMEnableModuleRequestMessage*>(request);
        providerModule = emReq->providerModule;
    }
    else if (request->getType() == CIM_DISABLE_MODULE_REQUEST_MESSAGE)
    {
        CIMDisableModuleRequestMessage* dmReq =
            dynamic_cast<CIMDisableModuleRequestMessage*>(request);
        providerModule = dmReq->providerModule;
    }
    else if ((request->getType() == CIM_STOP_ALL_PROVIDERS_REQUEST_MESSAGE) ||
             (request->getType() ==
                 CIM_SUBSCRIPTION_INIT_COMPLETE_REQUEST_MESSAGE) ||
             (request->getType() ==
                 CIM_INDICATION_SERVICE_DISABLED_REQUEST_MESSAGE) ||
             (request->getType() == CIM_NOTIFY_CONFIG_CHANGE_REQUEST_MESSAGE))
    {
        // This operation is not provider-specific
    }
    else
    {
        // Unrecognized message type.  This should never happen.
        PEGASUS_ASSERT(0);
        response.reset(request->buildResponse());
        response->cimException = PEGASUS_CIM_EXCEPTION(
            CIM_ERR_FAILED, "Unrecognized message type.");
        PEG_METHOD_EXIT();
        return response.release();
    }

    //
    // Process the request message
    //
    if (request->getType() == CIM_STOP_ALL_PROVIDERS_REQUEST_MESSAGE)
    {
        ProviderAgentContainer::setAllProvidersStopped();
        // Forward the CIMStopAllProvidersRequest to all providers
        response.reset(_forwardRequestToAllAgents(request));

        // Note: Do not uninitialize the ProviderAgentContainers here.
        // Just let the selecting thread notice when the agent connections
        // are closed.
    }
    else if (request->getType () ==
        CIM_SUBSCRIPTION_INIT_COMPLETE_REQUEST_MESSAGE)
    {
        ProviderAgentContainer::setSubscriptionInitComplete(true);

        //
        //  Forward the CIMSubscriptionInitCompleteRequestMessage to
        //  all providers
        //
        response.reset (_forwardRequestToAllAgents (request));
    }
    else if (request->getType () ==
        CIM_INDICATION_SERVICE_DISABLED_REQUEST_MESSAGE)
    {
        ProviderAgentContainer::setSubscriptionInitComplete(false);

        //
        //  Forward the CIMIndicationServiceDisabledRequestMessage to
        //  all providers
        //
        response.reset (_forwardRequestToAllAgents (request));
    }
    else if (request->getType() == CIM_NOTIFY_CONFIG_CHANGE_REQUEST_MESSAGE)
    {
        CIMNotifyConfigChangeRequestMessage* notifyRequest =
            dynamic_cast<CIMNotifyConfigChangeRequestMessage*>(request);
        PEGASUS_ASSERT(notifyRequest != 0);

        if (notifyRequest->currentValueModified)
        {
            // Forward the CIMNotifyConfigChangeRequestMessage to all providers
            response.reset(_forwardRequestToAllAgents(request));
        }
        else
        {
            // No need to notify provider agents about changes to planned value
            response.reset(request->buildResponse());
        }
    }
    else if (request->getType() == CIM_DISABLE_MODULE_REQUEST_MESSAGE)
    {
        // Fan out the request to all Provider Agent processes for this module

        // Retrieve the provider group name.
        String groupNameWithType;
        _getGroupNameWithType(providerModule, groupNameWithType);


        // Look up the Provider Agents for this module
        Array<ProviderAgentContainer*> paArray =
            _lookupProviderAgents(groupNameWithType);

        Array<ProviderAgentContainer*> paInit;

        for (Uint32 i=0; i<paArray.size(); i++)
        {
            //
            // Do not start up an agent process just to disable the module
            //
            if (paArray[i]->isInitialized())
            {
                paInit.append(paArray[i]);
            }
        }

        if(paInit.size() > 0)
        {
            RespAggCounter *respAggregator =
                new RespAggCounter(paInit.size());

            for (Uint32 i=0; i<paInit.size(); i++)
            {
                // Forward the request to the provider agent
                //
                response.reset(
                    paInit[i]->processMessage(request,respAggregator));

                // Note: Do not uninitialize the ProviderAgentContainer here
                // when a disable module operation is successful.  Just let the
                // selecting thread notice when the agent connection is closed.
            }
        }

        // Use a default response if no Provider Agents were called
        if (!response.get())
        {
            response.reset(request->buildResponse());

            CIMDisableModuleResponseMessage* dmResponse =
                dynamic_cast<CIMDisableModuleResponseMessage*>(response.get());
            PEGASUS_ASSERT(dmResponse != 0);

            Array<Uint16> operationalStatus;
            operationalStatus.append(CIM_MSE_OPSTATUS_VALUE_STOPPED);
            dmResponse->operationalStatus = operationalStatus;
        }
    }
    else if (request->getType() == CIM_ENABLE_MODULE_REQUEST_MESSAGE)
    {
        // Fan out the request to all Provider Agent processes for this module

        // Retrieve the provider module group name.
        String groupNameWithType;
        _getGroupNameWithType(providerModule, groupNameWithType);

        // Look up the Provider Agents for this module
        Array<ProviderAgentContainer*> paArray =
            _lookupProviderAgents(groupNameWithType);

        // Create an array of initialized provider agents.
        Array<ProviderAgentContainer*> paInit;

        // create an array of initialized provider agents.
        for (Uint32 i=0; i<paArray.size(); i++)
        {
            if (paArray[i]->isInitialized())
            {
                paInit.append(paArray[i]);
            }
        }

        if(paInit.size() > 0 )
        {
            RespAggCounter *respAggregator =
                new RespAggCounter(paInit.size());

            for (Uint32 i=0; i<paInit.size(); i++)
            {
                //
                // Forward the request to the provider agent
                //
                response.reset(
                    paInit[i]->processMessage(request,respAggregator));
            }
        }

        // Use a default response if no Provider Agents were called
        if (!response.get())
        {
            response.reset(request->buildResponse());

            CIMEnableModuleResponseMessage* emResponse =
                dynamic_cast<CIMEnableModuleResponseMessage*>(response.get());
            PEGASUS_ASSERT(emResponse != 0);

            Array<Uint16> operationalStatus;
            operationalStatus.append(CIM_MSE_OPSTATUS_VALUE_OK);
            emResponse->operationalStatus = operationalStatus;
        }
    }
    else
    {
        //
        // Look up the Provider Agent for this module instance and requesting
        // user
        //
        ProviderAgentContainer* pa = _lookupProviderAgent(providerModule,
            request);
        PEGASUS_ASSERT(pa != 0);

        //
        // Forward the request to the provider agent
        //
        response.reset(pa->processMessage(request));
    }

    PEG_METHOD_EXIT();
    return response.release();
}

ProviderAgentContainer* OOPProviderManagerRouter::_lookupProviderAgent(
    const CIMInstance& providerModule,
    CIMRequestMessage* request)
{
    // Retrieve the provider module group name
    String groupNameWithType;
    _getGroupNameWithType(providerModule, groupNameWithType);

    Uint16 bitness = PG_PROVMODULE_BITNESS_DEFAULT;
    Uint32 bIndex = providerModule.findProperty(
        PEGASUS_PROPERTYNAME_MODULE_BITNESS);

    if (bIndex != PEG_NOT_FOUND)
    {
        CIMValue value =
            providerModule.getProperty(bIndex).getValue();
        if (!value.isNull())
        {
            value.get(bitness);
        }
    }

#if defined(PEGASUS_OS_ZOS)
    // For z/OS we don't start an extra provider agent for
    // each user, since the userid is switched at the thread
    // level. Therefore we set the userName to an empty String,
    // as it is used below to build the key for the provider
    // agent table
    String userName;
    Uint16 userContext = PEGASUS_DEFAULT_PROV_USERCTXT;
#else

    // Retrieve the provider user context configuration
    Uint16 userContext = 0;
    Uint32 pos = providerModule.findProperty(
        PEGASUS_PROPERTYNAME_MODULE_USERCONTEXT);
    if (pos != PEG_NOT_FOUND)
    {
        CIMValue userContextValue =
            providerModule.getProperty(pos).getValue();
        if (!userContextValue.isNull())
        {
            userContextValue.get(userContext);
        }
    }

    if (userContext == 0)
    {
        // PASE has a default user context "QYCMCIMOM",
        // so we leave userContext unset here.
#ifndef PEGASUS_OS_PASE
        userContext = PEGASUS_DEFAULT_PROV_USERCTXT;
#endif
    }

    String userName;

    if (userContext == PG_PROVMODULE_USERCTXT_REQUESTOR)
    {
        if (request->operationContext.contains(IdentityContainer::NAME))
        {
            // User Name is in the OperationContext
            IdentityContainer ic = (IdentityContainer)
                request->operationContext.get(IdentityContainer::NAME);
            userName = ic.getUserName();
        }
        //else
        //{
        //    If no IdentityContainer is present, default to the CIM
        //    Server's user context
        //}

        // If authentication is disabled, use the CIM Server's user context
        if (!userName.size())
        {
            userName = System::getEffectiveUserName();
        }
    }
    else if (userContext == PG_PROVMODULE_USERCTXT_DESIGNATED)
    {
        // Retrieve the provider module designated user property value
        providerModule.getProperty(providerModule.findProperty(
            PEGASUS_PROPERTYNAME_MODULE_DESIGNATEDUSER)).getValue().
            get(userName);
    }
    else if (userContext == PG_PROVMODULE_USERCTXT_CIMSERVER)
    {
        userName = System::getEffectiveUserName();
    }
#ifdef PEGASUS_OS_PASE // it might be unset user in PASE in this branch.
    else if (userContext == 0)
    {
        userName = "QYCMCIMOM";
    }
#endif
    else    // Privileged User
    {
        PEGASUS_ASSERT(userContext == PG_PROVMODULE_USERCTXT_PRIVILEGED);
        userName = System::getPrivilegedUserName();
    }

    PEG_TRACE((
        TRC_PROVIDERMANAGER,
        Tracer::LEVEL4,
        "Group name with type = %s, User context = %hd, User name = %s",
        (const char*) groupNameWithType.getCString(),
        userContext,
        (const char*) userName.getCString()));
#endif

    ProviderAgentContainer* pa = 0;

#ifdef PEGASUS_OS_PASE
    String userUpper = userName;
    userUpper.toUpper();
    String key = groupNameWithType + ":" + userUpper;
#else
    String key = groupNameWithType + ":" + userName;
#endif

    AutoMutex lock(_providerAgentTableMutex);
    if (!_providerAgentTable.lookup(key, pa))
    {
        pa = new ProviderAgentContainer(
            bitness,
            groupNameWithType, userName, userContext,
            _indicationCallback, _responseChunkCallback,
            _providerModuleGroupFailCallback,
            _asyncResponseCallback,
            _threadPool);
        _providerAgentTable.insert(key, pa);
    }

    return pa;
}

Array<ProviderAgentContainer*> OOPProviderManagerRouter::_lookupProviderAgents(
    const String& groupNameWithType)
{
    Array<ProviderAgentContainer*> paArray;

    AutoMutex lock(_providerAgentTableMutex);
    for (ProviderAgentTable::Iterator i = _providerAgentTable.start(); i; i++)
    {
        if (i.value()->getGroupNameWithType() == groupNameWithType)
        {
            paArray.append(i.value());
        }
    }
    return paArray;
}

Array<ProviderAgentContainer*>
    OOPProviderManagerRouter::_getProviderAgentContainerCopy()
{
    Array<ProviderAgentContainer*> paContainerArray;

    AutoMutex tableLock(_providerAgentTableMutex);

    for (ProviderAgentTable::Iterator i = _providerAgentTable.start();
         i != 0; i++)
    {
        if(i.value()->isInitialized())
        {
            paContainerArray.append(i.value());
        }
    }
    return paContainerArray;
}

CIMResponseMessage* OOPProviderManagerRouter::_forwardRequestToAllAgents(
    CIMRequestMessage* request)
{
    PEG_METHOD_ENTER(TRC_PROVIDERMANAGER,
        "OOPProviderManagerRouter::_forwardRequestToAllAgents");

    // Get a list of the ProviderAgentContainers.  We need our own array copy
    // because we cannot hold the _providerAgentTableMutex while calling
    // _ProviderAgentContainer::processMessage().
    Array<ProviderAgentContainer*> paContainerArray=
        _getProviderAgentContainerCopy();

    Boolean responsePending = false;
    CIMResponseMessage *response = request->buildResponse();

    if(paContainerArray.size() > 0 )
    {
        RespAggCounter *respAggregator =
            new RespAggCounter(paContainerArray.size());

        // Forward the request to each of the initialized provider agents
        for (Uint32 j = 0; j < paContainerArray.size(); j++)
        {
            ProviderAgentContainer* pa = paContainerArray[j];

            // Note: The ProviderAgentContainer could become uninitialized
            // before _ProviderAgentContainer::processMessage() processes
            // this request.  In this case, the Provider Agent process will
            // (unfortunately) be started to process this message.
            AutoPtr<CIMResponseMessage> response;
            response.reset(pa->processMessage(request,respAggregator));
            responsePending = true;
        }

        response->isAsyncResponsePending = responsePending;
    }

    PEG_METHOD_EXIT();
    return response;
}

void OOPProviderManagerRouter::idleTimeCleanup()
{
    PEG_METHOD_ENTER(TRC_PROVIDERMANAGER,
        "OOPProviderManagerRouter::idleTimeCleanup");

    // Get a list of the ProviderAgentContainers.  We need our own array copy
    // because we cannot hold the _providerAgentTableMutex while calling
    // ProviderAgentContainer::unloadIdleProviders() &
    // ProviderAgentContainer::cleanDisconnectedClientRequests().
    Array<ProviderAgentContainer*> paContainerArray=
        _getProviderAgentContainerCopy();

    // Iterate through the _providerAgentTable unloading idle providers
    for (Uint32 j = 0; j < paContainerArray.size(); j++)
    {
        paContainerArray[j]->unloadIdleProviders();
    }

    // Iterate through the _providerAgentTable cleaning up disconnected clients.
    for (Uint32 k = 0; k < paContainerArray.size(); k++)
    {
        paContainerArray[k]->cleanDisconnectedClientRequests();
    }

    PEG_METHOD_EXIT();
}

void OOPProviderManagerRouter::enumerationContextCleanup(
    const String& contextId)
{
    PEG_METHOD_ENTER(TRC_PROVIDERMANAGER,
        "OOPProviderManagerRouter::enumerationContextCleanup");

    // KS_TODO since this is short-running, not sure we even need to make
    // the copy.
    // Get a list of the ProviderAgentContainers.  We need our own array copy
    // to avoid holding the _providerAgentTableMutex while calling
    //  the cleanup requests.
    Array<ProviderAgentContainer*> paContainerArray=
        _getProviderAgentContainerCopy();

    // Iterate through the _providerAgentTable
    // to find any entries with the defined contextId and to clean those
    // requests up.
    for (Uint32 k = 0; k < paContainerArray.size(); k++)
    {
        paContainerArray[k]->cleanClosedPullRequests(contextId);
    }

    PEG_METHOD_EXIT();
}

void OOPProviderManagerRouter::_getGroupNameWithType(
        const CIMInstance &providerModule,
        String &groupNameWithType)
{
    Uint32 idx = providerModule.findProperty(
            PEGASUS_PROPERTYNAME_MODULE_MODULEGROUPNAME);

    String moduleName;
    String groupName;

    if (idx != PEG_NOT_FOUND)
    {
        providerModule.getProperty(idx).getValue().get(groupName);
    }

    // Note: If group name is not found, module name is used as group name.
    // prefixes "grp" and "mod" is used to distinguish between the provider
    // modules who have same group and module names.
    if (groupName.size())
    {
        groupNameWithType.assign(_GROUP_PREFIX);
        groupNameWithType.append(groupName);
    }
    else
    {
        _getProviderModuleName(providerModule, moduleName);
        groupNameWithType.assign(_MODULE_PREFIX);
        groupNameWithType.append(moduleName);
    }
}

void OOPProviderManagerRouter::_getProviderModuleName(
        const CIMInstance & providerModule,
        String & moduleName)
{
    CIMValue nameValue = providerModule.getProperty(
        providerModule.findProperty(PEGASUS_PROPERTYNAME_NAME)).getValue();
    nameValue.get(moduleName);

#if defined(PEGASUS_OS_ZOS)
    // Retrieve the providers shareAS flag, to see if it will share the
    // provider address space with other providers or requests its own
    // address space.
    Boolean shareAS = true;
    Uint32 saIndex = providerModule.findProperty("ShareAS");
    if (saIndex != PEG_NOT_FOUND)
    {
        CIMValue shareValue=providerModule.getProperty(saIndex).getValue();
        shareValue.get(shareAS);
    }
    if (shareAS == true)
    {
        moduleName.assign("SharedProviderAgent");
    }
#endif
    return;
}


PEGASUS_NAMESPACE_END
