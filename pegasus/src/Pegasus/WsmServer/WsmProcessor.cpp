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

#include <cctype>
#include <cstdio>
#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/Tracer.h>
#include <Pegasus/Common/MessageLoader.h>
#include <Pegasus/Common/StringConversion.h>
#include <Pegasus/Common/AutoPtr.h>
#include "WsmConstants.h"
#include "SoapResponse.h"
#include "WsmProcessor.h"

PEGASUS_USING_STD;

PEGASUS_NAMESPACE_BEGIN

Uint64 WsmProcessor::_currentEnumContext = 0;

WsmProcessor::WsmProcessor(
    MessageQueue* cimOperationProcessorQueue,
    CIMRepository* repository)
    : MessageQueue(PEGASUS_QUEUENAME_WSMPROCESSOR),
      _wsmResponseEncoder(),
      _wsmRequestDecoder(this),
      _cimOperationProcessorQueue(cimOperationProcessorQueue),
      _repository(repository),
      _wsmToCimRequestMapper(repository)
{
    _initializeSubInfoTable();
}

WsmProcessor::~WsmProcessor()
{
    // Clean up enumeration responses that have not been pulled or released.
    for (WsmEnumerationContextTable::Iterator i =
             _WsmEnumerationContextTable.start(); i; i++)
    {
        delete i.value().response;
    }
}

void WsmProcessor::handleEnqueue(Message* message)
{
    if (!message)
    {
        return;
    }

    PEGASUS_ASSERT(dynamic_cast<CIMResponseMessage*>(message) != 0);
    handleResponse(dynamic_cast<CIMResponseMessage*>(message));
}

void WsmProcessor::handleEnqueue()
{
    Message* message = dequeue();
    handleEnqueue(message);
}

void WsmProcessor::addReqToSubContext(
    WsmRequest* wsmRequest,
    Boolean isCreateReq)
{
    PEG_METHOD_ENTER(TRC_WSMSERVER, "WsmProcessor::addReqToSubContext()");
    SubscriptionContext *subContext=0;
    String className;
    WxfSubCreateRequest *wsmCreateRequest=0;
    WxfSubDeleteRequest *wsmDeleteRequest=0;
    if(isCreateReq == true)
    {
        wsmCreateRequest = (WxfSubCreateRequest *)wsmRequest;
        className = wsmCreateRequest->instance.getClassName();
    }
    else
    {
        wsmDeleteRequest = (WxfSubDeleteRequest *)wsmRequest;
        className = wsmDeleteRequest->className;
    }

    AutoMutex lock(_subscriptionContextTableLock);
    // Look up for the Subscription context in the hash table.
    // If it does not exist, create a new entry.
    if(_subscriptionContextTable.contains(wsmRequest->messageId))
    {
        _subscriptionContextTable.lookupReference(
            wsmRequest->messageId,
            subContext);
    }
    else
    {
        SubscriptionContext subCon(wsmRequest->messageId);
        _subscriptionContextTable.insert(wsmRequest->messageId, subCon);
        _subscriptionContextTable.lookupReference(
            wsmRequest->messageId,
            subContext);
    }

    // Fill in the filter, handler and subscription requests
    if(className == PEGASUS_CLASSNAME_INDFILTER.getString())
    {
        if(isCreateReq == true)
        {
            subContext->filterReq = wsmCreateRequest;
        }
        else
        {
            subContext->filterDeleteReq = wsmDeleteRequest;
        }
    }
    else if ( className == PEGASUS_CLASSNAME_INDHANDLER_WSMAN.getString())
    {
        if (isCreateReq == true)
        {
            subContext->handlerReq = wsmCreateRequest;
        }
        else
        {
            subContext->handlerDeleteReq = wsmDeleteRequest;
        }
    }
    else if ( className == PEGASUS_CLASSNAME_INDSUBSCRIPTION.getString())
    {
        subContext->subReq = wsmCreateRequest;
    }
    PEG_METHOD_EXIT();
}


void WsmProcessor::handleRequest(WsmRequest* wsmRequest)
{
    PEG_METHOD_ENTER(TRC_WSMSERVER, "WsmProcessor::handleRequest()");

    // Process requests by type.  For now, only WS-Transfer operations are
    // implemented, and they all are handled by forwarding to the CIM Server.

    AutoPtr<WsmRequest> wsmRequestDestroyer(wsmRequest);
    try
    {
        CIMOperationRequestMessage* cimRequest =
            _wsmToCimRequestMapper.mapToCimRequest(wsmRequest);

        // Requests that do not have a CIM representation are mapped to NULL
        // and are meant to be handled by the WSM processor itself.
        if (cimRequest)
        {
            // Save the request until the response comes back.
            // Note that the CIM request has its own unique message ID.
            _requestTable.insert(cimRequest->messageId, wsmRequest);

            cimRequest->queueIds.push(getQueueId());
            _cimOperationProcessorQueue->enqueue(cimRequest);

            wsmRequestDestroyer.release();
        }
        else
        {
            switch (wsmRequest->getType())
            {
                case WS_ENUMERATION_PULL:
                    _handlePullRequest((WsenPullRequest*) wsmRequest);
                    break;

                case WS_ENUMERATION_RELEASE:
                    _handleReleaseRequest((WsenReleaseRequest*) wsmRequest);
                    break;

                default:
                    break;
            }
        }
    }
    catch (WsmFault& fault)
    {
        sendResponse(new WsmFaultResponse(wsmRequest, fault),wsmRequest);
    }
    catch (CIMException& e)
    {
        sendResponse(
            new WsmFaultResponse(
                wsmRequest,
                _cimToWsmResponseMapper.mapCimExceptionToWsmFault(e)),
            wsmRequest);
    }
    catch (Exception& e)
    {
        sendResponse(
            new WsmFaultResponse(
                wsmRequest,
                WsmFault(
                    WsmFault::wsman_InternalError,
                    e.getMessage(),
                    e.getContentLanguages())),
            wsmRequest);
    }
    catch (PEGASUS_STD(exception)& e)
    {
        sendResponse(
            new WsmFaultResponse(
                wsmRequest,
                WsmFault(WsmFault::wsman_InternalError, e.what())),
            wsmRequest);
    }
    catch (...)
    {
        sendResponse(
            new WsmFaultResponse(
                wsmRequest,
                WsmFault(WsmFault::wsman_InternalError)),
            wsmRequest);
    }
    // Note this requirement when Enumerate/Pull operations are supported:
    // DSP0226 R6.3-5: For operations that span multiple message sequences,
    // the wsman:Locale element is processed in the initial message only.
    // It should be ignored in subsequent messages because the first
    // message establishes the required locale. The service may issue a
    // fault if the wsman:Locale is present in subsequent messages and the
    // value is different from that used in the initiating request.

    PEG_METHOD_EXIT();
}

void WsmProcessor::handleResponse(CIMResponseMessage* cimResponse)
{
    PEG_METHOD_ENTER(TRC_WSMSERVER, "WsmProcessor::handleResponse()");

    AutoPtr<CIMResponseMessage> cimResponseDestroyer(cimResponse);

    // Lookup the request this response corresponds to
    WsmRequest* wsmRequest = 0;

    PEGASUS_FCT_EXECUTE_AND_ASSERT(
        true,
        _requestTable.lookup(cimResponse->messageId, wsmRequest));

    AutoPtr<WsmRequest> wsmRequestDestroyer(wsmRequest);
    _requestTable.remove(cimResponse->messageId);

    try
    {
        switch (wsmRequest->getType())
        {
            case WS_ENUMERATION_ENUMERATE:
                _handleEnumerateResponse(
                    cimResponse,
                    (WsenEnumerateRequest*) wsmRequest);
                break;
            case WS_SUBSCRIPTION_CREATE:
                _handleSubscriptionResponse(
                    cimResponse,
                    (WxfSubCreateRequest*) wsmRequest);
                break;
            case WS_SUBSCRIPTION_DELETE:
                _handleSubscriptionDeleteResponse(
                    cimResponse,
                    (WxfSubDeleteRequest*) wsmRequest);
                break;
            default:
                _handleDefaultResponse(cimResponse, wsmRequest);
                break;
        }
    }
    catch (WsmFault& fault)
    {
        sendResponse(new WsmFaultResponse(wsmRequest, fault),wsmRequest);
    }
    catch (CIMException& e)
    {
        sendResponse(
            new WsmFaultResponse(
                wsmRequest,
                _cimToWsmResponseMapper.mapCimExceptionToWsmFault(e)),
            wsmRequest);
    }
    catch (Exception& e)
    {
        sendResponse(
            new WsmFaultResponse(
                wsmRequest,
                WsmFault(
                    WsmFault::wsman_InternalError,
                    e.getMessage(),
                    e.getContentLanguages())),
            wsmRequest);
    }
    catch (PEGASUS_STD(exception)& e)
    {
        sendResponse(
            new WsmFaultResponse(
                wsmRequest,
                WsmFault(WsmFault::wsman_InternalError, e.what())),
            wsmRequest);
    }
    catch (...)
    {
        sendResponse(
            new WsmFaultResponse(
                wsmRequest,
                WsmFault(WsmFault::wsman_InternalError)),
            wsmRequest);
    }

    PEG_METHOD_EXIT();
}

void WsmProcessor::sendResponse(
    WsmResponse* wsmResponse,
    WsmRequest* wsmReq)
{
    PEG_METHOD_ENTER(TRC_WSMSERVER, "WsmProcessor::sendResponse()");
    if( wsmReq && wsmReq->getType() == WS_SUBSCRIPTION_CREATE)
    {
        SubscriptionContext *subContext = NULL;
        AutoMutex lock(_subscriptionContextTableLock);
        _subscriptionContextTable.lookupReference(
            wsmReq->messageId, subContext);
        if(wsmReq->getType() == WS_SUBSCRIPTION_CREATE)
        {
            Boolean createFilter=false;
            if(subContext && (subContext->filterReq != NULL))
            {
                createFilter = true;
            }
            WxfSubCreateRequest *req = (WxfSubCreateRequest *)wsmReq;
            if (req->instance.getClassName() ==
                PEGASUS_CLASSNAME_INDHANDLER_WSMAN.getString())
            {
                _cleanupFilterHandlerInstances(
                    req->messageId,
                    true,
                    false);
                _cleanupSubContext(wsmReq->messageId,
                    createFilter,
                    false,
                    true);

            }
            else if (req->instance.getClassName() ==
                PEGASUS_CLASSNAME_INDSUBSCRIPTION.getString())
            {
                _cleanupFilterHandlerInstances(
                    req->messageId,
                    true,
                    true);
                //Delete the subContext.
                _cleanupSubContext(wsmReq->messageId);
            }
            else if (req->instance.getClassName() ==
                PEGASUS_CLASSNAME_INDFILTER.getString())
            {
                _cleanupSubContext(wsmReq->messageId,
                    false,
                    false,
                    true);
            }
        }
        else if (wsmReq->getType() == WS_SUBSCRIPTION_DELETE)
        {
            Boolean deleteFilter = false;
            if(subContext && (subContext->filterDeleteReq != NULL))
            {
                deleteFilter = true;
            }
            WxfSubDeleteRequest *deleteReq = (WxfSubDeleteRequest *)wsmReq;
            if(deleteReq->className ==
                PEGASUS_CLASSNAME_INDSUBSCRIPTION.getString())
            {
                _cleanupSubContext(wsmReq->messageId,
                    false,
                    false,
                    false,
                    deleteFilter,
                    true);
            }
        }
    }
    _wsmResponseEncoder.enqueue(wsmResponse);

    delete wsmResponse;

    PEG_METHOD_EXIT();
}

Uint32 WsmProcessor::getWsmRequestDecoderQueueId()
{
    return _wsmRequestDecoder.getQueueId();
}

void WsmProcessor::_handleEnumerateResponse(
    CIMResponseMessage* cimResponse,
    WsenEnumerateRequest* wsmRequest)
{
    if (cimResponse->cimException.getCode() != CIM_ERR_SUCCESS)
    {
        _handleDefaultResponse(cimResponse, wsmRequest);
        return;
    }

    AutoPtr<SoapResponse> soapResponse;

    {
        AutoMutex lock(_WsmEnumerationContextTableLock);

        AutoPtr<WsenEnumerateResponse> wsmResponse(
            (WsenEnumerateResponse*) _cimToWsmResponseMapper.
                mapToWsmResponse(wsmRequest, cimResponse));

        // Get the enumeration expiration time
        CIMDateTime expiration;
        _getExpirationDatetime(wsmRequest->expiration, expiration);

        // Create a new context
        Uint64 contextId = _currentEnumContext++;
        _WsmEnumerationContextTable.insert(
            contextId,
            WsmEnumerationContext(
                contextId,
                wsmRequest->userName,
                wsmRequest->enumerationMode,
                expiration,
                wsmRequest->epr,
                wsmResponse.get()));
        wsmResponse->setEnumerationContext(contextId);

        // Get the requsted chunk of results
        AutoPtr<WsenEnumerateResponse> splitResponse(
            _splitEnumerateResponse(wsmRequest, wsmResponse.get(),
                wsmRequest->optimized ? wsmRequest->maxElements : 0));
        splitResponse->setEnumerationContext(contextId);

        // If no items are left in the original response, mark split
        // response as complete
        if (wsmResponse->getSize() == 0)
        {
            splitResponse->setComplete();
        }

        Uint32 numDataItemsEncoded = 0;
        soapResponse.reset(_wsmResponseEncoder.encodeWsenEnumerateResponse(
            splitResponse.get(), numDataItemsEncoded));

        if (splitResponse->getSize() > numDataItemsEncoded)
        {
            // Add unprocessed items back to the context
            splitResponse->remove(0, numDataItemsEncoded);
            wsmResponse->merge(splitResponse.get());
        }

        // Remove the context if there are no instances left
        if (wsmResponse->getSize() == 0)
        {
            _WsmEnumerationContextTable.remove(contextId);
        }
        else
        {
            // If the context is not removed, the pointer to the response is
            // now owned by the context
            wsmResponse.release();
        }
    }

    _wsmResponseEncoder.sendResponse(soapResponse.get());
}

void WsmProcessor::_handleSubscriptionResponse(
    CIMResponseMessage* cimResponse,
    WxfSubCreateRequest* wsmRequest)
{
    PEG_METHOD_ENTER(TRC_WSMSERVER,
        "WsmProcessor::_handleSubscriptionResponse()");
    SubscriptionContext *subContext = NULL;
    AutoMutex lock(_subscriptionContextTableLock);
    _subscriptionContextTable.lookupReference(
        wsmRequest->messageId, subContext);
    if(subContext)
    {
        Boolean createFilter = false;
        if (subContext->filterReq != NULL)
            createFilter = true;
        String className = wsmRequest->instance.getClassName();
        if (className == PEGASUS_CLASSNAME_INDHANDLER_WSMAN.getString())
        {
            // If handler response is successful
            if(cimResponse->cimException.getCode() == CIM_ERR_SUCCESS)
            {
                subContext->handlerResponse = true;
                // Proccess filter request if it exists
                if (createFilter)
                    handleRequest(subContext->filterReq);
                // Process Subscription request if filter request is NULL,
                // which means subscription uses existing filter
                else if (subContext->subReq != NULL)
                {
                    subContext->filterResponse = true;
                    handleRequest(subContext->subReq);
                }
            }
            else // If unsuccessful, do the cleanup
            {
                PEG_TRACE((TRC_WSMSERVER, Tracer::LEVEL2,
                    "Handler creation failed for the request with ID %s ",
                    (const char*)wsmRequest->messageId.getCString()));
                _subscriptionContextTableLock.unlock();
                sendResponse(new WsmFaultResponse(
                    wsmRequest,
                    _cimToWsmResponseMapper.mapCimExceptionToWsmFault(
                    cimResponse->cimException)));
                _subscriptionContextTableLock.lock();
                _cleanupSubContext(wsmRequest->messageId,
                    createFilter,
                    false,
                    true);
            }
        }
        else if (className == PEGASUS_CLASSNAME_INDFILTER.getString())
        {
            subContext->filterResponse = true;
            // If filter response is successful, process the handler
            if(cimResponse->cimException.getCode() == CIM_ERR_SUCCESS)
            {
                if (subContext->subReq != NULL)
                    handleRequest(subContext->subReq);
            }
            else // If unsuccessful, do the cleanup
            {
                PEG_TRACE((TRC_WSMSERVER, Tracer::LEVEL2,
                    "Filter creation failed for the request with ID %s ",
                    (const char*)wsmRequest->messageId.getCString()));
                // Cleanup handler
                _cleanupFilterHandlerInstances(
                    wsmRequest->messageId,
                    false,
                    true);
                _subscriptionContextTableLock.unlock();
                sendResponse(new WsmFaultResponse(
                    wsmRequest,
                    _cimToWsmResponseMapper.mapCimExceptionToWsmFault(
                    cimResponse->cimException)));
                _subscriptionContextTableLock.lock();
                _cleanupSubContext(wsmRequest->messageId,
                    false,
                    false,
                    true);

            }
        }
        else if (className == PEGASUS_CLASSNAME_INDSUBSCRIPTION.getString())
        {
            if ((cimResponse->cimException.getCode() == CIM_ERR_SUCCESS)&&
               (subContext->filterResponse == true) &&
               (subContext->handlerResponse == true))
            {
                _fillSubscriptionInfoTable(subContext->subReq);
                _subscriptionContextTableLock.unlock();
                // Subscription has been created successfully
                // Send Subscription response
                AutoPtr<WsmResponse> wsmResponse(
                    _cimToWsmResponseMapper.mapToWsmResponse(
                        wsmRequest,
                        cimResponse));
                cimResponse->updateThreadLanguages();
                cimResponse->queueIds.pop();
                _wsmResponseEncoder.enqueue(wsmResponse.get());
            }
            else
            {
                // Subscription creation failed, cleanup the filter and handler
                PEG_TRACE((TRC_WSMSERVER, Tracer::LEVEL2,
                    "Subscription creation failed for the request with ID %s ",
                    (const char*)wsmRequest->messageId.getCString()));
                _cleanupFilterHandlerInstances(
                    wsmRequest->messageId,
                    createFilter,
                    true);
                _subscriptionContextTableLock.unlock();
                sendResponse(new WsmFaultResponse(
                    wsmRequest,
                    _cimToWsmResponseMapper.mapCimExceptionToWsmFault(
                    cimResponse->cimException)));
            }
            _subscriptionContextTableLock.lock();
            //Delete the subContext
            _cleanupSubContext(wsmRequest->messageId);
        }
    }
    // Context entry should usually be found, in case it is not found
    // log it in the trace
    else
    {
        PEG_TRACE((TRC_WSMSERVER, Tracer::LEVEL2,
                   "Subscription context entry for %s not found.",
                   (const char*)wsmRequest->messageId.getCString()));
    }
    PEG_METHOD_EXIT();
}

void WsmProcessor::_handleSubscriptionDeleteResponse(
    CIMResponseMessage* cimResponse,
    WxfSubDeleteRequest* wsmRequest)
{
    PEG_METHOD_ENTER(TRC_WSMSERVER,
        "WsmProcessor::_handleSubscriptionDeleteResponse()");
    String className = wsmRequest->className;
    if(className == PEGASUS_CLASSNAME_INDSUBSCRIPTION.getString())
    {
        SubscriptionContext *subContext = NULL;
        AutoMutex lock(_subscriptionContextTableLock);
        _subscriptionContextTable.lookupReference(
            wsmRequest->messageId,
            subContext);
        Boolean deleteFilter = false;
        if(subContext && (subContext->filterDeleteReq != NULL))
        {
            deleteFilter = true;
        }
        if(cimResponse->cimException.getCode() == CIM_ERR_SUCCESS)
        {
            //delete the filter and handler.
            if(deleteFilter)
            {
                    handleRequest(subContext->filterDeleteReq);
            }
            if(subContext)
            {
                handleRequest(subContext->handlerDeleteReq);
            }
            //delete the entry from _subscriptionInfoTable if the
            //subscription is created with the existing filter.
            for(Uint32 i=0;
                i<wsmRequest->epr.selectorSet->selectors.size();i++)
            {
                if(wsmRequest->epr.selectorSet->selectors[i].name ==
                    PEGASUS_PROPERTYNAME_HANDLER.getString())
                {
                    WsmEndpointReference handlerEpr =
                        wsmRequest->epr.selectorSet->selectors[i].epr;
                    for(Uint32 j=0;
                        j<handlerEpr.selectorSet->selectors.size();j++)
                    {
                        String handlerName;
                        if(handlerEpr.selectorSet->selectors[j].name ==
                            PEGASUS_PROPERTYNAME_NAME.getString())
                        {
                            handlerName =
                                handlerEpr.selectorSet->selectors[j].value;
                            AutoMutex lock(_subscriptionInfoTableLock);
                            if(_subscriptionInfoTable.contains(handlerName))
                            {
                                _subscriptionInfoTable.remove(handlerName);
                            }
                            break;
                        }

                    }
                    break;
                }
            }
            //Delete the context
            _cleanupSubContext(wsmRequest->messageId);
        }
        else
        {
            _cleanupSubContext(wsmRequest->messageId,
                false,
                false,
                false,
                deleteFilter,
                true);
        }
        _subscriptionContextTableLock.unlock();
        AutoPtr<WsmResponse> wsmResponse(
            _cimToWsmResponseMapper.mapToWsmResponse(
                wsmRequest,
                cimResponse));
        cimResponse->updateThreadLanguages();
        cimResponse->queueIds.pop();

        _wsmResponseEncoder.enqueue(wsmResponse.get());
        _subscriptionContextTableLock.lock();
    }
    // Context entry should usually be found, in case it is not found
    // log it in the trace
    else
    {
        PEG_TRACE((TRC_WSMSERVER, Tracer::LEVEL2,
            "Subscription context entry for %s not found.",
            (const char*)wsmRequest->messageId.getCString()));
    }
    PEG_METHOD_EXIT();
    return;
}

void WsmProcessor::_handlePullRequest(WsenPullRequest* wsmRequest)
{
    AutoPtr<SoapResponse> soapResponse;

    {
        AutoMutex lock(_WsmEnumerationContextTableLock);
        WsmEnumerationContext* enumContext;

        if (_WsmEnumerationContextTable.lookupReference(
                wsmRequest->enumerationContext, enumContext))
        {
            // EPRs of the request and the enumeration context must match
            if (wsmRequest->epr != enumContext->epr)
            {
                throw WsmFault(
                    WsmFault::wsa_MessageInformationHeaderRequired,
                    MessageLoaderParms(
                        "WsmServer.WsmProcessor.INVALID_PULL_EPR",
                        "EPR of a Pull request does not match that of "
                        "the enumeration context."));
            }

            // User credentials of the request and the enumeration context must
            // match.
            if (wsmRequest->userName != enumContext->userName)
            {
                // DSP0226 R8.1-6:  The wsen:Pull and wsen:Release operations
                // are a continuation of the original wsen:Enumerate operation.
                // The service should enforce the same authentication and
                // authorization throughout the entire sequence of operations
                // and should fault any attempt to change credentials during
                // the sequence.

                throw WsmFault(WsmFault::wsman_AccessDenied);
            }

            AutoPtr<WsenPullResponse> wsmResponse(_splitPullResponse(
                wsmRequest, enumContext->response, wsmRequest->maxElements));
            wsmResponse->setEnumerationContext(enumContext->contextId);
            if (enumContext->response->getSize() == 0)
            {
                wsmResponse->setComplete();
            }

            Uint32 numDataItemsEncoded = 0;
            soapResponse.reset(_wsmResponseEncoder.encodeWsenPullResponse(
                wsmResponse.get(), numDataItemsEncoded));

            if (wsmResponse->getSize() > numDataItemsEncoded)
            {
                // Add unprocessed items back to the context
                wsmResponse->remove(0, numDataItemsEncoded);
                enumContext->response->merge(wsmResponse.get());
            }

            // Remove the context if there are no instances left
            if (enumContext->response->getSize() == 0)
            {
                delete enumContext->response;
                _WsmEnumerationContextTable.remove(
                    wsmRequest->enumerationContext);
            }
        }
        else
        {
            throw WsmFault(
                WsmFault::wsen_InvalidEnumerationContext,
                MessageLoaderParms(
                    "WsmServer.WsmProcessor.INVALID_ENUMERATION_CONTEXT",
                    "Enumeration context \"$0\" is not valid.",
                    wsmRequest->enumerationContext));
        }
    }

    _wsmResponseEncoder.sendResponse(soapResponse.get());
}

void WsmProcessor::_handleReleaseRequest(WsenReleaseRequest* wsmRequest)
{
    AutoPtr<WsenReleaseResponse> wsmResponse;

    {
        AutoMutex lock(_WsmEnumerationContextTableLock);

        WsmEnumerationContext enumContext;
        if (_WsmEnumerationContextTable.lookup(
                wsmRequest->enumerationContext, enumContext))
        {
            // EPRs of the request and the enumeration context must match
            if (wsmRequest->epr != enumContext.epr)
            {
                throw WsmFault(
                    WsmFault::wsa_MessageInformationHeaderRequired,
                    MessageLoaderParms(
                        "WsmServer.WsmProcessor.INVALID_RELEASE_EPR",
                        "EPR of a Release request does not match that of "
                        "the enumeration context."));
            }

            // User credentials of the request and the enumeration context must
            // match.
            if (wsmRequest->userName != enumContext.userName)
            {
                // DSP0226 R8.1-6:  The wsen:Pull and wsen:Release operations
                // are a continuation of the original wsen:Enumerate operation.
                // The service should enforce the same authentication and
                // authorization throughout the entire sequence of operations
                // and should fault any attempt to change credentials during
                // the sequence.

                throw WsmFault(WsmFault::wsman_AccessDenied);
            }

            wsmResponse.reset(new WsenReleaseResponse(
                wsmRequest, enumContext.response->getContentLanguages()));

            delete enumContext.response;
            _WsmEnumerationContextTable.remove(wsmRequest->enumerationContext);
        }
        else
        {
            throw WsmFault(
                WsmFault::wsen_InvalidEnumerationContext,
                MessageLoaderParms(
                    "WsmServer.WsmProcessor.INVALID_ENUMERATION_CONTEXT",
                    "Enumeration context \"$0\" is not valid.",
                    wsmRequest->enumerationContext));
        }
    }

    _wsmResponseEncoder.enqueue(wsmResponse.get());
}

void WsmProcessor::_handleDefaultResponse(
    CIMResponseMessage* cimResponse, WsmRequest* wsmRequest)
{
    AutoPtr<WsmResponse> wsmResponse(
        _cimToWsmResponseMapper.mapToWsmResponse(wsmRequest, cimResponse));

    cimResponse->updateThreadLanguages();
    cimResponse->queueIds.pop();

    _wsmResponseEncoder.enqueue(wsmResponse.get());
}

WsenEnumerateResponse* WsmProcessor::_splitEnumerateResponse(
    WsenEnumerateRequest* request, WsenEnumerateResponse* response, Uint32 num)
{
    WsenEnumerationData splitData;
    response->getEnumerationData().split(splitData, num);

    return new WsenEnumerateResponse(splitData, response->getItemCount(),
        request, response->getContentLanguages());
}

WsenPullResponse* WsmProcessor::_splitPullResponse(
    WsenPullRequest* request, WsenEnumerateResponse* response, Uint32 num)
{
    WsenEnumerationData splitData;
    response->getEnumerationData().split(splitData, num);

    return new WsenPullResponse(splitData, request,
        response->getContentLanguages());
}

void WsmProcessor::_getExpirationDatetime(
    const String& wsmDT, CIMDateTime& cimDT)
{
    CIMDateTime dt, currentDT;

    // Default expiration interval = 10 mins
    // ATTN WSMAN: what should the value be?
    CIMDateTime maxInterval(0, 0, 10, 0, 0, 6);

    // If expiration is not set, use the dafault.
    if (wsmDT == String::EMPTY)
    {
        dt = maxInterval;
    }
    else
    {
        try
        {
            WsmToCimRequestMapper::convertWsmToCimDatetime(wsmDT, dt);
        }
        catch (...)
        {
            throw WsmFault(
                WsmFault::wsen_InvalidExpirationTime,
            MessageLoaderParms(
                "WsmServer.WsmToCimRequestMapper.INVALID_EXPIRATION_TIME",
                "The expiration time \"$0\" is not valid", wsmDT));
        }
    }

    currentDT = CIMDateTime::getCurrentDateTime();
    if (dt.isInterval())
    {
        if (dt > maxInterval)
        {
            dt = maxInterval;
        }
        cimDT = currentDT + dt;
    }
    else
    {
        if ((dt <= currentDT))
        {
            throw WsmFault(
                WsmFault::wsen_InvalidExpirationTime,
            MessageLoaderParms(
                "WsmServer.WsmToCimRequestMapper.INVALID_EXPIRATION_TIME",
                "The expiration time \"$0\" is not valid", wsmDT));
        }

        if (dt - currentDT > maxInterval)
        {
            cimDT = currentDT + maxInterval;
        }
        else
        {
            cimDT = dt;
        }
    }
}

void WsmProcessor::cleanupExpiredContexts()
{
    CIMDateTime currentDT = CIMDateTime::getCurrentDateTime();
    Array<Uint64> expiredContextIds;
    Array<WsenEnumerateResponse*> expiredResponses;

    AutoMutex lock(_WsmEnumerationContextTableLock);
    for (WsmEnumerationContextTable::Iterator i =
             _WsmEnumerationContextTable.start (); i; i++)
    {
        WsmEnumerationContext context = i.value();
        if (context.expiration < currentDT)
        {
            expiredContextIds.append(context.contextId);
            expiredResponses.append(context.response);
        }
    }

    for (Uint32 i = 0; i < expiredContextIds.size(); i++)
    {
        delete expiredResponses[i];
        _WsmEnumerationContextTable.remove(expiredContextIds[i]);
    }
}

void WsmProcessor::_cleanupFilterHandlerInstances(
    String messageId,
    Boolean isfilterCleaup,
    Boolean isHandlerCleanup)
{
    PEG_METHOD_ENTER(TRC_WSMSERVER,
        "WsmProcessor::_cleanupFilterHandlerInstances()");
    SubscriptionContext *subContext = NULL;
    _subscriptionContextTable.lookupReference(
        messageId,
        subContext);
    AutoPtr<WxfSubDeleteRequest> deleteFilter;
    if(subContext)
    {
        String msgId = messageId.subString(PEGASUS_WS_UUID_LENGTH);
        if(isfilterCleaup)
        {
            //form a deleteInstance Request for filter.
            WsmEndpointReference filterEPR;
            _wsmRequestDecoder.getFilterOrHandlerEPR(filterEPR,
                subContext->subReq->epr.address,
                msgId,
                PEGASUS_CLASSNAME_INDFILTER.getString());
            AutoPtr<WxfSubDeleteRequest> deleteFilter(new WxfSubDeleteRequest(
                messageId,
                filterEPR,
                PEGASUS_CLASSNAME_INDFILTER.getString()));
            handleRequest(deleteFilter.release());
        }
        if(isHandlerCleanup)
        {
            //Form a deleteInstance request for handler.
            WsmEndpointReference handlerEPR;
            _wsmRequestDecoder.getFilterOrHandlerEPR(handlerEPR,
                subContext->subReq->epr.address,
                msgId,
                PEGASUS_CLASSNAME_INDHANDLER_WSMAN.getString());
            AutoPtr<WxfSubDeleteRequest>deleteHandler(new WxfSubDeleteRequest(
                messageId,
                handlerEPR,
                PEGASUS_CLASSNAME_INDHANDLER_WSMAN.getString()));
            handleRequest(deleteHandler.release());
        }
    }
    else
    {
        PEG_TRACE((TRC_WSMSERVER, Tracer::LEVEL2,
                   "Subscription context entry for %s not found.",
                   (const char*)messageId.getCString()));
    }
    PEG_METHOD_EXIT();
}

void WsmProcessor::_cleanupSubContext(String & messageId,
    Boolean isFilterCreate,
    Boolean isHanlderCreate,
    Boolean isSubCreate,
    Boolean isFilterDelete,
    Boolean isHandlerDelete)
{
    SubscriptionContext *subConTxt = 0;
    _subscriptionContextTable.lookupReference(messageId,subConTxt);
    if(subConTxt)
    {
        if(subConTxt->filterReq && isFilterCreate)
            delete subConTxt->filterReq;
        if(subConTxt->handlerReq && isHanlderCreate)
            delete subConTxt->handlerReq;
        if(subConTxt->subReq && isSubCreate)
            delete subConTxt->subReq;
        if(subConTxt->filterDeleteReq && isFilterDelete)
            delete subConTxt->filterDeleteReq;
        if(subConTxt->handlerDeleteReq && isHandlerDelete)
            delete subConTxt->handlerDeleteReq;

        _subscriptionContextTable.remove(messageId);
    }
}

void WsmProcessor::_fillSubscriptionInfoTable(WxfSubCreateRequest * subReq)
{

    // Filter name is message id without "uuid:"
    String msgId = subReq->messageId.subString(PEGASUS_WS_UUID_LENGTH);
    AutoMutex lock(_subscriptionInfoTableLock);
    if(!_subscriptionInfoTable.contains(msgId))
    {
        String filterName;
        WsmEndpointReference filterEPR;
        for(Uint32 i=0;i<subReq->instance.getPropertyCount();i++)
        {
            if((subReq->instance.getProperty(i)).getName() ==
                PEGASUS_PROPERTYNAME_FILTER.getString())
            {
                WsmValue filterPropVal =
                    subReq->instance.getProperty(i).getValue();
                filterPropVal.get(filterEPR);
                break;
            }
        }

        for(Uint32 i=0;i<filterEPR.selectorSet->selectors.size();i++)
        {
            if(filterEPR.selectorSet->selectors[i].name ==
                PEGASUS_PROPERTYNAME_NAME.getString())
            {
                filterName = filterEPR.selectorSet->selectors[i].value;
                break;
            }
        }
        // If msgId is not equal to filterName, then it is using an
        // existing filter.
        if( msgId != filterName)
            _subscriptionInfoTable.insert(msgId ,filterName);
    }
}

Boolean WsmProcessor::isSubCreatedWithExistingFilter(
    const String & subId,
    String & filterName)
{
    AutoMutex lock(_subscriptionInfoTableLock);
    if(_subscriptionInfoTable.lookup(subId,filterName))
    {
        return true;
    }
    else
    {
        return false;
    }
}

void WsmProcessor::_initializeSubInfoTable()
{
    if (_repository->nameSpaceExists(PEGASUS_NAMESPACENAME_INTEROP))
    {
        Array <CIMInstance> subscriptions;
        subscriptions = _repository->enumerateInstancesForClass(
            CIMNamespaceName(PEGASUS_NAMESPACENAME_INTEROP.getString()),
            PEGASUS_CLASSNAME_INDSUBSCRIPTION);
        for (Uint32 i = 0; i < subscriptions.size(); i++)
        {
            CIMObjectPath filterPath;
            String subscriptionInfo;
            Boolean filterNameUpdated = false;
            Boolean subInfoUpdated = false;
            String filterName;
            Boolean isWsmanSub = false;
            for(Uint32 j=0; j<subscriptions[i].getPropertyCount(); j++)
            {
                CIMProperty prop = subscriptions[i].getProperty(j);
                if(prop.getName()== PEGASUS_PROPERTYNAME_HANDLER.getString())
                {
                    CIMObjectPath handlePath;
                    prop.getValue().get(handlePath);
                    if(handlePath.getClassName() ==
                        PEGASUS_CLASSNAME_INDHANDLER_WSMAN.getString())
                    {
                        isWsmanSub = true;
                    }

                }
                else if(prop.getName() == PEGASUS_PROPERTYNAME_FILTER)
                {
                    prop.getValue().get(filterPath);
                    Array<CIMKeyBinding> filterKeyProp =
                        filterPath.getKeyBindings();
                    for(Uint32 k=0 ;k<filterKeyProp.size();k++)
                    {
                        if(filterKeyProp[k].getName().getString() ==
                            PEGASUS_PROPERTYNAME_NAME.getString())
                        {
                            filterName = filterKeyProp[k].getValue();
                            filterNameUpdated = true;
                        }
                    }
                }
                else if(prop.getName().getString() == "SubscriptionInfo")
                {
                    prop.getValue().get(subscriptionInfo);
                    subInfoUpdated = true;
                }
            }
            if(isWsmanSub && filterNameUpdated && subInfoUpdated &&
                (filterName != subscriptionInfo))
            {
                AutoMutex lock(_subscriptionInfoTableLock);
                _subscriptionInfoTable.insert(subscriptionInfo, filterName);
            }
        }
    }
}


PEGASUS_NAMESPACE_END
