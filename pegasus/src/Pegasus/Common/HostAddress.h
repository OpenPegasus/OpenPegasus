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

#ifndef Pegasus_HostAddress_h
#define Pegasus_HostAddress_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/Linkage.h>
#include <Pegasus/Common/String.h>
#include <Pegasus/Common/Socket.h>

#ifdef PEGASUS_OS_VMS
#include <netinet/in6.h>
#endif

PEGASUS_NAMESPACE_BEGIN

#ifdef INET_ADDRSTRLEN
#define PEGASUS_INET_ADDRSTR_LEN INET_ADDRSTRLEN
#else
#define PEGASUS_INET_ADDRSTR_LEN 16
#endif

#ifdef INET6_ADDRSTRLEN
#define PEGASUS_INET6_ADDRSTR_LEN INET6_ADDRSTRLEN
#else
#define PEGASUS_INET6_ADDRSTR_LEN 46
#endif

#ifdef IN6ADDR_LOOPBACK_INIT
#define PEGASUS_IPV6_LOOPBACK_INIT IN6ADDR_LOOPBACK_INIT
#else
#define PEGASUS_IPV6_LOOPBACK_INIT {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1}
#endif


#ifdef INADDR_LOOPBACK
#define PEGASUS_IPV4_LOOPBACK_INIT INADDR_LOOPBACK
#else
#define PEGASUS_IPV4_LOOPBACK_INIT 0x7F000001
#endif

#define PEGASUS_IPV4_LOOPBACK_RANGE_START 0x7F000000
#define PEGASUS_IPV4_LOOPBACK_RANGE_END 0x7FFFFFFF


#ifdef PEGASUS_ENABLE_IPV6
#define PEGASUS_IN6_ADDR_SIZE (sizeof (struct in6_addr))
#else
#define PEGASUS_IN6_ADDR_SIZE 0x10
#endif

/**
    This class is used to store the host address. HostAddress can be Hostname or
    IPv4 address or IPv6 address.
*/

class PEGASUS_COMMON_LINKAGE HostAddress
{
public:

    /**
        Address types that HostAddress can have.
    */
    enum
    {
        AT_INVALID,
        AT_IPV4 = AF_INET,
#ifdef PEGASUS_ENABLE_IPV6
        AT_IPV6 = AF_INET6,
#else
        AT_IPV6 = 0x17,
#endif
        AT_HOSTNAME
    };


    HostAddress();
    ~HostAddress();

    HostAddress(const HostAddress &rhs);
    HostAddress& operator =(const HostAddress &rhs);

    /**
    *   AddrStr can be HostName or IPv4Address or IPv6Address
    *   (without brackets).
    *   Returns true, if set successfully.
    *   Returns false, if unscessfully and the HostAddress is
    *   invalid.
    */

    Boolean setHostAddress(const String &addrStr);

    /**
        Returns true if the constructed HostAddress is valid.
        If valid it can be HostName or IPv4Address or IPv6Address.
        Check if HostAddress is valid by using isValid() method
        before making any calls on HostAddress object.
    */
    Boolean isValid() const;

    /**
        Verifies given IPv4Address and returns true if it is valid.
    */
    static Boolean isValidIPV4Address(const String &ipv4Address);

    /**
        Verifies given IPv6Address (without brackets) and returns
        true if it is valid.
    */
    static Boolean isValidIPV6Address(const String &ipv6Address);

    /**
        Verifies given hostName and returns true if it is valid.
    */
    static Boolean isValidHostName(const String &hostName);

    /**
        The covertBinaryToText (inet_ntop()) function shall convert a numeric
        address into a text string suitable for presentation. The af argument
        shall specify the family of the address. This can be AF_INET  or
        AF_INET6. The src argument points to a buffer  holding  an IPv4 address
        if the af argument is AF_INET,  or an IPv6 address if the af argument is
        AF_INET6. The address must be in network byte order. The dst argument
        points to a buffer  where  the  function  stores  the resulting  text
        string. It shall not be NULL. The size argument specifies the size of
        this buffer, which shall be  large enough to hold the text string
        (PEGASUS_INET_ADDRSTR_LEN  characters  for  IPv4,
        PEGASUS_INET6_ADDRSTR_LEN  characters  for IPv6).
    */
    static const char *convertBinaryToText(int af, const void *src,
        char *dst, Uint32 size);

    /*
        The  convertTextToBinray (inet_pton())  function  shall convert an
        address in its standard text presentation form into its numeric
        binary form.  The af argument  shall specify the family
        of the address. The AF_INET and AF_INET6 address families
        shall be supported. The src argument points to the
        string being passed in. The dst argument points to a buffer into which
        the function stores the numeric address. This shall be large enough to
        hold  the  numeric  address  (32  bits  for  AF_INET,  128 bits for
        AF_INET6).
    */
    static int convertTextToBinary(int af, const char *src, void *dst);

    /**
        Checks whether the two addresses in binary form are equal based on
        address family. af can be AF_INET or AF_INET6.
    */
    static Boolean equal(int af, void *p1, void *p2);

    /**
        Returns HostName or IPv4Address or IPv6Address. This returns an empty
        String if HostAddress is not valid.
        Check if HostAddress is valid by using isValid() method
        before making any calls on HostAddress object.
    */
    String getHost() const;

    /**
       Returns address type. It can be AT_IPV4, AT_IPV6 or AT_HOSTNAME.
    */
    Uint32 getAddressType() const;

    /*
     * get the scope ID for the link-local address
    */
    Uint32 getScopeID() const;

   //check if the address in the _hostaddrStr is link-local
   Boolean isHostAddLinkLocal() const; 

private:

    String _hostAddrStr;
    Uint16 _addrType;
    Boolean _isValid;
    Boolean _isAddrLinkLocal;
    Uint32 _scopeID;

    /**
    * Retruns true if ip6add is a valid IPv6 address.
    *   set _hostAddrStr to a valid IPv6
    *   _isAddrLinkLocal to true if ip6add is a link local address
    *   _scopeID if it is a link local address. Other wise false.
    * 
    */
    Boolean _checkIPv6AndLinkLocal(const String &ip6add);

};

PEGASUS_NAMESPACE_END

#endif //Pegasus_HostAddress_h
