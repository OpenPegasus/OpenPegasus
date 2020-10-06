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

#ifndef Pegasus_ProviderManagerRouter_h
#define Pegasus_ProviderManagerRouter_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/Message.h>
#include <Pegasus/ProviderManagerRouter/Linkage.h>

PEGASUS_NAMESPACE_BEGIN

class PEGASUS_PMR_LINKAGE ProviderManagerRouter
{
public:
    ProviderManagerRouter();
    virtual ~ProviderManagerRouter();

    /**
        Process the specified message by routing it to the appropriate
        ProviderManager.  Note: This operation may take a long time to
        complete and should be called on a non-critical thread.
     */
    virtual Message* processMessage(Message* message) = 0;

    /**
        Cleanup idle providers and disconnected client requests in all
        active ProviderManagers.
        Note: This operation may take a long time to complete and should
        be called on a non-critical thread.
     */
    virtual void idleTimeCleanup() = 0;

    /*
        Cleanup enumerationContexts that have timed out with incomplete
        responses from the providers.
        @param contextId String defining the contextId to cleanup.  Note
        that the contextId is also the request messageId.
    */
    virtual void enumerationContextCleanup(const String& contextId) = 0;

    /**
        Sets the SubscriptionInitComplete flag indicating whether the Indication
        Service has completed its initialization.
     */
    virtual void setSubscriptionInitComplete
        (Boolean subscriptionInitComplete);

protected:
    /**
        Indicates whether the Indication Service has completed initialization.
        During initialization, the Indication Service processes all active
        subscriptions from the repository, sending Create Subscription requests
        to the appropriate indication providers, and the providers'
        enableIndications method must be called only after all the Create
        Subscription requests have been processed.  Once Indication Service
        initialization is complete, the Indication Service sends the Provider
        Manager Service a Subscription Initialization Complete request message.
        At that time, the enableIndications method must be called on each
        provider with current subscriptions.  Subsequently, the
        enableIndications method must be called only after the first
        subscription is created for a provider, and the disableIndications
        method must be called when the last subscription is deleted for a
        provider.
     */
    Boolean _subscriptionInitComplete;
};

PEGASUS_NAMESPACE_END

#endif
