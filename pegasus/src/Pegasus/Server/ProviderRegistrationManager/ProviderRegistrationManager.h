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

#ifndef Pegasus_ProviderRegistrationManager_h
#define Pegasus_ProviderRegistrationManager_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Server/ProviderRegistrationManager/Linkage.h>
#include <Pegasus/Common/String.h>
#include <Pegasus/Common/MessageQueueService.h>
#include <Pegasus/Repository/CIMRepository.h>
#include <Pegasus/Provider/CIMInstanceProvider.h>
#include <Pegasus/Common/ModuleController.h>
#include <Pegasus/Common/CIMMessage.h>
#include <Pegasus/Common/Constants.h>

PEGASUS_NAMESPACE_BEGIN

struct RegistrationTable;

/**
   The name of the provider module name  property for provider capabilities
   class and PG_Provider class
*/
PEGASUS_PRM_LINKAGE extern const CIMName _PROPERTY_PROVIDERMODULENAME;

/**
   The name of the Vendor property for PG_ProviderModule class
*/
PEGASUS_PRM_LINKAGE extern const CIMName _PROPERTY_VENDOR;

/**
   The name of the Version property for PG_ProviderModule class
*/
PEGASUS_PRM_LINKAGE extern const CIMName _PROPERTY_VERSION;

/**
   The name of the interface type property for PG_ProviderModule class
*/
PEGASUS_PRM_LINKAGE extern const CIMName _PROPERTY_INTERFACETYPE;

/**
   The name of the interface version property for PG_ProviderModule class
*/
PEGASUS_PRM_LINKAGE extern const CIMName _PROPERTY_INTERFACEVERSION;

/**
   The name of the location property for PG_ProviderModule class
*/
PEGASUS_PRM_LINKAGE extern const CIMName _PROPERTY_LOCATION;

/**
   The name of the CapabilitiesID property for provider capabilities class
*/
PEGASUS_PRM_LINKAGE extern const CIMName _PROPERTY_CAPABILITIESID;

/**
   The name of the provider name  property for provider capabilities class
*/
PEGASUS_PRM_LINKAGE extern const CIMName _PROPERTY_PROVIDERNAME;

/**
   The name of the classname property for provider capabilities class
*/
PEGASUS_PRM_LINKAGE extern const CIMName _PROPERTY_CLASSNAME;

/**
   The name of the Namespace property for provider capabilities class
*/
PEGASUS_PRM_LINKAGE extern const CIMName _PROPERTY_NAMESPACES;

/**
   The name of the provider type  property for provider capabilities class
*/
PEGASUS_PRM_LINKAGE extern const CIMName _PROPERTY_PROVIDERTYPE;

/**
   The name of the supported properties property for provider capabilities class
*/
PEGASUS_PRM_LINKAGE extern const CIMName _PROPERTY_SUPPORTEDPROPERTIES;

/**
   The name of the supported methods property for provider capabilities class
*/
PEGASUS_PRM_LINKAGE extern const CIMName _PROPERTY_SUPPORTEDMETHODS;

/**
   The name of the Destinations property for PG_ConsumerCapabilities class
*/
PEGASUS_PRM_LINKAGE extern const CIMName _PROPERTY_INDICATIONDESTINATIONS;

/*
    Registered provider types
*/
#define _INSTANCE_PROVIDER Uint16(2)
#define _ASSOCIATION_PROVIDER Uint16(3)
#define _INDICATION_PROVIDER Uint16(4)
#define _METHOD_PROVIDER Uint16(5)
#define _INSTANCE_QUERY_PROVIDER Uint16(7)

class PEGASUS_PRM_LINKAGE ProviderRegistrationManager
{
public:
    ProviderRegistrationManager(CIMRepository* repository);

    virtual ~ProviderRegistrationManager();

    /**
        Lookup Instance, association, or query provider in table of
        registered providers depending on input parameters.

        @param nameSpace CIMNamespaceName defining namespace for provider
        @param classname CIMName defining class for provider
        @param provider CIMInstance returned with instance of PG_Provider
            if successful
        @param providerModule CIMInstance returned with instance of
            providerModule class for provider if successful
        @param is_assoc Boolean optional parameter. If set this is a request
            to lookup an association provider rather than an instance
            provider.  Normally this is used only by RegistrationManager
            lookupAssociationProvider function
        @param has_no_query optional parameter defines whether instance or
            instanceQuery provider to be looked up. If exists the parameter
            is not NULL and instance Query provider exists, returns true.
        @return Boolean true if provider found.
    */
    Boolean lookupInstanceProvider(
        const CIMNamespaceName & nameSpace,
        const CIMName & className,
        CIMInstance & provider,
        CIMInstance & providerModule,
        Boolean is_assoc = false,
        Boolean * has_no_query = NULL);

    /**
        Lookup Method Provider in table of registered providers for
        the nameSpace, classname provided.

        @param nameSpace CIMNamespaceName defining namespace for provider
        @param classname CIMName defining class for provider
        @param provider CIMInstance returned with instance of PG_Provider
            if successful
        @param providerModule CIMInstance returned with instance of
            providerModule class for provider if successful
        @return Boolean true if provider found
    */
    Boolean lookupMethodProvider(
        const CIMNamespaceName & nameSpace,
        const CIMName & className,
        const CIMName & method,
        CIMInstance & provider,
        CIMInstance & providerModule);

    /**
        Lookup Association Provider in table of registered providers for
        the nameSpace, classname defined.

        @param nameSpace CIMNamespaceName defining namespace for provider
        @param assocClassname CIMName defining class for provider
        @param provider CIMInstance returned with instance of PG_Provider
            if successful
        @param providerModule CIMInstance returned with instance of
            providerModule class for provider if successful
        @return Boolean true if provider found
    */
    Boolean lookupAssociationProvider(
        const CIMNamespaceName & nameSpace,
        const CIMName & assocClassName,
        Array<CIMInstance>& provider,
        Array<CIMInstance>& providerModule);

    /**
        Lookup IndicationConsumer in table of registered providers for
        the destination path input.

        @param String destinationPath defined for the consumer
        @param provider CIMInstance returned with instance of PG_Provider
            if successful
        @param providerModule CIMInstance returned with instance of
            providerModule class for provider if successful
        @return Boolean true if provider found
    */
    Boolean lookupIndicationConsumer(
        const String & destinationPath,
            CIMInstance & provider,
        CIMInstance & providerModule);

    Boolean getIndicationProviders(
        const CIMNamespaceName & nameSpace,
        const CIMName & className,
        const CIMPropertyList & requiredProperties,
        Array<CIMInstance> & provider,
        Array<CIMInstance> & providerModule);

    /**
        This function is called with a CIMObjectPath specifying an instance
        of PG_ProviderModule, PG_Provider, PG_ProviderCapabilities, or
        PG_ConsumerCapabilities to be returned.

        @param ref specifies the fully qualified object path of the
            instance of interest.
        @param includeQualifiers indicates whether the returned instance
            must include the qualifiers for the instance and properties.
        @param includeClassOrigin indicates whether the returned instance
            must include the class origin for each of the instance
            elements.
        @param propertyList specifies the minimum set of properties
            required in instances returned by this operation.
    */
    CIMInstance getInstance(
            const CIMObjectPath & ref,
            const Boolean includeQualifiers = false,
            const Boolean includeClassOrigin = false,
            const CIMPropertyList & propertyList = CIMPropertyList());

    Array<CIMInstance> enumerateInstancesForClass(
        const CIMObjectPath & classReference,
        const Boolean includeQualifiers = false,
        const Boolean includeClassOrigin = false,
        const CIMPropertyList & propertyList = CIMPropertyList());

    Array<CIMObjectPath> enumerateInstanceNamesForClass(
        const CIMObjectPath & ref);

    void modifyInstance(
        const CIMObjectPath & ref,
        const CIMInstance & instance,
        const Boolean includeQualifiers,
        const Array<CIMName> & propertyList);

    CIMObjectPath createInstance(
        const CIMObjectPath & ref,
        const CIMInstance & instance);

    void deleteInstance(const CIMObjectPath & ref);

    Array<Uint16> getProviderModuleStatus(const String & providerModuleName);

    /**
        Updates the Provider Module Status.  The value of the
        OperationalStatus property is updated in the Provider Module
        instance in the repository, and in the provider registration table.
        If any removeStatus values are specified, each one of the values
        found in the current OperationalStatus property value is removed
        from the array.  If any appendStatus values are specified, each one
        of the values not found in the current OperationalStatus property
        value is appended to the array.  The resulting OperationalStatus
        property value is returned in the output outStatus parameter.

        @param    providerModuleName String specifying name of the provider
                  module whose status is to be updated

        @param    removeStatus Array of Uint16 specifying values to be
                  removed from the status array (may be empty)

        @param    appendStatus Array of Uint16 specifying values to be
                  appended to the status array (may be empty)

        @param    outStatus OUTPUT Array of Uint16 returning the resulting
                  status array after update

        @return   True if provider module status is successfully updated;
                  False, otherwise
    */
    Boolean updateProviderModuleStatus(
        const String& providerModuleName,
        const Array<Uint16>& removeStatus,
        const Array<Uint16>& appendStatus,
        Array<Uint16>& outStatus);

    /**
        Retrieves all the provider module names who have the given
        moduleGroupName in common.

        @param    moduleGroupName String specifying name of the provider
                  module group.
        @param    moduleNames OUTPUT array of string returning the provider
                  module names for the group

    */
    void getProviderModuleNamesForGroup(
        const String& moduleGroupName,
        Array<String>& moduleNames);

    /**
        Sets the provider module group name for the specified provider
        module.

        @param    providerModuleName String specifying name of the provider
                  module.
        @param    moduleGroupName  String specifying name of the provider
                  module group.
        @param    errorMsg  Output arg, String containing the reason for failure

        @return   True if the ModuleGroupName set successfully,
                  Otherwise, return false.
    */
    Boolean setProviderModuleGroupName(
        const String& providerModuleName,
        const String& moduleGroupName,
        String &errorMsg);

    /**
        Determines whether specified provider is an indication provider.

        @param    moduleName specifies name of the provider module which
                  the provider is in.

        @param    providerName specifies name of the provider which
                  needs to be determine.

        @return   True if the provider is an indication provider;
                  Otherwise, return false.
    */
    Boolean isIndicationProvider(
       const String & moduleName,
       const String & providerName);

    void setPMInstAlertCallback(
        void (*)(
            const CIMInstance&,
            const CIMInstance&,
            PMInstAlertCause));

    void sendPMInstAlert(
        const CIMInstance &instance,
        PMInstAlertCause alertCause);

    void setInitComplete();
    Boolean getInitComplete();

    enum Operation {OP_CREATE = 1, OP_DELETE = 2, OP_MODIFY = 3};

protected:

    CIMRepository * _repository;

    /**
        HashTable to store instances of PG_ProviderModule, instances of
        PG_Provider, and instances of PG_ProviderCapability
    */
    RegistrationTable* _registrationTable;

    /**
        A lock used to control access to the _registrationTable.  Before
        accessing the _registrationTable, one must first lock this for
        read access.  Before updating the _registrationTable, one must
        first lock this for write access.  One should never attempt to
        lock the _registrationTableLock while holding the repository lock.
    */
    ReadWriteSem _registrationTableLock;

    Boolean _initComplete;

    void (*_PMInstAlertCallback)(
        const CIMInstance &providerModule,
        const CIMInstance &provider,
        PMInstAlertCause cause);

    String _generateKey(const String & name, const String & provider);

    String _generateKey(
        const CIMNamespaceName & namespaceName,
        const CIMName & className,
        const String & providerType);

    String _generateKey(
        const CIMNamespaceName & namespaceName,
        const CIMName & className,
        const String & supportedMethod,
        const String & providerType);

    MessageQueueService * _getIndicationService();

    void _sendMessageToSubscription(
        CIMRequestMessage * notify);

private:

    /**
        Initialize the registration table.  The caller must first lock
        _registrationTableLock for write access.
    */
    void _initialRegistrationTable();

    /**
        Adds an entry to the registration table for the specified
        instances.  The caller must first lock _registrationTableLock
        for write access.
    */
    void _addInstancesToTable(
        const String & key,
        Array<CIMInstance> & instances);

    /**
        Adds an entry to the registration table for the specified
        instances and key.  This method is intended for
        use in the initialization routine.  The caller must first
        lock _registrationTableLock for write access.
    */
    void _addInitialInstancesToTable(
        const String & key,
        const Array<CIMInstance> & instances);
    /**
        Adds generates a capability key and entry to the registration table
        for the specified instance.  This method is intended for use
        in the initialization routine.  The caller must first lock
        _registrationTableLock for write access.
    */
    void _addOneInitialInstanceToTable(
        const CIMInstance & instance,
        const String & namespaceName,
        const CIMName & className,
        const String & providerType);
    /**
        Adds generates a capability key and entry to the registration table
        for the specified instance.  This method is intended for use
        in the initialization routine.  The caller must first lock
        _registrationTableLock for write access.
    */
    void _addOneInitialMethodInstanceToTable(
        const CIMInstance & instance,
        const String & namespaceName,
        const CIMName & className,
        const String & methodName,
        const String & providerType);

    /**
        Get the provider instance and module instance corresponding to
        the specified provider name or provider module name from the
        registration table.  The caller must first lock
        _registrationTableLock for read (or write) access.
    */
    void _getInstances(
        const String & providerName,
        const String & moduleName,
        CIMInstance & providerInstance,
        CIMInstance & moduleInstance);

    void _getPropertyNames(
        const CIMInstance & instance,
        CIMPropertyList & propertyNames);

    /**
        Notify the subscription service that the specified provider
        capability instance was deleted.  The caller must first lock
        _registrationTableLock for read (or write) access.
    */
    void _sendDeleteNotifyMessage(const CIMInstance & instance);

    /**
        Notify the subscription service that the specified provider
        capability instance was modified.  The caller must first lock
        _registrationTableLock for read (or write) access.
    */
    void _sendModifyNotifyMessage(
        const CIMInstance & instance,
        const CIMInstance & origInstance);

    /**
        Register a provider.  The caller must first lock
        _registrationTableLock for write access.

        The instance parameter is modified to contain the instance that was
        actually created.
    */
    CIMObjectPath _createInstance(
        const CIMObjectPath & ref,
        CIMInstance & instance,
        Operation flag);

    /**
        Unregister a provider.  The caller must first lock
        _registrationTableLock for write access.
    */
    void _deleteInstance(
        const CIMObjectPath & ref,
        Operation flag,
        CIMInstance &deletedInstance);

    /**
        Set the status of the specified provider module instance and
        update the repository.  The caller must first lock the repository
        for write access.
    */
    void _setStatus(
        const Array<Uint16> & status,
        CIMInstance & moduleInstance);

    /**
        Gets the Provider Module Status.  The value of the OperationalStatus
        property is retrieved from the Provider Module instance in the
        provider registration table.

        @param    providerModuleName String specifying name of the provider
                  module whose status is to be retrieved

        @exception CIMException(CIM_ERR_NOT_FOUND) if the specified provider
                   module is not found.

        @return   Array of Uint16 containing the OperationalStatus property
                  value
    */
    Array<Uint16> _getProviderModuleStatus(
        const String& providerModuleName);

    /**
        Get the namespaces from the Namespaces property of the defined
        instance and set into the return value.
        @param    instance CIMInstance containing property named
                           PROPERTY_NAMESPACES

        @return   Array<CIMNamespaceName> with the namespaces from the
                  values in the property
    */
    Array<CIMNamespaceName> _getNamespaceNames(const CIMInstance& instance,
                                               Array<String>& nameSpaces);
};

class PEGASUS_PRM_LINKAGE WildCardNamespaceNames
{
    static Array<String> _nsstr;
    static Array<CIMNamespaceName> _ns;
    static Array<Uint32> _nsl;
public:
    static String add(String ns);
    static const CIMNamespaceName & check(const CIMNamespaceName & in);
    static void remap(
        CIMRepository *repos,
        Array<String> & in,
        Array<CIMNamespaceName> & names);
    static const Array<String> & getArray();
};

PEGASUS_NAMESPACE_END

#endif
