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

#include <Pegasus/Common/PegasusAssert.h>

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/Constants.h>
#include <Pegasus/Common/CIMDateTime.h>
#include <Pegasus/Common/OperationContext.h>
#include <Pegasus/Common/System.h>
#include <Pegasus/Query/QueryExpression/QueryExpression.h>
#include <Pegasus/Provider/CIMOMHandleQueryContext.h>

#include "IndicationTestProvider.h"

PEGASUS_USING_STD;

PEGASUS_USING_PEGASUS;

static IndicationResponseHandler * _handler = 0;
static Boolean _enabled = false;
static Uint32 _nextUID = 0;
static Uint32 _numSubscriptions = 0;

IndicationTestProvider::IndicationTestProvider() throw ()
{
#if defined(PEGASUS_OS_DARWIN)
    _handler = 0;
    _enabled = false;
    _nextUID = 0;
    _numSubscriptions = 0;
#endif
}

IndicationTestProvider::~IndicationTestProvider() throw ()
{
}

void IndicationTestProvider::initialize(CIMOMHandle& cimom)
{
  _cimom = cimom;
}

void IndicationTestProvider::terminate()
{
    delete this;
}

void IndicationTestProvider::enableIndications(
    IndicationResponseHandler& handler)
{
    //
    //  enableIndications should not be called if indications have already been
    //  enabled
    //
    if (_enabled)
    {
        PEGASUS_TEST_ASSERT(false);
    }

    _enabled = true;
    _handler = &handler;
}

void _generateIndication(
    IndicationResponseHandler * handler,
    const CIMName methodName,
    Uint32 indicationNumber)
{
    if (_enabled)
    {
        CIMInstance indicationInstance;

        if (methodName.equal("SendTestIndicationSubclass"))
        {
            indicationInstance =
                CIMInstance(CIMName("Test_IndicationProviderSubclass"));
        }
        else
        {
            indicationInstance =
                CIMInstance(CIMName("Test_IndicationProviderClass"));
        }

        CIMObjectPath path;
        if (methodName.equal("SendTestIndicationUnmatchingNamespace"))
        {
            //
            //  For SendTestIndicationUnmatchingNamespace, generate an
            //  indication instance with namespace that does not match the
            //  subscription instance name included in the operation context
            //  (nor does it match the namespace for which provider has
            //  registered)
            //
            path.setNameSpace("root/cimv2");
            path.setClassName("Test_IndicationProviderClass");
        }
        else if (methodName.equal("SendTestIndicationUnmatchingClassName"))
        {
            // the indication class name and object path class must match
            indicationInstance = CIMInstance("CIM_AlertIndication");

            //
            //  For SendTestIndicationUnmatchingClassName, generate an
            //  indication instance with classname that does not match the
            //  subscription instance name included in the operation context
            //  (nor does it match the classname for which provider has
            //  registered)
            //
            path.setNameSpace("test/TestProvider");
            path.setClassName("CIM_AlertIndication");
        }
        else if (methodName.equal("SendTestIndicationSubclass"))
        {
            //
            //  For SendTestIndicationSubclass, generate an indication instance
            //  of a the Test_IndicationProviderSubclass subclass
            //
            path.setNameSpace("test/TestProvider");
            path.setClassName("Test_IndicationProviderSubclass");
        }
        else
        {
            path.setNameSpace("test/TestProvider");
            path.setClassName("Test_IndicationProviderClass");
        }

        indicationInstance.setPath(path);

        char buffer[32];
        sprintf(buffer, "%u", ++_nextUID);
        indicationInstance.addProperty(
            CIMProperty("IndicationIdentifier", String(buffer)));

        CIMDateTime currentDateTime = CIMDateTime::getCurrentDateTime();
        indicationInstance.addProperty(
            CIMProperty("IndicationTime", currentDateTime));

        //
        //  For SendTestIndicationMissingProperty, leave out the
        //  CorrelatedIndications property
        //
        if (!methodName.equal("SendTestIndicationMissingProperty"))
        {
            Array <String> correlatedIndications;
            indicationInstance.addProperty(
                CIMProperty("CorrelatedIndications", correlatedIndications));
        }

        if ((methodName.equal("SendTestIndicationNormal")) ||
            (methodName.equal("SendTestIndicationsCount")) ||
            (methodName.equal("SendTestIndicationSubclass")) ||
            (methodName.equal("SendTestIndicationMissingProperty")) ||
            (methodName.equal("SendTestIndicationExtraProperty")) ||
            (methodName.equal("SendTestIndicationMatchingInstance")) ||
            (methodName.equal("SendTestIndicationUnmatchingNamespace")) ||
            (methodName.equal("SendTestIndicationUnmatchingClassName")))
        {
            indicationInstance.addProperty(
                CIMProperty("MethodName", CIMValue(methodName.getString())));
        }
        else if (methodName.equal("SendTestIndicationTrap"))
        {
            indicationInstance.addProperty(
                CIMProperty(
                    "MethodName",
                    CIMValue(methodName.getString())));

            indicationInstance.addProperty(
                CIMProperty(
                    "IndicationNumber",
                    CIMValue(Uint64(indicationNumber))));

            indicationInstance.addProperty(
                CIMProperty(
                    "TestOidDataType",
                    String("1.3.6.1.4.1.892.2.3.5006.5002")));
        }
        else
        {
            indicationInstance.addProperty(
                CIMProperty(
                    "MethodName",
                    CIMValue(String("generateIndication"))));
        }

        //
        //  For SendTestIndicationExtraProperty, add an extra property,
        //  ExtraProperty, that is not a member of the indication class
        //
        if (methodName.equal("SendTestIndicationExtraProperty"))
        {
            indicationInstance.addProperty(
                CIMProperty(
                    "ExtraProperty",
                    CIMValue(String("extraProperty"))));
        }

        CIMIndication cimIndication(indicationInstance);

        //
        //  For SendTestIndicationSubclass,
        //  SendTestIndicationMatchingInstance,
        //  SendTestIndicationUnmatchingNamespace or
        //  SendTestIndicationUnmatchingClassName, include
        //  SubscriptionInstanceNamesContainer in operation context
        //
        if ((methodName.equal("SendTestIndicationSubclass")) ||
            (methodName.equal("SendTestIndicationMatchingInstance")) ||
            (methodName.equal("SendTestIndicationUnmatchingNamespace")) ||
            (methodName.equal("SendTestIndicationUnmatchingClassName")))
        {
            Array <CIMObjectPath> subscriptionInstanceNames;
            Array <CIMKeyBinding> subscriptionKeyBindings;

            String filterString;
            filterString.append(
                "CIM_IndicationFilter."
                    "CreationClassName=\"CIM_IndicationFilter\","
                    "Name=\"PIFilter01\","
                    "SystemCreationClassName=\"");
            filterString.append(System::getSystemCreationClassName());
            filterString.append("\",SystemName=\"");
            filterString.append(System::getFullyQualifiedHostName());
            filterString.append("\"");
            subscriptionKeyBindings.append(CIMKeyBinding(
                "Filter", filterString, CIMKeyBinding::REFERENCE));

            String handlerString;
            handlerString.append(
                "CIM_IndicationHandlerCIMXML."
                    "CreationClassName=\"CIM_IndicationHandlerCIMXML\","
                    "Name=\"PIHandler01\","
                    "SystemCreationClassName=\"");
            handlerString.append(System::getSystemCreationClassName());
            handlerString.append("\",SystemName=\"");
            handlerString.append(System::getFullyQualifiedHostName());
            handlerString.append("\"");
            subscriptionKeyBindings.append(CIMKeyBinding(
                "Handler", handlerString, CIMKeyBinding::REFERENCE));

            CIMObjectPath subscriptionPath("",
                PEGASUS_NAMESPACENAME_INTEROP,
                CIMName("CIM_IndicationSubscription"),
                subscriptionKeyBindings);
            subscriptionInstanceNames.append(subscriptionPath);

            OperationContext context;
            context.insert(SubscriptionInstanceNamesContainer(
                subscriptionInstanceNames));

            handler->deliver(context, indicationInstance);
        }
        else
        {
            // deliver an indication without trapOid
            handler->deliver(indicationInstance);
        }

        //
        //  Only deliver extra indication with trapOid for SendTestIndication
        //
        if ((!methodName.equal("SendTestIndicationNormal")) &&
            (!methodName.equal("SendTestIndicationsCount")) &&
            (!methodName.equal("SendTestIndicationTrap")) &&
            (!methodName.equal("SendTestIndicationSubclass")) &&
            (!methodName.equal("SendTestIndicationMissingProperty")) &&
            (!methodName.equal("SendTestIndicationExtraProperty")) &&
            (!methodName.equal("SendTestIndicationMatchingInstance")) &&
            (!methodName.equal("SendTestIndicationUnmatchingNamespace")) &&
            (!methodName.equal("SendTestIndicationUnmatchingClassName")))
        {
            // deliver another indication with a trapOid which contains in the
            // operationContext container
            OperationContext context;

            // add trap OID to the context
            context.insert(
                SnmpTrapOidContainer("1.3.6.1.4.1.900.2.3.9002.9600"));
            handler->deliver(context, indicationInstance);
        }
    }
}

void IndicationTestProvider::disableIndications()
{
    _enabled = false;
    _handler->complete();
}

void IndicationTestProvider::createSubscription(
    const OperationContext& context,
    const CIMObjectPath& subscriptionName,
    const Array <CIMObjectPath>& classNames,
    const CIMPropertyList& propertyList,
    const Uint16 repeatNotificationPolicy)
{
    String funcName("IndicationTestProvider::createSubscription ");
    _checkOperationContext(context, funcName);

    _numSubscriptions++;
}

void IndicationTestProvider::modifySubscription(
    const OperationContext& context,
    const CIMObjectPath& subscriptionName,
    const Array <CIMObjectPath>& classNames,
    const CIMPropertyList& propertyList,
    const Uint16 repeatNotificationPolicy)
{
    String funcName("IndicationTestProvider::modifySubscription ");
    _checkOperationContext(context, funcName);

    _generateIndication(_handler, "modifySubscription", 0);
}

void IndicationTestProvider::deleteSubscription(
    const OperationContext& context,
    const CIMObjectPath& subscriptionName,
    const Array <CIMObjectPath>& classNames)
{
    _numSubscriptions--;

    if (_numSubscriptions == 0)
        _enabled = false;
}

void IndicationTestProvider::invokeMethod(
    const OperationContext& context,
    const CIMObjectPath& objectReference,
    const CIMName& methodName,
    const Array<CIMParamValue>& inParameters,
    MethodResultResponseHandler& handler)
{
    Uint32 indicationSendCount = 1;
    Boolean sendIndication = false;
    handler.processing();

    if (objectReference.getClassName().equal("Test_IndicationProviderClass") &&
        _enabled)
    {
        if ((methodName.equal("SendTestIndication")) ||
            (methodName.equal("SendTestIndicationNormal")) ||
            (methodName.equal("SendTestIndicationMissingProperty")) ||
            (methodName.equal("SendTestIndicationExtraProperty")) ||
            (methodName.equal("SendTestIndicationMatchingInstance")) ||
            (methodName.equal("SendTestIndicationUnmatchingNamespace")) ||
            (methodName.equal("SendTestIndicationUnmatchingClassName")))
        {
            sendIndication = true;
            handler.deliver(CIMValue(0));
        }
        else if ((methodName.equal("SendTestIndicationTrap")) ||
                 (methodName.equal("SendTestIndicationsCount")))
        {
            inParameters[0].getValue().get(indicationSendCount);
            sendIndication = true;
            handler.deliver( CIMValue( 0 ) );
        }
        else if (methodName.equal("GetSubscriptionCount"))
        {
            handler.deliver(CIMValue(_numSubscriptions));
        }
    }

    else if ((objectReference.getClassName().equal(
                 "Test_IndicationProviderSubclass")) &&
             _enabled)
    {
        if (methodName.equal("SendTestIndicationSubclass"))
        {
            sendIndication = true;
            handler.deliver( CIMValue( 0 ) );
        }
    }

    else
    {
         handler.deliver( CIMValue( 1 ) );
         PEGASUS_STD(cout) << "Provider is not enabled." <<
             PEGASUS_STD(endl);
    }

    handler.complete();

    if (sendIndication)
    {
        for (Uint32 numberOfInd = 1; numberOfInd <= indicationSendCount;
             numberOfInd++)
        {
            _generateIndication(_handler, methodName, numberOfInd);
        }
    }
}

void IndicationTestProvider::_checkOperationContext(
    const OperationContext& context,
    const String& funcName)
{
    //
    // Test the filter query container
    //
    SubscriptionFilterQueryContainer qContainer =
        context.get(SubscriptionFilterQueryContainer::NAME);
    if (qContainer.getFilterQuery() == String::EMPTY)
    {
        PEGASUS_STD(cout) << funcName << "- empty filter query" <<
            PEGASUS_STD(endl);
        throw CIMOperationFailedException(funcName + "- empty filter query");
    }
    if (qContainer.getQueryLanguage() == String::EMPTY)
    {
        PEGASUS_STD(cout) << funcName << "- empty filter query lang" <<
            PEGASUS_STD(endl);
        throw CIMOperationFailedException(
            funcName + "- empty filter query lang");
    }

    CIMNamespaceName tst("test/TestProvider");
    if (!qContainer.getSourceNameSpace().equal(tst))
    {
        PEGASUS_STD(cout) << funcName << "- incorrect source namespace" <<
            PEGASUS_STD(endl);
        throw CIMOperationFailedException(
            funcName + "- incorrect source namespace");
    }

    try
    {
        //
        // Try to parse the filter query from the filter query container
        //
        CIMOMHandleQueryContext ctx(qContainer.getSourceNameSpace(), _cimom);
        QueryExpression qe(qContainer.getQueryLanguage(),
                           qContainer.getFilterQuery(),
                           ctx);

        // Exercise the QueryExpression...this will cause repository access
        // through the CIMOMHandleQueryContext.
        qe.validate();
    }
    catch (Exception& e)
    {
        PEGASUS_STD(cout) << funcName << "- parse error: " <<
            e.getMessage() << PEGASUS_STD(endl);
        throw CIMOperationFailedException(
            funcName + "- parse error: " + e.getMessage());
    }

    //
    // Test the filter condition container.
    // Note:  since this only contains the WHERE clause, the condition could
    // be empty (and will be for some testcases)
    //
    SubscriptionFilterConditionContainer cContainer =
        context.get(SubscriptionFilterConditionContainer::NAME);
    if (cContainer.getQueryLanguage() == String::EMPTY)
    {
        PEGASUS_STD(cout) << funcName << "- empty filter condition lang" <<
            PEGASUS_STD(endl);
        throw CIMOperationFailedException(
            funcName + "- empty filter condition lang");
    }
}

