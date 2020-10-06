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
// This file has implementation for the log property owner class.
//
///////////////////////////////////////////////////////////////////////////////

#include "LogPropertyOwner.h"
#include <Pegasus/Common/Logger.h>
#include <Pegasus/Common/Constants.h>
#include <Pegasus/Common/StringConversion.h>
#include "ConfigExceptions.h"

PEGASUS_USING_STD;

PEGASUS_NAMESPACE_BEGIN

///////////////////////////////////////////////////////////////////////////////
//  LogPropertyOwner
///////////////////////////////////////////////////////////////////////////////

static struct ConfigPropertyRow properties[] =
{
#if defined(PEGASUS_OS_ZOS) && defined(PEGASUS_USE_RELEASE_DIRS)
# if !defined(PEGASUS_USE_SYSLOGS)
    {"logdir", "/var/wbem/logs", IS_DYNAMIC, IS_HIDDEN},
    {"maxLogFileSizeKBytes", "32678", IS_DYNAMIC, IS_VISIBLE},
# endif
    {"logLevel", "INFORMATION", IS_DYNAMIC, IS_VISIBLE}
#elif defined(PEGASUS_OS_PASE)
# if defined(PEGASUS_USE_RELEASE_CONFIG_OPTIONS)
        {"logdir", "/QOpenSys/QIBM/UserData/UME/Pegasus/logs", IS_DYNAMIC,
            IS_VISIBLE},
        {"maxLogFileSizeKBytes", "32678", IS_DYNAMIC, IS_VISIBLE},
# else
        {"logdir", "./logs", IS_DYNAMIC, IS_VISIBLE},
        {"maxLogFileSizeKBytes", "32678", IS_DYNAMIC, IS_VISIBLE},
# endif
    {"logLevel", "INFORMATION", IS_DYNAMIC, IS_VISIBLE}
#else
# if defined(PEGASUS_USE_RELEASE_CONFIG_OPTIONS)
#  if !defined(PEGASUS_USE_SYSLOGS)
    {"logdir", "./logs", IS_DYNAMIC, IS_HIDDEN},
    {"maxLogFileSizeKBytes", "32678", IS_DYNAMIC, IS_VISIBLE},
#  endif
    {"logLevel", "SEVERE", IS_DYNAMIC, IS_HIDDEN}
# else
#  if !defined(PEGASUS_USE_SYSLOGS)
    {"logdir", "./logs", IS_DYNAMIC, IS_VISIBLE},
    {"maxLogFileSizeKBytes", "32678", IS_DYNAMIC, IS_VISIBLE},
#  endif
    {"logLevel", "INFORMATION", IS_DYNAMIC, IS_VISIBLE}
# endif
#endif
};

const Uint32 NUM_PROPERTIES = sizeof(properties) / sizeof(properties[0]);


/** Constructors  */
LogPropertyOwner::LogPropertyOwner()
{
#if !defined(PEGASUS_USE_SYSLOGS)
    _logdir.reset(new ConfigProperty);
    _maxLogFileSizeKBytes.reset(new ConfigProperty);
#endif
    _logLevel.reset(new ConfigProperty);
}


/**
    Initialize the config properties.
*/
void LogPropertyOwner::initialize()
{
    for (Uint32 i = 0; i < NUM_PROPERTIES; i++)
    {
        //
        // Initialize the properties with default values
        //
#if !defined (PEGASUS_USE_SYSLOGS)
        if (String::equal(properties[i].propertyName, "logdir"))
        {
            _logdir->propertyName = properties[i].propertyName;
            _logdir->defaultValue = properties[i].defaultValue;
            _logdir->currentValue = properties[i].defaultValue;
            _logdir->plannedValue = properties[i].defaultValue;
            _logdir->dynamic = properties[i].dynamic;
            _logdir->externallyVisible = properties[i].externallyVisible;
        }
        else
        if (String::equal(
                properties[i].propertyName, "maxLogFileSizeKBytes"))
        {
            _maxLogFileSizeKBytes->propertyName = properties[i].propertyName;
            _maxLogFileSizeKBytes->defaultValue = properties[i].defaultValue;
            _maxLogFileSizeKBytes->currentValue = properties[i].defaultValue;
            _maxLogFileSizeKBytes->plannedValue = properties[i].defaultValue;
            _maxLogFileSizeKBytes->dynamic = properties[i].dynamic;
            _maxLogFileSizeKBytes->externallyVisible =
                properties[i].externallyVisible;

            Uint64 logFileSize;
            StringConversion::decimalStringToUint64(
                _maxLogFileSizeKBytes->currentValue.getCString(),
                logFileSize);

            Logger::setMaxLogFileSize((Uint32)(logFileSize *1024));
        }
        else
#endif
        if (String::equal(properties[i].propertyName, "logLevel"))
        {
            _logLevel->propertyName = properties[i].propertyName;
            _logLevel->defaultValue = properties[i].defaultValue;
            _logLevel->currentValue = properties[i].defaultValue;
            _logLevel->plannedValue = properties[i].defaultValue;
            _logLevel->dynamic = properties[i].dynamic;
            _logLevel->externallyVisible = properties[i].externallyVisible;

            Logger::setlogLevelMask(_logLevel->currentValue);
        }
    }
}

struct ConfigProperty* LogPropertyOwner::_lookupConfigProperty(
    const String& name) const
{
#if !defined(PEGASUS_USE_SYSLOGS)
    if (String::equal(_logdir->propertyName, name))
    {
        return _logdir.get();
    }
    else
    if (String::equal(_maxLogFileSizeKBytes->propertyName, name))
    {
        return _maxLogFileSizeKBytes.get();
    }
    else
#endif
    if (String::equal(_logLevel->propertyName, name))
    {
        return _logLevel.get();
    }
    else
    {
        throw UnrecognizedConfigProperty(name);
    }
}

/**
    Get information about the specified property.
*/
void LogPropertyOwner::getPropertyInfo(
    const String& name,
    Array<String>& propertyInfo) const
{
    struct ConfigProperty* configProperty = _lookupConfigProperty(name);

    buildPropertyInfo(name, configProperty, propertyInfo);
}

/**
    Get default value of the specified property.
*/
String LogPropertyOwner::getDefaultValue(const String& name) const
{
    struct ConfigProperty* configProperty = _lookupConfigProperty(name);

    return configProperty->defaultValue;
}

/**
    Get current value of the specified property.
*/
String LogPropertyOwner::getCurrentValue(const String& name) const
{
    struct ConfigProperty* configProperty = _lookupConfigProperty(name);

    return configProperty->currentValue;
}

/**
    Get planned value of the specified property.
*/
String LogPropertyOwner::getPlannedValue(const String& name) const
{
    struct ConfigProperty* configProperty = _lookupConfigProperty(name);

    return configProperty->plannedValue;
}


/**
    Init current value of the specified property to the specified value.
*/
void LogPropertyOwner::initCurrentValue(
    const String& name,
    const String& value)
{
    if (String::equal(_logLevel->propertyName,name))
    {
        _logLevel->currentValue = value;
        Logger::setlogLevelMask(_logLevel->currentValue);
    }
    else
#if !defined(PEGASUS_USE_SYSLOGS)
    if (String::equal(_maxLogFileSizeKBytes->propertyName,name))
    {
        Boolean status = false;
        Uint64 maxLogFileSizeKBytes=0;

        status = StringConversion::decimalStringToUint64(
            value.getCString(),
            maxLogFileSizeKBytes);

        if (!status||(maxLogFileSizeKBytes <
            PEGASUS_MAXLOGFILESIZEKBYTES_CONFIG_PROPERTY_MINIMUM_VALUE))
        {
            throw InvalidPropertyValue(name, value);
        }
        _maxLogFileSizeKBytes->currentValue = value;

        Logger::setMaxLogFileSize((Uint32)(maxLogFileSizeKBytes*1024));
    }
    else
#endif
    {
        struct ConfigProperty* configProperty = _lookupConfigProperty(name);
        configProperty->currentValue = value;
    }
}


/**
    Init planned value of the specified property to the specified value.
*/
void LogPropertyOwner::initPlannedValue(
    const String& name,
    const String& value)
{
    struct ConfigProperty* configProperty = _lookupConfigProperty(name);
    configProperty->plannedValue = value;
}

/**
    Update current value of the specified property to the specified value.
*/
void LogPropertyOwner::updateCurrentValue(
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

    //
    // Since the validations done in initCurrrentValue are sufficient and
    // no additional validations required for update, we will call
    // initCurrrentValue.
    //
    initCurrentValue(name, value);
}


/**
    Update planned value of the specified property to the specified value.
*/
void LogPropertyOwner::updatePlannedValue(
    const String& name,
    const String& value)
{
    //
    // Since the validations done in initPlannedValue are sufficient and
    // no additional validations required for update, we will call
    // initPlannedValue.
    //
    initPlannedValue(name, value);
}

/**
    Checks to see if the given value is valid or not.
*/
Boolean LogPropertyOwner::isValid(
    const String& name,
    const String& value) const
{
    if (String::equal(_logLevel->propertyName, name))
    {
        //
        // Check if the logLevel is valid
        //
        if (!Logger::isValidlogLevel(value))
        {
            throw InvalidPropertyValue(name, value);
        }
    }

    return true;
}

/**
    Checks to see if the specified property is dynamic or not.
*/
Boolean LogPropertyOwner::isDynamic(const String& name) const
{
    struct ConfigProperty* configProperty = _lookupConfigProperty(name);

    return (configProperty->dynamic == IS_DYNAMIC);
}

PEGASUS_NAMESPACE_END
