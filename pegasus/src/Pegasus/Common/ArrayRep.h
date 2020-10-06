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

#ifndef Pegasus_ArrayRep_h
#define Pegasus_ArrayRep_h

#include <new>
#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/Memory.h>
#include <Pegasus/Common/AtomicInt.h>
#include <Pegasus/Common/Linkage.h>

#define Array_rep (static_cast<ArrayRep<PEGASUS_ARRAY_T>*>(_rep))
#define Array_size (Array_rep)->size
#define Array_data (Array_rep)->data()
#define Array_capacity (Array_rep)->capacity
#define Array_refs (Array_rep)->refs

PEGASUS_NAMESPACE_BEGIN

struct PEGASUS_COMMON_LINKAGE ArrayRepBase
{
    // We put this first to avoid gaps in this structure. Some compilers may
    // align it on a large boundary.
    AtomicInt refs;

    Uint32 size;

    union
    {
        Uint32 capacity;
        Uint64 alignment;
    };

    // Called only only _empty_rep object. We set the reference count to
    // two and it will remain two for the lifetime of the process. Anything
    // other than one will do. If the _empty_rep.refs were one, an Array
    // object may think it is the sole owner and attempt to modify it.

    ArrayRepBase() : refs(2), size(0), capacity(0) { }

    static ArrayRepBase _empty_rep;
};

/*  ArrayRep<T>
    The ArrayRep object represents the array size, capacity,
    and elements in one contiguous chunk of memory. The elements
    follow immediately after the end of the ArrayRep structure in memory.
    The union is used to force 64 bit alignment of these elements. This is
    a private class and should not be accessed directly by the user.
*/
template<class T>
struct ArrayRep : public ArrayRepBase
{
    // Obtains a pointer to the first element in the array.
    T* data() { return (T*)(void*)(this + 1); }

    // Same as method above but returns a constant pointer.
    const T* data() const { return (const T*)(void*)(this + 1); }

    /* Make a new copy */
    static ArrayRep<T>* copy_on_write(ArrayRep<T>* rep);

    /* Create and initialize a ArrayRep instance. Note that the
        memory for the elements is unitialized and must be initialized by
        the caller.
    */
    static ArrayRep<T>* alloc(Uint32 size);

    static void ref(const ArrayRep<T>* rep);

    static void unref(const ArrayRep<T>* rep);
};

template<class T>
ArrayRep<T>* ArrayRep<T>::alloc(Uint32 size)
{
    // ATTN-MEB: throw out raising to next power of two (put this
    // logic in reserveCapacity().

    if (!size)
        return (ArrayRep<T>*)&ArrayRepBase::_empty_rep;

    // Calculate capacity (size rounded to the next power of two).

    Uint32 initialCapacity = 8;

    while ((initialCapacity != 0) && (initialCapacity < size))
    {
        initialCapacity <<= 1;
    }

    // Test for Uint32 overflow in the capacity
    if (initialCapacity == 0)
    {
        initialCapacity = size;
    }

    // Test for Uint32 overflow in the memory allocation size
    // throw a bad_alloc exception if overflow would occur.
    if (initialCapacity > (Uint32(0xffffffff)-sizeof(ArrayRep<T>))/sizeof(T))
    {
        throw PEGASUS_STD(bad_alloc)();
    }

    // Create object:

    ArrayRep<T>* rep = (ArrayRep<T>*)operator new(
        sizeof(ArrayRep<T>) + sizeof(T) * initialCapacity);

    rep->size = size;
    rep->capacity = initialCapacity;
    new(&rep->refs) AtomicInt(1);

    return rep;
}

template<class T>
inline void ArrayRep<T>::ref(const ArrayRep<T>* rep)
{
    if ((void*)rep != (void*)&ArrayRepBase::_empty_rep)
        ((ArrayRep<T>*)rep)->refs.inc();
}

template<class T>
inline void ArrayRep<T>::unref(const ArrayRep<T>* rep_)
{
    ArrayRep<T>* rep = (ArrayRep<T>*)rep_;

    if (rep != &ArrayRepBase::_empty_rep && rep->refs.decAndTestIfZero())
    {
        Destroy(rep->data(), rep->size);
        rep->refs.~AtomicInt();
        ::operator delete(rep);
    }
}

template<class T>
ArrayRep<T>* ArrayRep<T>::copy_on_write(ArrayRep<T>* rep)
{
    ArrayRep<T>* new_rep = ArrayRep<T>::alloc(rep->size);
    new_rep->size = rep->size;
    CopyToRaw(new_rep->data(), rep->data(), rep->size);
    unref(rep);
    return new_rep;
}

PEGASUS_NAMESPACE_END

#endif /* Pegasus_ArrayRep_h */
