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

#include <Executor/Log.h>
#include <Executor/Globals.h>
#include <Executor/tests/TestAssert.h>
#include <stdio.h>
#include <string.h>

int main()
{
    /* Test default logLevel value */
    PEGASUS_TEST_ASSERT(GetLogLevel() == LL_INFORMATION);

    /* Test InitLogLevel() */

    /* Test with no logLevel specified.  Value is unchanged. */
    {
        static const char* argv[] = { "program" };
        static const int argc = sizeof(argv) / sizeof(argv[0]);
        globals.argv = (char**)argv;
        globals.argc = argc;

        InitLogLevel();
        PEGASUS_TEST_ASSERT(GetLogLevel() == LL_INFORMATION);
    }

    /* Test with invalid logLevel specified.  Value is unchanged. */
    {
        static const char* argv[] = { "program", "logLevel=123" };
        static const int argc = sizeof(argv) / sizeof(argv[0]);
        globals.argv = (char**)argv;
        globals.argc = argc;

        InitLogLevel();
        PEGASUS_TEST_ASSERT(GetLogLevel() == LL_INFORMATION);
    }

    /* Test with logLevel WARNING specified */
    {
        static const char* argv[] = { "program", "logLevel=WARNING" };
        static const int argc = sizeof(argv) / sizeof(argv[0]);
        globals.argv = (char**)argv;
        globals.argc = argc;

        InitLogLevel();
        PEGASUS_TEST_ASSERT(GetLogLevel() == LL_WARNING);
    }

    /* Test SetLogLevel() */

    PEGASUS_TEST_ASSERT(SetLogLevel("TRACE") == 0);
    PEGASUS_TEST_ASSERT(GetLogLevel() == LL_TRACE);
    PEGASUS_TEST_ASSERT(SetLogLevel("INFORMATION") == 0);
    PEGASUS_TEST_ASSERT(GetLogLevel() == LL_INFORMATION);
    PEGASUS_TEST_ASSERT(SetLogLevel("WARNING") == 0);
    PEGASUS_TEST_ASSERT(GetLogLevel() == LL_WARNING);
    PEGASUS_TEST_ASSERT(SetLogLevel("SEVERE") == 0);
    PEGASUS_TEST_ASSERT(GetLogLevel() == LL_SEVERE);
    PEGASUS_TEST_ASSERT(SetLogLevel("FATAL") == 0);
    PEGASUS_TEST_ASSERT(GetLogLevel() == LL_FATAL);
    PEGASUS_TEST_ASSERT(SetLogLevel("TRACEERROR") == -1);
    PEGASUS_TEST_ASSERT(GetLogLevel() == LL_FATAL);

    printf("+++++ passed all tests\n");

    return 0;
}
