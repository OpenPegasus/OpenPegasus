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

#ifndef Pegasus_ProviderManagerService_h
#define Pegasus_ProviderManagerService_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/MessageQueueService.h>
#include <Pegasus/Common/CIMMessage.h>
#include <Pegasus/Common/OperationContextInternal.h>
#include <Pegasus/Common/AutoPtr.h>
#include <Pegasus/Common/List.h>
#include <Pegasus/Common/Mutex.h>
#include <Pegasus/Repository/CIMRepository.h>
#include \
    <Pegasus/Server/ProviderRegistrationManager/ProviderRegistrationManager.h>
#include <Pegasus/ProviderManager2/ProviderManager.h>
#include <Pegasus/ProviderManagerRouter/ProviderManagerRouter.h>
#include <Pegasus/ProviderManagerService/Linkage.h>

PEGASUS_NAMESPACE_BEGIN

class PEGASUS_PMS_LINKAGE ProviderManagerService : public MessageQueueService
{
public:
    ProviderManagerService(
        ProviderRegistrationManager* providerRegistrationManager,
        CIMRepository* repository,
        ProviderManager* (*createDefaultProviderManagerCallback)());

    virtual ~ProviderManagerService();

    void idleTimeCleanup();

    bool enumerationContextCleanup(const String& contextId);

    static void indicationCallback(CIMProcessIndicationRequestMessage* request);
    static void responseChunkCallback(
        CIMRequestMessage* request, CIMResponseMessage* response);
    static void asyncResponseCallback(
        CIMRequestMessage* request, CIMResponseMessage* response);

    /**
        Callback function to be called upon detection of failure of a
        provider module.
     */
    static void providerModuleGroupFailureCallback(
        const String &groupName,
        const String & userName,
        Uint16 userContext,
        Boolean isGroup);

private:
    ProviderManagerService();

    virtual void handleEnqueue();
    virtual void handleEnqueue(Message* message);

    virtual void _handle_async_request(AsyncRequest* request);

    static ThreadReturnType PEGASUS_THREAD_CDECL handleCimOperation(
        void* arg) ;

    void handleCimRequest(AsyncOpNode* op, Message* message);

    Message* _processMessage(CIMRequestMessage* request);

    static ThreadReturnType PEGASUS_THREAD_CDECL
        _idleTimeCleanupHandler(void* arg) throw();

    void _updateModuleStatusToEnabled(
        CIMEnableModuleResponseMessage *emResp,
        CIMInstance &providerModule);

    void _updateModuleStatusToDisabled(
        CIMDisableModuleResponseMessage *dmResp,
        CIMInstance &providerModule);

    void _updateProviderModuleStatus(
        CIMInstance& providerModule,
        const Array<Uint16>& removeStatus,
        const Array<Uint16>& appendStatus);

    static void _invokeProviderModuleStartMethod(
        const CIMObjectPath &ref);

    static void _reconcileProviderModuleFailure(
        const String &moduleName,
        const String & userName,
        Uint16 userContext);

    void _handleIndicationDeliveryResponse(Message *message);
    static void _indicationDeliveryRoutine(
        CIMProcessIndicationRequestMessage* request);

    static ProviderManagerService* providerManagerService;

    CIMRepository* _repository;

    List<AsyncOpNode,Mutex> _incomingQueue;
    List<AsyncOpNode,Mutex> _outgoingQueue;

    Boolean _forceProviderProcesses;
    ProviderManagerRouter* _basicProviderManagerRouter;
    ProviderManagerRouter* _oopProviderManagerRouter;

    static ProviderRegistrationManager* _providerRegistrationManager;


    static Boolean _allProvidersStopped;
    static Uint32 _indicationServiceQueueId;

    /**
        Indicates the number of threads currently attempting to cleanup idle
        providers and clean disconnected client requests.  This value is used
        to prevent multiple threads from initiating cleanup the same time.
     */
    AtomicInt _idleTimeCleanupBusy;
};

PEGASUS_NAMESPACE_END

#endif
