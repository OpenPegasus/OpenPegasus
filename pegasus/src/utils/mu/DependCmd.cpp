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

#include "Files.h"
#include "Dependency.h"
#include "DependCmd.h"
#include <cstdio>
#include <cstddef>
#include <cstring>

#if defined (OS_WINDOWS) || defined (OS_VMS)
# define OBJ_EXT ".obj"
#else
# define OBJ_EXT ".o"
#endif

static string objectFileName;

void PrintDependency(const string& fileName)
{
    printf("%s: %s\n\n", objectFileName.c_str(), fileName.c_str());
}

void ProcessDependCmdOptions(
    int& argc,
    char**& argv,
    const char* programName,
    vector<string>& includePath,
    string& objectDir,
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
        else if (*p == 'O')
        {
            if (*++p)
            {
                objectDir = p;
            }
            else
            {
                ErrorExit(programName, "Missing argument for -O option");
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

int DependCmdMain(int argc, char** argv)
{
    // Check arguments:

    if (argc == 1)
    {
        fprintf(stderr,
            "Usage: %s [-W]? [-Oobject_dir]? [-Iinclude_dir]* source_files...\n"
            "Where: \n"
            "    -W - warn about include files which cannot be found\n"
            "    -O - prepend this directory to object files\n"
            "    -I - search this directory for header files\n",
            argv[0]);
        exit(1);
    }

    // Extract the program name:

    const char *programName = argv[0];
    argc--;
    argv++;

    // Process all options:

    vector<string> includePath;
    string objectDir;
    bool warn = false;

    ProcessDependCmdOptions(argc, argv, programName, includePath, objectDir,
        warn);

    // There must be at least one source file; print error if not:

    if (argc < 1)
    {
        ErrorExit(programName, "no source files given");
    }

    // Process each file:

    for (int i = 0; i < argc; i++)
    {
        string fileName = argv[i];

        // Open the file:
        FILE* fp = fopen(argv[i], "rb");

        if (fp == NULL)
        {
            string message = "failed to open file: \"" + fileName + "\"";
            ErrorExit(programName, message);
        }

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

        objectFileName = "";
        if (objectDir.size())
        {
            objectFileName = objectDir;
            objectFileName += '/';
        }

        objectFileName.append(start, dot - start);
        objectFileName += OBJ_EXT;

        set<string, less<string> > cache;

        // both 'Depend' and 'SrcList' commands share the code
        // "ProcessFile", which takes an argument 'prependDir'.
        // for 'Depend' command, we pass in an empty string
        string prependDir = "";
        ProcessFile(fileName, programName, fp, includePath, prependDir, 0,
            cache, PrintDependency, warn);
    }

    return 0;
}

int DependCmd(const vector<string>& args)
{
    // Dummy up argc/argv structures and call DependCmdMain():

    int argc = static_cast<int>(args.size());
    char** argv = new char*[args.size()];

    for (int i = 0; i < argc; i++)
    {
        argv[i] = (char*)args[i].c_str();
    }

    int result = DependCmdMain(argc, argv);

    delete [] argv;

    return result;
}
