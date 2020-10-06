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

#include <cctype>
#include <cstdio>
#include <cstdlib>

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/MessageLoader.h>
#include <Pegasus/Common/Buffer.h>
#include <Pegasus/Common/StringConversion.h>
#include <Pegasus/Common/XmlReader.h>
#include <Pegasus/Common/Tracer.h>
#include <Pegasus/WQL/WQLSelectStatement.h>
#include <Pegasus/WQL/WQLParser.h>
#include <Pegasus/WsmServer/WsmConstants.h>
#include <Pegasus/WsmServer/WsmFault.h>
#include "WsmReader.h"
#include <Pegasus/WsmServer/WsmToCimRequestMapper.h>
PEGASUS_NAMESPACE_BEGIN

#define PEGASUS_PROPERTYNAME_FILTER_CSTRING \
    PEGASUS_PROPERTYNAME_FILTER.getString().getCString()

///////////////////////////////////////////////////////////////////////////////
//
// WsmReader
//
///////////////////////////////////////////////////////////////////////////////

WsmReader::WsmReader(char* text)
    : _parser(text, WsmNamespaces::supportedNamespaces)
{
}

WsmReader::~WsmReader()
{
}

void WsmReader::setHideEmptyTags(Boolean flag)
{
    _parser.setHideEmptyTags(flag);
}

//-----------------------------------------------------------------------------
//
// getXmlDeclaration()
//
//     <?xml version="1.0" encoding="utf-8"?>
//
//-----------------------------------------------------------------------------
Boolean WsmReader::getXmlDeclaration(
    const char*& xmlVersion,
    const char*& xmlEncoding)
{
    XmlEntry entry;

    if (_parser.next(entry))
    {
        if ((entry.type != XmlEntry::XML_DECLARATION) ||
            (strcmp(entry.text, "xml") != 0))
        {
            _parser.putBack(entry);
            return false;
        }

        entry.getAttributeValue("version", xmlVersion);
        entry.getAttributeValue("encoding", xmlEncoding);
        return true;
    }

    return false;
}

Boolean WsmReader::testStartTag(
    XmlEntry& entry,
    int nsType,
    const char* tagName)
{
    if (!_parser.next(entry))
    {
        return false;
    }

    if ((entry.type != XmlEntry::START_TAG) ||
        (entry.nsType != nsType) ||
        (tagName && strcmp(entry.localName, tagName) != 0))
    {
        _parser.putBack(entry);
        return false;
    }

    return true;
}

Boolean WsmReader::testStartOrEmptyTag(
    XmlEntry& entry,
    int nsType,
    const char* tagName)
{
    if (!_parser.next(entry))
    {
        return false;
    }

    if (((entry.type != XmlEntry::START_TAG) &&
         (entry.type != XmlEntry::EMPTY_TAG)) ||
        (entry.nsType != nsType) ||
        (tagName && strcmp(entry.localName, tagName) != 0))
    {
        _parser.putBack(entry);
        return false;
    }

    return true;
}

Boolean WsmReader::testEndTag(
    int nsType,
    const char* tagName)
{
    XmlEntry entry;

    if (!_parser.next(entry))
    {
        return false;
    }

    if ((entry.type != XmlEntry::END_TAG) ||
        (entry.nsType != nsType) ||
        (tagName && strcmp(entry.localName, tagName) != 0))
    {
        _parser.putBack(entry);
        return false;
    }

    return true;
}

void WsmReader::expectStartTag(
    XmlEntry& entry,
    int nsType,
    const char* tagName)
{
    if (!_parser.next(entry) ||
        entry.type != XmlEntry::START_TAG ||
        entry.nsType != nsType ||
        strcmp(entry.localName, tagName) != 0)
    {
        const char* nsUri;

        // The nsType must have already been declared in the XML or it must be
        // a supported namespace.
        XmlNamespace* ns = _parser.getNamespace(nsType);
        if (ns)
        {
            nsUri = ns->extendedName;
        }
        else
        {
            PEGASUS_ASSERT((nsType >= 0) && (nsType < WsmNamespaces::LAST));
            nsUri = WsmNamespaces::supportedNamespaces[nsType].extendedName;
        }

        MessageLoaderParms mlParms(
            "WsmServer.WsmReader.EXPECTED_OPEN",
            "Expecting a start tag for \"$0\" element in namespace \"$1\".",
            tagName, nsUri);
        throw XmlValidationError(_parser.getLine(), mlParms);
    }
}

int WsmReader::expectStartTag(
    XmlEntry& entry,
    const char* tagName)
{
    if (!_parser.next(entry) ||
        entry.type != XmlEntry::START_TAG ||
        strcmp(entry.localName, tagName) != 0)
    {
        const char* nsUri;
        int nsType = entry.nsType;

        // The nsType must have already been declared in the XML or it must be
        // a supported namespace.
        XmlNamespace* ns = _parser.getNamespace(nsType);
        if (ns)
        {
            nsUri = ns->extendedName;
        }
        else
        {
            PEGASUS_ASSERT((nsType >= 0) && (nsType < WsmNamespaces::LAST));
            nsUri = WsmNamespaces::supportedNamespaces[nsType].extendedName;
        }

        MessageLoaderParms mlParms(
            "WsmServer.WsmReader.EXPECTED_OPEN",
            "Expecting a start tag for \"$0\" element in namespace \"$1\".",
            tagName, nsUri);
        throw XmlValidationError(_parser.getLine(), mlParms);
    }

    return entry.nsType;
}

void WsmReader::expectStartOrEmptyTag(
    XmlEntry& entry,
    int nsType,
    const char* tagName)
{
    if (!_parser.next(entry) ||
        (entry.type != XmlEntry::START_TAG &&
         entry.type != XmlEntry::EMPTY_TAG) ||
        entry.nsType != nsType ||
        strcmp(entry.localName, tagName) != 0)
    {
        const char* nsUri;

        // The nsType must have already been declared in the XML or it must be
        // a supported namespace.
        XmlNamespace* ns = _parser.getNamespace(nsType);
        if (ns)
        {
            nsUri = ns->extendedName;
        }
        else
        {
            PEGASUS_ASSERT((nsType >= 0) && (nsType < WsmNamespaces::LAST));
            nsUri = WsmNamespaces::supportedNamespaces[nsType].extendedName;
        }

        MessageLoaderParms mlParms(
            "WsmServer.WsmReader.EXPECTED_OPENCLOSE",
            "Expecting a start tag or an empty tag for \"$0\" element in "
                "namespace \"$1\".",
            tagName, nsUri);
        throw XmlValidationError(_parser.getLine(), mlParms);
    }
}

void WsmReader::expectEndTag(
    int nsType,
    const char* tagName)
{
    XmlEntry entry;

    if (!_parser.next(entry) ||
        entry.type != XmlEntry::END_TAG ||
        entry.nsType != nsType ||
        strcmp(entry.localName, tagName) != 0)
    {
        // The nsType must have already been declared in the XML.  (Note that
        // ns->localName is null for a default namespace declaration.)
        XmlNamespace* ns = _parser.getNamespace(nsType);
        PEGASUS_ASSERT(ns);
        MessageLoaderParms mlParms(
            "WsmServer.WsmReader.EXPECTED_CLOSE",
            "Expecting an end tag for \"$0\" element in namespace \"$1\".",
            tagName, ns->extendedName);
        throw XmlValidationError(_parser.getLine(), mlParms);
    }
}

void WsmReader::expectContentOrCData(XmlEntry& entry)
{
    XmlReader::expectContentOrCData(_parser, entry);
}

Boolean WsmReader::next(XmlEntry& entry)
{
    return _parser.next(entry);
}

Boolean WsmReader::getAttributeValue(
    Uint32 lineNumber,
    XmlEntry& entry,
    const char* attributeName,
    String& attributeValue,
    Boolean required)
{
    if (!entry.getAttributeValue(attributeName, attributeValue))
    {
        if (required)
        {
            MessageLoaderParms parms(
                "WsmServer.WsmReader.MISSING_ATTRIBUTE",
                "The attribute $0.$1 is missing.",
                entry.text,
                attributeName);
            throw XmlValidationError(lineNumber, parms);
        }

        return false;
    }

    return true;
}

Boolean WsmReader::mustUnderstand(XmlEntry& entry)
{
    const XmlAttribute* attr = entry.findAttribute(
        WsmNamespaces::SOAP_ENVELOPE, "mustUnderstand");
    return (attr && (strcmp(attr->value, "true") == 0));
}

Boolean WsmReader::getElementStringValue(
    int nsType,
    const char* tagName,
    String& stringValue,
    Boolean required)
{
    XmlEntry entry;

    if (required)
    {
        expectStartTag(entry, nsType, tagName);
    }
    else if (!testStartTag(entry, nsType, tagName))
    {
        return false;
    }

    expectContentOrCData(entry);
    stringValue.assign(entry.text, entry.textLen);
    expectEndTag(nsType, tagName);
    return true;
}

const char* WsmReader::getElementContent(XmlEntry& entry)
{
    if (entry.type == XmlEntry::EMPTY_TAG)
    {
        return "";
    }

    expectContentOrCData(entry);
    return entry.text;
}

Uint64 WsmReader::getEnumerationContext(XmlEntry& entry)
{
    const char* content = getElementContent(entry);
    if (*content == '+')
    {
        content++;
    }

    Uint64 value;
    if (!StringConversion::decimalStringToUint64(content, value))
    {
        throw WsmFault(
            WsmFault::wsen_InvalidEnumerationContext,
            MessageLoaderParms(
                "WsmServer.WsmReader.INVALID_ENUMERATION_CONTEXT",
                "Enumeration context \"$1\" is not valid.",
                content));
    }
    return value;
}

Uint32 WsmReader::getUint32ElementContent(XmlEntry& entry, const char* name)
{
    const char* content = getElementContent(entry);
    if (*content == '+')
    {
        content++;
    }

    Uint64 value;
    if (!StringConversion::decimalStringToUint64(content, value) ||
        (value == 0) || (value > 0xFFFFFFFF))
    {
        throw WsmFault(
            WsmFault::wsa_InvalidMessageInformationHeader,
            MessageLoaderParms(
                "WsmServer.WsmReader.INVALID_UINT32_VALUE",
                "The $0 value \"$1\" is not a valid "
                "positive integer.",
                name, content));
    }

    return value & 0xFFFFFFFF;
}

Boolean WsmReader::getSelectorElement(WsmSelector& selector)
{
    XmlEntry entry;
    if (!testStartOrEmptyTag(entry, WsmNamespaces::WS_MAN, "Selector"))
    {
        return false;
    }

    Boolean empty = entry.type == XmlEntry::EMPTY_TAG;
    String name;

    getAttributeValue(_parser.getLine(), entry, "Name", name);

    if (empty)
    {
        selector = WsmSelector(name, String::EMPTY);
    }
    else
    {
        WsmEndpointReference endpointReference;

        if (getSelectorEPRElement(endpointReference))
        {
            selector = WsmSelector(name, endpointReference);
        }
        else
        {
            expectContentOrCData(entry);
            selector = WsmSelector(name, String(entry.text, entry.textLen));
        }

        expectEndTag(WsmNamespaces::WS_MAN, "Selector");
    }

    return true;
}

Boolean WsmReader::getSelectorSetElement(WsmSelectorSet& selectorSet)
{
    XmlEntry entry;
    if (!testStartTag(entry, WsmNamespaces::WS_MAN, "SelectorSet"))
    {
        return false;
    }

    selectorSet.selectors.clear();

    WsmSelector selector;

    while (getSelectorElement(selector))
    {
        selectorSet.selectors.append(selector);
    }

    // If the selector set is empty, report an error
    if (selectorSet.selectors.size() == 0)
    {
        expectStartTag(entry, WsmNamespaces::WS_MAN, "Selector");
    }

    // Note: This "should" requirement is not implemented.
    // DSP0226 R5.1.2.2-4: The Selector Name attribute shall not be duplicated
    // at the same level of nesting.  If this occurs, the service should return
    // a wsman:InvalidSelectors fault with the following detail code:
    //     http://schemas.dmtf.org/wbem/wsman/1/wsman/faultDetail/
    //         DuplicateSelectors

    expectEndTag(WsmNamespaces::WS_MAN, "SelectorSet");

    return true;
}

void WsmReader::getEPRElement(WsmEndpointReference& endpointReference)
{
    XmlEntry entry;

    // DSP0227 Section 5. WS-Management default addressing model is based
    // on WS-Addressing. It makes use of wsa:ReferenceParameter field
    // (containing wsa:ResourceURI and wsa:SelectorSet) to identify objects.
    // The use of other fields defined by WS-Addressing in not specified.

    // wsa:Address is required by WS-Addressing
    getElementStringValue(
        WsmNamespaces::WS_ADDRESSING,
        "Address",
        endpointReference.address,
        true);

    expectStartTag(entry, WsmNamespaces::WS_ADDRESSING, "ReferenceParameters");

    // Though DSP0227 does not explicitly require wsa:ResourceURI to be present,
    // valid targets of operations must either have a class specific
    // ResourceURI to target classes/instances or all classes URI to target
    // the service itself.
    getElementStringValue(
        WsmNamespaces::WS_MAN,
        "ResourceURI",
        endpointReference.resourceUri,
        true);

    // wsa:SelectorSet is optional
    if (testStartTag(entry, WsmNamespaces::WS_MAN, "SelectorSet"))
    {
        _parser.putBack(entry);
        // Return value ignored; assumed to succeed because of precheck
        getSelectorSetElement(*endpointReference.selectorSet);
    }

    expectEndTag(WsmNamespaces::WS_ADDRESSING, "ReferenceParameters");
}

Boolean WsmReader::getSelectorEPRElement(
    WsmEndpointReference& endpointReference)
{
    // EPRs in selectors have enclosing wsa:EndpointReference tags
    XmlEntry entry;
    if (!testStartTag(
            entry, WsmNamespaces::WS_ADDRESSING, "EndpointReference"))
    {
        return false;
    }

    getEPRElement(endpointReference);
    expectEndTag(WsmNamespaces::WS_ADDRESSING, "EndpointReference");
    return true;
}


Boolean WsmReader::getInstanceEPRElement(
    WsmEndpointReference& endpointReference)
{
    XmlEntry entry;

    if (!testStartTag(entry, WsmNamespaces::WS_ADDRESSING, "Address"))
    {
        return false;
    }
    else
    {
        _parser.putBack(entry);
    }

    getEPRElement(endpointReference);
    return true;
}

void WsmReader::skipElement(XmlEntry& entry)
{
    const char* elementName = entry.text;

    if (entry.type == XmlEntry::EMPTY_TAG)
    {
        return;
    }

    while (XmlReader::testStartTagOrEmptyTag(_parser, entry))
    {
        skipElement(entry);
    }

    // Skip content data, if present
    XmlReader::testContentOrCData(_parser, entry);

    XmlReader::expectEndTag(_parser, elementName);
}

// checkDuplicateHeader.  It is a duplicate if the isDuplicate parameter
// is true
void WsmReader:: checkDuplicateHeader(
    const char* elementName,
    Boolean isDuplicate)
{
    // DSP0226 R13.1-9: If a request contains multiple SOAP headers with the
    // same QName from WS-Management, WS-Addressing, or WS-Eventing, the
    // service should not process them and should issue a
    // wsa:InvalidMessageInformationHeaders fault if they are detected.
    // (No SOAP headers are defined by the WS-Transfer and WS-Enumeration
    // specifications.)

    if (isDuplicate)
    {
        throw WsmFault(
            WsmFault::wsa_InvalidMessageInformationHeader,
            MessageLoaderParms(
                "WsmServer.WsmReader.DUPLICATE_SOAP_HEADER",
                "Request contains multiple $0 SOAP Header elements.",
                elementName));
    }
}

void WsmReader::decodeRequestSoapHeaders(
    String& wsaMessageId,
    String& wsaTo,
    String& wsaAction,
    String& wsaFrom,
    String& wsaReplyTo,
    String& wsaFaultTo,
    String& wsmResourceUri,
    WsmSelectorSet& wsmSelectorSet,
    Uint32& wsmMaxEnvelopeSize,
    AcceptLanguageList& wsmLocale,
    Boolean& wsmRequestEpr,
    Boolean& wsmRequestItemCount,
    String& wseIdentifier)
{
    // Note: This method does not collect headers that should appear only in
    // responses: wsa:RelatesTo, wsman:RequestedEPR.

    XmlEntry entry;
    Boolean gotEntry;

    // The wsidentify operation may send an empty header element.
    _parser.setHideEmptyTags(true);
    expectStartTag(entry, WsmNamespaces::SOAP_ENVELOPE, "Header");
    _parser.setHideEmptyTags(false);

    while ((gotEntry = _parser.next(entry)) &&
           ((entry.type == XmlEntry::START_TAG) ||
            (entry.type == XmlEntry::EMPTY_TAG)))
    {
        int nsType = entry.nsType;
        const char* elementName = entry.localName;
        Boolean needEndTag = (entry.type == XmlEntry::START_TAG);

        if ((nsType == WsmNamespaces::WS_ADDRESSING) &&
            (strcmp(elementName, "To") == 0))
        {
            checkDuplicateHeader(entry.text, wsaTo.size());
            wsaTo = getElementContent(entry);
        }
        else if ((nsType == WsmNamespaces::WS_ADDRESSING) &&
            (strcmp(elementName, "From") == 0))
        {
            checkDuplicateHeader(entry.text, wsaFrom.size());
            wsaFrom = getElementContent(entry);
        }
        else if ((nsType == WsmNamespaces::WS_ADDRESSING) &&
            (strcmp(elementName, "ReplyTo") == 0))
        {
            checkDuplicateHeader(entry.text, wsaReplyTo.size());
            // ATTN: Reference parameters not handled yet.
            // DSP0226 R5.4.2-5: Any reference parameters supplied in the
            // wsa:ReplyTo address shall be included in the actual response
            // message as top-level headers as specified in WS-Addressing
            // unless the response is a fault.  If the response is a fault,
            // the service should include the reference parameters but may
            // omit these values if the resulting message size would exceed
            // the encoding limits.
            getElementStringValue(
                WsmNamespaces::WS_ADDRESSING, "Address", wsaReplyTo, true);
        }
        else if ((nsType == WsmNamespaces::WS_ADDRESSING) &&
            (strcmp(elementName, "FaultTo") == 0))
        {
            checkDuplicateHeader(entry.text, wsaFaultTo.size());
            // ATTN: Reference parameters not handled yet.
            // DSP0226 R5.4.3-4: Any reference parameters supplied in the
            // wsa:FaultTo address should be included as top-level headers in
            // the actual fault, as specified in the WS-Addressing
            // specification.  In some cases, including this information would
            // cause the fault to exceed encoding size limits, and thus may be
            // omitted in those cases.
            getElementStringValue(
                WsmNamespaces::WS_ADDRESSING, "Address", wsaFaultTo, true);
        }
        else if ((nsType == WsmNamespaces::WS_ADDRESSING) &&
           (strcmp(elementName, "Action") == 0))
        {
            checkDuplicateHeader(entry.text, wsaAction.size());
            wsaAction = getElementContent(entry);
        }

        else if ((nsType == WsmNamespaces::WS_ADDRESSING) &&
            (strcmp(elementName, "MessageID") == 0))
        {
            checkDuplicateHeader(entry.text, wsaMessageId.size());
            wsaMessageId = getElementContent(entry);
        }
        else if ((nsType == WsmNamespaces::WS_MAN) &&
            (strcmp(elementName, "ResourceURI") == 0))
        {
            checkDuplicateHeader(entry.text, wsmResourceUri.size());
            wsmResourceUri = getElementContent(entry);
        }
        else if ((nsType == WsmNamespaces::WS_MAN) &&
            (strcmp(elementName, "SelectorSet") == 0))
        {

            checkDuplicateHeader(entry.text, wsmSelectorSet.selectors.size());
            _parser.putBack(entry);
            getSelectorSetElement(wsmSelectorSet);
            // The end tag has already been consumed.
            needEndTag = false;
        }
        else if ((nsType == WsmNamespaces::WS_MAN) &&
            (strcmp(elementName, "OperationTimeout") == 0))
        {
            if (mustUnderstand(entry))
            {
                // DSP0226 R6.1-3: If the service does not support user-defined
                // timeouts, a wsman:UnsupportedFeature fault should be
                // returned with the following detail code:
                //     http://schemas.dmtf.org/wbem/wsman/faultDetail/
                //         OperationTimeout

                throw WsmFault(
                    WsmFault::wsman_UnsupportedFeature,
                    String::EMPTY,
                    ContentLanguageList(),
                    WSMAN_FAULTDETAIL_OPERATIONTIMEOUT);
            }

            // Note: It is not an error for a compliant service to ignore the
            // timeout value or treat it as a hint if mustUnderstand is omitted.

            skipElement(entry);
            // The end tag, if any, has already been consumed.
            needEndTag = false;
        }
        else if ((nsType == WsmNamespaces::WS_MAN) &&
            (strcmp(elementName, "MaxEnvelopeSize") == 0))
        {
            checkDuplicateHeader(entry.text, wsmMaxEnvelopeSize > 0);

            // DSP0226 R6.2-3: If the mustUnderstand attribute is set to
            // "false", the service may ignore the header.
            wsmMaxEnvelopeSize =
                getUint32ElementContent(entry, "MaxEnvelopeSize");

            // DSP0226 R6.2-4:  Services should reject any MaxEnvelopeSize
            // value less than 8192 octets.  This number is the safe minimum
            // in which faults can be reliably encoded for all character sets.
            // If the requested size is less than this, the service should
            // return a wsman:EncodingLimit fault with the following detail
            // code:
            //     http://schemas.dmtf.org/wbem/wsman/1/wsman/faultDetail/
            //         MinimumEnvelopeLimit

            if (wsmMaxEnvelopeSize < WSM_MIN_MAXENVELOPESIZE_VALUE)
            {
                throw WsmFault(
                    WsmFault::wsman_EncodingLimit,
                    MessageLoaderParms(
                        "WsmServer.WsmReader.MAXENVELOPESIZE_TOO_SMALL",
                        "The MaxEnvelopeSize $0 is less than "
                            "minimum allowable value of $1.",
                        wsmMaxEnvelopeSize, WSM_MIN_MAXENVELOPESIZE_VALUE),
                    WSMAN_FAULTDETAIL_MINIMUMENVELOPELIMIT);
            }
        }
        else if ((nsType == WsmNamespaces::WS_MAN) &&
            (strcmp(elementName, "Locale") == 0))
        {
            checkDuplicateHeader(entry.text, wsmLocale.size());

            if (mustUnderstand(entry))
            {
                // DSP0226 R6.3-2: If the mustUnderstand attribute is set to
                // "true", the service shall ensure that the replies contain
                // localized information where appropriate, or else the service
                // shall issue a wsman:UnsupportedFeature fault with the
                // following detail code:
                //     http://schemas.dmtf.org/wbem/wsman/1/wsman/faultDetail/
                //         Locale
                // A service may always fault if wsman:Locale contains
                // s:mustUnderstand set to "true", because it may not be able
                // to ensure that the reply is localized.

                throw WsmFault(
                    WsmFault::wsman_UnsupportedFeature,
                    MessageLoaderParms(
                        "WsmServer.WsmReader.LOCALE_NOT_ENSURED",
                        "Translation of responses to a specified locale "
                            "cannot be ensured."),
                    WSMAN_FAULTDETAIL_LOCALE);
            }

            // DSP0226 R6.3-1 If the mustUnderstand attribute is omitted or set
            // to "false", the service should use this value when composing the
            // response message and adjust any localizable values accordingly.
            // This use is recommended for most cases. The locale is treated as
            // a hint in this case.

            // DSP0226 R6.3-3: The value of the xml:lang attribute in the
            // wsman:Locale header shall be a valid RFC 3066 language code.

            String languageTag;
            if (getAttributeValue(
                    _parser.getLine(), entry, "xml:lang", languageTag, false))
            {
                wsmLocale.insert(LanguageTag(languageTag), Real32(1.0));
            }
        }
        else if ((nsType == WsmNamespaces::WS_MAN) &&
            (strcmp(elementName, "OptionSet") == 0))
        {
            // DSP0226 R6.4-3: If the mustUnderstand attribute is omitted from
            // the OptionSet block, the service may ignore the entire
            // wsman:OptionSet block. If it is present and the service does not
            // support wsman:OptionSet, the service shall return a
            // s:NotUnderstood fault.

            if (mustUnderstand(entry))
            {
                throw SoapNotUnderstoodFault(
                    _parser.getNamespace(nsType)->extendedName, elementName);
            }

            skipElement(entry);
            // The end tag, if any, has already been consumed.
            needEndTag = false;
        }
        else if ((nsType == WsmNamespaces::WS_MAN) &&
            (strcmp(elementName, "RequestEPR") == 0))
        {
            checkDuplicateHeader(entry.text, wsmRequestEpr);
            wsmRequestEpr = true;
        }
        else if ((nsType == WsmNamespaces::WS_MAN) &&
            (strcmp(elementName, "RequestTotalItemsCountEstimate") == 0))
        {
            checkDuplicateHeader(entry.text, wsmRequestItemCount);
            wsmRequestItemCount = true;
        }
        else if ((nsType == WsmNamespaces::WS_MAN) &&
            (strcmp(elementName, "FragmentTransfer") == 0))
        {
            // DSP0226 R7.7-1: A conformant service may support fragment-level
            // WS-Transfer.  If the service supports fragment-level access, the
            // service shall not behave as if normal WS-Transfer operations
            // were in place but shall operate exclusively on the fragments
            // specified.  If the service does not support fragment-level
            // access, it shall return a wsman:UnsupportedFeature fault with
            // the following detail code:
            //     http://schemas.dmtf.org/wbem/wsman/1/wsman/faultDetail/
            //         FragmentLevelAccess

            if (mustUnderstand(entry))
            {
                throw WsmFault(
                    WsmFault::wsman_UnsupportedFeature,
                    String::EMPTY,
                    ContentLanguageList(),
                    WSMAN_FAULTDETAIL_FRAGMENTLEVELACCESS);
            }

            skipElement(entry);
            // The end tag, if any, has already been consumed.
            needEndTag = false;
        }
        else if ((nsType == WsmNamespaces::WS_EVENTING) &&
            (strcmp(elementName, "Identifier") == 0))
        {
            checkDuplicateHeader(entry.text, wseIdentifier.size());

            wseIdentifier = getElementContent(entry);
        }
        else if (mustUnderstand(entry))
        {
            // DSP0226 R5.2-2: If a service cannot comply with a header
            // marked with mustUnderstand="true", it shall issue an
            // s:NotUnderstood fault.
            XmlNamespace* ns = _parser.getNamespace(nsType);
            if (ns)
            {
                throw SoapNotUnderstoodFault(ns->extendedName, elementName);
            }
            else
            {
                throw SoapNotUnderstoodFault(String::EMPTY, elementName);
            }
        }
        else
        {
            skipElement(entry);
            // The end tag, if any, has already been consumed.
            needEndTag = false;
        }

        if (needEndTag)
        {
            expectEndTag(nsType, elementName);
        }
    }

    if (gotEntry)
    {
        _parser.putBack(entry);
    }

    expectEndTag(WsmNamespaces::SOAP_ENVELOPE, "Header");
}

void WsmReader::getInstanceElement(WsmInstance& instance)
{
    XmlEntry entry;
    Boolean isInstanceElement = false;
    int nsClassType = 0;
    const char* classNameTag = 0;

    // The first tag should be the class name start tag.
    if (_parser.next(entry) && (entry.type == XmlEntry::START_TAG))
    {
        XmlNamespace* ns = _parser.getNamespace(entry.nsType);
        classNameTag = entry.localName;

        // Class namespace must be of the following form:
        // http://schemas.dmtf.org/wbem/wscim/1/cim-schema/2/<class name>
        if (ns)
        {
            const char* suffix = WsmUtils::skipHostUri(ns->extendedName);
            const char* pos = strrchr(suffix, '/');
            if ((pos == suffix +
                sizeof(WSM_RESOURCEURI_CIMSCHEMAV2_SUFFIX) - 1) &&
                (strncmp(suffix, WSM_RESOURCEURI_CIMSCHEMAV2_SUFFIX,
                    sizeof(WSM_RESOURCEURI_CIMSCHEMAV2_SUFFIX) - 1) == 0) &&
                (strcmp(pos + 1, classNameTag) == 0))
            {
                // All properties must be qualified with the class namespace
                nsClassType = entry.nsType;
                instance.setClassName(String(pos + 1));
                isInstanceElement = true;
            }
        }
    }

    if (!isInstanceElement)
    {
        MessageLoaderParms mlParms(
            "WsmServer.WsmReader.EXPECTED_INSTANCE_ELEMENT",
            "Expecting an instance element.");
        throw XmlValidationError(_parser.getLine(), mlParms);
    }

    String propName;
    WsmValue propValue;
    while (getPropertyElement(nsClassType, propName, propValue))
    {
        instance.addProperty(WsmProperty(propName, propValue));
    }

    expectEndTag(nsClassType, classNameTag);
}


Boolean WsmReader::getPropertyElement(
    int nsType,
    String& propName,
    WsmValue& propValue)
{
    XmlEntry entry;
    const char* propNameTag = 0;
    Boolean firstTime = true;

    // Initialize propValue to a null value string
    propValue = WsmValue();

    // The first time we enter this loop, propNameTag is NULL. It will
    // give us any tag name in the given namespace. Subsequent iterations
    // will get entries with the same name as retrieved in the first
    // iteration.
    while (testStartOrEmptyTag(entry, nsType, propNameTag))
    {
        // Look for xsi:nil="true" to determine of this entry represents
        // a null value.
        const XmlAttribute* attr =
            entry.findAttribute(WsmNamespaces::XML_SCHEMA_INSTANCE, "nil");
        Boolean isNilValue = (attr && strcmp(attr->value, "true") == 0);

        // The only time it's OK to see a null entry is on the first iteration.
        // The above indicates a null property value. If a null entry appears
        // in a sequence representing an array, it's an error.
        if ((isNilValue || propValue.isNull()) && !firstTime)
        {
            MessageLoaderParms mlParms(
                "WsmServer.WsmReader.NULL_ARRAY_ELEMENTS",
                "Setting array elements to null is not supported.");
            throw XmlValidationError(_parser.getLine(), mlParms);
        }

        // If we haven't found xsi:nil="true" attribute and this is
        // an empty tag, throw an exception: invalid property value.
        if (entry.type == XmlEntry::EMPTY_TAG && !isNilValue)
        {
            MessageLoaderParms mlParms(
                "WsmServer.WsmReader.MISSING_PROPERTY_VALUE",
                "No value specified for non-null property $0.",
                entry.text);
            throw XmlValidationError(_parser.getLine(), mlParms);
        }

        if (firstTime)
        {
            firstTime = false;

            // Set the property name
            propNameTag = entry.localName;
            propName = propNameTag;

            if (isNilValue)
            {
                // A null element should be either an empty tag or a start
                // tag followed by an end tag (no content).
                if (entry.type == XmlEntry::START_TAG)
                {
                    expectEndTag(nsType, propNameTag);
                }

                // No need to set propValue - it's initialized to a null
                // string value
            }
            else
            {
                PEGASUS_ASSERT(entry.type == XmlEntry::START_TAG);
                getValueElement(propValue, nsType, propNameTag);
            }
        }
        else
        {
            PEGASUS_ASSERT(entry.type == XmlEntry::START_TAG);
            WsmValue val;
            getValueElement(val, nsType, propNameTag);
            propValue.add(val);
        }
    }

    // If we never entered the main loop, it means that we haven't seen the
    // right initial tags to indicate a class property. In this case firstTime
    // is still set to true.
    return !firstTime;
}

void WsmReader::getValueElement(
    WsmValue& value, int nsType, const char* propNameTag)
{
    XmlEntry entry;

    if (testEndTag(nsType, propNameTag))
    {
        // It's an empty tag, enter and empty sting value
        value.set(String::EMPTY);
    }
    else
    {
        if (XmlReader::testContentOrCData(_parser, entry))
        {
            // Simple string content
            value.set(String(entry.text, entry.textLen));
        }
        else
        {
            // The value is either an EPR or an embedded instance.
            WsmEndpointReference epr;
            if (getInstanceEPRElement(epr))
            {
                value.set(epr);
            }
            else
            {
                // This must be an embedded instance
                WsmInstance instance;
                getInstanceElement(instance);
                value.set(instance);
            }
        }
        expectEndTag(nsType, propNameTag);
    }
}

void WsmReader::decodeEnumerateBody(
    String& expiration,
    WsmbPolymorphismMode& polymorphismMode,
    WsenEnumerationMode& enumerationMode,
    Boolean& optimized,
    Uint32& maxElements,
    WsmFilter& wsmFilter)
{
   PEG_METHOD_ENTER(TRC_WSMSERVER, "WsmReader::decodeEnumerateBody()");
    XmlEntry entry;
    expectStartOrEmptyTag(
        entry, WsmNamespaces::WS_ENUMERATION, "Enumerate");
    if (entry.type != XmlEntry::EMPTY_TAG)
    {
        Boolean gotEntry;
        while ((gotEntry = _parser.next(entry)) &&
               ((entry.type == XmlEntry::START_TAG) ||
                (entry.type == XmlEntry::EMPTY_TAG)))
        {
            int nsType = entry.nsType;
            const char* elementName = entry.localName;
            Boolean needEndTag = (entry.type == XmlEntry::START_TAG);

            if ((nsType == WsmNamespaces::WS_ENUMERATION) &&
                (strcmp(elementName, "EndTo") == 0))
            {
                // DSP0226 R5.2.1-1: A conformant service is NOT REQUIRED to
                // accept a wsen:Enumerate message with an EndTo address as
                // R5.1-4 recommends not supporting the EndEnumerate message,
                // and may issue a wsman:UnsupportedFeature fault with a detail
                // code:
                //     http://schemas.dmtf.org/wbem/wsman/1/wsman/
                //         faultDetail/AddressingMode
                PEG_METHOD_EXIT();
                throw WsmFault(
                    WsmFault::wsman_UnsupportedFeature,
                    MessageLoaderParms(
                        "WsmServer.WsmReader.ENUMERATE_END_TO_UNSUPPORTED",
                        "Alternate destinations for EnumerationEnd messages "
                        "are not supported."),
                    WSMAN_FAULTDETAIL_ADDRESSINGMODE);
            }
            else if ((nsType == WsmNamespaces::WS_ENUMERATION) &&
                (strcmp(elementName, "Expires") == 0))
            {
                checkDuplicateHeader(entry.text, expiration.size());
                expiration = getElementContent(entry);
            }
            else if ((nsType == WsmNamespaces::WS_MAN) &&
                (strcmp(elementName, PEGASUS_PROPERTYNAME_FILTER_CSTRING) == 0))
            {
                // R8.2.1-3: The wsman:Filter element (see 8.3) in the
                // Enumerate body shall be either simple text or a single
                // complex XML element. A conformant service shall not accept
                // mixed content of both text and elements, or multiple peer
                // XML elements under the wsman:Filter element.
                // Duplicate if filter type already set
                checkDuplicateHeader(entry.text,
                    wsmFilter.filterDialect != WsmFilter::NONE);

                _parser.putBack(entry);
                decodeFilter(wsmFilter);
                needEndTag = false;
            }
            else if ((nsType == WsmNamespaces::WS_MAN) &&
                (strcmp(elementName, "OptimizeEnumeration") == 0))
            {
                checkDuplicateHeader(entry.text, optimized);
                optimized = true;
            }
            else if ((nsType == WsmNamespaces::WS_MAN) &&
                (strcmp(elementName, "MaxElements") == 0))
            {
                checkDuplicateHeader(entry.text, maxElements > 0);
                maxElements = getUint32ElementContent(entry, "MaxElements");
            }
            else if ((nsType == WsmNamespaces::WS_MAN) &&
                (strcmp(elementName, "EnumerationMode") == 0))
            {
                checkDuplicateHeader(entry.text,
                    enumerationMode != WSEN_EM_UNKNOWN);
                const char* content = getElementContent(entry);
                if (strcmp(content, "EnumerateEPR") == 0)
                {
                    enumerationMode = WSEN_EM_EPR;
                }
                else if (strcmp(content, "EnumerateObjectAndEPR") == 0)
                {
                    enumerationMode = WSEN_EM_OBJECT_AND_EPR;
                }
                else
                {
                    PEG_METHOD_EXIT();
                    throw WsmFault(
                        WsmFault::wsman_UnsupportedFeature,
                        MessageLoaderParms(
                            "WsmServer.WsmReader.ENUMERATE_"
                                "ENUM_MODE_UNSUPPORTED",
                            "Enumeration mode \"$0\" is not supported.",
                            content),
                        WSMAN_FAULTDETAIL_ENUMERATION_MODE_UNSUPPORTED);
                }
            }
            else if ((nsType == WsmNamespaces::WS_CIM_BINDING) &&
                (strcmp(elementName, "PolymorphismMode") == 0))
            {
                checkDuplicateHeader(entry.text,
                    polymorphismMode != WSMB_PM_UNKNOWN);
                const char* content = getElementContent(entry);
                if (strcmp(content, "ExcludeSubClassProperties") == 0)
                {
                    polymorphismMode = WSMB_PM_EXCLUDE_SUBCLASS_PROPERTIES;
                }
                else if (strcmp(content, "IncludeSubClassProperties") == 0)
                {
                    polymorphismMode = WSMB_PM_INCLUDE_SUBCLASS_PROPERTIES;
                }
                else
                {
                    PEG_METHOD_EXIT();
                    throw WsmFault(
                        WsmFault::wsmb_PolymorphismModeNotSupported,
                        MessageLoaderParms(
                            "WsmServer.WsmReader.ENUMERATE_"
                                "POLYMORPHISM_MODE_UNSUPPORTED",
                            "Polymorphism mode \"$0\" is not supported.",
                            content));
                }
            }
            else if (mustUnderstand(entry))
            {
                // DSP0226 R5.2-2: If a service cannot comply with a header
                // marked with mustUnderstand="true", it shall issue an
                // s:NotUnderstood fault.
                XmlNamespace* ns = _parser.getNamespace(nsType);
                PEG_METHOD_EXIT();
                throw SoapNotUnderstoodFault(
                    ns ? ns->extendedName : String::EMPTY, elementName);
            }
            else
            {
                skipElement(entry);
                // The end tag, if any, has already been consumed.
                needEndTag = false;
            }

            if (needEndTag)
            {
                expectEndTag(nsType, elementName);
            }
        }

        if (gotEntry)
        {
            _parser.putBack(entry);
        }

        expectEndTag(WsmNamespaces::WS_ENUMERATION, "Enumerate");
    }
    PEG_METHOD_EXIT();
}

void WsmReader::decodePullBody(
    Uint64& enumerationContext,
    String& maxTime,
    Uint32& maxElements,
    Uint32& maxCharacters)
{
    Boolean seenEnumContext = false;
    XmlEntry entry;
    expectStartTag(entry, WsmNamespaces::WS_ENUMERATION, "Pull");

    Boolean gotEntry;
    while ((gotEntry = _parser.next(entry)) &&
           ((entry.type == XmlEntry::START_TAG) ||
            (entry.type == XmlEntry::EMPTY_TAG)))
    {
        int nsType = entry.nsType;
        const char* elementName = entry.localName;
        Boolean needEndTag = (entry.type == XmlEntry::START_TAG);

        if ((nsType == WsmNamespaces::WS_ENUMERATION) &&
            (strcmp(elementName, "EnumerationContext") == 0))
        {
            checkDuplicateHeader(entry.text, seenEnumContext);
            seenEnumContext = true;
            enumerationContext = getEnumerationContext(entry);
        }
        else if ((nsType == WsmNamespaces::WS_ENUMERATION) &&
            (strcmp(elementName, "MaxTime") == 0))
        {
            checkDuplicateHeader(entry.text, maxTime.size());
            maxTime = getElementContent(entry);
        }
        else if ((nsType == WsmNamespaces::WS_ENUMERATION) &&
            (strcmp(elementName, "MaxCharacters") == 0))
        {
            checkDuplicateHeader(entry.text, maxCharacters > 0);
            maxCharacters = getUint32ElementContent(entry, "MaxCharacters");
        }
        else if ((nsType == WsmNamespaces::WS_ENUMERATION) &&
            (strcmp(elementName, "MaxElements") == 0))
        {
            checkDuplicateHeader(entry.text, maxElements > 0);
            maxElements = getUint32ElementContent(entry, "MaxElements");
        }
        else if (mustUnderstand(entry))
        {
            // DSP0226 R5.2-2: If a service cannot comply with a header
            // marked with mustUnderstand="true", it shall issue an
            // s:NotUnderstood fault.
            XmlNamespace* ns = _parser.getNamespace(nsType);
            throw SoapNotUnderstoodFault(
                ns ? ns->extendedName : String::EMPTY, elementName);
        }
        else
        {
            skipElement(entry);
            // The end tag, if any, has already been consumed.
            needEndTag = false;
        }

        if (needEndTag)
        {
            expectEndTag(nsType, elementName);
        }
    }

    if (gotEntry)
    {
        _parser.putBack(entry);
    }

    // EnumerationContext is required; return a fault if it is missing.
    if (!seenEnumContext)
    {
        expectStartTag(
            entry, WsmNamespaces::WS_ENUMERATION, "EnumerationContext");
    }

    expectEndTag(WsmNamespaces::WS_ENUMERATION, "Pull");
}

void WsmReader::decodeReleaseBody(Uint64& enumerationContext)
{
    Boolean seenEnumContext = false;
    XmlEntry entry;
    expectStartTag(entry, WsmNamespaces::WS_ENUMERATION, "Release");

    Boolean gotEntry;
    while ((gotEntry = _parser.next(entry)) &&
           ((entry.type == XmlEntry::START_TAG) ||
            (entry.type == XmlEntry::EMPTY_TAG)))
    {
        int nsType = entry.nsType;
        const char* elementName = entry.localName;
        Boolean needEndTag = (entry.type == XmlEntry::START_TAG);

        if ((nsType == WsmNamespaces::WS_ENUMERATION) &&
            (strcmp(elementName, "EnumerationContext") == 0))
        {
            checkDuplicateHeader(entry.text, seenEnumContext);
            seenEnumContext = true;
            enumerationContext = getEnumerationContext(entry);
        }
        else if (mustUnderstand(entry))
        {
            // DSP0226 R5.2-2: If a service cannot comply with a header
            // marked with mustUnderstand="true", it shall issue an
            // s:NotUnderstood fault.
            XmlNamespace* ns = _parser.getNamespace(nsType);
            throw SoapNotUnderstoodFault(
                ns ? ns->extendedName : String::EMPTY, elementName);
        }
        else
        {
            skipElement(entry);
            // The end tag, if any, has already been consumed.
            needEndTag = false;
        }

        if (needEndTag)
        {
            expectEndTag(nsType, elementName);
        }
    }

    if (gotEntry)
    {
        _parser.putBack(entry);
    }

    // EnumerationContext is required; return a fault if it is missing.
    if (!seenEnumContext)
    {
        expectStartTag(
            entry, WsmNamespaces::WS_ENUMERATION, "EnumerationContext");
    }

    expectEndTag(WsmNamespaces::WS_ENUMERATION, "Release");
}

void WsmReader::decodeInvokeInputBody(
    const String& className,
    const String& methodName,
    WsmInstance& instance)
{
    XmlEntry entry;

    //
    // Parse the <s:Body> element. Here is an example:
    //
    //   <p:Foo_INPUT xmlns:p=
    //     "http://schemas.dmtf.org/wbem/wscim/1/cim-schema/2/SomeClass">
    //     <p:Arg1>
    //       ...
    //     </p:Arg1>
    //     <p:Arg2>
    //       ...
    //     </p:Arg2>
    //   </p:Foo_INPUT>
    //

    // Expect <METHODNAME_INPUT>
    Buffer tagName;
    tagName.append(methodName.getCString(), methodName.size());
    tagName.append("_INPUT", 6);

    _parser.setHideEmptyTags(true);
    int nsType = expectStartTag(entry, tagName.getData());

    // The following elements are input parameter.
    String name;
    WsmValue value;

    while (getPropertyElement(nsType, name, value))
    {
        instance.addProperty(WsmProperty(name, value));
    }

    // Expect </METHODNAME_INPUT>
    expectEndTag(nsType, tagName.getData());
    _parser.setHideEmptyTags(false);
}

void WsmReader::decodeFilter(WsmFilter& wsmFilter, int nsType)
{
    // Expect "Filter" element.
    _parser.setHideEmptyTags(true);

    XmlEntry entry;
    expectStartTag(entry, nsType, PEGASUS_PROPERTYNAME_FILTER_CSTRING);

    // Check Filter.Dialect attribute.
    {
        const char* value;

        if (!entry.getAttributeValue("Dialect", value))
        {
            MessageLoaderParms parms(
                "WsmServer.WsmReader.MISSING_ATTRIBUTE",
                "The attribute $0.$1 is missing.", "Filter", "Dialect");
            throw XmlValidationError(_parser.getLine(), parms);
        }

        const char* suffix = WsmUtils::skipHostUri(value);

        // Process for each acceptable dialect attribute

        // If WQL filter dialect found. Parse for WQL Statement
        if (strcmp(suffix, WSMAN_FILTER_DIALECT_WQL_SUFFIX) == 0)
        {
            wsmFilter.filterDialect = WsmFilter::WQL;
            // Expect query expression (contains the query text).

            expectContentOrCData(entry);
            wsmFilter.WQLFilter.query = entry.text;

            // Compile the query filter.

            try
            {
                wsmFilter.WQLFilter.selectStatement.reset(
                    new WQLSelectStatement);
                WQLParser::parse(wsmFilter.WQLFilter.query,
                    *wsmFilter.WQLFilter.selectStatement.get());
            }
            catch (ParseError& e)
            {
                MessageLoaderParms parms(
                    "WsmServer.WsmReader.INVALID_FILTER_QUERY_EXPRESSION",
                    "Invalid filter query expression: \"$0\".",
                    entry.text);
                throw WsmFault(WsmFault::wsen_CannotProcessFilter, parms);
            }

            // Set the queryLanguage
            wsmFilter.WQLFilter.queryLanguage = "WQL";
        }

        // else if AssociatedFilter Dialect per DSP227, Section 8.2
        else if (strcmp(suffix, WSMAN_ASSOCIATION_FILTER_SUFFIX) == 0)
        {
            wsmFilter.filterDialect = WsmFilter::ASSOCIATION;
            decodeAssociationFilter(wsmFilter);
        }
        else
        {
            MessageLoaderParms parms(
                "WsmServer.WsmReader.UNSUPPORTED_FILTER_DIALECT",
                "Unsupported filter dialect: \"$0\".",
                value);
            throw WsmFault(
                WsmFault::wsen_FilterDialectRequestedUnavailable, parms);
        }

    }

    // Expect </Filter>

    expectEndTag(nsType, PEGASUS_PROPERTYNAME_FILTER_CSTRING);
    _parser.setHideEmptyTags(false);
}

void WsmReader::decodeAssociationFilter(WsmFilter& wsmFilter)
{
    PEG_METHOD_ENTER(TRC_WSMSERVER,"WsmReader::decodeAssociationFilter");
    XmlEntry entry;

    // The next entry must be  either associated or association tag

    wsmFilter.AssocFilter.assocFilterType =
            WsmFilter::ASSOCIATION_INSTANCES;
    if (testStartTag(entry, WsmNamespaces::WS_CIM_BINDING,
        "AssociatedInstances"))
    {
        wsmFilter.AssocFilter.assocFilterType =
            WsmFilter::ASSOCIATED_INSTANCES;
    }
    else if (!testStartTag(entry, WsmNamespaces::WS_CIM_BINDING,
        "AssociationInstances"))
    {
        MessageLoaderParms parms(
            "WsmServer.WsmReader.INVALID_ASSOCIATED_FILTER_ELEMENT",
            "Invalid Association Filter Type Element: \"$0\".",
            entry.text);
        PEG_METHOD_EXIT();
        throw WsmFault(WsmFault::wsen_CannotProcessFilter, parms);
    }

    const char* associatedStartTag = entry.localName;

    // get the entires for object, AssociationClassName Role ResultClassName
    // ResultRole, etc. Only the object is required.
    Boolean seenObject = false;
    Boolean gotEntry;

    // Commented out because the propertyList feature not supported
    // Array<CIMName> propertyListArray;

    _parser.setHideEmptyTags(false);

    while ((gotEntry = _parser.next(entry)) &&
           ((entry.type == XmlEntry::START_TAG) ||
            (entry.type == XmlEntry::EMPTY_TAG)))
    {
        int nsType = entry.nsType;
        const char* elementName = entry.localName;
        Boolean needEndTag = (entry.type == XmlEntry::START_TAG);

        // Test all entires that are in namespace WS_CIM_BINDING
        if (nsType == WsmNamespaces::WS_CIM_BINDING)
        {
            if (strcmp(elementName, "Object") == 0)
            {
                checkDuplicateHeader(entry.text,
                    wsmFilter.AssocFilter.object.getNamespace().size());
                seenObject = true;
                if (!getInstanceEPRElement(wsmFilter.AssocFilter.object))
                {
                    PEG_METHOD_EXIT();
                    MessageLoaderParms parms(
                        "WsmServer.WsmReader.FILTER_OBJECT_EPR_RQD",
                        "Filter Object EPR required");
                    throw WsmFault(
                        WsmFault::wsa_DestinationUnreachable, parms);
                }

                // Namespace required
                if (wsmFilter.AssocFilter.object.getNamespace().size() == 0)
                {
                    PEG_METHOD_EXIT();
                    MessageLoaderParms parms(
                        "WsmServer.WsmReader.FILTER_OBJECT_NAMESPACE_RQD",
                        "Filter Object EPR __namespace element required");
                    throw WsmFault(
                        WsmFault::wsa_DestinationUnreachable, parms);
                }

                //R8.2.1-4: If the EPR of the source object does not reference
                // exactly one valid CIM instance, the service shall respond
                // with a wsen:CannotProcessFilter fault. Services should
                // include a textual description of the problem.
                // Selector must include namespace and at least one key.
                // This required because the Pegasus calls would map this
                // to a class operation without a key property in the
                // CIMObjectPath. We already know that there is a __nameSpacce
                // selector.
                if (wsmFilter.AssocFilter.object.selectorSet->selectors.size()
                    < 2)
                {
                    MessageLoaderParms parms(
                        "WsmServer.WsmReader.INVALID_OBJECT_SELECTOR",
                        "Invalid Selector. No Instance Keys.");
                    PEG_METHOD_EXIT();
                    throw WsmFault(WsmFault::wsen_CannotProcessFilter, parms);
                }
            }

            // get AssociationClassName. Do not test if this is part of
            // filter (i.e. not in association filter) since this is extra
            // parameter.
            if (strcmp(elementName, "AssociationClassName") == 0)
            {
                checkDuplicateHeader(entry.text,
                    wsmFilter.AssocFilter.assocClassName.getString().size());

                wsmFilter.AssocFilter.assocClassName =
                    CIMName(getElementContent(entry));
            }

            else if (strcmp(elementName, "ResultClassName") == 0)
            {
                checkDuplicateHeader(entry.text,
                    wsmFilter.AssocFilter.resultClassName.getString().size());
                wsmFilter.AssocFilter.resultClassName =
                    CIMName(getElementContent(entry));
            }

            else if (strcmp(elementName, "Role") == 0)
            {
                checkDuplicateHeader(entry.text,
                    wsmFilter.AssocFilter.role.size());
                wsmFilter.AssocFilter.role = getElementContent(entry);
            }

            else if (strcmp(elementName, "ResultRole") == 0)
            {
                checkDuplicateHeader(entry.text,
                    wsmFilter.AssocFilter.resultRole.size());
                wsmFilter.AssocFilter.resultRole = getElementContent(entry);
            }

        }   // any entries in other namespaces
        else if ((nsType == WsmNamespaces::WS_ADDRESSING) &&
            (strcmp(elementName, "IncludeResultProperty") == 0))
        {
            /* Error because we do not support fragments.
            R8.2.1-10: If the query includes one or more IncludeResultProperty
                 elements, the service shall return each instance representation
                 using the wsman:XmlFragment element. Within the
                 wsman:XmlFragment element, the service shall return property
                 values using the property GEDs defined in the
                 WS-CIM Mapping Specification. If the query includes one or
                 more IncludeResultProperty elements, the service shall not
                 return any IncludeResultProperty elements not specified.
                 The service shall ignore any IncludeResultProperty elements
                 that describe properties not defined by the target class. If
                 the service does not support fragment-level access, it shall
                 return a wsman:UnsupportedFeature fault with the following
                 detail code:
                http://schemas.dmtf.org/wbem/wsman/1/wsman/
                     //faultDetail/FragmentLevelAccess
            */
            PEG_METHOD_EXIT();
                MessageLoaderParms parms(
                    "WsmServer.WsmReader.INCLUDERESULTPROPERTY_INVALID",
                    "IncludeResultProperty not allowed.");
            throw WsmFault(
                    WsmFault::wsman_UnsupportedFeature,
                    parms,
                    WSMAN_FAULTDETAIL_FRAGMENTLEVELACCESS);
            // Implementation code when we support fragments.
            //          String s1;
            //          s1 = getElementContent(entry);
            //          propertyListArray.append(s1);

        }

        else
        {
            skipElement(entry);
            // The end tag, if any, has already been consumed.
            needEndTag = false;
        }

        if (needEndTag)
        {
            expectEndTag(nsType, elementName);
        }
    }

    if (gotEntry)
    {
        _parser.putBack(entry);
    }

    // object is required; return a fault if it is missing.
    if (!seenObject)
    {
        expectStartTag(
            entry, WsmNamespaces::WS_CIM_BINDING, "Object");
    }

    // if there were propertyList entries and we support fragments,
    // set into the propertyList parameter and we support fragments.
    //  if (propertyListArray.size() != 0)
    //  {
    //      propertyList.set(propertyListArray);
    //  }

    // Expect the end tag for AssociatedInstances or AssociationInstances
    expectEndTag(WsmNamespaces::WS_CIM_BINDING, associatedStartTag);

    PEG_TRACE((TRC_WSMSERVER, Tracer::LEVEL4,  // KS_TEMP
               "Association Filter "
               "Object Namespace=%s Address=%s resourceUri=%s "
               "assocFilterType=%u "
               "assocClassName=%s "
               "resultClassName=%s "
               "role=%s "
               "resultRole=%s",
               (const char*)
                   wsmFilter.AssocFilter.object.getNamespace().getCString(),
               (const char*)wsmFilter.AssocFilter.object.address.getCString(),
               (const char*)
                   wsmFilter.AssocFilter.object.resourceUri.getCString(),

               wsmFilter.AssocFilter.assocFilterType,
               (const char*)
                 wsmFilter.AssocFilter.assocClassName.getString().getCString(),
               (const char*)
                 wsmFilter.AssocFilter.resultClassName.getString().getCString(),
               (const char*)wsmFilter.AssocFilter.role.getCString(),
               (const char*)wsmFilter.AssocFilter.resultRole.getCString()));
    PEG_METHOD_EXIT();
}

void WsmReader::decodeSubscribeBody(
    String& deliveryMode,
    String& destination,
    String& subExpiration,
    WsmFilter& wsmFilter)
{
    PEG_METHOD_ENTER(TRC_WSMSERVER, "WsmReader::decodeSubscribeBody()");
    XmlEntry entry;
    expectStartOrEmptyTag(
        entry, WsmNamespaces::WS_EVENTING, PEGASUS_WS_SUBSCRIBE);
    Boolean gotEntry; 
    while ((gotEntry = _parser.next(entry)) &&
        ((entry.type == XmlEntry::START_TAG) ||
        (entry.type == XmlEntry::EMPTY_TAG)))
    {
        int nsType = entry.nsType;
        const char* elementName = entry.localName;
        Boolean needEndTag = (entry.type == XmlEntry::START_TAG);
        if((nsType == WsmNamespaces::WS_EVENTING) && 
            (strcmp(elementName, "Delivery") == 0))
        {
            _parser.putBack(entry);
            _decodeDeliveryField(
                deliveryMode,
                destination);
            needEndTag = false;
        }
        else if((nsType == WsmNamespaces::WS_EVENTING) &&
            (strcmp(elementName, "Expires") == 0))
        {
            subExpiration = getElementContent(entry); 

            CIMDateTime dt;
            try
            {
                WsmToCimRequestMapper::convertWsmToCimDatetime(
                    subExpiration,
                    dt);
            }
            catch (...)
            {
                throw WsmFault(
                    WsmFault::wse_InvalidExpirationTime,
                    MessageLoaderParms(
                        "WsmServer.WsmReader.INVALID_EXPIRATION_TIME",
                        "The expiration time \"$0\" is not valid",
                        subExpiration));
            }

            //If the expiration time is specified in the DateTime format
            //we need to calculate the duration.
            //The expiration DateTime - current time will be the duration

            if(dt.isTimeStamp())
            {
                dt = dt - CIMDateTime::getCurrentDateTime();
            }
 
            subExpiration = WsmUtils::toMicroSecondString(dt);
    
        }
        else if(((nsType == WsmNamespaces::WS_MAN) || 
        (nsType == WsmNamespaces::WS_EVENTING)) &&
            (strcmp(elementName, PEGASUS_PROPERTYNAME_FILTER_CSTRING) == 0))
        {
            checkDuplicateHeader(entry.text,
                wsmFilter.filterDialect != WsmFilter::NONE);
            _parser.putBack(entry);
            decodeFilter(wsmFilter, nsType);
            needEndTag = false;
        } 
        else if((nsType== WsmNamespaces::WS_MAN) &&
            (strcmp(elementName, "SendBookmarks") == 0))
        {
            throw WsmFault(
                    WsmFault::wsman_UnsupportedFeature,
                    MessageLoaderParms(
                        "WsmServer.WsmReader.UNSUPPORTED_FEATURE",
                        "The specified feature is not supported"),
                    WSMAN_FAULTDETAIL_BOOKMARKS_UNSUPPORTED);            
        }
        else if (mustUnderstand(entry))
        {
            // DSP0226 R5.2-2: If a service cannot comply with a header
            // marked with mustUnderstand="true", it shall issue an
            // s:NotUnderstood fault.
            XmlNamespace* ns = _parser.getNamespace(nsType);
                throw SoapNotUnderstoodFault(
                ns ? ns->extendedName : String::EMPTY, elementName);
        }
        else
        {
            skipElement(entry);
            // The end tag, if any, has already been consumed.
            needEndTag = false;
        }
        if (needEndTag)
        {
            expectEndTag(nsType, elementName);
        }
    }
    
    if (gotEntry)
    {
        _parser.putBack(entry);
    }

    expectEndTag(WsmNamespaces::WS_EVENTING, PEGASUS_WS_SUBSCRIBE); 
    PEG_METHOD_EXIT();
}


void WsmReader::_decodeDeliveryField(
    String& delMode,
    String& destination)
{
    PEG_METHOD_ENTER(TRC_WSMSERVER, "WsmReader::_decodeDeliveryField()");
    _parser.setHideEmptyTags(true);
    XmlEntry entry;

    expectStartTag(entry, WsmNamespaces::WS_EVENTING, PEGASUS_WS_DELIVERY);
    //Check delivery Mode attribute
    const char* value;
    if (!entry.getAttributeValue(PEGASUS_WS_DELMODE, value))
    {
        MessageLoaderParms parms(
            "WsmServer.WsmReader.MISSING_ATTRIBUTE",
            "The attribute $0.$1 is missing.", 
            PEGASUS_WS_DELIVERY, 
            PEGASUS_WS_DELMODE);
        throw XmlValidationError(_parser.getLine(), parms);
    }
    // The only supported delivery mode is PUSH. 
    // If this changes we need to add other delivery modes
    if(!((strcmp(value,WSMAN_DELIVEY_MODE_PUSH) == 0) || (
        strcmp(value,WSMAN_DELIVERY_MODE_PUSH_WITH_ACK) == 0)))
    {
        MessageLoaderParms parms(
            "WsmServer.WsmReader.UNSUPPORTED_DELIVERY_MODE",
            "The requested delivery mode is not supported.");
        throw WsmFault(WsmFault::wse_DeliveryModeRequestedUnavailable, parms);
    }
    else
    {
        deliveryMode mode;
        if(strcmp(value, WSMAN_DELIVEY_MODE_PUSH) == 0)
        {
            mode = Push;
        } 
        else
        {
            mode = PushWithAck; 
        }
        char buffer[22];
        Uint32 size;
        delMode = Uint16ToString(buffer, mode, size);
    }
    while((_parser.next(entry)) &&
        ((entry.type == XmlEntry::START_TAG) ||
        (entry.type == XmlEntry::EMPTY_TAG)))
    {
        if((entry.nsType== WsmNamespaces::WS_EVENTING) &&
            (strcmp(entry.localName, PEGASUS_WS_NOTIFYTO) == 0))
        {
            checkDuplicateHeader(entry.text, destination.size());
            getElementStringValue(
                WsmNamespaces::WS_ADDRESSING,
                "Address",
                destination,
                true);
            expectEndTag(WsmNamespaces::WS_EVENTING, PEGASUS_WS_NOTIFYTO);
        }
        else if((entry.nsType== WsmNamespaces::WS_MAN) &&
            (strcmp(entry.localName, "Heartbeats") == 0))
        {
            throw WsmFault(
                    WsmFault::wsman_UnsupportedFeature,
                    MessageLoaderParms(
                        "WsmServer.WsmReader.UNSUPPORTED_FEATURE",
                        "The specified feature is not supported"),
                    WSMAN_FAULTDETAIL_HEARTBEATS_UNSUPPORTED);            
        }
        else if((entry.nsType== WsmNamespaces::WS_MAN) &&
            (strcmp(entry.localName, "ConnectionRetry") == 0))
        {
            // Connection retry is not supported
            throw WsmFault(
                    WsmFault::wsman_UnsupportedFeature,
                    MessageLoaderParms(
                        "WsmServer.WsmReader.UNSUPPORTED_FEATURE",
                        "The specified feature is not supported"),
                    WSMAN_FAULTDETAIL_CONNECTION_RETRY_UNSUPPORTED);
        } 
    }
    _parser.setHideEmptyTags(false); 
    PEG_METHOD_EXIT();
}
XmlParser& WsmReader::getParser()
{
   return _parser;
}

PEGASUS_NAMESPACE_END
