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
// Author: Carol Ann Krug Graves, Hewlett-Packard Company
//             (carolann_graves@hp.com)
//
//%/////////////////////////////////////////////////////////////////////////////

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/Constants.h>
#include <Pegasus/Common/InternalException.h>
#include <Pegasus/Common/System.h>
#include <Pegasus/Client/CIMClient.h>

PEGASUS_USING_PEGASUS;
PEGASUS_USING_STD;

// Interop namespace used with PEGASUS_NAMESPACENAME_INTEROP in Constants.h
const CIMNamespaceName NAMESPACE1 = CIMNamespaceName("root/cimv2");
const CIMNamespaceName NAMESPACE2 = CIMNamespaceName("test/TestProvider");
const CIMNamespaceName NAMESPACE3 = CIMNamespaceName("root/SampleProvider");
const CIMNamespaceName SOURCENAMESPACE = CIMNamespaceName("test/TestProvider");

void _createHandlerInstance
    (CIMClient & client,
     const String & name,
     const String & destination,
     const CIMNamespaceName & handlerNS)
{
    CIMInstance handlerInstance (PEGASUS_CLASSNAME_INDHANDLER_CIMXML);
    handlerInstance.addProperty (CIMProperty (CIMName
        ("SystemCreationClassName"), System::getSystemCreationClassName ()));
    handlerInstance.addProperty (CIMProperty (CIMName ("SystemName"),
        System::getFullyQualifiedHostName ()));
    handlerInstance.addProperty (CIMProperty (CIMName ("CreationClassName"),
        PEGASUS_CLASSNAME_INDHANDLER_CIMXML.getString ()));
    handlerInstance.addProperty (CIMProperty (CIMName ("Name"), name));
    handlerInstance.addProperty (CIMProperty (CIMName ("Destination"),
        destination));

    CIMObjectPath path = client.createInstance (handlerNS, handlerInstance);
}

void _createFilterInstance
    (CIMClient & client,
     const String & name,
     const String & query,
     const String & qlang,
     const CIMNamespaceName & filterNS)
{
    CIMInstance filterInstance (PEGASUS_CLASSNAME_INDFILTER);
    filterInstance.addProperty (CIMProperty (CIMName
        ("SystemCreationClassName"), System::getSystemCreationClassName ()));
    filterInstance.addProperty (CIMProperty (CIMName ("SystemName"),
        System::getFullyQualifiedHostName ()));
    filterInstance.addProperty (CIMProperty (CIMName ("CreationClassName"),
        PEGASUS_CLASSNAME_INDFILTER.getString ()));
    filterInstance.addProperty (CIMProperty (CIMName ("Name"), name));
    filterInstance.addProperty (CIMProperty (CIMName ("Query"), query));
    filterInstance.addProperty (CIMProperty (CIMName ("QueryLanguage"),
        String (qlang)));
    filterInstance.addProperty (CIMProperty (CIMName ("SourceNamespace"),
        SOURCENAMESPACE.getString ()));

    CIMObjectPath path = client.createInstance (filterNS, filterInstance);
}

void _createSubscriptionInstance
    (CIMClient & client,
     const CIMObjectPath & filterPath,
     const CIMObjectPath & handlerPath,
     const CIMNamespaceName & subscriptionNS)
{
    CIMInstance subscriptionInstance (PEGASUS_CLASSNAME_INDSUBSCRIPTION);
    subscriptionInstance.addProperty (CIMProperty (CIMName ("Filter"),
        filterPath, 0, PEGASUS_CLASSNAME_INDFILTER));
    subscriptionInstance.addProperty (CIMProperty (CIMName ("Handler"),
        handlerPath, 0, PEGASUS_CLASSNAME_INDHANDLER_CIMXML));
    subscriptionInstance.addProperty (CIMProperty
        (CIMName ("SubscriptionState"), CIMValue ((Uint16) 2)));

    CIMObjectPath path = client.createInstance (subscriptionNS,
        subscriptionInstance);
}

void _sendIndicationShouldSucceed
    (CIMClient & client)
{
    Array <CIMParamValue> inParams;
    Array <CIMParamValue> outParams;
    Array <CIMKeyBinding> keyBindings;
    Sint32 result;

    CIMName methodName ("SendTestIndication");
    CIMObjectPath className (String::EMPTY, CIMNamespaceName (),
        CIMName("Test_IndicationProviderClass"), keyBindings);

    CIMValue retValue = client.invokeMethod
        (SOURCENAMESPACE,
        className,
        methodName,
        inParams,
        outParams);
    retValue.get (result);
    PEGASUS_TEST_ASSERT (result == 0);
}

void _sendIndicationShouldFail
    (CIMClient & client)
{
    Array <CIMParamValue> inParams;
    Array <CIMParamValue> outParams;
    Array <CIMKeyBinding> keyBindings;
    Sint32 result;

    CIMName methodName ("SendTestIndication");
    CIMObjectPath className (String::EMPTY, CIMNamespaceName (),
        CIMName("Test_IndicationProviderClass"), keyBindings);

    CIMValue retValue = client.invokeMethod
        (SOURCENAMESPACE,
        className,
        methodName,
        inParams,
        outParams);
    retValue.get (result);
    PEGASUS_TEST_ASSERT (result == 1);
}

void _sendIndicationShouldBeBlocked
    (CIMClient & client)
{
    Array <CIMParamValue> inParams;
    Array <CIMParamValue> outParams;
    Array <CIMKeyBinding> keyBindings;
    Sint32 result;

    CIMName methodName ("SendTestIndication");
    CIMObjectPath className (String::EMPTY, CIMNamespaceName (),
        CIMName("Test_IndicationProviderClass"), keyBindings);

    try
    {
        CIMValue retValue = client.invokeMethod
            (SOURCENAMESPACE,
            className,
            methodName,
            inParams,
            outParams);
        retValue.get (result);
        PEGASUS_TEST_ASSERT (false);
    }
    catch (CIMException & e)
    {
        PEGASUS_TEST_ASSERT (e.getCode () == CIM_ERR_NOT_SUPPORTED);
    }
}

void _deleteSubscriptionInstance
    (CIMClient & client,
     const String & filterName,
     const String & handlerName,
     const CIMNamespaceName & filterNS,
     const CIMNamespaceName & handlerNS,
     const CIMNamespaceName & subscriptionNS)
{
    Array<CIMKeyBinding> filterKeyBindings;
    filterKeyBindings.append (CIMKeyBinding ("SystemCreationClassName",
        System::getSystemCreationClassName (), CIMKeyBinding::STRING));
    filterKeyBindings.append (CIMKeyBinding ("SystemName",
        System::getFullyQualifiedHostName (), CIMKeyBinding::STRING));
    filterKeyBindings.append (CIMKeyBinding ("CreationClassName",
        PEGASUS_CLASSNAME_INDFILTER.getString(), CIMKeyBinding::STRING));
    filterKeyBindings.append (CIMKeyBinding ("Name", filterName,
        CIMKeyBinding::STRING));
    CIMObjectPath filterPath ("", filterNS,
        PEGASUS_CLASSNAME_INDFILTER, filterKeyBindings);

    Array<CIMKeyBinding> handlerKeyBindings;
    handlerKeyBindings.append (CIMKeyBinding ("SystemCreationClassName",
        System::getSystemCreationClassName (), CIMKeyBinding::STRING));
    handlerKeyBindings.append (CIMKeyBinding ("SystemName",
        System::getFullyQualifiedHostName (), CIMKeyBinding::STRING));
    handlerKeyBindings.append (CIMKeyBinding ("CreationClassName",
        PEGASUS_CLASSNAME_INDHANDLER_CIMXML.getString(),
        CIMKeyBinding::STRING));
    handlerKeyBindings.append (CIMKeyBinding ("Name", handlerName,
        CIMKeyBinding::STRING));
    CIMObjectPath handlerPath ("", handlerNS,
        PEGASUS_CLASSNAME_INDHANDLER_CIMXML, handlerKeyBindings);

    Array<CIMKeyBinding> subscriptionKeyBindings;
    subscriptionKeyBindings.append (CIMKeyBinding ("Filter",
        filterPath.toString (), CIMKeyBinding::REFERENCE));
    subscriptionKeyBindings.append (CIMKeyBinding ("Handler",
        handlerPath.toString (), CIMKeyBinding::REFERENCE));
    CIMObjectPath subscriptionPath ("", CIMNamespaceName (),
        PEGASUS_CLASSNAME_INDSUBSCRIPTION, subscriptionKeyBindings);
    client.deleteInstance (subscriptionNS, subscriptionPath);
}

void _deleteSubscriptionInstance
    (CIMClient & client,
     const String & filterName,
     const String & handlerName)
{
    _deleteSubscriptionInstance (client, filterName, handlerName,
        CIMNamespaceName (), CIMNamespaceName (),
        PEGASUS_NAMESPACENAME_INTEROP);
}

void _deleteHandlerInstance
    (CIMClient & client,
     const String & name,
     const CIMNamespaceName & handlerNS)
{
    Array<CIMKeyBinding> keyBindings;
    keyBindings.append (CIMKeyBinding ("SystemCreationClassName",
        System::getSystemCreationClassName (), CIMKeyBinding::STRING));
    keyBindings.append (CIMKeyBinding ("SystemName",
        System::getFullyQualifiedHostName (), CIMKeyBinding::STRING));
    keyBindings.append (CIMKeyBinding ("CreationClassName",
        PEGASUS_CLASSNAME_INDHANDLER_CIMXML.getString(),
        CIMKeyBinding::STRING));
    keyBindings.append (CIMKeyBinding ("Name", name,
        CIMKeyBinding::STRING));
    CIMObjectPath path ("", CIMNamespaceName (),
        PEGASUS_CLASSNAME_INDHANDLER_CIMXML, keyBindings);
    client.deleteInstance (handlerNS, path);
}

void _deleteFilterInstance
    (CIMClient & client,
     const String & name,
     const CIMNamespaceName & filterNS)
{
    Array<CIMKeyBinding> keyBindings;
    keyBindings.append (CIMKeyBinding ("SystemCreationClassName",
        System::getSystemCreationClassName (), CIMKeyBinding::STRING));
    keyBindings.append (CIMKeyBinding ("SystemName",
        System::getFullyQualifiedHostName (), CIMKeyBinding::STRING));
    keyBindings.append (CIMKeyBinding ("CreationClassName",
        PEGASUS_CLASSNAME_INDFILTER.getString(), CIMKeyBinding::STRING));
    keyBindings.append (CIMKeyBinding ("Name", name,
        CIMKeyBinding::STRING));
    CIMObjectPath path ("", CIMNamespaceName (),
        PEGASUS_CLASSNAME_INDFILTER, keyBindings);
    client.deleteInstance (filterNS, path);
}

void _usage ()
{
    PEGASUS_STD (cerr)
        << "Usage: TestDisableEnable2 "
        << "{setup | setup2 | create | create2 "
        << "| sendSucceed | sendFail | sendBlock "
        << "| delete | delete2 | cleanup | cleanup2} {WQL | DMTF:CQL}"
        << PEGASUS_STD (endl);
}

void _setup (CIMClient & client, String& qlang)
{
    try
    {
        _createFilterInstance (client, String ("DEFilter01"),
            String ("SELECT MethodName FROM Test_IndicationProviderClass"),
            qlang, PEGASUS_NAMESPACENAME_INTEROP);
        _createHandlerInstance (client, String ("DEHandler01"),
            String ("localhost/CIMListener/Pegasus_SimpleDisplayConsumer"),
            PEGASUS_NAMESPACENAME_INTEROP);
    }
    catch (Exception & e)
    {
        PEGASUS_STD (cerr) << "setup failed: " << e.getMessage ()
                           << PEGASUS_STD (endl);
        exit (-1);
    }

    PEGASUS_STD (cout) << "+++++ setup completed successfully"
                       << PEGASUS_STD (endl);
}

void _setup2 (CIMClient & client, String& qlang)
{
    try
    {
        _createFilterInstance (client, String ("DEFilter02"),
            String ("SELECT MethodName FROM Test_IndicationProviderClass"),
            qlang, NAMESPACE1);
        _createHandlerInstance (client, String ("DEHandler02"),
            String ("localhost/CIMListener/Pegasus_SimpleDisplayConsumer"),
            NAMESPACE2);
    }
    catch (Exception & e)
    {
        PEGASUS_STD (cerr) << "setup2 failed: " << e.getMessage ()
                           << PEGASUS_STD (endl);
        exit (-1);
    }

    PEGASUS_STD (cout) << "+++++ setup2 completed successfully"
                       << PEGASUS_STD (endl);
}

CIMObjectPath _buildFilterOrHandlerPath
    (const CIMName & className,
     const String & name,
     const CIMNamespaceName & namespaceName = CIMNamespaceName ())
{
    CIMObjectPath path;

    Array <CIMKeyBinding> keyBindings;
    keyBindings.append (CIMKeyBinding ("SystemCreationClassName",
        System::getSystemCreationClassName (), CIMKeyBinding::STRING));
    keyBindings.append (CIMKeyBinding ("SystemName",
        System::getFullyQualifiedHostName (), CIMKeyBinding::STRING));
    keyBindings.append (CIMKeyBinding ("CreationClassName",
        className.getString(), CIMKeyBinding::STRING));
    keyBindings.append (CIMKeyBinding ("Name", name, CIMKeyBinding::STRING));
    path.setClassName (className);
    path.setKeyBindings (keyBindings);
    path.setNameSpace (namespaceName);

    return path;
}

void _create (CIMClient & client)
{
    try
    {
        _createSubscriptionInstance (client,
            _buildFilterOrHandlerPath (PEGASUS_CLASSNAME_INDFILTER,
                "DEFilter01"),
            _buildFilterOrHandlerPath (PEGASUS_CLASSNAME_INDHANDLER_CIMXML,
                "DEHandler01"), PEGASUS_NAMESPACENAME_INTEROP);
    }
    catch (Exception & e)
    {
        PEGASUS_STD (cerr) << "create failed: " << e.getMessage ()
                           << PEGASUS_STD (endl);
        exit (-1);
    }

    PEGASUS_STD (cout) << "+++++ create completed successfully"
                       << PEGASUS_STD (endl);
}

void _create2 (CIMClient & client)
{
    try
    {
        _createSubscriptionInstance (client,
            _buildFilterOrHandlerPath (PEGASUS_CLASSNAME_INDFILTER,
                "DEFilter02", NAMESPACE1),
            _buildFilterOrHandlerPath (PEGASUS_CLASSNAME_INDHANDLER_CIMXML,
                "DEHandler02", NAMESPACE2), NAMESPACE3);
    }
    catch (Exception & e)
    {
        PEGASUS_STD (cerr) << "create2 failed: " << e.getMessage ()
                           << PEGASUS_STD (endl);
        exit (-1);
    }

    PEGASUS_STD (cout) << "+++++ create2 completed successfully"
                       << PEGASUS_STD (endl);
}

void _sendSucceed (CIMClient & client)
{
    try
    {
        _sendIndicationShouldSucceed (client);
    }
    catch (Exception & e)
    {
        PEGASUS_STD (cerr) << "sendSucceed failed: " << e.getMessage ()
                           << PEGASUS_STD (endl);
        exit (-1);
    }

    PEGASUS_STD (cout) << "+++++ sendSucceed completed successfully"
                       << PEGASUS_STD (endl);
}

void _sendFail (CIMClient & client)
{
    try
    {
        _sendIndicationShouldFail (client);
    }
    catch (Exception & e)
    {
        PEGASUS_STD (cerr) << "sendFail failed: " << e.getMessage ()
                           << PEGASUS_STD (endl);
        exit (-1);
    }

    PEGASUS_STD (cout) << "+++++ sendFail completed successfully"
                       << PEGASUS_STD (endl);
}

void _sendBlock (CIMClient & client)
{
    try
    {
        _sendIndicationShouldBeBlocked (client);
    }
    catch (Exception & e)
    {
        PEGASUS_STD (cerr) << "sendBlock failed: " << e.getMessage ()
                           << PEGASUS_STD (endl);
        exit (-1);
    }

    PEGASUS_STD (cout) << "+++++ sendBlock completed successfully"
                       << PEGASUS_STD (endl);
}

void _delete (CIMClient & client)
{
    try
    {
        _deleteSubscriptionInstance (client, String ("DEFilter01"),
            String ("DEHandler01"));
    }
    catch (Exception & e)
    {
        PEGASUS_STD (cerr) << "delete failed: " << e.getMessage ()
                           << PEGASUS_STD (endl);
        exit (-1);
    }

    PEGASUS_STD (cout) << "+++++ delete completed successfully"
                       << PEGASUS_STD (endl);
}

void _delete2 (CIMClient & client)
{
    try
    {
        _deleteSubscriptionInstance (client, String ("DEFilter02"),
            String ("DEHandler02"), NAMESPACE1, NAMESPACE2, NAMESPACE3);
    }
    catch (Exception & e)
    {
        PEGASUS_STD (cerr) << "delete2 failed: " << e.getMessage ()
                           << PEGASUS_STD (endl);
        exit (-1);
    }

    PEGASUS_STD (cout) << "+++++ delete2 completed successfully"
                       << PEGASUS_STD (endl);
}

void _cleanup (CIMClient & client)
{
    try
    {
        _deleteHandlerInstance (client, String ("DEHandler01"),
                                PEGASUS_NAMESPACENAME_INTEROP);
        _deleteFilterInstance (client, String ("DEFilter01"),
                               PEGASUS_NAMESPACENAME_INTEROP);
    }
    catch (Exception & e)
    {
        PEGASUS_STD (cerr) << "cleanup failed: " << e.getMessage ()
                           << PEGASUS_STD (endl);
        exit (-1);
    }

    PEGASUS_STD (cout) << "+++++ cleanup completed successfully"
                       << PEGASUS_STD (endl);
}

void _cleanup2 (CIMClient & client)
{
    try
    {
        _deleteHandlerInstance (client, String ("DEHandler02"), NAMESPACE2);
        _deleteFilterInstance (client, String ("DEFilter02"), NAMESPACE1);
    }
    catch (Exception & e)
    {
        PEGASUS_STD (cerr) << "cleanup2 failed: " << e.getMessage ()
                           << PEGASUS_STD (endl);
        exit (-1);
    }

    PEGASUS_STD (cout) << "+++++ cleanup2 completed successfully"
                       << PEGASUS_STD (endl);
}

int _test(CIMClient& client, const char* opt, String& qlang)
{
  if (String::equalNoCase (opt, "setup"))
  {
    _setup (client, qlang);
  }
  else if (String::equalNoCase (opt, "setup2"))
  {
    _setup2 (client, qlang);
  }
  else if (String::equalNoCase (opt, "create"))
  {
    _create (client);
  }
  else if (String::equalNoCase (opt, "create2"))
  {
    _create2 (client);
  }
  else if (String::equalNoCase (opt, "sendSucceed"))
  {
    _sendSucceed (client);
  }
  else if (String::equalNoCase (opt, "sendFail"))
  {
    _sendFail (client);
  }
  else if (String::equalNoCase (opt, "sendBlock"))
  {
    _sendBlock (client);
  }
  else if (String::equalNoCase (opt, "delete"))
  {
    _delete (client);
  }
  else if (String::equalNoCase (opt, "delete2"))
  {
    _delete2 (client);
  }
  else if (String::equalNoCase (opt, "cleanup"))
  {
    _cleanup (client);
  }
  else if (String::equalNoCase (opt, "cleanup2"))
  {
    _cleanup2 (client);
  }
  else
  {
    PEGASUS_STD (cerr) << "Invalid option: " << opt
                       << PEGASUS_STD (endl);
    _usage ();
    return -1;
  }

  return 0;
}

int main (int argc, char** argv)
{
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

    if (argc != 3)
    {
        _usage ();
        return 1;
    }

    const char* opt = argv[1];
    const char* optLang = argv[2];
    String qlang(optLang);

#ifndef PEGASUS_ENABLE_CQL
    if (qlang == "DMTF:CQL")
    {
        PEGASUS_STD(cout) << "+++++ cql test disabled" << PEGASUS_STD(endl);
        return 0;
    }
#endif

    return _test(client, opt, qlang);
}
