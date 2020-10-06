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

#ifndef _IPTESTCLIENT_H
#define _IPTESTCLIENT_H

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/String.h>
#include <Pegasus/Client/CIMClient.h>

PEGASUS_USING_PEGASUS;
PEGASUS_USING_STD;

// Used defines
static const CIMNamespaceName NAMESPACE("root/cimv2");
static const String IPPEP_DESCRIPTION_PREFIX
                    ("IP Protocol Endpoint for ");
static const String IPROUTE_DESCRIPTION_PREFIX
                    ("IP Route for Destination Address:");
static const String NHIPROUTE_DESCRIPTION_PREFIX
                    ("Next Hop IP Route for Destination Address:");
static const String NAME_FORMAT("<Protocol>_<InterfaceName>");


// Class Names

static const CIMName CLASS_PG_BINDS_IP_TO_LAN_ENDPOINT = CIMName
                     ("PG_BindsIPToLANEndpoint");
static const CIMName CLASS_CIM_IP_PROTOCOL_ENDPOINT = CIMName
                     ("CIM_IPProtocolEndpoint");
static const CIMName CLASS_PG_IP_ROUTE = CIMName ("PG_IPRoute");
static const CIMName CLASS_CIM_LAN_ENDPOINT = CIMName ("CIM_LANEndpoint");
static const CIMName CLASS_CIM_UNITARY_COMPUTER_SYSTEM = CIMName
                     ("CIM_UnitaryComputerSystem");
static const CIMName CLASS_PG_NEXT_HOP_IP_ROUTE = CIMName("PG_NextHopIPRoute");


// Property Names

static const CIMName PROPERTY_ADDRESS = CIMName ("Address");
static const CIMName PROPERTY_IPV6_ADDRESS = CIMName ("IPv6Address");
static const CIMName PROPERTY_IPV4_ADDRESS = CIMName ("IPv4Address");
static const CIMName PROPERTY_ADDRESS_TYPE = CIMName ("AddressType");
static const CIMName PROPERTY_ANTECEDENT = CIMName ("Antecedent");
static const CIMName PROPERTY_CAPTION = CIMName ("Caption");
static const CIMName PROPERTY_CREATION_CLASS_NAME = CIMName
                     ("CreationClassName");
static const CIMName PROPERTY_DEPENDENT = CIMName ("Dependent");
static const CIMName PROPERTY_DESCRIPTION = CIMName ("Description");
static const CIMName PROPERTY_DESTINATION_ADDRESS = CIMName
                     ("DestinationAddress");
static const CIMName PROPERTY_DESTINATION_MASK = CIMName ("DestinationMask");
static const CIMName PROPERTY_FRAME_TYPE = CIMName ("FrameType");
static const CIMName PROPERTY_INSTALL_DATE = CIMName ("InstallDate");
static const CIMName PROPERTY_IP_DESTINATION_ADDRESS = CIMName
                     ("IPDestinationAddress");
static const CIMName PROPERTY_IP_DESTINATION_MASK = CIMName
                     ("IPDestinationMask");
static const CIMName PROPERTY_IP_VERSION_SUPPORT = CIMName ("IPVersionSupport");
static const CIMName PROPERTY_IS_STATIC = CIMName ("IsStatic");
static const CIMName PROPERTY_NAME = CIMName ("Name");
static const CIMName PROPERTY_NAME_FORMAT = CIMName ("NameFormat");
static const CIMName PROPERTY_NEXT_HOP = CIMName ("NextHop");
static const CIMName PROPERTY_OTHER_TYPE_DESCRIPTION = CIMName
                     ("OtherTypeDescription");
static const CIMName PROPERTY_PROTOCOL_TYPE = CIMName ("ProtocolType");
static const CIMName PROPERTY_SERVICE_CREATION_CLASS_NAME = CIMName
                     ("ServiceCreationClassName");
static const CIMName PROPERTY_SERVICE_NAME = CIMName ("ServiceName");
static const CIMName PROPERTY_STATUS = CIMName ("Status");
static const CIMName PROPERTY_SUBNET_MASK = CIMName ("SubnetMask");
static const CIMName PROPERTY_SYSTEM_CREATION_CLASS_NAME = CIMName
                     ("SystemCreationClassName");
static const CIMName PROPERTY_SYSTEM_NAME = CIMName ("SystemName");
static const CIMName PROPERTY_PROTOCOL_IF_TYPE = CIMName("ProtocolIFType");
static const CIMName PROPERTY_INSTANCE_ID = CIMName("InstanceID");
static const CIMName PROPERTY_PREFIX_LENGTH = CIMName("PrefixLength");

class IPTestClient
{
public:
    IPTestClient(CIMClient &client);
    ~IPTestClient();

    void testEIN(CIMClient &client, CIMName className, Boolean verbose);
    void testEI(CIMClient &client, CIMName className, Boolean verbose);
    void testGI(CIMClient &client, CIMName className, Boolean verbose);

private:

    // utility methods for common test functions
    void _errorExit(const String &message);
    void _testLog(const String &message);


    // all the checks to see if the properties are valid
    // this are all intended to be in the OS-specific source

    // PG_BindsIPToLANEndpoint Checks
    void _check_BIPTLEp_Antecedent(String &pv, Boolean verbose);
    void _check_BIPTLEp_Dependent(String &pv, Boolean verbose);
    void _check_BIPTLEp_FrameType(Uint16 &pv, Boolean verbose);

    // CIM_IPProtocolEndpoint Checks -- key properties also needed as one of
    //                                  the references of PG_BindsToLANEndpoint
    void _check_IPPEp_Caption(String &pv, Boolean verbose);
    void _check_IPPEp_Description(String &pv, Boolean verbose);
    void _check_IPPEp_InstallDate(CIMDateTime &pv, Boolean verbose);
    void _check_IPPEp_Status(String &pv, Boolean verbose);
    void _check_IPPEp_SystemCreationClassName(String &pv, Boolean verbose);
    void _check_IPPEp_SystemName(String &pv, Boolean verbose);
    void _check_IPPEp_CreationClassName(String &pv, Boolean verbose);
    void _check_IPPEp_Name(String &pv, Boolean verbose);
    void _check_IPPEp_NameFormat(String &pv, Boolean verbose);
    void _check_IPPEp_ProtocolType(Uint16 &pv, Boolean verbose);
    void _check_IPPEp_OtherTypeDescription(String &pv, Boolean verbose);
    void _check_IPPEp_Address(String &pv, Boolean verbose);
    void _check_IPPEp_SubnetMask(String &pv, Boolean verbose);
    void _check_IPPEp_AddressType(Uint16 &pv, Boolean verbose);
    void _check_IPPEp_IPVersionSupport(Uint16 &pv, Boolean verbose);
    void _check_IPPEp_IPv6Address(String &pv, Boolean verbose);
    void _check_IPPEp_IPv4Address(String &pv, Boolean verbose);
    void _check_IPPEp_PrefixLength(Uint8 &pv, Boolean verbose);
    void _check_IPPEp_ProtocolIFType(Uint16 &pv, Boolean verbose);

    // PG_IPRoute Checks
    void _check_IPRoute_Caption(String &pv, Boolean verbose);
    void _check_IPRoute_Description(String &pv, Boolean verbose);
    void _check_IPRoute_InstallDate(CIMDateTime &pv, Boolean verbose);
    void _check_IPRoute_Status(String &pv, Boolean verbose);
    void _check_IPRoute_Name(String &pv, Boolean verbose);
    void _check_IPRoute_NextHop(String &pv, Boolean verbose);
    void _check_IPRoute_IsStatic(Boolean &pv, Boolean verbose);
    void _check_IPRoute_SystemCreationClassName(String &pv, Boolean verbose);
    void _check_IPRoute_SystemName(String &pv, Boolean verbose);
    void _check_IPRoute_ServiceCreationClassName(String &pv, Boolean verbose);
    void _check_IPRoute_ServiceName(String &pv, Boolean verbose);
    void _check_IPRoute_CreationClassName(String &pv, Boolean verbose);
    void _check_IPRoute_IPDestinationAddress(String &pv, Boolean verbose);
    void _check_IPRoute_IPDestinationMask(String &pv, Boolean verbose);
    void _check_IPRoute_AddressType(Uint16 &pv, Boolean verbose);
    void _check_IPRoute_DestinationAddress(String &pv, Boolean verbose);
    void _check_IPRoute_DestinationMask(String &pv, Boolean verbose);

    // PG_NextHopIPRoute Checks.
    void _check_NHIPRoute_InstanceID(String &pv, Boolean verbose);
    void _check_NHIPRoute_DestinationAddress(String &pv, Boolean verbose);
    void _check_NHIPRoute_DestinationMask(String &pv, Boolean verbose);
    void _check_NHIPRoute_PrefixLength(Uint8 &pv, Boolean verbose);
    void _check_NHIPRoute_AddressType(Uint16 &pv, Boolean verbose);
    void _check_NHIPRoute_Caption(String &pv, Boolean verbose);
    void _check_NHIPRoute_Description(String &pv, Boolean verbose);
    void _check_NHIPRoute_Name(String &pv, Boolean verbose);

    // PG_LANEndpoint Checks -- need because it's referenced by
    //                          PG_BindsToLANEndpoint
    void _check_LEP_SystemCreationClassName(String &pv, Boolean verbose);
    void _check_LEP_SystemName(String &pv, Boolean verbose);
    void _check_LEP_CreationClassName(String &pv, Boolean verbose);
    void _check_LEP_Name(String &pv, Boolean verbose);


    // validate keys of the class
    void _validateKeys(
        CIMObjectPath &cimRef,
        CIMName classname,
        Boolean verboseTest);

    // validate properties of the class
    void _validateProperties(
        CIMInstance &cimInst,
        CIMName classname,
        Boolean verboseTest);

    // Validate dot (IPv4) or colon (IPv6) address format.
    void _check_Address_Format(String&, const CIMName&, const CIMName&);
};

#endif
