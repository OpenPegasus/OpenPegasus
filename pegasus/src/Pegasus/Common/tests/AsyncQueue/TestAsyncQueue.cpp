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

#include <Pegasus/Common/PegasusAssert.h>
#include <Pegasus/Common/AsyncQueue.h>
#include <Pegasus/Common/Thread.h>
#include <iostream>

PEGASUS_USING_STD;
PEGASUS_USING_PEGASUS;

PEGASUS_NAMESPACE_BEGIN

const Uint32 ITERATIONS = 100000;
Boolean verbose = false;

struct TestMessage : public Linkable
{
    TestMessage(Uint32 x_) : x(x_) { }
    Uint32 x;
};

typedef AsyncQueue<TestMessage> Queue;

static ThreadReturnType PEGASUS_THREAD_CDECL _reader(void* self_)
{
    Thread* self = (Thread*)self_;
    Queue* queue = (Queue*)self->get_parm();

    for (Uint32 i = 0; i < ITERATIONS; i++)
    {
        TestMessage* message = queue->dequeue_wait();
        PEGASUS_TEST_ASSERT(message);

        if (verbose)
        {
            if (((i + 1) % 1000) == 0)
            printf("iterations: %05u\n", message->x);
        }

// The following was a noted issue for earlier versions of
// Solaris (ex. 5.8) It has been commented out for later
// version (ex. 10 and 11). NOTE: if we are to guarantee
// compatibility with earlier versions this may have to be
// reinstated for those versions.
//#ifdef PEGASUS_OS_SOLARIS
        // special dish of the day for Sun Solaris
        // reports say that running as root causes
        // the thread not being scheduled-out
        // until this is resolved the yield()
        // will stay here just for Solaris
//        Threads::yield();
//#endif

        delete message;
    }

    return ThreadReturnType(0);
}

static ThreadReturnType PEGASUS_THREAD_CDECL _writer(void* self_)
{
    Thread* self = (Thread*)self_;
    Queue* queue = (Queue*)self->get_parm();

    for (Uint32 i = 0; i < ITERATIONS; i++)
    {
        queue->enqueue(new TestMessage(i));
// The following was a noted issue for earlier versions of
// Solaris (ex. 5.8) It has been commented out for later
// version (ex. 10 and 11). NOTE: if we are to guarantee
// compatibility with earlier versions this may have to be
// reinstated for those versions.
// special dish of the day for Sun Solaris
// reports say that running as root causes
// the thread not being scheduled-out
// until this is resolved the yield()
// will stay here just for Solaris
//#ifdef PEGASUS_OS_SOLARIS
//        Threads::yield();
//#endif
    }

    return ThreadReturnType(0);
}

void testAsyncQueue()
{
    AsyncQueue<TestMessage> queue;

    Thread reader(_reader, &queue, false);
    Thread writer(_writer, &queue, false);
    reader.run();
    writer.run();

    reader.join();
    writer.join();
}

PEGASUS_NAMESPACE_END

int main(int, char **argv)
{
    verbose = (getenv("PEGASUS_TEST_VERBOSE")) ? true : false;
    try
    {
        testAsyncQueue();
    }
    catch (Exception& e)
    {
        cerr << argv[0] << " Exception: " << e.getMessage() << endl;
        exit(1);
    }

    cout << argv[0] << " +++++ passed all tests" << endl;
    return 0;
}
