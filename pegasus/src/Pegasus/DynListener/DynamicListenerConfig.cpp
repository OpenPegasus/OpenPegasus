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

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/FileSystem.h>
#include <Pegasus/Common/Tracer.h>
#include <Pegasus/Common/Exception.h>

#include <Pegasus/General/OptionManager.h>

#include "DynamicListenerConfig.h"

PEGASUS_NAMESPACE_BEGIN
PEGASUS_USING_STD;


const String LISTENER_HOME_DEFAULT  = ".";
String DynamicListenerConfig::_listenerHome = LISTENER_HOME_DEFAULT;


static struct OptionRow optionsTable[] =
//optionname defaultvalue rqd  type domain domainsize clname hlpmsg
{

{"listenerPort", "5999", false, Option::WHOLE_NUMBER, 0, 0,
 "listenerPort", "specifies system and port"},
#ifdef PEGASUS_HAS_SSL
{"enableHttpsListenerConnection", "false", false, Option::BOOLEAN, 0, 0,
 "enableHttpsListenerConnection", "specifies namespace to use for operation"},

{"sslKeyFilePath", "", false, Option::STRING, 0, 0,
 "sslKeyFilePath", "path to the listener's SSL private key"},

{"sslCertificateFilePath", "", false, Option::STRING, 0, 0,
 "sslCertificateFilePath",
 "path to the listener's SSL public key certificate."},

{"sslCipherSuite", "DEFAULT", false, Option::STRING, 0, 0,
 "sslCipherSuite", "ssl cipher value for authentication"},

{"sslBackwardCompatibility","false", false, Option::BOOLEAN, 0, 0,
 "sslBackwardCompatibility","ssl Compatiblity value to support "
 "invoking compatible api's of earlier protocols"},
#endif
{"consumerDir", "", false, Option::STRING, 0, 0,
 "consumerDir", "path to the consumer libraries"},

{"consumerConfigDir", "", false, Option::STRING, 0, 0,
 "consumerConfigDir", "path to the consumer configuration files"},

{"enableConsumerUnload", "false", false, Option::BOOLEAN, 0, 0,
 "enableConsumerUnload",
 "specifies whether the listener should unload idle consumers"},

{"consumerIdleTimeout", "300000", false, Option::WHOLE_NUMBER, 0, 0,
 "consumerIdleTimeout",
 "period of inactivity, in ms, before consumers are unloaded"},

{"shutdownTimeout", "10000", false, Option::WHOLE_NUMBER, 0, 0,
 "shutdownTimeout",
 "the length of time to wait for consumer threads to complete, in ms"},

{"traceFilePath", "cimlistener.trc", false, Option::STRING, 0, 0,
 "traceFilePath", "path to the listener's trace file"},

{"traceLevel", "0", false, Option::WHOLE_NUMBER, 0, 0,
 "traceLevel", "the level of logging detail"},

{"traceComponents", "LISTENER", false, Option::STRING, 0, 0,
 "traceComponents", "the components to trace"},

};

const Uint32 NUM_OPTIONS = sizeof(optionsTable) / sizeof(optionsTable[0]);

DynamicListenerConfig* DynamicListenerConfig::_instance = 0;

DynamicListenerConfig::DynamicListenerConfig()
{
    FileSystem::getCurrentDirectory(_listenerHome);
    FileSystem::translateSlashes(_listenerHome);
}

DynamicListenerConfig::~DynamicListenerConfig()
{
}

DynamicListenerConfig* DynamicListenerConfig::getInstance()
{
    if (!_instance)
    {
        _instance = new DynamicListenerConfig();
    }

    return _instance;
}

void DynamicListenerConfig::initOptions(const String& configFile)
{
    _optionMgr.registerOptions(optionsTable, NUM_OPTIONS);

    //do not throw an error if there's no config file; just use the defaults
    if (FileSystem::exists(configFile))
    {
        _optionMgr.mergeFile(configFile);
    }

    _optionMgr.checkRequiredOptions();
}

Boolean DynamicListenerConfig::lookupValue(
            const String& name,
            String& value) const
{
    String temp;
    if (!_optionMgr.lookupValue(name, temp))
    {
        return false;
    }

    if (String::equal(name, "consumerDir") ||
        String::equal(name, "consumerConfigDir"))
    {
        value = DynamicListenerConfig::getHomedPath(temp);

        if (!FileSystem::exists(value) ||
            !FileSystem::isDirectory(value) ||
            !FileSystem::canRead(value))
        {
            throw OMInvalidOptionValue(name, value);
        }

        // must be able to write pending requests to
        // .dat files in the config directory
        if (String::equal(name, "consumerConfigDir") &&
            !FileSystem::canWrite(value))
        {
            throw OMInvalidOptionValue(name, value);
        }

    } else if (String::equal(name, "traceFilePath"))
    {
        //a blank value is acceptable and indicates
        // that no tracing will be done
        if (String::equal(temp, ""))
        {
            value = String::EMPTY;
            return true;
        }

        value = DynamicListenerConfig::getHomedPath(temp);

        //check to make sure we can create trace file
        String path = FileSystem::extractFilePath(value);

        if (!FileSystem::exists(path) || !FileSystem::canWrite(path))
        {
            throw OMInvalidOptionValue(name, value);
        }
    }
#ifdef PEGASUS_HAS_SSL
    else if (String::equal(name, "sslKeyFilePath") ||
             String::equal(name, "sslCertificateFilePath"))
    {
        //a blank value is acceptable and is the default
        if (String::equal(temp, ""))
        {
            value = String::EMPTY;
            return true;
        }

        value = DynamicListenerConfig::getHomedPath(temp);

        if (!FileSystem::exists(value) || !FileSystem::canRead(value))
        {
            throw OMInvalidOptionValue(name, value);
        }
    }
    else if(String::equal(name, "sslCipherSuite"))
    {
        if(temp.size()== 0)
        {
            return false;
        }
        value = temp;
    } 
#endif
    else
    {
        value = temp;
    }

    return true;
}


Boolean DynamicListenerConfig::lookupIntegerValue(
            const String& name,
            Uint32& value) const
{
    if (!_optionMgr.lookupIntegerValue(name, value))
    {
        return false;
    }

    if (String::equal(name, "listenerPort"))
    {
    } else if (String::equal(name, "consumerIdleTimeout"))
    {
    } else if (String::equal(name, "shutdownTimeout"))
    {
    } else if (String::equal(name, "traceLevel"))
    {
        if (value > 4)
        {
            throw OMInvalidOptionValue(name, "");
        }
    }

    return true;
}

Boolean DynamicListenerConfig::valueEquals(
            const String& name,
            const String& value) const
{
    //not implemented yet -- do we need this in addition to lookupValue()?
    return _optionMgr.valueEquals(name, value);
}

Boolean DynamicListenerConfig::isTrue(const String& name) const
{
    //no additional checking for booleans
    return _optionMgr.isTrue(name);
}

String DynamicListenerConfig::getListenerHome()
{
    return _listenerHome;
}

//This sets the listener home to the FULL path
//We do it once here so we do not have to keep doing file operations later on
void DynamicListenerConfig::setListenerHome(const String& home)
{
    PEG_METHOD_ENTER(TRC_LISTENER, "DynamicListenerConfig::setListenerHome");

    if (System::is_absolute_path((const char *)home.getCString()))
    {
        _listenerHome = home;

    } else
    {
        String currentDir;
        FileSystem::getCurrentDirectory(currentDir);
        _listenerHome = FileSystem::getAbsolutePath(
                            (const char*)currentDir.getCString(),
                            home);
    }

    PEG_METHOD_EXIT();
}

String DynamicListenerConfig::getHomedPath(const String& value)
{
    PEG_METHOD_ENTER(TRC_LISTENER, "DynamicListenerConfig::getHomedPath()");

    String homedPath;

    if (String::equal(value, String::EMPTY))
    {
        homedPath = _listenerHome;

    } else
    {
        homedPath = FileSystem::getAbsolutePath(
                        (const char*)_listenerHome.getCString(),
                        value);
    }
    FileSystem::translateSlashes(homedPath);

    PEG_METHOD_EXIT();
    return homedPath;
}





PEGASUS_NAMESPACE_END
