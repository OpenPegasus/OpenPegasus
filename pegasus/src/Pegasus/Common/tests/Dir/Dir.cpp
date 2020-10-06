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
#include <Pegasus/Common/Dir.h>
#include <Pegasus/Common/ArrayInternal.h>

PEGASUS_USING_PEGASUS;
PEGASUS_USING_STD;

void test01()
{
    Array<String> names;

    for (Dir dir("testdir"); dir.more(); dir.next())
    {
    String name = dir.getName();

    if (String::equal(name, ".") || String::equal(name, "..")
        || String::equal(name, "CVS") || String::equal(name, ".svn"))
    {
        continue;
    }

    names.append(name);
    }

    BubbleSort(names);
    PEGASUS_TEST_ASSERT(names.size() == 3);
    PEGASUS_TEST_ASSERT(String::equal(names[0], "a"));
    PEGASUS_TEST_ASSERT(String::equal(names[1], "b"));
    PEGASUS_TEST_ASSERT(String::equal(names[2], "c"));
}

int main(int, char** argv)
{
    try
    {
        test01();
    }
    catch (Exception& e)
    {
       cout << e.getMessage() << endl;
       exit(1);
    }

    try
    {
        Dir dir("noSuchDirectory");
    }
    catch (CannotOpenDirectory &e )
    {
        cout << argv[0] << " Expected exceptions: " << e.getMessage() << endl;
        cout << argv[0] << " +++++ passed all tests" << endl;
        exit(0);
    }

    PEGASUS_TEST_ASSERT(0);
    exit(1);

    return 0;
}
