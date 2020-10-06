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

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/Constants.h>
#include <Pegasus/Common/Tracer.h>

#include "IndicationConstants.h"
#include "IndicationService.h"
#include "SubscriptionTable.h"

#ifdef PEGASUS_ENABLE_INDICATION_COUNT
# include "ProviderIndicationCountTable.h"
#endif

PEGASUS_USING_STD;

PEGASUS_NAMESPACE_BEGIN

SubscriptionTable::SubscriptionTable (
    SubscriptionRepository * subscriptionRepository)
    : _subscriptionRepository (subscriptionRepository)
{
}

SubscriptionTable::~SubscriptionTable ()
{
}

Boolean SubscriptionTable::getSubscriptionEntry (
    const CIMObjectPath & subscriptionPath,
    ActiveSubscriptionsTableEntry & tableValue) const
{
    PEG_METHOD_ENTER (TRC_INDICATION_SERVICE,
        "SubscriptionTable::getSubscriptionEntry");

    Boolean succeeded = false;
    SubscriptionKey activeSubscriptionsKey =
        SubscriptionKey(subscriptionPath);

    if (_lockedLookupActiveSubscriptionsEntry
        (activeSubscriptionsKey, tableValue))
    {
        succeeded = true;
    }
    else
    {
        //
        //  Subscription not found in Active Subscriptions table
        //
        PEG_TRACE((TRC_INDICATION_SERVICE, Tracer::LEVEL1,
            "Subscription (%s) not found in ActiveSubscriptionsTable",
            (const char*)activeSubscriptionsKey.toString().getCString()));
    }

    PEG_METHOD_EXIT ();
    return succeeded;
}

Array <SubscriptionWithSrcNamespace>
    SubscriptionTable::getMatchingSubscriptions(
        const CIMName & supportedClass,
        const Array <CIMNamespaceName> nameSpaces,
        const Boolean checkProvider,
        const CIMInstance & provider) const
{
    PEG_METHOD_ENTER (TRC_INDICATION_SERVICE,
        "SubscriptionTable::getMatchingSubscriptions");

    Array <SubscriptionWithSrcNamespace> matchingSubscriptions;
    Array <CIMInstance> subscriptions;

    for (Uint32 i = 0; i < nameSpaces.size (); i++)
    {
        //
        //  Look up the indicationClass-sourceNamespace pair in the
        //  Subscription Classes table
        //
        String subscriptionClassesKey = _generateSubscriptionClassesKey
            (supportedClass, nameSpaces [i]);
        SubscriptionClassesTableEntry scTableValue;
        if (_lockedLookupSubscriptionClassesEntry(
                subscriptionClassesKey, scTableValue))
        {
            subscriptions = scTableValue.subscriptions;
            for (Uint32 j = 0; j < subscriptions.size (); j++)
            {
                Boolean match = true;

                if (checkProvider)
                {
                    //
                    //  Check if the provider who generated this indication
                    //  accepted this subscription
                    //
                    SubscriptionKey activeSubscriptionsKey =
                        SubscriptionKey(subscriptions [j].getPath ());
                    ActiveSubscriptionsTableEntry asTableValue;
                    if (_lockedLookupActiveSubscriptionsEntry(
                            activeSubscriptionsKey, asTableValue))
                    {
                        //
                        //  If provider is not in list, it did not accept the
                        //  subscription
                        //
                        if ((providerInList(provider, asTableValue)) ==
                                PEG_NOT_FOUND)
                        {
                            match = false;
                            break;
                        }
                    }
                }

                if (match)
                {
                    //
                    //  Add current subscription to list
                    //
                    SubscriptionWithSrcNamespace subscriptionWithSrcNamespace;
                    subscriptionWithSrcNamespace.nameSpace = nameSpaces[i];
                    subscriptionWithSrcNamespace.subscription =
                        subscriptions[j];
                    matchingSubscriptions.append (subscriptionWithSrcNamespace);
                }
            }
        }
    }

    PEG_METHOD_EXIT ();
    return matchingSubscriptions;
}

Array <CIMInstance> SubscriptionTable::reflectProviderDisable (
    const CIMInstance & provider)
{
    PEG_METHOD_ENTER (TRC_INDICATION_SERVICE,
        "SubscriptionTable::reflectProviderDisable");

    Array <CIMInstance> providerSubscriptions;

    //
    //  Iterate through the subscription table to find subscriptions served by
    //  the provider
    //  NOTE: updating entries (remove and insert) while iterating through the
    //  table does not work reliably, and it is not clear if that is supposed to
    //  work; for now, the SubscriptionTable first iterates through the
    //  active subscriptions table to find subscriptions served by the
    //  provider, then looks up and updates each affected subscription
    //
    {
        //
        //  Acquire and hold the write lock during the entire
        //  lookup/remove/insert process, allowing competing threads to apply
        //  their logic over a consistent view of the data.
        //  Do not call any other methods that need
        //  _activeSubscriptionsTableLock.
        //
        WriteLock lock (_activeSubscriptionsTableLock);

        for (ActiveSubscriptionsTable::Iterator i =
            _activeSubscriptionsTable.start (); i; i++)
        {
            //
            //  If provider matches, append subscription to the list
            //
            ActiveSubscriptionsTableEntry asTableValue = i.value();
            for (Uint32 j = 0; j < asTableValue.providers.size(); j++)
            {
                if (asTableValue.providers[j].provider.getPath().identical(
                        provider.getPath()))
                {
                    //
                    //  Add the subscription to the list
                    //
                    providerSubscriptions.append(asTableValue.subscription);
                    break;
                }
            }
        }

        //
        //  Look up and update hash table entry for each affected subscription
        //
        for (Uint32 k = 0; k < providerSubscriptions.size (); k++)
        {
            //
            //  Update the entry in the active subscriptions hash table
            //
            SubscriptionKey activeSubscriptionsKey =
                SubscriptionKey(providerSubscriptions [k].getPath ());
            ActiveSubscriptionsTableEntry asTableValue;
            if (_activeSubscriptionsTable.lookup(
                    activeSubscriptionsKey, asTableValue))
            {
                //
                //  Remove the provider from the list of providers serving the
                //  subscription
                //
                Uint32 providerIndex = providerInList(provider, asTableValue);
                if (providerIndex != PEG_NOT_FOUND)
                {
                    asTableValue.providers.remove(providerIndex);

                    _updateSubscriptionProviders(
                        activeSubscriptionsKey,
                        asTableValue.subscription,
                        asTableValue.providers);
                }
                else
                {
                    PEG_TRACE((TRC_INDICATION_SERVICE,Tracer::LEVEL2,
                        "Provider (%s) not found in list for Subscription (%s)"
                        " in ActiveSubscriptionsTable",
                        (const char*)provider.getPath().toString().getCString(),
                        (const char*)
                            activeSubscriptionsKey.toString().getCString()));
                }
            }
            else
            {
                PEG_TRACE((TRC_INDICATION_SERVICE,Tracer::LEVEL2,
                    "Subscription (%s) not found in ActiveSubscriptionsTable",
                    (const char*)
                         activeSubscriptionsKey.toString().getCString()));
                //
                //  The subscription may have been deleted in the mean time
                //  If so, no further update is required
                //
            }
        }
    }

    PEG_METHOD_EXIT ();
    return providerSubscriptions;
}

Array <ActiveSubscriptionsTableEntry>
SubscriptionTable::reflectProviderModuleFailure
    (const String & moduleName,
     const String & userName,
     Boolean authenticationEnabled)
{
    PEG_METHOD_ENTER (TRC_INDICATION_SERVICE,
        "SubscriptionTable::reflectProviderModuleFailure");

    Array <ActiveSubscriptionsTableEntry> providerModuleSubscriptions;

    //
    //  Iterate through the subscription table to find subscriptions served by
    //  a provider in the specified module, with the specified userName as the
    //  subscription creator
    //  NOTE: updating entries (remove and insert) while iterating through the
    //  table is not allowed
    //  The SubscriptionTable first iterates through the active subscriptions
    //  table to find matching subscriptions served by a provider in the
    //  specified module, then looks up and updates each affected subscription
    //
    {
        //
        //  Acquire and hold the write lock during the entire
        //  lookup/remove/insert process, allowing competing threads to apply
        //  their logic over a consistent view of the data.
        //  Do not call any other methods that need
        //  _activeSubscriptionsTableLock.
        //
        WriteLock lock (_activeSubscriptionsTableLock);

        for (ActiveSubscriptionsTable::Iterator i =
            _activeSubscriptionsTable.start (); i; i++)
        {
            ActiveSubscriptionsTableEntry asTableValue;
            //
            //  Get subscription creator
            //
            asTableValue = i.value();
            String creator;
            CIMValue creatorValue = asTableValue.subscription.getProperty(
                asTableValue.subscription.findProperty(
                    PEGASUS_PROPERTYNAME_INDSUB_CREATOR)).getValue();
            creatorValue.get (creator);

            Array <ProviderClassList> failedProviderList;
            for (Uint32 j = 0; j < asTableValue.providers.size(); j++)
            {
                //
                //  Get provider module name
                //
                String providerModuleName;
                CIMValue nameValue =
                    asTableValue.providers[j].providerModule.getProperty(
                        asTableValue.providers[j].providerModule.findProperty(
                            PEGASUS_PROPERTYNAME_NAME)).getValue();
                nameValue.get (providerModuleName);

                //
                //  Get module user context setting
                //
                Uint16 moduleContext = PEGASUS_DEFAULT_PROV_USERCTXT;
                CIMValue contextValue =
                    asTableValue.providers[j].providerModule.getProperty(
                        asTableValue.providers [j].providerModule.findProperty(
                            PEGASUS_PROPERTYNAME_MODULE_USERCONTEXT)).
                    getValue();
                if (!contextValue.isNull ())
                {
                    contextValue.get (moduleContext);
                }

                //
                //  If provider module name matches,
                //  add provider to the list of failed providers
                //
                if (providerModuleName == moduleName)
                {
                    //
                    //  If authentication is enabled, and module was run as
                    //  requestor, subscription creator must also match module
                    //  user context name, to add provider to the list of
                    //  failed providers
                    //
                    if ((moduleContext != PG_PROVMODULE_USERCTXT_REQUESTOR) ||
                        (!authenticationEnabled) || (creator == userName))
                    {
                        //
                        //  Add the provider to the list
                        //
                        failedProviderList.append(asTableValue.providers[j]);
                    }
                }  //  if provider module name matches
            }  //  for each subscription provider

            //
            //  If there were any failed providers, add the subscription
            //  entry to the list of affected subscriptions
            //
            if (failedProviderList.size () > 0)
            {
                ActiveSubscriptionsTableEntry subscription;
                subscription.subscription = asTableValue.subscription;
                subscription.providers = failedProviderList;
                providerModuleSubscriptions.append (subscription);
            }
        }

        //
        //  Look up and update hash table entry for each affected subscription
        //
        for (Uint32 k = 0; k < providerModuleSubscriptions.size (); k++)
        {
            //
            //  Update the entry in the active subscriptions hash table
            //
            SubscriptionKey activeSubscriptionsKey =
                SubscriptionKey(
                    providerModuleSubscriptions[k].subscription.getPath());
            ActiveSubscriptionsTableEntry asTableValue;
            if (_activeSubscriptionsTable.lookup(
                    activeSubscriptionsKey, asTableValue))
            {
                Array <ProviderClassList> updatedProviderList;
                for (Uint32 l = 0; l < asTableValue.providers.size(); l++)
                {
                    String providerModuleName;
                    CIMValue nameValue =
                        asTableValue.providers[l].providerModule.getProperty(
                            asTableValue.providers[l].providerModule.
                            findProperty(PEGASUS_PROPERTYNAME_NAME)).getValue();
                    nameValue.get (providerModuleName);
                    if (providerModuleName != moduleName)
                    {
                        //
                        //  Provider is not in the failed module
                        //  Append provider to list of providers still serving
                        //  the subscription
                        //
                        updatedProviderList.append(asTableValue.providers[l]);
                    }
                }

                _updateSubscriptionProviders(
                    activeSubscriptionsKey,
                    asTableValue.subscription,
                    updatedProviderList);
            }
        }
    }

    PEG_METHOD_EXIT ();
    return providerModuleSubscriptions;
}

Boolean SubscriptionTable::_lockedLookupActiveSubscriptionsEntry (
    const SubscriptionKey & key,
    ActiveSubscriptionsTableEntry & tableEntry) const
{
    ReadLock lock(_activeSubscriptionsTableLock);

    return (_activeSubscriptionsTable.lookup (key, tableEntry));
}

void SubscriptionTable::_insertActiveSubscriptionsEntry (
    const CIMInstance & subscription,
    const Array <ProviderClassList> & providers)
{
    PEG_METHOD_ENTER (TRC_INDICATION_SERVICE,
        "SubscriptionTable::_insertActiveSubscriptionsEntry");

    SubscriptionKey activeSubscriptionsKey =
        SubscriptionKey(subscription.getPath ());

    ActiveSubscriptionsTableEntry entry;
    entry.subscription = subscription;
    entry.providers = providers;

    //
    //  Insert returns true on success, false if duplicate key
    //
    PEGASUS_FCT_EXECUTE_AND_ASSERT(
        true,
        _activeSubscriptionsTable.insert(activeSubscriptionsKey, entry));

#ifdef PEGASUS_INDICATION_HASHTRACE
    String traceString;
    traceString.append (activeSubscriptionsKey);
    traceString.append (" Providers: ");
    for (Uint32 i = 0; i < providers.size (); i++)
    {
        String providerName = providers [i].provider.getProperty
            (providers [i].provider.findProperty
            (PEGASUS_PROPERTYNAME_NAME)).getValue ().toString ();
        traceString.append (providerName);
        traceString.append ("  Classes: ");
        for (Uint32 j = 0; j < providers[i].classList.size (); j++)
        {
             traceString.append (providers[i].classList[j].getString());
             traceString.append ("  ");
        }
    }

    PEG_TRACE((TRC_INDICATION_SERVICE, Tracer::LEVEL4,
        "INSERTED _activeSubscriptionsTable entry: %s",
        (const char*)traceString.getCString()));
#endif

    PEG_METHOD_EXIT ();
}

void SubscriptionTable::_removeActiveSubscriptionsEntry (
    const SubscriptionKey & key)
{
    PEG_METHOD_ENTER (TRC_INDICATION_SERVICE,
        "SubscriptionTable::_removeActiveSubscriptionsEntry");

    //
    //  Remove returns true on success, false if not found
    //
    PEGASUS_FCT_EXECUTE_AND_ASSERT(true,_activeSubscriptionsTable.remove(key));

#ifdef PEGASUS_INDICATION_HASHTRACE
    PEG_TRACE((TRC_INDICATION_SERVICE,Tracer::LEVEL4,
         "REMOVED _activeSubscriptionsTable entry: %s",
         (const char*)key.toString().getCString()));
#endif

    PEG_METHOD_EXIT ();
}

String SubscriptionTable::_generateSubscriptionClassesKey (
    const CIMName & indicationClassName,
    const CIMNamespaceName & sourceNamespaceName) const
{
    String subscriptionClassesKey;

    //
    //  Append indication class name to key
    //
    subscriptionClassesKey.append (indicationClassName.getString ());

    //
    //  Append source namespace name to key
    //
    subscriptionClassesKey.append (sourceNamespaceName.getString ());

    return subscriptionClassesKey;
}

Boolean SubscriptionTable::_lockedLookupSubscriptionClassesEntry (
    const String & key,
    SubscriptionClassesTableEntry & tableEntry) const
{
    ReadLock lock(_subscriptionClassesTableLock);

    return (_subscriptionClassesTable.lookup (key, tableEntry));
}

void SubscriptionTable::_insertSubscriptionClassesEntry (
    const CIMName & indicationClassName,
    const CIMNamespaceName & sourceNamespaceName,
    const Array <CIMInstance> & subscriptions)
{
    PEG_METHOD_ENTER (TRC_INDICATION_SERVICE,
        "SubscriptionTable::_insertSubscriptionClassesEntry");

    String subscriptionClassesKey = _generateSubscriptionClassesKey
        (indicationClassName, sourceNamespaceName);
    SubscriptionClassesTableEntry entry;
    entry.indicationClassName = indicationClassName;
    entry.sourceNamespaceName = sourceNamespaceName;
    entry.subscriptions = subscriptions;

    //
    //  Insert returns true on success, false if duplicate key
    //
    PEGASUS_FCT_EXECUTE_AND_ASSERT(
        true,
        _subscriptionClassesTable.insert(subscriptionClassesKey, entry));

#ifdef PEGASUS_INDICATION_HASHTRACE
    String traceString;
    traceString.append (subscriptionClassesKey);
    traceString.append (" Subscriptions: ");
    for (Uint32 i = 0; i < subscriptions.size (); i++)
    {
        traceString.append (subscriptions [i].getPath ().toString());
        traceString.append ("  ");
    }

    PEG_TRACE((TRC_INDICATION_SERVICE, Tracer::LEVEL4,
        "INSERTED _subscriptionClassesTable entry: %s",
        (const char*)traceString.getCString()));
#endif

    PEG_METHOD_EXIT ();
}

void SubscriptionTable::_removeSubscriptionClassesEntry (
    const String & key)
{
    PEG_METHOD_ENTER (TRC_INDICATION_SERVICE,
        "SubscriptionTable::_removeSubscriptionClassesEntry");

    //
    //  Remove returns true on success, false if not found
    //
    PEGASUS_FCT_EXECUTE_AND_ASSERT(
        true,
        _subscriptionClassesTable.remove(key));

#ifdef PEGASUS_INDICATION_HASHTRACE
    PEG_TRACE((TRC_INDICATION_SERVICE, Tracer::LEVEL4,
        "REMOVED _subscriptionClassesTable entry: %s",
        (const char*)key.getCString()));
#endif

    PEG_METHOD_EXIT ();
}

void SubscriptionTable::_updateSubscriptionProviders
    (const SubscriptionKey & activeSubscriptionsKey,
     const CIMInstance & subscription,
     const Array <ProviderClassList> & updatedProviderList)
{
    PEG_METHOD_ENTER (TRC_INDICATION_SERVICE,
        "SubscriptionTable::_updateSubscriptionProviders");

    if (updatedProviderList.size () > 0)
    {
        //
        //  At least one provider is still serving the
        //  subscription
        //  Update entry in Active Subscriptions table
        //
        _removeActiveSubscriptionsEntry (activeSubscriptionsKey);
        _insertActiveSubscriptionsEntry (subscription, updatedProviderList);
    }
    else
    {
        //
        //  The disabled or failed provider(s) was (were) the only provider(s)
        //  serving the subscription
        //  Implement the subscription's On Fatal Error Policy
        //
        Boolean removedOrDisabled =
            _subscriptionRepository->reconcileFatalError (subscription);
        _removeActiveSubscriptionsEntry (activeSubscriptionsKey);
        if (!removedOrDisabled)
        {
            //
            //  If subscription was not disabled or deleted
            //  Update entry in Active Subscriptions table
            //  Note that in this case the updatedProviderList is
            //  empty - no providers are serving the subscription
            //  currently
            //
            _insertActiveSubscriptionsEntry (subscription,
                updatedProviderList);
        }
        else
        {
            // Delete subscription entries from SubscriptionClassesTable.
            WriteLock lock(_subscriptionClassesTableLock);
            Array<SubscriptionClassesTableEntry> scTableValues;
            for (SubscriptionClassesTable::Iterator i =
                _subscriptionClassesTable.start(); i; i++)
            {
                SubscriptionClassesTableEntry value = i.value();
                for (Uint32 j = 0, n = value.subscriptions.size(); j < n; ++j)
                {
                    if (value.subscriptions[j].getPath().identical(
                        subscription.getPath()))
                    {
                        value.subscriptions.remove(j);
                        scTableValues.append(value);
                        break;
                    }
                }
            }
            for (Uint32 i = 0, n = scTableValues.size(); i < n; ++i)
            {
                String subscriptionClassesKey = _generateSubscriptionClassesKey(
                    scTableValues[i].indicationClassName,
                    scTableValues[i].sourceNamespaceName);
                // If this is the only subscription for this class-namespace
                // pair delete the entry else update the subscription list
                // for this class-namespace pair.
                if (scTableValues[i].subscriptions.size())
                {
                    SubscriptionClassesTableEntry *entry = 0;
                    _subscriptionClassesTable.lookupReference(
                        subscriptionClassesKey,
                        entry);
                    PEGASUS_ASSERT(entry);
                    entry->subscriptions = scTableValues[i].subscriptions;
                }
                else
                {
                    _removeSubscriptionClassesEntry(subscriptionClassesKey);
                }
            }
        }
    }

    PEG_METHOD_EXIT ();
}

void SubscriptionTable::insertSubscription (
    const CIMInstance & subscription,
    const Array <ProviderClassList> & providers,
    const Array <NamespaceClassList> & indicationSubclassNames)
{
    PEG_METHOD_ENTER (TRC_INDICATION_SERVICE,
        "SubscriptionTable::insertSubscription");

    //
    //  Insert entry into active subscriptions table
    //
    {
        WriteLock lock(_activeSubscriptionsTableLock);

        _insertActiveSubscriptionsEntry(subscription, providers);
    }

    //
    //  Insert or update entries in subscription classes table
    //
    {
        //
        //  Acquire and hold the write lock during the entire
        //  lookup/remove/insert process, allowing competing threads to apply
        //  their logic over a consistent view of the data.
        //  Do not call any other methods that need
        //  _subscriptionClassesTableLock.
        //
        WriteLock lock (_subscriptionClassesTableLock);
        for (Uint32 i = 0; i < indicationSubclassNames.size (); i++)
        {
            for (Uint32 j = 0, n = indicationSubclassNames[i].classList.size();
                j < n; ++j)
            {
                String subscriptionClassesKey = _generateSubscriptionClassesKey(
                    indicationSubclassNames[i].classList[j],
                    indicationSubclassNames[i].nameSpace);

                SubscriptionClassesTableEntry scTableValue;
                if (_subscriptionClassesTable.lookup(
                    subscriptionClassesKey, scTableValue))
                {
                    //
                    //  If entry exists for this IndicationClassName-
                    //  SourceNamespace pair, remove old entry and insert
                    //  new entry
                    //
                    Array<CIMInstance> subscriptions =
                        scTableValue.subscriptions;
                    subscriptions.append (subscription);
                    _removeSubscriptionClassesEntry (subscriptionClassesKey);
                    _insertSubscriptionClassesEntry (
                        indicationSubclassNames[i].classList[j],
                        indicationSubclassNames[i].nameSpace,
                        subscriptions);
                }
                else
                {
                    //
                    //  If no entry exists for this
                    //  IndicationClassName-SourceNamespace pair, insert new
                    //  entry
                    //
                    Array <CIMInstance> subscriptions;
                    subscriptions.append (subscription);
                   _insertSubscriptionClassesEntry (
                       indicationSubclassNames[i].classList[j],
                       indicationSubclassNames[i].nameSpace,
                       subscriptions);
                }
            }
        }
    }

    PEG_METHOD_EXIT ();
}

void SubscriptionTable::updateProviders (
    const CIMObjectPath & subscriptionPath,
    const ProviderClassList & provider,
    Boolean addProvider)
{
    PEG_METHOD_ENTER (TRC_INDICATION_SERVICE,
        "SubscriptionTable::updateProviders");

    SubscriptionKey activeSubscriptionsKey =
        SubscriptionKey(subscriptionPath);

    ActiveSubscriptionsTableEntry asTableValue;
    {
        //
        //  Acquire and hold the write lock during the entire
        //  lookup/remove/insert process, allowing competing threads to apply
        //  their logic over a consistent view of the data.
        //  Do not call any other methods that need
        //  _activeSubscriptionsTableLock.
        //
        WriteLock lock (_activeSubscriptionsTableLock);
        if (_activeSubscriptionsTable.lookup(
                activeSubscriptionsKey, asTableValue))
        {
            Uint32 providerIndex =
                providerInList(provider.provider, asTableValue);
            if (addProvider)
            {
                if (providerIndex == PEG_NOT_FOUND)
                {
                    asTableValue.providers.append(provider);
                    _removeActiveSubscriptionsEntry(activeSubscriptionsKey);
                    _insertActiveSubscriptionsEntry(
                        asTableValue.subscription,
                        asTableValue.providers);
                }
                else
                {
                    CIMInstance p = provider.provider;
                    PEG_TRACE((TRC_INDICATION_SERVICE,Tracer::LEVEL2,
                        "Provider %s already in list for Subscription (%s) "
                        "in ActiveSubscriptionsTable",
                        (const char*)IndicationService::getProviderLogString(p)
                        .getCString(),
                        (const char*)
                            activeSubscriptionsKey.toString().getCString()));
                }
            }
            else
            {
                if (providerIndex != PEG_NOT_FOUND)
                {
                    asTableValue.providers.remove(providerIndex);
                    _removeActiveSubscriptionsEntry(activeSubscriptionsKey);
                    _insertActiveSubscriptionsEntry(
                        asTableValue.subscription,
                        asTableValue.providers);
                }
                else
                {
                    CIMInstance p = provider.provider;
                    PEG_TRACE((TRC_INDICATION_SERVICE,Tracer::LEVEL2,
                        "Provider %s not found in list for Subscription (%s) "
                        "in ActiveSubscriptionsTable",
                        (const char*)IndicationService::getProviderLogString(p)
                        .getCString(),
                        (const char*)
                            activeSubscriptionsKey.toString().getCString()));
                }
            }
        }
        else
        {
            PEG_TRACE((TRC_INDICATION_SERVICE, Tracer::LEVEL2,
                "Subscription (%s) not found in ActiveSubscriptionsTable",
                (const char*)activeSubscriptionsKey.toString().getCString()));

            //
            //  The subscription may have been deleted in the mean time
            //  If so, no further update is required
            //
        }
    }

    PEG_METHOD_EXIT ();
}

void SubscriptionTable::updateClasses (
    const CIMObjectPath & subscriptionPath,
    const CIMInstance & provider,
    const CIMNamespaceName &nameSpace,
    const CIMName & className)
{
    PEG_METHOD_ENTER (TRC_INDICATION_SERVICE,
        "SubscriptionTable::updateClasses");

    SubscriptionKey activeSubscriptionsKey = SubscriptionKey(subscriptionPath);
    ActiveSubscriptionsTableEntry asTableValue;

    {
        //
        //  Acquire and hold the write lock during the entire
        //  lookup/remove/insert process, allowing competing threads to apply
        //  their logic over a consistent view of the data.
        //  Do not call any other methods that need
        //  _activeSubscriptionsTableLock.
        //
        WriteLock lock (_activeSubscriptionsTableLock);
        if (_activeSubscriptionsTable.lookup(
                activeSubscriptionsKey, asTableValue))
        {
            Uint32 providerIndex = providerInList(provider, asTableValue);
            if (providerIndex != PEG_NOT_FOUND)
            {
                Uint32 classIndex = classInList(
                    className,
                    nameSpace,
                    asTableValue.providers[providerIndex]);
                if (classIndex == PEG_NOT_FOUND)
                {
                    for (Uint32 i = 0,
                        n = asTableValue.
                            providers[providerIndex].classList.size();
                        i < n; ++i)
                    {
                        if (asTableValue.providers[providerIndex].
                            classList[i].nameSpace == nameSpace)
                        {
                            asTableValue.providers[providerIndex].classList[i].
                                classList.append(className);
                            break;
                        }
                    }
                }
                else //  classIndex != PEG_NOT_FOUND
                {

                    for (Uint32 i = 0,
                        n = asTableValue.
                            providers[providerIndex].classList.size();
                        i < n; ++i)
                    {
                        if (asTableValue.providers[providerIndex].
                            classList[i].nameSpace == nameSpace)
                        {
                            asTableValue.providers[providerIndex].classList[i].
                                classList.remove(classIndex);
                            break;
                        }
                    }
                }

                _removeActiveSubscriptionsEntry(activeSubscriptionsKey);
                _insertActiveSubscriptionsEntry(
                    asTableValue.subscription,
                    asTableValue.providers);
            }
            else
            {
                PEG_TRACE((TRC_INDICATION_SERVICE,Tracer::LEVEL2,
                    "Provider (%s) not found in list for Subscription (%s) in "
                    "ActiveSubscriptionsTable",
                    (const char*)provider.getPath().toString().getCString(),
                    (const char*)
                        activeSubscriptionsKey.toString().getCString()));
            }
        }
        else
        {
            //
            //  Subscription not found in Active Subscriptions table
            //
        }
    }

    PEG_METHOD_EXIT ();
}

void SubscriptionTable::removeSubscription (
    const CIMInstance & subscription,
    const Array <NamespaceClassList> & indicationSubclassNames,
    const Array <ProviderClassList> & providers)
{
    PEG_METHOD_ENTER (TRC_INDICATION_SERVICE,
        "SubscriptionTable::removeSubscription");

    //
    //  Remove entry from active subscriptions table
    //
    {
        WriteLock lock(_activeSubscriptionsTableLock);

        _removeActiveSubscriptionsEntry(
            SubscriptionKey(subscription.getPath()));
    }

    //
    //  Remove or update entries in subscription classes table
    //
    {
        //
        //  Acquire and hold the write lock during the entire
        //  lookup/remove/insert process, allowing competing threads to apply
        //  their logic over a consistent view of the data.
        //  Do not call any other methods that need
        //  _subscriptionClassesTableLock.
        //
        WriteLock lock (_subscriptionClassesTableLock);
        for (Uint32 i = 0; i < indicationSubclassNames.size (); i++)
        {
            for (Uint32 k = 0, n = indicationSubclassNames[i].classList.size();
                k < n; ++k)
            {
                String subscriptionClassesKey = _generateSubscriptionClassesKey(
                    indicationSubclassNames[i].classList[k],
                    indicationSubclassNames[i].nameSpace);
                SubscriptionClassesTableEntry scTableValue;
                if (_subscriptionClassesTable.lookup(
                    subscriptionClassesKey, scTableValue))
                {
                    //
                    //  If entry exists for this IndicationClassName-
                    //  -SourceNamespace pair, remove subscription from the list
                    //
                    Array<CIMInstance> subscriptions =
                        scTableValue.subscriptions;
                    for (Uint32 j = 0; j < subscriptions.size (); j++)
                    {
                        if (subscriptions [j].getPath().identical
                           (subscription.getPath()))
                        {
                            subscriptions.remove (j);
                        }
                    }

                    //
                    //  Remove the old entry
                    //
                    _removeSubscriptionClassesEntry (subscriptionClassesKey);

                    //
                    //  If there are still subscriptions in the list, insert the
                    //  new entry
                    //
                    if (subscriptions.size () > 0)
                    {
                        _insertSubscriptionClassesEntry (
                        indicationSubclassNames[i].classList[k],
                        indicationSubclassNames[i].nameSpace,
                        subscriptions);
                    }
                }
                else
                {
                    //
                    //  Entry not found in Subscription Classes table
                    //
                    PEG_TRACE((TRC_INDICATION_SERVICE,Tracer::LEVEL2,
                        "Indication subclass and namespace (%s) not found "
                        "in SubscriptionClassesTable",
                        (const char*)subscriptionClassesKey.getCString()));
                }
            }
        }
    }

    PEG_METHOD_EXIT ();
}

Uint32 SubscriptionTable::providerInList
    (const CIMInstance & provider,
     const ActiveSubscriptionsTableEntry & tableValue,
     const CIMNamespaceName &nameSpace) const
{
    PEG_METHOD_ENTER (TRC_INDICATION_SERVICE,
        "SubscriptionTable::providerInList");

    //
    //  Look for the provider in the list
    //
    for (Uint32 i = 0; i < tableValue.providers.size (); i++)
    {
        if (tableValue.providers [i].provider.getPath ().identical
            (provider.getPath ()))
        {
            if (nameSpace.isNull())
            {
                PEG_METHOD_EXIT ();
                return i;
            }
            for (Uint32 j = 0; j < tableValue.providers[i].classList.size();
                ++j)
            {
                if (tableValue.providers[i].classList[j].nameSpace
                    == nameSpace)
                {
                    PEG_METHOD_EXIT ();
                    return i;
                }
            }
        }
    }

    PEG_METHOD_EXIT ();
    return PEG_NOT_FOUND;
}


Uint32 SubscriptionTable::classInList
    (const CIMName & className,
     const CIMNamespaceName &nameSpace,
     const ProviderClassList & providerClasses) const
{
    PEG_METHOD_ENTER (TRC_INDICATION_SERVICE, "SubscriptionTable::classInList");

    //
    //  Look for the class in the list
    //
    for (Uint32 i = 0; i < providerClasses.classList.size (); i++)
    {
        if (providerClasses.classList[i].nameSpace == nameSpace)
        {
            for (Uint32 j = 0;
                j < providerClasses.classList[i].classList.size(); i++)
            {
                if (providerClasses.classList[i].classList[j].equal(className))
                {
                    PEG_METHOD_EXIT ();
                    return i;
                }
            }
        }
    }

    PEG_METHOD_EXIT ();
    return PEG_NOT_FOUND;
}

void SubscriptionTable::clear ()
{
    PEG_METHOD_ENTER (TRC_INDICATION_SERVICE, "SubscriptionTable::clear");

    {
        WriteLock lock (_activeSubscriptionsTableLock);
        _activeSubscriptionsTable.clear ();
    }
    {
        WriteLock lock (_subscriptionClassesTableLock);
        _subscriptionClassesTable.clear ();
    }

    PEG_METHOD_EXIT ();
}

void SubscriptionTable::getMatchingClassNamespaceSubscriptions(
    const CIMName & supportedClass,
    const CIMNamespaceName & nameSpace,
    const CIMInstance& provider,
    Array<CIMInstance>& matchingSubscriptions,
    Array<SubscriptionKey>& matchingSubscriptionKeys)
{
    PEG_METHOD_ENTER (TRC_INDICATION_SERVICE,
        "SubscriptionTable::getMatchingClassNamespaceSubscriptions");

    Array <CIMInstance> subscriptions;
    matchingSubscriptions.clear();
    matchingSubscriptionKeys.clear();

    //
    //  Look up the indicationClass-sourceNamespace pair in the
    //  Subscription Classes table
    //
    String subscriptionClassesKey =
        _generateSubscriptionClassesKey(supportedClass, nameSpace);

    SubscriptionClassesTableEntry scTableValue;
    if (_lockedLookupSubscriptionClassesEntry(
            subscriptionClassesKey, scTableValue))
    {
        subscriptions = scTableValue.subscriptions;
        for (Uint32 j = 0; j < subscriptions.size (); j++)
        {
            //
            //  Check if the provider who generated this indication
            //  accepted this subscription
            //
            SubscriptionKey activeSubscriptionsKey =
                SubscriptionKey(subscriptions [j].getPath ());
            ActiveSubscriptionsTableEntry asTableValue;
            if (_lockedLookupActiveSubscriptionsEntry(
                    activeSubscriptionsKey, asTableValue))
            {
                //
                //  If provider is in list, the subscription is acceptted
                //
                if ((providerInList(provider, asTableValue, nameSpace))
                    != PEG_NOT_FOUND)
                {
                    //
                    //  Add current subscription to list
                    //
                    matchingSubscriptions.append(subscriptions[j]);
                    matchingSubscriptionKeys.append(activeSubscriptionsKey);
                }
            }
        }
    }

    PEGASUS_ASSERT(
        matchingSubscriptions.size() == matchingSubscriptionKeys.size());
    PEG_METHOD_EXIT ();
}

Array<ActiveSubscriptionsTableEntry>
    SubscriptionTable::getAllActiveSubscriptionEntries()
{
    PEG_METHOD_ENTER(TRC_INDICATION_SERVICE,
        "SubscriptionTable::getAllActiveSubscriptionEntries");

    Array <ActiveSubscriptionsTableEntry> subscriptionsEntries;

    //
    // Iterate through the ActiveSubscriptions table to get all active
    // subscriptions table entries
    //

    ReadLock lock(_activeSubscriptionsTableLock);

    for (ActiveSubscriptionsTable::Iterator i =
        _activeSubscriptionsTable.start(); i; i++)
    {
        subscriptionsEntries.append(i.value());
    }

    PEG_METHOD_EXIT();
    return subscriptionsEntries;
}

#ifdef PEGASUS_ENABLE_INDICATION_COUNT

void SubscriptionTable::updateMatchedIndicationCounts(
    const CIMInstance & providerInstance,
    const Array<SubscriptionKey>& activeSubscriptionsKeys)
{
    PEG_METHOD_ENTER(TRC_INDICATION_SERVICE,
        "SubscriptionTable::updateMatchedIndicationCounts");

    WriteLock lock(_activeSubscriptionsTableLock);

    for (Uint32 i = 0; i < activeSubscriptionsKeys.size(); i++)
    {
        ActiveSubscriptionsTableEntry* entry = 0;
        if (_activeSubscriptionsTable.lookupReference(
                activeSubscriptionsKeys[i], entry))
        {
            Uint32 providerIndex = providerInList(providerInstance, *entry);
            if (providerIndex != PEG_NOT_FOUND)
            {
                entry->providers[providerIndex].
                    matchedIndCountPerSubscription++;
            }
        }
        else
        {
            // The subscription may have been deleted in the mean time.
            // If so, no further update is required.
            PEG_TRACE((TRC_INDICATION_SERVICE, Tracer::LEVEL2,
                "Subscription %s not found in ActiveSubscriptionsTable",
                (const char *)
                    activeSubscriptionsKeys[i].toString().getCString()));
        }
    }
    PEG_METHOD_EXIT();
}


Array<CIMInstance>
    SubscriptionTable::enumerateSubscriptionIndicationDataInstances()
{
    PEG_METHOD_ENTER(TRC_INDICATION_SERVICE,
        "SubscriptionTable::enumerateSubscriptionIndicationDataInstances");

    Array<CIMInstance> instances;

    //
    // Get all active subscriptions table entries
    //
    Array<ActiveSubscriptionsTableEntry> activeSubscriptionEntries =
        getAllActiveSubscriptionEntries();

    for (Uint32 i = 0; i < activeSubscriptionEntries.size(); i++)
    {
        //
        // Gets filter name and handler name of the subscription
        //
        CIMInstance subscription = activeSubscriptionEntries[i].subscription;
        String sourceNS = subscription.getPath().getNameSpace().getString();

        String filterName;
        String handlerName;
        _getFilterAndHandlerNames(subscription, filterName, handlerName);

        Array<ProviderClassList> providers =
            activeSubscriptionEntries[i].providers;

        for (Uint32 j = 0; j < providers.size(); j++)
        {
            //
            // Gets provider name and provider module name
            //
            String providerName, providerModuleName;
            ProviderIndicationCountTable::getProviderKeys(
                providers[j].provider,
                providerModuleName,
                providerName);

            CIMInstance subscriptionIndDataInstance =
                _buildSubscriptionIndDataInstance(
                    filterName,
                    handlerName,
                    sourceNS,
                    providerModuleName,
                    providerName,
                    providers[j].matchedIndCountPerSubscription);

            instances.append(subscriptionIndDataInstance);
        }
    }

    PEG_METHOD_EXIT();
    return instances;
}

Array<CIMObjectPath>
    SubscriptionTable::enumerateSubscriptionIndicationDataInstanceNames()
{
    PEG_METHOD_ENTER(TRC_INDICATION_SERVICE,
        "SubscriptionTable::enumerateSubscriptionIndicationDataInstanceNames");

    Array<CIMObjectPath> instanceNames;

    //
    // Get all active subscriptions table entries
    //
    Array<ActiveSubscriptionsTableEntry> activeSubscriptionEntries =
        getAllActiveSubscriptionEntries();

    for (Uint32 i = 0; i < activeSubscriptionEntries.size(); i++)
    {
        //
        // Gets filter name and handler name of the subscription
        //
        CIMInstance subscription = activeSubscriptionEntries[i].subscription;
        String sourceNS = subscription.getPath().getNameSpace().getString();

        String filterName;
        String handlerName;
        _getFilterAndHandlerNames(subscription, filterName, handlerName);

        Array<ProviderClassList> providers =
            activeSubscriptionEntries[i].providers;

        for (Uint32 j = 0; j < providers.size(); j++)
        {
            //
            // Gets provider name and provider module name
            //
            String providerName, providerModuleName;
            ProviderIndicationCountTable::getProviderKeys(
                providers[j].provider,
                providerModuleName,
                providerName);

            CIMObjectPath path = _buildSubscriptionIndDataInstanceName(
                filterName,
                handlerName,
                sourceNS,
                providerModuleName,
                providerName);

            instanceNames.append(path);
        }
    }

    PEG_METHOD_EXIT();
    return instanceNames;
}

CIMInstance SubscriptionTable::getSubscriptionIndicationDataInstance(
    const CIMObjectPath& instanceName)
{
    PEG_METHOD_ENTER(TRC_INDICATION_SERVICE,
        "SubscriptionTable::getSubscriptionIndicationDataInstance");

    String filterName;
    String handlerName;
    String sourceNS;
    String specifiedProviderModuleName;
    String specifiedProviderName;

    //
    // Gets handler name, filter name, source namespace, provider module name,
    // and provider name from a PG_SubscriptionIndicationData instanceName
    //
    _getSubscriptionIndicationDataKeys(
        instanceName,
        filterName,
        handlerName,
        sourceNS,
        specifiedProviderModuleName,
        specifiedProviderName);

    // Builds the PG_Provider object path
    CIMObjectPath providerName = _buildProviderPath(
        specifiedProviderModuleName, specifiedProviderName);

    //
    // Builds subscription path by using the specified parameters
    //
    CIMObjectPath subscriptionPath = _buildSubscriptionPath(
        filterName, handlerName, sourceNS);

    //
    // Look up the subscription in the active subscriptions table
    //
    ActiveSubscriptionsTableEntry asTableValue;
    if (getSubscriptionEntry(subscriptionPath, asTableValue))
    {
        Array<ProviderClassList> providers = asTableValue.providers;
        for (Uint32 i = 0; i < providers.size(); i++)
        {
            if (providerName.identical(providers[i].provider.getPath()))
            {
                CIMInstance subIndDataInstance =
                    _buildSubscriptionIndDataInstance(
                        filterName,
                        handlerName,
                        sourceNS,
                        specifiedProviderModuleName,
                        specifiedProviderName,
                        providers[i].matchedIndCountPerSubscription);

                PEG_METHOD_EXIT();
                return subIndDataInstance;
            }
        }
    }

    PEG_METHOD_EXIT();
    throw CIMObjectNotFoundException(instanceName.toString());
}

void SubscriptionTable::_getFilterAndHandlerNames(
    const CIMInstance& subscription,
    String& filterName,
    String& handlerName)
{
    PEG_METHOD_ENTER(TRC_INDICATION_SERVICE,
        "SubscriptionTable::_getFilterAndHandlerNames");

    CIMObjectPath filterPath;
    CIMObjectPath handlerPath;

    subscription.getProperty(subscription.findProperty(
        PEGASUS_PROPERTYNAME_FILTER)).getValue().get(filterPath);
    subscription.getProperty(subscription.findProperty(
        PEGASUS_PROPERTYNAME_HANDLER)).getValue().get(handlerPath);

    //
    //  Get Filter namespace - if not set in Filter reference property
    //  value, namespace is the namespace of the subscription
    //
    CIMNamespaceName filterNS = filterPath.getNameSpace();
    if (filterNS.isNull())
    {
        filterNS = subscription.getPath().getNameSpace();
    }

    //
    // Get filter name
    //
    Array<CIMKeyBinding> filterKeyBindings = filterPath.getKeyBindings();
    for (Uint32 i = 0; i < filterKeyBindings.size(); i++)
    {
        if (filterKeyBindings[i].getName().equal(PEGASUS_PROPERTYNAME_NAME))
        {
            filterName.append(filterNS.getString());
            filterName.append(":");
            filterName.append(filterKeyBindings[i].getValue());
            break;
        }
    }

    //
    //  Get handler namespace - if not set in handler reference property
    //  value, namespace is the namespace of the subscription
    //
    CIMNamespaceName handlerNS = handlerPath.getNameSpace();
    if (handlerNS.isNull())
    {
        handlerNS = subscription.getPath().getNameSpace();
    }

    //
    // Get handler name
    //
    Array<CIMKeyBinding> handlerKeyBindings = handlerPath.getKeyBindings();
    for (Uint32 i = 0; i < handlerKeyBindings.size(); i++)
    {
        if (handlerKeyBindings[i].getName().equal(PEGASUS_PROPERTYNAME_NAME))
        {
            handlerName.append(handlerNS.getString());
            handlerName.append(":");
            handlerName.append(handlerPath.getClassName().getString());
            handlerName.append(".");
            handlerName.append(handlerKeyBindings[i].getValue());
            break;
        }
    }

    PEG_METHOD_EXIT();
}

void SubscriptionTable::_getSubscriptionIndicationDataKeys(
    const CIMObjectPath& instanceName,
    String& filterName,
    String& handlerName,
    String& sourceNS,
    String& providerModuleName,
    String& providerName)
{
    PEG_METHOD_ENTER(TRC_INDICATION_SERVICE,
        "SubscriptionTable::_getSubscriptionIndicationDataKeys");

    Array<CIMKeyBinding> keys = instanceName.getKeyBindings();
    for (Uint32 i = 0; i < keys.size(); i++)
    {
        if (keys[i].getName() == "FilterName")
        {
            filterName = keys[i].getValue();
        }
        else if (keys[i].getName() == "HandlerName")
        {
            handlerName = keys[i].getValue();
        }
        else if (keys[i].getName() == "SourceNamespace")
        {
            sourceNS = keys[i].getValue();
        }
        else if (keys[i].getName() == "ProviderModuleName")
        {
            providerModuleName = keys[i].getValue();
        }
        else if (keys[i].getName() == "ProviderName")
        {
            providerName = keys[i].getValue();
        }
    }

    PEG_METHOD_EXIT();
}


CIMObjectPath SubscriptionTable::_buildFilterPath(const String& filterName)
{
    PEG_METHOD_ENTER(TRC_INDICATION_SERVICE,
        "SubscriptionTable::_buildFilterPath");

    //
    // creates filter object path from input string filterName
    // (namespace:filtername)
    //
    String name;
    Uint32 colonIndex = filterName.find(':');

    if (colonIndex != PEG_NOT_FOUND)
    {
        name = filterName.subString(colonIndex + 1);
    }

    Array<CIMKeyBinding> filterKeys;
    filterKeys.append(CIMKeyBinding(
        "SystemCreationClassName",
        System::getSystemCreationClassName(),
        CIMKeyBinding::STRING));
    filterKeys.append(CIMKeyBinding(
        "SystemName",
        String::EMPTY,
        CIMKeyBinding::STRING));
    filterKeys.append(CIMKeyBinding(
        "CreationClassName",
        PEGASUS_CLASSNAME_INDFILTER.getString(),
        CIMKeyBinding::STRING));
    filterKeys.append(CIMKeyBinding(
        PEGASUS_PROPERTYNAME_NAME,
        name,
        CIMKeyBinding::STRING));

    CIMObjectPath filterPath = CIMObjectPath(
        String::EMPTY,
        CIMNamespaceName(),
        PEGASUS_CLASSNAME_INDFILTER,
        filterKeys);

    PEG_METHOD_EXIT();
    return filterPath;
}

CIMObjectPath SubscriptionTable::_buildHandlerPath(const String& handlerName)
{
    PEG_METHOD_ENTER(TRC_INDICATION_SERVICE,
        "SubscriptionTable::_buildHandlerPath");

    //
    // creates handler object path from input string handlerName
    // (namespace:classname.handlername)
    //
    String name;
    String classname;
    Uint32 colonIndex = handlerName.find(':');
    Uint32 dotIndex = handlerName.find('.');

    if (colonIndex != PEG_NOT_FOUND)
    {
        if ((dotIndex != PEG_NOT_FOUND) && (dotIndex > colonIndex))
        {
            classname = handlerName.subString(
                colonIndex + 1, dotIndex - 1 - colonIndex);
            name = handlerName.subString(dotIndex + 1);
        }
    }

    Array<CIMKeyBinding> handlerKeys;
    handlerKeys.append(CIMKeyBinding(
        "SystemCreationClassName",
        System::getSystemCreationClassName(),
        CIMKeyBinding::STRING));
    handlerKeys.append(CIMKeyBinding(
        "SystemName",
        String::EMPTY,
        CIMKeyBinding::STRING));
    handlerKeys.append(CIMKeyBinding(
        "CreationClassName",
        classname,
        CIMKeyBinding::STRING));
    handlerKeys.append(CIMKeyBinding(
        PEGASUS_PROPERTYNAME_NAME,
        name,
        CIMKeyBinding::STRING));

    CIMObjectPath handlerPath = CIMObjectPath(
        String::EMPTY,
        CIMNamespaceName(),
        classname,
        handlerKeys);

    PEG_METHOD_EXIT();
    return handlerPath;
}

CIMObjectPath SubscriptionTable::_buildSubscriptionPath(
    const String& filterName,
    const String& handlerName,
    const String& sourceNS)
{
    CIMObjectPath filterPath = _buildFilterPath(filterName);
    CIMObjectPath handlerPath = _buildHandlerPath(handlerName);

    CIMObjectPath subscriptionPath;
    Array<CIMKeyBinding> keyBindings;
    keyBindings.append(CIMKeyBinding(
        PEGASUS_PROPERTYNAME_FILTER,
        filterPath.toString(),
        CIMKeyBinding::REFERENCE));
    keyBindings.append(CIMKeyBinding(
        PEGASUS_PROPERTYNAME_HANDLER,
        handlerPath.toString(),
        CIMKeyBinding::REFERENCE));

    subscriptionPath.setClassName(PEGASUS_CLASSNAME_INDSUBSCRIPTION);
    subscriptionPath.setNameSpace(sourceNS);
    subscriptionPath.setKeyBindings(keyBindings);

    return subscriptionPath;
}

CIMInstance SubscriptionTable::_buildSubscriptionIndDataInstance(
    const String& filterName,
    const String& handlerName,
    const String& sourceNS,
    const String& providerModuleName,
    const String& providerName,
    Uint32 matchedIndicationCount)
{
    CIMInstance subscriptionIndDataInstance(
        PEGASUS_CLASSNAME_SUBSCRIPTIONINDDATA);
    subscriptionIndDataInstance.addProperty(CIMProperty(
        CIMName("FilterName"), filterName));
    subscriptionIndDataInstance.addProperty(CIMProperty(
        CIMName("HandlerName"), handlerName));
    subscriptionIndDataInstance.addProperty(CIMProperty(
        CIMName("SourceNamespace"), sourceNS));
    subscriptionIndDataInstance.addProperty(CIMProperty(
        CIMName("ProviderModuleName"), providerModuleName));
    subscriptionIndDataInstance.addProperty(CIMProperty(
        CIMName("ProviderName"), providerName));
    subscriptionIndDataInstance.addProperty(CIMProperty(
        CIMName("MatchedIndicationCount"),
        matchedIndicationCount));

    CIMObjectPath path = _buildSubscriptionIndDataInstanceName(
        filterName,
        handlerName,
        sourceNS,
        providerModuleName,
        providerName);
    subscriptionIndDataInstance.setPath(path);

    return subscriptionIndDataInstance;
}

CIMObjectPath SubscriptionTable::_buildSubscriptionIndDataInstanceName(
    const String& filterName,
    const String& handlerName,
    const String& sourceNS,
    const String& providerModuleName,
    const String& providerName)
{
    CIMObjectPath path;
    Array<CIMKeyBinding> keyBindings;
    keyBindings.append(CIMKeyBinding(
        "FilterName",
        filterName,
        CIMKeyBinding::STRING));
    keyBindings.append(CIMKeyBinding(
        "HandlerName",
        handlerName,
        CIMKeyBinding::STRING));
    keyBindings.append(CIMKeyBinding(
        "SourceNamespace",
        sourceNS,
        CIMKeyBinding::STRING));
    keyBindings.append(CIMKeyBinding(
        "ProviderModuleName",
        providerModuleName,
        CIMKeyBinding::STRING));
    keyBindings.append(CIMKeyBinding(
        "ProviderName",
        providerName,
        CIMKeyBinding::STRING));

    path.setClassName(PEGASUS_CLASSNAME_SUBSCRIPTIONINDDATA);
    path.setKeyBindings(keyBindings);

    return path;
}

CIMObjectPath SubscriptionTable::_buildProviderPath(
    const String& providerModuleName,
    const String& providerName)
{
    CIMObjectPath path;
    Array<CIMKeyBinding> keyBindings;
    keyBindings.append(CIMKeyBinding(
        _PROPERTY_PROVIDERMODULENAME,
        providerModuleName,
        CIMKeyBinding::STRING));
    keyBindings.append(CIMKeyBinding(
        PEGASUS_PROPERTYNAME_NAME,
        providerName,
        CIMKeyBinding::STRING));

    path.setClassName(PEGASUS_CLASSNAME_PROVIDER);
    path.setKeyBindings(keyBindings);

    return path;
}

#endif

PEGASUS_NAMESPACE_END
