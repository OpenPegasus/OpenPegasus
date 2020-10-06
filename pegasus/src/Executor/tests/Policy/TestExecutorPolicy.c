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

#include <Executor/Policy.h>
#include <Executor/Macro.h>
#include <Executor/tests/TestAssert.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define TEST_DUMP_FILE "dumpfile"
#define MAX_DUMP_SIZE 4096

static struct Policy _testPolicyTable[] =
{
    {
        EXECUTOR_PING_MESSAGE,
        NULL,
        NULL,
        100, /* flags */
    },
    {
        EXECUTOR_RENAME_FILE_MESSAGE,
        "${file1}",
        "${file2}",
        0, /* flags */
    },
    {
        EXECUTOR_RENAME_FILE_MESSAGE,
        "file1",
        "${file2}",
        0, /* flags */
    },
    {
        EXECUTOR_RENAME_FILE_MESSAGE,
        "file1",
        "file2",
        0, /* flags */
    }
};

static const size_t _testPolicyTableSize =
    sizeof(_testPolicyTable) / sizeof(_testPolicyTable[0]);

void testCheckPolicy(void)
{
    unsigned long flags = 0;

    /* Test non-existent policy */
    PEGASUS_TEST_ASSERT(CheckPolicy(
        _testPolicyTable,
        _testPolicyTableSize,
        EXECUTOR_UPDATE_LOG_LEVEL_MESSAGE,
        NULL,
        NULL,
        &flags) != 0);
    PEGASUS_TEST_ASSERT(flags == 0);

    /* Test policy with no arguments, but with 'flags' attribute */
    flags = 0;
    PEGASUS_TEST_ASSERT(CheckPolicy(
        _testPolicyTable,
        _testPolicyTableSize,
        EXECUTOR_PING_MESSAGE,
        NULL,
        NULL,
        &flags) == 0);
    PEGASUS_TEST_ASSERT(flags == 100);

    /* Test policies with invalid macro expansion in first argument and
     * non-match in first argument
     */
    PEGASUS_TEST_ASSERT(CheckPolicy(
        _testPolicyTable,
        _testPolicyTableSize,
        EXECUTOR_RENAME_FILE_MESSAGE,
        "MyFile",
        "file2",
        NULL) != 0);

    /* Test policies with invalid macro expansion in second argument and
     * non-match in second argument
     */
    PEGASUS_TEST_ASSERT(CheckPolicy(
        _testPolicyTable,
        _testPolicyTableSize,
        EXECUTOR_RENAME_FILE_MESSAGE,
        "file1",
        "MyFile",
        NULL) != 0);

    /* Test policy with successful match in both arguments */
    PEGASUS_TEST_ASSERT(CheckPolicy(
        _testPolicyTable,
        _testPolicyTableSize,
        EXECUTOR_RENAME_FILE_MESSAGE,
        "file1",
        "file2",
        NULL) == 0);
}

void testFilePolicies(void)
{
    const char* currentConfigFile = "MyConfigFile";
    const char* currentConfigFileBak = "MyConfigFile.bak";
    const char* noAccessFile = "NoAccessFile";

    /* Define a macro used in the static policy table */
    DefineMacro("currentConfigFilePath", currentConfigFile);

    PEGASUS_TEST_ASSERT(CheckOpenFilePolicy(currentConfigFile, 'w', NULL) == 0);
    PEGASUS_TEST_ASSERT(CheckOpenFilePolicy(noAccessFile, 'w', NULL) != 0);

    PEGASUS_TEST_ASSERT(CheckRemoveFilePolicy(currentConfigFile) == 0);
    PEGASUS_TEST_ASSERT(CheckRemoveFilePolicy(noAccessFile) != 0);

    PEGASUS_TEST_ASSERT(
        CheckRenameFilePolicy(currentConfigFile, currentConfigFileBak) == 0);
    PEGASUS_TEST_ASSERT(
        CheckRenameFilePolicy(currentConfigFile, noAccessFile) != 0);
}

void testDumpPolicy(void)
{
    FILE* dumpFile;
    char dumpFileBuffer[MAX_DUMP_SIZE];
    size_t numBytesRead;

    unlink(TEST_DUMP_FILE);

    /* Test DumpPolicy() with expandMacros=false */
    {
        const char* expectedDumpResult =
            "===== Policy:\n"
            "OpenFile(\"${currentConfigFilePath}\", \"w\")\n"
            "RenameFile(\"${currentConfigFilePath}\", "
                "\"${currentConfigFilePath}.bak\")\n"
            "RemoveFile(\"${currentConfigFilePath}\")\n"
            "RemoveFile(\"${currentConfigFilePath}.bak\")\n"
            "OpenFile(\"${plannedConfigFilePath}\", \"w\")\n"
            "RenameFile(\"${plannedConfigFilePath}\", "
                "\"${plannedConfigFilePath}.bak\")\n"
            "RemoveFile(\"${plannedConfigFilePath}\")\n"
            "RemoveFile(\"${plannedConfigFilePath}.bak\")\n"
            "OpenFile(\"${passwordFilePath}\", \"w\")\n"
            "RenameFile(\"${passwordFilePath}.bak\", \"${passwordFilePath}\")\n"
            "RenameFile(\"${passwordFilePath}\", \"${passwordFilePath}.bak\")\n"
            "RemoveFile(\"${passwordFilePath}.bak\")\n"
            "RemoveFile(\"${passwordFilePath}\")\n"
            "OpenFile(\"${sslKeyFilePath}\", \"r\")\n"
            "OpenFile(\"${sslTrustStore}/*\", \"w\")\n"
            "RemoveFile(\"${sslTrustStore}/*\")\n"
            "OpenFile(\"${crlStore}/*\", \"w\")\n"
            "RemoveFile(\"${crlStore}/*\")\n"
            "RemoveFile(\"${localAuthDir}/*\")\n"
            "\n";

        dumpFile = fopen(TEST_DUMP_FILE, "a");
        PEGASUS_TEST_ASSERT(dumpFile != 0);
        DumpPolicy(dumpFile, 0);
        fclose(dumpFile);

        dumpFile = fopen(TEST_DUMP_FILE, "rb");
        PEGASUS_TEST_ASSERT(dumpFile != 0);
        memset(dumpFileBuffer, 0, MAX_DUMP_SIZE);
        numBytesRead =
            fread(dumpFileBuffer, sizeof(char), MAX_DUMP_SIZE - 1, dumpFile);
        PEGASUS_TEST_ASSERT(numBytesRead != 0);
        fclose(dumpFile);

        PEGASUS_TEST_ASSERT(strcmp(dumpFileBuffer, expectedDumpResult) == 0);

        unlink(TEST_DUMP_FILE);
    }

    /* Test DumpPolicyHelper() with expandMacros=false */
    {
        const char* expectedDumpResult =
            "Ping()\n"
            "RenameFile(\"${file1}\", \"${file2}\")\n"
            "RenameFile(\"file1\", \"${file2}\")\n"
            "RenameFile(\"file1\", \"file2\")\n";

        dumpFile = fopen(TEST_DUMP_FILE, "a");
        PEGASUS_TEST_ASSERT(dumpFile != 0);
        DumpPolicyHelper(dumpFile, _testPolicyTable, _testPolicyTableSize, 0);
        fclose(dumpFile);

        dumpFile = fopen(TEST_DUMP_FILE, "rb");
        PEGASUS_TEST_ASSERT(dumpFile != 0);
        memset(dumpFileBuffer, 0, MAX_DUMP_SIZE);
        numBytesRead =
            fread(dumpFileBuffer, sizeof(char), MAX_DUMP_SIZE - 1, dumpFile);
        PEGASUS_TEST_ASSERT(numBytesRead != 0);
        fclose(dumpFile);

        PEGASUS_TEST_ASSERT(strcmp(dumpFileBuffer, expectedDumpResult) == 0);

        unlink(TEST_DUMP_FILE);
    }

    /* Test DumpPolicyHelper() with expandMacros=true */
    {
        const char* expectedDumpResult =
            "Ping()\n"
            "RenameFile(\"MyFile1\", \"MyFile2\")\n"
            "RenameFile(\"file1\", \"MyFile2\")\n"
            "RenameFile(\"file1\", \"file2\")\n";

        DefineMacro("file1", "MyFile1");
        DefineMacro("file2", "MyFile2");

        dumpFile = fopen(TEST_DUMP_FILE, "a");
        PEGASUS_TEST_ASSERT(dumpFile != 0);
        DumpPolicyHelper(dumpFile, _testPolicyTable, _testPolicyTableSize, 1);
        fclose(dumpFile);

        UndefineMacro("file1");
        UndefineMacro("file2");

        dumpFile = fopen(TEST_DUMP_FILE, "rb");
        PEGASUS_TEST_ASSERT(dumpFile != 0);
        memset(dumpFileBuffer, 0, MAX_DUMP_SIZE);
        numBytesRead =
            fread(dumpFileBuffer, sizeof(char), MAX_DUMP_SIZE - 1, dumpFile);
        PEGASUS_TEST_ASSERT(numBytesRead != 0);
        fclose(dumpFile);

        PEGASUS_TEST_ASSERT(strcmp(dumpFileBuffer, expectedDumpResult) == 0);

        unlink(TEST_DUMP_FILE);
    }
}

int main()
{
    testCheckPolicy();
    testFilePolicies();
    testDumpPolicy();

    printf("+++++ passed all tests\n");

    return 0;
}
