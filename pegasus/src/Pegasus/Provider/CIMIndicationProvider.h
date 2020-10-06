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

#ifndef Pegasus_CIMIndicationProvider_h
#define Pegasus_CIMIndicationProvider_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Provider/CIMProvider.h>

#include <Pegasus/Common/Array.h>
#include <Pegasus/Common/CIMObjectPath.h>
#include <Pegasus/Common/CIMDateTime.h>
#include <Pegasus/Common/CIMIndication.h>
#include <Pegasus/Provider/Linkage.h>

PEGASUS_NAMESPACE_BEGIN

/**
    This class defines the set of methods implemented by an indication
    provider.  A provider that derives from this class must implement all
    methods.  A minimal method implementation may simply throw a
    CIMNotSupportedException.
*/
class PEGASUS_PROVIDER_LINKAGE CIMIndicationProvider :
    public virtual CIMProvider
{
public:
    /**
        Constructs a default CIMIndicationProvider object.
    */
    CIMIndicationProvider();

    /**
        Destructs a CIMIndicationProvider object.
    */
    virtual ~CIMIndicationProvider();

    /**
        Instructs the provider to begin generating indications.  One or
        more createSubscription calls may precede the call to this method.
        (On start-up, createSubscription is called for each active
        subscription before the enableIndications method is called, so that
        the provider knows about all outstanding subscriptions before it
        begins generating indications.)

        The provider must set the namespace and class name in the object path
        of the generated indication instance (to allow the Indication Processor
        to filter generated indications based on the Filter SourceNamespace and
        Query indication class name), as in the following example:

        <pre>
        CIMObjectPath path(
            String::EMPTY,
            CIMNamespaceName("root/IndicationSource"),
            CIMName("IndicationClassName"),
            Array<CIMKeyBinding>());
        indicationInstance.setPath(path);
        </pre>

        where "root/IndicationSource" is the namespace from which the
        indication was generated, and "IndicationClassName" is the class name
        of the generated indication.  Note that indication instance paths never
        have key bindings.

        An exception thrown by this method is considered a provider error
        and does not affect CIM Server behavior.

        @param handler An IndicationResponseHandler used by the provider to
            deliver the indications it generates.  The IndicationResponseHandler
            object remains valid until the disableIndications method is called.
            After disableIndications is called, the behavior of the
            IndicationResponseHandler is undefined.
    */
    virtual void enableIndications(IndicationResponseHandler & handler) = 0;

    /**
        Instructs the provider to stop generating indications.  After this
        call, the IndicationResponseHandler given to the provider in the
        enableIndications method is no longer valid and its behavior is
        undefined.  Information regarding active subscriptions no longer
        applies to the provider and should be discarded at this time.
        (The provider will receive new createSubscription calls to restore
        necessary state before enableIndications is called again.)

        An exception thrown by this method is considered a provider error
        and does not affect CIM Server behavior.
    */
    virtual void disableIndications() = 0;

    /**
        Instructs the provider to monitor for indications matching the
        specified subscription.  The provider may reject the subscription by
        throwing a CIMNotSupportedException.  An indication provider must
        reject the subscription as not supported if the provider cannot carry
        out the specified RepeatNotificationPolicy with the specified
        RepeatNotificationInterval, RepeatNotificationGap and
        RepeatNotificationCount.

        Note that it is not necessary for a simple indication provider to
        take any action on createSubscription, modifySubscription, or
        deleteSubscription.  It may simply generate indications (or not)
        based on the enableIndications and disableIndications calls.

        An indication provider is not required to retain or make use of any
        specific subscription information.  (A provider that accepts a
        subscription with a RepeatNotificationPolicy value other than "None",
        however, requires this data for proper operation.)

        An indication should only be generated once regardless of the
        number of active subscriptions.  The indication will be sent to
        each applicable handler.  If an indication should not be sent to all
        subscribers (such as when the provider has accepted one or more
        subscriptions with a RepeatNotificationPolicy defined), the provider
        must include in a SubscriptionInstanceNamesContainer the list of
        subscriptions for which the indication is intended.

        @param context An OperationContext object containing the context for
            the processing of the subscription.  The context includes the name
            of the subscribing user, language information, and other data.
            Of particular interest to indication providers are the
            SubscriptionInstanceContainer and
            SubscriptionFilterConditionContainer containers.  The
            SubscriptionInstanceContainer contains the full
            CIM_IndicationSubscription instance (including, for example, more
            information about repeat notification policies).  The
            SubscriptionFilterConditionContainer contains the query string and
            query language from the CIM_IndicationFilter instance.

        @param subscriptionName A CIMObjectPath containing the name of the
            CIM_IndicationSubscription instance for which the provider is
            requested to generate indications.

        @param classNames Contains a list of indication class names for
            which the provider has registered as an indication provider and are
            included in the CIM_IndicationFilter for this subscription instance.
            A provider may use this as medium-level data for controlling which
            indications it generates.

        @param propertyList A CIMPropertyList specifying the minimum set of
            properties required in the indications generated for this
            subscription.  Support for this parameter is optional, so the
            generated indications may contain properties not specified in the
            list.  A null propertyList indicates that all properties must be
            included.  A non-null, but empty, propertyList indicates that no
            properites are required.

        @param repeatNotificationPolicy Contains the value of the property
            with the same name in the CIM_IndicationSubscription instance.
            Most providers will probably not support repeat notification
            policies, and should throw a CIMNotSupportedException if the value
            is anything other than "None".

        @exception CIMNotSupportedException
    */
    virtual void createSubscription(
        const OperationContext & context,
        const CIMObjectPath & subscriptionName,
        const Array<CIMObjectPath> & classNames,
        const CIMPropertyList & propertyList,
        const Uint16 repeatNotificationPolicy) = 0;

    /**
        Informs the provider that a subscription instance it is serving has
        changed.

        @param context An OperationContext object containing the context for
            the processing of the subscription.  The context includes the name
            of the subscribing user, language information, and other data.
            Of particular interest to indication providers are the
            SubscriptionInstanceContainer and
            SubscriptionFilterConditionContainer containers.  The
            SubscriptionInstanceContainer contains the full
            CIM_IndicationSubscription instance (including, for example, more
            information about repeat notification policies).  The
            SubscriptionFilterConditionContainer contains the query string and
            query language from the CIM_IndicationFilter instance.

        @param subscriptionName A CIMObjectPath containing the name of the
            CIM_IndicationSubscription instance that is being modified and for
            which the provider has been requested to generate indications.

        @param classNames Contains a list of indication class names for
            which the provider has registered as an indication provider and are
            included in the CIM_IndicationFilter for this subscription instance.
            A provider may use this as medium-level data for controlling which
            indications it generates.

        @param propertyList A CIMPropertyList specifying the minimum set of
            properties required in the indications generated for this
            subscription.  Support for this parameter is optional, so the
            generated indications may contain properties not specified in the
            list.  A null propertyList indicates that all properties must be
            included.  A non-null, but empty, propertyList indicates that no
            properites are required.

        @param repeatNotificationPolicy Contains the value of the property
            with the same name in the CIM_IndicationSubscription instance.
            Most providers will probably not support repeat notification
            policies, and should throw a CIMNotSupportedException if the value
            is anything other than "None".

        @exception CIMNotSupportedException
    */
    virtual void modifySubscription(
        const OperationContext & context,
        const CIMObjectPath & subscriptionName,
        const Array<CIMObjectPath> & classNames,
        const CIMPropertyList & propertyList,
        const Uint16 repeatNotificationPolicy) = 0;

    /**
        Instructs the provider to stop generating indications for a specified
        subscription.

        An exception thrown by this method is considered a provider error
        and does not affect CIM Server behavior.

        @param context An OperationContext object containing the context for
            the processing of the operation.  The context includes the name
            of the requesting user, language information, and other data.

        @param subscriptionName A CIMObjectPath containing the name of the
            CIM_IndicationSubscription instance for which the provider is
            requested to stop generating indications.

        @param classNames Contains a list of indication class names for
            which the provider has registered as an indication provider and are
            included in the CIM_IndicationFilter for this subscription instance.
            A provider may use this as medium-level data for controlling which
            indications it generates.
    */
    virtual void deleteSubscription(
        const OperationContext & context,
        const CIMObjectPath & subscriptionName,
        const Array<CIMObjectPath> & classNames) = 0;
};

PEGASUS_NAMESPACE_END

#endif
