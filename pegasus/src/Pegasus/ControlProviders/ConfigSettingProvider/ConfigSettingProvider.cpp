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


///////////////////////////////////////////////////////////////////////////////
//  ConfigSetting Provider
///////////////////////////////////////////////////////////////////////////////

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/PegasusVersion.h>

#include <cctype>
#include <iostream>

#include "ConfigSettingProvider.h"
#include <Pegasus/Common/String.h>
#include <Pegasus/Common/CIMNameCast.h>
#include <Pegasus/Common/System.h>
#include <Pegasus/Common/ArrayInternal.h>
#include <Pegasus/Common/CIMType.h>
#include <Pegasus/Common/CIMInstance.h>
#include <Pegasus/Common/CIMObjectPath.h>
#include <Pegasus/Common/InternalException.h>
#include <Pegasus/Common/CIMStatusCode.h>
#include <Pegasus/Common/Tracer.h>
#include <Pegasus/Config/ConfigManager.h>
#include <Pegasus/Config/ConfigExceptions.h>

#include <Pegasus/Repository/CIMRepository.h>
#include <Pegasus/Provider/CIMInstanceProvider.h>
#include <Pegasus/Common/ResponseHandler.h>
#include <Pegasus/Common/MessageLoader.h> //l10n
#include <Pegasus/Common/ModuleController.h>
#include <Pegasus/Common/CIMMessage.h>
#include <Pegasus/Common/AutoPtr.h>
#include <Pegasus/Common/AuditLogger.h>


PEGASUS_USING_STD;

PEGASUS_NAMESPACE_BEGIN

/**
    The constants representing the string literals for property
    names of the PG_ConfigSetting class
*/
static const CIMName PROPERTY_NAME    = CIMNameCast("PropertyName");

static const CIMName DEFAULT_VALUE    = CIMNameCast("DefaultValue");

static const CIMName CURRENT_VALUE    = CIMNameCast("CurrentValue");

static const CIMName PLANNED_VALUE    = CIMNameCast("PlannedValue");

static const CIMName DYNAMIC_PROPERTY = CIMNameCast("DynamicProperty");
static const CIMName DESCRIPTION      = CIMNameCast("Description");


/**
    The name of the method that implements the property value update using the
    timeout period.
*/
static const CIMName METHOD_UPDATE_PROPERTY_VALUE  =
    CIMName("UpdatePropertyValue");

/**
    The input parameter names for the UpdatePropertyValue() method.
*/
static const String PARAM_PROPERTYVALUE = String("PropertyValue");
static const String PARAM_RESETVALUE = String("ResetValue");
static const String PARAM_UPDATEPLANNEDVALUE = String("SetPlannedValue");
static const String PARAM_UPDATECURRENTVALUE = String("SetCurrentValue");
static const String PARAM_TIMEOUTPERIOD = String("TimeoutPeriod");

/**
    The constant representing the config setting class name
*/
static const CIMName PG_CONFIG_SETTING  = CIMNameCast("PG_ConfigSetting");

void ConfigSettingProvider::getInstance(
    const OperationContext & context,
    const CIMObjectPath& instanceName,
    const Boolean includeQualifiers,
    const Boolean includeClassOrigin,
    const CIMPropertyList& propertyList,
    InstanceResponseHandler & handler)
    {
        PEG_METHOD_ENTER(TRC_CONFIG, "ConfigSettingProvider::getInstance()");

        Array<String>     propertyInfo;
        CIMKeyBinding        kb;
        String            keyName;
        String            keyValue;

        //
        // check if the class name requested is PG_ConfigSetting
        //
        if (!instanceName.getClassName().equal (PG_CONFIG_SETTING))
        {
            PEG_METHOD_EXIT();
            throw PEGASUS_CIM_EXCEPTION(CIM_ERR_NOT_SUPPORTED,
                instanceName.getClassName().getString());
        }

        //
        // validate key bindings
        //
        Array<CIMKeyBinding> kbArray = instanceName.getKeyBindings();
        if ( (kbArray.size() != 1) ||
             (!kbArray[0].getName().equal (PROPERTY_NAME)))
        {
            PEG_METHOD_EXIT();
            throw PEGASUS_CIM_EXCEPTION_L(
                CIM_ERR_INVALID_PARAMETER,
                MessageLoaderParms(
                    "ControlProviders.ConfigSettingProvider."
                        "ConfigSettingProvider."
                        "INVALID_INSTANCE_NAME",
                    "Invalid instance name"));
        }

        keyValue.assign(kbArray[0].getValue());

        // begin processing the request
        handler.processing();

        //
        // Get values for the property
        //
        try
        {
            _configManager->getPropertyInfo(keyValue, propertyInfo);
        }
        catch (const UnrecognizedConfigProperty&)
        {
            PEG_METHOD_EXIT();
            throw PEGASUS_CIM_EXCEPTION_L(
                CIM_ERR_NOT_FOUND,
                MessageLoaderParms(
                    "ControlProviders.ConfigSettingProvider."
                        "ConfigSettingProvider."
                        "CONFIG_PROPERTY_NOT_FOUND",
                    "Configuration property \"$0\"",
                    keyValue));
        }

        if (propertyInfo.size() >= 5)
        {
            CIMInstance instance(PG_CONFIG_SETTING);

            //
            // construct the instance from Array<String> propertyInfo
            //
            instance.addProperty(CIMProperty(PROPERTY_NAME, propertyInfo[0]));
            instance.addProperty(CIMProperty(DEFAULT_VALUE, propertyInfo[1]));
            instance.addProperty(CIMProperty(CURRENT_VALUE, propertyInfo[2]));
            instance.addProperty(CIMProperty(PLANNED_VALUE, propertyInfo[3]));
            instance.addProperty(CIMProperty(DYNAMIC_PROPERTY,
                Boolean(propertyInfo[4]=="true"?true:false)));
            if (propertyInfo.size() > 6)
            {
                instance.addProperty(CIMProperty(DESCRIPTION,propertyInfo[6]));
            }

            handler.deliver(instance);

            // complete processing the request
            handler.complete();

            PEG_METHOD_EXIT();
            return ;
        }
    }

void ConfigSettingProvider::modifyInstance(
    const OperationContext & context,
    const CIMObjectPath & instanceReference,
    const CIMInstance& modifiedIns,
    const Boolean includeQualifiers,
    const CIMPropertyList& propertyList,
    ResponseHandler & handler)
{
    PEG_METHOD_ENTER(TRC_CONFIG, "ConfigSettingProvider::modifyInstance()");

    handler.processing();

    _modifyInstance(
        context,
        instanceReference,
        modifiedIns,
        propertyList,
        0);

    handler.complete();

    PEG_METHOD_EXIT();
}

void ConfigSettingProvider::_modifyInstance(
    const OperationContext & context,
    const CIMObjectPath & instanceReference,
    const CIMInstance& modifiedIns,
    const CIMPropertyList& propertyList,
    Uint32 timeoutSeconds)
    {
        PEG_METHOD_ENTER(TRC_CONFIG,
            "ConfigSettingProvider::_modifyInstance()");

        //
        // get userName
        //
        String userName;
        try
        {
            IdentityContainer container = context.get(IdentityContainer::NAME);
            userName = container.getUserName();
        }
        catch (...)
        {
            userName = String::EMPTY;
        }

        //
        // verify user authorizations
        // z/OS: authorization check is done in CIMOpReqAuth already
        //
#ifndef PEGASUS_OS_ZOS
        if (userName != String::EMPTY)
        {
            _verifyAuthorization(userName);
        }
#endif
        // NOTE: Qualifiers are not processed by this provider, so the
        // IncludeQualifiers flag is ignored.

        //
        // check if the class name requested is PG_ConfigSetting
        //
        if (!instanceReference.getClassName().equal (PG_CONFIG_SETTING))
        {
            PEG_METHOD_EXIT();
            throw PEGASUS_CIM_EXCEPTION(CIM_ERR_NOT_SUPPORTED,
                instanceReference.getClassName().getString());
        }

        //
        // validate key bindings
        //
        Array<CIMKeyBinding> kbArray = instanceReference.getKeyBindings();
        if ( (kbArray.size() != 1) ||
             (!kbArray[0].getName().equal (PROPERTY_NAME)))
        {
            PEG_METHOD_EXIT();
            throw PEGASUS_CIM_EXCEPTION_L(
                CIM_ERR_INVALID_PARAMETER,
                MessageLoaderParms(
                    "ControlProviders.ConfigSettingProvider."
                        "ConfigSettingProvider."
                        "INVALID_INSTANCE_NAME",
                    "Invalid instance name"));

        }

        String configPropertyName = kbArray[0].getValue();

        // Modification of the entire instance is not supported by this provider
        if (propertyList.isNull())
        {
            PEG_METHOD_EXIT();
            //l10n
            //throw PEGASUS_CIM_EXCEPTION(CIM_ERR_NOT_SUPPORTED,
            //"Modification of entire instance");
            throw PEGASUS_CIM_EXCEPTION_L(CIM_ERR_NOT_SUPPORTED,
                  MessageLoaderParms(
                      "ControlProviders.ConfigSettingProvider."
                          "ConfigSettingProvider."
                          "MODIFICATION_OF_ENTIRE_INSTANCE",
                     "Modification of entire instance"));
        }

        Boolean currentValueModified = false;
        Boolean plannedValueModified = false;

        for (Uint32 i = 0; i < propertyList.size(); ++i)
        {
            CIMName propertyName = propertyList[i];
            if (propertyName.equal (CURRENT_VALUE))
            {
                currentValueModified = true;
            }
            else if (propertyName.equal (PLANNED_VALUE))
            {
                plannedValueModified = true;
            }
            else
            {
                PEG_METHOD_EXIT();

                throw PEGASUS_CIM_EXCEPTION_L(CIM_ERR_NOT_SUPPORTED,
                    MessageLoaderParms(
                        "ControlProviders.ConfigSettingProvider."
                            "ConfigSettingProvider."
                            "MODIFICATION_NOT_SUPPORTED",
                        "Modification of property \"$0\"",
                        propertyName.getString()));
            }
        }

        String preValue;
        String currentValue;
        String plannedValue;
        Boolean currentValueIsNull = false;
        Boolean plannedValueIsNull = false;

        //
        // Get the current value from the instance
        //
        Uint32 pos = modifiedIns.findProperty(CURRENT_VALUE);
        if (pos == PEG_NOT_FOUND)
        {
            currentValueIsNull = true;
        }
        else
        {
            CIMConstProperty prop = modifiedIns.getProperty(pos);
            try
            {
                prop.getValue().get(currentValue);
            }
            catch (Exception& e)
            {
                PEG_METHOD_EXIT();
                throw PEGASUS_CIM_EXCEPTION(CIM_ERR_FAILED, e.getMessage());
            }
        }

        //
        // Get the planned value from the instance
        //
        pos = modifiedIns.findProperty(PLANNED_VALUE);
        if (pos == PEG_NOT_FOUND)
        {
            plannedValueIsNull = true;
        }
        else
        {
            CIMConstProperty prop = modifiedIns.getProperty(pos);
            try
            {
                prop.getValue().get(plannedValue);
            }
            catch (Exception& e)
            {
                PEG_METHOD_EXIT();
                throw PEGASUS_CIM_EXCEPTION(CIM_ERR_FAILED, e.getMessage());
            }
        }

        try
        {
            //
            // Update the current value, if requested
            //
            if (currentValueModified)
            {
                preValue = _configManager->getCurrentValue(configPropertyName);

                if ( !_configManager->updateCurrentValue(
                    configPropertyName,
                    currentValue,
                    userName,
                    timeoutSeconds,
                    currentValueIsNull))
                {
                    PEG_METHOD_EXIT();

                    throw PEGASUS_CIM_EXCEPTION_L(
                        CIM_ERR_FAILED,
                        MessageLoaderParms(
                            "ControlProviders.ConfigSettingProvider."
                                "ConfigSettingProvider."
                                "UPDATE_CURRENT_VALUE_FAILED",
                            "Failed to update the current value."));
                }

                // It is unset, get current value which is default
                if (currentValueIsNull)
                {
                    currentValue = _configManager->getCurrentValue(
                        configPropertyName);
                }

               // send notify config change message to Handler Service
               if(String::equal(configPropertyName,
                      "maxIndicationDeliveryRetryAttempts")||
                  String::equal(configPropertyName,
                      "minIndicationDeliveryRetryInterval"))
               {
                   _sendNotifyConfigChangeMessage(
                       configPropertyName,
                       currentValue,
                       userName,
                       PEGASUS_QUEUENAME_INDHANDLERMANAGER,
                       true);
               }
               // send notify config change message to
               // CIMOperationRequestDispatcher
               if(String::equal(configPropertyName,
                      "pullOperationsMaxObjectCount")||
                  String::equal(configPropertyName,
                      "pullOperationsMaxTimeout")||
                  String::equal(configPropertyName,
                      "pullOperationsDefaultTimeout"))
               {
                   _sendNotifyConfigChangeMessage(
                       configPropertyName,
                       currentValue,
                       userName,
                       PEGASUS_QUEUENAME_OPREQDISPATCHER,
                       true);
               }


                // send notify config change message to ProviderManager Service
                _sendNotifyConfigChangeMessage(
                    configPropertyName,
                    currentValue,
                    userName,
                    PEGASUS_QUEUENAME_PROVIDERMANAGER_CPP,
                    true);

               PEG_AUDIT_LOG(logSetConfigProperty(userName, configPropertyName,
                   preValue, currentValue, false));
            }

            //
            // Update the planned value, if requested
            //
            if (plannedValueModified)
            {
                preValue = _configManager->getPlannedValue(configPropertyName);

                if ( !_configManager->updatePlannedValue(
                    configPropertyName, plannedValue, plannedValueIsNull) )
                {
                    PEG_METHOD_EXIT();

                    throw PEGASUS_CIM_EXCEPTION_L(
                        CIM_ERR_FAILED,
                        MessageLoaderParms(
                            "ControlProviders.ConfigSettingProvider."
                                "ConfigSettingProvider."
                                "UPDATE_PLANNED_VALUE_FAILED",
                            "Failed to update the planned value."));
                }

                // It is unset, get planned value which is default
                if (plannedValueIsNull)
                {
                    plannedValue = _configManager->getPlannedValue(
                        configPropertyName);

                    if (String::equal(configPropertyName,
                           "maxIndicationDeliveryRetryAttempts") ||
                        String::equal(configPropertyName,
                            "minIndicationDeliveryRetryInterval"))
                    {
                        _sendNotifyConfigChangeMessage(
                            configPropertyName,
                            plannedValue,
                            userName,
                            PEGASUS_QUEUENAME_INDHANDLERMANAGER,
                            false);
                    }
                }

                // send notify config change message to ProviderManager Service
                _sendNotifyConfigChangeMessage(
                    configPropertyName,
                    plannedValue,
                    userName,
                    PEGASUS_QUEUENAME_PROVIDERMANAGER_CPP,
                    false);

               PEG_AUDIT_LOG(logSetConfigProperty(userName, configPropertyName,
                   preValue, plannedValue, true));
            }
        }
        catch (const NonDynamicConfigProperty& ndcp)
        {
            PEG_METHOD_EXIT();
            throw PEGASUS_CIM_EXCEPTION(
                CIM_ERR_NOT_SUPPORTED, ndcp.getMessage());
        }
        catch (const InvalidPropertyValue& ipv)
        {
            PEG_METHOD_EXIT();
            throw PEGASUS_CIM_EXCEPTION(
                CIM_ERR_FAILED, ipv.getMessage());
        }
        catch (const UnrecognizedConfigProperty&)
        {
            PEG_METHOD_EXIT();
            throw PEGASUS_CIM_EXCEPTION_L(
                CIM_ERR_NOT_FOUND,
                MessageLoaderParms(
                    "ControlProviders.ConfigSettingProvider."
                        "ConfigSettingProvider."
                        "CONFIG_PROPERTY_NOT_FOUND",
                    "Configuration property \"$0\"",
                    configPropertyName));
        }

        PEG_METHOD_EXIT();
        return;
    }

void ConfigSettingProvider::enumerateInstances(
    const OperationContext & context,
    const CIMObjectPath & ref,
    const Boolean includeQualifiers,
    const Boolean includeClassOrigin,
    const CIMPropertyList& propertyList,
    InstanceResponseHandler & handler)
    {
        PEG_METHOD_ENTER(TRC_CONFIG,
                         "ConfigSettingProvider::enumerateInstances()");

        Array<CIMInstance> instanceArray;
        Array<String> propertyNames;

        //
        // check if the class name requested is PG_ConfigSetting
        //
        if (!ref.getClassName().equal (PG_CONFIG_SETTING))
        {
            PEG_METHOD_EXIT();
            throw PEGASUS_CIM_EXCEPTION(CIM_ERR_NOT_SUPPORTED,
                                        ref.getClassName().getString());
        }

        // begin processing the request
        handler.processing();

        try
        {
            _configManager->getAllPropertyNames(propertyNames, false);


            for (Uint32 i = 0; i < propertyNames.size(); i++)
            {
                Array<String> propertyInfo;

                CIMInstance        instance(PG_CONFIG_SETTING);

                propertyInfo.clear();

                _configManager->getPropertyInfo(
                propertyNames[i], propertyInfo);

                Array<CIMKeyBinding> keyBindings;
                keyBindings.append(CIMKeyBinding(PROPERTY_NAME,
                    propertyInfo[0], CIMKeyBinding::STRING));
                CIMObjectPath instanceName(ref.getHost(),
                    ref.getNameSpace(),
                PG_CONFIG_SETTING, keyBindings);

                // construct the instance
                instance.addProperty(CIMProperty(PROPERTY_NAME,
                                     propertyInfo[0]));
                instance.addProperty(CIMProperty(DEFAULT_VALUE,
                                     propertyInfo[1]));
                instance.addProperty(CIMProperty(CURRENT_VALUE,
                                     propertyInfo[2]));
                instance.addProperty(CIMProperty(PLANNED_VALUE,
                                     propertyInfo[3]));
                instance.addProperty(CIMProperty(DYNAMIC_PROPERTY,
                   Boolean(propertyInfo[4]=="true"?true:false)));
                if (propertyInfo.size() > 6)
                {
                    instance.addProperty(
                        CIMProperty(DESCRIPTION, propertyInfo[6]));
                }

                instance.setPath(instanceName);
                instanceArray.append(instance);
            }
        }
        catch(Exception& e)
        {
            PEG_METHOD_EXIT();
            throw PEGASUS_CIM_EXCEPTION(CIM_ERR_FAILED, e.getMessage());
        }

        handler.deliver(instanceArray);

        // complete processing the request
        handler.complete();

        PEG_METHOD_EXIT();
    }

void ConfigSettingProvider::enumerateInstanceNames(
    const OperationContext & context,
    const CIMObjectPath & classReference,
    ObjectPathResponseHandler & handler)
    {
        PEG_METHOD_ENTER(TRC_CONFIG,
            "ConfigSettingProvider::enumerateInstanceNames()");

        Array<CIMObjectPath> instanceRefs;
        Array<String>       propertyNames;
        Array<CIMKeyBinding>   keyBindings;
        CIMKeyBinding          kb;
        String              hostName;

        hostName.assign(System::getHostName());

        const CIMName& className = classReference.getClassName();
        const CIMNamespaceName& nameSpace = classReference.getNameSpace();

        if (!className.equal (PG_CONFIG_SETTING))
        {
            PEG_METHOD_EXIT();
            throw PEGASUS_CIM_EXCEPTION( CIM_ERR_NOT_SUPPORTED,
                className.getString());
        }

        // begin processing the request
        handler.processing();

        try
        {
            _configManager->getAllPropertyNames(propertyNames, false);

            Uint32 size = propertyNames.size();

            for (Uint32 i = 0; i < size; i++)
            {
                 keyBindings.append(
                              CIMKeyBinding(PROPERTY_NAME, propertyNames[i],
                              CIMKeyBinding::STRING));

                 //
                 // Convert instance names to References
                 //
                 CIMObjectPath ref(hostName, nameSpace, className,
                                    keyBindings);

                 instanceRefs.append(ref);
                 keyBindings.clear();

            }
        }
        catch(Exception& e)
        {
            PEG_METHOD_EXIT();
            throw PEGASUS_CIM_EXCEPTION(CIM_ERR_FAILED, e.getMessage());
        }

        handler.deliver(instanceRefs);

        // complete processing the request
        handler.complete();

        PEG_METHOD_EXIT();
    }

void ConfigSettingProvider::invokeMethod(
    const OperationContext & context,
    const CIMObjectPath & cimObjectPath,
    const CIMName & methodName,
    const Array<CIMParamValue> & inParams,
    MethodResultResponseHandler & handler)
{
    PEG_METHOD_ENTER(TRC_CONFIG,
        "ConfigSettingProvider::invokeMethod");

    if (!methodName.equal(METHOD_UPDATE_PROPERTY_VALUE))
    {
        throw CIMException(
            CIM_ERR_METHOD_NOT_FOUND, methodName.getString());
    }

    // Prepare the instance for modification.

    String propValue;
    Boolean resetValue = false;
    Boolean currentValueSet = false;
    Boolean plannedValueSet = false;
    Uint32 timeoutSeconds = 0;
    Array<CIMName> propertyList;

    for (Uint32 i = 0, n = inParams.size(); i < n ; ++i)
    {
        CIMName name = inParams[i].getParameterName();
        if (name.equal(PARAM_PROPERTYVALUE))
        {
            inParams[i].getValue().get(propValue);
        }
        else if (name.equal(PARAM_RESETVALUE))
        {
            inParams[i].getValue().get(resetValue);
        }
        else if (name.equal(PARAM_UPDATECURRENTVALUE))
        {
            inParams[i].getValue().get(currentValueSet);
        }
        else if (name.equal(PARAM_UPDATEPLANNEDVALUE))
        {
            inParams[i].getValue().get(plannedValueSet);
        }
        else if (name.equal(PARAM_TIMEOUTPERIOD))
        {
            inParams[i].getValue().get(timeoutSeconds);
        }
        else
        {
            throw CIMException(
                CIM_ERR_INVALID_PARAMETER, name.getString());
        }
    }

    CIMInstance modifiedInst(PG_CONFIG_SETTING);

    if (currentValueSet)
    {
        if (!resetValue)
        {
            CIMProperty prop =
                CIMProperty(CURRENT_VALUE, CIMValue(propValue));
            modifiedInst.addProperty(prop);
        }
        propertyList.append(CURRENT_VALUE);
    }

    if (plannedValueSet)
    {
        if (!resetValue)
        {
            CIMProperty prop =
                CIMProperty(PLANNED_VALUE, CIMValue(propValue));
            modifiedInst.addProperty(prop);
        }
        propertyList.append(PLANNED_VALUE);
    }

    handler.processing();

    _modifyInstance(
        context,
        cimObjectPath,
        modifiedInst,
        propertyList,
        timeoutSeconds);

    handler.deliver(CIMValue(true));
    handler.complete();

    PEG_METHOD_EXIT();
}


//
// Verify user authorization
//
void ConfigSettingProvider::_verifyAuthorization(const String& userName)
    {
        PEG_METHOD_ENTER(TRC_CONFIG,
            "ConfigSettingProvider::_verifyAuthorization()");

        if ( System::isPrivilegedUser(userName) == false )
        {
            PEG_METHOD_EXIT();
            throw PEGASUS_CIM_EXCEPTION_L(CIM_ERR_ACCESS_DENIED,
                MessageLoaderParms(
                    "ControlProviders.ConfigSettingProvider."
                        "ConfigSettingProvider."
                        "USER_NOT_PRIVILEGED",
                    "Must be a privileged user to do this CIM operation."));
        }

        PEG_METHOD_EXIT();
    }

//
// send notify config change message to provider manager service
//
void ConfigSettingProvider::_sendNotifyConfigChangeMessage(
    const String& propertyName,
    const String& newPropertyValue,
    const String& userName,
    const char *queueName,
    Boolean currentValueModified)
{
    PEG_METHOD_ENTER(TRC_CONFIG,
        "ConfigSettingProvider::_sendNotifyConfigChangeMessage");

    ModuleController* controller = ModuleController::getModuleController();

    MessageQueue * queue = MessageQueue::lookup(queueName);

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
            IdentityContainer(userName));

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
            throw (e);
        }
    }
}

PEGASUS_NAMESPACE_END

