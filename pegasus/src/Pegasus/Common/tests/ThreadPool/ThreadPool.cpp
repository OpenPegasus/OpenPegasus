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

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/Thread.h>
#include <Pegasus/Common/ThreadPool.h>
#include <Pegasus/Common/Tracer.h>

#include <sys/types.h>
#if !defined(PEGASUS_OS_TYPE_WINDOWS)
# include <unistd.h>
#endif
#include <Pegasus/Common/PegasusAssert.h>
#include <iostream>
#include <stdio.h>
#include <string.h>

PEGASUS_USING_STD;
PEGASUS_USING_PEGASUS;

Boolean verbose = false;

ThreadReturnType PEGASUS_THREAD_CDECL funcSleepUntilCancelled(
    void* parm)
{
    AtomicInt* cancelled = static_cast<AtomicInt*>(parm);

    while (cancelled->get() == 0)
    {
        Threads::sleep(1);
    }

    return 0;
}

ThreadReturnType PEGASUS_THREAD_CDECL funcSleepSpecifiedMilliseconds(
    void* parm)
{
#ifdef PEGASUS_POINTER_64BIT
    Uint32 sleepMilliseconds = (Uint64)parm;
#elif defined(PEGASUS_PLATFORM_AIX_RS_IBMCXX) \
    || defined(PEGASUS_PLATFORM_PASE_ISERIES_IBMCXX)
    unsigned long sleepMilliseconds = (unsigned long)parm;
#else
    Uint32 sleepMilliseconds = (Uint32)parm;
#endif

    Threads::sleep(sleepMilliseconds);

    return 0;
}

ThreadReturnType PEGASUS_THREAD_CDECL funcIncrementCounter(
    void* parm)
{
    AtomicInt* counter = static_cast<AtomicInt*>(parm);

    (*counter)++;
    Threads::sleep(50);

    return 0;
}

ThreadReturnType PEGASUS_THREAD_CDECL funcThrow(void* parm)
{
    throw Uint32(10);
    PEGASUS_UNREACHABLE(return 0;)
}

void testDestructAsThreadCompletes()
{
    AtomicInt cancelled(0);

    struct timeval deallocateWait = {0, 0};
    ThreadPool* threadPool = new ThreadPool(0, "Tester", 0, 1, deallocateWait);

    threadPool->allocate_and_awaken(&cancelled, funcSleepUntilCancelled);

    cancelled = 1;

    delete threadPool;
}

void testloopDestructAsThreadCompletes()
{
    if (verbose)
    {
        cout << "testloopDestructAsThreadCompletes" << endl;
    }

    try
    {
        int done = 0;
        const int limit = 10000;
        const int display = limit / 10;
        while (done < limit)
        {
            if (verbose && (done % display == 0))
            {
                printf("testDestructAsThreadCompletes: iteration %d of %d\n",
                    done+1, limit);
            }
            testDestructAsThreadCompletes();
            done++;
        }
    }
    catch (const Exception& e)
    {
        cout << "Exception in testloopDestructAsThreadCompletes: " <<
            e.getMessage() << endl;
        PEGASUS_TEST_ASSERT(false);
    }
}

void testCleanupIdleThread()
{
    if (verbose)
    {
        cout << "testCleanupIdleThread" << endl;
    }

    try
    {
        struct timeval deallocateWait = { 0, 1 };
        ThreadPool threadPool(0, "test cleanup", 0, 6, deallocateWait);

        threadPool.allocate_and_awaken(
            (void*)1, funcSleepSpecifiedMilliseconds);
        Threads::sleep(1000);

        PEGASUS_TEST_ASSERT(threadPool.idleCount() == 1);
        threadPool.cleanupIdleThreads();
        PEGASUS_TEST_ASSERT(threadPool.idleCount() == 0);
    }
    catch (const Exception& e)
    {
        cout << "Exception in testCleanupIdleThread: " <<
            e.getMessage() << endl;
        PEGASUS_TEST_ASSERT(false);
    }
}

void testDestructWithRunningThreads()
{
    if (verbose)
    {
        cout << "testDestructWithRunningThreads" << endl;
    }

    try
    {
        struct timeval deallocateWait = { 0, 1 };
        ThreadPool threadPool(0, "test destruct", 0, 0, deallocateWait);

        threadPool.allocate_and_awaken(
            (void*)100, funcSleepSpecifiedMilliseconds);
        threadPool.allocate_and_awaken(
            (void*)200, funcSleepSpecifiedMilliseconds);
        threadPool.allocate_and_awaken(
            (void*)300, funcSleepSpecifiedMilliseconds);
        threadPool.allocate_and_awaken(
            (void*)400, funcSleepSpecifiedMilliseconds);
        threadPool.allocate_and_awaken(
            (void*)500, funcSleepSpecifiedMilliseconds);
        threadPool.allocate_and_awaken(
            (void*)600, funcSleepSpecifiedMilliseconds);
        threadPool.allocate_and_awaken(
            (void*)700, funcSleepSpecifiedMilliseconds);
        threadPool.allocate_and_awaken(
            (void*)800, funcSleepSpecifiedMilliseconds);
        threadPool.allocate_and_awaken(
            (void*)900, funcSleepSpecifiedMilliseconds);

        PEGASUS_TEST_ASSERT(threadPool.runningCount() > 0);
    }
    catch (const Exception& e)
    {
        cout << "Exception in testDestructWithRunningThreads: " <<
            e.getMessage() << endl;
        PEGASUS_TEST_ASSERT(false);
    }
}

void testOverloadPool()
{
    if (verbose)
    {
        cout << "testOverloadPool" << endl;
    }

    try
    {
        struct timeval deallocateWait = { 0, 1 };
        ThreadPool threadPool(0, "test overload", 0, 4, deallocateWait);
        ThreadStatus threadStarted;

        threadStarted = threadPool.allocate_and_awaken(
            (void*)3000, funcSleepSpecifiedMilliseconds);
        PEGASUS_TEST_ASSERT(threadStarted == PEGASUS_THREAD_OK);

        threadStarted = threadPool.allocate_and_awaken(
            (void*)3000, funcSleepSpecifiedMilliseconds);
        PEGASUS_TEST_ASSERT(threadStarted == PEGASUS_THREAD_OK);

        threadStarted = threadPool.allocate_and_awaken(
            (void*)3000, funcSleepSpecifiedMilliseconds);
        PEGASUS_TEST_ASSERT(threadStarted == PEGASUS_THREAD_OK);

        threadStarted = threadPool.allocate_and_awaken(
            (void*)3000, funcSleepSpecifiedMilliseconds);
        PEGASUS_TEST_ASSERT(threadStarted == PEGASUS_THREAD_OK);

        threadStarted = threadPool.allocate_and_awaken(
            (void*)300, funcSleepSpecifiedMilliseconds);
        PEGASUS_TEST_ASSERT(threadStarted ==
                PEGASUS_THREAD_INSUFFICIENT_RESOURCES);

        ThreadStatus rc = PEGASUS_THREAD_OK;
        while ( (rc =threadPool.allocate_and_awaken(
            (void*)100, funcSleepSpecifiedMilliseconds)) != PEGASUS_THREAD_OK)
        {
          if (rc ==PEGASUS_THREAD_INSUFFICIENT_RESOURCES)
            Threads::yield();
          else
           throw Exception("Could not allocate and awaken a thread.");
        }
    }
    catch (const Exception& e)
    {
        cout << "Exception in testOverloadPool: " << e.getMessage() << endl;
        PEGASUS_TEST_ASSERT(false);
    }
}

void testHighWorkload()
{
    if (verbose)
    {
        cout << "testHighWorkload" << endl;
    }

    try
    {
        AtomicInt counter(0);

        struct timeval deallocateWait = { 0, 1 };
        ThreadPool* threadPool =
            new ThreadPool(0, "test workload", 0, 10, deallocateWait);

        for (Uint32 i = 0; i < 50; i++)
        {
            ThreadStatus rc = PEGASUS_THREAD_OK;
            while ( (rc =threadPool->allocate_and_awaken(
                &counter, funcIncrementCounter)) != PEGASUS_THREAD_OK)
            {
                if (rc == PEGASUS_THREAD_INSUFFICIENT_RESOURCES)
                {
                    Threads::yield();
                }
                else
                {
                    throw Exception("Could not allocate a thread for counter");
                }
            }
        }

        delete threadPool;

        PEGASUS_TEST_ASSERT(counter.get() == 50);
    }
    catch (const Exception& e)
    {
        cout << "Exception in testHighWorkload: " << e.getMessage() << endl;
        PEGASUS_TEST_ASSERT(false);
    }
}

void testWorkException()
{
    if (verbose)
    {
        cout << "testWorkException" << endl;
    }

    try
    {
        struct timeval deallocateWait = { 0, 1 };
        ThreadPool threadPool(0, "test exception", 0, 6, deallocateWait);

        threadPool.allocate_and_awaken((void*)1, funcThrow);
        Threads::sleep(100);
    }
    catch (const Exception& e)
    {
        cout << "Exception in testWorkException: " << e.getMessage() << endl;
        PEGASUS_TEST_ASSERT(false);
    }
}

void testBlockingThread()
{
    if (verbose)
    {
        cout << "testBlockingThread" << endl;
    }

    try
    {
        struct timeval deallocateWait = { 5, 0 };
        ThreadPool threadPool(0, "test blocking", 0, 6, deallocateWait);
        Semaphore blocking(0);
        ThreadStatus rt = PEGASUS_THREAD_OK;
        while ( (rt =threadPool.allocate_and_awaken(
            (void*)16, funcSleepSpecifiedMilliseconds, &blocking)) !=
                PEGASUS_THREAD_OK)
        {
            if (rt == PEGASUS_THREAD_INSUFFICIENT_RESOURCES)
            {
                    Threads::yield();
            }
            else
            {
                    throw Exception("Could not allocate thread for"
                        " funcSleepSpecifiedMilliseconds function.");
            }
        }

        blocking.wait();
        threadPool.cleanupIdleThreads();
    }
    catch (const Exception& e)
    {
        cout << "Exception in testBlockingThread: " << e.getMessage() << endl;
        PEGASUS_TEST_ASSERT(false);
    }
}

int main(int, char **argv)
{
    verbose = (getenv("PEGASUS_TEST_VERBOSE")) ? true : false;

#if defined(PEGASUS_DEBUG)
    if (verbose)
    {
        Tracer::setTraceComponents("ALL");
        Tracer::setTraceLevel(Tracer::LEVEL4);
        Tracer::setTraceFile("thread_pool.trc");
    }
#endif

    testCleanupIdleThread();
    testDestructWithRunningThreads();
    testOverloadPool();
    testWorkException();
    testHighWorkload();
    testBlockingThread();

#if defined(PEGASUS_DEBUG)
    if (verbose)
    {
        Tracer::setTraceComponents("");
    }
#endif

    testloopDestructAsThreadCompletes();

    cout << argv[0] << " +++++ passed all tests" << endl;

    return(0);
}
