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

#ifndef Pegasus_XmlGenerator_h
#define Pegasus_XmlGenerator_h

#include <iostream>
#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/Linkage.h>
#include <Pegasus/Common/ArrayInternal.h>
#include <Pegasus/Common/String.h>
#include <Pegasus/Common/CIMName.h>
#include <Pegasus/Common/Buffer.h>
#include <Pegasus/Common/AcceptLanguageList.h>
#include <Pegasus/Common/ContentLanguageList.h>
#include <Pegasus/Common/XmlParser.h>

PEGASUS_NAMESPACE_BEGIN

class PEGASUS_COMMON_LINKAGE XmlGenerator
{
public:

    static void append(Buffer& out, char x)
    {
        out.append(x);
    }

    static void append(Buffer& out, const Char16& x);
    static void append(Buffer& out, Boolean x);
    static void append(Buffer& out, Uint32 x);
    static void append(Buffer& out, Sint32 x);
    static void append(Buffer& out, Uint64 x);
    static void append(Buffer& out, Sint64 x);
    static void append(Buffer& out, Real32 x);
    static void append(Buffer& out, Real64 x);
    static void append(Buffer& out, const char* str);
    static void append(Buffer& out, const String& str);

    static void appendSpecial(Buffer& out, const Char16& x);
    static void appendSpecial(Buffer& out, char x);
    static void appendSpecial(Buffer& out, const char* str);
    static void appendSpecial(Buffer& out, const String& str);

    static void appendSpecial(Buffer& out, const char* str, Uint32 size);

    static String encodeURICharacters(const String& uriString);

    static void indentedPrint(
        PEGASUS_STD(ostream)& os,
        const char* text,
        Uint32 indentChars = 2);

    static StrLit xmlWriterTypeStrings(CIMType type)
    {
        return _XmlWriterTypeStrings[type];
    }
    
    static StrLit xmlWriterKeyTypeStrings(CIMType type)
    {
        return _XmlWriterKeyTypeStrings[type];
    }

private:

    static void _appendChar(Buffer& out, const Char16& c);
    static void _appendSpecialChar7(Buffer& out, char c);
    static void _appendSpecialChar(Buffer& out, const Char16& c);
    static void _appendSpecialChar(PEGASUS_STD(ostream)& os, char c);
    static void _appendSurrogatePair(
        Buffer& out, Uint16 high, Uint16 low);
    static void _appendSpecial(
        PEGASUS_STD(ostream)& os, const char* str);
#ifdef PEGASUS_OS_TYPE_WINDOWS
    static void _normalizeRealValueString(char *str);
#endif

    static void _encodeURIChar(String& outString, Sint8 char8);

    static void _printAttributes(
        PEGASUS_STD(ostream)& os,
        const XmlAttribute* attributes,
        Uint32 attributeCount);

    static void _indent(
        PEGASUS_STD(ostream)& os,
        Uint32 level,
        Uint32 indentChars);

        XmlGenerator();

    static const StrLit _XmlWriterTypeStrings[17];
    static const StrLit _XmlWriterKeyTypeStrings[17];
};

PEGASUS_COMMON_LINKAGE Buffer& operator<<(
    Buffer& out,
    const char* x);

inline Buffer& operator<<(Buffer& out, char x)
{
    out.append(x);
    return out;
}

inline Buffer& operator<<(Buffer& out, const char* s)
{
    out.append(s, (Uint32)strlen(s));
    return out;
}

PEGASUS_COMMON_LINKAGE Buffer& operator<<(
    Buffer& out,
    const AcceptLanguageList& al);

PEGASUS_COMMON_LINKAGE Buffer& operator<<(
    Buffer& out,
    const ContentLanguageList& cl);

PEGASUS_COMMON_LINKAGE Buffer& operator<<(Buffer& out, const Char16& x);

PEGASUS_COMMON_LINKAGE Buffer& operator<<(
    Buffer& out,
    const String& x);

PEGASUS_COMMON_LINKAGE Buffer& operator<<(
    Buffer& out,
    const Buffer& x);

PEGASUS_COMMON_LINKAGE Buffer& operator<<(
    Buffer& out,
    Uint32 x);

PEGASUS_COMMON_LINKAGE Buffer& operator<<(
    Buffer& out,
    const CIMName& name);

// This is a shortcut macro for outputing content length. This
// pads the output number to the max characters representing a Uint32 number
// so that it can be overwritten easily with a transfer encoding line later
// on in HTTPConnection if required. This is strictly for performance since
// messages can be very large. This overwriting shortcut allows us to NOT have
// to repackage a large message later.

#define OUTPUT_CONTENTLENGTH(out, contentLength)                           \
{                                                                          \
    char contentLengthP[11];                                               \
    int n = sprintf(contentLengthP,"%.10u", (unsigned int)contentLength);  \
    out << STRLIT("content-length: ");                                     \
    out.append(contentLengthP, n);                                         \
    out << STRLIT("\r\n");                                                 \
}

PEGASUS_NAMESPACE_END

#endif /* Pegasus_XmlGenerator_h */
