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
#include <errno.h>
#include <sys/utsname.h>

PEGASUS_USING_STD;
PEGASUS_USING_PEGASUS;

String IPInterface::_hostname = String::EMPTY;  // Allocate this static

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
      s = "IP Protocol Endpoint for " + sn + " (" + _address + ")";
      return true;
  }
  else
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
  s = _protocol + "_" + _simpleIfName;
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
    struct hostent *he;
    char hn[PEGASUS_MAXHOSTNAMELEN + 1];

    // fill in hn with what this system thinks is its name
    gethostname(hn, sizeof(hn));
    hn[sizeof(hn)-1] = 0;

    // find out what the nameservices think is its full name
    // but if that failed, return what gethostname said
    if ((he = gethostbyname(hn)) != 0)
    {
        s = String(he->h_name);
    }
    else
    {
        s = String(hn);
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
    /*
       From the MOF for CIM_ProtocolEndpoint.ProtocolType:
       ValueMap {"0", "1", "2", "3", "4", "5", "6", "7", "8", "9",
           "10", "11", "12", "13", "14", "15", "16", "17", "18",
           "19", "20", "21", "22", "23", "24", "25", "26", "27"},
       Values {"Unknown", "Other", "IPv4", "IPv6", "IPX",
           "AppleTalk", "DECnet", "SNA", "CONP", "CLNP",
           "VINES", "XNS", "ATM", "Frame Relay",
           "Ethernet", "TokenRing", "FDDI", "Infiniband",
           "Fibre Channel", "ISDN BRI Endpoint",
           "ISDN B Channel Endpoint", "ISDN D Channel Endpoint",
           // 22
           "IPv4/v6", "BGP", "OSPF", "MPLS", "UDP", "TCP"},
    */

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
    else return false;

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
    s = _address;
    return true;
}

/*
================================================================================
NAME              : getIPv4Address.
DESCRIPTION       : The IPv4 address that this ProtocolEndpoint represents.
ASSUMPTIONS       :
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
================================================================================
*/
Boolean IPInterface::getIPv4Address(String& s) const
{
    if (String::equal(_protocol,PROTOCOL_IPV4))
    {
        s = _address;
        return true;
    }
    else
    {
        return false;
    }
}

/*
================================================================================
NAME              : getIPv6Address.
DESCRIPTION       : The IPv6 address that this ProtocolEndpoint represents.
ASSUMPTIONS       :
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
================================================================================
*/
Boolean IPInterface::getIPv6Address(String& s) const
{
    if (String::equal(_protocol,PROTOCOL_IPV6))
    {
        s = _address;
        return true;
    }
    else
    {
        return false;
    }
}

/*
================================================================================
NAME              : getPrefixLength.
DESCRIPTION       :
ASSUMPTIONS       :
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
================================================================================
*/
Boolean IPInterface::getPrefixLength(Uint8& i8) const
{
    if (String::equal(_protocol,PROTOCOL_IPV6))
    {
        i8 = _prefixLength;
        return true;
    }
    else
    {
        return false;
    }
}

/*
================================================================================
NAME              : getSubnetMask.
DESCRIPTION       :
ASSUMPTIONS       :
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
================================================================================
*/
Boolean IPInterface::getSubnetMask(String& s) const
{
    if (String::equal(_protocol,PROTOCOL_IPV4))
    {
        s = _subnetMask;
        return true;
    }
    else
    {
        return false;
    }
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
    else return false;

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
    else return false;

}

/*
================================================================================
NAME              : getProtocolIFType
DESCRIPTION       :
ASSUMPTIONS       :
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
================================================================================
*/
Boolean IPInterface::getProtocolIFType(Uint16& i16) const
{
    // From CIM v2.6.0 MOF for CIM_IPProtocolEndpoint.ProtocolIFType:
    //    ValueMap { "1", "225..4095", "4096", "4097", "4098",
    //       "4301..32767", "32768.." },
    //    Values { "Other", "IANA Reserved", "IPv4", "IPv6", "IPv4/v6",
    //       "DMTF Reserved", "Vendor Reserved" }]

    if (String::equal(_protocol,PROTOCOL_IPV4))
    {
        i16 = 4096;  // IPv4 Only.
        return true;
    }
    else
    {
        if (String::equal(_protocol,PROTOCOL_IPV6))
        {
            i16 = 4097;  // IPv6 Only.
            return true;
        }
        else
        {
            if (String::equal(_protocol,PROTOCOL_IPV4_V6))
            {
                i16 = 4098;  // IPv4/v6.
                return true;
            }
            else
            {
                return false;
            }
        }
    }
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
    // form "lanX:Y", e.g. "lan0:1".

    Uint32 pos = _simpleIfName.find(":");

    if (pos == PEG_NOT_FOUND)
        return _simpleIfName;
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
        return true;
    else
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
NAME              : set_prefixLength.
DESCRIPTION       : Platform-specific routine to set IPv6 PrefixLength.
ASSUMPTIONS       :
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
================================================================================
*/
void IPInterface::set_prefixLength(const Uint8& pl)
{
    _prefixLength = pl;
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
    int fd,                       // file descriptors
        i, j,                     // general purpose indicies
        numif,                    // number of interfaces
        numip;                    // number of IP addresses
    struct ifconf ifconf;         // interface configuration for IPv4
    unsigned int len;             // length of get_mib_info() buffer
    struct nmparms parms;         // get_mib_info() arguments
    mib_ipAdEnt * addr_buf;       // IP Address Buffer
    struct in_addr t;             // temporary variable for extracting
                                  //     IP Address Buffer contents
    struct sockaddr_in *sin;      // temporary variable for extracting
                                  //     interface name for IPv4
#ifdef PEGASUS_ENABLE_IPV6
    struct if_laddrconf ifconf6;  // Interface configuration for IPv6.
    mib_ipv6AddrEnt * addr6_buf;  // IPv6 Address Buffer.
    struct in6_addr t6;           // Temporary variable for extracting
                                  // IPv6 Address Buffer contents.
    struct sockaddr_in6 *sin6;    // Temporary variable for extracting
                                  // interface name for IPv6.
    int numif6, numip6;           // equivalent for IPv6.
    Boolean _ipv6Present=false;   // Flag to check if ipv6 is available.
#endif

#ifdef IPPROVIDER_DEBUG
    cout << "InterfaceList::InterfaceList()" << endl;
#endif

    // Load the IPv4 interface name structures.

    if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        throw CIMOperationFailedException(
            "Error in opening socket: " + String(strerror(errno)));
    }

    if (ioctl(fd, SIOCGIFNUM, &numif) < 0)
    {
        throw CIMOperationFailedException(
            "Error in ioctl() request SIOCGIFNUM: " + String(strerror(errno)));
    }

    // HP-UX < B.11.31 return the number of interfaces plus 1. In this
    // case numif must be corrected

    struct utsname unameInfo;
    if ((uname(&unameInfo) < 0) && (errno != EOVERFLOW))
    {
        throw CIMOperationFailedException(
            "Error in uname: " + String(strerror(errno)));
    }

    if (strcmp(unameInfo.release,"B.11.31") < 0)
    {
        numif--;
    }

    ifconf.ifc_len = numif * sizeof (struct ifreq);
    ifconf.ifc_req = (struct ifreq *) calloc(numif, sizeof (struct ifreq));

    if (ioctl (fd, SIOCGIFCONF, &ifconf) < 0)
    {
        free (ifconf.ifc_req);
        throw CIMOperationFailedException(
            "Error in ioctl() request SIOCGIFCONF: " + String(strerror(errno)));
    }

    close(fd);

#ifdef PEGASUS_ENABLE_IPV6
    // Load the IPv6 interface name structures.

    if ((fd = socket(AF_INET6, SOCK_DGRAM, 0)) >= 0)
    {
        if (ioctl(fd, SIOCGLIFNUM, &numif6) < 0)
        {
            free (ifconf.ifc_req);
            throw CIMOperationFailedException(
                "Error in ioctl() request SIOCGIFNUM: "
                + String(strerror(errno)));
        }

        ifconf6.iflc_len = numif6 * sizeof (struct if_laddrreq);
        ifconf6.iflc_req = (struct if_laddrreq *) calloc(
            numif6, sizeof (struct if_laddrreq));

        if (ioctl (fd, SIOCGLIFCONF, &ifconf6) < 0)
        {
            free (ifconf.ifc_req);
            free (ifconf6.iflc_req);
            throw CIMOperationFailedException(
                "Error in ioctl() request SIOCGIFCONF: "
                + String(strerror(errno)));
        }
    }
    else
    {
        if (errno != EPROTONOSUPPORT)
        {
            free (ifconf.ifc_req);
            throw CIMOperationFailedException(
                "Error in opening socket: " + String(strerror(errno)));
        }
    }

    close(fd);

    if ((fd = open_mib("/dev/ip6", O_RDONLY, 0, 0)) == -1)
    {
        // Failed to open the /dev/ip6 device file. Therefore the IPv6 product
        // is not installed on the system. An application should use the
        // existing IPv4 code.
        free (ifconf6.iflc_req);
#endif

        if ((fd = open_mib("/dev/ip", O_RDONLY, 0, 0)) < 0)
        {
            free (ifconf.ifc_req);
            throw CIMOperationFailedException(
                "Can't open /dev/ip: " + String(strerror(errno)));
        }

#ifdef PEGASUS_ENABLE_IPV6
    }
    else
    {
        // The /dev/ip6 device file does exists, so the IPv6 product
        // is probably installed. IPv6 APIs can handle both IPv4 and
        // IPv6 traffic.
        _ipv6Present = true;
    }
#endif

    // IPv4

    parms.objid = ID_ipAddrNumEnt;
    parms.buffer = (char *) &numip;
    len = sizeof(numip);
    parms.len = &len;

    if (get_mib_info (fd, &parms) < 0)
    {
        free(ifconf.ifc_req);
#ifdef PEGASUS_ENABLE_IPV6
        free(ifconf6.iflc_req);
#endif
        throw CIMOperationFailedException(
            "Can't get ID_ipAddrNumEnt from get_mib_info(): " +
                String(strerror(errno)));
    }

    addr_buf = (mib_ipAdEnt *)malloc(numip*sizeof(mib_ipAdEnt));

    if (addr_buf == 0)
    {
        free (ifconf.ifc_req);
#ifdef PEGASUS_ENABLE_IPV6
        free (ifconf6.iflc_req);
#endif
        free (addr_buf);
        throw CIMOperationFailedException(
            "Error in allocating space for the kernel interface table: " +
                String(strerror(errno)));
    }

    parms.objid = ID_ipAddrTable;
    parms.buffer = (char *) addr_buf;
    len = numip * sizeof(mib_ipAdEnt);
    parms.len = &len;

    if (get_mib_info (fd, &parms) < 0)
    {
        free(ifconf.ifc_req);
#ifdef PEGASUS_ENABLE_IPV6
        free (ifconf6.iflc_req);
#endif
        free(addr_buf);
        throw CIMOperationFailedException(
            "Can't get ID_ipAddrTable from get_mib_info(): " +
                String(strerror(errno)));
    }

    // Create the IPv4 interface list entries

    for (i=0; i < numip ; i++)
    {
        IPInterface _ipif;

        t.s_addr = addr_buf[i].Addr;
        _ipif.set_address(inet_ntoa(t));

        _ipif.set_protocol(PROTOCOL_IPV4);

        for (j = 0; j < numif; j++)
        {
            sin = reinterpret_cast<struct sockaddr_in*>(
                &ifconf.ifc_req[j].ifr_addr);
            if (sin->sin_addr.s_addr == t.s_addr)
            {
                _ipif.set_simpleIfName(ifconf.ifc_req[j].ifr_name);
            }
        } /* for */

        t.s_addr = addr_buf[i].NetMask;
        _ipif.set_subnetMask(inet_ntoa(t));

        _ifl.push_back(_ipif);   // Add another IP interface to the list

    } /* for */

    // Freeing up structures not needed anymore.
    free(ifconf.ifc_req);
    free(addr_buf);

#ifdef PEGASUS_ENABLE_IPV6
    if (_ipv6Present)
    {
        // IPv6.

        parms.objid = ID_ipv6AddrTableNumEnt;
        parms.buffer = (char *) &numip6;
        len = sizeof(numip6);
        parms.len = &len;

        if (get_mib_info (fd, &parms) < 0)
        {
            free(ifconf6.iflc_req);
            throw CIMOperationFailedException(
                "Can't get ID_ipv6AddrTableNumEnt from get_mib_info(): " +
                    String(strerror(errno)));
        }

        addr6_buf = (mib_ipv6AddrEnt *)malloc(numip6*sizeof(mib_ipv6AddrEnt));

        if (addr6_buf == 0)
        {
            free (ifconf6.iflc_req);
            free (addr6_buf);
            throw CIMOperationFailedException(
                "Error in allocating space for the kernel " +
                String("IPv6 interface table: ") +
                String(strerror(errno)));
        }

        parms.objid = ID_ipv6AddrTable;
        parms.buffer = (char *) addr6_buf;
        len = numip6 * sizeof(mib_ipv6AddrEnt);
        parms.len = &len;

        if (get_mib_info (fd, &parms) < 0)
        {
            free(ifconf6.iflc_req);
            free(addr6_buf);
            throw CIMOperationFailedException(
                "Can't get ID_ipv6AddrTable from get_mib_info(): " +
                    String(strerror(errno)));
        }

        // Create the IPv6 interface list entries.

        for (i=0; i < numip6 ; i++)
        {
            IPInterface _ipif;

            t6 = addr6_buf[i].ipv6AddrAddress;
            char _dest[INET6_ADDRSTRLEN];
            if (inet_ntop(AF_INET6, &t6, _dest, INET6_ADDRSTRLEN) == 0)
            {
                free(ifconf6.iflc_req);
                free(addr6_buf);
                throw CIMOperationFailedException(
                    "Can't convert number to string in IPv6 format. " +
                        String(strerror(errno)));
            }

            _ipif.set_address(_dest);
            _ipif.set_protocol(PROTOCOL_IPV6);
            _ipif.set_prefixLength((Uint8)addr6_buf[i].ipv6AddrPfxLength);

            for (j = 0; j < numif6; j++)
            {
                sin6 = reinterpret_cast<struct sockaddr_in6*>(
                    &ifconf6.iflc_req[j].iflr_addr);

                char _addr1[INET6_ADDRSTRLEN], _addr2[INET6_ADDRSTRLEN];
                if (inet_ntop(
                    AF_INET6,
                    t6.s6_addr,
                    _addr1,
                    INET6_ADDRSTRLEN) == 0 ||
                    inet_ntop(
                    AF_INET6,
                    sin6->sin6_addr.s6_addr,
                    _addr2,
                    INET6_ADDRSTRLEN) == 0)
                {
                    free(ifconf6.iflc_req);
                    free(addr6_buf);
                    throw CIMOperationFailedException(
                        "Can't convert number to string in IPv6 format. " +
                            String(strerror(errno)));
                }

                if ( strcmp(_addr1, _addr2) == 0 )
                {
                    _ipif.set_simpleIfName(ifconf6.iflc_req[j].iflr_name);
                }
            }

            _ifl.push_back(_ipif);   // Add another IP interface to the list.

        }

        free(ifconf6.iflc_req);
        free(addr6_buf);

    }
#endif

    close_mib(fd);

#ifdef IPPROVIDER_DEBUG
    cout << "InterfaceList::InterfaceList() -- done" << endl;
#endif
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
    // for example "IPv4_lan0".

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
  s = "IP Route for Destination Address: " + _destAddr +
      " (" + _protocolType + ")";
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
  // Don't know how to get this property.
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
    else return false;
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
RouteList::RouteList()
{
    int fd,                      // file descriptor
        i,                       // general purpose indicies
        count;                   // number of raw IP Routes
    struct nmparms parms;        // get_mib_info() arguments
    mib_ipRouteEnt * route_buf;  // IP Route Buffer
    unsigned int len;            // length of get_mib_info() buffer
    struct in_addr t;            // temporary variable for extracting
                               //   IP route buffer contents

#ifdef IPPROVIDER_DEBUG
    cout << "RouteList::RouteList()" << endl;
#endif

    // Load the interface name structures.

    if ((fd = open_mib("/dev/ip", O_RDONLY, 0, 0)) < 0)
    {
        throw CIMOperationFailedException(
            "Can't open /dev/ip: " + String(strerror(errno)));
    }

    parms.objid = ID_ipRouteNumEnt;
    parms.buffer = (char *) &count;
    len = sizeof(count);
    parms.len = (unsigned int *) &len;

    if (get_mib_info (fd, &parms) < 0)
    {
        throw CIMOperationFailedException(
            "Can't get ID_ipRouteNumEnt from get_mib_info(): " +
                String(strerror(errno)));
    }

    route_buf = (mib_ipRouteEnt *)malloc(count*sizeof(mib_ipRouteEnt));

    if (route_buf == 0)
    {
        free(route_buf);
        throw CIMOperationFailedException(
            "Error in allocating space for the kernel interface table: " +
                String(strerror(errno)));
    }

    parms.objid = ID_ipRouteTable;
    parms.buffer = (char *) route_buf;
    len = count * sizeof(mib_ipRouteEnt);
    parms.len = &len;

    if (get_mib_info (fd, &parms) < 0)
    {
        free(route_buf);
        throw CIMOperationFailedException(
            "Can't get ID_ipRouteTable from get_mib_info(): " +
                String(strerror(errno)));
    }

    // Create the IP Route List entries

    for (i=0; i < count ; i++)
    {
        IPRoute _ipr;

        // check to see that this is a valid type to represent
        if (route_buf[i].Type == 3 || route_buf[i].Type == 4)
        {
            t.s_addr = route_buf[i].Dest;
            _ipr.set_destAddr(inet_ntoa(t));

            t.s_addr = route_buf[i].Mask;
            _ipr.set_destMask(inet_ntoa(t));

            t.s_addr = route_buf[i].NextHop;
            _ipr.set_nextHop(inet_ntoa(t));

            // ATTN-LEW-2002-09-13: Enhance this to deal with IPv6 too.
            _ipr.set_protocolType(PROTOCOL_IPV4);

            _iprl.push_back(_ipr);   // Add another IP Route to the list
        }

    } /* for */

    close_mib(fd);
    free(route_buf);

#ifdef IPPROVIDER_DEBUG
    cout << "RouteList::RouteList() -- done" << endl;
#endif

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

#ifdef IPPROVIDER_DEBUG
    cout << "RouteList::findRoute(): NOT FOUND destAddr=" << destAddr <<
        ", destMask=" << destMask <<
        ", addrType=" << addrType << endl;
#endif

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
NAME              : getCaption.
DESCRIPTION       :
ASSUMPTIONS       :
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
================================================================================
*/
Boolean NextHopIPRoute::getCaption(String& s) const
{
  s = _destAddr;
  return true;
}

/*
================================================================================
NAME              : getDescription.
DESCRIPTION       :
ASSUMPTIONS       :
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
================================================================================
*/
Boolean NextHopIPRoute::getDescription(String& s) const
{
  s = "Next Hop IP Route for Destination Address: " +
      _destAddr +
      " (" +
      _protocolType +
      ")";
  return true;
}

/*
================================================================================
NAME              : getInstallDate.
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
NAME              : getName.
DESCRIPTION       :
ASSUMPTIONS       :
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
================================================================================
*/
Boolean NextHopIPRoute::getName(String& s) const
{
  s = _destAddr;
  return true;
}

/*
================================================================================
NAME              : getStatus.
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
NAME              : getInstanceID.
DESCRIPTION       :
ASSUMPTIONS       :
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
================================================================================
*/
Boolean NextHopIPRoute::getInstanceID(String& s) const
{
    s = _destAddr;
    return true;
}

/*
================================================================================
NAME              : getDestinationAddress.
DESCRIPTION       :
ASSUMPTIONS       :
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
================================================================================
*/
Boolean NextHopIPRoute::getDestinationAddress(String& s) const
{
  s = _destAddr;
  return true;
}

/*
================================================================================
NAME              : getDestinationMask.
DESCRIPTION       :
ASSUMPTIONS       :
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
================================================================================
*/
Boolean NextHopIPRoute::getDestinationMask(String& s) const
{
    if (String::equal(_protocolType,PROTOCOL_IPV4))
    {
        s = _destMask;
        return true;
    }
    else
    {
        return false;
    }
}

/*
================================================================================
NAME              : getPrefixLength.
DESCRIPTION       :
ASSUMPTIONS       :
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
================================================================================
*/
Boolean NextHopIPRoute::getPrefixLength(Uint8& pl) const
{
    if (String::equal(_protocolType,PROTOCOL_IPV6))
    {
        pl = _prefixLength;
        return true;
    }
    else
    {
        return false;
    }
}

/*
================================================================================
NAME              : getNextHop.
DESCRIPTION       :
ASSUMPTIONS       :
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
================================================================================
*/
Boolean NextHopIPRoute::getNextHop(String& s) const
{
  s = _nextHop;
  return true;
}

/*
================================================================================
NAME              : getRouteDerivation.
DESCRIPTION       :
ASSUMPTIONS       :
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
================================================================================
*/
Boolean NextHopIPRoute::getRouteDerivation(Uint16& s) const
{
  // Don't know how to get this property.
  return false;
}

/*
================================================================================
NAME              : getOtherDerivation.
DESCRIPTION       :
ASSUMPTIONS       :
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
================================================================================
*/
Boolean NextHopIPRoute::getOtherDerivation(String& s) const
{
  // Don't know how to get this property.
  return false;
}

/*
================================================================================
NAME              : getRouteMetric.
DESCRIPTION       :
ASSUMPTIONS       :
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
================================================================================
*/
Boolean NextHopIPRoute::getRouteMetric(Uint16& s) const
{
  // Don't know how to get this property.
  return false;
}

/*
================================================================================
NAME              : getAdminDistance.
DESCRIPTION       :
ASSUMPTIONS       :
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
================================================================================
*/
Boolean NextHopIPRoute::getAdminDistance(Uint16& s) const
{
  // Don't know how to get this property.
  return false;
}

/*
================================================================================
NAME              : getTypeOfRoute.
DESCRIPTION       :
ASSUMPTIONS       :
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
================================================================================
*/
Boolean NextHopIPRoute::getTypeOfRoute(Uint16& s) const
{
  // Don't know how to get this property.
  return false;
}

/*
================================================================================
NAME              : getIsStatic.
DESCRIPTION       :
ASSUMPTIONS       :
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
================================================================================
*/
Boolean NextHopIPRoute::getIsStatic(Boolean& s) const
{
  // Don't know how to get this property.
  return false;
}

/*
================================================================================
NAME              : getAddressType.
DESCRIPTION       :
ASSUMPTIONS       :
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
================================================================================
*/
Boolean NextHopIPRoute::getAddressType(Uint16& i16) const
{
    // From CIM v2.6.0 MOF for CIM_IPRoute.AddressType:
    //       ValueMap {"0", "1", "2"},
    //       Values {"Unknown", "IPv4", "IPv6"} ]

    if (String::equal(_protocolType,PROTOCOL_IPV4))
    {
        i16 = 1;  // IPv4
        return true;
    }
    else
    {
        if (String::equal(_protocolType,PROTOCOL_IPV6))
        {
            i16 = 2;  // IPv6
            return true;
        }
        else
        {
            return false;
        }
    }
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

#ifdef PEGASUS_ENABLE_IPV6
    if ( _routeType == IPV6ROUTETYPE_LOCAL)
#else
    if ( _routeType != NMREMOTE)
#endif
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
NAME              : set_destAddress.
DESCRIPTION       : Platform-specific routine to set the IP Destination Address.
ASSUMPTIONS       : None.
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
================================================================================
*/
void NextHopIPRoute::set_destAddr(const String& addr)
{
    _destAddr = addr;
}

/*
================================================================================
NAME              : set_destMask.
DESCRIPTION       : Platform-specific routine to set the IPv4 Destination Mask.
ASSUMPTIONS       : _protocolType is IPv4.
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
================================================================================
*/
void NextHopIPRoute::set_destMask(const String& dm)
{
    _destMask = dm;
}

/*
================================================================================
NAME              : set_prefixLength.
DESCRIPTION       : Platform-specific routine to set the (IPv6) prefix length.
ASSUMPTIONS       : _protocolType is IPv6.
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
================================================================================
*/
void NextHopIPRoute::set_prefixLength(const Uint8& pl)
{
    _prefixLength = pl;
}

/*
================================================================================
NAME              : set_nextHop.
DESCRIPTION       : Platform-specific routine to set the Next Hop Address.
ASSUMPTIONS       : None.
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
================================================================================
*/
void NextHopIPRoute::set_nextHop(const String& nh)
{
    _nextHop = nh;
}

/*
================================================================================
NAME              : set_routeType.
DESCRIPTION       : Platform-specific routine to set the route type.
ASSUMPTIONS       : None.
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
================================================================================
*/
void NextHopIPRoute::set_routeType(const Uint16& rt)
{
    _routeType = rt;
}

/*
================================================================================
NAME              : set_protocolType.
DESCRIPTION       : Platform-specific routine to set the Protocol Type.
ASSUMPTIONS       : None.
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
================================================================================
*/
void NextHopIPRoute::set_protocolType(const String& pt)
{
    _protocolType = pt;
}

/*
================================================================================
NAME              : NextHopRouteList Constructor.
DESCRIPTION       : Build the list of IP Routes.
ASSUMPTIONS       : None.
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
================================================================================
*/
NextHopRouteList::NextHopRouteList()
{
    int fd,                          // File descriptor.
        i,                           // General purpose indicies.
        count;                       // Number of raw IP Routes.
    struct nmparms parms;            // Get_mib_info() arguments.
    mib_ipRouteEnt * route_buf;      // IPv4 Route Buffer.
    unsigned int len;                // Length of get_mib_info() buffer.
    struct in_addr t;                // Temporary variable for extracting
                                     // IPv4 route buffer contents.

#ifdef PEGASUS_ENABLE_IPV6
    mib_ipv6RouteEnt * route6_buf;   // IPv6 Route Buffer.
    struct in6_addr t6;              // Temporary variable for extracting
                                     // IPv6 route buffer contents.
    Boolean _ipv6Present = false;    // Flag to check if ipv6 is available.
#endif

#ifdef IPPROVIDER_DEBUG
    cout << "NextHopRouteList::NextHopRouteList()" << endl;
#endif

    // Load the interface name structures.

#ifdef PEGASUS_ENABLE_IPV6
    if ((fd = open_mib("/dev/ip6", O_RDONLY, 0, 0)) == -1)
    {
        // Failed to open the /dev/ip6 device file. Therefore the IPv6 product
        // is not installed on the system. An application should use the
        // existing IPv4 code.
#endif

        if ((fd = open_mib("/dev/ip", O_RDONLY, 0, 0)) < 0)
        {
            throw CIMOperationFailedException(
                "Can't open /dev/ip: " + String(strerror(errno)));
        }

#ifdef PEGASUS_ENABLE_IPV6
    }
    else
    {
        // The /dev/ip6 device file does exists, so the IPv6 product
        // is probably installed. IPv6 APIs can handle both IPv4 and
        // IPv6 traffic.
        _ipv6Present = true;
    }
#endif

    // IPv4.

    parms.objid = ID_ipRouteNumEnt;
    parms.buffer = (char *) &count;
    len = sizeof(count);
    parms.len = (unsigned int *) &len;

    if (get_mib_info (fd, &parms) < 0)
    {
        throw CIMOperationFailedException(
            "Can't get ID_ipRouteNumEnt from get_mib_info(): " +
            String(strerror(errno)));
    }

    route_buf = (mib_ipRouteEnt *)malloc(count*sizeof(mib_ipRouteEnt));

    if (route_buf == 0)
    {
        free(route_buf);
        throw CIMOperationFailedException(
            "Error in allocating space for the kernel interface table: " +
            String(strerror(errno)));
    }

    parms.objid = ID_ipRouteTable;
    parms.buffer = (char *) route_buf;
    len = count * sizeof(mib_ipRouteEnt);
    parms.len = &len;

    if (get_mib_info (fd, &parms) < 0)
    {
        free(route_buf);
        throw CIMOperationFailedException(
            "Can't get ID_ipRouteTable from get_mib_info(): " +
            String(strerror(errno)));
    }

    // Create the IPv4 Route List entries.

    for (i=0; i < count ; i++)
    {
        NextHopIPRoute _ipr;

        // check to see that this is a valid type to represent.
        if (route_buf[i].Type == 3 || route_buf[i].Type == 4)
        {
            _ipr.set_protocolType(PROTOCOL_IPV4);

            t.s_addr = route_buf[i].Dest;
            _ipr.set_destAddr(inet_ntoa(t));

            t.s_addr = route_buf[i].Mask;
            _ipr.set_destMask(inet_ntoa(t));

            t.s_addr = route_buf[i].NextHop;
            _ipr.set_nextHop(inet_ntoa(t));

            _ipr.set_routeType(
                (Uint16)route_buf[i].Type);

            _nhiprl.push_back(_ipr);   // Add another IP Route to the list.
        }

    }

    free(route_buf);

#ifdef PEGASUS_ENABLE_IPV6
    if (_ipv6Present)
    {
        // IPv6.

        parms.objid = ID_ipv6RouteTableNumEnt;
        parms.buffer = (char *) &count;
        len = sizeof(count);
        parms.len = (unsigned int *) &len;

        if (get_mib_info (fd, &parms) < 0)
        {
            throw CIMOperationFailedException(
                "Can't get ID_ipRouteNumEnt from get_mib_info(): " +
                String(strerror(errno)));
        }

        route6_buf = (mib_ipv6RouteEnt *)malloc(count*sizeof(mib_ipv6RouteEnt));

        if (route6_buf == 0)
        {
            free(route6_buf);
            throw CIMOperationFailedException(
                "Error in allocating space for the kernel interface table: " +
                String(strerror(errno)));
        }

        parms.objid = ID_ipv6RouteTable;
        parms.buffer = (char *) route6_buf;
        len = count * sizeof(mib_ipv6RouteEnt);
        parms.len = &len;

        if (get_mib_info (fd, &parms) < 0)
        {
            free(route6_buf);
            throw CIMOperationFailedException(
                "Can't get ID_ipRouteTable from get_mib_info(): " +
                String(strerror(errno)));
        }

        // Create the IPv6 Route List entries.

        for (i=0; i < count ; i++)
        {
            NextHopIPRoute _nhipr;

            // check to see that this is a valid type to represent.
            if (route6_buf[i].ipv6RouteType == 3 ||
                route6_buf[i].ipv6RouteType == 4)
            {
                _nhipr.set_protocolType(PROTOCOL_IPV6);

                char _dest[INET6_ADDRSTRLEN];
                t6 = route6_buf[i].ipv6RouteDest;
                _nhipr.set_destAddr(
                    inet_ntop(AF_INET6, &t6, _dest, INET6_ADDRSTRLEN));

                _nhipr.set_prefixLength(
                    (Uint8)route6_buf[i].ipv6RoutePfxLength);

                _nhipr.set_routeType(
                    (Uint16)route6_buf[i].ipv6RouteType);

                t6 = route6_buf[i].ipv6RouteNextHop;
                _nhipr.set_nextHop(
                    inet_ntop(AF_INET6, &t6, _dest, INET6_ADDRSTRLEN));

                _nhiprl.push_back(_nhipr); // Add another IP Route to the list.
            }

        }

        free(route6_buf);

    }
#endif

    close_mib(fd);

#ifdef IPPROVIDER_DEBUG
    cout << "NextHopRouteList::NextHopRouteList() -- done" << endl;
#endif

}

/*
================================================================================
NAME              : NextHopRouteList Destructor.
DESCRIPTION       : None.
ASSUMPTIONS       : None.
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
NAME              : findRoute.
DESCRIPTION       : Find the requested IP Route based on the destination
                  : address, destination mask, prefix length, and address type.
ASSUMPTIONS       : None.
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
================================================================================
*/
Boolean NextHopRouteList::findRoute(
    const String &instanceID,
    NextHopIPRoute &ipRInst) const
{
    int i;

    for (i = 0; i < _nhiprl.size(); i++)
    {
        String _instID;

        if (_nhiprl[i].getInstanceID(_instID) &&
            String::equal(_instID,instanceID))
        {
            ipRInst = _nhiprl[i];
            return true;
        }
    }

#ifdef IPPROVIDER_DEBUG
    cout << "NextHopRouteList::findRoute(): NOT FOUND instanceID="
         << instanceID << endl;
#endif

    // IP Route not found.
    return false;
}


/*
================================================================================
NAME              : getRoute.
DESCRIPTION       : Get an IP Route based on an index.
ASSUMPTIONS       : None.
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
================================================================================
*/
NextHopIPRoute NextHopRouteList::getRoute(const int index) const
{
    return _nhiprl[index];
}

/*
================================================================================
NAME              : size.
DESCRIPTION       : Find the size of the Route List.
ASSUMPTIONS       : None.
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
================================================================================
*/
int NextHopRouteList::size() const
{
    return _nhiprl.size();
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
    s = _name;
    return true;
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
    s = _accessInfo;
    return true;
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
    ui = _infoFormat;
    return true;
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
    if (_infoFormat == 1)
    {
        s = _otherInfoFmtDesc;
        return true;
    }
    else
    {
        return false;
    }
}

/*
================================================================================
NAME              : set_name
DESCRIPTION       : Platform-specific routine to set the Name property of the
                  : Remote Service Access Point.
ASSUMPTIONS       : None
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
================================================================================
*/
void RSAp::set_name(const String& n)
{
    _name = n;
}

/*
================================================================================
NAME              : set_accessInfo
DESCRIPTION       : Platform-specific routine to set the AccessInfo property of
                  : the Remote Service Access Point.
ASSUMPTIONS       : None
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
================================================================================
*/
void RSAp::set_accessInfo(const String& aI)
{
    _accessInfo = aI;
}

/*
================================================================================
NAME              : set_infoFormat
DESCRIPTION       : Platform-specific routine to set the InfoFormat property of
                  : the Remote Service Access Point.
ASSUMPTIONS       : None
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
================================================================================
*/
void RSAp::set_infoFormat(const Uint16& iF)
{
    _infoFormat = iF;
}

/*
================================================================================
NAME              : set_otherInfoFmtDesc
DESCRIPTION       : Platform-specific routine to set the
                  : OtherInfoFormatDescription property of the Remote Service
                  : Access Point.
ASSUMPTIONS       : None
PRE-CONDITIONS    :
POST-CONDITIONS   :
NOTES             :
================================================================================
*/
void RSAp::set_otherInfoFmtDesc(const String& oifd)
{
    _otherInfoFmtDesc = oifd;
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

    for (Uint16 i=0; i<_nhiprl.size(); i++)
    {
        if (!_nhiprl[i].isRouteLocal())
        {
            RSAp _rsap;
            String _name,
                _destAddr;

            if (_nhiprl[i].getNextHop(_name))
            {
                if (_nhiprl[i].getDestinationAddress(_destAddr))
                {
                    _rsap.set_name(_name);
                    _rsap.set_accessInfo(_name+"/"+_destAddr);
                    _rsap.set_infoFormat(1);
                    _rsap.set_otherInfoFmtDesc(
                        "IPAddress/IPDestinationAddress");
                    _rsapl.push_back(_rsap);
                }
            }
        }
    }

#ifdef IPPROVIDER_DEBUG
    cout << "RSApList::RSApList() -- done" << endl;
#endif

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
    int i;

    for (i = 0; i < _rsapl.size(); i++)
    {
        String _name;

        if (_rsapl[i].getName(_name) &&
            String::equal(_name,name))
        {
            rRSAp = _rsapl[i];
            return true;
        }
    }

#ifdef IPPROVIDER_DEBUG
    cout << "RSApList::findService(): NOT FOUND name="
         << name << endl;
#endif

    // Service not found.
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
    return _rsapl[index];
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
    return _rsapl.size();
}

