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
#include <Pegasus/Common/Mutex.h>
#include <Pegasus/General/Stopwatch.h>
#include <Pegasus/Server/ProviderRegistrationManager/\
ProviderRegistrationManager.h>
#include <Pegasus/Consumer/CIMIndicationConsumer.h>
#include <Pegasus/Listener/CIMListener.h>
#include <Pegasus/Common/StringConversion.h>

PEGASUS_USING_PEGASUS;

PEGASUS_USING_STD;

static const CIMNamespaceName NAMESPACE = CIMNamespaceName("test/TestProvider");
static const Uint32 HANDLER_COUNT = 4;
static const Uint32 MAX_INDICATIONS = 5;
static const Uint32 MAX_WAITTIME = 600;
Sint64 NextSequenceNumber = 0;

static Boolean verbose;

class DeliveryRetryTestConsumer : public CIMIndicationConsumer
{
public:
    DeliveryRetryTestConsumer(const String &name);
    ~DeliveryRetryTestConsumer();

    void consumeIndication(const OperationContext& context,
        const String & url,
        const CIMInstance& indicationInstance);

    void clearIndications()
    {
        AutoMutex mtx(_mutex);
        _indications.clear();
    }

    Array<CIMInstance> getIndications()
    {
        AutoMutex mtx(_mutex);
        return _indications;
    }

    Uint32 getIndicationCount()
    {
        AutoMutex mtx(_mutex);
        return _indications.size();
    }
private:
    String name;
    Array<CIMInstance> _indications;
    Mutex _mutex;
};

DeliveryRetryTestConsumer::DeliveryRetryTestConsumer(const String &name)
{
    this->name = name;
}

DeliveryRetryTestConsumer::~DeliveryRetryTestConsumer()
{
}

void DeliveryRetryTestConsumer::consumeIndication(
    const OperationContext & context,
    const String & url,
    const CIMInstance& indication)
{
    AutoMutex mtx(_mutex);
    _indications.append(indication);
}

void _waitForIndications(
    CIMClient& client,
    DeliveryRetryTestConsumer *consumer,
    Uint32 expectedCount)
{
    Uint32 iteration = 0;
    Boolean expectedCountObserved = false;
    while (iteration < MAX_WAITTIME)
    {
        System::sleep(1);
        iteration++;
        if (consumer->getIndicationCount() == expectedCount)
        {
            expectedCountObserved = true;
            break;
        }
    }

#ifndef PEGASUS_TEST_VALGRIND_LOG_DIR
    PEGASUS_TEST_ASSERT(expectedCountObserved);
#endif
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
    const String& handlerName,
    Uint16 onFatalErrorPolicy = 2)
{
    CIMObjectPath filterPath;
    CIMObjectPath handlerPath;
    filterPath = _buildFilterOrHandlerPath(
        PEGASUS_CLASSNAME_INDFILTER, filterName, String::EMPTY,
        CIMNamespaceName());
    handlerPath = _buildFilterOrHandlerPath(
        PEGASUS_CLASSNAME_LSTNRDST_CIMXML, handlerName, String::EMPTY,
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

void _deleteSubscription(
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

Sint32 _getValue(
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
    Sint32 rc;
    returnValue.get(rc);

    return rc;
}

void _setup(CIMClient& client)
{
    _createFilterInstance(client, String("DRFilter01"),
        String("SELECT * FROM DeliveryRetryTestClass"), "WQL");

    char hName[256];

    for (Uint32 i = 1; i <= HANDLER_COUNT ; ++i)
    {
        sprintf(hName,"DRHandler%i",i);
        _createHandlerInstance(client, hName,
            String ("http://localhost:2003/CIMListener/DeliveryRetryListener"));
        _createSubscription(client, "DRFilter01", hName);
    }
}

void _cleanup(CIMClient& client)
{
    char hName[256];

    for (Uint32 i = 1; i <= HANDLER_COUNT ; ++i)
    {
        sprintf(hName,"DRHandler%i",i);
        _deleteSubscription(client, "DRFilter01", hName);
        _deleteHandlerInstance(client, String(hName));
    }

    _deleteFilterInstance(client, String("DRFilter01"));
}

static void _generateIndications(CIMClient &client, Uint32 count)
{
    Array<CIMParamValue> outParams;
    Array<CIMParamValue> inParams;
    inParams.append(
        CIMParamValue(String("count"),
            CIMValue(count)));

    client.invokeMethod(
        NAMESPACE,
        CIMObjectPath("DeliveryRetryTestClass"),
        "generateIndications",
        inParams,
        outParams);
}

static void _resetIdentfier(CIMClient &client)
{
    Array<CIMParamValue> outParams;
    Array<CIMParamValue> inParams;

    client.invokeMethod(
        NAMESPACE,
        CIMObjectPath("DeliveryRetryTestClass"),
        "resetIdentifier",
        inParams,
        outParams);
}


static void _testIndications(const Array<CIMInstance> &indications)
{
    Uint32 size = indications.size();

    Sint64 SequenceNumber;
    Sint64 MaxSequenceNumber = 0;
    Uint64 maxSequenceId = 0;
    Uint64 sequenceId;

    for (Uint32 i = 0 ; i < size; ++i)
    {
        indications[i].getProperty(
            indications[i].findProperty("SequenceNumber")).getValue().get(
                SequenceNumber);
        indications[i].getProperty(
            indications[i].findProperty("sequenceId")).getValue().get(
                sequenceId);

        if (SequenceNumber > MaxSequenceNumber)
        {
            MaxSequenceNumber = SequenceNumber;
        }

        if (sequenceId > maxSequenceId)
        {
            maxSequenceId = sequenceId;
        }
    }

#ifndef PEGASUS_TEST_VALGRIND_LOG_DIR
    PEGASUS_TEST_ASSERT(Uint64 (MaxSequenceNumber + 1) == maxSequenceId);
#endif

    NextSequenceNumber = MaxSequenceNumber + 1;
}

static void _testIndicationsWithListenerRunning(CIMClient &client)
{
    Stopwatch watch;
    watch.start();
    cout << "Testing indications with listener running..." << endl;
    CIMListener listener(2003);
    DeliveryRetryTestConsumer* consumer = new DeliveryRetryTestConsumer("1");
    listener.addConsumer(consumer);
    listener.start();
    _generateIndications(client, MAX_INDICATIONS);
    _waitForIndications(client, consumer, HANDLER_COUNT * MAX_INDICATIONS);
#ifdef PEGASUS_ENABLE_DMTF_INDICATION_PROFILE_SUPPORT
    _testIndications(consumer->getIndications());
#endif
    watch.stop();
    cout << "Completed in ";
    watch.printElapsed();

    delete consumer;
}

static Array<CIMInstance> _getDestinationQueues(CIMClient &client)
{
    return client.enumerateInstances(
        "root/PG_Internal",
        "PG_ListenerDestinationQueue",
        false,
        false,
        false,
        false,
        CIMPropertyList());

}

template <class T>
static Boolean _checkValue(
    CIMInstance &instance,
    const String &prop,
    T value)
{
    T lvalue;
    instance.getProperty(instance.findProperty(prop)).getValue().get(lvalue);
    return lvalue == value;
}

static void _waitForNextMaxSequenceNumber(CIMClient &client)
{
    Uint32 iteration = 0;
    Boolean expected = false;

    while (iteration < MAX_WAITTIME)
    {
        System::sleep(3);
        iteration += 3;
        Array<CIMInstance> queues = _getDestinationQueues(client);
        if (queues.size() == HANDLER_COUNT)
        {
            expected = true;
            break;
        }
    }

    Sint64 expVal = NextSequenceNumber + MAX_INDICATIONS;
    while (iteration < MAX_WAITTIME)
    {
        System::sleep(3);
        iteration += 3;
        Array<CIMInstance> queues = _getDestinationQueues(client);
        Boolean done = true;
        for (Uint32 i = 0, n = queues.size() ; i < n ; ++i)
        {
            if (!_checkValue(
                queues[i], "NextSequenceNumber", expVal))
            {
                done = false;
                break;
            }
        }
        if (done)
        {
            break;
        }
    }

#ifndef PEGASUS_TEST_VALGRIND_LOG_DIR
    PEGASUS_TEST_ASSERT(iteration < MAX_WAITTIME);
#endif
}

static void _testIndicationsWithListenerNotRunning(CIMClient &client)
{
    Stopwatch watch;
    watch.start();
    cout << "Testing indications with listener not running..." << endl;
    CIMListener listener(2003);
    DeliveryRetryTestConsumer* consumer = new DeliveryRetryTestConsumer("1");
    listener.addConsumer(consumer);
    _generateIndications(client, MAX_INDICATIONS);
    _waitForNextMaxSequenceNumber(client);
    listener.start();
    _waitForIndications(client, consumer, HANDLER_COUNT * MAX_INDICATIONS);
    _testIndications(consumer->getIndications());
    watch.stop();
    cout << "Completed in ";
    watch.printElapsed();
    listener.stop();
    delete consumer;
}

static void _waitForSequenceIdentifierLifetimeExpiration(
    CIMClient &client,
    const Uint64 expValue)
{
    Uint32 iteration = 0;

    while (iteration < MAX_WAITTIME)
    {
        System::sleep(3);
        iteration += 3;
        Array<CIMInstance> queues = _getDestinationQueues(client);
        Boolean done = true;
        for (Uint32 i = 0, n = queues.size() ; i < n ; ++i)
        {
            if (!_checkValue(
                queues[i], "RetryAttemptsExceededIndications", expValue))
            {
                done = false;
                break;
            }
        }
        if (done)
        {
            break;
        }
    }
#ifndef PEGASUS_TEST_VALGRIND_LOG_DIR
    PEGASUS_TEST_ASSERT(iteration < MAX_WAITTIME);
#endif
}

static void _testRetryAttemptsExceededIndications(CIMClient &client)
{
    Stopwatch watch;
    watch.start();
    cout << "Testing the retry attempts exceeded indications..." << endl;
    _generateIndications(client, MAX_INDICATIONS);
    System::sleep(3);
    _generateIndications(client, MAX_INDICATIONS);
    _waitForSequenceIdentifierLifetimeExpiration(client, 2 * MAX_INDICATIONS);
    watch.stop();
    cout << "Completed in ";
    watch.printElapsed();
}

static void _testDeliveryRetry(CIMClient &client)
{
    try
    {
        cout << "Testing DeliveryRetry functionality..." << endl;
        _setup(client);
        _resetIdentfier(client);
        _testIndicationsWithListenerRunning(client);
#ifdef PEGASUS_ENABLE_DMTF_INDICATION_PROFILE_SUPPORT
        _testIndicationsWithListenerNotRunning(client);
        // Run again the same test, this time SequenceNumber should change
        cout << "Test listener with next sequence ID..." << endl;
        _testIndicationsWithListenerNotRunning(client);
        _testRetryAttemptsExceededIndications(client);
#endif
        _cleanup(client);
#ifdef PEGASUS_ENABLE_DMTF_INDICATION_PROFILE_SUPPORT
        // There should not be any queues after cleanup
        Array<CIMInstance> queues = _getDestinationQueues(client);
        PEGASUS_TEST_ASSERT(queues.size() == 0);
#endif
    }
    catch (Exception& e)
    {
        cerr << "Error: " << e.getMessage() << endl;
        exit(1);
    }
}

// The code is based on test case 
// Pegasus/ControlProviders/InteropProvider/tests/interop.cpp

String _getCurrentConfigDeliveryRetryProperty(
    const CIMName &propertyName, CIMClient &client)
{
    // The following assumes localconnect.
    String _hostName;
    _hostName.assign(System::getHostName());

    CIMProperty prop;
    Array<CIMKeyBinding> kbArray;
    CIMKeyBinding kb;

    kb.setName(CIMName ("PropertyName"));
    kb.setValue(propertyName.getString());
    kb.setType(CIMKeyBinding::STRING);

    kbArray.append(kb);

    String propertyNameValue;
    String currentValue;
    try
    {
        CIMObjectPath reference(
            _hostName, PEGASUS_NAMESPACENAME_CONFIG,
            PEGASUS_CLASSNAME_CONFIGSETTING, kbArray);

        CIMInstance cimInstance =
            client.getInstance(PEGASUS_NAMESPACENAME_CONFIG, reference);

        Uint32 pos = cimInstance.findProperty(CIMName ("PropertyName"));
        prop = (CIMProperty)cimInstance.getProperty(pos);
        propertyNameValue = prop.getValue().toString();

        pos = cimInstance.findProperty(CIMName ("CurrentValue"));
        prop = (CIMProperty)cimInstance.getProperty(pos);
        currentValue = prop.getValue().toString();

        if (verbose)
        {
            cout << " Config return: "
                << " Requested Name: " <<  propertyName.getString()
                << " Returned Name: " <<  propertyNameValue
                << " current: " << currentValue << endl;
        }


    }
    catch(const CIMException& e)
    {
        cout << "CIM Exception during get Config " << e.getMessage() << endl;
        return(String::EMPTY);
    }

    return (currentValue);
}

 

static Uint16 _getCurrentConfigDeliveryRetryValue(String & propertyName)
{

    Uint64 deliveryRetryValue = 0;

    StringConversion::decimalStringToUint64(
         propertyName.getCString(), deliveryRetryValue);

    return (Uint16) deliveryRetryValue;

}

int main(int, char** argv)
{
    verbose = getenv("PEGASUS_TEST_VERBOSE") ? true : false;
    try
    {
        CIMClient client;
        client.setTimeout(60 * 1000);
        client.connectLocal();

        
        // The test case instruments reliable indication
        // maxIndicationDeliveryRetryAttempts value = 0  which disables the
        // reliable indication feature. A test for indication delivery 
        // when reliable indication framework is not working or  disabled
        // already exists in folder src/Providers/TestProviders
        //  /IndicationStressTestProvider

        String  currentValue = _getCurrentConfigDeliveryRetryProperty(
            "maxIndicationDeliveryRetryAttempts" ,client);
        if(_getCurrentConfigDeliveryRetryValue(currentValue))           
        {
            _testDeliveryRetry(client);
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
