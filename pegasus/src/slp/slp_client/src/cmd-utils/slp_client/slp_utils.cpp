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

#include "slp_utils.h"

#include <ctype.h>

#define AT_IPV4 AF_INET
#ifdef PEGASUS_ENABLE_IPV6
#define AT_IPV6 AF_INET6
#else
#define AT_IPV6 0xA
#endif

#if defined (PEGASUS_OS_TYPE_WINDOWS) || !defined (PEGASUS_ENABLE_IPV6)

/*
    Address conversion utility functions.
*/

/*
    Converts given "src" text address (Ex: 127.0.0.1) to equivalent binary form
    and stores in "dst"  buffer (Ex 0x7f000001). Returns 1 if given ipv4 address
    is valid or returns -1 if invalid. Returns value in network byte order.
*/

static int _inet_ptonv4(const char *src, void *dst)
{
    BOOL isValid = 1;
    uint16 octetValue[4] = {0};
    int octet, i;
    int value;

     // Check for valid IPV4 address.
    for (octet = 1, i = 0; octet <= 4; octet++)
    {
        int j = 0;
        if (!(isascii(src[i]) && isdigit(src[i])))
        {
            isValid = 0;
            break;
        }
        while (isascii(src[i]) && isdigit(src[i]))
        {
            if (j == 3)
            {
                isValid = 0;
                break;
            }
            octetValue[octet-1] = octetValue[octet-1]*10 + (src[i] - '0');
            i++;
            j++;
        }
        if (octetValue[octet-1] > 255)
        {
            isValid = 0;
            break;
        }
        // Check for invalid character in IP address
        if ((octet != 4) && (src[i++] != '.'))
        {
            isValid = 0;
            break;
        }
        // Check for the case where it's a valid host name that happens
        // to have 4 (or more) leading all-numeric host segments.
        if ((octet == 4) && (src[i] != ':') &&
            src[i] != 0)
        {
            isValid = 0;
            break;
        }
    }
    if (!isValid)
    {
        return 0;
    }

    // Return the value in network byte order.
    value = octetValue[0];
    value = (value << 8) + octetValue[1];
    value = (value << 8) + octetValue[2];
    value = (value << 8) + octetValue[3];
    value = htonl(value);
    memcpy (dst, &value, sizeof(int));

    return 1;
}

/*
     Converts given ipv6 text address (ex. ::1) to binary form and stroes
     in "dst" buffer (ex. 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1). Returns 1
     if src ipv6 address is valid or returns -1 if invalid. Returns value
     in network byte order.
*/
static int _inet_ptonv6(const char *src, void *dst)
{
    int ccIndex = -1;
    int sNumber = 0;
    uint16 sValues[8] = {0};
    BOOL ipv4Mapped = 0;
    int chars;
    int i,j;

    while (*src && sNumber < 8)
    {
        if (*src == ':')
        {
            if (!*++src)
            {
                return 0;
            }
            if (*src == ':')
            {
                if (ccIndex != -1)
                {
                    return 0;
                }
                ccIndex = sNumber;
                if (!*++src)
                {
                    break;
                }
            }
        }
        if ((isalpha(*src) && tolower(*src) <= 'f') || isdigit(*src))
        {
            // Check for ipv4 compatible ipv6 or ipv4 mapped ipv6 addresses
            if(!strchr(src, ':') && strchr(src, '.'))
            {
                if ( _inet_ptonv4 (src, sValues + sNumber) != 1)
                {
                    return 0;
                }
                sNumber += 2;
                ipv4Mapped = 1;
                break;
            }
            chars = 0;
            while (*src && *src != ':')
            {
                if (chars++ == 4)
                {
                    return 0;
                }
                if (!((isalpha(*src) && tolower(*src) <= 'f') || isdigit(*src)))
                {
                    return 0;
                }
                sValues[sNumber] = sValues[sNumber] * 16 +
                    (isdigit(*src) ? *src - '0' : (tolower(*src) - 'a' + 10));
                ++src;
            }
            sValues[sNumber] = htons(sValues[sNumber]);
            ++sNumber;
        }
        else
        {
            return 0;
        }
    }

    if ((!ipv4Mapped &&*src) || (ccIndex == -1 && sNumber < 8) ||
        (ccIndex != -1 && sNumber == 8) )
    {
        return 0;
    }
    memset(dst, 0, PEGASUS_IN6_ADDR_SIZE);
    for (i = 0, j = 0; i < 8 ; ++i)
    {
        if (ccIndex == i)
        {
            i += 7 - sNumber;
        }
        else
        {
            memcpy ((char*) dst + i * 2, sValues + j++ , 2);
        }
    }
    return 1;
}

/*
    Converts given ipv4 address in binary form to text form. Ex. 0x7f000001
    to 127.0.0.1.
*/
static const char *_inet_ntopv4(const void *src, char *dst, int size)
{

   int n;

   memset(dst, 0, size);
   memcpy(&n, src, sizeof (int));
   n = ntohl(n);
   sprintf(dst, "%u.%u.%u.%u", n >> 24 & 0xFF ,
       n >> 16 & 0xFF, n >> 8 & 0xFF, n & 0xFF);

   return dst;
}

/*
    Converts given ipv6 address in binary form to text form. Ex.
    0000000000000001 to ::1.
*/
static const char *_inet_ntopv6(const void *src, char *dst, int size)
{

    uint16 n[8];
    int ccIndex = -1;
    int maxZeroCnt = 0;
    int zeroCnt = 0;
    int index = 0;
    int i;
    char tmp[50];
    BOOL mapped;

    memcpy (n, src, PEGASUS_IN6_ADDR_SIZE);
    memset(dst, 0, size);
    for (i = 0; i < 8 ; ++i)
    {
        if (n[i])
        {
            if (zeroCnt)
            {
                if (zeroCnt > maxZeroCnt)
                {
                    ccIndex = index;
                    maxZeroCnt = zeroCnt;
                }
                zeroCnt = index = 0;
            }
            n[i] = ntohs (n[i]);
        }
        else
        {
            if(!zeroCnt++)
            {
                if (ccIndex == -1)
                {
                    ccIndex = i;
                }
                index = i;
            }
        }
    }
    *dst = 0;
    zeroCnt = 0;

    for (i = 0; i < 8 ; ++i)
    {
        if (i == ccIndex)
        {
            sprintf(tmp, "::");
            while ( i < 8 && !n[i])
            {
                ++i;
                ++zeroCnt;
            }
            --i;
        }
        else
        {
            mapped = FALSE;
            if (ccIndex == 0 && zeroCnt > 4)
            {
                // check for ipv4 mapped ipv6 and ipv4 compatible ipv6
                // addresses.
                if (zeroCnt == 5 && n[i] == 0xffff)
                {
                    strcat(dst,"ffff:");
                    mapped = 1;
                }
                else if (zeroCnt == 6 && n[6])
                {
                    mapped = 1;
                }
            }
            if (mapped)
            {
                int m;
                m = htons(n[7]);
                m = (m << 16) + htons(n[6]);
                slp_ntop(AF_INET, &m, tmp, 50);
                i += 2;
            }
            else
            {
                sprintf(tmp, i < 7 && ccIndex != i + 1 ? "%x:" : "%x", n[i]);
            }
        }
        strcat(dst,tmp);
    }

    return dst;
}
#endif  // defined (PEGASUS_OS_TYPE_WINDOWS) || !defined (PEGASUS_ENABLE_IPV6)

BOOL slp_addr_equal(int af, void *p1, void *p2)
{
    switch (af)
    {
        case AT_IPV6:
             return !memcmp(p1, p2, PEGASUS_IN6_ADDR_SIZE);
        case AT_IPV4:
             return !memcmp(p1, p2, sizeof(struct in_addr));
    }

    return FALSE;
}

BOOL slp_is_loop_back(int af, void *addr)
{
#ifdef PEGASUS_ENABLE_IPV6
    struct in6_addr ip6 = PEGASUS_IPV6_LOOPBACK_INIT;
#endif
    if (!addr)
    {
        return FALSE;
    }
    switch (af)
    {
#ifdef PEGASUS_ENABLE_IPV6
        case AF_INET6:
            return !memcmp(&ip6, addr, sizeof (ip6));
#endif
        case AF_INET:
            {
                uint32 n = ntohl( *(uint32*)addr);
                return n >= PEGASUS_IPV4_LOOPBACK_RANGE_START && 
                    n <= PEGASUS_IPV4_LOOPBACK_RANGE_END;
            }
    }

    return FALSE;
}

BOOL slp_is_valid_ip6_addr(const char *ip6_addr)
{
    const char* p = ip6_addr;
    int numColons = 0;

#ifdef PEGASUS_ENABLE_IPV6
    struct in6_addr iaddr;
#else
    char iaddr[PEGASUS_IN6_ADDR_SIZE];
#endif

    while (*p)
    {
        if (*p == ':')
            numColons++;

        p++;
    }

    // No need to check whether IPV6 if no colons found.

    if (numColons == 0)
        return 0;

    return  slp_pton(AT_IPV6,
       (const char*)ip6_addr, (void*)&iaddr) == 1;
}

BOOL slp_is_valid_ip4_addr(const char *ip4_addr)
{
    const char* src = ip4_addr;
    int octetValue[4] = {0};
    int octet;
    int i;

    for (octet = 1, i = 0; octet <= 4; octet++)
    {
        int j = 0;

        if (!(isascii(src[i]) && isdigit(src[i])))
            return 0;

        while (isascii(src[i]) && isdigit(src[i]))
        {
            if (j == 3)
                return 0;

            octetValue[octet-1] = octetValue[octet-1] * 10 + (src[i] - '0');
            i++;
            j++;
        }

        if (octetValue[octet-1] > 255)
            return 0;

        if ((octet != 4) && (src[i++] != '.'))
            return 0;

        if ((octet == 4) && (src[i] != ':') && src[i] != 0)
            return 0;
    }

    return 1;
}

BOOL slp_is_valid_host_name(const char *host)
{
    const char *hostName = host;

    int i = 0;
    int expectHostSegment = 1;
    int hostSegmentIsNumeric;
    while (expectHostSegment)
    {
        expectHostSegment = 0;
        hostSegmentIsNumeric = 1; // assume all-numeric host segment
        if (!(isascii(hostName[i]) &&
            (isalnum(hostName[i]) || (hostName[i] == '_'))))
        {
            return 0;
        }
        while (isascii(hostName[i]) &&
            (isalnum(hostName[i]) || (hostName[i] == '-') ||
                (hostName[i] == '_')))
        {
            // If a non-digit is encountered, set "all-numeric"
            // flag to 0
            if (isalpha(hostName[i]) || (hostName[i] == '-') ||
                (hostName[i] == '_'))
            {
                hostSegmentIsNumeric = 0;
            }
            i++;
        }
        if (hostName[i] == '.')
        {
            i++;
            expectHostSegment = 1;
        }
    }
    // If the last Host Segment is all numeric, then return 0.
    // RFC 1123 says "highest-level component label will be alphabetic".
    if (hostSegmentIsNumeric || hostName[i] != 0)
    {
        return 0;
    }

    return 1;
}


int slp_pton(int af, const char *src, void *dst)
{
#if defined (PEGASUS_OS_TYPE_WINDOWS) || !defined (PEGASUS_ENABLE_IPV6)
    if (af == AT_IPV4)
    {
        return _inet_ptonv4(src, dst);
    }
    else if(af == AT_IPV6 || af == 0xA)
    {
        return _inet_ptonv6(src, dst);
    }
    return -1; // Unsupported address family.
#else
    return inet_pton(af, src, dst);
#endif
}

const char * slp_ntop(int af, const void *src, char *dst, int size)
{
#if defined (PEGASUS_OS_TYPE_WINDOWS) || !defined (PEGASUS_ENABLE_IPV6)
    if (af == AT_IPV4)
    {
        return _inet_ntopv4(src, dst, size);
    }
    else if (af == AT_IPV6 || af == 0xA)
    {
        return _inet_ntopv6(src, dst, size);
    }
    return 0; // Unsupported address family.
#else
    return inet_ntop(af, src, dst, size);
#endif
}

static  int _slp_get_socket_error()
{
#ifdef PEGASUS_OS_TYPE_WINDOWS
    return WSAGetLastError();
#else
    return errno;
#endif
}

#ifdef PEGASUS_OS_TYPE_WINDOWS
#   define PEGASUS_INVALID_SOCKET INVALID_SOCKET
#else
#   define PEGASUS_INVALID_SOCKET (-1)
#endif

static BOOL _slp_is_stack_active(int af)
{
    SOCKETD sock;
    if ((sock = _LSLP_SOCKET(af, SOCK_STREAM, IPPROTO_TCP))
        == SLP_INVALID_SOCKET)
    {
        if (_slp_get_socket_error() == SLP_INVALID_ADDRESS_FAMILY)
        {
            return FALSE;
        }
    }
    else
    {
        _LSLP_CLOSESOCKET(sock);
    }

    return TRUE;
}

BOOL slp_is_ip4_stack_active()
{
    return _slp_is_stack_active(AF_INET);
}

#ifdef PEGASUS_ENABLE_IPV6
BOOL slp_is_ip6_stack_active()
{
    return _slp_is_stack_active(AF_INET6);
}

// See RFC 3111, section 4.1
unsigned long slp_hash(const char *pc, unsigned int len)
{
    unsigned long h = 0;
    while (len-- != 0)
    {
        h *= 33;
        h += *pc++;
    }
    return (0x3FF & h); /* round to a range of 0-1023 */
}
#endif

BOOL slp_is_loop_back_addr(char *addr)
{
    char buff[PEGASUS_INET6_ADDRSTR_LEN];

    if (!addr)
    {
        return FALSE;
    }
    if (slp_is_valid_ip4_addr(addr))
    {
        slp_pton(AF_INET, addr, buff);
        return slp_is_loop_back(AF_INET, buff);
    }
#ifdef PEGASUS_ENABLE_IPV6
    if (slp_is_valid_ip6_addr(addr))
    {
        slp_pton(AF_INET6, addr, buff);
        return slp_is_loop_back(AF_INET6, buff);
    }
#endif
    return FALSE;
}

