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

#ifndef _Pegasus_StringRep_h
#define _Pegasus_StringRep_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/AtomicInt.h>
#include <Pegasus/Common/CommonUTF.h>
#include <new>

PEGASUS_NAMESPACE_BEGIN

struct PEGASUS_COMMON_LINKAGE StringRep
{
    StringRep();

    ~StringRep();

    static StringRep* alloc(size_t cap);

    static void free(StringRep* rep);

    static StringRep* create(const Uint16* data, size_t size);

    static StringRep* create(const char* data, size_t size);

    static StringRep* copyOnWrite(StringRep* rep);

    static Uint32 length(const Uint16* str);

    static void ref(const StringRep* rep);

    static void unref(const StringRep* rep);

    static StringRep _emptyRep;

    // Number of characters in this string, excluding the null terminator.
    size_t size;

    // Number of characters this representation has room for. This is
    // greater or equal to size.
    size_t cap;

    // Number of string refering to this StringRep (1, 2, etc).
    AtomicInt refs;

    // The first character in the string. Extra space is allocated off the
    // end of this structure for additional characters.
    Uint16 data[1];
};

inline void StringRep::free(StringRep* rep)
{
    rep->refs.~AtomicInt();
    ::operator delete(rep);
}

inline StringRep::StringRep() : size(0), cap(0), refs(2)
{
    // Only called on _emptyRep. We set the reference count to two to
    // keep a String from modifying it (if the reference count were one,
    // a string would think it was the sole owner of the StringRep object).
    data[0] = 0;
}

inline StringRep::~StringRep()
{
    // Only called on _emptyRep.
}

inline void StringRep::ref(const StringRep* rep)
{
    if (rep != &StringRep::_emptyRep)
        ((StringRep*)rep)->refs++;
}

inline void StringRep::unref(const StringRep* rep)
{
    if (rep != &StringRep::_emptyRep &&
        ((StringRep*)rep)->refs.decAndTestIfZero())
        StringRep::free((StringRep*)rep);
}

PEGASUS_COMMON_LINKAGE void StringThrowOutOfBounds();

PEGASUS_COMMON_LINKAGE void StringAppendCharAux(StringRep*& _rep);

PEGASUS_COMMON_LINKAGE Boolean StringEqualNoCase(
    const String& s1, const String& s2);

PEGASUS_COMMON_LINKAGE Uint32 StringFindAux(
    const StringRep* _rep, const Char16* s, Uint32 n);

inline void _checkBounds(size_t index, size_t size)
{
    if (index > size)
    {
        StringThrowOutOfBounds();
    }
}

template<class P, class Q>
static void _copy(P* p, const Q* q, size_t n)
{
    // The following employs loop unrolling for efficiency. Please do not
    // eliminate.

    while (n >= 8)
    {
        p[0] = q[0];
        p[1] = q[1];
        p[2] = q[2];
        p[3] = q[3];
        p[4] = q[4];
        p[5] = q[5];
        p[6] = q[6];
        p[7] = q[7];
        p += 8;
        q += 8;
        n -= 8;
    }

    while (n >= 4)
    {
        p[0] = q[0];
        p[1] = q[1];
        p[2] = q[2];
        p[3] = q[3];
        p += 4;
        q += 4;
        n -= 4;
    }

    while (n--)
        *p++ = *q++;
}

static size_t _copyFromUTF8(
    Uint16* dest,
    const char* src,
    size_t n,
    size_t& utf8_error_index)
{
    Uint16* p = dest;
    const Uint8* q = (const Uint8*)src;

    // Process leading 7-bit ASCII characters (to avoid UTF8 overhead later).
    // Use loop-unrolling.

    while ( (n >=8) && ((q[0]|q[1]|q[2]|q[3]|q[4]|q[5]|q[6]|q[7]) & 0x80) == 0)
    {
        p[0] = q[0];
        p[1] = q[1];
        p[2] = q[2];
        p[3] = q[3];
        p[4] = q[4];
        p[5] = q[5];
        p[6] = q[6];
        p[7] = q[7];
        p += 8;
        q += 8;
        n -= 8;
    }

    while ((n >=4) && ((q[0]|q[1]|q[2]|q[3]) & 0x80) == 0)
    {
        p[0] = q[0];
        p[1] = q[1];
        p[2] = q[2];
        p[3] = q[3];
        p += 4;
        q += 4;
        n -= 4;
    }

    switch (n)
    {
        case 0:
            return p - dest;
        case 1:
            if (q[0] < 128)
            {
                p[0] = q[0];
                return p + 1 - dest;
            }
            break;
        case 2:
            if (((q[0]|q[1]) & 0x80) == 0)
            {
                p[0] = q[0];
                p[1] = q[1];
                return p + 2 - dest;
            }
            break;
        case 3:
            if (((q[0]|q[1]|q[2]) & 0x80) == 0)
            {
                p[0] = q[0];
                p[1] = q[1];
                p[2] = q[2];
                return p + 3 - dest;
            }
            break;
    }

    // Process remaining characters.

    while (n)
    {
        // Optimize for 7-bit ASCII case.

        if (*q < 128)
        {
            *p++ = *q++;
            n--;
        }
        else
        {
            Uint8 c = UTF_8_COUNT_TRAIL_BYTES(*q) + 1;

            if (c > n || !isValid_U8(q, c) ||
                UTF8toUTF16(&q, q + c, &p, p + n) != 0)
            {
                utf8_error_index = q - (const Uint8*)src;
                return size_t(-1);
            }

            n -= c;
        }
    }

    return p - dest;
}

static inline size_t _convert(
    Uint16* p, const char* q, size_t n, size_t& utf8_error_index)
{
#ifdef PEGASUS_STRING_NO_UTF8
    _copy(p, q, n);
    return n;
#else
    return _copyFromUTF8(p, q, n, utf8_error_index);
#endif
}

PEGASUS_NAMESPACE_END

#endif /* _Pegasus_StringRep_h */
