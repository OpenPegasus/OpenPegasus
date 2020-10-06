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
#include <Pegasus/Common/CIMNameCast.h>
#include <cctype>
#include <cstdio>

#include <Pegasus/Common/InternalException.h>
#include <Pegasus/Common/DeclContext.h>
#include <Pegasus/Common/Resolver.h>
#include <Pegasus/Common/System.h>
#include <Pegasus/Common/Tracer.h>
#include <Pegasus/Common/MessageLoader.h>
#include <Pegasus/Common/FileSystem.h>
#include <Pegasus/Common/CIMNameCast.h>
#include "SQLiteStore.h"

// No build option to disable the association class cache is currently provided.
#define USE_ASSOC_CLASS_CACHE

PEGASUS_NAMESPACE_BEGIN

struct CloseSQLiteDb
{
    void operator()(sqlite3* db)
    {
        // This operator may be invoked automatically when an exception is
        // thrown, so it should not throw an exception itself.

        int rc = sqlite3_close(db);

        // It is expected that all prepared statements are finalized before
        // this operator is invoked.
        PEGASUS_ASSERT(rc == SQLITE_OK);
    }
};

struct FinalizeSQLiteStatement
{
    void operator()(sqlite3_stmt* stmt)
    {
        if (stmt)
        {
            sqlite3_finalize(stmt);
        }
    }
};


///////////////////////////////////////////////////////////////////////////////
//
// DbConnectionManager
//
///////////////////////////////////////////////////////////////////////////////

static const Uint32 MAX_CONNECTION_CACHE_SIZE = 4;

DbConnectionManager::~DbConnectionManager()
{
    for (Uint32 i = 0; i < _cache.size(); i++)
    {
        sqlite3_close(_cache[i].db);
    }
}

sqlite3* DbConnectionManager::getDbConnection(const CIMNamespaceName& nameSpace)
{
    AutoMutex lock(_cacheLock);

    // Get the database connection handle from the cache, if available

    for (Uint32 i = 0; i < _cache.size(); i++)
    {
        if (_cache[i].nameSpace == nameSpace)
        {
            sqlite3* db = _cache[i].db;
            _cache.remove(i);
            return db;
        }
    }

    // Create a database connection handle for this namespace

    return openDb(getDbPath(nameSpace).getCString());
}

void DbConnectionManager::cacheDbConnection(
    const CIMNamespaceName& nameSpace,
    sqlite3* db)
{
    AutoMutex lock(_cacheLock);

    // Make room in the cache

    if (_cache.size() == MAX_CONNECTION_CACHE_SIZE)
    {
        int rc = sqlite3_close(_cache[0].db);

        // It is expected that all prepared statements are finalized.
        PEGASUS_ASSERT(rc == SQLITE_OK);

        _cache.remove(0);
    }

    _cache.append(CacheEntry(nameSpace, db));
}

String DbConnectionManager::getDbPath(const CIMNamespaceName& nameSpace)
{
    String dbFileName = nameSpace.getString();
    dbFileName.toLower();

    for (Uint32 i = 0; i < dbFileName.size(); i++)
    {
        if (dbFileName[i] == '/')
        {
            dbFileName[i] = '#';
        }
    }

    return _repositoryRoot + "/" + escapeStringEncoder(dbFileName) + ".db";
}

sqlite3* DbConnectionManager::openDb(const char* fileName)
{
    sqlite3* db;

    int rc = sqlite3_open(fileName, &db);
    if (rc != SQLITE_OK)
    {
        sqlite3_close(db);
        throw Exception(MessageLoaderParms(
            "Repository.SQLiteStore.DB_OPEN_ERROR",
            "Failed to open SQLite repository database file \"$0\".",
            fileName));
    }

    return db;
}


///////////////////////////////////////////////////////////////////////////////
//
// DbConnection
//
///////////////////////////////////////////////////////////////////////////////

/**
    The DbConnection class provides scope-based management of database
    connection handles.  It provides a simple mechanism for closing a database
    connection and when an error occurs and caching it for reuse otherwise.
    The release() method must be called before the object is destructed to
    allow the connection to be reused.
*/
class DbConnection
{
public:
    DbConnection(DbConnectionManager& dbcm, const CIMNamespaceName& nameSpace)
        : _dbcm(dbcm),
          _nameSpace(nameSpace)
    {
        _db = _dbcm.getDbConnection(nameSpace);
    }

    ~DbConnection()
    {
        if (_db)
        {
            int rc = sqlite3_close(_db);

            // It is expected that all prepared statements are finalized.
            PEGASUS_ASSERT(rc == SQLITE_OK);
        }
    }

    sqlite3* get()
    {
        return _db;
    }

    void release()
    {
        _dbcm.cacheDbConnection(_nameSpace, _db);
        _db = 0;
    }

private:
    DbConnectionManager& _dbcm;
    CIMNamespaceName _nameSpace;
    sqlite3* _db;
};


///////////////////////////////////////////////////////////////////////////////
//
// SQLiteStore
//
///////////////////////////////////////////////////////////////////////////////

// Size optimization:  Keep the exception throwing logic in a function so it
// does not get replicated with each error check.
static void _throwSQLiteOperationException(sqlite3* db)
{
    throw Exception(MessageLoaderParms(
        "Repository.SQLiteStore.DB_OP_ERROR",
        "Repository SQLite database operation failed with error $0: $1",
        sqlite3_errcode(db),
        String((const Char16*)sqlite3_errmsg16(db))));
}

inline void CHECK_RC_OK(int rc, sqlite3* db)
{
    if (rc != SQLITE_OK)
    {
        _throwSQLiteOperationException(db);
    }
}

inline void CHECK_RC_DONE(int rc, sqlite3* db)
{
    if (rc != SQLITE_DONE)
    {
        _throwSQLiteOperationException(db);
    }
}

Boolean SQLiteStore::isExistingRepository(const String& repositoryRoot)
{
    return FileSystem::exists(repositoryRoot + "/root.db");
}

SQLiteStore::SQLiteStore(
    const String& repositoryRoot,
    ObjectStreamer* streamer)
    : _dbcm(repositoryRoot)
{
    PEG_METHOD_ENTER(TRC_REPOSITORY, "SQLiteStore::SQLiteStore");

    // Create the repository directory if it does not already exist.

    if (!FileSystem::isDirectory(repositoryRoot))
    {
        if (!FileSystem::makeDirectory(repositoryRoot))
        {
            PEG_METHOD_EXIT();
            throw CannotCreateDirectory(repositoryRoot);
        }
    }

    _repositoryRoot = repositoryRoot;
    _streamer = streamer;

    PEG_METHOD_EXIT();
}

SQLiteStore::~SQLiteStore()
{
}

void SQLiteStore::_execDbStatement(
    sqlite3* db,
    const char* sqlStatement)
{
    PEG_METHOD_ENTER(TRC_REPOSITORY, "SQLiteStore::_execDbStatement");

    PEG_TRACE((TRC_REPOSITORY, Tracer::LEVEL4,
        "Executing SQLite operation \"%s\"", sqlStatement));

    char* zErrMsg = 0;
    int rc = sqlite3_exec(db, sqlStatement, 0, 0, &zErrMsg);
    if (rc)
    {
        PEG_TRACE((TRC_REPOSITORY, Tracer::LEVEL1,
            "SQLite operation \"%s\" failed with error code %d: %s",
            sqlStatement,
            rc,
            zErrMsg));
        MessageLoaderParms parms(
            "Repository.SQLiteStore.DB_OP_ERROR",
            "Repository SQLite database operation failed with error $0: $1",
            rc,
            zErrMsg);
        sqlite3_free(zErrMsg);
        PEG_METHOD_EXIT();
        throw Exception(parms);
    }

    PEG_METHOD_EXIT();
}

void SQLiteStore::_initSchema(sqlite3* db)
{
    PEG_METHOD_ENTER(TRC_REPOSITORY, "SQLiteStore::_initSchema");

    // The NamespaceTable contains a single row describing the namespace
    // corresponding to the database file.
    _execDbStatement(
        db,
        "CREATE TABLE NamespaceTable("
            "nsname TEXT PRIMARY KEY NOT NULL,"
            "shareable INTEGER NOT NULL,"
            "updatesallowed INTEGER NOT NULL,"
            "parentnsname TEXT NOT NULL,"
            "remoteinfo TEXT NOT NULL);");

    _execDbStatement(
        db,
        "CREATE TABLE QualifierTable("
            "qualname TEXT NOT NULL,"
            "normqualname TEXT PRIMARY KEY NOT NULL,"
            "rep BLOB NOT NULL);");

    _execDbStatement(
        db,
        "CREATE TABLE ClassTable("
            "classname TEXT NOT NULL,"
            "normclassname TEXT PRIMARY KEY NOT NULL,"
            "superclassname TEXT NOT NULL,"
            "rep BLOB NOT NULL);");

    _execDbStatement(
        db,
        "CREATE TABLE ClassAssocTable("
            "assocclassname TEXT NOT NULL,"
            "normassocclassname TEXT NOT NULL,"
            "normfromclassname TEXT NOT NULL,"
            "normfrompropname TEXT NOT NULL,"
            "toclassname TEXT NOT NULL,"
            "normtoclassname TEXT NOT NULL,"
            "normtopropname TEXT NOT NULL,"
            "PRIMARY KEY("
                "normassocclassname, normfrompropname, normtopropname));");

    _execDbStatement(
        db,
        "CREATE TABLE InstanceTable("
            "normclassname TEXT NOT NULL,"
            "instname TEXT NOT NULL,"
            "norminstname TEXT PRIMARY KEY NOT NULL,"
            "rep BLOB NOT NULL);");

    _execDbStatement(
        db,
        "CREATE TABLE InstanceAssocTable("
            "associnstname TEXT NOT NULL,"
            "normassocinstname TEXT NOT NULL,"
            "normassocclassname TEXT NOT NULL,"
            "normfrominstname TEXT NOT NULL,"
            "normfrompropname TEXT NOT NULL,"
            "toinstname TEXT NOT NULL,"
            "normtoclassname TEXT NOT NULL,"
            "normtopropname TEXT NOT NULL,"
            "PRIMARY KEY("
                "normassocinstname, normfrompropname, normtopropname));");

    PEG_METHOD_EXIT();
}

void SQLiteStore::_beginTransaction(sqlite3* db)
{
    _execDbStatement(db, "BEGIN;");
}

void SQLiteStore::_commitTransaction(sqlite3* db)
{
    _execDbStatement(db, "COMMIT;");
}

Array<NamespaceDefinition> SQLiteStore::enumerateNameSpaces()
{
    PEG_METHOD_ENTER(TRC_REPOSITORY, "SQLiteStore::enumerateNameSpaces");

    Array<NamespaceDefinition> nameSpaces;

    Array<String> dbFileNames;
    if (!FileSystem::glob(_repositoryRoot, "*.db", dbFileNames))
    {
        throw CannotOpenDirectory(_repositoryRoot);
    }

    const char* sqlStatement = "SELECT nsname, shareable, updatesallowed, "
        "parentnsname, remoteinfo FROM NamespaceTable;";

    for (Uint32 i = 0; i < dbFileNames.size(); i++)
    {
        AutoPtr<sqlite3, CloseSQLiteDb> db(DbConnectionManager::openDb(
            (_repositoryRoot + "/" + dbFileNames[i]).getCString()));

        sqlite3_stmt* stmt = 0;
        CHECK_RC_OK(
            sqlite3_prepare_v2(db.get(), sqlStatement, -1, &stmt, 0),
            db.get());
        AutoPtr<sqlite3_stmt, FinalizeSQLiteStatement> stmtDestroyer(stmt);

        int rc = sqlite3_step(stmt);

        if (rc == SQLITE_ROW)
        {
            NamespaceDefinition nsdef(String(
                (const Char16*) sqlite3_column_text16(stmt, 0),
                (Uint32) sqlite3_column_bytes16(stmt, 0) / 2));
            nsdef.shareable = (Boolean) sqlite3_column_int(stmt, 1);
            nsdef.updatesAllowed = (Boolean) sqlite3_column_int(stmt, 2);
            String parentNameSpace = String(
                (const Char16*) sqlite3_column_text16(stmt, 3),
                (Uint32) sqlite3_column_bytes16(stmt, 3) / 2);
            if (parentNameSpace.size())
            {
                nsdef.parentNameSpace = parentNameSpace;
            }
            nsdef.remoteInfo = String(
                (const Char16*) sqlite3_column_text16(stmt, 4),
                (Uint32) sqlite3_column_bytes16(stmt, 4) / 2);
            nameSpaces.append(nsdef);
        }
        else
        {
            // There should always be one entry in the table.  This namespace
            // must be corrupt.  Skip it.
            PEG_TRACE((TRC_REPOSITORY, Tracer::LEVEL1,
                "Corrupt namespace database %s",
                (const char*) dbFileNames[i].getCString()));
        }
    }

    PEG_METHOD_EXIT();
    return nameSpaces;
}

void SQLiteStore::createNameSpace(
    const CIMNamespaceName& nameSpace,
    Boolean shareable,
    Boolean updatesAllowed,
    const String& parentNameSpace,
    const String& remoteInfo)
{
    PEG_METHOD_ENTER(TRC_REPOSITORY, "SQLiteStore::createNameSpace");

    String dbPath = _dbcm.getDbPath(nameSpace);

    if (FileSystem::exists(dbPath))
    {
        throw Exception(MessageLoaderParms(
            "Repository.SQLiteStore.DB_FILE_ALREADY_EXISTS",
            "Cannot create namespace $0: SQLite database file \"$1\" already "
                "exists in the repository."));
    }

    try
    {
        AutoPtr<sqlite3, CloseSQLiteDb> db(
            DbConnectionManager::openDb(dbPath.getCString()));

        _initSchema(db.get());

        const char* sqlStatement =
            "INSERT INTO NamespaceTable VALUES(?,?,?,?,?);";

        sqlite3_stmt* stmt = 0;
        CHECK_RC_OK(
            sqlite3_prepare_v2(db.get(), sqlStatement, -1, &stmt, 0),
            db.get());
        AutoPtr<sqlite3_stmt, FinalizeSQLiteStatement> stmtDestroyer(stmt);

        String ns = nameSpace.getString();

        CHECK_RC_OK(
            sqlite3_bind_text16(
                stmt, 1, ns.getChar16Data(), ns.size() * 2, SQLITE_STATIC),
            db.get());
        CHECK_RC_OK(
            sqlite3_bind_int(stmt, 2, shareable? 1 : 0), db.get());
        CHECK_RC_OK(
            sqlite3_bind_int(stmt, 3, updatesAllowed? 1 : 0), db.get());
        CHECK_RC_OK(
            sqlite3_bind_text16(
                stmt,
                4,
                parentNameSpace.getChar16Data(),
                parentNameSpace.size() * 2,
                SQLITE_STATIC),
            db.get());

       String remoteId;
#ifdef PEGASUS_ENABLE_REMOTE_CMPI
       if (remoteInfo.size())
       {
           remoteId.append("r10");
           remoteId.append(remoteInfo);
       }
#endif
        CHECK_RC_OK(
            sqlite3_bind_text16(
                stmt,
                5,
                remoteId.getChar16Data(),
                remoteId.size() * 2,
                SQLITE_STATIC),
            db.get());

        CHECK_RC_DONE(sqlite3_step(stmt), db.get());
    }
    catch (...)
    {
        // Do not leave a partially created database file
        FileSystem::removeFile(dbPath);
        PEG_METHOD_EXIT();
        throw;
    }

    PEG_METHOD_EXIT();
}

void SQLiteStore::modifyNameSpace(
    const CIMNamespaceName& nameSpace,
    Boolean shareable,
    Boolean updatesAllowed)
{
    PEG_METHOD_ENTER(TRC_REPOSITORY, "SQLiteStore::modifyNameSpace");

    DbConnection db(_dbcm, nameSpace);

    // The NamespaceTable contains only a single row, so no WHERE clause is
    // needed.
    const char* sqlStatement = "UPDATE NamespaceTable SET shareable=?, "
        "updatesallowed=?;";

    sqlite3_stmt* stmt = 0;
    CHECK_RC_OK(
        sqlite3_prepare_v2(db.get(), sqlStatement, -1, &stmt, 0),
        db.get());
    AutoPtr<sqlite3_stmt, FinalizeSQLiteStatement> stmtDestroyer(stmt);

    CHECK_RC_OK(sqlite3_bind_int(stmt, 1, shareable? 1 : 0), db.get());
    CHECK_RC_OK(sqlite3_bind_int(stmt, 2, updatesAllowed? 1 : 0), db.get());

    CHECK_RC_DONE(sqlite3_step(stmt), db.get());

    stmtDestroyer.reset();
    db.release();

    PEG_METHOD_EXIT();
}

void SQLiteStore::modifyNameSpaceName(
    const CIMNamespaceName& nameSpace,
    const CIMNamespaceName& newNameSpaceName)
{
    PEG_METHOD_ENTER(TRC_REPOSITORY, "SQLiteStore::modifyNameSpaceName");
    
    String dbPath = _dbcm.getDbPath(nameSpace);
    String dbPathnew = _dbcm.getDbPath(newNameSpaceName);
   
    DbConnection db(_dbcm, nameSpace);

    String ns = newNameSpaceName.getString();
    sqlite3_stmt* stmt = 0;

    // The NamespaceTable contains only a single row, so no WHERE clause is
    // needed.
    const char* sqlStatement = "UPDATE NamespaceTable SET nsname=?;";

    CHECK_RC_OK(
        sqlite3_prepare_v2(db.get(), sqlStatement, -1, &stmt, 0),
        db.get());
    AutoPtr<sqlite3_stmt, FinalizeSQLiteStatement> stmtDestroyer(stmt);

    CHECK_RC_OK(
            sqlite3_bind_text16(
                stmt, 1, ns.getChar16Data(), ns.size() * 2, SQLITE_STATIC),
            db.get());
    CHECK_RC_DONE(sqlite3_step(stmt), db.get());

    stmtDestroyer.reset();
    db.release();
    FileSystem::renameFile(dbPath, dbPathnew);

    PEG_METHOD_EXIT();
}
void SQLiteStore::deleteNameSpace(const CIMNamespaceName& nameSpace)
{
    PEG_METHOD_ENTER(TRC_REPOSITORY, "SQLiteStore::deleteNameSpace");

    String dbPath = _dbcm.getDbPath(nameSpace);

    if (!FileSystem::removeFile(dbPath))
    {
        throw CannotRemoveFile(dbPath);
    }

    PEG_METHOD_EXIT();
}

Boolean SQLiteStore::isNameSpaceEmpty(const CIMNamespaceName& nameSpace)
{
    PEG_METHOD_ENTER(TRC_REPOSITORY, "SQLiteStore::isNameSpaceEmpty");

    Boolean isEmpty =
        ((enumerateQualifiers(nameSpace).size() == 0) &&
         (enumerateClassNames(nameSpace).size() == 0));

    PEG_METHOD_EXIT();
    return isEmpty;
}

Array<CIMQualifierDecl> SQLiteStore::enumerateQualifiers(
    const CIMNamespaceName& nameSpace)
{
    PEG_METHOD_ENTER(TRC_REPOSITORY, "SQLiteStore::enumerateQualifiers");

    Array<CIMQualifierDecl> qualifiers;
    DbConnection db(_dbcm, nameSpace);

    const char* sqlStatement = "SELECT rep FROM QualifierTable;";

    sqlite3_stmt* stmt = 0;
    CHECK_RC_OK(
        sqlite3_prepare_v2(db.get(), sqlStatement, -1, &stmt, 0),
        db.get());
    AutoPtr<sqlite3_stmt, FinalizeSQLiteStatement> stmtDestroyer(stmt);

    int rc;
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW)
    {
        Buffer data(
            (const char*)sqlite3_column_blob(stmt, 0),
            (Uint32)sqlite3_column_bytes(stmt, 0));
        CIMQualifierDecl qualifier;
        _streamer->decode(data, 0, qualifier);
        qualifiers.append(qualifier);
    }

    CHECK_RC_DONE(rc, db.get());

    stmtDestroyer.reset();
    db.release();

    PEG_METHOD_EXIT();
    return qualifiers;
}

CIMQualifierDecl SQLiteStore::getQualifier(
    const CIMNamespaceName& nameSpace,
    const CIMName& qualifierName)
{
    PEG_METHOD_ENTER(TRC_REPOSITORY, "SQLiteStore::getQualifier");

    CIMQualifierDecl qualifier;

    DbConnection db(_dbcm, nameSpace);

    const char* sqlStatement = "SELECT rep FROM QualifierTable "
        "WHERE normqualname=?;";

    sqlite3_stmt* stmt = 0;
    CHECK_RC_OK(
        sqlite3_prepare_v2(db.get(), sqlStatement, -1, &stmt, 0),
        db.get());
    AutoPtr<sqlite3_stmt, FinalizeSQLiteStatement> stmtDestroyer(stmt);

    String qualname = _getNormalizedName(qualifierName);

    CHECK_RC_OK(
        sqlite3_bind_text16(
            stmt,
            1,
            qualname.getChar16Data(),
            qualname.size() * 2,
            SQLITE_STATIC),
        db.get());

    int rc;
    rc = sqlite3_step(stmt);

    if (rc == SQLITE_ROW)
    {
        Buffer data(
            (const char*)sqlite3_column_blob(stmt, 0),
            (Uint32)sqlite3_column_bytes(stmt, 0));
        _streamer->decode(data, 0, qualifier);
    }
    else
    {
        // SQLITE_DONE here means the qualifier is not found
        CHECK_RC_DONE(rc, db.get());
    }

    stmtDestroyer.reset();
    db.release();

    PEG_METHOD_EXIT();
    return qualifier;
}

void SQLiteStore::setQualifier(
    const CIMNamespaceName& nameSpace,
    const CIMQualifierDecl& qualifierDecl)
{
    PEG_METHOD_ENTER(TRC_REPOSITORY, "SQLiteStore::setQualifier");

    // Note: This is a relatively inefficient way to determine the existence
    // of a qualifier, but setting a qualifier is expected to be an infrequent
    // operation.
    if (!getQualifier(nameSpace, qualifierDecl.getName()).isUninitialized())
    {
        PEG_METHOD_EXIT();
        throw PEGASUS_CIM_EXCEPTION(
            CIM_ERR_NOT_SUPPORTED, qualifierDecl.getName().getString());
    }

    DbConnection db(_dbcm, nameSpace);

    // Qualifier does not exist yet; create it
    const char* sqlStatement =
        "INSERT INTO QualifierTable VALUES(?,?,?);";

    sqlite3_stmt* stmt = 0;
    CHECK_RC_OK(
        sqlite3_prepare_v2(db.get(), sqlStatement, -1, &stmt, 0),
        db.get());
    AutoPtr<sqlite3_stmt, FinalizeSQLiteStatement> stmtDestroyer(stmt);

    String qualname = qualifierDecl.getName().getString();
    String normqualname = _getNormalizedName(qualifierDecl.getName());
    Buffer data;
    _streamer->encode(data, qualifierDecl);

    CHECK_RC_OK(
        sqlite3_bind_text16(
            stmt,
            1,
            qualname.getChar16Data(),
            qualname.size() * 2,
            SQLITE_STATIC),
        db.get());
    CHECK_RC_OK(
        sqlite3_bind_text16(
            stmt,
            2,
            normqualname.getChar16Data(),
            normqualname.size() * 2,
            SQLITE_STATIC),
        db.get());
    CHECK_RC_OK(
        sqlite3_bind_blob(
            stmt, 3, data.getData(), data.size(), SQLITE_STATIC),
        db.get());

    CHECK_RC_DONE(sqlite3_step(stmt), db.get());

    db.release();

    PEG_METHOD_EXIT();
}

void SQLiteStore::deleteQualifier(
    const CIMNamespaceName& nameSpace,
    const CIMName& qualifierName)
{
    PEG_METHOD_ENTER(TRC_REPOSITORY, "SQLiteStore::deleteQualifier");

    // Note: This is a relatively inefficient way to verify the existence of a
    // qualifier, but deleting a qualifier is expected to be an infrequent
    // operation.
    if (getQualifier(nameSpace, qualifierName).isUninitialized())
    {
        PEG_METHOD_EXIT();
        throw PEGASUS_CIM_EXCEPTION(
            CIM_ERR_NOT_FOUND, qualifierName.getString());
    }

    DbConnection db(_dbcm, nameSpace);

    const char* sqlStatement = "DELETE FROM QualifierTable "
        "WHERE normqualname=?;";

    sqlite3_stmt* stmt = 0;
    CHECK_RC_OK(
        sqlite3_prepare_v2(db.get(), sqlStatement, -1, &stmt, 0),
        db.get());
    AutoPtr<sqlite3_stmt, FinalizeSQLiteStatement> stmtDestroyer(stmt);

    String qualname = _getNormalizedName(qualifierName);

    CHECK_RC_OK(
        sqlite3_bind_text16(
            stmt,
            1,
            qualname.getChar16Data(),
            qualname.size() * 2,
            SQLITE_STATIC),
        db.get());

    CHECK_RC_DONE(sqlite3_step(stmt), db.get());

    stmtDestroyer.reset();
    db.release();

    PEG_METHOD_EXIT();
}

Array<Pair<String, String> > SQLiteStore::enumerateClassNames(
    const CIMNamespaceName& nameSpace)
{
    PEG_METHOD_ENTER(TRC_REPOSITORY, "SQLiteStore::enumerateClassNames");

    Array<Pair<String, String> > classList;
    DbConnection db(_dbcm, nameSpace);

    const char* sqlStatement = "SELECT classname, superclassname "
        "FROM ClassTable;";

    sqlite3_stmt* stmt = 0;
    CHECK_RC_OK(
        sqlite3_prepare_v2(db.get(), sqlStatement, -1, &stmt, 0),
        db.get());
    AutoPtr<sqlite3_stmt, FinalizeSQLiteStatement> stmtDestroyer(stmt);

    int rc;
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW)
    {
        classList.append(Pair<String, String>(
            String(
                (const Char16*)sqlite3_column_text16(stmt, 0),
                (Uint32)sqlite3_column_bytes16(stmt, 0) / 2),
            String(
                (const Char16*)sqlite3_column_text16(stmt, 1),
                (Uint32)sqlite3_column_bytes16(stmt, 1) / 2)));
    }

    CHECK_RC_DONE(rc, db.get());

    stmtDestroyer.reset();
    db.release();

    PEG_METHOD_EXIT();
    return classList;
}

CIMClass SQLiteStore::getClass(
    const CIMNamespaceName& nameSpace,
    const CIMName& className,
    const CIMName& superClassName)
{
    PEG_METHOD_ENTER(TRC_REPOSITORY, "SQLiteStore::getClass");

    CIMClass cimClass;
    DbConnection db(_dbcm, nameSpace);

    const char* sqlStatement = "SELECT rep FROM ClassTable "
        "WHERE normclassname=?;";

    sqlite3_stmt* stmt = 0;
    CHECK_RC_OK(
        sqlite3_prepare_v2(db.get(), sqlStatement, -1, &stmt, 0),
        db.get());
    AutoPtr<sqlite3_stmt, FinalizeSQLiteStatement> stmtDestroyer(stmt);

    String classname = _getNormalizedName(className);

    CHECK_RC_OK(
        sqlite3_bind_text16(
            stmt,
            1,
            classname.getChar16Data(),
            classname.size() * 2,
            SQLITE_STATIC),
        db.get());

    int rc = sqlite3_step(stmt);

    if (rc != SQLITE_ROW)
    {
        CHECK_RC_DONE(rc, db.get());

        stmtDestroyer.reset();
        db.release();

        PEG_METHOD_EXIT();
        throw PEGASUS_CIM_EXCEPTION(CIM_ERR_NOT_FOUND, className.getString());
    }

    Buffer data(
        (const char*)sqlite3_column_blob(stmt, 0),
        (Uint32)sqlite3_column_bytes(stmt, 0));
    _streamer->decode(data, 0, cimClass);

    PEGASUS_ASSERT(sqlite3_step(stmt) == SQLITE_DONE);

    stmtDestroyer.reset();
    db.release();

    PEG_METHOD_EXIT();
    return cimClass;
}

void SQLiteStore::createClass(
    const CIMNamespaceName& nameSpace,
    const CIMClass& newClass,
    const Array<ClassAssociation>& classAssocEntries)
{
    PEG_METHOD_ENTER(TRC_REPOSITORY, "SQLiteStore::createClass");

    DbConnection db(_dbcm, nameSpace);

    _beginTransaction(db.get());

    const char* sqlStatement = "INSERT INTO ClassTable VALUES(?,?,?,?);";

    sqlite3_stmt* stmt = 0;
    CHECK_RC_OK(
        sqlite3_prepare_v2(db.get(), sqlStatement, -1, &stmt, 0),
        db.get());
    AutoPtr<sqlite3_stmt, FinalizeSQLiteStatement> stmtDestroyer(stmt);

    String classname = newClass.getClassName().getString();
    String normclassname = _getNormalizedName(newClass.getClassName());
    String superclassname = newClass.getSuperClassName().getString();
    Buffer data;
    _streamer->encode(data, newClass);

    CHECK_RC_OK(
        sqlite3_bind_text16(
            stmt,
            1,
            classname.getChar16Data(),
            classname.size() * 2,
            SQLITE_STATIC),
        db.get());
    CHECK_RC_OK(
        sqlite3_bind_text16(
            stmt,
            2,
            normclassname.getChar16Data(),
            normclassname.size() * 2,
            SQLITE_STATIC),
        db.get());
    CHECK_RC_OK(
        sqlite3_bind_text16(
            stmt,
            3,
            superclassname.getChar16Data(),
            superclassname.size() * 2,
            SQLITE_STATIC),
        db.get());
    CHECK_RC_OK(
        sqlite3_bind_blob(stmt, 4, data.getData(), data.size(), SQLITE_STATIC),
        db.get());

    CHECK_RC_DONE(sqlite3_step(stmt), db.get());

    // Create the class association entries
    if (classAssocEntries.size())
    {
        _addClassAssociationEntries(db.get(), nameSpace, classAssocEntries);
    }

    _commitTransaction(db.get());

    stmtDestroyer.reset();
    db.release();

    PEG_METHOD_EXIT();
}

void SQLiteStore::modifyClass(
    const CIMNamespaceName& nameSpace,
    const CIMClass& modifiedClass,
    const CIMName& oldSuperClassName,
    Boolean isAssociation,
    const Array<ClassAssociation>& classAssocEntries)
{
    PEG_METHOD_ENTER(TRC_REPOSITORY, "SQLiteStore::modifyClass");

    DbConnection db(_dbcm, nameSpace);

    _beginTransaction(db.get());

    const char* sqlStatement = "UPDATE ClassTable SET rep=? "
        "WHERE normclassname=?;";

    sqlite3_stmt* stmt = 0;
    CHECK_RC_OK(
        sqlite3_prepare_v2(db.get(), sqlStatement, -1, &stmt, 0),
        db.get());
    AutoPtr<sqlite3_stmt, FinalizeSQLiteStatement> stmtDestroyer(stmt);

    Buffer data;
    _streamer->encode(data, modifiedClass);
    String normclassname = _getNormalizedName(modifiedClass.getClassName());

    CHECK_RC_OK(
        sqlite3_bind_blob(stmt, 1, data.getData(), data.size(), SQLITE_STATIC),
        db.get());
    CHECK_RC_OK(
        sqlite3_bind_text16(
            stmt,
            2,
            normclassname.getChar16Data(),
            normclassname.size() * 2,
            SQLITE_STATIC),
        db.get());

    CHECK_RC_DONE(sqlite3_step(stmt), db.get());

    //
    // Update the association entries
    //

    if (isAssociation)
    {
        _removeClassAssociationEntries(
            db.get(), nameSpace, modifiedClass.getClassName());
        if (classAssocEntries.size())
        {
            _addClassAssociationEntries(db.get(), nameSpace, classAssocEntries);
        }
    }

    _commitTransaction(db.get());

    stmtDestroyer.reset();
    db.release();

    PEG_METHOD_EXIT();
}

void SQLiteStore::deleteClass(
    const CIMNamespaceName& nameSpace,
    const CIMName& className,
    const CIMName& superClassName,
    Boolean isAssociation,
    const Array<CIMNamespaceName>& dependentNameSpaceNames)
{
    PEG_METHOD_ENTER(TRC_REPOSITORY, "SQLiteStore::deleteClass");

    // Note: This is a relatively inefficient way to verify the existence of a
    // class, but deleting a class is expected to be an infrequent operation.
    CIMClass dummy = getClass(nameSpace, className, superClassName);

    DbConnection db(_dbcm, nameSpace);

    _beginTransaction(db.get());

    const char* sqlStatement = "DELETE FROM ClassTable "
        "WHERE normclassname=?;";

    sqlite3_stmt* stmt = 0;
    CHECK_RC_OK(
        sqlite3_prepare_v2(db.get(), sqlStatement, -1, &stmt, 0),
        db.get());
    AutoPtr<sqlite3_stmt, FinalizeSQLiteStatement> stmtDestroyer(stmt);

    String normclassname = _getNormalizedName(className);

    CHECK_RC_OK(
        sqlite3_bind_text16(
            stmt,
            1,
            normclassname.getChar16Data(),
            normclassname.size() * 2,
            SQLITE_STATIC),
        db.get());

    CHECK_RC_DONE(sqlite3_step(stmt), db.get());

    //
    // Remove association entries
    //

    if (isAssociation)
    {
        _removeClassAssociationEntries(db.get(), nameSpace, className);
    }

    _commitTransaction(db.get());

    stmtDestroyer.reset();
    db.release();

    PEG_METHOD_EXIT();
}

Array<CIMObjectPath> SQLiteStore::enumerateInstanceNamesForClass(
    const CIMNamespaceName& nameSpace,
    const CIMName& className)
{
    PEG_METHOD_ENTER(TRC_REPOSITORY,
        "SQLiteStore::enumerateInstanceNamesForClass");

    Array<CIMObjectPath> instanceNames;
    DbConnection db(_dbcm, nameSpace);

    const char* sqlStatement = "SELECT instname FROM InstanceTable "
        "WHERE normclassname=?;";

    sqlite3_stmt* stmt = 0;
    CHECK_RC_OK(
        sqlite3_prepare_v2(db.get(), sqlStatement, -1, &stmt, 0),
        db.get());
    AutoPtr<sqlite3_stmt, FinalizeSQLiteStatement> stmtDestroyer(stmt);

    String classname = _getNormalizedName(className);

    CHECK_RC_OK(
        sqlite3_bind_text16(
            stmt,
            1,
            classname.getChar16Data(),
            classname.size() * 2,
            SQLITE_STATIC),
        db.get());

    int rc;
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW)
    {
        instanceNames.append(CIMObjectPath(String(
            (const Char16*)sqlite3_column_text16(stmt, 0),
            (Uint32)sqlite3_column_bytes16(stmt, 0) / 2)));
    }

    CHECK_RC_DONE(rc, db.get());

    stmtDestroyer.reset();
    db.release();

    PEG_METHOD_EXIT();
    return instanceNames;
}

Array<CIMInstance> SQLiteStore::enumerateInstancesForClass(
    const CIMNamespaceName& nameSpace,
    const CIMName& className)
{
    PEG_METHOD_ENTER(TRC_REPOSITORY,
        "SQLiteStore::enumerateInstancesForClass");

    Array<CIMInstance> cimInstances;

    DbConnection db(_dbcm, nameSpace);

    const char* sqlStatement = "SELECT instname, rep FROM InstanceTable "
        "WHERE normclassname=?;";

    sqlite3_stmt* stmt = 0;
    CHECK_RC_OK(
        sqlite3_prepare_v2(db.get(), sqlStatement, -1, &stmt, 0),
        db.get());
    AutoPtr<sqlite3_stmt, FinalizeSQLiteStatement> stmtDestroyer(stmt);

    String classname = _getNormalizedName(className);

    CHECK_RC_OK(
        sqlite3_bind_text16(
            stmt,
            1,
            classname.getChar16Data(),
            classname.size() * 2,
            SQLITE_STATIC),
        db.get());

    int rc;
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW)
    {
        CIMObjectPath instanceName = CIMObjectPath(String(
            (const Char16*)sqlite3_column_text16(stmt, 0),
            (Uint32)sqlite3_column_bytes16(stmt, 0) / 2));
        Buffer data(
            (const char*)sqlite3_column_blob(stmt, 1),
            (Uint32)sqlite3_column_bytes(stmt, 1));

        CIMInstance cimInstance;
        _streamer->decode(data, 0, cimInstance);
        cimInstance.setPath(instanceName);
        cimInstances.append(cimInstance);
    }

    CHECK_RC_DONE(rc, db.get());

    stmtDestroyer.reset();
    db.release();

    PEG_METHOD_EXIT();
    return cimInstances;
}

CIMInstance SQLiteStore::getInstance(
    const CIMNamespaceName& nameSpace,
    const CIMObjectPath& instanceName)
{
    PEG_METHOD_ENTER(TRC_REPOSITORY, "SQLiteStore::getInstance");

    CIMInstance cimInstance;

    DbConnection db(_dbcm, nameSpace);

    const char* sqlStatement = "SELECT instname, rep FROM InstanceTable "
        "WHERE norminstname=?;";

    sqlite3_stmt* stmt = 0;
    CHECK_RC_OK(
        sqlite3_prepare_v2(db.get(), sqlStatement, -1, &stmt, 0),
        db.get());
    AutoPtr<sqlite3_stmt, FinalizeSQLiteStatement> stmtDestroyer(stmt);

    String norminstname = instanceName._toStringCanonical();

    CHECK_RC_OK(
        sqlite3_bind_text16(
            stmt,
            1,
            norminstname.getChar16Data(),
            norminstname.size() * 2,
            SQLITE_STATIC),
        db.get());

    int rc = sqlite3_step(stmt);

    if (rc != SQLITE_ROW)
    {
        CHECK_RC_DONE(rc, db.get());

        stmtDestroyer.reset();
        db.release();

        PEG_METHOD_EXIT();
        throw PEGASUS_CIM_EXCEPTION(CIM_ERR_NOT_FOUND, instanceName.toString());
    }

    Buffer data(
        (const char*)sqlite3_column_blob(stmt, 1),
        (Uint32)sqlite3_column_bytes(stmt, 1));
    _streamer->decode(data, 0, cimInstance);

    PEGASUS_ASSERT(sqlite3_step(stmt) == SQLITE_DONE);

    stmtDestroyer.reset();
    db.release();

    PEG_METHOD_EXIT();
    return cimInstance;
}

void SQLiteStore::createInstance(
    const CIMNamespaceName& nameSpace,
    const CIMObjectPath& instanceName,
    const CIMInstance& cimInstance,
    const Array<InstanceAssociation>& instAssocEntries)
{
    PEG_METHOD_ENTER(TRC_REPOSITORY, "SQLiteStore::createInstance");

    DbConnection db(_dbcm, nameSpace);

    _beginTransaction(db.get());

    const char* sqlStatement = "INSERT INTO InstanceTable VALUES(?,?,?,?);";

    sqlite3_stmt* stmt = 0;
    CHECK_RC_OK(
        sqlite3_prepare_v2(db.get(), sqlStatement, -1, &stmt, 0),
        db.get());
    AutoPtr<sqlite3_stmt, FinalizeSQLiteStatement> stmtDestroyer(stmt);

    String classname = _getNormalizedName(cimInstance.getClassName());
    String instname = instanceName.toString();
    String norminstname = instanceName._toStringCanonical();
    Buffer data;
    _streamer->encode(data, cimInstance);

    CHECK_RC_OK(
        sqlite3_bind_text16(
            stmt,
            1,
            classname.getChar16Data(),
            classname.size() * 2,
            SQLITE_STATIC),
        db.get());
    CHECK_RC_OK(
        sqlite3_bind_text16(
            stmt,
            2,
            instname.getChar16Data(),
            instname.size() * 2,
            SQLITE_STATIC),
        db.get());
    CHECK_RC_OK(
        sqlite3_bind_text16(
            stmt,
            3,
            norminstname.getChar16Data(),
            norminstname.size() * 2,
            SQLITE_STATIC),
        db.get());
    CHECK_RC_OK(
        sqlite3_bind_blob(stmt, 4, data.getData(), data.size(), SQLITE_STATIC),
        db.get());

    CHECK_RC_DONE(sqlite3_step(stmt), db.get());

    //
    // Create association entries if an association instance.
    //

    if (instAssocEntries.size())
    {
        _addInstanceAssociationEntries(db.get(), nameSpace, instAssocEntries);
    }

    _commitTransaction(db.get());

    stmtDestroyer.reset();
    db.release();

    PEG_METHOD_EXIT();
}

void SQLiteStore::modifyInstance(
    const CIMNamespaceName& nameSpace,
    const CIMObjectPath& instanceName,
    const CIMInstance& cimInstance)
{
    PEG_METHOD_ENTER(TRC_REPOSITORY, "SQLiteStore::modifyInstance");

    if (!instanceExists(nameSpace, instanceName))
    {
        PEG_METHOD_EXIT();
        throw PEGASUS_CIM_EXCEPTION(CIM_ERR_NOT_FOUND, instanceName.toString());
    }

    DbConnection db(_dbcm, nameSpace);

    const char* sqlStatement = "UPDATE InstanceTable SET rep=? "
        "WHERE norminstname=?;";

    sqlite3_stmt* stmt = 0;
    CHECK_RC_OK(
        sqlite3_prepare_v2(db.get(), sqlStatement, -1, &stmt, 0),
        db.get());
    AutoPtr<sqlite3_stmt, FinalizeSQLiteStatement> stmtDestroyer(stmt);

    Buffer data;
    _streamer->encode(data, cimInstance);
    String norminstname = instanceName._toStringCanonical();

    CHECK_RC_OK(
        sqlite3_bind_blob(stmt, 1, data.getData(), data.size(), SQLITE_STATIC),
        db.get());
    CHECK_RC_OK(
        sqlite3_bind_text16(
            stmt,
            2,
            norminstname.getChar16Data(),
            norminstname.size() * 2,
            SQLITE_STATIC),
        db.get());

    CHECK_RC_DONE(sqlite3_step(stmt), db.get());

    stmtDestroyer.reset();
    db.release();

    PEG_METHOD_EXIT();
}

void SQLiteStore::deleteInstance(
    const CIMNamespaceName& nameSpace,
    const CIMObjectPath& instanceName)
{
    PEG_METHOD_ENTER(TRC_REPOSITORY, "SQLiteStore::deleteInstance");

    if (!instanceExists(nameSpace, instanceName))
    {
        throw PEGASUS_CIM_EXCEPTION(CIM_ERR_NOT_FOUND, instanceName.toString());
    }

    DbConnection db(_dbcm, nameSpace);

    _beginTransaction(db.get());

    const char* sqlStatement = "DELETE FROM InstanceTable "
        "WHERE norminstname=?;";

    sqlite3_stmt* stmt = 0;
    CHECK_RC_OK(
        sqlite3_prepare_v2(db.get(), sqlStatement, -1, &stmt, 0),
        db.get());
    AutoPtr<sqlite3_stmt, FinalizeSQLiteStatement> stmtDestroyer(stmt);

    String norminstname = instanceName._toStringCanonical();

    CHECK_RC_OK(
        sqlite3_bind_text16(
            stmt,
            1,
            norminstname.getChar16Data(),
            norminstname.size() * 2,
            SQLITE_STATIC),
        db.get());

    CHECK_RC_DONE(sqlite3_step(stmt), db.get());

    //
    // Delete from association table (if an assocation).
    //

    _removeInstanceAssociationEntries(db.get(), nameSpace, instanceName);

    _commitTransaction(db.get());

    stmtDestroyer.reset();
    db.release();

    PEG_METHOD_EXIT();
}

Boolean SQLiteStore::instanceExists(
    const CIMNamespaceName& nameSpace,
    const CIMObjectPath& instanceName)
{
    PEG_METHOD_ENTER(TRC_REPOSITORY, "SQLiteStore::instanceExists");

    DbConnection db(_dbcm, nameSpace);

    const char* sqlStatement = "SELECT instname FROM InstanceTable "
        "WHERE norminstname=?;";

    sqlite3_stmt* stmt = 0;
    CHECK_RC_OK(
        sqlite3_prepare_v2(db.get(), sqlStatement, -1, &stmt, 0),
        db.get());
    AutoPtr<sqlite3_stmt, FinalizeSQLiteStatement> stmtDestroyer(stmt);

    String norminstname = instanceName._toStringCanonical();

    CHECK_RC_OK(
        sqlite3_bind_text16(
            stmt,
            1,
            norminstname.getChar16Data(),
            norminstname.size() * 2,
            SQLITE_STATIC),
        db.get());

    int rc = sqlite3_step(stmt);
    Boolean found = false;

    if (rc == SQLITE_ROW)
    {
        found = true;
    }
    else
    {
        CHECK_RC_DONE(rc, db.get());
    }

    stmtDestroyer.reset();
    db.release();

    PEG_METHOD_EXIT();
    return found;
}

void SQLiteStore::_initAssocClassCache(
    const CIMNamespaceName& nameSpace,
    AssocClassCache* cache)
{
#ifdef USE_ASSOC_CLASS_CACHE
    PEG_METHOD_ENTER(TRC_REPOSITORY, "SQLiteStore::_initAssocClassCache");

    DbConnection db(_dbcm, nameSpace);

    const char* sqlStatement =
        "SELECT assocclassname, normfromclassname, normfrompropname, "
            "toclassname, normtopropname FROM ClassAssocTable;";

    sqlite3_stmt* stmt = 0;
    CHECK_RC_OK(
        sqlite3_prepare_v2(db.get(), sqlStatement, -1, &stmt, 0),
        db.get());
    AutoPtr<sqlite3_stmt, FinalizeSQLiteStatement> stmtDestroyer(stmt);

    int rc;
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW)
    {
        ClassAssociation classAssociation;
        classAssociation.assocClassName = CIMNameCast(String(
                (const Char16*)sqlite3_column_text16(stmt, 0),
                (Uint32)sqlite3_column_bytes16(stmt, 0) / 2));
        classAssociation.fromClassName = CIMNameCast(String(
                (const Char16*)sqlite3_column_text16(stmt, 1),
                (Uint32)sqlite3_column_bytes16(stmt, 1) / 2));
        classAssociation.fromPropertyName = CIMNameCast(String(
                (const Char16*)sqlite3_column_text16(stmt, 2),
                (Uint32)sqlite3_column_bytes16(stmt, 2) / 2));
        classAssociation.toClassName = CIMNameCast(String(
                (const Char16*)sqlite3_column_text16(stmt, 3),
                (Uint32)sqlite3_column_bytes16(stmt, 3) / 2));
        classAssociation.toPropertyName = CIMNameCast(String(
                (const Char16*)sqlite3_column_text16(stmt, 4),
                (Uint32)sqlite3_column_bytes16(stmt, 4) / 2));
        cache->addRecord(classAssociation.fromClassName, classAssociation);
    }

    CHECK_RC_DONE(rc, db.get());

    cache->setActive(true);

    stmtDestroyer.reset();
    db.release();

    PEG_METHOD_EXIT();
#endif
}

void SQLiteStore::_addClassAssociationEntries(
    sqlite3* db,
    const CIMNamespaceName& nameSpace,
    const Array<ClassAssociation>& classAssocEntries)
{
    PEG_METHOD_ENTER(TRC_REPOSITORY,
        "SQLiteStore::_addClassAssociationEntries");

    const char* sqlStatement =
        "INSERT INTO ClassAssocTable VALUES(?,?,?,?,?,?,?);";

    sqlite3_stmt* stmt = 0;
    CHECK_RC_OK(
        sqlite3_prepare_v2(db, sqlStatement, -1, &stmt, 0),
        db);
    AutoPtr<sqlite3_stmt, FinalizeSQLiteStatement> stmtDestroyer(stmt);

    for (Uint32 i = 0; i < classAssocEntries.size(); i++)
    {
        String assocclassname =
            classAssocEntries[i].assocClassName.getString();
        String normassocclassname = _getNormalizedName(
            classAssocEntries[i].assocClassName);
        String normfromclassname = _getNormalizedName(
            classAssocEntries[i].fromClassName);
        String normfrompropname = _getNormalizedName(
            classAssocEntries[i].fromPropertyName);
        String toclassname =
            classAssocEntries[i].toClassName.getString();
        String normtoclassname = _getNormalizedName(
            classAssocEntries[i].toClassName);
        String normtopropname = _getNormalizedName(
            classAssocEntries[i].toPropertyName);

        CHECK_RC_OK(
            sqlite3_bind_text16(
                stmt,
                1,
                assocclassname.getChar16Data(),
                assocclassname.size() * 2,
                SQLITE_STATIC),
            db);
        CHECK_RC_OK(
            sqlite3_bind_text16(
                stmt,
                2,
                normassocclassname.getChar16Data(),
                normassocclassname.size() * 2,
                SQLITE_STATIC),
            db);
        CHECK_RC_OK(
            sqlite3_bind_text16(
                stmt,
                3,
                normfromclassname.getChar16Data(),
                normfromclassname.size() * 2,
                SQLITE_STATIC),
            db);
        CHECK_RC_OK(
            sqlite3_bind_text16(
                stmt,
                4,
                normfrompropname.getChar16Data(),
                normfrompropname.size() * 2,
                SQLITE_STATIC),
            db);
        CHECK_RC_OK(
            sqlite3_bind_text16(
                stmt,
                5,
                toclassname.getChar16Data(),
                toclassname.size() * 2,
                SQLITE_STATIC),
            db);
        CHECK_RC_OK(
            sqlite3_bind_text16(
                stmt,
                6,
                normtoclassname.getChar16Data(),
                normtoclassname.size() * 2,
                SQLITE_STATIC),
            db);
        CHECK_RC_OK(
            sqlite3_bind_text16(
                stmt,
                7,
                normtopropname.getChar16Data(),
                normtopropname.size() * 2,
                SQLITE_STATIC),
            db);

        CHECK_RC_DONE(sqlite3_step(stmt), db);

        CHECK_RC_OK(sqlite3_reset(stmt), db);

        CHECK_RC_OK(sqlite3_clear_bindings(stmt), db);
    }

#ifdef USE_ASSOC_CLASS_CACHE

    String assocClassCacheName = nameSpace.getString();
    assocClassCacheName.toLower();
    AssocClassCache* cache =
        _assocClassCacheManager.getAssocClassCache(assocClassCacheName);

    if (cache->isActive())
    {
        for (Uint32 i = 0; i < classAssocEntries.size(); i++)
        {
            cache->addRecord(
                classAssocEntries[i].fromClassName, classAssocEntries[i]);
        }
    }

#endif

    PEG_METHOD_EXIT();
}

void SQLiteStore::_removeClassAssociationEntries(
    sqlite3* db,
    const CIMNamespaceName& nameSpace,
    const CIMName& assocClassName)
{
    PEG_METHOD_ENTER(TRC_REPOSITORY,
        "SQLiteStore::_removeClassAssociationEntries");

    const char* sqlStatement = "DELETE FROM ClassAssocTable "
        "WHERE normassocclassname=?;";

    sqlite3_stmt* stmt = 0;
    CHECK_RC_OK(
        sqlite3_prepare_v2(db, sqlStatement, -1, &stmt, 0),
        db);
    AutoPtr<sqlite3_stmt, FinalizeSQLiteStatement> stmtDestroyer(stmt);

    String normassocclassname = _getNormalizedName(assocClassName);

    CHECK_RC_OK(
        sqlite3_bind_text16(
            stmt,
            1,
            normassocclassname.getChar16Data(),
            normassocclassname.size() * 2,
            SQLITE_STATIC),
        db);

    CHECK_RC_DONE(sqlite3_step(stmt), db);

#ifdef USE_ASSOC_CLASS_CACHE

    String assocClassCacheName = nameSpace.getString();
    assocClassCacheName.toLower();
    AssocClassCache* cache =
        _assocClassCacheManager.getAssocClassCache(assocClassCacheName);

    if (cache->isActive())
    {
        cache->removeAssocClassRecords(assocClassName);
    }

#endif

    PEG_METHOD_EXIT();
}

void SQLiteStore::getClassAssociatorNames(
    const CIMNamespaceName& nameSpace,
    const Array<CIMName>& classList,
    const Array<CIMName>& assocClassList,
    const Array<CIMName>& resultClassList,
    const String& role,
    const String& resultRole,
    Array<String>& associatorNames)
{
    PEG_METHOD_ENTER(TRC_REPOSITORY,
        "SQLiteStore::getClassAssociatorNames");

    DbConnection db(_dbcm, nameSpace);

    String sqlStatement =
        "SELECT DISTINCT toclassname FROM ClassAssocTable WHERE (";

    // Match "from" class name
    for (Uint32 i = 0; i < classList.size(); i++)
    {
        if (i > 0)
        {
            sqlStatement.append(" OR ");
        }

        sqlStatement.append("normfromclassname=?");
    }
    sqlStatement.append(')');

    // Match role
    if (role.size())
    {
        sqlStatement.append(" AND normfrompropname=?");
    }

    // Match resultRole
    if (resultRole.size())
    {
        sqlStatement.append(" AND normtopropname=?");
    }

    // Match association class name
    if (assocClassList.size())
    {
        sqlStatement.append(" AND (");

        for (Uint32 i = 0; i < assocClassList.size(); i++)
        {
            if (i > 0)
            {
                sqlStatement.append(" OR ");
            }

            sqlStatement.append("normassocclassname=?");
        }

        sqlStatement.append(')');
    }

    // Match result class name
    if (resultClassList.size())
    {
        sqlStatement.append(" AND (");

        for (Uint32 i = 0; i < resultClassList.size(); i++)
        {
            if (i > 0)
            {
                sqlStatement.append(" OR ");
            }

            sqlStatement.append("normtoclassname=?");
        }

        sqlStatement.append(')');
    }

    sqlStatement.append(';');

    sqlite3_stmt* stmt = 0;
    CHECK_RC_OK(
        sqlite3_prepare_v2(db.get(), sqlStatement.getCString(), -1, &stmt, 0),
        db.get());
    AutoPtr<sqlite3_stmt, FinalizeSQLiteStatement> stmtDestroyer(stmt);

    int bindIndex = 1;

    for (Uint32 i = 0; i < classList.size(); i++)
    {
        String normfromclassname = _getNormalizedName(classList[i]);
        CHECK_RC_OK(
            sqlite3_bind_text16(
                stmt,
                bindIndex++,
                normfromclassname.getChar16Data(),
                normfromclassname.size() * 2,
                SQLITE_TRANSIENT),
            db.get());
    }

    if (role.size())
    {
        String normfrompropname = _getNormalizedName(CIMNameCast(role));
        CHECK_RC_OK(
            sqlite3_bind_text16(
                stmt,
                bindIndex++,
                normfrompropname.getChar16Data(),
                normfrompropname.size() * 2,
                SQLITE_TRANSIENT),
            db.get());
    }

    if (resultRole.size())
    {
        String normtopropname =
            _getNormalizedName(CIMNameCast(resultRole));
        CHECK_RC_OK(
            sqlite3_bind_text16(
                stmt,
                bindIndex++,
                normtopropname.getChar16Data(),
                normtopropname.size() * 2,
                SQLITE_TRANSIENT),
            db.get());
    }

    for (Uint32 i = 0; i < assocClassList.size(); i++)
    {
        String normassocclassname = _getNormalizedName(assocClassList[i]);
        CHECK_RC_OK(
            sqlite3_bind_text16(
                stmt,
                bindIndex++,
                normassocclassname.getChar16Data(),
                normassocclassname.size() * 2,
                SQLITE_TRANSIENT),
            db.get());
    }

    for (Uint32 i = 0; i < resultClassList.size(); i++)
    {
        String normtopropname = _getNormalizedName(resultClassList[i]);
        CHECK_RC_OK(
            sqlite3_bind_text16(
                stmt,
                bindIndex++,
                normtopropname.getChar16Data(),
                normtopropname.size() * 2,
                SQLITE_TRANSIENT),
            db.get());
    }

    int rc;
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW)
    {
        associatorNames.append(String(
            (const Char16*)sqlite3_column_text16(stmt, 0),
            (Uint32)sqlite3_column_bytes16(stmt, 0) / 2));
    }

    CHECK_RC_DONE(rc, db.get());

    stmtDestroyer.reset();
    db.release();

    PEG_METHOD_EXIT();
}

void SQLiteStore::getClassReferenceNames(
    const CIMNamespaceName& nameSpace,
    const Array<CIMName>& classList,
    const Array<CIMName>& resultClassList,
    const String& role,
    Array<String>& referenceNames)
{
    PEG_METHOD_ENTER(TRC_REPOSITORY,
        "SQLiteStore::getClassReferenceNames");

#ifdef USE_ASSOC_CLASS_CACHE

    String assocClassCacheName = nameSpace.getString();
    assocClassCacheName.toLower();
    AssocClassCache* cache =
        _assocClassCacheManager.getAssocClassCache(assocClassCacheName);

    if (!cache->isActive())
    {
        _initAssocClassCache(nameSpace, cache);
    }

    cache->getReferenceNames(classList, resultClassList, role, referenceNames);

#else

    DbConnection db(_dbcm, nameSpace);

    String sqlStatement =
        "SELECT DISTINCT assocclassname FROM ClassAssocTable WHERE (";

    // Match "from" class name
    for (Uint32 i = 0; i < classList.size(); i++)
    {
        if (i > 0)
        {
            sqlStatement.append(" OR ");
        }

        sqlStatement.append("normfromclassname=?");
    }
    sqlStatement.append(')');

    // Match role
    if (role.size())
    {
        sqlStatement.append(" AND normfrompropname=?");
    }

    // Match result class name
    if (resultClassList.size())
    {
        sqlStatement.append(" AND (");

        for (Uint32 i = 0; i < resultClassList.size(); i++)
        {
            if (i > 0)
            {
                sqlStatement.append(" OR ");
            }

            sqlStatement.append("normassocclassname=?");
        }

        sqlStatement.append(')');
    }

    sqlStatement.append(';');

    sqlite3_stmt* stmt = 0;
    CHECK_RC_OK(
        sqlite3_prepare_v2(db.get(), sqlStatement.getCString(), -1, &stmt, 0),
        db.get());
    AutoPtr<sqlite3_stmt, FinalizeSQLiteStatement> stmtDestroyer(stmt);

    int bindIndex = 1;

    for (Uint32 i = 0; i < classList.size(); i++)
    {
        String normfromclassname = _getNormalizedName(classList[i]);
        CHECK_RC_OK(
            sqlite3_bind_text16(
                stmt,
                bindIndex++,
                normfromclassname.getChar16Data(),
                normfromclassname.size() * 2,
                SQLITE_TRANSIENT),
            db.get());
    }

    if (role.size())
    {
        String normfrompropname = _getNormalizedName(CIMNameCast(role));
        CHECK_RC_OK(
            sqlite3_bind_text16(
                stmt,
                bindIndex++,
                normfrompropname.getChar16Data(),
                normfrompropname.size() * 2,
                SQLITE_TRANSIENT),
            db.get());
    }

    for (Uint32 i = 0; i < resultClassList.size(); i++)
    {
        String normassocclassname = _getNormalizedName(resultClassList[i]);
        CHECK_RC_OK(
            sqlite3_bind_text16(
                stmt,
                bindIndex++,
                normassocclassname.getChar16Data(),
                normassocclassname.size() * 2,
                SQLITE_TRANSIENT),
            db.get());
    }

    int rc;
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW)
    {
        referenceNames.append(String(
            (const Char16*)sqlite3_column_text16(stmt, 0),
            (Uint32)sqlite3_column_bytes16(stmt, 0) / 2));
    }

    CHECK_RC_DONE(rc, db.get());

    stmtDestroyer.reset();
    db.release();
#endif

    PEG_METHOD_EXIT();
}

void SQLiteStore::_addInstanceAssociationEntries(
    sqlite3* db,
    const CIMNamespaceName& nameSpace,
    const Array<InstanceAssociation>& instanceAssocEntries)
{
    PEG_METHOD_ENTER(TRC_REPOSITORY,
        "SQLiteStore::_addInstanceAssociationEntries");

    const char* sqlStatement =
        "INSERT INTO InstanceAssocTable VALUES(?,?,?,?,?,?,?,?);";

    sqlite3_stmt* stmt = 0;
    CHECK_RC_OK(
        sqlite3_prepare_v2(db, sqlStatement, -1, &stmt, 0),
        db);
    AutoPtr<sqlite3_stmt, FinalizeSQLiteStatement> stmtDestroyer(stmt);

    for (Uint32 i = 0; i < instanceAssocEntries.size(); i++)
    {
        String associnstname =
            instanceAssocEntries[i].assocInstanceName;
        // Note: This converts a String to a CIMObjectPath and back to a String
        String normassocinstname = CIMObjectPath(
            instanceAssocEntries[i].assocInstanceName)._toStringCanonical();
        String normassocclassname = _getNormalizedName(
            instanceAssocEntries[i].assocClassName);
        // Note: This converts a String to a CIMObjectPath and back to a String
        String normfrominstname = CIMObjectPath(
            instanceAssocEntries[i].fromInstanceName)._toStringCanonical();
        String normfrompropname = _getNormalizedName(
            instanceAssocEntries[i].fromPropertyName);
        String toinstname =
            instanceAssocEntries[i].toInstanceName;
        String normtoclassname = _getNormalizedName(
            instanceAssocEntries[i].toClassName);
        String normtopropname = _getNormalizedName(
            instanceAssocEntries[i].toPropertyName);

        CHECK_RC_OK(
            sqlite3_bind_text16(
                stmt,
                1,
                associnstname.getChar16Data(),
                associnstname.size() * 2,
                SQLITE_STATIC),
            db);
        CHECK_RC_OK(
            sqlite3_bind_text16(
                stmt,
                2,
                normassocinstname.getChar16Data(),
                normassocinstname.size() * 2,
                SQLITE_STATIC),
            db);
        CHECK_RC_OK(
            sqlite3_bind_text16(
                stmt,
                3,
                normassocclassname.getChar16Data(),
                normassocclassname.size() * 2,
                SQLITE_STATIC),
            db);
        CHECK_RC_OK(
            sqlite3_bind_text16(
                stmt,
                4,
                normfrominstname.getChar16Data(),
                normfrominstname.size() * 2,
                SQLITE_STATIC),
            db);
        CHECK_RC_OK(
            sqlite3_bind_text16(
                stmt,
                5,
                normfrompropname.getChar16Data(),
                normfrompropname.size() * 2,
                SQLITE_STATIC),
            db);
        CHECK_RC_OK(
            sqlite3_bind_text16(
                stmt,
                6,
                toinstname.getChar16Data(),
                toinstname.size() * 2,
                SQLITE_STATIC),
            db);
        CHECK_RC_OK(
            sqlite3_bind_text16(
                stmt,
                7,
                normtoclassname.getChar16Data(),
                normtoclassname.size() * 2,
                SQLITE_STATIC),
            db);
        CHECK_RC_OK(
            sqlite3_bind_text16(
                stmt,
                8,
                normtopropname.getChar16Data(),
                normtopropname.size() * 2,
                SQLITE_STATIC),
            db);

        CHECK_RC_DONE(sqlite3_step(stmt), db);

        CHECK_RC_OK(sqlite3_reset(stmt), db);

        CHECK_RC_OK(sqlite3_clear_bindings(stmt), db);
    }

    PEG_METHOD_EXIT();
}

void SQLiteStore::_removeInstanceAssociationEntries(
    sqlite3* db,
    const CIMNamespaceName& nameSpace,
    const CIMObjectPath& assocInstanceName)
{
    PEG_METHOD_ENTER(TRC_REPOSITORY,
        "SQLiteStore::_removeInstanceAssociationEntries");

    const char* sqlStatement = "DELETE FROM InstanceAssocTable "
        "WHERE normassocinstname=?;";

    sqlite3_stmt* stmt = 0;
    CHECK_RC_OK(
        sqlite3_prepare_v2(db, sqlStatement, -1, &stmt, 0),
        db);
    AutoPtr<sqlite3_stmt, FinalizeSQLiteStatement> stmtDestroyer(stmt);

    String associnstname = assocInstanceName._toStringCanonical();

    CHECK_RC_OK(
        sqlite3_bind_text16(
            stmt,
            1,
            associnstname.getChar16Data(),
            associnstname.size() * 2,
            SQLITE_STATIC),
        db);

    CHECK_RC_DONE(sqlite3_step(stmt), db);

    PEG_METHOD_EXIT();
}

void SQLiteStore::getInstanceAssociatorNames(
    const CIMNamespaceName& nameSpace,
    const CIMObjectPath& instanceName,
    const Array<CIMName>& assocClassList,
    const Array<CIMName>& resultClassList,
    const String& role,
    const String& resultRole,
    Array<String>& associatorNames)
{
    PEG_METHOD_ENTER(TRC_REPOSITORY,
        "SQLiteStore::getInstanceAssociatorNames");

    DbConnection db(_dbcm, nameSpace);

    String sqlStatement = "SELECT DISTINCT toinstname "
        "FROM InstanceAssocTable WHERE normfrominstname=?";

    // Match role
    if (role.size())
    {
        sqlStatement.append(" AND normfrompropname=?");
    }

    // Match resultRole
    if (resultRole.size())
    {
        sqlStatement.append(" AND normtopropname=?");
    }

    // Match association class name
    if (assocClassList.size())
    {
        sqlStatement.append(" AND (");

        for (Uint32 i = 0; i < assocClassList.size(); i++)
        {
            if (i > 0)
            {
                sqlStatement.append(" OR ");
            }

            sqlStatement.append("normassocclassname=?");
        }

        sqlStatement.append(')');
    }

    // Match result class name
    if (resultClassList.size())
    {
        sqlStatement.append(" AND (");

        for (Uint32 i = 0; i < resultClassList.size(); i++)
        {
            if (i > 0)
            {
                sqlStatement.append(" OR ");
            }

            sqlStatement.append("normtoclassname=?");
        }

        sqlStatement.append(')');
    }

    sqlStatement.append(';');

    sqlite3_stmt* stmt = 0;
    CHECK_RC_OK(
        sqlite3_prepare_v2(db.get(), sqlStatement.getCString(), -1, &stmt, 0),
        db.get());
    AutoPtr<sqlite3_stmt, FinalizeSQLiteStatement> stmtDestroyer(stmt);

    String normfrominstname = instanceName._toStringCanonical();
    int bindIndex = 1;

    CHECK_RC_OK(
        sqlite3_bind_text16(
            stmt,
            bindIndex++,
            normfrominstname.getChar16Data(),
            normfrominstname.size() * 2,
            SQLITE_STATIC),
        db.get());

    if (role.size())
    {
        String normfrompropname = _getNormalizedName(CIMNameCast(role));
        CHECK_RC_OK(
            sqlite3_bind_text16(
                stmt,
                bindIndex++,
                normfrompropname.getChar16Data(),
                normfrompropname.size() * 2,
                SQLITE_TRANSIENT),
            db.get());
    }

    if (resultRole.size())
    {
        String normtopropname =
            _getNormalizedName(CIMNameCast(resultRole));
        CHECK_RC_OK(
            sqlite3_bind_text16(
                stmt,
                bindIndex++,
                normtopropname.getChar16Data(),
                normtopropname.size() * 2,
                SQLITE_TRANSIENT),
            db.get());
    }

    for (Uint32 i = 0; i < assocClassList.size(); i++)
    {
        String normassocclassname = _getNormalizedName(assocClassList[i]);
        CHECK_RC_OK(
            sqlite3_bind_text16(
                stmt,
                bindIndex++,
                normassocclassname.getChar16Data(),
                normassocclassname.size() * 2,
                SQLITE_TRANSIENT),
            db.get());
    }

    for (Uint32 i = 0; i < resultClassList.size(); i++)
    {
        String normtoclassname = _getNormalizedName(resultClassList[i]);
        CHECK_RC_OK(
            sqlite3_bind_text16(
                stmt,
                bindIndex++,
                normtoclassname.getChar16Data(),
                normtoclassname.size() * 2,
                SQLITE_TRANSIENT),
            db.get());
    }

    int rc;
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW)
    {
        associatorNames.append(String(
            (const Char16*)sqlite3_column_text16(stmt, 0),
            (Uint32)sqlite3_column_bytes16(stmt, 0) / 2));
    }

    CHECK_RC_DONE(rc, db.get());

    stmtDestroyer.reset();
    db.release();

    PEG_METHOD_EXIT();
}

void SQLiteStore::getInstanceReferenceNames(
    const CIMNamespaceName& nameSpace,
    const CIMObjectPath& instanceName,
    const Array<CIMName>& resultClassList,
    const String& role,
    Array<String>& referenceNames)
{
    PEG_METHOD_ENTER(TRC_REPOSITORY,
        "SQLiteStore::getInstanceReferenceNames");

    DbConnection db(_dbcm, nameSpace);

    String sqlStatement = "SELECT DISTINCT associnstname "
        "FROM InstanceAssocTable WHERE normfrominstname=?";

    // Match role
    if (role.size())
    {
        sqlStatement.append(" AND normfrompropname=?");
    }

    // Match result class name
    if (resultClassList.size())
    {
        sqlStatement.append(" AND (");

        for (Uint32 i = 0; i < resultClassList.size(); i++)
        {
            if (i > 0)
            {
                sqlStatement.append(" OR ");
            }

            sqlStatement.append("normassocclassname=?");
        }

        sqlStatement.append(')');
    }

    sqlStatement.append(';');

    sqlite3_stmt* stmt = 0;
    CHECK_RC_OK(
        sqlite3_prepare_v2(db.get(), sqlStatement.getCString(), -1, &stmt, 0),
        db.get());
    AutoPtr<sqlite3_stmt, FinalizeSQLiteStatement> stmtDestroyer(stmt);

    String normfrominstname = instanceName._toStringCanonical();
    int bindIndex = 1;

    CHECK_RC_OK(
        sqlite3_bind_text16(
            stmt,
            bindIndex++,
            normfrominstname.getChar16Data(),
            normfrominstname.size() * 2,
            SQLITE_STATIC),
        db.get());

    if (role.size())
    {
        String normfrompropname = _getNormalizedName(CIMNameCast(role));
        CHECK_RC_OK(
            sqlite3_bind_text16(
                stmt,
                bindIndex++,
                normfrompropname.getChar16Data(),
                normfrompropname.size() * 2,
                SQLITE_TRANSIENT),
            db.get());
    }

    for (Uint32 i = 0; i < resultClassList.size(); i++)
    {
        String normassocclassname = _getNormalizedName(resultClassList[i]);
        CHECK_RC_OK(
            sqlite3_bind_text16(
                stmt,
                bindIndex++,
                normassocclassname.getChar16Data(),
                normassocclassname.size() * 2,
                SQLITE_TRANSIENT),
            db.get());
    }

    int rc;
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW)
    {
        referenceNames.append(String(
            (const Char16*)sqlite3_column_text16(stmt, 0),
            (Uint32)sqlite3_column_bytes16(stmt, 0) / 2));
    }

    CHECK_RC_DONE(rc, db.get());

    stmtDestroyer.reset();
    db.release();

    PEG_METHOD_EXIT();
}

PEGASUS_NAMESPACE_END
