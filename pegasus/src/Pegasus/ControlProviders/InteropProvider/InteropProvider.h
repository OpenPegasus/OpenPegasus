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


#ifndef InteropProvider_h
#define InteropProvider_h

///////////////////////////////////////////////////////////////////////////////
//  Interop Provider
///////////////////////////////////////////////////////////////////////////////

#include <Pegasus/Common/Config.h>
#include <Pegasus/ControlProviders/InteropProvider/Linkage.h>

#include <Pegasus/Common/String.h>
#include <Pegasus/Common/System.h>
#include <Pegasus/Common/ArrayInternal.h>
#include <Pegasus/Common/CIMType.h>
#include <Pegasus/Common/CIMInstance.h>
#include <Pegasus/Common/CIMObjectPath.h>
#include <Pegasus/Common/InternalException.h>
#include <Pegasus/Common/CIMStatusCode.h>
#include <Pegasus/Common/Tracer.h>
#include <Pegasus/Config/ConfigManager.h>

#include <Pegasus/Repository/CIMRepository.h>
#include <Pegasus/Provider/CIMInstanceProvider.h>
#include <Pegasus/Provider/CIMAssociationProvider.h>
#include <Pegasus/Provider/CIMMethodProvider.h>
#include <Pegasus/General/VersionUtil.h>

#include \
    <Pegasus/Server/ProviderRegistrationManager/ProviderRegistrationManager.h>

PEGASUS_NAMESPACE_BEGIN

/**
 * The InteropProvider services the Interop classes of the DMTF CIM Schema
 * in the root/PG_InterOp namespace (as well as some related cross-namespace
 * associations in other namespaces). Through this implementation, combined
 * with the SLP provider and one or more vendor-supplied SMI providers, the
 * Pegasus WBEM Server is able to provide a fully-functional implementation of
 * the SMI-S Server profile (currently, version 1.1.0).
 *
 * The following is a list of the association and instance classes supported
 * by this provider in the root/PG_InterOp namespace:
 *
 *  PG_CIMXMLCommunicationMechanism (CIM_CIMXMLCommunicationMechanism)
 *  PG_CommMechanismForManager (CIM_CommMechanismForManager)
 *  PG_ComputerSystem (CIM_ComputerSystem)
 *  PG_ElementConformsToProfile (CIM_ElementConformsToProfile)
 *  PG_ElementSoftwareIdentity (CIM_ElementSoftwareIdentity)
 *  PG_HostedAccessPoint (CIM_HostedAccessPoint)
 *  PG_HostedObjectManager (CIM_HostedService)
 *  PG_InstalledSoftwareIdentity (CIM_InstalledSoftwareIdentity)
 *  PG_Namespace (CIM_Namespace)
 *  PG_NamespaceInManager (CIM_NamespaceInManager)
 *  PG_ObjectManager (CIM_ObjectManager)
 *  PG_ReferencedProfile (CIM_ReferencedProfile)
 *  PG_RegisteredProfile (CIM_RegisteredProfile)
 *  PG_RegisteredSubProfile (CIM_RegisteredSubProfile)
 *  PG_SoftwareIdentity (CIM_SoftwareIdentity)
 *  PG_SubProfileRequiredProfile (CIM_SubProfileRequiresProfile)
 *
 */

typedef Array<CIMName> CIMNameArray;
typedef Array<CIMNamespaceName> CIMNamespaceArray;

class PEGASUS_INTEROPPROVIDER_LINKAGE InteropProvider :
        public CIMInstanceProvider,
        public CIMAssociationProvider,
        public CIMMethodProvider
{
public:

    InteropProvider(
        CIMRepository* repository,
        ProviderRegistrationManager* provRegManager);

    virtual ~InteropProvider()
    {
        PEG_METHOD_ENTER(TRC_CONTROLPROVIDER,
            "InteropProvider::~InteropProvider");
        PEG_METHOD_EXIT();
    }

    // Note:  The initialize() and terminate() methods are not called for
    // Control Providers.
    void initialize(CIMOMHandle& handle) { }
    void terminate() { }

    virtual void createInstance(
        const OperationContext & context,
        const CIMObjectPath & instanceReference,
        const CIMInstance& myInstance,
        ObjectPathResponseHandler & handler);

    virtual void deleteInstance(
        const OperationContext & context,
        const CIMObjectPath& instanceName,
        ResponseHandler & handler);

    virtual void getInstance(
        const OperationContext & context,
        const CIMObjectPath& instanceName,
        const Boolean includeQualifiers,
        const Boolean includeClassOrigin,
        const CIMPropertyList& propertyList,
        InstanceResponseHandler & handler);

    void modifyInstance(
        const OperationContext & context,
        const CIMObjectPath & instanceReference,
        const CIMInstance& modifiedIns,
        const Boolean includeQualifiers,
        const CIMPropertyList& propertyList,
        ResponseHandler & handler);

    virtual void enumerateInstances(
        const OperationContext & context,
        const CIMObjectPath & ref,
        const Boolean includeQualifiers,
        const Boolean includeClassOrigin,
        const CIMPropertyList& propertyList,
        InstanceResponseHandler & handler);

    virtual void enumerateInstanceNames(
        const OperationContext & context,
        const CIMObjectPath & classReference,
        ObjectPathResponseHandler & handler);


    // CIMAssociationProvider interface
    virtual void associators(
        const OperationContext & context,
        const CIMObjectPath & objectName,
        const CIMName & associationClass,
        const CIMName & resultClass,
        const String & role,
        const String & resultRole,
        const Boolean includeQualifiers,
        const Boolean includeClassOrigin,
        const CIMPropertyList & propertyList,
        ObjectResponseHandler & handler);

    virtual void associatorNames(
        const OperationContext & context,
        const CIMObjectPath & objectName,
        const CIMName & associationClass,
        const CIMName & resultClass,
        const String & role,
        const String & resultRole,
        ObjectPathResponseHandler & handler);

    virtual void references(
        const OperationContext & context,
        const CIMObjectPath & objectName,
        const CIMName & resultClass,
        const String & role,
        const Boolean includeQualifiers,
        const Boolean includeClassOrigin,
        const CIMPropertyList & propertyList,
        ObjectResponseHandler & handler);

    virtual void referenceNames(
        const OperationContext & context,
        const CIMObjectPath & objectName,
        const CIMName & resultClass,
        const String & role,
        ObjectPathResponseHandler & handler);

    // CIMMethodProvider interface
    virtual void invokeMethod(
        const OperationContext & context,
        const CIMObjectPath & objectReference,
        const CIMName & methodName,
        const Array<CIMParamValue> & inParameters,
        MethodResultResponseHandler & handler);

private:

    void initProvider();

    CIMInstance buildInstanceSkeleton(
        const CIMNamespaceName & nameSpace,
        const CIMName& className,
        Boolean includeQualifiers,
        CIMClass& returnedClass);
    /*
     * CIM communication mechanism instance building starts with this function
     * and completed by buildCIMXMLCommunicationMechanismInstance
     */
    void _buildCommInstSkeleton(
        const Boolean isHttpsEnabled,
        const Array<String> &ips,
        const CIMClass &commMechClass,
        Array<CIMInstance> &instances );

    CIMInstance buildCIMXMLCommunicationMechanismInstance(
        const String& namespaceType,
        const Uint16& accessProtocol,
        const String& IPAddress,
        const CIMClass & commMechClass);

    Array<CIMInstance> enumCIMXMLCommunicationMechanismInstances();

    Array<CIMInstance> enumHostedAccessPointInstances(
        const OperationContext &opContext);

    CIMInstance getObjectManagerInstance();

    CIMInstance getComputerSystemInstance(const OperationContext &opContext);

    CIMInstance getHostedObjectManagerInstance(
        const OperationContext &opContext);

    Array<CIMInstance> enumNamespaceInstances();

    CIMInstance buildNamespaceInstance(const String & nameSpace);

    CIMInstance getNameSpaceInstance(const CIMObjectPath & ref);

    CIMObjectPath createNamespace(const CIMInstance & namespaceInstance);
    void deleteNamespace(const CIMObjectPath & instanceName);

    CIMObjectPath createProviderProfileCapabilityInstance(
        const CIMInstance & profileInstance,
        const OperationContext & context);

    void deleteProviderProfileCapabilityInstance(
        const CIMObjectPath & instanceName,
        const OperationContext & context);

    Array<CIMInstance> enumNamespaceInManagerInstances();

    Array<CIMInstance> enumCommMechanismForManagerInstances();

    void modifyObjectManagerInstance(const OperationContext & context,
        const CIMObjectPath & instanceReference,
        const CIMInstance& modifiedIns,
        const Boolean includeQualifiers,
        const CIMPropertyList& propertyList);

    void extractSoftwareIdentityInfo(
        const CIMInstance & providerInstance,
        String & moduleName,
        String & providerName,
        String & version,
        String & vendor,
        Uint16 & majorVersion,
        Uint16 & minorVersion,
        Uint16 & revision,
        Uint16 & build,
        bool & extendedVersionSupplied,
        String & interfaceType,
        String & elementName,
        String & caption);

    Array<CIMInstance> enumRegisteredProfileInstances();
    Array<CIMInstance> enumRegisteredSubProfileInstances();
    Array<CIMInstance> enumReferencedProfileInstances();
    Array<CIMInstance> getProfilesForVersion(
        Array<CIMInstance>& subprofs,
        Uint16 regOrg,
        Uint32 majorVer,
        Uint32 minorVer,
        Uint32 updateVer);
    Array<CIMInstance> enumElementConformsToProfileInstances(
        const OperationContext & opContext,
        const CIMNamespaceName & opNamespace);
    Array<CIMInstance> enumElementConformsToProfileRPRPInstances(
        const OperationContext & opContext,
        const CIMNamespaceName & opNamespace);
    Array<CIMInstance> enumSubProfileRequiresProfileInstances();
    Array<CIMInstance> enumSoftwareIdentityInstances();
    Array<CIMInstance> enumElementSoftwareIdentityInstances();
    Array<CIMInstance> enumInstalledSoftwareIdentityInstances(
        const OperationContext &opContext);
    Array<CIMInstance> enumDefaultSoftwareIdentityInstances();

    CIMInstance getSoftwareIdentityInstance(
        const CIMObjectPath &ref);

    Array<CIMInstance> enumProviderProfileCapabilityInstances(
        Boolean checkProviders = true,
        Boolean includeQualifiers = false,
        Boolean includeClassOrigin = false,
        const CIMPropertyList &propertyList = CIMPropertyList());

#ifdef PEGASUS_ENABLE_DMTF_INDICATION_PROFILE_SUPPORT
    Array<CIMInstance> enumIndicationServiceInstances(
        const OperationContext &opContext);
    Array<CIMInstance> enumElementCapabilityInstances(
        const OperationContext & opContext);
    Array<CIMInstance> enumHostedIndicationServiceInstances(
        const OperationContext & opContext);
    Array<CIMInstance> enumServiceAffectsElementInstances(
        const OperationContext & opContext);
    CIMInstance buildAssociationInstance(
        const CIMName &className,
        const CIMName &propName1,
        const CIMObjectPath &objPath1,
        const CIMName &propName2,
        const CIMObjectPath &objPath2);
#endif

    CIMInstance buildRegisteredProfile(
        const String & instanceId,
        const String & profileName,
        const String & profileVersion,
        Uint16 profileOrganization,
        const String & otherProfileOrganization,
        const CIMClass & profileClass);

    CIMInstance buildDependencyInstance(
        const String & antecedentId,
        const CIMName & antecedentClass,
        const String & dependentId,
        const CIMName & dependentClass,
        const CIMClass & dependencyClass);

    CIMInstance buildSoftwareIdentity(
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
        const String & caption);

    Array<CIMInstance> getProfileInstances(
        const CIMName & profileType,
        const Array<String> & defaultSniaProfiles);

    Array<CIMInstance> getDMTFProfileInstances(
        const CIMName & profileType);

    // The following are internal equivalents of the operations
    // allowing the operations to call one another internally within
    // the provider.
    Array<CIMInstance> localEnumerateInstances(
        const OperationContext & context,
        const CIMObjectPath & ref,
        const CIMPropertyList& propertyList=CIMPropertyList());

    Array<CIMInstance> localReferences(
        const OperationContext & context,
        const CIMObjectPath & objectName,
        const CIMName & resultClass,
        String & originRole,
        String & targetRole,
        const CIMPropertyList & propertyList=CIMPropertyList(),
        const CIMName & targetClass=CIMName());

    CIMInstance localGetInstance(
        const OperationContext & context,
        const CIMObjectPath & instanceName,
        const CIMPropertyList & propertyList);

    // This function fetches the other side of the reference.
    Array<CIMInstance> getReferencedInstances(
        const Array<CIMInstance> &refs,
        const String &targetRole,
        const OperationContext & context,
        const CIMPropertyList & propertyList);

    void cacheProfileRegistrationInfo();
    void verifyCachedInfo();
    void initializeNamespaces();

    bool validAssocClassForObject(
        const OperationContext & context,
        const CIMName & assocClass, const CIMObjectPath & objectName,
        const CIMNamespaceName & opNamespace,
        String & originProperty, String & targetProperty);

#ifdef PEGASUS_ENABLE_SLP
    void sendUpdateRegMessageToSLPProvider(
        const OperationContext & context);
#endif


    // Repository Instance variable
    CIMOMHandle cimomHandle;
    CIMRepository * repository;
    ProviderRegistrationManager *providerRegistrationManager;
    String objectManagerName;
    String hostName;
    CIMClass profileCapabilitiesClass;
    CIMClass softwareIdentityClass;
    Array<Uint16> providerClassifications;
    Mutex interopMut;
    bool providerInitialized;
    AtomicInt updateProfileCache;

    // Registration info to cache
    Array<String> profileIds;
    Array<CIMNameArray> conformingElements;
    Array<CIMNamespaceArray> elementNamespaces;

    Boolean enableSLP;
    String httpPort;
    String httpsPort;

    // Cached CIM_ObjectManager instance.
    CIMInstance _CIMObjectManagerInst;
};

PEGASUS_NAMESPACE_END

#endif // InteropProvider_h
