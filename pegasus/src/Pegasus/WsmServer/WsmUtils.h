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

#ifndef Pegasus_WsmUtils_h
#define Pegasus_WsmUtils_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/String.h>
#include <Pegasus/Common/XmlParser.h>
#include <Pegasus/Common/CIMDateTime.h>
#include <Pegasus/WsmServer/Linkage.h>

PEGASUS_NAMESPACE_BEGIN

class PEGASUS_WSMSERVER_LINKAGE WsmNamespaces
{
public:

    enum Type
    {
        UNKNOWN = -1,

        SOAP_ENVELOPE = 0,
        SOAP_ENCODING,
        XML_NAMESPACE,
        XML_SCHEMA_INSTANCE,
        XML_SCHEMA,
        WS_MAN,
        WS_CIM_BINDING,
        WS_ADDRESSING,
        WS_TRANSFER,
        WS_ENUMERATION,
        WS_EVENTING,
        WS_POLICY,
        WSDL,
        WS_CIM_SCHEMA,

        COUNT,
        LAST = COUNT
    };

    static XmlNamespace supportedNamespaces[];

private:

    WsmNamespaces();
};


class PEGASUS_WSMSERVER_LINKAGE WsmUtils
{
public:

    static String getMessageId();

    // Attempt to skip over the host URI of the form: http://host
    // return a pointer to the character immdiately after the host URI
    // (a slash or a zero terminator). Otherwise return a pointer to the
    // first nonmatching character.
    static const char* skipHostUri(const char* str);

    // Return the leading part of resourceUri that matches the
    // following expression (where <SERVER> is any server expression.
    //     http://<SERVER>/wbem/wscim/1/cim-schema/2"
    static String getRootResourceUri(const String& resourceUri);
   
    /** Returns a string representing the DateTime value of the
        CIMDateTime object in the microsecond format.
        @return String representing the DateTime value in the microsecond
        format.
    */
    static String toMicroSecondString(const CIMDateTime &rep);


private:

    WsmUtils();
};

PEGASUS_NAMESPACE_END

#endif /* Pegasus_WsmUtils_h */
