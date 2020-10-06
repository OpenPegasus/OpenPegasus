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

#include <Pegasus/WsmServer/WsmConstants.h>
#include <Pegasus/WsmServer/WsmUtils.h>
#include <Pegasus/WsmServer/WsmReader.h>
#include <Pegasus/WsmServer/WsmWriter.h>
#include <Pegasus/WsmServer/WsmValue.h>
#include <Pegasus/WsmServer/WsmEndpointReference.h>
#include <Pegasus/WsmServer/WsmInstance.h>
#include <Pegasus/WsmServer/WsmSelectorSet.h>
#include <Pegasus/WsmServer/WsmRequest.h>
#include <Pegasus/WsmServer/WsmResponse.h>
#include <Pegasus/WsmServer/SoapResponse.h>

PEGASUS_USING_PEGASUS;
PEGASUS_USING_STD;

static Boolean verbose;

static Boolean _compareInstances(WsmInstance& inst1, WsmInstance& inst2);

static Boolean _compareEPRs(
    WsmEndpointReference& epr1,
    WsmEndpointReference& epr2)
{
    return epr1 == epr2;
}

static Boolean _compareValues(WsmValue& val1, WsmValue& val2)
{
    if (val1.getType() != val2.getType() ||
        val1.isNull() != val2.isNull() ||
        val1.isArray() != val2.isArray() ||
        val1.getArraySize() != val1.getArraySize())
        return false;

    if (val1.isArray())
    {
        switch (val1.getType())
        {
            case WSMTYPE_REFERENCE:
            {
                Array<WsmEndpointReference> epr1, epr2;
                val1.get(epr1);
                val2.get(epr2);
                for (Uint32 i = 0; i < epr1.size(); i++)
                {
                    if (!_compareEPRs(epr1[i], epr2[i]))
                        return false;
                }
                break;
            }
            case WSMTYPE_INSTANCE:
            {
                Array<WsmInstance> inst1, inst2;
                val1.get(inst1);
                val2.get(inst2);
                for (Uint32 i = 0; i < inst1.size(); i++)
                {
                    if (!_compareInstances(inst1[i], inst2[i]))
                        return false;
                }
                break;
            }
            case WSMTYPE_OTHER:
            {
                Array<String> str1, str2;
                val1.get(str1);
                val2.get(str2);
                for (Uint32 i = 0; i < str1.size(); i++)
                {
                    if (str1[i] != str2[i])
                        return false;
                }
                break;
            }
            default:
                PEGASUS_TEST_ASSERT(0);
        }
    }
    else
    {
        switch (val1.getType())
        {
            case WSMTYPE_REFERENCE:
            {
                WsmEndpointReference epr1, epr2;
                val1.get(epr1);
                val2.get(epr2);
                return _compareEPRs(epr1, epr2);
            }
            case WSMTYPE_INSTANCE:
            {
                WsmInstance inst1, inst2;
                val1.get(inst1);
                val2.get(inst2);
                return _compareInstances(inst1, inst2);
            }
            case WSMTYPE_OTHER:
            {
                String str1, str2;
                val1.get(str1);
                val2.get(str2);
                return str1 == str2;
            }
            default:
                PEGASUS_TEST_ASSERT(0);
        }
    }

    return true;
}

static Boolean _compareProperties(WsmProperty& prop1, WsmProperty& prop2)
{
    if (prop1.getName() != prop2.getName())
        return false;

    return _compareValues(prop1.getValue(), prop2.getValue());
}

static Boolean _compareInstances(WsmInstance& inst1, WsmInstance& inst2)
{
    if (inst1.getClassName() != inst2.getClassName() ||
        inst1.getPropertyCount() != inst2.getPropertyCount())
        return false;

    for (Uint32 i = 0; i < inst1.getPropertyCount(); i++)
    {
        if (!_compareProperties(inst1.getProperty(i), inst2.getProperty(i)))
            return false;
    }

    return true;
}

static void _appendSoapEnvelopeStart(Buffer& out)
{
    out << STRLIT("<");
    out << WsmNamespaces::supportedNamespaces[
        WsmNamespaces::SOAP_ENVELOPE].localName;
    out << STRLIT(":Envelope");

    for (unsigned int i = 0; i < WsmNamespaces::LAST; i++)
    {
        out << STRLIT(" xmlns:");
        out << WsmNamespaces::supportedNamespaces[i].localName;
        out << STRLIT("=\"");
        out << WsmNamespaces::supportedNamespaces[i].extendedName;
        out << STRLIT("\"");
    }
    out << STRLIT(">");
}

static void _checkInstance(WsmInstance& inst, const char* text)
{
    Buffer out;
    _appendSoapEnvelopeStart(out);
    WsmWriter::appendInstanceElement(out, WSM_RESOURCEURI_CIMSCHEMAV2, inst,
        PEGASUS_INSTANCE_NS, false);
    WsmWriter::appendEndTag(
        out, WsmNamespaces::SOAP_ENVELOPE, STRLIT("Envelope"));

    WsmInstance inst1;
    XmlEntry entry;
    WsmReader reader((char*)out.getData());
    reader.expectStartTag(entry, WsmNamespaces::SOAP_ENVELOPE, "Envelope");
    reader.getInstanceElement(inst1);
    reader.expectEndTag(WsmNamespaces::SOAP_ENVELOPE, "Envelope");
    if (!_compareInstances(inst, inst1))
        throw Exception(text);
}

static void _testInstances(void)
{
    // Simple test: instance with a few string properties
    {
        WsmInstance inst("testClass");
        WsmValue val_1("value_1");
        WsmValue val_2("value_2");
        inst.addProperty(WsmProperty("property_1", val_1));
        inst.addProperty(WsmProperty("property_2", val_2));

        _checkInstance(inst,
            "Instances with string properties do not compare");
    }

    // Test string array properties
    {
        WsmInstance inst("testClass");
        Array<String> stra;
        for (int i = 0; i < 5; i++)
        {
            char buf[20];
            sprintf(buf, "prop_1 value: %d", i);
            stra.append(buf);
        }
        WsmValue val1(stra);
        inst.addProperty(WsmProperty("property_1", val1));

        stra.clear();
        for (int i = 0; i < 10; i++)
        {
            char buf[20];
            sprintf(buf, "prop_2 value: %d", i);
            stra.append(buf);
        }
        WsmValue val2(stra);
        inst.addProperty(WsmProperty("property_2", val2));

        _checkInstance(inst,
            "Instances with string array properties do not compare");
    }

    // Test EPR properties
    {
        WsmInstance inst("testClass");
        WsmEndpointReference epr;
        epr.address = "http://www.acme.com:5988/wsman";
        epr.resourceUri = "TestURI";
        epr.selectorSet->selectors.
            append(WsmSelector("sel_1 name", "sel_1 value"));
        epr.selectorSet->selectors.
            append(WsmSelector("sel_2 name", "sel_2 value"));
        WsmValue val(epr);
        inst.addProperty(WsmProperty("property_1", val));

        _checkInstance(inst, "Instances with EPR properties do not compare");
    }

    // Test EPR array properties
    {
        WsmInstance inst("testClass");
        Array<WsmEndpointReference> epra;
        for (int i = 0; i < 5; i++)
        {
            char buf[50];
            WsmEndpointReference epr;
            sprintf(buf, "http://www.acme.com_%d:5988/wsman", i);
            epr.address = buf;
            sprintf(buf, "TestURI_%d", i);
            epr.resourceUri = buf;
            sprintf(buf, "selector value %d", i);
            epr.selectorSet->selectors.
                append(WsmSelector("sel_name", buf));
            epra.append(epr);

        }
        WsmValue val(epra);
        inst.addProperty(WsmProperty("property_1", val));

        _checkInstance(inst,
            "Instances with EPR array properties do not compare");
    }

    // Test recursive EPR properties
    {
        WsmInstance inst("testClass");

        WsmEndpointReference epr1;
        epr1.address = "http://www.acme.com_1:5988/wsman";
        epr1.resourceUri = "TestURI_1";
        epr1.selectorSet->selectors.
            append(WsmSelector("sel_0 name", "sel_1 value"));
        epr1.selectorSet->selectors.
            append(WsmSelector("sel_1 name", "sel_2 value"));

        WsmEndpointReference epr2;
        epr2.address = "http://www.acme.com_2:5988/wsman";
        epr2.resourceUri = "TestURI_2";
        epr2.selectorSet->selectors.append(WsmSelector("sel_2 name", epr1));

        WsmEndpointReference epr3;
        epr3.address = "http://www.acme.com_3:5988/wsman";
        epr3.resourceUri = "TestURI_3";
        epr3.selectorSet->selectors.append(WsmSelector("sel_3 name", epr2));

        WsmEndpointReference epr;
        epr.address = "http://www.acme.com:5988/wsman";
        epr.resourceUri = "TestURI";
        epr.selectorSet->selectors.append(WsmSelector("sel name", epr3));

        WsmValue val(epr);
        inst.addProperty(WsmProperty("property_1", val));

        _checkInstance(inst,
            "Instances with recursive EPR properties do not compare");
    }

    // Test embedded instance properties
    {
        WsmInstance inst1("testClass_1");
        WsmValue val_1("value_1");
        WsmValue val_2("value_2");
        inst1.addProperty(WsmProperty("property_1", val_1));
        inst1.addProperty(WsmProperty("property_2", val_2));

        WsmInstance inst("testClass");
        WsmValue val_3(inst1);
        inst.addProperty(WsmProperty("property_3", val_3));

        _checkInstance(inst,
            "Instances with instance properties do not compare");
    }

    // Test arrays of embedded instances
    {
        WsmInstance inst("testClass");
        Array<WsmInstance> insta;
        for (int i = 0; i < 5; i++)
        {
            char buf[20];
            sprintf(buf, "testClass_%d", i);
            WsmInstance inst1(buf);
            sprintf(buf, "value_%d", i);
            WsmValue val(buf);
            sprintf(buf, "property_%d", i);
            inst1.addProperty(WsmProperty(buf, val));

            insta.append(inst1);
        }
        WsmValue val(insta);
        inst.addProperty(WsmProperty("prop_array", val));

        _checkInstance(inst,
            "Instances with instance array properties do not compare");
    }

    // Test recursive embedded instances
    {
        WsmInstance inst1("testClass_1");
        WsmValue val_0("value_0");
        WsmValue val_1("value_1");
        inst1.addProperty(WsmProperty("property_0", val_0));
        inst1.addProperty(WsmProperty("property_1", val_1));

        WsmInstance inst2("testClass_2");
        WsmValue val_2(inst1);
        inst2.addProperty(WsmProperty("property_2", val_2));

        WsmInstance inst3("testClass_3");
        WsmValue val_3(inst2);
        inst3.addProperty(WsmProperty("property_3", val_3));

        WsmInstance inst("testClass");
        WsmValue val(inst3);
        inst.addProperty(WsmProperty("property", val));

        _checkInstance(inst,
            "Instances with recursive instance properties do not compare");
    }
}

static void  _testResponseFormatting(void)
{
    ContentLanguageList contentLanguage;
    Buffer body, header, out;
    const char expectedHttpHeader[] =
        "HTTP/1.1 200 OK\r\nContent-Type: application/soap+xml;charset=UTF-8"
        "\r\ncontent-length: 0000000000\r\nSOAPAction: "
        "http://schemas.xmlsoap.org/ws/2004/09/transfer/GetResponse\r\n\r\n";

    // Create FaultTo header
    WsmWriter::appendStartTag(
        header, WsmNamespaces::WS_ADDRESSING, STRLIT("FaultTo"));
    WsmWriter::appendTagValue(header, WsmNamespaces::WS_ADDRESSING,
        STRLIT("Address"), "http://www.acme.com:5988/wsman");
    WsmWriter::appendEndTag(
        header, WsmNamespaces::WS_ADDRESSING, STRLIT("FaultTo"));

    // Create an instance for Soap body
    WsmInstance inst("testClass");
    WsmValue val("value");
    inst.addProperty(WsmProperty("property", val));
    WsmWriter::appendInstanceElement(body, WSM_RESOURCEURI_CIMSCHEMAV2, inst,
        PEGASUS_INSTANCE_NS, false);

    String messageId = WsmUtils::getMessageId();
    WxfGetRequest request(messageId, WsmEndpointReference());
    WxfGetResponse response(inst, &request, contentLanguage);
    SoapResponse soapResponse(&response);
    soapResponse.appendHeader(header);
    soapResponse.appendBodyContent(body);
    out = soapResponse.getResponseContent();

    char* ptr = (char*) out.getData();
    if (strncmp(ptr, expectedHttpHeader, sizeof(expectedHttpHeader)-1) != 0)
        throw Exception("HTTP header incorrect");
    ptr += sizeof(expectedHttpHeader)-1;

    WsmReader reader(ptr);
    const char* xmlVersion = 0;
    const char* xmlEncoding = 0;
    reader.getXmlDeclaration(xmlVersion, xmlEncoding);
    if (strcmp(xmlVersion, "1.0") != 0 || strcmp(xmlEncoding, "utf-8") != 0)
        throw Exception("XML version or encoding incorrect");

    XmlEntry entry;
    reader.expectStartTag(entry, WsmNamespaces::SOAP_ENVELOPE, "Envelope");

    String wsaMessageId;
    String wsaAction;
    String wsaFrom;
    String wsaReplyTo;
    String wsaFaultTo;
    String WsaIdentifier;
    WsmEndpointReference epr;
    Uint32 wsmMaxEnvelopeSize = 0;
    AcceptLanguageList wsmLocale;
    Boolean wsmRequestEpr = false;
    Boolean wsmRequestItemCount = false;
    reader.decodeRequestSoapHeaders(
        wsaMessageId, epr.address, wsaAction, wsaFrom, wsaReplyTo,
        wsaFaultTo, epr.resourceUri, *epr.selectorSet, wsmMaxEnvelopeSize,
        wsmLocale, wsmRequestEpr, wsmRequestItemCount,WsaIdentifier);

    if (epr.address != WSM_ADDRESS_ANONYMOUS ||
        wsaAction != WSM_ACTION_GET_RESPONSE ||
        wsaFrom != String::EMPTY ||
        wsaReplyTo != String::EMPTY ||
        wsaMessageId != response.getMessageId() ||
        wsaFaultTo != "http://www.acme.com:5988/wsman" ||
        epr.resourceUri != String::EMPTY ||
        wsmMaxEnvelopeSize != 0 ||
        wsmRequestEpr != false)
        throw Exception("Invalid Soap headers");

    // Read and verify the instance in Soap body.
    WsmInstance inst1;
    reader.expectStartTag(entry, WsmNamespaces::SOAP_ENVELOPE, "Body");
    reader.getInstanceElement(inst1);
    if (!_compareInstances(inst, inst1))
        throw Exception("Instances do not compare");
    reader.expectEndTag(WsmNamespaces::SOAP_ENVELOPE, "Body");

    reader.expectEndTag(WsmNamespaces::SOAP_ENVELOPE, "Envelope");
}

static void  _testFaultFormatting(void)
{
    String relatesTo = WsmUtils::getMessageId();

    // Test Soap NotUnderstood fault.
    {
        const char expectedHttpHeader[] =
            "HTTP/1.1 500 Internal Server Error\r\nContent-Type: "
            "application/soap+xml;charset=UTF-8\r\ncontent-length: "
            "0000000000\r\nSOAPAction: "
            "http://schemas.xmlsoap.org/ws/2004/08/addressing/fault\r\n\r\n";

        SoapNotUnderstoodFault soapFault(WsmNamespaces::supportedNamespaces[
            WsmNamespaces::WS_MAN].extendedName, "RequestedEPR");
        SoapFaultResponse response(relatesTo, 0, HTTP_METHOD__POST,
            false, false, soapFault);
        SoapResponse soapResponse(&response);

        Buffer out = soapResponse.getResponseContent();

        char* ptr = (char*) out.getData();
        if (strncmp(ptr, expectedHttpHeader,
                    sizeof(expectedHttpHeader)-1) != 0)
            throw Exception("HTTP header incorrect");
        ptr += sizeof(expectedHttpHeader)-1;

        WsmReader reader(ptr);
        const char* xmlVersion = 0;
        const char* xmlEncoding = 0;
        reader.getXmlDeclaration(xmlVersion, xmlEncoding);
        if (strcmp(xmlVersion, "1.0") != 0 ||
            strcmp(xmlEncoding, "utf-8") != 0)
            throw Exception("XML version or encoding incorrect");

        XmlEntry entry;
        reader.expectStartTag(entry, WsmNamespaces::SOAP_ENVELOPE, "Envelope");
        reader.expectStartTag(entry, WsmNamespaces::SOAP_ENVELOPE, "Header");
        reader.expectStartOrEmptyTag(
            entry, WsmNamespaces::SOAP_ENVELOPE, "NotUnderstood");

        String addr, action, mid, relto;
        reader.getElementStringValue(
            WsmNamespaces::WS_ADDRESSING, "To", addr, true);
        reader.getElementStringValue(
            WsmNamespaces::WS_ADDRESSING, "Action", action, true);
        reader.getElementStringValue(
            WsmNamespaces::WS_ADDRESSING, "MessageID", mid, true);
        reader.getElementStringValue(
            WsmNamespaces::WS_ADDRESSING, "RelatesTo", relto, true);
        if (addr != WSM_ADDRESS_ANONYMOUS ||
            action != WSM_ACTION_WSA_FAULT ||
            mid != response.getMessageId() ||
            relto != relatesTo)
            throw Exception("Invalid header");

        reader.expectEndTag(WsmNamespaces::SOAP_ENVELOPE, "Header");
        reader.expectStartTag(entry, WsmNamespaces::SOAP_ENVELOPE, "Body");
        reader.expectStartTag(entry, WsmNamespaces::SOAP_ENVELOPE, "Fault");
        reader.expectStartTag(entry, WsmNamespaces::SOAP_ENVELOPE, "Code");
        String code, reason;
        reader.getElementStringValue(
            WsmNamespaces::SOAP_ENVELOPE, "Value", code, true);
        reader.expectEndTag(WsmNamespaces::SOAP_ENVELOPE, "Code");
        reader.expectStartTag(entry, WsmNamespaces::SOAP_ENVELOPE, "Reason");
        reader.getElementStringValue(
            WsmNamespaces::SOAP_ENVELOPE, "Text", reason, true);
        if (code != String(WsmNamespaces::supportedNamespaces[
               WsmNamespaces::SOAP_ENVELOPE].localName) + ":MustUnderstand" ||
            reason != "Header not understood.")
            throw Exception("Invalid Soap fault");

        reader.expectEndTag(WsmNamespaces::SOAP_ENVELOPE, "Reason");
        reader.expectEndTag(WsmNamespaces::SOAP_ENVELOPE, "Fault");
        reader.expectEndTag(WsmNamespaces::SOAP_ENVELOPE, "Body");
        reader.expectEndTag(WsmNamespaces::SOAP_ENVELOPE, "Envelope");
    }

    // Test Wsm fault.
    {
        const char expectedHttpHeader[] =
            "HTTP/1.1 500 Internal Server Error\r\nContent-Type: "
            "application/soap+xml;charset=UTF-8\r\ncontent-length: "
            "0000000000\r\nSOAPAction: "
            "http://schemas.dmtf.org/wbem/wsman/1/wsman/fault\r\n\r\n";

        WsmFault wsmFault(WsmFault::wsman_AccessDenied, "Whatever reason",
            ContentLanguageList(), "Whatever fault detail");
        WsmFaultResponse response(relatesTo, 0, HTTP_METHOD__POST,
            false, false, wsmFault);
        SoapResponse soapResponse(&response);

        Buffer out = soapResponse.getResponseContent();

        char* ptr = (char*) out.getData();
        if (strncmp(ptr, expectedHttpHeader,
                    sizeof(expectedHttpHeader)-1) != 0)
            throw Exception("HTTP header incorrect");
        ptr += sizeof(expectedHttpHeader)-1;

        WsmReader reader(ptr);
        const char* xmlVersion = 0;
        const char* xmlEncoding = 0;
        reader.getXmlDeclaration(xmlVersion, xmlEncoding);
        if (strcmp(xmlVersion, "1.0") != 0 ||
            strcmp(xmlEncoding, "utf-8") != 0)
            throw Exception("XML version or encoding incorrect");

        XmlEntry entry;
        reader.expectStartTag(entry, WsmNamespaces::SOAP_ENVELOPE, "Envelope");
        reader.expectStartTag(entry, WsmNamespaces::SOAP_ENVELOPE, "Header");

        String addr, action, mid, relto;
        reader.getElementStringValue(
            WsmNamespaces::WS_ADDRESSING, "To", addr, true);
        reader.getElementStringValue(
            WsmNamespaces::WS_ADDRESSING, "Action", action, true);
        reader.getElementStringValue(
            WsmNamespaces::WS_ADDRESSING, "MessageID", mid, true);
        reader.getElementStringValue(
            WsmNamespaces::WS_ADDRESSING, "RelatesTo", relto, true);
        if (addr != WSM_ADDRESS_ANONYMOUS ||
            action != WSM_ACTION_WSMAN_FAULT ||
            mid != response.getMessageId() ||
            relto != relatesTo)
            throw Exception("Invalid header");

        reader.expectEndTag(WsmNamespaces::SOAP_ENVELOPE, "Header");
        reader.expectStartTag(entry, WsmNamespaces::SOAP_ENVELOPE, "Body");
        reader.expectStartTag(entry, WsmNamespaces::SOAP_ENVELOPE, "Fault");
        reader.expectStartTag(entry, WsmNamespaces::SOAP_ENVELOPE, "Code");
        String code, subcode, reason, detail;
        reader.getElementStringValue(
            WsmNamespaces::SOAP_ENVELOPE, "Value", code, true);
        reader.expectStartTag(entry, WsmNamespaces::SOAP_ENVELOPE, "Subcode");
        reader.getElementStringValue(
            WsmNamespaces::SOAP_ENVELOPE, "Value", subcode, true);
        reader.expectEndTag(WsmNamespaces::SOAP_ENVELOPE, "Subcode");
        reader.expectEndTag(WsmNamespaces::SOAP_ENVELOPE, "Code");
        reader.expectStartTag(entry, WsmNamespaces::SOAP_ENVELOPE, "Reason");
        reader.getElementStringValue(
            WsmNamespaces::SOAP_ENVELOPE, "Text", reason, true);
        reader.expectEndTag(WsmNamespaces::SOAP_ENVELOPE, "Reason");
        reader.expectStartTag(entry, WsmNamespaces::SOAP_ENVELOPE, "Detail");
        reader.getElementStringValue(
            WsmNamespaces::WS_MAN, "FaultDetail", detail, true);
        reader.expectEndTag(WsmNamespaces::SOAP_ENVELOPE, "Detail");

        if (code != String(WsmNamespaces::supportedNamespaces[
               WsmNamespaces::SOAP_ENVELOPE].localName) + ":Sender" ||
            subcode != String(WsmNamespaces::supportedNamespaces[
               WsmNamespaces::WS_MAN].localName) + ":AccessDenied" ||
            reason != "Whatever reason" ||
            detail != "Whatever fault detail")
            throw Exception("Invalid Wsm fault");

        reader.expectEndTag(WsmNamespaces::SOAP_ENVELOPE, "Fault");
        reader.expectEndTag(WsmNamespaces::SOAP_ENVELOPE, "Body");
        reader.expectEndTag(WsmNamespaces::SOAP_ENVELOPE, "Envelope");
   }
}

int main(int argc, char** argv)
{
    verbose = getenv("PEGASUS_TEST_VERBOSE") ? true : false;

    try
    {
        if (verbose)
            cout << "Testing instances." << endl;
        _testInstances();

        if (verbose)
            cout << "Testing response formatting." << endl;
        _testResponseFormatting();

        if (verbose)
            cout << "Testing fault formatting." << endl;
        _testFaultFormatting();
    }

    catch(Exception& e)
    {
        cerr << "Error: " << e.getMessage() << endl;
        exit(1);
    }

    cout << argv[0] << " +++++ passed all tests" << endl;

    return 0;
}
