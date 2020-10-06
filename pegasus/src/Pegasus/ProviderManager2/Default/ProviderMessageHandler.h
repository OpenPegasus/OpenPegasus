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

#ifndef Pegasus_ProviderMessageHandler_h
#define Pegasus_ProviderMessageHandler_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/CIMMessage.h>
#include <Pegasus/Provider/CIMProvider.h>
#include <Pegasus/ProviderManager2/OperationResponseHandler.h>
#include <Pegasus/ProviderManager2/Default/ProviderStatus.h>
#include <Pegasus/ProviderManager2/Default/Linkage.h>

PEGASUS_NAMESPACE_BEGIN

class PEGASUS_DEFPM_LINKAGE ProviderMessageHandler
{
public:
    ProviderMessageHandler(
        const String& moduleName,
        const String& name,
        CIMProvider* provider,
        PEGASUS_INDICATION_CALLBACK_T indicationCallback,
        PEGASUS_RESPONSE_CHUNK_CALLBACK_T responseChunkCallback,
        Boolean subscriptionInitComplete);

    virtual ~ProviderMessageHandler();

    String getName() const;
    CIMProvider* getProvider();
    void setProvider(CIMProvider* provider);
    void initialize(CIMOMHandle& cimom);
    void terminate();
    void subscriptionInitComplete();
    void indicationServiceDisabled();

    CIMResponseMessage* processMessage(CIMRequestMessage* request);

private:
    OperationContext _createProviderOperationContext(
        const OperationContext& context);

    CIMResponseMessage* _handleGetInstanceRequest(
        CIMRequestMessage* message);
    CIMResponseMessage* _handleEnumerateInstancesRequest(
        CIMRequestMessage* message);
    CIMResponseMessage* _handleEnumerateInstanceNamesRequest(
        CIMRequestMessage* message);
    CIMResponseMessage* _handleCreateInstanceRequest(
        CIMRequestMessage* message);
    CIMResponseMessage* _handleModifyInstanceRequest(
        CIMRequestMessage* message);
    CIMResponseMessage* _handleDeleteInstanceRequest(
        CIMRequestMessage* message);

    CIMResponseMessage* _handleExecQueryRequest(
        CIMRequestMessage* message);

    CIMResponseMessage* _handleAssociatorsRequest(
        CIMRequestMessage* message);
    CIMResponseMessage* _handleAssociatorNamesRequest(
        CIMRequestMessage* message);
    CIMResponseMessage* _handleReferencesRequest(
        CIMRequestMessage* message);
    CIMResponseMessage* _handleReferenceNamesRequest(
        CIMRequestMessage* message);

    CIMResponseMessage* _handleGetPropertyRequest(
        CIMRequestMessage* message);
    CIMResponseMessage* _handleSetPropertyRequest(
        CIMRequestMessage* message);

    CIMResponseMessage* _handleInvokeMethodRequest(
        CIMRequestMessage* message);

    CIMResponseMessage* _handleCreateSubscriptionRequest(
        CIMRequestMessage* message);
    CIMResponseMessage* _handleModifySubscriptionRequest(
        CIMRequestMessage* message);
    CIMResponseMessage* _handleDeleteSubscriptionRequest(
        CIMRequestMessage* message);

    CIMResponseMessage* _handleExportIndicationRequest(
        CIMRequestMessage* message);

    /**
        Calls the provider's enableIndications() method.
        If successful, the indications response handler is stored in
        _indicationResponseHandler.

        Note that since an exception thrown by the provider's
        enableIndications() method is considered a provider error, any such
        exception is logged and not propagated by this method.
     */
    void _enableIndications();

    void _disableIndications();

    String _name;
    String _fullyQualifiedProviderName;
    CIMProvider* _provider;
    PEGASUS_INDICATION_CALLBACK_T _indicationCallback;
    PEGASUS_RESPONSE_CHUNK_CALLBACK_T _responseChunkCallback;
    Boolean _subscriptionInitComplete;
    EnableIndicationsResponseHandler* _indicationResponseHandler;

public:
    ProviderStatus status;
};


/**
    Encapsulates the calling of operationBegin() and operationEnd() for a
    ProviderMessageHandler to help ensure an accurate count of provider
    operations.
*/
class PEGASUS_DEFPM_LINKAGE ProviderOperationCounter
{
public:
    ProviderOperationCounter(ProviderMessageHandler* p)
        : _provider(p)
    {
        PEGASUS_ASSERT(_provider != 0);
        _provider->status.operationBegin();
    }

    ProviderOperationCounter(const ProviderOperationCounter& p)
        : _provider(p._provider)
    {
        PEGASUS_ASSERT(_provider != 0);
        _provider->status.operationBegin();
    }

    ~ProviderOperationCounter()
    {
        _provider->status.operationEnd();
    }

    ProviderMessageHandler& GetProvider()
    {
        return *_provider;
    }

private:
    ProviderOperationCounter();
    ProviderOperationCounter& operator=(const ProviderOperationCounter&);

    ProviderMessageHandler* _provider;
};

PEGASUS_NAMESPACE_END

#endif
