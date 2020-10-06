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

#include "Mutex.h"
#include "Time.h"
#include "PegasusAssert.h"
#include "Once.h"
#include "Exception.h"
#include "System.h"

#define MUTEX_LOCK_FAILED_KEY "Common.InternalException.MUTEX_LOCK_FAILED"
#define MUTEX_LOCK_FAILED_MSG "Failed to acquire mutex lock: $0"

PEGASUS_NAMESPACE_BEGIN

//==============================================================================
//
// PEGASUS_HAVE_PTHREADS
//
//==============================================================================

#if defined(PEGASUS_HAVE_PTHREADS)

static Once _once = PEGASUS_ONCE_INITIALIZER;
static pthread_mutexattr_t _attr;

static void _init_attr()
{
    pthread_mutexattr_init(&_attr);
    pthread_mutexattr_settype(&_attr, PTHREAD_MUTEX_RECURSIVE);
}

Mutex::Mutex()
{
    once(&_once, _init_attr);
    pthread_mutex_init(&_rep.mutex, &_attr);
#if defined(PEGASUS_DEBUG)
    _rep.count = 0;
#endif
}

Mutex::Mutex(RecursiveTag)
{
    once(&_once, _init_attr);
    pthread_mutex_init(&_rep.mutex, &_attr);
#if defined(PEGASUS_DEBUG)
    _rep.count = 0;
#endif
}

Mutex::Mutex(NonRecursiveTag)
{
    pthread_mutex_init(&_rep.mutex, NULL);
#if defined(PEGASUS_DEBUG)
    _rep.count = 0;
#endif
}

Mutex::~Mutex()
{
    PEGASUS_DEBUG_ASSERT(_magic);
    pthread_mutex_destroy(&_rep.mutex);
}

void Mutex::lock()
{
    PEGASUS_DEBUG_ASSERT(_magic);

    int r = pthread_mutex_lock(&_rep.mutex);

    if (r == 0)
    {
#if defined(PEGASUS_DEBUG)
        _rep.count++;
#endif
    }
    else
    {
        if (r != -1)
        {
            // Special behavior for Single UNIX Specification, Version 3
            errno = r;
        }

        throw Exception(MessageLoaderParms(
            MUTEX_LOCK_FAILED_KEY,
            MUTEX_LOCK_FAILED_MSG,
            PEGASUS_SYSTEM_ERRORMSG_NLS));
    }
}

Boolean Mutex::try_lock()
{
    PEGASUS_DEBUG_ASSERT(_magic);

    int r = pthread_mutex_trylock(&_rep.mutex);

    if (r == 0)
    {
#if defined(PEGASUS_DEBUG)
        _rep.count++;
#endif
        return true;
    }

    if (r != -1)
    {
        // Special behavior for Single UNIX Specification, Version 3
        errno = r;
    }

    if (errno == EBUSY)
    {
        return false;
    }

    throw Exception(MessageLoaderParms(
        MUTEX_LOCK_FAILED_KEY,
        MUTEX_LOCK_FAILED_MSG,
        PEGASUS_SYSTEM_ERRORMSG_NLS));
}

Boolean Mutex::timed_lock(Uint32 milliseconds)
{
    struct timeval now;
    struct timeval finish;
    struct timeval remaining;
    {
        Uint32 usec;
        gettimeofday(&finish, NULL);
        finish.tv_sec += (milliseconds / 1000 );
        milliseconds %= 1000;
        usec = finish.tv_usec + ( milliseconds * 1000 );
        finish.tv_sec += (usec / 1000000);
        finish.tv_usec = usec % 1000000;
    }

    while (!try_lock())
    {
        gettimeofday(&now, NULL);

        if (Time::subtract(&remaining, &finish, &now))
        {
            return false;
        }

        Threads::yield();
    }

    return true;
}

void Mutex::unlock()
{
    PEGASUS_DEBUG_ASSERT(_magic);
    PEGASUS_DEBUG_ASSERT(_rep.count > 0);

#if defined(PEGASUS_DEBUG)
    _rep.count--;
#endif

    // All documented error codes represent coding errors.
    PEGASUS_FCT_EXECUTE_AND_ASSERT(0, pthread_mutex_unlock(&_rep.mutex));
}

#if defined(PEGASUS_OS_LINUX)
void Mutex::reinitialize()
{
    pthread_mutex_init(&_rep.mutex, &_attr);
#if defined(PEGASUS_DEBUG)
    _rep.count = 0;
#endif
}
#endif

#endif /* PEGASUS_HAVE_PTHREADS */

//==============================================================================
//
// PEGASUS_HAVE_WINDOWS_THREADS
//
//==============================================================================

#if defined(PEGASUS_HAVE_WINDOWS_THREADS)

static inline void _initialize(MutexRep& rep)
{
    rep.handle = CreateMutex(NULL, FALSE, NULL);
    if (rep.handle == NULL)
    {
        throw Exception(MessageLoaderParms(
            "Common.InternalException.CREATE_MUTEX_FAILED",
            "CreateMutex failed : $0",
            PEGASUS_SYSTEM_ERRORMSG_NLS));
    }
    rep.count = 0;
}

Mutex::Mutex()
{
    _initialize(_rep);
}

Mutex::Mutex(RecursiveTag)
{
    _initialize(_rep);
}

Mutex::Mutex(NonRecursiveTag)
{
    _initialize(_rep);
}

Mutex::~Mutex()
{
    PEGASUS_DEBUG_ASSERT(_magic);

    WaitForSingleObject(_rep.handle, INFINITE);
    CloseHandle(_rep.handle);
}

void Mutex::lock()
{
    PEGASUS_DEBUG_ASSERT(_magic);

    DWORD rc = WaitForSingleObject(_rep.handle, INFINITE);

    if (rc == WAIT_FAILED)
    {
        throw Exception(MessageLoaderParms(
            MUTEX_LOCK_FAILED_KEY,
            MUTEX_LOCK_FAILED_MSG,
            PEGASUS_SYSTEM_ERRORMSG_NLS));
    }

    _rep.count++;
}

Boolean Mutex::try_lock()
{
    PEGASUS_DEBUG_ASSERT(_magic);

    DWORD rc = WaitForSingleObject(_rep.handle, 0);

    if (rc == WAIT_TIMEOUT)
    {
        return false;
    }

    if (rc == WAIT_FAILED)
    {
        throw Exception(MessageLoaderParms(
            MUTEX_LOCK_FAILED_KEY,
            MUTEX_LOCK_FAILED_MSG,
            PEGASUS_SYSTEM_ERRORMSG_NLS));
    }

    _rep.count++;
    return true;
}

Boolean Mutex::timed_lock(Uint32 milliseconds)
{
    PEGASUS_DEBUG_ASSERT(_magic);

    DWORD rc = WaitForSingleObject(_rep.handle, milliseconds);

    if (rc == WAIT_TIMEOUT)
        return false;

    if (rc == WAIT_FAILED)
    {
        throw Exception(MessageLoaderParms(
            MUTEX_LOCK_FAILED_KEY,
            MUTEX_LOCK_FAILED_MSG,
            PEGASUS_SYSTEM_ERRORMSG_NLS));
    }

    _rep.count++;
    return true;
}

void Mutex::unlock()
{
    PEGASUS_DEBUG_ASSERT(_magic);
    PEGASUS_DEBUG_ASSERT(_rep.count > 0);

    _rep.count--;
    ReleaseMutex(_rep.handle);
}

#endif /* PEGASUS_HAVE_WINDOWS_THREADS */

PEGASUS_NAMESPACE_END
