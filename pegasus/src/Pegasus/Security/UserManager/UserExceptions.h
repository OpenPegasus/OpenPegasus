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


////////////////////////////////////////////////////////////////////////////////
//  This file contains the exception classes used in the UserManager
//  classes.
////////////////////////////////////////////////////////////////////////////////

#ifndef Pegasus_UserExceptions_h
#define Pegasus_UserExceptions_h

#include <Pegasus/Common/Exception.h>
#include <Pegasus/Security/UserManager/Linkage.h>
#include <Pegasus/Common/MessageLoader.h>

PEGASUS_NAMESPACE_BEGIN

/**
    DuplicateUser Exception class
*/
class PEGASUS_USERMANAGER_LINKAGE DuplicateUser : public Exception
{
public:
    DuplicateUser(const String& userName)
        : Exception(MessageLoaderParms(
              "Security.UserManager.UserExceptions.USER_ALREADY_EXISTS",
              "User already exists, user name: $0",
              userName))
    {
    }
};

/**
    InvalidUser Exception class
*/
class PEGASUS_USERMANAGER_LINKAGE InvalidUser : public Exception
{
public:
    InvalidUser(const String& userName)
        : Exception(MessageLoaderParms(
              "Security.UserManager.UserExceptions.INVALID_CIM_USER",
              "The specified user is not a valid CIM user, user name: $0.",
              userName))
    {
    }
};

/**
    InvalidSystemUser Exception class
*/
class PEGASUS_USERMANAGER_LINKAGE InvalidSystemUser : public Exception
{
public:
    InvalidSystemUser(const String& userName)
        : Exception(MessageLoaderParms(
              "Security.UserManager.UserExceptions."
                  "INVALID_USER_ON_LOCAL_SYSTEM",
              "The specified user is not a valid user on the local system, "
                  "user name: $0",
              userName))
    {
    }
};

/**
    PasswordMismatch Exception class
*/
class PEGASUS_USERMANAGER_LINKAGE PasswordMismatch : public Exception
{
public:
    PasswordMismatch(const String& userName)
        : Exception(MessageLoaderParms(
              "Security.UserManager.UserExceptions.PASSWORD_MISMATCH",
              "The specified password does not match user's current password, "
                  "user name: $0",
              userName))
    {
    }
};

/**
    PasswordCacheError Exception class
*/
class PEGASUS_USERMANAGER_LINKAGE PasswordCacheError : public Exception
{
public:
    PasswordCacheError()
        : Exception(MessageLoaderParms(
              "Security.UserManager.UserExceptions.INTERNAL_ERROR_PWD_CACHE",
              "Internal error while processing password cache table."))
    {
    }
};

/**
    InvalidUserAndNamespace Exception class
*/
class PEGASUS_USERMANAGER_LINKAGE InvalidUserAndNamespace : public Exception
{
public:
    InvalidUserAndNamespace(
        const String& userName,
        const String& nameSpace)
        : Exception(MessageLoaderParms(
              "Security.UserManager.UserExceptions."
                  "USER_NAMESPACE_NOT_AUTHORIZED",
              "The specified user '$0' and namespace '$1' are not authorized.",
              userName,
              nameSpace))
    {
    }
};

/**
    AuthorizationCacheError Exception class
*/
class PEGASUS_USERMANAGER_LINKAGE AuthorizationCacheError : public Exception
{
public:
    AuthorizationCacheError()
        : Exception(MessageLoaderParms(
              "Security.UserManager.UserExceptions.INTERNAL_ERROR_AUTH_CACHE",
              "Internal error while processing authorization cache table."))
    {
    }
};

/**
    AuthorizationEntryNotFound Exception class
*/
class PEGASUS_USERMANAGER_LINKAGE AuthorizationEntryNotFound : public Exception
{
public:
    AuthorizationEntryNotFound(
        const String& userName,
        const String& nameSpace)
        : Exception(MessageLoaderParms(
              "Security.UserManager.UserExceptions.AUTH_ENTRY_NOT_FOUND",
              "Authorization entry not found for user '$0' with the namespace "
                  "'$1'.",
              userName,
              nameSpace))
    {
    }
};

/**
    InvalidNamespace Exception class
*/
class PEGASUS_USERMANAGER_LINKAGE InvalidNamespace : public Exception
{
public:
    InvalidNamespace(String nameSpace)
        : Exception(MessageLoaderParms(
              "Security.UserManager.UserExceptions.NAMESPACE_DOES_NOT_EXIST",
              "The specified namespace does not exist, $0",
              nameSpace))
    {
    }
};

/**
    CannotCreateUserManagerInstance Exception class
*/
class PEGASUS_USERMANAGER_LINKAGE CannotCreateUserManagerInstance
    : public Exception
{
public:
    CannotCreateUserManagerInstance()
        : Exception(MessageLoaderParms(
              "Security.UserManager.UserExceptions."
                  "CANT_CREATE_USERMANAGER_INSTANCE",
              "Cannot create UserManager instance, repository handle passed "
                  "may be invalid."))
    {
    }
};

PEGASUS_NAMESPACE_END

#endif /* Pegasus_UserExceptions_h */
