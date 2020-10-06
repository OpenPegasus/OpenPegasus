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

/* ==========================================================================
   Includes.
   ========================================================================== */

#include "IPPlatform.h"

PEGASUS_USING_STD;
PEGASUS_USING_PEGASUS;

IPInterface::IPInterface()
{
}

IPInterface::~IPInterface()
{
}

/*
================================================================================
NAME              : getCaption
DESCRIPTION       :
ASSUMPTIONS       :
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
================================================================================
*/
Boolean IPInterface::getCaption(String& s) const
{
  s = String::EMPTY;
  return true;
}

/*
================================================================================
NAME              : getDescription
DESCRIPTION       :
ASSUMPTIONS       :
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
================================================================================
*/
Boolean IPInterface::getDescription(String& s) const
{
  s = String::EMPTY;
  return true;
}

/*
================================================================================
NAME              : getInstallDate
DESCRIPTION       :
ASSUMPTIONS       :
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
================================================================================
*/
Boolean IPInterface::getInstallDate(CIMDateTime& d) const
{
  // Not supported. This property is inherited from
  // CIM_ManagedSystemElement, but has no useful meaning
  // for an IP address.

  return false;
}

/*
================================================================================
NAME              : getName
DESCRIPTION       :
ASSUMPTIONS       :
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
================================================================================
*/
Boolean IPInterface::getName(String& s) const
{
  // not supported
  return false;
}

/*
================================================================================
NAME              : getStatus
DESCRIPTION       :
ASSUMPTIONS       :
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
================================================================================
*/
Boolean IPInterface::getStatus(String& s) const
{
  // This property is inherited from CIM_ManagedSystemElement,
  // is not relevant.

  return false;
}


/*
================================================================================
NAME              : getSystemName
DESCRIPTION       : Platform-specific routine to get the System Name
ASSUMPTIONS       : None
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
================================================================================
*/
Boolean IPInterface::getSystemName(String& s)
{
  // not supported
  return false;
}


/*
================================================================================
NAME              : getNameFormat
DESCRIPTION       :
ASSUMPTIONS       :
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
================================================================================
*/
Boolean IPInterface::getNameFormat(String& s) const
{
  // not supported
  return false;
}

/*
================================================================================
NAME              : getProtocolType
DESCRIPTION       :
ASSUMPTIONS       :
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
================================================================================
*/
Boolean IPInterface::getProtocolType(Uint16& i16) const
{
  // not supported
  return false;
}

/*
================================================================================
NAME              : getOtherTypeDesription
DESCRIPTION       :
ASSUMPTIONS       :
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
================================================================================
*/
Boolean IPInterface::getOtherTypeDescription(String& s) const
{
  // not supported
  return false;
}

/*
================================================================================
NAME              : getAddress
DESCRIPTION       :
ASSUMPTIONS       :
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
================================================================================
*/
Boolean IPInterface::getAddress(String& s) const
{
  // not supported
  return false;
}

/*
================================================================================
NAME              : getIPv4Address
DESCRIPTION       : The IPv4 address that this ProtocolEndpoint represents.
ASSUMPTIONS       :
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
================================================================================
*/
Boolean IPInterface::getIPv4Address(String& s) const
{
  // not supported
  return false;
}

/*
================================================================================
NAME              : getIPv6Address
DESCRIPTION       : The IPv6 address that this ProtocolEndpoint represents.
ASSUMPTIONS       :
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
================================================================================
*/
Boolean IPInterface::getIPv6Address(String& s) const
{
  // not supported
  return false;
}

/*
================================================================================
NAME              : getPrefixLength
DESCRIPTION       :
ASSUMPTIONS       :
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
================================================================================
*/
Boolean IPInterface::getPrefixLength(Uint8& i8) const
{
    // not supported.
    return false;
}

/*
================================================================================
NAME              : getSubnetMask
DESCRIPTION       :
ASSUMPTIONS       :
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
================================================================================
*/
Boolean IPInterface::getSubnetMask(String& s) const
{
  // not supported
  return false;
}

/*
================================================================================
NAME              : getAddressType
DESCRIPTION       :
ASSUMPTIONS       :
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
================================================================================
*/
Boolean IPInterface::getAddressType(Uint16& i16) const
{
  // not supported
  return false;
}

/*
================================================================================
NAME              : getIPVersionSupport
DESCRIPTION       :
ASSUMPTIONS       :
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
================================================================================
*/
Boolean IPInterface::getIPVersionSupport(Uint16& i16) const
{
  // not supported
  return false;
}

/*
================================================================================
NAME              : getProtocolIFType
DESCRIPTION       :
ASSUMPTIONS       :
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             : Deprecated by ProtocolIFType
================================================================================
*/
Boolean IPInterface::getProtocolIFType(Uint16& i16) const
{
  // not supported
  return false;
}

/*
================================================================================
NAME              : getFrameType
DESCRIPTION       :
ASSUMPTIONS       :
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
================================================================================
*/
Boolean IPInterface::getFrameType(Uint16& i16) const
{
  // not supported
  return false;
}


/*
================================================================================
NAME              : get_LANInterfaceName
DESCRIPTION       :
ASSUMPTIONS       :
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
================================================================================
*/
String IPInterface::get_LANInterfaceName() const
{
  // This routine must be written to return a value.
  return String::EMPTY;
}

/*
================================================================================
NAME              : bindsToLANInterface
DESCRIPTION       :
ASSUMPTIONS       :
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
================================================================================
*/
Boolean IPInterface::bindsToLANInterface() const
{
  return false;
}


/*
================================================================================
NAME              : initSystemName
DESCRIPTION       : Platform-specific routine to get the System Name
ASSUMPTIONS       : None
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
================================================================================
*/
void IPInterface::initSystemName()
{
  // Do nothing
}

/*
================================================================================
NAME              : InterfaceList Constructor
DESCRIPTION       : Build the list IP Interfaces
ASSUMPTIONS       : None
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
================================================================================
*/
InterfaceList::InterfaceList()
{
}

/*
================================================================================
NAME              : InterfaceList Destructor
DESCRIPTION       : None
ASSUMPTIONS       : None
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
================================================================================
*/
InterfaceList::~InterfaceList()
{
}


/*
================================================================================
NAME              : findInterface
DESCRIPTION       : find the requested interface
ASSUMPTIONS       : None
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
================================================================================
*/
Boolean InterfaceList::findInterface(
    const String &ifName,
    IPInterface &ipIfInst) const
{
    // Always return interface not found
    return false;
}


/*
================================================================================
NAME              : getInterface
DESCRIPTION       : Get an interface based on an index.
ASSUMPTIONS       : None
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
================================================================================
*/
IPInterface InterfaceList::getInterface(const int index) const
{
    // give an interface (this should never get called since size will
    // always be zero).
    IPInterface i;
    return i;
}

/*
================================================================================
NAME              : size
DESCRIPTION       : Find the size of the Interface List.
ASSUMPTIONS       : None
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
================================================================================
*/
int InterfaceList::size() const
{
    // no interfaces
    return 0;
}

/////////////////////////////////////////////////////////////////////////
//
// PG_IPRoute related methods
//
/////////////////////////////////////////////////////////////////////////

IPRoute::IPRoute()
{
}

IPRoute::~IPRoute()
{
}

/*
================================================================================
NAME              : getCaption
DESCRIPTION       :
ASSUMPTIONS       :
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
================================================================================
*/
Boolean IPRoute::getCaption(String& s) const
{
  s = String::EMPTY;
  return true;
}

/*
================================================================================
NAME              : getDescription
DESCRIPTION       :
ASSUMPTIONS       :
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
================================================================================
*/
Boolean IPRoute::getDescription(String& s) const
{
  s = String::EMPTY;
  return true;
}

/*
================================================================================
NAME              : getInstallDate
DESCRIPTION       :
ASSUMPTIONS       :
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
================================================================================
*/
Boolean IPRoute::getInstallDate(CIMDateTime& d) const
{
  // Not supported. This property is inherited from
  // CIM_ManagedSystemElement, but has no useful meaning
  // for an IP Route.

  return false;
}

/*
================================================================================
NAME              : getName
DESCRIPTION       :
ASSUMPTIONS       :
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
================================================================================
*/
Boolean IPRoute::getName(String& s) const
{
  // not supported
  return false;
}

/*
================================================================================
NAME              : getStatus
DESCRIPTION       :
ASSUMPTIONS       :
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
================================================================================
*/
Boolean IPRoute::getStatus(String& s) const
{
  // This property, inherited from CIM_ManagedSystemElement,
  // is not relevant.

  return false;
}

/*
================================================================================
NAME              : getDestinationAddress
DESCRIPTION       :
ASSUMPTIONS       :
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
================================================================================
*/
Boolean IPRoute::getDestinationAddress(String& s) const
{
  // not supported
  return false;
}

/*
================================================================================
NAME              : getDestinationMask
DESCRIPTION       :
ASSUMPTIONS       :
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
================================================================================
*/
Boolean IPRoute::getDestinationMask(String& s) const
{
  // not supported
  return false;
}

/*
================================================================================
NAME              : getNextHop
DESCRIPTION       :
ASSUMPTIONS       :
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
================================================================================
*/
Boolean IPRoute::getNextHop(String& s) const
{
  // not supported
  return false;
}

/*
================================================================================
NAME              : getIsStatic
DESCRIPTION       :
ASSUMPTIONS       :
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
================================================================================
*/
Boolean IPRoute::getIsStatic(Boolean& s) const
{
  // not supported
  return false;
}

/*
================================================================================
NAME              : getAddressType
DESCRIPTION       :
ASSUMPTIONS       :
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
================================================================================
*/
Boolean IPRoute::getAddressType(Uint16& i16) const
{
  // not supported
  return false;
}

/*
================================================================================
NAME              : RouteList Constructor
DESCRIPTION       : Build the list of IP Routes
ASSUMPTIONS       : None
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
================================================================================
*/
RouteList::RouteList()
{
}

/*
================================================================================
NAME              : RouteList Destructor
DESCRIPTION       : None
ASSUMPTIONS       : None
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
================================================================================
*/
RouteList::~RouteList()
{
}


/*
================================================================================
NAME              : findRoute
DESCRIPTION       : Find the requested IP Route based on the destination
                  : address, destination mask, and address type.
ASSUMPTIONS       : None
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
================================================================================
*/
Boolean RouteList::findRoute(
    const String &destAddr,
    const String &destMask,
    const Uint16 &addrType,
    IPRoute &ipRInst) const
{
    // Always return route not found
    return false;
}


/*
================================================================================
NAME              : getRoute
DESCRIPTION       : Get an IP Route based on an index.
ASSUMPTIONS       : None
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
================================================================================
*/
IPRoute RouteList::getRoute(const int index) const
{
    // give a route (this should never get called since size will
    // always be zero).
    IPRoute i;
    return i;
}

/*
================================================================================
NAME              : size
DESCRIPTION       : Find the size of the Route List.
ASSUMPTIONS       : None
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
================================================================================
*/
int RouteList::size() const
{
    return 0;
}


/////////////////////////////////////////////////////////////////////////

NextHopIPRoute::NextHopIPRoute()
{
}

NextHopIPRoute::~NextHopIPRoute()
{
}

/*
================================================================================
NAME              : getCaption
DESCRIPTION       :
ASSUMPTIONS       :
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
================================================================================
*/
Boolean NextHopIPRoute::getCaption(String& s) const
{
    // not supported
    return false;
}

/*
================================================================================
NAME              : getDescription
DESCRIPTION       :
ASSUMPTIONS       :
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
================================================================================
*/
Boolean NextHopIPRoute::getDescription(String& s) const
{
    // not supported
    return false;
}

/*
================================================================================
NAME              : getInstallDate
DESCRIPTION       :
ASSUMPTIONS       :
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
================================================================================
*/
Boolean NextHopIPRoute::getInstallDate(CIMDateTime& d) const
{
    // Not supported. This property is inherited from
    // CIM_ManagedSystemElement, but has no useful meaning
    // for an IP Route.
    return false;
}

/*
================================================================================
NAME              : getName
DESCRIPTION       :
ASSUMPTIONS       :
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
================================================================================
*/
Boolean NextHopIPRoute::getName(String& s) const
{
    // not supported
    return false;
}

/*
================================================================================
NAME              : getStatus
DESCRIPTION       :
ASSUMPTIONS       :
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
================================================================================
*/
Boolean NextHopIPRoute::getStatus(String& s) const
{
    // This property, inherited from CIM_ManagedSystemElement,
    // is not relevant.
    return false;
}

/*
================================================================================
NAME              : getInstanceID
DESCRIPTION       :
ASSUMPTIONS       :
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
================================================================================
*/
Boolean NextHopIPRoute::getInstanceID(String& s) const
{
    // not supported
    return false;
}

/*
================================================================================
NAME              : getDestinationAddress
DESCRIPTION       :
ASSUMPTIONS       :
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
================================================================================
*/
Boolean NextHopIPRoute::getDestinationAddress(String& s) const
{
    // not supported
    return false;
}

/*
================================================================================
NAME              : getDestinationMask
DESCRIPTION       :
ASSUMPTIONS       :
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
================================================================================
*/
Boolean NextHopIPRoute::getDestinationMask(String& s) const
{
    // not supported
    return false;
}

/*
================================================================================
NAME              : getPrefixLength
DESCRIPTION       :
ASSUMPTIONS       :
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
================================================================================
*/
Boolean NextHopIPRoute::getPrefixLength(Uint8& pl) const
{
    // not supported
    return false;
}

/*
================================================================================
NAME              : getNextHop
DESCRIPTION       :
ASSUMPTIONS       :
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
================================================================================
*/
Boolean NextHopIPRoute::getNextHop(String& s) const
{
    // not supported
    return false;
}

/*
================================================================================
NAME              : getRouteDerivation
DESCRIPTION       :
ASSUMPTIONS       :
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
================================================================================
*/
Boolean NextHopIPRoute::getRouteDerivation(Uint16& s) const
{
    // not supported
    return false;
}

/*
================================================================================
NAME              : getOtherDerivation
DESCRIPTION       :
ASSUMPTIONS       :
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
================================================================================
*/
Boolean NextHopIPRoute::getOtherDerivation(String& s) const
{
    // not supported
    return false;
}

/*
================================================================================
NAME              : getRouteMetric
DESCRIPTION       :
ASSUMPTIONS       :
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
================================================================================
*/
Boolean NextHopIPRoute::getRouteMetric(Uint16& s) const
{
    // not supported
    return false;
}

/*
================================================================================
NAME              : getAdminDistance
DESCRIPTION       :
ASSUMPTIONS       :
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
================================================================================
*/
Boolean NextHopIPRoute::getAdminDistance(Uint16& s) const
{
    // not supported
    return false;
}

/*
================================================================================
NAME              : getTypeOfRoute
DESCRIPTION       :
ASSUMPTIONS       :
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
================================================================================
*/
Boolean NextHopIPRoute::getTypeOfRoute(Uint16& s) const
{
    // not supported
    return false;
}

/*
================================================================================
NAME              : getRouteType
DESCRIPTION       :
ASSUMPTIONS       :
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
================================================================================
*/
Boolean NextHopIPRoute::getRouteType(Uint16& s) const
{
    // not supported
    return false;
}

/*
================================================================================
NAME              : getIsStatic
DESCRIPTION       :
ASSUMPTIONS       :
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
================================================================================
*/
Boolean NextHopIPRoute::getIsStatic(Boolean& s) const
{
    // not supported
    return false;
}

/*
================================================================================
NAME              : getAddressType
DESCRIPTION       :
ASSUMPTIONS       :
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
================================================================================
*/
Boolean NextHopIPRoute::getAddressType(Uint16& i16) const
{
    // not supported
    return false;
}

/*
================================================================================
NAME              : isRouteLocal().
DESCRIPTION       : Determines if Route is Local.
ASSUMPTIONS       : None.
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
================================================================================
*/
Boolean NextHopIPRoute::isRouteLocal() const
{
    return false;
}

/*
================================================================================
NAME              : NextHopRouteList Constructor
DESCRIPTION       : Build the list of IP Routes
ASSUMPTIONS       : None
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
================================================================================
*/
NextHopRouteList::NextHopRouteList()
{
}

/*
================================================================================
NAME              : NextHopRouteList Destructor
DESCRIPTION       : None
ASSUMPTIONS       : None
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
================================================================================
*/
NextHopRouteList::~NextHopRouteList()
{
}

/*
================================================================================
NAME              : findRoute
DESCRIPTION       : Find the requested IP Route based on the destination
                  : address, destination mask, prefix length, and address type.
ASSUMPTIONS       : None
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
================================================================================
*/
Boolean NextHopRouteList::findRoute(
    const String &instanceID,
    NextHopIPRoute &ipRInst) const
{
    // Always return route not found
    return false;
}


/*
================================================================================
NAME              : getRoute
DESCRIPTION       : Get an IP Route based on an index.
ASSUMPTIONS       : None
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
================================================================================
*/
NextHopIPRoute NextHopRouteList::getRoute(const int index) const
{
    // give a route (this should never get called since size will
    // always be zero).
    NextHopIPRoute i;
    return i;
}

/*
================================================================================
NAME              : size
DESCRIPTION       : Find the size of the Route List.
ASSUMPTIONS       : None
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
================================================================================
*/
int NextHopRouteList::size() const
{
    return 0;
}


/////////////////////////////////////////////////////////////////////////

RSAp::RSAp()
{
}

RSAp::~RSAp()
{
}

/*
================================================================================
NAME              : getName.
DESCRIPTION       :
ASSUMPTIONS       :
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
================================================================================
*/
Boolean RSAp::getName(String& s) const
{
    // not supported
    return false;
}

/*
================================================================================
NAME              : getAccessInfo.
DESCRIPTION       :
ASSUMPTIONS       :
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
================================================================================
*/
Boolean RSAp::getAccessInfo(String& s) const
{
    // not supported
    return false;
}

/*
================================================================================
NAME              : getInfoFormat.
DESCRIPTION       :
ASSUMPTIONS       :
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
================================================================================
*/
Boolean RSAp::getInfoFormat(Uint16& ui) const
{
    // not supported
    return false;
}

/*
================================================================================
NAME              : getOtherInfoFmtDesc.
DESCRIPTION       :
ASSUMPTIONS       :
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
================================================================================
*/
Boolean RSAp::getOtherInfoFormatDescription(String& s) const
{
    // not supported
    return false;
}

/*
================================================================================
NAME              : RSApList Constructor.
DESCRIPTION       : Build the list of Remote Services Access Point.
ASSUMPTIONS       : None.
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
================================================================================
*/
RSApList::RSApList()
{
}

/*
================================================================================
NAME              : RSApList Destructor.
DESCRIPTION       : None.
ASSUMPTIONS       : None.
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
================================================================================
*/
RSApList::~RSApList()
{
}

/*
================================================================================
NAME              : findService.
DESCRIPTION       : Find the requested Remote Service based on the Name
                  : property.
ASSUMPTIONS       : None.
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
================================================================================
*/
Boolean RSApList::findService(
    const String &name,
    RSAp &rRSAp) const
{
    // Allways return Service not found.
    return false;
}


/*
================================================================================
NAME              : getService.
DESCRIPTION       : Get a Service based on an index.
ASSUMPTIONS       : None.
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
================================================================================
*/
RSAp RSApList::getService(const Uint16 index) const
{
    // give a service (this should never get called since size will
    // always be zero).
    RSAp r;
    return r;
}

/*
================================================================================
NAME              : size.
DESCRIPTION       : Find the size of the Remote Services Access Point List.
ASSUMPTIONS       : None.
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
================================================================================
*/
int RSApList::size() const
{
    return 0;
}

