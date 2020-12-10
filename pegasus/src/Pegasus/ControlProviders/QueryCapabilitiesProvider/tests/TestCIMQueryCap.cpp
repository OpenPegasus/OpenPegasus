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
//
//%/////////////////////////////////////////////////////////////////////////////


#include <Pegasus/Client/CIMClient.h>
#include <Pegasus/Common/CIMClass.h>
#include <Pegasus/Common/CIMName.h>
//NOCHKSRC
#include <Pegasus/ControlProviders/QueryCapabilitiesProvider/CIMQueryCapabilitiesProvider.h>
// DOCHKSRC
#include <iostream>

PEGASUS_USING_PEGASUS;
PEGASUS_USING_STD;

PEGASUS_NAMESPACE_BEGIN

// To add more capabilities, change the below static variables
const Uint16 FeatureSet[] = {2,8};
const int NUM_QUERY_CAPABILITIES = 2;

#define NAMESPACE_CIMV2 "root/cimv2"
#define NAMESPACE_SAMPLEPROVIDER "root/SampleProvider"
static const String CIM_QUERYCAPCLASS_NAME("CIM_QueryCapabilities");

void getFeatureSet(Array<Uint16>& features)
{
  for(int i=0; i<NUM_QUERY_CAPABILITIES; i++)
  {
    features[i] = FeatureSet[i];
  }
}


void _checkIfReturnedValueIsCorrect(Array<Uint16>& providerReturnedVal)
{

   Array<Uint16> actualVal(NUM_QUERY_CAPABILITIES);

   if (providerReturnedVal.size() != (unsigned)NUM_QUERY_CAPABILITIES)
   {
      throw Exception(
          "Number of capabilities returned by the Provider "
              "does not match the actual value.");
   }

   getFeatureSet(actualVal);

   for (unsigned int j=0; j<(unsigned)NUM_QUERY_CAPABILITIES; j++)
   {
     if (providerReturnedVal[j] != actualVal[j])
     {
        char msg[1024];
        sprintf(msg, "Expected capability value=%hu, returned=%hu.",
                actualVal[j], providerReturnedVal[j]);
        throw Exception(msg);
     }
   }
}

void testEnumInstanceNames(CIMClient& client,const char *ns)
{
   Array<CIMObjectPath> refs;

   refs = client.enumerateInstanceNames(ns,CIM_QUERYCAPCLASS_NAME);

   if(refs.size() != 1) throw Exception("references.size() incorrect");

   Array<CIMKeyBinding> keys = refs[0].getKeyBindings();

   //-- make sure we're the right instance
   CIMName keyName;
   String keyValue;

   if (keys.size() != NUM_KEY_PROPERTIES)
       throw Exception("Wrong number of keys");

    keyName = keys[0].getName();
    keyValue = keys[0].getValue();

    if(keyName.getString() != String(PROPERTY_NAME_INSTANCEID) )
        throw Exception("Incorrect Key");
    if(keyValue != String(INSTANCEID_VALUE))
        throw Exception(keyValue);

}

void testEnumInstances(CIMClient& client, const char* ns)
{
    Array<CIMInstance> instances;

   instances = client.enumerateInstances(ns,CIM_QUERYCAPCLASS_NAME);

   if(instances.size() != 1) throw Exception("instances.size() incorrect");

   Array<Uint16> providerReturnedVal;
   //Array<Uint16> actualVal(NUM_QUERY_CAPABILITIES);
   Uint32 prop = 0;
   CIMObjectPath path;

   path = instances[0].getPath();
   Array<CIMKeyBinding> keys = path.getKeyBindings();

   //-- make sure we're the right instance
   CIMName keyName;
   String keyValue;

   if (keys.size() != NUM_KEY_PROPERTIES)
       throw Exception("Wrong number of keys");

    keyName = keys[0].getName();
    keyValue = keys[0].getValue();

    if(keyName.getString() != String(PROPERTY_NAME_INSTANCEID) )
        throw Exception("Incorrect Key");
    if(keyValue != String(INSTANCEID_VALUE))
        throw Exception(keyValue);

     prop = instances[0].findProperty(CIMName(PROPERTY_NAME_CQLFEATURES));
     instances[0].getProperty(prop).getValue().get(providerReturnedVal);

     _checkIfReturnedValueIsCorrect(providerReturnedVal);

}


void testGetInstance(CIMClient& client, const char* ns)
{

   CIMInstance instance;
   Array<CIMKeyBinding> keyBindings;
   keyBindings.append(CIMKeyBinding(PROPERTY_NAME_INSTANCEID,
                                INSTANCEID_VALUE,
                                CIMKeyBinding::STRING));

   CIMObjectPath objectPath(String::EMPTY, ns,
                  CIM_QUERYCAPCLASS_NAME, keyBindings);

   instance = client.getInstance(ns, objectPath, false);

   Array<Uint16> providerReturnedVal;

   Uint32 prop = instance.findProperty(CIMName(PROPERTY_NAME_CQLFEATURES));
   instance.getProperty(prop).getValue().get(providerReturnedVal);

   _checkIfReturnedValueIsCorrect(providerReturnedVal);
}

void testCreateInstance(CIMClient& client, const char* ns)
{
  CIMObjectPath path;
  Array<CIMInstance> instances;
  CIMName keyName;
  String keyValue;

  instances = client.enumerateInstances(ns, CIM_QUERYCAPCLASS_NAME);

  Array<CIMKeyBinding> keys = instances[0].getPath().getKeyBindings();

  keys[0].setValue("100");

  path = instances[0].getPath();
  path.setKeyBindings(keys);

  try
  {
    path = client.createInstance(ns, instances[0]);
  }
  catch(Exception)
  {
    // Do nothing. This is expected since createInstance is NOT
    // supported.
    return;
  }

  throw Exception("createInstance is supported");
}


void testDeleteInstance(CIMClient& client, const char* ns)
{
  Array<CIMInstance> instances;

  instances = client.enumerateInstances(ns, CIM_QUERYCAPCLASS_NAME);

  try
  {
    client.deleteInstance(ns, instances[0].getPath());
  }
  catch(Exception)
  {
    // Do nothing. This is expected since deleteInstance is NOT
    // supported.
    return;
  }

  throw Exception("deleteInstance is supported");
}


void testModifyInstance(CIMClient& client, const char* ns)
{
  Array<CIMInstance> instances;

  instances = client.enumerateInstances(ns, CIM_QUERYCAPCLASS_NAME);

  CIMProperty prop;
  Uint32 pos = instances[0].findProperty(PROPERTY_NAME_CAPTION);
  prop = instances[0].getProperty(pos);
  instances[0].removeProperty(pos);

  prop.setValue(CIMValue(String("MODIFIED CAPTION")));

  instances[0].addProperty(prop);

  try
  {
    client.modifyInstance(ns, instances[0]);
  }
  catch(Exception)
  {
    // Do nothing. This is expected since modifyInstance is NOT
    // supported.
    return;
  }

  throw Exception("modifyInstance is supported");
}

PEGASUS_NAMESPACE_END


int main(int, char** argv)
{

   CIMClient client;

   try
   {
     client.connectLocal();
   }
   catch (Exception& e)
   {
      cerr << "Error: " << e.getMessage() <<  endl;
      cerr << "Exception occured while trying to connect to the server."
           << endl;
      return(1);
   }

   String testCaseName;

   try
   {
     testCaseName.assign("Enumerate Instance Names");
     testEnumInstanceNames(client, NAMESPACE_CIMV2);
     testEnumInstanceNames(client, NAMESPACE_SAMPLEPROVIDER);

     testCaseName.assign("Enumerate Instances");
     testEnumInstances(client, NAMESPACE_CIMV2);
     testEnumInstances(client, NAMESPACE_SAMPLEPROVIDER);

     testCaseName.assign("Get Instance");
     testGetInstance(client, NAMESPACE_CIMV2);
     testGetInstance(client, NAMESPACE_SAMPLEPROVIDER);

     testCaseName.assign("Create Instance");
     testCreateInstance(client, NAMESPACE_CIMV2);
     testCreateInstance(client, NAMESPACE_SAMPLEPROVIDER);

     testCaseName.assign("Delete Instance");
     testDeleteInstance(client, NAMESPACE_CIMV2);
     testDeleteInstance(client, NAMESPACE_SAMPLEPROVIDER);

     testCaseName.assign("Modify Instance");
     testModifyInstance(client, NAMESPACE_CIMV2);
     testModifyInstance(client, NAMESPACE_SAMPLEPROVIDER);
   }
   catch(Exception& e)
   {
      cerr << argv[0] << ": Exception Occcured. " << e.getMessage() << endl;
      cerr << argv[0] << ": " << testCaseName << " Failed. " << endl;
      exit(1);
   }

   cout << argv[0] << " +++++ passed all tests" << endl;
   return 0;
}


