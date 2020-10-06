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

#include "TestServerProfileProvider.h"
#include <Pegasus/Common/Tracer.h>

PEGASUS_USING_PEGASUS;

const String storageSystemProvider("StorageSystemTestProvider");
const CIMName storageSystemClass("Test_StorageSystem");

const String mcsStorageSystemProvider("MCSStorageSystemTestProvider");
const CIMName mcsStorageSystemClass("Test_MCSStorageSystem");

const CIMName elementConformsClass("Test_ElementConformsToProfile");
const String arrayProfileName("Array");

const String otherSystemProvider("OtherSystemTestProvider");
const CIMName otherSystemClass("Test_OtherSystem");

const String otherDynamicProfileProvider(
    "TestDynamicProfileCapabilityProvider");
const String otherDynamicProfileClass("Test_DynamicProfile");

const String otherDynamicSubProfileProvider1(
    "TestDynamicSubProfile1CapabilityProvider");
const String otherDynamicSubProfileClass1("Test_DynamicSubProfile1");

const String otherDynamicSubProfileProvider2(
    "TestDynamicSubProfile2CapabilityProvider");
const String otherDynamicSubProfileClass2("Test_DynamicSubProfile2");

TestServerProfileProvider::TestServerProfileProvider(
    const String& providerName)
{
    if (String::equalNoCase(providerName, storageSystemProvider))
    {
        testClassName = storageSystemClass;
        names.append(String("StorageSystemInstance1"));
        names.append(String("StorageSystemInstance2"));
    }
    else if (String::equalNoCase(providerName, mcsStorageSystemProvider))
    {
        testClassName = mcsStorageSystemClass;
        names.append(String("MCSStorageSystemInstance1"));
        names.append(String("MCSStorageSystemInstance2"));
        names.append(String("MCSStorageSystemInstance1-1"));
        names.append(String("MCSStorageSystemInstance1-2"));
        names.append(String("MCSStorageSystemInstance2-1"));
        names.append(String("MCSStorageSystemInstance2-2"));
    }
    else if (String::equalNoCase(providerName, otherSystemProvider))
    {
        testClassName = otherSystemClass;
        names.append(String("OtherSystemInstance1"));
        names.append(String("OtherSystemInstance2"));
    }
    else if (String::equalNoCase(providerName, otherDynamicProfileProvider))
    {
        testClassName = otherDynamicProfileClass;
        names.append(String("OtherDynamicProfileInstance1"));
        names.append(String("OtherDynamicProfileInstance2"));
    }
    else if (String::equalNoCase(
        providerName,
        otherDynamicSubProfileProvider1))
    {
        testClassName = otherDynamicSubProfileClass1;
        names.append(String("OtherDynamicSubProfile1Instance1"));
        names.append(String("OtherDynamicSubProfile1Instance2"));
    }
    else if (String::equalNoCase(
        providerName,
        otherDynamicSubProfileProvider2))
    {
        testClassName = otherDynamicSubProfileClass2;
        names.append(String("OtherDynamicSubProfile2Instance1"));
        names.append(String("OtherDynamicSubProfile2Instance2"));
    }
    else
    {
        throw CIMInvalidParameterException(providerName);
    }
}

TestServerProfileProvider::~TestServerProfileProvider()
{
}

void TestServerProfileProvider::initialize(CIMOMHandle& cimom)
{
    cimomHandle = cimom;
    try
    {
        testClass = cimomHandle.getClass(
            OperationContext(), CIMNamespaceName("test/TestProvider"),
            testClassName, false, true, true, CIMPropertyList());
    }
    catch (const CIMException& e)
    {
        PEG_TRACE((TRC_CONTROLPROVIDER, Tracer::LEVEL4,
                   "Initialize Error testClassName %s CIMException %u %s",
                   (const char *)testClassName.getString().getCString(),
                   e.getCode(), cimStatusCodeToString(e.getCode()) ));
        throw (e);
    }
    catch (const Exception& e)
    {
        PEG_TRACE((TRC_CONTROLPROVIDER, Tracer::LEVEL4,
                   "Initialize Error testClassName %s Exception %s",
                   (const char *)testClassName.getString().getCString(),
                   (const char *)e.getMessage().getCString() ));
        throw (e);
    }
    catch ( ... )
    {
        PEG_TRACE((TRC_CONTROLPROVIDER, Tracer::LEVEL4,
                   "Initialize Error testClassName %s Exception ...",
                   (const char *)testClassName.getString().getCString() ));
        throw ;
    }
}

void TestServerProfileProvider::terminate()
{
    delete this;
}

void TestServerProfileProvider::getInstance(
    const OperationContext& context,
    const CIMObjectPath& instanceReference,
    const Boolean includeQualifiers,
    const Boolean includeClassOrigin,
    const CIMPropertyList& propertyList,
    InstanceResponseHandler& handler)
{
    handler.processing();
    Array<CIMInstance> instances = localEnumerateInstances(
        instanceReference.getClassName(),
        includeQualifiers, includeClassOrigin, propertyList);
    CIMObjectPath instancePath(String::EMPTY,
        CIMNamespaceName("test/TestProvider"),
        instanceReference.getClassName(), instanceReference.getKeyBindings());
    bool found = false;
    for (unsigned int i = 0, n = instances.size(); i < n; ++i)
    {
        CIMInstance& currentInst = instances[i];
        if (currentInst.getPath() == instancePath)
        {
            handler.deliver(currentInst);
            found = true;
            break;
        }
    }

    if (!found)
    {
        throw CIMObjectNotFoundException(instanceReference.toString());
    }

    handler.complete();
}

void TestServerProfileProvider::enumerateInstances(
    const OperationContext& context,
    const CIMObjectPath& classReference,
    const Boolean includeQualifiers,
    const Boolean includeClassOrigin,
    const CIMPropertyList& propertyList,
    InstanceResponseHandler& handler)
{
    // begin processing the request
    handler.processing();
    Array<CIMInstance> instances = localEnumerateInstances(
        classReference.getClassName(),
        includeQualifiers, includeClassOrigin, propertyList);
    for (Uint32 i = 0, n = instances.size(); i < n; ++i)
    {
        // deliver instance
        handler.deliver(instances[i]);
    }

    // complete processing the request
    handler.complete();
}

void TestServerProfileProvider::enumerateInstanceNames(
    const OperationContext& context,
    const CIMObjectPath& classReference,
    ObjectPathResponseHandler& handler)
{
    // begin processing the request
    handler.processing();
    Array<CIMInstance> instances = localEnumerateInstances(
        classReference.getClassName(), false, false,
        CIMPropertyList(Array<CIMName>()));
    for (Uint32 i = 0, n = instances.size(); i < n; ++i)
    {
        // deliver instance name
        handler.deliver(instances[i].getPath());
    }

    // complete processing the request
    handler.complete();
}


void TestServerProfileProvider::modifyInstance(
    const OperationContext& context,
    const CIMObjectPath& instanceReference,
    const CIMInstance& instanceObject,
    const Boolean includeQualifiers,
    const CIMPropertyList& propertyList,
    ResponseHandler& handler)
{
    // deliver exception to the ProviderManager, which in turn will return the
    // error message to the requestor

    throw CIMNotSupportedException(
          "TestServerProfileProvider::modifyInstance()");
}

void TestServerProfileProvider::createInstance(
    const OperationContext& context,
    const CIMObjectPath& instanceReference,
    const CIMInstance& instanceObject,
    ObjectPathResponseHandler& handler)
{
    // deliver exception to the ProviderManager, which in turn will return the
    // error message to the requestor

    throw CIMNotSupportedException(
        "TestServerProfileProvider::createInstance()");
}


void TestServerProfileProvider::deleteInstance(
    const OperationContext& context,
    const CIMObjectPath& instanceReference,
    ResponseHandler& handler)
{
    throw CIMNotSupportedException(
        "TestServerProfileProvider::deleteInstance()");
}

void TestServerProfileProvider::associators(
    const OperationContext& context,
    const CIMObjectPath& objectName,
    const CIMName& associationClass,
    const CIMName& resultClass,
    const String& role,
    const String& resultRole,
    const Boolean includeQualifiers,
    const Boolean includeClassOrigin,
    const CIMPropertyList& propertyList,
    ObjectResponseHandler& handler)
{
    throw CIMNotSupportedException(
        "TestServerProfileProvider::associators()");
}

void TestServerProfileProvider::associatorNames(
    const OperationContext& context,
    const CIMObjectPath& objectName,
    const CIMName& associationClass,
    const CIMName& resultClass,
    const String& role,
    const String& resultRole,
    ObjectPathResponseHandler& handler)
{
    throw CIMNotSupportedException(
        "TestServerProfileProvider::associatorNames()");
}

void TestServerProfileProvider::references(
    const OperationContext& context,
    const CIMObjectPath& objectName,
    const CIMName& resultClass,
    const String& role,
    const Boolean includeQualifiers,
    const Boolean includeClassOrigin,
    const CIMPropertyList& propertyList,
    ObjectResponseHandler& handler)
{
    throw CIMNotSupportedException(
        "TestServerProfileProvider::references()");
}

void TestServerProfileProvider::referenceNames(
    const OperationContext& context,
    const CIMObjectPath& objectName,
    const CIMName& resultClass,
    const String& role,
    ObjectPathResponseHandler& handler)
{
    throw CIMNotSupportedException(
        "TestServerProfileProvider::referenceNames()");
}

Array<CIMInstance> TestServerProfileProvider::localEnumerateInstances(
    const CIMName& enumClass,
    Boolean includeQualifiers,
    Boolean includeClassOrigin,
    const CIMPropertyList& propList)
{
    Array<CIMInstance> instances;

    if (enumClass == testClassName)
    {
        CIMInstance templateInstance = testClass.buildInstance(
            includeQualifiers, includeClassOrigin, propList);

        Uint32 propIndex = templateInstance.findProperty("CreationClassName");
        if (propIndex != PEG_NOT_FOUND)
        {
            templateInstance.getProperty(propIndex).setValue(
                CIMValue(testClassName.getString()));
        }

        propIndex = templateInstance.findProperty("ElementName");
        if (propIndex != PEG_NOT_FOUND)
        {
            templateInstance.getProperty(propIndex).setValue(
                CIMValue(String("Instance of ") + testClassName.getString()));
        }

        propIndex = templateInstance.findProperty("OtherIdentifyingInfo");
        if (propIndex != PEG_NOT_FOUND)
        {
            templateInstance.getProperty(propIndex).setValue(Array<String>());
        }

        propIndex = templateInstance.findProperty("IdentifyingDescriptions");
        if (propIndex != PEG_NOT_FOUND)
        {
            templateInstance.getProperty(propIndex).setValue(Array<String>());
        }

        propIndex = templateInstance.findProperty("OperationalStatus");
        if (propIndex != PEG_NOT_FOUND)
        {
            Array<Uint16> opStatus;
            opStatus.append(2); // Ok
            templateInstance.getProperty(propIndex).setValue(opStatus);
        }

        propIndex = templateInstance.findProperty("NameFormat");
        if (propIndex != PEG_NOT_FOUND)
        {
            templateInstance.getProperty(propIndex).setValue(
                CIMValue(String("Pegasus Test Format")));
        }


        for (unsigned int i = 0, n = names.size(); i < n; ++i)
        {
            CIMInstance currentInstance = templateInstance.clone();
            propIndex = currentInstance.findProperty(CIMName("Name"));
            if (propIndex != PEG_NOT_FOUND)
            {
                currentInstance.getProperty(propIndex).setValue(
                    CIMValue(names[i]));
            }

            Array<CIMKeyBinding> keys;
            keys.append(CIMKeyBinding(CIMName("CreationClassName"),
                CIMValue(testClassName.getString())));
            keys.append(CIMKeyBinding(CIMName("Name"),
                CIMValue(names[i])));

            currentInstance.setPath(CIMObjectPath(
                String::EMPTY, CIMNamespaceName("test/TestProvider"),
                testClassName, keys));

            instances.append(currentInstance);
        }
    }
    else if (testClassName == mcsStorageSystemClass &&
        enumClass == elementConformsClass)
    {
        Array<CIMInstance> registeredProfiles = cimomHandle.enumerateInstances(
            OperationContext(),
            CIMNamespaceName(PEGASUS_NAMESPACENAME_INTEROP.getString()),
            CIMName("PG_RegisteredProfile"), false, false, false, false,
            CIMPropertyList());
        Array<CIMObjectPath> profilePaths;
        unsigned int i = 0;
        unsigned int n = registeredProfiles.size();
        for (; i < n; ++i)
        {
            CIMInstance currentInstance = registeredProfiles[i];
            Uint32 propIndex = currentInstance.findProperty(
                CIMName("RegisteredName"));
            if (propIndex != PEG_NOT_FOUND)
            {
                String profileName;
                currentInstance.getProperty(propIndex).getValue().get(
                    profileName);
                if (profileName == arrayProfileName)
                {
                    profilePaths.append(currentInstance.getPath());
                }
            }
        }

        // Now get the CS instances for which the ElementConformsToProfile
        // association applies.
        Array<CIMInstance> storageSystems = localEnumerateInstances(
            testClassName, false, false, CIMPropertyList());
        i = 0;
        n = storageSystems.size();
        for (; i < n; ++i)
        {
            CIMInstance currentSystem = storageSystems[i];
            Uint32 propIndex = currentSystem.findProperty(CIMName("Name"));
            if (propIndex != PEG_NOT_FOUND)
            {
                String arrayName;
                currentSystem.getProperty(propIndex).getValue().get(arrayName);
                if (arrayName.find('-') == PEG_NOT_FOUND)
                {
                    for (unsigned int j = 0, m = profilePaths.size();
                        j < m; ++j)
                    {
                        // Create an instance of Element Conforms To Profile
                        CIMInstance elementConformsInstance(
                            elementConformsClass);
                        elementConformsInstance.addProperty(CIMProperty(
                            CIMName("ConformantStandard"),
                            CIMValue(profilePaths[j])));
                        elementConformsInstance.addProperty(CIMProperty(
                            CIMName("ManagedElement"),
                            CIMValue(currentSystem.getPath())));

                        // Create the object path
                        Array<CIMKeyBinding> pathKeys;
                        pathKeys.append(CIMKeyBinding(
                            CIMName("ConformantStandard"),
                            CIMValue(profilePaths[j])));
                        pathKeys.append(CIMKeyBinding(
                            CIMName("ManagedElement"),
                            CIMValue(currentSystem.getPath())));
                        elementConformsInstance.setPath(CIMObjectPath(
                            String::EMPTY,
                            CIMNamespaceName("test/TestProviders"),
                            elementConformsClass,
                            pathKeys));

                        instances.append(elementConformsInstance);
                    }
                }
            }
        }
    }
    else
    {
        throw CIMNotSupportedException(enumClass.getString());
    }

    return instances;
}
