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
#include <Pegasus/Common/XmlWriter.h>

PEGASUS_USING_PEGASUS;
PEGASUS_USING_STD;

static Boolean verbose;

static const String NAMESPACE("test/TestProvider");
static const String Z_CLASSNAME("TST_InstanceZ");
static const String A_CLASSNAME("TST_InstanceA");
static const String B_CLASSNAME("TST_InstanceB");

// get repository instances (TST_InstanceZ)
void Test1(CIMClient& client)
{
    if (verbose)
    {
        cout << "Test1()" << endl;
    }

    Array<CIMInstance> zInstances =
        client.enumerateInstances(
            NAMESPACE,
            Z_CLASSNAME);

    for (Uint32 i = 0, n = zInstances.size(); i < n; i++)
    {
        if (verbose)
        {
            cout << zInstances[i].getPath().toString() << endl;
        }
    }
}

// compare repository instances (TST_InstanceZ) with TestProviderA instances
// (TST_InstanceA)
void Test2(CIMClient& client)
{
    if (verbose)
    {
        cout << "Test2()" << endl;
    }

    Array<CIMInstance> zInstances =
        client.enumerateInstances(
            NAMESPACE,
            Z_CLASSNAME);

    for (Uint32 i = 0, n = zInstances.size(); i < n; i++)
    {
        if (verbose)
        {
            cout << zInstances[i].getPath().toString() << endl;
        }
    }

    Array<CIMInstance> aInstances =
        client.enumerateInstances(
            NAMESPACE,
            A_CLASSNAME);

    for (Uint32 i = 0, n = aInstances.size(); i < n; i++)
    {
        if (verbose)
        {
            cout << aInstances[i].getPath().toString() << endl;
        }

        bool match = false;

        String aName = aInstances[i].getProperty(
            aInstances[i].findProperty("Name")).getValue().toString();
        CIMObjectPath aS = aInstances[i].getProperty(
            aInstances[i].findProperty("s")).getValue().toString();

        if (verbose)
        {
            cout << "aName = " << aName << endl;
            cout << "aS = " << aS.toString() << endl;
        }

        // search for a match (instances sets are not ordered).
        for (Uint32 j = 0, m = zInstances.size(); j < m; j++)
        {
            String zName = zInstances[i].getProperty(
                zInstances[i].findProperty("Name")).getValue().toString();
            CIMObjectPath zS = zInstances[i].getPath();

            if (verbose)
            {
                cout << "zName = " << zName << endl;
                cout << "zS = " << zS.toString() << endl;
            }

            if (String::equalNoCase(aName, zName) && (aS == zS))
            {
                match = true;
                break;
            }
        }

        if (!match)
        {
            throw Exception("could not find match");
        }
    }
}

// compare TestProviderA instances (TST_InstanceA) with TestProviderB
// instances (TST_InstanceB)
void Test3(CIMClient& client)
{
    if (verbose)
    {
        cout << "Test3()" << endl;
    }

    Array<CIMInstance> aInstances =
        client.enumerateInstances(
            NAMESPACE,
            A_CLASSNAME);

    for (Uint32 i = 0, n = aInstances.size(); i < n; i++)
    {
        if (verbose)
        {
            cout << aInstances[i].getPath().toString() << endl;
        }
    }

    Array<CIMInstance> bInstances =
        client.enumerateInstances(
            NAMESPACE,
            B_CLASSNAME);

    for (Uint32 i = 0, n = bInstances.size(); i < n; i++)
    {
        if (verbose)
        {
            cout << bInstances[i].getPath().toString() << endl;
        }

        bool match = false;

        String bName = bInstances[i].getProperty(
            bInstances[i].findProperty("Name")).getValue().toString();
        CIMObjectPath bS = bInstances[i].getProperty(
            bInstances[i].findProperty("s")).getValue().toString();

        if (verbose)
        {
            cout << "bName = " << bName << endl;
            cout << "bS = " << bS.toString() << endl;
        }

        // search for a match (instances sets are not ordered).
        for (Uint32 j = 0, m = aInstances.size(); j < m; j++)
        {
            String aName = aInstances[i].getProperty(
                aInstances[i].findProperty("Name")).getValue().toString();
            CIMObjectPath aS = aInstances[i].getPath();

            if (verbose)
            {
                cout << "aName = " << aName << endl;
                cout << "aS = " << aS.toString() << endl;
            }

            if (String::equalNoCase(bName, aName) && (bS == aS))
            {
                match = true;
                break;
            }
        }

        if (!match)
        {
            throw Exception("could not find match");
        }
    }
}

// Call the TestOperationsProvider to test each of the CIMOMHandle operations.
void TestOperations(CIMClient& client)
{
    Array<CIMParamValue> in;
    Array<CIMParamValue> out;
    CIMValue returnValue = client.invokeMethod(
        CIMNamespaceName("test/TestProvider"),
        CIMObjectPath("TST_OperationsDriver"),
        CIMName("testCIMOMHandle"),
        in,
        out);
    Uint32 rc;
    returnValue.get(rc);
    PEGASUS_TEST_ASSERT(rc == 0);
}

int main(int argc, char** argv)
{
    verbose = getenv("PEGASUS_TEST_VERBOSE") ? true : false;

    CIMClient client;

    try
    {
        client.connectLocal();

        if (argc == 1)
        {
            Test1(client);
            Test2(client);
            Test3(client);
            TestOperations(client);
        }
        else if ((argc == 2) && (strcmp(argv[1], "clean") == 0))
        {
            try
            {
                client.deleteClass(
                    CIMNamespaceName("test/TestProvider"),
                    CIMName("TST_OperationsTemp"));
            }
            catch (const CIMException&)
            {
                // Ignore exception; the class must not have existed
            }

            try
            {
                client.deleteInstance(
                    CIMNamespaceName("test/TestProvider"),
                    CIMObjectPath("TST_Operations1.key=3"));
            }
            catch (const CIMException&)
            {
                // Ignore exception; the instance must not have existed
            }

            return 0;
        }
        else
        {
            cerr << "Usage:  " << argv[0] << endl;
            cerr << "        " << argv[0] << " clean" << endl;
            return 1;
        }
    }
    catch (const CIMException& e)
    {
        client.disconnect();

        cout << "CIMException: " << e.getCode() << " " << e.getMessage() <<
            endl;

        return 1;
    }
    catch (const Exception& e)
    {
        client.disconnect();

        cout << "Exception: " << e.getMessage() << endl;

        return 1;
    }
    catch (...)
    {
        client.disconnect();

        cout << "unknown exception" << endl;

        return 1;
    }

    client.disconnect();

    cout << argv[0] << " +++++ passed all tests" << endl;

    return 0;
}
