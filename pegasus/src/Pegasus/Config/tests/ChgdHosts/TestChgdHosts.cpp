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
#include <Pegasus/Common/PegasusAssert.h>
#include <Pegasus/Common/Constants.h>
#include <Pegasus/Client/CIMClient.h>

#include <iostream>

PEGASUS_USING_PEGASUS;

PEGASUS_USING_STD;

static Boolean verbose;


/* SystemName key property should be set to fully qualified hostname
   which is set on cimserver at startup to "hugo.bert" before this test case
   is executed
 */
void testFilterSystemName(
    CIMClient & client)
{
    CIMInstance filter("CIM_IndicationFilter");
    String query("SELECT * FROM CIM_ProcessIndication");

    filter.addProperty(
        CIMProperty(CIMName("Name"),String("Filter01")));
    filter.addProperty(
        CIMProperty(CIMName("SourceNamespace"),String("root/SampleProvider")));

    filter.addProperty(
        CIMProperty(CIMName("Query"), query));
    filter.addProperty(
        CIMProperty(CIMName("QueryLanguage"),String("WQL")));

    CIMObjectPath path =
        client.createInstance(PEGASUS_NAMESPACENAME_INTEROP, filter);
    
    client.deleteInstance(PEGASUS_NAMESPACENAME_INTEROP, path);

    // SystemName is fourth (=last) keybinding
    Array<CIMKeyBinding> keys = path.getKeyBindings();
    CIMKeyBinding sysName=keys[3];

    if (verbose)
    {
        cout << "SystemName returned is: " << sysName.getValue() << endl;
    }
    
    Boolean result = String("hugo.bert") == sysName.getValue();

    if(!result)
    {
       cout << "Test failed, Make sure that cimserver is started " \
               "with \"hostname=hugo fullyQualifiedHostName=hugo.bert\"" \
               << endl;
    }

    PEGASUS_TEST_ASSERT(result);
}

/* SystemName property in ObjectManager instance should be set to 
   fully qualified hostname which is set on cimserver at startup to "hugo.bert" 
   before this test case is executed
   
   class resides in PEGASUS_NAMESPACENAME_INTEROP
 */
void testObjectManagerSystemName(
    CIMClient & client)
{
    Array<CIMInstance> instances=client.enumerateInstances(
        PEGASUS_NAMESPACENAME_INTEROP,
        CIMName("CIM_ObjectManager"));

    PEGASUS_TEST_ASSERT(instances.size() == 1);

    Uint32 pos = instances[0].findProperty("SystemName");
    PEGASUS_TEST_ASSERT(PEG_NOT_FOUND != pos);

    CIMProperty p = instances[0].getProperty(pos);
    CIMValue v = p.getValue();
    String s;
    v.get(s);

    PEGASUS_TEST_ASSERT(String::equal(s,"hugo.bert"));
}

/* Associations are returned with hostname in object path by OP CIM Server
   Check that hostname is hugo as hostname config property was set to that
   value at CIM Server startup for this test case
 */
#ifdef PEGASUS_ENABLE_CMPI_PROVIDER_MANAGER

void testAssociationsHostNameCMPI(
    CIMClient & client)
{
    Array<CIMObjectPath> objPaths = client.enumerateInstanceNames(
        CIMNamespaceName("test/TestProvider"),
        CIMName("cmpiperf_testclassa"));

    PEGASUS_TEST_ASSERT(objPaths.size() == 1);

    Array<CIMObject> objects = client.associators(
        CIMNamespaceName("test/TestProvider"),
        objPaths[0]);

    PEGASUS_TEST_ASSERT(objects.size() > 0);

    CIMObjectPath path = objects[0].getPath();    
    String pathHN = path.getHost();

    PEGASUS_TEST_ASSERT(String::equal(pathHN,"hugo"));
}

#endif //PEGASUS_ENABLE_CMPI_PROVIDER_MANAGER

void testAssociationsHostName(
    CIMClient & client)
{
    Array<CIMObjectPath> objPaths = client.enumerateInstanceNames(
        CIMNamespaceName("test/TestProvider"),
        CIMName("TST_Person"));

    PEGASUS_TEST_ASSERT(objPaths.size() > 0);

    Array<CIMObject> objects = client.associators(
        CIMNamespaceName("test/TestProvider"),
        objPaths[0]);

    PEGASUS_TEST_ASSERT(objects.size() > 0);

    CIMObjectPath path = objects[0].getPath();    
    String pathHN = path.getHost();

    PEGASUS_TEST_ASSERT(String::equal(pathHN,"hugo"));
}

int main(int, char** argv)
{
    verbose = getenv("PEGASUS_TEST_VERBOSE") ? true : false;

    CIMClient client;
    try
    {
        client.setTimeout(120000);
        client.connectLocal();

        testFilterSystemName(client);
#ifdef PEGASUS_ENABLE_INTEROP_PROVIDER
        testObjectManagerSystemName(client);
#endif 


#ifdef PEGASUS_ENABLE_CMPI_PROVIDER_MANAGER
        testAssociationsHostNameCMPI(client);
#endif 
        testAssociationsHostName(client);

        client.disconnect();
    }
    catch(Exception& e)
    {
        cerr << "Exception: " << e.getMessage() << endl;

        return(1);
    }

    cout << argv[0] << " +++++ passed all tests" << endl;

    return(0);
}
