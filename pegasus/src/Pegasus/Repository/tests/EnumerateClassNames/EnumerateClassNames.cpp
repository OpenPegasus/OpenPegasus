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

////////////////////////////////////////////////////////////////////////////////
//
// The inheritance structure for this test is:
//
//  Class1
//  Class2
//  A
//    X
//      M
//        Q
//        R
//      N
//        S
//        T
//    Y
//    Z
//
////////////////////////////////////////////////////////////////////////////////

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/PegasusAssert.h>
#include <Pegasus/Common/FileSystem.h>
#include <Pegasus/Repository/CIMRepository.h>

PEGASUS_USING_PEGASUS;
PEGASUS_USING_STD;

static Boolean verbose;

const CIMNamespaceName NAMESPACE = CIMNamespaceName ("test/zzz");
String repositoryRoot;

void print(const Array<CIMName>& array)
{
    if (verbose)
    {
        for (Uint32 i = 0, n = array.size(); i < n; i++)
        cout << "array[" << i << "]: " << array[i].getString() << endl;
    }
}

void TestCase1()
{
    if (verbose)
    {
        cout << "TestCase1" << endl;
    }
    CIMRepository r (repositoryRoot);

    // Enumerate the class names:

    Array<CIMName> classNames = r.enumerateClassNames(
            NAMESPACE, CIMName(), false);

    print(classNames);

    Array<CIMName> tmp;
    tmp.append(CIMName ("TST_A"));
    tmp.append(CIMName ("TST_Class1"));
    tmp.append(CIMName ("TST_Class2"));

    BubbleSort(tmp);
    BubbleSort(classNames);
    PEGASUS_TEST_ASSERT(tmp.size() == 3);
    PEGASUS_TEST_ASSERT(tmp.size() == classNames.size());

    PEGASUS_TEST_ASSERT(tmp == classNames);
}

void TestCase2()
{
    if (verbose)
    {
        cout << "TestCase2" << endl;
    }
    CIMRepository r (repositoryRoot);

    // Enumerate the class names:

    Array<CIMName> classNames = r.enumerateClassNames(
    NAMESPACE, CIMName(), true);

    print(classNames);

    Array<CIMName> tmp;
    tmp.append(CIMName ("TST_A"));
    tmp.append(CIMName ("TST_Class1"));
    tmp.append(CIMName ("TST_Class2"));
    tmp.append(CIMName ("TST_X"));
    tmp.append(CIMName ("TST_Y"));
    tmp.append(CIMName ("TST_Z"));
    tmp.append(CIMName ("TST_M"));
    tmp.append(CIMName ("TST_N"));
    tmp.append(CIMName ("TST_Q"));
    tmp.append(CIMName ("TST_R"));
    tmp.append(CIMName ("TST_S"));
    tmp.append(CIMName ("TST_T"));

    BubbleSort(tmp);
    BubbleSort(classNames);
    PEGASUS_TEST_ASSERT(tmp == classNames);
}

void TestCase3()
{
    if (verbose)
    {
        cout << "TestCase3" << endl;
    }
    CIMRepository r (repositoryRoot);

    // Enumerate the class names:

    Array<CIMName> classNames = r.enumerateClassNames(
    NAMESPACE, CIMName ("TST_X"), false);

    print(classNames);

    Array<CIMName> tmp;
    tmp.append(CIMName ("TST_M"));
    tmp.append(CIMName ("TST_N"));

    BubbleSort(tmp);
    BubbleSort(classNames);
    PEGASUS_TEST_ASSERT(tmp == classNames);
}

void TestCase4()
{
    if (verbose)
    {
        cout << "TestCase4" << endl;
    }
    CIMRepository r (repositoryRoot);

    // Enumerate the class names:

    Array<CIMName> classNames = r.enumerateClassNames(
    NAMESPACE, CIMName ("TST_X"), true);

    print(classNames);

    Array<CIMName> tmp;
    tmp.append(CIMName ("TST_M"));
    tmp.append(CIMName ("TST_N"));
    tmp.append(CIMName ("TST_Q"));
    tmp.append(CIMName ("TST_R"));
    tmp.append(CIMName ("TST_S"));
    tmp.append(CIMName ("TST_T"));

    BubbleSort(tmp);
    BubbleSort(classNames);
    PEGASUS_TEST_ASSERT(tmp == classNames);
}

static void CreateClass(
    CIMRepository& r,
    const CIMName& className,
    const CIMName superClassName = CIMName())
{
    CIMClass c(className, superClassName);
    r.createClass(NAMESPACE, c);
}

int main(int argc, char** argv)
{
    verbose = getenv("PEGASUS_TEST_VERBOSE") ? true : false;

    const char* tmpDir;
    tmpDir = getenv ("PEGASUS_TMP");
    if (tmpDir == NULL)
    {
        repositoryRoot = ".";
    }
    else
    {
        repositoryRoot = tmpDir;
    }
    repositoryRoot.append("/repository");

    Uint32 mode;
    if(argc==1)
    {
      cout << argv[0] << ": No argument provided: " << endl;
          cout <<"Usage: EnumerateClassNames [XML | BIN]" << endl;
      return 1;
    }

    if (!strcmp(argv[1], "XML"))
    {
        mode = CIMRepository::MODE_XML;
        if (verbose) cout << argv[0]<< ": using XML mode repository" << endl;
    }
    else if (!strcmp(argv[1], "BIN"))
    {
        mode = CIMRepository::MODE_BIN;
        if (verbose) cout << argv[0]<< ": using BIN mode repository" << endl;
    }
    else
    {
        cout << argv[0] << ": invalid argument: " << argv[1] << endl;
        cout <<"Usage: EnumerateClassNames [XML | BIN]" << endl;
        return 1;
    }

    FileSystem::removeDirectoryHier(repositoryRoot);

    CIMRepository r (repositoryRoot, mode);

    try
    {
        r.createNameSpace(NAMESPACE);
        CreateClass(r, CIMName ("TST_Class1"));
        CreateClass(r, CIMName ("TST_Class2"));
        CreateClass(r, CIMName ("TST_A"));
        CreateClass(r, CIMName ("TST_X"), CIMName ("TST_A"));
        CreateClass(r, CIMName ("TST_Y"), CIMName ("TST_A"));
        CreateClass(r, CIMName ("TST_Z"), CIMName ("TST_A"));
        CreateClass(r, CIMName ("TST_M"), CIMName ("TST_X"));
        CreateClass(r, CIMName ("TST_N"), CIMName ("TST_X"));
        CreateClass(r, CIMName ("TST_Q"), CIMName ("TST_M"));
        CreateClass(r, CIMName ("TST_R"), CIMName ("TST_M"));
        CreateClass(r, CIMName ("TST_S"), CIMName ("TST_N"));
        CreateClass(r, CIMName ("TST_T"), CIMName ("TST_N"));
        TestCase1();
        TestCase2();
        TestCase3();
        TestCase4();
    }
    catch (Exception& e)
    {
        cout << "Exception " << e.getMessage() << endl;
        PEGASUS_TEST_ASSERT(false);
    }

    FileSystem::removeDirectoryHier(repositoryRoot);

    cout << argv[0] << " " << argv[1] << " +++++ passed all tests" << endl;

    return 0;
}
