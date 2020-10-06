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

#ifndef Pegasus_DNSService_h
#define Pegasus_DNSService_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Provider/CIMInstanceProvider.h>
#include <Pegasus/Provider/CIMMethodProvider.h>
#include <Pegasus/Common/OperationContext.h>
#include <Pegasus/Common/Pair.h>

PEGASUS_USING_PEGASUS;
PEGASUS_USING_STD;

// Role definitions
static const String DNS_ROLE_DOMAIN("domain");
static const String DNS_ROLE_NAMESERVER("nameserver");
static const String DNS_ROLE_SEARCH("search");

// Defines
#define CLASS_NAME CIMName("PG_DNSService")
#define SYSTEM_CREATION_CLASS_NAME CIMName("CIM_UnitaryComputerSystem")
#define CREATION_CLASS_NAME CIMName("PG_DNSService")
static const String DNS_CAPTION("DNS Service");
static const String DNS_DESCRIPTION(
    "Describes the Domain Name System (DNS) Service");

class DNSService
{
public:
    DNSService();
    virtual ~DNSService();

    // Verify and return Caption property
    Boolean getCaption(String & capt);

    // Verify and return Description property
    Boolean getDescription(String & desc);

    // This function retrieves the local host name
    Boolean getSystemName(String & hostName);

    // Verify and return Name property
    Boolean getDNSName(String & name);

    // Return SearchList parameter, if exists.
    Boolean getSearchList(Array<String> & srclst);

    // Verify and return Addresses parameter
    Boolean getAddresses(Array<String> & addrlst);

    // See if user has access to the configuration information
    Boolean AccessOk(const OperationContext & context);

private:
    String dnsName;
    Array<String> dnsSearchList;
    Array<String> dnsAddresses;

    // Retrieve DNS information from file /etc/resolv.conf
    Boolean getDNSInfo(void);

    // Verify if found string in array
    Boolean FindInArray(Array<String> src, String text);
};

#endif
