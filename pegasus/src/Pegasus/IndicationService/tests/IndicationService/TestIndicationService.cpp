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
#include <Pegasus/Common/PegasusAssert.h>
#include <Pegasus/Common/String.h>
#include <Pegasus/IndicationService/IndicationService.h>

PEGASUS_USING_PEGASUS;
PEGASUS_USING_STD;

const String ns="root/test";
const String className="MyClassName";

const String sourceKeyBinds=
    "CreationClassName=\"CIM_IndicationFilter\",Name=\"MyName\","
        "SystemCreationClassName=\"CIM_ComputerSystem\",SystemName=\"Test\"";

const String targetEmptyKeyBinds=
    "CreationClassName=\"CIM_IndicationFilter\",Name=\"MyName\","
        "SystemCreationClassName=\"CIM_ComputerSystem\",SystemName=\"\"";

const String targetSetKeyBinds=
    "CreationClassName=\"CIM_IndicationFilter\",Name=\"MyName\","
        "SystemCreationClassName=\"CIM_ComputerSystem\",SystemName=\"TestSys\"";

void test_setSystemNameInHandlerFilter()
{
    String fullSourcePath=ns+":"+className+"."+sourceKeyBinds;
    CIMObjectPath source;
    
    String fullEmptyTargetPath=
        ns+":"+className+"."+targetEmptyKeyBinds;
    CIMObjectPath emptyTarget(fullEmptyTargetPath);

    String fullSetTargetPath=
        ns+":"+className+"."+targetSetKeyBinds;
    CIMObjectPath setTarget(fullSetTargetPath);

    source.set(fullSourcePath);
    PEGASUS_TEST_ASSERT(source != emptyTarget);
    IndicationService::_setSystemNameInHandlerFilterPath(source,String::EMPTY);
    PEGASUS_TEST_ASSERT(source == emptyTarget);

    source.set(fullSourcePath);
    PEGASUS_TEST_ASSERT(source != setTarget);
    IndicationService::_setSystemNameInHandlerFilterPath(source,"TestSys");
    PEGASUS_TEST_ASSERT(source == setTarget);
}

void test_setOrAddSystemNameInHandlerFilter()
{
    CIMInstance target(CIMName("MyClass"));

    target.addProperty(
        CIMProperty(CIMName("Name"),String("Just its name.")));

    target.addProperty(
        CIMProperty(CIMName("ExampleProperty"),String("Example")));

    target.addProperty(
        CIMProperty(CIMName("Query"),String("SELECT me FROM database")));

    target.addProperty(
        CIMProperty(CIMName("QueryLanguage"),String("WQL")));

    target.addProperty(
        CIMProperty(
            CIMName("SystemCreationClassName"),
            String("PG_ComputerSystem")));

    target.addProperty(
        CIMProperty(CIMName("CreationClassName"),String("MyClass")));
    
    // clone before adding SystemName property to target
    CIMInstance emptyTarget = target.clone();
    CIMInstance setTarget = target.clone();

    emptyTarget.addProperty(
        CIMProperty(CIMName("SystemName"),String()));
    
    setTarget.addProperty(
        CIMProperty(CIMName("SystemName"),String("TestSys")));
    
    // test setting SystemName property to empty String
    CIMInstance noSystemName = target.clone();
    CIMInstance withSystemName = target.clone();
    
    withSystemName.addProperty(
        CIMProperty(CIMName("SystemName"),String("local")));

    PEGASUS_TEST_ASSERT(noSystemName != emptyTarget);
    PEGASUS_TEST_ASSERT(withSystemName != emptyTarget);

    IndicationService::_setOrAddSystemNameInHandlerFilter(
        noSystemName,
        String::EMPTY);
    IndicationService::_setOrAddSystemNameInHandlerFilter(
        withSystemName,
        String::EMPTY);

    PEGASUS_TEST_ASSERT(noSystemName == emptyTarget);
    PEGASUS_TEST_ASSERT(withSystemName == emptyTarget);

    // test setting SystemName property to "TestSys"
    noSystemName = target.clone();
    withSystemName = target.clone();
    
    withSystemName.addProperty(
        CIMProperty(CIMName("SystemName"),String("local")));

    PEGASUS_TEST_ASSERT(noSystemName != setTarget);
    PEGASUS_TEST_ASSERT(withSystemName != setTarget);

    IndicationService::_setOrAddSystemNameInHandlerFilter(
        noSystemName,
        "TestSys");
    IndicationService::_setOrAddSystemNameInHandlerFilter(
        withSystemName,
        "TestSys");

    PEGASUS_TEST_ASSERT(noSystemName == setTarget);
    PEGASUS_TEST_ASSERT(withSystemName == setTarget);
}

void test_setSystemNameInHandlerFilterReference()
{
    String fullSourcePath=ns+":"+className+"."+sourceKeyBinds;
    String fullEmptyTargetPath=
        ns+":"+className+"."+targetEmptyKeyBinds;
    String fullSetTargetPath=
        ns+":"+className+"."+targetSetKeyBinds;

    PEGASUS_TEST_ASSERT(fullSourcePath != fullEmptyTargetPath);
    PEGASUS_TEST_ASSERT(fullSourcePath != fullSetTargetPath);

    IndicationService::_setSystemNameInHandlerFilterReference(
        fullSourcePath,
        String::EMPTY);
    PEGASUS_TEST_ASSERT(fullSourcePath == fullEmptyTargetPath);
    
    IndicationService::_setSystemNameInHandlerFilterReference(
        fullSourcePath,
        "TestSys");
    PEGASUS_TEST_ASSERT(fullSourcePath == fullSetTargetPath);
}

CIMObjectPath _buildFilterOrHandlerPath
    (const CIMName & className,
     const String & name,
     const String & host,
     const CIMNamespaceName & namespaceName = CIMNamespaceName())
{
    CIMObjectPath path;

    Array <CIMKeyBinding> keyBindings;
    keyBindings.append (CIMKeyBinding ("SystemCreationClassName",
        System::getSystemCreationClassName(), CIMKeyBinding::STRING));
    keyBindings.append (CIMKeyBinding ("SystemName",
        System::getFullyQualifiedHostName(), CIMKeyBinding::STRING));
    keyBindings.append (CIMKeyBinding ("CreationClassName",
        className.getString(), CIMKeyBinding::STRING));
    keyBindings.append (CIMKeyBinding ("Name", name, CIMKeyBinding::STRING));
    path.setClassName (className);
    path.setKeyBindings (keyBindings);
    path.setNameSpace (namespaceName);
    path.setHost (host);

    return path;
}

CIMObjectPath _buildSubscriptionPath
    (const String & filterName,
     const CIMName & handlerClass,
     const String & handlerName,
     const String & filterHost,
     const String & handlerHost,
     const CIMNamespaceName & filterNS,
     const CIMNamespaceName & handlerNS)
{
    CIMObjectPath filterPath = _buildFilterOrHandlerPath
        (PEGASUS_CLASSNAME_INDFILTER, filterName, filterHost, filterNS);

    CIMObjectPath handlerPath = _buildFilterOrHandlerPath (handlerClass,
        handlerName, handlerHost, handlerNS);

    Array<CIMKeyBinding> subscriptionKeyBindings;
    subscriptionKeyBindings.append (CIMKeyBinding ("Filter",
        filterPath.toString(), CIMKeyBinding::REFERENCE));
    subscriptionKeyBindings.append (CIMKeyBinding ("Handler",
        handlerPath.toString(), CIMKeyBinding::REFERENCE));
    CIMObjectPath subscriptionPath ("", CIMNamespaceName(),
        PEGASUS_CLASSNAME_INDSUBSCRIPTION, subscriptionKeyBindings);

    return subscriptionPath;
}

// Build subscription with empty 
CIMObjectPath _buildSubscriptionPath
    (const String & filterName,
     const CIMName & handlerClass,
     const String & handlerName)
{
    return _buildSubscriptionPath(filterName, handlerClass, handlerName,
        String::EMPTY, String::EMPTY, CIMNamespaceName(), CIMNamespaceName());
}

CIMInstance _buildSubscriptionInstance
    (const CIMObjectPath & filterPath,
     const CIMName & handlerClass,
     const CIMObjectPath & handlerPath)
{
    CIMInstance subscriptionInstance (PEGASUS_CLASSNAME_INDSUBSCRIPTION);

    CIMObjectPath path = _buildSubscriptionPath ("Filter01",
        PEGASUS_CLASSNAME_INDHANDLER_CIMXML, "Handler01");
    subscriptionInstance.setPath (path);

    subscriptionInstance.addProperty (CIMProperty (CIMName ("Filter"),
        filterPath, 0, PEGASUS_CLASSNAME_INDFILTER));
    subscriptionInstance.addProperty (CIMProperty (CIMName ("Handler"),
        handlerPath, 0, handlerClass));

    return subscriptionInstance;
}


void test_isSubscription()
{
    CIMObjectPath filterPath = _buildFilterOrHandlerPath(
        PEGASUS_CLASSNAME_INDFILTER,
        "Filter1",
        System::getFullyQualifiedHostName(),
        PEGASUS_NAMESPACENAME_INTEROP);

    CIMObjectPath handlerPath = _buildFilterOrHandlerPath(
        PEGASUS_CLASSNAME_INDHANDLER_CIMXML,
        "handler1",
        System::getFullyQualifiedHostName(),
        PEGASUS_NAMESPACENAME_INTEROP);        
    
    PEGASUS_TEST_ASSERT(IndicationService::_isSubscription(filterPath) == false);
    PEGASUS_TEST_ASSERT(IndicationService::_isSubscription(handlerPath) == false);
    /* TODO, Not finished Superceeded by tests in Subscription
    CIMInstance subscription = _buildSubscriptionInstance(
        filterPath,
        PEGASUS_CLASSNAME_INDHANDLER_CIMXML,
        handlerPath);

    PEGASUS_TEST_ASSERT(IndicationService::_isSubscription(subscription) == true);
    */
}

void test_setSystemNameInSubscription()
{
    CIMObjectPath filterPath = _buildFilterOrHandlerPath(
        PEGASUS_CLASSNAME_INDFILTER,
        "Filter1",
        String::EMPTY,
        PEGASUS_NAMESPACENAME_INTEROP);

    CIMObjectPath handlerPath = _buildFilterOrHandlerPath(
        PEGASUS_CLASSNAME_INDHANDLER_CIMXML,
        "handler1",
        String::EMPTY,
        PEGASUS_NAMESPACENAME_INTEROP);
        
    CIMInstance subscription = _buildSubscriptionInstance(
        filterPath,
        PEGASUS_CLASSNAME_INDHANDLER_CIMXML,
        handlerPath);

    IndicationService::_setSystemNameInSubscription(
        subscription, System::getFullyQualifiedHostName(),
        true);

    // TODO test result
    
}

int main()
{
    test_setSystemNameInHandlerFilter();
    test_setOrAddSystemNameInHandlerFilter();
    test_setSystemNameInHandlerFilterReference();
    test_setSystemNameInSubscription();

    return 0;
}
