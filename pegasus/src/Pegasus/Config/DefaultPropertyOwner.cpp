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


///////////////////////////////////////////////////////////////////////////////
//
// This file has implementation for the default property owner class.
//
///////////////////////////////////////////////////////////////////////////////

#include "DefaultPropertyOwner.h"
#include "ConfigManager.h"
#include <Pegasus/Common/AuditLogger.h>
#include <Pegasus/Common/StringConversion.h>
#include <Pegasus/Common/HTTPAcceptor.h>
#include <Pegasus/Common/HTTPConnection.h>
#include <Pegasus/Common/CIMMessage.h>
#include <Pegasus/Common/CIMNameCast.h>
#include <Pegasus/Common/HostAddress.h>
#include "ConfigExceptions.h"


PEGASUS_USING_STD;

PEGASUS_NAMESPACE_BEGIN


///////////////////////////////////////////////////////////////////////////////
//  DefaultPropertyOwner
//
//  When a new property is added with the default owner, make sure to add
//  the property name and the default attributes of that property in
//  the table below.
///////////////////////////////////////////////////////////////////////////////

static struct ConfigPropertyRow properties[] =
{
#include "DefaultPropertyTable.h"
};

const Uint32 NUM_PROPERTIES = sizeof(properties) / sizeof(properties[0]);


/** Constructors  */
DefaultPropertyOwner::DefaultPropertyOwner()
{
    _configProperties.reset(new ConfigProperty[NUM_PROPERTIES]);
}


/**
    Initialize the config properties.
*/
void DefaultPropertyOwner::initialize()
{
    for (Uint32 i = 0; i < NUM_PROPERTIES; i++)
    {
        (_configProperties.get())[i].propertyName = properties[i].propertyName;
        (_configProperties.get())[i].defaultValue = properties[i].defaultValue;
        (_configProperties.get())[i].currentValue = properties[i].defaultValue;
        (_configProperties.get())[i].plannedValue = properties[i].defaultValue;
        (_configProperties.get())[i].dynamic = properties[i].dynamic;
        (_configProperties.get())[i].externallyVisible =
            properties[i].externallyVisible;
    }
}


/**
    Get information about the specified property.
*/
void DefaultPropertyOwner::getPropertyInfo(
    const String& name,
    Array<String>& propertyInfo) const
{
    propertyInfo.clear();

    for (Uint32 i = 0; i < NUM_PROPERTIES; i++)
    {
        if (String::equal(_configProperties.get()[i].propertyName, name))
        {
            propertyInfo.append(_configProperties.get()[i].propertyName);
            propertyInfo.append(_configProperties.get()[i].defaultValue);
            propertyInfo.append(_configProperties.get()[i].currentValue);
            propertyInfo.append(_configProperties.get()[i].plannedValue);

            if (_configProperties.get()[i].dynamic)
            {
                propertyInfo.append(STRING_TRUE);
            }
            else
            {
                propertyInfo.append(STRING_FALSE);
            }

            if (_configProperties.get()[i].externallyVisible)
            {
                propertyInfo.append(STRING_TRUE);
            }
            else
            {
                propertyInfo.append(STRING_FALSE);
            }

            propertyInfo.append(getPropertyHelp(name));

            return;
        }
    }

    //
    // specified property name is not found
    //
    throw UnrecognizedConfigProperty(name);
}

/**
    Get default value of the specified property
*/
String DefaultPropertyOwner::getDefaultValue(const String& name) const
{
    for (Uint32 i = 0; i < NUM_PROPERTIES; i++)
    {
        if (String::equal(_configProperties.get()[i].propertyName, name))
        {
            return _configProperties.get()[i].defaultValue;
        }
    }

    //
    // Specified property name could not be found
    //
    throw UnrecognizedConfigProperty(name);
}

/**
    Get current value of the specified property
*/
String DefaultPropertyOwner::getCurrentValue(const String& name) const
{
    for (Uint32 i = 0; i < NUM_PROPERTIES; i++)
    {
        if (String::equal(_configProperties.get()[i].propertyName, name))
        {
            if (String::equalNoCase(name, "hostname"))
            {
                if (0 == _configProperties.get()[i].currentValue.size())
                {
                    _configProperties.get()[i].currentValue=
                        System::getHostName();
                }
            }
            if (String::equalNoCase(name, "fullyQualifiedHostName"))
            {
                if (0 == _configProperties.get()[i].currentValue.size())
                {
                    _configProperties.get()[i].currentValue=
                        System::getFullyQualifiedHostName();
                }
            }
            if (String::equalNoCase(name, "maxProviderProcesses") ||
                String::equalNoCase(name, "maxFailedProviderModuleRestarts"))
            {
                AutoMutex lock(_dynamicConfigPropertyMutex);
                return _configProperties.get()[i].currentValue;
            }
            else
            {
                return _configProperties.get()[i].currentValue;
            }
        }
    }

    //
    // Specified property name could not be found
    //
    throw UnrecognizedConfigProperty(name);
}

/**
    Get planned value of the specified property
*/
String DefaultPropertyOwner::getPlannedValue(const String& name) const
{
    for (Uint32 i = 0; i < NUM_PROPERTIES; i++)
    {
        if (String::equal(_configProperties.get()[i].propertyName, name))
        {
            return _configProperties.get()[i].plannedValue;
        }
    }

    //
    // Specified property name could not be found
    //
    throw UnrecognizedConfigProperty(name);
}

/**
    Init current value of the specified property to the specified value
*/
void DefaultPropertyOwner::initCurrentValue(
    const String& name,
    const String& value)
{
    Uint32 index;
    for (index = 0; index < NUM_PROPERTIES; index++)
    {
        if (String::equal(
            _configProperties.get()[index].propertyName,
            name))
        {
            if (String::equalNoCase(name, "maxProviderProcesses") ||
                String::equalNoCase(name, "maxFailedProviderModuleRestarts"))
            {
                AutoMutex lock(_dynamicConfigPropertyMutex);
                _configProperties.get()[index].currentValue = value;
            }
            else if (String::equalNoCase(name, "hostname"))
            {
                if (0 == value.size())
                {
                    _configProperties.get()[index].currentValue=
                        System::getHostName();
                }
                else
                {
                    System::setHostName(value);
                    _configProperties.get()[index].currentValue=value;
                }

            }
            else if (String::equalNoCase(name, "fullyQualifiedHostName"))
            {
                if (0 == value.size())
                {
                    _configProperties.get()[index].currentValue=
                        System::getFullyQualifiedHostName();
                }
                else
                {
                    System::setFullyQualifiedHostName(value);
                    _configProperties.get()[index].currentValue=value;
                }
            }
            else
            {
               _configProperties.get()[index].currentValue = value;
            }
            break;
        }
    }

    if (index >= NUM_PROPERTIES)
    {
        //
        // Specified property name could not be found
        //
        throw UnrecognizedConfigProperty(name);
    }
    else if (String::equal(name, "idleConnectionTimeout"))
    {
        Uint64 v;
        StringConversion::decimalStringToUint64(value.getCString(), v);
        HTTPConnection::setIdleConnectionTimeout((Uint32)v);
    }
    else if (String::equal(name, "socketWriteTimeout"))
    {
        Uint64 v;
        StringConversion::decimalStringToUint64(value.getCString(), v);
        HTTPAcceptor::setSocketWriteTimeout((Uint32)v);
    }
    return;
}

/**
    Init planned value of the specified property to the specified value
*/
void DefaultPropertyOwner::initPlannedValue(
    const String& name,
    const String& value)
{
    for (Uint32 i = 0; i < NUM_PROPERTIES; i++)
    {
        if (String::equal(_configProperties.get()[i].propertyName, name))
        {
            _configProperties.get()[i].plannedValue = value;
            return;
        }
    }

    //
    // Specified property name could not be found
    //
    throw UnrecognizedConfigProperty(name);
}

/**
    Update current value of the specified property to the specified value
*/
void DefaultPropertyOwner::updateCurrentValue(
    const String& name,
    const String& value,
    const String& userName,
    Uint32 timeoutSeconds)
{
    //
    // make sure the property is dynamic before updating the value.
    //
    if (!isDynamic(name))
    {
        throw NonDynamicConfigProperty(name);
    }

#ifdef PEGASUS_ENABLE_DMTF_INDICATION_PROFILE_SUPPORT
    if (String::equal(name, "enableIndicationService"))
    {
        ConfigProperty *configProperty = 0;
        for (Uint32 i = 0; i < NUM_PROPERTIES; i++)
        {
            if (String::equal(_configProperties.get()[i].propertyName, name))
            {
                configProperty = &_configProperties.get()[i];
                break;
            }
        }
        PEGASUS_ASSERT(configProperty);
        _requestIndicationServiceStateChange(
            userName,
            ConfigManager::parseBooleanValue(value),
            timeoutSeconds);
        configProperty->currentValue = value;
        return;
    }
#endif

    //
    // Since the validations done in initCurrrentValue are sufficient and
    // no additional validations required for update, we shall call
    // initCurrrentValue.
    //
    initCurrentValue(name, value);

#ifdef PEGASUS_ENABLE_AUDIT_LOGGER

    if (String::equal(name, "enableAuditLog") && isValid(name, value))
    {
        Boolean enableAuditLog = ConfigManager::parseBooleanValue(value);
        AuditLogger::setEnabled(enableAuditLog);
    }

#endif

}


/**
    Update planned value of the specified property to the specified value
*/
void DefaultPropertyOwner::updatePlannedValue(
    const String& name,
    const String& value)
{
    //
    // Since the validations done in initPlannedValue are sufficient and
    // no additional validations required for update, we shall call
    // initPlannedValue.
    //
    initPlannedValue(name, value);
}

/**
 *Parse the list of comma seperated interface addresses
 * and return a list of string representation of interfaces
 * and works in following way
 *1)It checks for comma in a specified non empty listenAddress value
 *   a)if not found, it is assumed as single interface is specified
 *2) Otherwise, iterate to find comma and append
 */
Array<String> DefaultPropertyOwner::parseAndGetListenAddress
    (const String &value_)
{
    PEGASUS_ASSERT(value_.size() != 0 );
    String value = value_;
    Array<String> interfaces;
    try
    {
        Uint32 idx = value.find(",");
        interfaces.clear();

        //it has one ip address only
        if( idx == PEG_NOT_FOUND)
        {
            interfaces.append(value);
        }
        else // has multiple address
        {
            while(idx !=PEG_NOT_FOUND)
            {
                interfaces.append(value.subString(0,idx));
                value.remove(0,idx+1);
                idx = value.find(",");
            }
            //Last Remaining address
            PEGASUS_ASSERT (idx == PEG_NOT_FOUND);
            interfaces.append(value);
            value.remove(0);
        }
    }
    catch( Exception &e)
    {
        PEG_TRACE((TRC_CONFIG,Tracer::LEVEL1,
            "Exception caught while parsing listenAddresses %s",
        (const char*)e.getMessage().getCString()));
        throw;
    }
    return interfaces;
}

static Boolean isListenAddressValid(const String value_)
{
    if(value_.size() == 0 )
    {
        return false;
    }

    Boolean isIpListTrue = true;
    Array<String> interfaces = DefaultPropertyOwner::parseAndGetListenAddress(
                                   value_);

    HostAddress theAddress;
    for(Uint32 i = 0, m = interfaces.size(); i < m; ++i)
    {
        if(!theAddress.setHostAddress(interfaces[i]))
        {
            isIpListTrue = false;
            throw InvalidListenAddressPropertyValue(
                "listenAddress", interfaces[i]);
            break;
        }
    }

    return isIpListTrue;
}


/**
Checks to see if the given value is valid or not.
*/
Boolean DefaultPropertyOwner::isValid(
    const String& name,
    const String& value) const
{
    //
    // By default, no validation is done. It can optionally be added here
    // per property.
    //
    if (String::equal(name, "socketWriteTimeout"))
    {
        Uint64 v;
        return
            StringConversion::decimalStringToUint64(value.getCString(), v) &&
            StringConversion::checkUintBounds(v, CIMTYPE_UINT32) &&
            (v != 0);
    }
#ifdef PEGASUS_ENABLE_SLP
    if (String::equal(name, "slpProviderStartupTimeout"))
    {
        Uint64 v;
        return
            StringConversion::decimalStringToUint64(value.getCString(), v) &&
            StringConversion::checkUintBounds(v, CIMTYPE_UINT32) &&
            (v != 0);
    }
#endif
    if (String::equal(name, "maxProviderProcesses") ||
        String::equal(name, "idleConnectionTimeout") ||
        String::equal(name, "maxFailedProviderModuleRestarts"))
    {
        Uint64 v;
        return
            StringConversion::decimalStringToUint64(value.getCString(), v) &&
            StringConversion::checkUintBounds(v, CIMTYPE_UINT32);
    }
    else if (String::equal(name, "httpsPort") ||
            String::equal(name, "httpPort"))
    {
        Uint64 v;
        return StringConversion::decimalStringToUint64(value.getCString(), v) &&
                StringConversion::checkUintBounds(v, CIMTYPE_UINT16) && (v!=0);
    }
    else if (String::equal(name, "listenAddress"))
    {

        return isListenAddressValid(value);

    }
    else if (String::equal(name, "enableHttpConnection") ||
        String::equal(name, "enableHttpsConnection") ||
        String::equal(name, "daemon") ||
        String::equal(name, "enableAssociationTraversal") ||
        String::equal(name, "enableIndicationService") ||
        String::equal(name, "forceProviderProcesses")
#ifdef PEGASUS_ENABLE_SLP
        || String::equal(name, "slp")
#endif
#ifdef PEGASUS_ENABLE_AUDIT_LOGGER
        || String::equal(name, "enableAuditLog")
#endif
        )
    {
        return ConfigManager::isValidBooleanValue(value);
    }

    else if (String::equal(name, "hostname") ||
        String::equal(name, "fullyQualifiedHostName"))
    {
        return HostAddress::isValidHostName(value);
    }
    // If you change the following values, please also change
    // the help file which displays the min and max and possibly
    // the documentation.
    else if (String::equal(name, "pullOperationsMaxObjectCount"))
    {
        return ConfigManager::isValidUint32Value(value,
            (Uint32)1,
            (Uint32)PEGASUS_PULL_OPERATION_MAX_OBJECT_COUNT);
    }
    // If you change the following values fixed values, please also change
    // the help file which displays the min and max
    else if (String::equal(name, "pullOperationsMaxTimeout"))
    {
        return(ConfigManager::isValidUint32Value(value,
            (Uint32)1,
            (Uint32)PEGASUS_PULL_OPERATION_MAX_TIMEOUT_SEC));
    }
    // If you change the following values, please also change
    // the help file whcih displays the min and max
     else if (String::equal(name, "pullOperationsDefaultTimeout"))
    {
        return ConfigManager::isValidUint32Value(value,
            (Uint32)1,
            (Uint32)PEGASUS_DEFAULT_PULL_OPERATION_TIMEOUT_SEC);
    }
    return true;
}

/**
    Checks to see if the specified property is dynamic or not.
*/
Boolean DefaultPropertyOwner::isDynamic(const String& name) const
{
    for (Uint32 i = 0; i < NUM_PROPERTIES; i++)
    {
        if (String::equal(_configProperties.get()[i].propertyName, name))
        {
            return (_configProperties.get()[i].dynamic == IS_DYNAMIC);
        }
    }

    //
    // Specified property name could not be found
    //
    throw UnrecognizedConfigProperty(name);
}

#ifdef PEGASUS_ENABLE_DMTF_INDICATION_PROFILE_SUPPORT
void DefaultPropertyOwner::_requestIndicationServiceStateChange(
    const String& userName,
    Boolean enable,
    Uint32 timeoutSeconds)
{
    MessageQueue *queue = MessageQueue::lookup(
        PEGASUS_QUEUENAME_INDICATIONSERVICE);
    // Return if indication service can not be found
    if (!queue)
    {
        return;
    }

    Uint32 queueId = queue->getQueueId();

    const String METHOD_NAME = "RequestStateChange";
    const String PARAMNAME_REQUESTEDSTATE = "RequestedState";
    const String PARAMNAME_TIMEOUTPERIOD = "TimeoutPeriod";
    const Uint16 STATE_ENABLED = 2;
    const Uint16 STATE_DISABLED = 3;

    String referenceStr("//", 2);
    referenceStr.append(System::getHostName());
    referenceStr.append("/");
    referenceStr.append(PEGASUS_NAMESPACENAME_INTEROP.getString());
    referenceStr.append(":");
    referenceStr.append(
        PEGASUS_CLASSNAME_CIM_INDICATIONSERVICE.getString());
    CIMObjectPath reference(referenceStr);

    Array<CIMParamValue> inParams;
    Array<CIMParamValue> outParams;

    inParams.append(CIMParamValue(PARAMNAME_REQUESTEDSTATE,
        CIMValue(enable ? STATE_ENABLED : STATE_DISABLED)));

    inParams.append(CIMParamValue(PARAMNAME_TIMEOUTPERIOD,
        CIMValue(CIMDateTime(timeoutSeconds * 1000000, true))));

    MessageQueueService *controller = ModuleController::getModuleController();

    try
    {
        CIMInvokeMethodRequestMessage* request =
            new CIMInvokeMethodRequestMessage(
                XmlWriter::getNextMessageId(),
                PEGASUS_NAMESPACENAME_INTEROP,
                referenceStr,
                CIMNameCast(METHOD_NAME),
                inParams,
                QueueIdStack(queueId));

        request->operationContext.insert(
            IdentityContainer(userName));

        AsyncLegacyOperationStart *asyncRequest =
            new AsyncLegacyOperationStart(
                0,
                queueId,
                request);

        AsyncReply * asyncReply = controller->SendWait(asyncRequest);

        CIMInvokeMethodResponseMessage * response =
            reinterpret_cast<CIMInvokeMethodResponseMessage *>(
                (static_cast<AsyncLegacyOperationResult *>(
                    asyncReply))->get_result());

        CIMException e = response->cimException;

        delete response;
        delete asyncRequest;
        delete asyncReply;

        if (e.getCode() != CIM_ERR_SUCCESS)
        {
            throw e;
        }
    }
    catch(const Exception &e)
    {
        PEG_TRACE((TRC_CONFIG,Tracer::LEVEL1,
            "Exception caught while invoking CIM_IndicationService."
                "RequestStateChange()  method: %s",
        (const char*)e.getMessage().getCString()));
        throw;
    }
    catch(...)
    {
        PEG_TRACE_CSTRING(TRC_CONFIG,Tracer::LEVEL1,
            "Unknown exception caught while invoking CIM_IndicationService."
                "RequestStateChange()  method");
        throw;
    }
}
#endif

PEGASUS_NAMESPACE_END
