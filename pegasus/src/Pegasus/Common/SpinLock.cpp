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

#include "Network.h"

#if defined(PEGASUS_OS_SOLARIS)
# include <sys/types.h>
# include <unistd.h>
#endif

#include "SpinLock.h"

#if  !defined(PEGASUS_OS_VMS) &&  !defined(PEGASUS_OS_TYPE_WINDOWS)
# define PEGASUS_SPINLOCK_USE_PTHREADS
#endif

#ifdef PEGASUS_SPINLOCK_USE_PTHREADS
# include <pthread.h>
#else
# include "Mutex.h"
#endif

PEGASUS_NAMESPACE_BEGIN

SpinLock spinLockPool[PEGASUS_NUM_SHARED_SPIN_LOCKS];
int spinLockPoolInitialized;

#ifdef PEGASUS_SPINLOCK_USE_PTHREADS
pthread_mutex_t _spinLockInitMutex = PTHREAD_MUTEX_INITIALIZER;
#else
static Mutex _spinLockInitMutex;
#endif

void SpinLockCreatePool()
{
    // There's no need to check spinLockPoolInitialized before locking the
    // mutex, because the caller can check the flag before calling this
    // function.

#ifdef PEGASUS_SPINLOCK_USE_PTHREADS
    pthread_mutex_lock(&_spinLockInitMutex);
#else
    _spinLockInitMutex.lock();
#endif

    if (spinLockPoolInitialized == 0)
    {
        for (size_t i = 0; i < PEGASUS_NUM_SHARED_SPIN_LOCKS; i++)
            SpinLockCreate(spinLockPool[i]);

        spinLockPoolInitialized = 1;
    }

#ifdef PEGASUS_SPINLOCK_USE_PTHREADS
    pthread_mutex_unlock(&_spinLockInitMutex);
#else
    _spinLockInitMutex.unlock();
#endif
}

#if defined(PEGASUS_SPINLOCK_USE_PTHREADS)

// This function is called prior to forking.  We must obtain a lock
// on every mutex that the child will inherit.  These will remain locked
// until they are unlocked (by _unlockSpinLockPool()).  This prevents a
// child process from waiting indefinitely on a mutex that was locked by
// another thread in the parent process during the fork.

extern "C" void _lockSpinLockPool()
{
    // Initialize the spinlock pool if not already done.

    if (spinLockPoolInitialized == 0)
        SpinLockCreatePool();

    pthread_mutex_lock(&_spinLockInitMutex);

    for (size_t i = 0; i < PEGASUS_NUM_SHARED_SPIN_LOCKS; i++)
        SpinLockLock(spinLockPool[i]);
}

// This function is called after forking.  It unlocks the mutexes that
// were locked by _lockSpinLockPool() before the fork.

extern "C" void _unlockSpinLockPool()
{
    pthread_mutex_unlock(&_spinLockInitMutex);

    for (size_t i = 0; i < PEGASUS_NUM_SHARED_SPIN_LOCKS; i++)
        SpinLockUnlock(spinLockPool[i]);
}

class SpinLockInitializer
{
public:
    SpinLockInitializer()
    {
// ATTN: Temporary workaround for Bug 4559
#if defined(PEGASUS_OS_HPUX) || defined(PEGASUS_OS_SOLARIS)
        pthread_atfork(
            _lockSpinLockPool,
            _unlockSpinLockPool,
            _unlockSpinLockPool);
#endif
    }
};

static SpinLockInitializer spinLockInitializer;

#endif /* PEGASUS_SPINLOCK_USE_PTHREADS */

PEGASUS_NAMESPACE_END
