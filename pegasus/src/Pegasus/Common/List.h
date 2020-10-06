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

#ifndef _Pegasus_Common_List_h
#define _Pegasus_Common_List_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/Linkage.h>
#include <Pegasus/Common/Linkable.h>
#include <Pegasus/Common/Magic.h>

PEGASUS_NAMESPACE_BEGIN

/** The NullLock class can be passed as the LockType template parameter to the
    List template class. The NullLock implements no-ops for the lock(),
    try_lock() and unlock() operations.
*/
class NullLock
{
public:

    /** no-op lock().
    */
    void lock() { }

    /** no-op try_lock().
    */
    Boolean try_lock() { return true; }

    /** no-op unlock().
    */
    void unlock() { }
};

// Intrusive list implementation class (internal to List class).
class PEGASUS_COMMON_LINKAGE ListRep
{
public:

    typedef bool (*Equal)(const Linkable* elem, const void* client_data);

    typedef void (*Apply)(const Linkable* elem, const void* client_data);

    ListRep(void (*destructor)(Linkable*));

    ~ListRep();

    void clear();

    Uint32 size() const { return _size; }

    Boolean empty() const { return _size == 0; }

    Linkable* front() { return _front; }

    const Linkable* front() const { return _front; }

    Linkable* back() { return _back; }

    const Linkable* back() const { return _back; }

    void next(Linkable*& elem) { elem = elem->next; };

    void prev(Linkable*& elem) { elem = elem->prev; };

    void insert_front(Linkable* elem);

    void insert_back(Linkable* elem);

    void insert_after(Linkable* pos, Linkable* elem);

    void insert_before(Linkable* pos, Linkable* elem);

    void remove(Linkable* pos);

    bool contains(const Linkable* elem);

    Linkable* remove_front();

    Linkable* remove_back();

    Linkable* find(Equal equal, const void* client_data);

    Linkable* remove(Equal equal, const void* client_data);

    void apply(Apply apply, const void* client_data);

private:

    ListRep(const ListRep&) { }
    ListRep& operator=(const ListRep&) { return *this; }

    Magic<0x6456FD0A> _magic;
    Linkable* _front;
    Linkable* _back;
    Uint32 _size;
    void (*_destructor)(Linkable*);
};

/** The List class implements an intrusive linked list. We say it is intrusive
    because the element class must derive from Linkable, which provides the
    link fields used by this implementation. An intrusive implementation has
    two main benefits: it is more efficient and uses less memory. Recall that
    a non-intrusive linked list must allocate an additional "node" object that
    contains the links fields as well as a pointer to the element. This implies
    that removal is O(N) since an element must be located before it can be
    removed. Also, the extra memory object compromises space and efficiency.

    The List template takes two arguments: the element type and the lock type.
    Again, the element type must derive from Linkable. The lock type is used
    to synchronize access to the list operations and can be any class that
    implements these methods: lock(), try_lock(), unlock(). Here are three
    classes that can be used as the lock type.

        - NullLock - no locking at all.
        - Mutex - non-recursive mutex.
        - Mutex - recursive mutex.

    Now we consider an example. So you want to create a list of Person
    elements. First you must derive from the Linkable class as shown below.

        \code
        class Person : public Linkable
        {
            Person(const String& name);
            .
            .
            .
        }
        \endcode

    Linkable is a non-virtual base class and contains the link fields this
    implementation will use to place Person elements onto the list. Second,
    you must instantiate the List template. Here are three possibilities.

        \code
        List<Person, NullLink>; // Do no synchronization.
        List<Person, Mutex>; // Use Mutex class to synchronize.
        List<Person, Mutex>; // Use Mutex class to synchronize.
        \endcode

    Finally, use the list. The following example adds three Person objects
    to the back of the list and removes one from the front.

        \code
        typedef List<Person, Mutex> PersonList;
        PersonList list;

        list.insert_back(new Person("John"));
        list.insert_back(new Person("Mary"));
        list.insert_back(new Person("Jane"));

        Person* person = list.remove_front();
        \endcode

    One recurring usage pattern is exhausting the elements of a list. For
    example (this is thread-safe).

        \code
        Person* person;

        while ((person = list.remove_front())
        {
            // Do something with person here.
        }
        \endcode

    Another pattern is to iterate the elements of the list, which requires a
    lock for the duration of the iteration. For example:

        \code
        list.lock();

        for (Person* p = list.front(); p; p = list.next_of(p))
        {
            // Use p here!
        }

        list.unlock();
        \endcode

    Beware that this pattern fails for non-recursive mutexes since the mutex
    is locked once by our example and again by front(). You may have noticed
    that if an exception is thrown before unlock(), that the mutex will
    remain locked. For this reason, the List class contains an AutoLock class
    that will automatically release the mutex. We now rework the previous
    example to use an AutoLock object.

        \code
        {
            PersonList::AutoLock autoLock(list);

            for (Person* p = list.front(); p; p = list.next_of(p))
            {
                // Use p here!
            }
        }
        \endcode

    The List class provides a special find function to simplify recurring
    lookups of an element by some specific criteria. For example, lets add an
    equal function to our Person class as follows:

        \code
        class Person : public Linkable
        {
            Person(const String& name);
            const String& name();

            static bool equal(const Person* person, const char* client_data)
            {
                const String& name = *((const String*)client_data);
                return person->name() == name;
            }
        }
        \endcode

    This equal function may now be passed to the find() function to lookup
    a specific person in a thread-safe way. For example:

        \code
        const String name = "John";
        Person* person = list.find(Person::equal, &name);

        if (person)
        {
            // Found John!
        }
        \endcode

    Similary, we can delete "John" by using the equal-form of remove().

        \code
        const String name = "John";
        Person* person = list.remove(Person::equal, &name);
        \endcode

    Finally, we can "apply" a function to every element of the list using
    the apply method. For example:

        \code
        static void _print(Person* person, const void* client_data)
        {
            printf("%s\n", (const char*)client_data);
            person->print();
        }

        list.apply(_print, "My List");
        \endcode
*/
template<class ElemType, class LockType>
class List
{
public:

    typedef List<ElemType, LockType> This;

    typedef bool (*Equal)(const ElemType* elem, const void* client_data);

    typedef void (*Apply)(const ElemType* elem, const void* client_data);

    /** Default constructor.
    */
    List() : _rep(_destructor)
    {
    }

    /** Destructor.
    */
    ~List()
    {
        AutoLock al(*this);
        _rep.clear();
    }

    /** Remove and delete all elements in this list. size() returns zero
        after this is called.
    */
    void clear()
    {
        AutoLock al(*this);
        _rep.clear();
    }

    /** Returns the number of elements in the list.
    */
    Uint32 size() const
    {
        AutoLock al(*this);
        return _rep.size();
    }

    /** Returns true if the list is empty (i.e., has zero elements).
    */
    Boolean is_empty() const
    {
        AutoLock al(*this);
        return _rep.empty();
    }

    /** Obtains a pointer to the element at the front of the list.
    */
    ElemType* front()
    {
        AutoLock al(*this);
        return static_cast<ElemType*>(_rep.front());
    }

    /** Obtains a const-pointer to the element at the front of the list.
    */
    const ElemType* front() const
    {
        AutoLock al(*this);
        return static_cast<const ElemType*>(_rep.front());
    }

    /** Obtains a pointer to the element at the back of the list.
    */
    ElemType* back()
    {
        AutoLock al(*this);
        return static_cast<ElemType*>(_rep.back());
    }

    /** Obtains a const-pointer to the element at the back of the list.
    */
    const ElemType* back() const
    {
        AutoLock al(*this);
        return static_cast<const ElemType*>(_rep.back());
    }

    /** Returns the element after elem (used to iterate list).
    */
    ElemType* next_of(ElemType* elem)
    {
        return static_cast<ElemType*>(elem->next);
    }

    /** Returns the element before elem (used to iterate list).
    */
    ElemType* prev_of(ElemType* elem)
    {
        return static_cast<ElemType*>(elem->prev);
    }

    /** Insert elem at the front of the list.
    */
    void insert_front(ElemType* elem)
    {
        AutoLock al(*this);
        _rep.insert_front(elem);
    }

    /** Insert elem at the back of the list.
    */
    void insert_back(ElemType* elem)
    {
        AutoLock al(*this);
        _rep.insert_back(elem);
    }

    /** Insert elem after pos.
    */
    void insert_after(ElemType* pos, ElemType* elem)
    {
        AutoLock al(*this);
        _rep.insert_after(pos, elem);
    }

    /** Insert elem before pos.
    */
    void insert_before(ElemType* pos, ElemType* elem)
    {
        AutoLock al(*this);
        _rep.insert_before(pos, elem);
    }

    /** Remove the given element.
    */
    void remove(ElemType* pos)
    {
        AutoLock al(*this);
        _rep.remove(pos);
    }

    /** Returns true if the list contains the given element.
    */
    bool contains(const ElemType* elem)
    {
        AutoLock al(*this);
        return _rep.contains(elem);
    }

    /** Removes and returns the element at the front of the list.
    */
    ElemType* remove_front()
    {
        AutoLock al(*this);
        return static_cast<ElemType*>(_rep.remove_front());
    }

    /** Removes and returns the element at the back of the list.
    */
    ElemType* remove_back()
    {
        AutoLock al(*this);
        return static_cast<ElemType*>(_rep.remove_back());
    }

    /** Attempts to find an element in the list that satisfies the equal()
        predicate. Returns first element satisfying equal() or null if none.
    */
    ElemType* find(Equal equal, const void* client_data)
    {
        AutoLock al(*this);
        return static_cast<ElemType*>(
            _rep.find(ListRep::Equal(equal), client_data));
    }

    /** Attempts to remove the element in the list that satisfies the equal()
        predicate. Removes and returns the first element satisfying equal() or
        null if none.
    */
    ElemType* remove(Equal equal, const void* client_data)
    {
        AutoLock al(*this);
        return static_cast<ElemType*>(
            _rep.remove(ListRep::Equal(equal), client_data));
    }

    /** Apply the given function to every element of the list.
    */
    void apply(Apply apply, const void* client_data)
    {
        AutoLock al(*this);
        _rep.apply(ListRep::Apply(apply), client_data);
    }

    /** Lock the list.
    */
    void lock() { _lock.lock(); }

    /** Try to lock the list without blocking.
        @return A Boolean indicating whether the lock was acquired.
    */
    Boolean try_lock() { return _lock.try_lock(); }

    /** Unlock the list.
    */
    void unlock() { _lock.unlock(); }

    /** Get reference to lock type.
    */
    LockType& getLock() const { return ((This*)this)->_lock; }

    /** An instance of this class is used to lock this list on construction
        and later unlock it on destruction.
    */
    class AutoLock
    {
    public:

        /**
        */
        AutoLock(const This& list) : _lock(list.getLock())
        {
            _lock.lock();
        }

        /**
        */
        ~AutoLock()
        {
            _lock.unlock();
        }

    private:
        LockType& _lock;
    };

private:

    static void _destructor(Linkable* ptr)
    {
        delete static_cast<ElemType*>(ptr);
    }

    ListRep _rep;
    LockType _lock;
};

PEGASUS_NAMESPACE_END

#endif /* _Pegasus_Common_List_h */
