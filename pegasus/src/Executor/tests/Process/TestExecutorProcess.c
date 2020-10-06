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
/*
//
//%/////////////////////////////////////////////////////////////////////////////
*/

#include <Executor/Process.h>
#include <Executor/tests/TestAssert.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#define TEST_PID_FILE "testpidfile"

void testGetProcessName(void)
{
    char name[EXECUTOR_BUFFER_SIZE];

    PEGASUS_TEST_ASSERT(GetProcessName(getpid(), name) == 0);
    PEGASUS_TEST_ASSERT(strcmp(name, "TestExProcess") == 0);
}

void testReadPidFile(void)
{
    FILE* pidFile;
    int pid;

    /* Test non-existent PID file */

    unlink(TEST_PID_FILE);
    PEGASUS_TEST_ASSERT(ReadPidFile(TEST_PID_FILE, &pid) != 0);

    /* Test empty PID file */

    pidFile = fopen(TEST_PID_FILE, "a");
    PEGASUS_TEST_ASSERT(pidFile != 0);
    fclose(pidFile);
    PEGASUS_TEST_ASSERT(ReadPidFile(TEST_PID_FILE, &pid) != 0);

    /* Test PID file with invalid content */

    pidFile = fopen(TEST_PID_FILE, "a");
    PEGASUS_TEST_ASSERT(pidFile != 0);
    fprintf(pidFile, "%s", "A");
    fclose(pidFile);
    PEGASUS_TEST_ASSERT(ReadPidFile(TEST_PID_FILE, &pid) != 0);

    /* Test PID file with valid content */

    unlink(TEST_PID_FILE);
    pidFile = fopen(TEST_PID_FILE, "a");
    PEGASUS_TEST_ASSERT(pidFile != 0);
    fprintf(pidFile, "%s", "1234");
    fclose(pidFile);
    PEGASUS_TEST_ASSERT(ReadPidFile(TEST_PID_FILE, &pid) == 0);
    PEGASUS_TEST_ASSERT(pid == 1234);

    unlink(TEST_PID_FILE);
}

void testTestProcessRunning(void)
{
    FILE* pidFile;

    /* Test with non-existent PID file */

    unlink(TEST_PID_FILE);
    PEGASUS_TEST_ASSERT(
        TestProcessRunning(TEST_PID_FILE, "TestExProcess") != 0);

    /* Test with our process ID in PID file but incorrect process name */

    unlink(TEST_PID_FILE);
    pidFile = fopen(TEST_PID_FILE, "a");
    PEGASUS_TEST_ASSERT(pidFile != 0);
    fprintf(pidFile, "%d", (int)getpid());
    fclose(pidFile);
    PEGASUS_TEST_ASSERT(
        TestProcessRunning(TEST_PID_FILE, "NotOurProcess") != 0);

    /* Test with our process ID in PID file and correct process name */

    PEGASUS_TEST_ASSERT(
        TestProcessRunning(TEST_PID_FILE, "TestExProcess") == 0);

    unlink(TEST_PID_FILE);
}

int main()
{
#ifndef PEGASUS_TEST_VALGRIND
    testGetProcessName();
    testReadPidFile();
    testTestProcessRunning();

    printf("+++++ passed all tests\n");
#endif

    return 0;
}
