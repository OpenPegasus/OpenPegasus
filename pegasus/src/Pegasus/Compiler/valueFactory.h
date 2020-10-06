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


//
// Header for a class to generate CIMValue objects from String values
//

#ifndef _VALUEFACTORY_H_
#define _VALUEFACTORY_H_

#include <Pegasus/Common/String.h>
#include <Pegasus/Common/CIMValue.h>
#include <Pegasus/Compiler/Linkage.h>

PEGASUS_USING_PEGASUS;
PEGASUS_USING_STD;

// The valueFactory builds a CIMValue object given an indication of
// whether this is a Array type, the CIM type to be coerced, and
// a String representation of the value.
//
class PEGASUS_COMPILER_LINKAGE valueFactory
{
public:
    /**
        Escape any character comma found in the input string
        @param str String to be processed.
        @return String the processed string
     */
    static String stringEscapeComma(String str);

    /**
        Converts a String to a Uint64 according to the DMTF specification for
        MOF encoding.  A CIMType specification also allows for overflow
        checking of smaller unsigned integer type (e.g., Uint8, Uint16,
        Uint32).
        @param val The String to convert
        @param type A CIMType to use for bounds checking
        @exception Exception if the conversion is unsuccessful
        @return The converted Uint64 value
    */
    static Uint64 stringToUint(
        const String& val,
        CIMType type);

    /**
        Converts a String to a Sint64 according to the DMTF specification for
        MOF encoding.  A CIMType specification also allows for overflow
        checking of smaller unsigned integer type (e.g., Sint8, Sint16,
        Sint32).
        @param val The String to convert
        @param type A CIMType to use for bounds checking
        @exception Exception if the conversion is unsuccessful
        @return The converted Sint64 value
    */
    static Sint64 stringToSint(
        const String& val,
        CIMType type);

    /**
        Converts a String to a Rint64 according to the DMTF specification for
        MOF encoding.
        @param val The String to convert
        @param type A CIMType to use in an exception message
        @exception Exception if the conversion is unsuccessful
        @return The converted Real64 value
    */
    static Real64 stringToReal(
        const String& val,
        CIMType type);

    static CIMValue * createValue(CIMType type, int arrayDimension,
        Boolean isNull,
        int ParseType,
        const String *rep);

private:

    static CIMValue* _buildArrayValue(
        CIMType type,
        const String& rep);

    static Boolean compareTypeToParseType(CIMType type, int expectedType );
};

#endif
