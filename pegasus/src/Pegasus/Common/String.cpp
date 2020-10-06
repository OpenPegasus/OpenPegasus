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

#include <Pegasus/Common/PegasusAssert.h>
#include <cstring>
#include "InternalException.h"
#include "MessageLoader.h"
#include "StringRep.h"
#include <Pegasus/Common/Pegasus_inl.h>
#include <cstdarg>

#ifdef PEGASUS_HAS_ICU
# include <unicode/ures.h>
# include <unicode/ustring.h>
# include <unicode/uchar.h>
#endif

PEGASUS_NAMESPACE_BEGIN

//==============================================================================
//
// Compile-time macros (undefined by default).
//
//     PEGASUS_STRING_NO_UTF8 -- don't generate slower UTF8 code.
//
//==============================================================================

//==============================================================================
//
// File-scope definitions:
//
//==============================================================================

// Note: this table is much faster than the system toupper(). Please do not
// change.

const Uint8 _toUpperTable[256] =
{
    0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,
    0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F,
    0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,
    0x18,0x19,0x1A,0x1B,0x1C,0x1D,0x1E,0x1F,
    0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x27,
    0x28,0x29,0x2A,0x2B,0x2C,0x2D,0x2E,0x2F,
    0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,
    0x38,0x39,0x3A,0x3B,0x3C,0x3D,0x3E,0x3F,
    0x40,0x41,0x42,0x43,0x44,0x45,0x46,0x47,
    0x48,0x49,0x4A,0x4B,0x4C,0x4D,0x4E,0x4F,
    0x50,0x51,0x52,0x53,0x54,0x55,0x56,0x57,
    0x58,0x59,0x5A,0x5B,0x5C,0x5D,0x5E,0x5F,
    0x60,0x41,0x42,0x43,0x44,0x45,0x46,0x47,
    0x48,0x49,0x4A,0x4B,0x4C,0x4D,0x4E,0x4F,
    0x50,0x51,0x52,0x53,0x54,0x55,0x56,0x57,
    0x58,0x59,0x5A,0x7B,0x7C,0x7D,0x7E,0x7F,
    0x80,0x81,0x82,0x83,0x84,0x85,0x86,0x87,
    0x88,0x89,0x8A,0x8B,0x8C,0x8D,0x8E,0x8F,
    0x90,0x91,0x92,0x93,0x94,0x95,0x96,0x97,
    0x98,0x99,0x9A,0x9B,0x9C,0x9D,0x9E,0x9F,
    0xA0,0xA1,0xA2,0xA3,0xA4,0xA5,0xA6,0xA7,
    0xA8,0xA9,0xAA,0xAB,0xAC,0xAD,0xAE,0xAF,
    0xB0,0xB1,0xB2,0xB3,0xB4,0xB5,0xB6,0xB7,
    0xB8,0xB9,0xBA,0xBB,0xBC,0xBD,0xBE,0xBF,
    0xC0,0xC1,0xC2,0xC3,0xC4,0xC5,0xC6,0xC7,
    0xC8,0xC9,0xCA,0xCB,0xCC,0xCD,0xCE,0xCF,
    0xD0,0xD1,0xD2,0xD3,0xD4,0xD5,0xD6,0xD7,
    0xD8,0xD9,0xDA,0xDB,0xDC,0xDD,0xDE,0xDF,
    0xE0,0xE1,0xE2,0xE3,0xE4,0xE5,0xE6,0xE7,
    0xE8,0xE9,0xEA,0xEB,0xEC,0xED,0xEE,0xEF,
    0xF0,0xF1,0xF2,0xF3,0xF4,0xF5,0xF6,0xF7,
    0xF8,0xF9,0xFA,0xFB,0xFC,0xFD,0xFE,0xFF,
};

// Note: this table is much faster than the system tolower(). Please do not
// change.

const Uint8 _toLowerTable[256] =
{
    0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,
    0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F,
    0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,
    0x18,0x19,0x1A,0x1B,0x1C,0x1D,0x1E,0x1F,
    0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x27,
    0x28,0x29,0x2A,0x2B,0x2C,0x2D,0x2E,0x2F,
    0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,
    0x38,0x39,0x3A,0x3B,0x3C,0x3D,0x3E,0x3F,
    0x40,0x61,0x62,0x63,0x64,0x65,0x66,0x67,
    0x68,0x69,0x6A,0x6B,0x6C,0x6D,0x6E,0x6F,
    0x70,0x71,0x72,0x73,0x74,0x75,0x76,0x77,
    0x78,0x79,0x7A,0x5B,0x5C,0x5D,0x5E,0x5F,
    0x60,0x61,0x62,0x63,0x64,0x65,0x66,0x67,
    0x68,0x69,0x6A,0x6B,0x6C,0x6D,0x6E,0x6F,
    0x70,0x71,0x72,0x73,0x74,0x75,0x76,0x77,
    0x78,0x79,0x7A,0x7B,0x7C,0x7D,0x7E,0x7F,
    0x80,0x81,0x82,0x83,0x84,0x85,0x86,0x87,
    0x88,0x89,0x8A,0x8B,0x8C,0x8D,0x8E,0x8F,
    0x90,0x91,0x92,0x93,0x94,0x95,0x96,0x97,
    0x98,0x99,0x9A,0x9B,0x9C,0x9D,0x9E,0x9F,
    0xA0,0xA1,0xA2,0xA3,0xA4,0xA5,0xA6,0xA7,
    0xA8,0xA9,0xAA,0xAB,0xAC,0xAD,0xAE,0xAF,
    0xB0,0xB1,0xB2,0xB3,0xB4,0xB5,0xB6,0xB7,
    0xB8,0xB9,0xBA,0xBB,0xBC,0xBD,0xBE,0xBF,
    0xC0,0xC1,0xC2,0xC3,0xC4,0xC5,0xC6,0xC7,
    0xC8,0xC9,0xCA,0xCB,0xCC,0xCD,0xCE,0xCF,
    0xD0,0xD1,0xD2,0xD3,0xD4,0xD5,0xD6,0xD7,
    0xD8,0xD9,0xDA,0xDB,0xDC,0xDD,0xDE,0xDF,
    0xE0,0xE1,0xE2,0xE3,0xE4,0xE5,0xE6,0xE7,
    0xE8,0xE9,0xEA,0xEB,0xEC,0xED,0xEE,0xEF,
    0xF0,0xF1,0xF2,0xF3,0xF4,0xF5,0xF6,0xF7,
    0xF8,0xF9,0xFA,0xFB,0xFC,0xFD,0xFE,0xFF,
};

// Converts 16-bit characters to upper case. This routine is faster than the
// system toupper(). Please do not change.
inline Uint16 _toUpper(Uint16 x)
{
    return (x & 0xFF00) ? x : _toUpperTable[x];
}

// Converts 16-bit characters to lower case. This routine is faster than the
// system toupper(). Please do not change.
inline Uint16 _toLower(Uint16 x)
{
    return (x & 0xFF00) ? x : _toLowerTable[x];
}

// Rounds x up to the nearest power of two (or just returns 8 if x < 8).
static Uint32 _roundUpToPow2(Uint32 x)
{
    // Check for potential overflow in x
    PEGASUS_CHECK_CAPACITY_OVERFLOW(x);

    if (x < 8)
        return 8;

    x--;
    x |= (x >> 1);
    x |= (x >> 2);
    x |= (x >> 4);
    x |= (x >> 8);
    x |= (x >> 16);
    x++;

    return x;
}

static Uint16* _find(const Uint16* s, size_t n, Uint16 c)
{
    // The following employs loop unrolling for efficiency. Please do not
    // eliminate.

    while (n >= 4)
    {
        if (s[0] == c)
            return (Uint16*)s;
        if (s[1] == c)
            return (Uint16*)&s[1];
        if (s[2] == c)
            return (Uint16*)&s[2];
        if (s[3] == c)
            return (Uint16*)&s[3];

        n -= 4;
        s += 4;
    }

    if (n)
    {
        if (*s == c)
            return (Uint16*)s;
        s++;
        n--;
    }

    if (n)
    {
        if (*s == c)
            return (Uint16*)s;
        s++;
        n--;
    }

    if (n && *s == c)
        return (Uint16*)s;

    // Not found!
    return 0;
}

static int _compare(const Uint16* s1, const Uint16* s2)
{
    while (*s1 && *s2)
    {
        int r = *s1++ - *s2++;

        if (r)
            return r;
    }

    if (*s2)
        return -1;
    else if (*s1)
        return 1;

    return 0;
}

#ifdef PEGASUS_STRING_NO_UTF8
static int _compareNoUTF8(const Uint16* s1, const char* s2)
{
    Uint16 c1;
    Uint16 c2;

    do
    {
        c1 = *s1++;
        c2 = *s2++;

        if (c1 == 0)
            return c1 - c2;
    }
    while (c1 == c2);

    return c1 - c2;
}
#endif

static inline void _copy(Uint16* s1, const Uint16* s2, size_t n)
{
    memcpy(s1, s2, n * sizeof(Uint16));
}

void StringThrowOutOfBounds()
{
    throw IndexOutOfBoundsException();
}

inline void _checkNullPointer(const void* ptr)
{
    if (!ptr)
        throw NullPointer();
}

#define BADUTF8_MAX_CLEAR_CHAR 40
#define BADUTF8_MAX_CHAR_TO_HEX 10

static void _formatBadUTF8Chars(
    char* buffer,
    Uint32 index,
    const char* q,
    size_t n )
{

    char tmp[20];
    const char* start;

    size_t clearChar =
        (( index < BADUTF8_MAX_CLEAR_CHAR ) ? index : BADUTF8_MAX_CLEAR_CHAR );
    size_t charToHex =
        ((n-index-1) < BADUTF8_MAX_CHAR_TO_HEX ?
            (n-index-1) : BADUTF8_MAX_CHAR_TO_HEX );

    if (index < BADUTF8_MAX_CLEAR_CHAR)
    {
        start = q;
    } else
    {
        start = &(q[ index - BADUTF8_MAX_CLEAR_CHAR]);
    }

    // Intialize the buffer with the first character as '\0' to be able to use
    // strnchat() and strcat()
    buffer[0] = 0;
    // Start the buffer with the valid UTF8 chars
    strncat(buffer,start,clearChar);
    for (size_t i = clearChar, j = 0; j <= charToHex; i++,j++ )
    {
        tmp[0] = 0;
        sprintf(&(tmp[0])," 0x%02X",(Uint8)start[i]);
        strncat(buffer,&(tmp[0]),5);
    }

}

static void _StringThrowBadUTF8(Uint32 index, const char* q, size_t n)
{
    char buffer[1024];

    _formatBadUTF8Chars(&(buffer[0]),index,q,n);

    MessageLoaderParms parms(
        "Common.String.BAD_UTF8_LONG",
        "The byte sequence starting at index $0 "
        "is not valid UTF-8 encoding: $1",
        index,buffer);

    throw Exception(parms);
}

// Note: dest must be at least three times src (plus an extra byte for
// terminator).
static inline size_t _copyToUTF8(char* dest, const Uint16* src, size_t n)
{
    // The following employs loop unrolling for efficiency. Please do not
    // eliminate.

    const Uint16* q = src;
    Uint8* p = (Uint8*)dest;

    while (n >= 4 && q[0] < 128 && q[1] < 128 && q[2] < 128 && q[3] < 128)
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
            return p - (Uint8*)dest;
        case 1:
            if (q[0] < 128)
            {
                p[0] = q[0];
                return p + 1 - (Uint8*)dest;
            }
            break;
        case 2:
            if (q[0] < 128 && q[1] < 128)
            {
                p[0] = q[0];
                p[1] = q[1];
                return p + 2 - (Uint8*)dest;
            }
            break;
        case 3:
            if (q[0] < 128 && q[1] < 128 && q[2] < 128)
            {
                p[0] = q[0];
                p[1] = q[1];
                p[2] = q[2];
                return p + 3 - (Uint8*)dest;
            }
            break;
    }

    // If this line was reached, there must be characters greater than 128.

    UTF16toUTF8(&q, q + n, &p, p + 3 * n);

    return p - (Uint8*)dest;
}

//==============================================================================
//
// class CString
//
//==============================================================================

CString::CString(const CString& cstr) : _rep(0)
{
    if (cstr._rep)
    {
        size_t n = strlen(cstr._rep) + 1;
        _rep = (char*)operator new(n);
        memcpy(_rep, cstr._rep, n);
    }
}

CString& CString::operator=(const CString& cstr)
{
    if (&cstr != this)
    {
        if (_rep)
        {
            operator delete(_rep);
            _rep = 0;
        }

        if (cstr._rep)
        {
            size_t n = strlen(cstr._rep) + 1;
            _rep = (char*)operator new(n);
            memcpy(_rep, cstr._rep, n);
        }
    }

    return *this;
}

//==============================================================================
//
// class StringRep
//
//==============================================================================

StringRep StringRep::_emptyRep;

inline StringRep* StringRep::alloc(size_t cap)
{
    // Check for potential overflow in cap
    PEGASUS_CHECK_CAPACITY_OVERFLOW(cap);

    StringRep* rep = (StringRep*)::operator new(
        sizeof(StringRep) + cap * sizeof(Uint16));
    rep->cap = cap;
    new(&rep->refs) AtomicInt(1);

    return rep;
}

static inline void _reserve(StringRep*& rep, Uint32 cap)
{
    if (cap > rep->cap || rep->refs.get() != 1)
    {
        size_t n = _roundUpToPow2(cap);
        StringRep* newRep = StringRep::alloc(n);
        newRep->size = rep->size;
        _copy(newRep->data, rep->data, rep->size + 1);
        StringRep::unref(rep);
        rep = newRep;
    }
}

StringRep* StringRep::create(const Uint16* data, size_t size)
{
    StringRep* rep = StringRep::alloc(size);
    rep->size = size;
    _copy(rep->data, data, size);
    rep->data[size] = '\0';
    return rep;
}

StringRep* StringRep::copyOnWrite(StringRep* rep)
{
    // Return a new copy of rep. Release rep.

    StringRep* newRep = StringRep::alloc(rep->size);
    newRep->size = rep->size;
    _copy(newRep->data, rep->data, rep->size);
    newRep->data[newRep->size] = '\0';
    StringRep::unref(rep);
    return newRep;
}

StringRep* StringRep::create(const char* data, size_t size)
{
    StringRep* rep = StringRep::alloc(size);
    size_t utf8_error_index;
    rep->size = _convert((Uint16*)rep->data, data, size, utf8_error_index);

    if (rep->size == size_t(-1))
    {
        StringRep::free(rep);
        _StringThrowBadUTF8((Uint32)utf8_error_index, data,size);
    }

    rep->data[rep->size] = '\0';

    return rep;
}

Uint32 StringRep::length(const Uint16* str)
{
    // Note: We could unroll this but it is rarely called.

    const Uint16* end = (Uint16*)str;

    while (*end++)
        ;

    return (Uint32)(end - str - 1);
}

//==============================================================================
//
// class String
//
//==============================================================================

const String String::EMPTY;

String::String(const String& str, Uint32 n)
{
    _checkBounds(n, str._rep->size);
    _rep = StringRep::create(str._rep->data, n);
}

String::String(const Char16* str)
{
    _checkNullPointer(str);
    _rep = StringRep::create((Uint16*)str, StringRep::length((Uint16*)str));
}

String::String(const Char16* str, Uint32 n)
{
    _checkNullPointer(str);
    _rep = StringRep::create((Uint16*)str, n);
}

String::String(const char* str)
{
    _checkNullPointer(str);

    // Set this just in case create() throws an exception.
    _rep = &StringRep::_emptyRep;
    _rep = StringRep::create(str, strlen(str));
}

String::String(const char* str, Uint32 n)
{
    _checkNullPointer(str);

    // Set this just in case create() throws an exception.
    _rep = &StringRep::_emptyRep;
    _rep = StringRep::create(str, n);
}

String::String(const String& s1, const String& s2)
{
    size_t n1 = s1._rep->size;
    size_t n2 = s2._rep->size;
    size_t n = n1 + n2;
    _rep = StringRep::alloc(n);
    _copy(_rep->data, s1._rep->data, n1);
    _copy(_rep->data + n1, s2._rep->data, n2);
    _rep->size = n;
    _rep->data[n] = '\0';
}

String::String(const String& s1, const char* s2)
{
    _checkNullPointer(s2);
    size_t n1 = s1._rep->size;
    size_t n2 = strlen(s2);
    _rep = StringRep::alloc(n1 + n2);
    _copy(_rep->data, s1._rep->data, n1);
    size_t utf8_error_index;
    size_t tmp = _convert((Uint16*)_rep->data + n1, s2, n2, utf8_error_index);

    if (tmp == size_t(-1))
    {
        StringRep::free(_rep);
        _rep = &StringRep::_emptyRep;
        _StringThrowBadUTF8((Uint32)utf8_error_index,s2,n2);
    }

    _rep->size = n1 + tmp;
    _rep->data[_rep->size] = '\0';
}

String::String(const char* s1, const String& s2)
{
    _checkNullPointer(s1);
    size_t n1 = strlen(s1);
    size_t n2 = s2._rep->size;
    _rep = StringRep::alloc(n1 + n2);
    size_t utf8_error_index;
    size_t tmp = _convert((Uint16*)_rep->data, s1, n1, utf8_error_index);

    if (tmp ==  size_t(-1))
    {
        StringRep::free(_rep);
        _rep = &StringRep::_emptyRep;
        _StringThrowBadUTF8((Uint32)utf8_error_index,s1,n1);
    }

    _rep->size = n2 + tmp;
    _copy(_rep->data + n1, s2._rep->data, n2);
    _rep->data[_rep->size] = '\0';
}

String& String::assign(const String& str)
{
    if (_rep != str._rep)
    {
        StringRep::unref(_rep);
        StringRep::ref(_rep = str._rep);
    }

    return *this;
}

String& String::assign(const Char16* str, Uint32 n)
{
    _checkNullPointer(str);

    if (n > _rep->cap || _rep->refs.get() != 1)
    {
        StringRep::unref(_rep);
        _rep = StringRep::alloc(n);
    }

    _rep->size = n;
    _copy(_rep->data, (Uint16*)str, n);
    _rep->data[n] = '\0';

    return *this;
}

String& String::assign(const char* str, Uint32 n)
{
    _checkNullPointer(str);

    if (n > _rep->cap || _rep->refs.get() != 1)
    {
        StringRep::unref(_rep);
        _rep = StringRep::alloc(n);
    }

    size_t utf8_error_index;
    _rep->size = _convert(_rep->data, str, n, utf8_error_index);

    if (_rep->size ==  size_t(-1))
    {
        StringRep::free(_rep);
        _rep = &StringRep::_emptyRep;
        _StringThrowBadUTF8((Uint32)utf8_error_index,str,n);
    }

    _rep->data[_rep->size] = 0;

    return *this;
}

void String::clear()
{
    if (_rep->size)
    {
        if (_rep->refs.get() == 1)
        {
            _rep->size = 0;
            _rep->data[0] = '\0';
        }
        else
        {
            StringRep::unref(_rep);
            _rep = &StringRep::_emptyRep;
        }
    }
}

void String::reserveCapacity(Uint32 cap)
{
    _reserve(_rep, cap);
}

CString String::getCString() const
{
    // A UTF8 string can have three times as many characters as its UTF16
    // counterpart, so we allocate extra memory for the worst case. In the
    // best case, we may need only one third of the memory allocated. But
    // downsizing the string afterwards is expensive and unecessary since
    // CString objects are usually short-lived (disappearing after only a few
    // instructions). CString objects are typically created on the stack as
    // means to obtain a char* pointer.

#ifdef PEGASUS_STRING_NO_UTF8
    char* str = (char*)operator new(_rep->size + 1);
    _copy(str, _rep->data, _rep->size);
    str[_rep->size] = '\0';
    return CString(str);
#else
    Uint32 n = (Uint32)(3 * _rep->size);
    char* str = (char*)operator new(n + 1);
    size_t size = _copyToUTF8(str, _rep->data, _rep->size);
    str[size] = '\0';
    return CString(str);
#endif
}

String& String::append(const Char16* str, Uint32 n)
{
    _checkNullPointer(str);

    size_t oldSize = _rep->size;
    size_t newSize = oldSize + n;
    _reserve(_rep, (Uint32)newSize);
    _copy(_rep->data + oldSize, (Uint16*)str, n);
    _rep->size = newSize;
    _rep->data[newSize] = '\0';

    return *this;
}

String& String::append(const String& str)
{
    return append((Char16*)(&(str._rep->data[0])), (Uint32)str._rep->size);
}

String& String::append(const char* str, Uint32 size)
{
    _checkNullPointer(str);

    size_t oldSize = _rep->size;
    size_t cap = oldSize + size;

    _reserve(_rep, (Uint32)cap);
    size_t utf8_error_index;
    size_t tmp = _convert(
        (Uint16*)_rep->data + oldSize, str, size, utf8_error_index);

    if (tmp ==  size_t(-1))
    {
        StringRep::free(_rep);
        _rep = &StringRep::_emptyRep;
        _StringThrowBadUTF8((Uint32)utf8_error_index,str,size);
    }

    _rep->size += tmp;
    _rep->data[_rep->size] = '\0';

    return *this;
}

void String::remove(Uint32 index, Uint32 n)
{
    if (n == PEG_NOT_FOUND)
        n = (Uint32)(_rep->size - index);

    _checkBounds(index + n, _rep->size);

    if (_rep->refs.get() != 1)
        _rep = StringRep::copyOnWrite(_rep);

    PEGASUS_ASSERT(index + n <= _rep->size);

    size_t rem = _rep->size - (index + n);
    Uint16* data = _rep->data;

    if (rem)
        memmove(data + index, data + index + n, rem * sizeof(Uint16));

    _rep->size -= n;
    data[_rep->size] = '\0';
}

String String::subString(Uint32 index, Uint32 n) const
{
    // Note: this implementation is very permissive but used for
    // backwards compatibility.

    if (index < _rep->size)
    {
        if (n == PEG_NOT_FOUND || n > _rep->size - index)
            n = (Uint32)(_rep->size - index);

        return String((Char16*)(_rep->data + index), n);
    }

    return String();
}

Uint32 String::find(Char16 c) const
{
    Uint16* p = (Uint16*)_find(_rep->data, _rep->size, c);

    if (p)
        return static_cast<Uint32>(p - _rep->data);

    return PEG_NOT_FOUND;
}

Uint32 String::find(Uint32 index, Char16 c) const
{
    _checkBounds(index, _rep->size);

    if (index >= _rep->size)
        return PEG_NOT_FOUND;

    Uint16* p = (Uint16*)_find(_rep->data + index, _rep->size - index, c);

    if (p)
        return static_cast<Uint32>(p - _rep->data);

    return PEG_NOT_FOUND;
}

Uint32 StringFindAux(
    const StringRep* _rep, const Char16* s, Uint32 n)
{
    _checkNullPointer(s);

    const Uint16* data = _rep->data;
    size_t rem = _rep->size;

    while (n <= rem)
    {
        Uint16* p = (Uint16*)_find(data, rem, s[0]);

        if (!p)
            break;

        if (memcmp(p, s, n * sizeof(Uint16)) == 0)
            return static_cast<Uint32>(p - _rep->data);

        p++;
        rem -= p - data;
        data = p;
    }

    return PEG_NOT_FOUND;
}

Uint32 String::find(const char* s) const
{
    _checkNullPointer(s);

    // Note: could optimize away creation of temporary, but this is rarely
    // called.
    return find(String(s));
}

Uint32 String::reverseFind(Char16 c) const
{
    Uint16 x = c;
    Uint16* p = _rep->data;
    Uint16* q = _rep->data + _rep->size;

    while (q != p)
    {
        if (*--q == x)
            return static_cast<Uint32>(q - p);
    }

    return PEG_NOT_FOUND;
}

void String::toLower()
{
#ifdef PEGASUS_HAS_ICU

    if (InitializeICU::initICUSuccessful())
    {
        if (_rep->refs.get() != 1)
            _rep = StringRep::copyOnWrite(_rep);

        // This will do a locale-insensitive, but context-sensitive convert.
        // Since context-sensitive casing looks at adjacent chars, this
        // prevents optimizations where the us-ascii is converted before
        // calling ICU.
        // The string may shrink or expand after the convert.

        //// First calculate size of resulting string. u_strToLower() returns
        //// only the size when zero is passed as the destination size argument.

        UErrorCode err = U_ZERO_ERROR;

        int32_t newSize = u_strToLower(
            NULL, 0, (UChar*)_rep->data, _rep->size, NULL, &err);

        err = U_ZERO_ERROR;

        //// Reserve enough space for the result.

        if ((Uint32)newSize > _rep->cap)
            _reserve(_rep, newSize);

        //// Perform the conversion (overlapping buffers are allowed).

        u_strToLower((UChar*)_rep->data, newSize,
            (UChar*)_rep->data, _rep->size, NULL, &err);

        _rep->size = newSize;
        return;
    }

#endif /* PEGASUS_HAS_ICU */

    if (_rep->refs.get() != 1)
        _rep = StringRep::copyOnWrite(_rep);

    Uint16* p = _rep->data;
    size_t n = _rep->size;

    for (; n--; p++)
    {
        if (!(*p & 0xFF00))
            *p = _toLower(*p);
    }
}

void String::toUpper()
{
#ifdef PEGASUS_HAS_ICU

    if (InitializeICU::initICUSuccessful())
    {
        if (_rep->refs.get() != 1)
            _rep = StringRep::copyOnWrite(_rep);

        // This will do a locale-insensitive, but context-sensitive convert.
        // Since context-sensitive casing looks at adjacent chars, this
        // prevents optimizations where the us-ascii is converted before
        // calling ICU.
        // The string may shrink or expand after the convert.

        //// First calculate size of resulting string. u_strToUpper() returns
        //// only the size when zero is passed as the destination size argument.

        UErrorCode err = U_ZERO_ERROR;

        int32_t newSize = u_strToUpper(
            NULL, 0, (UChar*)_rep->data, _rep->size, NULL, &err);

        err = U_ZERO_ERROR;

        //// Reserve enough space for the result.

        if ((Uint32)newSize > _rep->cap)
            _reserve(_rep, newSize);

        //// Perform the conversion (overlapping buffers are allowed).

        u_strToUpper((UChar*)_rep->data, newSize,
            (UChar*)_rep->data, _rep->size, NULL, &err);

        _rep->size = newSize;

        return;
    }

#endif /* PEGASUS_HAS_ICU */

    if (_rep->refs.get() != 1)
        _rep = StringRep::copyOnWrite(_rep);

    Uint16* p = _rep->data;
    size_t n = _rep->size;

    for (; n--; p++)
        *p = _toUpper(*p);
}

int String::compare(const String& s1, const String& s2, Uint32 n)
{
    const Uint16* p1 = s1._rep->data;
    const Uint16* p2 = s2._rep->data;

    while (n--)
    {
        int r = *p1++ - *p2++;
        if (r)
        {
            return r;
        }
        else if (!p1[-1])
        {
            // We must have encountered a null terminator in both s1 and s2
            return 0;
        }
    }
    return 0;
}

int String::compare(const String& s1, const String& s2)
{
    return _compare(s1._rep->data, s2._rep->data);
}

int String::compare(const String& s1, const char* s2)
{
    _checkNullPointer(s2);

#ifdef PEGASUS_STRING_NO_UTF8
    return _compareNoUTF8(s1._rep->data, s2);
#else
    // ATTN: optimize this!
    return String::compare(s1, String(s2));
#endif
}

int String::compareNoCase(const String& str1, const String& str2)
{
#ifdef PEGASUS_HAS_ICU

    if (InitializeICU::initICUSuccessful())
    {
        return  u_strcasecmp(
            (const UChar*)str1._rep->data,
            (const UChar*)str2._rep->data,
            U_FOLD_CASE_DEFAULT
            );
    }

#endif /* PEGASUS_HAS_ICU */

    const Uint16* s1 = str1._rep->data;
    const Uint16* s2 = str2._rep->data;

    while (*s1 && *s2)
    {
        int r = _toLower(*s1++) - _toLower(*s2++);

        if (r)
            return r;
    }

    if (*s2)
        return -1;
    else if (*s1)
        return 1;

    return 0;
}

Boolean StringEqualNoCase(const String& s1, const String& s2)
{
#ifdef PEGASUS_HAS_ICU

    return String::compareNoCase(s1, s2) == 0;

#else /* PEGASUS_HAS_ICU */

    // The following employs loop unrolling for efficiency. Please do not
    // eliminate.

    Uint16* p = (Uint16*)s1.getChar16Data();
    Uint16* q = (Uint16*)s2.getChar16Data();
    Uint32 n = s2.size();

    while (n >= 8)
    {
        if (((p[0] - q[0]) && (_toUpper(p[0]) - _toUpper(q[0]))) ||
            ((p[1] - q[1]) && (_toUpper(p[1]) - _toUpper(q[1]))) ||
            ((p[2] - q[2]) && (_toUpper(p[2]) - _toUpper(q[2]))) ||
            ((p[3] - q[3]) && (_toUpper(p[3]) - _toUpper(q[3]))) ||
            ((p[4] - q[4]) && (_toUpper(p[4]) - _toUpper(q[4]))) ||
            ((p[5] - q[5]) && (_toUpper(p[5]) - _toUpper(q[5]))) ||
            ((p[6] - q[6]) && (_toUpper(p[6]) - _toUpper(q[6]))) ||
            ((p[7] - q[7]) && (_toUpper(p[7]) - _toUpper(q[7]))))
        {
            return false;
        }

        n -= 8;
        p += 8;
        q += 8;
    }

    while (n >= 4)
    {
        if (((p[0] - q[0]) && (_toUpper(p[0]) - _toUpper(q[0]))) ||
            ((p[1] - q[1]) && (_toUpper(p[1]) - _toUpper(q[1]))) ||
            ((p[2] - q[2]) && (_toUpper(p[2]) - _toUpper(q[2]))) ||
            ((p[3] - q[3]) && (_toUpper(p[3]) - _toUpper(q[3]))))
        {
            return false;
        }

        n -= 4;
        p += 4;
        q += 4;
    }

    while (n--)
    {
        if (((p[0] - q[0]) && (_toUpper(p[0]) - _toUpper(q[0]))))
            return false;

        p++;
        q++;
    }

    return true;

#endif /* PEGASUS_HAS_ICU */
}

Boolean String::equalNoCase(const String& s1, const char* s2)
{
    _checkNullPointer(s2);

#if defined(PEGASUS_HAS_ICU)

    return String::equalNoCase(s1, String(s2));

#elif defined(PEGASUS_STRING_NO_UTF8)

    const Uint16* p1 = (Uint16*)s1._rep->data;
    const char* p2 = s2;
    size_t n = s1._rep->size;

    while (n--)
    {
        if (!*p2)
            return false;

        if (_toUpper(*p1++) != _toUpperTable[int(*p2++)])
            return false;
    }

    if (*p2)
        return false;

    return true;

#else /* PEGASUS_HAS_ICU */

    // ATTN: optimize this!
    return String::equalNoCase(s1, String(s2));

#endif /* PEGASUS_HAS_ICU */
}

Boolean String::equal(const String& s1, const String& s2)
{
    return (s1._rep == s2._rep) ||
        ((s1._rep->size == s2._rep->size) &&
         memcmp(s1._rep->data,
                s2._rep->data,
                s1._rep->size * sizeof(Uint16)) == 0);
}

void String::appendPrintf(const char* format, ...)
{
    va_list ap;
    va_start(ap, format);

    // Format into allocated memory
    ////char* rtnCharPtr = _charVPrintf(format, ap);

    // Iniitial allocation size.  This is a guess assuming that
    // most printfs are one or two lines long
    int allocSize = 256;
    int rtnSize;
    char *p;

    // initial allocate for output
    if ((p = (char*)malloc(allocSize)) == NULL)
    {
        return;
    }

    // repeat formatting  with increased realloc until it works.
    do
    {
        rtnSize = vsnprintf(p, allocSize, format, ap);

        // return if successful; i.e. if not negative and
        // returns less than allocated size.
        if (rtnSize > -1 && rtnSize < allocSize)
        {
            break;
        }

        // increment alloc size. Positive return is
        // expected size and negative is error.
        allocSize = (rtnSize > -1)? (rtnSize + 1) : allocSize * 2;

    } while((p = (char*)peg_inln_realloc(p, allocSize)) != NULL);

    // get here only with error in malloc.

    va_end(ap);

    // Free allocated memory append printf output to current string
    append(p, rtnSize);
    free(p);
}

Boolean String::equal(const String& s1, const char* s2)
{
#ifdef PEGASUS_STRING_NO_UTF8

    _checkNullPointer(s2);

    const Uint16* p = (Uint16*)s1._rep->data;
    const char* q = s2;

    while (*p && *q)
    {
        if (*p++ != Uint16(*q++))
            return false;
    }

    return !(*p || *q);

#else /* PEGASUS_STRING_NO_UTF8 */

    return String::equal(s1, String(s2));

#endif /* PEGASUS_STRING_NO_UTF8 */
}

PEGASUS_STD(ostream)& operator<<(PEGASUS_STD(ostream)& os, const String& str)
{
#if defined(PEGASUS_HAS_ICU)

    if (InitializeICU::initICUSuccessful())
    {
        char *buf = NULL;
        const int size = str.size() * 6;
        UnicodeString UniStr(
            (const UChar *)str.getChar16Data(), (int32_t)str.size());
        Uint32 bufsize = UniStr.extract(0,size,buf);
        buf = new char[bufsize+1];
        UniStr.extract(0,bufsize,buf);
        os << buf;
        os.flush();
        delete [] buf;
        return os;
    }

#endif  // PEGASUS_HAS_ICU

    for (Uint32 i = 0, n = str.size(); i < n; i++)
    {
        Uint16 code = str[i];

        if (code > 0 && !(code & 0xFF00))
                os << char(code);
        else
            {
            // Print in hex format:
            char buffer[8];
            sprintf(buffer, "\\x%04X", code);
            os << buffer;
        }
    }

    return os;
}

void StringAppendCharAux(StringRep*& _rep)
{
    StringRep* tmp;

    if (_rep->cap)
    {
        tmp = StringRep::alloc(2 * _rep->cap);
        tmp->size = _rep->size;
        _copy(tmp->data, _rep->data, _rep->size);
    }
    else
    {
        tmp = StringRep::alloc(8);
        tmp->size = 0;
    }

    StringRep::unref(_rep);
    _rep = tmp;
}

void AssignASCII(String& s, const char* str, Uint32 n)
{
    class StringLayout
    {
    public:
        StringRep* rep;
    };

    StringLayout* that = reinterpret_cast<StringLayout*>(&s);

    _checkNullPointer(str);

    if (n > that->rep->cap || that->rep->refs.get() != 1)
    {
        StringRep::unref(that->rep);
        that->rep = StringRep::alloc(n);
    }

    _copy(that->rep->data, str, n);
    that->rep->size = n;
    that->rep->data[that->rep->size] = 0;
}

PEGASUS_NAMESPACE_END

/*
================================================================================

String optimizations:

    1.  Added mechanism allowing certain functions to be inlined only when
        used by internal Pegasus modules. External modules (i.e., providers)
        link to a non-inline version, which allows for binary compatibility.

    2.  Implemented copy-on-write with atomic increment/decrement. This
        yieled a 10% improvement for the 'gc' benchmark and a 11% improvment
        for the 'ni1000' benchmark.

    3.  Employed loop unrolling in several places. For example, see:

            static Uint16* _find(const Uint16* s, size_t n, Uint16 c);

    4.  Used the "empty-rep" optimization (described in whitepaper from the
        GCC Developers Summit). This reduced default construction to a simple
        pointer assignment.

            inline String::String() : _rep(&_emptyRep) { }

    5.  Implemented Uint16 versions of toupper() and tolower() using tables.
        For example:

            static const char _upper[] =
            {
                0,1,2,...255
            };

            inline Uint16 _toUpper(Uint16 x)
            {
                return (x & 0xFF00) ? x : _upper[x];
            }

        This outperforms the system implementation by avoiding an anding
        operation.

    6.  Implemented char* version of the following member functions to
        eliminate unecessary creation of anonymous string objects
        (temporaries).

            String(const String& s1, const char* s2);
            String(const char* s1, const String& s2);
            String& String::operator=(const char* str);
            Uint32 String::find(const char* s) const;
            bool String::equal(const String& s1, const char* s2);
            static int String::compare(const String& s1, const char* s2);
            String& String::append(const char* str);
            String& String::append(const char* str, Uint32 size);
            static bool String::equalNoCase(const String& s1, const char* s2);
            String& operator=(const char* str)
            String& String::assign(const char* str)
            String& String::append(const char* str)
            Boolean operator==(const String& s1, const char* s2)
            Boolean operator==(const char* s1, const String& s2)
            Boolean operator!=(const String& s1, const char* s2)
            Boolean operator!=(const char* s1, const String& s2)
            Boolean operator<(const String& s1, const char* s2)
            Boolean operator<(const char* s1, const String& s2)
            Boolean operator>(const String& s1, const char* s2)
            Boolean operator>(const char* s1, const String& s2)
            Boolean operator<=(const String& s1, const char* s2)
            Boolean operator<=(const char* s1, const String& s2)
            Boolean operator>=(const String& s1, const char* s2)
            Boolean operator>=(const char* s1, const String& s2)
            String operator+(const String& s1, const char* s2)
            String operator+(const char* s1, const String& s2)

    7.  Optimized _roundUpToPow2(), used in rounding the capacity to the next
        power of two (algorithm from the book "Hacker's Delight").

            static Uint32 _roundUpToPow2(Uint32 x)
            {
                if (x < 8)
                    return 8;

                x--;
                x |= (x >> 1);
                x |= (x >> 2);
                x |= (x >> 4);
                x |= (x >> 8);
                x |= (x >> 16);
                x++;

                return x;
            }

    8.  Implemented "concatenating constructors" to eliminate temporaries
        created by operator+(). This scheme employs the "return-value
        optimization" described by Stan Lippman.

            inline String operator+(const String& s1, const String& s2)
            {
                return String(s1, s2, 0);
            }

    9.  Experimented to find the optimial initial size for a short string.
        Eight seems to offer the best tradeoff between space and time.

    10. Inlined all members of the Char16 class.

    11. Used Uint16 internally in the String class. This showed no improvememnt
        since Char16 was already fully inlined and was essentially reduced to
        Uint16 in any case.

    12. Implemented conditional logic (#if) allowing error checking logic to
        be excluded to better performance. Examples include bounds checking
        and null-pointer checking.

    13. Used memcpy() and memcmp() where possible. These are implemented using
        the rep family of intructions under Intel and are much faster.

    14. Used loop unrolling, jump-tables, and short-circuiting to reduce UTF8
        copy routine overhead.

    15. Added ASCII7 form of the constructor and assign().

            String s("hello world", String::ASCII7);

            s.assignASCII7("hello world");

        This avoids slower UTF8 processing when not needed.

================================================================================
*/
