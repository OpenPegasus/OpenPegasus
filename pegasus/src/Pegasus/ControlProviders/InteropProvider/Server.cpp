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


#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/HostAddress.h>
#include <Pegasus/Common/PegasusVersion.h>

#include <cctype>
#include <iostream>

#include "InteropProvider.h"
#include "InteropProviderUtils.h"
#include "InteropConstants.h"

#include <Pegasus/Common/StatisticalData.h>
#include <Pegasus/General/Guid.h>

PEGASUS_USING_STD;
PEGASUS_NAMESPACE_BEGIN

const String CIMXMLProtocolVersion = "1.0";

// Property names for ObjectManager Class
//#define OM_PROPERTY_NAME COMMON_PROPERTY_NAME
#define OM_PROPERTY_ELEMENTNAME COMMON_PROPERTY_ELEMENTNAME
#define OM_PROPERTY_CREATIONCLASSNAME COMMON_PROPERTY_CREATIONCLASSNAME
//const CIMName OM_PROPERTY_GATHERSTATISTICALDATA("GatherStatisticalData");
const CIMName OM_PROPERTY_DESCRIPTION("Description");
const CIMName OM_PROPERTY_COMMUNICATIONMECHANISM("CommunicationMechanism");
const CIMName OM_PROPERTY_FUNCTIONALPROFILESSUPPORTED(
    "FunctionalProfilesSupported");
const CIMName OM_PROPERTY_FUNCTIONALPROFILEDESCRIPTIONS(
    "FunctionalProfileDescriptions");
const CIMName OM_PROPERTY_AUTHENTICATIONMECHANISMSSUPPORTED(
    "AuthenticationMechanismsSupported");
const CIMName OM_PROPERTY_AUTHENTICATIONMECHANISMDESCRIPTIONS(
    "AuthenticationMechanismDescriptions");
const CIMName OM_PROPERTY_MULTIPLEOPERATIONSSUPPORTED(
    "MultipleOperationsSupported");
const CIMName OM_PROPERTY_VERSION("Version");
const CIMName OM_PROPERTY_OPERATIONALSTATUS("OperationalStatus");
const CIMName OM_PROPERTY_STARTED("Started");

// Property Names for CIMXML CommunicationMechanism
const CIMName CIMXMLCOMMMECH_PROPERTY_CIMVALIDATED("CIMValidated");
const CIMName CIMXMLCOMMMECH_PROPERTY_COMMUNICATIONMECHANISM(
        "CommunicationMechanism");
const CIMName CIMXMLCOMMMECH_PROPERTY_FUNCTIONALPROFILESSUPPORTED(
        "FunctionalProfilesSupported");
const CIMName CIMXMLCOMMMECH_PROPERTY_FUNCTIONALPROFILEDESCRIPTIONS(
        "FunctionalProfileDescriptions");
const CIMName CIMXMLCOMMMECH_PROPERTY_AUTHENTICATIONMECHANISMSSUPPORTED(
        "AuthenticationMechanismsSupported");
const CIMName CIMXMLCOMMMECH_PROPERTY_AUTHENTICATIONMECHANISMDESCRIPTIONS(
        "AuthenticationMechanismDescriptions");
const CIMName CIMXMLCOMMMECH_PROPERTY_MULTIPLEOPERATIONSSUPPORTED(
        "MultipleOperationsSupported");
const CIMName CIMXMLCOMMMECH_PROPERTY_VERSION("Version");
const CIMName
   CIMXMLCOMMMECH_PROPERTY_CIMXMLPROTOVERSION("CIMXMLProtocolVersion");
const CIMName CIMXMLCOMMMECH_PROPERTY_NAMESPACETYPE("namespaceType");
const CIMName CIMXMLCOMMMECH_PROPERTY_NAMESPACEACCESSPROTOCOL(
    "namespaceAccessProtocol");
const CIMName CIMXMLCOMMMECH_PROPERTY_IPADDRESS("IPAddress");
#define CIMXMLCOMMMECH_PROPERTY_ELEMENTNAME OM_PROPERTY_ELEMENTNAME
#define CIMXMLCOMMMECH_PROPERTY_OPERATIONALSTATUS OM_PROPERTY_OPERATIONALSTATUS
#define CIMXMLCOMMMECH_PROPERTY_NAME COMMON_PROPERTY_NAME
#define CIMXMLCOMMMECH_PROPERTY_CREATIONCLASSNAME OM_PROPERTY_CREATIONCLASSNAME
const CIMName CIMXMLCOMMMECH_PROPERTY_ADVERTISETYPES("AdvertiseTypes");

//
// Fills in the CIMOperation functional profiles and corresponding description
// array.  This function is closely linked to compile and configuration
// features in the CIM Server to determine if certain features are
// enabled and/or compiled.  Definitions correspond to the DMTF SLP template
// version 1.0.
// @param Array<Uint16> profiles provides an array for the profiles
// @param Array<String> with the corresponding profile text descriptions
//
void getFunctionalProfiles(
    Array<Uint16> & profiles,
    Array<String> & profileDescriptions)
{
    // Note that zero and 1 are unknown and other. Not used by us
    // 2 - 5 are not optional in Pegasus
    profiles.append(2);
    profileDescriptions.append("Basic Read");

    profiles.append(3);
    profileDescriptions.append("Basic Write");

    profiles.append(4);
    profileDescriptions.append("Schema Manipulation");

    profiles.append(5);
    profileDescriptions.append("Instance Manipulation");

    ConfigManager* configManager = ConfigManager::getInstance();
    if (ConfigManager::parseBooleanValue(
        configManager->getCurrentValue("enableAssociationTraversal")))
    {
        profiles.append(6);
        profileDescriptions.append("Association Traversal");
    }
#ifndef PEGASUS_DISABLE_EXECQUERY
    profiles.append(7);
    profileDescriptions.append("Query Execution");
#endif
    profiles.append(8);
    profileDescriptions.append("Qualifier Declaration");

    if (ConfigManager::parseBooleanValue(
        configManager->getCurrentValue("enableIndicationService")))
    {
        profiles.append(9);
        profileDescriptions.append("Indications");
    }
}

String _getHostID()
{
    String ipAddress;
    int af;

    String hostName(System::getHostName());
    if (!System::getHostIP(hostName, &af, ipAddress))
    {
        // There is IP address for this host.
        // It is not reachable from out side.
        ipAddress = String("localhost");
        return ipAddress;
    }

    // change the dots to dashes
    for (Uint32 i=0; i<ipAddress.size(); i++)
    {
        if (ipAddress[i] == Char16('.') || 
            ipAddress[i] == Char16(':') )
        {
            ipAddress[i] = Char16('-');
        }
    }
    return ipAddress;
}
//
// Build a single instance of the CIMXMLCommunicationMechanism class using the
// parameters provided. Builds the complete instance and sets its object path.
//
CIMInstance InteropProvider::buildCIMXMLCommunicationMechanismInstance(
            const String& namespaceType,
            const Uint16& accessProtocol,
            const String& IPAddress,
            const CIMClass & targetClass)
{
    PEG_METHOD_ENTER(TRC_CONTROLPROVIDER,
            "InteropProvider::buildCIMXMLCommunicationMechanismInstance()");
    CIMInstance instance = targetClass.buildInstance(false, false,
        CIMPropertyList());

    setCommonKeys(instance);

    // CreationClassName property
    setPropertyValue(instance, CIMXMLCOMMMECH_PROPERTY_CREATIONCLASSNAME,
        PEGASUS_CLASSNAME_PG_CIMXMLCOMMUNICATIONMECHANISM.getString());

    // Name Property
    String nameProperty(STRLIT_ARGS("PEGASUSCOMM"));
    nameProperty.append(namespaceType);
    nameProperty.append(Char16('+'));
    nameProperty.append(IPAddress);

    setPropertyValue(instance, CIMXMLCOMMMECH_PROPERTY_NAME,
        nameProperty);

    // CommunicationMechanism Property - Force to 2.
    setPropertyValue(instance, CIMXMLCOMMMECH_PROPERTY_COMMUNICATIONMECHANISM,
        Uint16(2));

    //Functional Profiles Supported Property.
    Array<Uint16> profiles;
    Array<String> profileDescriptions;
    getFunctionalProfiles(profiles, profileDescriptions);

    // Set functional profiles for the instance
    setPropertyValue(instance,
        CIMXMLCOMMMECH_PROPERTY_FUNCTIONALPROFILESSUPPORTED, profiles);

    setPropertyValue(instance,
        CIMXMLCOMMMECH_PROPERTY_FUNCTIONALPROFILEDESCRIPTIONS,
        profileDescriptions);

    // MultipleOperationsSupported Property
    setPropertyValue(instance,
        CIMXMLCOMMMECH_PROPERTY_MULTIPLEOPERATIONSSUPPORTED, false);

    // AuthenticationMechanismsSupported Property
    Array<Uint16> authentications;
    Array<String> authenticationDescriptions;

    //TODO - get from system.
    authentications.append(3);
    authenticationDescriptions.append("Basic");

    setPropertyValue(instance,
        CIMXMLCOMMMECH_PROPERTY_AUTHENTICATIONMECHANISMSSUPPORTED,
        authentications);

    setPropertyValue(instance,
        CIMXMLCOMMMECH_PROPERTY_AUTHENTICATIONMECHANISMDESCRIPTIONS,
        authenticationDescriptions);

    // Version Property
    setPropertyValue(instance, CIMXMLCOMMMECH_PROPERTY_VERSION,
        CIMXMLProtocolVersion);

    //Populate "required" property that's not populated.
    // CIMXMLProtocolVersion Property
    setPropertyValue(instance, CIMXMLCOMMMECH_PROPERTY_CIMXMLPROTOVERSION,
        Uint16(1));

    // NamespaceType Property
    setPropertyValue(instance, CIMXMLCOMMMECH_PROPERTY_NAMESPACETYPE,
        namespaceType);

    // NamespaceAccessProtocol property
    setPropertyValue(instance, CIMXMLCOMMMECH_PROPERTY_NAMESPACEACCESSPROTOCOL,
        accessProtocol);

    // IPAddress property
    setPropertyValue(instance, CIMXMLCOMMMECH_PROPERTY_IPADDRESS,
        IPAddress);

    // ElementName property
    setPropertyValue(instance, CIMXMLCOMMMECH_PROPERTY_ELEMENTNAME,
        String("Pegasus CIMXML Communication Mechanism"));

    // CIMValidated property
    setPropertyValue(instance, CIMXMLCOMMMECH_PROPERTY_CIMVALIDATED,
        Boolean(false));

    // OperationalStatus property
    Array<Uint16> opStatus;
    opStatus.append(2); // "OK"
    setPropertyValue(instance, CIMXMLCOMMMECH_PROPERTY_OPERATIONALSTATUS,
        opStatus);

    // AdvertiseTypes property
    Array<Uint16> advertiseTypes;
    if (enableSLP)
    {
        advertiseTypes.append(3); // Advertised via SLP
    }
    else
    {
        advertiseTypes.append(2); // Not advertised
    }
    setPropertyValue(instance, CIMXMLCOMMMECH_PROPERTY_ADVERTISETYPES,
        advertiseTypes);

    // build the instance path and set into instance
    CIMObjectPath objPath = instance.buildPath(targetClass);
    objPath.setNameSpace(PEGASUS_NAMESPACENAME_INTEROP);
    objPath.setHost(hostName);
    instance.setPath(objPath);

    PEG_METHOD_EXIT();
    return instance;
}

//isHttpsEnabled signifies if the instance to be built is for https
void InteropProvider::_buildCommInstSkeleton(
    const Boolean isHttpsEnabled,
    const Array<String> &ips,
    const CIMClass &commMechClass,
    Array<CIMInstance> &instances )
{
    // Build the CommunicationMechanism instance
    Uint32 namespaceAccessProtocol = 2;
    String namespaceType = "http";
    String port = httpPort;
    if( isHttpsEnabled)
    {
        namespaceType = "https";
        namespaceAccessProtocol = 3;
        port  = httpsPort;
    }
    CIMInstance instance;
    for (Uint32 i = 0; i < ips.size() ; ++i)
    {
        String addr = ips[i];
        HostAddress tmp;
        tmp.setHostAddress(addr);
        if(tmp.getAddressType() == HostAddress::AT_IPV6)
        {
            addr = "[" + addr + "]";
        }
        addr.append(":");
        addr.append(port);

        instance  = buildCIMXMLCommunicationMechanismInstance(
                namespaceType,
                namespaceAccessProtocol,
                addr,
                commMechClass);
        instances.append(instance);
    }
    // If System::getInterfaceAddrs() fails add ip4 addr here.
    if (!ips.size())
    {
        instance  = buildCIMXMLCommunicationMechanismInstance(
                namespaceType,
                namespaceAccessProtocol,
                getHostAddress(hostName, namespaceAccessProtocol, port),
                commMechClass);
        instances.append(instance);
    }
}

//
// Retrieves all of the instances of CIMXMLCommunicationMechanism for the
// CIMOM.
//
Array<CIMInstance> InteropProvider::enumCIMXMLCommunicationMechanismInstances()
{
    PEG_METHOD_ENTER(TRC_CONTROLPROVIDER,
            "InteropProvider::enumCIMXMLCommunicationMechanismInstances");

    ConfigManager* configManager = ConfigManager::getInstance();
    Boolean enableHttpConnection = ConfigManager::parseBooleanValue(
        configManager->getCurrentValue("enableHttpConnection"));
    Boolean enableHttpsConnection = ConfigManager::parseBooleanValue(
        configManager->getCurrentValue("enableHttpsConnection"));
    String listenAdd = configManager->getCurrentValue(
        "listenAddress");

    Array<CIMInstance> instances;

    CIMClass commMechClass = repository->getClass(
        PEGASUS_NAMESPACENAME_INTEROP,
        PEGASUS_CLASSNAME_PG_CIMXMLCOMMUNICATIONMECHANISM, false, true, false);

    Array<String> ips;

    if(!String::equalNoCase(listenAdd, "All"))
    {
        ips = DefaultPropertyOwner::parseAndGetListenAddress(listenAdd);
        //Filter out the loopback addresses without going deeper to TCP Layer
        for(Uint32 i = 0, n = ips.size(); i < n; ++i)
        {
            String add = ips[i];
            if((add.size() >= 3) && 
                ((add[0] == Char16(':') && add[1] == Char16(':') &&
                    add[2] == Char16('1')) ||
                ( add[0] == Char16('1') && add[1] == Char16('2') &&
                    add[2] == Char16('7'))))
            {
                ips.remove(i);
            }
        }

    }
    else
    {
#ifdef PEGASUS_ENABLE_IPV6
    ips = System::getInterfaceAddrs();
#endif
    }
    if (enableHttpConnection)
    {
        // Build the CommunicationMechanism instance for the HTTP protocol
        _buildCommInstSkeleton( false, ips, commMechClass, instances);
    }

    if (enableHttpsConnection)
    {
        // Build the CommunicationMechanism instance for the HTTPS protocol
        _buildCommInstSkeleton( true, ips, commMechClass, instances);
    }

    PEG_METHOD_EXIT();
    return instances;
}

//
// Get the instance of the CIM_ObjectManager class, creating the instance
// eache time the cimserve is re-started.
//
// @param includeQualifiers Boolean
// @param includeClassOrigin Boolean
// @param propertylist CIMPropertyList
//
// @return CIMInstance with a single built instance of the class
//
// @exception repository instances if exception to enumerateInstances
// for this class.
//
CIMInstance InteropProvider::getObjectManagerInstance()
{
    PEG_METHOD_ENTER(TRC_CONTROLPROVIDER,
        "InteropProvider::getObjectManagerInstance()");

    if (_CIMObjectManagerInst.isUninitialized())
    {
        PEG_TRACE_CSTRING(TRC_CONTROLPROVIDER, Tracer::LEVEL4,
             " _CIMObjectManagerInst is to be initialized.");

        CIMClass omClass;
        CIMInstance instance;
        instance = buildInstanceSkeleton(
                       PEGASUS_NAMESPACENAME_INTEROP,
                       PEGASUS_CLASSNAME_PG_OBJECTMANAGER,
                       false,
                       omClass);

        // Set the common key properties
        setCommonKeys(instance);

        setPropertyValue(
            instance,
            OM_PROPERTY_CREATIONCLASSNAME,
            PEGASUS_CLASSNAME_PG_OBJECTMANAGER.getString());
        setPropertyValue(
            instance,
            OM_PROPERTY_NAME,
            String(PEGASUS_INSTANCEID_GLOBAL_PREFIX) + ":" + _getHostID());
        setPropertyValue(
            instance, 
            OM_PROPERTY_ELEMENTNAME, 
            String(PEGASUS_PG_OBJECTMANAGER_ELEMENTNAME));

        Array<Uint16> operationalStatus;
        operationalStatus.append(2);
        setPropertyValue(
            instance,
            OM_PROPERTY_OPERATIONALSTATUS,
            operationalStatus);
        setPropertyValue(
            instance,
            OM_PROPERTY_STARTED,
            CIMValue(Boolean(true)));

        //
        // Description property this object manager instance.
        // If PEGASUS_CIMOM_DESCRIPTION is non-zero length, use it.
        // Otherwise build form the components below, as defined in
        // PegasusVersion.h.
        String description = String(PEGASUS_CIMOM_DESCRIPTION);
        if (description.size() == 0)
        {
            String pegasusProductStatus(PEGASUS_PRODUCT_STATUS);

            description.append(String(PEGASUS_CIMOM_GENERIC_NAME));
            description.append(Char16(' '));
            description.append(String(PEGASUS_PRODUCT_NAME));
            description.append(" Version ");
            description.append(String(PEGASUS_PRODUCT_VERSION));

            if(pegasusProductStatus.size() > 0)
            {
                description.append(Char16(' '));
                description.append(String(pegasusProductStatus));
            }

        }
        setPropertyValue(instance, OM_PROPERTY_DESCRIPTION, description);

        // Property GatherStatisticalData. Initially this is set to false
        // and can then be modified by a modify instance on the instance.
        Boolean gatherStatDataFlag = false;
        setPropertyValue(
            instance,
            OM_PROPERTY_GATHERSTATISTICALDATA,
            gatherStatDataFlag);

        // Set the statistics property into the Statisticaldata class so that
        // it can perform statistics gathering if necessary.
    #ifndef PEGASUS_DISABLE_PERFINST
        StatisticalData* sd = StatisticalData::current();
        sd->setCopyGSD(gatherStatDataFlag);
    #endif

        // build the instance path and set into instance
        CIMObjectPath objPath = instance.buildPath(omClass);
        objPath.setNameSpace(PEGASUS_NAMESPACENAME_INTEROP);
        objPath.setHost(System::getHostName());
        instance.setPath(objPath);

        _CIMObjectManagerInst = instance;
    }

    PEG_METHOD_EXIT();
    return _CIMObjectManagerInst;

}

//
// Modify the existing Object Manager instance.  Only a single property
// modification is allowed, the statistical data setting.  Any other change is
// rejected with an exception.
//
void InteropProvider::modifyObjectManagerInstance(
    const OperationContext & context,
    const CIMObjectPath & instanceReference,
    const CIMInstance& modifiedIns,
    const Boolean includeQualifiers,
    const CIMPropertyList& propertyList)
{
    PEG_METHOD_ENTER(TRC_CONTROLPROVIDER,
        "InteropProvider::modifyObjectManagerInstance()");

    // Modification only allowed when Performance staticistics are active
#ifndef PEGASUS_DISABLE_PERFINST
    Uint32 propListSize = propertyList.size();
    if(propListSize == 0 && !propertyList.isNull())
    {
        PEG_METHOD_EXIT();
        return;
    }

    if(propertyList.size() != 1 ||
        propertyList[0] != OM_PROPERTY_GATHERSTATISTICALDATA)
    {
        PEG_METHOD_EXIT();
        throw CIMNotSupportedException(String("Only modification of ") +
            OM_PROPERTY_GATHERSTATISTICALDATA.getString() + " allowed");
    }

    Boolean statisticsFlag;
    CIMInstance omInstance;

    // We modify only if this property exists.
    // could either use the property from modifiedIns or simply replace
    // value in property from object manager.
    if (modifiedIns.findProperty(OM_PROPERTY_GATHERSTATISTICALDATA) !=
        PEG_NOT_FOUND)
    {
        omInstance = getObjectManagerInstance();
        if(omInstance.isUninitialized())
        {
            PEG_METHOD_EXIT();
            throw CIMObjectNotFoundException(instanceReference.toString());
        }
        statisticsFlag = getPropertyValue(modifiedIns,
            OM_PROPERTY_GATHERSTATISTICALDATA, false);
        // set the changed property into the instance
        setPropertyValue(omInstance, OM_PROPERTY_GATHERSTATISTICALDATA,
            statisticsFlag);
    }
    else
    {
        // if statistics property not in place, simply exit. Nothing to do
        // not considered an error
        PEG_METHOD_EXIT();
        return;
    }
    PEG_TRACE((
        TRC_CONTROLPROVIDER,
        Tracer::LEVEL3,
        "Interop Provider Set Statistics gathering in CIM_ObjectManager: %s",
        (statisticsFlag? "true" : "false")));
    StatisticalData* sd = StatisticalData::current();
    sd->setCopyGSD(statisticsFlag);
    PEG_METHOD_EXIT();
    return;

#else
    PEG_METHOD_EXIT();
    throw CIMNotSupportedException
        (OM_PROPERTY_GATHERSTATISTICALDATA.getString() +
                " modify operation not supported by Interop Provider");
#endif
}


//
// Get an instance of the PG_ComputerSystem class produced by the
// ComputerSystem provider in the root/cimv2 namespace.
//
// @param opContext Operation context.
//
// @return CIMInstance of PG_ComputerSystem class.
//
// @exception ObjectNotFound exception if a ComputerSystem instance cannot
//     be retrieved.
//
CIMInstance InteropProvider::getComputerSystemInstance(
    const OperationContext &opContext)
{
    PEG_METHOD_ENTER(TRC_CONTROLPROVIDER,
        "InteropProvider::getComputerSystemInstance");

    CIMInstance instance;
    AutoMutex mut(interopMut);
    Array<CIMInstance> tmpInstances = cimomHandle.enumerateInstances(
        opContext,
        PEGASUS_NAMESPACENAME_CIMV2,
        PEGASUS_CLASSNAME_PG_COMPUTERSYSTEM, true, false, false, false,
        CIMPropertyList());
    Uint32 numInstances = tmpInstances.size();
    PEGASUS_ASSERT(numInstances <= 1);
    if(numInstances > 0)
    {
        instance = tmpInstances[0];
        CIMObjectPath tmpPath = instance.getPath();
        tmpPath.setHost(hostName);
        tmpPath.setNameSpace(PEGASUS_NAMESPACENAME_INTEROP);
        instance.setPath(tmpPath);
    }

    if(instance.isUninitialized())
    {
        PEG_METHOD_EXIT();
        throw PEGASUS_CIM_EXCEPTION(CIM_ERR_NOT_FOUND,
            "Could not find ComputerSystem instance");
    }

    PEG_METHOD_EXIT();
    return instance;
}

//
// Returns an instance of the HostedObjectManager association linking the
// ObjectManager and ComputerSystem instances managed by this provider.
//
CIMInstance InteropProvider::getHostedObjectManagerInstance(
    const OperationContext &opContext)
{
    PEG_METHOD_ENTER(TRC_CONTROLPROVIDER,
        "InteropProvider::getHostedObjectManagerInstance");

    // Try to get the current object.  If true then it is already created.
    CIMInstance instance;

    CIMObjectPath csPath = getComputerSystemInstance(opContext).getPath();
    CIMObjectPath omPath = getObjectManagerInstance().getPath();
    String csPathString = csPath.toString();
    String omPathString = omPath.toString();

    CIMClass hostedOMClass = repository->getClass(
        PEGASUS_NAMESPACENAME_INTEROP,
        PEGASUS_CLASSNAME_PG_HOSTEDOBJECTMANAGER,
        false, true, false);

    instance = hostedOMClass.buildInstance(false, false, CIMPropertyList());

    setPropertyValue(instance, PROPERTY_ANTECEDENT,
        CIMValue(csPath));
    setPropertyValue(instance, PROPERTY_DEPENDENT,
        CIMValue(omPath));

    instance.setPath(instance.buildPath(hostedOMClass));

    PEG_METHOD_EXIT();
    return instance;
}

//
// Returns an array containing all of the HostedAccessPoint association
// instances for this CIMOM. One will be produced for every instance of
// CIMXMLCommunicatiomMechanism managed by this provider.
//
Array<CIMInstance> InteropProvider::enumHostedAccessPointInstances(
    const OperationContext &opContext)
{
    PEG_METHOD_ENTER(TRC_CONTROLPROVIDER,
        "InteropProvider::enumHostedAccessPointInstance");
    Array<CIMInstance> instances;

    CIMObjectPath csPath = getComputerSystemInstance(opContext).getPath();
    Array<CIMInstance> commMechs = enumCIMXMLCommunicationMechanismInstances();
    CIMClass hapClass = repository->getClass(PEGASUS_NAMESPACENAME_INTEROP,
        PEGASUS_CLASSNAME_PG_HOSTEDACCESSPOINT, false, true, false);
    for(Uint32 i = 0, n = commMechs.size(); i < n; ++i)
    {
        CIMInstance & currentCommMech = commMechs[i];
        CIMInstance hapInstance = hapClass.buildInstance(false, false,
            CIMPropertyList());
        setPropertyValue(hapInstance, PROPERTY_ANTECEDENT, csPath);
        setPropertyValue(hapInstance, PROPERTY_DEPENDENT,
            currentCommMech.getPath());
        hapInstance.setPath(hapInstance.buildPath(hapClass));
        instances.append(hapInstance);
    }

    PEG_METHOD_EXIT();
    return instances;
}


//
// Returns an array containing all of the CommMechanismForManager association
// instances for this CIMOM. One will be produced for every instance of
// CIMXMLCommunicatiomMechanism managed by this provider.
//
Array<CIMInstance> InteropProvider::enumCommMechanismForManagerInstances()
{
    PEG_METHOD_ENTER(TRC_CONTROLPROVIDER,
        "InteropProvider::enumCommMechanismForManagerInstances");

    Array<CIMInstance> commInstances =
        enumCIMXMLCommunicationMechanismInstances();

    CIMInstance instanceObjMgr = getObjectManagerInstance();

    CIMObjectPath refObjMgr = instanceObjMgr.getPath();

    Array<CIMInstance> assocInstances;
    CIMClass targetClass;
    CIMInstance instanceskel = buildInstanceSkeleton(
        PEGASUS_NAMESPACENAME_INTEROP,
        PEGASUS_CLASSNAME_PG_COMMMECHANISMFORMANAGER, true, targetClass);
    for (Uint32 i = 0, n = commInstances.size(); i < n; ++i)
    {
        CIMInstance instance = instanceskel.clone();

        setPropertyValue(instance, PROPERTY_ANTECEDENT, refObjMgr);

        setPropertyValue(instance, PROPERTY_DEPENDENT,
          commInstances[i].getPath());

        instance.setPath(instance.buildPath(targetClass));
        assocInstances.append(instance);
    }

    PEG_METHOD_EXIT();
    return assocInstances;
}

#ifdef PEGASUS_ENABLE_DMTF_INDICATION_PROFILE_SUPPORT

Array<CIMInstance> InteropProvider::enumIndicationServiceInstances(
    const OperationContext &opContext)
{
    Array<CIMInstance> instances = cimomHandle.enumerateInstances(
        opContext,
        PEGASUS_NAMESPACENAME_INTEROP,
        PEGASUS_CLASSNAME_CIM_INDICATIONSERVICE,
        true,
        false,
        true,
        true,
        CIMPropertyList());
    PEGASUS_ASSERT(instances.size() == 1);

    return instances;
}

CIMInstance InteropProvider::buildAssociationInstance(
    const CIMName &className,
    const CIMName &propName1,
    const CIMObjectPath &objPath1,
    const CIMName &propName2,
    const CIMObjectPath &objPath2)
{
    CIMClass cimClass = repository->getClass(
        PEGASUS_NAMESPACENAME_INTEROP,
        className,
        false,
        true,
        true);

    CIMInstance instance = cimClass.buildInstance(
        true,
        true,
        CIMPropertyList());

    instance.getProperty(instance.findProperty(propName1)).setValue(objPath1);
    instance.getProperty(instance.findProperty(propName2)).setValue(objPath2);
    instance.setPath(instance.buildPath(cimClass));

    return instance;
}

Array<CIMInstance> InteropProvider::enumElementCapabilityInstances(
    const OperationContext &opContext)
{
    // Get CIM_IndicationServiceCapabilities instance
    Array<CIMObjectPath> capPaths = cimomHandle.enumerateInstanceNames(
        opContext,
        PEGASUS_NAMESPACENAME_INTEROP,
        PEGASUS_CLASSNAME_CIM_INDICATIONSERVICECAPABILITIES);
    PEGASUS_ASSERT(capPaths.size() == 1);

    // Get CIM_IndicationService instance
    Array<CIMObjectPath> servicePaths = cimomHandle.enumerateInstanceNames(
        opContext,
        PEGASUS_NAMESPACENAME_INTEROP,
        PEGASUS_CLASSNAME_CIM_INDICATIONSERVICE);
    PEGASUS_ASSERT(servicePaths.size() == 1);

    Array<CIMInstance> instances;

    instances.append(
        buildAssociationInstance(
            PEGASUS_CLASSNAME_PG_ELEMENTCAPABILITIES,
            PROPERTY_CAPABILITIES,
            capPaths[0],
            PROPERTY_MANAGEDELEMENT,
            servicePaths[0]));

    return instances;
}

Array<CIMInstance> InteropProvider::enumHostedIndicationServiceInstances(
    const OperationContext &opContext)
{
    Array<CIMInstance> instances;
    CIMInstance cInst = getComputerSystemInstance(opContext);

    // Get CIM_IndicationService instance
    Array<CIMObjectPath> servicePaths = cimomHandle.enumerateInstanceNames(
        opContext,
        PEGASUS_NAMESPACENAME_INTEROP,
        PEGASUS_CLASSNAME_CIM_INDICATIONSERVICE);

    PEGASUS_ASSERT(servicePaths.size() == 1);

    instances.append(
        buildAssociationInstance(
            PEGASUS_CLASSNAME_PG_HOSTEDINDICATIONSERVICE,
            PROPERTY_ANTECEDENT,
            cInst.getPath(),
            PROPERTY_DEPENDENT,
            servicePaths[0]));

    return instances;
}

Array<CIMInstance> InteropProvider::enumServiceAffectsElementInstances(
    const OperationContext &opContext)
{
    Array<CIMInstance> instances;

    // Get CIM_IndicationService instance
    Array<CIMObjectPath> servicePaths = cimomHandle.enumerateInstanceNames(
        opContext,
        PEGASUS_NAMESPACENAME_INTEROP,
        PEGASUS_CLASSNAME_CIM_INDICATIONSERVICE);
    PEGASUS_ASSERT(servicePaths.size() == 1);

    Array<CIMNamespaceName> namespaceNames = repository->enumerateNameSpaces();
    // Get CIM_IndicationFilter and CIM_ListenerDestination instances in all
    // namespaces and associate them with CIM_IndicationService instance using
    // PG_ServiceAffectsElement instance.
    for (Uint32 i = 0, n = namespaceNames.size() ; i < n ; ++i)
    {
        Array<CIMObjectPath> filterPaths;
        try
        {
            // Get CIM_IndicationFilter instance names
            filterPaths = cimomHandle.enumerateInstanceNames(
                opContext,
                namespaceNames[i],
                PEGASUS_CLASSNAME_INDFILTER);
        }
        catch(CIMException &e)
        {
            // Ignore exception with CIM_ERR_INVALID_CLASS code. This will
            // happen when the class CIM_IndicationFilter can not be found
            // in this namespace.
            if (e.getCode() != CIM_ERR_INVALID_CLASS)
            {
                PEG_TRACE((
                    TRC_CONTROLPROVIDER,
                    Tracer::LEVEL2,
                    "CIMException while enumerating the "
                        "CIM_IndicationFilter instances"
                            " in the namespace %s: %s.",
                     (const char*)namespaceNames[i].getString().getCString(),
                     (const char*)e.getMessage().getCString()));
            }
        }
        catch(Exception &e)
        {
            PEG_TRACE((
                TRC_CONTROLPROVIDER,
                Tracer::LEVEL1,
                "Exception while enumerating the "
                    "CIM_IndicationFilter instances"
                        " in the namespace %s: %s.",
                (const char*)namespaceNames[i].getString().getCString(),
                (const char*)e.getMessage().getCString()));
        }
        catch(...)
        {
            PEG_TRACE((
                TRC_CONTROLPROVIDER,
                Tracer::LEVEL1,
                "Unknown error occurred while enumerating the "
                    "CIM_IndicationFilter instances in the namespace %s.",
                (const char*)namespaceNames[i].getString().getCString()));
        }
        for (Uint32 f = 0, fn = filterPaths.size(); f < fn ; ++f)
        {
            filterPaths[f].setNameSpace(namespaceNames[i]);
            instances.append(
                buildAssociationInstance(
                    PEGASUS_CLASSNAME_PG_SERVICEAFFECTSELEMENT,
                    PROPERTY_AFFECTEDELEMENT,
                    filterPaths[f],
                    PROPERTY_AFFECTINGELEMENT,
                    servicePaths[0]));
        }

        Array<CIMObjectPath> handlerPaths;
        try
        {
            // Get CIM_ListenerDestination instance names
            handlerPaths = cimomHandle.enumerateInstanceNames(
                opContext,
                namespaceNames[i],
                PEGASUS_CLASSNAME_LSTNRDST);
        }
        catch(CIMException &e)
        {
            // Ignore exception with CIM_ERR_INVALID_CLASS code. This will
            // happen when the class CIM_ListenerDestination can not be found
            // in this namespace.
            if (e.getCode() != CIM_ERR_INVALID_CLASS)
            {
                PEG_TRACE((
                    TRC_CONTROLPROVIDER,
                    Tracer::LEVEL2,
                    "CIMException while enumerating the "
                        "CIM_ListenerDestination instances"
                            " in the namespace %s: %s.",
                     (const char*)namespaceNames[i].getString().getCString(),
                     (const char*)e.getMessage().getCString()));
            }
        }
        catch(Exception &e)
        {
            PEG_TRACE((
                TRC_CONTROLPROVIDER,
                Tracer::LEVEL1,
                "Exception while enumerating the "
                    "CIM_ListenerDestination instances"
                        " in the namespace %s: %s.",
                (const char*)namespaceNames[i].getString().getCString(),
                (const char*)e.getMessage().getCString()));
        }
        catch(...)
        {
            PEG_TRACE((
                TRC_CONTROLPROVIDER,
                Tracer::LEVEL1,
                "Unknown error occurred while enumerating the "
                    "CIM_ListenerDestination instances in the namespace %s.",
                (const char*)namespaceNames[i].getString().getCString()));
        }
        for (Uint32 h = 0, hn = handlerPaths.size(); h < hn ; ++h)
        {
            handlerPaths[h].setNameSpace(namespaceNames[i]);
            instances.append(
                buildAssociationInstance(
                    PEGASUS_CLASSNAME_PG_SERVICEAFFECTSELEMENT,
                    PROPERTY_AFFECTEDELEMENT,
                    handlerPaths[h],
                    PROPERTY_AFFECTINGELEMENT,
                    servicePaths[0]));
        }
    }
    return instances;
}
#endif

PEGASUS_NAMESPACE_END

// END OF FILE
