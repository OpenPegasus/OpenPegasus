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

/*
    Test of IPC functions including threads, condition variables, and
    MessageQueue.
    NOTE: This is a very limited test and not a comprehensive test of
          any of these functions
*/

#include <Pegasus/Common/Condition.h>
#include <Pegasus/Common/Thread.h>
#include <Pegasus/Common/Mutex.h>
#include <Pegasus/Common/MessageQueue.h>
#include <sys/types.h>
#include <Pegasus/Common/PegasusAssert.h>
#include <iostream>
#ifdef PEGASUS_OS_TYPE_WINDOWS
# include <windows.h>
#else
# include <unistd.h>
#endif

PEGASUS_USING_PEGASUS;
PEGASUS_USING_STD;

#define VCOUT if (verbose) cout

Boolean verbose = false;

// Fibonacci test parameter definition
class parmdef
{
public:
    parmdef()
    {
        th = NULL;
        cond_start = new Condition();
        mq = NULL;
        cond_mutex = new Mutex();
        id = 0;
        started = 0;
    }

    ~parmdef()
    {
        delete cond_start;
        delete cond_mutex;
    }

    int first;
    int second;
    int count;
    Thread * th;
    Condition * cond_start;
    MessageQueue * mq;
    Mutex * cond_mutex;
    Uint32 id;
    Boolean started;
};

String _addComma(Boolean x)
{
    return (x)? String(", ") : String("");
}

ThreadReturnType PEGASUS_THREAD_CDECL fibonacci(void * parm)
{
    // generate local version of parm variables
    Thread* my_thread = (Thread *)parm;
    parmdef * Parm = (parmdef *)my_thread->get_parm();

    int first = Parm->first;
    int second = Parm->second;
    int count = Parm->count;
    Condition * condstart = Parm->cond_start;
    MessageQueue * mq = Parm->mq;
    Mutex * cond_mutex = Parm->cond_mutex;
    Uint32 id = Parm->id;

    VCOUT << "Enter fibonacci thread " << id
          << " first = " << first << " second = " << second
          << " count = " << count
          << endl;

    // signal thread started
    cond_mutex->lock();
    Parm->started = true;
    condstart->signal();
    cond_mutex->unlock();

    int add_to_type = 0;
    if (count < 20)
        add_to_type = 100;

    // Create message for each fibonacci number calculated from first
    // for count numbers
    for (int i = 0; i < count; i++)
    {
        int sum = first + second;
        first = second;
        second = sum;

        VCOUT << _addComma(count != 0) << first;

        Message * message = new Message(static_cast<MessageType>(
            (i + add_to_type))
            , 0, sum);
        mq->enqueue(message);
    }
    VCOUT << endl;

    if (!add_to_type)
        Parm->th->thread_switch();

    return ThreadReturnType(0);
}

ThreadReturnType PEGASUS_THREAD_CDECL deq(void * parm)
{
    Thread* my_thread = (Thread *)parm;

    parmdef * Parm = (parmdef *)my_thread->get_parm();
    MessageType type;

    int first = Parm->first;
    int second = Parm->second;
    int count = Parm->count;
    VCOUT << first << " " << second << " " << count << endl;
    Condition * condstart = Parm->cond_start;
    MessageQueue * mq = Parm->mq;
    Mutex * cond_mutex = Parm->cond_mutex;
    Uint32 id = Parm->id;

    VCOUT << "Enter deq %u" << id << endl;

    // Signal that thread started
    cond_mutex->lock();
    Parm->started = true;
    condstart->signal();
    cond_mutex->unlock();

    Message * message;
    type = static_cast<MessageType>(0);

    while (type != CLOSE_CONNECTION_MESSAGE)
    {
        message = mq->dequeue();
        while (!message)
        {
            message = mq->dequeue();
        }

        type = message->getType();
        delete message;
    }
    if (verbose)
    {
        cout << "Received Cancel Message. id = "
              << id << " Ending." << endl;
    }

    return ThreadReturnType(0);
}

// Test Thread, MessageQueue, and Condition
int test01()
{
    MessageQueue * mq = new MessageQueue("testQueue");
    parmdef * parm[4];

    for (int i = 0; i < 4;i++)
    {
        parm[i] = new parmdef();
        parm[i]->mq = mq;
        parm[i]->id = i;
    }

    // instrument Parm for 20 Fibonacci numbers starting at 0
    parm[0]->first = 0;
    parm[0]->second = 1;
    parm[0]->count = 20;

    // Instrument Parm for 10 numbers starting at 4
    parm[3]->first = 4;
    parm[3]->second = 6;
    parm[3]->count = 10;

    parm[0]->th = new Thread(fibonacci, parm[0], false);
    parm[1]->th = new Thread(deq, parm[1], false);
    parm[2]->th = new Thread(deq, parm[2], false);
    parm[3]->th = new Thread(fibonacci, parm[3], false);

    // Start all 4 threads
    for (int i = 0; i < 4;i++)
    {
       //parm[i]->cond_start->wait();
       parm[i]->th->run();
    }

    // Let the threads start and wait for Start Condition to be signaled
    // on all threads
    for (int i = 0; i < 4;i++)
    {
        parm[i]->cond_mutex->lock();
        while (parm[i]->started == false)
        {
            parm[i]->cond_start->wait(*parm[i]->cond_mutex);
        }
        parm[i]->cond_mutex->unlock();
    }

    // all started successfully

    // Finish the enqueueing tasks
    parm[0]->th->join();
    parm[3]->th->join();

    // Tell one of the dequeueing tasks to finish
    Message * message;
    message = new Message(CLOSE_CONNECTION_MESSAGE, 0);
    mq->enqueue(message);

    // Tell the other dequeueing task to finish
    message = new Message(CLOSE_CONNECTION_MESSAGE, 0);
    mq->enqueue(message);

    // Finish the dequeueing tasks
    parm[1]->th->join();
    parm[2]->th->join();

    // Clean up
    for (int i = 0; i < 4; i++)
    {
        delete parm[i]->th;
        delete parm[i];
    }

    delete mq;

    return 0;
}

ThreadReturnType PEGASUS_THREAD_CDECL atomicIncrement(void * parm)
{
    Thread* my_thread  = (Thread *)parm;
    AtomicInt * atom = (AtomicInt *)my_thread->get_parm();

    (*atom)++;
    (*atom)++; (*atom)++; (*atom)++; (*atom)++;
    (*atom)--; (*atom)--;
    (*atom)--;
    Boolean zero = atom->decAndTestIfZero();
    PEGASUS_TEST_ASSERT(zero == false);

    return ThreadReturnType(0);
}

// Test Thread and AtomicInt
void test02()
{
    const Uint32 numThreads = 64;
    AtomicInt * atom = new AtomicInt(0);
    Thread* threads[numThreads];

    VCOUT << "Enter test02" << endl;

    (*atom)++;
    Boolean zero = atom->decAndTestIfZero();
    PEGASUS_TEST_ASSERT(zero);

    for (Uint32 i=0; i<numThreads; i++)
    {
        threads[i] = new Thread(atomicIncrement, atom, false);
    }

    for (Uint32 i=0; i<numThreads; i++)
    {
        threads[i]->run();
    }

    for (Uint32 i=0; i<numThreads; i++)
    {
        threads[i]->join();
        delete threads[i];
    }

    PEGASUS_TEST_ASSERT(atom->get() == numThreads);
    delete atom;
}

int main(int argc, char** argv)
{
    verbose = (getenv("PEGASUS_TEST_VERBOSE")) ? true : false;

    Uint32 LoopCount = 10;

    for (Uint32 loop = 0; loop < LoopCount; loop++)
    {
        test01();
    }

        VCOUT << "+++++ passed test 1" << endl;

    for (Uint32 loop = 0; loop < LoopCount; loop++)
    {
        test02();
    }

        VCOUT << "+++++ passed test 2" << endl;

    cout << argv[0] << " +++++ passed all tests" << endl;

    return 0;
}
