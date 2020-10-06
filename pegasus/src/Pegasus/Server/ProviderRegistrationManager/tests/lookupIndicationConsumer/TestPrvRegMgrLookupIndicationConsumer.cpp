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
#include <Pegasus/Repository/CIMRepository.h>
#include <Pegasus/Client/CIMClient.h>
#include \
    <Pegasus/Server/ProviderRegistrationManager/ProviderRegistrationManager.h>

PEGASUS_USING_PEGASUS;
PEGASUS_USING_STD;

const String PEGASUS_NAMESPACE = "root/test";

Boolean testLookupIndicationConsumer(
    ProviderRegistrationManager & provRegManager)
{
    //
    // create Provider module instances
    //
    CIMObjectPath returnRefProvMod;

    CIMClass cimClassProvMod(PEGASUS_CLASSNAME_PROVIDERMODULE);

    CIMInstance cimInstanceProvMod(PEGASUS_CLASSNAME_PROVIDERMODULE);

    cimInstanceProvMod.addProperty(
        CIMProperty("Name", String("providersModule1")));
    cimInstanceProvMod.addProperty(CIMProperty("Vendor", String("IBM")));
    cimInstanceProvMod.addProperty(CIMProperty("Version", String("2.0")));
    cimInstanceProvMod.addProperty(
        CIMProperty("InterfaceType", String("PG_DefaultC++")));
    cimInstanceProvMod.addProperty(
        CIMProperty("InterfaceVersion", String("2.0")));
    cimInstanceProvMod.addProperty(
        CIMProperty("Location", String("/tmp/module1")));

    CIMObjectPath instanceNameProvMod = cimInstanceProvMod.buildPath(
        cimClassProvMod);

    instanceNameProvMod.setNameSpace(PEGASUS_NAMESPACE);
    instanceNameProvMod.setClassName(PEGASUS_CLASSNAME_PROVIDERMODULE);

    returnRefProvMod = provRegManager.createInstance(
        instanceNameProvMod,
        cimInstanceProvMod);

    // create PG_Provider instances

    CIMObjectPath returnRefProv;

    CIMClass cimClassProv(PEGASUS_CLASSNAME_PROVIDER);

    CIMInstance cimInstanceProv(PEGASUS_CLASSNAME_PROVIDER);

    cimInstanceProv.addProperty(
        CIMProperty("ProviderModuleName", String("providersModule1")));
    cimInstanceProv.addProperty(
        CIMProperty("Name", String("PG_ProviderInstance1")));

    CIMObjectPath instanceNameProv = cimInstanceProv.buildPath(cimClassProv);

    instanceNameProv.setNameSpace(PEGASUS_NAMESPACE);
    instanceNameProv.setClassName(PEGASUS_CLASSNAME_PROVIDER);

    returnRefProv = provRegManager.createInstance(
        instanceNameProv,
        cimInstanceProv);

    //
    // Create consumer capability instance
    //
    CIMObjectPath returnRefConCap;
    CIMClass cimClassConCap(PEGASUS_CLASSNAME_CONSUMERCAPABILITIES);
    CIMInstance cimInstanceConCap(PEGASUS_CLASSNAME_CONSUMERCAPABILITIES);
    Array<Uint16> providerType;
    Array<String> providerDest;

    providerType.append(6);
    providerDest.append("/TestIndicationConsumer");

    cimInstanceConCap.addProperty(
        CIMProperty("ProviderModuleName", String("providersModule1")));
    cimInstanceConCap.addProperty(
        CIMProperty("ProviderName", String("PG_ProviderInstance1")));
    cimInstanceConCap.addProperty(
        CIMProperty("CapabilityID", String("capability1")));
    cimInstanceConCap.addProperty(CIMProperty("ProviderType", providerType));
    cimInstanceConCap.addProperty(
        CIMProperty("Destinations", providerDest));

    CIMObjectPath instanceNameConCap = cimInstanceConCap.buildPath(
        cimClassConCap);

    instanceNameConCap.setNameSpace(PEGASUS_NAMESPACE);
    instanceNameConCap.setClassName(PEGASUS_CLASSNAME_CONSUMERCAPABILITIES);

    returnRefConCap = provRegManager.createInstance(
        instanceNameConCap,
        cimInstanceConCap);

    // Test lookupIndicationConsumer Interface

    const String destinationPath = "/TestIndicationConsumer";

    CIMInstance providerIns;
    CIMInstance providerModuleIns;

    if (provRegManager.lookupIndicationConsumer(
        destinationPath,
        providerIns,
        providerModuleIns))
    {
        // Test for ProviderModule Name
        String _providerModuleName;
        String _providerModuleName2;

        providerIns.getProperty(
            providerIns.
            findProperty(CIMName("ProviderModuleName"))).
            getValue().
            get(_providerModuleName);

        providerModuleIns.getProperty(
            providerModuleIns.
            findProperty(CIMName ("Name"))).
            getValue().
            get(_providerModuleName2);

        PEGASUS_TEST_ASSERT(
            String::equal(_providerModuleName, _providerModuleName2) == 1);

        // Test for Provider Name
        String _providerName;

        providerIns.getProperty(
            providerIns.
            findProperty(CIMName("Name"))).
            getValue().
            get(_providerName);

        PEGASUS_TEST_ASSERT(
            String::equal(_providerName, "PG_ProviderInstance1") == 1);

        return true;
    }
    else
    {
        return false;
    }
}

int main(int, char** argv)
{
    const char* tmpDir = getenv("PEGASUS_TMP");
    String repositoryRoot;

    if (tmpDir == NULL)
    {
        repositoryRoot = ".";
    }
    else
    {
        repositoryRoot = tmpDir;
    }

    repositoryRoot.append("/repository");
    CIMRepository r(repositoryRoot) ;
    ProviderRegistrationManager provRegManager(&r);

    try
    {
        if (!testLookupIndicationConsumer(provRegManager))
        {
            PEGASUS_STD(cerr)
                << "Error: lookupIndicationConsumer Failed"
                << PEGASUS_STD(endl);
            exit(-1);
        }
    }
    catch (Exception& e)
    {
        PEGASUS_STD(cerr) << "Error: " << e.getMessage() << PEGASUS_STD(endl);
        PEGASUS_STD(cout)
            << argv[0]
            << " +++++ lookup indication consumer failed"
            << PEGASUS_STD(endl);
        exit(-1);
    }

    PEGASUS_STD(cout)
        << argv[0]
        << " +++++ passed all tests"
        << PEGASUS_STD(endl);

    return 0;
}
