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
#include <Pegasus/Common/CIMDateTime.h>
#include <Pegasus/Common/PegasusAssert.h>
#include <Pegasus/Common/System.h>

#include "OOPModuleFailureTestProvider.h"

PEGASUS_USING_STD;

PEGASUS_USING_PEGASUS;

static IndicationResponseHandler * _handler = 0;
static Boolean _enabled = false;
static Uint32 _numSubscriptions = 0;
static String _providerName;

extern "C" PEGASUS_EXPORT CIMProvider * PegasusCreateProvider (
    const String & providerName)
{
    _providerName = providerName;
    if (String::equalNoCase (providerName, "OOPModuleInitFailureTestProvider"))
    {
        return (new OOPModuleFailureTestProvider ());
    }
    else if (String::equalNoCase
        (providerName, "OOPModuleCreateFailureTestProvider"))
    {
        return (new OOPModuleFailureTestProvider ());
    }
    else if (String::equalNoCase
        (providerName, "OOPModuleCreate2FailureTestProvider"))
    {
        return (new OOPModuleFailureTestProvider ());
    }
    else if (String::equalNoCase
        (providerName, "OOPModuleEnableFailureTestProvider"))
    {
        return (new OOPModuleFailureTestProvider ());
    }
    else if (String::equalNoCase
        (providerName, "OOPModuleModifyFailureTestProvider"))
    {
        return (new OOPModuleFailureTestProvider ());
    }
    else if (String::equalNoCase
        (providerName, "OOPModuleInvokeFailureTestProvider"))
    {
        return (new OOPModuleFailureTestProvider ());
    }
    else if (String::equalNoCase
        (providerName, "OOPModuleDeleteFailureTestProvider"))
    {
        return (new OOPModuleFailureTestProvider ());
    }
    else if (String::equalNoCase
        (providerName, "OOPModuleDelete2FailureTestProvider"))
    {
        return (new OOPModuleFailureTestProvider ());
    }
    else if (String::equalNoCase
        (providerName, "OOPModuleDisableFailureTestProvider"))
    {
        return (new OOPModuleFailureTestProvider ());
    }
    else if (String::equalNoCase
        (providerName, "OOPModuleTermFailureTestProvider"))
    {
        return (new OOPModuleFailureTestProvider ());
    }
    return (0);
}

void _generateIndication (
    IndicationResponseHandler * handler,
    const String & identifier)
{
    if (_enabled)
    {
        CIMInstance indicationInstance (CIMName ("FailureTestIndication"));

        CIMObjectPath path;
        path.setNameSpace ("test/testProvider");
        path.setClassName ("FailureTestIndication");

        indicationInstance.setPath (path);

        indicationInstance.addProperty
            (CIMProperty ("IndicationIdentifier", identifier));

        CIMDateTime currentDateTime = CIMDateTime::getCurrentDateTime ();
        indicationInstance.addProperty
            (CIMProperty ("IndicationTime", currentDateTime));

        Array <String> correlatedIndications;
        indicationInstance.addProperty
            (CIMProperty ("CorrelatedIndications", correlatedIndications));

        indicationInstance.addProperty
            (CIMProperty ("Description", String ("Failure Test Indication")));

        indicationInstance.addProperty
            (CIMProperty ("AlertingManagedElement",
            System::getFullyQualifiedHostName ()));

        indicationInstance.addProperty
            (CIMProperty ("AlertingElementFormat", Uint16 (0)));

        indicationInstance.addProperty
            (CIMProperty ("AlertType", Uint16 (1)));

        indicationInstance.addProperty
            (CIMProperty ("OtherAlertType", String ("Test")));

        indicationInstance.addProperty
            (CIMProperty ("PerceivedSeverity", Uint16 (2)));

        indicationInstance.addProperty
            (CIMProperty ("ProbableCause", Uint16 (1)));

        indicationInstance.addProperty
            (CIMProperty ("ProbableCauseDescription", String ("Test")));

        indicationInstance.addProperty
            (CIMProperty ("Trending", Uint16 (4)));

        Array <String> recommendedActions;
        recommendedActions.append ("Test");
        indicationInstance.addProperty
            (CIMProperty ("RecommendedActions", recommendedActions));

        indicationInstance.addProperty
            (CIMProperty ("EventID", String ("Test")));

        CIMDateTime eventTime = CIMDateTime::getCurrentDateTime ();
        indicationInstance.addProperty
            (CIMProperty ("EventTime", eventTime));

        indicationInstance.addProperty
            (CIMProperty ("SystemCreationClassName",
            System::getSystemCreationClassName ()));

        indicationInstance.addProperty
            (CIMProperty ("SystemName",
            System::getFullyQualifiedHostName ()));

        indicationInstance.addProperty
            (CIMProperty ("ProviderName", _providerName));

        CIMIndication cimIndication (indicationInstance);

        handler->deliver (cimIndication);
    }
}

OOPModuleFailureTestProvider::OOPModuleFailureTestProvider ()
{
}

OOPModuleFailureTestProvider::~OOPModuleFailureTestProvider ()
{
}

void OOPModuleFailureTestProvider::initialize (CIMOMHandle & cimom)
{
    //
    // save cimom handle
    //
    _cimom = cimom;

    //
    //  If I am the OOPModuleInitFailureTestProvider, fail (i.e. exit)
    //
    if (String::equalNoCase (_providerName, "OOPModuleInitFailureTestProvider"))
    {
        exit (-1);
    }
}

void OOPModuleFailureTestProvider::terminate ()
{
    //
    //  If I am the OOPModuleTermFailureTestProvider, and there are
    //  subscriptions, fail (i.e. exit)
    //
    if (String::equalNoCase (_providerName, "OOPModuleTermFailureTestProvider"))
    {
        if (_numSubscriptions > 0)
        {
            exit (-1);
        }
    }

    delete this;
}

void OOPModuleFailureTestProvider::enableIndications (
    IndicationResponseHandler & handler)
{
    //
    //  enableIndications should not be called if indications have already been
    //  enabled
    //
    if (_enabled)
    {
        PEGASUS_TEST_ASSERT (false);
    }

    _enabled = true;
    _handler = &handler;

    //
    //  If I am the OOPModuleEnableFailureTestProvider, fail (i.e. exit)
    //
    if (String::equalNoCase (_providerName,
        "OOPModuleEnableFailureTestProvider"))
    {
        exit (-1);
    }
}

void OOPModuleFailureTestProvider::disableIndications ()
{
    _enabled = false;
    _handler->complete ();

    //
    //  If I am the OOPModuleDisableFailureTestProvider, fail (i.e. exit)
    //
    if (String::equalNoCase (_providerName,
        "OOPModuleDisableFailureTestProvider"))
    {
        exit (-1);
    }
}

void OOPModuleFailureTestProvider::createSubscription (
    const OperationContext & context,
    const CIMObjectPath & subscriptionName,
    const Array <CIMObjectPath> & classNames,
    const CIMPropertyList & propertyList,
    const Uint16 repeatNotificationPolicy)
{
    _numSubscriptions++;

    //
    //  If I am the OOPModuleCreateFailureTestProvider, fail (i.e. exit)
    //
    if (String::equalNoCase (_providerName,
        "OOPModuleCreateFailureTestProvider"))
    {
        exit (-1);
    }

    //
    //  If I am the OOPModuleCreate2FailureTestProvider, and this subscription
    //  is not my first, fail (i.e. exit)
    //
    else if (String::equalNoCase (_providerName,
        "OOPModuleCreate2FailureTestProvider"))
    {
        if (_numSubscriptions > 1)
        {
            exit (-1);
        }
    }
}

void OOPModuleFailureTestProvider::modifySubscription (
    const OperationContext & context,
    const CIMObjectPath & subscriptionName,
    const Array <CIMObjectPath> & classNames,
    const CIMPropertyList & propertyList,
    const Uint16 repeatNotificationPolicy)
{
    //
    //  If I am the OOPModuleModifyFailureTestProvider, fail (i.e. exit)
    //
    if (String::equalNoCase (_providerName,
        "OOPModuleModifyFailureTestProvider"))
    {
        exit (-1);
    }
}

void OOPModuleFailureTestProvider::deleteSubscription (
    const OperationContext & context,
    const CIMObjectPath & subscriptionName,
    const Array <CIMObjectPath> & classNames)
{
    _numSubscriptions--;

    if (_numSubscriptions == 0)
        _enabled = false;

    //
    //  If I am the OOPModuleDeleteFailureTestProvider, fail (i.e. exit)
    //
    if (String::equalNoCase (_providerName,
        "OOPModuleDeleteFailureTestProvider"))
    {
        exit (-1);
    }

    //
    //  If I am the OOPModuleDelete2FailureTestProvider, and there are
    //  remaining subscriptions, fail (i.e. exit)
    //
    else if (String::equalNoCase (_providerName,
        "OOPModuleDelete2FailureTestProvider"))
    {
        if (_numSubscriptions > 0)
        {
            exit (-1);
        }
    }
}

void OOPModuleFailureTestProvider::invokeMethod (
    const OperationContext & context,
    const CIMObjectPath & objectReference,
    const CIMName & methodName,
    const Array <CIMParamValue> & inParameters,
    MethodResultResponseHandler & handler)
{
    Boolean sendIndication = false;
    String identifier;
    handler.processing ();

    if (objectReference.getClassName ().equal ("FailureTestIndication") &&
        _enabled)
    {
        if (methodName.equal ("SendTestIndication"))
        {
            if ((inParameters.size() > 0) &&
                (inParameters[0].getParameterName () == "identifier"))
            {
                inParameters[0].getValue().get(identifier);
            }
            sendIndication = true;
            handler.deliver (CIMValue (0));
        }
    }
    else
    {
         handler.deliver (CIMValue (1));
         PEGASUS_STD (cout) << "Provider is not enabled." <<
         PEGASUS_STD (endl);
    }

    handler.complete ();

    if (sendIndication)
    {
        _generateIndication (_handler, identifier);
    }

    //
    //  If I am the OOPModuleInvokeFailureTestProvider, fail (i.e. exit)
    //
    if (String::equalNoCase (_providerName,
        "OOPModuleInvokeFailureTestProvider"))
    {
        if (methodName.equal ("Fail"))
        {
            exit (-1);
        }
    }
}
