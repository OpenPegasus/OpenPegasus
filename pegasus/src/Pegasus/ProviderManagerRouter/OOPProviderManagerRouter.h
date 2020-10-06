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

#ifndef Pegasus_OOPProviderManagerRouter_h
#define Pegasus_OOPProviderManagerRouter_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/String.h>
#include <Pegasus/Common/HashTable.h>
#include <Pegasus/Common/CIMMessage.h>

#include <Pegasus/ProviderManager2/ProviderManager.h>
#include <Pegasus/ProviderManagerRouter/ProviderManagerRouter.h>
#include <Pegasus/ProviderManagerRouter/Linkage.h>

PEGASUS_NAMESPACE_BEGIN

typedef void (*PEGASUS_PROVIDERMODULEGROUPFAIL_CALLBACK_T)(const String &,
    const String &, Uint16, Boolean);

typedef void (*PEGASUS_ASYNC_RESPONSE_CALLBACK_T)(
    CIMRequestMessage* request, CIMResponseMessage* response);

class ProviderAgentContainer;

typedef HashTable<String, ProviderAgentContainer*, EqualFunc<String>,
    HashFunc<String> > ProviderAgentTable;

class PEGASUS_PMR_LINKAGE OOPProviderManagerRouter
    : public ProviderManagerRouter
{
public:
    OOPProviderManagerRouter(
        PEGASUS_INDICATION_CALLBACK_T indicationCallback,
        PEGASUS_RESPONSE_CHUNK_CALLBACK_T responseChunkCallback,
        PEGASUS_PROVIDERMODULEGROUPFAIL_CALLBACK_T
            providerModuleGroupFailCallback,
        PEGASUS_ASYNC_RESPONSE_CALLBACK_T asyncResponseCallback);

    virtual ~OOPProviderManagerRouter();

    virtual Message* processMessage(Message* message);

    virtual void idleTimeCleanup();

    virtual void enumerationContextCleanup(const String& contextId);

private:
    //
    // Private methods
    //

    /** determine moduleName from providerModule, function sets moduleName */
    void _getProviderModuleName(
        const CIMInstance & providerModule,
        String & moduleName);

    /**
        Determine groupName from the providerModule. Function sets
        groupName. A type prefix(grp or mod) is added to distinguish
        between group and module names.
        If ModuleGroupName property value is not specified, moduleName is set
        to groupName.
    */
    void _getGroupNameWithType(
        const CIMInstance &providerModule,
        String &groupNameWithType);

    /** Unimplemented */
    OOPProviderManagerRouter();
    /** Unimplemented */
    OOPProviderManagerRouter(const OOPProviderManagerRouter&);
    /** Unimplemented */
    OOPProviderManagerRouter& operator=(const OOPProviderManagerRouter&);

    /**
        Return a pointer to the ProviderAgentContainer for the specified
        module instance and requesting user.  If no appropriate
        ProviderAgentContainer exists, one is created in an uninitialized state.
     */
    ProviderAgentContainer* _lookupProviderAgent(
        const CIMInstance& providerModule,
        CIMRequestMessage* request);

    /** Function creates a copy of _providerAgentTable. Do NOT inline this
        method ever since it needs obtain the _providerAgentTableMutex and also
        does have to release the mutex before returning. When inlined the
        AutoMutex will not unlock.
      */
    Array<ProviderAgentContainer*> _getProviderAgentContainerCopy();

    /**
        Return an array of pointers to ProviderAgentContainers for the
        specified moduleName.
     */
    Array<ProviderAgentContainer*> _lookupProviderAgents(
        const String& moduleName);

    /**
        Send the specified CIMRequestMessage to all initialized Provider
        Agents in the _providerAgentTable.
     */
    CIMResponseMessage* _forwardRequestToAllAgents(CIMRequestMessage* request);

    void _handleIndicationDeliveryResponse(CIMResponseMessage *response);

    //
    // Private data
    //

    /**
        Callback function to which all generated indications are sent for
        processing.
     */
    PEGASUS_INDICATION_CALLBACK_T _indicationCallback;

    /**
        Callback function to which all response chunks are sent for processing.
     */
    PEGASUS_RESPONSE_CHUNK_CALLBACK_T _responseChunkCallback;

    /**
        Callback function to be called upon detection of failure of a
        provider module.
     */
    PEGASUS_PROVIDERMODULEGROUPFAIL_CALLBACK_T _providerModuleGroupFailCallback;

    /**
       Callback function to be called for all async responses.
    */
    PEGASUS_ASYNC_RESPONSE_CALLBACK_T _asyncResponseCallback;

    /**
        The _providerAgentTable contains a ProviderAgentContainer entry for
        each of the Provider Agent processes for which a request has been
        processed.  ProviderAgentContainer objects are persistent; once one
        is created it is never deleted.
     */
    ProviderAgentTable _providerAgentTable;
    /**
        The _providerAgentTableMutex must be locked whenever the
        _providerAgentTable is accessed.
     */
    Mutex _providerAgentTableMutex;

    /**
        Pointer to the MessageQueueService thread pool.
    */
    ThreadPool *_threadPool;

};

PEGASUS_NAMESPACE_END

#endif
