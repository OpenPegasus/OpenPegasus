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

#include "BasicProviderManagerRouter.h"

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/OperationContextInternal.h>
#include <Pegasus/Common/CIMMessage.h>
#include <Pegasus/Common/Tracer.h>
#include <Pegasus/Common/Logger.h>
#include <Pegasus/Common/FileSystem.h>
#include <Pegasus/Config/ConfigManager.h>
#include <Pegasus/ProviderManagerRouter/ProviderManagerModule.h>
#include <Pegasus/ProviderManager2/ProviderManager.h>


PEGASUS_NAMESPACE_BEGIN


class ProviderManagerContainer
{
public:

    ProviderManagerContainer(
        const String& physicalName,
        const String& logicalName,
        const String& interfaceName,
        PEGASUS_INDICATION_CALLBACK_T indicationCallback,
        PEGASUS_RESPONSE_CHUNK_CALLBACK_T responseChunkCallback,
        Boolean subscriptionInitComplete)
    : _manager(0)
    {
#if defined (PEGASUS_OS_VMS)
    String provDir = ConfigManager::getInstance()->
                                    getCurrentValue("providerDir");
    _physicalName = ConfigManager::getHomedPath(provDir) + "/" +
                       FileSystem::buildLibraryFileName(physicalName);
#else
        _physicalName = physicalName;  // providerMgrPath comes with full path
        //_physicalName = ConfigManager::getHomedPath(PEGASUS_DEST_LIB_DIR) +
        //    String("/") + FileSystem::buildLibraryFileName(physicalName);
#endif

        _logicalName = logicalName;
        _interfaceName = interfaceName;

        _module.reset(new ProviderManagerModule(_physicalName));
        Boolean moduleLoaded = _module->load();

        if (moduleLoaded)
        {
            _manager = _module->getProviderManager(_logicalName);
        }
        else
        {
            PEG_TRACE_CSTRING(TRC_PROVIDERMANAGER, Tracer::LEVEL2,
                "ProviderManagerModule load failed.");
        }

        if (_manager == 0)
        {
            MessageLoaderParms parms(
                "ProviderManager.BasicProviderManagerRouter."
                    "PROVIDERMANAGER_LOAD_FAILED",
                "Failed to load the Provider Manager for interface "
                    "type \"$0\" from library \"$1\".",
                _interfaceName, _physicalName);
            Logger::put_l(
                Logger::ERROR_LOG, System::CIMSERVER, Logger::SEVERE,
                parms);
            throw PEGASUS_CIM_EXCEPTION_L(CIM_ERR_FAILED, parms);
        }

        _manager->setIndicationCallback(indicationCallback);
        _manager->setResponseChunkCallback(responseChunkCallback);

        _manager->setSubscriptionInitComplete (subscriptionInitComplete);
    }

    ProviderManagerContainer(
        const String& interfaceName,
        PEGASUS_INDICATION_CALLBACK_T indicationCallback,
        PEGASUS_RESPONSE_CHUNK_CALLBACK_T responseChunkCallback,
        Boolean subscriptionInitComplete,
        ProviderManager* manager)
        :
        _interfaceName(interfaceName),
        _manager(manager),
        _module(0)
    {
        _manager->setIndicationCallback(indicationCallback);
        _manager->setResponseChunkCallback(responseChunkCallback);
        _manager->setSubscriptionInitComplete(subscriptionInitComplete);
    }

    ~ProviderManagerContainer()
    {
        delete _manager;

        if (_module.get())
            _module->unload();
    }

    ProviderManager* getProviderManager()
    {
        return _manager;
    }

    const String& getInterfaceName() const
    {
        return _interfaceName;
    }


private:

    ProviderManagerContainer();
    ProviderManagerContainer& operator=(const ProviderManagerContainer&);
    ProviderManagerContainer(const ProviderManagerContainer&);

    String _physicalName;
    String _logicalName;
    String _interfaceName;
    ProviderManager* _manager;
    AutoPtr<ProviderManagerModule> _module;
};

PEGASUS_INDICATION_CALLBACK_T
    BasicProviderManagerRouter::_indicationCallback = 0;

PEGASUS_RESPONSE_CHUNK_CALLBACK_T
    BasicProviderManagerRouter::_responseChunkCallback = 0;

ProviderManager*
    (*BasicProviderManagerRouter::_createDefaultProviderManagerCallback)() = 0;

BasicProviderManagerRouter::BasicProviderManagerRouter(
    PEGASUS_INDICATION_CALLBACK_T indicationCallback,
    PEGASUS_RESPONSE_CHUNK_CALLBACK_T responseChunkCallback,
    ProviderManager* (*createDefaultProviderManagerCallback)())
{
    PEG_METHOD_ENTER(TRC_PROVIDERMANAGER,
        "BasicProviderManagerRouter::BasicProviderManagerRouter");

    _indicationCallback = indicationCallback;
    _responseChunkCallback = responseChunkCallback;
    _subscriptionInitComplete = false;
    _createDefaultProviderManagerCallback =
        createDefaultProviderManagerCallback;

    PEG_METHOD_EXIT();
}

BasicProviderManagerRouter::~BasicProviderManagerRouter()
{
    PEG_METHOD_ENTER(TRC_PROVIDERMANAGER,
        "BasicProviderManagerRouter::~BasicProviderManagerRouter");
    /* Clean up the provider managers */
    for (Uint32 i = 0, n = _providerManagerTable.size(); i < n; i++)
    {
         ProviderManagerContainer* pmc=_providerManagerTable[i];
         delete pmc;
    }
    PEG_METHOD_EXIT();
}

Message* BasicProviderManagerRouter::processMessage(Message * message)
{
    PEG_METHOD_ENTER(TRC_PROVIDERMANAGER,
        "BasicProviderManagerRouter::processMessage");

    CIMRequestMessage* request = dynamic_cast<CIMRequestMessage *>(message);
    PEGASUS_ASSERT(request != 0);

    Message* response = 0;
    Boolean remoteNameSpaceRequest=false;
    Boolean loadProviderManager=true;

    //
    // Retrieve the ProviderManager routing information
    //

    CIMInstance providerModule;

    if ((dynamic_cast<CIMOperationRequestMessage*>(request) != 0) ||
        (request->getType() == CIM_EXPORT_INDICATION_REQUEST_MESSAGE))
    {
        // Provider information is in OperationContext
        ProviderIdContainer pidc = (ProviderIdContainer)
            request->operationContext.get(ProviderIdContainer::NAME);
        providerModule = pidc.getModule();
        remoteNameSpaceRequest=pidc.isRemoteNameSpace();
    }
    else if (dynamic_cast<CIMIndicationRequestMessage*>(request) != 0)
    {
        // Provider information is in CIMIndicationRequestMessage
        CIMIndicationRequestMessage* indReq =
            dynamic_cast<CIMIndicationRequestMessage*>(request);
        ProviderIdContainer pidc =
            indReq->operationContext.get(ProviderIdContainer::NAME);
        providerModule = pidc.getModule();
    }
    else if (request->getType() == CIM_ENABLE_MODULE_REQUEST_MESSAGE)
    {
        // Provider information is in CIMEnableModuleRequestMessage
        CIMEnableModuleRequestMessage* emReq =
            dynamic_cast<CIMEnableModuleRequestMessage*>(request);
        providerModule = emReq->providerModule;
        // Do not try to load the provider manager module if not already loaded.
        loadProviderManager=false;
    }
    else if (request->getType() == CIM_DISABLE_MODULE_REQUEST_MESSAGE)
    {
        // Provider information is in CIMDisableModuleRequestMessage
        CIMDisableModuleRequestMessage* dmReq =
            dynamic_cast<CIMDisableModuleRequestMessage*>(request);
        providerModule = dmReq->providerModule;
        // Do not try to load the provider manager module if not already loaded.
        loadProviderManager=false;
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
        // Error: Unrecognized message type.
        PEGASUS_ASSERT(0);
        CIMResponseMessage* resp = request->buildResponse();
        resp->cimException = PEGASUS_CIM_EXCEPTION(CIM_ERR_FAILED,
            "Unknown message type.");
        response = resp;
    }

    //
    // Forward the request to the appropriate ProviderManager(s)
    //

    if ((request->getType() == CIM_STOP_ALL_PROVIDERS_REQUEST_MESSAGE) ||
        (request->getType() ==
            CIM_SUBSCRIPTION_INIT_COMPLETE_REQUEST_MESSAGE) ||
        (request->getType() ==
            CIM_INDICATION_SERVICE_DISABLED_REQUEST_MESSAGE))
    {
        if (request->getType() ==
            CIM_INDICATION_SERVICE_DISABLED_REQUEST_MESSAGE)
        {
            _subscriptionInitComplete = false;
        }
        else
        {
            _subscriptionInitComplete = true;
        }

        // Send CIMStopAllProvidersRequestMessage or
        // CIMIndicationServiceDisabledRequestMessage or
        // CIMSubscriptionInitCompleteRequestMessage to all ProviderManagers
        ReadLock tableLock(_providerManagerTableLock);
        for (Uint32 i = 0, n = _providerManagerTable.size(); i < n; i++)
        {
            ProviderManagerContainer* pmc=_providerManagerTable[i];
            Message* resp = pmc->getProviderManager()->processMessage(request);
            delete resp;
        }

        response = request->buildResponse();
    }
    else if (request->getType() == CIM_NOTIFY_CONFIG_CHANGE_REQUEST_MESSAGE)
    {
        // Do not need to forward this request to in-process provider
        // managers
        response = request->buildResponse();
    }
    else
    {
        // Retrieve the provider interface type
        String interfaceType;
        CIMValue itValue = providerModule.getProperty(
            providerModule.findProperty("InterfaceType")).getValue();
        itValue.get(interfaceType);
        // Get providerManager path
        String provMgrPath;
        if (request->operationContext.contains(ProviderIdContainer::NAME))
        {
            ProviderIdContainer pidc = (ProviderIdContainer)
            request->operationContext.get(ProviderIdContainer::NAME);
            provMgrPath = pidc.getProvMgrPath();
        }

        ProviderManager* pm = 0;
        try
        {
            // Look up the appropriate ProviderManager by InterfaceType

            pm = _getProviderManager(
                    interfaceType,
                    provMgrPath,
                    loadProviderManager);
        }
        catch (const CIMException& e)
        {
            CIMResponseMessage *cimResponse = request->buildResponse();
            cimResponse->cimException = e;
            response = cimResponse;
        }

        // Incase of CIMEnableModuleRequestMessage or 
        // CIMDisableModuleRequestMessage, there must be not necessarily 
        // a running provider manager. This is not an error.
        // This means there is no provider to enable or disable.
        if (0 == pm)
        {
            if (request->getType() == CIM_ENABLE_MODULE_REQUEST_MESSAGE)
            {
                CIMEnableModuleResponseMessage* emResponse =
                    dynamic_cast<CIMEnableModuleResponseMessage*>(
                        request->buildResponse());
                emResponse->operationalStatus.append(
                    CIM_MSE_OPSTATUS_VALUE_OK);
                response = emResponse;
            }
            else if (request->getType() == CIM_DISABLE_MODULE_REQUEST_MESSAGE)
            {
                CIMDisableModuleResponseMessage* dmResponse =
                    dynamic_cast<CIMDisableModuleResponseMessage*>(
                        request->buildResponse());
                dmResponse->operationalStatus.append(
                    CIM_MSE_OPSTATUS_VALUE_STOPPED);
                response = dmResponse;
            } 
            else
            {                
                CIMResponseMessage* resp = request->buildResponse();
                resp->cimException = 
                    PEGASUS_CIM_EXCEPTION(CIM_ERR_FAILED,
                        "There is no Provider Manager for interfaceType '" +
                        interfaceType + ". The request message type is '" +
                        String(MessageTypeToString(request->getType())) + "'");

                PEG_TRACE((TRC_PROVIDERMANAGER,Tracer::LEVEL1,"%s",
                    (const char*)resp->cimException.getMessage().getCString()));

                response = resp;
            }
        } 
        else
        {
            if ( remoteNameSpaceRequest && !pm->supportsRemoteNameSpaces())
            {
                CIMResponseMessage* resp = request->buildResponse();
                resp->cimException = PEGASUS_CIM_EXCEPTION(CIM_ERR_FAILED,
                 "Remote Namespace operations not supported for interface type "
                        + interfaceType);
                response = resp;
            }
            else
            {
                response = pm->processMessage(request);
            }
        }
    }

    PEG_METHOD_EXIT();
    return response;
}

// ATTN: May need to add interfaceVersion parameter to further constrain lookup
ProviderManager* BasicProviderManagerRouter::_getProviderManager(
    const String& interfaceType,
    const String& providerManagerPath,
    Boolean loadProviderManager)
{
    PEG_METHOD_ENTER(TRC_PROVIDERMANAGER,
        "BasicProviderManagerRouter::_getProviderManager");

    //
    // Search for this InterfaceType in the table of loaded ProviderManagers
    //
    {
        ReadLock tableLock(_providerManagerTableLock);

        ProviderManager* pm = _lookupProviderManager(interfaceType);
        if (pm)
        {
            PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL4,
                "Provider Manager for interfaceType '%s' already loaded.",
                (const char*)interfaceType.getCString()));
            PEG_METHOD_EXIT();
            return pm;
        }
    }

    //
    // If requested, do not load the ProviderManger.
    // 
    if (!loadProviderManager)
    {
        PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL4,
            "Requested not to load the Provider Manager "
                "for interfaceType '%s'.",
            (const char*)interfaceType.getCString()));
        PEG_METHOD_EXIT();
        return 0;
    }

    //
    // Load the ProviderManager for this InterfaceType and add it to the table
    //
    {
        WriteLock tableLock(_providerManagerTableLock);

        ProviderManager* pm = _lookupProviderManager(interfaceType);
        if (pm)
        {
            PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL4,
                "Provider Manager for interfaceType '%s' already loaded.",
                (const char*)interfaceType.getCString()));
            PEG_METHOD_EXIT();
            return pm;
        }

        // The DefaultProviderManager is now statically linked rather than
        // dynamically loaded. This code is no longer used but remains for
        // reference purposes.

#if defined(PEGASUS_ENABLE_DEFAULT_PROVIDER_MANAGER)
        if (interfaceType == "C++Default" &&
            _createDefaultProviderManagerCallback)
        {
            pm = (*_createDefaultProviderManagerCallback)();
            ProviderManagerContainer* pmc = new ProviderManagerContainer(
                "C++Default",
                _indicationCallback,
                _responseChunkCallback,
                _subscriptionInitComplete,
                pm);
            _providerManagerTable.append(pmc);
            PEG_METHOD_EXIT();
            return pmc->getProviderManager();
        }
#endif

        PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL4,
            "Crating new Provider Manager for interfaceType '%s', "
                "providerManagerPath '%s'.",
            (const char*)interfaceType.getCString(),
            (const char*)providerManagerPath.getCString()));

        ProviderManagerContainer* pmc = new ProviderManagerContainer(
            providerManagerPath,
            interfaceType,
            interfaceType,
            _indicationCallback,
            _responseChunkCallback,
            _subscriptionInitComplete);
        _providerManagerTable.append(pmc);
        PEG_METHOD_EXIT();
        return pmc->getProviderManager();
    }

}

// NOTE: The caller must lock _providerManagerTableLock before calling this
// method.
ProviderManager* BasicProviderManagerRouter::_lookupProviderManager(
    const String& interfaceType)
{
    PEG_METHOD_ENTER(TRC_PROVIDERMANAGER,
        "BasicProviderManagerRouter::_lookupProviderManager");

    //
    // Search for this InterfaceType in the table of loaded ProviderManagers
    //
    for (Uint32 i = 0, n = _providerManagerTable.size(); i < n; i++)
    {
        if (interfaceType == _providerManagerTable[i]->getInterfaceName())
        {
            ProviderManagerContainer* pmc = _providerManagerTable[i];
            PEG_METHOD_EXIT();
            return pmc->getProviderManager();
        }
    }

    // Not found
    PEG_METHOD_EXIT();
    return 0;
}

Boolean BasicProviderManagerRouter::hasActiveProviders()
{
    PEG_METHOD_ENTER(TRC_PROVIDERMANAGER,
        "BasicProviderManagerRouter::hasActiveProviders");

    ReadLock tableLock(_providerManagerTableLock);
    for (Uint32 i = 0, n = _providerManagerTable.size(); i < n; i++)
    {
        ProviderManagerContainer* pmc = _providerManagerTable[i];
        if (pmc->getProviderManager()->hasActiveProviders())
        {
            PEG_METHOD_EXIT();
            return true;
        }
    }

    PEG_METHOD_EXIT();
    return false;
}

void BasicProviderManagerRouter::idleTimeCleanup()
{
    PEG_METHOD_ENTER(TRC_PROVIDERMANAGER,
        "BasicProviderManagerRouter::idleTimeCleanup");

    //
    // Save pointers to the ProviderManagerContainers so we don't hold the
    // _providerManagerTableLock while unloading idle providers
    //
    Array<ProviderManagerContainer*> pmcs;
    {
        ReadLock tableLock(_providerManagerTableLock);
        for (Uint32 i = 0, n = _providerManagerTable.size(); i < n; ++i)
        {
            pmcs.append(_providerManagerTable[i]);
        }
    }

    //
    // Unload idle providers in each of the active ProviderManagers
    // _providerManagerTableLock while unloading idle providers
    //
    for (Uint32 i = 0; i < pmcs.size(); ++i)
    {
        pmcs[i]->getProviderManager()->unloadIdleProviders();
    }

    PEG_METHOD_EXIT();
}
// Does nothing today.
void BasicProviderManagerRouter::enumerationContextCleanup(
    const String& contextId)
{
}
PEGASUS_NAMESPACE_END
