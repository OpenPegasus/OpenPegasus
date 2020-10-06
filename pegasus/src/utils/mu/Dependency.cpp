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
#include <utils/mu/Dependency.h>
#include <cstdio>
#include <cstddef>
#include <cstring>

void ErrorExit(const char* programName, const string& message)
{
    fprintf(stderr, "%s: Error: %s\n", programName, message.c_str());
    exit(1);
}

void Warning(const char* programName, string& message)
{
    fprintf(stderr, "%s: Warning: %s\n", programName, message.c_str());
}

void PrintVector(const vector<string>& v)
{
    for (size_t i = 0; i < v.size(); i++)
    {
        printf("%s\n", v[i].c_str());
    }
}

////////////////////////////////////////////////////////////////////////////////
//
// GetIncludePath():
//
//     Get the include path from an #include directive.
//
////////////////////////////////////////////////////////////////////////////////

bool GetIncludePath(
    const string& fileName,
    size_t lineNumber,
    const char* line,
    string& path,
    char& openDelim)
{
    if (line[0] == '#')
    {
        const char* p = line + 1;

        // Skip whitespace:

        while (isspace(*p))
        {
            p++;
        }
        // Check for "include" keyword:

        const char INCLUDE[] = "include";

        if (memcmp(p, INCLUDE, sizeof(INCLUDE) - 1) == 0)
        {
            // Advance past "include" keyword:

            p += sizeof(INCLUDE) - 1;

            // Skip whitespace:

            while (isspace(*p))
            {
                p++;
            }

            // Expect opening '"' or '<':

            if (*p != '"' && *p != '<')
            {
                return false;
#if 0
                // ATTN: noticed that "#include /**/ <path>" style not
                // handle so just returning silently when this situration
                // encountered!

                char message[64];
                sprintf(message,
                    "corrupt #include directive at %s(%d)",
                    fileName.c_str(),
                    lineNumber);
                ErrorExit(message);
#endif
            }

            openDelim = *p++;

            // Skip whitespace:

            while (isspace(*p))
            {
                p++;
            }
            // Store pointer to start of path:

            const char* start = p;

            // Look for closing '"' or '>':

            while (*p && *p != '"' && *p != '>')
            {
                p++;
            }

            if (*p != '"' && *p != '>')
            {
                return false;
#if 0
                char message[64];
                sprintf(message,
                    "corrupt #include directive at %s(%d)",
                    fileName.c_str(),
                    lineNumber);
                ErrorExit(message);
#endif
            }

            // Find end of the path (go backwards, skipping whitespace
            // between the closing delimiter and the end of the path:

            if (p == start)
            {
                return false;
#if 0
                char message[64];
                sprintf(message,
                    "empty path in #include directive at %s(%d)",
                    fileName.c_str(),
                    lineNumber);
                ErrorExit(message);
#endif
            }

            p--;

            while (isspace(*p))
            {
                p--;
            }

            if (p == start)
            {
                return false;
#if 0
                char message[64];
                sprintf(message,
                    "empty path in #include directive at %s(%d)",
                    fileName.c_str(),
                    lineNumber);
                ErrorExit(message);
#endif
            }

            path.assign(start, p - start + 1);
            return true;
        }
    }

    return false;
}

FILE* FindFile(
    const vector<string>& includePath,
    const string& prependDir,
    const string& path,
    char openDelim,
    string& fullPath)
{
    // If the opening delimiter was '"', then check the current
    // directory first:
    if (openDelim == '"')
    {
        if (prependDir.size())
        {
            GetFileFullPath(prependDir, path, fullPath);
        }
        else
        {
            fullPath = path;
        }

        FILE* fp = fopen(fullPath.c_str(), "rb");
        if (fp)
        {
            return fp;
        }
    }

    // Search the include path for the file:

    vector<string>::const_iterator first = includePath.begin();
    vector<string>::const_iterator last = includePath.end();

    for (; first != last; first++)
    {
        fullPath = *first;
        fullPath += '/';
        fullPath += path;

        FILE* fp = fopen(fullPath.c_str(), "rb");

        if (fp)
        {
            return fp;
        }
    }

    return NULL;
}

void ProcessFile(
    const string& fileName,
    const char* programName,
    FILE* fp,
    const vector<string>& includePath,
    string& prependDir,
    size_t nesting,
    set<string, less<string> >& cache,
    PrintFunc printFunc,
    bool& warn)
{
    printFunc(fileName);

    if (nesting == 100)
    {
        ErrorExit(programName,
            "Infinite include file recursion? nesting level reached 100");
    }

    if(fp == NULL)
    {
        ErrorExit(programName,
            "Input error, the fp is NULL\n");
    }
    
    // For each line in the file:

    char line[4096];
    size_t lineNumber = 1;

    for (; fgets(line, sizeof(line), fp) != NULL; lineNumber++)
    {
        // Check for include directive:

        string path;
        char openDelim;

        if (line[0] == '#' &&
            GetIncludePath(fileName, lineNumber, line, path, openDelim))
        {
            // ATTN: danger! not distinguising between angle brack delimited
            // and quote delimited paths!

            set<string, less<string> >::const_iterator pos
                = cache.find(path);

            if (pos != cache.end())
            {
                continue;
            }

            cache.insert(path);

            string fullPath;
            FILE* includeFp =
                FindFile(includePath, prependDir, path, openDelim, fullPath);

            if (!includeFp)
            {
                if (warn)
                {
                    string message = "header file not found: " + path +
                        " included from " + fileName;
                    Warning(programName, message);
                }
            }
            else
            {
                ProcessFile(fullPath, programName, includeFp, includePath,
                    prependDir, nesting + 1, cache, printFunc, warn);
            }
        }
    }

    fclose(fp);
}

