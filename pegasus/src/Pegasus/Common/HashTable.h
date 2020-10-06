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

#ifndef Pegasus_HashTable_h
#define Pegasus_HashTable_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/String.h>
#include <Pegasus/Common/CIMObjectPath.h>
#include <Pegasus/Common/Linkage.h>

PEGASUS_NAMESPACE_BEGIN

/*  This is the default hash function object used by the HashTable template.
    Specializations are provided for common types.
*/
template<class K>
struct HashFunc
{
};

PEGASUS_TEMPLATE_SPECIALIZATION struct PEGASUS_COMMON_LINKAGE HashFunc<String>
{
    static Uint32 hash(const String& str);
};

PEGASUS_TEMPLATE_SPECIALIZATION struct HashFunc<Uint32>
{
    static Uint32 hash(Uint32 x) { return x + 13; }
};

PEGASUS_TEMPLATE_SPECIALIZATION struct HashFunc <CIMObjectPath>
{
    static Uint32 hash (const CIMObjectPath & path)
    {
        return path.makeHashCode ();
    }
};

//
// Computes a hash code for a string without regard to case. For example, it
// yields the same hash code for "AB", "ab", "Ab", and "aB".
//
struct PEGASUS_COMMON_LINKAGE HashLowerCaseFunc
{
    static Uint32 hash(const String& str);
};

/*  This is a function object used by the HashTable to compare keys. This is
    the default implementation. Others may be defined and passed in the
    template argument list to perform other kinds of comparisons.
*/
template<class K>
struct EqualFunc
{
    static Boolean equal(const K& x, const K& y)
    {
        return x == y;
    }
};

PEGASUS_TEMPLATE_SPECIALIZATION struct EqualFunc <CIMObjectPath>
{
    static Boolean equal (const CIMObjectPath & x, const CIMObjectPath & y)
    {
        return x.identical (y);
    }
};

/*
    Equal function object that can be used by HashTable to compare keys that
    should be treated as case insensitive.

    This function can be used for hash table keys constructed from strings that
    should be treated as case insensitive (e.g. class names, namespace names,
    system names).

    Note: this function compares Strings based on the process locale.
*/
struct EqualNoCaseFunc
{
    static Boolean equal (const String & x, const String & y)
    {
        return (0 == String::compareNoCase (x, y));
    }
};

/*  Representation for a bucket. The HashTable class derives from this
    bucket to append a key and value. This base class just defines
    the pointer to the next bucket in the chain.
*/
class PEGASUS_COMMON_LINKAGE _BucketBase
{
public:

    /* Default constructor. */
    _BucketBase() : next(0) { }

    /* Virtual destructor to ensure destruction of derived class
        elements.
    */
    virtual ~_BucketBase();

    /* returns true if the key pointed to by the key argument is equal
        to the internal key of this bucket. This method must be overridden
        by the derived class.
    */
    virtual Boolean equal(const void* key) const = 0;

    /* Clone this bucket. */
    virtual _BucketBase* clone() const = 0;

    _BucketBase* next;
};

/* This class implements a simple hash table forward iterator. */
class PEGASUS_COMMON_LINKAGE _HashTableIteratorBase
{
public:

    _HashTableIteratorBase(_BucketBase** first, _BucketBase** last);

    operator int() const { return _bucket != 0; }

    void operator++();

    void operator++(int)
    {
        operator++();
    }

protected:

    // Note:  The default copy constructor/assignment operator is used by the
    // postfix increment operator.  The member pointers may be safely copied
    // because they refer to structures that must not change while the iterator
    // is in scope.

    _BucketBase** _first;
    _BucketBase** _last;
    _BucketBase* _bucket;
};

// ATTN: reorganization not supported yet.

/*- The _HashTableRep class is the representation class used by HashTable.

    This code is primarily an internal class used to implement the HashTable.
    But there may be occasions to use it directly.

    _HashTableRep parcels out much of the large code so that that code is not
    instantiated by the HashTable template class many times. This scheme helps
    reduce code bloat caused by templates. The HashTable template class below
    acts as kind of a wrapper around this class.

    _HashTableRep is implemented as an array of pointers to chains of hash
    buckets. The table initially allocates some number of chains (which can
    be controlled by the constructor) and then may increase the number of
    chains later (resulting in a reorganization of the hash table).
*/
class PEGASUS_COMMON_LINKAGE _HashTableRep
{
public:

    /*- This constructor allocates an array of pointers to chains of buckets,
        which of course are all empty at this time. The numChains argument
        If the numChains argument is less than eight, then eight chains will
        be created.
        @param numChains specifies the initial number of chains.
    */
    _HashTableRep(Uint32 numChains);

    /*- Copy constructor. */
    _HashTableRep(const _HashTableRep& x);

    /*- Destructor. */
    ~_HashTableRep();

    /*- Assignment operator. */
    _HashTableRep& operator=(const _HashTableRep& x);

    /*- Returns the size of this hash table (the number of entries). */
    Uint32 size() const { return _size; }

    /*- Clears the contents of this hash table. After this is called, the
        size() method returns zero.
    */
    void clear();

    /*- Inserts new key-value pair into hash table. Deletes the bucket on
        failure so caller need not.
        @param hashCode hash code generated by caller's hash function.
        @param bucket bucket to be inserted.
        @param key pointer to key.
        @return true if insertion successful; false if duplicate key.
    */
    Boolean insert(Uint32 hashCode, _BucketBase* bucket, const void* key);

    /*- Finds the bucket with the given key. This method uses the
        _BucketBase::equal() method to compare keys.
        @param hashCode hash code generated by caller's hash function.
        @param key void pointer to key.
        @return pointer to bucket with that key or zero otherwise.
    */
    const _BucketBase* lookup(Uint32 hashCode, const void* key) const;

    /*- Removes the bucket with the given key. This method uses the
        _BucketBase::equal() method to compare keys.
        @param hashCode hash code generated by caller's hash function.
        @param key void pointer to key.
        @return true if entry found and removed and false otherwise.
    */
    Boolean remove(Uint32 hashCode, const void* key);

    _BucketBase** getChains() const { return _chains; }

    Uint32 getNumChains() const { return _numChains; }

protected:

    Uint32 _size;
    Uint32 _numChains;
    _BucketBase** _chains;
};

/* The _Bucket class is used to implement the HashTable class.
*/
template<class K, class V, class E>
class _Bucket : public _BucketBase
{
public:

    _Bucket(const K& key, const V& value) : _key(key), _value(value) { }

    virtual ~_Bucket();

    virtual Boolean equal(const void* key) const;

    virtual _BucketBase* clone() const;

    K& getKey() { return _key; }

    V& getValue() { return _value; }

private:

    K _key;
    V _value;
};

template<class K, class V, class E>
Boolean _Bucket<K, V, E>::equal(const void* key) const
{
    return E::equal(*((K*)key), _key);
}

template<class K, class V, class E>
_Bucket<K, V, E>::~_Bucket()
{

}

template<class K, class V, class E>
_BucketBase* _Bucket<K, V, E>::clone() const
{
    return new _Bucket<K, V, E>(_key, _value);
}

/* Iterator for HashTable class. */
template<class K, class V, class E>
class _HashTableIterator : public _HashTableIteratorBase
{
public:

    _HashTableIterator(_BucketBase** first, _BucketBase** last)
        : _HashTableIteratorBase(first, last) { }

    const K& key() const { return ((_Bucket<K, V, E>*)_bucket)->getKey(); }

    const V& value() const { return ((_Bucket<K, V, E>*)_bucket)->getValue(); }
};

/** The HashTable class provides a simple hash table implementation which
    associates key-value pairs.

    This implementation minimizes template bloat considerably by factoring out
    most of the code into a common non-template class (see _HashTableRep).
    The HashTable class is mostly a wrapper to add proper type semantics to the
    use of its representation class.

    Hashing as always is O(1).

    HashTable uses the most popular hash table implementation which utilizes
    an array of pointers to bucket chains. This is organized as follows:

        <pre>
           +---+
           |   |   +-----+-------+
         0 | ----->| key | value |
           |   |   +-----+-------+
           +---+
           |   |   +-----+-------+   +-----+-------+   +-----+-------+
         1 | ----->| key | value |-->| key | value |-->| key | value |
           |   |   +-----+-------+   +-----+-------+   +-----+-------+
           +---+
             .
             .
             .
           +---+
           |   |   +-----+-------+   +-----+-------+
        N-1| ----->| key | value |-->| key | value |
           |   |   +-----+-------+   +-----+-------+
           +---+
        </pre>

    To locate an item a hash function is applied to the key to produce an
    integer value. Then the modulo of that integer is taken with N to select
    a chain (as shown above). Then the chain is searched for a bucket whose
    key value is the same as the target key.

    The number of chains default to DEFAULT_NUM_CHAINS but should be about
    one-third the number of expected entries (so that the average chain
    will be three long). Making the number of chains too large will waste
    space causing the hash table to be very sparse. But for optimal efficiency,
    one might set the number of chains to be the same as the expected number
    of entries.

    This implementation does have NOT an adaptive growth algorithm yet which
    would allow it to increase the number of chains periodically based on some
    statistic (e.g., when the number of entries is more than three times the
    number of chains; this would keep the average chain length below three).

    The following example shows how to instantiate a HashTable which associates
    String keys with Uint32 values.

        <pre>
        typedef HashTable&lt;String, Uint32&gt; HT;
        HT ht;
        </pre>

    Some of the template arguments are defaulted in the above example (the
    third and forth). The instantiation is explicitly qualified like this
    (which by the way has exactly the same effect).

        <pre>
        typedef HashTable&lt;String, Uint32,
            EqualFunc&lt;String&gt;, HashFunc&lt;String&gt;&gt; HT;
        </pre>

    The third and fourth arguments are described more in detail later.

    Then, entries may be inserted like this:

        <pre>
        ht.insert("Red", 111);
        ht.insert("Green", 222);
        ht.insert("Blue", 222);
        </pre>

    And entries may be looked up as follows:

        <pre>
        Uint32 value;
        ht.lookup("Red", value);
        </pre>

    And entries may be removed like this:

        <pre>
        h.remove("Red");
        </pre>

    Iteration is done like this:

        <pre>
        for (HT::Iterator i = ht.start(); i; i++)
        {
            // To access the key call i.key()!
            // To access the value call i.value()!
        }
        </pre>

    Note that only forward iteration is supported (no backwards iteration),
    AND that the hashtable MUST NOT be modified during the iteration!!!

    Equality of keys is determined using the EqualFunc class which is
    the default third argument of the template argument list. A new function
    object may be defined and passed to modify the behavior (for example, one
    might define equality of strings to ignore whitespace). Here is how to
    define and use a new equality function object:

        <pre>
        struct MyEqualFunc
        {
            static Boolean equal(const String& x, const String& y)
            {
                // do something here to test for equality!
            }
        };

        ...

        EqualFunc&lt;String, Uint32, MyEqualFunc&gt; ht;
        </pre>

    When the lookup(), insert(), and remove() methods are called, the
    MyEqualFunc::equal() method will be used to determine equality.

    Hash functions are provided for common types (as part of the default
    HashFunc class). For other types it is possible to define a custom function
    object as follows:

        <pre>
        struct MyHashFunc
        {
            static Uint32 hash(const String& x)
            {
                // Do some hashing here!
            }
        };

        ...

        EqualFunc&lt;String, Uint32, MyEqualFunc, MyHashFunc&gt; ht;
        </pre>

    As always, the hash function should provide a reasonably uniform
    distrubtion so that all of the entries don't get crowded into a few
    chains. Note that a hash function which returns zero, would force
    the pathalogical case in which all entries are placed in the first
    chain.
*/
template<class K, class V, class E , class H >
class HashTable
{
public:

    typedef _HashTableIterator<K, V, E> Iterator;

    /* By default, we create this many chains initially */
    enum { DEFAULT_NUM_CHAINS = 32 };

    /** Constructor.
        @param numChains number of chains to create.
    */
    HashTable(Uint32 numChains = DEFAULT_NUM_CHAINS) : _rep(numChains)
    {

    }

    /** Copy constructor. */
    HashTable(const HashTable<K,V,E,H>& x) : _rep(x._rep)
    {

    }

    /** Assignment operator. */
    HashTable<K,V,E,H>& operator=(const HashTable<K,V,E,H>& x)
    {
        if (this != &x)
            _rep = x._rep;
        return *this;
    }

    /** Returns the size of this hash table (the number of entries). */
    Uint32 size() const { return _rep.size(); }

    /** Clears the contents of this hash table. After this is called, the
        size() method returns zero.
    */
    void clear() { _rep.clear(); }

    /** Inserts new key-value pair into hash table.
        @param key key component.
        @param value value component.
        @return true on success; false if duplicate key.
    */
    Boolean insert(const K& key, const V& value)
    {
        return _rep.insert(
            H::hash(key), new _Bucket<K, V, E>(key, value), &key);
    }

    /** Checks to see if hash table contains an entry with the given key.
        @param key key to be searched for
        @return true if hash table contains an entry with the given key.
    */
    Boolean contains(const K& key) const
    {
        V value;
        return lookup(key, value);
    }

    /** Looks up the entry with the given key.
        @param key key of entry to be located.
        @param value output value.
        @return true if found; false otherwise.
    */
    Boolean lookup(const K& key, V& value) const;

    /** Looks up the entry with the given key and returns a pointer to the
        value.  Note that this pointer may become invalid when the HashTable
        is updated.
        @param key key of entry to be located.
        @param value Output pointer to the value.
        @return true if found; false otherwise.
    */
    Boolean lookupReference(const K& key, V*& value);

    /** Removes the entry with the given key.
        @param key key of entry to be removed.
        @return true on success; false otherwise.
    */
    Boolean remove(const K& key)
    {
        return _rep.remove(H::hash(key), &key);
    }

    /** Obtains an iterator for this object. */
    Iterator start() const
    {
        return Iterator(
            _rep.getChains(), _rep.getChains() + _rep.getNumChains());
    }

private:

    _HashTableRep _rep;
};

template<class K, class V, class E, class H>
inline Boolean HashTable<K, V, E, H>::lookup(const K& key, V& value) const
{
    _Bucket<K, V, E>* bucket
        = (_Bucket<K, V, E>*)_rep.lookup(H::hash(key), &key);

    if (bucket)
    {
        value = bucket->getValue();
        return true;
    }

    return false;
}

template<class K, class V, class E, class H>
inline Boolean HashTable<K, V, E, H>::lookupReference(const K& key, V*& value)
{
    _Bucket<K, V, E>* bucket =
        (_Bucket<K, V, E>*)_rep.lookup(H::hash(key), &key);

    if (bucket)
    {
        value = &bucket->getValue();
        return true;
    }

    return false;
}

PEGASUS_NAMESPACE_END

#endif /* Pegasus_HashTable_h */
