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

#include <Pegasus/Common/MessageLoader.h>
#include <Pegasus/Common/StringConversion.h>
#include "ConfigManager.h"
#include "IndicationServicePropertyOwner.h"
#include "ConfigExceptions.h"


PEGASUS_USING_STD;

PEGASUS_NAMESPACE_BEGIN

static struct ConfigPropertyRow properties[] =
{
    {"maxIndicationDeliveryRetryAttempts", "3", IS_DYNAMIC, IS_VISIBLE},
    {"minIndicationDeliveryRetryInterval", "30",IS_DYNAMIC, IS_VISIBLE},
};

const Uint32 NUM_PROPERTIES = sizeof(properties) / sizeof(properties[0]);

IndicationServicePropertyOwner::IndicationServicePropertyOwner():
   _initialized(false)
{
    _maxIndicationDeliveryRetryAttempts.reset(new ConfigProperty);
    _minIndicationDeliveryRetryInterval.reset(new ConfigProperty);
}

/**
    Initialize the config properties.
*/
void IndicationServicePropertyOwner::initialize()
{
    if (_initialized)
    {
        return;
    }

    for (Uint32 i = 0; i < NUM_PROPERTIES; i++)
    {
        if (String::equal(
            properties[i].propertyName, "maxIndicationDeliveryRetryAttempts"))
        {
            _maxIndicationDeliveryRetryAttempts->propertyName
                = properties[i].propertyName;
            _maxIndicationDeliveryRetryAttempts->defaultValue
                = properties[i].defaultValue;
            _maxIndicationDeliveryRetryAttempts->currentValue
                = properties[i].defaultValue;
            _maxIndicationDeliveryRetryAttempts->plannedValue
                = properties[i].defaultValue;
            _maxIndicationDeliveryRetryAttempts->dynamic
                = properties[i].dynamic;
            _maxIndicationDeliveryRetryAttempts->externallyVisible =
                properties[i].externallyVisible;
        }
        else if (String::equal(
            properties[i].propertyName, "minIndicationDeliveryRetryInterval"))
        {
            _minIndicationDeliveryRetryInterval->propertyName
                = properties[i].propertyName;
            _minIndicationDeliveryRetryInterval->defaultValue
                = properties[i].defaultValue;
            _minIndicationDeliveryRetryInterval->currentValue
                = properties[i].defaultValue;
            _minIndicationDeliveryRetryInterval->plannedValue
                = properties[i].defaultValue;
            _minIndicationDeliveryRetryInterval->dynamic
                = properties[i].dynamic;
            _minIndicationDeliveryRetryInterval->externallyVisible =
                properties[i].externallyVisible;
        }
        else
        {
            PEGASUS_UNREACHABLE(PEGASUS_ASSERT(false);)
        }
    }
    _initialized = true;
}

struct ConfigProperty* IndicationServicePropertyOwner::_lookupConfigProperty(
    const String& name) const
{
    if (String::equal(_maxIndicationDeliveryRetryAttempts->propertyName, name))
    {
        return _maxIndicationDeliveryRetryAttempts.get();
    }
    else if (String::equal(
        _minIndicationDeliveryRetryInterval->propertyName, name))
    {
        return _minIndicationDeliveryRetryInterval.get();
    }
    else
    {
        throw UnrecognizedConfigProperty(name);
    }
}

void IndicationServicePropertyOwner::getPropertyInfo(
    const String& name,
    Array<String>& propertyInfo) const
{

    struct ConfigProperty * configProperty = _lookupConfigProperty(name);

    buildPropertyInfo(name, configProperty, propertyInfo);
}

String IndicationServicePropertyOwner::getDefaultValue(const String& name) const
{
    struct ConfigProperty * configProperty = _lookupConfigProperty(name);
    return configProperty->defaultValue;
}

String IndicationServicePropertyOwner::getCurrentValue(const String& name) const
{
    struct ConfigProperty * configProperty = _lookupConfigProperty(name);
    return configProperty->currentValue;
}

String IndicationServicePropertyOwner::getPlannedValue(const String& name) const
{
    struct ConfigProperty * configProperty = _lookupConfigProperty(name);
    return configProperty->plannedValue;
}

void IndicationServicePropertyOwner::initCurrentValue(
    const String& name,
    const String& value)
{
    if (String::equal(_maxIndicationDeliveryRetryAttempts->propertyName, name))
    {
        _maxIndicationDeliveryRetryAttempts->currentValue = value;
    }
    else if (String::equal(
        _minIndicationDeliveryRetryInterval->propertyName, name))
    {
        _minIndicationDeliveryRetryInterval->currentValue = value;
    }
    else
    {
        throw UnrecognizedConfigProperty(name);
    }
}

void IndicationServicePropertyOwner::initPlannedValue(
    const String& name,
    const String& value)
{
    struct ConfigProperty* configProperty = _lookupConfigProperty(name);
    configProperty->plannedValue = value;
}

void IndicationServicePropertyOwner::updateCurrentValue(
    const String& name,
    const String& value,
    const String& userName,
    Uint32 timeoutSeconds)
{
    if (!isDynamic(name))
    {
        throw NonDynamicConfigProperty(name);
    }
    initCurrentValue(name, value);
}

void IndicationServicePropertyOwner::updatePlannedValue(
    const String& name,
    const String& value)
{
    struct ConfigProperty* configProperty = _lookupConfigProperty(name);
    configProperty->plannedValue = value;
}

Boolean IndicationServicePropertyOwner::isValid(
    const String& name,
    const String& value) const
{
    Uint64 v;
    if (String::equal(
        _maxIndicationDeliveryRetryAttempts->propertyName, name) ||
        String::equal(
            _minIndicationDeliveryRetryInterval->propertyName, name))
    {
        return
            StringConversion::decimalStringToUint64(value.getCString(), v) &&
            StringConversion::checkUintBounds(v, CIMTYPE_UINT32);
    }
    else
    {
        throw UnrecognizedConfigProperty(name);
    }
}

Boolean IndicationServicePropertyOwner::isDynamic(const String& name) const
{
    struct ConfigProperty* configProperty = _lookupConfigProperty(name);
    return (configProperty->dynamic == IS_DYNAMIC);
}

PEGASUS_NAMESPACE_END
