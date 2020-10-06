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

const char* arg0 = "";

void ProcessFile(const char* fileName)
{
    ifstream is(fileName PEGASUS_IOS_BINARY);

    if (!is)
    {
        cerr << arg0 << "warning: failed to open \"" << fileName << "\"";
        cerr << endl;
        return;
    }

    const char TEMP_FILE_NAME[] = "stripcrs.tmp";

    ofstream os(TEMP_FILE_NAME PEGASUS_IOS_BINARY);

    if (!os)
    {
        cerr << arg0 << ": failed to open \"";
        cerr << TEMP_FILE_NAME << "\"" << endl;
        exit(1);
    }

    char c;

    while (is.get(c))
    {
        if (c != '\r')
            os.put(c);
    }

    is.close();
    os.close();

    if (!CopyFile(TEMP_FILE_NAME, fileName))
    {
        cerr << arg0 << ": failed to copy back file" << endl;
        exit(1);
    }

    RemoveFile(TEMP_FILE_NAME);
}

int main(int argc, char** argv)
{
    if (argc < 2)
    {
        cerr << "Usage: " << argv[0] << " filenames..." << endl;
        exit(1);
    }

    for (int i = 1; i < argc; i++)
    {
        cout << argv[i] << endl;
        ProcessFile(argv[i]);
    }

    return 0;
}
