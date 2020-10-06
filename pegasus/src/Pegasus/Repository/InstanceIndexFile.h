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

#ifndef Pegasus_InstanceIndexFile_h
#define Pegasus_InstanceIndexFile_h

#include <fstream>
#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/String.h>
#include <Pegasus/Common/InternalException.h>
#include <Pegasus/Common/CIMObjectPath.h>
#include <Pegasus/Repository/Linkage.h>

PEGASUS_NAMESPACE_BEGIN

/** This class manages access to an "instance index file" which maps
    instance names to offsets of the instances contained in the "instance
    data file".

    All files belonging to the instance repository are stored under the
    repository/instances directory. For each class, two files are maintained:
    an index file (with a ".idx" extension) and an instance file which bears
    the name of the class whose instances it contains. For example, suppose
    there is a class called "Zebra". Then two files are used to manage its
    instances:

        <pre>
        repository/instances/Zebra.idx (called the index file)
        repository/instances/Zebra (called the instance file)
        </pre>

    The first line of the index file is a free count, expressed as eight hex
    digits followed by a newline. The free count indicates how many instances
    are free (but not reclaimed). When instances are deleted or modified, the
    corresponding entry in this index file is marked as free (by changing the
    first column of the entry from '0' to '1'). To improve performance,
    reclamation of unused space in the instance file (called gaps) is
    postponed until there are m gaps.

    Reorganization is expensive: the entire instance file and index file must
    be rewritten. The time complexity is O(n) where n is the number of
    instances in the file. By postponing reorganization, the time complexity
    may be reduced to O(1).

    All subsequent lines contain an entry with the following form:

    <pre>
        <free> <hash-code> <offset> <size> ClassName.key1=val1,...,keyN=valN
    </pre>

    <ul>
    <li>
    free - '1' if the corresponding instance was deleted, '0' otherwise.
        When instances are deleted, the corresponding entry in the index file
        is marked as free and a gap is left in the instance file until
        reorganization time.
    </li>
    <li>
    hash-code - hash code for the key field below. This field is provided to
        speed lookup of index entries. When looking up an entry, compute the
        hash code of the key and look for entries with the same hash code.
        It is still necessary to compare the keys when the hash codes are
        the same (since collisions are possible), but only when they are the
        same which is rare and hence this scheme saves many comparisons.
    </li>
    <li>
    offset - offset within the instance file to where the instance begins.
    </li>
    <li>
    size - size in bytes of the instance itself (as it appears in the instance
        file).
    </li>
    <li>
    key - the compound key of the instance (including all key binding pairs).
    </li>
    </ul>

    Here's an example of an index file:

    <pre>
        00000001
        0 A6B275A9 0 1425 Employee.ssn=444332222
        1 A6BA08B1 1425 1430 Employee.ssn=555667777
    </pre>

    Notice that the dirty count is equal to one and that one entry is marked
    as deleted (these quantities must be equal). This indicates that the
    instance file has one instance which is no longer used. The space used by
    this instance will be reclaimed during reorganization.

    The layout of the instance file is trivial. Instances are always appended
    to the instance file. The instances are kept end-to-end in the file.

    To lookup an entry in the index file, first take the hash code of the
    target key. Then find the first non-free entry whose hash code and key
    are the same as the target hash code and key. Note that an entry may
    usually be ruled out by comparing the hash codes (except in the case of
    clashes).

    There are three operations which may be performed on the instance
    repository: create, modify, and delete.

    Creation. During creation, the instance is appended to the instance file
    and an entry is appended to the index file. If an instance with the same
    key is found, then the operation is disallowed.

    Deletion. To delete an instance, the corresponding entry in the index file
    is marked as deleted (by changing the first column from '0' to '1'). And
    then the dirty count is incremented and updated. If the dirty count has
    reached the configured threshold, the index and instance files are
    reogranized.

    Modification. To modify an instance, the new modified instance is appended
    to the instance file. Next the old entry with the same key is marked as
    deleted.  Finally, a new entry is inserted into the index file.
*/
class PEGASUS_REPOSITORY_LINKAGE InstanceIndexFile
{
public:

    /** Searches the instance index file for the given instance name. Sets
        the index parameter to the corresponding index, and sets the size
        parameter to the corresponding instance record size.  Returns true
        on success.

        @param path the file path of the instance index file
        @param instanceName the name of the instance
        @param indexOut the index of the instance record found
        @param sizeOut the size of the instance record found
        @return true if the instance is found; false otherwise.
    */
    static Boolean lookupEntry(
        const String& path,
        const CIMObjectPath& instanceName,
        Uint32& indexOut,
        Uint32& sizeOut);

    /** Creates a new entry in the instance index file. Saves the index and
        size of the instance record passed in.  This method assumes that the
        keys in the instance name are in sorted order. This must be done
        prior to calling the method.  Returns true on success.

        @param path the file path of the instance index file
        @param instanceName the name of the instance
        @param indexIn the index of the new instance record
        @param sizeIn the size of the new instance record
        @return true if successful; false otherwise.
    */
    static Boolean createEntry(
        const String& path,
        const CIMObjectPath& instanceName,
        Uint32 indexIn,
        Uint32 sizeIn);

    /** Deletes the entry with the given instance name.
        @param path path of the instance index file
        @param instanceName name of the instance
        @return true on success
    */
    static Boolean deleteEntry(
        const String& path,
        const CIMObjectPath& instanceName,
        Uint32& freeCount);

    /** Modifies an entry by first removing the old entry and then inserting
        a new entry at the end of the file.

        @param path the file path of the instance index file.
        @param instanceName the name of the instance.
        @param indexIn the index of the modified instance record.
        @param sizeIn the size of the modified instance record.
        @return true on success.
    */
    static Boolean modifyEntry(
        const String& path,
        const CIMObjectPath& instanceName,
        Uint32 indexIn,
        Uint32 sizeIn,
        Uint32& freeCount);

    /** Gets the information stored in the index file for all the instances
        of the given class.  Appends the instance names, indices and sizes
        to the given arrays (does not clear the arrays first).  Returns
        true on success.

        @param path path of the instance index file.
        @param instanceNames array to hold the instance names.
        @param indices array to hold the indices of the instance records.
        @param sizes an array to hold the sizes of the instance records.
        @return true on success.
    */
    static Boolean enumerateEntries(
        const String& path,
        Array<Uint32>& freeFlags,
        Array<Uint32>& indices,
        Array<Uint32>& sizes,
        Array<CIMObjectPath>& instanceNames,
        Boolean includeFreeEntries);


    /** Returns true if this index file has any non-free entries:
    */
    static Boolean hasNonFreeEntries(const String& path);

    /** Begin a transaction to modify this file. The effect of subsequent
        modifications can be rolled back by calling rollbackTransaction().
        The current implementation simply copies the index file to a a file
        with the same name but with ".rollback" appended to it.
    */
    static Boolean beginTransaction(const String& path);

    /** In case of a failure in the beginTransaction(), undo the changes
        done in the begin transaction and restore the repository
        to the previous state. The current implementation removes the
        index file and copies the rollback file to the index file.
    */
    static void undoBeginTransaction(const String& path);

    /** Roll back any changes to the file since the last time
        beginTransaction() was called. The current implementation deletes
        the current file and renames the rollback file to the same name.
    */
    static Boolean rollbackTransaction(const String& path);

    /** Commit changes made after beginTransaction() was called. The curent
        implementation simply removes the .rollback file.
    */
    static Boolean commitTransaction(const String& path);

    /** Compact the file by removing entries which are marked as free.
    */
    static Boolean compact(
        const String& path);

private:

    /** Open the index file and position the file pointer on the first
        entry (immediately after the free count). Create the file if it
        does not exist and if the create flag is true (writing a free
        count of zero).
    */
    static Boolean _openFile(
        const String& path,
        PEGASUS_STD(fstream)& fs,
        Boolean create = false);

    /** Appends a new entry to the index file; called by both createEntry()
        and modifyEntry().
    */
    static Boolean _appendEntry(
        PEGASUS_STD(fstream)& fs,
        const CIMObjectPath& instanceName,
        Uint32 indexIn,
        Uint32 sizeIn);

    /** Increment the index file's free count; called by _markEntryFree().
        The resulting value is left in the freeCount parameter.
    */
    static Boolean _incrementFreeCount(
        PEGASUS_STD(fstream)& fs,
        Uint32& freeCount);

    /** Marks the entry matching the given instanceName as free; called by both
        deleteEntry() and modifyEntry().
    */
    static Boolean _markEntryFree(
        PEGASUS_STD(fstream)& fs,
        const CIMObjectPath& instanceName);

    /** Helper method for lookupEntry() which takes a file stream rather than
        a path.
    */
    static Boolean _lookupEntry(
        PEGASUS_STD(fstream)& fs,
        const CIMObjectPath& instanceName,
        Uint32& indexOut,
        Uint32& sizeOut,
        Uint32& entryOffset);
};

PEGASUS_NAMESPACE_END

#endif /* Pegasus_InstanceIndexFile_h */
