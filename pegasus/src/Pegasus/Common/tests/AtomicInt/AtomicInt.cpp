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
//%/////////////////////////////////////////////////////////////////////////////

#include <Pegasus/Common/PegasusAssert.h>
#include <Pegasus/Common/Thread.h>
#include <Pegasus/Common/InternalException.h>

PEGASUS_USING_PEGASUS;
PEGASUS_USING_STD;

void test01()
{
    try
    {
        Boolean verbose = (getenv("PEGASUS_TEST_VERBOSE")) ? true : false;

    AtomicInt i,j,ii,jj;

    if (verbose) {
        cout << "Testing: i++, ++ii, i--, --i   "<< endl;
    }
    i = 5;
    i++;
    PEGASUS_TEST_ASSERT( i.get() == 6 );
    i++;
    PEGASUS_TEST_ASSERT( i.get() == 7 );
    i--;
    PEGASUS_TEST_ASSERT( i.get() == 6 );
    i--;
    PEGASUS_TEST_ASSERT( i.get() == 5 );

    if (verbose)
        cout << "Testing: i+Uint32, i+AtomicInt, i-Uint32, etc.. "<<endl;
    }
    catch (Exception & e)
    {
        cout << "Exception: " << e.getMessage () << endl;
        exit (1);
    }
}

static AtomicInt _ai1(0);
static AtomicInt _ai2(0);

ThreadReturnType PEGASUS_THREAD_CDECL test_thread(void* parm)
{
    for (;;)
    {
    const size_t N = 100000;

    for (size_t i = 0; i < N; i++)
    {
        for (size_t j = 0; j < 3; j++)
        {
        _ai1++;
        _ai2++;
        }

        for (size_t j = 0; j < 3; j++)
        {
        _ai1.decAndTestIfZero();
        _ai2.decAndTestIfZero();
        }
    }

    break;
    }

    return 0;
}

void test02()
{
    Thread t1(test_thread, 0, false);
    t1.run();

    Thread t2(test_thread, 0, false);
    t2.run();

    t1.join();
    t2.join();

    PEGASUS_TEST_ASSERT(_ai1.get() == 0);
    PEGASUS_TEST_ASSERT(_ai2.get() == 0);
}

int main(int, char** argv)
{
    test01();
    test02();

    cout << argv[0] << " +++++ passed all tests" << endl;

    return 0;
}
