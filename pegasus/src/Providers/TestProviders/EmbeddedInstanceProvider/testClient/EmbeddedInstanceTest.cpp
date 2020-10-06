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

#include <Pegasus/Common/Thread.h>
#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/Constants.h>
#include <Pegasus/Common/CIMName.h>
#include <Pegasus/Common/System.h>
#include <Pegasus/Common/FileSystem.h>
#include <Pegasus/Common/InternalException.h>
#include <Pegasus/Common/Array.h>
#include <Pegasus/Common/AutoPtr.h>

#include <Pegasus/General/Stopwatch.h>

#include <Pegasus/Client/CIMClient.h>
#include <Pegasus/Consumer/CIMIndicationConsumer.h>
#include <Pegasus/Listener/CIMListener.h>

#define MAX_COUNT 60

PEGASUS_USING_PEGASUS;
PEGASUS_USING_STD;

const CIMNamespaceName TEST_NAMESPACE =
    CIMNamespaceName("test/EmbeddedInstance/Dynamic");
const int listenerPort = 2003;
CIMObjectPath destinationPath;
CIMObjectPath filterPath;
CIMObjectPath subscriptionPath;

CIMObjectPath errorPath;
CIMObjectPath indicationPath;

AutoPtr<CIMInstance> errorInstance;
AutoPtr<CIMInstance> embeddedInstance;
AutoPtr<CIMInstance> indicationInstance;
Array<CIMName> errorPropList;
Array<CIMName> indicationPropList;

int receivedIndications;
AutoPtr<CIMInstance> receivedIndication;
Mutex* mut = 0;

////////////////////////////////////////////////////////////////////////////////
//
// IndicationConsumer Class
//
////////////////////////////////////////////////////////////////////////////////

class EmbeddedInstanceIndicationConsumer : public CIMIndicationConsumer
{
public:
    EmbeddedInstanceIndicationConsumer();
    ~EmbeddedInstanceIndicationConsumer();

    void consumeIndication(
        const OperationContext& context,
        const String& url,
        const CIMInstance& indicationInstance);
};

EmbeddedInstanceIndicationConsumer::EmbeddedInstanceIndicationConsumer()
{
}

EmbeddedInstanceIndicationConsumer::~EmbeddedInstanceIndicationConsumer()
{
}

void EmbeddedInstanceIndicationConsumer::consumeIndication(
    const OperationContext& context,
    const String& url,
    const CIMInstance& indicationInstance)
{
    AutoMutex a(*mut);
    if (receivedIndications == 0)
        receivedIndication.reset(new CIMInstance(indicationInstance));
    else if (!receivedIndication->identical(indicationInstance))
    {
        cout << "Received multiple different indications" << endl;
    }
    receivedIndications++;
}

int createErrorInstance(CIMClient& client)
{
    // Build the embedded instance
    Array<CIMKeyBinding> embeddedPathKeys;
    CIMName key1Name("key1");
    CIMValue key1Value(String("fake-key1"));
    CIMName key2Name("key2");
    CIMValue key2Value(String("fake-key2"));

    embeddedPathKeys.append(CIMKeyBinding(key1Name, key1Value));
    embeddedPathKeys.append(CIMKeyBinding(key2Name, key2Value));

    CIMObjectPath embeddedPath(
        "localhost",
        TEST_NAMESPACE,
        CIMName("PG_EmbeddedClass"),
        embeddedPathKeys);

    embeddedInstance.reset(new CIMInstance(CIMName("PG_EmbeddedClass")));
    //embeddedInstance->setPath(embeddedPath);
    embeddedInstance->addProperty(CIMProperty(key1Name,
        key1Value));
    embeddedInstance->addProperty(CIMProperty(key2Name,
        key2Value));
    embeddedInstance->addProperty(CIMProperty(CIMName("prop01"),
        Uint8(234)));
    embeddedInstance->addProperty(CIMProperty(CIMName("prop02"),
        Uint16(16234)));
    embeddedInstance->addProperty(CIMProperty(CIMName("prop03"),
        Uint32(32234)));
    embeddedInstance->addProperty(CIMProperty(CIMName("prop04"),
        Uint64(64234)));
    embeddedInstance->addProperty(CIMProperty(CIMName("prop05"),
        Sint8(-234)));
    embeddedInstance->addProperty(CIMProperty(CIMName("prop06"),
        Sint16(-16234)));
    embeddedInstance->addProperty(CIMProperty(CIMName("prop07"),
        Sint32(-32234)));
    embeddedInstance->addProperty(CIMProperty(CIMName("prop08"),
        Sint64(-64234)));
    embeddedInstance->addProperty(CIMProperty(CIMName("prop09"),
        Real32(-64234.46)));
    embeddedInstance->addProperty(CIMProperty(CIMName("prop10"),
        Real64(-23464234.78)));
    embeddedInstance->addProperty(CIMProperty(CIMName("prop11"),
        CIMDateTime(60, true)));

    // Build the embedded instance
    Array<CIMKeyBinding> errorPathKeys;
    CIMName errorKeyName("errorKey");
    CIMValue errorKeyValue(String("error key 1"));
    errorPathKeys.append(CIMKeyBinding(errorKeyName, errorKeyValue));
    CIMObjectPath localErrorPath(
        "localhost",
        TEST_NAMESPACE,
        CIMName("PG_EmbeddedError"),
        errorPathKeys);
    errorInstance.reset(new CIMInstance(CIMName("PG_EmbeddedError")));
    errorInstance->setPath(localErrorPath);
    errorInstance->addProperty(CIMProperty(errorKeyName,
        errorKeyValue));
    errorInstance->addProperty(CIMProperty(CIMName("EmbeddedInst"),
        CIMValue(*embeddedInstance)));
    try
    {
        errorPath = client.createInstance(TEST_NAMESPACE, *errorInstance);
        PEGASUS_STD(cout) << "Created EmbeddedError: " << errorPath.toString()
            << PEGASUS_STD(endl);
        errorInstance->setPath(errorPath);
    }
    catch (Exception& e)
    {
        PEGASUS_STD(cout) << "Exception while creating Error Instance: "
            << e.getMessage() << PEGASUS_STD(endl);
        return -1;
    }

    return 0;
}

int retrieveErrorInstance(CIMClient& client)
{
    try
    {
        PEGASUS_STD(cout) << "Getting error instance: " << errorPath.toString()
            << PEGASUS_STD(endl);
        CIMInstance ret = client.getInstance(
            TEST_NAMESPACE, errorPath, true, false, false, errorPropList);
        ret.setPath(errorPath);
        if (!errorInstance->identical(ret))
        {
            if (!ret.getPath().identical(errorInstance->getPath()))
            {
                PEGASUS_STD(cout) << "Object Paths not identical"
                    << PEGASUS_STD(endl);
            }
            PEGASUS_STD(cout) << "Error Instance and instance retrieved "
                << "through GetInstance operation not the same\n"
                << PEGASUS_STD(endl);
            PEGASUS_STD(cout) << "Local Error Instance: "
                << errorInstance->getPath().toString() << PEGASUS_STD(endl);
            for (unsigned int i = 0, n = errorInstance->getPropertyCount();
                 i < n; i++)
            {
                CIMProperty prop = errorInstance->getProperty(i);
                PEGASUS_STD(cout) << i << ". " << prop.getName().getString()
                    << prop.getValue().toString() << PEGASUS_STD(endl);
            }

            PEGASUS_STD(cout) << "Retrieved Error Instance: " <<
                ret.getPath().toString() << PEGASUS_STD(endl);
            for (unsigned int i = 0, n = ret.getPropertyCount();
                 i < n; i++)
            {
                CIMProperty prop = ret.getProperty(i);
                PEGASUS_STD(cout) << i << ". " << prop.getName().getString()
                    << prop.getValue().toString() << PEGASUS_STD(endl);
            }

            CIMProperty localEmbeddedProp = errorInstance->getProperty(
                errorInstance->findProperty("EmbeddedInst"));
            CIMProperty retEmbeddedProp = ret.getProperty(
                ret.findProperty("EmbeddedInst"));
            CIMInstance localEmbeddedInst;
            CIMInstance retEmbeddedInst;
            localEmbeddedProp.getValue().get(localEmbeddedInst);
            retEmbeddedProp.getValue().get(retEmbeddedInst);
            CIMObjectPath localEmbeddedPath = localEmbeddedInst.getPath();
            CIMObjectPath retEmbeddedPath = retEmbeddedInst.getPath();

            PEGASUS_STD(cout) << "Local Embedded Path: " <<
                localEmbeddedPath.toString() << PEGASUS_STD(endl);
            PEGASUS_STD(cout) << "Ret Embedded Path: " <<
                retEmbeddedPath.toString() << PEGASUS_STD(endl);
            return -1;
        }
    }
    catch (Exception& e)
    {
        cout << "Exception caught while getting error instance: "
            << e.getMessage() << endl;
        return -1;
    }

    try
    {
        Array<CIMInstance> ret = client.enumerateInstances(
            TEST_NAMESPACE,
            "PG_EmbeddedError",
            true,
            true,
            false,
            false,
            errorPropList);
        int count = ret.size();
        for (int i = 0; i < count; i++)
        {
            if (!errorInstance->identical(ret[i]))
            {
                printf("Error instance and instance retrieved through "
                    "EnumerateInstances operation not the same\n");
                return -1;
            }
        }
    }
    catch (Exception& e)
    {
        cout << "Exception caught while enumerating error instances: "
            << e.getMessage() << endl;
        return -1;
    }

    return 0;
}

int createSubscription(CIMClient& client)
{
    // Create a Listener Destination
    CIMInstance destinationInstance("CIM_ListenerDestinationCIMXML");
    destinationInstance.addProperty(CIMProperty(
        CIMName("SystemCreationClassName"),
        System::getSystemCreationClassName()));
//    destinationInstance.addProperty(CIMProperty(CIMName("SystemName"),
//        System::getFullyQualifiedHostName()));
//    destinationInstance.addProperty(CIMProperty(CIMName("CreationClassName"),
//        String("CIM_ListenerDestinationCIMXML")));
    destinationInstance.addProperty(CIMProperty(CIMName("Name"),
        String("EmbeddedInstance Test Handler")));
    destinationInstance.addProperty(CIMProperty(CIMName("Destination"),
      String("http://localhost:2003")));

    try
    {
        destinationPath =
            client.createInstance(TEST_NAMESPACE, destinationInstance);
    }
    catch (Exception& e)
    {
        cout << "Exception caught while creating listener destination: "
            << e.getMessage() << endl;
        return -1;
    }

    // Create a Filter
    CIMInstance filterInstance("CIM_IndicationFilter");
    filterInstance.addProperty(CIMProperty(
        CIMName("SystemCreationClassName"),
        System::getSystemCreationClassName()));
    filterInstance.addProperty(CIMProperty(CIMName("SystemName"),
        System::getFullyQualifiedHostName()));
    filterInstance.addProperty(CIMProperty(CIMName("CreationClassName"),
        String("CIM_IndicationFilter")));
    filterInstance.addProperty(CIMProperty(CIMName("Name"),
        String("Embedded Instance Test Filter")));
    filterInstance.addProperty(CIMProperty(CIMName("Query"),
        String("SELECT * FROM PG_InstMethodIndication")));
    filterInstance.addProperty(CIMProperty(CIMName("QueryLanguage"),
        String("CIM:CQL")));
    filterInstance.addProperty(CIMProperty(CIMName("SourceNamespace"),
        TEST_NAMESPACE.getString()));

    try
    {
        filterPath = client.createInstance (TEST_NAMESPACE, filterInstance);
    }
    catch (Exception& e)
    {
        cout << "Exception caught while creating indication filter: "
            << e.getMessage() << endl;
        return -1;
    }

    // Create a Subscription
    CIMInstance subscriptionInstance("CIM_IndicationSubscription");
    subscriptionInstance.addProperty(CIMProperty(CIMName("Filter"),
        filterPath, 0, "CIM_IndicationFilter"));
    subscriptionInstance.addProperty(CIMProperty(CIMName("Handler"),
        destinationPath, 0, "CIM_ListenerDestinationCIMXML"));
    subscriptionInstance.addProperty(CIMProperty(
        CIMName("SubscriptionState"), CIMValue((Uint16) 2)));

    try
    {
        subscriptionPath = client.createInstance(TEST_NAMESPACE,
            subscriptionInstance);
    }
    catch (Exception& e)
    {
        cout << "Exception caught while creating subscription: "
            << e.getMessage() << endl;
        return -1;
    }

    return 0;
}

int signalIndication(CIMClient& client)
{
    AutoMutex a(*mut);
    Array<CIMParamValue> inParameters;
    Array<CIMParamValue> outParameters;

    try
    {
        inParameters.append(CIMParamValue("error", CIMValue(*errorInstance)));
        client.invokeMethod(
            TEST_NAMESPACE,
            errorPath,
            CIMName("PropagateError"),
            inParameters,
            outParameters);

        if (outParameters.size() != 1)
        {
            cout << "Expected to get 1 output parameter from PropagateError. "
                << "Received " << outParameters.size() << endl;
            return -1;
        }

        CIMInstance outParamInstance;
        outParameters[0].getValue().get(outParamInstance);
        outParamInstance.setPath(errorInstance->getPath());
        if (!errorInstance->identical(outParamInstance))
        {
            cout << "Output Instance differs from Input Instance" << endl;
            return -1;
        }
    }
    catch (Exception& e)
    {
        cout << "Exception caught during invoke method of PropagateError: "
            << e.getMessage() << endl;
        return -1;
    }

    return 0;
}

int retrieveIndicationInstance(CIMClient& client)
{
    // Approximates a timeout, each loop is roughly 1 second
    Uint32 loopCount = 0;
    while ((receivedIndications == 0) && (loopCount < MAX_COUNT))
    {
        Thread::sleep(1000);
        loopCount++;
    }

    AutoMutex a(*mut);
    Array<CIMInstance> indications;
    try
    {
        indications = client.enumerateInstances(
            TEST_NAMESPACE, "PG_InstMethodIndication");
    }
    catch (Exception& e)
    {
        cout << "Exception caught while enumerating indications: "
            << e.getMessage() << endl;
        return -1;
    }

    if (indications.size() != 5)
    {
        cout << "Expected to get one instance. Received "
            << indications.size() << endl;
        return -1;
    }

    indicationInstance.reset(new CIMInstance(indications[0]));

    if (receivedIndication.get() == 0)
    {
        PEGASUS_STD(cout) << "Did not receive indication via listener" <<
            PEGASUS_STD(endl);
        return -1;
    }
#ifdef PEGASUS_ENABLE_DMTF_INDICATION_PROFILE_SUPPORT
    Uint32 idx;
    if ((idx=receivedIndication->findProperty("SequenceNumber"))
        != PEG_NOT_FOUND)
    {
        receivedIndication->removeProperty(idx);
    }
    if ((idx=receivedIndication->findProperty("SequenceContext"))
        != PEG_NOT_FOUND)
    {
        receivedIndication->removeProperty(idx);
    }
#endif
    if (!indicationInstance->identical(*receivedIndication))
    {
        PEGASUS_STD(cout)
            << "Indication instance retrieved via listener does not match "
            << "instance retrieved via enumeration" << PEGASUS_STD(endl);
        return -1;
    }

    return 0;
}

int removeErrorInstance(CIMClient& client)
{
    try
    {
        client.deleteInstance(TEST_NAMESPACE, errorPath);
    }
    catch (Exception & e)
    {
        PEGASUS_STD(cout)
            << "Exception encountered while removing Error Instance: "
            << e.getMessage() << PEGASUS_STD(endl);
    }
    return 0;
}

int removeSubscription(CIMClient& client)
{
    try
    {
        client.deleteInstance(TEST_NAMESPACE, subscriptionPath);
    }
    catch (Exception& e)
    {
        cout << "Exception caught while deleting subscription: "
            << e.getMessage() << endl;
    }

    try
    {
        client.deleteInstance(TEST_NAMESPACE, filterPath);
    }
    catch (Exception& e)
    {
        cout << "Exception caught while deleting indication filter: "
            << e.getMessage() << endl;
    }

    try
    {
        client.deleteInstance(TEST_NAMESPACE, destinationPath);
    }
    catch (Exception& e)
    {
        cout << "Exception caught while deleting listener destination: "
            << e.getMessage() << endl;
    }

    return 0;
}

int main()
{
    CIMClient client;
    CIMListener listener(listenerPort);
    receivedIndications = 0;
    mut = new Mutex();
    EmbeddedInstanceIndicationConsumer consumer1;
    listener.addConsumer(&consumer1);
    listener.start();

    try
    {
        client.connectLocal();
    }
    catch (...)
    {
        printf("Embedded Test Failed: Could not connect to server\n");
        return -1;
    }

    int ret = 0;

    ret = createSubscription(client);

    if (ret == 0)
        ret = createErrorInstance(client);

    errorPropList.append("errorKey");
    errorPropList.append("EmbeddedInst");
    if (ret == 0)
        ret = retrieveErrorInstance(client);

    if (ret == 0)
        ret = signalIndication(client);

    if (ret == 0)
        ret = retrieveIndicationInstance(client);

    if (ret == 0)
        ret = removeErrorInstance(client);

    if (ret == 0)
        ret = removeSubscription(client);
    else
    {
        removeErrorInstance(client);
        removeSubscription(client);
    }

    listener.removeConsumer(&consumer1);
    listener.stop();

    if (ret == 0)
        printf("+++EmbeddedInstanceTest passed+++\n");
    delete mut;
    return ret;
}
