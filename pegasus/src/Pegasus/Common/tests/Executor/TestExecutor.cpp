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

#include <Pegasus/Common/Executor.h>
#include <Pegasus/Common/PegasusAssert.h>

#include <cstdlib>
#include <iostream>

PEGASUS_USING_PEGASUS;
PEGASUS_USING_STD;

void testExecutorLoopbackImpl()
{
    PEGASUS_TEST_ASSERT(Executor::detectExecutor() == -1);
    PEGASUS_TEST_ASSERT(Executor::ping() == -1);
    PEGASUS_TEST_ASSERT(Executor::daemonizeExecutor() == -1);

    FILE* file;
    const char* fileName = "testfile";
    const char* newFileName = "renamedtestfile";

    Executor::removeFile(fileName);

    file = Executor::openFile(fileName, 'r');
    PEGASUS_TEST_ASSERT(file == NULL);
    file = Executor::openFile(fileName, 'w');
    PEGASUS_TEST_ASSERT(file != NULL);
    fclose(file);
    file = Executor::openFile(fileName, 'a');
    PEGASUS_TEST_ASSERT(file != NULL);
    fclose(file);

    PEGASUS_TEST_ASSERT(Executor::renameFile(fileName, newFileName) == 0);
    PEGASUS_TEST_ASSERT(Executor::renameFile(fileName, newFileName) == -1);

    PEGASUS_TEST_ASSERT(Executor::removeFile(newFileName) == 0);
    PEGASUS_TEST_ASSERT(Executor::removeFile(newFileName) == -1);

#if defined(PEGASUS_ENABLE_PRIVILEGE_SEPARATION)
    int pid;
    AnonymousPipe* readPipe;
    AnonymousPipe* writePipe;
    PEGASUS_TEST_ASSERT(Executor::startProviderAgent(
        1, "MyModule", "HomeDir", "me", pid, readPipe, writePipe) == -1);

    PEGASUS_TEST_ASSERT(Executor::reapProviderAgent(123) == -1);
#endif

    PEGASUS_TEST_ASSERT(Executor::authenticatePassword(
        "xnonexistentuserx", "wrongpassword") != 0);
    PEGASUS_TEST_ASSERT(Executor::validateUser("xnonexistentuserx") != 0);

    char challengeFilePath[EXECUTOR_BUFFER_SIZE];
    PEGASUS_TEST_ASSERT(Executor::challengeLocal(
        "me", challengeFilePath) == -1);
    PEGASUS_TEST_ASSERT(Executor::authenticateLocal(
        challengeFilePath, "secret") == -1);

    PEGASUS_TEST_ASSERT(Executor::updateLogLevel("WARNING") == 0);
}

#if defined(PEGASUS_ENABLE_PRIVILEGE_SEPARATION)
void testExecutorSocketImpl()
{
    Executor::setSock(10);
    PEGASUS_TEST_ASSERT(Executor::detectExecutor() == 0);

    // These calls fail because the socket is not valid
    PEGASUS_TEST_ASSERT(Executor::ping() == -1);
    PEGASUS_TEST_ASSERT(Executor::daemonizeExecutor() == -1);

    PEGASUS_TEST_ASSERT(Executor::openFile("abc", 'r') == NULL);
    PEGASUS_TEST_ASSERT(Executor::openFile("abc", 'w') == NULL);
    PEGASUS_TEST_ASSERT(Executor::openFile("abc", 'a') == NULL);
    PEGASUS_TEST_ASSERT(Executor::openFile("abc", 'z') == NULL);

    PEGASUS_TEST_ASSERT(Executor::renameFile("abc", "def") == -1);
    PEGASUS_TEST_ASSERT(Executor::removeFile("abc") == -1);

    int pid;
    AnonymousPipe* readPipe;
    AnonymousPipe* writePipe;
    PEGASUS_TEST_ASSERT(Executor::startProviderAgent(
        1, "MyModule", "HomeDir", "me", pid, readPipe, writePipe) == -1);

    PEGASUS_TEST_ASSERT(Executor::reapProviderAgent(123) == 0);

    PEGASUS_TEST_ASSERT(Executor::authenticatePassword(
        "xnonexistentuserx", "wrongpassword") == -1);
    PEGASUS_TEST_ASSERT(Executor::validateUser("xnonexistentuserx") == -1);

    char challengeFilePath[EXECUTOR_BUFFER_SIZE];
    PEGASUS_TEST_ASSERT(Executor::challengeLocal(
        "me", challengeFilePath) == -1);
    PEGASUS_TEST_ASSERT(Executor::authenticateLocal(
        challengeFilePath, "secret") == -1);

    PEGASUS_TEST_ASSERT(Executor::updateLogLevel("WARNING") == -1);
}
#endif

int main(int argc, char** argv)
{
    if ((argc != 2) ||
        ((strcmp(argv[1], "loopback") != 0)
#if defined(PEGASUS_ENABLE_PRIVILEGE_SEPARATION)
         && (strcmp(argv[1], "socket") != 0)
#endif
       ))
    {
        cerr << "Usage: " << argv[0] << " (loopback | socket)" << endl;
        exit(1);
    }

    try
    {
        if (strcmp(argv[1], "loopback") == 0)
        {
            testExecutorLoopbackImpl();
        }
#if defined(PEGASUS_ENABLE_PRIVILEGE_SEPARATION)
        else
        {
            testExecutorSocketImpl();
        }
#endif

        cout << argv[0] << " +++++ passed all tests" << endl;
    }
    catch (Exception& e)
    {
        cerr << argv[0] << " Exception " << e.getMessage() << endl;
        exit(1);
    }
    return 0;
}
