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

#ifndef Pegasus_WsmWriter_h
#define Pegasus_WsmWriter_h

#include <iostream>
#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/InternalException.h>
#include <Pegasus/Common/ArrayInternal.h>
#include <Pegasus/Common/String.h>
#include <Pegasus/Common/ContentLanguageList.h>
#include <Pegasus/Common/Message.h>
#include <Pegasus/Common/Buffer.h>
#include <Pegasus/Common/StrLit.h>
#include <Pegasus/Common/XmlGenerator.h>
#include <Pegasus/WsmServer/WsmUtils.h>
#include <Pegasus/WsmServer/WsmInstance.h>
#include <Pegasus/WsmServer/WsmProperty.h>
#include <Pegasus/WsmServer/WsmValue.h>
#include <Pegasus/WsmServer/WsmFault.h>
#include <Pegasus/WsmServer/WsmEndpointReference.h>
#include <Pegasus/Common/HTTPMessage.h>
#include <Pegasus/Client/ClientAuthenticator.h>

PEGASUS_NAMESPACE_BEGIN

class PEGASUS_WSMSERVER_LINKAGE WsmWriter : public XmlGenerator
{
public:

    static void appendSoapFaultHeaders(
        Buffer& out,
        const SoapNotUnderstoodFault& fault,
        const String& action,
        const String& messageId,
        const String& relatesTo);
    static void appendSoapFaultBody(
        Buffer& out,
        const SoapNotUnderstoodFault& fault);

    static void appendWsmFaultBody(
        Buffer& out,
        const WsmFault& fault);

    static Buffer formatHttpErrorRspMessage(
        const String& status,
        const String& cimError = String::EMPTY,
        const String& errorDetail = String::EMPTY);

    static void appendInstanceElement(
        Buffer& out,
        const String& resourceUri,
        WsmInstance& instance,
        const char* ns,
        Boolean isEmbedded);

     static void appendPropertyElement(
        Buffer& out,
        const String& resourceUri,
        WsmProperty& property,
        const char* ns);

    static void appendEPRElement(
        Buffer& out, const WsmEndpointReference& epr);
    static void appendStringElement(
        Buffer& out, const String& str);

    static void appendStartTag(
        Buffer& out,
        WsmNamespaces::Type nsType,
        const StrLit& tagName,
        const char* attrName = 0,
        const String& attrValue = String::EMPTY);
    static void appendEndTag(
        Buffer& out,
        WsmNamespaces::Type nsType,
        const StrLit& tagName);
    static void appendEmptyTag(
        Buffer& out,
        WsmNamespaces::Type nsType,
        const StrLit& tagName);
    static void appendTagValue(
        Buffer& out,
        WsmNamespaces::Type nsType,
        const StrLit& tagName,
        const String& value,
        const char* attrName = 0,
        const String& attrValue = String::EMPTY);

    static void appendHTTPResponseHeader(
        Buffer& out,
        const String& action,
        HttpMethod httpMethod,
        Boolean omitXMLProcessingInstruction,
        const ContentLanguageList& contentLanguages,
        Boolean isFault,
        Uint32 contentLength = 0);

    static void appendSoapEnvelopeStart(
        Buffer& out,
        const ContentLanguageList& contentLanguages = ContentLanguageList());
    static void appendSoapEnvelopeEnd(Buffer& out);
    static void appendSoapHeaderStart(Buffer& out);
    static void appendSoapHeaderEnd(Buffer& out);
    static void appendSoapBodyStart(Buffer& out);
    static void appendSoapBodyEnd(Buffer& out);

    static void appendSoapHeader(
        Buffer& out,
        const String& action,
        const String& messageId,
        const String& relatesTo,
        const String& toAddress="",
        const String& replyTo="",
        const Boolean& ackRequired=false);

    static void appendInvokeOutputElement(
        Buffer& out,
        const String& resourceUri,
        const String& className,
        const String& methodName,
        WsmInstance& instance,
        const char* ns);
    
   static Buffer appendHTTPRequestHeader( XmlParser& parser,
            const String& hostName,
            Boolean useMPost,
            Boolean useHTTP11,
            Buffer& content,
            Buffer& httpHeaders,
            const String& destination);
    
    static void addAuthHeader(
        HTTPMessage*& httpMessage,
        AutoPtr<ClientAuthenticator>& authenticator);

private:

    WsmWriter();
};

PEGASUS_NAMESPACE_END

#endif /* Pegasus_WsmWriter_h */
