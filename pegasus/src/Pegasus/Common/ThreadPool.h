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
//%////////////////////////////////////////////////////////////////////////////

#ifndef Pegasus_ThreadPool_h
#define Pegasus_ThreadPool_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/Linkage.h>
#include <Pegasus/Common/Thread.h>

PEGASUS_NAMESPACE_BEGIN

class PEGASUS_COMMON_LINKAGE ThreadPool
{
public:

    /**
        Constructs a new ThreadPool object.
        @param initialSize The number of threads that are initially added to
            the thread pool.
        @param key A name for this thread pool that can be used to determine
            equality of two thread pool objects.  Only the first 16 characters
            of this value are used.
        @param minThreads The minimum number of threads that should be
            contained in this thread pool at any given time.
        @param maxThreads The maximum number of threads that should be
            contained in this thread pool at any given time.
        @param deallocateWait The minimum time that a thread should be idle
            before it is removed from the pool and cleaned up.
     */
    ThreadPool(
        Sint16 initialSize,
        const char* key,
        Sint16 minThreads,
        Sint16 maxThreads,
        struct timeval& deallocateWait);

    /**
        Destructs the ThreadPool object.
     */
     ~ThreadPool();

    /**
        Allocate and start a thread to do a unit of work.
        @param parm A generic parameter to pass to the thread
        @param work A pointer to the function that is to be executed by
                    the thread
        @param blocking A pointer to an optional semaphore which, if
                        specified, is signaled after the thread finishes
                        executing the work function
        @return PEGASUS_THREAD_OK if the thread is started successfully,
                PEGASUS_THREAD_INSUFFICIENT_RESOURCES  if the
                resources necessary to start the thread are not currently
                available.  PEGASUS_THREAD_SETUP_FAILURE if the thread
                could not be setup properly. PEGASUS_THREAD_UNAVAILABLE
                if this service is shutting down and no more threads can
                be allocated.
     */
    ThreadStatus allocate_and_awaken(
        void* parm,
        ThreadReturnType(PEGASUS_THREAD_CDECL* work) (void*),
        Semaphore* blocking = 0);

    /**
        Cleans up idle threads if they have been running longer than the
        deallocate_wait configuration and more than the configured
        minimum number of threads is running.
        @return The number of threads that were cleaned up.
     */
    Uint32 cleanupIdleThreads();

    void get_key(Sint8* buf, int bufsize);

    inline void setMinThreads(Sint16 min)
    {
        _minThreads = min;
    }

    inline Sint16 getMinThreads() const
    {
        return _minThreads;
    }

    inline void setMaxThreads(Sint16 max)
    {
        _maxThreads = max;
    }

    inline Sint16 getMaxThreads() const
    {
        return _maxThreads;
    }

    inline Uint32 runningCount()
    {
        return (Uint32) _runningThreads.size();
    }

    inline Uint32 idleCount()
    {
        return (Uint32) _idleThreads.size();
    }

private:

    ThreadPool();               // Unimplemented
    ThreadPool(const ThreadPool &);     // Unimplemented
    ThreadPool & operator=(const ThreadPool &); // Unimplemented

    static ThreadReturnType PEGASUS_THREAD_CDECL _loop(void *);

    static Boolean _timeIntervalExpired(
        struct timeval* start,
        struct timeval* interval);

    static void _deleteSemaphore(void* p);

    void _cleanupThread(Thread* thread);
    Thread* _initializeThread();
    void _addToIdleThreadsQueue(Thread* th);

    Sint16 _maxThreads;
    Sint16 _minThreads;
    AtomicInt _currentThreads;
    struct timeval _deallocateWait;
    char _key[17];
    List<Thread, Mutex> _idleThreads;
    List<Thread, Mutex> _runningThreads;
    AtomicInt _dying;
};

PEGASUS_NAMESPACE_END

#endif // Pegasus_ThreadPool_h
