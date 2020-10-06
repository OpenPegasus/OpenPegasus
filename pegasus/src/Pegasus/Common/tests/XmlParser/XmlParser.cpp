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

#include <iostream>
#include <Pegasus/Common/XmlParser.h>
#include <Pegasus/Common/Array.h>
#include <Pegasus/Common/FileSystem.h>
#include <cstdio>

PEGASUS_USING_PEGASUS;
PEGASUS_USING_STD;

Boolean verbose = false;

static void _parseFile(const char* fileName, Boolean hideEmptyTags)
{
    // cout << "Parsing: " << fileName << endl;

    Buffer text;
    FileSystem::loadFileToMemory(text, fileName);

    XmlParser parser((char*)text.getData(), 0, hideEmptyTags);

    XmlEntry entry;

    // Get initial comment and ignore
    parser.next(entry, true);
    // get next comment, check for file Description
    parser.next(entry, true);
    if (!String::equal(entry.text, "Test XML file") )
    {
        throw CIMException(CIM_ERR_FAILED, "Comment Error");
    }
    PEGASUS_TEST_ASSERT (parser.getLine () == 2);
    PEGASUS_TEST_ASSERT (parser.getStackSize () == 0);
    // Put the Comment back...
    parser.putBack (entry);
    PEGASUS_TEST_ASSERT (parser.getLine () == 2);
    PEGASUS_TEST_ASSERT (parser.getStackSize () == 0);
    while (parser.next(entry))
    {
        if (verbose)
        {
            entry.print();
        }
    }
    PEGASUS_TEST_ASSERT (parser.next (entry, true) == false);
}

#define ASSERT_XML_EXCEPTION(statement)  \
    do                                   \
    {                                    \
        Boolean caughtException = false; \
        try                              \
        {                                \
            statement;                   \
        }                                \
        catch (XmlException& e)          \
        {                                \
            caughtException = true;      \
        }                                \
        PEGASUS_TEST_ASSERT(caughtException); \
    } while(0)

void testNamespaceSupport(Boolean hideEmptyTags)
{
    XmlNamespace testNamespaces[] =
    {
        {
            "ns0",
            "urn:0",
            0,
            0
        },
        {
            "ns1",
            "urn:1",
            1,
            0
        },
        {
            "ns2",
            "urn:2",
            2,
            0
        },
        { 0, 0, 0, 0 }    // Terminator
    };

    XmlEntry entry;
    const XmlAttribute* attr;

    // Test namespace scoping
    {
        char xmlContent[] =
            "<a:tag xmlns:a=\"urn:0\" xmlns:b=\"urn:1\">"
            " <b:tag xmlns=\"urn:2\" ignore=\"false\" a:attr=\"true\">"
            "  <tag/>"
            " </b:tag>"
            " <tag xmlns=\"urn:0\" xml:lang=\"en-US\">"
            "  Data"
            " </tag>"
            " <d:tag xmlns:d=\"urn:x\"/>"
            " <b:tag xmlns:b=\"urn:1\"/>"
            "</a:tag>";
        XmlParser p(xmlContent, testNamespaces, hideEmptyTags);

        // <a:tag xmlns:a=\"urn:0\" xmlns:b=\"urn:1\">
        PEGASUS_TEST_ASSERT(p.next(entry));
        PEGASUS_TEST_ASSERT(entry.type == XmlEntry::START_TAG);
        PEGASUS_TEST_ASSERT(!strcmp(entry.text, "a:tag"));
        PEGASUS_TEST_ASSERT(entry.nsType == 0);
        PEGASUS_TEST_ASSERT(!strcmp(entry.localName, "tag"));
        PEGASUS_TEST_ASSERT(p.getNamespace(0) != 0);
        PEGASUS_TEST_ASSERT(p.getNamespace(1) != 0);
        PEGASUS_TEST_ASSERT(p.getNamespace(2) == 0);

        // <b:tag xmlns=\"urn:2\" ignore=\"false\" a:attr=\"true\">
        PEGASUS_TEST_ASSERT(p.next(entry));
        PEGASUS_TEST_ASSERT(entry.type == XmlEntry::START_TAG);
        PEGASUS_TEST_ASSERT(!strcmp(entry.text, "b:tag"));
        PEGASUS_TEST_ASSERT(entry.nsType == 1);
        PEGASUS_TEST_ASSERT(!strcmp(entry.localName, "tag"));
        PEGASUS_TEST_ASSERT(p.getNamespace(2) != 0);
        attr = entry.findAttribute(0, "attr");
        PEGASUS_TEST_ASSERT(attr != 0);
        PEGASUS_TEST_ASSERT(attr->nsType == 0);
        PEGASUS_TEST_ASSERT(strcmp(attr->name, "a:attr") == 0);
        PEGASUS_TEST_ASSERT(strcmp(attr->localName, "attr") == 0);
        PEGASUS_TEST_ASSERT(strcmp(attr->value, "true") == 0);
        PEGASUS_TEST_ASSERT(entry.findAttribute(1, "attr") == 0);
        attr = entry.findAttribute(2, "ignore");
        PEGASUS_TEST_ASSERT(attr != 0);
        PEGASUS_TEST_ASSERT(attr->nsType == 2);
        PEGASUS_TEST_ASSERT(strcmp(attr->name, "ignore") == 0);
        PEGASUS_TEST_ASSERT(strcmp(attr->localName, "ignore") == 0);
        PEGASUS_TEST_ASSERT(strcmp(attr->value, "false") == 0);

        // <tag>
        PEGASUS_TEST_ASSERT(p.next(entry));

        if (hideEmptyTags)
            PEGASUS_TEST_ASSERT(entry.type == XmlEntry::START_TAG);
        else
            PEGASUS_TEST_ASSERT(entry.type == XmlEntry::EMPTY_TAG);

        PEGASUS_TEST_ASSERT(!strcmp(entry.text, "tag"));
        PEGASUS_TEST_ASSERT(entry.nsType == 2);
        PEGASUS_TEST_ASSERT(!strcmp(entry.localName, "tag"));

        if (hideEmptyTags)
        {
            // </tag>
            PEGASUS_TEST_ASSERT(p.next(entry));
            PEGASUS_TEST_ASSERT(entry.type == XmlEntry::END_TAG);
            PEGASUS_TEST_ASSERT(!strcmp(entry.text, "tag"));
            PEGASUS_TEST_ASSERT(entry.nsType == 2);
            PEGASUS_TEST_ASSERT(!strcmp(entry.localName, "tag"));
        }

        // </b:tag>
        PEGASUS_TEST_ASSERT(p.next(entry));
        PEGASUS_TEST_ASSERT(entry.type == XmlEntry::END_TAG);
        PEGASUS_TEST_ASSERT(!strcmp(entry.text, "b:tag"));
        PEGASUS_TEST_ASSERT(entry.nsType == 1);
        PEGASUS_TEST_ASSERT(!strcmp(entry.localName, "tag"));
        PEGASUS_TEST_ASSERT(p.getNamespace(0) != 0);
        PEGASUS_TEST_ASSERT(p.getNamespace(1) != 0);
        PEGASUS_TEST_ASSERT(p.getNamespace(2) != 0);

        // <tag xmlns=\"urn:0\" xml:lang=\"en-US\">
        PEGASUS_TEST_ASSERT(p.next(entry));
        PEGASUS_TEST_ASSERT(entry.type == XmlEntry::START_TAG);
        PEGASUS_TEST_ASSERT(!strcmp(entry.text, "tag"));
        PEGASUS_TEST_ASSERT(entry.nsType == 0);
        PEGASUS_TEST_ASSERT(!strcmp(entry.localName, "tag"));
        PEGASUS_TEST_ASSERT(p.getNamespace(0) != 0);
        PEGASUS_TEST_ASSERT(p.getNamespace(1) != 0);
        PEGASUS_TEST_ASSERT(p.getNamespace(2) == 0);
        attr = entry.findAttribute("xml:lang");
        PEGASUS_TEST_ASSERT(attr != 0);
        PEGASUS_TEST_ASSERT(attr->nsType == -1);
        PEGASUS_TEST_ASSERT(strcmp(attr->name, "xml:lang") == 0);
        PEGASUS_TEST_ASSERT(strcmp(attr->localName, "lang") == 0);
        PEGASUS_TEST_ASSERT(strcmp(attr->value, "en-US") == 0);

        // Data
        PEGASUS_TEST_ASSERT(p.next(entry));
        PEGASUS_TEST_ASSERT(entry.type == XmlEntry::CONTENT);
        PEGASUS_TEST_ASSERT(!strcmp(entry.text, "Data"));

        // </tag>
        PEGASUS_TEST_ASSERT(p.next(entry));
        PEGASUS_TEST_ASSERT(entry.type == XmlEntry::END_TAG);
        PEGASUS_TEST_ASSERT(!strcmp(entry.text, "tag"));
        PEGASUS_TEST_ASSERT(entry.nsType == 0);
        PEGASUS_TEST_ASSERT(!strcmp(entry.localName, "tag"));

        // <d:tag xmlns:d=\"urn:x\">
        PEGASUS_TEST_ASSERT(p.next(entry));

        if (hideEmptyTags)
            PEGASUS_TEST_ASSERT(entry.type == XmlEntry::START_TAG);
        else
            PEGASUS_TEST_ASSERT(entry.type == XmlEntry::EMPTY_TAG);

        PEGASUS_TEST_ASSERT(!strcmp(entry.text, "d:tag"));
        PEGASUS_TEST_ASSERT(entry.nsType == -2);
        PEGASUS_TEST_ASSERT(!strcmp(entry.localName, "tag"));
        PEGASUS_TEST_ASSERT(p.getNamespace(-2) != 0);

        if (hideEmptyTags)
        {
            // </d:tag>
            PEGASUS_TEST_ASSERT(p.next(entry));
            PEGASUS_TEST_ASSERT(entry.type == XmlEntry::END_TAG);
            PEGASUS_TEST_ASSERT(!strcmp(entry.text, "d:tag"));
            PEGASUS_TEST_ASSERT(entry.nsType == -2);
            PEGASUS_TEST_ASSERT(!strcmp(entry.localName, "tag"));
            PEGASUS_TEST_ASSERT(p.getNamespace(-2) != 0);
        }

        // <b:tag xmlns:b=\"urn:1\">
        PEGASUS_TEST_ASSERT(p.next(entry));

        if (hideEmptyTags)
            PEGASUS_TEST_ASSERT(entry.type == XmlEntry::START_TAG);
        else
            PEGASUS_TEST_ASSERT(entry.type == XmlEntry::EMPTY_TAG);

        PEGASUS_TEST_ASSERT(!strcmp(entry.text, "b:tag"));
        PEGASUS_TEST_ASSERT(entry.nsType == 1);
        PEGASUS_TEST_ASSERT(!strcmp(entry.localName, "tag"));
        PEGASUS_TEST_ASSERT(p.getNamespace(-2) == 0);
        PEGASUS_TEST_ASSERT(p.getNamespace(0) != 0);
        PEGASUS_TEST_ASSERT(p.getNamespace(1) != 0);
        PEGASUS_TEST_ASSERT(p.getNamespace(2) == 0);

        if (hideEmptyTags)
        {
            // </b:tag xmlns:b=\"urn:1\">
            PEGASUS_TEST_ASSERT(p.next(entry));
            PEGASUS_TEST_ASSERT(entry.type == XmlEntry::END_TAG);
            PEGASUS_TEST_ASSERT(!strcmp(entry.text, "b:tag"));
            PEGASUS_TEST_ASSERT(entry.nsType == 1);
            PEGASUS_TEST_ASSERT(!strcmp(entry.localName, "tag"));
            PEGASUS_TEST_ASSERT(p.getNamespace(-2) == 0);
            PEGASUS_TEST_ASSERT(p.getNamespace(0) != 0);
            PEGASUS_TEST_ASSERT(p.getNamespace(1) != 0);
            PEGASUS_TEST_ASSERT(p.getNamespace(2) == 0);
        }

        // </a:tag>
        PEGASUS_TEST_ASSERT(p.next(entry));
        PEGASUS_TEST_ASSERT(entry.type == XmlEntry::END_TAG);
        PEGASUS_TEST_ASSERT(!strcmp(entry.text, "a:tag"));
        PEGASUS_TEST_ASSERT(entry.nsType == 0);
        PEGASUS_TEST_ASSERT(!strcmp(entry.localName, "tag"));
        PEGASUS_TEST_ASSERT(p.getNamespace(0) != 0);
        PEGASUS_TEST_ASSERT(p.getNamespace(1) != 0);
    }

    // Test undeclared namespace
    {
        char xmlContent[] = "<a:tag xmlns:b=\"urn:1\"/>";
        XmlParser p(xmlContent, testNamespaces, hideEmptyTags);
        ASSERT_XML_EXCEPTION(p.next(entry));
    }

    // Test invalid QNames
    {
        char xmlContent[] = "<.a:tag xmlns:a=\"urn:0\"/>";
        XmlParser p(xmlContent, testNamespaces, hideEmptyTags);
        ASSERT_XML_EXCEPTION(p.next(entry));
    }

    {
        char xmlContent[] = "<a&:tag xmlns:a=\"urn:0\"/>";
        XmlParser p(xmlContent, testNamespaces, hideEmptyTags);
        ASSERT_XML_EXCEPTION(p.next(entry));
    }

    {
        char xmlContent[] = "<a:.tag xmlns:a=\"urn:0\"/>";
        XmlParser p(xmlContent, testNamespaces, hideEmptyTags);
        ASSERT_XML_EXCEPTION(p.next(entry));
    }

    {
        char xmlContent[] = "<a:ta:g xmlns:a=\"urn:0\"/>";
        XmlParser p(xmlContent, testNamespaces, hideEmptyTags);
        ASSERT_XML_EXCEPTION(p.next(entry));
    }

    {
        char xmlContent[] = "<a:ta";
        XmlParser p(xmlContent, testNamespaces, hideEmptyTags);
        ASSERT_XML_EXCEPTION(p.next(entry));
    }
}

static void testWhitespaceHandling()
{
    char text[] = "<tag attr=\"\r\nvalue  +  \">\r\nvalue  +  </tag>";
    XmlParser parser(text);
    XmlEntry entry;

    // Make sure attribute value has whitespace normalized.

    parser.next(entry);
    const char* attrValue;
    PEGASUS_TEST_ASSERT(entry.getAttributeValue("attr", attrValue));
    PEGASUS_TEST_ASSERT(strcmp(attrValue, "value +") == 0);

    // Make sure element value has leading and trailing whitespace trimmed,
    // but internal whitespace is not compressed.

    parser.next(entry);
    PEGASUS_TEST_ASSERT(entry.type == XmlEntry::CONTENT);
    PEGASUS_TEST_ASSERT(strcmp(entry.text, "value  +") == 0);
}

int main(int argc, char** argv)
{

    verbose = (getenv ("PEGASUS_TEST_VERBOSE")) ? true : false;

    if (argc < 2)
    {
        cerr << "Usage: " << argv[0] << " xml-filename ..." << endl;
        exit(1);
    }

    for (Uint32 i = 1; i < Uint32(argc); i++)
    {
        try
        {
            _parseFile(argv[i], true);
            _parseFile(argv[i], false);
        }
        catch (Exception& e)
        {
            cerr << "Error: " << e.getMessage() << endl;
            exit(1);
        }
    }

    testWhitespaceHandling();

    testNamespaceSupport(true);
    testNamespaceSupport(false);

    cout << argv[0] << " +++++ passed all tests" << endl;

    return 0;
}
