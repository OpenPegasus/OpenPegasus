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

#ifndef Pegasus_CharSet_h
#define Pegasus_CharSet_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/Linkage.h>

PEGASUS_NAMESPACE_BEGIN

/*  This class provides fast methods for checking whether a character is a
    member of a given character set (e.g., isAlpha()) and for translating
    characters (e.g., toUpper()). These functions are faster than their system
    library counterparts since they restrict their arguments to Uint8. For
    example, the system isupper() takes an int argument and must check to see
    if the int is between 0 and 128 before using it as an index into a table.
    Also, nearly all system implementation require an extra AND operation since
    a single table is used to implement all of the ctype functions (one bit
    per function).

    This class also implements additional functions not supported by the
    system library, like isAlphaUnder(), and isAlNumUnder(). Expression like
    this:

        if (isalpha(c) || c == '_')
            ...

    Become this:

        if (CharSet::isAlphaUnder(c))
            ...

    The optimization is even faster than it looks. Recall that isalpha() incurs
    an extra branch and an extra AND operation, as explained above.
*/
class CharSet
{
public:

    static Uint8 isAlphaUnder(Uint8 c) { return _isAlphaUnder[c]; }
    static Uint8 isAlNumUnder(Uint8 c) { return _isAlNumUnder[c]; }
    static Uint8 isSpace(Uint8 c) { return _isSpace[c]; }
    /**
        According to the XML specification, the space (0x20), carriage
        return (0x09), line feed (0x0D), and tab (0x0A) characters are
        considered white space.
     */
    static Uint8 isXmlWhiteSpace(Uint8 c) { return _isXmlWhiteSpace[c]; }
    static Uint8 toUpper(Uint8 c) { return _toUpper[c]; }
    static Uint8 toLower(Uint8 c) { return _toLower[c]; }
    static Uint8 isNotSpaceNorTerm(Uint8 c) { return _isNotSpaceNorTerm[c]; }

    static Uint16 toUpperHash(Uint8 c) { return _toUpperHash[c]; }

private:
    static const Uint8 _isAlphaUnder[256];
    static const Uint8 _isAlNumUnder[256];
    static const Uint8 _toUpper[256];
    static const Uint8 _toLower[256];
    static const Uint8 _isSpace[256];
    static const Uint8 _isXmlWhiteSpace[256];
    static const Uint8 _isNotSpaceNorTerm[256];
    static const Uint16 _toUpperHash[256];
};

PEGASUS_NAMESPACE_END

#endif /* Pegasus_CharSet_h */
