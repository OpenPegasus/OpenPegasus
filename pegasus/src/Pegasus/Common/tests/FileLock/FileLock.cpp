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

/*
    This unit test exercises the AutoFileLock class.  Since a file lock does
    not protect against multiple locks by the same process, it is necessary
    to use separate processes to exercise the locking mechanism.

    This test spawns child processes, each of which iterates the steps of
    locking a LOCK_FILE, retrieving a counter value from a COUNTER_FILE,
    incrementing the counter, writing it back to the COUNTER_FILE, and
    unlocking the LOCK_FILE.  The master process ensures that the correct
    number of increments are performed.
*/

#include <Pegasus/Common/System.h>
#include <Pegasus/Common/FileSystem.h>
#include <Pegasus/Common/Thread.h>
#include <Pegasus/Common/Threads.h>
#include <Pegasus/Common/PegasusAssert.h>
#include <iostream>

PEGASUS_USING_PEGASUS;
PEGASUS_USING_STD;

const char* LOCK_FILE = "lockFile";
const char* COUNTER_FILE = "counterFile";
const Uint32 NUM_SUBTESTS = 10;
const Uint32 NUM_ITERATIONS = 20;

// PEGASUS_POPEN and PEGASUS_PCLOSE macros are introduced to use a common calls
// on both unix and windows.
#if defined(PEGASUS_OS_TYPE_UNIX)
# define PEGASUS_POPEN popen
# define PEGASUS_PCLOSE pclose
#elif defined(PEGASUS_OS_TYPE_WINDOWS)
# define PEGASUS_POPEN _popen
# define PEGASUS_PCLOSE _pclose
#endif

void master(char testProgram[])
{
    // Master process

    // Change to a temporary directory, if configured

    const char* tmpDir = getenv ("PEGASUS_TMP");
    if (tmpDir != NULL)
    {
        FileSystem::changeDirectory(tmpDir);
    }

    // Create the lock file

    fstream fs;
    FileSystem::removeFile(LOCK_FILE);
    fs.open(LOCK_FILE, ios::out PEGASUS_OR_IOS_BINARY);
    fs.close();

    // Initialize the counter file to "0"

    FileSystem::removeFile(COUNTER_FILE);
    fs.open(COUNTER_FILE, ios::out PEGASUS_OR_IOS_BINARY);
    fs.write("00000000", 8);
    fs.close();
#if defined(PEGASUS_OS_TYPE_UNIX) || defined(PEGASUS_OS_TYPE_WINDOWS)

    // Start the subtests

    FILE* fd[NUM_SUBTESTS];
    Uint32 i;

    for ( i = 0; i < NUM_SUBTESTS; i++)
    {
        fd[i] = PEGASUS_POPEN(testProgram, "r");
        PEGASUS_TEST_ASSERT(fd[i]);
    }

    // Wait for the subtests to complete

    for ( i = 0; i < NUM_SUBTESTS; i++)
    {
        PEGASUS_PCLOSE(fd[i]);
    }

#endif

    // Verify the result

    char buffer[9];
    FileSystem::openNoCase(
    fs, COUNTER_FILE, ios::in PEGASUS_OR_IOS_BINARY);
    fs.read(buffer, 8);
    buffer[8] = 0;
    fs.close();

    PEGASUS_TEST_ASSERT(atoi(buffer) == NUM_SUBTESTS*NUM_ITERATIONS);

    // Clean up the files

    FileSystem::removeFile(COUNTER_FILE);
    FileSystem::removeFile(LOCK_FILE);

    // Test an invalid lock file
    AutoFileLock lock("not/a/valid/file");
}

void subtest()
{
    for (Uint32 i = 0; i < NUM_ITERATIONS; i++)
    {
        AutoFileLock lock(LOCK_FILE);

        // Get the counter from the file

        char buffer[9];
        fstream fs;
        fs.open(COUNTER_FILE, ios::in PEGASUS_OR_IOS_BINARY);
        fs.read(buffer, 8);
        buffer[8] = 0;
        fs.close();
        Uint32 counter = atoi(buffer);

        // Wait a bit to increase the collision window with other processes

        Threads::sleep(10);

        // Write the incremented counter to the file

        sprintf(buffer, "%08u", counter+1);
        fs.open(COUNTER_FILE, ios::out PEGASUS_OR_IOS_BINARY);
        fs.write(buffer, 8);
        fs.close();
    }
}

int main(int argc, char** argv)
{
    if ((argc > 2) || ((argc == 2) && (strcmp(argv[1], "master") != 0)))
    {
        cerr << "Usage:  " << argv[0] << " master - start the tests" << endl;
        cerr << "        " << argv[0] << "        - run a subtest" << endl;
        exit(1);
    }

    if (argc == 2)
    {
        master(argv[0]);

        cout << argv[0] << " +++++ passed all tests" << endl;
    }
    else
    {
        subtest();
    }

    return 0;
}
