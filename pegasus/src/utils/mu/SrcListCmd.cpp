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

#include "Files.h"
#include "Dependency.h"
#include "SrcListCmd.h"
#include <cstdio>
#include <cstddef>
#include <cstring>

void PrintSrcDependency(const string& fileName)
{
    printf("%s\n", fileName.c_str());
}

void ProcessSrcListOptions(
    int& argc,
    char**& argv,
    const char* programName,
    vector<string>& includePath,
    bool& warn)
{
    int i;
    for (i = 0; i < argc; i++)
    {
        const char* p = argv[i];

        if (*p != '-')
        {
            break;
        }

        p++;
        if (*p == 'I')
        {
            if (*++p)
            {
                includePath.push_back(p);
            }
            else
            {
                ErrorExit(programName, "Missing argument for -I option");
            }
        }
        else if (*p == 'W' && p[1] == '\0')
        {
            warn = true;
        }
        else
        {
            ErrorExit(programName, string("Unknown option: -") + *p);
        }
    }

    argc -= i;
    argv += i;
}

int SrcListCmdMain(int argc, char** argv)
{
    // Check arguments:

    if (argc == 1)
    {
        fprintf(stderr,
            "Usage: %s [-W]? [-Iinclude_dir]* source_files...\n"
            "Where: \n"
            "    -W - warn about include files which cannot be found\n"
            "    -I - search this directory for header files\n"
            "Example: \n"
            "mu srclist -I/var/buildMAIN/pegasus/src mu.cpp\n",
            argv[0]);
        exit(1);
    }

    // Extract the program name:

    static char *programName = argv[0];
    argc--;
    argv++;

    // Process all options:

    vector<string> includePath;
    bool warn = false;

    ProcessSrcListOptions(argc, argv, programName, includePath, warn);

    // There must be at least one source file; print error if not:

    if (argc < 1)
    {
        ErrorExit(programName, "no source files given");
    }

    // Process each file:

    for (int i = 0; i < argc; i++)
    {
        string filePath = argv[i];

        // Open the file:
        FILE* fp = fopen(argv[i], "rb");

        if (fp == NULL)
        {
            string message = "failed to open file: \"" + filePath + "\"";
            ErrorExit(programName, message);
        }

        string fileName;
        string prependDir;

        // Get absolute directory (prependDir) for the source file
        GetSrcFileDir(filePath, fileName, prependDir);

        const char* start = fileName.c_str();
        const char* dot = strrchr(start, '.');

        if( dot == NULL )
        {
            ErrorExit(programName, "bad extension: must be \".c\", \".cpp\","
                "or \".s\": " + fileName);
        }

        if ((strcmp(dot, ".cpp") != 0) &&
            (strcmp(dot, ".c") != 0) &&
            (strcmp(dot, ".s") != 0))
        {
            ErrorExit(programName, "bad extension: must be \".c\", \".cpp\","
                "or \".s\": " + fileName);
        }

        set<string, less<string> > cache;

        ProcessFile(fileName, programName, fp, includePath, prependDir,
            0, cache, PrintSrcDependency, warn);
    }

    return 0;
}

int SrcListCmd (const vector<string>& args)
{
    // Dummy up argc/argv structures and call DependCmdMain():

    int argc = static_cast<int>(args.size());
    char** argv = new char*[args.size()];

    for (int i = 0; i < argc; i++)
    {
        argv[i] = (char*)args[i].c_str();
    }

    int result = SrcListCmdMain(argc, argv);

    delete [] argv;

    return result;
}
