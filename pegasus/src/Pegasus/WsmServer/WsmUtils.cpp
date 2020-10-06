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

#include <Pegasus/Common/String.h>
#include <Pegasus/Common/StringConversion.h>
#include <Pegasus/Common/PegasusAssert.h>
#include "WsmUtils.h"
#include "WsmConstants.h"

PEGASUS_NAMESPACE_BEGIN

XmlNamespace WsmNamespaces::supportedNamespaces[] =
{
    {
        "SOAP-ENV",
        "http://www.w3.org/2003/05/soap-envelope",
        SOAP_ENVELOPE,
        0
    },
    {
        "SOAP-ENC",
        "http://www.w3.org/2003/05/soap-encoding",
        SOAP_ENCODING,
        0
    },
    {
        "xml",
        "http://www.w3.org/XML/1998/namespace",
        XML_NAMESPACE,
        0
    },
    {
        "xsi",
        "http://www.w3.org/2001/XMLSchema-instance",
        XML_SCHEMA_INSTANCE,
        0
    },
    {
        "xsd",
        "http://www.w3.org/2001/XMLSchema",
        XML_SCHEMA,
        0
    },
    {
        "wsman",
        "http://schemas.dmtf.org/wbem/wsman/1/wsman.xsd",
        WS_MAN,
        0
    },
    {
        "wsmb",
        "http://schemas.dmtf.org/wbem/wsman/1/cimbinding.xsd",
        WS_CIM_BINDING,
        0
    },
    {
        "wsa",
        "http://schemas.xmlsoap.org/ws/2004/08/addressing",
        WS_ADDRESSING,
        0
    },
    {
        "wxf",
        "http://schemas.xmlsoap.org/ws/2004/09/transfer",
        WS_TRANSFER,
        0
    },
    {
        "wsen",
        "http://schemas.xmlsoap.org/ws/2004/09/enumeration",
        WS_ENUMERATION,
        0
    },
    {
        "wse",
        "http://schemas.xmlsoap.org/ws/2004/08/eventing",
        WS_EVENTING,
        0
    },
    {
        "wsp",
        "http://schemas.xmlsoap.org/ws/2004/09/policy",
        WS_POLICY,
        0
    },
    {
        "wsdl",
        "http://schemas.xmlsoap.org/wsdl",
        WSDL,
        0
    },
    {
        "wscim",
        "http://schemas.dmtf.org/wbem/wscim/1/cim-schema/2",
        WS_CIM_SCHEMA,
        0
    },
    {0, 0, LAST, 0}
};


String WsmUtils::getMessageId()
{
    // DSP0226 R5.4.4-1: The MessageID and RelatesTo URIs may be of any format,
    // as long as they are valid URIs according to RFC 3986.  Two URIs are
    // considered different even if the characters in the URIs differ only by
    // case.
    //
    // The following two formats are endorsed by this specification.  The first
    // is considered a best practice because it is backed by IETF RFC 4122:
    //     urn:uuid:xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx
    //     or
    //     uuid:xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx
    //
    // In these formats, each x is an uppercase or lowercase hexadecimal digit
    // (lowercase is required by RFC 4122); there are no spaces or other
    // tokens.  The value may be a DCE-style universally unique identifier
    // (UUID) with provable uniqueness properties in this format, however, it
    // is not necessary to have provable uniqueness properties in the URIs
    // used in the wsa:MessageID and wsa:RelatesTo headers.
    //
    // Regardless of format, the URI should not exceed the maximum defined in
    // R13.1-6. [sic]

    // DSP0226 R5.4.4-2: The MessageID should be generated according to any
    // algorithm that ensures that no two MessageIDs are repeated.  Because
    // the value is treated as case-sensitive (R5.4.4-1), confusion can arise
    // if the same value is reused differing only in case.  As a result, the
    // service shall not create or employ MessageID values that differ only in
    // case.  For any message transmitted by the service, the MessageID shall
    // not be reused.

    // Note: This algorithm could be improved, but provable uniqueness is not
    // required (see above).

    char uuid[42];
    sprintf(uuid, "uuid:%08X-%04X-%04X-%04X-%08X%04X",
        rand(),
        rand() & 0xFFFF,
        rand() & 0xFFFF,
        rand() & 0xFFFF,
        rand(),
        rand() & 0xFFFF);
    return String(uuid, 41);
}

const char* WsmUtils::skipHostUri(const char* uri)
{
    const char* p = uri;

    // Skip over "http://" part:

    if (p[0] != 'h')
        return &p[0];
    if (p[1] != 't')
        return &p[1];
    if (p[2] != 't')
        return &p[2];
    if (p[3] != 'p')
        return &p[3];
    if (p[4] != ':')
        return &p[4];
    if (p[5] != '/')
        return &p[5];
    if (p[6] != '/')
        return &p[6];

    p += 7;

    // Return the first character that is not a '/' (or return pointer to
    // zero terminator).

    while (*p && *p != '/')
        p++;

    return p;
}

String WsmUtils::getRootResourceUri(const String& resourceUri)
{
    CString cstr(resourceUri.getCString());
    const char* start = cstr;
    const char* end = skipHostUri(start);
    const size_t n = sizeof(WSM_RESOURCEURI_CIMSCHEMAV2_SUFFIX) - 1;

    if (strncmp(end, WSM_RESOURCEURI_CIMSCHEMAV2_SUFFIX, n) == 0)
    {
        end += n;
        return String(start, end - start);
    }

    const int NS = WsmNamespaces::WS_CIM_SCHEMA;
    return String(WsmNamespaces::supportedNamespaces[NS].extendedName);
}

String WsmUtils::toMicroSecondString(const CIMDateTime &rep)
{
    const Uint32 strDurationLength=26;
    Uint64 duration = rep.toMicroSeconds();
    Uint32 outputLength = 0;
    char buffer[strDurationLength];
    const char* output = Uint64ToString(buffer, duration, outputLength); 
    return String(output, outputLength);
}

PEGASUS_NAMESPACE_END
