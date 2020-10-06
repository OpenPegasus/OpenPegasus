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
#include <Pegasus/Common/FileSystem.h>
#include <Pegasus/Common/Constants.h>
#include <Pegasus/Common/System.h>
#include <Pegasus/Common/PegasusAssert.h>
#include <Pegasus/HandlerService/HandlerTable.h>
#include <Pegasus/Client/CIMClient.h>

PEGASUS_USING_PEGASUS;
PEGASUS_USING_STD;


// Uses interop namespace defined by PEGASUS_NAMESPACENAME_INTEROP in
// Constants.h

static CIMObjectPath CreateHandlerInstance( 
    CIMClient &client, 
    const String &path, 
    const String &name)
{
    CIMInstance handlerInstance(PEGASUS_CLASSNAME_LSTNRDST_FILE);
    handlerInstance.addProperty (CIMProperty (CIMName
                ("SystemCreationClassName"), 
                System::getSystemCreationClassName()));
    handlerInstance.addProperty (CIMProperty (CIMName ("SystemName"),
                System::getFullyQualifiedHostName()));
    handlerInstance.addProperty (CIMProperty (CIMName ("CreationClassName"),
                PEGASUS_CLASSNAME_LSTNRDST_FILE.getString()));
    handlerInstance.addProperty(CIMProperty(CIMName ("Name"),
                name));
    handlerInstance.addProperty(CIMProperty(CIMName ("File"),path));

    CIMObjectPath Ref = client.createInstance(PEGASUS_NAMESPACENAME_INTEROP,
            handlerInstance);
    return Ref;
}

static void  testHandleIndication(
    CIMInstance hInstance, 
    CIMInstance subscription)
{
    const char* pegasusHome = getenv("PEGASUS_HOME");
    if (!pegasusHome)
    {
        cerr << "PEGASUS_HOME environment variable not set" << endl;
        exit(1);
    }

    ConfigManager::setPegasusHome(pegasusHome);

    CIMRepository repository("./repository");
    HandlerTable hTable;
    CIMHandler *hdlr = hTable.getHandler(
            String("FileListenerDestination"), &repository);
    PEGASUS_TEST_ASSERT(hdlr);

    OperationContext context;
    CIMObjectPath path;
    path.setNameSpace("test/TestProvider");
    path.setClassName("Test_IndicationProviderClass");
    CIMInstance indicationInstance(CIMName("Test_IndicationProviderClass"));
    indicationInstance.setPath(path);
    indicationInstance.addProperty(
            CIMProperty("IndicationIdentifier", String("101")));
    CIMDateTime currentDateTime = CIMDateTime::getCurrentDateTime();
    indicationInstance.addProperty(
            CIMProperty("IndicationTime", currentDateTime));

    ContentLanguageList contentLanguages;

    try
    {
        hdlr->handleIndication(context,
                PEGASUS_NAMESPACENAME_INTEROP.getString(),
                indicationInstance,
                hInstance,
                subscription,
                contentLanguages);
    }
    catch (CannotOpenFile &e)
    {
        throw e;
    }
    catch (const Exception &e)
    {
        throw e;
    }
}

static CIMObjectPath CreateFilterInstance(CIMClient& client,
        const String & query,
        const String & name)
{
    CIMInstance filterInstance(PEGASUS_CLASSNAME_INDFILTER);
    filterInstance.addProperty (CIMProperty (
        CIMName("SystemCreationClassName"), 
        System::getSystemCreationClassName()));
    filterInstance.addProperty (CIMProperty (CIMName ("SystemName"),
                System::getFullyQualifiedHostName()));
    filterInstance.addProperty(CIMProperty(CIMName ("CreationClassName"),
                PEGASUS_CLASSNAME_INDFILTER.getString()));
    filterInstance.addProperty(CIMProperty(CIMName ("Name"), name));
    filterInstance.addProperty (CIMProperty(CIMName ("Query"), query));
    filterInstance.addProperty (CIMProperty(CIMName ("QueryLanguage"),
                String("WQL")));
    filterInstance.addProperty (CIMProperty(CIMName ("SourceNamespace"),
                String("test/TestProvider")));

    CIMObjectPath Ref = client.createInstance(PEGASUS_NAMESPACENAME_INTEROP,
            filterInstance);
    return Ref;
}

static CIMObjectPath CreateFormattedSubscriptionIns(
        CIMClient& client,
        const CIMObjectPath & handlerRef,
        const CIMObjectPath & filterRef,
        const String & textFormat,
        const Array<String> & textFormatParams)
{
    CIMInstance subscriptionInstance
        (PEGASUS_CLASSNAME_FORMATTEDINDSUBSCRIPTION);
    subscriptionInstance.addProperty(CIMProperty(CIMName ("Filter"),
                filterRef, 0, PEGASUS_CLASSNAME_INDFILTER));
    subscriptionInstance.addProperty(CIMProperty(CIMName ("Handler"),
                handlerRef, 0, PEGASUS_CLASSNAME_LSTNRDST_FILE));
    subscriptionInstance.addProperty (CIMProperty
            (CIMName ("SubscriptionState"), CIMValue ((Uint16) 2)));
    subscriptionInstance.addProperty (CIMProperty
            (CIMName ("TextFormat"), textFormat));
    subscriptionInstance.addProperty (CIMProperty
            (CIMName ("TextFormatParameters"), textFormatParams));

    CIMObjectPath Ref = client.createInstance(
            PEGASUS_NAMESPACENAME_INTEROP, subscriptionInstance);
    return Ref;
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
                PEGASUS_CLASSNAME_INDFILTER.getString(), 
                CIMKeyBinding::STRING));
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
                PEGASUS_CLASSNAME_LSTNRDST_FILE.getString(),
                CIMKeyBinding::STRING));
    handlerKeyBindings.append(CIMKeyBinding("Name", handlerName,
                CIMKeyBinding::STRING));
    CIMObjectPath handlerPath("", CIMNamespaceName(),
            PEGASUS_CLASSNAME_LSTNRDST_FILE, handlerKeyBindings);

    Array<CIMKeyBinding> subscriptionKeyBindings;
    subscriptionKeyBindings.append(CIMKeyBinding("Filter",
                filterPath.toString(), CIMKeyBinding::REFERENCE));
    subscriptionKeyBindings.append(CIMKeyBinding("Handler",
                handlerPath.toString(), CIMKeyBinding::REFERENCE));

    return CIMObjectPath(
            "",
            CIMNamespaceName(),
            PEGASUS_CLASSNAME_FORMATTEDINDSUBSCRIPTION,
            subscriptionKeyBindings);
}

CIMObjectPath _getFilterPath(const String &filterName)
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
    return filterPath;

}

CIMObjectPath _getHandlerPath(const String &handlerName)
{
    Array<CIMKeyBinding> handlerKeyBindings;
    handlerKeyBindings.append(CIMKeyBinding("SystemCreationClassName",
                System::getSystemCreationClassName(), CIMKeyBinding::STRING));
    handlerKeyBindings.append(CIMKeyBinding("SystemName",
                System::getFullyQualifiedHostName(), CIMKeyBinding::STRING));
    handlerKeyBindings.append(CIMKeyBinding("CreationClassName",
                PEGASUS_CLASSNAME_LSTNRDST_FILE.getString(),
                CIMKeyBinding::STRING));
    handlerKeyBindings.append(CIMKeyBinding("Name", handlerName,
                CIMKeyBinding::STRING));
    CIMObjectPath handlerPath("", CIMNamespaceName(),
            PEGASUS_CLASSNAME_LSTNRDST_FILE, handlerKeyBindings);
    return handlerPath;
}

void testFileHandler(
    CIMClient &client, const String &hName, const String &fName)
{
    CIMObjectPath handlerPath = _getHandlerPath(hName);
    CIMObjectPath SubsObjPath = _getSubscriptionPath( fName,hName);

    // test File Listener handler
    testHandleIndication(
            client.getInstance(
                PEGASUS_NAMESPACENAME_INTEROP,handlerPath),
            client.getInstance(PEGASUS_NAMESPACENAME_INTEROP, SubsObjPath));
}


void DeleteInstance(CIMClient& client, const CIMObjectPath Ref)
{
    client.deleteInstance(PEGASUS_NAMESPACENAME_INTEROP, Ref);
}
/* 
 * Usage : 
 * setup
 * test
 * clean
 */


int main(int argc, char** argv)
{

    if( argc != 2 )
    {
        cerr << "Usage :" << argv[0] << " setup | test | clean \n"; 
        return -1;
    }

    CIMClient client;
    try
    {
        client.connectLocal();
    }
    catch (const Exception & e)
    {
        cerr << "Exception: " << e.getMessage() << endl;
        cerr << "Client connects local failed" << endl;
        return -1;
    }

    CIMObjectPath goodHandlerRef;
    if(!strcmp(argv[1], "setup"))
    {
        try
        {

            CIMObjectPath badHandlerRef;
#ifndef PEGASUS_OS_TYPE_WINDOWS
            badHandlerRef = CreateHandlerInstance( 
                client, "/tmp/", "badFileDestination");
            goodHandlerRef = CreateHandlerInstance( 
                client, "/tmp/indiLog", "goodFileDestination");
#else
            badHandlerRef = CreateHandlerInstance( 
                client, "C:\\tmp\\","badFileDestination") ;
            goodHandlerRef = CreateHandlerInstance( 
                client,"C:\\tmp\\indiLog","goodFileDestination");
#endif

            CIMObjectPath Filter1Ref ;
            String query="SELECT * FROM Test_IndicationProviderClass";
            String filterName1 = "TestFilter01";
            Filter1Ref = CreateFilterInstance (client, query, filterName1);

            Array<String> textFormatParams;
            textFormatParams.append("IndicationTime");
            textFormatParams.append("IndicationIdentifier");

            // create a formatted subscription
            CIMObjectPath Subscription1Ref ;
            Subscription1Ref = CreateFormattedSubscriptionIns(
                    client, badHandlerRef, Filter1Ref,
                    "The indication occured at {0, datetime} "
                    "with Indication ID {1}.",
                    textFormatParams);
            // create a formatted subscription
            CIMObjectPath Subscription2Ref ;
            Subscription2Ref = CreateFormattedSubscriptionIns (
                    client, goodHandlerRef, Filter1Ref,
                    "The indication occured at {0, datetime} "
                    "with Indication ID {1}.",
                    textFormatParams);
        }
        catch (const Exception& e)
        {
            cerr << "Exception: " << e.getMessage() << endl;
            return -1;
        }

        cout << "+++++ setup completed succesfully +++++ \n";
    }
    else if( !strcmp( argv[1], "test"))
    {
        bool gotException = false;
        try
        {
            // test bad File Listener handler
            testFileHandler( client,"badFileDestination", "TestFilter01");

        }
        catch(const CannotOpenFile &e)
        {
           cout << "++++Testing File can not be opened+++++\n"
                << e.getMessage() << ": Test passed\n";

           gotException = true;
        }
        catch(...)
        {
            cerr << "Unexpected exception caught: Test Failed \n"; 
            return -1;
        }
        PEGASUS_TEST_ASSERT(gotException);

        try
        {
            // test good File Listener handler
            testFileHandler( client,"goodFileDestination", "TestFilter01");

        }
        catch (const Exception& e)
        {
            cerr << "Exception: " << e.getMessage() << endl;
            return -1;
        }

        cout << "+++++ test completed succesfully +++++ \n";
    }
    else if(!strcmp( argv[1], "clean"))
    {
        try
        {
            DeleteInstance (
                    client,
                    _getSubscriptionPath("TestFilter01","badFileDestination"));
            DeleteInstance (
                    client,
                    _getSubscriptionPath("TestFilter01","goodFileDestination"));

            DeleteInstance (client, _getFilterPath("TestFilter01"));
            DeleteInstance (client, _getHandlerPath("badFileDestination"));
            DeleteInstance (client, _getHandlerPath("goodFileDestination"));
            FileSystem::removeDirectoryHier("./repository");

            client.disconnect();
        }
        catch (const Exception& e)
        {
            cerr << "Exception: " << e.getMessage() << endl;
            cerr << "delete instance failed" << endl;
            exit (-1);
        }

        cout << "+++++ cleanup successful+++++\n";
    }
    else
    {
        cerr << "Usage :" << argv[0] << " setup | test | clean \n"; 
        return -1;
    }
}
