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
// Author: Mike Brasher (mbrasher@bmc.com)
//
// Modified By:  Nag Boranna, Hewlett-Packard Company (nagaraja_boranna@hp.com)
//               Chip Vincent (cvincent@us.ibm.com)
//               Yi Zhou, Hewlett-Packard Company (yi_zhou@hp.com)
//               Nitin Upasani, Hewlett-Packard Company (Nitin_Upasani@hp.com)
//               Roger Kumpf, Hewlett-Packard Company (roger_kumpf@hp.com)
//               Mike Day (mdday@us.ibm.com)
//               Carol Ann Krug Graves, Hewlett-Packard Company
//                   (carolann_graves@hp.com)
//               Barbara Packard, Hewlett-Packard Company
//                   (barbara_packard@hp.com)
//               Jair Santos, Hewlett-Packard Company (jair.santos@hp.com)
//               Terry Martin, Hewlett-Packard Company (terry.martin@hp.com)
//               Amit K Arora, IBM (amita@in.ibm.com) for PEP#101
//                 Seema Gupta (gseema@in.ibm.com) for PEP135
//
//%////////////////////////////////////////////////////////////////////////////

#include "CIMOperationRequestDispatcher.h"
#include <Pegasus/Common/Constants.h>
#include <Pegasus/Common/XmlReader.h> // stringToValue(), stringArrayToValue()
#include <Pegasus/Common/Tracer.h>
#include <Pegasus/Config/ConfigManager.h>
#include <Pegasus/Common/StatisticalData.h>

///////////////////////////////////////////////////////////////////////////////
// Need to include these before the WMI Provider headers
#include <atlbase.h>
#include <comdef.h>
#include <wbemcli.h>

///////////////////////////////////////////////////////////////////////////////
// WMI Provider interface headers
#include <WMIMapper/WMIProvider/WMIInstanceProvider.h>
#include <WMIMapper/WMIProvider/WMIClassProvider.h>
#include <WMIMapper/WMIProvider/WMIAssociatorProvider.h>
#include <WMIMapper/WMIProvider/WMIReferenceProvider.h>
#include <WMIMapper/WMIProvider/WMIQualifierProvider.h>
#include <WMIMapper/WMIProvider/WMIMethodProvider.h>
#include <WMIMapper/WMIProvider/WMIQueryProvider.h>

PEGASUS_NAMESPACE_BEGIN

PEGASUS_USING_STD;

#ifndef TRC_WMI_MAPPER
#define TRC_WMI_MAPPER TRC_DISPATCHER
#endif

typedef struct threadParm
{
    Message* request;
    CIMOperationRequestDispatcher* dispatcher;
} ThreadParm_t;
CIMOperationRequestDispatcher::CIMOperationRequestDispatcher( )
//    CIMRepository* repository,
//    ProviderRegistrationManager* providerRegistrationManager)
      :
      Base(PEGASUS_QUEUENAME_OPREQDISPATCHER)
//      _repository(repository),
//      _providerRegistrationManager(providerRegistrationManager)
{
    PEG_METHOD_ENTER(TRC_WMI_MAPPER,
        "CIMOperationRequestDispatcher::CIMOperationRequestDispatcher");

    // Check whether or not AssociationTraversal is supported.
    //
    ConfigManager* configManager = ConfigManager::getInstance();
    _enableAssociationTraversal = ConfigManager::parseBooleanValue(
        configManager->getCurrentValue("enableAssociationTraversal"));
    //   _enableIndicationService = ConfigManager::parseBooleanValue(
    //        configManager->getCurrentValue("enableIndicationService"));

    PEG_METHOD_EXIT();
}

CIMOperationRequestDispatcher::~CIMOperationRequestDispatcher(void)
{
    PEG_METHOD_ENTER(TRC_WMI_MAPPER,
        "CIMOperationRequestDispatcher::~CIMOperationRequestDispatcher");
    _dying = 1;
    PEG_METHOD_EXIT();
}


void CIMOperationRequestDispatcher::_enqueueResponse(
   CIMRequestMessage* request,
   CIMResponseMessage* response)
{
    PEG_METHOD_ENTER(TRC_WMI_MAPPER,
        "CIMOperationRequestDispatcher::_enqueueResponse");

    // Use the same key as used in the request:

    //response->setKey(request->getKey());
    response->dest = request->queueIds.top();

    if( true == Base::_enqueueResponse(request, response))
    {
        PEG_METHOD_EXIT();
        return;
    }

    MessageQueue * queue = MessageQueue::lookup(request->queueIds.top());
    PEGASUS_ASSERT(queue != 0 );

    queue->enqueue(response);

    PEG_METHOD_EXIT();
}

ThreadReturnType PEGASUS_THREAD_CDECL
    CIMOperationRequestDispatcher::_callMethodHandler(void *parm)
{
    ThreadParm_t* myParms = reinterpret_cast<ThreadParm_t *>(parm);
    myParms->dispatcher->_handleOperationMessage(myParms->request);
    delete myParms;

    return 0;
}


void CIMOperationRequestDispatcher::_handleOperationMessage(Message *request)
{
    PEG_METHOD_ENTER(TRC_WMI_MAPPER,
        "CIMOperationRequestDispatcher::_handleOperationMessage("
        "Message *request)");

    if(!request)
    {
        PEG_METHOD_EXIT();
        return;
    }

    switch(request->getType())
    {

        case CIM_GET_CLASS_REQUEST_MESSAGE:
            handleGetClassRequest(
                (CIMGetClassRequestMessage*)request);
            break;

        case CIM_GET_INSTANCE_REQUEST_MESSAGE:
            handleGetInstanceRequest(
                (CIMGetInstanceRequestMessage*)request);
            break;

        case CIM_DELETE_CLASS_REQUEST_MESSAGE:
            handleDeleteClassRequest(
                (CIMDeleteClassRequestMessage*)request);
            break;

        case CIM_DELETE_INSTANCE_REQUEST_MESSAGE:
            handleDeleteInstanceRequest(
                (CIMDeleteInstanceRequestMessage*)request);
            break;

        case CIM_CREATE_CLASS_REQUEST_MESSAGE:
            handleCreateClassRequest(
                (CIMCreateClassRequestMessage*)request);
            break;

        case CIM_CREATE_INSTANCE_REQUEST_MESSAGE:
            handleCreateInstanceRequest(
                (CIMCreateInstanceRequestMessage*)request);
            break;

        case CIM_MODIFY_CLASS_REQUEST_MESSAGE:
            handleModifyClassRequest(
                (CIMModifyClassRequestMessage*)request);
            break;

        case CIM_MODIFY_INSTANCE_REQUEST_MESSAGE:
            handleModifyInstanceRequest(
                (CIMModifyInstanceRequestMessage*)request);
            break;

        case CIM_ENUMERATE_CLASSES_REQUEST_MESSAGE:
            handleEnumerateClassesRequest(
                (CIMEnumerateClassesRequestMessage*)request);
            break;

        case CIM_ENUMERATE_CLASS_NAMES_REQUEST_MESSAGE:
            handleEnumerateClassNamesRequest(
                (CIMEnumerateClassNamesRequestMessage*)request);
            break;

        case CIM_ENUMERATE_INSTANCES_REQUEST_MESSAGE:
            handleEnumerateInstancesRequest(
                (CIMEnumerateInstancesRequestMessage*)request);
            break;

        case CIM_ENUMERATE_INSTANCE_NAMES_REQUEST_MESSAGE:
            handleEnumerateInstanceNamesRequest(
                (CIMEnumerateInstanceNamesRequestMessage*)request);
            break;

        case CIM_EXEC_QUERY_REQUEST_MESSAGE:
            handleExecQueryRequest(
                (CIMExecQueryRequestMessage*)request);
            break;

        case CIM_ASSOCIATORS_REQUEST_MESSAGE:
            handleAssociatorsRequest(
                (CIMAssociatorsRequestMessage*)request);
            break;

        case CIM_ASSOCIATOR_NAMES_REQUEST_MESSAGE:
            handleAssociatorNamesRequest(
                (CIMAssociatorNamesRequestMessage*)request);
            break;

        case CIM_REFERENCES_REQUEST_MESSAGE:
            handleReferencesRequest(
                (CIMReferencesRequestMessage*)request);
            break;

        case CIM_REFERENCE_NAMES_REQUEST_MESSAGE:
            handleReferenceNamesRequest(
                (CIMReferenceNamesRequestMessage*)request);
            break;

        case CIM_GET_PROPERTY_REQUEST_MESSAGE:
            handleGetPropertyRequest(
                (CIMGetPropertyRequestMessage*)request);
            break;

        case CIM_SET_PROPERTY_REQUEST_MESSAGE:
            handleSetPropertyRequest(
                (CIMSetPropertyRequestMessage*)request);
            break;

        case CIM_GET_QUALIFIER_REQUEST_MESSAGE:
            handleGetQualifierRequest(
                (CIMGetQualifierRequestMessage*)request);
            break;

        case CIM_SET_QUALIFIER_REQUEST_MESSAGE:
            handleSetQualifierRequest(
                (CIMSetQualifierRequestMessage*)request);
            break;

        case CIM_DELETE_QUALIFIER_REQUEST_MESSAGE:
            handleDeleteQualifierRequest(
                (CIMDeleteQualifierRequestMessage*)request);
            break;

        case CIM_ENUMERATE_QUALIFIERS_REQUEST_MESSAGE:
            handleEnumerateQualifiersRequest(
                (CIMEnumerateQualifiersRequestMessage*)request);
            break;

        case CIM_INVOKE_METHOD_REQUEST_MESSAGE:
            handleInvokeMethodRequest(
                (CIMInvokeMethodRequestMessage*)request);
            break;

        default:
            throw CIMException(CIM_ERR_NOT_SUPPORTED);
    }

    delete request;
    PEG_METHOD_EXIT();
}



// allocate a CIM Operation_async,  opnode, context, and response handler
// initialize with pointers to async top and async bottom
// link to the waiting q
void CIMOperationRequestDispatcher::handleEnqueue()
{
    PEG_METHOD_ENTER(TRC_WMI_MAPPER,
        "CIMOperationRequestDispatcher::_handleOperationMessage");

    Message* request = dequeue();

    if(request)
        handleEnqueue(request);

    PEG_METHOD_EXIT();
}

void CIMOperationRequestDispatcher::handleEnqueue(Message *request)
{
    PEG_METHOD_ENTER(TRC_WMI_MAPPER,
        "CIMOperationRequestDispatcher::handleEnqueue(Message *request)");

    if(!request)
    {
        PEG_METHOD_EXIT();
        return;
    }

    ThreadParm_t* myParm = new ThreadParm_t;

    myParm->dispatcher = this;
    myParm->request = request;

    // allocate and run one thread for each request
    MessageQueueService::get_thread_pool()->allocate_and_awaken(
        myParm,
        _callMethodHandler);

    PEG_METHOD_EXIT();
}

void CIMOperationRequestDispatcher::handleGetClassRequest(
   CIMGetClassRequestMessage* request)
{
    PEG_METHOD_ENTER(TRC_WMI_MAPPER,
        "CIMOperationRequestDispatcher::handleGetClassRequest");

    AutoPtr<CIMGetClassResponseMessage> response(
        dynamic_cast<CIMGetClassResponseMessage *>(
            request->buildResponse()));

    WMIClassProvider provider;

    try
    {
        WMIMapperUserInfoContainer container =
            request->operationContext.get("WMIMapperUserInfoContainer");

        StatProviderTimeMeasurement providerTime(response.get());

        provider.initialize(
            String::equalNoCase(request->authType, LOCAL_CALL_AUTH_TYPE));

        response->cimClass = provider.getClass(
            request->nameSpace.getString(),
            ((IdentityContainer) request->operationContext.get
                (IdentityContainer::NAME)).getUserName(),
            container.getPassword(),
            request->className.getString(),
            request->localOnly,
            request->includeQualifiers,
            request->includeClassOrigin,
            request->propertyList);
    }
    catch(CIMException& exception)
    {
        response->cimException = exception;
    }
    catch(Exception& exception)
    {
        response->cimException =
            PEGASUS_CIM_EXCEPTION(CIM_ERR_FAILED, exception.getMessage());
    }
    catch(...)
    {
        response->cimException = PEGASUS_CIM_EXCEPTION(CIM_ERR_FAILED,
            "GetClass() failed!");
    }

    //terminate the provider
    provider.terminate();

    // and send the response
    _enqueueResponse(request, response.release());
    PEG_METHOD_EXIT();
}

void CIMOperationRequestDispatcher::handleGetInstanceRequest(
   CIMGetInstanceRequestMessage* request)
{
    PEG_METHOD_ENTER(TRC_WMI_MAPPER,
        "CIMOperationRequestDispatcher::handleGetInstanceRequest");

    AutoPtr<CIMGetInstanceResponseMessage> response(
        dynamic_cast<CIMGetInstanceResponseMessage *>(
            request->buildResponse()));

    WMIInstanceProvider provider;

    // get the class name
    String className = request->instanceName.getClassName().getString();

    try
    {
        WMIMapperUserInfoContainer container =
            request->operationContext.get("WMIMapperUserInfoContainer");

        StatProviderTimeMeasurement providerTime(response.get());

        provider.initialize(
            String::equalNoCase(request->authType, LOCAL_CALL_AUTH_TYPE));

        CIMInstance cimInstance = provider.getInstance(
            request->nameSpace.getString(),
            ((IdentityContainer) request->operationContext.get
                (IdentityContainer::NAME)).getUserName(),
            container.getPassword(),
            request->instanceName,
            request->localOnly,
            request->includeQualifiers,
            request->includeClassOrigin,
            request->propertyList);

        AutoPtr<CIMGetInstanceResponseMessage> response(
            dynamic_cast<CIMGetInstanceResponseMessage*>(
                request->buildResponse()));
        response->setCimInstance(cimInstance);


    }
    catch(CIMException& exception)
    {
        response->cimException = exception;
    }
    catch(Exception& exception)
    {
        response->cimException =
            PEGASUS_CIM_EXCEPTION(CIM_ERR_FAILED, exception.getMessage());
    }
    catch(...)
    {
        response->cimException = PEGASUS_CIM_EXCEPTION(CIM_ERR_FAILED,
            "GetInstance() failed!");
    }

    // cancel the provider
    provider.terminate();

    // and send the response
    _enqueueResponse(request, response.release());
    PEG_METHOD_EXIT();
}

void CIMOperationRequestDispatcher::handleDeleteClassRequest(
   CIMDeleteClassRequestMessage* request)
{
    PEG_METHOD_ENTER(TRC_WMI_MAPPER,
        "CIMOperationRequestDispatcher::handleDeleteClassRequest");

    AutoPtr<CIMDeleteClassResponseMessage> response(
        dynamic_cast<CIMDeleteClassResponseMessage *>(
            request->buildResponse()));

    WMIClassProvider provider;

    try
    {
        WMIMapperUserInfoContainer container =
            request->operationContext.get("WMIMapperUserInfoContainer");

        StatProviderTimeMeasurement providerTime(response.get());

        provider.initialize(
            String::equalNoCase(request->authType, LOCAL_CALL_AUTH_TYPE));

        provider.deleteClass(
            request->nameSpace.getString(),
            ((IdentityContainer) request->operationContext.get
                (IdentityContainer::NAME)).getUserName(),
            container.getPassword(),
            request->className.getString());
    }
    catch(CIMException& exception)
    {
        response->cimException = exception;
    }
    catch(Exception& exception)
    {
        response->cimException =
            PEGASUS_CIM_EXCEPTION(CIM_ERR_FAILED, exception.getMessage());
    }
    catch(...)
    {
        response->cimException = PEGASUS_CIM_EXCEPTION(CIM_ERR_FAILED,
            "DeleteClass() failed!");
    }

    //terminate the provider
    provider.terminate();

    // and send the response
    _enqueueResponse(request, response.release());

    PEG_METHOD_EXIT();
}

void CIMOperationRequestDispatcher::handleDeleteInstanceRequest(
   CIMDeleteInstanceRequestMessage* request)
{
    PEG_METHOD_ENTER(TRC_WMI_MAPPER,
        "CIMOperationRequestDispatcher::handleDeleteInstanceRequest");

    AutoPtr<CIMDeleteInstanceResponseMessage> response(
        dynamic_cast<CIMDeleteInstanceResponseMessage *>(
            request->buildResponse()));

    WMIInstanceProvider provider;

    try
    {
        WMIMapperUserInfoContainer container =
            request->operationContext.get("WMIMapperUserInfoContainer");

        StatProviderTimeMeasurement providerTime(response.get());

        provider.initialize(
            String::equalNoCase(request->authType, LOCAL_CALL_AUTH_TYPE));

        provider.deleteInstance(
            request->nameSpace.getString(),
            ((IdentityContainer) request->operationContext.get
                (IdentityContainer::NAME)).getUserName(),
            container.getPassword(),
            request->instanceName);
    }
    catch(CIMException& exception)
    {
        response->cimException = exception;
    }
    catch(Exception& exception)
    {
        response->cimException =
            PEGASUS_CIM_EXCEPTION(CIM_ERR_FAILED, exception.getMessage());
    }
    catch(...)
    {
        response->cimException = PEGASUS_CIM_EXCEPTION(CIM_ERR_FAILED,
            "DeleteInstance() failed!");
    }

    //terminate the provider
    provider.terminate();

    // and send it
    _enqueueResponse(request, response.release());

    PEG_METHOD_EXIT();
}

void CIMOperationRequestDispatcher::handleCreateClassRequest(
   CIMCreateClassRequestMessage* request)
{
    PEG_METHOD_ENTER(TRC_WMI_MAPPER,
        "CIMOperationRequestDispatcher::handleCreateClassRequest");

    AutoPtr<CIMCreateClassResponseMessage> response(
        dynamic_cast<CIMCreateClassResponseMessage *>(
            request->buildResponse()));

    WMIClassProvider provider;

    try
    {
        WMIMapperUserInfoContainer container =
            request->operationContext.get("WMIMapperUserInfoContainer");

        StatProviderTimeMeasurement providerTime(response.get());

        provider.initialize(
            String::equalNoCase(request->authType, LOCAL_CALL_AUTH_TYPE));

        provider.createClass(
            request->nameSpace.getString(),
            ((IdentityContainer) request->operationContext.get
                (IdentityContainer::NAME)).getUserName(),
            container.getPassword(),
            request->newClass);
    }
    catch(CIMException& exception)
    {
        response->cimException = exception;
    }
    catch(Exception& exception)
    {
        response->cimException =
            PEGASUS_CIM_EXCEPTION(CIM_ERR_FAILED, exception.getMessage());
    }
    catch(...)
    {
        response->cimException = PEGASUS_CIM_EXCEPTION(CIM_ERR_FAILED,
            "CreateClass() failed!");
    }

    //terminate the provider
    provider.terminate();

    // and send the response
    _enqueueResponse(request, response.release());

    PEG_METHOD_EXIT();
}

void CIMOperationRequestDispatcher::handleCreateInstanceRequest(
   CIMCreateInstanceRequestMessage* request)
{
    PEG_METHOD_ENTER(TRC_WMI_MAPPER,
        "CIMOperationRequestDispatcher::handleCreateInstanceRequest");

    AutoPtr<CIMCreateInstanceResponseMessage> response(
        dynamic_cast<CIMCreateInstanceResponseMessage *>(
            request->buildResponse()));

    WMIInstanceProvider provider;

    try
    {
        WMIMapperUserInfoContainer container =
            request->operationContext.get("WMIMapperUserInfoContainer");

        StatProviderTimeMeasurement providerTime(response.get());

        provider.initialize(
            String::equalNoCase(request->authType, LOCAL_CALL_AUTH_TYPE));

        response->instanceName = provider.createInstance(
            request->nameSpace.getString(),
            ((IdentityContainer) request->operationContext.get
                (IdentityContainer::NAME)).getUserName(),
            container.getPassword(),
            request->newInstance);
    }
    catch(CIMException& exception)
    {
        response->cimException = exception;
    }
    catch(Exception& exception)
    {
        response->cimException =
            PEGASUS_CIM_EXCEPTION(CIM_ERR_FAILED, exception.getMessage());
    }
    catch(...)
    {
        response->cimException = PEGASUS_CIM_EXCEPTION(CIM_ERR_FAILED,
            "CreateInstance() failed!");
    }

    //terminate the provider
    provider.terminate();

    // and send it
    _enqueueResponse(request, response.release());

    PEG_METHOD_EXIT();
}

void CIMOperationRequestDispatcher::handleModifyClassRequest(
   CIMModifyClassRequestMessage* request)
{
    PEG_METHOD_ENTER(TRC_WMI_MAPPER,
        "CIMOperationRequestDispatcher::handleModifyClassRequest");

    AutoPtr<CIMModifyClassResponseMessage> response(
        dynamic_cast<CIMModifyClassResponseMessage *>(
            request->buildResponse()));

    WMIClassProvider provider;

    try
    {
        WMIMapperUserInfoContainer container =
            request->operationContext.get("WMIMapperUserInfoContainer");

        StatProviderTimeMeasurement providerTime(response.get());

        provider.initialize(
            String::equalNoCase(request->authType, LOCAL_CALL_AUTH_TYPE));

        provider.modifyClass(
            request->nameSpace.getString(),
            ((IdentityContainer) request->operationContext.get
                (IdentityContainer::NAME)).getUserName(),
            container.getPassword(),
            request->modifiedClass);
    }
    catch(CIMException& exception)
    {
        response->cimException = exception;
    }
    catch(Exception& exception)
    {
        response->cimException =
            PEGASUS_CIM_EXCEPTION(CIM_ERR_FAILED, exception.getMessage());
    }
    catch(...)
    {
        response->cimException = PEGASUS_CIM_EXCEPTION(CIM_ERR_FAILED,
            "modifyClass() failed!");
    }

    //terminate the provider
    provider.terminate();

    // and send the response
    _enqueueResponse(request, response.release());

    PEG_METHOD_EXIT();
}

void CIMOperationRequestDispatcher::handleModifyInstanceRequest(
   CIMModifyInstanceRequestMessage* request)
{
    PEG_METHOD_ENTER(TRC_WMI_MAPPER,
        "CIMOperationRequestDispatcher::handleModifyInstanceRequest");

    AutoPtr<CIMModifyInstanceResponseMessage> response(
        dynamic_cast<CIMModifyInstanceResponseMessage *>(
            request->buildResponse()));

    WMIInstanceProvider provider;

    try
    {
        WMIMapperUserInfoContainer container =
            request->operationContext.get("WMIMapperUserInfoContainer");

        StatProviderTimeMeasurement providerTime(response.get());

        provider.initialize(
            String::equalNoCase(request->authType, LOCAL_CALL_AUTH_TYPE));

        provider.modifyInstance(
           request->nameSpace.getString(),
          ((IdentityContainer) request->operationContext.get
                (IdentityContainer::NAME)).getUserName(),
           container.getPassword(),
           request->modifiedInstance,
           request->includeQualifiers,
           request->propertyList);
    }
    catch(CIMException& exception)
    {
        response->cimException = exception;
    }
    catch(Exception& exception)
    {
        response->cimException =
            PEGASUS_CIM_EXCEPTION(CIM_ERR_FAILED, exception.getMessage());
    }
    catch(...)
    {
        response->cimException = PEGASUS_CIM_EXCEPTION(CIM_ERR_FAILED,
            "modifyInstance() failed!");
    }

    //terminate the provider
    provider.terminate();

    // and send it
    _enqueueResponse(request, response.release());

    PEG_METHOD_EXIT();
}

void CIMOperationRequestDispatcher::handleEnumerateClassesRequest(
   CIMEnumerateClassesRequestMessage* request)
{
    PEG_METHOD_ENTER(TRC_WMI_MAPPER,
        "CIMOperationRequestDispatcher::handleEnumerateClassesRequest");

    AutoPtr<CIMEnumerateClassesResponseMessage> response(
        dynamic_cast<CIMEnumerateClassesResponseMessage *>(
            request->buildResponse()));

    WMIClassProvider provider;

    try
    {
        WMIMapperUserInfoContainer container =
            request->operationContext.get("WMIMapperUserInfoContainer");

        StatProviderTimeMeasurement providerTime(response.get());

        provider.initialize(
            String::equalNoCase(request->authType, LOCAL_CALL_AUTH_TYPE));

        response->cimClasses = provider.enumerateClasses(
            request->nameSpace.getString(),
            ((IdentityContainer) request->operationContext.get
                (IdentityContainer::NAME)).getUserName(),
            container.getPassword(),
            request->className.getString(),
            request->deepInheritance,
            request->localOnly,
            request->includeQualifiers,
            request->includeClassOrigin);
    }
    catch(CIMException& exception)
    {
        response->cimException = exception;
    }
    catch(Exception& exception)
    {
        response->cimException =
            PEGASUS_CIM_EXCEPTION(CIM_ERR_FAILED, exception.getMessage());
    }
    catch(...)
    {
        response->cimException = PEGASUS_CIM_EXCEPTION(CIM_ERR_FAILED,
            "enumerateClasses() failed!");
    }

    //terminate the provider
    provider.terminate();

    // and send the response
    _enqueueResponse(request, response.release());

    PEG_METHOD_EXIT();
}

void CIMOperationRequestDispatcher::handleEnumerateClassNamesRequest(
   CIMEnumerateClassNamesRequestMessage* request)
{
    PEG_METHOD_ENTER(TRC_WMI_MAPPER,
        "CIMOperationRequestDispatcher::handleEnumerateClassNamesRequest");

    AutoPtr<CIMEnumerateClassNamesResponseMessage> response(
        dynamic_cast<CIMEnumerateClassNamesResponseMessage *>(
            request->buildResponse()));

    WMIClassProvider provider;

    try
    {
        WMIMapperUserInfoContainer container =
            request->operationContext.get("WMIMapperUserInfoContainer");

        StatProviderTimeMeasurement providerTime(response.get());

        provider.initialize(
            String::equalNoCase(request->authType, LOCAL_CALL_AUTH_TYPE));

        response->classNames = provider.enumerateClassNames(
            request->nameSpace.getString(),
            ((IdentityContainer) request->operationContext.get
                (IdentityContainer::NAME)).getUserName(),
            container.getPassword(),
            request->className.getString(),
            request->deepInheritance);
    }
    catch(CIMException& exception)
    {
        response->cimException = exception;
    }
    catch(Exception& exception)
    {
        response->cimException =
            PEGASUS_CIM_EXCEPTION(CIM_ERR_FAILED, exception.getMessage());
    }
    catch(...)
    {
        response->cimException = PEGASUS_CIM_EXCEPTION(CIM_ERR_FAILED,
            "enumerateClassNames() failed!");
    }

    //terminate the provider
    provider.terminate();

    // and send the response
    _enqueueResponse(request, response.release());

    PEG_METHOD_EXIT();
}

void CIMOperationRequestDispatcher::handleEnumerateInstancesRequest(
   CIMEnumerateInstancesRequestMessage* request)
{
    PEG_METHOD_ENTER(TRC_WMI_MAPPER,
        "CIMOperationRequestDispatcher::handleEnumerateInstanceRequest");

    AutoPtr<CIMEnumerateInstancesResponseMessage> response(
        dynamic_cast<CIMEnumerateInstancesResponseMessage *>(
            request->buildResponse()));

    WMIInstanceProvider provider;
    Array<CIMInstance> cimNamedInstances;

    try
    {
        WMIMapperUserInfoContainer container =
            request->operationContext.get("WMIMapperUserInfoContainer");
        CIMPropertyList propertyList(request->propertyList);

        StatProviderTimeMeasurement providerTime(response.get());

        provider.initialize(
            String::equalNoCase(request->authType, LOCAL_CALL_AUTH_TYPE));

        cimNamedInstances = provider.enumerateInstances(
            request->nameSpace.getString(),
            ((IdentityContainer) request->operationContext.get
                (IdentityContainer::NAME)).getUserName(),
            container.getPassword(),
            request->className.getString(),
            request->deepInheritance, request->localOnly, false, false,
            propertyList);
    }
    catch(CIMException& exception)
    {
        response->cimException = exception;
    }
    catch(Exception& exception)
    {
        response->cimException =
            PEGASUS_CIM_EXCEPTION(CIM_ERR_FAILED, exception.getMessage());
    }
    catch(...)
    {
        response->cimException = PEGASUS_CIM_EXCEPTION(CIM_ERR_FAILED,
            "enumerateInstances() failed!");
    }

    response->setNamedInstances(cimNamedInstances);

    // cancel the provider
    provider.terminate();

    // and send the response
    _enqueueResponse(request, response.release());
    PEG_METHOD_EXIT();
}

void CIMOperationRequestDispatcher::handleEnumerateInstanceNamesRequest(
   CIMEnumerateInstanceNamesRequestMessage* request)
{
    PEG_METHOD_ENTER(TRC_WMI_MAPPER,
        "CIMOperationRequestDispatcher::handleEnumerateInstanceNamesRequest");

    AutoPtr<CIMEnumerateInstanceNamesResponseMessage> response(
        dynamic_cast<CIMEnumerateInstanceNamesResponseMessage *>(
            request->buildResponse()));

    WMIInstanceProvider provider;

    try
    {
        WMIMapperUserInfoContainer container =
            request->operationContext.get("WMIMapperUserInfoContainer");

        StatProviderTimeMeasurement providerTime(response.get());

        // make the request...
        provider.initialize(
            String::equalNoCase(request->authType, LOCAL_CALL_AUTH_TYPE));

        response->instanceNames = provider.enumerateInstanceNames(
            request->nameSpace.getString(),
            ((IdentityContainer) request->operationContext.get
                (IdentityContainer::NAME)).getUserName(),
            container.getPassword(),
            request->className.getString());
    }
    catch(CIMException& exception)
    {
        response->cimException = exception;
    }
    catch(Exception& exception)
    {
        response->cimException =
            PEGASUS_CIM_EXCEPTION(CIM_ERR_FAILED, exception.getMessage());
    }
    catch(...)
    {
        response->cimException = PEGASUS_CIM_EXCEPTION(CIM_ERR_FAILED,
            "enumerateInstanceNames() failed!");
    }

    // cancel the provider
    provider.terminate();

    // and send the response
    _enqueueResponse(request, response.release());
    PEG_METHOD_EXIT();
}

void CIMOperationRequestDispatcher::handleAssociatorsRequest(
   CIMAssociatorsRequestMessage* request)
{
    PEG_METHOD_ENTER(TRC_WMI_MAPPER,
        "CIMOperationRequestDispatcher::handleAssociatorsRequest");

    AutoPtr<CIMAssociatorsResponseMessage> response(
        dynamic_cast<CIMAssociatorsResponseMessage *>(
            request->buildResponse()));

    WMIAssociatorProvider provider;

    try
    {
        WMIMapperUserInfoContainer container =
            request->operationContext.get("WMIMapperUserInfoContainer");
        CIMPropertyList propertyList(request->propertyList);

        StatProviderTimeMeasurement providerTime(response.get());

        provider.initialize(
            String::equalNoCase(request->authType, LOCAL_CALL_AUTH_TYPE));

        // ATTN: fix parameter list

        response->cimObjects = provider.associators(
            request->nameSpace.getString(),
            ((IdentityContainer) request->operationContext.get
                (IdentityContainer::NAME)).getUserName(),
            container.getPassword(),
            request->objectName,
            request->assocClass.getString(),
            request->resultClass.getString(),
            request->role,
            request->resultRole,
            request->includeQualifiers,
            request->includeClassOrigin,
            propertyList);
    }
    catch(CIMException& exception)
    {
        response->cimException = exception;
    }
    catch(Exception& exception)
    {
        response->cimException =
            PEGASUS_CIM_EXCEPTION(CIM_ERR_FAILED, exception.getMessage());
    }
    catch(...)
    {
        response->cimException = PEGASUS_CIM_EXCEPTION(CIM_ERR_FAILED,
            "associators() failed!");
    }

    // cancel the provider
    provider.terminate();

    // and send it
    _enqueueResponse(request, response.release());

    PEG_METHOD_EXIT();
}

void CIMOperationRequestDispatcher::handleAssociatorNamesRequest(
   CIMAssociatorNamesRequestMessage* request)
{
    PEG_METHOD_ENTER(TRC_WMI_MAPPER,
        "CIMOperationRequestDispatcher::handleAssociatorNamesRequest");

    AutoPtr<CIMAssociatorNamesResponseMessage> response(
        dynamic_cast<CIMAssociatorNamesResponseMessage *>(
            request->buildResponse()));

    WMIAssociatorProvider provider;

    try
    {
        WMIMapperUserInfoContainer container =
            request->operationContext.get("WMIMapperUserInfoContainer");

        StatProviderTimeMeasurement providerTime(response.get());

        provider.initialize(
            String::equalNoCase(request->authType, LOCAL_CALL_AUTH_TYPE));

        response->objectNames = provider.associatorNames(
            request->nameSpace.getString(),
            ((IdentityContainer) request->operationContext.get
                (IdentityContainer::NAME)).getUserName(),
            container.getPassword(),
            request->objectName,
            request->assocClass.getString(),
            request->resultClass.getString(),
            request->role,
            request->resultRole);
    }
    catch(CIMException& exception)
    {
        response->cimException = exception;
    }
    catch(Exception& exception)
    {
        response->cimException =
            PEGASUS_CIM_EXCEPTION(CIM_ERR_FAILED, exception.getMessage());
    }
    catch(...)
    {
        response->cimException = PEGASUS_CIM_EXCEPTION(CIM_ERR_FAILED,
            "associatorNames() failed!");
    }

    // cancel the provider
    provider.terminate();

    // and send it
    _enqueueResponse(request, response.release());

    PEG_METHOD_EXIT();
}

void CIMOperationRequestDispatcher::handleReferencesRequest(
   CIMReferencesRequestMessage* request)
{
    PEG_METHOD_ENTER(TRC_WMI_MAPPER,
        "CIMOperationRequestDispatcher::handleReferencesRequest");

    AutoPtr<CIMReferencesResponseMessage> response(
        dynamic_cast<CIMReferencesResponseMessage *>(
            request->buildResponse()));

    WMIReferenceProvider provider;

    try
    {
        WMIMapperUserInfoContainer container =
            request->operationContext.get("WMIMapperUserInfoContainer");
        CIMPropertyList propertyList(request->propertyList);

        StatProviderTimeMeasurement providerTime(response.get());

        provider.initialize(
            String::equalNoCase(request->authType, LOCAL_CALL_AUTH_TYPE));

        // ATTN: fix parameter list

        response->cimObjects = provider.references(
            request->nameSpace.getString(),
            ((IdentityContainer) request->operationContext.get
                (IdentityContainer::NAME)).getUserName(),
            container.getPassword(),
            request->objectName,
            request->resultClass.getString(),
            request->role,
            request->includeQualifiers,
            request->includeClassOrigin,
            propertyList);
    }
    catch(CIMException& exception)
    {
        response->cimException = exception;
    }
    catch(Exception& exception)
    {
        response->cimException =
            PEGASUS_CIM_EXCEPTION(CIM_ERR_FAILED, exception.getMessage());
    }
    catch(...)
    {
        response->cimException = PEGASUS_CIM_EXCEPTION(CIM_ERR_FAILED,
            "references() failed!");
    }

    // cancel the provider
    provider.terminate();

    // and send it
    _enqueueResponse(request, response.release());

    PEG_METHOD_EXIT();
}

void CIMOperationRequestDispatcher::handleReferenceNamesRequest(
   CIMReferenceNamesRequestMessage* request)
{
    PEG_METHOD_ENTER(TRC_WMI_MAPPER,
        "CIMOperationRequestDispatcher::handleReferenceNamesRequest");

    AutoPtr<CIMReferenceNamesResponseMessage> response(
        dynamic_cast<CIMReferenceNamesResponseMessage *>(
            request->buildResponse()));

    WMIReferenceProvider provider;

    try
    {
        WMIMapperUserInfoContainer container =
            request->operationContext.get("WMIMapperUserInfoContainer");

        StatProviderTimeMeasurement providerTime(response.get());

        provider.initialize(
            String::equalNoCase(request->authType, LOCAL_CALL_AUTH_TYPE));

        response->objectNames = provider.referenceNames(
            request->nameSpace.getString(),
            ((IdentityContainer) request->operationContext.get
                (IdentityContainer::NAME)).getUserName(),
            container.getPassword(),
            request->objectName,
            request->resultClass.getString(),
            request->role);
    }
    catch(CIMException& exception)
    {
        response->cimException = exception;
    }
    catch(Exception& exception)
    {
        response->cimException =
            PEGASUS_CIM_EXCEPTION(CIM_ERR_FAILED, exception.getMessage());
    }
    catch(...)
    {
        response->cimException = PEGASUS_CIM_EXCEPTION(CIM_ERR_FAILED,
            "referenceNames() failed!");
    }

    // cancel the provider
    provider.terminate();

    // and send it
    _enqueueResponse(request, response.release());

    PEG_METHOD_EXIT();
}

void CIMOperationRequestDispatcher::handleGetPropertyRequest(
   CIMGetPropertyRequestMessage* request)
{
    PEG_METHOD_ENTER(TRC_WMI_MAPPER,
        "CIMOperationRequestDispatcher::handleGetPropertyRequest");

    AutoPtr<CIMGetPropertyResponseMessage> response(
        dynamic_cast<CIMGetPropertyResponseMessage *>(
            request->buildResponse()));

    WMIInstanceProvider provider;

    try
    {
        WMIMapperUserInfoContainer container =
            request->operationContext.get("WMIMapperUserInfoContainer");

        StatProviderTimeMeasurement providerTime(response.get());

        provider.initialize(
            String::equalNoCase(request->authType, LOCAL_CALL_AUTH_TYPE));

        response->value = provider.getProperty(
            request->nameSpace.getString(),
            ((IdentityContainer) request->operationContext.get
                (IdentityContainer::NAME)).getUserName(),
            container.getPassword(),
            request->instanceName,
            request->propertyName.getString());
    }
    catch(CIMException& exception)
    {
        response->cimException = exception;
    }
    catch(Exception& exception)
    {
        response->cimException =
            PEGASUS_CIM_EXCEPTION(CIM_ERR_FAILED, exception.getMessage());
    }
    catch(...)
    {
        response->cimException = PEGASUS_CIM_EXCEPTION(CIM_ERR_FAILED,
            "getProperty() failed!");
    }

    provider.terminate();

    // and send it
    _enqueueResponse(request, response.release());

    PEG_METHOD_EXIT();
}

void CIMOperationRequestDispatcher::handleSetPropertyRequest(
   CIMSetPropertyRequestMessage* request)
{
    PEG_METHOD_ENTER(TRC_WMI_MAPPER,
        "CIMOperationRequestDispatcher::handleSetPropertyRequest");

    AutoPtr<CIMSetPropertyResponseMessage> response(
        dynamic_cast<CIMSetPropertyResponseMessage *>(
            request->buildResponse()));

    // corrects the type of an property. Set property xml do not have an
    // place to specify the input property type. It could cause an
    // type mismatch in the Object types. To solve it, the method,
    // _fixSetPropertyValueType gets the correct type from Server side.

    WMIInstanceProvider provider;

    {
        CIMException cimException;
        try
        {
            _fixSetPropertyValueType(request);
        }
        catch (CIMException& exception)
        {
            response->cimException = exception;
        }
        catch(Exception& exception)
        {
            response->cimException =
                PEGASUS_CIM_EXCEPTION(CIM_ERR_FAILED, exception.getMessage());
        }
        catch(...)
        {
            response->cimException = PEGASUS_CIM_EXCEPTION(CIM_ERR_FAILED,
                String::EMPTY);
        }

        if (cimException.getCode() != CIM_ERR_SUCCESS)
        {
            _enqueueResponse(request, response.release());

            PEG_METHOD_EXIT();
            return;
        }
    }


    try
    {
        WMIMapperUserInfoContainer container =
            request->operationContext.get("WMIMapperUserInfoContainer");

        StatProviderTimeMeasurement providerTime(response.get());

        provider.initialize(
            String::equalNoCase(request->authType, LOCAL_CALL_AUTH_TYPE));

        provider.setProperty(
            request->nameSpace.getString(),
            ((IdentityContainer) request->operationContext.get
                (IdentityContainer::NAME)).getUserName(),
            container.getPassword(),
            request->instanceName,
            request->propertyName.getString(),
            request->newValue);
    }
    catch(CIMException& exception)
    {
        response->cimException = exception;
    }
    catch(Exception& exception)
    {
        response->cimException =
            PEGASUS_CIM_EXCEPTION(CIM_ERR_FAILED, exception.getMessage());
    }
    catch(...)
    {
        response->cimException = PEGASUS_CIM_EXCEPTION(CIM_ERR_FAILED,
            "setProperty() failed!");
    }

    provider.terminate();

    // and send it
    _enqueueResponse(request, response.release());

    PEG_METHOD_EXIT();
}

void CIMOperationRequestDispatcher::handleGetQualifierRequest(
   CIMGetQualifierRequestMessage* request)
{
    PEG_METHOD_ENTER(TRC_WMI_MAPPER,
        "CIMOperationRequestDispatcher::handleGetQualifierRequest");

    AutoPtr<CIMGetQualifierResponseMessage> response(
        dynamic_cast<CIMGetQualifierResponseMessage *>(
            request->buildResponse()));

    WMIQualifierProvider provider;

    try
    {
        WMIMapperUserInfoContainer container =
            request->operationContext.get("WMIMapperUserInfoContainer");

        StatProviderTimeMeasurement providerTime(response.get());

        provider.initialize(
            String::equalNoCase(request->authType, LOCAL_CALL_AUTH_TYPE));

        response->cimQualifierDecl = provider.getQualifier(
            request->nameSpace.getString(),
            ((IdentityContainer) request->operationContext.get
                (IdentityContainer::NAME)).getUserName(),
            container.getPassword(),
            request->qualifierName.getString());
    }
    catch(CIMException& exception)
    {
        response->cimException = exception;
    }
    catch(Exception& exception)
    {
        response->cimException =
            PEGASUS_CIM_EXCEPTION(CIM_ERR_FAILED, exception.getMessage());
    }
    catch(...)
    {
        response->cimException = PEGASUS_CIM_EXCEPTION(CIM_ERR_FAILED,
            "getQualifier() failed!");
    }

    provider.terminate();

    // and send it
    _enqueueResponse(request, response.release());

    PEG_METHOD_EXIT();
}

void CIMOperationRequestDispatcher::handleSetQualifierRequest(
   CIMSetQualifierRequestMessage* request)
{
    PEG_METHOD_ENTER(TRC_WMI_MAPPER,
        "CIMOperationRequestDispatcher::handleSetQualifierRequest");

    AutoPtr<CIMSetQualifierResponseMessage> response(
        dynamic_cast<CIMSetQualifierResponseMessage *>(
            request->buildResponse()));

    WMIQualifierProvider provider;

    try
    {
        WMIMapperUserInfoContainer container =
            request->operationContext.get("WMIMapperUserInfoContainer");

        StatProviderTimeMeasurement providerTime(response.get());

        provider.initialize(
            String::equalNoCase(request->authType, LOCAL_CALL_AUTH_TYPE));

        provider.setQualifier(
            request->nameSpace.getString(),
            ((IdentityContainer) request->operationContext.get
                (IdentityContainer::NAME)).getUserName(),
            container.getPassword(),
            request->qualifierDeclaration);
    }
    catch(CIMException& exception)
    {
        response->cimException = exception;
    }
    catch(Exception& exception)
    {
        response->cimException =
            PEGASUS_CIM_EXCEPTION(CIM_ERR_FAILED, exception.getMessage());
    }
    catch(...)
    {
        response->cimException = PEGASUS_CIM_EXCEPTION(CIM_ERR_FAILED,
            "setQualifier() failed!");
    }

    provider.terminate();

    // and send it
    _enqueueResponse(request, response.release());

    PEG_METHOD_EXIT();
}

void CIMOperationRequestDispatcher::handleDeleteQualifierRequest(
   CIMDeleteQualifierRequestMessage* request)
{
    PEG_METHOD_ENTER(TRC_WMI_MAPPER,
        "CIMOperationRequestDispatcher::handleDeleteQualifierRequest");

    AutoPtr<CIMDeleteQualifierResponseMessage> response(
        dynamic_cast<CIMDeleteQualifierResponseMessage *>(
            request->buildResponse()));

    WMIQualifierProvider provider;

    try
    {
        WMIMapperUserInfoContainer container =
            request->operationContext.get("WMIMapperUserInfoContainer");

        StatProviderTimeMeasurement providerTime(response.get());

        provider.initialize(
            String::equalNoCase(request->authType, LOCAL_CALL_AUTH_TYPE));

        provider.deleteQualifier(
            request->nameSpace.getString(),
            ((IdentityContainer) request->operationContext.get
                (IdentityContainer::NAME)).getUserName(),
            container.getPassword(),
            request->qualifierName.getString());
    }
    catch(CIMException& exception)
    {
        response->cimException = exception;
    }
    catch(Exception& exception)
    {
        response->cimException =
            PEGASUS_CIM_EXCEPTION(CIM_ERR_FAILED, exception.getMessage());
    }
    catch(...)
    {
        response->cimException = PEGASUS_CIM_EXCEPTION(CIM_ERR_FAILED,
            "deleteQualifier() failed!");
    }

    provider.terminate();

    // and send it
    _enqueueResponse(request, response.release());

    PEG_METHOD_EXIT();
}

void CIMOperationRequestDispatcher::handleEnumerateQualifiersRequest(
   CIMEnumerateQualifiersRequestMessage* request)
{
    PEG_METHOD_ENTER(TRC_WMI_MAPPER,
        "CIMOperationRequestDispatcher::handleEnumerateQualifiersRequest");

    AutoPtr<CIMEnumerateQualifiersResponseMessage> response(
        dynamic_cast<CIMEnumerateQualifiersResponseMessage *>(
            request->buildResponse()));

    WMIQualifierProvider provider;

    try
    {
        WMIMapperUserInfoContainer container =
            request->operationContext.get("WMIMapperUserInfoContainer");

        StatProviderTimeMeasurement providerTime(response.get());

        provider.initialize(
            String::equalNoCase(request->authType, LOCAL_CALL_AUTH_TYPE));

        response->qualifierDeclarations = provider.enumerateQualifiers(
            request->nameSpace.getString(),
            ((IdentityContainer) request->operationContext.get
                (IdentityContainer::NAME)).getUserName(),
            container.getPassword());
    }
    catch(CIMException& exception)
    {
        response->cimException = exception;
    }
    catch(Exception& exception)
    {
        response->cimException =
            PEGASUS_CIM_EXCEPTION(CIM_ERR_FAILED, exception.getMessage());
    }
    catch(...)
    {
        response->cimException = PEGASUS_CIM_EXCEPTION(CIM_ERR_FAILED,
            "enumerateQualifiers() failed!");
    }

    provider.terminate();

    // and send it
    _enqueueResponse(request, response.release());

    PEG_METHOD_EXIT();
}

void CIMOperationRequestDispatcher::handleExecQueryRequest(
   CIMExecQueryRequestMessage* request)
{
    PEG_METHOD_ENTER(TRC_WMI_MAPPER,
        "CIMOperationRequestDispatcher::handleExecQueryRequest");

    AutoPtr<CIMExecQueryResponseMessage> response(
        dynamic_cast<CIMExecQueryResponseMessage *>(
            request->buildResponse()));

    WMIQueryProvider provider;

    try
    {
        WMIMapperUserInfoContainer container =
            request->operationContext.get("WMIMapperUserInfoContainer");

        StatProviderTimeMeasurement providerTime(response.get());

        provider.initialize(
            String::equalNoCase(request->authType, LOCAL_CALL_AUTH_TYPE));

        response->cimObjects = provider.execQuery(
            request->nameSpace.getString(),
            ((IdentityContainer) request->operationContext.get
                (IdentityContainer::NAME)).getUserName(),
            container.getPassword(),
            request->queryLanguage,
            request->query);
    }
    catch(CIMException& exception)
    {
        response->cimException = exception;
    }
    catch(Exception& exception)
    {
        response->cimException =
            PEGASUS_CIM_EXCEPTION(CIM_ERR_FAILED, exception.getMessage());
    }
    catch(...)
    {
        response->cimException = PEGASUS_CIM_EXCEPTION(CIM_ERR_FAILED,
            "execQuery() failed!");
    }

    provider.terminate();

    // and send it
    _enqueueResponse(request, response.release());

    PEG_METHOD_EXIT();
}

void CIMOperationRequestDispatcher::handleInvokeMethodRequest(
   CIMInvokeMethodRequestMessage* request)
{
    PEG_METHOD_ENTER(TRC_WMI_MAPPER,
        "CIMOperationRequestDispatcher::handleInvokeMethodRequest");

    AutoPtr<CIMInvokeMethodResponseMessage> response(
        dynamic_cast<CIMInvokeMethodResponseMessage *>(
            request->buildResponse()));

    /*
    {
        try
        {
            _fixInvokeMethodParameterTypes(request);
        }
        catch(CIMException& exception)
        {
            response->cimException = exception;
        }
        catch(Exception& exception)
        {
            response->cimException =
                PEGASUS_CIM_EXCEPTION(CIM_ERR_FAILED, exception.getMessage());
        }
        catch(...)
        {
            response->cimException = PEGASUS_CIM_EXCEPTION(CIM_ERR_FAILED,
                String::EMPTY);
        }

        if (errorCode != CIM_ERR_SUCCESS)
        {
            _enqueueResponse(request, response.release());

            PEG_METHOD_EXIT();
            return;
        }
    }
    */

    WMIMethodProvider provider;

    try
    {
        WMIMapperUserInfoContainer container =
            request->operationContext.get("WMIMapperUserInfoContainer");

        StatProviderTimeMeasurement providerTime(response.get());

        provider.initialize(
            String::equalNoCase(request->authType, LOCAL_CALL_AUTH_TYPE));

        response->retValue = provider.invokeMethod(
            request->nameSpace.getString(),
            ((IdentityContainer) request->operationContext.get
                (IdentityContainer::NAME)).getUserName(),
            container.getPassword(),
            request->instanceName,
            request->methodName.getString(),
            request->inParameters,
            response->outParameters);
    }
    catch(CIMException& exception)
    {
        response->cimException = exception;
    }
    catch(Exception& exception)
    {
        response->cimException =
            PEGASUS_CIM_EXCEPTION(CIM_ERR_FAILED, exception.getMessage());
    }
    catch(...)
    {
        response->cimException = PEGASUS_CIM_EXCEPTION(CIM_ERR_FAILED,
            "invokeMethod() failed!");
    }

    // ATTN:  I need some exception handling here
    provider.terminate();

    // and send it
    _enqueueResponse(request, response.release());

    PEG_METHOD_EXIT();
}

/**
   Convert the specified CIMValue to the specified type, and return it in
   a new CIMValue.
*/
CIMValue CIMOperationRequestDispatcher::_convertValueType(
   const CIMValue& value,
   CIMType type)
{
    PEG_METHOD_ENTER(TRC_WMI_MAPPER,
        "CIMOperationRequestDispatcher::_convertValueType");

    CIMValue newValue;

    if (value.isArray())
    {
        Array<String> stringArray;
        Array<char*> charPtrArray;
        Array<const char*> constCharPtrArray;

        //
        // Convert the value to Array<const char*> to send to conversion method
        //
        // ATTN-RK-P3-20020221: Deal with TypeMismatch exception
        // (Shouldn't really ever get that exception)
        value.get(stringArray);

        for (Uint32 k=0; k<stringArray.size(); k++)
        {
            // Need to build an Array<const char*> to send to the conversion
            // routine, but also need to keep track of them pointers as char*
            // because Windows won't let me delete a const char*.
            char* charPtr = strdup(stringArray[k].getCString());
            charPtrArray.append(charPtr);
            constCharPtrArray.append(charPtr);
        }

        //
        // Convert the value to the specified type
        //
        try
        {
            newValue = XmlReader::stringArrayToValue(0,
                constCharPtrArray, type);
        }
        catch (XmlSemanticError&)
        {
            PEG_METHOD_EXIT();
            throw PEGASUS_CIM_EXCEPTION(
            CIM_ERR_INVALID_PARAMETER,
            String("Malformed ") + cimTypeToString (type) + " value");
        }

        for (Uint32 k=0; k<charPtrArray.size(); k++)
        {
            delete charPtrArray[k];
        }
    }
    else
    {
        String stringValue;

        // ATTN-RK-P3-20020221: Deal with TypeMismatch exception
        // (Shouldn't really ever get that exception)
        value.get(stringValue);

        try
        {
            newValue = XmlReader::stringToValue(0,
                stringValue.getCString(), type);
        }
        catch (XmlSemanticError&)
        {
            PEG_METHOD_EXIT();
            throw PEGASUS_CIM_EXCEPTION(
            CIM_ERR_INVALID_PARAMETER,
            String("Malformed ") + cimTypeToString (type) + " value");
        }
    }

    PEG_METHOD_EXIT();
    return newValue;
}


/**
   Find the CIMParamValues in the InvokeMethod request whose types were
   not specified in the XML encoding, and convert them to the types
   specified in the method schema.
*/
void CIMOperationRequestDispatcher::_fixInvokeMethodParameterTypes(
   CIMInvokeMethodRequestMessage* request)
{
    PEG_METHOD_ENTER(TRC_WMI_MAPPER,
        "CIMOperationRequestDispatcher::_fixInvokeMethodParameterTypes");

    Boolean gotMethodDefinition = false;
    CIMMethod method;

    //
    // Cycle through the input parameters, converting the untyped ones.
    //
    Array<CIMParamValue> inParameters = request->inParameters;
    Uint32 numInParamValues = inParameters.size();
    for (Uint32 i=0; i<numInParamValues; i++)
    {
        if (!inParameters[i].isTyped())
        {
            //
            // Retrieve the method definition, if we haven't already done so
            // (only look up the method if we have an untyped parameter value)
            //
            if (!gotMethodDefinition)
            {
                //
                // Get the class definition for this method
                //
                CIMClass cimClass;

                try
                {
                    WMIClassProvider provider;

                    provider.initialize(
                        String::equalNoCase(
                            request->authType, 
                            LOCAL_CALL_AUTH_TYPE));

                    WMIMapperUserInfoContainer container =
                        request->operationContext.get
                        ("WMIMapperUserInfoContainer");

                    cimClass = provider.getClass(
                        request->nameSpace.getString(),
                        ((IdentityContainer) request->operationContext.get
                            (IdentityContainer::NAME)).getUserName(),
                        container.getPassword(),
                        request->instanceName.getClassName().getString(),
                        false, //localOnly,
                        false, //includeQualifiers,
                        false, //includeClassOrigin,
                        CIMPropertyList());
                }
                catch (CIMException&)
                {
                    PEG_METHOD_EXIT();
                    throw;
                }
                catch (Exception&)
                {
                    PEG_METHOD_EXIT();
                    throw;
                }
                catch (...)
                {
                    PEG_METHOD_EXIT();
                    throw PEGASUS_CIM_EXCEPTION(CIM_ERR_METHOD_NOT_FOUND,
                                                String::EMPTY);
                }

                //
                // Get the method definition from the class
                //
                Uint32 methodPos = cimClass.findMethod(request->methodName);
                if (methodPos == PEG_NOT_FOUND)
                {
                    throw CIMException(CIM_ERR_METHOD_NOT_FOUND);
                }
                method = cimClass.getMethod(methodPos);

                gotMethodDefinition = true;
            }

            //
            // Find the parameter definition for this input parameter
            //
            CIMName paramName = inParameters[i].getParameterName();
            Uint32 numParams = method.getParameterCount();
            for (Uint32 j=0; j<numParams; j++)
            {
                CIMParameter param = method.getParameter(j);
                if (paramName == param.getName())
                {
                    //
                    // Retype the input parameter value according to the
                    // type defined in the class/method schema
                    //
                    CIMType paramType = param.getType();
                    CIMValue newValue;

                    if (inParameters[i].getValue().isNull())
                    {
                        newValue.setNullValue(param.getType(),
                            param.isArray());
                    }
                    else if (inParameters[i].getValue().isArray() !=
                                 param.isArray())
                    {
                        // ATTN-RK-P1-20020222: Who catches this?  They aren't.
                        PEG_METHOD_EXIT();
                        throw PEGASUS_CIM_EXCEPTION(CIM_ERR_TYPE_MISMATCH,
                                                    String::EMPTY);
                    }
                    else
                    {
                        newValue = _convertValueType(
                            inParameters[i].getValue(), paramType);
                    }

                    inParameters[i].setValue(newValue);
                    inParameters[i].setIsTyped(true);
                    break;
                }
            }
        }
    }

    PEG_METHOD_EXIT();
}

/**
   Convert the CIMValue given in a SetProperty request to the correct
   type according to the schema, because it is not possible to specify
   the property type in the XML encoding.
   Modified - bbp 08/15/02
*/

void CIMOperationRequestDispatcher::_fixSetPropertyValueType(
   CIMSetPropertyRequestMessage* request)
{
    PEG_METHOD_ENTER(TRC_WMI_MAPPER,
        "CIMOperationRequestDispatcher::_fixSetPropertyValueType");


    String sClassName;
    sClassName = (request->instanceName).getClassName().getString();

    String propertyName;
    propertyName = request->propertyName.getString();

    Array<CIMName> propertyNames;

    CIMName propName = propertyName;
    propertyNames.append(propName);

    CIMPropertyList propertyList = CIMPropertyList(propertyNames);

    CIMValue inValue = request->newValue;

    //
    // Only do the conversion if the type is not already set
    //
    if ((inValue.getType() != CIMTYPE_STRING))
    {
        PEG_METHOD_EXIT();
        return;
    }

    //
    // Get the class definition for this property
    //
    CIMClass cimClass;
    CIMException cimException;
    WMIClassProvider provider;

    try
    {
        WMIMapperUserInfoContainer container =
            request->operationContext.get("WMIMapperUserInfoContainer");

        provider.initialize(
            String::equalNoCase(request->authType, LOCAL_CALL_AUTH_TYPE));

        cimClass = provider.getClass(
            request->nameSpace.getString(),
            ((IdentityContainer) request->operationContext.get
                (IdentityContainer::NAME)).getUserName(),
            container.getPassword(),
            sClassName,
            false,
            false,
            false,
            propertyList);
    }
    catch(CIMException& exception)
    {
        cimException = exception;
    }
    catch(Exception& exception)
    {
        cimException =
            PEGASUS_CIM_EXCEPTION(CIM_ERR_FAILED, exception.getMessage());
    }
    catch(...)
    {
        cimException = PEGASUS_CIM_EXCEPTION(CIM_ERR_FAILED, String::EMPTY);
    }

    //terminate the provider
    provider.terminate();

    //
    // Get the property definition from the class
    //
    Uint32 propertyPos = cimClass.findProperty(request->propertyName);
    if (propertyPos == PEG_NOT_FOUND)
    {
        PEG_METHOD_EXIT();
        throw PEGASUS_CIM_EXCEPTION(CIM_ERR_TYPE_MISMATCH, String::EMPTY);
    }
    CIMProperty property = cimClass.getProperty(propertyPos);

    //
    // Retype the input property value according to the
    // type defined in the schema
    //
    CIMValue newValue;

    if (inValue.isNull())
    {
        newValue.setNullValue(property.getType(), property.isArray());
    }
    else if (inValue.isArray() != property.isArray())
    {
        // ATTN-RK-P1-20020222: Who catches this?  They aren't.
        PEG_METHOD_EXIT();
        throw PEGASUS_CIM_EXCEPTION(CIM_ERR_TYPE_MISMATCH, String::EMPTY);
    }
    else
    {
        newValue = _convertValueType(inValue, property.getType());
    }

    //
    // Put the retyped value back into the message
    //
    request->newValue = newValue;

    PEG_METHOD_EXIT();
}

PEGASUS_NAMESPACE_END
