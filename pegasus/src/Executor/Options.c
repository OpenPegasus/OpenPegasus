/*
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
*/

#include "Options.h"
#include <string.h>

/*
**==============================================================================
**
** _TestFlagOption()
**
**     Check whether argv contains the given option. Return 0 if so. Else
**     return -1. Remove the argument from the list if the *remove* argument
**     is non-zero.
**
**         if (_TestFlagOption(&argc, &argv, "--help", 0) == 0)
**         {
**         }
**
**==============================================================================
*/

static int _TestFlagOption(
    int* argc_, char*** argv_, const char* option, int remove)
{
    int argc = *argc_;
    char** argv = *argv_;
    int i;

    for (i = 0; i < argc; i++)
    {
        if (strcmp(argv[i], option) == 0)
        {
            if (remove)
            {
                memmove(&argv[i], &argv[i + 1], (argc-i-1) * sizeof(char*));
                argc--;
            }

            *argc_ = argc;
            *argv_ = argv;
            return 0;
        }
    }

    /* Not found */
    return -1;
}

/*
**==============================================================================
**
** GetOptions()
**
**     Get all "minus" options from the command line. Place corresponding flags
**     into Options structure.
**
**==============================================================================
*/

void GetOptions(int* argc, char*** argv, struct Options* options)
{
    memset(options, 0, sizeof(struct Options));

    if (_TestFlagOption(argc, argv, "--dump", 1) == 0)
        options->dump = 1;

    if (_TestFlagOption(argc, argv, "--status", 1) == 0)
        options->status = 1;

    if (_TestFlagOption(argc, argv, "--version", 0) == 0)
        options->version = 1;

    if (_TestFlagOption(argc, argv, "-v", 0) == 0)
        options->version = 1;

    if (_TestFlagOption(argc, argv, "--help", 0) == 0)
        options->help = 1;

    if (_TestFlagOption(argc, argv, "-h", 0) == 0)
        options->help = 1;

    if (_TestFlagOption(argc, argv, "-s", 0) == 0)
        options->shutdown = 1;

    if (_TestFlagOption(argc, argv, "-X", 0) == 0)
        options->bindVerbose = 1;
}
