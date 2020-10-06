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

#include "Config.h"
#include "Files.h"
#include <iostream>
#include <fstream>
#include <cstdio>
#include <cstring>

static void _SplitPath(const string& path, vector<string>& components)
{
    char* tmp = new char[path.size() + 1];
    strcpy(tmp, path.c_str());

#if defined (OS_VMS)
    components.push_back (tmp);
#else
    if (path[0] == '/')
        components.push_back("/");

    for (char* p = strtok(tmp, "/"); p != NULL; p = strtok(NULL, "/"))
        components.push_back(p);

    // Fixup the drive letter:

    if (components.size() > 1)
    {
        string s = components[0];

        if (s.size() == 2 && isalpha(s[0]) && s[1] == ':')
        {
            components[0] += "/" + components[1];
            components.erase(components.begin() + 1, components.begin() + 2);
        }
    }

    delete [] tmp;
#endif
}

// string.find_last_of() is broken in GNU C++.

static inline size_t _find_last_of(const string& str, char c)
{
    const char* p = strrchr(str.c_str(), c);

    if (p)
        return size_t(p - str.c_str());

    return (size_t)-1;
}

void _SplitPath(
    const string& path,
    string& dirname,
    string& basename)
{
    size_t pos = _find_last_of(path, '/');

#if defined (OS_VMS)
    size_t pos1 = _find_last_of (path, ']');
    if ((pos == (size_t) -1) && (pos1 == (size_t) -1))
#else
    if (pos == (size_t)-1)
#endif
    {
        dirname = ".";
        basename = path;
    }
    else
    {
#if defined (OS_VMS)
        // Did we find a slash?
        if (pos == (size_t) -1)
        {
            // No. Must be a close bracket.
            dirname = path.substr (0, pos1 + 1);
            basename = path.substr (pos1 + 1);
        }
        else
        {
            // Yes.
            dirname = path.substr (0, pos);
            basename = path.substr (pos + 1);
        }
#else
        dirname = path.substr(0, pos);
        basename = path.substr(pos + 1);
#endif
    }
}

bool RemoveFile(const string& path, bool recurse)
{
    if (!IsDir(path))
        return RemoveFile(path);

    if (!recurse)
        return RemoveDir(path);

    vector<string> filenames;

    if (GetDirEntries(path, filenames))
    {
        string save_cwd;
        GetCwd(save_cwd);

        if (!ChangeDir(path))
            return false;

        for (size_t i = 0; i < filenames.size(); i++)
            RemoveFile(filenames[i], true);

        if (!ChangeDir(save_cwd))
            return false;
    }

    return RemoveDir(path);
}

bool MkDirHier(const string& path)
{
    vector<string> components;

    _SplitPath(path, components);

    for (size_t i = 0; i < components.size(); i++)
    {
        if (!IsDir(components[i]))
        {
            if (!MakeDir(components[i].c_str()))
                return false;
        }

        if (!ChangeDir(components[i]))
            return false;
    }

    return true;
}

static int _Match(const char* pattern, const char* str)
{
    const char* p;
    const char* q;

    /* Now match expression to str. */

    for (p = pattern, q = str; *p && *q; )
    {
        if (*p == '*')
        {
            const char* r;

            p++;

            /* Recursively call to find the shortest match. */

            for (r = q; *r; r++)
            {
                if (_Match(p, r) == 0)
                    break;
            }

            q = r;

        }
        else if (*p == *q)
        {
            p++;
            q++;
        }
        else
            return -1;
    }

    /* If src was exhausted but pattern has a single '*' remaining charcters,
     * then match the result.
     */

    if (p[0] == '*' && p[1] == '\0')
        return 0;

    /* If anything left over, then they do not match. */

    if (*p || *q)
        return -1;

    return 0;
}

static bool _contains_special_chars(const string& str)
{
    const char* p = str.c_str();

    return
        strchr(p, '[') || strchr(p, ']') || strchr(p, '*') || strchr(p, '?');
}

bool Glob(const string& pattern_, vector<string>& fileNames)
{
    // Remove trailing slashes:

    string pattern = pattern_;

    while (pattern.size() > 0 && pattern[pattern.size()-1] == '/')
    {
        pattern.erase(pattern.end() - 1);
    }

    // Split the pattern into directory name and base name:

    string dirname;
    string basename;
    _SplitPath(pattern, dirname, basename);

    if (!_contains_special_chars(basename))
        fileNames.push_back(pattern_);
    else
    {
        // Find all files in the given directory matching the pattern:

        bool found = false;
        vector<string> filenames;

        if (!GetDirEntries(dirname, filenames))
            return false;

        for (size_t i = 0; i < filenames.size(); i++)
        {
            if (_Match(
                    (const char*)basename.c_str(),
                    (const char*)filenames[i].c_str()) == 0)
            {
                found = true;

                if (dirname == ".")
                    fileNames.push_back(filenames[i]);
                else
                    fileNames.push_back(dirname + "/" + filenames[i]);
            }
        }

        if (!found)
            return false;
    }

    return true;
}

bool CopyFile(const string& from_file, const string& to_file)
{
    // Open input file:

#ifdef OS_WINDOWS
    ifstream is(from_file.c_str(), ios::binary);
#else
    ifstream is(from_file.c_str());
#endif

    if (!is)
        return false;

    // Open output file:

#ifdef OS_WINDOWS
    ofstream os(to_file.c_str(), ios::binary);
#else
    ofstream os(to_file.c_str());
#endif

    if (!os)
        return false;

    // ATTN: optimize this for speed! Use block-oriented copy approach.
    // Copy the blocks:

    char c;

    while (is.get(c))
        os.put(c);

    return true;
}

bool CopyFiles(const vector<string>& from, const string& to)
{
    // There are two cases. If there is more than one from file, then the
    // to argument must designate a directory. If there is exactly one from
    // file then the to may designate either the destination directory or
    // the new file name.

    if (from.size() > 1)
    {
        if (!IsDir(to))
            return false;

        bool success = true;

        for (size_t i = 0; i < from.size(); i++)
        {
            string dirname;
            string basename;
            _SplitPath(from[i], dirname, basename);

            if (!CopyFile(from[i], to + "/" + basename))
                success = false;
        }

        return success;
    }
    else if (from.size() == 1)
    {
        if (IsDir(to))
        {
            string dirname;
            string basename;
            _SplitPath(from[0], dirname, basename);

            return CopyFile(from[0], to + "/" + basename);
        }
        else
            return CopyFile(from[0], to);
    }
    else
        return false;
}

bool CompareFiles(
    const string& filename1,
    const string& filename2,
    size_t& offset)
{
#ifdef OS_WINDOWS
    ifstream is1(filename1.c_str(), ios::binary);
#else
    ifstream is1(filename1.c_str());
#endif

    if (!is1)
        return false;

#ifdef OS_WINDOWS
    ifstream is2(filename2.c_str(), ios::binary);
#else
    ifstream is2(filename2.c_str());
#endif

    if (!is2)
        return false;

    char c1 = 0;
    char c2 = 0;
    offset = 0;

    while (c1 == c2)
    {
        bool more1 = is1.get(c1) ? true : false;
        bool more2 = is2.get(c2) ? true : false;

        if (!more1 || !more2)
            return more1 == more2;

        offset++;
    }

    return false;
}

/** test if the file path is relative to parent dir.
    For example,
        filePath = "../../mu.cpp"
    @param filePath specifies the source file path from mu command argument.
        or header file path from #include directive.
    @return true if the file path start with "../"; otherwise, false.
*/
bool IsPathRelativeToParentDir(const string& filePath)
{
    const char* start = filePath.c_str();

    return ((start[0] == '.') &&
        (start[1] == '.') &&
        (start[2] == '/'));
}

/** Get the full (absolute) path of a given file.
    For example, given:
        prependDir = "/var/buildMAIN/pegasus/src/utils/mu/tests"
        filePath = "../mu.cpp"
    then:
        fileFullpath = "/var/buildMAIN/pegasus/src/utils/mu/mu.cpp"

    @param prependDir specifies the directory prepended to source file
        or hearder file.
    @param filePath is the source file path from mu command argument
        or header file path from #include directive.
    @param fileFullpath is set to the full path for the file.
*/
void GetFileFullPath(
    const string& prependDir,
    const string& filePath,
    string& fileFullpath)
{
    //check if the file path is already the full path
    if (filePath.c_str()[0] == '/')
    {
        fileFullpath = filePath;
    }
    else {

        // part1 and part2 are the two parts of the fileFullPath
        string part1 = prependDir;
        string part2 = filePath;
        while (IsPathRelativeToParentDir(part2))
        {
            part2.erase(0,3);
            int found;
            if (!(found = _find_last_of(part1,'/')))
            {
                cerr << "GetFileFullPath: "
                    << "error: cannot resolve relative path: " << filePath <<
                    " with prependDir: " << prependDir << endl;
                exit(1);
            }
            part1 = part1.substr(0,found);
        }

        fileFullpath = part1;
        fileFullpath += '/';
        fileFullpath += part2;
    }
}


/** Get the absolute directory of the source file.
    @param filePath specifies the source file path from mu command argument.
    @param fileFullpath is set to the absolute path of the source file
        specified by filePath.
    @param srcDir is set to the absolute directory of the source file.
*/
void GetSrcFileDir(
    const string& filePath,
    string& fileFullpath,
    string& srcDir)
{
    string workingDir;
    // get the working directory
    if (!GetCwd(workingDir))
    {
        cerr << "GetSrcFileDir: "
            << "error: failed to access working directory" << endl;
        exit(1);
    }
    // check if filePath is a relative directory followed by file name
    GetFileFullPath(workingDir, filePath, fileFullpath);

    string filename;
    _SplitPath(fileFullpath, srcDir, filename);
}
