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
// This file has implementation for the repository property owner class.
//
///////////////////////////////////////////////////////////////////////////////

#include "RepositoryPropertyOwner.h"
#include "ConfigManager.h"
#include "ConfigExceptions.h"

PEGASUS_USING_STD;

PEGASUS_NAMESPACE_BEGIN

///////////////////////////////////////////////////////////////////////////////
//  RepositoryPropertyOwner
///////////////////////////////////////////////////////////////////////////////

static struct ConfigPropertyRow properties[] =
{
#if defined(PEGASUS_OS_LINUX)
# ifdef PEGASUS_USE_RELEASE_CONFIG_OPTIONS
    {"repositoryIsDefaultInstanceProvider", "false", IS_STATIC, IS_HIDDEN},
# else
    {"repositoryIsDefaultInstanceProvider", "true", IS_STATIC, IS_VISIBLE},
# endif
#else
    {"repositoryIsDefaultInstanceProvider", "true", IS_STATIC, IS_VISIBLE},
#endif
#ifndef PEGASUS_OS_ZOS
    {"enableBinaryRepository", "false", IS_STATIC, IS_VISIBLE}
#else
    {"enableBinaryRepository", "true", IS_STATIC, IS_HIDDEN}
#endif
};

const Uint32 NUM_PROPERTIES = sizeof(properties) / sizeof(properties[0]);


/** Constructors */
RepositoryPropertyOwner::RepositoryPropertyOwner()
{
    _repositoryIsDefaultInstanceProvider = new ConfigProperty;
    _enableBinaryRepository = new ConfigProperty;
}

/** Destructor */
RepositoryPropertyOwner::~RepositoryPropertyOwner()
{
    delete _repositoryIsDefaultInstanceProvider;
    delete _enableBinaryRepository;
}


/**
    Initialize the config properties.
*/
void RepositoryPropertyOwner::initialize()
{
    for (Uint32 i = 0; i < NUM_PROPERTIES; i++)
    {
        //
        // Initialize the properties with default values
        //
        if (String::equal(properties[i].propertyName,
                "repositoryIsDefaultInstanceProvider"))
        {
            _repositoryIsDefaultInstanceProvider->propertyName =
                properties[i].propertyName;
            _repositoryIsDefaultInstanceProvider->defaultValue =
                properties[i].defaultValue;
            _repositoryIsDefaultInstanceProvider->currentValue =
                properties[i].defaultValue;
            _repositoryIsDefaultInstanceProvider->plannedValue =
                properties[i].defaultValue;
            _repositoryIsDefaultInstanceProvider->dynamic =
                properties[i].dynamic;
            _repositoryIsDefaultInstanceProvider->externallyVisible =
                properties[i].externallyVisible;
        }
        else if (String::equal(
            properties[i].propertyName, "enableBinaryRepository"))
        {
            _enableBinaryRepository->propertyName = properties[i].propertyName;
            _enableBinaryRepository->defaultValue = properties[i].defaultValue;
            _enableBinaryRepository->currentValue = properties[i].defaultValue;
            _enableBinaryRepository->plannedValue = properties[i].defaultValue;
            _enableBinaryRepository->dynamic = properties[i].dynamic;
            _enableBinaryRepository->externallyVisible =
                properties[i].externallyVisible;
        }
    }
}

struct ConfigProperty* RepositoryPropertyOwner::_lookupConfigProperty(
    const String& name) const
{
    if (String::equal(
            _repositoryIsDefaultInstanceProvider->propertyName, name))
    {
        return _repositoryIsDefaultInstanceProvider;
    }
    else if (String::equal(
            _enableBinaryRepository->propertyName, name))
    {
        return _enableBinaryRepository;
    }
    else
    {
        throw UnrecognizedConfigProperty(name);
    }
}

/**
    Get information about the specified property.
*/
void RepositoryPropertyOwner::getPropertyInfo(
    const String& name,
    Array<String>& propertyInfo) const
{
    struct ConfigProperty * configProperty = _lookupConfigProperty(name);
    buildPropertyInfo(name, configProperty, propertyInfo);
}

/**
    Get default value of the specified property.
*/
String RepositoryPropertyOwner::getDefaultValue(const String& name) const
{
    struct ConfigProperty * configProperty = _lookupConfigProperty(name);

    return configProperty->defaultValue;
}

/**
    Get current value of the specified property.
*/
String RepositoryPropertyOwner::getCurrentValue(const String& name) const
{
    struct ConfigProperty * configProperty = _lookupConfigProperty(name);

    return configProperty->currentValue;
}

/**
    Get planned value of the specified property.
*/
String RepositoryPropertyOwner::getPlannedValue(const String& name) const
{
    struct ConfigProperty * configProperty = _lookupConfigProperty(name);

    return configProperty->plannedValue;
}


/**
    Init current value of the specified property to the specified value.
*/
void RepositoryPropertyOwner::initCurrentValue(
    const String& name,
    const String& value)
{
    struct ConfigProperty* configProperty = _lookupConfigProperty(name);
    configProperty->currentValue = value;
}


/**
    Init planned value of the specified property to the specified value.
*/
void RepositoryPropertyOwner::initPlannedValue(
    const String& name,
    const String& value)
{
    struct ConfigProperty* configProperty = _lookupConfigProperty(name);
    configProperty->plannedValue = value;
}

/**
    Update current value of the specified property to the specified value.
*/
void RepositoryPropertyOwner::updateCurrentValue(
    const String& name,
    const String& value,
    const String& userName,
    Uint32 timeoutSeconds)
{
    struct ConfigProperty * configProperty = _lookupConfigProperty(name);

    //
    // make sure the property is dynamic before updating the value.
    //
    if (configProperty->dynamic != IS_DYNAMIC)
    {
        throw NonDynamicConfigProperty(name);
    }

    configProperty->currentValue = value;
}


/**
    Update planned value of the specified property to the specified value.
*/
void RepositoryPropertyOwner::updatePlannedValue(
    const String& name,
    const String& value)
{
    //
    // Update does the same thing as initialization
    //
    initPlannedValue(name, value);
}

/**
    Checks to see if the given value is valid or not.
*/
Boolean RepositoryPropertyOwner::isValid(
    const String& name,
    const String& value) const
{
    Boolean retVal = false;

    //
    // Validate the specified value
    //
    if (String::equal(
            _repositoryIsDefaultInstanceProvider->propertyName, name) ||
        String::equal(_enableBinaryRepository->propertyName, name))
    {
        retVal = ConfigManager::isValidBooleanValue(value);
    }
    else
    {
        throw UnrecognizedConfigProperty(name);
    }
    return retVal;
}

/**
    Checks to see if the specified property is dynamic or not.
*/
Boolean RepositoryPropertyOwner::isDynamic(const String& name) const
{
    struct ConfigProperty * configProperty = _lookupConfigProperty(name);

    return (configProperty->dynamic == IS_DYNAMIC);
}

PEGASUS_NAMESPACE_END
