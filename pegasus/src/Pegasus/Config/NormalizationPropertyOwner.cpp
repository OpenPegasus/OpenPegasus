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

#include<Pegasus/Common/ObjectNormalizer.h>
#include "ConfigManager.h"
#include "NormalizationPropertyOwner.h"
#include "ConfigExceptions.h"


PEGASUS_NAMESPACE_BEGIN

static struct ConfigPropertyRow properties[] =
{
#ifdef PEGASUS_USE_RELEASE_CONFIG_OPTIONS
    { "enableNormalization", "false", IS_DYNAMIC, IS_VISIBLE },
#else
    { "enableNormalization", "true", IS_DYNAMIC, IS_VISIBLE },
#endif
    { "excludeModulesFromNormalization", "", IS_STATIC, IS_VISIBLE }
};

const Uint32 NUM_PROPERTIES = sizeof(properties) / sizeof(properties[0]);

NormalizationPropertyOwner::NormalizationPropertyOwner()
{
    _providerObjectNormalizationEnabled.reset(new ConfigProperty());
    _providerObjectNormalizationModuleExclusions.reset(new ConfigProperty());
}

void NormalizationPropertyOwner::initialize()
{
    for (Uint8 i = 0; i < NUM_PROPERTIES; i++)
    {
        if (String::equal(
                properties[i].propertyName, "enableNormalization"))
        {
            _providerObjectNormalizationEnabled->propertyName =
                properties[i].propertyName;
            _providerObjectNormalizationEnabled->defaultValue =
                properties[i].defaultValue;
            _providerObjectNormalizationEnabled->currentValue =
                properties[i].defaultValue;
            _providerObjectNormalizationEnabled->plannedValue =
                properties[i].defaultValue;
            _providerObjectNormalizationEnabled->dynamic =
                properties[i].dynamic;
            _providerObjectNormalizationEnabled->externallyVisible =
                properties[i].externallyVisible;
            ObjectNormalizer::setEnableNormalization(
                ConfigManager::parseBooleanValue(properties[i].defaultValue));
        }
        else if (String::equal(properties[i].propertyName,
                     "excludeModulesFromNormalization"))
        {
            _providerObjectNormalizationModuleExclusions->propertyName =
                properties[i].propertyName;
            _providerObjectNormalizationModuleExclusions->defaultValue =
                properties[i].defaultValue;
            _providerObjectNormalizationModuleExclusions->currentValue =
                properties[i].defaultValue;
            _providerObjectNormalizationModuleExclusions->plannedValue =
                properties[i].defaultValue;
            _providerObjectNormalizationModuleExclusions->dynamic =
                properties[i].dynamic;
            _providerObjectNormalizationModuleExclusions->externallyVisible =
                properties[i].externallyVisible;
        }
    }
}

void NormalizationPropertyOwner::getPropertyInfo(
    const String& name,
    Array<String>& propertyInfo) const
{
    struct ConfigProperty* configProperty = _lookupConfigProperty(name);

    buildPropertyInfo(name, configProperty, propertyInfo);
}

String NormalizationPropertyOwner::getDefaultValue(const String& name) const
{
    struct ConfigProperty* configProperty = _lookupConfigProperty(name);

    return configProperty->defaultValue;
}

String NormalizationPropertyOwner::getCurrentValue(const String& name) const
{
    struct ConfigProperty* configProperty = _lookupConfigProperty(name);

    return configProperty->currentValue;
}

String NormalizationPropertyOwner::getPlannedValue(const String& name) const
{
    struct ConfigProperty* configProperty = _lookupConfigProperty(name);

    return configProperty->plannedValue;
}

void NormalizationPropertyOwner::initCurrentValue(
    const String& name,
    const String& value)
{
    struct ConfigProperty* configProperty = _lookupConfigProperty(name);

    configProperty->currentValue = value;
    ObjectNormalizer::setEnableNormalization(
        ConfigManager::parseBooleanValue(value));
}

void NormalizationPropertyOwner::initPlannedValue(
    const String& name,
    const String& value)
{
    struct ConfigProperty * configProperty = _lookupConfigProperty(name);

    configProperty->plannedValue = value;
}

void NormalizationPropertyOwner::updateCurrentValue(
    const String& name,
    const String& value,
    const String& userName,
    Uint32 timeoutSeconds)
{
    struct ConfigProperty* configProperty = _lookupConfigProperty(name);

    // make sure the property is dynamic before updating the value.
    if (configProperty->dynamic != IS_DYNAMIC)
    {
        throw NonDynamicConfigProperty(name);
    }

    configProperty->currentValue = value;
    ObjectNormalizer::setEnableNormalization(
        ConfigManager::parseBooleanValue(value));


}

void NormalizationPropertyOwner::updatePlannedValue(
    const String& name,
    const String& value)
{
    struct ConfigProperty* configProperty = _lookupConfigProperty(name);

    configProperty->plannedValue = value;
}

Boolean NormalizationPropertyOwner::isValid(
    const String& name,
    const String& value) const
{
    if (String::equal(name, "enableNormalization"))
    {
        // valid values are "true" and "false"
        return ConfigManager::isValidBooleanValue(value);
    }
    else if (String::equal(name, "excludeModulesFromNormalization"))
    {
        // valid values must be in the form "n.n.n"

        // TODO: validate value

        return true;
    }

    return false;
}

Boolean NormalizationPropertyOwner::isDynamic(const String& name) const
{
    struct ConfigProperty* configProperty = _lookupConfigProperty(name);

    return(configProperty->dynamic == IS_DYNAMIC);
}

struct ConfigProperty* NormalizationPropertyOwner::_lookupConfigProperty(
    const String& name) const
{
    if (String::equal(
            name, _providerObjectNormalizationEnabled->propertyName))
    {
        return _providerObjectNormalizationEnabled.get();
    }
    else if (String::equal(
        name, _providerObjectNormalizationModuleExclusions->propertyName))
    {
        return _providerObjectNormalizationModuleExclusions.get();
    }
    else
    {
        throw UnrecognizedConfigProperty(name);
    }
}

PEGASUS_NAMESPACE_END
