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

#include <Pegasus/Common/Time.h>
#include <Pegasus/Common/Exception.h>
#include <Pegasus/Common/System.h>
#include "Semaphore.h"

PEGASUS_NAMESPACE_BEGIN

static const Uint32 PEGASUS_SEM_VALUE_MAX = 0x0000ffff;

//==============================================================================
//
// PEGASUS_USE_PTHREAD_SEMAPHORE
//
//==============================================================================

#if defined(PEGASUS_USE_PTHREAD_SEMAPHORE)

Semaphore::Semaphore(Uint32 initial)
{
    pthread_mutex_init(&_rep.mutex, NULL);
    pthread_cond_init(&_rep.cond, NULL);

    if (initial > PEGASUS_SEM_VALUE_MAX)
    {
        _rep.count = PEGASUS_SEM_VALUE_MAX - 1;
    }
    else
    {
        _rep.count = initial;
    }

    _rep.owner = Threads::self();
    _rep.waiters = 0;
}

Semaphore::~Semaphore()
{
#if !defined(PEGASUS_PLATFORM_AIX_RS_IBMCXX) \
    && !defined(PEGASUS_PLATFORM_PASE_ISERIES_IBMCXX)
    pthread_mutex_lock(&_rep.mutex);
    int r = 0;
    while (((r = pthread_cond_destroy(&_rep.cond)) == EBUSY) ||
           (r == -1 && errno == EBUSY))
    {
        pthread_mutex_unlock(&_rep.mutex);
        Threads::yield();
        pthread_mutex_lock(&_rep.mutex);
    }
    pthread_mutex_unlock(&_rep.mutex);
    pthread_mutex_destroy(&_rep.mutex);
#else
    int val;
    val = pthread_mutex_destroy(&_rep.mutex);

    if (val != 0)
    {
        pthread_cond_destroy(&_rep.cond);
    }
    else
    {
        val = pthread_cond_destroy(&_rep.cond);
    }

    while (EBUSY == val)
    {
        Threads::yield();
        val = pthread_mutex_destroy(&_rep.mutex);
        if (val != 0)
        {
            pthread_cond_destroy(&_rep.cond);
        }
        else
        {
            val = pthread_cond_destroy(&_rep.cond);
        }
    }
#endif
}

#if defined(PEGASUS_PLATFORM_AIX_RS_IBMCXX) \
    || defined(PEGASUS_PLATFORM_PASE_ISERIES_IBMCXX)
// cleanup function
static void semaphore_cleanup(void *arg)
{
    // cast back to proper type and unlock mutex
    SemaphoreRep *s = (SemaphoreRep *) arg;
    pthread_mutex_unlock(&s->mutex);
}
#endif

// block until this semaphore is in a signalled state or
// throw an exception if the wait fails
void Semaphore::wait()
{
    // Acquire mutex to enter critical section.
    pthread_mutex_lock(&_rep.mutex);

    // Keep track of the number of waiters so that <sema_post> works correctly.
    _rep.waiters++;

    // Wait until the semaphore count is > 0, then atomically release
    // <lock_> and wait for <count_nonzero_> to be signaled.
    while (_rep.count == 0)
    {
        pthread_cond_wait(&_rep.cond, &_rep.mutex);
    }

    // <_rep.mutex> is now held.

    // Decrement the waiters count.
    _rep.waiters--;

    // Decrement the semaphore's count.
    _rep.count--;

    // Since we push an unlock onto the cleanup stack
    // We will pop it off to release the mutex when leaving the critical
    // section.
    
    // Release mutex to leave critical section.
    pthread_mutex_unlock(&_rep.mutex);
}

Boolean Semaphore::time_wait(Uint32 milliseconds)
{
    // Acquire mutex to enter critical section.
    pthread_mutex_lock(&_rep.mutex);
    Boolean timedOut = false;

    // Keep track of the number of waiters so that <sema_post> works correctly.
    _rep.waiters++;

    struct timeval now = { 0, 0 };
    struct timespec waittime = { 0, 0 };
    gettimeofday(&now, NULL);

    waittime.tv_sec = now.tv_sec + (milliseconds / 1000);
    milliseconds = milliseconds % 1000;
    waittime.tv_nsec = now.tv_usec + (milliseconds * 1000);     // microseconds
    waittime.tv_sec += (waittime.tv_nsec / 1000000);    // roll overflow into
    waittime.tv_nsec = (waittime.tv_nsec % 1000000);    // the "seconds" part
    waittime.tv_nsec = waittime.tv_nsec * 1000; // convert to nanoseconds

    while ((_rep.count == 0) && !timedOut)
    {
        int r = pthread_cond_timedwait(&_rep.cond, &_rep.mutex, &waittime);

#ifdef PEGASUS_OS_ZOS
        if (((r==-1 && errno==EAGAIN) || (r==ETIMEDOUT)) && _rep.count==0)
#else
        if (((r==-1 && errno==ETIMEDOUT) || (r==ETIMEDOUT)) && _rep.count==0)
#endif
        {
            timedOut = true;
        }
    }

    if (!timedOut)
    {
        // Decrement the semaphore's count.
        _rep.count--;
    }

    // Decrement the waiters count.
    _rep.waiters--;

    // Release mutex to leave critical section.
    pthread_mutex_unlock(&_rep.mutex);

    return !timedOut;
}

// increment the count of the semaphore
void Semaphore::signal()
{
    pthread_mutex_lock(&_rep.mutex);

    // Always allow one thread to continue if it is waiting.
    if (_rep.waiters > 0)
    {
        pthread_cond_signal(&_rep.cond);
    }

    // Increment the semaphore's count.
    _rep.count++;

    pthread_mutex_unlock(&_rep.mutex);
}

#endif /* PEGASUS_USE_PTHREAD_SEMAPHORE */

//==============================================================================
//
// PEGASUS_USE_POSIX_SEMAPHORE
//
//==============================================================================

#if defined(PEGASUS_USE_POSIX_SEMAPHORE)

Semaphore::Semaphore(Uint32 initial)
{
    if (initial > PEGASUS_SEM_VALUE_MAX)
    {
        initial = PEGASUS_SEM_VALUE_MAX - 1;
    }

    _rep.owner = Threads::self();
    if (sem_init(&_rep.sem, 0, initial) == -1)
    {
        throw Exception(MessageLoaderParms(
            "Common.InternalException.SEMAPHORE_INIT_FAILED",
            "Semaphore initialization failed: $0",
            PEGASUS_SYSTEM_ERRORMSG_NLS));
    }
}

Semaphore::~Semaphore()
{
    while (sem_destroy(&_rep.sem) == -1 && errno == EBUSY)
    {
        Threads::yield();
    }
}

// block until this semaphore is in a signalled state, or
// throw an exception if the wait fails
void Semaphore::wait()
{
    do
    {
        int rc = sem_wait(&_rep.sem);
        if (rc == 0)
            break;

        if (errno != EINTR)
        {
            throw Exception(MessageLoaderParms(
                "Common.InternalException.SEMAPHORE_WAIT_FAILED",
                "Semaphore wait failed: $0",
                PEGASUS_SYSTEM_ERRORMSG_NLS));
        }

        // keep going if above conditions fail
    }
    while (true);

}

// wait for milliseconds and throw an exception
// if wait times out without gaining the semaphore
Boolean Semaphore::time_wait(Uint32 milliseconds)
{
    int retcode;

    struct timeval now, finish, remaining;
    Uint32 usec;

    gettimeofday(&finish, NULL);
    finish.tv_sec += (milliseconds / 1000);
    usec = finish.tv_usec + ((milliseconds % 1000) * 1000);
    finish.tv_sec += (usec / 1000000);
    finish.tv_usec = usec % 1000000;

    while (1)
    {
        do
        {
            retcode = sem_trywait(&_rep.sem);
        }
        while (retcode == -1 && errno == EINTR);

        if (retcode == 0)
        {
            break;
        }

        if (retcode == -1 && errno != EAGAIN)
        {
            throw Exception(MessageLoaderParms(
                "Common.InternalException.SEMAPHORE_WAIT_FAILED",
                "Semaphore wait failed: $0",
                PEGASUS_SYSTEM_ERRORMSG_NLS));
        }

        gettimeofday(&now, NULL);
        if (Time::subtract(&remaining, &finish, &now))
        {
            return false;
        }
        // yield just marks the thread as eligible to be not scheduled by 
        // hypervisor, sleep forces thread to actually take a break
        // which what is called for here to avoid CPU spikes from close loop
        Threads::sleep(milliseconds/100+1);
    }

    return true;
}

// increment the count of the semaphore
void Semaphore::signal()
{
    if (sem_post(&_rep.sem) == -1)
    {
        throw Exception(MessageLoaderParms(
            "Common.InternalException.SEMAPHORE_SIGNAL_FAILED",
            "Failed to signal semaphore: $0",
            PEGASUS_SYSTEM_ERRORMSG_NLS));
    }
}

#endif /* PEGASUS_USE_POSIX_SEMAPHORE */

//==============================================================================
//
// PEGASUS_USE_WINDOWS_SEMAPHORE
//
//==============================================================================

#if defined(PEGASUS_USE_WINDOWS_SEMAPHORE)

Semaphore::Semaphore(Uint32 initial)
{
    if (initial > PEGASUS_SEM_VALUE_MAX)
    {
        initial = PEGASUS_SEM_VALUE_MAX - 1;
    }
    _rep.owner = Threads::self();
    _rep.sem = CreateSemaphore(NULL, initial, PEGASUS_SEM_VALUE_MAX, NULL);
    if (_rep.sem == NULL)
    {
        throw Exception(MessageLoaderParms(
            "Common.InternalException.SEMAPHORE_INIT_FAILED",
            "Semaphore initialization failed: $0",
            PEGASUS_SYSTEM_ERRORMSG_NLS));
    }
}

Semaphore::~Semaphore()
{
    CloseHandle(_rep.sem);
}

// block until this semaphore is in a signalled state
void Semaphore::wait()
{
    DWORD errorcode = WaitForSingleObject(_rep.sem, INFINITE);
    if (errorcode == WAIT_FAILED)
    {
        throw Exception(MessageLoaderParms(
            "Common.InternalException.SEMAPHORE_WAIT_FAILED",
            "Semaphore wait failed: $0",
            PEGASUS_SYSTEM_ERRORMSG_NLS));
    }
}

Boolean Semaphore::time_wait(Uint32 milliseconds)
{
    DWORD errorcode = WaitForSingleObject(_rep.sem, milliseconds);

    if (errorcode == WAIT_TIMEOUT)
    {
        return false;
    }

    if (errorcode == WAIT_FAILED)
    {
        throw Exception(MessageLoaderParms(
            "Common.InternalException.SEMAPHORE_WAIT_FAILED",
            "Semaphore wait failed: $0",
            PEGASUS_SYSTEM_ERRORMSG_NLS));
    }

    return true;
}

// increment the count of the semaphore
void Semaphore::signal()
{
    ReleaseSemaphore(_rep.sem, 1, NULL);
}

#endif /* PEGASUS_USE_WINDOWS_SEMAPHORE */

PEGASUS_NAMESPACE_END
