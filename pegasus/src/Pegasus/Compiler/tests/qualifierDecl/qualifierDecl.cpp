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
    Test some basic characteristics of compiling qualifier declarations.
    Tests specifically for a number of issues found in bug 133 including:
       - Adding Class Qualifier when Schema qualifier found
       - Capitalizing names for Association and Indication Qualifiers
*/

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/PegasusAssert.h>
#include <Pegasus/Common/String.h>
#include <Pegasus/Repository/CIMRepository.h>

PEGASUS_USING_PEGASUS;
PEGASUS_USING_STD;


static Boolean verbose;
#define VCOUT if (verbose) cout

void TestQualifierDecl(CIMRepository& r)
{
    VCOUT << "TestAssociations Starting" << endl;

    String nameSpace = "root";

    // Test for existence of Schema qualifier (Should exist)
    CIMQualifierDecl q1 =  r.getQualifier(nameSpace, "Schema");

    CIMScope s1t;
    s1t.addScope(CIMScope::PROPERTY);
    s1t.addScope(CIMScope::METHOD);
    CIMScope s1 = q1.getScope();

    PEGASUS_TEST_ASSERT(s1.equal(s1t));

    // test for existence of Class qualifier.  Should not exist (Bug 133)
    try
    {
        CIMQualifierDecl q1 =  r.getQualifier(nameSpace, "Class");
        PEGASUS_TEST_ASSERT(false);
    }

    catch (CIMException& e)
    {
        if (e.getCode() != 6)
        {
            cerr << "CIMException " << e.getCode() << " "
                << e.getMessage() << endl;
            exit(1);
        }
        else
        {
            VCOUT << "Found Error for qualifier named Class" << endl;
        }
    }
    catch (Exception& e)
    {
        cerr << e.getMessage() << endl;
        exit(1);
    }

    // Test to confirm case of Association qualifier (see Bug 133)
    CIMQualifierDecl qAssoc =  r.getQualifier(nameSpace, "Association");

    VCOUT << "Association Qualifier Name " << qAssoc.getName().getString()
          << endl;

    PEGASUS_TEST_ASSERT(qAssoc.getName().getString() == "Association");

    CIMScope sAssocTest;
    sAssocTest.addScope(CIMScope::ASSOCIATION);
    CIMScope sAssoc = qAssoc.getScope();
    PEGASUS_TEST_ASSERT(sAssoc.equal(sAssocTest));

    // Test to confirm case of Indication qualifier (see Bug 133)
    CIMQualifierDecl qInd =  r.getQualifier(nameSpace, "Indication");

    VCOUT << "Indication Qualifier Name " << qInd.getName().getString()
          << endl;

    PEGASUS_TEST_ASSERT(qInd.getName().getString() == "Indication");
    CIMScope sIndTest;
    sIndTest.addScope(CIMScope::INDICATION);
    sIndTest.addScope(CIMScope::CLASS);
    CIMScope sInd = qInd.getScope();
    VCOUT << "Indication Qualifier Decl " << sInd.toString() << " tests "
        << sIndTest.toString() << endl;
    PEGASUS_TEST_ASSERT(sInd.equal(sIndTest));

    // Test qualifier with no Flavor defined

    CIMQualifierDecl noFlavor =  r.getQualifier(nameSpace, "noFlavor");
    CIMFlavor nf = noFlavor.getFlavor();

    // confirm result is DEFAULTS flavor
    PEGASUS_TEST_ASSERT(nf.equal(CIMFlavor::DEFAULTS));
}

int main(int, char** argv)
{
    String repositoryRoot;

    verbose = (getenv("PEGASUS_TEST_VERBOSE")) ? true : false;
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

    try
    {
        CIMRepository r (repositoryRoot);

        TestQualifierDecl(r);
    }
    catch (Exception& e)
    {
        cerr << e.getMessage() << endl;
        exit(1);
    }

    cout << argv[0] << " +++++ passed all tests" << endl;

    return 0;
}
