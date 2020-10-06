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


/*
    This program executes a number of FQL queries against the server
    and analyzes the results.
*/

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/PegasusAssert.h>
#include <Pegasus/Client/CIMClient.h>
#include <Pegasus/Common/StringConversion.h>
#include <stdio.h>
#include <stdlib.h>
#include <cstdarg>
#include <string.h>

PEGASUS_USING_PEGASUS;
PEGASUS_USING_STD;

#define VCOUT if (verbose) cout
Boolean verbose = false;

#define TRACE cout << "line=" << __LINE__ << endl;
//#define PROVIDERNS test/TestProvider
//#define TEST_CLASS_DEFINE Test_CLITestProviderClass

String namespace_opt = "";
String host_opt = "";
String user_opt = "";
String password_opt = "";

const CIMName TEST_CLASS("Test_CLITestProviderClass");
const CIMNamespaceName PROVIDERNS = CIMNamespaceName("test/TestProvider");
/*
    Execute a defined query and return the instances found
*/
void testEnumerate(
    CIMClient& client,
    const String& query,
    Uint32 testCount,
    CIMStatusCode expectedError = CIM_ERR_SUCCESS)
{
    Array<CIMInstance> pulledInstances;
    Boolean endOfSequence = false;
    CIMEnumerationContext enumerationContext;

    try
    {
        pulledInstances = client.openEnumerateInstances(
            enumerationContext,
            endOfSequence,
            PROVIDERNS,
            TEST_CLASS,
            true,
            false,
            CIMPropertyList(),
            "DMTF:FQL",
            query,
            30,
            false,
            10);

        if (expectedError != CIM_ERR_SUCCESS)
        {
            cout << "ERROR. Error code expected but good response receied."
                << endl;
        }
    }
    catch (CIMException& e)
    {
        if (e.getCode() == expectedError)
        {
            VCOUT << "Error caught correctly. " << e.getCode() << endl;
            return;
        }
        else
        {
            cout << "Error Rtn expected but Incorrect code received"
                 << " Query " << query << endl;
            exit(1);
        }
    }
    while (!endOfSequence)
    {
        Array<CIMInstance> cimInstancesPulled = client.pullInstancesWithPath(
        enumerationContext,
        endOfSequence,
        10);
        pulledInstances.appendArray(cimInstancesPulled);
    }
    if (testCount != pulledInstances.size())
    {
        cout << "Incorrect Count returned. Expected " << testCount
             << " Received " << pulledInstances.size()
             << " Query " << query
             << endl;
        exit(1);
    }
    else
    {
        return;
    }
}


void testAssociators(
    CIMClient& client,
    const String& query,
    CIMObjectPath targetPath,
    Uint32 testCount,
    CIMStatusCode expectedError = CIM_ERR_SUCCESS)
{
    Array<CIMInstance> pulledInstances;
    Boolean endOfSequence = false;
    CIMEnumerationContext enumerationContext;
    //CIMObjectPath assoc = CIMObjectPath("TEST_CLASS.Id=\"Mike"\")
    try
    {
        pulledInstances = client.openAssociatorInstances(
            enumerationContext,
            endOfSequence,
            PROVIDERNS,
            targetPath,
            CIMName(),
            CIMName(),
            String(),
            String(),
            false,
            CIMPropertyList(),
            "DMTF:FQL",
            query,
            30,
            false,
            10);

        if (expectedError != CIM_ERR_SUCCESS)
        {
            cout << "ERROR. Error code expected but good response receied."
                << endl;
        }
    }
    catch (CIMException& e)
    {
        if (e.getCode() == expectedError)
        {
            VCOUT << "Error caught correctly. " << e.getCode() << endl;
            return;
        }
        else
        {
            cout << "Error Rtn expected but Incorrect code received"
                 << " Query " << query << " Expected " << expectedError
                 << " received " << e.getCode()
                 << " msg " << e.getMessage() << endl;
            exit(1);
        }
    }
    while (!endOfSequence)
    {
        Array<CIMInstance> cimInstancesPulled = client.pullInstancesWithPath(
        enumerationContext,
        endOfSequence,
        10);
        pulledInstances.appendArray(cimInstancesPulled);
    }

    VCOUT << "pullAssociators " << targetPath.toString()
          << " returned " << pulledInstances.size() << " instances."
          << endl;
    if (testCount != pulledInstances.size())
    {
        cout << "Incorrect Count returned. Expected " << testCount
             << " Received " << pulledInstances.size()
             << " Query " << query
             << endl;
        exit(1);
    }
    else
    {
        return;
    }
}


void testReferences(
    CIMClient& client,
    const String& query,
    CIMObjectPath targetPath,
    Uint32 testCount,
    CIMStatusCode expectedError = CIM_ERR_SUCCESS)
{
    Array<CIMInstance> pulledInstances;
    Boolean endOfSequence = false;
    CIMEnumerationContext enumerationContext;
    //CIMObjectPath assoc = CIMObjectPath("TEST_CLASS.Id=\"Mike"\")
    try
    {
        pulledInstances = client.openReferenceInstances(
            enumerationContext,
            endOfSequence,
            PROVIDERNS,
            targetPath,
            CIMName(),
            String(),
            false,
            CIMPropertyList(),
            "DMTF:FQL",
            query,
            30,
            false,
            10);

        if (expectedError != CIM_ERR_SUCCESS)
        {
            cout << "ERROR. Error code expected but good response receied."
                << endl;
        }
    }
    catch (CIMException& e)
    {
        if (e.getCode() == expectedError)
        {
            VCOUT << "Error caught correctly. " << e.getCode() << endl;
            return;
        }
        else
        {
            cout << "Error Rtn expected but Incorrect code received"
                 << " Query " << query << " Expected " << expectedError
                 << " received " << e.getCode()
                 << " msg " << e.getMessage() << endl;
            exit(1);
        }
    }
    while (!endOfSequence)
    {
        Array<CIMInstance> cimInstancesPulled = client.pullInstancesWithPath(
        enumerationContext,
        endOfSequence,
        10);
        pulledInstances.appendArray(cimInstancesPulled);
    }

    VCOUT << "pullAssociators " << targetPath.toString()
          << " returned " << pulledInstances.size() << " instances."
          << endl;
    if (testCount != pulledInstances.size())
    {
        cout << "Incorrect Count returned. Expected " << testCount
             << " Received " << pulledInstances.size()
             << " Query " << query
             << endl;
        exit(1);
    }
    else
    {
        return;
    }
}

CIMObjectPath createInstance(CIMClient& client, CIMInstance& newInstance)
{
    CIMObjectPath path = client.createInstance(PROVIDERNS,
       newInstance);

    return path;
}

bool setPropertyValue(CIMInstance& instance, const CIMName& propertyName,
    const CIMValue & value)
{
    unsigned int pos = instance.findProperty(propertyName);
    if(pos != PEG_NOT_FOUND)
    {
        instance.getProperty(pos).setValue(value);
        return true;
    }
    return false;
}
/*
    Build an instance of the test class
*/
CIMInstance buildInstance(CIMClient& client, String& instanceId)
{
    CIMClass cl = client.getClass(PROVIDERNS, TEST_CLASS);
    CIMInstance inst = cl.buildInstance(false, false, CIMPropertyList());
    setPropertyValue(inst, "Id", CIMValue(instanceId));

    return inst;
}

void deleteInstance(CIMClient& client, CIMObjectPath& path)
{
    client.deleteInstance(PROVIDERNS, path);
}

// Parse Hostname input into name and port number
Boolean parseHostName(const String arg, String& hostName, Uint32& port)
{
    port = 5988;
    hostName = arg;

    Uint32 pos;
    if (!((pos = arg.reverseFind(':')) == PEG_NOT_FOUND))
    {
        Uint64 temp;
        if (StringConversion::decimalStringToUint64(
            hostName.subString(pos+1).getCString(), temp)
            &&
            StringConversion::checkUintBounds(temp,CIMTYPE_UINT32))
        {
            hostName.remove(pos);
            port = (Uint32)temp;
        }
        else
        {
            return false;
        }
    }
    return true;
}

bool connectToHost(CIMClient& client, String host_opt = "")
{
    try
    {
        if (host_opt == "")
        {
            VCOUT << "connectLocal" << endl;
            client.connectLocal();
        }
        else
        {
            String hostName;
            Uint32 port;
            if (parseHostName(host_opt, hostName, port))
            {
                VCOUT << " Connect to host "
                       << hostName << ":" << port << endl;
                client.connect(hostName, port, user_opt, password_opt);
            }
            else
            {
                cerr << "Host name parameter error " << hostName << endl;
                PEGASUS_TEST_ASSERT(false);
            }
        }
    }

    catch (CIMException& e)
    {
        cerr << "CIMException Error: in connect " << e.getMessage() << endl;
        PEGASUS_TEST_ASSERT(false);
    }
    catch (Exception& e)
    {
        cerr << "Error: in connect " << e.getMessage() << endl;
        PEGASUS_TEST_ASSERT(false);
    }
    return true;
}

int main(int , char** argv)
{
    verbose = getenv("PEGASUS_TEST_VERBOSE");

    const char* arg0 = argv[0];


    CIMClient client;
    if (!connectToHost(client))
    {
        cerr << "Connection to host failed. Terminating" << endl;
        exit(1);
    }

    Array<CIMObjectPath> paths;

    //
    //  Create instances that will have specified test properties in them
    //  that we can use for queries.  We used the class
    //  Test_CLITestProviderClass because it has a broad set of testable
    //  properties.
    //

    String id1 = "PropertyOpTest1";
    CIMInstance inst1 = buildInstance(client, id1);

    {
        setPropertyValue(inst1,"scalBool", CIMValue(true));
        setPropertyValue(inst1,"scalUint8",CIMValue(Uint8(12)));
        setPropertyValue(inst1,"scalSint8",CIMValue(Sint8(12)));
        setPropertyValue(inst1,"scalUint16",CIMValue(Uint16(500)));
        setPropertyValue(inst1,"scalSint16",CIMValue(Sint16(500)));

        setPropertyValue(inst1,"scalUint32",CIMValue(Uint32(9999)));
        setPropertyValue(inst1,"scalSint32",CIMValue(Sint32(9999)));

        setPropertyValue(inst1,"scalUint64",CIMValue(Uint64(9999)));
        setPropertyValue(inst1,"scalSint64",CIMValue(Sint64(9999)));

        setPropertyValue(inst1,"scalString",CIMValue(String("String1")));
        Array<Uint32> lArrayUint32;
        lArrayUint32.append(0);
        lArrayUint32.append(128);
        lArrayUint32.append(256);
        lArrayUint32.append(65536);
        lArrayUint32.append(4294967295);
        setPropertyValue(inst1,"arrayUint32",CIMValue(lArrayUint32));
    }

    CIMObjectPath p1 = createInstance(client, inst1);
    paths.append(p1);

    String id2 = "PropertyOpTest2";
    CIMInstance inst2 = buildInstance(client, id2);
    {
        setPropertyValue(inst2,"scalBool", CIMValue(false));
        setPropertyValue(inst2,"scalUint8",CIMValue(Uint8(20)));
        setPropertyValue(inst2,"scalSint8",CIMValue(Sint8(8)));
        setPropertyValue(inst2,"scalUint16",CIMValue(Uint16(1000)));
        setPropertyValue(inst2,"scalSint16",CIMValue(Sint16(500)));

        setPropertyValue(inst2,"scalUint32",CIMValue(Uint32(7698)));
        setPropertyValue(inst2,"scalSint32",CIMValue(Sint32(9999)));

        setPropertyValue(inst2,"scalUint64",CIMValue(Uint64(9999)));
        setPropertyValue(inst2,"scalSint64",CIMValue(Sint64(9999)));

        setPropertyValue(inst2,"scalString",CIMValue(String("String2")));
        Array<Uint32> lArrayUint32;
        lArrayUint32.append(0);
        lArrayUint32.append(128);
        lArrayUint32.append(256);
        lArrayUint32.append(165536);
        lArrayUint32.append(4294967295);
        lArrayUint32.append(876543);
        setPropertyValue(inst2,"arrayUint32",CIMValue(lArrayUint32));
    }

    CIMObjectPath p2 = createInstance(client, inst2);
    paths.append(p2);

    //
    //  Execute the query tests
    //

    testEnumerate(client, "scalBool = true", 2);
    testEnumerate(client,"scalUint32 = 7698", 1);
    testEnumerate(client, "scalString = 'String2'", 1);


    testEnumerate(client, "arrayUint32[5] = 876543", 1);
    testEnumerate(client, "ANY arrayUint32 = 876543", 1);
    testEnumerate(client, "scalString <> 'NoSuchString'", 3);
    testEnumerate(client, "scalString = 'String2'", 1);

    // the following are error tests

    testEnumerate(client, "scalUint32 = 'NotANumber'", 0, CIMStatusCode(15));
    testEnumerate(client, "scalUint32 = 123a", 0, CIMStatusCode(15));
    testEnumerate(client, "scalbool = 1234", 0, CIMStatusCode(15));

    // Test associator

    CIMObjectPath assoc =
        CIMObjectPath("Test_CLITestProviderClass.Id=\"Mike\"");
    testAssociators(client, "scalBool = true",assoc, 1);
    testAssociators(client, "scalBool = false",assoc, 0);

    // Cheap test because only possible comparison is cimobject path
    // and that means building correct object path for the query
    // KS_TODO create a valid CIMObjectPath and test it.
    testReferences(client, "scalBool = false", assoc,0);

    for (Uint32 i = 0; i < paths.size(); i++)
    {
        deleteInstance(client,paths[i]);
    }

cout << arg0 << " +++++ passed all tests.";

}
