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
//%////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
//
// This file implements the functionality required to manage password file.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef Pegasus_UserFileHandler_h
#define Pegasus_UserFileHandler_h

#include <cctype>
#include <fstream>

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/AutoPtr.h>
#include <Pegasus/Common/Mutex.h>

#include <Pegasus/Security/UserManager/PasswordFile.h>
#include <Pegasus/Security/UserManager/Linkage.h>

PEGASUS_NAMESPACE_BEGIN

/**
  This class implements the functionality required to manage password file.
*/

class PEGASUS_USERMANAGER_LINKAGE UserFileHandler
{

private:

    //
    // Contains the property name for password filepath
    //
    static const String _PROPERTY_NAME_PASSWORD_FILEPATH;

    //
    // Contains the salt string for password encryption
    //
    static const unsigned char _SALT_STRING[];

    //
    // Denotes the types of update operations
    //
    enum UpdateOperations
    {
        ADD_USER,
        MODIFY_USER,
        REMOVE_USER
    };

    //
    // Contains the mutex timeout value
    //
    static const Uint32 _MUTEX_TIMEOUT;

    //
    // Password cache
    //
    PasswordTable _passwordTable;

    //
    // Instance of the PasswordFile
    //
    AutoPtr<PasswordFile> _passwordFile;

    //
    // Mutex variable for consistent Password File and cache updates
    //
    AutoPtr<Mutex> _mutex;

    /**
        Generate random salt key for password encryption

        @param salt  A array of 3 characters
    */
    void _GetSalt (char* salt);

    /**
        Update the password hash table and write to password file
    */
    void _Update(
        char operation,
        const String& userName,
        const String& password = String::EMPTY);


protected:

    /**
        Load the user information from the password file.

        @exception PasswordFileSyntaxError if password file contains a syntax
        error.
        @exception CannotRenameFile if password file cannot be renamed.
    */
    void _loadAllUsers();

public:

    /** Constructor. */
    UserFileHandler();

    /** Destructor. */
    ~UserFileHandler();

    /**
        Add user entry to file

        @param  userName  The name of the user to add.
        @param  password  The password for the user.

        @exception FileNotReadable    if unable to read password file
        @exception DuplicateUser      if the user is already exists
        @exception PasswordCacheError if there is an error processing
                                      password hashtable
        @exception CannotRenameFile if password file cannot be renamed.
    */
    void addUserEntry(
        const String& userName,
        const String& password);

    /**
        Modify user entry in file

        @param  userName       The name of the user to modify.
        @param  password       User's old password.
        @param  newPassword    User's new password.

        @exception InvalidUser        if the user does not exist.
        @exception PasswordMismatch   if the specified password does not match
                                      user's current password.
        @exception PasswordCacheError if there is an error processing
                                      password hashtable
        @exception CannotRenameFile   if password file cannot be renamed.

    */
    void modifyUserEntry(
        const String& userName,
        const String& password,
        const String& newPassword);

    /**
        Remove user entry from file

        @param  userName  The name of the user to add.

        @exception FileNotReadable    if unable to read password file
        @exception InvalidUser        if the user is does not exist
        @exception PasswordCacheError if there is an error processing
                                      password hashtable
        @exception CannotRenameFile if password file cannot be renamed.
    */
    void removeUserEntry(const String& userName);

    /**
        Get a list of all the user names.

        @param userNames  List containing all the user names.

        @exception FileNotReadable    if unable to read password file
    */
    void getAllUserNames(Array<String>& userNames);

    /**
        Verify user exists in the cimserver password file

        @param userName  Name of the user to be verified
        @return true if the user exists, else false

        @exception FileNotReadable    if unable to read password file
    */
    Boolean verifyCIMUser(const String& userName);

    /**
        Verify user's password matches specified password

        @param userName  Name of the user to be verified
        @param password  password to be verified
        @return true if the user's password matches existing password, else
        false

        @exception FileNotReadable    if unable to read password file
        @exception InvalidUser        if the specified user does not exist
    */
    Boolean verifyCIMUserPassword(
        const String& userName,
        const String& password);
};

PEGASUS_NAMESPACE_END

#endif /* Pegasus_UserFileHandler_h */

