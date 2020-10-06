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
    This program tests the CIM QualifierList functions.
*/

#include <Pegasus/Common/PegasusAssert.h>
#include <Pegasus/Common/CIMQualifier.h>
#include <Pegasus/Common/CIMQualifierList.h>
#include <Pegasus/Common/CIMProperty.h>
#include <Pegasus/Common/DeclContext.h>

PEGASUS_USING_PEGASUS;
PEGASUS_USING_STD;

int main(int, char** argv)
{
    Boolean verbose = (getenv("PEGASUS_TEST_VERBOSE")) ? true : false;
    try
    {
    const String NAMESPACE = "/zzz";

    // Create and populate a declaration context:

    SimpleDeclContext context;

    context.addQualifierDecl(NAMESPACE, CIMQualifierDecl
            (CIMName ("abstract"),
        false, CIMScope::CLASS, CIMFlavor::OVERRIDABLE));

    context.addQualifierDecl(NAMESPACE, CIMQualifierDecl
            (CIMName ("description"),
        String(), CIMScope::CLASS, CIMFlavor::OVERRIDABLE));

    context.addQualifierDecl(NAMESPACE, CIMQualifierDecl(CIMName ("q1"),
        false, CIMScope::CLASS, CIMFlavor::OVERRIDABLE +
            CIMFlavor::TOSUBCLASS));

    context.addQualifierDecl(NAMESPACE, CIMQualifierDecl(CIMName ("q2"),
        false, CIMScope::CLASS, CIMFlavor::TOSUBCLASS));

    // ATTN: KS P1 29 Mar 2002 - Add Tests for Null Value

    // ATTN: KS P1 29 Mar 2002 - Add tests for array values in Qualifier.

    // Create qualifier list 1:

    CIMQualifierList qualifiers0;
    CIMQualifierList qualifiers1;

    qualifiers1
        .add(CIMQualifier(CIMName ("Abstract"), true))
        .add(CIMQualifier(CIMName ("Description"),
                String("CIMQualifier List 1")))
        .add(CIMQualifier(CIMName ("q1"), false))
        .add(CIMQualifier(CIMName ("q2"), false));

    // Run the find, get, etc tests.

    PEGASUS_TEST_ASSERT(qualifiers1.getCount() == 4);
    PEGASUS_TEST_ASSERT(qualifiers1.find(CIMName ("Abstract")) == 0);
    PEGASUS_TEST_ASSERT(qualifiers1.exists(CIMName ("Abstract")));
    PEGASUS_TEST_ASSERT(qualifiers1.isTrue(CIMName ("Abstract")));
    PEGASUS_TEST_ASSERT(qualifiers1.exists(CIMName ("q1")));
    PEGASUS_TEST_ASSERT(!qualifiers1.isTrue(CIMName ("q1")));
    PEGASUS_TEST_ASSERT(qualifiers1.exists(CIMName ("q2")));
    PEGASUS_TEST_ASSERT(!qualifiers1.isTrue(CIMName ("q2")));

    PEGASUS_TEST_ASSERT(qualifiers1.find(CIMName ("QualifierDoesNotExist")) ==
            PEG_NOT_FOUND);
    PEGASUS_TEST_ASSERT(!qualifiers1.exists(CIMName("QualifierDoesNotExist")));

    qualifiers1.resolve(
        &context, NAMESPACE, CIMScope::CLASS, false, qualifiers0, true);

    // Qualifiers after the resolve.  Should have resolved against the
    // declarations.
    PEGASUS_TEST_ASSERT(qualifiers1.getCount() == 4);
    PEGASUS_TEST_ASSERT(qualifiers1.find(CIMName ("Abstract")) == 0);
    PEGASUS_TEST_ASSERT(qualifiers1.exists(CIMName ("Abstract")));
    PEGASUS_TEST_ASSERT(qualifiers1.isTrue(CIMName ("Abstract")));
    PEGASUS_TEST_ASSERT(qualifiers1.exists(CIMName ("q1")));
    PEGASUS_TEST_ASSERT(!qualifiers1.isTrue(CIMName ("q1")));
    PEGASUS_TEST_ASSERT(qualifiers1.exists(CIMName ("q2")));
    PEGASUS_TEST_ASSERT(!qualifiers1.isTrue(CIMName ("q2")));

    PEGASUS_TEST_ASSERT(qualifiers1.find(CIMName ("QualifierDoesNotExist")) ==
            PEG_NOT_FOUND);
    PEGASUS_TEST_ASSERT(!qualifiers1.exists(CIMName("QualifierDoesNotExist")));
    if(verbose)
        qualifiers1.print();


    // Add test for double add of a name
    Boolean exceptionCaught = false;
    try
    {
        qualifiers1.add(CIMQualifier(CIMName ("Abstract"), true));
    }
    catch (Exception&)
    {
        exceptionCaught = true;
    }
    PEGASUS_TEST_ASSERT(exceptionCaught);

    // Test some of the basics again after the double insertion problem
    PEGASUS_TEST_ASSERT(qualifiers1.getCount() == 4);
    PEGASUS_TEST_ASSERT(qualifiers1.find(CIMName ("Abstract")) == 0);
    PEGASUS_TEST_ASSERT(qualifiers1.exists(CIMName ("Abstract")));
    PEGASUS_TEST_ASSERT(qualifiers1.isTrue(CIMName ("Abstract")));

    // Create qualifier list 2: Will be resolved against qualifiers1

    CIMQualifierList qualifiers2;

    qualifiers2
        .add(CIMQualifier(CIMName ("Description"),
                String("CIMQualifier List 1")))
        .add(CIMQualifier(CIMName ("q1"), Boolean(true),
                CIMFlavor::OVERRIDABLE));

    if(verbose)
        qualifiers2.print();

    PEGASUS_TEST_ASSERT(qualifiers2.getCount() == 2);
    PEGASUS_TEST_ASSERT(qualifiers2.exists(CIMName ("Description")));
    PEGASUS_TEST_ASSERT(qualifiers2.exists(CIMName ("q1")));
    PEGASUS_TEST_ASSERT(qualifiers2.isTrue(CIMName ("q1")));

    // Resolve the qualifiers against the previous list qualifiers1
    qualifiers2.resolve(
        &context, NAMESPACE, CIMScope::CLASS, false, qualifiers1, true);

    if(verbose)
        qualifiers2.print();

    // Post resolution
    PEGASUS_TEST_ASSERT(qualifiers2.getCount() == 4);
    PEGASUS_TEST_ASSERT(qualifiers2.exists(CIMName ("Description")));
    PEGASUS_TEST_ASSERT(qualifiers2.exists(CIMName ("abstract")));
    PEGASUS_TEST_ASSERT(qualifiers2.isTrue(CIMName ("abstract")));


    PEGASUS_TEST_ASSERT(qualifiers2.exists(CIMName ("q1")));
    PEGASUS_TEST_ASSERT(qualifiers2.isTrue(CIMName ("q1")));

    PEGASUS_TEST_ASSERT(qualifiers2.exists(CIMName ("q2")));
    // Should inherit the value from the superclass
    PEGASUS_TEST_ASSERT(!qualifiers2.isTrue(CIMName ("q2")));

    }
    catch (Exception& e)
    {
        cerr << "Exception: " << e.getMessage() << endl;
        exit(1);
    }

    cout << argv[0] << " +++++ passed all tests" << endl;

    return 0;
}
