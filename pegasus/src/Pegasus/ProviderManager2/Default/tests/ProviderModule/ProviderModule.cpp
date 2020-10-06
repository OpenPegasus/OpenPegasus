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
#include <Pegasus/Common/FileSystem.h>
#include <Pegasus/ProviderManager2/Default/ProviderModule.h>

#include <iostream>

PEGASUS_USING_PEGASUS;
PEGASUS_USING_STD;

// Uses the pegasus/src/Providers/TestProviders/PG_TestPropertyTypes provider
static const String GOOD_PROVIDER_LIBRARY_NAME = "PG_TestPropertyTypes";
static const String GOOD_PROVIDER_NAME = "PG_TestPropertyTypes";
static const String BAD_PROVIDER_NAME = "BogusProvider";

// Made-up library name
static const String NONEXISTENT_PROVIDER_LIBRARY_NAME = "BogusLibrary";

// Uses the DynLib library from Common/tests
static const String MISSING_ENTRY_POINT_PROVIDER_LIBRARY_NAME = "TestDynLib";

static Boolean verbose;

String getLibraryFileName(const String& libraryName) {
#if defined(PEGASUS_OS_VMS)
    String prefixDir;
# if defined(PEGASUS_USE_RELEASE_DIRS)
    prefixDir = String("/wbem_opt/wbem/providers/lib/");
# else
    const char* tmp = getenv("PEGASUS_HOME");
    if (tmp)
    {
        prefixDir = tmp + String("/bin/");
    }
# endif
    return prefixDir +
           FileSystem::buildLibraryFileName(libraryName);
# else
    return FileSystem::buildLibraryFileName(libraryName);
#endif
}

void testGoodProvider()
{
    ProviderModule module(getLibraryFileName(GOOD_PROVIDER_LIBRARY_NAME));

    PEGASUS_TEST_ASSERT(module.getFileName() ==
        getLibraryFileName(GOOD_PROVIDER_LIBRARY_NAME));

    CIMProvider* provider = module.load(GOOD_PROVIDER_NAME);
    PEGASUS_TEST_ASSERT(provider != 0);
    provider->terminate();
    module.unloadModule();

    provider = module.load(GOOD_PROVIDER_NAME);
    PEGASUS_TEST_ASSERT(provider != 0);

    // Load incorrect provider name while another provider is loaded
    {
        Boolean caughtException = false;

        try
        {
            module.load(BAD_PROVIDER_NAME);
        }
        catch (Exception& e)
        {
            caughtException = true;
            if (verbose)
            {
                cout << "Got expected exception: " << e.getMessage() << endl;
            }
        }

        PEGASUS_TEST_ASSERT(caughtException);
    }

    // Load the same provider a second time
    CIMProvider* provider1 = module.load(GOOD_PROVIDER_NAME);
    PEGASUS_TEST_ASSERT(provider1 != 0);

    provider->terminate();
    module.unloadModule();

    provider1->terminate();
    module.unloadModule();

    // Load incorrect provider name while no other provider is loaded
    {
        Boolean caughtException = false;

        try
        {
            module.load(BAD_PROVIDER_NAME);
        }
        catch (Exception& e)
        {
            caughtException = true;
            if (verbose)
            {
                cout << "Got expected exception: " << e.getMessage() << endl;
            }
        }

        PEGASUS_TEST_ASSERT(caughtException);
    }
}

void testNonexistentProvider()
{
    ProviderModule module(
        getLibraryFileName(NONEXISTENT_PROVIDER_LIBRARY_NAME));

    Boolean caughtException = false;

    try
    {
        module.load(BAD_PROVIDER_NAME);
    }
    catch (Exception& e)
    {
        caughtException = true;
        if (verbose)
        {
            cout << "Got expected exception: " << e.getMessage() << endl;
        }
    }

    PEGASUS_TEST_ASSERT(caughtException);
}

void testMissingProviderEntryPoint()
{
    ProviderModule module(
        getLibraryFileName(MISSING_ENTRY_POINT_PROVIDER_LIBRARY_NAME));

    Boolean caughtException = false;

    try
    {
        module.load(BAD_PROVIDER_NAME);
    }
    catch (Exception& e)
    {
        caughtException = true;
        if (verbose)
        {
            cout << "Got expected exception: " << e.getMessage() << endl;
        }
    }

    PEGASUS_TEST_ASSERT(caughtException);
}

int main(int, char** argv)
{
    verbose = getenv ("PEGASUS_TEST_VERBOSE") ? true : false;

    try
    {
        testGoodProvider();
        testNonexistentProvider();
        testMissingProviderEntryPoint();
    }
    catch(Exception& e)
    {
        cout << "Caught unexpected exception: " << e.getMessage() << endl;
        return 1;
    }
    catch(...)
    {
        cout << "Caught unexpected exception" << endl;
        return 1;
    }

    cout << argv[0] << " +++++ passed all tests" << endl;

    return 0;
}
