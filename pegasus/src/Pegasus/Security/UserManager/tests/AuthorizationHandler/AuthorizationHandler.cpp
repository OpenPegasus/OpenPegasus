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
#include <Pegasus/Common/HashTable.h>
#include <Pegasus/Repository/CIMRepository.h>
#include <Pegasus/Security/UserManager/UserManager.h>
#include <Pegasus/Security/UserManager/AuthorizationHandler.h>

// Uncomment this if you want detailed messages to be printed.
//#define DEBUG 1

PEGASUS_USING_PEGASUS;

PEGASUS_USING_STD;

static Boolean verbose;

static const CIMNamespaceName GOOD_NAMESPACE = CIMNamespaceName ("root/cimv2");

static const CIMNamespaceName BAD_NAMESPACE = CIMNamespaceName ("root/cimvx99");

static const CIMNamespaceName GOOD_CIMV2_NAMESPACE = 
    CIMNamespaceName ("root/CIMV2");


//
// main
//
int main(int, char** argv)
{
    verbose = (getenv ("PEGASUS_TEST_VERBOSE")) ? true : false;
    if (verbose) cout << argv[0] << ": started" << endl;

    CIMNamespaceName nameSpace;
    String testUser = System::getEffectiveUserName();

    // Create a test repository
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

    UserManager*  userManager = UserManager::getInstance(repository);

    //
    // Test authorization
    //
    try
    {
        nameSpace = BAD_NAMESPACE;
        PEGASUS_TEST_ASSERT(!userManager->verifyNamespace(nameSpace));

        nameSpace = GOOD_NAMESPACE;
        PEGASUS_TEST_ASSERT(!userManager->verifyNamespace(nameSpace));

        userManager->setAuthorization(testUser, nameSpace, "rw");
        userManager->setAuthorization("root", nameSpace, "w");

        String temp = userManager->getAuthorization(testUser, nameSpace);

        if (testUser != "root")
            PEGASUS_TEST_ASSERT(
                String::equal(temp, "rw") || String::equal(temp, "wr"));

        temp = userManager->getAuthorization("root", nameSpace);
        PEGASUS_TEST_ASSERT(String::equal(temp, "w"));

        nameSpace = GOOD_CIMV2_NAMESPACE; // upper case namespace name
        temp = userManager->getAuthorization("root", nameSpace);
        PEGASUS_TEST_ASSERT(String::equal(temp, "w"));

        userManager->removeAuthorization("root", nameSpace);
        temp.clear();
        try
        {
            temp = userManager->getAuthorization("root", nameSpace);
            PEGASUS_TEST_ASSERT(temp.size() == 0);
        }catch(const Exception&) { }

        userManager->setAuthorization("root", nameSpace, "w");

        if (testUser != "root")
            PEGASUS_TEST_ASSERT(userManager->verifyAuthorization(
                testUser, nameSpace, CIMName("GetInstance")));
        PEGASUS_TEST_ASSERT(!userManager->verifyAuthorization(
            "root", nameSpace, CIMName("GetInstance")));

        userManager->setAuthorization("root", nameSpace, "r");

        PEGASUS_TEST_ASSERT(!userManager->verifyAuthorization(
            "root", nameSpace, CIMName("SetProperty")));

        userManager->removeAuthorization("root", nameSpace);
        if (testUser != "root")
           userManager->removeAuthorization(testUser, nameSpace);
    }
    catch(Exception& e)
    {
        cout << argv[0] << " Exception: " << e.getMessage() << endl;
        PEGASUS_TEST_ASSERT(0);
    }

    UserManager::destroy();
    delete repository;
    FileSystem::removeDirectoryHier(repositoryPath);

    cout << argv[0] << " +++++ passed all tests" << endl;

      return 0;
}
