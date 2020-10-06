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

// Only include if not included as general template or if explicit instantiation
#if !defined(Pegasus_ArrayImpl_h) || defined(PEGASUS_ARRAY_T)
#if !defined(PEGASUS_ARRAY_T)
#define Pegasus_ArrayImpl_h
#endif

PEGASUS_NAMESPACE_END

#include <Pegasus/Common/Memory.h>
#include <Pegasus/Common/ArrayRep.h>
#include <Pegasus/Common/InternalException.h>
#include <Pegasus/Common/Linkage.h>

PEGASUS_NAMESPACE_BEGIN

PEGASUS_COMMON_LINKAGE void ArrayThrowIndexOutOfBoundsException();

#ifndef PEGASUS_ARRAY_T
template<class PEGASUS_ARRAY_T>
#endif
Array<PEGASUS_ARRAY_T>::Array()
{
    _rep = &ArrayRepBase::_empty_rep;
}

#ifndef PEGASUS_ARRAY_T
template<class PEGASUS_ARRAY_T>
#endif
Array<PEGASUS_ARRAY_T>::Array(const Array<PEGASUS_ARRAY_T>& x)
{
    _rep = x._rep;
    ArrayRep<PEGASUS_ARRAY_T>::ref(Array_rep);
}

#ifndef PEGASUS_ARRAY_T
template<class PEGASUS_ARRAY_T>
#endif
Array<PEGASUS_ARRAY_T>::Array(Uint32 size)
{
    _rep = ArrayRep<PEGASUS_ARRAY_T>::alloc(size);

    // ArrayRep<PEGASUS_ARRAY_T>::alloc() throws a bad_alloc exception if
    // storage could not be obtained.

    InitializeRaw(Array_data, size);
}

#ifndef PEGASUS_ARRAY_T
template<class PEGASUS_ARRAY_T>
#endif
Array<PEGASUS_ARRAY_T>::Array(Uint32 size, const PEGASUS_ARRAY_T& x)
{
    _rep = ArrayRep<PEGASUS_ARRAY_T>::alloc(size);

    // ArrayRep<PEGASUS_ARRAY_T>::alloc() throws a bad_alloc exception if
    // storage could not be obtained.

    PEGASUS_ARRAY_T* data = Array_data;

    // Note: we could use template specialization (by adding functions to
    // Memory.h) so that this loop becomes a memset() for single byte raw
    // types, but this function is rarely called.

    while (size--)
        new(data++) PEGASUS_ARRAY_T(x);
}

#ifndef PEGASUS_ARRAY_T
template<class PEGASUS_ARRAY_T>
#endif
Array<PEGASUS_ARRAY_T>::Array(const PEGASUS_ARRAY_T* items, Uint32 size)
{
    _rep = ArrayRep<PEGASUS_ARRAY_T>::alloc(size);

    // ArrayRep<PEGASUS_ARRAY_T>::alloc() throws a bad_alloc exception if
    // storage could not be obtained.

    CopyToRaw(Array_data, items, size);
}

#ifndef PEGASUS_ARRAY_T
template<class PEGASUS_ARRAY_T>
#endif
Array<PEGASUS_ARRAY_T>::~Array()
{
    ArrayRep<PEGASUS_ARRAY_T>::unref(Array_rep);
}

#ifndef PEGASUS_ARRAY_T
template<class PEGASUS_ARRAY_T>
#endif
Array<PEGASUS_ARRAY_T>& Array<PEGASUS_ARRAY_T>::operator=(
    const Array<PEGASUS_ARRAY_T>& x)
{
    if (x._rep != Array_rep)
    {
        ArrayRep<PEGASUS_ARRAY_T>::unref(Array_rep);
        _rep = x._rep;
        ArrayRep<PEGASUS_ARRAY_T>::ref(Array_rep);
    }

    return *this;
}

#ifndef PEGASUS_ARRAY_T
template<class PEGASUS_ARRAY_T>
#endif
void Array<PEGASUS_ARRAY_T>::clear()
{
    if (Array_size)
    {
        if (Array_refs.get() == 1)
        {
            Destroy(Array_data, Array_size);
            Array_size = 0;
        }
        else
        {
            ArrayRep<PEGASUS_ARRAY_T>::unref(Array_rep);
            _rep = &ArrayRepBase::_empty_rep;
        }
    }
}

#ifndef PEGASUS_ARRAY_T
template<class PEGASUS_ARRAY_T>
#endif
void Array<PEGASUS_ARRAY_T>::reserveCapacity(Uint32 capacity)
{
    if (capacity > Array_capacity || Array_refs.get() != 1)
    {
        ArrayRep<PEGASUS_ARRAY_T>* rep =
            ArrayRep<PEGASUS_ARRAY_T>::alloc(capacity);

        // ArrayRep<PEGASUS_ARRAY_T>::alloc() throws a bad_alloc exception if
        // storage could not be obtained.

        rep->size = Array_size;

        if (Array_refs.get() == 1)
        {
            memcpy( 
                (void*)rep->data(), 
                (void*)Array_data, 
                Array_size*sizeof(PEGASUS_ARRAY_T));
            Array_size = 0;
        }
        else
            CopyToRaw(rep->data(), Array_data, Array_size);

        ArrayRep<PEGASUS_ARRAY_T>::unref(Array_rep);
        _rep = rep;
    }
}

#ifndef PEGASUS_ARRAY_T
template<class PEGASUS_ARRAY_T>
#endif
void Array<PEGASUS_ARRAY_T>::grow(Uint32 size, const PEGASUS_ARRAY_T& x)
{
    reserveCapacity(Array_size + size);
    PEGASUS_ARRAY_T* p = Array_data + Array_size;
    Uint32 n = size;

    while (n--)
        new(p++) PEGASUS_ARRAY_T(x);

    Array_size += size;
}

#ifndef PEGASUS_ARRAY_T
template<class PEGASUS_ARRAY_T>
#endif
void Array<PEGASUS_ARRAY_T>::swap(Array<PEGASUS_ARRAY_T>& x)
{
    ArrayRep<PEGASUS_ARRAY_T>* tmp = Array_rep;
    _rep = x._rep;
    x._rep = tmp;
}

#ifndef PEGASUS_ARRAY_T
template<class PEGASUS_ARRAY_T>
#endif
void Array<PEGASUS_ARRAY_T>::append(const PEGASUS_ARRAY_T& x)
{
    Uint32 n = Array_size + 1;

    if (n > Array_capacity || Array_refs.get() != 1)
        reserveCapacity(n);

    new (Array_data + Array_size) PEGASUS_ARRAY_T(x);
    Array_size++;
}

#ifndef PEGASUS_ARRAY_T
template<class PEGASUS_ARRAY_T>
#endif
void Array<PEGASUS_ARRAY_T>::append(const PEGASUS_ARRAY_T* x, Uint32 size)
{
    Uint32 n = Array_size + size;
    reserveCapacity(n);
    CopyToRaw(Array_data + Array_size, x, size);
    Array_size = n;
}

#ifndef PEGASUS_ARRAY_T
template<class PEGASUS_ARRAY_T>
#endif
void Array<PEGASUS_ARRAY_T>::appendArray(const Array<PEGASUS_ARRAY_T>& x)
{
    append(x.getData(), x.size());
}

#ifndef PEGASUS_ARRAY_T
template<class PEGASUS_ARRAY_T>
#endif
void Array<PEGASUS_ARRAY_T>::prepend(const PEGASUS_ARRAY_T& x)
{
    prepend(&x, 1);
}

#ifndef PEGASUS_ARRAY_T
template<class PEGASUS_ARRAY_T>
#endif
void Array<PEGASUS_ARRAY_T>::prepend(const PEGASUS_ARRAY_T* x, Uint32 size)
{
    reserveCapacity(Array_size + size);
    memmove(
        (void*)(Array_data + size),
        (void*)Array_data,
        sizeof(PEGASUS_ARRAY_T) * Array_size);
    CopyToRaw(Array_data, x, size);
    Array_size += size;
}

#ifndef PEGASUS_ARRAY_T
template<class PEGASUS_ARRAY_T>
#endif
void Array<PEGASUS_ARRAY_T>::insert(Uint32 index, const PEGASUS_ARRAY_T& x)
{
    insert(index, &x, 1);
}

#ifndef PEGASUS_ARRAY_T
template<class PEGASUS_ARRAY_T>
#endif
void Array<PEGASUS_ARRAY_T>::insert(
    Uint32 index, const PEGASUS_ARRAY_T* x, Uint32 size)
{
    if (index > Array_size)
    {
        throw IndexOutOfBoundsException();
    }

    reserveCapacity(Array_size + size);

    Uint32 n = Array_size - index;

    if (n)
    {
        memmove(
            Array_data + index + size,
            Array_data + index,
            sizeof(PEGASUS_ARRAY_T) * n);
    }

    CopyToRaw(Array_data + index, x, size);
    Array_size += size;
}

#ifndef PEGASUS_ARRAY_T
template<class PEGASUS_ARRAY_T>
#endif
void Array<PEGASUS_ARRAY_T>::remove(Uint32 index)
{
    remove(index, 1);
}

#ifndef PEGASUS_ARRAY_T
template<class PEGASUS_ARRAY_T>
#endif
void Array<PEGASUS_ARRAY_T>::remove(Uint32 index, Uint32 size)
{
    if (size == 0)
    {
        return;
    }

    if (Array_refs.get() != 1)
        _rep = ArrayRep<PEGASUS_ARRAY_T>::copy_on_write(Array_rep);

    // Case 1: attempting to remove last element (this is an optimization
    // for when the array is used as a stack; see Stack class).

    if (index + 1 == Array_size)
    {
        Destroy(Array_data + index, 1);
        Array_size--;
        return;
    }

    // Case 2: not attempting to remove last element:

    if (index + size - 1 > Array_size)
    {
        throw IndexOutOfBoundsException();
    }

    Destroy(Array_data + index, size);
    Uint32 rem = Array_size - (index + size);

    if (rem)
    {
        memmove(
            Array_data + index,
            Array_data + index + size,
            sizeof(PEGASUS_ARRAY_T) * rem);
    }

    Array_size -= size;
}

#ifndef PEGASUS_ARRAY_T
template<class PEGASUS_ARRAY_T>
#endif
Uint32 Array<PEGASUS_ARRAY_T>::size() const
{
    return Array_size;
}

#ifndef PEGASUS_ARRAY_T
template<class PEGASUS_ARRAY_T>
#endif
PEGASUS_ARRAY_T& Array<PEGASUS_ARRAY_T>::operator[](
    Uint32 index)
{
    if (index >= Array_size)
        ArrayThrowIndexOutOfBoundsException();

    if (Array_refs.get() != 1)
        _rep = ArrayRep<PEGASUS_ARRAY_T>::copy_on_write(Array_rep);

    return Array_data[index];
}

#ifndef PEGASUS_ARRAY_T
template<class PEGASUS_ARRAY_T>
#endif
const PEGASUS_ARRAY_T& Array<PEGASUS_ARRAY_T>::operator[](
    Uint32 index) const
{
    if (index >= Array_size)
        ArrayThrowIndexOutOfBoundsException();

    return Array_data[index];
}

#ifndef PEGASUS_ARRAY_T
template<class PEGASUS_ARRAY_T>
#endif
Uint32 Array<PEGASUS_ARRAY_T>::getCapacity() const
{
    return Array_capacity;
}

#ifndef PEGASUS_ARRAY_T
template<class PEGASUS_ARRAY_T>
#endif
const PEGASUS_ARRAY_T* Array<PEGASUS_ARRAY_T>::getData() const
{
    return Array_data;
}

#ifndef PEGASUS_ARRAY_T
template<class PEGASUS_ARRAY_T>
#endif
PEGASUS_ARRAY_T* Array<PEGASUS_ARRAY_T>::_data() const
{
    return Array_data;
}

#endif //!defined(Pegasus_ArrayImpl_h) || !defined(PEGASUS_ARRAY_T)
