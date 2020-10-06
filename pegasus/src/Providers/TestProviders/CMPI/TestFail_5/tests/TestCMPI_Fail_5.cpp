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

#include <Pegasus/Client/CIMClient.h>
#include <string.h>
#include <Pegasus/Provider/CMPI/cmpidt.h>
#include <Pegasus/Provider/CMPI/cmpift.h>
#include <Pegasus/Provider/CMPI/cmpimacs.h>
#include <Pegasus/Common/XmlWriter.h>

PEGASUS_USING_STD;
PEGASUS_USING_PEGASUS;

CIMNamespaceName providerNamespace;
const CIMName CMPI_TEST_PERSON = CIMName ("CMPI_TEST_Person");
const CIMName CMPI_TEST_FAIL = CIMName ("TestCMPI_Fail_5");

const char *queries =  "SELECT * FROM TestCMPI_Fail_5";

Boolean verbose = false;

void _errorExit(String message)
{
    cerr << "TestCMPI_Fail_5 error: " << message << endl;
    exit(1);
}

// This method calls the CMPIProviderManager::handleAssociatorsRequest and
// CMPIProviderManager::handleAssociatorNamesRequest and
// 'if (rc.rc != CMPI_RC_OK)' condition in CMPIProviderManager.cpp succeeds.

void _testAssociators(
    CIMClient& client,
    CIMName assocClass,
    CIMObjectPath instancePath )
{
    try
    {
        CIMName resultClass;
        String role;
        String resultRole;

        // Get the CIM instances that are associated with the specified source
        // instance via an instance of the AssocClass

        Array<CIMObject> resultObjects =
            client.associators(
                providerNamespace,
                instancePath,
                assocClass,
                resultClass,
                role,
                resultRole);

        Array<CIMObjectPath> resultObjectPaths =
            client.associatorNames(
                providerNamespace,
                instancePath,
                assocClass,
                resultClass,
                role,
                resultRole);

    }
    catch (Exception& e)
    {
        _errorExit(e.getMessage());
    }
}

// This method calls CMPIProviderManager::handleReferencesRequest and
// CMPIProviderManager::handleReferenceNamesRequest and
// 'if( rc.rc != CMPI_RC_OK)' condition in CMPIProviderManager.cpp succeeds.

void _testReferences(CIMClient& client, CIMObjectPath instancePath)
{
    try
    {
        // get the association reference instances
        Array<CIMObject> resultObjects;
        Array<CIMObjectPath> resultObjectPaths;
        CIMName resultClass;
        String role;

        resultObjects = client.references(
            providerNamespace,
            instancePath,
            resultClass,
            role);
        resultObjectPaths = client.referenceNames(
            providerNamespace,
            instancePath,
            resultClass,
            role);
    }
    catch (const Exception& e)
    {
         _errorExit(e.getMessage());
    }
}

// This method calls the  CMPIProviderManager::handleExecQueryRequest
// and 'if (rc.rc != CMPI_RC_OK)' condition in CMPIProviderManager.cpp succeeds.

static void _testInstanceError(CIMClient &client)
{
#ifndef PEGASUS_DISABLE_EXECQUERY
    try
    {
        String wql ("WQL");

        Array < CIMObject > objects = client.execQuery(
            providerNamespace,
            wql,
            queries);
    }
    catch (const Exception & e)
    {
        _errorExit(e.getMessage());
    }
#endif
}
// This method calls CMPIProviderManager::handleInvokeMethodRequest and
// 'if(rc.rc != CMPI_RC_OK)' condition  in CMPIProviderManager.cpp succeeds.
void _testMethodError(CIMClient & client)
{
    CIMObjectPath instanceName;
    instanceName.setNameSpace (providerNamespace);
    instanceName.setClassName (CMPI_TEST_FAIL);

    Array < CIMParamValue > inParams;
    Array < CIMParamValue > outParams;
    Boolean caughtException;

    caughtException = false;
    try
    {
        CIMValue retValue = client.invokeMethod(
             providerNamespace,
             instanceName,
             "test",
             inParams,
             outParams);
    }
    catch (const CIMException &e)
    {
        if (e.getCode() == CIM_ERR_NOT_SUPPORTED)
        {
            caughtException = true;
        }
    }
    PEGASUS_TEST_ASSERT (caughtException);
}


int main(int, char** argv)
{
    providerNamespace = CIMNamespaceName ("test/TestProvider");
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

    // Get the instance object paths for the Person and Vehicle class
    Array<CIMObjectPath> personRefs;
    try
    {
        personRefs =
            client.enumerateInstanceNames(providerNamespace, CMPI_TEST_PERSON);
    }
    catch (Exception& e)
    {
        cerr << "enumerateInstanceNames() failed." << endl;
        _errorExit(e.getMessage());
    }
    Uint32 numPersonInstances = personRefs.size();

    // Test associators
    for (Uint32 i = 0; i < numPersonInstances; i++)
    {
        _testAssociators(client, CMPI_TEST_FAIL, personRefs[i]);
    }

    // Test references
    for (Uint32 i = 0; i < numPersonInstances; i++)
    {
        _testReferences(client, personRefs[i]);
    }

    _testInstanceError(client);
    _testMethodError(client);

    cout << argv[0] << " +++++ passed all tests" << endl;
    return(0);
}
