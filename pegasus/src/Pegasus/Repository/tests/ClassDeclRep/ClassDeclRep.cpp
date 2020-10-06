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

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/PegasusAssert.h>
#include <Pegasus/Common/CIMName.h>
#include <Pegasus/Repository/CIMRepository.h>
#include <Pegasus/Common/FileSystem.h>

PEGASUS_USING_PEGASUS;
PEGASUS_USING_STD;
static Boolean verbose;

void Test01(Uint32 mode)
{
    String repositoryRoot;
    const char* tmpDir = getenv ("PEGASUS_TMP");
    if (tmpDir == NULL)
    {
        repositoryRoot = ".";
    }
    else
    {
        repositoryRoot = tmpDir;
    }

    repositoryRoot.append("/repository");

    FileSystem::removeDirectoryHier(repositoryRoot);

    CIMRepository r (repositoryRoot, mode);

    // Create a namespace:

    const CIMNamespaceName NAMESPACE = CIMNamespaceName ("zzz");
    r.createNameSpace(NAMESPACE);

    // Create a qualifier (and read it back):

    CIMQualifierDecl q1(CIMName ("abstract"), false, CIMScope::CLASS);
    r.setQualifier(NAMESPACE, q1);

    CIMConstQualifierDecl q2 = r.getQualifier(NAMESPACE, CIMName ("abstract"));
    PEGASUS_TEST_ASSERT(q1.identical(q2));

    // Create two simple classes:

    CIMClass class1(CIMName ("Class1"));
    class1.addQualifier(
        CIMQualifier(CIMName ("abstract"), true, CIMFlavor::DEFAULTS));
    CIMClass class2(CIMName ("Class2"), CIMName ("Class1"));

    r.createClass(NAMESPACE, class1);
    r.createClass(NAMESPACE, class2);

    // Enumerate the class names:
    Array<CIMName> classNames =
        r.enumerateClassNames(NAMESPACE, CIMName(), true);

    BubbleSort(classNames);

    PEGASUS_TEST_ASSERT(classNames.size() == 2);
    PEGASUS_TEST_ASSERT(classNames[0] == "Class1");
    PEGASUS_TEST_ASSERT(classNames[1] == "Class2");

    // Get the classes and determine if they are identical with input

    CIMClass c1 = r.getClass(NAMESPACE, CIMName ("Class1"), true, true, false);
    CIMClass c2 = r.getClass(NAMESPACE, CIMName ("Class2"), true, true, false);

    PEGASUS_TEST_ASSERT(c1.identical(class1));
    PEGASUS_TEST_ASSERT(c1.identical(class1));

    Array<CIMClass> classes =
        r.enumerateClasses(NAMESPACE, CIMName (), true, true, true);

    // Attempt to delete Class1. It should fail since the class has
    // children.

    try
    {
        r.deleteClass(NAMESPACE, CIMName ("Class1"));
    }
    catch (CIMException& e)
    {
        PEGASUS_TEST_ASSERT(e.getCode() == CIM_ERR_CLASS_HAS_CHILDREN);
    }

    // Delete all classes created here:

    r.deleteClass(NAMESPACE, CIMName ("Class2"));
    r.deleteClass(NAMESPACE, CIMName ("Class1"));

    // Be sure the classes are really gone:

    try
    {
        CIMClass c1 = r.getClass(
            NAMESPACE, CIMName ("Class1"), true, true, true);
        PEGASUS_TEST_ASSERT(false);
    }
    catch (CIMException& e)
    {
        PEGASUS_TEST_ASSERT(e.getCode() == CIM_ERR_NOT_FOUND);
    }

    try
    {
        CIMClass c2 = r.getClass(
            NAMESPACE, CIMName ("Class2"), true, true, true);
        PEGASUS_TEST_ASSERT(false);
    }
    catch (CIMException& e)
    {
        PEGASUS_TEST_ASSERT(e.getCode() == CIM_ERR_NOT_FOUND);
    }

    FileSystem::removeDirectoryHier(repositoryRoot);
}

int main(int argc, char** argv)
{
    verbose = getenv("PEGASUS_TEST_VERBOSE") ? true : false;

    if (argc != 2)
    {
        cout << "Usage: " << argv[0] << " XML | BIN" << endl;
        return 1;
    }

    try
    {
      Uint32 mode;
      if (!strcmp(argv[1],"XML") )
    {
      mode = CIMRepository::MODE_XML;
      if (verbose) cout << argv[0]<< ": using XML mode repository" << endl;
    }
      else if (!strcmp(argv[1],"BIN") )
    {
      mode = CIMRepository::MODE_BIN;
      if (verbose) cout << argv[0]<< ": using BIN mode repository" << endl;
    }
      else
    {
      cout << argv[0] << ": invalid argument: " << argv[1] << endl;
      return 1;
    }

    Test01(mode);
    }
    catch (Exception& e)
    {
    cout << e.getMessage() << endl;
    exit(1);
    }

    cout << argv[0] << " " << argv[1] << " +++++ passed all tests" << endl;

    return 0;
}
