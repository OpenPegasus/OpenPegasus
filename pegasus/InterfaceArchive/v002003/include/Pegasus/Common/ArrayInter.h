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
/* NOCHKSRC */

//
// Author: Mike Brasher (mbrasher@bmc.com)
//
// Modified By: Roger Kumpf, Hewlett-Packard Company (roger_kumpf@hp.com)
//
//%/////////////////////////////////////////////////////////////////////////////

// Only include if not included as general template or if explicit instantiation
#if !defined(Pegasus_ArrayInter_h) || defined(PEGASUS_ARRAY_T)
#if !defined(PEGASUS_ARRAY_T)
#define Pegasus_ArrayInter_h
#endif

#include <Pegasus/Common/Linkage.h>

#ifndef PEGASUS_ARRAY_T
/** This class is used to represent arrays of intrinsic data types in CIM. 
*/
template<class PEGASUS_ARRAY_T> class Array
#else
PEGASUS_TEMPLATE_SPECIALIZATION class PEGASUS_COMMON_LINKAGE Array<PEGASUS_ARRAY_T>
#endif
{
public:

    /** Constructs an array object with null values (default constructor).
	*/
    Array();

    /** Creates a new Array object using the parameters and values in the Array object.
	@param x Specifies the new Array object name.
	*/
    Array(const Array<PEGASUS_ARRAY_T>& x);

    /** Constructs an array with size elements. The elements are
        initialized with their copy constructor.
        @param size Defines the number of elements.
    */
    Array(Uint32 size);

    /** Constructs an array with size elements. The elements are
        initialized with array x.
    	@param size Defines the number of elements.
    	@param x Specifies the new array object name.
    */
    Array(Uint32 size, const PEGASUS_ARRAY_T& x);

    /** Constructs an array with size elements. The values come from
        the items pointer.
    	@param items References the values of the specified array.
    	@param size Uint32 representing how many elements are in the array.
    */
    Array(const PEGASUS_ARRAY_T* items, Uint32 size);

    /**Destroys the objects, freeing any resources.
    */
    ~Array();

    /**The values of one array object are assigned to another (assignment operator).
    	@param x Array object to assign the Array parameters to.	
    */
    Array<PEGASUS_ARRAY_T>& operator=(const Array<PEGASUS_ARRAY_T>& x);

    /** Clears the contents of the array. After calling this, size()
        returns zero.
    */
    void clear();

    /** Reserves memory for capacity elements. Notice that this does not
        change the size of the array (size() returns what it did before).
        If the capacity of the array is already greater or equal to the
        capacity argument, this method has no effect. After calling
        reserveCapacity(), getCapacity() returns a value which is greater
        or equal to the capacity argument.
        @param capacity Defines the size that is to be reserved
    */
    void reserveCapacity(Uint32 capacity);

    /** Make the size of the array grow by size elements. The new size will 
        be size() + size. The new elements (there are size of them) are
        initialized with x.
        @param size Defines the number of elements by which the array is to
        grow.
    */
    void grow(Uint32 size, const PEGASUS_ARRAY_T& x);

    /** Swaps the contents of two arrays.  Array x references the original values in the Array object (y) and the values of the Array object (y) reference the original values of Array x.
    */
    void swap(Array<PEGASUS_ARRAY_T>& x);

    /** Returns the number of elements in the array.
        @return The number of elements in the array.
    */
    Uint32 size() const;

    /** Returns the capacity of the array.
    	@return The capacity of the array.
    */
    Uint32 getCapacity() const;

    /** Returns a pointer to the first element of the array.
	    @return A pointer to the first element of the array.
	*/
    const PEGASUS_ARRAY_T* getData() const;

    /** Returns the element indicated by the index argument.
        @return A reference to the element defined by index so that it may be
        modified.
    */
    PEGASUS_ARRAY_T& operator[](Uint32 index);

    /** Returns the element in the const array specified as the index argument. The return value cannot be modified since it
        is constant.
	    @return A reference to the element defined by index but the reference cannot be modified.
    */
    const PEGASUS_ARRAY_T& operator[](Uint32 index) const;

    /** Appends an element to the end of the array. This increases the size
        of the array by one.
        @param x Element to append.
    */
    void append(const PEGASUS_ARRAY_T& x);

    /** Appends size elements at x to the end of this array.
		@param size Uint32 value to append to the size of the array.
	*/
    void append(const PEGASUS_ARRAY_T* x, Uint32 size);

    /** Appends one array to another. The size of this array grows by the
        size of the other.
		@param x Array to add to the appended array.
    */
    void appendArray(const Array<PEGASUS_ARRAY_T>& x);

    /** Appends one element to the beginning of the array. This increases
        the size by one.
        @param x The element to pre pend.
    */
    void prepend(const PEGASUS_ARRAY_T& x);

    /** Appends size elements to the array starting at the memory address
        given by x. The array grows by size elements.
		@param size Uint32 size to add to the array at address x.
		@param x Specifies where to begin increasing size elements of the array.
    */
    void prepend(const PEGASUS_ARRAY_T* x, Uint32 size);

    /** Inserts the element at the given index in the array. Subsequent
        elements are moved down. The size of the array grows by one.
		@param x Specifies the element to add to the array.
    */
    void insert(Uint32 index, const PEGASUS_ARRAY_T& x);

    /** Inserts size elements at x into the array at the given index.
        Subsequent elements are moved down. The size of the array grows
        by size elements.
		@param x Specifies where to begin adding elements in the array.
		@param size Uint32 size to add to the array starting at element x.
    */
    void insert(Uint32 index, const PEGASUS_ARRAY_T* x, Uint32 size);

    /** Removes the element at the given index from the array. The
        size of the array shrinks by one.
		@param index Specifies the array element to remove.
    */
    void remove(Uint32 index);

    /** Removes size elements starting at the given index. The size of
        the array shrinks by size elements.
		@param index Specifies where in the array to begin removing elements.
		@param size Uint32 size that specifies how many elements to remove from the array.
    */
    void remove(Uint32 index, Uint32 size);

private:

    PEGASUS_ARRAY_T* _data() const;

    void* _rep;
};

#endif //!defined(Pegasus_ArrayInter_h) || !defined(PEGASUS_ARRAY_T)
