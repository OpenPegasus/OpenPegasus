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

#ifndef Pegasus_Condition_h
#define Pegasus_Condition_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/Linkage.h>
#include <Pegasus/Common/Mutex.h>
#include <Pegasus/Common/AtomicInt.h>

#if defined(PEGASUS_HAVE_PTHREADS)
# include <pthread.h>
#elif defined(PEGASUS_HAVE_WINDOWS_THREADS)
# include <windows.h>
# include "List.h"
#else
# error "<Pegasus/Common/Condition.h>: not implemented"
#endif

PEGASUS_NAMESPACE_BEGIN

//==============================================================================
//
// ConditionRep
//
//==============================================================================

#if defined(PEGASUS_HAVE_PTHREADS)

struct ConditionRep
{
    pthread_cond_t cond;
};

#endif /* PEGASUS_HAVE_PTHREADS */

#if defined(PEGASUS_HAVE_WINDOWS_THREADS)

struct ConditionWaiter : public Linkable
{
    HANDLE event;
};

struct ConditionRep
{
    List<ConditionWaiter,Mutex> waiters;
};

#endif /* PEGASUS_HAVE_WINDOWS_THREADS */

//==============================================================================
//
// Condition
//
//==============================================================================

/**
 * This class defines a condition variable based on the
 * Pthreads model that allows signaling between threads for more complex
 * conditions than semaphores.
 * The general pattern for a condition variable is:
 * 1. Define the condition variable and corresponding Mutex
 *     Condition testCondition;
 *     Mutex testCondtionMutex;
 *
 * 2. Define the condition wait logic as a while loop and protect it with
 * the mutex.
 *
 *     testConditionMutex.lock();
 *     while (testdata < 3 && !endOfTransaction)
 *     {
 *         testCondition.wait(testConditionMutex)
 *     }
 *     testConditionMutex.unlock();
 *
 * If the condition is true, execution continues.  If not, the wait is
 * executed which unlocks the mutex and goes to sleep to wait for a signal
 * on the definedcondition variable from another thread. Pthreads
 * DOES not guarantee that there will not be spurious signals so the
 * condition wait loop MUST test for the condition.
 * . . .
 * 3. As part of some thread, define the signal function as follows.
 * When the following signal code is executed, the condition variable in
 * one thread in the wait condition will be  awakened.
 *
 *     testConditionMutex.lock();
 *     testCondition.signal();
 *     testCondition.unlock();
 *
 * The user should be careful that the conditions are only modified under
 * control of the mutex associated with the condition variable.
 */

class PEGASUS_COMMON_LINKAGE Condition
{
public:

    /**
     * Define a condition variable.
     *
     */
    Condition();

    ~Condition();

    /**
     * Signal the condition variable that the condition might be
     * satisfied.  The signal should always be protected by the
     * condition mutex and need not fully test the condition when
     * executed.
     */
    void signal();

    /**
     *  Wait on the signal from another thread for the defined
     *  condition variable. The wait will return when a signal is
     *  received. The wait function should always be protected by
     *  the condition mutex.  Each time the wait is executed it
     *  unlocks the mutex, sleeps awating a signal. When a signal is
     *  received, the mutex is locked.
     *
     *  @param mutex - Condition variable mutex that should protect
     *  both the wait and signal execution.
     */
    void wait(Mutex& mutex);

private:
    Condition(const Condition&);
    Condition& operator=(const Condition&);
    ConditionRep _rep;
};

PEGASUS_NAMESPACE_END

#endif /* Pegasus_Condition_h */
