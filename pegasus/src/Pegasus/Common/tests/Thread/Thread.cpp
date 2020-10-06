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

#include <Pegasus/Common/Thread.h>
#include <Pegasus/Common/ReadWriteSem.h>
#include <Pegasus/Common/Condition.h>
#include <sys/types.h>
#if defined(PEGASUS_OS_TYPE_WINDOWS)
#else
#include <unistd.h>
#endif
#if defined(PEGASUS_OS_HPUX) || defined(PEGASUS_OS_LINUX)
# include <memory>
#endif
#include <Pegasus/Common/PegasusAssert.h>
#include <iostream>
#include <stdio.h>
#include <string.h>

PEGASUS_USING_PEGASUS;
PEGASUS_USING_STD;

Boolean die = false;

//ThreadReturnType PEGASUS_THREAD_CDECL test3_thread( void* parm );

AtomicInt testval1(0);

Boolean verbose = false;

////////////////////////////////////////////////////////////////////
//
// testDeadlocThread()
//
// ////////////////////////////////////////////////////////////////
ThreadReturnType PEGASUS_THREAD_CDECL testDeadlockThread( void* parm );
static Mutex deadLockSemaphore(Mutex::NON_RECURSIVE);
static Condition deadLockCondition;

void testCancelDeadLockedThread(const char * commandName)
{
    // Test deadlocked thread handling
    Thread t(testDeadlockThread, 0, false);
    t.run();
    // give the deadlock thread time to actually lock and wait for condition
    // signal
    Threads::sleep(1000);
    // cancel the deadlocked thread
    t.cancel();
    if (verbose)
    {
        cout << commandName
             << " - If this hangs here, there is a thread deadlock"
                   " handling bug..."
             << endl;
    }
    // Wait for deadlocked thread to end
    t.join();
    // Shouldn't hang forever
    if (verbose)
    {
        cout << commandName << " - Deadlock test finished." << endl;
    }
}


// The following thread trys to get the lock on an already reserved semaphore
// means the thread will just deadlock and wait
ThreadReturnType PEGASUS_THREAD_CDECL testDeadlockThread( void* parm )
{
    // Lock the semaphore the deadlocked thread will wait for
    if (verbose) cout << "DeadLock Thread going to lock Semaphore" << endl;
    deadLockSemaphore.lock();
    if (verbose) cout << "DeadLock Thread waiting for Condition" << endl;
    deadLockCondition.wait(deadLockSemaphore);
    if (verbose) cout << "DeadLock Thread got Semaphore free signal" << endl;
    if (verbose) cout << "This should not ever happen..." << endl;
    abort();
    return ThreadReturnType(52);
}

//////////////////////////////////////////////////////////////////////////
// Test run, join, sleep,type return for a single thread
ThreadReturnType PEGASUS_THREAD_CDECL test1_thread( void* parm );

void testOneThread()
{
    if (verbose)
    {
        cout << "testOneThread" << endl;
    }
    // Test return code
    Thread t( test1_thread, 0, false );
    if (t.run()!=PEGASUS_THREAD_OK)
    {
        cerr << "Error. Thread Run returned Error "
            << endl;
        abort();
    }

    t.join();
    if( t.get_exit() != (ThreadReturnType)32 )
    {
        cerr << "Error test return code mismatch" << endl;
        abort();
    }
    // TODO: Programatically check
    //Threads::sleep( 10000 );
}

// Thread execution function for TestOneThread()
ThreadReturnType PEGASUS_THREAD_CDECL test1_thread( void* parm )
{
    Threads::sleep( 1000 );
    return ThreadReturnType(32);
}

///////////////////////////////////////////////////////////////////
//
// Test multiple threads including TSD
//
///////////////////////////////////////////////////

// define number of threads to create
#define  THREAD_NR 500

struct TestThreadData
{
    char chars[2];
    Uint32 lInteger;
    Array<Uint32> lArray;
};

void deleteTestThreadData(void* data)
{
    delete reinterpret_cast<TestThreadData*>(data);
}

// thread function definition for testMultipleThreads
ThreadReturnType PEGASUS_THREAD_CDECL testMultipleThread( void* parm );

void testMultipleThreads()
{
    if (verbose)
    {
        cout << "testMultipleThreads" << endl;
    }
    // Test creating THREAD_NR Threads with
    // thread specific data.
    Thread* threads[THREAD_NR];
    int max_threads = THREAD_NR;
    for( int i = 0; i < THREAD_NR; i++ )
    {
        threads[i] = new Thread( testMultipleThread, 0, false );
        TestThreadData* data = NULL;
        try {
            data = new struct TestThreadData;
        }
        // if there was a bad allocation, test with reduced
        // number of threads
        catch (bad_alloc&)
        {
            cerr << "Not enough memory. Reducing Number of threads used to "
                 << i << "."
                << endl;
            max_threads = i;
            delete threads[i];
            delete data;
            break;
        }
        data->chars[0] = 'B';
        data->chars[1] = 'E';
        data->lInteger = 3456;
        data->lArray.append(1);
        data->lArray.append(9999);

        threads[i]->put_tsd(TSD_RESERVED_1, deleteTestThreadData, 2, data);
        if (threads[i]->run()!=PEGASUS_THREAD_OK)
        {
            cerr << "Not enough memory. Reducing Number of threads used to "
                << i << "."
                << endl;
            max_threads = i;
            delete threads[i];
            break;
        }
    }
    for( int i = 0; i < max_threads; i++ )
    {
            threads[i]->join();
            if( threads[i]->get_exit() != (ThreadReturnType)32 )
            {
                cerr << "Error test return code mismatch" << endl;
                exit (1);
            }
    }
    for( int i = 0; i < max_threads; i++ )
        delete threads[i];
}

// Thread function for testMultipleThreads. simply tests the
// thread local data validity and derefences the data
//
ThreadReturnType PEGASUS_THREAD_CDECL testMultipleThread( void* parm )
{
    Thread* thread = (Thread*)parm;
    TestThreadData* data = (TestThreadData*)thread->reference_tsd(
        TSD_RESERVED_1);
    PEGASUS_TEST_ASSERT (data != NULL);

    PEGASUS_TEST_ASSERT (data->chars[0] == 'B');
    PEGASUS_TEST_ASSERT (data->chars[1] == 'E');
    PEGASUS_TEST_ASSERT (data->lInteger == 3456);
    PEGASUS_TEST_ASSERT (data->lArray.size() == 2);
    PEGASUS_TEST_ASSERT (data->lArray[0] == 1);
    PEGASUS_TEST_ASSERT (data->lArray[1] == 9999);

    thread->dereference_tsd();

    return ThreadReturnType(32);
}

//////////////////////////////////////////////////////////////////////////
//
//  testReadWriteThreads
//
/////////////////////////////////////////////////////////////////////////

AtomicInt read_count ;
AtomicInt write_count ;

#define READER_COUNT 40
#define WRITER_COUNT 10

ThreadReturnType PEGASUS_THREAD_CDECL reading_thread(void *parm);
ThreadReturnType PEGASUS_THREAD_CDECL writing_thread(void *parm);

void testReadWriteThreads()
{
    if (verbose)
    {
        cout << "testReadWriteThreads" << endl;
    }
    ReadWriteSem *rwSem = new ReadWriteSem();
    Thread *readers[READER_COUNT];
    Thread *writers[WRITER_COUNT];

    for(int i = 0; i < READER_COUNT; i++)
    {
       readers[i] = new Thread(reading_thread, rwSem, false);
       readers[i]->run();
    }

    for( int i = 0; i < WRITER_COUNT; i++)
    {
       writers[i] = new Thread(writing_thread, rwSem, false);
       writers[i]->run();
    }
    Threads::sleep(20000);
    die = true;

    for( int i = 0; i < 40; i++)
    {
      readers[i]->join();
       delete readers[i];
    }

    for( int i = 0; i < 10; i++)
    {
       writers[i]->join();
       delete writers[i];
    }

    delete rwSem;
    if (verbose)
    {
        cout << endl
             << "read operations: " << read_count.get() << endl
             << "write operations: " << write_count.get() << endl;
    }
}
void exit_one(void *parm)
{

   if (verbose)
       cout << "1";
}

void exit_two(void *parm)
{

   if (verbose)
       cout << "2";
}

void deref(void *parm)
{
   Thread *my_handle = (Thread *)parm;
   try
   {
      my_handle->dereference_tsd();
   }
   catch(...)
   {
      cout << "exception dereferencing the tsd " << endl;
      abort();
   }
   return;
}


ThreadReturnType PEGASUS_THREAD_CDECL reading_thread(void *parm)
{
   Thread *my_handle = (Thread *)parm;
   ReadWriteSem * my_parm = (ReadWriteSem *)my_handle->get_parm();

   ThreadType myself = Threads::self();

   if (verbose)
       cout << "r";

   const TSD_Key keys[] =
   {
      TSD_RESERVED_1,
      TSD_RESERVED_2,
      TSD_RESERVED_3,
      TSD_RESERVED_4,
   };

   try
   {
      my_handle->cleanup_push(exit_one , my_handle );
   }
   catch (...)
   {
      cout << "Exception while trying to push cleanup handler" << endl;
      abort();
   }

   try
   {
      my_handle->cleanup_push(exit_two , my_handle );
   }

   catch (...)
   {
      cout << "Exception while trying to push cleanup handler" << endl;
      abort();
   }

   while(die == false)
   {
      int i = 0;

#ifndef PEGASUS_OS_ZOS
      char *my_storage = (char *)calloc(256, sizeof(char));
#else
      char *my_storage = (char *)::operator new(256);
#endif
      //    sprintf(my_storage, "%ld", myself + i);
      try
      {
#ifndef PEGASUS_OS_ZOS
         my_handle->put_tsd(keys[i % 4], free, 256, my_storage);
#else
         my_handle->put_tsd(keys[i % 4], ::operator delete,
                            256, my_storage);
#endif
      }
      catch (...)
      {
          cout << "Exception while trying to put local storage: "
              << Threads::id(myself).buffer << endl;
          abort();
      }

      try
      {
          my_parm->waitRead();
      }
      catch (...)
      {
         cout << "Exception while trying to get a read lock" << endl;
         abort();
      }

      read_count++;
      //if (verbose)
      //    cout << "+";
      my_handle->sleep(1);

      try
      {
          my_handle->cleanup_push(deref , my_handle );
      }
      catch (...)
      {
         cout << "Exception while trying to push cleanup handler" << endl;
         abort();
      }

      try
      {
         my_handle->reference_tsd(keys[i % 4]);
      }

      catch (...)
      {
         cout << "Exception while trying to reference local storage" << endl;
         abort();
      }

      try
      {
         my_handle->cleanup_pop(true);
      }
      catch (...)
      {
         cout << "Exception while trying to pop cleanup handler" << endl;
         abort();
      }
      try
      {
         my_parm->unlockRead();
      }
      catch (...)
      {
         cout << "Exception while trying to release a read lock" << endl;
         abort();
      }

      try
      {
          my_handle->delete_tsd(keys[i % 4]);
      }
      catch (...)
      {
         cout << "Exception while trying to delete local storage: "
                 << Threads::id(myself).buffer << endl;
         abort();
      }
      i++;
   }

   return ThreadReturnType(0);
}


ThreadReturnType PEGASUS_THREAD_CDECL writing_thread(void *parm)
{

   Thread *my_handle = (Thread *)parm;
   ReadWriteSem * my_parm = (ReadWriteSem *)my_handle->get_parm();

   ThreadType myself = Threads::self();

   if (verbose)
       cout << "w";

   while(die == false)
   {
      try
      {
         my_parm->waitWrite();
      }
      catch (...)
      {
         cout << "Exception while trying to get a write lock" << endl;
         abort();
      }
      write_count++;
      if (verbose)
          cout << "*";
      my_handle->sleep(1);
      try
      {
         my_parm->unlockWrite();
      }
      catch (...)
      {
         cout << "Exception while trying to release a write lock:"
              << Threads::id(myself).buffer << endl;
         abort();
      }
   }

   return ThreadReturnType(0);
}

/////////////////////////////////////////////////////////////////////////
//
// Main
//
// //////////////////////////////////////////////////////////////////////
int main(int, char **argv)
{
    verbose = (getenv("PEGASUS_TEST_VERBOSE")) ? true : false;

    testOneThread();

    testMultipleThreads();

    // Check for a thread deadlock handling Bug
    testCancelDeadLockedThread(argv[0]);

    testReadWriteThreads();

    cout << argv[0] << " +++++ passed all tests" << endl;
    return(0);
}

void test3_thread_cleanup1(void*)
{
    testval1 = 42;
}


