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
#include <Pegasus/Common/PegasusAssert.h>
#include <Pegasus/Repository/InheritanceTree.h>
#include <Pegasus/Common/ArrayInternal.h>   // bubblesort

PEGASUS_USING_PEGASUS;
PEGASUS_USING_STD;

static Boolean verbose;

void TestGetSubClassNames(
    const InheritanceTree& it,
    const CIMName& className,
    Boolean deepInheritance,
    Array<CIMName>& expectedSubClassNames)
{
    Array<CIMName> subClassNames;
    it.getSubClassNames(className, deepInheritance, subClassNames);

    if(verbose)
    {
        for (Uint32 i = 0; i < subClassNames.size(); i++)
        {
            if (i != 0)
            {
                cout << ", ";
            }
            cout << subClassNames[i].getString();
        }
        cout << endl;
    }

    BubbleSort(expectedSubClassNames);
    BubbleSort(subClassNames);

    // If error display both arrays
    if (!(expectedSubClassNames == subClassNames))
    {
        cout << "ERROR: expected and result differ. Class " 
             << className.getString() << endl;
        cout << "Result =   ";
        for (Uint32 i = 0; i < subClassNames.size(); i++)
        {
            if (i != 0)
            {
                cout << ", ";
            }
            cout << subClassNames[i].getString();
        }
        cout << endl;
        cout << "Expected = ";
        for (Uint32 i = 0; i < expectedSubClassNames.size(); i++)
        {
            if (i != 0)
            {
                cout << ", ";
            }
            cout << expectedSubClassNames[i].getString();
        }
        cout << endl;                
    }
    PEGASUS_TEST_ASSERT(expectedSubClassNames == subClassNames);
}

int main(int, char** argv)
{
    verbose = getenv("PEGASUS_TEST_VERBOSE") ? true : false;

    try
    {
    InheritanceTree it;

    /*
    //----------------------------------------------------------------------
    //
    //        A
    //      /   \
    //     B     C
    //   /   \     \
    //  D     E     F
    //
    //----------------------------------------------------------------------
    */

    it.insert("D", "B");
    it.insert("E", "B");
    it.insert("B", "A");
    it.insert("C", "A");
    it.insert("F", "C");
    it.insert("A", "");
    it.check();

        if (verbose)
            it.print(cout);

    {
        Array<CIMName> expected;
        expected.append("B");
        expected.append("C");
        expected.append("D");
        expected.append("E");
        expected.append("F");
        TestGetSubClassNames(it, "A", true, expected);
    }
    {
        Array<CIMName> expected;
        expected.append("B");
        expected.append("C");
        TestGetSubClassNames(it, "A", false, expected);
    }
    {
        Array<CIMName> expected;
        expected.append("A");
        TestGetSubClassNames(it, CIMName(), false, expected);
    }
    {
        Array<CIMName> expected;
        expected.append("A");
        expected.append("B");
        expected.append("C");
        expected.append("D");
        expected.append("E");
        expected.append("F");
        TestGetSubClassNames(it, CIMName(), true, expected);
    }
    {
        Array<CIMName> expected;
        expected.append("F");
        TestGetSubClassNames(it, "C", true, expected);
    }
    {
        Array<CIMName> expected;
        expected.append("F");
        TestGetSubClassNames(it, "C", false, expected);
    }
    }
    catch (Exception& e)
    {
    cerr << e.getMessage() << endl;
    exit(1);
    }

    try
    {
        // build a valid inheritance tree
        InheritanceTree it;

        it.insert("H", "");
        it.insert("I", "H");
        it.insert("J", "H");
        it.insert("K", "I");
        it.insert("L", "I");
        it.insert("M", "J");
        it.insert("N", "J");
        it.insert("O", "J");
        it.insert("P", "J");

        it.check();
        if (verbose)
            it.print(cout);
    }
    catch (Exception& e)
    {
        cerr << e.getMessage() << endl;
        exit(1);
    }

    try
    {
        // build an invalid inheritance tree
        InheritanceTree it;
        it.insert("D", "B");
        it.insert("E", "B");
        it.insert("C", "A");
        it.insert("F", "C");
        it.insert("A", "");
        it.check();
        // check() should have thrown an InvalidInheritanceTree exception.
        PEGASUS_TEST_ASSERT(false);
    }
    catch (InvalidInheritanceTree& e)
    {
        if (verbose)
        cout << argv[0] << " " << e.getMessage() << endl;
    }
    catch (Exception& e)
    {
        cerr << e.getMessage() << endl;
        exit(1);
    }

    cout << argv[0] << " +++++ passed all tests" << endl;

    return 0;
}
