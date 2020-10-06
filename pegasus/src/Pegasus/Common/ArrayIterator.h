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

#ifndef Pegasus_ArrayIterator_h
#define Pegasus_ArrayIterator_h

#include <Pegasus/Common/Array.h>

PEGASUS_NAMESPACE_BEGIN

//
// This class provides a faster way of iterating arrays. Due to the expense
// of calling operator[], iteration is slower than necessary. Consider this
// example:
//
//     Array<Uint32> array;
//     Uint32 sum = 0;
//
//     for (Uint32 i = 0, n = array.size(); i < n; i++)
//         sum += array[i];
//
// Every use of array[i] results in a function call that checks for a bounds
// violation. We can see that an array bounds violation is impossible in this
// case. We can use the ConstArrayIterator class to eliminate this overhead.
//
//     Array<Uint32> array;
//     Uint32 sum = 0;
//
//     ConstArrayIterator<Uint32> iterator(array);
//
//     for (Uint32 i = 0; i < iterator.size(); i++)
//         sum += iterator[i];
//
// The constructor pre-saves the array size and data before entering the loop
// so that iterator.size() and iterator[i] are trivial inline functions that
// access these members.
//
template<class T>
class ConstArrayIterator
{
public:

    ConstArrayIterator(const Array<T>& x) : _data(x.getData()), _size(x.size())
    {
    }

    Uint32 size() const
    {
        return _size;
    }

    const T& operator[](Uint32 i)  const
    {
        PEGASUS_DEBUG_ASSERT(i < _size);
        return _data[i];
    }

private:

    ConstArrayIterator& operator=(const ConstArrayIterator<T>& x);
    ConstArrayIterator(const ConstArrayIterator<T>& x);

    const T* _data;
    Uint32 _size;
};

//
// This class is similar to ConstArrayIterator except it provides a non-const
// version of operator[].
//
template<class T>
class ArrayIterator
{
public:

    ArrayIterator(Array<T>& x) : _data((T*)x.getData()), _size(x.size())
    {
    }

    Uint32 size() const
    {
        return _size;
    }

    const T& operator[](Uint32 i)  const
    {
        PEGASUS_DEBUG_ASSERT(i < _size);
        return _data[i];
    }

    T& operator[](Uint32 i)
    {
        PEGASUS_DEBUG_ASSERT(i < _size);
        return _data[i];
    }

    //
    // The reset() method sets the Iterators object members to new
    // data pointer and size of a given Array<T>
    //
    void reset(Array<T>& x)
    {
        _data = (T*)x.getData();
        _size = x.size();
    }

private:

    ArrayIterator& operator=(const ArrayIterator<T>& x);
    ArrayIterator(const ArrayIterator<T>& x);

    T* _data;
    Uint32 _size;
};

PEGASUS_NAMESPACE_END

#endif /* Pegasus_ArrayIterator_h */
