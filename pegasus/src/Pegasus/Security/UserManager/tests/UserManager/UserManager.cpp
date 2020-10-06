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

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/PegasusAssert.h>
#include <iostream>
#include <Pegasus/Common/String.h>
#include <Pegasus/Common/System.h>
#include <Pegasus/Common/FileSystem.h>
#include <Pegasus/Security/UserManager/UserManager.h>
#include <Pegasus/Security/UserManager/UserExceptions.h>

PEGASUS_USING_PEGASUS;

PEGASUS_USING_STD;

static const String BAD_USER        = "nosuchuser";

//
// main
//

int main(int, char** argv)
{
#ifdef PEGASUS_OS_HPUX

#ifndef PEGASUS_NO_PASSWORDFILE
    // -- Create a test repository:

    const char* tmpDir = getenv ("PEGASUS_TMP");
    String repositoryPath;
    if (tmpDir == NULL)
    {
        repositoryPath = ".";
    }
    else
    {
        repositoryPath = tmpDir;
    }
    repositoryPath.append("/repository");

    FileSystem::removeDirectoryHier(repositoryPath);

    CIMRepository* repository = new CIMRepository(repositoryPath);

    // -- Create a UserManager object:

    UserManager* userManager = UserManager::getInstance(repository);
    Boolean exceptionFlag = false;
    Array<String> userNames;

    try
    {
        //
        // Try to add an invalid system user
        //
        userManager->addUser(BAD_USER, BAD_USER);
    }
    catch (InvalidSystemUser& isu)
    {
        exceptionFlag = true;
    }
    PEGASUS_TEST_ASSERT( exceptionFlag == true );

    exceptionFlag = false;
    try
    {
        //
        // Try to modify an invalid user
        //
        userManager->modifyUser(BAD_USER, BAD_USER, BAD_USER);
    }
    catch (Exception& e)
    {
        exceptionFlag = true;
    }
    PEGASUS_TEST_ASSERT( exceptionFlag == true );

    exceptionFlag = false;
    try
    {
        //
        // Try to remove an invalid user
        //
        userManager->removeUser(BAD_USER);
    }
    catch (Exception& e)
    {
        exceptionFlag = true;
    }
    PEGASUS_TEST_ASSERT( exceptionFlag == true );

    exceptionFlag = false;
    try
    {
        //
        // Try to list users
        //
        userManager->getAllUserNames(userNames);
    }
    catch (Exception& e)
    {
        exceptionFlag = true;
    }
    PEGASUS_TEST_ASSERT( exceptionFlag == false );

    //
    // Positive tests
    //

    String goodUser = System::getEffectiveUserName();

    exceptionFlag = false;
    try
    {
        //
        // Try to add a valid system user
        //
        userManager->addUser(goodUser, goodUser);
    }
    catch (DuplicateUser& du)
    {
    }
    catch (Exception& e)
    {
        exceptionFlag = true;
    }
    PEGASUS_TEST_ASSERT( exceptionFlag == false );

    exceptionFlag = false;
    try
    {
        //
        // Try to modify a valid user's password
        //
        userManager->modifyUser(goodUser, goodUser, BAD_USER);
    }
    catch (PasswordMismatch& pm)
    {
    }
    catch (Exception& e)
    {
        exceptionFlag = true;
    }
    PEGASUS_TEST_ASSERT( exceptionFlag == false );

    exceptionFlag = false;
    try
    {
        //
        // Try to remove a valid user
        //
        userManager->removeUser(goodUser);
    }
    catch (Exception& e)
    {
        exceptionFlag = true;
    }
    PEGASUS_TEST_ASSERT( exceptionFlag == false );

    exceptionFlag = false;
    try
    {
        //
        // Try to list users
        //
        userManager->getAllUserNames(userNames);
    }
    catch (Exception& e)
    {
        exceptionFlag = true;
    }
    PEGASUS_TEST_ASSERT( exceptionFlag == false );

    UserManager::destroy();
    delete repository;
    FileSystem::removeDirectoryHier(repositoryPath);

#endif
    cout << argv[0] << " +++++ passed all tests" << endl;
    return 0;
#else
    cout << argv[0] << " +++++ passed all tests" << endl;
    return 0;
#endif
}
