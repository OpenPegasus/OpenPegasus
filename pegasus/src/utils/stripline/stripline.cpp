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
#include <Pegasus/Common/Config.h>
#include <fstream>
#include <string>
#include <iostream>
#include <cstdlib>

#ifdef PEGASUS_OS_TYPE_WINDOWS
# include <direct.h>
#else
# include <unistd.h>
#endif

PEGASUS_USING_STD;

bool RemoveFile(const string& path)
{
#ifdef OS_TYPE_WINDOWS
    return _unlink(path.c_str()) == 0;
#else
    return unlink(path.c_str()) == 0;
#endif
}

bool CopyFile(const string& fromFile, const string& toFile)
{
    // Open input file:

    ifstream is(fromFile.c_str() PEGASUS_IOS_BINARY);

    if (!is)
        return false;

    // Open output file:

    ofstream os(toFile.c_str() PEGASUS_IOS_BINARY);

    if (!os)
        return false;

    char c;

    while (is.get(c))
        os.put(c);

    return true;
}

static int _Stripline(
    const string& arg0,
    const string& startString,
    const string& endString,
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

    bool found = false;
    string line;

    while (getline(is, line))
    {
        if (line.substr(0, startString.size()) == startString)
        {
            if (line.substr(
                line.size() - endString.size(), endString.size()) == endString)
            {
                found = true;
                continue;
            }
        }
        os << line << endl;
    }

    is.close();
    os.close();

    if (!found)
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

int main(int argc, char** argv)
{
    if (argc != 4)
    {
        cerr << argv[0] << " removes characters between <start> and ";
        cerr << "<end> from each textline in <input_file>." << endl;
        cerr << "Usage: " << argv[0];
        cerr << "<input_file> <start> <end> <tmp_file> " << endl;
        exit(1);
    }
    return _Stripline(argv[0], argv[1], argv[2], argv[3]);
}
