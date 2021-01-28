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

PEGASUS_USING_STD;
PEGASUS_NAMESPACE_BEGIN

//
// Given the two references in the association, this function creates an
// instance of the PG_ElementConformsToProfile class.
//
CIMInstance buildElementConformsToProfile(
    const CIMObjectPath & currentProfile,
    const CIMObjectPath & currentElement,
    const CIMClass & elementConformsClass)
{
    Array<CIMName> elementPropArray;
    elementPropArray.append(
        ELEMENTCONFORMSTOPROFILE_PROPERTY_CONFORMANTSTANDARD);
    elementPropArray.append(
        ELEMENTCONFORMSTOPROFILE_PROPERTY_MANAGEDELEMENT);
    CIMPropertyList elementPropList(elementPropArray);

    CIMInstance tmpInstance =
        elementConformsClass.buildInstance(false, false,
            elementPropList);
    setPropertyValue(tmpInstance,
        ELEMENTCONFORMSTOPROFILE_PROPERTY_CONFORMANTSTANDARD,
        currentProfile);
    setPropertyValue(tmpInstance,
        ELEMENTCONFORMSTOPROFILE_PROPERTY_MANAGEDELEMENT,
        currentElement);
    tmpInstance.setPath(tmpInstance.buildPath(
        elementConformsClass));
    return tmpInstance;
}

Array<CIMInstance> InteropProvider::enumElementConformsToProfileRPRPInstances(
    const OperationContext & opContext,
    const CIMNamespaceName & opNamespace)
{
    CIMClass elementConformsClass = repository->getClass(
        PEGASUS_NAMESPACENAME_INTEROP,
        PEGASUS_CLASSNAME_PG_ELEMENTCONFORMSTOPROFILE_RP_RP,
        false, true, false);

    Array<CIMInstance> instances;
    CIMObjectPath smisVersionProfile, profRegProfile;

    if (opNamespace == PEGASUS_NAMESPACENAME_INTEROP)
    {
        //Add associations between the 1.2 SMIS-Version profile and all
        //the version 1.2.0 profiles and subprofiles.
        smisVersionProfile = buildDependencyReference(
            hostName,
            buildProfileInstanceId(SNIA_NAME, "SMI-S", SNIA_VER_120),
            PEGASUS_CLASSNAME_PG_REGISTEREDPROFILE);

        Array<CIMInstance> profileInstances =
            enumRegisteredProfileInstances();
        Array<CIMInstance> subprofileInstances =
            enumRegisteredSubProfileInstances();
        profileInstances.appendArray(subprofileInstances);
        Array<CIMInstance> profilesForVersion = getProfilesForVersion(
            profileInstances, SNIA_NUM, 1, 2, 0);
        for (Uint32 i = 0, n = profilesForVersion.size(); i < n; ++i)
        {
            instances.append(buildElementConformsToProfile(
                smisVersionProfile,
                profilesForVersion[i].getPath(),
                elementConformsClass));
        }
    }
    return instances;
}

//
// Enumerates all of the ElementConformsToProfile association instances.
//
Array<CIMInstance> InteropProvider::enumElementConformsToProfileInstances(
    const OperationContext & opContext, const CIMNamespaceName & opNamespace)
{
    CIMClass elementConformsClass = repository->getClass(
        PEGASUS_NAMESPACENAME_INTEROP,
        PEGASUS_CLASSNAME_PG_ELEMENTCONFORMSTOPROFILE,
        false, true, false);

    AutoMutex holder(interopMut);
    Array<CIMInstance> instances;
    verifyCachedInfo();
    // Loop through the cached profile Id's and related info about its
    // conforming elements.
    for (Uint32 i = 0, n = profileIds.size(); i < n; ++i)
    {
        String & profileId = profileIds[i];
        Array<CIMName> & elementList = conformingElements[i];
        Array<CIMNamespaceName> & namespaceList = elementNamespaces[i];
        Array<CIMObjectPath> conformingElementPaths;
        for (Uint32 j = 0, m = elementList.size(); j < m; ++j)
        {
            CIMName & currentElement = elementList[j];
            CIMNamespaceName & currentNamespace = namespaceList[j];

            if (opNamespace == PEGASUS_NAMESPACENAME_INTEROP ||
                opNamespace == currentNamespace)
            {
                String currentElementStr(currentElement.getString());
                if (currentElementStr.find(PEGASUS_DYNAMIC) == 0)
                {
                    // If the provider profile registration did not provide a
                    // list of conforming elements (presumably because there is
                    // no such definite list), then the provider is required
                    // to provide instances of ElementConformsToProfile in the
                    // vendor namespace, so we do not generate instances.
                    if (opNamespace != PEGASUS_NAMESPACENAME_INTEROP)
                    {
                        continue;
                    }
                    CIMName subclassName(
                        currentElementStr.subString(PEGASUS_DYNAMIC_LEN));
                    Array<CIMInstance> elementConformsInstances =
                        cimomHandle.enumerateInstances(opContext,
                        currentNamespace, subclassName, true, false, false,
                        true, CIMPropertyList());

                    // Retrieve the Conforming Element
                    for (Uint32 k = 0, x = elementConformsInstances.size();
                        k < x; ++k)
                    {
                        CIMInstance & currentInstance =
                            elementConformsInstances[k];

                        // NOCHKSRC
                        // Make sure that the current instance points to the
                        // current profile ID.
                        CIMObjectPath profilePath =
                            getRequiredValue<CIMObjectPath>(
                                elementConformsInstances[k],
                                ELEMENTCONFORMSTOPROFILE_PROPERTY_CONFORMANTSTANDARD);
                        // DOCHKSRC
                        const Array<CIMKeyBinding> & keys =
                            profilePath.getKeyBindings();
                        if (keys.size() != 1)
                            continue;
                        if (keys.size() == 1 && keys[0].getValue() == profileId)
                        {
                            // NOCHKSRC
                            conformingElementPaths.append(
                                getRequiredValue<CIMObjectPath>(
                                currentInstance,
                                ELEMENTCONFORMSTOPROFILE_PROPERTY_MANAGEDELEMENT));
                            // DOCHKSRC
                        }
                    }
                }
                else
                {
                    // All of the instances of the current element in the
                    // corresponding namespace conform to the current profile.
                    Array<CIMObjectPath> paths =
                        cimomHandle.enumerateInstanceNames(opContext,
                            currentNamespace, currentElement);
                    // Set the namespace in the paths just in case
                    for (Uint32 k = 0, x = paths.size();
                        k < x; ++k)
                    {
                        CIMObjectPath & curPath = paths[k];
                        curPath.setNameSpace(currentNamespace);
                        curPath.setHost(hostName);
                    }
                    conformingElementPaths.appendArray(paths);
                }
            }
        }

        // Create the object path for the RegisteredProfile using the given
        // profileId.
        CIMObjectPath profilePath = buildDependencyReference(
            hostName, profileIds[i], PEGASUS_CLASSNAME_PG_REGISTEREDPROFILE);

        // Build all of the ElementConformsToProfile instances for the current
        // profile.
        for (Uint32 k = 0, x = conformingElementPaths.size(); k < x; ++k)
        {
            instances.append(buildElementConformsToProfile(profilePath,
                conformingElementPaths[k], elementConformsClass));
        }
    }

    // Now add the default instance: the association between the Server Profile
    // and the ObjectManager (if we're in the Interop namespace)
    if (opNamespace == PEGASUS_NAMESPACENAME_INTEROP)
    {
        // Build up the Object Path for the server profile version 1.1.0
        CIMObjectPath serverProfile = buildDependencyReference(
            hostName,
            buildProfileInstanceId(SNIA_NAME, "Server", SNIA_VER_110),
            PEGASUS_CLASSNAME_PG_REGISTEREDPROFILE);
        // Retrieve the Object Manager instance
        CIMInstance objManager = getObjectManagerInstance();

        instances.append(
            buildElementConformsToProfile(
                serverProfile,
                objManager.getPath(),
                elementConformsClass));

        // Build up the Object Path for the server profile ver 1.2.0
        // and add the elementconformstoprofile association instance
        serverProfile = buildDependencyReference(
            hostName,
            buildProfileInstanceId(SNIA_NAME, "Server", SNIA_VER_120),
            PEGASUS_CLASSNAME_PG_REGISTEREDPROFILE);
        instances.append(
            buildElementConformsToProfile(
                serverProfile,
                objManager.getPath(),
                elementConformsClass));

    }

#ifdef PEGASUS_ENABLE_DMTF_INDICATION_PROFILE_SUPPORT
    // Now add the  association between the Indication profile
    // and IndicationService
    if (opNamespace == PEGASUS_NAMESPACENAME_INTEROP)
    {
        CIMObjectPath serverProfile = buildDependencyReference(
            hostName,
            buildProfileInstanceId(DMTF_NAME, "Indications", DMTF_VER_110),
            PEGASUS_CLASSNAME_PG_REGISTEREDPROFILE);
        // Retrieve the IndicationService instance
        Array<CIMInstance> indService =
            enumIndicationServiceInstances(OperationContext());

        instances.append(
            buildElementConformsToProfile(
                serverProfile,
                indService[0].getPath(),
                elementConformsClass));
    }
#endif

    return instances;
}

//Method that filters the registered profile or registered subprofile instances
//whose versions are greater or equal to the given version.
Array<CIMInstance> InteropProvider::getProfilesForVersion(
    Array<CIMInstance>& profiles,
    Uint16 regOrg,
    Uint32 majorVer,
    Uint32 minorVer,
    Uint32 updateVer)
{
    static const String SMISProfileName("SMI-S");
    static const String SNIAIndicationProfileName("Indication");
    static const String DMTFIndicationProfileName("Indications");
    static const String ServerProfileName("Server");
    static const String SoftwareProfileName("Software");

    Array<CIMInstance> instances;
    instances.clear();
    for (Uint32 i = 0, n = profiles.size(); i < n; ++i)
    {
        String versionNumber;
        String profileName;
        //Can use 0 here as registered organization value of 0 is 
        //ruled out(not specified in the CIM Schema
        Uint16 regOrgNo = 0;

        Uint32 index = profiles[i].findProperty("RegisteredVersion");
        if (index != PEG_NOT_FOUND)
        {
            const CIMValue &tmpVal = profiles[i].getProperty(index).getValue();
            if (!tmpVal.isNull())
            {
              tmpVal.get(versionNumber);
            }
        }
        index = profiles[i].findProperty("RegisteredName");
        if (index != PEG_NOT_FOUND)
        {
            const CIMValue &tmpVal = profiles[i].getProperty(index).getValue();
            if (!tmpVal.isNull())
            {
                tmpVal.get(profileName);
            }
        }
        index = profiles[i].findProperty("RegisteredOrganization");
        if (index != PEG_NOT_FOUND)
        {
            const CIMValue &tmpVal = profiles[i].getProperty(index).getValue();
            if (!tmpVal.isNull())
            {
                tmpVal.get(regOrgNo);
            }
        }

        if ( regOrgNo != 0 && regOrg == regOrgNo)
        {
            if (profileName == ServerProfileName ||
                (regOrg == SNIA_NUM &&
                    profileName == SNIAIndicationProfileName) ||
                (regOrg == DMTF_NUM
                    && profileName == DMTFIndicationProfileName) ||
                profileName == SoftwareProfileName)
            {
                if (VersionUtil::isVersionGreaterOrEqual(
                    versionNumber, majorVer, minorVer, updateVer))
                {
                    instances.append(profiles[i]);
                }
            }
            else if (profileName != SMISProfileName)
            {
                instances.append(profiles[i]);
            }
        }
    }
    return instances;
}


typedef Array<String> StringArray;

void InteropProvider::verifyCachedInfo()
{
    if (!updateProfileCache.get())
    {
        return;
    }
    // At present cache will be rebuilt in the following conditions.
    // (1) When instances of PG_ProviderProfileCapabilities is created
    //     or deleted.
    // (2) When Provider and ProviderCapabilities instances
    //     are created or Provider, ProviderModule and ProviderCapabilities
    //     instance are deleted.
    // (3) When Provider or ProviderModule is disabled or enabled.
    AutoMutex mtx(interopMut);
    if (updateProfileCache.get())
    {
        initializeNamespaces();
        cacheProfileRegistrationInfo();
        updateProfileCache--;
    }
}

//
// Function that determines in which namespaces a provider supports a given
// class. This information is needed to properly implement the
// ElementConformsToProfile association.
//
Array<String> findProviderNamespacesForElement(
    const String & moduleName, const String & providerName,
    const CIMName & elementClass, CIMRepository * repository,
    Array<CIMInstance> & providerCapabilitiesInstances)
{
    Array<CIMInstance> capabilities;
    if (providerCapabilitiesInstances.size() == 0)
    {
        Array<CIMName> propList;
        propList.append(PROVIDERCAPABILITIES_PROPERTY_PROVIDERMODULENAME);
        propList.append(PROVIDERCAPABILITIES_PROPERTY_PROVIDERNAME);
        propList.append(PROVIDERCAPABILITIES_PROPERTY_NAMESPACES);
        propList.append(PROVIDERCAPABILITIES_PROPERTY_CLASSNAME);
        capabilities = repository->enumerateInstancesForClass(
            PEGASUS_NAMESPACENAME_INTEROP,
            PEGASUS_CLASSNAME_PROVIDERCAPABILITIES, false, false);
    }
    else
    {
        capabilities = providerCapabilitiesInstances;
    }

    for (Uint32 i = 0, n = capabilities.size(); i < n; ++i)
    {
        CIMInstance & currentCapabilities = capabilities[i];
        Uint32 propIndex = currentCapabilities.findProperty(
            PROVIDERCAPABILITIES_PROPERTY_PROVIDERMODULENAME);
        PEGASUS_ASSERT(propIndex != PEG_NOT_FOUND);
        String currentName;
        currentCapabilities.getProperty(propIndex).getValue().get(
            currentName);
        if (currentName == moduleName)
        {
            propIndex = currentCapabilities.findProperty(
                PROVIDERCAPABILITIES_PROPERTY_PROVIDERNAME);
            PEGASUS_ASSERT(propIndex != PEG_NOT_FOUND);
            currentCapabilities.getProperty(propIndex).getValue().get(
                currentName);
            if (currentName == providerName)
            {
                propIndex = currentCapabilities.findProperty(
                    PROVIDERCAPABILITIES_PROPERTY_CLASSNAME);
                PEGASUS_ASSERT(propIndex != PEG_NOT_FOUND);
                currentCapabilities.getProperty(propIndex).getValue().get(
                    currentName);
                if (elementClass.equal(CIMName(currentName)))
                {
                    propIndex = currentCapabilities.findProperty(
                      PROVIDERCAPABILITIES_PROPERTY_NAMESPACES);
                    PEGASUS_ASSERT(propIndex != PEG_NOT_FOUND);
                    Array<String> namespaces;
                    currentCapabilities.getProperty(propIndex).getValue().get(
                        namespaces);
                    return namespaces;
                }
            }
        }
    }

    throw PEGASUS_CIM_EXCEPTION(CIM_ERR_NOT_FOUND,
        "Could not find provider capabilities registered for module " +
        moduleName + ", provider " + providerName + ", and class " +
        elementClass.getString());
}

//
// Method that caches certain profile registration information. Specifically,
// information pertaining to the ElementConformsToProfofile association
// implementation is kept here.
//
void InteropProvider::cacheProfileRegistrationInfo()
{
    Array<CIMInstance> instances;
    Array<CIMInstance> providerCapabilitiesInstances;

    // Clear existing cache
    profileIds.clear();
    conformingElements.clear();
    elementNamespaces.clear();

    // Retrieve all of the provider profile registration info
    Array<CIMName> propList;
    propList.append(CAPABILITIES_PROPERTY_PROVIDERMODULENAME);
    propList.append(CAPABILITIES_PROPERTY_PROVIDERNAME);
    propList.append(PROFILECAPABILITIES_PROPERTY_PROFILEVERSION);
    propList.append(PROFILECAPABILITIES_PROPERTY_REGISTEREDPROFILE);
    propList.append(PROFILECAPABILITIES_PROPERTY_OTHERREGISTEREDPROFILE);
    propList.append(PROFILECAPABILITIES_PROPERTY_OTHERPROFILEORGANIZATION);
    propList.append(PROFILECAPABILITIES_PROPERTY_CONFORMINGELEMENTS);

    Array<CIMInstance> providerProfileInstances =
        enumProviderProfileCapabilityInstances(
            true,
            false,
            false,
            CIMPropertyList(propList));

    CIMClass elementConformsClass = repository->getClass(
        PEGASUS_NAMESPACENAME_INTEROP,
        PEGASUS_CLASSNAME_PG_ELEMENTCONFORMSTOPROFILE,
        false, true, false);
    CIMClass registeredProfileClass = repository->getClass(
        PEGASUS_NAMESPACENAME_INTEROP,
        PEGASUS_CLASSNAME_PG_REGISTEREDPROFILE,
        false, true, false);
    Array<CIMInstance> capabilities;

    // Loop through the provider profile info to determine what profiles are
    // supported by what providers, and to build the ElementConformsToProfile
    // associations.
    for (Uint32 i = 0, n = providerProfileInstances.size(); i < n; ++i)
    {
        CIMInstance & currentProfileInstance = providerProfileInstances[i];
        String moduleName = getRequiredValue<String>(currentProfileInstance,
            CAPABILITIES_PROPERTY_PROVIDERMODULENAME);
        String providerName = getRequiredValue<String>(currentProfileInstance,
            CAPABILITIES_PROPERTY_PROVIDERNAME);
        String profileName;
        Uint16 profileOrganization = 0;
        String profileVersion;
        String organizationName;
        Array<String> profileNames; // Not going to use this info
        Array<String> profileVersions; // Not going to use this info
        Array<Uint16> profileOrganizations; // Not going to use this info
        Array<String> profileOrganizationNames; // Not going to use this info
        Array<String> providerModuleNames; // Not going to use this info
        Array<String> providerNames; // Not going to use this info

        String profileId = extractProfileInfo(currentProfileInstance,
            profileCapabilitiesClass,
            registeredProfileClass,
            profileName,
            profileVersion,
            profileOrganization,
            organizationName,
            profileNames,
            profileVersions,
            profileOrganizations,
            profileOrganizationNames,
            providerModuleNames,
            providerNames,
            true);

        Uint32 propIndex = currentProfileInstance.findProperty(
              PROFILECAPABILITIES_PROPERTY_CONFORMINGELEMENTS);

        Array<CIMName> elementPropArray;
        elementPropArray.append(
            ELEMENTCONFORMSTOPROFILE_PROPERTY_CONFORMANTSTANDARD);
        elementPropArray.append(
            ELEMENTCONFORMSTOPROFILE_PROPERTY_MANAGEDELEMENT);
        CIMPropertyList elementPropList(elementPropArray);

        Array<CIMName> conformingElementsForProfile;
        Array<CIMNamespaceName> elementNamespacesForProfile;

        Array<String> elementClasses;
        currentProfileInstance.getProperty(propIndex).getValue().get(
            elementClasses);
        //if (propIndex == PEG_NOT_FOUND)
        if (elementClasses.size() == 0)
        {
            // Get the namespaces in which this provider operates and trim down
            // the list of capabilities instaces to just those that are related
            // to this one.
            if (capabilities.size() == 0)
            {
                Array<CIMName> capPropList;
                capPropList.append(
                    PROVIDERCAPABILITIES_PROPERTY_PROVIDERMODULENAME);
                capPropList.append(PROVIDERCAPABILITIES_PROPERTY_PROVIDERNAME);
                capPropList.append(PROVIDERCAPABILITIES_PROPERTY_NAMESPACES);
                capPropList.append(PROVIDERCAPABILITIES_PROPERTY_CLASSNAME);
                capabilities = repository->enumerateInstancesForClass(
                    PEGASUS_NAMESPACENAME_INTEROP,
                    PEGASUS_CLASSNAME_PROVIDERCAPABILITIES, false, false);
            }
            Array<CIMInstance> capabilitiesForProvider;
            Array<CIMNamespaceName> namespacesForProvider;
            Array<CIMNameArray> subclassesForNamespace;
            for (Uint32 j = 0, m = capabilities.size(); j < m; ++j)
            {
                CIMInstance & currentInstance = capabilities[j];
                String curModuleName = getRequiredValue<String>(
                    currentInstance, CAPABILITIES_PROPERTY_PROVIDERMODULENAME);
                String curProviderName = getRequiredValue<String>(
                    currentInstance, CAPABILITIES_PROPERTY_PROVIDERNAME);
                if (curModuleName == moduleName &&
                    curProviderName == providerName)
                {
                    CIMName currentClass(getRequiredValue<String>(
                        currentInstance,
                        PROVIDERCAPABILITIES_PROPERTY_CLASSNAME));
                    capabilitiesForProvider.append(currentInstance);
                    StringArray curNamespaces =
                        getRequiredValue<StringArray>(currentInstance,
                            PROVIDERCAPABILITIES_PROPERTY_NAMESPACES);
                    Sint32 z = 0;
                    Sint32 y = curNamespaces.size();

                    // If one of the namespaces is Interop, then continue
                    bool interopNamespaceFound = false;
                    for (; z < y; ++z)
                    {
                        if (CIMNamespaceName(curNamespaces[z]) ==
                            PEGASUS_NAMESPACENAME_INTEROP)
                        {
                            interopNamespaceFound = true;
                            break;
                        }
                    }
                    if (interopNamespaceFound)
                        continue;

                    // See if the current namespaces are already listed
                    for (Sint32 w = 0; w < y; ++w)
                    {
                        Sint32 foundIndex = -1;
                        CIMNamespaceName curNamespace = curNamespaces[w];
                        Uint32 k = 0;
                        Uint32 x = namespacesForProvider.size();
                        for (; k < x; ++k)
                        {
                            if (curNamespace == namespacesForProvider[k])
                            {
                                foundIndex = (Sint32)k;
                                break;
                            }
                        }
                        if (foundIndex == -1)
                        {
                            // Get all the subclasses of
                            // ElementConformsToProfile in the namespace and
                            // cache them.
                            foundIndex = namespacesForProvider.size();
                            Array<CIMName> subClasses =
                                repository->enumerateClassNames(curNamespace,
                                PEGASUS_CLASSNAME_CIM_ELEMENTCONFORMSTOPROFILE,
                                true);
                            subClasses.append(
                                PEGASUS_CLASSNAME_CIM_ELEMENTCONFORMSTOPROFILE
                                );
                            namespacesForProvider.append(curNamespace);
                            subclassesForNamespace.append(subClasses);
                        }

                        // Now search to see if the current class is one of the
                        // subclasses in this namespace, and finally, if it is
                        // add it to the list
                        Array<CIMName> & subClasses =
                            subclassesForNamespace[foundIndex];
                        for (k = 0, x = subClasses.size(); k < x; ++k)
                        {
                            if (subClasses[k] == currentClass)
                            {
                                String dynamicElement = PEGASUS_DYNAMIC +
                                    currentClass.getString();
                                conformingElementsForProfile.append(
                                    dynamicElement);
                                elementNamespacesForProfile.append(
                                    curNamespace);
                            }
                        }
                    }
                }
            }
        }
        else
        {
            //Array<String> elementClasses;
            //currentProfileInstance.getProperty(propIndex).getValue().get(
            //    elementClasses);
            for (Uint32 j = 0, m = elementClasses.size(); j < m; ++j)
            {
                CIMName elementClass(elementClasses[j]);
                Array<String> searchNamespaces =
                    findProviderNamespacesForElement(
                        moduleName, providerName,
                        elementClass,
                        repository,
                        providerCapabilitiesInstances);
                Uint32 k = 0;
                Uint32 x = searchNamespaces.size();
                for (; k < x; ++k)
                {
                    conformingElementsForProfile.append(elementClass);
                    elementNamespacesForProfile.append(searchNamespaces[k]);
                }
            }
        }

        Sint32 foundIndex = -1;
        for (Sint32 j = 0, m = profileIds.size(); j < m; ++j)
        {
            if (profileIds[j] == profileId)
            {
                foundIndex = j;
                break;
            }
        }

        if (foundIndex >= 0)
        {
            // Append the results to already existing entries
            conformingElements[foundIndex].appendArray(
                conformingElementsForProfile);
            elementNamespaces[foundIndex].appendArray(
                elementNamespacesForProfile);
        }
        else
        {
            profileIds.append(profileId);
            conformingElements.append(conformingElementsForProfile);
            elementNamespaces.append(elementNamespacesForProfile);
        }
    }
}

PEGASUS_NAMESPACE_END
// END_OF_FILE
