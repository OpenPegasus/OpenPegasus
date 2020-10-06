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

#include <Pegasus/Common/SharedPtr.h>
#include <Pegasus/Common/PegasusAssert.h>
#include <iostream>

PEGASUS_USING_PEGASUS;
PEGASUS_USING_STD;

struct Monkey
{
    Monkey()
    {
        // printf("Monkey()\n");
        numConstructions++;
        index = numConstructions;
    }

    ~Monkey()
    {
        // printf("~Monkey()\n");
        numDestructions++;
    }

    static Uint32 numConstructions;
    static Uint32 numDestructions;
    Uint32 index;
};

Uint32 Monkey::numConstructions = 0;
Uint32 Monkey::numDestructions = 0;

void testSharedPtr()
{
    Monkey::numConstructions = 0;
    Monkey::numDestructions = 0;

    {
        SharedPtr<Monkey> p1(new Monkey);
        SharedPtr<Monkey> p2(p1);
        SharedPtr<Monkey> p3(p2);
        SharedPtr<Monkey> p4;

        PEGASUS_TEST_ASSERT(p1);
        PEGASUS_TEST_ASSERT(p2);
        PEGASUS_TEST_ASSERT(p3);
        PEGASUS_TEST_ASSERT(!p4);

        PEGASUS_TEST_ASSERT(Monkey::numConstructions == 1);
        PEGASUS_TEST_ASSERT(Monkey::numDestructions == 0);
        PEGASUS_TEST_ASSERT(p1.count() == 3);
        PEGASUS_TEST_ASSERT(p1->index == p3->index);

        p4 = p1;

        PEGASUS_TEST_ASSERT(p4);

        PEGASUS_TEST_ASSERT(Monkey::numConstructions == 1);
        PEGASUS_TEST_ASSERT(Monkey::numDestructions == 0);
        PEGASUS_TEST_ASSERT(p2.count() == 4);
        PEGASUS_TEST_ASSERT(p3->index == p4->index);

        SharedPtr<Monkey> p5(new Monkey);
        p1 = p5;

        PEGASUS_TEST_ASSERT(Monkey::numConstructions == 2);
        PEGASUS_TEST_ASSERT(Monkey::numDestructions == 0);
        PEGASUS_TEST_ASSERT(p1.count() == 2);
        PEGASUS_TEST_ASSERT(p2.count() == 3);
        PEGASUS_TEST_ASSERT(p3.count() == 3);
        PEGASUS_TEST_ASSERT(p4.count() == 3);
        PEGASUS_TEST_ASSERT(p5.count() == 2);

        PEGASUS_TEST_ASSERT(p1->index == p5->index);
        PEGASUS_TEST_ASSERT(p1->index != p2->index);
        PEGASUS_TEST_ASSERT(p1.get()->index == p5.get()->index);
        PEGASUS_TEST_ASSERT(p1.get()->index != p2.get()->index);
        PEGASUS_TEST_ASSERT((*p1).index == (*p5).index);
        PEGASUS_TEST_ASSERT((*p1).index != (*p2).index);

        PEGASUS_TEST_ASSERT(p1 != 0);
        PEGASUS_TEST_ASSERT(p2 != 0);
        PEGASUS_TEST_ASSERT(p3 != 0);
        PEGASUS_TEST_ASSERT(p4 != 0);

        PEGASUS_TEST_ASSERT(p1);
        p1.reset();
        PEGASUS_TEST_ASSERT(!p1);
        PEGASUS_TEST_ASSERT(p5.count() == 1);
        PEGASUS_TEST_ASSERT(Monkey::numDestructions == 0);

        PEGASUS_TEST_ASSERT(p2);
        p2.reset();
        PEGASUS_TEST_ASSERT(!p2);
        PEGASUS_TEST_ASSERT(p3.count() == 2);
        PEGASUS_TEST_ASSERT(Monkey::numDestructions == 0);

        PEGASUS_TEST_ASSERT(p5);
        p5.reset();
        PEGASUS_TEST_ASSERT(!p5);
        PEGASUS_TEST_ASSERT(Monkey::numDestructions == 1);
    }

    PEGASUS_TEST_ASSERT(Monkey::numConstructions == 2);
    PEGASUS_TEST_ASSERT(Monkey::numDestructions == 2);
}

void testSharedArrayPtr()
{
    Monkey::numConstructions = 0;
    Monkey::numDestructions = 0;

    {
        SharedArrayPtr<Monkey> p1(new Monkey[3]);

        PEGASUS_TEST_ASSERT(Monkey::numConstructions == 3);
        PEGASUS_TEST_ASSERT(Monkey::numDestructions == 0);

        SharedArrayPtr<Monkey> p5(new Monkey[2]);
        p1 = p5;

        PEGASUS_TEST_ASSERT(Monkey::numConstructions == 5);
        PEGASUS_TEST_ASSERT(Monkey::numDestructions == 3);

        PEGASUS_TEST_ASSERT(p1[0].index == p5[0].index);
        PEGASUS_TEST_ASSERT(p1[1].index == p5[1].index);
        PEGASUS_TEST_ASSERT(p1[0].index != p1[1].index);
    }

    PEGASUS_TEST_ASSERT(Monkey::numConstructions == 5);
    PEGASUS_TEST_ASSERT(Monkey::numDestructions == 5);
}

void testCharPtr()
{
    // Test with FreeCharPtr<> function object.
    SharedPtr<char, FreeCharPtr> s((char*)malloc(1024));
}

int main()
{
    testSharedPtr();
    testSharedArrayPtr();
    testCharPtr();
    printf("+++++ passed all tests\n");

    return 0;
}
