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
//
// This testclient uses the sample Lifecycle Indication provider. It creates a
// couple subscriptions (when "setup" is specified). Then it can be invoked to
// create ("createInstance") or delete ("deleteInstance") instances of the class
// Sample_LifecycleIndicationProviderClass. Each create or delete will result in
// the generation of a Lifecycle Indication (subclass of CIM_InstCreation or
// CIM_InstDeletion, respectively) that is delivered to the Pegasus
// SimpleDisplayConsumer, where all indications since the "setup" was issued
// are written to the file $PEGASUS_HOME/indicationLog.
//
//%/////////////////////////////////////////////////////////////////////////////

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/Constants.h>
#include <Pegasus/Common/System.h>
#include <Pegasus/Common/FileSystem.h>
#include <Pegasus/Common/InternalException.h>
#include <Pegasus/Client/CIMClient.h>

#include <iostream>

PEGASUS_USING_PEGASUS;
PEGASUS_USING_STD;


// Interop namespace used with PEGASUS_NAMESPACENAME_INTEROP in Constants.h
const CIMNamespaceName SOURCE_NAMESPACE =
    CIMNamespaceName ("root/SampleProvider");
const CIMName SAMPLE_CLASSNAME  =
    CIMName ("Sample_LifecycleIndicationProviderClass");

void _renameLogFile()
{
    //
    //  Remove previous indication log file, if there
    //
    String previousIndicationFile, oldIndicationFile;

    previousIndicationFile = INDICATION_DIR;
    previousIndicationFile.append ("/indicationLog");

    if (FileSystem::exists (previousIndicationFile))
    {
        oldIndicationFile = INDICATION_DIR;
        oldIndicationFile.append ("/oldIndicationFile");
        if (FileSystem::exists (oldIndicationFile))
        {
            FileSystem::removeFile (oldIndicationFile);
        }
        if (!FileSystem::renameFile (previousIndicationFile, oldIndicationFile))
        {
            FileSystem::removeFile (previousIndicationFile);
        }
    }
}

void _createInstance(CIMClient & client)
{
    Array<CIMObjectPath> objectNames;
    Uint32 _uniqueId;
    try
    {
        objectNames = client.enumerateInstanceNames(
                        SOURCE_NAMESPACE, SAMPLE_CLASSNAME);

        _uniqueId = objectNames.size() + 1;

        CIMInstance cimInstance(SAMPLE_CLASSNAME);
        cimInstance.addProperty(
            CIMProperty(CIMName ("uniqueId"), Uint32(_uniqueId)));
        cimInstance.addProperty(
                CIMProperty(CIMName ("lastOp"), String("createInstance")));
        CIMObjectPath createdinstanceName =
            client.createInstance(SOURCE_NAMESPACE, cimInstance);
    }
    catch (Exception & e)
    {
        PEGASUS_STD (cerr) << "createInstance failed: " << e.getMessage ()
                           << PEGASUS_STD (endl);
        exit (-1);
    }

    PEGASUS_STD (cout) << "+++++ createInstance completed successfully"
                       << PEGASUS_STD (endl);
}

void _deleteInstance(CIMClient & client)
{
    Array<CIMObjectPath> objectNames;
    Uint32 _uniqueId;
    try
    {
        objectNames = client.enumerateInstanceNames(
                        SOURCE_NAMESPACE, SAMPLE_CLASSNAME);

        _uniqueId = objectNames.size();

        if (_uniqueId == 0)
        {
            PEGASUS_STD (cerr) << "No instances to delete."
                << PEGASUS_STD (endl);
            exit (-1);
        }

        client.deleteInstance (SOURCE_NAMESPACE, objectNames[_uniqueId-1]);
    }
    catch (Exception & e)
    {
        PEGASUS_STD (cerr) << "deleteInstance failed: " << e.getMessage ()
                           << PEGASUS_STD (endl);
        exit (-1);
    }

    PEGASUS_STD (cout) << "+++++ deleteInstance completed successfully"
                       << PEGASUS_STD (endl);
}

void _createHandlerInstance
    (CIMClient & client,
     const String & name,
     const String & destination)
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

    CIMObjectPath path = client.createInstance (
                            PEGASUS_NAMESPACENAME_INTEROP, handlerInstance);
}

void _createFilterInstance
    (CIMClient & client,
     const String & name,
     const String & query,
     const String & qlang)
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
        SOURCE_NAMESPACE.getString ()));

    CIMObjectPath path = client.createInstance (
                            PEGASUS_NAMESPACENAME_INTEROP, filterInstance);
}

void _createSubscriptionInstance
    (CIMClient & client,
     const CIMObjectPath & filterPath,
     const CIMObjectPath & handlerPath)
{
    CIMInstance subscriptionInstance (PEGASUS_CLASSNAME_INDSUBSCRIPTION);
    subscriptionInstance.addProperty (CIMProperty (CIMName ("Filter"),
        filterPath, 0, PEGASUS_CLASSNAME_INDFILTER));
    subscriptionInstance.addProperty (CIMProperty (CIMName ("Handler"),
        handlerPath, 0, PEGASUS_CLASSNAME_INDHANDLER_CIMXML));
    subscriptionInstance.addProperty (CIMProperty
        (CIMName ("SubscriptionState"), CIMValue ((Uint16) 2)));

    CIMObjectPath path = client.createInstance (PEGASUS_NAMESPACENAME_INTEROP,
        subscriptionInstance);
}

void _deleteSubscriptionInstance
    (CIMClient & client,
     const String & filterName,
     const String & handlerName)
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
    CIMObjectPath filterPath ("", CIMNamespaceName (),
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
    CIMObjectPath handlerPath ("", CIMNamespaceName (),
        PEGASUS_CLASSNAME_INDHANDLER_CIMXML, handlerKeyBindings);

    Array<CIMKeyBinding> subscriptionKeyBindings;
    subscriptionKeyBindings.append (CIMKeyBinding ("Filter",
        filterPath.toString (), CIMKeyBinding::REFERENCE));
    subscriptionKeyBindings.append (CIMKeyBinding ("Handler",
        handlerPath.toString (), CIMKeyBinding::REFERENCE));
    CIMObjectPath subscriptionPath ("", CIMNamespaceName (),
        PEGASUS_CLASSNAME_INDSUBSCRIPTION, subscriptionKeyBindings);
    client.deleteInstance (PEGASUS_NAMESPACENAME_INTEROP, subscriptionPath);
}

void _deleteHandlerInstance
    (CIMClient & client,
     const String & name)
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
    client.deleteInstance (PEGASUS_NAMESPACENAME_INTEROP, path);
}

void _deleteFilterInstance
    (CIMClient & client,
     const String & name)
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
    client.deleteInstance (PEGASUS_NAMESPACENAME_INTEROP, path);
}

void _usage ()
{
    PEGASUS_STD (cerr)
        << "Usage:" << PEGASUS_STD (endl)
        << "\tTestLifecycleIndication setup [ WQL | CIM:CQL ]"
        << PEGASUS_STD (endl)
        << "\tTestLifecycleIndication createInstance" << PEGASUS_STD (endl)
        << "\tTestLifecycleIndication deleteInstance" << PEGASUS_STD (endl)
        << "\tTestLifecycleIndication cleanup" << PEGASUS_STD (endl);
}

void _setup (CIMClient & client, String& qlang)
{
    try
    {
        // The first filter is a subscription for the class
        // InstCreation_for_Sample_LifecycleIndicationProviderClass,
        // which is a subclass
        // of CIM_InstCreation. This filter also selects the properties:
        //    IndicationIdentifier, IndicationTime, SourceInstance
        _createFilterInstance (client,
                               String ("LIFilter01"),
                               String ("SELECT IndicationIdentifier, "
                                   "IndicationTime, SourceInstance FROM "
                                   "InstCreation_for_Sample_Lifecycle"
                                   "IndicationProviderClass"),
                               qlang);
    }
    catch (Exception & e)
    {
        PEGASUS_STD (cerr) << "setup 1 failed: " << e.getMessage ()
            << PEGASUS_STD (endl);
    }

    try
    {
        // The second filter is a subscription for the class
        // InstDeletion_for_Sample_LifecycleIndicationProviderClass,
        // which is a subclass
        // of CIM_InstDeletion. This filter also selects the properties:
        //    IndicationIdentifier, SourceInstance
        _createFilterInstance (client,
                               String ("LIFilter02"),
                               String ("SELECT IndicationIdentifier, "
                                   "SourceInstance FROM InstDeletion_for_"
                                   "Sample_LifecycleIndicationProviderClass"),
                               qlang);
    }
    catch (Exception & e)
    {
        PEGASUS_STD (cerr) << "setup 2 failed: " << e.getMessage ()
            << PEGASUS_STD (endl);
    }

    try
    {
        _createHandlerInstance (client,
                                String ("LIHandler01"),
                                String ("localhost/CIMListener/"
                                    "Pegasus_SimpleDisplayConsumer"));
    }
    catch (Exception & e)
    {
        PEGASUS_STD (cerr) << "setup 3 failed: " << e.getMessage ()
            << PEGASUS_STD (endl);
    }

    try
    {
        String filterPathString;
        filterPathString.append ("CIM_IndicationFilter.CreationClassName="
                "\"CIM_IndicationFilter\",Name=\"LIFilter01\","
                "SystemCreationClassName=\"");
        filterPathString.append (System::getSystemCreationClassName ());
        filterPathString.append ("\",SystemName=\"");
        filterPathString.append (System::getFullyQualifiedHostName ());
        filterPathString.append ("\"");
        String handlerPathString;
        handlerPathString.append ("CIM_IndicationHandlerCIMXML."
            "CreationClassName=\"CIM_IndicationHandlerCIMXML\","
            "Name=\"LIHandler01\",SystemCreationClassName=\"");
        handlerPathString.append (System::getSystemCreationClassName ());
        handlerPathString.append ("\",SystemName=\"");
        handlerPathString.append (System::getFullyQualifiedHostName ());
        handlerPathString.append ("\"");
        _createSubscriptionInstance (client, CIMObjectPath (filterPathString),
            CIMObjectPath (handlerPathString));
    }
    catch (Exception & e)
    {
        PEGASUS_STD (cerr) << "setup 4 failed: " << e.getMessage ()
            << PEGASUS_STD (endl);
    }

    try
    {
        String filterPathString;
        filterPathString.append ("CIM_IndicationFilter.CreationClassName"
                "=\"CIM_IndicationFilter\",Name=\"LIFilter02\","
                "SystemCreationClassName=\"");
        filterPathString.append (System::getSystemCreationClassName ());
        filterPathString.append ("\",SystemName=\"");
        filterPathString.append (System::getFullyQualifiedHostName ());
        filterPathString.append ("\"");
        String handlerPathString;
        handlerPathString.append ("CIM_IndicationHandlerCIMXML."
                "CreationClassName=\"CIM_IndicationHandlerCIMXML\","
                "Name=\"LIHandler01\",SystemCreationClassName=\"");
        handlerPathString.append (System::getSystemCreationClassName ());
        handlerPathString.append ("\",SystemName=\"");
        handlerPathString.append (System::getFullyQualifiedHostName ());
        handlerPathString.append ("\"");
        _createSubscriptionInstance (client, CIMObjectPath (filterPathString),
            CIMObjectPath (handlerPathString));
    }
    catch (Exception & e)
    {
        PEGASUS_STD (cerr) << "setup 5 failed: " << e.getMessage ()
            << PEGASUS_STD (endl);
    }

    PEGASUS_STD (cout) << "+++++ setup completed successfully"
        << PEGASUS_STD (endl);
}

void _cleanup (CIMClient & client)
{
    try
    {
        _deleteSubscriptionInstance (client, String ("LIFilter01"),
            String ("LIHandler01"));
    }
    catch (Exception & e)
    {
        PEGASUS_STD (cerr) << "cleanup 1 failed: " << e.getMessage ()
            << PEGASUS_STD (endl);
    }

    try
    {
        _deleteSubscriptionInstance (client, String ("LIFilter02"),
            String ("LIHandler01"));
    }
    catch (Exception & e)
    {
        PEGASUS_STD (cerr) << "cleanup 2 failed: " << e.getMessage ()
            << PEGASUS_STD (endl);
    }

    try
    {
        _deleteHandlerInstance (client, String ("LIHandler01"));
    }
    catch (Exception & e)
    {
        PEGASUS_STD (cerr) << "cleanup 3 failed: " << e.getMessage ()
            << PEGASUS_STD (endl);
    }

    try
    {
        _deleteFilterInstance (client, String ("LIFilter01"));
    }
    catch (Exception & e)
    {
        PEGASUS_STD (cerr) << "cleanup 4 failed: " << e.getMessage ()
            << PEGASUS_STD (endl);
    }

    try
    {
        _deleteFilterInstance (client, String ("LIFilter02"));
    }
    catch (Exception & e)
    {
        PEGASUS_STD (cerr) << "cleanup 5 failed: " << e.getMessage ()
            << PEGASUS_STD (endl);
    }

    PEGASUS_STD (cout) << "+++++ cleanup completed successfully"
        << PEGASUS_STD (endl);
}

int _test(CIMClient& client, const char* opt, const char* optLang)
{
    if (String::equalNoCase (opt, "setup"))
    {
        if (optLang == NULL)
        {
            cerr << "Error, query language not specified, must be 'WQL' "
                "or 'CIM:CQL'" << endl;
            _usage ();
            return -1;
        }
        String qlang(optLang);
        _renameLogFile();
        _setup (client, qlang);
    }
    else if (String::equalNoCase (opt, "createInstance"))
    {
        _createInstance (client);
    }
    else if (String::equalNoCase (opt, "deleteInstance"))
    {
        _deleteInstance (client);
    }
    else if (String::equalNoCase (opt, "cleanup"))
    {
        _cleanup (client);
    }
    else
    {
        PEGASUS_STD (cerr) << "Invalid option: " << opt << PEGASUS_STD (endl);
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

    if (argc < 2 || argc > 3 )
    {
        _usage ();
        return 1;
    }
    else
    {
        const char * opt = argv [1];
        const char * optLang;

        if (argc == 3)
        {
            optLang = argv [2];
        }
        else
        {
            optLang = NULL;
        }

#ifndef PEGASUS_ENABLE_CQL
        if (qlang == "CIM:CQL")
        {
            PEGASUS_STD (cout) << "+++++ cql test disabled"
                << PEGASUS_STD (endl);
            return 0;
        }
#endif
        return _test(client, opt, optLang);
    }

    return 0;
}
