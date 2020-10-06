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

#include <Executor/Socket.h>
#include <Executor/tests/TestAssert.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

static const char token[] = "A65F5382BC3D4F12AE24A1F6110015AF";

void Child(int sock)
{
    char buffer[EXECUTOR_BUFFER_SIZE];
    int fd;

    /* Echo token from parent. */

    memset(buffer, 0xFF, sizeof(buffer));
    PEGASUS_TEST_ASSERT(
        RecvNonBlock(sock, buffer, sizeof(token)) == sizeof(token));
    PEGASUS_TEST_ASSERT(strcmp(buffer, token) == 0);

    /* Open file for parent and pass descriptor back. */

    fd = open("token.dat", O_RDONLY);
    PEGASUS_TEST_ASSERT(fd >= 0);

    /* Send descriptor back to parent. */

    PEGASUS_TEST_ASSERT(
        SendNonBlock(sock, buffer, sizeof(token)) == sizeof(token));
    PEGASUS_TEST_ASSERT(SendDescriptorArray(sock, &fd, 1) == 0);
    PEGASUS_TEST_ASSERT(close(fd) == 0);
    exit(55);
}

void Parent(int pid, int sock)
{
    char buffer[EXECUTOR_BUFFER_SIZE];
    int status;
    int fd;

    /* Send token to child and read it back. */

    memset(buffer, 0xFF, sizeof(buffer));
    PEGASUS_TEST_ASSERT(
        SendNonBlock(sock, token, sizeof(token)) == sizeof(token));
    PEGASUS_TEST_ASSERT(
        RecvNonBlock(sock, buffer, sizeof(token)) == sizeof(token));
    PEGASUS_TEST_ASSERT(strcmp(token, buffer) == 0);

    /* Wait for descriptor from child. */

    memset(buffer, 0xFF, sizeof(buffer));

    PEGASUS_TEST_ASSERT(SetBlocking(sock) == 0);
    PEGASUS_TEST_ASSERT(RecvDescriptorArray(sock, &fd, 1) == 0);
    PEGASUS_TEST_ASSERT(SetNonBlocking(sock) == 0);

    PEGASUS_TEST_ASSERT(read(fd, buffer, sizeof(token)) == sizeof(token));
    PEGASUS_TEST_ASSERT(close(fd) == 0);
    PEGASUS_TEST_ASSERT(strcmp(token, buffer) == 0);

    /* Delete the file. */
    PEGASUS_TEST_ASSERT(unlink("token.dat") == 0);

    /* Wait for child to exit. */

    waitpid(pid, &status, 0);

    PEGASUS_TEST_ASSERT(WIFEXITED(status));
    PEGASUS_TEST_ASSERT(WEXITSTATUS(status) == 55);

    printf("+++++ passed all tests\n");
    exit(0);
}

int main()
{
    int pair[2];
    int pid;

    /* Create a file with a token in it. */
    {
        int fd;
        fd = open("token.dat", O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
        PEGASUS_TEST_ASSERT(fd >= 0);
        PEGASUS_TEST_ASSERT(write(fd, token, sizeof(token)) == sizeof(token));
        close(fd);
    }

    /* Create socket pair for talking to child. */

    PEGASUS_TEST_ASSERT(CreateSocketPair(pair) == 0);
    PEGASUS_TEST_ASSERT(SetNonBlocking(pair[0]) == 0);
    PEGASUS_TEST_ASSERT(SetNonBlocking(pair[1]) == 0);

    /* Fork child. */

    pid = fork();
    PEGASUS_TEST_ASSERT(pid >= 0);

    /* Child */

    if (pid == 0)
    {
        close(pair[1]);
        Child(pair[0]);
    }

    /* Parent */

    if (pid != 0)
    {
        close(pair[0]);
        Parent(pid, pair[1]);
    }

    printf("+++++ passed all tests\n");

    return 0;
}
