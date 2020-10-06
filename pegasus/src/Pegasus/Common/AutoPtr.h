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

#ifndef Pegasus_AutoPtr_h
#define Pegasus_AutoPtr_h

#include <Pegasus/Common/Config.h>

PEGASUS_NAMESPACE_BEGIN

/** Function object for deleting pointer referring to a single object.
*/
template<class T>
struct DeletePtr
{
    void operator()(T* ptr)
    {
        delete ptr;
    }
};

/** Function object for freeing (rather than deleting) pointer to char*. For
    example:

        <pre>
        char* ptr = (char*)malloc(80);
        AutoPtr<char, FreeCharPtr> ap(ptr);
        </pre>
*/
struct FreeCharPtr
{
    void operator()(char* ptr)
    {
        free(ptr);
    }
};

/** Function object for deleting a pointer referring to an array.
*/
template<class T>
struct DeleteArrayPtr
{
    void operator()(T* ptr)
    {
        delete [] ptr;
    }
};

/**
    This class provides a convenient way of disposing of a heap object.
    It automatically deletes the enclosed pointer on destruction. For
    example:

    <pre>
        A* a = new A;
        AutoPtr<A> dummy(a);
    </pre>

    When the AutoPtr object destructs, it frees the instance of A.
    This is particularly useful when a function has multiple returns or
    exception conditions.

    There are two classes here: AutoPtr<> and AutoArrayPtr<>. The
    AutoArrayPtr<> class is used when a pointer must be deleted using the
    array form of the delete operator ("delete []").
*/

template<class X, class D = DeletePtr<X> > class AutoPtr
{
public:

    // This constructor helps this AutoPtr to take ownership of the memory
    // object pointed by p. It also acts as a default constructor (if no
    // argument is passed, it assigns a value of "0" to _ptr.
    // Example:  AutoPtr<SSLContext> sslContextA(new SSLContext());
    //           AutoPtr<SSLContext> sslContextB;
    //   sslContextB here has _ptr set to "0".
    explicit AutoPtr(X* p = 0) throw()
        : _ptr(p)
    {
    }

    // Destructor makes sure to delete the object pointed by _ptr thus
    // avoiding memory leaks
    ~AutoPtr() throw()
    {
        d(_ptr);
    }

    // This method can be used to get the pointer to heap object encapsulated
    // in 'this' AutoPtr object
    // Example:   AutoPtr<classA> objA;
    //            func1(objA.get(), NULL);
    // Here func1() is a function which takes first argument as pointer to the
    // object of classA.
    inline X* get() const throw()
    {
        return _ptr;
    }

    // Returns the heap object itself (not the pointer to it)
    inline X& operator*() const throw()
    {
        return *_ptr;
    }

    // A very important overloading, which allows you to directly use 'this'
    // object as a pointer, whenever accessing a member variable/function of
    // the object pointed to by _ptr.
    // Example:   AutoPtr<classA> objA;
    //            objA->funcA();
    // funcA() is a function in the classA. Although objA is an AutoPtr, still
    // "->" operator would result in calling _ptr->funcA() because of this
    // overloading only.
    inline X* operator->() const throw()
    {
        return _ptr;
    }

    // Relase the ownership of the memory object without deleting it !
    // Return the pointer to the heap object and set _ptr to "0".
    inline X* release() throw()
    {
        X* t = _ptr;
        _ptr = 0;
        return t;
    }

    // Delete the heap object and thus release ownership
    inline void reset(X* p = 0) throw()
    {
        if (p != _ptr)
        {
            d(_ptr);
            _ptr = p;
        }
    }

private:
    AutoPtr(const AutoPtr<X>&);
    AutoPtr<X>& operator=(const AutoPtr<X>&);

    // An object that knows how to delete the dynamic memory correctly
    D d;

    // A pointer to the heap object
    X* _ptr;
};


template<class X> class AutoArrayPtr : public AutoPtr<X, DeleteArrayPtr<X> >
{
public:

    explicit AutoArrayPtr(X* p = 0) throw()
        : AutoPtr<X, DeleteArrayPtr<X> >(p)
    {
    }

    X& operator[](Uint32 index) throw()
    {
        return this->get()[index];
    }

    const X& operator[](Uint32 index) const throw()
    {
        return this->get()[index];
    }

private:
    AutoArrayPtr(const AutoArrayPtr<X>&);
    AutoArrayPtr<X>& operator=(const AutoArrayPtr<X>&);
};

PEGASUS_NAMESPACE_END

#endif /* Pegasus_AutoPtr_h */
