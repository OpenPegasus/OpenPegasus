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
//%////////////////////////////////////////////////////////////////////////////

/**
    Function: This CIM Client executable tests the Association provider
    defined in the providers/testproviders/AssociationTest directory

    This code tests associations as provided by the associationtest provider
    AND the associationtest.mof defined as part of the association test.
    It tests the following:
    referencenames, references, associatornames, associators  class request as
    defined in the MOF.
*/
//#define CDEBUG(X) cout << "AssociationTestPGM " << X << endl
#define CDEBUG(X)

// ==========================================================================
// Includes
// ==========================================================================

#include <Pegasus/Client/CIMClient.h>

// ==========================================================================
// Miscellaneous defines
// ==========================================================================

#define NAMESPACE "root/testassoc"
#define HOST      "localhost"
#define PORTNUMBER 5988
#define TIMEOUT   10000                    // timeout value in milliseconds

static char* verbose;

PEGASUS_USING_STD;
PEGASUS_USING_PEGASUS;

CIMClient c;

void errorExit(Exception& e)
{
    cout << "Error: Failed" << endl << e.getMessage() << endl;
    exit(1);
}

// Test a Class by enumerating the instances.
int testClass(const CIMName& className)
{
    Array<CIMObjectPath> refs;

    // =======================================================================
    // enumerateInstanceNames
    // =======================================================================

    cout << "+++++ enumerateInstanceNames(" << NAMESPACE << " " <<
        className << ") ";
    try
    {
        refs = c.enumerateInstanceNames(NAMESPACE,className);
    }
    catch (Exception& e)
    {
        cout << endl;
        errorExit(e);
    }

    cout << refs.size() << " instances" << endl;
    // if zero instances, not an error, but can't proceed
    if (refs.size() == 0)
    {
        cout << "+++++ test completed early" << endl;
        return 0;
    }
    return 0;
}

// Simply run through the classes and check number of instances
int testClassList(const Array<CIMName>& classList)
{
    int rc;

    for (Uint32 i = 0; i < classList.size() ; i++)
    {
        rc = testClass(classList[i]);
    }
    return 0;
}

int testAssocNames(
    const CIMObjectPath& objectName,
    const CIMName& assocClass,
    const CIMName& resultClass,
    const String& role,
    const String& resultRole,
    Uint32 expectedCount)
{
    //CIMObjectPath instanceName = CIMObjectPath ("Person.name=\"Mike\"");
    CDEBUG("testReferenceNames " << objectName.toString() <<
        " resultClass " << resultClass << " role " << role);

    Array<CIMObjectPath> result = c.associatorNames(
        NAMESPACE,
        objectName,
        assocClass,
        resultClass,
        role,
        resultRole);
    Array<CIMObject> resultObjects = c.associators(
        NAMESPACE,
        objectName,
        assocClass,
        resultClass,
        role,
        resultClass);

    if (result.size() != resultObjects.size())
    {
        cout << "ERROR, Associator and AssociatorName count returned difference"
             << endl;
    }

    for (Uint32 i = 0; i < result.size(); i++)
    {
        if (resultObjects[i].getPath().toString() != result[i].toString())
        {
            cout << "Name response Error" << endl;
        }
    }

    if (verbose)
    {
        cout << "REQUEST: Associators, Object: " << objectName
             << " assocClass " << assocClass
             << " resultClass " << resultClass
             << " role " << role
             << " resultClass " << resultClass
             << endl
             << "RESPONSE: ";
        for (Uint32 i = 0; i < result.size(); i++)
            cout << " " << result[i].toString() << " ";
        cout << endl;
    }

    if (result.size() != expectedCount)
    {
        cout << "AssociatorName Error Object " << objectName.toString() <<
            "Expected count = " << expectedCount <<
            " received " << result.size();
        return 1;
    }
    return 0;
}

/**
    testReferences -
*/
int testReferences(
    const CIMObjectPath& objectName,
    const CIMName& resultClass,
    const String& role,
    CIMPropertyList& propertyList,
    Uint32 expectedCount)
{
    CDEBUG("testRefrenceNames " << objectName.toString() <<
        " resultClass " << resultClass << " role " << role);
    Array<CIMObject> result = c.references(
        NAMESPACE,
        objectName,
        resultClass,
        role,
        false,
        false,
        propertyList);

    CDEBUG(" Rtned from c.references ");
    if (verbose)
    {
        for (Uint32 i = 0; i < result.size(); i++)
        {
            cout << "[" << result[i].getPath().toString() << "]" << endl;
        }
    }
    CDEBUG(" Now try the size comparison with input ");

    if (result.size() != expectedCount)
    {
        cout << "ReferenceName Error Object " << objectName.toString() <<
            "Expected count = " << expectedCount <<
            " received " << result.size();
    }

    return 0;
}

// Test references and reference names common elements.
/**
    testReferences executes the references and referencenames against the
    defined set of input parameters.  It compares the return from the two of
    them and reports differences between the information received.

    NOTE: One of the objectives is to compare the results between references
    and referencenames requests.  They should have the same number of items
    and the same objectpath.
    ATTN: Does the existence of the propertyList change that?  Therefore we
    do not include this on the operation.

    @param objectName - Class or instance name for the object being requested.
        Corresponds to the objectName parameter in the refrences request.
    @param resultClass - CIMName() i.e. NULL or class name to be supplied with
        the request
    @param role - empty string or valid role parameter
    @param expectedCount - Defines number of objects expected on the return.
*/
int testReferenceNames(
    const CIMObjectPath& objectName,
    const CIMName& resultClass,
    const String& role,
    Uint32 expectedCount)
{
    //CIMObjectPath instanceName = CIMObjectPath ("Person.name=\"Mike\"");
    CDEBUG ("testReferenceNames " << objectName.toString() << " resultClass "
        << resultClass << " role " << role);

    Array<CIMObjectPath> result = c.referenceNames(
        NAMESPACE,
        objectName,
        resultClass,
        role);
    CDEBUG("Return from Instance Names");
    Array<CIMObject> resultObjects = c.references(
        NAMESPACE,
        objectName,
        resultClass,
        role);

    CDEBUG("Return from calls. compare sizes");
    if (result.size() != resultObjects.size())
    {
        cout << "ERROR, Reference and reference Name size difference" << endl;
    }
    CDEBUG("Size tests finished, now compare the returns");
    for (Uint32 i = 0; i < result.size(); i++)
    {
        if (resultObjects[i].getPath().toString() != result[i].toString())
        {
            cout << "Name response Error: "
                 << resultObjects[i].getPath().toString()
                 << " != "
                 << result[i].toString()
                 << endl;
        }
    }
    CDEBUG("Returns Compared. Now to the display");
    if (verbose)
    {
        cout << "REQUEST: References, Object: " << objectName << " Filter: "
                << resultClass << " role: " << role << endl
                << "RESPONSE:";
        for (Uint32 i = 0; i < result.size(); i++)
            cout << " " << result[i].getClassName().getString() << "";
        cout << endl;
    }

    if (result.size() != expectedCount)
    {
        cout << "ReferenceName Error Object " << objectName.toString()
            << "Expected count: " << expectedCount
            << " Received: " << result.size()
            << endl;

        return 0;
    }
    return 0;
}

#define ASRT(X) X
//#define ASRT(X) PEGASUS_TEST_ASSERT(X)

// ===========================================================================
// Main
// Options:
//  Test or show - TBD
// ===========================================================================
int main()
{
    //verbose = getenv("PEGASUS_TEST_VERBOSE");
    verbose = "test";
    cout << "+++++ Testing AssociationTest Provider" << endl;

    // Build array of classes
    Array<CIMName> Classes;
    Classes.append("TST_ClassA");
    Classes.append("TST_ClassB");
    Classes.append("TST_ClassC");
    Classes.append("TST_ClassD");
    Classes.append("TSTDY_ClassA");
    Classes.append("TSTDY_ClassB");
    Classes.append("TSTDY_ClassC");
    Classes.append("TSTDY_ClassD");

    Array<CIMName> AssocClasses;
    AssocClasses.append("TST_AssocI1");
    AssocClasses.append("TST_AssocI2");
    AssocClasses.append("TST_AssocI3");
    AssocClasses.append("TST_AssocI4");
    AssocClasses.append("TST_AssocI5");
    AssocClasses.append("TST_AssocI6");

    AssocClasses.append("TST_AssocNI1");
    AssocClasses.append("TST_AssocNI2");
    AssocClasses.append("TST_AssocNI3");
    AssocClasses.append("TST_AssocNI4");
    AssocClasses.append("TST_AssocNI5");
    AssocClasses.append("TST_AssocNI6");

    // Connect
    try
    {
        c.connect (HOST, PORTNUMBER, String::EMPTY, String::EMPTY);
    }
    catch (Exception& e)
    {
        errorExit(e);
    }

    int rc;
    // Start by confirming the existence of the classes, etc.
    if ((rc = testClassList(Classes)) != 0) return rc;
    if ((rc = testClassList(AssocClasses)) != 0) return rc;
    // Now start the association tests.
    // Reference Names Test

    // Class A Refrence Names Test

    ASRT(testReferenceNames(CIMObjectPath("TST_ClassA"), CIMName(), "", 2));
    ASRT(testReferenceNames(CIMObjectPath("TST_ClassA"), CIMName(), "to", 2));
    ASRT(testReferenceNames(CIMObjectPath("TST_ClassA"), CIMName(), "from", 2));

    ASRT(testReferenceNames(
        CIMObjectPath("TST_ClassA"), CIMName("TST_AssocI1"), "", 1));
    ASRT(testReferenceNames(
        CIMObjectPath("TST_ClassA"), CIMName("TST_AssocI1"), "to", 1));
    ASRT(testReferenceNames(
        CIMObjectPath("TST_ClassA"), CIMName("TST_AssocI1"), "from", 1));

    ASRT(testReferenceNames(
        CIMObjectPath("TST_ClassA"), CIMName("TST_AssocI3"), "", 1));
    ASRT(testReferenceNames(
        CIMObjectPath("TST_ClassA"), CIMName("TST_AssocI3"), "to", 0));
    ASRT(testReferenceNames(
        CIMObjectPath("TST_ClassA"), CIMName("TST_AssocI3"), "from", 1));

    ASRT(testReferenceNames(
        CIMObjectPath("TST_ClassA"), CIMName("TST_AssocI5"), "", 0));
    ASRT(testReferenceNames(
        CIMObjectPath("TST_ClassA"), CIMName("TST_AssocI5"), "to", 0));
    ASRT(testReferenceNames(
        CIMObjectPath("TST_ClassA"), CIMName("TST_AssocI5"), "from", 0));

    // Class B Refrence Names Test
    ASRT(testReferenceNames(CIMObjectPath("TST_ClassB"), CIMName(), "", 4));
    ASRT(testReferenceNames(CIMObjectPath("TST_ClassB"), CIMName(), "to", 2));
    ASRT(testReferenceNames(CIMObjectPath("TST_ClassB"), CIMName(), "from", 4));

    ASRT(testReferenceNames(
        CIMObjectPath("TST_ClassB"), CIMName("TST_AssocI2"), "", 1));
    ASRT(testReferenceNames(
        CIMObjectPath("TST_ClassB"), CIMName("TST_AssocI2"), "to", 1));
    ASRT(testReferenceNames(
        CIMObjectPath("TST_ClassB"), CIMName("TST_AssocI2"), "from", 1));

    ASRT(testReferenceNames(
        CIMObjectPath("TST_ClassB"), CIMName("TST_AssocI2"), "", 1));
    ASRT(testReferenceNames(
        CIMObjectPath("TST_ClassB"), CIMName("TST_AssocI2"), "to", 0));
    ASRT(testReferenceNames(
        CIMObjectPath("TST_ClassB"), CIMName("TST_AssocI2"), "from", 1));

    ASRT(testReferenceNames(
        CIMObjectPath("TST_ClassB"), CIMName("TST_AssocI5"), "", 1));
    ASRT(testReferenceNames(
        CIMObjectPath("TST_ClassB"), CIMName("TST_AssocI5"), "to", 1));
    ASRT(testReferenceNames(
        CIMObjectPath("TST_ClassB"), CIMName("TST_AssocI5"), "from", 1));

    // TODO TestReferences for class c and class d

    // testRefernceName Instances from static store
    ASRT(testReferenceNames(
        CIMObjectPath("TST_ClassA.name=\"InstanceA1\""), CIMName(), "", 2));
    ASRT(testReferenceNames(
        CIMObjectPath("TST_ClassA.name=\"InstanceA1\""), CIMName(), "to", 2));
    ASRT(testReferenceNames(
        CIMObjectPath("TST_ClassA.name=\"InstanceA1\""), CIMName(), "from", 2));

    ASRT(testReferenceNames(
        CIMObjectPath("TST_ClassA.name=\"InstanceA1\""),
        CIMName("TST_AssocI1"), "", 1));
    ASRT(testReferenceNames(
        CIMObjectPath("TST_ClassA.name=\"InstanceA1\""),
        CIMName("TST_AssocI1"), "to", 1));
    ASRT(testReferenceNames(
        CIMObjectPath("TST_ClassA.name=\"InstanceA1\""),
        CIMName("TST_AssocI1"), "from", 1));

    ASRT(testReferenceNames(
        CIMObjectPath("TST_ClassA.name=\"InstanceA1\""),
        CIMName("TST_AssocI3"), "", 1));
    ASRT(testReferenceNames(
        CIMObjectPath("TST_ClassA.name=\"InstanceA1\""),
        CIMName("TST_AssocI3"), "to", 0));
    ASRT(testReferenceNames(
        CIMObjectPath("TST_ClassA.name=\"InstanceA1\""),
        CIMName("TST_AssocI3"), "from", 1));

    ASRT(testReferenceNames(
        CIMObjectPath("TST_ClassA.name=\"InstanceA1\""),
        CIMName("TST_AssocI5"), "", 0));
    ASRT(testReferenceNames(
        CIMObjectPath("TST_ClassA.name=\"InstanceA1\""),
        CIMName("TST_AssocI5"), "to", 0));
    ASRT(testReferenceNames(
        CIMObjectPath("TST_ClassA.name=\"InstanceA1\""),
        CIMName("TST_AssocI5"), "from", 0));

    // Lets make the previous a common test between ref and ref names.

    // References
    // Limited test since we learned most in the previous test of reference
    // names
    CIMPropertyList NullPL;

    CIMPropertyList emptyPL;

    CIMPropertyList fullPL;
    Array<CIMName> fullPLinput;
    fullPLinput.append("name");
    fullPL.set(fullPLinput);


    CIMPropertyList errorPL;
    Array<CIMName> errorPLinput;
    errorPLinput.append("junk");
    errorPL.set(errorPLinput);

    ASRT(testReferences(CIMObjectPath("TST_ClassA"), CIMName(), "", emptyPL,2));

    ASRT(testReferences(
        CIMObjectPath("TST_ClassA.name=\"InstanceA1\""),
        CIMName(), "", fullPL,2));

    // Testing associators and and associator names functions.

    ASRT(testAssocNames(
        CIMObjectPath("TST_ClassA"), CIMName(), CIMName(), "", "", 3));

    cout << "+++++ passed all tests" << endl;
    return 0;
}
