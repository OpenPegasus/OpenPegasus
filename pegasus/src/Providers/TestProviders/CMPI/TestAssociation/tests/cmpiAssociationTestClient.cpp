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

// This program makes CIMClient association method calls for CMPI (associators,
// associatorNames, references, referenceNames) to get information about the
// relationships between the CMPI_TEST_Person and CMPI_TEST_Vehicle classes.
//
// The CMPIAssociationProvider is the instance and association provider
// for the following classes:
//
//     CMPI_TEST_Person
//     CMPI_TEST_Vehicle
//     CMPI_TEST_Racing  (association class)
//
// The executable for this CIM client application is: cmpiAssociationTestClient.
// To display the test results (returned instances or classes), define the
// environment variable PEGASUS_TEST_VERBOSE.
//
// Test results are verified by comparing the number of returned objects
// with the expected number of returned objects.  Any unexpected result
// is considered a failure.  If there is a failure, an error message is
// displayed and the program terminates.

#include <Pegasus/Client/CIMClient.h>
#include <string.h>

#include <Pegasus/Provider/CMPI/cmpidt.h>
#include <Pegasus/Provider/CMPI/cmpift.h>
#include <Pegasus/Provider/CMPI/cmpimacs.h>
#include <Pegasus/Common/PegasusAssert.h>

PEGASUS_USING_STD;
PEGASUS_USING_PEGASUS;

CIMNamespaceName providerNamespace;
const CIMName CMPI_TEST_PERSON = CIMName ("CMPI_TEST_Person");
const CIMName CMPI_TEST_VEHICLE = CIMName ("CMPI_TEST_Vehicle");
const CIMName CMPI_TEST_RACING = CIMName ("CMPI_TEST_Racing");


Boolean verbose = false;

// exepected results - number of returned objects expected for each test

static const Uint32 resultArray_asso_P1[] = { 3, 3 };
static const Uint32 resultArray_asso_V1[] = { 3, 3 };
static const Uint32 resultArray_ref_P[] =   { 3, 3, 3, 3 };
static const Uint32 resultArray_ref_V[] =   { 3, 3, 3, 3 };

////////////////////////////////////////////////////////////////////////////
//  _errorExit
////////////////////////////////////////////////////////////////////////////

void _errorExit(String message)
{
    cerr << "cmpiAssociationTestClient error: " << message << endl;
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
void _verifyResult(const Uint32 numObjects, const Uint32 numExpectedObjects)
{
    if (numObjects != numExpectedObjects)
    {
        cerr << "Error: Unexpected number of objects returned. " << endl;;
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
void _displayResult(const Array<CIMObject> & objects)
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
void _displayResult(const Array<CIMObjectPath> & objectPaths)
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
    CIMName assocClass,
    CIMObjectPath instancePath,
    Uint32 numExpectedObjects)
{
    if (verbose)
    {
        cout << "Association Class: " << assocClass.getString() << endl;
        cout << "Object Name: " << instancePath.toString() << endl;
    }

    try
    {
        CIMName resultClass;
        String role;
        String resultRole;

        // Get the CIM instances that are associated with the specified source
        // instance via an instance of the AssocClass
        //

        Array<CIMObject> resultObjects =
            client.associators(providerNamespace, instancePath, assocClass,
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
}

////////////////////////////////////////////////////////////////////////////
//  _testAssociatorNames
////////////////////////////////////////////////////////////////////////////

void _testAssociatorNames(CIMClient& client, CIMName assocClass,
                          CIMObjectPath instancePath, Uint32 numExpectedObjects)
{
    if (verbose)
    {
        cout << "Association Class: " << assocClass.getString() << endl;
        cout << "Object Name: " << instancePath.toString() << endl;
    }

    try
    {
        // Get the names of the CIM instances that are associated to the
        // specified source instance via an instance of the AssocClass.
        //
        CIMName resultClass;

        String role;
        String resultRole;

        Array<CIMObjectPath> resultObjectPaths =
            client.associatorNames(providerNamespace, instancePath,
                                   assocClass, resultClass, role, resultRole);
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

void _testReferences(CIMClient& client, CIMObjectPath instancePath,
                     Uint32 numExpectedObjects)
{
    if (verbose)
    {
        cout << "Object Name: " << instancePath.toString() << endl;
    }

    try
    {
        // get the association reference instances
        //
        Array<CIMObject> resultObjects;
        CIMName resultClass;
        String role;

        resultObjects = client.references(providerNamespace, instancePath,
                                          resultClass,role);
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

void _testReferenceNames(CIMClient& client, CIMObjectPath instancePath,
                         Uint32 numExpectedObjects)
{
    if (verbose)
    {
        cout << "Object Name: " << instancePath.toString() << endl;
    }

    try
    {
        // get the reference instance names
        //
        Array<CIMObjectPath> resultObjectPaths;
        CIMName resultClass;
        String role;

        resultObjectPaths = client.referenceNames(
            providerNamespace,
            instancePath,
            resultClass,
            role);

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
//  _testCMPIAssociationClassOperations
////////////////////////////////////////////////////////////////////////////

void _testCMPIAssociationClassOperations(CIMClient& client, CIMName className)
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
        cout << "+++++ Test associators for (" << className.getString();
        cout << ")" << endl;
    }

    try
    {
        // get the association classes
        resultObjects = client.associators(providerNamespace, op, assocClass,
            resultClass, role, resultRole);

        // display result
        _displayResult(resultObjects);
    }
    catch (Exception& e)
    {
        // Do nothing.
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
        cout << "+++++ Test associatorNames for (" << className.getString();
        cout << ")" << endl;
    }

    try
    {
        resultObjectPaths = client.associatorNames(
            providerNamespace,
            op,
            assocClass,
            resultClass,
            role,
            resultRole);

        // display result
        _displayResult(resultObjectPaths);
    }
    catch (Exception& e)
    {
        // Do nothing.
         _errorExit(e.getMessage());
    }

    // =======================================================================
    // references
    //
    // Get the association classes that refer to the specified CIM class.
    // =======================================================================

    if (verbose)
    {
        cout << "+++++ Test references for (" << className.getString()
             << ")" << endl;
    }

    try
    {
        resultObjects = client.references(
            providerNamespace,
            op,
            resultClass,
            role);

        // display result
        _displayResult(resultObjects);
    }
    catch (Exception& e)
    {
        // Do nothing.
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
        cout << "+++++ Test referenceNames for (" << className.getString();
        cout << ")" << endl;
    }

    try
    {
        resultObjectPaths =
            client.referenceNames(providerNamespace, op, resultClass, role);

        // display result
        _displayResult(resultObjectPaths);
    }
    catch (Exception& e)
    {
        // Do nothing.
         _errorExit(e.getMessage());
    }
}

void _testCMPIFilterOfAssociation(CIMClient& client)
{
    try
    {
        Array<CIMObjectPath> personRefs;
        Uint32 numPersonInstances ;
        personRefs = 
            client.enumerateInstanceNames(providerNamespace,CMPI_TEST_PERSON);
        numPersonInstances = personRefs.size();
        CIMName resultClass;
        String role;
        String resultRole;
        cout<<"++++++++Filtering the NULL propery list+++"<<endl;
        for (Uint32 i = 0; i < numPersonInstances; ++i)
        {
            Array<CIMObject> resultObjects =
                client.associators(
                    providerNamespace,
                    personRefs[i],
                    CMPI_TEST_RACING,
                    resultClass,
                    role,
                    resultRole,
                    false,
                    false,
                    CIMPropertyList());
            Uint32 size = resultObjects.size();
            for(Uint32 j = 0;j<size;j++)
            {
                Uint32 propCount = resultObjects[j].getPropertyCount();
                Uint32 propNameCount = 0;
                if(propCount != 0)
                {
                    String propName=
                        resultObjects[j].getProperty(0).getName().getString();
                    if(verbose)
                    {
                        cout<<"Property Name of :"<<i<<":"<<propName<<endl;
                    }
                    if(propName == "vehiclename") 
                    {
                        propNameCount++;
                    }
                }
                if((size != 0)&&(propCount == 1) &&(propNameCount == 1))
                {
                    cout<<"Filter associator test on CMPI_TEST_RACING SUCCEEDED"
                        <<":Filtering the ciminstance with a NULL property list"
                        <<" returned all properties as expected"<<endl;
                }
                else
                {
                    cout<<"Filter associator test on CMPI_TEST_RACING FAILED"
                        <<":Filtering the ciminstance with a NULL property list"
                        <<" did not return all properties as expected"<<endl;
                    PEGASUS_TEST_ASSERT(false);
                }
            }

        }
        cout<<"++++++Filtering the empty propery list+++"<<endl;
        Array<CIMName> propList;
        for (Uint32 i = 0; i < numPersonInstances; ++i)
        {
            Array<CIMObject> resultObjects =
                client.associators(
                    providerNamespace,
                    personRefs[i],
                    CMPI_TEST_RACING,
                    resultClass,
                    role,
                    resultRole,
                    false,
                    false,
                    CIMPropertyList(propList));
            Uint32 size = resultObjects.size();
            for(Uint32 j = 0;j<size;j++)
            {
                Uint32 propCount = resultObjects[j].getPropertyCount();
                if((size != 0)&&(propCount == 0))
                {
                    cout<<"Filter associator test on CMPI_TEST_RACING SUCCEEDED"
                        <<":Filtering the ciminstance with a empty property "
                        <<"list returned zero properties as expected"<<endl;
                }
                else
                { 
                    cout<<"Filter associators test on CMPI_TEST_RACING FAILED"
                        <<":Filtering the ciminstance with a empty property "
                        <<"list returned some properties which is not expected"
                        <<endl; 
                    PEGASUS_TEST_ASSERT(false);
                }
            }
        }
        cout<<"+++filtering the wrong properties ++++++"<<endl;
        Array<CIMName> propList1;
        propList1.append(CIMName(String("nam")));
        for (Uint32 i = 0; i < numPersonInstances; ++i)
        {
            Array<CIMObject> resultObjects =
                client.associators(
                    providerNamespace,
                    personRefs[i],
                    CMPI_TEST_RACING,
                    resultClass,
                    role,
                    resultRole,
                    false,
                    false,
                    CIMPropertyList(propList1));
            Uint32 size = resultObjects.size();
            for(Uint32 j = 0;j<size;j++)
            {
                Uint32 propCount = resultObjects[j].getPropertyCount();
                if((size != 0)&&(propCount == 0))
                {
                    cout<<"Filter associators test on CMPI_TEST_RACING "
                        <<"SUCCEEDED:Filtering the ciminstance with a wrong "
                        <<"property list returned zero properties as " 
                        <<"expected"<<endl;
                }
                else
                {
                    cout<<"Filter associators test on CMPI_TEST_RACING FAILED"
                         <<":Filtering the ciminstance with a wrong property "
                         <<"list returned some properties which is not"
                         <<" expected"<<endl; 
                    PEGASUS_TEST_ASSERT(false);
                }
            }
        }
        cout<<"++++++++Filtering the mentioned propery list+++"<<endl;
        Array<CIMName> propArr;
        propArr.append(CIMName(String("vehiclename")));
        for (Uint32 i = 0; i < numPersonInstances; ++i)
        {
            Array<CIMObject> resultObjects =
                client.associators(
                    providerNamespace,
                    personRefs[i],
                    CMPI_TEST_RACING,
                    resultClass,
                    role,
                    resultRole,
                    false,
                    false,
                    CIMPropertyList( propArr));
            Uint32 size = resultObjects.size();
            for(Uint32 j = 0;j<size;j++)
            {
                Uint32 propCount = resultObjects[j].getPropertyCount();
                Uint32 propNameCount = 0;
                if(propCount != 0)
                {
                    String propName=
                        resultObjects[j].getProperty(0).getName().getString();
                    if(verbose)
                    {
                        cout<<"Property Name of :"<<i<<":"<<propName<<endl;
                    }
                    if(propName == "vehiclename")
                    {
                        propNameCount++;
                    }
                }
                if((size != 0)&&(propCount == 1) &&(propNameCount == 1))
                {
                     cout<<"Filter associators test on CMPI_TEST_RACING "
                         <<"SUCCEEDED:Filtering the ciminstance with a "
                         <<"mentioned property list returned all properties"
                         <<" as expected"<<endl;
                }
                else
                {
                    cout<<"Filter associators test on CMPI_TEST_RACING FAILED"
                        <<":Filtering the ciminstance with a mentioned property"
                        <<" list did not return properties as expected "<<endl;
                    PEGASUS_TEST_ASSERT(false);
                }
            }
        }
    }
    catch(Exception& e)
    {
        cerr << "enumerateInstanceNames() failed." << endl;
            _errorExit(e.getMessage());
    }
}

// =========================================================================
//    Main
// =========================================================================

int main(int argc, char** argv)
{
    verbose = (getenv("PEGASUS_TEST_VERBOSE")) ? true : false;

    //
    // Check for command line option
    //
    if (argc != 2)
    {
        cerr << "Usage: TestCMPIAssociation {namespace}" << endl;
        return(1);
    }

    providerNamespace = CIMNamespaceName (argv[1]);

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

    // Get the instance object paths for the Person and Vehicle class
    //
    Array<CIMObjectPath> personRefs;
    Array<CIMObjectPath> vehicleRefs;
    try
    {
        personRefs =
            client.enumerateInstanceNames(providerNamespace, CMPI_TEST_PERSON);
        vehicleRefs =
            client.enumerateInstanceNames(providerNamespace, CMPI_TEST_VEHICLE);
    }
    catch (Exception& e)
    {
        cerr << "enumerateInstanceNames() failed." << endl;
        _errorExit(e.getMessage());
    }

    for(Uint32 i=0;i<personRefs.size();i++)
    {
        cout<<"PersonRefs values : "<<personRefs[i].toString()<<endl;
    }
    for(Uint32 i=0;i<vehicleRefs.size();i++)
    {
        cout<<"VehicleRefs values : "<<vehicleRefs[i].toString()<<endl;
    }
    Uint32 numPersonInstances = personRefs.size();
    Uint32 numVehicleInstances = vehicleRefs.size();

    cout << "Number of PersonInstances: " << numPersonInstances << endl;
    cout << "Number of VehicleInstances: " << numVehicleInstances << endl;
    


    // =======================================================================
    // Test associators
    //
    // Get the CIM instances that are associated with the specified source CIM
    // instance via an instance of a specified association class.
    // =======================================================================

    if (verbose)
    {
        cout << "+++++ Test associators" << endl;
    }

    for (Uint32 i = 0; i < numPersonInstances; i++)
    {
        _testAssociators(
            client,
            CMPI_TEST_RACING,
            personRefs[i],
            resultArray_asso_P1[i]);
    }

    for (Uint32 i = 0; i < numVehicleInstances; i++)
    {
       _testAssociators(client, CMPI_TEST_RACING, vehicleRefs[i],
                        resultArray_asso_V1[i]);
    }

    // =======================================================================
    // Test associatorNames
    //
    // Get the names of the CIM instances that are associated with the specified
    // source CIM instance via an instance of a association class.
    // =======================================================================

    if (verbose)
    {
        cout << "+++++ Test associatorNames" << endl;
    }

    for (Uint32 i = 0; i < numPersonInstances; i++)
    {
        _testAssociatorNames(client, CMPI_TEST_RACING, personRefs[i],
            resultArray_asso_P1[i]);
    }
    for (Uint32 i = 0; i < numVehicleInstances; i++)
    {
        _testAssociatorNames(client, CMPI_TEST_RACING, vehicleRefs[i],
            resultArray_asso_V1[i]);
    }

    // =======================================================================
    // Test references
    //
    // Get the association instances that refer to the specified target CIM
    // instance.
    // =======================================================================

    if (verbose)
    {
        cout << "+++++ Test references" << endl;
    }

    for (Uint32 i = 0; i < numPersonInstances; i++)
    {
        _testReferences(client, personRefs[i], resultArray_ref_P[i]);
    }
    for (Uint32 i = 0; i < numVehicleInstances; i++)
    {
        _testReferences(client, vehicleRefs[i], resultArray_ref_V[i]);
    }
    
    // =======================================================================
    // Test property filter for associators and references
    // =======================================================================
    _testCMPIFilterOfAssociation(client);

    // =======================================================================
    // Test referenceNames
    //
    // Get the names of the association instances that refer to the specified
    // source CIM instance.
    // =======================================================================

    if (verbose)
    {
        cout << "+++++ Test referenceNames" << endl;
    }

    for (Uint32 i = 0; i < numPersonInstances; i++)
    {
        _testReferenceNames(client, personRefs[i], resultArray_ref_P[i]);
    }
    for (Uint32 i = 0; i < numVehicleInstances; i++)
    {
        _testReferenceNames(client, vehicleRefs[i], resultArray_ref_V[i]);
    }

    // =======================================================================
    // Test passing Class object path to the Association Methods
    // =======================================================================

    if (verbose)
    {
        cout << "+++++ Test association class operations" << endl;
    }

    _testCMPIAssociationClassOperations(client, CMPI_TEST_PERSON);
    _testCMPIAssociationClassOperations(client, CMPI_TEST_VEHICLE);

    // =======================================================================
    // Association tests completed
    // =======================================================================

    cout << argv[0] << " +++++ passed all tests" << endl;
    return(0);
}
