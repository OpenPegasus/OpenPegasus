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

static Boolean verbose;
#define VCOUT if (verbose) cout

const CIMNamespaceName NAMESPACE = CIMNamespaceName ("root/test");
const CIMName CLASSNAME = CIMName ("PG_ProviderModule");
const CIMName CLASSNAME2 = CIMName ("PG_Provider");
const CIMName CLASSNAME3 = CIMName ("PG_ProviderCapabilities");

void TestDeleteInstances(ProviderRegistrationManager & prmanager)
{
   CIMKeyBinding kb1(CIMName ("ProviderModuleName"), "providersModule1",
       CIMKeyBinding::STRING);
   CIMKeyBinding kb2(CIMName ("ProviderName"), "PG_ProviderInstance1",
       CIMKeyBinding::STRING);
   CIMKeyBinding kb3(CIMName ("CapabilityID"), "capability1",
       CIMKeyBinding::STRING);
   CIMKeyBinding kbp1(CIMName ("ProviderModuleName"), "providersModule1",
       CIMKeyBinding::STRING);
   CIMKeyBinding kbp2(CIMName ("Name"), "PG_ProviderInstance1",
       CIMKeyBinding::STRING);
   CIMKeyBinding kbm1(CIMName ("Name"), "providersModule1",
       CIMKeyBinding::STRING);

   CIMObjectPath instanceName;

  try
  {
    for (Uint32 i=1; i<=3; i++)
    {
      CIMObjectPath instanceName2;
      Array<CIMKeyBinding> keys2;
      Array<CIMKeyBinding> keysm;
      Array<CIMKeyBinding> keys;

      //
      // create Provider module instances
      //
      if (i <= 1)
      {
        CIMObjectPath returnRef;
        CIMClass cimClass(CLASSNAME);
        CIMInstance cimInstance(CLASSNAME);

        cimInstance.addProperty(CIMProperty(CIMName ("Name"),
            String("providersModule1")));
        cimInstance.addProperty(CIMProperty(CIMName ("Vendor"), String("HP")));
        cimInstance.addProperty(CIMProperty(CIMName ("Version"),
            String("2.0")));
        cimInstance.addProperty(CIMProperty(CIMName ("InterfaceType"),
            String("PG_DefaultC++")));
        cimInstance.addProperty(CIMProperty(CIMName ("InterfaceVersion"),
            String("2.0")));
        cimInstance.addProperty(CIMProperty(CIMName ("Location"),
            String("/tmp/module1")));

        instanceName = cimInstance.buildPath(cimClass);

        instanceName.setNameSpace(NAMESPACE);
        instanceName.setClassName(CLASSNAME);

        returnRef = prmanager.createInstance(instanceName, cimInstance);
    }

    // create PG_Provider instances

    if (i <= 2 )
    {
        CIMObjectPath returnRef2;

        CIMClass cimClass2(CLASSNAME2);

        CIMInstance cimInstance2(CLASSNAME2);

        cimInstance2.addProperty(CIMProperty(CIMName ("ProviderModuleName"),
            String("providersModule1")));
        cimInstance2.addProperty(CIMProperty(CIMName ("Name"),
            String("PG_ProviderInstance1")));

        instanceName2 = cimInstance2.buildPath(cimClass2);

        instanceName2.setNameSpace(NAMESPACE);
        instanceName2.setClassName(CLASSNAME2);

        returnRef2 = prmanager.createInstance(instanceName2, cimInstance2);
    }

    //
    // create provider capability instances
    //

    Array <String> namespaces;
    Array <Uint16> providerType;
    Array <String> supportedMethods;

    namespaces.append("test_namespace1");
    namespaces.append("test_namespace2");

    providerType.append(2);
    providerType.append(5);

    supportedMethods.append("test_method1");
    supportedMethods.append("test_method2");

    CIMObjectPath returnRef3;

    CIMClass cimClass3(CLASSNAME3);

    CIMInstance cimInstance3(CLASSNAME3);

    cimInstance3.addProperty(CIMProperty(CIMName ("ProviderModuleName"),
        String("providersModule1")));
    cimInstance3.addProperty(CIMProperty(CIMName ("ProviderName"),
        String("PG_ProviderInstance1")));
    cimInstance3.addProperty(CIMProperty(CIMName ("CapabilityID"),
        String("capability1")));
    cimInstance3.addProperty(CIMProperty(CIMName ("ClassName"),
        String("test_class1")));
    cimInstance3.addProperty(CIMProperty(CIMName ("Namespaces"), namespaces));
    cimInstance3.addProperty(CIMProperty(CIMName ("ProviderType"),
        providerType));
    cimInstance3.addProperty(CIMProperty(CIMName ("SupportedMethods"),
        supportedMethods));

    CIMObjectPath instanceName3 = cimInstance3.buildPath(cimClass3);

    instanceName3.setNameSpace(NAMESPACE);
    instanceName3.setClassName(CLASSNAME3);

    returnRef3 = prmanager.createInstance(instanceName3, cimInstance3);

    Boolean callFailed = false;
    switch (i)
    {
        case 1:
                //
                // delete cimInstance2
                //
                keys2.append(kbp1);
                keys2.append(kbp2);

                instanceName2.setKeyBindings(keys2);

                prmanager.deleteInstance(instanceName2);

                // Test duplicate delete which should fail
                try
                {
                    prmanager.deleteInstance(instanceName2);
                }
                catch(CIMException& e)
                {
                    callFailed = true;
                    VCOUT << "CIMException code " << e.getCode()
                        << "(" << cimStatusCodeToString(e.getCode()) << ")"
                        <<  "\nDescription \"" << e.getMessage() << "\""
                        << endl;
                    PEGASUS_TEST_ASSERT(e.getCode() == CIM_ERR_NOT_FOUND);
                }
                PEGASUS_TEST_ASSERT(callFailed);

            break;

        case 2:
                //
                // delete cimInstance3
                //
                keys.append(kb1);
                keys.append(kb2);
                keys.append(kb3);

                instanceName3.setKeyBindings(keys);

                prmanager.deleteInstance(instanceName3);
            break;

        case 3:
                //
                // delete cimInstance
                //
                keysm.append(kbm1);

                instanceName.setKeyBindings(keysm);

            prmanager.deleteInstance(instanceName);
            break;
    }
    }
  }
  catch(CIMException& e)
  {
      throw (e);
  }
}

int main(int, char** argv)
{
    verbose = (getenv ("PEGASUS_TEST_VERBOSE")) ? true : false;
    if (verbose) cout << argv[0] << ": started" << endl;

    const char* tmpDir = getenv ("PEGASUS_TMP");
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

    CIMRepository r (repositoryRoot) ;

    ProviderRegistrationManager prmanager(&r);

    try
    {
        TestDeleteInstances(prmanager);
    }

    catch(Exception& e)
    {
    PEGASUS_STD(cerr) << "Error: " << e.getMessage() << PEGASUS_STD(endl);
    PEGASUS_STD (cout) << argv[0] << " +++++ delete instances failed"
                       << PEGASUS_STD (endl);
    exit(-1);
    }

    PEGASUS_STD(cout) << argv[0]<< " +++++ passed all tests"
                      << PEGASUS_STD(endl);

    return 0;
}
