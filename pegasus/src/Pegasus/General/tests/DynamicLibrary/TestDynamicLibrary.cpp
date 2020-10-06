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

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/FileSystem.h>
#include <Pegasus/Common/PegasusAssert.h>

#include <Pegasus/General/DynamicLibrary.h>

#include <Pegasus/Config/ConfigManager.h>

#include <iostream>

PEGASUS_USING_PEGASUS;
PEGASUS_USING_STD;

static const String VALID_LIBRARY_NAME = "TestDynLib";
static const String INVALID_LIBRARY_NAME = "BADDynLib";

static Boolean verbose;

String getLibraryFileName(const String& libraryName)
{
#if defined(PEGASUS_OS_VMS)
    String prefixDir;
# if defined(PEGASUS_USE_RELEASE_DIRS)
    prefixDir = String("/wbem_lib/");
# else
    const char* tmp = getenv("PEGASUS_HOME");
    if (tmp)
    {
        prefixDir = tmp + String("/bin/");
    }
# endif
    return prefixDir +
           FileSystem::buildLibraryFileName(libraryName);

#elif defined(PEGASUS_OS_DARWIN)

    String libName = FileSystem::buildLibraryFileName(libraryName);
    const char* pegasusHome = getenv("PEGASUS_HOME");

    if (pegasusHome)
    {
    // Build full library path:
    return String(pegasusHome) + String("/lib/") + libName;
    }

    // Use LD_LIBRARY_PATH to locate library (if not set, test will fail).
    return libName;

#else
    return FileSystem::buildLibraryFileName(libraryName);
#endif
}

// load a valid module, export a symbol, call it, and unload module
void Test1()
{
    DynamicLibrary library(getLibraryFileName(VALID_LIBRARY_NAME));
    PEGASUS_TEST_ASSERT(library.getFileName() ==
        getLibraryFileName(VALID_LIBRARY_NAME));

    library.load();
    PEGASUS_TEST_ASSERT(library.isLoaded());

    Uint32 (* callme)(void) = (Uint32 (*)(void))library.getSymbol("callme");
    PEGASUS_TEST_ASSERT(callme);
    PEGASUS_TEST_ASSERT(callme() == 0xdeadbeef);

    Uint32 (* badfunc)(void) = (Uint32 (*)(void))library.getSymbol("badfunc");
    PEGASUS_TEST_ASSERT(badfunc == 0);

    library.unload();
    PEGASUS_TEST_ASSERT(!library.isLoaded());
}

// load valid module, assignment
void Test2()
{
    DynamicLibrary library(getLibraryFileName(VALID_LIBRARY_NAME));
    PEGASUS_TEST_ASSERT(!library.isLoaded());

    {
        DynamicLibrary library2(library);
        PEGASUS_TEST_ASSERT(!library2.isLoaded());
    }

    library.load();
    PEGASUS_TEST_ASSERT(library.isLoaded());

    {
        DynamicLibrary library2(library);
        PEGASUS_TEST_ASSERT(library2.isLoaded());
    }

    {
        DynamicLibrary library2;

        library2 = library;
        PEGASUS_TEST_ASSERT(library2.isLoaded());
    }

    {
        DynamicLibrary library2(getLibraryFileName(VALID_LIBRARY_NAME));
        library2.load();
        PEGASUS_TEST_ASSERT(library2.isLoaded());
        library2 = library;
        PEGASUS_TEST_ASSERT(library2.isLoaded());
    }

    library = library;
    PEGASUS_TEST_ASSERT(library.isLoaded());

    library.unload();
    PEGASUS_TEST_ASSERT(!library.isLoaded());
}

// load an invalid module
void Test3()
{
    DynamicLibrary library(getLibraryFileName(INVALID_LIBRARY_NAME));
    PEGASUS_TEST_ASSERT(!library.isLoaded());

    Boolean loaded = library.load();
    PEGASUS_TEST_ASSERT(!loaded);
    PEGASUS_TEST_ASSERT(!library.isLoaded());

    String errorMessage = library.getLoadErrorMessage();
    String fileName = library.getFileName();
    if (verbose)
    {
        cout << "Invalid library name: " << fileName << endl;
        cout << "Invalid library load error: " << errorMessage << endl;
    }

    PEGASUS_TEST_ASSERT(fileName == getLibraryFileName(INVALID_LIBRARY_NAME));
}

// Test reference counting
void Test4()
{
    DynamicLibrary library(getLibraryFileName(VALID_LIBRARY_NAME));
    PEGASUS_TEST_ASSERT(!library.isLoaded());

    // Load the library
    Boolean loaded = library.load();
    PEGASUS_TEST_ASSERT(loaded);
    PEGASUS_TEST_ASSERT(library.isLoaded());

    // Load the library again
    loaded = library.load();
    PEGASUS_TEST_ASSERT(loaded);
    PEGASUS_TEST_ASSERT(library.isLoaded());

    // Unload the library
    library.unload();
    PEGASUS_TEST_ASSERT(library.isLoaded());

    // Unload the library again
    library.unload();
    PEGASUS_TEST_ASSERT(!library.isLoaded());

    // Load the library and then assign to a new instance
    library.load();
    PEGASUS_TEST_ASSERT(library.isLoaded());
    library.load();
    PEGASUS_TEST_ASSERT(library.isLoaded());
    library = DynamicLibrary(getLibraryFileName(VALID_LIBRARY_NAME));
    PEGASUS_TEST_ASSERT(!library.isLoaded());

    library.load();
    PEGASUS_TEST_ASSERT(library.isLoaded());
    library.load();
    PEGASUS_TEST_ASSERT(library.isLoaded());
    // Call the destructor while the library is loaded
}

int main(int, char** argv)
{
    verbose = getenv("PEGASUS_TEST_VERBOSE") ? true : false;

    Test1();
    Test2();
    Test3();
    Test4();

    cout << argv[0] << " +++++ passed all tests" << endl;

    return(0);
}
