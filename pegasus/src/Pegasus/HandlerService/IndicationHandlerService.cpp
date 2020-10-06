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
#include <Pegasus/Common/CIMName.h>
#include <Pegasus/Common/Tracer.h>
#include <Pegasus/Common/CIMMessage.h>
#include <Pegasus/Common/XmlWriter.h>
#include <Pegasus/Common/PegasusVersion.h>
#include <Pegasus/Common/Constants.h>
#include <Pegasus/Common/MessageLoader.h>
#include <Pegasus/Common/AutoPtr.h>
#include <Pegasus/Common/StringConversion.h>
#include "IndicationHandlerConstants.h"
#include "IndicationHandlerService.h"

PEGASUS_USING_STD;

PEGASUS_USING_PEGASUS;

PEGASUS_NAMESPACE_BEGIN

#ifdef PEGASUS_ENABLE_DMTF_INDICATION_PROFILE_SUPPORT
static struct timeval deallocateWait = { 300, 0 };
#endif

IndicationHandlerService::IndicationHandlerService(CIMRepository* repository)
    : Base("IndicationHandlerService"),
      _repository(repository)
#ifdef PEGASUS_ENABLE_DMTF_INDICATION_PROFILE_SUPPORT
      ,_destinationQueueTable(),
      _deliveryThreadPool(0, "IndicationHandlerService", 0, 5, deallocateWait),
      _dispatcherThread(_dispatcherRoutine, this, true),
      _maxDeliveryThreads(5),
      _needDestinationQueueCleanup(false) 
#endif
{
#ifdef PEGASUS_ENABLE_DMTF_INDICATION_PROFILE_SUPPORT
    _startDispatcher();
    // Initialize with default value which is three
    _maxDeliveryRetry=3 ; 
    Uint64 v;
    // Determine the value for the configuration parameter
    // maxIndicationDeliveryRetryAttempts
    ConfigManager* configManager = ConfigManager::getInstance();
    String strValue = configManager->getCurrentValue(
        "maxIndicationDeliveryRetryAttempts");
    if (StringConversion::decimalStringToUint64(strValue.getCString(), v) &&
        StringConversion::checkUintBounds(v, CIMTYPE_UINT16) )
    {
        _maxDeliveryRetry = (Uint16)v ;
        PEG_TRACE((
            TRC_IND_HANDLER, Tracer::LEVEL4,
            "Value of maxIndicationDeliveryRetryAttempts when "
                "cimserver start = %u",
            _maxDeliveryRetry));
    }

#endif
}

IndicationHandlerService::~IndicationHandlerService()
{
    PEG_METHOD_ENTER(TRC_IND_HANDLER,
        "IndicationHandlerService::~IndicationHandlerService");

#ifdef PEGASUS_ENABLE_DMTF_INDICATION_PROFILE_SUPPORT
    _stopDispatcher();
    
    if(_needDestinationQueueCleanup)
    {
        _destinationQueuesCleanup();
    } 
#endif

    PEG_METHOD_EXIT();
}

void IndicationHandlerService::_handle_async_request(AsyncRequest* req)
{
    if (req->getType() == ASYNC_CIMSERVICE_STOP)
    {
#ifdef PEGASUS_ENABLE_DMTF_INDICATION_PROFILE_SUPPORT
        _stopDispatcher();
#endif
        handle_CimServiceStop(static_cast<CimServiceStop *>(req));
    }
    else if (req->getType() == ASYNC_ASYNC_LEGACY_OP_START)
    {
        AutoPtr<Message> legacy(
            static_cast<AsyncLegacyOperationStart *>(req)->get_action());

        // Update the requested language to the service thread language context.
        if (dynamic_cast<CIMMessage *>(legacy.get()) != 0)
        {
            try
            {
                ((CIMMessage*)legacy.get())->updateThreadLanguages();
            }
            catch (Exception& e)
            {
                PEG_TRACE((TRC_THREAD, Tracer::LEVEL2,
                    "IndicationHandlerService::_handle_async_request update "
                    "thread languages failed because of %s", 
                    (const char*)e.getMessage().getCString()));
            }
        }

        AutoPtr<CIMResponseMessage> response;

        CIMHandleIndicationRequestMessage *handleIndicationRequest = 0;

        try
        {
            switch(legacy->getType())
            {
                case CIM_HANDLE_INDICATION_REQUEST_MESSAGE:
                    handleIndicationRequest = 
                        (CIMHandleIndicationRequestMessage*) legacy.get();
                    response.reset(_handleIndication(handleIndicationRequest));
                    break;
#ifdef PEGASUS_ENABLE_DMTF_INDICATION_PROFILE_SUPPORT
               case CIM_NOTIFY_SUBSCRIPTION_NOT_ACTIVE_REQUEST_MESSAGE:
                   response.reset(
                       _handleSubscriptionNotActiveRequest(
                           (CIMNotifySubscriptionNotActiveRequestMessage*)
                           legacy.get()));
                   break;
               case CIM_NOTIFY_LISTENER_NOT_ACTIVE_REQUEST_MESSAGE:
                   response.reset(
                       _handleListenerNotActiveRequest(
                           (CIMNotifyListenerNotActiveRequestMessage*)
                           legacy.get()));
                   break;
               case CIM_ENUMERATE_INSTANCES_REQUEST_MESSAGE:
                   response.reset(
                       _handleEnumerateInstancesRequest(
                           (CIMEnumerateInstancesRequestMessage*)
                           legacy.get()));
                   break;
               case CIM_ENUMERATE_INSTANCE_NAMES_REQUEST_MESSAGE:
                   response.reset(
                       _handleEnumerateInstanceNamesRequest(
                           (CIMEnumerateInstanceNamesRequestMessage*)
                           legacy.get()));
                   break;
               case CIM_GET_INSTANCE_REQUEST_MESSAGE:
                   response.reset(
                       _handleGetInstanceRequest(
                           (CIMGetInstanceRequestMessage*)
                           legacy.get()));
                   break;

               case CIM_NOTIFY_CONFIG_CHANGE_REQUEST_MESSAGE:
                   response.reset(
                       _handlePropertyUpdateRequest(
                           (CIMNotifyConfigChangeRequestMessage*)
                               legacy.get()));
                   break;  

#endif
               default:
                   PEG_TRACE((TRC_DISCARDED_DATA, Tracer::LEVEL2,
                       "IndicationHandlerService::_handle_async_request got "
                            "unexpected legacy message type '%u'",
                        legacy->getType()));
                   _make_response(req, async_results::CIM_NAK);
                   return;
            }
        }
        catch (Exception& e)
        {
            response.reset(((CIMRequestMessage*)legacy.get())->buildResponse());
            response->cimException =
                PEGASUS_CIM_EXCEPTION(CIM_ERR_FAILED, e.getMessage());
        }
        catch (...)
        {
            response.reset(((CIMRequestMessage*)legacy.get())->buildResponse());
            response->cimException =
                PEGASUS_CIM_EXCEPTION(CIM_ERR_FAILED, "Exception: Unknown");
        }

        AutoPtr<AsyncLegacyOperationResult> result(
            new AsyncLegacyOperationResult(
                req->op,
                response.get()));

        if (handleIndicationRequest &&
            handleIndicationRequest->deliveryStatusAggregator)
        {
            handleIndicationRequest->deliveryStatusAggregator->complete();
        }

        response.release();
        result.release();
        _complete_op_node(req->op);
    }
    else
    {
        Base::_handle_async_request(req);
    }
}

void IndicationHandlerService::handleEnqueue(Message* message)
{
    PEGASUS_ASSERT(message != 0);

    AutoPtr<CIMMessage> cimMessage(dynamic_cast<CIMMessage *>(message));
    PEGASUS_ASSERT(cimMessage.get() != 0);

    // Set the client's requested language into this service thread.
    // This will allow functions in this service to return messages
    // in the correct language.
    cimMessage->updateThreadLanguages();

    switch (message->getType())
    {
        case CIM_HANDLE_INDICATION_REQUEST_MESSAGE:
        {
            CIMHandleIndicationRequestMessage *handleIndicationRequest =
                (CIMHandleIndicationRequestMessage*) message;

            AutoPtr<CIMHandleIndicationResponseMessage> response(
                _handleIndication(handleIndicationRequest));

            if (handleIndicationRequest->deliveryStatusAggregator)
            {
                handleIndicationRequest->deliveryStatusAggregator->complete();
            }

            SendForget(response.get());
            response.release();
            break;
        }

        default:
            PEGASUS_UNREACHABLE(PEGASUS_ASSERT(0);)
            break;
    }
}

void IndicationHandlerService::handleEnqueue()
{
   AutoPtr<Message> message(dequeue());

   PEGASUS_ASSERT(message.get() != 0);
   if (message.get())
   {
       handleEnqueue(message.get());
       message.release();
   }
}

CIMHandleIndicationResponseMessage* IndicationHandlerService::_handleIndication(
    CIMHandleIndicationRequestMessage* request)
{
    PEG_METHOD_ENTER(TRC_IND_HANDLER,
        "IndicationHandlerService::_handleIndication()");

    Boolean handleIndicationSuccess = true;
    CIMException cimException =
        PEGASUS_CIM_EXCEPTION(CIM_ERR_SUCCESS, String::EMPTY);

    CIMName className = request->handlerInstance.getClassName();
    CIMNamespaceName nameSpace = request->nameSpace;

    CIMInstance indication = request->indicationInstance;
    CIMInstance handler = request->handlerInstance;

    PEG_TRACE ((TRC_INDICATION_GENERATION, Tracer::LEVEL4,
        "Handler service received %s Indication %s for %s:%s.%s Handler",
        (const char*)(indication.getClassName().getString().getCString()),
        (const char*)(request->messageId.getCString()),
        (const char*)(request->nameSpace.getString().getCString()),
        (const char*)(handler.getClassName().getString().getCString()),
        (const char*)(handler.getProperty(handler.findProperty(
        PEGASUS_PROPERTYNAME_NAME)).getValue().toString().getCString())));

    Uint32 pos = PEG_NOT_FOUND;

    if (className.equal (PEGASUS_CLASSNAME_INDHANDLER_CIMXML) ||
        className.equal (PEGASUS_CLASSNAME_LSTNRDST_CIMXML) || 
        className.equal(PEGASUS_CLASSNAME_INDHANDLER_WSMAN))
    {
        pos = handler.findProperty(PEGASUS_PROPERTYNAME_LSTNRDST_DESTINATION);

        if (pos == PEG_NOT_FOUND)
        {
            cimException = PEGASUS_CIM_EXCEPTION_L(CIM_ERR_FAILED,
                MessageLoaderParms(
                    "HandlerService.IndicationHandlerService."
                        "CIMXML_HANDLER_WITHOUT_DESTINATION",
                    "CIMXml Handler missing Destination property"));
            handleIndicationSuccess = false;
        }
        else
        {
            CIMProperty prop = handler.getProperty(pos);
            String destination = prop.getValue().toString();

            if (destination.size() == 0)
            {
                cimException = PEGASUS_CIM_EXCEPTION_L(CIM_ERR_FAILED,
                    MessageLoaderParms(
                        "HandlerService.IndicationHandlerService."
                            "INVALID_DESTINATION",
                        "invalid destination"));
                handleIndicationSuccess = false;
            }
//compared index 10 is not :
            else if (destination.subString(0, 10) == String("localhost/"))
            {
                if (request->deliveryStatusAggregator &&
                    !request->deliveryStatusAggregator->waitUntilDelivered)
                {
                    request->deliveryStatusAggregator->complete();
                    request->deliveryStatusAggregator = 0;
                }
                Uint32 exportServer =
                    find_service_qid(PEGASUS_QUEUENAME_EXPORTREQDISPATCHER);

                // Listener is build with Cimom, so send message to ExportServer
               AutoPtr<CIMExportIndicationRequestMessage> exportmessage(
                    new CIMExportIndicationRequestMessage(
                        XmlWriter::getNextMessageId(),
                        //taking localhost/CIMListener portion out from reg
                        destination.subString(21),
                        indication,
                        QueueIdStack(exportServer, getQueueId()),
                        String::EMPTY,
                        String::EMPTY));

                exportmessage->operationContext.insert(
                    IdentityContainer(String::EMPTY));
                exportmessage->operationContext.set(
                    request->operationContext.get(
                    ContentLanguageListContainer::NAME));
                AutoPtr<AsyncOpNode> op( this->get_op());

                AutoPtr<AsyncLegacyOperationStart> asyncRequest(
                    new AsyncLegacyOperationStart(
                    op.get(),
                    exportServer,
                    exportmessage.get()));

                exportmessage.release();

                PEG_TRACE((TRC_IND_HANDLER, Tracer::LEVEL4,
                    "Indication handler forwarding message to %s",
                        ((MessageQueue::lookup(exportServer)) ?
                            ((MessageQueue::lookup(exportServer))->
                                getQueueName()):
                            "BAD queue name")));
                PEG_TRACE ((TRC_INDICATION_GENERATION, Tracer::LEVEL4,
                    "Sending %s Indication %s to destination %s",
                    (const char*) (indication.getClassName().getString().
                    getCString()),
                    (const char*)(request->messageId.getCString()),
                    (const char*) destination.getCString()));

                //SendAsync(op,
                //      exportServer[0],
                //      IndicationHandlerService::_handleIndicationCallBack,
                //      this,
                //      (void *)request->queueIds.top());
                AutoPtr<AsyncReply> asyncReply(SendWait(asyncRequest.get()));
                asyncRequest.release();

                // Return the ExportIndication results in HandleIndication
                //response
                AutoPtr<CIMExportIndicationResponseMessage> exportResponse(
                    reinterpret_cast<CIMExportIndicationResponseMessage *>(
                        (static_cast<AsyncLegacyOperationResult *>(
                            asyncReply.get()))->get_result()));

                cimException = exportResponse->cimException;

                this->return_op(op.release());
            }
            else
            {
                 // Set sequence-identfier and enqueue if the indication
                 // profile is enabled.
#ifdef PEGASUS_ENABLE_DMTF_INDICATION_PROFILE_SUPPORT
                // If the maxIndicationDeliveryRetryAttempts is set to 0 ,
                // which indicates that reliable indications is disabled,
                // try indication delivery once
                                
                if(_maxDeliveryRetry)   
                {
                    
                    _setSequenceIdentifierAndEnqueue(request);
                    _needDestinationQueueCleanup = true;
                    if (request->deliveryStatusAggregator &&
                        request->deliveryStatusAggregator->waitUntilDelivered)
                    {
                        request->deliveryStatusAggregator = 0;
                    }
                }
                else
                {

                    if (request->deliveryStatusAggregator)
                    {
                        request->deliveryStatusAggregator->complete();
                        request->deliveryStatusAggregator = 0;
                    }
                    handleIndicationSuccess = _loadHandler(
                        request, cimException);
                    // check if DestinationQueue needs to be Cleaned up
                    if(_needDestinationQueueCleanup)
                    {
                        _destinationQueuesCleanup() ;
                    }       
             
                    PEG_TRACE ((TRC_INDICATION_GENERATION, Tracer::LEVEL4,
                        "Reliable indication is  %s",
                        _maxDeliveryRetry ? "enable" : "disable"));
                }  

#else
                if (request->deliveryStatusAggregator)
                {
                    request->deliveryStatusAggregator->complete();
                    request->deliveryStatusAggregator = 0;
                }
                handleIndicationSuccess = _loadHandler(request, cimException);
#endif
            }
        }
    }
    else if (className.equal (PEGASUS_CLASSNAME_INDHANDLER_SNMP))
    {
        if (request->deliveryStatusAggregator)
        {
            request->deliveryStatusAggregator->complete();
            request->deliveryStatusAggregator = 0;
        }
        pos = handler.findProperty(PEGASUS_PROPERTYNAME_LSTNRDST_TARGETHOST);

        if (pos == PEG_NOT_FOUND)
        {
            cimException = PEGASUS_CIM_EXCEPTION_L(CIM_ERR_FAILED,
                MessageLoaderParms(
                    "HandlerService.IndicationHandlerService."
                        "SNMP_HANDLER_WITHOUT_TARGETHOST",
                    "Snmp Handler missing Targethost property"));
            handleIndicationSuccess = false;
        }
        else
        {
            CIMProperty prop = handler.getProperty(pos);
            String destination = prop.getValue().toString();

            if (destination.size() == 0)
            {
                cimException = PEGASUS_CIM_EXCEPTION_L(CIM_ERR_FAILED,
                    MessageLoaderParms(
                        "HandlerService.IndicationHandlerService."
                            "INVALID_TARGETHOST",
                        "invalid targethost"));
                handleIndicationSuccess = false;
            }
            else
            {
                handleIndicationSuccess = _loadHandler(request, cimException);
            }
        }
    }
    else if ((className.equal (PEGASUS_CLASSNAME_LSTNRDST_SYSTEM_LOG)) ||
             (className.equal (PEGASUS_CLASSNAME_LSTNRDST_EMAIL)) ||
             (className.equal (PEGASUS_CLASSNAME_LSTNRDST_FILE)))
    {
        if (request->deliveryStatusAggregator)
        {
            request->deliveryStatusAggregator->complete();
            request->deliveryStatusAggregator = 0;
        }
        handleIndicationSuccess = _loadHandler(request, cimException);
    }

    // no success to handle indication
    // somewhere an exception message was build
    // time to write the error message to the log
    if (!handleIndicationSuccess)
    {
        Logger::put_l(Logger::ERROR_LOG, System::CIMSERVER, Logger::WARNING,
            MessageLoaderParms(
                "HandlerService.IndicationHandlerService."
                    "INDICATION_DELIVERY_FAILED",
                "Failed to deliver an indication: $0",
                cimException.getMessage()));
    }

    CIMHandleIndicationResponseMessage* response =
        dynamic_cast<CIMHandleIndicationResponseMessage*>(
            request->buildResponse());
    response->cimException = cimException;

    PEG_METHOD_EXIT();
    return response;
}

Boolean IndicationHandlerService::_loadHandler(
    CIMHandleIndicationRequestMessage* request,
    CIMException& cimException)
{
    return _loadHandler(
                request->operationContext,
                request->nameSpace.getString(),
                request->indicationInstance,
                request->handlerInstance,
                request->subscriptionInstance,
                cimException,
                0);
}

Boolean IndicationHandlerService::_loadHandler(
    const OperationContext& operationContext,
    const String nameSpace,
    CIMInstance& indicationInstance,
    CIMInstance& handlerInstance,
    CIMInstance& subscriptionInstance,
    CIMException& cimException,
    IndicationExportConnection **connection)
{
    PEG_METHOD_ENTER(TRC_IND_HANDLER,
        "IndicationHandlerService::_loadHandler()");


    try
    {
        CIMName className = handlerInstance.getClassName();
        CIMHandler* handlerLib = _lookupHandlerForClass(className);

        if (handlerLib)
        {
            ContentLanguageList langs =
                ((ContentLanguageListContainer)operationContext.
                get(ContentLanguageListContainer::NAME)).getLanguages();
            handlerLib->handleIndication(
                operationContext,
                nameSpace,
                indicationInstance,
                handlerInstance,
                subscriptionInstance,
                langs,
                connection);
        }
        else
        {
            cimException = PEGASUS_CIM_EXCEPTION_L(CIM_ERR_FAILED,
                MessageLoaderParms("HandlerService."
                "IndicationHandlerService.FAILED_TO_LOAD",
                "Failed to load Handler"));
            PEG_METHOD_EXIT();
            return false;
        }

    }
    catch (Exception& e)
    {
        cimException =
            PEGASUS_CIM_EXCEPTION(CIM_ERR_FAILED, e.getMessage());
        PEG_METHOD_EXIT();
        return false;
    }
    catch (...)
    {
        cimException =
            PEGASUS_CIM_EXCEPTION(CIM_ERR_FAILED, "Exception: Unknown");
        PEG_METHOD_EXIT();
        return false;
    }
    PEG_METHOD_EXIT();
    return true;
}

CIMHandler* IndicationHandlerService::_lookupHandlerForClass(
   const CIMName& className)
{
   PEG_METHOD_ENTER(TRC_IND_HANDLER,
        "IndicationHandlerService::_lookupHandlerForClass()");

   String handlerId;

   if (className.equal(PEGASUS_CLASSNAME_INDHANDLER_CIMXML) ||
       className.equal(PEGASUS_CLASSNAME_LSTNRDST_CIMXML))
   {
       handlerId = String("CIMxmlIndicationHandler");
   }
   else if (className.equal(PEGASUS_CLASSNAME_INDHANDLER_SNMP))
   {
       handlerId = String("snmpIndicationHandler");
   }
   else if (className.equal(PEGASUS_CLASSNAME_LSTNRDST_SYSTEM_LOG))
   {
       handlerId = String("SystemLogListenerDestination");
   }
   else if (className.equal(PEGASUS_CLASSNAME_LSTNRDST_EMAIL))
   {
       handlerId = String("EmailListenerDestination");
   }
   else if (className.equal(PEGASUS_CLASSNAME_INDHANDLER_WSMAN))
   {
       handlerId = String("wsmanIndicationHandler"); 
   }
   else if (className.equal(PEGASUS_CLASSNAME_LSTNRDST_FILE))
   {
       handlerId = String("FileListenerDestination");
   }


   PEGASUS_ASSERT(handlerId.size() != 0);

   CIMHandler* handler = _handlerTable.getHandler(handlerId, _repository);

   PEG_METHOD_EXIT();
   return handler;
}

#ifdef PEGASUS_ENABLE_DMTF_INDICATION_PROFILE_SUPPORT

CIMResponseMessage*
     IndicationHandlerService::_handleGetInstanceRequest(
         CIMGetInstanceRequestMessage *message)
{
    PEG_METHOD_ENTER(TRC_IND_HANDLER,
        "IndicationHandlerService::_handleGetInstanceRequest");

    CIMGetInstanceResponseMessage* response =
         dynamic_cast<CIMGetInstanceResponseMessage*>
             (message->buildResponse());

    response->getResponseData().setInstances(
        _getDestinationQueues(
            message->instanceName,
            message->includeQualifiers,
            message->includeClassOrigin,
            message->propertyList));

    PEG_METHOD_EXIT();

    return response;
}

CIMResponseMessage*
     IndicationHandlerService::_handleEnumerateInstancesRequest(
         CIMEnumerateInstancesRequestMessage *message)
{
    PEG_METHOD_ENTER(TRC_IND_HANDLER,
        "IndicationHandlerService::_handleEnumerateInstancesRequest");

    CIMEnumerateInstancesResponseMessage* response =
         dynamic_cast<CIMEnumerateInstancesResponseMessage*>
             (message->buildResponse());

    response->getResponseData().setInstances(
        _getDestinationQueues(
            CIMObjectPath(),
            message->includeQualifiers,
            message->includeClassOrigin,
            message->propertyList));

    PEG_METHOD_EXIT();

    return response;
}

Array<CIMInstance> IndicationHandlerService::_getDestinationQueues(
    const CIMObjectPath &getInstanceName,
    Boolean includeQualifiers,
    Boolean includeClassOrigin,
    const CIMPropertyList &propertyList)
{
    Boolean found = false;
    CIMClass cimClass =
        _repository->getClass(
            PEGASUS_NAMESPACENAME_INTERNAL,
            PEGASUS_CLASSNAME_PG_LSTNRDSTQUEUE,
            false,
            true,
            true,
            CIMPropertyList());

    Array<CIMInstance> instances;

    ReadLock lock(_destinationQueueTableLock);
    DestinationQueueTable::Iterator i =
        _destinationQueueTable.start();
    DestinationQueue *queue;

    CIMInstance sInstance = cimClass.buildInstance(
        true,
        true,
        CIMPropertyList());

    Uint32 propArray[12];

    propArray[0] = sInstance.findProperty(_PROPERTY_LSTNRDST_NAME);
    propArray[1] = sInstance.findProperty(_PROPERTY_CREATIONTIME);
    propArray[2] = sInstance.findProperty(_PROPERTY_SEQUENCECONTEXT);
    propArray[3] = sInstance.findProperty(_PROPERTY_NEXTSEQUENCENUMBER);
    propArray[4] = sInstance.findProperty(_PROPERTY_MAXQUEUELENGTH);
    propArray[5] = sInstance.findProperty(_PROPERTY_SEQUENCEIDENTIFIERLIFETIME);
    propArray[6] = sInstance.findProperty(_PROPERTY_CURRENTINDICATIONS);
    propArray[7] = sInstance.findProperty(
        _PROPERTY_QUEUEFULLDROPPEDINDICATIONS);
    propArray[8] = sInstance.findProperty(_PROPERTY_LIFETIMEEXPIREDINDICATIONS);
    propArray[9] = sInstance.findProperty(
        _PROPERTY_RETRYATTEMPTSEXCEEDEDINDICATIONS);
    propArray[10] = sInstance.findProperty(
        _PROPERTY_SUBSCRIPTIONDISABLEDROPPEDINDICATIONS);
    propArray[11] = sInstance.findProperty(
        _PROPERTY_LASTSUCCESSFULDELIVERYTIME);

    DestinationQueue::QueueInfo qinfo;
    for(; i; i++)
    {
        queue = i.value();
        queue->getInfo(qinfo);

        Array<CIMKeyBinding> kbArray;
        kbArray.append(
            CIMKeyBinding(
                _PROPERTY_LSTNRDST_NAME,
                _getQueueName(queue->getHandler().getPath()),
                CIMKeyBinding::STRING));

        CIMObjectPath instanceName = CIMObjectPath(
            String(),
            CIMNamespaceName(),
            PEGASUS_CLASSNAME_PG_LSTNRDSTQUEUE,
            kbArray);

        if (getInstanceName.getKeyBindings().size())
        {
            if (instanceName.identical(getInstanceName))
            {
                found = true;
            }
            else
            {
                continue;
            }
        }

        CIMInstance instance = sInstance.clone();

        instance.setPath(instanceName);

       instance.getProperty(propArray[0]).setValue(
                CIMValue(_getQueueName(qinfo.handlerName)));

        instance.getProperty(propArray[1]).setValue(
                CIMValue(qinfo.queueCreationTimeUsec));

        instance.getProperty(propArray[2]).setValue(
                CIMValue(qinfo.sequenceContext));

        instance.getProperty(propArray[3]).setValue(
                CIMValue(qinfo.nextSequenceNumber));

        instance.getProperty(propArray[4]).setValue(
                CIMValue(qinfo.maxQueueLength));

        instance.getProperty(propArray[5]).setValue(
                CIMValue(qinfo.sequenceIdentifierLifetimeSeconds));

        instance.getProperty(propArray[6]).setValue(
                CIMValue(qinfo.size));

        instance.getProperty(propArray[7]).setValue(
                CIMValue(qinfo.queueFullDroppedIndications));

        instance.getProperty(propArray[8]).setValue(
                CIMValue(qinfo.lifetimeExpiredIndications));

        instance.getProperty(propArray[9]).setValue(
                CIMValue(qinfo.retryAttemptsExceededIndications));

        instance.getProperty(propArray[10]).setValue(
                CIMValue(qinfo.subscriptionDisableDroppedIndications));

        instance.getProperty(propArray[11]).setValue(
                CIMValue(qinfo.lastSuccessfulDeliveryTimeUsec));

        filterInstance(
            includeQualifiers,
            includeClassOrigin,
            propertyList,
            instance);

        instances.append(instance);
        if (found)
        {
            return instances;
        }
    }

    return instances;
}

CIMResponseMessage*
     IndicationHandlerService::_handleEnumerateInstanceNamesRequest(
         CIMEnumerateInstanceNamesRequestMessage *message)
{
    PEG_METHOD_ENTER(TRC_IND_HANDLER,
        "IndicationHandlerService::_handleEnumerateInstanceNamesRequest");

    CIMEnumerateInstanceNamesResponseMessage* response =
         dynamic_cast<CIMEnumerateInstanceNamesResponseMessage*>
             (message->buildResponse());

    Array<CIMObjectPath> instanceNames;

    ReadLock lock(_destinationQueueTableLock);

    DestinationQueueTable::Iterator i =
        _destinationQueueTable.start();

    DestinationQueue *queue;

    for(; i; i++)
    {
        queue = i.value();

        Array<CIMKeyBinding> kbArray;
        kbArray.append(
            CIMKeyBinding(
                _PROPERTY_LSTNRDST_NAME,
                _getQueueName(queue->getHandler().getPath()),
                CIMKeyBinding::STRING));

        CIMObjectPath instanceName = CIMObjectPath(
            String(),
            PEGASUS_NAMESPACENAME_INTERNAL,
            PEGASUS_CLASSNAME_PG_LSTNRDSTQUEUE,
            kbArray);

        instanceNames.append(instanceName);
    }

    response->getResponseData().setInstanceNames(instanceNames);
    PEG_METHOD_EXIT();

    return response;
}

CIMNotifySubscriptionNotActiveResponseMessage*
    IndicationHandlerService::_handleSubscriptionNotActiveRequest(
        CIMNotifySubscriptionNotActiveRequestMessage *message)
{
    PEG_METHOD_ENTER(TRC_IND_HANDLER,
        "IndicationHandlerService::_handleSubscriptionNotActiveRequest");

    String queueName = _getQueueName(message->subscriptionName);

    DestinationQueue *queue = 0;
    WriteLock lock(_destinationQueueTableLock);
    if (_destinationQueueTable.lookup(queueName, queue))
    {
        queue->deleteMatchedIndications(message->subscriptionName);
    }

    CIMNotifySubscriptionNotActiveResponseMessage *response =
        dynamic_cast<CIMNotifySubscriptionNotActiveResponseMessage*>(
            message->buildResponse());

    PEG_METHOD_EXIT();
    return response;
}

CIMNotifyListenerNotActiveResponseMessage*
    IndicationHandlerService::_handleListenerNotActiveRequest(
        CIMNotifyListenerNotActiveRequestMessage *message)
{
    PEG_METHOD_ENTER(TRC_IND_HANDLER,
        "IndicationHandlerService::_handleListenerNotActiveRequest");

    DestinationQueue *queue = 0;
    WriteLock lock(_destinationQueueTableLock);

    String queueName = _getQueueName(message->handlerName);

    if (_destinationQueueTable.lookup(queueName, queue))
    {
        queue->cleanup();
        delete queue;
        PEGASUS_FCT_EXECUTE_AND_ASSERT(
            true,
            _destinationQueueTable.remove(queueName));
    }

    CIMNotifyListenerNotActiveResponseMessage *response =
        dynamic_cast<CIMNotifyListenerNotActiveResponseMessage*>(
            message->buildResponse());
    PEG_METHOD_EXIT();

    return response;
}

//
// Update the DeliveryRetryAttempts & DeliveryRetryInterval
// with the new property value
//
CIMNotifyConfigChangeResponseMessage*
    IndicationHandlerService::_handlePropertyUpdateRequest(
        CIMNotifyConfigChangeRequestMessage *message)
{
    PEG_METHOD_ENTER(TRC_IND_HANDLER,
        "IndicationHandlerService::_handlePropertyUpdateRequest");

    CIMNotifyConfigChangeRequestMessage * notifyRequest=
           dynamic_cast<CIMNotifyConfigChangeRequestMessage*>(message);

    Uint64 v;

    StringConversion::decimalStringToUint64(
        notifyRequest->newPropertyValue.getCString(),v);

    if (String::equal(
        notifyRequest->propertyName, "maxIndicationDeliveryRetryAttempts")) 
    {
        _maxDeliveryRetry = (Uint16)v ;  
        DestinationQueue::setDeliveryRetryAttempts(v);
    }
    else if(String::equal(
        notifyRequest->propertyName, "minIndicationDeliveryRetryInterval"))
    {
        DestinationQueue:: setminDeliveryRetryInterval(v);
    }
    else
    {
        PEGASUS_UNREACHABLE(PEGASUS_ASSERT(0);)
    }


    CIMNotifyConfigChangeResponseMessage *response =
        dynamic_cast<CIMNotifyConfigChangeResponseMessage*>(
            message->buildResponse());
    PEG_METHOD_EXIT();
    return response;
}



void IndicationHandlerService::_stopDispatcher()
{
    PEG_METHOD_ENTER(TRC_IND_HANDLER,
        "IndicationHandlerService::_stopDispatcher");

    _stopDispatcherThread++;
    _dispatcherWaitSemaphore.signal();

    while (_dispatcherThreadRunning.get())
    {
        Threads::yield();
        Threads::sleep(50);
    }

    PEG_TRACE_CSTRING(TRC_IND_HANDLER,Tracer::LEVEL4,
        "Dispatcher thread stopped");
    PEG_METHOD_EXIT();
}

void IndicationHandlerService::_deliverIndication(IndicationInfo *info)
{
    CIMException cimException;

    Boolean deliveryOk =  _loadHandler(
        info->context,
        info->nameSpace,
        info->indication,
        info->queue->getHandler(),
        info->subscription,
        cimException,
        info->queue->getConnectionPtr());

   if (deliveryOk)
   {
       info->queue->updateDeliveryRetrySuccess(info);
   }
   else
   {
       info->queue->updateDeliveryRetryFailure(info, cimException);
   }
}

void IndicationHandlerService::_setSequenceIdentifierAndEnqueue(
    CIMHandleIndicationRequestMessage *message)
{
    PEG_METHOD_ENTER(TRC_IND_HANDLER,
        "IndicationHandlerService::_setSequenceIdentifierAndEnqueue");

    CIMInstance handler = message->handlerInstance;
    String queueName = _getQueueName(message->subscriptionInstance.getPath());

    DestinationQueue *queue;

    {
        ReadLock lock(_destinationQueueTableLock);

        if (_destinationQueueTable.lookup(queueName, queue))
        {
            queue->enqueue(message);
            _dispatcherWaitSemaphore.signal();
            PEG_TRACE((TRC_IND_HANDLER, Tracer::LEVEL4,
                "DestinationQueue %s already exists",
                (const char*)queueName.getCString()));
            PEG_METHOD_EXIT();
            return;
        }
    }

    WriteLock lock(_destinationQueueTableLock);
    if (_destinationQueueTable.lookup(queueName, queue))
    {
        queue->enqueue(message);
        _dispatcherWaitSemaphore.signal();
        PEG_TRACE((TRC_IND_HANDLER, Tracer::LEVEL4,
            "DestinationQueue %s already exists",
            (const char*)queueName.getCString()));
        PEG_METHOD_EXIT();
        return;
    }

    queue = new DestinationQueue(handler);
    PEGASUS_FCT_EXECUTE_AND_ASSERT(
        true,
        _destinationQueueTable.insert(queueName, queue));

    queue->enqueue(message);
    _dispatcherWaitSemaphore.signal();
    PEG_TRACE((TRC_IND_HANDLER, Tracer::LEVEL4,
        "DestinationQueue %s created",
        (const char*)queueName.getCString()));
    PEG_METHOD_EXIT();
}


void  IndicationHandlerService::_startDispatcher()
{
    ThreadStatus tr;
    while ((tr = _dispatcherThread.run()) != PEGASUS_THREAD_OK)
    {
        if (tr == PEGASUS_THREAD_INSUFFICIENT_RESOURCES)
        {
            Threads::yield();
        }
        else
        {
            throw Exception(
                MessageLoaderParms(
                    "HandlerService.IndicationHandlerService."
                        "NOT_ENOUGH_THREADS",
                    "Could not allocate thread for indication dispatcher"));
        }
    }
    _dispatcherThreadRunning++;
}

String IndicationHandlerService::_getQueueName(
    const CIMObjectPath &instancePath)
{
    PEG_METHOD_ENTER(TRC_IND_HANDLER,
        "IndicationHandlerService::_getQueueName");

    Array<CIMKeyBinding> subscriptionKB = instancePath.getKeyBindings();
    CIMObjectPath handlerName = instancePath;

    for (Uint32 i = 0, n = subscriptionKB.size(); i < n ; i++)
    {
        if ((subscriptionKB [i].getName() == PEGASUS_PROPERTYNAME_HANDLER) &&
            (subscriptionKB [i].getType() == CIMKeyBinding::REFERENCE))
        {
            handlerName = subscriptionKB[i].getValue();
            break;
        }
    }

    Array<CIMKeyBinding> handlerKB = handlerName.getKeyBindings();
    String queueName;

    if (handlerName.getNameSpace().isNull())
    {
        queueName = instancePath.getNameSpace().getString();
    }
    else
    {
        queueName = handlerName.getNameSpace().getString();
    }

    queueName.append(Char16(':'));
    queueName.append(handlerName.getClassName().getString());
    queueName.append(Char16('.'));

    for (Uint32 i = 0 , n = handlerKB.size(); i < n ; i++)
    {
        if (handlerKB [i].getName() == PEGASUS_PROPERTYNAME_NAME)
        {
            queueName.append(handlerKB[i].getValue());
            break;
        }
    }

    PEG_TRACE((TRC_IND_HANDLER, Tracer::LEVEL4,
       "Returning the queue name %s",
       (const char*)queueName.getCString()));
    PEG_METHOD_EXIT();

    return queueName;
}

ThreadReturnType PEGASUS_THREAD_CDECL
    IndicationHandlerService::_dispatcherRoutine(void *parm)
{
    PEG_METHOD_ENTER(TRC_IND_HANDLER,
        "IndicationHandlerService::_dispatcherRoutine");

    Thread *myself = reinterpret_cast<Thread *>(parm);
    IndicationHandlerService *service =
        reinterpret_cast<IndicationHandlerService*>(myself->get_parm());

    const Uint32 RETRY_THREAD_MAX_WAITTIME = // milliseconds
        DestinationQueue::getDeliveryRetryIntervalSeconds() * 1000;

    DestinationQueue *queue;
    Uint64 timeNowUsec;
    IndicationInfo *indication;
    Uint64 idleTimeoutUsec;

    // Holds the minimum of next eligible indication DeliveryRetryInterval
    // expiration time from all DestinationQueues.
    Uint32 nextMinIndDRIExpTime = RETRY_THREAD_MAX_WAITTIME;

    idleTimeoutUsec = System::getCurrentTimeUsec();
    service->_deliveryThreadsRunningCount = 0;

    for (;;)
    {
        try
        {
            service->_dispatcherWaitSemaphore.time_wait(nextMinIndDRIExpTime);

            // Check if we need to terminate
            if (service->_stopDispatcherThread.get())
            {
                service->_dispatcherThreadRunning = 0;
                break;
            }

            nextMinIndDRIExpTime = RETRY_THREAD_MAX_WAITTIME;
            timeNowUsec = System::getCurrentTimeUsec();

            Uint64 nextIndDRIExpTime;

            {
                ReadLock lock(service->_destinationQueueTableLock);

                DestinationQueueTable::Iterator i =
                    service->_destinationQueueTable.start();

                for(; i; i++)
                {
                    queue = i.value();
                    if ((indication = queue->getNextIndicationForDelivery(
                        timeNowUsec, nextIndDRIExpTime)))
                    {
                        service->_deliveryQueue.insert_back(indication);
                        if (service->_deliveryThreadsRunningCount.get()
                            < service->_maxDeliveryThreads)
                        {
                            service->_deliveryThreadsRunningCount++;
                            if (service->
                                _deliveryThreadPool.allocate_and_awaken(
                                    service, _deliveryRoutine, 0)
                                        != PEGASUS_THREAD_OK)
                            {
                                service->_deliveryThreadsRunningCount--;
                            }
                        }
                    }

                    // convert to milliseconds
                    nextIndDRIExpTime/= 1000;
                    if (nextIndDRIExpTime < nextMinIndDRIExpTime)
                    {
                        nextMinIndDRIExpTime = nextIndDRIExpTime;
                    }
                }
            }

            // Cleanup idle threads for every 5 minutes
            if (timeNowUsec - idleTimeoutUsec >= 300000000)
            {
                service->_deliveryThreadPool.cleanupIdleThreads();

                PEG_TRACE_CSTRING(TRC_IND_HANDLER,Tracer::LEVEL4,
                    "DeliveryThreadPool.cleanupIdleThreads() called");

                idleTimeoutUsec = timeNowUsec;
            }
        }
        catch(const Exception &e)
        {
            PEG_TRACE((TRC_IND_HANDLER, Tracer::LEVEL4,
                "Unexpected exception in IndicationHandlerService::"
                    "_dispatcherRoutine() : %s",
            (const char*)e.getMessage().getCString()));
        }
        catch(...)
        {
            PEG_TRACE_CSTRING(TRC_IND_HANDLER, Tracer::LEVEL4,
                "Unexpected exception in IndicationHandlerService::"
                    "_dispatcherRoutine() : Unknown");
        }
    }
    PEG_METHOD_EXIT();

    return (ThreadReturnType)0;
}

ThreadReturnType PEGASUS_THREAD_CDECL
    IndicationHandlerService::_deliveryRoutine(void *parm)
{
    PEG_METHOD_ENTER(TRC_IND_HANDLER,
        "IndicationHandlerService::_deliveryRoutine");

    IndicationHandlerService *service =
        reinterpret_cast<IndicationHandlerService *>(parm);

    IndicationInfo *indication;

    while ((indication = service->_deliveryQueue.remove_front()))
    {
        service->_deliverIndication(indication);
        // Notify the dispatcher that the indication delivery has finished.
        service->_dispatcherWaitSemaphore.signal();
    }
    service->_deliveryThreadsRunningCount--;
    PEG_METHOD_EXIT();

    return (ThreadReturnType)0;
}

void IndicationHandlerService::filterInstance(bool includeQualifiers,
    bool includeClassOrigin,
    const CIMPropertyList& myPropertyList,
    CIMInstance & newInstance)
{
    if(!includeQualifiers && newInstance.getQualifierCount() > 0)
    {
        while(newInstance.getQualifierCount())
        {
            newInstance.removeQualifier(0);
        }
    }
    // Delete any properties not on the property list
    for(Uint32 i=0; i < newInstance.getPropertyCount();i++)
    {
        CIMConstProperty p = newInstance.getProperty(i);
        CIMName name = p.getName();
        Array<CIMName> pl = myPropertyList.getPropertyNameArray();
        if (myPropertyList.isNull() || Contains(pl, name))
        {
            if (!includeClassOrigin)
            {
                (newInstance.getProperty(i)).setClassOrigin(CIMName());
            }
            if (!includeQualifiers && newInstance.getProperty(i).
                getQualifierCount() > 0)
            {
                while (newInstance.getProperty(i).getQualifierCount() > 0)
                {
                    newInstance.getProperty(i).removeQualifier(0);
                }
            }
        }
        else
        {
            newInstance.removeProperty(i--);
        }

    }

}

void IndicationHandlerService:: _destinationQueuesCleanup()
{
    WriteLock lock(_destinationQueueTableLock);
    _needDestinationQueueCleanup = false;
    // Cleanup all DestinationQueues.
    DestinationQueueTable::Iterator i =  _destinationQueueTable.start();
    DestinationQueue *queue;
    for(; i; i++)
    {
        queue = i.value();
        queue->shutdown();
        delete queue;
    }
    _destinationQueueTable.clear();
}
#endif

PEGASUS_NAMESPACE_END
