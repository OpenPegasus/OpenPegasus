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
//%////////////////////////////////////////////////////////////////////////////

#ifndef PG_IP_HPUX_H
#define PG_IP_HPUX_H

// =============================================================================
// Includes.
// =============================================================================

#include <Pegasus/Provider/CIMInstanceProvider.h>
#include <unistd.h>       // gethostname
#include <sys/socket.h>   // gethostbyname
#include <netinet/in.h>   // gethostbyname
#include <netdb.h>        // gethostbyname
extern "C" {
#define __STDC__
#include <sys/mib.h>      // open_mib, close_mib, get_mib_info
}
#include <arpa/inet.h>    // ioctl
#include <net/if.h>       // ioctl
#include <string.h>       // strerror
#include <errno.h>        // errno
#include <vector>         // vector container

PEGASUS_USING_STD;
PEGASUS_USING_PEGASUS;

// =============================================================================
// Defines.
// =============================================================================

// Support IP PROTOCOL TYPES

#define PROTOCOL_IPV4     "IPv4"
#define PROTOCOL_IPV6     "IPv6"
#define PROTOCOL_IPV4_V6  "IPv4/v6"

// =============================================================================
// Type Definitions.
// =============================================================================

class IPInterface
{
public:

    IPInterface();
    ~IPInterface();

    // Property Accessor Methods

    Boolean getCaption(String&) const;
    Boolean getDescription(String&) const;
    Boolean getInstallDate(CIMDateTime&) const;
    Boolean getName(String&) const;
    Boolean getStatus(String&) const;
    Boolean getNameFormat(String&) const;
    Boolean getProtocolType(Uint16&) const;
    Boolean getOtherTypeDescription(String&) const;
    Boolean getAddress(String&) const;
    Boolean getIPv4Address(String&) const;
    Boolean getIPv6Address(String&) const;
    Boolean getPrefixLength(Uint8&) const;
    Boolean getSubnetMask(String&) const;
    Boolean getAddressType(Uint16&) const;
    Boolean getIPVersionSupport(Uint16&) const;
    Boolean getProtocolIFType(Uint16&) const;
    Boolean getFrameType(Uint16&) const;

    // System name is the same for all instances
    static Boolean getSystemName(String&);

    // Method to Get the Simple Interface Name before the colon (":")
    String get_LANInterfaceName() const;

    // Method to determine if this IP interface ties to a LAN Interface
    Boolean bindsToLANInterface() const;

    // Method to obtain/save the fully qualified host name
    static void initSystemName();

    // Methods to Load Instances

    void set_address(const String& addr);
    void set_prefixLength(const Uint8& pl);
    void set_subnetMask(const String& snm);
    void set_protocol(const String& proto);
    void set_simpleIfName(const String& name);

private:

    String _address;           // dot (IPv4) colon (IPv6) notation
    String _subnetMask;        // dot (IPv4) notation
    Uint8  _prefixLength;      // IPv6 Prefix Length
    String _protocol;          // e.g. IPv4, IPv6
    String _simpleIfName;      // e.g. lan0, lan0:1, lan1, lo0
    static String _hostname;   // fully qualified hostname

};

class InterfaceList
{
public:

    InterfaceList();  // Constructor - should load list of interfaces

    ~InterfaceList();

    // Method to get a particular element based on an Interface Name
    // Returns false if one is not found.
    Boolean findInterface(
        const String &ifName,
        IPInterface &ipIfInst) const;

    // Method to get a particular element based on an index
    IPInterface getInterface(const int index) const;

    // Number of Elements in the InterfaceList
    int size() const;

private:

    vector<IPInterface> _ifl;

};

class IPRoute
{
public:

    IPRoute();
    ~IPRoute();

    // Property Accessor Methods

    Boolean getCaption(String&) const;
    Boolean getDescription(String&) const;
    Boolean getInstallDate(CIMDateTime&) const;
    Boolean getName(String&) const;
    Boolean getStatus(String&) const;
    Boolean getDestinationAddress(String&) const;
    Boolean getDestinationMask(String&) const;
    Boolean getNextHop(String&) const;
    Boolean getIsStatic(Boolean&) const;
    Boolean getAddressType(Uint16&) const;

    // Methods to Load Instances

    void set_destAddr(const String& addr);
    void set_destMask(const String& mask);
    void set_nextHop(const String& nhop);
    void set_protocolType(const String& pt);

private:

    String _destAddr;          // dot (IPv4) or colon (IPv6) notation
    String _destMask;          // dot (IPv4) or colon (IPv6) notation
    String _nextHop;           // dot (IPv4) or colon (IPv6) notation
    String _protocolType;      // IPv4 or IPv6
    static String _hostname;   // fully qualified hostname

};

class RouteList
{
public:

    RouteList();  // Constructor - should load list of interfaces

    ~RouteList();

    // Method to find a particular IP Route based on a Destination
    // Address, Destination Mask, and Address Type (e.g. IPv4):
    // Returns false if a match is not found.
    Boolean findRoute(
        const String &destAddr,
        const String &destMask,
        const Uint16 &addrType,
        IPRoute &ipRInst) const;

    // Method to get a particular element based on an index
    IPRoute getRoute(const int index) const;

    // Number of Elements in the IP Route
    int size() const;

private:

    vector<IPRoute> _iprl;

};

class NextHopIPRoute
{
public:

    NextHopIPRoute();
    virtual ~NextHopIPRoute();


    // Property Accessor Methods

    Boolean getCaption(String&) const;
    Boolean getDescription(String&) const;
    Boolean getInstallDate(CIMDateTime&) const;
    Boolean getName(String&) const;
    Boolean getStatus(String&) const;
    Boolean getDestinationAddress(String&) const;
    Boolean getDestinationMask(String&) const;
    Boolean getRouteDerivation(Uint16&) const;
    Boolean getOtherDerivation(String&) const;
    Boolean getPrefixLength(Uint8&) const;
    Boolean getInstanceID(String&) const;
    Boolean getAdminDistance(Uint16&) const;
    Boolean getRouteMetric(Uint16&) const;
    Boolean getTypeOfRoute(Uint16&) const;
    Boolean getNextHop(String&) const;
    Boolean getIsStatic(Boolean&) const;
    Boolean getAddressType(Uint16&) const;

    // Methods to check if a given next hop route is local.
    Boolean isRouteLocal() const;

    // Methods to Load Instances
    void set_prefixLength(const Uint8 &pl);
    void set_nextHop(const String& nhop);
    void set_routeType(const Uint16& rt);
    void set_destAddr(const String& addr);
    void set_destMask(const String& dm);
    void set_protocolType(const String& pt);

private:

    Uint8 _prefixLength;      // IPv6 Prefix Length
    String _destAddr;
    String _protocolType;
    String _destMask;

protected:
    String _nextHop;         // colon (IPv6) notation.
    Uint16 _routeType;       // Says if route is local or remote type.

};

class NextHopRouteList
{
public:

    NextHopRouteList();  // Constructor - should load list of interfaces

    virtual ~NextHopRouteList();

    // Method to find a particular IP Route based on a Instance ID
    // Returns false if a match is not found.
    Boolean findRoute(
        const String &instanceID,
        NextHopIPRoute &ipRInst) const;

    // Method to get a particular element based on an index
    NextHopIPRoute getRoute(const int index) const;

    // Number of Elements in the IP Route
    int size() const;

protected:

    vector<NextHopIPRoute> _nhiprl;

};

class RSAp : public NextHopIPRoute
{
public:
    RSAp();
    ~RSAp();


    // Property Accessor Methods

    Boolean getName(String&) const;
    Boolean getAccessInfo(String&) const;
    Boolean getInfoFormat(Uint16&) const;
    Boolean getOtherInfoFormatDescription(String&) const;

    // Methods to Load Instances
    void set_name(const String&);
    void set_accessInfo(const String&);
    void set_infoFormat(const Uint16&);
    void set_otherInfoFmtDesc(const String&);

private:
    String _name;
    String _accessInfo;
    Uint16 _infoFormat;
    String _otherInfoFmtDesc;
};

class RSApList : public NextHopRouteList
{
public:

    RSApList();  // Constructor - should load list of remote services

    ~RSApList();

    // Method to find a particular remote service based on Name key property.
    // Returns false if a match is not found.
    Boolean findService(
        const String &name,
        RSAp &rsap) const;

    // Method to get a particular element based on an index
    RSAp getService(const Uint16 index) const;

    // Number of Elements in the Remote Services Access Point List.
    int size() const;

private:

    vector<RSAp> _rsapl;

};

#endif  /* #ifndef PG_IP_HPUX_H */
