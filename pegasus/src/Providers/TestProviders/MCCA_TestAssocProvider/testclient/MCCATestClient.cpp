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
// Author: Marek Szermutzky (MSzermutzky@de.ibm.com) PEP#139 Stage2
//         Robert Kieninger, (KIENINGR@de.ibm.com)
//
//%/////////////////////////////////////////////////////////////////////////////


#include <iostream>
#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/Signal.h>
#include <Pegasus/Common/Constants.h>
#include <Pegasus/Common/System.h>
#include <Pegasus/Common/FileSystem.h>
#include <Pegasus/Common/String.h>
#include <Pegasus/Common/PegasusVersion.h>

#include <Pegasus/getoopt/getoopt.h>
#include <Clients/cliutils/CommandException.h>
#include "MCCATestClient.h"


PEGASUS_NAMESPACE_BEGIN


MCCATestClient::MCCATestClient()
{
    cca_dccm = new CIMDefaultClientConnectionManager();
}

MCCATestClient::~MCCATestClient()
{
    delete cca_dccm;
}

void MCCATestClient::initialiseConnectionManager(const String& host,
                                                 const String& port)
{
    cca_dccm->addConnection(host,port, String::EMPTY, String::EMPTY);
}

Uint32 MCCATestClient::enumerateInstancesNames(const String& _host,
                                               const String& _port)
{
    const CIMNamespaceName nameSpace = CIMNamespaceName ("root/cimv2");
    const CIMName className = CIMName ("CIM_ComputerSystem");
    try
    {
        Array<CIMObjectPath> cimInstanceNames = client.enumerateInstanceNames(
                                                        _host,
                                                        _port,
                                                        nameSpace,
                                                        CIMName(className));
        Uint32 numberInstances = cimInstanceNames.size();
        return numberInstances;
    }
    catch (Exception&)
    {
        exit(1);
    }
    return 0;
}

Uint32 MCCATestClient::enumerateInstances(const String& _host,
                                          const String& _port)
{
    const CIMNamespaceName nameSpace = CIMNamespaceName ("root/cimv2");
    const CIMName className = CIMName ("CIM_ComputerSystem");
    try
    {
        Array<CIMInstance> cimInstance = client.enumerateInstances(
            _host, _port, nameSpace, CIMName(className));
        Uint32 numberInstances = cimInstance.size();
        return numberInstances;
    }
    catch (Exception&)
    {
        exit(2);
    }
    return 0;
}

void MCCATestClient::createInstance(
    const String& _host,
    const String& _port,
    const CIMNamespaceName& nameSpace,
    Uint32 i)
{
    const CIMName className = CIMName ("MCCA_TestClass");
    try
    {
        //
        // Test create Provider module instances
        //
        CIMObjectPath returnRef;
        // CIMClass cimClass= CIMClass(className);
        char  buffer[100];

        // create a new CIMInstance based on class with name className
        CIMInstance cimInstance= CIMInstance(className);

        // add properties to the CIMInstance object
        cimInstance.addProperty( CIMProperty( CIMName("theKey"), i));
        cimInstance.addProperty( CIMProperty( CIMName("theData"), 20+i));
        sprintf(buffer,"ABC-%u",20*i+i);
        cimInstance.addProperty(CIMProperty(CIMName ("theString"),
                                            String(buffer)));

        // get the object path, so we can complete it
        CIMObjectPath instanceName = CIMObjectPath(cimInstance.getPath());

        // combine host and port for usage in object path
        String fullHost = String(_host);
        if (_port != String::EMPTY)
        {
                fullHost.append(":");
                fullHost.append(_port);
        }
        CLDEBUG("createInstanceTest with fullHost="
                << fullHost
                << " and NameSpace="
                << nameSpace.getString());
        instanceName.setHost(fullHost);
        instanceName.setNameSpace(nameSpace);
        // set the now complete object path
        cimInstance.setPath(instanceName);

        // the real call ...
        returnRef = client.createInstance(cimInstance);

        CLDEBUG("Successfully created Instance.");
    }
    catch (Exception& e)
    {
        CLDEBUG("Create Instance " << i << " failed.");
        CLDEBUG("Exception=" << e.getMessage());
        exit(3);
    }
}

CIMInstance MCCATestClient::associatorsTest(const String& _host,
                                            const String& _port,
                                            const CIMNamespaceName& fromNS,
                                            const CIMNamespaceName& toNS,
                                            Uint32 key)
{
    CIMName testClassName = CIMName ("MCCA_TestClass");
    CIMName assocClassName = CIMName ("MCCA_TestAssocClass");
    CIMObjectPath   targetObjectPath = CIMObjectPath();

    // combine host and port for usage in object path
    String fullHost = String(_host);
    if (_port != String::EMPTY)
    {
            fullHost.append(":");
            fullHost.append(_port);
    }
    // CLDEBUG("Host in object path of associatorsTest()=" << fullHost);
    // preparing object path first
    targetObjectPath.setHost(fullHost);
    targetObjectPath.setNameSpace(fromNS);
    targetObjectPath.setClassName(testClassName);

    CIMKeyBinding  testClassKey = CIMKeyBinding(CIMName("theKey"),
                                                CIMValue((Uint32) key));
    Array<CIMKeyBinding> targetKeyBindings = Array<CIMKeyBinding>();
    targetKeyBindings.append(testClassKey);
    targetObjectPath.setKeyBindings(targetKeyBindings);

    // CLDEBUG("Preparation of object path for associators() call ready.");
    CLDEBUG("Association call for host=" << fullHost
            << ",Namespace=" << fromNS.getString()
            << ",key=" << key);

    // Time to do the call
    try
    {
        cout << "targetObjectPath " << targetObjectPath.toString()
             << " assocClassName " << assocClassName.getString()
             << " testClassName " << testClassName.getString() << endl;

        Array<CIMObject> cimInstance =  client.associators(
                                                targetObjectPath,
                                                assocClassName,
                                                testClassName);
        // only one instance should be returned and that one should
        // have an equal key
        if (cimInstance.size() == 0)
        {
                CLDEBUG("Associators call returned zero objects");
                exit(7);
        }

        if (cimInstance.size() > 1)
        {
                CLDEBUG("Associators call returned multiple objects");
                exit(8);
        }
        CIMObjectPath instanceRef = cimInstance[0].getPath();

        if (!testClassName.equal(instanceRef.getClassName().getString()))
        {
                CLDEBUG("resultClasswith wrong name returned.");
                exit(4);
        }

        if (!toNS.equal(instanceRef.getNameSpace()) )
        {
                CLDEBUG("target object has wrong Namespace");
                exit(5);
        }

        Array<CIMKeyBinding> resultKeyBindings = instanceRef.getKeyBindings();
        // only one key, so only one keybinding
        String  keyValueString = String(resultKeyBindings[0].getValue());
        Uint32  sourceKey =
            strtoul((const char*) keyValueString.getCString(), NULL, 0);
        if (sourceKey != key)
        {
                CLDEBUG("Key of returned instance wrong.");
                exit(6);
        }
        CLDEBUG("Association call successful.");
        return (CIMInstance) cimInstance[0];
    }
    catch (Exception& e)
    {
        CLDEBUG("Association call failed with exception.");
        CLDEBUG("Exception=" << e.getMessage());
        exit(7);
    }
    // gets never called, but compilers will ask for it anyway
    return CIMInstance();
}

CIMInstance MCCATestClient::getInstance(const String& _host,
                                        const String& _port,
                                        const CIMNamespaceName& nameSpace,
                                        Uint32 key)
{
    CIMName testClassName = CIMName ("MCCA_TestClass");
    CIMObjectPath   targetObjectPath = CIMObjectPath();
    // combine host and port for usage in object path
    String fullHost = String(_host);
    if (_port != String::EMPTY)
    {
        fullHost.append(":");
        fullHost.append(_port);
    }

    // preparing object path first
    targetObjectPath.setHost(fullHost);
    targetObjectPath.setNameSpace(nameSpace);
    targetObjectPath.setClassName(testClassName);

    CIMKeyBinding  testClassKey = CIMKeyBinding(CIMName("theKey"),
                                                CIMValue((Uint32) key));
    Array<CIMKeyBinding> keyBindings = Array<CIMKeyBinding>();
    keyBindings.append(testClassKey);
    targetObjectPath.setKeyBindings(keyBindings);
    // CLDEBUG("Preparation of object path for getInstance() call ready.");
    CLDEBUG("getInstance call for host="
            << fullHost << ",Namespace=" << nameSpace.getString()
            << ",key=" << key);
    // Time to do the call
    try
    {
        CIMInstance cimInstance = client.getInstance(targetObjectPath);
        CLDEBUG("getInstance call successful.");
        return cimInstance;
    }
    catch (Exception&)
    {
        CLDEBUG("Exception on getInstance.");
        exit(8);
    }
    // gets never called, but compilers will ask for it anyway
    return CIMInstance();
}

void MCCATestClient::deleteInstance(const CIMInstance& toDelete)
{
    CLDEBUG("deleteInstance called with");
    try
    {
        CLDEBUG("Host=" << toDelete.getPath().getHost());
        CLDEBUG("NS=" << toDelete.getPath().getNameSpace().getString());
        CLDEBUG("Key=" << toDelete.getPath().getKeyBindings()[0].getValue() );
        client.deleteInstance(toDelete.getPath());
        CLDEBUG("deleteInstance call successful.");
    }
    catch (Exception& e)
    {
        CLDEBUG("Exception" << e.getMessage());
        CLDEBUG("Failed to delete Instance.");
        exit(10);
    }
}

PEGASUS_NAMESPACE_END

// exclude main from the Pegasus Namespace
PEGASUS_USING_PEGASUS;
PEGASUS_USING_STD;

int main (int argc, char* argv [])
{
    String usedHost;
    String usedPort;
    Uint32 instanceKEY;
    CIMNamespaceName testNameSpaceA =
        CIMNamespaceName("root/MCCAtest/A");
    CIMNamespaceName testNameSpaceB =
        CIMNamespaceName("root/MCCAtest/B");


    // parsing for host/port and key of instances information
    if (argc != 2)
    {
       cout << "Usage: MCCATestClient <num>" << endl;
       return 99;
    }
    else
    {
       usedHost=String("localhost");
       usedPort=String("5988");
       instanceKEY = atoi(argv[1]);
    }

    // Preparing MCCA Test Client Object
    MCCATestClient mcca_test = MCCATestClient();
    // adding a connection to our CIMDefaulClientConnectionManager
    mcca_test.initialiseConnectionManager(usedHost,usedPort);
    // hand it over to CIMManagedClient
    mcca_test.client = CIMManagedClient(mcca_test.cca_dccm);


    // Create Instance A' in NameSpace A
    mcca_test.createInstance(usedHost, usedPort, testNameSpaceA,
                             instanceKEY);
    // Create Instance B' in NameSpace B
    mcca_test.createInstance(usedHost, usedPort, testNameSpaceB,
                             instanceKEY);

    // test Association call across namespaces ...
    CIMInstance assocInstance = mcca_test.associatorsTest(usedHost,
        usedPort, testNameSpaceA, testNameSpaceB, instanceKEY);
    CIMInstance gotInstance = mcca_test.getInstance(usedHost,
        usedPort, testNameSpaceB, instanceKEY);

    CLDEBUG("assocInstance="  << assocInstance.getPath().getHost()
            << "," << assocInstance.getPath().getNameSpace().getString());

    CLDEBUG("gotInstance="  << gotInstance.getPath().getHost()
            << "," << gotInstance.getPath().getNameSpace().getString());

    String host1 = String(assocInstance.getPath().getHost());
    String host2 = String(gotInstance.getPath().getHost());

    if (!String::equal(host1,host2))
    {
            CLDEBUG("host("
                    << host1 << ")of associators call not equal host("
                    << host2 << ") of getInstance call.");
            exit(11);
    }


    // deleting the two created instances A' and B'
    mcca_test.deleteInstance(mcca_test.getInstance(usedHost, usedPort,
                                                   testNameSpaceA,
                                                   instanceKEY));
    mcca_test.deleteInstance(mcca_test.getInstance(usedHost, usedPort,
                                                   testNameSpaceB,
                                                   instanceKEY));

    PEGASUS_STD(cout) << "+++++ "<< argv[0]
        << " Terminated Normally" << PEGASUS_STD(endl);
    exit(0);
}
