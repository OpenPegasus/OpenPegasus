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

#ifndef Pegasus_SharedPtr_h
#define Pegasus_SharedPtr_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/AtomicInt.h>
#include <Pegasus/Common/AutoPtr.h>

PEGASUS_NAMESPACE_BEGIN

/** Rep class for SharedPtr<> and SharedArrayPtr<>.
*/
template<class T, class D>
class SharedPtrRep
{
public:

    SharedPtrRep()
    {
        _impl = new Impl(0);
    }

    SharedPtrRep(const SharedPtrRep<T, D>& x)
    {
        Impl::ref(_impl = x._impl);
    }

    explicit SharedPtrRep(T* ptr)
    {
        _impl = new Impl(ptr);
    }

    ~SharedPtrRep()
    {
        Impl::unref(_impl);
    }

    void assign(const SharedPtrRep<T, D>& x)
    {
        if (_impl != x._impl)
        {
            Impl::unref(_impl);
            Impl::ref(_impl = x._impl);
        }
    }

    void reset(T* ptr = 0)
    {
        if (ptr == _impl->ptr)
            return;

        if (_impl->refs.get() == 1)
        {
            _impl->d(_impl->ptr);
            _impl->ptr = ptr;
        }
        else
        {
            Impl::unref(_impl);
            _impl = new Impl(ptr);
        }
    }

    T* get() const
    {
        return _impl->ptr;
    }

    size_t count() const
    {
        return _impl->refs.get();
    }

private:

    struct Impl
    {
        D d;
        AtomicInt refs;
        T* ptr;

        Impl(T* ptr_) : refs(1), ptr(ptr_)
        {
        }

        ~Impl()
        {
            d(ptr);
        }

        static void ref(Impl* impl_)
        {
            Impl* impl = (Impl*)(impl_);

            if (impl)
                impl->refs++;
        }

        static void unref(Impl* impl_)
        {
            Impl* impl = (Impl*)(impl_);

            if (impl && impl->refs.decAndTestIfZero())
                delete impl;
        }
    };

    Impl* _impl;
};

/** This class implements a shared pointer, similar to the one found in BOOST
    (and in countless other libraries). The implementation maintains a pointer
    to an intermediate object that contains the pointer to the target object
    as well as a reference counter. Because the target object need not have
    a reference counter of its own, we say that this implementation is
    non-invasive (not imposing requirements on the target class). The instance
    diagram below depicts two SharedPtr objects sharing a target instance (of
    the class Monkey).

    \verbatim
    +-----------+
    | SharedPtr |
    +-----------+    +---------------+
    | _impl     |--->| SharedPtrImpl |
    +-----------+    +---------------+    +--------+
                     | ptr           |--->| Monkey | (target instance)
    +-----------+    +---------------+    +--------+
    | SharedPtr |--->| refs = 2      |
    +-----------+    +---------------+
    + _impl     |
    +-----------+
    \endverbatim

    The code snippet below constructs this instance diagram.

    \code
    SharedPtr p1(new Monkey);
    SharedPtr p2 = p1;
    \endcode

    The target instance is deleted when all shared pointers that refer to it
    are destroyed (or reset).

    A pointer may be reset, which causes the previous referent to be released.
    For example:

    \code
    SharedPtr p(new Monkey);

    // Release the old monkey (destroying it in this case).
    p.reset(new Monkey);
    \endcode

    SharedPtr implements pointer semantics by overloading the related operators.
    For example:

    \code
    SharedPtr p(new Monkey);

    if (p)
    {
        // Print the monkey object (print() is a member of the Monkey class).
        p->print();
    }
    \endcode

    Shared pointers may be dereferenced but beware that doing so to a null
    pointer is an error and will result in a crash.

    \code
    SharedPtr p(0);

    // Core dump! (dereferencing a null pointer).
    Monkey& monkey = *p;

    // Core dump! (dereferencing a null pointer).
    p->print();
    \endcode

    The get() method obtains the target pointer but beware that constructing
    a new SharedPtr with this pointer will result in double deletes (that is
    why the SharedPtr(T*) constructor is explicit). For example, avoid this:

    \code
    SharedPtr p1(new Monkey);
    SharedPtr p2(p1.get());
    \endcode
*/
template<class T, class D = DeletePtr<T> >
class SharedPtr
{
public:

    typedef SharedPtrRep<T, D> Rep;

    SharedPtr()
    {
    }

    SharedPtr(const SharedPtr<T, D>& x) : _rep(x._rep)
    {
    }

    explicit SharedPtr(T* ptr) : _rep(ptr)
    {
    }

    ~SharedPtr()
    {
    }

    SharedPtr<T, D>& operator=(const SharedPtr<T, D>& x)
    {
        _rep.assign(x._rep);
        return *this;
    }

    void reset(T* ptr = 0)
    {
        _rep.reset(ptr);
    }

    T& operator*() const
    {
        return *get();
    }

    T* operator->() const
    {
        return get();
    }

    T* get() const
    {
        return _rep.get();
    }

    operator bool() const
    {
        return get() != 0;
    }

    size_t count() const
    {
        return _rep.count();
    }

protected:

    Rep _rep;
};

/** SharedArrayPtr<> has the same interface as SharedPtr<> but works on
    arrays.
*/
template<class T, class D = DeleteArrayPtr<T> >
class SharedArrayPtr : public SharedPtr<T, D>
{
public:

    SharedArrayPtr()
    {
    }

    explicit SharedArrayPtr(T* ptr) : SharedPtr<T, D>(ptr)
    {
    }

    T& operator[](size_t i) const
    {
        return this->_rep.get()[i];
    }
};

PEGASUS_NAMESPACE_END

#endif /* Pegasus_SharedPtr_h */
