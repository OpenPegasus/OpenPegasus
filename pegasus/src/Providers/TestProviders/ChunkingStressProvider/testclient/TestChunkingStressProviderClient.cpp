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

#include <cstdio>

#include <Pegasus/Common/Thread.h>
#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/Constants.h>
#include <Pegasus/Common/Exception.h>
#include <Pegasus/Common/ArrayInternal.h>
#include <Pegasus/Common/AutoPtr.h>
#include <Pegasus/Common/AtomicInt.h>

#include <Pegasus/General/Stopwatch.h>

#include <Pegasus/Client/CIMClient.h>

#define EXPECTED_INSTANCES 4004
#define EXPECTED_INSTANCENAMES 4004
#define EXPECTED_ASSOCIATORS 4004
#define EXPECTED_ASSOCIATORNAMES 4004
#define EXPECTED_REFERENCES 4004
#define EXPECTED_REFERENCENAMES 4004

PEGASUS_USING_PEGASUS;
PEGASUS_USING_STD;

static Boolean verbose = 0;
static Boolean shutdownFlag = false;
AtomicInt expectedResults;
AtomicInt actualResults;

CIMObjectPath referenceObjName;

static const String NAMESPACE("test/TestProvider");
static const String CLASSNAME("TST_ChunkingStressInstance");
static const String ASSOC_CLASSNAME("TST_ChunkingStressAssoc");
////////////////////////////////////////////////////////////////////////////////
//
// Thread Parameters Class
//
////////////////////////////////////////////////////////////////////////////////
class T_Parms{
   public:
    CIMClient* client;
    Uint32 duration;
    Uint32 uniqueID;
};

void _usage ()
{
   cerr << endl
        << "Usage:" << endl
        << "    TestChunkingStressProviderClient xxxxxx abcdef\n"
        << "    where: " << endl
        << "       xxxxxx - is the time in seconds that the test will run\n"
        << "       abcdef - is a 6 digit number where each digit represents\n"
        << "                the number of threads to run for each chunking\n"
        << "                CIM operation:\n"
        << "                  a - enumerateInstances\n"
        << "                  b - enumerateInstanceNames\n"
        << "                  c - associators\n"
        << "                  d - associatorNames\n"
        << "                  e - references\n"
        << "                  f - referenceNames\n"
        << "       For example:\n"
        << "          TestChunkingStressProviderClient 60 210777\n"
        << "       will run a total of 24 threads for about 60 seconds:\n"
        << "          2 threads will loop making enumerateInstances calls\n"
        << "          1 thread  will loop making enumerateInstanceNames calls\n"
        << "          7 threads will loop making associatorNames calls\n"
        << "          7 threads will loop making references calls\n"
        << "          7 threads will loop making referenceNames calls\n"
        << "          No threads will make any associator calls\n"
        << endl << endl;
}

/* Status display of the various steps.  Shows message of function and
time to execute.  Grow this to a class so we have start and stop and time
display with success/failure for each function.
*/
static void _testStart(
    const String& cimop, const Uint32 uniqueID,
    const Uint32 duration, const String& message)
{
    cout << "+++++ " << cimop << " thread " << uniqueID << ": " << message
         << " for " << duration << " seconds" << endl;
}

static void _testEnd(
    const String& cimop, const Uint32 uniqueID,
    const Uint32 iterations, const double elapsedTime)
{
    if (shutdownFlag)
    {
        cout << "????? " << cimop << " thread " << uniqueID
             << ": shutting down due to indicated failure on another thread"
             << endl;
    }
    else
    {
        cout << "+++++ " << cimop << " thread " << uniqueID
             << ": passed in " << elapsedTime
             << " seconds with " << iterations << " iterations" << endl;
    }
}

ThreadReturnType PEGASUS_THREAD_CDECL _executeEI(void *parm)
{
    Thread *my_thread = (Thread *)parm;
    T_Parms *parms = (T_Parms *)my_thread->get_parm();
    CIMClient *client = parms->client;
    Uint32 duration = parms->duration;
    Uint32 uniqueID = parms->uniqueID;

    char exceptionMsg[100];
    Uint32 iterations = 0;
    double elapsedSeconds = 0.0;

    try
    {
        Stopwatch elapsedTime;

        _testStart("EI", uniqueID, duration, "Starting enumerateInstances");
        elapsedTime.reset();
        while ((elapsedSeconds < duration) && !shutdownFlag)
        {
            expectedResults++;

            iterations++;
            elapsedTime.start();

            Array<CIMInstance> cimInstances =
                client->enumerateInstances(NAMESPACE, CLASSNAME);

            elapsedTime.stop();
            elapsedSeconds = elapsedTime.getElapsed();

            actualResults++;

            if (cimInstances.size() == EXPECTED_INSTANCES)
            {
                if (verbose)
                {
                    cout << "      EI thread " << uniqueID
                         << ": iteration " << iterations
                         << ": received " << cimInstances.size()
                         << " instances,"
                         << " et " << elapsedSeconds
                         << " of " << duration << endl;
                }
            }
            else
            {
                shutdownFlag = true;
                sprintf(exceptionMsg,
                    "----- EI thread %u expected %d instances, received %u",
                    uniqueID,
                    EXPECTED_INSTANCES,
                    cimInstances.size());

                throw Exception(exceptionMsg);
            }
        }
        _testEnd("EI", uniqueID, iterations, elapsedSeconds);
    }
    catch (Exception& e)
    {
        cout << "---- EI thread " << uniqueID << " caught exception: "
            << e.getMessage() << endl;
    }

    delete parms;

    return ThreadReturnType(0);
}

ThreadReturnType PEGASUS_THREAD_CDECL _executeNI(void *parm)
{
    Thread *my_thread = (Thread *)parm;
    T_Parms *parms = (T_Parms *)my_thread->get_parm();
    CIMClient *client = parms->client;
    Uint32 duration = parms->duration;
    Uint32 uniqueID = parms->uniqueID;

    char exceptionMsg[100];
    Uint32 iterations = 0;
    double elapsedSeconds = 0.0;

    try
    {
        Stopwatch elapsedTime;

        _testStart("NI", uniqueID, duration, "Starting enumerateInstanceNames");
        elapsedTime.reset();
        while ((elapsedSeconds < duration) && !shutdownFlag)
        {
            expectedResults++;

            iterations++;
            elapsedTime.start();

            Array<CIMObjectPath> cimInstanceNames =
                client->enumerateInstanceNames(NAMESPACE, CLASSNAME);

            elapsedTime.stop();
            elapsedSeconds = elapsedTime.getElapsed();

            actualResults++;

            if (cimInstanceNames.size() == EXPECTED_INSTANCENAMES)
            {
                if (verbose)
                {
                    cout << "      NI thread " << uniqueID
                         << ": iteration " << iterations
                         << ": received " << cimInstanceNames.size()
                         << " inst names,"
                         << " et " << elapsedSeconds
                         << " of " << duration << endl;
                }
            }
            else
            {
                shutdownFlag = true;
                sprintf(exceptionMsg,
                    "----- NI thread %u expected %d instancenames, received %u",
                    uniqueID,
                    EXPECTED_INSTANCENAMES,
                    cimInstanceNames.size());

                throw Exception(exceptionMsg);
            }
        }
        _testEnd("NI", uniqueID, iterations, elapsedSeconds);
    }
    catch (Exception& e)
    {
        cout << "---- NI thread " << uniqueID << " caught exception: "
            << e.getMessage() << endl;
    }

    delete parms;

    return ThreadReturnType(0);
}

ThreadReturnType PEGASUS_THREAD_CDECL _executeA(void *parm)
{
    Thread *my_thread = (Thread *)parm;
    T_Parms *parms = (T_Parms *)my_thread->get_parm();
    CIMClient *client = parms->client;
    Uint32 duration = parms->duration;
    Uint32 uniqueID = parms->uniqueID;

    char exceptionMsg[100];
    Uint32 iterations = 0;
    double elapsedSeconds = 0.0;

    try
    {
        Stopwatch elapsedTime;

        _testStart("A ", uniqueID, duration, "Starting associators");
        elapsedTime.reset();
        while ((elapsedSeconds < duration) && !shutdownFlag)
        {
            expectedResults++;

            iterations++;
            elapsedTime.start();

            Array<CIMObject> cimObjects =
                client->associators(NAMESPACE, referenceObjName);

            elapsedTime.stop();
            elapsedSeconds = elapsedTime.getElapsed();

            actualResults++;

            if (cimObjects.size() == EXPECTED_ASSOCIATORS)
            {
                if (verbose) // alternative: check verbose flag
                {
                    cout << "      A  thread " << uniqueID
                         << ": iteration " << iterations
                         << ": received " << cimObjects.size()
                         << " objects,"
                         << " et " << elapsedSeconds
                         << " of " << duration << endl;
                }
            }
            else
            {
                shutdownFlag = true;
                sprintf(exceptionMsg,
                    "----- A  thread %u expected %d objects, received %u",
                    uniqueID,
                    EXPECTED_ASSOCIATORS,
                    cimObjects.size());

                throw Exception(exceptionMsg);
            }
        }
        _testEnd("A ", uniqueID, iterations, elapsedSeconds);
    }
    catch (Exception& e)
    {
        cout << "---- A  thread " << uniqueID << " caught exception: "
            << e.getMessage() << endl;
    }

    delete parms;

    return ThreadReturnType(0);
}

ThreadReturnType PEGASUS_THREAD_CDECL _executeAN(void *parm)
{
    Thread *my_thread = (Thread *)parm;
    T_Parms *parms = (T_Parms *)my_thread->get_parm();
    CIMClient *client = parms->client;
    Uint32 duration = parms->duration;
    Uint32 uniqueID = parms->uniqueID;

    char exceptionMsg[100];
    Uint32 iterations = 0;
    double elapsedSeconds = 0.0;

    try
    {
        Stopwatch elapsedTime;

        _testStart("AN", uniqueID, duration, "Starting associatorNames");
        elapsedTime.reset();
        while ((elapsedSeconds < duration) && !shutdownFlag)
        {
            expectedResults++;

            iterations++;
            elapsedTime.start();

            Array<CIMObjectPath> cimObjectNames =
                client->associatorNames(NAMESPACE, referenceObjName);

            elapsedTime.stop();
            elapsedSeconds = elapsedTime.getElapsed();

            actualResults++;

            if (cimObjectNames.size() == EXPECTED_ASSOCIATORNAMES)
            {
                if (verbose) // alternative: check verbose flag
                {
                    cout << "      AN thread " << uniqueID
                         << ": iteration " << iterations
                         << ": received " << cimObjectNames.size()
                         << " object names,"
                         << " et " << elapsedSeconds
                         << " of " << duration << endl;
                }
            }
            else
            {
                shutdownFlag = true;
                sprintf(exceptionMsg,
                    "----- AN thread %u expected %d objectnames, received %u",
                    uniqueID,
                    EXPECTED_ASSOCIATORNAMES,
                    cimObjectNames.size());

                throw Exception(exceptionMsg);
            }
        }
        _testEnd("AN", uniqueID, iterations, elapsedSeconds);
    }
    catch (Exception& e)
    {
        cout << "---- AN thread " << uniqueID << " caught exception: "
            << e.getMessage() << endl;
    }

    delete parms;

    return ThreadReturnType(0);
}

ThreadReturnType PEGASUS_THREAD_CDECL _executeR(void *parm)
{
    Thread *my_thread = (Thread *)parm;
    T_Parms *parms = (T_Parms *)my_thread->get_parm();
    CIMClient *client = parms->client;
    Uint32 duration = parms->duration;
    Uint32 uniqueID = parms->uniqueID;

    char exceptionMsg[100];
    Uint32 iterations = 0;
    double elapsedSeconds = 0.0;

    try
    {
        Stopwatch elapsedTime;

        _testStart("R ", uniqueID, duration, "Starting references");
        elapsedTime.reset();
        while ((elapsedSeconds < duration) && !shutdownFlag)
        {
            expectedResults++;

            iterations++;
            elapsedTime.start();

            Array<CIMObject> cimObjects =
                client->references(NAMESPACE, referenceObjName);

            elapsedTime.stop();
            elapsedSeconds = elapsedTime.getElapsed();

            actualResults++;

            if (cimObjects.size() == EXPECTED_REFERENCES)
            {
                if (verbose) // alternative: check verbose flag
                {
                    cout << "      R  thread " << uniqueID
                         << ": iteration " << iterations
                         << ": received " << cimObjects.size()
                         << " objects,"
                         << " et " << elapsedSeconds
                         << " of " << duration << endl;
                }
            }
            else
            {
                shutdownFlag = true;
                sprintf(exceptionMsg,
                    "----- R  thread %u expected %d objects, received %u",
                    uniqueID,
                    EXPECTED_REFERENCES,
                    cimObjects.size());

                throw Exception(exceptionMsg);
            }
        }
        _testEnd("R ", uniqueID, iterations, elapsedSeconds);
    }
    catch (Exception& e)
    {
        cout << "---- R  thread " << uniqueID << " caught exception: "
            << e.getMessage() << endl;
    }

    delete parms;

    return ThreadReturnType(0);
}

ThreadReturnType PEGASUS_THREAD_CDECL _executeRN(void *parm)
{
    Thread *my_thread = (Thread *)parm;
    T_Parms *parms = (T_Parms *)my_thread->get_parm();
    CIMClient *client = parms->client;
    Uint32 duration = parms->duration;
    Uint32 uniqueID = parms->uniqueID;

    char exceptionMsg[100];
    Uint32 iterations = 0;
    double elapsedSeconds = 0.0;

    try
    {
        Stopwatch elapsedTime;

        _testStart("RN", uniqueID, duration, "Starting referenceNames");
        elapsedTime.reset();
        while ((elapsedSeconds < duration) && !shutdownFlag)
        {
            expectedResults++;

            iterations++;
            elapsedTime.start();

            Array<CIMObjectPath> cimObjectNames =
                client->referenceNames(NAMESPACE, referenceObjName);

            elapsedTime.stop();
            elapsedSeconds = elapsedTime.getElapsed();

            actualResults++;

            if (cimObjectNames.size() == EXPECTED_REFERENCENAMES)
            {
                if (verbose) // alternative: check verbose flag
                {
                    cout << "      RN thread " << uniqueID
                         << ": iteration " << iterations
                         << ": received " << cimObjectNames.size()
                         << " object names,"
                         << " et " << elapsedSeconds
                         << " of " << duration << endl;
                }
            }
            else
            {
                shutdownFlag = true;
                sprintf(exceptionMsg,
                    "----- RN thread %u expected %d objectnames, received %u",
                    uniqueID,
                    EXPECTED_REFERENCENAMES,
                    cimObjectNames.size());

                throw Exception(exceptionMsg);
            }
        }
        _testEnd("RN", uniqueID, iterations, elapsedSeconds);
    }
    catch (Exception& e)
    {
        cout << "---- RN thread " << uniqueID << " caught exception: "
            << e.getMessage() << endl;
    }

    delete parms;

    return ThreadReturnType(0);
}

Thread * _runTestThreads(
    CIMClient* client,
    ThreadReturnType (PEGASUS_THREAD_CDECL *_executeFn)(void *),
    const Uint32 duration,
    Uint32 uniqueID)
{
    // package parameters, create thread and run...
    AutoPtr<T_Parms> parms(new T_Parms());
    parms->client = client;
    parms->duration = duration;
    parms->uniqueID = uniqueID;
    AutoPtr<Thread> t(new Thread(_executeFn, (void*)parms.release(), false));
    // zzzzz... (1 second) zzzzz...
    Threads::sleep(1000);
    t->run();
    return t.release();
}

void _beginTest(const Uint32 duration, const char* thdCountStr)
{
    // From argument string, determine number of threads to perform each
    // of the chunking CIM operations.
    Uint32 totalThdCount;
    Uint32 ei_count, ni_count, a_count, an_count, r_count, rn_count;
    sscanf(thdCountStr, "%1u%1u%1u%1u%1u%1u",
           &ei_count, &ni_count, &a_count, &an_count, &r_count, &rn_count);

    totalThdCount = ei_count+ni_count+a_count+an_count+r_count+rn_count;

    Array<CIMClient *> clientConnections;

    // declare the clients
    CIMClient * tmpClient;
    for(Uint32 i = 0; i < totalThdCount; i++)
    {
        tmpClient = new CIMClient();
        clientConnections.append(tmpClient);
    }

    // connect the clients
    for(Uint32 i = 0; i < totalThdCount; i++)
    {
        clientConnections[i]->setTimeout(totalThdCount*20000);
        clientConnections[i]->connectLocal();
    }

    // run tests
    Array<Thread *> clientThreads;
    Uint32 thdIndex = 0;

    // start enumerateInstances threads
    for(Uint32 i = 0; i < ei_count; i++, thdIndex++)
    {
        clientThreads.append(_runTestThreads(clientConnections[thdIndex],
                                             _executeEI,
                                             duration,
                                             thdIndex));
    }

    // start enumerateInstanceNames threads
    for(Uint32 i = 0; i < ni_count; i++, thdIndex++)
    {
        clientThreads.append(_runTestThreads(clientConnections[thdIndex],
                                             _executeNI,
                                             duration,
                                             thdIndex));
    }

    // start associator threads
    for(Uint32 i = 0; i < a_count; i++, thdIndex++)
    {
        clientThreads.append(_runTestThreads(clientConnections[thdIndex],
                                             _executeA,
                                             duration,
                                             thdIndex));
    }

    // start associatorNames threads
    for(Uint32 i = 0; i < an_count; i++, thdIndex++)
    {
        clientThreads.append(_runTestThreads(clientConnections[thdIndex],
                                             _executeAN,
                                             duration,
                                             thdIndex));
    }

    // start references threads
    for(Uint32 i = 0; i < r_count; i++, thdIndex++)
    {
        clientThreads.append(_runTestThreads(clientConnections[thdIndex],
                                             _executeR,
                                             duration,
                                             thdIndex));
    }

    // start referenceNames threads
    for(Uint32 i = 0; i < rn_count; i++, thdIndex++)
    {
        clientThreads.append(_runTestThreads(clientConnections[thdIndex],
                                             _executeRN,
                                             duration,
                                             thdIndex));
    }

    // wait for threads to terminate
    for(Uint32 i=0; i< clientThreads.size(); i++)
    {
        clientThreads[i]->join();
    }

    // clean up connections
    for(Uint32 i=0; i< clientConnections.size(); i++)
    {
        delete clientConnections[i];
    }
    // clean up threads
    for(Uint32 i=0; i < clientThreads.size(); i++)
    {
        delete clientThreads[i];
    }
}

int main(int argc, char** argv)
{
    Uint32 duration = 0;
    expectedResults.set(0);
    actualResults.set(0);

    verbose = getenv("PEGASUS_TEST_VERBOSE")? true : false;

    if (argc <=1 || argc > 3)
    {
        cerr << "Invalid argument count: " << argc << endl;
        _usage();
        return 1;
    }

    const char * optOne = argv[1];
    duration = atoi(optOne);  //TODO: add error checking

    const char * optTwo = argv[2];
    if (strlen(optTwo) != 6)
    {
        cerr << "Invalid second argument, not 6 digits: " << optTwo << endl;
        _usage();
        return 1;
    }

    try
    {
        {
            // Just pick any instance for associator and reference ops
            CIMClient workClient;
            workClient.connectLocal();
            Array<CIMObjectPath> cimInstanceNames =
                workClient.enumerateInstanceNames(NAMESPACE, CLASSNAME);
            referenceObjName = cimInstanceNames[0];
            workClient.disconnect();
        }
        _beginTest(duration, optTwo);
        cout << "Expected Results: " << expectedResults.get() << endl;
        cout << "Actual Results  : " << actualResults.get() << endl;
        if (expectedResults.get() != actualResults.get())
        {
            cerr << " ---- failed tests" << endl;
            return(1);
        }
    }
    catch(const CIMException & e)
    {
        cout << "CIMException: " << e.getCode() << " "
            << e.getMessage() << endl;
        return(1);
    }
    catch(const Exception & e)
    {
        cout << "Exception: " << e.getMessage() << endl;
        return(1);
    }
    catch(...)
    {
        cout << "unknown exception" << endl;
        return(1);
    }

    if (shutdownFlag)
        return(1);

    cout << argv[0] << " +++++ passed all tests" << endl;

    return(0);
}
