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

////////////////////////////////////////////////////////////////////////////////
//
// Memory.h
//
//      This file contains assorted memory-oriented routines:
//
//          Zeros(): fills memory with zeros.
//          Destroy(): destructs multiple objects in contiguous memory.
//          CopyToRaw(): copies multiple objects to raw memory.
//          InitializeRaw(): default constructs mulitple object over raw memory.
//
//      Each of these is a template but specializations are provide for
//      efficiency (which in some cases removes uncessary loops).
//
////////////////////////////////////////////////////////////////////////////////

#ifndef Pegasus_Memory_h
#define Pegasus_Memory_h

#include <cstring>
#include <memory.h>
#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/CIMType.h>
#include <Pegasus/Common/Char16.h>

PEGASUS_NAMESPACE_BEGIN

template<class T>
inline void Zeros(T* items, Uint32 size)
{
    memset(items, 0, sizeof(T) * size);
}

template<class T>
inline void Destroy(T* items, Uint32 size)
{
#ifdef PEGASUS_OS_HPUX
    // Empirical evidence shows that the memory allocator on HP-UX performs
    // better when deallocating more recently allocated memory first (LIFO).
    // Since most arrays are built up by appending elements to the end, it is
    // advantageous to destruct the elements in reverse order.
    items += size-1;
    while (size--)
        (items--)->~T();
#else
    while (size--)
        items++->~T();
#endif
}

template<class T>
inline void CopyToRaw(T* to, const T* from, Uint32 size)
{
    while (size--)
    {
        // The following fails on TRU64:
        //     new(to++) T(*from++);
        // Probably a compiler error so I changed it to this:
        // Mike Brasher

        new(to) T(*from);
        to++;
        from++;
    }
}

inline void CopyToRaw(char* to, const char* from, Uint32 size)
{
    memcpy(to, from, sizeof(char) * size);
}

inline void CopyToRaw(Boolean* to, const Boolean* from, Uint32 size)
{
    memcpy(to, from, sizeof(Boolean) * size);
}

inline void CopyToRaw(Uint8* to, const Uint8* from, Uint32 size)
{
    memcpy(to, from, sizeof(Uint8) * size);
}

inline void CopyToRaw(Sint8* to, const Sint8* from, Uint32 size)
{
    memcpy(to, from, sizeof(Sint8) * size);
}

inline void CopyToRaw(Uint16* to, const Uint16* from, Uint32 size)
{
    memcpy(to, from, sizeof(Uint16) * size);
}

inline void CopyToRaw(Sint16* to, const Sint16* from, Uint32 size)
{
    memcpy(to, from, sizeof(Sint16) * size);
}

inline void CopyToRaw(Uint32* to, const Uint32* from, Uint32 size)
{
    memcpy(to, from, sizeof(Uint32) * size);
}

inline void CopyToRaw(Sint32* to, const Sint32* from, Uint32 size)
{
    memcpy(to, from, sizeof(Sint32) * size);
}

inline void CopyToRaw(Uint64* to, const Uint64* from, Uint32 size)
{
    memcpy(to, from, sizeof(Uint64) * size);
}

inline void CopyToRaw(Sint64* to, const Sint64* from, Uint32 size)
{
    memcpy(to, from, sizeof(Sint64) * size);
}

inline void CopyToRaw(Real32* to, const Real32* from, Uint32 size)
{
    memcpy(to, from, sizeof(Real32) * size);
}

inline void CopyToRaw(Real64* to, const Real64* from, Uint32 size)
{
    memcpy(to, from, sizeof(Real64) * size);
}

inline void CopyToRaw(Char16* to, const Char16* from, Uint32 size)
{
    memcpy(to, from, sizeof(Char16) * size);
}

template<class T>
inline void InitializeRaw(T* items, Uint32 size)
{
    while (size--)
    {
        new(items) T();
        items++;
    }
}

#define PEGASUS_MEMORY_FUNCTIONS(T) \
    inline void Destroy(T*, Uint32) { } \
    inline void InitializeRaw(T* items, Uint32 size) { Zeros(items, size); }

PEGASUS_MEMORY_FUNCTIONS(char*)
PEGASUS_MEMORY_FUNCTIONS(const char*)
PEGASUS_MEMORY_FUNCTIONS(Boolean)
PEGASUS_MEMORY_FUNCTIONS(Uint8)
PEGASUS_MEMORY_FUNCTIONS(Sint8)
PEGASUS_MEMORY_FUNCTIONS(Uint16)
PEGASUS_MEMORY_FUNCTIONS(Sint16)
PEGASUS_MEMORY_FUNCTIONS(Uint32)
PEGASUS_MEMORY_FUNCTIONS(Sint32)
PEGASUS_MEMORY_FUNCTIONS(Uint64)
PEGASUS_MEMORY_FUNCTIONS(Sint64)
PEGASUS_MEMORY_FUNCTIONS(Real32)
PEGASUS_MEMORY_FUNCTIONS(Real64)
PEGASUS_MEMORY_FUNCTIONS(Char16)

PEGASUS_NAMESPACE_END

#endif /* Pegasus_Memory_h */
