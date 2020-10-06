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

#include <Pegasus/Common/PegasusAssert.h>
#include <Pegasus/Common/Mutex.h>
#include <Pegasus/Common/Thread.h>

#if defined(PEGASUS_OS_TYPE_UNIX)
# include <unistd.h>
# if defined(PEGASUS_HAS_SIGNALS)
#  include <sys/wait.h>
# endif
#endif

PEGASUS_USING_PEGASUS;
PEGASUS_USING_STD;

Boolean verbose = false;

static Mutex forkSafeMutex;
PEGASUS_FORK_SAFE_MUTEX(forkSafeMutex)

ThreadReturnType PEGASUS_THREAD_CDECL mutexLockThread(void* parm)
{
    if (verbose)
    {
        cout << "mutexLockThread locking" << endl;
    }

    forkSafeMutex.lock();

    if (verbose)
    {
        cout << "mutexLockThread got lock" << endl;
    }

    Threads::sleep(1000);

    if (verbose)
    {
        cout << "mutexLockThread unlocking" << endl;
    }

    forkSafeMutex.unlock();

    return 0;
}

#if defined(PEGASUS_OS_TYPE_UNIX)
void testForkSafeMutex()
{
    Thread th1(mutexLockThread, 0, false);
    th1.run();
    Threads::sleep(100);

    pid_t forkStatus = fork();
    if (forkStatus < 0)
    {
        cerr << "fork() error " << errno << endl;
        PEGASUS_TEST_ASSERT(0);
    }
    else if (forkStatus == 0)
    {
        // Child

        if (verbose)
        {
            cout << "Child process locking" << endl;
        }

        forkSafeMutex.lock();

        if (verbose)
        {
            cout << "Child process got lock" << endl;
        }

        forkSafeMutex.unlock();

        if (verbose)
        {
            cout << "Child process unlocking" << endl;
        }

        exit(0);
    }
    else
    {
        // Parent
#if defined(PEGASUS_HAS_SIGNALS)
        // Make sure the child process exits
        pid_t status = 0;
        do
        {
            status = waitpid(forkStatus, 0, 0);
        } while ((status == -1) && (errno == EINTR));

        PEGASUS_TEST_ASSERT(status != -1);
#endif
    }

    th1.join();
}
#endif

int main(int, char** argv)
{
    verbose = (getenv("PEGASUS_TEST_VERBOSE")) ? true : false;

    try
    {
#if defined(PEGASUS_OS_TYPE_UNIX) && !defined(PEGASUS_OS_ZOS)
        testForkSafeMutex();
#endif
    }
    catch (Exception& e)
    {
        cerr << argv[0] << " Exception " << e.getMessage() << endl;
        exit(1);
    }

    cout << argv[0] << " +++++ passed all tests" << endl;
    return 0;
}
