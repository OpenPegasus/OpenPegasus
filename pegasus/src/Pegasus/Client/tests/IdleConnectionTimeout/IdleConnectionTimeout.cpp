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

#include <stdlib.h>
#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/ArrayInternal.h>
#include <Pegasus/Common/AutoPtr.h>
#include <Pegasus/Common/AtomicInt.h>
#include <Pegasus/Common/Exception.h>
#include <Pegasus/Common/String.h>
#include <Pegasus/Common/System.h>
#include <Pegasus/Common/Thread.h>
#include <Pegasus/Client/CIMClient.h>

#define NUM_THREADS 2
#define TEST_PASSED true
#define TEST_FAILED false
#define OSINFO_NAMESPACE CIMNamespaceName ("root/cimv2")
#define OSINFO_CLASSNAME CIMName ("PG_OperatingSystem")

PEGASUS_USING_PEGASUS;
PEGASUS_USING_STD;

Boolean test2CaughtException;
AtomicInt sleepIterations;

////////////////////////////////////////////////////////////////////////////////
//
// Thread Parameters Class
//
////////////////////////////////////////////////////////////////////////////////
class T_Parms{
    public:
        Uint32 durationSeconds;
        const char * testUserid;
        const char * testPasswd;
};


ThreadReturnType PEGASUS_THREAD_CDECL _runningThd(void *parm)
{
    Thread *my_thread = (Thread *)parm;
    AutoPtr<T_Parms> parms((T_Parms *)my_thread->get_parm());
    Uint32 durationSeconds = parms->durationSeconds;
    const char * testUserid = parms->testUserid;
    const char * testPasswd = parms->testPasswd;

    CIMClient client;

    for(Uint32 i = 0; i <= durationSeconds; i++)
    {
        Threads::sleep(1000);
        sleepIterations++;
        if (testUserid == NULL)
        {
            client.connectLocal();
        }
        else
        {
            client.connect(System::getHostName(), 5988,
                String(testUserid), String(testPasswd));
        }

        try
        {
            CIMClass tmpClass =
                client.getClass (OSINFO_NAMESPACE, OSINFO_CLASSNAME);
        }
        catch(Exception& e)
        {
            cerr << "Error: " << e.getMessage() << endl;
            throw;
        }

        client.disconnect();
    }

    return ThreadReturnType(0);
}

ThreadReturnType PEGASUS_THREAD_CDECL _idleThd(void *parm)
{
    Thread *my_thread = (Thread *)parm;
    AutoPtr<T_Parms> parms((T_Parms *)my_thread->get_parm());
    Uint32 durationSeconds = parms->durationSeconds;
    const char * testUserid = parms->testUserid;
    const char * testPasswd = parms->testPasswd;

    CIMClient client;

    try
    {
        if (testUserid == NULL)
        {
            client.connectLocal();
        }
        else
        {
            client.connect(System::getHostName(), 5988,
                String(testUserid), String(testPasswd));
        }

        CIMClass tmpClass =
            client.getClass (OSINFO_NAMESPACE, OSINFO_CLASSNAME);

        cout << "Test 2 of 2: Begin " << durationSeconds
            << " second idle period..." << endl;

        while (sleepIterations.get() < durationSeconds)
        {
            Threads::sleep(1000);
        }
        //the client shall reconnect if the connection was closed and 
        //so the operation should succeed.
        CIMClass tmpClass2 =
            client.getClass (OSINFO_NAMESPACE, OSINFO_CLASSNAME);
    }
    catch(Exception& e)
    {
        test2CaughtException = true;
        cerr << "Error: " << e.getMessage() << endl;
    }
    client.disconnect();

    return ThreadReturnType(0);
}

Thread * _runTestThreads(
    ThreadReturnType (PEGASUS_THREAD_CDECL *_executeFn)(void *),
    const Uint32 durationSeconds,
    const char * testUserid,
    const char * testPasswd)
{
    // package parameters, create thread and run...
    AutoPtr<T_Parms> parms(new T_Parms());
    parms->durationSeconds = durationSeconds;
    parms->testUserid = testUserid;
    parms->testPasswd = testPasswd;
    AutoPtr<Thread> t(new Thread(_executeFn, (void*)parms.release(), false));
    // zzzzz... (1 second) zzzzz...
    Threads::sleep(1000);
    t->run();
    return t.release();
}

// _test1 verifies that a CIM Operation on a client connection succeeds
// if there is no other server activity, even if the idleConnectionTimeout
// period has been exceeded. In this case, no exception should occur for the
// second call to getClass().
Boolean _test1(
    int durationSeconds, const char * testUserid, const char * testPasswd)
{
    try
    {
        CIMClient client;

        if (testUserid == NULL)
        {
            client.connectLocal();
        }
        else
        {
            client.connect(System::getHostName(), 5988,
                String(testUserid), String(testPasswd));
        }

        CIMClass tmpClass =
            client.getClass (OSINFO_NAMESPACE, OSINFO_CLASSNAME);

        cout << "Test 1 of 2: Begin " << durationSeconds
            << " second idle period..." << endl;
        Threads::sleep(durationSeconds*1000);

        CIMClass tmpClass2 =
            client.getClass (OSINFO_NAMESPACE, OSINFO_CLASSNAME);
    }
    catch(Exception& e)
    {
        cerr << "Error: " << e.getMessage() << endl;
        return TEST_FAILED;
    }
    cout << "Test 1 success." << endl;
    return TEST_PASSED;
}

// _test2 verifies that a CIM Operation on a client connection will still
// succeed when the idleConnectionTimeout period is exceeded (_idleThd) *and*
// there is concurrent server activity (_runningThd) so that the Monitor
// will wake up and check for timeouts and close the connection.
// In this case, the second call to getClass() in _idleThd() will reconnect 
// and send the request.
Boolean _test2(
    int durationSeconds, const char * testUserid, const char * testPasswd)
{
    try
    {
        sleepIterations.set(0);
        // run tests
        Array<Thread *> clientThreads;

        test2CaughtException = false;

        clientThreads.append(
            _runTestThreads(
                _runningThd, durationSeconds, testUserid, testPasswd));

        clientThreads.append(
            _runTestThreads(
                _idleThd, durationSeconds, testUserid, testPasswd));

        // wait for threads to terminate
        for(Uint32 i=0; i< clientThreads.size(); i++)
        {
            clientThreads[i]->join();
        }

        // clean up threads
        for(Uint32 i=0; i < clientThreads.size(); i++)
        {
            if(clientThreads[i])
                delete clientThreads[i];
        }
    }
    catch(Exception& e)
    {
        cerr << "Error: " << e.getMessage() << endl;
        return TEST_FAILED;
    }

    // We do not expect exception in this case.
    return (test2CaughtException == false);
}

int main(int argc, char** argv)
{
    int durationSeconds = 0;
    char testUserid[256];
    char testPasswd[256];

    if (argc != 2)
    {
        cerr
            << "Invalid argument count, only parameter is sleep time in seconds"
            << endl;
        return 1;
    }

    const char * optOne = argv[1];
    durationSeconds = atoi(optOne);

    cout << "Testing connectLocal()" << endl;

    if (_test1(durationSeconds, NULL, NULL) == TEST_FAILED)
    {
        cerr << argv[0] << "----- _test1() localConnect failed" << endl;
        return 1;
    }

    if (_test2(durationSeconds, NULL, NULL) == TEST_FAILED)
    {
        cerr << argv[0] << "----- _test2() localConnect failed" << endl;
        return 1;
    }

    if ((getenv("PEGASUS_TEST_USER_ID")) &&
        (getenv("PEGASUS_TEST_USER_PASS")))
    {

        strncpy(testUserid, getenv("PEGASUS_TEST_USER_ID"),
            sizeof(testUserid));
        strncpy(testPasswd, getenv("PEGASUS_TEST_USER_PASS"),
            sizeof(testPasswd));

        cout << "Testing connect() with "
            << "host: " << System::getHostName()
            << ", port: 5988"
            << ", user: " << testUserid << endl;

        if (_test1(durationSeconds, testUserid, testPasswd) == TEST_FAILED)
        {
            cerr << argv[0] << "----- _test1() connect failed" << endl;
            return 1;
        }

        if (_test2(durationSeconds, testUserid, testPasswd) == TEST_FAILED)
        {
            cerr << argv[0] << "----- _test2() connect failed" << endl;
            return 1;
        }
    }
    else
    {
        cout << "Skipping connect() test, PEGASUS_TEST_USER_ID and "
            "PEGASUS_TEST_USER_PASS not defined." << endl;
    }

    PEGASUS_STD(cout) << "+++++ passed all tests" << PEGASUS_STD(endl);
    return 0;
}
