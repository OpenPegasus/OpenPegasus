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
#include <Pegasus/Common/Array.h>
#include <Pegasus/Common/String.h>
#include <Pegasus/Common/CIMInstance.h>
#include <Pegasus/Common/CIMName.h>
#include <Pegasus/Common/CIMObjectPath.h>
#include <Pegasus/Common/CIMParamValue.h>
#include <Pegasus/Common/CIMProperty.h>
#include <Pegasus/Common/Exception.h>
#include <Pegasus/Client/CIMClient.h>

PEGASUS_USING_PEGASUS;
PEGASUS_USING_STD;

const CIMNamespaceName INTEROPNAMESPACE = CIMNamespaceName("root/PG_InterOp");
const CIMNamespaceName
    SOURCENAMESPACE = CIMNamespaceName ("SDKExamples/DefaultCXX");
static const CIMName
    FILTER_CLASSNAME = CIMName ("CIM_IndicationFilter");
static const CIMName
    HANDLER_CLASSNAME = CIMName ("CIM_ListenerDestinationCIMXML");
static const CIMName
    SUBSCRIPTION_CLASSNAME = CIMName ("CIM_IndicationSubscription");

enum indicationHandlerProtocol{
    PROTOCOL_CIMXML_INTERNAL = 1,
    PROTOCOL_CIMXML_HTTPS    = 2};

void _usage()
{
    cerr << "Usage: SendTestIndications" << endl
         << "[[ HTTPS ] |" << endl
         << " [subscribe | sendTestIndication | unsubscribe]]" << endl
         << "    where:" << endl
         << "       [ HTTPS ] is used to specify the protocol to" << endl
         << "            be used by the CIM-XML Indication Handler to" << endl
         << "            send indications to the Destination." << endl
         << "            If no value is specified, by default, the" << endl
         << "            internal protocol is used." << endl
         << "       [subscribe | sendTestIndication | unsubscribe] may" << endl
         << "            be used to execute an individual step of the" << endl
         << "            sample client." << endl
         << "            For example, after successfully running the" << endl
         << "            subscribe step, the sendTestIndication step" << endl
         << "            may be run as many times as necessary for" << endl
         << "            testing." << endl
         << "            As another example, if the test fails for" << endl
         << "            some reason in the sendTestIndication step," << endl
         << "            the unsubscribe step may be run to clean up" << endl
         << "            the repository." << endl
         << endl;
}

CIMObjectPath _createFilter
   (CIMClient & client,
    const String & name,
    const String & query,
    const String & queryLanguage,
    const String & sourceNamespace)
{
    CIMInstance filter (FILTER_CLASSNAME);

    filter.addProperty (CIMProperty (CIMName ("Name"), name));
    filter.addProperty (CIMProperty (CIMName ("Query"), query));
    filter.addProperty (CIMProperty (CIMName ("QueryLanguage"), queryLanguage));
    filter.addProperty (CIMProperty (CIMName ("SourceNamespace"),
        sourceNamespace));

    CIMObjectPath path = client.createInstance (INTEROPNAMESPACE, filter);
    return path;
}

CIMObjectPath _createHandler
   (CIMClient & client,
    const String & name,
    const String & destination)
{
    CIMInstance handler (HANDLER_CLASSNAME);

    handler.addProperty (CIMProperty (CIMName ("Name"), name));
    handler.addProperty (CIMProperty (CIMName ("Destination"), destination));

    CIMObjectPath path = client.createInstance (INTEROPNAMESPACE, handler);
    return path;
}

CIMObjectPath _createSubscription
   (CIMClient & client,
    const CIMObjectPath & filterPath,
    const CIMObjectPath & handlerPath)
{
    CIMInstance subscription (SUBSCRIPTION_CLASSNAME);

    subscription.addProperty (CIMProperty (CIMName ("Filter"),
        filterPath, 0, FILTER_CLASSNAME));
    subscription.addProperty (CIMProperty (CIMName ("Handler"),
        handlerPath, 0, HANDLER_CLASSNAME));

    CIMObjectPath path = client.createInstance (INTEROPNAMESPACE,
        subscription);
    return path;
}

CIMObjectPath _findFilterOrHandlerPath
   (CIMClient & client,
    const CIMName & className,
    const String & name)
{
    CIMObjectPath path;
    Array <CIMObjectPath> paths;
    paths = client.enumerateInstanceNames (INTEROPNAMESPACE,
        className);

    for (Uint32 i = 0; i < paths.size (); i++)
    {
        Array <CIMKeyBinding> keyBindings = paths [i].getKeyBindings ();
        for (Uint32 j = 0; j < keyBindings.size (); j++)
        {
            if (keyBindings [j].getName ().equal ("Name"))
            {
                if (keyBindings [j].getValue () == name)
                {
                    path = paths [i];
                    break;
                }
            }
        }
    }

    return path;
}

CIMObjectPath _buildSubscriptionPath
   (const CIMObjectPath & filterPath,
    const CIMObjectPath & handlerPath)
{
    CIMObjectPath path;
    Array <CIMKeyBinding> keyBindings;
    keyBindings.append (CIMKeyBinding ("Filter",
        filterPath.toString (), CIMKeyBinding::REFERENCE));
    keyBindings.append (CIMKeyBinding ("Handler",
        handlerPath.toString (), CIMKeyBinding::REFERENCE));

    path.setClassName (SUBSCRIPTION_CLASSNAME);
    path.setKeyBindings (keyBindings);

    return path;
}

//
//  Create subscription to receive indications of the RT_TestIndication
//  class and have them sent to the SimpleDisplayConsumer
//  The SimpleDisplayConsumer logs received indications to the
//  indicationLog file in PEGASUS_INDICATION_CONSUMER_LOG_DIR
//
void _subscribe(
    CIMClient& client,
    indicationHandlerProtocol handlerProtocol)
{
    //
    //  Create filter
    //
    CIMObjectPath filterPath = _createFilter
       (client,
        "TestFilter",
        "SELECT MethodName FROM RT_TestIndication",
        "WQL",
         SOURCENAMESPACE.getString ());

    //
    //  Create handler
    //
    String destinationPath;
    if (handlerProtocol == PROTOCOL_CIMXML_INTERNAL)
    {
        destinationPath = "localhost";
    }
    else if (handlerProtocol == PROTOCOL_CIMXML_HTTPS)
    {
        destinationPath = "https://localhost:5989";
    }
    else
    {
        destinationPath = "localhost";

    }
    destinationPath.append("/CIMListener/SDK_SimpleDisplayConsumer");
    CIMObjectPath handlerPath = _createHandler
       (client,
        "TestHandler",
        destinationPath);

    //
    //  Create subscription to receive indications of the RT_TestIndication
    //  class and have them sent to the SimpleDisplayConsumer
    //  The SimpleDisplayConsumer logs received indications to the
    //  indicationLog file in PEGASUS_INDICATION_CONSUMER_LOG_DIR
    //
    CIMObjectPath subscriptionPath = _createSubscription
       (client,
        filterPath,
        handlerPath);
}

//
//  Send test indication
//  The IndicationProvider SendTestIndication method is invoked
//  If the provider is enabled, it sends a test indication
//  Examine the indicationLog file in PEGASUS_INDICATION_CONSUMER_LOG_DIR
//  to verify successful indication receipt
//
Sint32 _sendTestIndication
   (CIMClient & client)
{
    //
    //  Invoke method to send test indication
    //
    Array <CIMParamValue> inParams;
    Array <CIMParamValue> outParams;
    Array <CIMKeyBinding> keyBindings;
    Sint32 result;

    CIMObjectPath className (String::EMPTY, CIMNamespaceName (),
        CIMName ("RT_TestIndication"), keyBindings);

    CIMValue retValue = client.invokeMethod
        (SOURCENAMESPACE,
        className,
        CIMName ("SendTestIndication"),
        inParams,
        outParams);
    retValue.get (result);

    return result;
}

//
//  Delete the subscription, handler, and filter instances
//
void _unsubscribe(CIMClient& client)
{
    CIMObjectPath filterPath =
        _findFilterOrHandlerPath (client, FILTER_CLASSNAME, "TestFilter");
    CIMObjectPath handlerPath =
        _findFilterOrHandlerPath (client, HANDLER_CLASSNAME, "TestHandler");
    CIMObjectPath subscriptionPath =
        _buildSubscriptionPath (filterPath, handlerPath);

    //
    //  Delete subscription
    //
    try
    {
        client.deleteInstance (INTEROPNAMESPACE, subscriptionPath);
    }
    catch (CIMException& e)
    {
        if (e.getCode() != CIM_ERR_NOT_FOUND)
        {
            throw(e);
        }
    }
    //
    //  Delete handler
    //
    try
    {
        client.deleteInstance (INTEROPNAMESPACE, handlerPath);
    }
    catch (CIMException& e)
    {
        if (e.getCode() != CIM_ERR_NOT_FOUND)
        {
            throw(e);
        }
    }
    //
    //  Delete filter
    //
    try
    {
        client.deleteInstance (INTEROPNAMESPACE, filterPath);
    }
    catch (CIMException& e)
    {
        if (e.getCode() != CIM_ERR_NOT_FOUND)
        {
            throw(e);
        }
    }
}

void _test(
    CIMClient& client,
    indicationHandlerProtocol handlerProtocol)
{
    _subscribe (client, handlerProtocol);

    Sint32 result = _sendTestIndication (client);

    String protocolString;
    if (handlerProtocol == PROTOCOL_CIMXML_INTERNAL)
    {
        protocolString = "internal protocol";
    }
    else if (handlerProtocol == PROTOCOL_CIMXML_HTTPS)
    {
        protocolString = "HTTPS";
    }

    if (result == 0)
    {
        cout << "Successfully sent test indication via " << protocolString <<
                endl;
    }
    else
    {
        cerr << "Failed to send test indication via " << protocolString <<
                endl;
    }

    _unsubscribe (client);
}

int main (int argc, char ** argv)
{
    try
    {
        CIMClient client;

        //
        //  Connect to CIM Server
        //
        try
        {
            client.connectLocal ();
        }

        catch(Exception& e)
        {
            cerr << "Exception thrown by client.connectLocal(): "
                << e.getMessage() << endl;
            return -1;
        }

        if (argc > 2)
        {
            _usage ();
            return 1;
        }

        else if (argc == 1)
        {
            try
            {
                _test(client, PROTOCOL_CIMXML_INTERNAL);
            }
            catch(Exception& e)
            {
                cerr << "SendTestIndications using internal protocol failed: "
                     << e.getMessage() << endl;
                return -1;
            }
        }

        else
        {
            const char * opt = argv [1];

            if (String::equalNoCase (opt, "HTTPS"))
            {
                try
                {
                    _test(client, PROTOCOL_CIMXML_HTTPS);
                }
                catch(Exception& e)
                {
                    cerr << "SendTestIndications using HTTPS protocol failed: "
                         << e.getMessage() << endl;
                    return -1;
                }
            }
            else if (String::equalNoCase (opt, "subscribe"))
            {
                try
                {
                    _subscribe (client, PROTOCOL_CIMXML_INTERNAL);
                }
                catch(Exception& e)
                {
                    cerr << "Exception thrown by _subscribe method: "
                         << e.getMessage() << endl;
                    return -1;
                }
            }
            else if (String::equalNoCase (opt, "sendTestIndication"))
            {
                Sint32 result = _sendTestIndication (client);

                if (result == 0)
                {
                    cout << "Successfully sent test indication" << endl;
                }
                else
                {
                    cerr << "Failed to send test indication" << endl;
                }
            }
            else if (String::equalNoCase (opt, "unsubscribe"))
            {
                try
                {
                    _unsubscribe (client);
                }
                catch(Exception& e)
                {
                    cerr << "Exception thrown by _unsubscribe method: "
                         << e.getMessage() << endl;
                    return -1;
                }
            }
            else
            {
                cerr << "Invalid option: " << opt << endl;
                _usage ();
                return -1;
            }
        }
    }
    catch (Exception & e)
    {
        cerr << "SendTestIndications failed: " << e.getMessage () << endl;
        return -1;
    }

    return 0;
}
