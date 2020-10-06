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
#include <Pegasus/Common/Constants.h>
#include <Pegasus/Common/CIMType.h>
#include <Pegasus/Common/CIMName.h>
#include <Pegasus/Common/CIMInstance.h>
#include <Pegasus/Common/CIMProperty.h>
#include <Pegasus/Common/CIMValue.h>
#include <Pegasus/Common/System.h>
#include <Pegasus/Common/InternalException.h>
#include <Pegasus/Client/CIMClient.h>

PEGASUS_USING_PEGASUS;
PEGASUS_USING_STD;


CIMNamespaceName providerNamespace;
const CIMName CLASSNAME = CIMName ("TestCMPI_Fail_1");

const String CMPI_TESTFAIL_INST_ERROR = "CIM_ERR_FAILED: Error initializing"
    " CMPI MI TestCMPIFail_1Provider, the following MI factory function(s)"
    " returned an error: TestCMPIFail_1Provider_Create_InstanceMI";

const String CMPI_TESTFAIL_METH_ERROR = "CIM_ERR_FAILED: Error initializing"
    " CMPI MI TestCMPIFail_1Provider, the following MI factory function(s)"
    " returned an error: TestCMPIFail_1Provider_Create_MethodMI";

const String CMPI_TESTFAIL_ASSOC_ERROR = "CIM_ERR_FAILED: Error initializing"
    " CMPI MI TestCMPIFail_1Provider, the following MI factory function(s)"
    " returned an error: TestCMPIFail_1Provider_Create_AssociationMI";

const String RCMPI_ERROR = "CIM_ERR_FAILED: ProviderInitFailure:"
    " Error initializing the API's _Create<mi-type>MI";

Boolean verbose;

void _usage ()
{
     cerr << "Usage: TestCMPI_Fail_1 {test} {namespace}" << endl;
}

void test01 (CIMClient & client)
{
    try
    {
        Array<CIMObjectPath> array = client.enumerateInstanceNames (
            providerNamespace, CLASSNAME);
        // enumerateInstanceNames should throw exception.
        PEGASUS_TEST_ASSERT(0);
    }
    catch (const CIMException &e)
    {
        if (e.getMessage() != CMPI_TESTFAIL_INST_ERROR
                && e.getMessage() != RCMPI_ERROR)
        {
            throw e;
        }
    }
}

void test02 (CIMClient & client)
{
    try
    {
        CIMObjectPath instanceName(
            String::EMPTY,
            providerNamespace,
            CLASSNAME,
            Array<CIMKeyBinding>());

        Array<CIMParamValue> outParams;

        CIMValue value = client.invokeMethod(
             providerNamespace,
             instanceName,
             "foo",
             Array<CIMParamValue>(),
             outParams);
    }
    catch (const CIMException &e)
    {
        if (e.getMessage() != CMPI_TESTFAIL_METH_ERROR
                && e.getMessage() != RCMPI_ERROR)
        {
            throw e;
        }
    }
}

void test03 (CIMClient & client)
{
    try
    {
        CIMObjectPath instanceName(
            String::EMPTY,
            providerNamespace,
            CLASSNAME,
            Array<CIMKeyBinding>());

        Array<CIMObjectPath> result = client.associatorNames(
            providerNamespace,
            instanceName);
    }
    catch (const CIMException &e)
    {
        if (e.getMessage() != CMPI_TESTFAIL_ASSOC_ERROR
                && e.getMessage() != RCMPI_ERROR)
        {
            throw e;
        }
    }
}

void _test (CIMClient & client)
{
    try
    {
        test01 (client);
        test02 (client);
        test03 (client);
    }
    catch (Exception & e)
    {
        cerr << "test failed: " << e.getMessage () << endl;
        exit (-1);
    }
}


int main (int argc, char **argv)
{
    verbose = (getenv ("PEGASUS_TEST_VERBOSE")) ? true : false;
    CIMClient client;
    try
    {
        client.connectLocal ();
    }
    catch (Exception & e)
    {
        cerr << e.getMessage () << endl;
        return -1;
    }

    if (argc != 3)
    {
        _usage ();
        return 1;
    }
    else
    {
        const char *opt = argv[1];
        if (String::equalNoCase (opt, "test"))
        {
            providerNamespace = CIMNamespaceName (argv[2]);
            _test (client);
        }
        else
        {
            cerr << "Invalid option: " << opt << endl;
            _usage ();
            return -1;
        }
    }

    cout << argv[0] << " +++++ passed all tests" << endl;

    return 0;
}
