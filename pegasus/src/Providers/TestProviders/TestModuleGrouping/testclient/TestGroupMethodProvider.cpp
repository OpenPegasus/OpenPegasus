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

#include <Pegasus/Common/PegasusAssert.h>
#include <Pegasus/Common/Config.h>
#include <Pegasus/Client/CIMClient.h>
#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/Constants.h>
#include <Pegasus/Common/FileSystem.h>
#include <Pegasus/Common/PegasusAssert.h>
#include <Pegasus/Common/System.h>
#include <Pegasus/Server/ProviderRegistrationManager/\
ProviderRegistrationManager.h>

PEGASUS_USING_PEGASUS;

PEGASUS_USING_PEGASUS;
PEGASUS_USING_STD;

static const CIMNamespaceName NAMESPACE = CIMNamespaceName("test/TestProvider");

static Boolean verbose;

Boolean _validateStatus(
    CIMClient& client,
    const String& providerModuleName,
    Uint16 expectedStatus)
{
    Boolean result = false;

    try
    {
        //
        //  Get instance for module
        //
        CIMInstance moduleInstance;
        CIMKeyBinding keyBinding(CIMName("Name"), providerModuleName,
            CIMKeyBinding::STRING);
        Array<CIMKeyBinding> kbArray;
        kbArray.append(keyBinding);
        CIMObjectPath modulePath("", PEGASUS_NAMESPACENAME_INTEROP,
            PEGASUS_CLASSNAME_PROVIDERMODULE, kbArray);

        moduleInstance = client.getInstance(PEGASUS_NAMESPACENAME_INTEROP,
            modulePath);

        //
        //  Get status from instance
        //
        Array<Uint16> operationalStatus;
        Uint32 index = moduleInstance.findProperty(
            CIMName("OperationalStatus"));
        if (index != PEG_NOT_FOUND)
        {
            CIMValue statusValue =
                moduleInstance.getProperty(index).getValue();
            if (!statusValue.isNull())
            {
                statusValue.get(operationalStatus);
                for (Uint32 i = 0; i < operationalStatus.size(); i++)
                {
                    if (operationalStatus [i] == expectedStatus)
                    {
                        result = true;
                        break;
                    }
                }
            }
        }
    }
    catch (...)
    {
    }

    return result;
}

void _checkStatus(
    CIMClient& client,
    const String& providerModuleName,
    Uint16 expectedStatus)
{
    Uint32 iteration = 0;
    Boolean expectedStatusObserved = false;
    while (iteration < 300)
    {
        iteration++;
        if (_validateStatus(client, providerModuleName, expectedStatus))
        {
            expectedStatusObserved = true;
            break;
        }
        else
        {
            System::sleep(1);
        }
    }

    PEGASUS_TEST_ASSERT(expectedStatusObserved);
}


void _createModuleInstance(
    CIMClient& client,
    const String& name,
    const String& location,
    const String& groupName,
    Uint16 userContext)
{
    CIMInstance moduleInstance(PEGASUS_CLASSNAME_PROVIDERMODULE);
    moduleInstance.addProperty(CIMProperty(CIMName("Name"), name));
    moduleInstance.addProperty(CIMProperty(CIMName("Vendor"),
        String("OpenPegasus")));
    moduleInstance.addProperty(CIMProperty(CIMName("Version"),
        String("2.0")));
    moduleInstance.addProperty(CIMProperty(CIMName("InterfaceType"),
        String("C++Default")));
    moduleInstance.addProperty(CIMProperty(CIMName("InterfaceVersion"),
        String("2.5.0")));
    moduleInstance.addProperty(CIMProperty(CIMName("Location"), location));

    if (groupName.size())
    {
        moduleInstance.addProperty(
            CIMProperty(CIMName("ModuleGroupName"), groupName));
    }

#ifndef PEGASUS_DISABLE_PROV_USERCTXT
    if (userContext != 0)
    {
        moduleInstance.addProperty(CIMProperty(CIMName("UserContext"),
            userContext));
    }
#endif

    CIMObjectPath path = client.createInstance(PEGASUS_NAMESPACENAME_INTEROP,
        moduleInstance);
}

void _createProviderInstance(
    CIMClient& client,
    const String& name,
    const String& providerModuleName)
{
    CIMInstance providerInstance(PEGASUS_CLASSNAME_PROVIDER);
    providerInstance.addProperty(CIMProperty(CIMName("Name"), name));
    providerInstance.addProperty(CIMProperty(CIMName("ProviderModuleName"),
        providerModuleName));

    CIMObjectPath path = client.createInstance(PEGASUS_NAMESPACENAME_INTEROP,
        providerInstance);
}

void _createCapabilityInstance(
    CIMClient& client,
    const String& providerModuleName,
    const String& providerName,
    const String& capabilityID,
    const String& className,
    const Array<String>& namespaces,
    const Array<Uint16>& providerType,
    const CIMPropertyList& supportedProperties,
    const CIMPropertyList& supportedMethods)
{
    CIMInstance capabilityInstance(PEGASUS_CLASSNAME_PROVIDERCAPABILITIES);
    capabilityInstance.addProperty(CIMProperty(CIMName("ProviderModuleName"),
        providerModuleName));
    capabilityInstance.addProperty(CIMProperty(CIMName("ProviderName"),
        providerName));
    capabilityInstance.addProperty(CIMProperty(CIMName("CapabilityID"),
        capabilityID));
    capabilityInstance.addProperty(CIMProperty(CIMName("ClassName"),
        className));
    capabilityInstance.addProperty(CIMProperty(CIMName("Namespaces"),
        namespaces));
    capabilityInstance.addProperty(CIMProperty(CIMName("ProviderType"),
        CIMValue(providerType)));
    if (!supportedProperties.isNull())
    {
        Array<String> propertyNameStrings;
        for (Uint32 i = 0; i < supportedProperties.size(); i++)
        {
            propertyNameStrings.append(supportedProperties [i].getString());
        }
        capabilityInstance.addProperty(CIMProperty(
            CIMName("supportedProperties"), CIMValue(propertyNameStrings)));
    }
    if (!supportedMethods.isNull())
    {
        Array<String> methodNameStrings;
        for (Uint32 i = 0; i < supportedMethods.size(); i++)
        {
            methodNameStrings.append(supportedMethods [i].getString());
        }
        capabilityInstance.addProperty(CIMProperty(
            CIMName("supportedMethods"), CIMValue(methodNameStrings)));
    }

    CIMObjectPath path = client.createInstance(PEGASUS_NAMESPACENAME_INTEROP,
        capabilityInstance);
}

void _deleteCapabilityInstance(
    CIMClient& client,
    const String& providerModuleName,
    const String& providerName,
    const String& capabilityID)
{
    Array<CIMKeyBinding> keyBindings;
    keyBindings.append(CIMKeyBinding("ProviderModuleName",
        providerModuleName, CIMKeyBinding::STRING));
    keyBindings.append(CIMKeyBinding("ProviderName",
        providerName, CIMKeyBinding::STRING));
    keyBindings.append(CIMKeyBinding("CapabilityID",
        capabilityID, CIMKeyBinding::STRING));
    CIMObjectPath path("", CIMNamespaceName(),
        CIMName("PG_ProviderCapabilities"), keyBindings);
    client.deleteInstance(PEGASUS_NAMESPACENAME_INTEROP, path);
}

void _deleteProviderInstance(
    CIMClient& client,
    const String& name,
    const String& providerModuleName)
{
    Array<CIMKeyBinding> keyBindings;
    keyBindings.append(CIMKeyBinding("Name",
        name, CIMKeyBinding::STRING));
    keyBindings.append(CIMKeyBinding("ProviderModuleName",
        providerModuleName, CIMKeyBinding::STRING));
    CIMObjectPath path("", CIMNamespaceName(),
        CIMName("PG_Provider"), keyBindings);
    client.deleteInstance(PEGASUS_NAMESPACENAME_INTEROP, path);
}

void _deleteModuleInstance(
    CIMClient& client,
    const String& name)
{
    Array<CIMKeyBinding> keyBindings;
    keyBindings.append(CIMKeyBinding("Name",
        name, CIMKeyBinding::STRING));
    CIMObjectPath path("", CIMNamespaceName(),
        CIMName("PG_ProviderModule"), keyBindings);
    client.deleteInstance(PEGASUS_NAMESPACENAME_INTEROP, path);
}

void _createFilterInstance(
    CIMClient& client,
    const String& name,
    const String& query,
    const String& qlang)
{
    CIMInstance filterInstance(PEGASUS_CLASSNAME_INDFILTER);
    filterInstance.addProperty(CIMProperty(CIMName
       ("SystemCreationClassName"), System::getSystemCreationClassName()));
    filterInstance.addProperty(CIMProperty(CIMName("SystemName"),
        System::getFullyQualifiedHostName()));
    filterInstance.addProperty(CIMProperty(CIMName("CreationClassName"),
        PEGASUS_CLASSNAME_INDFILTER.getString()));
    filterInstance.addProperty(CIMProperty(CIMName("Name"), name));
    filterInstance.addProperty(CIMProperty(CIMName("Query"), query));
    filterInstance.addProperty(CIMProperty(CIMName("QueryLanguage"),
        String(qlang)));
    filterInstance.addProperty(CIMProperty(CIMName("SourceNamespace"),
        NAMESPACE.getString()));

    CIMObjectPath path = client.createInstance(PEGASUS_NAMESPACENAME_INTEROP,
        filterInstance);
}

void _createHandlerInstance(
    CIMClient& client,
    const String& name,
    const String& destination)
{
    CIMInstance handlerInstance(PEGASUS_CLASSNAME_LSTNRDST_CIMXML);
    handlerInstance.addProperty(CIMProperty(CIMName
       ("SystemCreationClassName"), System::getSystemCreationClassName()));
    handlerInstance.addProperty(CIMProperty(CIMName("SystemName"),
        System::getFullyQualifiedHostName()));
    handlerInstance.addProperty(CIMProperty(CIMName("CreationClassName"),
        PEGASUS_CLASSNAME_LSTNRDST_CIMXML.getString()));
    handlerInstance.addProperty(CIMProperty(CIMName("Name"), name));
    handlerInstance.addProperty(CIMProperty(CIMName("Destination"),
        destination));

    CIMObjectPath path = client.createInstance(PEGASUS_NAMESPACENAME_INTEROP,
        handlerInstance);
}

CIMObjectPath _buildFilterOrHandlerPath(
    const CIMName& className,
    const String& name,
    const String& host,
    const CIMNamespaceName& namespaceName = CIMNamespaceName())
{
    CIMObjectPath path;

    Array<CIMKeyBinding> keyBindings;
    keyBindings.append(CIMKeyBinding("SystemCreationClassName",
        System::getSystemCreationClassName(), CIMKeyBinding::STRING));
    keyBindings.append(CIMKeyBinding("SystemName",
        System::getFullyQualifiedHostName(), CIMKeyBinding::STRING));
    keyBindings.append(CIMKeyBinding("CreationClassName",
        className.getString(), CIMKeyBinding::STRING));
    keyBindings.append(CIMKeyBinding("Name", name, CIMKeyBinding::STRING));
    path.setClassName(className);
    path.setKeyBindings(keyBindings);
    path.setNameSpace(namespaceName);
    path.setHost(host);

    return path;
}

void _createSubscriptionInstance(
    CIMClient& client,
    const CIMObjectPath& filterPath,
    const CIMObjectPath& handlerPath,
    Uint16 onFatalErrorPolicy)
{
    CIMInstance subscriptionInstance(PEGASUS_CLASSNAME_INDSUBSCRIPTION);
    subscriptionInstance.addProperty(CIMProperty(CIMName("Filter"),
        filterPath, 0, PEGASUS_CLASSNAME_INDFILTER));
    subscriptionInstance.addProperty(CIMProperty(CIMName("Handler"),
        handlerPath, 0, PEGASUS_CLASSNAME_LSTNRDST_CIMXML));
    subscriptionInstance.addProperty(CIMProperty(
        CIMName("SubscriptionState"), CIMValue((Uint16) 2)));
    subscriptionInstance.addProperty(
        CIMProperty(
            CIMName("OnFatalErrorPolicy"),
            CIMValue((Uint16) onFatalErrorPolicy)));
    CIMObjectPath path = client.createInstance(PEGASUS_NAMESPACENAME_INTEROP,
        subscriptionInstance);
}

void _createSubscription(
    CIMClient& client,
    const String& filterName,
    Uint16 onFatalErrorPolicy = 2)
{
    CIMObjectPath filterPath;
    CIMObjectPath handlerPath;
    filterPath = _buildFilterOrHandlerPath(
        PEGASUS_CLASSNAME_INDFILTER, filterName, String::EMPTY,
        CIMNamespaceName());
    handlerPath = _buildFilterOrHandlerPath(
        PEGASUS_CLASSNAME_LSTNRDST_CIMXML, "OOPHandler01", String::EMPTY,
        CIMNamespaceName());
    _createSubscriptionInstance(
        client,
        filterPath,
        handlerPath,
        onFatalErrorPolicy);
}

CIMObjectPath _getSubscriptionPath(
    const String& filterName,
    const String& handlerName)
{
    Array<CIMKeyBinding> filterKeyBindings;
    filterKeyBindings.append(CIMKeyBinding("SystemCreationClassName",
        System::getSystemCreationClassName(), CIMKeyBinding::STRING));
    filterKeyBindings.append(CIMKeyBinding("SystemName",
        System::getFullyQualifiedHostName(), CIMKeyBinding::STRING));
    filterKeyBindings.append(CIMKeyBinding("CreationClassName",
        PEGASUS_CLASSNAME_INDFILTER.getString(), CIMKeyBinding::STRING));
    filterKeyBindings.append(CIMKeyBinding("Name", filterName,
        CIMKeyBinding::STRING));
    CIMObjectPath filterPath("", CIMNamespaceName(),
        PEGASUS_CLASSNAME_INDFILTER, filterKeyBindings);

    Array<CIMKeyBinding> handlerKeyBindings;
    handlerKeyBindings.append(CIMKeyBinding("SystemCreationClassName",
        System::getSystemCreationClassName(), CIMKeyBinding::STRING));
    handlerKeyBindings.append(CIMKeyBinding("SystemName",
        System::getFullyQualifiedHostName(), CIMKeyBinding::STRING));
    handlerKeyBindings.append(CIMKeyBinding("CreationClassName",
        PEGASUS_CLASSNAME_LSTNRDST_CIMXML.getString(),
        CIMKeyBinding::STRING));
    handlerKeyBindings.append(CIMKeyBinding("Name", handlerName,
        CIMKeyBinding::STRING));
    CIMObjectPath handlerPath("", CIMNamespaceName(),
        PEGASUS_CLASSNAME_LSTNRDST_CIMXML, handlerKeyBindings);

    Array<CIMKeyBinding> subscriptionKeyBindings;
    subscriptionKeyBindings.append(CIMKeyBinding("Filter",
        filterPath.toString(), CIMKeyBinding::REFERENCE));
    subscriptionKeyBindings.append(CIMKeyBinding("Handler",
        handlerPath.toString(), CIMKeyBinding::REFERENCE));

    return CIMObjectPath(
               "",
               CIMNamespaceName(),
               PEGASUS_CLASSNAME_INDSUBSCRIPTION,
               subscriptionKeyBindings);
}

void _deleteSubscriptionInstance(
    CIMClient& client,
    const String& filterName,
    const String& handlerName)
{
    client.deleteInstance(
        PEGASUS_NAMESPACENAME_INTEROP,
        _getSubscriptionPath(filterName, handlerName));
}

void _deleteHandlerInstance(
    CIMClient& client,
    const String& name)
{
    Array<CIMKeyBinding> keyBindings;
    keyBindings.append(CIMKeyBinding("SystemCreationClassName",
        System::getSystemCreationClassName(), CIMKeyBinding::STRING));
    keyBindings.append(CIMKeyBinding("SystemName",
        System::getFullyQualifiedHostName(), CIMKeyBinding::STRING));
    keyBindings.append(CIMKeyBinding("CreationClassName",
        PEGASUS_CLASSNAME_LSTNRDST_CIMXML.getString(),
        CIMKeyBinding::STRING));
    keyBindings.append(CIMKeyBinding("Name", name,
        CIMKeyBinding::STRING));
    CIMObjectPath path("", CIMNamespaceName(),
        PEGASUS_CLASSNAME_LSTNRDST_CIMXML, keyBindings);
    client.deleteInstance(PEGASUS_NAMESPACENAME_INTEROP, path);
}

void _deleteFilterInstance(
    CIMClient& client,
    const String& name)
{
    Array<CIMKeyBinding> keyBindings;
    keyBindings.append(CIMKeyBinding("SystemCreationClassName",
        System::getSystemCreationClassName(), CIMKeyBinding::STRING));
    keyBindings.append(CIMKeyBinding("SystemName",
        System::getFullyQualifiedHostName(), CIMKeyBinding::STRING));
    keyBindings.append(CIMKeyBinding("CreationClassName",
        PEGASUS_CLASSNAME_INDFILTER.getString(), CIMKeyBinding::STRING));
    keyBindings.append(CIMKeyBinding("Name", name,
        CIMKeyBinding::STRING));
    CIMObjectPath path("", CIMNamespaceName(),
        PEGASUS_CLASSNAME_INDFILTER, keyBindings);
    client.deleteInstance(PEGASUS_NAMESPACENAME_INTEROP, path);
}

Uint32 _getValue(
    CIMClient &client,
    const String &className,
    const CIMName &methodName)
{
    Array<CIMParamValue> inParams;
    Array<CIMParamValue> outParams;

    CIMObjectPath instName =
        CIMObjectPath(className);

    CIMValue returnValue = client.invokeMethod(
        NAMESPACE,
        instName,
        methodName,
        inParams,
        outParams);
    Uint32 rc;
    returnValue.get(rc);

    return rc;
}

void _validateValue(
    CIMClient &client,
    const String &className,
    const CIMName &methodName,
    Uint32 expectedValue)
{
    Uint32 iteration = 0;
    Boolean expectedValueObserved = false;
    while (iteration < 300)
    {
        iteration++;
        if (_getValue(client, className, methodName) == expectedValue)
        {
            expectedValueObserved = true;
            break;
        }
        else
        {
            System::sleep(1);
        }
    }

    PEGASUS_TEST_ASSERT(expectedValueObserved);
}

void _setup(CIMClient& client)
{
    //
    //  Create Filters and Handler for subscriptions
    //
    _createFilterInstance(client, String("OOPFilter01"),
        String("SELECT * FROM Test_GroupingClass1"), "WQL");

    _createFilterInstance(client, String("OOPFilter02"),
        String("SELECT * FROM Test_GroupingClass2"), "WQL");

    _createHandlerInstance(client, String("OOPHandler01"),
        String("localhost/CIMListener/Pegasus_SimpleDisplayConsumer"));
}

void _cleanup(CIMClient& client)
{
    //
    //  Delete Filters and Handler for subscriptions
    //
    _deleteHandlerInstance(client, String("OOPHandler01"));
    _deleteFilterInstance(client, String("OOPFilter01"));
    _deleteFilterInstance(client, String("OOPFilter02"));
}

void _register(
    CIMClient& client,
    Uint16 userContext,
    const String& providerModuleName,
    const String& location,
    const String& providerName,
    const String& capabilityID,
    const String& className,
    const String& groupName,
    const Array<String>& namespaces,
    const Array<Uint16>& providerType,
    const CIMPropertyList& supportedProperties,
    const CIMPropertyList& supportedMethods)
{
    //
    //  Create provider module instance
    //
    _createModuleInstance(
        client,
        providerModuleName,
        location,
        groupName,
        userContext);

    //
    //  Create the provider and capability instances
    //
    _createProviderInstance(
        client,
        providerName,
        providerModuleName);

    _createCapabilityInstance(
        client,
        providerModuleName,
        providerName,
        capabilityID,
        className,
        namespaces,
        providerType,
        supportedProperties,
        supportedMethods);
}

void _deregister(
    CIMClient& client,
    const String& providerModuleName,
    const String& providerName,
    const String& capabilityID)
{
    _deleteCapabilityInstance(
        client,
        providerModuleName,
        providerName,
        capabilityID);

    _deleteProviderInstance(
        client,
        providerName,
        providerModuleName);

    _deleteModuleInstance(
        client,
        providerModuleName);
}

static void _registerProviders2(
    CIMClient &client, const String &id, Uint32 userContext = 0)
{
    Array<String> namespaces;
    namespaces.append("test/TestProvider");

    if (verbose)
    {
        cout << "registering the providers..." << endl;
    }

    Array<Uint16> providerType;
    providerType.append(_METHOD_PROVIDER);
    providerType.append(_INDICATION_PROVIDER);

    _register(
        client, // client
        userContext, // userContext
        "TestGroupingProviderModule" + id, //providerModuleName
        "TestGroupingProvider" + id, // location,
        "TestGroupingProvider" + id, // providerName
        "TestGroupingProviderCapability" + id, // capabilityID
        "Test_GroupingClass" + id , // className
        "TestGroup1", //groupName
        namespaces,
        providerType,
        CIMPropertyList(),
        CIMPropertyList());
}

static void _deregisterProviders2(CIMClient &client, const String &id)
{
    if (verbose)
    {
        cout << "deregistering the providers..." << endl;
    }

    _deregister(
        client,
        "TestGroupingProviderModule" + id,
        "TestGroupingProvider" + id,
        "TestGroupingProviderCapability" + id);
}

static void _registerProviders(CIMClient &client, Uint32 userContext = 0)
{
    _registerProviders2(client, "1", userContext);
    _registerProviders2(client, "2", userContext);
}

static void _deregisterProviders(CIMClient &client)
{
    _deregisterProviders2(client, "1");
    _deregisterProviders2(client, "2");
}

static void _testGrouping(CIMClient &client)
{
    try
    {
        if (verbose)
        {
            cout << "Testing module grouping..." <<  endl;
        }

        PEGASUS_TEST_ASSERT(
            _getValue(client, "Test_GroupingClass1", "resetIdentifier") == 0);
        PEGASUS_TEST_ASSERT(
            _getValue(client, "Test_GroupingClass1", "getNextIdentifier") == 1);
        PEGASUS_TEST_ASSERT(
            _getValue(client, "Test_GroupingClass2", "getNextIdentifier") == 2);
        PEGASUS_TEST_ASSERT(
            _getValue(client, "Test_GroupingClass1", "getNextIdentifier") == 3);
        PEGASUS_TEST_ASSERT(
            _getValue(client, "Test_GroupingClass2", "getNextIdentifier") == 4);
        PEGASUS_TEST_ASSERT(
            _getValue(client, "Test_GroupingClass1", "resetIdentifier") == 0);
        PEGASUS_TEST_ASSERT(
            _getValue(client, "Test_GroupingClass2", "getNextIdentifier") == 1);

    }
    catch (Exception& e)
    {
        cerr << "Error: " << e.getMessage() << endl;
        exit(1);
    }
}

static void _testGroupingWithProviderDisabled(CIMClient &client)
{
    try
    {
        if (verbose)
        {
            cout << "Testing module grouping with provider disabled..." << endl;
        }

        PEGASUS_TEST_ASSERT(
            _getValue(client, "Test_GroupingClass1", "getNextIdentifier") == 2);
        PEGASUS_TEST_ASSERT(
            _getValue(client, "Test_GroupingClass1", "getNextIdentifier") == 3);
        PEGASUS_TEST_ASSERT(
            _getValue(client, "Test_GroupingClass1", "resetIdentifier") == 0);
        PEGASUS_TEST_ASSERT(
            _getValue(client, "Test_GroupingClass1", "getNextIdentifier") == 1);
    }
    catch (Exception& e)
    {
        cerr << "Error: " << e.getMessage() << endl;
        exit(1);
    }
}

static void _testIndications(CIMClient &client, Boolean moduleRestart)
{
    try
    {
        if (verbose)
        {
            cout << "Testing indication provider grouping failure..." << endl;
        }
        _setup(client);
        _registerProviders(client, 2);
        _createSubscription(client, String("OOPFilter01"));
        _createSubscription(client, String("OOPFilter02"));
        Array<CIMParamValue> outParams;
        Array<CIMParamValue> inParams;
        try
        {
            client.invokeMethod(
                NAMESPACE,
                CIMObjectPath("Test_GroupingClass1"),
                "testFail",
                inParams,
                outParams);
        }
        catch(...)
        {
        }

        if (moduleRestart)
        {
            _checkStatus(
                client,
                "TestGroupingProviderModule1",
                CIM_MSE_OPSTATUS_VALUE_OK);

            _checkStatus(
                client,
                "TestGroupingProviderModule2",
                CIM_MSE_OPSTATUS_VALUE_OK);

            Boolean enabled = false;
            while (!enabled)
            {
                try
                {
                    _validateValue(
                        client,
                        "Test_GroupingClass1",
                        "getSubscriptionCount",
                        1);

                    _validateValue(
                        client,
                        "Test_GroupingClass2",
                        "getSubscriptionCount",
                        1);

                    client.invokeMethod(
                        NAMESPACE,
                        CIMObjectPath("Test_GroupingClass1"),
                        "testFail",
                        inParams,
                        outParams);
                }
                catch(const CIMException&e)
                {
                    if (e.getCode() != CIM_ERR_NOT_SUPPORTED)
                    {
                        enabled = true;
                    }
                }
            }
        }
        _checkStatus(
            client,
            "TestGroupingProviderModule1",
            CIM_MSE_OPSTATUS_VALUE_DEGRADED);

        _checkStatus(
            client,
            "TestGroupingProviderModule2",
            CIM_MSE_OPSTATUS_VALUE_DEGRADED);

        _deleteSubscriptionInstance(client, String("OOPFilter01"),
            String("OOPHandler01"));
        _deleteSubscriptionInstance(client, String("OOPFilter02"),
            String("OOPHandler01"));
        _cleanup(client);
        _deregisterProviders(client);
    }
    catch (Exception& e)
    {
        cerr << "Error: " << e.getMessage() << endl;
        exit(1);
    }
}

int main(int, char** argv)
{
    verbose = getenv("PEGASUS_TEST_VERBOSE") ? true : false;

    try
    {
        CIMClient client;
        client.connectLocal();

        if (!strcmp(argv[1], "testGrouping"))
        {
            _testGrouping(client);
        }
        if (!strcmp(argv[1], "testGroupingWithProviderDisabled"))
        {
            _testGroupingWithProviderDisabled(client);
        }
        else if (!strcmp(argv[1], "registerProviders"))
        {
            _registerProviders(client);
        }
        else if (!strcmp(argv[1], "deregisterProviders"))
        {
            _deregisterProviders(client);
        }
        else if (!strcmp(argv[1], "testIndications"))
        {
#ifndef PEGASUS_DISABLE_PROV_USERCTXT
            Boolean moduleRestart = false;
            if (argv[2] && !strcmp(argv[2], "restart"))
            {
                moduleRestart = true;
            }
            _testIndications(client, moduleRestart);
#else
            cout << argv [0] <<
                ": Grouping indication providers tests skipped because"
                " PEGASUS_DISABLE_PROV_USERCTXT is defined" << endl;
#endif

        }
    }
    catch (Exception& e)
    {
        cerr << "Error: " << e.getMessage() << endl;
        exit(1);
    }

    cout << argv[0] << " +++++ passed all tests" << endl;

    return 0;
}
