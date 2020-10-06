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

#ifndef Pegasus_StrLit_h
#define Pegasus_StrLit_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/Buffer.h>

#define STRLIT_ARGS(STR) STR, (sizeof(STR)-1)
#define STRLIT(STR) StrLit(STRLIT_ARGS(STR))

PEGASUS_NAMESPACE_BEGIN

/*  The StrLit class and associated macros provide a mechanism for retaining the
    length C string literals at compile time. This is preferrable to repeated
    recalculation of the length, usually with strlen(). For example, this:

        String s("hello world");

    Is less efficient than this:

        String s("hello world", 11);

    The first form, forces the String constructor to call strlen(), an O(N)
    operation. This is unfortunate for C string literals since the length can
    be obtained at compile time with the sizeof operator. For example:

        String s("hello world", sizeof("hello world") - 1);

    But repeating the literal twice is error prone, so instead we use the
    STRLIT_ARGS() macro.

        String s(STRLIT_ARGS("hello world"));

    This macro can also be used to define StrLit objects at compile time.
    For example:

        const StrLit DEFAULT_HOSTNAME(STRLIT_ARGS("localhost"));

    You can implement functions that take StrLit objects. For example, we
    define this function:

        operator<<(const Buffer&, const StrLit&);

    This function can be used in two ways. You can pass predefined StrLit
    object to it like this:

        Buffer out;
        out << DEFAULT_HOSTNAME;

    Or you can use the STRLIT() macro to construct on on the fly:

        Buffer out;
        out << STRLIT("localhost");

    Note that the latter form would be faster than this, since somebody
    is going to have to call strlen() eventually.

        Buffer out;
        out << "localhost";

    At first glance, this may seem like a small optimization, but this
    technique alone was used to decrease the Pegasus CIM server latency
    by ten percent (with only moderate application to the XML marshalling
    routines).
*/
struct StrLit
{
    StrLit(const char* s, size_t n) : str(s), size((Uint32)n) { }
    const char* str;
    const Uint32 size;
};

inline Buffer& operator<<(Buffer& out, const StrLit& x)
{
    out.append(x.str, x.size);
    return out;
}

PEGASUS_NAMESPACE_END

#endif /* Pegasus_StrLit_h */
