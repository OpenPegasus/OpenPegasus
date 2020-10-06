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

#ifndef Pegasus_Stack_h
#define Pegasus_Stack_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/ArrayInternal.h>
#include <Pegasus/Common/String.h>
#include <Pegasus/Common/InternalException.h>

PEGASUS_NAMESPACE_BEGIN

/** The Stack class provides a simple stack implementation.
    This class provides a stack implementation which is based on the Array<>
    template class.
*/
template<class T>
class Stack
{
public:

    /** Default constructor. */
    Stack() { }

    /** Copy constructor. */
    Stack(const Stack<T>& x) : _rep(x._rep) { }

    /** This constructor was added to provide a fast way of creating a stack
        with a single element on it. This constructor is necessary to realize
        the return-value compiler optimization which permits objects used in
        return/constructor expressions to be initialized only once.

        Notice that this constructor is explicit to avoid implicit
        initialization of a stack with the type of T.
        which
    */
    explicit Stack(const T& x) { _rep.append(x); }

    /** Destructor. */
    ~Stack() { }

    /** Assignment operator. */
    Stack<T>& operator=(const Stack<T>& x) { _rep = x._rep; return *this; }

    /** Returns size of stack. */
    Uint32 size() const { return _rep.size(); }

    /** Tests whether stack is empty. */
    Boolean isEmpty() const { return _rep.size() == 0; }

    /** Pushes entry onto the stack. */
    void push(const T& x) { _rep.append(x); }

    /** Returns reference to the top element on the stack.
        @return reference to top element on stack.
        @exception throws StackUnderflow if stack is empty.
    */
    T& top();

    /** Const version of top() method.
    */
    const T& top() const { return ((Stack<T>*)this)->top(); }

    /** Pops top entry from stack. */
    void pop();

    /** Provides indexing for stack. */
    T& operator[](Uint32 i) { return _rep[i]; }

    /** Const version of indxing operator. */
    const T& operator[](Uint32 i) const { return _rep[i]; }

    void reserveCapacity(Uint32 capacity) { _rep.reserveCapacity(capacity); }

private:

    Array<T> _rep;
};

template<class T>
T& Stack<T>::top()
{
    if (isEmpty())
        throw StackUnderflow();

    return _rep[_rep.size() - 1];
}

template<class T>
void Stack<T>::pop()
{
    if (isEmpty())
        throw StackUnderflow();

    _rep.remove(_rep.size() - 1);
}

PEGASUS_NAMESPACE_END

#endif /* Pegasus_Stack_h */
