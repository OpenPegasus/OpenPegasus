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

#include <cstdlib>
#include <cstdio>

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/Constants.h>
#include <Pegasus/Common/XmlWriter.h>
#include <Pegasus/Common/Buffer.h>
#include <Pegasus/Common/StrLit.h>
#include <Pegasus/Common/StringConversion.h>
#include "WsmConstants.h"
#include "WsmUtils.h"
#include "WsmWriter.h"
#include "WsmSelectorSet.h"
#include <Pegasus/Common/StrLit.h>
#include <Clients/wbemexec/HttpConstants.h>

PEGASUS_NAMESPACE_BEGIN

inline void _writeNewlineForReadability(Buffer& out)
{
    // Uncomment this statement for XML response readability.
    // out << '\n';
}

// Append something like this to buffer: "<class:"
static inline void _appendStartPrefix(Buffer& out, const char* ns)
{
    out.append('<');
    out.append(ns, strlen(ns));
    out.append(':');
}

// Append something like this to buffer: "</class:"
static inline void _appendEndPrefix(Buffer& out, const char* ns)
{
    out.append('<');
    out.append('/');
    out.append(ns, strlen(ns));
    out.append(':');
}

//-----------------------------------------------------------------------------
//
// WsmWriter::formatHttpErrorRspMessage()
//
//-----------------------------------------------------------------------------

Buffer WsmWriter::formatHttpErrorRspMessage(
    const String& status,
    const String& cimError,
    const String& errorDetail)
{
    return XmlWriter::formatHttpErrorRspMessage(status, cimError, errorDetail);
}

void WsmWriter::appendSoapFaultHeaders(
    Buffer& out,
    const SoapNotUnderstoodFault& fault,
    const String& action,
    const String& messageId,
    const String& relatesTo)
{
    // Append the 'NotUnderstood' tag
    out << STRLIT("<");
    out << WsmNamespaces::supportedNamespaces[
        WsmNamespaces::SOAP_ENVELOPE].localName;
    out << STRLIT(":NotUnderstood qname=\"");
    if (fault.getNamespace().size())
    {
        out << STRLIT("ns:");
    }
    out << fault.getHeaderName();
    out << STRLIT("\"");
    if (fault.getNamespace().size())
    {
        out << STRLIT(" xmlns:ns=\"");
        out << fault.getNamespace();
        out << STRLIT("\"");
    }
    out << STRLIT("/>");
    _writeNewlineForReadability(out);

    appendSoapHeader(out, action, messageId, relatesTo);
}

void WsmWriter::appendSoapFaultBody(
    Buffer& out,
    const SoapNotUnderstoodFault& fault)
{
    ContentLanguageList msgLang = fault.getMessageLanguage();

    appendStartTag(out, WsmNamespaces::SOAP_ENVELOPE, STRLIT("Fault"));
    appendStartTag(out, WsmNamespaces::SOAP_ENVELOPE, STRLIT("Code"));
    String codeValue = String(WsmNamespaces::supportedNamespaces[
        WsmNamespaces::SOAP_ENVELOPE].localName) + String(":MustUnderstand");
    appendTagValue(
        out, WsmNamespaces::SOAP_ENVELOPE, STRLIT("Value"), codeValue);
    appendEndTag(out, WsmNamespaces::SOAP_ENVELOPE, STRLIT("Code"));

    String msg = fault.getMessage();
    if (msg.size() != 0)
    {
        String lang;
        if (msgLang.size() == 0)
            lang = "en-US";
        else
            lang = msgLang.getLanguageTag(0).toString();

        appendStartTag(out, WsmNamespaces::SOAP_ENVELOPE, STRLIT("Reason"));
        appendTagValue(out, WsmNamespaces::SOAP_ENVELOPE, STRLIT("Text"),
            msg, "xml:lang", lang);
        appendEndTag(out, WsmNamespaces::SOAP_ENVELOPE, STRLIT("Reason"));
    }

    appendEndTag(out, WsmNamespaces::SOAP_ENVELOPE, STRLIT("Fault"));
}

void WsmWriter::appendWsmFaultBody(
    Buffer& out,
    const WsmFault& fault)
{
    ContentLanguageList reasonLang = fault.getReasonLanguage();

    appendStartTag(out, WsmNamespaces::SOAP_ENVELOPE, STRLIT("Fault"));
    appendStartTag(out, WsmNamespaces::SOAP_ENVELOPE, STRLIT("Code"));
    appendTagValue(out, WsmNamespaces::SOAP_ENVELOPE, STRLIT("Value"),
        fault.getCode());
    appendStartTag(out, WsmNamespaces::SOAP_ENVELOPE, STRLIT("Subcode"));
    appendTagValue(out, WsmNamespaces::SOAP_ENVELOPE, STRLIT("Value"),
        fault.getSubcode());
    appendEndTag(out, WsmNamespaces::SOAP_ENVELOPE, STRLIT("Subcode"));
    appendEndTag(out, WsmNamespaces::SOAP_ENVELOPE, STRLIT("Code"));

    String reason = fault.getReason();
    if (reason.size() != 0)
    {
        String lang;
        if (reasonLang.size() == 0)
            lang = "en-US";
        else
            lang = reasonLang.getLanguageTag(0).toString();

        appendStartTag(out, WsmNamespaces::SOAP_ENVELOPE, STRLIT("Reason"));
        appendTagValue(out, WsmNamespaces::SOAP_ENVELOPE, STRLIT("Text"),
                       reason, "xml:lang", lang);
        appendEndTag(out, WsmNamespaces::SOAP_ENVELOPE, STRLIT("Reason"));
    }

    String detail = fault.getFaultDetail();
    if (detail.size() != 0)
    {
        appendStartTag(out, WsmNamespaces::SOAP_ENVELOPE, STRLIT("Detail"));
        appendTagValue(
            out, WsmNamespaces::WS_MAN, STRLIT("FaultDetail"), detail);
        appendEndTag(out, WsmNamespaces::SOAP_ENVELOPE, STRLIT("Detail"));
    }

    appendEndTag(out, WsmNamespaces::SOAP_ENVELOPE, STRLIT("Fault"));
}

void WsmWriter::appendInstanceElement(
    Buffer& out,
    const String& resourceUri,
    WsmInstance& instance,
    const char* ns,
    Boolean isEmbedded)
{
    size_t nsLength = strlen(ns);

    // Class opening element:
    _appendStartPrefix(out, ns);
    out << instance.getClassName();
    out << STRLIT(" xmlns:");
    out.append(ns, nsLength);
    out << STRLIT("=\"");
    out << WsmUtils::getRootResourceUri(resourceUri);
    out << STRLIT("/") << instance.getClassName();
    out << STRLIT("\"");

    // DSP0230, section 7.2.5.2. The property element MUST contain an
    // xsi:type attribute with the XSD type of the class of the instance
    // (see section 7.3.1).
    if (isEmbedded)
    {
        out << STRLIT(" ");
        out << WsmNamespaces::supportedNamespaces[
            WsmNamespaces::XML_SCHEMA_INSTANCE].localName;
        out << STRLIT(":type=\"");
        out << instance.getClassName();
        out << STRLIT("_Type\"");
    }
    out << STRLIT(">");
    _writeNewlineForReadability(out);

    // Sort properties before writing them out
    instance.sortProperties();

    // Properties:
    for (Uint32 i = 0, n = instance.getPropertyCount(); i < n; i++)
        appendPropertyElement(out, resourceUri, instance.getProperty(i), ns);

    // Class closing element:
    _appendEndPrefix(out, ns);
    out << instance.getClassName() << STRLIT(">");
    _writeNewlineForReadability(out);
}

void WsmWriter::appendPropertyElement(
    Buffer& out,
    const String& resourceUri,
    WsmProperty& property,
    const char* ns)
{
    WsmValue val = property.getValue();
    String propName = property.getName();

    // Form start element prefix. For example: "<class:"
    Buffer startBuffer;
    _appendStartPrefix(startBuffer, ns);
    StrLit start(startBuffer.getData(), startBuffer.size());

    // Form end element prefix. For example: "</class:"
    Buffer endBuffer;
    _appendEndPrefix(endBuffer, ns);
    StrLit end(endBuffer.getData(), endBuffer.size());

    if (val.isNull())
    {
        out << start << propName;
        out << " " << WsmNamespaces::supportedNamespaces[
            WsmNamespaces::XML_SCHEMA_INSTANCE].localName;
        out << STRLIT(":nil=\"true\"/>");
        return;
    }

    if (val.isArray())
    {
        switch (val.getType())
        {
            case WSMTYPE_REFERENCE:
            {
                Array<WsmEndpointReference> eprs;
                val.get(eprs);
                for (Uint32 i = 0, n = eprs.size(); i < n; i++)
                {
                    out << start << propName << STRLIT(">");
                    _writeNewlineForReadability(out);
                    appendEPRElement(out, eprs[i]);
                    out << end << propName << STRLIT(">");
                    _writeNewlineForReadability(out);
                }
                break;
            }
            case WSMTYPE_INSTANCE:
            {
                Array<WsmInstance> instances;
                val.get(instances);
                for (Uint32 i = 0, n = instances.size(); i < n; i++)
                {
                    out << start << propName << STRLIT(">");
                    _writeNewlineForReadability(out);
                    appendInstanceElement(out, resourceUri, instances[i],
                        PEGASUS_INSTANCE_NS, true);
                    out << end << propName << STRLIT(">");
                    _writeNewlineForReadability(out);
                }
                break;
            }
            case WSMTYPE_OTHER:
            {
                Array<String> strs;
                val.get(strs);
                for (Uint32 i = 0, n = strs.size(); i < n; i++)
                {
                    out << start << propName << STRLIT(">");
                    _writeNewlineForReadability(out);
                    appendStringElement(out, strs[i]);
                    out << end << propName << STRLIT(">");
                    _writeNewlineForReadability(out);
                }
                break;
            }
            default:
            {
                PEGASUS_UNREACHABLE(PEGASUS_ASSERT(0);)
            }
        }
    }
    else
    {
        switch (val.getType())
        {
            case WSMTYPE_REFERENCE:
            {
                WsmEndpointReference epr;
                val.get(epr);
                out << start << propName << STRLIT(">");
                _writeNewlineForReadability(out);
                appendEPRElement(out, epr);
                out << end << propName << STRLIT(">");
                _writeNewlineForReadability(out);
                break;
            }
            case WSMTYPE_INSTANCE:
            {
                WsmInstance instance;
                val.get(instance);
                out << start << propName << STRLIT(">");
                _writeNewlineForReadability(out);
                appendInstanceElement(out, resourceUri, instance,
                    PEGASUS_INSTANCE_NS, true);
                out << end << propName << STRLIT(">");
                _writeNewlineForReadability(out);
                break;
            }
            case WSMTYPE_OTHER:
            {
                String str;
                val.get(str);
                out << start << propName << STRLIT(">");
                _writeNewlineForReadability(out);
                appendStringElement(out, str);
                out << end << propName << STRLIT(">");
                _writeNewlineForReadability(out);
                break;
            }
            default:
            {
                PEGASUS_UNREACHABLE(PEGASUS_ASSERT(0);)
            }
        }
    }
}

void WsmWriter::appendEPRElement(
    Buffer& out,
    const WsmEndpointReference& epr)
{
    appendTagValue(
        out, WsmNamespaces::WS_ADDRESSING, STRLIT("Address"), epr.address);
    appendStartTag(
        out, WsmNamespaces::WS_ADDRESSING, STRLIT("ReferenceParameters"));
    appendTagValue(
        out, WsmNamespaces::WS_MAN, STRLIT("ResourceURI"), epr.resourceUri);

    if (epr.selectorSet && epr.selectorSet->selectors.size() > 0)
    {
        appendStartTag(out, WsmNamespaces::WS_MAN, STRLIT("SelectorSet"));
        for (Uint32 i = 0, n = epr.selectorSet->selectors.size(); i < n; i++)
        {
            WsmSelector selector = epr.selectorSet->selectors[i];
            appendStartTag(out, WsmNamespaces::WS_MAN, STRLIT("Selector"),
                "Name", selector.name);
            if (selector.type == WsmSelector::EPR)
            {
                appendStartTag(out,
                    WsmNamespaces::WS_ADDRESSING,
                    STRLIT("EndpointReference"));
                appendEPRElement(out, selector.epr);
                appendEndTag(out,
                    WsmNamespaces::WS_ADDRESSING,
                    STRLIT("EndpointReference"));
            }
            else
            {
                XmlGenerator::appendSpecial(out, selector.value);
                _writeNewlineForReadability(out);
            }
            appendEndTag(out, WsmNamespaces::WS_MAN, STRLIT("Selector"));
        }
        appendEndTag(out, WsmNamespaces::WS_MAN, STRLIT("SelectorSet"));
    }
    appendEndTag(
        out, WsmNamespaces::WS_ADDRESSING, STRLIT("ReferenceParameters"));
}

void WsmWriter::appendStringElement(
    Buffer& out,
    const String& str)
{
    XmlGenerator::appendSpecial(out, str);
    _writeNewlineForReadability(out);
}

void WsmWriter::appendStartTag(
    Buffer& out,
    WsmNamespaces::Type nsType,
    const StrLit& tagName,
    const char* attrName,
    const String& attrValue)
{
    out << STRLIT("<");
    out << WsmNamespaces::supportedNamespaces[nsType].localName;
    out << STRLIT(":") << tagName;
    if (attrName)
    {
        out << STRLIT(" ") << attrName << STRLIT("=");
        out << STRLIT("\"") << attrValue << STRLIT("\"");
    }
    out << STRLIT(">");
    _writeNewlineForReadability(out);
}

void WsmWriter::appendEndTag(
    Buffer& out,
    WsmNamespaces::Type nsType,
    const StrLit& tagName)
{
    out << STRLIT("</");
    out << WsmNamespaces::supportedNamespaces[nsType].localName;
    out << STRLIT(":") << tagName << STRLIT(">");
    _writeNewlineForReadability(out);
}

void WsmWriter::appendEmptyTag(
    Buffer& out,
    WsmNamespaces::Type nsType,
    const StrLit& tagName)
{
    out << STRLIT("<");
    out << WsmNamespaces::supportedNamespaces[nsType].localName;
    out << STRLIT(":") << tagName << STRLIT("/>");
    _writeNewlineForReadability(out);
}

void WsmWriter::appendTagValue(
    Buffer& out,
    WsmNamespaces::Type nsType,
    const StrLit& tagName,
    const String& value,
    const char* attrName,
    const String& attrValue)
{
    appendStartTag(out, nsType, tagName, attrName, attrValue);
    out << value;
    _writeNewlineForReadability(out);
    appendEndTag(out, nsType, tagName);
}

void WsmWriter::appendHTTPResponseHeader(
    Buffer& out,
    const String& action,
    HttpMethod httpMethod,
    Boolean omitXMLProcessingInstruction,
    const ContentLanguageList& contentLanguages,
    Boolean isFault,
    Uint32 contentLength)
{
    if (isFault)
    {
        out << STRLIT("HTTP/1.1 " HTTP_STATUS_INTERNALSERVERERROR "\r\n");
    }
    else
    {
        out << STRLIT("HTTP/1.1 " HTTP_STATUS_OK "\r\n");
    }

    out << STRLIT("Content-Type: application/soap+xml;charset=UTF-8\r\n");
    OUTPUT_CONTENTLENGTH(out, contentLength);

    if (contentLanguages.size() > 0)
    {
        out << STRLIT("Content-Language: ") << contentLanguages
            << STRLIT("\r\n");
    }
    if (httpMethod == HTTP_METHOD_M_POST)
    {
        char nn[] = { char('0'+(rand() % 10)), char('0'+(rand() % 10)),'\0' };

        out << STRLIT("Ext:\r\n");
        out << STRLIT("Cache-Control: no-cache\r\n");
        out << STRLIT("Man: urn:schemas-xmlsoap-org:soap.v1; ns=") << nn
            << STRLIT("\r\n");
        out << nn << STRLIT("-SOAPAction: ") << action << STRLIT("\r\n");
    }
    else
    {
        out << STRLIT("SOAPAction: ");
        out << action << STRLIT("\r\n");
    }
    out << STRLIT("\r\n");

    if (!omitXMLProcessingInstruction)
    {
        out << STRLIT("<?xml version=\"1.0\" encoding=\"utf-8\" ?>");
    }

    _writeNewlineForReadability(out);
}

void WsmWriter::appendSoapEnvelopeStart(
    Buffer& out,
    const ContentLanguageList& contentLanguages)
{
    out << STRLIT("<");
    out << WsmNamespaces::supportedNamespaces[
        WsmNamespaces::SOAP_ENVELOPE].localName;
    out << STRLIT(":Envelope");

    // DSP0226 R6.3-4: In any response, event, or singleton message, the
    // service should include the xml:lang attribute in the s:Envelope (or
    // other elements) to signal to the receiver that localized content
    // appears in the body of the message. This attribute may be omitted
    // if no descriptive content appears in the body. Including the
    // xml:lang attribute is not an error, even if no descriptive content
    // occurs.

    if (contentLanguages.size() > 0)
    {
        out << STRLIT(" xml:lang=\"");
        out << contentLanguages.getLanguageTag(0).toString() << STRLIT("\"");
    }

    for (Uint32 i = 0; i < WsmNamespaces::LAST; i++)
    {
        _writeNewlineForReadability(out);
        out << STRLIT(" xmlns:");
        out << WsmNamespaces::supportedNamespaces[i].localName;
        out << STRLIT("=\"");
        out << WsmNamespaces::supportedNamespaces[i].extendedName;
        out << STRLIT("\"");
    }
    out << STRLIT(">");
    _writeNewlineForReadability(out);
}

void WsmWriter::appendSoapEnvelopeEnd(Buffer& out)
{
    appendEndTag(out, WsmNamespaces::SOAP_ENVELOPE, STRLIT("Envelope"));
}

void WsmWriter::appendSoapHeaderStart(Buffer& out)
{
    appendStartTag(out, WsmNamespaces::SOAP_ENVELOPE, STRLIT("Header"));
}

void WsmWriter::appendSoapHeaderEnd(Buffer& out)
{
    appendEndTag(out, WsmNamespaces::SOAP_ENVELOPE, STRLIT("Header"));
}

void WsmWriter::appendSoapBodyStart(Buffer& out)
{
    appendStartTag(out, WsmNamespaces::SOAP_ENVELOPE, STRLIT("Body"));
}

void WsmWriter::appendSoapBodyEnd(Buffer& out)
{
    appendEndTag(out, WsmNamespaces::SOAP_ENVELOPE, STRLIT("Body"));
}

void WsmWriter::appendSoapHeader(
    Buffer& out,
    const String& action,
    const String& messageId,
    const String& relatesTo,
    const String& toAddress,
    const String& replyTo,
    const Boolean& ackRequired)
{
    // Add <wsa:To> entry
    appendStartTag(out, WsmNamespaces::WS_ADDRESSING, STRLIT("To"));

    if(toAddress.size())
    {
        out.append((const char *)toAddress.getCString(),toAddress.size());
    }
    else
    {
        out << STRLIT(WSM_ADDRESS_ANONYMOUS);
    }
    //writeNewlineForReadability(out);
    appendEndTag(out, WsmNamespaces::WS_ADDRESSING, STRLIT("To"));

    // Add <wsa:Action> entry
    appendTagValue(out, WsmNamespaces::WS_ADDRESSING, STRLIT("Action"), action);

    // Add <wsa:MessageID> entry
    appendTagValue(
        out, WsmNamespaces::WS_ADDRESSING, STRLIT("MessageID"), messageId);

    // Add <wsa:RelatesTo> entry only if necessary. An empty relates to tag
    // breaks some WS-Management clients (such as WinRM).
    if (relatesTo.size())
    {
        appendTagValue(
            out, WsmNamespaces::WS_ADDRESSING, STRLIT("RelatesTo"), relatesTo);
    }
    if(replyTo.size())
    {
        appendStartTag(out, WsmNamespaces::WS_ADDRESSING, STRLIT("ReplyTo"));
        appendTagValue(
            out,
            WsmNamespaces::WS_ADDRESSING,
            STRLIT("Address"),
            replyTo);
        appendEndTag(out, WsmNamespaces::WS_ADDRESSING, STRLIT("ReplyTo"));
    }
    if(ackRequired)
    {
        appendTagValue(out, WsmNamespaces::WS_MAN, STRLIT("AckRequested"), "");
    }

}

void WsmWriter::appendInvokeOutputElement(
    Buffer& out,
    const String& resourceUri,
    const String& className,
    const String& methodName,
    WsmInstance& instance,
    const char* ns)
{
    CString mn(methodName.getCString());

    // Form tagname.
    Buffer tagBuffer;
    tagBuffer.append(ns, strlen(ns));
    tagBuffer.append(':');
    tagBuffer.append(mn, strlen(mn));
    tagBuffer.append("_OUTPUT", 7);
    const char* tag = tagBuffer.getData();
    size_t tagSize = tagBuffer.size();

    // Write start tag.
    out.append('<');
    out.append(tag, tagSize);

    out << STRLIT(" xmlns:");
    out.append(ns, strlen(ns));
    out << STRLIT("=\"");
    out << WsmUtils::getRootResourceUri(resourceUri);
    out << STRLIT("/");
    out << className;
    out << STRLIT("\">");
    _writeNewlineForReadability(out);

    // Write properties:
    for (Uint32 i = 0, n = instance.getPropertyCount(); i < n; i++)
        appendPropertyElement(out, resourceUri, instance.getProperty(i), ns);

    // Write end tag.
    out << STRLIT("</");
    out.append(tag, tagSize);
    out.append('>');

    _writeNewlineForReadability(out);
}

Buffer  WsmWriter::appendHTTPRequestHeader(
    XmlParser& parser,
    const String& hostName,
    Boolean useMPost,
    Boolean useHTTP11,
    Buffer& content,
    Buffer& httpHeaders,
    const String& destination
    )
{
    XmlEntry entry;
    Buffer message;

    if (parser.next(entry) &&
        entry.type == XmlEntry::START_TAG &&
        strcmp(entry.localName, "Envelope") == 0)
    {
        //  Set WSMAN headers and content.
        message << HTTP_METHOD_POST << HTTP_SP
            << destination << HTTP_SP
            << HTTP_PROTOCOL << HTTP_VERSION_11 << HTTP_CRLF;
        message << HEADER_NAME_HOST << HEADER_SEPARATOR
            << HTTP_SP << hostName << HTTP_CRLF;
        message << HEADER_NAME_CONTENTTYPE << HEADER_SEPARATOR
            << HTTP_SP << WSMAN_HEADER_VALUE_CONTENTTYPE
            << HTTP_CRLF;
        message << HEADER_NAME_CONTENTLENGTH << HEADER_SEPARATOR
            << HTTP_SP << (Uint32)content.size () << HTTP_CRLF;

        httpHeaders << message;
        message << HTTP_CRLF;
        message << content;
    }
    return message;   
}

void WsmWriter::addAuthHeader(
    HTTPMessage*& httpMessage,
    AutoPtr<ClientAuthenticator>& authenticator)
{
    // Add authentication headers to the message
    String authHeader = authenticator->buildRequestAuthHeader();
    if (authHeader != String::EMPTY)
    {
        // Find the separator between the start line and the headers, so we
        // can add the authorization header there.

        const char* messageStart = httpMessage->message.getData();
        char* headerStart =
            (char*)memchr(messageStart, '\n', httpMessage->message.size());

        if (headerStart)
        {
            // Handle a CRLF or LF separator
            if ((headerStart != messageStart) && (headerStart[-1] == '\r'))
            {
                headerStart[-1] = 0;
            }
            else
            {
                *headerStart = 0;
            }

            headerStart++;

            // Build a new message with the original start line, the new
            // authorization header, and the original headers and content.

            Buffer newMessageBuffer;
            newMessageBuffer << messageStart << HTTP_CRLF;
            newMessageBuffer << authHeader << HTTP_CRLF;
            newMessageBuffer << headerStart;

            HTTPMessage* newMessage = new HTTPMessage(newMessageBuffer);
            delete httpMessage;
            httpMessage = newMessage;
        }
    }
}



PEGASUS_NAMESPACE_END
