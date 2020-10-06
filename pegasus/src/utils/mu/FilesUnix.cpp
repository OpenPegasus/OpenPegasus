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
#include <cstdio>
#include "Config.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <utime.h>
#include <dirent.h>
#include <fcntl.h>
#include "Files.h"

bool GetCwd(string& path)
{
    char tmp[4096];

    getcwd(tmp, sizeof(tmp));
    path = tmp;
    return true;
}

bool ChangeDir(const string& path)
{
    return chdir(path.c_str()) == 0;
}

bool RemoveDir(const string& path)
{
    return rmdir(path.c_str()) == 0;
}

bool RemoveFile(const string& path)
{
    return unlink(path.c_str()) == 0;
}

bool MakeDir(const string& path)
{
    return mkdir(path.c_str(), 0777) == 0;
}

bool GetDirEntries(const string& path, vector<string>& filenames)
{
    filenames.erase(filenames.begin(), filenames.end());

    DIR* dir = opendir(path.c_str());

    if (!dir)
        return false;

    for (dirent* entry = readdir(dir); entry; entry = readdir(dir))
    {
        string name = entry->d_name;

        if (name != "." && name != "..")
            filenames.push_back(name);
    }

    closedir(dir);

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

    // File does exist:

    size_t size = sbuf.st_size;

    if (size == 0)
    {
        // Open file:

        int fd = open(path.c_str(), O_RDWR, 0666);

        if (fd < 0)
            return false;

        char c = '\0';

        // Write one byte:

        if (write(fd, &c, sizeof(char)) != 1)
            return false;

        // Truncate back to zero size:

        if (ftruncate(fd, size) < 0)
            return false;

        // Close the file:

        close(fd);

        return true;
    }
    else
    {
        // Open the file:

        int fd = open(path.c_str(), O_RDWR, 0666);

        if (fd < 0)
            return false;

        // Read first character, rewind, then rewrite it:

        char c;

        if (read(fd, &c, sizeof(char)) != 1)
            return false;

        if (lseek(fd, 0, SEEK_SET) < 0)
            return false;

        if (write(fd, &c, sizeof(char)) != 1)
            return false;

        // Truncate file to force mod of times:

        if (ftruncate(fd, size) < 0)
            return false;

        close(fd);
    }

    return true;
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
    return access(path.c_str(), F_OK) == 0;
}

bool Readable(const string& path)
{
    return access(path.c_str(), R_OK) == 0;
}

bool Writable(const string& path)
{
    return access(path.c_str(), W_OK) == 0;
}

bool IsDir(const string& path)
{
    struct stat st;
    return stat(path.c_str(), &st) == 0 && S_ISDIR(st.st_mode);
}
