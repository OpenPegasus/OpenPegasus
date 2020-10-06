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
#include <Pegasus/Common/Logger.h>
#include <Pegasus/Common/AuditLogger.h>

#ifdef PEGASUS_OS_DARWIN
# include <crt_externs.h>
#endif

#ifndef PEGASUS_OS_TYPE_WINDOWS
# include <unistd.h>
#endif

#ifdef PEGASUS_OS_VMS
# include <unixlib.h>
#endif

#ifdef PEGASUS_OS_SOLARIS
extern char** environ;
#endif

#include <stdlib.h>

#ifdef PEGASUS_ENABLE_AUDIT_LOGGER

#if defined(PEGASUS_OS_SOLARIS)
extern char** environ;
#endif

PEGASUS_USING_STD;

PEGASUS_NAMESPACE_BEGIN

static const String providerModuleStatus [] = {
    "Unknown", "Other", "OK", "Degraded",
    "Stressed", "Predictive Failure", "Error", "Non-Recoverable Error",
    "Starting", "Stopping", "Stopped", "In Service", "No Contact",
    "Lost Communication"};

Boolean AuditLogger::_auditLogFlag = false;

AuditLogger::PEGASUS_AUDITLOGINITIALIZE_CALLBACK_T
    AuditLogger::_auditLogInitializeCallback = 0;

AuditLogger::PEGASUS_AUDITLOG_CALLBACK_T AuditLogger::_writeAuditMessage =
    AuditLogger::_writeAuditMessageToLog;

void AuditLogger::logCurrentConfig(
    const Array<String> & propertyNames,
    const Array<String> & propertyValues)
{
    for (Uint32 i = 0; i < propertyNames.size(); i++)
    {
        String propertyStr = propertyNames[i] + "=" + propertyValues[i];

        MessageLoaderParms msgParms("Common.AuditLogger.CURRENT_CONFIG",
            "cimserver configuration $0", propertyStr);

        _writeAuditMessage(TYPE_CONFIGURATION,
            SUBTYPE_CURRENT_CONFIGURATION,
            EVENT_START_UP, Logger::INFORMATION, msgParms);
    }
}

void AuditLogger::logCurrentRegProvider(
    const Array < CIMInstance > & instances)
{
    String moduleName;
    Array<Uint16> moduleStatus;
    String statusValue;
    Uint32 pos;

    // get all the registered provider module names and status
    for (Uint32 i = 0; i <instances.size(); i++)
    {
        instances[i].getProperty(instances[i].findProperty(
            _PROPERTY_PROVIDERMODULE_NAME)).getValue().get(moduleName);

        pos = instances[i].findProperty(_PROPERTY_OPERATIONALSTATUS);

        if (pos == PEG_NOT_FOUND)
        {
            moduleStatus.append(0);
        }
        else
        {
            CIMValue theValue = instances[i].getProperty(pos).getValue();

            if (theValue.isNull())
            {
                moduleStatus.append(0);
            }
            else
            {
                theValue.get(moduleStatus);
            }
        }

        statusValue = _getModuleStatusValue(moduleStatus);

        MessageLoaderParms msgParms(
            "Common.AuditLogger.CURRENT_PROVIDER_REGISTRATION",
            "Provider module \"$0\" has status \"$1\".",
            moduleName, statusValue);

        _writeAuditMessage(TYPE_CONFIGURATION,
            SUBTYPE_CURRENT_PROVIDER_REGISTRATION,
            EVENT_START_UP, Logger::INFORMATION, msgParms);
    }
}

void AuditLogger::logCurrentEnvironmentVar()
{
#ifdef PEGASUS_OS_DARWIN
    char** envp = *_NSGetEnviron();
#else
    char** envp = environ;
#endif

    Uint32 i = 0;

    while (envp[i])
    {
        MessageLoaderParms msgParms("Common.AuditLogger.CURRENT_ENV",
           "cimserver environment variable: $0", envp[i]);

        _writeAuditMessage(TYPE_CONFIGURATION,
            SUBTYPE_CURRENT_ENVIRONMENT_VARIABLES,
            EVENT_START_UP, Logger::INFORMATION, msgParms);

        i++;
    }
}

void AuditLogger::logSetConfigProperty(
    const String & userName,
    const String & propertyName,
    const String & prePropertyValue,
    const String & newPropertyValue,
    Boolean isPlanned)
{
    if (isPlanned)
    {
        MessageLoaderParms msgParms(
            "Common.AuditLogger.SET_PLANNED_CONFIG_PROPERTY",
            "The planned value of property \"$0\" is modified from "
                "value \"$1\" to value \"$2\" by user \"$3\".",
            propertyName, prePropertyValue, newPropertyValue, userName);

        _writeAuditMessage(TYPE_CONFIGURATION,
            SUBTYPE_CONFIGURATION_CHANGE,
            EVENT_UPDATE, Logger::INFORMATION, msgParms);
    }
    else
    {
        MessageLoaderParms msgParms(
            "Common.AuditLogger.SET_CURRENT_CONFIG_PROPERTY",
            "The current value of property \"$0\" is modified from "
                "value \"$1\" to value \"$2\" by user \"$3\".",
            propertyName, prePropertyValue, newPropertyValue, userName);

        _writeAuditMessage(TYPE_CONFIGURATION,
            SUBTYPE_CONFIGURATION_CHANGE,
            EVENT_UPDATE, Logger::INFORMATION, msgParms);
    }
}

void AuditLogger::logUpdateClassOperation(
    const char* cimMethodName,
    AuditEvent eventType,
    const String& userName,
    const String& ipAddr,
    const CIMNamespaceName& nameSpace,
    const CIMName& className,
    CIMStatusCode statusCode)
{
    MessageLoaderParms msgParms(
        "Common.AuditLogger.OPERATION_UPDATE_CLASS",
        "A CIM $0 operation on class \"$1\" in namespace \"$2\" by user "
            "\"$3\" connected from system \"$4\" resulted in status \"$5\".",
        cimMethodName,
        className.getString(),
        nameSpace.getString(),
        userName,
        ipAddr,
        cimStatusCodeToString(statusCode));

    _writeAuditMessage(TYPE_CIMOPERATION, SUBTYPE_SCHEMA_OPERATION,
        eventType, Logger::INFORMATION, msgParms);
}

void AuditLogger::logUpdateQualifierOperation(
    const char* cimMethodName,
    AuditEvent eventType,
    const String& userName,
    const String& ipAddr,
    const CIMNamespaceName& nameSpace,
    const CIMName& className,
    CIMStatusCode statusCode)
{
    MessageLoaderParms msgParms(
        "Common.AuditLogger.OPERATION_UPDATE_QUALIFIER",
        "A CIM $0 operation on qualifier \"$1\" in namespace \"$2\" by user "
            "\"$3\" connected from system \"$4\" resulted in status \"$5\".",
        cimMethodName,
        className.getString(),
        nameSpace.getString(),
        userName,
        ipAddr,
        cimStatusCodeToString(statusCode));

    _writeAuditMessage(TYPE_CIMOPERATION, SUBTYPE_SCHEMA_OPERATION,
        eventType, Logger::INFORMATION, msgParms);
}

void AuditLogger::logUpdateInstanceOperation(
    const char* cimMethodName,
    AuditEvent eventType,
    const String& userName,
    const String& ipAddr,
    const CIMNamespaceName& nameSpace,
    const CIMObjectPath& instanceName,
    const String& moduleName,
    const String& providerName,
    CIMStatusCode statusCode)
{
    if (providerName != String::EMPTY)
    {
        MessageLoaderParms msgParms(
            "Common.AuditLogger.OPERATION_UPDATE_INSTANCE_WITH_PROVIDER",
            "A CIM $0 operation on instance \"$1\" in namespace \"$2\" by "
                "user \"$3\" connected from system \"$4\" resulted in "
                "status \"$5\".  "
                "The provider for this operation is \"$6\" in module \"$7\".",
            cimMethodName,
            CIMObjectPath("", CIMNamespaceName(), instanceName.getClassName(),
                instanceName.getKeyBindings()).toString(),
            nameSpace.getString(),
            userName,
            ipAddr,
            cimStatusCodeToString(statusCode),
            providerName,
            moduleName);

        _writeAuditMessage(TYPE_CIMOPERATION, SUBTYPE_INSTANCE_OPERATION,
            eventType, Logger::INFORMATION, msgParms);
    }
    else
    {
        MessageLoaderParms msgParms(
            "Common.AuditLogger.OPERATION_UPDATE_INSTANCE",
            "A CIM $0 operation on instance \"$1\" in namespace \"$2\" by "
                "user \"$3\" connected from system \"$4\" resulted in "
                "status \"$5\".  ",
            cimMethodName,
            CIMObjectPath("", CIMNamespaceName(), instanceName.getClassName(),
                instanceName.getKeyBindings()).toString(),
            nameSpace.getString(),
            userName,
            ipAddr,
            cimStatusCodeToString(statusCode));

        _writeAuditMessage(TYPE_CIMOPERATION, SUBTYPE_INSTANCE_OPERATION,
            eventType, Logger::INFORMATION, msgParms);
    }
}

void AuditLogger::logInvokeMethodOperation(
    const String& userName,
    const String& ipAddr,
    const CIMNamespaceName& nameSpace,
    const CIMObjectPath& objectName,
    const CIMName& methodName,
    const String& moduleName,
    const String& providerName,
    CIMStatusCode statusCode)
{
    if (providerName != String::EMPTY)
    {
        MessageLoaderParms msgParms(
            "Common.AuditLogger.OPERATION_INVOKE_METHOD_WITH_PROVIDER",
            "A CIM InvokeMethod operation on method \"$0\" of object \"$1\" "
                "in namespace \"$2\" by user \"$3\" connected from system "
                "\"$4\" resulted in status \"$5\".  The provider for this "
                "operation is \"$6\" in module \"$7\".",
            methodName.getString(),
            CIMObjectPath("", CIMNamespaceName(), objectName.getClassName(),
                objectName.getKeyBindings()).toString(),
            nameSpace.getString(),
            userName,
            ipAddr,
            cimStatusCodeToString(statusCode),
            providerName,
            moduleName);

        _writeAuditMessage(TYPE_CIMOPERATION, SUBTYPE_INSTANCE_OPERATION,
            EVENT_INVOKE, Logger::INFORMATION, msgParms);
    }
    else
    {
        MessageLoaderParms msgParms(
            "Common.AuditLogger.OPERATION_INVOKE_METHOD",
            "A CIM InvokeMethod operation on method \"$0\" of object \"$1\" "
                "in namespace \"$2\" by user \"$3\" connected from system "
                "\"$4\" resulted in status \"$5\".",
            methodName.getString(),
            CIMObjectPath("", CIMNamespaceName(), objectName.getClassName(),
                objectName.getKeyBindings()).toString(),
            nameSpace.getString(),
            userName,
            ipAddr,
            cimStatusCodeToString(statusCode));

        _writeAuditMessage(TYPE_CIMOPERATION, SUBTYPE_INSTANCE_OPERATION,
            EVENT_INVOKE, Logger::INFORMATION, msgParms);
    }
}

void AuditLogger::logSetProvModuleGroupName(
    const String & moduleName,
    const String & oldModuleGroupName,
    const String & newModuleGroupName)
{
    MessageLoaderParms msgParms(
        "Common.AuditLogger.SET_PROVIDER_MODULE_GROUP",
        "The ModuleGroupName of provider module \"$0\" has changed from \"$1\""
            " to \"$2\".",
        moduleName, oldModuleGroupName, newModuleGroupName);

    _writeAuditMessage(TYPE_CONFIGURATION,
        SUBTYPE_PROVIDER_MODULE_STATUS_CHANGE,
        EVENT_UPDATE, Logger::INFORMATION, msgParms);
}


void AuditLogger::logUpdateProvModuleStatus(
    const String & moduleName,
    const Array<Uint16> currentModuleStatus,
    const Array<Uint16> newModuleStatus)
{
    String currentModuleStatusValue =
        _getModuleStatusValue(currentModuleStatus);

    String newModuleStatusValue = _getModuleStatusValue(newModuleStatus);

    MessageLoaderParms msgParms(
        "Common.AuditLogger.UPDATE_PROVIDER_MODULE_STATUS",
        "The operational status of module \"$0\" has changed from \"$1\""
        " to \"$2\".",
        moduleName, currentModuleStatusValue, newModuleStatusValue);

    _writeAuditMessage(TYPE_CONFIGURATION,
        SUBTYPE_PROVIDER_MODULE_STATUS_CHANGE,
        EVENT_UPDATE, Logger::INFORMATION, msgParms);
}

void AuditLogger::logLocalAuthentication(
    const String& userName,
    Boolean successful)
{
    MessageLoaderParms msgParms(
        "Common.AuditLogger.LOCAL_AUTHENTICATION",
        "Local authentication attempt: "
        "successful = $0, user = $1. ",
        CIMValue(successful).toString(),
        userName);

    _writeAuditMessage(
        TYPE_AUTHENTICATION,
        SUBTYPE_LOCAL_AUTHENTICATION,
        successful ? EVENT_AUTH_SUCCESS : EVENT_AUTH_FAILURE,
        successful ? Logger::INFORMATION : Logger::WARNING,
        msgParms);
}

void AuditLogger::logBasicAuthentication(
    const String& userName,
    const String& ipAddr,
    Boolean successful)
{
    MessageLoaderParms msgParms(
        "Common.AuditLogger.BASIC_AUTHENTICATION",
        "Basic authentication attempt: "
        "successful = $0, from IP address = $2, user = $1.",
        CIMValue(successful).toString(),
        userName,
        ipAddr);

    _writeAuditMessage( TYPE_AUTHENTICATION,
        SUBTYPE_BASIC_AUTHENTICATION,
        successful ? EVENT_AUTH_SUCCESS : EVENT_AUTH_FAILURE,
        successful ? Logger::INFORMATION: Logger::WARNING,
        msgParms);
}

void AuditLogger::logCertificateBasedAuthentication(
    const String& issuerName,
    const String& subjectName,
    const String& serialNumber,
    const String& ipAddr,
    Boolean successful)
{
    MessageLoaderParms msgParms(
        "Common.AuditLogger.CERTIFICATE_BASED_AUTHENTICATION",
        "Certificate based authentication attempt: "
            "successful = $0, from IP address = $4, issuer = $1, "
            "subject = $2, serialNumber = $3.",
        CIMValue(successful).toString(),
        issuerName,
        subjectName,
        serialNumber,
        ipAddr);

    _writeAuditMessage(TYPE_AUTHENTICATION,
        SUBTYPE_CERTIFICATE_BASED_AUTHENTICATION,
        successful ? EVENT_AUTH_SUCCESS : EVENT_AUTH_FAILURE,
        successful ? Logger::INFORMATION: Logger::WARNING,
        msgParms);
}

void AuditLogger::logCertificateBasedUserValidation(
    const String& userName,
    const String& issuerName,
    const String& subjectName,
    const String& serialNumber,
    const String& ipAddr,
    Boolean successful)
{
    MessageLoaderParms msgParms(
        "Common.AuditLogger.CERTIFICATE_BASED_USER_VALIDATION",
        "Certificate based user validation attempt: "
            "successful = $0, from IP address = $5, userName = $1, "
            "issuer = $2,  subject = $3, serialNumber = $4.",
        CIMValue(successful).toString(),
        userName,
        issuerName,
        subjectName,
        serialNumber,
        ipAddr);

    _writeAuditMessage( TYPE_AUTHORIZATION,
        SUBTYPE_CERTIFICATE_BASED_USER_VALIDATION,
        successful ? EVENT_AUTH_SUCCESS : EVENT_AUTH_FAILURE,
        successful ? Logger::INFORMATION: Logger::WARNING,
        msgParms);
}

void AuditLogger::setInitializeCallback(
    PEGASUS_AUDITLOGINITIALIZE_CALLBACK_T auditLogInitializeCallback)
{
    _auditLogInitializeCallback = auditLogInitializeCallback;
}

void AuditLogger::setEnabled(Boolean enabled)
{
    // Only write the enable/disable messages if we are set up to handle them
    if (_auditLogInitializeCallback != 0)
    {
        if (enabled)
        {
            if (!_auditLogFlag)
            {
                _auditLogInitializeCallback();

                MessageLoaderParms msgParms(
                    "Common.AuditLogger.ENABLE_AUDIT_LOG",
                    "Audit logging is enabled.");

                _writeAuditMessage(TYPE_CONFIGURATION,
                    SUBTYPE_CONFIGURATION_CHANGE,
                    EVENT_UPDATE, Logger::INFORMATION, msgParms);
            }
        }
        else
        {
            if (_auditLogFlag)
            {
                MessageLoaderParms msgParms(
                    "Common.AuditLogger.DISABLE_AUDIT_LOG",
                    "Audit logging is disabled.");

                _writeAuditMessage(TYPE_CONFIGURATION,
                    SUBTYPE_CONFIGURATION_CHANGE,
                    EVENT_UPDATE, Logger::INFORMATION, msgParms);
            }
        }
    }

    _auditLogFlag = enabled;
}

void AuditLogger::setAuditLogWriterCallback(
    PEGASUS_AUDITLOG_CALLBACK_T writeAuditLogCallback)
{
    _writeAuditMessage = writeAuditLogCallback;
}

void AuditLogger::_writeAuditMessageToLog(
    AuditType,
    AuditSubType,
    AuditEvent,
    Uint32 logLevel,
    MessageLoaderParms & msgParms)
{
    String identifier = "cimserver audit";

    Logger::put_l(Logger::AUDIT_LOG, identifier, logLevel, msgParms);
}

String AuditLogger::_getModuleStatusValue(
    const Array<Uint16>  moduleStatus)
{
    String moduleStatusValue, statusValue;
    Uint32 moduleStatusSize = moduleStatus.size();

    for (Uint32 j=0; j < moduleStatusSize; j++)
    {
        statusValue = providerModuleStatus[moduleStatus[j]];
        moduleStatusValue.append(statusValue);

        if (j < moduleStatusSize - 1)
        {
            moduleStatusValue.append(",");
        }
    }

    return moduleStatusValue;
}

PEGASUS_NAMESPACE_END

#endif
