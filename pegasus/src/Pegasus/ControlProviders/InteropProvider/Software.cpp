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

///////////////////////////////////////////////////////////////////////////////
//  Interop Provider - This provider services those classes from the
//  DMTF Interop schema in an implementation compliant with the SMI-S v1.1
//  Server Profile
//
//  Please see PG_ServerProfile20.mof in the directory
//  $(PEGASUS_ROOT)/Schemas/Pegasus/InterOp/VER20 for retails regarding the
//  classes supported by this control provider.
//
//  Interop forces all creates to the PEGASUS_NAMESPACENAME_INTEROP
//  namespace. There is a test on each operation that returns
//  the Invalid Class CIMDError
//  This is a control provider and as such uses the Tracer functions
//  for data and function traces.  Since we do not expect high volume
//  use we added a number of traces to help diagnostics.
///////////////////////////////////////////////////////////////////////////////

#include "InteropProvider.h"
#include "InteropProviderUtils.h"
#include "InteropConstants.h"

#include <Pegasus/Common/PegasusVersion.h>

PEGASUS_USING_STD;
PEGASUS_NAMESPACE_BEGIN

// Property names for SoftwareIdentity
#define SOFTWAREIDENTITY_PROPERTY_NAME  COMMON_PROPERTY_NAME
#define SOFTWAREIDENTITY_PROPERTY_ELEMENTNAME  COMMON_PROPERTY_ELEMENTNAME
#define SOFTWAREIDENTITY_PROPERTY_CAPTION  COMMON_PROPERTY_CAPTION
#define SOFTWAREIDENTITY_PROPERTY_MAJORVERSION \
    PROVIDERMODULE_PROPERTY_MAJORVERSION
#define SOFTWAREIDENTITY_PROPERTY_MINORVERSION \
    PROVIDERMODULE_PROPERTY_MINORVERSION
#define SOFTWAREIDENTITY_PROPERTY_REVISIONNUMBER \
    PROVIDERMODULE_PROPERTY_REVISIONNUMBER
#define SOFTWAREIDENTITY_PROPERTY_BUILDNUMBER \
    PROVIDERMODULE_PROPERTY_BUILDNUMBER
const CIMName SOFTWAREIDENTITY_PROPERTY_INSTANCEID("InstanceID");
const CIMName SOFTWAREIDENTITY_PROPERTY_VERSION("VersionString");
const CIMName SOFTWAREIDENTITY_PROPERTY_MANUFACTURER("Manufacturer");
const CIMName SOFTWAREIDENTITY_PROPERTY_CLASSIFICATIONS(
    "Classifications");

//
// Given the necessary software information, this function constructs an
// instance of the PG_SoftwareIdentity class.
//
CIMInstance InteropProvider::buildSoftwareIdentity(
        const String & module,
        const String & provider,
        const String & vendor,
        const String & version,
        Uint16 majorVersion,
        Uint16 minorVersion,
        Uint16 revisionNumber,
        Uint16 buildNumber,
        bool extendedVersionSupplied,
        const String & interfaceType,
        const String & elementName,
        const String & caption)
{
    String instanceId(module + "+" + provider);
    String name(provider + " (" + interfaceType + ")");

    // Use double-ifs to prevent locking for every request
    if (softwareIdentityClass.isUninitialized())
    {
        AutoMutex autoMut(interopMut);
        if (softwareIdentityClass.isUninitialized())
        {
            softwareIdentityClass = repository->getClass(
                PEGASUS_NAMESPACENAME_INTEROP,
                PEGASUS_CLASSNAME_PG_SOFTWAREIDENTITY, false, true, false);
        }
    }

    CIMInstance softwareIdentity = softwareIdentityClass.buildInstance(false,
        false, CIMPropertyList());

    setPropertyValue(softwareIdentity, SOFTWAREIDENTITY_PROPERTY_INSTANCEID,
        instanceId);
    setPropertyValue(softwareIdentity, SOFTWAREIDENTITY_PROPERTY_NAME, name);
    setPropertyValue(softwareIdentity, SOFTWAREIDENTITY_PROPERTY_VERSION,
        version);
    setPropertyValue(softwareIdentity, SOFTWAREIDENTITY_PROPERTY_MANUFACTURER,
        vendor);
    setPropertyValue(softwareIdentity,
        SOFTWAREIDENTITY_PROPERTY_CLASSIFICATIONS, providerClassifications);
    if (extendedVersionSupplied)
    {
        setPropertyValue(softwareIdentity,
            SOFTWAREIDENTITY_PROPERTY_MAJORVERSION, majorVersion);
        setPropertyValue(softwareIdentity,
            SOFTWAREIDENTITY_PROPERTY_MINORVERSION, minorVersion);
        setPropertyValue(softwareIdentity,
            SOFTWAREIDENTITY_PROPERTY_REVISIONNUMBER, revisionNumber);
        setPropertyValue(softwareIdentity,
            SOFTWAREIDENTITY_PROPERTY_BUILDNUMBER, buildNumber);
    }

    if (elementName.size() > 0)
    {
        setPropertyValue(softwareIdentity,
            SOFTWAREIDENTITY_PROPERTY_ELEMENTNAME, elementName);
    }

    if (caption.size() > 0)
    {
        setPropertyValue(softwareIdentity,
            SOFTWAREIDENTITY_PROPERTY_CAPTION, caption);
    }

    CIMObjectPath path = softwareIdentity.buildPath(softwareIdentityClass);
    path.setNameSpace(PEGASUS_NAMESPACENAME_INTEROP);
    softwareIdentity.setPath(path);

    return softwareIdentity;
}

//
// Given an instance of PG_Provider, this method extracts information needed to
// construct SoftwareIdentity instances for that provider.
//
void InteropProvider::extractSoftwareIdentityInfo(
    const CIMInstance & providerInstance,
    String & moduleName,
    String & providerName,
    String & vendor,
    String & version,
    Uint16 & majorVersion,
    Uint16 & minorVersion,
    Uint16 & revisionNumber,
    Uint16 & buildNumber,
    bool & extendedVersionSupplied,
    String & interfaceType,
    String & elementName,
    String & caption)
{
    // Get the module and provider name from the PG_ProviderInstance supplied
    moduleName = getRequiredValue<String>(providerInstance,
        PROVIDER_PROPERTY_PROVIDERMODULENAME);
    providerName = getRequiredValue<String>(providerInstance,
        PROVIDER_PROPERTY_NAME);

    // Need to find the ProviderModule instance for this provider to retrieve
    // version information.
    CIMKeyBinding pmKey(PROVIDERMODULE_PROPERTY_NAME, moduleName);
    Array<CIMKeyBinding> pmKeyBindings;
    pmKeyBindings.append(pmKey);
    CIMObjectPath providerModuleName(
        String::EMPTY,
        CIMNamespaceName(),
        PEGASUS_CLASSNAME_PROVIDERMODULE,
        pmKeyBindings);
    CIMInstance providerModule = repository->getInstance(
        PEGASUS_NAMESPACENAME_INTEROP, providerModuleName,
        false, false, CIMPropertyList());

    version = getRequiredValue<String>(providerModule,
        PROVIDERMODULE_PROPERTY_VERSION);
    vendor = getRequiredValue<String>(providerModule,
        PROVIDERMODULE_PROPERTY_VENDOR);
    interfaceType = getRequiredValue<String>(providerModule,
        PROVIDERMODULE_PROPERTY_INTERFACETYPE);

    // Extract the element name if present
    Uint32 elementNameIndex = providerInstance.findProperty(
        PROVIDER_PROPERTY_ELEMENTNAME);
    if (elementNameIndex != PEG_NOT_FOUND)
    {
        CIMValue elementNameValue(
            providerInstance.getProperty(elementNameIndex).getValue());
        if (!elementNameValue.isNull())
        {
            elementNameValue.get(elementName);
        }
    }

    // Extract the caption if present
    Uint32 captionIndex = providerInstance.findProperty(
        PROVIDER_PROPERTY_CAPTION);
    if (elementNameIndex != PEG_NOT_FOUND)
    {
        CIMValue captionValue(
            providerInstance.getProperty(captionIndex).getValue());
        if (!captionValue.isNull())
        {
            captionValue.get(caption);
        }
    }

    // Now see if optional extended version information is available
    extendedVersionSupplied = false;
    Uint32 majorIndex = providerInstance.findProperty(
        PROVIDERMODULE_PROPERTY_MAJORVERSION);
    if (majorIndex != PEG_NOT_FOUND)
    {
        CIMValue majorValue =
            providerInstance.getProperty(majorIndex).getValue();
        if (!majorValue.isNull())
        {
            extendedVersionSupplied = true;
            majorValue.get(majorVersion);
            minorVersion = 0;
            revisionNumber = 0;
            buildNumber = 0;

            // Get the Version if present
            Uint32 index = providerInstance.findProperty(
                PROVIDERMODULE_PROPERTY_VERSION);
            if (index != PEG_NOT_FOUND)
            {
                CIMValue propValue =
                  providerInstance.getProperty(index).getValue();
                if (!propValue.isNull())
                {
                    propValue.get(version);
                }
            }

            // Get the Minor version if present
            index = providerInstance.findProperty(
                PROVIDERMODULE_PROPERTY_MINORVERSION);
            if (index != PEG_NOT_FOUND)
            {
                CIMValue propValue =
                  providerInstance.getProperty(index).getValue();
                if (!propValue.isNull())
                {
                    propValue.get(minorVersion);
                }
            }

            // Get the revision number if present
            index = providerInstance.findProperty(
                PROVIDERMODULE_PROPERTY_REVISIONNUMBER);
            if (index != PEG_NOT_FOUND)
            {
                CIMValue propValue =
                  providerInstance.getProperty(index).getValue();
                if (!propValue.isNull())
                {
                    propValue.get(revisionNumber);
                }
            }

            // Get the build number if present
            index = providerInstance.findProperty(
                    PROVIDERMODULE_PROPERTY_BUILDNUMBER);
            if (index != PEG_NOT_FOUND)
            {
                CIMValue propValue =
                  providerInstance.getProperty(index).getValue();
                if (!propValue.isNull())
                {
                    propValue.get(buildNumber);
                }
            }
        }
    }
}

CIMInstance InteropProvider::getSoftwareIdentityInstance(
    const CIMObjectPath &ref)
{
    Array<CIMKeyBinding> keyBindings = ref.getKeyBindings();
    String id;

    for (Uint32 j = 0, n = keyBindings.size(); j < n ; j++)
    {
        if (keyBindings[j].getName().equal(
            SOFTWAREIDENTITY_PROPERTY_INSTANCEID))
        {
            id = keyBindings[j].getValue();
            break;
        }
    }

    Uint32 index;
    String provModuleName;
    String provName;

    if ((index = id.find('+')) != PEG_NOT_FOUND)
    {
        provModuleName = id.subString(0, index);
        provName = id.subString(index +1);
    }
    else
    {
        throw CIMObjectNotFoundException(ref.toString());
    }

    Array<CIMKeyBinding> provKeys;
    
    provKeys.append(
        CIMKeyBinding(
            PROVIDER_PROPERTY_PROVIDERMODULENAME,
            provModuleName,
            CIMKeyBinding::STRING));

    provKeys.append(
        CIMKeyBinding(
            PROVIDER_PROPERTY_NAME,
            provName,
            CIMKeyBinding::STRING));

    CIMInstance provider;
    Boolean providerFound = true;
    try
    {
        provider =  providerRegistrationManager->getInstance(
            CIMObjectPath( 
                String(),
                PEGASUS_NAMESPACENAME_INTEROP,
                PEGASUS_CLASSNAME_PROVIDER,
                provKeys)
            );
    }
    catch(const CIMException&)
    {
        providerFound = false;
    }

    if (!providerFound)
    {
        Array<CIMInstance> instances = 
            enumDefaultSoftwareIdentityInstances();
      
        for(Uint32 i = 0, n = instances.size(); i < n; i++)
        {
            CIMObjectPath currentInstRef = instances[i].getPath();
            currentInstRef.setHost(ref.getHost());
            currentInstRef.setNameSpace(ref.getNameSpace());
            if(ref == currentInstRef)
            {
                return instances[i];
            }
        }
        throw CIMObjectNotFoundException(ref.toString());
     }

    String moduleName;
    String providerName;
    String version;
    String vendor;
    String interfaceType;
    Uint16 majorVersion;
    Uint16 minorVersion;
    Uint16 revisionNumber;
    Uint16 buildNumber;
    String elementName;
    String caption;
    bool extendedVersionInfo;

    extractSoftwareIdentityInfo(
        provider,
        moduleName,
        providerName,
        vendor,
        version,
        majorVersion,
        minorVersion,
        revisionNumber,
        buildNumber,
        extendedVersionInfo,
        interfaceType,
        elementName,
        caption);

    return buildSoftwareIdentity(
        moduleName,
        providerName,
        vendor,
        version,
        majorVersion,
        minorVersion,
        revisionNumber,
        buildNumber,
        extendedVersionInfo,
        interfaceType,
        elementName,
        caption);
}

//Gets default software identity instances served by CIMOM
Array<CIMInstance> InteropProvider::enumDefaultSoftwareIdentityInstances()
{
    Array<CIMInstance> instances;

    // Interop provider
    instances.append(buildSoftwareIdentity(PEGASUS_MODULE_NAME,
        INTEROP_PROVIDER_NAME, PEGASUS_CIMOM_GENERIC_NAME,
        PEGASUS_PRODUCT_VERSION,
        0, 0, 0, 0, false, // no extended revision info
        PEGASUS_INTERNAL_PROVIDER_TYPE, String::EMPTY, String::EMPTY));

    // Pegasus Indication Service acts as Provider for CIM Event classes.
    // If DMTF Indications Profile support is enabled create SoftwareIdentity
    // instance with Pegasus Indication Service.
#ifdef PEGASUS_ENABLE_DMTF_INDICATION_PROFILE_SUPPORT
    instances.append(
        buildSoftwareIdentity(
            PEGASUS_MODULE_NAME,
            INDICATION_SERVICE_NAME,
            PEGASUS_CIMOM_GENERIC_NAME,
            PEGASUS_PRODUCT_VERSION,
            0,
            0,
            0,
            0,
            false, // no extended revision info
            PEGASUS_INTERNAL_SERVICE_TYPE,
            String::EMPTY,
            String::EMPTY));
#endif
    return instances;
}

//
// Method that enumerates instances of the PG_SoftwareIdentity class. There
// should be one instance for every provider registered with the CIMOM, i.e.
// one for every instance of PG_Provider in the interop namespace in the
// repository.
//
Array<CIMInstance> InteropProvider::enumSoftwareIdentityInstances()
{
    Array<CIMInstance> instances;

    Array<CIMInstance> registeredProviders =
        repository->enumerateInstancesForClass(
            PEGASUS_NAMESPACENAME_INTEROP,
            PEGASUS_CLASSNAME_PROVIDER,
            false);
    for (Uint32 i = 0, n = registeredProviders.size(); i < n; ++i)
    {
        String moduleName;
        String providerName;
        String version;
        String vendor;
        String interfaceType;
        Uint16 majorVersion;
        Uint16 minorVersion;
        Uint16 revisionNumber;
        Uint16 buildNumber;
        String elementName;
        String caption;
        bool extendedVersionInfo;
        extractSoftwareIdentityInfo(registeredProviders[i], moduleName,
            providerName, vendor, version, majorVersion, minorVersion,
            revisionNumber, buildNumber, extendedVersionInfo, interfaceType,
            elementName, caption);

        instances.append(buildSoftwareIdentity(moduleName, providerName,
            vendor, version, majorVersion, minorVersion, revisionNumber,
            buildNumber, extendedVersionInfo, interfaceType, elementName,
            caption));
    }

    instances.appendArray(enumDefaultSoftwareIdentityInstances());

    return instances;
}

//
// Enumerates instances of the ElementSoftwareIdentity association class. There
// will be one instance for each profile and subprofile contained in an
// instance of ProviderProfileCapabilities.
//
Array<CIMInstance> InteropProvider::enumElementSoftwareIdentityInstances()
{
    Array<CIMInstance> instances;

    Array<CIMInstance> profileCapabilities =
        enumProviderProfileCapabilityInstances(true);

    CIMClass elementSoftwareIdentityClass = repository->getClass(
        PEGASUS_NAMESPACENAME_INTEROP,
        PEGASUS_CLASSNAME_PG_ELEMENTSOFTWAREIDENTITY, false, true, false);
    CIMClass registeredProfileClass = repository->getClass(
        PEGASUS_NAMESPACENAME_INTEROP, PEGASUS_CLASSNAME_PG_REGISTEREDPROFILE,
        false, true, false);

    for (Uint32 i = 0, n = profileCapabilities.size(); i < n; ++i)
    {
        CIMInstance & currentCapabilities = profileCapabilities[i];
        String version;
        String organizationName;
        Array<String> subprofiles;
        String profileName;
        Uint16 profileOrg = 0;
        Array<Uint16> dummyIntArray;
        Array<String> subprofileVersions;
        Array<String> subprofileOrgs;
        Array<String> subprofileProviderModuleNames;
        Array<String> subprofileProviderNames;

        String profileId = extractProfileInfo(currentCapabilities,
            profileCapabilitiesClass,
            registeredProfileClass,
            profileName,
            version,
            profileOrg,
            organizationName,
            subprofiles,
            subprofileVersions,
            dummyIntArray, // Throw away subprofile organization enums
            subprofileOrgs,
            subprofileProviderModuleNames,
            subprofileProviderNames,
            false); // Get subprofile information

        if (!(String::equalNoCase(profileName,"SMI-S")))
        {
            String moduleName = getRequiredValue<String>(currentCapabilities,
                CAPABILITIES_PROPERTY_PROVIDERMODULENAME);
            String providerName = getRequiredValue<String>(currentCapabilities,
                CAPABILITIES_PROPERTY_PROVIDERNAME);

            String softwareInstanceId = moduleName + "+" + providerName;
            // Create an association between the Provider's SoftwareIdentity and
            // this Registered Profile.
            instances.append(buildDependencyInstance(
                softwareInstanceId,
                PEGASUS_CLASSNAME_PG_SOFTWAREIDENTITY,
                profileId,
                PEGASUS_CLASSNAME_PG_REGISTEREDPROFILE,
                elementSoftwareIdentityClass));

            // Loop through the subprofile info and create associations between
            // the Provider's SoftwareIdentity and the Registered Subprofiles.
            for (Uint32 j = 0, m = subprofiles.size(); j < m; ++j)
            {
                String subprofileVersion;
                if (subprofileVersions.size() == 0)
                {
                    subprofileVersion = version;
                }
                else
                {
                    subprofileVersion = subprofileVersions[j];
                }
                // Check if subprofile is implemented in a different provider
                // module or provider.
                if (subprofileProviderModuleNames.size())
                {
                    softwareInstanceId = subprofileProviderModuleNames[j];
                    softwareInstanceId.append("+");
                    softwareInstanceId.append(subprofileProviderNames[j]);
                }

                instances.append(buildDependencyInstance(
                    softwareInstanceId,
                    PEGASUS_CLASSNAME_PG_SOFTWAREIDENTITY,
                    buildProfileInstanceId(subprofileOrgs[j], subprofiles[j],
                        subprofileVersion),
                    profileOrg == DMTF_NUM ?
                        PEGASUS_CLASSNAME_PG_REGISTEREDPROFILE :
                        PEGASUS_CLASSNAME_PG_REGISTEREDSUBPROFILE,
                    elementSoftwareIdentityClass));
            }
        }
    }

    // Create default association between Server profile, Indications
    // subprofile, and the Pegasus Interoperability provider software identity
    String interopSoftwareIdentity = PEGASUS_MODULE_NAME + "+" +
        INTEROP_PROVIDER_NAME;
    String serverProfileId, indicationProfileId, softwareProfileId;

    //add instances for smi-s version 1.1.0
    serverProfileId = buildProfileInstanceId(
        SNIA_NAME,
        "Server",
        SNIA_VER_110);
    indicationProfileId = buildProfileInstanceId(
        SNIA_NAME,
        "Indication",
        SNIA_VER_110);
    softwareProfileId = buildProfileInstanceId(
        SNIA_NAME,
        "Software",
        SNIA_VER_110);

    instances.append(buildDependencyInstance(
        interopSoftwareIdentity,
        PEGASUS_CLASSNAME_PG_SOFTWAREIDENTITY,
        serverProfileId,
        PEGASUS_CLASSNAME_PG_REGISTEREDPROFILE,
        elementSoftwareIdentityClass));

    instances.append(buildDependencyInstance(
        interopSoftwareIdentity,
        PEGASUS_CLASSNAME_PG_SOFTWAREIDENTITY,
        indicationProfileId,
        PEGASUS_CLASSNAME_PG_REGISTEREDSUBPROFILE,
        elementSoftwareIdentityClass));

    instances.append(buildDependencyInstance(
        interopSoftwareIdentity,
        PEGASUS_CLASSNAME_PG_SOFTWAREIDENTITY,
        softwareProfileId,
        PEGASUS_CLASSNAME_PG_REGISTEREDSUBPROFILE,
        elementSoftwareIdentityClass));

    //Add instances for smi-s version 1.2.0

    String profileRegistrationProfileId;
    serverProfileId = buildProfileInstanceId(
        SNIA_NAME,
        "Server",
        SNIA_VER_120);
    profileRegistrationProfileId = buildProfileInstanceId(
        SNIA_NAME,
        "Profile Registration",
        SNIA_VER_100);
    indicationProfileId = buildProfileInstanceId(
        SNIA_NAME,
        "Indication",
        SNIA_VER_120);
    softwareProfileId = buildProfileInstanceId(
        SNIA_NAME,
        "Software",
        SNIA_VER_120);

    instances.append(buildDependencyInstance(
        interopSoftwareIdentity,
        PEGASUS_CLASSNAME_PG_SOFTWAREIDENTITY,
        serverProfileId,
        PEGASUS_CLASSNAME_PG_REGISTEREDPROFILE,
        elementSoftwareIdentityClass));

    instances.append(buildDependencyInstance(
        interopSoftwareIdentity,
        PEGASUS_CLASSNAME_PG_SOFTWAREIDENTITY,
        profileRegistrationProfileId,
        PEGASUS_CLASSNAME_PG_REGISTEREDPROFILE,
        elementSoftwareIdentityClass));

    instances.append(buildDependencyInstance(
        interopSoftwareIdentity,
        PEGASUS_CLASSNAME_PG_SOFTWAREIDENTITY,
        indicationProfileId,
        PEGASUS_CLASSNAME_PG_REGISTEREDSUBPROFILE,
        elementSoftwareIdentityClass));

    instances.append(buildDependencyInstance(
        interopSoftwareIdentity,
        PEGASUS_CLASSNAME_PG_SOFTWAREIDENTITY,
        softwareProfileId,
        PEGASUS_CLASSNAME_PG_REGISTEREDSUBPROFILE,
        elementSoftwareIdentityClass));

#ifdef PEGASUS_ENABLE_DMTF_INDICATION_PROFILE_SUPPORT
    String indProfileId = buildProfileInstanceId(
        DMTF_NAME,
        "Indications",
        DMTF_VER_110);

    String indicationServiceSoftwareIdentity(PEGASUS_MODULE_NAME);
    indicationServiceSoftwareIdentity.append("+");
    indicationServiceSoftwareIdentity.append(INDICATION_SERVICE_NAME);

    instances.append(
        buildDependencyInstance(
            indicationServiceSoftwareIdentity,
            PEGASUS_CLASSNAME_PG_SOFTWAREIDENTITY,
            indProfileId,
            PEGASUS_CLASSNAME_PG_REGISTEREDPROFILE,
            elementSoftwareIdentityClass));
#endif

    return instances;
}

//
// Enumerates instances of the InstalledSoftwareIdentity association class.
//
Array<CIMInstance> InteropProvider::enumInstalledSoftwareIdentityInstances(
    const OperationContext &opContext)
{
    // All of the software identity instances are associated to the
    // ComputerSystem on which the object manager resides. Simply loop through
    // all the instances and build the association.
    Array<CIMInstance> instances;
    CIMObjectPath csPath = getComputerSystemInstance(opContext).getPath();
    Array<CIMInstance> softwareInstances = enumSoftwareIdentityInstances();
    CIMClass installedSoftwareClass;
    CIMInstance skeletonInst =  buildInstanceSkeleton(
        PEGASUS_NAMESPACENAME_INTEROP,
        PEGASUS_CLASSNAME_PG_INSTALLEDSOFTWAREIDENTITY,
        true,
        installedSoftwareClass);
    for (Uint32 i = 0, n = softwareInstances.size(); i < n; ++i)
    {
        CIMInstance installedSoftwareInstance = skeletonInst.clone();
        setPropertyValue(installedSoftwareInstance,
            INSTALLEDSOFTWAREIDENTITY_PROPERTY_INSTALLEDSOFTWARE,
            CIMValue(softwareInstances[i].getPath()));
        setPropertyValue(installedSoftwareInstance,
            INSTALLEDSOFTWAREIDENTITY_PROPERTY_SYSTEM,
            CIMValue(csPath));
        installedSoftwareInstance.setPath(installedSoftwareInstance.buildPath(
            installedSoftwareClass));
        instances.append(installedSoftwareInstance);
    }

    return instances;
}

PEGASUS_NAMESPACE_END
// END_OF_FILE
