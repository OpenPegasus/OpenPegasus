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

#include <iostream>
#include <fstream>
#include <string>
#include "ReplaceCmd.h"

int ReplaceCmd(const vector<string>& args)
{
    string findString=args[2];
    string replaceString=args[3];
    string fileName = args[1];
    int sz=findString.length();
    ifstream ifile(fileName.c_str(),ios::binary);
    ifile.seekg(0,ios::end);
    long s=ifile.tellg();
    char *buffer=new char[s];
    ifile.seekg(0);
    ifile.read(buffer,s);
    ifile.close();
    string txt(buffer,s);
    delete[] buffer;
    size_t off=0;
    while ((off=txt.find(findString,off))!=string::npos)
       txt.replace(off,sz,replaceString);
    ofstream ofile(fileName.c_str());
    ofile.write(txt.c_str(),txt.size());
    return 0;
}
