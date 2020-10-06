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
//  ConfigManager
/////////////////////////////////////////////////////////////////////////////

#include <Pegasus/Common/Tracer.h>
#include <Pegasus/Common/Logger.h>
#include <Pegasus/Common/PegasusVersion.h>
#include <Pegasus/Common/FileSystem.h>
#include <Pegasus/Common/Constants.h>
#include <Pegasus/Common/StringConversion.h>

#include "ConfigExceptions.h"
#include "ConfigManager.h"

PEGASUS_NAMESPACE_BEGIN

/////////////////////////////////////////////////////////////////////////////
//
//  When a new property owner is created be sure to create a static
//  object of the new property owner.
//
/////////////////////////////////////////////////////////////////////////////

TracePropertyOwner ConfigManager::traceOwner;
LogPropertyOwner ConfigManager::logOwner;
DefaultPropertyOwner ConfigManager::defaultOwner;
SecurityPropertyOwner ConfigManager::securityOwner;
RepositoryPropertyOwner ConfigManager::repositoryOwner;
ShutdownPropertyOwner ConfigManager::shutdownOwner;
FileSystemPropertyOwner ConfigManager::fileSystemOwner;
ProviderDirPropertyOwner ConfigManager::providerDirOwner;
NormalizationPropertyOwner ConfigManager::normalizationOwner;

#ifdef PEGASUS_ENABLE_DMTF_INDICATION_PROFILE_SUPPORT
IndicationServicePropertyOwner ConfigManager::indicationServiceOwner;
#endif

/////////////////////////////////////////////////////////////////////////////
//
//   When a new property is created be sure to add the new property name
//   and the owner object to the OwnerEntry table below.
//
/////////////////////////////////////////////////////////////////////////////

struct OwnerEntry
{
    const char* propertyName;
    ConfigPropertyOwner* propertyOwner;
};

static struct OwnerEntry _properties[] =
{
    {"traceLevel",
         (ConfigPropertyOwner*)&ConfigManager::traceOwner},
    {"traceComponents",
         (ConfigPropertyOwner*)&ConfigManager::traceOwner},
    {"traceFilePath",
         (ConfigPropertyOwner*)&ConfigManager::traceOwner},
    {"traceFacility",
         (ConfigPropertyOwner*)&ConfigManager::traceOwner},
    {"traceMemoryBufferKbytes",
         (ConfigPropertyOwner*)&ConfigManager::traceOwner},
    {"traceFileSizeKBytes",
         (ConfigPropertyOwner*)&ConfigManager::traceOwner},
    {"numberOfTraceFiles",
         (ConfigPropertyOwner*)&ConfigManager::traceOwner},
#if !defined(PEGASUS_USE_SYSLOGS)
    {"logdir",
         (ConfigPropertyOwner*)&ConfigManager::logOwner},
    {"maxLogFileSizeKBytes",
         (ConfigPropertyOwner*)&ConfigManager::logOwner},
#endif
    {"logLevel",
         (ConfigPropertyOwner*)&ConfigManager::logOwner},
    {"enableHttpConnection",
         (ConfigPropertyOwner*)&ConfigManager::defaultOwner},
    {"enableHttpsConnection",
         (ConfigPropertyOwner*)&ConfigManager::defaultOwner},
    {"httpPort",
         (ConfigPropertyOwner*)&ConfigManager::defaultOwner},
    {"httpsPort",
         (ConfigPropertyOwner*)&ConfigManager::defaultOwner},
    {"daemon",
         (ConfigPropertyOwner*)&ConfigManager::defaultOwner},
#ifdef PEGASUS_ENABLE_SLP
    {"slp",
         (ConfigPropertyOwner*)&ConfigManager::defaultOwner},
    {"slpProviderStartupTimeout",
         (ConfigPropertyOwner*)&ConfigManager::defaultOwner},
#endif
    {"enableAssociationTraversal",
         (ConfigPropertyOwner*)&ConfigManager::defaultOwner},
    {"enableIndicationService",
         (ConfigPropertyOwner*)&ConfigManager::defaultOwner},
    {"enableAuthentication",
         (ConfigPropertyOwner*)&ConfigManager::securityOwner},
    {"enableNamespaceAuthorization",
         (ConfigPropertyOwner*)&ConfigManager::securityOwner},
    {"httpAuthType",
         (ConfigPropertyOwner*)&ConfigManager::securityOwner},
    {"passwordFilePath",
         (ConfigPropertyOwner*)&ConfigManager::securityOwner},
    {"sslCertificateFilePath",
         (ConfigPropertyOwner*)&ConfigManager::securityOwner},
    {"sslKeyFilePath",
         (ConfigPropertyOwner*)&ConfigManager::securityOwner},
    {"sslTrustStore",
         (ConfigPropertyOwner*)&ConfigManager::securityOwner},
    {"sslBackwardCompatibility",
         (ConfigPropertyOwner*)&ConfigManager::securityOwner},
#ifdef PEGASUS_ENABLE_SSL_CRL_VERIFICATION
    {"crlStore",
         (ConfigPropertyOwner*)&ConfigManager::securityOwner},
#endif
    {"sslClientVerificationMode",
         (ConfigPropertyOwner*)&ConfigManager::securityOwner},
    {"sslTrustStoreUserName",
         (ConfigPropertyOwner*)&ConfigManager::securityOwner},
#ifdef PEGASUS_OS_ZOS
    {"enableCFZAPPLID",
         (ConfigPropertyOwner*)&ConfigManager::securityOwner},
#endif
    {"sslCipherSuite",
         (ConfigPropertyOwner*)&ConfigManager::securityOwner},
    {"repositoryIsDefaultInstanceProvider",
         (ConfigPropertyOwner*)&ConfigManager::repositoryOwner},
    {"enableBinaryRepository",
         (ConfigPropertyOwner*)&ConfigManager::repositoryOwner},
    {"shutdownTimeout",
         (ConfigPropertyOwner*)&ConfigManager::shutdownOwner},
    {"repositoryDir",
         (ConfigPropertyOwner*)&ConfigManager::fileSystemOwner},
    {"providerManagerDir",
         (ConfigPropertyOwner*)&ConfigManager::fileSystemOwner},
    {"providerDir",
         (ConfigPropertyOwner*)&ConfigManager::providerDirOwner},
    {"enableRemotePrivilegedUserAccess",
         (ConfigPropertyOwner*)&ConfigManager::securityOwner},
    {"enableSubscriptionsForNonprivilegedUsers",
         (ConfigPropertyOwner*)&ConfigManager::securityOwner},
#ifdef PEGASUS_ENABLE_USERGROUP_AUTHORIZATION
    {"authorizedUserGroups",
         (ConfigPropertyOwner*)&ConfigManager::securityOwner},
#endif
    {"messageDir",
         (ConfigPropertyOwner*)&ConfigManager::fileSystemOwner},
#ifdef PEGASUS_ENABLE_OBJECT_NORMALIZATION
    {"enableNormalization",
         (ConfigPropertyOwner*)&ConfigManager::normalizationOwner},
    {"excludeModulesFromNormalization",
         (ConfigPropertyOwner*)&ConfigManager::normalizationOwner},
#endif
    {"forceProviderProcesses",
         (ConfigPropertyOwner*)&ConfigManager::defaultOwner},
    {"maxProviderProcesses",
         (ConfigPropertyOwner*)&ConfigManager::defaultOwner},
#ifdef PEGASUS_ENABLE_AUDIT_LOGGER
    {"enableAuditLog",
         (ConfigPropertyOwner*)&ConfigManager::defaultOwner},
#endif
#ifdef PEGASUS_ENABLE_PROTOCOL_WEB
      {"webRoot",
               (ConfigPropertyOwner*)&ConfigManager::defaultOwner},
      {"indexFile",
               (ConfigPropertyOwner*)&ConfigManager::defaultOwner},
      {"mimeTypesFile",
               (ConfigPropertyOwner*)&ConfigManager::defaultOwner},
#endif
    {"socketWriteTimeout",
         (ConfigPropertyOwner*)&ConfigManager::defaultOwner},
    {"idleConnectionTimeout",
         (ConfigPropertyOwner*)&ConfigManager::defaultOwner},
    {"maxFailedProviderModuleRestarts",
         (ConfigPropertyOwner*)&ConfigManager::defaultOwner},
    {"listenAddress",
         (ConfigPropertyOwner*)&ConfigManager::defaultOwner},
    {"hostname",
         (ConfigPropertyOwner*)&ConfigManager::defaultOwner},
    {"fullyQualifiedHostName",
        (ConfigPropertyOwner*)&ConfigManager::defaultOwner}

#ifdef PEGASUS_ENABLE_SESSION_COOKIES
    ,{"httpSessionTimeout",
        (ConfigPropertyOwner*)&ConfigManager::securityOwner}
#endif

#ifdef PEGASUS_ENABLE_DMTF_INDICATION_PROFILE_SUPPORT
    ,{"maxIndicationDeliveryRetryAttempts",
        (ConfigPropertyOwner*)&ConfigManager::indicationServiceOwner},
    {"minIndicationDeliveryRetryInterval",
        (ConfigPropertyOwner*)&ConfigManager::indicationServiceOwner}
#endif

#ifdef PEGASUS_NEGOTIATE_AUTHENTICATION
    ,{"mapToLocalName",
         (ConfigPropertyOwner*)&ConfigManager::securityOwner}
#endif
    ,{"pullOperationsMaxObjectCount",
        (ConfigPropertyOwner*)&ConfigManager::defaultOwner},
    {"pullOperationsMaxTimeout",
        (ConfigPropertyOwner*)&ConfigManager::defaultOwner},
    {"pullOperationsDefaultTimeout",
        (ConfigPropertyOwner*)&ConfigManager::defaultOwner}
};

const Uint32 NUM_PROPERTIES = sizeof(_properties) / sizeof(_properties[0]);


/////////////////////////////////////////////////////////////////////////////
//
//   To use a fixed value for a property rather than delegating to a property
//   owner, add the property to the FixedValueEntry table below.  An entry in
//   the OwnerEntry table above for this same property will be initialized
//   and given the (fixed) initial current value, but will thereafter be
//   ignored.
//
//   Fixed values are only returned by getDefaultValue(), getCurrentValue(),
//   and getPlannedValue().  All other methods will treat fixed properties as
//   unrecognized properties.
//
/////////////////////////////////////////////////////////////////////////////

struct FixedValueEntry
{
    const char* propertyName;
    const char* fixedValue;
};

static struct FixedValueEntry _fixedValues[] =
{
#include "FixedPropertyTable.h"
};

const Uint32 NUM_FIXED_PROPERTIES =
    sizeof(_fixedValues) / sizeof( _fixedValues[0]);


/**
    Initialize the default PEGASUS_HOME location, the default is set to the
    current directory.
*/
const String ConfigManager::PEGASUS_HOME_DEFAULT = ".";

String ConfigManager::_pegasusHome = PEGASUS_HOME_DEFAULT;

//
// The singleton ConfigManager instance
//
AutoPtr<ConfigManager> ConfigManager::_instance;


/** Constructor. */
ConfigManager::ConfigManager()
    : useConfigFiles(false)
{
    //
    // Initialize the instance variables
    //
    _propertyTable.reset(new PropertyTable);

    //
    // Initialize the property owners
    //
    _initPropertyTable();
}

/**
    Get a reference to the singleton ConfigManager instance.  If no
    ConfigManager instance exists, construct one.
*/
ConfigManager* ConfigManager::getInstance()
{
    if (!_instance.get())
    {
        _instance.reset(new ConfigManager());
    }
    return _instance.get();
}


/**
    Initialize the current value of a config property
*/
Boolean ConfigManager::initCurrentValue(
    const String& propertyName,
    const String& propertyValue)
{
    ConfigPropertyOwner* propertyOwner = 0;

    //
    // get property owner object from the config table.
    //
    if (!_propertyTable->ownerTable.lookup(propertyName, propertyOwner))
    {
        throw UnrecognizedConfigProperty(propertyName);
    }

    if (useConfigFiles && !propertyOwner->isValid(propertyName, propertyValue))
    {
        throw InvalidPropertyValue(propertyName, propertyValue);
    }

    //
    // update the value with the property owner
    //
    propertyOwner->initCurrentValue(propertyName, propertyValue);

    Boolean success = true;
    if (useConfigFiles)
    {
        try
        {
            // update the value in the current config file
            success = _configFileHandler->updateCurrentValue(
                          propertyName,
                          propertyValue,
                          String(),
                          0,
                          false);
        }
        catch (Exception& e)
        {
            throw FailedSaveProperties(e.getMessage());
        }
    }
    return success;
}

/**
    Update current value of a property.
*/
Boolean ConfigManager::updateCurrentValue(
    const String& name,
    const String& value,
    const String& userName,
    Uint32 timeoutSeconds,
    Boolean unset)
{

    //
    // get property owner object from the config table.
    //
    ConfigPropertyOwner* propertyOwner;

    if (!_propertyTable->ownerTable.lookup(name, propertyOwner))
    {
        throw UnrecognizedConfigProperty(name);
    }

    //
    // keep a copy of the existing config value
    //
    String prevValue = propertyOwner->getCurrentValue(name);

    //
    // ask owner to update the current value
    //
    if (unset)
    {
        propertyOwner->updateCurrentValue(
            name,
            propertyOwner->getDefaultValue(name),
            userName,
            timeoutSeconds);
    }
    else
    {
        if (useConfigFiles && !propertyOwner->isValid(name, value))
        {
            throw InvalidPropertyValue(name, value);
        }

        propertyOwner->updateCurrentValue(
            name, value, userName, timeoutSeconds);
    }

    if (useConfigFiles)
    {
        try
        {
            //
            // update the new value in the current config file
            //
            if (!_configFileHandler->updateCurrentValue(
                name, value, userName, timeoutSeconds, unset))
            {
                // Failed to update the current value, so roll back.
                propertyOwner->updateCurrentValue(
                    name, prevValue, userName, timeoutSeconds);
                return false;
            }
        }
        catch (Exception& e)
        {
            // Failed to update the current value, so roll back.
            propertyOwner->updateCurrentValue(
                name, prevValue, userName, timeoutSeconds);
            throw FailedSaveProperties(e.getMessage());
        }
    }

    return true;
}


/**
Update planned value of a property.
*/
Boolean ConfigManager::updatePlannedValue(
    const String& name,
    const String& value,
    Boolean unset)
{

    //
    // get property owner object from the config table.
    //
    ConfigPropertyOwner* propertyOwner;

    if (!_propertyTable->ownerTable.lookup(name, propertyOwner))
    {
        throw UnrecognizedConfigProperty(name);
    }

    //
    // keep a copy of the existing config value
    //
    String prevValue = propertyOwner->getPlannedValue(name);

    //
    // ask owner to update the planned value to new value
    //
    if (unset)
    {
        propertyOwner->updatePlannedValue(name,
            propertyOwner->getDefaultValue(name));
    }
    else
    {
        if (useConfigFiles && !propertyOwner->isValid(name, value))
        {
            throw InvalidPropertyValue(name, value);
        }

        propertyOwner->updatePlannedValue(name, value);
    }

    if (useConfigFiles)
    {
        try
        {
            //
            // update the new value in the planned config file
            //
            if (!_configFileHandler->updatePlannedValue(name, value, unset))
            {
                // Failed to update the planned value, so roll back.
                propertyOwner->updatePlannedValue(name, prevValue);
                return false;
            }
        }
        catch (Exception& e)
        {
            // Failed to update the planned value, so roll back.
            propertyOwner->updatePlannedValue(name, prevValue);
            throw FailedSaveProperties(e.getMessage());
        }
    }

    return true;
}


/**
Validate the value of a specified property.
*/
Boolean ConfigManager::validatePropertyValue(
    const String& name,
    const String& value)
{
    //
    // get property owner object from config table
    //
    ConfigPropertyOwner* propertyOwner;

    if (!_propertyTable->ownerTable.lookup(name, propertyOwner))
    {
        throw UnrecognizedConfigProperty(name);
    }

    return propertyOwner->isValid(name, value);
}

/**
Get default value of the specified property.
*/
String ConfigManager::getDefaultValue(const String& name) const
{
    String fixedValue;
    if (_fixedValueCheck(name, fixedValue))
    {
        return fixedValue;
    }

    //
    // get property owner object from config table
    //
    ConfigPropertyOwner* propertyOwner;

    if (!_propertyTable->ownerTable.lookup(name, propertyOwner))
    {
        throw UnrecognizedConfigProperty(name);
    }

    return propertyOwner->getDefaultValue(name);
}

/**
    Get current value of the specified property.
*/
String ConfigManager::getCurrentValue(const String& name) const
{
    String fixedValue;
    if (_fixedValueCheck(name, fixedValue))
    {
        return fixedValue;
    }

    //
    // get property owner object from config table
    //
    ConfigPropertyOwner* propertyOwner;

    if (!_propertyTable->ownerTable.lookup(name, propertyOwner))
    {
        throw UnrecognizedConfigProperty(name);
    }

    return propertyOwner->getCurrentValue(name);
}


/**
Get planned value of the specified property.
*/
String ConfigManager::getPlannedValue(const String& name) const
{
    String fixedValue;
    if (_fixedValueCheck(name, fixedValue))
    {
        return fixedValue;
    }

    //
    // get property owner object from config table
    //
    ConfigPropertyOwner* propertyOwner;

    if (!_propertyTable->ownerTable.lookup(name, propertyOwner))
    {
        throw UnrecognizedConfigProperty(name);
    }

    return propertyOwner->getPlannedValue(name);
}

/**
Get help on specified attribute
*/
void ConfigManager::getPropertyHelp(
    const String& name,
    String& propertyHelp) const
{
    //
    // get property owner object from config table
    //
    ConfigPropertyOwner* propertyOwner;
    if ( !_propertyTable->ownerTable.lookup(name,
        propertyOwner))
    {
        throw UnrecognizedConfigProperty(name);
    }
    propertyHelp.append(propertyOwner->getPropertyHelp(name));
    propertyHelp.append(propertyOwner->getPropertyHelpSupplement(name));
}

/**
Get all the attributes of the specified property.
*/
void ConfigManager::getPropertyInfo(
    const String& name,
    Array<String>& propertyInfo) const
{
    //
    // get property owner object from config table
    //
    ConfigPropertyOwner* propertyOwner;

    if (!_propertyTable->ownerTable.lookup(name, propertyOwner))
    {
        throw UnrecognizedConfigProperty(name);
    }

    propertyOwner->getPropertyInfo(name, propertyInfo);
}


/**
Get a list of all property names.
*/
void ConfigManager::getAllPropertyNames(
    Array<String>& propertyNames,
    Boolean includeHiddenProperties) const
{
    Array<String> propertyInfo;
    propertyNames.clear();

    for (OwnerTable::Iterator i = _propertyTable->ownerTable.start(); i; i++)
    {
        if (includeHiddenProperties)
        {
            propertyNames.append(i.key());
        }
        else
        {
            //
            // Check if property is to be externally visible or not.
            // If the property should not be externally visible do not list the
            // property information.
            //
            propertyInfo.clear();
            getPropertyInfo(i.key(), propertyInfo);

            if (propertyInfo[5] == STRING_TRUE)
            {
                propertyNames.append(i.key());
            }
        }
    }
}


/**
    Merge the config properties from the specified planned config file
    with the properties in the specified current config file.
*/
void ConfigManager::mergeConfigFiles(
    const String& currentFile,
    const String& plannedFile)
{
    PEGASUS_ASSERT(useConfigFiles);

    _configFileHandler.reset(new ConfigFileHandler(currentFile, plannedFile));

    //
    // copy the contents of planned config file over
    // the current config file
    //
    _configFileHandler->copyPlannedFileOverCurrentFile();

    _loadConfigProperties();
}


/**
    Merge the config properties from the default planned config file
    with the properties in the default current config file.
*/
void ConfigManager::mergeConfigFiles()
{
    PEGASUS_ASSERT(useConfigFiles);

    _configFileHandler.reset(new ConfigFileHandler());

    //
    // copy the contents of planned config file over
    // the current config file
    //
    _configFileHandler->copyPlannedFileOverCurrentFile();

    _loadConfigProperties();
}


/**
Load the config properties from the current and planned files.
*/
void ConfigManager::loadConfigFiles()
{
    PEGASUS_ASSERT(useConfigFiles);

    _configFileHandler.reset(new ConfigFileHandler());

    _loadConfigProperties();
}


/**
    Merge config properties specified on the command line
*/
void ConfigManager::mergeCommandLine(int& argc, char**& argv)
{
    // Remove the command name from the command line
    if (argc > 0)
    {
        memmove(&argv[0], &argv[1], (argc) * sizeof(char*));
        argc--;
    }

    //
    //  Merge properties from the command line
    //
    for (Sint32 i = 0; i < argc; )
    {
        const char* arg = argv[i];

        if (*arg == '-')
        {
            throw UnrecognizedCommandLineOption();
        }

        // Set porperty with command line value.
        if (!_initPropertyWithCommandLineOption(arg))
        {
            throw UnrecognizedConfigProperty(arg);
        }

        // Remove the option from the command line
        memmove(&argv[i], &argv[i + 1], (argc-i) * sizeof(char*));
        argc--;
    }
}


/**
    load config properties from the file
*/
void ConfigManager::_loadConfigProperties()
{
    PEGASUS_ASSERT(useConfigFiles);

    //
    // load all the properties from the current and planned
    // config files in to tables.
    //
    _configFileHandler->loadAllConfigProperties();

    Array<CIMName> propertyNames;
    Array<String>  propertyValues;

    _configFileHandler->getAllCurrentProperties(propertyNames, propertyValues);

    Uint32 size = propertyNames.size();

    //
    // initialize all the property owners with the values
    // from the config files.
    //
    for (Uint32 i = 0; i < size; i++)
    {
        //
        // initialize the current value of the property owner
        // with the value from the config file handler
        //
        try
        {
            //
            // get property owner object from the config table.
            //
            ConfigPropertyOwner* propertyOwner;

            String propertyName = propertyNames[i].getString();

            if (_propertyTable->ownerTable.lookup(
                propertyName, propertyOwner))
            {
                if (propertyOwner->isValid(
                    propertyName, propertyValues[i]))
                {
                    propertyOwner->initCurrentValue(
                        propertyName, propertyValues[i]);

                    propertyOwner->initPlannedValue(
                        propertyName, propertyValues[i]);
                }
                else
                {
                    throw InvalidPropertyValue(propertyName, propertyValues[i]);
                }
            }
            else
            {
                // if the property is a fixed property then just log that
                // this property is not supported and continue.  In all other
                // cases terminate the cimserver
                if (_propertyTable->fixedValueTable.contains(propertyName))
                {
                    Logger::put_l(Logger::STANDARD_LOG, System::CIMSERVER,
                        Logger::WARNING,
                        MessageLoaderParms(
                            "Config.ConfigManager.NOTSUPPORTED_CONFIG_PROPERTY",
                            "Configuration property $0 is not supported. "
                                "Setting ignored.",
                            propertyName));
                }
                else
                {
                    throw UnrecognizedConfigProperty(propertyName);
                }
            }
        }
        catch (UnrecognizedConfigProperty& ucp)
        {
            PEG_TRACE_CSTRING(TRC_CONFIG, Tracer::LEVEL1,
                (const char*)ucp.getMessage().getCString());
            throw;
        }
        catch (InvalidPropertyValue& ipv)
        {
            PEG_TRACE_CSTRING(TRC_CONFIG, Tracer::LEVEL1,
                (const char*)ipv.getMessage().getCString());
            throw;
        }
    }
}


/**
    Initialize config property with the value specified in the command line.
*/
Boolean ConfigManager::_initPropertyWithCommandLineOption(
    const String& option)
{
    Uint32 pos = option.find('=');

    if (pos == PEG_NOT_FOUND)
    {
        //
        // The property value was not specified
        //
        throw UnrecognizedConfigProperty(option);
    }

    //
    // Get the property name and value
    //
    String propertyName = option.subString(0, pos);
    String propertyValue = option.subString(pos + 1);

    return initCurrentValue(propertyName, propertyValue);
}

/**
    Initialize config property owners and add them to the property owner table
*/
void ConfigManager::_initPropertyTable()
{
    //
    // add config property and its fixed value to fixed value table
    //
    for (Uint32 i = 0; i < NUM_FIXED_PROPERTIES; i++)
    {
        _propertyTable->fixedValueTable.insert(_fixedValues[i].propertyName,
            _fixedValues[i].fixedValue);
    }

    //
    // add config property and its owner object to owners table (but only if
    // the property is not also listed in the fixed value table.
    //
    for (Uint32 i = 0; i < NUM_PROPERTIES; i++)
    {
        const char* fixedValue = 0;

        _properties[i].propertyOwner->initialize();

        if (!_propertyTable->fixedValueTable.lookup(
                _properties[i].propertyName, fixedValue))
        {
            _propertyTable->ownerTable.insert(_properties[i].propertyName,
                _properties[i].propertyOwner);
        }
        else
        {
            //
            // Set the value for the fixed properties
            //
            _properties[i].propertyOwner->initCurrentValue(
                _properties[i].propertyName, fixedValue);
        }
    }
}

Boolean ConfigManager::_fixedValueCheck(const String& name,String & value) const
{
    //
    // Check for a property with a fixed value
    //
    const char* fixedValue = 0;

    _propertyTable->fixedValueTable.lookup(name, fixedValue);

    // no fixed property 'name' in FixedPropertyTable, bail out
    if (0 == fixedValue)
    {
        return false;
    }

    // if the fixed value is set to blank, need to replace the value with
    // the system-supplied host name
    if (String::equalNoCase(name, "fullyQualifiedHostName"))
    {
        if (0 == strlen(fixedValue))
        {
            value.assign(System::getFullyQualifiedHostName());
        }
        else
        {
            value.assign(fixedValue);
            System::setFullyQualifiedHostName(value);
        }
        // returning here already to avoid the following and in this case
        // unnecessary string compare and assign
        return true;
    }
    if (String::equalNoCase(name, "hostname"))
    {
        if (0 == strlen(fixedValue))
        {
            value.assign(System::getHostName());
        }
        else
        {
            value.assign(fixedValue);
            System::setHostName(value);
        }
        // returning here already to avoid the following and in this case
        // unnecessary assign
        return true;
    }
    value.assign(fixedValue);
    return true;
}

/**
    Get Pegasus Home
*/
String ConfigManager::getPegasusHome()
{
    return _pegasusHome;
}

/**
    Set Pegasus Home variable
*/
void ConfigManager::setPegasusHome(const String& home)
{
    if (home != String::EMPTY)
    {
        _pegasusHome = home;
    }
}

/**
    Get the homed path for a given property.
*/
String ConfigManager::getHomedPath(const String& value)
{
    String homedPath;

    if (value.size() != 0 )
    {
        if (System::is_absolute_path((const char *)value.getCString()))
        {
            return value;
        }

        //
        // Get the pegasusHome and prepend it
        //

        String temp = value;
        Uint32 pos = 0;
        Uint32 token = 0;
        do
        {
            if ((pos = temp.find(FileSystem::getPathDelimiter())) ==
                    PEG_NOT_FOUND)
            {
                pos = temp.size();
                token = 0;
            }
            else
            {
                token = 1;
            }

            if (System::is_absolute_path(
                    (const char *)temp.subString(0, pos).getCString()))
            {
                homedPath.append(temp.subString(0,pos));
            }
            else
            {
                homedPath.append(_pegasusHome + "/" + temp.subString(0, pos));
            }

            if (token == 1)
            {
                homedPath.append(FileSystem::getPathDelimiter());
            }
            temp.remove(0, pos + token);
        } while (temp.size() > 0);
    }
    return homedPath;
}

Boolean ConfigManager::parseBooleanValue(const String& propertyValue)
{
    return String::equalNoCase(propertyValue, "true");
}

Boolean ConfigManager::isValidBooleanValue(const String& value)
{
    if ((String::equalNoCase(value, STRING_TRUE)) ||
        (String::equalNoCase(value, STRING_FALSE)))
    {
        return true;
    }
    return false;
}

Uint32 ConfigManager::parseUint32Value(const String& propertyValue)
{
    Uint64 v;
    StringConversion::decimalStringToUint64(
       (const char*)propertyValue.getCString(), v);
    return (Uint32) v;
}

Boolean ConfigManager::isValidUint32Value(const String& strValue,
    Uint32 min,
    Uint32 max)
{
    Uint32 v;
    Boolean rtn = StringConversion::decimalStringToUint32(strValue, v);
    if (rtn && ( (v >= min) && (v <= max)) )
    {
        return true;
    }
    return false;
}

Array<HostAddress> ConfigManager::getListenAddress(const String &propertyValue)
{
    Array<String> interfaces = DefaultPropertyOwner::parseAndGetListenAddress (
        propertyValue);

    HostAddress theAddress;
    Array<HostAddress> listenAddrs;
    for(Uint32 i = 0, n = interfaces.size(); i < n; i++)
    {
        theAddress.setHostAddress(interfaces[i]);
        listenAddrs.append(theAddress);
    }
    return listenAddrs;
}

String ConfigManager::getDynamicAttributeStatus(const String& name)
{
    //
    // get property owner object from config table
    //
    ConfigPropertyOwner* propertyOwner;
    if ( !_propertyTable->ownerTable.lookup(name, propertyOwner))
    {
        throw UnrecognizedConfigProperty(name);
    }

    Boolean _isDynamic = propertyOwner->isDynamic(name);

    MessageLoaderParms parms(
        (_isDynamic? "Config.ConfigManager.DYNAMIC":
                     "Config.ConfigManager.STATIC"),
        (_isDynamic? "Dynamic" : "Static"));

    parms.msg_src_path = "pegasus/pegasusServer";
    return MessageLoader::getMessage(parms);
}

PEGASUS_NAMESPACE_END
