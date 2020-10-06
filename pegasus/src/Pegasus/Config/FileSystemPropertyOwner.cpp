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
// This file has implementation for the file system property owner class.
//
///////////////////////////////////////////////////////////////////////////////

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/Tracer.h>
#include <Pegasus/Common/FileSystem.h>
#include <Pegasus/Config/ConfigManager.h>
#include "FileSystemPropertyOwner.h"
#include "ConfigExceptions.h"



PEGASUS_USING_STD;

PEGASUS_NAMESPACE_BEGIN

///////////////////////////////////////////////////////////////////////////////
//  FileSystemPropertyOwner
//
//  When a new FileSystem property is added, make sure to add the property name
//  and the default attributes of that property in the table below.
///////////////////////////////////////////////////////////////////////////////

static struct ConfigPropertyRow properties[] =
{
    {"repositoryDir", PEGASUS_REPOSITORY_DIR, IS_STATIC, IS_VISIBLE},
#if defined(PEGASUS_OS_PASE)
    {"messageDir", "/QOpenSys/QIBM/ProdData/UME/Pegasus/msg", IS_STATIC,
        IS_VISIBLE},
#else
    {"messageDir", "msg", IS_STATIC, IS_VISIBLE},
#endif
#if defined(PEGASUS_OS_TYPE_WINDOWS)
    {"providerManagerDir", "bin", IS_STATIC, IS_VISIBLE},
#elif defined(PEGASUS_OS_ZOS)
    {"providerManagerDir", "lib", IS_STATIC, IS_VISIBLE},
#elif defined(PEGASUS_OS_PASE) && defined(PEGASUS_USE_RELEASE_DIRS)
    {"providerManagerDir", "/QOpenSys/QIBM/ProdData/UME/Pegasus/lib",
        IS_STATIC, IS_VISIBLE},
#elif defined(PEGASUS_OS_AIX) && defined(PEGASUS_USE_RELEASE_DIRS)
    {"providerManagerDir", "/usr/lib", IS_STATIC, IS_VISIBLE},
#elif defined(PEGASUS_OS_VMS)
    {"providerManagerDir", "/wbem_lib", IS_STATIC, IS_VISIBLE},
#else
    {"providerManagerDir", "lib", IS_STATIC, IS_VISIBLE},
#endif
};

const Uint32 NUM_PROPERTIES = sizeof(properties) / sizeof(properties[0]);


/** Constructors  */
FileSystemPropertyOwner::FileSystemPropertyOwner()
{
    _repositoryDir.reset(new ConfigProperty);
    _messageDir.reset(new ConfigProperty);
    _providerManagerDir.reset(new ConfigProperty);
}


/**
    Checks if the given directory is existing and writable
*/
Boolean isDirValid(const String& dirName)
{
    String directoryName(ConfigManager::getHomedPath(dirName));
    if (FileSystem::isDirectory(directoryName) &&
        FileSystem::canWrite(directoryName))
    {
        return true;
    }
    return false;
}

/**
    Initialize the config properties.
*/
void FileSystemPropertyOwner::initialize()
{
    for (Uint32 i = 0; i < NUM_PROPERTIES; i++)
    {
        //
        // Initialize the properties with default values
        //
        if (String::equal(properties[i].propertyName, "repositoryDir"))
        {
            _repositoryDir->propertyName = properties[i].propertyName;
            _repositoryDir->defaultValue = properties[i].defaultValue;
            _repositoryDir->currentValue = properties[i].defaultValue;
            _repositoryDir->plannedValue = properties[i].defaultValue;
            _repositoryDir->dynamic = properties[i].dynamic;
            _repositoryDir->externallyVisible = properties[i].externallyVisible;
        }
        else if (String::equal(properties[i].propertyName, "messageDir"))
        {
            _messageDir->propertyName = properties[i].propertyName;
            _messageDir->defaultValue = properties[i].defaultValue;
            _messageDir->currentValue = properties[i].defaultValue;
            _messageDir->plannedValue = properties[i].defaultValue;
            _messageDir->dynamic = properties[i].dynamic;
            _messageDir->externallyVisible = properties[i].externallyVisible;
        }
        else if (String::equal(properties[i].propertyName,
                 "providerManagerDir"))
        {
            _providerManagerDir->propertyName = properties[i].propertyName;
            _providerManagerDir->defaultValue = properties[i].defaultValue;
            _providerManagerDir->currentValue = properties[i].defaultValue;
            _providerManagerDir->plannedValue = properties[i].defaultValue;
            _providerManagerDir->dynamic = properties[i].dynamic;
            _providerManagerDir->externallyVisible =
                                           properties[i].externallyVisible;
        }
    }
}

struct ConfigProperty* FileSystemPropertyOwner::_lookupConfigProperty(
    const String& name) const
{
    if (String::equal(_repositoryDir->propertyName, name))
    {
        return _repositoryDir.get();
    }
    if (String::equal(_messageDir->propertyName, name))
    {
        return _messageDir.get();
    }
    if (String::equal(_providerManagerDir->propertyName, name))
    {
        return _providerManagerDir.get();
    }
    throw UnrecognizedConfigProperty(name);
}

/**
    Get information about the specified property.
*/
void FileSystemPropertyOwner::getPropertyInfo(
    const String& name,
    Array<String>& propertyInfo) const
{
    struct ConfigProperty* configProperty = _lookupConfigProperty(name);

    buildPropertyInfo(name, configProperty, propertyInfo);
}

/**
    Get default value of the specified property.
*/
String FileSystemPropertyOwner::getDefaultValue(const String& name) const
{
    struct ConfigProperty* configProperty = _lookupConfigProperty(name);

    return configProperty->defaultValue;
}

/**
    Get current value of the specified property.
*/
String FileSystemPropertyOwner::getCurrentValue(const String& name) const
{
    struct ConfigProperty* configProperty = _lookupConfigProperty(name);

    return configProperty->currentValue;
}

/**
    Get planned value of the specified property.
*/
String FileSystemPropertyOwner::getPlannedValue(const String& name) const
{
    struct ConfigProperty* configProperty = _lookupConfigProperty(name);

    return configProperty->plannedValue;
}

/**
    Init current value of the specified property to the specified value.
*/
void FileSystemPropertyOwner::initCurrentValue(
    const String& name,
    const String& value)
{
    struct ConfigProperty* configProperty = _lookupConfigProperty(name);
    configProperty->currentValue = value;
}


/**
    Init planned value of the specified property to the specified value.
*/
void FileSystemPropertyOwner::initPlannedValue(
    const String& name,
    const String& value)
{
    struct ConfigProperty* configProperty = _lookupConfigProperty(name);
    configProperty->plannedValue = value;
}

/**
    Update current value of the specified property to the specified value.
*/
void FileSystemPropertyOwner::updateCurrentValue(
    const String& name,
    const String& value,
    const String& userName,
    Uint32 timeoutSeconds)
{
    struct ConfigProperty* configProperty = _lookupConfigProperty(name);

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
void FileSystemPropertyOwner::updatePlannedValue(
    const String& name,
    const String& value)
{
    struct ConfigProperty* configProperty = _lookupConfigProperty(name);
    configProperty->plannedValue = value;
}

/**
    Checks to see if the given value is valid or not.
*/
Boolean FileSystemPropertyOwner::isValid(
    const String& name,
    const String& value) const
{
    if (!isDirValid(value))
    {
        throw InvalidDirectoryPropertyValue(name, value);
    }

    return true;
}

/**
    Checks to see if the specified property is dynamic or not.
*/
Boolean FileSystemPropertyOwner::isDynamic(const String& name) const
{
    struct ConfigProperty* configProperty = _lookupConfigProperty(name);

    return (configProperty->dynamic == IS_DYNAMIC);
}


PEGASUS_NAMESPACE_END
