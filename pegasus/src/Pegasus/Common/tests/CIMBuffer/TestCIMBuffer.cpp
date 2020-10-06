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
#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/CIMBuffer.h>

PEGASUS_USING_PEGASUS;
PEGASUS_USING_STD;

// test uint32 get and put
void test1()
{
    CIMBuffer cb(1024);
    const Uint32 N = 10000;

    for (Uint32 i = 0; i < N; i++)
    {
        cb.putUint32(i);
    }

    cb.rewind();

    for (Uint32 i = 0; i < N; i++)
    {
        Uint32 x = 0;
        PEGASUS_TEST_ASSERT(cb.getUint32(x));
        PEGASUS_TEST_ASSERT(x == i);
    }
}

// test put and get String
void test2()
{
    CIMBuffer cb(1024);
    const Uint32 N = 10000;

    for (Uint32 i = 0; i < N; i++)
    {
        char buffer[32];
        sprintf(buffer, "%u", i);
        cb.putString(buffer);
    }

    cb.rewind();

    for (Uint32 i = 0; i < N; i++)
    {
        String x;
        PEGASUS_TEST_ASSERT(cb.getString(x));
        char buffer[32];
        sprintf(buffer, "%u", i);
        PEGASUS_TEST_ASSERT(x == buffer);
    }
}

// test array of Uint32
void test3()
{
    CIMBuffer cb(1024);
    const Uint32 N = 10000;

    Array<Uint32> x1;

    for (Uint32 i = 0; i < N; i++)
    {
        x1.append(i);
    }

    cb.putUint32A(x1);

    cb.rewind();
    Array<Uint32> x2;
    PEGASUS_TEST_ASSERT(cb.getUint32A(x2));

    PEGASUS_TEST_ASSERT(x2.size() == N);

    for (Uint32 i = 0; i < N; i++)
    {
        PEGASUS_TEST_ASSERT(x1[i] == x2[i]);
    }
}

// test CIMValue puts and gets
void test4()
{
    CIMBuffer cb(1024);
    CIMValue a1;
    CIMValue a2;
    CIMValue a3;
    CIMValue b1;
    CIMValue b2;
    CIMValue b3;

    a1.set(String("Hello World"));
    a2.set(Boolean(false));
    a3.set(Uint32(12345));

    cb.putValue(a1);
    cb.putValue(a2);
    cb.putValue(a3);

    cb.rewind();

    PEGASUS_TEST_ASSERT(cb.getValue(b1));
    PEGASUS_TEST_ASSERT(cb.getValue(b2));
    PEGASUS_TEST_ASSERT(cb.getValue(b3));

    PEGASUS_TEST_ASSERT(a1 == b1);
    PEGASUS_TEST_ASSERT(a2 == b2);
    PEGASUS_TEST_ASSERT(a3 == b3);
}

// test CIMOBjectPath Puts and Gets.
void test5()
{
    CIMBuffer cb(1024);

    CIMObjectPath cop1("Class.Key1=\"hello\",Key2=true,Key3=12345");
    cb.putObjectPath(cop1);
    cb.rewind();
    CIMObjectPath cop2;

    PEGASUS_TEST_ASSERT(cb.getObjectPath(cop2));

    PEGASUS_TEST_ASSERT(cop1 == cop2);
    PEGASUS_TEST_ASSERT(cop1.toString() == cop2.toString());
}

void test6()
{
    CIMBuffer cb(1024);

    CIMInstance ci1("MyClass");
    ci1.addProperty(CIMProperty("Number", Uint32(12345)));
    ci1.addProperty(CIMProperty("Flag", Boolean(true)));
    ci1.addProperty(CIMProperty("Message", String("Hello")));
    cb.putInstance(ci1);

    cb.rewind();
    CIMInstance ci2;
    PEGASUS_TEST_ASSERT(cb.getInstance(ci2));

    PEGASUS_TEST_ASSERT(ci1.identical(ci2));
}

void test7()
{
    CIMBuffer cb(1024);

    CIMObjectPath cop1;
    cb.putObjectPath(cop1);
    cb.rewind();
    CIMObjectPath cop2;

    PEGASUS_TEST_ASSERT(cb.getObjectPath(cop2));

    PEGASUS_TEST_ASSERT(cop1 == cop2);
    PEGASUS_TEST_ASSERT(cop1.toString() == cop2.toString());
}

void test8()
{
    CIMBuffer cb;

    CIMObjectPath cop1;
    cb.putObjectPath(cop1);
    cb.rewind();
    CIMObjectPath cop2;

    PEGASUS_TEST_ASSERT(cb.getObjectPath(cop2));

    PEGASUS_TEST_ASSERT(cop1 == cop2);
    PEGASUS_TEST_ASSERT(cop1.toString() == cop2.toString());
}

void test9()
{
    CIMBuffer cb;

    cb.putBoolean(true);
    cb.putBoolean(false);
    cb.putBoolean(true);
    cb.putBoolean(true);

    Array<Boolean> a;
    a.append(true);
    a.append(false);
    a.append(true);
    a.append(true);
    cb.putBooleanA(a);

    cb.putUint32(1234);

    cb.rewind();

    Boolean x;
    PEGASUS_TEST_ASSERT(cb.getBoolean(x) && x == true);
    PEGASUS_TEST_ASSERT(cb.getBoolean(x) && x == false);
    PEGASUS_TEST_ASSERT(cb.getBoolean(x) && x == true);
    PEGASUS_TEST_ASSERT(cb.getBoolean(x) && x == true);

    Array<Boolean> b;
    PEGASUS_TEST_ASSERT(cb.getBooleanA(b) && a == b);
    PEGASUS_TEST_ASSERT(a.size() == 4);
    PEGASUS_TEST_ASSERT(a.size() == b.size());
    PEGASUS_TEST_ASSERT(a[0] == b[0]);
    PEGASUS_TEST_ASSERT(a[1] == b[1]);
    PEGASUS_TEST_ASSERT(a[2] == b[2]);
    PEGASUS_TEST_ASSERT(a[3] == b[3]);

    Uint32 y;
    PEGASUS_TEST_ASSERT(cb.getUint32(y) && y == 1234);
}

// test dateTime puts and gets.
void test10()
{
    CIMBuffer cb;
    CIMDateTime dateTime1("20090602104500.123456+330");

    cb.putDateTime(dateTime1);

    cb.rewind();

    CIMDateTime dateTime2;
    PEGASUS_TEST_ASSERT(cb.getDateTime(dateTime2));
    PEGASUS_TEST_ASSERT(dateTime1.toString() == dateTime2.toString());
}

// test Uing32Arg Puts and gets
void test11()
{
    CIMBuffer cb;
    Uint32Arg a1;           // default init - Null
    Uint32Arg a2(2300);     // known value
    Uint32Arg a3;           // used to set value
    Uint32Arg a4(9000);     // Used to setNullValue
    a4.setNullValue();
    a3.setValue(2300);

    cb.putUint32Arg(a1);
    cb.putUint32Arg(a2);
    cb.putUint32Arg(a3);
    cb.putUint32Arg(a4);

    cb.rewind();
    Uint32Arg ra1;
    Uint32Arg ra2;
    Uint32Arg ra3;
    Uint32Arg ra4;
    PEGASUS_ASSERT(a1 == ra1);
    PEGASUS_ASSERT(a2 == ra2);
    PEGASUS_ASSERT(a3 == ra3);
    PEGASUS_ASSERT(a4 == ra4);

    PEGASUS_ASSERT(!(a3 == ra4));
}

int main(int, char** argv)
{
    test1();
    test2();
    test3();
    test4();
    test5();
    test6();
    test7();
    test8();
    test9();
    test10();

    cout << argv[0] << " +++++ passed all tests" << endl;
    return 0;
}
