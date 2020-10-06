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

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/ArrayInternal.h>
#include "AssocClassCache.h"

PEGASUS_USING_STD;

PEGASUS_NAMESPACE_BEGIN

AssocClassCacheManager::AssocClassCacheManager()
{
}

AssocClassCacheManager::~AssocClassCacheManager()
{
    for (Uint32 i = _assocClassCacheList.size(); i > 0; i--)
    {
        delete _assocClassCacheList[i-1];
        _assocClassCacheList.remove(i-1);
    }
}

/**
    Retrieves a singleton instance of the class cache for the given namespace.
*/
AssocClassCache* AssocClassCacheManager::getAssocClassCache(
    const String& nameSpace)
{
    for (Uint32 i = 0; i < _assocClassCacheList.size(); i++)
    {
        if (nameSpace == _assocClassCacheList[i]->getNameSpace())
        {
            return _assocClassCacheList[i];
        }
    }

    // If we got here, no cache exists for the given namespace so far,
    // so we will create a new one.
    AssocClassCache* newCache = new AssocClassCache(nameSpace);
    _assocClassCacheList.append(newCache);

    return newCache;
}


/** Retrieve the list of entries for a from class through direct
    access via the from class name.
*/
Boolean AssocClassCache::getAssocClassEntry(
    const CIMName& fromClassName,
    Array<ClassAssociation>& entryList)
{
    return _assocTable.lookup(fromClassName.getString(), entryList);
}

Boolean AssocClassCache::getReferenceNames(
    const Array<CIMName>& classList,
    const Array<CIMName>& resultClassList,
    const String& role,
    Array<String>& referenceNames)
{
    Array<ClassAssociation> records;
    Boolean found = false;

    // For each of the target classes retrieve the list of matching
    // association classes from the cache.
    // The cache uses the from class name as an index and returns all
    // association class records having that from class.

    for (Uint16 idx = 0; idx < classList.size(); idx++)
    {
        String fromClassName = classList[idx].getString();
        if (getAssocClassEntry(fromClassName, records))
        {
            for (Uint16 rx = 0; rx < records.size(); rx++)
            {
                if ((role.size() == 0) ||
                    (records[rx].fromPropertyName == role))
                {
                    // Skip classes that do not appear in the result class list
                    if ((resultClassList.size() != 0) &&
                        (!Contains(resultClassList,
                             records[rx].assocClassName)))
                    {
                        continue;
                    }

                    // This class qualifies; add it to the list (skipping
                    // duplicates)
                    if (!Contains(referenceNames,
                            records[rx].assocClassName.getString()))
                    {
                        referenceNames.append(
                            records[rx].assocClassName.getString());
                    }
                    found = true;
                }
            }
        }
    }

    return found;
}

/** Add a new record to the association cache.
    If an entry for the given from class name already exists,
    the new entry is appended to the old entry. Otherwise a new entry
    is added to the cache.
*/
Boolean AssocClassCache::addRecord(
    const CIMName& fromClassName,
    const ClassAssociation& assocClassRecord)
{
    Array<ClassAssociation> oldAssocClassEntryList;

    if (_assocTable.lookup(fromClassName.getString(), oldAssocClassEntryList))
    {
        _assocTable.remove(fromClassName.getString());
    }

    oldAssocClassEntryList.append(assocClassRecord);

    return _assocTable.insert(
        fromClassName.getString(), oldAssocClassEntryList);
}

/** Remove an association record from the association cache specified by the
    given from class name and association name.
*/
Boolean AssocClassCache::removeRecord(
    const CIMName& fromClassName,
    const CIMName& assocClassName)
{
    Array<ClassAssociation> oldAssocClassEntryList;

    if (_assocTable.lookup(fromClassName.getString(), oldAssocClassEntryList))
    {
        for (Uint32 idx=0; idx < oldAssocClassEntryList.size(); idx++)
        {
            // Find the record for the association class and remove
            // it from the cache entry.
            if (oldAssocClassEntryList[idx].assocClassName == assocClassName)
            {
                _assocTable.remove(fromClassName.getString());
                if (oldAssocClassEntryList.size() > 1)
                {
                    oldAssocClassEntryList.remove(idx);
                    _assocTable.insert(
                        fromClassName.getString(), oldAssocClassEntryList);
                }
                return true;
            }
        }
    }

    return false;
}

/** Remove association records from the association cache specified by the
    association class name.
*/
Boolean AssocClassCache::removeAssocClassRecords(const CIMName& assocClassName)
{
    Array<CIMName> fromClassNames;

    for (AssocClassCacheHashTableType::Iterator i = _assocTable.start(); i; i++)
    {
        Array<ClassAssociation> assocClassEntryList = i.value();
        for (Uint32 j = 0; j < assocClassEntryList.size(); j++)
        {
            if (assocClassEntryList[j].assocClassName == assocClassName)
            {
                // Note: We cannot remove an entry from the HashTable while
                // iterating over it.
                fromClassNames.append(assocClassEntryList[j].fromClassName);
                break;
            }
        }
    }

    for (Uint32 i = 0; i < fromClassNames.size(); i++)
    {
        removeRecord(fromClassNames[i], assocClassName);
    }

    return fromClassNames.size();
}

/** Check if the cache is loaded with objects already.
*/
Boolean AssocClassCache::isActive()
{
    return _isInitialized;
}

void AssocClassCache::setActive(Boolean flag)
{
    _isInitialized = flag;
}


AssocClassCache::~AssocClassCache()
{
}

AssocClassCache::AssocClassCache(const String& nameSpace)
    : _nameSpace(nameSpace),
      _isInitialized(false),
      _assocTable(1000)
{
}

PEGASUS_NAMESPACE_END
