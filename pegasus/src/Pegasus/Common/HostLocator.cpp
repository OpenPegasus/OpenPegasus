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

#include <Pegasus/Common/HostLocator.h>

PEGASUS_NAMESPACE_BEGIN

static bool _parseLocator(
    const String &locator,
    HostAddress& addr,
    Uint32& port)
{
    const Uint16* first = (const Uint16*)locator.getChar16Data();
    const Uint16* last = first + locator.size();

    port = HostLocator::PORT_UNSPECIFIED;

    // Reject zero length locators.

    if (first == last)
    {
        return false;
    }

    // Parse the host address.

    const Uint16* p = first;

    if (*p == '[')
    {
        // Parse "[...]" expresion.

        const Uint16* start = ++p;

        while (*p && *p != ']')
            p++;

        if (*p != ']')
        {
            return false;
        }

        addr.setHostAddress(String((const Char16*)start, p - start));
        p++;

        // Only IPV6 addresses may be enclosed in braces.

        if (addr.getAddressType() != HostAddress::AT_IPV6)
        {
            return false;
        }
    }
    else
    {
        // Find end-of-string host address (null terminator or colon).

        const Uint16* start = p;

        while (*p && *p != ':')
            p++;

        addr.setHostAddress(String((const Char16*)start, p - start));

        if (!addr.isValid())
        {
            return false;
        }

        // IPV6 addresses must be enclosed in braces.

        if (addr.getAddressType() == HostAddress::AT_IPV6)
        {
            return false;
        }
    }

    // Parse the port number:

    if (*p == ':')
    {
        const Uint16* start = ++p;

        // If empty port number, ignore and proceed as unspecified port.
        if (start == last)
        {
            return true;
        }

        port = HostLocator::PORT_INVALID;

        // Convert string port number to integer (start at end of string).

        Uint32 r = 1;
        Uint32 x = 0;

        for (const Uint16* q = last; q != start; q--)
        {
            Uint16 c = q[-1];

            if (c > 127 || !isdigit(c))
                return false;

            x += r * (c - '0');
            r *= 10;
        }

        if (x > HostLocator::MAX_PORT_NUMBER)
        {
            return false;
        }

        port = x;

        p++;
        return true;
    }
    else if (*p != '\0')
    {
        return false;
    }

    // Unreachable!
    return true;
}

HostLocator::HostLocator() : _isValid(false), _portNumber(PORT_UNSPECIFIED)
{
}

HostLocator::HostLocator(const String &locator)
{
    _isValid = _parseLocator(locator, _hostAddr, _portNumber);
}

HostLocator& HostLocator::operator =(const HostLocator &rhs)
{
    if (this != &rhs)
    {
        _hostAddr = rhs._hostAddr;
        _portNumber = rhs._portNumber;
        _isValid = rhs._isValid;
    }

    return *this;
}

HostLocator::HostLocator(const HostLocator &rhs)
{
    *this = rhs;
}

HostLocator::~HostLocator()
{
}

void HostLocator::setHostLocator(const String &locator)
{
    _isValid = _parseLocator(locator, _hostAddr, _portNumber);
}

Uint32 HostLocator::getPort()
{
    return _portNumber;
}

String HostLocator::getPortString()
{
    char portStr[20];
    sprintf(portStr, "%u", _portNumber);
    return String(portStr);
}

Uint16 HostLocator::getAddressType()
{
    return _hostAddr.getAddressType();
}

Boolean HostLocator::isValid()
{
    return _isValid;
}

String HostLocator::getHost()
{
    return _hostAddr.getHost();
}

Boolean HostLocator::isPortSpecified()
{
    return _portNumber != PORT_UNSPECIFIED && _portNumber != PORT_INVALID;
}

PEGASUS_NAMESPACE_END
