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

#include "ThreadPool.h"
#include "Thread.h"
#include <exception>
#include <Pegasus/Common/Tracer.h>
#include "Time.h"

PEGASUS_USING_STD;

PEGASUS_NAMESPACE_BEGIN

///////////////////////////////////////////////////////////////////////////////
//
// ThreadPool
//
///////////////////////////////////////////////////////////////////////////////

ThreadPool::ThreadPool(
    Sint16 initialSize,
    const char* key,
    Sint16 minThreads,
    Sint16 maxThreads,
    struct timeval
    &deallocateWait)
    : _maxThreads(maxThreads),
      _minThreads(minThreads),
      _currentThreads(0),
      _idleThreads(),
      _runningThreads(),
      _dying(0)
{
    _deallocateWait.tv_sec = deallocateWait.tv_sec;
    _deallocateWait.tv_usec = deallocateWait.tv_usec;

    memset(_key, 0x00, 17);
    if (key != 0)
    {
        strncpy(_key, key, 16);
    }

    if ((_maxThreads > 0) && (_maxThreads < initialSize))
    {
        _maxThreads = initialSize;
    }

    if (_minThreads > initialSize)
    {
        _minThreads = initialSize;
    }

    for (int i = 0; i < initialSize; i++)
    {
        _addToIdleThreadsQueue(_initializeThread());
    }
}

ThreadPool::~ThreadPool()
{
    PEG_METHOD_ENTER(TRC_THREAD, "ThreadPool::~ThreadPool");

    try
    {
        // Set the dying flag so all thread know the destructor has been
        // entered
        _dying++;
        PEG_TRACE((TRC_THREAD, Tracer::LEVEL3,
            "Cleaning up %d idle threads.", _currentThreads.get()));

        while (_currentThreads.get() > 0)
        {
            Thread* thread = _idleThreads.remove_front();
            if (thread != 0)
            {
                _cleanupThread(thread);
                _currentThreads--;
            }
            else
            {
                Threads::yield();
            }
        }
    }
    catch (...)
    {
    }
}

ThreadReturnType PEGASUS_THREAD_CDECL ThreadPool::_loop(void* parm)
{
    PEG_METHOD_ENTER(TRC_THREAD, "ThreadPool::_loop");

    try
    {
        Thread *myself = (Thread *) parm;
        PEGASUS_ASSERT(myself != 0);

        // Set myself into thread specific storage
        // This will allow code to get its own Thread
        Thread::setCurrent(myself);

        ThreadPool *pool = (ThreadPool *) myself->get_parm();
        PEGASUS_ASSERT(pool != 0);

        Semaphore *sleep_sem = 0;
        struct timeval *lastActivityTime = 0;

        try
        {
            sleep_sem = (Semaphore *) myself->reference_tsd(TSD_SLEEP_SEM);
            myself->dereference_tsd();
            PEGASUS_ASSERT(sleep_sem != 0);

            lastActivityTime =
                (struct timeval *) myself->
                reference_tsd(TSD_LAST_ACTIVITY_TIME);
            myself->dereference_tsd();
            PEGASUS_ASSERT(lastActivityTime != 0);
        }
        catch (...)
        {
            PEG_TRACE_CSTRING(TRC_DISCARDED_DATA, Tracer::LEVEL1,
                "ThreadPool::_loop: Failure getting sleep_sem or "
                    "lastActivityTime.");
            pool->_idleThreads.remove(myself);
            pool->_currentThreads--;
            PEG_METHOD_EXIT();
            return (ThreadReturnType) 1;
        }

        while (1)
        {
            try
            {
                sleep_sem->wait();
            }
            catch (...)
            {
                PEG_TRACE_CSTRING(TRC_DISCARDED_DATA, Tracer::LEVEL1,
                    "ThreadPool::_loop: failure on sleep_sem->wait().");
                pool->_idleThreads.remove(myself);
                pool->_currentThreads--;
                PEG_METHOD_EXIT();
                return (ThreadReturnType) 1;
            }

            // When we awaken we reside on the _runningThreads queue, not the
            // _idleThreads queue.

            ThreadReturnType(PEGASUS_THREAD_CDECL * work) (void *) = 0;
            void *workParm = 0;
            Semaphore *blocking_sem = 0;

            try
            {
                work = (ThreadReturnType(PEGASUS_THREAD_CDECL *) (void *))
                    myself->reference_tsd(TSD_WORK_FUNC);
                myself->dereference_tsd();
                workParm = myself->reference_tsd(TSD_WORK_PARM);
                myself->dereference_tsd();
                blocking_sem =
                    (Semaphore *) myself->reference_tsd(TSD_BLOCKING_SEM);
                myself->dereference_tsd();
            }
            catch (...)
            {
                PEG_TRACE_CSTRING(TRC_DISCARDED_DATA, Tracer::LEVEL1,
                    "ThreadPool::_loop: Failure accessing work func, work "
                        "parm, or blocking sem.");
                pool->_idleThreads.remove(myself);
                pool->_currentThreads--;
                PEG_METHOD_EXIT();
                return (ThreadReturnType) 1;
            }

            if (work == 0)
            {
                PEG_TRACE_CSTRING(TRC_THREAD, Tracer::LEVEL4,
                    "ThreadPool::_loop: work func is 0, meaning we should "
                        "exit.");
                break;
            }

            Time::gettimeofday(lastActivityTime);

            try
            {
                PEG_TRACE_CSTRING(TRC_THREAD, Tracer::LEVEL4,
                                 "Work starting.");
                work(workParm);
                PEG_TRACE_CSTRING(TRC_THREAD, Tracer::LEVEL4,
                                 "Work finished.");
            }
            catch (Exception& e)
            {
                PEG_TRACE((TRC_DISCARDED_DATA, Tracer::LEVEL1,
                    "Exception from work in ThreadPool::_loop: %s",
                    (const char*)e.getMessage().getCString()));
            }
            catch (const exception& e)
            {
                PEG_TRACE((TRC_DISCARDED_DATA, Tracer::LEVEL1,
                    "Exception from work in ThreadPool::_loop: %s",e.what()));
            }
            catch (...)
            {
                PEG_TRACE_CSTRING(TRC_DISCARDED_DATA, Tracer::LEVEL1,
                    "Unknown exception from work in ThreadPool::_loop.");
            }

            // put myself back onto the available list
            try
            {
                Time::gettimeofday(lastActivityTime);
                if (blocking_sem != 0)
                {
                    blocking_sem->signal();
                }

                pool->_runningThreads.remove(myself);
                pool->_idleThreads.insert_front(myself);
            }
            catch (...)
            {
                PEG_TRACE_CSTRING(TRC_DISCARDED_DATA, Tracer::LEVEL1,
                    "ThreadPool::_loop: Adding thread to idle pool failed.");
                pool->_currentThreads--;
                PEG_METHOD_EXIT();
                return (ThreadReturnType) 1;
            }
        }
    }
    catch (const Exception & e)
    {
        PEG_TRACE((TRC_DISCARDED_DATA, Tracer::LEVEL1,
            "Caught exception: \"%s\".  Exiting _loop.",
            (const char*)e.getMessage().getCString()));
    }
    catch (...)
    {
        PEG_TRACE_CSTRING(TRC_DISCARDED_DATA, Tracer::LEVEL1,
            "Caught unrecognized exception.  Exiting _loop.");
    }

    PEG_METHOD_EXIT();
    return (ThreadReturnType) 0;
}

ThreadStatus ThreadPool::allocate_and_awaken(
    void* parm,
    ThreadReturnType (PEGASUS_THREAD_CDECL* work) (void*),
    Semaphore* blocking)
{
    PEG_METHOD_ENTER(TRC_THREAD, "ThreadPool::allocate_and_awaken");

    // Allocate_and_awaken will not run if the _dying flag is set.
    // Once the lock is acquired, ~ThreadPool will not change
    // the value of _dying until the lock is released.

    try
    {
        if (_dying.get())
        {
            PEG_TRACE_CSTRING(TRC_DISCARDED_DATA, Tracer::LEVEL3,
                "ThreadPool::allocate_and_awaken: ThreadPool is dying(1).");
            return PEGASUS_THREAD_UNAVAILABLE;
        }
        struct timeval start;
        Time::gettimeofday(&start);
        Thread *th = 0;

        th = _idleThreads.remove_front();

        if (th == 0)
        {
            if ((_maxThreads == 0) ||
                (_currentThreads.get() < Uint32(_maxThreads)))
            {
                th = _initializeThread();
            }
        }

        if (th == 0)
        {
            PEG_TRACE((TRC_THREAD, Tracer::LEVEL1,
                "ThreadPool::allocate_and_awaken: Insufficient resources: "
                    " pool = %s, running threads = %d, idle threads = %d",
                _key, _runningThreads.size(), _idleThreads.size()));
            return PEGASUS_THREAD_INSUFFICIENT_RESOURCES;
        }

        // initialize the thread data with the work function and parameters
        PEG_TRACE((TRC_THREAD, Tracer::LEVEL4,
            "Initializing thread(%s)"
                " with work function and parameters: parm = %p",
            Threads::id(th->getThreadHandle().thid).buffer,
            parm));

        th->delete_tsd(TSD_WORK_FUNC);
        th->put_tsd(TSD_WORK_FUNC, NULL,
                    sizeof (ThreadReturnType(PEGASUS_THREAD_CDECL *)
                            (void *)), (void *) work);
        th->delete_tsd(TSD_WORK_PARM);
        th->put_tsd(TSD_WORK_PARM, NULL, sizeof (void *), parm);
        th->delete_tsd(TSD_BLOCKING_SEM);
        if (blocking != 0)
            th->put_tsd(TSD_BLOCKING_SEM, NULL, sizeof (Semaphore *), blocking);

        // put the thread on the running list
        _runningThreads.insert_front(th);

        // signal the thread's sleep semaphore to awaken it
        Semaphore *sleep_sem = (Semaphore *) th->reference_tsd(TSD_SLEEP_SEM);
        PEGASUS_ASSERT(sleep_sem != 0);

        PEG_TRACE_CSTRING(TRC_THREAD, Tracer::LEVEL4,
            "Signal thread to awaken");
        sleep_sem->signal();
        th->dereference_tsd();
    }
    catch (...)
    {
        PEG_TRACE_CSTRING(TRC_DISCARDED_DATA, Tracer::LEVEL1,
                      "ThreadPool::allocate_and_awaken: Operation Failed.");
        PEG_METHOD_EXIT();
        // ATTN: Error result has not yet been defined
        return PEGASUS_THREAD_SETUP_FAILURE;
    }
    PEG_METHOD_EXIT();
    return PEGASUS_THREAD_OK;
}

// caller is responsible for only calling this routine during slack periods
// but should call it at least once per _deallocateWait interval.

Uint32 ThreadPool::cleanupIdleThreads()
{
    PEG_METHOD_ENTER(TRC_THREAD, "ThreadPool::cleanupIdleThreads");

    Uint32 numThreadsCleanedUp = 0;

    const Uint32 numIdleThreads = _idleThreads.size();
    for (Uint32 i = 0; i < numIdleThreads; i++)
    {
        // Do not dip below the minimum thread count
        if (_currentThreads.get() <= (Uint32) _minThreads)
        {
            break;
        }

        Thread *thread = _idleThreads.remove_back();

        // If there are no more threads in the _idleThreads queue, we're
        // done.
        if (thread == 0)
        {
            break;
        }

        void* tsd = thread->reference_tsd(TSD_LAST_ACTIVITY_TIME);
        struct timeval *lastActivityTime =
            reinterpret_cast<struct timeval*>(tsd);
        PEGASUS_ASSERT(lastActivityTime != 0);

        Boolean cleanupThisThread =
            _timeIntervalExpired(lastActivityTime, &_deallocateWait);
        thread->dereference_tsd();

        if (cleanupThisThread)
        {
            _cleanupThread(thread);
            _currentThreads--;
            numThreadsCleanedUp++;
        }
        else
        {
            _idleThreads.insert_front(thread);
        }
    }

    PEG_METHOD_EXIT();
    return numThreadsCleanedUp;
}

void ThreadPool::_cleanupThread(Thread * thread)
{
    PEG_METHOD_ENTER(TRC_THREAD, "ThreadPool::cleanupThread");

    // Set the TSD_WORK_FUNC and TSD_WORK_PARM to 0 so _loop() knows to exit.
    thread->delete_tsd(TSD_WORK_FUNC);
    thread->put_tsd(TSD_WORK_FUNC, 0,
                    sizeof (ThreadReturnType(PEGASUS_THREAD_CDECL *)
                            (void *)), (void *) 0);
    thread->delete_tsd(TSD_WORK_PARM);
    thread->put_tsd(TSD_WORK_PARM, 0, sizeof (void *), 0);

    // signal the thread's sleep semaphore to awaken it
    Semaphore *sleep_sem = (Semaphore *) thread->reference_tsd(TSD_SLEEP_SEM);
    PEGASUS_ASSERT(sleep_sem != 0);
    sleep_sem->signal();
    thread->dereference_tsd();

    thread->join();
    delete thread;

    PEG_METHOD_EXIT();
}

Boolean ThreadPool::_timeIntervalExpired(
    struct timeval* start,
    struct timeval* interval)
{
    PEGASUS_ASSERT(interval != 0);

    // never time out if the interval is zero
    if ((interval->tv_sec == 0) && (interval->tv_usec == 0))
    {
        return false;
    }

    struct timeval now, finish, remaining;
    Uint32 usec;
    Time::gettimeofday(&now);

    memset(&remaining, 0, sizeof(remaining));

    finish.tv_sec = start->tv_sec + interval->tv_sec;
    usec = start->tv_usec + interval->tv_usec;
    finish.tv_sec += (usec / 1000000);
    usec %= 1000000;
    finish.tv_usec = usec;

    return (Time::subtract(&remaining, &finish, &now) != 0);
}

void ThreadPool::_deleteSemaphore(void *p)
{
    delete(Semaphore *) p;
}

Thread *ThreadPool::_initializeThread()
{
    PEG_METHOD_ENTER(TRC_THREAD, "ThreadPool::_initializeThread");

    Thread *th = (Thread *) new Thread(_loop, this, false);

    // allocate a sleep semaphore and pass it in the thread context
    // initial count is zero, loop function will sleep until
    // we signal the semaphore
    Semaphore *sleep_sem = (Semaphore *) new Semaphore(0);
    th->put_tsd(
        TSD_SLEEP_SEM, &_deleteSemaphore, sizeof(Semaphore), (void*) sleep_sem);

    struct timeval* lastActivityTime =
        (struct timeval *)::operator  new(sizeof (struct timeval));
    Time::gettimeofday(lastActivityTime);

    th->put_tsd(
        TSD_LAST_ACTIVITY_TIME,
        thread_data::default_delete,
        sizeof(struct timeval),
        (void*) lastActivityTime);
    // thread will enter _loop() and sleep on sleep_sem until we signal it

    if (th->run() != PEGASUS_THREAD_OK)
    {
        PEG_TRACE((TRC_THREAD, Tracer::LEVEL1,
            "Could not create thread. Error code is %d.", errno));
        delete th;
        return 0;
    }
    _currentThreads++;

    PEG_METHOD_EXIT();
    return th;
}

void ThreadPool::_addToIdleThreadsQueue(Thread * th)
{
    if (th == 0)
    {
        PEG_TRACE_CSTRING(TRC_DISCARDED_DATA, Tracer::LEVEL1,
            "ThreadPool::_addToIdleThreadsQueue: Thread pointer is null.");
        throw NullPointer();
    }

    try
    {
        _idleThreads.insert_front(th);
    }
    catch (...)
    {
        PEG_TRACE_CSTRING(TRC_DISCARDED_DATA, Tracer::LEVEL1,
            "ThreadPool::_addToIdleThreadsQueue: _idleThreads.insert_front "
                "failed.");
    }
}

PEGASUS_NAMESPACE_END
