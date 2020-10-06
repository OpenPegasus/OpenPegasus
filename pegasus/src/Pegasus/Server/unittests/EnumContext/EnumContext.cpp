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

#include <iostream>
#include <Pegasus/Common/PegasusAssert.h>
#include <Pegasus/Server/EnumerationContext.h>
#include <Pegasus/Server/EnumerationContextTable.h>
#include <Pegasus/Server/CIMOperationRequestDispatcher.h>
#include <Pegasus/General/Guid.h>
#include <Pegasus/Common/Thread.h>
#include <Pegasus/Common/CIMResponseData.h>
#include <Pegasus/Common/CIMMessage.h>

PEGASUS_USING_STD;
PEGASUS_USING_PEGASUS;

static Boolean verbose;
#define VCOUT if (verbose) cout

// Reduce creation of simple context to one line for these tests.
// Returns the Id in the name argument
EnumerationContext* createEnumContext(EnumerationContextTable* enumTable,
                                        String& name)
{
    const CIMNamespaceName ns = "/test/namespace";
    String messageId = "1111";
    String enumerationContext = "6000";
    Uint32 maxObjectCount(500);
    Uint32Arg operationTimeout(30);
    CIMName className = "CIM_Nothing";
    // Used only to set values into the context.  Released at
    // end of this function
    AutoPtr<CIMOpenEnumerateInstancesRequestMessage> request(
        new CIMOpenEnumerateInstancesRequestMessage(
            messageId,
            ns,
            className,
            false,
            false,
            CIMPropertyList(),
            String(),
            String(),
            operationTimeout,
            false,
            maxObjectCount,
            QueueIdStack()));

    EnumerationContext* en = enumTable->createContext(
        request.get(),
        CIM_PULL_INSTANCES_WITH_PATH_REQUEST_MESSAGE,
        CIMResponseData::RESP_INSTANCES);
    name = en->getContextId();
    return en;
}
// Create
void test01()
{
    VCOUT << "Start test01" << endl;
    EnumerationContextTable* enumTable = new EnumerationContextTable(
        1000,    // max OpenContexts
        30,      // defaultInteroperationTimeout
        1000);   // responseCache max size

    String createdContextName1;
    EnumerationContext* en1 = createEnumContext(enumTable,
                                                  createdContextName1);

    PEGASUS_TEST_ASSERT(en1->responseCacheSize() == 0);

    //String createdContextName1 = et.createContext(enContext1);

    String createdContextName2;
    EnumerationContext* en2 = createEnumContext(enumTable,
                                             createdContextName2);

    EnumerationContext* enFound2 = enumTable->find(createdContextName2);

    PEGASUS_TEST_ASSERT(enFound2->valid());

    PEGASUS_TEST_ASSERT(enFound2 != 0);

    // test find on string not in table.
    PEGASUS_TEST_ASSERT(enumTable->find(String("xxxx")) == 0);

    PEGASUS_TEST_ASSERT(enumTable->size() == 2);

    en1->setClientClosed();
    en1->setProvidersComplete();

    en2->setClientClosed();
    en2->setProvidersComplete();
    en1->lockContext();
    en2->lockContext();
    enumTable->releaseContext(en1);
    enumTable->releaseContext(en2);

    PEGASUS_TEST_ASSERT(enumTable->size() == 0);

    enumTable->removeContextTable();
    delete(enumTable);

    VCOUT << "EnumTest End test01 successful" << endl;
}

// test creating a lot of contexts and then finding them
void test02()
{
    VCOUT << "Start test02" << endl;
    // Create the enumeration table
    EnumerationContextTable* enumTable = new EnumerationContextTable(
        1200,    // max OpenContexts
        30,      // defaultInteroperationTimeout
        1000);   // responseCache max size

    // Build a large set of contexts
    Uint32 testSize = 1000;
    Array<String> rtndContextNames;

    PEGASUS_TEST_ASSERT(enumTable->size() == 0);

    // Build a set of contexts defined by testSize
    for (Uint32 i = 0 ; i < testSize ; i++)
    {
        String createdContextName;
        PEGASUS_TEST_ASSERT(createEnumContext(enumTable,createdContextName));

        rtndContextNames.append(createdContextName);
        PEGASUS_TEST_ASSERT(rtndContextNames[i].size() != 0);
    }

    PEGASUS_TEST_ASSERT(enumTable->size() == testSize);

    // confirm that we can find all of the created contexts
    for (Uint32 i = 0 ; i < testSize ; i++)
    {
        EnumerationContext* en = enumTable->find(rtndContextNames[i]);
        PEGASUS_TEST_ASSERT(en != 0);
        PEGASUS_TEST_ASSERT(en->valid());
    }
    PEGASUS_TEST_ASSERT(enumTable->size() == testSize);

    // Confirm that contexts are unique.
    // KS_TODO the uniqueness test.

    // remove all contexts from table.
    for (Uint32 i = 0 ; i < testSize ; i++)
    {
        EnumerationContext* en = enumTable->find(rtndContextNames[i]);
        en->setProvidersComplete();
        en->setClientClosed();
        en->lockContext();
        enumTable->releaseContext(en);
    }

    PEGASUS_TEST_ASSERT(enumTable->size() == 0);

    enumTable->removeContextTable();
    delete(enumTable);

    VCOUT << "test02 Complete success" << endl;
}

// test the cache get functions.
void test04()
{
    VCOUT << "Start test04" << endl;
    // Create an entry for the cache

    // define an EnumerationContext
    EnumerationContextTable* enumTable = new EnumerationContextTable(
        1000,    // max OpenContexts
        30,      // defaultInteroperationTimeout
        1000);   // responseCache max size
    String createdContextName;
    EnumerationContext* en = createEnumContext(enumTable,
                                               createdContextName);

    PEGASUS_TEST_ASSERT(createdContextName == en->getContextId());


    // Put them into the cache
    VCOUT << "add response instance to cache" << endl;
    // Add one instance to cache.  Should not cause wait to end.
    for (Uint32 i = 0; i < 12; i++)
    {
        {
            QueueIdStack x;
            CIMEnumerateInstancesResponseMessage* response
                =  new CIMEnumerateInstancesResponseMessage(
                    "",
                    CIMException(),
                    x);

            en->putCache((CIMResponseMessage*&)response, false);
        }
    }
    //  get from the cache
    VCOUT << "getCacheResponseData" << endl;
    CIMResponseData from(CIMResponseData::RESP_INSTANCES);
    en->getCache(10, from);
    PEGASUS_TEST_ASSERT(from.size() == 10);

    enumTable->removeContextTable();
    delete(enumTable);
}


// test the cache condition variable functions. This includes thread functions
// and the test function.

Boolean threadComplete;
ThreadReturnType PEGASUS_THREAD_CDECL setProvidersComplete(void * parm)
{
    VCOUT << "Thread Start" << endl;
    Thread* my_thread = (Thread *)parm;
    EnumerationContext* enumerationContext =
        (EnumerationContext *)my_thread->get_parm();

    PEGASUS_ASSERT(enumerationContext->valid());

    // Add a test where we add some small number of items to the
    // cache.

    // signal again after setting providersComplete.
    // This one should wake up the condition.
    sleep(1);
    threadComplete = true;
    VCOUT << "Thread Sleep complete" << endl;
    enumerationContext->setProvidersComplete();
    VCOUT << "setProvidersComplete() Signaled" << endl;
    return ThreadReturnType(0);
}

// Test ProvidersComplete flag setting and getCacheResponseData
void test06()
{
    VCOUT << "Test06 Started" << endl;
    // define an EnumerationContext
    EnumerationContextTable* enumTable = new EnumerationContextTable(
        1000,    // max OpenContexts
        30,      // defaultInteroperationTimeout
        1000);   // responseCache max size

    // Create an enumeration context
    String enContextIdStr;
    EnumerationContext* en = createEnumContext(enumTable,
                                                 enContextIdStr);

    PEGASUS_TEST_ASSERT(en->responseCacheSize() == 0);
    threadComplete = false;

    Thread * th = new Thread(setProvidersComplete, (void *)en, false);

    th->run();

    // call the testresponseCacheSize Condition. Should return only when the
    // thread has signaled.
    VCOUT << "getCacheResponseData" << endl;
    CIMResponseData from(CIMResponseData::RESP_INSTANCES);
    en->getCache(10, from);

    th->join();
    delete th;
    en->lockContext();
    enumTable->releaseContext(en);

    enumTable->removeContextTable();
    delete(enumTable);
}
ThreadReturnType PEGASUS_THREAD_CDECL setresponseCacheSize(void * parm)
{
    VCOUT << "Thread setresponseCacheSize Start" << endl;
    Thread* my_thread = (Thread *)parm;
    EnumerationContext* enumerationContext =
        (EnumerationContext *)my_thread->get_parm();

    PEGASUS_ASSERT(enumerationContext->valid());
    QueueIdStack x;
//  VCOUT << "create request" << endl;
//  CIMEnumerateInstancesRequestMessage* request =
//      new CIMEnumerateInstancesRequestMessage(
//          "", CIMNamespaceName(), CIMName(), true, true,
//          true, CIMPropertyList(),x);

//  VCOUT << "Create poA" << endl;
//  OperationAggregate* poA = new OperationAggregate(
//      new CIMEnumerateInstancesRequestMessage(*request),
//          request->className,
//          request->nameSpace,
//          0,
//          false, true);

    VCOUT << "add response instance to cache" << endl;
    // Add one instance to cache.  Should not cause wait to end.
    {
        CIMEnumerateInstancesResponseMessage* response
            =  new CIMEnumerateInstancesResponseMessage(
                "",
                CIMException(),
                x);

        enumerationContext->putCache((CIMResponseMessage*&)response, false);
        delete(response);
    }
    // signal again after setting providersComplete.
    // This one should wake up the condition.
    sleep(1);
    threadComplete = true;
    VCOUT << "Thread Sleep complete" << endl;
    {
        // add second instance to cache
        CIMEnumerateInstancesResponseMessage* response
            =  new CIMEnumerateInstancesResponseMessage(
                "",
                CIMException(),
                x);

        enumerationContext->putCache((CIMResponseMessage*&)response, true);
        delete(response);
    }
    VCOUT << "setresponseCacheSize contains "
          << enumerationContext->responseCacheSize()
          <<  "items. ThreadComplete" << endl;
    return ThreadReturnType(0);
}

// Test cache size  setting and getCacheResponseData
void test07()
{
    VCOUT << "test07 Start" << endl;
    // define an EnumerationContext
    EnumerationContextTable* enumTable = new EnumerationContextTable(
        1000,    // max OpenContexts
        30,      // defaultInteroperationTimeout
        1000);   // responseCache max size

    // Create an enumeration context
    String enContextIdStr;
    EnumerationContext* en = createEnumContext(enumTable,
                                                 enContextIdStr);

    PEGASUS_TEST_ASSERT(en->responseCacheSize() == 0);
    threadComplete = false;
    // Create a thread with Enumeration Context as parameter
    Thread * th = new Thread(setresponseCacheSize, (void *)en, false);

    th->run();

    // call the testresponseCacheSize Condition. Should return only when the
    // thread has signaled.
    VCOUT << "getCacheResponseData Wait" << endl;
    CIMResponseData resp(CIMResponseData::RESP_INSTANCES);

    // Sleep to allow thread to complete
    sleep(2);

    cout << "ResponseCacheSize() " << en->responseCacheSize() << endl;
    PEGASUS_TEST_ASSERT(en->responseCacheSize() == 2);
    en->getCache(1, resp);

    PEGASUS_TEST_ASSERT(resp.size() == 1);

    PEGASUS_TEST_ASSERT(en->responseCacheSize() == 1);
    VCOUT << "getCacheResponseData getCache returned" << endl;

    // get the second response

    CIMResponseData resp1(CIMResponseData::RESP_INSTANCES);
    VCOUT << "Cache Size " << en->responseCacheSize() << endl;;
    en->getCache(1, resp1);

    PEGASUS_TEST_ASSERT(resp1.size() == 1);
    PEGASUS_TEST_ASSERT(en->responseCacheSize() == 0);

    PEGASUS_TEST_ASSERT(threadComplete);

    th->join();
    delete th;

    enumTable->removeContextTable();
    delete(enumTable);
}


int main(int argc, char** argv)
{
    verbose = getenv("PEGASUS_TEST_VERBOSE") ? true : false;
    test01();
    test02();

    // putcache and getResponseCache test functions.
    test06();

    // test getcache function with counts less than what is in cache.
    // Issue with the test for the moment.  KS 26 July 2014
    //test07();

    cout << argv[0] << " +++++ passed all tests" << endl;

    return 0;
}
