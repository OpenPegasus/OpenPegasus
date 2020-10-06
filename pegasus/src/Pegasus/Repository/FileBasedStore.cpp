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
#include <cctype>
#include <cstdio>
#include <fstream>

#include <Pegasus/Common/InternalException.h>
#include <Pegasus/Common/DeclContext.h>
#include <Pegasus/Common/Resolver.h>
#include <Pegasus/Common/System.h>
#include <Pegasus/Common/Tracer.h>
#include <Pegasus/Common/MessageLoader.h>
#include <Pegasus/Common/FileSystem.h>
#include <Pegasus/Common/Dir.h>
#include <Pegasus/Common/CommonUTF.h>
#include <Pegasus/Common/ReadWriteSem.h>
#include "InstanceIndexFile.h"
#include "InstanceDataFile.h"
#include "AssocInstTable.h"
#include "FileBasedStore.h"

#ifdef PEGASUS_ENABLE_COMPRESSED_REPOSITORY
// #define win32
# include <zlib.h>
# include <sstream>
#endif

PEGASUS_USING_STD;

PEGASUS_NAMESPACE_BEGIN

static const char _CLASSES_DIR[] = "classes";
static const char _INSTANCES_DIR[] = "instances";
static const char _QUALIFIERS_DIR[] = "qualifiers";

static const char _CLASSES_SUFFIX[] = "/classes";
static const char _INSTANCES_SUFFIX[] = "/instances";
static const char _QUALIFIERS_SUFFIX[] = "/qualifiers";
static const char _ASSOCIATIONS_SUFFIX[] = "/associations";

// The config file name is selected such that it cannot collide with a
// namespace directory name.  Namespace directories may not contain a '.'.
static const char _CONFIGFILE_NAME[] = "repository.conf";

static const Uint32 _MAX_FREE_COUNT = 16;

#define REPOSITORY_BEGIN_PROGRESS_FILE     "begin.progress";
#define REPOSITORY_COMMIT_PROGRESS_FILE    "commit.progress";
#define REPOSITORY_ROLLBACK_PROGRESS_FILE  "rollback.progress";

//
// This static variable is used inside "rollbackInstanceTransaction" function
// to determine if it is called from the constructor of "CIMRepository" during
// startup in order to avoid the creation of "rollback.progress" state file
// as it increases the startup time of the cimserver process.
// true => startup
//
static bool startup = true;


static inline String _escapeUtf8FileNameCharacters(const String& fileName)
{
#ifdef PEGASUS_REPOSITORY_ESCAPE_UTF8
    // All chars above 0x7F will be escape.
    return escapeStringEncoder(fileName);
#else
    return fileName;
#endif
}

static inline String _unescapeUtf8FileNameCharacters(const String& fileName)
{
#ifdef PEGASUS_REPOSITORY_ESCAPE_UTF8
    return escapeStringDecoder(fileName);
#else
    return fileName;
#endif
}

////////////////////////////////////////////////////////////////////////////////
//
// _namespaceNameToDirName()
//
////////////////////////////////////////////////////////////////////////////////

static String _namespaceNameToDirName(const CIMNamespaceName& namespaceName)
{
    String nameSpaceDirName = namespaceName.getString();

    for (Uint32 i = 0; i < nameSpaceDirName.size(); i++)
    {
        if (nameSpaceDirName[i] == '/')
        {
            nameSpaceDirName[i] = '#';
        }
    }

    return _escapeUtf8FileNameCharacters(nameSpaceDirName);
}

////////////////////////////////////////////////////////////////////////////////
//
// _dirNameToNamespaceName()
//
////////////////////////////////////////////////////////////////////////////////

static String _dirNameToNamespaceName(const String& nameSpaceDirName)
{
    String namespaceName = nameSpaceDirName;

    for (Uint32 i = 0; i < namespaceName.size(); i++)
    {
        if (namespaceName[i] == '#')
        {
            namespaceName[i] = '/';
        }
    }
#ifdef PEGASUS_REPOSITORY_ESCAPE_UTF8
    // All chars above 0x7F will be escape.
    return escapeStringDecoder(namespaceName);
#else
    return namespaceName;
#endif
}

////////////////////////////////////////////////////////////////////////////////
//
// _LoadFileToMemory()  PEP214
//
// The gzxxxx functions read both compresed and non-compresed files.
//
// There is no conditional flag on reading of files since gzread()
// (from zlib) is capable of reading compressed and non-compressed
// files (so it contains the logic that examines the header
// and magic number). Everything will work properly if the repository
// has some compressed and some non-compressed files.
//
//
////////////////////////////////////////////////////////////////////////////////

static void _LoadFileToMemory(Buffer& data, const String& path)
{

#ifdef PEGASUS_ENABLE_COMPRESSED_REPOSITORY

    Uint32 fileSize;

    if (!FileSystem::getFileSize(path, fileSize))
        throw CannotOpenFile(path);

    gzFile fp = gzopen(path.getCString(), "rb");

    if (fp == NULL)
        throw CannotOpenFile(path);

    data.reserveCapacity(fileSize);
    char buffer[4096];
    int n;

    while ((n = gzread(fp, buffer, sizeof(buffer))) > 0)
        data.append(buffer, n);

    gzclose(fp);

#else

    FileSystem::loadFileToMemory(data, path);

#endif /* PEGASUS_ENABLE_COMPRESSED_REPOSITORY */
}

////////////////////////////////////////////////////////////////////////////////
//
// _LoadObject()
//
//      Loads objects (classes and qualifiers) from disk to
//      memory objects.
//
////////////////////////////////////////////////////////////////////////////////

template<class Object>
void _LoadObject(
    const String& path,
    Object& object,
    ObjectStreamer* streamer)
{
    PEG_METHOD_ENTER(TRC_REPOSITORY, "FileBasedStore::_LoadObject");

    // Get the real path of the file:

    String realPath;

    if (!FileSystem::existsNoCase(path, realPath))
    {
        PEG_TRACE((TRC_REPOSITORY, Tracer::LEVEL1,
            "%s does not exist.",(const char*)path.getCString()));
        PEG_METHOD_EXIT();
        throw CannotOpenFile(path);
    }

    PEG_TRACE((TRC_REPOSITORY, Tracer::LEVEL4, "realpath = %s",
        (const char*)realPath.getCString()));

    // Load file into memory:

    Buffer data;

    _LoadFileToMemory(data, realPath);

    streamer->decode(data, 0, object);

    PEG_METHOD_EXIT();
}

////////////////////////////////////////////////////////////////////////////////
//
// _beginInstanceTransaction()
//
//      Creates rollback files to allow an incomplete transaction to be voided.
//
////////////////////////////////////////////////////////////////////////////////

static void _beginInstanceTransaction(
    const String& indexFilePath,
    const String& dataFilePath)
{
    PEG_METHOD_ENTER(TRC_REPOSITORY, "_beginInstanceTransaction");

    // Create a state file for the begin instance transaction in the same
    // directory where the rollback files will be created for
    // index and data file.
    // The CIMRepository checks if this file is present during
    // its initialization.
    // If it is present, it is assumed that the last begin transaction was
    // incomplete due to system failure and it removes the rollback files along
    // with this state file

    String dirPath = FileSystem::extractFilePath(indexFilePath);
    String stateFilePath = dirPath + REPOSITORY_BEGIN_PROGRESS_FILE;

    fstream fs;

    fs.open(stateFilePath.getCString(), ios::out PEGASUS_OR_IOS_BINARY);

    if (!fs)
    {
        PEG_METHOD_EXIT();
            throw PEGASUS_CIM_EXCEPTION_L(CIM_ERR_FAILED,
                MessageLoaderParms(
                    "Repository.CIMRepository.BEGIN_FAILED",
                    "The attempt to begin the transaction failed."));
    }
    fs.close();

    //
    // Begin the transaction (an incomplete transaction will cause
    // a rollback the next time an instance-oriented routine is invoked).
    //

    if (!InstanceIndexFile::beginTransaction(indexFilePath))
    {
        //
        // Remove the state file
        //
        FileSystem::removeFile(stateFilePath);

        PEG_METHOD_EXIT();
        throw PEGASUS_CIM_EXCEPTION_L(CIM_ERR_FAILED,
            MessageLoaderParms(
                "Repository.CIMRepository.BEGIN_FAILED",
                "The attempt to begin the transaction failed."));
    }

    if (!InstanceDataFile::beginTransaction(dataFilePath))
    {
        //
        // The creation of the index and data rollback file should be atomic
        // So undo the begin transaction of index file in case of error in
        // the begin transaction of the data file
        //
        InstanceIndexFile::undoBeginTransaction(indexFilePath);

        //
        // Remove the state file
        //
        FileSystem::removeFile(stateFilePath);

        PEG_METHOD_EXIT();
        throw PEGASUS_CIM_EXCEPTION_L(CIM_ERR_FAILED,
            MessageLoaderParms(
                "Repository.CIMRepository.BEGIN_FAILED",
                "The attempt to begin the transaction failed."));
    }
    //
    // Since both the rollback files are created, begin transaction is over.
    // So remove the state file for the begin transaction
    //
    FileSystem::removeFile(stateFilePath);


    PEG_METHOD_EXIT();
}

////////////////////////////////////////////////////////////////////////////////
//
// _commitInstanceTransaction()
//
//      Removes the rollback files to complete the transaction.
//
////////////////////////////////////////////////////////////////////////////////

static void _commitInstanceTransaction(
    const String& indexFilePath,
    const String& dataFilePath)
{
    PEG_METHOD_ENTER(TRC_REPOSITORY, "_commitInstanceTransaction");

    // Create a state file for the commit instance transaction in the same
    // directory where the rollback files are created for index and data file.
    // The CIMRepository checks if this file is present during
    // its initialization.
    // If it is present, it is assumed that the last commit transaction was
    // incomplete due to system failure and it removes the rollback files along
    // with this state file

    String dirPath = FileSystem::extractFilePath(indexFilePath);
    String stateFilePath = dirPath + REPOSITORY_COMMIT_PROGRESS_FILE;

    fstream fs;

    fs.open(stateFilePath.getCString(), ios::out PEGASUS_OR_IOS_BINARY);

    if (!fs)
    {
        PEG_METHOD_EXIT();
        throw PEGASUS_CIM_EXCEPTION_L(CIM_ERR_FAILED,
            MessageLoaderParms(
                "Repository.CIMRepository.COMMIT_FAILED",
                "The commit operation failed."));
    }

    fs.close();

    //
    //
    // Commit the transaction by removing the rollback files.
    //

    if (!InstanceIndexFile::commitTransaction(indexFilePath))
    {
        //
        // Remove the state file
        //
        FileSystem::removeFile(stateFilePath);

        PEG_METHOD_EXIT();
        throw PEGASUS_CIM_EXCEPTION_L(CIM_ERR_FAILED,
            MessageLoaderParms(
                "Repository.CIMRepository.COMMIT_FAILED",
                "The commit operation failed."));
    }

    if (!InstanceDataFile::commitTransaction(dataFilePath))
    {
        //
        // Remove the state file
        //

        FileSystem::removeFile(stateFilePath);

        PEG_METHOD_EXIT();
        throw PEGASUS_CIM_EXCEPTION_L(CIM_ERR_FAILED,
            MessageLoaderParms(
                "Repository.CIMRepository.COMMIT_FAILED",
                "The commit operation failed."));
    }

    //
    // Since both the rollback files are removed, commit transaction is over.
    // So remove the state file for the commit transaction
    //
    FileSystem::removeFile(stateFilePath);

    PEG_METHOD_EXIT();
}

////////////////////////////////////////////////////////////////////////////////
//
// _rollbackInstanceTransaction()
//
//      Restores instance index and data files to void an incomplete operation.
//      If there are no rollback files, this method has no effect.
//
////////////////////////////////////////////////////////////////////////////////

static String _dirName(const String& path)
{
    Uint32 n = path.size();

    for (Uint32 i = n; i != 0; )
    {
        if (path[--i] == '/')
            return path.subString(0, i);
    }

    return String(".");
}

static void _rollbackInstanceTransaction(
    const String& indexFilePath,
    const String& dataFilePath)
{
    PEG_METHOD_ENTER(TRC_REPOSITORY, "_rollbackInstanceTransaction");

    // Avoid rollback logic if directory has no .rollback files.

    String path = _dirName(indexFilePath);
    Array<String> rollbackFiles;

    if (FileSystem::glob(path, "*.rollback", rollbackFiles))
    {
        if (rollbackFiles.size() == 0)
            return;
    }

    // Proceed to rollback logic.
    String dirPath = FileSystem::extractFilePath(indexFilePath);
    String stateFilePath = dirPath + REPOSITORY_ROLLBACK_PROGRESS_FILE;

    //
    // Check the static variable "startup" to determine if this function is
    // called from the constructor of "CIMRepository" during startup.
    // If it is true do not create the "rollback.progress" state file.
    //

    if (!startup)
    {
        // Create a state file for the rollback instance transaction in the same
        // directory where the rollback files are created for
        // index and data file.
        // The CIMRepository checks if this file is present during
        // its initialization.
        // If it is present, it is assumed that the last rollback transaction
        // was incomplete due to system failure. So it completes
        // the rollback instance transaction and removes this state file

        fstream fs;

        fs.open(stateFilePath.getCString(), ios::out PEGASUS_OR_IOS_BINARY);

        if (!fs)
        {
            PEG_METHOD_EXIT();
                throw PEGASUS_CIM_EXCEPTION_L(CIM_ERR_FAILED,
                    MessageLoaderParms(
                        "Repository.CIMRepository.ROLLBACK_FAILED",
                        "The rollback operation failed."));
        }

        fs.close();
    }

    if (!InstanceIndexFile::rollbackTransaction(indexFilePath))
    {
        if(!startup)
        {
            // Remove the state file
            FileSystem::removeFile(stateFilePath);
        }

        PEG_METHOD_EXIT();
        throw PEGASUS_CIM_EXCEPTION_L(CIM_ERR_FAILED,
            MessageLoaderParms(
                "Repository.CIMRepository.ROLLBACK_FAILED",
                "The rollback operation failed."));
    }

    if (!InstanceDataFile::rollbackTransaction(dataFilePath))
    {
        if(!startup)
        {
            // Remove the state file
            FileSystem::removeFile(stateFilePath);
        }

        PEG_METHOD_EXIT();
        throw PEGASUS_CIM_EXCEPTION_L(CIM_ERR_FAILED,
            MessageLoaderParms(
                "Repository.CIMRepository.ROLLBACK_FAILED",
                "The rollback operation failed."));
    }

    if (!startup)
    {
        // Since both the rollback files are removed,
        // rollback transaction is over.
        // So remove the state file for the rollback transaction
        FileSystem::removeFile(stateFilePath);
    }

    PEG_METHOD_EXIT();
}


////////////////////////////////////////////////////////////////////////////////
//
//  FileBasedStore::_completeTransactions()
//
//      Searches for state file in the "instance" directory of all
//      namespaces.
//      i)   Removes the rollback files to void a begin operation.
//      ii)  Removes the rollback files to complete a commit operation.
//      iii) Restores instance index and data files complete a rollback
//           operation.
//      If no state files are present, this method returns false
//
////////////////////////////////////////////////////////////////////////////////

Boolean FileBasedStore::_completeTransactions()
{
    PEG_METHOD_ENTER(TRC_REPOSITORY,
        "FileBasedStore::_completeTransactions");

    for (Dir dir(_repositoryPath); dir.more(); dir.next())
    {
        String nameSpaceDirName = dir.getName();
        if ((nameSpaceDirName == "..") ||
            (nameSpaceDirName == ".") ||
            (nameSpaceDirName == _CONFIGFILE_NAME))
        {
            continue;
        }

        String nameSpacePath = _repositoryPath + "/" + nameSpaceDirName;
        String nameSpaceName = _dirNameToNamespaceName(nameSpaceDirName);

        if (!FileSystem::isDirectory(nameSpacePath + _CLASSES_SUFFIX) ||
            !FileSystem::isDirectory(nameSpacePath + _INSTANCES_SUFFIX) ||
            !FileSystem::isDirectory(nameSpacePath + _QUALIFIERS_SUFFIX))
        {
            PEG_TRACE((TRC_REPOSITORY, Tracer::LEVEL2,
                "Namespace: %s ignored -- "
                "subdirectories are not correctly formed",
                (const char*)nameSpaceDirName.getCString()));
            continue;
        }

        //
        // Get the instance directory path
        //
        String dirPath = nameSpacePath + _INSTANCES_SUFFIX;
        String classesPath = nameSpacePath + _CLASSES_SUFFIX;

        //
        // Get the paths of the state files
        //
        String beginFilePath=dirPath+"/"+REPOSITORY_BEGIN_PROGRESS_FILE;
        String commitFilePath=dirPath+"/"+REPOSITORY_COMMIT_PROGRESS_FILE;
        String rollbackfilePath=dirPath+"/"+REPOSITORY_ROLLBACK_PROGRESS_FILE;

        Array<String> classNames;

        for (Dir dir(classesPath); dir.more(); dir.next())
        {
            String fileName = dir.getName();
            // Ignore the current and parent directories.
            if (fileName == "." || fileName == "..")
                continue;

            Uint32 dot = fileName.find('.');

            // Ignore files without dots in them:
            if (dot == PEG_NOT_FOUND)
            {
                continue;
            }
            String className =
                _unescapeUtf8FileNameCharacters(fileName.subString(0, dot));
            classNames.append(className);
        }

        if(FileSystem::exists(beginFilePath))
        {
            //
            // Either the begin or the commit operation is left incomplete
            // Begin -> Actual repository update operation is not started.
            // Commit -> Actual repository update operation is complete.
            //
            for (Uint32 j = 0; j < classNames.size(); j++)
            {
                //
                // Get paths of index and data files:
                //

                String indexFilePath = _getInstanceIndexFilePath(
                    nameSpaceName, classNames[j]);

                String dataFilePath = _getInstanceDataFilePath(
                    nameSpaceName, classNames[j]);

                InstanceIndexFile::undoBeginTransaction(indexFilePath);
                InstanceDataFile::undoBeginTransaction(dataFilePath);
            }

            FileSystem::removeFile(beginFilePath);

            PEG_METHOD_EXIT();
            return true;
        }

        if(FileSystem::exists(commitFilePath))
        {
            //
            // Either the begin or the commit operation is left incomplete
            // Begin -> Actual repository update operation is not started.
            // Commit -> Actual repository update operation is complete.
            // In both cases, we can safely remove the rollback files
            //

            for (Uint32 j = 0; j < classNames.size(); j++)
            {
                //
                // Get paths of index and data files:
                //

                String indexFilePath = _getInstanceIndexFilePath(
                    nameSpaceName, classNames[j]);

                String dataFilePath = _getInstanceDataFilePath(
                    nameSpaceName, classNames[j]);

                InstanceIndexFile::commitTransaction(indexFilePath);
                InstanceDataFile::commitTransaction(dataFilePath);
            }

            FileSystem::removeFile(commitFilePath);

            PEG_METHOD_EXIT();
            return true;
        }

        if(FileSystem::exists(rollbackfilePath))
        {
            // Rollback transaction is left incomplete
            // Rollback -> Call rollback

            for (Uint32 j = 0; j < classNames.size(); j++)
            {
                // Get paths of index and data files:
                String indexFilePath = _getInstanceIndexFilePath(
                    nameSpaceName, classNames[j]);

                String dataFilePath = _getInstanceDataFilePath(
                    nameSpaceName, classNames[j]);

                _rollbackInstanceTransaction(indexFilePath, dataFilePath);
            }

            FileSystem::removeFile(rollbackfilePath);

            PEG_METHOD_EXIT();
            return true;
        }
    }

    PEG_METHOD_EXIT();
    return false;
}

////////////////////////////////////////////////////////////////////////////////
//
// InstanceTransactionHandler
//
//      This class uses a simple recovery scheme to avoid corruption of the
//      instance repository during a "write" operation.  A transaction is
//      started when the class is instantiated, committed when the complete()
//      method is called, and rolled back if the destructor is called without
//      a prior call to complete().
//
//      The appropriate repository write locks must be owned while an
//      InstanceTransactionHandler instance exists.
//
//      This algorithm is used to allow recovery on an operation failure:
//
//      1.  Check to see if any rollback files exist for instances of the
//          given class. If so, perform rollback
//      2.  Create a rollback file for the instance file. The rollback file
//          contains the original size of the instance file.
//      3.  Create a rollback file for the index file. The rollback file is a
//          copy of the instance file.
//      4.  Update the instance file to perform the operation.
//      5.  Update the index file to perform the operation.
//      6.  Increment the free count in the index file if necessary, and
//          perform reorganization if the limit is reached.
//      7.  Delete the rollback files.
//
//      The recoverability algorithm itself works as follows:
//
//      1.  Delete the index file.
//      2.  Rename the index rollback file to have the same name as the
//          index file.
//      3.  Truncate the instance file to have the same number of bytes as
//          indicated in the instance rollback file.
//      4.  Delete the rollback files.
//
////////////////////////////////////////////////////////////////////////////////

class InstanceTransactionHandler
{
public:
    InstanceTransactionHandler(
        const String& indexFilePath,
        const String& dataFilePath)
    : _indexFilePath(indexFilePath),
      _dataFilePath(dataFilePath),
      _isComplete(false)
    {
        _rollbackInstanceTransaction(_indexFilePath, _dataFilePath);
        _beginInstanceTransaction(_indexFilePath, _dataFilePath);
    }

    ~InstanceTransactionHandler()
    {
        if (!_isComplete)
        {
            _rollbackInstanceTransaction(_indexFilePath, _dataFilePath);
        }
    }

    void complete()
    {
        _commitInstanceTransaction(_indexFilePath, _dataFilePath);
        _isComplete = true;
    }

private:
    String _indexFilePath;
    String _dataFilePath;
    Boolean _isComplete;
};


////////////////////////////////////////////////////////////////////////////////
//
// FileBasedStore
//
////////////////////////////////////////////////////////////////////////////////

Boolean FileBasedStore::isExistingRepository(const String& repositoryRoot)
{
    // If this is an existing FileBasedStore repository directory, a "root"
    // subdirectory will exist.
    return FileSystem::isDirectory(repositoryRoot + "/root");
}

FileBasedStore::FileBasedStore(
    const String& repositoryPath,
    ObjectStreamer* streamer,
    Boolean compressMode)
    : _repositoryPath(repositoryPath),
      _streamer(streamer)
#ifdef PEGASUS_ENABLE_COMPRESSED_REPOSITORY
      ,_compressMode(compressMode)
#endif
{
    PEG_METHOD_ENTER(TRC_REPOSITORY, "FileBasedStore::FileBasedStore");

    // Create the repository directory if it does not already exist.

    if (!FileSystem::isDirectory(_repositoryPath))
    {
        if (!FileSystem::makeDirectory(_repositoryPath))
        {
            PEG_METHOD_EXIT();
            throw CannotCreateDirectory(_repositoryPath);
        }
    }

    // Initialize the _storeCompleteClasses member based on the existing
    // repository configuration (if it already exists) or the build option.

#ifdef PEGASUS_REPOSITORY_STORE_COMPLETE_CLASSES
    _storeCompleteClasses = true;
#else
    _storeCompleteClasses = false;
#endif

    String configFilePath = _repositoryPath + "/" + _CONFIGFILE_NAME;

    if (!FileBasedStore::isExistingRepository(_repositoryPath))
    {
        // This is a new repository instance.  Use the setting defined by
        // the build option.

#ifndef PEGASUS_REPOSITORY_STORE_COMPLETE_CLASSES
        PEGASUS_STD(ofstream) os;
        if (!OpenAppend(os, configFilePath))
        {
            PEG_METHOD_EXIT();
            throw CannotOpenFile(configFilePath);
        }
        os << "storeCompleteClasses=false" << endl;
#endif
    }
    else
    {
        // Repository existed previously.  Determine whether its classes are
        // complete.

        if (FileSystem::exists(configFilePath))
        {
            ifstream ifs(configFilePath.getCString());

            if (!ifs)
            {
                PEG_METHOD_EXIT();
                throw CannotOpenFile(configFilePath);
            }

            // Config file exists.  Read storeCompleteClasses property.
            // For now, this is the only thing that may appear in the file,
            // so the parsing is easy.
            String line;
            if (GetLine(ifs, line))
            {
                if (String::equal(line, "storeCompleteClasses=false"))
                {
                    _storeCompleteClasses = false;
                }
                else
                {
                    throw Exception(MessageLoaderParms(
                        "Repository.CIMRepository.INVALID_CONFIG_FILE_ENTRY",
                        "File $0 contains an invalid entry: \"$1\".",
                        (const char*)configFilePath.getCString(),
                        (const char*)line.getCString()));
                }
            }
            else
            {
                throw Exception(MessageLoaderParms(
                    "Repository.CIMRepository.EMPTY_CONFIG_FILE",
                    "File $0 is empty.",
                    (const char*)configFilePath.getCString()));
            }
        }
        else
        {
            // An existing repository without a config file indicates the
            // legacy behavior.
            _storeCompleteClasses = true;
        }
    }

    //
    // Check if any state files are present in the repository.
    // and bring the repository to a consistent state based
    // on the particular state file found.
    //
    if (!_completeTransactions())
    {
        //
        // No state files are found in the repository. So, try to
        // rollback any incomplete transactions in the repository
        //
        _rollbackIncompleteTransactions();
    }

    //
    // Reset the variable "startup" to false to indicate the
    // end of CIMRepository constructor
    //
    startup = false;

    PEG_METHOD_EXIT();
}

FileBasedStore::~FileBasedStore()
{
}

////////////////////////////////////////////////////////////////////////////////
//
// FileBasedStore::_rollbackIncompleteTransactions()
//
//      Searches for incomplete instance transactions for all classes in all
//      namespaces.  Restores instance index and data files to void an
//      incomplete operation.  If no incomplete instance transactions are
//      outstanding, this method has no effect.
//
////////////////////////////////////////////////////////////////////////////////

void FileBasedStore::_rollbackIncompleteTransactions()
{
    PEG_METHOD_ENTER(TRC_REPOSITORY,
        "FileBasedStore::_rollbackIncompleteTransactions");

    for (Dir dir(_repositoryPath); dir.more(); dir.next())
    {
        String nameSpaceDirName = dir.getName();
        if ((nameSpaceDirName == "..") ||
            (nameSpaceDirName == ".") ||
            (nameSpaceDirName == _CONFIGFILE_NAME))
        {
            continue;
        }

        String instanceDirPath =
            _repositoryPath + "/" + nameSpaceDirName + _INSTANCES_SUFFIX;

        // Form a list of .rollback files.

        Array<String> rollbackFiles;
        FileSystem::glob(instanceDirPath, "*.rollback", rollbackFiles);

        // Perform a rollback operation for each class for which a rollback
        // file was found.  A rollback may be performed twice for the same
        // class if index and instance rollback files exist, but that will
        // not cause a problem.

        for (Uint32 i = 0; i < rollbackFiles.size(); i++)
        {
            // Parse the class name out of the file name.  (We know the
            // file name contains a '.', since it matched the pattern above.)
            String className =
                rollbackFiles[i].subString(0, rollbackFiles[i].find('.'));

            _rollbackInstanceTransaction(
                instanceDirPath + "/" + className + ".idx",
                instanceDirPath + "/" + className + ".instances");
        }
    }

    PEG_METHOD_EXIT();
}

// This function needs to be called from within a transaction scope for
// proper error handling in compacting index and data files.
static void _CompactInstanceRepository(
    const String& indexFilePath,
    const String& dataFilePath)
{
    PEG_METHOD_ENTER(TRC_REPOSITORY,
        "FileBasedStore::_CompactInstanceRepository");

    //
    // Compact the data file first:
    //

    Array<Uint32> freeFlags;
    Array<Uint32> indices;
    Array<Uint32> sizes;
    Array<CIMObjectPath> instanceNames;

    if (!InstanceIndexFile::enumerateEntries(
            indexFilePath, freeFlags, indices, sizes, instanceNames, true))
    {
        PEG_METHOD_EXIT();
        throw PEGASUS_CIM_EXCEPTION_L(CIM_ERR_FAILED,
            MessageLoaderParms(
                "Repository.CIMRepository.INDEX_ENUM_ENTRIES_FAILED",
                "Failed to obtain the entries from the Repository Instance"
                " Index file."));
    }

    if (!InstanceDataFile::compact(dataFilePath, freeFlags, indices, sizes))
    {
        PEG_METHOD_EXIT();
        throw PEGASUS_CIM_EXCEPTION_L(CIM_ERR_FAILED,
            MessageLoaderParms(
                "Repository.CIMRepository.COMPACT_FAILED",
                "Failed to compact the Repository Instance Data file."));
    }

    //
    // Now compact the index file:
    //

    if (!InstanceIndexFile::compact(indexFilePath))
    {
        PEG_METHOD_EXIT();
        throw PEGASUS_CIM_EXCEPTION_L(CIM_ERR_FAILED,
            MessageLoaderParms(
                "Repository.CIMRepository.INDEX_COMPACT_FAILED",
                "Failed to compact the Repository Instance Index file."));
    }

    PEG_METHOD_EXIT();
}

//----------------------------------------------------------------------
//
// _getNameSpaceDirPath()
//
//      returns the path of a namespace directory.
//
//----------------------------------------------------------------------

String FileBasedStore::_getNameSpaceDirPath(
    const CIMNamespaceName& nameSpace) const
{
    String path;
    PEGASUS_FCT_EXECUTE_AND_ASSERT(
        true,
        _nameSpacePathTable.lookup(nameSpace.getString(), path));
    
    return path;
}

//----------------------------------------------------------------------
//
// _getQualifierFilePath()
//
//      returns the path of the qualifier file.
//
//----------------------------------------------------------------------

String FileBasedStore::_getQualifierFilePath(
    const CIMNamespaceName& nameSpace,
    const CIMName& qualifierName) const
{
    String tmp = _getNameSpaceDirPath(nameSpace);
    tmp.append(_QUALIFIERS_SUFFIX);
    tmp.append('/');
    tmp.append(_escapeUtf8FileNameCharacters(qualifierName.getString()));
    return tmp;
}

//----------------------------------------------------------------------
//
// _getClassFilePath()
//
//      returns the path of the class file.
//
//----------------------------------------------------------------------

String FileBasedStore::_getClassFilePath(
    const CIMNamespaceName& nameSpace,
    const CIMName& className,
    const CIMName& superClassName) const
{
    String tmp = _getNameSpaceDirPath(nameSpace);
    tmp.append(_CLASSES_SUFFIX);
    tmp.append('/');
    tmp.append(_escapeUtf8FileNameCharacters(className.getString()));

    if (superClassName.isNull())
    {
        tmp.append(".#");
    }
    else
    {
        tmp.append('.');
        tmp.append(_escapeUtf8FileNameCharacters(superClassName.getString()));
    }

    return tmp;
}

//----------------------------------------------------------------------
//
// _getInstanceIndexFilePath()
//
//      returns the path of the instance index file.
//
//----------------------------------------------------------------------

String FileBasedStore::_getInstanceIndexFilePath(
    const CIMNamespaceName& nameSpace,
    const CIMName& className) const
{
    String tmp = _getNameSpaceDirPath(nameSpace);
    tmp.append(_INSTANCES_SUFFIX);
    tmp.append('/');
    tmp.append(_escapeUtf8FileNameCharacters(className.getString()));
    tmp.append(".idx");
    return tmp;
}

//----------------------------------------------------------------------
//
// _getInstanceDataFilePath()
//
//      returns the path of the instance file.
//
//----------------------------------------------------------------------

String FileBasedStore::_getInstanceDataFilePath(
    const CIMNamespaceName& nameSpace,
    const CIMName& className) const
{
    String tmp = _getNameSpaceDirPath(nameSpace);
    tmp.append(_INSTANCES_SUFFIX);
    tmp.append('/');
    tmp.append(_escapeUtf8FileNameCharacters(className.getString()));
    tmp.append(".instances");
    return tmp;
}

//----------------------------------------------------------------------
//
// _getAssocClassPath()
//
//      returns the path of the class association file.
//
//----------------------------------------------------------------------

String FileBasedStore::_getAssocClassPath(
    const CIMNamespaceName& nameSpace) const
{
    String tmp = _getNameSpaceDirPath(nameSpace);
    tmp.append(_CLASSES_SUFFIX);
    tmp.append(_ASSOCIATIONS_SUFFIX);
    return tmp;
}

//----------------------------------------------------------------------
//
// _getAssocInstPath()
//
//      returns the path of the instance association file.
//
//----------------------------------------------------------------------

String FileBasedStore::_getAssocInstPath(
    const CIMNamespaceName& nameSpace) const
{
    String tmp = _getNameSpaceDirPath(nameSpace);
    tmp.append(_INSTANCES_SUFFIX);
    tmp.append(_ASSOCIATIONS_SUFFIX);
    return tmp;
}

Boolean FileBasedStore::_loadInstance(
    const String& path,
    CIMInstance& object,
    Uint32 index,
    Uint32 size)
{
    PEG_METHOD_ENTER(TRC_REPOSITORY, "FileBasedStore::_loadInstance");

    //
    // Load instance (in XML) from instance file into memory:
    //

    Buffer data;

    if (!InstanceDataFile::loadInstance(path, index, size, data))
    {
        PEG_METHOD_EXIT();
        return false;
    }

    //
    // Convert XML into an actual object:
    //

    _streamer->decode(data, 0, object);

    PEG_METHOD_EXIT();
    return true;
}

Boolean FileBasedStore::_loadAllInstances(
    const CIMNamespaceName& nameSpace,
    const CIMName& className,
    Array<CIMInstance>& namedInstances)
{
    PEG_METHOD_ENTER(TRC_REPOSITORY, "FileBasedStore::_loadAllInstances");

    Array<CIMObjectPath> instanceNames;
    Buffer data;
    Array<Uint32> indices;
    Array<Uint32> sizes;

    //
    // Form the names of the instance index and data files
    //

    String indexFilePath = _getInstanceIndexFilePath(nameSpace, className);
    String dataFilePath = _getInstanceDataFilePath(nameSpace, className);

    //
    // Enumerate the index file:
    //

    Array<Uint32> freeFlags;

    if (!InstanceIndexFile::enumerateEntries(
            indexFilePath, freeFlags, indices, sizes, instanceNames, true))
    {
        PEG_METHOD_EXIT();
        return false;
    }

    //
    // Form the array of instances result:
    //

    if (instanceNames.size() > 0)
    {
        //
        // Load all instances from the data file:
        //

        if (!InstanceDataFile::loadAllInstances(dataFilePath, data))
        {
            PEG_METHOD_EXIT();
            return false;
        }

        //
        // for each instance loaded, call XML parser to parse the XML
        // data and create a CIMInstance object.
        //

        CIMInstance tmpInstance;

        const char* buffer = data.getData();

        for (Uint32 i = 0; i < instanceNames.size(); i++)
        {
            if (!freeFlags[i])
            {
                Uint32 pos= (Uint32)((&(buffer[indices[i]]))-buffer);
                _streamer->decode(data, pos, tmpInstance);

                tmpInstance.setPath(instanceNames[i]);

                namedInstances.append(tmpInstance);
            }
        }
    }

    PEG_METHOD_EXIT();
    return true;
}

Array<NamespaceDefinition> FileBasedStore::enumerateNameSpaces()
{
    PEG_METHOD_ENTER(TRC_REPOSITORY, "FileBasedStore::enumerateNameSpaces");

    Array<NamespaceDefinition> nameSpaces;

    for (Dir dir(_repositoryPath); dir.more(); dir.next())
    {
        String nameSpaceDirName = dir.getName();
        if ((nameSpaceDirName == "..") ||
            (nameSpaceDirName == ".") ||
            (nameSpaceDirName == _CONFIGFILE_NAME))
        {
            continue;
        }

        String nameSpacePath = _repositoryPath + "/" + nameSpaceDirName;

        if (!FileSystem::isDirectory(nameSpacePath + _CLASSES_SUFFIX) ||
            !FileSystem::isDirectory(nameSpacePath + _INSTANCES_SUFFIX) ||
            !FileSystem::isDirectory(nameSpacePath + _QUALIFIERS_SUFFIX))
        {
            PEG_TRACE((TRC_REPOSITORY, Tracer::LEVEL2,
                "Namespace: %s ignored -- "
                "subdirectories are not correctly formed",
                (const char*)nameSpaceDirName.getCString()));
            continue;
        }

        NamespaceDefinition nsdef(_dirNameToNamespaceName(nameSpaceDirName));

        Boolean skipThisNamespace = false;

        for (Dir subdir(nameSpacePath); subdir.more(); subdir.next())
        {
            String nameSpaceSubDirName = subdir.getName();
            if (nameSpaceSubDirName == ".." || nameSpaceSubDirName == ".")
            {
                continue;
            }

            String tmp = nameSpaceSubDirName;
            tmp.toLower();

            if (tmp[0] == 's')
            {
                if ((tmp[1]=='w' || tmp[1]=='r') &&
                    (tmp[2]=='f' || tmp[2]=='s'))
                {
                    nsdef.shareable = tmp[2]=='s';
                    nsdef.updatesAllowed = tmp[1]=='w';
                    String parent = nameSpaceSubDirName.subString(3);
                    if (parent.size())
                    {
                        nsdef.parentNameSpace = _dirNameToNamespaceName(parent);
                    }
                }
                else
                {
                    PEG_TRACE((TRC_REPOSITORY, Tracer::LEVEL2,
                        "Namespace %s ignored - "
                        "using incorrect parent namespace specification: %s",
                        (const char*)nameSpaceDirName.getCString(),
                        (const char*)nameSpaceSubDirName.getCString()));

                    skipThisNamespace = true;
                }
#ifndef PEGASUS_ENABLE_REMOTE_CMPI
                break;
#endif
            }
#ifdef PEGASUS_ENABLE_REMOTE_CMPI
            else if (tmp[0] == 'r')
            {
                // remoteInfo format is "ridhost[@port]" where "id" is a lower
                // case two-character identifier
                nsdef.remoteInfo = nameSpaceSubDirName;

                PEG_TRACE((TRC_REPOSITORY, Tracer::LEVEL4,
                    "Remote namespace: %s >%s",
                    (const char*)nameSpaceDirName.getCString(),
                    (const char*)nameSpaceSubDirName.getCString()));
            }
#endif
        }

        if (!skipThisNamespace)
        {
            _nameSpacePathTable.insert(
                nsdef.name.getString(),
                _repositoryPath + "/" + nameSpaceDirName);
            nameSpaces.append(nsdef);
        }
    }

    PEG_METHOD_EXIT();
    return nameSpaces;
}

void FileBasedStore::createNameSpace(
    const CIMNamespaceName& nameSpace,
    Boolean shareable,
    Boolean updatesAllowed,
    const String& parentNameSpace,
    const String& remoteInfo)
{
    PEG_METHOD_ENTER(TRC_REPOSITORY, "FileBasedStore::createNameSpace");

#ifndef PEGASUS_SUPPORT_UTF8_FILENAME
    // Do not allow file names to contain characters outside of 7-bit ASCII.
    String nameSpaceNameString = nameSpace.getString();
    Uint32 len = nameSpaceNameString.size();
    for (Uint32 i = 0; i < len; ++i)
    {
        if ((Uint16)nameSpaceNameString[i] > 0x007F)
        {
            PEG_METHOD_EXIT();
            throw PEGASUS_CIM_EXCEPTION(
                CIM_ERR_INVALID_PARAMETER, nameSpaceNameString);
        }
    }
#endif

    // Attempt to create all the namespace directories:

    String nameSpacePath =
        _repositoryPath + "/" + _namespaceNameToDirName(nameSpace);

    if (!FileSystem::makeDirectory(nameSpacePath))
    {
        throw CannotCreateDirectory(nameSpacePath);
    }

    String classesPath = nameSpacePath + _CLASSES_SUFFIX;
    String instancesPath = nameSpacePath + _INSTANCES_SUFFIX;
    String qualifiersPath = nameSpacePath + _QUALIFIERS_SUFFIX;

    if (!FileSystem::makeDirectory(classesPath))
    {
        throw CannotCreateDirectory(classesPath);
    }

    if (!FileSystem::makeDirectory(instancesPath))
    {
        throw CannotCreateDirectory(instancesPath);
    }

    if (!FileSystem::makeDirectory(qualifiersPath))
    {
        throw CannotCreateDirectory(qualifiersPath);
    }

    if (shareable || !updatesAllowed || parentNameSpace.size())
    {
        String path = nameSpacePath + "/S" + (updatesAllowed ? "W" : "R") +
            (shareable ? "S" : "F");
        if (parentNameSpace.size())
        {
            path.append(_namespaceNameToDirName(parentNameSpace));
        }

        if (!FileSystem::makeDirectory(path))
        {
            throw CannotCreateDirectory(path);
        }
    }

#ifdef PEGASUS_ENABLE_REMOTE_CMPI
    if (remoteInfo.size())
    {
        String remoteDir(nameSpacePath);
        remoteDir.append("/r10");
        remoteDir.append(remoteInfo);
        if (!FileSystem::makeDirectory(remoteDir))
        {
            throw CannotCreateDirectory(remoteDir);
        }
    }
#else
    // use unused parameter
    remoteInfo.size();
#endif

    _nameSpacePathTable.insert(nameSpace.getString(), nameSpacePath);

    PEG_METHOD_EXIT();
}

void FileBasedStore::modifyNameSpace(
    const CIMNamespaceName& nameSpace,
    Boolean shareable,
    Boolean updatesAllowed)
{
    PEG_METHOD_ENTER(TRC_REPOSITORY, "FileBasedStore::modifyNameSpace");

    String nameSpacePath = _getNameSpaceDirPath(nameSpace);
    String oldSpecialDirName;

    for (Dir subdir(nameSpacePath); subdir.more(); subdir.next())
    {
        String dirName = subdir.getName();

        if ((dirName[0] == 's') || (dirName[0] == 'S'))
        {
            oldSpecialDirName = dirName;
            break;
        }
    }

    String newSpecialDirName = oldSpecialDirName;

    if (newSpecialDirName.size() == 0)
    {
        newSpecialDirName = "SWF";
    }

    newSpecialDirName[0] = 'S';
    newSpecialDirName[1] = updatesAllowed ? 'W' : 'R';
    newSpecialDirName[2] = shareable ? 'S' : 'F';

    if (newSpecialDirName != oldSpecialDirName)
    {
        if (oldSpecialDirName.size())
        {
            FileSystem::removeDirectoryHier(
                nameSpacePath + "/" + oldSpecialDirName);
        }

        if (newSpecialDirName != "SWF")
        {
            String newPath = nameSpacePath + "/" + newSpecialDirName;
            if (!FileSystem::makeDirectory(newPath))
            {
                PEG_METHOD_EXIT();
                throw CannotCreateDirectory(newPath);
            }
        }
    }

    PEG_METHOD_EXIT();
}

void FileBasedStore::modifyNameSpaceName(
        const CIMNamespaceName& nameSpace,
        const CIMNamespaceName& newNameSpaceName)
{
    PEG_METHOD_ENTER(TRC_REPOSITORY, "FileBasedStore::modifyNameSpaceName");

    String nameSpacePath = _getNameSpaceDirPath(nameSpace);
    String repositoryPath = nameSpacePath;
    Uint32 pos = repositoryPath.reverseFind('/');
    repositoryPath.remove(pos+1);
    repositoryPath.append(_namespaceNameToDirName(newNameSpaceName));
    if (!FileSystem::renameFile(nameSpacePath, repositoryPath))
    {
        PEG_METHOD_EXIT();
        throw CannotRenameFile(nameSpacePath);
    }
    PEG_METHOD_EXIT();
}

void FileBasedStore::deleteNameSpace(const CIMNamespaceName& nameSpace)
{
    PEG_METHOD_ENTER(TRC_REPOSITORY, "FileBasedStore::deleteNameSpace");

    String nameSpacePath = _getNameSpaceDirPath(nameSpace);

    if (!FileSystem::removeDirectoryHier(nameSpacePath))
    {
        PEG_METHOD_EXIT();
        throw CannotRemoveDirectory(nameSpacePath);
    }

    _nameSpacePathTable.remove(nameSpace.getString());

    PEG_METHOD_EXIT();
}

Boolean FileBasedStore::isNameSpaceEmpty(const CIMNamespaceName& nameSpace)
{
    PEG_METHOD_ENTER(TRC_REPOSITORY, "FileBasedStore::isNameSpaceEmpty");

    String nameSpacePath = _getNameSpaceDirPath(nameSpace);

    for (Dir dir(nameSpacePath); dir.more(); dir.next())
    {
        const char* name = dir.getName();

        if (strcmp(name, ".") != 0 &&
            strcmp(name, "..") != 0 &&
            System::strcasecmp(name, _CLASSES_DIR) != 0 &&
            System::strcasecmp(name, _INSTANCES_DIR) != 0 &&
            System::strcasecmp(name, _QUALIFIERS_DIR) != 0)
        {
            // ATTN: Is it assumed that dependent namespaces are empty?
            return true;
        }
    }

    String classesPath = nameSpacePath + _CLASSES_SUFFIX;
    String instancesPath = nameSpacePath + _INSTANCES_SUFFIX;
    String qualifiersPath = nameSpacePath + _QUALIFIERS_SUFFIX;

    PEG_METHOD_EXIT();
    return
        FileSystem::isDirectoryEmpty(classesPath) &&
        FileSystem::isDirectoryEmpty(instancesPath) &&
        FileSystem::isDirectoryEmpty(qualifiersPath);
}

Array<CIMQualifierDecl> FileBasedStore::enumerateQualifiers(
    const CIMNamespaceName& nameSpace)
{
    PEG_METHOD_ENTER(TRC_REPOSITORY, "FileBasedStore::enumerateQualifiers");

    String qualifiersRoot =
        _getNameSpaceDirPath(nameSpace) + _QUALIFIERS_SUFFIX;

    Array<String> qualifierNames;

    if (!FileSystem::getDirectoryContents(qualifiersRoot, qualifierNames))
    {
        PEG_METHOD_EXIT();
        String str ="enumerateQualifiers()";
        throw PEGASUS_CIM_EXCEPTION_L(CIM_ERR_FAILED,
            MessageLoaderParms("Repository.CIMRepository.INTERNAL_ERROR",
                "$0: internal error",
                str));
    }

    Array<CIMQualifierDecl> qualifiers;

    for (Uint32 i = 0; i < qualifierNames.size(); i++)
    {
        CIMQualifierDecl qualifier = getQualifier(
            nameSpace, _unescapeUtf8FileNameCharacters(qualifierNames[i]));
        qualifiers.append(qualifier);
    }

    PEG_METHOD_EXIT();
    return qualifiers;
}

CIMQualifierDecl FileBasedStore::getQualifier(
    const CIMNamespaceName& nameSpace,
    const CIMName& qualifierName)
{
    PEG_METHOD_ENTER(TRC_REPOSITORY, "FileBasedStore::getQualifier");

    CIMQualifierDecl qualifierDecl;

    //
    // Get path of qualifier file:
    //

    String qualifierFilePath = _getQualifierFilePath(nameSpace, qualifierName);

    try
    {
        _LoadObject(qualifierFilePath, qualifierDecl, _streamer);
    }
    catch (const CannotOpenFile&)
    {
        // Qualifier not found
        PEG_METHOD_EXIT();
        return CIMQualifierDecl();
    }

    PEG_METHOD_EXIT();
    return qualifierDecl;
}

void FileBasedStore::setQualifier(
    const CIMNamespaceName& nameSpace,
    const CIMQualifierDecl& qualifierDecl)
{
    PEG_METHOD_ENTER(TRC_REPOSITORY, "FileBasedStore::setQualifier");

    // -- Get path of qualifier file:

    String qualifierFilePath =
        _getQualifierFilePath(nameSpace, qualifierDecl.getName());

    // -- If qualifier already exists, throw exception:

    if (FileSystem::existsNoCase(qualifierFilePath))
    {
        PEG_METHOD_EXIT();
        throw PEGASUS_CIM_EXCEPTION(
            CIM_ERR_NOT_SUPPORTED, qualifierDecl.getName().getString());
    }

    // -- Save qualifier:

    Buffer qualifierDeclXml;
    _streamer->encode(qualifierDeclXml, qualifierDecl);
    _SaveObject(qualifierFilePath, qualifierDeclXml);

    PEG_METHOD_EXIT();
}

void FileBasedStore::deleteQualifier(
    const CIMNamespaceName& nameSpace,
    const CIMName& qualifierName)
{
    PEG_METHOD_ENTER(TRC_REPOSITORY, "FileBasedStore::deleteQualifier");

    // -- Get path of qualifier file:

    String qualifierFilePath = _getQualifierFilePath(nameSpace, qualifierName);

    // -- Delete qualifier:

    if (!FileSystem::removeFileNoCase(qualifierFilePath))
    {
        PEG_METHOD_EXIT();
        throw PEGASUS_CIM_EXCEPTION(
            CIM_ERR_NOT_FOUND, qualifierName.getString());
    }

    PEG_METHOD_EXIT();
}

Array<Pair<String, String> > FileBasedStore::enumerateClassNames(
    const CIMNamespaceName& nameSpace)
{
    PEG_METHOD_ENTER(TRC_REPOSITORY, "FileBasedStore::enumerateClassNames");

    Array<Pair<String, String> > classList;

    String classesPath = _getNameSpaceDirPath(nameSpace) + _CLASSES_SUFFIX;

    for (Dir dir(classesPath); dir.more(); dir.next())
    {
        String fileName = dir.getName();

        // Ignore the current and parent directories.

        if (fileName == "." || fileName == "..")
            continue;

        Uint32 dot = fileName.find('.');

        // Ignore files without dots in them:

        if (dot == PEG_NOT_FOUND)
            continue;

        String className =
            _unescapeUtf8FileNameCharacters(fileName.subString(0, dot));
        String superClassName =
            _unescapeUtf8FileNameCharacters(fileName.subString(dot + 1));

        if (superClassName == "#")
            superClassName.clear();

        classList.append(Pair<String, String>(className, superClassName));
    }

    PEG_METHOD_EXIT();
    return classList;
}

CIMClass FileBasedStore::getClass(
    const CIMNamespaceName& nameSpace,
    const CIMName& className,
    const CIMName& superClassName)
{
    PEG_METHOD_ENTER(TRC_REPOSITORY, "FileBasedStore::getClass");

    String classFilePath =
        _getClassFilePath(nameSpace, className, superClassName);
    CIMClass cimClass;
    _LoadObject(classFilePath, cimClass, _streamer);

    PEG_METHOD_EXIT();
    return cimClass;
}

void FileBasedStore::createClass(
    const CIMNamespaceName& nameSpace,
    const CIMClass& newClass,
    const Array<ClassAssociation>& classAssocEntries)
{
    PEG_METHOD_ENTER(TRC_REPOSITORY, "FileBasedStore::createClass");

#ifndef PEGASUS_SUPPORT_UTF8_FILENAME
    // Do not allow file names to contain characters outside of 7-bit ASCII.
    String classNameString = newClass.getClassName().getString();
    Uint32 len = classNameString.size();
    for (Uint32 i = 0; i < len; ++i)
    {
        if ((Uint16)classNameString[i] > 0x007F)
        {
            PEG_METHOD_EXIT();
            throw PEGASUS_CIM_EXCEPTION(
                CIM_ERR_INVALID_PARAMETER, classNameString);
        }
    }
#endif

    String classFilePath = _getClassFilePath(
        nameSpace, newClass.getClassName(), newClass.getSuperClassName());
    Buffer classXml;
    _streamer->encode(classXml, newClass);
    _SaveObject(classFilePath, classXml);

    if (classAssocEntries.size())
    {
        _addClassAssociationEntries(nameSpace, classAssocEntries);
    }

    PEG_METHOD_EXIT();
}

void FileBasedStore::modifyClass(
    const CIMNamespaceName& nameSpace,
    const CIMClass& modifiedClass,
    const CIMName& oldSuperClassName,
    Boolean isAssociation,
    const Array<ClassAssociation>& classAssocEntries)
{
    PEG_METHOD_ENTER(TRC_REPOSITORY, "FileBasedStore::modifyClass");

    String classFilePath = _getClassFilePath(
        nameSpace,
        modifiedClass.getClassName(),
        modifiedClass.getSuperClassName());

    String oldClassFilePath = _getClassFilePath(
        nameSpace,
        modifiedClass.getClassName(),
        oldSuperClassName);

    //
    // Delete the old file containing the class:
    //

    if (!FileSystem::removeFileNoCase(oldClassFilePath))
    {
        PEG_METHOD_EXIT();
        // ATTN: Parameter should be file name, not method name.
        String str = "FileBasedStore::modifyClass()";
        throw PEGASUS_CIM_EXCEPTION_L(CIM_ERR_FAILED,
            MessageLoaderParms(
                "Repository.CIMRepository.FAILED_TO_REMOVE_FILE",
                "failed to remove file in $0", classFilePath /*str*/));
    }

    //
    // Create new class file:
    //

    Buffer classXml;
    _streamer->encode(classXml, modifiedClass);
    _SaveObject(classFilePath, classXml);

    //
    // Update the association entries
    //

    if (isAssociation)
    {
        _removeClassAssociationEntries(nameSpace, modifiedClass.getClassName());
        if (classAssocEntries.size())
        {
            _addClassAssociationEntries(nameSpace, classAssocEntries);
        }
    }

    PEG_METHOD_EXIT();
}

void FileBasedStore::deleteClass(
    const CIMNamespaceName& nameSpace,
    const CIMName& className,
    const CIMName& superClassName,
    Boolean isAssociation,
    const Array<CIMNamespaceName>& dependentNameSpaceNames)
{
    PEG_METHOD_ENTER(TRC_REPOSITORY, "FileBasedStore::deleteClass");

    //
    // Clean up the instance files in this namespace and in all dependent
    // namespaces.  (It was already checked that no instances exist.)
    //

    for (Uint32 i = 0; i < dependentNameSpaceNames.size(); i++)
    {
        String indexFilePath =
            _getInstanceIndexFilePath(nameSpace, className);
        String dataFilePath =
            _getInstanceDataFilePath(nameSpace, className);

        FileSystem::removeFileNoCase(indexFilePath);
        FileSystem::removeFileNoCase(dataFilePath);
    }

    // Remove class file

    String classFilePath =
        _getClassFilePath(nameSpace, className, superClassName);

    if (!FileSystem::removeFileNoCase(classFilePath))
    {
        PEG_METHOD_EXIT();
        throw CannotRemoveFile(classFilePath);
    }

    //
    // Remove association entries
    //

    if (isAssociation)
    {
        _removeClassAssociationEntries(nameSpace, className);
    }

    PEG_METHOD_EXIT();
}

Array<CIMObjectPath> FileBasedStore::enumerateInstanceNamesForClass(
    const CIMNamespaceName& nameSpace,
    const CIMName& className)
{
    PEG_METHOD_ENTER(TRC_REPOSITORY,
        "FileBasedStore::enumerateInstanceNamesForClass");

    //
    // Get instance names from the instance index file for the class:
    //
    Array<Uint32> indices;
    Array<Uint32> sizes;

    //
    // Form the names of the instance index and data files
    //

    String indexFilePath = _getInstanceIndexFilePath(nameSpace, className);
    String dataFilePath = _getInstanceDataFilePath(nameSpace, className);

    //
    // Get all instances for the class:
    //

    Array<CIMObjectPath> instanceNames;
    Array<Uint32> freeFlags;

    if (!InstanceIndexFile::enumerateEntries(
            indexFilePath, freeFlags, indices, sizes, instanceNames, false))
    {
        PEG_METHOD_EXIT();
        throw PEGASUS_CIM_EXCEPTION_L(CIM_ERR_FAILED,
            MessageLoaderParms(
                "Repository.CIMRepository.FAILED_TO_LOAD_INSTANCE_NAMES",
                "Failed to load instance names in class $0",
                className.getString()));
    }

    PEG_METHOD_EXIT();
    return instanceNames;
}

Array<CIMInstance> FileBasedStore::enumerateInstancesForClass(
    const CIMNamespaceName& nameSpace,
    const CIMName& className)
{
    PEG_METHOD_ENTER(TRC_REPOSITORY,
        "FileBasedStore::enumerateInstancesForClass");

    Array<CIMInstance> cimInstances;

    if (!_loadAllInstances(nameSpace, className, cimInstances))
    {
        PEG_METHOD_EXIT();
        throw PEGASUS_CIM_EXCEPTION_L(CIM_ERR_FAILED,
            MessageLoaderParms(
                "Repository.CIMRepository.FAILED_TO_LOAD_INSTANCES",
                "Failed to load instances in class $0",
                className.getString()));
    }

    PEG_METHOD_EXIT();
    return cimInstances;
}

CIMInstance FileBasedStore::getInstance(
    const CIMNamespaceName& nameSpace,
    const CIMObjectPath& instanceName)
{
    PEG_METHOD_ENTER(TRC_REPOSITORY, "FileBasedStore::getInstance");

    //
    // Get paths of index and data files:
    //

    String indexFilePath = _getInstanceIndexFilePath(
        nameSpace, instanceName.getClassName());

    String dataFilePath = _getInstanceDataFilePath(
        nameSpace, instanceName.getClassName());

    //
    // Get the index for this instance:
    //

    Uint32 index;
    Uint32 size;

    if (!InstanceIndexFile::lookupEntry(
            indexFilePath, instanceName, index, size))
    {
        PEG_METHOD_EXIT();
        throw PEGASUS_CIM_EXCEPTION(CIM_ERR_NOT_FOUND, instanceName.toString());
    }

    //
    // Load the instance from file:
    //

    CIMInstance cimInstance;

    if (!_loadInstance(dataFilePath, cimInstance, index, size))
    {
        PEG_METHOD_EXIT();
        throw CannotOpenFile(dataFilePath);
    }

    PEG_METHOD_EXIT();
    return cimInstance;
}

void FileBasedStore::createInstance(
    const CIMNamespaceName& nameSpace,
    const CIMObjectPath& instanceName,
    const CIMInstance& cimInstance,
    const Array<InstanceAssociation>& instAssocEntries)
{
    PEG_METHOD_ENTER(TRC_REPOSITORY, "FileBasedStore::createInstance");

    //
    // Get paths to data and index files:
    //

    String indexFilePath = _getInstanceIndexFilePath(
        nameSpace, cimInstance.getClassName());

    String dataFilePath = _getInstanceDataFilePath(
        nameSpace, cimInstance.getClassName());

    //
    // Perform the operation in a transaction scope to enable rollback on
    // failure.
    //

    InstanceTransactionHandler transaction(indexFilePath, dataFilePath);

    //
    // Save instance to file:
    //

    Uint32 index;
    Uint32 size;

    {
        Buffer data;
        _streamer->encode(data, cimInstance);
        size = data.size();

        if (!InstanceDataFile::appendInstance(dataFilePath, data, index))
        {
            PEG_METHOD_EXIT();
            throw PEGASUS_CIM_EXCEPTION_L(CIM_ERR_FAILED,
                MessageLoaderParms(
                    "Repository.CIMRepository.FAILED_TO_CREATE_INSTANCE",
                    "Failed to create instance: $0",
                    instanceName.toString()));
        }
    }

    //
    // Create entry in index file:
    //

    if (!InstanceIndexFile::createEntry(
            indexFilePath, instanceName, index, size))
    {
        PEG_METHOD_EXIT();
        throw PEGASUS_CIM_EXCEPTION_L(CIM_ERR_FAILED,
            MessageLoaderParms(
                "Repository.CIMRepository.FAILED_TO_CREATE_INSTANCE",
                "Failed to create instance: $0",
                instanceName.toString()));
    }

    transaction.complete();

    //
    // Create association entries if an association instance.
    //

    if (instAssocEntries.size())
    {
        _addInstanceAssociationEntries(nameSpace, instAssocEntries);
    }

    PEG_METHOD_EXIT();
}

void FileBasedStore::modifyInstance(
    const CIMNamespaceName& nameSpace,
    const CIMObjectPath& instanceName,
    const CIMInstance& cimInstance)
{
    PEG_METHOD_ENTER(TRC_REPOSITORY, "FileBasedStore::modifyInstance");

    //
    // Get paths of index and data files:
    //

    String indexFilePath = _getInstanceIndexFilePath(
        nameSpace, cimInstance.getClassName());

    String dataFilePath = _getInstanceDataFilePath(
        nameSpace, cimInstance.getClassName());

    //
    // Look up the specified instance
    //

    Uint32 oldSize;
    Uint32 oldIndex;
    Uint32 newSize;
    Uint32 newIndex;

    if (!InstanceIndexFile::lookupEntry(
            indexFilePath, instanceName, oldIndex, oldSize))
    {
        PEG_METHOD_EXIT();
        throw PEGASUS_CIM_EXCEPTION(CIM_ERR_NOT_FOUND, instanceName.toString());
    }

    //
    // Perform the operation in a transaction scope to enable rollback on
    // failure.
    //

    InstanceTransactionHandler transaction(indexFilePath, dataFilePath);

    //
    // Modify the data file:
    //

    {
        Buffer out;
        _streamer->encode(out, cimInstance);

        newSize = out.size();

        if (!InstanceDataFile::appendInstance(dataFilePath, out, newIndex))
        {
            PEG_METHOD_EXIT();
            throw PEGASUS_CIM_EXCEPTION_L(CIM_ERR_FAILED,
                MessageLoaderParms(
                    "Repository.CIMRepository.FAILED_TO_MODIFY_INSTANCE",
                    "Failed to modify instance $0",
                    instanceName.toString()));
        }
    }

    //
    // Modify the index file:
    //

    Uint32 freeCount;

    if (!InstanceIndexFile::modifyEntry(indexFilePath, instanceName, newIndex,
        newSize, freeCount))
    {
        PEG_METHOD_EXIT();
        throw PEGASUS_CIM_EXCEPTION_L(CIM_ERR_FAILED,
            MessageLoaderParms(
                "Repository.CIMRepository.FAILED_TO_MODIFY_INSTANCE",
                "Failed to modify instance $0",
                instanceName.toString()));
    }

    //
    // Compact the index and data files if the free count max was
    // reached.
    //

    if (freeCount >= _MAX_FREE_COUNT)
    {
        _CompactInstanceRepository(indexFilePath, dataFilePath);
    }

    transaction.complete();

    PEG_METHOD_EXIT();
}

void FileBasedStore::deleteInstance(
    const CIMNamespaceName& nameSpace,
    const CIMObjectPath& instanceName)
{
    PEG_METHOD_ENTER(TRC_REPOSITORY, "FileBasedStore::deleteInstance");

    //
    // Get paths of index and data files:
    //

    String indexFilePath = _getInstanceIndexFilePath(
        nameSpace, instanceName.getClassName());

    String dataFilePath = _getInstanceDataFilePath(
        nameSpace, instanceName.getClassName());

    //
    // Perform the operation in a transaction scope to enable rollback on
    // failure.
    //

    InstanceTransactionHandler transaction(indexFilePath, dataFilePath);

    //
    // Lookup instance from the index file (raise error if not found).
    //

    Uint32 index;
    Uint32 size;

    if (!InstanceIndexFile::lookupEntry(
            indexFilePath, instanceName, index, size))
    {
        PEG_METHOD_EXIT();
        throw PEGASUS_CIM_EXCEPTION(CIM_ERR_NOT_FOUND, instanceName.toString());
    }

    //
    // Remove entry from index file.
    //

    Uint32 freeCount;

    if (!InstanceIndexFile::deleteEntry(indexFilePath, instanceName, freeCount))
    {
        PEG_METHOD_EXIT();
        throw PEGASUS_CIM_EXCEPTION_L(CIM_ERR_FAILED,
            MessageLoaderParms(
                "Repository.CIMRepository.FAILED_TO_DELETE_INSTANCE",
                "Failed to delete instance: $0",
                instanceName.toString()));
    }

    //
    // Compact the index and data files if the free count max was
    // reached.
    //

    if (freeCount >= _MAX_FREE_COUNT)
    {
        _CompactInstanceRepository(indexFilePath, dataFilePath);
    }

    transaction.complete();

    //
    // Delete from association table (if an association).
    //

    _removeInstanceAssociationEntries(nameSpace, instanceName);

    PEG_METHOD_EXIT();
}

Boolean FileBasedStore::instanceExists(
    const CIMNamespaceName& nameSpace,
    const CIMObjectPath& instanceName)
{
    PEG_METHOD_ENTER(TRC_REPOSITORY, "FileBasedStore::instanceExists");

    String path =
        _getInstanceIndexFilePath(nameSpace, instanceName.getClassName());

    Uint32 index;
    Uint32 size;
    if (InstanceIndexFile::lookupEntry(path, instanceName, index, size))
    {
        PEG_METHOD_EXIT();
        return true;
    }

    PEG_METHOD_EXIT();
    return false;
}

void FileBasedStore::_addClassAssociationEntries(
    const CIMNamespaceName& nameSpace,
    const Array<ClassAssociation>& classAssocEntries)
{
    PEG_METHOD_ENTER(TRC_REPOSITORY,
        "FileBasedStore::_addClassAssociationEntries");

    String assocFileName = _getAssocClassPath(nameSpace);
    PEGASUS_STD(ofstream) os;

    if (!OpenAppend(os, assocFileName))
    {
        PEG_METHOD_EXIT();
        throw CannotOpenFile(assocFileName);
    }

    for (Uint32 i = 0; i < classAssocEntries.size(); i++)
    {
        _assocClassTable.append(
            os,
            assocFileName,
            classAssocEntries[i]);
    }

    PEG_METHOD_EXIT();
}

void FileBasedStore::_removeClassAssociationEntries(
    const CIMNamespaceName& nameSpace,
    const CIMName& assocClassName)
{
    PEG_METHOD_ENTER(TRC_REPOSITORY,
        "FileBasedStore::_removeClassAssociationEntries");

    String assocFileName = _getAssocClassPath(nameSpace);
    _assocClassTable.deleteAssociation(assocFileName, assocClassName);

    PEG_METHOD_EXIT();
}

void FileBasedStore::getClassAssociatorNames(
    const CIMNamespaceName& nameSpace,
    const Array<CIMName>& classList,
    const Array<CIMName>& assocClassList,
    const Array<CIMName>& resultClassList,
    const String& role,
    const String& resultRole,
    Array<String>& associatorNames)
{
    PEG_METHOD_ENTER(TRC_REPOSITORY,
        "FileBasedStore::getClassAssociatorNames");

    String assocFileName = _getAssocClassPath(nameSpace);

    // ATTN: Return value is ignored
    _assocClassTable.getAssociatorNames(
        assocFileName,
        classList,
        assocClassList,
        resultClassList,
        role,
        resultRole,
        associatorNames);

    PEG_METHOD_EXIT();
}

void FileBasedStore::getClassReferenceNames(
    const CIMNamespaceName& nameSpace,
    const Array<CIMName>& classList,
    const Array<CIMName>& resultClassList,
    const String& role,
    Array<String>& referenceNames)
{
    PEG_METHOD_ENTER(TRC_REPOSITORY, "FileBasedStore::getClassReferenceNames");

    String assocFileName = _getAssocClassPath(nameSpace);

    // ATTN: Return value is ignored
    _assocClassTable.getReferenceNames(
         assocFileName,
         classList,
         resultClassList,
         role,
         referenceNames);

    PEG_METHOD_EXIT();
}

void FileBasedStore::_addInstanceAssociationEntries(
    const CIMNamespaceName& nameSpace,
    const Array<InstanceAssociation>& instanceAssocEntries)
{
    PEG_METHOD_ENTER(TRC_REPOSITORY,
        "FileBasedStore::_addInstanceAssociationEntries");

    String assocFileName = _getAssocInstPath(nameSpace);
    PEGASUS_STD(ofstream) os;

    if (!OpenAppend(os, assocFileName))
    {
        PEG_METHOD_EXIT();
        throw CannotOpenFile(assocFileName);
    }

    for (Uint32 i = 0; i < instanceAssocEntries.size(); i++)
    {
        AssocInstTable::append(
            os,
            instanceAssocEntries[i].assocInstanceName,
            instanceAssocEntries[i].assocClassName,
            instanceAssocEntries[i].fromInstanceName,
            instanceAssocEntries[i].fromClassName,
            instanceAssocEntries[i].fromPropertyName,
            instanceAssocEntries[i].toInstanceName,
            instanceAssocEntries[i].toClassName,
            instanceAssocEntries[i].toPropertyName);
    }

    PEG_METHOD_EXIT();
}

void FileBasedStore::_removeInstanceAssociationEntries(
    const CIMNamespaceName& nameSpace,
    const CIMObjectPath& assocInstanceName)
{
    PEG_METHOD_ENTER(TRC_REPOSITORY,
        "FileBasedStore::_removeInstanceAssociationEntries");

    String assocFileName = _getAssocInstPath(nameSpace);
    AssocInstTable::deleteAssociation(assocFileName, assocInstanceName);

    PEG_METHOD_EXIT();
}

void FileBasedStore::getInstanceAssociatorNames(
    const CIMNamespaceName& nameSpace,
    const CIMObjectPath& instanceName,
    const Array<CIMName>& assocClassList,
    const Array<CIMName>& resultClassList,
    const String& role,
    const String& resultRole,
    Array<String>& associatorNames)
{
    PEG_METHOD_ENTER(TRC_REPOSITORY,
        "FileBasedStore::getInstanceAssociatorNames");

    String assocFileName = _getAssocInstPath(nameSpace);

    // ATTN: Return value is ignored.
    AssocInstTable::getAssociatorNames(
        assocFileName,
        instanceName,
        assocClassList,
        resultClassList,
        role,
        resultRole,
        associatorNames);

    PEG_METHOD_EXIT();
}

void FileBasedStore::getInstanceReferenceNames(
    const CIMNamespaceName& nameSpace,
    const CIMObjectPath& instanceName,
    const Array<CIMName>& resultClassList,
    const String& role,
    Array<String>& referenceNames)
{
    PEG_METHOD_ENTER(TRC_REPOSITORY,
        "FileBasedStore::getInstanceReferenceNames");

    String assocFileName = _getAssocInstPath(nameSpace);

    // ATTN: Return value is ignored.
    AssocInstTable::getReferenceNames(
        assocFileName,
        instanceName,
        resultClassList,
        role,
        referenceNames);

    PEG_METHOD_EXIT();
}

////////////////////////////////////////////////////////////////////////////////
//
// _SaveObject()
//
//      Saves objects (classes and qualifiers) from memory to
//      disk files.
//
////////////////////////////////////////////////////////////////////////////////

void FileBasedStore::_SaveObject(
    const String& path,
    Buffer& objectXml)
{
    PEG_METHOD_ENTER(TRC_REPOSITORY, "FileBasedStore::_SaveObject");

#ifdef PEGASUS_ENABLE_COMPRESSED_REPOSITORY
    if (_compressMode)            // PEP214
    {
        PEGASUS_STD(ostringstream) os;
        _streamer->write(os, objectXml);
        string str = os.str();

        gzFile fp = gzopen(path.getCString(), "wb");

        if (fp == NULL)
          throw CannotOpenFile(path);

        const char* ptr = str.data();
        size_t rem = str.size();
        int n;

        while (rem > 0 && (n = gzwrite(fp, (char*)ptr, rem)) > 0)
        {
            ptr += n;
            rem -= n;
        }

        gzclose(fp);
    }
    else
#endif /* PEGASUS_ENABLE_COMPRESSED_REPOSITORY */
    {
        PEGASUS_STD(ofstream) os(path.getCString() PEGASUS_IOS_BINARY);

        if (!os)
        {
            PEG_METHOD_EXIT();
            throw CannotOpenFile(path);
        }

        _streamer->write(os, objectXml);
    }
    PEG_METHOD_EXIT();
}

PEGASUS_NAMESPACE_END
