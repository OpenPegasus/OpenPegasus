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
#include <Pegasus/Common/System.h>
#include <Pegasus/Common/FileSystem.h>
#include <Pegasus/Common/InternalException.h>
#include <Pegasus/Client/CIMClient.h>
#include <Pegasus/Common/XmlWriter.h>

#include <iostream>

PEGASUS_USING_PEGASUS;
PEGASUS_USING_STD;

const CIMNamespaceName SOURCE_NAMESPACE = CIMNamespaceName(
                                              "root/SampleProvider");
const CIMName SAMPLE_CLASSNAME  = CIMName("CWS_PlainFile");

Array<CIMObjectPath> objectNames;
CIMInstance instance;
CIMValue value;

static Boolean verbose;

void _enumerateInstanceNames(CIMClient & client)
{
    try
    {
        objectNames = client.enumerateInstanceNames(
                                 SOURCE_NAMESPACE,
                                 SAMPLE_CLASSNAME);
        if (verbose)
        {
            PEGASUS_STD (cout) << "+++++ enumerateInstanceNames "
                                      "completed successfully"
                               << PEGASUS_STD (endl);
        }
    }
    catch (Exception & e)
    {
        PEGASUS_STD (cerr) << "enumerateInstanceNames failed: "
                           << e.getMessage() << PEGASUS_STD (endl);
        exit (-1);
    }
}

void _getInstance(CIMClient & client)
{
    try
    {
        instance = client.getInstance(SOURCE_NAMESPACE, objectNames[0]);
        if (verbose)
        {
            XmlWriter::printInstanceElement(instance, PEGASUS_STD(cout));
            PEGASUS_STD (cout) << "+++++ getInstance completed successfully"
                               << PEGASUS_STD (endl);
        }
    }
    catch (Exception & e)
    {
        PEGASUS_STD (cerr) << "getInstance failed: " << e.getMessage ()
                           << PEGASUS_STD (endl);
        exit (-1);
    }
}

void _getProperty(CIMClient & client)
{
    try
    {
        value = client.getProperty(
                    SOURCE_NAMESPACE,
                    objectNames[0],
                    CIMName("FileSize"));
        if (verbose)
        {
            PEGASUS_STD (cout) << "+++++ getProperty completed successfully, "
                               << "FileSize is: " << value.toString()
                               << PEGASUS_STD (endl);
        }
    }
    catch (Exception & e)
    {
        PEGASUS_STD (cerr) << "getProperty failed: " << e.getMessage ()
                           << PEGASUS_STD (endl);
        exit (-1);
    }
}

void _setProperty(CIMClient & client, Uint64 newUint64Value)
{
    try
    {
        CIMValue newValue(newUint64Value);
        client.setProperty(
            SOURCE_NAMESPACE,
            objectNames[0],
            CIMName("FileSize"),
            newValue);
        if (verbose)
        {
            PEGASUS_STD (cout) << "+++++ setProperty completed successfully, "
                               << "FileSize has been set to: "
                               << newValue.toString() << PEGASUS_STD (endl);
        }
    }
    catch (Exception & e)
    {
        PEGASUS_STD (cerr) << "setProperty failed: " << e.getMessage ()
                           << PEGASUS_STD (endl);
        exit (-1);
    }
}

int main ()
{
    String stringVal;
    verbose = (getenv("PEGASUS_TEST_VERBOSE")) ? true : false;

    CIMClient client;
    try
    {
        client.connectLocal ();
    }
    catch (Exception & e)
    {
        PEGASUS_STD (cerr) << e.getMessage () << PEGASUS_STD (endl);
        return -1;
    }

    _enumerateInstanceNames(client);
    _getInstance(client);
    _setProperty(client, 7890);
    _getProperty(client);
    // getProperty() only returns CIMValues of type String.
    value.get(stringVal);
    PEGASUS_TEST_ASSERT(atoi((const char*)stringVal.getCString())==7890);
    _setProperty(client,1234);
    _getProperty(client);
    // getProperty() only returns CIMValues of type String.
    // Verify that setProperty worked as expected.
    value.get(stringVal);
    PEGASUS_TEST_ASSERT(atoi((const char*)stringVal.getCString())==1234);

    return 0;
}
