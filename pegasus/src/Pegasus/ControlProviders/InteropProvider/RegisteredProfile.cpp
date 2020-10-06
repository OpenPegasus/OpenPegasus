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

#include <Pegasus/Common/CIMNameCast.h>
#include "InteropProvider.h"
#include "InteropProviderUtils.h"
#include "InteropConstants.h"

PEGASUS_USING_STD;
PEGASUS_NAMESPACE_BEGIN

//
// Values and ValueMap qualifier names
//
static const CIMName VALUES_QUALIFIERNAME("Values");
static const CIMName VALUEMAP_QUALIFIERNAME("ValueMap");


// Property names for RegisteredProfile class
#define REGISTEREDPROFILE_PROPERTY_INSTANCEID COMMON_PROPERTY_INSTANCEID
const CIMName REGISTEREDPROFILE_PROPERTY_ADVERTISETYPES(
    "AdvertiseTypes");
const CIMName REGISTEREDPROFILE_PROPERTY_REGISTEREDNAME(
    "RegisteredName");
const CIMName REGISTEREDPROFILE_PROPERTY_REGISTEREDVERSION(
    "RegisteredVersion");
const CIMName REGISTEREDPROFILE_PROPERTY_REGISTEREDORGANIZATION(
    "RegisteredOrganization");
const CIMName REGISTEREDPROFILE_PROPERTY_OTHERREGISTEREDORGANIZATION(
    "OtherRegisteredOrganization");

// Property names for Provider Referenced Profiles
const CIMName REFERENCEDPROFILES_PROPERTY_REGISTEREDPROFILES(
    "RegisteredProfiles");
const CIMName REFERENCEDPROFILES_PROPERTY_DEPENDENTPROFILES(
    "DependentProfiles");
const CIMName REFERENCEDPROFILES_PROPERTY_REGISTEREDPROFILEVERSIONS(
    "RegisteredProfileVersions");
const CIMName REFERENCEDPROFILES_PROPERTY_DEPENDENTPROFILEVERSIONS(
    "DependentProfileVersions");
const CIMName REFERENCEDPROFILES_PROPERTY_OTHERREGISTEREDPROFILES(
    "OtherRegisteredProfiles");
const CIMName REFERENCEDPROFILES_PROPERTY_OTHERDEPENDENTPROFILES(
    "OtherDependentProfiles");
const CIMName
REFERENCEDPROFILES_PROPERTY_OTHERREGISTEREDPROFILEORGANIZATIONS(
    "OtherRegisteredProfileOrganizations");
const CIMName
REFERENCEDPROFILES_PROPERTY_OTHERDEPENDENTPROFILEORGANIZATIONS(
    "OtherDependentProfileOrganizations");

//
// Method that constructs a CIMInstance object representing an instance of the
// PG_RegisteredProfile or PG_RegisteredSubProfile class (depending on the
// profileClass parameter).
//
CIMInstance InteropProvider::buildRegisteredProfile(
    const String & instanceId,
    const String & profileName,
    const String & profileVersion,
    Uint16         profileOrganization,
    const String & otherProfileOrganization,
    const CIMClass & profileClass)
{
    // Form the skeleton instance
    CIMInstance instance = profileClass.buildInstance(
        false, false, CIMPropertyList());

    //
    setPropertyValue(instance, REGISTEREDPROFILE_PROPERTY_INSTANCEID,
        instanceId);
    setPropertyValue(instance, REGISTEREDPROFILE_PROPERTY_REGISTEREDNAME,
        profileName);
    setPropertyValue(instance, REGISTEREDPROFILE_PROPERTY_REGISTEREDVERSION,
        profileVersion);
    setPropertyValue(instance,
        REGISTEREDPROFILE_PROPERTY_REGISTEREDORGANIZATION,
        profileOrganization);
    if (profileOrganization == 1) // Other
    {
        setPropertyValue(instance,
            REGISTEREDPROFILE_PROPERTY_OTHERREGISTEREDORGANIZATION,
            otherProfileOrganization);
    }

    // Determine if SLP is currently enabled in the server. If so, specify
    // SLP as the advertise type.
    Array<Uint16> advertiseTypes;
    if (enableSLP)
    {
        advertiseTypes.append(3); // Advertised via SLP
    }
    else
    {
        advertiseTypes.append(2); // Not advertised
    }
    setPropertyValue(instance,
        REGISTEREDPROFILE_PROPERTY_ADVERTISETYPES,
        advertiseTypes);

    CIMObjectPath objPath = instance.buildPath(profileClass);
    objPath.setHost(hostName);
    objPath.setNameSpace(PEGASUS_NAMESPACENAME_INTEROP);
    instance.setPath(objPath);

    return instance;
}

#ifdef PEGASUS_ENABLE_SLP
void InteropProvider::sendUpdateRegMessageToSLPProvider(
    const OperationContext &context)
{
    PEG_METHOD_ENTER(
        TRC_CONTROLPROVIDER,
        "InteropProvider::sendUpdateRegMessageToSLPProvider()");

    if (!ConfigManager::parseBooleanValue(
            ConfigManager::getInstance()->getCurrentValue("slp")))
    {
        PEG_METHOD_EXIT();
        return;
    }

    String referenceStr("//", 2);
    referenceStr.append(System::getHostName());
    referenceStr.append("/");
    referenceStr.append(PEGASUS_NAMESPACENAME_INTERNAL.getString());
    referenceStr.append(":");
    referenceStr.append(PEGASUS_CLASSNAME_WBEMSLPTEMPLATE.getString());
    CIMObjectPath reference(referenceStr);

    Array<CIMParamValue> inParams;
    Array<CIMParamValue> outParams;

    // Invoke SLP Provider's update method to update registrations.
    try
    {
        cimomHandle.invokeMethod(
            context,
            PEGASUS_NAMESPACENAME_INTERNAL,
            reference,
            CIMNameCast("update"),
            inParams,
            outParams);
    }
    catch(const Exception &e)
    {
        PEG_TRACE((TRC_CONTROLPROVIDER,Tracer::LEVEL1,
            "Exception caught while invoking SLPProvider 'update' method: %s",
            (const char*)e.getMessage().getCString()));
    }
    catch(...)
    {
        PEG_TRACE_CSTRING(
            TRC_CONTROLPROVIDER,
            Tracer::LEVEL1,
            "Unknown error occurred while"
                " invoking SLPProvider 'update' method.");
    }
    PEG_METHOD_EXIT();
}
#endif

CIMObjectPath InteropProvider::createProviderProfileCapabilityInstance(
    const CIMInstance & profileInstance,
    const OperationContext & context)
{
    // get registered profile class
    CIMClass registeredProfileClass = repository->getClass(
        PEGASUS_NAMESPACENAME_INTEROP,
        PEGASUS_CLASSNAME_PG_REGISTEREDPROFILE,
        false,
        true,
        false);

    String profileName;
    Uint16 profileOrganization = 0;
    String profileVersion;
    String profileOrganizationName;
    Array<String> profileNames;
    Array<String> profileVersions;
    Array<Uint16> profileOrganizations;
    Array<String> profileOrganizationNames;
    Array<String> profileProviderModuleNames;
    Array<String> profileProviderNames;

    // We don't use the any information we extract here. If profileInstance
    // is not valid extractProfileInfo() method throws exception. Calling
    // this method here will check for validity of profileInstance.

    extractProfileInfo(
        profileInstance,
        profileCapabilitiesClass,
        registeredProfileClass,
        profileName,
        profileVersion,
        profileOrganization,
        profileOrganizationName,
        profileNames,
        profileVersions,
        profileOrganizations,
        profileOrganizationNames,
        profileProviderModuleNames,
        profileProviderNames,
        false);

    CIMObjectPath path = repository->createInstance(
        PEGASUS_NAMESPACENAME_INTEROP,
        profileInstance);

    updateProfileCache++;

#ifdef PEGASUS_ENABLE_SLP
    sendUpdateRegMessageToSLPProvider(context);
#endif

    return path;
}

void InteropProvider::deleteProviderProfileCapabilityInstance(
    const CIMObjectPath & instanceName,
    const OperationContext & context)
{
    repository->deleteInstance(PEGASUS_NAMESPACENAME_INTEROP, instanceName);
    updateProfileCache++;

#ifdef PEGASUS_ENABLE_SLP
    sendUpdateRegMessageToSLPProvider(context);
#endif
}

Array<CIMInstance> InteropProvider::enumProviderProfileCapabilityInstances(
    Boolean checkProviders,
    Boolean includeQualifiers,
    Boolean includeClassOrigin,
    const CIMPropertyList &propertyList)
{
    PEG_METHOD_ENTER(TRC_CONTROLPROVIDER,
        "InteropProvider::enumProviderProfileCapabilityInstances()");

    Array<CIMInstance> profileInstances =
        repository->enumerateInstancesForClass(
            PEGASUS_NAMESPACENAME_INTEROP,
            PEGASUS_CLASSNAME_PG_PROVIDERPROFILECAPABILITIES,
            includeQualifiers,
            includeClassOrigin,
            propertyList);

    Array<CIMInstance> enabledInstances;

    if (!checkProviders)
    {
        PEG_METHOD_EXIT();
        return profileInstances;
    }

    for (Uint32 i = 0; i < profileInstances.size() ; ++i)
    {

        String moduleName = getRequiredValue<String>(
            profileInstances[i],
            PROVIDER_PROPERTY_PROVIDERMODULENAME);

        String providerName = getRequiredValue<String>(
            profileInstances[i],
            CAPABILITIES_PROPERTY_PROVIDERNAME);

        Boolean moduleOk = false;
        try
        {
            // get operational status.
            Array<Uint16> status =
                providerRegistrationManager->getProviderModuleStatus(
                    moduleName);

            for (Uint32 s = 0, ssize = status.size(); s < ssize; ++s)
            {
                if (status[s] == CIM_MSE_OPSTATUS_VALUE_OK)
                {
                    moduleOk = true;
                    break;
                }
            }
        }
        catch(...)
        {
            PEG_TRACE((
                TRC_CONTROLPROVIDER,
                Tracer::LEVEL2,
                "ProviderModule %s can not be found.",
                (const char*)moduleName.getCString()));
        }

        if (moduleOk)
        {
            CIMKeyBinding pKey(PROVIDER_PROPERTY_NAME, providerName);

            CIMKeyBinding moduleKey(
                PROVIDER_PROPERTY_PROVIDERMODULENAME,
                moduleName);

            Array<CIMKeyBinding> pKeyBindings;
            pKeyBindings.append(moduleKey);
            pKeyBindings.append(pKey);

            CIMObjectPath providerRef(
                String::EMPTY,
                CIMNamespaceName(),
                PEGASUS_CLASSNAME_PROVIDER,
                pKeyBindings);
            Boolean providerFound = false;
            try
            {
                CIMInstance provider = providerRegistrationManager->getInstance(
                    providerRef);
                providerFound = true;
            }
            catch(...)
            {
                PEG_TRACE((
                    TRC_CONTROLPROVIDER,
                    Tracer::LEVEL2,
                    "Provider %s can not be found.",
                    (const char*)providerName.getCString()));
            }
            if (providerFound)
            {
                enabledInstances.append(profileInstances[i]);
            }
        }
    }
    PEG_METHOD_EXIT();

    return enabledInstances;
}

Array<CIMInstance> InteropProvider::getDMTFProfileInstances(
    const CIMName & profileType)
{
    Array<CIMInstance> instances;
    Boolean isReferencedProfileOperation = profileType.equal(
        PEGASUS_CLASSNAME_PG_REFERENCEDPROFILE);

    Array<CIMInstance> profileCapabilities =
        enumProviderProfileCapabilityInstances(true);

    Array<String> instanceIDs;

    CIMClass registeredProfileClass = repository->getClass(
        PEGASUS_NAMESPACENAME_INTEROP,
        PEGASUS_CLASSNAME_PG_REGISTEREDPROFILE,
        false,
        true,
        false);

    CIMClass referencedProfileClass;
    if (isReferencedProfileOperation)
    {
        referencedProfileClass = repository->getClass(
            PEGASUS_NAMESPACENAME_INTEROP,
            PEGASUS_CLASSNAME_PG_REFERENCEDPROFILE,
            false,
            true,
            false);
    }

    // Get only DMTF Profiles.
    for (Uint32 i =0, n = profileCapabilities.size(); i < n; ++i)
    {
        // Extract the useful properties
        String profileName;
        Uint16 profileOrganization = 0;
        String profileVersion;
        String profileOrganizationName;
        CIMInstance & currentCapabilities = profileCapabilities[i];
        Array<String> profileNames;
        Array<String> profileVersions;
        Array<Uint16> profileOrganizations;
        Array<String> profileOrganizationNames;
        Array<String> profileProviderModuleNames;
        Array<String> profileProviderNames;

        String profileId = extractProfileInfo(
            currentCapabilities,
            profileCapabilitiesClass,
            registeredProfileClass,
            profileName,
            profileVersion,
            profileOrganization,
            profileOrganizationName,
            profileNames,
            profileVersions,
            profileOrganizations,
            profileOrganizationNames,
            profileProviderModuleNames,
            profileProviderNames,
            false);

        // Only DMTF profiles.
        if (profileOrganization != DMTF_NUM)
        {
            continue;
        }
        Array<String> tmpInstanceIds;
        for (Uint32 j = 0, m = profileNames.size(); j < m; ++j)
        {
            tmpInstanceIds.append(
                buildProfileInstanceId(
                    profileOrganizationNames[j],
                    profileNames[j],
                    profileVersions[j]));
        }

        if (!isReferencedProfileOperation)
        {
            tmpInstanceIds.append(profileId);
            profileNames.append(profileName);
            profileVersions.append(profileVersion);
            profileOrganizations.append(profileOrganization);
            profileOrganizationNames.append(profileOrganizationName);
        }

        for (Uint32 j = 0, m = tmpInstanceIds.size(); j < m ; ++j)
        {
            Boolean unique = true;
            String tmpId;
            if (isReferencedProfileOperation)
            {
                tmpId = profileId + ":" + tmpInstanceIds[j];
            }
            else
            {
                tmpId = tmpInstanceIds[j];
            }
            for (Uint32 k = 0, x = instanceIDs.size(); k < x; ++k)
            {
                if (instanceIDs[k] == tmpId)
                {
                    unique = false;
                    break;
                }
            }

            if (unique)
            {
                if (isReferencedProfileOperation)
                {
                    instances.append(
                        buildDependencyInstance(
                            tmpInstanceIds[j],
                            PEGASUS_CLASSNAME_PG_REGISTEREDPROFILE,
                            profileId,
                            PEGASUS_CLASSNAME_PG_REGISTEREDPROFILE,
                            referencedProfileClass));
                }
                else
                {
                    String tmpProfileVersion = profileVersion;
                    if (profileVersions.size() >= j)
                    {
                        tmpProfileVersion = profileVersions[j];
                    }
                    instances.append(
                        buildRegisteredProfile(
                            tmpId,
                            profileNames[j],
                            tmpProfileVersion,
                            profileOrganizations[j],
                            profileOrganizationNames[j],
                            registeredProfileClass));
                }
                instanceIDs.append(tmpId);
            }
        }
    }
    if (!isReferencedProfileOperation)
    {
        // Build DMTF Profile Registration instance
        String instanceId = buildProfileInstanceId(
            DMTF_NAME,
            "Profile Registration",
            DMTF_VER_100);
        instances.append(
            buildRegisteredProfile(
            instanceId,
            "Profile Registration",
            DMTF_VER_100, DMTF_NUM,
            String::EMPTY,
            registeredProfileClass));

        // Build  DMTF Indications Profile instance.
#ifdef PEGASUS_ENABLE_DMTF_INDICATION_PROFILE_SUPPORT
        String indProfileId = buildProfileInstanceId(
            DMTF_NAME,
            "Indications",
            DMTF_VER_110);
        instances.append(
            buildRegisteredProfile(
            indProfileId,
            "Indications",
            DMTF_VER_110,
            DMTF_NUM,
            String::EMPTY,
            registeredProfileClass));
#endif
    }
    // Build Refernced profile association between DMTF Profile Registration
    // and DMTF Indications Profile.
#ifdef PEGASUS_ENABLE_DMTF_INDICATION_PROFILE_SUPPORT
    else
    {
        String profileRegId = buildProfileInstanceId(
            DMTF_NAME,
            "Profile Registration",
            DMTF_VER_100);

        String indProfileId = buildProfileInstanceId(
            DMTF_NAME,
            "Indications",
            DMTF_VER_110);

        instances.append(
            buildDependencyInstance(
                profileRegId,
                PEGASUS_CLASSNAME_PG_REGISTEREDPROFILE,
                indProfileId,
                PEGASUS_CLASSNAME_PG_REGISTEREDPROFILE,
                referencedProfileClass));
    }
#endif

    return instances;
}

//
// Generic method for retrieving instances of profile-related classes. This is
// currently used for enumerating the RegisteredProfle, RegisteredSubprofile,
// and SubprofileRequiresProfile classes.
//
Array<CIMInstance> InteropProvider::getProfileInstances(
    const CIMName & profileType, const Array<String> & defaultSniaProfiles)
{
    Array<CIMInstance> instances;
    bool isRequiresProfileOperation = profileType.equal(
        PEGASUS_CLASSNAME_PG_SUBPROFILEREQUIRESPROFILE);
    Array<CIMInstance> profileCapabilities =
        enumProviderProfileCapabilityInstances(true);

    Array<String> instanceIDs;

    CIMClass registeredProfileClass;
    CIMClass subprofileReqProfileClass;
    if (isRequiresProfileOperation)
    {
        registeredProfileClass = repository->getClass(
            PEGASUS_NAMESPACENAME_INTEROP,
            PEGASUS_CLASSNAME_PG_REGISTEREDPROFILE, false, true, false);
        subprofileReqProfileClass = repository->getClass(
            PEGASUS_NAMESPACENAME_INTEROP,
            PEGASUS_CLASSNAME_PG_SUBPROFILEREQUIRESPROFILE, false, true,
            false);
    }
    else
    {
        registeredProfileClass = repository->getClass(
            PEGASUS_NAMESPACENAME_INTEROP, profileType, false, true, false);
    }

    //
    // First build instances based on vendor-created
    // ProviderProfileCapabilities instances.
    //
    Uint32 i = 0;
    Uint32 n = profileCapabilities.size();
    for (; i < n; ++i)
    {
        // Extract the useful properties
        String profileName;
        Uint16 profileOrganization = 0;
        String profileVersion;
        String profileOrganizationName;
        bool getRegisteredProfileInfo = profileType.equal(
            PEGASUS_CLASSNAME_PG_REGISTEREDPROFILE);
        CIMInstance & currentCapabilities = profileCapabilities[i];
        Array<String> profileNames;
        Array<String> profileVersions;
        Array<Uint16> profileOrganizations;
        Array<String> profileOrganizationNames;
        Array<String> profileProviderModuleNames;
        Array<String> profileProviderNames;

        String profileId = extractProfileInfo(
            currentCapabilities,
            profileCapabilitiesClass,
            registeredProfileClass,
            profileName,
            profileVersion,
            profileOrganization,
            profileOrganizationName,
            profileNames,
            profileVersions,
            profileOrganizations,
            profileOrganizationNames,
            profileProviderModuleNames,
            profileProviderNames,
            getRegisteredProfileInfo);

        Array<String> tmpInstanceIds;

        // Skip DMTF profiles.
        if (profileOrganization == DMTF_NUM)
        {
            continue;
        }
        if (getRegisteredProfileInfo)
        {
            tmpInstanceIds.append(profileId);
            profileNames.append(profileName);
            profileVersions.append(profileVersion);
            profileOrganizations.append(profileOrganization);
            profileOrganizationNames.append(profileOrganizationName);
        }
        else
        {
            for (Uint32 j = 0, m = profileNames.size(); j < m; ++j)
            {
                tmpInstanceIds.append(buildProfileInstanceId(
                    profileOrganizationNames[j], profileNames[j],
                    profileVersions[j]));
            }
        }

        for (Uint32 j = 0, m = tmpInstanceIds.size(); j < m; ++j)
        {
            //See if we've already retrieved an equivalent RegisteredSubProfile
            bool unique = true;
            String tmpId;
            if (isRequiresProfileOperation)
              tmpId = profileId + ":" + tmpInstanceIds[j];
            else
              tmpId = tmpInstanceIds[j];
            for (Uint32 k = 0, x = instanceIDs.size(); k < x; ++k)
            {
                if (instanceIDs[k] == tmpId)
                {
                    unique = false;
                    break;
                }
            }

            if (unique)
            {
                if (isRequiresProfileOperation)
                {
                    instances.append(buildDependencyInstance(
                        profileId,
                        PEGASUS_CLASSNAME_PG_REGISTEREDPROFILE,
                        tmpInstanceIds[j],
                        PEGASUS_CLASSNAME_PG_REGISTEREDSUBPROFILE,
                        subprofileReqProfileClass));
                }
                else
                {
                    String subprofileVersion = profileVersion;
                    if (profileVersions.size() >= j)
                    {
                        subprofileVersion = profileVersions[j];
                    }
                    instances.append(buildRegisteredProfile(tmpId,
                        profileNames[j], subprofileVersion,
                        profileOrganizations[j],
                        profileOrganizationNames[j],
                        registeredProfileClass));
                }
                instanceIDs.append(tmpId);
            }
        }
    }

    //
    // Now build instances for the Profiles and/or Subprofiles that Pegasus
    // implements in this provider.
    //
    for (i = 0, n = defaultSniaProfiles.size(); i < n; ++i)
    {
        if (isRequiresProfileOperation)
        {
            static String serverProfileId(buildProfileInstanceId(
                SNIA_NAME, "Server", SNIA_VER_110));
            String subprofileId = buildProfileInstanceId(
                SNIA_NAME, defaultSniaProfiles[i], SNIA_VER_110);
            String compoundId = serverProfileId + ":" + subprofileId;
            bool unique = true;
            for (Uint32 k = 0, x = instanceIDs.size(); k < x; ++k)
            {
                if (instanceIDs[k] == compoundId)
                {
                    unique = false;
                    break;
                }
            }

            if (unique)
            {
                instances.append(buildDependencyInstance(
                    serverProfileId, PEGASUS_CLASSNAME_PG_REGISTEREDPROFILE,
                    subprofileId,
                    PEGASUS_CLASSNAME_PG_REGISTEREDSUBPROFILE,
                    subprofileReqProfileClass));
            }

            //Add instances for SMI-S version 1.2.0
            static String serverProfileId1(buildProfileInstanceId(
                SNIA_NAME,
                "Server",
                SNIA_VER_120));
            subprofileId = buildProfileInstanceId(
                SNIA_NAME,
                defaultSniaProfiles[i],
                SNIA_VER_120);
            compoundId = serverProfileId1 + ":" + subprofileId;
            unique = true;
            for (Uint32 k = 0, x = instanceIDs.size(); k < x; ++k)
            {
                if (instanceIDs[k] == compoundId)
                {
                    unique = false;
                    break;
                }
            }

            if (unique)
            {
                instances.append(buildDependencyInstance(
                    serverProfileId1,
                    PEGASUS_CLASSNAME_PG_REGISTEREDPROFILE,
                    subprofileId,
                    PEGASUS_CLASSNAME_PG_REGISTEREDSUBPROFILE,
                    subprofileReqProfileClass));
            }
        }
        else
        {
            const String & currentProfile = defaultSniaProfiles[i];
            String instanceId;
            bool defaultProfileUnique = true;

            //Add profile registration profile instance.
            if (String::equal(
              defaultSniaProfiles[i],
              String("Profile Registration")))
            {
                instanceId = buildProfileInstanceId(
                    SNIA_NAME,
                    defaultSniaProfiles[i],
                    SNIA_VER_100);
                for (Uint32 j = 0, m = instanceIDs.size(); j < m; ++j)
                {
                    if (instanceIDs[j] == instanceId)
                    {
                        defaultProfileUnique = false;
                        break;
                    }
                }

                if (defaultProfileUnique)
                {
                    instances.append(
                        buildRegisteredProfile(
                            instanceId,
                            currentProfile,
                            SNIA_VER_100, 11 /*"SNIA"*/,
                            String::EMPTY,
                            registeredProfileClass));
                    instanceIDs.append(instanceId);
                }
                continue;
            }

            //Add instances for SMI-S version 1.1.0.
            defaultProfileUnique = true;
            if (String::equal(defaultSniaProfiles[i], String("Server")) ||
                String::equal(defaultSniaProfiles[i], String("Indication")) ||
                String::equal(defaultSniaProfiles[i], String("Software")))
            {
                instanceId = buildProfileInstanceId(
                    SNIA_NAME,
                    defaultSniaProfiles[i],
                    SNIA_VER_110);
                for (Uint32 j = 0, m = instanceIDs.size(); j < m; ++j)
                {
                    if (instanceIDs[j] == instanceId)
                    {
                        defaultProfileUnique = false;
                        break;
                    }
                }

                if (defaultProfileUnique)
                {
                    instances.append(
                        buildRegisteredProfile(
                            instanceId,
                            currentProfile,
                            SNIA_VER_110, 11 /*"SNIA"*/,
                            String::EMPTY,
                            registeredProfileClass));
                    instanceIDs.append(instanceId);
                }
            }

            //Add instances for SMI-S version 1.2.0.
            defaultProfileUnique = true;
            instanceId = buildProfileInstanceId(
                SNIA_NAME,
                defaultSniaProfiles[i],
                SNIA_VER_120);
            defaultProfileUnique = true;
            for (Uint32 j = 0, m = instanceIDs.size(); j < m; ++j)
            {
                if (instanceIDs[j] == instanceId)
                {
                    defaultProfileUnique = false;
                    break;
                }
            }

            if (defaultProfileUnique)
            {
                instances.append(
                    buildRegisteredProfile(
                        instanceId,
                        currentProfile,
                        SNIA_VER_120, 11 /*"SNIA"*/,
                        String::EMPTY,
                        registeredProfileClass));
                instanceIDs.append(instanceId);
            }
        }
    }

    return instances;
}

//
// Retrieve the RegisteredProfile instances, making use of the generic
// getProfileInstances function above.
//
Array<CIMInstance> InteropProvider::enumRegisteredProfileInstances()
{
    static String serverProfileName("Server");
    static String profileRegistrationProfileName("Profile Registration");
    static String SMISVersionProfileName("SMI-S");
    Array<String> defaultSubprofiles;
    defaultSubprofiles.append(profileRegistrationProfileName);
    defaultSubprofiles.append(SMISVersionProfileName);
    defaultSubprofiles.append(serverProfileName);

    // Get all SNIA and other profiles except DMTF.
    Array<CIMInstance> instances = getProfileInstances(
        PEGASUS_CLASSNAME_PG_REGISTEREDPROFILE, defaultSubprofiles);
    // Get DMTF profiles.
    instances.appendArray(getDMTFProfileInstances(
        PEGASUS_CLASSNAME_PG_REGISTEREDPROFILE));

    return instances;
}

const String indicationProfileName("Indication");
const String softwareProfileName("Software");

//
// Retrieve the RegisteredSubProfile instances, making use of the generic
// getProfileInstances function above.
//
Array<CIMInstance> InteropProvider::enumRegisteredSubProfileInstances()
{
    Array<String> defaultSubprofiles;
    defaultSubprofiles.append(indicationProfileName);
    defaultSubprofiles.append(softwareProfileName);
    return getProfileInstances(PEGASUS_CLASSNAME_PG_REGISTEREDSUBPROFILE,
        defaultSubprofiles);
}

//
// Retrieve the SubProfileRequiresProfile instances, making use of the generic
// getProfileInstances function above.
//
Array<CIMInstance> InteropProvider::enumSubProfileRequiresProfileInstances()
{
    Array<String> defaultSubprofiles;
    defaultSubprofiles.append(indicationProfileName);
    defaultSubprofiles.append(softwareProfileName);
    return getProfileInstances(PEGASUS_CLASSNAME_PG_SUBPROFILEREQUIRESPROFILE,
        defaultSubprofiles);
}

Array<CIMInstance> InteropProvider::enumReferencedProfileInstances()
{
    Array<CIMInstance> instances;

    //
    // Retrieve all of the ProviderReferencedProfiles provider registration
    // instances. Those instances contain the lists used to create the
    // ReferencedProfiles associations.
    //
    Array<CIMInstance> referencedProfiles =
        repository->enumerateInstancesForClass(
            PEGASUS_NAMESPACENAME_INTEROP,
            PEGASUS_CLASSNAME_PG_PROVIDERREFERENCEDPROFILES,
            false);

    CIMClass providerRefProfileClass = repository->getClass(
            PEGASUS_NAMESPACENAME_INTEROP,
            PEGASUS_CLASSNAME_PG_PROVIDERREFERENCEDPROFILES,
            false, true, false);
    CIMClass referencedProfileClass = repository->getClass(
            PEGASUS_NAMESPACENAME_INTEROP,
            PEGASUS_CLASSNAME_PG_REFERENCEDPROFILE,
            false, true, false);

    Array<String> instanceIds;
    for (unsigned int i = 0, n = referencedProfiles.size(); i < n; ++i)
    {
        //
        // Retrieve the required properties linking profile instances via
        // this association.
        //
        CIMInstance & currentReferencedProfile = referencedProfiles[i];
        Array<Uint16> registeredProfiles = getRequiredValue<Array<Uint16> >(
            currentReferencedProfile,
            REFERENCEDPROFILES_PROPERTY_REGISTEREDPROFILES);
        Array<Uint16> dependentProfiles = getRequiredValue<Array<Uint16> >(
            currentReferencedProfile,
            REFERENCEDPROFILES_PROPERTY_DEPENDENTPROFILES);
        Array<String> profileVersions = getRequiredValue<Array<String> >(
            currentReferencedProfile,
            REFERENCEDPROFILES_PROPERTY_REGISTEREDPROFILEVERSIONS);
        Array<String> dependentVersions = getRequiredValue<Array<String> >(
            currentReferencedProfile,
            REFERENCEDPROFILES_PROPERTY_DEPENDENTPROFILEVERSIONS);

        Uint32 m = registeredProfiles.size();
        if (m != dependentProfiles.size() || m != profileVersions.size() ||
            m != dependentVersions.size())
        {
            throw CIMOperationFailedException(
                currentReferencedProfile.getPath().toString() +
                " mismatch in num values between corresponding properties");
        }

        //
        // Retrieve the "other" information about profiles. This is used when
        // a provider supports a profile not currently listed in the Pegasus
        // provider registration schema.
        //
        Uint32 otherProfilesIndex = 0;
        Uint32 otherDependentsIndex = 0;
        Uint32 numOtherProfiles = 0;
        Uint32 numOtherDependents = 0;
        Array<String> otherProfiles;
        Array<String> otherDependentProfiles;
        Array<String> otherProfileOrganizations;
        Array<String> otherDependentOrganizations;
        Uint32 index = currentReferencedProfile.findProperty(
            REFERENCEDPROFILES_PROPERTY_OTHERREGISTEREDPROFILES);
        if (index != PEG_NOT_FOUND)
        {
            currentReferencedProfile.getProperty(index).getValue().get(
                otherProfiles);
            numOtherProfiles = otherProfiles.size();
        }

        index = currentReferencedProfile.findProperty(
            REFERENCEDPROFILES_PROPERTY_OTHERDEPENDENTPROFILES);
        if (index != PEG_NOT_FOUND)
        {
            currentReferencedProfile.getProperty(index).getValue().get(
                otherDependentProfiles);
            numOtherDependents = otherDependentProfiles.size();
        }

        index = currentReferencedProfile.findProperty(
            REFERENCEDPROFILES_PROPERTY_OTHERREGISTEREDPROFILEORGANIZATIONS);
        if (index != PEG_NOT_FOUND)
        {
            currentReferencedProfile.getProperty(index).getValue().get(
                otherProfileOrganizations);
        }

        index = currentReferencedProfile.findProperty(
            REFERENCEDPROFILES_PROPERTY_OTHERDEPENDENTPROFILEORGANIZATIONS);
        if (index != PEG_NOT_FOUND)
        {
            currentReferencedProfile.getProperty(index).getValue().get(
                otherDependentOrganizations);
        }

        if (otherDependentOrganizations.size() != numOtherDependents ||
            otherProfileOrganizations.size() != numOtherProfiles)
        {
            throw CIMOperationFailedException(
                currentReferencedProfile.getPath().toString() +
                " mismatch in num values between corresponding properties");
        }

        //
        // Loop through the registered profile and dependent profile
        // information gathered above.
        //
        for (Uint32 j = 0; j < m; ++j)
        {
            Uint16 currentProfile = registeredProfiles[j];
            Uint16 currentDependent = dependentProfiles[j];
            String profileName;
            String dependentName;
            String profileOrgName;
            String dependentOrgName;

            //
            // Get information about the scoping/antecedent profile
            //
            if (currentProfile == 0) // Other
            {
                if (otherProfilesIndex == numOtherProfiles)
                {
                    throw CIMOperationFailedException(
                        currentReferencedProfile.getPath().toString() +
                        " not enough entries in property " +
                        REFERENCEDPROFILES_PROPERTY_OTHERREGISTEREDPROFILES.
                            getString());
                }

                profileName = otherProfiles[otherProfilesIndex];
                profileOrgName =
                    otherProfileOrganizations[otherProfilesIndex++];
            }
            else
            {
                profileName = translateValue(currentProfile,
                    REFERENCEDPROFILES_PROPERTY_REGISTEREDPROFILES,
                    VALUEMAP_QUALIFIERNAME, VALUES_QUALIFIERNAME,
                    providerRefProfileClass);
                Uint32 colonIndex = profileName.find(Char16(':'));
                PEGASUS_ASSERT(colonIndex != PEG_NOT_FOUND);
                profileOrgName = profileName.subString(0, colonIndex);
                profileName = profileName.subString(colonIndex+1);
            }

            //
            // Get information about the referencing/dependent profile
            //
            if (currentDependent == 0) // Other
            {
                if (otherDependentsIndex == numOtherDependents)
                {
                    throw CIMOperationFailedException(
                        currentReferencedProfile.getPath().toString() +
                        " not enough entries in property " +
                        REFERENCEDPROFILES_PROPERTY_OTHERDEPENDENTPROFILES.
                            getString());
                }

                dependentName = otherDependentProfiles[otherDependentsIndex];
                dependentOrgName =
                    otherDependentOrganizations[otherDependentsIndex++];
            }
            else
            {
                dependentName = translateValue(currentDependent,
                    REFERENCEDPROFILES_PROPERTY_DEPENDENTPROFILES,
                    VALUEMAP_QUALIFIERNAME, VALUES_QUALIFIERNAME,
                    providerRefProfileClass);
                Uint32 colonIndex = dependentName.find(Char16(':'));
                PEGASUS_ASSERT(colonIndex != PEG_NOT_FOUND);
                dependentOrgName = dependentName.subString(0, colonIndex);
                dependentName = dependentName.subString(colonIndex+1);
            }

            //
            // Create the instanceID's for the profile and dependent profile
            // and determine if this ReferencedProfile association is unique
            // or if it's already been created.
            //
            String profileId = buildProfileInstanceId(profileOrgName,
                profileName, profileVersions[j]);
            String dependentId = buildProfileInstanceId(dependentOrgName,
                dependentName, dependentVersions[j]);
            String instanceId = profileId + ":" + dependentId;
            bool unique = true;
            for (Uint32 k = 0, x = instanceIds.size(); k < x; ++k)
            {
                if (instanceIds[k] == instanceId)
                {
                    unique = false;
                    break;
                }
            }

            if (unique)
            {
                // This ReferencedProfile association hasn't been created yet.
                // Adding this to the list of instanceIds ensures that a
                // duplicate won't be created later.
                instanceIds.append(instanceId);

                // Now find out whether the profile and dependent profiles are
                // RegisteredProfile or RegisteredSubProfile instances.
                CIMName profileType;
                if (currentProfile >= 1000)
                    profileType = PEGASUS_CLASSNAME_PG_REGISTEREDSUBPROFILE;
                else
                    profileType = PEGASUS_CLASSNAME_PG_REGISTEREDPROFILE;

                CIMName dependentType;
                if (currentDependent >= 1000)
                    dependentType = PEGASUS_CLASSNAME_PG_REGISTEREDSUBPROFILE;
                else
                    dependentType = PEGASUS_CLASSNAME_PG_REGISTEREDPROFILE;

                //
                // Create the actual ReferencedProfile association instance.
                //
                instances.append(buildDependencyInstance(
                    profileId, profileType, dependentId, dependentType,
                    referencedProfileClass));
            }
        }
    }

    // Get DMTF Referenced profiles.
    instances.appendArray(
        getDMTFProfileInstances(
            PEGASUS_CLASSNAME_PG_REFERENCEDPROFILE));

    //Add a referencedprofile association instance between
    // the server profile and the profile registration profile.
    String profileId = buildProfileInstanceId(
        SNIA_NAME,
        "Server",
        SNIA_VER_120);
    String dependentId = buildProfileInstanceId(
        SNIA_NAME,
        "Profile Registration",
        SNIA_VER_100);
    String instanceId = profileId + ":" + dependentId;
    bool unique = true;
    for (Uint32 k = 0, x = instanceIds.size(); k < x; ++k)
    {
        if (instanceIds[k] == instanceId)
        {
            unique = false;
            break;
        }
    }

    if (unique)
    {
        // This ReferencedProfile association hasn't been created yet.
        // Adding this to the list of instanceIds ensures that a
        // duplicate won't be created later.
        instanceIds.append(instanceId);

        //
        // Create the actual ReferencedProfile association instance.
        //
        instances.append(
            buildDependencyInstance(
                profileId,
                PEGASUS_CLASSNAME_PG_REGISTEREDPROFILE,
                dependentId,
                PEGASUS_CLASSNAME_PG_REGISTEREDPROFILE,
                referencedProfileClass));
    }
    return instances;
}

//
// Given an instance of PG_ProviderProfileCapabilities, this method retrieves
// the values necessary for constructing instances of PG_RegisteredProfile,
// PG_RegisteredSubProfile, and all of their associations.
//
String extractProfileInfo(const CIMInstance & profileCapabilities,
                          const CIMClass & capabilitiesClass,
                          const CIMClass & profileClass,
                          String & name,
                          String & version,
                          Uint16 & organization,
                          String & organizationName,
                          Array<String> & subprofileNames,
                          Array<String> & subprofileVersions,
                          Array<Uint16> & subprofileOrganizations,
                          Array<String> & subprofileOrganizationNames,
                          Array<String> & subProfileProviderModuleNames,
                          Array<String> & subProfileProviderNames,
                          bool noSubProfileInfo)
{
    Uint16 registeredProfile = getRequiredValue<Uint16>(profileCapabilities,
        PROFILECAPABILITIES_PROPERTY_REGISTEREDPROFILE);

    // Retrieve information about the RegisteredProfile
    if (registeredProfile == 0) // Other
    {
        name = getRequiredValue<String>(profileCapabilities,
            PROFILECAPABILITIES_PROPERTY_OTHERREGISTEREDPROFILE);
        organizationName = getRequiredValue<String>(profileCapabilities,
            PROFILECAPABILITIES_PROPERTY_OTHERPROFILEORGANIZATION);
    }
    else
    {
        // Retrieve the profile and organization name from the ValueMap
        String mappedProfileName = translateValue(registeredProfile,
            PROFILECAPABILITIES_PROPERTY_REGISTEREDPROFILE,
            VALUEMAP_QUALIFIERNAME, VALUES_QUALIFIERNAME, capabilitiesClass);
        if (mappedProfileName.size() == 0)
        {
            throw CIMOperationFailedException(
                profileCapabilities.getPath().toString() +
                " has invalid property " +
                PROFILECAPABILITIES_PROPERTY_REGISTEREDPROFILE.getString());
        }

        Uint32 colonIndex = mappedProfileName.find(Char16(':'));
        PEGASUS_ASSERT(colonIndex != PEG_NOT_FOUND);
        organizationName = mappedProfileName.subString(0, colonIndex);
        name = mappedProfileName.subString(colonIndex+1);
    }

    version = getRequiredValue<String>(profileCapabilities,
        PROFILECAPABILITIES_PROPERTY_PROFILEVERSION);

    // Translate the organization name into the organization ValueMap value
    // that will be used to create a PG_RegisteredProfile instance for this
    // profile.
    String organizationMapping = translateValue(organizationName,
        REGISTEREDPROFILE_PROPERTY_REGISTEREDORGANIZATION,
        VALUES_QUALIFIERNAME, VALUEMAP_QUALIFIERNAME, profileClass);
    if (organizationMapping.size() == 0)
    {
        organization = 1;
    }
    else
    {
        organization = atoi((const char *)organizationMapping.getCString());
    }


    // Check whether information about the subprofiles associated to the
    // registered profile is requested.
    if (!noSubProfileInfo)
    {
        // Retrieve the ValueMap values for the subprofiles associated with the
        // RegisteredProfile.
        Array<Uint16> registeredSubprofiles =
            getRequiredValue<Array<Uint16> >(profileCapabilities,
                PROFILECAPABILITIES_PROPERTY_REGISTEREDSUBPROFILES);

        // It is possible that a subprofile could contain a different version
        // number than its parent profile, so retrieve the list. If the
        // SubprofileVersions property isn't present, is NULL, or is an empty
        // array, then the version from the parent profile will be used.
        Uint32 subprofileVersionsIndex = profileCapabilities.findProperty(
            PROFILECAPABILITIES_PROPERTY_SUBPROFILEVERSIONS);
        if (subprofileVersionsIndex != PEG_NOT_FOUND)
        {
            CIMValue val = profileCapabilities.getProperty(
                subprofileVersionsIndex).getValue();
            if (!val.isNull())
            {
                val.get(subprofileVersions);
                Uint32 numVersions = subprofileVersions.size();
                if (numVersions != 0 &&
                    numVersions != registeredSubprofiles.size())
                {
                    throw CIMOperationFailedException(
                        profileCapabilities.getPath().toString() +
                        " does not contain enough entries in property " +
                        PROFILECAPABILITIES_PROPERTY_SUBPROFILEVERSIONS
                            .getString());
                }
            }
        }

        // Either none were supplied or the property wasn't supplied, so
        // use the version value from the scoping profile.
        if (subprofileVersions.size() == 0)
        {
            // Add a version string for each registered subprofile
            for (unsigned int i = 0, n = registeredSubprofiles.size();
                i < n; ++i)
            {
                subprofileVersions.append(version);
            }
        }

        // Retrieve any specified "Other" Registered Subprofiles.
        Array<String> otherRegisteredSubprofiles;
        Uint32 otherSubprofileIndex = profileCapabilities.findProperty(
            PROFILECAPABILITIES_PROPERTY_OTHERREGISTEREDSUBPROFILES);
        Uint32 numOtherSubprofiles = 0;
        if (otherSubprofileIndex != PEG_NOT_FOUND)
        {
            profileCapabilities.getProperty(otherSubprofileIndex).getValue().
                get(otherRegisteredSubprofiles);
            numOtherSubprofiles = otherRegisteredSubprofiles.size();
        }

        Array<String> otherSubprofileOrganizations;
        Uint32 otherOrganizationsIndex = profileCapabilities.findProperty(
            PROFILECAPABILITIES_PROPERTY_OTHERSUBPROFILEORGANIZATIONS);
        Uint32 numOrgs = 0;
        if (otherOrganizationsIndex != PEG_NOT_FOUND)
        {
            CIMValue val = profileCapabilities.getProperty(
                otherOrganizationsIndex).getValue();
            if (!val.isNull())
            {
                val.get(otherSubprofileOrganizations);
                numOrgs = otherSubprofileOrganizations.size();
            }
        }

        // There must be corresponding entries in the
        // OtherRegisteredSubprofiles and OtherSubprofileOrganizations
        // properties
        if (numOrgs != numOtherSubprofiles)
        {
            throw CIMOperationFailedException(
                profileCapabilities.getPath().toString() +
                " does not contain enough entries in property " +
                PROFILECAPABILITIES_PROPERTY_OTHERSUBPROFILEORGANIZATIONS
                    .getString());
        }

        // Now loop through all of the retrieved subprofile information and
        // set the output parameters.
        otherSubprofileIndex = 0;
        for (Uint32 k = 0, x = registeredSubprofiles.size(); k < x; ++k)
        {
            Uint16 subprofileMapping = registeredSubprofiles[k];
            String subprofileName;
            String subprofileOrg;
            if (subprofileMapping == 0) // "Other"
            {
                // Retrieve the subprofile name and organization from the
                // arrays containing the "other" information
                if (otherSubprofileIndex == numOtherSubprofiles)
                {
                    throw CIMOperationFailedException(
                        profileCapabilities.getPath().toString() +
                        " does not contain enough entries in property " +
                        PROFILECAPABILITIES_PROPERTY_OTHERREGISTEREDSUBPROFILES
                            .getString());
                }
                subprofileName =
                    otherRegisteredSubprofiles[otherSubprofileIndex];
                subprofileOrg =
                    otherSubprofileOrganizations[otherSubprofileIndex++];
            }
            else
            {
                // Retrieve the subprofile name and organization from the
                // ValueMap value.
                subprofileName = translateValue(
                    subprofileMapping,
                    PROFILECAPABILITIES_PROPERTY_REGISTEREDSUBPROFILES,
                    VALUEMAP_QUALIFIERNAME, VALUES_QUALIFIERNAME,
                    capabilitiesClass);
                if (subprofileName.size() == 0)
                {
                    throw CIMOperationFailedException(
                        profileCapabilities.getPath().toString() +
                        " has invalid property " +
                        PROFILECAPABILITIES_PROPERTY_REGISTEREDSUBPROFILES.
                            getString());
                }

                Uint32 orgIndex = subprofileName.find(Char16(':'));
                if (orgIndex != PEG_NOT_FOUND)
                {
                    subprofileOrg = subprofileName.subString(0, orgIndex);
                    subprofileName = subprofileName.subString(orgIndex+1);
                }
                else
                {
                    subprofileOrg = organizationName;
                }
            }

            subprofileNames.append(subprofileName);
            subprofileOrganizationNames.append(subprofileOrg);

            // Translate the organization name into an integral value
            String orgMapping = translateValue(organizationName,
                REGISTEREDPROFILE_PROPERTY_REGISTEREDORGANIZATION,
                VALUES_QUALIFIERNAME, VALUEMAP_QUALIFIERNAME, profileClass);
            if (organizationMapping.size() == 0)
            {
                subprofileOrganizations.append(Uint16(1)); // "Other"
            }
            else
            {
                subprofileOrganizations.append(
                    atoi((const char *)organizationMapping.getCString()));
            }
        }

        // Get sub profiles ProviderModule names and Provider names if present
        Array<String> providerModuleNames;
        Array<String> providerNames;

        Uint32 providerModuleNamesIndex = profileCapabilities.findProperty(
            PROFILECAPABILITIES_PROPERTY_SUBPROFILEPROVIDERMODULENAMES);
        Uint32 providerNamesIndex = profileCapabilities.findProperty(
            PROFILECAPABILITIES_PROPERTY_SUBPROFILEPROVIDERNAMES);
        Uint32 numProviderModuleNames = 0;
        Uint32 numProviderNames = 0;

        if (providerModuleNamesIndex != PEG_NOT_FOUND)
        {
            CIMValue val = profileCapabilities.getProperty(
                providerModuleNamesIndex).getValue();
            if (!val.isNull())
            {
                val.get(providerModuleNames);
                numProviderModuleNames = providerModuleNames.size();
            }
        }

        if (providerNamesIndex != PEG_NOT_FOUND)
        {
            CIMValue val = profileCapabilities.getProperty(
                providerNamesIndex).getValue();
            if (!val.isNull())
            {
                val.get(providerNames);
                numProviderNames = providerNames.size();
            }
        }
        if (numProviderModuleNames || numProviderNames)
        {
            Uint32 numRegSubProfiles = registeredSubprofiles.size();
            String propName;
            if (numProviderModuleNames != numRegSubProfiles)
            {
                propName =
                    PROFILECAPABILITIES_PROPERTY_SUBPROFILEPROVIDERMODULENAMES.
                        getString();
            }
            else if (numProviderNames != numRegSubProfiles)
            {
                propName = PROFILECAPABILITIES_PROPERTY_SUBPROFILEPROVIDERNAMES.
                    getString();
            }
            if (propName != String::EMPTY)
            {
                MessageLoaderParms mparms(
                    "ControlProviders.InteropProvider.INVALID_PROPERTY_ENTRIES",
                    "$0 has invalid number of entries in property $1.",
                    profileCapabilities.getPath().toString(),
                    propName);
                throw CIMOperationFailedException(mparms);
            }
            subProfileProviderModuleNames = providerModuleNames;
            subProfileProviderNames = providerNames;
        }
    }

    return buildProfileInstanceId(organizationName, name, version);
}

PEGASUS_NAMESPACE_END

// END_OF_FILE
