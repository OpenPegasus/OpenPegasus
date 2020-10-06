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
#include <Pegasus/Common/AcceptLanguageList.h>
#include <Pegasus/Common/ContentLanguageList.h>
#include <Pegasus/Client/CIMClient.h>

PEGASUS_USING_PEGASUS;
PEGASUS_USING_STD;


CIMNamespaceName providerNamespace;
const CIMName CLASSNAME = CIMName ("TestCMPI_Broker");

Boolean verbose;
Boolean useDefaultMsg;


void _checkStringValue (CIMValue & theValue,
    const String & value,
    Boolean null = false)
{
    PEGASUS_TEST_ASSERT (theValue.getType () == CIMTYPE_STRING);
    PEGASUS_TEST_ASSERT (!theValue.isArray ());
    if (null)
    {
        PEGASUS_TEST_ASSERT (theValue.isNull ());
    }
    else
    {
        PEGASUS_TEST_ASSERT (!theValue.isNull ());
        String result;
        theValue.get (result);

        if (verbose)
        {
            if (result != value)
            {
                cerr << "Property value comparison failed.  ";
                cerr << "Expected " << value << "; ";
                cerr << "Actual property value was " << result << "." << endl;
            }
        }

        PEGASUS_TEST_ASSERT (result == value);
    }
}


void _checkUint32Value (CIMValue & theValue, Uint32 value)
{
    PEGASUS_TEST_ASSERT (theValue.getType () == CIMTYPE_UINT32);
    PEGASUS_TEST_ASSERT (!theValue.isArray ());
    PEGASUS_TEST_ASSERT (!theValue.isNull ());

    Uint32 result;
    theValue.get (result);

    if (verbose)
    {
        if (result != value)
        {
            cerr << "Property value comparison failed.  ";
            cerr << "Expected " << value << "; ";
            cerr << "Actual property value was " << result << "." << endl;
        }
    }

    PEGASUS_TEST_ASSERT (result == value);
}

void _usage ()
{
    cerr << "Usage: TestCMPIBroker {test} {namespace}" << endl;
}

void test1 (CIMClient &client)
{
    CIMObjectPath instanceName;
    CIMValue output;
    instanceName.setNameSpace (providerNamespace);
    instanceName.setClassName (CLASSNAME);

    Array < CIMParamValue > inParams;
    Array < CIMParamValue > outParams;

    CIMValue retValue = client.invokeMethod (providerNamespace,
        instanceName,
        "testBrokerServices",
        inParams,
        outParams);

    _checkUint32Value (retValue, 1);
}

void _test (CIMClient & client)
{
    try
    {
        test1 (client);
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
    useDefaultMsg = (getenv("PEGASUS_USE_DEFAULT_MESSAGES")) ? true : false;
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
    cout << argv[0] << " +++++ completed" << endl;
    return 0;
}
