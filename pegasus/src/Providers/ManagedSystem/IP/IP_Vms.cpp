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
#include <pthread.h>

PEGASUS_USING_STD;
PEGASUS_USING_PEGASUS;
/*
=============================================================================
Structer declration
==============================================================================
*/

typedef struct
{
    string destAddr;
    string subnetMask;
    string nextHop;
}ipInfo;

/*
==============================================================================
Function Prototype
==============================================================================
*/

static int _findNetworkProtocolType(int *findNetProtoType,const char *Str);
static int _getIpRoutingInfo(string lineStr,ipInfo *ipInfoStr);

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
    s = _address;
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
    String sn;

    if (getSystemName(sn))
    {
        s = "IP Protocol Endpoint for ";
        s.append(sn);
        s.append(" (");
        s.append(_address);
        s.append(")");
        return true;
    }
    else
    {
        return false;
    }
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
    s = _protocol;
    s.append("_");
    s.append(_simpleIfName);
    return true;
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
    char hostName[PEGASUS_MAXHOSTNAMELEN + 1];
    struct addrinfo *info, hints;

    if (gethostname(hostName, sizeof(hostName)) != 0)
    {
        s.assign("Unknown");
        return true;
    }
    hostName[sizeof(hostName)-1] = 0;

    // Now get the official hostname.  If this call fails then return
    // the value from gethostname().
    // Note: gethostbyname() is not reentrant and VMS does not
    // have gethostbyname_r() so use getaddrinfo().

    info = 0;
    memset (&hints, 0, sizeof(struct addrinfo));
    hints.ai_flags = AI_CANONNAME;
    hints.ai_family = AF_INET;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_socktype = SOCK_STREAM;

    int rc = 0;
    rc = System::getAddrInfo(hostName, 0, &hints, &info);
    if (rc) 
    {
        if (NULL != info)
        {
            freeaddrinfo(info);
            info = NULL;
        }
    }

    if ((!rc) && (info) && (info->ai_canonname))
    {
        // Note: if assign throws an exception, freeaddrinfo is not called.
        s.assign(info->ai_canonname);
    }
    else
    {
        s.assign(hostName);
    }
    return true;
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
    s = "<Protocol>_<InterfaceName>";
    return true;
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
    if (String::equal(_protocol,PROTOCOL_IPV4))
    {
        i16 = 2;  // IPv4
        return true;
    }
    else if (String::equal(_protocol,PROTOCOL_IPV6))
    {
        i16 = 3;  // IPv6
        return true;
    }
    else
    {
        return false;
    }
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
    // The caller must know to set the value to NULL (XML: no <VALUE> element)
    s = String::EMPTY;
    return true;
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
    s = _address;
    return true;
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
    // not supported
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
    s = _subnetMask;
    return true;
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
    /*
        From CIM v2.6.0 MOF for CIM_IPProtocolEndpoint.AddressType:
           ValueMap {"0", "1", "2"},
           Values {"Unknown", "IPv4", "IPv6"} ]
    */

    if (String::equal(_protocol,PROTOCOL_IPV4))
    {
        i16 = 1;  // IPv4
        return true;
    }
    else if (String::equal(_protocol,PROTOCOL_IPV6))
    {
        i16 = 2;  // IPv6
        return true;
    }
    else
    {
        return false;
    }
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
    /*
        From CIM v2.6.0 MOF for CIM_IPProtocolEndpoint.IPVersionSupport:
           ValueMap {"0", "1", "2"},
           Values {"Unknown", "IPv4 Only", "IPv6 Only"} ]
    */

    if (String::equal(_protocol,PROTOCOL_IPV4))
    {
        i16 = 1;  // IPv4 Only
        return true;
    }
    else if (String::equal(_protocol,PROTOCOL_IPV6))
    {
        i16 = 2;  // IPv6 Only
        return true;
    }
    else
    {
        return false;
    }
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
    //not supported
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
    /*
       From CIM v2.6.0 MOF for CIM_BindsToLANEndpoint.FrameType
          ValueMap {"0", "1", "2", "3", "4"},
          Values {"Unknown", "Ethernet", "802.2", "SNAP", "Raw802.3"} ]
    */

    i16 = 1;  // Ethernet
    return true;
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
    // Get rid of everything after the colon (":") if the name is of the
    // form "EIAX:Y", e.g. "EIA0:1".

    Uint32 pos = _simpleIfName.find(":");

    if (pos == PEG_NOT_FOUND)
    {
        return _simpleIfName;
    }
    else
    {
        String s = _simpleIfName;
        s.remove(pos,PEG_NOT_FOUND);
        return s;
    }
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
    // if this is a local ("lo") interface, then it doesn't bind to
    // an actual LAN Interface

    if (_simpleIfName.find("lo") == PEG_NOT_FOUND)
    {
        return true;
    }
    else
    {
        return false;
    }
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
    //not supported

}

/*
================================================================================
NAME              : set_address
DESCRIPTION       : Platform-specific routine to set the IP Address
ASSUMPTIONS       : None
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
================================================================================
*/
void IPInterface::set_address(const String& addr)
{
    _address = addr;
}

/*
================================================================================
NAME              : set_subnetMask
DESCRIPTION       : Platform-specific routine to set the Subnet Mask
ASSUMPTIONS       : None
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
================================================================================
*/
void IPInterface::set_subnetMask(const String& snm)
{
    _subnetMask = snm;
}

/*
================================================================================
NAME              : set_protocol
DESCRIPTION       : Platform-specific routine to set the IP Protocol
ASSUMPTIONS       : None
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
================================================================================
*/
void IPInterface::set_protocol(const String& proto)
{
    _protocol = proto;
}

/*
================================================================================
NAME              : set_simpleIfName
DESCRIPTION       : Platform-specific routine to set the Interface Name
ASSUMPTIONS       : None
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
================================================================================
*/
void IPInterface::set_simpleIfName(const String& name)
{
    _simpleIfName = name;
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
    IPP_DEBUG_OUT("InterfaceList::InterfaceList()");
    short channel;
    short sck_parm[2] = {IPPROTO_UDP, SOCK_DGRAM};
    int status;
    short iosb [4];      /* I/O status block - should check errors */
    char buffer[MYBUF], addr[64];
    struct ifconf ifc = {MYBUF, buffer};
    struct ifconf bad_ifc = {0, NULL};
    struct ifreq *ifr, *next;
    struct ip6ifreq *ifr6;

    $DESCRIPTOR(inet_dev, "TCPIP$DEVICE");
    struct
    {
        int cmd;
        void *ptr;
    } ioctl_desc = {NEW_SIOCGIFCONF, &ifc};
    struct
    {
        int cmd;
        void *ptr;
    } bad_ioctl_desc = {NEW_SIOCGIFCONF, &bad_ifc};
    struct
    {
        short len, param;
        void *ptr;
    }
    p6 = {sizeof(ioctl_desc), TCPIP$C_IOCTL, &ioctl_desc};
    struct
    {
        short len, param;
        void *ptr;
    }
    bad_p6 = {sizeof(ioctl_desc), TCPIP$C_IOCTL, &bad_ioctl_desc};

    status = sys$assign(&inet_dev, &channel, 0, 0);
    if (!(status & 1))
    {
        IPP_DEBUG_OUT("Error in assigning the channel, "
            "st: 0x" << hex << status);
        throw CIMOperationFailedException(
            "Error in assign the channel: "
            + String(strerror(errno)));

    }
    try
    {
        status = sys$qiow(
            EFN$C_ENF,          /* Event flag */
            channel,            /* Channel number */
            IO$_SETMODE,        /* I/O function */
            iosb,               /* I/O status block */
            0, 0, &sck_parm,    /* P1 socket creation param */
            0, 0, 0, 0, 0);
        if (status & 1)
        {
            status = iosb[0];
        }
        if (!(status & 1))
        {
            IPP_DEBUG_OUT("Error in setmode, st: 0x"
                << hex << status);
            throw CIMOperationFailedException(
                "Error in setmode: " + String(strerror(errno)));
        }

        /* Find out the size of the structure, to show how that's done */
        status = sys$qiow(
            EFN$C_ENF, channel, IO$_SENSEMODE, iosb,
            0, 0, 0, 0, 0, 0, 0, &bad_p6);
        if (status & 1)
        {
            status = iosb[0];
        }
        if (!(status & 1))
        {
            IPP_DEBUG_OUT("Error in bad sensemode, st: 0x"
                << hex << status);
            throw CIMOperationFailedException(
                "Error in bad sensemode: " + String(strerror(errno)));
        }

        IPP_DEBUG_OUT("Size of returned ifconf struct will be: "
            << bad_ifc.ifc_len);

        /* Now find out what other addresses are on the host */
        status = sys$qiow(
            EFN$C_ENF, channel, IO$_SENSEMODE, iosb,
            0, 0, 0, 0, 0, 0, 0, &p6);
        if (status & 1)
        {
            status = iosb[0];
        }
        if (!(status & 1))
        {
            IPP_DEBUG_OUT("Error in sensemode, st: 0x" << hex << status);
            throw CIMOperationFailedException(
                "Error in sensemode: " + String(strerror(errno)));
        }

        for (ifr = ifc.ifc_req ; ifr->ifr_name[0] ; ifr = next )
        {
            //IPInterface _ipif;
            struct sockaddr_in *sin =
                (struct sockaddr_in *)&ifr->ifr_addr;
            struct sockaddr_in sad;
            next = (struct ifreq *) (sin->sin_len + (char *)sin);
            if (sin->sin_family == AF_INET)
    {
                IPInterface _ipif;
                IPP_DEBUG_OUT(ifr->ifr_name
                    << "ProtocolType: IPv4   "
                    << "Addr: " << inet_ntoa(sin->sin_addr));
                _ipif.set_simpleIfName(ifr->ifr_name);
                _ipif.set_address(inet_ntoa(sin->sin_addr));
                _ipif.set_protocol(PROTOCOL_IPV4);

                /* Look up the network mask*/
                /*for this particular address */
                ioctl_desc.cmd = NEW_SIOCGIFNETMASK;
                ioctl_desc.ptr = ifr;
                ifr->ifr_addr.sa_len = sizeof(struct sockaddr_in);
                ifr->ifr_addr.sa_family = AF_INET;
                memcpy(&sad, sin, sizeof(struct sockaddr_in));
                status = sys$qiow(
                    EFN$C_ENF, channel, IO$_SENSEMODE, iosb,
                    0, 0, 0, 0, 0, 0, 0, &p6);
                if (status & 1)
                {
                    status = iosb[0];
                }
                if (!(status & 1))
                {
                    IPP_DEBUG_OUT("  no mask, err: 0x\n"
                        << hex << status);
                }
                else
                {
                    _ipif.set_subnetMask(inet_ntoa(sin->sin_addr));
                    IPP_DEBUG_OUT("   Msk: "
                        << inet_ntoa(sin->sin_addr));
                }

                /* Now look up the broadcast */
                /* address (same input as above) */
                ioctl_desc.cmd = NEW_SIOCGIFBRDADDR;
                memcpy(sin, &sad, sizeof(struct sockaddr_in));
                status = sys$qiow(
                    EFN$C_ENF, channel, IO$_SENSEMODE, iosb,
                    0, 0, 0, 0, 0, 0, 0, &p6);
                if (status & 1)
                {
                    status = iosb[0];
                }
                if (!(status & 1))
                {
                    IPP_DEBUG_OUT("  no broadcast, err: 0x" << hex << status);
                }
                else
                {
                    IPP_DEBUG_OUT("   Brd: "
                        << inet_ntoa(sin->sin_addr));
                }
                // Add another IP interface to the list
                _ifl.push_back(_ipif);
            }
            else if (sin->sin_family == AF_INET6)
            {
                IPInterface _ipif;
                struct sockaddr_in6 *sin6 =
                    (struct sockaddr_in6 *)&ifr->ifr_addr;
                struct sockaddr_in6 sad6;
                IPP_DEBUG_OUT(ifr->ifr_name
                    << "ProtocolType: IPv6   "
                    << "Addr: " << inet_ntoa(sin->sin_addr));
                _ipif.set_simpleIfName(ifr->ifr_name);
                inet_ntop(TCPIP$C_AF_INET6,
                    &sin6->sin6_addr,
                    addr,
                    sizeof(addr));
                _ipif.set_address(addr);
                _ipif.set_protocol(PROTOCOL_IPV6);

                IPP_DEBUG_OUT(ifr->ifr_name
                    << "ProtocolType: IPv6   "
                    << "Addr: " << addr);

                /* Look up the network mask for this address */
                ioctl_desc.cmd = NEW_SIOCGIFNETMASK;
                ioctl_desc.ptr = ifr;
                memcpy(&sad, sin, sizeof(struct sockaddr_in6));
                status = sys$qiow(
                    EFN$C_ENF, channel, IO$_SENSEMODE, iosb,
                    0, 0, 0, 0, 0, 0, 0, &p6);
                if (status & 1)
                {
                    status = iosb[0];
                }
                if (!(status & 1))
                {
                    IPP_DEBUG_OUT("  no mask, err: 0x"
                        << hex << status);
                }
                else
                {
                    inet_ntop(
                        TCPIP$C_AF_INET6,
                        &sin6->sin6_addr,
                        addr,
                        sizeof(addr));
                    _ipif.set_subnetMask(addr);
                    IPP_DEBUG_OUT("   Msk: " << addr);
                }

                /* Now look up the broadcast address */
                ioctl_desc.cmd = NEW_SIOCGIFBRDADDR;
                memcpy(sin, &sad, sizeof(struct sockaddr_in6));
                status = sys$qiow(
                    EFN$C_ENF, channel, IO$_SENSEMODE, iosb,
                    0, 0, 0, 0, 0, 0, 0, &p6);
                if (status & 1)
                {
                    status = iosb[0];
                }
                if (!(status & 1))
                {
                    IPP_DEBUG_OUT("  no broadcast, err: 0x"
                        << hex << status);
                }
                else
                {
                    inet_ntop(
                        TCPIP$C_AF_INET6,
                        &sin6->sin6_addr,
                        addr,
                        sizeof(addr));
                    IPP_DEBUG_OUT("   Brd: " << addr);
                }
                _ifl.push_back(_ipif);   // Add another IP interface to the list
            }
            else if (sin->sin_family == AF_LINK)
            {
                IPP_DEBUG_OUT("Ignoring AF_LINK interface");
            }
            else
            {
                IPP_DEBUG_OUT("Ignoring interface type: "
                    << sin->sin_family);
            }

        }
    }
    catch (...)
    {
        status = sys$dassgn(channel);
        if (!(status & 1))
        {
            IPP_DEBUG_OUT("Error in dassgn, st: 0x"
                          << hex << status);
            throw CIMOperationFailedException(
                "Inside handler, error in deassigning the channel: "
                + String(strerror(errno)));
        }
        throw;
    }
    status = sys$dassgn(channel);
    if (!(status & 1))
    {
        IPP_DEBUG_OUT("Error in dassgn, st: 0x"
                      << hex << status);
        throw CIMOperationFailedException(
            "Error in deassigning the channel: "
            + String(strerror(errno)));
    }
    IPP_DEBUG_OUT("InterfaceList::InterfaceList() -- done");
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
    // ifName has the format <Protocol>_<InterfaceName>,
    // for example "IPv4_l00".

    int i;

    for (i = 0; i < _ifl.size(); i++)
    {
        String s;

        if (_ifl[i].getName(s) && String::equal(s,ifName))
        {
            ipIfInst = _ifl[i];
            return true;
        }
    }

    // Interface not found
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
    return _ifl[index];
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
    return _ifl.size();
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
    s = _destAddr;
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
    s = "IP Route for Destination Address: ";
    s.append(_destAddr);
    s.append(" (");
    s.append(_protocolType);
    s.append(")");
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
    s = _destAddr;
    return true;
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
    s = _destAddr;
    return true;
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
    s = _destMask;
    return true;
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
    s = _nextHop;
    return true;
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
    /*
        From CIM v2.6.0 MOF for CIM_IPRoute.AddressType:
           ValueMap {"0", "1", "2"},
           Values {"Unknown", "IPv4", "IPv6"} ]
    */

    if (String::equal(_protocolType,PROTOCOL_IPV4))
    {
        i16 = 1;  // IPv4
        return true;
    }
    else if (String::equal(_protocolType,PROTOCOL_IPV6))
    {
        i16 = 2;  // IPv6
        return true;
    }
    else
    {
        return false;
    }
}
/*
================================================================================
NAME              : set_destAddress
DESCRIPTION       : Platform-specific routine to set the IP Destination Address
ASSUMPTIONS       : None
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
================================================================================
*/
void IPRoute::set_destAddr(const String& addr)
{
    _destAddr = addr;
}

/*
================================================================================
NAME              : set_destMask
DESCRIPTION       : Platform-specific routine to set the IP Destination Mask
ASSUMPTIONS       : None
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
================================================================================
*/
void IPRoute::set_destMask(const String& dm)
{
    _destMask = dm;
}

/*
================================================================================
NAME              : set_nextHop
DESCRIPTION       : Platform-specific routine to set the Next Hop Address
ASSUMPTIONS       : None
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
================================================================================
*/
void IPRoute::set_nextHop(const String& nh)
{
    _nextHop = nh;
}

/*
================================================================================
NAME              : set_protocolType
DESCRIPTION       : Platform-specific routine to set the Protocol Type
ASSUMPTIONS       : None
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
================================================================================
*/
void IPRoute::set_protocolType(const String& pt)
{
    _protocolType = pt;
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
    FILE* fp = NULL;
    char buffer[257];
    int lineNo = 0,foundNetProtoType=0,ret=0;;
    static pthread_mutex_t sIplock = PTHREAD_MUTEX_INITIALIZER;
    const char *networkType[] = {"TCPIP" , "MultiNet" , "TCPware"};

    IPP_DEBUG_OUT("RouteList::RouteList()");

    try
    {
        /* Lock the mutex sIplock */
        pthread_mutex_lock (&sIplock);

        /* TCPIP */
        ret = _findNetworkProtocolType(
            &foundNetProtoType,
            networkType[0]);
        if (ret != 0)
        {
            switch (ret)
            {
                case 1:
                    throw CIMOperationFailedException(
                        "Error in Opening the file WBEM_TMP:mydata.txt: "
                        + String(strerror(errno)));
                    break;
                default:
                    throw CIMOperationFailedException(
                        "Error in function findNetworkProtocolType  "
                        + String(strerror(errno)));
            }
        }

        /* MUltinet */
        if (foundNetProtoType != 1)
        {
            ret = _findNetworkProtocolType(
                &foundNetProtoType,
                networkType[1]);
            if (ret != 0)
            {
                switch (ret)
                {
                    case 1:
                        throw CIMOperationFailedException(
                            "Error in Opening the file WBEM_TMP:mydata.txt: "
                            + String(strerror(errno)));
                        break;

                    default:
                        throw CIMOperationFailedException(
                            "Error in function findNetworkProtocolType  "
                            + String(strerror(errno)));
                }
            }
        }

        /* Test for TCPware */
        if (foundNetProtoType != 1)
        {
            ret = _findNetworkProtocolType(
                &foundNetProtoType,
                networkType[2]);
            if (ret != 0)
            {
                switch (ret)
                {
                    case 1:
                        throw CIMOperationFailedException(
                            "Error in Opening the file WBEM_TMP:mydata.txt: "
                            + String(strerror(errno)));
                        break;

                    default:
                        throw CIMOperationFailedException(
                            "Error in function findNetworkProtocolType  "
                            + String(strerror(errno)));
                }
            }
        }

        if (foundNetProtoType != 1)
        {
            system("delete WBEM_TMP:mydata.txt;*");
            throw CIMOperationFailedException(
                "Error in finding the Network Protocol Type  "
                + String(strerror(errno)));
        }

        /* Open  Start Parsing the contains of txt file */
        fp = fopen ("WBEM_TMP:mydata.txt", "r");

        if (NULL == fp)
        {
            IPP_DEBUG_OUT("Unable to open the file WBEM_TMP:mydata.txt");
            throw CIMOperationFailedException(
                "Error in Opening the file WBEM_TMP:mydata.txt: "
                + String(strerror(errno)));
        }

        while (!feof(fp))
        {
            char *temp = NULL;
            if (NULL == fgets (buffer, 257, fp))
            {
                break;
            }
            else
            {
                string  line(buffer);

                if ((line.at(0) == 'R') || (line.at(0) == 'D') ||
                    (line.at(0) == ' ' &&
                    (line.at(1) == 'T' || line.at(1) == 'D' ||
                    line.at(1) == ' ' || line.at(1) == '-')) ||
                    (line.at(0) == '\n') ||(line.at(0) == 'M') ||
                    (line.at(0) == '-') || (line.at(0) == 'T'))
                {
                    continue;
                }
                else
                {
                    IPRoute _ipr;
                    ipInfo ipInfoStr = {"","",""};

                    /* process each line and Create the IP
                    Route info  List entries */
                    ret = _getIpRoutingInfo(line,&ipInfoStr);

                    if ( ret != 0)
                    {
                        throw CIMOperationFailedException(
                            "Error in function getIpRoutingInfo  "
                            + String(strerror(errno)));
                    }
                    /* set the destination address */
                    _ipr.set_destAddr(ipInfoStr.destAddr.data());

                    /* set the destination Mask */
                    _ipr.set_destMask(ipInfoStr.subnetMask.data());

                    /* set the nextHop */
                    _ipr.set_nextHop(ipInfoStr.nextHop.data());

                    /* set the  protocolType */
                    _ipr.set_protocolType(PROTOCOL_IPV4);

                    // Add another IP Route to the list
                    _iprl.push_back(_ipr);
                }
            }
        }
        fclose(fp);
        system("delete WBEM_TMP:mydata.txt;*");
        /* UnLock the mutex sIplock */
        pthread_mutex_unlock (&sIplock);
    }
    catch (...)
    {
        /* check and close file pointer, if it's still opened */
        if (NULL != fp)
        {
            fclose(fp);
            system("delete WBEM_TMP:mydata.txt;*");
        }
        /* UnLock the mutex sIplock */
        pthread_mutex_unlock (&sIplock);
        throw;
    }

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
    int i;

    for (i = 0; i < _iprl.size(); i++)
    {
        String sda, sdm;
        Uint16 sat;

        if (_iprl[i].getDestinationAddress(sda) &&
            String::equal(sda,destAddr) &&
            _iprl[i].getDestinationMask(sdm) &&
            String::equal(sdm,destMask) &&
            _iprl[i].getAddressType(sat) &&
            sat == addrType)
        {
            ipRInst = _iprl[i];
            return true;
        }
    }

    IPP_DEBUG_OUT("RouteList::findRoute(): NOT FOUND destAddr=" << destAddr <<
        ", destMask=" << destMask <<
        ", addrType=" << addrType);

    // IP Route not found

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
    return _iprl[index];
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
    return _iprl.size();
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
    // not supported
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
/*
================================================================================
NAME              : _findNetworkProtocolType.
DESCRIPTION       : Find the correct Network Protocol Type.
ASSUMPTIONS       : None.
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
================================================================================
*/

static int _findNetworkProtocolType(int *findNetProtoType,const char *Str)
{
    FILE* fp = NULL;
    char buffer[257];
    const string networkStr=Str;
    if (networkStr.compare("TCPIP")== 0)
    {
        system("pipe tcpip netstat -rn | "
            "copy sys$input WBEM_TMP:mydata.txt");
    }
    else if (networkStr.compare("MultiNet")== 0)
    {
        system(
            "pipe multinet show/route/NOSYMBOLIC_ADDRESSES | "
            "copy sys$input WBEM_TMP:mydata.txt");
    }
    else if (networkStr.compare("TCPware")== 0)
    {
        system("pipe @tcpware:netstat.com | "
            "copy sys$input WBEM_TMP:mydata.txt");
    }
    else
    {
        IPP_DEBUG_OUT("No match found for networkStr ");
        return 2;
    }

    /* open the file */
    fp = fopen ("WBEM_TMP:mydata.txt", "r");

    if (NULL == fp)
    {
        IPP_DEBUG_OUT("Unable to open the file WBEM_TMP:mydata.txt ");
        return 1;
    }
    while (!feof(fp))
    {
        if (NULL == fgets (buffer, 257, fp))
        {
            break;
        }
        else
        {
            string  line(buffer);
            string::size_type pos1 = line.find_first_not_of(" \t\r\n");
            string::size_type pos2;

            if (networkStr.compare("TCPware")== 0)
            {
                pos2 = line.find_first_of("(");
            }
            else
            {
                pos2 = line.find_first_of(" \t\r\n");
            }
            string substring = line.substr(
                pos1 == string::npos ? 0 : pos1,
                pos2 == string::npos ? line.length() - 1 : pos2 - pos1);
            if (substring.compare("Routing")== 0)
            {
                *findNetProtoType = 1;
                IPP_DEBUG_OUT("TCP/IP Configurartion");
            }
            else if (substring.compare("MultiNet") == 0)
            {
                *findNetProtoType = 1;
                IPP_DEBUG_OUT("Multinet Configuration");
            }
            else if (substring.compare("TCPware") == 0)
            {
                *findNetProtoType = 1;
                IPP_DEBUG_OUT("TCPware Configuration");
            }
            else
            {
                IPP_DEBUG_OUT("Error in finding the Networkprotocol type");
                fclose(fp);
                return 2;
            }
            break;
        }
    }
    if (*findNetProtoType == 1)
    {
        fclose(fp);
    }
    else
    {
        fclose(fp);
        system("delete WBEM_TMP:mydata.txt;*");
        *findNetProtoType = 0;
    }
    return 0;

}

/*
===============================================================================
NAME              : _getIpRoutingInfo.
DESCRIPTION       : Parse the input line and fill the IP Route info.
ASSUMPTIONS       : None.
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
===============================================================================
*/

static int _getIpRoutingInfo(string lineStr,ipInfo *ipInfoStr)
{
    /* trim the line */
    string line = "";
    line=lineStr;
    string::size_type pos1 = line.find_first_not_of(" \t\r\n");
    string::size_type pos2 = line.find_last_not_of(" \t\r\n");
    string substring = line.substr(
        pos1 == string::npos ? 0 : pos1,
        pos2 == string::npos ? line.length() - 1 : pos2 - pos1 + 1);
    int pos = 0;
    string temp;
    while (!substring.empty())
    {

        string::size_type pos3 =
            substring.find_first_not_of(" \t\r\n");
        string::size_type pos0 = substring.find_last_of(")");
        string::size_type pos4;
        if (pos0 == string::npos)
        {
            pos4 = substring.find_first_of(" \t\r\n");
        }
        else
        {
            pos4 = pos0+1;
        }
        if (pos4 == string::npos)
        {
            break;
        }
        else
        {
            temp=substring.substr(pos3,pos4);
        }
        /* find the position of mask string */
        string::size_type pos8=temp.find("/");
        switch (pos)
        {
            case 0:
                if ((temp.compare("default")== 0) ||
                    (temp.compare("all others (default)")== 0))
                {
                    ipInfoStr->destAddr="0.0.0.0";
                    ipInfoStr->subnetMask="0.0.0.0";
                }
                else if (pos8 != string::npos)
                {
                    int cnt=0;
                    char ch;
                    char *tempstr=NULL;
                    char *str1=NULL,*str2=NULL;
                    tempstr=strdup(temp.data());
                    str1=strtok(tempstr,"/");
                    str2=strtok(NULL,"/");
                    for (int j=0;j<strlen(str1);j++)
                    {
                        ch=str1[j];
                        switch (ch)
                        {
                            case '.':
                                cnt++;
                        }
                    }
                    int k = atoi(str2);
                    const char *subnetMaskParts[] =
                        {"0", "128", "192", "224", "240",
                         "248", "252", "254", "255"
                        };
                    switch (cnt)
                    {
                        case 0:
                            ipInfoStr->subnetMask = subnetMaskParts[k];
                            ipInfoStr->subnetMask.append(".0.0.0");
                            strcat(str1,".0.0.0");
                            ipInfoStr->destAddr=str1;
                            break;
                        case 1:
                            ipInfoStr->subnetMask="255.";
                            ipInfoStr->subnetMask.append(subnetMaskParts[k-8]);
                            ipInfoStr->subnetMask.append(".0.0");
                            strcat(str1,".0.0");
                            ipInfoStr->destAddr=str1;
                            break;

                        case 2:
                            ipInfoStr->subnetMask="255.255.";
                            ipInfoStr->subnetMask.append(
                                subnetMaskParts[k-16]);
                            ipInfoStr->subnetMask.append(".0");
                            strcat(str1,".0");
                            ipInfoStr->destAddr=str1;
                            break;
                        case 3:
                            ipInfoStr->destAddr=str1;
                            if (k < 8)
                            {
                                ipInfoStr->subnetMask = subnetMaskParts[k];
                                ipInfoStr->subnetMask.append(".0.0.0");
                            }
                            else if (k < 16)
                            {
                                ipInfoStr->subnetMask = "255.";
                                ipInfoStr->subnetMask.append(
                                    subnetMaskParts[k-8]);
                                ipInfoStr->subnetMask.append(".0.0");
                            }
                            else if (k < 24)
                            {
                                ipInfoStr->subnetMask = "255.255.";
                                ipInfoStr->subnetMask.append(
                                    subnetMaskParts[k-16]);
                                ipInfoStr->subnetMask.append(".0");
                            }
                            else
                            {
                                ipInfoStr->subnetMask="255.255.255.";
                                ipInfoStr->subnetMask.append(
                                    subnetMaskParts[k-24]);
                            }
                    }
                    free(tempstr);
                }
                else if (temp.compare("127.0.0.0") == 0)
                {
                    ipInfoStr->destAddr=temp.data();
                    ipInfoStr->subnetMask="255.0.0.0";
                }
                else
                {
                    ipInfoStr->destAddr=temp.data();
                    ipInfoStr->subnetMask="0.0.0.0";
                }
                IPP_DEBUG_OUT("DestAddr: " << ipInfoStr->destAddr.data()
                              << "DestMask: " << ipInfoStr->subnetMask.data());

                pos++;
                break;
            case 1:
                ipInfoStr->nextHop=temp.data();
                IPP_DEBUG_OUT("nextHop: " << ipInfoStr->nextHop.data());
                pos++;
                break;
            case 3:
                IPP_DEBUG_OUT("ProtocolType: IPv4");
                pos++;
                break;
            default:
                pos++;
        }
        string::size_type pos5 = substring.find_last_of("0123456789UIL");
        substring=substring.substr(pos4,pos5-pos4+1);
        string::size_type pos6 = substring.find_first_not_of(" \t\r\n");
        string::size_type pos7 = substring.find_last_of("0123456789UIL");
        substring=substring.substr(pos6,pos7-pos6+1);
    }
    return 0;

}
