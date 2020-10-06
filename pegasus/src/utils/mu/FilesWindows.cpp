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

#include "Config.h"
#include <iostream>
#include <fstream>
#include <cstdio>
#include <direct.h>
#include <sys/types.h>
#include <sys/utime.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <io.h>
#include "Files.h"
#include <stddef.h>

bool GetCwd(string& path)
{
    char* tmp = _getcwd(NULL, 0);

    if (!tmp)
        return false;

    path = tmp;
    delete [] tmp;
    return true;
}

bool ChangeDir(const string& path)
{
    return _chdir(path.c_str()) == 0;
}

bool RemoveDir(const string& path)
{
    return _rmdir(path.c_str()) == 0;
}

bool RemoveFile(const string& path)
{
    return _unlink(path.c_str()) == 0;
}

bool MakeDir(const string& path)
{
    return _mkdir(path.c_str()) == 0;
}

bool GetDirEntries(const string& path_, vector<string>& filenames)
{
    string path = path_ + "/*.*";

    filenames.clear();

    struct _finddata_t fileinfo;

#if _MSC_VER < 1300
    long handle;
#else
    intptr_t handle;
#endif
    handle = _findfirst(path.c_str(), &fileinfo);

    if (handle == -1)
        return false;

    do
    {
        string name = fileinfo.name;

        if (name != "." && name != "..")
            filenames.push_back(name);

    } while (_findnext(handle, &fileinfo) != -1);

    _findclose(handle);

    return true;
}


bool TouchFile(const string& path)
{
    if (IsDir(path))
        return false;

    // Get file-size:

    struct stat sbuf;

    // If file does not exist:

    if (stat(path.c_str(), &sbuf) != 0)
    {
        int fd = open(path.c_str(), O_WRONLY | O_CREAT, 0666);

        if (fd < 0)
            return false;

        close(fd);
        return true;
    }

    // Call utime() to set file's time; pass NULL to cause current time
    // to be used.

    return _utime(path.c_str(), NULL) == 0;
}

bool GetFileSize(const string& path, size_t& size)
{
    struct stat st;

    if (stat(path.c_str(), &st) != 0)
        return false;

    size = (size_t)(st.st_size);
    return true;
}

bool Exists(const string& path)
{
    return access(path.c_str(), 0) == 0;
}

bool Readable(const string& path)
{
    return access(path.c_str(), 4) == 0;
}

bool Writable(const string& path)
{
    return access(path.c_str(), 2) == 0;
}

bool IsDir(const string& path)
{
    struct stat st;
    return stat(path.c_str(), &st) == 0 && (_S_IFDIR & st.st_mode);
}
