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
//%///////////////////////////////////////////////////////////////////////

#include <cstdio>

#include <Pegasus/Common/Thread.h>
#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/Constants.h>
#include <Pegasus/Common/Exception.h>
#include <Pegasus/Common/ArrayInternal.h>
#include <Pegasus/Common/AutoPtr.h>
#include <Pegasus/Common/AtomicInt.h>
#include <Pegasus/Common/PegasusAssert.h>
#include <Pegasus/General/Stopwatch.h>
#include <Pegasus/Common/System.h>
#include <Pegasus/Common/Exception.h>
#include <Pegasus/Client/CIMClient.h>
#if defined(PEGASUS_OS_TYPE_UNIX)
# include <unistd.h>
# include <errno.h>
# include <limits.h>
#endif


PEGASUS_USING_PEGASUS;
PEGASUS_USING_STD;


/*
 * Here we test  the availability of cimserver even after it crosses
 * its socket fd limit(max client connections) or a faulty provider crosses
 * its thread limit.
 *
 * 1. The setup has a provider module with 1 faulty instance provider and one
 * indication provider. The faulty provider does not respond correctly
 * or respond after connection timeout or responds after the idle cleanup thread
 * has been run by the cimserver. There is also a good instance provider which 
 * serves the same class hierarchy as the faulty instance provider. The test 
 * client inundates the cimserver with huge number of CIM requests for the 
 * faulty provider and good provider on multiple connections causing the 
 * cimserver to reach its thread/fd limit whichever  is lower.
 * 2. The test client detects that cimserver crossed fd limit if it recieved
 * CannotConnectException after few requests .
 * OR
 * 3. The test client detects that cimserver crossed thread limit if it 
 * received CIMException after  few requests.Then the test client runs a test
 * on sample instance provider which should succeed.
 * 4. Then the test client waits for cimserver cleanup interval for cimserver to
 * release the held resources (fds/threads) after which it executes a test on
 * a sample provider which should succeed.
 * 5. The indication provider is exercised at the same time when a request is
 * sent to the faulty instance provider.
 * 6. The above tests should run through CHO and should pass the leak test.
 */
#define DEFAULT_MAX_CIM_REQUESTS 2000
#define MAX_CONNECTIONS 100 
#define CONN_TIMEOUT 3
#define EXPECTED_INSTANCENAMES 2
//
//idle time clean up is currently hardcoded at 300 sec, 
//so making it just little more than 300 sec
#define CIMSERVER_CLEANUP_TIME 305

static char * verbose = 0;
static CIMException expectedNICIMException(CIM_ERR_FAILED);

static AtomicInt  recievedNICIMExceptionCount(0);
static AtomicInt  recievedNIConnTimeoutExceptionCount(0);
static AtomicInt  recievedNICannotConnectExceptionCount(0);

static const String NAMESPACE("test/TestProvider");
static const String FAULTY_CLASSNAME("TST_FaultyInstance");
static const String WELLBEHAVED_CLASSNAME("TST_Instance1");

class T_Parms{
public:
    CIMClient* client;
    long requestCount;
    Uint32 uniqueID;
};

Thread * _runTestThreads(
    CIMClient* client,
    ThreadReturnType (PEGASUS_THREAD_CDECL *_executeFn)(void *),
    Uint32 uniqueID,
    long requestCount)
{
    // package parameters, create thread and run...
    AutoPtr<T_Parms> parms(new T_Parms());

    parms->client = client;
    parms->uniqueID = uniqueID;
    parms->requestCount = requestCount;
    AutoPtr<Thread> t(new Thread(_executeFn, (void*)parms.release(), false));
    // zzzzz... (1 second) zzzzz...
    Threads::sleep(100);
    t->run();
    return t.release();
}

void _executeRequest(Array<CIMClient *>& clientConnections,
    ThreadReturnType (PEGASUS_THREAD_CDECL *_executeFn)(void *),
    long requests)
{
    // run tests
    Array<Thread *> clientThreads;
    Uint32 thdIndex = 0;
   
     //lets split the cim requests equally among client connections
    long requestCount = (requests/clientConnections.size());

    // start enumerateInstanceNames threads
    for(Uint32 i = 0; i < clientConnections.size(); i++, thdIndex++)
    {
        clientThreads.append(
            _runTestThreads(clientConnections[thdIndex],
                _executeFn,
                    thdIndex,
                requestCount));
    }

    // wait for threads to terminate
    for(Uint32 i=0; i< clientThreads.size(); i++)
    {
        clientThreads[i]->join();
    }

    // clean up threads
    for(Uint32 i=0; i < clientThreads.size(); i++)
    {
        delete clientThreads[i];
    }
}

bool TestWellbehavedInstanceProvider()
{
    bool result = false;

    try
    {
        CIMClient client;
        client.connectLocal();

        Array<CIMObjectPath> cimInstanceNames =
            client.enumerateInstanceNames(
                NAMESPACE,
                WELLBEHAVED_CLASSNAME);
        if(cimInstanceNames.size() > 0)
        {
            result = true;
        }
     }
     catch (Exception& e)
     {
         cout << "---- caught exception from test instance provider : "
             << e.getMessage()<<endl;
         result = false;
     }

     return result;
}

ThreadReturnType PEGASUS_THREAD_CDECL _createSubFunc(void *parm)
{
    // client for creating subscription on the good 
    // provider which is part of the FaultyProviderModule.
    CIMClient client;
    client.connectLocal();

   // create filter and handler
    String queryFaulty01("SELECT * FROM CIM_AlertIndication");
 
    String filterFaulty01("FilterFaulty01");

    CIMInstance filterInstance (PEGASUS_CLASSNAME_INDFILTER);
    filterInstance.addProperty (CIMProperty (CIMName
        ("SystemCreationClassName"), System::getSystemCreationClassName ()));
    filterInstance.addProperty (CIMProperty (CIMName ("SystemName"),
        System::getFullyQualifiedHostName ()));
    filterInstance.addProperty (CIMProperty (CIMName ("CreationClassName"),
        PEGASUS_CLASSNAME_INDFILTER.getString ()));
    filterInstance.addProperty (CIMProperty (CIMName ("Name"), filterFaulty01));
    filterInstance.addProperty (CIMProperty (CIMName ("Query"), queryFaulty01));
    filterInstance.addProperty (CIMProperty (CIMName ("QueryLanguage"),
        String ("WQL")));
    filterInstance.addProperty (CIMProperty (CIMName ("SourceNamespace"),
        String(NAMESPACE)));

    CIMObjectPath filterObjectPath;
    try
    {
        filterObjectPath = client.createInstance(
            NAMESPACE,
            filterInstance);
    }
    catch(Exception& e)
    {
        cerr << "----- Error: Filter Instance Not Created: " <<
           e.getMessage() << endl;
        PEGASUS_TEST_ASSERT(false);
    }

    String handlerFaulty01("Handlerfaulty01");
    String destination =String("http//localhost");

    CIMInstance handlerInstance (PEGASUS_CLASSNAME_INDHANDLER_CIMXML);
    handlerInstance.addProperty (CIMProperty (CIMName
        ("SystemCreationClassName"), System::getSystemCreationClassName ()));
    handlerInstance.addProperty (CIMProperty (CIMName ("SystemName"),
        System::getFullyQualifiedHostName ()));
    handlerInstance.addProperty (CIMProperty (CIMName ("CreationClassName"),
        PEGASUS_CLASSNAME_INDHANDLER_CIMXML.getString ()));
    handlerInstance.addProperty (CIMProperty (CIMName ("Name"), 
        handlerFaulty01));
    handlerInstance.addProperty (CIMProperty (CIMName ("Destination"),
        destination));

    CIMObjectPath handlerObjPath;
    try
    {
        handlerObjPath = client.createInstance(
            NAMESPACE,
            handlerInstance);
    }
    catch(Exception& e)
    {
        cerr << "----- Error: Handler Instance Not Created: " <<
            e.getMessage() << endl;
        PEGASUS_TEST_ASSERT(false);
    }

    // Thread 1 : subscription creation
    CIMInstance subscriptionInstance (PEGASUS_CLASSNAME_INDSUBSCRIPTION);
    subscriptionInstance.addProperty (CIMProperty (CIMName ("Filter"),
        filterObjectPath, 0, PEGASUS_CLASSNAME_INDFILTER));
    subscriptionInstance.addProperty (CIMProperty (CIMName ("Handler"),
        handlerObjPath, 0, PEGASUS_CLASSNAME_INDHANDLER_CIMXML));
    subscriptionInstance.addProperty (CIMProperty
        (CIMName ("SubscriptionState"), CIMValue ((Uint16) 2)));

    CIMObjectPath subscriptionObjPath;
    try
    {
        subscriptionObjPath = client.createInstance(
            NAMESPACE,
            subscriptionInstance);
    }
    catch(Exception& e)
    {
        cerr << "----- Error: Subscription Instance Not Created: " <<
            e.getMessage() << endl;
        PEGASUS_TEST_ASSERT(false);
    }
    cout << "Successfully created subscription instance" << endl;

    // delete subscription
    try
    {
        client.deleteInstance(NAMESPACE,subscriptionObjPath);
        client.deleteInstance(NAMESPACE,filterObjectPath);
        client.deleteInstance(NAMESPACE,handlerObjPath);
    }
    catch(Exception& e)
    {
        cerr << "----- Error: Subscription deletion failed : " <<
            e.getMessage() << endl;
        PEGASUS_TEST_ASSERT(false);
    }
 
    return ThreadReturnType(0);
}

ThreadReturnType PEGASUS_THREAD_CDECL _enumInstanceNamesFunc(void *parm)
{
    // client for enumerateInstanceNames request on the faulty
    // provider.
    CIMClient client;
    client.connectLocal();
 
    // Thread 2 : enumerate instance request to faulty provider.   
    try
    {
        Array<CIMObjectPath> objs = client.enumerateInstanceNames(NAMESPACE,
            FAULTY_CLASSNAME);
    }
    catch(Exception &e)
    {
        cout << "Caught exception while enumerating class " << 
            FAULTY_CLASSNAME << ": " << e.getMessage() << endl;
        return ThreadReturnType(0);
    }

    return ThreadReturnType(0);
}

bool TestIndicationProviderAndFaultyProvider()
{
    Thread t1(_createSubFunc, 0, false);
    Thread t2(_enumInstanceNamesFunc, 0, false);

    t1.run();

    t2.run();
   
    t1.join();
    t2.join();
 
    return true;
} 

ThreadReturnType PEGASUS_THREAD_CDECL _executeNIonFaultyProvider(void *parm)
{
    Thread *my_thread = (Thread *)parm;
    T_Parms *parms = (T_Parms *)my_thread->get_parm();
    CIMClient *client = parms->client;
    Uint32 uniqueID = parms->uniqueID;
    long requestCount = parms->requestCount;
    bool connectionAlreadyRefused=false;

    long i=0;
    for(i=0;i<requestCount;i++)
    {
        try
        {
            Array<CIMObjectPath> cimInstanceNames =
                client->enumerateInstanceNames(NAMESPACE, FAULTY_CLASSNAME);

           if(verbose)
               cout<<"Request " << i << ": Faulty provider responded..!"<<endl;
        }
        catch (CIMException& e)
        {
            if(e.getCode() == expectedNICIMException.getCode())
            {
                recievedNICIMExceptionCount.inc();
            }
            else
            {
                cout << "---- NI  thread " << uniqueID
                     <<" caught unexpected CIM exception: "<< e.getMessage()
                     << endl;
                break;
            }
        }

        catch (ConnectionTimeoutException& e)
        {
            recievedNIConnTimeoutExceptionCount.inc();
        }
        catch (CannotConnectException& e)
        {
             //While testing its observed that because of huge number of
             //requests and responses cimserver could be busy and unable to
             //accept a aconnection and hence ignoring the exception once.
            if(!connectionAlreadyRefused)
            {
                i--;
                connectionAlreadyRefused=true;
                Threads::sleep(1 * 1000);
            }
            else
            { 
                recievedNICannotConnectExceptionCount.inc();
                if(verbose )
                    cout << "---- NI  thread " << uniqueID 
                         << " caught exception: "<< e.getMessage() << endl;
                break;
            }
        }
        catch(CIMClientMalformedHTTPException& e)
        {
            // This is the exception thrown on HP-UX if connection cannot be
            // established.
            if(!connectionAlreadyRefused)
            {
                i--;
                connectionAlreadyRefused=true;
                Threads::sleep(1 * 1000);
            }
            else
            {
                recievedNICannotConnectExceptionCount.inc();
                if(verbose )
                    cout << "---- NI  thread " << uniqueID 
                         << " caught exception: "<< e.getMessage() << endl;
                break;
            }
        }

        catch (Exception& e)
        {
            cout << "---- NI  thread " << uniqueID 
                 << " caught unexpected exception: "<< e.getMessage() << endl;

            break;
        }
    }

    if(verbose && (i >= requestCount))
        cout<<"---- NI  thread " << uniqueID 
            << " successfully completed"<<endl;
    
    delete parms;
    return ThreadReturnType(0);
}

void TestCimserverAvailability()
{
#ifndef PEGASUS_OS_TYPE_WINDOWS
   //run a test on a test instance provider
    bool resultWellbehavedProvider = TestWellbehavedInstanceProvider();
    PEGASUS_TEST_ASSERT(resultWellbehavedProvider);    

    TestIndicationProviderAndFaultyProvider();

    Array<CIMClient *> clientConnections;

    // declare the clients
    CIMClient * tmpClient;

    // Calculate the thread limit on the OS.
    long maxThread(-1);
    #if defined(PEGASUS_OS_TYPE_UNIX)
        maxThread = sysconf(_SC_THREAD_THREADS_MAX);
    #endif

    // The maximum requests to be sent are derived from the maximum
    // thread limit on the OS. The Faulty provider responds properly
    // to some requests. Hence for the cimprovagt to reach its
    // thread limit we have to inundate it with maxThread*2 requests.
    long maxRequests;
    maxThread == -1 ?
        maxRequests = DEFAULT_MAX_CIM_REQUESTS:
        maxRequests = maxThread * 2;

    for(Uint32 i = 0; i < MAX_CONNECTIONS; i++)
    {
        tmpClient = new CIMClient();
        tmpClient->setTimeout(CONN_TIMEOUT * 1000);
        tmpClient->connectLocal();
        clientConnections.append(tmpClient);
    }

    //initialize
    recievedNICIMExceptionCount = 0;
    recievedNIConnTimeoutExceptionCount = 0;
    recievedNICannotConnectExceptionCount = 0;

    // run tests
    cout <<"Executing " << maxRequests
        <<" NI requests for faulty provider on " <<MAX_CONNECTIONS 
        <<" client connections for "
        << (maxRequests/MAX_CONNECTIONS)*CONN_TIMEOUT
        <<" sec."<<endl;

    _executeRequest(clientConnections,
        _executeNIonFaultyProvider,
        maxRequests);

   // clean up connections
   for(Uint32 i=0; i< clientConnections.size(); i++)
   {
       delete clientConnections[i];
   }

   if(verbose)
       cout<<"received CIM exceptions = "
           <<recievedNICIMExceptionCount.get()<<endl
           <<"received Connection Timeout exceptions = "
           <<recievedNIConnTimeoutExceptionCount.get()<<endl
           <<"received CannotConnect exceptions = "
           <<recievedNICannotConnectExceptionCount.get()<<endl;

   //we should have recieved either CIM exception(for crossing thread limit) or 
   //cannot connect exceptions(for crossing fd limit) AND not both/none.
   if(!(recievedNICIMExceptionCount.get() > 0) ^ 
       (recievedNICannotConnectExceptionCount.get() > 0))
   {
       cout << "The test could have failed due to two reasons : " << endl
           << "Reason 1 : The code did not behave as expected. " << endl
           << "Reason 2 : The number of requests used to inundate the "
           << "cimserver is insufficient." << endl; 
   }

   PEGASUS_TEST_ASSERT((recievedNICIMExceptionCount.get() > 0) ^ 
       (recievedNICannotConnectExceptionCount.get() > 0));

   //if cimserver reached its thread limit for the faulty provider
   //before fd limit..then evaluate the results.
   if(recievedNIConnTimeoutExceptionCount.get() == 0)
   {
       throw Exception(
           "did not receive any conn timeout exceptions "
           "for NI requests on faulty provider. ");
   }
   else
   if(recievedNICIMExceptionCount.get() > 0)
   {
       cout<<"Faulty provider agent has reached its threadpool limit."<<endl;

       //run a test on test instance provider
       resultWellbehavedProvider = TestWellbehavedInstanceProvider();
       PEGASUS_TEST_ASSERT(resultWellbehavedProvider);
   }
   else if(recievedNICannotConnectExceptionCount.get() > 0)
   {
       cout<<"cimserver may have reached its fd limit"
           " before the thread limit"<<endl;
   }
   else
   {
       PEGASUS_TEST_ASSERT(false);
   }

   // wait for cim server to clean up disconnected clients' resources(threads 
   // and fds).
   cout<<"Waiting for clean up routine to initiate cleanup for " << 
       CIMSERVER_CLEANUP_TIME<<" sec."<<endl;
   Threads::sleep(CIMSERVER_CLEANUP_TIME * 1000);

   //run a test on test instance provider
   resultWellbehavedProvider = TestWellbehavedInstanceProvider();
 
   if(resultWellbehavedProvider == false)
   {
       throw Exception(
           "A cim request on well behaved test instance provider failed"
           " after cimserver crossed its fd/thread limit");
   }
#else 
   cerr << "ATTN, This test is not executed on Windows due to unable to"
       " restrict threads and file pointer and should be fixed \n";
#endif
}

int main(int, char** argv)
{
    verbose = getenv("PEGASUS_TEST_VERBOSE");

    try
    {
        TestCimserverAvailability();
    }
    catch(const CIMException & e)
    {
        cout << "CIMException: " << e.getCode() << " "
            << e.getMessage() << endl;
        return(-1);
    }
    catch(const Exception & e)
    {
        cout << "Exception: " << e.getMessage() << endl;
        return(-1);
    }
    catch(...)
    {
        cout << "unknown exception" << endl;
        return(-1);
    }

    cout << argv[0] << " +++++ passed all tests" << endl;

    return(0);
}


