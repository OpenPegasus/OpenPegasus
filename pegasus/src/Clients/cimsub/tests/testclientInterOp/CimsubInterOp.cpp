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

#include <Providers/TestProviders/TestProviderRegistration.h>

PEGASUS_USING_PEGASUS;
PEGASUS_USING_STD;

#define IGNORE_EXCEPTION(FUNC) \
    try \
    { \
        FUNC \
    } \
    catch (...) \
    { \
    }

const CIMNamespaceName NAMESPACE = PEGASUS_NAMESPACENAME_INTEROP;
const CIMNamespaceName NAMESPACE1 = CIMNamespaceName ("root/cimv2");
const CIMNamespaceName NAMESPACE2 = CIMNamespaceName ("test/TestProvider");
const CIMNamespaceName NAMESPACE3 = CIMNamespaceName ("root/SampleProvider");
const CIMNamespaceName SOURCENAMESPACE =
    CIMNamespaceName ("root/SampleProvider");
const CIMNamespaceName NULL_NAMESPACE;


Boolean verbose;

void _modifyCapabilityInstance(
    CIMClient & client,
    const String & providerModuleName,
    const String & providerName,
    const String & capabilityID,
    const CIMPropertyList & supportedProperties)
{
    CIMInstance capabilityInstance(PEGASUS_CLASSNAME_PROVIDERCAPABILITIES);
    if (!supportedProperties.isNull())
    {
        Array <String> propertyNameStrings;
        for (Uint32 i = 0; i < supportedProperties.size(); i++)
        {
            propertyNameStrings.append(supportedProperties[i].getString());
        }
        capabilityInstance.addProperty(CIMProperty
            (CIMName("SupportedProperties"), propertyNameStrings));
    }

    CIMObjectPath path;
    Array <CIMKeyBinding> keyBindings;
    keyBindings.append(CIMKeyBinding("ProviderModuleName", providerModuleName,
        CIMKeyBinding::STRING));
    keyBindings.append(CIMKeyBinding("ProviderName", providerName,
        CIMKeyBinding::STRING));
    keyBindings.append(CIMKeyBinding("CapabilityID", capabilityID,
        CIMKeyBinding::STRING));
    path.setClassName(PEGASUS_CLASSNAME_PROVIDERCAPABILITIES);
    path.setKeyBindings(keyBindings);
    capabilityInstance.setPath(path);

    Array <CIMName> propertyNames;
    propertyNames.append(CIMName("SupportedProperties"));
    CIMPropertyList properties(propertyNames);
    client.modifyInstance(PEGASUS_NAMESPACENAME_INTEROP, capabilityInstance,
        false, properties);
}

void _addStringProperty(
    CIMInstance & instance,
    const String & name,
    const String & value,
    Boolean null = false,
    Boolean isArray = false)
{
    if (null)
    {
        instance.addProperty(CIMProperty(CIMName(name),
            CIMValue(CIMTYPE_STRING, false)));
    }
    else
    {
        if (isArray)
        {
            Array <String> values;
            values.append(value);
            instance.addProperty(CIMProperty(CIMName(name), values));
        }
        else
        {
            instance.addProperty(CIMProperty(CIMName(name), value));
        }
    }
}

void _addUint16Property(
    CIMInstance & instance,
    const String & name,
    Uint16 value,
    Boolean null = false,
    Boolean isArray = false)
{
    if (null)
    {
        instance.addProperty(CIMProperty(CIMName(name),
            CIMValue(CIMTYPE_UINT16, false)));
    }
    else
    {
        if (isArray)
        {
            Array <Uint16> values;
            values.append(value);
            instance.addProperty(CIMProperty(CIMName(name), values));
        }
        else
        {
            instance.addProperty(CIMProperty(CIMName(name), value));
        }
    }
}

void _addUint32Property(
    CIMInstance & instance,
    const String & name,
    Uint32 value,
    Boolean null = false,
    Boolean isArray = false)
{
    if (null)
    {
        instance.addProperty(CIMProperty(CIMName(name),
            CIMValue(CIMTYPE_UINT32, false)));
    }
    else
    {
        if (isArray)
        {
            Array <Uint32> values;
            values.append(value);
            instance.addProperty(CIMProperty(CIMName(name), values));
        }
        else
        {
            instance.addProperty(CIMProperty(CIMName(name), value));
        }
    }
}

void _addUint64Property(
    CIMInstance & instance,
    const String & name,
    Uint64 value,
    Boolean null = false,
    Boolean isArray = false)
{
    if (null)
    {
        instance.addProperty(CIMProperty(CIMName(name),
            CIMValue(CIMTYPE_UINT64, false)));
    }
    else
    {
        if (isArray)
        {
            Array <Uint64> values;
            values.append(value);
            instance.addProperty(CIMProperty(CIMName(name), values));
        }
        else
        {
            instance.addProperty(CIMProperty(CIMName(name), value));
        }
    }
}

CIMObjectPath _buildFilterOrHandlerPath(
    const CIMName & className,
    const String & name,
    const String & host = String::EMPTY,
    const CIMNamespaceName & namespaceName = CIMNamespaceName())
{
    CIMObjectPath path;

    Array <CIMKeyBinding> keyBindings;
    keyBindings.append(CIMKeyBinding("SystemCreationClassName",
        System::getSystemCreationClassName(), CIMKeyBinding::STRING));
    keyBindings.append(CIMKeyBinding("SystemName",
        System::getFullyQualifiedHostName(), CIMKeyBinding::STRING));
    keyBindings.append(CIMKeyBinding("CreationClassName",
        className.getString(), CIMKeyBinding::STRING));
    keyBindings.append(CIMKeyBinding(PEGASUS_PROPERTYNAME_NAME, name,
        CIMKeyBinding::STRING));
    path.setClassName(className);
    path.setKeyBindings(keyBindings);
    path.setNameSpace(namespaceName);
    path.setHost(host);

    return path;
}

CIMObjectPath _buildSubscriptionPath(
    const String & filterName,
    const CIMName & handlerClass,
    const String & handlerName,
    const String & filterHost = String::EMPTY,
    const String & handlerHost = String::EMPTY,
    const CIMNamespaceName & filterNS = NULL_NAMESPACE,
    const CIMNamespaceName & handlerNS = CIMNamespaceName())
{
    CIMObjectPath filterPath = _buildFilterOrHandlerPath(
        PEGASUS_CLASSNAME_INDFILTER, filterName, filterHost, filterNS);

    CIMObjectPath handlerPath = _buildFilterOrHandlerPath(handlerClass,
        handlerName, handlerHost, handlerNS);

    Array<CIMKeyBinding> subscriptionKeyBindings;
    subscriptionKeyBindings.append(CIMKeyBinding(
        PEGASUS_PROPERTYNAME_FILTER, filterPath.toString(),
        CIMKeyBinding::REFERENCE));
    subscriptionKeyBindings.append(CIMKeyBinding(
        PEGASUS_PROPERTYNAME_HANDLER,
        handlerPath.toString(), CIMKeyBinding::REFERENCE));
    CIMObjectPath subscriptionPath ("", CIMNamespaceName (),
        PEGASUS_CLASSNAME_INDSUBSCRIPTION, subscriptionKeyBindings);

    return subscriptionPath;
}

CIMInstance _buildSubscriptionInstance(
    const CIMObjectPath & filterPath,
    const CIMName & handlerClass,
    const CIMObjectPath & handlerPath)
{
    CIMInstance subscriptionInstance(PEGASUS_CLASSNAME_INDSUBSCRIPTION);

    subscriptionInstance.addProperty(CIMProperty(
            PEGASUS_PROPERTYNAME_FILTER.getString(),
        filterPath, 0, PEGASUS_CLASSNAME_INDFILTER));
    subscriptionInstance.addProperty(CIMProperty(
        PEGASUS_PROPERTYNAME_HANDLER.getString(),
        handlerPath, 0, handlerClass));

    return subscriptionInstance;
}

void _deleteSubscriptionInstance(
    CIMClient & client,
    const String & filterName,
    const CIMName & handlerClass,
    const String & handlerName,
    const String & filterHost = String::EMPTY,
    const String & handlerHost = String::EMPTY,
    const CIMNamespaceName & filterNS = NULL_NAMESPACE,
    const CIMNamespaceName & handlerNS = CIMNamespaceName(),
    const CIMNamespaceName & subscriptionNS = NAMESPACE)
{
    CIMObjectPath subscriptionPath = _buildSubscriptionPath(
        filterName, handlerClass, handlerName, filterHost, handlerHost,
        filterNS, handlerNS);
    client.deleteInstance(subscriptionNS, subscriptionPath);
}

void _deleteHandlerInstance(
    CIMClient & client,
    const CIMName & className,
    const String & name,
    const CIMNamespaceName & nameSpace = NAMESPACE)
{
    CIMObjectPath path = _buildFilterOrHandlerPath(className, name);
    client.deleteInstance(nameSpace, path);
}

void _deleteFilterInstance(
    CIMClient & client,
    const String & name,
    const CIMNamespaceName & nameSpace = NAMESPACE)
{
    CIMObjectPath path = _buildFilterOrHandlerPath(
        PEGASUS_CLASSNAME_INDFILTER, name);
    client.deleteInstance(nameSpace, path);
}

void _usage()
{
    cerr
        << "Usage: TestCimsubInteOp "
        << "{register | test | "
#ifdef PEGASUS_ENABLE_EMAIL_HANDLER
        << "test_email |"
#endif
#ifdef PEGASUS_ENABLE_SYSTEM_LOG_HANDLER
        << " test_syslog |"
#endif
        << " unregister | cleanup }"
        << endl;
}

void _register(CIMClient & client)
{
    try
    {
        Array <String> namespaces;
        Array <Uint16> providerTypes;
        namespaces.append(SOURCENAMESPACE.getString());
        providerTypes.append(4);
        providerTypes.append(5);

        //
        //  Register the ProcessIndicationProvider
        //
        TestProviderRegistration::createModuleInstance(
            client,
            String("ProcessIndicationProviderModule"),
            String("ProcessIndicationProvider"));
        TestProviderRegistration::createProviderInstance(
            client,
            String("ProcessIndicationProvider"),
            String("ProcessIndicationProviderModule"));
        TestProviderRegistration::createCapabilityInstance(
            client,
            String("ProcessIndicationProviderModule"),
            String("ProcessIndicationProvider"),
            String("ProcessIndicationProviderCapability"),
            String("CIM_ProcessIndication"),
            namespaces,
            providerTypes,
            CIMPropertyList());

        //
        //  Register the AlertIndicationProvider
        //
        TestProviderRegistration::createModuleInstance(
            client,
            String("AlertIndicationProviderModule"),
            String("AlertIndicationProvider"));
        TestProviderRegistration::createProviderInstance(
            client,
            String("AlertIndicationProvider"),
            String("AlertIndicationProviderModule"));
        TestProviderRegistration::createCapabilityInstance(
            client,
            String("AlertIndicationProviderModule"),
            String("AlertIndicationProvider"),
            String("AlertIndicationProviderCapability"),
            String("CIM_AlertIndication"),
            namespaces,
            providerTypes,
            CIMPropertyList());
    }
    catch (Exception & e)
    {
        cerr << "register failed: " << e.getMessage() << endl;
        exit (-1);
    }

    cout << "+++++ register completed successfully" << endl;
}

//
//  Creates filters, handlers, and subscriptions to test the cimsub CLI
//
void _createCimsubTests(CIMClient & client, String& qlang)
{
    CIMObjectPath path;
    String query;

    //
    //  Create filter that selects all properties from CIM_ProcessIndication
    //
    CIMInstance filter01(PEGASUS_CLASSNAME_INDFILTER);
    query = "SELECT * FROM CIM_ProcessIndication";
    _addStringProperty(filter01, "SystemCreationClassName",
         System::getSystemCreationClassName());
    _addStringProperty(filter01, "SystemName",
         System::getFullyQualifiedHostName());
    _addStringProperty(filter01,
         PEGASUS_PROPERTYNAME_CREATIONCLASSNAME.getString(),
         PEGASUS_CLASSNAME_INDFILTER.getString());
    _addStringProperty(filter01, PEGASUS_PROPERTYNAME_NAME.getString(),
        "Filter01");
    _addStringProperty(filter01, "SourceNamespace",
        SOURCENAMESPACE.getString());
    _addStringProperty(filter01, PEGASUS_PROPERTYNAME_QUERY.getString(),
        query);
    _addStringProperty(filter01,
        PEGASUS_PROPERTYNAME_QUERYLANGUAGE.getString(), qlang);
    path = client.createInstance(NAMESPACE, filter01);

    //
    //  Create filter that selects some properties from
    //  CIM_ProcessIndication
    //
    CIMInstance filter02(PEGASUS_CLASSNAME_INDFILTER);
    query = "SELECT IndicationTime, IndicationIdentifier, "
        "CorrelatedIndications "
        "FROM CIM_ProcessIndication";
    _addStringProperty(filter02, PEGASUS_PROPERTYNAME_NAME.getString(),
        "Filter02");
    _addStringProperty(filter02, PEGASUS_PROPERTYNAME_QUERY.getString(),
        query);
    _addStringProperty(filter02,
        PEGASUS_PROPERTYNAME_QUERYLANGUAGE.getString(), qlang);
    _addStringProperty(filter02, "SourceNamespace",
        SOURCENAMESPACE.getString());
    path = client.createInstance(NAMESPACE, filter02);

    //
    //  Create filter that selects one property from CIM_ProcessIndication
    //
    CIMInstance filter03(PEGASUS_CLASSNAME_INDFILTER);
    query = "SELECT IndicationTime FROM CIM_ProcessIndication";
    _addStringProperty(filter03, PEGASUS_PROPERTYNAME_NAME.getString(),
        "Filter03");
    _addStringProperty(filter03, PEGASUS_PROPERTYNAME_QUERY.getString(),
        query);
    _addStringProperty(filter03,
        PEGASUS_PROPERTYNAME_QUERYLANGUAGE.getString(), qlang);
    _addStringProperty(filter03, "SourceNamespace",
        SOURCENAMESPACE.getString());
    path = client.createInstance(NAMESPACE, filter03);

    //
    //  Create filter that selects properties from CIM_ProcessIndication
    //  and has a where clause condition
    //
    CIMInstance filter04(PEGASUS_CLASSNAME_INDFILTER);
    query = "SELECT IndicationTime, IndicationIdentifier "
          "FROM CIM_ProcessIndication "
          "WHERE IndicationTime IS NOT NULL";
    _addStringProperty(filter04, PEGASUS_PROPERTYNAME_NAME.getString(),
        "Filter04");
    _addStringProperty(filter04, PEGASUS_PROPERTYNAME_QUERY.getString(),
        query);
    _addStringProperty(filter04,
        PEGASUS_PROPERTYNAME_QUERYLANGUAGE.getString(), qlang);
    _addStringProperty(filter04, "SourceNamespace",
        SOURCENAMESPACE.getString());
    path = client.createInstance(NAMESPACE, filter04);


#ifdef PEGASUS_ENABLE_CQL
    //
    //  Create filter that selects properties from CIM_ProcessIndication
    //  and has a where clause condition that includes an array property.
    //  Note: this is only allowed by CQL.
    //
    CIMInstance filter04a(PEGASUS_CLASSNAME_INDFILTER);
    query = "SELECT IndicationTime, IndicationIdentifier "
          "FROM CIM_ProcessIndication "
          "WHERE IndicationTime IS NOT NULL AND CorrelatedIndications IS"
          " NOT NULL";
    _addStringProperty(filter04a, PEGASUS_PROPERTYNAME_NAME.getString(),
        "Filter04a");
    _addStringProperty(filter04a, PEGASUS_PROPERTYNAME_QUERY.getString(),
        query);

     // hardcode to CQL

    _addStringProperty(filter04a,
        PEGASUS_PROPERTYNAME_QUERYLANGUAGE.getString(), "DMTF:CQL");
    _addStringProperty(filter04a, "SourceNamespace",
        SOURCENAMESPACE.getString());
    path = client.createInstance(NAMESPACE, filter04a);
#endif

    //
    //  Create filter that selects all properties from CIM_ProcessIndication
    //  and has a where clause condition
    //
    CIMInstance filter05(PEGASUS_CLASSNAME_INDFILTER);
    query = "SELECT * FROM CIM_ProcessIndication "
        "WHERE IndicationTime IS NOT NULL";
    _addStringProperty(filter05, PEGASUS_PROPERTYNAME_NAME.getString(),
        "Filter05");
    _addStringProperty(filter05, PEGASUS_PROPERTYNAME_QUERY.getString(),
        query);
    _addStringProperty(filter05,
        PEGASUS_PROPERTYNAME_QUERYLANGUAGE.getString(), qlang);
    _addStringProperty(filter05, "SourceNamespace",
        SOURCENAMESPACE.getString());
    path = client.createInstance(NAMESPACE, filter05);

    //
    //  Create filter that selects all properties from CIM_AlertIndication
    //  and has a where clause condition
    //
    CIMInstance filter06(PEGASUS_CLASSNAME_INDFILTER);
    query = "SELECT * FROM CIM_AlertIndication WHERE AlertType = 5";
    _addStringProperty(filter06, PEGASUS_PROPERTYNAME_NAME.getString(),
        "Filter06");
    _addStringProperty(filter06, PEGASUS_PROPERTYNAME_QUERY.getString(),
        query);
    _addStringProperty(filter06,
        PEGASUS_PROPERTYNAME_QUERYLANGUAGE.getString(), qlang);
    _addStringProperty(filter06, "SourceNamespace",
        SOURCENAMESPACE.getString());
    path = client.createInstance(NAMESPACE, filter06);

    //
    //  Create persistent CIMXML handler
    //
    CIMInstance handler01(PEGASUS_CLASSNAME_INDHANDLER_CIMXML);
    _addStringProperty(handler01, "SystemCreationClassName",
         System::getSystemCreationClassName());
    _addStringProperty(handler01, "SystemName",
         System::getFullyQualifiedHostName());
    _addStringProperty(handler01,
         PEGASUS_PROPERTYNAME_CREATIONCLASSNAME.getString(),
         PEGASUS_CLASSNAME_INDHANDLER_CIMXML.getString());
    _addStringProperty(handler01, PEGASUS_PROPERTYNAME_NAME.getString(),
        "Handler01");
    _addStringProperty(handler01, "Owner", "an owner");
    _addUint16Property(handler01,
        PEGASUS_PROPERTYNAME_PERSISTENCETYPE.getString(),
        PERSISTENCE_PERMANENT);
    _addStringProperty(handler01, "OtherPersistenceType", String::EMPTY,
        true);
    _addStringProperty(handler01,
        PEGASUS_PROPERTYNAME_LSTNRDST_DESTINATION.getString(),
        "localhost/CIMListener/test1");
    path = client.createInstance(NAMESPACE, handler01);

    //
    //  Create transient CIMXML handler
    //
    CIMInstance handler02(PEGASUS_CLASSNAME_INDHANDLER_CIMXML);
    _addStringProperty(handler02, PEGASUS_PROPERTYNAME_NAME.getString(),
        "Handler02");
    _addUint16Property(handler02,
        PEGASUS_PROPERTYNAME_PERSISTENCETYPE.getString(),
        PERSISTENCE_TRANSIENT);
    _addStringProperty(handler02,
        PEGASUS_PROPERTYNAME_LSTNRDST_DESTINATION.getString(),
        "localhost/CIMListener/test2");
    path = client.createInstance(NAMESPACE, handler02);

    //
    //  Create transient CIMXML handler
    //
    CIMInstance handler02t(PEGASUS_CLASSNAME_INDHANDLER_CIMXML);
    _addStringProperty(handler02t, "SystemCreationClassName",
         System::getSystemCreationClassName());
    _addStringProperty(handler02t, "SystemName",
         System::getFullyQualifiedHostName());
    _addStringProperty(handler02t,
         PEGASUS_PROPERTYNAME_CREATIONCLASSNAME.getString(),
         PEGASUS_CLASSNAME_INDHANDLER_CIMXML.getString());
    _addStringProperty(handler02t, PEGASUS_PROPERTYNAME_NAME.getString(),
        "Handler02t");
    _addUint16Property(handler02t,
        PEGASUS_PROPERTYNAME_PERSISTENCETYPE.getString(),
        PERSISTENCE_TRANSIENT);
    _addStringProperty(handler02t,
        PEGASUS_PROPERTYNAME_LSTNRDST_DESTINATION.getString(),
        "localhost/CIMListener/test2");
    path = client.createInstance(NAMESPACE, handler02t);

    //
    //  Create persistent CIMXML handler
    //
    CIMInstance handler02p(PEGASUS_CLASSNAME_INDHANDLER_CIMXML);
    _addStringProperty(handler02p, "SystemCreationClassName",
         System::getSystemCreationClassName());
    _addStringProperty(handler02p, "SystemName",
         System::getFullyQualifiedHostName());
    _addStringProperty(handler02p,
         PEGASUS_PROPERTYNAME_CREATIONCLASSNAME.getString(),
         PEGASUS_CLASSNAME_INDHANDLER_CIMXML.getString());
    _addStringProperty(handler02p, PEGASUS_PROPERTYNAME_NAME.getString(),
        "Handler02p");
    _addUint16Property(handler02p,
        PEGASUS_PROPERTYNAME_PERSISTENCETYPE.getString(),
        PERSISTENCE_PERMANENT);
    _addStringProperty(handler02p,
        PEGASUS_PROPERTYNAME_LSTNRDST_DESTINATION.getString(),
        "localhost/CIMListener/test2");
    path = client.createInstance(NAMESPACE, handler02p);

    //
    //  Create persistent SNMP handler
    //
    CIMInstance handler03(PEGASUS_CLASSNAME_INDHANDLER_SNMP);
    _addStringProperty(handler03, "SystemCreationClassName",
         System::getSystemCreationClassName());
    _addStringProperty(handler03, "SystemName",
         System::getFullyQualifiedHostName());
    _addStringProperty(handler03,
         PEGASUS_PROPERTYNAME_CREATIONCLASSNAME.getString(),
         PEGASUS_CLASSNAME_INDHANDLER_SNMP.getString());
    _addStringProperty(handler03, PEGASUS_PROPERTYNAME_NAME.getString(),
        "Handler03");
    _addStringProperty(handler03, "Owner", "an owner");
    _addUint16Property(handler03,
        PEGASUS_PROPERTYNAME_PERSISTENCETYPE.getString(),
        PERSISTENCE_PERMANENT);
    _addStringProperty(handler03, "OtherPersistenceType",
        String::EMPTY, true);
    _addStringProperty(handler03,
        PEGASUS_PROPERTYNAME_LSTNRDST_TARGETHOST.getString(),
        "localhost");
    _addUint16Property(handler03, "TargetHostFormat", 2);
    _addUint32Property(handler03, "PortNumber", 162);
    _addUint16Property(handler03,
        PEGASUS_PROPERTYNAME_SNMPVERSION.getString(), SNMPV2C_TRAP);
    _addStringProperty(handler03, "SNMPSecurityName", "a name");
    _addStringProperty(handler03, "SNMPEngineID", "an ID");
    path = client.createInstance(NAMESPACE, handler03);


    //
    //  Create persistent CIMXML listener destination
    //
    CIMInstance listenerdestination01(PEGASUS_CLASSNAME_LSTNRDST_CIMXML);
    _addStringProperty(listenerdestination01, "SystemCreationClassName",
         System::getSystemCreationClassName());
    _addStringProperty (listenerdestination01, "SystemName",
         System::getFullyQualifiedHostName());
    _addStringProperty(listenerdestination01,
         PEGASUS_PROPERTYNAME_CREATIONCLASSNAME.getString(),
         PEGASUS_CLASSNAME_LSTNRDST_CIMXML.getString());
    _addStringProperty(listenerdestination01,
        PEGASUS_PROPERTYNAME_NAME.getString(),
        "ListenerDestination01");
    _addUint16Property(listenerdestination01,
        PEGASUS_PROPERTYNAME_PERSISTENCETYPE.getString(),
        PERSISTENCE_PERMANENT);
    _addStringProperty(listenerdestination01, "OtherPersistenceType",
        String::EMPTY, true);
    _addStringProperty(listenerdestination01,
        PEGASUS_PROPERTYNAME_LSTNRDST_DESTINATION.getString(),
        "localhost/CIMListener/test3");
    path = client.createInstance(NAMESPACE, listenerdestination01);

    //
    //  Create transient CIMXML listener destination
    //
    CIMInstance listenerdestination01t(PEGASUS_CLASSNAME_LSTNRDST_CIMXML);
    _addStringProperty(listenerdestination01t, "SystemCreationClassName",
         System::getSystemCreationClassName());
    _addStringProperty(listenerdestination01t, "SystemName",
         System::getFullyQualifiedHostName());
    _addStringProperty(listenerdestination01t,
         PEGASUS_PROPERTYNAME_CREATIONCLASSNAME.getString(),
         PEGASUS_CLASSNAME_LSTNRDST_CIMXML.getString());
    _addStringProperty(listenerdestination01t,
        PEGASUS_PROPERTYNAME_NAME.getString(),
        "ListenerDestination01t");
    _addUint16Property(listenerdestination01t,
        PEGASUS_PROPERTYNAME_PERSISTENCETYPE.getString(),
        PERSISTENCE_TRANSIENT);
    _addStringProperty(listenerdestination01t, "OtherPersistenceType",
        String::EMPTY, true);
    _addStringProperty(listenerdestination01t,
        PEGASUS_PROPERTYNAME_LSTNRDST_DESTINATION.getString(),
        "localhost/CIMListener/test3");
    path = client.createInstance(NAMESPACE, listenerdestination01t);

    //
    //  Create subscriptions
    //
    CIMInstance subscription01 = _buildSubscriptionInstance(
        _buildFilterOrHandlerPath(PEGASUS_CLASSNAME_INDFILTER, "Filter01"),
        PEGASUS_CLASSNAME_INDHANDLER_CIMXML,
        _buildFilterOrHandlerPath(PEGASUS_CLASSNAME_INDHANDLER_CIMXML,
            "Handler01"));
    _addUint16Property(subscription01, "OnFatalErrorPolicy", 2);
    _addStringProperty(subscription01, "OtherOnFatalErrorPolicy",
        String::EMPTY, true);
    _addUint64Property(subscription01, "FailureTriggerTimeInterval", 60);
    _addUint16Property(subscription01,
         PEGASUS_PROPERTYNAME_SUBSCRIPTION_STATE.getString(),
         STATE_ENABLED);
    _addStringProperty(subscription01, "OtherSubscriptionState",
        String::EMPTY, true);
    _addUint64Property(subscription01, "SubscriptionDuration",
        PEGASUS_UINT64_LITERAL(60000000000));
    _addUint16Property(subscription01, "RepeatNotificationPolicy", 1);
    _addStringProperty(subscription01, "OtherRepeatNotificationPolicy",
        "another policy");
    _addUint64Property(subscription01, "RepeatNotificationInterval", 60);
    _addUint64Property(subscription01, "RepeatNotificationGap", 30);
    _addUint16Property(subscription01, "RepeatNotificationCount", 5);
    path = client.createInstance(NAMESPACE, subscription01);

    CIMInstance subscription02 = _buildSubscriptionInstance(
        _buildFilterOrHandlerPath(PEGASUS_CLASSNAME_INDFILTER, "Filter02"),
        PEGASUS_CLASSNAME_INDHANDLER_CIMXML,
        _buildFilterOrHandlerPath(PEGASUS_CLASSNAME_INDHANDLER_CIMXML,
            "Handler01"));
    path = client.createInstance(NAMESPACE, subscription02);

    CIMInstance subscription03 = _buildSubscriptionInstance(
        _buildFilterOrHandlerPath(PEGASUS_CLASSNAME_INDFILTER, "Filter03"),
        PEGASUS_CLASSNAME_INDHANDLER_CIMXML,
        _buildFilterOrHandlerPath(PEGASUS_CLASSNAME_INDHANDLER_CIMXML,
            "Handler01"));
    path = client.createInstance(NAMESPACE, subscription03);

    CIMInstance subscription04 = _buildSubscriptionInstance(
        _buildFilterOrHandlerPath(PEGASUS_CLASSNAME_INDFILTER, "Filter04"),
        PEGASUS_CLASSNAME_INDHANDLER_CIMXML,
        _buildFilterOrHandlerPath(PEGASUS_CLASSNAME_INDHANDLER_CIMXML,
            "Handler01"));
    path = client.createInstance(NAMESPACE, subscription04);

    CIMInstance subscription05 = _buildSubscriptionInstance(
        _buildFilterOrHandlerPath(PEGASUS_CLASSNAME_INDFILTER, "Filter05"),
        PEGASUS_CLASSNAME_INDHANDLER_CIMXML,
        _buildFilterOrHandlerPath(PEGASUS_CLASSNAME_INDHANDLER_CIMXML,
            "Handler01"));
    path = client.createInstance(NAMESPACE, subscription05);

    CIMInstance subscription06 = _buildSubscriptionInstance(
        _buildFilterOrHandlerPath(PEGASUS_CLASSNAME_INDFILTER, "Filter06"),
        PEGASUS_CLASSNAME_INDHANDLER_CIMXML,
        _buildFilterOrHandlerPath(PEGASUS_CLASSNAME_INDHANDLER_CIMXML,
            "Handler01"));
    path = client.createInstance(NAMESPACE, subscription06);

    //
    //  Create subscription with transient handler
    //
    CIMInstance subscription07 = _buildSubscriptionInstance(
        _buildFilterOrHandlerPath(PEGASUS_CLASSNAME_INDFILTER, "Filter01"),
        PEGASUS_CLASSNAME_INDHANDLER_CIMXML,
        _buildFilterOrHandlerPath(PEGASUS_CLASSNAME_INDHANDLER_CIMXML,
            "Handler02"));
    path = client.createInstance(NAMESPACE, subscription07);

    //
    //  Create subscription with transient handler
    //
    CIMInstance subscription07t = _buildSubscriptionInstance(
        _buildFilterOrHandlerPath(PEGASUS_CLASSNAME_INDFILTER, "Filter01"),
        PEGASUS_CLASSNAME_INDHANDLER_CIMXML,
        _buildFilterOrHandlerPath(PEGASUS_CLASSNAME_INDHANDLER_CIMXML,
            "Handler02t"));
    path = client.createInstance(NAMESPACE, subscription07t);

    //
    //  Create subscription with persistent handler
    //
    CIMInstance subscription07p = _buildSubscriptionInstance(
        _buildFilterOrHandlerPath(PEGASUS_CLASSNAME_INDFILTER, "Filter01"),
        PEGASUS_CLASSNAME_INDHANDLER_CIMXML,
        _buildFilterOrHandlerPath(PEGASUS_CLASSNAME_INDHANDLER_CIMXML,
            "Handler02p"));
    path = client.createInstance(NAMESPACE, subscription07p);

    //
    //  Create subscription with SNMP handler
    //
    CIMInstance subscription08 = _buildSubscriptionInstance(
        _buildFilterOrHandlerPath(PEGASUS_CLASSNAME_INDFILTER, "Filter01"),
        PEGASUS_CLASSNAME_INDHANDLER_SNMP,
        _buildFilterOrHandlerPath(PEGASUS_CLASSNAME_INDHANDLER_SNMP,
            "Handler03"));
    path = client.createInstance(NAMESPACE, subscription08);

    //
    //  Create subscription with CIMXML Listener Destination
    //
    CIMInstance subscription09 = _buildSubscriptionInstance(
        _buildFilterOrHandlerPath(PEGASUS_CLASSNAME_INDFILTER, "Filter01"),
        PEGASUS_CLASSNAME_LSTNRDST_CIMXML,
        _buildFilterOrHandlerPath(PEGASUS_CLASSNAME_LSTNRDST_CIMXML,
            "ListenerDestination01"));
    path = client.createInstance(NAMESPACE, subscription09);

    //
    //  Create subscription with transient CIMXML Listener Destination
    //
    CIMInstance subscription09t = _buildSubscriptionInstance(
        _buildFilterOrHandlerPath(PEGASUS_CLASSNAME_INDFILTER, "Filter01"),
        PEGASUS_CLASSNAME_LSTNRDST_CIMXML,
        _buildFilterOrHandlerPath(PEGASUS_CLASSNAME_LSTNRDST_CIMXML,
            "ListenerDestination01t"));
    path = client.createInstance(NAMESPACE, subscription09t);

    //
    //  Create Subscription with correct Host and Namespace in Filter and
    //  Handler reference property value
    //
    CIMObjectPath fPath;
    CIMObjectPath hPath;
    CIMObjectPath sPath;
    CIMInstance filter08(PEGASUS_CLASSNAME_INDFILTER);
    query = "SELECT * FROM CIM_ProcessIndication";
    _addStringProperty(filter08,
        PEGASUS_PROPERTYNAME_NAME.getString(), "Filter08");
    _addStringProperty(filter08, "SourceNamespace",
        SOURCENAMESPACE.getString());
    _addStringProperty(filter08, PEGASUS_PROPERTYNAME_QUERY.getString(),
        query);
    _addStringProperty(filter08,
        PEGASUS_PROPERTYNAME_QUERYLANGUAGE.getString(), qlang);
    fPath = client.createInstance(NAMESPACE, filter08);
    fPath.setHost (System::getFullyQualifiedHostName());
    fPath.setNameSpace(NAMESPACE);

    CIMInstance listenerdestination02(PEGASUS_CLASSNAME_LSTNRDST_CIMXML);
    _addStringProperty(listenerdestination02,
        PEGASUS_PROPERTYNAME_NAME.getString(),
        "ListenerDestination02");
    _addStringProperty(listenerdestination02,
        PEGASUS_PROPERTYNAME_LSTNRDST_DESTINATION.getString(),
        "localhost/CIMListener/test4");
    hPath = client.createInstance
        (NAMESPACE, listenerdestination02);
    hPath.setHost (System::getFullyQualifiedHostName());
    hPath.setNameSpace(NAMESPACE);

    CIMInstance subscription10 = _buildSubscriptionInstance(
        fPath, PEGASUS_CLASSNAME_LSTNRDST_CIMXML, hPath);
    sPath = client.createInstance(NAMESPACE, subscription10);

    //
    //  Create Subscription with correct Namespace in Filter and Handler
    //  reference property value
    //
    CIMInstance filter09(PEGASUS_CLASSNAME_INDFILTER);
    query = "SELECT * FROM CIM_ProcessIndication";
    _addStringProperty(filter09,
        PEGASUS_PROPERTYNAME_NAME.getString(), "Filter09");
    _addStringProperty(filter09, "SourceNamespace",
        SOURCENAMESPACE.getString());
    _addStringProperty(filter09, PEGASUS_PROPERTYNAME_QUERY.getString(),
        query);
    _addStringProperty(filter09,
        PEGASUS_PROPERTYNAME_QUERYLANGUAGE.getString(), qlang);
    fPath = client.createInstance(NAMESPACE, filter09);
    fPath.setNameSpace(NAMESPACE);

    CIMInstance listenerdestination03(PEGASUS_CLASSNAME_LSTNRDST_CIMXML);
    _addStringProperty(listenerdestination03,
        PEGASUS_PROPERTYNAME_NAME.getString(),
        "ListenerDestination03");
    _addStringProperty(listenerdestination03,
        PEGASUS_PROPERTYNAME_LSTNRDST_DESTINATION.getString(),
        "localhost/CIMListener/test5");
    hPath = client.createInstance
        (NAMESPACE, listenerdestination03);
    hPath.setNameSpace(NAMESPACE);

    CIMInstance subscription11 = _buildSubscriptionInstance(
        fPath, PEGASUS_CLASSNAME_LSTNRDST_CIMXML, hPath);
    sPath = client.createInstance(NAMESPACE, subscription11);

    //
    //  Create Subscription with Filter and Handler in different namespaces
    //
    CIMInstance filter11(PEGASUS_CLASSNAME_INDFILTER);
    query = "SELECT * FROM CIM_ProcessIndication";
    _addStringProperty(filter11,
         PEGASUS_PROPERTYNAME_NAME.getString(), "Filter11");
    _addStringProperty(filter11, "SourceNamespace",
        SOURCENAMESPACE.getString());
    _addStringProperty(filter11, PEGASUS_PROPERTYNAME_QUERY.getString(),
        query);
    _addStringProperty(filter11,
        PEGASUS_PROPERTYNAME_QUERYLANGUAGE.getString(), qlang);
    fPath = client.createInstance(NAMESPACE1, filter11);
    fPath.setNameSpace(NAMESPACE1);

    CIMInstance listenerdestination05(PEGASUS_CLASSNAME_LSTNRDST_CIMXML);
    _addStringProperty(listenerdestination05,
        PEGASUS_PROPERTYNAME_NAME.getString(),
        "ListenerDestination05");
    _addStringProperty(listenerdestination05,
        PEGASUS_PROPERTYNAME_LSTNRDST_DESTINATION.getString(),
        "localhost/CIMListener/test6");
    hPath = client.createInstance(NAMESPACE2, listenerdestination05);
    hPath.setNameSpace(NAMESPACE2);

    CIMInstance subscription13 = _buildSubscriptionInstance(
        fPath, PEGASUS_CLASSNAME_LSTNRDST_CIMXML, hPath);
    sPath = client.createInstance(NAMESPACE3, subscription13);

    //
    //  Create a second filter in different namespace
    //
    CIMInstance filter11a(PEGASUS_CLASSNAME_INDFILTER);
    query = "SELECT * FROM CIM_ProcessIndication";
    _addStringProperty(filter11a,
         PEGASUS_PROPERTYNAME_NAME.getString(), "Filter11");
    _addStringProperty(filter11a, "SourceNamespace",
        SOURCENAMESPACE.getString());
    _addStringProperty(filter11a, PEGASUS_PROPERTYNAME_QUERY.getString(),
        query);
    _addStringProperty(filter11a,
        PEGASUS_PROPERTYNAME_QUERYLANGUAGE.getString(), qlang);
    CIMObjectPath fPath2 = client.createInstance(NAMESPACE2, filter11a);
    fPath2.setNameSpace(NAMESPACE2);

    //
    //  Create a second handler in different namespace
    //
    CIMInstance listenerdestination05a(PEGASUS_CLASSNAME_LSTNRDST_CIMXML);
    _addStringProperty(listenerdestination05a,
        PEGASUS_PROPERTYNAME_NAME.getString(), "ListenerDestination05");
    _addStringProperty(listenerdestination05a,
        PEGASUS_PROPERTYNAME_LSTNRDST_DESTINATION.getString(),
        "localhost/CIMListener/test6");
    CIMObjectPath hPath2 = client.createInstance(NAMESPACE1,
        listenerdestination05a);
    hPath2.setNameSpace(NAMESPACE1);

    //
    //  Create Filter for remove filter test
    //
    CIMInstance filter13(PEGASUS_CLASSNAME_INDFILTER);
    query = "SELECT * FROM CIM_ProcessIndication";
    _addStringProperty(filter13,
         PEGASUS_PROPERTYNAME_NAME.getString(),
         "Filter13");
    _addStringProperty(filter13, "SourceNamespace",
        SOURCENAMESPACE.getString());
    _addStringProperty(filter13, PEGASUS_PROPERTYNAME_QUERY.getString(),
        query);
    _addStringProperty(filter13,
        PEGASUS_PROPERTYNAME_QUERYLANGUAGE.getString(), qlang);
    fPath = client.createInstance(NAMESPACE1, filter13);
    fPath.setNameSpace(NAMESPACE1);

    //
    //  Create Subscription with Filter and Handler in different namespaces
    //
    CIMInstance filter12(PEGASUS_CLASSNAME_INDFILTER);
    query = "SELECT * FROM CIM_ProcessIndication";
    _addStringProperty(filter12,
         PEGASUS_PROPERTYNAME_NAME.getString(),
         "Filter12");
    _addStringProperty(filter12, "SourceNamespace",
        SOURCENAMESPACE.getString());
    _addStringProperty(filter12, PEGASUS_PROPERTYNAME_QUERY.getString(),
        query);
    _addStringProperty(filter12,
        PEGASUS_PROPERTYNAME_QUERYLANGUAGE.getString(), qlang);
    fPath = client.createInstance(NAMESPACE1, filter12);
    fPath.setNameSpace(NAMESPACE1);

    CIMInstance listenerdestination08(PEGASUS_CLASSNAME_LSTNRDST_CIMXML);
    _addStringProperty(listenerdestination08,
         PEGASUS_PROPERTYNAME_NAME.getString(),
        "ListenerDestination08");
    _addStringProperty(listenerdestination08,
        PEGASUS_PROPERTYNAME_LSTNRDST_DESTINATION.getString(),
        "localhost/CIMListener/test6");
    hPath = client.createInstance(NAMESPACE2, listenerdestination08);
    hPath.setNameSpace(NAMESPACE2);

    //
    //  Create transient CIMXML listener destination
    //
    CIMInstance listenerdestination06(PEGASUS_CLASSNAME_LSTNRDST_CIMXML);
    _addStringProperty(listenerdestination06,
        PEGASUS_PROPERTYNAME_NAME.getString(), "ListenerDestination06");
    _addUint16Property(listenerdestination06,
        PEGASUS_PROPERTYNAME_PERSISTENCETYPE.getString(),
        PERSISTENCE_TRANSIENT);
    _addStringProperty(listenerdestination06,
        PEGASUS_PROPERTYNAME_LSTNRDST_DESTINATION.getString(),
        "localhost/CIMListener/test7");
    hPath = client.createInstance(NAMESPACE2, listenerdestination06);
    hPath.setNameSpace(NAMESPACE2);

    //
    //  Create subscription with transient handler
    //
    CIMInstance subscription14 = _buildSubscriptionInstance(
        fPath, PEGASUS_CLASSNAME_LSTNRDST_CIMXML, hPath);
    sPath = client.createInstance(NAMESPACE3, subscription14);

    //
    //  Create another transient CIMXML listener destination in a different
    //  namespace
    //
    CIMInstance listenerdestination06a(PEGASUS_CLASSNAME_LSTNRDST_CIMXML);
    _addStringProperty(listenerdestination06a,
        PEGASUS_PROPERTYNAME_NAME.getString(),
        "ListenerDestination06");
    _addUint16Property(listenerdestination06a,
        PEGASUS_PROPERTYNAME_PERSISTENCETYPE.getString(),
        PERSISTENCE_TRANSIENT);
    _addStringProperty(listenerdestination06a,
        PEGASUS_PROPERTYNAME_LSTNRDST_DESTINATION.getString(),
        "localhost/CIMListener/test7");
    hPath = client.createInstance(NAMESPACE3, listenerdestination06a);
    hPath.setNameSpace(NAMESPACE3);

    //
    //  Create subscription with transient handler
    //
    CIMInstance subscription14a = _buildSubscriptionInstance(
        fPath, PEGASUS_CLASSNAME_LSTNRDST_CIMXML, hPath);
    CIMObjectPath sPath14a = client.createInstance(NAMESPACE2, subscription14a);

    //
    //  Create subscription with transient handler
    //
    CIMObjectPath sPath14b = client.createInstance(NAMESPACE, subscription14);
    //
    //  Create expired subscription
    //
    CIMInstance filter15(PEGASUS_CLASSNAME_INDFILTER);
    query = "SELECT * FROM CIM_ProcessIndication";
    _addStringProperty(filter15,
        PEGASUS_PROPERTYNAME_NAME.getString(), "Filter15");
    _addStringProperty(filter15, "SourceNamespace",
        SOURCENAMESPACE.getString());
    _addStringProperty(filter15, PEGASUS_PROPERTYNAME_QUERY.getString(),
        query);
    _addStringProperty(filter15,
        PEGASUS_PROPERTYNAME_QUERYLANGUAGE.getString(), qlang);
    fPath = client.createInstance(NAMESPACE, filter15);
    fPath.setNameSpace(NAMESPACE);

    CIMInstance handler15(PEGASUS_CLASSNAME_LSTNRDST_CIMXML);
    _addStringProperty(handler15,
        PEGASUS_PROPERTYNAME_NAME.getString(), "Handler15");
    _addUint16Property(handler15,
        PEGASUS_PROPERTYNAME_PERSISTENCETYPE.getString(),
        PERSISTENCE_TRANSIENT);
    _addStringProperty(handler15,
        PEGASUS_PROPERTYNAME_LSTNRDST_DESTINATION.getString(),
        "localhost/CIMListener/test7");
    hPath = client.createInstance(NAMESPACE, handler15);
    hPath.setNameSpace(NAMESPACE);

    CIMInstance subscription15 = _buildSubscriptionInstance(
        _buildFilterOrHandlerPath(PEGASUS_CLASSNAME_INDFILTER, "Filter15"),
        PEGASUS_CLASSNAME_LSTNRDST_CIMXML,
        _buildFilterOrHandlerPath(PEGASUS_CLASSNAME_LSTNRDST_CIMXML,
        "Handler15"));
    _addUint16Property(subscription15, "OnFatalErrorPolicy", 2);
    _addStringProperty(subscription15, "OtherOnFatalErrorPolicy",
        String::EMPTY, true);
    _addUint64Property(subscription15, "FailureTriggerTimeInterval", 60);
    _addUint16Property(subscription15,
         PEGASUS_PROPERTYNAME_SUBSCRIPTION_STATE.getString(),
         STATE_ENABLED);
    _addStringProperty(subscription15, "OtherSubscriptionState", String::EMPTY,
        true);
    _addUint64Property(subscription15, "SubscriptionDuration",
        PEGASUS_UINT64_LITERAL(1));
    _addUint16Property(subscription15, "RepeatNotificationPolicy", 1);
    _addStringProperty(subscription15, "OtherRepeatNotificationPolicy",
        "another policy");
    _addUint64Property(subscription15, "RepeatNotificationInterval", 60);
    _addUint64Property(subscription15, "RepeatNotificationGap", 30);
    _addUint16Property(subscription15, "RepeatNotificationCount", 5);
    path = client.createInstance(NAMESPACE, subscription15);
    System::sleep(2);
}

void _delete (CIMClient & client)
{
    //
    //  Delete subscription instances
    //
    _deleteSubscriptionInstance(client, "Filter01",
        PEGASUS_CLASSNAME_INDHANDLER_CIMXML, "Handler01");
    _deleteSubscriptionInstance(client, "Filter02",
        PEGASUS_CLASSNAME_INDHANDLER_CIMXML, "Handler01");
    _deleteSubscriptionInstance(client, "Filter03",
        PEGASUS_CLASSNAME_INDHANDLER_CIMXML, "Handler01");
    _deleteSubscriptionInstance(client, "Filter04",
        PEGASUS_CLASSNAME_INDHANDLER_CIMXML, "Handler01");
    _deleteSubscriptionInstance(client, "Filter05",
        PEGASUS_CLASSNAME_INDHANDLER_CIMXML, "Handler01");
    _deleteSubscriptionInstance(client, "Filter06",
        PEGASUS_CLASSNAME_INDHANDLER_CIMXML, "Handler01");
    _deleteSubscriptionInstance(client, "Filter01",
        PEGASUS_CLASSNAME_INDHANDLER_CIMXML, "Handler02p");
    _deleteSubscriptionInstance(client, "Filter01",
        PEGASUS_CLASSNAME_INDHANDLER_SNMP, "Handler03");
    _deleteSubscriptionInstance(client, "Filter01",
        PEGASUS_CLASSNAME_LSTNRDST_CIMXML, "ListenerDestination01");
    _deleteSubscriptionInstance(client, "Filter11",
        PEGASUS_CLASSNAME_LSTNRDST_CIMXML, "ListenerDestination06");

    //
    //  Delete handler instances
    //
    _deleteHandlerInstance(client, PEGASUS_CLASSNAME_INDHANDLER_CIMXML,
        "Handler01");
    _deleteHandlerInstance(client, PEGASUS_CLASSNAME_INDHANDLER_CIMXML,
        "Handler02p");
    _deleteHandlerInstance(client, PEGASUS_CLASSNAME_INDHANDLER_SNMP,
        "Handler03");
    _deleteHandlerInstance(client, PEGASUS_CLASSNAME_LSTNRDST_CIMXML,
        "ListenerDestination01");
    _deleteHandlerInstance(client, PEGASUS_CLASSNAME_LSTNRDST_EMAIL,
        "EmailHandler01");
    _deleteHandlerInstance(client, PEGASUS_CLASSNAME_LSTNRDST_EMAIL,
        "EmailHandler02");

    //
    //  Delete filter instances
    //
    _deleteFilterInstance(client, "Filter01");
    _deleteFilterInstance(client, "Filter02");
    _deleteFilterInstance(client, "Filter03");
    _deleteFilterInstance(client, "Filter04");
#ifdef PEGASUS_ENABLE_CQL
    _deleteFilterInstance(client, "Filter04a");
#endif
    _deleteFilterInstance(client, "Filter05");
    _deleteFilterInstance(client, "Filter06");
}

void _test(CIMClient & client)
{
    try
    {
         String wql("WQL");
         String cql("DMTF:CQL");

        _createCimsubTests(client, wql);
        //        _delete(client);

#ifdef PEGASUS_ENABLE_CQL
        //        _createCimsubTests(client, cql);
        //        _delete(client);
#endif
    }

    catch (Exception & e)
    {
        cerr << "test failed: " << e.getMessage() << endl;
        exit (-1);
    }
}

#ifdef PEGASUS_ENABLE_SYSTEM_LOG_HANDLER

//
//  Creates filters, handlers, and subscriptions for syslog handler
//
void _createCimsubSyslogTests(CIMClient & client, String& qlang)
{
    CIMObjectPath path;
    String query;

    CIMInstance syslogfilter01(PEGASUS_CLASSNAME_INDFILTER);
    query = "SELECT * FROM CIM_ProcessIndication";
    _addStringProperty(syslogfilter01, "SystemCreationClassName",
         System::getSystemCreationClassName());
    _addStringProperty(syslogfilter01, "SystemName",
         System::getFullyQualifiedHostName());
    _addStringProperty(syslogfilter01,
         PEGASUS_PROPERTYNAME_CREATIONCLASSNAME.getString(),
         PEGASUS_CLASSNAME_INDFILTER.getString());
    _addStringProperty(syslogfilter01,
        PEGASUS_PROPERTYNAME_NAME.getString(), "SyslogFilter01");
    _addStringProperty(syslogfilter01, "SourceNamespace",
        SOURCENAMESPACE.getString());
    _addStringProperty(syslogfilter01,
        PEGASUS_PROPERTYNAME_QUERY.getString(),
        query);
    _addStringProperty(syslogfilter01,
        PEGASUS_PROPERTYNAME_QUERYLANGUAGE.getString(), qlang);
    path = client.createInstance(NAMESPACE, syslogfilter01);

    //
    //  Create persistent syslog listener destination
    //
    CIMInstance syslogHandler01(PEGASUS_CLASSNAME_LSTNRDST_SYSTEM_LOG);
    _addStringProperty(syslogHandler01, "SystemCreationClassName",
         System::getSystemCreationClassName());
    _addStringProperty(syslogHandler01, "SystemName",
         System::getFullyQualifiedHostName());
    _addStringProperty(syslogHandler01,
         PEGASUS_PROPERTYNAME_CREATIONCLASSNAME.getString(),
         PEGASUS_CLASSNAME_LSTNRDST_SYSTEM_LOG.getString());
    _addStringProperty(syslogHandler01,
        PEGASUS_PROPERTYNAME_NAME.getString(), "SyslogHandler01");
    _addUint16Property(syslogHandler01,
        PEGASUS_PROPERTYNAME_PERSISTENCETYPE.getString(),
        PERSISTENCE_PERMANENT);
    _addStringProperty(syslogHandler01, "OtherPersistenceType",
        String::EMPTY, true);
    path = client.createInstance(NAMESPACE, syslogHandler01);

    //
    //  Create subscription with syslog handler
    //
    CIMInstance subscription_syslog1 = _buildSubscriptionInstance(
        _buildFilterOrHandlerPath(PEGASUS_CLASSNAME_INDFILTER,
            "SyslogFilter01", String::EMPTY,NAMESPACE),
        PEGASUS_CLASSNAME_LSTNRDST_SYSTEM_LOG,
        _buildFilterOrHandlerPath(PEGASUS_CLASSNAME_LSTNRDST_SYSTEM_LOG,
            "SyslogHandler01"));
    path = client.createInstance(NAMESPACE, subscription_syslog1);
}

void _testSyslog(CIMClient & client)
{
    try
    {
         String wql("WQL");
         String cql("DMTF:CQL");

        _createCimsubSyslogTests(client, wql);
        //        _delete(client);

#ifdef PEGASUS_ENABLE_CQL
        //        _createCimsubSyslogTests(client, cql);
        //        _delete (client);
#endif
    }
    catch (Exception & e)
    {
        cerr << "setup failed: " << e.getMessage() << endl;
        exit (-1);
    }

    cout << "+++++ setup completed successfully" << endl;
}
#endif

#ifdef PEGASUS_ENABLE_EMAIL_HANDLER

//
//  Creates filters, handlers, and subscriptions for email handler
//
void _createCimsubEmailTests(CIMClient & client, String& qlang)
{
    CIMObjectPath path;
    String query;
    //
    //  Create persistent email handler
    //
    String mailAddr1 = "foo@x.x.x";

    //
    //  Create filter that selects all properties from CIM_ProcessIndication
    //
    CIMInstance emailfilter01(PEGASUS_CLASSNAME_INDFILTER);
    query = "SELECT * FROM CIM_ProcessIndication";
    _addStringProperty(emailfilter01, "SystemCreationClassName",
         System::getSystemCreationClassName());
    _addStringProperty(emailfilter01, "SystemName",
         System::getFullyQualifiedHostName());
    _addStringProperty(emailfilter01,
         PEGASUS_PROPERTYNAME_CREATIONCLASSNAME.getString(),
         PEGASUS_CLASSNAME_INDFILTER.getString());
    _addStringProperty(emailfilter01,
        PEGASUS_PROPERTYNAME_NAME.getString(), "EmailFilter01");
    _addStringProperty(emailfilter01, "SourceNamespace",
        SOURCENAMESPACE.getString());
    _addStringProperty(emailfilter01,
        PEGASUS_PROPERTYNAME_QUERY.getString(),
        query);
    _addStringProperty(emailfilter01,
        PEGASUS_PROPERTYNAME_QUERYLANGUAGE.getString(), qlang);
    path = client.createInstance(NAMESPACE, emailfilter01);

    CIMInstance emailHandler01(PEGASUS_CLASSNAME_LSTNRDST_EMAIL);
    _addStringProperty(emailHandler01, "SystemCreationClassName",
         System::getSystemCreationClassName());
    _addStringProperty(emailHandler01, "SystemName",
         System::getFullyQualifiedHostName());
    _addStringProperty(emailHandler01,
         PEGASUS_PROPERTYNAME_CREATIONCLASSNAME.getString(),
         PEGASUS_CLASSNAME_LSTNRDST_EMAIL.getString());
    _addStringProperty(emailHandler01,
        PEGASUS_PROPERTYNAME_NAME.getString(), "EmailHandler01");
    _addUint16Property(emailHandler01,
        PEGASUS_PROPERTYNAME_PERSISTENCETYPE.getString(),
        PERSISTENCE_PERMANENT);
    _addStringProperty(emailHandler01, "OtherPersistenceType",
        String::EMPTY, true);
    _addStringProperty(emailHandler01,
        PEGASUS_PROPERTYNAME_LSTNRDST_MAILTO.getString(), mailAddr1,
        false, true );
    _addStringProperty(emailHandler01,
        PEGASUS_PROPERTYNAME_LSTNRDST_MAILCC.getString(),
        "fooCC@x.x.x", false, true);
    _addStringProperty(emailHandler01,
        PEGASUS_PROPERTYNAME_LSTNRDST_MAILSUBJECT.getString(),
        "cimsub test");
    path = client.createInstance(NAMESPACE, emailHandler01);

    CIMInstance emailHandler02(PEGASUS_CLASSNAME_LSTNRDST_EMAIL);
    _addStringProperty(emailHandler02, "SystemCreationClassName",
         System::getSystemCreationClassName());
    _addStringProperty(emailHandler02, "SystemName",
         System::getFullyQualifiedHostName());
    _addStringProperty(emailHandler02,
         PEGASUS_PROPERTYNAME_CREATIONCLASSNAME.getString(),
         PEGASUS_CLASSNAME_LSTNRDST_EMAIL.getString());
    _addStringProperty(emailHandler02,
        PEGASUS_PROPERTYNAME_NAME.getString(), "EmailHandler02");
    _addUint16Property(emailHandler02,
        PEGASUS_PROPERTYNAME_PERSISTENCETYPE.getString(),
        PERSISTENCE_PERMANENT);
    _addStringProperty(emailHandler02, "OtherPersistenceType",
        String::EMPTY, true);
    _addStringProperty(emailHandler02,
        PEGASUS_PROPERTYNAME_LSTNRDST_MAILTO.getString(), mailAddr1,
        false, true );
    _addStringProperty(emailHandler02,
        PEGASUS_PROPERTYNAME_LSTNRDST_MAILSUBJECT.getString(),
        "cimsub test");
    path = client.createInstance(NAMESPACE, emailHandler02);

    //
    //  Create subscription with email handler
    //
    CIMInstance subscription_email1 = _buildSubscriptionInstance(
        _buildFilterOrHandlerPath(PEGASUS_CLASSNAME_INDFILTER,
            "EmailFilter01", String::EMPTY,NAMESPACE),
        PEGASUS_CLASSNAME_LSTNRDST_EMAIL,
        _buildFilterOrHandlerPath(PEGASUS_CLASSNAME_LSTNRDST_EMAIL,
            "EmailHandler01"));
    path = client.createInstance(NAMESPACE, subscription_email1);

    //
    //  Create subscription with email handler
    //
    CIMInstance subscription_email2 = _buildSubscriptionInstance(
        _buildFilterOrHandlerPath(PEGASUS_CLASSNAME_INDFILTER,
            "EmailFilter01", String::EMPTY,NAMESPACE),
        PEGASUS_CLASSNAME_LSTNRDST_EMAIL,
        _buildFilterOrHandlerPath(PEGASUS_CLASSNAME_LSTNRDST_EMAIL,
            "EmailHandler02"));
    path = client.createInstance(NAMESPACE, subscription_email2);
}

void _testEmail(CIMClient & client)
{
    try
    {
         String wql("WQL");
         String cql("DMTF:CQL");

        _createCimsubEmailTests(client, wql);
        //        _delete(client);

#ifdef PEGASUS_ENABLE_CQL
        //        _createCimsubEmailTests(client, cql);
        //        _delete(client);
#endif
    }

    catch (Exception & e)
    {
        cerr << "setup failed: " << e.getMessage() << endl;
        exit (-1);
    }

    cout << "+++++ setup completed successfully" << endl;
}
#endif

//
//  NOTE: the cleanup command line option is provided to clean up the
//  repository in case the test fails and not all objects created by
//  the test were deleted
//  This method attempts to delete each object that could have been created by
//  this test and that still exists in the repository
//  Since the repository could contain none, any or all of the objects, any
//  exceptions thrown are ignored and this method continues to attempt to
//  delete the objects
//
void _cleanup(CIMClient & client)
{
    //
    //  Delete subscription instances
    //
         IGNORE_EXCEPTION(_deleteSubscriptionInstance(client, "Filter00",
            PEGASUS_CLASSNAME_INDHANDLER_CIMXML, "Handler00");)

        IGNORE_EXCEPTION(_deleteSubscriptionInstance(client, "Filter01",
            PEGASUS_CLASSNAME_INDHANDLER_CIMXML, "Handler01");)

        IGNORE_EXCEPTION(_deleteSubscriptionInstance(client, "Filter02",
            PEGASUS_CLASSNAME_INDHANDLER_CIMXML, "Handler01");)

        IGNORE_EXCEPTION(_deleteSubscriptionInstance(client, "Filter03",
            PEGASUS_CLASSNAME_INDHANDLER_CIMXML, "Handler01");)

        IGNORE_EXCEPTION(_deleteSubscriptionInstance(client, "Filter04",
            PEGASUS_CLASSNAME_INDHANDLER_CIMXML, "Handler01");)

        IGNORE_EXCEPTION(_deleteSubscriptionInstance(client, "Filter05",
            PEGASUS_CLASSNAME_INDHANDLER_CIMXML, "Handler01");)

        IGNORE_EXCEPTION(_deleteSubscriptionInstance(client, "Filter06",
            PEGASUS_CLASSNAME_INDHANDLER_CIMXML, "Handler01");)

        IGNORE_EXCEPTION(_deleteSubscriptionInstance(client, "Filter07",
            PEGASUS_CLASSNAME_INDHANDLER_CIMXML, "Handler00");)

        IGNORE_EXCEPTION(_deleteSubscriptionInstance(client, "Filter01",
            PEGASUS_CLASSNAME_INDHANDLER_CIMXML, "Handler02");)

        IGNORE_EXCEPTION(_deleteSubscriptionInstance(client, "Filter01",
            PEGASUS_CLASSNAME_INDHANDLER_CIMXML, "Handler02p");)

        IGNORE_EXCEPTION(_deleteSubscriptionInstance(client, "Filter01",
            PEGASUS_CLASSNAME_INDHANDLER_CIMXML, "Handler02t");)

        IGNORE_EXCEPTION(_deleteSubscriptionInstance(client, "Filter01",
            PEGASUS_CLASSNAME_INDHANDLER_SNMP, "Handler03");)

#ifdef PEGASUS_ENABLE_EMAIL_HANDLER

        IGNORE_EXCEPTION(_deleteSubscriptionInstance(client, "EmailFilter01",
            PEGASUS_CLASSNAME_LSTNRDST_EMAIL, "EmailHandler01",
            String::EMPTY, String::EMPTY,
            NAMESPACE, CIMNamespaceName (), NAMESPACE);)

        IGNORE_EXCEPTION(_deleteSubscriptionInstance(client, "EmailFilter01",
            PEGASUS_CLASSNAME_LSTNRDST_EMAIL, "EmailHandler02",
            String::EMPTY, String::EMPTY,
            NAMESPACE, CIMNamespaceName (), NAMESPACE);)

#endif

#ifdef PEGASUS_ENABLE_SYSTEM_LOG_HANDLER

        IGNORE_EXCEPTION(_deleteSubscriptionInstance(client, "SyslogFilter01",
            PEGASUS_CLASSNAME_LSTNRDST_SYSTEM_LOG, "SyslogHandler01",
            String::EMPTY, String::EMPTY,
            NAMESPACE, CIMNamespaceName (), NAMESPACE);)

#endif

        IGNORE_EXCEPTION(_deleteSubscriptionInstance(client, "Filter01",
            PEGASUS_CLASSNAME_LSTNRDST_CIMXML, "ListenerDestination01t");)

        IGNORE_EXCEPTION(_deleteSubscriptionInstance(client, "Filter01",
            PEGASUS_CLASSNAME_LSTNRDST_CIMXML, "ListenerDestination01");)

        IGNORE_EXCEPTION(_deleteSubscriptionInstance(client, "Filter08",
            PEGASUS_CLASSNAME_LSTNRDST_CIMXML, "ListenerDestination02",
            System::getFullyQualifiedHostName(),
            System::getFullyQualifiedHostName(),
            NAMESPACE, NAMESPACE, NAMESPACE);)

        IGNORE_EXCEPTION(_deleteSubscriptionInstance(client, "Filter09",
            PEGASUS_CLASSNAME_LSTNRDST_CIMXML, "ListenerDestination03",
            String::EMPTY, String::EMPTY, NAMESPACE, NAMESPACE, NAMESPACE);)

        IGNORE_EXCEPTION(_deleteSubscriptionInstance(client, "Filter10",
            PEGASUS_CLASSNAME_LSTNRDST_CIMXML, "ListenerDestination04",
            System::getFullyQualifiedHostName(),
            System::getFullyQualifiedHostName(),
            NAMESPACE, NAMESPACE, NAMESPACE);)

        IGNORE_EXCEPTION(_deleteSubscriptionInstance(client, "Filter11",
            PEGASUS_CLASSNAME_LSTNRDST_CIMXML, "ListenerDestination05",
            String::EMPTY, String::EMPTY, NAMESPACE1, NAMESPACE2, NAMESPACE3);)

        IGNORE_EXCEPTION(_deleteSubscriptionInstance(client, "Filter11",
            PEGASUS_CLASSNAME_LSTNRDST_CIMXML, "ListenerDestination07",
            String::EMPTY, String::EMPTY, NAMESPACE1, NAMESPACE2, NAMESPACE3);)

        IGNORE_EXCEPTION(_deleteSubscriptionInstance(client, "Filter12",
            PEGASUS_CLASSNAME_LSTNRDST_CIMXML, "ListenerDestination06",
            String::EMPTY, String::EMPTY, NAMESPACE1, NAMESPACE2, NAMESPACE3);)

        IGNORE_EXCEPTION(_deleteSubscriptionInstance(client, "Filter12",
            PEGASUS_CLASSNAME_LSTNRDST_CIMXML, "ListenerDestination08",
            String::EMPTY, String::EMPTY, NAMESPACE1, NAMESPACE2, NAMESPACE);)

        IGNORE_EXCEPTION(_deleteSubscriptionInstance(client, "Filter11",
            PEGASUS_CLASSNAME_LSTNRDST_CIMXML, "ListenerDestination05",
            String::EMPTY, String::EMPTY, NAMESPACE2, NAMESPACE1, NAMESPACE3);)

        IGNORE_EXCEPTION(_deleteSubscriptionInstance(client, "Filter15",
            PEGASUS_CLASSNAME_LSTNRDST_CIMXML, "Handler15",
            String::EMPTY, String::EMPTY, NAMESPACE, NAMESPACE, NAMESPACE);)

    //
    //  Delete handler instances
    //
        IGNORE_EXCEPTION(_deleteHandlerInstance(client,
            PEGASUS_CLASSNAME_INDHANDLER_CIMXML, "Handler00");)

        IGNORE_EXCEPTION(_deleteHandlerInstance(client,
             PEGASUS_CLASSNAME_INDHANDLER_CIMXML, "Handler01");)

        IGNORE_EXCEPTION(_deleteHandlerInstance(client,
             PEGASUS_CLASSNAME_INDHANDLER_CIMXML, "Handler02");)

        IGNORE_EXCEPTION(_deleteHandlerInstance(client,
             PEGASUS_CLASSNAME_INDHANDLER_CIMXML, "Handler02t");)

        IGNORE_EXCEPTION(_deleteHandlerInstance(client,
             PEGASUS_CLASSNAME_INDHANDLER_CIMXML, "Handler02p");)

        IGNORE_EXCEPTION(_deleteHandlerInstance(client,
             PEGASUS_CLASSNAME_INDHANDLER_SNMP, "Handler03");)

        IGNORE_EXCEPTION(_deleteHandlerInstance(client,
             PEGASUS_CLASSNAME_LSTNRDST_CIMXML, "ListenerDestination01");)

        IGNORE_EXCEPTION(_deleteHandlerInstance(client,
             PEGASUS_CLASSNAME_LSTNRDST_CIMXML, "ListenerDestination01t");)

        IGNORE_EXCEPTION(_deleteHandlerInstance(client,
             PEGASUS_CLASSNAME_LSTNRDST_CIMXML, "ListenerDestination02");)

        IGNORE_EXCEPTION(_deleteHandlerInstance(client,
             PEGASUS_CLASSNAME_LSTNRDST_CIMXML, "ListenerDestination03");)

        IGNORE_EXCEPTION(_deleteHandlerInstance(client,
             PEGASUS_CLASSNAME_LSTNRDST_CIMXML, "ListenerDestination04");)

        IGNORE_EXCEPTION(_deleteHandlerInstance(client,
             PEGASUS_CLASSNAME_LSTNRDST_CIMXML, "ListenerDestination05",
                NAMESPACE1);)

        IGNORE_EXCEPTION(_deleteHandlerInstance(client,
             PEGASUS_CLASSNAME_LSTNRDST_CIMXML, "ListenerDestination05",
                NAMESPACE2);)

        IGNORE_EXCEPTION(_deleteHandlerInstance(client,
             PEGASUS_CLASSNAME_LSTNRDST_CIMXML, "ListenerDestination06",
                NAMESPACE2);)

        IGNORE_EXCEPTION(_deleteHandlerInstance(client,
             PEGASUS_CLASSNAME_LSTNRDST_CIMXML, "ListenerDestination06",
                NAMESPACE3);)

        IGNORE_EXCEPTION(_deleteHandlerInstance(client,
             PEGASUS_CLASSNAME_LSTNRDST_CIMXML, "ListenerDestination07",
                NAMESPACE2);)

        IGNORE_EXCEPTION(_deleteHandlerInstance(client,
             PEGASUS_CLASSNAME_LSTNRDST_CIMXML, "ListenerDestination08",
                NAMESPACE2);)

        IGNORE_EXCEPTION(_deleteHandlerInstance(client,
             PEGASUS_CLASSNAME_LSTNRDST_CIMXML, "Handler15", NAMESPACE);)

        IGNORE_EXCEPTION(_deleteHandlerInstance(client,
             PEGASUS_CLASSNAME_LSTNRDST_EMAIL, "EmailHandler01", NAMESPACE);)

        IGNORE_EXCEPTION(_deleteHandlerInstance(client,
             PEGASUS_CLASSNAME_LSTNRDST_EMAIL, "EmailHandler02", NAMESPACE);)

#ifdef PEGASUS_ENABLE_SYSTEM_LOG_HANDLER

        IGNORE_EXCEPTION(_deleteHandlerInstance(client,
            PEGASUS_CLASSNAME_LSTNRDST_SYSTEM_LOG, "SyslogHandler01",
                NAMESPACE);)

#endif

    //
    //  Delete filter instances
    //
        IGNORE_EXCEPTION(_deleteFilterInstance(client, "Filter00");)

        IGNORE_EXCEPTION(_deleteFilterInstance(client, "Filter01");)

        IGNORE_EXCEPTION(_deleteFilterInstance(client, "EmailFilter01");)

        IGNORE_EXCEPTION(_deleteFilterInstance(client, "SyslogFilter01");)

        IGNORE_EXCEPTION(_deleteFilterInstance(client, "Filter02");)

        IGNORE_EXCEPTION(_deleteFilterInstance(client, "Filter03");)

        IGNORE_EXCEPTION(_deleteFilterInstance(client, "Filter04");)

        IGNORE_EXCEPTION(_deleteFilterInstance(client, "Filter04a");)

        IGNORE_EXCEPTION(_deleteFilterInstance(client, "Filter05");)

        IGNORE_EXCEPTION(_deleteFilterInstance(client, "Filter06");)

        IGNORE_EXCEPTION(_deleteFilterInstance(client, "Filter07");)

        IGNORE_EXCEPTION(_deleteFilterInstance(client, "Filter08");)

        IGNORE_EXCEPTION(_deleteFilterInstance(client, "Filter09");)

        IGNORE_EXCEPTION(_deleteFilterInstance(client, "Filter10");)

        IGNORE_EXCEPTION(_deleteFilterInstance(client, "Filter11", NAMESPACE1);)

        IGNORE_EXCEPTION(_deleteFilterInstance(client, "Filter11", NAMESPACE2);)

        IGNORE_EXCEPTION(_deleteFilterInstance(client, "Filter12", NAMESPACE1);)

        IGNORE_EXCEPTION(_deleteFilterInstance(client, "Filter13", NAMESPACE1);)

        IGNORE_EXCEPTION(_deleteFilterInstance(client, "Filter15", NAMESPACE);)

    //
    //  Delete provider registration instances
    //
        IGNORE_EXCEPTION(TestProviderRegistration::deleteCapabilityInstance(
            client,
            "AlertIndicationProviderModule",
            "AlertIndicationProvider",
            "AlertIndicationProviderCapability");)

        IGNORE_EXCEPTION(TestProviderRegistration::deleteProviderInstance(
            client,
            "AlertIndicationProvider",
            "AlertIndicationProviderModule");)

        IGNORE_EXCEPTION(TestProviderRegistration::deleteModuleInstance(
            client,
            "AlertIndicationProviderModule");)

        IGNORE_EXCEPTION(TestProviderRegistration::deleteCapabilityInstance(
            client,
            "ProcessIndicationProviderModule",
            "ProcessIndicationProvider",
            "ProcessIndicationProviderCapability");)

        IGNORE_EXCEPTION(TestProviderRegistration::deleteProviderInstance(
            client,
            "ProcessIndicationProvider",
            "ProcessIndicationProviderModule");)

        IGNORE_EXCEPTION(TestProviderRegistration::deleteModuleInstance(
            client,
            "ProcessIndicationProviderModule");)

    cout << "+++++ cleanup completed successfully" << endl;
}

void _unregister (CIMClient & client)
{
    try
    {
        TestProviderRegistration::deleteCapabilityInstance(
            client,
            "AlertIndicationProviderModule",
            "AlertIndicationProvider",
            "AlertIndicationProviderCapability");
        TestProviderRegistration::deleteProviderInstance(
            client,
            "AlertIndicationProvider",
            "AlertIndicationProviderModule");
        TestProviderRegistration::deleteModuleInstance(
            client,
            "AlertIndicationProviderModule");

        TestProviderRegistration::deleteCapabilityInstance(
            client,
            "ProcessIndicationProviderModule",
            "ProcessIndicationProvider",
            "ProcessIndicationProviderCapability");
        TestProviderRegistration::deleteProviderInstance(
            client,
            "ProcessIndicationProvider",
            "ProcessIndicationProviderModule");
        TestProviderRegistration::deleteModuleInstance(
            client,
            "ProcessIndicationProviderModule");
    }
    catch (Exception & e)
    {
        cerr << "unregister failed: " << e.getMessage()
                           << endl;
        exit (-1);
    }

    cout << "+++++ unregister completed successfully"
                       << endl;
}

int main (int argc, char** argv)
{
    verbose = (getenv("PEGASUS_TEST_VERBOSE")) ? true : false;

    CIMClient client;
    try
    {
        client.setTimeout(120000);
        client.connectLocal();
    }
    catch (Exception & e)
    {
        cerr << e.getMessage() << endl;
        return -1;
    }

    if (argc != 2)
    {
        _usage();
        return 1;
    }

    else
    {
        const char * opt = argv [1];

        if (String::equalNoCase(opt, "register"))
        {
            _register(client);
        }
        else if (String::equalNoCase(opt, "test"))
        {
            _test(client);
        }
#ifdef PEGASUS_ENABLE_EMAIL_HANDLER

        else if (String::equalNoCase(opt, "test_email"))
        {
            _testEmail(client);
        }
#endif
#ifdef PEGASUS_ENABLE_SYSTEM_LOG_HANDLER
        else if (String::equalNoCase(opt, "test_syslog"))
        {
            _testSyslog(client);
        }
#endif
        else if (String::equalNoCase(opt, "cleanup"))
        {
            _cleanup(client);
        }
        else if (String::equalNoCase(opt, "unregister"))
        {
            _unregister(client);
        }
        else
        {
            cerr << "Invalid option: " << opt << endl;
            _usage();
            return -1;
        }
    }

    return 0;
}
