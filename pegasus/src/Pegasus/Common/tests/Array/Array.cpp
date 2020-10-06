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

#define NEED_STRING_EQUAL

#include <Pegasus/Common/PegasusAssert.h>
#include <Pegasus/Common/ArrayInternal.h>
#include <Pegasus/Common/InternalException.h>
#include <Pegasus/Common/String.h>
#include "Str.h"
#include "Int.h"

PEGASUS_USING_PEGASUS;
PEGASUS_USING_STD;

template<class T>
void Print(const Array<T>& arr)
{
    for (Uint32 i = 0; i < arr.size(); i++)
        cout << arr[i] << endl;
    cout << "-- end" << endl;
}

template<class STR>
void test01(STR*)
{
    Array<STR> arr(3, STR("Hello"));
    PEGASUS_TEST_ASSERT(arr.size() == 3);
    PEGASUS_TEST_ASSERT(arr[0] == STR("Hello"));
    PEGASUS_TEST_ASSERT(arr[1] == STR("Hello"));
    PEGASUS_TEST_ASSERT(arr[2] == STR("Hello"));
    if(getenv("PEGASUS_TEST_VERBOSE"))
        Print(arr);
}

template<class STR>
void test02(STR*)
{
    Array<STR> arr;
    PEGASUS_TEST_ASSERT(arr.size() == 0);

    arr.append("three");
    arr.append("four");
    arr.prepend("one");
    arr.prepend("zero");
    arr.insert(2, "two");

    PEGASUS_TEST_ASSERT(arr.size() == 5);
    PEGASUS_TEST_ASSERT(arr[0] == "zero");
    PEGASUS_TEST_ASSERT(arr[1] == "one");
    PEGASUS_TEST_ASSERT(arr[2] == "two");
    PEGASUS_TEST_ASSERT(arr[3] == "three");
    PEGASUS_TEST_ASSERT(arr[4] == "four");

    arr.remove(2);
    PEGASUS_TEST_ASSERT(arr.size() == 4);
    PEGASUS_TEST_ASSERT(arr[0] == "zero");
    PEGASUS_TEST_ASSERT(arr[1] == "one");
    PEGASUS_TEST_ASSERT(arr[2] == "three");
    PEGASUS_TEST_ASSERT(arr[3] == "four");

    // test removing multiple items.
    arr.remove(1, 2);
    PEGASUS_TEST_ASSERT(arr[0] == "zero");
    PEGASUS_TEST_ASSERT(arr[1] == "four");
    PEGASUS_TEST_ASSERT(arr.size() == 2);

    // test removing zero items
    arr.remove(1, 0);
    PEGASUS_TEST_ASSERT(arr.size() == 2);
    arr.remove(0, 0);
    PEGASUS_TEST_ASSERT(arr.size() == 2);
}

/* test for numeric arrays*/
template<class T>
void test03(const T*)
{
    Array<T> arr;

    arr.insert(0, 2);
    arr.insert(0, 1);
    arr.insert(0, 0);
    arr.append(3);
    arr.insert(4, 4);

    Array<T> arr2 = arr;

    Array<T> arr3;
    arr3 = arr2;
    arr = arr3;

    PEGASUS_TEST_ASSERT(arr.size() == 5);
    PEGASUS_TEST_ASSERT(arr[0] == 0);
    PEGASUS_TEST_ASSERT(arr[1] == 1);
    PEGASUS_TEST_ASSERT(arr[2] == 2);
    PEGASUS_TEST_ASSERT(arr[3] == 3);
    PEGASUS_TEST_ASSERT(arr[4] == 4);

    arr.remove(4);
    PEGASUS_TEST_ASSERT(arr.size() == 4);
    PEGASUS_TEST_ASSERT(arr[0] == 0);
    PEGASUS_TEST_ASSERT(arr[1] == 1);
    PEGASUS_TEST_ASSERT(arr[2] == 2);
    PEGASUS_TEST_ASSERT(arr[3] == 3);

    arr.remove(0);
    PEGASUS_TEST_ASSERT(arr.size() == 3);
    PEGASUS_TEST_ASSERT(arr[0] == 1);
    PEGASUS_TEST_ASSERT(arr[1] == 2);
    PEGASUS_TEST_ASSERT(arr[2] == 3);

    arr.remove(0);
    arr.remove(1);
    PEGASUS_TEST_ASSERT(arr.size() == 1);
    PEGASUS_TEST_ASSERT(arr[0] == 2);

    arr.remove(0);
    PEGASUS_TEST_ASSERT(arr.size() == 0);
}

void test04()
{
    Array<String> arr(3, "Hello");
    PEGASUS_TEST_ASSERT(arr.size() == 3);
    PEGASUS_TEST_ASSERT(arr[0] == "Hello");
    PEGASUS_TEST_ASSERT(arr[1] == "Hello");
    PEGASUS_TEST_ASSERT(arr[2] == "Hello");

    arr.remove(1, 2);
    PEGASUS_TEST_ASSERT(arr.size() == 1);

}

void test05()
{
    Array<String> arr;
    PEGASUS_TEST_ASSERT(arr.size() == 0);

    arr.append("three");
    arr.append("four");
    arr.prepend("one");
    arr.prepend("zero");
    arr.insert(2, "two");

    PEGASUS_TEST_ASSERT(arr.size() == 5);
    PEGASUS_TEST_ASSERT(arr[0] == "zero");
    PEGASUS_TEST_ASSERT(arr[1] == "one");
    PEGASUS_TEST_ASSERT(arr[2] == "two");
    PEGASUS_TEST_ASSERT(arr[3] == "three");
    PEGASUS_TEST_ASSERT(arr[4] == "four");

    arr.remove(2);
    PEGASUS_TEST_ASSERT(arr.size() == 4);
    PEGASUS_TEST_ASSERT(arr[0] == "zero");
    PEGASUS_TEST_ASSERT(arr[1] == "one");
    PEGASUS_TEST_ASSERT(arr[2] == "three");
    PEGASUS_TEST_ASSERT(arr[3] == "four");

    arr.remove(0,3);
    PEGASUS_TEST_ASSERT(arr.size() == 1);
    PEGASUS_TEST_ASSERT(arr[0] == "four");


}

void test15()
{
    {
        Array<Uint32> arr;
        PEGASUS_TEST_ASSERT(arr.size() == 0);
        Array<Uint32> arr2;
        PEGASUS_TEST_ASSERT(arr.size() == 0);
        arr.append(1);
        arr.append(2);
        arr.append(3);
        arr.append(4);
        PEGASUS_TEST_ASSERT(arr.size() == 4);
        arr2.append(5);
        arr2.append(6);
        arr2.append(7);
        arr2.append(8);
        PEGASUS_TEST_ASSERT(arr2.size() == 4);
        arr.appendArray(arr2);
        PEGASUS_TEST_ASSERT(arr.size() == 8);
        PEGASUS_TEST_ASSERT(arr2.size() == 4);
        for (Uint32 i = 0 ; i < 8 ; i++)
        {
            PEGASUS_TEST_ASSERT(arr[i] == i+1);
        }
    }
    {
        Array<String> arr;
        PEGASUS_TEST_ASSERT(arr.size() == 0);
        Array<String> arr2;
        PEGASUS_TEST_ASSERT(arr.size() == 0);
        arr.append("zero");
        arr.append("one");
        arr.append("two");
        arr.append("three");
        arr.append("four");
        PEGASUS_TEST_ASSERT(arr.size() == 5);
        arr2.append("five");
        arr2.append("six");
        arr2.append("seven");
        arr2.append("eight");
        PEGASUS_TEST_ASSERT(arr2.size() == 4);
        arr.appendArray(arr2);
        PEGASUS_TEST_ASSERT(arr.size() == 9);
        PEGASUS_TEST_ASSERT(arr2.size() == 4);
        PEGASUS_TEST_ASSERT(arr[0] == "zero");
        PEGASUS_TEST_ASSERT(arr[1] == "one");
        PEGASUS_TEST_ASSERT(arr[2] == "two");
        PEGASUS_TEST_ASSERT(arr[3] == "three");
        PEGASUS_TEST_ASSERT(arr[4] == "four");
        PEGASUS_TEST_ASSERT(arr[5] == "five");
        PEGASUS_TEST_ASSERT(arr[6] == "six");
        PEGASUS_TEST_ASSERT(arr[7] == "seven");
        PEGASUS_TEST_ASSERT(arr[8] == "eight");
    }

}

void test06()
{
    Boolean exceptionCaught;

    // Test constructor memory overflow
    exceptionCaught = false;
    try
    {
        Array<Uint32> arr(0xffff0000);
    }
    catch (const PEGASUS_STD(bad_alloc)&)
    {
        exceptionCaught = true;
    }
    PEGASUS_TEST_ASSERT(exceptionCaught);

    // Test constructor memory overflow
    exceptionCaught = false;
    try
    {
        Array<Uint32> arr(0xffff0000, 100);
    }
    catch (const PEGASUS_STD(bad_alloc)&)
    {
        exceptionCaught = true;
    }
    PEGASUS_TEST_ASSERT(exceptionCaught);

    // Test constructor memory overflow
    exceptionCaught = false;
    try
    {
        Uint32 myInt = 50;
        Array<Uint32> arr(&myInt, 0xffff0000);
    }
    catch (const PEGASUS_STD(bad_alloc)&)
    {
        exceptionCaught = true;
    }
    PEGASUS_TEST_ASSERT(exceptionCaught);

    // Test reserveCapacity memory overflow
    {
        Array<Uint32> arr(128);
        PEGASUS_TEST_ASSERT(arr.getCapacity() == 128);
        exceptionCaught = false;
        try
        {
            arr.reserveCapacity(0xffff0000);
        }
        catch (const PEGASUS_STD(bad_alloc)&)
        {
            exceptionCaught = true;
        }
        PEGASUS_TEST_ASSERT(exceptionCaught);
        PEGASUS_TEST_ASSERT(arr.getCapacity() == 128);
    }
}

// Test the append with count function with multiple CIM data types
void test07()
{
    // Test for the array append by count function.
    Array<String> Arr1;
    Array<String> Arr2;
    Array<String> Arr3;

    Arr1.append("100");
    Arr1.append("2000");

    Arr2.append("30000");
    Arr2.append("400000");
    // append from the above two arrays specific counts of objects.
    Arr3.append(Arr1.getData() ,1);

    Arr3.append(Arr2.getData() ,2);

    PEGASUS_TEST_ASSERT(Arr3.size() == 3);
    PEGASUS_TEST_ASSERT(Arr3[0] == "100");
    PEGASUS_TEST_ASSERT(Arr3[1] == "30000");
    PEGASUS_TEST_ASSERT(Arr3[2] == "400000");

    // Test for the array append by count function with Integers
    Array<Uint32> Arr4;
    Array<Uint32> Arr5;
    Array<Uint32> Arr6;

    Arr4.append(1);
    Arr4.append(2);

    Arr5.append(3);
    Arr5.append(4);
    // append from the above two arrays specific counts of objects.
    Arr6.append(Arr4.getData() ,1);
    Arr6.append(Arr5.getData() ,2);

    PEGASUS_TEST_ASSERT(Arr6.size() == 3);
    PEGASUS_TEST_ASSERT(Arr6[0] == 1);
    PEGASUS_TEST_ASSERT(Arr6[1] == 3);
    PEGASUS_TEST_ASSERT(Arr6[2] == 4);

}

int main(int, char** argv)
{
    try
    {
        test01((Str*)0);
        test02((Str*)0);
        test01((String*)0);
        test02((String*)0);
        test03((Int*)0);
        test03((int*)0);
        test04();
        test05();
        test15();
        test06();
        PEGASUS_TEST_ASSERT(Int::_count == 0);
        PEGASUS_TEST_ASSERT(Str::_constructions == Str::_destructions);
    }
    catch(const Exception& e)
    {
        cerr << argv[0] << " Exception: " << e.getMessage() << endl;
        exit(1);
    }

    cout << argv[0] << " +++++ passed all tests" << endl;

    return 0;
}
