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
// Author: Michael E. Brasher, Jim Wunderlich
//
//%=============================================================================

#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include "SortCmd.h"

#ifdef OS_WINDOWS
# include<io.h>
# include<fcntl.h>
#endif

static int _compare(const void* px, const void* qx)
{
    return ((string*)px)->compare(*((string*)qx));
}
// **************************************************************************
// *
// * Sort command
// *
// * Usage:
// *
// * mu sort unsorted_file > sorted_file
// *
// *  where:
// *      unsorted_file      is the input file to be sorted
// *      sorted_file        is the sorted output file
// *
// *  file is sorted in ascending order
// *
// **************************************************************************


int SortCmd(const vector<string>& args)
{
    //
    // Check arguments:
    //

    if (args.size() != 2)
    {
        cerr << args[0] << ": wrong number of arguments" << endl;
        return 1;
    }

    //
    // Read file into memory.
    //

    string path = args[1];

#ifdef OS_WINDOWS
    ifstream is(path.c_str(), ios::binary);
#else
    ifstream is(path.c_str());
#endif

    vector<string> lines;
    string line;

    while (getline(is, line))
        lines.push_back(line);

    is.close();

    //
    // Sort if necessary.
    //
    if ( lines.size() > 1 )
    {
        qsort(&lines[0], lines.size(), sizeof(string), _compare);
    }

    //
    // Write the results out to standard output:
    //

#ifdef OS_WINDOWS
    setmode(1, O_BINARY);
#endif

    for (size_t i = 0; i < lines.size(); i++)
        cout << lines[i] << '\n';

    return 0;
}
