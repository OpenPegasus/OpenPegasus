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
#include <Pegasus/Common/System.h>
#include <Pegasus/Common/PegasusAssert.h>
#include <Pegasus/Client/CIMClient.h>

#include <Providers/TestProviders/TestProviderRegistration.h>

PEGASUS_USING_PEGASUS;
PEGASUS_USING_STD;

static const CIMNamespaceName SOURCENAMESPACE =
    CIMNamespaceName("test/TestProvider");

static const String FILTER_NAME = "ICFilter01";
static const String HANDLER_NAME = "ICHandler01";
static const String FILTER_NAME2 = "ICFilter02";
static const String HANDLER_NAME2 = "ICHandler02";

static Uint32 _matchedIndicationsCount = 5;
static Uint32 _orphanIndicationCount = 1;

CIMObjectPath _createHandlerInstance(
    CIMClient& client,
    const String& name,
    const String& destination)
{
    CIMInstance handlerInstance(PEGASUS_CLASSNAME_INDHANDLER_CIMXML);
    handlerInstance.addProperty(CIMProperty(
        CIMName("SystemCreationClassName"),
        System::getSystemCreationClassName()));
    handlerInstance.addProperty(CIMProperty(
        CIMName("SystemName"),
        System::getFullyQualifiedHostName()));
    handlerInstance.addProperty(CIMProperty(
        CIMName("CreationClassName"),
        PEGASUS_CLASSNAME_INDHANDLER_CIMXML.getString()));
    handlerInstance.addProperty(CIMProperty(CIMName("Name"), name));
    handlerInstance.addProperty(CIMProperty(
        CIMName("Destination"),
        destination));

    return client.createInstance(
        PEGASUS_NAMESPACENAME_INTEROP,
        handlerInstance);
}

CIMObjectPath _createFilterInstance(
    CIMClient& client,
    const String& name,
    const String& query,
    const String& qlang)
{
    CIMInstance filterInstance(PEGASUS_CLASSNAME_INDFILTER);
    filterInstance.addProperty(CIMProperty(
        CIMName("SystemCreationClassName"),
        System::getSystemCreationClassName()));
    filterInstance.addProperty(CIMProperty(
        CIMName("SystemName"),
        System::getFullyQualifiedHostName()));
    filterInstance.addProperty(CIMProperty(
        CIMName("CreationClassName"),
        PEGASUS_CLASSNAME_INDFILTER.getString()));
    filterInstance.addProperty(CIMProperty(CIMName("Name"), name));
    filterInstance.addProperty(CIMProperty(CIMName("Query"), query));
    filterInstance.addProperty(CIMProperty(CIMName("QueryLanguage"), qlang));
    filterInstance.addProperty(CIMProperty(
        CIMName("SourceNamespace"),
        SOURCENAMESPACE.getString()));

    return client.createInstance(PEGASUS_NAMESPACENAME_INTEROP, filterInstance);
}

CIMObjectPath _getFilterObjectPath(const String& name)
{
    Array<CIMKeyBinding> keyBindings;
    keyBindings.append(CIMKeyBinding(
        "SystemCreationClassName",
        System::getSystemCreationClassName(),
        CIMKeyBinding::STRING));
    keyBindings.append(CIMKeyBinding(
        "SystemName",
        System::getFullyQualifiedHostName(),
        CIMKeyBinding::STRING));
    keyBindings.append(CIMKeyBinding(
        "CreationClassName",
        PEGASUS_CLASSNAME_INDFILTER.getString(),
        CIMKeyBinding::STRING));
    keyBindings.append(CIMKeyBinding("Name", name, CIMKeyBinding::STRING));

    return CIMObjectPath(
        String::EMPTY,
        CIMNamespaceName(),
        PEGASUS_CLASSNAME_INDFILTER,
        keyBindings);
}

CIMObjectPath _getHandlerObjectPath(const String& name)
{
    Array<CIMKeyBinding> keyBindings;
    keyBindings.append(CIMKeyBinding(
        "SystemCreationClassName",
        System::getSystemCreationClassName(),
        CIMKeyBinding::STRING));
    keyBindings.append(CIMKeyBinding(
        "SystemName",
        System::getFullyQualifiedHostName(),
        CIMKeyBinding::STRING));
    keyBindings.append(CIMKeyBinding(
        "CreationClassName",
        PEGASUS_CLASSNAME_INDHANDLER_CIMXML.getString(),
        CIMKeyBinding::STRING));
    keyBindings.append(CIMKeyBinding("Name", name, CIMKeyBinding::STRING));

    return CIMObjectPath(
        String::EMPTY,
        CIMNamespaceName(),
        PEGASUS_CLASSNAME_INDHANDLER_CIMXML,
        keyBindings);
}

void _createSubscriptionInstance(
    CIMClient& client,
    const CIMObjectPath& filterPath,
    const CIMObjectPath& handlerPath)
{
    CIMInstance subscriptionInstance(PEGASUS_CLASSNAME_INDSUBSCRIPTION);
    subscriptionInstance.addProperty(CIMProperty(
        CIMName("Filter"),
        filterPath,
        0,
        PEGASUS_CLASSNAME_INDFILTER));
    subscriptionInstance.addProperty(CIMProperty(
        CIMName("Handler"),
        handlerPath,
        0,
        PEGASUS_CLASSNAME_INDHANDLER_CIMXML));
    subscriptionInstance.addProperty(CIMProperty(
        CIMName("SubscriptionState"),
        CIMValue(Uint16(2))));

    client.createInstance(PEGASUS_NAMESPACENAME_INTEROP, subscriptionInstance);
}

void _sendTestIndication(
    CIMClient& client,
    const CIMName& methodName,
    Uint32 indicationSendCount)
{
    //
    //  Invoke method to send test indication
    //
    Array<CIMParamValue> inParams;
    Array<CIMParamValue> outParams;
    Array<CIMKeyBinding> keyBindings;
    Sint32 result;

    CIMObjectPath className(
        String::EMPTY,
        CIMNamespaceName(),
        CIMName("Test_IndicationProviderClass"),
        keyBindings);

    inParams.append(CIMParamValue(
        String("indicationSendCount"),
        CIMValue(indicationSendCount)));

    CIMValue retValue = client.invokeMethod(
        SOURCENAMESPACE,
        className,
        methodName,
        inParams,
        outParams);

    retValue.get(result);
    PEGASUS_TEST_ASSERT(result == 0);

    //
    //  Allow time for the indication to be received and forwarded
    //
    System::sleep(2);
}

void _disableModule(
    CIMClient& client,
    const String& providerModuleName)
{
    //
    // Invoke method to disable module
    //
    Array<CIMParamValue> inParams;
    Array<CIMParamValue> outParams;
    CIMObjectPath moduleRef;
    Array<CIMKeyBinding> keyBindings;
    Sint16 result;

    keyBindings.append(CIMKeyBinding(
        "Name",
        providerModuleName,
        CIMKeyBinding::STRING));

    moduleRef.setNameSpace(PEGASUS_NAMESPACENAME_PROVIDERREG);
    moduleRef.setClassName(PEGASUS_CLASSNAME_PROVIDERMODULE);
    moduleRef.setKeyBindings(keyBindings);

    CIMValue retValue = client.invokeMethod(
        PEGASUS_NAMESPACENAME_PROVIDERREG,
        moduleRef,
        CIMName("stop"),
        inParams,
        outParams);

    retValue.get(result);
    PEGASUS_TEST_ASSERT(result == 0);
}

void _enableModule(
    CIMClient& client,
    const String& providerModuleName)
{
    //
    // Invoke method to enable module
    //
    Array<CIMParamValue> inParams;
    Array<CIMParamValue> outParams;
    CIMObjectPath moduleRef;
    Array<CIMKeyBinding> keyBindings;
    Sint16 result;

    keyBindings.append(CIMKeyBinding(
        "Name",
        providerModuleName,
        CIMKeyBinding::STRING));

    moduleRef.setNameSpace(PEGASUS_NAMESPACENAME_PROVIDERREG);
    moduleRef.setClassName(PEGASUS_CLASSNAME_PROVIDERMODULE);
    moduleRef.setKeyBindings(keyBindings);

    CIMValue retValue = client.invokeMethod(
        PEGASUS_NAMESPACENAME_PROVIDERREG,
        moduleRef,
        CIMName("start"),
        inParams,
        outParams);

    retValue.get(result);
    PEGASUS_TEST_ASSERT(result == 0);
}

void _deleteSubscriptionInstance(
    CIMClient& client,
    const String& filterName,
    const String& handlerName)
{
    CIMObjectPath filterPath = _getFilterObjectPath(filterName);
    CIMObjectPath handlerPath = _getHandlerObjectPath(handlerName);

    Array<CIMKeyBinding> subscriptionKeyBindings;
    subscriptionKeyBindings.append(CIMKeyBinding(
        "Filter", filterPath.toString(), CIMKeyBinding::REFERENCE));
    subscriptionKeyBindings.append(CIMKeyBinding(
        "Handler", handlerPath.toString(), CIMKeyBinding::REFERENCE));
    CIMObjectPath subscriptionPath(
        String::EMPTY,
        CIMNamespaceName(),
        PEGASUS_CLASSNAME_INDSUBSCRIPTION,
        subscriptionKeyBindings);
    client.deleteInstance(PEGASUS_NAMESPACENAME_INTEROP, subscriptionPath);
}

void _deleteHandlerInstance(
    CIMClient& client,
    const String& name)
{
    CIMObjectPath path = _getHandlerObjectPath(name);
    client.deleteInstance(PEGASUS_NAMESPACENAME_INTEROP, path);
}

void _deleteFilterInstance(
    CIMClient& client,
    const String& name)
{
    CIMObjectPath path = _getFilterObjectPath(name);
    client.deleteInstance(PEGASUS_NAMESPACENAME_INTEROP, path);
}

void _setup(CIMClient& client)
{
    CIMObjectPath filterObjectPath;
    CIMObjectPath handlerObjectPath;

    try
    {
        filterObjectPath = _createFilterInstance(
            client,
            FILTER_NAME,
            String("SELECT * FROM Test_IndicationProviderClass"),
            "WQL");
    }
    catch (CIMException& e)
    {
        if (e.getCode() == CIM_ERR_ALREADY_EXISTS)
        {
            filterObjectPath = _getFilterObjectPath(FILTER_NAME);
            cerr << "----- Warning: Filter Instance Not Created: "
                << e.getMessage() << endl;
        }
        else
        {
            cerr << "----- Error: Filter Instance Not Created: " << endl;
            throw;
        }
    }

    try
    {
        handlerObjectPath = _createHandlerInstance(
            client,
            HANDLER_NAME,
            String("localhost/CIMListener/Pegasus_SimpleDisplayConsumer"));
    }
    catch (CIMException& e)
    {
        if (e.getCode() == CIM_ERR_ALREADY_EXISTS)
        {
            handlerObjectPath = _getHandlerObjectPath(HANDLER_NAME);
            cerr << "----- Warning: Handler Instance Not Created: "
                << e.getMessage() << endl;
        }
        else
        {
            cerr << "----- Error: Handler Instance Not Created: " << endl;
            throw;
        }
    }

    try
    {
        _createSubscriptionInstance(
            client, filterObjectPath, handlerObjectPath);
    }
    catch (CIMException& e)
    {
        if (e.getCode() == CIM_ERR_ALREADY_EXISTS)
        {
            cerr << "----- Warning: Client Subscription Instance: "
                << e.getMessage () << endl;
        }
        else
        {
            cerr << "----- Error: Client Subscription Instance: " << endl;
            throw;
        }
    }
    cout << "+++++ setup completed successfully" << endl;
}

void _createIndicationsCountSubscription(CIMClient& client)
{
    CIMObjectPath filterObjectPath;
    CIMObjectPath handlerObjectPath;

    try
    {
        filterObjectPath = _createFilterInstance(
            client,
            FILTER_NAME2,
            String("SELECT * FROM CIM_ProcessIndication"),
            String("WQL"));
    }
    catch (CIMException& e)
    {
        if (e.getCode() == CIM_ERR_ALREADY_EXISTS)
        {
            filterObjectPath = _getFilterObjectPath(FILTER_NAME2);
            cerr << "----- Warning: Filter Instance Not Created: "
                << e.getMessage() << endl;
        }
        else
        {
            cerr << "----- Error: Filter Instance Not Created: " << endl;
            throw;
        }
    }

    try
    {
        handlerObjectPath = _createHandlerInstance(
            client,
            HANDLER_NAME2,
            String("localhost/CIMListener/Pegasus_SimpleDisplayConsumer"));
    }
    catch (CIMException& e)
    {
        if (e.getCode() == CIM_ERR_ALREADY_EXISTS)
        {
            handlerObjectPath = _getHandlerObjectPath(HANDLER_NAME2);
            cerr << "----- Warning: Handler Instance Not Created: "
                << e.getMessage() << endl;
        }
        else
        {
            cerr << "----- Error: Handler Instance Not Created: " << endl;
            throw;
        }
    }

    try
    {
        _createSubscriptionInstance(
            client, filterObjectPath, handlerObjectPath);
    }
    catch (CIMException& e)
    {
        if (e.getCode() == CIM_ERR_ALREADY_EXISTS)
        {
            cerr << "----- Warning: Client Subscription Instance: "
                << e.getMessage () << endl;
        }
        else
        {
            cerr << "----- Error: Client Subscription Instance: " << endl;
            throw;
        }
    }
}

void _generateTestIndications(CIMClient& client)
{
    // Generate matched indications
    _sendTestIndication(
        client,
        "SendTestIndicationsCount",
        _matchedIndicationsCount);

    // Generate orphan indication
    _sendTestIndication(
        client,
        "SendTestIndicationUnmatchingClassName",
        _orphanIndicationCount);

    cout << "+++++ generate indications completed successfully" << endl;
}

CIMObjectPath _buildProviderIndicationDataInstanceName(
    const String& providerModuleName,
    const String& providerName)
{
    CIMObjectPath instanceName;
    Array<CIMKeyBinding> keyBindings;
    keyBindings.append(CIMKeyBinding(
        "ProviderModuleName",
        providerModuleName,
        CIMKeyBinding::STRING));
    keyBindings.append(CIMKeyBinding(
        "ProviderName",
        providerName,
        CIMKeyBinding::STRING));

    instanceName.setClassName(PEGASUS_CLASSNAME_PROVIDERINDDATA);
    instanceName.setKeyBindings(keyBindings);
    return instanceName;
}

CIMInstance _buildProviderIndicationDataInstance(
    const String& providerModuleName,
    const String& providerName,
    Uint32 indicationsCount,
    Uint32 orphanIndicationCount,
    Boolean includeObjectPath = true)
{
    CIMInstance providerIndDataInstance(PEGASUS_CLASSNAME_PROVIDERINDDATA);
    providerIndDataInstance.addProperty(CIMProperty(
        CIMName("ProviderModuleName"),
        String(providerModuleName)));
    providerIndDataInstance.addProperty(CIMProperty(
        CIMName("ProviderName"),
        String(providerName)));
    providerIndDataInstance.addProperty(CIMProperty(
        CIMName("IndicationCount"),
        Uint32(indicationsCount)));
    providerIndDataInstance.addProperty(CIMProperty(
        CIMName("OrphanIndicationCount"),
        Uint32(orphanIndicationCount)));

    if (includeObjectPath)
    {
        CIMObjectPath instanceName = _buildProviderIndicationDataInstanceName(
            providerModuleName, providerName);
        providerIndDataInstance.setPath(instanceName);
    }

    return providerIndDataInstance;
}

CIMObjectPath _buildSubscriptionIndicationDataInstanceName(
    const String& filterName,
    const String& handlerName,
    const String& sourceNamespace,
    const String& providerModuleName,
    const String& providerName)
{
    CIMObjectPath path;
    Array<CIMKeyBinding> keyBindings;
    keyBindings.append(CIMKeyBinding(
        "FilterName",
        filterName,
        CIMKeyBinding::STRING));
    keyBindings.append(CIMKeyBinding(
        "HandlerName",
        handlerName,
        CIMKeyBinding::STRING));
    keyBindings.append(CIMKeyBinding(
        "SourceNamespace",
        sourceNamespace,
        CIMKeyBinding::STRING));
    keyBindings.append(CIMKeyBinding(
        "ProviderModuleName",
        providerModuleName,
        CIMKeyBinding::STRING));
    keyBindings.append(CIMKeyBinding(
        "ProviderName",
        providerName,
        CIMKeyBinding::STRING));

    path.setClassName(PEGASUS_CLASSNAME_SUBSCRIPTIONINDDATA);
    path.setKeyBindings(keyBindings);

    return path;
}

CIMInstance _buildSubscriptionIndicationDataInstance(
    const String& filterName,
    const String& handlerName,
    const String& sourceNS,
    const String& providerModuleName,
    const String& providerName,
    Uint32 matchedIndicationsCount,
    Boolean includeObjectPath = true)
{
    Array<CIMInstance> subIndDataInstances;

    CIMInstance subIndDataInstance(PEGASUS_CLASSNAME_SUBSCRIPTIONINDDATA);
    subIndDataInstance.addProperty(CIMProperty(
        CIMName("FilterName"),
        String(filterName)));
    subIndDataInstance.addProperty(CIMProperty(
        CIMName("HandlerName"),
        String(handlerName)));
    subIndDataInstance.addProperty(CIMProperty(
        CIMName("SourceNamespace"),
        String(sourceNS)));
    subIndDataInstance.addProperty(CIMProperty(
        CIMName("ProviderModuleName"),
        String(providerModuleName)));
    subIndDataInstance.addProperty(CIMProperty(
        CIMName("ProviderName"),
        String(providerName)));
    subIndDataInstance.addProperty(CIMProperty(
        CIMName("MatchedIndicationCount"),
        Uint32(matchedIndicationsCount)));

    if (includeObjectPath)
    {
        CIMObjectPath instanceName =
            _buildSubscriptionIndicationDataInstanceName(
                filterName,
                handlerName,
                sourceNS,
                providerModuleName,
                providerName);
        subIndDataInstance.setPath(instanceName);
    }

    return subIndDataInstance;
}


void _unregister(CIMClient& client)
{
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
        client, "ProcessIndicationProviderModule");
}

void _register(CIMClient& client)
{
    Array<String> namespaces;
    Array<Uint16> providerTypes;
    namespaces.append(SOURCENAMESPACE.getString());
    providerTypes.append(4);

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
    // creates a subscription served by this provider
    //
    _createIndicationsCountSubscription(client);

}

void _checkResult(
    const Array<CIMInstance> returnedProvIndDataInstances,
    const Array<CIMInstance> returnedSubIndDataInstances,
    const Array<CIMInstance> expectedProvIndDataInstances,
    const Array<CIMInstance> expectedSubIndDataInstances)
{
    Array<CIMInstance> matchedProvIndDataInstances;
    for (Uint32 i = 0; i < returnedProvIndDataInstances.size(); i++)
    {
        for (Uint32 j = 0; j < expectedProvIndDataInstances.size(); j++)
        {
            if (returnedProvIndDataInstances[i].identical(
                expectedProvIndDataInstances[j]))
            {
                matchedProvIndDataInstances.append(
                    returnedProvIndDataInstances[i]);
                break;
            }
        }
    }

    PEGASUS_TEST_ASSERT(
        matchedProvIndDataInstances.size() ==
        expectedProvIndDataInstances.size());

    Array<CIMInstance> matchedSubIndDataInstances;
    for (Uint32 i = 0; i < returnedSubIndDataInstances.size(); i++)
    {
        for (Uint32 j = 0; j < expectedSubIndDataInstances.size(); j++)
        {
            if (returnedSubIndDataInstances[i].identical(
                expectedSubIndDataInstances[j]))
            {
                matchedSubIndDataInstances.append(
                    returnedSubIndDataInstances[i]);
            }
        }
    }

    PEGASUS_TEST_ASSERT(
        matchedSubIndDataInstances.size() ==
        expectedSubIndDataInstances.size());
}

void _checkEnumInstanceNames(
    const Array<CIMObjectPath> returnedProvIndDataInstanceNames,
    const Array<CIMObjectPath> returnedSubIndDataInstanceNames,
    const Array<CIMObjectPath> expectedProvIndDataInstanceNames,
    const Array<CIMObjectPath> expectedSubIndDataInstanceNames)
{
    Array<CIMObjectPath> matchedProvIndDataInstanceNames;
    for (Uint32 i = 0; i < returnedProvIndDataInstanceNames.size(); i++)
    {
        for (Uint32 j = 0; j < expectedProvIndDataInstanceNames.size(); j++)
        {
            if (returnedProvIndDataInstanceNames[i].identical(
                expectedProvIndDataInstanceNames[j]))
            {
                matchedProvIndDataInstanceNames.append(
                    returnedProvIndDataInstanceNames[i]);
                break;
            }
        }
    }

    PEGASUS_TEST_ASSERT(
        matchedProvIndDataInstanceNames.size() ==
        expectedProvIndDataInstanceNames.size());

    Array<CIMObjectPath> matchedSubIndDataInstanceNames;
    for (Uint32 i = 0; i < returnedSubIndDataInstanceNames.size(); i++)
    {
        for (Uint32 j = 0; j < expectedSubIndDataInstanceNames.size(); j++)
        {
            if (returnedSubIndDataInstanceNames[i].identical(
                expectedSubIndDataInstanceNames[j]))
            {
                matchedSubIndDataInstanceNames.append(
                    returnedSubIndDataInstanceNames[i]);
                break;
            }
        }
    }

    PEGASUS_TEST_ASSERT(
        matchedSubIndDataInstanceNames.size() ==
        expectedSubIndDataInstanceNames.size());
}

void _cleanup(CIMClient& client)
{
    IGNORE_CIM_ERR_NOT_FOUND(
        _deleteSubscriptionInstance(client, FILTER_NAME, HANDLER_NAME),
        "----- Error: deleteSubscriptionInstance failure: ");

    IGNORE_CIM_ERR_NOT_FOUND(
        _deleteSubscriptionInstance(client, FILTER_NAME2, HANDLER_NAME2),
        "----- Error: deleteSubscriptionInstance failure: ");

    IGNORE_CIM_ERR_NOT_FOUND(
        _deleteFilterInstance(client, FILTER_NAME),
        "----- Error: deleteFilterInstance failure: ");

    IGNORE_CIM_ERR_NOT_FOUND(
        _deleteFilterInstance(client, FILTER_NAME2),
        "----- Error: deleteFilterInstance failure: ");

    IGNORE_CIM_ERR_NOT_FOUND(
        _deleteHandlerInstance(client, HANDLER_NAME),
        "----- Error: deleteHandlerInstance failure: ");

    IGNORE_CIM_ERR_NOT_FOUND(
        _deleteHandlerInstance(client, HANDLER_NAME2),
        "----- Error: deleteHandlerInstance failure: ");

    IGNORE_CIM_ERR_NOT_FOUND(
        TestProviderRegistration::deleteCapabilityInstance(
            client,
            "ProcessIndicationProviderModule",
            "ProcessIndicationProvider",
            "ProcessIndicationProviderCapability"),
        "----- Error: deleteCapabilityInstance failure: ");

    IGNORE_CIM_ERR_NOT_FOUND(
        TestProviderRegistration::deleteProviderInstance(
            client,
            "ProcessIndicationProvider",
            "ProcessIndicationProviderModule"),
        "----- Error: deleteProviderInstance failure: ");

    IGNORE_CIM_ERR_NOT_FOUND(
        TestProviderRegistration::deleteModuleInstance(
            client, "ProcessIndicationProviderModule"),
        "----- Error: deleteModuleInstance failure: ");

    cout << "+++++ cleanup completed successfully" << endl;
}

void _test(CIMClient& client)
{
    //
    // reset the table before the setup
    //
    _disableModule(client, "IndicationTestProviderModule");
    _enableModule(client, "IndicationTestProviderModule");

    _setup(client);
    _generateTestIndications(client);

    Uint32 indicationsCount =
        _matchedIndicationsCount + _orphanIndicationCount;

    //
    // test enumerate instances
    //

    Array<CIMInstance> expectedProvIndDataInstances;
    CIMInstance expectedProvIndDataInstance =
        _buildProviderIndicationDataInstance(
            "IndicationTestProviderModule",
            "IndicationTestProvider",
            indicationsCount,
            _orphanIndicationCount);
    expectedProvIndDataInstances.append(expectedProvIndDataInstance);

    Array<CIMInstance> expectedSubIndDataInstances;
    String interopNamespace = PEGASUS_NAMESPACENAME_INTEROP.getString();
    CIMInstance expectedSubIndDataInstance =
        _buildSubscriptionIndicationDataInstance(
            interopNamespace+":ICFilter01",
            interopNamespace+":CIM_IndicationHandlerCIMXML.ICHandler01",
            interopNamespace,
            "IndicationTestProviderModule",
            "IndicationTestProvider",
            _matchedIndicationsCount);
    expectedSubIndDataInstances.append(expectedSubIndDataInstance);

    Array<CIMInstance> returnedProvIndDataInstances =
        client.enumerateInstances(
            PEGASUS_NAMESPACENAME_INTERNAL,
            PEGASUS_CLASSNAME_PROVIDERINDDATA);

    Array<CIMInstance> returnedSubIndDataInstances =
        client.enumerateInstances(
            PEGASUS_NAMESPACENAME_INTERNAL,
            PEGASUS_CLASSNAME_SUBSCRIPTIONINDDATA);

    _checkResult(
        returnedProvIndDataInstances,
        returnedSubIndDataInstances,
        expectedProvIndDataInstances,
        expectedSubIndDataInstances);

    //
    // test enumerate instance names
    //
    CIMObjectPath instanceName =
        _buildProviderIndicationDataInstanceName(
            "IndicationTestProviderModule",
            "IndicationTestProvider");

    Array<CIMObjectPath> expectedProvIndDataInstanceNames;
    expectedProvIndDataInstanceNames.append(instanceName);

    CIMObjectPath subscriptionIndDataInstanceName =
        _buildSubscriptionIndicationDataInstanceName(
            interopNamespace+":ICFilter01",
            interopNamespace+":CIM_IndicationHandlerCIMXML.ICHandler01",
            interopNamespace,
            "IndicationTestProviderModule",
            "IndicationTestProvider");

    Array<CIMObjectPath> expectedSubIndDataInstanceNames;
    expectedSubIndDataInstanceNames.append(subscriptionIndDataInstanceName);

    Array<CIMObjectPath> returnedProvIndDataInstanceNames =
        client.enumerateInstanceNames(
            PEGASUS_NAMESPACENAME_INTERNAL,
            PEGASUS_CLASSNAME_PROVIDERINDDATA);

    Array<CIMObjectPath> returnedSubIndDataInstanceNames =
        client.enumerateInstanceNames(
            PEGASUS_NAMESPACENAME_INTERNAL,
            PEGASUS_CLASSNAME_SUBSCRIPTIONINDDATA);

    _checkEnumInstanceNames(
        returnedProvIndDataInstanceNames,
        returnedSubIndDataInstanceNames,
        expectedProvIndDataInstanceNames,
        expectedSubIndDataInstanceNames);

    //
    // test get instance
    //
    expectedProvIndDataInstance =
        _buildProviderIndicationDataInstance(
            "IndicationTestProviderModule",
            "IndicationTestProvider",
            indicationsCount,
            _orphanIndicationCount,
            false);

    CIMObjectPath providerObjPath = _buildProviderIndicationDataInstanceName(
        "IndicationTestProviderModule", "IndicationTestProvider");

    CIMInstance returnedProvIndDataInstance = client.getInstance(
        PEGASUS_NAMESPACENAME_INTERNAL, providerObjPath);

    PEGASUS_TEST_ASSERT(returnedProvIndDataInstance.identical(
        expectedProvIndDataInstance));

    expectedSubIndDataInstance =
        _buildSubscriptionIndicationDataInstance(
            interopNamespace+":ICFilter01",
            interopNamespace+":CIM_IndicationHandlerCIMXML.ICHandler01",
            interopNamespace,
            "IndicationTestProviderModule",
            "IndicationTestProvider",
            _matchedIndicationsCount,
            false);

    CIMObjectPath subObjPath = _buildSubscriptionIndicationDataInstanceName(
        interopNamespace+":ICFilter01",
        interopNamespace+":CIM_IndicationHandlerCIMXML.ICHandler01",
        interopNamespace,
        "IndicationTestProviderModule",
        "IndicationTestProvider");

    CIMInstance returnedSubIndDataInstance = client.getInstance(
    PEGASUS_NAMESPACENAME_INTERNAL, subObjPath);

    PEGASUS_TEST_ASSERT(returnedSubIndDataInstance.identical(
        expectedSubIndDataInstance));

    cout << "+++++ test completed successfully" << endl;
}

void _testReset(CIMClient& client)
{
    Array<CIMInstance> expectedProvIndDataInstances;
    Array<CIMInstance> expectedSubIndDataInstances;

    Uint32 indicationsCount =
        _matchedIndicationsCount + _orphanIndicationCount;

    _disableModule(client, "IndicationTestProviderModule");

    //
    // The entry of the tables gets removed if the provider is disabled
    //
    Array<CIMInstance> returnedProvIndDataInstances =
        client.enumerateInstances(
            PEGASUS_NAMESPACENAME_INTERNAL,
            PEGASUS_CLASSNAME_PROVIDERINDDATA);

    Array<CIMInstance> returnedSubIndDataInstances =
        client.enumerateInstances(
            PEGASUS_NAMESPACENAME_INTERNAL,
            PEGASUS_CLASSNAME_SUBSCRIPTIONINDDATA);

    _checkResult(
        returnedProvIndDataInstances,
        returnedSubIndDataInstances,
        expectedProvIndDataInstances,
        expectedSubIndDataInstances);

    //
    // register a provider, the provider entry should be inserted to tables
    // if a subscription needs to be served by the provider
    //
    _register(client);

    CIMInstance expectedProvIndDataInstance =
        _buildProviderIndicationDataInstance(
            "ProcessIndicationProviderModule",
            "ProcessIndicationProvider",
            0,
            0);
    expectedProvIndDataInstances.append(expectedProvIndDataInstance);
    String interopNamespace = PEGASUS_NAMESPACENAME_INTEROP.getString();
    CIMInstance expectedSubIndDataInstance =
        _buildSubscriptionIndicationDataInstance(
            interopNamespace+":ICFilter02",
            interopNamespace+":CIM_IndicationHandlerCIMXML.ICHandler02",
            interopNamespace,
            "ProcessIndicationProviderModule",
            "ProcessIndicationProvider",
            0);
    expectedSubIndDataInstances.append(expectedSubIndDataInstance);

    returnedProvIndDataInstances = client.enumerateInstances(
         PEGASUS_NAMESPACENAME_INTERNAL,
         PEGASUS_CLASSNAME_PROVIDERINDDATA);

    returnedSubIndDataInstances = client.enumerateInstances(
         PEGASUS_NAMESPACENAME_INTERNAL,
         PEGASUS_CLASSNAME_SUBSCRIPTIONINDDATA);

    _checkResult(
        returnedProvIndDataInstances,
        returnedSubIndDataInstances,
        expectedProvIndDataInstances,
        expectedSubIndDataInstances);

    //
    // unregister the provider, the provider entry should be removed from
    // tables
    //
    _unregister(client);

    Array<CIMInstance> provIndDataInstances;
    Array<CIMInstance> subIndDataInstances;

    returnedProvIndDataInstances = client.enumerateInstances(
        PEGASUS_NAMESPACENAME_INTERNAL,
        PEGASUS_CLASSNAME_PROVIDERINDDATA);

    returnedSubIndDataInstances = client.enumerateInstances(
        PEGASUS_NAMESPACENAME_INTERNAL,
        PEGASUS_CLASSNAME_SUBSCRIPTIONINDDATA);

    _checkResult(
        returnedProvIndDataInstances,
        returnedSubIndDataInstances,
        provIndDataInstances,
        subIndDataInstances);

    //
    // enable the disabled module, the provider entry should be inserted to
    // tables
    //
    _enableModule(client, "IndicationTestProviderModule");

    expectedProvIndDataInstance = _buildProviderIndicationDataInstance(
        "IndicationTestProviderModule",
        "IndicationTestProvider",
        0,
        0);
    expectedProvIndDataInstances.clear();
    expectedProvIndDataInstances.append(expectedProvIndDataInstance);

    CIMInstance instance =
        _buildSubscriptionIndicationDataInstance(
            interopNamespace+":ICFilter01",
            interopNamespace+":CIM_IndicationHandlerCIMXML.ICHandler01",
            interopNamespace,
            "IndicationTestProviderModule",
            "IndicationTestProvider",
            0);
    expectedSubIndDataInstances.clear();
    expectedSubIndDataInstances.append(instance);

    returnedProvIndDataInstances = client.enumerateInstances(
        PEGASUS_NAMESPACENAME_INTERNAL,
        PEGASUS_CLASSNAME_PROVIDERINDDATA);

    returnedSubIndDataInstances = client.enumerateInstances(
        PEGASUS_NAMESPACENAME_INTERNAL,
        PEGASUS_CLASSNAME_SUBSCRIPTIONINDDATA);

    _checkResult(
        returnedProvIndDataInstances,
        returnedSubIndDataInstances,
        expectedProvIndDataInstances,
        expectedSubIndDataInstances);

    _generateTestIndications(client);

    expectedProvIndDataInstance = _buildProviderIndicationDataInstance(
        "IndicationTestProviderModule",
        "IndicationTestProvider",
        indicationsCount,
        _orphanIndicationCount);
    expectedProvIndDataInstances.clear();
    expectedProvIndDataInstances.append(expectedProvIndDataInstance);

    expectedSubIndDataInstance =
        _buildSubscriptionIndicationDataInstance(
            interopNamespace+":ICFilter01",
            interopNamespace+":CIM_IndicationHandlerCIMXML.ICHandler01",
            interopNamespace,
            "IndicationTestProviderModule",
            "IndicationTestProvider",
            _matchedIndicationsCount);

    expectedSubIndDataInstances.clear();
    expectedSubIndDataInstances.append(expectedSubIndDataInstance);

    returnedProvIndDataInstances = client.enumerateInstances(
        PEGASUS_NAMESPACENAME_INTERNAL,
        PEGASUS_CLASSNAME_PROVIDERINDDATA);

    returnedSubIndDataInstances = client.enumerateInstances(
        PEGASUS_NAMESPACENAME_INTERNAL,
        PEGASUS_CLASSNAME_SUBSCRIPTIONINDDATA);

    _checkResult(
        returnedProvIndDataInstances,
        returnedSubIndDataInstances,
        expectedProvIndDataInstances,
        expectedSubIndDataInstances);

    cout << "+++++ testReset completed successfully" << endl;
}

int main(int argc, char** argv)
{
#ifdef PEGASUS_ENABLE_INDICATION_COUNT
    CIMClient client;
    try
    {
        client.connectLocal();

        if (argc == 1)
        {
            // tests normal path
            _test(client);

            // tests provider disable, enable, register, and unregister
            _testReset(client);

            _cleanup(client);
        }
        else if (argc == 2 && (String::equalNoCase(argv[1], "cleanup")))
        {
            _cleanup(client);
            return 0;
        }
        else
        {
            cerr << "Invalid option: " << argv[1] << endl;
            return -1;
        }
    }
    catch (Exception& e)
    {
        cerr << e.getMessage() << endl;
        return -1;
    }
#else
    cout << argv[0]
         << ": Tracking indications count is not enabled; test skipped"
         << endl;
#endif

    cout << "+++++ IndicationsCount passed all tests" << endl;
    return 0;
}
