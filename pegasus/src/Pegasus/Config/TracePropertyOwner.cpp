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
// This file has implementation for the trace property owner class.
//
///////////////////////////////////////////////////////////////////////////////

#include <Pegasus/Common/Tracer.h>
#include <Pegasus/Common/Logger.h>
#include <Pegasus/Common/FileSystem.h>
#include <Pegasus/Common/MessageLoader.h>
#include <Pegasus/Common/StringConversion.h>
#include "ConfigManager.h"
#include "TracePropertyOwner.h"
#include "ConfigExceptions.h"

PEGASUS_USING_STD;

PEGASUS_NAMESPACE_BEGIN

///////////////////////////////////////////////////////////////////////////////
//  TracePropertyOwner
//
//  When a new trace property is added, make sure to add the property name
//  and the default attributes of that property in the table below.
///////////////////////////////////////////////////////////////////////////////

static struct ConfigPropertyRow properties[] =
{
#ifdef PEGASUS_OS_HPUX
    {"traceLevel", "0", IS_DYNAMIC, IS_HIDDEN},
    {"traceComponents", "", IS_DYNAMIC, IS_HIDDEN},
    {"traceMemoryBufferKbytes", "10240", IS_STATIC, IS_HIDDEN},
    {"traceFacility", "File", IS_DYNAMIC, IS_HIDDEN},
    {"traceFileSizeKBytes", "1048576", IS_DYNAMIC, IS_HIDDEN},
    {"numberOfTraceFiles", "3", IS_DYNAMIC, IS_HIDDEN},
#elif defined(PEGASUS_OS_PASE)
    {"traceLevel", "0", IS_DYNAMIC, IS_VISIBLE},
    {"traceComponents", "", IS_DYNAMIC, IS_VISIBLE},
    {"traceMemoryBufferKbytes", "10240", IS_STATIC, IS_VISIBLE},
    {"traceFacility", "File", IS_DYNAMIC, IS_VISIBLE},
    {"traceFileSizeKBytes", "1048576", IS_DYNAMIC, IS_VISIBLE},
    {"numberOfTraceFiles", "3", IS_DYNAMIC, IS_VISIBLE},
#elif defined(PEGASUS_OS_ZOS)
    {"traceLevel", "2", IS_DYNAMIC, IS_VISIBLE},
    {"traceComponents", "All", IS_DYNAMIC, IS_VISIBLE},
    {"traceMemoryBufferKbytes", "10240", IS_STATIC, IS_VISIBLE},
    {"traceFacility", "Memory", IS_DYNAMIC, IS_VISIBLE},
    {"traceFileSizeKBytes", "1048576", IS_DYNAMIC, IS_VISIBLE},
    {"numberOfTraceFiles", "3", IS_DYNAMIC, IS_VISIBLE},
#else
# if defined (PEGASUS_USE_RELEASE_CONFIG_OPTIONS)
    {"traceLevel", "0", IS_DYNAMIC, IS_HIDDEN},
    {"traceComponents", "", IS_DYNAMIC, IS_HIDDEN},
    {"traceMemoryBufferKbytes", "10240", IS_STATIC, IS_HIDDEN},
    {"traceFacility", "File", IS_DYNAMIC, IS_HIDDEN},
    {"traceFileSizeKBytes", "1048576", IS_DYNAMIC, IS_HIDDEN},
    {"numberOfTraceFiles", "3", IS_DYNAMIC, IS_HIDDEN},
# else
    {"traceLevel", "0", IS_DYNAMIC, IS_VISIBLE},
    {"traceComponents", "", IS_DYNAMIC, IS_VISIBLE},
    {"traceMemoryBufferKbytes", "10240", IS_STATIC, IS_VISIBLE},
    {"traceFacility", "File", IS_DYNAMIC, IS_VISIBLE},
    {"traceFileSizeKBytes", "1048576", IS_DYNAMIC, IS_VISIBLE},
    {"numberOfTraceFiles", "3", IS_DYNAMIC, IS_VISIBLE},
# endif
#endif
#if defined(PEGASUS_OS_ZOS)
# if defined(PEGASUS_USE_RELEASE_DIRS)
    {"traceFilePath", "/tmp/cimserver.trc", IS_DYNAMIC, IS_VISIBLE},
# else
    {"traceFilePath", "cimserver.trc", IS_DYNAMIC, IS_VISIBLE},
# endif
#elif defined(PEGASUS_OS_PASE)
    {"traceFilePath", "/tmp/cimserver.trc", IS_DYNAMIC, IS_VISIBLE},
#else
    {"traceFilePath", "trace/cimserver.trc", IS_DYNAMIC, IS_VISIBLE},
#endif
};

const Uint32 NUM_PROPERTIES = sizeof(properties) / sizeof(properties[0]);

static const char TRACE_POSSIBLE_VALUE [] = "Possible Values: ";

//
// Checks if the trace level is valid
//
Boolean TracePropertyOwner::isLevelValid(const String& traceLevel) const
{
    //
    // Check if the level is valid
    //
    return (traceLevel == "0" || traceLevel == "1" ||
            traceLevel == "2" || traceLevel == "3" ||
            traceLevel == "4" || traceLevel == "5");
}

//
// Get the appropriate trace level
//
Uint32 TracePropertyOwner::getTraceLevel(const String& traceLevel)
{
    if ( traceLevel == "0")
    {
        return Tracer::LEVEL0;
    }
    else if ( traceLevel == "1" )
    {
        return Tracer::LEVEL1;
    }
    else if ( traceLevel == "2" )
    {
        return Tracer::LEVEL2;
    }
    else if ( traceLevel == "3" )
    {
        return Tracer::LEVEL3;
    }
    else if ( traceLevel == "4" )
    {
        return Tracer::LEVEL4;
    }
    else if ( traceLevel == "5" )
    {
        return Tracer::LEVEL5;
    }
    else
    {
        return Tracer::LEVEL0;
    }
}

/** Constructors  */
TracePropertyOwner::TracePropertyOwner():
   _initialized(false)
{
    _traceLevel.reset(new ConfigProperty);
    _traceFilePath.reset(new ConfigProperty);
    _traceComponents.reset(new ConfigProperty);
    _traceFacility.reset(new ConfigProperty);
    _traceMemoryBufferKbytes.reset(new ConfigProperty);
    _traceFileSizeKBytes.reset(new ConfigProperty);
    _numberOfTraceFiles.reset(new ConfigProperty);
}

/**
    Initialize the config properties.
*/
void TracePropertyOwner::initialize()
{
    //
    // Initialize the properties with default values only once !
    //
    if (_initialized)
    {
        return;
    }

    for (Uint32 i = 0; i < NUM_PROPERTIES; i++)
    {
        if (String::equal(properties[i].propertyName, "traceComponents"))
        {
            _traceComponents->propertyName = properties[i].propertyName;
            _traceComponents->defaultValue = properties[i].defaultValue;
            _traceComponents->currentValue = properties[i].defaultValue;
            _traceComponents->plannedValue = properties[i].defaultValue;
            _traceComponents->dynamic = properties[i].dynamic;
            _traceComponents->externallyVisible =
                properties[i].externallyVisible;

            Tracer::setTraceComponents(_traceComponents->defaultValue);
        }
        else if (String::equal(properties[i].propertyName, "traceLevel"))
        {
            _traceLevel->propertyName = properties[i].propertyName;
            _traceLevel->defaultValue = properties[i].defaultValue;
            _traceLevel->currentValue = properties[i].defaultValue;
            _traceLevel->plannedValue = properties[i].defaultValue;
            _traceLevel->dynamic = properties[i].dynamic;
            _traceLevel->externallyVisible =
                properties[i].externallyVisible;

            PEGASUS_ASSERT(_traceLevel->defaultValue.size()!= 0);

            Tracer::setTraceLevel(getTraceLevel(_traceLevel->defaultValue));

        }
        else if (String::equal(
                     properties[i].propertyName, "traceFilePath"))
        {
            _traceFilePath->propertyName = properties[i].propertyName;
            _traceFilePath->defaultValue = properties[i].defaultValue;
            _traceFilePath->currentValue = properties[i].defaultValue;
            _traceFilePath->plannedValue = properties[i].defaultValue;
            _traceFilePath->dynamic = properties[i].dynamic;
            _traceFilePath->externallyVisible =
                properties[i].externallyVisible;

            // set the default value in the Trace
            Uint32 retCode = Tracer::setTraceFile(ConfigManager::getHomedPath(
                _traceFilePath->defaultValue).getCString());
            if ( retCode == 1 )
            {
                Logger::put_l(
                    Logger::ERROR_LOG,System::CIMSERVER,Logger::WARNING,
                    MessageLoaderParms(
                    "Config.TracePropertyOwner.UNABLE_TO_WRITE_TRACE_FILE",
                    "Unable to write to trace file $0",
                    _traceFilePath->defaultValue));
                _traceFilePath->currentValue.clear();
            }

        }
        else if (String::equal(
                     properties[i].propertyName, "traceMemoryBufferKbytes"))
        {
            _traceMemoryBufferKbytes->propertyName = properties[i].propertyName;
            _traceMemoryBufferKbytes->defaultValue = properties[i].defaultValue;
            _traceMemoryBufferKbytes->currentValue = properties[i].defaultValue;
            _traceMemoryBufferKbytes->plannedValue = properties[i].defaultValue;
            _traceMemoryBufferKbytes->dynamic = properties[i].dynamic;
            _traceMemoryBufferKbytes->externallyVisible =
                properties[i].externallyVisible;

            PEGASUS_ASSERT(_traceMemoryBufferKbytes->defaultValue.size()!= 0);

            Uint32 bufferSize;
            StringConversion::decimalStringToUint32(
                _traceMemoryBufferKbytes->defaultValue, bufferSize );
            Tracer::setTraceMemoryBufferSize(bufferSize);

        }
        else if (String::equal(
                     properties[i].propertyName, "traceFacility"))
        {
            _traceFacility->propertyName = properties[i].propertyName;
            _traceFacility->defaultValue = properties[i].defaultValue;
            _traceFacility->currentValue = properties[i].defaultValue;
            _traceFacility->plannedValue = properties[i].defaultValue;
            _traceFacility->dynamic = properties[i].dynamic;
            _traceFacility->externallyVisible =
                properties[i].externallyVisible;

            PEGASUS_ASSERT(_traceFacility->defaultValue.size()!= 0);

            Tracer::setTraceFacility(_traceFacility->defaultValue);
        }
        else if (String::equalNoCase(
                       properties[i].propertyName, "traceFileSizeKBytes"))
        {
            _traceFileSizeKBytes->propertyName = properties[i].propertyName;
            _traceFileSizeKBytes->defaultValue = properties[i].defaultValue;
            _traceFileSizeKBytes->currentValue = properties[i].defaultValue;
            _traceFileSizeKBytes->plannedValue = properties[i].defaultValue;
            _traceFileSizeKBytes->dynamic = properties[i].dynamic;
            _traceFileSizeKBytes->externallyVisible =
                   properties[i].externallyVisible;

            String value = _traceFileSizeKBytes->defaultValue;
            PEGASUS_ASSERT(value.size());

            Tracer::setMaxTraceFileSize(value);
        }
        else if (String::equalNoCase(
                          properties[i].propertyName, "numberOfTraceFiles"))
        {
            _numberOfTraceFiles->propertyName = properties[i].propertyName;
            _numberOfTraceFiles->defaultValue = properties[i].defaultValue;
            _numberOfTraceFiles->currentValue = properties[i].defaultValue;
            _numberOfTraceFiles->plannedValue = properties[i].defaultValue;
            _numberOfTraceFiles->dynamic = properties[i].dynamic;
            _numberOfTraceFiles->externallyVisible =
                     properties[i].externallyVisible;

            PEGASUS_ASSERT(_numberOfTraceFiles->defaultValue.size()!= 0);

            Tracer::setMaxTraceFileNumber(_numberOfTraceFiles->currentValue);

        }
    }
    _initialized = true;
}

struct ConfigProperty* TracePropertyOwner::_lookupConfigProperty(
    const String& name) const
{
    if (String::equal(_traceComponents->propertyName, name))
    {
        return _traceComponents.get();
    }
    else if (String::equal(_traceLevel->propertyName, name))
    {
        return _traceLevel.get();
    }
    else if (String::equal(_traceFilePath->propertyName, name))
    {
        return _traceFilePath.get();
    }
    else if (String::equal(_traceFacility->propertyName, name))
    {
        return _traceFacility.get();
    }
    else if (String::equal(_traceMemoryBufferKbytes->propertyName, name))
    {
        return _traceMemoryBufferKbytes.get();
    }
    else if (String::equalNoCase(_traceFileSizeKBytes->propertyName, name))
    {
        return _traceFileSizeKBytes.get();
    }
    else if (String::equalNoCase(_numberOfTraceFiles->propertyName, name))
    {
        return _numberOfTraceFiles.get();
    }
    else
    {
        throw UnrecognizedConfigProperty(name);
    }
}

/**
    Get information about the specified property.
*/
void TracePropertyOwner::getPropertyInfo(
    const String& name,
    Array<String>& propertyInfo) const
{
    struct ConfigProperty * configProperty = _lookupConfigProperty(name);

    buildPropertyInfo(name, configProperty, propertyInfo);
}

/*
    Get supplementary help information.
*/
String TracePropertyOwner::getPropertyHelpSupplement(
    const String& name) const
{
    String localPropertyInfo;
    // Used to get list of trace components from the Tracer itself
    // for the Possible values list. Note that this does not show up
    // in the message bundle.
    if (String::equalNoCase(_traceComponents->propertyName, name))
    {
       // Set list of possible traceComponent Strings
        const Uint32 _NUM_COMPONENTS = Tracer::_NUM_COMPONENTS;
        localPropertyInfo.append("\n");

        localPropertyInfo.append(loadMessage(
            "Config.TracePropertyOwner.TRACECOMPONENTS_POSSIBLE_VALUE",
            TRACE_POSSIBLE_VALUE));

        String possibleValues = "\n    ALL ";
        Uint32 lineSize = possibleValues.size();
        // Get the list of traceComponents from Tracer and append to formatted
        // string
        for ( Uint32 index = 0;index < _NUM_COMPONENTS; index++)
        {
            if ((strlen(Tracer::TRACE_COMPONENT_LIST[index]) + lineSize) >= 79)
            {
                possibleValues.append("\n    ");
                lineSize = 5;
            }
            possibleValues.append(Tracer::TRACE_COMPONENT_LIST[index]);
            possibleValues.append(" ");

            lineSize += (strlen(Tracer::TRACE_COMPONENT_LIST[index]) + 1);
        }
        possibleValues = possibleValues.subString(0, possibleValues.size() - 1);
        localPropertyInfo.append(possibleValues);
    }
    return localPropertyInfo;
}

/**
    Get default value of the specified property.
*/
String TracePropertyOwner::getDefaultValue(const String& name) const
{
    struct ConfigProperty * configProperty = _lookupConfigProperty(name);
    return configProperty->defaultValue;
}

/**
    Get current value of the specified property.
*/
String TracePropertyOwner::getCurrentValue(const String& name) const
{
    struct ConfigProperty * configProperty = _lookupConfigProperty(name);
    return configProperty->currentValue;
}

/**
    Get planned value of the specified property.
*/
String TracePropertyOwner::getPlannedValue(const String& name) const
{
    struct ConfigProperty * configProperty = _lookupConfigProperty(name);
    return configProperty->plannedValue;
}

/**
    Init current value of the specified property to the specified value.
*/
void TracePropertyOwner::initCurrentValue(
    const String& name,
    const String& value)
{
    if (String::equal(_traceComponents->propertyName, name))
    {
        _traceComponents->currentValue = value;
        Tracer::setTraceComponents(_traceComponents->currentValue);
    }
    else if (String::equal(_traceLevel->propertyName, name))
    {
        _traceLevel->currentValue = value;
        Uint32 traceLevel = getTraceLevel(_traceLevel->currentValue);
        Tracer::setTraceLevel(traceLevel);
    }
    else if (String::equal(_traceFilePath->propertyName, name))
    {
        _traceFilePath->currentValue = value;
        Uint32 retCode = Tracer::setTraceFile(ConfigManager::getHomedPath(
            _traceFilePath->currentValue).getCString());
        if ( retCode == 1 )
        {
            Logger::put_l(
                Logger::ERROR_LOG,System::CIMSERVER,Logger::WARNING,
                MessageLoaderParms(
                "Config.TracePropertyOwner.UNABLE_TO_WRITE_TRACE_FILE",
                "Unable to write to trace file $0",
                _traceFilePath->currentValue));
            _traceFilePath->currentValue.clear();
        }
    }
    else if (String::equal(_traceFacility->propertyName, name))
    {
        _traceFacility->currentValue = value;
        //set trace facility
        Tracer::setTraceFacility(value);

        //should take effect only when the tracing is on "File"
        Tracer::setMaxTraceFileSize(_traceFileSizeKBytes->currentValue);
        Tracer::setMaxTraceFileNumber(value);
    }
    else if (String::equal(_traceMemoryBufferKbytes->propertyName, name))
    {
        Uint32 bufferSize;

        _traceMemoryBufferKbytes->currentValue = value;

        // KS_TODO confirm that we can ignore the error return
        StringConversion::decimalStringToUint32( value, bufferSize );
        Tracer::setTraceMemoryBufferSize(bufferSize);
    }
    else if (String::equalNoCase(_traceFileSizeKBytes->propertyName,name))
    {
        _traceFileSizeKBytes->currentValue = value;
        Tracer::setMaxTraceFileSize(value);

    }
    else if (String::equalNoCase(_numberOfTraceFiles->propertyName,name))
    {
         _numberOfTraceFiles->currentValue = value;
         Tracer::setMaxTraceFileNumber(value);
    }
    else
    {
        throw UnrecognizedConfigProperty(name);
    }
}


/**
    Init planned value of the specified property to the specified value.
*/
void TracePropertyOwner::initPlannedValue(
    const String& name,
    const String& value)
{
    struct ConfigProperty* configProperty = _lookupConfigProperty(name);
    configProperty->plannedValue = value;
}

/**
    Update current value of the specified property to the specified value.
*/
void TracePropertyOwner::updateCurrentValue(
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
    // Update does the same thing as initialization
    //
    initCurrentValue(name, value);
}


/**
    Update planned value of the specified property to the specified value.
*/
void TracePropertyOwner::updatePlannedValue(
    const String& name,
    const String& value)
{
    struct ConfigProperty* configProperty = _lookupConfigProperty(name);
    configProperty->plannedValue = value;
}

/**
    Checks to see if the given value is valid or not.
*/
Boolean TracePropertyOwner::isValid(
    const String& name,
    const String& value) const
{
    if (String::equal(_traceComponents->propertyName, name))
    {
        String newValue = value;
        String invalidComponents;

        //
        // Check if the trace components are valid
        //
        if (!Tracer::isValidComponents(newValue,invalidComponents))
        {
            throw InvalidPropertyValue(name, invalidComponents);
        }

        return true;
    }
    else if (String::equal(_traceLevel->propertyName, name))
    {
        //
        // Check if the level is valid
        //
        if ( isLevelValid( value ) )
        {
            return true;
        }
        else
        {
            throw InvalidPropertyValue(name, value);
        }
    }
    else if (String::equal(_traceFilePath->propertyName, name))
    {
        //
        // Check if the file path is valid.  An empty string is currently
        // considered a valid value; the traceFilePath is set to the empty
        // string when a trace file cannot be opened successfully.
        //
        if ((value != String::EMPTY) &&
            !Tracer::isValidFileName((const char*)value.getCString()))
        {
            throw InvalidPropertyValue(name, value);
        }
        return true;
    }
    else if (String::equal(_traceFacility->propertyName, name))
    {
        //
        // Check if the trace facility is valid
        //
        if (!Tracer::isValidTraceFacility(value))
        {
            throw InvalidPropertyValue(name, value);
        }
        return true;
    }
    else if (String::equal(_traceMemoryBufferKbytes->propertyName, name))
    {
        //
        // Ckeck if the trace memeory buffer size is valid
        //
        if (!ConfigManager::isValidUint32Value(value,
            PEGASUS_TRC_BUFFER_MIN_SIZE_KB,
            PEGASUS_TRC_BUFFER_MAX_SIZE_KB))
        {
             throw InvalidPropertyValue(name, value);
        }
        return true;
    }
    else if (String::equal(_traceFileSizeKBytes->propertyName,name))
    {
        const Uint32 minimumFileSizeKBytes = 10240;
        const Uint32 maximumFileSizeKBytes = 2097152;

        if (!ConfigManager::isValidUint32Value(value,
            minimumFileSizeKBytes,
            maximumFileSizeKBytes))
        {
             throw InvalidPropertyValue(name, value);
        }
        return true;
    }
    else if (String::equal(_numberOfTraceFiles->propertyName,name))
    {
        const Uint32 minimumNumberOfTraceFiles = 3;
        const Uint32 maximumNumberOfTraceFiles = 20;

        if (!ConfigManager::isValidUint32Value(value,
            minimumNumberOfTraceFiles,
            maximumNumberOfTraceFiles))
        {
             throw InvalidPropertyValue(name, value);
        }
        return true;
    }
    else
    {
        throw UnrecognizedConfigProperty(name);
    }
}

/**
    Checks to see if the specified property is dynamic or not.
*/
Boolean TracePropertyOwner::isDynamic(const String& name) const
{
    struct ConfigProperty* configProperty = _lookupConfigProperty(name);
    return (configProperty->dynamic == IS_DYNAMIC);
}

PEGASUS_NAMESPACE_END
