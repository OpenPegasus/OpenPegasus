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
#include "PrependCmd.h"
#include "Files.h"

static int _Prepend(
    const string& arg0,
    const string& prependFile,
    const string& changeFile)
{

    // -- Open first input file:

    ifstream is1(prependFile.c_str());

    if (!is1)
    {
        cerr << arg0 << ": failed to open \"" << prependFile << "\"" << endl;
        return 1;
    }

    // -- Open second input file:

    ifstream is2(changeFile.c_str());

    if (!is2)
    {
        cerr << arg0 << ": failed to open \"" << changeFile << "\"" << endl;
        return 1;
    }

    // -- Open output temporary file:

    string tmpFileName = changeFile + ".tmp";

    ofstream os(tmpFileName.c_str());

    if (!os)
    {
        cerr << arg0 << ": failed to open \"" << tmpFileName << "\"" << endl;
        return 1;
    }

    // -- Write contents of two input files to the output file:

    string line;

    while (getline(is1, line))
        os << line << endl;

    while (getline(is2, line))
        os << line << endl;

    is1.close();
    is2.close();
    os.close();

    // -- Copy temporary file back over change file:

    if (!CopyFile(tmpFileName, changeFile))
    {
        cerr << arg0 << ": failed to copy file" << endl;
        return 1;
    }

    // -- Remove the temporary file:

    RemoveFile(tmpFileName);

    return 0;
}

int PrependCmd(const vector<string>& args)
{
    // -- Check arguments:

    if (args.size() < 3)
    {
        cerr << args[0] << ": insufficient arguments" << endl;
        return 1;
    }

    // -- Create glob list:

    vector<string> fileNames;

    for (size_t i = 2; i < args.size(); i++)
        Glob(args[i], fileNames);

    // -- For each matching file:

    for (size_t j = 0; j < fileNames.size(); j++)
    {
        int result = _Prepend(args[0], args[1], fileNames[j]);

        if (result != 0)
            return result;
    }

    return 0;
}
