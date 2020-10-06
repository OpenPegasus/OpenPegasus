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

#include <Pegasus/Common/AutoPtr.h>
#include <Pegasus/Common/String.h>
#include <Pegasus/Common/PegasusAssert.h>

PEGASUS_USING_STD;
PEGASUS_USING_PEGASUS;

class TestClass
{
public:
    TestClass()
    {
        value = 0;
        instanceCount++;
    }

    // Do not pass a value of -1 to this constructor.
    TestClass(int i)
    {
        PEGASUS_TEST_ASSERT(i != -1);
        value = i;
        instanceCount++;
    }

    ~TestClass()
    {
        PEGASUS_TEST_ASSERT(value != -1);
        value = -1;
        instanceCount--;
    }

    // Do not pass a value of -1 to this method.
    void setValue(int i)
    {
        PEGASUS_TEST_ASSERT(i != -1);
        PEGASUS_TEST_ASSERT(value != -1);
        value = i;
    }

    int getValue()
    {
        PEGASUS_TEST_ASSERT(value != -1);
        return value;
    }

    static int instanceCount;

private:
    int value;
};

int TestClass::instanceCount = 0;

void testConstructorAndDestructor()
{
    // Test default constructor
    {
        PEGASUS_TEST_ASSERT(TestClass::instanceCount == 0);

        AutoPtr<TestClass> autoPtr;
        AutoArrayPtr<TestClass> autoArrayPtr;

        PEGASUS_TEST_ASSERT(autoPtr.get() == 0);
        PEGASUS_TEST_ASSERT(autoArrayPtr.get() == 0);
        PEGASUS_TEST_ASSERT(TestClass::instanceCount == 0);
    }
    PEGASUS_TEST_ASSERT(TestClass::instanceCount == 0);

    // Test constructor with pointer argument
    {
        TestClass* testClassPtr = new TestClass();
        TestClass* testClassArrayPtr = new TestClass[4];

        AutoPtr<TestClass> autoPtr(testClassPtr);
        AutoArrayPtr<TestClass> autoArrayPtr(testClassArrayPtr);

        PEGASUS_TEST_ASSERT(autoPtr.get() == testClassPtr);
        PEGASUS_TEST_ASSERT(autoArrayPtr.get() == testClassArrayPtr);
        PEGASUS_TEST_ASSERT(TestClass::instanceCount == 5);
    }
    PEGASUS_TEST_ASSERT(TestClass::instanceCount == 0);
}

void testMethods()
{
    // Test get and reset methods
    {
        TestClass* testClassPtr = new TestClass();
        TestClass* testClassArrayPtr = new TestClass[4];

        PEGASUS_TEST_ASSERT(TestClass::instanceCount == 5);

        AutoPtr<TestClass> autoPtr;
        AutoArrayPtr<TestClass> autoArrayPtr;

        PEGASUS_TEST_ASSERT(autoPtr.get() == 0);
        PEGASUS_TEST_ASSERT(autoArrayPtr.get() == 0);
        PEGASUS_TEST_ASSERT(TestClass::instanceCount == 5);

        autoPtr.reset(testClassPtr);
        autoArrayPtr.reset(testClassArrayPtr);

        PEGASUS_TEST_ASSERT(autoPtr.get() == testClassPtr);
        PEGASUS_TEST_ASSERT(autoArrayPtr.get() == testClassArrayPtr);
        PEGASUS_TEST_ASSERT(TestClass::instanceCount == 5);

        autoPtr.reset();
        autoArrayPtr.reset();

        PEGASUS_TEST_ASSERT(autoPtr.get() == 0);
        PEGASUS_TEST_ASSERT(autoArrayPtr.get() == 0);
        PEGASUS_TEST_ASSERT(TestClass::instanceCount == 0);
    }
    PEGASUS_TEST_ASSERT(TestClass::instanceCount == 0);

    // Test release method
    {
        TestClass* testClassPtr = new TestClass();
        TestClass* testClassArrayPtr = new TestClass[4];

        AutoPtr<TestClass> autoPtr(testClassPtr);
        AutoArrayPtr<TestClass> autoArrayPtr(testClassArrayPtr);

        PEGASUS_TEST_ASSERT(autoPtr.get() == testClassPtr);
        PEGASUS_TEST_ASSERT(autoArrayPtr.get() == testClassArrayPtr);
        PEGASUS_TEST_ASSERT(TestClass::instanceCount == 5);

        autoPtr.release();
        autoArrayPtr.release();

        PEGASUS_TEST_ASSERT(autoPtr.get() == 0);
        PEGASUS_TEST_ASSERT(autoArrayPtr.get() == 0);
        PEGASUS_TEST_ASSERT(TestClass::instanceCount == 5);

        delete testClassPtr;
        delete [] testClassArrayPtr;
    }
    PEGASUS_TEST_ASSERT(TestClass::instanceCount == 0);
}

void testOperators()
{
    // Test operator*
    {
        TestClass* testClassPtr = new TestClass(5);
        TestClass* testClassArrayPtr = new TestClass[4];
        testClassArrayPtr[0].setValue(50);

        AutoPtr<TestClass> autoPtr(testClassPtr);
        AutoArrayPtr<TestClass> autoArrayPtr(testClassArrayPtr);

        PEGASUS_TEST_ASSERT(autoPtr.get() == testClassPtr);
        PEGASUS_TEST_ASSERT(autoArrayPtr.get() == testClassArrayPtr);

        PEGASUS_TEST_ASSERT(autoPtr.get()->getValue() == 5);
        PEGASUS_TEST_ASSERT(autoArrayPtr.get()[0].getValue() == 50);
        PEGASUS_TEST_ASSERT((*autoPtr).getValue() == 5);
        PEGASUS_TEST_ASSERT((*autoArrayPtr).getValue() == 50);
    }

    // Test operator->
    {
        TestClass* testClassPtr = new TestClass(5);
        TestClass* testClassArrayPtr = new TestClass[4];
        testClassArrayPtr[0].setValue(50);

        AutoPtr<TestClass> autoPtr(testClassPtr);
        AutoArrayPtr<TestClass> autoArrayPtr(testClassArrayPtr);

        PEGASUS_TEST_ASSERT(autoPtr.get() == testClassPtr);
        PEGASUS_TEST_ASSERT(autoArrayPtr.get() == testClassArrayPtr);

        PEGASUS_TEST_ASSERT(autoPtr.get()->getValue() == 5);
        PEGASUS_TEST_ASSERT(autoArrayPtr.get()[0].getValue() == 50);
        PEGASUS_TEST_ASSERT(autoPtr->getValue() == 5);
        PEGASUS_TEST_ASSERT(autoArrayPtr->getValue() == 50);
    }

    // Test operator[]
    {
        TestClass* testClassArrayPtr = new TestClass[4];
        testClassArrayPtr[1].setValue(1000);
        testClassArrayPtr[2].setValue(80);

        AutoArrayPtr<TestClass> autoArrayPtr(testClassArrayPtr);

        PEGASUS_TEST_ASSERT(autoArrayPtr.get() == testClassArrayPtr);

        PEGASUS_TEST_ASSERT(autoArrayPtr.get()[1].getValue() == 1000);
        PEGASUS_TEST_ASSERT(autoArrayPtr[1].getValue() == 1000);
        PEGASUS_TEST_ASSERT(autoArrayPtr.get()[2].getValue() == 80);
        PEGASUS_TEST_ASSERT(autoArrayPtr[2].getValue() == 80);

        autoArrayPtr[1].setValue(2000);

        PEGASUS_TEST_ASSERT(autoArrayPtr.get()[1].getValue() == 2000);
        PEGASUS_TEST_ASSERT(autoArrayPtr[1].getValue() == 2000);
        PEGASUS_TEST_ASSERT(autoArrayPtr.get()[2].getValue() == 80);
        PEGASUS_TEST_ASSERT(autoArrayPtr[2].getValue() == 80);
    }
}

int main(int, char** argv)
{
    testConstructorAndDestructor();
    testMethods();
    testOperators();

    cout << argv[0] << " +++++ passed all tests" << endl;
    return 0;
}
