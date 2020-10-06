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
#include <Pegasus/Common/Audit_zOS_SMF.h>

#include <stdlib.h>

#ifndef PEGASUS_DISABLE_AUDIT_LOGGER

PEGASUS_USING_STD;

PEGASUS_NAMESPACE_BEGIN

// Initialize the global and static variables:
// - Audit logging is disabled.
Boolean AuditLogger::_auditLogFlag = false;

// - The initiation call back is set to zero.
AuditLogger::PEGASUS_AUDITLOGINITIALIZE_CALLBACK_T
    AuditLogger::_auditLogInitializeCallback = 0;

// - The interanl writer for audit messages is used
AuditLogger::PEGASUS_AUDITLOG_CALLBACK_T AuditLogger::_writeAuditMessage =
    Audit_zOS_SMF::writeRecord ;
Boolean _isInternalWriterUsed = true;

// - Instantiate the smf helper class.
Audit_zOS_SMF _smf;

// Definition of the CIM operation types for the
// CIM operation SMF record.
enum _smf_cim_oper_type {
    CLASS_OPERATION=0,
    QUALIFER_OPERATION=1,
    INSTANCE_OPERATION=2,
    INVOKE_METHOD=3
} ;

// Definition of the CIM authentication mode for the
// CIM authentication SMF record.
enum _smf_cim_auth_mode {
    LOCAL=0,
    BASIC=1,
    ATTLS=2,
} ;


void AuditLogger::setInitializeCallback(
        PEGASUS_AUDITLOGINITIALIZE_CALLBACK_T auditLogInitializeCallback)
{
    _auditLogInitializeCallback = auditLogInitializeCallback;
}


void AuditLogger::setAuditLogWriterCallback(
    PEGASUS_AUDITLOG_CALLBACK_T writeAuditLogCallback)
{
    _writeAuditMessage = writeAuditLogCallback;
    _isInternalWriterUsed = false;
}

void AuditLogger::logCurrentConfig(
    const Array<String> & propertyNames,
    const Array<String> & propertyValues)
{
    // check if SMF is gathering this type of records.
    if (_smf.isRecording(CONFIGURATION) ||
        (! _isInternalWriterUsed) )
    {
        unsigned char * confRecord;
        unsigned char * cursor;
        int cursorOffset;

        _smf86_configuration * confSection;
        int nameLen;
        int valueLen;

        // For all properties write a record.
        for (Uint32 i = 0; i < propertyNames.size(); i++)
        {
            // +1 for the 0x00 termination.
            nameLen = propertyNames[i].size()+1;
            valueLen =  propertyValues[i].size()+1;

            // Allocate the full record.
            confRecord = (unsigned char *) calloc(1,
                sizeof(_smf86_configuration_record) + nameLen + valueLen );

            // Initialize the header and product section.
            // The length is the total of subtype section + variable parts.
            _smf.initMyProlog((_smf86_record_prolog *)confRecord, CONFIGURATION,
                sizeof(_smf86_configuration) + nameLen + valueLen );

            // Set the pointer to the subtype section.
            confSection = (_smf86_configuration *)
                (confRecord + sizeof(_smf86_record_prolog));

            // No user Id for logging current configuration.
            _smf.setEBCDICRecordField(confSection->UserID, "",
                sizeof(confSection->UserID),false);

            // Configutation is listed
            confSection->PropChange = 0;

            // The variable values are starting
            // at the end of the subtype section.
            cursor = confRecord + sizeof(_smf86_configuration_record);
            cursorOffset = sizeof(_smf86_configuration);

           // Set the propety name
           confSection->NameOf = cursorOffset;
           confSection->NameNo = 1;
           confSection->NameLen = nameLen;
           _smf.setEBCDICRecordField(cursor,
               (const char*)propertyNames[i].getCString(),
               nameLen,true);

           cursor = cursor + nameLen;
           cursorOffset = cursorOffset + nameLen;

           // Set the property value.
           confSection->ValueOf = cursorOffset;
           confSection->ValueNo = 1;
           confSection->ValueLen = valueLen;
           _smf.setEBCDICRecordField(cursor,
               (const char*)propertyValues[i].getCString(),
               valueLen,true);

            // New property is set to 0, not used at listing the configuration.
            confSection->NewValueOf = 0;
            confSection->NewValueNo = 0;
            confSection->NewValueLen = 0;

            _writeAuditMessage(CONFIGURATION,(char *)confRecord);
            free(confRecord);
        } // End for all properties do.
    }
}

void AuditLogger::logCurrentRegProvider(
    const Array < CIMInstance > & instances)
{
    // check if SMF is gathering this type of records.
    if (_smf.isRecording(CONFIGURATION) ||
        (! _isInternalWriterUsed) )
    {
        unsigned char * provStatRecord;
        unsigned char * cursor;
        int cursorOffset;

        _smf86_provider_status * provStatSection;

        String moduleName;
        int moduleNameLen;
        String statusValue;
        Uint32 pos;

        // For all current registered providers.
        for (Uint32 i = 0; i < instances.size(); i++)
        {
            // Get the module name.
            instances[i].getProperty(instances[i].findProperty(
                _PROPERTY_PROVIDERMODULE_NAME)).getValue().get(moduleName);

            // +1 for the 0x00 termination.
            moduleNameLen = moduleName.size()+1;

            // Allocate the full record.
            provStatRecord = (unsigned char *) calloc(1,
                sizeof(_smf86_provider_status_record) + moduleNameLen );

            // Initialize the header and product section.
            // The length is the total of subtype section + variable parts.
            _smf.initMyProlog((_smf86_record_prolog *)provStatRecord,
                PROVIDER_STATUS,
                sizeof(_smf86_provider_status) + moduleNameLen );

            // Set the pointer to the subtype section.
            provStatSection = (_smf86_provider_status *)
                (provStatRecord + sizeof(_smf86_record_prolog));

            pos = instances[i].findProperty(_PROPERTY_OPERATIONALSTATUS);

            if (pos == PEG_NOT_FOUND)
            {
                provStatSection->CurrStatus = 0;
            }
            else
            {
                CIMValue theValue = instances[i].getProperty(pos).getValue();
                if (theValue.isNull())
                {
                    provStatSection->CurrStatus = 0;
                }
                else
                {
                    Array<Uint16> moduleStatus;
                    // Get the module status
                    theValue.get(moduleStatus);
                    // reset the smf record field
                    provStatSection->CurrStatus = 0;
                    for (int j = 0; j < moduleStatus.size();j++)
                    {
                        // Accumulate the status of the provider
                        // by shifting a bit the value of moduleStatus
                        // times to the left to get the right bit set.
                        provStatSection->CurrStatus =
                            provStatSection->CurrStatus +
                            ( 1 << moduleStatus[j] );
                    }

                }
            } // End of status set.

            // The provider does not change its state.
            provStatSection->IsChanging=0;
            provStatSection->NewStatus=0;

            // The variable values are starting
            // at the end of the subtype section.
            cursor = provStatRecord + sizeof(_smf86_provider_status_record);
            cursorOffset = sizeof(_smf86_provider_status);

            // Set the provider module name.
            provStatSection->ProvNameOf = cursorOffset;
            provStatSection->ProvNameNo = 1;
            provStatSection->ProvNameLen = moduleNameLen;
            _smf.setEBCDICRecordField(cursor,
                (const char*)moduleName.getCString(),
                moduleNameLen,true);

            _writeAuditMessage(PROVIDER_STATUS,(char *)provStatRecord);
            free(provStatRecord);

        } // For all current registered providers.
    }
}

void AuditLogger::logCurrentEnvironmentVar()
{
    // This logging is currently not done within z/OS
}

void AuditLogger::logSetConfigProperty(
    const String & userName,
    const String & propertyName,
    const String & prePropertyValue,
    const String & newPropertyValue,
    Boolean isPlanned)
{
    // check if SMF is gathering this type of records.

    if (_smf.isRecording(CONFIGURATION) ||
        (! _isInternalWriterUsed) )
    {
        unsigned char * confRecord;
        unsigned char * cursor;
        int cursorOffset;

        _smf86_configuration * confSection;

        // +1 for the 0x00 termination
        int nameLen = propertyName.size()+1;
        int valueLen =  prePropertyValue.size()+1;
        int newValueLen =  newPropertyValue.size()+1;

        // allocate the full record.
        confRecord = (unsigned char *) calloc(1,
                          sizeof(_smf86_configuration_record)
                          + nameLen + valueLen + newValueLen );

        // Initialize the header and product section.
        // The length is the total of subtype section + variable parts.
        _smf.initMyProlog((_smf86_record_prolog *)confRecord,
            CONFIGURATION,sizeof(_smf86_configuration) +
            nameLen + valueLen + newValueLen );


        // Set the pointer to the subtype section.
        confSection = (_smf86_configuration *)
                         (confRecord + sizeof(_smf86_record_prolog));

       // Set the user id.
        _smf.setEBCDICRecordField(confSection->UserID,
                                  (const char*)userName.getCString(),
                                  sizeof(confSection->UserID),false);

        if (isPlanned)
        {
            // It is a planed configuration change.
            confSection->PropChange = 2;
        }
        else
        {
            // It is a current configuration change.
            confSection->PropChange = 1;
        }


        // The variable values are starting
        // at the end of the subtype section.
        cursor = confRecord + sizeof(_smf86_configuration_record);
        cursorOffset = sizeof(_smf86_configuration);

        // set the propety name
        confSection->NameOf = cursorOffset;
        confSection->NameNo = 1;
        confSection->NameLen = nameLen;
        _smf.setEBCDICRecordField(cursor,
            (const char*)propertyName.getCString(),
            nameLen,true);

        cursor = cursor + nameLen;
        cursorOffset = cursorOffset + nameLen;

        // set the property value
        confSection->ValueOf = cursorOffset;
        confSection->ValueNo = 1;
        confSection->ValueLen = valueLen;
        _smf.setEBCDICRecordField(cursor,
            (const char*)prePropertyValue.getCString(),
            valueLen,true);

        cursor = cursor + valueLen;
        cursorOffset = cursorOffset + valueLen;

        // set the new property value
        confSection->NewValueOf = cursorOffset;
        confSection->NewValueNo = 1;
        confSection->NewValueLen = newValueLen;
        _smf.setEBCDICRecordField(cursor,
            (const char*)newPropertyValue.getCString(),
            newValueLen,true);

        _writeAuditMessage(CONFIGURATION,(char *)confRecord);
        free(confRecord);
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
    // check if SMF is gathering this type of records.
    if (_smf.isRecording(CIM_OPERATION) ||
        (! _isInternalWriterUsed) )
    {
        _writeCIMOperationRecord(
            CLASS_OPERATION, userName, statusCode,
            ipAddr, cimMethodName, className.getString(),
            nameSpace.getString(), String::EMPTY, String::EMPTY );
    }
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
    // check if SMF is gathering this type of records.
    if (_smf.isRecording(CIM_OPERATION) ||
        (! _isInternalWriterUsed) )
    {
        _writeCIMOperationRecord(
            QUALIFER_OPERATION, userName, statusCode,
            ipAddr, cimMethodName, className.getString(),
            nameSpace.getString(), String::EMPTY, String::EMPTY );
    }
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
    // check if SMF is gathering this type of records.
    if (_smf.isRecording(CIM_OPERATION) ||
        (! _isInternalWriterUsed) )
    {
        String cimInstanceName =
            CIMObjectPath("", CIMNamespaceName(), instanceName.getClassName(),
            instanceName.getKeyBindings()).toString();

        _writeCIMOperationRecord(
            INSTANCE_OPERATION, userName, statusCode,
            ipAddr, cimMethodName, cimInstanceName,
            nameSpace.getString(), providerName, moduleName );
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
    // check if SMF is gathering this type of records.
    if (_smf.isRecording(CIM_OPERATION) ||
        (! _isInternalWriterUsed) )
    {
        String cimObjectName =
            CIMObjectPath("", CIMNamespaceName(), objectName.getClassName(),
            objectName.getKeyBindings()).toString();

        _writeCIMOperationRecord(
            INVOKE_METHOD, userName, statusCode,
            ipAddr, methodName.getString(), cimObjectName,
            nameSpace.getString(), providerName,moduleName );
    }
}

void AuditLogger::logSetProvModuleGroupName(
    const String & moduleName,
    const String & oldModuleGroupName,
    const String & newModuleGroupName)
{
// ATTN: Implement for z/OS.
}

void AuditLogger::logUpdateProvModuleStatus(
    const String & moduleName,
    const Array<Uint16> currentModuleStatus,
    const Array<Uint16> newModuleStatus)
{
        // check if SMF is gathering this type of records.
    if (_smf.isRecording(CONFIGURATION) ||
        (! _isInternalWriterUsed) )
    {
        unsigned char * provStatRecord;
        unsigned char * cursor;
        int cursorOffset;

        _smf86_provider_status * provStatSection;

        // +1 for the 0x00 termination.
        int moduleNameLen = moduleName.size()+1;;
        String statusValue;
        Uint32 pos;

        // Allocate the full record.
        provStatRecord = (unsigned char *) calloc(1,
            sizeof(_smf86_provider_status_record) + moduleNameLen );

        // Initialize the header and product section.
        // The length is the total of subtype section + variable parts.
        _smf.initMyProlog((_smf86_record_prolog *)provStatRecord,
            PROVIDER_STATUS,
            sizeof(_smf86_provider_status) + moduleNameLen );

        // Set the pointer to the subtype section.
        provStatSection = (_smf86_provider_status *)
            (provStatRecord + sizeof(_smf86_record_prolog));

        provStatSection->CurrStatus = 0;

        if (currentModuleStatus.size() > 0)
        {
            for (int j = 0; j < currentModuleStatus.size();j++)
            {
                // Accumulate the status of the provider
                // by shifting a bit the value of moduleStatus
                // times to the left to get the right bit set.
                provStatSection->CurrStatus =
                    provStatSection->CurrStatus +
                    ( 1 << currentModuleStatus[j] );
            }

        }

        // The provider does change.
        provStatSection->IsChanging=1;

        provStatSection->NewStatus=0;

        if (newModuleStatus.size() > 0)
        {
            // Accumulate the new status of the provider
            // by shifting a bit the value of moduleStatus
            // times to the left to get the right bit set.
            for (int j = 0; j < newModuleStatus.size();j++)
            {
                provStatSection->NewStatus =
                    provStatSection->NewStatus + ( 1 << newModuleStatus[j] );
            }

        }

        // The variable values are starting
        // at the end of the subtype section.
        cursor = provStatRecord + sizeof(_smf86_provider_status_record);
        cursorOffset =  sizeof(_smf86_provider_status);

        // Set the provider module name.
        provStatSection->ProvNameOf =cursorOffset;
        provStatSection->ProvNameNo = 1;
        provStatSection->ProvNameLen = moduleNameLen;
        _smf.setEBCDICRecordField(cursor,
            (const char*)moduleName.getCString(),
            moduleNameLen,true);

        _writeAuditMessage(PROVIDER_STATUS,(char *)provStatRecord);
        free(provStatRecord);
    }

}

void AuditLogger::logLocalAuthentication(
    const String& userName,
    Boolean successful)
{
    // check if SMF is gathering this type of records.
    if (_smf.isRecording(AUTHENTICATION) ||
        (! _isInternalWriterUsed) )
    {
        _writeAuthenticationRecord(LOCAL,userName,successful,
            "localhost");
    }
}

void AuditLogger::logBasicAuthentication(
    const String& userName,
    const String& ipAddr,
    Boolean successful)
{
    // check if SMF is gathering this type of records.
    if (_smf.isRecording(AUTHENTICATION) ||
        (! _isInternalWriterUsed) )
    {
        _writeAuthenticationRecord(BASIC,userName,successful,
            ipAddr);
    }
}

void AuditLogger::logCertificateBasedAuthentication(
    const String& issuerName,
    const String& subjectName,
    const String& serialNumber,
    const String& ipAddr,
    Boolean successful)
{
    // This fuction is unimplemented within z/OS.
    // AT-TLS is doing this kind of logging.
}

void AuditLogger::logCertificateBasedUserValidation(
    const String& userName,
    const String& issuerName,
    const String& subjectName,
    const String& serialNumber,
    const String& ipAddr,
    Boolean successful)
{
    // check if SMF is gathering this type of records.
    if (_smf.isRecording(AUTHENTICATION) ||
        (! _isInternalWriterUsed) )
    {
        _writeAuthenticationRecord(ATTLS,userName,successful,
            ipAddr);
    }

}


void AuditLogger::setEnabled(Boolean enable)
{
    if (enable)
    {
        if (!_auditLogFlag)
        {
            Logger::put_l(Logger::STANDARD_LOG, System::CIMSERVER,
                Logger::INFORMATION,
                MessageLoaderParms(
                    "Common.AuditLogger.ENABLE_AUDIT_LOG",
                    "Audit logging is enabled."));

            // only call the audit intiialzation call back if set.
            if (_auditLogInitializeCallback != 0)
            {
                _auditLogInitializeCallback();
            }
        }
    }
    else
    {
        if (_auditLogFlag)
        {
            Logger::put_l(Logger::STANDARD_LOG, System::CIMSERVER,
                Logger::INFORMATION,
                MessageLoaderParms(
                    "Common.AuditLogger.DISABLE_AUDIT_LOG",
                    "Audit logging is disabled."));
        }
    }

    _auditLogFlag = enable;

}

inline void AuditLogger::_writeCIMOperationRecord(
    unsigned short cimOpType,
    String userID,
    unsigned short cimStatusCode,
    String clientIP,
    String operName,
    String objPath,
    String nameSpace,
    String provName,
    String provModName
    )
{
    unsigned char * cimOperRecord;
    unsigned char * cursor;
    int cursorOffset;

    _smf86_cim_operation * cimOperSection;

    int operNameLen = operName.size();
    int objPathLen = objPath.size();
    int nameSpaceLen = nameSpace.size();
    int provNameLen = provName.size();
    int provModNameLen = provModName.size();

    // if a string is not empty, + 1 for the trailing 0x00
    if( operNameLen > 0 )
        operNameLen = operNameLen + 1 ;

    if( objPathLen > 0 )
        objPathLen = objPathLen + 1;

    if( nameSpaceLen > 0 )
        nameSpaceLen = nameSpaceLen + 1;

    if( provNameLen > 0 )
        provNameLen = provNameLen + 1;

    if( provModNameLen > 0 )
        provModNameLen = provModNameLen + 1;

    // Allocate the full record.
    cimOperRecord = (unsigned char *) calloc(1,
        sizeof(_smf86_cim_operation_record)+
        operNameLen + objPathLen + nameSpaceLen +
        provNameLen + provModNameLen );

    // Initialize the header and product section.
    // The length is the total of subtype section + variable parts.
    _smf.initMyProlog((_smf86_record_prolog *)cimOperRecord,
        CIM_OPERATION, sizeof(_smf86_cim_operation)+
        operNameLen + objPathLen + nameSpaceLen +
        provNameLen + provModNameLen );

    // Set the pointer to the subtype section.
    cimOperSection = (_smf86_cim_operation *)
        (cimOperRecord + sizeof(_smf86_record_prolog));

    // Set cim operation type.
    cimOperSection->CIMOpType = cimOpType;

    // Set cim operation status code
    cimOperSection->CIMStatusCode = cimStatusCode;

    // Set the user id.
    _smf.setEBCDICRecordField(cimOperSection->UserID,
                              (const char*)userID.getCString(),
                              sizeof(cimOperSection->UserID),false);

    // Set the client IP adress.
    _smf.setEBCDICRecordField(cimOperSection->ClientIP,
                              (const char*)clientIP.getCString(),
                              sizeof(cimOperSection->ClientIP),false);


    // The operation name starts from the end of the static record information.
    cursor = cimOperRecord + sizeof(_smf86_cim_operation_record);
    cursorOffset = sizeof(_smf86_cim_operation);

    // Set operation name
    if (operNameLen > 0 )
    {
        cimOperSection->OperNameOf = cursorOffset;
        cimOperSection->OperNameLen = operNameLen;
        cimOperSection->OperNameNo = 1;
        _smf.setEBCDICRecordField(cursor,(const char*)operName.getCString(),
                                  operNameLen,true);
        cursor = cursor + operNameLen;
        cursorOffset = cursorOffset + operNameLen;
    }
    else
    {
        cimOperSection->OperNameOf = 0;
        cimOperSection->OperNameLen = 0;
        cimOperSection->OperNameNo = 0;
    }

    // Set object path
    if (objPathLen > 0 )
    {
        cimOperSection->ObjPathOf = cursorOffset;
        cimOperSection->ObjPathLen = objPathLen;
        cimOperSection->ObjPathNo = 1;
        _smf.setEBCDICRecordField(cursor,(const char*)objPath.getCString(),
                                  objPathLen,true);
        cursor = cursor + objPathLen;
        cursorOffset = cursorOffset + objPathLen;
    }
    else
    {
        cimOperSection->ObjPathOf = 0;
        cimOperSection->ObjPathLen = 0;
        cimOperSection->ObjPathNo = 0;
    }

    // Set name space
    if (nameSpaceLen > 0 )
    {
        cimOperSection->NameSpaceOf = cursorOffset;
        cimOperSection->NameSpaceLen = nameSpaceLen;
        cimOperSection->NameSpaceNo = 1;
        _smf.setEBCDICRecordField(cursor,(const char*)nameSpace.getCString(),
                                  nameSpaceLen,true);
        cursor = cursor + nameSpaceLen;
        cursorOffset = cursorOffset + nameSpaceLen;
    }
    else
    {
        cimOperSection->NameSpaceOf = 0;
        cimOperSection->NameSpaceLen = 0;
        cimOperSection->NameSpaceNo = 0;
    }

    // Set provider name
    if (provNameLen > 0 )
    {
        cimOperSection->ProvNameOf = cursorOffset;
        cimOperSection->ProvNameLen = provNameLen;
        cimOperSection->ProvNameNo = 1;
        _smf.setEBCDICRecordField(cursor,(const char*)provName.getCString(),
                                  provNameLen,true);
        cursor = cursor + provNameLen;
        cursorOffset = cursorOffset + provNameLen;
    }
    else
    {
        cimOperSection->ProvNameOf = 0;
        cimOperSection->ProvNameLen = 0;
        cimOperSection->ProvNameNo = 0;
    }

    // Set provider module name
    if (provModNameLen > 0 )
    {
        cimOperSection->ProvModNameOf = cursorOffset;
        cimOperSection->ProvModNameLen = provModNameLen;
        cimOperSection->ProvModNameNo = 1;
        _smf.setEBCDICRecordField(cursor,(const char*)provModName.getCString(),
                                  provModNameLen,true);
        cursor = cursor + provModNameLen;
        cursorOffset = cursorOffset + provModNameLen;
    }
    else
    {
        cimOperSection->ProvModNameOf = 0;
        cimOperSection->ProvModNameLen = 0;
        cimOperSection->ProvModNameNo = 0;
    }

    _writeAuditMessage(CIM_OPERATION,(char *)cimOperRecord);
    free(cimOperRecord);


}

inline void AuditLogger::_writeAuthenticationRecord(
    unsigned short authMode,
    String userID,
    Boolean isAuthenticated,
    String clientIP )
{
    unsigned char * authenticationRecord;
    _smf86_authentication * authenticationSection;

    // Allocate the full record.
    authenticationRecord = (unsigned char *) calloc(1,
        sizeof(_smf86_auththentication_record));

    // Initialize the header and product section.
    // The length is the total of subtype section + variable parts.
    _smf.initMyProlog((_smf86_record_prolog *)authenticationRecord,
        AUTHENTICATION, sizeof(_smf86_authentication));

    // Set the pointer to the subtype section.
    authenticationSection = (_smf86_authentication *)
                     (authenticationRecord + sizeof(_smf86_record_prolog));

    authenticationSection->AuthMode = authMode;

    // Set the user id.
    _smf.setEBCDICRecordField(authenticationSection->UserID,
                              (const char*)userID.getCString(),
                              sizeof(authenticationSection->UserID),false);

    if(isAuthenticated)
    {
        authenticationSection->AuthResult=0;
    }
    else
    {
        authenticationSection->AuthResult=1;
    }

    // Set the client IP adress.
    _smf.setEBCDICRecordField(authenticationSection->ClientIP,
                              (const char*)clientIP.getCString(),
                              sizeof(authenticationSection->ClientIP),false);

    _writeAuditMessage(AUTHENTICATION,(char *)authenticationRecord);
    free(authenticationRecord);
}

PEGASUS_NAMESPACE_END

#endif
