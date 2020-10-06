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
#include <Pegasus/ProviderManager2/Default/ProviderStatus.h>
#include <Pegasus/Common/Time.h>
#include <Pegasus/Common/PegasusAssert.h>

#include <iostream>

PEGASUS_USING_PEGASUS;
PEGASUS_USING_STD;

static Boolean verbose;

void testInitialization()
{
    ProviderStatus status;

    PEGASUS_TEST_ASSERT(!status.isInitialized());

    AutoMutex lock1(status.getStatusMutex());

    // Test that the status mutex allows recursive locks
    AutoMutex lock2(status.getStatusMutex());

    status.setInitialized(true);
    PEGASUS_TEST_ASSERT(status.isInitialized());

    status.setModule((ProviderModule*) 5);
    PEGASUS_TEST_ASSERT(status.getModule() == (ProviderModule*) 5);

    CIMInstance providerInstance("Test_Class");
    status.setProviderInstance(providerInstance);
    PEGASUS_TEST_ASSERT(status.getProviderInstance().getClassName() ==
        "Test_Class");

    status.setInitialized(false);
    PEGASUS_TEST_ASSERT(!status.isInitialized());
    PEGASUS_TEST_ASSERT(status.getModule() == 0);
}

void testIndications()
{
    ProviderStatus status;
    status.setInitialized(true);

    PEGASUS_TEST_ASSERT(!status.getIndicationsEnabled());
    PEGASUS_TEST_ASSERT(status.isIdle());

    status.setIndicationsEnabled(true);
    PEGASUS_TEST_ASSERT(status.getIndicationsEnabled());
    PEGASUS_TEST_ASSERT(!status.isIdle());

    PEGASUS_TEST_ASSERT(!status.testSubscriptions());
    PEGASUS_TEST_ASSERT(status.testIfZeroAndIncrementSubscriptions());
    PEGASUS_TEST_ASSERT(!status.testIfZeroAndIncrementSubscriptions());
    PEGASUS_TEST_ASSERT(status.testSubscriptions());
    PEGASUS_TEST_ASSERT(!status.decrementSubscriptionsAndTestIfZero());
    PEGASUS_TEST_ASSERT(status.decrementSubscriptionsAndTestIfZero());
    PEGASUS_TEST_ASSERT(status.testIfZeroAndIncrementSubscriptions());
    status.resetSubscriptions();
    PEGASUS_TEST_ASSERT(status.getIndicationsEnabled());
    PEGASUS_TEST_ASSERT(!status.isIdle());
    PEGASUS_TEST_ASSERT(!status.testSubscriptions());
    PEGASUS_TEST_ASSERT(status.testIfZeroAndIncrementSubscriptions());

    status.setInitialized(false);
    PEGASUS_TEST_ASSERT(!status.getIndicationsEnabled());
    PEGASUS_TEST_ASSERT(!status.isIdle());
    PEGASUS_TEST_ASSERT(!status.testSubscriptions());
}

void testOperations()
{
    ProviderStatus status;

    PEGASUS_TEST_ASSERT(!status.isIdle());
    PEGASUS_TEST_ASSERT(status.numCurrentOperations() == 0);
    status.setInitialized(true);

    PEGASUS_TEST_ASSERT(status.isIdle());
    PEGASUS_TEST_ASSERT(status.numCurrentOperations() == 0);
    status.operationBegin();
    PEGASUS_TEST_ASSERT(!status.isIdle());
    PEGASUS_TEST_ASSERT(status.numCurrentOperations() == 1);
    status.operationBegin();
    PEGASUS_TEST_ASSERT(!status.isIdle());
    PEGASUS_TEST_ASSERT(status.numCurrentOperations() == 2);
    status.operationEnd();
    PEGASUS_TEST_ASSERT(!status.isIdle());
    PEGASUS_TEST_ASSERT(status.numCurrentOperations() == 1);
    status.operationEnd();
    PEGASUS_TEST_ASSERT(status.isIdle());
    PEGASUS_TEST_ASSERT(status.numCurrentOperations() == 0);

    struct timeval lastOperationTime;
    status.getLastOperationEndTime(&lastOperationTime);

    // Test that the operation time is within one second of now
    struct timeval now;
    Time::gettimeofday(&now);
    PEGASUS_TEST_ASSERT(now.tv_sec >= lastOperationTime.tv_sec);
    PEGASUS_TEST_ASSERT(now.tv_sec <= lastOperationTime.tv_sec + 1);

    status.operationBegin();
    PEGASUS_TEST_ASSERT(!status.isIdle());
    PEGASUS_TEST_ASSERT(status.numCurrentOperations() == 1);

    status.setInitialized(false);
    PEGASUS_TEST_ASSERT(!status.isIdle());
    PEGASUS_TEST_ASSERT(status.numCurrentOperations() == 0);
}

void testCIMOMHandle()
{
    ProviderStatus status;
    status.setInitialized(true);

    status.setCIMOMHandle((CIMOMHandle*) 10);
    PEGASUS_TEST_ASSERT(status.getCIMOMHandle() == (CIMOMHandle*) 10);

    status.setInitialized(false);
    PEGASUS_TEST_ASSERT(status.getCIMOMHandle() == 0);

    status.setInitialized(true);
    CIMOMHandle cimomHandle;
    status.setCIMOMHandle(&cimomHandle);
    PEGASUS_TEST_ASSERT(status.isIdle());

    cimomHandle.disallowProviderUnload();
    PEGASUS_TEST_ASSERT(!status.isIdle());
    cimomHandle.disallowProviderUnload();
    PEGASUS_TEST_ASSERT(!status.isIdle());
    cimomHandle.allowProviderUnload();
    PEGASUS_TEST_ASSERT(!status.isIdle());
    cimomHandle.allowProviderUnload();
    PEGASUS_TEST_ASSERT(status.isIdle());

    // An extra allowProviderUnload() call should be ignored
    cimomHandle.allowProviderUnload();
    PEGASUS_TEST_ASSERT(status.isIdle());
}

int main(int, char** argv)
{
    verbose = getenv ("PEGASUS_TEST_VERBOSE") ? true : false;

    try
    {
        testInitialization();
        testIndications();
        testOperations();
        testCIMOMHandle();
    }
    catch (Exception& e)
    {
        cout << "Caught unexpected exception: " << e.getMessage() << endl;
        return 1;
    }
    catch (...)
    {
        cout << "Caught unexpected exception" << endl;
        return 1;
    }

    cout << argv[0] << " +++++ passed all tests" << endl;

    return 0;
}
