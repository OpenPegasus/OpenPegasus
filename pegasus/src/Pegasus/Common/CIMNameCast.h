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

#ifndef Pegasus_CIMNameCast_h
#define Pegasus_CIMNameCast_h

#include <Pegasus/Common/CIMName.h>

PEGASUS_NAMESPACE_BEGIN

//
// This function performs a compile-time cast from String to CIMName. It should
// only be used where the String is already known to contain a valid CIM name,
// thereby avoiding the overhead of checking every character of the String.
// This cast is possible because CIMName has a single String member. Note that
// that sizeof(CIMName) == sizeof(String) and that the classes are identical
// in their representation, differing only by interface. When compiled for
// debug, this function checks that str refers to a valid CIM name.
//
inline const CIMName& CIMNameCast(const String& str)
{
#if defined(PEGASUS_DEBUG)

    if (str.size() && !CIMName::legal(str))
    {
        throw InvalidNameException(str);
    }

#endif

    return *(reinterpret_cast<const CIMName*>(&str));
}

inline const CIMNamespaceName& CIMNamespaceNameCast(const String& str)
{
#if defined(PEGASUS_DEBUG)

    if (str.size() && !CIMNamespaceName::legal(str))
    {
        throw InvalidNamespaceNameException(str);
    }

#endif

    return *(reinterpret_cast<const CIMNamespaceName*>(&str));
}


/** Checks whether a given character string consists of ASCII only and
    legal characters for a CIMName (i.e. letter, numbers and underscore)
    The first character has to be a letter or underscore
    @param str character string to be checked
    @return  0 in case non-legal ASCII character was found
            >0 length of the character string str
*/
PEGASUS_COMMON_LINKAGE Uint32 CIMNameLegalASCII(const char* str);

PEGASUS_NAMESPACE_END

#endif /* Pegasus_CIMNameCast_h */
