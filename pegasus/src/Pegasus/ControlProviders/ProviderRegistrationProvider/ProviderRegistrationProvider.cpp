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
//%/////////////////////////////////////////////////////////////////////////////

#include "ProviderRegistrationProvider.h"

#include <Pegasus/Common/PegasusVersion.h>
#include <Pegasus/Common/XmlWriter.h>
#include <Pegasus/Common/Constants.h>
#include <Pegasus/Common/CIMMessage.h>
#include <Pegasus/Common/OperationContextInternal.h>
#include <Pegasus/Common/System.h>
#include <Pegasus/Common/MessageLoader.h> //l10n
#include <Pegasus/Common/Constants.h>
#include <Pegasus/Common/Tracer.h>
#include <Pegasus/Common/CIMNameCast.h>

#include <Pegasus/Server/ProviderRegistrationManager/ProviderManagerMap.h>

PEGASUS_NAMESPACE_BEGIN

IndicationResponseHandler*
    ProviderRegistrationProvider::_indicationResponseHandler;

/**
   The name of the CapabilityID property for provider capabilities class
*/
static const CIMName _PROPERTY_CAPABILITYID = CIMNameCast("CapabilityID");

/**
   stopping provider method
*/
static const CIMName _STOP_PROVIDER = CIMNameCast("Stop");

/**
   starting provider method
*/
static const CIMName _START_PROVIDER = CIMNameCast("Start");

/**
   setting provider module group method
*/
static const CIMName _SET_MODULEGROUPNAME = CIMNameCast("SetModuleGroupName");

/**
   input param for setModuleGroupName name method
*/
static const CIMName _PARAM_MODULEGROUPNAME = CIMNameCast("ModuleGroupName");

Boolean ProviderRegistrationProvider::_enableIndications = false;
Mutex ProviderRegistrationProvider::_indicationDeliveryMtx;

ProviderRegistrationProvider::ProviderRegistrationProvider(
    ProviderRegistrationManager * providerRegistrationManager)
{
    _providerRegistrationManager = providerRegistrationManager;
    _providerRegistrationManager->setPMInstAlertCallback(_PMInstAlertCallback);

    _controller = ModuleController::getModuleController();
    _sentEnabledIndications = false;
}

ProviderRegistrationProvider::~ProviderRegistrationProvider(void)
{
}

void ProviderRegistrationProvider::_generatePMIndications(
    PMInstAlertCause alertCause)
{
    CIMObjectPath path = 
        CIMObjectPath("", CIMNamespaceName(), PEGASUS_CLASSNAME_PROVIDERMODULE);

    Array<CIMInstance> enumInstances =
        _providerRegistrationManager->enumerateInstancesForClass(
            path,
            false,
            false,
            CIMPropertyList());

    _sendIndication(enumInstances, CIMInstance(), alertCause);
}

void ProviderRegistrationProvider::_PMInstAlertCallback(
    const CIMInstance &providerModule,
    const CIMInstance &provider,
    PMInstAlertCause alertCause)
{
    Array<CIMInstance> providerModules;
    providerModules.append(providerModule);
    _sendIndication(providerModules, provider, alertCause);
}

void ProviderRegistrationProvider::_sendIndication(
    const Array<CIMInstance> &providerModules,
    const CIMInstance &provider,
    PMInstAlertCause alertCause)
{
    AutoMutex mtx(_indicationDeliveryMtx);

    if (!_enableIndications)
    {
        return;
    }
    PEGASUS_ASSERT(_indicationResponseHandler);

    String providerName;

    try
    {
       // Note: The following code determines whether this indiction
       // actually needs to be delivered. Don't deliver the indication
       // if operationalStatus have multiple status values. For example,
       // during the provider disable, operationalStatus is updated
       // with "Ok,Stopping" first and later with "Stopped". providerModules
       // array will have more than one provider module if the indication
       // is generated during the cimserver start or stop.
       if (providerModules.size() == 1)
       {
            Array<Uint16> operationalStatus;
            providerModules[0].getProperty(providerModules[0].findProperty(
                _PROPERTY_OPERATIONALSTATUS)).getValue().get(operationalStatus);

            if (operationalStatus.size() != 1)
            {
                return;
            }
            // Not all alerts will have provider associated with them.
            // Get the provider name if provider is involved.
            if (!provider.isUninitialized())
            {
                provider.getProperty(provider.findProperty(
                    PEGASUS_PROPERTYNAME_NAME)).getValue().get(providerName);
            }
        }

        CIMInstance indication(
            PEGASUS_CLASSNAME_PROVIDERMODULE_INSTALERT);

        Uint16 cause = 0;
        if (alertCause == PM_CREATED || alertCause == PM_DELETED)
        {
            if (providerName.size())
            {
                cause = (alertCause == PM_CREATED) ?
                    PM_PROVIDER_ADDED : PM_PROVIDER_REMOVED;

                indication.addProperty(CIMProperty(
                    "ProviderName",
                    providerName));
            }
            else
            {
                cause = alertCause;
            }
        }
        else
        {
            cause = alertCause;
        }

        indication.addProperty(
            CIMProperty("AlertCause", Uint16(cause)));

        Array<CIMObject> modules;
        for (Uint32 i = 0, n = providerModules.size() ; i < n; ++i)
        {
            modules.append(CIMObject(providerModules[i]));
        }

        indication.addProperty(
            CIMProperty("ProviderModules", modules));

        CIMObjectPath path =  CIMObjectPath(
            String(),
            PEGASUS_NAMESPACENAME_INTEROP,
            PEGASUS_CLASSNAME_PROVIDERMODULE_INSTALERT);
        indication.setPath (path);
        if (cause == PM_DISABLED_CIMSERVER_STOP)
        {
            OperationContext context;
            // Default 20 seconds timeout to deliver the indication.
            context.insert(TimeoutContainer(20 * 1000));
            _indicationResponseHandler->deliver(context, indication);
        }
        else
        {
            _indicationResponseHandler->deliver(indication);
        }
    }
    catch(const Exception &e)
    {
        PEG_TRACE((TRC_CONTROLPROVIDER,Tracer::LEVEL1,
            "Exception in ProviderRegistrationProvider::"
                "PMInstAlertCallback(): %s",
            (const char*)e.getMessage().getCString()));
    }
    catch(...)
    {
        PEG_TRACE_CSTRING(TRC_CONTROLPROVIDER,Tracer::LEVEL1,
            "Unknown exception in ProviderRegistrationProvider::"
                "PMInstAlertCallback()");
    }
}

// get registered provider
void ProviderRegistrationProvider::getInstance(
    const OperationContext & context,
    const CIMObjectPath & instanceReference,
    const Boolean includeQualifiers,
    const Boolean includeClassOrigin,
    const CIMPropertyList & propertyList,
    InstanceResponseHandler & handler)
{

    if(!instanceReference.getNameSpace().equal (PEGASUS_NAMESPACENAME_INTEROP))
    {
        throw PEGASUS_CIM_EXCEPTION(CIM_ERR_NOT_SUPPORTED,
            instanceReference.getNameSpace().getString());
    }

    // ensure the class existing in the specified namespace
    CIMName className = instanceReference.getClassName();

    if(!className.equal (PEGASUS_CLASSNAME_PROVIDER) &&
       !className.equal (PEGASUS_CLASSNAME_PROVIDERCAPABILITIES) &&
       !className.equal (PEGASUS_CLASSNAME_CONSUMERCAPABILITIES) &&
       !className.equal (PEGASUS_CLASSNAME_PROVIDERMODULE))
    {
        throw PEGASUS_CIM_EXCEPTION(CIM_ERR_NOT_SUPPORTED,
            className.getString());
    }

    // begin processing the request
    handler.processing();

    CIMInstance instance;

    instance = _providerRegistrationManager->getInstance(
        instanceReference, includeQualifiers, includeClassOrigin, propertyList);

    handler.deliver(instance);

    // complete processing the request
    handler.complete();
}

// get all registered providers
void ProviderRegistrationProvider::enumerateInstances(
    const OperationContext & context,
    const CIMObjectPath & classReference,
    const Boolean includeQualifiers,
    const Boolean includeClassOrigin,
    const CIMPropertyList & propertyList,
    InstanceResponseHandler & handler)
{
    if(!classReference.getNameSpace().equal (PEGASUS_NAMESPACENAME_INTEROP))
    {
        throw PEGASUS_CIM_EXCEPTION(CIM_ERR_NOT_SUPPORTED,
            classReference.getNameSpace().getString());
    }

    // ensure the class existing in the specified namespace
    CIMName className = classReference.getClassName();

    if(!className.equal (PEGASUS_CLASSNAME_PROVIDER) &&
       !className.equal (PEGASUS_CLASSNAME_PROVIDERCAPABILITIES) &&
       !className.equal (PEGASUS_CLASSNAME_CONSUMERCAPABILITIES) &&
       !className.equal (PEGASUS_CLASSNAME_PROVIDERMODULE))
    {
        throw PEGASUS_CIM_EXCEPTION(CIM_ERR_NOT_SUPPORTED,
            className.getString());
    }

    // begin processing the request
    handler.processing();

    Array<CIMInstance> enumInstances;

    enumInstances =
        _providerRegistrationManager->enumerateInstancesForClass(
            classReference,
            includeQualifiers,
            includeClassOrigin,
            propertyList);

    handler.deliver(enumInstances);

    // complete processing the request
    handler.complete();
}

// get all registered provider names
void ProviderRegistrationProvider::enumerateInstanceNames(
    const OperationContext & context,
    const CIMObjectPath & classReference,
    ObjectPathResponseHandler & handler)
{
    if(!classReference.getNameSpace().equal (PEGASUS_NAMESPACENAME_INTEROP))
    {
        throw PEGASUS_CIM_EXCEPTION(CIM_ERR_NOT_SUPPORTED,
            classReference.getNameSpace().getString());
    }

    // ensure the class existing in the specified namespace
    CIMName className = classReference.getClassName();

    if(!className.equal (PEGASUS_CLASSNAME_PROVIDER) &&
       !className.equal (PEGASUS_CLASSNAME_PROVIDERCAPABILITIES) &&
       !className.equal (PEGASUS_CLASSNAME_CONSUMERCAPABILITIES) &&
       !className.equal (PEGASUS_CLASSNAME_PROVIDERMODULE))
    {
        throw PEGASUS_CIM_EXCEPTION(CIM_ERR_NOT_SUPPORTED,
            className.getString());
    }

    // begin processing the request
    handler.processing();

    Array<CIMObjectPath> enumInstanceNames;

    // get all instance names from repository
    enumInstanceNames =
        _providerRegistrationManager->enumerateInstanceNamesForClass(
            classReference);

    handler.deliver(enumInstanceNames);

    // complete processing the request
    handler.complete();
}

// change properties for the registered provider
// only support to change property of Namespaces, property of
// SupportedProperties, and property of SupportedMethods
void ProviderRegistrationProvider::modifyInstance(
        const OperationContext & context,
        const CIMObjectPath & instanceReference,
        const CIMInstance & instanceObject,
        const Boolean includeQualifiers,
        const CIMPropertyList & propertyList,
        ResponseHandler & handler)
{
    // get userName and only privileged user can execute this operation
    String userName;
    try
    {
        IdentityContainer container = context.get(IdentityContainer::NAME);
        userName = container.getUserName();
    }
    catch (...)
    {
        userName = String::EMPTY;
    }

#ifndef PEGASUS_OS_ZOS
    if ((userName != String::EMPTY) && !System::isPrivilegedUser(userName))
    {
        throw PEGASUS_CIM_EXCEPTION_L(CIM_ERR_ACCESS_DENIED,MessageLoaderParms(
            "ControlProviders.ProviderRegistrationProvider."
                "ProviderRegistrationProvider."
                "SUPERUSER_PRIVILEGE_REQUIRED_MODIFY_REGISTRATION",
            "You must have superuser privilege to modify the"
                " registration."));
    }
#endif

    if(!instanceReference.getNameSpace().equal (PEGASUS_NAMESPACENAME_INTEROP))
    {
        throw PEGASUS_CIM_EXCEPTION(CIM_ERR_NOT_SUPPORTED,
            instanceReference.getNameSpace().getString());
    }

    //
    // only support to modify the instance of PG_ProviderCapabilities
    //
    if (!instanceReference.getClassName().equal
        (PEGASUS_CLASSNAME_PROVIDERCAPABILITIES))
    {
        throw PEGASUS_CIM_EXCEPTION(CIM_ERR_NOT_SUPPORTED,
            instanceReference.getClassName().getString());
    }

    //
    // only can modify the property of Namespaces, property of
    // SupportedProperties, and property of SupportedMethods
    //
    if (propertyList.isNull())
    {
        throw PEGASUS_CIM_EXCEPTION_L(CIM_ERR_NOT_SUPPORTED, MessageLoaderParms(
            "ControlProviders.ProviderRegistrationProvider."
                "ProviderRegistrationProvider."
            "CAN_ONLY_MODIFY_ERR",
            "Only can modify Namespaces, SupportedProperties,"
                "and SupportedMethods."));
    }

    Array<CIMName> propertyArray = propertyList.getPropertyNameArray();
    for (Uint32 i=0; i<propertyArray.size(); i++)
    {
        if (!propertyArray[i].equal (_PROPERTY_NAMESPACES) &&
            !propertyArray[i].equal (_PROPERTY_SUPPORTEDPROPERTIES) &&
            !propertyArray[i].equal (_PROPERTY_SUPPORTEDMETHODS))
        {
            throw PEGASUS_CIM_EXCEPTION(CIM_ERR_NOT_SUPPORTED,
                propertyArray[i].getString());
        }
    }

    // begin processing the request
    handler.processing();

    _providerRegistrationManager->modifyInstance(
        instanceReference, instanceObject, includeQualifiers, propertyArray);

#ifdef PEGASUS_ENABLE_INTEROP_PROVIDER
    _sendUpdateCacheMessagetoInteropProvider(context);
#endif

    // complete processing the request
    handler.complete();
}

// register a provider
void ProviderRegistrationProvider::createInstance(
    const OperationContext & context,
    const CIMObjectPath & instanceReference,
    const CIMInstance & instanceObject,
    ObjectPathResponseHandler & handler)
{
    // get userName and only privileged user can execute this operation
    String userName;
    try
    {
        IdentityContainer container = context.get(IdentityContainer::NAME);
        userName = container.getUserName();
    }
    catch (...)
    {
        userName = String::EMPTY;
    }

#ifndef PEGASUS_OS_ZOS
    if ((userName != String::EMPTY) && !System::isPrivilegedUser(userName))
    {
        throw PEGASUS_CIM_EXCEPTION_L(CIM_ERR_ACCESS_DENIED, MessageLoaderParms(
            "ControlProviders.ProviderRegistrationProvider."
                "ProviderRegistrationProvider."
                "SUPERUSER_PRIVILEGE_REQUIRED_REGISTER_PROVIDERS",
            "You must have superuser privilege to register providers."));
    }
#endif

    CIMName className = instanceReference.getClassName();
    CIMNamespaceName nameSpace = instanceReference.getNameSpace();

    CIMObjectPath returnReference;

    CIMInstance instance = instanceObject;

    if(!nameSpace.equal (PEGASUS_NAMESPACENAME_INTEROP))
    {
        throw PEGASUS_CIM_EXCEPTION(CIM_ERR_NOT_SUPPORTED,
            nameSpace.getString());
    }

    // ensure the class existing in the specified namespace
    if(!className.equal (PEGASUS_CLASSNAME_PROVIDER) &&
       !className.equal (PEGASUS_CLASSNAME_PROVIDERCAPABILITIES) &&
       !className.equal (PEGASUS_CLASSNAME_CONSUMERCAPABILITIES) &&
       !className.equal (PEGASUS_CLASSNAME_PROVIDERMODULE))
    {
        throw PEGASUS_CIM_EXCEPTION(CIM_ERR_NOT_SUPPORTED,
            className.getString());
    }

    //
    // Check all required properties are set
    //
    if (className.equal (PEGASUS_CLASSNAME_PROVIDERMODULE))
    {
        //
        // Name, Version, InterfaceType, InterfaceVersion, and Location
        // properties must be set
        // OperationalStatus property needs to be set. If not, set to default
        //
        if (instanceObject.findProperty(_PROPERTY_PROVIDERMODULE_NAME) ==
            PEG_NOT_FOUND)
        {
            throw PEGASUS_CIM_EXCEPTION_L(CIM_ERR_INVALID_PARAMETER,
                MessageLoaderParms(
                    "ControlProviders.ProviderRegistrationProvider."
                        "ProviderRegistrationProvider."
                        "MISSING_NAME_IN_PG_PROVIDERMODULE",
                    "Missing Name which is required property in "
                        "PG_ProviderModule class."));
        }

        if (instanceObject.findProperty(_PROPERTY_VENDOR) == PEG_NOT_FOUND)
        {
            throw PEGASUS_CIM_EXCEPTION_L(CIM_ERR_INVALID_PARAMETER,
                MessageLoaderParms(
                    "ControlProviders.ProviderRegistrationProvider."
                        "ProviderRegistrationProvider."
                        "MISSING_VENDOR_IN_PG_PROVIDERMODULE",
                    "Missing Vendor which is required property in"
                        " PG_ProviderModule class."));
        }

        Uint32 bitnessIndx = instanceObject.findProperty(
             PEGASUS_PROPERTYNAME_MODULE_BITNESS);

        if (bitnessIndx != PEG_NOT_FOUND)
        {
            CIMValue value = instanceObject.getProperty(bitnessIndx).getValue();
            if (!value.isNull())
            {
                if (value.getType() != CIMTYPE_UINT16)
                {
                    throw PEGASUS_CIM_EXCEPTION(CIM_ERR_TYPE_MISMATCH,
                        PEGASUS_PROPERTYNAME_MODULE_BITNESS.getString());
                }
                Uint16 bitness;
                value.get(bitness);
                if(bitness != PG_PROVMODULE_BITNESS_DEFAULT
#ifdef PEGASUS_PLATFORM_FOR_32BIT_PROVIDER_SUPPORT
                    && bitness != PG_PROVMODULE_BITNESS_32
                    && bitness != PG_PROVMODULE_BITNESS_64
#endif
                    )
                {
                    throw PEGASUS_CIM_EXCEPTION(CIM_ERR_NOT_SUPPORTED,
                        PEGASUS_PROPERTYNAME_MODULE_BITNESS.getString());
                }
            }
        }

        if (instanceObject.findProperty(_PROPERTY_VERSION) == PEG_NOT_FOUND)
        {
            throw PEGASUS_CIM_EXCEPTION_L(CIM_ERR_INVALID_PARAMETER,
                MessageLoaderParms(
                    "ControlProviders.ProviderRegistrationProvider."
                        "ProviderRegistrationProvider."
                        "MISSING_VERSION_IN_PG_PROVIDERMODULE",
                    "Missing Version which is required property in"
                        " PG_ProviderModule class."));
        }

        Uint32 ifcTypeIndex =
            instanceObject.findProperty(_PROPERTY_INTERFACETYPE);
        if (ifcTypeIndex == PEG_NOT_FOUND)
        {
            throw PEGASUS_CIM_EXCEPTION_L(CIM_ERR_INVALID_PARAMETER,
                MessageLoaderParms(
                    "ControlProviders.ProviderRegistrationProvider."
                        "ProviderRegistrationProvider."
                        "MISSING_INTERFACETYPE_IN_PG_PROVIDERMODULE",
                    "Missing InterfaceType which is required property in"
                        " PG_ProviderModule class."));
        }
        String ifcTypeString;
        instanceObject.getProperty(ifcTypeIndex).getValue().
            get(ifcTypeString);

        String ifcEmptyVersion;
        if (!ProviderManagerMap::instance().
                isValidProvMgrIfc(ifcTypeString, ifcEmptyVersion))
        {
            throw PEGASUS_CIM_EXCEPTION_L(CIM_ERR_NOT_SUPPORTED,
                MessageLoaderParms(
                "ControlProviders.ProviderRegistrationProvider."
                    "ProviderRegistrationProvider."
                    "UNSUPPORTED_INTERFACETYPE_VALUE",
                "Unsupported InterfaceType value: \"$0\"",ifcTypeString));
        }

        Uint32 ifcVersionIndex =
            instanceObject.findProperty(_PROPERTY_INTERFACEVERSION);
        if (ifcVersionIndex == PEG_NOT_FOUND)
        {
            throw PEGASUS_CIM_EXCEPTION_L(CIM_ERR_INVALID_PARAMETER,
                MessageLoaderParms(
                    "ControlProviders.ProviderRegistrationProvider."
                        "ProviderRegistrationProvider."
                        "MISSING_INTERFACEVERSION_IN_PG_PROVIDERMODULE",
                    "Missing InterfaceVersion which is required property "
                        "in PG_ProviderModule class."));
        }
        String ifcVersionString;
        instanceObject.getProperty(ifcVersionIndex).getValue().
            get(ifcVersionString);

        if (!ProviderManagerMap::instance().isValidProvMgrIfc(
                    ifcTypeString, ifcVersionString))
        {
            String unsupported = "InterfaceVersion";
            throw PEGASUS_CIM_EXCEPTION_L(CIM_ERR_NOT_SUPPORTED,
                MessageLoaderParms(
                    "ControlProviders.ProviderRegistrationProvider."
                    "ProviderRegistrationProvider."
                    "UNSUPPORTED_INTERFACEVERSION_VALUE",
                    "Unsupported InterfaceVersion value: \"$0\"",
                ifcVersionString));
        }

        if (instanceObject.findProperty(_PROPERTY_LOCATION) == PEG_NOT_FOUND)
        {
            throw PEGASUS_CIM_EXCEPTION_L(CIM_ERR_INVALID_PARAMETER,
                MessageLoaderParms(
                    "ControlProviders.ProviderRegistrationProvider."
                        "ProviderRegistrationProvider."
                        "MISSING_LOCATION_IN_PG_PROVIDERMODULE",
                    "Missing Location which is required property in"
                        " PG_ProviderModule class."));
        }

        if (instanceObject.findProperty(_PROPERTY_OPERATIONALSTATUS) ==
            PEG_NOT_FOUND)
        {
            Array<Uint16> _operationalStatus;
            _operationalStatus.append(CIM_MSE_OPSTATUS_VALUE_OK);
            instance.addProperty (CIMProperty
                (_PROPERTY_OPERATIONALSTATUS, _operationalStatus));
        }

        //
        // Validate the UserContext property
        //
        CIMValue userContextValue;
        Uint32 userContextIndex = instanceObject.findProperty(
            PEGASUS_PROPERTYNAME_MODULE_USERCONTEXT);

        if (userContextIndex != PEG_NOT_FOUND)
        {
            userContextValue =
                instanceObject.getProperty(userContextIndex).getValue();
        }

        if (!userContextValue.isNull())
        {
            Uint16 userContext;
            userContextValue.get(userContext);

#ifdef PEGASUS_DISABLE_PROV_USERCTXT
            if (userContext != PG_PROVMODULE_USERCTXT_CIMSERVER)
            {
                throw PEGASUS_CIM_EXCEPTION_L(CIM_ERR_NOT_SUPPORTED,
                    MessageLoaderParms(
                        "ControlProviders.ProviderRegistrationProvider."
                            "ProviderRegistrationProvider."
                            "USERCONTEXT_UNSUPPORTED",
                        "The UserContext property in the PG_ProviderModule "
                            "class is not supported."));
            }
#else
            if (!((userContext == PG_PROVMODULE_USERCTXT_REQUESTOR) ||
#ifndef PEGASUS_OS_ZOS
                  (userContext == PG_PROVMODULE_USERCTXT_CIMSERVER) ||
                  (userContext == PG_PROVMODULE_USERCTXT_PRIVILEGED) ||
#endif
                  (userContext == PG_PROVMODULE_USERCTXT_DESIGNATED)))
            {
                throw PEGASUS_CIM_EXCEPTION_L(CIM_ERR_NOT_SUPPORTED,
                    MessageLoaderParms(
                        "ControlProviders.ProviderRegistrationProvider."
                            "ProviderRegistrationProvider."
                            "UNSUPPORTED_USERCONTEXT_VALUE",
                        "Unsupported UserContext value: \"$0\".",
                        userContext));
            }

            // DesignatedUserContext property is required when UserContext == 3
            if (userContext == PG_PROVMODULE_USERCTXT_DESIGNATED)
            {
                Uint32 designatedUserIndex = instanceObject.findProperty(
                    PEGASUS_PROPERTYNAME_MODULE_DESIGNATEDUSER);
                if ((designatedUserIndex == PEG_NOT_FOUND) ||
                    instanceObject.getProperty(designatedUserIndex).getValue()
                        .isNull())
                {
                    throw PEGASUS_CIM_EXCEPTION_L(CIM_ERR_INVALID_PARAMETER,
                        MessageLoaderParms(
                            "ControlProviders.ProviderRegistrationProvider."
                                "ProviderRegistrationProvider."
                                "MISSING_DESIGNATEDUSER_IN_PG_PROVIDERMODULE",
                            "Missing DesignatedUserContext property in "
                                "PG_ProviderModule instance."));
                }
                else
                {
                    // Validate that DesignatedUserContext is of String type
                    String designatedUser;
                    instanceObject.getProperty(designatedUserIndex).getValue()
                        .get(designatedUser);
                }
            }
#endif
        }
    }
    else if (className.equal (PEGASUS_CLASSNAME_PROVIDERCAPABILITIES))
    {
        //
        // ProviderModuleName, ProviderName, InstanceID, ClassName,
        // Namespaces, and ProviderType properties must be set
        //

        if (instanceObject.findProperty(_PROPERTY_PROVIDERMODULENAME) ==
            PEG_NOT_FOUND)
        {
            throw PEGASUS_CIM_EXCEPTION_L(CIM_ERR_INVALID_PARAMETER,
                MessageLoaderParms(
                    "ControlProviders.ProviderRegistrationProvider."
                        "ProviderRegistrationProvider."
                        "MISSING_PROVIDERMODULENAME_IN_PG_PROVIDERCAPABILITIES",
                    "Missing ProviderModuleName which is required property in"
                        " PG_ProviderCapabilities class."));
        }

        if (instanceObject.findProperty(_PROPERTY_PROVIDERNAME) ==
            PEG_NOT_FOUND)
        {
            String missing = "ProviderName";
            throw PEGASUS_CIM_EXCEPTION_L(CIM_ERR_INVALID_PARAMETER,
                MessageLoaderParms(
                    "ControlProviders.ProviderRegistrationProvider."
                        "ProviderRegistrationProvider."
                        "MISSING_PROVIDERNAME_IN_PG_PROVIDERCAPABILITIES",
                    "Missing the required ProviderName property in "
                        "PG_ProviderCapabilities class."));
        }

        if (instanceObject.findProperty(_PROPERTY_CAPABILITYID) ==
            PEG_NOT_FOUND)
        {
            throw PEGASUS_CIM_EXCEPTION_L(CIM_ERR_INVALID_PARAMETER,
                MessageLoaderParms(
                    "ControlProviders.ProviderRegistrationProvider."
                        "ProviderRegistrationProvider."
                        "MISSING_CAPABILITYID_IN_PG_PROVIDERCAPABILITIES",
                    "Missing the required CapabilityID property in "
                        "PG_ProviderCapabilities class."));
        }

        if (instanceObject.findProperty(_PROPERTY_CLASSNAME) == PEG_NOT_FOUND)
        {
            throw PEGASUS_CIM_EXCEPTION_L(CIM_ERR_INVALID_PARAMETER,
                MessageLoaderParms(
                    "ControlProviders.ProviderRegistrationProvider."
                        "ProviderRegistrationProvider."
                        "MISSING_CLASSNAME_IN_PG_PROVIDERCAPABILITIES",
                    "Missing ClassName which is required property in"
                        " PG_ProviderCapabilities class."));
        }

        // Validate the Namespaces property

        Uint32 namespacesIndex =
            instanceObject.findProperty(_PROPERTY_NAMESPACES);
        Array<String> namespacesArray;
        if (namespacesIndex != PEG_NOT_FOUND)
        {
            CIMValue namespacesValue =
                instanceObject.getProperty(namespacesIndex).getValue();
            if (!namespacesValue.isNull())
            {
                namespacesValue.get(namespacesArray);
            }
        }

        if (namespacesArray.size() == 0)
        {
            throw PEGASUS_CIM_EXCEPTION_L(CIM_ERR_INVALID_PARAMETER,
                MessageLoaderParms(
                    "ControlProviders.ProviderRegistrationProvider."
                        "ProviderRegistrationProvider."
                        "MISSING_NAMESPACES_IN_PG_PROVIDERCAPABILITIES",
                    "Missing Namespaces which is required property in "
                        "PG_ProviderCapabilities class."));
        }

        // Validate the ProviderType property

        Uint32 providerTypeIndex =
            instanceObject.findProperty(_PROPERTY_PROVIDERTYPE);
        Array<Uint16> providerTypeArray;
        if (providerTypeIndex != PEG_NOT_FOUND)
        {
            CIMValue providerTypeValue =
                instanceObject.getProperty(providerTypeIndex).getValue();
            if (!providerTypeValue.isNull())
            {
                providerTypeValue.get(providerTypeArray);
            }
        }

        if (providerTypeArray.size() == 0)
        {
            throw PEGASUS_CIM_EXCEPTION_L(CIM_ERR_INVALID_PARAMETER,
                MessageLoaderParms(
                    "ControlProviders.ProviderRegistrationProvider."
                        "ProviderRegistrationProvider."
                        "MISSING_PROVIDERTYPE_IN_PG_PROVIDERCAPABILITIES",
                    "Missing ProviderType which is required property in "
                        "PG_ProviderCapabilities class."));
        }

        for (Uint32 i = 0; i < providerTypeArray.size(); i++)
        {
            if ((providerTypeArray[i] < 2) ||
#ifdef PEGASUS_DISABLE_EXECQUERY
                (providerTypeArray[i] > 6)
#else
                (providerTypeArray[i] > 7)
#endif
                                          )
            {
                throw PEGASUS_CIM_EXCEPTION_L(CIM_ERR_NOT_SUPPORTED,
                    MessageLoaderParms(
                        "ControlProviders.ProviderRegistrationProvider."
                            "ProviderRegistrationProvider.UNSUPPORTED_"
                            "PROVIDERTYPE_IN_PG_PROVIDERCAPABILITIES",
                        "Unsupported ProviderType value \"$0\" in "
                            "PG_ProviderCapabilities instance.",
                        providerTypeArray[i]));
            }
        }
    }
    else if (className.equal (PEGASUS_CLASSNAME_CONSUMERCAPABILITIES))
    {
        //
        // ProviderModuleName, ProviderName, CapabilityID, ProviderType,
        // and Destinations properties must be set
        //

        if (instanceObject.findProperty(_PROPERTY_PROVIDERMODULENAME) ==
            PEG_NOT_FOUND)
        {
            MessageLoaderParms parms(
                "ControlProviders.ProviderRegistrationProvider."
                    "ProviderRegistrationProvider."
                    "MISSING_PROVIDER_MODULE_NAME_WHICH_IS_REQUIRED",
                "Missing ProviderModuleName which is required property in"
                    " PG_ConsumerCapabilities class.");

            throw PEGASUS_CIM_EXCEPTION_L(CIM_ERR_INVALID_PARAMETER, parms);
        }

        if (instanceObject.findProperty(_PROPERTY_PROVIDERNAME) ==
            PEG_NOT_FOUND)
        {

            MessageLoaderParms parms(
                "ControlProviders.ProviderRegistrationProvider."
                    "ProviderRegistrationProvider."
                    "MISSING_PROVIDER_NAME_WHICH_IS_REQUIRED",
                "Missing ProviderName which is required property in"
                    " PG_ConsumerCapabilities class.");

            throw PEGASUS_CIM_EXCEPTION_L(CIM_ERR_INVALID_PARAMETER, parms);
        }

        if (instanceObject.findProperty(_PROPERTY_CAPABILITYID) ==
            PEG_NOT_FOUND)
        {

            MessageLoaderParms parms(
                "ControlProviders.ProviderRegistrationProvider."
                    "ProviderRegistrationProvider."
                    "MISSING_CAPABILITY_ID_WHICH_IS_REQUIRED",
                "Missing the required CapabilityID property in "
                    "PG_ConsumerCapabilities class.");

            throw PEGASUS_CIM_EXCEPTION_L(CIM_ERR_INVALID_PARAMETER, parms);
        }

        if (instanceObject.findProperty
            (_PROPERTY_PROVIDERTYPE) == PEG_NOT_FOUND)
        {

            MessageLoaderParms parms(
                "ControlProviders.ProviderRegistrationProvider."
                    "ProviderRegistrationProvider."
                    "MISSING_PROVIDER_TYPE_WHICH_IS_REQUIRED",
                "Missing ProviderType which is required property in"
                    " PG_ConsumerCapabilities class.");

            throw PEGASUS_CIM_EXCEPTION_L(CIM_ERR_INVALID_PARAMETER, parms);
        }

        if (instanceObject.findProperty(_PROPERTY_INDICATIONDESTINATIONS) ==
            PEG_NOT_FOUND)
        {

            MessageLoaderParms parms(
                "ControlProviders.ProviderRegistrationProvider."
                    "ProviderRegistrationProvider."
                    "MISSING_DESTINATIONS_TYPE_WHICH_IS_REQUIRED",
                "Missing Destinations which is required property in"
                    " PG_ConsumerCapabilities class.");

            throw PEGASUS_CIM_EXCEPTION_L(CIM_ERR_INVALID_PARAMETER, parms);
        }
    }
    else // PEGASUS_CLASSNAME_PROVIDER
    {
        //
        // Name and ProviderModuleName properties must be set
        //
        if (instanceObject.findProperty
                (PEGASUS_PROPERTYNAME_NAME) == PEG_NOT_FOUND)
        {
            throw PEGASUS_CIM_EXCEPTION_L(CIM_ERR_INVALID_PARAMETER,
                MessageLoaderParms(
                    "ControlProviders.ProviderRegistrationProvider."
                        "ProviderRegistrationProvider."
                        "MISSING_NAME_IN_PG_PROVIDER",
                    "Missing the required Name property in PG_Provider "
                        "class."));
        }

        if (instanceObject.findProperty(_PROPERTY_PROVIDERMODULENAME) ==
            PEG_NOT_FOUND)
        {
            throw PEGASUS_CIM_EXCEPTION_L(CIM_ERR_INVALID_PARAMETER,
                MessageLoaderParms(
                    "ControlProviders.ProviderRegistrationProvider."
                        "ProviderRegistrationProvider."
                        "MISSING_PROVIDERMODULENAME_IN_PG_PROVIDER",
                    "Missing ProviderModuleName which is required property in "
                        "PG_Provider class."));
        }
    }

    // begin processing the request
    handler.processing();

    returnReference = _providerRegistrationManager->createInstance(
        instanceReference, instance);

#ifdef PEGASUS_ENABLE_INTEROP_PROVIDER
    if(className.equal (PEGASUS_CLASSNAME_PROVIDER) ||
       className.equal (PEGASUS_CLASSNAME_PROVIDERCAPABILITIES))
    {
        _sendUpdateCacheMessagetoInteropProvider(context);
    }
#endif

    handler.deliver(returnReference);

    // complete processing request
    handler.complete();
}

// Unregister a provider
void ProviderRegistrationProvider::deleteInstance(
    const OperationContext & context,
    const CIMObjectPath & instanceReference,
    ResponseHandler & handler)
{
    // get userName and only privileged user can execute this operation
    String userName;
    try
    {
        IdentityContainer container = context.get(IdentityContainer::NAME);
        userName = container.getUserName();
    }
    catch (...)
    {
        userName = String::EMPTY;
    }

#ifndef PEGASUS_OS_ZOS
    if ((userName != String::EMPTY) && !System::isPrivilegedUser(userName))
    {
        throw PEGASUS_CIM_EXCEPTION_L(CIM_ERR_ACCESS_DENIED,MessageLoaderParms(
            "ControlProviders.ProviderRegistrationProvider."
                "ProviderRegistrationProvider."
                "SUPERUSER_PRIVILEGE_REQUIRED_UNREGISTER_PROVIDERS",
            "You must have superuser privilege to unregister providers."));
    }
#endif

    if(!instanceReference.getNameSpace().equal (PEGASUS_NAMESPACENAME_INTEROP))
    {
        throw PEGASUS_CIM_EXCEPTION(CIM_ERR_NOT_SUPPORTED,
            instanceReference.getNameSpace().getString());
    }


    AcceptLanguageList al;
    try
    {
        AcceptLanguageListContainer al_container =
                (AcceptLanguageListContainer)context.get
                (AcceptLanguageListContainer::NAME);
        al = al_container.getLanguages();
    }
    catch (...)
    {
        ;   // Leave AcceptLanguageList empty
    }

    CIMName className = instanceReference.getClassName();

    // ensure the class existing in the specified namespace
    if(!className.equal (PEGASUS_CLASSNAME_PROVIDER) &&
       !className.equal (PEGASUS_CLASSNAME_PROVIDERCAPABILITIES) &&
       !className.equal (PEGASUS_CLASSNAME_CONSUMERCAPABILITIES) &&
       !className.equal (PEGASUS_CLASSNAME_PROVIDERMODULE))
    {
        throw PEGASUS_CIM_EXCEPTION(CIM_ERR_NOT_SUPPORTED,
            className.getString());
    }

    // begin processing the request
    handler.processing();

    String moduleName;
    Boolean moduleFound = false;
    Array<CIMKeyBinding> keys = instanceReference.getKeyBindings();

    //
    // disable provider before delete provider
    // registration if the class is PG_Provider
    //
    if (className.equal (PEGASUS_CLASSNAME_PROVIDER))
    {
        // get module name from reference

        for(Uint32 i=0; i<keys.size() ; i++)
        {
            if(keys[i].getName().equal (_PROPERTY_PROVIDERMODULENAME))
            {
                moduleName = keys[i].getValue();
                moduleFound = true;
            }
        }

        // if _PROPERTY_PROVIDERMODULENAME key not found
        if( !moduleFound)
        {
            throw PEGASUS_CIM_EXCEPTION_L(CIM_ERR_INVALID_PARAMETER,
                    MessageLoaderParms(
                "ControlProviders.ProviderRegistrationProvider."
                    "ProviderRegistrationProvider."
                    "PROVIDERMODULENAME_KEY_NOT_FOUND",
                "key ProviderModuleName was not found"));
        }

        //
        // disable the provider
        //
        Sint16 ret_value = _disableModule(
            instanceReference, moduleName, true, al);

        //
        // if the provider disable failed
        //
        if (ret_value == -1)
        {
            throw PEGASUS_CIM_EXCEPTION_L(CIM_ERR_FAILED,
                MessageLoaderParms(
                    "ControlProviders.ProviderRegistrationProvider."
                        "ProviderRegistrationProvider."
                        "DISABLE_PROVIDER_FAILED",
                    "disable the provider failed."));
        }

        //
        // The provider disable failed since there are pending requests
        //
        if (ret_value == -2)
        {
            throw PEGASUS_CIM_EXCEPTION_L(CIM_ERR_FAILED,
                MessageLoaderParms(
                    "ControlProviders.ProviderRegistrationProvider."
                        "ProviderRegistrationProvider."
                        "DISABLE_PROVIDER_FAILED_PROVIDER_BUSY",
                    "disable the provider failed: Provider is busy."));
        }
    }

    //
    // disable provider module before remove provider registration
    // if the class is PG_ProviderModule
    //

    if (className.equal (PEGASUS_CLASSNAME_PROVIDERMODULE))
    {
        // get module name from reference

        for(Uint32 i=0; i<keys.size() ; i++)
        {
            if(keys[i].getName().equal (_PROPERTY_PROVIDERMODULE_NAME))
            {
                moduleName = keys[i].getValue();
                moduleFound = true;
            }
        }

        // if _PROPERTY_PROVIDERMODULE_NAME key not found
        if( !moduleFound)
        {
            throw PEGASUS_CIM_EXCEPTION_L(CIM_ERR_INVALID_PARAMETER,
                MessageLoaderParms(
                "ControlProviders.ProviderRegistrationProvider."
                "ProviderRegistrationProvider.NAME_KEY_NOT_FOUND",
                "key Name was not found"));
        }

        //
        // disable the provider module
        //
        Sint16 ret_value = _disableModule(
            instanceReference, moduleName, false, al);

        //
        // if the provider module disable failed
        //
        if (ret_value == -1)
        {
            throw PEGASUS_CIM_EXCEPTION_L(CIM_ERR_FAILED,
                MessageLoaderParms(
                    "ControlProviders.ProviderRegistrationProvider."
                        "ProviderRegistrationProvider."
                        "DISABLE_PROVIDER_MODULE_FAILED",
                    "disable the provider module failed."));
        }

        //
        // The provider module disable failed since there are
        // pending requests
        //
        if (ret_value == -2)
        {
            throw PEGASUS_CIM_EXCEPTION_L(CIM_ERR_FAILED,
                MessageLoaderParms(
                    "ControlProviders.ProviderRegistrationProvider."
                        "ProviderRegistrationProvider."
                        "DISABLE_PROVIDER_MODULE_FAILED_PROVIDER_BUSY",
                    "disable the provider module failed: Provider is busy."));
        }
    }

    _providerRegistrationManager->deleteInstance(instanceReference);

#ifdef PEGASUS_ENABLE_INTEROP_PROVIDER
    if(className.equal (PEGASUS_CLASSNAME_PROVIDER) ||
       className.equal (PEGASUS_CLASSNAME_PROVIDERCAPABILITIES) ||
       className.equal (PEGASUS_CLASSNAME_PROVIDERMODULE))
    {
        _sendUpdateCacheMessagetoInteropProvider(context);
    }
#endif
    // complete processing the request
    handler.complete();
}

// Block a provider, unblock a provider, and stop a provider
void ProviderRegistrationProvider::invokeMethod(
    const OperationContext & context,
    const CIMObjectPath & objectReference,
    const CIMName & methodName,
    const Array<CIMParamValue> & inParameters,
    MethodResultResponseHandler & handler)
{
    // get userName and only privileged user can execute this operation
    String userName;
    try
    {
        IdentityContainer container = context.get(IdentityContainer::NAME);
        userName = container.getUserName();
    }
    catch (...)
    {
        userName = String::EMPTY;
    }

#ifndef PEGASUS_OS_ZOS
    if ((userName != String::EMPTY) && !System::isPrivilegedUser(userName))
    {
        throw PEGASUS_CIM_EXCEPTION_L(CIM_ERR_ACCESS_DENIED,
            MessageLoaderParms(
            "ControlProviders.ProviderRegistrationProvider."
                "ProviderRegistrationProvider."
                "SUPERUSER_PRIVILEGE_REQUIRED_DISABLE_ENABLE_PROVIDERS",
            "You must have superuser privilege to disable or enable"
                " providers."));
    }
#endif

    if(!objectReference.getNameSpace().equal (PEGASUS_NAMESPACENAME_INTEROP))
    {
        throw PEGASUS_CIM_EXCEPTION(CIM_ERR_NOT_SUPPORTED,
            objectReference.getNameSpace().getString());
    }


    // l10n
    // Get the client's list of preferred languages for the response
    AcceptLanguageList al;
    try
    {
        AcceptLanguageListContainer al_container =
                (AcceptLanguageListContainer)context.get
                (AcceptLanguageListContainer::NAME);
        al = al_container.getLanguages();
    }
    catch (...)
    {
        ;   // Leave AcceptLanguageList empty
    }

    String moduleName;
    Boolean moduleFound = false;

    // get module name from reference
    Array<CIMKeyBinding> keys = objectReference.getKeyBindings();

    for(Uint32 i=0; i<keys.size() ; i++)
    {
        if(keys[i].getName().equal (_PROPERTY_PROVIDERMODULE_NAME))
        {
            moduleName = keys[i].getValue();
            moduleFound = true;
        }
    }


    // if _PROPERTY_PROVIDERMODULE_NAME key not found
    if( !moduleFound)
    {
        throw PEGASUS_CIM_EXCEPTION_L(CIM_ERR_INVALID_PARAMETER,
            MessageLoaderParms(
                "ControlProviders.ProviderRegistrationProvider."
                    "ProviderRegistrationProvider.NAME_KEY_NOT_FOUND",
                "key Name was not found"));
    }

    handler.processing();

    Sint16 ret_value;

    if (methodName.equal(_STOP_PROVIDER))
    {
        // disable module
        ret_value = _disableModule(objectReference, moduleName, false, al);
    }
    else if (methodName.equal(_START_PROVIDER))
    {
        // enable module
        ret_value = _enableModule(objectReference, moduleName, al);
    }
    else if (methodName.equal(_SET_MODULEGROUPNAME))
    {
        String paramName;
        if (!inParameters.size() ||
            ((paramName = inParameters[0].getParameterName())
                != _PARAM_MODULEGROUPNAME))
        {
            throw PEGASUS_CIM_EXCEPTION(CIM_ERR_INVALID_PARAMETER, paramName);
        }
        // set group
        String moduleGroupName;
        inParameters[0].getValue().get(moduleGroupName);
        ret_value = _setModuleGroupName(
            objectReference, moduleName, moduleGroupName, al);
    }
    else
    {
        throw PEGASUS_CIM_EXCEPTION(CIM_ERR_METHOD_NOT_AVAILABLE,
            String::EMPTY);
    }
#ifdef PEGASUS_ENABLE_INTEROP_PROVIDER
    _sendUpdateCacheMessagetoInteropProvider(context);
#endif

    CIMValue retValue(ret_value);
    handler.deliver(retValue);
    handler.complete();
}

Sint16 ProviderRegistrationProvider::_setModuleGroupName(
    const CIMObjectPath & moduleRef,
    const String & moduleName,
    const String & moduleGroupName,
    const AcceptLanguageList & al)
{
    Sint32 disableRC = _disableModule (moduleRef, moduleName, false, al);

    if (disableRC == -1)
    {
        throw PEGASUS_CIM_EXCEPTION_L(CIM_ERR_FAILED,
            MessageLoaderParms(
                "ControlProviders.ProviderRegistrationProvider."
                    "ProviderRegistrationProvider."
                    "DISABLE_PROVIDER_FAILED",
                "disable the provider failed."));
    }

    if (disableRC == -2)
    {
        throw PEGASUS_CIM_EXCEPTION_L(CIM_ERR_FAILED,
            MessageLoaderParms(
                "ControlProviders.ProviderRegistrationProvider."
                    "ProviderRegistrationProvider."
                    "DISABLE_PROVIDER_FAILED_PROVIDER_BUSY",
                "disable the provider failed: Provider is busy."));
    }

    String errorMsg;

    if (!_providerRegistrationManager->setProviderModuleGroupName(
        moduleName, moduleGroupName, errorMsg))
    {
        throw PEGASUS_CIM_EXCEPTION_L(CIM_ERR_FAILED,
            MessageLoaderParms(
                "ControlProviders.ProviderRegistrationProvider."
                    "ProviderRegistrationProvider."
                    "SET_PROVIDERMODULEGROUPNAME_FAILED",
                "Failed to set the ModuleGroupName: $0.",
                errorMsg));
    }

    // Enable the module only if it was previously enabled.
    if (disableRC == 0)
    {
        try
        {
            _enableModule(moduleRef, moduleName, al);
        }
        catch(const Exception &e)
        {
            PEG_TRACE((TRC_CONTROLPROVIDER,Tracer::LEVEL1,
                "Exception caught while enabling the provider module %s : %s",
                (const char*)moduleName.getCString(),
                (const char*)e.getMessage().getCString()));
        }
        catch(...)
        {
            PEG_TRACE((
                TRC_CONTROLPROVIDER,
                Tracer::LEVEL1,
                "Unknown error occurred while"
                    " enabling the provider modules %s.",
                (const char*)moduleName.getCString()));
        }
    }

    return 0;
}


// get provider manager service
MessageQueueService * ProviderRegistrationProvider::_getProviderManagerService()
{
    MessageQueue * queue =
        MessageQueue::lookup(PEGASUS_QUEUENAME_PROVIDERMANAGER_CPP);
    MessageQueueService * _service = dynamic_cast<MessageQueueService *>(queue);

    return(_service);
}

ProviderRegistrationProvider & ProviderRegistrationProvider::operator=
    (const ProviderRegistrationProvider & handle)
{
    if(this == &handle)
    {
        return(*this);
    }

    return(*this);
}

Array<Uint16>
    ProviderRegistrationProvider::_sendDisableMessageToProviderManager(
        CIMDisableModuleRequestMessage * disable_req)
{
    MessageQueueService * _service = _getProviderManagerService();
    Uint32 _queueId = _service->getQueueId();

    // create request envelope
    AsyncLegacyOperationStart * asyncRequest =
        new AsyncLegacyOperationStart (
            NULL,
            _queueId,
            disable_req);

    AsyncReply * asyncReply =
        _controller->ClientSendWait(_queueId, asyncRequest);
    CIMDisableModuleResponseMessage * response =
        reinterpret_cast<CIMDisableModuleResponseMessage *>(
             (dynamic_cast<AsyncLegacyOperationResult
                  *>(asyncReply))->get_result());
    if (response->cimException.getCode() != CIM_ERR_SUCCESS)
    {
        CIMException e = response->cimException;
        delete asyncRequest;
        delete asyncReply;
        delete response;
        throw e;
    }

    Array<Uint16> operationalStatus = response->operationalStatus;

    delete asyncRequest;
    delete asyncReply;
    delete response;

    return(operationalStatus);
}

Array<Uint16> ProviderRegistrationProvider::_sendEnableMessageToProviderManager(
        CIMEnableModuleRequestMessage * enable_req)
{
    MessageQueueService * _service = _getProviderManagerService();
    Uint32 _queueId = _service->getQueueId();

    // create request envelope
    AsyncLegacyOperationStart * asyncRequest =
        new AsyncLegacyOperationStart (
            NULL,
            _queueId,
            enable_req);

    AsyncReply * asyncReply =
        _controller->ClientSendWait(_queueId, asyncRequest);
    CIMEnableModuleResponseMessage * response =
        reinterpret_cast<CIMEnableModuleResponseMessage *>(
             (dynamic_cast<AsyncLegacyOperationResult
                  *>(asyncReply))->get_result());
    if (response->cimException.getCode() != CIM_ERR_SUCCESS)
    {
        CIMException e = response->cimException;
        delete asyncRequest;
        delete asyncReply;
        delete response;
        throw (e);
    }

    Array<Uint16> operationalStatus = response->operationalStatus;

    delete asyncRequest;
    delete asyncReply;
    delete response;

    return(operationalStatus);
}

// send termination message to subscription service
void ProviderRegistrationProvider::_sendTerminationMessageToSubscription(
    const CIMObjectPath & ref, const String & moduleName,
    const Boolean disableProviderOnly,
    const AcceptLanguageList & al)
{
    CIMInstance instance;
    String _moduleName;
    Array<CIMInstance> instances;

    if (!disableProviderOnly)
    {
        CIMObjectPath reference("", PEGASUS_NAMESPACENAME_INTEROP,
            PEGASUS_CLASSNAME_PROVIDER, ref.getKeyBindings());

        Array<CIMObjectPath> instanceNames =
            _providerRegistrationManager->enumerateInstanceNamesForClass(
                reference);

        // find all the instances which have same module name as moduleName
        for (Uint32 i = 0, n=instanceNames.size(); i < n; i++)
        {
            //
            // get provider module name from reference
            //

            Array<CIMKeyBinding> keys = instanceNames[i].getKeyBindings();

            for(Uint32 j=0; j < keys.size(); j++)
            {
                if(keys[j].getName().equal (_PROPERTY_PROVIDERMODULENAME))
                {
                    _moduleName = keys[j].getValue();
                }
            }

            if (String::equalNoCase(moduleName, _moduleName))
            {
                reference.setKeyBindings(keys);
                instance = _providerRegistrationManager->getInstance(reference);
                //
                // if the provider is indication provider
                //
                if (_isIndicationProvider(moduleName, instance))
                {
                    instances.append(instance);
                }
            }
        }
    }
    else
    {
        instance = _providerRegistrationManager->getInstance(ref);

        //
        // if the provider is indication provider
        //
        if (_isIndicationProvider(moduleName, instance))
        {
            instances.append(instance);
        }
    }

    //
    // get indication server queueId
    //
    MessageQueueService * _service = _getIndicationService();

    if (_service != NULL)
    {
        Uint32 _queueId = _service->getQueueId();

        CIMNotifyProviderTerminationRequestMessage * termination_req =
            new CIMNotifyProviderTerminationRequestMessage(
                XmlWriter::getNextMessageId (),
                instances,
                QueueIdStack(_service->getQueueId()));

        // l10n
        termination_req->operationContext.set(AcceptLanguageListContainer(al));

        // create request envelope
        AsyncLegacyOperationStart asyncRequest(
            NULL,
            _queueId,
            termination_req);

        AutoPtr <AsyncReply> asyncReply(
            _controller->ClientSendWait(_queueId, &asyncRequest));

        AutoPtr <CIMNotifyProviderTerminationResponseMessage> response
            (reinterpret_cast <CIMNotifyProviderTerminationResponseMessage *>
            ((dynamic_cast <AsyncLegacyOperationResult *>
            (asyncReply.get ()))->get_result ()));

        if (response->cimException.getCode () != CIM_ERR_SUCCESS)
        {
            CIMException e = response->cimException;
            throw e;
        }
    }
}

// get indication service
MessageQueueService * ProviderRegistrationProvider::_getIndicationService()
{
    MessageQueue * queue = MessageQueue::lookup(
        PEGASUS_QUEUENAME_INDICATIONSERVICE);

    MessageQueueService * _service =
        dynamic_cast<MessageQueueService *>(queue);
    return(_service);
}

// disable provider module, return 0 if module is disabled successfully,
// return 1 if module is already disabled, return -2 if module can not be
// disabled since there are pending requests, otherwise, return -1
Sint16 ProviderRegistrationProvider::_disableModule(
    const CIMObjectPath & objectReference,
    const String & moduleName,
    Boolean disableProviderOnly,
    const AcceptLanguageList & al)
{
        //
        // get module status
        //
        Array<Uint16> _OperationalStatus =
            _providerRegistrationManager->getProviderModuleStatus( moduleName);

        for (Uint32 i = 0; i<_OperationalStatus.size(); i++)
        {
            // retValue equals 1 if module is already disabled
            if (_OperationalStatus[i] == CIM_MSE_OPSTATUS_VALUE_STOPPED ||
                _OperationalStatus[i] == CIM_MSE_OPSTATUS_VALUE_STOPPING)
            {
                return (1);
            }
        }

        CIMInstance instance;
        Array<CIMInstance> instances;
        CIMInstance mInstance;
        String _moduleName;
        CIMObjectPath providerRef;
        Boolean indProvider = false;
        Array<Boolean> indicationProviders;

        // disable a provider module or delete a provider module
        if (!disableProviderOnly)
        {
            providerRef = CIMObjectPath(objectReference.getHost(),
                                 objectReference.getNameSpace(),
                                 PEGASUS_CLASSNAME_PROVIDER,
                                 objectReference.getKeyBindings());

            // get module instance
            mInstance =
                _providerRegistrationManager->getInstance(objectReference);

        }
        else // disable a provider
        {
            // get module instance
            Array <CIMKeyBinding> moduleKeyBindings;
            moduleKeyBindings.append (CIMKeyBinding
                (_PROPERTY_PROVIDERMODULE_NAME, moduleName,
                 CIMKeyBinding::STRING));

            CIMObjectPath moduleRef(objectReference.getHost(),
                                    objectReference.getNameSpace(),
                                    PEGASUS_CLASSNAME_PROVIDERMODULE,
                                    moduleKeyBindings);

            mInstance =
                _providerRegistrationManager->getInstance(moduleRef);
        }

        if (!disableProviderOnly)
        {
            // get all provider instances which have same module name as
            // moduleName
            Array<CIMObjectPath> instanceNames =
                _providerRegistrationManager->enumerateInstanceNamesForClass(
                    providerRef);

            for(Uint32 i = 0, n=instanceNames.size(); i < n; i++)
            {
                //
                // get provider module name from reference
                //

                Array<CIMKeyBinding> keys = instanceNames[i].getKeyBindings();

                for(Uint32 j=0; j < keys.size(); j++)
                {
                    if(keys[j].getName().equal (_PROPERTY_PROVIDERMODULENAME))
                    {
                        _moduleName = keys[j].getValue();
                    }
                }

                if (String::equalNoCase(_moduleName, moduleName))
                {
                    providerRef.setKeyBindings(keys);
                    instance = _providerRegistrationManager->getInstance
                        (providerRef);
                    if (_isIndicationProvider(moduleName, instance))
                    {
                        indProvider = true;
                        indicationProviders.append(true);
                    }
                    else
                    {
                        indicationProviders.append(false);
                    }
                    instances.append(instance);
                }

            }
        }
        else
        {
            instance =
                _providerRegistrationManager->getInstance(objectReference);
            if (_isIndicationProvider(moduleName, instance))
            {
                indProvider = true;
                indicationProviders.append(true);
            }
            else
            {
                indicationProviders.append(false);
            }

            instances.append(instance);
        }

        //
        // get provider manager service
        //
        MessageQueueService * _service = _getProviderManagerService();

        if (_service != NULL)
        {
            // create CIMDisableModuleRequestMessage
            CIMDisableModuleRequestMessage * disable_req =
                new CIMDisableModuleRequestMessage(
                    XmlWriter::getNextMessageId (),
                    mInstance,
                    instances,
                    disableProviderOnly,
                    indicationProviders,
                    QueueIdStack(_service->getQueueId()));
                // l10n
                disable_req->operationContext.set
                    (AcceptLanguageListContainer(al));;

            Array<Uint16> _opStatus =
                _sendDisableMessageToProviderManager(disable_req);

            if (!disableProviderOnly) // disable provider module
            {
                for (Uint32 i = 0; i<_opStatus.size(); i++)
                {
                    // module was disabled successfully
                    if (_opStatus[i] == CIM_MSE_OPSTATUS_VALUE_STOPPED)
                    {
                        if (indProvider)
                        {
                            // send termination message to subscription service
                            _sendTerminationMessageToSubscription(
                                objectReference, moduleName, false, al);
                        }
                        return (0);
                    }

                    // module is not disabled since there are pending
                    // requests for the providers in the module
                    if (_opStatus[i] == CIM_MSE_OPSTATUS_VALUE_OK)
                    {
                        return (-2);
                    }
                }
            }
            else // disable provider
            {
                if (indProvider)
                {
                    _sendTerminationMessageToSubscription(objectReference,
                        moduleName, true, al);
                }
                return (0);
            }
        }

        // disable failed
        return (-1);
}

// enable provider module
// return 0 if module is enabled successfully,
// return 1 if module is already enabled,
// return 2 if module can not be enabled since module is stopping,
// otherwise, return -1
Sint16 ProviderRegistrationProvider::_enableModule(
    const CIMObjectPath & moduleRef,
    const String & moduleName,
    const AcceptLanguageList & al)
{
        //
        // get module status
        //
        Array<Uint16> _OperationalStatus =
            _providerRegistrationManager->getProviderModuleStatus( moduleName);

        for (Uint32 i = 0; i<_OperationalStatus.size(); i++)
        {
            // retValue equals 1 if module is already enabled
            if (_OperationalStatus[i] == CIM_MSE_OPSTATUS_VALUE_OK)
            {
                return (1);
            }

            // retValue equals 2 if module is stopping
            // at this stage, module can not be started
            if (_OperationalStatus[i] == CIM_MSE_OPSTATUS_VALUE_STOPPING)
            {
                return (2);
            }
        }

        // get module instance
        CIMInstance mInstance =
            _providerRegistrationManager->getInstance(moduleRef);

        //
        // get provider manager service
        //
        MessageQueueService * _service = _getProviderManagerService();
        Boolean enabled = false;

        if (_service != NULL)
        {
            // create CIMEnableModuleRequestMessage
            CIMEnableModuleRequestMessage * enable_req =
                new CIMEnableModuleRequestMessage(
                    XmlWriter::getNextMessageId (),
                    mInstance,
                    QueueIdStack(_service->getQueueId()));

            enable_req->operationContext.set(AcceptLanguageListContainer(al));

            Array<Uint16> _opStatus;
            _opStatus = _sendEnableMessageToProviderManager(enable_req);

            for (Uint32 i = 0; i<_opStatus.size(); i++)
            {
                // module is enabled successfully
                if (_opStatus[i] == CIM_MSE_OPSTATUS_VALUE_OK)
                {
                    enabled = true;
                }
            }
        }

        if (enabled)
        {
            //
            // Since module is enabled, need get updated module instance
            //
            CIMInstance updatedModuleInstance =
                _providerRegistrationManager->getInstance(moduleRef);

            //
            // The module is enabled, need to send enable message to
            // subscription service if the provider is an indication provider
            //

            CIMObjectPath providerRef = CIMObjectPath(String::EMPTY,
                                    moduleRef.getNameSpace(),
                                    PEGASUS_CLASSNAME_PROVIDER,
                                    Array<CIMKeyBinding>());

            //
            // get all provider instances which have same module name as
            // moduleName
            //
            Array<CIMObjectPath> instanceNames =
                _providerRegistrationManager->enumerateInstanceNamesForClass(
                    providerRef);
            CIMInstance pInstance;
            String _moduleName;
            String _providerName;
            Array<CIMInstance> capInstances;

            for(Uint32 i = 0, n=instanceNames.size(); i < n; i++)
            {

                Array<CIMKeyBinding> keys = instanceNames[i].getKeyBindings();

                for(Uint32 j=0; j < keys.size(); j++)
                {
                    //
                    // get provider module name from reference
                    //
                    if(keys[j].getName().equal (_PROPERTY_PROVIDERMODULENAME))
                    {
                        _moduleName = keys[j].getValue();
                    }

                    //
                    // get provider name from reference
                    //
                    if(keys[j].getName().equal (PEGASUS_PROPERTYNAME_NAME))
                    {
                        _providerName = keys[j].getValue();
                    }
                }

                if (String::equalNoCase(_moduleName, moduleName))
                {
                    providerRef.setKeyBindings(keys);
                    pInstance = _providerRegistrationManager->getInstance
                        (providerRef);
                    //
                    // get all the indication capability instances which
                    // belongs to this provider
                    //
                    capInstances = _getIndicationCapInstances(
                                   moduleName, pInstance, providerRef);

                    //
                    // if there are indication capability instances
                    //
                    if (capInstances.size() != 0)
                    {
                        _sendEnableMessageToSubscription(updatedModuleInstance,
                                                         pInstance,
                                                         capInstances,
                                                         al);
                    }
                }
            }
            return (0);
        }


        // enable failed
        return (-1);
}

// send enable message to indication service
void ProviderRegistrationProvider::_sendEnableMessageToSubscription(
    const CIMInstance & mInstance,
    const CIMInstance & pInstance,
    const Array<CIMInstance> & capInstances,
    const AcceptLanguageList & al)
{
    //
    // get indication server queueId
    //
    MessageQueueService * _service = _getIndicationService();

    if (_service != NULL)
    {
        Uint32 _queueId = _service->getQueueId();

        CIMNotifyProviderEnableRequestMessage * enable_req =
                new CIMNotifyProviderEnableRequestMessage (
                    XmlWriter::getNextMessageId (),
                    capInstances,
                    QueueIdStack(_service->getQueueId()));

        enable_req->operationContext.set(AcceptLanguageListContainer(al));
    enable_req->operationContext.insert(ProviderIdContainer
                                        (mInstance,pInstance));

        // create request envelope
        AsyncLegacyOperationStart * asyncRequest =
            new AsyncLegacyOperationStart (
                NULL,
                _queueId,
                enable_req);

        AsyncReply * asyncReply =
            _controller->ClientSendWait(_queueId, asyncRequest);

        CIMNotifyProviderEnableResponseMessage * response =
            reinterpret_cast<CIMNotifyProviderEnableResponseMessage *>(
                (dynamic_cast<AsyncLegacyOperationResult *>
                    (asyncReply))->get_result());

        if (response->cimException.getCode() != CIM_ERR_SUCCESS)
        {
            CIMException e = response->cimException;
            delete asyncRequest;
            delete asyncReply;
            delete response;
            throw e;
        }

        delete asyncRequest;
        delete asyncReply;
        delete response;
    }
}

// If provider is an indication provider, return true,
// otherwise, return false
Boolean ProviderRegistrationProvider::_isIndicationProvider(
    const String & moduleName,
    const CIMInstance & instance)
{
    // get provider name
    String providerName;
    Uint32 pos = instance.findProperty(PEGASUS_PROPERTYNAME_NAME);
    if (pos != PEG_NOT_FOUND)
    {
        instance.getProperty(pos).getValue().get(providerName);
    }

    return (_providerRegistrationManager->isIndicationProvider(
            moduleName, providerName));
}

//
// get all the capability instances whose provider type is indication
//
Array<CIMInstance> ProviderRegistrationProvider::_getIndicationCapInstances(
    const String & moduleName,
    const CIMInstance & instance,
    const CIMObjectPath & providerRef)
{
    // get provider name
    String providerName;
    Uint32 pos = instance.findProperty(PEGASUS_PROPERTYNAME_NAME);
    if (pos != PEG_NOT_FOUND)
    {
        instance.getProperty(pos).getValue().get(providerName);
    }

    CIMObjectPath capabilityRef;

    capabilityRef = CIMObjectPath(providerRef.getHost(),
                                  providerRef.getNameSpace(),
                                  PEGASUS_CLASSNAME_PROVIDERCAPABILITIES,
                                  providerRef.getKeyBindings());

    // get all Capabilities instances
    Array<CIMObjectPath> instanceNames =
        _providerRegistrationManager->enumerateInstanceNamesForClass(
            capabilityRef);

    String _moduleName, _providerName;
    CIMInstance capInstance;
    Array<Uint16> providerTypes;
    Array<CIMInstance> indCapInstances = 0;
    for(Uint32 i = 0, n=instanceNames.size(); i < n; i++)
    {
        Array<CIMKeyBinding> keys = instanceNames[i].getKeyBindings();

        for(Uint32 j=0; j < keys.size(); j++)
        {
             if(keys[j].getName().equal (_PROPERTY_PROVIDERMODULENAME))
             {
                  _moduleName = keys[j].getValue();
             }

             if(keys[j].getName().equal (_PROPERTY_PROVIDERNAME))
             {
                  _providerName = keys[j].getValue();
             }
        }

       //
       // if capability instance has same module name as moduleName
       // and same provider name as providerName, get provider type
       //
       if(String::equal(_moduleName, moduleName) &&
          String::equal(_providerName, providerName))
       {
            capInstance = _providerRegistrationManager->getInstance
                                (instanceNames[i]);

            Uint32 providerTypePos = capInstance.findProperty(
                CIMName (_PROPERTY_PROVIDERTYPE));
            if (providerTypePos != PEG_NOT_FOUND)
            {
                capInstance.getProperty(providerTypePos).getValue().get(
                    providerTypes);

                for (Uint32 k=0; k < providerTypes.size(); k++)
                {
                    //
                    // if provider type of the instance is indication,
                    // append the instance
                    //
                    if (providerTypes[k] == _INDICATION_PROVIDER)
                    {
                        indCapInstances.append(capInstance);
                    }
                }
            }
        }
    }

    return (indCapInstances);
}

// CIMIndicationProvider interface

void ProviderRegistrationProvider::enableIndications(
    IndicationResponseHandler& handler)
{
    {
        AutoMutex mtx(_indicationDeliveryMtx);
        _enableIndications = true;
        _indicationResponseHandler = &handler;
    }

    if (!_providerRegistrationManager->getInitComplete())
    {
        _generatePMIndications(PM_ENABLED_CIMSERVER_START);
    }
}

void ProviderRegistrationProvider::disableIndications()
{
    _generatePMIndications(PM_DISABLED_CIMSERVER_STOP);
    AutoMutex mtx(_indicationDeliveryMtx);
    _enableIndications = false;
    _indicationResponseHandler = 0;
}

#ifdef PEGASUS_ENABLE_INTEROP_PROVIDER
void ProviderRegistrationProvider::_sendUpdateCacheMessagetoInteropProvider(
    const OperationContext & context)
{
    String referenceStr("//", 2);
    referenceStr.append(System::getHostName());
    referenceStr.append("/");
    referenceStr.append(PEGASUS_NAMESPACENAME_INTEROP.getString());
    referenceStr.append(":");
    referenceStr.append(
        PEGASUS_CLASSNAME_PG_PROVIDERPROFILECAPABILITIES.getString());
    CIMObjectPath reference(referenceStr);

    Array<CIMParamValue> inParams;

    try
    {
        CIMInvokeMethodRequestMessage* request =
            new CIMInvokeMethodRequestMessage(
                XmlWriter::getNextMessageId(),
                PEGASUS_NAMESPACENAME_INTEROP,
                referenceStr,
                "updateCache",
                inParams,
                QueueIdStack(_controller->getQueueId()));

        request->operationContext = context;

        AsyncModuleOperationStart* moduleControllerRequest =
            new AsyncModuleOperationStart(
                0,
                _controller->getQueueId(),
                PEGASUS_MODULENAME_INTEROPPROVIDER,
                request);

        _controller->SendForget(moduleControllerRequest);
    }
    catch(const Exception &e)
    {
        PEG_TRACE((TRC_CONTROLPROVIDER,Tracer::LEVEL1,
            "Exception caught while invoking InteropProvider 'updateCache'"
                " method: %s",(const char*)e.getMessage().getCString()));
    }
    catch(...)
    {
        PEG_TRACE_CSTRING(
            TRC_CONTROLPROVIDER,
            Tracer::LEVEL1,
            "Unknown error occurred while"
                " invoking InteropProvider 'updateCache' method.");
    }
}
#endif

PEGASUS_NAMESPACE_END
