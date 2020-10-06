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

#include "ReadWriteSem.h"
#include "Time.h"
#include "PegasusAssert.h"
#include "Threads.h"
#include "Exception.h"
#include "System.h"

PEGASUS_NAMESPACE_BEGIN

//==============================================================================
//
// PEGASUS_USE_POSIX_RWLOCK
//
//==============================================================================

#ifdef PEGASUS_USE_POSIX_RWLOCK

ReadWriteSem::ReadWriteSem()
{
    pthread_rwlock_init(&_rwlock.rwlock, NULL);
}

ReadWriteSem::~ReadWriteSem()
{
    int r = 0;
    while ((r = pthread_rwlock_destroy(&_rwlock.rwlock)) == EBUSY ||
           (r == -1 && errno == EBUSY))
    {
        Threads::yield();
    }
}

void ReadWriteSem::waitRead()
{
    int r = pthread_rwlock_rdlock(&_rwlock.rwlock);

    if (r != 0)
    {
        if (r != -1)
        {
            // Special behavior for Single UNIX Specification, Version 3
            errno = r;
        }

        throw Exception(MessageLoaderParms(
            "Common.InternalException.READ_LOCK_FAILED",
            "Failed to acquire read lock: $0",
            PEGASUS_SYSTEM_ERRORMSG_NLS));
    }
}

void ReadWriteSem::waitWrite()
{
    int r = pthread_rwlock_wrlock(&_rwlock.rwlock);

    if (r != 0)
    {
        if (r != -1)
        {
            // Special behavior for Single UNIX Specification, Version 3
            errno = r;
        }

        throw Exception(MessageLoaderParms(
            "Common.InternalException.WRITE_LOCK_FAILED",
            "Failed to acquire write lock: $0",
            PEGASUS_SYSTEM_ERRORMSG_NLS));
    }
}

void ReadWriteSem::unlockRead()
{
    // All documented error codes represent coding errors.
    PEGASUS_FCT_EXECUTE_AND_ASSERT(0, pthread_rwlock_unlock(&_rwlock.rwlock));
}

void ReadWriteSem::unlockWrite()
{
    // All documented error codes represent coding errors.
    PEGASUS_FCT_EXECUTE_AND_ASSERT(0, pthread_rwlock_unlock(&_rwlock.rwlock));
}

#endif /* PEGASUS_USE_POSIX_RWLOCK */

//==============================================================================
//
// PEGASUS_USE_SEMAPHORE_RWLOCK
//
//==============================================================================

#if defined(PEGASUS_USE_SEMAPHORE_RWLOCK)

// // If i get cancelled, I MUST ensure:
// 1) I do not hold the internal mutex
// 2) I do not hold the write lock
// 3) I am not using a reader slot

ReadWriteSem::ReadWriteSem() : _rwlock()
{
}

ReadWriteSem::~ReadWriteSem()
{
    // lock everyone out of this object
    try
    {
        _rwlock._internal_lock.lock();
    }
    catch (...)
    {
        PEGASUS_ASSERT(0);
    }
    while (_rwlock._readers.get() > 0 || _rwlock._writers.get() > 0)
    {
        Threads::yield();
    }
    _rwlock._internal_lock.unlock();
}

void ReadWriteSem::waitRead()
{
    // Lock the internal mutex to ensure only one waiter is processed at a time.
    AutoMutex lock(_rwlock._internal_lock);

    // Wait for the existing writer (if any) to clear.
    while (_rwlock._writers.get() > 0)
    {
        Threads::yield();
    }

    // Wait for a reader slot to open up.
    _rwlock._rlock.wait();

    // Increment the number of readers.
    _rwlock._readers++;
}

void ReadWriteSem::waitWrite()
{
    // Lock the internal mutex to ensure only one waiter is processed at a time.
    AutoMutex lock(_rwlock._internal_lock);

    // Allow all the readers to exit.
    while (_rwlock._readers.get() > 0)
    {
        Threads::yield();
    }

    // Obtain the write mutex.
    _rwlock._wlock.lock();

    // Set the writer count to one.
    _rwlock._writers = 1;
}

void ReadWriteSem::unlockRead()
{
    PEGASUS_ASSERT(_rwlock._readers.get() > 0);
    _rwlock._readers--;
    _rwlock._rlock.signal();
}

void ReadWriteSem::unlockWrite()
{
    PEGASUS_ASSERT(_rwlock._writers.get() == 1);
    _rwlock._writers = 0;
    _rwlock._wlock.unlock();
}

#endif /* !PEGASUS_USE_SEMAPHORE_RWLOCK */

PEGASUS_NAMESPACE_END
