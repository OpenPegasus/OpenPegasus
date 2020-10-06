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

#ifndef Pegasus_FileSystem_h
#define Pegasus_FileSystem_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/String.h>
#include <Pegasus/Common/Buffer.h>
#include <Pegasus/Common/ArrayInternal.h>
#include <Pegasus/Common/InternalException.h>
#include <Pegasus/Common/System.h>
#include <Pegasus/Common/Linkage.h>
#include <fstream>
#include <cstdio>
#include <Pegasus/Common/Buffer.h>

PEGASUS_NAMESPACE_BEGIN

/** The FileSystem class provides methods for manipulating the file system.

    This class provides an methods for:
    <ul>
        <li>Manipulating directories (create, remove, change).</li>
        <li>Checking files for ability to read and write.</li>
        <li>Removing files.</li>
        <li>Comparing files.</li>
        <li>Loading files into memory.</li>
    </ul>

    The methods of this class are all static. So there is no need to
    instantiate this class to use it. In fact, instantiation is precluded
    by a private default constructor.

    A word about the "NoCase" extensions. Some methods of this class have
    a "NoCase" version. For example, there is a canRead() and a canReadNoCase().
    The "NoCase" variation ignores the case of the file while it is being
    located. For example, suppose there is a file called "File1". Then
    canReadNoCase("file1") finds the file called "File1" and returns true (of
    course there is a possibility that there really is a file called "file1"
    in the same directory in which case the behavior of this method is
    undefined). Notice that Windows does this anyway. These methods were
    developed primarily for Unix which is case sensitive with respect to file
    names. It should be noted that the no-case methods are slower (since they
    must stat the directory and look at the file names).

    The no-case variations are used by the repository which--according to
    CIM--must treat two classes names with different case characterisits, but
    othererwise similar, as identical. For example, "MyClass", "myclass", and
    "MYCLASS" all refer to the same class. Since the default repository
    implementation uses disk file names to represent class names (e.g., there
    may be a file called "MyClass.#) that there must be a way of opening
    a file without regard to its case.
*/
class PEGASUS_COMMON_LINKAGE FileSystem
{
public:

    /** Determines whether file exists.
        @param path path of the file.
        @return true if the file exists.
    */
    static Boolean exists(const String& path);

    /** Determine whether the file exists. Ignores case of the file.
        @param path path of the file.
        @param pathOut path of the file with actual case.
        @return true if the file exists; false otherwise.
    */
    static Boolean existsNoCase(const String& path, String& pathOut);

    /** Determine whether the file exists. Ignores the case of the file.
        @param path path of the file.
        @return true if the file exists; false otherwise.
    */
    static Boolean existsNoCase(const String& path);

    /** Determines whether the file can be read.
        @param path path of the file.
        @return true if the file can be read.
    */
    static Boolean canRead(const String& path);

    /** Determines whether the file can be read. Ignores case of file.
        @param path path of the file.
        @return true if the file can be read.
    */
    static Boolean canReadNoCase(const String& path);

    /** Determines whether the file can be written.
        @param path path of the file.
        @return true if the file can be written.
    */
    static Boolean canWrite(const String& path);

    /** Determines whether the file can be written. Ignores case of file.
        @param path path of the file.
        @return true if the file can be written.
    */
    static Boolean canWriteNoCase(const String& path);

    /** Get the size of the file in bytes.
        @param path path of file.
        @param size set to size of file.
        @return true on success.
    */
    static Boolean getFileSize(const String& path, Uint32& size);

    /** Get the size of the file in bytes.
        @param path path of file.
        @param size set to size of file.
        @return true on success.
    */
    static Boolean getFileSizeNoCase(const String& path, Uint32& size);

    /** Removes a file.
        @param path of file to be removed.
        @return true on sucess.
    */
    static Boolean removeFile(const String& path);

    /** Removes a file. Ignores case of file.
        @param path of file to be removed.
        @return true on sucess.
    */
    static Boolean removeFileNoCase(const String& path);

    /** Produces an array of filenames that match the given pattern under
        the directory given by path. The pattern is limited to asterisks
        only. Examples: "*.txt", "hello*world.c". Returns true on success.
        Return false if the base diretory does not exist or cannot be
        accessed.
    */
    static Boolean glob(
        const String& path,
        const String& pattern,
        Array<String>& filenames);

    /** Loads contents of the file into the array. Note that the file is
        opened using binary mode (newline sequences are not expanded to
        carriage-return-line-feed sequences on Windows).
        @param array set to the contents of the file upon return.
        @param fileName name of file to be loaded.
        @exception CannotOpenFile
    */
    static void loadFileToMemory(
        Buffer& array,
        const String& fileName);

    /** Determines whether two files have exactly the same content.
        @param path1 path of first file.
        @param path2 path of second file.
        @return true if files are identical.
        @exception CannotOpenFile
    */
    static Boolean compareFiles(
        const String& path1,
        const String& path2);

    /**
        Renames a file.  If the new name refers to an existing file, it is
        removed and replaced with the renamed file.  The rename operation is
        performed atomically.
        @param oldPath A String containing the name of the file to rename.
        @param newPath A String containing the name to which to rename the file.
        @return A Boolean indicating whether the rename operation was
            successful.
    */
    static Boolean renameFile(
        const String& oldPath,
        const String& newPath);

    /** Same as rename file except that the case of the file referred to
        by oldPath is ignored.  The case resolution of the oldPath is
        performed prior to the atomic rename operation.
    */
    static Boolean renameFileNoCase(
        const String& oldPath,
        const String& newPath);

    /** Copy a file.
        @param fromPath name of existing file.
        @param toPath name of new file.
        @return true on success.
    */
    static Boolean copyFile(
        const String& fromPath,
        const String& toPath);

    /** Opens a file and ignores the case of the file. Note that the file
        will be opend in binary mode (no translation of carriage-return-line-
        feed sequences on Windows).
        @param os file stream to be opend.
        @param path path of file to be opened.
        @return true on success.
    */
    static Boolean openNoCase(PEGASUS_STD(ifstream)& is, const String& path);

    /** Opens a file and ignores the case of the file. Note that the file
        open mode of the file must be passed in.
        @param os file stream to be opend.
        @param path path of file to be opened.
        @param mode mode to open the file in.
        @return true on success.
    */
    static Boolean openNoCase(
        PEGASUS_STD(fstream)& fs,
        const String& path,
        int mode);

    /** Determines whether the path refers to a directory.
        @param path path of the directory.
        @return true if path refers to a directory.
    */
    static Boolean isDirectory(const String& path);

    /** Changes the current directory.
        @param path path of directory to be changed to.
        @return true on success.
    */
    static Boolean changeDirectory(const String& path);

    /** Creates a directory.
        @param path path of directory to be created.
        @return true on success.
    */
    static Boolean makeDirectory(const String& path);

    /** Get the path of the current working Directory.
        @param path set to current working directory upon return.
        @return true on success (operation may fail if the current
            working directory becomes stale; this can happen on
            Unix if it is removed but is impossible on Windows
            due to reference counting).
    */
    static Boolean getCurrentDirectory(String& path);

    /** Remove the given directory. The directory must be empty
        to be eligible for removal
        @param String path is the relative or ablsolute path to
        the directory to remove
        @return true if directory removed
    */
    static Boolean removeDirectory(const String& path);

    /** Remove a directory and all files and directories under it.
        WARNING: This differs significantly from the <tt>removeDirectory</tt>
        function in that it removes both directories and files and
        removes a complete hiearchy.  Use with caution.
        @param path path of directory to be removed.
        @return true on success.
    */
    static Boolean removeDirectoryHier(const String& path);

    /** Gets names of all entries (files and directories) of a directory.
        Note that this function excludes the "." and ".." entries.
        @param path path path of directory.
        @param paths contains list of entry names upon return. Note that
            the entry names only are provided (no path part).
        @return true on success.
    */
    static Boolean getDirectoryContents(
        const String& path,
        Array<String>& paths);

    /** Determines whether the given directory is empty. A directory is
        empty if it contains no files or directories.
        @param path path of directory.
        @return true if directory is empty.
    */
    static Boolean isDirectoryEmpty(const String& path);

    /** Translate backward slashes to forward slashes.
        @param path to be translated.
    */
    static void translateSlashes(String& path);

    /** Get an absolute path from an absolute directory and a relative or
        absolute file name.  If the file name is fully specified, it is
        returned unchanged.  Otherwise, the specified directory is prepended
        to the file name.
    */
    static String getAbsolutePath(const char* path, const String& filename);

    /** Return the just the filename to the file name into base.
    */
    static String extractFileName(const String& base);

    /** Return the just the path to the file name into path.
    */
    static String extractFilePath(const String& path);

    /** Changes file permissions on the given file.
        @param path path of the file.
        @param mode the bit-wise inclusive OR of the values for the
        desired permissions.
        @return true on success, false on error and errno is set appropriately.
    */
    static Boolean changeFilePermissions(const String& path, mode_t mode);

    /**
       Return OS path specific delimiter.

       @return delimiter specific to the platform
    */
    static String getPathDelimiter();

    /**
       Returns the absolute pathname for the specified filename.

       @param paths directories seperated by an OS specific delimiter to search
       @param filename filename to search for in the paths

       @return the full absolute pathname to the found filename or an empty
       string on failure.
    */
    static String getAbsoluteFileName(
        const String& paths,
        const String& filename);

    /**
        Convert a library name to its corresponding file name by adding the
        appropriate prefix and suffix.

        @param libraryName The name of the library for which to build the file
                           name.
        @return The file name corresponding to the specified library name.
    */
    static String buildLibraryFileName(const String &libraryName);

    /**
       Returns the platform-specific file name extension for dynamic
       libraries.

       @return the platform-specific file name extension for dynamic
       libraries.
    */
    static String getDynamicLibraryExtension();

    static Boolean changeFileOwner(
        const String& fileName,
        const String& userName);

    /**
        Flushes the data from the iostream buffers to the OS buffers and
        then flushes the data from the OS buffers to the disk.

        This will avoid the possible data loss in case of an OS crash when
        OS filesystem commit directory-level changes immediately while
        file-level changes remain cached (e.g. HP-UX).

        @param fstream. The iostream that we want to flush data.
    */
    static void syncWithDirectoryUpdates(PEGASUS_STD(fstream)&);

private:

    FileSystem() { }
};

inline Boolean FileSystem::existsNoCase(const String& path)
{
    String dummy;
    return existsNoCase(path, dummy);
}

inline Boolean FileSystem::canReadNoCase(const String& path)
{
    String realPath;

    if (!existsNoCase(path, realPath))
        return false;

    return FileSystem::canRead(realPath);
}

inline Boolean FileSystem::canWriteNoCase(const String& path)
{
    String realPath;

    if (!existsNoCase(path, realPath))
        return false;

    return FileSystem::canWrite(realPath);
}

inline Boolean FileSystem::removeFileNoCase(const String& path)
{
    String realPath;

    if (!existsNoCase(path, realPath))
        return false;

    return FileSystem::removeFile(realPath);
}

inline Boolean FileSystem::renameFileNoCase(
    const String& oldPath,
    const String& newPath)
{
    String realPath;

    if (!existsNoCase(oldPath, realPath))
        return false;

    return FileSystem::renameFile(realPath, newPath);
}

inline Boolean FileSystem::getFileSizeNoCase(const String& path, Uint32& size)
{
    String realPath;

    if (!existsNoCase(path, realPath))
        return false;

    return FileSystem::getFileSize(realPath, size);
}

inline String FileSystem::getAbsolutePath(
    const char* path,
    const String& filename)
{
    String absolutePath;

    if (filename != String::EMPTY)
    {
        if (!System::is_absolute_path(filename.getCString()) && path && path[0])
        {
            absolutePath.append(path);
            absolutePath.append('/');
        }
        absolutePath.append(filename);
    }
    translateSlashes(absolutePath);

    return absolutePath;
}

inline Boolean Open(PEGASUS_STD(ifstream)& is, const String& path)
{
    is.open(path.getCString());
    return !!is;
}

inline Boolean Open(PEGASUS_STD(ofstream)& os, const String& path)
{
    os.open(path.getCString());
    return !!os;
}

inline Boolean OpenAppend(PEGASUS_STD(ofstream)& os, const String& path)
{
    os.open(path.getCString(), PEGASUS_STD(ios::app));
    return !!os;
}

inline String FileSystem::getPathDelimiter()
{
#if defined(PEGASUS_OS_TYPE_WINDOWS)
    return String(";");
#else
    return String(":");
#endif
}

/** Get the next line from the input file.
*/
PEGASUS_COMMON_LINKAGE Boolean GetLine(PEGASUS_STD(istream)& is, Buffer& line);

inline Boolean GetLine(PEGASUS_STD(istream)& is, String& line)
{
    Buffer lineBuffer;
    Boolean result = GetLine(is, lineBuffer);
    line = String(lineBuffer.getData(), lineBuffer.size());
    return result;
}

PEGASUS_NAMESPACE_END

#endif /* Pegasus_FileSystem_h */
