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

const CIMNamespaceName SOURCENAMESPACE =
CIMNamespaceName ("test/TestProvider");

const char *queries_CQL[] = {
    "SELECT * FROM TestCMPI_Indication WHERE "
        " s='s' AND c='c' OR n32>=32 OR n16<=17 OR n64>23 OR r32>=1.0"
        " OR r64=1.2 or b=1 AND s8>1 AND s16>16 AND s32>32 AND s64>64",
    "SELECT * FROM TestCMPI_Indication WHERE "
        "(PropertyA='PropertyA' OR PropertyB='Nothing') AND s='s'",
    "SELECT PropertyA, PropertyB FROM TestCMPI_Indication WHERE "
        "PropertyA='PropertyA' OR PropertyB='Nothing' OR s='s'",
    "SELECT s FROM TestCMPI_Indication WHERE "
        "NOT (PropertyA='PropertyA' OR PropertyB='Nothing') "
        "AND PropertyA='AccessorPropertyA'",
    "SELECT * FROM TestCMPI_Indication WHERE (s IS NOT NULL) OR (c IS NULL)",
    "SELECT n32,r64 FROM TestCMPI_Indication WHERE n32=42 AND r64>=2.2",
    "SELECT n8,n16,n32,n64 FROM TestCMPI_Indication WHERE"
        " n8=8 AND n16=16 AND n32=32 AND n64>32",
    "SELECT s8,s16,s32,s64 FROM TestCMPI_Indication WHERE"
        " s8>1 AND s16=256 OR s32>9999 AND NOT (r64 > 2.2) AND NOT (s16>=288)"
        " AND NOT (s IS NULL) AND NOT (c IS NOT NULL)",
    "SELECT c,b FROM TestCMPI_Indication WHERE "
        " c=c OR b=1 AND PropertyA LIKE 'Acc' AND NOT (PropertyB LIKE 'Acc')",
    "SELECT r32,r64 FROM TestCMPI_Indication WHERE "
        "NOT (s8<3) AND r32<1.23 AND NOT(n16<=17) OR r64=3.14 ",
    "SELECT * FROM TestCMPI_Indication",
};

const char *queries_WQL[] = {
    "SELECT *  FROM TestCMPI_Indication WHERE s IS NOT NULL",
    "SELECT * FROM TestCMPI_Indication",
    "SELECT * FROM TestCMPI_Indication WHERE (s IS NOT NULL) OR (c IS NULL)",
    "SELECT n32,r64 FROM TestCMPI_Indication WHERE n32=42 AND r64>=2.2",
    "SELECT c,b FROM TestCMPI_Indication WHERE c=c OR b=true",
    "SELECT * FROM TestCMPI_Indication WHERE b!=true AND c=c",
    "SELECT n8,n16,n32,n64 FROM TestCMPI_Indication WHERE "
        "n8=8 AND n16=16 AND n32=32 AND n64>32",
    "SELECT s8,s16,s32,s64 FROM TestCMPI_Indication WHERE "
        "s8>1 AND s16=256 OR s32>9999 or s64>=2232",
    "SELECT r32,r64 FROM TestCMPI_Indication WHERE r32<=1.23 OR r64=3.14",
    "SELECT s FROM TestCMPI_Indication WHERE "
        "NOT NOT (PropertyA=\"PropertyA\" OR PropertyB=\"Nothing\") AND "
        "PropertyA=\"AccessorPropertyA\"",
    "SELECT s FROM TestCMPI_Indication WHERE "
        "NOT (PropertyA=\"PropertyA\" OR PropertyB=\"Nothing\") AND "
        "(PropertyA=\"AccessorPropertyA\" OR PropertyB=\"Nothing\")",
    "SELECT s FROM TestCMPI_Indication WHERE "
        "NOT (PropertyA!=\"PropertyA\") AND NOT (n32<42) AND NOT (r64>64) AND "
        "NOT (r32<=1.23) AND NOT NOT NOT(r64 >= 20)",
    "SELECT * FROM TestCMPI_Indication WHERE "
        "PropertyA<PropertyB AND PropertyB<PropertyA",
    "SELECT s FROM TestCMPI_Indication WHERE "
        "NOT (PropertyB=\"Nothing\" AND PropertyA=\"AccessorPropertyA\")",
    "SELECT PropertyA FROM TestCMPI_Indication WHERE NOT "
        "(PropertyB=\"Nothing\" AND PropertyA=\"AccessorPropertyA\" "
        "OR n32=42)",
    "SELECT * FROM TestCMPI_Indication WHERE "
        "s=\"s\" AND c=\"c\" OR n32>=32 OR n16<=17 OR n64>23 OR r32>=1.0 OR "
        "r64=1.2 or b=1 AND s8>1 AND s16>16 AND s32>32 AND s64>64",
    "SELECT * FROM TestCMPI_Indication WHERE "
        "(PropertyA=\"PropertyA\" OR PropertyB=\"Nothing\" OR n64>2) AND "
        "s=\"s\"",
    "SELECT PropertyA, PropertyB FROM TestCMPI_Indication WHERE "
        "PropertyA=\"PropertyA\" OR PropertyB=\"Nothing\" OR s=\"s\"",
    "SELECT s FROM TestCMPI_Indication WHERE "
        "NOT (PropertyA=\"PropertyA\" OR PropertyB=\"Nothing\") AND "
        "PropertyA=\"AccessorPropertyA\"",
};

#define QUERIES_CQL 11
#define QUERIES_WQL 19
String _filter ("TestCMPI_Indication_Filter_");
String _handler ("TestCMPI_Indication_Handler_");
String _destination ("localhost/CIMListener/Pegasus_SimpleDisplayConsumer");

Boolean verbose;

void _addStringProperty
  (CIMInstance & instance,
   const String & name,
   const String & value, Boolean null = false)
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
   const String & name,
   Uint16 value, Boolean null = false)
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
   const String & name,
   Uint64 value, Boolean null = false)
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
    PEGASUS_TEST_ASSERT (
        path.getClassName () == PEGASUS_CLASSNAME_INDSUBSCRIPTION);
    Array < CIMKeyBinding > keyBindings = path.getKeyBindings ();
    Boolean filterFound = false;
    Boolean handlerFound = false;
    for (Uint32 i = 0; i < keyBindings.size (); i++)
    {
        if (keyBindings[i].getName ().equal ("Filter"))
        {
            filterFound = true;
            CIMObjectPath filterPath = _buildFilterOrHandlerPath(
                                           PEGASUS_CLASSNAME_INDFILTER,
                                           filterName);
            PEGASUS_TEST_ASSERT (
                keyBindings[i].getValue () == filterPath.toString ());
        }
        else if (keyBindings[i].getName ().equal ("Handler"))
        {
            handlerFound = true;
            CIMObjectPath handlerPath = _buildFilterOrHandlerPath (
                                            handlerClass,
                                            handlerName);
            PEGASUS_TEST_ASSERT (
                keyBindings[i].getValue () == handlerPath.toString ());
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
      char buffer[32];    // Should need 21 chars max
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

void _addSubscription (CIMClient & client, String & filter, String & handler)
{
    CIMObjectPath path;
    CIMInstance retrievedInstance;
    //
    //  Create subscriptions
    //
    CIMInstance subscription01 = _buildSubscriptionInstance (
        _buildFilterOrHandlerPath (PEGASUS_CLASSNAME_INDFILTER, filter),
        PEGASUS_CLASSNAME_INDHANDLER_CIMXML,
        _buildFilterOrHandlerPath (
            PEGASUS_CLASSNAME_INDHANDLER_CIMXML,
            handler) );
    _addUint16Property (subscription01, "OnFatalErrorPolicy", 2);
    _addStringProperty (
        subscription01,
        "OtherOnFatalErrorPolicy",
        String::EMPTY,
        true);
    _addUint64Property (subscription01, "FailureTriggerTimeInterval", 60);
    _addUint16Property (subscription01, "SubscriptionState", 2);
    _addStringProperty (
        subscription01,
        "OtherSubscriptionState",
        String::EMPTY,
        true);
    _addUint64Property (
        subscription01,
        "SubscriptionDuration",
        PEGASUS_UINT64_LITERAL (60000000000));
    _addUint16Property (subscription01, "RepeatNotificationPolicy", 1);
    _addStringProperty (
        subscription01,
        "OtherRepeatNotificationPolicy",
        "another policy");
    _addUint64Property (subscription01, "RepeatNotificationInterval", 60);
    _addUint64Property (subscription01, "RepeatNotificationGap", 30);
    _addUint16Property (subscription01, "RepeatNotificationCount", 5);
    path = client.createInstance (
               PEGASUS_NAMESPACENAME_INTEROP,
               subscription01);
    _checkSubscriptionPath (
        path,
        filter,
        PEGASUS_CLASSNAME_INDHANDLER_CIMXML,
        handler);
    retrievedInstance = client.getInstance (
                            PEGASUS_NAMESPACENAME_INTEROP,
                            path);
    _checkUint16Property (retrievedInstance, "OnFatalErrorPolicy", 2);
    _checkStringProperty (
        retrievedInstance,
        "OtherOnFatalErrorPolicy",
        String::EMPTY,
        true);
    _checkUint64Property (retrievedInstance, "FailureTriggerTimeInterval", 60);
    _checkUint16Property (retrievedInstance, "SubscriptionState", 2);
    _checkStringProperty (
        retrievedInstance,
        "OtherSubscriptionState",
        String::EMPTY,
        true);
    _checkUint64Property (
        retrievedInstance,
        "SubscriptionDuration",
        PEGASUS_UINT64_LITERAL (60000000000));
    _checkUint16Property (retrievedInstance, "RepeatNotificationPolicy", 1);
    _checkStringProperty (
        retrievedInstance,
        "OtherRepeatNotificationPolicy",
        "another policy");
    _checkUint64Property (retrievedInstance, "RepeatNotificationInterval", 60);
    _checkUint64Property (retrievedInstance, "RepeatNotificationGap", 30);
    _checkUint16Property (retrievedInstance, "RepeatNotificationCount", 5);
}

void _delete (CIMClient & client, String & filter, String & handler)
{
  //
  //  Delete subscription instances
  //
    _deleteSubscriptionInstance (
        client,
        filter,
        PEGASUS_CLASSNAME_INDHANDLER_CIMXML,
        handler);

  //
  //  Delete handler instances
  //
  _deleteHandlerInstance (client, PEGASUS_CLASSNAME_INDHANDLER_CIMXML,handler);
  //
  //  Delete filter instances
  //
  _deleteFilterInstance (client, filter);
}

void _test (CIMClient & client)
{
    try
    {
        String cql ("DMTF:CQL");
        String wql ("WQL");
        char cnt[512];
        String filter;
        String query;
        String handler;
        for (Uint32 i = 0; i < QUERIES_CQL; i++)
        {
            filter.clear ();
            sprintf (cnt, "%u", i);
            filter.append (_filter);
            filter.append (cnt);

            handler.clear ();
            handler.append (_handler);
            handler.append (cnt);

            query.clear ();
            query.append (queries_CQL[i]);

            if (verbose)
            {
                cerr << " Creating filter " << filter << endl;
                cerr << " with query: " << query << endl;
                cerr << " and handler" << handler << endl;
            }

            // Do the CQL first
            _addFilter (client, filter, query, cql);
            _addHandler (client, handler, _destination);
            _addSubscription (client, filter, handler);
            _delete (client, filter, handler);

        }

        //WQL Queries
        for (Uint32 i = 0; i < QUERIES_WQL; i++)
        {
            filter.clear ();
            sprintf (cnt, "%u", i);
            filter.append (_filter);
            filter.append (cnt);

            handler.clear ();
            handler.append (_handler);
            handler.append (cnt);

            query.clear ();
            query.append (queries_WQL[i]);

            if (verbose)
            {
                cerr << " Creating filter " << filter << endl;
                cerr << " with query: " << query << endl;
                cerr << " and handler" << handler << endl;
            }

            _addFilter (client, filter, query, wql);
            _addHandler (client, handler, _destination);
            _addSubscription (client, filter, handler);
            _delete (client, filter, handler);
        }

    } catch (const Exception &e)
    {
        cerr << e.getMessage() << endl;
        exit(1);
    }

    cout << "+++++ test completed successfully" << endl;
}

void _cleanup (CIMClient & client)
{
    char cnt[512];
    String filter;
    String handler;

    for (Uint32 i = 0; i < QUERIES_CQL; i++)
    {
        sprintf (cnt, "%u", i);
        filter.clear ();
        filter.append (_filter);
        filter.append (cnt);
        handler.clear ();
        handler.append (_handler);
        handler.append (cnt);

        if (verbose)
        {
            cerr << " Deleting filter " << filter << endl;
            cerr << " Deleting handler" << handler << endl;
        }
        //
        //  Delete subscription instances
        //
        try
        {
                _deleteSubscriptionInstance (
                    client,
                    filter,
                    PEGASUS_CLASSNAME_INDHANDLER_CIMXML,
                    handler);
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
                                  handler);
      }
      catch (...)
      {
      }
      //
      //  Delete filter instances
      //
      try
      {
          _deleteFilterInstance (client, filter);
      }
      catch (...)
      {
      }
    }
  cout << "+++++ cleanup completed successfully" << endl;
}


int main (int argc, char **argv)
{
    verbose = (getenv ("PEGASUS_TEST_VERBOSE")) ? true : false;
    CIMClient client;
    try
    {
        client.connectLocal ();
        client.setTimeout(400000);
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
