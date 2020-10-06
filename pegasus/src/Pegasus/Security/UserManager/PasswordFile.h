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

#ifndef Pegasus_PasswordFile_h
#define Pegasus_PasswordFile_h

#include <Pegasus/Common/String.h>
#include <Pegasus/Common/HashTable.h>
#include <Pegasus/Common/ArrayInternal.h>

#include <Pegasus/Security/UserManager/Linkage.h>

PEGASUS_NAMESPACE_BEGIN

////////////////////////////////////////////////////////////////////////////////
// PasswordTable
////////////////////////////////////////////////////////////////////////////////
#ifdef PEGASUS_OS_TYPE_WINDOWS
typedef HashTable<String, String, EqualNoCaseFunc, HashLowerCaseFunc>
    PasswordTable;
#else
typedef HashTable<String, String, EqualFunc<String>, HashFunc<String> >
    PasswordTable;
#endif

////////////////////////////////////////////////////////////////////////////////
//  PasswordFile Class
////////////////////////////////////////////////////////////////////////////////

/**
    This class provides methods to read/write from the
    password file.
*/
class PEGASUS_USERMANAGER_LINKAGE PasswordFile
{
private:

    //
    // Contains the full path name of the password file
    //
    String    _passwordFile;

    //
    // Contains the full path name of the password backup file
    //
    String    _passwordBackupFile;

public:

    /**
        Constructor.

        @exception  FileNotReadable  if the specified file is not readable.
    */
    PasswordFile(const String& fileName);


    /** Destructor. */
    ~PasswordFile();


    /**
        Get the name of the password file.

        @return String    name of the password file.
    */
    String getFileName() const {return _passwordFile;}


    /**
        Load the username and password from the password file.

        @param passwordTable hash table containing the username and password.

        Note: Original code was taken from OptionManager::mergeFile()
    */
    void load(PasswordTable& passwordTable);


    /**
        Save the username and password to the password file.

        @param passwordTable hash table containing the user name and password.
        @exception CannotRenameFile if failed to create the backup file.
    */
    void save(const PasswordTable& passwordTable);
};

PEGASUS_NAMESPACE_END

#endif /* Pegasus_PasswordFile_h */
