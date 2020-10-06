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
#include <Pegasus/Client/CIMClient.h>

PEGASUS_USING_PEGASUS;
PEGASUS_USING_STD;

// Uses interop namespace defined by PEGASUS_NAMESPACENAME_INTEROP in
// Constants.h

CIMObjectPath CreateHandler1Instance(CIMClient& client)
{
    Array <String> mailTo;

    String mailAddr1;
    mailAddr1.append(System::getEffectiveUserName());
    mailAddr1.append("@");
    mailAddr1.append(System::getFullyQualifiedHostName());

    mailTo.append(mailAddr1);

    CIMInstance handlerInstance(PEGASUS_CLASSNAME_LSTNRDST_EMAIL);
    handlerInstance.addProperty (CIMProperty (CIMName
        ("SystemCreationClassName"), System::getSystemCreationClassName ()));
    handlerInstance.addProperty (CIMProperty (CIMName ("SystemName"),
        System::getFullyQualifiedHostName ()));
    handlerInstance.addProperty (CIMProperty (CIMName ("CreationClassName"),
        PEGASUS_CLASSNAME_LSTNRDST_EMAIL.getString ()));
    handlerInstance.addProperty(CIMProperty(CIMName ("Name"),
        String("Handler1")));
    handlerInstance.addProperty(CIMProperty(CIMName ("MailTo"), mailTo));
    handlerInstance.addProperty(CIMProperty(CIMName ("MailSubject"),
        String("Test Email Handler")));

    CIMObjectPath Ref = client.createInstance(PEGASUS_NAMESPACENAME_INTEROP,
        handlerInstance);
    return Ref;
}

CIMObjectPath CreateFilterInstance(CIMClient& client,
    const String & query,
    const String & name)
{
    CIMInstance filterInstance(PEGASUS_CLASSNAME_INDFILTER);
    filterInstance.addProperty (CIMProperty (CIMName
        ("SystemCreationClassName"), System::getSystemCreationClassName ()));
    filterInstance.addProperty (CIMProperty (CIMName ("SystemName"),
        System::getFullyQualifiedHostName ()));
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

CIMObjectPath CreateFormattedSubscriptionIns(CIMClient& client,
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
        handlerRef, 0, PEGASUS_CLASSNAME_LSTNRDST_EMAIL));
    subscriptionInstance.addProperty (CIMProperty
        (CIMName ("SubscriptionState"), CIMValue ((Uint16) 2)));
    subscriptionInstance.addProperty (CIMProperty
        (CIMName ("TextFormat"), textFormat));
    subscriptionInstance.addProperty (CIMProperty
        (CIMName ("TextFormatParameters"), textFormatParams));

    CIMObjectPath Ref = client.createInstance(PEGASUS_NAMESPACENAME_INTEROP,
        subscriptionInstance);
    return Ref;
}

void generateIndication(CIMClient& client)
{
    CIMObjectPath path ;
    path.setClassName("Test_IndicationProviderClass");

    Array<CIMParamValue> inParams;
    Array<CIMParamValue> outParams;

    CIMValue ret_value = client.invokeMethod(
        "test/TestProvider",
        path,
        "SendTestIndication",
        inParams,
        outParams);
}

void DeleteInstance(CIMClient& client, const CIMObjectPath &Ref)
{
    client.deleteInstance(PEGASUS_NAMESPACENAME_INTEROP, Ref);
}

int main()
{
    CIMClient client;
    try
    {
        client.connectLocal();
    }
    catch (Exception & e)
    {
        PEGASUS_STD (cerr) << "Exception: " << e.getMessage()
                           << PEGASUS_STD (endl);
        PEGASUS_STD (cerr) << "Client connect local failed"
                           << PEGASUS_STD (endl);
        return -1;
    }

    CIMObjectPath Handler1Ref;
    CIMObjectPath Filter1Ref;
    CIMObjectPath Subscription1Ref;

    try
    {
        Handler1Ref = CreateHandler1Instance (client);
    }
    catch (Exception& e)
    {
        PEGASUS_STD (cerr) << "Exception: " << e.getMessage()
                           << PEGASUS_STD (endl);
        PEGASUS_STD (cerr) << "create handler instance failed"
                           << PEGASUS_STD (endl);
        return -1;
    }

    PEGASUS_STD (cout) << "+++++ handler instance created"
                       << PEGASUS_STD (endl);
    try
    {
        String query="SELECT * FROM Test_IndicationProviderClass";
        String name1 = "TestFilter01";
        Filter1Ref = CreateFilterInstance (client, query, name1);
    }
    catch (Exception& e)
    {
        PEGASUS_STD (cerr) << "Exception: " << e.getMessage()
                           << PEGASUS_STD (endl);
        PEGASUS_STD (cerr) << "create filter instance failed"
                           << PEGASUS_STD (endl);
        return -1;
    }

    PEGASUS_STD (cout) << "+++++ filter instance created"
                       << PEGASUS_STD (endl);
    try
    {
        Array<String> textFormatParams;
        textFormatParams.append("IndicationTime");
        textFormatParams.append("IndicationIdentifier");

        // create a formatted subscription
        Subscription1Ref =
          CreateFormattedSubscriptionIns (client, Handler1Ref, Filter1Ref,
          "The indication occurred at {0, datetime} with Indication ID {1}.",
          textFormatParams);

    }
    catch (Exception& e)
    {
        PEGASUS_STD (cerr) << "Exception: " << e.getMessage()
                           << PEGASUS_STD (endl);
        PEGASUS_STD (cerr) << "create subscription instance failed"
                           << PEGASUS_STD (endl);
        return -1;
    }

    PEGASUS_STD (cout) << "+++++ subscription instance created"
                       << PEGASUS_STD (endl);

    try
    {
        // generate indication

        generateIndication(client);
    }
    catch (Exception& e)
    {
        PEGASUS_STD (cerr) << "Exception: " << e.getMessage()
                           << PEGASUS_STD (endl);
        PEGASUS_STD (cerr) << "generate indication failed"
                           << PEGASUS_STD (endl);
        return -1;
    }

    PEGASUS_STD (cout) << "+++++ indication generated"
                       << PEGASUS_STD (endl);

    try
    {
        DeleteInstance (client, Subscription1Ref);
        DeleteInstance (client, Filter1Ref);
        DeleteInstance (client, Handler1Ref);
    }
    catch (Exception& e)
    {
        PEGASUS_STD (cerr) << "Exception: " << e.getMessage()
                           << PEGASUS_STD (endl);
        PEGASUS_STD (cerr) << "delete instance failed"
                           << PEGASUS_STD (endl);
        exit (-1);
    }

    PEGASUS_STD (cout) << "+++++ instances deleted"
                       << PEGASUS_STD (endl);

    PEGASUS_STD (cout) << "+++++ TestEmailHandler passed all tests"
                       << PEGASUS_STD (endl);

}
