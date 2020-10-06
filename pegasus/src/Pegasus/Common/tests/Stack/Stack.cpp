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

#include <Pegasus/Common/Stack.h>

#include <cstdlib>
#include <Pegasus/Common/PegasusAssert.h>

PEGASUS_USING_PEGASUS;
PEGASUS_USING_STD;

int main(int, char** argv)
{
    try
    {
        // Simple test with Uint32 Stack of push, pop, top, and tests.
        Stack<Uint32> s1;
        PEGASUS_TEST_ASSERT (s1.isEmpty());

        s1.push(1);
        PEGASUS_TEST_ASSERT(s1.size() == 1);
        PEGASUS_TEST_ASSERT (!s1.isEmpty());

        s1.push(2);
        PEGASUS_TEST_ASSERT(s1.size() == 2);
        PEGASUS_TEST_ASSERT (!s1.isEmpty());

        PEGASUS_TEST_ASSERT(s1.top() == 2);

        s1.pop();
        PEGASUS_TEST_ASSERT(s1.size() == 1);
        PEGASUS_TEST_ASSERT (!s1.isEmpty());

        s1.pop();
        PEGASUS_TEST_ASSERT(s1.size() == 0);
        PEGASUS_TEST_ASSERT (s1.isEmpty());

        // Performance tests

        Stack<Uint32> s2;
        Uint32 stackSize = 1000000;

        for (Uint32 i = 0; i < stackSize; i++)
            s2.push(i);

        PEGASUS_TEST_ASSERT(s2.size() == stackSize);
        for (Uint32 i = 0; i < stackSize; i++)
            s2.pop();

        // SF-HP
        Stack<Uint32> s3;
            s3 = s1;

        const Stack<Uint32> s4;

        // throws Stack underflow
        try
        {
            s4.top();
            PEGASUS_TEST_ASSERT(false);
        }
        catch(StackUnderflow&)
        {
        }

        // throws OutOfBounds
        try
        {
            Uint32 tmp = 314;
            tmp = s4[0];
            PEGASUS_TEST_ASSERT(314 == tmp);
            PEGASUS_TEST_ASSERT(false);
        }
        catch(IndexOutOfBoundsException&)
        {
        }

        s1.push(2);
        Uint32 tmp = s1[0];
        PEGASUS_TEST_ASSERT(2 == tmp);
    }
    catch (Exception& e)
    {
        cout << "Exception: " << e.getMessage() << endl;
        exit(1);
    }

    cout << argv[0] << " +++++ passed all tests" << endl;

    return 0;
}
