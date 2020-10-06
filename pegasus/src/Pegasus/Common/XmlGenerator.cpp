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

#include <cstdlib>
#include <cstdio>

#include <Pegasus/Common/Config.h>
#include "XmlGenerator.h"
#include "Constants.h"
#include "StrLit.h"
#include "CommonUTF.h"
#include "StringConversion.h"
#include "LanguageParser.h"
#include "AutoPtr.h"

PEGASUS_NAMESPACE_BEGIN

////////////////////////////////////////////////////////////////////////////////
//
// SpecialChar and table.
//
////////////////////////////////////////////////////////////////////////////////

// Note: we cannot use StrLit here since it has a constructor (forbids
// structure initialization).

struct SpecialChar
{
    const char* str;
    Uint32 size;
};

// Defines encodings of special characters. Just use a 7-bit ASCII character
// as an index into this array to retrieve its string encoding and encoding
// length in bytes.
static const SpecialChar _specialChars[] =
{
    {STRLIT_ARGS("&#0;")},
    {STRLIT_ARGS("&#1;")},
    {STRLIT_ARGS("&#2;")},
    {STRLIT_ARGS("&#3;")},
    {STRLIT_ARGS("&#4;")},
    {STRLIT_ARGS("&#5;")},
    {STRLIT_ARGS("&#6;")},
    {STRLIT_ARGS("&#7;")},
    {STRLIT_ARGS("&#8;")},
    {STRLIT_ARGS("&#9;")},
    {STRLIT_ARGS("&#10;")},
    {STRLIT_ARGS("&#11;")},
    {STRLIT_ARGS("&#12;")},
    {STRLIT_ARGS("&#13;")},
    {STRLIT_ARGS("&#14;")},
    {STRLIT_ARGS("&#15;")},
    {STRLIT_ARGS("&#16;")},
    {STRLIT_ARGS("&#17;")},
    {STRLIT_ARGS("&#18;")},
    {STRLIT_ARGS("&#19;")},
    {STRLIT_ARGS("&#20;")},
    {STRLIT_ARGS("&#21;")},
    {STRLIT_ARGS("&#22;")},
    {STRLIT_ARGS("&#23;")},
    {STRLIT_ARGS("&#24;")},
    {STRLIT_ARGS("&#25;")},
    {STRLIT_ARGS("&#26;")},
    {STRLIT_ARGS("&#27;")},
    {STRLIT_ARGS("&#28;")},
    {STRLIT_ARGS("&#29;")},
    {STRLIT_ARGS("&#30;")},
    {STRLIT_ARGS("&#31;")},
    {STRLIT_ARGS(" ")},
    {STRLIT_ARGS("!")},
    {STRLIT_ARGS("&quot;")},
    {STRLIT_ARGS("#")},
    {STRLIT_ARGS("$")},
    {STRLIT_ARGS("%")},
    {STRLIT_ARGS("&amp;")},
    {STRLIT_ARGS("&apos;")},
    {STRLIT_ARGS("(")},
    {STRLIT_ARGS(")")},
    {STRLIT_ARGS("*")},
    {STRLIT_ARGS("+")},
    {STRLIT_ARGS(",")},
    {STRLIT_ARGS("-")},
    {STRLIT_ARGS(".")},
    {STRLIT_ARGS("/")},
    {STRLIT_ARGS("0")},
    {STRLIT_ARGS("1")},
    {STRLIT_ARGS("2")},
    {STRLIT_ARGS("3")},
    {STRLIT_ARGS("4")},
    {STRLIT_ARGS("5")},
    {STRLIT_ARGS("6")},
    {STRLIT_ARGS("7")},
    {STRLIT_ARGS("8")},
    {STRLIT_ARGS("9")},
    {STRLIT_ARGS(":")},
    {STRLIT_ARGS(";")},
    {STRLIT_ARGS("&lt;")},
    {STRLIT_ARGS("=")},
    {STRLIT_ARGS("&gt;")},
    {STRLIT_ARGS("?")},
    {STRLIT_ARGS("@")},
    {STRLIT_ARGS("A")},
    {STRLIT_ARGS("B")},
    {STRLIT_ARGS("C")},
    {STRLIT_ARGS("D")},
    {STRLIT_ARGS("E")},
    {STRLIT_ARGS("F")},
    {STRLIT_ARGS("G")},
    {STRLIT_ARGS("H")},
    {STRLIT_ARGS("I")},
    {STRLIT_ARGS("J")},
    {STRLIT_ARGS("K")},
    {STRLIT_ARGS("L")},
    {STRLIT_ARGS("M")},
    {STRLIT_ARGS("N")},
    {STRLIT_ARGS("O")},
    {STRLIT_ARGS("P")},
    {STRLIT_ARGS("Q")},
    {STRLIT_ARGS("R")},
    {STRLIT_ARGS("S")},
    {STRLIT_ARGS("T")},
    {STRLIT_ARGS("U")},
    {STRLIT_ARGS("V")},
    {STRLIT_ARGS("W")},
    {STRLIT_ARGS("X")},
    {STRLIT_ARGS("Y")},
    {STRLIT_ARGS("Z")},
    {STRLIT_ARGS("[")},
    {STRLIT_ARGS("\\")},
    {STRLIT_ARGS("]")},
    {STRLIT_ARGS("^")},
    {STRLIT_ARGS("_")},
    {STRLIT_ARGS("`")},
    {STRLIT_ARGS("a")},
    {STRLIT_ARGS("b")},
    {STRLIT_ARGS("c")},
    {STRLIT_ARGS("d")},
    {STRLIT_ARGS("e")},
    {STRLIT_ARGS("f")},
    {STRLIT_ARGS("g")},
    {STRLIT_ARGS("h")},
    {STRLIT_ARGS("i")},
    {STRLIT_ARGS("j")},
    {STRLIT_ARGS("k")},
    {STRLIT_ARGS("l")},
    {STRLIT_ARGS("m")},
    {STRLIT_ARGS("n")},
    {STRLIT_ARGS("o")},
    {STRLIT_ARGS("p")},
    {STRLIT_ARGS("q")},
    {STRLIT_ARGS("r")},
    {STRLIT_ARGS("s")},
    {STRLIT_ARGS("t")},
    {STRLIT_ARGS("u")},
    {STRLIT_ARGS("v")},
    {STRLIT_ARGS("w")},
    {STRLIT_ARGS("x")},
    {STRLIT_ARGS("y")},
    {STRLIT_ARGS("z")},
    {STRLIT_ARGS("{")},
    {STRLIT_ARGS("|")},
    {STRLIT_ARGS("}")},
    {STRLIT_ARGS("~")},
    {STRLIT_ARGS("&#127;")},
};

// If _isSpecialChar7[ch] is true, then ch is a special character, which must
// have a special encoding in XML.
// Remaining 128 values are automatically initialised to 0 by compiler.
static const int _isSpecialChar7[256] =
{
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,1,0,0,
    0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,1,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
};

// If _isSpecialChar7[ch] is true, then ch is a special character, which must
// have a special encoding in XML.
static const int _isNormalChar7[] =
{
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,1,1,
    1,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,0,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,
// remaining 128 values are used on multi-byte UTF-8 and should not be escaped
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
};


////////////////////////////////////////////////////////////////////////////////

Buffer& operator<<(Buffer& out, const Char16& x)
{
    XmlGenerator::append(out, x);
    return out;
}

Buffer& operator<<(Buffer& out, const String& x)
{
    XmlGenerator::append(out, x);
    return out;
}

Buffer& operator<<(Buffer& out, const Buffer& x)
{
    out.append(x.getData(), x.size());
    return out;
}

Buffer& operator<<(Buffer& out, Uint32 x)
{
    XmlGenerator::append(out, x);
    return out;
}

Buffer& operator<<(Buffer& out, const CIMName& name)
{
    XmlGenerator::append(out, name.getString ());
    return out;
}

Buffer& operator<<(Buffer& out, const AcceptLanguageList& al)
{
    XmlGenerator::append(out, LanguageParser::buildAcceptLanguageHeader(al));
    return out;
}

Buffer& operator<<(Buffer& out, const ContentLanguageList& cl)
{
    XmlGenerator::append(out, LanguageParser::buildContentLanguageHeader(cl));
    return out;
}

const StrLit XmlGenerator::_XmlWriterTypeStrings[17] =
{
    STRLIT("TYPE=\"boolean\""),   STRLIT("TYPE=\"uint8\""),
    STRLIT("TYPE=\"sint8\""),     STRLIT("TYPE=\"uint16\""),
    STRLIT("TYPE=\"sint16\""),    STRLIT("TYPE=\"uint32\""),
    STRLIT("TYPE=\"sint32\""),    STRLIT("TYPE=\"uint64\""),
    STRLIT("TYPE=\"sint64\""),    STRLIT("TYPE=\"real32\""),
    STRLIT("TYPE=\"real64\""),    STRLIT("TYPE=\"char16\""),
    STRLIT("TYPE=\"string\""),    STRLIT("TYPE=\"datetime\""),
    STRLIT("TYPE=\"reference\""), STRLIT("TYPE=\"object\""),
    STRLIT("TYPE=\"instance\"")
};

const StrLit XmlGenerator::_XmlWriterKeyTypeStrings[17] =
{
    STRLIT("boolean"), STRLIT("numeric"),
    STRLIT("numeric"), STRLIT("numeric"),
    STRLIT("numeric"), STRLIT("numeric"),
    STRLIT("numeric"), STRLIT("numeric"),
    STRLIT("numeric"), STRLIT("numeric"),
    STRLIT("numeric"), STRLIT("string"),
    STRLIT("string"),  STRLIT("string"),
    /* The following are not valid values for a keytype, but left in here
       so in case something is going wrong it can be easily concluded from the
       generated XML */
    STRLIT("reference"), STRLIT("object"),
    STRLIT("instance")
};

void XmlGenerator::_appendChar(Buffer& out, const Char16& c)
{
    // We need to convert the Char16 to UTF8 then append the UTF8
    // character into the array.
    // NOTE: The UTF8 character could be several bytes long.
    // WARNING: This function will put in replacement character for
    // all characters that have surogate pairs.
    char str[6];
    memset(str,0x00,sizeof(str));
    Uint8* charIN = (Uint8 *)&c;

    const Uint16 *strsrc = (Uint16 *)charIN;
    Uint16 *endsrc = (Uint16 *)&charIN[1];

    Uint8 *strtgt = (Uint8 *)str;
    Uint8 *endtgt = (Uint8 *)&str[5];

    UTF16toUTF8(
        &strsrc,
        endsrc,
        &strtgt,
        endtgt);

    out.append(str, UTF_8_COUNT_TRAIL_BYTES(str[0]) + 1);
}

void XmlGenerator::_appendSpecialChar7(Buffer& out, char c)
{
    if (_isSpecialChar7[int(c)])
        out.append(_specialChars[int(c)].str, _specialChars[int(c)].size);
    else
        out.append(c);
}

void XmlGenerator::_appendSpecialChar(Buffer& out, const Char16& c)
{
    if (c < 128)
        _appendSpecialChar7(out, char(c));
    else
        _appendChar(out, c);
}


                                   
//Based on the platforms CHAR_MIN == -127 or CHAR_MIN == 0
//The above value is used to decide if the platform assumes
//char as signed char or unsigned char required to stop build
//btreaks for PPC under linux
static inline Boolean isSpecialChar(const char &c) 
{ 
#if CHAR_MIN < 0
    return (((c < 0x20) && (c >= 0)) || (c == 0x7f)); 
#else
    return ((c < 0x20) || (c == 0x7f)); 
#endif
}

void XmlGenerator::_appendSpecialChar(PEGASUS_STD(ostream)& os, char c)
{
    if(isSpecialChar(c))
    {
        char scratchBuffer[22];
        Uint32 outputLength;
        const char * output = Uint8ToString(scratchBuffer,
                                            static_cast<Uint8>(c),
                                            outputLength);
        os << "&#" << output << ";";
    }
    else
    {
        switch (c)
        {
            case '&':
                os << "&amp;";
                break;

            case '<':
                os << "&lt;";
                break;

            case '>':
                os << "&gt;";
                break;

            case '"':
                os << "&quot;";
                break;

            case '\'':
                os << "&apos;";
                break;

            default:
                os << c;
        }
    }
}

void XmlGenerator::_appendSurrogatePair(Buffer& out, Uint16 high, Uint16 low)
{
    char str[6];
    Uint8 charIN[5];
    memset(str,0x00,sizeof(str));
    memcpy(&charIN,&high,2);
    memcpy(&charIN[2],&low,2);
    const Uint16 *strsrc = (Uint16 *)charIN;
    Uint16 *endsrc = (Uint16 *)&charIN[3];

    Uint8 *strtgt = (Uint8 *)str;
    Uint8 *endtgt = (Uint8 *)&str[5];

    UTF16toUTF8(
        &strsrc,
        endsrc,
        &strtgt,
        endtgt);

    Uint32 number1 = UTF_8_COUNT_TRAIL_BYTES(str[0]) + 1;
    out.append(str,number1);
}

void XmlGenerator::_appendSpecial(PEGASUS_STD(ostream)& os, const char* str)
{
    while (*str)
        _appendSpecialChar(os, *str++);
}

void XmlGenerator::append(Buffer& out, const Char16& x)
{
    _appendChar(out, x);
}

void XmlGenerator::append(Buffer& out, Boolean x)
{
    if (x)
        out.append(STRLIT_ARGS("TRUE"));
    else
        out.append(STRLIT_ARGS("FALSE"));
}

void XmlGenerator::append(Buffer& out, Uint32 x)
{
    Uint32 outputLength=0;
    char buffer[22];
    const char * output = Uint32ToString(buffer, x, outputLength);
    out.append(output, outputLength);
}

void XmlGenerator::append(Buffer& out, Sint32 x)
{
    Uint32 outputLength=0;
    char buffer[22];
    const char * output = Sint32ToString(buffer, x, outputLength);
    out.append(output, outputLength);
}

void XmlGenerator::append(Buffer& out, Uint64 x)
{
    Uint32 outputLength=0;
    char buffer[22];
    const char * output = Uint64ToString(buffer, x, outputLength);
    out.append(output, outputLength);
}

void XmlGenerator::append(Buffer& out, Sint64 x)
{
    Uint32 outputLength=0;
    char buffer[22];
    const char * output = Sint64ToString(buffer, x, outputLength);
    out.append(output, outputLength);
}

void XmlGenerator::append(Buffer& out, Real32 x)
{
    Uint32 outputLength=0;
    char buffer[128];
    const char * output = Real32ToString(buffer, x, outputLength);
    out.append(output, outputLength);
}

void XmlGenerator::append(Buffer& out, Real64 x)
{
    Uint32 outputLength=0;
    char buffer[128];
    const char * output = Real64ToString(buffer, x, outputLength);
    out.append(output, outputLength);
}

void XmlGenerator::append(Buffer& out, const char* str)
{
    size_t n = strlen(str);
    out.append(str, n);
}

void XmlGenerator::append(Buffer& out, const String& str)
{
    const Uint16* p = (const Uint16*)str.getChar16Data();
    size_t n = str.size();

    // Handle leading ASCII 7 characers in these next two loops (use unrolling).

    while (n >= 8 && ((p[0]|p[1]|p[2]|p[3]|p[4]|p[5]|p[6]|p[7]) & 0xFF80) == 0)
    {
        out.append(p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7]);
        p += 8;
        n -= 8;
    }

    while (n >= 4 && ((p[0]|p[1]|p[2]|p[3]) & 0xFF80) == 0)
    {
        out.append(p[0], p[1], p[2], p[3]);
        p += 4;
        n -= 4;
    }

    while (n--)
    {
        Uint16 c = *p++;

        // Special processing for UTF8 case:

        if (c < 128)
        {
            out.append(c);
            continue;
        }

        // Handle UTF8 case (if reached).

        if (((c >= FIRST_HIGH_SURROGATE) && (c <= LAST_HIGH_SURROGATE)) ||
            ((c >= FIRST_LOW_SURROGATE) && (c <= LAST_LOW_SURROGATE)))
        {
            Char16 highSurrogate = p[-1];
            Char16 lowSurrogate = p[0];
            p++;
            n--;

            _appendSurrogatePair(
                out, Uint16(highSurrogate),Uint16(lowSurrogate));
        }
        else
        {
            _appendChar(out, c);
        }
    }
}

void XmlGenerator::appendSpecial(Buffer& out, const Char16& x)
{
    _appendSpecialChar(out, x);
}

void XmlGenerator::appendSpecial(Buffer& out, char x)
{
    _appendSpecialChar7(out, x);
}

void XmlGenerator::appendSpecial(Buffer& out, const char* str)
{
    while (*str)
        _appendSpecialChar7(out, *str++);
}

void XmlGenerator::appendSpecial(Buffer& out, const String& str)
{
    const Uint16* p = (const Uint16*)str.getChar16Data();
    // prevCharIsSpace is true when the last character written to the Buffer
    // is a space character (not a character reference).
    Boolean prevCharIsSpace = false;

    // If the first character is a space, use a character reference to avoid
    // space compression.
    if (*p == ' ')
    {
        out.append(STRLIT_ARGS("&#32;"));
        p++;
    }

    Uint16 c;
    while ((c = *p++) != 0)
    {
        if (c < 128)
        {
            if (_isSpecialChar7[c])
            {
                // Write the character reference for the special character
                out.append(
                    _specialChars[int(c)].str, _specialChars[int(c)].size);
                prevCharIsSpace = false;
            }
            else if (prevCharIsSpace && (c == ' '))
            {
                // Write the character reference for the space character, to
                // avoid compression
                out.append(STRLIT_ARGS("&#32;"));
                prevCharIsSpace = false;
            }
            else
            {
                out.append(c);
                prevCharIsSpace = (c == ' ');
            }
        }
        else
        {
            // Handle UTF8 case

            if ((((c >= FIRST_HIGH_SURROGATE) && (c <= LAST_HIGH_SURROGATE)) ||
                 ((c >= FIRST_LOW_SURROGATE) && (c <= LAST_LOW_SURROGATE))) &&
                *p)
            {
                _appendSurrogatePair(out, c, *p++);
            }
            else
            {
                _appendChar(out, c);
            }

            prevCharIsSpace = false;
        }
    }

    // If the last character is a space, use a character reference to avoid
    // space compression.
    if (prevCharIsSpace)
    {
        out.remove(out.size() - 1);
        out.append(STRLIT_ARGS("&#32;"));
    }
}

// str has to be UTF-8 encoded
// that means the characters used cannot be larger than 7bit ASCII in value
// range
void XmlGenerator::appendSpecial(Buffer& out, const char* str, Uint32 size)
{
    // employ loop unrolling and a less checking optimized Buffer access

    // Buffer cannot grow more than 6*size characters (ie. 4*size+2*size)
    Uint32 newMaxSize = (size << 2) + (size << 1);
    if (out.size() + newMaxSize >= out.capacity())
    {
        out.reserveCapacity(out.capacity() + newMaxSize);
    }

    // Before using a loop unrolled algorithm to pick out the special chars
    // we are going to assume there is no special char as this is the case most
    // of the time anyway
    Uint32 sizeStart=size;
    const Uint8* p= (const Uint8*) str;

    while (size >= 4 &&
             (_isNormalChar7[p[0]] &
              _isNormalChar7[p[1]] &
              _isNormalChar7[p[2]] &
              _isNormalChar7[p[3]]))
    {
        size -= 4;
        p += 4;
    }
    out.append_unchecked(str,sizeStart-size);
    str=(const char*)p;

    while (size>=8)
    {
        register Uint8 c;
        c = str[0];
        if (_isSpecialChar7[c])
        {
            out.append_unchecked(
                _specialChars[c].str,
                _specialChars[c].size);
        }
        else
        {
            out.append_unchecked(c);
        }
        c = str[1];
        if (_isSpecialChar7[c])
        {
            out.append_unchecked(
                _specialChars[c].str,
                _specialChars[c].size);
        }
        else
        {
            out.append_unchecked(c);
        }
        c = str[2];
        if (_isSpecialChar7[c])
        {
            out.append_unchecked(
                _specialChars[c].str,
                _specialChars[c].size);
        }
        else
        {
            out.append_unchecked(c);
        }
        c = str[3];
        if (_isSpecialChar7[c])
        {
            out.append_unchecked(
                _specialChars[c].str,
                _specialChars[c].size);
        }
        else
        {
            out.append_unchecked(c);
        }
        c = str[4];
        if (_isSpecialChar7[c])
        {
            out.append_unchecked(
                _specialChars[c].str,
                _specialChars[c].size);
        }
        else
        {
            out.append_unchecked(c);
        }
        c = str[5];
        if (_isSpecialChar7[c])
        {
            out.append_unchecked(
                _specialChars[c].str,
                _specialChars[c].size);
        }
        else
        {
            out.append_unchecked(c);
        }
        c = str[6];
        if (_isSpecialChar7[c])
        {
            out.append_unchecked(
                _specialChars[c].str,
                _specialChars[c].size);
        }
        else
        {
            out.append_unchecked(c);
        }
        c = str[7];
        if (_isSpecialChar7[c])
        {
            out.append_unchecked(
                _specialChars[c].str,
                _specialChars[c].size);
        }
        else
        {
            out.append_unchecked(c);
        }
        str+=8;
        size-=8;
    }

    while (size>=4)
    {
        register Uint8 c;
        c = str[0];
        if (_isSpecialChar7[c])
        {
            out.append_unchecked(
                _specialChars[c].str,
                _specialChars[c].size);
        }
        else
        {
            out.append_unchecked(c);
        }
        c = str[1];
        if (_isSpecialChar7[c])
        {
            out.append_unchecked(
                _specialChars[c].str,
                _specialChars[c].size);
        }
        else
        {
            out.append_unchecked(c);
        }
        c = str[2];
        if (_isSpecialChar7[c])
        {
            out.append_unchecked(
                _specialChars[c].str,
                _specialChars[c].size);
        }
        else
        {
            out.append_unchecked(c);
        }
        c = str[3];
        if (_isSpecialChar7[c])
        {
            out.append_unchecked(
                _specialChars[c].str,
                _specialChars[c].size);
        }
        else
        {
            out.append_unchecked(c);
        }
        str+=4;
        size-=4;
    }

    while (size--)
    {
        register Uint8 c;
        c=*str;
        if (_isSpecialChar7[c])
        {
            out.append_unchecked(
                _specialChars[c].str,
                _specialChars[c].size);
        }
        else
        {
            out.append_unchecked(c);
        }
        str++;
    }
}


// See http://www.ietf.org/rfc/rfc2396.txt section 2
// Reserved characters = ';' '/' '?' ':' '@' '&' '=' '+' '$' ','
// Excluded characters:
//   Control characters = 0x00-0x1f, 0x7f
//   Space character = 0x20
//   Delimiters = '<' '>' '#' '%' '"'
//   Unwise = '{' '}' '|' '\\' '^' '[' ']' '`'
//

static const char _is_uri[128] =
{
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,
    1,1,0,0,0,0,1,1,0,0,1,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,1,
};

// Perform the necessary URI encoding of characters in HTTP header values.
// This is required by the HTTP/1.1 specification and the CIM/HTTP
// Specification (section 3.3.2).
void XmlGenerator::_encodeURIChar(String& outString, Sint8 char8)
{
    Uint8 c = (Uint8)char8;
    if (c > 127 || _is_uri[int(c)])
    {
        char hexencoding[4];
        int n = sprintf(hexencoding, "%%%X%X", c/16, c%16);
        outString.append(hexencoding, n);
    }
    else
    {
        outString.append((Uint16)c);
    }
}

String XmlGenerator::encodeURICharacters(const String& uriString)
{
    String encodedString;

    // See the "CIM Operations over HTTP" spec, section 3.3.2 and
    // 3.3.3, for the treatment of non US-ASCII (UTF-8) chars

    // First, convert to UTF-8 (include handling of surrogate pairs)
    Buffer utf8;
    for (Uint32 i = 0; i < uriString.size(); i++)
    {
        Uint16 c = uriString[i];

        if (((c >= FIRST_HIGH_SURROGATE) && (c <= LAST_HIGH_SURROGATE)) ||
            ((c >= FIRST_LOW_SURROGATE) && (c <= LAST_LOW_SURROGATE)))
        {
            Char16 highSurrogate = uriString[i];
            Char16 lowSurrogate = uriString[++i];

            _appendSurrogatePair(
                utf8, Uint16(highSurrogate),Uint16(lowSurrogate));
        }
        else
        {
            _appendChar(utf8, uriString[i]);
        }
    }

    // Second, escape the non HTTP-safe chars
    for (Uint32 i=0; i<utf8.size(); i++)
    {
        _encodeURIChar(encodedString, utf8[i]);
    }

    return encodedString;
}

//------------------------------------------------------------------------------
//
// _printAttributes()
//
//------------------------------------------------------------------------------

void XmlGenerator::_printAttributes(
    PEGASUS_STD(ostream)& os,
    const XmlAttribute* attributes,
    Uint32 attributeCount)
{
    for (Uint32 i = 0; i < attributeCount; i++)
    {
        os << attributes[i].name << "=";

        os << '"';
        _appendSpecial(os, attributes[i].value);
        os << '"';

        if (i + 1 != attributeCount)
            os << ' ';
    }
}

//------------------------------------------------------------------------------
//
// _indent()
//
//------------------------------------------------------------------------------

void XmlGenerator::_indent(
    PEGASUS_STD(ostream)& os,
    Uint32 level,
    Uint32 indentChars)
{
    Uint32 n = level * indentChars;

    for (Uint32 i = 0; i < n; i++)
        os << ' ';
}

//------------------------------------------------------------------------------
//
// indentedPrint()
//
//------------------------------------------------------------------------------

void XmlGenerator::indentedPrint(
    PEGASUS_STD(ostream)& os,
    const char* text,
    Uint32 indentChars)
{
    AutoArrayPtr<char> tmp(strcpy(new char[strlen(text) + 1], text));

    XmlParser parser(tmp.get());
    XmlEntry entry;
    Stack<const char*> stack;

    while (parser.next(entry))
    {
        switch (entry.type)
        {
            case XmlEntry::XML_DECLARATION:
            {
                _indent(os, stack.size(), indentChars);

                os << "<?" << entry.text << " ";
                _printAttributes(
                    os, entry.attributes.getData(), entry.attributes.size());
                os << "?>";
                break;
            }

            case XmlEntry::START_TAG:
            {
                _indent(os, stack.size(), indentChars);

                os << "<" << entry.text;

                if (entry.attributes.size())
                    os << ' ';

                _printAttributes(
                    os, entry.attributes.getData(), entry.attributes.size());
                os << ">";
                stack.push(entry.text);
                break;
            }

            case XmlEntry::EMPTY_TAG:
            {
                _indent(os, stack.size(), indentChars);

                os << "<" << entry.text << " ";
                _printAttributes(
                    os, entry.attributes.getData(), entry.attributes.size());
                os << "/>";
                break;
            }

            case XmlEntry::END_TAG:
            {
                if (!stack.isEmpty() && strcmp(stack.top(), entry.text) == 0)
                    stack.pop();

                _indent(os, stack.size(), indentChars);

                os << "</" << entry.text << ">";
                break;
            }

            case XmlEntry::COMMENT:
            {
                _indent(os, stack.size(), indentChars);
                os << "<!--";
                _appendSpecial(os, entry.text);
                os << "-->";
                break;
            }

            case XmlEntry::CONTENT:
            {
                _indent(os, stack.size(), indentChars);
                _appendSpecial(os, entry.text);
                break;
            }

            case XmlEntry::CDATA:
            {
                _indent(os, stack.size(), indentChars);
                os << "<![CDATA[" << entry.text << "]]>";
                break;
            }

            case XmlEntry::DOCTYPE:
            {
                _indent(os, stack.size(), indentChars);
                os << "<!DOCTYPE...>";
                break;
            }
        }

        os << PEGASUS_STD(endl);
    }
}

PEGASUS_NAMESPACE_END
