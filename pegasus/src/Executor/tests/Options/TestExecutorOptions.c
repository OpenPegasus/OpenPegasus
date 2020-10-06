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

#include <Executor/Options.h>
#include <Executor/tests/TestAssert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main()
{
    /* Test removal of --dump option */
    {
        int testArgc;
        char** testArgv;
        struct Options options;

        testArgc = 4;
        testArgv = (char**)malloc(testArgc * sizeof(char*));
        PEGASUS_TEST_ASSERT(testArgv);
        testArgv[0] = "-h";
        testArgv[1] = "--dump";
        testArgv[2] = "a";
        testArgv[3] = "-s";

        GetOptions(&testArgc, &testArgv, &options);

        PEGASUS_TEST_ASSERT(testArgc == 3);
        PEGASUS_TEST_ASSERT(strcmp(testArgv[0], "-h") == 0);
        PEGASUS_TEST_ASSERT(strcmp(testArgv[1], "a") == 0);
        PEGASUS_TEST_ASSERT(strcmp(testArgv[2], "-s") == 0);

        PEGASUS_TEST_ASSERT(options.dump == 1);
        PEGASUS_TEST_ASSERT(options.version == 0);
        PEGASUS_TEST_ASSERT(options.help == 1);
        PEGASUS_TEST_ASSERT(options.shutdown == 1);
        PEGASUS_TEST_ASSERT(options.bindVerbose == 0);

        free(testArgv);
    }

    /* Test option flag settings */

    {
        int testArgc;
        char** testArgv;
        struct Options options;

        testArgc = 2;
        testArgv = (char**)malloc(testArgc * sizeof(char*));
        PEGASUS_TEST_ASSERT(testArgv);
        testArgv[0] = "-h";
        testArgv[1] = "-s";

        GetOptions(&testArgc, &testArgv, &options);

        PEGASUS_TEST_ASSERT(testArgc == 2);
        PEGASUS_TEST_ASSERT(options.dump == 0);
        PEGASUS_TEST_ASSERT(options.version == 0);
        PEGASUS_TEST_ASSERT(options.help == 1);
        PEGASUS_TEST_ASSERT(options.shutdown == 1);
        PEGASUS_TEST_ASSERT(options.bindVerbose == 0);
        free(testArgv);
    }

    {
        int testArgc;
        char** testArgv;
        struct Options options;

        testArgc = 1;
        testArgv = (char**)malloc(testArgc * sizeof(char*));
        PEGASUS_TEST_ASSERT(testArgv);
        testArgv[0] = "--dump";

        GetOptions(&testArgc, &testArgv, &options);

        PEGASUS_TEST_ASSERT(testArgc == 0);
        PEGASUS_TEST_ASSERT(options.dump == 1);
        PEGASUS_TEST_ASSERT(options.version == 0);
        PEGASUS_TEST_ASSERT(options.help == 0);
        PEGASUS_TEST_ASSERT(options.shutdown == 0);
        PEGASUS_TEST_ASSERT(options.bindVerbose == 0);
        free(testArgv);
    }

    {
        int testArgc;
        char** testArgv;
        struct Options options;

        testArgc = 2;
        testArgv = (char**)malloc(testArgc * sizeof(char*));
        PEGASUS_TEST_ASSERT(testArgv);
        testArgv[0] = "-v";
        testArgv[1] = "-X";

        GetOptions(&testArgc, &testArgv, &options);

        PEGASUS_TEST_ASSERT(testArgc == 2);
        PEGASUS_TEST_ASSERT(options.dump == 0);
        PEGASUS_TEST_ASSERT(options.version == 1);
        PEGASUS_TEST_ASSERT(options.help == 0);
        PEGASUS_TEST_ASSERT(options.shutdown == 0);
        PEGASUS_TEST_ASSERT(options.bindVerbose == 1);
        free(testArgv);
    }

    {
        int testArgc;
        char** testArgv;
        struct Options options;

        testArgc = 2;
        testArgv = (char**)malloc(testArgc * sizeof(char*));
        PEGASUS_TEST_ASSERT(testArgv);
        testArgv[0] = "--help";
        testArgv[1] = "--dump";

        GetOptions(&testArgc, &testArgv, &options);

        PEGASUS_TEST_ASSERT(testArgc == 1);
        PEGASUS_TEST_ASSERT(options.dump == 1);
        PEGASUS_TEST_ASSERT(options.version == 0);
        PEGASUS_TEST_ASSERT(options.help == 1);
        PEGASUS_TEST_ASSERT(options.shutdown == 0);
        PEGASUS_TEST_ASSERT(options.bindVerbose == 0);
        free(testArgv);
    }

    {
        int testArgc;
        char** testArgv;
        struct Options options;

        testArgc = 2;
        testArgv = (char**)malloc(testArgc * sizeof(char*));
        PEGASUS_TEST_ASSERT(testArgv);
        testArgv[0] = "-s";
        testArgv[1] = "--version";

        GetOptions(&testArgc, &testArgv, &options);

        PEGASUS_TEST_ASSERT(testArgc == 2);
        PEGASUS_TEST_ASSERT(options.dump == 0);
        PEGASUS_TEST_ASSERT(options.version == 1);
        PEGASUS_TEST_ASSERT(options.help == 0);
        PEGASUS_TEST_ASSERT(options.shutdown == 1);
        PEGASUS_TEST_ASSERT(options.bindVerbose == 0);
        free(testArgv);
    }

    {
        int testArgc;
        char** testArgv;
        struct Options options;

        testArgc = 7;
        testArgv = (char**)malloc(testArgc * sizeof(char*));
        PEGASUS_TEST_ASSERT(testArgv);
        testArgv[0] = "-s";
        testArgv[1] = "-v";
        testArgv[2] = "--version";
        testArgv[3] = "-h";
        testArgv[4] = "--help";
        testArgv[5] = "--dump";
        testArgv[6] = "-X";

        GetOptions(&testArgc, &testArgv, &options);

        PEGASUS_TEST_ASSERT(testArgc == 6);
        PEGASUS_TEST_ASSERT(options.dump == 1);
        PEGASUS_TEST_ASSERT(options.version == 1);
        PEGASUS_TEST_ASSERT(options.help == 1);
        PEGASUS_TEST_ASSERT(options.shutdown == 1);
        PEGASUS_TEST_ASSERT(options.bindVerbose == 1);
        free(testArgv);
    }

    {
        int testArgc;
        char** testArgv;
        struct Options options;

        testArgc = 1;
        testArgv = (char**)malloc(testArgc * sizeof(char*));
        PEGASUS_TEST_ASSERT(testArgv);
        testArgc = 0;

        GetOptions(&testArgc, &testArgv, &options);

        PEGASUS_TEST_ASSERT(testArgc == 0);
        PEGASUS_TEST_ASSERT(options.dump == 0);
        PEGASUS_TEST_ASSERT(options.version == 0);
        PEGASUS_TEST_ASSERT(options.help == 0);
        PEGASUS_TEST_ASSERT(options.shutdown == 0);
        PEGASUS_TEST_ASSERT(options.bindVerbose == 0);
        free(testArgv);
    }

    printf("+++++ passed all tests\n");

    return 0;
}
