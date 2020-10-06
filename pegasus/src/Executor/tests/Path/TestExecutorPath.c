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

#include <Executor/Path.h>
#include <Executor/Strlcpy.h>
#include <Executor/Strlcat.h>
#include <Executor/tests/TestAssert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

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

static int test(const char* path, const char* expect)
{
    char buffer[EXECUTOR_BUFFER_SIZE];
    DirName(path, buffer);
    return strcmp(buffer, expect) == 0 ? 0 : -1;
}

int main()
{
    /* DirName() */
    {
        PEGASUS_TEST_ASSERT(test("/aaa", "/") == 0);
        PEGASUS_TEST_ASSERT(test("/aaa/", "/") == 0);
        PEGASUS_TEST_ASSERT(test("/aaa/bbb", "/aaa") == 0);
        PEGASUS_TEST_ASSERT(test("/aaa/bbb/ccc", "/aaa/bbb") == 0);
        PEGASUS_TEST_ASSERT(test("aaa/bbb", "aaa") == 0);
        PEGASUS_TEST_ASSERT(test("aaa", ".") == 0);
        PEGASUS_TEST_ASSERT(test("aaa/", ".") == 0);
        PEGASUS_TEST_ASSERT(test("", ".") == 0);
        PEGASUS_TEST_ASSERT(test("/", "/") == 0);
        PEGASUS_TEST_ASSERT(test("////", "/") == 0);
        PEGASUS_TEST_ASSERT(test("/etc/passwd", "/etc") == 0);
    }

    /* GetHomedPath() */
    {
        char expect[EXECUTOR_BUFFER_SIZE];
        char actual[EXECUTOR_BUFFER_SIZE];
        const char* home;

        PEGASUS_TEST_ASSERT((home = getenv("PEGASUS_HOME")) != NULL);

        /* Test relative path */
        Strlcpy(expect, home, sizeof(expect));
        Strlcat(expect, "/somefile", sizeof(expect));

        PEGASUS_TEST_ASSERT(GetHomedPath("somefile", actual) == 0);
        PEGASUS_TEST_ASSERT(strcmp(expect, actual) == 0);

        /* Test absolute path */
        memset(actual, 0, sizeof(actual));
        PEGASUS_TEST_ASSERT(GetHomedPath(expect, actual) == 0);
        PEGASUS_TEST_ASSERT(strcmp(expect, actual) == 0);

        /* Test null path */
        memset(actual, 0, sizeof(actual));
        PEGASUS_TEST_ASSERT(GetHomedPath(NULL, actual) == 0);
        PEGASUS_TEST_ASSERT(strcmp(home, actual) == 0);
    }

    /* GetPegasusInternalBinDir() */
    {
        char expect[EXECUTOR_BUFFER_SIZE];
        char actual[EXECUTOR_BUFFER_SIZE];
        const char* home;

        PEGASUS_TEST_ASSERT((home = getenv("PEGASUS_HOME")) != NULL);
        Strlcpy(expect, home, sizeof(expect));
        Strlcat(expect, "/bin", sizeof(expect));

        PEGASUS_TEST_ASSERT(GetPegasusInternalBinDir(actual) == 0);
        PEGASUS_TEST_ASSERT(strcmp(expect, actual) == 0);
    }

    /* Remove PEGASUS_HOME from the environment */

    UnsetEnvironmentVariable("PEGASUS_HOME");

    /* GetHomedPath() with no PEGASUS_HOME defined */
    {
        char buffer[EXECUTOR_BUFFER_SIZE];
        PEGASUS_TEST_ASSERT(GetHomedPath("somefile", buffer) != 0);
    }

    /* GetPegasusInternalBinDir() with no PEGASUS_HOME defined */
    {
        char buffer[EXECUTOR_BUFFER_SIZE];
        PEGASUS_TEST_ASSERT(GetPegasusInternalBinDir(buffer) != 0);
    }

    printf("+++++ passed all tests\n");

    return 0;
}
