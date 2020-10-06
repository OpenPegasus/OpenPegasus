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

#include <Pegasus/Common/UintArgs.h>
#include <Pegasus/Common/String.h>
#include <Pegasus/Common/PegasusAssert.h>

PEGASUS_USING_STD;
PEGASUS_USING_PEGASUS;

Boolean verbose;
#define VCOUT if (verbose) cout

void foo32(const Uint32Arg& arg)
{
    PEGASUS_TEST_ASSERT(arg.getValue() == 1234);
    PEGASUS_TEST_ASSERT(arg.isNull() == false);
}

void foo64(const Uint64Arg& arg)
{
    PEGASUS_TEST_ASSERT(arg.getValue() == 12345678);
    PEGASUS_TEST_ASSERT(arg.isNull() == false);
}

void testUint32()
{

    {
        // Test for Null value constructor and toString
        Uint32Arg argNull;

        PEGASUS_TEST_ASSERT(argNull.isNull() == true);
        String x =  argNull.toString();
        VCOUT << x << endl;
        PEGASUS_TEST_ASSERT(x == String("NULL"));

        // set value and test for that value
        Uint32Arg arg(10);

        VCOUT << arg.toString() << endl;
        PEGASUS_TEST_ASSERT(arg.toString() == String("10"));

        PEGASUS_TEST_ASSERT(arg.getValue() == 10);
        PEGASUS_TEST_ASSERT(arg.isNull() == false);

        // test resetting to Null
        arg.setNullValue();
        PEGASUS_TEST_ASSERT(arg.getValue() == 0);
        PEGASUS_TEST_ASSERT(arg.isNull() == true);

        VCOUT << arg.toString() << endl;

        PEGASUS_TEST_ASSERT(arg.toString() == String("NULL"));
    }

    {
        // test starting with null constructor and setting value
        Uint32Arg arg;

        PEGASUS_TEST_ASSERT(arg.getValue() == 0);
        PEGASUS_TEST_ASSERT(arg.isNull() == true);

        arg.setValue(10);
        PEGASUS_TEST_ASSERT(arg.getValue() == 10);
        PEGASUS_TEST_ASSERT(arg.isNull() == false);
    }

    {
        // test copy operations.
        Uint32Arg arg;
        PEGASUS_TEST_ASSERT(arg.getValue() == 0);
        PEGASUS_TEST_ASSERT(arg.isNull() == true);

        Uint32Arg arg2 = arg;
        PEGASUS_TEST_ASSERT(arg2.getValue() == 0);
        PEGASUS_TEST_ASSERT(arg2.isNull() == true);

        arg.setValue(10);
        PEGASUS_TEST_ASSERT(arg.getValue() == 10);
        PEGASUS_TEST_ASSERT(arg.isNull() == false);

        Uint32Arg arg3 = arg;
        PEGASUS_TEST_ASSERT(arg3.getValue() == 10);
        PEGASUS_TEST_ASSERT(arg3.isNull() == false);
    }

    {
        // test setting to max Uint32 value
        Uint32Arg arg;
        arg.setValue(4294967295UL);

        PEGASUS_TEST_ASSERT(arg.getValue() == 4294967295UL);
        PEGASUS_TEST_ASSERT(arg.isNull() == false);

        VCOUT << arg.toString() << endl;
        PEGASUS_TEST_ASSERT(arg.toString() == String("4294967295"));
    }
    {
        // test equal function
        Uint32Arg arg1;
        Uint32Arg arg2;
        PEGASUS_TEST_ASSERT(arg1.equal(arg2));
        PEGASUS_TEST_ASSERT(arg1 == arg2);
        Uint32Arg arg3(987654);
        Uint32Arg arg4(987654);
        PEGASUS_TEST_ASSERT(arg3.equal(arg4));
        PEGASUS_TEST_ASSERT(arg3 == arg4);
        PEGASUS_TEST_ASSERT(!arg1.equal(arg3));
        PEGASUS_TEST_ASSERT(!(arg1 == arg3));
        arg4.setValue(2);
        PEGASUS_TEST_ASSERT(!arg3.equal(arg4));
        PEGASUS_TEST_ASSERT(!(arg3 == arg4));
    }

    // test as parameter of a call.
    foo32(1234);
}

void testUint64()
{
    {
        // test constructors and setting value and null
        Uint64Arg argNull;
        String x =  argNull.toString();
        VCOUT << x << endl;
        PEGASUS_TEST_ASSERT(x == String("NULL"));

        //PEGASUS_TEST_ASSERT(x == "NULL");

        Uint64Arg arg(102958);

        PEGASUS_TEST_ASSERT(arg.getValue() == 102958);
        PEGASUS_TEST_ASSERT(arg.isNull() == false);

        VCOUT << arg.toString() << endl;
        PEGASUS_TEST_ASSERT(arg.toString() == String("102958"));

        arg.setNullValue();
        PEGASUS_TEST_ASSERT(arg.getValue() == 0);
        PEGASUS_TEST_ASSERT(arg.isNull() == true);

        String val = arg.toString();
        VCOUT << val << endl;

        PEGASUS_TEST_ASSERT(val == String("NULL"));
    }

    {
        // test starting with Null constructor and setting value
        Uint64Arg arg;

        PEGASUS_TEST_ASSERT(arg.getValue() == 0);
        PEGASUS_TEST_ASSERT(arg.isNull() == true);

        arg.setValue(10);
        PEGASUS_TEST_ASSERT(arg.getValue() == 10);
        PEGASUS_TEST_ASSERT(arg.isNull() == false);
    }
   {
        // test copy operations.
        Uint64Arg arg;
        PEGASUS_TEST_ASSERT(arg.getValue() == 0);
        PEGASUS_TEST_ASSERT(arg.isNull() == true);

        Uint64Arg arg2 = arg;
        PEGASUS_TEST_ASSERT(arg2.getValue() == 0);
        PEGASUS_TEST_ASSERT(arg2.isNull() == true);

        arg.setValue(10);
        PEGASUS_TEST_ASSERT(arg.getValue() == 10);
        PEGASUS_TEST_ASSERT(arg.isNull() == false);

        Uint64Arg arg3 = arg;
        PEGASUS_TEST_ASSERT(arg3.getValue() == 10);
        PEGASUS_TEST_ASSERT(arg3.isNull() == false);
    }

    {
        // test setting to max Uint32 value
        Uint64Arg arg;
        arg.setValue(18446744073709551615ULL);

        PEGASUS_TEST_ASSERT(arg.getValue() == 18446744073709551615ULL);
        PEGASUS_TEST_ASSERT(arg.isNull() == false);

        VCOUT << arg.toString() << endl;
        PEGASUS_TEST_ASSERT(arg.toString() == String("18446744073709551615"));
    }
    {
        // test equal function
        Uint64Arg arg1;
        Uint64Arg arg2;
        PEGASUS_TEST_ASSERT(arg1.equal(arg2));
        PEGASUS_TEST_ASSERT(arg1 == arg2);
        Uint64Arg arg3(987654);
        Uint64Arg arg4(987654);
        PEGASUS_TEST_ASSERT(arg3.equal(arg4));
        PEGASUS_TEST_ASSERT(arg3 == arg4);
        PEGASUS_TEST_ASSERT(!arg1.equal(arg3));
        PEGASUS_TEST_ASSERT(!(arg1 == arg3));
        arg4.setValue(2);
        PEGASUS_TEST_ASSERT(!arg3.equal(arg4));
        PEGASUS_TEST_ASSERT(!(arg3 == arg4));
    }

    foo64(12345678);
}

int main(int argc, char** argv)
{
    verbose = getenv("PEGASUS_TEST_VERBOSE") ? true : false;

    testUint32();
    testUint64();

    cout << argv[0] << " +++++ passed all tests" << endl;
    return 0;
}
