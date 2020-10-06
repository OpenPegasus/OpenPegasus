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
#include <fstream>
#include <cctype>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <Pegasus/Common/FileSystem.h>
#include <Pegasus/Common/Tracer.h>
#include "InstanceIndexFile.h"

PEGASUS_USING_STD;

PEGASUS_NAMESPACE_BEGIN

////////////////////////////////////////////////////////////////////////////////
//
// Local routines:
//
////////////////////////////////////////////////////////////////////////////////

#ifdef PEGASUS_OS_ZOS

static Uint32 getOffset( streampos sp )
{
    Uint32 result = (streamoff)sp;

    fpos_t posArray = sp.seekpos();


    result += posArray.__fpos_elem[1];

    return result;
}

#endif

//
// Converts a CIMObjectPath to a form that can be used as a key in the index
// file.  Newline and carriage return characters are escaped to prevent
// problems with the line-based file format.
//

static String _convertInstanceNameToKey(const CIMObjectPath& instanceName)
{
    String instanceNameString(instanceName.toString());
    const Uint32 stringLength = instanceNameString.size();
    String keyString;
    keyString.reserveCapacity(stringLength);

    for (Uint32 i = 0; i < stringLength; i++)
    {
        Char16 instanceNameChar = instanceNameString[i];

        if (instanceNameChar == '\n')
        {
            keyString.append("\\n");
        }
        else if (instanceNameChar == '\r')
        {
            keyString.append("\\r");
        }
        else
        {
            keyString.append(instanceNameChar);
        }
    }

    return keyString;
}

//
// Converts an index file key to a CIMObjectPath object.  Newline and
// carriage return characters are un-escaped.
//

static CIMObjectPath _convertKeyToInstanceName(const char* key)
{
    String keyString(key);

    for (Uint32 i = 0; i < keyString.size() - 1; i++)
    {
        if (keyString[i] == '\\')
        {
            if (keyString[i+1] == 'n')
            {
                keyString[i] = '\n';
                keyString.remove(i+1, 1);
            }
            else if (keyString[i+1] == 'r')
            {
                keyString[i] = '\r';
                keyString.remove(i+1, 1);
            }
            else
            {
                i++;
            }
        }
    }

    return CIMObjectPath(keyString);
}

inline void _SkipWhitespace(char*& p)
{
    while (*p && isspace(*p))
        p++;
}

//
// Get an integer field from the character pointer and advance the
// pointer past the field.
//

Boolean _GetIntField(
    const char*& ptr,
    Boolean& errorOccurred,
    Uint32& value,
    int base)
{
    char* end = 0;
    value = strtoul(ptr, &end, base);

    errorOccurred = false;

    if (!end)
    {
        errorOccurred = true;
        return false;
    }

    _SkipWhitespace(end);

    if (*end == '\0')
    {
        errorOccurred = true;
        return false;
    }

    ptr = end;
    return true;
}

//
// Gets the next record in the index file.
//

static Boolean _GetNextRecord(
    fstream& fs,
    Buffer& line,
    Uint32& freeFlag,
    Uint32& hashCode,
    Uint32& index,
    Uint32& size,
    const char*& instanceName,
    Boolean& errorOccurred)
{
    errorOccurred = false;

    //
    // Get next line:
    //

    if (!GetLine(fs, line))
        return false;

    //
    // Get the free flag field:
    //

    const char* end = (char*)line.getData();

    if (!_GetIntField(end, errorOccurred, freeFlag, 10))
        return false;

    if (freeFlag != 0 && freeFlag != 1)
    {
        errorOccurred = true;
        return false;
    }

    //
    // Get the hash-code field:
    //

    if (!_GetIntField(end, errorOccurred, hashCode, 16))
        return false;

    //
    // Get index field:
    //

    if (!_GetIntField(end, errorOccurred, index, 10))
        return false;

    //
    // Get size field:
    //

    if (!_GetIntField(end, errorOccurred, size, 10))
        return false;

    //
    // Get instance name:
    //

    instanceName = end;

    return true;
}

////////////////////////////////////////////////////////////////////////////////
//
// InstanceIndexFile:
//
////////////////////////////////////////////////////////////////////////////////

Boolean InstanceIndexFile::lookupEntry(
    const String& path,
    const CIMObjectPath& instanceName,
    Uint32& indexOut,
    Uint32& sizeOut)
{
    PEG_METHOD_ENTER(TRC_REPOSITORY, "InstanceIndexFile::lookupEntry()");

    fstream fs;

    if (!_openFile(path, fs))
    {
        PEG_METHOD_EXIT();
        return false;
    }

    Uint32 entryOffset = 0;

    Boolean result = _lookupEntry(
        fs, instanceName, indexOut, sizeOut, entryOffset);

    fs.close();

    PEG_METHOD_EXIT();
    return result;
}

Boolean InstanceIndexFile::createEntry(
    const String& path,
    const CIMObjectPath& instanceName,
    Uint32 indexIn,
    Uint32 sizeIn)
{
    PEG_METHOD_ENTER(TRC_REPOSITORY, "InstanceIndexFile::createEntry()");

    //
    // Open the file:
    //

    fstream fs;

    if (!_openFile(path, fs, true))
    {
        PEG_METHOD_EXIT();
        return false;
    }

    //
    // Return false if entry already exists:
    //

    Uint32 tmpIndex;
    Uint32 tmpSize;
    Uint32 tmpEntryOffset;

    if (InstanceIndexFile::_lookupEntry(
        fs, instanceName, tmpIndex, tmpSize, tmpEntryOffset))
    {
        PEG_METHOD_EXIT();
        return false;
    }

    //
    // Append the new entry to the end of the file:
    //

    if (!_appendEntry(fs, instanceName, indexIn, sizeIn))
    {
        PEG_METHOD_EXIT();
        return false;
    }

    //
    // Close the file:
    //

    fs.close();

    PEG_METHOD_EXIT();
    return true;
}

Boolean InstanceIndexFile::deleteEntry(
    const String& path,
    const CIMObjectPath& instanceName,
    Uint32& freeCount)
{
    PEG_METHOD_ENTER(TRC_REPOSITORY, "InstanceIndexFile::deleteEntry()");

    freeCount = 0;

    //
    // Open the file:
    //

    fstream fs;

    if (!_openFile(path, fs))
    {
        PEG_METHOD_EXIT();
        return false;
    }

    //
    // Mark the entry as free:
    //

    if (!_markEntryFree(fs, instanceName))
    {
        PEG_METHOD_EXIT();
        return false;
    }

    //
    // Increment the free count:
    //

    freeCount = 0;

    if (!_incrementFreeCount(fs, freeCount))
    {
        PEG_METHOD_EXIT();
        return false;
    }

    //
    // Close the file:
    //

    fs.close();

    PEG_METHOD_EXIT();
    return true;
}

Boolean InstanceIndexFile::modifyEntry(
    const String& path,
    const CIMObjectPath& instanceName,
    Uint32 indexIn,
    Uint32 sizeIn,
    Uint32& freeCount)
{
    PEG_METHOD_ENTER(TRC_REPOSITORY, "InstanceIndexFile::modifyEntry()");

    //
    // Open the file:
    //

    fstream fs;

    if (!_openFile(path, fs))
    {
        PEG_METHOD_EXIT();
        return false;
    }

    //
    // Mark the entry as free:
    //

    if (!_markEntryFree(fs, instanceName))
    {
        PEG_METHOD_EXIT();
        return false;
    }

    //
    // Append new entry:
    //

    if (!_appendEntry(fs, instanceName, indexIn, sizeIn))
    {
        PEG_METHOD_EXIT();
        return false;
    }

    //
    // Increment the free count:
    //

    freeCount = 0;

    if (!_incrementFreeCount(fs, freeCount))
    {
        PEG_METHOD_EXIT();
        return false;
    }

    //
    // Close the file:
    //

    fs.close();

    PEG_METHOD_EXIT();
    return true;
}

Boolean InstanceIndexFile::enumerateEntries(
    const String& path,
    Array<Uint32>& freeFlags,
    Array<Uint32>& indices,
    Array<Uint32>& sizes,
    Array<CIMObjectPath>& instanceNames,
    Boolean includeFreeEntries)
{
    PEG_METHOD_ENTER(TRC_REPOSITORY, "InstanceIndexFile::enumerateEntries()");

    //
    // Reserve space for at least COUNT entries:
    //

    const Uint32 COUNT = 1024;

    freeFlags.reserveCapacity(COUNT);
    indices.reserveCapacity(COUNT);
    sizes.reserveCapacity(COUNT);
    instanceNames.reserveCapacity(COUNT);

    //
    // Open input file:
    //

    fstream fs;

    if (!_openFile(path, fs))
    {
        // file does not exist, just return with no instanceNames
        PEG_METHOD_EXIT();
        return true;
    }

    //
    // Iterate over all instances to build output arrays:
    //

    Buffer line;
    Uint32 freeFlag;
    Uint32 hashCode;
    const char* instanceName;
    Uint32 index;
    Uint32 size;
    Boolean errorOccurred;

    while (_GetNextRecord(
        fs, line, freeFlag, hashCode, index, size, instanceName, errorOccurred))
    {
        if (!freeFlag || includeFreeEntries)
        {
            freeFlags.append(freeFlag);
            indices.append(index);
            sizes.append(size);
            instanceNames.append(_convertKeyToInstanceName(instanceName));
        }
    }

    if (errorOccurred)
    {
        PEG_METHOD_EXIT();
        return false;
    }

    PEG_METHOD_EXIT();
    return true;
}

Boolean InstanceIndexFile::_incrementFreeCount(
    PEGASUS_STD(fstream)& fs,
    Uint32& freeCount)
{
    PEG_METHOD_ENTER(TRC_REPOSITORY,
        "InstanceIndexFile::_incrementFreeCount()");

    //
    // Position file pointer to beginning of file (where free count is
    // located) and read the current free count.
    //

    fs.seekg(0);
    char hexString[9];
    fs.read(hexString, 8);

    if (!fs)
    {
        PEG_METHOD_EXIT();
        return false;
    }

    hexString[8] = '\0';

    //
    // Convert hex string to integer:
    //

    char* end = 0;
    long tmp = strtol(hexString, &end, 16);

    if (!end || *end != '\0' || tmp < 0)
    {
        PEG_METHOD_EXIT();
        return false;
    }

    freeCount = Uint32(tmp);

    //
    // Increment and rewrite the free count:
    //

    sprintf(hexString, "%08X", ++freeCount);
    fs.seekg(0);
    fs.write(hexString, 8);

    PEG_METHOD_EXIT();
    return !!fs;
}

Boolean InstanceIndexFile::_openFile(
    const String& path,
    PEGASUS_STD(fstream)& fs,
    Boolean create)
{
    PEG_METHOD_ENTER(TRC_REPOSITORY, "InstanceIndexFile::_openFile()");

    const char ZERO_FREE_COUNT[] = "00000000\n";

    //
    // Open the file:
    //

    if (!FileSystem::openNoCase(
            fs, path, ios::in | ios::out PEGASUS_OR_IOS_BINARY))
    {
        if (create)
        {
            //
            // File does not exist so create it:
            //
            fs.open(path.getCString(), ios::out PEGASUS_OR_IOS_BINARY);

            if (!fs)
            {
                PEG_METHOD_EXIT();
                return false;
            }

            fs.write(ZERO_FREE_COUNT, sizeof(ZERO_FREE_COUNT) - 1);
            fs.close();

            //
            // Reopen the file:
            //

            if (!FileSystem::openNoCase(
                    fs, path, ios::in | ios::out PEGASUS_OR_IOS_BINARY))
            {
                PEG_METHOD_EXIT();
                return false;
            }
        }
        else
        {
            PEG_METHOD_EXIT();
            return false;
        }
    }

    //
    // Position the file pointer beyond the free count:
    //

    fs.seekg(sizeof(ZERO_FREE_COUNT) - 1);

    PEG_METHOD_EXIT();
    return true;
}

Boolean InstanceIndexFile::_appendEntry(
    PEGASUS_STD(fstream)& fs,
    const CIMObjectPath& instanceName,
    Uint32 indexIn,
    Uint32 sizeIn)
{
    PEG_METHOD_ENTER(TRC_REPOSITORY, "InstanceIndexFile::_appendEntry()");

    //
    // Position the file at the end:
    //

    fs.seekg(0, ios::end);

    if (!fs)
    {
        PEG_METHOD_EXIT();
        return false;
    }

    //
    // Write the entry:
    //

    Uint32 targetHashCode = instanceName.makeHashCode();

    char buffer[32];
    sprintf(buffer, "%08X", targetHashCode);

    fs << "0 " << buffer << ' ' << indexIn << ' ' << sizeIn << ' ';

    // Calling getCString to ensure that utf-8 goes to the file
    // Calling write to ensure no data conversion by the stream
    CString name = _convertInstanceNameToKey(instanceName).getCString();
    fs.write((const char *)name,
        static_cast<streamsize>(strlen((const char *)name)));
    fs << endl;

    PEG_METHOD_EXIT();
    return !!fs;
}

Boolean InstanceIndexFile::_markEntryFree(
    PEGASUS_STD(fstream)& fs,
    const CIMObjectPath& instanceName)
{
    PEG_METHOD_ENTER(TRC_REPOSITORY, "InstanceIndexFile::_markEntryFree()");

    //
    // First look up the entry:
    //

    Uint32 index = 0;
    Uint32 size = 0;
    Uint32 entryOffset = 0;

    if (!InstanceIndexFile::_lookupEntry(
        fs, instanceName, index, size, entryOffset))
    {
        PEG_METHOD_EXIT();
        return false;
    }

    //
    // Now mark the entry as free (change the first character of the entry
    // from a '0' to a '1').
    //

    fs.seekg(entryOffset);

    if (!fs)
    {
        PEG_METHOD_EXIT();
        return false;
    }

    fs.write("1", 1);

    PEG_METHOD_EXIT();
    return !!fs;
}

Boolean InstanceIndexFile::_lookupEntry(
    PEGASUS_STD(fstream)& fs,
    const CIMObjectPath& instanceName,
    Uint32& indexOut,
    Uint32& sizeOut,
    Uint32& entryOffset)
{
    PEG_METHOD_ENTER(TRC_REPOSITORY, "InstanceIndexFile::_lookupEntry()");

    indexOut = 0;
    sizeOut = 0;
    entryOffset = 0;

    Uint32 targetHashCode = instanceName.makeHashCode();
    Buffer line;
    Uint32 freeFlag;
    Uint32 hashCode;
    const char* instanceNameTmp;
    Uint32 index;
    Uint32 size;
    Boolean errorOccurred;
#ifndef PEGASUS_OS_ZOS
        entryOffset = (Uint32)fs.tellp();
#else
        entryOffset = getOffset(fs.tellp());
#endif

    while (_GetNextRecord(
        fs, line, freeFlag, hashCode, index,
        size, instanceNameTmp, errorOccurred))
    {

#ifdef PEGASUS_REPOSITORY_NOT_NORMALIZED
        // See bugzilla 1207.  If the object paths in the repository
        // are not normalized, then the hashcodes cannot be used for
        // the look up (because the hash is based on the normalized path).
        if (freeFlag == 0 &&
            _convertKeyToInstanceName(instanceNameTmp) == instanceName)
#else
        if (freeFlag == 0 &&
            hashCode == targetHashCode &&
            _convertKeyToInstanceName(instanceNameTmp) == instanceName)
#endif
        {
            indexOut = index;
            sizeOut = size;
            PEG_METHOD_EXIT();
            return true;
        }

#ifndef PEGASUS_OS_ZOS
        entryOffset = (Uint32)fs.tellp();
#else
        entryOffset = getOffset(fs.tellp());
#endif
    }

    fs.clear();

    PEG_METHOD_EXIT();
    return false;
}

Boolean InstanceIndexFile::compact(
    const String& path)
{
    PEG_METHOD_ENTER(TRC_REPOSITORY, "InstanceIndexFile::compact()");

    //
    // Open input file:
    //

    fstream fs;

    if (!_openFile(path, fs))
    {
        PEG_METHOD_EXIT();
        return false;
    }

    //
    // Open the output file (temporary data file):
    //

    const String outputFilePath = path + ".tmp";
    String leftoverOutputFilePath;

    // Ensure the output file does not already exist
    if (FileSystem::existsNoCase(outputFilePath, leftoverOutputFilePath))
    {
        if (!FileSystem::removeFile(leftoverOutputFilePath))
        {
            PEG_METHOD_EXIT();
            return false;
        }
    }

    fstream tmpFs;

    if (!_openFile(outputFilePath, tmpFs, true))
    {
        PEG_METHOD_EXIT();
        return false;
    }

    //
    // Iterate over all instances to build output arrays:
    //

    Buffer line;
    Uint32 freeFlag;
    Uint32 hashCode;
    const char* instanceName;
    Uint32 index;
    Uint32 size;
    Boolean errorOccurred;
    Uint32 adjust = 0;

    while (_GetNextRecord(
        fs, line, freeFlag, hashCode, index, size, instanceName, errorOccurred))
    {
        //
        // Copy the entry over to the temporary file if it is not free.
        // Otherwise, discard the entry and update subsequent indices to
        // compensate for removal of this block.
        //

        if (freeFlag)
        {
            adjust += size;
        }
        else
        {
            if (!_appendEntry(tmpFs, _convertKeyToInstanceName(instanceName),
                index - adjust, size))
            {
                errorOccurred = true;
                break;
            }
        }
    }

    //
    // Close both files:

    fs.close();

    FileSystem::syncWithDirectoryUpdates(tmpFs);
    tmpFs.close();

    //
    // If an error occurred, remove the temporary file and
    // return false.
    //

    if (errorOccurred)
    {
        FileSystem::removeFile(outputFilePath);
        PEG_METHOD_EXIT();
        return false;
    }

    //
    // Replace index file with temporary file:
    //

    PEG_METHOD_EXIT();
    return FileSystem::renameFile(outputFilePath, path);
}

Boolean InstanceIndexFile::hasNonFreeEntries(const String& path)
{
    //
    // If file does not exist, there are no instances:
    //

    if (!FileSystem::existsNoCase(path))
        return false;

    //
    // We must iterate all the entries looking for a non-free one:
    //

    Array<Uint32> freeFlags;
    Array<Uint32> indices;
    Array<Uint32> sizes;
    Array<CIMObjectPath> instanceNames;

    if (!InstanceIndexFile::enumerateEntries(
        path, freeFlags, indices, sizes, instanceNames, false))
    {
        // This won't happen!
        return false;
    }

    return freeFlags.size() != 0;
}

Boolean InstanceIndexFile::beginTransaction(const String& path)
{
    PEG_METHOD_ENTER(TRC_REPOSITORY, "InstanceIndexFile::beginTransaction()");

    String rollbackPath = path;
    rollbackPath.append(".rollback");

    //
    // If the index file does not exist, then create a rollback file with
    // freecount of 0.
    //
    if (!FileSystem::existsNoCase(path))
    {
        // Make sure the rollback file does not exist.
        if (FileSystem::existsNoCase(rollbackPath))
        {
            if (!FileSystem::removeFileNoCase(rollbackPath))
            {
                PEG_METHOD_EXIT();
                return false;
            }
        }

        // Create the rollback file, and write the freecount of 0.
        fstream fs;
        if (!_openFile(rollbackPath, fs, true))
        {
            // Make sure no rollback file is left over.
            FileSystem::removeFileNoCase(rollbackPath);

            PEG_METHOD_EXIT();
                return false;
        }
        fs.close();

        PEG_METHOD_EXIT();
        return true;
    }

    //
    // Create a rollback file which is a copy of the index file. The
    // new filename is formed by appending ".rollback" to the name of
    // the index file.  The rollback file, if it exists, is considered
    // the "master" copy of the data.  To ensure its completeness, the
    // index file is renamed to the rollback file and the data is then
    // copied back to the index file.
    //

    do
    {
        if (!FileSystem::renameFileNoCase(path, rollbackPath))
        {
            break;
        }

        if (!FileSystem::copyFile(rollbackPath, path))
        {
            break;
        }

        PEG_METHOD_EXIT();
        return true;
    }
    while(0);

    // Try to restore the initial state
    undoBeginTransaction(path);

    PEG_METHOD_EXIT();
    return false;
}

void InstanceIndexFile::undoBeginTransaction(const String& path)
{
    PEG_METHOD_ENTER(TRC_REPOSITORY,
        "InstanceIndexFile::undoBeginTransaction()");

    String rollbackPath = path;
    rollbackPath.append(".rollback");

    //
    // Remove the original index file and
    // Rename the rollback file to the original file
    // If the rollback file is present, this function has no effect
    //
    if(FileSystem::existsNoCase(rollbackPath))
    {
        FileSystem::removeFileNoCase(path);
        FileSystem::renameFileNoCase(rollbackPath, path);
    }

    PEG_METHOD_EXIT();
}

Boolean InstanceIndexFile::rollbackTransaction(const String& path)
{
    PEG_METHOD_ENTER(TRC_REPOSITORY,
        "InstanceIndexFile::rollbackTransaction()");

    //
    // If the rollback file does not exist, then everything is fine (nothing
    // to roll back).
    //

    if (!FileSystem::existsNoCase(path + ".rollback"))
    {
        PEG_METHOD_EXIT();
        return true;
    }

    //
    // To roll back, simply rename the rollback file over the index file.
    //

    PEG_METHOD_EXIT();
    return FileSystem::renameFileNoCase(path + ".rollback", path);
}

Boolean InstanceIndexFile::commitTransaction(const String& path)
{
    PEG_METHOD_ENTER(TRC_REPOSITORY, "InstanceIndexFile::commitTransaction()");

    //
    // To commit, simply remove the rollback file:
    //

    String rollbackPath = path;
    rollbackPath.append(".rollback");

    PEG_METHOD_EXIT();
    return FileSystem::removeFileNoCase(rollbackPath);
}

PEGASUS_NAMESPACE_END
