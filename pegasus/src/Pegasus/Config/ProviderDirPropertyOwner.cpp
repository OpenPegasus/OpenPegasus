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
// This file has implementation for the providerDir property owner class.
//
///////////////////////////////////////////////////////////////////////////////

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/Tracer.h>
#include <Pegasus/Common/FileSystem.h>
#include <Pegasus/Config/ConfigManager.h>
#include "ProviderDirPropertyOwner.h"
#include "ConfigExceptions.h"

PEGASUS_NAMESPACE_BEGIN

///////////////////////////////////////////////////////////////////////////////
//  ProviderDirPropertyOwner
//
//  When a new ProviderDir property is added, make sure to add the property name
//  and the default attributes of that property in the table below.
///////////////////////////////////////////////////////////////////////////////

static struct ConfigPropertyRow properties[] =
{
#if defined(PEGASUS_OS_TYPE_WINDOWS)
    {"providerDir", "lib;bin", IS_DYNAMIC, IS_VISIBLE},
#elif defined(PEGASUS_OS_ZOS)
    {"providerDir", "lib:provider", IS_DYNAMIC, IS_VISIBLE},
#elif defined(PEGASUS_OS_PASE) && defined(PEGASUS_USE_RELEASE_DIRS)
    {"providerDir", "/QOpenSys/QIBM/ProdData/UME/Pegasus/provider",
        IS_DYNAMIC, IS_VISIBLE}
#elif defined(PEGASUS_OS_VMS)
    {"providerDir", "/wbem_lib", IS_DYNAMIC, IS_VISIBLE},
#else
    {"providerDir", "lib", IS_DYNAMIC, IS_VISIBLE},
#endif
};

const Uint32 NUM_PROPERTIES = sizeof(properties) / sizeof(properties[0]);


/** Constructors  */
ProviderDirPropertyOwner::ProviderDirPropertyOwner()
{
    _providerDir = new ConfigProperty;
}

/** Destructor  */
ProviderDirPropertyOwner::~ProviderDirPropertyOwner()
{
    delete _providerDir;
}

/**
    Checks if the given directory is existing and writable
*/
Boolean isProviderDirValid(const String& dirName)
{
    String path;
    Uint32 pos=0;
    Uint32 token=0;

    // make multiple, relative paths absolute for check
    String temp = ConfigManager::getHomedPath(dirName);
    do
    {
        if (( pos = temp.find(FileSystem::getPathDelimiter())) == PEG_NOT_FOUND)
        {
            pos = temp.size();
            token = 0;
        }
        else
        {
            token = 1;
        }
        path = temp.subString(0,pos);
        if (!FileSystem::isDirectory(path))
        {
            Logger::put_l(
                Logger::ERROR_LOG, System::CIMSERVER, Logger::SEVERE,
                MessageLoaderParms(
                    "Config.ProviderDirPropertyOwner.NOT_A_DIRECTORY",
                    "providerDir configuration value $0 is not a directory.",
                    path));
            return false;
        }
        if (!FileSystem::canRead(path))
        {
            Logger::put_l(
                Logger::ERROR_LOG, System::CIMSERVER, Logger::SEVERE,
                MessageLoaderParms(
                    "Config.ProviderDirPropertyOwner.DIRECTORY_NOT_READABLE",
                    "providerDir configuration directory $0 is not readable.",
                    path));
            return false;
        }
        temp.remove(0,pos+token);
    }
    while ( temp.size() > 0 );
    return true;
}

/**
    Initialize the config properties.
*/
void ProviderDirPropertyOwner::initialize()
{
    for (Uint32 i = 0; i < NUM_PROPERTIES; i++)
    {
        //
        // Initialize the properties with default values
        //
        if (String::equal(properties[i].propertyName, "providerDir"))
        {
            _providerDir->propertyName = properties[i].propertyName;
            _providerDir->defaultValue = properties[i].defaultValue;
            _providerDir->currentValue = properties[i].defaultValue;
            _providerDir->plannedValue = properties[i].defaultValue;
            _providerDir->dynamic = properties[i].dynamic;
            _providerDir->externallyVisible = properties[i].externallyVisible;
        }
    }
}

struct ConfigProperty* ProviderDirPropertyOwner::_lookupConfigProperty(
    const String& name) const
{
    if (String::equal(_providerDir->propertyName, name))
    {
        return _providerDir;
    }
    else
    {
        throw UnrecognizedConfigProperty(name);
    }
}

/**
    Get information about the specified property.
*/
void ProviderDirPropertyOwner::getPropertyInfo(
    const String& name,
    Array<String>& propertyInfo) const
{
    struct ConfigProperty * configProperty = _lookupConfigProperty(name);

    buildPropertyInfo(name, configProperty, propertyInfo);
}

/**
    Get default value of the specified property.
*/
String ProviderDirPropertyOwner::getDefaultValue(const String& name) const
{
    struct ConfigProperty * configProperty = _lookupConfigProperty(name);

    return configProperty->defaultValue;
}

/**
    Get current value of the specified property.
*/
String ProviderDirPropertyOwner::getCurrentValue(const String& name) const
{
    struct ConfigProperty * configProperty = _lookupConfigProperty(name);

    AutoMutex lock(_providerDirMutex);
    return configProperty->currentValue;
}

/**
    Get planned value of the specified property.
*/
String ProviderDirPropertyOwner::getPlannedValue(const String& name) const
{
   struct ConfigProperty * configProperty = _lookupConfigProperty(name);

    return configProperty->plannedValue;
}

/**
    Init current value of the specified property to the specified value.
*/
void ProviderDirPropertyOwner::initCurrentValue(
    const String& name,
    const String& value)
{
    struct ConfigProperty* configProperty = _lookupConfigProperty(name);
    AutoMutex lock(_providerDirMutex);
    configProperty->currentValue = value;
}


/**
    Init planned value of the specified property to the specified value.
*/
void ProviderDirPropertyOwner::initPlannedValue(
    const String& name,
    const String& value)
{
    struct ConfigProperty* configProperty = _lookupConfigProperty(name);
    configProperty->plannedValue = value;
}

/**
    Update current value of the specified property to the specified value.
*/
void ProviderDirPropertyOwner::updateCurrentValue(
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

    AutoMutex lock(_providerDirMutex);
    configProperty->currentValue = value;
}


/**
    Update planned value of the specified property to the specified value.
*/
void ProviderDirPropertyOwner::updatePlannedValue(
    const String& name,
    const String& value)
{
    struct ConfigProperty* configProperty = _lookupConfigProperty(name);
    configProperty->plannedValue = value;
}

/**
    Checks to see if the given value is valid or not.
*/
Boolean ProviderDirPropertyOwner::isValid(
    const String& name,
    const String& value) const
{

    if (!isProviderDirValid( value ))
    {
        throw InvalidPropertyValue(name, value);
    }

    return true;
}

/**
    Checks to see if the specified property is dynamic or not.
*/
Boolean ProviderDirPropertyOwner::isDynamic(const String& name) const
{
    struct ConfigProperty * configProperty = _lookupConfigProperty(name);

    return (configProperty->dynamic == IS_DYNAMIC);
}


PEGASUS_NAMESPACE_END
