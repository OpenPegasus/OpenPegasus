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

#include <Executor/User.h>
#include <Executor/LocalAuth.h>
#include <Executor/tests/TestAssert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

void testSuccessfulAuthentication(void)
{
    char challengeFilePath[EXECUTOR_BUFFER_SIZE];
    char response[EXECUTOR_BUFFER_SIZE];
    FILE* is;

    /* Start authentication and get challenge file path. */

    PEGASUS_TEST_ASSERT(StartLocalAuthentication(
        PEGASUS_CIMSERVERMAIN_USER, challengeFilePath) == 0);

    /* Read secret token from file. */

    is = fopen(challengeFilePath, "r");
    PEGASUS_TEST_ASSERT(is != NULL);
    PEGASUS_TEST_ASSERT(fgets(response, sizeof(response), is) != NULL);

    /* Finish authentication. */

    PEGASUS_TEST_ASSERT(
        FinishLocalAuthentication(challengeFilePath, response) == 0);
}

void testCreateLocalAuthFile(void)
{
    int testUid;
    int testGid;
    PEGASUS_TEST_ASSERT(
        GetUserInfo(PEGASUS_CIMSERVERMAIN_USER, &testUid, &testGid) == 0);

    /* Test with file path that already exists */
    {
        ssize_t result;
        const char* path = "testlocalauthfile";
        int fd = open(path, O_WRONLY | O_EXCL | O_CREAT | O_TRUNC, S_IRUSR);
        EXECUTOR_RESTART(write(fd, "test", 4), result);
        close(fd);
        PEGASUS_TEST_ASSERT(CreateLocalAuthFile(path, testUid, testGid) == 0);
        unlink(path);
    }

    /* Test with non-existent directory in file path */
    {
        const char* path =
            "/tmp/nonexistentdirectory/anotherone/pegasus/localauthtestfile";
        PEGASUS_TEST_ASSERT(CreateLocalAuthFile(path, testUid, testGid) != 0);
    }
}

void testCheckLocalAuthToken(void)
{
    /* Test with file path that does not exist */
    {
        const char* path = "nonexistenttestfile";
        PEGASUS_TEST_ASSERT(CheckLocalAuthToken(path, "secret") != 0);
    }

    /* Test with secret token that is too short */
    {
        ssize_t result;
        const char* path = "testlocalauthfile";
        int fd = open(path, O_WRONLY | O_EXCL | O_CREAT | O_TRUNC, S_IRUSR);
        EXECUTOR_RESTART(write(fd, "secret", 6), result);
        close(fd);
        PEGASUS_TEST_ASSERT(CheckLocalAuthToken(path, "secret") != 0);
        unlink(path);
    }

    /* Test with incorrect secret token */
    {
        ssize_t result;
        const char* path = "testlocalauthfile";
        int fd = open(path, O_WRONLY | O_EXCL | O_CREAT | O_TRUNC, S_IRUSR);
        EXECUTOR_RESTART(
            write(fd, "1234567890123456789012345678901234567890", 40), result);
        close(fd);
        PEGASUS_TEST_ASSERT(CheckLocalAuthToken(
            path, "123456789012345678901234567890123456789X") != 0);
        unlink(path);
    }
}

void testStartLocalAuthentication(void)
{
    /* Test with non-existent user */
    {
        const char* invalidUserName = "xinvaliduserx";
        int uid;
        int gid;
        char challengeFilePath[EXECUTOR_BUFFER_SIZE];

        /* Only run this test if the user does not exist on the test system */
        if (GetUserInfo(invalidUserName, &uid, &gid) != 0)
        {
            PEGASUS_TEST_ASSERT(StartLocalAuthentication(
                invalidUserName, challengeFilePath) != 0);
        }
    }
}

void testFinishLocalAuthentication(void)
{
    /* Test with non-existent challenge file path */
    {
        const char* path = "nonexistenttestfile";
        PEGASUS_TEST_ASSERT(FinishLocalAuthentication(path, "secret") != 0);
    }
}

int main()
{
    testSuccessfulAuthentication();
    testCreateLocalAuthFile();
    testCheckLocalAuthToken();
    testStartLocalAuthentication();
    testFinishLocalAuthentication();

    printf("+++++ passed all tests\n");

    return 0;
}
