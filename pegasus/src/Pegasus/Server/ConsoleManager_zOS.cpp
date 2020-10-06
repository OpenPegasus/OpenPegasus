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
//%////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////
//  ConsoleManager
/////////////////////////////////////////////////////////////////////////////

#include <Pegasus/Common/Constants.h>
#include <Pegasus/Common/Tracer.h>
#include <Pegasus/Common/Logger.h>
#include <Pegasus/Common/PegasusVersion.h>
#include <Pegasus/Common/FileSystem.h>
#include <Pegasus/Common/AuditLogger.h>
#include <Pegasus/Config/ConfigManager.h>

#include <sys/__messag.h>

#include "ConsoleManager_zOS.h"
#include "CIMServer.h"

PEGASUS_NAMESPACE_BEGIN

#define ZOSCONSOLEMANAGER_TOKEN_CONFIG  "CONFIG,"
#define ZOSCONSOLEMANAGER_TOKEN_VERSION "VERSION"
#define ZOSCONSOLEMANAGER_TOKEN_ENV     "ENV"
#define ZOSCONSOLEMANAGER_TOKEN_PLANNED "PLANNED"

enum CommandType
{
    consoleCmdConfig,
    consoleCmdVersion,
    consoleCmdEnv,
    consoleCmdInvalid
};


char* ZOSConsoleManager::_skipBlanks( char* commandPtr)
{
    if (commandPtr != NULL)
    {
        while (*commandPtr == ' ')
        {
            commandPtr++;
        }
    }

    return commandPtr;
}

void ZOSConsoleManager::_stripTrailingBlanks( char* token )
{
    if (token != NULL)
    {
        int len = strlen(token)-1;

        while ((len >= 0) && (token[len] == ' '))
        {
            token[len] = '\0';
            len--;
        }
    }

    return;
}

void ZOSConsoleManager::_displayServiceLevel()
{
    PEG_METHOD_ENTER(TRC_SERVER,
        "ZOSConsoleManager::_displayServiceLevel");

    // PEGASUS_ZOS_SERVICE_STRING is defined in the z/OS platform make file
    String serviceString(STRLIT_ARGS(PEGASUS_ZOS_SERVICE_STRING));

    Logger::put_l(
        Logger::STANDARD_LOG, System::CIMSERVER, Logger::INFORMATION,
        MessageLoaderParms(
            "Server.ConsoleManager_zOS.VERSION.PEGASUS_OS_ZOS",
            "CIM Server Service Level: $0",
            serviceString));

    PEG_METHOD_EXIT();
}

void ZOSConsoleManager::_issueSyntaxError(const char* command)
{
    PEG_METHOD_ENTER(TRC_SERVER,
        "ZOSConsoleManager::_issueSyntaxError");

    Logger::put_l(
        Logger::ERROR_LOG, System::CIMSERVER, Logger::SEVERE,
        MessageLoaderParms(
            "Server.ConsoleManager_zOS.CON_SYNTAX_ERR.PEGASUS_OS_ZOS",
            "CIM MODIFY COMMAND REJECTED DUE TO SYNTAX ERROR"));

    if (!strncmp(command,STRLIT_ARGS(ZOSCONSOLEMANAGER_TOKEN_CONFIG)))
    {
        Logger::put_l(
            Logger::STANDARD_LOG, System::CIMSERVER, Logger::INFORMATION,
            MessageLoaderParms(
                "Server.ConsoleManager_zOS.CON_MODIFY_SYNTAX.PEGASUS_OS_ZOS",
                "Syntax is: "
                    "MODIFY CFZCIM,APPL=CONFIG,<name>=<value>[,PLANNED]"));
    } 
    else if (!strncmp(command,STRLIT_ARGS(ZOSCONSOLEMANAGER_TOKEN_ENV)))
    {
        Logger::put_l(
            Logger::STANDARD_LOG, System::CIMSERVER, Logger::INFORMATION,
            MessageLoaderParms(
                "Server.ConsoleManager_zOS."
                    "CON_MODIFY_ENV_SYNTAX.PEGASUS_OS_ZOS",
                "Syntax is:"
                    " MODIFY CFZCIM,APPL=ENV[,<varname>][=<value>]"));
    }

    PEG_METHOD_EXIT();
}


void ZOSConsoleManager::_updateConfiguration( 
    const String& configProperty,
    const String& propertyValue,
    Boolean currentValueIsNull,
    Boolean planned)
{
    PEG_METHOD_ENTER(TRC_SERVER,"ZOSConsoleManager::_updateConfiguration");

    String preValue;
    String currentValue;
    String displayValue;
    String defaultValue;

    try
    {
        ConfigManager* _configManager = ConfigManager::getInstance();

        preValue = _configManager->getCurrentValue(configProperty);
        defaultValue = _configManager->getDefaultValue(configProperty);

        //If currentValueIsNull is true, the updateCurrentValue() will use the
        // default value, so we need the default value for the messages.
        if(currentValueIsNull)
        {
            displayValue = defaultValue;
        }
        else
        {
            displayValue = propertyValue;
        }

        if (!planned)
        {
            //
            // Update the current value
            //
            if ( !_configManager->updateCurrentValue(
                                      configProperty,
                                      propertyValue,
                                      System::getEffectiveUserName(),
                                      0,
                                      currentValueIsNull) )
            {
                Logger::put_l(
                    Logger::ERROR_LOG, System::CIMSERVER, Logger::SEVERE,
                    MessageLoaderParms(
                        "Server.ConsoleManager_zOS."
                            "CON_MODIFY_FAILED.PEGASUS_OS_ZOS",
                        "Failed to update CONFIG value."));
            }
            else
            {
                Logger::put_l(
                    Logger::STANDARD_LOG, System::CIMSERVER,Logger::INFORMATION,
                    MessageLoaderParms(
                        "Server.ConsoleManager_zOS."
                            "CON_MODIFY_UPDATED.PEGASUS_OS_ZOS",
                        "Updated current value for $0 to $1",
                        configProperty,
                        displayValue));
            }
        }
        else
        {
            //
            // Update the planned value
            //
            if ( !_configManager->updatePlannedValue(configProperty,
                                                     propertyValue,
                                                     currentValueIsNull) )
            {
                Logger::put_l(
                    Logger::ERROR_LOG, System::CIMSERVER, Logger::SEVERE,
                    MessageLoaderParms(
                        "Server.ConsoleManager_zOS."
                            "CON_MODIFY_FAILED.PEGASUS_OS_ZOS",
                        "Failed to update CONFIG value."));
            }
            else
            {
                Logger::put_l(
                    Logger::STANDARD_LOG, System::CIMSERVER,
                    Logger::INFORMATION,
                    MessageLoaderParms(
                        "Server.ConsoleManager_zOS."
                            "CON_MODIFY_PLANNED.PEGASUS_OS_ZOS",
                        "Updated planned value for $0 to $1",
                        configProperty,
                        displayValue));

                Logger::put_l(
                    Logger::STANDARD_LOG, System::CIMSERVER,
                    Logger::INFORMATION,
                    MessageLoaderParms(
                        "Server.ConsoleManager_zOS."
                            "CON_MODIFY_PLANNED2.PEGASUS_OS_ZOS",
                        "This change will become effective "
                        "after CIM Server restart."));
            }
        }

        // send notify config change message to ProviderManager Service
        _sendNotifyConfigChangeMessage(displayValue,
                                       currentValue,
                                          !planned);

        PEG_AUDIT_LOG(logSetConfigProperty("OPERATOR",
                                           displayValue,
                                           preValue,
                                           currentValue,
                                           planned));

    }
    catch(const Exception& e)
    {
        Logger::put_l(
            Logger::ERROR_LOG, System::CIMSERVER, Logger::SEVERE,
            MessageLoaderParms(
                "Server.ConsoleManager_zOS.CON_MODIFY_ERR.PEGASUS_OS_ZOS",
                "MODIFY command failed: \"$0\"",
                e.getMessage()));
    }
    PEG_METHOD_EXIT();
}


void ZOSConsoleManager::_updateEnvironment( 
    const char* envVarName,
    const char* envVarValue)
{
    PEG_METHOD_ENTER(TRC_SERVER,
        "ZOSConsoleManager::_updateEnvironment");

    String envVarNameString(envVarName);

    int rc=setenv(envVarName,envVarValue,1);

    if (rc==0)
    {
        String envVarValueString;
        if (envVarValue!=NULL)
        {
            envVarValueString.assign(envVarValue);
            Logger::put_l(
                Logger::STANDARD_LOG, System::CIMSERVER, Logger::INFORMATION,
                MessageLoaderParms(
                    "Server.ConsoleManager_zOS."
                        "CON_MODIFY_SETENV.PEGASUS_OS_ZOS",
                    "Environment variable \"$0\" set to \"$1\" successfully.",
                    envVarNameString, 
                    envVarValueString));
        }
        else
        {
            Logger::put_l(
                Logger::STANDARD_LOG, System::CIMSERVER, Logger::INFORMATION,
                MessageLoaderParms(
                    "Server.ConsoleManager_zOS."
                        "CON_MODIFY_DELETEENV.PEGASUS_OS_ZOS",
                    "Environment variable \"$0\" deleted successfully.",
                    envVarNameString));
        }
    }
    else
    {
        Logger::put_l(
            Logger::ERROR_LOG, System::CIMSERVER, Logger::SEVERE,
            MessageLoaderParms(
                "Server.ConsoleManager_zOS.CON_MODIFYENV_FAILED.PEGASUS_OS_ZOS",
                "Failed to update environment variable \"$0\".",
                envVarNameString));
    }

    // To be 100% complete we would need an AuditLog here.
    // But since we don't track env vars for z/OS, this is
    // omitted intentionally.

    PEG_METHOD_EXIT();
}


/******************************************************************************
 Either displays the value of a specific environment variable
 (envVarName!=null) or the complete list of all currently defined
 environment variables with their values.
******************************************************************************/
void ZOSConsoleManager::_showEnvironment(const char* envVarName)
{
    PEG_METHOD_ENTER(TRC_SERVER,
        "ZOSConsoleManager::_showEnvironment");

    // See XL C/C++ Runtime Library Reference for documentation on the
    // **environ variable
    extern char** environ;

    if ((0==envVarName) || strlen(envVarName)==0)
    {
        // Display list of all environment variables
        char** var;
        char varMessage[4080];
        char* varMsgPtr = &(varMessage[0]);
        unsigned int varMsgLength = 0;
        // For each line we add 3 extra characters: '-' + ' ' + '\n'
        const unsigned int extraCharsPerLine = 3;

        var = environ;
        while (*var != NULL)
        {
            int varLen = strlen(*var);
            varMsgLength += (varLen+extraCharsPerLine);

            // The Logger will not display messages longer than 4000 characters
            // Therefore we list the environment variables in multiple messages
            // when the size of the environment string exceeds 3900.
            do
            {
                *varMsgPtr='-';     // extra char #1
                varMsgPtr++;
                *varMsgPtr=' ';     // extra char #2
                varMsgPtr++;
                memcpy(varMsgPtr,*var,varLen);
                varMsgPtr+=varLen;
                *varMsgPtr='\n';    // extra char #3
                varMsgPtr++;

                // Move on to the next env var
                ++var;
                varLen = strlen(*var);
                varMsgLength += (varLen+extraCharsPerLine);
            } while ((varMsgLength < 3900) && (*var != NULL));

            *varMsgPtr = '\0';
            String envVarString(varMessage);
            Logger::put_l(
                Logger::STANDARD_LOG, System::CIMSERVER,
                Logger::INFORMATION,
                MessageLoaderParms(
                    "Server.ConsoleManager_zOS."
                        "CON_MODIFY_DISPLAY_ALLENV.PEGASUS_OS_ZOS",
                    "CFZENV: $0",envVarString));

            varMsgPtr = &(varMessage[0]);
            varMsgLength = 0;

        } // end for
    }
    else
    {
        // Just display a single environment variable
        String envVarNameString(envVarName);
        const char* envVarValue = getenv(envVarName);
        if (envVarValue)
        {
            String envVarValueString(envVarValue);
            Logger::put_l(
                Logger::STANDARD_LOG, System::CIMSERVER,
                Logger::INFORMATION,
                MessageLoaderParms(
                    "Server.ConsoleManager_zOS."
                        "CON_MODIFY_DISPLAY_ENV.PEGASUS_OS_ZOS",
                    "CFZENV: $0=$1",
                    envVarNameString, envVarValueString));
        }
        else
        {
            Logger::put_l(
                Logger::ERROR_LOG, System::CIMSERVER, Logger::SEVERE,
                MessageLoaderParms(
                    "Server.ConsoleManager_zOS."
                        "CON_MODIFY_ENV_UNDEFINED.PEGASUS_OS_ZOS",
                    "CFZENV: Variable '$0' is undefined",
                    envVarNameString));
        }
    }
    PEG_METHOD_EXIT();
}

/******************************************************************************
 Syntax for Modify command is:

 MODIFY <jobname>,APPL=CONFIG,<properyname>=['<value>'|<value>][,PLANNED]
                       VERSION
                       ENV[,<varname>][=<value>]

 Parameter <command> represents the string following the "APPL=" token.

******************************************************************************/
void ZOSConsoleManager::processModifyCommand( char* command )
{
    PEG_METHOD_ENTER(TRC_SERVER,
        "ZOSConsoleManager::processModifyCommand");

    char* currentPtr = command;
    char* cmdPtr = NULL;
    char* cfgProperty = NULL;
    char* cfgValue = NULL;
    char* planned = NULL;
    Boolean currentValueIsNull = false;
    Boolean valueIsQuoted = false;
    CommandType consCmd=consoleCmdInvalid;


    currentPtr = _skipBlanks(currentPtr);

    if (!memcmp(currentPtr,STRLIT_ARGS(ZOSCONSOLEMANAGER_TOKEN_CONFIG)))
    {
        currentPtr += strlen(ZOSCONSOLEMANAGER_TOKEN_CONFIG);
        consCmd = consoleCmdConfig;
    }
    else if (!memcmp(currentPtr,STRLIT_ARGS(ZOSCONSOLEMANAGER_TOKEN_VERSION)))
    {
        consCmd = consoleCmdVersion;
        _displayServiceLevel();
        PEG_METHOD_EXIT();
        return;
    }
    else if (!memcmp(currentPtr,STRLIT_ARGS(ZOSCONSOLEMANAGER_TOKEN_ENV)))
    {
        consCmd = consoleCmdEnv;
        currentPtr += strlen(ZOSCONSOLEMANAGER_TOKEN_ENV);
        if (*currentPtr == ',')
        {
            currentPtr++;
        }
    }
    else
    {
        _issueSyntaxError(command);
        PEG_METHOD_EXIT();
        return;
    }


    // Here currentPtr points after the [CONFIG,|VERSION|ENV] token.
    // Following is either the name of a variable or nothing
    currentPtr = _skipBlanks(currentPtr);

    cfgProperty = currentPtr;
    currentPtr = strchr(currentPtr,'=');

    if (currentPtr==NULL)
    {
        if (consCmd == consoleCmdEnv)
        {
            _showEnvironment(cfgProperty);
            PEG_METHOD_EXIT();
            return;
        }
        else
        {
            _issueSyntaxError(command);
            PEG_METHOD_EXIT();
            return;
        }
    }
    else
    {
        // skip the "="
        *currentPtr = '\0';
        currentPtr++;

        currentPtr = _skipBlanks(currentPtr);

        if (*currentPtr == '\0' || *currentPtr ==',')
        {
            currentValueIsNull=true;
        }
        else if (*currentPtr == '\'')
        {
            // Check if value is enclosed in quotes
            char* temp = strchr(currentPtr+1,'\'');
            if (temp!=NULL)
            {
                // skip the starting "'"
                *currentPtr = '\0';
                currentPtr++;

                cfgValue = currentPtr;
                currentPtr = temp;

                // skip the ending "'"
                *currentPtr = '\0';
                currentPtr++;
                valueIsQuoted = true;
            }
            else
            {
                _issueSyntaxError(command);
                PEG_METHOD_EXIT();
                return;
            }
        }
        else
        {
            cfgValue = currentPtr;
        }
    }

    currentPtr = _skipBlanks(currentPtr);

    planned = strchr(currentPtr,',');
    if (planned!=NULL)
    {
        *planned = '\0';
        planned++;

        planned = _skipBlanks(planned);

        if (memcmp(planned,STRLIT_ARGS(ZOSCONSOLEMANAGER_TOKEN_PLANNED)))
        {
            _issueSyntaxError(command);
            PEG_METHOD_EXIT();
            return;
        }
    }


    _stripTrailingBlanks( cfgProperty );
    PEG_TRACE((TRC_SERVER, Tracer::LEVEL4,"Update property: %s", cfgProperty));

    if (currentValueIsNull)
    {
        PEG_TRACE_CSTRING(TRC_SERVER, Tracer::LEVEL4,
            "Set property with null value");
    }
    else
    {
        _stripTrailingBlanks( cfgValue );
        PEG_TRACE((TRC_SERVER, Tracer::LEVEL4,
            "Update property value to: %s", cfgValue));
    }

    if (planned != NULL)
    {
        PEG_TRACE_CSTRING(TRC_SERVER, Tracer::LEVEL4,
            "Updating planned value");
    }

    if (consCmd == consoleCmdEnv)
    {
        _updateEnvironment(cfgProperty, cfgValue);
        PEG_METHOD_EXIT();
        return;
    }

    String propertyString(cfgProperty);
    String propertyValue;

    if (!currentValueIsNull)
    {
         propertyValue.assign(cfgValue);

         if (!valueIsQuoted)
         {
             // All values that were not enclosed in quotes are
             // converted to lowercase
             propertyValue.toLower();
         }

    }

    _updateConfiguration(propertyString,
                        propertyValue,
                        currentValueIsNull,
                        planned);
    PEG_METHOD_EXIT();
}


void ZOSConsoleManager::startConsoleWatchThread(void)
{
    PEG_METHOD_ENTER(TRC_SERVER,
        "ZOSConsoleManager::startConsoleWatchThread");

    pthread_t thid;

    if ( pthread_create(&thid,
                        NULL,
                        ZOSConsoleManager::_consoleCommandWatchThread,
                        NULL) != 0 )
    {
        char str_errno2[10];
        sprintf(str_errno2,"%08X",__errno2());
        Logger::put_l(Logger::ERROR_LOG, System::CIMSERVER, Logger::SEVERE,
            MessageLoaderParms(
                "Server.ConsoleManager_zOS.NO_CONSOLE_THREAD.PEGASUS_OS_ZOS",
                "CIM Server Console command thread cannot be created: "
                    "$0 ( errno $1, reason code 0x$2 ).",
                strerror(errno),
                errno,
                str_errno2));
    }

    PEG_METHOD_EXIT();
}



//
// z/OS console interface waiting for operator stop command
//
void* ZOSConsoleManager::_consoleCommandWatchThread(void*)
{
    PEG_METHOD_ENTER(TRC_SERVER,
        "ZOSConsoleManager::_consoleCommandWatchThread");

    struct __cons_msg    cons;
    int                  concmd=0;
    char                 modstr[128];
    int                  rc;

    memset(&cons,0,sizeof(cons));
    memset(modstr,0,sizeof(modstr));

    do
    {
        rc = __console(&cons, modstr, &concmd);

        if (rc != 0)
        {
            int errornumber = errno;
            char str_errno2[10];
            sprintf(str_errno2,"%08X",__errno2());

            Logger::put_l(
                Logger::ERROR_LOG, System::CIMSERVER, Logger::SEVERE,
                MessageLoaderParms(
                    "Server.ConsoleManager_zOS.CONSOLE_ERROR.PEGASUS_OS_ZOS",
                    "Console Communication Service failed:"
                        "$0 ( errno $1, reason code 0x$2 ).",
                    strerror(errornumber),
                    errornumber,
                    str_errno2));

            break;
        }

        // Check if we received a stop command from the console
        if (concmd == _CC_modify)
        {
            // Ensure the command we received from the console is
            // null terminated.
            modstr[127] = '\0';

            PEG_TRACE((TRC_SERVER, Tracer::LEVEL4,
                "Received MODIFY command: %s", modstr));

            processModifyCommand(modstr);
        }
        else if (concmd != _CC_stop)
        {
            // Just issue a console message that the command was
            // not recognized and wait again for the stop command.
            Logger::put_l(
                Logger::STANDARD_LOG, System::CIMSERVER, Logger::INFORMATION,
                MessageLoaderParms(
                    "Server.ConsoleManager_zOS.CONSOLE_NO_MODIFY."
                        "PEGASUS_OS_ZOS",
                    "Command not recognized by CIM server."));
        }
        else
        {
            Logger::put_l(
                Logger::STANDARD_LOG, System::CIMSERVER, Logger::INFORMATION,
                MessageLoaderParms(
                    "Server.ConsoleManager_zOS.CONSOLE_STOP.PEGASUS_OS_ZOS",
                    "STOP command received from z/OS console,"
                        " initiating shutdown."));
        }

    // keep on until we encounter an error or received a STOP
    } while ( (concmd != _CC_stop) && (rc == 0) );

    CIMServer::shutdownSignal();

    PEG_METHOD_EXIT();
    pthread_exit(0);

    return NULL;
}


//
// Send notify config change message to provider manager service
// This code was borrowed from the ConfigSettingProvider and should
// be kept in sync.
// The purpose is to ensure that OOP agents also get the update.
// TBD, or is it for other reasons as well?
//
void ZOSConsoleManager::_sendNotifyConfigChangeMessage(
    const String& propertyName,
    const String& newPropertyValue,
    Boolean currentValueModified)
{
    PEG_METHOD_ENTER(TRC_SERVER,
        "ZOSConsoleManager::_sendNotifyConfigChangeMessage");

    ModuleController* controller = ModuleController::getModuleController();

    MessageQueue * queue = MessageQueue::lookup(
        PEGASUS_QUEUENAME_PROVIDERMANAGER_CPP);

    MessageQueueService * service = dynamic_cast<MessageQueueService *>(queue);

    if (service != NULL)
    {
        // create CIMNotifyConfigChangeRequestMessage
        CIMNotifyConfigChangeRequestMessage * notify_req =
            new CIMNotifyConfigChangeRequestMessage (
            XmlWriter::getNextMessageId (),
            propertyName,
            newPropertyValue,
            currentValueModified,
            QueueIdStack(service->getQueueId()));

        notify_req->operationContext.insert(
            IdentityContainer(System::getEffectiveUserName()));

        // create request envelope
        AsyncLegacyOperationStart asyncRequest(
            NULL,
            service->getQueueId(),
            notify_req);

        AutoPtr<AsyncReply> asyncReply(
            controller->ClientSendWait(service->getQueueId(), &asyncRequest));

        AutoPtr<CIMNotifyConfigChangeResponseMessage> response(
            reinterpret_cast<CIMNotifyConfigChangeResponseMessage *>(
            (static_cast<AsyncLegacyOperationResult *>
            (asyncReply.get()))->get_result()));

        if (response->cimException.getCode() != CIM_ERR_SUCCESS)
        {
            CIMException e = response->cimException;
            const CString exMsg = e.getMessage().getCString();
            PEG_TRACE((TRC_SERVER, Tracer::LEVEL1,
                       "Notify config changed failed with rc=%d, message = %s",
                       e.getCode(), 
                       (const char*)exMsg));

            PEG_METHOD_EXIT();
        }
    }
    PEG_METHOD_EXIT();
}



PEGASUS_NAMESPACE_END
