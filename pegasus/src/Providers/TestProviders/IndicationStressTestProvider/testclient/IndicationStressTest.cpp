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
#include <Pegasus/Common/Thread.h>
#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/Constants.h>
#include <Pegasus/Common/CIMName.h>
#include <Pegasus/General/OptionManager.h>
#include <Pegasus/Common/System.h>
#include <Pegasus/Common/FileSystem.h>
#include <Pegasus/Common/Exception.h>
#include <Pegasus/General/Stopwatch.h>
#include <Pegasus/Common/Array.h>
#include <Pegasus/Common/AutoPtr.h>

#include <Pegasus/Client/CIMClient.h>
#include <Pegasus/Consumer/CIMIndicationConsumer.h>
#include <Pegasus/Listener/CIMListener.h>


PEGASUS_USING_PEGASUS;
PEGASUS_USING_STD;

Array<String> sourceNamespaces;
String indicationClassName;
Boolean Ipv6Test;

const CIMNamespaceName DEFAULT_NAMESPACE =
    CIMNamespaceName ("test/TestProvider");
const String DEFAULT_CLASS_NAME = String ("IndicationStressTestClass");
const String HTTP_IPV6_DESTINATION = String ("http://[::1]:5988");
const String HTTPS_IPV6_DESTINATION = String ("https://[::1]:5989");
const String HTTP_IPV4_DESTINATION = String ("http://localhost:5988");
const String HTTPS_IPV4_DESTINATION = String ("https://localhost:5989");

const CIMNamespaceName INDICATION_CONSUMER_NAMESPACE =
          CIMNamespaceName ("test/TestProvider");
const String INDICATION_CONSUMER_CLASS_NAME = "PG_IndicationStressTestConsumer";
const String INDICATION_CLASS_NAME = String ("IndicationStressTestClass");

const String SERVER_RESIDENT_HANDLER_NAME = String ("IPHandler01");
const String CLIENT_RESIDENT_HANDLER_NAME = String ("IPHandler02");
const String FILTER_NAME = String ("IPFilter01");
const String INDICATION_COUNT_PROPERTY = String ("indicationsReceived");
const String INDICATION_COUNT_FROM_EXPECTED_SENDER_PROPERTY =
     String ("indicationsReceivedFromExpectedIdentity");


AtomicInt receivedIndicationCount(0);

#define MAX_UNIQUE_IDS 10000
Uint64 seqNumPrevious[MAX_UNIQUE_IDS];
AtomicInt seqNumberErrors(0);
Uint32 seqNumberErrorsDisplay = 0;
Uint32 indicationSendCountTotal = 0;
Uint64 sendRecvDeltaTimeTotal = 0;
int sendRecvDeltaTimeCnt = 0;
int sendRecvDeltaTimeMax = 0;
int sendRecvDeltaTimeMin = 0x7fffffff;

AtomicInt errorsEncountered(0);

enum indicationHandleProtocol {
                 PROTOCOL_CIMXML_INTERNAL = 1,
                 PROTOCOL_CIMXML_HTTP     = 2,
                 PROTOCOL_CIMXML_HTTPS    = 3};

////////////////////////////////////////////////////////////////////////////////
//
// Thread Parameters Class
//
////////////////////////////////////////////////////////////////////////////////

class T_Parms{
   public:
    CIMClient* client;
    Uint32 indicationSendCount;
    Uint32 uniqueID;
};

////////////////////////////////////////////////////////////////////////////////
//
// IndicationConsumer Class
//
////////////////////////////////////////////////////////////////////////////////

class MyIndicationConsumer : public CIMIndicationConsumer
{
public:
    MyIndicationConsumer(String name);
    ~MyIndicationConsumer();

    void consumeIndication(const OperationContext& context,
        const String & url,
        const CIMInstance& indicationInstance);

private:
    String name;

};

MyIndicationConsumer::MyIndicationConsumer(String name_)
{
    this->name = name_;
    for (Uint32 i=0; i < MAX_UNIQUE_IDS; i++)
      seqNumPrevious[i] = 1;

//  cout << "Constructing MyIndicationConsumer" << endl;
}

MyIndicationConsumer::~MyIndicationConsumer()
{
//  cout << "Destructing MyIndicationConsumer" << endl;
}

Boolean maxUniqueIDMsgIssued = false;

void MyIndicationConsumer::consumeIndication(
                         const OperationContext & context,
                         const String & url,
                         const CIMInstance& indicationInstance)
{
  //
  // Increment the count of indications received
  //
  receivedIndicationCount++;
  PEGASUS_TEST_ASSERT(
      indicationInstance.getClassName().getString() == indicationClassName);
  if (receivedIndicationCount.get() % 200 == 0)
    cout << "+++++     received indications = "
         << receivedIndicationCount.get()
         << " of " << indicationSendCountTotal << endl;

  // cout << "IndicationStressTest consumer - recvd indication = "
  //     << ((CIMObject)indicationInstance).toString() << endl;

  //
  // Get the date and time from the indication
  // Compare it to the current date
  // calculate the time it took to be delivered.
  // add it to the total delivery time to calculate the average
  //      indication delivery time for the test.
  // Update the min and max delta times.
  //

  //
  // Calculate the time diference between when sent and received (now)
  //

  Uint32 indicationTimeIndex =
      indicationInstance.findProperty("IndicationTime");


  if (indicationTimeIndex == PEG_NOT_FOUND)
    {
      cout << "+++++ ERROR: Indication Stress Test Consumer - indicationTime"
           << " NOT FOUND" << endl;
      errorsEncountered++;
      return;
    }


  CIMConstProperty indicationTime_property =
      indicationInstance.getProperty(indicationTimeIndex);
  // cout << "indicationTime = "
  //      << indicationTime_property.getValue().toString() << endl;

  CIMDateTime indicationTime;
  indicationTime_property.getValue().get(indicationTime);

  CIMDateTime currentDateTime = CIMDateTime::getCurrentDateTime ();
  Sint64 sendRecvDeltaTime =
      CIMDateTime::getDifference(indicationTime, currentDateTime);

  // cout << "sendRecvDeltaTime = "
 //       << (long)(sendRecvDeltaTime/1000) << " milli-seconds" << endl;

  sendRecvDeltaTimeTotal += sendRecvDeltaTime;
  sendRecvDeltaTimeCnt++;

  if (sendRecvDeltaTime > sendRecvDeltaTimeMax)
    sendRecvDeltaTimeMax = sendRecvDeltaTime;

  if (sendRecvDeltaTime < sendRecvDeltaTimeMin)
    sendRecvDeltaTimeMin = sendRecvDeltaTime;

  //
  // Get the unique ID
  //
  // This is sort of a Thread ID except that the unique ID keeps incrementing
  // across tests runs as long as the server continues to run)
  //

  Uint32 uniqueIDIndex =
      indicationInstance.findProperty("IndicationIdentifier");


  if (uniqueIDIndex == PEG_NOT_FOUND)
    {
      cout << "+++++ ERROR: Indication Stress Test Consumer - indication"
           << " Unique id NOT FOUND" << endl;
      errorsEncountered++;
      return;
    }


  CIMConstProperty uniqueID_property =
      indicationInstance.getProperty(uniqueIDIndex);
  // cout << "uniqueID = " << uniqueID_property.getValue().toString() << endl;

  String uniqueID_string;
  Uint32 uniqueID = 0;
  uniqueID_property.getValue().get(uniqueID_string);
  uniqueID = atoi (uniqueID_string.getCString());

  // cout << "uniqueID = " << uniqueID << endl;

  //
  // Get the seq number
  //

  Uint32 seqNumIndex =
      indicationInstance.findProperty ("IndicationSequenceNumber");

  if (seqNumIndex == PEG_NOT_FOUND)
    {
      cout << "+++++ ERROR: Indication Stress Test Consumer - indication"
           << " seq number NOT FOUND" << endl;
      errorsEncountered++;
    }
  else if ((long)uniqueID >= MAX_UNIQUE_IDS)
    {
      if (!maxUniqueIDMsgIssued)
        {
          maxUniqueIDMsgIssued = true;
          cout << endl;
          cout << "+++++ ERROR: Indication Stress TestConsumer - recvd"
               << " uniqueID( " << (long)uniqueID
               << " ) >= MAX_UNIQUE_IDS ( " << MAX_UNIQUE_IDS << " )"
               << endl;
          cout << "+++++        To correct: Stop and start the server, this"
               << " resets the uniqueID generated by the provider."
               << endl;
          cout << "+++++        Sequence number checking is not completly"
               << " enabled without this" << endl << endl;
          errorsEncountered++;
        }

    }
  else
    {
      CIMConstProperty seqNum_property =
          indicationInstance.getProperty(seqNumIndex);
      // cout << "seqNum = " << (seqNum_property.getValue()).toString()
      //        << endl;

      Uint64 seqNumRecvd;
      seqNum_property.getValue().get(seqNumRecvd);
      // cout << "seqNumRecvd = " << (long)seqNumRecvd << endl;

      //
      // See if seqNumRecvd less than previous received matches seqNumPrevious
      //
      // The method used to determine the out of order count is
      // (received < previous) received.
      //
      // The other choice would would be (received != expected) where expected
      // is the previous received +1.
      //
      // The (actual < previous) was chosen as giving results that
      // are more representative of the ordering problems.
      //
      // Consider these indication sequences:
      //
      //     A: 1,3,4,2,5
      //     B: 1,4,3,2,5
      //
      // The "out of sequence" counts for "(actual != expected)" are A=3, B=2,
      // while "(actual < previous)" gives A=1, B=2.
      //
      // Thanks to Roger Kump at HP for suggesting the
      // actual <  previous method.
      //
      // JR Wunderlich 7/14/2005
      //

      if (seqNumRecvd < seqNumPrevious[uniqueID])
        {
          seqNumberErrors++;
          if (seqNumberErrorsDisplay)
            {
              cout << "+++++ ERROR: Indication Stress Test Consumer"
                   << "- Sequence error "
                   << " previous = "
                   << (unsigned long) seqNumPrevious[uniqueID]
                   << " received = " << (unsigned long) seqNumRecvd << endl;
            }
        }
      seqNumPrevious[uniqueID] = seqNumRecvd;
    }


  return;

}

///////////////////////////////////////////////////////////////////////////

CIMObjectPath _getFilterObjectPath
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
    return(CIMObjectPath("", CIMNamespaceName (),
        PEGASUS_CLASSNAME_INDFILTER, keyBindings));
}

CIMObjectPath _getHandlerObjectPath
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
    return(CIMObjectPath("", CIMNamespaceName (),
        PEGASUS_CLASSNAME_INDHANDLER_CIMXML, keyBindings));
}

CIMObjectPath _getSubscriptionObjectPath
    (CIMClient & client,
     const String & filterName,
     const String & handlerName)
{
    CIMObjectPath filterObjectPath =
        _getFilterObjectPath(client, filterName);

    CIMObjectPath handlerObjectPath =
        _getHandlerObjectPath(client, handlerName);

    Array<CIMKeyBinding> subscriptionKeyBindings;
    subscriptionKeyBindings.append (CIMKeyBinding ("Filter",
        CIMValue(filterObjectPath)));
    subscriptionKeyBindings.append (CIMKeyBinding ("Handler",
        CIMValue(handlerObjectPath)));
    return(CIMObjectPath("", CIMNamespaceName (),
        PEGASUS_CLASSNAME_INDSUBSCRIPTION, subscriptionKeyBindings));
}

Boolean _subscriptionExists
    (CIMClient & client,
     const String & filterName,
     const String & handlerName)
{
    try
    {
        CIMObjectPath subscriptionObjectPath =
             _getSubscriptionObjectPath(client, filterName, handlerName);
        client.getInstance(
            PEGASUS_NAMESPACENAME_INTEROP,
            subscriptionObjectPath);
    }
    catch (CIMException& e)
    {
        if (e.getCode() == CIM_ERR_NOT_FOUND)
        {
           return(false);
        }
        else
        {
            cerr << "----- Error: subscriptionExists failed: " << endl;
            throw(e);
        }
    }
    return(true);
}

CIMObjectPath _createHandlerInstance
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

    return(client.createInstance(
        PEGASUS_NAMESPACENAME_INTEROP,
        handlerInstance));
}

CIMObjectPath _createFilterInstance
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
    filterInstance.addProperty (CIMProperty (CIMName ("SourceNamespaces"),
        sourceNamespaces));

    return(client.createInstance(
        PEGASUS_NAMESPACENAME_INTEROP,
        filterInstance));
}

CIMObjectPath _createSubscriptionInstance
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

    return(client.createInstance(
        PEGASUS_NAMESPACENAME_INTEROP,
        subscriptionInstance));
}

// Returns the Number of Subscriptions from a Provider
static Uint32  _getCount(CIMClient&  client)
{
    Array <CIMParamValue> inParams;
    Array <CIMParamValue> outParams;
    Uint32 result;

    CIMObjectPath  objPath;

    objPath.setNameSpace("test/TestProvider");
    objPath.setClassName("TestCMPI_IndicationStressTestClass");

    CIMValue retValue = client.invokeMethod(
        CIMNamespaceName("test/TestProvider"),
        objPath,
        "getSubscriptionCount",
        inParams,
        outParams);

    retValue.get(result);
    PEGASUS_TEST_ASSERT (result == 12);

    return result;
}

void _sendTestIndication(
    CIMClient* client,
    const CIMName & methodName,
    Uint32 indicationSendCount)
{
    //
    //  Invoke method to send test indication
    //
    Array <CIMParamValue> inParams;
    Array <CIMParamValue> outParams;
    Array <CIMKeyBinding> keyBindings;
    Sint32 result;

    CIMValue sendCountValue(indicationSendCount);
    inParams.append(
        CIMParamValue(String("indicationSendCount"),
        CIMValue(indicationSendCount)));

    CIMObjectPath className (String::EMPTY, CIMNamespaceName (),
        CIMName (indicationClassName), keyBindings);

    CIMValue retValue = client->invokeMethod(
        "test/TestProvider",
        className,
        methodName,
        inParams,
        outParams);
    retValue.get (result);
    PEGASUS_TEST_ASSERT (result == 0);

    //
    //  Allow time for the indication to be received and forwarded
    //
    System::sleep (5);
}


void _sendTestIndicationNormal(CIMClient* client, Uint32 indicationSendCount)
{
    _sendTestIndication (client, CIMName ("SendTestIndicationNormal"),
        indicationSendCount);
}

void _deleteSubscriptionInstance
    (CIMClient & client,
     const String & filterName,
     const String & handlerName)
{
    CIMObjectPath subscriptionObjectPath =
       _getSubscriptionObjectPath(client, filterName, handlerName);
    client.deleteInstance(
        PEGASUS_NAMESPACENAME_INTEROP,
        subscriptionObjectPath);
}

void _deleteHandlerInstance
    (CIMClient & client,
     const String & name)
{
    CIMObjectPath handlerObjectPath = _getHandlerObjectPath(client, name);
    client.deleteInstance (PEGASUS_NAMESPACENAME_INTEROP, handlerObjectPath);
}

void _deleteFilterInstance
    (CIMClient & client,
     const String & name)
{
    CIMObjectPath filterObjectPath = _getFilterObjectPath(client, name);
    client.deleteInstance (PEGASUS_NAMESPACENAME_INTEROP, filterObjectPath);
}

void _usage ()
{
   cerr << endl
        << "Usage:" << endl
        << "    TestIndicationStressTest ClassName Namespace\n"
        << "                  [setup | setupCL | setupSL]\n"
        << "                  [ WQL | DMTF:CQL ]\n"
        << "                  [INTERNAL | HTTP | HTTPS]\n"
        << "    where: " << endl
        << "       setup configures both the Client-resident Listener and\n"
        << "            the Server-resident Lister.\n"
        << "       setupCL configures only the Client-resident Listener.\n"
        << "       setupSL configures only the Server-resident Listener.\n"
        << "       [INTERNAL | HTTP | HTTPS] is used to select the protocol\n"
        << "            used, by the CIM-XML Indication Handler, to send\n"
        << "            Indications to the Server-resident Listener.\n"
        << "            INTERNAL is the default value."
        << endl << endl
        << "    TestIndicationStressTest ClassName Namespace"
        << " run <indicationSendCount> [<threads>]\n"
        << "    where: " << endl
        << "       <indicationSendCount> is the number of indications to\n"
        << "            generate, and can be zero to measure the overhead in\n"
        << "            calling the provider." << endl
        << "       <threads> is an optional number of client threads to\n"
        << "            create, default is one." << endl
        << "       <SenderIdentity> is the system user name associated with\n"
        << "            the certificate of the server sending the Indication.\n"
        << "            The cimtrust utility can be used to associate a\n"
        << "            user name with a certificate.  This feature is only\n"
        << "            supported for Server-resident Listeners.\n"
        << "               cimtrust -a -U guest -f server.pem\n"
        << "            Note: the <threads> parameter must be specified if\n"
        << "            the <SenderIdentity> value is defined.\n"
        << endl << endl
        << "    TestIndicationStressTest ClassName Namespace "
        << "getSubscriptionCount\n "
        << "       getSubscriptionCount returns the number of\n"
        << "           active Subscriptions from Provider.\n"
        << endl << endl
        << "    TestIndicationStressTest ClassName Namespace cleanup"
        << endl << endl;
}

void _getTestResults(
    CIMClient &client,
    Uint32 &indicationCount,
    Uint32 &indicationCountFromExpectedIdentity)
 {
    const CIMObjectPath classPath = CIMObjectPath
        (INDICATION_CONSUMER_CLASS_NAME);
    Array<CIMParamValue> inParams;
    Array<CIMParamValue> outParams;

    CIMValue retValue = client.invokeMethod(
        INDICATION_CONSUMER_NAMESPACE.getString (),
        classPath,
        CIMName("getTestResults"),
        inParams,
        outParams);

    Uint32 status;
    retValue.get(status);
    if (status != 0)
    {
        throw Exception ("Failure status returned from getTestResults");
    }

    if (outParams.size() != 2)
    {
        throw Exception (
            "Invalid number of parameters returned from getTestResults");
    }
    else
    {
        String paramName0 = outParams[0].getParameterName();
        String paramName1 = outParams[1].getParameterName();
        if (paramName0 == INDICATION_COUNT_PROPERTY)
        {
            outParams[0].getValue().get(indicationCount);
            if (paramName1 == INDICATION_COUNT_FROM_EXPECTED_SENDER_PROPERTY)
            {
                outParams[1].getValue().get(
                    indicationCountFromExpectedIdentity);
            }
            else
            {
                throw Exception("Invalid getTestResults parameter");
            }
        }
        else if (paramName0 == INDICATION_COUNT_FROM_EXPECTED_SENDER_PROPERTY)
        {
            outParams[0].getValue().get(indicationCountFromExpectedIdentity);
            if (paramName1 == INDICATION_COUNT_PROPERTY)
            {
                outParams[1].getValue().get(indicationCount);
            }
            else
            {
                throw Exception("Invalid getTestResults parameter");
            }
        }
        else
        {
            throw Exception ("Invalid getTestResults parameter");
        }
    }
}


void _setupServerResidentListener(CIMClient &client,
    String expectedSenderIdentity)
{
    //
    //  Remove previous indication log file, if there
    //
    String previousIndicationFile, oldIndicationFile;

    previousIndicationFile = INDICATION_DIR;
    previousIndicationFile.append ("/IndicationStressTestLog");

    if (FileSystem::exists (previousIndicationFile))
    {
        oldIndicationFile = INDICATION_DIR;
        oldIndicationFile.append ("/oldIndicationStressTestLog");
        if (FileSystem::exists (oldIndicationFile))
        {
            FileSystem::removeFile (oldIndicationFile);
        }
        if (!FileSystem::renameFile (previousIndicationFile, oldIndicationFile))
        {
            FileSystem::removeFile (previousIndicationFile);
        }
    }

    const CIMObjectPath classPath = CIMObjectPath
        (INDICATION_CONSUMER_CLASS_NAME);
    Array<CIMParamValue> inParams;
    Array<CIMParamValue> outParams;

    inParams.append( CIMParamValue("indicationsReceivedFromExpectedIdentity",
         CIMValue(expectedSenderIdentity)));

    CIMValue retValue = client.invokeMethod(
        INDICATION_CONSUMER_NAMESPACE.getString (),
        classPath,
        CIMName("setupTestConfiguration"),
        inParams,
        outParams);

    Uint32 status;
    retValue.get(status);
    if (status != 0)
    {
        throw Exception ("Failure status returned from getTestResults");
    }
}

void _setup (CIMClient & client, String& qlang,
    indicationHandleProtocol handleProtocol,
    Boolean configureServerResidentListener,
    Boolean configureClientResidentListener)
{
    CIMObjectPath filterObjectPath;
    CIMObjectPath serverHandlerObjectPath;
    CIMObjectPath clientHandlerObjectPath;

    Boolean instanceAlreadyExists;

    try
    {
        instanceAlreadyExists = false;
        String query ("SELECT * FROM ");
        query.append (indicationClassName);
        filterObjectPath = _createFilterInstance (client, FILTER_NAME,
                                                  query, qlang);
    }
    catch (CIMException& e)
    {
        if (e.getCode() == CIM_ERR_ALREADY_EXISTS)
        {
            instanceAlreadyExists = true;
            cerr << "----- Warning: Filter Instance Not Created: "
                << e.getMessage () << endl;
        }
        else
        {
            cerr << "----- Error: Filter Instance Not Created: " << endl;
            throw(e);
        }
    }
    if (instanceAlreadyExists)
    {
        filterObjectPath = _getFilterObjectPath(client, FILTER_NAME);
        instanceAlreadyExists = false;
    }

    if (configureServerResidentListener)
    {
        try
        {
            // Create the handler for the Server-resident Listener
            String destinationProtocol;
            if (handleProtocol == PROTOCOL_CIMXML_INTERNAL)
            {
               destinationProtocol = "localhost";
            }
            else if (handleProtocol == PROTOCOL_CIMXML_HTTP)
            {
                 if (Ipv6Test)
                 {
                     destinationProtocol = HTTP_IPV6_DESTINATION;
                 }
                 else
                 {
                     destinationProtocol = HTTP_IPV4_DESTINATION;
                 }
            }
            else if (handleProtocol == PROTOCOL_CIMXML_HTTPS)
            {
                 if (Ipv6Test)
                 {
                     destinationProtocol = HTTPS_IPV6_DESTINATION;
                 }
                 else
                 {
                     destinationProtocol = HTTPS_IPV4_DESTINATION;
                 }
            }
            else
            {
               PEGASUS_TEST_ASSERT(0);
            }
            serverHandlerObjectPath = _createHandlerInstance (client,
                SERVER_RESIDENT_HANDLER_NAME, destinationProtocol +
                String ("/CIMListener/Pegasus_IndicationStressTestConsumer"));
        }
        catch (CIMException& e)
        {
            if (e.getCode() == CIM_ERR_ALREADY_EXISTS)
            {
                instanceAlreadyExists = true;
                cerr << "----- Warning: Server Handler Instance Not Created: "
                    << e.getMessage () << endl;
            }
            else
            {
                cerr << "----- Error: Server Handler Instance Not Created: "
                    << endl;
                throw(e);
            }
        }
        if (instanceAlreadyExists)
        {
            serverHandlerObjectPath = _getHandlerObjectPath(client,
                SERVER_RESIDENT_HANDLER_NAME);
            instanceAlreadyExists = false;
        }

        try
        {
            _createSubscriptionInstance (client, filterObjectPath,
                 serverHandlerObjectPath);
        }
        catch (CIMException& e)
        {
            if (e.getCode() == CIM_ERR_ALREADY_EXISTS)
            {
                cerr << "----- Warning: Server Subscription Instance: "
                    << e.getMessage () << endl;
            }
            else
            {
                cerr << "----- Error: Server Subscription Instance: " << endl;
                throw(e);
            }
        }
    }

    if (configureClientResidentListener)
    {
        try
        {
            // Create the handler with this program as the CIMListener
            clientHandlerObjectPath = _createHandlerInstance (client,
                CLIENT_RESIDENT_HANDLER_NAME,
                Ipv6Test ?
                String ("http://[::1]:2005/TestIndicationStressTest") :
                String ("http://localhost:2005/TestIndicationStressTest"));
        }
        catch (CIMException& e)
        {
            if (e.getCode() == CIM_ERR_ALREADY_EXISTS)
            {
                instanceAlreadyExists = true;
                cerr << "----- Warning: Client Handler Instance Not Created: "
                    << e.getMessage () << endl;
            }
            else
            {
                cerr << "----- Error: Client Handler Instance Not Created: "
                    << endl;
                throw(e);
            }
        }
        if (instanceAlreadyExists)
        {
            clientHandlerObjectPath = _getHandlerObjectPath(client,
                CLIENT_RESIDENT_HANDLER_NAME);
            instanceAlreadyExists = false;
        }
        try
        {
            _createSubscriptionInstance (client, filterObjectPath,
                 clientHandlerObjectPath);
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
                throw(e);
            }
        }
    }
}

void _sendNormal(CIMClient* client, Uint32 indicationSendCount)
{
    try
    {
        _sendTestIndicationNormal(client, indicationSendCount);
    }
    catch (Exception & e)
    {
        cerr << "----- sendNormal failed: " << e.getMessage () << endl;
        exit (-1);
    }

//  cout << "+++++ sendNormal completed successfully" << endl;
}

void _cleanup (CIMClient & client)
{
    try
    {
        _deleteSubscriptionInstance (client, FILTER_NAME,
            SERVER_RESIDENT_HANDLER_NAME);
    }
    catch (CIMException& e)
    {
        if (e.getCode() != CIM_ERR_NOT_FOUND)
        {
            cerr << "----- Error: deleteSubscriptionInstance failure: "
                 << endl;
            throw(e);
        }
    }

    try
    {
        _deleteSubscriptionInstance (client, FILTER_NAME,
            CLIENT_RESIDENT_HANDLER_NAME);
    }
    catch (CIMException& e)
    {
        if (e.getCode() != CIM_ERR_NOT_FOUND)
        {
            cerr << "----- Error: deleteSubscriptionInstance failure: "
                 << endl;
            throw(e);
        }
    }
    try
    {
        _deleteFilterInstance (client, FILTER_NAME);
    }
    catch (CIMException& e)
    {
        if (e.getCode() != CIM_ERR_NOT_FOUND)
        {
            cerr << "----- Error: deleteFilterInstance failure: " << endl;
            throw(e);
        }
    }

    try
    {
        _deleteHandlerInstance (client, SERVER_RESIDENT_HANDLER_NAME);
    }
    catch (CIMException& e)
    {
        if (e.getCode() != CIM_ERR_NOT_FOUND)
        {
            cerr << "----- Error: deleteHandlerInstance failusre: " << endl;
            throw(e);
        }
    }

    try
    {
        _deleteHandlerInstance (client, CLIENT_RESIDENT_HANDLER_NAME);
    }
    catch (CIMException& e)
    {
        if (e.getCode() != CIM_ERR_NOT_FOUND)
        {
            cerr << "----- Error: deleteHandlerInstance failure: " << endl;
            throw(e);
        }
    }
}

/* Status display of the various steps.  Shows message of function and
time to execute.  Grow this to a class so we have start and stop and time
display with success/failure for each function.
*/
static void _testStart(const String& uniqueID, const String& message)
{
    cout << "+++++ thread" << uniqueID << ": " << message << endl;
}

static void _testEnd(const String& uniqueID, const double elapsedTime)
{
    cout << "+++++ thread" << uniqueID << ": passed in " << elapsedTime
        << " seconds" << endl;
}

ThreadReturnType PEGASUS_THREAD_CDECL _executeTests(void *parm)
{
    Thread *my_thread = (Thread *)parm;
    T_Parms *parms = (T_Parms *)my_thread->get_parm();
    CIMClient *client = parms->client;
    Uint32 indicationSendCount = parms->indicationSendCount;
    Uint32 id = parms->uniqueID;
    char id_[4];
    memset(id_,0x00,sizeof(id_));
    sprintf(id_,"%u",id);
    String uniqueID = "_";
    uniqueID.append(id_);

    try
    {
        Stopwatch elapsedTime;

        _testStart(uniqueID,
            "Calling client->invokeMethod to start indication generation");
        elapsedTime.reset();
        elapsedTime.start();
        _sendNormal(client, indicationSendCount);
        elapsedTime.stop();
        _testEnd(uniqueID, elapsedTime.getElapsed());
    }
    catch(Exception e)
    {
        cout << e.getMessage() << endl;
    }

    delete parms;

    return ThreadReturnType(0);
}

Thread * _runTestThreads(
    CIMClient* client,
    Uint32 indicationSendCount,
    Uint32 uniqueID)
{
    // package parameters, create thread and run...
    AutoPtr<T_Parms> parms(new T_Parms());
    parms->client = client;
    parms->indicationSendCount = indicationSendCount;
    parms->uniqueID = uniqueID;
    AutoPtr<Thread> t(new Thread(_executeTests, (void*)parms.release(), false));

    // zzzzz... (1 second) zzzzz...
    Threads::sleep(1000);
    t->run();
    return t.release();
}

int _beginTest(CIMClient& workClient, const char* opt,
     const char* optTwo, const char* optThree, const char* optFour)
{

    Boolean setupCommand =  false;
    Boolean configureServerResidentListener = false;
    Boolean configureClientResidentListener = false;
    String expectedSenderIdentity(String::EMPTY);

    if (String::equalNoCase(opt, "setup"))
    {
       setupCommand = true;
       configureServerResidentListener = true;
       configureClientResidentListener = true;
    }
    else if (String::equalNoCase(opt, "setupSL"))
    {
       setupCommand = true;
       configureServerResidentListener = true;
    }
    else if (String::equalNoCase(opt, "setupCL"))
    {
       setupCommand = true;
       configureClientResidentListener = true;
    }
    if (setupCommand)
    {
        if ((optTwo == NULL) ||
            (!(String::equal(optTwo, "WQL") ||
               String::equal(optTwo, "DMTF:CQL"))))
        {
            cerr << "Invalid query language: '" << optTwo << "'" << endl;
            _usage();
            return -1;
        }
        String qlang(optTwo);

        indicationHandleProtocol handleProtocol = PROTOCOL_CIMXML_INTERNAL;
        if ((optThree == NULL) || (String::equal(optThree, "INTERNAL")))
        {
           handleProtocol = PROTOCOL_CIMXML_INTERNAL;
        }
        else if (String::equal(optThree, "HTTP"))
        {
           handleProtocol = PROTOCOL_CIMXML_HTTP;
        }
        else if (String::equal(optThree, "HTTPS"))
        {
           handleProtocol = PROTOCOL_CIMXML_HTTPS;
        }
        else
        {
           cerr << "Invalid Indication Handler Protocol: '" <<
                optThree << "'" << endl;
           _usage();
           return -1;
        }

        _setup(workClient, qlang, handleProtocol,
            configureServerResidentListener, configureClientResidentListener);
        cout << "+++++ setup completed successfully" << endl;
    }
    else if (String::equalNoCase(opt, "run"))
    {
        if (optTwo == NULL)
        {
            cerr << "Invalid indicationSendCount." << endl;
            _usage ();
            return -1;
        }
        Uint32 indicationSendCount = atoi(optTwo);

        Uint32 runClientThreadCount = 1;
        if (optThree != NULL)
        {
            runClientThreadCount = atoi(optThree);
            if (optFour != NULL)
            {
                expectedSenderIdentity = optFour;
            }
        }

        Boolean monitorClientResidentListener = _subscriptionExists(
            workClient, FILTER_NAME, CLIENT_RESIDENT_HANDLER_NAME);

        Boolean monitorServerResidentListener = _subscriptionExists(
            workClient, FILTER_NAME, SERVER_RESIDENT_HANDLER_NAME);

        if (!monitorServerResidentListener &&
                !monitorClientResidentListener)
        {
            cerr << "Error: No Listeners Configured" << endl;
            return -1;
        }

        if (monitorServerResidentListener)
        {
            _setupServerResidentListener(workClient, expectedSenderIdentity);
        }


        Uint32 portNumber = 2005;
        CIMListener listener(portNumber);
        MyIndicationConsumer* consumer1 = new MyIndicationConsumer("1");

        if (monitorClientResidentListener)
        {
            // Add our consumer
            listener.addConsumer(consumer1);

            // Finish starting the CIMListener
            try
            {
                cout << "+++++ Starting the CIMListener at destination\n";
                if (Ipv6Test)
                {
                    cout  << "     http://[::1]:2005/TestIndicationStressTest";
                }
                else
                {
                    cout  <<
                         "     http://localhost:2005/TestIndicationStressTest";
                }
                cout << endl;

                // Start the listener
                listener.start();
            }
            catch (BindFailedException&)
            {
                // Got a bind error.  The port is probably already in use.
                // Put out a message and fail.
                cerr << endl
                     << "==>WARNING: unable to bind to listener port 2005"
                     << endl;
                cerr << "The listener port may be in use." << endl;
                throw;
            }
        }

        Array<CIMClient *> clientConnections;

        CIMClient * tmpClient;
        for(Uint32 i = 0; i < runClientThreadCount; i++)
        {
            tmpClient = new CIMClient();
            clientConnections.append(tmpClient);
        }
        // determine total number of indication send count
        indicationSendCountTotal = indicationSendCount * runClientThreadCount;


        // calculate the timeout based on the total send count allowing
        // using the MSG_PER_SEC rate
        // allow 20 seconds of test overhead for very small tests

#define MSG_PER_SEC 4

        Uint32 testTimeout = 20000+(indicationSendCountTotal/MSG_PER_SEC)*1000;
        cout << "++++ Estimated test duration = " <<
          testTimeout/60000 << " minutes." << endl;

        // connect the clients
        for(Uint32 i = 0; i < runClientThreadCount; i++)
        {
            clientConnections[i]->setTimeout(testTimeout);
            clientConnections[i]->connectLocal();
        }

        // run tests
        Array<Thread *> clientThreads;

        Stopwatch serverResidentListenerElapsedTime;
        if (monitorServerResidentListener)
        {
            serverResidentListenerElapsedTime.reset();
            serverResidentListenerElapsedTime.start();
        }

        Stopwatch clientResidentListenerElapsedTime;
        if (monitorClientResidentListener)
        {
            clientResidentListenerElapsedTime.reset();
            clientResidentListenerElapsedTime.start();
        }

        for(Uint32 i = 0; i < clientConnections.size(); i++)
        {
            clientThreads.append(_runTestThreads(clientConnections[i],
                indicationSendCount, i));
        }

        for(Uint32 i=0; i< clientThreads.size(); i++)
        {
            clientThreads[i]->join();
        }

        // clean up
        for(Uint32 i=0; i< clientConnections.size(); i++)
        {
            delete clientConnections[i];
        }
        for(Uint32 i=0; i < clientThreads.size(); i++)
        {
            delete clientThreads[i];
        }

        //
        //  Allow time for the indication to be received and forwarded
        //  Wait in SLEEP_SEC second intervals.
        //  Put msg out every MSG_SEC intervals
        //

#define SLEEP_SEC 1
#define COUT_TIME_INTERVAL 30
#define MAX_NO_CHANGE_ITERATIONS COUT_TIME_INTERVAL*3

        Uint32 noChangeIterations = 0;
        Uint32 priorClientResidentIndicationCount = 0;
        Uint32 priorServerResidentIndicationCount = 0;
        Uint32 currentClientResidentIndicationCount = 0;
        Uint32 currentServerResidentIndicationCount = 0;
        Uint32 currentServerResidentIdentityIndicationCount = 0;
        Uint32 totalIterations = 0;

        //
        // Wait for the Listeners to received the expected
        // number of Indications, indicationSendCountTotal.
        //
        // We will continue to wait until either indicationSendCountTotal
        // Indications have been received by the Consumers or no new
        // Indications have been received in the previous
        // MAX_NO_CHANGE_ITERATIONS.
        // iterations.
        //

        Boolean clientResidentCountComplete = !monitorClientResidentListener;
        Boolean serverResidentCountComplete = !monitorServerResidentListener;
        Boolean clientResidentNoChange = true;
        Boolean serverResidentNoChange = true;

        while (noChangeIterations <= MAX_NO_CHANGE_ITERATIONS)
        {
            totalIterations++;
            System::sleep (SLEEP_SEC);

            if (monitorClientResidentListener)
            {
                currentClientResidentIndicationCount =
                    receivedIndicationCount.get();
                if (totalIterations % COUT_TIME_INTERVAL == 1)
                {
                    cout << "*+*+ The Client Resident Listener has received "
                    << currentClientResidentIndicationCount << " of "
                    << indicationSendCountTotal << " Indications."
                    << endl;
                }
                if (indicationSendCountTotal ==
                     currentClientResidentIndicationCount)
                {
                     clientResidentCountComplete = true;
                     clientResidentListenerElapsedTime.stop();
                }
                if (!(clientResidentNoChange =
                        (priorClientResidentIndicationCount ==
                        currentClientResidentIndicationCount)))
                {
                     priorClientResidentIndicationCount =
                         currentClientResidentIndicationCount;
                }
            }
            if (monitorServerResidentListener)
            {
                _getTestResults(workClient,
                    currentServerResidentIndicationCount,
                    currentServerResidentIdentityIndicationCount);
                if (totalIterations % COUT_TIME_INTERVAL == 1)
                {
                     cout << "+*+* The Server Resident Listener has received "
                     << currentServerResidentIndicationCount << " of "
                     << indicationSendCountTotal << " Indications."
                     << endl;
                }
                if (indicationSendCountTotal ==
                        currentServerResidentIndicationCount)
                {
                     serverResidentCountComplete = true;
                     serverResidentListenerElapsedTime.stop();
                }
                if (!(serverResidentNoChange =
                        (priorServerResidentIndicationCount ==
                        currentServerResidentIndicationCount)))
                {
                     priorServerResidentIndicationCount =
                         currentServerResidentIndicationCount;
                }
            }
            if (clientResidentCountComplete && serverResidentCountComplete)
            {
                break;
            }
            if (clientResidentNoChange && serverResidentNoChange)
            {
               noChangeIterations++;
            }
            else
            {
               noChangeIterations = 0;
            }
        }

        if (!serverResidentCountComplete)
        {
            serverResidentListenerElapsedTime.stop();
        }

        if (!clientResidentCountComplete)
        {
            clientResidentListenerElapsedTime.stop();
        }

        if (monitorClientResidentListener)
        {
            cout << "+++++ Stopping the listener"  << endl;
            listener.stop();
            listener.removeConsumer(consumer1);
            delete consumer1;

            cout << endl;
            cout << "+++++ TEST RESULTS: Client Resident Listener " << endl;
            cout << "+++++     Number of Test Threads = "
                 << runClientThreadCount << endl;
            cout << "+++++     Expected Number of Indications = "
                 << indicationSendCountTotal <<endl;
            cout << "+++++     Indications Received  = "
                 << currentClientResidentIndicationCount << endl;
            cout << "+++++     Out of Sequence = "
                 << seqNumberErrors.get() << endl;
            if (sendRecvDeltaTimeCnt)
            {
                cout << "+++++     Avg. Send-Recv Delta time = "
                     << (long)(sendRecvDeltaTimeTotal/1000)/sendRecvDeltaTimeCnt
                     << " milli-seconds" << endl;
            }
            else
            {
                cout << "+++++     Avg. Send-Recv Delta time = "
                     << "*** No Indications Received ***" << endl;
            }
            cout << "+++++     Min. Send-Recv Delta time = "
                 << sendRecvDeltaTimeMin/1000
                 << " milli-seconds" << endl;
            cout << "+++++     Max. Send-Recv Delta time = "
                 << (long)(sendRecvDeltaTimeMax/1000)
                 << " milli-seconds" << endl;
            double eTime =  clientResidentListenerElapsedTime.getElapsed();
            cout << "+++++     Elapsed time = " << eTime
                 << " seconds, or  " << eTime/60
                 << " minutes." << endl;
            cout << "+++++     Rate = "
                 << currentClientResidentIndicationCount/eTime
                 << " indications per second." << endl << endl;
        }
        if (monitorServerResidentListener)
        {
            cout << "+++++ TEST RESULTS: Server Resident Listener " << endl;
            cout << "+++++     Number of Test Threads = "
                 << runClientThreadCount << endl;
            cout << "+++++     Expected Number of Indications = "
                 << indicationSendCountTotal <<endl;
            cout << "+++++     Number of Indications Received  = "
                 << currentServerResidentIndicationCount << endl;
            if (expectedSenderIdentity != String::EMPTY)
            {
                cout << "+++++     Number of Indications Received from "
                     << expectedSenderIdentity << " = "
                     << currentServerResidentIdentityIndicationCount << endl;
            }
            cout << "+++++     Elapsed time = "
                 << serverResidentListenerElapsedTime.getElapsed()
                 << " seconds, or  "
                 << serverResidentListenerElapsedTime.getElapsed()/60
                 << " minutes." << endl;
            cout << "+++++     Rate = "
                 << currentServerResidentIndicationCount/
                        serverResidentListenerElapsedTime.getElapsed()
                 << " indications per second." << endl << endl;
        }

        // assert that all indications sent have been received.
        if (monitorClientResidentListener)
        {
            PEGASUS_TEST_ASSERT(indicationSendCountTotal ==
               currentClientResidentIndicationCount);
        }

        if (monitorServerResidentListener)
        {
            PEGASUS_TEST_ASSERT(indicationSendCountTotal ==
                currentServerResidentIndicationCount);
            if (expectedSenderIdentity != String::EMPTY)
            {
                PEGASUS_TEST_ASSERT(indicationSendCountTotal ==
                    currentServerResidentIdentityIndicationCount);
            }
        }

        // if error encountered then fail the test.
        if (errorsEncountered.get())
          {
          cout << "+++++ test failed" << endl;
          return (-1);
          }
        else
          {
          cout << "+++++ passed all tests" << endl;
          }

    }
    else if (String::equalNoCase (opt, "getSubscriptionCount"))
    {
        Uint32 res = _getCount(workClient);
        cout << " ++++++++ Number of Subscriptions " << res <<endl;
    }
    else if (String::equalNoCase (opt, "cleanup"))
    {
        _cleanup (workClient);
        cout << "+++++ cleanup completed successfully" << endl;
    }
    else
    {
      cerr << "Invalid option: " << opt << endl;
      _usage ();
      return -1;
    }

  return 0;
}

int main (int argc, char** argv)
{
    // This client connection is used soley to create subscriptions.
    CIMClient workClient;
    CIMNamespaceName sourceNamespace;
    try
    {
        workClient.connectLocal();
    }
    catch (Exception & e)
    {
        cerr << e.getMessage () << endl;
        return -1;
    }

    if ((argc <= 3) || (argc > 7))
    {
        cerr << "Invalid argument count: " << argc << endl;
        _usage();
        return 1;
    }

    const char* opt = argv[3];
    const char* optTwo = NULL;
    const char* optThree = NULL;
    const char* optFour = NULL;

    if (argc > 4)
    {
        optTwo = argv[4];
    }

    if (argc > 5)
    {
        optThree = argv[5];
    }

    if (argc > 6)
    {
        optFour = argv[6];
    }

    Ipv6Test = false;

    // Check if class name is IPv6TestClass, handle this class name
    // differently. We use default class-name and namespace for
    // IPv6TestClass class. IPv6TestClass class does not exist, it is used
    // to test IndicationStressTestProvider on IPv6.
    if (!strcmp(argv[1], "IPv6TestClass"))
    {
        Ipv6Test = true;
        indicationClassName = DEFAULT_CLASS_NAME;
        sourceNamespace = DEFAULT_NAMESPACE;
        cout << "++++ Testing with IPv6 LoopBack address " << endl;
    }
    else
    {
        indicationClassName = argv[1];
        sourceNamespace = CIMNamespaceName (argv[2]);
    }
    cout << "++++ Testing with class " << indicationClassName
         << " and Namespace " << sourceNamespace.getString () << endl;

    int rc = 0;

    sourceNamespaces.append("test/testProvider");
    sourceNamespaces.append("test/testIndSrcNS1");
    sourceNamespaces.append("test/testIndSrcNS2");

    try
    {
        rc = _beginTest(workClient, opt, optTwo, optThree, optFour);
    }
    catch (Exception & e)
    {
        cerr << e.getMessage() << endl;
        rc = -1;
    }

    return rc;
}
