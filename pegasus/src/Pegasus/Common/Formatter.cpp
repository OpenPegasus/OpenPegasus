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
#include <cstdio>
#include "Formatter.h"
#include "StrLit.h"

PEGASUS_NAMESPACE_BEGIN

const Formatter::Arg Formatter::DEFAULT_ARG;

String Formatter::Arg::toString() const
{
    switch (_type)
    {
        case INTEGER:
        {
            char buffer[32];
            sprintf(buffer, "%d", _integer);
            return buffer;
        }

        case UINTEGER:
        {
            char buffer[32];
            sprintf(buffer, "%u", _uinteger);
            return buffer;
        }

        case BOOLEAN:
        {
            //char buffer[32];
            //buffer = (_boolean ? "true": "false");
            //return buffer;
            return  (_boolean ? "true": "false");
        }

        case REAL:
        {
            char buffer[32];
            sprintf(buffer, "%f", _real);
            return buffer;
        }

        case LINTEGER:
        {
            char buffer[32];
            sprintf(buffer, "%" PEGASUS_64BIT_CONVERSION_WIDTH "d", _lInteger);
            return buffer;
        }

        case ULINTEGER:
        {
            char buffer[32];
            sprintf(buffer, "%" PEGASUS_64BIT_CONVERSION_WIDTH "u", _lUInteger);
            return buffer;
        }

        case STRING:
            return _string;

        case CSTRLIT:
            return String(_cstrlit->str, _cstrlit->size);
            break;

        case VOIDT:
        default:
            return String();
    }
}

struct StrRef
{
    const char* str;
    size_t size;
};

static const StrRef _num_strings[] =
{
    { STRLIT_ARGS("0") },
    { STRLIT_ARGS("1") },
    { STRLIT_ARGS("2") },
    { STRLIT_ARGS("3") },
    { STRLIT_ARGS("4") },
    { STRLIT_ARGS("5") },
    { STRLIT_ARGS("6") },
    { STRLIT_ARGS("7") },
    { STRLIT_ARGS("8") },
    { STRLIT_ARGS("9") },
    { STRLIT_ARGS("10") },
    { STRLIT_ARGS("11") },
    { STRLIT_ARGS("12") },
    { STRLIT_ARGS("13") },
    { STRLIT_ARGS("14") },
    { STRLIT_ARGS("15") },
    { STRLIT_ARGS("16") },
    { STRLIT_ARGS("17") },
    { STRLIT_ARGS("18") },
    { STRLIT_ARGS("19") },
    { STRLIT_ARGS("20") },
    { STRLIT_ARGS("21") },
    { STRLIT_ARGS("22") },
    { STRLIT_ARGS("23") },
    { STRLIT_ARGS("24") },
    { STRLIT_ARGS("25") },
    { STRLIT_ARGS("26") },
    { STRLIT_ARGS("27") },
    { STRLIT_ARGS("28") },
    { STRLIT_ARGS("29") },
    { STRLIT_ARGS("30") },
    { STRLIT_ARGS("31") },
    { STRLIT_ARGS("32") },
    { STRLIT_ARGS("33") },
    { STRLIT_ARGS("34") },
    { STRLIT_ARGS("35") },
    { STRLIT_ARGS("36") },
    { STRLIT_ARGS("37") },
    { STRLIT_ARGS("38") },
    { STRLIT_ARGS("39") },
    { STRLIT_ARGS("40") },
    { STRLIT_ARGS("41") },
    { STRLIT_ARGS("42") },
    { STRLIT_ARGS("43") },
    { STRLIT_ARGS("44") },
    { STRLIT_ARGS("45") },
    { STRLIT_ARGS("46") },
    { STRLIT_ARGS("47") },
    { STRLIT_ARGS("48") },
    { STRLIT_ARGS("49") },
    { STRLIT_ARGS("50") },
    { STRLIT_ARGS("51") },
    { STRLIT_ARGS("52") },
    { STRLIT_ARGS("53") },
    { STRLIT_ARGS("54") },
    { STRLIT_ARGS("55") },
    { STRLIT_ARGS("56") },
    { STRLIT_ARGS("57") },
    { STRLIT_ARGS("58") },
    { STRLIT_ARGS("59") },
    { STRLIT_ARGS("60") },
    { STRLIT_ARGS("61") },
    { STRLIT_ARGS("62") },
    { STRLIT_ARGS("63") },
    { STRLIT_ARGS("64") },
    { STRLIT_ARGS("65") },
    { STRLIT_ARGS("66") },
    { STRLIT_ARGS("67") },
    { STRLIT_ARGS("68") },
    { STRLIT_ARGS("69") },
    { STRLIT_ARGS("70") },
    { STRLIT_ARGS("71") },
    { STRLIT_ARGS("72") },
    { STRLIT_ARGS("73") },
    { STRLIT_ARGS("74") },
    { STRLIT_ARGS("75") },
    { STRLIT_ARGS("76") },
    { STRLIT_ARGS("77") },
    { STRLIT_ARGS("78") },
    { STRLIT_ARGS("79") },
    { STRLIT_ARGS("80") },
    { STRLIT_ARGS("81") },
    { STRLIT_ARGS("82") },
    { STRLIT_ARGS("83") },
    { STRLIT_ARGS("84") },
    { STRLIT_ARGS("85") },
    { STRLIT_ARGS("86") },
    { STRLIT_ARGS("87") },
    { STRLIT_ARGS("88") },
    { STRLIT_ARGS("89") },
    { STRLIT_ARGS("90") },
    { STRLIT_ARGS("91") },
    { STRLIT_ARGS("92") },
    { STRLIT_ARGS("93") },
    { STRLIT_ARGS("94") },
    { STRLIT_ARGS("95") },
    { STRLIT_ARGS("96") },
    { STRLIT_ARGS("97") },
    { STRLIT_ARGS("98") },
    { STRLIT_ARGS("99") },
    { STRLIT_ARGS("100") },
    { STRLIT_ARGS("101") },
    { STRLIT_ARGS("102") },
    { STRLIT_ARGS("103") },
    { STRLIT_ARGS("104") },
    { STRLIT_ARGS("105") },
    { STRLIT_ARGS("106") },
    { STRLIT_ARGS("107") },
    { STRLIT_ARGS("108") },
    { STRLIT_ARGS("109") },
    { STRLIT_ARGS("110") },
    { STRLIT_ARGS("111") },
    { STRLIT_ARGS("112") },
    { STRLIT_ARGS("113") },
    { STRLIT_ARGS("114") },
    { STRLIT_ARGS("115") },
    { STRLIT_ARGS("116") },
    { STRLIT_ARGS("117") },
    { STRLIT_ARGS("118") },
    { STRLIT_ARGS("119") },
    { STRLIT_ARGS("120") },
    { STRLIT_ARGS("121") },
    { STRLIT_ARGS("122") },
    { STRLIT_ARGS("123") },
    { STRLIT_ARGS("124") },
    { STRLIT_ARGS("125") },
    { STRLIT_ARGS("126") },
    { STRLIT_ARGS("127") },
};

template<class T>
static inline char* _append_unsigned(char* end, T x)
{
    char* p = end;

    do
    {
        *--p = '0' + static_cast<char>(x % 10);
    }
    while ((x /= 10) != 0);

    return p;
}

void Formatter::Arg::appendToString(String& out) const
{
    switch (_type)
    {
        case INTEGER:
        {
            Sint32 x = _integer;

            if (x >= 0 && x < 128)
                out.append(_num_strings[x].str, _num_strings[x].size);
            else
            {
                char buffer[32];
                int n = sprintf(buffer, "%d", x);
                out.append(buffer, n);
            }
            break;
        }

        case UINTEGER:
        {
            Uint32 x = _uinteger;

            if (x < 128)
            {
                out.append(_num_strings[x].str, _num_strings[x].size);
            }
            else
            {
                char buffer[32];
                int n = sprintf(buffer, "%u", x);
                out.append(buffer, n);
            }
            break;
        }

        case BOOLEAN:
        {
            if (_boolean)
                out.append("true", 4);
            else
                out.append("false", 5);
            break;
        }

        case REAL:
        {
            char buffer[32];
            int n = sprintf(buffer, "%f", _real);
            out.append(buffer, n);
            break;
        }

        case LINTEGER:
        {
            char buffer[32];
            int n = sprintf(
                buffer, "%" PEGASUS_64BIT_CONVERSION_WIDTH "d", _lInteger);
            out.append(buffer, n);
            break;
        }

        case ULINTEGER:
        {
            Uint64 x = _lUInteger;

            if (x < 128)
            {
                out.append(_num_strings[x].str, _num_strings[x].size);
            }
            else
            {
                char buffer[32];
                char* end = &buffer[32];
                char* p = _append_unsigned(end, x);
                out.append(p, (Uint32)(end - p));
            }
            break;
        }

        case STRING:
        {
            out.append(_string);
            break;
        }

        case CSTRLIT:
        {
            out.append(_cstrlit->str, _cstrlit->size);
            break;
        }

        case VOIDT:
        default:
            break;
    }
}

//
// Non-special characters (any but these: '$', '\0', '\\')
//
static const Uint8 _isNonSpecial[256] =
{
    0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
};

PEGASUS_USING_STD;

String Formatter::format(
    const String& format,
    const Arg& arg0,
    const Arg& arg1,
    const Arg& arg2,
    const Arg& arg3,
    const Arg& arg4,
    const Arg& arg5,
    const Arg& arg6,
    const Arg& arg7,
    const Arg& arg8,
    const Arg& arg9)
{
    String result;

#if 0
    cout << format << endl;
#endif

    result.reserveCapacity(256);

    const Uint16* p = (const Uint16*)format.getChar16Data();

    for (;;)
    {
        //// Skip over non-special characters:

        const Uint16* start = p;

        while (*p < 128 && _isNonSpecial[*p])
            p++;

        //// Append any non-special characters.

        size_t r = p - start;

        if (r)
            result.append((const Char16*)start, (Uint32)r);

        //// Process next special character:

        if (*p == '$')
        {
            Uint16 c = p[1];

            switch (c - '0')
            {
                case 0: arg0.appendToString(result); break;
                case 1: arg1.appendToString(result); break;
                case 2: arg2.appendToString(result); break;
                case 3: arg3.appendToString(result); break;
                case 4: arg4.appendToString(result); break;
                case 5: arg5.appendToString(result); break;
                case 6: arg6.appendToString(result); break;
                case 7: arg7.appendToString(result); break;
                case 8: arg8.appendToString(result); break;
                case 9: arg9.appendToString(result); break;
                default: break;
            }

            p += 2;
        }
        else if (*p == '\\')
        {
            result.append(p[1]);
            p += 2;
        }
        else if (*p == '\0')
        {
            break;
        }
        else
        {
            result.append(p[0]);
            p++;
        }
    }

#if 0
    cout << result << endl;
#endif

    return result;
}

PEGASUS_NAMESPACE_END
