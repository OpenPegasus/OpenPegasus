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
//
//%/////////////////////////////////////////////////////////////////////////////

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/ArrayInternal.h>
#include <Pegasus/Repository/InstanceIndexFile.h>
#include <Pegasus/Repository/InstanceDataFile.h>

#include <iostream>
#include <cstring>
#include <Pegasus/Common/PegasusAssert.h>

PEGASUS_USING_PEGASUS;
PEGASUS_USING_STD;

char * tmpDir = 0;

void _Test01()
{
    String indexPath (tmpDir);
    indexPath.append("/X.idx");
    CString ipd = indexPath.getCString();
    const char* PATH = ipd;
    Uint32 index;
    Uint32 size;
    Boolean result;
    Uint32 freeCount = 0;

    // create 5 entries
    size = 1427;
    index = 0;
    CIMObjectPath instName1("X.key1=1001,key2=\"Hello World 1\"");
    result = InstanceIndexFile::createEntry(PATH, instName1, index, size);
    PEGASUS_TEST_ASSERT(result);

    size = 1433;
    index = 1427;
    CIMObjectPath instName2("X.key1=1002,key2=\"Hello World 2\"");
    result = InstanceIndexFile::createEntry(PATH, instName2, index, size);
    PEGASUS_TEST_ASSERT(result);

    size = 1428;
    index = 2860;
    CIMObjectPath instName3("X.key1=1003,key2=\"Hello \nWorld\\\\n \\\"\r 3\"");
    result = InstanceIndexFile::createEntry(PATH, instName3, index, size);
    PEGASUS_TEST_ASSERT(result);

    size = 1433;
    index = 4288;
    CIMObjectPath instName4("X.key1=1004,key2=\"Hello World 4\"");
    result = InstanceIndexFile::createEntry(PATH, instName4, index, size);
    PEGASUS_TEST_ASSERT(result);

    size = 1431;
    index = 5721;
    CIMObjectPath instName5("X.key1=1005,key2=\"Hello World 5\"");
    result = InstanceIndexFile::createEntry(PATH, instName5, index, size);
    PEGASUS_TEST_ASSERT(result);

    // delete the 3rd entry
    result = InstanceIndexFile::deleteEntry(PATH, instName3, freeCount);
    PEGASUS_TEST_ASSERT(result);

    // create a new entry
    size = 1428;
    index = 2860;
    result = InstanceIndexFile::createEntry(PATH, instName3, index, size);
    PEGASUS_TEST_ASSERT(result);

    // delete the newly created entry
    result = InstanceIndexFile::deleteEntry(PATH, instName3, freeCount);
    PEGASUS_TEST_ASSERT(result);

    // delete the first entry
    result = InstanceIndexFile::deleteEntry(PATH, instName1, freeCount);
    PEGASUS_TEST_ASSERT(result);

    // modify the 5th entry
    size = 9999;
    index = 8888;
    result = InstanceIndexFile::modifyEntry(
        PATH, instName5, index, size, freeCount);
    PEGASUS_TEST_ASSERT(result);

    //
    // Iterate the entries in the file:
    //   There should be 7 entries
    //   The 1st, 3rd, 5th and 6th entries should be marked as 'free'.
    //

    {
    Array<Uint32> freeFlags;
    Array<Uint32> indices;
    Array<Uint32> sizes;
    Array<CIMObjectPath> instanceNames;

    Boolean flag = InstanceIndexFile::enumerateEntries(
        PATH, freeFlags, indices, sizes, instanceNames, true);

    PEGASUS_TEST_ASSERT(flag);

    PEGASUS_TEST_ASSERT(freeFlags.size() == indices.size());
    PEGASUS_TEST_ASSERT(indices.size() == sizes.size());
    PEGASUS_TEST_ASSERT(sizes.size() == instanceNames.size());

        PEGASUS_TEST_ASSERT( freeFlags[0] == 1 &&
                freeFlags[2] == 1 &&
                freeFlags[4] == 1 &&
                freeFlags[5] == 1);
    }

    //
    // Now attempt to compact:
    //
    PEGASUS_TEST_ASSERT(InstanceIndexFile::compact(PATH));

    //
    // Verify the result:
    //   There should be 3 entries and no 'free' entries
    //
    {
    Array<Uint32> freeFlags;
    Array<Uint32> indices;
    Array<Uint32> sizes;
    Array<CIMObjectPath> instanceNames;

        Boolean flag = InstanceIndexFile::enumerateEntries(
            PATH, freeFlags, indices, sizes, instanceNames, true);

        PEGASUS_TEST_ASSERT(flag);

        PEGASUS_TEST_ASSERT(freeFlags.size() == 3);
        PEGASUS_TEST_ASSERT(freeFlags.size() == indices.size());
        PEGASUS_TEST_ASSERT(indices.size() == sizes.size());
        PEGASUS_TEST_ASSERT(sizes.size() == instanceNames.size());

        for (Uint32 i = 0; i < freeFlags.size(); i++)
        {
            PEGASUS_TEST_ASSERT(freeFlags[i] == 0);
        }
    }
}

void _Test02()
{
    String instancesPath (tmpDir);
    instancesPath.append("/X.instances");
    CString ipd = instancesPath.getCString();
    const char* PATH = ipd;

    //
    // Append some instances:
    //

    Buffer data;
    Uint32 index;

    data.append("AAAAAAAA", 8);
    InstanceDataFile::appendInstance(PATH, data, index);
    data.clear();

    data.append("BBBBBBBB", 8);
    InstanceDataFile::appendInstance(PATH, data, index);
    data.clear();

    data.append("CCCCCCCC", 8);
    InstanceDataFile::appendInstance(PATH, data, index);
    data.clear();

    //
    // Load some instances:
    //

    InstanceDataFile::loadInstance(PATH, 8, 8, data);
    PEGASUS_TEST_ASSERT(memcmp(data.getData(), "BBBBBBBB", 8) == 0);
    data.clear();

    InstanceDataFile::loadInstance(PATH, 0, 8, data);
    PEGASUS_TEST_ASSERT(memcmp(data.getData(), "AAAAAAAA", 8) == 0);
    data.clear();

    InstanceDataFile::loadInstance(PATH, 16, 8, data);
    PEGASUS_TEST_ASSERT(memcmp(data.getData(), "CCCCCCCC", 8) == 0);
    data.clear();

    //
    // Load all instances:
    //

    InstanceDataFile::loadAllInstances(PATH, data);
    PEGASUS_TEST_ASSERT(
        memcmp(data.getData(), "AAAAAAAABBBBBBBBCCCCCCCC", 24) == 0);
    PEGASUS_TEST_ASSERT(data.size() == 3 * 8);
    data.clear();

    //
    // Now attempt rollback:
    //

    PEGASUS_TEST_ASSERT(InstanceDataFile::beginTransaction(PATH));

    data.append("ZZZZZZZZ", 8);
    InstanceDataFile::appendInstance(PATH, data, index);
    data.clear();

    PEGASUS_TEST_ASSERT(InstanceDataFile::rollbackTransaction(PATH));

    //
    // Now attempt commit:
    //

    PEGASUS_TEST_ASSERT(InstanceDataFile::beginTransaction(PATH));

    data.append("DDDDDDDD", 8);
    InstanceDataFile::appendInstance(PATH, data, index);
    data.clear();

    PEGASUS_TEST_ASSERT(InstanceDataFile::commitTransaction(PATH));

    //
    // Verify the result:
    //

    InstanceDataFile::loadAllInstances(PATH, data);
    PEGASUS_TEST_ASSERT(
        memcmp(data.getData(), "AAAAAAAABBBBBBBBCCCCCCCCDDDDDDDD", 32) == 0);
    PEGASUS_TEST_ASSERT(data.size() == 4 * 8);
    data.clear();

    //
    // Now attempt to compact:
    //

    Array<Uint32> freeFlags;
    Array<Uint32> indices;
    Array<Uint32> sizes;

    freeFlags.append(0);
    indices.append(0);
    sizes.append(8);

    freeFlags.append(0);
    indices.append(16);
    sizes.append(8);

    PEGASUS_TEST_ASSERT(
        InstanceDataFile::compact(PATH, freeFlags, indices, sizes));

    //
    // Verify the result:
    //

    InstanceDataFile::loadAllInstances(PATH, data);
    PEGASUS_TEST_ASSERT(memcmp(data.getData(), "AAAAAAAACCCCCCCC", 16) == 0);
    PEGASUS_TEST_ASSERT(data.size() == 2 * 8);
    data.clear();
}

int main(int, char** argv)
{
    const char * envTmpDir = getenv ("PEGASUS_TMP");
    if (envTmpDir == 0 || strlen(envTmpDir) == 0)
    {
        tmpDir = strdup(".");
    } else tmpDir = strdup(envTmpDir);

    try
    {
    _Test01();
    _Test02();
        free(tmpDir);
    }

    catch (Exception& e)
    {
    cerr << "Error: " << e.getMessage() << endl;
        free(tmpDir);
    exit(1);
    }

    cout << argv[0] << " +++++ passed all tests" << endl;

    return 0;
}
