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

#include "DefaultProviderManager.h"

#include <Pegasus/Common/CIMMessage.h>
#include <Pegasus/Common/OperationContext.h>
#include <Pegasus/Common/Time.h>
#include <Pegasus/Common/Tracer.h>
#include <Pegasus/Common/StatisticalData.h>
#include <Pegasus/Common/Logger.h>
#include <Pegasus/Common/MessageLoader.h>
#include <Pegasus/Common/FileSystem.h>
#include <Pegasus/Common/PegasusVersion.h>
#include <Pegasus/Common/Constants.h>

#include <Pegasus/Query/QueryExpression/QueryExpression.h>
#include <Pegasus/ProviderManager2/QueryExpressionFactory.h>

#include <Pegasus/ProviderManager2/OperationResponseHandler.h>

PEGASUS_NAMESPACE_BEGIN


// A request class to hold the provider info passed to the
// AsyncRequestExecutor for processing on different threads.
class UnloadProviderRequest : public AsyncRequestExecutor::AsyncRequestMsg
{
public:
    UnloadProviderRequest(ProviderMessageHandler* provider)
        :_provider(provider) {}

public:
    ProviderMessageHandler* _provider;
};

//
// Default Provider Manager
//
DefaultProviderManager::DefaultProviderManager()
{
    _subscriptionInitComplete = false;
}

DefaultProviderManager::~DefaultProviderManager()
{
    PEG_METHOD_ENTER(TRC_PROVIDERMANAGER,
        "DefaultProviderManager::~DefaultProviderManager");

    _shutdownAllProviders();

    for (ProviderTable::Iterator i = _providers.start(); i != 0; i++)
    {
        ProviderMessageHandler* provider = i.value();
        delete provider;
    }

    for (ModuleTable::Iterator j = _modules.start(); j != 0; j++)
    {
        ProviderModule* module = j.value();
        delete module;
    }

    PEG_METHOD_EXIT();
}

Message* DefaultProviderManager::processMessage(Message* message)
{
    PEG_METHOD_ENTER(TRC_PROVIDERMANAGER,
        "DefaultProviderManager::processMessage()");

    CIMRequestMessage* request = dynamic_cast<CIMRequestMessage*>(message);
    PEGASUS_ASSERT(request != 0);

    CIMResponseMessage* response = 0;

    try
    {
        // pass the request message to a handler method based on message type
        switch(request->getType())
        {
        case CIM_GET_INSTANCE_REQUEST_MESSAGE:
        case CIM_ENUMERATE_INSTANCES_REQUEST_MESSAGE:
        case CIM_ENUMERATE_INSTANCE_NAMES_REQUEST_MESSAGE:
        case CIM_CREATE_INSTANCE_REQUEST_MESSAGE:
        case CIM_MODIFY_INSTANCE_REQUEST_MESSAGE:
        case CIM_DELETE_INSTANCE_REQUEST_MESSAGE:
        case CIM_EXEC_QUERY_REQUEST_MESSAGE:
        case CIM_ASSOCIATORS_REQUEST_MESSAGE:
        case CIM_ASSOCIATOR_NAMES_REQUEST_MESSAGE:
        case CIM_REFERENCES_REQUEST_MESSAGE:
        case CIM_REFERENCE_NAMES_REQUEST_MESSAGE:
        case CIM_GET_PROPERTY_REQUEST_MESSAGE:
        case CIM_SET_PROPERTY_REQUEST_MESSAGE:
        case CIM_INVOKE_METHOD_REQUEST_MESSAGE:
        case CIM_CREATE_SUBSCRIPTION_REQUEST_MESSAGE:
        case CIM_MODIFY_SUBSCRIPTION_REQUEST_MESSAGE:
        case CIM_DELETE_SUBSCRIPTION_REQUEST_MESSAGE:
        case CIM_EXPORT_INDICATION_REQUEST_MESSAGE:
        {
            ProviderIdContainer providerId =
                request->operationContext.get(ProviderIdContainer::NAME);

            // resolve provider name
            ProviderName name = _resolveProviderName(providerId);

            // get cached or load new provider module
            ProviderOperationCounter poc(
                _getProvider(
                    name.getPhysicalName(),
                    name.getModuleName(),
                    name.getLogicalName()));

            response = poc.GetProvider().processMessage(request);
            break;
        }

        case CIM_DISABLE_MODULE_REQUEST_MESSAGE:
            response = _handleDisableModuleRequest(request);
            break;

        case CIM_ENABLE_MODULE_REQUEST_MESSAGE:
            response = _handleEnableModuleRequest(request);
            break;

        case CIM_STOP_ALL_PROVIDERS_REQUEST_MESSAGE:
            // tell the provider manager to shutdown all the providers
            _shutdownAllProviders();
            response = request->buildResponse();
            break;

        case CIM_SUBSCRIPTION_INIT_COMPLETE_REQUEST_MESSAGE:
            response = _handleSubscriptionInitCompleteRequest(request);
            break;

        case CIM_INDICATION_SERVICE_DISABLED_REQUEST_MESSAGE:
            response = _handleIndicationServiceDisabledRequest(request);
            break;

        default:
            PEGASUS_UNREACHABLE(PEGASUS_ASSERT(0);)
            break;
        }
    }
    catch (CIMException& e)
    {
        PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL1,"CIMException: %s",
            (const char*)e.getMessage().getCString()));
        response = request->buildResponse();
        response->cimException = PEGASUS_CIM_EXCEPTION_LANG(
            e.getContentLanguages(), e.getCode(), e.getMessage());
    }
    catch (Exception& e)
    {
        PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL1,"Exception: %s",
            (const char*)e.getMessage().getCString()));
        response = request->buildResponse();
        response->cimException = PEGASUS_CIM_EXCEPTION_LANG(
            e.getContentLanguages(), CIM_ERR_FAILED, e.getMessage());
    }
    catch (...)
    {
        PEG_TRACE_CSTRING(TRC_PROVIDERMANAGER, Tracer::LEVEL1,
            "Exception: Unknown");
        response = request->buildResponse();
        response->cimException = PEGASUS_CIM_EXCEPTION(
            CIM_ERR_FAILED, "Unknown error.");
    }

    PEG_METHOD_EXIT();
    return response;
}

CIMResponseMessage* DefaultProviderManager::_handleDisableModuleRequest(
    CIMRequestMessage* message)
{
    PEG_METHOD_ENTER(TRC_PROVIDERMANAGER,
        "DefaultProviderManager::_handleDisableModuleRequest");

    CIMDisableModuleRequestMessage* request =
        dynamic_cast<CIMDisableModuleRequestMessage*>(message);
    PEGASUS_ASSERT(request != 0);

    Array<Uint16> operationalStatus;
    CIMException cimException;

    try
    {
        // get provider module name
        String moduleName;
        CIMInstance mInstance = request->providerModule;
        Uint32 pos = mInstance.findProperty(PEGASUS_PROPERTYNAME_NAME);
        PEGASUS_ASSERT(pos != PEG_NOT_FOUND);
        mInstance.getProperty(pos).getValue().get(moduleName);

        //
        // Unload providers
        //
        Array<CIMInstance> providerInstances = request->providers;

        for (Uint32 i = 0, n = providerInstances.size(); i < n; i++)
        {
            String pName;
            providerInstances[i].getProperty(
                providerInstances[i].findProperty(PEGASUS_PROPERTYNAME_NAME)).
                    getValue().get(pName);

            Sint16 ret_value = _disableProvider(moduleName, pName);

            if (ret_value == 0)
            {
                // disable failed since there are pending requests,
                // stop trying to disable other providers in this module.
                operationalStatus.append(CIM_MSE_OPSTATUS_VALUE_OK);
                break;
            }
            else if (ret_value != 1)  // Not success
            {
                // disable failed for other reason, throw exception
                throw PEGASUS_CIM_EXCEPTION_L(
                    CIM_ERR_FAILED,
                    MessageLoaderParms(
                        "ProviderManager.ProviderManagerService."
                            "DISABLE_PROVIDER_FAILED",
                        "Failed to disable the provider."));
            }
        }
    }
    catch (CIMException& e)
    {
        PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL1,"CIMException: %s",
            (const char*)e.getMessage().getCString()));
        cimException = e;
    }
    catch (Exception& e)
    {
        PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL1,"Exception: %s",
            (const char*)e.getMessage().getCString()));
        cimException = CIMException(CIM_ERR_FAILED, e.getMessage());
    }
    catch (...)
    {
        PEG_TRACE_CSTRING(TRC_PROVIDERMANAGER, Tracer::LEVEL1,
            "Exception: Unknown");
        cimException = PEGASUS_CIM_EXCEPTION_L(
            CIM_ERR_FAILED,
            MessageLoaderParms(
                "ProviderManager.ProviderManagerService.UNKNOWN_ERROR",
                "Unknown Error"));
    }

    if (cimException.getCode() == CIM_ERR_SUCCESS)
    {
        // Status is set to OK if a provider was busy
        if (operationalStatus.size() == 0)
        {
            operationalStatus.append(CIM_MSE_OPSTATUS_VALUE_STOPPED);
        }
    }
    else
    {
        // If exception occurs, module is not stopped
        operationalStatus.append(CIM_MSE_OPSTATUS_VALUE_OK);
    }

    CIMDisableModuleResponseMessage* response =
        dynamic_cast<CIMDisableModuleResponseMessage*>(
            request->buildResponse());
    PEGASUS_ASSERT(response != 0);

    response->operationalStatus = operationalStatus;

    PEG_METHOD_EXIT();

    return response;
}

CIMResponseMessage* DefaultProviderManager::_handleEnableModuleRequest(
    CIMRequestMessage* message)
{
    PEG_METHOD_ENTER(TRC_PROVIDERMANAGER,
        "DefaultProviderManager::_handleEnableModuleRequest");

    CIMEnableModuleRequestMessage* request =
        dynamic_cast<CIMEnableModuleRequestMessage*>(message);
    PEGASUS_ASSERT(request != 0);

    Array<Uint16> operationalStatus;
    operationalStatus.append(CIM_MSE_OPSTATUS_VALUE_OK);

    CIMEnableModuleResponseMessage* response =
        dynamic_cast<CIMEnableModuleResponseMessage*>(
            request->buildResponse());
    PEGASUS_ASSERT(response != 0);

    response->operationalStatus = operationalStatus;

    PEG_METHOD_EXIT();
    return response;
}


CIMResponseMessage*
DefaultProviderManager::_handleIndicationServiceDisabledRequest(
    CIMRequestMessage* message)
{
    PEG_METHOD_ENTER(TRC_PROVIDERMANAGER,
        "DefaultProviderManager::_handleIndicationServiceDisabledRequest");

    CIMIndicationServiceDisabledRequestMessage* request =
        dynamic_cast<CIMIndicationServiceDisabledRequestMessage*>(message);
    PEGASUS_ASSERT(request != 0);

    CIMIndicationServiceDisabledResponseMessage* response =
        dynamic_cast<CIMIndicationServiceDisabledResponseMessage*>(
            request->buildResponse());
    PEGASUS_ASSERT(response != 0);

    _subscriptionInitComplete = false;

    // Make a copy of the table so it is not locked during the provider calls
    Array<ProviderMessageHandler*> providerList;
    {
        AutoMutex lock(_providerTableMutex);

        for (ProviderTable::Iterator i = _providers.start(); i != 0; i++)
        {
            providerList.append(i.value());
        }
    }

    //
    // Notify all providers that indication service is disabled
    //
    for (Uint32 j = 0; j < providerList.size(); j++)
    {
        AutoMutex lock(providerList[j]->status.getStatusMutex());

        if (providerList[j]->status.isInitialized())
        {
            providerList[j]->indicationServiceDisabled();
        }
    }

    PEG_METHOD_EXIT();
    return response;
}

CIMResponseMessage*
DefaultProviderManager::_handleSubscriptionInitCompleteRequest(
    CIMRequestMessage* message)
{
    PEG_METHOD_ENTER(TRC_PROVIDERMANAGER,
        "DefaultProviderManager::_handleSubscriptionInitCompleteRequest");

    CIMSubscriptionInitCompleteRequestMessage* request =
        dynamic_cast<CIMSubscriptionInitCompleteRequestMessage*>(message);
    PEGASUS_ASSERT(request != 0);

    CIMSubscriptionInitCompleteResponseMessage* response =
        dynamic_cast<CIMSubscriptionInitCompleteResponseMessage*>(
            request->buildResponse());
    PEGASUS_ASSERT(response != 0);

    _subscriptionInitComplete = true;

    // Make a copy of the table so it is not locked during the provider calls
    Array<ProviderMessageHandler*> providerList;
    {
        AutoMutex lock(_providerTableMutex);

        for (ProviderTable::Iterator i = _providers.start(); i != 0; i++)
        {
            providerList.append(i.value());
        }
    }

    //
    // Notify all providers that subscription initialization is complete
    //
    for (Uint32 j = 0; j < providerList.size(); j++)
    {
        AutoMutex lock(providerList[j]->status.getStatusMutex());

        if (providerList[j]->status.isInitialized())
        {
            providerList[j]->subscriptionInitComplete();
        }
    }

    PEG_METHOD_EXIT();
    return response;
}

ProviderName DefaultProviderManager::_resolveProviderName(
    const ProviderIdContainer& providerId)
{
    String providerName;
    String fileName;
    String moduleName;
    CIMValue genericValue;

    genericValue = providerId.getModule().getProperty(
        providerId.getModule().findProperty(
            PEGASUS_PROPERTYNAME_NAME)).getValue();
    genericValue.get(moduleName);

    genericValue = providerId.getProvider().getProperty(
        providerId.getProvider().findProperty(
            PEGASUS_PROPERTYNAME_NAME)).getValue();
    genericValue.get(providerName);

    genericValue = providerId.getModule().getProperty(
        providerId.getModule().findProperty("Location")).getValue();
    genericValue.get(fileName);

    String resolvedFileName = _resolvePhysicalName(fileName);

    if (resolvedFileName == String::EMPTY)
    {
        // Provider library not found
        throw Exception(MessageLoaderParms(
            "ProviderManager.ProviderManagerService.PROVIDER_FILE_NOT_FOUND",
            "File \"$0\" was not found for provider module \"$1\".",
            FileSystem::buildLibraryFileName(fileName), moduleName));
    }

    return ProviderName(moduleName, providerName, resolvedFileName);
}

ProviderOperationCounter DefaultProviderManager::_getProvider(
    const String& moduleFileName,
    const String& moduleName,
    const String& providerName)
{
    PEG_METHOD_ENTER(TRC_PROVIDERMANAGER,
        "DefaultProviderManager::_getProvider");

    ProviderMessageHandler* pr = _lookupProvider(moduleName, providerName);

    if (!pr->status.isInitialized())
    {
        _initProvider(pr, moduleFileName);
    }

    AutoMutex lock(pr->status.getStatusMutex());

    if (!pr->status.isInitialized())
    {
        PEG_METHOD_EXIT();
        throw PEGASUS_CIM_EXCEPTION(
            CIM_ERR_FAILED, "provider initialization failed");
    }

    ProviderOperationCounter poc(pr);

    PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL4,"Returning Provider %s",
        (const char*)providerName.getCString()));

    PEG_METHOD_EXIT();
    return poc;
}

ProviderMessageHandler* DefaultProviderManager::_lookupProvider(
    const String& moduleName,
    const String& providerName)
{
    PEG_METHOD_ENTER(TRC_PROVIDERMANAGER,
        "DefaultProviderManager::_lookupProvider");

    // lock the providerTable mutex
    AutoMutex lock(_providerTableMutex);

    // Construct the lookup key. We need a compound key to differentiate
    // providers with the same name from different modules. The size field is
    // added to handle the unlikely case when moduleName+providerName
    // produce identical strings but define different providers.
    char buffer[12];
    sprintf(buffer, "%u:", providerName.size());
    const String key = buffer + moduleName + ":" + providerName;

    // look up provider in cache
    ProviderMessageHandler* pr = 0;
    if (_providers.lookup(key, pr))
    {
        PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL4,
            "Found Provider %s in Provider Manager Cache",
            (const char*)providerName.getCString()));
    }
    else
    {
        // create provider
        pr = new ProviderMessageHandler(
            moduleName, providerName,
            0, _indicationCallback, _responseChunkCallback,
            _subscriptionInitComplete);

        // insert provider in provider table
        _providers.insert(key, pr);

        PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL4,"Created provider %s",
            (const char*)pr->getName().getCString()));
    }

    PEG_METHOD_EXIT();
    return pr;
}

ProviderMessageHandler* DefaultProviderManager::_initProvider(
    ProviderMessageHandler* provider,
    const String& moduleFileName)
{
    PEG_METHOD_ENTER(TRC_PROVIDERMANAGER,
        "DefaultProviderManager::_initProvider");

    ProviderModule* module = 0;
    CIMProvider* base;

    // lookup provider module
    module = _lookupModule(moduleFileName);

    // lock the provider status mutex
    AutoMutex lock(provider->status.getStatusMutex());

    if (provider->status.isInitialized())
    {
        // Initialization is already complete
        return provider;
    }

    PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL4,
        "Loading/Linking Provider Module %s",
        (const char*)moduleFileName.getCString()));

    // load the provider
    try
    {
        base = module->load(provider->getName());
    }
    catch (...)
    {
        PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL1,
            "Exception caught Loading/Linking Provider Module %s",
            (const char*)moduleFileName.getCString()));
        PEG_METHOD_EXIT();
        throw;
    }

    // initialize the provider
    PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL2, "Initializing Provider %s",
        (const char*)provider->getName().getCString()));

    CIMOMHandle* cimomHandle = new CIMOMHandle();
    provider->status.setCIMOMHandle(cimomHandle);
    provider->status.setModule(module);
    provider->setProvider(base);

    Boolean initializeError = false;

    try
    {
        provider->initialize(*cimomHandle);
    }
    catch (...)
    {
        PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL3,
            "Initialization Error.  Provider %s",
            (const char*)provider->getName().getCString()));
        initializeError = true;
    }

    // The cleanup code executed when an exception occurs was previously
    // included in the catch block above. Unloading the provider module
    // from inside the catch block resulted in a crash when an exception
    // was thrown from a provider's initialize() method. The issue is that
    // when an exception is thrown, the program maintains internal
    // pointers related to the code that threw the exception. In the case
    // of an exception thrown from a provider during the initialize()
    // method, those pointers point into the provider library, so when
    // the DefaultProviderManager unloads the library, the pointers into
    // the library that the program was holding are invalid.

    if (initializeError == true)
    {
        // Allow the provider to clean up
        provider->terminate();

        // delete the cimom handle
        delete cimomHandle;

        provider->setProvider(0);

        // unload provider module
        module->unloadModule();
    }

    provider->status.setInitialized(!initializeError);

    PEG_METHOD_EXIT();
    return provider;
}

ProviderModule* DefaultProviderManager::_lookupModule(
    const String& moduleFileName)
{
    PEG_METHOD_ENTER(TRC_PROVIDERMANAGER,
        "DefaultProviderManager::_lookupModule");

    // lock the providerTable mutex
    AutoMutex lock(_providerTableMutex);

    // look up provider module in cache
    ProviderModule* module = 0;

    if (_modules.lookup(moduleFileName, module))
    {
        // found provider module in cache
        PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL4,
            "Found Provider Module %s in Provider Manager Cache",
            (const char*)moduleFileName.getCString()));
    }
    else
    {
        // provider module not found in cache, create provider module
        PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL4,
            "Creating Provider Module %s",
            (const char*)moduleFileName.getCString()));

        module = new ProviderModule(moduleFileName);

        // insert provider module in module table
        _modules.insert(moduleFileName, module);
    }

    PEG_METHOD_EXIT();
    return module;
}

Boolean DefaultProviderManager::hasActiveProviders()
{
    PEG_METHOD_ENTER(TRC_PROVIDERMANAGER,
        "DefaultProviderManager::hasActiveProviders");

    try
    {
        AutoMutex lock(_providerTableMutex);
        PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL4,
            "Number of providers in _providers table = %d", _providers.size()));

        // Iterate through the _providers table looking for an active provider
        for (ProviderTable::Iterator i = _providers.start(); i != 0; i++)
        {
            if (i.value()->status.isInitialized())
            {
                PEG_METHOD_EXIT();
                return true;
            }
        }
    }
    catch (...)
    {
        // Unexpected exception; do not assume that no providers are loaded
        PEG_TRACE_CSTRING(TRC_PROVIDERMANAGER, Tracer::LEVEL1,
            "Unexpected Exception in hasActiveProviders.");
        PEG_METHOD_EXIT();
        return true;
    }

    // No active providers were found in the _providers table
    PEG_METHOD_EXIT();
    return false;
}

void DefaultProviderManager::unloadIdleProviders()
{
    PEG_METHOD_ENTER(TRC_PROVIDERMANAGER,
        "DefaultProviderManager::unloadIdleProviders");

    try
    {
        struct timeval now;
        Time::gettimeofday(&now);

        // Make a copy of the table so it is not locked during provider calls
        Array<ProviderMessageHandler*> providerList;
        {
            AutoMutex lock(_providerTableMutex);

            for (ProviderTable::Iterator i = _providers.start(); i != 0; i++)
            {
                providerList.append(i.value());
            }
        }

        for (Uint32 i = 0; i < providerList.size(); i++)
        {
            ProviderMessageHandler* provider = providerList[i];

            AutoMutex lock(provider->status.getStatusMutex());

            if (!provider->status.isInitialized())
            {
                continue;
            }

            struct timeval providerTime = {0, 0};
            provider->status.getLastOperationEndTime(&providerTime);

            PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL4,
                "provider->status.isIdle() returns: %s",
                (const char*)CIMValue(provider->status.isIdle())
                       .toString().getCString()));

            if (provider->status.isIdle() &&
                ((now.tv_sec - providerTime.tv_sec) >
                 ((Sint32)PEGASUS_PROVIDER_IDLE_TIMEOUT_SECONDS)))
            {
                PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL3,
                    "Unloading idle provider: %s",
                    (const char*)provider->getName().getCString()));
                _unloadProvider(provider);
            }
        }
    }
    catch (...)
    {
        PEG_TRACE_CSTRING(TRC_PROVIDERMANAGER, Tracer::LEVEL1,
            "Caught unexpected exception in unloadIdleProviders.");
    }

    PEG_METHOD_EXIT();
}

void DefaultProviderManager::_shutdownAllProviders()
{
    PEG_METHOD_ENTER(TRC_PROVIDERMANAGER,
        "DefaultProviderManager::_shutdownAllProviders");

    AutoMutex lock(_providerTableMutex);
    PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL4,
        "providers in cache = %d", _providers.size()));

    //create an array of UnloadProviderRequest requests one per
    //provider to process shutdown of providers simultaneously.
    Array<AsyncRequestExecutor::AsyncRequestMsg*> ProviderRequests;
    for (ProviderTable::Iterator i = _providers.start(); i != 0; i++)
    {
        AutoMutex lock(i.value()->status.getStatusMutex());
        if(i.value()->status.isInitialized())
        {
            ProviderRequests.append(
                new UnloadProviderRequest(i.value()));
        }
    }

    //run the stop request on all providers on multiple threads using
    //the request executor. This will invoke _asyncRequestCallback() on a
    //seperate thread for each provider which in turn will unload that
    //provider.

    CIMException exception =
        AsyncRequestExecutor(&_asyncRequestCallback,this).executeRequests(
            ProviderRequests);

    if(exception.getCode() != CIM_ERR_SUCCESS)
    {
        PEG_TRACE_CSTRING(TRC_PROVIDERMANAGER, Tracer::LEVEL2,
            "Unexpected Exception in _shutdownAllProviders().");
    }

    PEG_METHOD_EXIT();
}

Sint16 DefaultProviderManager::_disableProvider(
    const String& moduleName,
    const String& providerName)
{
    PEG_METHOD_ENTER(TRC_PROVIDERMANAGER,
        "DefaultProviderManager::_disableProvider");

    ProviderMessageHandler* pr = _lookupProvider(moduleName, providerName);
    if (!pr->status.isInitialized())
    {
        PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL2,
            "Provider %s is not loaded",
            (const char*)providerName.getCString()));
        PEG_METHOD_EXIT();
        return 1;
    }

    PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL4,"Disable Provider %s",
        (const char*)pr->getName().getCString()));
    //
    // Check to see if there are pending requests. If there are pending
    // requests and the disable timeout has not expired, loop and wait one
    // second until either there is no pending requests or until timeout
    // expires.
    //
    Uint32 waitTime = PROVIDER_DISABLE_TIMEOUT;
    while ((pr->status.numCurrentOperations() > 0) && (waitTime > 0))
    {
        Threads::sleep(1000);
        waitTime = waitTime - 1;
    }

    // There are still pending requests, do not disable
    if (pr->status.numCurrentOperations() > 0)
    {
        PEG_TRACE_CSTRING(TRC_PROVIDERMANAGER, Tracer::LEVEL4,
            "Disable failed since there are pending requests.");
        PEG_METHOD_EXIT();
        return 0;
    }

    try
    {
        AutoMutex lock(pr->status.getStatusMutex());

        if (pr->status.isInitialized())
        {
            PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL4,
                "Unloading Provider %s",
                (const char*)pr->getName().getCString()));
            _unloadProvider(pr);
        }
    }
    catch (...)
    {
        PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL2,
            "Unload provider failed %s",
            (const char*)pr->getName().getCString()));
        PEG_METHOD_EXIT();
        return -1;
    }

    PEG_METHOD_EXIT();
    return 1;
}

void DefaultProviderManager::_unloadProvider(ProviderMessageHandler* provider)
{
    //
    // NOTE:  It is the caller's responsibility to make sure that the
    // provider->status.getStatusMutex() mutex is locked before calling
    // this method.
    //

    PEG_METHOD_ENTER(TRC_PROVIDERMANAGER,
        "DefaultProviderManager::_unloadProvider");

    if (provider->status.numCurrentOperations() > 0)
    {
        PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL4,
            "Provider cannot be unloaded due to pending operations: %s",
            (const char*)provider->getName().getCString()));
    }
    else
    {
        PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL4,
            "Terminating Provider %s",
            (const char*)provider->getName().getCString()));

        provider->terminate();

        // unload provider module
        PEGASUS_ASSERT(provider->status.getModule() != 0);
        PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL3,
            "Unloading provider module: %s",
            (const char*)provider->getName().getCString()));
        provider->status.getModule()->unloadModule();

        PEG_TRACE((TRC_PROVIDERMANAGER,Tracer::LEVEL3,
            "DefaultProviderManager: Unloaded provider %s",
            (const char*)provider->getName().getCString()));

        // NOTE: The "delete provider->status.getCIMOMHandle()" operation
        //   was moved to be called after the unloadModule() call above
        //   as part of a fix for bugzilla 3669. For some providers
        //   run out-of-process on Windows platforms (i.e. running
        //   the cimserver with the forceProviderProcesses config option
        //   set to "true"), deleting the provider's CIMOMHandle before
        //   unloading the provider library caused the unload mechanism
        //   to deadlock, making that provider unavailable and preventing
        //   the cimserver from shutting down. It should NOT be moved back
        //   above the unloadModule() call. See bugzilla 3669 for details.

        // delete the cimom handle
        PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL4,
            "Destroying provider's CIMOMHandle: %s",
            (const char*)provider->getName().getCString()));
        delete provider->status.getCIMOMHandle();

        // set provider status to uninitialized
        provider->status.setInitialized(false);
    }

    PEG_METHOD_EXIT();
}

ProviderManager* DefaultProviderManager::createDefaultProviderManagerCallback()
{
    return new DefaultProviderManager();
}

//async request handler method invoked on a seperate thread per provider
//through the async request executor.
CIMException DefaultProviderManager::_asyncRequestCallback(
    void *callbackPtr,
    AsyncRequestExecutor::AsyncRequestMsg* request)
{
    PEG_METHOD_ENTER(TRC_PROVIDERMANAGER,
        "DefaultProviderManager::_asyncRequestCallback");

    CIMException responseException;

    //extract the parameters
    UnloadProviderRequest* my_request =
        dynamic_cast<UnloadProviderRequest*>(request);
    if(my_request != NULL)
    {
        PEGASUS_ASSERT(0 != callbackPtr);

        DefaultProviderManager *dpmPtr =
            static_cast<DefaultProviderManager*>(callbackPtr);

        ProviderMessageHandler* provider =
            dynamic_cast<ProviderMessageHandler*>(my_request->_provider);
        try
        {
            AutoMutex lock(provider->status.getStatusMutex());
            //unload the provider
            if (provider->status.isInitialized())
            {
                dpmPtr->_unloadProvider(provider);
            }
            else
            {
                PEGASUS_ASSERT(0);
            }
       }
        catch (CIMException& e)
        {
            PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL1,"CIMException: %s",
                (const char*)e.getMessage().getCString()));
            responseException = e;
        }
        catch (Exception& e)
        {
            PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL1,"Exception: %s",
                (const char*)e.getMessage().getCString()));
            responseException = CIMException(CIM_ERR_FAILED, e.getMessage());
        }
        catch (PEGASUS_STD(exception)& e)
        {
            responseException = CIMException(CIM_ERR_FAILED, e.what());
        }
        catch (...)
        {
            PEG_TRACE_CSTRING(TRC_PROVIDERMANAGER, Tracer::LEVEL1,
                "Exception: Unknown");
            responseException = PEGASUS_CIM_EXCEPTION(
                CIM_ERR_FAILED, "Unknown error.");
        }
    }

    // delete the UnloadProviderRequest.
    delete request;

    PEG_METHOD_EXIT();
    return responseException;
}

PEGASUS_NAMESPACE_END

PEGASUS_USING_PEGASUS;

// This entry point is not needed because the DefaultProviderManager is created
// using DefaultProviderManager::createDefaultProviderManagerCallback().
#if 0
extern "C" PEGASUS_EXPORT ProviderManager* PegasusCreateProviderManager(
    const String& providerManagerName)
{
    if (String::equalNoCase(providerManagerName, "Default"))
    {
        return new DefaultProviderManager();
    }

    return 0;
}
#endif
