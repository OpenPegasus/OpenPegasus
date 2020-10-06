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
/*
    This test exercises the NameSpaceManager class.
    It creates namespaces, adds and deletes classes, and deletes namespaces.
    The method it tests includes:
    constructor
    createNamespace
    deleteNameSpace
    getNameSpaceNames
    nameSpaceExists
    createClass
    deleteClass
    getSuperClassName

    The functions it DOES NOT test today includes:
    isRemoteNameSpace
    checkModify
    modifyNameSpace
    getSubClassNames
    GetSuperClassNames
    classHasInstances
    classExists
    Note: Many of these untested functions are adequately tested through their
    use by higher level functions in CIMRepository.
*/

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/PegasusAssert.h>
#include <Pegasus/Common/AutoPtr.h>
#include <Pegasus/Repository/NameSpaceManager.h>

PEGASUS_USING_STD;
PEGASUS_USING_PEGASUS;

static Boolean verbose;

Array<CIMNamespaceName> _nameSpaceNames;

#define NUM_NAMESPACE_NAMES _nameSpaceNames.size()

void test01()
{
    NameSpaceManager nsm;

    _nameSpaceNames.append(CIMNamespaceName("aa"));
    _nameSpaceNames.append(CIMNamespaceName("aa/bb"));
    _nameSpaceNames.append(CIMNamespaceName("aa/bb/cc"));
    _nameSpaceNames.append(CIMNamespaceName("/lmnop/qrstuv"));
    _nameSpaceNames.append(CIMNamespaceName("root"));
    _nameSpaceNames.append(CIMNamespaceName("xx"));
    _nameSpaceNames.append(CIMNamespaceName("xx/yy"));

    for (Uint32 j = 0; j < _nameSpaceNames.size(); j++)
    {
        // NOTE: The "root" namespace is created by CIMRepository, which is not
        // used by this test program.  So the "root" namespace is not expected
        // to be created automatically in this case.
        //if (!_nameSpaceNames[j].equal(CIMNamespaceName("root")))
        {
            // Create a namespace
            nsm.createNameSpace(_nameSpaceNames[j], true, true, String::EMPTY);
        }
    }

    Array<CIMNamespaceName> nameSpaceNames;
    nsm.getNameSpaceNames(nameSpaceNames);
    if (verbose)
        nsm.print(cout);
    PEGASUS_TEST_ASSERT(nameSpaceNames.size() == NUM_NAMESPACE_NAMES);
    BubbleSort(nameSpaceNames);

    for (Uint32 i = 0; i < NUM_NAMESPACE_NAMES; i++)
    {
        PEGASUS_TEST_ASSERT(_nameSpaceNames[i] == nameSpaceNames[i]);
        PEGASUS_TEST_ASSERT(nsm.nameSpaceExists(nameSpaceNames[i]));
    }

    nsm.deleteNameSpace(CIMNamespaceName("lmnop/qrstuv"));
    nsm.getNameSpaceNames(nameSpaceNames);
    PEGASUS_TEST_ASSERT(nameSpaceNames.size() == NUM_NAMESPACE_NAMES - 1);

    // Create and delete a class to test these functions
    nsm.createClass(CIMNamespaceName("aa/bb"), "MySuperClass", CIMName());
    nsm.createClass(CIMNamespaceName("aa/bb"), "MyClass", "MySuperClass");
    PEGASUS_TEST_ASSERT(
        nsm.getSuperClassName(CIMNamespaceName("aa/bb"), "MySuperClass") ==
        CIMName());
    PEGASUS_TEST_ASSERT(
        nsm.getSuperClassName(CIMNamespaceName("aa/bb"), "MyClass") ==
        "MySuperClass");
    nsm.deleteClass(CIMNamespaceName("aa/bb"), "MyClass");
    nsm.deleteClass(CIMNamespaceName("aa/bb"), "MySuperClass");

    for (Uint32 j = 0; j < _nameSpaceNames.size(); j++)
    {
        if (!_nameSpaceNames[j].equal(CIMNamespaceName("root")) &&
            !_nameSpaceNames[j].equal(CIMNamespaceName("lmnop/qrstuv")))
        {
            nsm.deleteNameSpace(CIMNamespaceName(_nameSpaceNames[j]));
        }
    }
    nsm.getNameSpaceNames(nameSpaceNames);
    // Only the root namespace should be left.
    PEGASUS_TEST_ASSERT(nameSpaceNames.size() == 1);
    PEGASUS_TEST_ASSERT(nameSpaceNames[0].equal(CIMNamespaceName("root")));
    // confirm we can delete root
    nsm.deleteNameSpace(nameSpaceNames[0]);
    nsm.getNameSpaceNames(nameSpaceNames);
    PEGASUS_TEST_ASSERT(nameSpaceNames.size() == 0);
}

int main(int, char** argv)
{

    verbose = getenv ("PEGASUS_TEST_VERBOSE") ? true : false;
    if (verbose) cout << argv[0] << ": started" << endl;

    try
    {
        test01();
    }
    catch (Exception& e)
    {
        cout << argv[0] << ": " << e.getMessage() << endl;
        exit (1);
    }

    cout << argv[0] << " +++++ passed all tests" << endl;
    return 0;
}
