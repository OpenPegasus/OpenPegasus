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

#include "ProviderMessageHandler.h"

#include <exception>

#include <Pegasus/Common/OperationContextInternal.h>
#include <Pegasus/Common/Tracer.h>
#include <Pegasus/Common/StatisticalData.h>
#include <Pegasus/Common/Logger.h>
#include <Pegasus/Common/MessageLoader.h>
#include <Pegasus/Common/Constants.h>

#include <Pegasus/Provider/CIMInstanceQueryProvider.h>
#include <Pegasus/Provider/CIMAssociationProvider.h>
#include <Pegasus/Provider/CIMMethodProvider.h>
#include <Pegasus/Provider/CIMIndicationProvider.h>
#include <Pegasus/Provider/CIMIndicationConsumerProvider.h>

#include <Pegasus/Query/QueryExpression/QueryExpression.h>
#include <Pegasus/ProviderManager2/QueryExpressionFactory.h>

#include <Pegasus/ProviderManager2/SimpleResponseHandler.h>
#include <Pegasus/ProviderManager2/OperationResponseHandler.h>
#include <Pegasus/ProviderManager2/AutoPThreadSecurity.h>

#define HANDLE_PROVIDER_CALL(traceString, providerCall, handler)       \
    do                                                                 \
    {                                                                  \
        PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL4,                \
            "Calling provider." #traceString ": %s",                   \
            (const char*)_fullyQualifiedProviderName.getCString()));   \
        try                                                            \
        {                                                              \
            providerCall;                                              \
        }                                                              \
        catch (CIMException& e)                                        \
        {                                                              \
            PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL1,            \
                "Provider CIMException: %s",                           \
                (const char*)e.getMessage().getCString()));            \
            handler.setCIMException(e);                                \
        }                                                              \
        catch (Exception& e)                                           \
        {                                                              \
            PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL1,            \
                "Provider Exception: %s",                              \
                (const char*)e.getMessage().getCString()));            \
            handler.setStatus(                                         \
                CIM_ERR_FAILED,                                        \
                e.getContentLanguages(),                               \
                e.getMessage());                                       \
        }                                                              \
        catch (const PEGASUS_STD(exception)& e)                        \
        {                                                              \
            PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL1,            \
                "Provider exception: %s", e.what()));                  \
            handler.setStatus(CIM_ERR_FAILED, e.what());               \
        }                                                              \
        catch (...)                                                    \
        {                                                              \
            PEG_TRACE_CSTRING(TRC_PROVIDERMANAGER, Tracer::LEVEL1,     \
                "Provider unknown exception");                         \
            handler.setStatus(CIM_ERR_FAILED, "Unknown error.");       \
        }                                                              \
        PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL4,                \
            "Returned from provider." #traceString ": %s",             \
            (const char*)_fullyQualifiedProviderName.getCString()));   \
    } while (0)

PEGASUS_NAMESPACE_BEGIN

template<class T>
inline T* getProviderInterface(CIMProvider* provider)
{
    T* p = dynamic_cast<T*>(provider);

    if (p == 0)
    {
        throw PEGASUS_CIM_EXCEPTION_L(CIM_ERR_NOT_SUPPORTED, MessageLoaderParms(
            "ProviderManager.ProviderFacade.INVALID_PROVIDER_INTERFACE",
            "Invalid provider interface."));
    }

    return p;
}

//
// Default Provider Manager
//
ProviderMessageHandler::ProviderMessageHandler(
    const String& moduleName,
    const String& name,
    CIMProvider* provider,
    PEGASUS_INDICATION_CALLBACK_T indicationCallback,
    PEGASUS_RESPONSE_CHUNK_CALLBACK_T responseChunkCallback,
    Boolean subscriptionInitComplete)
    : _name(name),
      _fullyQualifiedProviderName(moduleName + ":" + name),
      _provider(provider),
      _indicationCallback(indicationCallback),
      _responseChunkCallback(responseChunkCallback),
      _subscriptionInitComplete(subscriptionInitComplete),
      _indicationResponseHandler(0)
{
}

ProviderMessageHandler::~ProviderMessageHandler()
{
}

String ProviderMessageHandler::getName() const
{
    return _name;
}

CIMProvider* ProviderMessageHandler::getProvider()
{
    return _provider;
}

void ProviderMessageHandler::setProvider(CIMProvider* provider)
{
    _provider = provider;
}

void ProviderMessageHandler::initialize(CIMOMHandle& cimom)
{
    PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL4,
        "Calling provider.initialize: %s",
        (const char*)_fullyQualifiedProviderName.getCString()));

    try
    {
        _provider->initialize(cimom);
    }
    catch (CIMException& e)
    {
        PEG_TRACE((TRC_DISCARDED_DATA, Tracer::LEVEL1,
            "Caught CIMexception from provider %s initialize() method."
            " Code: %u Msg: %s",
            (const char*)_fullyQualifiedProviderName.getCString(),
             e.getCode(), cimStatusCodeToString(e.getCode()) ));
        throw;
    }
    catch (Exception& e)
    {

        PEG_TRACE((TRC_DISCARDED_DATA, Tracer::LEVEL1,
            "Caught Exception from provider %s initialize() method. %s",
            (const char*)_fullyQualifiedProviderName.getCString(),
            (const char*) e.getMessage().getCString() ));
        throw;
    }
     catch (...)
    {
        PEG_TRACE((TRC_DISCARDED_DATA, Tracer::LEVEL1,
            "Caught exception from provider %s initialize() method.",
            (const char*)_fullyQualifiedProviderName.getCString() ));
        throw;
    }

    PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL4,
        "Returned from provider.initialize: %s",
        (const char*)_fullyQualifiedProviderName.getCString()));
}

void ProviderMessageHandler::terminate()
{
    _disableIndications();

    PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL4,
        "Calling provider.terminate: %s",
        (const char*)_fullyQualifiedProviderName.getCString()));

    try
    {
        _provider->terminate();
    }
    catch (...)
    {
        PEG_TRACE((TRC_DISCARDED_DATA, Tracer::LEVEL1,
            "Caught exception from provider %s terminate() method.",
            (const char*)_fullyQualifiedProviderName.getCString()));
        throw;
    }

    PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL4,
        "Returned from provider.terminate: %s",
        (const char*)_fullyQualifiedProviderName.getCString()));
}

void ProviderMessageHandler::subscriptionInitComplete()
{
    if (status.testSubscriptions())
    {
        _enableIndications();
    }

    _subscriptionInitComplete = true;
}

void ProviderMessageHandler::indicationServiceDisabled()
{
    _subscriptionInitComplete = false;
}

CIMResponseMessage* ProviderMessageHandler::processMessage(
    CIMRequestMessage* request)
{
    PEG_METHOD_ENTER(TRC_PROVIDERMANAGER,
        "ProviderMessageHandler::processMessage()");

    CIMResponseMessage* response = 0;

    try
    {
        // pass the request message to a handler method based on message type
        switch(request->getType())
        {
        case CIM_GET_INSTANCE_REQUEST_MESSAGE:
            response = _handleGetInstanceRequest(request);
            break;

        case CIM_ENUMERATE_INSTANCES_REQUEST_MESSAGE:
            response = _handleEnumerateInstancesRequest(request);
            break;

        case CIM_ENUMERATE_INSTANCE_NAMES_REQUEST_MESSAGE:
            response = _handleEnumerateInstanceNamesRequest(request);
            break;

        case CIM_CREATE_INSTANCE_REQUEST_MESSAGE:
            response = _handleCreateInstanceRequest(request);
            break;

        case CIM_MODIFY_INSTANCE_REQUEST_MESSAGE:
            response = _handleModifyInstanceRequest(request);
            break;

        case CIM_DELETE_INSTANCE_REQUEST_MESSAGE:
            response = _handleDeleteInstanceRequest(request);
            break;

        case CIM_EXEC_QUERY_REQUEST_MESSAGE:
            response = _handleExecQueryRequest(request);
            break;

        case CIM_ASSOCIATORS_REQUEST_MESSAGE:
            response = _handleAssociatorsRequest(request);
            break;

        case CIM_ASSOCIATOR_NAMES_REQUEST_MESSAGE:
            response = _handleAssociatorNamesRequest(request);
            break;

        case CIM_REFERENCES_REQUEST_MESSAGE:
            response = _handleReferencesRequest(request);
            break;

        case CIM_REFERENCE_NAMES_REQUEST_MESSAGE:
            response = _handleReferenceNamesRequest(request);
            break;

        case CIM_GET_PROPERTY_REQUEST_MESSAGE:
            response = _handleGetPropertyRequest(request);
            break;

        case CIM_SET_PROPERTY_REQUEST_MESSAGE:
            response = _handleSetPropertyRequest(request);
            break;

        case CIM_INVOKE_METHOD_REQUEST_MESSAGE:
            response = _handleInvokeMethodRequest(request);
            break;

        case CIM_CREATE_SUBSCRIPTION_REQUEST_MESSAGE:
            response = _handleCreateSubscriptionRequest(request);
            break;

        case CIM_MODIFY_SUBSCRIPTION_REQUEST_MESSAGE:
            response = _handleModifySubscriptionRequest(request);
            break;

        case CIM_DELETE_SUBSCRIPTION_REQUEST_MESSAGE:
            response = _handleDeleteSubscriptionRequest(request);
            break;

        case CIM_EXPORT_INDICATION_REQUEST_MESSAGE:
            response = _handleExportIndicationRequest(request);
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

OperationContext ProviderMessageHandler::_createProviderOperationContext(
    const OperationContext& context)
{
    OperationContext providerContext;

    providerContext.insert(context.get(IdentityContainer::NAME));
    providerContext.insert(context.get(AcceptLanguageListContainer::NAME));
    providerContext.insert(context.get(ContentLanguageListContainer::NAME));

    if (context.contains(UserRoleContainer::NAME))
    {
        providerContext.insert(context.get(UserRoleContainer::NAME));
    }

    return providerContext;
}

CIMResponseMessage* ProviderMessageHandler::_handleGetInstanceRequest(
    CIMRequestMessage* message)
{
    PEG_METHOD_ENTER(TRC_PROVIDERMANAGER,
        "ProviderMessageHandler::_handleGetInstanceRequest");

    CIMGetInstanceRequestMessage* request =
        dynamic_cast<CIMGetInstanceRequestMessage*>(message);
    PEGASUS_ASSERT(request != 0);

    AutoPtr<CIMGetInstanceResponseMessage> response(
        dynamic_cast<CIMGetInstanceResponseMessage*>(
            request->buildResponse()));
    PEGASUS_ASSERT(response.get() != 0);

    // create a handler for this request
    GetInstanceResponseHandler handler(
        request, response.get(), _responseChunkCallback);

    // make target object path
    CIMObjectPath objectPath(
        System::getHostName(),
        request->nameSpace,
        request->instanceName.getClassName(),
        request->instanceName.getKeyBindings());

    PEG_TRACE((
        TRC_PROVIDERMANAGER,
        Tracer::LEVEL3,
        "ProviderMessageHandler::_handleGetInstanceRequest - "
            "Object path: %s",
        (const char*) objectPath.toString().getCString()));

    OperationContext providerContext(
        _createProviderOperationContext(request->operationContext));

    AutoPThreadSecurity threadLevelSecurity(request->operationContext);

    CIMInstanceProvider* provider =
        getProviderInterface<CIMInstanceProvider>(_provider);

    StatProviderTimeMeasurement providerTime(response.get());

    HANDLE_PROVIDER_CALL(
        getInstance,
        provider->getInstance(
            providerContext,
            objectPath,
            request->includeQualifiers,
            request->includeClassOrigin,
            request->propertyList,
            handler),
        handler);

    PEG_METHOD_EXIT();
    return response.release();
}

CIMResponseMessage* ProviderMessageHandler::_handleEnumerateInstancesRequest(
    CIMRequestMessage* message)
{
    PEG_METHOD_ENTER(TRC_PROVIDERMANAGER,
        "ProviderMessageHandler::_handleEnumerateInstanceRequest");

    CIMEnumerateInstancesRequestMessage* request =
        dynamic_cast<CIMEnumerateInstancesRequestMessage*>(message);
    PEGASUS_ASSERT(request != 0);

    AutoPtr<CIMEnumerateInstancesResponseMessage> response(
        dynamic_cast<CIMEnumerateInstancesResponseMessage*>(
            request->buildResponse()));
    PEGASUS_ASSERT(response.get() != 0);

    // create a handler for this request
    EnumerateInstancesResponseHandler handler(
        request, response.get(), _responseChunkCallback);

    // make target object path
    CIMObjectPath objectPath(
        System::getHostName(),
        request->nameSpace,
        request->className);

    PEG_TRACE((
        TRC_PROVIDERMANAGER,
        Tracer::LEVEL3,
        "ProviderMessageHandler::_handleEnumerateInstancesRequest - "
            "Object path: %s MessageId=%s",
        (const char*) objectPath.toString().getCString(),
        (const char*)message->messageId.getCString()));

    OperationContext providerContext(
        _createProviderOperationContext(request->operationContext));

    AutoPThreadSecurity threadLevelSecurity(request->operationContext);

    CIMInstanceProvider* provider =
        getProviderInterface<CIMInstanceProvider>(_provider);

    StatProviderTimeMeasurement providerTime(response.get());

    HANDLE_PROVIDER_CALL(
        enumerateInstances,
        provider->enumerateInstances(
            providerContext,
            objectPath,
            request->includeQualifiers,
            request->includeClassOrigin,
            request->propertyList,
            handler),
        handler);

    PEG_METHOD_EXIT();
    return response.release();
}

CIMResponseMessage*
    ProviderMessageHandler::_handleEnumerateInstanceNamesRequest(
        CIMRequestMessage* message)
{
    PEG_METHOD_ENTER(TRC_PROVIDERMANAGER,
        "ProviderMessageHandler::_handleEnumerateInstanceNamesRequest");

    CIMEnumerateInstanceNamesRequestMessage* request =
        dynamic_cast<CIMEnumerateInstanceNamesRequestMessage*>(message);
    PEGASUS_ASSERT(request != 0);

    AutoPtr<CIMEnumerateInstanceNamesResponseMessage> response(
        dynamic_cast<CIMEnumerateInstanceNamesResponseMessage*>(
            request->buildResponse()));
    PEGASUS_ASSERT(response.get() != 0);

    // create a handler for this request
    EnumerateInstanceNamesResponseHandler handler(
        request, response.get(), _responseChunkCallback);

    // make target object path
    CIMObjectPath objectPath(
        System::getHostName(),
        request->nameSpace,
        request->className);

    PEG_TRACE((
        TRC_PROVIDERMANAGER,
        Tracer::LEVEL3,
        "ProviderMessageHandler::_handleEnumerateInstanceNamesRequest - "
            "Object path: %s messageId=%s",
        (const char*) objectPath.toString().getCString(),
        (const char*)message->messageId.getCString() ));

    OperationContext providerContext(
        _createProviderOperationContext(request->operationContext));

    AutoPThreadSecurity threadLevelSecurity(request->operationContext);

    CIMInstanceProvider* provider =
        getProviderInterface<CIMInstanceProvider>(_provider);

    StatProviderTimeMeasurement providerTime(response.get());

    HANDLE_PROVIDER_CALL(
        enumerateInstanceNames,
        provider->enumerateInstanceNames(
            providerContext,
            objectPath,
            handler),
        handler);

    PEG_METHOD_EXIT();
    return response.release();
}

CIMResponseMessage* ProviderMessageHandler::_handleCreateInstanceRequest(
    CIMRequestMessage* message)
{
    PEG_METHOD_ENTER(TRC_PROVIDERMANAGER,
        "ProviderMessageHandler::_handleCreateInstanceRequest");

    CIMCreateInstanceRequestMessage* request =
        dynamic_cast<CIMCreateInstanceRequestMessage*>(message);
    PEGASUS_ASSERT(request != 0);

    AutoPtr<CIMCreateInstanceResponseMessage> response(
        dynamic_cast<CIMCreateInstanceResponseMessage*>(
            request->buildResponse()));
    PEGASUS_ASSERT(response.get() != 0);

    // create a handler for this request
    CreateInstanceResponseHandler handler(
        request, response.get(), _responseChunkCallback);

    // make target object path
    CIMObjectPath objectPath(
        System::getHostName(),
        request->nameSpace,
        request->newInstance.getPath().getClassName(),
        request->newInstance.getPath().getKeyBindings());

    PEG_TRACE((
        TRC_PROVIDERMANAGER,
        Tracer::LEVEL3,
        "ProviderMessageHandler::_handleCreateInstanceRequest - "
            "Object path: %s",
        (const char*) objectPath.toString().getCString()));

    OperationContext providerContext(
        _createProviderOperationContext(request->operationContext));

    AutoPThreadSecurity threadLevelSecurity(request->operationContext);

    CIMInstanceProvider* provider =
        getProviderInterface<CIMInstanceProvider>(_provider);

    StatProviderTimeMeasurement providerTime(response.get());

    HANDLE_PROVIDER_CALL(
        createInstance,
        provider->createInstance(
            providerContext,
            objectPath,
            request->newInstance,
            handler),
        handler);

    PEG_METHOD_EXIT();
    return response.release();
}

CIMResponseMessage* ProviderMessageHandler::_handleModifyInstanceRequest(
    CIMRequestMessage* message)
{
    PEG_METHOD_ENTER(TRC_PROVIDERMANAGER,
        "ProviderMessageHandler::_handleModifyInstanceRequest");

    CIMModifyInstanceRequestMessage* request =
        dynamic_cast<CIMModifyInstanceRequestMessage*>(message);
    PEGASUS_ASSERT(request != 0);

    AutoPtr<CIMModifyInstanceResponseMessage> response(
        dynamic_cast<CIMModifyInstanceResponseMessage*>(
            request->buildResponse()));
    PEGASUS_ASSERT(response.get() != 0);

    // create a handler for this request
    ModifyInstanceResponseHandler handler(
        request, response.get(), _responseChunkCallback);

    // make target object path
    CIMObjectPath objectPath(
        System::getHostName(),
        request->nameSpace,
        request->modifiedInstance.getPath().getClassName(),
        request->modifiedInstance.getPath().getKeyBindings());

    PEG_TRACE((
        TRC_PROVIDERMANAGER,
        Tracer::LEVEL3,
        "ProviderMessageHandler::_handleModifyInstanceRequest - "
            "Object path: %s",
        (const char*) objectPath.toString().getCString()));

    OperationContext providerContext(
        _createProviderOperationContext(request->operationContext));

    AutoPThreadSecurity threadLevelSecurity(request->operationContext);

    CIMInstanceProvider* provider =
        getProviderInterface<CIMInstanceProvider>(_provider);

    StatProviderTimeMeasurement providerTime(response.get());

    HANDLE_PROVIDER_CALL(
        modifyInstance,
        provider->modifyInstance(
            providerContext,
            objectPath,
            request->modifiedInstance,
            request->includeQualifiers,
            request->propertyList,
            handler),
        handler);

    PEG_METHOD_EXIT();
    return response.release();
}

CIMResponseMessage* ProviderMessageHandler::_handleDeleteInstanceRequest(
    CIMRequestMessage* message)
{
    PEG_METHOD_ENTER(TRC_PROVIDERMANAGER,
        "ProviderMessageHandler::_handleDeleteInstanceRequest");

    CIMDeleteInstanceRequestMessage* request =
        dynamic_cast<CIMDeleteInstanceRequestMessage*>(message);
    PEGASUS_ASSERT(request != 0);

    AutoPtr<CIMDeleteInstanceResponseMessage> response(
        dynamic_cast<CIMDeleteInstanceResponseMessage*>(
            request->buildResponse()));
    PEGASUS_ASSERT(response.get() != 0);

    // create a handler for this request
    DeleteInstanceResponseHandler handler(
        request, response.get(), _responseChunkCallback);

    // make target object path
    CIMObjectPath objectPath(
        System::getHostName(),
        request->nameSpace,
        request->instanceName.getClassName(),
        request->instanceName.getKeyBindings());

    PEG_TRACE((
        TRC_PROVIDERMANAGER,
        Tracer::LEVEL3,
        "ProviderMessageHandler::_handleDeleteInstanceRequest - "
            "Object path: %s",
        (const char*) objectPath.toString().getCString()));

    OperationContext providerContext(
        _createProviderOperationContext(request->operationContext));

    AutoPThreadSecurity threadLevelSecurity(request->operationContext);

    CIMInstanceProvider* provider =
        getProviderInterface<CIMInstanceProvider>(_provider);

    StatProviderTimeMeasurement providerTime(response.get());

    HANDLE_PROVIDER_CALL(
        deleteInstance,
        provider->deleteInstance(
            providerContext,
            objectPath,
            handler),
        handler);

    PEG_METHOD_EXIT();
    return response.release();
}

CIMResponseMessage* ProviderMessageHandler::_handleExecQueryRequest(
    CIMRequestMessage* message)
{
    PEG_METHOD_ENTER(TRC_PROVIDERMANAGER,
        "ProviderMessageHandler::_handleExecQueryRequest");

    CIMExecQueryRequestMessage* request =
        dynamic_cast<CIMExecQueryRequestMessage*>(message);
    PEGASUS_ASSERT(request != 0);

    AutoPtr<CIMExecQueryResponseMessage> response(
        dynamic_cast<CIMExecQueryResponseMessage*>(
            request->buildResponse()));
    PEGASUS_ASSERT(response.get() != 0);

    // create a handler for this request
    ExecQueryResponseHandler handler(
        request, response.get(), _responseChunkCallback);

    // make target object path
    CIMObjectPath objectPath(
        System::getHostName(),
        request->nameSpace,
        request->className);

    PEG_TRACE((
        TRC_PROVIDERMANAGER,
        Tracer::LEVEL3,
        "ProviderMessageHandler::_handleExecQueryRequest - "
            "Object path: %s MessageId=%s",
        (const char*) objectPath.toString().getCString(),
        (const char*)message->messageId.getCString()));

    QueryExpression qx(request->queryLanguage,request->query);

    OperationContext providerContext(
        _createProviderOperationContext(request->operationContext));

    AutoPThreadSecurity threadLevelSecurity(request->operationContext);

    CIMInstanceQueryProvider* provider =
        getProviderInterface<CIMInstanceQueryProvider>(_provider);

    StatProviderTimeMeasurement providerTime(response.get());

    HANDLE_PROVIDER_CALL(
        execQuery,
        provider->execQuery(
            providerContext,
            objectPath,
            qx,
            handler),
        handler);

    PEG_METHOD_EXIT();
    return response.release();
}

CIMResponseMessage* ProviderMessageHandler::_handleAssociatorsRequest(
    CIMRequestMessage* message)
{
    PEG_METHOD_ENTER(TRC_PROVIDERMANAGER,
        "ProviderMessageHandler::_handleAssociatorsRequest");

    CIMAssociatorsRequestMessage* request =
        dynamic_cast<CIMAssociatorsRequestMessage*>(message);
    PEGASUS_ASSERT(request != 0);

    AutoPtr<CIMAssociatorsResponseMessage> response(
        dynamic_cast<CIMAssociatorsResponseMessage*>(
            request->buildResponse()));
    PEGASUS_ASSERT(response.get() != 0);

    // create a handler for this request
    AssociatorsResponseHandler handler(
        request, response.get(), _responseChunkCallback);

    // make target object path
    CIMObjectPath objectPath(
        System::getHostName(),
        request->nameSpace,
        request->objectName.getClassName());

    objectPath.setKeyBindings(request->objectName.getKeyBindings());

    PEG_TRACE((
        TRC_PROVIDERMANAGER,
        Tracer::LEVEL3,
        "ProviderMessageHandler::_handleAssociatorsRequest - "
            "Object path: %s MessageId=%s",
        (const char*) objectPath.toString().getCString(),
        (const char*)message->messageId.getCString()));

    CIMObjectPath assocPath(
        System::getHostName(),
        request->nameSpace,
        request->assocClass.getString());

    OperationContext providerContext(
        _createProviderOperationContext(request->operationContext));

    AutoPThreadSecurity threadLevelSecurity(request->operationContext);

    CIMAssociationProvider* provider =
        getProviderInterface<CIMAssociationProvider>(_provider);

    StatProviderTimeMeasurement providerTime(response.get());

    HANDLE_PROVIDER_CALL(
        associators,
        provider->associators(
            providerContext,
            objectPath,
            request->assocClass,
            request->resultClass,
            request->role,
            request->resultRole,
            request->includeQualifiers,
            request->includeClassOrigin,
            request->propertyList,
            handler),
        handler);

    PEG_METHOD_EXIT();
    return response.release();
}

CIMResponseMessage* ProviderMessageHandler::_handleAssociatorNamesRequest(
    CIMRequestMessage* message)
{
    PEG_METHOD_ENTER(TRC_PROVIDERMANAGER,
        "ProviderMessageHandler::_handleAssociatorNamesRequest");

    CIMAssociatorNamesRequestMessage* request =
        dynamic_cast<CIMAssociatorNamesRequestMessage*>(message);
    PEGASUS_ASSERT(request != 0);

    AutoPtr<CIMAssociatorNamesResponseMessage> response(
        dynamic_cast<CIMAssociatorNamesResponseMessage*>(
            request->buildResponse()));
    PEGASUS_ASSERT(response.get() != 0);

    // create a handler for this request
    AssociatorNamesResponseHandler handler(
        request, response.get(), _responseChunkCallback);

    // make target object path
    CIMObjectPath objectPath(
        System::getHostName(),
        request->nameSpace,
        request->objectName.getClassName());

    objectPath.setKeyBindings(request->objectName.getKeyBindings());

    PEG_TRACE((
        TRC_PROVIDERMANAGER,
        Tracer::LEVEL3,
        "ProviderMessageHandler::_handleAssociationNamesRequest - "
            "Object path: %s MessageId=%s",
        (const char*) objectPath.toString().getCString(),
        (const char*)message->messageId.getCString()));

    CIMObjectPath assocPath(
        System::getHostName(),
        request->nameSpace,
        request->assocClass.getString());

    OperationContext providerContext(
        _createProviderOperationContext(request->operationContext));

    AutoPThreadSecurity threadLevelSecurity(request->operationContext);

    CIMAssociationProvider* provider =
        getProviderInterface<CIMAssociationProvider>(_provider);

    StatProviderTimeMeasurement providerTime(response.get());

    HANDLE_PROVIDER_CALL(
        associatorNames,
        provider->associatorNames(
            providerContext,
            objectPath,
            request->assocClass,
            request->resultClass,
            request->role,
            request->resultRole,
            handler),
        handler);

    PEG_METHOD_EXIT();
    return response.release();
}

CIMResponseMessage* ProviderMessageHandler::_handleReferencesRequest(
    CIMRequestMessage* message)
{
    PEG_METHOD_ENTER(TRC_PROVIDERMANAGER,
        "ProviderMessageHandler::_handleReferencesRequest");

    CIMReferencesRequestMessage* request =
        dynamic_cast<CIMReferencesRequestMessage*>(message);
    PEGASUS_ASSERT(request != 0);

    AutoPtr<CIMReferencesResponseMessage> response(
        dynamic_cast<CIMReferencesResponseMessage*>(
            request->buildResponse()));
    PEGASUS_ASSERT(response.get() != 0);

    // create a handler for this request
    ReferencesResponseHandler handler(
        request, response.get(), _responseChunkCallback);

    // make target object path
    CIMObjectPath objectPath(
        System::getHostName(),
        request->nameSpace,
        request->objectName.getClassName());

    objectPath.setKeyBindings(request->objectName.getKeyBindings());

    PEG_TRACE((
        TRC_PROVIDERMANAGER,
        Tracer::LEVEL3,
        "ProviderMessageHandler::_handleReferencesRequest - "
            "Object path: %s MessageId=%s",
        (const char*) objectPath.toString().getCString(),
        (const char*)message->messageId.getCString()));

    CIMObjectPath resultPath(
        System::getHostName(),
        request->nameSpace,
        request->resultClass.getString());

    OperationContext providerContext(
        _createProviderOperationContext(request->operationContext));

    AutoPThreadSecurity threadLevelSecurity(request->operationContext);

    CIMAssociationProvider* provider =
        getProviderInterface<CIMAssociationProvider>(_provider);

    StatProviderTimeMeasurement providerTime(response.get());

    HANDLE_PROVIDER_CALL(
        references,
        provider->references(
            providerContext,
            objectPath,
            request->resultClass,
            request->role,
            request->includeQualifiers,
            request->includeClassOrigin,
            request->propertyList,
            handler),
        handler);

    PEG_METHOD_EXIT();
    return response.release();
}

CIMResponseMessage* ProviderMessageHandler::_handleReferenceNamesRequest(
    CIMRequestMessage* message)
{
    PEG_METHOD_ENTER(TRC_PROVIDERMANAGER,
        "ProviderMessageHandler::_handleReferenceNamesRequest");

    CIMReferenceNamesRequestMessage* request =
        dynamic_cast<CIMReferenceNamesRequestMessage*>(message);
    PEGASUS_ASSERT(request != 0);

    AutoPtr<CIMReferenceNamesResponseMessage> response(
        dynamic_cast<CIMReferenceNamesResponseMessage*>(
            request->buildResponse()));
    PEGASUS_ASSERT(response.get() != 0);

    // create a handler for this request
    ReferenceNamesResponseHandler handler(
        request, response.get(), _responseChunkCallback);

    // make target object path
    CIMObjectPath objectPath(
        System::getHostName(),
        request->nameSpace,
        request->objectName.getClassName());

    objectPath.setKeyBindings(request->objectName.getKeyBindings());

    PEG_TRACE((
        TRC_PROVIDERMANAGER,
        Tracer::LEVEL3,
        "ProviderMessageHandler::_handleReferenceNamesRequest - "
            "Object path: %s MessageId=%s",
        (const char*) objectPath.toString().getCString(),
        (const char*)message->messageId.getCString()));

    // KS_TODO Confirm that the following is cruft and not needed.
//    CIMObjectPath resultPath(
//       System::getHostName(),
//        request->nameSpace,
//        request->resultClass.getString());

    OperationContext providerContext(
        _createProviderOperationContext(request->operationContext));

    AutoPThreadSecurity threadLevelSecurity(request->operationContext);

    CIMAssociationProvider* provider =
        getProviderInterface<CIMAssociationProvider>(_provider);

    StatProviderTimeMeasurement providerTime(response.get());

    HANDLE_PROVIDER_CALL(
        referenceNames,
        provider->referenceNames(
            providerContext,
            objectPath,
            request->resultClass,
            request->role,
            handler),
        handler);

    PEG_METHOD_EXIT();
    return response.release();
}

CIMResponseMessage* ProviderMessageHandler::_handleGetPropertyRequest(
    CIMRequestMessage* message)
{
    PEG_METHOD_ENTER(TRC_PROVIDERMANAGER,
        "ProviderMessageHandler::_handleGetPropertyRequest");

    CIMGetPropertyRequestMessage* request =
        dynamic_cast<CIMGetPropertyRequestMessage*>(message);
    PEGASUS_ASSERT(request != 0);

    AutoPtr<CIMGetPropertyResponseMessage> response(
        dynamic_cast<CIMGetPropertyResponseMessage*>(
            request->buildResponse()));
    PEGASUS_ASSERT(response.get() != 0);

    //
    // Translate the GetProperty request to a GetInstance request message
    //

    Array<CIMName> propertyList;
    propertyList.append(request->propertyName);

    CIMGetInstanceRequestMessage getInstanceRequest(
        request->messageId,
        request->nameSpace,
        request->instanceName,
        false,  // includeQualifiers
        false,  // includeClassOrigin
        propertyList,
        request->queueIds);

    getInstanceRequest.operationContext = request->operationContext;

    AutoPtr<CIMGetInstanceResponseMessage> getInstanceResponse(
        dynamic_cast<CIMGetInstanceResponseMessage*>(
            getInstanceRequest.buildResponse()));
    PEGASUS_ASSERT(getInstanceResponse.get() != 0);

    //
    // Process the GetInstance operation
    //

    // create a handler for this request (with chunking disabled)
    GetInstanceResponseHandler handler(
        &getInstanceRequest, getInstanceResponse.get(), 0);

    // make target object path
    CIMObjectPath objectPath(
        System::getHostName(),
        getInstanceRequest.nameSpace,
        getInstanceRequest.instanceName.getClassName(),
        getInstanceRequest.instanceName.getKeyBindings());

    PEG_TRACE((
        TRC_PROVIDERMANAGER,
        Tracer::LEVEL3,
        "ProviderMessageHandler::_handleGetPropertyRequest - "
            "Object path: %s, Property: %s",
        (const char*) objectPath.toString().getCString(),
        (const char*) request->propertyName.getString().getCString()));

    OperationContext providerContext(
        _createProviderOperationContext(getInstanceRequest.operationContext));

    AutoPThreadSecurity threadLevelSecurity(request->operationContext);

    CIMInstanceProvider* provider =
        getProviderInterface<CIMInstanceProvider>(_provider);

    StatProviderTimeMeasurement providerTime(response.get());

    HANDLE_PROVIDER_CALL(
        getInstance,
        provider->getInstance(
            providerContext,
            objectPath,
            getInstanceRequest.includeQualifiers,
            getInstanceRequest.includeClassOrigin,
            getInstanceRequest.propertyList,
            handler),
        handler);

    //
    // Copy the GetInstance response into the GetProperty response message
    //

    response->cimException = getInstanceResponse->cimException;

    if (response->cimException.getCode() == CIM_ERR_SUCCESS)
    {
        CIMInstance instance =
            getInstanceResponse->getResponseData().getInstance();

        Uint32 pos = instance.findProperty(request->propertyName);

        if (pos != PEG_NOT_FOUND)
        {
            response->value = instance.getProperty(pos).getValue();
        }
        else    // Property not found. Return CIM_ERR_NO_SUCH_PROPERTY.
        {
            response->cimException = PEGASUS_CIM_EXCEPTION(
                CIM_ERR_NO_SUCH_PROPERTY,
                request->propertyName.getString());
        }
    }

    response->operationContext = getInstanceResponse->operationContext;

    PEG_METHOD_EXIT();
    return response.release();
}

CIMResponseMessage* ProviderMessageHandler::_handleSetPropertyRequest(
    CIMRequestMessage* message)
{
    PEG_METHOD_ENTER(TRC_PROVIDERMANAGER,
        "ProviderMessageHandler::_handleSetPropertyRequest");

    CIMSetPropertyRequestMessage* request =
        dynamic_cast<CIMSetPropertyRequestMessage*>(message);
    PEGASUS_ASSERT(request != 0);

    AutoPtr<CIMSetPropertyResponseMessage> response(
        dynamic_cast<CIMSetPropertyResponseMessage*>(
            request->buildResponse()));
    PEGASUS_ASSERT(response.get() != 0);

    //
    // Translate the SetProperty request to a ModifyInstance request message
    //

    // make target object path
    CIMObjectPath objectPath(
        System::getHostName(),
        request->nameSpace,
        request->instanceName.getClassName(),
        request->instanceName.getKeyBindings());

    CIMInstance instance(request->instanceName.getClassName());
    instance.addProperty(CIMProperty(
        request->propertyName, request->newValue));
    instance.setPath(objectPath);

    Array<CIMName> propertyList;
    propertyList.append(request->propertyName);

    CIMModifyInstanceRequestMessage modifyInstanceRequest(
        request->messageId,
        request->nameSpace,
        instance,
        false,  // includeQualifiers
        propertyList,
        request->queueIds);

    modifyInstanceRequest.operationContext = request->operationContext;

    AutoPtr<CIMModifyInstanceResponseMessage> modifyInstanceResponse(
        dynamic_cast<CIMModifyInstanceResponseMessage*>(
            modifyInstanceRequest.buildResponse()));
    PEGASUS_ASSERT(modifyInstanceResponse.get() != 0);

    //
    // Process the ModifyInstance operation
    //

    // create a handler for this request (with chunking disabled)
    ModifyInstanceResponseHandler handler(
        &modifyInstanceRequest, modifyInstanceResponse.get(), 0);

    PEG_TRACE((
        TRC_PROVIDERMANAGER,
        Tracer::LEVEL3,
        "ProviderMessageHandler::_handleSetPropertyRequest - "
            "Object path: %s, Property: %s",
        (const char*) objectPath.toString().getCString(),
        (const char*) request->propertyName.getString().getCString()));

    OperationContext providerContext(
        _createProviderOperationContext(request->operationContext));

    AutoPThreadSecurity threadLevelSecurity(request->operationContext);

    CIMInstanceProvider* provider =
        getProviderInterface<CIMInstanceProvider>(_provider);

    StatProviderTimeMeasurement providerTime(response.get());

    HANDLE_PROVIDER_CALL(
        modifyInstance,
        provider->modifyInstance(
            providerContext,
            objectPath,
            modifyInstanceRequest.modifiedInstance,
            modifyInstanceRequest.includeQualifiers,
            modifyInstanceRequest.propertyList,
            handler),
        handler);

    //
    // Copy the ModifyInstance response into the GetProperty response message
    //

    response->cimException = modifyInstanceResponse->cimException;
    response->operationContext = modifyInstanceResponse->operationContext;

    PEG_METHOD_EXIT();
    return response.release();
}

CIMResponseMessage* ProviderMessageHandler::_handleInvokeMethodRequest(
    CIMRequestMessage* message)
{
    PEG_METHOD_ENTER(TRC_PROVIDERMANAGER,
        "ProviderMessageHandler::_handleInvokeMethodRequest");

    CIMInvokeMethodRequestMessage* request =
        dynamic_cast<CIMInvokeMethodRequestMessage*>(message);
    PEGASUS_ASSERT(request != 0);

    AutoPtr<CIMInvokeMethodResponseMessage> response(
        dynamic_cast<CIMInvokeMethodResponseMessage*>(
            request->buildResponse()));
    PEGASUS_ASSERT(response.get() != 0);

    // create a handler for this request
    InvokeMethodResponseHandler handler(
        request, response.get(), _responseChunkCallback);

    // make target object path
    CIMObjectPath objectPath(
        System::getHostName(),
        request->nameSpace,
        request->instanceName.getClassName(),
        request->instanceName.getKeyBindings());

    PEG_TRACE((
        TRC_PROVIDERMANAGER,
        Tracer::LEVEL3,
        "ProviderMessageHandler::_handleInvokeMethodRequest - "
            "Object path: %s, Method: %s",
        (const char*) objectPath.toString().getCString(),
        (const char*) request->methodName.getString().getCString()));

    OperationContext providerContext(
        _createProviderOperationContext(request->operationContext));

    AutoPThreadSecurity threadLevelSecurity(request->operationContext);

    CIMMethodProvider* provider =
        getProviderInterface<CIMMethodProvider>(_provider);

    StatProviderTimeMeasurement providerTime(response.get());

    HANDLE_PROVIDER_CALL(
        invokeMethod,
        provider->invokeMethod(
            providerContext,
            objectPath,
            request->methodName,
            request->inParameters,
            handler),
        handler);

    PEG_METHOD_EXIT();
    return response.release();
}

CIMResponseMessage* ProviderMessageHandler::_handleCreateSubscriptionRequest(
    CIMRequestMessage* message)
{
    PEG_METHOD_ENTER(TRC_PROVIDERMANAGER,
        "ProviderMessageHandler::_handleCreateSubscriptionRequest");

    CIMCreateSubscriptionRequestMessage* request =
        dynamic_cast<CIMCreateSubscriptionRequestMessage*>(message);
    PEGASUS_ASSERT(request != 0);

    AutoPtr<CIMCreateSubscriptionResponseMessage> response(
        dynamic_cast<CIMCreateSubscriptionResponseMessage*>(
            request->buildResponse()));
    PEGASUS_ASSERT(response.get() != 0);

    OperationResponseHandler handler(
        request, response.get(), _responseChunkCallback);

    String temp;

    for (Uint32 i = 0, n = request->classNames.size(); i < n; i++)
    {
        temp.append(request->classNames[i].getString());

        if (i < (n - 1))
        {
            temp.append(", ");
        }
    }

    PEG_TRACE((
        TRC_PROVIDERMANAGER,
        Tracer::LEVEL3,
        "ProviderMessageHandler::_handleCreateSubscriptionRequest - "
            "Host name: %s  Name space: %s  Class name(s): %s",
        (const char*) System::getHostName().getCString(),
        (const char*) request->nameSpace.getString().getCString(),
        (const char*) temp.getCString()));

    //
    //  Save the provider instance from the request
    //
    ProviderIdContainer pidc = (ProviderIdContainer)
        request->operationContext.get(ProviderIdContainer::NAME);
    status.setProviderInstance(pidc.getProvider());

    // convert arguments

    Array<CIMObjectPath> classNames;

    for (Uint32 i = 0, n = request->classNames.size(); i < n; i++)
    {
        CIMObjectPath className(
            System::getHostName(),
            request->nameSpace,
            request->classNames[i]);

        classNames.append(className);
    }

    OperationContext providerContext(
        _createProviderOperationContext(request->operationContext));
    providerContext.insert(request->operationContext.get(
        SubscriptionInstanceContainer::NAME));
    providerContext.insert(request->operationContext.get(
        SubscriptionFilterConditionContainer::NAME));
    providerContext.insert(request->operationContext.get(
        SubscriptionFilterQueryContainer::NAME));

    AutoPThreadSecurity threadLevelSecurity(request->operationContext);

    CIMIndicationProvider* provider =
        getProviderInterface<CIMIndicationProvider>(_provider);

    StatProviderTimeMeasurement providerTime(response.get());

    HANDLE_PROVIDER_CALL(
        createSubscription,
        provider->createSubscription(
            providerContext,
            request->subscriptionInstance.getPath(),
            classNames,
            request->propertyList,
            request->repeatNotificationPolicy),
        handler);

    //
    //  Increment count of current subscriptions for this provider
    //
    if (status.testIfZeroAndIncrementSubscriptions())
    {
        PEG_TRACE_CSTRING(TRC_PROVIDERMANAGER, Tracer::LEVEL4,
            "First accepted subscription");

        //
        //  If there were no current subscriptions before the increment,
        //  the first subscription has been created
        //  Call the provider's enableIndications method
        //
        if (_subscriptionInitComplete)
        {
            _enableIndications();
        }
    }

    PEG_METHOD_EXIT();
    return response.release();
}

CIMResponseMessage* ProviderMessageHandler::_handleModifySubscriptionRequest(
    CIMRequestMessage* message)
{
    PEG_METHOD_ENTER(TRC_PROVIDERMANAGER,
        "ProviderMessageHandler::_handleModifySubscriptionRequest");

    CIMModifySubscriptionRequestMessage* request =
        dynamic_cast<CIMModifySubscriptionRequestMessage*>(message);
    PEGASUS_ASSERT(request != 0);

    AutoPtr<CIMModifySubscriptionResponseMessage> response(
        dynamic_cast<CIMModifySubscriptionResponseMessage*>(
            request->buildResponse()));
    PEGASUS_ASSERT(response.get() != 0);

    OperationResponseHandler handler(
        request, response.get(), _responseChunkCallback);

    String temp;

    for (Uint32 i = 0, n = request->classNames.size(); i < n; i++)
    {
        temp.append(request->classNames[i].getString());

        if (i < (n - 1))
        {
            temp.append(", ");
        }
    }

    PEG_TRACE((
        TRC_PROVIDERMANAGER,
        Tracer::LEVEL3,
        "ProviderMessageHandler::_handleModifySubscriptionRequest - "
            "Host name: %s  Name space: %s  Class name(s): %s",
        (const char*) System::getHostName().getCString(),
        (const char*) request->nameSpace.getString().getCString(),
        (const char*) temp.getCString()));

    // convert arguments

    Array<CIMObjectPath> classNames;

    for (Uint32 i = 0, n = request->classNames.size(); i < n; i++)
    {
        CIMObjectPath className(
            System::getHostName(),
            request->nameSpace,
            request->classNames[i]);

        classNames.append(className);
    }

    OperationContext providerContext(
        _createProviderOperationContext(request->operationContext));
    providerContext.insert(request->operationContext.get(
        SubscriptionInstanceContainer::NAME));
    providerContext.insert(request->operationContext.get(
        SubscriptionFilterConditionContainer::NAME));
    providerContext.insert(request->operationContext.get(
        SubscriptionFilterQueryContainer::NAME));

    AutoPThreadSecurity threadLevelSecurity(request->operationContext);

    CIMIndicationProvider* provider =
        getProviderInterface<CIMIndicationProvider>(_provider);

    StatProviderTimeMeasurement providerTime(response.get());

    HANDLE_PROVIDER_CALL(
        modifySubscription,
        provider->modifySubscription(
            providerContext,
            request->subscriptionInstance.getPath(),
            classNames,
            request->propertyList,
            request->repeatNotificationPolicy),
        handler);

    PEG_METHOD_EXIT();
    return response.release();
}

CIMResponseMessage* ProviderMessageHandler::_handleDeleteSubscriptionRequest(
    CIMRequestMessage* message)
{
    PEG_METHOD_ENTER(TRC_PROVIDERMANAGER,
        "ProviderMessageHandler::_handleDeleteSubscriptionRequest");

    CIMDeleteSubscriptionRequestMessage* request =
        dynamic_cast<CIMDeleteSubscriptionRequestMessage*>(message);
    PEGASUS_ASSERT(request != 0);

    AutoPtr<CIMDeleteSubscriptionResponseMessage> response(
        dynamic_cast<CIMDeleteSubscriptionResponseMessage*>(
            request->buildResponse()));
    PEGASUS_ASSERT(response.get() != 0);

    OperationResponseHandler handler(
        request, response.get(), _responseChunkCallback);

    String temp;

    for (Uint32 i = 0, n = request->classNames.size(); i < n; i++)
    {
        temp.append(request->classNames[i].getString());

        if (i < (n - 1))
        {
            temp.append(", ");
        }
    }

    PEG_TRACE((
        TRC_PROVIDERMANAGER,
        Tracer::LEVEL3,
        "ProviderMessageHandler::_handleDeleteSubscriptionRequest - "
            "Host name: %s  Name space: %s  Class name(s): %s",
        (const char*) System::getHostName().getCString(),
        (const char*) request->nameSpace.getString().getCString(),
        (const char*) temp.getCString()));

    Array<CIMObjectPath> classNames;

    for (Uint32 i = 0, n = request->classNames.size(); i < n; i++)
    {
        CIMObjectPath className(
            System::getHostName(),
            request->nameSpace,
            request->classNames[i]);

        classNames.append(className);
    }

    OperationContext providerContext(
        _createProviderOperationContext(request->operationContext));
    providerContext.insert(request->operationContext.get(
        SubscriptionInstanceContainer::NAME));

    AutoPThreadSecurity threadLevelSecurity(request->operationContext);

    CIMIndicationProvider* provider =
        getProviderInterface<CIMIndicationProvider>(_provider);

    StatProviderTimeMeasurement providerTime(response.get());

    HANDLE_PROVIDER_CALL(
        deleteSubscription,
        provider->deleteSubscription(
            providerContext,
            request->subscriptionInstance.getPath(),
            classNames),
        handler);

    //
    //  Decrement count of current subscriptions for this provider
    //
    if (status.decrementSubscriptionsAndTestIfZero())
    {
        //
        //  If there are no current subscriptions after the decrement,
        //  the last subscription has been deleted
        //  Call the provider's disableIndications method
        //
        if (_subscriptionInitComplete)
        {
            _disableIndications();
        }
    }

    PEG_METHOD_EXIT();
    return response.release();
}

CIMResponseMessage* ProviderMessageHandler::_handleExportIndicationRequest(
    CIMRequestMessage* message)
{
    PEG_METHOD_ENTER(TRC_PROVIDERMANAGER,
        "ProviderMessageHandler::_handleExportIndicationRequest");

    CIMExportIndicationRequestMessage* request =
        dynamic_cast<CIMExportIndicationRequestMessage*>(message);
    PEGASUS_ASSERT(request != 0);

    AutoPtr<CIMExportIndicationResponseMessage> response(
        dynamic_cast<CIMExportIndicationResponseMessage*>(
            request->buildResponse()));
    PEGASUS_ASSERT(response.get() != 0);

    OperationResponseHandler handler(
        request, response.get(), _responseChunkCallback);

    // NOTE: Accept-Languages do not need to be set in the consume msg.
    OperationContext providerContext;
    providerContext.insert(request->operationContext.get(
        IdentityContainer::NAME));
//L10N_TODO
// ATTN-CEC 06/04/03 NOTE: I can't find where the consume msg is sent.  This
// does not appear to be hooked up.  When it is added, need to
// make sure that Content-Language is set in the consume msg.
    providerContext.insert(request->operationContext.get(
        ContentLanguageListContainer::NAME));

    CIMIndicationConsumerProvider* provider =
        getProviderInterface<CIMIndicationConsumerProvider>(_provider);

    StatProviderTimeMeasurement providerTime(response.get());

    HANDLE_PROVIDER_CALL(
        consumeIndication,
        provider->consumeIndication(
            providerContext,
            request->destinationPath,
            request->indicationInstance),
        handler);

    PEG_METHOD_EXIT();
    return response.release();
}

void ProviderMessageHandler::_enableIndications()
{
    PEG_METHOD_ENTER(TRC_PROVIDERMANAGER,
        "DefaultProviderManager::_enableIndications");

    try
    {
        EnableIndicationsResponseHandler* indicationResponseHandler =
            new EnableIndicationsResponseHandler(
                0,    // request
                0,    // response
                status.getProviderInstance(),
                _indicationCallback,
                _responseChunkCallback);

        _indicationResponseHandler = indicationResponseHandler;

        status.setIndicationsEnabled(true);

        CIMIndicationProvider* provider =
            getProviderInterface<CIMIndicationProvider>(_provider);

        PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL4,
            "Calling provider.enableIndications: %s",
            (const char*)_fullyQualifiedProviderName.getCString()));

        try
        {
            provider->enableIndications(*indicationResponseHandler);
        }
        catch (...)
        {
            PEG_TRACE((TRC_DISCARDED_DATA, Tracer::LEVEL1,
                "Caught exception from provider %s "
                    "enableIndications() method.",
                (const char*)_fullyQualifiedProviderName.getCString()));
            throw;
        }

        PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL4,
            "Returned from provider.enableIndications: %s",
            (const char*)_fullyQualifiedProviderName.getCString()));
    }
    catch (Exception& e)
    {
        Logger::put_l(Logger::ERROR_LOG, System::CIMSERVER, Logger::WARNING,
            MessageLoaderParms(
                "ProviderManager.Default.DefaultProviderManager."
                    "ENABLE_INDICATIONS_FAILED",
                "Failed to enable indications for provider $0: $1.",
                 _fullyQualifiedProviderName, e.getMessage()));
    }
    catch(...)
    {
        Logger::put_l(Logger::ERROR_LOG, System::CIMSERVER, Logger::WARNING,
            MessageLoaderParms(
                "ProviderManager.Default.DefaultProviderManager."
                    "ENABLE_INDICATIONS_FAILED_UNKNOWN",
                "Failed to enable indications for provider $0.",
                _fullyQualifiedProviderName));
    }

    PEG_METHOD_EXIT();
}

void ProviderMessageHandler::_disableIndications()
{
    PEG_METHOD_ENTER(TRC_PROVIDERMANAGER,
        "DefaultProviderManager::_disableIndications");

    try
    {
        if (status.getIndicationsEnabled())
        {
            CIMIndicationProvider* provider =
                getProviderInterface<CIMIndicationProvider>(_provider);

            PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL4,
                "Calling provider.disableIndications: %s",
                (const char*)_fullyQualifiedProviderName.getCString()));

            try
            {
                provider->disableIndications();
            }
            catch (...)
            {
                PEG_TRACE((TRC_DISCARDED_DATA, Tracer::LEVEL1,
                    "Caught exception from provider %s "
                        "disableIndications() method.",
                    (const char*)_fullyQualifiedProviderName.getCString()));
                throw;
            }

            PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL4,
                "Returned from  provider.disableIndications: %s",
                (const char*)_fullyQualifiedProviderName.getCString()));

            status.setIndicationsEnabled(false);

            status.resetSubscriptions();

            PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL4,
                "Destroying indication response handler for %s",
                (const char*)_fullyQualifiedProviderName.getCString()));

            delete _indicationResponseHandler;
            _indicationResponseHandler = 0;
        }
    }
    catch(...)
    {
        PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL3,
            "Error occured disabling indications in provider %s",
            (const char*)_fullyQualifiedProviderName.getCString()));
    }
    PEG_METHOD_EXIT();
}
PEGASUS_NAMESPACE_END
