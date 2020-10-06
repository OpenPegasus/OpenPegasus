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

#include <Pegasus/Common/PegasusAssert.h>
#include <Pegasus/Common/StringConversion.h>
#include <cstring>
#include <iostream>
#include <math.h>

PEGASUS_USING_PEGASUS;
PEGASUS_USING_STD;

const Sint64 HI = 1024 * 100;
const Sint64 LO = -HI;

template<class T>
struct Cvt
{
};

template<>
struct Cvt<Uint8>
{
    static const char* func(char buffer[22], Uint8 x, Uint32& size)
    {
        return Uint8ToString(buffer, x, size);
    }
};

template<>
struct Cvt<Uint16>
{
    static const char* func(char buffer[22], Uint16 x, Uint32& size)
    {
        return Uint16ToString(buffer, x, size);
    }
};

template<>
struct Cvt<Uint32>
{
    static const char* func(char buffer[22], Uint32 x, Uint32& size)
    {
        return Uint32ToString(buffer, x, size);
    }
};

template<>
struct Cvt<Uint64>
{
    static const char* func(char buffer[22], Uint64 x, Uint32& size)
    {
        return Uint64ToString(buffer, x, size);
    }
};

template<>
struct Cvt<Sint8>
{
    static const char* func(char buffer[22], Sint8 x, Uint32& size)
    {
        return Sint8ToString(buffer, x, size);
    }
};

template<>
struct Cvt<Sint16>
{
    static const char* func(char buffer[22], Sint16 x, Uint32& size)
    {
        return Sint16ToString(buffer, x, size);
    }
};

template<>
struct Cvt<Sint32>
{
    static const char* func(char buffer[22], Sint32 x, Uint32& size)
    {
        return Sint32ToString(buffer, x, size);
    }
};

template<>
struct Cvt<Sint64>
{
    static const char* func(char buffer[22], Sint64 x, Uint32& size)
    {
        return Sint64ToString(buffer, x, size);
    }
};

template<class S, class U, Sint64 LO, Sint64 HI>
struct Test
{
    static void testUint(const char* format)
    {
        for (Uint64 i = 0; i <= Uint64(HI); i++)
        {
            const char* str;
            char buffer1[32];
            Uint32 size;
            str = Cvt<U>::func(buffer1, U(i), size);

            char buffer2[32];
            sprintf(buffer2, format, U(i));

            PEGASUS_TEST_ASSERT(strcmp(str, buffer2) == 0);
            PEGASUS_TEST_ASSERT(strlen(str) == size);
        }
    }

    static void testSint(const char* format)
    {
        for (Sint64 i = LO; i <= HI; i++)
        {
            const char* str;
            char buffer1[32];
            Uint32 size;
            str = Cvt<S>::func(buffer1, S(i), size);

            char buffer2[32];
            S s = S(i);    // Resolves a non-truncation issue on HP-UX 11.00
            sprintf(buffer2, format, s);

            PEGASUS_TEST_ASSERT(strcmp(str, buffer2) == 0);
            PEGASUS_TEST_ASSERT(strlen(str) == size);
        }
    }
};

void testIntegerToStringConversions()
{
    Test<Sint8, Uint8, -128, 127>::testUint("%u");
    Test<Sint16, Uint16, 0, 65535>::testUint("%u");
    Test<Sint32, Uint32, LO, HI>::testUint("%u");
    Test<Sint64, Uint64, LO, HI>::testUint(
        "%" PEGASUS_64BIT_CONVERSION_WIDTH "u");
    Test<Sint8, Uint8, 0, 255>::testSint("%d");
    Test<Sint16, Uint16, 0, 65535>::testSint("%d");
    Test<Sint32, Uint32, LO, HI>::testSint("%d");
    Test<Sint64, Uint64, LO, HI>::testSint(
        "%" PEGASUS_64BIT_CONVERSION_WIDTH "d");
}

void testHexCharToNumeric()
{
    PEGASUS_TEST_ASSERT(StringConversion::hexCharToNumeric('0') == 0);
    PEGASUS_TEST_ASSERT(StringConversion::hexCharToNumeric('1') == 1);
    PEGASUS_TEST_ASSERT(StringConversion::hexCharToNumeric('2') == 2);
    PEGASUS_TEST_ASSERT(StringConversion::hexCharToNumeric('3') == 3);
    PEGASUS_TEST_ASSERT(StringConversion::hexCharToNumeric('4') == 4);
    PEGASUS_TEST_ASSERT(StringConversion::hexCharToNumeric('5') == 5);
    PEGASUS_TEST_ASSERT(StringConversion::hexCharToNumeric('6') == 6);
    PEGASUS_TEST_ASSERT(StringConversion::hexCharToNumeric('7') == 7);
    PEGASUS_TEST_ASSERT(StringConversion::hexCharToNumeric('8') == 8);
    PEGASUS_TEST_ASSERT(StringConversion::hexCharToNumeric('9') == 9);
    PEGASUS_TEST_ASSERT(StringConversion::hexCharToNumeric('a') == 10);
    PEGASUS_TEST_ASSERT(StringConversion::hexCharToNumeric('A') == 10);
    PEGASUS_TEST_ASSERT(StringConversion::hexCharToNumeric('b') == 11);
    PEGASUS_TEST_ASSERT(StringConversion::hexCharToNumeric('B') == 11);
    PEGASUS_TEST_ASSERT(StringConversion::hexCharToNumeric('c') == 12);
    PEGASUS_TEST_ASSERT(StringConversion::hexCharToNumeric('C') == 12);
    PEGASUS_TEST_ASSERT(StringConversion::hexCharToNumeric('d') == 13);
    PEGASUS_TEST_ASSERT(StringConversion::hexCharToNumeric('D') == 13);
    PEGASUS_TEST_ASSERT(StringConversion::hexCharToNumeric('e') == 14);
    PEGASUS_TEST_ASSERT(StringConversion::hexCharToNumeric('E') == 14);
    PEGASUS_TEST_ASSERT(StringConversion::hexCharToNumeric('f') == 15);
    PEGASUS_TEST_ASSERT(StringConversion::hexCharToNumeric('F') == 15);
}

void testDecimalStringToUint64()
{
    Uint64 u64;

    PEGASUS_TEST_ASSERT(
        StringConversion::decimalStringToUint64("0", u64));
    PEGASUS_TEST_ASSERT(u64 == 0);
    PEGASUS_TEST_ASSERT(
        StringConversion::decimalStringToUint64("18446744073709551615", u64));
    PEGASUS_TEST_ASSERT(u64 == PEGASUS_UINT64_LITERAL(0xFFFFFFFFFFFFFFFF));

    PEGASUS_TEST_ASSERT(
        !StringConversion::decimalStringToUint64(0, u64));
    PEGASUS_TEST_ASSERT(
        !StringConversion::decimalStringToUint64("", u64));
    PEGASUS_TEST_ASSERT(
        !StringConversion::decimalStringToUint64("01", u64));
    PEGASUS_TEST_ASSERT(
        !StringConversion::decimalStringToUint64("18446744073709551616", u64));
    PEGASUS_TEST_ASSERT(
        !StringConversion::decimalStringToUint64("184467440737095516150", u64));
    PEGASUS_TEST_ASSERT(
        !StringConversion::decimalStringToUint64("-1", u64));
    PEGASUS_TEST_ASSERT(
        !StringConversion::decimalStringToUint64("1234F", u64));
    PEGASUS_TEST_ASSERT(
        !StringConversion::decimalStringToUint64("C0", u64));
}

void testHexStringToUint64()
{
    Uint64 u64;

    PEGASUS_TEST_ASSERT(
        StringConversion::hexStringToUint64("0X0", u64));
    PEGASUS_TEST_ASSERT(u64 == 0);
    PEGASUS_TEST_ASSERT(
        StringConversion::hexStringToUint64("0xFFFFFFFFFFFFFFFF", u64));
    PEGASUS_TEST_ASSERT(u64 == PEGASUS_UINT64_LITERAL(0xFFFFFFFFFFFFFFFF));
    PEGASUS_TEST_ASSERT(
        StringConversion::hexStringToUint64("0X0000000100000000000", u64));
    PEGASUS_TEST_ASSERT(u64 == PEGASUS_UINT64_LITERAL(0x100000000000));

    PEGASUS_TEST_ASSERT(
        !StringConversion::hexStringToUint64(0, u64));
    PEGASUS_TEST_ASSERT(
        !StringConversion::hexStringToUint64("", u64));
    PEGASUS_TEST_ASSERT(
        !StringConversion::hexStringToUint64("0", u64));
    PEGASUS_TEST_ASSERT(
        !StringConversion::hexStringToUint64("01", u64));
    PEGASUS_TEST_ASSERT(
        !StringConversion::hexStringToUint64("0y0", u64));
    PEGASUS_TEST_ASSERT(
        !StringConversion::hexStringToUint64("0x10000000000000000", u64));
    PEGASUS_TEST_ASSERT(
        !StringConversion::hexStringToUint64("-0x0", u64));
    PEGASUS_TEST_ASSERT(
        !StringConversion::hexStringToUint64("0xAG", u64));
}

void testOctalStringToUint64()
{
    Uint64 u64;

    PEGASUS_TEST_ASSERT(
        StringConversion::octalStringToUint64("00", u64));
    PEGASUS_TEST_ASSERT(u64 == 0);
    PEGASUS_TEST_ASSERT(
        StringConversion::octalStringToUint64("01777777777777777777777", u64));
    PEGASUS_TEST_ASSERT(u64 == PEGASUS_UINT64_LITERAL(0xFFFFFFFFFFFFFFFF));
    PEGASUS_TEST_ASSERT(
        StringConversion::octalStringToUint64("00000000100000000000", u64));
    PEGASUS_TEST_ASSERT(u64 == PEGASUS_UINT64_LITERAL(0x200000000));

    PEGASUS_TEST_ASSERT(
        !StringConversion::octalStringToUint64(0, u64));
    PEGASUS_TEST_ASSERT(
        !StringConversion::octalStringToUint64("", u64));
    PEGASUS_TEST_ASSERT(
        !StringConversion::octalStringToUint64("0", u64));
    PEGASUS_TEST_ASSERT(
        !StringConversion::octalStringToUint64("1", u64));
    PEGASUS_TEST_ASSERT(
        !StringConversion::octalStringToUint64("08", u64));
    PEGASUS_TEST_ASSERT(
        !StringConversion::octalStringToUint64("0A", u64));
    PEGASUS_TEST_ASSERT(
        !StringConversion::octalStringToUint64("02000000000000000000000", u64));
    PEGASUS_TEST_ASSERT(
        !StringConversion::octalStringToUint64("-01", u64));
    PEGASUS_TEST_ASSERT(
        !StringConversion::octalStringToUint64("01b", u64));
}

void testBinaryStringToUint64()
{
    Uint64 u64;

    PEGASUS_TEST_ASSERT(
        StringConversion::binaryStringToUint64("0b", u64));
    PEGASUS_TEST_ASSERT(u64 == 0);
    PEGASUS_TEST_ASSERT(
        StringConversion::binaryStringToUint64(
            "1111111111111111111111111111111111111111111111111111111111111111B",
            u64));
    PEGASUS_TEST_ASSERT(u64 == PEGASUS_UINT64_LITERAL(0xFFFFFFFFFFFFFFFF));
    PEGASUS_TEST_ASSERT(
        StringConversion::binaryStringToUint64("0000000100000000000b", u64));
    PEGASUS_TEST_ASSERT(u64 == 2048);

    PEGASUS_TEST_ASSERT(
        !StringConversion::binaryStringToUint64(0, u64));
    PEGASUS_TEST_ASSERT(
        !StringConversion::binaryStringToUint64("", u64));
    PEGASUS_TEST_ASSERT(
        !StringConversion::binaryStringToUint64("0", u64));
    PEGASUS_TEST_ASSERT(
        !StringConversion::binaryStringToUint64("b", u64));
    PEGASUS_TEST_ASSERT(
        !StringConversion::binaryStringToUint64("01", u64));
    PEGASUS_TEST_ASSERT(
        !StringConversion::binaryStringToUint64(
           "10000000000000000000000000000000000000000000000000000000000000000B",
           u64));
    PEGASUS_TEST_ASSERT(
        !StringConversion::binaryStringToUint64("-0b", u64));
    PEGASUS_TEST_ASSERT(
        !StringConversion::binaryStringToUint64("012B", u64));
    PEGASUS_TEST_ASSERT(
        !StringConversion::binaryStringToUint64("01c", u64));
}

void testCheckUintBounds()
{
    PEGASUS_TEST_ASSERT(StringConversion::checkUintBounds(
        255, CIMTYPE_UINT8));
    PEGASUS_TEST_ASSERT(!StringConversion::checkUintBounds(
        256, CIMTYPE_UINT8));

    PEGASUS_TEST_ASSERT(StringConversion::checkUintBounds(
        0xFFFF, CIMTYPE_UINT16));
    PEGASUS_TEST_ASSERT(!StringConversion::checkUintBounds(
        0x10000, CIMTYPE_UINT16));

    PEGASUS_TEST_ASSERT(StringConversion::checkUintBounds(
        PEGASUS_UINT64_LITERAL(0x00000000FFFFFFFF), CIMTYPE_UINT32));
    PEGASUS_TEST_ASSERT(!StringConversion::checkUintBounds(
        PEGASUS_UINT64_LITERAL(0x0000000100000000), CIMTYPE_UINT32));

    PEGASUS_TEST_ASSERT(StringConversion::checkUintBounds(
        PEGASUS_UINT64_LITERAL(0xFFFFFFFFFFFFFFFF), CIMTYPE_UINT64));
}

void testStringToSint64()
{
    Sint64 s64;

    PEGASUS_TEST_ASSERT(StringConversion::stringToSint64(
        "0", StringConversion::decimalStringToUint64, s64));
    PEGASUS_TEST_ASSERT(s64 == 0);
    PEGASUS_TEST_ASSERT(StringConversion::stringToSint64(
        "+9223372036854775807", StringConversion::decimalStringToUint64, s64));
    PEGASUS_TEST_ASSERT(s64 == PEGASUS_SINT64_LITERAL(0x7FFFFFFFFFFFFFFF));
    PEGASUS_TEST_ASSERT(StringConversion::stringToSint64(
        "-9223372036854775808", StringConversion::decimalStringToUint64, s64));
    PEGASUS_TEST_ASSERT(s64 == PEGASUS_SINT64_LITERAL(0x8000000000000000));

    PEGASUS_TEST_ASSERT(StringConversion::stringToSint64(
        "0x0", StringConversion::hexStringToUint64, s64));
    PEGASUS_TEST_ASSERT(s64 == 0);
    PEGASUS_TEST_ASSERT(StringConversion::stringToSint64(
        "+0x7FFFFFFFFFFFFFFF", StringConversion::hexStringToUint64, s64));
    PEGASUS_TEST_ASSERT(s64 == PEGASUS_SINT64_LITERAL(0x7FFFFFFFFFFFFFFF));
    PEGASUS_TEST_ASSERT(StringConversion::stringToSint64(
        "-0x8000000000000000", StringConversion::hexStringToUint64, s64));
    PEGASUS_TEST_ASSERT(s64 == PEGASUS_SINT64_LITERAL(0x8000000000000000));

    PEGASUS_TEST_ASSERT(StringConversion::stringToSint64(
        "00", StringConversion::octalStringToUint64, s64));
    PEGASUS_TEST_ASSERT(s64 == 0);
    PEGASUS_TEST_ASSERT(StringConversion::stringToSint64(
        "+0777777777777777777777", StringConversion::octalStringToUint64, s64));
    PEGASUS_TEST_ASSERT(s64 == PEGASUS_SINT64_LITERAL(0x7FFFFFFFFFFFFFFF));
    PEGASUS_TEST_ASSERT(StringConversion::stringToSint64(
        "-01000000000000000000000", StringConversion::octalStringToUint64,s64));
    PEGASUS_TEST_ASSERT(s64 == PEGASUS_SINT64_LITERAL(0x8000000000000000));

    PEGASUS_TEST_ASSERT(StringConversion::stringToSint64(
        "0b", StringConversion::binaryStringToUint64, s64));
    PEGASUS_TEST_ASSERT(s64 == 0);
    PEGASUS_TEST_ASSERT(StringConversion::stringToSint64(
        "+111111111111111111111111111111111111111111111111111111111111111b",
        StringConversion::binaryStringToUint64,
        s64));
    PEGASUS_TEST_ASSERT(s64 == PEGASUS_SINT64_LITERAL(0x7FFFFFFFFFFFFFFF));
    PEGASUS_TEST_ASSERT(StringConversion::stringToSint64(
        "-1000000000000000000000000000000000000000000000000000000000000000b",
        StringConversion::binaryStringToUint64,
        s64));
    PEGASUS_TEST_ASSERT(s64 == PEGASUS_SINT64_LITERAL(0x8000000000000000));

    PEGASUS_TEST_ASSERT(!StringConversion::stringToSint64(
        0, StringConversion::decimalStringToUint64, s64));
    PEGASUS_TEST_ASSERT(!StringConversion::stringToSint64(
        "", StringConversion::decimalStringToUint64, s64));

    PEGASUS_TEST_ASSERT(!StringConversion::stringToSint64(
        "+9223372036854775808", StringConversion::decimalStringToUint64, s64));
    PEGASUS_TEST_ASSERT(!StringConversion::stringToSint64(
        "-9223372036854775809", StringConversion::decimalStringToUint64, s64));

    PEGASUS_TEST_ASSERT(!StringConversion::stringToSint64(
        "+0x8000000000000000", StringConversion::hexStringToUint64, s64));
    PEGASUS_TEST_ASSERT(!StringConversion::stringToSint64(
        "-0x8000000000000001", StringConversion::hexStringToUint64, s64));

    PEGASUS_TEST_ASSERT(!StringConversion::stringToSint64(
        "+01000000000000000000000", StringConversion::octalStringToUint64,s64));
    PEGASUS_TEST_ASSERT(!StringConversion::stringToSint64(
        "-01000000000000000000001", StringConversion::octalStringToUint64,s64));

    PEGASUS_TEST_ASSERT(!StringConversion::stringToSint64(
        "+1000000000000000000000000000000000000000000000000000000000000000b",
        StringConversion::binaryStringToUint64,
        s64));
    PEGASUS_TEST_ASSERT(!StringConversion::stringToSint64(
        "-1000000000000000000000000000000000000000000000000000000000000001b",
        StringConversion::binaryStringToUint64,
        s64));
}

void testCheckSintBounds()
{
    PEGASUS_TEST_ASSERT(StringConversion::checkSintBounds(
        127, CIMTYPE_SINT8));
    PEGASUS_TEST_ASSERT(!StringConversion::checkSintBounds(
        128, CIMTYPE_SINT8));
    PEGASUS_TEST_ASSERT(StringConversion::checkSintBounds(
        -128, CIMTYPE_SINT8));
    PEGASUS_TEST_ASSERT(!StringConversion::checkSintBounds(
        -129, CIMTYPE_SINT8));

    PEGASUS_TEST_ASSERT(StringConversion::checkSintBounds(
        0x7FFF, CIMTYPE_SINT16));
    PEGASUS_TEST_ASSERT(!StringConversion::checkSintBounds(
        0x8000, CIMTYPE_SINT16));
    PEGASUS_TEST_ASSERT(StringConversion::checkSintBounds(
        -0x8000, CIMTYPE_SINT16));
    PEGASUS_TEST_ASSERT(!StringConversion::checkSintBounds(
        -0x8001, CIMTYPE_SINT16));

    PEGASUS_TEST_ASSERT(StringConversion::checkSintBounds(
        PEGASUS_SINT64_LITERAL(0x000000007FFFFFFF), CIMTYPE_SINT32));
    PEGASUS_TEST_ASSERT(!StringConversion::checkSintBounds(
        PEGASUS_SINT64_LITERAL(0x0000000080000000), CIMTYPE_SINT32));
    PEGASUS_TEST_ASSERT(StringConversion::checkSintBounds(
        PEGASUS_SINT64_LITERAL(0xFFFFFFFF80000000), CIMTYPE_SINT32));
    PEGASUS_TEST_ASSERT(!StringConversion::checkSintBounds(
        PEGASUS_SINT64_LITERAL(0xFFFFFFFF7FFFFFFF), CIMTYPE_SINT32));

    PEGASUS_TEST_ASSERT(StringConversion::checkSintBounds(
        PEGASUS_SINT64_LITERAL(0x7FFFFFFFFFFFFFFF), CIMTYPE_SINT64));
    PEGASUS_TEST_ASSERT(StringConversion::checkSintBounds(
        PEGASUS_SINT64_LITERAL(0x8000000000000000), CIMTYPE_SINT64));
}

void testStringToReal64()
{
    Real64 r64;

    PEGASUS_TEST_ASSERT(
        StringConversion::stringToReal64("0.0", r64));
    PEGASUS_TEST_ASSERT(r64 == 0);
    PEGASUS_TEST_ASSERT(
        StringConversion::stringToReal64("+0.0", r64));
    PEGASUS_TEST_ASSERT(r64 == 0);
    PEGASUS_TEST_ASSERT(
        StringConversion::stringToReal64("-0.0", r64));
    PEGASUS_TEST_ASSERT(r64 == 0);
    PEGASUS_TEST_ASSERT(
        StringConversion::stringToReal64("0.0e+1", r64));
    PEGASUS_TEST_ASSERT(r64 == 0);
    PEGASUS_TEST_ASSERT(
        StringConversion::stringToReal64("1.0e1", r64));
    PEGASUS_TEST_ASSERT(r64 == 10);
    PEGASUS_TEST_ASSERT(
        StringConversion::stringToReal64("10.0E-0001", r64));
    PEGASUS_TEST_ASSERT(r64 == 1.0);
    PEGASUS_TEST_ASSERT(
        StringConversion::stringToReal64(".1e1", r64));
    PEGASUS_TEST_ASSERT(r64 == 1.0);

    PEGASUS_TEST_ASSERT(
        !StringConversion::stringToReal64(0, r64));
    PEGASUS_TEST_ASSERT(
        !StringConversion::stringToReal64("", r64));
    PEGASUS_TEST_ASSERT(
        !StringConversion::stringToReal64("0", r64));
    PEGASUS_TEST_ASSERT(
        !StringConversion::stringToReal64("123a", r64));
    PEGASUS_TEST_ASSERT(
        !StringConversion::stringToReal64("1.e+1", r64));
    PEGASUS_TEST_ASSERT(
        !StringConversion::stringToReal64("1.123546789e", r64));
    PEGASUS_TEST_ASSERT(
        !StringConversion::stringToReal64("++1.0", r64));
    PEGASUS_TEST_ASSERT(
        !StringConversion::stringToReal64("1.1e", r64));
    PEGASUS_TEST_ASSERT(
        !StringConversion::stringToReal64("1.1e+", r64));
    PEGASUS_TEST_ASSERT(
        !StringConversion::stringToReal64("1.1e++1", r64));
    PEGASUS_TEST_ASSERT(
        !StringConversion::stringToReal64("1.1e+1a", r64));
    PEGASUS_TEST_ASSERT(
        !StringConversion::stringToReal64("1.1e+123456789", r64));
}

void testReal32ToString()
{
    Real32 r32;
    Real64 r64r;
    char buffer[128];
    Uint32 size;
    {
        r32 = 1.5;
        const char* result = Real32ToString(buffer, r32, size);
        PEGASUS_TEST_ASSERT(strlen(result) == size);
        PEGASUS_TEST_ASSERT(StringConversion::stringToReal64(result, r64r));
        PEGASUS_TEST_ASSERT(fabs(r32 - r64r) < r32/1000);
    }

    {
        r32 = 0;
        const char* result = Real32ToString(buffer, r32, size);
        PEGASUS_TEST_ASSERT(strlen(result) == size);
        PEGASUS_TEST_ASSERT(StringConversion::stringToReal64(result, r64r));
        PEGASUS_TEST_ASSERT(fabs(r32 - r64r) < .0001);
    }

    {
        r32 = 10.0E-0001;
        const char* result = Real32ToString(buffer, r32, size);
        PEGASUS_TEST_ASSERT(strlen(result) == size);
        PEGASUS_TEST_ASSERT(StringConversion::stringToReal64(result, r64r));
        PEGASUS_TEST_ASSERT(fabs(r32 - r64r) < r32/1000);
    }
    {
        r32 = 10.0E+5;
        const char* result = Real32ToString(buffer, r32, size);
        PEGASUS_TEST_ASSERT(strlen(result) == size);
        PEGASUS_TEST_ASSERT(StringConversion::stringToReal64(result, r64r));
        PEGASUS_TEST_ASSERT(fabs(r32 - r64r) < r32/1000);
    }
}

void testReal64ToString()
{
    Real64 r64;
    Real64 r64r;
    char buffer[128];
    Uint32 size;
    {
        r64 = 1.5;
        const char* result = Real64ToString(buffer, r64, size);
        PEGASUS_TEST_ASSERT(strlen(result) == size);
        PEGASUS_TEST_ASSERT(StringConversion::stringToReal64(result, r64r));
        PEGASUS_TEST_ASSERT(fabs(r64 - r64r) < r64/1000);
    }

    {
        r64 = 0;
        const char* result = Real64ToString(buffer, r64, size);
        PEGASUS_TEST_ASSERT(strlen(result) == size);
        PEGASUS_TEST_ASSERT(StringConversion::stringToReal64(result, r64r));
        PEGASUS_TEST_ASSERT(fabs(r64 - r64r) < .0001);
    }

    {
        r64 = 10.0E-0001;
        const char* result = Real64ToString(buffer, r64, size);
        PEGASUS_TEST_ASSERT(strlen(result) == size);
        PEGASUS_TEST_ASSERT(StringConversion::stringToReal64(result, r64r));
        PEGASUS_TEST_ASSERT(fabs(r64 - r64r) < r64/1000);
    }
    {
        r64 = 10.0E+5;
        const char* result = Real64ToString(buffer, r64, size);
        PEGASUS_TEST_ASSERT(strlen(result) == size);
        PEGASUS_TEST_ASSERT(StringConversion::stringToReal64(result, r64r));
        PEGASUS_TEST_ASSERT(fabs(r64 - r64r) < r64/1000);
    }
}

void testOneStringVal(const String& s, Boolean rtn, Uint32 testValue)
{
    {
        Uint32 rtnVal;
        if (rtn)
        {
            PEGASUS_TEST_ASSERT(StringConversion::decimalStringToUint32(
                                                  s,rtnVal));
            PEGASUS_TEST_ASSERT(rtnVal == testValue);
        }
        else
        {
            PEGASUS_TEST_ASSERT(!StringConversion::decimalStringToUint32(
                                                   s,rtnVal));
            PEGASUS_TEST_ASSERT(rtnVal == 0);
        }
    }
}

void testOneVal(const char * s, Boolean rtn, Uint32 testValue)
{
    {
        Uint32 rtnVal;
        if (rtn)
        {
            PEGASUS_TEST_ASSERT(StringConversion::decimalStringToUint32(
                                                  s,rtnVal));
            PEGASUS_TEST_ASSERT(rtnVal == testValue);
        }
        else
        {
            PEGASUS_TEST_ASSERT(!StringConversion::decimalStringToUint32(
                                                   s,rtnVal));
            PEGASUS_TEST_ASSERT(rtnVal == 0);
        }
    }
}

void testDecimalStringToUint32()
{
    testOneVal("0", true, 0);
    testOneVal("1", true, 1);
    testOneVal("9999", true, 9999);
    testOneVal("4294967295", true, 4294967295);

    testOneVal("4294967296", false, 0);

    testOneVal("-1", false, 0);
    testOneVal("1a", false, 0);
    testOneVal("a1", false, 0);
    // error if input string is empty
    testOneVal("", false, 0);

    testOneStringVal(String("0"), true, 0);
    testOneStringVal(String("1"), true, 1);
    testOneStringVal(String("9999"), true, 9999);
    testOneStringVal(String("4294967295"), true, 4294967295);

    testOneStringVal(String("4294967296"), false, 0);

    testOneStringVal(String("-1"), false, 0);
    testOneStringVal(String("1a"), false, 0);
    testOneStringVal(String("a1"), false, 0);
    // error if input string is empty
    testOneStringVal(String(""), false, 0);
}

int main(int, char** argv)
{
    testIntegerToStringConversions();
    testHexCharToNumeric();
    testDecimalStringToUint64();
    testHexStringToUint64();
    testOctalStringToUint64();
    testBinaryStringToUint64();
    testStringToSint64();
    testCheckSintBounds();
    testStringToReal64();
    testReal32ToString();
    testReal64ToString();
    testDecimalStringToUint32();

    cout << argv[0] << " +++++ passed all tests" << endl;

    return 0;
}
