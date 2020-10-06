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
#include <Pegasus/Config/ConfigManager.h>
#include <Pegasus/Server/ProviderRegistrationManager/ProviderManagerMap.h>

PEGASUS_USING_PEGASUS;
PEGASUS_USING_STD;

static Boolean verbose;

int main(int, char** argv)
{
    verbose = (getenv ("PEGASUS_TEST_VERBOSE")) ? true : false;

    const char* _pegHome = getenv ("PEGASUS_HOME");
    if (_pegHome == NULL)
    {
        PEGASUS_STD (cout) << argv[0] << " +++++ tests failed: "
            << "PEGASUS_HOME environment variable must be set"
                           << PEGASUS_STD (endl);
        return -1;
    }
    ConfigManager::setPegasusHome(_pegHome);

    try
    {
        String name;
        String ver;

        name = "C++Default";
        ver = "2.1.0";
        PEGASUS_TEST_ASSERT(
            ProviderManagerMap::instance().isValidProvMgrIfc(name, ver));
        ver = "2.6.0";
        PEGASUS_TEST_ASSERT(
            ProviderManagerMap::instance().isValidProvMgrIfc(name, ver));
        name = "Junk";
        PEGASUS_TEST_ASSERT(
            !ProviderManagerMap::instance().isValidProvMgrIfc(name, ver));
#ifdef PEGASUS_ENABLE_CMPI_PROVIDER_MANAGER
        name = "CMPI";
        ver = "2.0.0";
        PEGASUS_TEST_ASSERT(
            ProviderManagerMap::instance().isValidProvMgrIfc(name, ver));
        ver = "1.2.3";
        PEGASUS_TEST_ASSERT(
            !ProviderManagerMap::instance().isValidProvMgrIfc(name, ver));
#endif
#ifdef PEGASUS_ENABLE_JMPI_PROVIDER_MANAGER
        name = "JMPI";
        ver = "2.2.0";
        PEGASUS_TEST_ASSERT(
            ProviderManagerMap::instance().isValidProvMgrIfc(name, ver));
#endif
    }
    catch(Exception& e)
    {
    PEGASUS_STD(cerr) << "Error: " << e.getMessage() << PEGASUS_STD(endl);
    PEGASUS_STD (cout) << argv[0] << " +++++ tests failed"
                       << PEGASUS_STD (endl);
    exit(-1);
    }


    PEGASUS_STD(cout) << argv[0] << " +++++ passed all tests"
                      << PEGASUS_STD(endl);

    return 0;
}

