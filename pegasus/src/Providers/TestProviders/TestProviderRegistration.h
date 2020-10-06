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

#ifndef Pegasus_TestProviderRegistration_h
#define Pegasus_TestProviderRegistration_h

#include <Pegasus/Client/CIMClient.h>
#include <Pegasus/Common/Constants.h>

PEGASUS_USING_PEGASUS;

//
// Following class is defined to provide a way creating provider registration
// instances.
//
class TestProviderRegistration
{
public:
    static CIMObjectPath createModuleInstance(
        CIMClient& client,
        const String& name,
        const String& location);

    static CIMObjectPath createProviderInstance(
        CIMClient& client,
        const String& name,
        const String& providerModuleName);

    static CIMObjectPath createCapabilityInstance(
        CIMClient& client,
        const String& providerModuleName,
        const String& providerName,
        const String& capabilityID,
        const String& className,
        const Array<String>& namespaces,
        const Array<Uint16>& providerTypes,
        const CIMPropertyList& supportedProperties);

    static void deleteCapabilityInstance(
        CIMClient& client,
        const String& providerModuleName,
        const String& providerName,
        const String& capabilityID);

    static void deleteProviderInstance(
        CIMClient& client,
        const String& providerName,
        const String& providerModuleName);

    static void deleteModuleInstance(
        CIMClient& client,
        const String& providerModuleName);
};

inline CIMObjectPath TestProviderRegistration::createModuleInstance(
    CIMClient& client,
    const String& name,
    const String& location)
{
    CIMInstance moduleInstance(PEGASUS_CLASSNAME_PROVIDERMODULE);
    moduleInstance.addProperty(CIMProperty(CIMName("Name"), name));
    moduleInstance.addProperty(CIMProperty(
        CIMName("Vendor"),
        String("Hewlett-Packard Company")));
    moduleInstance.addProperty(CIMProperty(
        CIMName("Version"),
        String("2.0.0")));
    moduleInstance.addProperty(CIMProperty(
        CIMName("InterfaceType"),
        String("C++Default")));
    moduleInstance.addProperty(CIMProperty(
        CIMName("InterfaceVersion"),
        String ("2.6.0")));
    moduleInstance.addProperty(CIMProperty(CIMName("Location"), location));

    CIMObjectPath path = client.createInstance(
        PEGASUS_NAMESPACENAME_INTEROP,
        moduleInstance);

    return path;
}

inline CIMObjectPath TestProviderRegistration::createProviderInstance(
    CIMClient& client,
    const String& name,
    const String& providerModuleName)
{
    CIMInstance providerInstance(PEGASUS_CLASSNAME_PROVIDER);
    providerInstance.addProperty(CIMProperty(CIMName("Name"), name));
    providerInstance.addProperty(CIMProperty(
        CIMName("ProviderModuleName"),
        providerModuleName));

    CIMObjectPath path = client.createInstance(
        PEGASUS_NAMESPACENAME_INTEROP,
        providerInstance);

    return path;
}

inline CIMObjectPath TestProviderRegistration::createCapabilityInstance(
    CIMClient& client,
    const String& providerModuleName,
    const String& providerName,
    const String& capabilityID,
    const String& className,
    const Array<String>& namespaces,
    const Array<Uint16>& providerTypes,
    const CIMPropertyList& supportedProperties)
{
    CIMInstance capabilityInstance(PEGASUS_CLASSNAME_PROVIDERCAPABILITIES);
    capabilityInstance.addProperty(CIMProperty(
        CIMName("ProviderModuleName"),
        providerModuleName));
    capabilityInstance.addProperty(CIMProperty(
        CIMName("ProviderName"),
        providerName));
    capabilityInstance.addProperty(CIMProperty(
        CIMName("CapabilityID"),
        capabilityID));
    capabilityInstance.addProperty(CIMProperty(
        CIMName("ClassName"),
        className));
    capabilityInstance.addProperty(CIMProperty(
        CIMName("Namespaces"),
        namespaces));
    capabilityInstance.addProperty(CIMProperty(
        CIMName("ProviderType"),
        providerTypes));
    if (!supportedProperties.isNull())
    {
        Array<String> propertyNameStrings;
        for (Uint32 i = 0; i < supportedProperties.size(); i++)
        {
            propertyNameStrings.append(supportedProperties[i].getString());
        }
        capabilityInstance.addProperty(CIMProperty(
            CIMName("supportedProperties"),
            propertyNameStrings));
    }

    CIMObjectPath path = client.createInstance(
        PEGASUS_NAMESPACENAME_INTEROP,
        capabilityInstance);

    return path;
}

inline void TestProviderRegistration::deleteCapabilityInstance(
    CIMClient& client,
    const String& providerModuleName,
    const String& providerName,
    const String& capabilityID)
{
    Array<CIMKeyBinding> keyBindings;
    keyBindings.append(CIMKeyBinding(
        "ProviderModuleName",
        providerModuleName,
        CIMKeyBinding::STRING));
    keyBindings.append(CIMKeyBinding(
        "ProviderName",
        providerName,
        CIMKeyBinding::STRING));
    keyBindings.append(CIMKeyBinding(
        "CapabilityID",
        capabilityID,
        CIMKeyBinding::STRING));
    CIMObjectPath path(
        String::EMPTY,
        CIMNamespaceName(),
        CIMName("PG_ProviderCapabilities"),
        keyBindings);

    client.deleteInstance(PEGASUS_NAMESPACENAME_INTEROP, path);
}

inline void TestProviderRegistration::deleteProviderInstance(
    CIMClient& client,
    const String& providerName,
    const String& providerModuleName)
{
    Array<CIMKeyBinding> keyBindings;
    keyBindings.append(CIMKeyBinding(
        "Name",
        providerName,
        CIMKeyBinding::STRING));
    keyBindings.append(CIMKeyBinding(
        "ProviderModuleName",
        providerModuleName,
        CIMKeyBinding::STRING));
    CIMObjectPath path(
        String::EMPTY,
        CIMNamespaceName(),
        CIMName("PG_Provider"),
        keyBindings);

    client.deleteInstance(PEGASUS_NAMESPACENAME_INTEROP, path);
}

inline void TestProviderRegistration::deleteModuleInstance(
    CIMClient& client,
    const String& providerModuleName)
{
    Array<CIMKeyBinding> keyBindings;
    keyBindings.append(CIMKeyBinding(
        "Name",
        providerModuleName,
        CIMKeyBinding::STRING));
    CIMObjectPath path(
        String::EMPTY,
        CIMNamespaceName(),
        CIMName("PG_ProviderModule"),
        keyBindings);

    client.deleteInstance(PEGASUS_NAMESPACENAME_INTEROP, path);
}

# define IGNORE_CIM_ERR_NOT_FOUND(statement, message) \
    try \
    { \
        statement; \
    } \
    catch (CIMException& e) \
    { \
        if (e.getCode() != CIM_ERR_NOT_FOUND) \
        { \
            cerr << message << endl; \
            throw; \
        } \
    } \

#endif
