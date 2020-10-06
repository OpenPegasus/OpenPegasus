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

#ifndef Pegasus_WsmProcessor_h
#define Pegasus_WsmProcessor_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/MessageQueue.h>
#include <Pegasus/Common/CIMMessage.h>
#include <Pegasus/Repository/CIMRepository.h>
#include <Pegasus/WsmServer/WsmRequestDecoder.h>
#include <Pegasus/WsmServer/WsmResponseEncoder.h>
#include <Pegasus/WsmServer/WsmToCimRequestMapper.h>
#include <Pegasus/WsmServer/CimToWsmResponseMapper.h>
#include <Pegasus/WsmServer/Linkage.h>

PEGASUS_NAMESPACE_BEGIN

class WsmEnumerationContext
{
public:
    WsmEnumerationContext() {}
    WsmEnumerationContext(
        Uint64 contextId_,
        const String& userName_,
        WsenEnumerationMode enumerationMode_,
        CIMDateTime expiration_,
        WsmEndpointReference epr_,
        WsenEnumerateResponse* response_)
        : contextId(contextId_),
          userName(userName_),
          enumerationMode(enumerationMode_),
          expiration(expiration_),
          epr(epr_),
          response(response_) {}

    Uint64 contextId;
    String userName;
    WsenEnumerationMode enumerationMode;
    CIMDateTime expiration;
    WsmEndpointReference epr;
    WsenEnumerateResponse* response;
};

/**
    SubscriptionContextTable is a hash table containing entries of
    SubscriptionContext.The contxt ID is the subscription message ID.
    This context is used to process create and delete instance requests
    for subscription, filter and handler serially.An entry is made while
    proceesing Subscribe or Unsubscribe request via the addReqToSubContext()
    by WsmRequestDecoder. Once all the requests are complete,
    the entry is deleted.

    For subscribe request, all the three create instance requests for filter,
    handler and subscription are filled. Only filter request is first processed
    and once the filter response is recieved, the handler request is processed.
    Once handler response is recieved,only then is the subscription request
    processed. After subscription response is recieved,the context entry is
    deleted and the subscribe response is sent back to the client.If any of
    the requests get exeception then the cleanup function
    _cleanupFilterHandlerInstances() is called appropriatley.

    For unsubscribe the subscription delete instance request is processed
    first. Once the response is recieved, then the delete instance requests
    for filter and handler are processed.
*/
class SubscriptionContext
{
public:
    SubscriptionContext() {}
    SubscriptionContext(String contextId_)
    {
        contextId = contextId_;
        filterResponse= false;
        handlerResponse= false;
        filterReq = NULL;
        handlerReq = NULL;
        subReq = NULL;
        filterDeleteReq = NULL;
        handlerDeleteReq = NULL;
    }
    String contextId;
    Boolean filterResponse;
    Boolean handlerResponse;
    WxfSubCreateRequest* filterReq;
    WxfSubCreateRequest* handlerReq;
    WxfSubCreateRequest* subReq;
    WxfSubDeleteRequest* filterDeleteReq;
    WxfSubDeleteRequest* handlerDeleteReq;
};

PEGASUS_TEMPLATE_SPECIALIZATION struct HashFunc<Uint64>
{
    static Uint32 hash(Uint64 x) { return (Uint32) x + 13; }
};


/**
    Processes WsmRequest messages and produces WsmResponse messages.
*/
class PEGASUS_WSMSERVER_LINKAGE WsmProcessor : public MessageQueue
{
public:

    WsmProcessor(
        MessageQueue* cimOperationProcessorQueue,
        CIMRepository* repository);

    ~WsmProcessor();

    virtual void handleEnqueue(Message *);

    virtual void handleEnqueue();

    void handleRequest(WsmRequest* wsmRequest);
    void handleResponse(CIMResponseMessage* cimResponse);

    void sendResponse(
        WsmResponse* wsmResponse,
        WsmRequest* wsmReq = NULL);

    Uint32 getWsmRequestDecoderQueueId();

    void setServerTerminating(Boolean flag)
    {
        _wsmRequestDecoder.setServerTerminating(flag);
    }

    void cleanupExpiredContexts();

    void addReqToSubContext(WsmRequest *, Boolean isCreateReq);
    /**
        This function returns the information whether subscription is
        is created using existing filter or not.
    */
    Boolean isSubCreatedWithExistingFilter(
        const String & subId,
        String & filterName);

private:

    void _handlePullRequest(WsenPullRequest* wsmRequest);
    void _handleReleaseRequest(WsenReleaseRequest* wsmRequest);
    void _handleEnumerateResponse(
        CIMResponseMessage* cimResponse,
        WsenEnumerateRequest* wsmRequest);
    void _handleSubscriptionResponse(
        CIMResponseMessage* cimResponse,
        WxfSubCreateRequest* wsmRequest);
    void _handleSubscriptionDeleteResponse(
        CIMResponseMessage* cimResponse,
        WxfSubDeleteRequest* wsmRequest);
    void _handleDefaultResponse(
        CIMResponseMessage* cimResponse,
        WsmRequest* wsmRequest);
    WsenEnumerateResponse* _splitEnumerateResponse(
        WsenEnumerateRequest* request,
        WsenEnumerateResponse* response,
        Uint32 num);
    WsenPullResponse* _splitPullResponse(
        WsenPullRequest* request,
        WsenEnumerateResponse* response,
        Uint32 num);
    void _getExpirationDatetime(const String& wsmDT, CIMDateTime& cimDT);
    void _cleanupFilterHandlerInstances(
        String messageId,
        Boolean isfilterCleanup,
        Boolean isHandlerCleanup);

    /**
        This function is used to cleanup the filter, handler and
        subscription requests in failure scenarios. As we are processing the
        filter, handler and subscription requests sequentially, if filter
        request fails then the stored handler and subscription requests
        should be deleted from the subscription context manually.
    */
    void _cleanupSubContext(String & messageId,
        Boolean isFilterCreate=false,
        Boolean isHanlderCreate=false,
        Boolean isSubCreate=false,
        Boolean isFilterDelete=false,
        Boolean isHandlerDelete=false);
    /**
        This function is used to fill the SubscriptionInfoTable.
    */
    void _fillSubscriptionInfoTable(WxfSubCreateRequest * subReq);

    /**
        This function is used to initialize the info table during the
        cimserver startup.
    */
    void _initializeSubInfoTable();
    WsmResponseEncoder _wsmResponseEncoder;
    WsmRequestDecoder _wsmRequestDecoder;

    /**
        A pointer to a CIMOperationRequestDispatcher that can be used to
        process CIM operation requests.
    */
    MessageQueue* _cimOperationProcessorQueue;

    /**
        A repository object that can be used to look up schema definitions.
    */
    CIMRepository* _repository;

    WsmToCimRequestMapper _wsmToCimRequestMapper;
    CimToWsmResponseMapper _cimToWsmResponseMapper;

    typedef HashTable<String,
        WsmRequest*, EqualFunc<String>, HashFunc<String> > RequestTable;
    /**
        The RequestTable stores the original WS-Management request until the
        operation is complete.  It is used to create an appropriate
        WS-Management response from the CIM operation response message.
        A unique message ID is used for CIM operation messages, different from
        the WS-Management request message ID.  The CIM operation message ID
        is used as the hash key.
    */
    RequestTable _requestTable;

    typedef HashTable<Uint64, WsmEnumerationContext,
        EqualFunc<Uint64>, HashFunc<Uint64> > WsmEnumerationContextTable;

    WsmEnumerationContextTable _WsmEnumerationContextTable;
    Mutex _WsmEnumerationContextTableLock;
    static Uint64 _currentEnumContext;

    typedef HashTable<String, SubscriptionContext,
        EqualFunc<String>, HashFunc<String> > SubscriptionContextTable;
    SubscriptionContextTable _subscriptionContextTable;

    /**
        The SubscriptionInfoTable stores the subscriptions which are created
        using an existing filter. It'll be filled by _fillSubscriptionInfoTable
        function. It will be accessed by WsmRequestDecoder while processing
        an unsubscribe request to form the subscription EPR.
        Entry from this table will be deleted once unsubscribe request is
        processed successfully.
    */
    typedef HashTable<String,String,EqualFunc<String>,
        HashFunc<String> > SubscriptionInfoTable;
    SubscriptionInfoTable _subscriptionInfoTable;

    Mutex _subscriptionContextTableLock;
    Mutex _subscriptionInfoTableLock;
};

PEGASUS_NAMESPACE_END

#endif /* Pegasus_WsmProcessor_h */
