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
// Author: Mike Brasher (mbrasher@bmc.com)
//
// Modified By:
//
//%/////////////////////////////////////////////////////////////////////////////
#include <iostream>
#include <Pegasus/Common/PegasusAssert.h>
#include <Pegasus/Common/TimeValue.h>
#include <Pegasus/Common/System.h>
#include <Pegasus/Common/StringConversion.h>

PEGASUS_USING_STD;
PEGASUS_USING_PEGASUS;

static Boolean verbose;

int main(int, char** argv)
{
    verbose = getenv("PEGASUS_TEST_VERBOSE") ? true : false;

    TimeValue tv0( 3, 100000);   // set to 3 sec, 100,000 microseconds
    Uint64 ms = tv0.toMilliseconds();
    if (verbose)
    {
        char buffer[22];
        Uint32 size;
        cout << "milliseconds=" << Uint64ToString(buffer, ms, size) << endl;
    }
    PEGASUS_TEST_ASSERT(ms == (3 * 1000) + 100);

    tv0.setSeconds(4);
    tv0.setMicroseconds(200000);
    ms = tv0.toMilliseconds();
    PEGASUS_TEST_ASSERT(ms == (4 * 1000) + 200);

    // Test the tomilliseconds function
    TimeValue tvx(1,0);
    PEGASUS_TEST_ASSERT(tvx.toMilliseconds() == 1 * 1000);
    tvx.setSeconds(2);
    PEGASUS_TEST_ASSERT(tvx.toMilliseconds() == 2 * 1000);
    tvx.setMicroseconds(500000);
    PEGASUS_TEST_ASSERT(tvx.toMilliseconds() == ((2 * 1000) + 500));


    // Test time difference between two getCurrentTime calls.
    TimeValue tv1 = TimeValue::getCurrentTime();
    System::sleep(5);
    TimeValue tv2 = TimeValue::getCurrentTime();

    Uint64 milliseconds = tv2.toMilliseconds() - tv1.toMilliseconds();

    if (verbose)
    {
        char buffer[22];
        Uint32 size;

        cout << "tv1 " << tv1.getSeconds() << " Seconds "
             << tv1.getMicroseconds() << " microseconds "
             << Uint64ToString(buffer, tv1.toMilliseconds(), size)
             << " total milliseconds" << endl;
        cout << "tv2 " << tv2.getSeconds() << " Seconds "
             << tv2.getMicroseconds() << " microseconds "
             << Uint64ToString(buffer, tv2.toMilliseconds(), size)
             << " total milliseconds" <<  endl;
        cout << "milliseconds=" << Uint64ToString(buffer, milliseconds, size)
             << endl;
    }

    PEGASUS_TEST_ASSERT(milliseconds >= 4500 && milliseconds <= 5500);

    if (verbose)
        cout << System::getCurrentASCIITime() << endl;

    cout << argv[0] << " +++++ passed all tests" << endl;

    return 0;
}
