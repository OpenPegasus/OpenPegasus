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
//%///////////////////////////////////////////////////////////////////////


#include <Pegasus/Client/CIMClient.h>

#define QUERY1 "Select * from TST_Person"

#define QUERY2 "Select * from TST_Person where name = 'Mike'"
#define NAME2  "Mike"

#define QUERY3 "select * from TST_Person where Name = 'Saara' and \
                extraProperty = 'default'"
#define NAME3  "Saara"
#define PROP3  "default"

#define QUERY4 "select * from TST_Person where Name = 'Mike' or \
                name = 'Father'"
#define NAME4a "Mike"
#define NAME4b "Father"

PEGASUS_USING_PEGASUS;
PEGASUS_USING_STD;

CIMNamespaceName NAMESPACE("test/TestProvider");

void testQuery1(CIMClient& client)
{
    unsigned int matchedCount = 0;
 
    Array<CIMObject> arr=
        client.execQuery(NAMESPACE,String("DMTF:CQL"),String(QUERY1));
 
    Array<CIMInstance> instances = 
        client.enumerateInstances(NAMESPACE, CIMName("TST_Person"));
 
    if (arr.size() != instances.size())
    {
        throw Exception("Number of instances returned do not match.");
    }
 
    for (unsigned int i=0;i<arr.size();i++)
    {
        int idx = arr[i].findProperty(CIMName("name"));
        CIMValue cv1 =  arr[i].getProperty(idx).getValue();
        for (unsigned int j=0;j<instances.size();j++)
        {
            idx = instances[j].findProperty(CIMName("name"));
            CIMValue cv2 =  instances[j].getProperty(idx).getValue();
 
            if (cv1.equal(cv2))
            {
                matchedCount++;
                instances.remove(j);
            }
        }
    }
 
    if(matchedCount != arr.size())
    {
        throw Exception("The Property values do not match.");
    }
}


void testQuery2(CIMClient& client)
{
    unsigned int matchedCount = 0;
    Array<CIMObject> arr=
        client.execQuery(NAMESPACE,String("DMTF:CQL"),String(QUERY2));
 
    Array<CIMInstance> instancesAll =
        client.enumerateInstances(NAMESPACE, CIMName("TST_Person"));
 
    Array<CIMInstance> instances;
 
    for (unsigned int i=0;i<instancesAll.size();i++)
    {
        int idx = instancesAll[i].findProperty(CIMName("name"));
        CIMValue cv1 =  instancesAll[i].getProperty(idx).getValue();
        if(String::equal(cv1.toString(), String(NAME2)))
        {
            instances.append(instancesAll[i]);
        }
    }
 
    if (arr.size() != instances.size())
    {
        throw Exception("Number of instances returned do not match.");
    }
 
    for (unsigned int i=0;i<arr.size();i++)
    {
        int idx = arr[i].findProperty(CIMName("name"));
        CIMValue cv1 =  arr[i].getProperty(idx).getValue();
        for (unsigned int j=0;j<instances.size();j++)
        {
            idx = instances[j].findProperty(CIMName("name"));
            CIMValue cv2 =  instances[j].getProperty(idx).getValue();
 
            if(cv1.equal(cv2))
            {
                matchedCount++;
                instances.remove(j);
            }
        }
    }
 
    if (matchedCount != arr.size())
    {
        throw Exception("The Property values do not match.");
    }
}


void testQuery3(CIMClient& client)
{
    unsigned int matchedCount = 0;
    Array<CIMObject> arr=
        client.execQuery(NAMESPACE,String("DMTF:CQL"),String(QUERY3));
 
    Array<CIMInstance> instancesAll =
        client.enumerateInstances(NAMESPACE, CIMName("TST_Person"));
 
    Array<CIMInstance> instances;
 
    for (unsigned int i=0;i<instancesAll.size();i++)
    {
        Boolean testCond = false;
        int idx = instancesAll[i].findProperty(CIMName("name"));
        CIMValue cv1 =  instancesAll[i].getProperty(idx).getValue();
        idx = instancesAll[i].findProperty(CIMName("extraProperty"));
        if (idx >= 0)
        {
            CIMValue cv2 =  instancesAll[i].getProperty(idx).getValue();
            if (String::equal(cv2.toString(),String(PROP3)))
            {
                testCond = true;
            }
        }
 
        if (String::equal(cv1.toString(),String(NAME3)) && testCond)
        {
            instances.append(instancesAll[i]);
        }
    }
 
    if (arr.size() != instances.size())
    {
       throw Exception("Number of instances returned do not match.");
    }
 
    for (unsigned int i=0;i<arr.size();i++)
    {
        int idx = arr[i].findProperty(CIMName("name"));
        CIMValue cv1 =  arr[i].getProperty(idx).getValue();
        for (unsigned int j=0;j<instances.size();j++)
        {
            idx = instances[j].findProperty(CIMName("name"));
            CIMValue cv2 =  instances[j].getProperty(idx).getValue();
            
            if(cv1.equal(cv2))
            {
                matchedCount++;
                instances.remove(j);
            }
        }
    }
 
    if(matchedCount != arr.size())
    {
        throw Exception("The Property values do not match.");
    }
}


void testQuery4(CIMClient& client)
{
    unsigned int matchedCount = 0;
    Array<CIMObject> arr=
        client.execQuery(NAMESPACE,String("DMTF:CQL"),String(QUERY4));
 
    Array<CIMInstance> instancesAll =
        client.enumerateInstances(NAMESPACE, CIMName("TST_Person"));
 
    Array<CIMInstance> instances;
 
    for (unsigned int i=0;i<instancesAll.size();i++)
    {
        int idx = instancesAll[i].findProperty(CIMName("name"));
        CIMValue cv1 =  instancesAll[i].getProperty(idx).getValue();
 
        if (String::equal(cv1.toString(),String(NAME4a)) ||
            String::equal(cv1.toString(),String(NAME4b)))
        {
            instances.append(instancesAll[i]);
        }
    }
 
    if (arr.size() != instances.size())
    {
        throw Exception("Number of instances returned do not match.");
    }
 
    for (unsigned int i=0;i<arr.size();i++)
    {
        int idx = arr[i].findProperty(CIMName("name"));
        CIMValue cv1 =  arr[i].getProperty(idx).getValue();
        for (unsigned int j=0;j<instances.size();j++)
        {
            idx = instances[j].findProperty(CIMName("name"));
            CIMValue cv2 =  instances[j].getProperty(idx).getValue();
 
            if (cv1.equal(cv2))
            {
                matchedCount++;
                instances.remove(j);
            }
        }
    }
 
    if(matchedCount != arr.size())
    {
        throw Exception("The Property values do not match.");
    }
}



int main(int, char** argv)
{
    CIMClient client;
    String testName;
  
    try
    {
        client.connectLocal();
      
        testName = String("testQuery1");
        testQuery1(client);
        testName = String("testQuery2");
        testQuery2(client);
        testName = String("testQuery3");
        testQuery3(client);
        testName = String("testQuery4");
        testQuery4(client);
    }
    catch(Exception& e)
    {
        cout << argv[0] << ": "<< e.getMessage() << endl;
        cout << argv[0] << " ----- " << testName << " testcase failed" << endl;
        client.disconnect();
        return 1;
    }
  
    client.disconnect();
    cout << argv[0] << " +++++ passed all tests" << endl;
  
    return 0;
}


