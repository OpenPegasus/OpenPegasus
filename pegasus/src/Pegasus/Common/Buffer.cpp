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

#include <cstring>
#include "Buffer.h"
#include "Pegasus/Common/InternalException.h"
#include "Pegasus/Common/Pegasus_inl.h"

PEGASUS_NAMESPACE_BEGIN

//
// Note: _empty_rep is the only BufferRep object that may have a zero capacity.
// So "_rep->cap == 0" implies "_rep == _empty_rep". But some platforms produce
// more than one instance of _empty_rep (strangely). Therefore, it is safer to
// use the former test rather than the latter.
//
BufferRep Buffer::_empty_rep =
{
    0, /* size */
    0, /* cap (zero implies it is the _empty_rep) */
    {0} /* data[0] */
};

static Uint32 _next_pow_2(Uint32 x, Uint32 minCap)
{
    // Check for potential overflow in x.
    PEGASUS_CHECK_CAPACITY_OVERFLOW(x);

    if (x < minCap)
        return minCap;

    x--;
    x |= (x >> 1);
    x |= (x >> 2);
    x |= (x >> 4);
    x |= (x >> 8);
    x |= (x >> 16);
    x++;

    return x;
}

static inline BufferRep* _allocate(Uint32 cap, Uint32 minCap)
{
    if (cap < minCap)
        cap = minCap;

    // Allocate an extra byte for null-termination performed by getData().
    BufferRep* rep = (BufferRep*)malloc(sizeof(BufferRep) + cap + 1);

    if (!rep)
    {
        throw PEGASUS_STD(bad_alloc)();
    }
    rep->cap = cap;
    return rep;
}

static inline BufferRep* _reallocate(BufferRep* rep, Uint32 cap)
{
    // Allocate an extra byte for null-termination performed by getData().
    rep = (BufferRep*)peg_inln_realloc(rep, sizeof(BufferRep) + cap + 1);

    rep->cap = cap;
    return rep;
}

Buffer::Buffer(const Buffer& x)
{
    _rep = _allocate(x._rep->cap, x._minCap);
    memcpy(_rep->data, x._rep->data, x._rep->size);
    _rep->size = x._rep->size;
    _minCap=x._minCap;
}

Buffer::Buffer(const char* data, Uint32 size, Uint32 minCap): _minCap(minCap)
{
    _rep = _allocate(size, _minCap);
    _rep->size = size;
    memcpy(_rep->data, data, size);
}

Buffer& Buffer::operator=(const Buffer& x)
{
    if (&x != this)
    {
        if (x._rep->size > _rep->cap)
        {
            if (_rep->cap != 0)
                free(_rep);

            _rep = _allocate(x._rep->cap, x._minCap);
        }

        memcpy(_rep->data, x._rep->data, x._rep->size);
        _rep->size = x._rep->size;
        _minCap = x._minCap;
    }
    return *this;
}

void Buffer::_reserve_aux(Uint32 cap)
{
    if (_rep->cap == 0)
    {
        _rep = _allocate(cap, _minCap);
        _rep->size = 0;
    }
    else
        _rep = _reallocate(_rep, _next_pow_2(cap, _minCap));
}

void Buffer::_append_char_aux()
{
    if (_rep->cap == 0)
    {
        _rep = _allocate(_minCap, _minCap);
        _rep->size = 0;
    }
    else
    {
        // Check for potential overflow.
        PEGASUS_CHECK_CAPACITY_OVERFLOW(_rep->cap);
        _rep = _reallocate(_rep, _rep->cap ? (2 * _rep->cap) : _minCap);
    }
}

void Buffer::insert(Uint32 pos, const char* data, Uint32 size)
{
    if (pos > _rep->size)
        return;

    Uint32 cap = _rep->size + size;
    Uint32 rem = _rep->size - pos;

    if (cap > _rep->cap)
    {
        BufferRep* rep = _allocate(cap, _minCap);
        rep->size = cap;

        memcpy(rep->data, _rep->data, pos);
        memcpy(rep->data + pos, data, size);
        memcpy(rep->data + pos + size, _rep->data + pos, rem);

        if (_rep->cap != 0)
            free(_rep);

        _rep = rep;
    }
    else
    {
        memmove(_rep->data + pos + size, _rep->data + pos, rem);
        memcpy(_rep->data + pos, data, size);
        _rep->size += size;
    }
}

void Buffer::insertWithOverlay(
    Uint32 pos,
    const char* data,
    Uint32 size,
    Uint32 overlay)
{
    if (pos > _rep->size)
        return;

    Uint32 rem = _rep->size - pos;

    memmove(_rep->data + pos + size - overlay, _rep->data + pos, rem);
    memcpy(_rep->data + pos, data, size);

    _rep->size += (size-overlay);
}

void Buffer::remove(Uint32 pos, Uint32 size)
{
    if (pos + size > _rep->size)
        return;

    Uint32 rem = _rep->size - (pos + size);

    if (rem)
        memmove(_rep->data + pos, _rep->data + pos + size, rem);

    _rep->size -= size;
}

PEGASUS_NAMESPACE_END
