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

#ifndef Pegasus_Buffer_h
#define Pegasus_Buffer_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/Linkage.h>
#include <cstring>

PEGASUS_NAMESPACE_BEGIN

struct BufferRep
{
    Uint32 size;
    Uint32 cap;
    char data[1];
};

class PEGASUS_COMMON_LINKAGE Buffer
{
public:

    Buffer(Uint32 minCap=2048);

    Buffer(const Buffer& x);

    Buffer(const char* data, Uint32 size, Uint32 minCap=2048);

    ~Buffer();

    Buffer& operator=(const Buffer& x);

    void swap(Buffer& x);

    Uint32 size() const;

    Uint32 capacity() const;

    /**
        Returns a pointer to a character buffer with the Buffer contents.
        The character buffer is null-terminated even if the Buffer contents
        do not include a null termination character.
     */
    const char* getData() const;

    /**
        Returns a pointer to the character buffer with the Buffer contents.
        This function does NOT append a null character.
        Resulting data should NOT be used with C string processing functions.
    */
    char* getContentPtr();

    char get(Uint32 i) const;

    void set(Uint32 i, char x);

    const char& operator[](Uint32 i) const;

    void reserveCapacity(Uint32 cap);

    void grow(Uint32 size, char x = '\0');

    void append(char x);

    void append_unchecked(char x);

    void append(const char* data, Uint32 size);

    void append_unchecked(const char* data, Uint32 size);

    void append(char c1, char c2);

    void append(char c1, char c2, char c3);

    void append(char c1, char c2, char c3, char c4);

    void append(
        char c1, char c2, char c3, char c4, char c5, char c6, char c7, char c8);

    void insert(Uint32 pos, const char* data, Uint32 size);

    /* inserts size bytes of data at position pos, but overwrites overlay
       bytes of the original Buffer data.
       This function is used in HTTPConnection.
       ATTENTION: Function does NOT check if enough capacity is available in
                 the Buffer and expects the caller to take care of that upfront.
    */
    void insertWithOverlay(
        Uint32 pos,
        const char* data,
        Uint32 size,
        Uint32 overlay);

    void remove(Uint32 pos, Uint32 size);

    void remove(Uint32 pos);

    void clear();

private:

    void _reserve_aux(Uint32 cap);

    void _append_char_aux();

    BufferRep* _rep;
    static BufferRep _empty_rep;
    Uint32 _minCap;
};

inline Buffer::Buffer(Uint32 minCap) : _rep(&_empty_rep), _minCap(minCap)
{
}

inline Buffer::~Buffer()
{
    if (_rep->cap != 0)
        free(_rep);
}

inline void Buffer::swap(Buffer& x)
{
    BufferRep* tmp = _rep;
    _rep = x._rep;
    x._rep = tmp;
}

inline Uint32 Buffer::size() const
{
    return _rep->size;
}

inline Uint32 Buffer::capacity() const
{
    return _rep->cap;
}

inline const char* Buffer::getData() const
{
    if (_rep->cap == 0)
    {
        const_cast<Buffer*>(this)->_append_char_aux();
    }

    _rep->data[_rep->size] = '\0';

    return _rep->data;
}

inline char* Buffer::getContentPtr()
{
    return _rep->data;
}

inline char Buffer::get(Uint32 i) const
{
    return _rep->data[i];
}

inline void Buffer::set(Uint32 i, char x)
{
    _rep->data[i] = x;
}

inline const char& Buffer::operator[](Uint32 i) const
{
    return _rep->data[i];
}

inline void Buffer::reserveCapacity(Uint32 cap)
{
    if (cap > _rep->cap)
        _reserve_aux(cap);
}

inline void Buffer::grow(Uint32 size, char x)
{
    Uint32 cap = _rep->size + size;

    if (cap > _rep->cap)
        _reserve_aux(cap);

    memset(_rep->data + _rep->size, x, size);
    _rep->size += size;
}

inline void Buffer::append(char x)
{
    if (_rep->size == _rep->cap)
        _append_char_aux();

    _rep->data[_rep->size++] = x;
}

inline void Buffer::append_unchecked(char x)
{
    _rep->data[_rep->size++] = x;
}

inline void Buffer::append(const char* data, Uint32 size)
{
    Uint32 cap = _rep->size + size;

    if (cap > _rep->cap)
        _reserve_aux(cap);

    memcpy(_rep->data + _rep->size, data, size);
    _rep->size += size;
}

inline void Buffer::append_unchecked(const char* data, Uint32 size)
{
    memcpy(_rep->data + _rep->size, data, size);
    _rep->size += size;
}

inline void Buffer::clear()
{
    if (_rep->cap != 0)
        _rep->size = 0;
}

inline void Buffer::remove(Uint32 pos)
{
    remove(pos, 1);
}

inline void Buffer::append(char c1, char c2)
{
    Uint32 cap = _rep->size + 2;

    if (cap > _rep->cap)
        _reserve_aux(cap);

    char* p = _rep->data + _rep->size;
    p[0] = c1;
    p[1] = c2;
    _rep->size += 2;
}

inline void Buffer::append(char c1, char c2, char c3)
{
    Uint32 cap = _rep->size + 3;

    if (cap > _rep->cap)
        _reserve_aux(cap);

    char* p = _rep->data + _rep->size;
    p[0] = c1;
    p[1] = c2;
    p[2] = c3;
    _rep->size += 3;
}

inline void Buffer::append(char c1, char c2, char c3, char c4)
{
    Uint32 cap = _rep->size + 4;

    if (cap > _rep->cap)
        _reserve_aux(cap);

    char* p = _rep->data + _rep->size;
    p[0] = c1;
    p[1] = c2;
    p[2] = c3;
    p[3] = c4;
    _rep->size += 4;
}

inline void Buffer::append(
    char c1, char c2, char c3, char c4, char c5, char c6, char c7, char c8)
{
    Uint32 cap = _rep->size + 8;

    if (cap > _rep->cap)
        _reserve_aux(cap);

    char* p = _rep->data + _rep->size;
    p[0] = c1;
    p[1] = c2;
    p[2] = c3;
    p[3] = c4;
    p[4] = c5;
    p[5] = c6;
    p[6] = c7;
    p[7] = c8;
    _rep->size += 8;
}

inline bool operator==(const Buffer& x, const Buffer& y)
{
    return memcmp(x.getData(), y.getData(), x.size()) == 0;
}

PEGASUS_NAMESPACE_END

#endif /* Pegasus_Buffer_h */
