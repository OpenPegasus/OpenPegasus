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

#include <string.h>
#include "Packer.h"

PEGASUS_NAMESPACE_BEGIN

static bool _constains16BitChars(const String& x)
{
    const Char16* p = x.getChar16Data();
    Uint32 n = x.size();

    while (n--)
    {
        if (Uint16(*p++) > 0xFF)
            return true;
    }

    return false;
}

void Packer::packString(Buffer& out, const String& x)
{
    Uint32 n = x.size();

    if (_constains16BitChars(x))
    {
        packUint8(out, 16);
        packSize(out, n);
        packChar16(out, x.getChar16Data(), n);
    }
    else
    {
        packUint8(out, 8);
        packSize(out, n);
        const Char16* data = x.getChar16Data();

        for (Uint32 i = 0; i < n; i++)
            Packer::packUint8(out, (Uint8)data[i]);
    }
}

void Packer::packSize(Buffer& out, Uint32 x)
{
    // Top two bits indicate the number of bytes used to
    // pack this size:
    //
    //     00 : 1 byte
    //     01 : 2 bytes
    //     10 : 4 bytes
    //     11 : illegal

    if (x > 16383)
    {
        // Use four bytes for size (tag == '1')
        packUint32(out, 0x80000000 | x);
    }
    else if (x > 63)
    {
        // Use two bytes for size.
        packUint16(out, 0x4000 | Uint16(x));
    }
    else /* x > 1073741823 */
    {
        // Use one byte for size.
        packUint8(out, 0x00 | Uint8(x));
    }
}

void Packer::unpackSize(const Buffer& in, Uint32& pos, Uint32& x)
{
    // Top two bits form a tag that indicates the number of bytes used to
    // pack this size:
    //
    //     00 : 1 byte
    //     01 : 2 bytes
    //     10 : 4 bytes
    //     11 : illegal

    // Unpack first byte.

    Uint8 byte = Uint8(in[pos++]);
    Uint8 tag = byte & 0xC0;

    if (!tag)
    {
        // One-byte size:
        x = byte;
    }
    else if (tag == 0x80)
    {
        // Four-byte size:
        Uint8 b0 = tag ^ byte;
        Uint8 b1;
        Uint8 b2;
        Uint8 b3;

        Packer::unpackUint8(in, pos, b1);
        Packer::unpackUint8(in, pos, b2);
        Packer::unpackUint8(in, pos, b3);
        Uint32 tmp = (Uint32(b0) << 24) |
            (Uint32(b1) << 16) |
            (Uint32(b2) <<  8) |
            (Uint32(b3));

        x = tmp;
    }
    else if (tag == 0x40)
    {
        // Two-byte size:
        x = (tag ^ byte) << 8;
        Packer::unpackUint8(in, pos, byte);
        x |= byte;
    }
    else
    {
        PEGASUS_DEBUG_ASSERT(0);
    }

    PEGASUS_DEBUG_ASSERT(pos <= in.size());
}

template<class T>
void _pack_array(Buffer& out, const T* x, Uint32 n)
{
    Uint32 bytes = n * sizeof(T);
    out.reserveCapacity(out.size() + bytes);

    if (Packer::isLittleEndian())
    {
        for (size_t i = 0; i < n; i++)
        {
            T tmp = Packer::swap(x[i]);
            out.append((char*)&tmp, sizeof(tmp));
        }
    }
    else
        out.append((char*)x, bytes);
}

void Packer::packBoolean(Buffer& out, const Boolean* x, Uint32 n)
{
    out.reserveCapacity(out.size() + n);

    for (size_t i = 0; i < n; i++)
    {
        Uint8 tmp = Uint8(x[i]);
        out.append((char*)&tmp, sizeof(tmp));
    }
}

void Packer::packUint8(Buffer& out, const Uint8* x, Uint32 n)
{
    out.append((char*)x, n);
}

void Packer::packUint16(Buffer& out, const Uint16* x, Uint32 n)
{
    _pack_array(out, x, n);
}

void Packer::packUint32(Buffer& out, const Uint32* x, Uint32 n)
{
    _pack_array(out, x, n);
}

void Packer::packUint64(Buffer& out, const Uint64* x, Uint32 n)
{
    _pack_array(out, x, n);
}

void Packer::packString(Buffer& out, const String* x, Uint32 n)
{
    for (size_t i = 0; i < n; i++)
        packString(out, x[i]);
}

void Packer::unpackUint16(
    const Buffer& in, Uint32& pos, Uint16& x)
{
    memcpy(&x, &in[pos], sizeof(x));
    pos += sizeof(x);

    if (isLittleEndian())
        x = Packer::swapUint16(x);
}

void Packer::unpackUint32(
    const Buffer& in, Uint32& pos, Uint32& x)
{
    memcpy(&x, &in[pos], sizeof(x));
    pos += sizeof(x);

    if (isLittleEndian())
        x = Packer::swapUint32(x);

    PEGASUS_DEBUG_ASSERT(pos <= in.size());
}

void Packer::unpackUint64(
    const Buffer& in, Uint32& pos, Uint64& x)
{
    memcpy(&x, &in[pos], sizeof(x));
    pos += sizeof(x);

    if (isLittleEndian())
        x = Packer::swapUint64(x);

    PEGASUS_DEBUG_ASSERT(pos <= in.size());
}

void Packer::unpackString(const Buffer& in, Uint32& pos, String& x)
{
    // Determine whether packed as 8-bit or 16-bit.

    Uint8 bits = Uint8(in[pos++]);

    PEGASUS_DEBUG_ASSERT(bits == 16 || bits == 8);

    // Unpack array size.

    Uint32 n;
    unpackSize(in, pos, n);

    if (bits & 8)
    {
        x.assign(&in[pos], n);
        pos += n;
    }
    else
    {
        x.clear();
        x.reserveCapacity(n);

        for (size_t i = 0; i < n; i++)
        {
            Char16 tmp;
            unpackChar16(in , pos, tmp);
            x.append(tmp);
        }
    }

    PEGASUS_DEBUG_ASSERT(pos <= in.size());
}

void Packer::unpackBoolean(
    Buffer& in, Uint32& pos, Boolean* x, Uint32 n)
{
    for (size_t i = 0; i < n; i++)
        unpackBoolean(in, pos, x[i]);

    PEGASUS_DEBUG_ASSERT(pos <= in.size());
}

void Packer::unpackUint8(
    Buffer& in, Uint32& pos, Uint8* x, Uint32 n)
{
    for (size_t i = 0; i < n; i++)
        unpackUint8(in, pos, x[i]);

    PEGASUS_DEBUG_ASSERT(pos <= in.size());
}

void Packer::unpackUint16(
    Buffer& in, Uint32& pos, Uint16* x, Uint32 n)
{
    for (size_t i = 0; i < n; i++)
        unpackUint16(in, pos, x[i]);

    PEGASUS_DEBUG_ASSERT(pos <= in.size());
}

void Packer::unpackUint32(
    Buffer& in, Uint32& pos, Uint32* x, Uint32 n)
{
    for (size_t i = 0; i < n; i++)
        unpackUint32(in, pos, x[i]);

    PEGASUS_DEBUG_ASSERT(pos <= in.size());
}

void Packer::unpackUint64(
    Buffer& in, Uint32& pos, Uint64* x, Uint32 n)
{
    for (size_t i = 0; i < n; i++)
        unpackUint64(in, pos, x[i]);

    PEGASUS_DEBUG_ASSERT(pos <= in.size());
}

void Packer::unpackString(
    Buffer& in, Uint32& pos, String* x, Uint32 n)
{
    for (size_t i = 0; i < n; i++)
        unpackString(in, pos, x[i]);

    PEGASUS_DEBUG_ASSERT(pos <= in.size());
}

PEGASUS_NAMESPACE_END
