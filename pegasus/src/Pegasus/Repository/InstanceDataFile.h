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

#ifndef Pegasus_InstanceDataFile_h
#define Pegasus_InstanceDataFile_h

#include <fstream>
#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/String.h>
#include <Pegasus/Common/InternalException.h>
#include <Pegasus/Common/CIMObjectPath.h>
#include <Pegasus/Repository/Linkage.h>
#include <Pegasus/Common/Buffer.h>

PEGASUS_NAMESPACE_BEGIN

/** This class manages access to an instance data file which contains all
    instances of a particular class.

    The instances are stored in ASCII XML format one after another. A separate
    instance index file is maintained (see InstanceIndexFile) which contains
    an index entry for each instance in the data file. This index ties the key
    of the instance to an offset and size within the instance file.

    The index file and data file are named according to the class whose
    instances they contain information about. The index file and data file for
    a class named "Zebra" would be:

    <pre>
        Zebra (index file).
        Zebra.instances (instance-file).
    </pre>

    When an instance is created, it is appended to the end of the instance
    file. When one is deleted, it is marked as free in the index file. When
    an instance is modified, the old one is marked as deleted and the new
    modified instance is appended to the end of the data file. Note that
    deletion and modification may leave unused gaps in the data file. These
    gaps are reclaimed during compaction (performed when the data file has
    N gaps where N is some arbitrarily chosen number for now). Performing
    compaction during each modify and delete would be extremely inefficient.
    By postponing it until N such operations have been performed, we
    improve performance considerably.

    Note the three operations which may be performed on an instance and there
    associated effect on the data file.

    <ul>
    <li>Create - appends an instance to the end of the file.</li>
    <li>Modify - appends an instance to the end of the file.</li>
    <li>Delete - has no effect on the data file</li>
    </ul>

    To avoid corruption of the data file we provide a rollback scheme. A
    rollback file is created (the name is formed by appending ".rollback" to
    the name of the data file) which contains the original size of the data
    file. After the modification of the data file is complete, the rollback
    file is removed. If we discover a rollback file when we start an operation,
    this means that the last operation did not succeed. We then rollback the
    operation by truncating the file to its old size.
*/
class PEGASUS_REPOSITORY_LINKAGE InstanceDataFile
{
public:

    /** loads an instance from the data file into memory.

        @param path the file path of the instance file
        @param index the byte positon of the instance record
        @param size the size of the instance record
        @param data the buffer to hold the instance data
        @return true on success.
    */
    static Boolean loadInstance(
        const String& path,
        Uint32 index,
        Uint32 size,
        Buffer& data);

    /** loads all the instances from the data file into memory.

        @param path the file path of the instance file
        @param data the buffer to hold the data
        @return true on success.
    */
    static Boolean loadAllInstances(
        const String& path,
        Buffer& data);

    /** Appends a new instance to the end of the file.

        @param out the buffer containing the CIM/XML encoding of the
        @param path the file path of the instance file
        @param index the byte positon of the instance record
        @return true on success
    */
    static Boolean appendInstance(
        const String& path,
        const Buffer& data,
        Uint32& index);

    /** Begin a transaction to modify this file. The effect of subsequent
        modifications can be rolled back by calling rollbackTransaction().
    */
    static Boolean beginTransaction(const String& path);

    /** In case of a failure in the beginTransaction(), undo the changes
        done in the begin transaction and restore the repository
        to the previous state
    */
    static void undoBeginTransaction(const String& path);

    /** Roll back any changes to the file since the last time
        beginTransaction() was called.
    */
    static Boolean rollbackTransaction(const String& path);

    /** Commit changes made after beginTransaction() was called.
    */
    static Boolean commitTransaction(const String& path);

    /** Reorganizes the data file to reclaim free space. This is done by
        copying over all non-free instances to a temporary file and then
        deleting the original file and renaming the temporary file to the
        same name as the original.
    */
    static Boolean compact(
        const String& path,
        const Array<Uint32>& freeFlags,
        const Array<Uint32>& indices,
        const Array<Uint32>& sizes);

private:

    static Boolean _openFile(
        PEGASUS_STD(fstream)& fs,
        const String& path,
        int mode);
};

PEGASUS_NAMESPACE_END

#endif /* Pegasus_InstanceDataFile_h */
