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


#ifndef _DNSTESTCLIENT_H
#define _DNSTESTCLIENT_H

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/String.h>
#include <Pegasus/Client/CIMClient.h>

PEGASUS_USING_PEGASUS;
PEGASUS_USING_STD;

// Miscelaneous defines
static const CIMNamespaceName NAMESPACE("root/cimv2");
static const CIMName CLASS_NAME("PG_DNSService");
static const String RESOLV_CONF("/etc/resolv.conf");
static const String CAPTION("DNS Service");
static const String DESCRIPTION(
    "Describes the Domain Name System (DNS) Service");
static const String NAME_FORMAT("IP");

class DNSTestClient
{
public:
    DNSTestClient(CIMClient &client);
    ~DNSTestClient();

    void testEnumerateInstanceNames(CIMClient &client, Boolean verbose);
    void testEnumerateInstances(CIMClient &client, Boolean verbose);
    void testGetInstance(CIMClient &client, Boolean verbose);

    // utility methods for common test functions
    void errorExit(const String &message);
    void testLog(const String &message);

    // class is hardcoded, but flag for level of logging
    Boolean goodCreationClassName(const String &ccn,
                                  Boolean verbose);
    // have DNS-specific checks
    Boolean goodName(const String &name, Boolean verbose);

    // have searchlist array checks
    Boolean goodSearchList(const Array<String> &src, Boolean verbose);

    // have addresses array checks
    Boolean goodAddresses(const Array<String> &addr, Boolean verbose);

    // caption is hardcoded
    Boolean goodCaption(const String &cap, Boolean verbose);

    // description is hardcoded
    Boolean goodDescription(const String &des, Boolean verbose);

    // nameformat is hardcoded
    Boolean goodNameFormat(const String &nf, Boolean verbose);

private:
    // Verify if string is equal, then return true
    Boolean IsEqual(char text[], const char txtcomp[]);

    // Verify if found string in array
    Boolean FindInArray(Array<String> src, String text);

    // validate keys of the class
    void _validateKeys(CIMObjectPath &cimRef, Boolean verboseTest);

    // validate properties of the class
    void _validateProperties(CIMInstance &cimInst, Boolean verboseTest);
};

#endif
