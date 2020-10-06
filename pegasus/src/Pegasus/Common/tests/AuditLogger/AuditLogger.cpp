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
#include <Pegasus/Common/Constants.h>
#include <iostream>
#include <Pegasus/Common/System.h>
#include <Pegasus/Common/FileSystem.h>
#include <Pegasus/Common/Logger.h>
#include <Pegasus/Common/AuditLogger.h>

#ifdef PEGASUS_OS_ZOS
#include <Pegasus/General/SetFileDescriptorToEBCDICEncoding.h>
#include <Pegasus/Common/Audit_zOS_SMF.h>
#endif

PEGASUS_USING_PEGASUS;
PEGASUS_USING_STD;

#ifdef PEGASUS_ENABLE_AUDIT_LOGGER

typedef void (*PEGASUS_AUDITLOGINITIALIZE_CALLBACK_T)();

const String TEST_USER("guest");
const String TEST_IP("127.0.0.1");

#ifdef PEGASUS_OS_ZOS
const String MASTER_FILE("/src/Pegasus/Common"
                 "/tests/AuditLogger/masterOutput_zOS");
#else
const String MASTER_FILE("/src/Pegasus/Common/tests/AuditLogger/masterOutput");
#endif

String auditTestLogFile;

CIMInstance _createModuleInstance(
    const String & name,
    const String & location,
    const Array<Uint16> status)
{
    CIMInstance moduleInstance (PEGASUS_CLASSNAME_PROVIDERMODULE);
    moduleInstance.addProperty (CIMProperty (CIMName ("Name"), name));
    moduleInstance.addProperty (CIMProperty (CIMName ("Vendor"),
        String ("Hewlett-Packard Company")));
    moduleInstance.addProperty (CIMProperty (CIMName ("Version"),
        String ("2.6")));
    moduleInstance.addProperty (CIMProperty (CIMName ("InterfaceType"),
        String ("C++Default")));
    moduleInstance.addProperty (CIMProperty (CIMName ("InterfaceVersion"),
        String ("2.6.0")));
    moduleInstance.addProperty (CIMProperty (CIMName ("OperationalStatus"),
        status));
    moduleInstance.addProperty (CIMProperty (CIMName ("Location"), location));

    return(moduleInstance);
}

void testLogCurrentConf()
{
    Array<String> propertyNames;
    Array<String> propertyValues;

    propertyNames.append("AuditLogProperty1");
    propertyNames.append("AuditLogProperty2");
    propertyNames.append("AuditLogProperty3");

    propertyValues.append("AuditLogValue1");
    propertyValues.append("AuditLogValue2");
    propertyValues.append("AuditLogValue3");

    AuditLogger::logCurrentConfig(propertyNames, propertyValues);
}

#ifdef PEGASUS_OS_ZOS

static void printSMFRecord (int subtype, char* record )
{
    char printLine[3][80];
    int p;
    int len;
    char item;
    int total = ((_smf86_header *)record)->SMF86LEN;

    FILE * _auditTestLogFileHandle =
        fopen(auditTestLogFile.getCString(), "a+");

    setEBCDICEncoding(fileno(_auditTestLogFileHandle));

    for (int i = 0; i <= total;i=i+1)
    {
        p = i%80;

        if (p == 0 && i > 0 ||
            i == total )
        {
            for (int y = 0; y < 3; y=y+1)
            {
                if (p == 0)
                {
                    len = 80;
                } else
                {
                    len = p;
                }

                for (int x = 0; x < len; x=x+1)
                {
                    if (y == 0)
                    {
                        fprintf(_auditTestLogFileHandle,"%c",printLine[y][x]);
                    }
                    else
                    {
                        fprintf( _auditTestLogFileHandle,"%1X",printLine[y][x]);
                    }
                }
                fprintf(_auditTestLogFileHandle,"\n");
            }
            fprintf(_auditTestLogFileHandle, "\n");
        }

        // delete CIM software level, MVS software level,
        // System and Sysplex name, Process ID, and Thread ID
        // to be able to comparable with master result
        if (i > 51 && i < 110 )
        {
          record[i] = 0;
        }

        item = record[i];
        __e2a_l(&item,1);

        if (item < 32 || item > 126 )
        {
            printLine[0][p] = '.';
        } else
        {
            printLine[0][p] = item;
        }

        printLine[1][p] = record[i]/16;
        printLine[2][p] = record[i]%16;

    }

    fclose(_auditTestLogFileHandle);
}

#else

static void writeAuditLogToFile(
    AuditLogger::AuditType auditType, AuditLogger::AuditSubType auditSubType,
    AuditLogger::AuditEvent auditEvent,
    Uint32 logLevel, MessageLoaderParms & msgParms)
{
    FILE * _auditTestLogFileHandle =
        fopen(auditTestLogFile.getCString(), "a+");
    PEGASUS_TEST_ASSERT(_auditTestLogFileHandle);

    fprintf(_auditTestLogFileHandle, "%s\n",
        (const char *)MessageLoader::getMessage(msgParms).getCString());

    fclose(_auditTestLogFileHandle);
}

#endif

void testLogCurrentRegProvider()
{
    Array<CIMInstance> instances;

    Array<Uint16> status0, status1, status2, status3, status4, status5;
    Array<Uint16> status6, status7, status8, status9, status10, status11;
    Array<Uint16> status12, status13, status14;

    status0.append(0);
    status1.append(1);
    status2.append(2);
    status3.append(3);
    status4.append(4);
    status5.append(5);
    status6.append(6);
    status7.append(7);
    status8.append(8);
    status9.append(9);
    status10.append(10);
    status11.append(11);
    status12.append(12);
    status13.append(13);

    instances.append(_createModuleInstance("AuditLogProviderModule0",
        "AuditLogProvider0", status0));
    instances.append(_createModuleInstance("AuditLogProviderModule1",
        "AuditLogProvider1", status1));
    instances.append(_createModuleInstance("AuditLogProviderModule2",
        "AuditLogProvider2", status2));
    instances.append(_createModuleInstance("AuditLogProviderModule3",
        "AuditLogProvider3", status3));
    instances.append(_createModuleInstance("AuditLogProviderModule4",
        "AuditLogProvider4", status4));
    instances.append(_createModuleInstance("AuditLogProviderModule5",
        "AuditLogProvider5", status5));
    instances.append(_createModuleInstance("AuditLogProviderModule6",
        "AuditLogProvider6", status6));
    instances.append(_createModuleInstance("AuditLogProviderModule7",
        "AuditLogProvider7", status7));
    instances.append(_createModuleInstance("AuditLogProviderModule8",
        "AuditLogProvider8", status8));
    instances.append(_createModuleInstance("AuditLogProviderModule9",
        "AuditLogProvider9", status9));
    instances.append(_createModuleInstance("AuditLogProviderModule10",
        "AuditLogProvider10", status10));
    instances.append(_createModuleInstance("AuditLogProviderModule11",
        "AuditLogProvider11", status11));
    instances.append(_createModuleInstance("AuditLogProviderModule12",
        "AuditLogProvider12", status12));
    instances.append(_createModuleInstance("AuditLogProviderModule13",
        "AuditLogProvider13", status13));

    AuditLogger::logCurrentRegProvider(instances);
}

void testLogSetConfigProperty()
{
    // log setting planned value of config property
    AuditLogger::logSetConfigProperty("guest", "logdir", "./logs",
        "/tmp", true);

    // log setting current value of config property
    AuditLogger::logSetConfigProperty("guest", "logdir",
        "./logs", "/tmp", false);
}

void testLogSchemaOperations()
{
    AuditLogger::logUpdateClassOperation(
        "CreateClass", AuditLogger::EVENT_CREATE,
        "user1", "127.0.0.1", "root/test", "class1", CIM_ERR_FAILED);
    AuditLogger::logUpdateClassOperation(
        "ModifyClass", AuditLogger::EVENT_UPDATE,
        "", "localhost", "test", "CIM_MyClass", CIM_ERR_ACCESS_DENIED);
    AuditLogger::logUpdateClassOperation(
        "DeleteClass", AuditLogger::EVENT_DELETE,
        "guest", "1.1.1.1", "a/b/c/d", "ByeByeClass", CIM_ERR_SUCCESS);
    AuditLogger::logUpdateQualifierOperation(
        "SetQualifier", AuditLogger::EVENT_UPDATE,
        "rootabc", "255.0.0.255", "test2", "Test", CIM_ERR_NOT_SUPPORTED);
    AuditLogger::logUpdateQualifierOperation(
        "DeleteQualifier", AuditLogger::EVENT_DELETE,
        "r", "4.3.2.1", "test1/test2", "Bogus", CIM_ERR_NOT_FOUND);
}

void testLogInstanceOperations()
{
    AuditLogger::logUpdateInstanceOperation(
        "CreateInstance", AuditLogger::EVENT_CREATE,
        "user1", "127.0.0.9", "root/test", CIMObjectPath("class1"),
        "", "", CIM_ERR_FAILED);
    AuditLogger::logUpdateInstanceOperation(
        "CreateInstance", AuditLogger::EVENT_CREATE,
        "user1", "127.0.0.9", "root/test", CIMObjectPath("class1.id=\"v1\""),
        "Module1", "Provider1", CIM_ERR_SUCCESS);

    AuditLogger::logUpdateInstanceOperation(
        "ModifyInstance", AuditLogger::EVENT_UPDATE,
        "", "localhost", "test", CIMObjectPath("CIM_MyClass.id=1"),
        "", "", CIM_ERR_INVALID_CLASS);
    AuditLogger::logUpdateInstanceOperation(
        "ModifyInstance", AuditLogger::EVENT_UPDATE,
        "", "localhost", "test", CIMObjectPath("CIM_MyClass.id=1"),
        "TheModule", "TheProvider", CIM_ERR_INVALID_CLASS);

    AuditLogger::logUpdateInstanceOperation(
        "DeleteInstance", AuditLogger::EVENT_DELETE,
        "guest", "127.0.0.1", "a/b/c", CIMObjectPath("THE_Class.a=\"A\",z=10"),
        "", "", CIM_ERR_NOT_FOUND);
    AuditLogger::logUpdateInstanceOperation(
        "DeleteInstance", AuditLogger::EVENT_DELETE,
        "guest", "127.0.0.1", "a/b/c", CIMObjectPath("THE_Class.a=\"A\",z=10"),
        "AHappyModule", "AHappyProvider", CIM_ERR_NOT_FOUND);

    AuditLogger::logUpdateInstanceOperation(
        "SetProperty", AuditLogger::EVENT_UPDATE,
        "me", "1.0.0.0", "a/b/c/d/e/f/g", CIMObjectPath("Z.x=0"),
        "", "", CIM_ERR_INVALID_CLASS);
    AuditLogger::logUpdateInstanceOperation(
        "SetProperty", AuditLogger::EVENT_UPDATE,
        "me", "1.0.0.0", "a/b/c/d/e/f/g", CIMObjectPath("Z.x=0"),
        "InstModule", "InstProv", CIM_ERR_INVALID_CLASS);

    AuditLogger::logInvokeMethodOperation(
        "guest", "1.1.1.1", "t", CIMObjectPath("Test"), "MyMethod",
        "", "", CIM_ERR_FAILED);
    AuditLogger::logInvokeMethodOperation(
        "guest", "1.1.1.1", "t", CIMObjectPath("Test.a=1,b=2,c=3"), "MyMethod",
        "m", "p", CIM_ERR_FAILED);
}

void testLogUpdateProvModuleStatus()
{
    Array<Uint16> currentModuleStatus, newModuleStatus;

    currentModuleStatus.append(10);
    newModuleStatus.append(2);

    // module status is updated successfully
    AuditLogger::logUpdateProvModuleStatus("AuditLogUpdateModuleStatus",
        currentModuleStatus, newModuleStatus);

    // Failed to update the module status
    newModuleStatus.clear();
    newModuleStatus.append(10);
    AuditLogger::logUpdateProvModuleStatus("AuditLogUpdateModuleStatus",
        currentModuleStatus, newModuleStatus);

    currentModuleStatus.clear();
    newModuleStatus.clear();
    currentModuleStatus.append(9);
    currentModuleStatus.append(10);
    newModuleStatus.append(8);
    newModuleStatus.append(2);
    AuditLogger::logUpdateProvModuleStatus("AuditLogUpdateModuleStatus",
        currentModuleStatus, newModuleStatus);
}

void testLogLocalAuthentication()
{
    // log success message
    AuditLogger::logLocalAuthentication(
        TEST_USER,
        true);

    // log failure message
    AuditLogger::logLocalAuthentication(
        TEST_USER,
        false);
}

void testLogBasicAuthentication()
{
    // log success message
    AuditLogger::logBasicAuthentication(
        TEST_USER,
        TEST_IP,
        true);

    // log failure message
    AuditLogger::logBasicAuthentication(
        TEST_USER,
        TEST_IP,
        false);
}

void testLogCertBasedAuthentication()
{
    const String TEST_ISSUER =
       "/C=US/ST=VIRGINIA/L=Fairfax/O=OpenGroup/OU=OpenPegasus/"
           "CN=TestSelfSigned1-Jun";
    const String TEST_SUBJECT =
       "/C=US/ST=VIRGINIA/L=Fairfax/O=OpenGroup/OU=OpenPegasus/"
           "CN=TestSelfSigned1-Jun";
    const String TEST_SERIAL_NUM = "9000a46a9c7e78118567699555108792";

    // log success message
    AuditLogger::logCertificateBasedAuthentication(
        TEST_ISSUER,
        TEST_SUBJECT,
        TEST_SERIAL_NUM,
        TEST_IP,
        true);

    // log failure message
    AuditLogger::logCertificateBasedAuthentication(
        TEST_ISSUER,
        TEST_SUBJECT,
        TEST_SERIAL_NUM,
        TEST_IP,
        false);
}

void testLogCertBasedUserValidation()
{
    const String TEST_ISSUER =
       "/C=US/ST=VIRGINIA/L=Fairfax/O=OpenGroup/OU=OpenPegasus"
           "/CN=TestSelfSigned1-Jun";
    const String TEST_SUBJECT =
       "/C=US/ST=VIRGINIA/L=Fairfax/O=OpenGroup/OU=OpenPegasus"
            "/CN=TestSelfSigned1-Jun";
    const String TEST_SERIAL_NUM = "9000a46a9c7e78118567699555108792";
    const String TEST_CERT_USER = "guest";

    // log success message
    AuditLogger::logCertificateBasedUserValidation(
        TEST_CERT_USER,
        TEST_ISSUER,
        TEST_SUBJECT,
        TEST_SERIAL_NUM,
        TEST_IP,
        true);

    // log failure message
    AuditLogger::logCertificateBasedUserValidation(
        TEST_CERT_USER,
        TEST_ISSUER,
        TEST_SUBJECT,
        TEST_SERIAL_NUM,
        TEST_IP,
        false);
}

void auditLogInitializeCallback()
{
    PEGASUS_TEST_ASSERT(!AuditLogger::isEnabled());
}

void testSetInitializeCallback()
{
    AuditLogger::setInitializeCallback(auditLogInitializeCallback);
}

void testSetEnabled(Boolean enabled)
{
    AuditLogger::setEnabled(enabled);
}

void testEnabled()
{
    PEGASUS_TEST_ASSERT(AuditLogger::isEnabled());
}

void testDisabled()
{
    PEGASUS_TEST_ASSERT(!(AuditLogger::isEnabled()));
}
#endif

int main(int, char** argv)
{
#ifdef PEGASUS_ENABLE_AUDIT_LOGGER

#ifdef PEGASUS_OS_ZOS
    AuditLogger::setAuditLogWriterCallback(printSMFRecord);
#else
    AuditLogger::setAuditLogWriterCallback(writeAuditLogToFile);
#endif

    const char* pegasusHomeDir = getenv("PEGASUS_HOME");

    if (pegasusHomeDir == NULL)
    {
        pegasusHomeDir = ".";
    }

    auditTestLogFile = pegasusHomeDir;
    auditTestLogFile.append("/AuditTest.log");

    System::removeFile(auditTestLogFile.getCString());

#ifdef PEGASUS_OS_ZOS
    // set file encoding to EBCDIC, because the z/OS master file is
    // in EBCDIC and we are running in ASCII.
    FILE * _auditTestLogFileHandle =
        fopen(auditTestLogFile.getCString(), "a+");
    setEBCDICEncoding(fileno(_auditTestLogFileHandle));
    fclose(_auditTestLogFileHandle);
#endif

    const char * masterDir = getenv("PEGASUS_ROOT");

    String masterFile (masterDir);
    masterFile.append(MASTER_FILE);

    try
    {
        testSetInitializeCallback();
        testSetEnabled(true);
        testEnabled();
        testLogCurrentConf();
        testLogCurrentRegProvider();
        testLogSetConfigProperty();
        testLogSchemaOperations();
        testLogInstanceOperations();
        testLogUpdateProvModuleStatus();
        testLogLocalAuthentication();
        testLogBasicAuthentication();
        testLogCertBasedAuthentication();
        testLogCertBasedUserValidation();
        testSetEnabled(false);
        testDisabled();

        PEGASUS_TEST_ASSERT(FileSystem::compareFiles(
            auditTestLogFile, masterFile));
    }
    catch (Exception& e)
    {
        cout << "Caught unexpected exception: " << e.getMessage() << endl;
        return 1;
    }
    catch (...)
    {
        cout << "Caught unexpected exception" << endl;
        return 1;
    }

    System::removeFile(auditTestLogFile.getCString());

    cout << argv[0] << " +++++ passed all tests" << endl;

#else

    cout << argv[0] << ": AuditLogger is not enabled; test skipped" << endl;

#endif

    return 0;
}
