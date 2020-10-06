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

// Using the general CIMOM TestClient as an example, developed an
// IP Provider test client to exercise the intrinsic methods of
// the CIM_IPProtocolEndpoint, PG_BindsToLANEndpoint, and PG_IPRoute classes,
// testing EnumerateInstanceNames, EnumerateInstances, and GetInstance.

#include "IPTestClient.h"

// include the appropriate IPProvider-specific file for checking results
#if defined (PEGASUS_PLATFORM_HPUX_ACC)
# include "IPTestClient_HPUX.cpp"
#else
# include "IPTestClient_Stub.cpp"
#endif

PEGASUS_USING_PEGASUS;
PEGASUS_USING_STD;

//
//  Constructor for IP Test Client
//
IPTestClient::IPTestClient(CIMClient &client)
{
}

//
//  Destructor for IP Test Client
//
IPTestClient::~IPTestClient()
{
}

//  ErrorExit - Print out the error message as an error and get out.
//    @param - Text for error message
//    @return - None, Terminates the program
//    @exception - This function terminates the program
void IPTestClient::_errorExit(const String& message)
{
    cerr << "Error: " << message << endl;
    cerr << "Re-run with verbose for details (IPTestClient -v)" << endl;
    exit(1);
}

// _testLog method used for messages to really stand out
// for example, Test Start and Test Passed messages
void IPTestClient::_testLog(const String& message)
{
    cout << "+++ " << message << " +++" << endl;
}

//
//   _validateKeys method of the IP provider Test Client
//
void IPTestClient::_validateKeys(
    CIMObjectPath &cimRef,
    CIMName className,
    Boolean verbose)
{
   // don't have a try here - want it to be caught by caller
   CIMName keyName;
   Array<CIMKeyBinding> keyBindings = cimRef.getKeyBindings();

   if (verbose)
   {
      cout << "Retrieved " << keyBindings.size() << " keys for class " <<
          className.getString() << endl;
   }

   for (Uint32 j = 0; j < keyBindings.size(); j++)
   {
      keyName = keyBindings[j].getName();

      if (verbose)
      {
         cout << "  checking key " << keyName.getString() << endl;
      }

      if (className.equal(CLASS_PG_BINDS_IP_TO_LAN_ENDPOINT))
      {

          if (keyName.equal(PROPERTY_ANTECEDENT))
          {
              CIMObjectPath keyVal = keyBindings[j].getValue();
              _validateKeys(keyVal, CLASS_CIM_LAN_ENDPOINT, verbose);
          }

          else if (keyName.equal(PROPERTY_DEPENDENT))
          {
              CIMObjectPath keyVal = keyBindings[j].getValue();
              _validateKeys(keyVal, CLASS_CIM_IP_PROTOCOL_ENDPOINT, verbose);
          }

      } // end of keys for BindToIPProtocolEndpoint

      else if (className.equal(CLASS_CIM_IP_PROTOCOL_ENDPOINT))
      {

          if (keyName.equal(PROPERTY_SYSTEM_CREATION_CLASS_NAME))
          {
              String keyVal = keyBindings[j].getValue();
              _check_IPPEp_SystemCreationClassName(keyVal, verbose);
          }

          else if (keyName.equal(PROPERTY_SYSTEM_NAME))
          {
              String keyVal = keyBindings[j].getValue();
              _check_IPPEp_SystemName(keyVal, verbose);
          }

          else if (keyName.equal(PROPERTY_CREATION_CLASS_NAME))
          {
              String keyVal = keyBindings[j].getValue();
              _check_IPPEp_CreationClassName(keyVal, verbose);
          }

          else if (keyName.equal(PROPERTY_NAME))
          {
              String keyVal = keyBindings[j].getValue();
              _check_IPPEp_Name(keyVal, verbose);
          }

      } // end of keys for IPProtocolEndpoint

      else if (className.equal(CLASS_PG_IP_ROUTE))
      {

          if (keyName.equal(PROPERTY_SYSTEM_CREATION_CLASS_NAME))
          {
              String keyVal = keyBindings[j].getValue();
              _check_IPRoute_SystemCreationClassName(keyVal, verbose);
          }

          else if (keyName.equal(PROPERTY_SYSTEM_NAME))
          {
              String keyVal = keyBindings[j].getValue();
              _check_IPRoute_SystemName(keyVal, verbose);
          }

          else if (keyName.equal(PROPERTY_SERVICE_CREATION_CLASS_NAME))
          {
              String keyVal = keyBindings[j].getValue();
           _check_IPRoute_ServiceCreationClassName(keyVal, verbose);
          }

          else if (keyName.equal(PROPERTY_SERVICE_NAME))
          {
              String keyVal = keyBindings[j].getValue();
              _check_IPRoute_ServiceName(keyVal, verbose);
          }

          else if (keyName.equal(PROPERTY_CREATION_CLASS_NAME))
          {
              String keyVal = keyBindings[j].getValue();
              _check_IPRoute_CreationClassName(keyVal, verbose);
          }

          else if (keyName.equal(PROPERTY_IP_DESTINATION_ADDRESS))
          {
              String keyVal = keyBindings[j].getValue();
              _check_IPRoute_IPDestinationAddress(keyVal, verbose);
          }

          else if (keyName.equal(PROPERTY_IP_DESTINATION_MASK))
          {
              String keyVal = keyBindings[j].getValue();
              _check_IPRoute_IPDestinationMask(keyVal, verbose);
          }

          else if (keyName.equal(PROPERTY_ADDRESS_TYPE))
          {
              String sVal = keyBindings[j].getValue();
              int val;
              sscanf((const char *)sVal.getCString(), "%d", &val);
              Uint16 keyVal = val;
              _check_IPRoute_AddressType(keyVal, verbose);
          }

      } // end of properties for IPRoute

      else if (className.equal(CLASS_CIM_LAN_ENDPOINT))
      {

           if (keyName.equal(PROPERTY_SYSTEM_CREATION_CLASS_NAME))
          {
              String keyVal = keyBindings[j].getValue();
              _check_LEP_SystemCreationClassName(keyVal, verbose);
          }

          else if (keyName.equal(PROPERTY_SYSTEM_NAME))
          {
              String keyVal = keyBindings[j].getValue();
              _check_LEP_SystemName(keyVal, verbose);
          }

          else if (keyName.equal(PROPERTY_CREATION_CLASS_NAME))
          {
              String keyVal = keyBindings[j].getValue();
              _check_LEP_CreationClassName(keyVal, verbose);
          }

          else if (keyName.equal(PROPERTY_NAME))
          {
              String keyVal = keyBindings[j].getValue();
              _check_LEP_Name(keyVal, verbose);
          }

      } // end of keys for LANEndpoint

      else if (className.equal(CLASS_PG_NEXT_HOP_IP_ROUTE))
      {

          if (keyName.equal(PROPERTY_INSTANCE_ID))
          {
              String keyVal = keyBindings[j].getValue();
              _check_NHIPRoute_InstanceID(keyVal, verbose);
          }

      } // end of properties for PG_NexHopIPRoute

   } // end for loop of keys
}

//
//   _validateProperties method of the IP provider Test Client
//
void IPTestClient::_validateProperties(
    CIMInstance &inst,
    CIMName className,
    Boolean verbose)
{
    if (verbose)
    {
        cout << "Checking " << inst.getPropertyCount() <<
            " properties on class " << className.getString() << endl;
    }

    // loop through the properties
    for (Uint32 j=0; j < inst.getPropertyCount(); j++)
    {
        CIMName propertyName = inst.getProperty(j).getName();

        if(verbose)
            cout << "  Property name: " << propertyName.getString() << endl;

        if (className.equal(CLASS_PG_BINDS_IP_TO_LAN_ENDPOINT))
        {

            if (propertyName.equal(PROPERTY_ANTECEDENT))
            {
                String propertyValue;
                inst.getProperty(j).getValue().get(propertyValue);
                _check_BIPTLEp_Antecedent(propertyValue, verbose);
            }

            else if (propertyName.equal(PROPERTY_DEPENDENT))
            {
                String propertyValue;
                inst.getProperty(j).getValue().get(propertyValue);
                _check_BIPTLEp_Dependent(propertyValue, verbose);
            }

            else if (propertyName.equal(PROPERTY_FRAME_TYPE))
            {
                Uint16 propertyValue;
                inst.getProperty(j).getValue().get(propertyValue);
                _check_BIPTLEp_FrameType(propertyValue, verbose);
            }

        } // end of properties for BindToIPProtocolEndpoint

        else if (className.equal(CLASS_CIM_IP_PROTOCOL_ENDPOINT))
        {

            if (propertyName.equal(PROPERTY_CAPTION))
            {
                String propertyValue;
                inst.getProperty(j).getValue().get(propertyValue);
                _check_IPPEp_Caption(propertyValue, verbose);
            }

            else if (propertyName.equal(PROPERTY_DESCRIPTION))
            {
                String propertyValue;
                inst.getProperty(j).getValue().get(propertyValue);
                _check_IPPEp_Description(propertyValue, verbose);
            }

            else if (propertyName.equal(PROPERTY_INSTALL_DATE))
            {
                CIMDateTime propertyValue;
                inst.getProperty(j).getValue().get(propertyValue);
                _check_IPPEp_InstallDate(propertyValue, verbose);
            }

            else if (propertyName.equal(PROPERTY_STATUS))
            {
                String propertyValue;
                inst.getProperty(j).getValue().get(propertyValue);
                _check_IPPEp_Status(propertyValue, verbose);
            }

            else if (propertyName.equal(PROPERTY_SYSTEM_CREATION_CLASS_NAME))
            {
                String propertyValue;
                inst.getProperty(j).getValue().get(propertyValue);
                _check_IPPEp_SystemCreationClassName(propertyValue, verbose);
            }

            else if (propertyName.equal(PROPERTY_SYSTEM_NAME))
            {
                String propertyValue;
                inst.getProperty(j).getValue().get(propertyValue);
                _check_IPPEp_SystemName(propertyValue, verbose);
            }

            else if (propertyName.equal(PROPERTY_CREATION_CLASS_NAME))
            {
                String propertyValue;
                inst.getProperty(j).getValue().get(propertyValue);
                _check_IPPEp_CreationClassName(propertyValue, verbose);
            }

            else if (propertyName.equal(PROPERTY_NAME))
            {
                String propertyValue;
                inst.getProperty(j).getValue().get(propertyValue);
                _check_IPPEp_Name(propertyValue, verbose);
            }

            else if (propertyName.equal(PROPERTY_NAME_FORMAT))
            {
                String propertyValue;
                inst.getProperty(j).getValue().get(propertyValue);
                _check_IPPEp_NameFormat(propertyValue, verbose);
            }

            else if (propertyName.equal(PROPERTY_PROTOCOL_TYPE))
            {
                Uint16 propertyValue;
                inst.getProperty(j).getValue().get(propertyValue);
                _check_IPPEp_ProtocolType(propertyValue, verbose);
            }

            else if (propertyName.equal(PROPERTY_OTHER_TYPE_DESCRIPTION))
            {
                String propertyValue;
                inst.getProperty(j).getValue().get(propertyValue);
                _check_IPPEp_OtherTypeDescription(propertyValue, verbose);
            }

            else if (propertyName.equal(PROPERTY_ADDRESS))
            {
                String propertyValue;
                inst.getProperty(j).getValue().get(propertyValue);
                _check_IPPEp_Address(propertyValue, verbose);
            }

            else if (propertyName.equal(PROPERTY_SUBNET_MASK))
            {
                String propertyValue;
                inst.getProperty(j).getValue().get(propertyValue);
                _check_IPPEp_SubnetMask(propertyValue, verbose);
            }

            else if (propertyName.equal(PROPERTY_ADDRESS_TYPE))
            {
                Uint16 propertyValue;
                inst.getProperty(j).getValue().get(propertyValue);
                _check_IPPEp_AddressType(propertyValue, verbose);
            }

            else if (propertyName.equal(PROPERTY_IP_VERSION_SUPPORT))
            {
                Uint16 propertyValue;
                inst.getProperty(j).getValue().get(propertyValue);
                _check_IPPEp_IPVersionSupport(propertyValue, verbose);
            }

            else if (propertyName.equal(PROPERTY_PREFIX_LENGTH))
            {
                Uint8 propertyValue;
                inst.getProperty(j).getValue().get(propertyValue);
                _check_IPPEp_PrefixLength(propertyValue, verbose);
            }

            else if (propertyName.equal(PROPERTY_IPV6_ADDRESS))
            {
                String propertyValue;
                inst.getProperty(j).getValue().get(propertyValue);
                _check_IPPEp_IPv6Address(propertyValue, verbose);
            }

            else if (propertyName.equal(PROPERTY_IPV4_ADDRESS))
            {
                String propertyValue;
                inst.getProperty(j).getValue().get(propertyValue);
                _check_IPPEp_IPv4Address(propertyValue, verbose);
            }

            else if (propertyName.equal(PROPERTY_PROTOCOL_IF_TYPE))
            {
                Uint16 propertyValue;
                inst.getProperty(j).getValue().get(propertyValue);
                _check_IPPEp_ProtocolIFType(propertyValue, verbose);
            }

        } // end of properties for IPProtocolEndpoint

        else if (className.equal(CLASS_PG_IP_ROUTE))
        {

            if (propertyName.equal(PROPERTY_CAPTION))
            {
                String propertyValue;
                inst.getProperty(j).getValue().get(propertyValue);
                _check_IPRoute_Caption(propertyValue, verbose);
            }

            else if (propertyName.equal(PROPERTY_DESCRIPTION))
            {
                String propertyValue;
                inst.getProperty(j).getValue().get(propertyValue);
                _check_IPRoute_Description(propertyValue, verbose);
            }

            else if (propertyName.equal(PROPERTY_INSTALL_DATE))
            {
                CIMDateTime propertyValue;
                inst.getProperty(j).getValue().get(propertyValue);
                _check_IPRoute_InstallDate(propertyValue, verbose);
            }

            else if (propertyName.equal(PROPERTY_STATUS))
            {
                String propertyValue;
                inst.getProperty(j).getValue().get(propertyValue);
                _check_IPRoute_Status(propertyValue, verbose);
            }

            else if (propertyName.equal(PROPERTY_NAME))
            {
                String propertyValue;
                inst.getProperty(j).getValue().get(propertyValue);
                _check_IPRoute_Name(propertyValue, verbose);
            }

            else if (propertyName.equal(PROPERTY_NEXT_HOP))
            {
                String propertyValue;
                inst.getProperty(j).getValue().get(propertyValue);
                _check_IPRoute_NextHop(propertyValue, verbose);
            }

            else if (propertyName.equal(PROPERTY_IS_STATIC))
            {
                Boolean propertyValue;
                inst.getProperty(j).getValue().get(propertyValue);
                _check_IPRoute_IsStatic(propertyValue, verbose);
            }

            else if (propertyName.equal(PROPERTY_SYSTEM_CREATION_CLASS_NAME))
            {
                String propertyValue;
                inst.getProperty(j).getValue().get(propertyValue);
                _check_IPRoute_SystemCreationClassName(propertyValue, verbose);
            }

            else if (propertyName.equal(PROPERTY_SYSTEM_NAME))
            {
                String propertyValue;
                inst.getProperty(j).getValue().get(propertyValue);
                _check_IPRoute_SystemName(propertyValue, verbose);
            }

            else if (propertyName.equal(PROPERTY_SERVICE_CREATION_CLASS_NAME))
            {
                String propertyValue;
                inst.getProperty(j).getValue().get(propertyValue);
                _check_IPRoute_ServiceCreationClassName(propertyValue, verbose);
            }

            else if (propertyName.equal(PROPERTY_SERVICE_NAME))
            {
                String propertyValue;
                inst.getProperty(j).getValue().get(propertyValue);
                _check_IPRoute_ServiceName(propertyValue, verbose);
            }

            else if (propertyName.equal(PROPERTY_CREATION_CLASS_NAME))
            {
                String propertyValue;
                inst.getProperty(j).getValue().get(propertyValue);
                _check_IPRoute_CreationClassName(propertyValue, verbose);
            }

            else if (propertyName.equal(PROPERTY_IP_DESTINATION_ADDRESS))
            {
                String propertyValue;
                inst.getProperty(j).getValue().get(propertyValue);
                _check_IPRoute_IPDestinationAddress(propertyValue, verbose);
            }

            else if (propertyName.equal(PROPERTY_IP_DESTINATION_MASK))
            {
                String propertyValue;
                inst.getProperty(j).getValue().get(propertyValue);
                _check_IPRoute_IPDestinationMask(propertyValue, verbose);
            }

            else if (propertyName.equal(PROPERTY_ADDRESS_TYPE))
            {
                Uint16 propertyValue;
                inst.getProperty(j).getValue().get(propertyValue);
                _check_IPRoute_AddressType(propertyValue, verbose);
            }

            else if (propertyName.equal(PROPERTY_DESTINATION_ADDRESS))
            {
                String propertyValue;
                inst.getProperty(j).getValue().get(propertyValue);
                _check_IPRoute_DestinationAddress(propertyValue, verbose);
            }

            else if (propertyName.equal(PROPERTY_DESTINATION_MASK))
            {
                String propertyValue;
                inst.getProperty(j).getValue().get(propertyValue);
                _check_IPRoute_DestinationMask(propertyValue, verbose);
            }

        } // end of properties for IPRoute

        else if (className.equal(CLASS_PG_NEXT_HOP_IP_ROUTE))
        {

            if (propertyName.equal(PROPERTY_NAME))
            {
                String propertyValue;
                inst.getProperty(j).getValue().get(propertyValue);
                _check_NHIPRoute_Name(propertyValue, verbose);
            }

            else if (propertyName.equal(PROPERTY_CAPTION))
            {
                String propertyValue;
                inst.getProperty(j).getValue().get(propertyValue);
                _check_NHIPRoute_Caption(propertyValue, verbose);
            }

            else if (propertyName.equal(PROPERTY_DESCRIPTION))
            {
                String propertyValue;
                inst.getProperty(j).getValue().get(propertyValue);
                _check_NHIPRoute_Description(propertyValue, verbose);
            }

            else if (propertyName.equal(PROPERTY_DESTINATION_ADDRESS))
            {
                String propertyValue;
                inst.getProperty(j).getValue().get(propertyValue);
                _check_NHIPRoute_DestinationAddress(propertyValue, verbose);
            }

            else if (propertyName.equal(PROPERTY_DESTINATION_MASK))
            {
                String propertyValue;
                inst.getProperty(j).getValue().get(propertyValue);
                _check_NHIPRoute_DestinationMask(propertyValue, verbose);
            }

            else if (propertyName.equal(PROPERTY_PREFIX_LENGTH))
            {
                Uint8 propertyValue;
                inst.getProperty(j).getValue().get(propertyValue);
                _check_NHIPRoute_PrefixLength(propertyValue, verbose);
            }

            else if (propertyName.equal(PROPERTY_ADDRESS_TYPE))
            {
                Uint16 propertyValue;
                inst.getProperty(j).getValue().get(propertyValue);
                _check_NHIPRoute_AddressType(propertyValue, verbose);
            }

        } // end of properties for PG_NextHopIPRoute.


   }  // end for loop of all properties

}


//
//   testEnumerateInstanceNames of the IP provider.
//
void IPTestClient::testEI(
    CIMClient &client,
    CIMName className,
    Boolean verbose)
{
    try
    {
        _testLog("IPTestClient: Starting EIN for class " +
            className.getString());

        Array<CIMObjectPath> cimReferences =
            client.enumerateInstanceNames(NAMESPACE, className);

        Uint32 numberInstances = cimReferences.size();

        if (verbose)
        {
            cout << numberInstances << " instance(s) of " <<
                className.getString() << endl;
        }

        for (Uint32 i = 0; i < numberInstances; i++)
        {
            CIMName className = cimReferences[i].getClassName();
            if (cimReferences[i].getClassName() != className)
            {
                _errorExit("EnumInstanceNames failed - wrong class");
            }

            _validateKeys(cimReferences[i], className, verbose);

        }   // end for looping through instances

        _testLog("IPTestClient: EIN Passed for class " +
            className.getString());

    }  // end try

    catch(Exception& e)
    {
        _errorExit(e.getMessage());
    }
}

//
//   testEnumerateInstances of the IP provider.
//
void IPTestClient::testEIN(
    CIMClient &client,
    CIMName className,
    Boolean verbose)
{
    try
    {
        Boolean deepInheritance = true;
        Boolean localOnly = true;
        Boolean includeQualifiers = false;
        Boolean includeClassOrigin = false;

        _testLog("IPTestClient: Starting EI for class " +
            className.getString());

        Array<CIMInstance> cimNInstances =
            client.enumerateInstances(NAMESPACE, className,
                deepInheritance, localOnly,
                includeQualifiers, includeClassOrigin);

        Uint32 numberInstances = cimNInstances.size();

        if (verbose)
        {
            cout << numberInstances << " instance(s) of " <<
                className.getString() << endl;
        }

        for (Uint32 i = 0; i < numberInstances; i++)
        {
            CIMObjectPath instanceRef = cimNInstances[i].getPath ();

            if (verbose)
                cout<<"Instance ClassName is " <<
                    instanceRef.getClassName().getString() << endl;

            if( !(instanceRef.getClassName().equal( className.getString() ) ) )
            {
                _errorExit("EnumInstances failed");
            }

            // now validate the properties
            _validateProperties(cimNInstances[i], className, verbose);

        }  // end for looping through instances

        _testLog("IPTestClient: EI Passed for class " +
            className.getString());

    }  // end try

    catch(Exception& e)
    {
        _errorExit(e.getMessage());
    }
}

//
//   testGetInstance of the IP provider.
//
void IPTestClient::testGI(
    CIMClient &client,
    CIMName className,
    Boolean verbose)
{
    try
    {
        _testLog("IPTestClient: Starting GI for class " +
            className.getString());

        // first do an EnumerateInstanceNames - select one to play with
        // doesn't hurt to keep testing enumerate :-)

        Boolean localOnly = true;
        Boolean includeQualifiers = false;
        Boolean includeClassOrigin = false;

        Array<CIMObjectPath> cimReferences =
            client.enumerateInstanceNames(NAMESPACE, className);

        Uint32 numberInstances = cimReferences.size();

        if (verbose)
        {
            cout << numberInstances << " instance(s) of " <<
                className.getString() << endl;
        }

        for (Uint32 i = 0; i < numberInstances; i++)
        {
            CIMName thisClassName = cimReferences[i].getClassName();
            if (!(thisClassName.equal(className.getString())))
            {
                _errorExit("EnumInstanceNames failed - wrong class");
            }

            // add in some content checks on the keys returned

            if (verbose)
            {
                cout << " Validating keys of " << className.getString() <<
                    " instance #" << i << " . . ." << endl;
            }

            _validateKeys(cimReferences[i], className, verbose);


            // now call GetInstance with the appropriate references
            CIMInstance getTestInstance =
                client.getInstance(NAMESPACE,
                    cimReferences[i],
                    localOnly,
                    includeQualifiers,
                    includeClassOrigin);

            if (verbose)
            {
                cout << " Validating properties of " <<
                    className.getString() << " instance #" <<
                    i << " . . ." << endl;
            }

            // now validate the properties returned
            _validateProperties(getTestInstance, className, verbose);

        }   // end for looping through instances

        _testLog("IPTestClient: GI passed for class " +
            className.getString());

    }  // end try

    catch(Exception& e)
    {
        _errorExit(e.getMessage());
    }
}

///////////////////////////////////////////////////////////////
//    MAIN
///////////////////////////////////////////////////////////////
int main(int argc, char** argv)
{
    Boolean verbose = false;

    // check what's on the command line

    for (int i=1; i < argc; i++)
    {
        String Arg = argv[i];

        if (Arg.find('h') != PEG_NOT_FOUND)
        {
            cout << "usage: " << argv[0] << " [ -h | -v ]" << endl;
            cout << "  -h: prints this message" << endl;
            cout << "  -v: verbose output" << endl;
            return 0;
        }

        if (Arg.find('v') != PEG_NOT_FOUND)
        {
           verbose = true;
        }

    }  // for

    // need to first connect to the CIMOM

    if (verbose)
        cout << "Starting IP Client test" << endl;

    try
    {
        if (verbose)
            cout << "Create client" << endl;

        // specify the timeout value for the connection (if inactive)
        // in milliseconds, thus setting to one minute
        CIMClient client;
        client.setTimeout(60 * 1000);

        if (verbose)
            cout << "Client created" << endl;

        cout << "IPTestClient connecting to local host" << endl;

        client.connectLocal();    // Connect to local host
        cout << "IPTestClient Connected" << endl;

        IPTestClient testClient(client);

        testClient.testEIN(client, CLASS_PG_IP_ROUTE, verbose);
        testClient.testEI(client, CLASS_PG_IP_ROUTE, verbose);
        testClient.testGI(client, CLASS_PG_IP_ROUTE, verbose);

        testClient.testEIN(client, CLASS_CIM_IP_PROTOCOL_ENDPOINT, verbose);
        testClient.testEI(client, CLASS_CIM_IP_PROTOCOL_ENDPOINT, verbose);
        testClient.testGI(client, CLASS_CIM_IP_PROTOCOL_ENDPOINT, verbose);

        testClient.testEIN(client, CLASS_PG_BINDS_IP_TO_LAN_ENDPOINT, verbose);
        testClient.testEI(client, CLASS_PG_BINDS_IP_TO_LAN_ENDPOINT, verbose);
        testClient.testGI(client, CLASS_PG_BINDS_IP_TO_LAN_ENDPOINT, verbose);

        testClient.testEIN(client, CLASS_PG_NEXT_HOP_IP_ROUTE, verbose);
        testClient.testEI(client, CLASS_PG_NEXT_HOP_IP_ROUTE, verbose);
        testClient.testGI(client, CLASS_PG_NEXT_HOP_IP_ROUTE, verbose);

        cout << "IPTestClient disconnecting from CIMOM " << endl;
        client.disconnect();
    }
    catch(Exception& e)
    {
       cout << "---- IP Provider Test Failed " << e.getMessage() << endl;
    }

    return 0;
}
