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
#include <Pegasus/Common/CIMDateTime.h>
#include <Pegasus/Common/CIMName.h>

#include <Pegasus/Common/OperationContext.h>
#include <Pegasus/Common/OperationContextInternal.h>

#include <iostream>

PEGASUS_USING_PEGASUS;
PEGASUS_USING_STD;

static Boolean verbose;

CIMInstance _createFilterInstance1(void)
{
    CIMInstance filterInstance(PEGASUS_CLASSNAME_INDFILTER);

    // add properties
    filterInstance.addProperty(CIMProperty("SystemCreationClassName",
                                   String("CIM_UnitaryComputerSystem")));
    filterInstance.addProperty(CIMProperty("SystemName",
                                    String("server001.acme.com")));
    filterInstance.addProperty(CIMProperty("CreationClassName",
                                    PEGASUS_CLASSNAME_INDFILTER.getString()));
    filterInstance.addProperty(CIMProperty("Name",
                                   String("Filter1")));
    filterInstance.addProperty(CIMProperty("Query",
                                   String("SELECT * FROM CIM_AlertIndication"
                                          " WHERE AlertType = 5")));
    filterInstance.addProperty(CIMProperty("QueryLanguage",
                                   String("WQL1")));
    filterInstance.addProperty(CIMProperty("SourceNamespace",
                                   PEGASUS_NAMESPACENAME_INTEROP.getString()));

    // create keys
    Array<CIMKeyBinding> keys;

    keys.append(CIMKeyBinding("SystemCreationClassName",
                              "CIM_UnitaryComputerSystem",
                              CIMKeyBinding::STRING));
    keys.append(CIMKeyBinding("SystemName",
                              "server001.acme.com",
                              CIMKeyBinding::STRING));
    keys.append(CIMKeyBinding("CreationClassName",
                              PEGASUS_CLASSNAME_INDFILTER.getString(),
                              CIMKeyBinding::STRING));
    keys.append(CIMKeyBinding("Name", "Filter1", CIMKeyBinding::STRING));

    // update object path
    CIMObjectPath objectPath = filterInstance.getPath();

    objectPath.setKeyBindings(keys);

    filterInstance.setPath(objectPath);

    return(filterInstance);
}

CIMInstance _createHandlerInstance1(void)
{
    CIMInstance handlerInstance(PEGASUS_CLASSNAME_INDHANDLER_CIMXML);

    // add properties
    handlerInstance.addProperty(CIMProperty(
        "SystemCreationClassName", String("CIM_UnitaryComputerSystem")));
    handlerInstance.addProperty(CIMProperty(
        "SystemName", String("server001.acme.com")));
    handlerInstance.addProperty(CIMProperty(
        "CreationClassName", PEGASUS_CLASSNAME_INDHANDLER_CIMXML.getString()));
    handlerInstance.addProperty(CIMProperty(
        "Name", String("Handler1")));
    handlerInstance.addProperty(CIMProperty(
        "Destination", String("localhost:5988/test1")));

    // create keys
    Array<CIMKeyBinding> keys;

    keys.append(CIMKeyBinding("SystemCreationClassName",
                              "CIM_UnitaryComputerSystem",
                              CIMKeyBinding::STRING));
    keys.append(CIMKeyBinding("SystemName",
                              "server001.acme.com",
                              CIMKeyBinding::STRING));
    keys.append(CIMKeyBinding("CreationClassName",
                              PEGASUS_CLASSNAME_INDHANDLER_CIMXML.getString(),
                              CIMKeyBinding::STRING));
    keys.append(CIMKeyBinding(
        "Name", "Handler1", CIMKeyBinding::STRING));

    // update object path
    CIMObjectPath objectPath = handlerInstance.getPath();

    objectPath.setKeyBindings(keys);

    handlerInstance.setPath(objectPath);

    return(handlerInstance);
}

CIMInstance _createFilterInstance2(void)
{
    CIMInstance filterInstance(PEGASUS_CLASSNAME_INDFILTER);

    // add properties
    filterInstance.addProperty(CIMProperty(
        "SystemCreationClassName", String("CIM_UnitaryComputerSystem")));
    filterInstance.addProperty(CIMProperty(
        "SystemName", String("server001.acme.com")));
    filterInstance.addProperty(CIMProperty(
        "CreationClassName", PEGASUS_CLASSNAME_INDFILTER.getString()));
    filterInstance.addProperty(CIMProperty(
        "Name", String("Filter2")));
    filterInstance.addProperty(CIMProperty(
        "Query",
        String("SELECT * FROM CIM_AlertIndication WHERE AlertType = 8")));
    filterInstance.addProperty(CIMProperty(
        "QueryLanguage", String("WQL1")));
    filterInstance.addProperty(CIMProperty(
        "SourceNamespace", PEGASUS_NAMESPACENAME_INTEROP.getString()));

    // create keys
    Array<CIMKeyBinding> keys;

    keys.append(CIMKeyBinding("SystemCreationClassName",
                              "CIM_UnitaryComputerSystem",
                              CIMKeyBinding::STRING));
    keys.append(CIMKeyBinding("SystemName",
                              "server001.acme.com",
                              CIMKeyBinding::STRING));
    keys.append(CIMKeyBinding("CreationClassName",
                              PEGASUS_CLASSNAME_INDFILTER.getString(),
                              CIMKeyBinding::STRING));
    keys.append(CIMKeyBinding("Name", "Filter2", CIMKeyBinding::STRING));

    // update object path
    CIMObjectPath objectPath = filterInstance.getPath();

    objectPath.setKeyBindings(keys);

    filterInstance.setPath(objectPath);

    return(filterInstance);
}

CIMInstance _createHandlerInstance2(void)
{
    CIMInstance handlerInstance(PEGASUS_CLASSNAME_INDHANDLER_CIMXML);

    // add properties
    handlerInstance.addProperty(CIMProperty(
        "SystemCreationClassName", String("CIM_UnitaryComputerSystem")));
    handlerInstance.addProperty(CIMProperty(
        "SystemName", String("server001.acme.com")));
    handlerInstance.addProperty(CIMProperty(
        "CreationClassName", PEGASUS_CLASSNAME_INDHANDLER_CIMXML.getString()));
    handlerInstance.addProperty(CIMProperty(
        "Name", String("Handler2")));
    handlerInstance.addProperty(CIMProperty(
        "Destination", String("localhost:5988/test2")));

    // create keys
    Array<CIMKeyBinding> keys;

    keys.append(CIMKeyBinding("SystemCreationClassName",
                              "CIM_UnitaryComputerSystem",
                              CIMKeyBinding::STRING));
    keys.append(CIMKeyBinding("SystemName",
                              "server001.acme.com",
                              CIMKeyBinding::STRING));
    keys.append(CIMKeyBinding("CreationClassName",
                              PEGASUS_CLASSNAME_INDHANDLER_CIMXML.getString(),
                              CIMKeyBinding::STRING));
    keys.append(CIMKeyBinding("Name", "Handler2", CIMKeyBinding::STRING));

    // update object path
    CIMObjectPath objectPath = handlerInstance.getPath();

    objectPath.setKeyBindings(keys);

    handlerInstance.setPath(objectPath);

    return(handlerInstance);
}

CIMInstance _createSubscriptionInstance1(void)
{
    CIMInstance filterInstance1 = _createFilterInstance1();
    CIMInstance handlerInstance1 = _createHandlerInstance1();

    CIMInstance subscriptionInstance(PEGASUS_CLASSNAME_INDSUBSCRIPTION);

    // add properties
    subscriptionInstance.addProperty(CIMProperty(
        "Filter", filterInstance1.getPath(), 0,
        PEGASUS_CLASSNAME_INDFILTER));
    subscriptionInstance.addProperty(CIMProperty(
        "Handler", handlerInstance1.getPath(), 0,
        PEGASUS_CLASSNAME_INDHANDLER_CIMXML));
    subscriptionInstance.addProperty(CIMProperty(
        "OnFatalErrorPolicy", Uint16(4)));
    subscriptionInstance.addProperty(CIMProperty(
        "FailureTriggerTimeInterval", Uint64(60)));
    subscriptionInstance.addProperty(CIMProperty(
        "SubscriptionState", Uint16(2)));
    subscriptionInstance.addProperty(CIMProperty(
        "TimeOfLastStateChange", CIMDateTime::getCurrentDateTime()));
    subscriptionInstance.addProperty(CIMProperty(
        "SubscriptionDuration", Uint64(86400)));
    subscriptionInstance.addProperty(CIMProperty(
        "SubscriptionStartTime", CIMDateTime::getCurrentDateTime()));
    subscriptionInstance.addProperty(CIMProperty(
        "SubscriptionTimeRemaining", Uint64(86400)));
    subscriptionInstance.addProperty(CIMProperty(
        "RepeatNotificationPolicy", Uint16(1)));
    subscriptionInstance.addProperty(CIMProperty(
        "OtherRepeatNotificationPolicy", String("AnotherPolicy")));
    subscriptionInstance.addProperty(CIMProperty(
        "RepeatNotificationInterval", Uint64(60)));
    subscriptionInstance.addProperty(CIMProperty(
        "RepeatNotificationGap", Uint64(15)));
    subscriptionInstance.addProperty(CIMProperty(
        "RepeatNotificationCount", Uint16(3)));

    // create keys
    Array<CIMKeyBinding> keys;

    keys.append(CIMKeyBinding("Filter",
                              filterInstance1.getPath().toString(),
                              CIMKeyBinding::REFERENCE));
    keys.append(CIMKeyBinding("Handler",
                               handlerInstance1.getPath().toString(),
                              CIMKeyBinding::REFERENCE));

    // update object path
    CIMObjectPath objectPath = subscriptionInstance.getPath();

    objectPath.setKeyBindings(keys);

    subscriptionInstance.setPath(objectPath);

    return(subscriptionInstance);
}

CIMInstance _createSubscriptionInstance2(void)
{
    CIMInstance filterInstance2 = _createFilterInstance2();
    CIMInstance handlerInstance2 = _createHandlerInstance2();

    CIMInstance subscriptionInstance(PEGASUS_CLASSNAME_INDSUBSCRIPTION);

    // add properties
    subscriptionInstance.addProperty(CIMProperty(
        "Filter", filterInstance2.getPath(), 0,
        PEGASUS_CLASSNAME_INDFILTER));
    subscriptionInstance.addProperty(CIMProperty(
        "Handler", handlerInstance2.getPath(), 0,
         PEGASUS_CLASSNAME_INDHANDLER_CIMXML));
    subscriptionInstance.addProperty(CIMProperty(
        "OnFatalErrorPolicy", Uint16(2)));
    subscriptionInstance.addProperty(CIMProperty(
        "FailureTriggerTimeInterval", Uint64(120)));
    subscriptionInstance.addProperty(CIMProperty(
        "SubscriptionState", Uint16(2)));
    subscriptionInstance.addProperty(CIMProperty(
        "TimeOfLastStateChange", CIMDateTime::getCurrentDateTime()));
    subscriptionInstance.addProperty(CIMProperty(
        "SubscriptionDuration", Uint64(172800)));
    subscriptionInstance.addProperty(CIMProperty(
        "SubscriptionStartTime", CIMDateTime::getCurrentDateTime()));
    subscriptionInstance.addProperty(CIMProperty(
        "SubscriptionTimeRemaining", Uint64(172800)));
    subscriptionInstance.addProperty(CIMProperty(
        "RepeatNotificationPolicy", Uint16(1)));
    subscriptionInstance.addProperty(CIMProperty(
        "OtherRepeatNotificationPolicy", String("AnotherPolicy2")));
    subscriptionInstance.addProperty(CIMProperty(
        "RepeatNotificationInterval", Uint64(120)));
    subscriptionInstance.addProperty(CIMProperty(
        "RepeatNotificationGap", Uint64(30)));
    subscriptionInstance.addProperty(CIMProperty(
        "RepeatNotificationCount", Uint16(6)));

    // create keys
    Array<CIMKeyBinding> keys;

    keys.append(CIMKeyBinding("Filter", filterInstance2.getPath().toString(),
                              CIMKeyBinding::REFERENCE));
    keys.append(CIMKeyBinding("Handler", handlerInstance2.getPath().toString(),
                              CIMKeyBinding::REFERENCE));

    // update object path
    CIMObjectPath objectPath = subscriptionInstance.getPath();

    objectPath.setKeyBindings(keys);

    subscriptionInstance.setPath(objectPath);

    return(subscriptionInstance);
}

//
//  IdentityContainer
//
void Test1(void)
{
    if(verbose)
    {
        cout << "Test1()" << endl;
    }

    OperationContext context;

    {
        String userName("Yoda");

        context.insert(IdentityContainer(userName));

        IdentityContainer container = context.get(IdentityContainer::NAME);

        if(userName != container.getUserName())
        {
            cout << "----- Identity Container failed" << endl;

            throw 0;
        }
    }

    context.clear();

    {
        String userName("Yoda");

        context.insert(IdentityContainer(userName));

        //
        //  This test exercises the IdentityContainer copy constructor
        //
        IdentityContainer container = context.get(IdentityContainer::NAME);

        if(userName != container.getUserName())
        {
            cout << "----- Identity Container copy constructor failed" << endl;

            throw 0;
        }
    }

    context.clear();

    {
        String userName("Yoda");

        context.insert(IdentityContainer(userName));

        //
        //  This test exercises the IdentityContainer assignment operator
        //
        IdentityContainer container = IdentityContainer(" ");

        container = context.get(IdentityContainer::NAME);

        if(userName != container.getUserName())
        {
            cout << "----- Identity Container assignment operator failed"
                << endl;

            throw 0;
        }
    }
}

//
//  SubscriptionInstanceContainer
//
void Test2(void)
{
    if(verbose)
    {
        cout << "Test2()" << endl;
    }

    OperationContext context;

    CIMInstance subscriptionInstance = _createSubscriptionInstance1();

    {
        context.insert(SubscriptionInstanceContainer(subscriptionInstance));

        SubscriptionInstanceContainer container =
            context.get(SubscriptionInstanceContainer::NAME);

        if(!subscriptionInstance.identical(container.getInstance()))
        {
            cout << "----- Subscription Instance Container failed"
                 << endl;

            throw 0;
        }
    }

    context.clear();

    {
        context.insert(SubscriptionInstanceContainer(subscriptionInstance));

        //
        //  This test exercises the SubscriptionInstanceContainer copy
        //  constructor
        //
        SubscriptionInstanceContainer container =
            context.get(SubscriptionInstanceContainer::NAME);

        if(!subscriptionInstance.identical(container.getInstance()))
        {
            cout << "----- Subscription Instance Container copy"
                    " constructor failed" << endl;

            throw 0;
        }
    }

    context.clear();

    {
        context.insert(SubscriptionInstanceContainer(subscriptionInstance));

        //
        //  This test exercises the SubscriptionInstanceContainer assignment
        //  operator
        //
        SubscriptionInstanceContainer container =
            SubscriptionInstanceContainer(CIMInstance());

        container = context.get(SubscriptionInstanceContainer::NAME);

        if(!subscriptionInstance.identical(container.getInstance()))
        {
            cout << "----- Subscription Instance Container assignment"
                    " operator failed" << endl;

            throw 0;
        }
    }
}

//
//  SubscriptionFilterConditionContainer
//
void Test3(void)
{
    if(verbose)
    {
        cout << "Test3()" << endl;
    }

    OperationContext context;

    {
        String filterCondition("AlertType = 5");
        String queryLanguage("WQL1");

        context.insert(SubscriptionFilterConditionContainer(filterCondition,
                                                            queryLanguage));

        SubscriptionFilterConditionContainer container =
            context.get(SubscriptionFilterConditionContainer::NAME);

        if((filterCondition != container.getFilterCondition()) ||
           (queryLanguage != container.getQueryLanguage()))
        {
            cout << "----- Subscription Filter Condition Container failed"
                 << endl;

            throw 0;
        }
    }

    context.clear();

    {
        String filterCondition("AlertType = 5");
        String queryLanguage("WQL1");

        context.insert(
            SubscriptionFilterConditionContainer(filterCondition,
                                                 queryLanguage));

        //
        //  This test exercises the SubscriptionFilterConditionContainer copy
        //  constructor
        //
        SubscriptionFilterConditionContainer container =
            context.get(SubscriptionFilterConditionContainer::NAME);

        if((filterCondition != container.getFilterCondition()) ||
           (queryLanguage != container.getQueryLanguage()))
        {
            cout << "----- SubscriptionFilterCondition Container copy"
                    " constructor failed" << endl;

            throw 0;
        }
    }

    context.clear();

    {
        String filterCondition("AlertType = 5");
        String queryLanguage("WQL1");

        context.insert(
            SubscriptionFilterConditionContainer(filterCondition,
                                                 queryLanguage));

        //
        //  This test exercises the SubscriptionFilterConditionContainer
        //  assignment operator
        //
        SubscriptionFilterConditionContainer container =
            SubscriptionFilterConditionContainer(" ", " ");

        container = context.get(SubscriptionFilterConditionContainer::NAME);

        if((filterCondition != container.getFilterCondition()) ||
           (queryLanguage != container.getQueryLanguage()))
        {
            cout << "----- SubscriptionFilterCondition Container assignment"
                    " operator failed" << endl;

            throw 0;
        }
    }
}

//
//  SubscriptionFilterQueryContainer
//
void Test4(void)
{
    if(verbose)
    {
        cout << "Test4()" << endl;
    }

    OperationContext context;

    {
        String filterQuery(
            "SELECT * FROM CIM_AlertIndication WHERE AlertType = 5");
        String queryLanguage("WQL1");
        CIMNamespaceName sourceNamespace("root/sampleprovider");

        context.insert(
            SubscriptionFilterQueryContainer(filterQuery,
                                             queryLanguage,
                                             sourceNamespace));

        SubscriptionFilterQueryContainer container =
            context.get(SubscriptionFilterQueryContainer::NAME);

        if((filterQuery != container.getFilterQuery()) ||
           (queryLanguage != container.getQueryLanguage()) ||
           (!(sourceNamespace == container.getSourceNameSpace())))
        {
            cout << "----- Subscription Filter Query Container failed" << endl;

            throw 0;
        }
    }

    context.clear();

    {
        String filterQuery(
            "SELECT * FROM CIM_AlertIndication WHERE AlertType = 5");
        String queryLanguage("WQL1");
        CIMNamespaceName sourceNamespace("root/sampleprovider");

        context.insert(
            SubscriptionFilterQueryContainer(filterQuery,
                                             queryLanguage,
                                             sourceNamespace));

        //
        //  This test exercises the SubscriptionFilterQueryContainer copy
        //  constructor
        //
        SubscriptionFilterQueryContainer container =
            (SubscriptionFilterQueryContainer)context.get
            (SubscriptionFilterQueryContainer::NAME);

        if((filterQuery != container.getFilterQuery()) ||
           (queryLanguage != container.getQueryLanguage()) ||
           (!(sourceNamespace == container.getSourceNameSpace())))
        {
            cout << "----- SubscriptionFilterQuery Container copy"
                    " constructor failed" << endl;

            throw 0;
        }
    }

    context.clear();

    {
        String filterQuery(
            "SELECT * FROM CIM_AlertIndication WHERE AlertType = 5");
        String queryLanguage("WQL1");
        CIMNamespaceName sourceNamespace("root/sampleprovider");
        CIMNamespaceName junkNamespace("root/junk");

        context.insert(
            SubscriptionFilterQueryContainer(filterQuery,
                                             queryLanguage,
                                             sourceNamespace));

        //
        //  This test exercises the SubscriptionFilterQueryContainer
        //  assignment operator
        //
        SubscriptionFilterQueryContainer container =
            SubscriptionFilterQueryContainer(" ", " ", junkNamespace);

        container = context.get(SubscriptionFilterQueryContainer::NAME);

        if((filterQuery != container.getFilterQuery()) ||
           (queryLanguage != container.getQueryLanguage()) ||
           (!(sourceNamespace == container.getSourceNameSpace())))
        {
            cout << "----- SubscriptionFilterQuery Container assignment"
                    " operator failed" << endl;

            throw 0;
        }
    }
}

//
//  SubscriptionInstanceNamesContainer
//
void Test5(void)
{
    if(verbose)
    {
        cout << "Test5()" << endl;
    }

    OperationContext context;

    CIMInstance subscriptionInstance1 = _createSubscriptionInstance1();
    CIMInstance subscriptionInstance2 = _createSubscriptionInstance2();

    Array<CIMObjectPath> subscriptionInstanceNames;

    subscriptionInstanceNames.append(subscriptionInstance1.getPath());
    subscriptionInstanceNames.append(subscriptionInstance2.getPath());

    {
        context.insert(
            SubscriptionInstanceNamesContainer(subscriptionInstanceNames));

        SubscriptionInstanceNamesContainer container =
            context.get(SubscriptionInstanceNamesContainer::NAME);

        Array<CIMObjectPath> returnedInstanceNames =
            container.getInstanceNames();

        for(Uint8 i = 0,n = subscriptionInstanceNames.size(); i < n; i++)
        {
            if(!subscriptionInstanceNames[i].identical(
                returnedInstanceNames[i]))
            {
                cout << "----- Subscription Instance Names Container failed"
                     << endl;

                throw 0;
            }
        }
    }

    context.clear();

    {
        context.insert(
            SubscriptionInstanceNamesContainer(subscriptionInstanceNames));

        //
        //  This test exercises the SubscriptionInstanceNamesContainer copy
        //  constructor
        //
        SubscriptionInstanceNamesContainer container =
            context.get(SubscriptionInstanceNamesContainer::NAME);

        Array<CIMObjectPath> returnedInstanceNames =
            container.getInstanceNames();

        for(Uint8 i = 0, n = subscriptionInstanceNames.size(); i < n; i++)
        {
            if(!subscriptionInstanceNames[i].identical(
                                                returnedInstanceNames[i]))
            {
                cout << "----- Subscription Instance Names Container copy"
                        " constructor failed" << endl;

                throw 0;
            }
        }
    }

    context.clear();

    {
        context.insert(
            SubscriptionInstanceNamesContainer(subscriptionInstanceNames));

        //
        //  This test exercises the SubscriptionInstanceNamesContainer
        //  assignment operator
        //
        Array<CIMObjectPath> returnedInstanceNames;

        SubscriptionInstanceNamesContainer container =
            SubscriptionInstanceNamesContainer(returnedInstanceNames);

        container = context.get(SubscriptionInstanceNamesContainer::NAME);

        returnedInstanceNames = container.getInstanceNames();

        for(Uint8 i = 0, n = subscriptionInstanceNames.size(); i < n; i++)
        {
            if(!subscriptionInstanceNames[i].identical(
                                                returnedInstanceNames[i]))
            {
                cout << "----- Subscription Instance Names Container assignment"
                        " operator failed" << endl;

                throw 0;
            }
        }
    }
}

void Test6(void)
{
    if(verbose)
    {
        cout << "Test6()" << endl;
    }

    OperationContext context;

    String languageId("en-US");

    context.insert(LocaleContainer(languageId));

    LocaleContainer container = context.get(LocaleContainer::NAME);

    if(languageId != container.getLanguageId())
    {
        cout << "----- Locale Container failed" << endl;

        throw 0;
    }
}

void Test7(void)
{
    if(verbose)
    {
        cout << "Test7()" << endl;
    }

    OperationContext context;

    CIMInstance module("PG_ProviderModule");
    CIMInstance provider("PG_Provider");
    Boolean isRemoteNameSpace = true;
    String remoteInfo("remote_info");

    context.insert(ProviderIdContainer(module,
                                       provider,
                                       isRemoteNameSpace,
                                       remoteInfo));

    ProviderIdContainer container = context.get(ProviderIdContainer::NAME);

    if(!module.identical(container.getModule()) ||
       !provider.identical(container.getProvider()) ||
       (isRemoteNameSpace != container.isRemoteNameSpace()) ||
       (remoteInfo != container.getRemoteInfo()))
    {
        cout << "----- Provider Id Container failed" << endl;

        throw 0;
    }
}

void Test8(void)
{
    if(verbose)
    {
        cout << "Test8()" << endl;
    }

    OperationContext context;

    try
    {
        OperationContext scopeContext;

        scopeContext = context;

        scopeContext.remove(IdentityContainer::NAME);
        scopeContext.remove(SubscriptionInstanceContainer::NAME);
        scopeContext.remove(SubscriptionFilterConditionContainer::NAME);
        scopeContext.remove(SubscriptionFilterQueryContainer::NAME);
        scopeContext.remove(LocaleContainer::NAME);
        scopeContext.remove(ProviderIdContainer::NAME);
    }
    catch(...)
    {
    }
}

//
//  SnmpTrapOidContainer
//
void Test9(void)
{
    if(verbose)
    {
        cout << "Test9()" << endl;
    }

    OperationContext context;

    {
        String snmpTrapOid ("1.3.6.1.4.1.992.2.3.9210.8400");

        context.insert(SnmpTrapOidContainer(snmpTrapOid));

        SnmpTrapOidContainer container =
            context.get(SnmpTrapOidContainer::NAME);

        if(snmpTrapOid != container.getSnmpTrapOid())
        {
            cout << "----- Snmp Trap Oid Container failed" << endl;

            throw 0;
        }
    }

    context.clear();

    {
        String snmpTrapOid ("1.3.6.1.4.1.992.2.3.9210.8400");

        context.insert(SnmpTrapOidContainer(snmpTrapOid));

        //
        //  This test exercises the SnmpTrapOidContainer copy
        //  constructor
        //
        SnmpTrapOidContainer container =
            context.get(SnmpTrapOidContainer::NAME);

        if(snmpTrapOid != container.getSnmpTrapOid())
        {
            cout << "----- SnmpTrapOid Container copy constructor failed"
                 << endl;

            throw 0;
        }
    }

    context.clear();

    {
        String snmpTrapOid ("1.3.6.1.4.1.992.2.3.9210.8400");

        context.insert(SnmpTrapOidContainer(snmpTrapOid));

        //
        //  This test exercises the SnmpTrapOidContainer
        //  assignment operator
        //
        SnmpTrapOidContainer container = SnmpTrapOidContainer(" ");

        container = context.get(SnmpTrapOidContainer::NAME);

        if(snmpTrapOid != container.getSnmpTrapOid())
        {
            cout << "----- SnmpTrapOid Container assignment operator failed"
                 << endl;

            throw 0;
        }
    }
}

void Test10(void)
{
    if(verbose)
    {
        cout << "Test10()" << endl;
    }

    OperationContext context;

    CIMClass cimClass("CachedClass");

    context.insert(CachedClassDefinitionContainer(cimClass));

    CachedClassDefinitionContainer container =
        context.get(CachedClassDefinitionContainer::NAME);

    if(cimClass.getClassName().getString() !=
                        container.getClass().getClassName().getString())
    {
        cout << "----- CachedClassDefinitionContainer failed" << endl;

        throw 0;
    }
}

void Test11(void)
{
    if(verbose)
    {
        cout << "Test11()" << endl;
    }

    OperationContext context;

    String userRole("GodLikeEntity");

    context.insert(UserRoleContainer(userRole));

    UserRoleContainer container = context.get(UserRoleContainer::NAME);

    if(userRole != container.getUserRole())
    {
        cout << "----- Locale Container failed" << endl;

        throw 0;
    }
}

int main(int, char** argv)
{
    verbose = getenv("PEGASUS_TEST_VERBOSE") ? true : false;

    try
    {
        Test1();
        Test2();
        Test3();
        Test4();
        Test5();
        Test6();
        Test7();
        Test8();
        Test9();
        Test10();
        Test11();

        cout << argv[0] << " +++++ passed all tests" << endl;
    }
    catch(CIMException & e)
    {
        cout << argv[0] << " ----- failed with CIMException("
             << e.getCode() << "):" << e.getMessage() << endl;
    }
    catch(Exception & e)
    {
        cout << argv[0] << " ----- failed with Exception:" << e.getMessage()
             << endl;
    }
    catch(...)
    {
        cout << argv[0] << " ----- failed with unknown exception" <<  endl;
    }

    return(0);
}
