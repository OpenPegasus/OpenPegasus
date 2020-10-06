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

#ifndef Pegasus_Dir_h
#define Pegasus_Dir_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/String.h>
#include <Pegasus/Common/InternalException.h>
#include <Pegasus/Common/Linkage.h>
#include <Pegasus/Common/AutoPtr.h>

#if defined(PEGASUS_OS_SOLARIS)
# include <sys/param.h>
#endif

#if defined(PEGASUS_OS_TYPE_WINDOWS)
# include <io.h>
#else // if defined(PEGASUS_OS_TYPE_UNIX) || defined (PEGASUS_OS_VMS)
# include <dirent.h>
#endif

PEGASUS_NAMESPACE_BEGIN

#if defined(PEGASUS_OS_TYPE_WINDOWS)

typedef struct
{
# if _MSC_VER < 1300
    long file;
# else
    intptr_t file;
# endif
    struct _finddata_t findData;
} DirRep;

#else // if defined(PEGASUS_OS_TYPE_UNIX) || defined (PEGASUS_OS_VMS)

struct DirRep
{
    DIR* dir;
    struct dirent* entry;

# ifdef PEGASUS_OS_SOLARIS
private:
    char buf[sizeof(dirent) + MAXNAMELEN];
public:
    struct dirent& buffer;
    inline DirRep()
        : buffer(*reinterpret_cast<struct dirent *>(buf))
    { }
# else /* ifdef PEGASUS_OS_SOLARIS */
    struct dirent buffer;
# endif /* ifdef PEGASUS_OS_SOLARIS */
};
#endif

/** The Dir class provides a platform independent way of iterating the
    files in a directory.
*/
class PEGASUS_COMMON_LINKAGE Dir
{
public:

    /** Starts this iterator class on the given path.
        @param String path is the path to the target directory
        @return
        @exception throws CannotOpenDirectory if invalid directory.

        <pre>
        char* path = "."
        try
        {
           for (Dir dir(path); dir.more(); dir.next())
           {
               cout << "name: " << dir.getName() << endl;
           }
        }
        catch (CannotOpenDirectory&)
        {
           // Error!
        }
        </pre>
    */
    Dir(const String& path);

    /** Release any iterator resources. */
    ~Dir();

    /** Return true if there are more file names to iterator. */
    Boolean more() const { return _more; }


    /** Returns the current file name. */
    const char* getName() const;

    /** Advance to next file in directory. */
    void next();

private:

    Boolean _more;
    String _path;
    DirRep _dirRep;
};

PEGASUS_NAMESPACE_END

#endif /* Pegasus_Dir_h */
