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
#include "HashTable.h"

PEGASUS_NAMESPACE_BEGIN

////////////////////////////////////////////////////////////////////////////////
//
// Hash routines
//
////////////////////////////////////////////////////////////////////////////////

Uint32 HashFunc<String>::hash(const String& str)
{
    // Caution: do not change this hash algorithm since hash codes are stored
    // in the repository. Any change here will break backwards compatibility
    // of the repository.

    Uint32 h = 0;
    const Uint16* p = (const Uint16*)str.getChar16Data();
    Uint32 n = str.size();

    while (n--)
        h = 5 * h + *p++;

    return h;
}

static const Uint8 _toLowerTable[256] =
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

Uint32 HashLowerCaseFunc::hash(const String& str)
{
    Uint16* p = (Uint16*)str.getChar16Data();
    Uint32 h = 0;
    Uint32 n = str.size();

    while (n >= 4)
    {
        h = ((h << 9) | (h >> 23)) ^ (Uint32)_toLowerTable[p[0] & 0x007F];
        h = ((h << 9) | (h >> 23)) ^ (Uint32)_toLowerTable[p[1] & 0x007F];
        h = ((h << 9) | (h >> 23)) ^ (Uint32)_toLowerTable[p[2] & 0x007F];
        h = ((h << 9) | (h >> 23)) ^ (Uint32)_toLowerTable[p[3] & 0x007F];
        n -= 4;
        p += 4;
    }

    while (*p)
        h = ((h << 9) | (h >> 23)) ^ (Uint32)_toLowerTable[*p++ & 0x007F];

    return h;
}

////////////////////////////////////////////////////////////////////////////////
//
// _HashTableRep::_BucketBase
//
////////////////////////////////////////////////////////////////////////////////

_BucketBase::~_BucketBase()
{

}

////////////////////////////////////////////////////////////////////////////////
//
// _HashTableIteratorBase
//
////////////////////////////////////////////////////////////////////////////////

void _HashTableIteratorBase::operator++()
{
    // At the end?

    if (!_bucket)
        return;

    // More buckets this chain?

    if ((_bucket = _bucket->next))
        return;

    // Find the next non-empty chain:

    _bucket = 0;

    while (_first != _last)
    {
        if (*_first)
        {
            _bucket = *_first++;
            break;
        }

        _first++;
    }

    return;
}

_HashTableIteratorBase::_HashTableIteratorBase(
    _BucketBase** first,
    _BucketBase** last) : _first(first), _last(last)
{
    _bucket = 0;

    while (_first != last)
    {
        if (*_first)
        {
            _bucket = *_first++;
            break;
        }

        _first++;
    }
}

////////////////////////////////////////////////////////////////////////////////
//
// _HashTableRep
//
////////////////////////////////////////////////////////////////////////////////

_HashTableRep::_HashTableRep(Uint32 numChains) : _size(0), _numChains(numChains)
{
    if (numChains < 8)
        numChains = 8;

    _chains = new _BucketBase*[_numChains];
    memset(_chains, 0, sizeof(_BucketBase*) * _numChains);
}

_HashTableRep::_HashTableRep(const _HashTableRep& x)
{
    _size = 0;
    _numChains = 0;
    _chains = 0;
    operator=(x);
}

_HashTableRep::~_HashTableRep()
{
    clear();
    delete [] _chains;
}

_HashTableRep& _HashTableRep::operator=(const _HashTableRep& x)
{
    if (this == &x)
        return *this;

    // Destroy the old representation:

    clear();

    delete [] _chains;

    // Create chain array:

    _chains = new _BucketBase*[_numChains = x._numChains];
    memset(_chains, 0, sizeof(_BucketBase*) * _numChains);
    _size = x._size;

    // Copy over the buckets:

    for (Uint32 i = 0; i < _numChains; i++)
    {
        if (x._chains[i])
        {
            _chains[i] = x._chains[i]->clone();

            _BucketBase* curr = _chains[i];
            _BucketBase* next = x._chains[i]->next;

            for (; next; next = next->next)
            {
                curr->next = next->clone();
                curr = curr->next;
            }
        }
    }

    return *this;
}

void _HashTableRep::clear()
{
    for (Uint32 i = 0; i < _numChains; i++)
    {
        for (_BucketBase* bucket = _chains[i]; bucket; )
        {
            _BucketBase* next = bucket->next;
            delete bucket;
            bucket = next;
        }
    }

    _size = 0;

    if (_numChains)
        memset(_chains, 0, sizeof(_BucketBase*) * _numChains);
}

Boolean _HashTableRep::insert(
    Uint32 hashCode,
    _BucketBase* bucket,
    const void* key)
{
    // Check for duplicate entry with same key:

    Uint32 i = hashCode % _numChains;
    _BucketBase* last = 0;

    for (_BucketBase* b = _chains[i]; b; b = b->next)
    {
        if (b->equal(key))
        {
            delete bucket;
            return false;
        }

        last = b;
    }

    // Insert bucket:

    bucket->next = 0;

    if (last)
        last->next = bucket;
    else
        _chains[i] = bucket;

    _size++;
    return true;
}

const _BucketBase* _HashTableRep::lookup(
    Uint32 hashCode,
    const void* key) const
{
#ifdef PEGASUS_OS_VMS

// This is to prevent a crash when the hash
//  code hasn't been initialized!

    if (_numChains == 0)
    {
      return 0;
    }
#endif
    Uint32 i = hashCode % _numChains;

    for (_BucketBase* bucket = _chains[i]; bucket; bucket = bucket->next)
    {
        if (bucket->equal(key))
            return bucket;
    }

    // Not found!
    return 0;
}

Boolean _HashTableRep::remove(Uint32 hashCode, const void* key)
{
    Uint32 i = hashCode % _numChains;

    _BucketBase* prev = 0;

    for (_BucketBase* bucket = _chains[i]; bucket; bucket = bucket->next)
    {
        if (bucket->equal(key))
        {
            if (prev)
                prev->next = bucket->next;
            else
                _chains[i] = bucket->next;

            delete bucket;
            _size--;
            return true;
        }
        prev = bucket;
    }

    return false;
}

PEGASUS_NAMESPACE_END
