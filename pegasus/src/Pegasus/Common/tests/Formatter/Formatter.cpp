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
#include <iostream>
#include <Pegasus/Common/Formatter.h>

PEGASUS_USING_PEGASUS;
PEGASUS_USING_STD;

#define DDD(X) // X
//#define DDD(X) X
static Boolean verbose;

int main(int, char** argv)
{
    verbose = getenv("PEGASUS_TEST_VERBOSE") ? true : false;

    {
        String str = Formatter::format(
            "\\$dollar\\$ X=$0, Y=$1, Z=$2, X=$0", 88, "Hello World", 7.5);

        if (verbose)
        {
            cout << "str[" << str << "]" << endl;
        }
    }

    // test for Uint32 expressions.

    {
        Uint32 test = 99;
        Uint32 big = 32999;

        String str2 = Formatter::format("test $0 big $1", test, big);

        if (verbose)
            cout << "str2[" << str2 << "]" << endl;

        PEGASUS_TEST_ASSERT(Formatter::format("[TRUE$0]", test) == "[TRUE99]");
    }
    // Test for booleans.
    {
        Boolean flag = true;
        Boolean flag2 = false;

        String str = Formatter::format(" TRUE $0 FALSE $1 FALSE $2",
                         flag, !flag, flag2);

        if (verbose)
            cout << "str[" << str << "]" << endl;

        PEGASUS_TEST_ASSERT(
            Formatter::format("TRUE$0", flag ) == "TRUEtrue");
        PEGASUS_TEST_ASSERT(
            Formatter::format("FALSE$0", flag2 ) == "FALSEfalse");
    }

    // Tests for Uint32 and Sint32
    {
        Uint32 test1 = 9999;
        Sint32 test2 = 9999;
        String str = Formatter::format("\\$test1 $0 \\$test2 $1", test1,
                test2);

        if (verbose)
            cout << "str[" << str << "]" << endl;

        PEGASUS_TEST_ASSERT(
            Formatter::format("[TRUE$0]", test1 ) == "[TRUE9999]");
        PEGASUS_TEST_ASSERT(
            Formatter::format("[TRUE$0]", test2 ) == "[TRUE9999]");
    }

    // Test for 64 bit signed integer
    {
        Sint64 test1 = PEGASUS_SINT64_LITERAL(0x7fffffffffffffff);
        Sint64 test2 = PEGASUS_SINT64_LITERAL(0x8000000000000000);
        Sint64 test3 = PEGASUS_SINT64_LITERAL(0xffffffffffffffff);
        String str = Formatter::format(
            "test1 $0 test2 $1 test3 $2", test1, test2, test3);

        if (verbose)
        {
            cout << "str[" << str << "]" << endl;
        }

        PEGASUS_TEST_ASSERT(Formatter::format("[TRUE$0]", test1) ==
            "[TRUE9223372036854775807]");
        PEGASUS_TEST_ASSERT(Formatter::format("[TRUE$0]", test2) ==
            "[TRUE-9223372036854775808]");
        PEGASUS_TEST_ASSERT(Formatter::format("[TRUE$0]", test3) ==
            "[TRUE-1]");
    }

    // Test for 64 bit unsigned integer
    {
        Uint64 test1 = PEGASUS_UINT64_LITERAL(0x7fffffffffffffff);
        Uint64 test2 = PEGASUS_UINT64_LITERAL(0x8000000000000000);
        Uint64 test3 = PEGASUS_UINT64_LITERAL(0xffffffffffffffff);
        String str = Formatter::format(
            "test1 $0 test2 $1 test3 $2", test1, test2, test3);

        if (verbose)
        {
            cout << "str[" << str << "]" << endl;
        }

        PEGASUS_TEST_ASSERT(Formatter::format("[TRUE$0]", test1) ==
            "[TRUE9223372036854775807]");
        PEGASUS_TEST_ASSERT(Formatter::format("[TRUE$0]", test2) ==
            "[TRUE9223372036854775808]");
        PEGASUS_TEST_ASSERT(Formatter::format("[TRUE$0]", test3) ==
            "[TRUE18446744073709551615]");
    }

    // Test for all  10 parameters in the string
    {
        Uint32 t0 = 1;
        Uint32 t1 = 1;
        Uint32 t2 = 2;
        Uint32 t3 = 3;
        Uint32 t4 = 4;
        Uint32 t5 = 5;
        Uint32 t6 = 6;
        Uint32 t7 = 7;
        Uint32 t8 = 8;
        Uint32 t9 = 9;

        String str = Formatter::format("[$0$1$2$3$4$5$6$7$8$9]",
                           t0,t1,t2,t3,t4,t5,t6,t7,t8,t9);

        if (verbose)
                cout << "str[" << str << "]" << endl;

        PEGASUS_TEST_ASSERT(Formatter::format("[$0$1$2$3$4$5$6$7$8$9]",
                           t0,t1,t2,t3,t4,t5,t6,t7,t8,t9)
                           == "[1123456789]");
    }

    cout << argv[0] << " +++++ passed all tests" << endl;

    return 0;
}
