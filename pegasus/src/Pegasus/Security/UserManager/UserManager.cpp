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
// User Manager
//
///////////////////////////////////////////////////////////////////////////////

#include <Pegasus/Common/System.h>
#include <Pegasus/Common/Tracer.h>
#include <Pegasus/Common/PegasusVersion.h>

#include <Pegasus/Security/UserManager/UserManager.h>
#include <Pegasus/Security/UserManager/UserExceptions.h>


PEGASUS_USING_STD;

PEGASUS_NAMESPACE_BEGIN

/**
    Initialize UserManager instance
*/
UserManager* UserManager::_instance = 0;
Mutex UserManager::_userManagerMutex;

//
// Constructor
//
UserManager::UserManager(CIMRepository* repository)
{
    PEG_METHOD_ENTER(TRC_USER_MANAGER, "UserManager::UserManager");

#ifndef PEGASUS_NO_PASSWORDFILE
    _userFileHandler.reset(new UserFileHandler());
#endif
    _authHandler.reset(new AuthorizationHandler(repository));

    PEG_METHOD_EXIT();
}

//
// Destructor
//
UserManager::~UserManager()
{
    PEG_METHOD_ENTER(TRC_USER_MANAGER, "UserManager::~UserManager");

    PEG_METHOD_EXIT();
}

//
// Terminates the usermanager;
//
void UserManager::destroy()
{
    delete _instance;
    _instance = 0;
}

//
// Construct the singleton instance of the UserManager and return a
// pointer to that instance.
//
UserManager* UserManager::getInstance(CIMRepository* repository)
{
    PEG_METHOD_ENTER(TRC_USER_MANAGER, "UserManager::getInstance");

    if (!_instance && !repository)
    {
        PEG_METHOD_EXIT();
        throw CannotCreateUserManagerInstance();
    }

    if (!_instance)
    {
        AutoMutex lock(_userManagerMutex);
        if (!_instance)
        {
            _instance = new UserManager(repository);
        }
    }

    PEG_METHOD_EXIT();

    return _instance;
}

//
// Add a user
//
void UserManager::addUser(
    const String& userName,
    const String& password)
{
    PEG_METHOD_ENTER(TRC_USER_MANAGER, "UserManager::addUser");

#ifndef PEGASUS_NO_PASSWORDFILE
    //
    // Check if the user is a valid system user
    //
    if (!System::isSystemUser(userName.getCString()))
    {
        PEG_METHOD_EXIT();
        throw InvalidSystemUser(userName);
    }

    //
    // Add the user to the password file
    //
    _userFileHandler->addUserEntry(userName, password);
#endif

    PEG_METHOD_EXIT();
}

//
// Modify user's password
//
void UserManager::modifyUser(
    const String& userName,
    const String& password,
    const String& newPassword)
{
    PEG_METHOD_ENTER(TRC_USER_MANAGER, "UserManager::modifyUser");

#ifndef PEGASUS_NO_PASSWORDFILE
    _userFileHandler->modifyUserEntry(userName, password, newPassword);
#endif

    PEG_METHOD_EXIT();
}

//
// Remove a user
//
void UserManager::removeUser(const String& userName)
{
    PEG_METHOD_ENTER(TRC_USER_MANAGER, "UserManager::removeUser");

#ifndef PEGASUS_NO_PASSWORDFILE
    _userFileHandler->removeUserEntry(userName);
#endif

    PEG_METHOD_EXIT();
}


//
// Get a list of all the user names.
//
void UserManager::getAllUserNames(Array<String>& userNames)
{
    PEG_METHOD_ENTER(TRC_USER_MANAGER, "UserManager::getAllUserNames");

#ifndef PEGASUS_NO_PASSWORDFILE
    _userFileHandler->getAllUserNames(userNames);
#endif

    PEG_METHOD_EXIT();
}

//
// Verify whether the specified CIM user is valid
//
Boolean UserManager::verifyCIMUser(const String& userName)
{
    PEG_METHOD_ENTER(TRC_USER_MANAGER, "UserManager::verifyCIMUser");

#ifndef PEGASUS_NO_PASSWORDFILE
    if (_userFileHandler->verifyCIMUser(userName))
    {
        PEG_METHOD_EXIT();
        return true;
    }
#endif

    PEG_METHOD_EXIT();
    return false;
}

//
// Verify whether the specified user's password is valid
//
Boolean UserManager::verifyCIMUserPassword(
    const String& userName,
    const String& password)
{
    PEG_METHOD_ENTER(TRC_USER_MANAGER, "UserManager::verifyCIMUserPassword");

#ifndef PEGASUS_NO_PASSWORDFILE
    if (_userFileHandler->verifyCIMUserPassword(userName, password))
    {
        PEG_METHOD_EXIT();
        return true;
    }
#endif

    PEG_METHOD_EXIT();
    return false;
}

//
// Verify whether the specified namespace is valid
//
Boolean UserManager::verifyNamespace(const CIMNamespaceName& myNamespace)
{
    PEG_METHOD_ENTER(TRC_AUTHORIZATION, "UserManager::verifyNamespace");

    if (_authHandler->verifyNamespace(myNamespace))
    {
        PEG_METHOD_EXIT();
        return true;
    }
    else
    {
        PEG_METHOD_EXIT();
        return false;
    }
}

//
// Verify whether the specified operation has authorization
// to be performed by the specified user.
//
Boolean UserManager::verifyAuthorization(
    const String& userName,
    const CIMNamespaceName& nameSpace,
    const CIMName& cimMethodName)
{
    PEG_METHOD_ENTER(TRC_AUTHORIZATION, "UserManager::verifyAuthorization");

    if (_authHandler->verifyAuthorization(
            userName, nameSpace, cimMethodName))
    {
        PEG_METHOD_EXIT();
        return true;
    }
    else
    {
        PEG_METHOD_EXIT();
        return false;
    }
}

//
// Set the authorizations
//
void UserManager::setAuthorization(
    const String& userName,
    const CIMNamespaceName& myNamespace,
    const String& auth)
{
    PEG_METHOD_ENTER(TRC_AUTHORIZATION, "UserManager::setAuthorization");

    _authHandler->setAuthorization(userName, myNamespace, auth);

    PEG_METHOD_EXIT();
}

//
// Remove the authorizations for the specified user and namespace
//
void UserManager::removeAuthorization(
    const String& userName,
    const CIMNamespaceName& myNamespace)
{
    PEG_METHOD_ENTER(TRC_AUTHORIZATION, "UserManager::removeAuthorization");

    _authHandler->removeAuthorization(userName, myNamespace);

    PEG_METHOD_EXIT();
}


//
// Get the authorizations for the specified user and namespace
//
String UserManager::getAuthorization(
    const String& userName,
    const CIMNamespaceName& myNamespace)
{
    PEG_METHOD_ENTER(TRC_AUTHORIZATION, "UserManager::getAuthorization");

    String auth = _authHandler->getAuthorization(userName, myNamespace);

    PEG_METHOD_EXIT();

    return auth;
}

PEGASUS_NAMESPACE_END
