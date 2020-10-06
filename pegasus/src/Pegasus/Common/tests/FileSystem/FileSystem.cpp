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

#include <Pegasus/Common/PegasusAssert.h>
#include <fstream>
#include <iostream>
#include <cstdio>

#include <Pegasus/Common/FileSystem.h>

PEGASUS_USING_PEGASUS;
PEGASUS_USING_STD;

static char * verbose;

void TestGetLine(const String& tmpDir)
{
    const char utf8Mixed[14] = {
        'a',
        '\x79',                  // letter y
        'b',
        '\xC3', '\xA4',          // umlaut ae
        'c',
        '\xC2', '\xAE',          // sign for registered trademark
        'd',
        '\xE2', '\x82', '\xAC',  // euro sign
        'e',
        '\0'                     // null termination
    };

    String fileName = tmpDir + "/TestGetLine.txt";
    String outputLine(utf8Mixed);

    {
        ofstream os;
        PEGASUS_TEST_ASSERT(Open(os, fileName));
        os.write(utf8Mixed, 13);
        os.put('\n');
    }

    {
        ifstream is;
        PEGASUS_TEST_ASSERT(Open(is, fileName));
        String inputLine;
        PEGASUS_TEST_ASSERT(GetLine(is, inputLine));
        PEGASUS_TEST_ASSERT(inputLine == outputLine);
    }
}

void _printArray(const String& name, const Array<String>& globList)
    {
        if (verbose)
        {
            cout << name << " count= " << globList.size() << endl;
            for (Uint32 i = 0 ; i < globList.size(); i++)
            {
                cout << name << " " << i << " " << globList[i] << endl;
            }
        }
    }

void _filterArray(Array<String>& ra)
{
    for (Uint32 i = 0; i < ra.size(); i++)
    {
        if (String::equal(ra[i], "CVS") || ra[i][0] == '.')
        {
            ra.remove(i--);
        }
    }
}


static void _cleanup(const String& tmpDir)
{
    FileSystem::removeFile(tmpDir + "/TestFile.txt");
    FileSystem::removeFile(tmpDir + "/TestFile1.txt");
    FileSystem::removeFile(tmpDir + "/file1.txt");
    FileSystem::removeFile(tmpDir + "/file2.txt");
    FileSystem::removeFile(tmpDir + "/TestGetLine.txt");

    // We should have removed all files from the test directory
    Array<String> globList;
    FileSystem::glob(tmpDir + " /*", "*", globList);
    PEGASUS_TEST_ASSERT(globList.size() == 0);
    FileSystem::removeDirectoryHier(tmpDir + "/TestDirectory2");
    FileSystem::removeDirectoryHier(tmpDir + "/TestDirectory");
}

int main(int, char** argv)
{
    verbose = getenv("PEGASUS_TEST_VERBOSE");
    const char* tmpDir = getenv ("PEGASUS_TMP");
    if (tmpDir == NULL)
    {
        tmpDir = ".";
    }

    _cleanup(tmpDir);

    TestGetLine(tmpDir);

    String path;
    PEGASUS_TEST_ASSERT(FileSystem::getCurrentDirectory(path));
    // Need to add test to confirm that the directory
    // is indeed FileSystem.
    PEGASUS_TEST_ASSERT(FileSystem::exists("FileSystem.cpp"));
    PEGASUS_TEST_ASSERT(!FileSystem::exists("NoSuchFile.dat"));
    // PEGASUS_TEST_ASSERT(!FileSystem::canExecute("FileSytem.cpp"));
    PEGASUS_TEST_ASSERT(!FileSystem::canRead("NoSuchFile.dat"));
    PEGASUS_TEST_ASSERT(!FileSystem::canWrite("NoSuchFile.dat"));
    // PEGASUS_TEST_ASSERT(!FileSystem::canRead("NoCanRead.dat"));
    PEGASUS_TEST_ASSERT(FileSystem::isDirectory(".."));
    PEGASUS_TEST_ASSERT(FileSystem::isDirectory("."));
    PEGASUS_TEST_ASSERT(!FileSystem::isDirectory("FileSystem.cpp"));
    PEGASUS_TEST_ASSERT(FileSystem::isDirectory("./testdir"));

    // test the glob function
    // Test for explicit patterns because we may not know all
    // files in the directory.
    Array<String> globList;

    PEGASUS_TEST_ASSERT(FileSystem::glob(path ,  "*", globList));
    _printArray("glob 1 . *" , globList);
    PEGASUS_TEST_ASSERT(globList.size() >= 3);

    PEGASUS_TEST_ASSERT(FileSystem::glob( ".",  "*.cpp", globList));
    PEGASUS_TEST_ASSERT(globList.size() == 1);
    PEGASUS_TEST_ASSERT(globList[0] == "FileSystem.cpp");

    PEGASUS_TEST_ASSERT(FileSystem::glob( ".",  "*ile*.cpp", globList));
    PEGASUS_TEST_ASSERT(globList.size() == 1);
    PEGASUS_TEST_ASSERT(globList[0] == "FileSystem.cpp");

    PEGASUS_TEST_ASSERT(FileSystem::glob( ".",  "*akefile", globList));
    PEGASUS_TEST_ASSERT(globList.size() == 1);
    PEGASUS_TEST_ASSERT(globList[0] == "Makefile");

    PEGASUS_TEST_ASSERT(FileSystem::glob( ".",  "Makefil*", globList));
    PEGASUS_TEST_ASSERT(globList.size() == 1);
    PEGASUS_TEST_ASSERT(globList[0] == "Makefile");

    PEGASUS_TEST_ASSERT(FileSystem::glob( "testdir",  "*", globList));
    _filterArray(globList);
    PEGASUS_TEST_ASSERT(globList.size() == 3);

    Array<String> paths;
    PEGASUS_TEST_ASSERT( FileSystem::getDirectoryContents("./testdir", paths) );

    _filterArray(paths);

    String realName;
    PEGASUS_TEST_ASSERT(FileSystem::existsNoCase("filesystem.cpp", realName));
    PEGASUS_TEST_ASSERT(String::equal(realName, "FileSystem.cpp"));

    PEGASUS_TEST_ASSERT(FileSystem::existsNoCase(
        "../FileSystem/filesystem.cpp", realName));
    PEGASUS_TEST_ASSERT(String::equal(realName,
        "../FileSystem/FileSystem.cpp"));

    BubbleSort(paths);
    PEGASUS_TEST_ASSERT(paths.size() == 3);
    PEGASUS_TEST_ASSERT(String::equal(paths[0], "a"));
    PEGASUS_TEST_ASSERT(String::equal(paths[1], "b"));
    PEGASUS_TEST_ASSERT(String::equal(paths[2], "c"));

    // Test for getCurrentDirectory
    // Go to testdir, test for file "a"
    // Then return and test for file
    {
        String saveDir;
        PEGASUS_TEST_ASSERT(FileSystem::getCurrentDirectory(saveDir));
        PEGASUS_TEST_ASSERT(FileSystem::changeDirectory("testdir"));
        PEGASUS_TEST_ASSERT(FileSystem::exists("a"));
        FileSystem::changeDirectory(saveDir);
        String newSaveDir;
        PEGASUS_TEST_ASSERT(FileSystem::getCurrentDirectory(newSaveDir));
        PEGASUS_TEST_ASSERT(saveDir == newSaveDir);
        PEGASUS_TEST_ASSERT(FileSystem::exists("FileSystem.cpp"));
    }
    // Test the Create and delete functions
    // Creates directories and files and deletes them.
    {
        String t (tmpDir);
        t.append("/TestDirectory");
        String t1 (tmpDir);
        t1.append("/TestDirectory2");
        String tf ("TestFile.txt");
        String tf1 ("TestFile1.txt");
        String tf2 ("TestFileNoExt");

        FileSystem::makeDirectory(t);
        PEGASUS_TEST_ASSERT(FileSystem::isDirectory(t));
        FileSystem::removeDirectory(t);
        PEGASUS_TEST_ASSERT(!FileSystem::isDirectory(t));

        // Tests for remove hierarchy command
        // because remove hiearchy does not work yet.
        FileSystem::makeDirectory(t);

        String save_cwd;
        FileSystem::getCurrentDirectory(save_cwd);

        // create some files in new directory
        if (!FileSystem::changeDirectory(t))
            // ATTN: what is valid error return?
            return -1;

        ofstream of1(tf.getCString());
        of1 << "test" << endl;
        of1.close();
        PEGASUS_TEST_ASSERT(FileSystem::exists(tf));

        ofstream of2(tf1.getCString());
        of2 << "test" << endl;
        of2.close();
        PEGASUS_TEST_ASSERT(FileSystem::exists(tf1));

        ofstream of2a(tf2.getCString());
        of2a << "test" << endl;
        of2a.close();
        PEGASUS_TEST_ASSERT(FileSystem::exists(tf2));

        // test globing of files in the new directory
        PEGASUS_TEST_ASSERT(FileSystem::glob(".", "*", globList));
        PEGASUS_TEST_ASSERT(globList.size() == 3);

        PEGASUS_TEST_ASSERT(FileSystem::glob(".", "T*", globList));
        PEGASUS_TEST_ASSERT(globList.size() == 3);

        PEGASUS_TEST_ASSERT(FileSystem::glob(".", "*.*", globList));
        PEGASUS_TEST_ASSERT(globList.size() == 2);

        PEGASUS_TEST_ASSERT(FileSystem::glob(".", "*Ext", globList));
        PEGASUS_TEST_ASSERT(globList.size() == 1);
        PEGASUS_TEST_ASSERT(globList[0] == "TestFileNoExt");

        PEGASUS_TEST_ASSERT(FileSystem::glob(".", "T*Ex*", globList));
        PEGASUS_TEST_ASSERT(globList.size() == 1);
        PEGASUS_TEST_ASSERT(globList[0] == "TestFileNoExt");

        FileSystem::glob(".", "TestFileNoEx*", globList);
        PEGASUS_TEST_ASSERT(globList.size() == 1);
        PEGASUS_TEST_ASSERT(globList[0] == "TestFileNoExt");

        PEGASUS_TEST_ASSERT(FileSystem::glob(".", "*junk*", globList));
        PEGASUS_TEST_ASSERT(globList.size() == 0);

        // Create a second level directory
        FileSystem::makeDirectory(t1);

        // Create files in this dir
        if (!FileSystem::changeDirectory(t1))
            return -1;

        ofstream of3("testfile3.txt");
        of3 << "test" << endl;
        of3.close();

            ofstream of4("testfile4.txt");
        of4 << "test" << endl;
        of4.close();

        // Go back to top level directory

        FileSystem::changeDirectory(save_cwd);
            PEGASUS_TEST_ASSERT(FileSystem::isDirectory(t));
        FileSystem::removeDirectoryHier(t);
        // be sure directory is removed
        PEGASUS_TEST_ASSERT(!FileSystem::isDirectory(t));
    }
    // Test renameFile:
    {
        String FILE1 (tmpDir);
        FILE1.append("/file1.txt");
        String FILE2 (tmpDir);
        FILE2.append("/file2.txt");

        ofstream of1(FILE1.getCString());
        of1 << "test" << endl;
        of1.close();
        PEGASUS_TEST_ASSERT(FileSystem::exists(FILE1));

        PEGASUS_TEST_ASSERT(FileSystem::exists(FILE1));
        PEGASUS_TEST_ASSERT(!FileSystem::exists(FILE2));
        PEGASUS_TEST_ASSERT(FileSystem::renameFile(FILE1, FILE2));

        PEGASUS_TEST_ASSERT(!FileSystem::exists(FILE1));

        PEGASUS_TEST_ASSERT(FileSystem::exists(FILE2));
        PEGASUS_TEST_ASSERT(FileSystem::renameFile(FILE2, FILE1));
        PEGASUS_TEST_ASSERT(FileSystem::exists(FILE1));
        PEGASUS_TEST_ASSERT(!FileSystem::exists(FILE2));
        PEGASUS_TEST_ASSERT(FileSystem::removeFile(FILE1));
        PEGASUS_TEST_ASSERT(!FileSystem::exists(FILE1));
    }
     // Test getFileNameFromPath
    {
        FileSystem::changeDirectory(path);
        String pathName = FileSystem::getAbsoluteFileName("./testdir","a");
        PEGASUS_TEST_ASSERT(pathName.size()!=0);    // It should be there.
        pathName = FileSystem::getAbsoluteFileName("./testdir","#$@#(@$#!");
        PEGASUS_TEST_ASSERT(pathName.size()==0);    // It should not be there.
    }

    // Test changeFileOwner
    {
        String testUser;
        testUser.assign(System::getEffectiveUserName());
        String cd(tmpDir);
        cd.append("/TestFile.txt");
        ofstream of1(cd.getCString());
        of1 << "test" << endl;
        of1.close();
        //checking that the file exists
        PEGASUS_TEST_ASSERT(FileSystem::exists(cd));

        //testing changeFileOwner with valid username
        PEGASUS_TEST_ASSERT(
            FileSystem::changeFileOwner(cd,testUser));

        //Added to test if file is owned by effective user for
        // current process
        PEGASUS_TEST_ASSERT(
            (System::verifyFileOwnership(cd.getCString()))==true);
#ifndef PEGASUS_OS_TYPE_WINDOWS
        //testing to fail changeFileOwner with an unusual username @@@###
        //on Windows changeFileOwner always returns true
        PEGASUS_TEST_ASSERT(!FileSystem::changeFileOwner(cd,"@@@###"));
#endif
    }

    //Test changeFilePermissions
    {
        String cd(tmpDir);
        cd.append("/TestFile.txt");
        PEGASUS_TEST_ASSERT(FileSystem::exists(cd));

#if defined(PEGASUS_OS_TYPE_WINDOWS)
        PEGASUS_TEST_ASSERT(
            FileSystem::changeFilePermissions(cd, _S_IREAD|_S_IWRITE));
#elif defined(PEGASUS_OS_VXWORKS)
        // Test is not meaningful for VxWorks since this file is on the
        // host and the target may not access this file.
#else
        PEGASUS_TEST_ASSERT(
            FileSystem::changeFilePermissions(cd,S_IRUSR|S_IRGRP|S_IROTH));
#endif

        PEGASUS_TEST_ASSERT(FileSystem::canReadNoCase(cd));

        if (System::isPrivilegedUser(System::getEffectiveUserName()))
        {
            PEGASUS_TEST_ASSERT(FileSystem::canWrite(cd));
        }

        // While granting access permissions Windows seems to be not
        // considering the privilege level of the user (i.e. either admin or
        // normaluser).  In case of normal user also write permission is
        // granted. Hence the below assert is failing. To avoid this
        // failure, added ifndef.
#ifndef PEGASUS_OS_TYPE_WINDOWS
        else
        {
            PEGASUS_TEST_ASSERT(!FileSystem::canWrite(cd));
        }
#endif
    }

    _cleanup(tmpDir);

    cout << argv[0] << " +++++ passed all tests" << endl;

    return 0;
}
