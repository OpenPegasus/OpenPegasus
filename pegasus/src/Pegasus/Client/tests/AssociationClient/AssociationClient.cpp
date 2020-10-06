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
// This is a sample CIM client application that calls the CIMClient association
// methods (associators, associatorNames, references, referenceNames) to
// get information about the relationships between the Sample_Teacher and
// Sample_Student classes.
//
// The SampleAssociationProvider is the instance and association provider
// for the following classes:
//
//     Sample_Teacher
//     Sample_Student
//     Sample_TeacherStudent    (association class)
//     Sample_AdvisorStudent    (association class)
//
// The executable for this CIM client application is:  AssociationClient.
// To display the result objects (CIM instances or classes), use the -v
// (verbose) option in the command line:  "AssociationClient -v".
//
// The following are example result output when verbose option is specified:
//
// (ex.)
//    Number of associator objects = 2
//    //server1/root/SampleProvider:Sample_Student.Name="Student1"
//    //server1/root/SampleProvider:Sample_Student.Name="Student2"
//
// (ex.)
//    Number of reference objects = 2
//    //server1/root/SampleProvider:Sample_TeacherStudent.TaughtBy="Sample_
//    Student.Name=\"Student1\"",Teaches="Sample_Teacher.Name=\"Teacher1\""
//    //server1/root/SampleProvider:Sample_TeacherStudent.TaughtBy="Sample_
//    Student.Name=\"Student2\"",Teaches="Sample_Teacher.Name=\"Teacher1\""
//
// Test results are verified by comparing the number of returned objects
// with the expected number of returned objects.  Any unexpected result
// is considered a failure.  If there is a failure, an error message is
// displayed and the program terminates.
//

#include <Pegasus/Common/Config.h>
#include <Pegasus/Client/CIMClient.h>
#include <Pegasus/Common/InternalException.h>
#include <string.h>

PEGASUS_USING_STD;
PEGASUS_USING_PEGASUS;

const CIMNamespaceName NAMESPACE = CIMNamespaceName("root/SampleProvider");
const CIMName SAMPLE_TEACHER = CIMName("Sample_Teacher");
const CIMName SAMPLE_STUDENT = CIMName("Sample_Student");
const CIMName SAMPLE_TEACHERSTUDENT = CIMName("Sample_TeacherStudent");
const CIMName SAMPLE_ADVISORSTUDENT = CIMName("Sample_AdvisorStudent");

//
//  Verify that the number of objects returned is as expected.  If the
//  number of objects returned is not expected, display an error and
//  terminate the program.
//
int _verifyResult(const Uint32 numObjects, const Uint32 expectedNumObjects)
{
    if (numObjects != expectedNumObjects)
    {
        cerr << "Error: Unexpected number of objects returned.  ";
        cerr << "Expected " << expectedNumObjects << " object(s), but ";
        cerr << numObjects << " object(s) were returned." << endl;
        return 1;
    }
    return 0;
}

//
//  Display the result objects if verbose option is specified.
//
void _displayResult(const Array<CIMObject> & objects, Boolean verbose)
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

//
//  Display the result object names if verbose option is specified.
//
void _displayResult(const Array<CIMObjectPath> & objectPaths, Boolean verbose)
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

///////////////////////////////////////////////////////////////////////
//    MAIN
///////////////////////////////////////////////////////////////////////

int main(int argc, char** argv)
{
    CIMClient               client;
    Boolean                 verbose = false;
    Uint32                  numObjects;
    Array<CIMObjectPath>    resultObjectPaths;
    Array<CIMObject>        resultObjects;
    CIMName                 assocClass;
    CIMName                 resultClass;
    String                  role;
    String                  resultRole;

    //
    // Check command line option
    //
    if (argc > 2)
    {
        cerr << "Usage: AssociationClient [-v]" << endl;
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
            cerr << "Usage: AssociationClient [-v]" << endl;
            return 1;
        }
    }

    try
    {
        // ===================================================================
        // connectLocal
        //
        // The connectLocal Client API creates a connection to the server for
        // a local client.  The connection is automatically authenticated
        // for the current user.  (The connect Client API can be used to create
        // an HTTP connection with the server defined by the URL in address.
        // User name and Password information can be passed using the connect
        // Client API.)
        // ===================================================================

        client.connectLocal();

        // ===================================================================
        // associators
        //
        // Get the CIM instances (Sample_Student instances) that are associated
        // to the source CIM instance (Sample_Teacher.Name = "Teacher1") via an
        // instance of the Sample_TeacherStudent association class.
        // ===================================================================

        assocClass = SAMPLE_TEACHERSTUDENT;
        CIMObjectPath instancePath("Sample_Teacher.Name=\"Teacher1\"");

        resultObjects = client.associators(
                                NAMESPACE,
                                instancePath,
                                assocClass,
                                resultClass,
                                role,
                                resultRole);

        // verify result
        numObjects  = resultObjects.size();
        if (_verifyResult(numObjects, 3) != 0)
            return -1;

        // display result
        // cout << "Number of associator objects = " << numObjects << endl;
        _displayResult(resultObjects, verbose);

        // ===================================================================
        // associators
        //
        // Validate role and resultRole parameters syntax.
        // ===================================================================

        // invalid role parameter syntax
        String invalidRole = "Teaches_*student";

        Boolean gotException = false;
        try
        {
            resultObjects = client.associators(
                                    NAMESPACE,
                                    instancePath,
                                    assocClass,
                                    resultClass,
                                    invalidRole,
                                    resultRole);

        }
        catch (CIMException& e)
        {
            if (e.getCode() == CIM_ERR_INVALID_PARAMETER)
            {
                gotException = true;
                if (verbose)
                {
                    cout << "Test role parameter syntax: " <<
                        e.getMessage() << endl;
                }
            }
            else
            {
                throw;
            }
        }
        PEGASUS_TEST_ASSERT(gotException);

        // invalid resultRole parameter syntax
        String invalidResultRole = "3Taught_By";
        gotException = false;
        try
        {
            resultObjects = client.associators(
                                    NAMESPACE,
                                    instancePath,
                                    assocClass,
                                    resultClass,
                                    role,
                                    invalidResultRole);

        }
        catch (CIMException& e)
        {
            if (e.getCode() == CIM_ERR_INVALID_PARAMETER)
            {
                gotException = true;
                if (verbose)
                {
                    cout << "Test resultRole parameter syntax: " <<
                        e.getMessage() << endl;
                }
            }
            else
            {
                throw;
            }
        }
        PEGASUS_TEST_ASSERT(gotException);

        // ===================================================================
        // associatorNames
        //
        // Get the names of the CIM instances (Sample_Student instances) that
        // are associated to the source CIM instance (Sample_Teacher.Name =
        // "Teacher1") via an instance of the Sample_TeacherStudent association
        // class.
        // ===================================================================

        resultObjectPaths = client.associatorNames(
                                NAMESPACE,
                                instancePath,
                                assocClass,
                                resultClass,
                                role,
                                resultRole);

        // verify result
        numObjects = resultObjectPaths.size();
        if (_verifyResult(numObjects, 3) != 0)
            return -1;

        // display result
        // cout << "Number of associator name objects = " << numObjects << endl;
        _displayResult(resultObjectPaths, verbose);

        // ===================================================================
        // associatorNames
        //
        // Validate role and resultRole parameters syntax.
        // ===================================================================

        // invalid role parameter syntax
        gotException = false;
        try
        {
            resultObjectPaths = client.associatorNames(
                                    NAMESPACE,
                                    instancePath,
                                    assocClass,
                                    resultClass,
                                    invalidRole,
                                    resultRole);
        }
        catch (CIMException& e)
        {
            if (e.getCode() == CIM_ERR_INVALID_PARAMETER)
            {
                gotException = true;
                if (verbose)
                {
                    cout << "Test role parameter syntax: " <<
                        e.getMessage() << endl;
                }
            }
            else
            {
                throw;
            }
        }
        PEGASUS_TEST_ASSERT(gotException);

        // invalid resultRole parameter syntax
        gotException = false;
        try
        {
            resultObjectPaths = client.associatorNames(
                                    NAMESPACE,
                                    instancePath,
                                    assocClass,
                                    resultClass,
                                    role,
                                    invalidResultRole);
        }
        catch (CIMException& e)
        {
            if (e.getCode() == CIM_ERR_INVALID_PARAMETER)
            {
                gotException = true;
                if (verbose)
                {
                    cout << "Test resultRole parameter syntax: " <<
                        e.getMessage() << endl;
                }
            }
            else
            {
                throw;
            }
        }
        PEGASUS_TEST_ASSERT(gotException);

        // ===================================================================
        // references
        //
        // Get the association instances (Sample_TeacherStudent instances) that
        // refer to the specified target CIM instance (Sample_Teacher.Name =
        // "Teacher1").
        // ===================================================================

        resultObjects = client.references(
                                NAMESPACE,
                                instancePath,
                                resultClass,
                                role);

        // verify result
        numObjects = resultObjects.size();
        if (_verifyResult(numObjects, 5) != 0)
            return -1;

        // display result
        // cout << "Number of reference objects = " << numObjects << endl;
        _displayResult(resultObjects, verbose);

        // ===================================================================
        // references
        //
        // Validate role parameter syntax.
        // ===================================================================

        // invalid role parameter syntax
        gotException = false;
        try
        {
            resultObjects = client.references(
                                    NAMESPACE,
                                    instancePath,
                                    resultClass,
                                    invalidRole);
        }
        catch (CIMException& e)
        {
            if (e.getCode() == CIM_ERR_INVALID_PARAMETER)
            {
                gotException = true;
                if (verbose)
                {
                    cout << "Test role parameter syntax: " <<
                        e.getMessage() << endl;
                }
            }
            else
            {
                throw;
            }
        }
        PEGASUS_TEST_ASSERT(gotException);

        // ===================================================================
        // referenceNames
        //
        // Get the names of the association instances (Sample_TeacherStudent
        // instances) that refer to the specified target CIM instance
        // (Sample_Teacher.Name = "Teacher1").
        // ===================================================================

        resultObjectPaths = client.referenceNames(
                                NAMESPACE,
                                instancePath,
                                resultClass,
                                role);

        // verify result
        numObjects = resultObjectPaths.size();
        if (_verifyResult(numObjects, 5) != 0)
            return -1;

        // display result
        // cout << "Number of reference name objects = " << numObjects << endl;
        _displayResult(resultObjectPaths, verbose);

        // ===================================================================
        // referenceNames
        //
        // Validate role parameter syntax.
        // ===================================================================

        // invalid role parameter syntax
        gotException = false;
        try
        {
            resultObjectPaths = client.referenceNames(
                                        NAMESPACE,
                                        instancePath,
                                        resultClass,
                                        invalidRole);
        }
        catch (CIMException& e)
        {
            if (e.getCode() == CIM_ERR_INVALID_PARAMETER)
            {
                gotException = true;
                if (verbose)
                {
                    cout << "Test role parameter syntax: " <<
                        e.getMessage() << endl;
                }
            }
            else
            {
                throw;
            }
        }
        PEGASUS_TEST_ASSERT(gotException);

        // ===================================================================
        // Call the association methods with different filters specified.
        // Filters used are: role, resultClass, resultRole, assocClass.
        // ===================================================================

        //
        // get all the students who are taught by 'Teacher1'
        //
        role = "Teaches";
        resultRole = "TaughtBy";
        resultClass = CIMName("Sample_Student");
        assocClass = SAMPLE_TEACHERSTUDENT;

        resultObjects = client.associators(
                                NAMESPACE,
                                instancePath,
                                assocClass,
                                resultClass,
                                role,
                                resultRole);
        // verify result
        numObjects = resultObjects.size();
        if (_verifyResult(numObjects, 3) != 0)
            return -1;

        // display result
        // cout << "Number of associator objects = " << numObjects << endl;
        _displayResult(resultObjects, verbose);

        //
        // get all the students who have 'Teacher1' as an advisor
        //
        role = "Advises";
        resultRole = "AdvisedBy";
        resultClass = CIMName("Sample_Student");
        assocClass = SAMPLE_ADVISORSTUDENT;

        resultObjectPaths = client.associatorNames(
                                NAMESPACE,
                                instancePath,
                                assocClass,
                                resultClass,
                                role,
                                resultRole);

        // verify result
        numObjects = resultObjectPaths.size();
        if (_verifyResult(numObjects, 2) != 0)
            return -1;

        // display result
        // cout << "Number of associator name objects = " << numObjects << endl;
        _displayResult(resultObjectPaths, verbose);

        //
        // get all the TeacherStudent association instances in which 'Teacher1'
        // plays the role of a teacher.
        //
        role = "Teaches";
        resultClass = CIMName("Sample_TeacherStudent");

        resultObjects = client.references(
                                NAMESPACE,
                                instancePath,
                                resultClass,
                                role);

        // verify result
        numObjects = resultObjects.size();
        if (_verifyResult(numObjects, 3) != 0)
            return -1;

        // display result
        // cout << "Number of reference objects = " << numObjects << endl;
        _displayResult(resultObjects, verbose);

        //
        // get all the AdvisorStudent association instances in which 'Teacher1'
        // plays the role of an advisor.
        //
        role = "Advises";
        resultClass = CIMName("Sample_AdvisorStudent");

        resultObjectPaths = client.referenceNames(
                                NAMESPACE,
                                instancePath,
                                resultClass,
                                role);

        // verify result
        numObjects = resultObjectPaths.size();
        if (_verifyResult(numObjects, 2) != 0)
            return -1;

        // display result
        // cout << "Number of reference objects = " << numObjects << endl;
        _displayResult(resultObjectPaths, verbose);

        // ===================================================================
        // Pass Class object to the Association Methods
        // ===================================================================

        //
        // get the CIM classes that are associated with the Sample_Teacher class
        //
        CIMObjectPath classPath("Sample_Teacher");
        assocClass = CIMName();
        resultClass = CIMName();
        role = String::EMPTY;
        resultRole = String::EMPTY;

        resultObjects = client.associators(
                                NAMESPACE,
                                classPath,
                                assocClass,
                                resultClass,
                                role,
                                resultRole);

        // verify result
        numObjects  = resultObjects.size();
        if (_verifyResult(numObjects, 1) != 0)
            return -1;

        // display result
        // cout << "Number of associated class objects = " <<
        //     numObjects << endl;
        _displayResult(resultObjects, verbose);

        //
        // get the association classes that refer to the Sample_Teacher class
        //
        resultObjects = client.references(
                                NAMESPACE,
                                classPath,
                                resultClass,
                                role);

        // verify result
        numObjects  = resultObjects.size();
        if (_verifyResult(numObjects, 2) != 0)
            return -1;

        // display result
        // cout << "Number of association class objects = " <<
        //     numObjects << endl;
        _displayResult(resultObjects, verbose);
    }
    catch (Exception& e)
    {
        cerr << "Error: " << e.getMessage() << endl;
        return -1;
    }

    cout << "AssociationClient +++++ passed all tests" << endl;

    return 0;
}
