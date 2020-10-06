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

#ifndef Pegasus_AssocClassCache_h
#define Pegasus_AssocClassCache_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/String.h>
#include <Pegasus/Common/CIMName.h>
#include <Pegasus/Common/ArrayInternal.h>
#include <Pegasus/Common/HashTable.h>
#include <Pegasus/Repository/PersistentStoreData.h>
#include <Pegasus/Repository/Linkage.h>

PEGASUS_NAMESPACE_BEGIN

class AssocClassCache;

/** Maintains association class caches for all namespaces.
*/
class PEGASUS_REPOSITORY_LINKAGE AssocClassCacheManager
{
public:

    AssocClassCacheManager();
    ~AssocClassCacheManager();

    /** Retrieves the association class cache for the given namespace.
    */
    AssocClassCache* getAssocClassCache(const String& nameSpace);

private:

    Array<AssocClassCache*> _assocClassCacheList;
};

/** Maintains a cache for all association classes in a namespace.
*/
class PEGASUS_REPOSITORY_LINKAGE AssocClassCache
{
public:

    AssocClassCache(const String& nameSpace);
    ~AssocClassCache();

    const String& getNameSpace()
    {
        return _nameSpace;
    }

    /** Retrieve an entry for an association class through direct
        access via the from class name.
    */
    Boolean getAssocClassEntry(
        const CIMName& fromClassName,
        Array<ClassAssociation>& entryList);

    /** Gets the names of the association classes which reference the specified
        classes.  The results can be constrained by specifying a set of
        resulting association classes and the property name that refers to the
        specified class.
    */
    Boolean getReferenceNames(
        const Array<CIMName>& classList,
        const Array<CIMName>& resultClassList,
        const String& role,
        Array<String>& referenceNames);

    /** Add a new entry to the association cache.
    */
    Boolean addRecord(
        const CIMName& fromClassName,
        const ClassAssociation& assocClassEntry);

    /** Remove an association record from the association cache specified by
        the given from class name and association name.
    */
    Boolean removeRecord(
        const CIMName& fromClassName,
        const CIMName& assocClassName);

    /** Remove association records from the association cache specified by the
        association class name.
    */
    Boolean removeAssocClassRecords(
        const CIMName& assocClassName);

    /** Check if the cache is loaded with objects already.
    */
    Boolean isActive();
    void setActive(Boolean flag);

private:
    String _nameSpace;
    Boolean _isInitialized;

    typedef HashTable<String, Array<ClassAssociation>,
        EqualNoCaseFunc, HashLowerCaseFunc > AssocClassCacheHashTableType;

    AssocClassCacheHashTableType _assocTable;
};

PEGASUS_NAMESPACE_END

#endif /* Pegasus_AssocClassCache_h */
