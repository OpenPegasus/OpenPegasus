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

#ifndef Pegasus_Char16_h
#define Pegasus_Char16_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/Linkage.h>

PEGASUS_NAMESPACE_BEGIN

/** The Char16 class represents a CIM sixteen bit character (char16).
    This class is a trivial wrapper for a sixteen bit integer. It is used
    as the element type in the String class (used to represent the CIM
    string type). Ordinarily Uint16 could be used; however, a distinguishable
    type was needed for the purposes of function overloaded which occurs in
    the CIMValue class.
*/
class PEGASUS_COMMON_LINKAGE Char16
{
public:

    /**
        Constructs a Char16 with a null (0) value.
    */
    Char16();

    /**
        Constructs a Char16 with a specified 16-bit character value.
        @param x The Uint16 value with which to initialize the Char16.
    */
    Char16(Uint16 x);

    /**
        Constructs a Char16 by copying another Char16 value.
        @param x The Char16 object to copy.
    */
    Char16(const Char16& x);

    /**
        Destructs a Char16 object.
    */
    ~Char16();

    /**
        Assigns the value to a specified 16-bit character value.
        @param x The Uint16 value to copy.
    */
    Char16& operator=(Uint16 x);

    /**
        Assigns the value from another Char16 object.
        @param x The Char16 object to copy.
    */
    Char16& operator=(const Char16& x);

    /**
        Implicitly converts the Char16 value to a 16-bit integer.
    */
    operator Uint16() const;

private:
    Uint16 _code;
};

/**
    Compares two Char16 objects for equality.
    @param x The first Char16 to compare.
    @param y The second Char16 to compare.
    @return A Boolean indicating whether both operands represent the same
        character value.
*/
PEGASUS_COMMON_LINKAGE Boolean operator==(const Char16& x, const Char16& y);

/**
    Compares a Char16 object and an 8-bit character for equality.
    @param x The Char16 to compare.
    @param y The 8-bit character to compare.
    @return A Boolean indicating whether both operands represent the same
        character value.
*/
PEGASUS_COMMON_LINKAGE Boolean operator==(const Char16& x, char y);

/**
    Compares a Char16 object and an 8-bit character for equality.
    @param x The 8-bit character to compare.
    @param y The Char16 to compare.
    @return A Boolean indicating whether both operands represent the same
        character value.
*/
PEGASUS_COMMON_LINKAGE Boolean operator==(char x, const Char16& y);

/**
    Compares two Char16 objects for inequality.
    @param x The first Char16 to compare.
    @param y The second Char16 to compare.
    @return A Boolean indicating whether the operands represent different
        character values.
*/
PEGASUS_COMMON_LINKAGE Boolean operator!=(const Char16& x, const Char16& y);

/**
    Compares a Char16 object and an 8-bit character for inequality.
    @param x The Char16 to compare.
    @param y The 8-bit character to compare.
    @return A Boolean indicating whether the operands represent different
        character values.
*/
PEGASUS_COMMON_LINKAGE Boolean operator!=(const Char16& x, char y);

/**
    Compares a Char16 object and an 8-bit character for inequality.
    @param x The 8-bit character to compare.
    @param y The Char16 to compare.
    @return A Boolean indicating whether the operands represent different
        character values.
*/
PEGASUS_COMMON_LINKAGE Boolean operator!=(char x, const Char16& y);

PEGASUS_NAMESPACE_END

#if defined(PEGASUS_INTERNALONLY) && !defined(PEGASUS_DISABLE_INTERNAL_INLINES)
# include "Char16Inline.h"
#endif

#endif /* Pegasus_Char16_h */
