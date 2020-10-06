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

#ifndef Pegasus_SubscriptionTable_h
#define Pegasus_SubscriptionTable_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Server/Linkage.h>
#include <Pegasus/Common/CIMInstance.h>
#include <Pegasus/Common/CIMObjectPath.h>
#include <Pegasus/Common/String.h>
#include <Pegasus/Common/Array.h>
#include <Pegasus/Common/HashTable.h>
#include <Pegasus/General/SubscriptionKey.h>

#include "ProviderClassList.h"
#include "SubscriptionRepository.h"

PEGASUS_NAMESPACE_BEGIN

class IndicationService;

/**
    Entry for ActiveSubscriptions table
 */
struct ActiveSubscriptionsTableEntry
{
    CIMInstance subscription;
    Array <ProviderClassList> providers;
};

/**
    Table for active subscription information

    The ActiveSubscriptions table is used to keep track of active subscriptions.
    This table contains an entry for each subscription that is enabled
    (value of SubscriptionState property is enabled).
    Each entry consists of a Subscription instance, and a list of
    ProviderClassList structs representing the providers, if any, currently
    serving each subscription and the relevant indication subclasses served by
    each provider.
    The Key is a string constructed from the elements of a subscriptions
    object path: <filterInstanceName><handlerInstanceName>/namespace/classname
    Entries are inserted into the table on initialization, when an enabled
    subscription instance is created, or when a subscription instance is
    modified to be enabled.
    Entries are removed from the table when an enabled subscription instance is
    deleted, or when a subscription instance is modified to be disabled.
    Entries are updated (remove followed by insert) when a provider serving a
    subscription is disabled or enabled, or a provider registration change
    occurs.
    The _handleProcessIndicationRequest() function, when a list of
    subscriptions is included in request, looks up each subscription in the
    table.
    The _handleNotifyProviderRegistrationRequest() function, once matching
    subscriptions have been identified, looks up the provider information for
    each matching subscription in the table.
    The _handleModifyInstanceRequest() and _handleDeleteInstanceRequest()
    functions, when sending delete requests to providers, look up the providers
    for the subscription in the table.
    The _handleNotifyProviderTerminationRequest() function, when a provider is
    disabled, iterates through the table to retrieve all active
    subscriptions being served by the provider.
    The terminate() function, when the CIM Server is being shut down, iterates
    through the table to retrieve all active subscriptions.
 */
typedef HashTable <SubscriptionKey,
                   ActiveSubscriptionsTableEntry,
                   SubscriptionKeyEqualFunc,
                   SubscriptionKeyHashFunc> ActiveSubscriptionsTable;

/**
    Entry for SubscriptionClasses table
 */
struct SubscriptionClassesTableEntry
{
    CIMName indicationClassName;
    CIMNamespaceName sourceNamespaceName;
    Array <CIMInstance> subscriptions;
};

/**
    Table for subscription classes information

    The SubscriptionClasses Table is used to keep track of active subscriptions
    for each indication subclass-source namespace pair.
    Each entry consists of an indication subclass name, a source namespace name,
    and a list of subscription instances.
    The Key is generated by concatenating the indication subclass name, and the
    source namespace name.
    The _handleProcessIndicationRequest() function, when no list of
    subscriptions is included in the request, looks up matching subscriptions
    in the table, using the class name and namespace name of the indication
    instance.
    The _handleNotifyProviderRegistrationRequest() function, when an indication
    provider registration instance has been created or deleted, looks up
    matching subscriptions in the table, using the class name and namespace
    names from the provider registration instance.
 */
typedef HashTable <String,
                   SubscriptionClassesTableEntry,
                   EqualNoCaseFunc,
                   HashLowerCaseFunc> SubscriptionClassesTable;

/**

    The SubscriptionTable class manages an in-memory cache of indication
    subscription information in two hash tables.  The Active Subscriptions
    table includes all enabled subscriptions along with the providers currently
    serving them.  The Subscription Classes table includes the enabled
    subscription instances for each indication class/namespace pair, based on
    the filter query and sourceNamespace.

    @author  Hewlett-Packard Company

 */

class PEGASUS_SERVER_LINKAGE SubscriptionTable
{
public:

    /**
        Constructs a SubscriptionTable instance.
     */
    SubscriptionTable (
        SubscriptionRepository * subscriptionRepository);

    /**
        Destructs a SubscriptionTable instance.
     */
    ~SubscriptionTable ();

    /**
        Inserts entries (or updates existing entries) for the specified
        subscription in the Active Subscriptions and Subscription Classes
        tables.

        @param   subscription            the subscription instance
        @param   providers               the list of providers
        @param   indicationSubclassNames the list of indication subclass names
                                         with the source namespace name
     */
    void insertSubscription (
        const CIMInstance & subscription,
        const Array <ProviderClassList> & providers,
        const Array <NamespaceClassList> & indicationSubclassNames);

    /**
        Updates an entry in the Active Subscriptions table to either add a
        provider to or remove a provider from the list of providers serving the
        subscription.

        @param   subscriptionPath        the subscription object path
        @param   provider                the provider to be added or removed
        @param   addProvider             indicates if adding or removing
                                         provider
     */
    void updateProviders (
        const CIMObjectPath & subscriptionPath,
        const ProviderClassList & provider,
        Boolean addProvider);

    /**
        Updates an entry in the Active Subscriptions table to either add a
        class to or remove a class from the list of indication subclasses served
        by a provider serving the subscription.

        @param   subscriptionPath        the subscription object path
        @param   provider                the provider
        @param   nameSpace               namespace to add or remove
        @param   className               the class to be added or removed
     */
    void updateClasses (
        const CIMObjectPath & subscriptionPath,
        const CIMInstance & provider,
        const CIMNamespaceName &nameSpace,
        const CIMName & className);

    /**
        Removes entries (or updates entries) for the specified subscription in
        the Active Subscriptions and Subscription Classes tables.

        @param   subscription            the subscription instance
        @param   indicationSubclassNames the list of indication subclass names
                                         with the source namespace name
        @param   providers               the list of providers that had been
                                         serving the subscription
     */
    void removeSubscription (
        const CIMInstance & subscription,
        const Array <NamespaceClassList>& indicationSubclassNames,
        const Array <ProviderClassList> & providers);

    /**
        Retrieves the Active Subscriptions table entry for the specified
        subscription.  If this function returns False, the value of tableValue
        is not changed.

        @param   subscriptionPath      the object path of the subscription
        @param   tableValue            the retrieved table entry

        @return   True, if the specified subscription table entry was found;
                  False otherwise
     */
    Boolean getSubscriptionEntry (
        const CIMObjectPath & subscriptionPath,
        ActiveSubscriptionsTableEntry & tableValue) const;

    /**
        Retrieves list of enabled subscription instances in the specified
        namespaces, where the subscription indication class matches or is a
        superclass of the supported class.  If the checkProvider parameter
        value is True, a subscription is only included in the list returned if
        the specified provider accepted the subscription.  If the checkProvider
        parameter value is False, the provider parameter is not used (ignored).

        @param   supportedClass       the supported class
        @param   nameSpaces           the list of supported namespaces
        @param   checkProvider        indicates whether provider acceptance is
                                          checked
        @param   provider             the provider (used if checkProvider True)

        @return   list of subscriptions with the source namespace
     */
    Array <SubscriptionWithSrcNamespace> getMatchingSubscriptions (
        const CIMName & supportedClass,
        const Array <CIMNamespaceName> nameSpaces,
        const Boolean checkProvider = false,
        const CIMInstance & provider = CIMInstance ()) const;

    /**
        Retrieves list of enabled subscription instances in all namespaces,
        that are served by the specified provider.  This function is called
        when a provider is disabled.  In the Active Subscriptions table, the
        specified provider is removed from the list of providers serving the
        subscription.

        Note: this method may call the SubscriptionRepository
        reconcileFatalError() method, which may call the CIMRepository
        modifyInstance() or deleteInstance() method, while a WriteLock is held
        on the Active Subscriptions table.  The determination of whether the
        SubscriptionRepository reconcileFatalError() method must be called (and
        whether the repository must be updated) requires a lookup of the Active
        Subscriptions table to see if any other providers are serving the
        subscription.  The SubscriptionRepository reconcileFatalError() method
        does not need to access the repository to determine the subscription
        policy, but if the policy is Disable or Remove, it does need to call the
        CIMRepository modifyInstance() or deleteInstance() method.  The return
        value from the SubscriptionRepository reconcileFatalError() method (True
        if the subscription was successfully disabled or removed) in turn
        determines whether the entry in the Active Subscriptions table must be
        updated or removed.

        @param   provider          the provider instance

        @return   list of CIMInstance subscriptions
     */
    Array <CIMInstance> reflectProviderDisable (
        const CIMInstance & provider);

    /**
        Retrieves a list of subscription table entries for subscriptions that
        were being served by providers in the specified provider module, where
        the subscription creator matches the specified provider module userName.
        The Active Subscriptions table is updated to reflect the fact that these
        subscriptions are no longer being served by the provider(s) in the
        failed provider module.
        This method is called when notification is received that the specified
        provider module has failed.  In the Active Subscriptions table, any
        provider in the specified provider module is removed from the list of
        providers serving the subscription.
        The returned list of Active Subscriptions table entries includes for
        each subscription only those providers that are in the specified
        (failed) provider module and therefore are no longer serving the
        subscription.

        @param    moduleName             the provider module name
        @param    userName               the user name for the context in which
                                           the provider module was invoked
        @param    authenticationEnabled  indicates whether authentication is
                                           currently enabled

        @return   list of ActiveSubscriptionsTableEntry structs including the
                  subscriptions served by providers in the specified provider
                  module
     */
    Array <ActiveSubscriptionsTableEntry> reflectProviderModuleFailure (
        const String & moduleName,
        const String & userName,
        Boolean authenticationEnabled);

    /**
        Determines if the specified provider is in the list of providers
        serving the subscription.

        @param   provider              the provider instance
        @param   tableValue            the Active Subscriptions Table entry
        @param   nameSpace             namespace of the provider
        @return  The index of the provider in the list, if found;
                 PEG_NOT_FOUND otherwise
    */
    Uint32 providerInList
        (const CIMInstance & provider,
         const ActiveSubscriptionsTableEntry & tableValue,
         const CIMNamespaceName &nameSpace = CIMNamespaceName()) const;

    /**
        Determines if the specified class is in the list of indication
        subclasses served by the specified provider, serving the subscription.

        @param   className             the class name
        @param   nameSpace             namespace to lookup
        @param   providerClasses       a provider serving the subscription,
                                           with the indication classes served

        @return  The index of the class name in the list, if found;
                 PEG_NOT_FOUND otherwise
    */
    Uint32 classInList
        (const CIMName & className,
         const CIMNamespaceName &nameSpace,
         const ProviderClassList & providerClasses) const;

    /**
        Removes all entries from the Active Subscriptions and Subscription
        Classes tables.
     */
    void clear ();

    /**
        Retrieves list of enabled subscription instances in the specified
        namespace, where the subscription indication class matches or is a
        superclass of the supported class. A subscription is only included
        in the list returned if the specified provider accepted the
        subscription.

        @param   supportedClass      The supported class
        @param   nameSpace           The specified namespace
        @param   provider            The provider instance which accepts
                                     subscriptions
        @param   subscriptions       Output Array of subscription instances
        @param   subscriptionKeys    Output Array of associated subscription
                                     keys
     */
    void getMatchingClassNamespaceSubscriptions(
        const CIMName & supportedClass,
        const CIMNamespaceName&  nameSpace,
        const CIMInstance & provider,
        Array<CIMInstance>& matchingSubscriptions,
        Array<SubscriptionKey>& matchingSubscriptionKeys);

    /**
        Returns all the Active Subscriptions table entries.

        @return An Array containing the complete list of
            ActiveSubscriptionsTable entries.
     */
    Array<ActiveSubscriptionsTableEntry> getAllActiveSubscriptionEntries();

#ifdef PEGASUS_ENABLE_INDICATION_COUNT
    /**
        Updates entries in the Active Subscriptions table to increase
        matched indication counts for a specified provider which serves the
        subscriptions.

        @param provider A PG_Provider instance representing the provider that
            serves the subscriptions.
        @param subscriptionsKeys The keys of matched subscriptions which
            are served by the provider.
     */
    void updateMatchedIndicationCounts(
        const CIMInstance& providerInstance,
        const Array<SubscriptionKey>& subscriptionsKeys);

    /**
        Enumerates PG_SubscriptionIndicationData instances using the data
        stored in the Active Subscriptions table.

        @return All the PG_SubscriptionIndicationData instances.
    */
    Array<CIMInstance> enumerateSubscriptionIndicationDataInstances();

    /**
        Enumerates PG_SubscriptionIndicationData instance names using the data
        stored in the Active Subscriptions table.

        @return All the PG_SubscriptionIndicationData instanceName.
    */
    Array<CIMObjectPath> enumerateSubscriptionIndicationDataInstanceNames();

    /**
        Gets the PG_SubscriptionIndicationData instance for the specified CIM
        object path.

        @param instanceName CIMObjectpath specifies a CIM instance to be
            returned
        @return The specified PG_SubscriptionIndicationData instance.
                If the specified instance does not exist, throw a
                CIMObjectNotFoundException
    */
    CIMInstance getSubscriptionIndicationDataInstance(
        const CIMObjectPath& instanceName);
#endif

private:

    /**  Copy Constructor just to avoid the compiler provided one.
     */
    SubscriptionTable ( const SubscriptionTable& subscriptionTable );

    /** Overloaded Assignment Operator
     */
    SubscriptionTable& operator=( const SubscriptionTable& rhs );

    /**
        Generates a unique String key for the Active Subscriptions table
        from the subscription object path.

        @param   subscription          the subscription object path

        @return  the generated key
     */
    SubscriptionKey _generateActiveSubscriptionsKey (
        const CIMObjectPath & subscription) const;

    /**
        Locks the _activeSubscriptionsTableLock for read access and looks
        up an entry in the Active Subscriptions table.

        @param   key                   the hash table key
        @param   tableEntry            the table entry retrieved

        @return  true if the key is found in the table; false otherwise
     */
    Boolean _lockedLookupActiveSubscriptionsEntry (
        const SubscriptionKey & key,
        ActiveSubscriptionsTableEntry & tableEntry) const;

    /**
        Inserts an entry into the Active Subscriptions table.  The caller
        must first lock the _activeSubscriptionsTableLock for write access.

        @param   subscription          the subscription instance
        @param   providers             the list of providers
     */
    void _insertActiveSubscriptionsEntry (
        const CIMInstance & subscription,
        const Array <ProviderClassList> & providers);

    /**
        Removes an entry from the Active Subscriptions table.  The caller
        must first lock the _activeSubscriptionsTableLock for write access.

        @param   key                   the key of the entry to remove
     */
    void _removeActiveSubscriptionsEntry (
        const SubscriptionKey & key);

    /**
        Generates a unique String key for the Subscription Classes table from
        the indication class name and source namespace name.

        @param   indicationClassName   the indication class name
        @param   sourceNamespaceName   the source namespace name

        @return  the generated key
     */
    String _generateSubscriptionClassesKey (
        const CIMName & indicationClassName,
        const CIMNamespaceName & sourceNamespaceName) const;

    /**
        Locks the _subscriptionClassesTableLock for read access and looks
        up an entry in the Subscription Classes table.

        @param   key                   the hash table key
        @param   tableEntry            the table entry retrieved

        @return  true if the key is found in the table; false otherwise
     */
    Boolean _lockedLookupSubscriptionClassesEntry (
        const String & key,
        SubscriptionClassesTableEntry & tableEntry) const;

    /**
        Inserts an entry into the Subscription Classes table.  The caller must
        first lock the _subscriptionClassesTableLock for write access.

        @param   indicationClassName   the indication class name
        @param   sourceNamespaceName   the source namespace name
        @param   subscriptions         the list of subscription instances
     */
    void _insertSubscriptionClassesEntry (
        const CIMName & indicationClassName,
        const CIMNamespaceName & sourceNamespaceName,
        const Array <CIMInstance> & subscriptions);

    /**
        Removes an entry from the Subscription Classes table.  The caller must
        first lock the _subscriptionClassesTableLock for write access.

        @param   key                   the key of the entry to remove
     */
    void _removeSubscriptionClassesEntry (
        const String & key);

    /**
        Updates the providers for an entry in the Active Subscriptions table.
        The caller must first lock the _activeSubscriptionsTableLock for write
        access.

        @param   activeSubscriptionsKey  the key of the entry to update
        @param   subscription            the subscription instance for the entry
        @param   updatedProviderList     the updated list of providers for the
                                           entry (may be empty)
     */
    void _updateSubscriptionProviders (
        const SubscriptionKey & activeSubscriptionsKey,
        const CIMInstance & subscription,
        const Array <ProviderClassList> & updatedProviderList);

#ifdef PEGASUS_ENABLE_INDICATION_COUNT
    /**
        Gets filter name and handler name from the specified subscription.
        The format of filter name is namespace:filtername where the
        namespace is the namespace of filter instance created, and
        the filtername is the value of property Name in the filter instance.
        The format of handler name is namespace:classname.handlername where
        namespace is the namespace of handler instance created, the classname
        is the class of the handler instance, and handlername is the value
        of property Name in the handler instance.

        @param   subscription   Input subscription instance used to get
                                filterName and handlerName
        @param   filterName     Output string containing the colon-separated
                                the namespace of filter instance created and
                                the value of property Name in the filter
                                instance
        @param   handlerName    Output string containing the colon-separated
                                the namespace of handler instance created and
                                the class of the handler instance with the
                                dot-connected the value of property Name in
                                the Handler instance
     */
    void _getFilterAndHandlerNames(
        const CIMInstance& subscription,
        String& filterName,
        String& handlerName);

    /**
        Gets handler name, filter name, source namespace, provider module name,
        and provider name from a specified PG_SubscriptionIndicationData
        instanceName

        @param instanceName Input subscription indication data instance
            object path used to get filterName, handlerName, sourceNamespace,
            providerModuleName, and providerName.
        @param filterName  Output string containing the colon-separated
            the namespace of filter instance created and the value of property
            Name in the filter instance.
        @param handlerName  Output string containing the colon-separated the
            the namespace of handler instance created and the class of the
            handler instance with the dot-connected the value of property Name
            in the handler instance.
        @param sourceNS Output string containing source namespace of the
            subscription.
        @param providerModuleName Output string containing the provider module
            name.
        @param providerName Output string containing the provider name.
     */
    void _getSubscriptionIndicationDataKeys(
        const CIMObjectPath& instanceName,
        String& filterName,
        String& handlerName,
        String& sourceNS,
        String& providerModuleName,
        String& providerName);

    /**
        Builds the filter object path by using the specified filterName.

        @param filterName Input string containing the colon-separated
            the namespace of filter instance created and the value of property
            Name in the filter instance.
        @return The created filter object path.
     */
    CIMObjectPath _buildFilterPath(const String& filterName);

    /**
        Builds the handler object path by using the specified handlerName.

        @param handlerName Input string containing the colon-separated the
            namespace of handler instance created and the class of the handler
            instance with the dot-connected the value of property Name in
            the handler instance
        @return The created handler object path.
     */
    CIMObjectPath _buildHandlerPath(const String& handlerName);

    /**
        Builds the subscription object path by using the specified filterName,
            handlerName, and sourceNS.

        @param filterName Input string containing the colon-separated
            the namespace of filter instance created and the value of property
            Name in the filter instance.
        @param handlerName Input string containing the colon-separated the
            namespace of handler instance created and the class of the handler
            instance with the dot-connected the value of property Name in
            the handler instance.
        @param sourceNS Input string containing source namespace of the
            subscription.
        @return The created subscription object path.
     */
    CIMObjectPath _buildSubscriptionPath(
        const String& filterName,
        const String& handlerName,
        const String& sourceNS);

    /**
        Creates the subscription indication data instance by using the
            specified filterName, handlerName, sourceNS, providerModuleName,
            providerName, and  matchedIndicationCount.

        @param filterName Input string containing the colon-separated
            the namespace of filter instance created and the value of property
            Name in the filter instance.
        @param handlerName Input string containing the colon-separated the
            namespace of handler instance created and the class of the handler
            instance with the dot-connected the value of property Name in
            the handler instance.
        @param sourceNS Input string containing source namespace of the
            subscription.
        @param providerModuleName The provider module name used to build the
            instance.
        @param providerName The provider name used to build the instance.
        @param matchedIndicationCount The matched indication count used to
            build the instance.
        @return The created subscription indication data instance.
     */
    CIMInstance _buildSubscriptionIndDataInstance(
        const String& filterName,
        const String& handlerName,
        const String& sourceNS,
        const String& providerModuleName,
        const String& providerName,
        Uint32 matchedIndicationCount);

    /**
        Builds the subscription indication data instance object path by using
            specified filterName, handlerName, sourceNS, providerModuleName,
            and providerName.

        @param filterName Input string containing the colon-separated
            the namespace of filter instance created and the value of property
            Name in the filter instance.
        @param handlerName Input string containing the colon-separated the
            namespace of handler instance created and the class of the handler
            instance with the dot-connected the value of property Name in
            the handler instance.
        @param sourceNS Input string containing source namespace of the
            subscription.
            the specified parameters.
        @param providerModuleName The provider module name used to build the
            instance path.
        @param providerName The provider name used to build the instance path.
        @return The created subscription indication data instance object path.
     */
    CIMObjectPath _buildSubscriptionIndDataInstanceName(
        const String& filterName,
        const String& handlerName,
        const String& sourceNS,
        const String& providerModuleName,
        const String& providerName);

    /**
        Builds the PG_Provider instance object path by using specified
            providerModuleName and providerName.
        @param providerModuleName The provider module name used to build the
            instance path.
        @param providerName The provider name used to build the instance path.
        @return The created PG_Provider instance object path.
     */
    CIMObjectPath _buildProviderPath(
        const String& providerModuleName,
        const String& providerName);
#endif

    /**
        Active Subscriptions information table.  Access to this table is
        controlled by the _activeSubscriptionsTableLock.
     */
    ActiveSubscriptionsTable _activeSubscriptionsTable;

    /**
        A lock to control access to the _activeSubscriptionsTable.  Before
        accessing the _activeSubscriptionsTable, one must first lock this for
        read access.  Before updating the _activeSubscriptionsTable, one must
        first lock this for write access.
     */
    mutable ReadWriteSem _activeSubscriptionsTableLock;

    /**
        Subscription Classes information table.  Access to this table is
        controlled by the _subscriptionClassesTableLock.
     */
    SubscriptionClassesTable _subscriptionClassesTable;

    /**
        A lock to control access to the _subscriptionClassesTable.  Before
        accessing the _subscriptionClassesTable, one must first lock this for
        read access.  Before updating the _subscriptionClassesTable, one must
        first lock this for write access.
     */
    mutable ReadWriteSem _subscriptionClassesTableLock;

    SubscriptionRepository * _subscriptionRepository;
};

PEGASUS_NAMESPACE_END

#endif  /* Pegasus_SubscriptionTable_h */
