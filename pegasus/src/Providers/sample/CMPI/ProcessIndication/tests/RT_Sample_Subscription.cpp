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
// Author: Konrad Rzeszutek <konradr@us.ibm.com>
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

const CIMNamespaceName SOURCENAMESPACE =
CIMNamespaceName ("root/SampleProvider");

String _query ("SELECT * FROM CMPI_RT_SampleIndication");
String _filter ("CMPI_RT_SampleIndication_Filter_");
String _handler ("CMPI_RT_SampleIndication_Handler_");
String _destination ("localhost/CIMListener/Pegasus_SimpleDisplayConsumer");
String _lang ("WQL");

Boolean verbose;

void _addStringProperty
  (CIMInstance & instance,
   const String & name, const String & value, Boolean null = false)
{
  if (null)
    {
      instance.addProperty (CIMProperty (CIMName (name),
                                         CIMValue (CIMTYPE_STRING, false)));
    }
  else
    {
      instance.addProperty (CIMProperty (CIMName (name), value));
    }
}

void _addUint16Property
  (CIMInstance & instance,
   const String & name, Uint16 value, Boolean null = false)
{
  if (null)
    {
      instance.addProperty (CIMProperty (CIMName (name),
                                         CIMValue (CIMTYPE_UINT16, false)));
    }
  else
    {
      instance.addProperty (CIMProperty (CIMName (name), value));
    }
}
void _addUint64Property
  (CIMInstance & instance,
   const String & name, Uint64 value, Boolean null = false)
{
  if (null)
    {
      instance.addProperty (CIMProperty (CIMName (name),
                                         CIMValue (CIMTYPE_UINT64, false)));
    }
  else
    {
      instance.addProperty (CIMProperty (CIMName (name), value));
    }
}

CIMObjectPath _buildFilterOrHandlerPath
  (const CIMName & className, const String & name)
{
  CIMObjectPath path;

  Array < CIMKeyBinding > keyBindings;
  keyBindings.append (CIMKeyBinding ("SystemCreationClassName",
                                     System::getSystemCreationClassName (),
                                     CIMKeyBinding::STRING));
  keyBindings.
    append (CIMKeyBinding
            ("SystemName", System::getFullyQualifiedHostName (),
             CIMKeyBinding::STRING));
  keyBindings.
    append (CIMKeyBinding
            ("CreationClassName", className.getString (),
             CIMKeyBinding::STRING));
  keyBindings.append (CIMKeyBinding ("Name", name, CIMKeyBinding::STRING));
  path.setClassName (className);
  path.setKeyBindings (keyBindings);

  return path;
}

CIMObjectPath _buildSubscriptionPath
  (const String & filterName,
   const CIMName & handlerClass, const String & handlerName)
{
  CIMObjectPath filterPath = _buildFilterOrHandlerPath
    (PEGASUS_CLASSNAME_INDFILTER, filterName);

  CIMObjectPath handlerPath = _buildFilterOrHandlerPath (handlerClass,
                                                         handlerName);

  Array < CIMKeyBinding > subscriptionKeyBindings;
  subscriptionKeyBindings.append (CIMKeyBinding ("Filter",
                                                 filterPath.toString (),
                                                 CIMKeyBinding::REFERENCE));
  subscriptionKeyBindings.
    append (CIMKeyBinding
            ("Handler", handlerPath.toString (), CIMKeyBinding::REFERENCE));
  CIMObjectPath subscriptionPath ("", CIMNamespaceName (),
                                  PEGASUS_CLASSNAME_INDSUBSCRIPTION,
                                  subscriptionKeyBindings);

  return subscriptionPath;
}

CIMInstance _buildSubscriptionInstance
  (const CIMObjectPath & filterPath,
   const CIMName & handlerClass, const CIMObjectPath & handlerPath)
{
  CIMInstance subscriptionInstance (PEGASUS_CLASSNAME_INDSUBSCRIPTION);

  subscriptionInstance.addProperty (CIMProperty (CIMName ("Filter"),
                                                 filterPath, 0,
                                                 PEGASUS_CLASSNAME_INDFILTER));
  subscriptionInstance.
    addProperty (CIMProperty
                 (CIMName ("Handler"), handlerPath, 0, handlerClass));

  return subscriptionInstance;
}

void _checkFilterOrHandlerPath
  (const CIMObjectPath & path, const CIMName & className, const String & name)
{
  PEGASUS_TEST_ASSERT (path.getClassName () == className);
  Array < CIMKeyBinding > keyBindings = path.getKeyBindings ();
  Boolean SCCNfound = false;
  Boolean SNfound = false;
  Boolean CCNfound = false;
  Boolean Nfound = false;
  for (Uint32 i = 0; i < keyBindings.size (); i++)
    {
      if (keyBindings[i].getName ().equal ("SystemCreationClassName"))
        {
          SCCNfound = true;
          PEGASUS_TEST_ASSERT (keyBindings[i].getValue () ==
                               System::getSystemCreationClassName ());
        }
      else if (keyBindings[i].getName ().equal ("SystemName"))
        {
          SNfound = true;
          PEGASUS_TEST_ASSERT (keyBindings[i].getValue () ==
                               System::getFullyQualifiedHostName ());
        }
      else if (keyBindings[i].getName ().equal ("CreationClassName"))
        {
          CCNfound = true;
          PEGASUS_TEST_ASSERT (keyBindings[i].getValue () ==
                               className.getString ());
        }
      else if (keyBindings[i].getName ().equal ("Name"))
        {
          Nfound = true;
          PEGASUS_TEST_ASSERT (keyBindings[i].getValue () == name);
        }
      else
        {
          PEGASUS_TEST_ASSERT (false);
        }
    }

  PEGASUS_TEST_ASSERT (SCCNfound);
  PEGASUS_TEST_ASSERT (SNfound);
  PEGASUS_TEST_ASSERT (CCNfound);
  PEGASUS_TEST_ASSERT (Nfound);
}

void _checkSubscriptionPath
  (const CIMObjectPath & path,
   const String & filterName,
   const CIMName & handlerClass, const String & handlerName)
{
  PEGASUS_TEST_ASSERT (path.getClassName () ==
                       PEGASUS_CLASSNAME_INDSUBSCRIPTION);
  Array < CIMKeyBinding > keyBindings = path.getKeyBindings ();
  Boolean filterFound = false;
  Boolean handlerFound = false;
  for (Uint32 i = 0; i < keyBindings.size (); i++)
    {
      if (keyBindings[i].getName ().equal ("Filter"))
        {
          filterFound = true;
          CIMObjectPath filterPath = _buildFilterOrHandlerPath
            (PEGASUS_CLASSNAME_INDFILTER, filterName);
          PEGASUS_TEST_ASSERT (keyBindings[i].getValue () ==
                               filterPath.toString ());
        }
      else if (keyBindings[i].getName ().equal ("Handler"))
        {
          handlerFound = true;
          CIMObjectPath handlerPath = _buildFilterOrHandlerPath
            (handlerClass, handlerName);
          PEGASUS_TEST_ASSERT (keyBindings[i].getValue () ==
                               handlerPath.toString ());
        }
      else
        {
          PEGASUS_TEST_ASSERT (false);
        }
    }

  PEGASUS_TEST_ASSERT (filterFound);
  PEGASUS_TEST_ASSERT (handlerFound);
}

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

void
_checkUint32Value (CIMValue & theValue, Uint32 value)
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

void _checkExceptionCode
  (const CIMException & e, const CIMStatusCode expectedCode)
{
  if (verbose)
    {
      if (e.getCode () != expectedCode)
        {
          cerr << "CIMException comparison failed.  ";
          cerr << "Expected " << cimStatusCodeToString (expectedCode) << "; ";
          cerr << "Actual exception was " << e.getMessage () << "." << endl;
        }
    }

  PEGASUS_TEST_ASSERT (e.getCode () == expectedCode);
}

void _deleteSubscriptionInstance
  (CIMClient & client,
   const String & filterName,
   const CIMName & handlerClass, const String & handlerName)
{
  CIMObjectPath subscriptionPath = _buildSubscriptionPath
    (filterName, handlerClass, handlerName);
  client.deleteInstance (PEGASUS_NAMESPACENAME_INTEROP, subscriptionPath);
}

void _deleteHandlerInstance
  (CIMClient & client, const CIMName & className, const String & name)
{
  CIMObjectPath path = _buildFilterOrHandlerPath (className, name);
  client.deleteInstance (PEGASUS_NAMESPACENAME_INTEROP, path);
}

void
_deleteFilterInstance (CIMClient & client, const String & name)
{
  CIMObjectPath path = _buildFilterOrHandlerPath
    (PEGASUS_CLASSNAME_INDFILTER, name);
  client.deleteInstance (PEGASUS_NAMESPACENAME_INTEROP, path);
}


void
_usage ()
{
  cerr
    << "Usage: TestCMPIIndicationSubscription "
    << "{test | cleanup }" << endl;
}


//
//  Valid test cases: create, get, enumerate, modify, delete operations
//
void
_addFilter (CIMClient & client, String & filter, String & query,
            String & qlang)
{
  CIMObjectPath path;
  CIMInstance retrievedInstance;

  CIMInstance filter01 (PEGASUS_CLASSNAME_INDFILTER);
  _addStringProperty (filter01, "SystemCreationClassName",
                      System::getSystemCreationClassName ());
  _addStringProperty (filter01, "SystemName",
                      System::getFullyQualifiedHostName ());
  _addStringProperty (filter01, "CreationClassName",
                      PEGASUS_CLASSNAME_INDFILTER.getString ());
  _addStringProperty (filter01, "Name", filter);
  _addStringProperty (filter01, "SourceNamespace",
                      SOURCENAMESPACE.getString ());
  _addStringProperty (filter01, "Query", query);
  _addStringProperty (filter01, "QueryLanguage", qlang);
  path = client.createInstance (PEGASUS_NAMESPACENAME_INTEROP, filter01);

  _checkFilterOrHandlerPath (path, PEGASUS_CLASSNAME_INDFILTER, filter);
  retrievedInstance = client.getInstance (PEGASUS_NAMESPACENAME_INTEROP, path);
  _checkStringProperty (retrievedInstance, "SystemCreationClassName",
                        System::getSystemCreationClassName ());
  _checkStringProperty (retrievedInstance, "SystemName",
                        System::getFullyQualifiedHostName ());
  _checkStringProperty (retrievedInstance, "CreationClassName",
                        PEGASUS_CLASSNAME_INDFILTER.getString ());
  _checkStringProperty (retrievedInstance, "Name", filter);
  _checkStringProperty (retrievedInstance, "SourceNamespace",
                        SOURCENAMESPACE.getString ());
  _checkStringProperty (retrievedInstance, "Query", query);
  _checkStringProperty (retrievedInstance, "QueryLanguage", qlang);

}

void
_addHandler (CIMClient & client, String & handler, String & dest)
{
  CIMObjectPath path;
  CIMInstance retrievedInstance;
  //
  //  Create persistent CIMXML handler
  //
  CIMInstance handler01 (PEGASUS_CLASSNAME_INDHANDLER_CIMXML);
  _addStringProperty (handler01, "SystemCreationClassName",
                      System::getSystemCreationClassName ());
  _addStringProperty (handler01, "SystemName",
                      System::getFullyQualifiedHostName ());
  _addStringProperty (handler01, "CreationClassName",
                      PEGASUS_CLASSNAME_INDHANDLER_CIMXML.getString ());
  _addStringProperty (handler01, "Name", handler);
  _addStringProperty (handler01, "Owner", "an owner");
  _addUint16Property (handler01, "PersistenceType", 2);
  _addStringProperty (handler01, "OtherPersistenceType", String::EMPTY, true);
  _addStringProperty (handler01, "Destination", dest);


  path = client.createInstance (PEGASUS_NAMESPACENAME_INTEROP, handler01);

  _checkFilterOrHandlerPath (path, PEGASUS_CLASSNAME_INDHANDLER_CIMXML,
                             handler);

  retrievedInstance = client.getInstance (PEGASUS_NAMESPACENAME_INTEROP, path);
  _checkStringProperty (retrievedInstance, "SystemCreationClassName",
                        System::getSystemCreationClassName ());
  _checkStringProperty (retrievedInstance, "SystemName",
                        System::getFullyQualifiedHostName ());
  _checkStringProperty (retrievedInstance, "CreationClassName",
                        PEGASUS_CLASSNAME_INDHANDLER_CIMXML.getString ());
  _checkStringProperty (retrievedInstance, "Name", handler);
  _checkStringProperty (retrievedInstance, "Owner", "an owner");
  _checkUint16Property (retrievedInstance, "PersistenceType", 2);
  _checkStringProperty (retrievedInstance, "OtherPersistenceType",
                        String::EMPTY, true);
  _checkStringProperty (retrievedInstance, "Destination", dest);
}

void
_addSubscription (CIMClient & client, String & filter, String & handler)
{
  CIMObjectPath path;
  CIMInstance retrievedInstance;
  //
  //  Create subscriptions
  //
  CIMInstance subscription01 = _buildSubscriptionInstance
    (_buildFilterOrHandlerPath (PEGASUS_CLASSNAME_INDFILTER, filter),
     PEGASUS_CLASSNAME_INDHANDLER_CIMXML,
     _buildFilterOrHandlerPath (PEGASUS_CLASSNAME_INDHANDLER_CIMXML,
                                handler));
  _addUint16Property (subscription01, "OnFatalErrorPolicy", 2);
  _addStringProperty (subscription01, "OtherOnFatalErrorPolicy",
                      String::EMPTY, true);
  _addUint64Property (subscription01, "FailureTriggerTimeInterval", 60);
  _addUint16Property (subscription01, "SubscriptionState", 2);
  _addStringProperty (subscription01, "OtherSubscriptionState", String::EMPTY,
                      true);
  _addUint64Property (subscription01, "SubscriptionDuration",
                      PEGASUS_UINT64_LITERAL (60000000000));
  _addUint16Property (subscription01, "RepeatNotificationPolicy", 1);
  _addStringProperty (subscription01, "OtherRepeatNotificationPolicy",
                      "another policy");
  _addUint64Property (subscription01, "RepeatNotificationInterval", 60);
  _addUint64Property (subscription01, "RepeatNotificationGap", 30);
  _addUint16Property (subscription01, "RepeatNotificationCount", 5);
  path = client.createInstance (PEGASUS_NAMESPACENAME_INTEROP, subscription01);

  _checkSubscriptionPath (path, filter,
                          PEGASUS_CLASSNAME_INDHANDLER_CIMXML, handler);
  retrievedInstance = client.getInstance (PEGASUS_NAMESPACENAME_INTEROP, path);
  _checkUint16Property (retrievedInstance, "OnFatalErrorPolicy", 2);
  _checkStringProperty (retrievedInstance, "OtherOnFatalErrorPolicy",
                        String::EMPTY, true);
  _checkUint64Property (retrievedInstance, "FailureTriggerTimeInterval", 60);
  _checkUint16Property (retrievedInstance, "SubscriptionState", 2);
  _checkStringProperty (retrievedInstance, "OtherSubscriptionState",
                        String::EMPTY, true);
  _checkUint64Property (retrievedInstance, "SubscriptionDuration",
                        PEGASUS_UINT64_LITERAL (60000000000));
  _checkUint16Property (retrievedInstance, "RepeatNotificationPolicy", 1);
  _checkStringProperty (retrievedInstance, "OtherRepeatNotificationPolicy",
                        "another policy");
  _checkUint64Property (retrievedInstance, "RepeatNotificationInterval", 60);
  _checkUint64Property (retrievedInstance, "RepeatNotificationGap", 30);
  _checkUint16Property (retrievedInstance, "RepeatNotificationCount", 5);
}

void
_delete (CIMClient & client, String & filter, String & handler)
{
  //
  //  Delete subscription instances
  //
  _deleteSubscriptionInstance (client, filter,
                               PEGASUS_CLASSNAME_INDHANDLER_CIMXML, handler);

  //
  //  Delete handler instances
  //
  _deleteHandlerInstance (client, PEGASUS_CLASSNAME_INDHANDLER_CIMXML,
                          handler);
  //
  //  Delete filter instances
  //
  _deleteFilterInstance (client, filter);
}

void
_executeMethod (CIMClient & client)
{
  CIMObjectPath instanceName;

  instanceName.setNameSpace (SOURCENAMESPACE);
  instanceName.setClassName ("CMPI_RT_SampleIndication");

  Array < CIMParamValue > inParams;
  Array < CIMParamValue > outParams;

  /*     Uint32 SendTestIndication(); */
  CIMValue retValue = client.invokeMethod (SOURCENAMESPACE,
                                           instanceName,
                                           "SendTestIndication",
                                           inParams,
                                           outParams);

  _checkUint32Value (retValue, 0);
}

void
_test (CIMClient & client)
{
  try
  {
    if (verbose)
      {
        cerr << " Creating filter " << _filter << endl;
        cerr << " with query " << _query << endl;
      }
    _addFilter (client, _filter, _query, _lang);

    if (verbose)
      {
        cerr << " Creating handler " << _handler << endl;
        cerr << " with destination " << _destination << endl;
      }
    _addHandler (client, _handler, _destination);

    if (verbose)
      {
        cerr << " Creating subscription with filter " << _filter <<
          " and handler " << _handler << endl;
      }
    _addSubscription (client, _filter, _handler);

    // Call the method.
    if (verbose)
      {
        cerr << " Executing method" << endl;
      }
    _executeMethod (client);
    if (verbose)
      {
        cerr << " Deleting filter, handler, and subscription." << endl;
      }
    _delete (client, _filter, _handler);

  }
  catch (const Exception & e)
  {
    cerr << e.getMessage () << endl;
    exit (1);
  }

  cout << "+++++ test completed successfully" << endl;
}

void
_cleanup (CIMClient & client)
{

  if (verbose)
    {
      cerr << " Deleting filter " << _filter << endl;
      cerr << " Deleting handler" << _handler << endl;
    }
  //
  //  Delete subscription instances
  //
  try
  {
    _deleteSubscriptionInstance (client, _filter,
                                 PEGASUS_CLASSNAME_INDHANDLER_CIMXML,
                                 _handler);
  }
  catch (...)
  {
  }
  //
  //  Delete handler instances
  //
  try
  {
    _deleteHandlerInstance (client, PEGASUS_CLASSNAME_INDHANDLER_CIMXML,
                            _handler);
  }
  catch (...)
  {
  }
  //
  //  Delete filter instances
  //
  try
  {
    _deleteFilterInstance (client, _filter);
  }
  catch (...)
  {
  }
  cout << "+++++ cleanup completed successfully" << endl;
}


int
main (int argc, char **argv)
{
  verbose = (getenv ("PEGASUS_TEST_VERBOSE")) ? true : false;
  CIMClient client;
  try
  {
    client.connectLocal ();
    client.setTimeout (400000);
  }
  catch (const Exception & e)
  {
    cerr << e.getMessage () << endl;
    return -1;
  }

  if (argc != 2)
    {
      _usage ();
      return 1;
    }

  else
    {
      const char *opt = argv[1];
      if (String::equalNoCase (opt, "test"))

        {
          _test (client);
        }
      else if (String::equalNoCase (opt, "cleanup"))
        {
          _cleanup (client);
        }
      else
        {
          cerr << "Invalid option: " << opt << endl;
          _usage ();
          return -1;
        }
    }

  return 0;
}
