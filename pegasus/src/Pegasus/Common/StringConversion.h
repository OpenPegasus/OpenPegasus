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

#ifndef Pegasus_String_Conversion_h
#define Pegasus_String_Conversion_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/Linkage.h>
#include <Pegasus/Common/CIMType.h>
#include <cctype>
#include <Pegasus/Common/String.h>

PEGASUS_NAMESPACE_BEGIN

// The following functions convert the integer "x" to a string. The "buffer"
// argument is a scratch area that may or may not be used in the conversion.
// These functions return a pointer to the converted string and set "size" to
// the length of that string.

PEGASUS_COMMON_LINKAGE
const char* Uint8ToString(char buffer[22], Uint8 x, Uint32& size);

PEGASUS_COMMON_LINKAGE
const char* Uint16ToString(char buffer[22], Uint16 x, Uint32& size);

PEGASUS_COMMON_LINKAGE
const char* Uint32ToString(char buffer[22], Uint32 x, Uint32& size);

PEGASUS_COMMON_LINKAGE
const char* Uint64ToString(char buffer[22], Uint64 x, Uint32& size);

PEGASUS_COMMON_LINKAGE
const char* Sint8ToString(char buffer[22], Sint8 x, Uint32& size);

PEGASUS_COMMON_LINKAGE
const char* Sint16ToString(char buffer[22], Sint16 x, Uint32& size);

PEGASUS_COMMON_LINKAGE
const char* Sint32ToString(char buffer[22], Sint32 x, Uint32& size);

PEGASUS_COMMON_LINKAGE
const char* Sint64ToString(char buffer[22], Sint64 x, Uint32& size);

PEGASUS_COMMON_LINKAGE
const char* Real32ToString(char buffer[128], Real32 x, Uint32& size);

PEGASUS_COMMON_LINKAGE
const char* Real64ToString(char buffer[128], Real64 x, Uint32& size);


class PEGASUS_COMMON_LINKAGE StringConversion
{
public:

    /**
        Converts a valid hexadecimal character to a Uint8 value.
        @param c The hexadecimal character to convert
        @return The converted Uint8 value
    */
    static inline Uint8 hexCharToNumeric(const char c)
    {
        Uint8 n;

        if (isdigit(c))
            n = (c - '0');
        else if (isupper(c))
            n = (c - 'A' + 10);
        else // if (islower(c))
            n = (c - 'a' + 10);

        return n;
    }

    /**
        Converts a character string to a Uint64 value according to the DMTF
        specifications for decimal formatting of integer values in MOF and XML.
        (The two specifications are identical.)
        @param stringValue The character string to convert
        @param x The converted Uint64 value
        @return true if the character string is well-formatted and the
            conversion is successful, false otherwise
    */
    static Boolean decimalStringToUint64(
        const char* stringValue,
        Uint64& x,
        Boolean allowLeadingZeros=false);

    /**
        Converts a character string to a Uint64 value according to the DMTF
        specifications for octal formatting of integer values in MOF
        and XML.  (The two specifications are identical.)
        @param stringValue The character string to convert
        @param x The converted Uint64 value
        @return true if the character string is well-formatted and the
            conversion is successful, false otherwise
    */
    static Boolean octalStringToUint64(
        const char* stringValue,
        Uint64& x,
        Boolean allowLeadingZeros = false);

    /**
        Converts a character string to a Uint64 value according to the DMTF
        specifications for hexadecimal formatting of integer values in MOF
        and XML.  (The two specifications are identical.)
        @param stringValue The character string to convert
        @param x The converted Uint64 value
        @return true if the character string is well-formatted and the
            conversion is successful, false otherwise
    */
    static Boolean hexStringToUint64(
        const char* stringValue,
        Uint64& x,
        Boolean allowLeadingZeros = false);

    /**
        Converts a character string to a Uint64 value according to the DMTF
        specifications for binary formatting of integer values in MOF
        and XML.  (The two specifications are identical.)
        @param stringValue The character string to convert
        @param x The converted Uint64 value
        @return true if the character string is well-formatted and the
            conversion is successful, false otherwise
    */
    static Boolean binaryStringToUint64(
        const char* stringValue,
        Uint64& x,
        Boolean allowLeadingZeros = false);

    /**
        Checks whether a specified Uint64 value will fit within a specified
        unsigned integer type (e.g., Uint8, Uint16, Uint32) without overflow.
        @param x The Uint64 value to check
        @param type A CIMType specifying the constraining unsigned integer size
        @return true if the specified integer fits within the specified type,
            false otherwise
    */
    static Boolean checkUintBounds(
        Uint64 x,
        CIMType type);

    /**
        Converts a character string to an Sint64 value by interpreting the
        optional leading sign character and using the specified function to
        convert the remainder of the string to a Uint64.  Bounds checking is
        performed when converting the Uint64 to Sint64.
        @param stringValue The character string to convert
        @param uint64Converter The function used to convert the unsigned
            portion of the string to a Uint64 value.
        @param x The converted Sint64 value
        @return true if the character string is well-formatted and the
            conversion is successful, false otherwise
    */
    static Boolean stringToSint64(
        const char* stringValue,
        Boolean (*uint64Converter)(const char*, Uint64&,Boolean),
        Sint64& x);

    /**
        Checks whether a specified Sint64 value will fit within a specified
        signed integer type (e.g., Sint8, Sint16, Sint32) without overflow.
        @param x The Sint64 value to check
        @param type A CIMType specifying the constraining signed integer size
        @return true if the specified integer fits within the specified type,
            false otherwise
    */
    static Boolean checkSintBounds(
        Sint64 x,
        CIMType type);

    /**
        Converts a character string to a Real64 value according to the DMTF
        specifications for formatting real values in MOF and XML.  (The two
        specifications are identical.)
        @param stringValue The character string to convert
        @param x The converted Real64 value
        @return true if the character string is well-formatted and the
            conversion is successful, false otherwise
    */
    static Boolean stringToReal64(
        const char* stringValue,
        Real64& x);

    static Boolean stringToSignedInteger(
        const char* stringValue,
        Sint64& x);

    static Boolean stringToUnsignedInteger(
        const char* stringValue,
        Uint64& x);

    /**
        If the string reperesentation is valid decimal, converts it and
        returns value in valueUint32 parameter.
        @param  StringWithValue   const char * valid decimal string
                                  representation of a Uint32
        @param  valueUint32       Returns the value as Uint32.
        @return Boolean           True if input string is a valid decimal
                                  Uint32. Otherwise returns false and
                                  sets valueUint32 to 0. Returns error
                                  if the input string parameter is empty
    */
    static Boolean decimalStringToUint32(
        const char * StringWithValue,
        Uint32& uint32Value );

    /**
        Same as above but with Pegasus String input
    */
    static Boolean decimalStringToUint32(
        const String& StringWithValue,
        Uint32& uint32Value);
};

/******************************************************************
   inline functions
*******************************************************************/
inline Boolean StringConversion::decimalStringToUint32(
        const String& StringWithValue,
        Uint32& uint32Value)
{
    return StringConversion::decimalStringToUint32(
        StringWithValue.getCString(),
        uint32Value);
}

PEGASUS_NAMESPACE_END

#endif /* Pegasus_String_Conversion_h */
