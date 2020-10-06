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

#include <stdlib.h>
#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/FileSystem.h>
#include <Pegasus/Common/PegasusAssert.h>
#include <Pegasus/Common/MessageQueue.h>
#include <Pegasus/Common/MessageQueueService.h>

#include <Pegasus/General/DynamicLibrary.h>

#include <Pegasus/Config/ConfigManager.h>

#include <Pegasus/ProviderManager2/ProviderManager.h>

PEGASUS_USING_PEGASUS;
PEGASUS_USING_STD;

int trace = 0;

#ifdef PEGASUS_DEBUG
#define DDD(x) if (trace) x;
#else
#define DDD(x)
#endif

static const String JMPI_LIBRARY_NAME = "JMPIProviderManager";

String getLibraryFileName (const String& libraryName)
{
#if defined(PEGASUS_OS_VMS)
    String prefixDir;
# if defined(PEGASUS_USE_RELEASE_DIRS)
    prefixDir = String ("/wbem_opt/wbem/providers/lib/");
# else
    const char* tmp = getenv ("PEGASUS_HOME");

    if (tmp)
    {
        prefixDir = tmp + String ("/bin/");
    }
# endif

    return prefixDir +
           FileSystem::buildLibraryFileName(libraryName) + ".exe";

#elif defined(PEGASUS_OS_DARWIN)

    String libName = FileSystem::buildLibraryFileName (libraryName);
    const char* pegasusHome = getenv ("PEGASUS_HOME");

    if (pegasusHome)
    {
        // Build full library path:
        return String (pegasusHome) + String ("/lib/") + libName;
    }

    // Use LD_LIBRARY_PATH to locate library (if not set, test will fail).
    return libName;

#else
    return FileSystem::buildLibraryFileName (libraryName);
#endif
}

void
printEnvironmentVariables ()
{
    const char *apszEnvVariables[] = {
       "PEGASUS_HOME",
       "PEGASUS_ROOT",
       "PEGASUS_PLATFORM",
       "PEGASUS_DEBUG",
       "PEGASUS_TEST_VERBOSE",
       "PATH",
       "LD_LIBRARY_PATH",
       "CLASSPATH",
       "JAVA_SDK",
       "JAVA_SDKINC"
    };

    for (size_t i = 0;
         i < sizeof (apszEnvVariables)/sizeof (apszEnvVariables[0]);
         i++)
    {
        const char *pszValue = 0;

        pszValue = getenv (apszEnvVariables[i]);

        if (pszValue)
        {
            printf ("%s = \"%s\"\n", apszEnvVariables[i], pszValue);
        }
        else
        {
            printf ("%s = NULL\n", apszEnvVariables[i]);
        }
    }
}

int
main (int argc, char *argv[])
{
    int rc = 1;

    if (getenv ("PEGASUS_TEST_VERBOSE"))
        trace = 1;
    else
        trace = 0;

    printEnvironmentVariables ();

    DynamicLibrary library (getLibraryFileName (JMPI_LIBRARY_NAME));

    DDD(cerr << "library.getFileName () = " << library.getFileName () <<endl);

    PEGASUS_TEST_ASSERT (library.getFileName () == getLibraryFileName (
    JMPI_LIBRARY_NAME));

    if (library.getFileName () != getLibraryFileName (JMPI_LIBRARY_NAME))
    {
        cout << "Error: Cannot create library class for JMPI!" << endl;

        return rc;
    }

    library.load ();

    DDD(cerr << "library.isLoaded () = " << library.isLoaded () << endl);

    PEGASUS_TEST_ASSERT (library.isLoaded ());

    if (!library.isLoaded ())
    {
        cout << "Error: Cannot load JMPI library!" << endl;

        return rc;
    }

    typedef ProviderManager* (* PCPM)(const String & providerManagerName);

    PCPM pcpm = 0;

    pcpm = (PCPM)library.getSymbol ("PegasusCreateProviderManager");

    DDD(cerr << "pcpm = " << hex << (void *)pcpm << dec << endl);

    PEGASUS_TEST_ASSERT(pcpm);

    if (!pcpm)
    {
        cout << "Error: Cannot query PegasusCreateProviderManager function in "
        "JMPI!" << endl;

        return rc;
    }

    String           interface = "JMPI";
    ProviderManager *pm        = 0;

    pm = pcpm (interface);

    DDD(cerr << "pm = " << hex << (void *)pm << dec << endl);

    PEGASUS_TEST_ASSERT(pm);

    if (!pm)
    {
        cout << "Error: PegasusCreateProviderManager failed to return a "
        "provider manager!" << endl;

        return rc;
    }

    delete pm;

    library.unload ();

    DDD(cerr << "library.isLoaded () = " << library.isLoaded () << endl);

    PEGASUS_TEST_ASSERT(!library.isLoaded ());

    if (library.isLoaded ())
    {
        cout << "Error: Cannot unload JMPI library!" << endl;

        return rc;
    }

    rc = 0;

    cout << argv[0] << ": Success" << endl;

    return rc;
}
