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
#include <Pegasus/Common/System.h>
#include <Pegasus/HandlerService/HandlerTable.h>
#include <Pegasus/Repository/CIMRepository.h>
#include <Pegasus/Config/ConfigManager.h>

PEGASUS_USING_PEGASUS;
PEGASUS_USING_STD;

const String NAMESPACE("TestSnmpHandler");

const CIMNamespaceName NS = CIMNamespaceName ("TestSnmpHandler");

const CIMName testClass1 = CIMName ("SnmpTestClass1");
const CIMName testClass2 = CIMName ("SnmpTestClass2");
const CIMName testClass3 = CIMName ("SnmpTestClass3");
const CIMName testClass4 = CIMName ("SnmpTestClass4");
const CIMName testClass5 = CIMName ("SnmpTestClass5");
const CIMName testClass6 = CIMName ("SnmpTestClass6");
const CIMName testClass7 = CIMName ("SnmpTestClass7");
const CIMName testClass8 = CIMName ("SnmpTestClass8");

static CIMInstance CreateHandlerInstance()
{
    CIMInstance handlerInstance(PEGASUS_CLASSNAME_INDHANDLER_SNMP);
    handlerInstance.addProperty (CIMProperty (CIMName
        ("SystemCreationClassName"), System::getSystemCreationClassName ()));
    handlerInstance.addProperty (CIMProperty (CIMName ("SystemName"),
        System::getFullyQualifiedHostName ()));
    handlerInstance.addProperty (CIMProperty (CIMName ("CreationClassName"),
        PEGASUS_CLASSNAME_INDHANDLER_SNMP.getString ()));
    handlerInstance.addProperty(CIMProperty(CIMName ("Name"),
        String("Handler1")));
    return (handlerInstance);
}

static void TestException(
    CIMHandler* handler,
    CIMInstance indicationHandlerInstance,
    CIMInstance indicationInstance,
    CIMStatusCode statusCode)
{
    OperationContext context;
    CIMInstance indicationSubscriptionInstance;
    ContentLanguageList contentLanguages;

    Boolean exceptionCaught = false;
    CIMException testException;

    try
    {
        handler->handleIndication(context, NAMESPACE, indicationInstance,
            indicationHandlerInstance, indicationSubscriptionInstance,
            contentLanguages);
    }
    catch (CIMException& e)
    {
        exceptionCaught = true;
        testException = e;
    }

    PEGASUS_TEST_ASSERT(exceptionCaught &&
        testException.getCode() == statusCode);
}

// Snmp traps are sent, but, only partial data are in the trap since
// there are errors in some data
static void TestError(
    CIMHandler* handler,
    CIMInstance indicationHandlerInstance,
    CIMInstance indicationInstance)
{
    OperationContext context;
    CIMInstance indicationSubscriptionInstance;
    ContentLanguageList contentLanguages;

    Boolean exceptionCaught = false;

    try
    {
        handler->handleIndication(context, NAMESPACE, indicationInstance,
            indicationHandlerInstance, indicationSubscriptionInstance,
            contentLanguages);
    }
    catch (CIMException&)
    {
        exceptionCaught = true;
    }

    PEGASUS_TEST_ASSERT(!exceptionCaught);
}

static void CreateRepository(CIMRepository & repository)
{
    repository.createNameSpace(NS);

    Array<String> qualifierValue;
    qualifierValue.append("");

    CIMQualifierDecl q1(CIMName ("MappingStrings"), qualifierValue,
        CIMScope::PROPERTY + CIMScope::CLASS);

    // Qualifier name must be "MappingStrings", test the qualifier
    // name is not "MappingStrings"
    CIMQualifierDecl q2(CIMName ("NotMappingStrings"), qualifierValue,
        CIMScope::CLASS);

    repository.setQualifier(NS, q1);
    repository.setQualifier(NS, q2);

    Array<String> classMappingStr;
    classMappingStr.append("OID.IETF | SNMP.1.3.6.1.4.1.892.2.3.9000.8600");

    CIMClass class1(testClass1);
    class1.addQualifier(CIMQualifier(CIMName ("MappingStrings"),
        CIMValue(classMappingStr)));

    Array<String> invalidFormatStr;
    invalidFormatStr.append(
        "Wrong format OID.IETF | SNMP.1.3.6.1.4.1.2.3.9000.8600");
    invalidFormatStr.append("DataType.IETF | OctetString ");

    // create wrong format property mappingStrings value
    class1.addProperty(
        CIMProperty(CIMName("OidDataType"), String("OctetString"))
        .addQualifier(CIMQualifier(CIMName ("MappingStrings"),
            CIMValue(invalidFormatStr))));

    repository.createClass(NS, class1);

    // create invalid mapping string value
    Array<String> class2MappingStr;
    Array<String> mappingStr2;

    class2MappingStr.append("OID.IETF |Invalid Mapping String Value");

    mappingStr2.append("OID.IETF | SNMP.1.3.6.1.4.1.2.3.9000.8600");
    mappingStr2.append("DataType.IETF OctetString ");

    CIMClass class2(testClass2);
    class2.addQualifier(CIMQualifier(CIMName ("MappingStrings"),
        CIMValue(class2MappingStr)));

    class2.addProperty(CIMProperty(CIMName ("OidDataType"), String())
        .addQualifier(CIMQualifier(CIMName ("MappingStrings"),
            CIMValue(mappingStr2))));
    repository.createClass(NS, class2);

    // create non MappingStrings qualifier
    CIMClass class3(testClass3);
    class3.addQualifier(CIMQualifier(CIMName ("NotMappingStrings"),
        CIMValue(classMappingStr)));

    repository.createClass(NS, class3);

    // error building ASN.1 representation
    Array<String> class4MappingStr;
    class4MappingStr.append("OID.IETF | SNMP.1.204.6.1.6.3.1.330.5.1.0 ");

    CIMClass class4(testClass4);
    class4.addQualifier(CIMQualifier(CIMName ("MappingStrings"),
        CIMValue(class4MappingStr)));

    repository.createClass(NS, class4);

    // create incorrect class mappingStrings value
    Array<String> class5MappingStr;
    class5MappingStr.append("OID.IETF | SNMP.1.3.6.1.6.test.1.1.5.1.3 ");

    CIMClass class5(testClass5);
    class5.addQualifier(CIMQualifier(CIMName
        ("MappingStrings"), CIMValue(class5MappingStr)));

    // create incorrect property name
    class5.addProperty(
        CIMProperty(CIMName ("WrongPropertyName"), String("OctetString"))
            .addQualifier(CIMQualifier(CIMName ("MappingStrings"),
                CIMValue(class5MappingStr))));

    repository.createClass(NS, class5);

    // create incorrect property mappingStrings value
    Array<String> class6MappingStr;
    class6MappingStr.append("OID.IETF | SNMP.1.3.6.1.6.3.1.1.0.1 ");

    Array<String> mappingStr6;
    mappingStr6.append("OID.IETF | SNMP.1.3.6.1.6.test.1.1.5.1.3");
    mappingStr6.append("DataType.IETF | OctetString");

    CIMClass class6(testClass6);
    class6.addQualifier(CIMQualifier(CIMName ("MappingStrings"),
        CIMValue(class6MappingStr)));
    class6.addProperty(
        CIMProperty(CIMName ("OidDataType"), String("OctetString"))
            .addQualifier(CIMQualifier(CIMName ("MappingStrings"),
                CIMValue(mappingStr6))));

    repository.createClass(NS, class6);

    // create unsupportted SNMP Data Type for the CIM property
    Array<String> class7MappingStr;
    class7MappingStr.append("OID.IETF | SNMP.1.3.6.1.6.3.1.1.5.1 ");

    Array<String> mappingStr7;
    mappingStr7.append("OID.IETF | SNMP.1.3.6.1.6.test.1.1.5.1.3");
    mappingStr7.append("DataType.IETF | test ");

    CIMClass class7(testClass7);
    class7.addQualifier(CIMQualifier(CIMName ("MappingStrings"),
        CIMValue(class7MappingStr)));
    class7.addProperty(
        CIMProperty(CIMName ("OidDataType"), String("test"))
            .addQualifier(CIMQualifier(CIMName ("MappingStrings"),
                CIMValue(mappingStr7))));

    repository.createClass(NS, class7);

    // create invalid syntax for MappingStrings qualifier
    Array<String> invalidSyntax;
    Array<String> class8MappingStr;
    class8MappingStr.append("OID.IETF Invalid Syntax for MappingStrings");

    Array<String> mappingStr8;
    mappingStr8.append("OID.IETF | SNMP.1.3.6.1.4.1.2.3.9000.8600");
    mappingStr8.append("DataType.IETF | OctetString ");

    CIMClass class8(testClass8);
    class8.addQualifier(CIMQualifier(CIMName ("MappingStrings"),
        CIMValue(class8MappingStr)));

    class8.addProperty(CIMProperty(CIMName ("OidDataType"), String())
        .addQualifier(CIMQualifier(CIMName ("MappingStrings"),
            CIMValue(mappingStr8))));
    repository.createClass(NS, class8);
}

static void TestExceptionHandling(CIMHandler* handler)
{
    CIMInstance indicationHandlerInstance;

    // Test "invalid IndicationHandlerSNMPMapper instance" exception
    CIMInstance indicationInstance(testClass1);
    indicationHandlerInstance = CreateHandlerInstance();
    TestException(handler, indicationHandlerInstance, indicationInstance,
        CIM_ERR_FAILED);

    // Test "no required property TargetHostFormat" exception
    indicationInstance = CIMInstance(testClass1);
    indicationInstance.addProperty(CIMProperty(
       CIMName ("OidDataType"), String("OctetString")));
    indicationHandlerInstance = CreateHandlerInstance();
    indicationHandlerInstance.addProperty(CIMProperty(
       CIMName("TargetHost"), String("15.13.140.120")));
    indicationHandlerInstance.addProperty(CIMProperty(
        CIMName("SNMPVersion"), Uint16(2)));
    TestException(handler, indicationHandlerInstance, indicationInstance,
        CIM_ERR_FAILED);

    // Test "no required property SNMPVersion" exception
    indicationInstance = CIMInstance(testClass1);
    indicationHandlerInstance = CreateHandlerInstance();
    indicationHandlerInstance.addProperty(CIMProperty(
       CIMName("TargetHost"), String("15.13.140.120")));
    indicationHandlerInstance.addProperty(CIMProperty(
        CIMName("TargetHostFormat"), Uint16(3)));
    TestException(handler, indicationHandlerInstance, indicationInstance,
        CIM_ERR_FAILED);

    // Test "unsupported snmp Version" exception
    indicationInstance = CIMInstance(testClass1);
    indicationHandlerInstance = CreateHandlerInstance();
    indicationHandlerInstance.addProperty(CIMProperty(
       CIMName("TargetHost"), String("15.13.140.120")));
    indicationHandlerInstance.addProperty(CIMProperty(
        CIMName("TargetHostFormat"), Uint16(3)));
    indicationHandlerInstance.addProperty(CIMProperty(
        CIMName("SNMPVersion"), Uint16(5)));
    TestException(handler, indicationHandlerInstance, indicationInstance,
        CIM_ERR_FAILED);

    // Test "invalid MappingStrings Syntax" exception
    indicationInstance = CIMInstance(testClass8);
    indicationInstance.addProperty(CIMProperty(
        CIMName ("OidDataType"), String("OctetString")));
    indicationHandlerInstance = CreateHandlerInstance();
    indicationHandlerInstance.addProperty(CIMProperty(
       CIMName("TargetHost"), String("15.13.140.120")));
    indicationHandlerInstance.addProperty(CIMProperty(
        CIMName("TargetHostFormat"), Uint16(3)));
    indicationHandlerInstance.addProperty(CIMProperty(
        CIMName("SNMPVersion"), Uint16(2)));
    TestException(handler, indicationHandlerInstance, indicationInstance,
        CIM_ERR_FAILED);

    // Test "invalid MappingStrings value" exception
    indicationInstance = CIMInstance(testClass2);
    indicationInstance.addProperty(CIMProperty(
        CIMName ("OidDataType"), String("OctetString")));
    indicationHandlerInstance = CreateHandlerInstance();
    indicationHandlerInstance.addProperty(CIMProperty(
       CIMName("TargetHost"), String("15.13.140.120")));
    indicationHandlerInstance.addProperty(CIMProperty(
        CIMName("TargetHostFormat"), Uint16(3)));
    indicationHandlerInstance.addProperty(CIMProperty(
        CIMName("SNMPVersion"), Uint16(2)));
    TestException(handler, indicationHandlerInstance, indicationInstance,
        CIM_ERR_FAILED);

    // Test "no MappingStrings qualifier" exception
    indicationInstance = CIMInstance(testClass3);
    indicationHandlerInstance = CreateHandlerInstance();
    indicationHandlerInstance.addProperty(CIMProperty(
       CIMName("TargetHost"), String("15.13.140.120")));
    indicationHandlerInstance.addProperty(CIMProperty(
        CIMName("TargetHostFormat"), Uint16(3)));
    indicationHandlerInstance.addProperty(CIMProperty(
        CIMName("SNMPVersion"), Uint16(2)));
    TestException(handler, indicationHandlerInstance, indicationInstance,
        CIM_ERR_FAILED);

    // Test "send trap failed" exception
    indicationInstance = CIMInstance(testClass4);
    indicationHandlerInstance = CreateHandlerInstance();
    indicationHandlerInstance.addProperty(CIMProperty(
       CIMName("TargetHost"), String("15.13.140.120")));
    indicationHandlerInstance.addProperty(CIMProperty(
        CIMName("TargetHostFormat"), Uint16(3)));
    indicationHandlerInstance.addProperty(CIMProperty(
        CIMName("SNMPVersion"), Uint16(2)));
    indicationHandlerInstance.addProperty(CIMProperty(
        CIMName("SNMPSecurityName"), String("t")));
    indicationHandlerInstance.addProperty(CIMProperty(
       CIMName("OtherTargetHostFormat"), String("testOtherTargetHostFormat")));
    TestException(handler, indicationHandlerInstance, indicationInstance,
        CIM_ERR_FAILED);

    // Test "failed to add snmp variables to PDU",
    // Both a DiscardedData message and an error message
    // are logged to log file
    indicationInstance = CIMInstance(testClass5);
    indicationInstance.addProperty(CIMProperty(
        CIMName ("OidDataType"), String("OctetString")));
    indicationHandlerInstance = CreateHandlerInstance();
    indicationHandlerInstance.addProperty(CIMProperty(
       CIMName("TargetHost"), String("15.13.140.120")));
    indicationHandlerInstance.addProperty(CIMProperty(
        CIMName("TargetHostFormat"), Uint16(3)));
    indicationHandlerInstance.addProperty(CIMProperty(
        CIMName("SNMPVersion"), Uint16(2)));
    indicationHandlerInstance.addProperty(CIMProperty(
        CIMName("SNMPSecurityName"), String("t")));
    indicationHandlerInstance.addProperty(CIMProperty(
       CIMName("OtherTargetHostFormat"), String("testOtherTargetHostFormat")));
    TestError(handler, indicationHandlerInstance, indicationInstance);

    // Test "convert enterprise OID from numeric form to a list of"
    // "subidentifiers failed".
    // Both a DiscardedData message and an error message
    // are logged to log file
    indicationInstance = CIMInstance(testClass5);
    indicationInstance.addProperty(CIMProperty(
        CIMName ("OidDataType"), String("OctetString")));
    indicationHandlerInstance = CreateHandlerInstance();
    indicationHandlerInstance.addProperty(CIMProperty(
       CIMName("TargetHost"), String("15.13.140.120")));
    indicationHandlerInstance.addProperty(CIMProperty(
        CIMName("TargetHostFormat"), Uint16(3)));
    indicationHandlerInstance.addProperty(CIMProperty(
        CIMName("SNMPVersion"), Uint16(3)));
    indicationHandlerInstance.addProperty(CIMProperty(
        CIMName("SNMPSecurityName"), String("t")));
    indicationHandlerInstance.addProperty(CIMProperty(
       CIMName("OtherTargetHostFormat"), String("testOtherTargetHostFormat")));
    TestError(handler, indicationHandlerInstance, indicationInstance);

    // Test "convert property OID from numeric form to a list of"
    // "subidentifiers failed".
    // Both a DiscardedData message and an error message
    // are logged to log file
    indicationInstance = CIMInstance(testClass6);
    indicationInstance.addProperty(CIMProperty(
        CIMName ("OidDataType"), String("OctetString")));
    indicationHandlerInstance = CreateHandlerInstance();
    indicationHandlerInstance.addProperty(CIMProperty(
       CIMName("TargetHost"), String("15.13.140.120")));
    indicationHandlerInstance.addProperty(CIMProperty(
        CIMName("TargetHostFormat"), Uint16(3)));
    indicationHandlerInstance.addProperty(CIMProperty(
        CIMName("SNMPVersion"), Uint16(2)));
    indicationHandlerInstance.addProperty(CIMProperty(
       CIMName("OtherTargetHostFormat"), String("testOtherTargetHostFormat")));
    TestError(handler, indicationHandlerInstance, indicationInstance);

    // Test "unsupported SNMP data type for the CIM property"
    // Both a DiscardedData message and an error message
    // are logged to log file
    indicationInstance = CIMInstance(testClass7);
    indicationInstance.addProperty(CIMProperty(
        CIMName ("OidDataType"), String("test")));
    indicationHandlerInstance = CreateHandlerInstance();
    indicationHandlerInstance.addProperty(CIMProperty(
       CIMName("TargetHost"), String("15.13.140.120")));
    indicationHandlerInstance.addProperty(CIMProperty(
        CIMName("TargetHostFormat"), Uint16(3)));
    indicationHandlerInstance.addProperty(CIMProperty(
        CIMName("SNMPVersion"), Uint16(2)));
    TestError(handler, indicationHandlerInstance, indicationInstance);
}


int main()
{

    const char* pegasusHome = getenv("PEGASUS_HOME");
    if (!pegasusHome)
    {
        cerr << "PEGASUS_HOME environment variable not set" << endl;
        exit(1);
    }

    String repositoryRoot = pegasusHome;
    repositoryRoot.append("/repository");

    CIMRepository* repository = new CIMRepository(
        repositoryRoot, CIMRepository::MODE_BIN);

    ConfigManager::setPegasusHome(pegasusHome);

    // -- Create repository and namespaces:

    CreateRepository(*repository);

    try
    {
        HandlerTable handlerTable;
        String handlerId = "snmpIndicationHandler";
        CIMHandler* handler = handlerTable.getHandler(handlerId, repository);
        PEGASUS_TEST_ASSERT(handler != 0);

        TestExceptionHandling(handler);

        //
        // -- Clean up classes:
        //
        repository->deleteClass(NS, testClass1);
        repository->deleteClass(NS, testClass2);
        repository->deleteClass(NS, testClass3);
        repository->deleteClass(NS, testClass4);
        repository->deleteClass(NS, testClass5);
        repository->deleteClass(NS, testClass6);
        repository->deleteClass(NS, testClass7);
        repository->deleteClass(NS, testClass8);

        //
        // -- Delete the qualifier:
        //
        repository->deleteQualifier(NS, CIMName ("MappingStrings"));
        repository->deleteQualifier(NS, CIMName ("NotMappingStrings"));

        repository->deleteNameSpace(NS);
    }
    catch(Exception& e)
    {
        PEGASUS_STD(cerr) << "Error: " << e.getMessage() << PEGASUS_STD(endl);
        exit(1);
    }

    delete repository;

    cout << "+++++ passed all tests" << endl;
    return 0;
}
