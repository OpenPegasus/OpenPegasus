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
#include <Pegasus/Common/ArrayInternal.h>
#include <Pegasus/Common/FileSystem.h>

#include <Pegasus/ProviderManagerRouter/ProviderManagerModule.h>

#include <iostream>

PEGASUS_USING_PEGASUS;
PEGASUS_USING_STD;

static String fileName;

void test1()
{
    ProviderManagerModule module(fileName);

    module.load();

    if (!module.isLoaded())
    {
        cout << "failed to load " << module.getFileName() << endl;

        throw 0;
    }

    ProviderManager * p = module.getProviderManager("TEST");

    if (p == 0)
    {
        module.unload();

        cout << "failed to get provider manager from " <<
            module.getFileName() << endl;

        throw 0;
    }

    delete p;

    module.unload();
}

void test2()
{
    ProviderManagerModule module(fileName);

    module.load();

    {
        ProviderManagerModule module2(module);

        if (module2.isLoaded() != module.isLoaded())
        {
            cout << "failed to preserve module state in copy " <<
                module.getFileName() << endl;
        }

        module2.unload();
    }

    module.unload();
}

// array behavior experiment
void test3()
{
    Array<ProviderManagerModule> modules;

    for (Uint32 i = 0, n = 3; i < n; i++)
    {
        cout << "creating ProviderManagerModule object for " << fileName <<
            endl;

        ProviderManagerModule module(fileName);

        modules.append(module);
    }

    for (Uint32 i = 0, n = modules.size(); i < n; i++)
    {
        cout << "loading ProviderManagerModule object for " <<
            modules[i].getFileName() << endl;

        modules[i].load();
    }

    for (Uint32 i = 0, n = modules.size(); i < n; i++)
    {
        cout << "unloading ProviderManagerModule object for " <<
            modules[i].getFileName() << endl;

        modules[i].unload();
    }

    while (modules.size() != 0)
    {
        cout << "removing (destroying) ProviderManagerModule object for " <<
            modules[0].getFileName() << endl;

        modules.remove(0);
    }
}

int main(int argc, char** argv)
{
    const String FILE_NAME =
        FileSystem::buildLibraryFileName("TestProviderManager");

    // Use "bin" directory for Windows, to be consistent with the default
    // providerDir value in Config/ProviderDirPropertyOwner.cpp.
#if defined(PEGASUS_OS_TYPE_WINDOWS)
    fileName = String(getenv("PEGASUS_HOME")) + String("/bin/") + FILE_NAME;
#elif defined(PEGASUS_OS_VMS)
    fileName = String(getenv("PEGASUS_HOME")) + String("/bin/") + FILE_NAME;
#else
    fileName = String(getenv("PEGASUS_HOME")) + String("/lib/") + FILE_NAME;
#endif

    test1();
    test2();
    test3();

    cout << argv[0] << " +++++ passed all tests" << endl;

    return 0;
}
