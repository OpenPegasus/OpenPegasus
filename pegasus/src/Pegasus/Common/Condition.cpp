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

#include "Condition.h"
#include "PegasusAssert.h"
#include "Exception.h"
#include "System.h"

PEGASUS_NAMESPACE_BEGIN

//==============================================================================
//
// PEGASUS_HAVE_PTHREADS
//
//==============================================================================

#if defined(PEGASUS_HAVE_PTHREADS)

Condition::Condition()
{
    pthread_cond_init(&_rep.cond, NULL);
}

Condition::~Condition()
{
    pthread_cond_destroy(&_rep.cond);
}

void Condition::signal()
{
    pthread_cond_signal(&_rep.cond);
}

void Condition::wait(Mutex& lock)
{
    pthread_cond_wait(&_rep.cond, &lock._rep.mutex);
}

#endif /* PEGASUS_HAVE_PTHREADS */

//==============================================================================
//
// PEGASUS_HAVE_WINDOWS_THREADS
//
//==============================================================================

#if defined(PEGASUS_HAVE_WINDOWS_THREADS)

ConditionWaiter* _get_waiter()
{
    // ATTN: since Windows Thread Local Storage does not have a cleanup
    // routine mechanism, the ConditionWaiter object (and its event) will
    // be leaked when the thread exists.

    static DWORD _waiter_tls = TlsAlloc();

    // Obtain (or create) the waiter for this thread.

    ConditionWaiter* waiter = (ConditionWaiter*)TlsGetValue(_waiter_tls);

    if (waiter == 0)
    {
        waiter = new ConditionWaiter;
        waiter->event = CreateEvent(0, TRUE, FALSE, 0);
        if (waiter->event == NULL)
        {
            throw Exception(MessageLoaderParms(
                "Common.InternalException.CREATE_EVENT_FAILED",
                "CreateEvent failed : $0",
                PEGASUS_SYSTEM_ERRORMSG_NLS));
        }
        TlsSetValue(_waiter_tls, waiter);
    }

    return waiter;
}

Condition::Condition()
{
}

Condition::~Condition()
{
    // Remove all of the waiters without deleting them (they are owned by
    // the thread-local-storage slot defined above and should never be
    // deleted.

    while (_rep.waiters.size())
        _rep.waiters.remove_front();
}

void Condition::signal()
{
    // Remove the waiter at the front of the queue.

    ConditionWaiter* waiter = _rep.waiters.remove_front();

    if (waiter)
        SetEvent(waiter->event);
}

void Condition::wait(Mutex& mutex)
{
    // Insert this thread's waiter at end of waiters queue.

    ConditionWaiter* waiter = _get_waiter();
    _rep.waiters.insert_back(waiter);

    // Unlock the mutex.

    size_t count = mutex._rep.count;

    for (size_t i = 0; i < count; i++)
        mutex.unlock();

    // Wait for a signal.

    DWORD rc = WaitForSingleObject(waiter->event, INFINITE);
    PEGASUS_DEBUG_ASSERT(rc == WAIT_OBJECT_0);

    // Relock the mutex.

    for (size_t i = 0; i < count; i++)
        mutex.lock();

    // Reset the event.

    ResetEvent(waiter->event);
}

#endif /* PEGASUS_HAVE_WINDOWS_THREADS */

PEGASUS_NAMESPACE_END
