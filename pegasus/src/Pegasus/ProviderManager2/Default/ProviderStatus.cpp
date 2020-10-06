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

#include "ProviderStatus.h"
#include <Pegasus/Common/Time.h>
#include <Pegasus/Common/PegasusAssert.h>

PEGASUS_NAMESPACE_BEGIN

ProviderStatus::ProviderStatus()
{
    setInitialized(false);
}

ProviderStatus::~ProviderStatus()
{
}

Boolean ProviderStatus::isInitialized()
{
    AutoMutex lock(_statusMutex);
    return _isInitialized;
}

void ProviderStatus::setInitialized(Boolean initialized)
{
    AutoMutex lock(_statusMutex);
    _isInitialized = initialized;

    if (!_isInitialized)
    {
        _module = 0;
        _cimomHandle = 0;
        _currentOperations = 0;
        _indicationsEnabled = false;
        _currentSubscriptions = 0;
    }

    Time::gettimeofday(&_lastOperationEndTime);
}

ProviderModule* ProviderStatus::getModule() const
{
    return _module;
}

void ProviderStatus::setModule(ProviderModule* module)
{
    _module = module;
}

CIMOMHandle* ProviderStatus::getCIMOMHandle()
{
    return _cimomHandle;
}

void ProviderStatus::setCIMOMHandle(CIMOMHandle* cimomHandle)
{
    _cimomHandle = cimomHandle;
}

void ProviderStatus::getLastOperationEndTime(struct timeval* t)
{
    PEGASUS_ASSERT(t != 0);
    AutoMutex lock(_lastOperationEndTimeMutex);
    memcpy(t, &_lastOperationEndTime, sizeof(struct timeval));
}

Boolean ProviderStatus::isIdle()
{
    if (!_isInitialized ||
        (_currentOperations.get() > 0) ||
        _indicationsEnabled)
    {
        return false;
    }

    if (_cimomHandle)
    {
        return _cimomHandle->unload_ok();
    }

    return true;
}

void ProviderStatus::operationBegin()
{
    _currentOperations++;
}

void ProviderStatus::operationEnd()
{
    _currentOperations--;

    // Update the timer used to detect idle providers
    AutoMutex lock(_lastOperationEndTimeMutex);
    Time::gettimeofday(&_lastOperationEndTime);
}

Uint32 ProviderStatus::numCurrentOperations() const
{
    return _currentOperations.get();
}

Boolean ProviderStatus::getIndicationsEnabled() const
{
    return _indicationsEnabled;
}

void ProviderStatus::setIndicationsEnabled(Boolean indicationsEnabled)
{
    _indicationsEnabled = indicationsEnabled;
}

Boolean ProviderStatus::testIfZeroAndIncrementSubscriptions()
{
    AutoMutex lock(_currentSubscriptionsMutex);
    return (_currentSubscriptions++ == 0);
}

Boolean ProviderStatus::decrementSubscriptionsAndTestIfZero()
{
    AutoMutex lock(_currentSubscriptionsMutex);
    return (--_currentSubscriptions == 0);
}

Boolean ProviderStatus::testSubscriptions()
{
    AutoMutex lock(_currentSubscriptionsMutex);
    return (_currentSubscriptions > 0);
}

void ProviderStatus::resetSubscriptions()
{
    AutoMutex lock(_currentSubscriptionsMutex);
    _currentSubscriptions = 0;
}

void ProviderStatus::setProviderInstance(const CIMInstance& instance)
{
    _providerInstance = instance;
}

CIMInstance ProviderStatus::getProviderInstance()
{
    return _providerInstance;
}

Mutex& ProviderStatus::getStatusMutex()
{
    return _statusMutex;
}

PEGASUS_NAMESPACE_END
