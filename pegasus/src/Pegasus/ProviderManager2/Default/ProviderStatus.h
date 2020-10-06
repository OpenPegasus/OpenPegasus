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

#ifndef Pegasus_ProviderStatus_h
#define Pegasus_ProviderStatus_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/Mutex.h>
#include <Pegasus/Provider/CIMOMHandle.h>
#include <Pegasus/ProviderManager2/Default/ProviderModule.h>
#include <Pegasus/ProviderManager2/Default/Linkage.h>

PEGASUS_NAMESPACE_BEGIN

// The Provider class represents the logical provider extracted from a
// provider module. It is wrapped in a facade to stabalize the interface
// and is directly tied to a module.

class PEGASUS_DEFPM_LINKAGE ProviderStatus
{
public:
    ProviderStatus();

    ~ProviderStatus();

    Boolean isInitialized();

    /**
        Set the initialization state of the provider.  If the initialized
        parameter is false, the ProviderStatus values are reset.
     */
    void setInitialized(Boolean initialized);

    ProviderModule* getModule() const;
    void setModule(ProviderModule* module);

    CIMOMHandle* getCIMOMHandle();
    void setCIMOMHandle(CIMOMHandle* cimomHandle);

    /**
        Indicates that the provider is busy processing an operation.
     */
    void operationBegin();

    /**
        Indicates that the provider is finished processing an operation.
     */
    void operationEnd();

    /**
        Returns the time at which the operationEnd() method was last called.
     */
    void getLastOperationEndTime(struct timeval*);

    /**
        Returns true if the provider is initialized, is not processing any
        operations, is not enabled as an indication provider, and has not
        been protected against unloads via the CIMOMHandle.
     */
    Boolean isIdle();

    /**
        Gets the number of provider operations currently in progress.
     */
    Uint32 numCurrentOperations() const;

    /**
        Returns a Boolean indicating whether indications have been enabled
        for this provider.
     */
    Boolean getIndicationsEnabled() const;

    /**
        Sets a flag indicating whether indications have been enabled
        for this provider.
     */
    void setIndicationsEnabled(Boolean indicationsEnabled);

    /**
        Increments the count of current subscriptions for this provider, and
        determines if there were no current subscriptions before the increment.
        If there were no current subscriptions before the increment, the first
        subscription has been created, and the provider's enableIndications
        method should be called.

        @return  True, if before the increment there were no current
                       subscriptions for this provider;
                 False, otherwise
     */
    Boolean testIfZeroAndIncrementSubscriptions();

    /**
        Decrements the count of current subscriptions for this provider, and
        determines if there are no current subscriptions after the decrement.
        If there are no current subscriptions after the decrement, the last
        subscription has been deleted, and the provider's disableIndications
        method should be called.

        @return  True, if after the decrement there are no current subscriptions
                       for this provider;
                 False, otherwise
     */
    Boolean decrementSubscriptionsAndTestIfZero();

    /**
        Determines if there are current subscriptions for this provider.

        @return  True, if there is at least one current subscription
                       for this provider;
                 False, otherwise
     */
    Boolean testSubscriptions();

    /**
        Resets the count of current subscriptions for the indication provider.
     */
    void resetSubscriptions();

    /**
        Sets the provider instance for the provider.

        Note: the provider instance is set only for an indication provider, and
        is set when a Create Subscription request is processed for the provider.

        @param  instance  the Provider CIMInstance for the provider
     */
    void setProviderInstance(const CIMInstance& instance);

    /**
        Gets the provider instance for the provider.

        Note: the provider instance is set only for an indication provider, and
        only if a Create Subscription request has been processed for the
        provider.

        @return  the Provider CIMInstance for the provider
     */
    CIMInstance getProviderInstance();

    /**
        The status mutex must be locked while changes to the initialization
        status of the provider are in progress.  This mutex may be locked
        recursively.
     */
    Mutex& getStatusMutex();

private:
    ProviderStatus(const ProviderStatus&);
    ProviderStatus& operator=(const ProviderStatus&);

    Boolean _isInitialized;
    Mutex _statusMutex;    // Must be a recursive mutex

    ProviderModule* _module;
    CIMOMHandle* _cimomHandle;

    AtomicInt _currentOperations;
    Boolean _indicationsEnabled;

    struct timeval _lastOperationEndTime;
    Mutex _lastOperationEndTimeMutex;

    /**
        Count of current subscriptions for this provider.  Access to this
        data member is controlled by the _currentSubscriptionsLock.
     */
    Uint32 _currentSubscriptions;

    /**
        A mutex to control access to the _currentSubscriptions member variable.
        Before any access (test, increment, decrement or reset) of the
        _currentSubscriptions member variable, the _currentSubscriptionsMutex
        is first locked.
     */
    Mutex _currentSubscriptionsMutex;

    /**
        The Provider CIMInstance for the provider.
        The Provider CIMInstance is set only for indication providers, and only
        if a Create Subscription request has been processed for the provider.
        The Provider CIMInstance is needed in order to construct the
        EnableIndicationsResponseHandler to send to the indication provider
        when the provider's enableIndications() method is called.
        The Provider CIMInstance is needed in the
        EnableIndicationsResponseHandler in order to construct the Process
        Indication request when an indication is delivered by the provider.
        The Provider CIMInstance is needed in the Process Indication request
        to enable the Indication Service to determine if the provider that
        generated the indication accepted a matching subscription.
     */
    CIMInstance _providerInstance;
};

PEGASUS_NAMESPACE_END

#endif
