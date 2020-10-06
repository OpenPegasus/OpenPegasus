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

#if defined(PEGASUS_OS_ZOS)
#define _XOPEN_SOURCE_EXTENDED 1
#endif
PEGASUS_USING_STD;

PEGASUS_NAMESPACE_BEGIN

String Guid::getGuid(const String &prefix)
{
    Uint32 seconds(0), milliSeconds(0);
    System::getCurrentTime(seconds, milliSeconds);
    CIMValue secondsValue(seconds);
    CIMValue milliSecondsValue(milliSeconds);
    String ipAddress;
    int af;

    String hostName(System::getHostName());
    if (!System::getHostIP(hostName, &af, ipAddress))
    {
        // set default address if everything else failed
        ipAddress = String("127.0.0.1");
    }

    // change the dots to dashes
    for (Uint32 i=0; i<ipAddress.size(); i++)
    {
        if (ipAddress[i] == Char16('.'))
        {
            ipAddress[i] = Char16('-');
        }
    }

    String guid(prefix);
    guid.append(secondsValue.toString());
    guid.append(milliSecondsValue.toString());
    guid.append(Char16('-'));
    guid.append(ipAddress);

    return guid;
}

PEGASUS_NAMESPACE_END
// END_OF_FILE
