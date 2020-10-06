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

#include <fstream>
#include <Pegasus/Common/Config.h>
#include "InstanceDataFile.h"
#include <Pegasus/Common/System.h>
#include <Pegasus/Common/FileSystem.h>
#include <Pegasus/Common/Tracer.h>

PEGASUS_USING_STD;

PEGASUS_NAMESPACE_BEGIN

Boolean InstanceDataFile::_openFile(
    PEGASUS_STD(fstream)& fs,
    const String& path,
    int mode)
{
    PEG_METHOD_ENTER(TRC_REPOSITORY, "InstanceDataFile::_openFile()");
#if defined(__GNUC__) && GCC_VERSION >= 30200
    if (FileSystem::openNoCase(fs, path, PEGASUS_STD(ios_base::openmode)(mode)))
#else
    if (FileSystem::openNoCase(fs, path, mode))
#endif
    {
       PEG_METHOD_EXIT();
       return true;
    }
#if defined(__GNUC__) && GCC_VERSION >= 30200
    fs.open(path.getCString(), PEGASUS_STD(ios_base::openmode)(mode));
#else
    fs.open(path.getCString(), mode);
#endif
    PEG_METHOD_EXIT();
    return !!fs;
}

Boolean InstanceDataFile::loadInstance(
    const String& path,
    Uint32 index,
    Uint32 size,
    Buffer& data)
{
    PEG_METHOD_ENTER(TRC_REPOSITORY, "InstanceDataFile::loadInstance()");

    //
    // Open the file:
    //

    fstream fs;

    if (!_openFile(fs, path, ios::in PEGASUS_OR_IOS_BINARY))
    {
        PEG_METHOD_EXIT();
        return false;
    }

    //
    // Postion file pointer:
    //

    fs.seekg(index);

    if (!fs)
    {
        PEG_METHOD_EXIT();
        return false;
    }

    //
    // Read the instance:
    //

    data.grow(size, '\0');
    fs.read((char*)data.getData(), size);

    if (!fs)
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

Boolean InstanceDataFile::loadAllInstances(
    const String& path,
    Buffer& data)
{
    PEG_METHOD_ENTER(TRC_REPOSITORY, "InstanceDataFile::loadAllInstance()");

    //
    // Get size of the data file:
    //

    Uint32 fileSize;

    if (!FileSystem::getFileSizeNoCase(path, fileSize))
    {
        PEG_METHOD_EXIT();
        return false;
    }

    //
    // Open the file:
    //

    fstream fs;

    if (!_openFile(fs, path, ios::in PEGASUS_OR_IOS_BINARY))
    {
        PEG_METHOD_EXIT();
        return false;
    }

    //
    // Suck the entire contents of the file into the data array parameter:
    //

    data.grow(fileSize, '\0');
    fs.read((char*)data.getData(), fileSize);

    if (!fs)
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

Boolean InstanceDataFile::appendInstance(
    const String& path,
    const Buffer& data,
    Uint32& index)
{
    PEG_METHOD_ENTER(TRC_REPOSITORY, "InstanceDataFile::appendInstance()");

    //
    // Get size of the data file:
    //

    if (!FileSystem::getFileSizeNoCase(path, index))
        index = 0;

    //
    // Open the file for append:
    //

    fstream fs;

    if (!_openFile(fs, path, ios::app | ios::out PEGASUS_OR_IOS_BINARY))
    {
        PEG_METHOD_EXIT();
        return false;
    }

    //
    // Save index to data:
    //

    // index = fs.tellp();

    //
    // Write the instance:
    //

    fs.write(data.getData(), data.size());

    if (!fs)
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

Boolean InstanceDataFile::beginTransaction(const String& path)
{
    PEG_METHOD_ENTER(TRC_REPOSITORY, "InstanceDataFile::beginTransaction()");

    Uint32 fileSize;

    //
    // If the file does not exist, then set the file size to zero:
    //

    if (!FileSystem::existsNoCase(path))
    {
        fileSize = 0;
    }
    else
    {
        if (!FileSystem::getFileSizeNoCase(path, fileSize))
        {
            PEG_METHOD_EXIT();
            return false;
        }
    }

    //
    // Open the rollback file:
    //

    // ATTN-SF-P3-20020517: FUTURE: Need to look in to this. Empty rollback
    // files are getting created in some error conditions.

    fstream fs;
    String rollbackPath = path;
    rollbackPath.append(".rollback");

    do
    {
        if (!_openFile(fs, rollbackPath, ios::out PEGASUS_OR_IOS_BINARY))
        {
            break;
        }

        //
        // Save the size of the data file in the rollback file.
        //

        char buffer[9];
        sprintf(buffer, "%08x", fileSize);
        fs.write(buffer, static_cast<streamsize>(strlen(buffer)));

        if (!fs)
        {
            break;
        }

        //
        // If the badbit is set, it indicates error in the file write operation.
        //
        if (fs.bad())
        {
            //
            // Close the file
            //

            fs.close();
            break;
        }

        //
        // Close the file.
        //

        FileSystem::syncWithDirectoryUpdates(fs);
        fs.close();

        PEG_METHOD_EXIT();
        return true;
    }
    while(0);

    // Undo the begin transaction for instance data file
    undoBeginTransaction(path);

    PEG_METHOD_EXIT();
    return false;
}

void InstanceDataFile::undoBeginTransaction(const String& path)
{
    PEG_METHOD_ENTER(TRC_REPOSITORY,"InstanceDataFile::undoBeginTransaction()");

    String rollbackPath = path;
    rollbackPath.append(".rollback");

    // Remove the incorrect instance data rollback file as a part of cleanup
    FileSystem::removeFileNoCase(rollbackPath);

    PEG_METHOD_EXIT();
}

Boolean InstanceDataFile::rollbackTransaction(const String& path)
{
    PEG_METHOD_ENTER(TRC_REPOSITORY, "InstanceDataFile::rollbackTransaction()");

    //
    // If rollback file does not exist, then everything is fine, just
    // return.
    //

    if (!FileSystem::existsNoCase(path + ".rollback"))
    {
        PEG_METHOD_EXIT();
        return true;
    }

    //
    // Open the rollback file:
    //

    fstream rollbackFs;

    if (!_openFile(
            rollbackFs, path + ".rollback", ios::in PEGASUS_OR_IOS_BINARY))
    {
        PEG_METHOD_EXIT();
        return false;
    }

    //
    // Retrieve the file size from the rollback file:
    //

    char buffer[9];
    rollbackFs.read(buffer, 8);

    if (!rollbackFs)
    {
        PEG_METHOD_EXIT();
        return false;
    }

    buffer[8] = '\0';

    char* end = 0;
    long fileSize = strtol(buffer, &end, 16);

    if (!end || *end != '\0' || fileSize < 0)
    {
        PEG_METHOD_EXIT();
        return false;
    }

    rollbackFs.close();

    //
    // If the instance data file does not yet exist, create it
    //

    if (fileSize == 0)
    {
        fstream ofs;

        if (!_openFile(ofs, path, ios::out PEGASUS_OR_IOS_BINARY))
        {
            PEG_METHOD_EXIT();
            return false;
        }

        ofs.close();
    }

    //
    // Truncate the data file to its initial size
    //

    if (!System::truncateFile(path.getCString(), fileSize))
    {
        PEG_METHOD_EXIT();
        return false;
    }

    //
    // Now get rid of rollback file!
    //

    PEG_METHOD_EXIT();
    return FileSystem::removeFileNoCase(path + ".rollback");
}

Boolean InstanceDataFile::commitTransaction(const String& path)
{
    PEG_METHOD_ENTER(TRC_REPOSITORY, "InstanceDataFile::commitTransaction()");

    //
    // To commit the transaction, we simply remove the rollback file:
    //

    PEG_METHOD_EXIT();
    return FileSystem::removeFileNoCase(path + ".rollback");
}

Boolean InstanceDataFile::compact(
    const String& path,
    const Array<Uint32>& freeFlags,
    const Array<Uint32>& indices,
    const Array<Uint32>& sizes)
{
    PEG_METHOD_ENTER(TRC_REPOSITORY, "InstanceDataFile::compact()");

    //
    // Open the input file (the data file):
    //

    fstream fs;

    if (!_openFile(fs, path, ios::in PEGASUS_OR_IOS_BINARY))
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

    if (!_openFile(tmpFs, outputFilePath, ios::out PEGASUS_OR_IOS_BINARY))
    {
        PEG_METHOD_EXIT();
        return false;
    }

    //
    // Copy over instances which have not been freed:
    //

    Buffer data;

    for (Uint32 i = 0, n = freeFlags.size(); i < n; i++)
    {
        //
        // If this entry is not free, then copy it over to the output file.
        //

        if (!freeFlags[i])
        {
            //
            // Read the next instance:
            //

            if (!fs.seekg(indices[i]))
            {
                FileSystem::removeFile(outputFilePath);
                PEG_METHOD_EXIT();
                return false;
            }

            data.grow(sizes[i], '\0');

            fs.read((char*)data.getData(), sizes[i]);

            if (!fs)
            {
                FileSystem::removeFile(outputFilePath);
                PEG_METHOD_EXIT();
                return false;
            }

            //
            //  Write out the next instance:
            //

            tmpFs.write(data.getData(), sizes[i]);
        }
    }

    //
    // Close the files:
    //

    fs.close();

    FileSystem::syncWithDirectoryUpdates(tmpFs);
    tmpFs.close();

    //
    // Copy the new file over the old one:
    //

    PEG_METHOD_EXIT();
    return FileSystem::renameFile(outputFilePath, path);
}

PEGASUS_NAMESPACE_END
