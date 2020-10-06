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

#include <Executor/Macro.h>
#include <Executor/Globals.h>
#include <Executor/tests/TestAssert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#define TEST_DUMP_FILE "dumpfile"
#define MAX_DUMP_SIZE 1024

static void UnsetEnvironmentVariable(const char* name)
{
    int namelen = strlen(name);
    int i;

    for (i = 0; environ[i]; i++)
    {
        if ((strncmp(name, environ[i], namelen) == 0) &&
            (environ[i][namelen] == '='))
        {
            break;
        }
    }

    if (environ[i])
    {
        int j = i;
        while (environ[++j])
            ;
        environ[i] = environ[j-1];
        environ[j-1] = 0;
    }
}

int main()
{
    /* Test DefineMacro() */
    {
        PEGASUS_TEST_ASSERT(DefineMacro("x", "100") == 0);
        PEGASUS_TEST_ASSERT(DefineMacro("y", "hello") == 0);
        PEGASUS_TEST_ASSERT(DefineMacro("z", "true") == 0);
        PEGASUS_TEST_ASSERT(DefineMacro("z", "false") != 0);
    }

    /* Test FindMacro() */
    {
        const char* x;
        const char* y;
        const char* z;

        x = FindMacro("x");
        PEGASUS_TEST_ASSERT(x != NULL && strcmp(x, "100") == 0);

        y = FindMacro("y");
        PEGASUS_TEST_ASSERT(y != NULL && strcmp(y, "hello") == 0);

        z = FindMacro("z");
        PEGASUS_TEST_ASSERT(z != NULL && strcmp(z, "true") == 0);
    }

    /* Test ExpandMacro() */
    {
        char buffer[EXECUTOR_BUFFER_SIZE];

        PEGASUS_TEST_ASSERT(ExpandMacros("${x} ${y} ${z}", buffer) == 0);
        PEGASUS_TEST_ASSERT(strcmp(buffer, "100 hello true") == 0);

        PEGASUS_TEST_ASSERT(ExpandMacros("${x}$", buffer) == 0);
        PEGASUS_TEST_ASSERT(strcmp(buffer, "100$") == 0);

        PEGASUS_TEST_ASSERT(ExpandMacros("${x", buffer) != 0);
        PEGASUS_TEST_ASSERT(ExpandMacros("${a}", buffer) != 0);
    }

    /* Test DumpMacros() */
    {
        FILE* dumpFile;
        char dumpFileBuffer[MAX_DUMP_SIZE];
        size_t numBytesRead;
        const char* expectedDumpResult =
            "===== Macros:\n"
            "x=100\n"
            "y=hello\n"
            "z=true\n"
            "\n";

        unlink(TEST_DUMP_FILE);

        dumpFile = fopen(TEST_DUMP_FILE, "a");
        PEGASUS_TEST_ASSERT(dumpFile != 0);
        DumpMacros(dumpFile);
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

    /* Test UndefineMacro() */
    {
        PEGASUS_TEST_ASSERT(UndefineMacro("a") != 0);
        PEGASUS_TEST_ASSERT(UndefineMacro("x") == 0);
        PEGASUS_TEST_ASSERT(UndefineMacro("z") == 0);
    }

    /* Test DefineConfigPathMacro() */
    {
        static const char* argv[] =
            { "program", "option1=one", "option2=/two" };
        static const int argc = sizeof(argv) / sizeof(argv[0]);
        const char* macroDef;

        globals.argv = (char**)argv;
        globals.argc = argc;

        PEGASUS_TEST_ASSERT(
            DefineConfigPathMacro("ImportantPath", "/var/important") == 0);
        macroDef = FindMacro("ImportantPath");
        PEGASUS_TEST_ASSERT(macroDef != NULL);
        PEGASUS_TEST_ASSERT(strcmp(macroDef, "/var/important") == 0);
        PEGASUS_TEST_ASSERT(UndefineMacro("ImportantPath") == 0);

        PEGASUS_TEST_ASSERT(DefineConfigPathMacro("option2", "/var/two") == 0);
        macroDef = FindMacro("option2");
        PEGASUS_TEST_ASSERT(macroDef != NULL);
        PEGASUS_TEST_ASSERT(strcmp(macroDef, "/two") == 0);
        PEGASUS_TEST_ASSERT(UndefineMacro("option2") == 0);
    }

    /* Remove PEGASUS_HOME from the environment */

    UnsetEnvironmentVariable("PEGASUS_HOME");

    /* Test DefineConfigPathMacro() with no PEGASUS_HOME defined */
    {
        static const char* argv[] =
            { "program", "option1=one", "option2=/two" };
        static const int argc = sizeof(argv) / sizeof(argv[0]);

        globals.argv = (char**)argv;
        globals.argc = argc;

        PEGASUS_TEST_ASSERT(DefineConfigPathMacro("option1", "one") != 0);
        PEGASUS_TEST_ASSERT(FindMacro("option1") == NULL);

        PEGASUS_TEST_ASSERT(DefineConfigPathMacro("option3", "three") != 0);
        PEGASUS_TEST_ASSERT(FindMacro("option3") == NULL);
    }

    printf("+++++ passed all tests\n");

    return 0;
}
