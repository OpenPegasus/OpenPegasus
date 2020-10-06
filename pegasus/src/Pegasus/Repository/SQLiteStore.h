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

#ifndef Pegasus_SQLiteStore_h
#define Pegasus_SQLiteStore_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/CommonUTF.h>
#include <Pegasus/Common/Mutex.h>
#include <Pegasus/Repository/PersistentStore.h>
#include <Pegasus/Repository/AssocClassCache.h>
#include <Pegasus/Repository/Linkage.h>

#include <sqlite3.h>

PEGASUS_NAMESPACE_BEGIN

/**
    The DbConnectionManager caches database handles for reuse.
    It has a fixed maximum cache size (currently 4) and uses an LRU algorithm
    to determine which entry to evict when a new handle is returned to the
    cache.  Note that multiple handles may be cached for a single namespace
    (database file).
*/
class DbConnectionManager
{
public:
    DbConnectionManager(const String& repositoryRoot)
        : _repositoryRoot(repositoryRoot)
    {
    }

    ~DbConnectionManager();

    /**
        Gets a database connection handle for the specified namespace.  It may
        return a handle removed from its cache or a newly opened one.  The
        caller is responsible for cleaning up the dynamic resources for the
        database connection or returning it to the cache.
    */
    sqlite3* getDbConnection(const CIMNamespaceName& nameSpace);

    /**
        Add a database connection handle to the cache.  If the cache is full,
        the least recently used handle is evicted.
    */
    void cacheDbConnection(
        const CIMNamespaceName& nameSpace,
        sqlite3* db);

    /**
        Converts a namespace name to the database file path which contains
        the namespace data.
    */
    String getDbPath(const CIMNamespaceName& nameSpace);

    /**
        Opens a database handle for the specified file path.  This handle is
        not cached.
    */
    static sqlite3* openDb(const char* fileName);

private:

    class CacheEntry
    {
    public:
        CacheEntry(const CIMNamespaceName& nameSpace_, sqlite3* db_)
            : nameSpace(nameSpace_),
              db(db_)
        {
        }

        // Note: The default copy constructor and assignment operator are used.
        // The caller is responsible for ensuring proper pointer management.

        CIMNamespaceName nameSpace;
        sqlite3* db;
    };

    Array<CacheEntry> _cache;
    Mutex _cacheLock;
    String _repositoryRoot;
};


class PEGASUS_REPOSITORY_LINKAGE SQLiteStore : public PersistentStore
{
public:

    static Boolean isExistingRepository(const String& repositoryRoot);

    SQLiteStore(
        const String& repositoryRoot,
        ObjectStreamer* streamer);

    ~SQLiteStore();

    Boolean storeCompleteClassDefinitions()
    {
        return false;
    }

    Array<NamespaceDefinition> enumerateNameSpaces();
    void createNameSpace(
        const CIMNamespaceName& nameSpace,
        Boolean shareable,
        Boolean updatesAllowed,
        const String& parentNameSpace,
        const String& remoteInfo);
    void modifyNameSpace(
        const CIMNamespaceName& nameSpace,
        Boolean shareable,
        Boolean updatesAllowed);
    void modifyNameSpaceName(
        const CIMNamespaceName& nameSpace,
        const CIMNamespaceName& newNameSpaceName);
    void deleteNameSpace(const CIMNamespaceName& nameSpace);
    Boolean isNameSpaceEmpty(const CIMNamespaceName& nameSpace);

    Array<CIMQualifierDecl> enumerateQualifiers(
        const CIMNamespaceName& nameSpace);
    /**
        Gets a qualifier declaration for a specified qualifier name in a
        specified namespace.  Returns an uninitialized object if the qualifier
        is not found.
    */
    CIMQualifierDecl getQualifier(
        const CIMNamespaceName& nameSpace,
        const CIMName& qualifierName);
    void setQualifier(
        const CIMNamespaceName& nameSpace,
        const CIMQualifierDecl& qualifierDecl);
    void deleteQualifier(
        const CIMNamespaceName& nameSpace,
        const CIMName& qualifierName);

    Array<Pair<String, String> > enumerateClassNames(
        const CIMNamespaceName& nameSpace);
    CIMClass getClass(
        const CIMNamespaceName& nameSpace,
        const CIMName& className,
        const CIMName& superClassName);
    void createClass(
        const CIMNamespaceName& nameSpace,
        const CIMClass& newClass,
        const Array<ClassAssociation>& classAssocEntries);
    void modifyClass(
        const CIMNamespaceName& nameSpace,
        const CIMClass& modifiedClass,
        const CIMName& oldSuperClassName,
        Boolean isAssociation,
        const Array<ClassAssociation>& classAssocEntries);
    void deleteClass(
        const CIMNamespaceName& nameSpace,
        const CIMName& className,
        const CIMName& superClassName,
        Boolean isAssociation,
        const Array<CIMNamespaceName>& dependentNameSpaceNames);
    Array<CIMObjectPath> enumerateInstanceNamesForClass(
        const CIMNamespaceName& nameSpace,
        const CIMName& className);
    Array<CIMInstance> enumerateInstancesForClass(
        const CIMNamespaceName& nameSpace,
        const CIMName& className);
    CIMInstance getInstance(
        const CIMNamespaceName& nameSpace,
        const CIMObjectPath& instanceName);
    void createInstance(
        const CIMNamespaceName& nameSpace,
        const CIMObjectPath& instanceName,
        const CIMInstance& cimInstance,
        const Array<InstanceAssociation>& instAssocEntries);
    void modifyInstance(
        const CIMNamespaceName& nameSpace,
        const CIMObjectPath& instanceName,
        const CIMInstance& cimInstance);
    void deleteInstance(
        const CIMNamespaceName& nameSpace,
        const CIMObjectPath& instanceName);
    Boolean instanceExists(
        const CIMNamespaceName& nameSpace,
        const CIMObjectPath& instanceName);

    void getClassAssociatorNames(
        const CIMNamespaceName& nameSpace,
        const Array<CIMName>& classList,
        const Array<CIMName>& assocClassList,
        const Array<CIMName>& resultClassList,
        const String& role,
        const String& resultRole,
        Array<String>& associatorNames);
    void getClassReferenceNames(
        const CIMNamespaceName& nameSpace,
        const Array<CIMName>& classList,
        const Array<CIMName>& resultClassList,
        const String& role,
        Array<String>& referenceNames);

    void getInstanceAssociatorNames(
        const CIMNamespaceName& nameSpace,
        const CIMObjectPath& instanceName,
        const Array<CIMName>& assocClassList,
        const Array<CIMName>& resultClassList,
        const String& role,
        const String& resultRole,
        Array<String>& associatorNames);
    void getInstanceReferenceNames(
        const CIMNamespaceName& nameSpace,
        const CIMObjectPath& instanceName,
        const Array<CIMName>& resultClassList,
        const String& role,
        Array<String>& referenceNames);

private:

    void _execDbStatement(
        sqlite3* db,
        const char* sqlStatement);

    void _initSchema(sqlite3* db);

    void _beginTransaction(sqlite3* db);
    void _commitTransaction(sqlite3* db);

    String _getNormalizedName(const CIMName& className)
    {
        String cn = className.getString();
        cn.toLower();
        return cn;
    }

    void _initAssocClassCache(
        const CIMNamespaceName& nameSpace,
        AssocClassCache* cache);
    void _addClassAssociationEntries(
        sqlite3* db,
        const CIMNamespaceName& nameSpace,
        const Array<ClassAssociation>& classAssocEntries);
    void _removeClassAssociationEntries(
        sqlite3* db,
        const CIMNamespaceName& nameSpace,
        const CIMName& assocClassName);

    void _addInstanceAssociationEntries(
        sqlite3* db,
        const CIMNamespaceName& nameSpace,
        const Array<InstanceAssociation>& instanceAssocEntries);
    void _removeInstanceAssociationEntries(
        sqlite3* db,
        const CIMNamespaceName& nameSpace,
        const CIMObjectPath& assocInstanceName);

    String _repositoryRoot;
    DbConnectionManager _dbcm;
    ObjectStreamer* _streamer;
    AssocClassCacheManager _assocClassCacheManager;
};

PEGASUS_NAMESPACE_END

#endif /* Pegasus_SQLiteStore_h */
