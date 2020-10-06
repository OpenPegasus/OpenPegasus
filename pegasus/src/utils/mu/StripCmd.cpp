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
// Author: Michael E. Brasher
//
//%=============================================================================

#include <iostream>
#include <fstream>
#include "Files.h"
#include "StripCmd.h"
#include "Files.h"

static int _Strip(
    const string& arg0,
    const string& startPattern,
    const string& endPattern,
    const string& fileName)
{
    // -- Open input file:

    ifstream is(fileName.c_str());

    if (!is)
    {
        cerr << arg0 << ": failed to open \"" << fileName << "\"" << endl;
        return 1;
    }

    string tmpFileName = fileName + ".tmp";


    // -- Open output file:

    ofstream os(tmpFileName.c_str());

    if (!os)
    {
        cerr << arg0 << ": failed to open \"" << tmpFileName << "\"" << endl;
        return 1;
    }

    // -- Strip out the unwanted lines:

    bool inside = false;
    bool foundStart = false;
    bool foundEnd = false;
    string line;

    while (getline(is, line))
    {
        if (!foundStart && line.substr(0, startPattern.size()) == startPattern)
        {
            foundStart = true;
            inside = true;
            continue;
        }

        if (!inside)
            os << line << endl;

        if (!foundEnd && line.substr(0, endPattern.size()) == endPattern)
        {
            foundEnd = true;
            inside = false;
        }
    }

    is.close();
    os.close();

    if (!foundStart || !foundEnd)
    {
        RemoveFile(tmpFileName);
        return 0;
    }


    // -- Copy the temporary file back over the original:

    if (!CopyFile(tmpFileName, fileName))
    {
        cerr << arg0 << ": failed to copy file" << endl;
        return 1;
    }

    // -- Remove the temporary file:

    RemoveFile(tmpFileName);

    return 0;
}

int StripCmd(const vector<string>& args)
{
    // -- Check arguments:

    if (args.size() < 4)
    {
        cerr << args[0] << ": insufficient arguments" << endl;
        return 1;
    }

    // -- Create complete glob list:

    vector<string> fileNames;

    for (size_t i = 3; i < args.size(); i++)
        Glob(args[i], fileNames);

    for (size_t j = 0; j < fileNames.size(); j++)
    {
        int result = _Strip(args[0], args[1], args[2], fileNames[j]);

        if (result != 0)
            return result;
    }

    return 0;
}
