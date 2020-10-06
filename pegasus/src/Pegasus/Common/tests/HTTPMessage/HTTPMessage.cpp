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


#include <Pegasus/Common/HTTPMessage.h>
#include <Pegasus/Common/String.h>
#include <Pegasus/Common/PegasusAssert.h>

PEGASUS_USING_STD;
PEGASUS_USING_PEGASUS;

static void _testContentType()
{
    const char* contentType;
    String type;
    String charset;

    contentType = "application/xml    ;  charset =     utf-8  ";
    PEGASUS_TEST_ASSERT(HTTPMessage::parseContentTypeHeader(
        contentType, type, charset));
    PEGASUS_TEST_ASSERT(String::equalNoCase(type, "application/xml"));
    PEGASUS_TEST_ASSERT(String::equalNoCase(charset, "utf-8"));

    contentType = "text/xml    ;  CHARSET = utf-8";
    PEGASUS_TEST_ASSERT(HTTPMessage::parseContentTypeHeader(
        contentType, type, charset));
    PEGASUS_TEST_ASSERT(String::equalNoCase(type, "text/xml"));
    PEGASUS_TEST_ASSERT(String::equalNoCase(charset, "utf-8"));

    contentType = "   application/xml;charset=UTF-8   ";
    PEGASUS_TEST_ASSERT(HTTPMessage::parseContentTypeHeader(
        contentType, type, charset));
    PEGASUS_TEST_ASSERT(String::equalNoCase(type, "application/xml"));
    PEGASUS_TEST_ASSERT(String::equalNoCase(charset, "utf-8"));

    contentType = "application/xml    ;  charset =     \"utf-8\"  ";
    PEGASUS_TEST_ASSERT(HTTPMessage::parseContentTypeHeader(
        contentType, type, charset));
    PEGASUS_TEST_ASSERT(String::equalNoCase(type, "application/xml"));
    PEGASUS_TEST_ASSERT(String::equalNoCase(charset, "utf-8"));

    contentType = "text/xml    ;  CHARSET = \"utf-8\"";
    PEGASUS_TEST_ASSERT(HTTPMessage::parseContentTypeHeader(
        contentType, type, charset));
    PEGASUS_TEST_ASSERT(String::equalNoCase(type, "text/xml"));
    PEGASUS_TEST_ASSERT(String::equalNoCase(charset, "utf-8"));

    contentType = "   application/xml;charset=\"UTF-8\"   ";
    PEGASUS_TEST_ASSERT(HTTPMessage::parseContentTypeHeader(
        contentType, type, charset));
    PEGASUS_TEST_ASSERT(String::equalNoCase(type, "application/xml"));
    PEGASUS_TEST_ASSERT(String::equalNoCase(charset, "utf-8"));

    contentType = "application/xml";
    PEGASUS_TEST_ASSERT(HTTPMessage::parseContentTypeHeader(
        contentType, type, charset));
    PEGASUS_TEST_ASSERT(String::equalNoCase(type, "application/xml"));
    PEGASUS_TEST_ASSERT(String::equalNoCase(charset, "utf-8"));

    contentType = "TexT/XmL";
    PEGASUS_TEST_ASSERT(HTTPMessage::parseContentTypeHeader(
        contentType, type, charset));
    PEGASUS_TEST_ASSERT(String::equalNoCase(type, "text/xml"));
    PEGASUS_TEST_ASSERT(String::equalNoCase(charset, "utf-8"));

    contentType = "APPLICATION/XML;charset=utf-8";
    PEGASUS_TEST_ASSERT(HTTPMessage::parseContentTypeHeader(
        contentType, type, charset));
    PEGASUS_TEST_ASSERT(String::equalNoCase(type, "application/xml"));
    PEGASUS_TEST_ASSERT(String::equalNoCase(charset, "utf-8"));

    contentType = "TEXT/XML;CHARSET=UTF-8";
    PEGASUS_TEST_ASSERT(HTTPMessage::parseContentTypeHeader(
        contentType, type, charset));
    PEGASUS_TEST_ASSERT(String::equalNoCase(type, "text/xml"));
    PEGASUS_TEST_ASSERT(String::equalNoCase(charset, "utf-8"));

    contentType = "APPLICATION/XML;charset=\"utf-8\"";
    PEGASUS_TEST_ASSERT(HTTPMessage::parseContentTypeHeader(
        contentType, type, charset));
    PEGASUS_TEST_ASSERT(String::equalNoCase(type, "application/xml"));
    PEGASUS_TEST_ASSERT(String::equalNoCase(charset, "utf-8"));

    contentType = "TEXT/XML;CHARSET=\"UTF-8\"";
    PEGASUS_TEST_ASSERT(HTTPMessage::parseContentTypeHeader(
        contentType, type, charset));
    PEGASUS_TEST_ASSERT(String::equalNoCase(type, "text/xml"));
    PEGASUS_TEST_ASSERT(String::equalNoCase(charset, "utf-8"));

    contentType = "\ttext/xml\t    \t;\t  \tCHARSET\t = \tutf-8\t";
    PEGASUS_TEST_ASSERT(HTTPMessage::parseContentTypeHeader(
        contentType, type, charset));
    PEGASUS_TEST_ASSERT(String::equalNoCase(type, "text/xml"));
    PEGASUS_TEST_ASSERT(String::equalNoCase(charset, "utf-8"));

    // Test failure cases
    contentType = "application/xml   ;";
    PEGASUS_TEST_ASSERT(!HTTPMessage::parseContentTypeHeader(
        contentType, type, charset));

    contentType = "application/xml; charset=\"utf-8 \"";
    PEGASUS_TEST_ASSERT(HTTPMessage::parseContentTypeHeader(
        contentType, type, charset));
    PEGASUS_TEST_ASSERT(String::equalNoCase(type, "application/xml"));
    PEGASUS_TEST_ASSERT(!String::equalNoCase(charset, "utf-8"));

    contentType = "application/xml;  charset = \"utf-88\"";
    PEGASUS_TEST_ASSERT(HTTPMessage::parseContentTypeHeader(
        contentType, type, charset));
    PEGASUS_TEST_ASSERT(String::equalNoCase(type, "application/xml"));
    PEGASUS_TEST_ASSERT(!String::equalNoCase(charset, "utf-8"));

    contentType = "application/xml ;  charset = \"utf-8\" ;";
    PEGASUS_TEST_ASSERT(!HTTPMessage::parseContentTypeHeader(
        contentType, type, charset));

    contentType = "application/xml ;  charset  =";
    PEGASUS_TEST_ASSERT(HTTPMessage::parseContentTypeHeader(
        contentType, type, charset));
    PEGASUS_TEST_ASSERT(String::equalNoCase(type, "application/xml"));
    PEGASUS_TEST_ASSERT(charset == String::EMPTY);

    contentType = "application/xml ;  charset";
    PEGASUS_TEST_ASSERT(!HTTPMessage::parseContentTypeHeader(
        contentType, type, charset));

    contentType = "application/xml    ;   = \"utf-8\"";
    PEGASUS_TEST_ASSERT(!HTTPMessage::parseContentTypeHeader(
        contentType, type, charset));

    contentType = "text/xml; charset  = ututf-8 ";
    PEGASUS_TEST_ASSERT(HTTPMessage::parseContentTypeHeader(
        contentType, type, charset));
    PEGASUS_TEST_ASSERT(String::equalNoCase(type, "text/xml"));
    PEGASUS_TEST_ASSERT(!String::equalNoCase(charset, "utf-8"));

    contentType = "text/xml;  charset = \"utf-8";
    PEGASUS_TEST_ASSERT(!HTTPMessage::parseContentTypeHeader(
        contentType, type, charset));
}

int main(int, char** argv)
{
    _testContentType();

    cout << argv[0] << " +++++ passed all tests" << endl;

    return 0;
}
