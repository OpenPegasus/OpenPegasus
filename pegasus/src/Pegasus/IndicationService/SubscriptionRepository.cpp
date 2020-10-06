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
#include <Pegasus/Common/Tracer.h>
#include <Pegasus/Common/LanguageParser.h>
#include <Pegasus/Repository/ObjectCache.h>

#include "IndicationConstants.h"
#include "SubscriptionRepository.h"

PEGASUS_USING_STD;

PEGASUS_NAMESPACE_BEGIN



/**
   Handler and Filter cache

   Note that a single cache can be used for handler and filter instances,
   since the string representation of the object path is used as the key
   and this is unique anyway.
*/
#define PEGASUS_INDICATION_HANDLER_FILTER_CACHE_SIZE 50

static ObjectCache<CIMInstance>
    _handlerFilterCache(PEGASUS_INDICATION_HANDLER_FILTER_CACHE_SIZE);
static Mutex _handlerFilterCacheMutex;


static String _getHandlerFilterCacheKey(
    const CIMObjectPath &instanceName,
    const CIMNamespaceName &defaultNamespace)
{
    CIMObjectPath path = instanceName;
    if (path.getNameSpace().isNull())
    {
        path.setNameSpace(defaultNamespace);
    }
    return path.toString();
}


SubscriptionRepository::SubscriptionRepository (
    CIMRepository * repository)
    : _repository (repository)
{
    _uncommittedCreateSubscriptionRequests = 0;
    _normalizedSubscriptionTable.reset(
        new NormalizedSubscriptionTable(getAllSubscriptions()));
}

SubscriptionRepository::~SubscriptionRepository ()
{
}

Uint32 SubscriptionRepository::getUncommittedCreateSubscriptionRequests()
{
    return _uncommittedCreateSubscriptionRequests;
}

void SubscriptionRepository::beginCreateSubscription(
    const CIMObjectPath &subPath)
{
    Boolean subscriptionExists;
    AutoMutex mtx(_normalizedSubscriptionTableMutex);
    if(_normalizedSubscriptionTable->exists(subPath, subscriptionExists))
    {
        if (subscriptionExists)
        {
            throw PEGASUS_CIM_EXCEPTION(CIM_ERR_ALREADY_EXISTS,
                subPath.toString());
        }
        throw PEGASUS_CIM_EXCEPTION_L(CIM_ERR_FAILED,
            MessageLoaderParms(
                "IndicationService.IndicationService."
                    "_MSG_DUPLICATE_SUBSCRIPTION_REQUEST",
                "Similar create subscription request is being processed. "
                    "Subscription path : $0",
                subPath.toString()));
    }
    _normalizedSubscriptionTable->add(subPath, false);
    _uncommittedCreateSubscriptionRequests++;
}

void SubscriptionRepository::cancelCreateSubscription(
    const CIMObjectPath &subPath)
{
    AutoMutex mtx(_normalizedSubscriptionTableMutex);
    _normalizedSubscriptionTable->remove(subPath);
    _uncommittedCreateSubscriptionRequests--;
}

void SubscriptionRepository::commitCreateSubscription(
    const CIMObjectPath &subPath)
{
    AutoMutex mtx(_normalizedSubscriptionTableMutex);
    _normalizedSubscriptionTable->remove(subPath);
    _normalizedSubscriptionTable->add(subPath, true);
    _uncommittedCreateSubscriptionRequests--;
}

CIMObjectPath SubscriptionRepository::createInstance (
    CIMInstance instance,
    const CIMNamespaceName & nameSpace,
    const String & userName,
    const AcceptLanguageList & acceptLanguages,
    const ContentLanguageList & contentLanguages,
    Boolean enabled)
{
    PEG_METHOD_ENTER (TRC_INDICATION_SERVICE,
        "SubscriptionRepository::createInstance");

    CIMObjectPath instanceRef;

    //
    //  Add creator property to Instance
    //  NOTE: userName is only set if authentication is turned on
    //
    String currentUser = userName;
    if (instance.findProperty (PEGASUS_PROPERTYNAME_INDSUB_CREATOR) ==
        PEG_NOT_FOUND)
    {
        instance.addProperty (CIMProperty
            (PEGASUS_PROPERTYNAME_INDSUB_CREATOR, currentUser));
    }
    else
    {
        CIMProperty creator = instance.getProperty
            (instance.findProperty
            (PEGASUS_PROPERTYNAME_INDSUB_CREATOR));
        creator.setValue (CIMValue (currentUser));
    }

    // Add CreationTime to the Listener Destination instances
    // Note: CreationTime is added only for CIMXML handlers at present.
    CIMName className = instance.getPath().getClassName();

    if ((className.equal(PEGASUS_CLASSNAME_INDHANDLER_CIMXML) ||
        className.equal(PEGASUS_CLASSNAME_LSTNRDST_CIMXML) || 
        className.equal(PEGASUS_CLASSNAME_INDHANDLER_WSMAN)) &&
        instance.findProperty(PEGASUS_PROPERTYNAME_LSTNRDST_CREATIONTIME)
            == PEG_NOT_FOUND)
    {
        instance.addProperty(
            CIMProperty(
                PEGASUS_PROPERTYNAME_LSTNRDST_CREATIONTIME,
                System::getCurrentTimeUsec()));
    }


    // l10n
    // Add the language properties to the Instance
    // Note:  These came from the Accept-Language and Content-Language
    // headers in the HTTP message, and may be empty
    AcceptLanguageList acceptLangs = acceptLanguages;
    if (instance.findProperty (PEGASUS_PROPERTYNAME_INDSUB_ACCEPTLANGS) ==
        PEG_NOT_FOUND)
    {
        instance.addProperty (CIMProperty
            (PEGASUS_PROPERTYNAME_INDSUB_ACCEPTLANGS,
            LanguageParser::buildAcceptLanguageHeader(acceptLangs)));
    }
    else
    {
        CIMProperty langs = instance.getProperty
            (instance.findProperty
            (PEGASUS_PROPERTYNAME_INDSUB_ACCEPTLANGS));
        langs.setValue (CIMValue (
            LanguageParser::buildAcceptLanguageHeader(acceptLangs)));
    }

    ContentLanguageList contentLangs = contentLanguages;
    if (instance.findProperty (PEGASUS_PROPERTYNAME_INDSUB_CONTENTLANGS) ==
        PEG_NOT_FOUND)
    {
        instance.addProperty (CIMProperty
            (PEGASUS_PROPERTYNAME_INDSUB_CONTENTLANGS,
            LanguageParser::buildContentLanguageHeader(contentLangs)));
    }
    else
    {
        CIMProperty langs = instance.getProperty
            (instance.findProperty
            (PEGASUS_PROPERTYNAME_INDSUB_CONTENTLANGS));
        langs.setValue (CIMValue (
            LanguageParser::buildContentLanguageHeader(contentLangs)));
    }
    // l10n -end

    if ((instance.getClassName ().equal
        (PEGASUS_CLASSNAME_INDSUBSCRIPTION)) ||
        (instance.getClassName ().equal
        (PEGASUS_CLASSNAME_FORMATTEDINDSUBSCRIPTION)))
    {
        //
        //  Set Time of Last State Change to current date time
        //
        CIMDateTime currentDateTime =
            CIMDateTime::getCurrentDateTime ();
        if (instance.findProperty (_PROPERTY_LASTCHANGE) ==
            PEG_NOT_FOUND)
        {
            instance.addProperty
                (CIMProperty (_PROPERTY_LASTCHANGE, currentDateTime));
        }
        else
        {
            CIMProperty lastChange = instance.getProperty
                (instance.findProperty (_PROPERTY_LASTCHANGE));
            lastChange.setValue (CIMValue (currentDateTime));
        }

        CIMDateTime startDateTime;
        if (enabled)
        {
            startDateTime = currentDateTime;
        }
        else
        {
            //
            //  If subscription is not enabled, set Subscription
            //  Start Time to null CIMDateTime value
            //
            startDateTime = CIMDateTime ();
        }

        //
        //  Set Subscription Start Time
        //
        if (instance.findProperty (_PROPERTY_STARTTIME) ==
            PEG_NOT_FOUND)
        {
            instance.addProperty
                (CIMProperty (_PROPERTY_STARTTIME, startDateTime));
        }
        else
        {
            CIMProperty startTime = instance.getProperty
                (instance.findProperty (_PROPERTY_STARTTIME));
            startTime.setValue (CIMValue (startDateTime));
        }
    }

    //
    //  Create instance in repository
    //
    try
    {
        instanceRef = _repository->createInstance (nameSpace, instance);
    }
    catch (const CIMException &)
    {
        PEG_METHOD_EXIT ();
        throw;
    }
    catch (const Exception & exception)
    {
        PEG_METHOD_EXIT ();
        throw PEGASUS_CIM_EXCEPTION (CIM_ERR_FAILED, exception.getMessage ());
    }

    PEG_METHOD_EXIT ();
    return instanceRef;
}

Boolean SubscriptionRepository::getActiveSubscriptions (
    Array <CIMInstance> & activeSubscriptions) const
{
    PEG_METHOD_ENTER (TRC_INDICATION_SERVICE,
        "SubscriptionRepository::getActiveSubscriptions");

    Array <CIMNamespaceName> nameSpaceNames;
    Array <CIMInstance> subscriptions;
    CIMValue subscriptionStateValue;
    Uint16 subscriptionState;
    Boolean invalidInstance = false;

    activeSubscriptions.clear ();

    //
    //  Get list of namespaces in repository
    //
    nameSpaceNames = _repository->enumerateNameSpaces ();

    //
    //  Get existing subscriptions from each namespace in the repository
    //
    for (Uint32 i = 0; i < nameSpaceNames.size (); i++)
    {

        //
        //  Get existing subscriptions in current namespace
        //
        subscriptions = getSubscriptions (nameSpaceNames [i]);

        //
        //  Process each subscription
        //
        for (Uint32 j = 0; j < subscriptions.size (); j++)
        {
            //
            //  Get subscription state
            //
            if (!getState (subscriptions [j], subscriptionState))
            {
                //
                //  This instance from the repository is corrupted
                //  Skip it
                //
                invalidInstance = true;
                break;
            }

            //
            //  Process each enabled subscription
            //
            if ((subscriptionState == STATE_ENABLED) ||
                (subscriptionState == STATE_ENABLEDDEGRADED))
            {
                //
                //  CIMInstances returned from repository do not include
                //  namespace
                //  Set namespace here
                //
                CIMObjectPath instanceName = subscriptions [j].getPath ();
                instanceName.setNameSpace (nameSpaceNames [i]);
                subscriptions [j].setPath (instanceName);
                activeSubscriptions.append (subscriptions [j]);
            }  // if subscription is enabled
        }  // for each subscription
    }  // for each namespace

    PEG_METHOD_EXIT ();
    return invalidInstance;
}

Array <CIMInstance> SubscriptionRepository::getAllSubscriptions () const
{
    PEG_METHOD_ENTER (TRC_INDICATION_SERVICE,
        "SubscriptionRepository::getAllSubscriptions");

    Array <CIMNamespaceName> nameSpaceNames;
    Array <CIMInstance> subscriptions;
    Array <CIMInstance> allSubscriptions;

    //
    //  Get list of namespaces in repository
    //
    nameSpaceNames = _repository->enumerateNameSpaces ();

    //
    //  Get all subscriptions from each namespace in the repository
    //
    for (Uint32 i = 0; i < nameSpaceNames.size (); i++)
    {
        //
        //  Get all subscriptions in current namespace
        //
        subscriptions = getSubscriptions (nameSpaceNames [i]);

        //
        //  Append subscriptions in current namespace to list of all
        //  subscriptions
        //
        allSubscriptions.appendArray (subscriptions);
    }

    PEG_METHOD_EXIT ();
    return allSubscriptions;
}

Array <CIMInstance> SubscriptionRepository::getSubscriptions (
    const CIMNamespaceName & nameSpace) const
{
    PEG_METHOD_ENTER (TRC_INDICATION_SERVICE,
        "SubscriptionRepository::getSubscriptions");

    Array <CIMInstance> subscriptions;

    //
    //  Get the CIM_IndicationSubscription and
    //  CIM_FormattedIndicationSubscription instances in specified namespace
    //
    try
    {
        subscriptions = _repository->enumerateInstancesForClass(
            nameSpace, PEGASUS_CLASSNAME_INDSUBSCRIPTION);
        subscriptions.appendArray(_repository->enumerateInstancesForClass(
            nameSpace, PEGASUS_CLASSNAME_FORMATTEDINDSUBSCRIPTION));
    }
    catch (const CIMException& e)
    {
        //
        //  Some namespaces may not include the subscription class
        //  In that case, just return no subscriptions
        //  Any other exception is an error
        //
        if (e.getCode () != CIM_ERR_INVALID_CLASS)
        {
            PEG_METHOD_EXIT ();
            throw;
        }
    }

    //
    //  Process each subscription
    //
    for (Uint32 i = 0; i < subscriptions.size(); i++)
    {
        //
        //  CIMInstances returned from repository do not include
        //  namespace
        //  Set namespace here
        //
        CIMObjectPath instanceName = subscriptions[i].getPath();
        instanceName.setNameSpace(nameSpace);
        subscriptions[i].setPath(instanceName);
    }

    PEG_METHOD_EXIT ();
    return subscriptions;
}

Boolean SubscriptionRepository::getState (
    const CIMInstance & instance,
    Uint16 & state) const
{
    PEG_METHOD_ENTER (TRC_INDICATION_SERVICE,
        "SubscriptionRepository::getState");

    Uint32 stateIndex =
        instance.findProperty(PEGASUS_PROPERTYNAME_SUBSCRIPTION_STATE);
    if (stateIndex != PEG_NOT_FOUND)
    {
        CIMValue stateValue = instance.getProperty
            (stateIndex).getValue ();
        if (stateValue.isNull ())
        {
            PEG_TRACE_CSTRING (TRC_INDICATION_SERVICE,Tracer::LEVEL1,
                "Null SubscriptionState property value");

            //
            //  This is a corrupted/invalid instance
            //
            return false;
        }
        else if ((stateValue.getType () != CIMTYPE_UINT16) ||
            (stateValue.isArray ()))
        {
            PEG_TRACE((TRC_INDICATION_SERVICE,Tracer::LEVEL1,
                "SubscriptionState property value of incorrect type:%s %s",
                (stateValue.isArray()) ? " array of" : " ",
                cimTypeToString(stateValue.getType())));

            //
            //  This is a corrupted/invalid instance
            //
            return false;
        }
        else
        {
            stateValue.get (state);
        }
    }
    else
    {
        PEG_TRACE_CSTRING(TRC_INDICATION_SERVICE,Tracer::LEVEL1,
            "Missing SubscriptionState property");

        //
        //  This is a corrupted/invalid instance
        //
        return false;
    }

    PEG_METHOD_EXIT ();
    return true;
}

CIMInstance SubscriptionRepository::deleteSubscription (
    CIMObjectPath & subscription)
{
    PEG_METHOD_ENTER (TRC_INDICATION_SERVICE,
        "SubscriptionRepository::deleteSubscription");

    CIMInstance subscriptionInstance;
    CIMNamespaceName nameSpace = subscription.getNameSpace ();
    subscription.setNameSpace (CIMNamespaceName ());

    //
    //  Get instance from repository
    //
    try
    {
        subscriptionInstance = _repository->getInstance(
            nameSpace, subscription);
    }
    catch (Exception & exception)
    {
        PEG_TRACE((TRC_INDICATION_SERVICE, Tracer::LEVEL1,
            "Exception caught in retrieving subscription (%s): %s",
            (const char*)subscriptionInstance.getPath().toString().getCString(),
            (const char*)exception.getMessage().getCString()));

        //
        //  If the subscription could not be retrieved, it may already have
        //  been deleted by another thread
        //
        PEG_METHOD_EXIT ();
        return CIMInstance ();
    }

    //
    //  Delete the subscription instance
    //
    try
    {
        deleteInstance (nameSpace, subscription);
    }
    catch (Exception & exception)
    {
        PEG_TRACE((TRC_INDICATION_SERVICE, Tracer::LEVEL1,
            "Exception caught in deleting subscription (%s): %s",
            (const char*)subscriptionInstance.getPath().toString().getCString(),
            (const char*)exception.getMessage().getCString()));

        //
        //  If the subscription could not be deleted, it may already have
        //  been deleted by another thread
        //
        PEG_METHOD_EXIT ();
        return CIMInstance ();
    }

    //
    //  Reset namespace in object path
    //
    subscription.setNameSpace (nameSpace);

    PEG_METHOD_EXIT ();
    return subscriptionInstance;
}

Array <CIMInstance> SubscriptionRepository::deleteReferencingSubscriptions (
    const CIMNamespaceName & nameSpace,
    const CIMName & referenceProperty,
    const CIMObjectPath & handler)
{
    PEG_METHOD_ENTER (TRC_INDICATION_SERVICE,
        "SubscriptionRepository::deleteReferencingSubscriptions");

    Array <CIMInstance> subscriptions;
    Array <CIMInstance> deletedSubscriptions;

    //
    //  Get all subscriptions in all namespaces
    //
    subscriptions = getAllSubscriptions ();

    //
    //  Check each subscription for a reference to the specified instance
    //
    for (Uint32 i = 0; i < subscriptions.size (); i++)
    {
        //
        //  Get the reference property value from the subscription instance
        //
        CIMValue propValue = subscriptions [i].getProperty
            (subscriptions [i].findProperty (referenceProperty)).getValue ();
        CIMObjectPath ref;
        propValue.get (ref);

        //
        //  If the Handler reference property value includes namespace, check
        //  if it is the namespace of the Handler being deleted.
        //  If the Handler reference property value does not include namespace,
        //  check if the current subscription namespace is the namespace of the
        //  Handler being deleted.
        //
        CIMNamespaceName handlerNS = ref.getNameSpace ();
        if (((handlerNS.isNull ()) &&
            (subscriptions[i].getPath ().getNameSpace () == nameSpace))
            || (handlerNS == nameSpace))
        {
            //
            //  Remove Host and Namespace from reference property value, if
            //  present, before comparing
            //
            CIMObjectPath href ("", CIMNamespaceName (),
                ref.getClassName (), ref.getKeyBindings ());

            //
            //  Remove Host and Namespace from reference of handler instance to
            //  be deleted, if present, before comparing
            //
            CIMObjectPath iref ("", CIMNamespaceName (),
                handler.getClassName (), handler.getKeyBindings ());

            //
            //  If the current subscription references the specified instance,
            //  delete it
            //
            if (iref == href)
            {
                //
                //  Delete referencing subscription instance from repository
                //
                try
                {
                    //
                    //  Namespace and host must not be set in path passed to
                    //  repository
                    //
                    CIMObjectPath path ("", CIMNamespaceName (),
                        subscriptions [i].getPath ().getClassName (),
                        subscriptions [i].getPath ().getKeyBindings ());
                    deleteInstance
                        (subscriptions [i].getPath ().getNameSpace (), path);
                }
                catch (Exception & exception)
                {
                    //
                    //  Deletion of referencing subscription failed
                    //
                    PEG_TRACE((TRC_INDICATION_SERVICE, Tracer::LEVEL1,
                        "Exception caught deleting referencing "
                        "subscription (%s): %s",
                        (const char*)subscriptions [i].getPath().toString()
                               .getCString(),
                        (const char*)exception.getMessage().getCString()));

                }

                deletedSubscriptions.append (subscriptions [i]);
            }
        }
    }

    PEG_METHOD_EXIT ();
    return deletedSubscriptions;
}

CIMInstance SubscriptionRepository::getHandler (
    const CIMInstance & subscription) const
{
    PEG_METHOD_ENTER (TRC_INDICATION_SERVICE,
        "SubscriptionRepository::getHandler");

    CIMValue handlerValue;
    CIMObjectPath handlerRef;
    CIMInstance handlerInstance;
    CIMNamespaceName nameSpaceName;
    String handlerName;

    //
    //  Get Handler reference from subscription instance
    //
    handlerValue = subscription.getProperty (subscription.findProperty
        (PEGASUS_PROPERTYNAME_HANDLER)).getValue ();

    handlerValue.get (handlerRef);

    //
    //  Get handler namespace - if not set in Handler reference property value,
    //  namespace is the namespace of the subscription
    //
    nameSpaceName = handlerRef.getNameSpace ();
    if (nameSpaceName.isNull ())
    {
        nameSpaceName = subscription.getPath ().getNameSpace ();
    }

    handlerName = _getHandlerFilterCacheKey(handlerRef, nameSpaceName);

    if (!_handlerFilterCache.get(handlerName, handlerInstance))
    {
        //
        //  Not in cache so get Handler instance from the repository
        //
        AutoMutex mtx(_handlerFilterCacheMutex);
        try
        {
            handlerInstance = _repository->getInstance(
                nameSpaceName, handlerRef, false, false, CIMPropertyList());
        }
        catch (const Exception & exception)
        {
            PEG_TRACE((TRC_DISCARDED_DATA, Tracer::LEVEL1,
                "Exception caught trying to get Handler instance (%s): %s",
                (const char*)handlerRef.toString().getCString(),
                (const char*)exception.getMessage().getCString()));
            PEG_METHOD_EXIT ();
            throw;
        }

        //
        //  Set namespace in path in CIMInstance
        //
        handlerRef.setNameSpace (nameSpaceName);
        handlerInstance.setPath (handlerRef);

        //
        //  Add handler to cache
        //
        _handlerFilterCache.put(handlerName, handlerInstance);

    } /* if not in cache */

    PEG_METHOD_EXIT ();
    return handlerInstance;
}

Boolean SubscriptionRepository::isTransient (
    const CIMNamespaceName & nameSpace,
    const CIMObjectPath & handler) const
{
    PEG_METHOD_ENTER (TRC_INDICATION_SERVICE,
        "SubscriptionRepository::isTransient");

    CIMValue persistenceValue;
    Uint16 persistenceType;

    //
    //  Get the handler instance from the repository
    //
    CIMInstance instance;

    instance = _repository->getInstance(
        nameSpace, handler, false, false, CIMPropertyList());

    //
    //  Get Persistence Type
    //
    persistenceValue = instance.getProperty (instance.findProperty
        (PEGASUS_PROPERTYNAME_PERSISTENCETYPE)).getValue ();
    persistenceValue.get (persistenceType);

    if (persistenceType == PERSISTENCE_TRANSIENT)
    {
        PEG_METHOD_EXIT ();
        return true;
    }
    else
    {
        PEG_METHOD_EXIT ();
        return false;
    }
}

void SubscriptionRepository::getFilterProperties (
    const CIMInstance & subscription,
    String & query,
    Array<CIMNamespaceName> &sourceNameSpaces,
    String & queryLanguage,
    String & filterName)
{
    PEG_METHOD_ENTER (TRC_INDICATION_SERVICE,
        "SubscriptionRepository::getFilterProperties");

    CIMValue filterValue;
    CIMObjectPath filterReference;
    CIMInstance filterInstance;
    CIMNamespaceName nameSpaceName;
    String filterNameInCache;

    filterValue = subscription.getProperty (subscription.findProperty
        (PEGASUS_PROPERTYNAME_FILTER)).getValue ();

    filterValue.get (filterReference);

    //
    //  Get filter namespace - if not set in Filter reference property value,
    //  namespace is the namespace of the subscription
    //
    nameSpaceName = filterReference.getNameSpace ();
    if (nameSpaceName.isNull ())
    {
        nameSpaceName = subscription.getPath ().getNameSpace ();
    }

    filterNameInCache =
        _getHandlerFilterCacheKey(filterReference, nameSpaceName);

    if (!_handlerFilterCache.get(filterNameInCache, filterInstance))
    {
        //
        //  Not in cache so get filter instance from the repository
        //
        AutoMutex mtx(_handlerFilterCacheMutex);
        try
        {
            filterInstance = _repository->getInstance(
                nameSpaceName, filterReference);
        }
        catch (const Exception & exception)
        {
            PEG_TRACE((TRC_DISCARDED_DATA, Tracer::LEVEL1,
                "Exception caught trying to get Filter instance (%s): %s",
                (const char*)filterReference.toString().getCString(),
                (const char*)exception.getMessage().getCString()));
            PEG_METHOD_EXIT ();
            throw;
        }
        //
        //  Add filter to cache
        //
       _handlerFilterCache.put(filterNameInCache, filterInstance);
    }

    query = filterInstance.getProperty (filterInstance.findProperty
        (PEGASUS_PROPERTYNAME_QUERY)).getValue ().toString ();


    queryLanguage = filterInstance.getProperty
        (filterInstance.findProperty (PEGASUS_PROPERTYNAME_QUERYLANGUAGE)).
        getValue ().toString ();

    filterName = filterInstance.getProperty
        (filterInstance.findProperty (PEGASUS_PROPERTYNAME_NAME)).
        getValue ().toString ();

    getSourceNamespaces(
        filterInstance,
        nameSpaceName,
        sourceNameSpaces);

    PEG_METHOD_EXIT ();
}

void SubscriptionRepository::getSourceNamespaces(
    const CIMInstance &instance,
    const CIMNamespaceName &defaultNameSpace,
    Array<CIMNamespaceName> &sourceNamespaces)
{
    Uint32 srcNSPos = instance.findProperty(_PROPERTY_SOURCENAMESPACE);
    Uint32 srcNSSPos = instance.findProperty(_PROPERTY_SOURCENAMESPACES);

    CIMValue srcNSValue;
    if (srcNSPos != PEG_NOT_FOUND)
    {
        srcNSValue = instance.getProperty(srcNSPos).getValue();
    }
    CIMValue srcNSSValue;
    if (srcNSSPos != PEG_NOT_FOUND)
    {
        srcNSSValue = instance.getProperty(srcNSSPos).getValue();
    }

    if (!srcNSSValue.isNull())
    {
        Array<String> srcNamespaces;
        srcNSSValue.get(srcNamespaces);
        for(Uint32 i = 0, n = srcNamespaces.size(); i < n; ++i)
        {
            sourceNamespaces.append(srcNamespaces[i]);
        }
    }

    if (!srcNSValue.isNull())
    {
        String srcNS;
        srcNSValue.get(srcNS);
        // If both sourceNamespace and sourceNamespaces properties exist,
        // sourceNamespaces value should contain sourceNamespace value.
        if (sourceNamespaces.size())
        {
            Boolean found = false;
            for (Uint32 i = 0; i < sourceNamespaces.size(); ++i)
            {
                if (sourceNamespaces[i].equal(srcNS))
                {
                    found =true;
                    break;
                }
            }
            if (!found)
            {
                throw PEGASUS_CIM_EXCEPTION_L(
                    CIM_ERR_INVALID_PARAMETER,
                    MessageLoaderParms(
                        "IndicationService.IndicationService."
                            "_INVALID_SOURCENAMESPACE_VALUE",
                        "The values in the SourceNamespaces property and the "
                            "SourceNamespace property are not additive. If both"
                            " sourceNamespace and SourceNamespaces are non NULL"
                            ", the namespace defined in sourceNamespace must"
                            "also exist in sourceNamespaces."));
            }
        }
        else
        {
            sourceNamespaces.append(srcNS);
        }
    }

    if (sourceNamespaces.size() == 0)
    {
        sourceNamespaces.append(defaultNameSpace);
    }
}

void SubscriptionRepository::getFilterProperties (
    const CIMInstance & subscription,
    String & query,
    Array<CIMNamespaceName> &sourceNameSpaces)
{
    PEG_METHOD_ENTER (TRC_INDICATION_SERVICE,
        "SubscriptionRepository::getFilterProperties");

    CIMValue filterValue;
    CIMObjectPath filterReference;
    CIMInstance filterInstance;
    CIMNamespaceName nameSpaceName;

    filterValue = subscription.getProperty (subscription.findProperty
        (PEGASUS_PROPERTYNAME_FILTER)).getValue ();

    filterValue.get (filterReference);

    //
    //  Get filter namespace - if not set in Filter reference property value,
    //  namespace is the namespace of the subscription
    //
    nameSpaceName = filterReference.getNameSpace ();
    if (nameSpaceName.isNull ())
    {
        nameSpaceName = subscription.getPath ().getNameSpace ();
    }

    try
    {
        filterInstance = _repository->getInstance(
            nameSpaceName, filterReference);
    }
    catch (const Exception & exception)
    {
        PEG_TRACE((TRC_INDICATION_SERVICE, Tracer::LEVEL1,
            "Exception caught in getting filter instance (%s): %s",
            (const char*)filterReference.toString().getCString(),
            (const char*)exception.getMessage().getCString()));
        PEG_METHOD_EXIT ();
        throw;
    }

    query = filterInstance.getProperty (filterInstance.findProperty
        (PEGASUS_PROPERTYNAME_QUERY)).getValue ().toString ();

    
    getSourceNamespaces(
        filterInstance,
        nameSpaceName,
        sourceNameSpaces);

    PEG_METHOD_EXIT ();
}

void SubscriptionRepository::getFilterProperties (
    const CIMInstance & subscription,
    String & query)
{
    PEG_METHOD_ENTER (TRC_INDICATION_SERVICE,
        "SubscriptionRepository::getFilterProperties");

    CIMValue filterValue;
    CIMObjectPath filterReference;
    CIMInstance filterInstance;
    CIMNamespaceName nameSpaceName;

    filterValue = subscription.getProperty (subscription.findProperty
        (PEGASUS_PROPERTYNAME_FILTER)).getValue ();

    filterValue.get (filterReference);

    //
    //  Get filter namespace - if not set in Filter reference property value,
    //  namespace is the namespace of the subscription
    //
    nameSpaceName = filterReference.getNameSpace ();
    if (nameSpaceName.isNull ())
    {
        nameSpaceName = subscription.getPath ().getNameSpace ();
    }

    try
    {
        filterInstance = _repository->getInstance(
            nameSpaceName, filterReference);
    }
    catch (const Exception & exception)
    {
        PEG_TRACE((TRC_INDICATION_SERVICE, Tracer::LEVEL1,
            "Exception caught in getting filter instance (%s): %s",
            (const char*)filterReference.toString().getCString(),
            (const char*)exception.getMessage().getCString()));
        PEG_METHOD_EXIT ();
        throw;
    }

    query = filterInstance.getProperty (filterInstance.findProperty
        (PEGASUS_PROPERTYNAME_QUERY)).getValue ().toString ();

    PEG_METHOD_EXIT ();
}

Boolean SubscriptionRepository::validateIndicationClassName (
    const CIMName & indicationClassName,
    const CIMNamespaceName & nameSpaceName) const
{
    PEG_METHOD_ENTER (TRC_INDICATION_SERVICE,
        "SubscriptionRepository::validateIndicationClassName");

    //
    //  Validate that class is an Indication class
    //  The Indication Qualifier should exist and have the value True
    //
    Boolean validClass = false;
    CIMClass theClass;

    try
    {
        theClass = _repository->getClass (nameSpaceName, indicationClassName,
            false, true, false, CIMPropertyList ());
    }
    catch (const Exception & exception)
    {
        PEG_TRACE((TRC_DISCARDED_DATA, Tracer::LEVEL1,
            "Exception caught trying to get indication class (%s): %s",
            (const char*)indicationClassName.getString().getCString(),
            (const char*)exception.getMessage().getCString()));
        PEG_METHOD_EXIT ();
        throw;
    }

    if (theClass.findQualifier (_QUALIFIER_INDICATION) != PEG_NOT_FOUND)
    {
        CIMQualifier theQual = theClass.getQualifier (theClass.findQualifier
            (_QUALIFIER_INDICATION));
        CIMValue theVal = theQual.getValue ();
        if (!theVal.isNull ())
        {
            Boolean indicationClass;
            theVal.get (indicationClass);
            validClass = indicationClass;
        }
    }

    PEG_METHOD_EXIT ();
    return validClass;
}

Array <CIMName> SubscriptionRepository::getIndicationSubclasses (
        const CIMNamespaceName & nameSpace,
        const CIMName & indicationClassName) const
{
    PEG_METHOD_ENTER (TRC_INDICATION_SERVICE,
        "SubscriptionRepository::getIndicationSubclasses");

    Array <CIMName> indicationSubclasses;

    indicationSubclasses = _repository->enumerateClassNames(
        nameSpace, indicationClassName, true);

    indicationSubclasses.append (indicationClassName);

    PEG_METHOD_EXIT ();
    return indicationSubclasses;
}

Boolean SubscriptionRepository::reconcileFatalError (
    const CIMInstance &subscription)
{
    PEG_METHOD_ENTER (TRC_INDICATION_SERVICE,
        "SubscriptionRepository::reconcileFatalError");

    Boolean removeOrDisable = false;

    //
    //  Get the value of the On Fatal Error Policy property
    //
    CIMValue errorPolicyValue;
    Uint16 onFatalErrorPolicy;
    errorPolicyValue = subscription.getProperty
        (subscription.findProperty
        (_PROPERTY_ONFATALERRORPOLICY)).getValue ();
    errorPolicyValue.get (onFatalErrorPolicy);

    if (onFatalErrorPolicy == _ERRORPOLICY_DISABLE)
    {
        //
        //  FUTURE: Failure Trigger Time Interval should be allowed to pass
        //  before implementing On Fatal Error Policy
        //
        //  Set the Subscription State to disabled
        //
        _disableSubscription (subscription);
        removeOrDisable = true;
    }
    else if (onFatalErrorPolicy == _ERRORPOLICY_REMOVE)
    {
        //
        //  FUTURE: Failure Trigger Time Interval should be allowed to pass
        //  before implementing On Fatal Error Policy
        //
        //  Delete the subscription
        //
        _deleteSubscription (subscription);
        removeOrDisable = true;
    }

    PEG_METHOD_EXIT ();
    return removeOrDisable;
}

CIMClass SubscriptionRepository::getClass (
    const CIMNamespaceName & nameSpaceName,
    const CIMName & className,
    Boolean localOnly,
    Boolean includeQualifiers,
    Boolean includeClassOrigin,
    const CIMPropertyList & propertyList) const
{
    try
    {
        return _repository->getClass (nameSpaceName, className, localOnly,
            includeQualifiers, includeClassOrigin, propertyList);
    }
    catch (const Exception & exception)
    {
        PEG_TRACE((TRC_DISCARDED_DATA, Tracer::LEVEL1,
            "Exception caught trying to get class (%s) %s",
            (const char*)className.getString().getCString(),
            (const char*)exception.getMessage().getCString()));
        throw;
    }
}

CIMInstance SubscriptionRepository::getInstance (
    const CIMNamespaceName & nameSpace,
    const CIMObjectPath & instanceName,
    Boolean includeQualifiers,
    Boolean includeClassOrigin,
    const CIMPropertyList & propertyList)
{
    return _repository->getInstance (nameSpace, instanceName,
        includeQualifiers, includeClassOrigin, propertyList);
}

void SubscriptionRepository::modifyInstance (
    const CIMNamespaceName & nameSpace,
    const CIMInstance & modifiedInstance,
    Boolean includeQualifiers,
    const CIMPropertyList & propertyList)
{
    CIMObjectPath instanceName = modifiedInstance.getPath();
    if (instanceName.getClassName().equal(
            PEGASUS_CLASSNAME_INDFILTER) ||
        instanceName.getClassName().equal(
            PEGASUS_CLASSNAME_INDHANDLER_CIMXML) ||
        instanceName.getClassName().equal(
            PEGASUS_CLASSNAME_LSTNRDST_CIMXML) ||
        instanceName.getClassName().equal(
            PEGASUS_CLASSNAME_INDHANDLER_SNMP) ||
        instanceName.getClassName().equal(
            PEGASUS_CLASSNAME_LSTNRDST_FILE) ||
        instanceName.getClassName().equal(
            PEGASUS_CLASSNAME_LSTNRDST_EMAIL) ||
        instanceName.getClassName().equal(
            PEGASUS_CLASSNAME_LSTNRDST_SYSTEM_LOG) ||
        instanceName.getClassName().equal(
            PEGASUS_CLASSNAME_INDHANDLER_WSMAN))
    {
        AutoMutex mtx(_handlerFilterCacheMutex);

        _repository->modifyInstance (nameSpace, modifiedInstance,
             includeQualifiers, propertyList);

        // Try to remove the handler/filter from the cache.
        // It may not have been added there as it was not used for any
        // indication processing yet, so we don't care when the remove
        // fails.
        String objName = _getHandlerFilterCacheKey(instanceName, nameSpace);
        _handlerFilterCache.evict(objName);
    }
    else
    {
        _repository->modifyInstance (nameSpace, modifiedInstance,
             includeQualifiers, propertyList);
    }
}

void SubscriptionRepository::deleteInstance (
    const CIMNamespaceName & nameSpace,
    const CIMObjectPath & instanceName)
{
    // If deleted instance was SubscriptionInstance, delete from
    // Normalized subscriptions table.
    if (instanceName.getClassName().equal(
        PEGASUS_CLASSNAME_INDSUBSCRIPTION) ||
        instanceName.getClassName ().equal(
            PEGASUS_CLASSNAME_FORMATTEDINDSUBSCRIPTION))
    {
        _repository->deleteInstance (nameSpace, instanceName);

        CIMObjectPath tmpPath = instanceName;
        tmpPath.setNameSpace(nameSpace);
        _normalizedSubscriptionTable->remove(tmpPath);
    }
    else if (instanceName.getClassName().equal(
                 PEGASUS_CLASSNAME_INDFILTER) ||
             instanceName.getClassName().equal(
                 PEGASUS_CLASSNAME_INDHANDLER_CIMXML) ||
             instanceName.getClassName().equal(
                 PEGASUS_CLASSNAME_LSTNRDST_CIMXML) ||
             instanceName.getClassName().equal(
                 PEGASUS_CLASSNAME_INDHANDLER_SNMP) ||
             instanceName.getClassName().equal(
                 PEGASUS_CLASSNAME_LSTNRDST_FILE) ||
             instanceName.getClassName().equal(
                 PEGASUS_CLASSNAME_LSTNRDST_EMAIL) ||
             instanceName.getClassName().equal(
                 PEGASUS_CLASSNAME_LSTNRDST_SYSTEM_LOG) ||
             instanceName.getClassName().equal(
                 PEGASUS_CLASSNAME_INDHANDLER_WSMAN))
    {
        AutoMutex mtx(_handlerFilterCacheMutex);

        _repository->deleteInstance (nameSpace, instanceName);

        // Try to remove the handler/filter from the cache.
        // It may not have been added there as it was not used for any
        // indication processing yet, so we don't care when the remove
        // fails.
        String objName = _getHandlerFilterCacheKey(instanceName, nameSpace);
        _handlerFilterCache.evict(objName);
    }
    else
    {
        _repository->deleteInstance (nameSpace, instanceName);
    }
}

Array <CIMInstance> SubscriptionRepository::enumerateInstancesForClass (
    const CIMNamespaceName & nameSpace,
    const CIMName & className,
    Boolean includeQualifiers,
    Boolean includeClassOrigin,
    const CIMPropertyList & propertyList)
{
    return _repository->enumerateInstancesForClass (nameSpace, className,
        includeQualifiers, includeClassOrigin, propertyList);
}

Array <CIMObjectPath> SubscriptionRepository::enumerateInstanceNamesForClass (
    const CIMNamespaceName & nameSpace,
    const CIMName & className)
{
    return _repository->enumerateInstanceNamesForClass(nameSpace, className);
}

void SubscriptionRepository::_disableSubscription (
    CIMInstance subscription)
{
    PEG_METHOD_ENTER (TRC_INDICATION_SERVICE,
        "SubscriptionRepository::_disableSubscription");

    //
    //  Create property list
    //
    CIMPropertyList propertyList;
    Array <CIMName> properties;
    properties.append (PEGASUS_PROPERTYNAME_SUBSCRIPTION_STATE);
    propertyList = CIMPropertyList (properties);

    //
    //  Set Time of Last State Change to current date time
    //
    CIMInstance instance = subscription;
    CIMDateTime currentDateTime = CIMDateTime::getCurrentDateTime ();
    if (instance.findProperty (_PROPERTY_LASTCHANGE) == PEG_NOT_FOUND)
    {
        instance.addProperty
            (CIMProperty (_PROPERTY_LASTCHANGE, currentDateTime));
    }
    else
    {
        CIMProperty lastChange = instance.getProperty
            (instance.findProperty (_PROPERTY_LASTCHANGE));
        lastChange.setValue (CIMValue (currentDateTime));
    }

    //
    //  Set Subscription State to Disabled
    //
    CIMProperty state = instance.getProperty (instance.findProperty
        (PEGASUS_PROPERTYNAME_SUBSCRIPTION_STATE));
    state.setValue(CIMValue(Uint16(STATE_DISABLED)));

    //
    //  Modify the instance in the repository
    //
    try
    {
        _repository->modifyInstance
            (subscription.getPath ().getNameSpace (),
            subscription, false, propertyList);
    }
    catch (Exception & exception)
    {
        PEG_TRACE((TRC_INDICATION_SERVICE, Tracer::LEVEL1,
           "Exception caught in attempting to disable a subscription: %s",
            (const char*)exception.getMessage().getCString()));
    }

    PEG_METHOD_EXIT ();
}

void SubscriptionRepository::_deleteSubscription (
    const CIMInstance &subscription)
{
    PEG_METHOD_ENTER (TRC_INDICATION_SERVICE,
        "SubscriptionRepository::_deleteSubscription");

    //
    //  Delete subscription instance from repository
    //
    try
    {
        deleteInstance
            (subscription.getPath ().getNameSpace (),
            subscription.getPath ());
    }
    catch (Exception & exception)
    {
        PEG_TRACE((TRC_INDICATION_SERVICE, Tracer::LEVEL1,
           "Exception caught in attempting to delete a subscription: %s",
           (const char*)exception.getMessage().getCString()));
    }

    PEG_METHOD_EXIT ();
}

PEGASUS_NAMESPACE_END
