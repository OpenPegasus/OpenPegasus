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

#include <Pegasus/Common/Constants.h>
#include <Pegasus/Common/CIMInstance.h>
#include <Pegasus/Common/ArrayInternal.h>
#include <Pegasus/Common/CIMDateTime.h>
#include <Pegasus/Common/CIMProperty.h>
#include <Pegasus/Common/MessageQueue.h>
#include <Pegasus/Common/System.h>
#include <Pegasus/Common/Tracer.h>
#include <Pegasus/Common/XmlWriter.h>
#include <Pegasus/Common/PegasusVersion.h>
#include <Pegasus/Common/LanguageParser.h>
#include <Pegasus/Common/OperationContextInternal.h>
#include <Pegasus/Common/MessageLoader.h>
#include <Pegasus/Common/String.h>

#include <Pegasus/General/Guid.h>
#ifdef PEGASUS_INDICATION_PERFINST
#include <Pegasus/General/Stopwatch.h>
#endif

#include <Pegasus/Server/ProviderRegistrationManager/\
ProviderRegistrationManager.h>
#include <Pegasus/Query/QueryExpression/QueryExpression.h>
#include <Pegasus/Query/QueryCommon/QueryException.h>
#include <Pegasus/Repository/RepositoryQueryContext.h>

#include <Pegasus/Handler/IndicationFormatter.h>


#include "IndicationConstants.h"
#include "SubscriptionRepository.h"
#include "SubscriptionTable.h"
#include "IndicationService.h"


PEGASUS_USING_STD;

PEGASUS_NAMESPACE_BEGIN

//
// Message constants
//

static const char _MSG_PROPERTY_KEY[] =
   "IndicationService.IndicationService._MSG_PROPERTY";
static const char _MSG_PROPERTY[] = "The required property $0 is missing.";

static const char _MSG_NO_PROVIDERS_KEY[] =
    "IndicationService.IndicationService._MSG_NO_PROVIDERS";
static const char _MSG_NO_PROVIDERS[] =
    "No providers are capable of servicing the subscription.";

static const char _MSG_INVALID_TYPE_FOR_PROPERTY_KEY[] =
    "IndicationService.IndicationService._MSG_INVALID_TYPE_FOR_PROPERTY";
static const char _MSG_INVALID_TYPE_FOR_PROPERTY[] =
    "The value of type $0 is not valid for property $1.";

static const char _MSG_INVALID_TYPE_ARRAY_OF_FOR_PROPERTY_KEY[] =
    "IndicationService.IndicationService."
        "_MSG_INVALID_TYPE_ARRAY_OF_FOR_PROPERTY";
static const char _MSG_INVALID_TYPE_ARRAY_OF_FOR_PROPERTY[] =
    "The value of an array of type $0 is not valid for property $1.";

static const char _MSG_INVALID_VALUE_FOR_PROPERTY_KEY[] =
    "IndicationService.IndicationService._MSG_INVALID_VALUE_FOR_PROPERTY";
static const char _MSG_INVALID_VALUE_FOR_PROPERTY[] =
    "The value $0 is not valid for property $1.";

static const char _MSG_UNSUPPORTED_VALUE_FOR_PROPERTY_KEY[] =
    "IndicationService.IndicationService._MSG_UNSUPPORTED_VALUE_FOR_PROPERTY";
static const char _MSG_UNSUPPORTED_VALUE_FOR_PROPERTY[] =
    "The value $0 is not supported for property $1.";

static const char _MSG_CLASS_NOT_SERVED_KEY[] =
    "IndicationService.IndicationService._MSG_CLASS_NOT_SERVED";
static const char _MSG_CLASS_NOT_SERVED[] =
    "The specified class is not serviced by the CIM Indication service.";

static const char _MSG_INVALID_INSTANCES_KEY[] =
    "IndicationService.IndicationService."
        "INVALID_SUBSCRIPTION_INSTANCES_IGNORED";
static const char _MSG_INVALID_INSTANCES[] =
    "One or more subscription instances are not valid and are ignored.";

static const char _MSG_PROVIDER_NO_LONGER_SERVING_KEY[] =
    "IndicationService.IndicationService._MSG_PROVIDER_NO_LONGER_SERVING";
static const char _MSG_PROVIDER_NO_LONGER_SERVING[] =
    "Provider ($0) is no longer serving subscription ($1) in namespace $2";

static const char _MSG_PROVIDER_NOW_SERVING_KEY[] =
    "IndicationService.IndicationService._MSG_PROVIDER_NOW_SERVING";
static const char _MSG_PROVIDER_NOW_SERVING[] =
    "Provider ($0) is now serving subscription ($1) in namespace $2";

static const char _MSG_NO_PROVIDER_KEY[] =
    "IndicationService.IndicationService._MSG_NO_PROVIDER";
static const char _MSG_NO_PROVIDER[] =
    "Subscription ($0) in namespace $1 has no provider";

static const char _MSG_STATE_CHANGE_FAILED_KEY[] =
    "IndicationService.IndicationService.STATE_CHANGE_FAILED";
static const char _MSG_STATE_CHANGE_FAILED[] =
    "The requested state change failed : $0. Current IndicationService"
        " EnabledState : $1, HealthState : $2.";

static const char _MSG_NOT_CREATOR_KEY[] =
    "IndicationService.IndicationService._MSG_NOT_CREATOR";
static const char _MSG_NOT_CREATOR[] =
    "The current user($0) is not the creator($1)."
    "Hence operation not permitted";

// ATTN-RK-20020730: Temporary hack to fix Windows build
Boolean ContainsCIMName(const Array<CIMName>& a, const CIMName& x)
{
    Uint32 n = a.size();

    for (Uint32 i = 0; i < n; i++)
    {
        if (a[i].equal(x))
            return true;
    }

    return false;
}

Mutex IndicationService::_mutex;

/**
    See CIM_EnabledLogicalElement.RequestStateChange() method for return codes
    and CIM_EnabledLogicalElement.EnabledState property for  service states.

    ATTN: Currently very few states are supported and the following utility
    functions are  hard coded to return service states directly. Write a
    generic function to get values  from ValueMap independent of class.
*/
String _getEnabledStateString(Uint32 code)
{
    // Check for service states
    switch(code)
    {
        case _ENABLEDSTATE_ENABLED:
            return String("Enabled");
        case _ENABLEDSTATE_DISABLED:
            return String("Disabled");
        case _ENABLEDSTATE_SHUTTINGDOWN:
            return String("Shutting Down");
        case _ENABLEDSTATE_STARTING:
            return String("Starting");
    }
    PEGASUS_UNREACHABLE(PEGASUS_ASSERT(false);)

    return String("Unknown");
}

String _getHealthStateString(Uint32 code)
{
    // Service health states
    switch(code)
    {
        case _HEALTHSTATE_OK:
            return String("OK");
        case _HEALTHSTATE_DEGRADEDWARNING:
            return String("Degraded/Warning");
    }
    // Never reach to unknown state at present.
    PEGASUS_UNREACHABLE(PEGASUS_ASSERT(false);)

    return String("Unknown");
}

IndicationService::IndicationService(
    CIMRepository* repository,
    ProviderRegistrationManager* providerRegManager)
    : MessageQueueService(
          PEGASUS_QUEUENAME_INDICATIONSERVICE),
      _providerRegManager(providerRegManager),
      _cimRepository(repository),
     // NOTE: Create ControlProvIndRegTable with only one chain. Only one
     // indication control provider is available at this time.
     _controlProvIndRegTable(1)
{
    _buildInternalControlProvidersRegistration();
    _enableSubscriptionsForNonprivilegedUsers = false;
    _authenticationEnabled = true;

    try
    {
        // Determine the value for the configuration parameter
        // enableSubscriptionsForNonprivilegedUsers
        ConfigManager* configManager = ConfigManager::getInstance();

        if (ConfigManager::parseBooleanValue(
            configManager->getCurrentValue("enableAuthentication")))
        {
            _enableSubscriptionsForNonprivilegedUsers =
                ConfigManager::parseBooleanValue(
                    configManager->getCurrentValue(
                        "enableSubscriptionsForNonprivilegedUsers"));
        }
        else
        {
            _authenticationEnabled = false;
            // Authentication needs to be enabled to perform authorization
            // tests.
            _enableSubscriptionsForNonprivilegedUsers = true;
        }
     }
     catch (...)
     {
        // If there is an error reading the configuration file then
        // the value of _enableSubscriptionsForNonprivilegedUsers will
        // default to false (i.e., the more restrictive security
        // setting.
        PEG_TRACE_CSTRING(TRC_INDICATION_SERVICE, Tracer::LEVEL2,
            "Failure attempting to read configuration parameters during "
                "initialization.");
     }

    PEG_TRACE((TRC_INDICATION_SERVICE, Tracer::LEVEL4,
        "Value of _enableSubscriptionsForNonprivilegedUsers is %d",
        _enableSubscriptionsForNonprivilegedUsers));

    try
    {
        // Create IndicationsProfileInstance Repository
#ifdef PEGASUS_ENABLE_DMTF_INDICATION_PROFILE_SUPPORT
        _indicationServiceConfiguration.reset(
            new IndicationServiceConfiguration(_cimRepository));
#else
        _enabledState = _ENABLEDSTATE_DISABLED;
#endif
        // Initialize the Indication Service
        _initialize();
    }
    catch (Exception& e)
    {
        PEG_TRACE((TRC_INDICATION_SERVICE, Tracer::LEVEL1,
           "Exception caught in attempting to "
           "initialize Indication Service: %s",
           (const char*)e.getMessage().getCString()));
#ifdef PEGASUS_ENABLE_DMTF_INDICATION_PROFILE_SUPPORT
        _indicationServiceConfiguration->setHealthState(
             _HEALTHSTATE_DEGRADEDWARNING);
#endif
    }

}

IndicationService::~IndicationService()
{
    _controlProvIndRegTable.clear();
}

void IndicationService::_buildInternalControlProvidersRegistration()
{
    // ProvRegistrationProvider
    ControlProvIndReg regProvider;

    regProvider.className =
        PEGASUS_CLASSNAME_PROVIDERMODULE_INSTALERT;

   regProvider.providerModule =
        CIMInstance(PEGASUS_CLASSNAME_PROVIDERMODULE);

    regProvider.providerModule.addProperty(
        CIMProperty(
            PEGASUS_PROPERTYNAME_NAME,
            String(PEGASUS_MODULENAME_PROVREGPROVIDER)));

    regProvider.providerModule.addProperty(
        CIMProperty(
            PEGASUS_PROPERTYNAME_MODULE_USERCONTEXT,
            CIMValue()));

    regProvider.provider =
         CIMInstance(PEGASUS_CLASSNAME_PROVIDER);

    regProvider.provider.addProperty(
        CIMProperty(
            PEGASUS_PROPERTYNAME_NAME,
            String(PEGASUS_MODULENAME_PROVREGPROVIDER)));

    regProvider.provider.addProperty(
        CIMProperty(
            _PROPERTY_PROVIDERMODULENAME,
            String(PEGASUS_MODULENAME_PROVREGPROVIDER)));

    Array<CIMKeyBinding> keys;

    CIMKeyBinding kb1(
        PEGASUS_PROPERTYNAME_NAME,
        String(PEGASUS_MODULENAME_PROVREGPROVIDER),
        CIMKeyBinding::STRING);
    keys.append(kb1);

    CIMObjectPath pmPath =
        CIMObjectPath(
            String(),
            CIMNamespaceName(),
            PEGASUS_CLASSNAME_PROVIDERMODULE,
            keys);
    regProvider.providerModule.setPath (pmPath);

    CIMKeyBinding kb2(
        _PROPERTY_PROVIDERMODULENAME,
        String(PEGASUS_MODULENAME_PROVREGPROVIDER),
        CIMKeyBinding::STRING);
    keys.append(kb2);

    CIMObjectPath providerPath =
        CIMObjectPath(
            String(),
            CIMNamespaceName(),
            PEGASUS_CLASSNAME_PROVIDER,
            keys);

    regProvider.provider.setPath (providerPath);

    String key = PEGASUS_MODULENAME_PROVREGPROVIDER;
    key.append(PEGASUS_CLASSNAME_PROVIDERMODULE_INSTALERT.getString());

    PEGASUS_FCT_EXECUTE_AND_ASSERT(
        true,
        _controlProvIndRegTable.insert(key,regProvider));
}

Uint16  IndicationService::_getEnabledState()
{
#ifdef PEGASUS_ENABLE_DMTF_INDICATION_PROFILE_SUPPORT
    return _indicationServiceConfiguration->getEnabledState();
#else
    return _enabledState;
#endif
}

void IndicationService::_setEnabledState(Uint16 state)
{
#ifdef PEGASUS_ENABLE_DMTF_INDICATION_PROFILE_SUPPORT
    _indicationServiceConfiguration->setEnabledState(state);
#else
    _enabledState = state;
#endif
}

void IndicationService::_handle_async_request(AsyncRequest *req)
{
    if (req->getType() == ASYNC_CIMSERVICE_STOP)
    {
        //
        //  Call _terminate
        //
        _terminate();

        handle_CimServiceStop(static_cast<CimServiceStop *>(req));
    }
    else if (req->getType() == ASYNC_CIMSERVICE_START)
    {
        handle_CimServiceStart(static_cast<CimServiceStart *>(req));
    }
    else if (req->getType() == ASYNC_ASYNC_LEGACY_OP_START)
    {
        try
        {
            Message* legacy =
                static_cast<AsyncLegacyOperationStart *>(req)->get_action();
            legacy->put_async(req);

            handleEnqueue(legacy);
        }
        catch(Exception& )
        {
            PEG_TRACE_CSTRING(TRC_INDICATION_SERVICE, Tracer::LEVEL1,
                "Caught Exception in IndicationService while handling a "
                    "wrapped legacy message ");
                _make_response(req, async_results::CIM_NAK);
        }

        return;
    }
    else
        MessageQueueService::_handle_async_request(req);
}

void IndicationService::handleEnqueue(Message* message)
{
#ifdef PEGASUS_INDICATION_PERFINST
    Stopwatch stopWatch;

    stopWatch.start();
#endif

    if (message->getType() == CIM_PROCESS_INDICATION_RESPONSE_MESSAGE)
    {
        _handleProcessIndicationResponse(message);
        return;
    }

    CIMRequestMessage* cimRequest = dynamic_cast<CIMRequestMessage *>(message);
    PEGASUS_ASSERT(cimRequest);

    // Set the client's requested language into this service thread.
    // This will allow functions in this service to return messages
    // in the correct language.
    cimRequest->updateThreadLanguages();

    try
    {
        if (_getEnabledState() != _ENABLEDSTATE_ENABLED)
        {
            _handleCimRequestWithServiceNotEnabled(message);
        }
        else
        {
            _handleCimRequest(message);
        }
    }
    catch (CIMException& e)
    {
        PEG_TRACE((TRC_INDICATION_SERVICE, Tracer::LEVEL1,
            "CIMException caught in IndicationService::handleEnqueue: %s",
            (const char*)e.getMessage().getCString()));
        CIMResponseMessage* response = cimRequest->buildResponse();
        response->cimException = e;
        _enqueueResponse(cimRequest, response);
    }
    catch (Exception& e)
    {
        PEG_TRACE((TRC_INDICATION_SERVICE, Tracer::LEVEL1,
            "Exception caught in IndicationService::handleEnqueue: %s",
            (const char*)e.getMessage().getCString()));
        CIMResponseMessage* response = cimRequest->buildResponse();
        response->cimException =
            PEGASUS_CIM_EXCEPTION(CIM_ERR_FAILED, e.getMessage());
        _enqueueResponse(cimRequest, response);
    }
    catch (...)
    {
        PEG_TRACE((TRC_INDICATION_SERVICE, Tracer::LEVEL1,
            "Unknown exception caught in IndicationService::handleEnqueue."));
        CIMResponseMessage* response = cimRequest->buildResponse();
        response->cimException = PEGASUS_CIM_EXCEPTION_L(
            CIM_ERR_FAILED,
            MessageLoaderParms(
                "IndicationService.IndicationService.UNKNOWN_ERROR",
                "Unknown Error"));
        _enqueueResponse(cimRequest, response);
    }

#ifdef PEGASUS_INDICATION_PERFINST
    stopWatch.stop();

    PEG_TRACE((TRC_INDICATION_SERVICE, Tracer::LEVEL4,
        "%s: %.3f seconds",
        MessageTypeToString(message->getType()),
        stopWatch.getElapsed()));
#endif

   delete message;
}

void IndicationService::_handleCimRequest(Message *message)
{
    switch(message->getType())
    {
        case CIM_GET_INSTANCE_REQUEST_MESSAGE:
            _handleGetInstanceRequest(message);
        break;

        case CIM_ENUMERATE_INSTANCES_REQUEST_MESSAGE:
            _handleEnumerateInstancesRequest(message);
            break;

        case CIM_ENUMERATE_INSTANCE_NAMES_REQUEST_MESSAGE:
            _handleEnumerateInstanceNamesRequest(message);
            break;

        case CIM_CREATE_INSTANCE_REQUEST_MESSAGE:
            _handleCreateInstanceRequest(message);
            break;

        case CIM_MODIFY_INSTANCE_REQUEST_MESSAGE:
            _handleModifyInstanceRequest(message);
            break;

        case CIM_DELETE_INSTANCE_REQUEST_MESSAGE:
            _handleDeleteInstanceRequest(message);
            break;

        case CIM_PROCESS_INDICATION_REQUEST_MESSAGE:
            _handleProcessIndicationRequest(message);
            break;

        case CIM_NOTIFY_PROVIDER_REGISTRATION_REQUEST_MESSAGE:
            _handleNotifyProviderRegistrationRequest(message);
            break;

        case CIM_NOTIFY_PROVIDER_TERMINATION_REQUEST_MESSAGE:
            _handleNotifyProviderTerminationRequest(message);
            break;

        case CIM_NOTIFY_PROVIDER_ENABLE_REQUEST_MESSAGE:
            _handleNotifyProviderEnableRequest(message);
            break;

        case CIM_NOTIFY_PROVIDER_FAIL_REQUEST_MESSAGE:
            _handleNotifyProviderFailRequest(message);
            break;

#ifdef PEGASUS_ENABLE_DMTF_INDICATION_PROFILE_SUPPORT
        case CIM_INVOKE_METHOD_REQUEST_MESSAGE:
            _handleInvokeMethodRequest(message);
            break;
#endif

        default:
            CIMRequestMessage* cimRequest =
                dynamic_cast<CIMRequestMessage *>(message);
            //
            //  A message type not supported by the Indication Service
            //  Should not reach here
            //
            PEG_TRACE((TRC_INDICATION_SERVICE, Tracer::LEVEL1,
                "IndicationService::_handleCimRequest rcv'd unsupported "
                    "message of type %s.",
                MessageTypeToString(message->getType())));

            // Note: not setting Content-Language in the response
            CIMResponseMessage* response = cimRequest->buildResponse();
            response->cimException = PEGASUS_CIM_EXCEPTION_L(
                CIM_ERR_NOT_SUPPORTED,
                MessageLoaderParms(
                    "IndicationService.IndicationService."
                        "UNSUPPORTED_OPERATION",
                    "The requested operation is not supported or not "
                        "recognized by the indication service."));

            _enqueueResponse(cimRequest, response);
    }
}

void IndicationService::_handleCimRequestWithServiceNotEnabled(
    Message *message)
{
    Boolean requestHandled = false;
    CIMRequestMessage* cimRequest = dynamic_cast<CIMRequestMessage *>(message);

#ifdef PEGASUS_ENABLE_DMTF_INDICATION_PROFILE_SUPPORT
    requestHandled = true;
    switch(message->getType())
    {
        case CIM_INVOKE_METHOD_REQUEST_MESSAGE:
            _handleInvokeMethodRequest(message);
            break;
        case CIM_NOTIFY_PROVIDER_REGISTRATION_REQUEST_MESSAGE:
        case CIM_NOTIFY_PROVIDER_TERMINATION_REQUEST_MESSAGE:
        case CIM_NOTIFY_PROVIDER_ENABLE_REQUEST_MESSAGE:
        case CIM_NOTIFY_PROVIDER_FAIL_REQUEST_MESSAGE:
            _enqueueResponse(cimRequest, cimRequest->buildResponse());
            break;

        case CIM_PROCESS_INDICATION_REQUEST_MESSAGE:
            _handleProcessIndicationRequest(message);
            break;

        // Handle only CIM_IndicationService class operations.
        case CIM_GET_INSTANCE_REQUEST_MESSAGE:
            {
                CIMGetInstanceRequestMessage *request =
                    (CIMGetInstanceRequestMessage*)message;
                if (request->className.equal(
                    PEGASUS_CLASSNAME_CIM_INDICATIONSERVICE))
                {
                    _handleGetInstanceRequest(message);
                }
                else
                {
                    requestHandled = false;
                }
            }
            break;
        case CIM_ENUMERATE_INSTANCES_REQUEST_MESSAGE:
            {
                CIMEnumerateInstancesRequestMessage *request =
                    (CIMEnumerateInstancesRequestMessage*)message;
                if (request->className.equal(
                    PEGASUS_CLASSNAME_CIM_INDICATIONSERVICE))
                {
                    _handleEnumerateInstancesRequest(message);
                }
                else
                {
                    requestHandled = false;
                }
            }
            break;

        case CIM_ENUMERATE_INSTANCE_NAMES_REQUEST_MESSAGE:
            {
                CIMEnumerateInstanceNamesRequestMessage *request =
                    (CIMEnumerateInstanceNamesRequestMessage*)message;
                if (request->className.equal(
                    PEGASUS_CLASSNAME_CIM_INDICATIONSERVICE))
                {
                    _handleEnumerateInstanceNamesRequest(message);
                }
                else
                {
                    requestHandled = false;
                }
            }
            break;
        default:
            requestHandled = false;
            break;
    }
#endif

    if (!requestHandled)
    {
        Logger::put_l(
            Logger::STANDARD_LOG,
            System::CIMSERVER,
            Logger::WARNING,
            MessageLoaderParms(
                "IndicationService.IndicationService."
                    "CANNOT_EXECUTE_REQUEST",
                "The requested operation cannot be executed."
                    " IndicationService EnabledState : $0.",
                _getEnabledStateString(_getEnabledState())));

        CIMResponseMessage* response = cimRequest->buildResponse();
        response->cimException = PEGASUS_CIM_EXCEPTION_L(
            CIM_ERR_FAILED,
            MessageLoaderParms(
                "IndicationService.IndicationService."
                    "CANNOT_EXECUTE_REQUEST",
                "The requested operation cannot be executed."
                    " IndicationService EnabledState : $0.",
                _getEnabledStateString(_getEnabledState())));
        _enqueueResponse(cimRequest, response);
    }
}


void IndicationService::handleEnqueue()
{
    Message * message = dequeue();

    PEGASUS_ASSERT(message != 0);
    handleEnqueue(message);
}

void IndicationService::_setOrAddSystemNameInHandlerFilter(
    CIMInstance& instance,
    const String& sysname)
{
    // Key property SystemName should be ignored by server according to
    // DSP1054 v1.2, setting it to empty string for further processing
    // host name will replace empty string on returning instances
    Uint32 sysNamePos = instance.findProperty(_PROPERTY_SYSTEMNAME);
    CIMValue x = CIMValue(sysname);

    if (PEG_NOT_FOUND == sysNamePos)
    {
        instance.addProperty(
            CIMProperty(_PROPERTY_SYSTEMNAME,x));
    }
    else
    {
        CIMProperty p=instance.getProperty(sysNamePos);
        p.setValue(x);
    }
}

void IndicationService::_setSystemNameInHandlerFilter(
    CIMObjectPath& objPath,
    const String& sysname)
{
    Array<CIMKeyBinding> keys=objPath.getKeyBindings();
    Array<CIMKeyBinding> updatedKeys;

    updatedKeys.append(keys[0]);
    updatedKeys.append(keys[1]);
    updatedKeys.append(keys[2]);
    updatedKeys.append(CIMKeyBinding(
        _PROPERTY_SYSTEMNAME,
        sysname,
        CIMKeyBinding::STRING));
    objPath.setKeyBindings(updatedKeys);
    objPath.setHost(String::EMPTY);
}

void IndicationService::_setSystemNameInHandlerFilterReference(
    String& reference,
    const String& sysname)
{
    static const Char16 quote = 0x0022;

    reference.remove(reference.size()-1);

    Uint32 quotePos=reference.reverseFind(quote);

    reference.remove(quotePos+1);
    reference.append(sysname);
    reference.append(quote);

    static const Char16 slash = 0x002F;
    // remove hostname, don't need it
    if (reference[0] == slash && reference[1] == slash)
    {
        // namespace starts after next slash
        Uint32 ns = reference.find(2, slash);
        reference.remove(0,ns+1);
    }
}

void IndicationService::_setSubscriptionSystemName(
    CIMObjectPath& objPath,
    const String& sysname)
{
    Array<CIMKeyBinding> keys=objPath.getKeyBindings();

    String filterValue = keys[0].getValue();
    String handlerValue = keys[1].getValue();

    _setSystemNameInHandlerFilterReference(filterValue,sysname);
    _setSystemNameInHandlerFilterReference(handlerValue,sysname);

    Array<CIMKeyBinding> newKeys;

    newKeys.append(CIMKeyBinding(
        PEGASUS_PROPERTYNAME_FILTER,
        filterValue,
        CIMKeyBinding::REFERENCE));

    newKeys.append(CIMKeyBinding(
        PEGASUS_PROPERTYNAME_HANDLER,
        handlerValue,
        CIMKeyBinding::REFERENCE));

    objPath.setKeyBindings(newKeys);
}

void IndicationService::_setSystemName(
    CIMObjectPath& objPath,
    const String& sysname)
{

    // Need different handling for subscriptions
    if ((objPath.getClassName().equal(
             PEGASUS_CLASSNAME_INDSUBSCRIPTION)) ||
        (objPath.getClassName().equal(
             PEGASUS_CLASSNAME_FORMATTEDINDSUBSCRIPTION)))
    {
        _setSubscriptionSystemName(objPath,sysname);
    }
    else
    {
        // this is a Filter or Handler object path
        _setSystemNameInHandlerFilter(objPath,sysname);
    }
}

void IndicationService::_setSystemName(
    CIMInstance& instance,
    const String& sysname)
{

    PEG_METHOD_ENTER(TRC_INDICATION_SERVICE,
        "IndicationService::_setSystemName");

    CIMObjectPath newPath=instance.getPath();

    // Need different handling for subscriptions
    if ((instance.getClassName().equal(
             PEGASUS_CLASSNAME_INDSUBSCRIPTION)) ||
        (instance.getClassName().equal(
             PEGASUS_CLASSNAME_FORMATTEDINDSUBSCRIPTION)))
    {
        _setSubscriptionSystemName(newPath,sysname);
    }
    else
    {
        // this is a Filter or Handler instance
        _setOrAddSystemNameInHandlerFilter(instance,sysname);
        _setSystemNameInHandlerFilter(newPath,sysname);

    }
    instance.setPath(newPath);

    PEG_METHOD_EXIT();
}

void IndicationService::_initialize()
{
    PEG_METHOD_ENTER(TRC_INDICATION_SERVICE, "IndicationService::_initialize");

    //
    //  Find required services
    _providerManager = find_service_qid(PEGASUS_QUEUENAME_PROVIDERMANAGER_CPP);
    _handlerService = find_service_qid(PEGASUS_QUEUENAME_INDHANDLERMANAGER);
    _moduleController = find_service_qid(PEGASUS_QUEUENAME_CONTROLSERVICE);

    //
    //  Set arrays of supported property values
    //
    //  Note: Valid values are defined by the CIM Event Schema MOF
    //  Supported values are a subset of the valid values
    //  Some valid values, as defined in the MOF, are not currently supported
    //  by the Pegasus IndicationService
    //
    _supportedStates.append(STATE_ENABLED);
    _supportedStates.append(STATE_DISABLED);
    _supportedRepeatPolicies.append(_POLICY_UNKNOWN);
    _supportedRepeatPolicies.append(_POLICY_OTHER);
    _supportedRepeatPolicies.append(_POLICY_NONE);
    _supportedRepeatPolicies.append(_POLICY_SUPPRESS);
    _supportedRepeatPolicies.append(_POLICY_DELAY);
    _supportedErrorPolicies.append(_ERRORPOLICY_IGNORE);
    _supportedErrorPolicies.append(_ERRORPOLICY_DISABLE);
    _supportedErrorPolicies.append(_ERRORPOLICY_REMOVE);
    _supportedPersistenceTypes.append(PERSISTENCE_PERMANENT);
    _supportedPersistenceTypes.append(PERSISTENCE_TRANSIENT);
    _supportedSNMPVersion.append(SNMPV1_TRAP);
    _supportedSNMPVersion.append(SNMPV2C_TRAP);
    _supportedSNMPVersion.append(SNMPV3_TRAP);


    ConfigManager* configManager = ConfigManager::getInstance();

    if (ConfigManager::parseBooleanValue(
        configManager->getCurrentValue("enableIndicationService")))
    {
     _setEnabledState(_ENABLEDSTATE_ENABLED);
     _initializeActiveSubscriptionsFromRepository(0);
    }

    PEG_METHOD_EXIT();
}



#ifdef PEGASUS_ENABLE_DMTF_INDICATION_PROFILE_SUPPORT

String _getReturnCodeString(Uint32 code)
{
    // Method return codes
    switch(code)
    {
        case _RETURNCODE_TIMEOUT:
            return String("Cannot complete within Timeout Period");
        case _RETURNCODE_NOTSUPPORTED:
            return String("Not Supported");
        case _RETURNCODE_FAILED:
            return String("Failed");
        case _RETURNCODE_INVALIDPARAMETER:
            return String("Invalid Parameter");
    }
    // Never reach to unknown return code
    PEGASUS_UNREACHABLE(PEGASUS_ASSERT(false);)

    return String("Unknown");
}

void IndicationService::_sendIndicationServiceDisabled()
{
    PEG_METHOD_ENTER(TRC_INDICATION_SERVICE,
        "IndicationService::_sendIndicationServiceDisabled");

    if (_indicationServiceConfiguration->getEnabledState() ==
             _ENABLEDSTATE_ENABLED)
    {
        PEG_METHOD_EXIT();
        return;
    }

    CIMIndicationServiceDisabledRequestMessage * request =
        new CIMIndicationServiceDisabledRequestMessage(
            XmlWriter::getNextMessageId(),
            QueueIdStack(_providerManager, getQueueId()));

    CIMIndicationServiceDisabledRequestMessage *requestCopy =
        new CIMIndicationServiceDisabledRequestMessage(*request);

    AsyncLegacyOperationStart * asyncRequest =
        new AsyncLegacyOperationStart(
            0,
            _providerManager,
            requestCopy);
    AutoPtr<AsyncReply> asyncReply(SendWait(asyncRequest));

    delete asyncRequest;


    // Now send to all indication internal control providers

    for (ControlProvIndRegTable::Iterator j =
        _controlProvIndRegTable.start (); j; j++)
    {
        ControlProvIndReg reg = j.value();
        String controlProviderName;
        reg.provider.getProperty(reg.provider.findProperty(
            PEGASUS_PROPERTYNAME_NAME)).getValue().get(controlProviderName);

        requestCopy = new CIMIndicationServiceDisabledRequestMessage(*request);

        AsyncModuleOperationStart * asyncRequest =
            new AsyncModuleOperationStart(
                0,
                _moduleController,
                controlProviderName,
                requestCopy);

        AutoPtr<AsyncReply> asyncReply(SendWait(asyncRequest));
        delete asyncRequest;
    }

    delete request;

    PEG_METHOD_EXIT();
}

void IndicationService::_handleInvokeMethodRequest(Message *message)
{
    Uint32 timeoutSeconds = 0;

    CIMInvokeMethodRequestMessage *request =
        dynamic_cast<CIMInvokeMethodRequestMessage*>(message);

    PEGASUS_ASSERT(request);

    CIMInvokeMethodResponseMessage *response =
        static_cast<CIMInvokeMethodResponseMessage*>(request->buildResponse());

    // Get userName and only privileged user can execute this operation
    String userName = ((IdentityContainer)request->operationContext.get(
        IdentityContainer::NAME)).getUserName();

#ifndef PEGASUS_OS_ZOS
    if (userName.size() && !System::isPrivilegedUser(userName))
    {
        throw PEGASUS_CIM_EXCEPTION_L(CIM_ERR_ACCESS_DENIED,
            MessageLoaderParms(
                "IndicationService.IndicationService."
                    "_MSG_NON_PRIVILEGED_ACCESS_DISABLED",
                "User ($0) is not authorized to perform this operation.",
                userName));
    }
#endif

    CIMException cimException;

    CIMNamespaceName nameSpace = request->nameSpace;
    CIMName className = request->instanceName.getClassName().getString();

    Uint32 retCode = _RETURNCODE_COMPLETEDWITHNOERROR;
    Uint16 requestedState = _ENABLEDSTATE_UNKNOWN;

    if(!nameSpace.equal(PEGASUS_NAMESPACENAME_INTEROP))
    {
        cimException = PEGASUS_CIM_EXCEPTION(CIM_ERR_NOT_SUPPORTED,
            nameSpace.getString());
    }
    else if(!className.equal(PEGASUS_CLASSNAME_CIM_INDICATIONSERVICE))
    {
        cimException = PEGASUS_CIM_EXCEPTION(CIM_ERR_NOT_SUPPORTED,
            className.getString());
    }
    else if (!request->methodName.equal(_METHOD_REQUESTSTATECHANGE))
    {
        cimException = PEGASUS_CIM_EXCEPTION(CIM_ERR_METHOD_NOT_FOUND,
            String::EMPTY);
    }
    else
    {
        CIMValue cimValue;

        for (Uint32 i = 0, n = request->inParameters.size(); i < n ; ++i)
        {
            CIMName name = request->inParameters[i].getParameterName();
            if (name.equal(_PARAM_REQUESTEDSTATE))
            {
                CIMValue cimValue = request->inParameters[i].getValue();
                cimValue.get(requestedState);
            }
            else if ((name.equal(_PARAM_TIMEOUTPERIOD)))
            {
                CIMDateTime timeoutInterval;
                CIMValue cimValue = request->inParameters[i].getValue();
                cimValue.get(timeoutInterval);
                if (!timeoutInterval.isInterval())
                {
                    retCode = _RETURNCODE_INVALIDPARAMETER;
                    break;
                }
                // Get timeout in seconds
                timeoutSeconds =
                    timeoutInterval.toMicroSeconds() / 1000000;
            }
            else
            {
                retCode = _RETURNCODE_INVALIDPARAMETER;
                break;
            }
        }
        if (requestedState == _ENABLEDSTATE_UNKNOWN)
        {
            cimException = PEGASUS_CIM_EXCEPTION(
                CIM_ERR_INVALID_PARAMETER,
                _PARAM_REQUESTEDSTATE.getString());
            retCode = _RETURNCODE_INVALIDPARAMETER;
        }
    }

    if (cimException.getCode() == CIM_ERR_SUCCESS &&
        retCode == _RETURNCODE_COMPLETEDWITHNOERROR)
    {
        if (requestedState == _ENABLEDSTATE_ENABLED)
        {
           retCode = _enableIndicationService(timeoutSeconds);
        }
        else if (requestedState == _ENABLEDSTATE_DISABLED)
        {
            retCode = _disableIndicationService(
                timeoutSeconds,
                cimException);
        }
        else
        {
            // We don't support any other state changes now.
            retCode = _RETURNCODE_NOTSUPPORTED;
        }
    }

    response->cimException = cimException;
    response->retValue = CIMValue(retCode);
    _enqueueResponse(request, response);
}


Uint32 IndicationService::_enableIndicationService(Uint32 timeoutSeconds)
{
    PEG_METHOD_ENTER(TRC_INDICATION_SERVICE,
        "IndicationService::_enableIndicationService");

    Uint32 retCode = _RETURNCODE_COMPLETEDWITHNOERROR;

    AutoMutex mtx(_mutex);

    // Check if the service is already enabled.
    if (_indicationServiceConfiguration->getEnabledState() ==
             _ENABLEDSTATE_ENABLED)
    {
        // Check if the service is in degraded state.
        if (_indicationServiceConfiguration->getHealthState() ==
                 _HEALTHSTATE_DEGRADEDWARNING)
        {
            struct timeval startTime;
            Time::gettimeofday(&startTime);

            // Wait if there are any pending requests.
            if (!_waitForAsyncRequestsComplete(&startTime, timeoutSeconds))
            {
                Logger::put(
                    Logger::STANDARD_LOG,
                    System::CIMSERVER,
                    Logger::WARNING,
                    "Failed to recover from degraded state within timeout "
                        "period of $0 seconds. There are $1 async"
                            " requests pending.",
                    timeoutSeconds,
                    _asyncRequestsPending.get());

                retCode = _RETURNCODE_TIMEOUT;
            }
            else
            {
                // No async requests pending.
                _indicationServiceConfiguration->setHealthState(
                     _HEALTHSTATE_OK);
            }
        }
        PEG_METHOD_EXIT();
        return retCode;
    }

    _indicationServiceConfiguration->setEnabledState(_ENABLEDSTATE_STARTING);

    String exceptionMsg;

    try
    {
        if (_initializeActiveSubscriptionsFromRepository(
            timeoutSeconds))
        {
            _indicationServiceConfiguration->setHealthState(_HEALTHSTATE_OK);
        }
        else
        {
            _indicationServiceConfiguration->setHealthState(
                 _HEALTHSTATE_DEGRADEDWARNING);
            retCode = _RETURNCODE_TIMEOUT;
        }
    }
    catch (const Exception &e)
    {
        exceptionMsg = e.getMessage();
    }
    catch (...)
    {
        exceptionMsg = "Unknown error";
    }

    _indicationServiceConfiguration->setEnabledState(_ENABLEDSTATE_ENABLED);
    sendSubscriptionInitComplete();

    if (exceptionMsg.size())
    {
        PEG_TRACE((TRC_INDICATION_SERVICE, Tracer::LEVEL1,
            "Exception while enabling the indication Service : %s",
            (const char*)exceptionMsg.getCString()));

        _indicationServiceConfiguration->setHealthState(
             _HEALTHSTATE_DEGRADEDWARNING);
    }

    PEG_METHOD_EXIT();
    return retCode;
}

Uint32 IndicationService::_disableIndicationService(Uint32 timeoutSeconds,
    CIMException &cimException)
{
    PEG_METHOD_ENTER(TRC_INDICATION_SERVICE,
        "IndicationService::_disableIndicationService");

    Uint32 retCode = _RETURNCODE_COMPLETEDWITHNOERROR;

    AutoMutex mtx(_mutex);

    // Check if the service is already disabled.
    if (_indicationServiceConfiguration->getEnabledState() ==
             _ENABLEDSTATE_DISABLED)
    {
        PEG_METHOD_EXIT();
        return retCode;
    }

    _indicationServiceConfiguration->setEnabledState(
         _ENABLEDSTATE_SHUTTINGDOWN);

    // Wait for threads running other than indication threads.
    while (_threads.get() - _processIndicationThreads.get() > 1)
    {
        Threads::sleep(100);
    }

    String exceptionMsg;

    try
    {
        if (_deleteActiveSubscriptions(timeoutSeconds))
        {
            _sendIndicationServiceDisabled();
            _indicationServiceConfiguration->setEnabledState(
                  _ENABLEDSTATE_DISABLED);
            _indicationServiceConfiguration->setHealthState(_HEALTHSTATE_OK);
        }
        else
        {
            _indicationServiceConfiguration->setEnabledState(
                 _ENABLEDSTATE_ENABLED);
            retCode = _RETURNCODE_TIMEOUT;
            _indicationServiceConfiguration->setHealthState(
                 _HEALTHSTATE_DEGRADEDWARNING);
            cimException = PEGASUS_CIM_EXCEPTION_L(CIM_ERR_FAILED,
                MessageLoaderParms(
                    _MSG_STATE_CHANGE_FAILED_KEY,
                    _MSG_STATE_CHANGE_FAILED,
                    _getReturnCodeString(_RETURNCODE_TIMEOUT),
                    _getEnabledStateString(
                         _indicationServiceConfiguration->getEnabledState()),
                    _getHealthStateString(
                         _indicationServiceConfiguration->getHealthState())));
        }
    }
    catch (const Exception &e)
    {
        exceptionMsg = e.getMessage();
    }
    catch (...)
    {
        exceptionMsg = "Unknown Error";
    }

    if (exceptionMsg.size())
    {
        PEG_TRACE((TRC_INDICATION_SERVICE, Tracer::LEVEL1,
            "Exception while disabling the indication Service : %s",
            (const char*)exceptionMsg.getCString()));

        _indicationServiceConfiguration->setEnabledState(
             _ENABLEDSTATE_ENABLED);
        retCode = _RETURNCODE_FAILED;
        _indicationServiceConfiguration->setHealthState(
             _HEALTHSTATE_DEGRADEDWARNING);
        cimException = PEGASUS_CIM_EXCEPTION_L(CIM_ERR_FAILED,
            MessageLoaderParms(
                _MSG_STATE_CHANGE_FAILED_KEY,
                _MSG_STATE_CHANGE_FAILED,
                exceptionMsg,
                _getEnabledStateString(
                     _indicationServiceConfiguration->getEnabledState()),
                _getHealthStateString(
                     _indicationServiceConfiguration->getHealthState())));
    }
    PEG_METHOD_EXIT();

    return retCode;
}

Boolean IndicationService::_deleteActiveSubscriptions(Uint32 timeoutSeconds)
{
    PEG_METHOD_ENTER(TRC_INDICATION_SERVICE,
        "IndicationService::_deleteActiveSubscriptions");

    struct timeval startTime;
    Time::gettimeofday(&startTime);
    Boolean completed = true;

    // Check if there are existing pending async requests
    if (!_waitForAsyncRequestsComplete(&startTime, timeoutSeconds))
    {
        Logger::put(
            Logger::STANDARD_LOG,
            System::CIMSERVER,
            Logger::WARNING,
            "Failed to disable Indication service within timeout "
                "period of $0 seconds. There are $1 existing async "
                    "requests pending.",
            timeoutSeconds,
            _asyncRequestsPending.get());

        PEG_METHOD_EXIT();
        return false;
    }

    Array <ActiveSubscriptionsTableEntry> subscriptionsEntries;
    subscriptionsEntries =
        _subscriptionTable->getAllActiveSubscriptionEntries();

    CIMPropertyList requiredProperties;
    String condition;
    String query;
    String queryLanguage;

    for (Uint32 i=0; i < subscriptionsEntries.size(); i++)
    {
        CIMInstance instance = subscriptionsEntries[i].subscription;
        String creator = instance.getProperty (instance.findProperty
            (PEGASUS_PROPERTYNAME_INDSUB_CREATOR)).getValue ().toString ();

        AcceptLanguageList acceptLangs;
        Uint32 propIndex = instance.findProperty(
            PEGASUS_PROPERTYNAME_INDSUB_ACCEPTLANGS);
        if (propIndex != PEG_NOT_FOUND)
        {
            String acceptLangsString;
            instance.getProperty(propIndex).getValue().get(acceptLangsString);
            if (acceptLangsString.size())
            {
                acceptLangs = LanguageParser::parseAcceptLanguageHeader(
                    acceptLangsString);
            }
        }
        ContentLanguageList contentLangs;
        propIndex = instance.findProperty
            (PEGASUS_PROPERTYNAME_INDSUB_CONTENTLANGS);
        if (propIndex != PEG_NOT_FOUND)
        {
            String contentLangsString;
            instance.getProperty(propIndex).getValue().get(
                contentLangsString);
            if (contentLangsString.size())
            {
                contentLangs = LanguageParser::parseContentLanguageHeader(
                    contentLangsString);
            }
        }
        Array<NamespaceClassList> indicationSubclasses;

        _getCreateParams (
            instance,
            indicationSubclasses,
            requiredProperties,
            condition,
            query,
            queryLanguage);

        _sendAsyncDeleteRequests(
            subscriptionsEntries[i].providers,
            instance,
            acceptLangs,
            contentLangs,
            0,  // no request
            indicationSubclasses,
            creator);

    }

    if (!_waitForAsyncRequestsComplete(&startTime, timeoutSeconds))
    {
        Logger::put(
            Logger::STANDARD_LOG,
            System::CIMSERVER,
            Logger::WARNING,
            "Failed to disable Indication service within timeout "
                "period of $0 seconds. There are $1 async requests pending.",
            timeoutSeconds,
            _asyncRequestsPending.get());

        completed = false;
    }
    else
    {
#ifdef PEGASUS_ENABLE_INDICATION_COUNT
        _providerIndicationCountTable.clear();
#endif

        _subscriptionTable->clear();
    }

    PEG_METHOD_EXIT();

    return completed;
}


Boolean IndicationService::_waitForAsyncRequestsComplete(
    struct timeval* startTime,
    Uint32 timeoutSeconds)
{
    PEG_METHOD_ENTER(TRC_INDICATION_SERVICE,
        "IndicationService::_waitForAsyncRequestsComplete");

    struct timeval timeNow;
    Boolean requestsPending = false;
    while (_asyncRequestsPending.get() > 0)
    {
        if (timeoutSeconds)
        {
            Time::gettimeofday(&timeNow);
            if ((Uint32)(timeNow.tv_sec - startTime->tv_sec) > timeoutSeconds)
            {
                requestsPending = true;
                break;
            }
        }
        Threads::sleep(100);
    }
    PEG_METHOD_EXIT();

    return !requestsPending;
}

void IndicationService::_sendSubscriptionNotActiveMessagetoHandlerService(
        const CIMObjectPath &subscriptionName)
{
    PEG_METHOD_ENTER(TRC_INDICATION_SERVICE,
        "IndicationService::_sendSubscriptionNotActiveMessagetoHandlerService");

       CIMRequestMessage * notifyRequest =
        new CIMNotifySubscriptionNotActiveRequestMessage (
            XmlWriter::getNextMessageId (),
            subscriptionName,
            QueueIdStack(_handlerService));

    AsyncLegacyOperationStart *req =
        new AsyncLegacyOperationStart(
        0,
        _handlerService,
        notifyRequest);

    AsyncReply *reply = SendWait(req);
    delete req;
    delete reply;

    PEG_METHOD_EXIT();
}

void IndicationService::_sendListenerNotActiveMessagetoHandlerService(
        const CIMObjectPath &handlerName)
{
    PEG_METHOD_ENTER(TRC_INDICATION_SERVICE,
        "IndicationService::_sendListenerNotActiveMessagetoHandlerService");

    // Send notify request only to CIMXML handlers
    {
        CIMRequestMessage * notifyRequest =
            new CIMNotifyListenerNotActiveRequestMessage (
                XmlWriter::getNextMessageId (),
                handlerName,
                QueueIdStack(_handlerService));

         AsyncLegacyOperationStart *req =
             new AsyncLegacyOperationStart(
             0,
             _handlerService,
             notifyRequest);

         AsyncReply *reply = SendWait(req);
         delete req;
         delete reply;
    }

    PEG_METHOD_EXIT();
}

#endif

void IndicationService::_updateAcceptedSubscription(
    CIMInstance &subscription,
    const Array<ProviderClassList> &acceptedProviders,
    const Array<NamespaceClassList> &indicationSubclasses)
{
    PEG_METHOD_ENTER(TRC_INDICATION_SERVICE,
        "IndicationService::_updateAcceptedSubscription");

    if (acceptedProviders.size() == 0)
    {
        PEG_TRACE((TRC_INDICATION_SERVICE, Tracer::LEVEL2,
            "No providers accepted subscription on initialization: %s",
            (const char *)
                subscription.getPath().toString().getCString()));

        //
        //  No providers accepted the subscription
        //  Implement the subscription's On Fatal Error Policy
        //  If subscription is not disabled or removed, send alert and
        //  Insert entries into the subscription hash tables
        //
        if (!_subscriptionRepository->reconcileFatalError(
            subscription))
        {
            //
            //  Insert entries into the subscription hash tables
            //
            _subscriptionTable->insertSubscription(
                subscription,
                acceptedProviders,
                indicationSubclasses);

#if 0
            //
            //  Send alert
            //
            //
            //  Send NoProviderAlertIndication to handler instances
            //  ATTN: NoProviderAlertIndication must be defined
            //
            Array<CIMInstance> subscriptions;
            subscriptions.append(activeSubscriptions[i]);
            CIMInstance indicationInstance = _createAlertInstance(
                _CLASS_NO_PROVIDER_ALERT, subscriptions);

            PEG_TRACE((TRC_INDICATION_SERVICE, Tracer::LEVEL4,
                "Sending NoProvider Alert for %u subscriptions",
                 subscriptions.size()));
            _sendAlerts(subscriptions, indicationInstance);
#endif

            //
            //  Get Subscription Filter Name and Handler Name
            //
            String logString = _getSubscriptionLogString(
                subscription);

            //
            //  Log a message for the subscription
            //
            Logger::put_l(Logger::STANDARD_LOG, System::CIMSERVER,
                Logger::WARNING,
                MessageLoaderParms(
                    _MSG_NO_PROVIDER_KEY,
                    _MSG_NO_PROVIDER,
                    logString,
                    subscription.getPath().getNameSpace().getString()));
        }
    }
    else
    {
        //
        //  At least one provider accepted the subscription
        //  Insert entries into the subscription hash tables
        //
        _subscriptionTable->insertSubscription(
            subscription,
            acceptedProviders,
            indicationSubclasses);
    }

    PEG_METHOD_EXIT();
}

Boolean IndicationService::_initializeActiveSubscriptionsFromRepository(
    Uint32 timeoutSeconds)
{
    PEG_METHOD_ENTER(TRC_INDICATION_SERVICE,
        "IndicationService::_initializeActiveSubscriptionsFromRepository");

    struct timeval startTime;
    Time::gettimeofday(&startTime);
    Boolean completed = true;

#ifdef PEGASUS_INDICATION_PERFINST
    Stopwatch stopWatch;

    stopWatch.start();
#endif

    //  Create Subscription Repository
    _subscriptionRepository.reset(new SubscriptionRepository(_cimRepository));

#ifdef PEGASUS_ENABLE_DMTF_INDICATION_PROFILE_SUPPORT
    _asyncRequestsPending = 0;
    _processIndicationThreads = 0;
#endif

   //  Create Subscription Table
   _subscriptionTable.reset(
       new SubscriptionTable(_subscriptionRepository.get()));

#ifdef PEGASUS_ENABLE_INDICATION_COUNT
    _providerIndicationCountTable.clear();
#endif

    Array<CIMInstance> activeSubscriptions;
    Array<CIMInstance> noProviderSubscriptions;
    Boolean invalidInstance = false;
    //
    //  Get existing active subscriptions from each namespace in the repository
    //
    invalidInstance = _subscriptionRepository->getActiveSubscriptions(
        activeSubscriptions);
    noProviderSubscriptions.clear();

    PEG_TRACE((TRC_INDICATION_SERVICE, Tracer::LEVEL4,
        "%u active subscription(s) found on initialization",
        activeSubscriptions.size()));

    String condition;
    String query;
    String queryLanguage;
    CIMPropertyList propertyList;
    Array<ProviderClassList> indicationProviders;

    for (Uint32 i = 0; i < activeSubscriptions.size(); i++)
    {
        //
        //  Check for expired subscription
        //
        try
        {
            if (_isExpired(activeSubscriptions[i]))
            {
                CIMObjectPath path = activeSubscriptions[i].getPath();

                PEG_TRACE((TRC_INDICATION_SERVICE, Tracer::LEVEL4,
                    "Deleting expired subscription on initialization: %s",
                    (const char *) path.toString().getCString()));

                _deleteExpiredSubscription(path);

#ifdef PEGASUS_ENABLE_DMTF_INDICATION_PROFILE_SUPPORT
                _sendSubscriptionNotActiveMessagetoHandlerService(path);
#endif
                // If subscription is expired delete the subscription
                // and continue on to the next one.
                continue;
            }
        }
        catch (DateTimeOutOfRangeException& e)
        {
            //
            //  This instance from the repository is invalid
            //  Log a message and skip it
            //
            Logger::put_l(Logger::STANDARD_LOG, System::CIMSERVER,
                Logger::WARNING,
                MessageLoaderParms(
                    "IndicationService.IndicationService."
                        "INVALID_SUBSCRIPTION_INSTANCE_IGNORED",
                    "An invalid Subscription instance was ignored: $0.",
                    e.getMessage()));
            continue;
        }

        Array<NamespaceClassList> indicationSubclasses;
        _getCreateParams(
            activeSubscriptions[i],
            indicationSubclasses,
            indicationProviders,
            propertyList,
            condition,
            query,
            queryLanguage);

        if (indicationProviders.size() == 0)
        {
            PEG_TRACE((TRC_INDICATION_SERVICE, Tracer::LEVEL2,
                "No providers found for subscription on initialization: %s",
                (const char *)
                    activeSubscriptions[i].getPath().toString().getCString()));

            //
            //  There are no providers that can support this subscription
            //  Implement the subscription's On Fatal Error Policy
            //  If subscription is not disabled or removed,
            //  Append this subscription to no provider list and
            //  Insert entries into the subscription hash tables
            //
            if (!_subscriptionRepository->reconcileFatalError(
                    activeSubscriptions[i]))
            {
                noProviderSubscriptions.append(activeSubscriptions[i]);

                _subscriptionTable->insertSubscription(
                    activeSubscriptions[i],
                    indicationProviders,
                    indicationSubclasses);
            }
            continue;
        }

        //
        //  Send Create request message to each provider
        //  NOTE: These Create requests are not associated with a user request,
        //  so there is no associated authType or userName
        //  The Creator from the subscription instance is used for userName,
        //  and authType is not set
        //
        CIMInstance instance = activeSubscriptions[i];
        String creator;
        if (!_getCreator(instance, creator))
        {
            //
            //  This instance from the repository is corrupted
            //  Skip it
            //
            invalidInstance = true;
            continue;
        }

        // Get the language tags that were saved with the subscription instance
        AcceptLanguageList acceptLangs;
        Uint32 propIndex = instance.findProperty(
            PEGASUS_PROPERTYNAME_INDSUB_ACCEPTLANGS);
        if (propIndex != PEG_NOT_FOUND)
        {
            String acceptLangsString;
            instance.getProperty(propIndex).getValue().get(acceptLangsString);
            if (acceptLangsString.size())
            {
                acceptLangs = LanguageParser::parseAcceptLanguageHeader(
                    acceptLangsString);
            }
        }
        ContentLanguageList contentLangs;
        propIndex = instance.findProperty(
            PEGASUS_PROPERTYNAME_INDSUB_CONTENTLANGS);
        if (propIndex != PEG_NOT_FOUND)
        {
            String contentLangsString;
            instance.getProperty(propIndex).getValue().get(contentLangsString);
            if (contentLangsString.size())
            {
                contentLangs = LanguageParser::parseContentLanguageHeader(
                    contentLangsString);
            }
        }

        // If Indication profile support is enabled indication service can be
        // enabled dynamically. Send create subscription requests using
        // SendAsync() to honor the timeout.
#ifdef PEGASUS_ENABLE_DMTF_INDICATION_PROFILE_SUPPORT
        if (timeoutSeconds > 0) // if timeout is specified
        {
            _sendAsyncCreateRequests(
                indicationProviders,
                propertyList,
                condition,
                query,
                queryLanguage,
                activeSubscriptions[i],
                acceptLangs,
                contentLangs,
                0, // original request is 0
                indicationSubclasses,
                creator);
        }
        else
#endif
        //
        //  Send Create request message to each provider using SendWait() if
        //  timeout is not specified.
        //  Note: SendWait is used instead of SendAsync.  Initialization must
        //  deal with multiple subscriptions, each with multiple providers.
        //  Using SendWait eliminates the need for a callback and the necessity
        //  to handle multiple levels of aggregation, which would add
        //  significant complexity.  Since initialization cannot complete
        //  anyway until responses have been received for all subscriptions,
        //  from all the providers, use of SendWait should not cause a
        //  significant performance issue.
        //
        {
            Array<ProviderClassList> acceptedProviders;
            acceptedProviders = _sendWaitCreateRequests(
                indicationProviders,
                propertyList,
                condition,
                query,
                queryLanguage,
                activeSubscriptions[i],
                acceptLangs,
                contentLangs,
                creator);

            _updateAcceptedSubscription(
                activeSubscriptions[i],
                acceptedProviders,
                indicationSubclasses);
        }
    }  // for each active subscription


#ifdef PEGASUS_ENABLE_DMTF_INDICATION_PROFILE_SUPPORT
        if (timeoutSeconds > 0)
        {
            if (!_waitForAsyncRequestsComplete(&startTime, timeoutSeconds))
            {
                Logger::put(
                    Logger::STANDARD_LOG,
                    System::CIMSERVER,
                    Logger::WARNING,
                    "Failed to enable Indication service within timeout "
                        "period of $0 seconds. There are $1 async"
                            " requests pending.",
                    timeoutSeconds,
                    _asyncRequestsPending.get());
                completed = false;
            }
        }
#endif

    //
    //  Log a message if any invalid instances were found
    //
    if (invalidInstance)
    {
        Logger::put_l(Logger::STANDARD_LOG, System::CIMSERVER, Logger::WARNING,
            MessageLoaderParms(
                _MSG_INVALID_INSTANCES_KEY, _MSG_INVALID_INSTANCES));
    }
    //
    //  Log a message for any subscription for which there is no longer any
    //  provider
    //
    if (noProviderSubscriptions.size() > 0)
    {
#if 0
        //
        //  Send NoProviderAlertIndication to handler instances
        //  ATTN: NoProviderAlertIndication must be defined
        //
        CIMInstance indicationInstance = _createAlertInstance(
            _CLASS_NO_PROVIDER_ALERT, noProviderSubscriptions);

        PEG_TRACE((TRC_INDICATION_SERVICE, Tracer::LEVEL4,
            "Sending NoProvider Alert for %u subscriptions",
            noProviderSubscriptions.size()));
        _sendAlerts(noProviderSubscriptions, indicationInstance);
#endif
        //
        //  Log a message for each subscription
        //
        for (Uint32 i = 0; i < noProviderSubscriptions.size(); i++)
        {
            //
            //  Get Subscription Filter Name and Handler Name
            //
            String logString =
                _getSubscriptionLogString(noProviderSubscriptions[i]);

            Logger::put_l(
                Logger::STANDARD_LOG, System::CIMSERVER, Logger::WARNING,
                MessageLoaderParms(
                    _MSG_NO_PROVIDER_KEY,
                    _MSG_NO_PROVIDER,
                    logString,
                    noProviderSubscriptions[i].getPath().getNameSpace().
                        getString()));
        }
    }

#ifdef PEGASUS_INDICATION_PERFINST
    stopWatch.stop();

    PEG_TRACE((TRC_INDICATION_SERVICE, Tracer::LEVEL4,
        "%s: %.3f seconds", "Initialize", stopWatch.getElapsed()));
#endif

    PEG_METHOD_EXIT();

    return completed;
}

void IndicationService::_terminate()
{
    PEG_METHOD_ENTER(TRC_INDICATION_SERVICE, "IndicationService::_terminate");

    Array<CIMInstance> activeSubscriptions;
    CIMInstance indicationInstance;

    //
    //  A message is already logged that CIM Server is shutting down --
    //  no need to log a message
    //
#if 0
    //
    //  Get existing active subscriptions from hash table
    //
    activeSubscriptions = _getActiveSubscriptions();

    if (activeSubscriptions.size() > 0)
    {
        //
        //  Create CimomShutdownAlertIndication instance
        //  ATTN: CimomShutdownAlertIndication must be defined
        //
        indicationInstance = _createAlertInstance(
            _CLASS_CIMOM_SHUTDOWN_ALERT, activeSubscriptions);

        //
        //  Send CimomShutdownAlertIndication to each unique handler instance
        //
        PEG_TRACE((TRC_INDICATION_SERVICE, Tracer::LEVEL4,
            "Sending CIMServerShutdown Alert for %u subscriptions",
            activeSubscriptions.size()));
        _sendAlerts(activeSubscriptions, indicationInstance);
    }
#endif

    //
    //  Remove entries from the SubscriptionTable's Active Subscriptions and
    //  Subscription Classes tables
    //
    //  NOTE: The table entries are removed when the SubscriptionTable
    //  destructor is called by the IndicationService destructor.  However,
    //  currently the IndicationService destructor is never called, so the
    //  IndicationService must call the SubscriptionTable clear() function to
    //  remove the table entries.
    _subscriptionTable->clear();

    PEG_METHOD_EXIT();
}

void IndicationService::_checkNonprivilegedAuthorization(
    const String& userName)
{
#ifndef PEGASUS_OS_ZOS
    PEG_METHOD_ENTER(TRC_INDICATION_SERVICE,
        "IndicationService::_checkNonprivilegedAuthorization");

    if (!_enableSubscriptionsForNonprivilegedUsers)
    {
        PEG_TRACE((TRC_INDICATION_SERVICE, Tracer::LEVEL4,
            "_checkNonprivilegedAuthorization - checking whether user %s is "
                "privileged",
            (const char*) userName.getCString()));
        if (!System::isPrivilegedUser(userName))
        {
            MessageLoaderParms parms(
                "IndicationService.IndicationService."
                    "_MSG_NON_PRIVILEGED_ACCESS_DISABLED",
                "User ($0) is not authorized to perform this operation.",
                userName);
            PEG_METHOD_EXIT();
            throw PEGASUS_CIM_EXCEPTION_L(CIM_ERR_ACCESS_DENIED, parms);
        }
    }

    PEG_METHOD_EXIT();
#endif
}

void IndicationService::_deliverWaitingIndications()
{
    // Deliver indications if any waiting for pending Create Subscription
    // requests.
    Message *message;
    while(!_subscriptionRepository->
         getUncommittedCreateSubscriptionRequests() &&
            (message = _deliveryWaitIndications.remove_front()))
    {
        handleEnqueue(message);
    }
}

void IndicationService::_beginCreateSubscription(const CIMObjectPath &objPath)
{
    _subscriptionRepository->beginCreateSubscription(objPath);
}

void IndicationService::_cancelCreateSubscription(const CIMObjectPath &objPath)
{
    _subscriptionRepository->cancelCreateSubscription(objPath);
    _deliverWaitingIndications();
}

void IndicationService::_commitCreateSubscription(const CIMObjectPath &objPath)
{
    _subscriptionRepository->commitCreateSubscription(objPath);
    _deliverWaitingIndications();
}

void IndicationService::_handleCreateInstanceRequest(const Message * message)
{
    PEG_METHOD_ENTER(TRC_INDICATION_SERVICE,
        "IndicationService::_handleCreateInstanceRequest");

    CIMCreateInstanceRequestMessage* request =
        (CIMCreateInstanceRequestMessage*) message;

    Boolean responseSent = false;

    CIMObjectPath instanceRef;
    CIMObjectPath subscriptionPath;

    CIMInstance instance = request->newInstance.clone();

    String userName = ((IdentityContainer)request->operationContext.get(
        IdentityContainer::NAME)).getUserName();
    _checkNonprivilegedAuthorization(userName);

    AcceptLanguageList acceptLangs =
        ((AcceptLanguageListContainer)request->operationContext.get(
            AcceptLanguageListContainer::NAME)).getLanguages();
    ContentLanguageList contentLangs =
        ((ContentLanguageListContainer)request->operationContext.get(
            ContentLanguageListContainer::NAME)).getLanguages();

    if (_canCreate(instance, request->nameSpace))
    {
        //
        //  If the instance is of the PEGASUS_CLASSNAME_INDSUBSCRIPTION
        //  class or the PEGASUS_CLASSNAME_FORMATTEDINDSUBSCRIPTION
        //  class and subscription state is enabled, determine if any
        //  providers can serve the subscription
        //
        Uint16 subscriptionState;
        String condition;
        String query;
        String queryLanguage;
        CIMPropertyList requiredProperties;
        Array<NamespaceClassList> indicationSubclasses;
        Array<ProviderClassList> indicationProviders;

        if ((instance.getClassName().equal(
                 PEGASUS_CLASSNAME_INDSUBSCRIPTION)) ||
            (instance.getClassName().equal(
                 PEGASUS_CLASSNAME_FORMATTEDINDSUBSCRIPTION)))
        {
            _beginCreateSubscription(instance.getPath());

            try
            {
                subscriptionPath = instance.getPath();
                //
                //  Get subscription state
                //
                //  NOTE: _canCreate has already validated the
                //  SubscriptionState property in the instance; if missing, it
                //  was added with the default value; if null, it was set to
                //  the default value; if invalid, an exception was thrown
                //
                CIMValue subscriptionStateValue;
                subscriptionStateValue = instance.getProperty(
                    instance.findProperty(
                        PEGASUS_PROPERTYNAME_SUBSCRIPTION_STATE)).getValue();
                subscriptionStateValue.get(subscriptionState);

                if ((subscriptionState == STATE_ENABLED) ||
                    (subscriptionState == STATE_ENABLEDDEGRADED))
                {
                    _getCreateParams(
                        instance,
                        indicationSubclasses,
                        indicationProviders,
                        requiredProperties,
                        condition,
                        query,
                        queryLanguage);

                    if (indicationProviders.size() == 0)
                    {
                        //
                        //  There are no providers that can support this
                        //  subscription
                        //

                        throw PEGASUS_CIM_EXCEPTION_L(CIM_ERR_NOT_SUPPORTED,
                            MessageLoaderParms(_MSG_NO_PROVIDERS_KEY,
                                _MSG_NO_PROVIDERS));
                    }

                    //
                    //  Send Create request message to each provider
                    //
                    _sendAsyncCreateRequests(
                        indicationProviders,
                        requiredProperties,
                        condition,
                        query,
                        queryLanguage,
                        instance,
                        acceptLangs,
                        contentLangs,
                        request,
                        indicationSubclasses,
                        userName,
                        request->authType);

                    //
                    //  Response is sent from _handleCreateResponseAggregation
                    //
                    responseSent = true;
                }
                else
                {
                    //
                    //  Create instance for disabled subscription
                    //
                    instanceRef = _subscriptionRepository->createInstance(
                        instance, request->nameSpace, userName,
                        acceptLangs, contentLangs, false);
                    _commitCreateSubscription(subscriptionPath);

                    // put correct SystemName in place
                    _setSubscriptionSystemName(
                        instanceRef,
                        System::getFullyQualifiedHostName());
                }
            }
            catch (...)
            {
                _cancelCreateSubscription(subscriptionPath);
                throw;
            }
        }
        else
        {
            //
            //  Create instance for filter or handler
            //
            instanceRef = _subscriptionRepository->createInstance(
                instance, request->nameSpace, userName,
                acceptLangs, contentLangs, false);

            // put correct SystemName in place
            _setSystemNameInHandlerFilter(
                instanceRef,
                System::getFullyQualifiedHostName());

        }
    }

    //
    //  Send response, if not sent from callback
    //  (for example, if there are no indication providers that can support a
    //  subscription)
    //
    if (!responseSent)
    {
// l10n - no Content-Language in response
        CIMCreateInstanceResponseMessage* response =
            dynamic_cast<CIMCreateInstanceResponseMessage*>(
                request->buildResponse());
        PEGASUS_ASSERT(response != 0);
        response->instanceName = instanceRef;
        _enqueueResponse(request, response);
    }

    PEG_METHOD_EXIT();
}

void IndicationService::_handleGetInstanceRequest(const Message* message)
{
    PEG_METHOD_ENTER(TRC_INDICATION_SERVICE,
        "IndicationService::_handleGetInstanceRequest");

    CIMGetInstanceRequestMessage* request =
        (CIMGetInstanceRequestMessage*) message;

    CIMInstance instance;
    String contentLangsString;

    String userName = ((IdentityContainer)request->operationContext.
        get(IdentityContainer::NAME)).getUserName();

#ifdef PEGASUS_ENABLE_DMTF_INDICATION_PROFILE_SUPPORT
    if (request->className.equal(PEGASUS_CLASSNAME_CIM_INDICATIONSERVICE)||
        request->className.equal(
            PEGASUS_CLASSNAME_CIM_INDICATIONSERVICECAPABILITIES))
    {
        instance = _indicationServiceConfiguration->getInstance(
            request->nameSpace,
            request->instanceName,
            request->includeQualifiers,
            request->includeClassOrigin,
            request->propertyList);
    }
    else
#endif

#ifdef PEGASUS_ENABLE_INDICATION_COUNT
    if (request->className.equal(PEGASUS_CLASSNAME_PROVIDERINDDATA))
    {
        instance = _providerIndicationCountTable.
            getProviderIndicationDataInstance(request->instanceName);
    }
    else if (request->className.equal(
             PEGASUS_CLASSNAME_SUBSCRIPTIONINDDATA))
    {
        instance = _subscriptionTable->
            getSubscriptionIndicationDataInstance(request->instanceName);
    }
    else
#endif
    {
        _checkNonprivilegedAuthorization(userName);

        //
        //  Add Creator to property list, if not null
        //  Also, if a Subscription and Time Remaining is requested,
        //  Ensure Subscription Duration and Start Time are in property list
        //
        Boolean setTimeRemaining;
        Boolean startTimeAdded;
        Boolean durationAdded;
        CIMPropertyList propertyList = request->propertyList;
        CIMName className = request->instanceName.getClassName();

        _updatePropertyList(
            className,
            propertyList,
            setTimeRemaining,
            startTimeAdded,
            durationAdded);

        // Set SystemName to empty String for internal processing
        // the SystemName will be fixed with correct fully qualified hostname
        // on return.
        _setSystemName(request->instanceName,String::EMPTY);

        //
        //  Get instance from repository
        //
        instance = _subscriptionRepository->getInstance(
            request->nameSpace,
            request->instanceName,
            request->includeQualifiers,
            request->includeClassOrigin,
            propertyList);

        //
        //  Remove Creator property from instance before returning
        //
        String creator;
        if (!_getCreator(instance, creator))
        {
            //
            //  This instance from the repository is corrupted
            //
            MessageLoaderParms parms(
                _MSG_INVALID_INSTANCES_KEY,
                _MSG_INVALID_INSTANCES);
            throw PEGASUS_CIM_EXCEPTION_L(CIM_ERR_FAILED, parms);
        }

        // check if this is SNMP Handler
        if (className.equal(PEGASUS_CLASSNAME_INDHANDLER_SNMP))
        {
            if(String::compare(creator,userName) != 0)
            {
                // only the creator of the handler has access to
                // the handler deata.
                MessageLoaderParms parms(
                "IndicationService.IndicationService."
                    "_MSG_NON_PRIVILEGED_ACCESS_DISABLED",
                "User ($0) is not authorized to perform this operation.",
                userName);
                PEG_METHOD_EXIT();
                throw PEGASUS_CIM_EXCEPTION_L(CIM_ERR_ACCESS_DENIED, parms);
            }
        }

        instance.removeProperty(
            instance.findProperty(
                PEGASUS_PROPERTYNAME_INDSUB_CREATOR));

        // Remove Creation Time property from CIMXML handlers

        if (className.equal(PEGASUS_CLASSNAME_INDHANDLER_CIMXML) ||
            className.equal(PEGASUS_CLASSNAME_LSTNRDST_CIMXML))
        {
            Uint32 idx = instance.findProperty(
                PEGASUS_PROPERTYNAME_LSTNRDST_CREATIONTIME);

            if (idx  != PEG_NOT_FOUND)
            {
                instance.removeProperty(idx);
            }
        }

        // Put host name back into SystemName property if not Subscription
        if ((!className.equal(PEGASUS_CLASSNAME_INDSUBSCRIPTION)) &&
            (!className.equal(PEGASUS_CLASSNAME_FORMATTEDINDSUBSCRIPTION)))
        {
            // this is a Filter or Handler instance
            _setOrAddSystemNameInHandlerFilter(
                instance,
                System::getFullyQualifiedHostName());
        }

        //
        //  Remove the language properties from instance before returning
        //
        Uint32 propIndex = instance.findProperty(
            PEGASUS_PROPERTYNAME_INDSUB_ACCEPTLANGS);
        if (propIndex != PEG_NOT_FOUND)
        {
            instance.removeProperty(propIndex);
        }

        propIndex = instance.findProperty(
            PEGASUS_PROPERTYNAME_INDSUB_CONTENTLANGS);
        if (propIndex != PEG_NOT_FOUND)
        {
             // Get the content languages to be sent in the Content-Language
             // header
             instance.getProperty(propIndex).getValue().
                 get(contentLangsString);
             instance.removeProperty(propIndex);
        }

        //
        //  If a subscription with a duration, calculate subscription time
        //  remaining, and add property to the instance
        //
        if (setTimeRemaining)
        {
            _setTimeRemaining(instance);
            if (startTimeAdded)
            {
                instance.removeProperty(
                    instance.findProperty(
                        _PROPERTY_STARTTIME));
            }
            if (durationAdded)
            {
                instance.removeProperty(
                    instance.findProperty(
                        _PROPERTY_DURATION));
            }
        }
    }

    CIMGetInstanceResponseMessage * response =
        dynamic_cast<CIMGetInstanceResponseMessage *>(request->buildResponse());
    if (contentLangsString.size())
    {
        // Note: setting Content-Language in the response to the
        // contentLanguage in the repository.
        response->operationContext.set(ContentLanguageListContainer(
            LanguageParser::parseContentLanguageHeader(contentLangsString)));
    }
    response->getResponseData().setInstance(instance);
    _enqueueResponse(request, response);

    PEG_METHOD_EXIT();
}

void IndicationService::_handleEnumerateInstancesRequest(const Message* message)
{
    PEG_METHOD_ENTER(TRC_INDICATION_SERVICE,
        "IndicationService::_handleEnumerateInstancesRequest");

    CIMEnumerateInstancesRequestMessage* request =
        (CIMEnumerateInstancesRequestMessage*) message;

    Array<CIMInstance> returnedInstances;
    String aggregatedLangs;

    String userName = ((IdentityContainer)request->operationContext.
        get(IdentityContainer::NAME)).getUserName();

#ifdef PEGASUS_ENABLE_DMTF_INDICATION_PROFILE_SUPPORT
    if (request->className.equal(PEGASUS_CLASSNAME_CIM_INDICATIONSERVICE) ||
        request->className.equal(
            PEGASUS_CLASSNAME_CIM_INDICATIONSERVICECAPABILITIES))
    {
        returnedInstances = _indicationServiceConfiguration->
            enumerateInstancesForClass(
                request->nameSpace,
                request->className,
                request->includeQualifiers,
                request->includeClassOrigin,
                request->propertyList);
    }
    else
#endif

#ifdef PEGASUS_ENABLE_INDICATION_COUNT
    if (request->className.equal(PEGASUS_CLASSNAME_PROVIDERINDDATA))
    {
        returnedInstances = _providerIndicationCountTable.
            enumerateProviderIndicationDataInstances();
    }
    else if (request->className.equal(
             PEGASUS_CLASSNAME_SUBSCRIPTIONINDDATA))
    {
        returnedInstances = _subscriptionTable->
            enumerateSubscriptionIndicationDataInstances();
    }
    else
#endif
    {
        _checkNonprivilegedAuthorization(userName);
        Array<CIMInstance> enumInstances;

        //
        //  Add Creator to property list, if not null
        //  Also, if a Subscription and Time Remaining is requested,
        //  Ensure Subscription Duration and Start Time are in property
        //  list
        //
        Boolean setTimeRemaining;
        Boolean startTimeAdded;
        Boolean durationAdded;
        CIMPropertyList propertyList = request->propertyList;
        _updatePropertyList(request->className,
            propertyList, setTimeRemaining, startTimeAdded, durationAdded);

        enumInstances =
            _subscriptionRepository->enumerateInstancesForClass(
                request->nameSpace, request->className,
                request->includeQualifiers, request->includeClassOrigin,
                propertyList);

        // Vars used to aggregate the content languages of the subscription
        // instances.
        Boolean langMismatch = false;
        Uint32 propIndex;

        //  In a loop do the following to all instances to be returned:
        //  ============================================================
        //  Remove Creator and language properties from instances before
        //  returning
        //  Remove CreationTime property from CIMXML handlers
        //  Fix-up Content-Language header if necessary
        //
        //  If a subscription with a duration, calculate subscription
        //  time remaining, and add property to the instance
        //
        //  put the host name into SystemName properties and key bindings
        //
        for (Uint32 i = 0; i < enumInstances.size(); i++)
        {
            CIMInstance adjustedInstance=enumInstances[i];
            String creator;
            if (!_getCreator(adjustedInstance, creator))
            {
                //
                //  This instance from the repository is corrupted
                //  Skip it
                //
                continue;
            }

            CIMName clsName = adjustedInstance.getClassName();

            // check if this is SNMP Handler
            if (clsName.equal(PEGASUS_CLASSNAME_INDHANDLER_SNMP))
            {
                if(String::compare(creator,userName) != 0)
                {
                    // only the creator of the handler has access to
                    // the handler deata.
                    continue;
                }
            }

            adjustedInstance.removeProperty(
                adjustedInstance.findProperty(
                    PEGASUS_PROPERTYNAME_INDSUB_CREATOR));

            // Remove CreationTime property from CIMXML handlers

            if (clsName.equal(PEGASUS_CLASSNAME_INDHANDLER_CIMXML) ||
                clsName.equal(PEGASUS_CLASSNAME_LSTNRDST_CIMXML))
            {
                Uint32 idx = adjustedInstance.findProperty(
                    PEGASUS_PROPERTYNAME_LSTNRDST_CREATIONTIME);

                if (idx  != PEG_NOT_FOUND)
                {
                    adjustedInstance.removeProperty(idx);
                }
            }

            propIndex = adjustedInstance.findProperty(
                PEGASUS_PROPERTYNAME_INDSUB_CONTENTLANGS);
            String contentLangs;
            if (propIndex != PEG_NOT_FOUND)
            {
                adjustedInstance.getProperty(propIndex).getValue().get(
                    contentLangs);
                adjustedInstance.removeProperty(propIndex);
            }

            propIndex = adjustedInstance.findProperty(
                PEGASUS_PROPERTYNAME_INDSUB_ACCEPTLANGS);
            if (propIndex != PEG_NOT_FOUND)
            {
                adjustedInstance.removeProperty(propIndex);
            }

            // Determine what to set into the Content-Language header back
            // to the client
            if (!langMismatch)
            {
                if (contentLangs == String::EMPTY)
                {
                    langMismatch = true;
                    aggregatedLangs = String::EMPTY;
                }
                else
                {
                    if (aggregatedLangs == String::EMPTY)
                    {
                        aggregatedLangs = contentLangs;
                    }
                    else if (aggregatedLangs != contentLangs)
                    {
                        langMismatch = true;
                        aggregatedLangs = String::EMPTY;
                    }
                }
            }

            //
            //  If a subscription with a duration, calculate subscription
            //  time remaining, and add property to the instance
            //
            if (setTimeRemaining)
            {
                try
                {
                    _setTimeRemaining(adjustedInstance);
                }
                catch (DateTimeOutOfRangeException&)
                {
                    //
                    //  This instance from the repository is invalid
                    //  Skip it
                    //
                    continue;
                }
                if (startTimeAdded)
                {
                    adjustedInstance.removeProperty(adjustedInstance.
                        findProperty(_PROPERTY_STARTTIME));
                }
                if (durationAdded)
                {
                    adjustedInstance.removeProperty(
                        adjustedInstance.findProperty(_PROPERTY_DURATION));
                }
            }
            // put the host name into SystemName properties and key bindings
            _setSystemName(
                adjustedInstance,
                System::getFullyQualifiedHostName());

            returnedInstances.append(adjustedInstance);
        }
    }

    CIMEnumerateInstancesResponseMessage* response =
        dynamic_cast<CIMEnumerateInstancesResponseMessage*>(
            request->buildResponse());
    PEGASUS_ASSERT(response != 0);
    if (aggregatedLangs.size())
    {
        // Note: setting Content-Language in the response to the aggregated
        // contentLanguage from the instances in the repository.
        response->operationContext.set(ContentLanguageListContainer(
            LanguageParser::parseContentLanguageHeader(aggregatedLangs)));
    }
    response->getResponseData().setInstances(returnedInstances);
    _enqueueResponse(request, response);

    PEG_METHOD_EXIT();
}

void IndicationService::_handleEnumerateInstanceNamesRequest(
    const Message* message)
{
    PEG_METHOD_ENTER(TRC_INDICATION_SERVICE,
        "IndicationService::_handleEnumerateInstanceNamesRequest");

    CIMEnumerateInstanceNamesRequestMessage* request =
        (CIMEnumerateInstanceNamesRequestMessage*) message;

    Array<CIMObjectPath> enumInstanceNames;

    String userName = ((IdentityContainer)request->operationContext.get(
        IdentityContainer::NAME)).getUserName();

#ifdef PEGASUS_ENABLE_DMTF_INDICATION_PROFILE_SUPPORT
    if (request->className.equal(
            PEGASUS_CLASSNAME_CIM_INDICATIONSERVICE) ||
        request->className.equal(
            PEGASUS_CLASSNAME_CIM_INDICATIONSERVICECAPABILITIES))
    {
        enumInstanceNames = _indicationServiceConfiguration->
            enumerateInstanceNamesForClass(
                request->nameSpace,
                request->className);
    }
    else
#endif

#ifdef PEGASUS_ENABLE_INDICATION_COUNT
    if (request->className.equal(PEGASUS_CLASSNAME_PROVIDERINDDATA))
    {
        enumInstanceNames = _providerIndicationCountTable.
            enumerateProviderIndicationDataInstanceNames();
    }
    else if (request->className.equal(
             PEGASUS_CLASSNAME_SUBSCRIPTIONINDDATA))
    {
        enumInstanceNames = _subscriptionTable->
            enumerateSubscriptionIndicationDataInstanceNames();
    }
    else
#endif
    {
        _checkNonprivilegedAuthorization(userName);
        enumInstanceNames =
            _subscriptionRepository->enumerateInstanceNamesForClass(
                request->nameSpace,
                request->className);

        // put the hostname back into SystemName key binding
        for (Uint32 i=0;i<enumInstanceNames.size();i++)
        {
            _setSystemName(
                enumInstanceNames[i],
                System::getFullyQualifiedHostName());
        }
    }

    // Note: not setting Content-Language in the response
    CIMEnumerateInstanceNamesResponseMessage* response =
        dynamic_cast<CIMEnumerateInstanceNamesResponseMessage *>(
            request->buildResponse());
    PEGASUS_ASSERT(response != 0);
    response->getResponseData().setInstanceNames(enumInstanceNames);
    _enqueueResponse(request, response);

    PEG_METHOD_EXIT();
}

void IndicationService::_handleModifyInstanceRequest(const Message* message)
{
    PEG_METHOD_ENTER(TRC_INDICATION_SERVICE,
        "IndicationService::_handleModifyInstanceRequest");

    CIMModifyInstanceRequestMessage* request =
        (CIMModifyInstanceRequestMessage*) message;

    Boolean responseSent = false;

    String userName = ((IdentityContainer)request->operationContext.get(
        IdentityContainer::NAME)).getUserName();
    _checkNonprivilegedAuthorization(userName);

    //
    //  Get modified instance and instance name from request
    //
    CIMInstance modifiedInstance = request->modifiedInstance;
    CIMObjectPath instanceReference = modifiedInstance.getPath();

    // set SystemName keybinding to empty in request's reference and instance
    _setSystemName(instanceReference,String::EMPTY);
    modifiedInstance.setPath(instanceReference);

    //
    //  Get instance from repository
    //
    CIMInstance instance;

    instance = _subscriptionRepository->getInstance(
        request->nameSpace, instanceReference);

    if (_canModify(request, instanceReference, instance, modifiedInstance))
    {
        //
        //  Set path in instance
        //
        instanceReference.setNameSpace(request->nameSpace);
        instance.setPath(instanceReference);

        //
        //  Check for expired subscription
        //
        try
        {
            if (_isExpired(instance))
            {
                //
                //  Delete the subscription instance
                //
                _deleteExpiredSubscription(instanceReference);
#ifdef PEGASUS_ENABLE_DMTF_INDICATION_PROFILE_SUPPORT
                _sendSubscriptionNotActiveMessagetoHandlerService(
                    instanceReference);
#endif
                PEG_METHOD_EXIT();

                throw PEGASUS_CIM_EXCEPTION_L(CIM_ERR_FAILED,
                    MessageLoaderParms(
                        "IndicationService.IndicationService._MSG_EXPIRED",
                        "An expired subscription cannot be modified:  the "
                            "subscription is deleted."));
            }
        }
        catch (DateTimeOutOfRangeException&)
        {
            //
            //  This instance from the repository is invalid
            //
            PEG_METHOD_EXIT();
            throw;
        }

        //
        //  _canModify, above, already checked that propertyList is not
        //  null, and that numProperties is 0 or 1
        //
        CIMPropertyList propertyList = request->propertyList;
        if (request->propertyList.size() > 0)
        {
            //
            //  Get current state from instance
            //
            Uint16 currentState;
            _subscriptionRepository->getState(instance, currentState);
            //
            //  Get new state
            //
            //  NOTE: _canModify has already validated the
            //  SubscriptionState property in the instance; if missing, it
            //  was added with the default value; if null, it was set to
            //  the default value; if invalid, an exception was thrown
            //
            Uint16 newState;
            modifiedInstance.getProperty(modifiedInstance.findProperty(
                PEGASUS_PROPERTYNAME_SUBSCRIPTION_STATE)).getValue().get(
                    newState);

            //
            //  If Subscription State has changed,
            //  Set Time of Last State Change to current date time
            //
            CIMDateTime currentDateTime =
                CIMDateTime::getCurrentDateTime();
            if (newState != currentState)
            {
                if (modifiedInstance.findProperty(_PROPERTY_LASTCHANGE) !=
                    PEG_NOT_FOUND)
                {
                    CIMProperty lastChange = modifiedInstance.getProperty(
                        modifiedInstance.findProperty(
                            _PROPERTY_LASTCHANGE));
                    lastChange.setValue(CIMValue(currentDateTime));
                }
                else
                {
                    modifiedInstance.addProperty(CIMProperty(
                        _PROPERTY_LASTCHANGE, CIMValue(currentDateTime)));
                }
                Array<CIMName> properties =
                    propertyList.getPropertyNameArray();
                properties.append(_PROPERTY_LASTCHANGE);
                propertyList.set(properties);
            }

            //
            //  If Subscription is to be enabled, and this is the first
            //  time, set Subscription Start Time
            //
            if ((newState == STATE_ENABLED) ||
                (newState == STATE_ENABLEDDEGRADED))
            {
                //
                //  If Subscription Start Time is null, set value
                //  to the current date time
                //
                CIMDateTime startTime;
                CIMProperty startTimeProperty = instance.getProperty(
                    instance.findProperty(_PROPERTY_STARTTIME));
                CIMValue startTimeValue = startTimeProperty.getValue();
                Boolean setStart = false;
                if (startTimeValue.isNull())
                {
                    setStart = true;
                }
                else
                {
                    startTimeValue.get(startTime);

                    if (startTime.isInterval())
                    {
                        if (startTime.equal(
                                CIMDateTime(_ZERO_INTERVAL_STRING)))
                        {
                            setStart = true;
                        }
                    }
                }

                if (setStart)
                {
                    if (modifiedInstance.findProperty(_PROPERTY_STARTTIME)
                        != PEG_NOT_FOUND)
                    {
                        CIMProperty startTimeProperty =
                            modifiedInstance.getProperty(
                                modifiedInstance.findProperty(
                                    _PROPERTY_STARTTIME));
                        startTimeProperty.setValue(CIMValue(currentDateTime));
                    }
                    else
                    {
                        modifiedInstance.addProperty(CIMProperty(
                            _PROPERTY_STARTTIME,
                            CIMValue(currentDateTime)));
                    }

                    Array<CIMName> properties =
                        propertyList.getPropertyNameArray();
                    properties.append(_PROPERTY_STARTTIME);
                    propertyList.set(properties);
                }
            }

            // Add the language properties to the modified instance.
            // Note:  These came from the Accept-Language and
            // Content-Language headers in the HTTP messages, and may be
            // empty.
            AcceptLanguageList acceptLangs =
                ((AcceptLanguageListContainer)request->operationContext.get(
                AcceptLanguageListContainer::NAME)).getLanguages();
            modifiedInstance.addProperty(CIMProperty(
                PEGASUS_PROPERTYNAME_INDSUB_ACCEPTLANGS,
                LanguageParser::buildAcceptLanguageHeader(acceptLangs)));

            ContentLanguageList contentLangs =
                ((ContentLanguageListContainer)request->operationContext.get
                (ContentLanguageListContainer::NAME)).getLanguages();
            modifiedInstance.addProperty (CIMProperty
                (PEGASUS_PROPERTYNAME_INDSUB_CONTENTLANGS,
                LanguageParser::buildContentLanguageHeader(contentLangs)));

            Array<CIMName> properties = propertyList.getPropertyNameArray();
            properties.append (PEGASUS_PROPERTYNAME_INDSUB_ACCEPTLANGS);
            properties.append (PEGASUS_PROPERTYNAME_INDSUB_CONTENTLANGS);
            propertyList.set (properties);

            //
            //  If subscription is to be enabled, determine if there are
            //  any indication providers that can serve the subscription
            //
            Array<ProviderClassList> indicationProviders;
            CIMPropertyList requiredProperties;
            String condition;
            String query;
            String queryLanguage;
            Array<NamespaceClassList> indicationSubclasses;

            if (((newState == STATE_ENABLED) ||
                 (newState == STATE_ENABLEDDEGRADED))
                && ((currentState != STATE_ENABLED) &&
                    (currentState != STATE_ENABLEDDEGRADED)))
            {
                //
                //  Subscription was previously not enabled but is now to
                //  be enabled
                //
                _getCreateParams(
                    instance,
                    indicationSubclasses,
                    indicationProviders,
                    requiredProperties,
                    condition,
                    query,
                    queryLanguage);

                if (indicationProviders.size() == 0)
                {
                    //
                    //  There are no providers that can support this
                    //  subscription
                    //
                    instance.setPath(instanceReference);
                    _subscriptionRepository->reconcileFatalError(instance);
                    PEG_METHOD_EXIT();

                    throw PEGASUS_CIM_EXCEPTION_L(CIM_ERR_NOT_SUPPORTED,
                        MessageLoaderParms(_MSG_NO_PROVIDERS_KEY,
                        _MSG_NO_PROVIDERS));
                }
            }

            //
            //  Modify the instance in the repository
            //
            modifiedInstance.setPath(instanceReference);
            _subscriptionRepository->modifyInstance(
                request->nameSpace, modifiedInstance,
                request->includeQualifiers, propertyList);

            PEG_TRACE((
                TRC_INDICATION_SERVICE,
                Tracer::LEVEL3,
                "IndicationService::_handleModifyInstanceRequest - "
                    "Name Space: %s  Instance name: %s",
                (const char*)
                request->nameSpace.getString().getCString(),
                (const char*)
                modifiedInstance.getClassName().getString().getCString()
            ));

            //
            //  If subscription is newly enabled, send Create requests
            //  and enable providers
            //
            if (((newState == STATE_ENABLED) ||
                 (newState == STATE_ENABLEDDEGRADED))
                && ((currentState != STATE_ENABLED) &&
                    (currentState != STATE_ENABLEDDEGRADED)))
            {
                instanceReference.setNameSpace(request->nameSpace);
                instance.setPath(instanceReference);

                _sendAsyncCreateRequests(
                    indicationProviders,
                    requiredProperties,
                    condition,
                    query,
                    queryLanguage,
                    instance,
                    acceptLangs,
                    contentLangs,
                    request,
                    indicationSubclasses,
                    userName,
                    request->authType);

                //
                //  Response is sent from _handleCreateResponseAggregation
                //
                responseSent = true;
            }
            else if ((newState == STATE_DISABLED) &&
                     ((currentState == STATE_ENABLED) ||
                      (currentState == STATE_ENABLEDDEGRADED)))
            {
                //
                //  Subscription was previously enabled but is now to be
                //  disabled
                //
                instanceReference.setNameSpace(request->nameSpace);
                instance.setPath(instanceReference);
                indicationProviders =
                    _getDeleteParams(instance, indicationSubclasses);

                //
                //  Send Delete requests
                //
                if (indicationProviders.size() > 0)
                {
                    _sendAsyncDeleteRequests(
                        indicationProviders,
                        instance,
                        acceptLangs,
                        contentLangs,
                        request,
                        indicationSubclasses,
                        userName,
                        request->authType);

                    //
                    //  Response is sent from
                    //  _handleDeleteResponseAggregation
                    //
                    responseSent = true;
#ifdef PEGASUS_ENABLE_DMTF_INDICATION_PROFILE_SUPPORT
                    _sendSubscriptionNotActiveMessagetoHandlerService(
                        instanceReference);
#endif
                }
            }
        }
    }

    //
    //  Send response, if not sent from callback
    //  (for example, if there are no indication providers that can support a
    //  subscription)
    //
    if (!responseSent)
    {
        // Note: don't need to set content-language in the response.
        CIMResponseMessage * response = request->buildResponse();
        _enqueueResponse(request, response);
    }

    PEG_METHOD_EXIT();
}

void IndicationService::_handleDeleteInstanceRequest(const Message* message)
{
    PEG_METHOD_ENTER(TRC_INDICATION_SERVICE,
        "IndicationService::_handleDeleteInstanceRequest");

    CIMDeleteInstanceRequestMessage* request =
        (CIMDeleteInstanceRequestMessage*) message;

    Boolean responseSent = false;

    String userName = ((IdentityContainer)request->operationContext.get(
        IdentityContainer::NAME)).getUserName();
    _checkNonprivilegedAuthorization(userName);

    // set eventual SystemName keybinding to empty string
    _setSystemName(request->instanceName,String::EMPTY);

    //
    //  Check if instance may be deleted -- a filter or handler instance
    //  referenced by a subscription instance may not be deleted
    //
    if (_canDelete(request->instanceName, request->nameSpace, userName))
    {
        //
        //  If a subscription, get the instance from the repository
        //
        CIMInstance subscriptionInstance;
        if (request->instanceName.getClassName().equal(
                PEGASUS_CLASSNAME_INDSUBSCRIPTION) ||
            request->instanceName.getClassName ().equal(
                PEGASUS_CLASSNAME_FORMATTEDINDSUBSCRIPTION))
        {
            subscriptionInstance =
                _subscriptionRepository->getInstance(
                    request->nameSpace, request->instanceName);
        }

        //
        //  Delete instance from repository
        //
        _subscriptionRepository->deleteInstance(
            request->nameSpace, request->instanceName);

#ifdef PEGASUS_ENABLE_DMTF_INDICATION_PROFILE_SUPPORT
        if (request->instanceName.getClassName().equal(
                PEGASUS_CLASSNAME_LSTNRDST_CIMXML) ||
            request->instanceName.getClassName().equal(
               PEGASUS_CLASSNAME_INDHANDLER_CIMXML) ||
            request->instanceName.getClassName().equal(
               PEGASUS_CLASSNAME_INDHANDLER_WSMAN))
        {
            CIMObjectPath handlerName = request->instanceName;
            handlerName.setNameSpace(request->nameSpace);
            _sendListenerNotActiveMessagetoHandlerService(handlerName);
        }
#endif

        PEG_TRACE((
            TRC_INDICATION_SERVICE,
            Tracer::LEVEL3,
            "IndicationService::_handleDeleteInstanceRequest - "
                "Name Space: %s  Instance name: %s",
            (const char*) request->nameSpace.getString().getCString(),
            (const char*)
           request->instanceName.getClassName().getString().getCString()
        ));

        if (request->instanceName.getClassName().equal(
                PEGASUS_CLASSNAME_INDSUBSCRIPTION) ||
            request->instanceName.getClassName ().equal(
                PEGASUS_CLASSNAME_FORMATTEDINDSUBSCRIPTION))
        {
            //
            //  If subscription is active, send delete requests to providers
            //  and update hash tables
            //
            Uint16 subscriptionState;
            CIMValue subscriptionStateValue;
            subscriptionStateValue = subscriptionInstance.getProperty(
                subscriptionInstance.findProperty(
                    PEGASUS_PROPERTYNAME_SUBSCRIPTION_STATE)).getValue();
            subscriptionStateValue.get(subscriptionState);

            if ((subscriptionState == STATE_ENABLED) ||
                (subscriptionState == STATE_ENABLEDDEGRADED))
            {
                Array<ProviderClassList> indicationProviders;
                Array<NamespaceClassList> indicationSubclasses;
                CIMObjectPath instanceReference = request->instanceName;
                instanceReference.setNameSpace(request->nameSpace);
                subscriptionInstance.setPath(instanceReference);

                indicationProviders = _getDeleteParams(
                    subscriptionInstance,
                    indicationSubclasses);

                if (indicationProviders.size() > 0)
                {
                    //
                    //  Send Delete requests
                    //
                    _sendAsyncDeleteRequests(
                        indicationProviders,
                        subscriptionInstance,
                        ((AcceptLanguageListContainer)
                            request->operationContext.get(
                                AcceptLanguageListContainer::NAME)).
                                    getLanguages(),
                        ((ContentLanguageListContainer)
                            request->operationContext.get(
                                ContentLanguageListContainer::NAME)).
                                    getLanguages(),
                        request,
                        indicationSubclasses,
                        userName,
                        request->authType);

                    //
                    //  Response is sent from
                    //  _handleDeleteResponseAggregation
                    //
                    responseSent = true;
#ifdef PEGASUS_ENABLE_DMTF_INDICATION_PROFILE_SUPPORT
                    _sendSubscriptionNotActiveMessagetoHandlerService(
                        instanceReference);
#endif
                }
                else
                {
                    //
                    //  Subscription was enabled, but had no providers
                    //  Remove entries from the subscription hash tables
                    //
                    _subscriptionTable->removeSubscription(
                        subscriptionInstance,
                        indicationSubclasses,
                        indicationProviders);
                }
            }
        }
    }

    //
    //  Send response, if not sent from callback
    //  (for example, if a subscription had no indication providers)
    //
    if (!responseSent)
    {
        CIMResponseMessage * response = request->buildResponse();
        _enqueueResponse(request, response);
    }

    PEG_METHOD_EXIT();
}

void IndicationService::_handleProcessIndicationResponse(Message* message)
{
    PEG_METHOD_ENTER(TRC_INDICATION_SERVICE,
        "IndicationService::_handleProcessIndicationResponse");

    CIMProcessIndicationResponseMessage* response = dynamic_cast<
        CIMProcessIndicationResponseMessage*> (message);
    PEGASUS_ASSERT(response != 0);

    CIMInstance instance = response->subscription;
    try
    {

        if (!_subscriptionRepository->reconcileFatalError(instance))
        {
            delete message;
            PEG_METHOD_EXIT();
            return;
        }

        String creator = instance.getProperty (instance.findProperty
            (PEGASUS_PROPERTYNAME_INDSUB_CREATOR)).getValue ().toString ();

        AcceptLanguageList acceptLangs;
        Uint32 propIndex = instance.findProperty
            (PEGASUS_PROPERTYNAME_INDSUB_ACCEPTLANGS);
        if (propIndex != PEG_NOT_FOUND)
        {
            String acceptLangsString;
            instance.getProperty(propIndex).getValue().get(acceptLangsString);
            if (acceptLangsString.size())
            {
                acceptLangs = LanguageParser::parseAcceptLanguageHeader(
                    acceptLangsString);
            }
        }
        ContentLanguageList contentLangs;
        propIndex = instance.findProperty(
           PEGASUS_PROPERTYNAME_INDSUB_CONTENTLANGS);

        if (propIndex != PEG_NOT_FOUND)
        {
            String contentLangsString;
            instance.getProperty(propIndex).getValue().get(contentLangsString);
            if (contentLangsString.size())
            {
                contentLangs = LanguageParser::parseContentLanguageHeader(
                    contentLangsString);
            }
        }
        Array<NamespaceClassList> indicationSubclasses;
        Array<ProviderClassList> indicationProviders;
        indicationProviders = _getDeleteParams(instance, indicationSubclasses);
        PEGASUS_ASSERT(indicationProviders.size() > 0);

        _subscriptionTable->removeSubscription(
            instance,
            indicationSubclasses,
            indicationProviders);

        _sendWaitDeleteRequests (
            indicationProviders,
            instance,
            acceptLangs,
            contentLangs,
            creator);
#ifdef PEGASUS_ENABLE_DMTF_INDICATION_PROFILE_SUPPORT
        _sendSubscriptionNotActiveMessagetoHandlerService(instance.getPath());
#endif
    }
    catch(const Exception &e)
    {
        PEG_TRACE ((TRC_DISCARDED_DATA, Tracer::LEVEL2,
            "Exception caught trying to reconcile the subscription's"
                " OnFatalErrorPolicy %s",
            (const char *) e.getMessage ().getCString()));
    }
    catch(...)
    {
        PEG_TRACE ((TRC_DISCARDED_DATA, Tracer::LEVEL2,
            "Unknown exception caught trying to reconcile the subscription's"
                " OnFatalErrorPolicy"));
    }
    delete message;
    PEG_METHOD_EXIT();
}


// l10n TODO - might need to globalize another flow and another consumer
// interface (ie. mdd's) if we can't agree on one export flow and consumer
// interface (see PEP67)

void IndicationService::_handleProcessIndicationRequest(Message* message)
{

#ifdef PEGASUS_ENABLE_DMTF_INDICATION_PROFILE_SUPPORT
    _processIndicationThreads++;
    AutoPtr<AtomicInt, DecAtomicInt> counter(&_processIndicationThreads);
#endif

#ifdef PEGASUS_INDICATION_PERFINST
    Stopwatch stopWatch;
#endif

    PEG_METHOD_ENTER(TRC_INDICATION_SERVICE,
        "IndicationService::_handleProcessIndicationRequest");

#ifdef PEGASUS_INDICATION_PERFINST
        stopWatch.reset();
        stopWatch.start();
#endif

    CIMProcessIndicationRequestMessage* request = dynamic_cast<
        CIMProcessIndicationRequestMessage*> (message);
    PEGASUS_ASSERT(request != 0);

    Array<CIMInstance> matchedSubscriptions;
    Array<SubscriptionKey> matchedSubscriptionsKeys;

    CIMInstance indication = request->indicationInstance;

    QueueIdStack qids = request->queueIds.copyAndPop();

    AutoPtr<DeliveryStatusAggregator, ExpectedResponseCountSetDone>
        deliveryStatusAggregator;

    if (qids.size())
    {
#ifdef PEGASUS_ENABLE_INDICATION_ORDERING
        deliveryStatusAggregator.reset(
            new DeliveryStatusAggregator(
                request->messageId,
                qids.top(),
                request->oopAgentName,
                request->timeoutMilliSec));
#endif
    }

    try
    {
        PEG_TRACE ((TRC_INDICATION_GENERATION, Tracer::LEVEL4,
           "Received %s Indication %s from namespace %s from provider %s",
           (const char*)(indication.getClassName().getString().getCString()),
           (const char*)(request->messageId.getCString()),
           (const char*)(request->nameSpace.getString().getCString()),
           (const char*)(request->provider.getProperty(request->provider.
               findProperty(PEGASUS_PROPERTYNAME_NAME)).getValue().toString().
                   getCString())));

        // If there are subscription requests pending with the providers,
        // deliver the indication later when all create subscription
        // requests are completed.
        // It is possible that indication arrives after provider's
        // enableIndication() method is called and before create
        // subscription request is completed by IndicationService.
        // This casuses the indications to be lost because of no matching
        // subscriptions exists.
        if (_subscriptionRepository->getUncommittedCreateSubscriptionRequests())
        {
            PEG_TRACE_CSTRING(TRC_INDICATION_GENERATION, Tracer::LEVEL3,
                "Pending Create subscription requets exists, indication"
                   " will be delivered after all pending create"
                   " subscription requets are completed");
             CIMProcessIndicationRequestMessage * requestCopy =
                    new CIMProcessIndicationRequestMessage(*request);
            // Delivery enqueue status is not required for this request.
            requestCopy->queueIds = QueueIdStack(requestCopy->queueIds.top());
            _deliveryWaitIndications.insert_back(requestCopy);
            PEG_METHOD_EXIT();
            return;
        }

        //
        // Get supported properties by the indication provider
        // Get Indication class properties
        // Check if the provider supports all properties of the indication
        // class, if so, set to null
        //
        Array<CIMName> providerSupportedProperties;
        Array<CIMName> indicationClassProperties;
        CIMPropertyList supportedPropertyList;

        for (Uint32 i = 0; i < indication.getPropertyCount(); i++)
        {
            providerSupportedProperties.append(
                indication.getProperty(i).getName());
        }

        supportedPropertyList = _checkPropertyList(providerSupportedProperties,
                                                   request->nameSpace,
                                                   indication.getClassName(),
                                                   indicationClassProperties);

        //
        // Get initial subscriptions based on the class name, namespace
        // of the generated indication, and subscriptions specified by the
        // indication provider if the provider included subscriptions
        // in the subscriptionInstanceNamesContainer
        //
        Array<CIMInstance> subscriptions;
        Array<SubscriptionKey> subscriptionKeys;

        _getRelevantSubscriptions(
            request->subscriptionInstanceNames,
            indication.getClassName(),
            request->nameSpace,
            request->provider,
            subscriptions,
            subscriptionKeys);

        for (Uint32 i = 0; i < subscriptions.size(); i++)
        {
            try
            {
                QueryExpression queryExpr;
                String filterQuery;
                String queryLanguage;
                String filterName;
                Array<CIMNamespaceName> sourceNameSpaces;

                //
                //  Get filter query expression of the subscription
                //
                _subscriptionRepository->getFilterProperties(
                     subscriptions[i],
                     filterQuery,
                     sourceNameSpaces,
                     queryLanguage,
                     filterName);

                queryExpr = _getQueryExpression(
                    filterQuery, queryLanguage, request->nameSpace);

                //
                // Evaluate if the subscription matches the indication by
                // checking:
                // 1) Whether the properties (in WHERE clause) from filter
                //    query are supported by the indication provider;
                // 2) Whether the subscripton is expired;
                // 3) Whether the filter criteria are met by the generated
                //    indication
                //
                if (_subscriptionMatch (subscriptions[i], indication,
                    supportedPropertyList, queryExpr, request->nameSpace))
                {
                    PEG_TRACE ((TRC_INDICATION_GENERATION, Tracer::LEVEL4,
                        "%s Indication %s satisfies filter %s:%s query "
                            "expression  \"%s\"",
                            (const char*)(indication.getClassName().
                                getString().getCString()),
                            (const char*)(request->messageId.getCString()),
                            (const char*)(request->nameSpace.getString().
                                getCString()),
                            (const char*)(filterName.getCString()),
                            (const char*)(filterQuery.getCString())));

                    //
                    // Format the indication
                    // This includes two parts:
                    // 1) Use QueryExpression::applyProjection to remove
                    //    properties not listed in the SELECT clause;
                    // 2) Remove any properties that may be left on the
                    //    indication that are not in the indication class.
                    //    These are properties added by the provider
                    //    incorrectly.
                    //
                    CIMInstance formattedIndication = indication.clone();

                    if (_formatIndication(formattedIndication,
                                          queryExpr,
                                          providerSupportedProperties,
                                          indicationClassProperties))
                    {
                        //
                        // get the handler instance and forward the formatted
                        // indication to the handler
                        //
                        CIMInstance handlerInstance =
                            _subscriptionRepository->getHandler(
                                subscriptions[i]);

                        PEG_TRACE((TRC_INDICATION_GENERATION, Tracer::LEVEL4,
                            "Handler %s:%s.%s found for %s Indication %s",
                            (const char*)(request->nameSpace.getString().
                                getCString()),
                            (const char*)(handlerInstance.getClassName().
                                getString().getCString()),
                            (const char*)(handlerInstance.getProperty(
                                handlerInstance.findProperty(
                                    PEGASUS_PROPERTYNAME_NAME)).getValue().
                                        toString().getCString()),
                            (const char*)(indication.getClassName().
                                getString().getCString()),
                            (const char*)(request->messageId.getCString())));

                        _forwardIndToHandler(subscriptions[i],
                                             handlerInstance,
                                             formattedIndication,
                                             request->nameSpace,
                                             request->operationContext,
                                             deliveryStatusAggregator.get());
                        matchedSubscriptions.append(subscriptions[i]);
                        matchedSubscriptionsKeys.append(subscriptionKeys[i]);
                    }
                }
            }
            catch (Exception& e)
            {
                PEG_TRACE ((TRC_DISCARDED_DATA, Tracer::LEVEL1,
                    "Exception caught in attempting to process indication "
                        "for the subscription %s: %s",
                        (const char *) subscriptions[i].getPath ().toString().
                            getCString(),
                        (const char *) e.getMessage ().getCString()));
            }
            catch (exception& e)
            {
                PEG_TRACE ((TRC_DISCARDED_DATA, Tracer::LEVEL1,
                    "Exception caught in attempting to process indication "
                        "for the subscription %s: %s",
                    (const char *) subscriptions[i].getPath ().toString().
                        getCString(), e.what()));
           }
           catch (...)
           {
               PEG_TRACE ((TRC_DISCARDED_DATA, Tracer::LEVEL1,
                   "Unknown exception caught in attempting to process "
                       "indication for the subscription %s",
                    (const char *) subscriptions[i].getPath ().toString ().
                        getCString()));
           }

        }

#ifdef PEGASUS_ENABLE_INDICATION_COUNT
        _providerIndicationCountTable.incrementEntry(
            request->provider, matchedSubscriptions.size() == 0);
        _subscriptionTable->updateMatchedIndicationCounts(
            request->provider, matchedSubscriptionsKeys);
#endif

        //
        //  Log subscriptions info to a trace message
        //
        if (matchedSubscriptions.size() == 0)
        {
           PEG_TRACE ((TRC_INDICATION_GENERATION, Tracer::LEVEL1,
               "No matching subscriptions found for %s Indication %s",
               (const char*)(indication.getClassName().getString().
                   getCString()),
               (const char*)(request->messageId.getCString())));
        }
        else
        {
            PEG_TRACE ((TRC_INDICATION_GENERATION, Tracer::LEVEL4,
                "%d subscriptions found for %s Indication %s in namespace %s",
                matchedSubscriptions.size(),
                (const char*)(indication.getClassName().getString().
                    getCString()),
                (const char*)(request->messageId.getCString()),
                (const char*)(request->nameSpace.getString().getCString())));
        }
    }
    catch (Exception& e)
    {
        PEG_TRACE((TRC_DISCARDED_DATA, Tracer::LEVEL1,
            "Exception caught while processing indication: %s.  "
                "Indication may be lost.",
            (const char*)e.getMessage().getCString()));
        PEG_METHOD_EXIT();
        throw;
    }
    catch (...)
    {
        PEG_TRACE_CSTRING(TRC_DISCARDED_DATA, Tracer::LEVEL1,
            "Exception caught while processing indication.  Indication may be "
                "lost.");
        PEG_METHOD_EXIT();
        throw;
    }

    _enqueueResponse(request, request->buildResponse());

#ifdef PEGASUS_INDICATION_PERFINST
    stopWatch.stop();

    PEG_TRACE((TRC_INDICATION_SERVICE, Tracer::LEVEL4,
        "%s: %.3f seconds", "Process Indication", stopWatch.getElapsed()));
#endif

    PEG_METHOD_EXIT ();
    return;
}

void IndicationService::_handleIndicationCallBack (
    AsyncOpNode * operation,
    MessageQueue * destination,
    void * userParameter)
{
    PEG_METHOD_ENTER (TRC_INDICATION_SERVICE,
        "IndicationService::_handleIndicationCallBack");

    IndicationService * service =
        static_cast<IndicationService *> (destination);
    AsyncReply * asyncReply =
        static_cast<AsyncReply *>(operation->removeResponse());
    CIMHandleIndicationResponseMessage* handlerResponse =
        reinterpret_cast<CIMHandleIndicationResponseMessage *>(
            (static_cast<AsyncLegacyOperationResult *>(
                asyncReply))->get_result());
    PEGASUS_ASSERT (handlerResponse != 0);

    if (handlerResponse->cimException.getCode () != CIM_ERR_SUCCESS)
    {
        PEG_TRACE((TRC_DISCARDED_DATA, Tracer::LEVEL1,
            "Sending Indication and HandlerService returns CIMException: %s",
            (const char*)
                handlerResponse->cimException.getMessage().getCString()));
    }

    delete handlerResponse;
    delete asyncReply;
    service->return_op (operation);

    PEG_METHOD_EXIT ();
}

void IndicationService::_handleNotifyProviderRegistrationRequest
    (const Message* message)
{
    PEG_METHOD_ENTER ( TRC_INDICATION_SERVICE,
        "IndicationService::_handleNotifyProviderRegistrationRequest");

    CIMNotifyProviderRegistrationRequestMessage* request =
        (CIMNotifyProviderRegistrationRequestMessage*) message;

    ProviderIdContainer pidc = request->operationContext.get
        (ProviderIdContainer::NAME);
    CIMInstance provider = pidc.getProvider();
    CIMInstance providerModule = pidc.getModule();
#ifdef PEGASUS_ENABLE_REMOTE_CMPI
    Boolean isRemoteNameSpace = pidc.isRemoteNameSpace();
    String remoteInfo = pidc.getRemoteInfo();
#endif

    CIMName className = request->className;
    Array<CIMNamespaceName> newNameSpaces = request->newNamespaces;
    Array<CIMNamespaceName> oldNameSpaces = request->oldNamespaces;
    CIMPropertyList newPropertyNames = request->newPropertyNames;
    CIMPropertyList oldPropertyNames = request->oldPropertyNames;

    Array<SubscriptionWithSrcNamespace> newSubscriptions;
    Array<SubscriptionWithSrcNamespace> formerSubscriptions;
    Array<ProviderClassList> indicationProviders;
    ProviderClassList indicationProvider;

    newSubscriptions.clear ();
    formerSubscriptions.clear ();

    switch (request->operation)
    {
        case OP_CREATE:
        {
            //
            //  Get matching subscriptions
            //
            newSubscriptions = _getMatchingSubscriptions (className,
                newNameSpaces, newPropertyNames);

            break;
        }

        case OP_DELETE:
        {
            //
            //  Get matching subscriptions
            //
            formerSubscriptions = _getMatchingSubscriptions (className,
                oldNameSpaces, oldPropertyNames);

#ifdef PEGASUS_ENABLE_INDICATION_COUNT
            _providerIndicationCountTable.removeEntry(provider);
#endif

            break;
        }

        case OP_MODIFY:
        {
            //
            //  Get lists of affected subscriptions
            //
            _getModifiedSubscriptions (className, newNameSpaces, oldNameSpaces,
                newPropertyNames, oldPropertyNames,
                newSubscriptions, formerSubscriptions);

            break;
        }
        default:
            //
            //  Error condition: operation not supported
            //
            PEG_METHOD_EXIT ();
            throw PEGASUS_CIM_EXCEPTION(CIM_ERR_NOT_SUPPORTED, String::EMPTY);
            break;
    }  // switch

    //
    //  Construct provider class list from input provider and class name
    //
    indicationProvider.provider = provider;
    indicationProvider.providerModule = providerModule;
    NamespaceClassList nscl;
    nscl.classList.append(className);
    indicationProvider.classList.append (nscl);
#ifdef PEGASUS_ENABLE_REMOTE_CMPI
    indicationProvider.isRemoteNameSpace = isRemoteNameSpace;
    indicationProvider.remoteInfo = remoteInfo;
#endif
    indicationProviders.append (indicationProvider);

    if (newSubscriptions.size () > 0)
    {
        CIMPropertyList requiredProperties;
        String condition;
        String query;
        String queryLanguage;

        //  Send Create or Modify request for each subscription that can newly
        //  be supported
        //
        for (Uint32 i = 0; i < newSubscriptions.size (); i++)
        {
            indicationProviders[0].classList[0].nameSpace =
                newSubscriptions[i].nameSpace;

            Array<NamespaceClassList> indicationSubclasses;
            _getCreateParams (
                newSubscriptions[i].subscription,
                indicationSubclasses,
                requiredProperties,
                condition,
                query,
                queryLanguage);


            //
            //  NOTE: These Create or Modify requests are not associated with a
            //  user request, so there is no associated authType or userName
            //  The Creator from the subscription instance is used for
            //  userName, and authType is not set
            //
            //  NOTE: the subscriptions in the newSubscriptions list came from
            //  the IndicationService's internal hash tables, and thus
            //  each instance is known to have a valid Creator property
            //
            CIMInstance instance = newSubscriptions[i].subscription;
            String creator = instance.getProperty (instance.findProperty
                (PEGASUS_PROPERTYNAME_INDSUB_CREATOR)).getValue ().toString ();

// l10n start
            AcceptLanguageList acceptLangs;
            Uint32 propIndex = instance.findProperty
                (PEGASUS_PROPERTYNAME_INDSUB_ACCEPTLANGS);
            if (propIndex != PEG_NOT_FOUND)
            {
                String acceptLangsString;
                instance.getProperty(propIndex).getValue().get(
                    acceptLangsString);
                if (acceptLangsString.size())
                {
                    acceptLangs = LanguageParser::parseAcceptLanguageHeader(
                        acceptLangsString);
                }
            }
            ContentLanguageList contentLangs;
            propIndex = instance.findProperty
                (PEGASUS_PROPERTYNAME_INDSUB_CONTENTLANGS);
            if (propIndex != PEG_NOT_FOUND)
            {
                String contentLangsString;
                instance.getProperty(propIndex).getValue().get(
                    contentLangsString);
                if (contentLangsString.size())
                {
                    contentLangs = LanguageParser::parseContentLanguageHeader(
                        contentLangsString);
                }
            }
// l10n end

            //
            //  Look up the subscription in the active subscriptions table
            //
            ActiveSubscriptionsTableEntry tableValue;
            if (_subscriptionTable->getSubscriptionEntry
                (newSubscriptions[i].subscription.getPath (), tableValue))
            {
                //
                //  If the provider is already in the subscription's list,
                //  send a Modify request, otherwise send a Create request
                //
                Uint32 providerIndex = _subscriptionTable->providerInList
                    (provider, tableValue);
                if (providerIndex != PEG_NOT_FOUND)
                {
                    //
                    //  Send Modify requests
                    //
                    _sendWaitModifyRequests (
                        indicationProviders,
                        requiredProperties,
                        condition,
                        query,
                        queryLanguage,
                        newSubscriptions[i].subscription,
                        acceptLangs,
                        contentLangs,
                        creator);
                }
                else
                {
                    //
                    //  Send Create requests
                    //
                    Array<ProviderClassList> acceptedProviders;
                    acceptedProviders = _sendWaitCreateRequests(
                        indicationProviders,
                        requiredProperties,
                        condition,
                        query,
                        queryLanguage,
                        newSubscriptions[i].subscription,
                        acceptLangs,
                        contentLangs,
                        creator);

                    if (acceptedProviders.size () > 0)
                    {
                        //
                        //  Provider is not yet in the list for this
                        //  subscription; add provider to the list
                        //
                        _subscriptionTable->updateProviders
                            (instance.getPath (), indicationProviders[0], true);
                    }
                }
            }
            else
            {
                //
                //  Subscription not found in Active Subscriptions table
                //
            }
        }

        //
        //  NOTE: When a provider that was previously not serving a subscription
        //  now serves the subscription due to a provider registration change,
        //  a log message is sent, even if there were previously other providers
        //  serving the subscription
        //

        //
        //  Log a message for each subscription
        //
        CIMClass providerClass = _subscriptionRepository->getClass
            (PEGASUS_NAMESPACENAME_INTEROP, PEGASUS_CLASSNAME_PROVIDER,
             true, true, false, CIMPropertyList ());
        CIMInstance providerCopy = provider.clone ();
        CIMObjectPath path = providerCopy.buildPath (providerClass);
        providerCopy.setPath (path);
        String logString1 = getProviderLogString (providerCopy);

        for (Uint32 j = 0; j < newSubscriptions.size (); j++)
        {
            //
            //  Get Provider Name, Subscription Filter Name and Handler Name
            //
            String logString2 = _getSubscriptionLogString
                (newSubscriptions[j].subscription);

            Logger::put_l(Logger::STANDARD_LOG, System::CIMSERVER,
                Logger::WARNING,
                MessageLoaderParms(
                    _MSG_PROVIDER_NOW_SERVING_KEY,
                    _MSG_PROVIDER_NOW_SERVING, logString1, logString2,
                    newSubscriptions[j].subscription.getPath().
                        getNameSpace().getString()));
        }
    }

    if (formerSubscriptions.size () > 0)
    {
        CIMPropertyList requiredProperties;
        String condition;
        String query;
        String queryLanguage;

        //
        //  Send Delete or Modify request for each subscription that can no
        //  longer be supported
        //
        for (Uint32 i = 0; i < formerSubscriptions.size (); i++)
        {
            indicationProviders[0].classList[0].nameSpace =
                formerSubscriptions[i].nameSpace;
            //
            //  NOTE: These Delete or Modify requests are not associated with a
            //  user request, so there is no associated authType or userName
            //  The Creator from the subscription instance is used for userName,
            //  and authType is not set
            //
            //  NOTE: the subscriptions in the formerSubscriptions list came
            //  from the IndicationService's internal hash tables, and thus
            //  each instance is known to have a valid Creator property
            //
            CIMInstance instance = formerSubscriptions[i].subscription;
            String creator = instance.getProperty (instance.findProperty
                (PEGASUS_PROPERTYNAME_INDSUB_CREATOR)).getValue ().toString ();
            AcceptLanguageList acceptLangs;
            Uint32 propIndex = instance.findProperty
                (PEGASUS_PROPERTYNAME_INDSUB_ACCEPTLANGS);
            if (propIndex != PEG_NOT_FOUND)
            {
                String acceptLangsString;
                instance.getProperty(propIndex).getValue().get(
                    acceptLangsString);
                if (acceptLangsString.size())
                {
                    acceptLangs = LanguageParser::parseAcceptLanguageHeader(
                        acceptLangsString);
                }
            }
            ContentLanguageList contentLangs;
            propIndex = instance.findProperty
                (PEGASUS_PROPERTYNAME_INDSUB_CONTENTLANGS);
            if (propIndex != PEG_NOT_FOUND)
            {
                String contentLangsString;
                instance.getProperty(propIndex).getValue().get(
                    contentLangsString);
                if (contentLangsString.size())
                {
                    contentLangs = LanguageParser::parseContentLanguageHeader(
                        contentLangsString);
                }
            }

            //
            //  Look up the subscription in the active subscriptions table
            //  If class list contains only the class name from the current
            //  operation, send a Delete request
            //  Otherwise, send a Modify request
            //
            ActiveSubscriptionsTableEntry tableValue;
            if (_subscriptionTable->getSubscriptionEntry
                (formerSubscriptions[i].subscription.getPath (), tableValue))
            {
                Uint32 providerIndex = _subscriptionTable->providerInList
                    (provider, tableValue);
                if (providerIndex != PEG_NOT_FOUND)
                {
                    Array<NamespaceClassList> indicationSubclasses;
                    _getCreateParams (
                        formerSubscriptions[i].subscription,
                        indicationSubclasses,
                        requiredProperties,
                        condition,
                        query,
                        queryLanguage);

                    //
                    //  If class list contains only the class name from the
                    //  current delete, send a Delete request
                    //
                    if ((tableValue.providers[providerIndex].classList.size()
                            == 1) &&
                        (tableValue.providers[providerIndex].classList[0].
                            classList[0].equal(className)))
                    {

                        if (tableValue.providers[providerIndex].
                           classList[0].nameSpace ==
                                formerSubscriptions[i].nameSpace)
                        {
                            _sendWaitDeleteRequests (
                                indicationProviders,
                                formerSubscriptions[i].subscription,
                                acceptLangs,
                                contentLangs,
                                creator);

                            //
                            //
                            //
                            _subscriptionTable->updateProviders(
                                instance.getPath(),
                                indicationProviders[0],
                                false);
                        }
                    }

                    //
                    //  Otherwise, send a Modify request
                    //
                    else
                    {
                        Uint32 classIndex = _subscriptionTable->classInList(
                            className,
                            formerSubscriptions[i].nameSpace,
                            tableValue.providers[providerIndex]);

                        if (classIndex != PEG_NOT_FOUND)
                        {
                            //
                            //  Send Modify requests
                            //
                            _sendWaitModifyRequests (
                                indicationProviders,
                                requiredProperties,
                                condition,
                                query,
                                queryLanguage,
                                formerSubscriptions[i].subscription,
                                acceptLangs,
                                contentLangs,
                                creator);
                        }
                        else
                        {
                            PEG_TRACE((TRC_INDICATION_SERVICE,Tracer::LEVEL1,
                                "Class %s not found in tableValue.providers",
                                (const char*)className.getString().getCString()
                                ));
                        }
                    }
                }
                else
                {
                    //
                    //  The subscription was not served by the provider
                    //
                }
            }
            else
            {
                //
                //  Subscription not found in Active Subscriptions table
                //
            }
        }

#if 0
        //
        //  Create NoProviderAlertIndication instance
        //  ATTN: NoProviderAlertIndication must be defined
        //
        CIMInstance indicationInstance = _createAlertInstance
            (_CLASS_NO_PROVIDER_ALERT, formerSubscriptions);

        //
        //  Send NoProviderAlertIndication to each unique handler instance
        //
        PEG_TRACE((TRC_INDICATION_SERVICE, Tracer::LEVEL4,
            "Sending NoProvider Alert for %u subscriptions",
            formerSubscriptions.size ()));
        _sendAlerts (formerSubscriptions, indicationInstance);
#endif
        //
        //  NOTE: When a provider that was previously serving a subscription
        //  no longer serves the subscription due to a provider registration
        //  change, a log message is sent, even if there are still other
        //  providers serving the subscription
        //

        //
        //  Log a message for each subscription
        //
        CIMClass providerClass = _subscriptionRepository->getClass
            (PEGASUS_NAMESPACENAME_INTEROP, PEGASUS_CLASSNAME_PROVIDER,
             true, true, false, CIMPropertyList ());
        CIMInstance providerCopy = provider.clone ();
        CIMObjectPath path = providerCopy.buildPath (providerClass);
        providerCopy.setPath (path);
        String logString1 = getProviderLogString (providerCopy);

        for (Uint32 j = 0; j < formerSubscriptions.size (); j++)
        {
            //
            //  Get Provider Name, Subscription Filter Name and Handler Name
            //
            String logString2 = _getSubscriptionLogString
                (formerSubscriptions[j].subscription);

            Logger::put_l(Logger::STANDARD_LOG, System::CIMSERVER,
                Logger::WARNING,
                MessageLoaderParms(
                    _MSG_PROVIDER_NO_LONGER_SERVING_KEY,
                    _MSG_PROVIDER_NO_LONGER_SERVING, logString1, logString2,
                    formerSubscriptions[j].subscription.getPath().
                        getNameSpace().getString()));
        }
    }

    //
    //  Send response
    //
    CIMResponseMessage * response = request->buildResponse ();
    _enqueueResponse (request, response);

    PEG_METHOD_EXIT ();
}

void IndicationService::_handleNotifyProviderTerminationRequest
    (const Message * message)
{
    PEG_METHOD_ENTER (TRC_INDICATION_SERVICE,
        "IndicationService::_handleNotifyProviderTermination");

    Array<CIMInstance> providerSubscriptions;
    CIMInstance indicationInstance;

    CIMNotifyProviderTerminationRequestMessage* request =
        (CIMNotifyProviderTerminationRequestMessage*) message;

    Array<CIMInstance> providers = request->providers;

    for (Uint32 i = 0; i < providers.size (); i++)
    {
#ifdef PEGASUS_ENABLE_INDICATION_COUNT
        _providerIndicationCountTable.removeEntry(providers[i]);
#endif

        //
        //  Get list of affected subscriptions
        //
        //  _subscriptionTable->reflectProviderDisable also updates the
        //  Active Subscriptions hash table, and implements each subscription's
        //  On Fatal Error policy, if necessary
        //
        providerSubscriptions.clear();
        providerSubscriptions = _subscriptionTable->reflectProviderDisable(
            providers[i]);

        if (providerSubscriptions.size() > 0)
        {
            //
            //  NOTE: When a provider that was previously serving a subscription
            //  no longer serves the subscription due to a provider termination,
            //  an alert is always sent, even if there are still other providers
            //  serving the subscription
            //

#if 0
            //
            //  Create ProviderTerminatedAlertIndication instance
            //  ATTN: ProviderTerminatedAlertIndication must be defined
            //
            indicationInstance = _createAlertInstance
                (_CLASS_PROVIDER_TERMINATED_ALERT, providerSubscriptions);

            //
            //  Send ProviderTerminatedAlertIndication to each unique handler
            //  instance
            //
            PEG_TRACE((TRC_INDICATION_SERVICE, Tracer::LEVEL4,
                "Sending ProviderDisabled Alert for %u subscriptions",
                providerSubscriptions.size ()));
            _sendAlerts (providerSubscriptions, indicationInstance);
#endif
            //
            //  Log a message for each subscription
            //
            CIMClass providerClass = _subscriptionRepository->getClass(
                PEGASUS_NAMESPACENAME_INTEROP, PEGASUS_CLASSNAME_PROVIDER,
                true, true, false, CIMPropertyList());
            CIMInstance providerCopy = providers[i].clone();
            CIMObjectPath path = providerCopy.buildPath (providerClass);
            providerCopy.setPath (path);
            for (Uint32 j = 0; j < providerSubscriptions.size (); j++)
            {
                //
                //  Get Provider Name, Subscription Filter Name and Handler Name
                //
                String logString1 = getProviderLogString (providerCopy);
                String logString2 = _getSubscriptionLogString
                    (providerSubscriptions[j]);

                Logger::put_l(Logger::STANDARD_LOG, System::CIMSERVER,
                    Logger::WARNING,
                    MessageLoaderParms(
                        _MSG_PROVIDER_NO_LONGER_SERVING_KEY,
                        _MSG_PROVIDER_NO_LONGER_SERVING, logString1, logString2,
                        providerSubscriptions[j].getPath().getNameSpace().
                            getString()));
            }
        }
    }

    CIMResponseMessage * response = request->buildResponse ();
    _enqueueResponse (request, response);

    PEG_METHOD_EXIT ();
}

void IndicationService::_handleNotifyProviderEnableRequest
    (const Message * message)
{
    PEG_METHOD_ENTER (TRC_INDICATION_SERVICE,
        "IndicationService::_handleNotifyProviderEnableRequest");

    CIMNotifyProviderEnableRequestMessage * request =
        (CIMNotifyProviderEnableRequestMessage *) message;
    ProviderIdContainer pidc = request->operationContext.get
        (ProviderIdContainer::NAME);
    CIMInstance providerModule = pidc.getModule();
    CIMInstance provider = pidc.getProvider();
#ifdef PEGASUS_ENABLE_REMOTE_CMPI
    Boolean isRemoteNameSpace = pidc.isRemoteNameSpace();
    String remoteInfo = pidc.getRemoteInfo();
#endif
    Array<CIMInstance> capabilities = request->capInstances;

    CIMException cimException;
    Array<SubscriptionWithSrcNamespace> subscriptions;
    Array<ProviderClassList> indicationProviders;

    //
    //  Get class name, namespace names, and property list
    //  from each capability instance
    //
    Uint32 numCapabilities = capabilities.size ();
    for (Uint32 i = 0; i < numCapabilities; i++)
    {
        CIMName className;
        Array<CIMNamespaceName> namespaceNames;
        CIMPropertyList propertyList;
        Array<SubscriptionWithSrcNamespace> currentSubscriptions;

        try
        {
            String cName;
            capabilities[i].getProperty (capabilities[i].findProperty
                (_PROPERTY_CLASSNAME)).getValue ().get (cName);
            className = CIMName (cName);

            Array<String> nsNames;
            capabilities[i].getProperty (capabilities[i].findProperty
                (_PROPERTY_NAMESPACES)).getValue ().get (nsNames);
            for (Uint32 j = 0; j < nsNames.size (); j++)
            {
                namespaceNames.append (CIMNamespaceName (nsNames[j]));
            }

            Array<String> pNames;
            Array<CIMName> propertyNames;
            Uint32 propertiesIndex = capabilities[i].findProperty
                (_PROPERTY_SUPPORTEDPROPERTIES);
            if (propertiesIndex != PEG_NOT_FOUND)
            {
                CIMValue propertiesValue = capabilities[i].getProperty
                    (propertiesIndex).getValue ();
                //
                //  If the property list is not null, set the property names
                //
                if (!propertiesValue.isNull ())
                {
                    propertiesValue.get (pNames);
                    for (Uint32 k = 0; k < pNames.size (); k++)
                    {
                        propertyNames.append (CIMName (pNames[k]));
                    }
                    propertyList.set (propertyNames);
                }
            }
        }
        catch (Exception& exception)
        {
            //
            //  Error getting information from Capabilities instance
            //
            PEG_TRACE((TRC_INDICATION_SERVICE, Tracer::LEVEL1,
               "Exception caught in handling provider enable notification: %s",
                (const char*)exception.getMessage().getCString()));

            cimException = CIMException(CIM_ERR_FAILED, exception.getMessage());
            break;
        }
        catch (...)
        {
            PEG_TRACE_CSTRING (TRC_INDICATION_SERVICE, Tracer::LEVEL1,
               "Error in handling provider enable notification");

            cimException = PEGASUS_CIM_EXCEPTION_L(
                CIM_ERR_FAILED,
                MessageLoaderParms(
                    "IndicationService.IndicationService.UNKNOWN_ERROR",
                    "Unknown Error"));
            break;
        }

        //
        //  Get matching subscriptions
        //
        currentSubscriptions = _getMatchingSubscriptions
            (className, namespaceNames, propertyList);

        for (Uint32 c = 0; c < currentSubscriptions.size (); c++)
        {
            Boolean inList = false;

            for (Uint32 m = 0; m < subscriptions.size (); m++)
            {
                //
                //  If the current subscription is already in the list of
                //  matching subscriptions, add the current class to the
                //  indication provider class list for the subscription
                //
                if (currentSubscriptions[c].subscription.identical(
                    subscriptions[m].subscription))
                {
                    for (Uint32 i = 0,
                        n = indicationProviders[m].classList.size();
                        i < n; ++i)
                    {
                        if (indicationProviders[m].classList[i].nameSpace
                            == currentSubscriptions[c].nameSpace)
                        {
                            indicationProviders[m].classList[i].
                                classList.append(className);
                            inList = true;
                            break;
                        }
                    }
                    if (!inList)
                    {
                        NamespaceClassList nscl;
                        nscl.nameSpace = currentSubscriptions[c].nameSpace;
                        nscl.classList.append(className);
                        indicationProviders[m].classList.append(nscl);
                        inList = true;
                    }
                    break;
                }
            }

            if (!inList)
            {
                //
                //  If the current subscription is not already in the list of
                //  matching subscriptions, add it to the list and add the
                //  indication provider class list for the subscription
                //
                subscriptions.append (currentSubscriptions[c]);
                ProviderClassList indicationProvider;
                indicationProvider.provider = provider;
                indicationProvider.providerModule = providerModule;
                NamespaceClassList nscl;
                nscl.classList.append(className);
                nscl.nameSpace = currentSubscriptions[c].nameSpace;
                indicationProvider.classList.append (nscl);
#ifdef PEGASUS_ENABLE_REMOTE_CMPI
                indicationProvider.isRemoteNameSpace = isRemoteNameSpace;
                indicationProvider.remoteInfo = remoteInfo;
#endif
                indicationProviders.append (indicationProvider);
            }
        }
    }  //  for each capability instance

    if (subscriptions.size () > 0)
    {
        CIMPropertyList requiredProperties;
        String condition;
        String query;
        String queryLanguage;

        //
        //  Get Provider Name
        //
        String logString1 = getProviderLogString (provider);

        for (Uint32 s = 0; s < subscriptions.size (); s++)
        {
            Array<CIMNamespaceName> sourceNamespaces;
            Array<NamespaceClassList> indicationSubclasses;
            CIMInstance instance = subscriptions[s].subscription;
            _getCreateParams (
                instance,
                indicationSubclasses,
                requiredProperties,
                condition,
                query,
                queryLanguage);

            //
            //  NOTE: These Create requests are not associated with a
            //  user request, so there is no associated authType or userName
            //  The Creator from the subscription instance is used for
            //  userName, and authType is not set
            //
            //  NOTE: the subscriptions in the subscriptions list came from
            //  the IndicationService's internal hash tables, and thus
            //  each instance is known to have a valid Creator property
            //
            String creator = instance.getProperty (instance.findProperty
                (PEGASUS_PROPERTYNAME_INDSUB_CREATOR)).getValue
                ().toString ();

            AcceptLanguageList acceptLangs;
            Uint32 propIndex = instance.findProperty
                (PEGASUS_PROPERTYNAME_INDSUB_ACCEPTLANGS);
            if (propIndex != PEG_NOT_FOUND)
            {
                String acceptLangsString;
                instance.getProperty(propIndex).getValue().get(
                    acceptLangsString);
                if (acceptLangsString.size())
                {
                    acceptLangs = LanguageParser::parseAcceptLanguageHeader(
                        acceptLangsString);
                }
            }
            ContentLanguageList contentLangs;
            propIndex = instance.findProperty
                (PEGASUS_PROPERTYNAME_INDSUB_CONTENTLANGS);
            if (propIndex != PEG_NOT_FOUND)
            {
                String contentLangsString;
                instance.getProperty(propIndex).getValue().get(
                    contentLangsString);
                if (contentLangsString.size())
                {
                    contentLangs = LanguageParser::parseContentLanguageHeader(
                        contentLangsString);
                }
            }

            //
            //  Send Create requests
            //
            Array<ProviderClassList> currentIndicationProviders;
            currentIndicationProviders.append (indicationProviders[s]);
            Array<ProviderClassList> acceptedProviders;
            acceptedProviders = _sendWaitCreateRequests(
                currentIndicationProviders,
                requiredProperties,
                condition,
                query,
                queryLanguage,
                instance,
                acceptLangs,
                contentLangs,
                creator);

            if (acceptedProviders.size () > 0)
            {
                //
                //  Get Subscription entry from Active Subscriptions table
                //
                ActiveSubscriptionsTableEntry tableValue;
                if (_subscriptionTable->getSubscriptionEntry
                    (instance.getPath (), tableValue))
                {
                    //
                    //  Look for the provider in the subscription's list
                    //
                    Uint32 providerIndex =
                        _subscriptionTable->providerInList
                            (indicationProviders[s].provider, tableValue);
                    if (providerIndex != PEG_NOT_FOUND)
                    {
                        //
                        //  Provider is already in the list for this
                        //  subscription; add class to provider class list
                        //
                        for (Uint32 cn = 0;
                             cn < indicationProviders[s].classList.size ();
                             cn++)
                        {
                            for(Uint32 sn = 0;
                                sn < indicationProviders[s].classList[cn].
                                    classList.size();
                                ++sn)
                            {
                                _subscriptionTable->updateClasses(
                                    instance.getPath (),
                                    indicationProviders[s].provider,
                                    indicationProviders[s].classList[cn].
                                        nameSpace,
                                    indicationProviders[s].classList[cn].
                                        classList[sn]);
                            }
                        }
                    }
                    else
                    {
                        //
                        //  Provider is not yet in the list for this
                        //  subscription; add provider to the list
                        //
                        _subscriptionTable->updateProviders
                            (instance.getPath (), indicationProviders[s],
                            true);

                        //
                        //  NOTE: When a provider that was previously not
                        //  serving a subscription now serves the
                        //  subscription due to a provider being enabled, a
                        //  log message is sent, even if there were
                        //  previously other providers serving the
                        //  subscription
                        //

                        //
                        //  Get Subscription Filter Name and Handler Name
                        //
                        String logString2 = _getSubscriptionLogString
                            (subscriptions[s].subscription);

                        //
                        //  Log a message for each subscription
                        //
                        Logger::put_l(Logger::STANDARD_LOG,
                            System::CIMSERVER, Logger::WARNING,
                            MessageLoaderParms(
                                _MSG_PROVIDER_NOW_SERVING_KEY,
                                _MSG_PROVIDER_NOW_SERVING,
                                logString1, logString2,
                                subscriptions[s].subscription.getPath().
                                    getNameSpace().getString()));
                    }
                }
            }  //  if any provider accepted the create subscription request
        }  //  for each matching subscription
    }  //  if any matching subscriptions

    //
    //  Send response
    //
    CIMResponseMessage * response = request->buildResponse ();
    response->cimException = cimException;
    _enqueueResponse (request, response);

    PEG_METHOD_EXIT ();
}

void IndicationService::_handleNotifyProviderFailRequest
    (Message * message)
{
    PEG_METHOD_ENTER (TRC_INDICATION_SERVICE,
        "IndicationService::_handleNotifyProviderFailRequest");

    CIMNotifyProviderFailRequestMessage* request =
        dynamic_cast<CIMNotifyProviderFailRequestMessage*>(message);
    PEGASUS_ASSERT(request != 0);

    String moduleName = request->moduleName;
    String userName = request->userName;

    //
    //  Determine providers in module that were serving active subscriptions
    //  and update the Active Subscriptions Table
    //
    Array<ActiveSubscriptionsTableEntry> providerModuleSubscriptions =
        _subscriptionTable->reflectProviderModuleFailure
            (moduleName, userName, _authenticationEnabled);

#ifdef PEGASUS_ENABLE_INDICATION_COUNT
    _providerIndicationCountTable.removeModuleEntries(moduleName);
#endif

    //
    //  FUTURE: Attempt to recreate the subscription state
    //

    //
    //  Send response
    //
    CIMResponseMessage * response = request->buildResponse ();
    CIMNotifyProviderFailResponseMessage * failResponse =
        (CIMNotifyProviderFailResponseMessage *) response;
    failResponse->numSubscriptionsAffected =
        providerModuleSubscriptions.size ();
    _enqueueResponse (request, response);
}

Boolean IndicationService::_canCreate (
    CIMInstance& instance,
    const CIMNamespaceName& nameSpace)
{
    PEG_METHOD_ENTER (TRC_INDICATION_SERVICE, "IndicationService::_canCreate");

    // REVIEW: Derived classes of CIM_IndicationSubscription not
    // handled. It is reasonable for a user to derive from this
    // class and add extra properties.

    // REVIEW: how does the provider manager know to forward
    // requests to this service? Is it by class name? If so,
    // shouldn't the provider use an is-a operator on the new
    // class?

    //
    //  Check all required properties exist
    //  For a property that has a default value, if it does not exist or is
    //  null, add or set property with default value
    //  For a property that has a specified set of valid values, validate
    //
    if ((instance.getClassName ().equal (PEGASUS_CLASSNAME_INDSUBSCRIPTION)) ||
        (instance.getClassName ().equal
            (PEGASUS_CLASSNAME_FORMATTEDINDSUBSCRIPTION)))
    {
        //
        //  Filter and Handler are key properties for Subscription
        //  No other properties are required
        //
        _checkRequiredProperty(
            instance,
            PEGASUS_PROPERTYNAME_FILTER,
            CIMTYPE_REFERENCE,
            true);
        _checkRequiredProperty(
            instance,
            PEGASUS_PROPERTYNAME_HANDLER,
            CIMTYPE_REFERENCE,
            true);

        //
        //  Get filter and handler property values
        //
        CIMProperty filterProperty = instance.getProperty
            (instance.findProperty (PEGASUS_PROPERTYNAME_FILTER));
        CIMValue filterValue = filterProperty.getValue ();
        CIMObjectPath filterPath;
        filterValue.get (filterPath);

        CIMProperty handlerProperty = instance.getProperty
            (instance.findProperty (PEGASUS_PROPERTYNAME_HANDLER));
        CIMValue handlerValue = handlerProperty.getValue ();
        CIMObjectPath handlerPath;
        handlerValue.get (handlerPath);

        //
        //  Currently, the Indication Service requires that a Subscription
        //  instance and the Filter and Handler instances to which it refers
        //  all be created on the same Host.
        //  Developers are recommended NOT to include Host in the
        //  Filter or Handler reference property values.
        //

        //
        //  If Host is included in a Filter or Handler reference property
        //  value, attempt to validate that it is correct.
        //  If Host cannot be validated, reject the create operation.
        //
        CIMObjectPath origFilterPath = filterPath;
        if (filterPath.getHost () != String::EMPTY)
        {
            if (!System::isLocalHost (filterPath.getHost()))
            {
                //
                //  Reject subscription creation
                //
                PEG_METHOD_EXIT();
                throw PEGASUS_CIM_EXCEPTION_L(
                    CIM_ERR_INVALID_PARAMETER,
                    MessageLoaderParms(
                        _MSG_INVALID_VALUE_FOR_PROPERTY_KEY,
                        _MSG_INVALID_VALUE_FOR_PROPERTY,
                        origFilterPath.toString(),
                        PEGASUS_PROPERTYNAME_FILTER.getString()));
            }
        }
        // Set SystemName key property to empty
        try
        {
            IndicationService::_setSystemNameInHandlerFilter(
                filterPath,
                String::EMPTY);
            filterValue.set(filterPath);
            filterProperty.setValue(filterValue);
        }
        catch(Exception &)
        {
            PEG_METHOD_EXIT();
            throw PEGASUS_CIM_EXCEPTION_L(
                CIM_ERR_INVALID_PARAMETER,
                MessageLoaderParms(
                    _MSG_INVALID_VALUE_FOR_PROPERTY_KEY,
                    _MSG_INVALID_VALUE_FOR_PROPERTY,
                    origFilterPath.toString(),
                    PEGASUS_PROPERTYNAME_FILTER.getString()));
        }

        CIMObjectPath origHandlerPath = handlerPath;
        if (handlerPath.getHost () != String::EMPTY)
        {
            if (!System::isLocalHost (handlerPath.getHost()))
            {
                //
                //  Reject subscription creation
                //
                PEG_METHOD_EXIT();
                throw PEGASUS_CIM_EXCEPTION_L(
                    CIM_ERR_INVALID_PARAMETER,
                    MessageLoaderParms(
                        _MSG_INVALID_VALUE_FOR_PROPERTY_KEY,
                        _MSG_INVALID_VALUE_FOR_PROPERTY,
                        origHandlerPath.toString(),
                        PEGASUS_PROPERTYNAME_HANDLER.getString()));
            }
        }
        // Set SystemName key property to empty
        try
        {
            IndicationService::_setSystemNameInHandlerFilter(
                handlerPath,
                String::EMPTY);
            handlerValue.set(handlerPath);
            handlerProperty.setValue(handlerValue);
        }
        catch(Exception &)
        {
            PEG_METHOD_EXIT();
            throw PEGASUS_CIM_EXCEPTION_L(
                CIM_ERR_INVALID_PARAMETER,
                MessageLoaderParms(
                    _MSG_INVALID_VALUE_FOR_PROPERTY_KEY,
                    _MSG_INVALID_VALUE_FOR_PROPERTY,
                    origHandlerPath.toString(),
                    PEGASUS_PROPERTYNAME_HANDLER.getString()));
        }

        //
        //  Get Filter namespace - if not set in Filter reference property
        //  value, namespace is the namespace of the subscription
        //
        CIMNamespaceName filterNS = filterPath.getNameSpace ();
        if (filterNS.isNull ())
        {
            filterNS = nameSpace;
        }

        //
        //  Get Handler namespace - if not set in Handler reference property
        //  value, namespace is the namespace of the subscription
        //
        CIMNamespaceName handlerNS = handlerPath.getNameSpace();
        if (handlerNS.isNull())
        {
            handlerNS = nameSpace;
        }

        //
        //  Validate the Filter and Handler reference properties
        //  Ensure Filter and Handler instances can be retrieved from the
        //  repository
        //
        CIMInstance filterInstance =
            _subscriptionRepository->getInstance(filterNS, filterPath,
                false, false, CIMPropertyList());

        CIMInstance handlerInstance =
            _subscriptionRepository->getInstance(handlerNS, handlerPath,
                false, false, CIMPropertyList());

        //
        //  Set the key bindings in the subscription instance
        //
        Array<CIMKeyBinding> kb;
        kb.append(CIMKeyBinding(PEGASUS_PROPERTYNAME_FILTER, filterValue));
        kb.append(CIMKeyBinding(PEGASUS_PROPERTYNAME_HANDLER, handlerValue));

        CIMObjectPath instanceRef = instance.getPath ();
        instanceRef.setKeyBindings(kb);
        instanceRef.setNameSpace(nameSpace);
        instance.setPath(instanceRef);

        //
        //  Subscription State, Repeat Notification Policy, and On Fatal Error
        //  Policy properties each has a default value, a corresponding
        //  Other___ property, and a set of valid values
        //
        _checkPropertyWithOther(
            instance,
            PEGASUS_PROPERTYNAME_SUBSCRIPTION_STATE,
            _PROPERTY_OTHERSTATE,
            (Uint16) STATE_ENABLED,
            (Uint16) STATE_OTHER,
            _supportedStates);

        _checkPropertyWithOther(
            instance,
            _PROPERTY_REPEATNOTIFICATIONPOLICY,
            _PROPERTY_OTHERREPEATNOTIFICATIONPOLICY,
            (Uint16) _POLICY_NONE,
            (Uint16) _POLICY_OTHER,
            _supportedRepeatPolicies);

        _checkPropertyWithOther(
            instance,
            _PROPERTY_ONFATALERRORPOLICY,
            _PROPERTY_OTHERONFATALERRORPOLICY,
            (Uint16) _ERRORPOLICY_IGNORE,
            (Uint16) _ERRORPOLICY_OTHER,
            _supportedErrorPolicies);

        if (instance.getClassName().equal(
            PEGASUS_CLASSNAME_FORMATTEDINDSUBSCRIPTION))
        {
            Array<String> textFormatParams;
            CIMValue textFormatParamsValue;
            CIMClass indicationClass;

            // get TextFormatParameters from instance
            Uint32 textFormatParamsPos =
            instance.findProperty(_PROPERTY_TEXTFORMATPARAMETERS);

            if (textFormatParamsPos != PEG_NOT_FOUND)
            {
                textFormatParamsValue = instance.getProperty(
                    textFormatParamsPos).getValue();

                if (!textFormatParamsValue.isNull())
                {
                    textFormatParamsValue.get(textFormatParams);
                }
            }

            // get indication class
            indicationClass = _getIndicationClass (instance);

            String textFormatStr;
            CIMValue textFormatValue;

            // get TextFormatStr from instance
            Uint32 textFormatPos =
            instance.findProperty(_PROPERTY_TEXTFORMAT);

            if (textFormatPos != PEG_NOT_FOUND)
            {
                textFormatValue = instance.getProperty(
                    textFormatPos).getValue();

#if defined(PEGASUS_ENABLE_SYSTEM_LOG_HANDLER) || \
    defined(PEGASUS_ENABLE_EMAIL_HANDLER)
                // if the value of textFormat is not null
                if (!(textFormatValue.isNull()) &&
                    (textFormatValue.getType() == CIMTYPE_STRING) &&
                    !(textFormatValue.isArray()))
                {
                    textFormatValue.get(textFormatStr);

                    // Validates the syntax and the provided type for the
                    // property TextFormat
                    IndicationFormatter::validateTextFormat (
                        textFormatStr, indicationClass,
                        textFormatParams);

                    // Validates the property names in TextFormatParameters
                    Array<CIMNamespaceName> sourceNamespaces;
                    String query;
                    String queryLanguage;
                    String filterName;
                    CIMPropertyList propertyList;

                    //  Get filter properties
                    _subscriptionRepository->getFilterProperties (instance,
                        query, sourceNamespaces, queryLanguage, filterName);

                    //  Build the query expression from the filter query
                    QueryExpression queryExpression = _getQueryExpression(query,
                        queryLanguage, sourceNamespaces[0]);

                    // the select clause projection
                    propertyList = queryExpression.getPropertyList();

                    IndicationFormatter::validateTextFormatParameters(
                    propertyList, indicationClass, textFormatParams);
                }
#endif
            }
        }
    }
    else  // Filter or Handler
    {
        //
        //  Name, CreationClassName, SystemName, and SystemCreationClassName
        //  are key properties for Filter and Handler
        //  Name must exist
        //  If others do not exist, add and set to default
        //  If they exist but are NULL, set value to the default
        //  If they exist and are not NULL, validate the value
        //

        if (instance.getClassName ().equal (PEGASUS_CLASSNAME_INDFILTER))
        {
            _checkRequiredProperty(
                instance,
                PEGASUS_PROPERTYNAME_NAME,
                CIMTYPE_STRING,
                true);
        }
        else //Handler
        {
#ifdef PEGASUS_ENABLE_DMTF_INDICATION_PROFILE_SUPPORT
            // Name is an optional property for Handler. If Name key property
            // not found then set the Name value using GUID.
            _checkPropertyWithGuid(
                instance,
                PEGASUS_PROPERTYNAME_NAME);
#else
            _checkRequiredProperty(
                instance,
                PEGASUS_PROPERTYNAME_NAME,
                CIMTYPE_STRING,
                true);
#endif
        }

        _initOrValidateStringProperty(
            instance,
            PEGASUS_PROPERTYNAME_CREATIONCLASSNAME,
            instance.getClassName().getString());

        _setOrAddSystemNameInHandlerFilter(instance,String::EMPTY);

        _initOrValidateStringProperty(
            instance,
            _PROPERTY_SYSTEMCREATIONCLASSNAME,
            System::getSystemCreationClassName());

        if (instance.getClassName ().equal (PEGASUS_CLASSNAME_INDFILTER))
        {
            //
            //  Query and QueryLanguage properties are required for Filter
            //
            _checkRequiredProperty(
                instance,
                PEGASUS_PROPERTYNAME_QUERY,
                CIMTYPE_STRING,
                false);
            _checkRequiredProperty(
                instance,
                PEGASUS_PROPERTYNAME_QUERYLANGUAGE,
                CIMTYPE_STRING,
                false);

            //
            //  Validate the query language is supported
            //
            String queryLanguage;
            instance.getProperty(
                instance.findProperty(PEGASUS_PROPERTYNAME_QUERYLANGUAGE)).
                    getValue().get(queryLanguage);

#ifndef PEGASUS_ENABLE_CQL
            // Special code to block CQL, if CQL is disabled
            if ((queryLanguage == "CIM:CQL") || (queryLanguage == "DMTF:CQL"))
            {
                // CQL is not allowed in this case
                PEG_METHOD_EXIT();
                throw PEGASUS_CIM_EXCEPTION(
                    CIM_ERR_NOT_SUPPORTED, queryLanguage);
            }
#endif
            Array<CIMNamespaceName> sourceNamespaces;

            _checkProperty(
                instance,
                _PROPERTY_SOURCENAMESPACE,
                CIMTYPE_STRING);

            _checkProperty(
                instance,
                _PROPERTY_SOURCENAMESPACES,
                CIMTYPE_STRING,
                true);

            _subscriptionRepository->getSourceNamespaces(
                instance,
                nameSpace,
                sourceNamespaces);

            if (sourceNamespaces.size() == 1 &&
                nameSpace == sourceNamespaces[0])
            {
                _checkPropertyWithDefault(
                    instance,
                    _PROPERTY_SOURCENAMESPACE,
                    nameSpace.getString());
            }

            //
            //  Validate the query and indication class name
            //  An exception is thrown if the query is invalid or the class
            //  is not an indication class
            //
            String filterQuery = instance.getProperty (instance.findProperty
                (PEGASUS_PROPERTYNAME_QUERY)).getValue ().toString ();

            for (Uint32 i = 0, n = sourceNamespaces.size(); i < n; ++i)
            {
                QueryExpression queryExpression;
                try
                {
                    queryExpression = _getQueryExpression(
                        filterQuery,
                        queryLanguage,
                        sourceNamespaces[i]);
                }
                catch (QueryLanguageInvalidException&)
                {
                    // The filter query had an invalid language name.
                    PEG_METHOD_EXIT();
                    throw PEGASUS_CIM_EXCEPTION(
                        CIM_ERR_NOT_SUPPORTED, queryLanguage);
                }

                CIMName indicationClassName = _getIndicationClassName(
                    queryExpression,
                    sourceNamespaces[i]);

                //
                // Make sure that the FROM class exists in the repository.
                //
                CIMClass indicationClass = _subscriptionRepository->getClass(
                    sourceNamespaces[i],
                    indicationClassName,
                    false,
                    false,
                    false,
                    CIMPropertyList ());

                //
                // Validate all the properties in the SELECT statement exist
                // on their class context.
                //
                try
                {
                    queryExpression.validate();
                }
                catch (QueryMissingPropertyException& qmp)
                {
                    // A property does not exist on the class it is scoped to.
                    PEG_METHOD_EXIT();
                    throw PEGASUS_CIM_EXCEPTION(
                        CIM_ERR_INVALID_PARAMETER,
                        qmp.getMessage());
                }
                catch (QueryValidationException& qv)
                {
                    // Received some other validation error.
                    // This includes detecting an array property
                    // is in the WHERE list for WQL.
                    PEG_METHOD_EXIT();
                    throw PEGASUS_CIM_EXCEPTION(
                        CIM_ERR_NOT_SUPPORTED,
                        qv.getMessage());
                }
            }
        }

        //
        //  Currently only seven subclasses of the Listener Destination
        //  class are supported -- further subclassing is not currently
        //  supported
        //
        else if ((instance.getClassName ().equal
                  (PEGASUS_CLASSNAME_INDHANDLER_CIMXML)) ||
                 (instance.getClassName ().equal
                  (PEGASUS_CLASSNAME_LSTNRDST_CIMXML)) ||
         (instance.getClassName ().equal
          (PEGASUS_CLASSNAME_LSTNRDST_SYSTEM_LOG)) ||
         (instance.getClassName ().equal
          (PEGASUS_CLASSNAME_LSTNRDST_EMAIL)) ||
                 (instance.getClassName ().equal
                  (PEGASUS_CLASSNAME_INDHANDLER_SNMP)) ||
                 (instance.getClassName ().equal
                  (PEGASUS_CLASSNAME_INDHANDLER_WSMAN)) ||
                 (instance.getClassName ().equal
                  (PEGASUS_CLASSNAME_LSTNRDST_FILE)))

        {

#ifndef PEGASUS_ENABLE_SYSTEM_LOG_HANDLER
            if (instance.getClassName ().equal
            (PEGASUS_CLASSNAME_LSTNRDST_SYSTEM_LOG))
            {
                //
                //  The System Log Handler is not enabled currently,
                //  this class is not currently served by the Indication Service
                //
                PEG_METHOD_EXIT ();

                throw PEGASUS_CIM_EXCEPTION_L (CIM_ERR_NOT_SUPPORTED,
                MessageLoaderParms(_MSG_CLASS_NOT_SERVED_KEY,
            _MSG_CLASS_NOT_SERVED));
        }
#endif

#if !defined(PEGASUS_ENABLE_EMAIL_HANDLER)

            if (instance.getClassName ().equal
            (PEGASUS_CLASSNAME_LSTNRDST_EMAIL))
            {
                //
                //  The Email Handler is not enabled currently,
                //  this class is not currently served by the Indication Service
                //
                PEG_METHOD_EXIT ();

                throw PEGASUS_CIM_EXCEPTION_L (CIM_ERR_NOT_SUPPORTED,
                MessageLoaderParms(_MSG_CLASS_NOT_SERVED_KEY,
                   _MSG_CLASS_NOT_SERVED));
        }
#endif

#ifndef PEGASUS_ENABLE_PROTOCOL_WSMAN
            if (instance.getClassName ().equal
            (PEGASUS_CLASSNAME_INDHANDLER_WSMAN))
            {
                //
                //  The WSMAN Handler is not enabled currently,
                //  this class is not currently served by the Indication Service
                //
                PEG_METHOD_EXIT ();

                throw PEGASUS_CIM_EXCEPTION_L (CIM_ERR_NOT_SUPPORTED,
                MessageLoaderParms(_MSG_CLASS_NOT_SERVED_KEY,
            _MSG_CLASS_NOT_SERVED));
        }
#endif
            _checkPropertyWithOther(
                instance,
                PEGASUS_PROPERTYNAME_PERSISTENCETYPE,
                _PROPERTY_OTHERPERSISTENCETYPE,
                (Uint16) PERSISTENCE_PERMANENT,
                (Uint16) PERSISTENCE_OTHER,
                _supportedPersistenceTypes);

            if (instance.getClassName().equal(
                    PEGASUS_CLASSNAME_INDHANDLER_CIMXML) ||
                instance.getClassName().equal(
                    PEGASUS_CLASSNAME_LSTNRDST_CIMXML) ||
                instance.getClassName ().equal(
                    PEGASUS_CLASSNAME_INDHANDLER_WSMAN))
            {
                //
                //  Destination property is required for CIMXML
                //  Handler subclass
                //
                _checkRequiredProperty(
                    instance,
                    PEGASUS_PROPERTYNAME_LSTNRDST_DESTINATION,
                    CIMTYPE_STRING,
                    false);
            }

            // WSMAN Indication Handler properties are checked below
            if (instance.getClassName().equal
                (PEGASUS_CLASSNAME_INDHANDLER_WSMAN))
            {
                // Delivery Mode property is required for WSMAN Handler
                _checkRequiredProperty(
                    instance,
                    PEGASUS_PROPERTYNAME_WSM_DELIVERY_MODE,
                    CIMTYPE_UINT16,
                    false);
            }

            if (instance.getClassName().equal
                (PEGASUS_CLASSNAME_INDHANDLER_SNMP))
            {
                //
                //  TargetHost property is required for SNMP
                //  Handler subclass
                //
                _checkRequiredProperty(
                    instance,
                    PEGASUS_PROPERTYNAME_LSTNRDST_TARGETHOST,
                    CIMTYPE_STRING,
                    false);

                //
                //  TargetHostFormat property is required for SNMP
                //  Handler subclass
                //
                _checkRequiredProperty(
                    instance,
                    _PROPERTY_TARGETHOSTFORMAT,
                    CIMTYPE_UINT16,
                    false);

                //
                //  SNMPVersion property is required for SNMP Handler
                //
                _checkRequiredProperty(
                    instance,
                    PEGASUS_PROPERTYNAME_SNMPVERSION,
                    CIMTYPE_UINT16,
                    false);

                // Currently, only SNMPv1 trap and SNMPv2C trap are supported,
                // verify if the value of SNMPVersion is one of them

                _checkValue(
                    instance,
                    PEGASUS_PROPERTYNAME_SNMPVERSION,
                    _supportedSNMPVersion);
            }

            if (instance.getClassName().equal
                (PEGASUS_CLASSNAME_LSTNRDST_FILE))
            {
                // Checks for required file path
                _checkRequiredProperty(
                    instance,
                    PEGASUS_PROPERTYNAME_LSTNRDST_FILE,
                    CIMTYPE_STRING,
                    false);
            }

            if (instance.getClassName().equal
                (PEGASUS_CLASSNAME_LSTNRDST_EMAIL))
            {
                //
                //  MailTo property is required for Email
                //  Handler subclass
                //
                _checkRequiredProperty(
                    instance,
                    PEGASUS_PROPERTYNAME_LSTNRDST_MAILTO,
                    CIMTYPE_STRING,
                    false,
                    true);

                // get MailTo from handler instance
                Array<String> mailTo;
                instance.getProperty(instance.findProperty(
                    PEGASUS_PROPERTYNAME_LSTNRDST_MAILTO)).getValue().get(
                        mailTo);

                // Build mail address string
                String mailAddrStr;
                Uint32 mailAddrSize = mailTo.size();

                for (Uint32 i=0; i < mailAddrSize; i++)
                {
                    mailAddrStr.append(mailTo[i]);

                    if (i < (mailAddrSize - 1))
                    {
                        mailAddrStr.append(",");
                    }
                }


                //
                // Email address can not be an empty string
                //
                if (mailAddrStr == String::EMPTY)
                {
                    PEG_METHOD_EXIT();
                    throw PEGASUS_CIM_EXCEPTION_L(
                        CIM_ERR_FAILED,
                        MessageLoaderParms(
                            "IndicationService.IndicationService."
                                "_MSG_DO_NOT_HAVE_EMAIL_ADDRESS",
                            "Do not have an e-mail address."));
                }

                //
                //  MailSubject property is required for Email
                //  Handler subclass
                //
                _checkRequiredProperty(
                    instance,
                    PEGASUS_PROPERTYNAME_LSTNRDST_MAILSUBJECT,
                    CIMTYPE_STRING,
                    false);
            }
        }
        else
        {
            //
            //  A class not currently served by the Indication Service
            //
            PEG_METHOD_EXIT();

            throw PEGASUS_CIM_EXCEPTION_L(
                CIM_ERR_NOT_SUPPORTED,
                MessageLoaderParms(
                    _MSG_CLASS_NOT_SERVED_KEY,
                    _MSG_CLASS_NOT_SERVED));
        }
    }

    PEG_METHOD_EXIT();
    return true;
}

void IndicationService::_checkRequiredProperty(
    CIMInstance& instance,
    const CIMName& propertyName,
    const CIMType expectedType,
    Boolean isKeyProperty,
    Boolean isArray)
{
    PEG_METHOD_ENTER (TRC_INDICATION_SERVICE,
        "IndicationService::_checkRequiredProperty");

    Boolean missingProperty = false;

    //
    //  Required property must exist in instance
    //
    if (instance.findProperty (propertyName) == PEG_NOT_FOUND)
    {
        missingProperty = true;
    }
    else
    {
        //
        //  Get the property
        //
        CIMProperty theProperty = instance.getProperty
            (instance.findProperty (propertyName));
        CIMValue theValue = theProperty.getValue ();

        //
        //  Required property must have a non-null value
        //
        if (theValue.isNull ())
        {
            missingProperty = true;
        }
        else
        {
            //
            //  Check that the property value is of the correct type
            //
            if ((theValue.getType () != expectedType) ||
                (theValue.isArray () != isArray))
            {
                if (theValue.isArray ())
                {
                    PEG_METHOD_EXIT();
                    throw PEGASUS_CIM_EXCEPTION_L(CIM_ERR_INVALID_PARAMETER,
                        MessageLoaderParms(
                            _MSG_INVALID_TYPE_ARRAY_OF_FOR_PROPERTY_KEY,
                            _MSG_INVALID_TYPE_ARRAY_OF_FOR_PROPERTY,
                            cimTypeToString(theValue.getType()),
                            propertyName.getString()));
                }
                else
                {
                    PEG_METHOD_EXIT();
                    throw PEGASUS_CIM_EXCEPTION_L(CIM_ERR_INVALID_PARAMETER,
                        MessageLoaderParms(
                            _MSG_INVALID_TYPE_FOR_PROPERTY_KEY,
                            _MSG_INVALID_TYPE_FOR_PROPERTY,
                            cimTypeToString(theValue.getType()),
                            propertyName.getString()));
                }
            }
        }
    }

    if (missingProperty)
    {
        if (isKeyProperty)
        {
            PEG_METHOD_EXIT();
            throw PEGASUS_CIM_EXCEPTION_L(
                CIM_ERR_INVALID_PARAMETER,
                MessageLoaderParms(
                    "IndicationService.IndicationService._MSG_KEY_PROPERTY",
                    "The key property $0 is missing.",
                    propertyName.getString()));
        }
        else
        {
            PEG_METHOD_EXIT();
            throw PEGASUS_CIM_EXCEPTION_L(
                CIM_ERR_INVALID_PARAMETER,
                MessageLoaderParms(
                    _MSG_PROPERTY_KEY,
                    _MSG_PROPERTY,
                    propertyName.getString()));
        }
    }

    PEG_METHOD_EXIT ();
}

void IndicationService::_checkPropertyWithOther (
    CIMInstance& instance,
    const CIMName& propertyName,
    const CIMName& otherPropertyName,
    const Uint16 defaultValue,
    const Uint16 otherValue,
    const Array<Uint16>& supportedValues)
{
    PEG_METHOD_ENTER (TRC_INDICATION_SERVICE,
        "IndicationService::_checkPropertyWithOther");

    Uint16 result = defaultValue;

    //
    //  If the property doesn't exist, add it with the default value
    //
    if (instance.findProperty (propertyName) == PEG_NOT_FOUND)
    {
        instance.addProperty (CIMProperty (propertyName,
            CIMValue (defaultValue)));
    }
    else
    {
        //
        //  Get the property
        //
        CIMProperty theProperty = instance.getProperty
            (instance.findProperty (propertyName));
        CIMValue theValue = theProperty.getValue ();

        //
        //  Check that the value is of the correct type
        //
        if ((theValue.getType () != CIMTYPE_UINT16) || (theValue.isArray ()))
        {
            String exceptionStr;
            if (theValue.isArray ())
            {
                MessageLoaderParms parms(
                    "IndicationService.IndicationService."
                        "_MSG_INVALID_TYPE_ARRAY_OF_FOR_PROPERTY",
                    "Invalid type array of $0 for property $1",
                    cimTypeToString(theValue.getType()),
                    propertyName.getString());

                exceptionStr.append(MessageLoader::getMessage(parms));
            }
            else
            {
                MessageLoaderParms parms(
                    "IndicationService.IndicationService."
                        "_MSG_INVALID_TYPE_FOR_PROPERTY",
                    "Invalid type $0 for property $1",
                    cimTypeToString(theValue.getType()),
                    propertyName.getString());

                exceptionStr.append(MessageLoader::getMessage(parms));
            }
            PEG_METHOD_EXIT ();
            throw PEGASUS_CIM_EXCEPTION (CIM_ERR_INVALID_PARAMETER,
                exceptionStr);
        }

        //
        //  If the value is null, set to the default value
        //
        if (theValue.isNull ())
        {
            theProperty.setValue (CIMValue (defaultValue));
        }
        else
        {
            theValue.get (result);
            //
            //  Check for valid values that are not supported
            //
            //  Note: Supported values are a subset of the valid values
            //  Some valid values, as defined in the MOF, are not currently
            //  supported by the Pegasus IndicationService
            //
            if (!Contains(supportedValues, result))
            {
                PEG_METHOD_EXIT();
                throw PEGASUS_CIM_EXCEPTION_L(
                    CIM_ERR_NOT_SUPPORTED,
                    MessageLoaderParms(
                        _MSG_UNSUPPORTED_VALUE_FOR_PROPERTY_KEY,
                        _MSG_UNSUPPORTED_VALUE_FOR_PROPERTY,
                        theValue.toString(),
                        propertyName.getString()));
            }
        }

        //
        //  If the value is Other, the Other
        //  property must exist, value must not be NULL and type must be String
        //
        if (result == otherValue)
        {
            if (instance.findProperty(otherPropertyName) == PEG_NOT_FOUND)
            {
                PEG_METHOD_EXIT();
                throw PEGASUS_CIM_EXCEPTION_L(
                    CIM_ERR_INVALID_PARAMETER,
                    MessageLoaderParms(
                        _MSG_PROPERTY_KEY,
                        _MSG_PROPERTY,
                        otherPropertyName.getString()));
            }
            else
            {
                CIMProperty otherProperty = instance.getProperty
                    (instance.findProperty(otherPropertyName));
                CIMValue theOtherValue = otherProperty.getValue();
                if (theOtherValue.isNull())
                {
                    PEG_METHOD_EXIT();
                    throw PEGASUS_CIM_EXCEPTION_L(
                        CIM_ERR_INVALID_PARAMETER,
                        MessageLoaderParms(
                            _MSG_PROPERTY_KEY,
                            _MSG_PROPERTY,
                            otherPropertyName.getString()));
                }
                else if (theOtherValue.getType() != CIMTYPE_STRING)
                {
                    //
                    //  Property exists and is not null,
                    //  but is not of correct type
                    //
                    PEG_METHOD_EXIT();
                    throw PEGASUS_CIM_EXCEPTION_L(
                        CIM_ERR_INVALID_PARAMETER,
                        MessageLoaderParms(
                            _MSG_INVALID_TYPE_FOR_PROPERTY_KEY,
                            _MSG_INVALID_TYPE_FOR_PROPERTY,
                            cimTypeToString(theOtherValue.getType()),
                            otherPropertyName.getString()));
                }
            }
        }

        //
        //  If value is not Other, Other property must not exist
        //  or must be NULL
        //
        else if (instance.findProperty (otherPropertyName) != PEG_NOT_FOUND)
        {
            CIMProperty otherProperty = instance.getProperty(
                instance.findProperty(otherPropertyName));
            CIMValue theOtherValue = otherProperty.getValue();
            if (!theOtherValue.isNull())
            {
                PEG_METHOD_EXIT();
                throw PEGASUS_CIM_EXCEPTION_L(
                    CIM_ERR_INVALID_PARAMETER,
                    MessageLoaderParms(
                        "IndicationService.IndicationService."
                            "_MSG_PROPERTY_PRESENT_BUT_VALUE_NOT",
                        "The $0 property is present, but the $1 value is "
                            "not $2.",
                        otherPropertyName.getString(),
                        propertyName.getString(),
                        CIMValue(otherValue).toString()));
            }
        }
    }

    PEG_METHOD_EXIT();
}

String IndicationService::_checkPropertyWithGuid(
    CIMInstance& instance,
    const CIMName& propertyName)
{
    PEG_METHOD_ENTER(TRC_INDICATION_SERVICE,
        "IndicationService::_checkPropertyWithGuid");

   String value = _checkPropertyWithDefault(
       instance,
       propertyName,
       Guid::getGuid(PEGASUS_INSTANCEID_GLOBAL_PREFIX));

   PEG_METHOD_EXIT();

   return value;
}

String IndicationService::_checkPropertyWithDefault(
    CIMInstance& instance,
    const CIMName& propertyName,
    const String& defaultValue)
{
    PEG_METHOD_ENTER(TRC_INDICATION_SERVICE,
        "IndicationService::_checkPropertyWithDefault");

    String result = defaultValue;

    //
    //  If the property doesn't exist, add it with the default value
    //
    if (instance.findProperty (propertyName) == PEG_NOT_FOUND)
    {
        instance.addProperty (CIMProperty (propertyName,
            CIMValue (defaultValue)));
    }
    else
    {
        //
        //  Get the property
        //
        CIMProperty theProperty = instance.getProperty
            (instance.findProperty (propertyName));
        CIMValue theValue = theProperty.getValue ();

        //
        //  If the value is null, set to the default value
        //
        if (theValue.isNull ())
        {
            theProperty.setValue (CIMValue (defaultValue));
        }
        else if ((theValue.getType () != CIMTYPE_STRING) ||
                 (theValue.isArray ()))
        {
            //
            //  Property exists and is not null,
            //  but is not of correct type
            //
            if (theValue.isArray ())
            {
                PEG_METHOD_EXIT();
                throw PEGASUS_CIM_EXCEPTION_L(
                    CIM_ERR_INVALID_PARAMETER,
                    MessageLoaderParms(
                        _MSG_INVALID_TYPE_ARRAY_OF_FOR_PROPERTY_KEY,
                        _MSG_INVALID_TYPE_ARRAY_OF_FOR_PROPERTY,
                        cimTypeToString(theValue.getType()),
                        propertyName.getString()));
            }
            else
            {
                PEG_METHOD_EXIT();
                throw PEGASUS_CIM_EXCEPTION_L(
                    CIM_ERR_INVALID_PARAMETER,
                    MessageLoaderParms(
                        _MSG_INVALID_TYPE_FOR_PROPERTY_KEY,
                        _MSG_INVALID_TYPE_FOR_PROPERTY,
                        cimTypeToString(theValue.getType()),
                        propertyName.getString()));
            }
        }
        else
        {
            theValue.get (result);
        }
    }

    PEG_METHOD_EXIT ();
    return result;
}

String IndicationService::_initOrValidateStringProperty (
    CIMInstance& instance,
    const CIMName& propertyName,
    const String& defaultValue)
{
    PEG_METHOD_ENTER (TRC_INDICATION_SERVICE,
        "IndicationService::_initOrValidateStringProperty");

    String result = defaultValue;

    String propertyValue = _checkPropertyWithDefault (instance, propertyName,
        defaultValue);

    if (propertyValue != defaultValue)
    {
        // SNIA requires SystemCreationClassName to be
        // overridden with the correct values.
        if (propertyName == _PROPERTY_SYSTEMCREATIONCLASSNAME)
        {
            // The property must exist after _checkPropertyWithDefault is called
            CIMProperty p =
                instance.getProperty(instance.findProperty(propertyName));
            p.setValue(CIMValue(defaultValue));
            PEG_METHOD_EXIT();
            return result;
        }

        //
        //  Property value specified is invalid
        //
        PEG_METHOD_EXIT();
        throw PEGASUS_CIM_EXCEPTION_L(
            CIM_ERR_INVALID_PARAMETER,
            MessageLoaderParms(
                _MSG_INVALID_VALUE_FOR_PROPERTY_KEY,
                _MSG_INVALID_VALUE_FOR_PROPERTY,
                propertyValue,
                propertyName.getString()));
    }

    PEG_METHOD_EXIT ();
    return result;
}

void IndicationService::_checkProperty (
    CIMInstance& instance,
    const CIMName& propertyName,
    const CIMType expectedType,
    const Boolean isArray)
{
    PEG_METHOD_ENTER (TRC_INDICATION_SERVICE,
        "IndicationService::_checkProperty");

    //
    //  If the property exists, get it
    //
    Uint32 propPos = instance.findProperty (propertyName);
    if (propPos != PEG_NOT_FOUND)
    {
        CIMProperty theProperty = instance.getProperty (propPos);
        CIMValue theValue = theProperty.getValue ();

        //
        //  If the value is not null, check the type
        //
        if (!theValue.isNull ())
        {
            if ((theValue.getType () != expectedType) ||
                (theValue.isArray () != isArray))
            {
                //
                //  Property exists and is not null, but is not of correct type
                //
                if (theValue.isArray ())
                {
                    PEG_METHOD_EXIT();
                    throw PEGASUS_CIM_EXCEPTION_L(
                        CIM_ERR_INVALID_PARAMETER,
                        MessageLoaderParms(
                            _MSG_INVALID_TYPE_ARRAY_OF_FOR_PROPERTY_KEY,
                            _MSG_INVALID_TYPE_ARRAY_OF_FOR_PROPERTY,
                            cimTypeToString(theValue.getType()),
                            propertyName.getString()));
                }
                else
                {
                    PEG_METHOD_EXIT();
                    throw PEGASUS_CIM_EXCEPTION_L(
                        CIM_ERR_INVALID_PARAMETER,
                        MessageLoaderParms(
                            _MSG_INVALID_TYPE_FOR_PROPERTY_KEY,
                            _MSG_INVALID_TYPE_FOR_PROPERTY,
                            cimTypeToString(theValue.getType()),
                            propertyName.getString()));
                }
            }
        }
    }
    PEG_METHOD_EXIT ();
}

void IndicationService::_checkValue (
    const CIMInstance& instance,
    const CIMName& propertyName,
    const Array<Uint16>& supportedValues)
{
    PEG_METHOD_ENTER (TRC_INDICATION_SERVICE,
        "IndicationService::_checkValue");

    Uint16 theValue;

    // get the property value
    Uint32 propPos = instance.findProperty (propertyName);
    if (propPos != PEG_NOT_FOUND)
    {
        CIMValue propertyValue = (instance.getProperty(propPos)).getValue();

        if (!(propertyValue.isNull()))
        {
            propertyValue.get(theValue);

            // Check for valid values that are not supported
            // Note: Supported values are a subset of the valid values
            // Some valid values, as defined in the MOF, are not currently
            // supported
            if (!Contains(supportedValues, theValue))
            {
                PEG_METHOD_EXIT();
                throw PEGASUS_CIM_EXCEPTION_L(
                    CIM_ERR_NOT_SUPPORTED,
                    MessageLoaderParms(
                        _MSG_UNSUPPORTED_VALUE_FOR_PROPERTY_KEY,
                        _MSG_UNSUPPORTED_VALUE_FOR_PROPERTY,
                        theValue,
                        propertyName.getString()));
            }
        }
    }

    PEG_METHOD_EXIT ();
}

Boolean IndicationService::_canModify (
    const CIMModifyInstanceRequestMessage * request,
    const CIMObjectPath& instanceReference,
    const CIMInstance& instance,
    CIMInstance& modifiedInstance)
{
    PEG_METHOD_ENTER (TRC_INDICATION_SERVICE, "IndicationService::_canModify");

    //
    //  Currently, only modification allowed is of Subscription State
    //  property in Subscription class
    //
    if (!(instanceReference.getClassName ().equal
        (PEGASUS_CLASSNAME_INDSUBSCRIPTION)) &&
    !(instanceReference.getClassName ().equal
    (PEGASUS_CLASSNAME_FORMATTEDINDSUBSCRIPTION)))
    {
        PEG_METHOD_EXIT ();
        throw PEGASUS_CIM_EXCEPTION(CIM_ERR_NOT_SUPPORTED, String::EMPTY);
    }

    if (request->includeQualifiers)
    {
        PEG_METHOD_EXIT ();
        throw PEGASUS_CIM_EXCEPTION(CIM_ERR_NOT_SUPPORTED, String::EMPTY);
    }

    //
    //  Request is invalid if property list is null, meaning all properties
    //  are to be updated
    //
    if (request->propertyList.isNull ())
    {
        PEG_METHOD_EXIT ();
        throw PEGASUS_CIM_EXCEPTION(CIM_ERR_NOT_SUPPORTED, String::EMPTY);
    }

    //
    //  Request is invalid if more than one property is specified
    //
    else if (request->propertyList.size() > 1)
    {
        PEG_METHOD_EXIT();
        throw PEGASUS_CIM_EXCEPTION(CIM_ERR_NOT_SUPPORTED, String::EMPTY);
    }

    //
    //  For request to be valid, zero or one property must be specified
    //  If one property specified, it must be Subscription State property
    //
    else if ((request->propertyList.size() == 1) &&
             (!request->propertyList[0].equal(
                   PEGASUS_PROPERTYNAME_SUBSCRIPTION_STATE)))
    {
        PEG_METHOD_EXIT();
        throw PEGASUS_CIM_EXCEPTION(CIM_ERR_NOT_SUPPORTED, String::EMPTY);
    }

    //
    //  Check the SubscriptionState property in the modified instance
    //
    _checkPropertyWithOther(
        modifiedInstance,
        PEGASUS_PROPERTYNAME_SUBSCRIPTION_STATE,
        _PROPERTY_OTHERSTATE,
        (Uint16) STATE_ENABLED,
        (Uint16) STATE_OTHER,
        _supportedStates);

    //
    //  Get creator from instance
    //
    String creator;
    if (!_getCreator (instance, creator))
    {
        //
        //  This instance from the repository is corrupted
        //
        PEG_METHOD_EXIT ();
        MessageLoaderParms parms(_MSG_INVALID_INSTANCES_KEY,
            _MSG_INVALID_INSTANCES);
        throw PEGASUS_CIM_EXCEPTION_L(CIM_ERR_FAILED, parms);
    }

    //
    //  Current user must be privileged user or instance Creator to modify
    //  NOTE: if authentication was not turned on when instance was created,
    //  instance creator will be String::EMPTY
    //  If creator is String::EMPTY, anyone may modify or delete the
    //  instance
    //
    String currentUser = ((IdentityContainer)request->operationContext.get
        (IdentityContainer :: NAME)).getUserName();
    if ((creator.size() != 0) &&
#ifndef PEGASUS_OS_ZOS
        (!System::isPrivilegedUser (currentUser)) &&
        (currentUser != creator))
#else
        !String::equalNoCase(currentUser, creator))
#endif
    {
        PEG_METHOD_EXIT ();
        throw PEGASUS_CIM_EXCEPTION_L(
                        CIM_ERR_ACCESS_DENIED,
                        MessageLoaderParms(
                            _MSG_NOT_CREATOR_KEY,
                            _MSG_NOT_CREATOR,
                            currentUser,
                            creator));
    }

    PEG_METHOD_EXIT ();
    return true;
}

Boolean IndicationService::_canDelete (
    const CIMObjectPath& instanceReference,
    const CIMNamespaceName& nameSpace,
    const String& currentUser)
{
    PEG_METHOD_ENTER (TRC_INDICATION_SERVICE, "IndicationService::_canDelete");

    CIMName superClass;
    CIMName propName;

    //
    //  Get the instance to be deleted from the repository
    //
    CIMInstance instance;

    instance = _subscriptionRepository->getInstance
        (nameSpace, instanceReference);

    //
    //  Get creator from instance
    //
    String creator;
    if (!_getCreator (instance, creator))
    {
        //
        //  This instance from the repository is corrupted
        //  Allow the delete if a Privileged User
        //      (or authentication turned off),
        //  Otherwise disallow as access denied
        //
#ifndef PEGASUS_OS_ZOS
        if ((!System::isPrivilegedUser (currentUser)) &&
            (currentUser != String::EMPTY))
        {
            PEG_METHOD_EXIT ();
            throw PEGASUS_CIM_EXCEPTION (CIM_ERR_ACCESS_DENIED, String::EMPTY);
        }
#endif
    }

    //
    //  Current user must be privileged user or instance Creator to delete
    //  NOTE: if authentication was not turned on when instance was created,
    //  instance creator will be String::EMPTY
    //  If creator is String::EMPTY, anyone may modify or delete the
    //  instance
    //
    if ((creator.size() != 0) &&
#ifndef PEGASUS_OS_ZOS
        (!System::isPrivilegedUser (currentUser)) &&
        (currentUser != creator))
#else
        !String::equalNoCase(currentUser, creator))
#endif
    {
        PEG_METHOD_EXIT ();
        throw PEGASUS_CIM_EXCEPTION_L(
                        CIM_ERR_ACCESS_DENIED,
                        MessageLoaderParms(
                            _MSG_NOT_CREATOR_KEY,
                            _MSG_NOT_CREATOR,
                            currentUser,
                            creator));
    }

    //
    //  Get the class and superclass of the instance to be deleted
    //
    CIMClass refClass;

    refClass = _subscriptionRepository->getClass (nameSpace,
        instanceReference.getClassName (), true, true, false,
        CIMPropertyList ());
    superClass = refClass.getSuperClassName();

    //
    //  If the class is Filter or superclass is Handler or Listener Destination,
    //  check for subscription instances referring to the instance to be deleted
    //
    if ((superClass.equal (PEGASUS_CLASSNAME_INDHANDLER)) ||
        (superClass.equal (PEGASUS_CLASSNAME_LSTNRDST)) ||
        (instanceReference.getClassName().equal (PEGASUS_CLASSNAME_INDFILTER)))
    {
        if (instanceReference.getClassName ().equal
               (PEGASUS_CLASSNAME_INDFILTER))
        {
            propName = PEGASUS_PROPERTYNAME_FILTER;
        }
        else if ((superClass.equal (PEGASUS_CLASSNAME_INDHANDLER)) ||
            (superClass.equal (PEGASUS_CLASSNAME_LSTNRDST)))
        {
            propName = PEGASUS_PROPERTYNAME_HANDLER;

            //
            //  If deleting transient handler, first delete any referencing
            //  subscriptions
            //
            if (_subscriptionRepository->isTransient (nameSpace,
                instanceReference))
            {
                _deleteReferencingSubscriptions (nameSpace, propName,
                    instanceReference);
                PEG_METHOD_EXIT ();
                return true;
            }
        }

        //
        //  Get all the subscriptions from the repository
        //
        Array<CIMInstance> subscriptions =
            _subscriptionRepository->getAllSubscriptions ();

        CIMValue propValue;

        //
        //  Check each subscription for a reference to the instance to be
        //  deleted
        //
        for (Uint32 i = 0; i < subscriptions.size(); i++)
        {
            //
            //  Get the subscription Filter or Handler property value
            //
            propValue = subscriptions[i].getProperty
                (subscriptions[i].findProperty
                (propName)).getValue();

            CIMObjectPath ref;
            propValue.get (ref);

            //
            //  If the Filter or Handler reference property value includes
            //  namespace, check if it is the namespace of the Filter or Handler
            //  being deleted.
            //  If the Filter or Handler reference property value does not
            //  include namespace, check if the current subscription namespace
            //  is the namespace of the Filter or Handler being deleted.
            //
            CIMNamespaceName instanceNS = ref.getNameSpace ();
            if (((instanceNS.isNull ()) &&
                (subscriptions[i].getPath ().getNameSpace () == nameSpace))
                || (instanceNS == nameSpace))
            {

                //
                //  Remove Host and Namespace from reference property value, if
                //  present, before comparing
                //
                CIMObjectPath path ("", CIMNamespaceName (),
                    ref.getClassName (), ref.getKeyBindings ());

                //
                //  Remove Host and Namespace from reference of instance to be
                //  deleted, if present, before comparing
                //
                CIMObjectPath iref ("", CIMNamespaceName (),
                    instanceReference.getClassName (),
                    instanceReference.getKeyBindings ());

                //
                //  If the current subscription Filter or Handler is the
                //  instance to be deleted, it may not be deleted
                //
                if (iref == path)
                {
                    PEG_METHOD_EXIT ();
                    throw PEGASUS_CIM_EXCEPTION_L(CIM_ERR_FAILED,
                        MessageLoaderParms(
                            "IndicationService.IndicationService."
                                "_MSG_REFERENCED",
                            "A filter or handler referenced by a subscription "
                                "cannot be deleted."));
                }
            }
        }
    }

    PEG_METHOD_EXIT ();
    return true;
}

Array<SubscriptionWithSrcNamespace>
    IndicationService::_getMatchingSubscriptions (
        const CIMName& supportedClass,
        const Array<CIMNamespaceName> nameSpaces,
        const CIMPropertyList& supportedProperties,
        const Boolean checkProvider,
        const CIMInstance& provider)
{
    PEG_METHOD_ENTER (TRC_INDICATION_SERVICE,
        "IndicationService::_getMatchingSubscriptions");

    Array<SubscriptionWithSrcNamespace> matchingSubscriptions;
    Array<SubscriptionWithSrcNamespace> subscriptions;

    subscriptions = _subscriptionTable->getMatchingSubscriptions(
        supportedClass,
        nameSpaces,
        checkProvider,
        provider);

    for (Uint32 i = 0; i < subscriptions.size (); i++)
    {
        Boolean match = true;

        //
        //  If supported properties is null (all properties)
        //  the subscription can be supported
        //
        if (!supportedProperties.isNull ())
        {
            String filterQuery;
            String queryLanguage;
            CIMName indicationClassName;
            Array<CIMNamespaceName> sourceNamespaces;
            CIMPropertyList propertyList;
            String filterName;

            try
            {
                //
                //  Get filter properties
                //
                _subscriptionRepository->getFilterProperties(
                     subscriptions[i].subscription,
                     filterQuery,
                     sourceNamespaces,
                     queryLanguage,
                     filterName);

                QueryExpression queryExpr = _getQueryExpression(
                    filterQuery, queryLanguage, subscriptions[i].nameSpace);

                // Get the class paths in the FROM list
                // Since neither WQL nor CQL support joins, so we can
                // assume one class path.
                indicationClassName =
                    queryExpr.getClassPathList()[0].getClassName();

                if (!_subscriptionRepository->validateIndicationClassName(
                    indicationClassName, subscriptions[i].nameSpace))
                {
                    // Invalid FROM class, skip the subscription
                    continue;
                }

                //
                //  Get required property list from filter query (WHERE clause)
                //
                //  Note that the supportedClass is passed in,
                //  not the indicationClassName.
                //  The supportedClass is the class of the indication
                //  instance, while the indicationClassName is the FROM class.
                //  This is needed because CQL can have class scoping operators
                //  on properties that may not be the same class
                //  as the FROM class.  The required properties
                //  for an indication are based on its class,
                //  not the FROM class.
                //
                //  Also note that for CQL, this does not return
                //  required embedded object properties.
                propertyList = _getPropertyList (queryExpr,
                                             subscriptions[i].nameSpace,
                                             supportedClass);

                //
                //  If the subscription requires all properties,
                //  but supported property list does not include all
                //  properties, the subscription cannot be supported
                //
                if (propertyList.isNull ())
                {
                    //
                    //  Current subscription does not match
                    //  Continue to next subscription in list
                    //
                    continue;
                }
                else
                {
                    //
                    //  Compare subscription required property list
                    //  with supported property list
                    //
                    for (Uint32 j = 0;
                         j < propertyList.size () && match;
                         j++)
                    {
                        if (!ContainsCIMName
                            (supportedProperties.getPropertyNameArray(),
                            propertyList[j]))
                        {
                            match = false;
                            break;
                        }
                    }
                }
            }
            catch(const Exception & e)
            {
                // This subscription is invalid
                // skip it
               PEG_TRACE ((TRC_DISCARDED_DATA, Tracer::LEVEL2,
                    "Exception caught trying to verify required properties"
                    " in a subscription are all contained in the list of"
                    " supported indication properties: %s",
                    (const char *) e.getMessage ().getCString()));
                continue;
            }
            catch(const exception & e)
            {
                // This subscription is invalid
                // skip it
               PEG_TRACE ((TRC_DISCARDED_DATA, Tracer::LEVEL2,
                    "Exception caught trying to verify required properties"
                    " in a subscription are all contained in the list of"
                    " supported indication properties: %s", e.what ()));
                continue;
            }
            catch(...)
            {
                // This subscription is invalid
                // skip it
                PEG_TRACE_CSTRING (TRC_DISCARDED_DATA, Tracer::LEVEL2,
                    "Unknown exception caught trying to verify "
                    "required properties in a subscription are all contained "
                    "in the list of supported indication properties.");
                continue;
            }
        }

        if (match)
        {
            //
            //  Add current subscription to list
            //
            matchingSubscriptions.append (subscriptions[i]);
        }
    }

    PEG_METHOD_EXIT ();
    return matchingSubscriptions;
}

void IndicationService::_getModifiedSubscriptions (
    const CIMName& supportedClass,
    const Array<CIMNamespaceName>& newNameSpaces,
    const Array<CIMNamespaceName>& oldNameSpaces,
    const CIMPropertyList& newProperties,
    const CIMPropertyList& oldProperties,
    Array<SubscriptionWithSrcNamespace>& newSubscriptions,
    Array<SubscriptionWithSrcNamespace>& formerSubscriptions)
{
    PEG_METHOD_ENTER (TRC_INDICATION_SERVICE,
        "IndicationService::_getModifiedSubscriptions");

    Array<SubscriptionWithSrcNamespace> newList;
    Array<SubscriptionWithSrcNamespace> formerList;
    Array<SubscriptionWithSrcNamespace> bothList;

    newSubscriptions.clear ();
    formerSubscriptions.clear ();

    //
    //  For each newly supported namespace, lookup to retrieve list of
    //  subscriptions for the indication class-source namespace pair
    //
    newList = _subscriptionTable->getMatchingSubscriptions
        (supportedClass, newNameSpaces);

    //
    //  For each formerly supported namespace, lookup to retrieve list of
    //  subscriptions for the indication class-source namespace pair
    //
    formerList = _subscriptionTable->getMatchingSubscriptions
        (supportedClass, oldNameSpaces);

    //
    //  Find subscriptions that appear in both lists, and move them to a third
    //  list
    //
    Sint8 found;
    for (Uint32 p = 0; p < newList.size (); p++)
    {
        found = -1;
        for (Uint32 q = 0; q < formerList.size (); q++)
        {
            if (newList[p].subscription.identical (formerList[q].subscription))
            {
                found = q;
                bothList.append (newList[p]);
                break;
            }
        }
        if (found >= 0)
        {
            newList.remove (p);
            p--;
            formerList.remove (found);
        }
    }

    //
    //  For indicationClassName-sourceNamespace pair that is now supported, but
    //  previously was not, add to list of newly supported subscriptions if
    //  required properties are now supported
    //
    for (Uint32 n = 0; n < newList.size (); n++)
    {
        String filterQuery;
        String queryLanguage;
        CIMName indicationClassName;
        Array<CIMNamespaceName> sourceNamespaces;
        CIMPropertyList requiredProperties;
        String filterName;

        //
        //  Get filter properties
        //
        _subscriptionRepository->getFilterProperties (
            newList[n].subscription,
            filterQuery,
            sourceNamespaces,
            queryLanguage,
            filterName);

        QueryExpression queryExpression = _getQueryExpression(
            filterQuery, queryLanguage, newList[n].nameSpace);

        //
        //  Get indication class name from filter query (FROM clause)
        //
        indicationClassName = _getIndicationClassName (queryExpression,
            newList[n].nameSpace);

        //
        //  Get required property list from filter query (WHERE clause)
        //
        //  Note: the supportedClass is passed to _getPropertyList
        //  rather than the FROM class because CQL could have
        //  class scoping operators that scope properties to
        //  specific subclasses of the FROM.
        //
        requiredProperties = _getPropertyList (queryExpression,
            newList[n].nameSpace, supportedClass);

        //
        //  Check if required properties are now supported
        //
        if (_inPropertyList (requiredProperties, newProperties))
        {
            newSubscriptions.append (newList[n]);
        }
    }

    //
    //  For indicationClassName-sourceNamespace pair that was previously
    //  supported, but now is not, add to list of formerly supported
    //  subscriptions
    //
    for (Uint32 f = 0; f < formerList.size (); f++)
    {
        formerSubscriptions.append (formerList[f]);
    }

    //
    //  For indicationClassName-sourceNamespace pair that is now supported,
    //  and was also previously supported, add to appropriate list, based on
    //  required properties
    //
    for (Uint32 b = 0; b < bothList.size (); b++)
    {
        String filterQuery;
        String queryLanguage;
        CIMName indicationClassName;
        Array<CIMNamespaceName> sourceNamespaces;
        CIMPropertyList requiredProperties;
        Boolean newMatch = false;
        Boolean formerMatch = false;
        String filterName;

        //
        //  Get filter properties
        //
        _subscriptionRepository->getFilterProperties(
            bothList[b].subscription,
            filterQuery,
            sourceNamespaces,
            queryLanguage,
            filterName);

        QueryExpression queryExpression = _getQueryExpression(
            filterQuery, queryLanguage, bothList[b].nameSpace);

        //
        //  Get indication class name from filter query (FROM clause)
        //
        indicationClassName = _getIndicationClassName(
            queryExpression,
            bothList[b].nameSpace);

        //
        //  Get required property list from filter query (WHERE clause)
        //
        //  Note: the supportedClass is passed to _getPropertyList
        //  rather than the FROM class because CQL could have
        //  class scoping operators that scope properties to
        //  specific subclasses of the FROM.
        //
        requiredProperties = _getPropertyList (
            queryExpression,
            bothList[b].nameSpace,
            supportedClass);

        //
        //  Check required properties
        //
        newMatch = _inPropertyList (requiredProperties,
            newProperties);
        formerMatch = _inPropertyList (requiredProperties,
            oldProperties);

        //
        //  Add current subscription to appropriate list
        //
        if (newMatch && !formerMatch)
        {
            newSubscriptions.append (bothList[b]);
        }
        else if (!newMatch && formerMatch)
        {
            formerSubscriptions.append (bothList[b]);
        }
    }

    PEG_METHOD_EXIT ();
}

Boolean IndicationService::_inPropertyList (
    const CIMPropertyList& requiredProperties,
    const CIMPropertyList& supportedProperties)
{
    PEG_METHOD_ENTER (TRC_INDICATION_SERVICE,
        "IndicationService::_inPropertyList");

    //
    //  If property list is null (all properties)
    //  all the required properties are supported
    //
    if (supportedProperties.isNull ())
    {
        PEG_METHOD_EXIT();
        return true;
    }
    else
    {
        //
        //  If the subscription requires all properties,
        //  but property list does not include all
        //  properties, the required properties cannot be supported
        //
        if (requiredProperties.isNull ())
        {
            PEG_METHOD_EXIT();
            return false;
        }
        else
        {
            //
            //  Compare required property list
            //  with property list
            //
            for (Uint32 i = 0; i < requiredProperties.size (); i++)
            {
                if (!ContainsCIMName
                    (supportedProperties.getPropertyNameArray (),
                    requiredProperties[i]))
                {
                    PEG_METHOD_EXIT();
                    return false;
                }
            }
        }
    }

    PEG_METHOD_EXIT ();
    return true;
}

QueryExpression IndicationService::_getQueryExpression(
    const String& filterQuery,
    const String& queryLanguage,
    const CIMNamespaceName& ns) const
{
    PEG_METHOD_ENTER(TRC_INDICATION_SERVICE,
        "IndicationService::_getQueryExpression");

    try
    {
        RepositoryQueryContext ctx(ns, _cimRepository);
        QueryExpression queryExpression(queryLanguage, filterQuery, ctx);
        PEG_METHOD_EXIT();
        return queryExpression;
    }
    catch (QueryParseException& qpe)
    {
        String exceptionStr = qpe.getMessage();

        PEG_METHOD_EXIT();
        throw PEGASUS_CIM_EXCEPTION(CIM_ERR_INVALID_PARAMETER, exceptionStr);
    }
    catch (ParseError& pe)
    {
        String exceptionStr = pe.getMessage();

        PEG_METHOD_EXIT();
        throw PEGASUS_CIM_EXCEPTION(CIM_ERR_INVALID_PARAMETER, exceptionStr);
    }
    catch (MissingNullTerminator& mnt)
    {
        String exceptionStr = mnt.getMessage();

        PEG_METHOD_EXIT();
        throw PEGASUS_CIM_EXCEPTION(CIM_ERR_INVALID_PARAMETER, exceptionStr);
    }
}

CIMName IndicationService::_getIndicationClassName (
    const QueryExpression& queryExpression,
    const CIMNamespaceName& nameSpaceName) const
{
    PEG_METHOD_ENTER (TRC_INDICATION_SERVICE,
        "IndicationService::_getIndicationClassName");

    CIMName indicationClassName;
    Array<CIMName> indicationSubclasses;

    // Get the class paths in the FROM list.
    // Note: neither WQL nor CQL support joins, so we can
    // assume one class path.
    // Note: neither WQL not CQL support wbem-uri for class paths,
    // so we can ignore the parts of the path before the class name.
    Array<CIMObjectPath> fromPaths = queryExpression.getClassPathList();
    indicationClassName = fromPaths[0].getClassName();

    //
    //  Validate that class is an Indication class
    //  The Indication Qualifier should exist and have the value True
    //
    Boolean validClass = _subscriptionRepository->validateIndicationClassName
        (indicationClassName, nameSpaceName);

    if (!validClass)
    {
        PEG_METHOD_EXIT();
        throw PEGASUS_CIM_EXCEPTION_L(
            CIM_ERR_INVALID_PARAMETER,
            MessageLoaderParms(
                "IndicationService.IndicationService."
                    "_MSG_INVALID_CLASSNAME_IN_FROM_PROPERTY",
                "The Indication class name $0 is not valid in the FROM clause "
                    "of $1 $2 property.",
                indicationClassName.getString(),
                PEGASUS_CLASSNAME_INDFILTER.getString(),
                PEGASUS_PROPERTYNAME_QUERY.getString()));
    }

    PEG_METHOD_EXIT ();
    return indicationClassName;
}

Array<ProviderClassList> IndicationService::_getIndicationProviders (
    const String &query,
    const String &queryLang,
    const CIMName& indicationClassName,
    const Array<NamespaceClassList>& indicationSubclasses) const
{
    PEG_METHOD_ENTER (TRC_INDICATION_SERVICE,
        "IndicationService::_getIndicationProviders");

    Array<ProviderClassList> indicationProviders;
    ProviderClassList provider;
    Array<CIMInstance> providerInstances;
    Array<CIMInstance> providerModuleInstances;

    CIMPropertyList requiredPropertyList;


    //
    //  For each indication subclass, get providers
    //
    for (Uint32 i = 0, n = indicationSubclasses.size (); i < n; i++)
    {
        for (Uint32 c = 0, nc = indicationSubclasses[i].classList.size();
            c < nc; ++c)
        {
            QueryExpression queryExpression = _getQueryExpression(
                query,
                queryLang,
                indicationSubclasses[i].nameSpace);

            //  Get required property list from filter query (WHERE clause)
            //  from this indication subclass
            //
            requiredPropertyList = _getPropertyList (
                queryExpression,
                indicationSubclasses[i].nameSpace,
                indicationSubclasses[i].classList[c]);

            //
            //  Get providers that can serve the subscription
            //
            providerInstances.clear ();
            providerModuleInstances.clear ();
            if (_providerRegManager->getIndicationProviders(
                 indicationSubclasses[i].nameSpace,
                 indicationSubclasses[i].classList[c],
                 requiredPropertyList,
                 providerInstances,
                 providerModuleInstances))
            {
                PEGASUS_ASSERT (providerInstances.size () ==
                    providerModuleInstances.size ());

                PEG_TRACE((TRC_INDICATION_SERVICE, Tracer::LEVEL4,
                    "%u indication provider(s) found for class %s",
                    providerInstances.size (),
                    (const char *)indicationSubclasses[i].classList[c].
                        getString ().getCString ()));

                //
                //  Merge into list of ProviderClassList structs
                //
                for (Uint32 j = 0, numI = providerInstances.size ();
                    j < numI; j++)
                {
                    provider.classList.clear ();
                    Boolean duplicate = false;

                    //
                    //  See if indication provider is already in list
                    //
                    for (Uint32 k = 0, numP = indicationProviders.size ();
                        k < numP && !duplicate; k++)
                    {
                        if ((providerInstances[j].getPath ().identical
                            (indicationProviders[k].provider.getPath ())) &&
                            (providerModuleInstances[j].getPath ().identical
                            (indicationProviders[k].providerModule.getPath ())))
                        {
                            //
                            //  Indication provider is already in list
                            //  Add subclass to provider's class list
                            //
                            for(Uint32 s = 0, ns = indicationProviders[k].
                                    classList.size();
                                s < ns;  ++s)
                            {
                                if (indicationProviders[k].classList[s].
                                    nameSpace == indicationSubclasses[i].
                                        nameSpace)
                                {
                                    indicationProviders[k].classList[s].
                                        classList.append(
                                            indicationSubclasses[i].
                                                classList[c]);
                                    duplicate = true;
                                    break;
                                }
                            }
                            if (!duplicate)
                            {
                                NamespaceClassList nscl;
                                nscl.nameSpace =
                                    indicationSubclasses[i].nameSpace;
                                nscl.classList.append(
                                    indicationSubclasses[i].classList[c]);
                                indicationProviders[k].classList.append(nscl);
                                duplicate = true;
                            }
                        }
                    }

                    if (!duplicate)
                    {
                        //
                        //  Current provider is not yet in list
                        //  Create new list entry
                        //
                        provider.provider = providerInstances[j];
                        provider.providerModule = providerModuleInstances[j];
                        NamespaceClassList nscl;
                        nscl.classList.append(
                            indicationSubclasses[i].classList[c]);
                        nscl.nameSpace =
                            indicationSubclasses[i].nameSpace;
                        provider.classList.append (nscl);
#ifdef PEGASUS_ENABLE_REMOTE_CMPI
                        String remoteInformation;
                        Boolean isRemote = _cimRepository->isRemoteNameSpace(
                            nscl.nameSpace, remoteInformation);
                        provider.isRemoteNameSpace = isRemote;
                        provider.remoteInfo = remoteInformation;
#endif
                        indicationProviders.append(provider);
                    }
                }  // for each indication provider instance
            }  // if any providers
        }  // for each indication subclass
    }  // for each source namespace

    // Verify if any control providers exists for this class
    if (!indicationProviders.size())
    {
        indicationProviders =
            _getInternalIndProviders(indicationSubclasses);
    }

    PEG_METHOD_EXIT();
    return indicationProviders;
}

CIMPropertyList IndicationService::_getPropertyList(
    const QueryExpression& queryExpression,
    const CIMNamespaceName& nameSpaceName,
    const CIMName& indicationClassName) const
{
    PEG_METHOD_ENTER(TRC_INDICATION_SERVICE,
        "IndicationService::_getPropertyList");

    CIMPropertyList propertyList;

    //  Get all the properties referenced in the condition (WHERE clause)
    //  Note: for CQL, this only returns the properties directly on the
    //  class name passed in, not any properties on embedded objects.
    //
    try
    {
      CIMObjectPath classPath(String::EMPTY,
                               nameSpaceName,
                               indicationClassName);
      propertyList = queryExpression.getWherePropertyList(classPath);
    }
    catch (QueryException& qe)
    {
      // The class path was not the FROM class, or a subclass
      // of the FROM class.
      String exceptionStr = qe.getMessage();

      PEG_METHOD_EXIT();
      throw PEGASUS_CIM_EXCEPTION(CIM_ERR_FAILED, exceptionStr);
    }

    if (propertyList.isNull())
    {
        //
        //  Return null property list for all properties
        //
        PEG_METHOD_EXIT();
        return propertyList;
    }
    else
    {
        Array<CIMName> propertyArray;

        //  Get the property names
        //
        propertyArray = propertyList.getPropertyNameArray();

        Array<CIMName> indicationClassProperties;
        PEG_METHOD_EXIT();
        return _checkPropertyList(propertyArray, nameSpaceName,
            indicationClassName, indicationClassProperties);
    }
}

CIMPropertyList IndicationService::_checkPropertyList(
    const Array<CIMName>& propertyList,
    const CIMNamespaceName& nameSpaceName,
    const CIMName& indicationClassName,
    Array<CIMName>& indicationClassProperties) const
{
    PEG_METHOD_ENTER(TRC_INDICATION_SERVICE,
        "IndicationService::_checkPropertyList");

    //
    //  Check if list includes all properties in class
    //  If so, must be set to NULL
    //
    CIMClass indicationClass;

    //
    //  Get the indication class object from the repository
    //  Specify localOnly=false because superclass properties are needed
    //  Specify includeQualifiers=false because qualifiers are not needed
    //
    indicationClass = _subscriptionRepository->getClass(
        nameSpaceName, indicationClassName, false, false, false,
        CIMPropertyList());

    Boolean allProperties = true;
    for (Uint32 i = 0; i < indicationClass.getPropertyCount(); i++)
    {
        indicationClassProperties.append(
            indicationClass.getProperty(i).getName());
        if (!ContainsCIMName(propertyList,
            indicationClass.getProperty(i).getName()))
        {
            allProperties = false;
        }
    }

    if (allProperties)
    {
        //
        //  Return NULL CIMPropertyList
        //
        PEG_METHOD_EXIT();
        return CIMPropertyList();
    }
    else
    {
        PEG_METHOD_EXIT();
        return CIMPropertyList(propertyList);
    }
}

String IndicationService::_getCondition(
    const String& filterQuery) const
{
    PEG_METHOD_ENTER(TRC_INDICATION_SERVICE,
        "IndicationService::_getCondition");

    String condition;

    //
    //  Get condition substring from filter query
    //
    if (filterQuery.find(_QUERY_WHERE) != PEG_NOT_FOUND)
    {
        condition = filterQuery.subString(filterQuery.find(_QUERY_WHERE) + 6);
    }

    PEG_METHOD_EXIT();
    return condition;
}

void IndicationService::_deleteReferencingSubscriptions(
    const CIMNamespaceName& nameSpace,
    const CIMName& referenceProperty,
    const CIMObjectPath& handler)
{
    PEG_METHOD_ENTER(TRC_INDICATION_SERVICE,
        "IndicationService::_deleteReferencingSubscriptions");

    Array<CIMInstance> deletedSubscriptions;

    //
    //  Delete referencing subscriptions from the repository
    //
    deletedSubscriptions =
        _subscriptionRepository->deleteReferencingSubscriptions(
            nameSpace, referenceProperty, handler);

    //
    //  Send delete request to each provider for each deleted subscription
    //
    for (Uint32 i = 0; i < deletedSubscriptions.size(); i++)
    {
        Array<ProviderClassList> indicationProviders;
        Array<NamespaceClassList> indicationSubclasses;

        indicationProviders = _getDeleteParams(deletedSubscriptions[i],
            indicationSubclasses);

        //
        //  Send Delete requests
        //
        //  NOTE: These Delete requests are not associated with a user
        //  request, so there is no associated authType or userName
        //  The Creator from the subscription instance is used for userName,
        //  and authType is not set
        //
        CIMInstance instance = deletedSubscriptions[i];
        String creator;
        _getCreator(instance, creator);

// l10n start
        AcceptLanguageList acceptLangs;
        Uint32 propIndex = instance.findProperty(
            PEGASUS_PROPERTYNAME_INDSUB_ACCEPTLANGS);
        if (propIndex != PEG_NOT_FOUND)
        {
            String acceptLangsString;
            instance.getProperty(propIndex).getValue().get(
                acceptLangsString);
            if (acceptLangsString.size())
            {
                acceptLangs = LanguageParser::parseAcceptLanguageHeader(
                    acceptLangsString);
            }
        }
        ContentLanguageList contentLangs;
        propIndex = instance.findProperty(
            PEGASUS_PROPERTYNAME_INDSUB_CONTENTLANGS);
        if (propIndex != PEG_NOT_FOUND)
        {
            String contentLangsString;
            instance.getProperty(propIndex).getValue().get(
                contentLangsString);
            if (contentLangsString.size())
            {
                contentLangs = LanguageParser::parseContentLanguageHeader(
                    contentLangsString);
            }
        }
// l10n end

        _sendAsyncDeleteRequests(
            indicationProviders,
            deletedSubscriptions[i],
            acceptLangs,
            contentLangs,
            0,  // no request
            indicationSubclasses,
            creator);
    }

    PEG_METHOD_EXIT();
}

Boolean IndicationService::_isExpired(
    const CIMInstance& instance) const
{
    PEG_METHOD_ENTER(TRC_INDICATION_SERVICE, "IndicationService::_isExpired");

    Boolean isExpired = true;
    Uint64 timeRemaining = 0;

    //
    //  Get time remaining, if subscription has a duration
    //
    if (_getTimeRemaining(instance, timeRemaining))
    {
        if (timeRemaining > 0)
        {
            isExpired = false;
        }
    }
    else
    {
        //
        //  If there is no duration, the subscription has no expiration date
        //
        isExpired = false;
    }

    PEG_METHOD_EXIT();
    return isExpired;
}

void IndicationService::_deleteExpiredSubscription(
    CIMObjectPath& subscription)
{
    PEG_METHOD_ENTER(TRC_INDICATION_SERVICE,
        "IndicationService::_deleteExpiredSubscription");

    CIMInstance subscriptionInstance;
    //
    //  Delete instance from repository
    //
    subscriptionInstance =
        _subscriptionRepository->deleteSubscription(subscription);

    //
    //  If a valid instance object was returned, the subscription was
    //  successfully deleted
    //
    if (!subscriptionInstance.isUninitialized())
    {
        //
        //  If subscription was active, send delete requests to providers
        //  and update hash tables
        //
        Uint16 subscriptionState;
        CIMValue subscriptionStateValue;
        subscriptionStateValue = subscriptionInstance.getProperty(
            subscriptionInstance.findProperty(
                PEGASUS_PROPERTYNAME_SUBSCRIPTION_STATE)).getValue();
        subscriptionStateValue.get(subscriptionState);

        if ((subscriptionState == STATE_ENABLED) ||
            (subscriptionState == STATE_ENABLEDDEGRADED))
        {
            Array<ProviderClassList> indicationProviders;
            Array<NamespaceClassList> indicationSubclasses;
            CIMNamespaceName sourceNamespaceName;

            subscriptionInstance.setPath(subscription);

            indicationProviders = _getDeleteParams(subscriptionInstance,
                indicationSubclasses);

            //
            //  Send Delete requests
            //
            //  NOTE: These Delete requests are not associated with a user
            //  request, so there is no associated authType or userName
            //  The Creator from the subscription instance is used for userName,
            //  and authType is not set
            //
            String creator;
            _getCreator(subscriptionInstance, creator);

            //
            // Get the language tags that were saved with the subscription
            // instance
            //
            AcceptLanguageList acceptLangs;
            Uint32 propIndex = subscriptionInstance.findProperty(
                PEGASUS_PROPERTYNAME_INDSUB_ACCEPTLANGS);
            if (propIndex != PEG_NOT_FOUND)
            {
                String acceptLangsString;
                subscriptionInstance.getProperty(propIndex).getValue().get(
                    acceptLangsString);
                if (acceptLangsString.size())
                {
                    acceptLangs = LanguageParser::parseAcceptLanguageHeader(
                        acceptLangsString);
                }
            }
            ContentLanguageList contentLangs;
            propIndex = subscriptionInstance.findProperty(
                PEGASUS_PROPERTYNAME_INDSUB_CONTENTLANGS);
            if (propIndex != PEG_NOT_FOUND)
            {
                String contentLangsString;
                subscriptionInstance.getProperty(propIndex).getValue().get(
                    contentLangsString);
                if (contentLangsString.size())
                {
                    contentLangs = LanguageParser::parseContentLanguageHeader(
                        contentLangsString);
                }
            }

            subscriptionInstance.setPath(subscription);
            _sendAsyncDeleteRequests(indicationProviders,
                subscriptionInstance,
                acceptLangs,
                contentLangs,
                0, // no request
                indicationSubclasses,
                creator);
        }
#ifdef PEGASUS_ENABLE_PROTOCOL_WSMAN
        _deleteFilterHandler(subscriptionInstance);
#endif
    }
    else
    {
        //
        //  The subscription may have already been deleted by another thread
        //
    }

    PEG_METHOD_EXIT();
}


#ifdef PEGASUS_ENABLE_PROTOCOL_WSMAN
//If the subscription is wsman then Delete the filter and handler
//also from the repository.
void IndicationService::_deleteFilterHandler(
    CIMInstance& subscriptionInstance)
{
    PEG_METHOD_ENTER(TRC_INDICATION_SERVICE,
        "IndicationService::_deleteFilterHandler");
    Uint32 handlerPropIndex = subscriptionInstance.findProperty(
        PEGASUS_PROPERTYNAME_HANDLER);
    if(handlerPropIndex != PEG_NOT_FOUND)
    {
        CIMProperty handlerProperty = subscriptionInstance.getProperty(
            handlerPropIndex);
        CIMObjectPath handlerObjPath;
        handlerProperty.getValue().get(handlerObjPath);
        if(handlerObjPath.getClassName() ==
            PEGASUS_CLASSNAME_INDHANDLER_WSMAN)
        {
            Array<CIMKeyBinding> keyBindings = handlerObjPath.
                getKeyBindings();
            // Get the Handler name
            String handlerName;
            for(Uint32 i = 0 ; i < keyBindings.size(); i++)
            {
                 if(keyBindings[i].getName().getString() ==
                     PEGASUS_PROPERTYNAME_NAME.getString())
                 {
                     handlerName = keyBindings[i].getValue();
                     break;
                 }
            }
            _subscriptionRepository->deleteInstance(
                handlerObjPath.getNameSpace(), handlerObjPath);
        }
    }

    Uint32 filterPropIndex = subscriptionInstance.findProperty(
        PEGASUS_PROPERTYNAME_FILTER);
    if(filterPropIndex != PEG_NOT_FOUND)
    {
        CIMProperty filterProperty = subscriptionInstance.
            getProperty(filterPropIndex);
        CIMObjectPath filterObjPath;
        filterProperty.getValue().get(filterObjPath);
        Array<CIMKeyBinding> keyBindings = filterObjPath.
            getKeyBindings();
        // Get Filter name
        String filterName;
        for(Uint32 i = 0 ; i < keyBindings.size(); i++)
        {
             if(keyBindings[i].getName().getString() ==
                 PEGASUS_PROPERTYNAME_NAME.getString())
             {
                 filterName = keyBindings[i].getValue();
                 break;
             }
        }
        // If filter was created by the wsman subscribe request,
        // then delete it. If filter is created by wsman subscribe
        // request, subscriptionInfo and filter name will match.
        Uint32 subInfoIndex = subscriptionInstance.findProperty(
            _PROPERTY_SUBSCRIPTION_INFO);
        CIMProperty subInfoProperty = subscriptionInstance.
            getProperty(subInfoIndex);
        String subscriptionInfo;
        subInfoProperty.getValue().get(subscriptionInfo);
        if (subscriptionInfo == filterName)
        {
            _subscriptionRepository->deleteInstance(
                filterObjPath.getNameSpace(),filterObjPath);
        }
    }
    PEG_METHOD_EXIT();
}
#endif

Boolean IndicationService::_getTimeRemaining(
    const CIMInstance& instance,
    Uint64& timeRemaining) const
{
    PEG_METHOD_ENTER(TRC_INDICATION_SERVICE,
        "IndicationService::_getTimeRemaining");

    Boolean hasDuration = true;
    timeRemaining = 0;

    //
    //  Calculate time remaining from subscription
    //  start time, subscription duration, and current date time
    //

    //
    //  NOTE: It is assumed that the instance passed to this method is a
    //  subscription instance, and that the Start Time property exists
    //  and has a value
    //

    //
    //  Get Subscription Start Time
    //
    CIMValue startTimeValue;
    CIMDateTime startTime;
    Uint32 startTimeIndex = instance.findProperty(_PROPERTY_STARTTIME);
    PEGASUS_ASSERT(startTimeIndex != PEG_NOT_FOUND);
    startTimeValue = instance.getProperty(startTimeIndex).getValue();
    PEGASUS_ASSERT(!(startTimeValue.isNull()));
    startTimeValue.get(startTime);

    //
    //  Get Subscription Duration
    //
    Uint32 durationIndex = instance.findProperty(_PROPERTY_DURATION);
    if (durationIndex != PEG_NOT_FOUND)
    {
        CIMValue durationValue;
        durationValue = instance.getProperty(durationIndex).getValue();
        if (durationValue.isNull())
        {
            hasDuration = false;
        }
        else
        {
            Uint64 duration;
            durationValue.get(duration);

            //
            //  A Start Time set to the _ZERO_INTERVAL_STRING indicates that
            //  the subscription has not yet been enabled for the first time
            //  In this case, the time remaining is equal to the Duration
            //
            if (startTime.isInterval())
            {
                if (startTime.equal(CIMDateTime(_ZERO_INTERVAL_STRING)))
                {
                    timeRemaining = (Sint64) duration;
                }

                //
                //  Any interval value other than _ZERO_INTERVAL_STRING
                //  indicates an invalid Start Time value in the instance
                //
                else
                {
                    PEGASUS_UNREACHABLE(PEGASUS_ASSERT(false);)
                }
            }

            else
            {
                //
                //  Get current date time, and calculate Subscription Time
                //  Remaining
                //
                CIMDateTime currentDateTime = CIMDateTime::getCurrentDateTime();

                Sint64 difference = CIMDateTime::getDifference(
                    startTime, currentDateTime);
                PEGASUS_ASSERT(difference >= 0);
                if (((Sint64) duration - difference) >= 0)
                {
                    timeRemaining = (Sint64) duration - difference;
                }
            }
        }
    }
    else
    {
        hasDuration = false;
    }

    PEG_METHOD_EXIT();
    return hasDuration;
}

void IndicationService::_setTimeRemaining(
    CIMInstance& instance)
{
    PEG_METHOD_ENTER(TRC_INDICATION_SERVICE,
        "IndicationService::_setTimeRemaining");

    Uint64 timeRemaining = 0;
    if (_getTimeRemaining(instance, timeRemaining))
    {
        //
        //  Add or set the value of the property with the calculated value
        //
        if (instance.findProperty(_PROPERTY_TIMEREMAINING) == PEG_NOT_FOUND)
        {
            instance.addProperty(
                CIMProperty(_PROPERTY_TIMEREMAINING, timeRemaining));
        }
        else
        {
            CIMProperty remaining = instance.getProperty(
                instance.findProperty(_PROPERTY_TIMEREMAINING));
            remaining.setValue(CIMValue(timeRemaining));
        }
    }

    PEG_METHOD_EXIT();
}

void IndicationService::_getCreateParams(
    const CIMInstance& subscriptionInstance,
    Array<NamespaceClassList>& indicationSubclasses,
    Array<ProviderClassList>& indicationProviders,
    CIMPropertyList& propertyList,
    String& condition,
    String& query,
    String& queryLanguage)
{
    PEG_METHOD_ENTER(TRC_INDICATION_SERVICE,
        "IndicationService::_getCreateParams");

    CIMName indicationClassName;
    condition = String::EMPTY;
    query = String::EMPTY;
    queryLanguage = String::EMPTY;
    String filterName;

    Array<CIMNamespaceName> sourceNameSpaces;
    //
    //  Get filter properties
    //
    _subscriptionRepository->getFilterProperties(
        subscriptionInstance,
        query,
        sourceNameSpaces,
        queryLanguage,
        filterName);

     for (Uint32 i = 0, n = sourceNameSpaces.size(); i < n; ++i)
     {

        //
        //  Build the query expression from the filter query
        //
        QueryExpression queryExpression = _getQueryExpression(
            query,
            queryLanguage,
            sourceNameSpaces[i]);

        //
        //  Get indication class name from filter query (FROM clause)
        //
        indicationClassName = _getIndicationClassName(
            queryExpression,
            sourceNameSpaces[i]);

        //
        //  Get list of subclass names for indication class
        //
        Array<CIMName> indSubclasses =
            _subscriptionRepository->getIndicationSubclasses(
                sourceNameSpaces[i],
                indicationClassName);

        NamespaceClassList namespaceClassList;
        namespaceClassList.nameSpace = sourceNameSpaces[i];
        namespaceClassList.classList = indSubclasses;
        indicationSubclasses.append(namespaceClassList);
    }

    //
    //  Get indication provider class lists
    //
    indicationProviders = _getIndicationProviders(
        query,
        queryLanguage,
        indicationClassName,
        indicationSubclasses);

    if (indicationProviders.size() > 0)
    {
        condition = _getCondition(query);
    }

    PEG_METHOD_EXIT();
}

Array<ProviderClassList> IndicationService::_getInternalIndProviders(
    const Array<NamespaceClassList>& indicationSubclasses) const
{
    Array<ProviderClassList> providers;
    for (Uint32 i = 0, n = indicationSubclasses.size(); i < n; ++i)
    {
        for (Uint32 k = 0, m = indicationSubclasses[i].classList.size();
            k < m; ++k)
        {
            for (ControlProvIndRegTable::Iterator j =
                _controlProvIndRegTable.start (); j; j++)
            {
                ControlProvIndReg reg = j.value();
                if (indicationSubclasses[i].classList[k] == reg.className &&
                    (reg.nameSpace.isNull() ||
                    reg.nameSpace == indicationSubclasses[i].nameSpace))
                {
                    String controlProviderName;
                    reg.provider.getProperty(
                        reg.provider.findProperty(
                           PEGASUS_PROPERTYNAME_NAME)).getValue().get(
                               controlProviderName);
                     ProviderClassList provider;
                     provider.controlProviderName = controlProviderName;
                     provider.provider = reg.provider;
                     provider.providerModule = reg.providerModule;
                     NamespaceClassList nscl;
                     nscl.nameSpace =
                         indicationSubclasses[i].nameSpace;
                     nscl.classList.append(reg.className);
                     provider.classList.append(nscl);
                     providers.append(provider);
                }
            }
        }
    }

    return providers;
}

void IndicationService::_getCreateParams(
    const CIMInstance& subscriptionInstance,
    Array<NamespaceClassList>& indicationSubclasses,
    CIMPropertyList& propertyList,
    String& condition,
    String& query,
    String& queryLanguage)
{
    PEG_METHOD_ENTER(TRC_INDICATION_SERVICE,
        "IndicationService::_getCreateParams");

    condition = String::EMPTY;
    query = String::EMPTY;
    queryLanguage = String::EMPTY;
    String filterName;
    Array<CIMNamespaceName> sourceNamespaces;

    //
    //  Get filter properties
    //
    _subscriptionRepository->getFilterProperties(
        subscriptionInstance,
        query,
        sourceNamespaces,
        queryLanguage,
        filterName);

    for (Uint32 i = 0, n = sourceNamespaces.size(); i < n; ++i)
    {
        QueryExpression queryExpression = _getQueryExpression(
            query,
            queryLanguage,
            sourceNamespaces[i]);

        //
        //  Get indication class name from filter query (FROM clause)
        //
        CIMName indicationClassName =
            _getIndicationClassName(queryExpression, sourceNamespaces[i]);

        //
        //  Get required property list from filter query (WHERE clause)
        //
        propertyList = _getPropertyList(
            queryExpression,
            sourceNamespaces[i],
            indicationClassName);

        //
        //  Get condition from filter query (WHERE clause)
        //
        condition = _getCondition(query);

        //
        //  Get list of subclass names for indication class
        //
        Array<CIMName> indSubclasses =
            _subscriptionRepository->getIndicationSubclasses(
                sourceNamespaces[i],
                indicationClassName);

        NamespaceClassList namespaceClassList;
        namespaceClassList.nameSpace = sourceNamespaces[i];
        namespaceClassList.classList = indSubclasses;
        indicationSubclasses.append(namespaceClassList);
    }

    PEG_METHOD_EXIT();
}

Array<ProviderClassList> IndicationService::_getDeleteParams(
    const CIMInstance& subscriptionInstance,
    Array<NamespaceClassList>& indicationSubclasses)
{
    PEG_METHOD_ENTER(TRC_INDICATION_SERVICE,
        "IndicationService::_getDeleteParams");

    String filterQuery;
    String queryLanguage;
    String filterName;
    CIMName indicationClassName;
    Array<ProviderClassList> indicationProviders;
    Array<CIMNamespaceName> sourceNamespaces;

    //
    //  Get filter properties
    //
    _subscriptionRepository->getFilterProperties(
        subscriptionInstance,
        filterQuery,
        sourceNamespaces,
        queryLanguage,
        filterName);

    for(Uint32 i = 0, n = sourceNamespaces.size(); i < n; ++i)
    {
        QueryExpression queryExpression =
            _getQueryExpression(
                filterQuery,
                queryLanguage,
                sourceNamespaces[i]);

        //
        //  Get indication class name from filter query (FROM clause)
        //
        indicationClassName =
            _getIndicationClassName(queryExpression, sourceNamespaces[i]);

        //
        //  Get list of subclass names for indication class
        //
        Array<CIMName> indSubclasses =
            _subscriptionRepository->getIndicationSubclasses(
                sourceNamespaces[i],
                indicationClassName);

        NamespaceClassList namespaceClassList;
        namespaceClassList.nameSpace = sourceNamespaces[i];
        namespaceClassList.classList = indSubclasses;
        indicationSubclasses.append(namespaceClassList);
    }

    //
    //  Get indication provider class lists from Active Subscriptions table
    //
    ActiveSubscriptionsTableEntry tableValue;
    if (_subscriptionTable->getSubscriptionEntry(
            subscriptionInstance.getPath(), tableValue))
    {
        indicationProviders = tableValue.providers;
    }
    else
    {
        //
        //  Subscription not found in Active Subscriptions table
        //
    }

    PEG_METHOD_EXIT();
    return indicationProviders;
}

Array<ProviderClassList> IndicationService::
    _getIndicationProvidersWithNamespaceClassList(
        const Array<ProviderClassList> &providers)
{
    Array<ProviderClassList> indProviders;
    for (Uint32 i = 0, n = providers.size(); i < n; ++i)
    {
        for (Uint32 j = 0, m = providers[i].classList.size(); j < m; ++j)
        {
            PEGASUS_ASSERT(
                providers[i].classList[j].nameSpace != CIMNamespaceName());
            ProviderClassList prcl = providers[i];
            NamespaceClassList nscl;
            nscl.nameSpace = providers[i].classList[j].nameSpace;
            nscl.classList = providers[i].classList[j].classList;
            prcl.classList.clear();
            prcl.classList.append(nscl);
            indProviders.append(prcl);

        }
    }
    return indProviders;
}

void IndicationService::_addProviderToAcceptedProviderList(
    Array<ProviderClassList> &acceptedProviders,
    ProviderClassList &provider)
{
    PEGASUS_ASSERT(provider.classList.size() == 1);

    for (Uint32 i = 0, n = acceptedProviders.size(); i < n; ++i)
    {
        if (acceptedProviders[i].provider.getPath().identical(
                provider.provider.getPath()) &&
            acceptedProviders[i].providerModule.getPath().identical(
                provider.providerModule.getPath()))
        {
            acceptedProviders[i].classList.append(provider.classList[0]);
            return;
        }
    }
    acceptedProviders.append(provider);
#ifdef PEGASUS_ENABLE_INDICATION_COUNT
    _providerIndicationCountTable.insertEntry(provider.provider);
#endif
}

void IndicationService::_sendAsyncCreateRequests(
    const Array<ProviderClassList>& providers,
    const CIMPropertyList& propertyList,
    const String& condition,
    const String& query,
    const String& queryLanguage,
    const CIMInstance& subscription,
    const AcceptLanguageList& acceptLangs,
    const ContentLanguageList& contentLangs,
    const CIMRequestMessage * origRequest,
    const Array<NamespaceClassList>& indicationSubclasses,
    const String& userName,
    const String& authType)
{
    PEG_METHOD_ENTER(TRC_INDICATION_SERVICE,
        "IndicationService::_sendAsyncCreateRequests");

    CIMValue propValue;
    Uint16 repeatNotificationPolicy;

    Array<ProviderClassList> indicationProviders =
        _getIndicationProvidersWithNamespaceClassList(providers);

    // If there are no providers to accept the subscription, just return
    if (indicationProviders.size() == 0)
    {
        PEG_METHOD_EXIT();
        return;
    }

#ifdef PEGASUS_ENABLE_DMTF_INDICATION_PROFILE_SUPPORT
    _asyncRequestsPending++;
    AutoPtr<AtomicInt, DecAtomicInt> counter(&_asyncRequestsPending);
#endif

    //
    //  Get repeat notification policy value from subscription instance
    //
    propValue = subscription.getProperty(
        subscription.findProperty(
            _PROPERTY_REPEATNOTIFICATIONPOLICY)).getValue();
    propValue.get(repeatNotificationPolicy);

    CIMRequestMessage * aggRequest=0;

    if (origRequest == 0)
    {
        //
        //  Initialize -- no request associated with this create
        //
        aggRequest = 0;
    }
    else
    {
        //
        //  Create Instance or Modify Instance
        //
        switch (origRequest->getType())
        {
            case CIM_CREATE_INSTANCE_REQUEST_MESSAGE:
            {
                CIMCreateInstanceRequestMessage * request =
                    (CIMCreateInstanceRequestMessage *) origRequest;
                CIMCreateInstanceRequestMessage * requestCopy =
                    new CIMCreateInstanceRequestMessage(*request);
                aggRequest = requestCopy;
                break;
            }

            case CIM_MODIFY_INSTANCE_REQUEST_MESSAGE:
            {
                CIMModifyInstanceRequestMessage * request =
                    (CIMModifyInstanceRequestMessage *) origRequest;
                CIMModifyInstanceRequestMessage * requestCopy =
                    new CIMModifyInstanceRequestMessage(*request);
                aggRequest = requestCopy;
                break;
            }

            default:
            {
                PEGASUS_UNREACHABLE(PEGASUS_ASSERT(false);)
                break;
            }
        }
    }

    //
    //  Create an aggregate object for the create subscription requests
    //
    AutoPtr<IndicationOperationAggregate> operationAggregate(
        new IndicationOperationAggregate(
            aggRequest,
            indicationProviders[0].controlProviderName,
            indicationSubclasses));

    operationAggregate.get()->setNumberIssued(indicationProviders.size());

    //
    //  Send Create request to each provider
    //
    for (Uint32 i = 0; i < indicationProviders.size(); i++)
    {
        //
        //  Create the create subscription request
        //
       AutoPtr<CIMCreateSubscriptionRequestMessage> request(
            new CIMCreateSubscriptionRequestMessage(
                XmlWriter::getNextMessageId(),
                indicationProviders[i].classList[0].nameSpace,
                subscription,
                indicationProviders[i].classList[0].classList,
                propertyList,
                repeatNotificationPolicy,
                query,
                QueueIdStack(_providerManager, getQueueId()),
                authType,
                userName));

        //
        //  Store a copy of the request in the operation aggregate instance
        //
        AutoPtr<CIMCreateSubscriptionRequestMessage> requestCopy(
            new CIMCreateSubscriptionRequestMessage(* (request.get())));

        requestCopy.get()->operationContext.insert(ProviderIdContainer(
            indicationProviders[i].providerModule
            ,indicationProviders[i].provider
#ifdef PEGASUS_ENABLE_REMOTE_CMPI
            ,indicationProviders[i].isRemoteNameSpace
            ,indicationProviders[i].remoteInfo
#endif
            ));
        operationAggregate.get()->appendRequest(requestCopy.get());
        request.get()->operationContext.insert(ProviderIdContainer(
            indicationProviders[i].providerModule
            ,indicationProviders[i].provider
#ifdef PEGASUS_ENABLE_REMOTE_CMPI
            ,indicationProviders[i].isRemoteNameSpace
            ,indicationProviders[i].remoteInfo
#endif
            ));
        request.get()->operationContext.insert(
            SubscriptionInstanceContainer(subscription));
        request.get()->operationContext.insert(
            SubscriptionFilterConditionContainer(condition,queryLanguage));
        request.get()->operationContext.insert(
            SubscriptionFilterQueryContainer(
                query,
                queryLanguage,
                indicationProviders[i].classList[0].nameSpace));
        request.get()->operationContext.insert(IdentityContainer(userName));
        request.get()->operationContext.set(
            ContentLanguageListContainer(contentLangs));
        request.get()->operationContext.set(
            AcceptLanguageListContainer(acceptLangs));

        AsyncOpNode * op = this->get_op();

        AutoPtr<AsyncRequest> asyncRequest;
        Uint32 serviceId;
        if (!indicationProviders[i].controlProviderName.size())
        {
            serviceId = _providerManager;
            asyncRequest.reset(
                new AsyncLegacyOperationStart(
                    op,
                    serviceId,
                    request.get()));
        }
        else
        {
           serviceId = _moduleController;
            asyncRequest.reset(
               new AsyncModuleOperationStart(
                   op,
                   serviceId,
                   indicationProviders[i].controlProviderName,
                   request.get()));
        }

        SendAsync(
            op,
            serviceId,
            IndicationService::_aggregationCallBack,
            this,
            operationAggregate.get());

        // Release objects from their AutoPtr to prevent double deletes.
        asyncRequest.release();
        requestCopy.release();
        request.release();

#ifdef PEGASUS_ENABLE_DMTF_INDICATION_PROFILE_SUPPORT
       // Release AutomicInt if atleast one request is sent for aggregation.
       counter.release();
#endif
    }
    operationAggregate.release();

    PEG_METHOD_EXIT();
}

Array<ProviderClassList> IndicationService::_sendWaitCreateRequests(
    const Array<ProviderClassList>& providers,
    const CIMPropertyList& propertyList,
    const String& condition,
    const String& query,
    const String& queryLanguage,
    const CIMInstance& subscription,
    const AcceptLanguageList& acceptLangs,
    const ContentLanguageList& contentLangs,
    const String& userName,
    const String& authType)
{
    PEG_METHOD_ENTER(TRC_INDICATION_SERVICE,
        "IndicationService::_sendWaitCreateRequests");

    Array<ProviderClassList> indicationProviders =
        _getIndicationProvidersWithNamespaceClassList(providers);

    CIMValue propValue;
    Uint16 repeatNotificationPolicy;
    Array<ProviderClassList> acceptedProviders;
    acceptedProviders.clear();

    // If there are no providers to accept the subscription, just return
    if (indicationProviders.size() == 0)
    {
        PEG_METHOD_EXIT();
        return acceptedProviders;
    }

    //
    //  Get repeat notification policy value from subscription instance
    //
    propValue = subscription.getProperty(
        subscription.findProperty(
            _PROPERTY_REPEATNOTIFICATIONPOLICY)).getValue();
    propValue.get(repeatNotificationPolicy);

    //
    //  Send Create request to each provider
    //
    for (Uint32 i = 0; i < indicationProviders.size(); i++)
    {
        //
        //  Create the create subscription request
        //
        CIMCreateSubscriptionRequestMessage * request =
            new CIMCreateSubscriptionRequestMessage(
                XmlWriter::getNextMessageId(),
                indicationProviders[i].classList[0].nameSpace,
                subscription,
                indicationProviders[i].classList[0].classList,
                propertyList,
                repeatNotificationPolicy,
                query,
                QueueIdStack(_providerManager, getQueueId()),
                authType,
                userName);

        //
        //  Set operation context
        //
        request->operationContext.insert(ProviderIdContainer(
            indicationProviders[i].providerModule
            ,indicationProviders[i].provider
#ifdef PEGASUS_ENABLE_REMOTE_CMPI
            ,indicationProviders[i].isRemoteNameSpace
            ,indicationProviders[i].remoteInfo
#endif
            ));
        request->operationContext.insert(
            SubscriptionInstanceContainer(subscription));
        request->operationContext.insert(
            SubscriptionFilterConditionContainer(condition,queryLanguage));
        request->operationContext.insert(
            SubscriptionFilterQueryContainer(
                query,
                queryLanguage,
                indicationProviders[i].classList[0].nameSpace));
        request->operationContext.insert(IdentityContainer(userName));
        request->operationContext.set(
            ContentLanguageListContainer(contentLangs));
        request->operationContext.set(AcceptLanguageListContainer(acceptLangs));

        AsyncRequest *asyncRequest;
        if (!indicationProviders[i].controlProviderName.size())
        {
            asyncRequest = new AsyncLegacyOperationStart(
                0,
                _providerManager,
                request);
        }
        else
        {
           asyncRequest = new AsyncModuleOperationStart(
               0,
               _moduleController,
               indicationProviders[i]. controlProviderName,
               request);
        }

        AsyncReply * asyncReply = SendWait(asyncRequest);

        MessageType msgType = asyncReply->getType();
        PEGASUS_ASSERT((msgType == ASYNC_ASYNC_LEGACY_OP_RESULT) ||
                   (msgType == ASYNC_ASYNC_MODULE_OP_RESULT));

        CIMCreateSubscriptionResponseMessage *response;

        if (msgType == ASYNC_ASYNC_LEGACY_OP_RESULT)
        {
            response = reinterpret_cast<CIMCreateSubscriptionResponseMessage *>(
                (static_cast<AsyncLegacyOperationResult *>(
                    asyncReply))->get_result());
        }
        else
        {
            response = reinterpret_cast<CIMCreateSubscriptionResponseMessage *>(
                (static_cast<AsyncModuleOperationResult *>(
                    asyncReply))->get_result());
        }


        if (response->cimException.getCode() == CIM_ERR_SUCCESS)
        {
            _addProviderToAcceptedProviderList(
                acceptedProviders,
                indicationProviders[i]);
        }
        else
        {
            //
            //  Provider rejected the subscription
            //
            PEG_TRACE((TRC_INDICATION_SERVICE, Tracer::LEVEL2,
                "Provider (%s) rejected create subscription: %s",
                (const char*)indicationProviders[i].provider.getPath()
                       .toString().getCString(),
                (const char*)response->cimException.getMessage().getCString()));
        }

        delete response;
        delete asyncRequest;
        delete asyncReply;
    }  //  for each indication provider
    PEG_METHOD_EXIT();
    return acceptedProviders;
}

void IndicationService::_sendWaitModifyRequests(
     const Array<ProviderClassList>& providers,
     const CIMPropertyList& propertyList,
     const String& condition,
     const String& query,
     const String& queryLanguage,
     const CIMInstance& subscription,
     const AcceptLanguageList& acceptLangs,
     const ContentLanguageList& contentLangs,
     const String& userName,
     const String& authType)
{
    PEG_METHOD_ENTER(TRC_INDICATION_SERVICE,
        "IndicationService::_sendWaitModifyRequests");

    Array<ProviderClassList> indicationProviders =
        _getIndicationProvidersWithNamespaceClassList(providers);

    CIMValue propValue;
    Uint16 repeatNotificationPolicy;

    // If there are no providers to accept the subscription update, just return
    if (indicationProviders.size() == 0)
    {
        PEG_METHOD_EXIT();
        return;
    }

    //
    //  Get repeat notification policy value from subscription instance
    //
    propValue = subscription.getProperty(
        subscription.findProperty(
            _PROPERTY_REPEATNOTIFICATIONPOLICY)).getValue();
    propValue.get(repeatNotificationPolicy);

    //
    //  Send Modify request to each provider
    //
    for (Uint32 i = 0; i < indicationProviders.size(); i++)
    {
        CIMModifySubscriptionRequestMessage * request =
            new CIMModifySubscriptionRequestMessage(
                XmlWriter::getNextMessageId(),
                indicationProviders[i].classList[0].nameSpace,
                subscription,
                indicationProviders[i].classList[0].classList,
                propertyList,
                repeatNotificationPolicy,
                query,
                QueueIdStack(_providerManager, getQueueId()),
                authType,
                userName);

        //
        //  Set operation context
        //
        request->operationContext.insert(ProviderIdContainer(
            indicationProviders[i].providerModule
            ,indicationProviders[i].provider
#ifdef PEGASUS_ENABLE_REMOTE_CMPI
            ,indicationProviders[i].isRemoteNameSpace
            ,indicationProviders[i].remoteInfo
#endif
            ));
        request->operationContext.insert(
            SubscriptionInstanceContainer(subscription));
        request->operationContext.insert(
            SubscriptionFilterConditionContainer(condition,queryLanguage));
        request->operationContext.insert(
            SubscriptionFilterQueryContainer(
                query,
                queryLanguage,
                indicationProviders[i].classList[0].nameSpace));
        request->operationContext.insert(IdentityContainer(userName));
        request->operationContext.set(
            ContentLanguageListContainer(contentLangs));
        request->operationContext.set(AcceptLanguageListContainer(acceptLangs));


        AsyncRequest *asyncRequest;
        if (!indicationProviders[i].controlProviderName.size())
        {
            asyncRequest = new AsyncLegacyOperationStart(
                0,
                _providerManager,
                request);
        }
        else
        {
           asyncRequest = new AsyncModuleOperationStart(
               0,
               _moduleController,
               indicationProviders[i].controlProviderName,
               request);
        }

        AsyncReply * asyncReply = SendWait(asyncRequest);

        MessageType msgType = asyncReply->getType();
        PEGASUS_ASSERT((msgType == ASYNC_ASYNC_LEGACY_OP_RESULT) ||
                   (msgType == ASYNC_ASYNC_MODULE_OP_RESULT));

        CIMModifySubscriptionResponseMessage *response;

        if (msgType == ASYNC_ASYNC_LEGACY_OP_RESULT)
        {
            response = reinterpret_cast<CIMModifySubscriptionResponseMessage *>(
                (static_cast<AsyncLegacyOperationResult *>(
                    asyncReply))->get_result());
        }
        else
        {
            response = reinterpret_cast<CIMModifySubscriptionResponseMessage *>(
                (static_cast<AsyncModuleOperationResult *>(
                    asyncReply))->get_result());
        }

        if (!(response->cimException.getCode() == CIM_ERR_SUCCESS))
        {
            //
            //  Provider rejected the subscription
            //
            PEG_TRACE((TRC_INDICATION_SERVICE, Tracer::LEVEL2,
                "Provider (%s) rejected modify subscription: %s",
                (const char*)indicationProviders[i].provider.getPath()
                       .toString().getCString(),
                (const char*)response->cimException.getMessage().getCString()));
        }

        delete response;
        delete asyncRequest;
        delete asyncReply;
    }  //  for each indication provider

    PEG_METHOD_EXIT();
}

void IndicationService::_sendAsyncDeleteRequests(
    const Array<ProviderClassList>& providers,
    const CIMInstance& subscription,
    const AcceptLanguageList& acceptLangs,
    const ContentLanguageList& contentLangs,
    const CIMRequestMessage * origRequest,
    const Array<NamespaceClassList>& indicationSubclasses,
    const String& userName,
    const String& authType)
{
    PEG_METHOD_ENTER(TRC_INDICATION_SERVICE,
        "IndicationService::_sendAsyncDeleteRequests");

    Array<ProviderClassList> indicationProviders =
        _getIndicationProvidersWithNamespaceClassList(providers);

    // If there are no providers to delete the subscription, just return
    if (indicationProviders.size() == 0)
    {
        PEG_METHOD_EXIT();
        return;
    }

#ifdef PEGASUS_ENABLE_DMTF_INDICATION_PROFILE_SUPPORT
    _asyncRequestsPending++;
    AutoPtr<AtomicInt, DecAtomicInt> counter(&_asyncRequestsPending);
#endif

    //
    //  Update subscription hash tables
    //
    _subscriptionTable->removeSubscription(
        subscription,
        indicationSubclasses,
        indicationProviders);

    CIMRequestMessage * aggRequest = 0;

    if (origRequest == 0)
    {
        //
        //  Delete a referencing or expired subscription -- no request
        //  associated with this delete
        //
        aggRequest = 0;
    }
    else
    {
        //
        //  Delete Instance or Modify Instance
        //
        switch (origRequest->getType())
        {
            case CIM_DELETE_INSTANCE_REQUEST_MESSAGE:
            {
                CIMDeleteInstanceRequestMessage * request =
                    (CIMDeleteInstanceRequestMessage *) origRequest;
                CIMDeleteInstanceRequestMessage * requestCopy =
                    new CIMDeleteInstanceRequestMessage(*request);
                aggRequest = requestCopy;
                break;
            }

            case CIM_MODIFY_INSTANCE_REQUEST_MESSAGE:
            {
                CIMModifyInstanceRequestMessage * request =
                    (CIMModifyInstanceRequestMessage *) origRequest;
                CIMModifyInstanceRequestMessage * requestCopy =
                    new CIMModifyInstanceRequestMessage(*request);
                aggRequest = requestCopy;
                break;
            }

            default:
            {
                PEGASUS_UNREACHABLE(PEGASUS_ASSERT(false);)
                break;
            }
        }
    }

    //
    //  Create an aggregate object for the delete subscription requests
    //
    IndicationOperationAggregate * operationAggregate =
        new IndicationOperationAggregate(
            aggRequest,
            indicationProviders[0].controlProviderName,
            indicationSubclasses);

    operationAggregate->setNumberIssued(indicationProviders.size());

    //
    //  Send Delete request to each provider
    //
    for (Uint32 i = 0; i < indicationProviders.size(); i++)
    {
        CIMDeleteSubscriptionRequestMessage * request =
            new CIMDeleteSubscriptionRequestMessage(
                XmlWriter::getNextMessageId(),
                indicationProviders[i].classList[0].nameSpace,
                subscription,
                indicationProviders[i].classList[0].classList,
                QueueIdStack(_providerManager, getQueueId()),
                authType,
                userName);

        //
        //  Store a copy of the request in the operation aggregate instance
        //
        CIMDeleteSubscriptionRequestMessage * requestCopy =
            new CIMDeleteSubscriptionRequestMessage(*request);
        requestCopy->operationContext.insert(ProviderIdContainer(
            indicationProviders[i].providerModule
            ,indicationProviders[i].provider
#ifdef PEGASUS_ENABLE_REMOTE_CMPI
            ,indicationProviders[i].isRemoteNameSpace
            ,indicationProviders[i].remoteInfo
#endif
             ));
        operationAggregate->appendRequest(requestCopy);
        request->operationContext.insert(ProviderIdContainer(
            indicationProviders[i].providerModule
            ,indicationProviders[i].provider
#ifdef PEGASUS_ENABLE_REMOTE_CMPI
            ,indicationProviders[i].isRemoteNameSpace
            ,indicationProviders[i].remoteInfo
#endif
            ));

        request->operationContext.insert(
            SubscriptionInstanceContainer(subscription));
        request->operationContext.insert(IdentityContainer(userName));
        request->operationContext.set(
            ContentLanguageListContainer(contentLangs));
        request->operationContext.set(AcceptLanguageListContainer(acceptLangs));

        AsyncOpNode * op = this->get_op();

        Uint32 serviceId;
        if (!indicationProviders[i].controlProviderName.size())
        {
            // constructor puts the object itself into a linked list
            // DO NOT remove the new !!!
            new AsyncLegacyOperationStart(
                op,
                _providerManager,
                request);
            serviceId = _providerManager;
        }
        else
        {
            // constructor puts the object itself into a linked list
            // DO NOT remove the new !!!
            new AsyncModuleOperationStart(
                op,
                _moduleController,
                indicationProviders[i].controlProviderName,
                request);
           serviceId = _moduleController;
        }

        SendAsync(
            op,
            serviceId,
            IndicationService::_aggregationCallBack,
            this,
            operationAggregate);

#ifdef PEGASUS_ENABLE_DMTF_INDICATION_PROFILE_SUPPORT
       // Release AutomicInt if atleast one request is sent for aggregation.
       counter.release();
#endif
    }

    PEG_METHOD_EXIT();
}

void IndicationService::_sendWaitDeleteRequests(
    const Array<ProviderClassList>& providers,
    const CIMInstance& subscription,
    const AcceptLanguageList& acceptLangs,
    const ContentLanguageList& contentLangs,
    const String& userName,
    const String& authType)
{
    PEG_METHOD_ENTER(TRC_INDICATION_SERVICE,
        "IndicationService::_sendWaitDeleteRequests");

    Array<ProviderClassList> indicationProviders =
        _getIndicationProvidersWithNamespaceClassList(providers);

    // If there are no providers to delete the subscription, just return
    if (indicationProviders.size() == 0)
    {
        PEG_METHOD_EXIT();
        return;
    }

    //
    //  Send Delete request to each provider
    //
    for (Uint32 i = 0; i < indicationProviders.size(); i++)
    {
        CIMDeleteSubscriptionRequestMessage * request =
            new CIMDeleteSubscriptionRequestMessage(
                XmlWriter::getNextMessageId(),
                indicationProviders[i].classList[0].nameSpace,
                subscription,
                indicationProviders[i].classList[0].classList,
                QueueIdStack(_providerManager, getQueueId()),
                authType,
                userName);

        //
        //  Set operation context
        //
        request->operationContext.insert(ProviderIdContainer(
            indicationProviders[i].providerModule
            ,indicationProviders[i].provider
#ifdef PEGASUS_ENABLE_REMOTE_CMPI
            ,indicationProviders[i].isRemoteNameSpace
            ,indicationProviders[i].remoteInfo
#endif
            ));
        request->operationContext.insert(
            SubscriptionInstanceContainer(subscription));
        request->operationContext.insert(IdentityContainer(userName));
        request->operationContext.set(
            ContentLanguageListContainer(contentLangs));
        request->operationContext.set(AcceptLanguageListContainer(acceptLangs));

        AsyncRequest *asyncRequest;
        if (!indicationProviders[i].controlProviderName.size())
        {
            asyncRequest =
                new AsyncLegacyOperationStart(
                    0,
                    _providerManager,
                    request);
        }
        else
        {
           asyncRequest =
               new AsyncModuleOperationStart(
                   0,
                   _moduleController,
                   indicationProviders[i].controlProviderName,
                   request);
        }

        AsyncReply * asyncReply = SendWait(asyncRequest);

        MessageType msgType = asyncReply->getType();
        PEGASUS_ASSERT((msgType == ASYNC_ASYNC_LEGACY_OP_RESULT) ||
                   (msgType == ASYNC_ASYNC_MODULE_OP_RESULT));

        CIMDeleteSubscriptionResponseMessage *response;

        if (msgType == ASYNC_ASYNC_LEGACY_OP_RESULT)
        {
            response = reinterpret_cast<CIMDeleteSubscriptionResponseMessage *>(
                (static_cast<AsyncLegacyOperationResult *>(
                    asyncReply))->get_result());
        }
        else
        {
            response = reinterpret_cast<CIMDeleteSubscriptionResponseMessage *>(
                (static_cast<AsyncModuleOperationResult *>(
                    asyncReply))->get_result());
        }

        if (!(response->cimException.getCode() == CIM_ERR_SUCCESS))
        {
            //
            //  Provider rejected the subscription
            //
            PEG_TRACE((TRC_INDICATION_SERVICE, Tracer::LEVEL2,
                "Provider (%s) rejected delete subscription: %s",
                (const char*)indicationProviders[i].provider.getPath()
                       .toString().getCString(),
                (const char*)response->cimException.getMessage().getCString()));
        }

        delete response;
        delete asyncRequest;
        delete asyncReply;
    }  //  for each indication provider

    PEG_METHOD_EXIT();
}

void IndicationService::_aggregationCallBack(
    AsyncOpNode * op,
    MessageQueue * q,
    void * userParameter)
{
    PEG_METHOD_ENTER(TRC_INDICATION_SERVICE,
        "IndicationService::_aggregationCallBack");

    IndicationService * service = static_cast<IndicationService *>(q);

    AsyncRequest * asyncRequest =
        static_cast<AsyncRequest *>(op->removeRequest());
    AsyncReply * asyncReply = static_cast<AsyncReply *>(op->removeResponse());

    IndicationOperationAggregate * operationAggregate =
        reinterpret_cast<IndicationOperationAggregate *>(userParameter);
    PEGASUS_ASSERT(operationAggregate != 0);

    CIMResponseMessage * response = 0;
    MessageType msgType = asyncReply->getType();
    PEGASUS_ASSERT((msgType == ASYNC_ASYNC_LEGACY_OP_RESULT) ||
                   (msgType == ASYNC_ASYNC_MODULE_OP_RESULT));

    if (msgType == ASYNC_ASYNC_LEGACY_OP_RESULT)
    {
        response = reinterpret_cast<CIMResponseMessage *>(
            (static_cast<AsyncLegacyOperationResult *>(
                asyncReply))->get_result());
    }
    else if (msgType == ASYNC_ASYNC_MODULE_OP_RESULT)
    {
        response = reinterpret_cast<CIMResponseMessage *>(
            (static_cast<AsyncModuleOperationResult *>(
                asyncReply))->get_result());
    }

    PEGASUS_ASSERT(response != 0);

    delete asyncRequest;
    delete asyncReply;
    service->return_op(op);

    Boolean isDoneAggregation = operationAggregate->appendResponse(response);
    if (isDoneAggregation)
    {
        service->_handleOperationResponseAggregation(operationAggregate);
#ifdef PEGASUS_ENABLE_DMTF_INDICATION_PROFILE_SUPPORT
        service->_asyncRequestsPending--;
#endif
    }

    PEG_METHOD_EXIT();
}

void IndicationService::_handleOperationResponseAggregation(
    IndicationOperationAggregate * operationAggregate)
{
    PEG_METHOD_ENTER(TRC_INDICATION_SERVICE,
        "IndicationService::_handleOperationResponseAggregation");

    switch (operationAggregate->getRequest(0)->getType())
    {
        case CIM_CREATE_SUBSCRIPTION_REQUEST_MESSAGE:
        {
            _handleCreateResponseAggregation(operationAggregate);
            break;
        }

        case CIM_DELETE_SUBSCRIPTION_REQUEST_MESSAGE:
        {
            _handleDeleteResponseAggregation(operationAggregate);
            break;
        }

        default:
        {
            PEGASUS_UNREACHABLE(PEGASUS_ASSERT(false);)
            break;
        }
    }

    //
    //  Requests and responses are deleted in destructor
    //
    delete operationAggregate;

    PEG_METHOD_EXIT();
}

void IndicationService::_handleCreateResponseAggregation(
    IndicationOperationAggregate * operationAggregate)
{
    PEG_METHOD_ENTER(TRC_INDICATION_SERVICE,
        "IndicationService::_handleCreateResponseAggregation");

    Array<ProviderClassList> acceptedProviders;
    CIMObjectPath instanceRef;
    CIMException cimException;

    //
    //  Examine provider responses
    //
    acceptedProviders.clear();
    for (Uint32 i = 0; i < operationAggregate->getNumberResponses(); i++)
    {
        //
        //  Find provider from which response was sent
        //
        CIMResponseMessage * response = operationAggregate->getResponse(i);
        ProviderClassList provider = operationAggregate->findProvider(
            response->messageId);
        if (response->cimException.getCode() == CIM_ERR_SUCCESS)
        {
            //
            //  If response is SUCCESS, provider accepted the subscription
            //  Add provider to list of providers that accepted subscription
            //
            _addProviderToAcceptedProviderList(
                acceptedProviders,
                provider);
        }
        else
        {
            PEG_TRACE((TRC_INDICATION_SERVICE, Tracer::LEVEL2,
                "Provider (%s) rejected create subscription: %s",
                (const char*)
                     provider.provider.getPath().toString().getCString(),
                (const char*)response->cimException.getMessage().getCString()));
        }
    }

    CIMCreateSubscriptionRequestMessage * request =
        (CIMCreateSubscriptionRequestMessage *)
            operationAggregate->getRequest(0);

    if (operationAggregate->getOrigType() ==
        CIM_CREATE_INSTANCE_REQUEST_MESSAGE)
    {
        instanceRef = request->subscriptionInstance.getPath();
    }


#ifdef PEGASUS_ENABLE_DMTF_INDICATION_PROFILE_SUPPORT
    if (operationAggregate->getOrigRequest() == 0)
    {
            //
            // There is no request associated with the aggregation object.
            // This request must have been sent during the indication
            // service initialization because of timeout specified.
            //
            _updateAcceptedSubscription(
                request->subscriptionInstance,
                acceptedProviders,
                operationAggregate->getIndicationSubclasses());
    }
    else
#endif
    if (acceptedProviders.size() == 0)
    {
        //
        //  No providers accepted this subscription
        //
        if (operationAggregate->requiresResponse())
        {
            //
            //  For Create Instance or Modify Instance request, set CIM
            //  exception for response
            //
            cimException = PEGASUS_CIM_EXCEPTION_L(CIM_ERR_NOT_SUPPORTED,
                MessageLoaderParms(
                    "IndicationService.IndicationService._MSG_NOT_ACCEPTED",
                    "No providers accepted the subscription."));
        }
    }
    else
    {
        //
        //  At least one provider accepted the subscription
        //
        if (operationAggregate->getOrigType() ==
            CIM_CREATE_INSTANCE_REQUEST_MESSAGE)
        {
            //
            //  Create Instance -- create the instance in the repository
            //
            CIMCreateInstanceRequestMessage * origRequest =
                (CIMCreateInstanceRequestMessage *)
                    operationAggregate->getOrigRequest();

            CIMInstance instance;
            try
            {
                instanceRef = _subscriptionRepository->createInstance(
                    request->subscriptionInstance, origRequest->nameSpace,
                    ((IdentityContainer)origRequest->operationContext.get
                        (IdentityContainer::NAME)).getUserName(),
                    ((AcceptLanguageListContainer)request->operationContext.get
                        (AcceptLanguageListContainer::NAME)).getLanguages(),
                    ((ContentLanguageListContainer)request->operationContext.get
                        (ContentLanguageListContainer::NAME)).getLanguages(),
                        true);
                instanceRef.setNameSpace(
                    request->subscriptionInstance.getPath().getNameSpace());
                instance = _subscriptionRepository->getInstance(
                    origRequest->nameSpace, instanceRef);
                instance.setPath(instanceRef);
            }
            catch (CIMException& exception)
            {
                cimException = exception;
            }
            catch (Exception& exception)
            {
                cimException = PEGASUS_CIM_EXCEPTION(
                    CIM_ERR_FAILED, exception.getMessage());
            }

            if (cimException.getCode() == CIM_ERR_SUCCESS)
            {
                //
                //  Insert entries into the subscription hash tables
                //
                _subscriptionTable->insertSubscription(
                    instance,
                    acceptedProviders,
                    operationAggregate->getIndicationSubclasses());

            }
        }
        else  //  CIM_MODIFY_INSTANCE_REQUEST_MESSAGE
        {

            PEGASUS_ASSERT(operationAggregate->getOrigType() ==
                CIM_MODIFY_INSTANCE_REQUEST_MESSAGE);
            //
            //  Insert entries into the subscription hash tables
            //
            _subscriptionTable->insertSubscription(
                request->subscriptionInstance,
                acceptedProviders,
                operationAggregate->getIndicationSubclasses());
        }
    }

    // If subscription could not be created, cancel create subscription request
    // or commit create subscription request if subscription was created.
    if (instanceRef.getKeyBindings().size())
    {
        if (cimException.getCode() != CIM_ERR_SUCCESS)
        {
            _cancelCreateSubscription(instanceRef);
        }
        else
        {
            _commitCreateSubscription(instanceRef);
        }
    }

    //
    //  For Create Instance or Modify Instance request, send response
    //
    if (operationAggregate->requiresResponse())
    {
        if (operationAggregate->getOrigType() ==
            CIM_CREATE_INSTANCE_REQUEST_MESSAGE)
        {
            // Note: don't need to set Content-language in the response
            CIMCreateInstanceResponseMessage* response =
                dynamic_cast<CIMCreateInstanceResponseMessage*>(
                    operationAggregate->getOrigRequest()->buildResponse());
            PEGASUS_ASSERT(response != 0);
            response->cimException = cimException;

            // put correct SystemName in place
            _setSubscriptionSystemName(
                instanceRef,
                System::getFullyQualifiedHostName());

            response->instanceName = instanceRef;
            _enqueueResponse(operationAggregate->getOrigRequest(), response);
        }

        else  //  CIM_MODIFY_INSTANCE_REQUEST_MESSAGE
        {
            PEGASUS_ASSERT(operationAggregate->getOrigType () ==
                CIM_MODIFY_INSTANCE_REQUEST_MESSAGE);
            // l10n
            // Note: don't need to set Content-language in the response
            //
            CIMResponseMessage * response =
                operationAggregate->getOrigRequest()->buildResponse();
            response->cimException = cimException;
            _enqueueResponse(operationAggregate->getOrigRequest(), response);
        }
    }

    PEG_METHOD_EXIT();
}

void IndicationService::_handleDeleteResponseAggregation(
    IndicationOperationAggregate * operationAggregate)
{
    PEG_METHOD_ENTER(TRC_INDICATION_SERVICE,
        "IndicationService::_handleDeleteResponseAggregation");

    CIMException cimException;

    //
    //  Examine provider responses
    //
    for (Uint32 i = 0; i < operationAggregate->getNumberResponses(); i++)
    {
        //
        //  Find provider from which response was sent and add to list
        //
        CIMResponseMessage * response = operationAggregate->getResponse(i);
        ProviderClassList provider = operationAggregate->findProvider(
            response->messageId);

        //
        //  If response is not SUCCESS, provider rejected the delete
        //
        if (response->cimException.getCode() != CIM_ERR_SUCCESS)
        {
            //
            //  Log a trace message
            //
            PEG_TRACE((TRC_INDICATION_SERVICE, Tracer::LEVEL2,
                "Provider (%s) rejected delete subscription: %s",
                (const char*)
                     provider.provider.getPath().toString().getCString(),
                (const char*)response->cimException.getMessage().getCString()));
        }
    }

    //
    //  For Delete Instance or Modify Instance request, send response
    //
    if (operationAggregate->requiresResponse())
    {
        CIMResponseMessage * response;
        if (operationAggregate->getOrigType() ==
            CIM_DELETE_INSTANCE_REQUEST_MESSAGE)
        {
            // l10n
            // Note: don't need to set Content-language in the response
            response = operationAggregate->getOrigRequest()->buildResponse();
            response->cimException = cimException;
        }

        else  //  CIM_MODIFY_INSTANCE_REQUEST_MESSAGE
        {
            PEGASUS_ASSERT(operationAggregate->getOrigType() ==
                CIM_MODIFY_INSTANCE_REQUEST_MESSAGE);
            // l10n
            // Note: don't need to set Content-language in the response
            response = operationAggregate->getOrigRequest()->buildResponse();
            response->cimException = cimException;
        }

        _enqueueResponse(operationAggregate->getOrigRequest(), response);
    }

    PEG_METHOD_EXIT();
}

CIMInstance IndicationService::_createAlertInstance(
    const CIMName& alertClassName,
    const Array<CIMInstance>& subscriptions)
{
    PEG_METHOD_ENTER(TRC_INDICATION_SERVICE,
        "IndicationService::_createAlertInstance");

    CIMInstance indicationInstance(alertClassName);

    //
    //  Add property values for all required properties of CIM_AlertIndication
    //
    indicationInstance.addProperty(
        CIMProperty(_PROPERTY_ALERTTYPE, CIMValue((Uint16) _TYPE_OTHER)));
    //
    //  ATTN: what should Other Alert Type value be??
    //  Currently using Alert class name
    //
    indicationInstance.addProperty(
        CIMProperty(_PROPERTY_OTHERALERTTYPE, alertClassName.getString()));

    indicationInstance.addProperty(
        CIMProperty(_PROPERTY_PERCEIVEDSEVERITY,
            CIMValue((Uint16) _SEVERITY_WARNING)));
    //
    //  ATTN: what should Probable Cause value be??
    //  Currently using Unknown
    //
    indicationInstance.addProperty(
        CIMProperty(_PROPERTY_PROBABLECAUSE,
            CIMValue((Uint16) _CAUSE_UNKNOWN)));

    //
    //  Add properties specific to each alert class
    //  ATTN: update once alert classes have been defined
    //  NB: for _CLASS_NO_PROVIDER_ALERT and _CLASS_PROVIDER_TERMINATED_ALERT,
    //  one of the properties will be a list of affected subscriptions
    //  It is for that reason that subscriptions is passed in as a parameter
    //
    if (alertClassName.equal(_CLASS_CIMOM_SHUTDOWN_ALERT))
    {
    }
    else if (alertClassName.equal(_CLASS_NO_PROVIDER_ALERT))
    {
    }
    else if (alertClassName.equal(_CLASS_PROVIDER_TERMINATED_ALERT))
    {
    }

    PEG_METHOD_EXIT();
    return indicationInstance;
}


#if 0
void IndicationService::_sendAlertsCallBack(AsyncOpNode *op,
    MessageQueue *q,
    void *parm)
{
    PEG_METHOD_ENTER(TRC_INDICATION_SERVICE,
        "IndicationService::_sendAlertsCallBack");

   IndicationService *service =
      static_cast<IndicationService *>(q);
   CIMInstance *_handler =
      reinterpret_cast<CIMInstance *>(parm);

   AsyncRequest *asyncRequest = static_cast<AsyncRequest *>(op->get_request());
   AsyncReply *asyncReply = static_cast<AsyncReply *>(op->get_response());
   CIMRequestMessage *request = reinterpret_cast<CIMRequestMessage *>(
      (static_cast<AsyncLegacyOperationStart *>(asyncRequest))->get_action());

   CIMHandleIndicationResponseMessage* response =
      reinterpret_cast<CIMHandleIndicationResponseMessage *>(
          (static_cast<AsyncLegacyOperationResult *>(
              asyncReply))->get_result());

   PEGASUS_ASSERT(response != 0);
   if (response->cimException.getCode() == CIM_ERR_SUCCESS)
   {
   }
   else
   {
   }

   //
   //  ATTN: Check for return value indicating invalid queue ID
   //  If received, need to find Handler Manager Service queue ID
   //  again
   //

// << Mon Jul 15 09:59:16 2002 mdd >> handler is allocated as an element in
// an array, don't delete here.
//   delete _handler;
   delete request;
   delete response;
   delete asyncRequest;
   delete asyncReply;
   op->release();
   service->return_op(op);

    PEG_METHOD_EXIT();
}


void IndicationService::_sendAlerts(
    const Array<CIMInstance>& subscriptions,
    /* const */ CIMInstance& alertInstance)
{
    PEG_METHOD_ENTER(TRC_INDICATION_SERVICE, "IndicationService::_sendAlerts");

    CIMInstance current;

    PEG_TRACE((TRC_INDICATION_SERVICE, Tracer::LEVEL4,
        "Sending alert: %s",
        (const char*)alertInstance.getClassName().getString().getCString()));

    //
    //  Get list of unique handler instances for all subscriptions in list
    //
    for (Uint32 i = 0; i < subscriptions.size(); i++)
    {
        PEG_TRACE((TRC_INDICATION_SERVICE, Tracer::LEVEL4,
            "Alert subscription: %s",
            (const char*)subscriptions[i].getPath().toString().getCString()));

        //
        //  Get handler instance
        //
        current = _subscriptionRepository->getHandler(subscriptions[i]);

    // ATTN: For the handlers which do not need subscription instance
    // need to check duplicate alter

            //
            //  Send handle indication request to the handler
            //
            CIMHandleIndicationRequestMessage * handler_request =
                new CIMHandleIndicationRequestMessage(
                    XmlWriter::getNextMessageId(),
                    current.getPath().getNameSpace(),
                    current,
            subscriptions[i],
                    alertInstance,
                    QueueIdStack(_handlerService, getQueueId()));

            AsyncOpNode* op = this->get_op();

            AsyncLegacyOperationStart *async_req =
                new AsyncLegacyOperationStart(
                    op,
                    _handlerService,
                    handler_request,
                    _queueId);

            SendAsync(op,
                  _handlerService,
                  IndicationService::_sendAlertsCallBack,
                  this,
                  (void *)&current);

    }

    PEG_METHOD_EXIT();
}
#endif

void IndicationService::sendSubscriptionInitComplete()
{
    PEG_METHOD_ENTER(TRC_INDICATION_SERVICE,
        "IndicationService::sendSubscriptionInitComplete");

    if (_getEnabledState() ==  _ENABLEDSTATE_DISABLED)
    {
        PEG_METHOD_EXIT();
        return;
    }
    //
    //  Create the Subscription Init Complete request
    //
    CIMSubscriptionInitCompleteRequestMessage * request =
        new CIMSubscriptionInitCompleteRequestMessage(
            XmlWriter::getNextMessageId(),
            QueueIdStack(_providerManager, getQueueId()));

    CIMSubscriptionInitCompleteRequestMessage *requestCopy =
        new CIMSubscriptionInitCompleteRequestMessage(*request);
    //
    //  Send Subscription Initialization Complete request to provider manager
    //  Provider Manager calls providers' enableIndications method
    //
    AsyncLegacyOperationStart * asyncRequest =
        new AsyncLegacyOperationStart(
            0,
            _providerManager,
            requestCopy);

    AutoPtr<AsyncReply> asyncReply(SendWait(asyncRequest));
    //
    //  Note: the response does not contain interesting data
    //
    delete asyncRequest;

    // Now send to all indication internal control providers

    for (ControlProvIndRegTable::Iterator j =
        _controlProvIndRegTable.start (); j; j++)
    {
        ControlProvIndReg reg = j.value();
        String controlProviderName;
        reg.provider.getProperty(reg.provider.findProperty(
            PEGASUS_PROPERTYNAME_NAME)).getValue().get(controlProviderName);
        requestCopy = new CIMSubscriptionInitCompleteRequestMessage(*request);
        AsyncModuleOperationStart * asyncRequest =
            new AsyncModuleOperationStart(
                0,
                _moduleController,
                controlProviderName,
                requestCopy);

        AutoPtr<AsyncReply> asyncReply(SendWait(asyncRequest));
        delete asyncRequest;
    }

    delete request;

    PEG_METHOD_EXIT();
}

Boolean IndicationService::_getCreator(
    const CIMInstance& instance,
    String& creator) const
{
    PEG_METHOD_ENTER(TRC_INDICATION_SERVICE, "IndicationService::_getCreator");

    Uint32 creatorIndex = instance.findProperty(
        PEGASUS_PROPERTYNAME_INDSUB_CREATOR);
    if (creatorIndex != PEG_NOT_FOUND)
    {
        CIMValue creatorValue = instance.getProperty(creatorIndex).getValue();
        if (creatorValue.isNull())
        {
            PEG_TRACE_CSTRING(TRC_INDICATION_SERVICE,Tracer::LEVEL1,
                "Null Subscription Creator property value");

            //
            //  This is a corrupted/invalid instance
            //
            PEG_METHOD_EXIT();
            return false;
        }
        else if ((creatorValue.getType() != CIMTYPE_STRING) ||
                 (creatorValue.isArray()))
        {
            PEG_TRACE((TRC_INDICATION_SERVICE,Tracer::LEVEL1,
                "Subscription Creator property value of incorrect type:%s %s",
                (creatorValue.isArray()) ? " array of" : " ",
                cimTypeToString(creatorValue.getType())));

            //
            //  This is a corrupted/invalid instance
            //
            PEG_METHOD_EXIT();
            return false;
        }
        else
        {
            creatorValue.get(creator);
        }
    }
    else
    {
        PEG_TRACE_CSTRING(TRC_INDICATION_SERVICE,Tracer::LEVEL1,
            "Missing Subscription Creator property");

        //
        //  This is a corrupted/invalid instance
        //
        PEG_METHOD_EXIT();
        return false;
    }

    PEG_METHOD_EXIT();
    return true;
}

void IndicationService::_updatePropertyList(
    CIMName& className,
    CIMPropertyList& propertyList,
    Boolean& setTimeRemaining,
    Boolean& startTimeAdded,
    Boolean& durationAdded)
{
    PEG_METHOD_ENTER( TRC_INDICATION_SERVICE,
        "IndicationService::_updatePropertyList");

    //
    //  A null propertyList means all properties
    //  If the class is Subscription, that includes the Time Remaining property
    //
    if (className.equal(PEGASUS_CLASSNAME_INDSUBSCRIPTION) ||
        className.equal(PEGASUS_CLASSNAME_FORMATTEDINDSUBSCRIPTION))
    {
        setTimeRemaining = true;
    }
    else
    {
        setTimeRemaining = false;
    }
    startTimeAdded = false;
    durationAdded = false;
    if (!propertyList.isNull())
    {
        setTimeRemaining = false;
        Array<CIMName> properties = propertyList.getPropertyNameArray();

        //
        //  Add Creator to property list
        //
        if (!ContainsCIMName(properties,
            PEGASUS_PROPERTYNAME_INDSUB_CREATOR))
        {
            properties.append(PEGASUS_PROPERTYNAME_INDSUB_CREATOR);
        }

        if (className.equal(PEGASUS_CLASSNAME_INDHANDLER_CIMXML) ||
            className.equal(PEGASUS_CLASSNAME_LSTNRDST_CIMXML))
        {
            properties.append(PEGASUS_PROPERTYNAME_LSTNRDST_CREATIONTIME);
        }

        //
        //  If a Subscription and Time Remaining is requested,
        //  Ensure Subscription Duration and Start Time are in property list
        //
        if (className.equal(PEGASUS_CLASSNAME_INDSUBSCRIPTION) ||
            className.equal(PEGASUS_CLASSNAME_FORMATTEDINDSUBSCRIPTION))
        {
            if (ContainsCIMName(properties, _PROPERTY_TIMEREMAINING))
            {
                setTimeRemaining = true;
                if (!ContainsCIMName(properties, _PROPERTY_STARTTIME))
                {
                    properties.append(_PROPERTY_STARTTIME);
                    startTimeAdded = true;
                }
                if (!ContainsCIMName(properties, _PROPERTY_DURATION))
                {
                    properties.append(_PROPERTY_DURATION);
                    durationAdded = true;
                }
            }
        }
        propertyList.clear();
        propertyList.set(properties);
    }

    PEG_METHOD_EXIT();
}

String IndicationService::_getSubscriptionLogString(CIMInstance& subscription)
{
    //
    //  Get Subscription Filter namespace and Name, and Handler namespace and
    //  Name
    //
    String logString;
    CIMValue filterValue;
    CIMObjectPath filterPath;
    CIMNamespaceName filterNS;
    Array<CIMKeyBinding> filterKeyBindings;
    CIMValue handlerValue;
    CIMObjectPath handlerPath;
    CIMNamespaceName handlerNS;
    Array<CIMKeyBinding> handlerKeyBindings;
    filterValue = subscription.getProperty(subscription.findProperty(
        PEGASUS_PROPERTYNAME_FILTER)).getValue();
    filterValue.get(filterPath);

    //
    //  Get Filter namespace - if not set in Filter reference property
    //  value, namespace is the namespace of the subscription
    //
    filterNS = filterPath.getNameSpace();
    if (filterNS.isNull())
    {
        filterNS = subscription.getPath().getNameSpace();
    }
    logString.append(filterNS.getString());
    logString.append(" ");
    filterKeyBindings = filterPath.getKeyBindings();
    for (Uint32 i = 0; i < filterKeyBindings.size(); i++)
    {
        if (filterKeyBindings[i].getName().equal(PEGASUS_PROPERTYNAME_NAME))
        {
            logString.append(filterKeyBindings[i].getValue());
            logString.append(", ");
            break;
        }
    }
    handlerValue = subscription.getProperty(
        subscription.findProperty(PEGASUS_PROPERTYNAME_HANDLER)).getValue();
    handlerValue.get(handlerPath);

    //
    //  Get Handler namespace - if not set in Handler reference property
    //  value, namespace is the namespace of the subscription
    //
    handlerNS = handlerPath.getNameSpace();
    if (handlerNS.isNull())
    {
        handlerNS = subscription.getPath().getNameSpace();
    }
    logString.append(handlerNS.getString());
    logString.append(" ");
    handlerKeyBindings = handlerPath.getKeyBindings();
    for (Uint32 j = 0; j < handlerKeyBindings.size(); j++)
    {
        if (handlerKeyBindings[j].getName().equal(PEGASUS_PROPERTYNAME_NAME))
        {
            logString.append(handlerKeyBindings[j].getValue());
            break;
        }
    }

    return logString;
}

String IndicationService::getProviderLogString(CIMInstance& provider)
{
    String logString;

    logString = provider.getProperty(
        provider.findProperty(PEGASUS_PROPERTYNAME_NAME)).getValue().toString();

    return logString;
}

CIMClass IndicationService::_getIndicationClass(
    const CIMInstance& subscriptionInstance)
{
    PEG_METHOD_ENTER(TRC_INDICATION_SERVICE,
    "IndicationService::_getIndicationClass");

    Array<CIMNamespaceName> sourceNamespaces;
    String query;
    String queryLanguage;
    CIMName indicationClassName;
    CIMClass indicationClass;
    String filterName;

    //  Get filter properties
    _subscriptionRepository->getFilterProperties(subscriptionInstance, query,
        sourceNamespaces, queryLanguage, filterName);

    //  Build the query expression from the filter query
    QueryExpression queryExpression = _getQueryExpression(query,
                              queryLanguage,
                              sourceNamespaces[0]);

    //  Get indication class name from filter query
    indicationClassName = _getIndicationClassName(
    queryExpression, sourceNamespaces[0]);

    //
    //  Get the indication class object from the repository
    //  Specify localOnly=false because superclass properties are needed
    //  Specify includeQualifiers=false because qualifiers are not needed
    //
    indicationClass = _subscriptionRepository->getClass(
        sourceNamespaces[0], indicationClassName, false, false, false,
        CIMPropertyList());

    PEG_METHOD_EXIT();
    return indicationClass;
}

void IndicationService::_getRelevantSubscriptions(
    const Array<CIMObjectPath> & providedSubscriptionNames,
    const CIMName& className,
    const CIMNamespaceName& nameSpace,
    const CIMInstance& indicationProvider,
    Array<CIMInstance>& subscriptions,
    Array<SubscriptionKey>& subscriptionKeys)
{
    PEG_METHOD_ENTER(TRC_INDICATION_SERVICE,
        "IndicationService::_getRelevantlSubscriptions");

    //
    // Retrieves list of enabled subscription instances in the specified
    // namespace, where the subscription indication class matches or is a
    // superclass of the supported class. A subscription is only included
    // in the list if the specified provider accepted the subscription.
    //
    _subscriptionTable->getMatchingClassNamespaceSubscriptions(
        className,
        nameSpace,
        indicationProvider,
        subscriptions,
        subscriptionKeys);

    //
    // If the indication provider included subscriptions in the
    // SubscriptionInstanceNamesContainer, the subset of subscriptions
    // specified by the indication provider that also appear in the initial
    // subscriptions list is returned.
    //
    if (providedSubscriptionNames.size() > 0)
    {
        Uint32 n = providedSubscriptionNames.size();
        Array<SubscriptionKey> provSubKeys(n);
        for (Uint32 i = 0; i < n; i++)
        {
            provSubKeys.append(SubscriptionKey(providedSubscriptionNames[i]));
        }
        for (Uint32 i = 0; i < subscriptions.size(); i++)
        {
            if (!Contains(provSubKeys, subscriptionKeys[i]))
            {
                subscriptions.remove(i);
                subscriptionKeys.remove(i);
                i--;
            }
        }
    }

    PEGASUS_ASSERT(subscriptions.size() == subscriptionKeys.size());
    PEG_METHOD_EXIT();
}

Boolean IndicationService::_subscriptionMatch(
    const CIMInstance& subscription,
    const CIMInstance& indication,
    const CIMPropertyList& supportedPropertyList,
    QueryExpression& queryExpr,
    const CIMNamespaceName sourceNameSpace)
{
    PEG_METHOD_ENTER(TRC_INDICATION_SERVICE,
        "IndicationService::_subscriptionMatch");

    //
    // If supported properties is null (all properties)
    // the subscription can be supported
    //
    if (!supportedPropertyList.isNull ())
    {
        try
        {
            // Get the class paths in the FROM list
            // Since neither WQL nor CQL support joins, so we can
            // assume one class path.
            CIMName indicationClassName =
                queryExpr.getClassPathList()[0].getClassName();

            if (!_subscriptionRepository->validateIndicationClassName(
                indicationClassName, sourceNameSpace))
            {
                //
                // Invalid FROM class, the subscription does not match
                //
                PEG_METHOD_EXIT();
                return false;
            }

            //
            //  Get required property list from filter query (WHERE clause)
            //
            //  Note:
            //  The class should be the class of the indication
            //  instance, not the FROM class.
            //  This is needed because CQL can have class scoping operators
            //  on properties that may not be the same class
            //  as the FROM class.  The required properties
            //  for an indication are based on indication instance class,
            //  not the FROM class.
            //

            CIMPropertyList requiredPropertyList = _getPropertyList(
                queryExpr, sourceNameSpace, indication.getClassName());

            //
            //  If the subscription requires all properties,
            //  but supported property list does not include all
            //  properties, the subscription cannot be supported
            //
            if (requiredPropertyList.isNull ())
            {
                //
                //  Current subscription does not match
                //
                PEG_METHOD_EXIT();
                return false;
            }
            else
            {
                //
                //  Compare subscription required property list
                //  with supported property list
                //
                for (Uint32 j = 0; j < requiredPropertyList.size (); j++)
                {
                    if (!ContainsCIMName
                        (supportedPropertyList.getPropertyNameArray(),
                         requiredPropertyList[j]))
                    {
                        //
                        //  Current subscription does not match
                        //
                        PEG_METHOD_EXIT();
                        return false;
                    }
                }
            }
        }
        catch(const Exception & e)
        {
            // This subscription is invalid
           PEG_TRACE ((TRC_DISCARDED_DATA, Tracer::LEVEL1,
                "Exception caught trying to verify required properties "
                    "in a subscription are all contained in the list of "
                    "supported indication properties: %s",
                    (const char *) e.getMessage ().getCString()));
            PEG_METHOD_EXIT();
            return false;
        }
        catch(const exception & e)
        {
            // This subscription is invalid
           PEG_TRACE ((TRC_DISCARDED_DATA, Tracer::LEVEL1,
                "Exception caught trying to verify required properties "
                    "in a subscription are all contained in the list of "
                    "supported indication properties: %s", e.what ()));
            PEG_METHOD_EXIT();
            return false;
        }
        catch(...)
        {
            // This subscription is invalid
            // skip it
            PEG_TRACE_CSTRING (TRC_DISCARDED_DATA, Tracer::LEVEL1,
                "Unknown exception caught trying to verify "
                    "required properties in a subscription are all contained "
                    "in the list of supported indication properties.");
            PEG_METHOD_EXIT();
            return false;
        }
    }

    //
    // Check for expired subscription
    //
    try
    {
        if (_isExpired(subscription))
        {
            // Delete expired subscription
            CIMObjectPath path = subscription.getPath ();
            _deleteExpiredSubscription (path);
#ifdef PEGASUS_ENABLE_DMTF_INDICATION_PROFILE_SUPPORT
            _sendSubscriptionNotActiveMessagetoHandlerService(path);
#endif
            PEG_TRACE ((TRC_INDICATION_GENERATION, Tracer::LEVEL3,
                "%s Indication Subscription expired",
                (const char*)(indication.getClassName().getString().
                    getCString())));
            PEG_METHOD_EXIT();
            return false;
        }
    }
    catch (DateTimeOutOfRangeException&)
    {
        PEG_TRACE_CSTRING(TRC_INDICATION_SERVICE, Tracer::LEVEL2,
            "Caught DateTimeOutOfRangeException in IndicationService while"
                "checking for expired subscription");
        PEG_METHOD_EXIT();
        return false;
    }

    //
    // Evaluate whether the filter criteria are met by the generated
    // indication
    //
    if (!queryExpr.evaluate(indication))
    {
        PEG_METHOD_EXIT();
        return false;
    }

    PEG_METHOD_EXIT();
    return true;
}

Boolean IndicationService::_formatIndication(
    CIMInstance& formattedIndication,
    QueryExpression& queryExpr,
    const Array<CIMName>& providerSupportedProperties,
    const Array<CIMName>& indicationClassProperties)
{
    PEG_METHOD_ENTER(TRC_INDICATION_SERVICE,
        "IndicationService::_formatIndication");

    //
    // Call QueryExpression::applyProjection to remove properties
    // not listed in the SELECT clause.  Note: for CQL,
    // this will handle properties on embedded objects.
    //
    // QueryExpression::applyProjection throws an exception if
    // the indication is missing a required property in the SELECT
    // clause.  Although we have checked for the indication missing
    // required properties, it would have not detected missing required
    // embedded object properties for CQL.  So, we need to catch the
    // missing property exception here.
    //
    try
    {
        queryExpr.applyProjection(formattedIndication, true);
    }
    catch (QueryRuntimePropertyException& re)
    {
        // The indication was missing a required property.
        PEG_TRACE((TRC_INDICATION_SERVICE, Tracer::LEVEL1,
            "Apply Projection error: %s",
            (const char*)re.getMessage().getCString()));
        PEG_METHOD_EXIT();
        return false;
    }

    //
    // Remove any properties that may be left on the indication
    // that are not in the indication class.  These are properties
    // added by the provider incorrectly.  It is possible that
    // these properties will remain after applyProjection if the
    // SELECT clause happens to have a property name not on the
    // indication class, and the indication has that same property.
    // Note: If SELECT includes all properties ("*"), it's still
    // necessary to check, in case the provider added properties
    // not in the indication class.
    //
    for (Uint32 j = 0; j < providerSupportedProperties.size(); j++)
    {
        Uint32 rmIndex =
            formattedIndication.findProperty(providerSupportedProperties[j]);
        if (rmIndex != PEG_NOT_FOUND &&
            !ContainsCIMName(
                 indicationClassProperties, providerSupportedProperties[j]))
        {
            formattedIndication.removeProperty(rmIndex);
        }
    }

    PEG_METHOD_EXIT();
    return true;
}

void IndicationService::_forwardIndToHandler(
    const CIMInstance& matchedSubscription,
    const CIMInstance& handlerInstance,
    const CIMInstance& formattedIndication,
    const CIMNamespaceName& namespaceName,
    const OperationContext& operationContext,
    DeliveryStatusAggregator *deliveryStatusAggregator)
{
    PEG_METHOD_ENTER(TRC_INDICATION_SERVICE,
        "IndicationService::_forwardIndToHandler");

    CIMHandleIndicationRequestMessage *handler_request =
        new CIMHandleIndicationRequestMessage (
            XmlWriter::getNextMessageId (),
            namespaceName,
            handlerInstance,
            formattedIndication,
            matchedSubscription,
            QueueIdStack(_handlerService, getQueueId()),
            String::EMPTY,
            String::EMPTY);

    handler_request->operationContext = operationContext;
    handler_request->deliveryStatusAggregator = deliveryStatusAggregator;

    AsyncLegacyOperationStart *async_req =
        new AsyncLegacyOperationStart(
        0,
        _handlerService,
        handler_request);

    PEG_TRACE((TRC_INDICATION_SERVICE, Tracer::LEVEL4,
        "Sending (SendAsync) Indication to %s "
        "via CIMHandleIndicationRequestMessage",
        (MessageQueue::lookup(_handlerService) ?
         MessageQueue::lookup(_handlerService)->getQueueName() :
        "BAD queue name")));

    SendForget(async_req);
    if (deliveryStatusAggregator)
    {
        deliveryStatusAggregator->incExpectedResponseCount();
    }
    PEG_METHOD_EXIT();
}

PEGASUS_NAMESPACE_END
