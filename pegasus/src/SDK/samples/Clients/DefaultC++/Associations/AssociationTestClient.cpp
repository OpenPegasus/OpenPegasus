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

//
// This CIM client program is used to test the SampleAssociationProvider.
//
// This program makes CIMClient association method calls (associators,
// associatorNames, references, referenceNames) to get information about the
// relationships between the Sample_Teacher and Sample_Student classes.
//
// The SampleAssociationProvider is the instance and association provider
// for the following classes:
//
//     Sample_Teacher
//     Sample_Student
//     Sample_TeacherStudent  (association class)
//     Sample_AdvisorStudent  (association class)
//
// The executable for this CIM client application is:  AssociationTestClient.
// To display the test results (returned instances or classes), use the -v
// (verbose) option in the command line:  'AssociationTestClient -v'.
//
// Test results are verified by comparing the number of returned objects
// with the expected number of returned objects.  Any unexpected result
// is considered a failure.  If there is a failure, an error message is
// displayed and the program terminates.
//

#include <Pegasus/Client/CIMClient.h>
#include <string.h>

PEGASUS_USING_STD;
PEGASUS_USING_PEGASUS;

const CIMNamespaceName NAMESPACE = CIMNamespaceName("SDKExamples/DefaultCXX");
const CIMName SAMPLE_TEACHER = CIMName("Sample_Teacher");
const CIMName SAMPLE_STUDENT = CIMName("Sample_Student");
const CIMName SAMPLE_TEACHERSTUDENT = CIMName("Sample_TeacherStudent");
const CIMName SAMPLE_ADVISORSTUDENT = CIMName("Sample_AdvisorStudent");

static Boolean verbose = false;
// exepected results - number of returned objects expected for each test
//
static const Uint32 resultArray_asso_T1[] = { 3, 2, 2, 1 };
static const Uint32 resultArray_asso_T2[] = { 2, 1, 0, 0 };
static const Uint32 resultArray_asso_S1[] = { 3, 3, 2 };
static const Uint32 resultArray_asso_S2[] = { 1, 1, 1 };
static const Uint32 resultArray_ref_T[] = { 5, 3, 2, 1 };
static const Uint32 resultArray_ref_S[] = { 4, 4, 3 };

////////////////////////////////////////////////////////////////////////////
//  _errorExit
////////////////////////////////////////////////////////////////////////////

void _errorExit(const String& message)
{
    cerr << "AssociationTestClient error: " << message << endl;
    exit(1);
}

////////////////////////////////////////////////////////////////////////////
//  _verifyResult
////////////////////////////////////////////////////////////////////////////

//
//  Verify that the number of objects returned is as expected.  If the
//  number of objects returned is not expected, display an error and
//  terminate the program.
//
void _verifyResult(
    const Uint32 numObjects,
    const Uint32 numExpectedObjects)
{
    if (numObjects != numExpectedObjects)
    {
        cerr << "Error: Unexpected number of objects returned.  ";
        cerr << "Expected " << numExpectedObjects << " object(s), but ";
        cerr << numObjects << " object(s) were returned." << endl;
        exit(1);
    }
}

////////////////////////////////////////////////////////////////////////////
//  _displayResult
////////////////////////////////////////////////////////////////////////////

//
//  Display the result objects if verbose option is specified.
//
void _displayResult(const Array<CIMObject>& objects)
{
    if (verbose)
    {
        Uint32 numObjects = objects.size();
        for (Uint32 i = 0; i < numObjects; i++)
        {
            cout << objects[i].getPath().toString() << endl;
        }
    }
}

////////////////////////////////////////////////////////////////////////////
//  _displayResult
////////////////////////////////////////////////////////////////////////////

//
//  Display the result object names if verbose option is specified.
//
void _displayResult(const Array<CIMObjectPath>& objectPaths)
{
    if (verbose)
    {
        Uint32 numObjectPaths = objectPaths.size();
        for (Uint32 i = 0; i < numObjectPaths; i++)
        {
            cout << objectPaths[i].toString() << endl;
        }
    }
}

////////////////////////////////////////////////////////////////////////////
//  _testAssociators
////////////////////////////////////////////////////////////////////////////

void _testAssociators(
    CIMClient& client,
    const CIMName& assocClass,
    const CIMObjectPath& instancePath,
    Uint32 numExpectedObjects)
{
    if (verbose)
    {
        cout << "\nAssociation Class: " << assocClass.getString() << endl;
        cout << "\nObject Name: " << instancePath.toString() << endl;
    }

    try
    {
        CIMName resultClass;
        String role;
        String resultRole;

        // Get the CIM instances that are associated with the specified source
        // instance via an instance of the AssocClass
        //
        Array<CIMObject> resultObjects = client.associators(
            NAMESPACE,
            instancePath,
            assocClass,
            resultClass,
            role,
            resultRole);

        // verify result
        _verifyResult(resultObjects.size(), numExpectedObjects);

        // display result
        _displayResult(resultObjects);
    }
    catch (Exception& e)
    {
        _errorExit(e.getMessage());
    }
}

////////////////////////////////////////////////////////////////////////////
//  _testAssociatorNames
////////////////////////////////////////////////////////////////////////////

void _testAssociatorNames(
    CIMClient& client,
    const CIMName& assocClass,
    const CIMObjectPath& instancePath,
    Uint32 numExpectedObjects)
{
    if (verbose)
    {
        cout << "\nAssociation Class: " << assocClass.getString() << endl;
        cout << "\nObject Name: " << instancePath.toString() << endl;
    }

    try
    {
        // Get the names of the CIM instances that are associated to the
        // specified source instance via an instance of the AssocClass.
        //
        CIMName resultClass;
        String role;
        String resultRole;

        Array<CIMObjectPath> resultObjectPaths = client.associatorNames(
            NAMESPACE,
            instancePath,
            assocClass,
            resultClass,
            role,
            resultRole);

        // verify result
        _verifyResult(resultObjectPaths.size(), numExpectedObjects);

        // display result
        _displayResult(resultObjectPaths);
    }
    catch (Exception& e)
    {
        _errorExit(e.getMessage());
    }
}

////////////////////////////////////////////////////////////////////////////
//  _testReferences
////////////////////////////////////////////////////////////////////////////

void _testReferences(
    CIMClient& client,
    const CIMObjectPath& instancePath,
    Uint32 numExpectedObjects)
{
    if (verbose)
    {
        cout << "\nObject Name: " << instancePath.toString() << endl;
    }

    try
    {
        // get the association reference instances
        //
        Array<CIMObject> resultObjects;
        CIMName resultClass;
        String role;

        resultObjects =
            client.references(NAMESPACE, instancePath, resultClass, role);

        // verify result
        _verifyResult(resultObjects.size(), numExpectedObjects);

        // display result
        _displayResult(resultObjects);
    }
    catch (Exception& e)
    {
        _errorExit(e.getMessage());
    }
}

////////////////////////////////////////////////////////////////////////////
//  _testReferenceNames
////////////////////////////////////////////////////////////////////////////

void _testReferenceNames(
    CIMClient& client,
    const CIMObjectPath& instancePath,
    Uint32 numExpectedObjects)
{
    if (verbose)
    {
        cout << "\nObject Name: " << instancePath.toString() << endl;
    }

    try
    {
        // get the reference instance names
        //
        Array<CIMObjectPath> resultObjectPaths;
        CIMName resultClass;
        String role;

        resultObjectPaths =
            client.referenceNames(NAMESPACE, instancePath, resultClass, role);

        // verify result
        _verifyResult(resultObjectPaths.size(), numExpectedObjects);

        // display result
        _displayResult(resultObjectPaths);
    }
    catch (Exception& e)
    {
        _errorExit(e.getMessage());
    }
}

////////////////////////////////////////////////////////////////////////////
//  _testAssociatorFilters
//
//  Call the associators() and associatorNames() methods with different filters
//  specified.  Both valid and invalid filters are specified.  The result is
//  verified by comparing the number of objects returned with the expected
//  number of objects returned.
//
////////////////////////////////////////////////////////////////////////////

void _testAssociatorFilters(
    CIMClient& client,
    const String& instancePath,
    const String& role,
    const String& resultRole,
    const CIMName& resultClass,
    const CIMName& assocClass,
    Uint32 numExpectedObjects,
    const String& testMsg)
{
    Array<CIMObjectPath> resultObjectPaths;
    Array<CIMObject> resultObjects;
    CIMObjectPath op(instancePath);

    // =======================================================================
    // associators
    // =======================================================================

    if (verbose)
    {
        cout << "\n+++++ Test associators with " << testMsg;
        cout << " (" << instancePath << ")" << endl;
    }

    try
    {
        // get the association instances
        resultObjects = client.associators(NAMESPACE, op, assocClass,
            resultClass, role, resultRole);

        // verify result
        _verifyResult(resultObjects.size(), numExpectedObjects);

        // display result
        _displayResult(resultObjects);
    }
    catch (Exception& e)
    {
        _errorExit(e.getMessage());
    }

    // =======================================================================
    // associatorNames
    // =======================================================================

    if (verbose)
    {
        cout << "\n+++++ Test associatorNames with " << testMsg;
        cout << " (" << instancePath << ")" << endl;
    }

    try
    {
        // get the associator instance names
        resultObjectPaths = client.associatorNames(NAMESPACE, op, assocClass,
            resultClass, role, resultRole);

        // verify result
        _verifyResult(resultObjectPaths.size(), numExpectedObjects);

        // display result
        _displayResult(resultObjectPaths);
    }
    catch (Exception& e)
    {
        _errorExit(e.getMessage());
    }
}

////////////////////////////////////////////////////////////////////////////
//  _testReferenceFilter
////////////////////////////////////////////////////////////////////////////

void _testReferenceFilter(
    CIMClient& client,
    const String& instancePath,
    const String& role,
    const String& resultRole,
    const CIMName& resultClass,
    const CIMName& assocClass,
    Uint32 numExpectedObjects)
{
    Array<CIMObjectPath> resultObjectPaths;
    Array<CIMObject> resultObjects;
    CIMObjectPath op(instancePath);

    // =======================================================================
    // references
    // =======================================================================

    if (verbose)
    {
        cout << "\n+++++ Test references with filters (" << instancePath;
        cout << ")" << endl;
    }

    try
    {
        // get the association reference instances
        resultObjects = client.references(NAMESPACE, op, resultClass, role);

        // verify result
        _verifyResult(resultObjects.size(), numExpectedObjects);

        // display result
        _displayResult(resultObjects);
    }
    catch (Exception& e)
    {
        _errorExit(e.getMessage());
    }

    // =======================================================================
    // referenceNames
    // =======================================================================

    if (verbose)
    {
        cout << "\n+++++ Test referenceNames with filters (" << instancePath;
        cout << ")" << endl;
    }

    try
    {
        // get the reference instance names
        resultObjectPaths =
            client.referenceNames(NAMESPACE, op, resultClass, role);

        // verify result
        _verifyResult(resultObjectPaths.size(), numExpectedObjects);

        // display result
        _displayResult(resultObjectPaths);
    }
    catch (Exception& e)
    {
        _errorExit(e.getMessage());
    }
}

////////////////////////////////////////////////////////////////////////////
//  _testAssociationClassOperations
////////////////////////////////////////////////////////////////////////////

void _testAssociationClassOperations(
    CIMClient& client,
    const CIMName& className)
{
    Array<CIMObjectPath> resultObjectPaths;
    Array<CIMObject> resultObjects;
    CIMObjectPath op(className.getString());

    CIMName assocClass;
    CIMName resultClass;
    String role;
    String resultRole;

    // =======================================================================
    // associators
    //
    // Get the CIM classes that are associated with the specified CIM Class
    // =======================================================================

    if (verbose)
    {
        cout << "\n+++++ Test associators for (" << className.getString();
        cout << ")" << endl;
    }

    try
    {
        // get the association classes
        resultObjects = client.associators(NAMESPACE, op, assocClass,
            resultClass, role, resultRole);

        // display result
        _displayResult(resultObjects);
    }
    catch (Exception& e)
    {
        _errorExit(e.getMessage());
    }

    // =======================================================================
    // associatorNames
    //
    // Get the name of the CIM classes that are associated with the specified
    // CIM class.
    // =======================================================================

    if (verbose)
    {
        cout << "\n+++++ Test associatorNames for (" << className.getString();
        cout << ")" << endl;
    }

    try
    {
        resultObjectPaths = client.associatorNames(NAMESPACE, op, assocClass,
            resultClass, role, resultRole);

        // display result
        _displayResult(resultObjectPaths);
    }
    catch (Exception& e)
    {
        _errorExit(e.getMessage());
    }

    // =======================================================================
    // references
    //
    // Get the association classes that refer to the specified CIM class.
    // =======================================================================

    if (verbose)
    {
        cout << "\n+++++ Test references for (" << className.getString() << ")"
             << endl;
    }

    try
    {
        resultObjects = client.references(NAMESPACE, op, resultClass, role);

        // display result
        _displayResult(resultObjects);
    }
    catch (Exception& e)
    {
        _errorExit(e.getMessage());
    }

    // =======================================================================
    // referenceNames
    //
    // Get the names of the association classes that refer to the specified
    // CIM class.
    // =======================================================================

    if (verbose)
    {
        cout << "\n+++++ Test referenceNames for (" << className.getString()
             << ")" << endl;
    }

    try
    {
        resultObjectPaths =
            client.referenceNames(NAMESPACE, op, resultClass, role);

        // display result
        _displayResult(resultObjectPaths);
    }
    catch (Exception& e)
    {
        _errorExit(e.getMessage());
    }
}

// =========================================================================
//    Main
// =========================================================================

int main(int argc, char** argv)
{
    //
    // Check command line option
    //
    if (argc > 2)
    {
        cerr << "Usage: AssociationTestClient [-v]" << endl;
        return 1;
    }

    if (argc == 2)
    {
        const char *opt = argv[1];
        if (strcmp(opt, "-v") == 0)
        {
            verbose = true;
        }
        else
        {
            cerr << "Usage: AssociationTestClient [-v]" << endl;
            return 1;
        }
    }

    CIMClient client;

    // Connect to server
    try
    {
        client.connectLocal();
    }
    catch (Exception& e)
    {
        _errorExit(e.getMessage());
    }

    // =======================================================================
    // Test passing Instance object path to the Association Methods
    // =======================================================================

    cout << "+++++ Test Sample Association Provider" << endl;

    // Get the instance object paths for the Teacher and Student class
    //
    Array<CIMObjectPath> teacherRefs;
    Array<CIMObjectPath> studentRefs;
    try
    {
        teacherRefs =
            client.enumerateInstanceNames(NAMESPACE, SAMPLE_TEACHER);
        studentRefs =
            client.enumerateInstanceNames(NAMESPACE, SAMPLE_STUDENT);
    }
    catch (Exception& e)
    {
        cout << "enumerateInstanceNames() failed." << endl;
        _errorExit(e.getMessage());
    }

    Uint32 numTeacherInstances = teacherRefs.size();
    Uint32 numStudentInstances = studentRefs.size();

    // =======================================================================
    // Test associators
    //
    // Get the CIM instances that are associated with the specified source CIM
    // instance via an instance of a specified association class.
    // =======================================================================

    cout << "\n+++++ Test associators" << endl;
    for (Uint32 i = 0; i < numTeacherInstances; i++)
    {
       _testAssociators(client, SAMPLE_TEACHERSTUDENT, teacherRefs[i],
           resultArray_asso_T1[i]);
       _testAssociators(client, SAMPLE_ADVISORSTUDENT, teacherRefs[i],
           resultArray_asso_T2[i]);
    }

    for (Uint32 i = 0; i < numStudentInstances; i++)
    {
       _testAssociators(client, SAMPLE_TEACHERSTUDENT, studentRefs[i],
           resultArray_asso_S1[i]);
       _testAssociators(client, SAMPLE_ADVISORSTUDENT, studentRefs[i],
           resultArray_asso_S2[i]);
    }

    // =======================================================================
    // Test associatorNames
    //
    // Get the names of the CIM instances that are associated with the specified
    // source CIM instance via an instance of a association class.
    // =======================================================================

    cout << "\n+++++ Test associatorNames" << endl;
    for (Uint32 i = 0; i < numTeacherInstances; i++)
    {
        _testAssociatorNames(client, SAMPLE_TEACHERSTUDENT, teacherRefs[i],
            resultArray_asso_T1[i]);
        _testAssociatorNames(client, SAMPLE_ADVISORSTUDENT, teacherRefs[i],
            resultArray_asso_T2[i]);
    }
    for (Uint32 i = 0; i < numStudentInstances; i++)
    {
        _testAssociatorNames(client, SAMPLE_TEACHERSTUDENT, studentRefs[i],
            resultArray_asso_S1[i]);
        _testAssociatorNames(client, SAMPLE_ADVISORSTUDENT, studentRefs[i],
            resultArray_asso_S2[i]);
    }

    // =======================================================================
    // Test references
    //
    // Get the association instances that refer to the specified target CIM
    // instance.
    // =======================================================================

    cout << "\n+++++ Test references" << endl;
    for (Uint32 i = 0; i < numTeacherInstances; i++)
    {
        _testReferences(client, teacherRefs[i], resultArray_ref_T[i]);
    }
    for (Uint32 i = 0; i < numStudentInstances; i++)
    {
        _testReferences(client, studentRefs[i], resultArray_ref_S[i]);
    }

    // =======================================================================
    // Test referenceNames
    //
    // Get the names of the association instances that refer to the specified
    // source CIM instance.
    // =======================================================================

    cout << "\n+++++ Test referenceNames" << endl;
    for (Uint32 i = 0; i < numTeacherInstances; i++)
    {
        _testReferenceNames(client, teacherRefs[i], resultArray_ref_T[i]);
    }
    for (Uint32 i = 0; i < numStudentInstances; i++)
    {
        _testReferenceNames(client, studentRefs[i], resultArray_ref_S[i]);
    }

    // =======================================================================
    // Test using filters in the associators and associatorNames methods.
    //
    // Filters used are: role, resultClass, resultRole
    //
    // =======================================================================

    String role;
    String resultRole;
    CIMName resultClass;
    CIMName assocClass;
    Uint32 numExpectedObjects = 0;

    // good filters
    role = "Teaches";
    resultRole = "TaughtBy";
    resultClass = SAMPLE_STUDENT;
    numExpectedObjects = 3;

    _testAssociatorFilters(client, "Sample_Teacher.Name=\"Teacher1\"", role,
        resultRole, resultClass, assocClass,
        numExpectedObjects,
        "all filters");

    // good filters
    role = "Advises";
    resultRole = "AdvisedBy";
    resultClass = SAMPLE_STUDENT;
    numExpectedObjects = 2;

    _testAssociatorFilters(client, "Sample_Teacher.Name=\"Teacher1\"", role,
        resultRole, resultClass, assocClass,
        numExpectedObjects,
        "all filters");

    // good filters
    role = "TaughtBy";
    resultRole = "Teaches";
    resultClass = CIMName("Sample_Teacher");
    numExpectedObjects = 3;

    _testAssociatorFilters(client, "Sample_Student.Name=\"Student1\"", role,
        resultRole, resultClass, assocClass,
        numExpectedObjects,
        "all filters");

    // good filters
    role = "AdvisedBy";
    resultRole = "Advises";
    resultClass = CIMName("Sample_Teacher");
    numExpectedObjects = 1;

    _testAssociatorFilters(client, "Sample_Student.Name=\"Student1\"", role,
        resultRole, resultClass, assocClass,
        numExpectedObjects,
        "all filters");

    // set only one filter - resultClass
    // Note:  since AssocClass is not specified, duplicate instances are
    //        returned.
    //
    role = String::EMPTY;
    resultRole = String::EMPTY;
    resultClass = CIMName();
    numExpectedObjects = 5;

    _testAssociatorFilters(client, "Sample_Teacher.Name=\"Teacher1\"", role,
        resultRole, resultClass, assocClass,
        numExpectedObjects,
        "resultClass filter");

    // set only one filter - role
    //
    role = "Teaches";
    resultRole = String::EMPTY;
    resultClass = CIMName();
    numExpectedObjects = 3;

    _testAssociatorFilters(client, "Sample_Teacher.Name=\"Teacher1\"", role,
        resultRole, resultClass, assocClass,
        numExpectedObjects,
        "role filter");

    // set only one filter - resultRole
    //
    role = String::EMPTY;
    resultRole = "TaughtBy";
    resultClass = CIMName();
    numExpectedObjects = 3;

    _testAssociatorFilters(client, "Sample_Teacher.Name=\"Teacher1\"", role,
        resultRole, resultClass, assocClass,
        numExpectedObjects,
        "resultRole filter");

    // bad parameter - role
    role = "TaughtBy";
    resultRole = "TaughtBy";
    resultClass = SAMPLE_STUDENT;
    numExpectedObjects = 0;

    _testAssociatorFilters(client, "Sample_Teacher.Name=\"Teacher1\"", role,
        resultRole, resultClass, assocClass,
        numExpectedObjects,
        "bad role filter");

    // bad parameter - resultRole
    role = "Teaches";
    resultRole = "Teaches";
    resultClass = SAMPLE_STUDENT;
    numExpectedObjects = 0;

    _testAssociatorFilters(client, "Sample_Teacher.Name=\"Teacher1\"", role,
        resultRole, resultClass, assocClass,
        numExpectedObjects,
        "bad resultRole filter");

    // bad parameter - resultClass
    role = "Teaches";
    resultRole = "TaughtBy";
    resultClass = CIMName("Sample_Teacher");
    numExpectedObjects = 0;

    _testAssociatorFilters(client, "Sample_Teacher.Name=\"Teacher1\"", role,
        resultRole, resultClass, assocClass,
        numExpectedObjects,
        "bad resultClass filter");

    // =======================================================================
    // Test passing Class object path to the Association Methods
    // =======================================================================

    cout << "\n+++++ Test association class operations" << endl;
    _testAssociationClassOperations(client, SAMPLE_TEACHER);
    _testAssociationClassOperations(client, SAMPLE_STUDENT);

    // =======================================================================
    // Association tests completed
    // =======================================================================

    cout << "\n+++++ passed all tests" << endl;
    return 0;
}
