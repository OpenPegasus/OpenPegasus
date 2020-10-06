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
#include <Pegasus/Common/XmlWriter.h>

PEGASUS_USING_PEGASUS;
PEGASUS_USING_STD;


CIMNamespaceName providerNamespace;
const char *queries[] = {
  // 0
  "select n32,s,ElementName,n64,n16,r32, r64,d "
      "from TestCMPI_ExecQuery where n32=42",
  // 1
  "SELECT * FROM TestCMPI_ExecQuery",
  // 2 - won't get anything back
  "SELECT not_present_property FROM TestCMPI_ExecQuery",
  // 3
  "SELECT * FROM TestCMPI_ExecQuery where s=s",
  // 4
  "SELECT * FROM TestCMPI_ExecQuery where s=s AND n64=64",
  // 5
  "SELECT s FROM TestCMPI_ExecQuery where n64=40",
#if 0
  // 6
  "SELECT * FROM TestCMPI_ExecQuery "
      "where TestCMPI_ExecQuery ISA TestCMPI_Parent",
  // 7 - wont'get anything back
  "SELECT * FROM TestCMPI_ExecQuery "
      "where TestCMPI_ExecQuery ISA TestCMPI_Indication",
#endif
  // 8
  "SELECT * FROM TestCMPI_ExecQuery WHERE (s IS NULL) OR (n16=16)",
  // 9
  "select n32,s,ElementName,n64,n16,f,d from TestCMPI_ExecQuery where n32>42",
  // 10
  "SELECT * FROM TestCMPI_ExecQuery where s<s",
  // 11
  "SELECT * FROM TestCMPI_ExecQuery where s<=s OR n64>=64",
  // 12
  "SELECT * FROM TestCMPI_ExecQuery where s=s OR n64<=64",
  // 13
  "SELECT * FROM TestCMPI_ExecQuery where n64<>40",
  // 14
  "SELECT * FROM TestCMPI_ExecQuery where n16 < 4",
  // 15
  "SELECT * FROM TestCMPI_ExecQuery where n16 > 4 AND n64 < 100",
  // 16
  "SELECT * FROM TestCMPI_ExecQuery where s>=s",
  // 17
  "SELECT * FROM TestCMPI_ExecQuery where c=c",
  // 18
  "SELECT * FROM TestCMPI_ExecQuery where b=1",
  // 19
  "SELECT * FROM TestCMPI_ExecQuery where n8<>64",
  // 20
  "SELECT * FROM TestCMPI_ExecQuery where n64=64",
  // 21
  "SELECT * FROM TestCMPI_ExecQuery where n32<=32",
  // 22
  "SELECT * FROM TestCMPI_ExecQuery where n16>=16",
  // 23
  "SELECT * FROM TestCMPI_ExecQuery where r32=1.232",
  // 24
  "SELECT * FROM TestCMPI_ExecQuery where r64>=112323",
  // 25
  "SELECT * FROM TestCMPI_ExecQuery where s8<=255",
  // 26
  "SELECT * FROM TestCMPI_ExecQuery where s16<=-11",
  // 27
  "SELECT * FROM TestCMPI_ExecQuery where s32>-11",
  // 28
  "SELECT * FROM TestCMPI_ExecQuery where s64<-11",
};

#define QUERIES 27
Boolean verbose;


void _checkStringProperty
  (CIMInstance & instance,
   const String & name, const String & value, Boolean null = false)
{
  Uint32 pos = instance.findProperty (name);
  PEGASUS_TEST_ASSERT (pos != PEG_NOT_FOUND);

  CIMProperty theProperty = instance.getProperty (pos);
  CIMValue theValue = theProperty.getValue ();

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

void _checkUint16Property
  (CIMInstance & instance, const String & name, Uint16 value)
{
  Uint32 pos = instance.findProperty (name);
  PEGASUS_TEST_ASSERT (pos != PEG_NOT_FOUND);

  CIMProperty theProperty = instance.getProperty (pos);
  CIMValue theValue = theProperty.getValue ();

  PEGASUS_TEST_ASSERT (theValue.getType () == CIMTYPE_UINT16);
  PEGASUS_TEST_ASSERT (!theValue.isArray ());
  PEGASUS_TEST_ASSERT (!theValue.isNull ());
  Uint16 result;
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

void _checkUint32Property
  (CIMInstance & instance, const String & name, Uint32 value)
{
  Uint32 pos = instance.findProperty (name);
  PEGASUS_TEST_ASSERT (pos != PEG_NOT_FOUND);

  CIMProperty theProperty = instance.getProperty (pos);
  CIMValue theValue = theProperty.getValue ();

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

void _checkUint64Property
  (CIMInstance & instance, const String & name, Uint64 value)
{
  Uint32 pos = instance.findProperty (name);
  PEGASUS_TEST_ASSERT (pos != PEG_NOT_FOUND);

  CIMProperty theProperty = instance.getProperty (pos);
  CIMValue theValue = theProperty.getValue ();

  PEGASUS_TEST_ASSERT (theValue.getType () == CIMTYPE_UINT64);
  PEGASUS_TEST_ASSERT (!theValue.isArray ());
  PEGASUS_TEST_ASSERT (!theValue.isNull ());
  Uint64 result;
  theValue.get (result);

  if (verbose)
    {
      if (result != value)
        {
          char buffer[32];      // Should need 21 chars max
          sprintf (buffer, "%" PEGASUS_64BIT_CONVERSION_WIDTH "u", value);
          cerr << "Property value comparison failed.  ";
          cerr << "Expected " << buffer << "; ";
          sprintf (buffer, "%" PEGASUS_64BIT_CONVERSION_WIDTH "u", result);
          cerr << "Actual property value was " << buffer << "." << endl;
        }
    }

  PEGASUS_TEST_ASSERT (result == value);
}

void
_usage ()
{
  cerr << "Usage: TestCMPIInstanceExecQuery {test} {namespace}" << endl;
}

static void
_test1 (CIMClient & client, const String & ql)
{
  try
  {
    for (Uint32 i = 0; i < QUERIES; i++)
      {

        if (verbose)
          cerr << "Querying " << queries[i] << endl;

        Array < CIMObject > objects = client.execQuery (providerNamespace,
                                                        ql, queries[i]);

        if (objects.size () == 0)
          {
            // Only the third (second when starting from zero)
            // and eight(7) won't return instances.
            //PEGASUS_TEST_ASSERT(i == 2 || i == 5 || i == 7
            //                    || i == 8 || i == 12);
            if (verbose)
              cerr <<i<< " No instance returned.. That is good" << endl;
          }

        for (Uint32 i = 0; i < objects.size (); i++)
          {

            if (objects[i].isInstance ())
              {

                CIMInstance inst (objects[i]);

                if (inst.findProperty ("ElementName") != PEG_NOT_FOUND)
                  _checkStringProperty (inst, "ElementName",
                                        "TestCMPI_ExecQuery");

                if (inst.findProperty ("s") != PEG_NOT_FOUND)
                  _checkStringProperty (inst, "s", "s");

                if (inst.findProperty ("n32") != PEG_NOT_FOUND)
                  _checkUint32Property (inst, "n32", 32);

                if (inst.findProperty ("n64") != PEG_NOT_FOUND)
                  _checkUint64Property (inst, "n64", 64);

                if (inst.findProperty ("n16") != PEG_NOT_FOUND)
                  _checkUint16Property (inst, "n16", 16);

              }
          }
      }
  }
  catch (const Exception & e)
  {
    cerr << "test failed: " << e.getMessage () << endl;
    exit (-1);
  }

}


static void
_test2 (CIMClient & client)
{
  Uint32 exceptions = 0;
  CIMObjectPath instanceName;
  Array < CIMKeyBinding > keyBindings;

  keyBindings.append (CIMKeyBinding ("ElementNameName",
                                     "TestCMPI_ExecQuery",
                                     CIMKeyBinding::STRING));

  instanceName.setNameSpace (providerNamespace);
  instanceName.setClassName ("TestCMPI_ExecQuery");
  instanceName.setKeyBindings (keyBindings);

  /* Call the unsupported functions of the provider. */
  try
  {
    CIMInstance instance (client.getInstance (providerNamespace,
                                              instanceName));
  } catch (const CIMException &)
  {
     exceptions ++;
  }


  try
  {
    client.deleteInstance (providerNamespace, instanceName);

  } catch (const CIMException & )
  {
     exceptions ++;
  }
  CIMClass thisClass = client.getClass(
                           providerNamespace,
                           "TestCMPI_ExecQuery",
                           false,
                           true,
                           true,
                           CIMPropertyList());
  Array<CIMName> propertyNameList;
  propertyNameList.append(CIMName("ElementName"));
  CIMPropertyList myPropertyList(propertyNameList);
  // create the instance with the defined properties
  CIMInstance newInstance = thisClass.buildInstance(true, true, myPropertyList);
  newInstance.getProperty(0).setValue(CIMValue(String("TestCMPI_execQuery") ));
  try
  {

    CIMObjectPath objectPath (client.createInstance (providerNamespace,
                                                     newInstance));


  } catch (const CIMException &)
  {
     exceptions ++;
  }

  try
  {
    client.modifyInstance (providerNamespace, newInstance);

  } catch (const CIMException &)
  {
     exceptions ++;
  }
  try
  {

    Array < CIMInstance > instances =
      client.enumerateInstances (providerNamespace,
                                 CIMName ("TestCMPI_ExecQuery"));
  } catch (const CIMException &)
  {
     exceptions ++;
  }

  try
  {
    Array < CIMObjectPath > objectPaths =
      client.enumerateInstanceNames (providerNamespace,
                                     CIMName ("TestCMPI_ExecQuery"));
  } catch (const CIMException &)
  {
     exceptions ++;
  }

  PEGASUS_TEST_ASSERT(exceptions ==  6);

}

void _test3(CIMClient & client)
{
    static const String NAMESPACE("test/TestProvider");
    static const String CLASSNAME("cmpiPerf_TestClassA");
    cout<<"enumerate instance request with empty prop list"<<endl;
    {
        try
        {
            Array<CIMName> propNames;
            Array < CIMInstance > instances =
            client.enumerateInstances(
                NAMESPACE,
                CLASSNAME,
                true,
                true,
                false,
                false,
                CIMPropertyList(propNames) );
            for(Uint32 i=0;i<instances.size();i++)
            {
                Uint32 propertyCount = instances[i].getPropertyCount();
                if (verbose)
                {
                    XmlWriter::printInstanceElement(instances[i]);
                }
                if(propertyCount == 0)
                {
                    cout<<"Filter enumerateInstances test on "
                        <<"cmpiPerf_TestClassA SUCCEEDED :Filtering "
                        <<"the ciminstance with a empty property list "
                        <<"returned zero properties as expected"<<endl;
                }
                else
                {
                    cout<<"Filter enumerateInstances test on "
                        <<"cmpiPerf_TestClassA  FAILED:Filtering the" 
                        <<"ciminstance with a empty property list returned " 
                        <<"some properties which is not expected"<<endl;
                    //PEGASUS_TEST_ASSERT(false);
                }
            }    
        }catch (const CIMException &e)
        {
            cout << "CIMException(" << e.getCode() << "): " <<
                e.getMessage() << endl;
        }
    }
    cout<<"++++++++property list with wrong prop names"
        <<" filtered output++++++++"<<endl;
    { 
        try
        {
            Array<CIMName> propNames;
            propNames.append(CIMName(String("theK")));
            Array < CIMInstance > instances =
            client.enumerateInstances(
                NAMESPACE,
                CLASSNAME,
                true,
                true,
                false,
                false,
                CIMPropertyList(propNames) );
            for(Uint32 i=0;i<instances.size();i++)
            {
                Uint32 propertyCount = instances[i].getPropertyCount();
                if (verbose)
                {
                    XmlWriter::printInstanceElement(instances[i]);
                }
                if(propertyCount == 0)
                {
                    cout<<"Filter enumerateInstances test on "
                        <<"cmpiPerf_TestClassA SUCCEEDED :Filtering "
                        <<"the ciminstance with a wrong property list "
                        <<"returned zero properties as expected"<<endl;
                }
                else
                {
                    cout<<"Filter enumerateInstances test on "
                        <<"cmpiPerf_TestClassA FAILED:Filtering the" 
                        <<"ciminstance with a wrong property list returned "
                        <<" some properties which is not expected"<<endl;
                    PEGASUS_TEST_ASSERT(false);
                }
            }
        }catch (const CIMException &e)
        {
            cout << "CIMException(" << e.getCode() << "): " <<
                e.getMessage() << endl;   
        }
    }
    cout<<"++++++++property list with mentioned property names"
            <<" filtered output++++++++"<<endl;
    {
        try
        {
            Array<CIMName> propNames;
            propNames.append(CIMName(String("theKey")));
            Array < CIMInstance > instances =
            client.enumerateInstances (
                NAMESPACE,
                CLASSNAME,
                true,
                true,
                false,
                false,
                CIMPropertyList(propNames) );
            for(Uint32 i=0;i<instances.size();i++)
            {
                Uint32 propertyCount = instances[i].getPropertyCount();
                Uint32 propNameCount = 0;
                if (verbose)
                {
                    XmlWriter::printInstanceElement(instances[i]);
                }
                for(Uint32 j=0;j<propertyCount;j++)
                {
                    String propName=
                        instances[i].getProperty(j).getName().getString();
                    if((propName == "theKey") )
                    {
                       propNameCount++;
                    }
                }
                if((propertyCount == 1)&&(propNameCount == 1))
                {
                    cout<<"Filter enumerateInstances test on "
                        <<"cmpiPerf_TestClassA SUCCEEDED :Filtering "
                        <<"the ciminstance with a mentioned property list "
                        <<"returned all properties as expected"<<endl;
                }
                else
                {
                    cout<<"Filter enumerateInstances test on "
                        <<"cmpiPerf_TestClassA FAILED:Filtering the "
                        <<"ciminstance with a mentioned property list did " 
                        <<"not return all properties as expected"<<endl;
                    PEGASUS_TEST_ASSERT(false);
                }
            }
        }catch (const CIMException &e)
        {
            cout << "CIMException(" << e.getCode() << "): " <<
                e.getMessage() << endl;
        }
    }
    cout<<"++++++++NULL proplist filtered output++++++++"<<endl;
    {
        try
        {
            Array < CIMInstance > instances =
            client.enumerateInstances(
                NAMESPACE,
                CLASSNAME,
                true,
                true,
                false,
                false,
                CIMPropertyList());
            for(Uint32 i=0;i<instances.size();i++)
            {
                Uint32 propertyCount = instances[i].getPropertyCount();
                if (verbose)
                {
                    XmlWriter::printInstanceElement(instances[i]);
                    cout<<"i::"<<i<<endl;
                }
                if(propertyCount == 24)
                {
                    cout<<"Filter enumerateInstances test on "
                        <<"cmpiPerf_TestClassA SUCCEEDED :Filtering "
                        <<"the ciminstance with a NULL property list "
                        <<"returned all properties as expected"<<endl;
                }
                else
                {
                    cout<<"Filter enumerateInstances test on "
                        <<"cmpiPerf_TestClassA FAILED:Filtering the "
                        <<"ciminstance with a NULL property list did not "
                        <<"return all properties as expected"<<endl;
                    PEGASUS_TEST_ASSERT(false);
                }
            }
        }catch (const CIMException &e)
        {
            cout << "CIMException(" << e.getCode() << "): " <<
                e.getMessage() << endl;
        }
    }
}

void _test4(CIMClient & client)
{
    static const String NAMESPACE("test/TestProvider");
    static const String CLASSNAME("cmpiPerf_TestClassA");
    Array<CIMObjectPath> cimInstanceNames =
    client.enumerateInstanceNames(
        NAMESPACE,
        CLASSNAME);
    cout<<"+++++++++empty property list filtered output++++++"<<endl;
    {
        Array<CIMName> propNames;
        for (Uint32 i = 0, n = cimInstanceNames.size(); i < n; i++)
        {
            CIMInstance cimInstance=client.getInstance(
                NAMESPACE,
                cimInstanceNames[i],
                true,
                false,
                false,
                CIMPropertyList(propNames));
            if (verbose)
            {
                XmlWriter::printInstanceElement(cimInstance);
            }
            Uint32 propertyCount = cimInstance.getPropertyCount();
            if(propertyCount == 0)
            {
                cout<<"Filter getInstance test on "
                    <<"cmpiPerf_TestClassA SUCCEEDED :Filtering "
                    <<"the ciminstance with a empty property list "
                    <<"returned zero properties as expected"<<endl;
            }
            else
            {
                cout<<"Filter getInstance test on cmpiPerf_TestClassA"
                    <<" FAILED:Filtering the ciminstance with a empty "
                    <<"property list returned some properties "
                    <<"which is not expected"<<endl;
                PEGASUS_TEST_ASSERT(false);
            } 
        }
    }
    cout<<"+++++++++wrong property list filtered output++++++"<<endl;
    {
        Array<CIMName> propNames;
        propNames.append(CIMName(String("theK")));
        for (Uint32 i = 0, n = cimInstanceNames.size(); i < n; i++)
        {
            CIMInstance cimInstance=client.getInstance(
                NAMESPACE,
                cimInstanceNames[i],
                true,
                false,
                false,
                CIMPropertyList(propNames));
            if (verbose)
            {
                XmlWriter::printInstanceElement(cimInstance);
            }
            Uint32 propertyCount = cimInstance.getPropertyCount();
            if(propertyCount == 0)
            {
                cout<<"Filter getInstance test on "
                    <<"cmpiPerf_TestClassA SUCCEEDED :Filtering "
                    <<"the ciminstance with a wrong property list "
                    <<"returned zero properties as expected"<<endl;
            }
            else
            {
                cout<<"Filter getInstance test on cmpiPerf_TestClassA"
                    <<" FAILED:Filtering the ciminstance with a wrong "
                    <<"property list returned some properties "
                    <<"which is not expected"<<endl;
                PEGASUS_TEST_ASSERT(false);
            }
        }
    }
    cout<<"+++++++++NULL property list filtered output++++++"<<endl;
    {
        for (Uint32 i = 0, n = cimInstanceNames.size(); i < n; i++)
        {
            CIMInstance cimInstance=client.getInstance(
                NAMESPACE,
                cimInstanceNames[i],
                true,
                false,
                false,
                CIMPropertyList());
            if (verbose)
            {
                XmlWriter::printInstanceElement(cimInstance);
            }
            Uint32 propertyCount = cimInstance.getPropertyCount();
            if(propertyCount == 24)
            {
                cout<<"Filter getInstance test on "
                    <<"cmpiPerf_TestClassA SUCCEEDED :Filtering "
                    <<"the ciminstance with a NULL property list "
                    <<"returned all properties as expected"<<endl;
            }
            else
            {
                cout<<"Filter getInstance test on cmpiPerf_TestClassA"
                    <<" FAILED:Filtering the ciminstance with a NULL "
                    <<"property list did not return all properties "
                    <<"as expected"<<endl;
                PEGASUS_TEST_ASSERT(false);
            }
        }
    }
    cout<<"+++++++++mentioned  property list filtered output++++++"<<endl;
    {
        Array<CIMName> propNames;
        propNames.append(CIMName(String("RequestedState")));
        propNames.append(CIMName(String("theKey")));
        for (Uint32 i = 0, n = cimInstanceNames.size(); i < n; i++)
        {
            CIMInstance cimInstance=client.getInstance(
                NAMESPACE,
                cimInstanceNames[i],
                true,
                false,
                false,
                CIMPropertyList(propNames));
            if (verbose)
            {
                XmlWriter::printInstanceElement(cimInstance);
            }
            Uint32 propertyCount = cimInstance.getPropertyCount();
            Uint32 propNameCount = 0;
            for(Uint32 j=0;j<propertyCount;j++)
            {
                String propName=
                    cimInstance.getProperty(j).getName().
                    getString();
                if((propName == "RequestedState")||(propName == "theKey"))
                {
                    propNameCount++;
                }
            } 
            if((propertyCount == 2) && (propNameCount == 2))
            {
                cout<<"Filter getInstance test on "
                    <<"cmpiPerf_TestClassA SUCCEEDED :Filtering "
                    <<"the ciminstance with a mentioned property list "
                    <<"returned all properties as expected"<<endl;
            }
            else
            {
                cout<<"Filter getInstance test on cmpiPerf_TestClassA"
                    <<" FAILED:Filtering the ciminstance with a mentioned "
                    <<"property list did not return all properties "
                    <<"as expected"<<endl;
                PEGASUS_TEST_ASSERT(false);
            }
        }
    } 
}

int
main (int argc, char **argv)
{
  verbose = (getenv ("PEGASUS_TEST_VERBOSE")) ? true : false;
  CIMClient client;
  try
  {
    client.connectLocal ();
  }
  catch (const Exception & e)
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
          _test1 (client,"WQL");
          _test1 (client,"DMTF:CQL");

          _test2 (client);
          _test3(client);
          _test4(client);          
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
