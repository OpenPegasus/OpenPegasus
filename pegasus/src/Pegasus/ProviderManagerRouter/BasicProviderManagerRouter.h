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

#ifndef Pegasus_BasicProviderManagerRouter_h
#define Pegasus_BasicProviderManagerRouter_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/ArrayInternal.h>
#include <Pegasus/Common/CIMMessage.h>
#include <Pegasus/Common/OperationContextInternal.h>
#include <Pegasus/Common/ReadWriteSem.h>

#include <Pegasus/ProviderManager2/ProviderManager.h>
#include <Pegasus/ProviderManagerRouter/ProviderManagerRouter.h>
#include <Pegasus/ProviderManagerRouter/Linkage.h>

PEGASUS_NAMESPACE_BEGIN

class ProviderManagerContainer;

class PEGASUS_PMR_LINKAGE BasicProviderManagerRouter
    : public ProviderManagerRouter
{
public:
    BasicProviderManagerRouter(
        PEGASUS_INDICATION_CALLBACK_T indicationCallback,
        PEGASUS_RESPONSE_CHUNK_CALLBACK_T responseChunkCallback,
        ProviderManager* (*createDefaultProviderManagerCallback)());

    virtual ~BasicProviderManagerRouter();

    virtual Message* processMessage(Message* message);

    /**
        Indicates whether any of the active ProviderManagers managed by this
        ProviderManagerRouter has an active (e.g., loaded/initialized)
        provider.  If no providers are active, this ProviderManagerRouter
        may be destructed without harm.
     */
    Boolean hasActiveProviders();

    virtual void idleTimeCleanup();

    virtual void enumerationContextCleanup(const String& contextId);

private:
    BasicProviderManagerRouter();
    BasicProviderManagerRouter(const BasicProviderManagerRouter&);
    BasicProviderManagerRouter& operator=(const BasicProviderManagerRouter&);

   // providerModuleName is used to report the error when ProviderManager
   // can not be found for the interfaceType.
    ProviderManager* _getProviderManager(
        const String& interfaceType,
        const String& providerManagerPath,
        Boolean loadProviderManager);

    ProviderManager* _lookupProviderManager(const String& interfaceType);

    Array<ProviderManagerContainer*> _providerManagerTable;
    ReadWriteSem _providerManagerTableLock;

    static PEGASUS_INDICATION_CALLBACK_T _indicationCallback;
    static PEGASUS_RESPONSE_CHUNK_CALLBACK_T _responseChunkCallback;
    static ProviderManager* (*_createDefaultProviderManagerCallback)();
};

PEGASUS_NAMESPACE_END

#endif
