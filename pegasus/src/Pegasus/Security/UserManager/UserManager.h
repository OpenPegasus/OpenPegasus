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
// This file implements the functionality required to manage users.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef Pegasus_UserManager_h
#define Pegasus_UserManager_h

#include <Pegasus/Common/Config.h>
#include <cctype>
#include <Pegasus/Common/String.h>
#include <Pegasus/Security/UserManager/UserFileHandler.h>
#include <Pegasus/Security/UserManager/AuthorizationHandler.h>
#include <Pegasus/Security/UserManager/Linkage.h>

PEGASUS_NAMESPACE_BEGIN

/**
    This class interfaces with UserFileHandler for creating, removing and
    listing users.
*/
class PEGASUS_USERMANAGER_LINKAGE UserManager
{
private:

    //
    // Singleton instance of UserManager, the constructor
    // and the destructor are made private
    //
    static UserManager* _instance;

    // UserManager Mutex member
    static Mutex _userManagerMutex;

    //
    // Instance of UserFileHandler
    //
    AutoPtr<UserFileHandler> _userFileHandler;

    //
    // Instance of AuthorizationHandler
    //
    AutoPtr<AuthorizationHandler> _authHandler;

    /** Constructor. */
    UserManager(CIMRepository* repository);

    // Default Constructor - Implementation Not Required.
    UserManager();

    // Copy COnstructor - Implementation Not Required.
    UserManager(const UserManager&);

    // Overloaded Assignment Operator - Implementation Not Required.
    UserManager& operator=(UserManager&);

    /** Destructor. */
    ~UserManager();

public:

    /**
        Construct the singleton instance of the UserManager and return a
        pointer to that instance.
    */
    static UserManager* getInstance(CIMRepository* repository = 0);


    /**
        Terminates the UserManager singleton.
    */
    static void destroy();

    /**
        Add a user.

        @param  userName  The name of the user to add.
        @param  password  The password for the user.

        @exception InvalidSystemUser  if the user is not a system user
        @exception FileNotReadable    if unable to read password file
        @exception DuplicateUser      if the user already exists
        @exception PasswordCacheError if there is an error processing
                                      password hashtable
        @exception CannotRenameFile   if password file cannot be renamed.
    */
    void addUser(
        const String& userName,
        const String& password);

    /**
        Modify user's password.

        @param  userName       The name of the user to modify.
        @param  password       User's old password.
        @param  newPassword    User's new password.

        @exception InvalidUser        if the user does not exist
        @exception PasswordMismatch   if the specified password does not match
                                      user's current password.
        @exception PasswordCacheError if there is an error processing
                                      password hashtable
        @exception CannotRenameFile   if password file cannot be renamed.
    */
    void modifyUser(
        const String& userName,
        const String& password,
        const String& newPassword);

    /**
        Remove a user.

        @param  userName  The name of the user to remove.

        @exception FileNotReadable    if unable to read password file
        @exception InvalidUser        if the user does not exist
        @exception PasswordCacheError if there is an error processing
                                      password hashtable
        @exception CannotRenameFile   if password file cannot be renamed.
    */
    void removeUser(const String& userName);


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

    /**
        Verify whether the spcefied namespace is a valid namespace.
        @param myNamespace   string containing the namespace name.
        @return true if the specified name space is valid and exists, false
        otherwise.
    */
    Boolean verifyNamespace( const CIMNamespaceName& myNamespace );

    /**
        Verify whether the specified operation has authorization to be performed
        by the specified user.
        @param userName    string containing the user name.
        @param nameSpace   string containing the namespace name.
        @param cimMethodName  string containing the cim method name.
        @return true if the specified user has authorizations to run the
        specified CIM operation on the specified namespace, false otherwise.
    */
    Boolean verifyAuthorization(
        const String& userName,
        const CIMNamespaceName& nameSpace,
        const CIMName& cimMethodName);

    /**
        Set the authorization to the specified user on the specified namespace.
        @param userName   string containing the user name.
        @param myNamespace  string containing the namespace name.
        @param auth string containing the authorizations.
    */
    void setAuthorization(
        const String& userName,
        const CIMNamespaceName& myNamespace,
        const String& auth);

    /**
        Remove the authorizations of the specified user on the specified
        namespace.
        @param userName   string containing the user name.
        @param myNamespace  string containing the namespace name.
    */
    void removeAuthorization(
        const String& userName,
        const CIMNamespaceName& myNamespace);

    /**
        Get the authorizations of the specified user on the specified namespace.
        @param userName   string containing the user name.
        @param myNamespace  string containing the namespace name.
        @return a string containing the authorizations.
    */
    String getAuthorization(
        const String& userName,
        const CIMNamespaceName& myNamespace);
};

PEGASUS_NAMESPACE_END

#endif /* Pegasus_UserManager_h */
