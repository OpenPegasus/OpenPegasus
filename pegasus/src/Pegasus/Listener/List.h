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

#ifndef Pegasus_List_h
#define Pegasus_List_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Listener/Linkage.h>

PEGASUS_NAMESPACE_BEGIN

/**
 * Iterator
 */
class Iterator
{
public:
    /**
     * Destroys the object.  This is needed so that an object of a
     * subclass can be correctly destroyed thru an Iterator pointer.
     */
    virtual ~Iterator() {};

    /**
   * Returns <tt>true</tt> if the iteration has more elements. (In other
   * words, returns <tt>true</tt> if <tt>next</tt> would return an element
   * rather than throwing an exception.)
   *
   * @return <tt>true</tt> if the iterator has more elements.
     */
    virtual Boolean hasNext() = 0;

    /**
     * Returns the next element in the interation.
     *
     * @returns the next element in the interation.
     * @exception NoSuchElementException iteration has no more elements.
     */
    virtual void* next() = 0;

    /**
     *
     * Removes from the underlying collection the last element returned by the
     * iterator (optional operation).  This method can be called only once per
     * call to <tt>next</tt>.  The behavior of an iterator is unspecified if
     * the underlying collection is modified while the iteration is in
     * progress in any way other than by calling this method.
     *
     * @exception UnsupportedOperationException if the <tt>remove</tt>
     *        operation is not supported by this Iterator.
     */
    virtual void remove() = 0;
};

/**
 * PtrList
 */
class PtrList
{
public:
    /**
        Constructs a PtrList object.
     */
    PtrList();
    /**
        Destructor of a PtrList object.
     */
  ~PtrList();

    /**
        Adds an element into the list.
        @param element The element pointer to add.
     */
    void add(void* element);

    /**
        Removes an element from the list
        @param element The element pointer to remove.
     */
    void remove(void* element);

    /**
        Returns an iterator over the elements in this list in proper sequence.
        @return an iterator over the elements in this list in proper sequence.
     */
    Iterator* iterator();

private:
    void* _rep;
};

PEGASUS_NAMESPACE_END

#endif /* Pegasus_List_h */

