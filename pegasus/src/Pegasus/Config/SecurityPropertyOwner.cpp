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
// This file has implementation for the security property owner class.
//
///////////////////////////////////////////////////////////////////////////////

#include "ConfigManager.h"
#include "SecurityPropertyOwner.h"
#include <Pegasus/Common/FileSystem.h>
#include <Pegasus/Common/System.h>
#include "ConfigExceptions.h"
#include <Pegasus/Common/StringConversion.h>

PEGASUS_USING_STD;

PEGASUS_NAMESPACE_BEGIN

/**
 * The Server message resource name
 */

//// static const char * SSL_POSSIBLE_VALUE_KEY =
////        "Config.SecurityPropertyOwner.SSLClientVerification_POSSIBLE_VALUE";

///////////////////////////////////////////////////////////////////////////////
//  SecurityPropertyOwner
///////////////////////////////////////////////////////////////////////////////

static struct ConfigPropertyRow properties[] =
{
///////////////////////////////////////////////////////////////////////////////
//  For z/OS, the follwing securtiy properties are fixed properties due to
//  listed in the FixedPropertyTablezOS.h . Because they are fixed, they
//  are hidden also:
//
//   enableAuthentication
//   passwordFilePath
//   sslCertificateFilePath
//   sslKeyFilePath
//   sslTrustStore
//   crlStore
//   sslClientVerificationMode
//   sslTrustStoreUserName
//   enableNamespaceAuthorization
//   kerberosServiceName
//   enableSubscriptionsForNonprivilegedUsers
//   authorizedUserGroups
///////////////////////////////////////////////////////////////////////////////
#ifdef PEGASUS_OS_PASE
    {"enableAuthentication", "true", IS_STATIC, IS_VISIBLE},
    {"httpAuthType", "Basic", IS_STATIC, IS_VISIBLE},
    {"passwordFilePath", "cimserver.passwd", IS_STATIC, IS_VISIBLE},
    {"sslCertificateFilePath", "ssl/keystore/servercert.pem", IS_STATIC,
        IS_VISIBLE},
    {"sslKeyFilePath", "ssl/keystore/serverkey.pem", IS_STATIC, IS_VISIBLE},
    {"sslTrustStore", "ssl/truststore/", IS_STATIC, IS_VISIBLE},
# ifdef PEGASUS_ENABLE_SSL_CRL_VERIFICATION
    {"crlStore", "ssl/crlstore/", IS_STATIC, IS_VISIBLE},
# endif
    {"sslClientVerificationMode", "optional", IS_STATIC, IS_VISIBLE},
    {"sslTrustStoreUserName", "QYCMCIMOM", IS_STATIC, IS_VISIBLE},
    {"enableNamespaceAuthorization", "false", IS_STATIC, IS_VISIBLE},
    {"sslBackwardCompatibility","false", IS_STATIC, IS_VISIBLE},
    {"enableSubscriptionsForNonprivilegedUsers", "false", IS_STATIC,
        IS_VISIBLE},
    {"enableRemotePrivilegedUserAccess", "true", IS_STATIC, IS_VISIBLE},
    {"authorizedUserGroups", "", IS_STATIC, IS_VISIBLE},
#ifdef PEGASUS_ENABLE_SESSION_COOKIES
    {"httpSessionTimeout", "0", IS_DYNAMIC, IS_VISIBLE},
#endif
#ifdef PEGASUS_NEGOTIATE_AUTHENTICATION
    {"mapToLocalName", "false", IS_STATIC, IS_VISIBLE},
#endif
#else // PEGASUS_OS_PASE
    {"enableAuthentication", "false", IS_STATIC, IS_VISIBLE},
    {"httpAuthType", "Basic", IS_STATIC, IS_VISIBLE},
    {"passwordFilePath", "cimserver.passwd", IS_STATIC, IS_VISIBLE},
#ifdef PEGASUS_OS_HPUX
    {"sslCertificateFilePath", "cert.pem", IS_STATIC, IS_VISIBLE},
#else
    {"sslCertificateFilePath", "server.pem", IS_STATIC, IS_VISIBLE},
#endif
    {"sslKeyFilePath", "file.pem", IS_STATIC, IS_VISIBLE},
    {"sslTrustStore", "cimserver_trust", IS_STATIC, IS_VISIBLE},
    {"sslBackwardCompatibility","false", IS_STATIC, IS_VISIBLE},
#ifdef PEGASUS_ENABLE_SSL_CRL_VERIFICATION
    {"crlStore", "crl", IS_STATIC, IS_VISIBLE},
#endif
    {"sslClientVerificationMode", "disabled", IS_STATIC, IS_VISIBLE},
    {"sslTrustStoreUserName", "", IS_STATIC, IS_VISIBLE},
    {"enableNamespaceAuthorization", "false", IS_STATIC, IS_VISIBLE},
#if defined(PEGASUS_OS_HPUX) || defined(PEGASUS_OS_LINUX)
# ifdef PEGASUS_USE_RELEASE_CONFIG_OPTIONS
    {"enableSubscriptionsForNonprivilegedUsers",
         "false", IS_STATIC, IS_VISIBLE},
# else
    {"enableSubscriptionsForNonprivilegedUsers", "true", IS_STATIC, IS_VISIBLE},
# endif
#else
    {"enableSubscriptionsForNonprivilegedUsers", "true", IS_STATIC, IS_HIDDEN},
#endif
#ifdef PEGASUS_OS_ZOS
    {"enableRemotePrivilegedUserAccess", "false", IS_STATIC, IS_VISIBLE},
    {"enableCFZAPPLID", "true", IS_STATIC, IS_VISIBLE},
#else
    {"enableRemotePrivilegedUserAccess", "true", IS_STATIC, IS_VISIBLE},
#endif
#ifdef PEGASUS_ENABLE_USERGROUP_AUTHORIZATION
    {"authorizedUserGroups", "", IS_STATIC, IS_VISIBLE},
#endif
    {"sslCipherSuite", "DEFAULT", IS_STATIC, IS_VISIBLE}
#ifdef PEGASUS_ENABLE_SESSION_COOKIES
    ,{"httpSessionTimeout", "0", IS_DYNAMIC, IS_VISIBLE}
#endif
#ifdef PEGASUS_NEGOTIATE_AUTHENTICATION
    ,{"mapToLocalName", "false", IS_STATIC, IS_VISIBLE},
#endif
#endif
};

const Uint32 NUM_PROPERTIES = sizeof(properties) / sizeof(properties[0]);


/** Constructors  */
SecurityPropertyOwner::SecurityPropertyOwner()
{
    _enableAuthentication.reset(new ConfigProperty());
    _enableNamespaceAuthorization.reset(new ConfigProperty());
    _httpAuthType.reset(new ConfigProperty());
    _passwordFilePath.reset(new ConfigProperty());
    _certificateFilePath.reset(new ConfigProperty());
    _sslBackwardCompatibility.reset(new ConfigProperty());
    _keyFilePath.reset(new ConfigProperty());
    _trustStore.reset(new ConfigProperty());
#ifdef PEGASUS_ENABLE_SSL_CRL_VERIFICATION
    _crlStore.reset(new ConfigProperty());
#endif
    _sslClientVerificationMode.reset(new ConfigProperty());
    _sslTrustStoreUserName.reset(new ConfigProperty());
    _enableRemotePrivilegedUserAccess.reset(new ConfigProperty());
    _enableSubscriptionsForNonprivilegedUsers.reset(new ConfigProperty());
#ifdef PEGASUS_ENABLE_USERGROUP_AUTHORIZATION
    _authorizedUserGroups.reset(new ConfigProperty());
#endif
#ifdef PEGASUS_OS_ZOS
    _enableCFZAPPLID.reset(new ConfigProperty());
#endif
    _cipherSuite.reset(new ConfigProperty());
#ifdef PEGASUS_ENABLE_SESSION_COOKIES
    _httpSessionTimeout.reset(new ConfigProperty());
#endif
#ifdef PEGASUS_NEGOTIATE_AUTHENTICATION
    _mapToLocalName.reset(new ConfigProperty());
#endif
}


/**
    Initialize the config properties.
*/
void SecurityPropertyOwner::initialize()
{
    for (Uint32 i = 0; i < NUM_PROPERTIES; i++)
    {
        //
        // Initialize the properties with default values
        //
        if (String::equal(
            properties[i].propertyName, "enableAuthentication"))
        {
            _enableAuthentication->propertyName = properties[i].propertyName;
            _enableAuthentication->defaultValue = properties[i].defaultValue;
            _enableAuthentication->currentValue = properties[i].defaultValue;
            _enableAuthentication->plannedValue = properties[i].defaultValue;
            _enableAuthentication->dynamic = properties[i].dynamic;
            _enableAuthentication->externallyVisible =
                properties[i].externallyVisible;
        }
        else if (String::equal(
            properties[i].propertyName, "enableNamespaceAuthorization"))
        {
            _enableNamespaceAuthorization->propertyName =
                properties[i].propertyName;
            _enableNamespaceAuthorization->defaultValue =
                properties[i].defaultValue;
            _enableNamespaceAuthorization->currentValue =
                properties[i].defaultValue;
            _enableNamespaceAuthorization->plannedValue =
                properties[i].defaultValue;
            _enableNamespaceAuthorization->dynamic = properties[i].dynamic;
            _enableNamespaceAuthorization->externallyVisible =
                properties[i].externallyVisible;
        }
        else if (String::equal(
                     properties[i].propertyName, "httpAuthType"))
        {
            _httpAuthType->propertyName = properties[i].propertyName;
            _httpAuthType->defaultValue = properties[i].defaultValue;
            _httpAuthType->currentValue = properties[i].defaultValue;
            _httpAuthType->plannedValue = properties[i].defaultValue;
            _httpAuthType->dynamic = properties[i].dynamic;
            _httpAuthType->externallyVisible = properties[i].externallyVisible;
        }
        else if (String::equal(
                     properties[i].propertyName, "passwordFilePath"))
        {
            _passwordFilePath->propertyName = properties[i].propertyName;
            _passwordFilePath->defaultValue = properties[i].defaultValue;
            _passwordFilePath->currentValue = properties[i].defaultValue;
            _passwordFilePath->plannedValue = properties[i].defaultValue;
            _passwordFilePath->dynamic = properties[i].dynamic;
            _passwordFilePath->externallyVisible =
                properties[i].externallyVisible;
        }
        else if (String::equal(
                     properties[i].propertyName, "sslCertificateFilePath"))
        {
            _certificateFilePath->propertyName = properties[i].propertyName;
            _certificateFilePath->defaultValue = properties[i].defaultValue;
            _certificateFilePath->currentValue = properties[i].defaultValue;
            _certificateFilePath->plannedValue = properties[i].defaultValue;
            _certificateFilePath->dynamic = properties[i].dynamic;
            _certificateFilePath->externallyVisible =
                properties[i].externallyVisible;
        }
        else if (String::equal(
                     properties[i].propertyName, "sslBackwardCompatibility"))
        {
            _sslBackwardCompatibility->propertyName = 
                properties[i].propertyName;
            _sslBackwardCompatibility->defaultValue =
                properties[i].defaultValue;
            _sslBackwardCompatibility->currentValue = 
                properties[i].defaultValue;
            _sslBackwardCompatibility->plannedValue = 
                properties[i].defaultValue;
            _sslBackwardCompatibility->dynamic = 
                properties[i].dynamic;
            _sslBackwardCompatibility->externallyVisible =
                properties[i].externallyVisible;
        }
        else if (String::equal(
                     properties[i].propertyName, "sslKeyFilePath"))
        {
            _keyFilePath->propertyName = properties[i].propertyName;
            _keyFilePath->defaultValue = properties[i].defaultValue;
            _keyFilePath->currentValue = properties[i].defaultValue;
            _keyFilePath->plannedValue = properties[i].defaultValue;
            _keyFilePath->dynamic = properties[i].dynamic;
            _keyFilePath->externallyVisible = properties[i].externallyVisible;
        }
        else if (String::equal(
                     properties[i].propertyName, "sslTrustStore"))
        {
            _trustStore->propertyName = properties[i].propertyName;
            _trustStore->defaultValue = properties[i].defaultValue;
            _trustStore->currentValue = properties[i].defaultValue;
            _trustStore->plannedValue = properties[i].defaultValue;
            _trustStore->dynamic = properties[i].dynamic;
            _trustStore->externallyVisible = properties[i].externallyVisible;

            // do not initialize trustpath; a truststore is not required
            // for SSL handshakes a server may wish to connect on HTTPS
            // but not verify clients
        }
#ifdef PEGASUS_ENABLE_SSL_CRL_VERIFICATION
        else if (String::equal(
                     properties[i].propertyName, "crlStore"))
        {
            _crlStore->propertyName = properties[i].propertyName;
            _crlStore->defaultValue = properties[i].defaultValue;
            _crlStore->currentValue = properties[i].defaultValue;
            _crlStore->plannedValue = properties[i].defaultValue;
            _crlStore->dynamic = properties[i].dynamic;
            _crlStore->externallyVisible = properties[i].externallyVisible;

        }
#endif
        else if (String::equal(
            properties[i].propertyName, "sslClientVerificationMode"))
        {
            _sslClientVerificationMode->propertyName =
                properties[i].propertyName;
            _sslClientVerificationMode->defaultValue =
                properties[i].defaultValue;
            _sslClientVerificationMode->currentValue =
                properties[i].defaultValue;
            _sslClientVerificationMode->plannedValue =
                properties[i].defaultValue;
            _sslClientVerificationMode->dynamic = properties[i].dynamic;
            _sslClientVerificationMode->externallyVisible =
                properties[i].externallyVisible;
        }
        else if (String::equal(
            properties[i].propertyName, "sslTrustStoreUserName"))
        {
            _sslTrustStoreUserName->propertyName = properties[i].propertyName;
            _sslTrustStoreUserName->defaultValue = properties[i].defaultValue;
            _sslTrustStoreUserName->currentValue = properties[i].defaultValue;
            _sslTrustStoreUserName->plannedValue = properties[i].defaultValue;
            _sslTrustStoreUserName->dynamic = properties[i].dynamic;
            _sslTrustStoreUserName->externallyVisible =
                properties[i].externallyVisible;
        }
        else if (String::equal(
            properties[i].propertyName, "enableRemotePrivilegedUserAccess"))
        {
            _enableRemotePrivilegedUserAccess->propertyName =
                properties[i].propertyName;
            _enableRemotePrivilegedUserAccess->defaultValue =
                properties[i].defaultValue;
            _enableRemotePrivilegedUserAccess->currentValue =
                properties[i].defaultValue;
            _enableRemotePrivilegedUserAccess->plannedValue =
                properties[i].defaultValue;
            _enableRemotePrivilegedUserAccess->dynamic = properties[i].dynamic;
            _enableRemotePrivilegedUserAccess->externallyVisible =
                properties[i].externallyVisible;
        }
        else if (String::equal(properties[i].propertyName,
                     "enableSubscriptionsForNonprivilegedUsers"))
        {
            _enableSubscriptionsForNonprivilegedUsers->propertyName =
                properties[i].propertyName;
            _enableSubscriptionsForNonprivilegedUsers->defaultValue =
                properties[i].defaultValue;
            _enableSubscriptionsForNonprivilegedUsers->currentValue =
                properties[i].defaultValue;
            _enableSubscriptionsForNonprivilegedUsers->plannedValue =
                properties[i].defaultValue;
            _enableSubscriptionsForNonprivilegedUsers->dynamic =
                properties[i].dynamic;
            _enableSubscriptionsForNonprivilegedUsers->externallyVisible =
                properties[i].externallyVisible;
        }
#ifdef PEGASUS_ENABLE_USERGROUP_AUTHORIZATION
        else if (String::equal(
                     properties[i].propertyName, "authorizedUserGroups"))
        {
            _authorizedUserGroups->propertyName = properties[i].propertyName;
            _authorizedUserGroups->defaultValue = properties[i].defaultValue;
            _authorizedUserGroups->currentValue = properties[i].defaultValue;
            _authorizedUserGroups->plannedValue = properties[i].defaultValue;
            _authorizedUserGroups->dynamic = properties[i].dynamic;
            _authorizedUserGroups->externallyVisible =
                properties[i].externallyVisible;
        }
#endif
#ifdef PEGASUS_OS_ZOS
        else if (String::equal(
                     properties[i].propertyName, "enableCFZAPPLID"))
        {
            _enableCFZAPPLID->propertyName = properties[i].propertyName;
            _enableCFZAPPLID->defaultValue = properties[i].defaultValue;
            _enableCFZAPPLID->currentValue = properties[i].defaultValue;
            _enableCFZAPPLID->plannedValue = properties[i].defaultValue;
            _enableCFZAPPLID->dynamic = properties[i].dynamic;
            _enableCFZAPPLID->externallyVisible =
                properties[i].externallyVisible;
        }
#endif
#ifdef PEGASUS_NEGOTIATE_AUTHENTICATION
        else if (String::equal(
                     properties[i].propertyName, "mapToLocalName"))
        {
            _mapToLocalName->propertyName = properties[i].propertyName;
            _mapToLocalName->defaultValue = properties[i].defaultValue;
            _mapToLocalName->currentValue = properties[i].defaultValue;
            _mapToLocalName->plannedValue = properties[i].defaultValue;
            _mapToLocalName->dynamic = properties[i].dynamic;
            _mapToLocalName->externallyVisible =
                properties[i].externallyVisible;
        }
#endif
        else if (String::equal(
                     properties[i].propertyName, "sslCipherSuite"))
        {
            _cipherSuite->propertyName = properties[i].propertyName;
            _cipherSuite->defaultValue = properties[i].defaultValue;
            _cipherSuite->currentValue = properties[i].defaultValue;
            _cipherSuite->plannedValue = properties[i].defaultValue;
            _cipherSuite->dynamic = properties[i].dynamic;
            _cipherSuite->externallyVisible =
                properties[i].externallyVisible;
        }
#ifdef PEGASUS_ENABLE_SESSION_COOKIES
        else if (String::equal(
                     properties[i].propertyName, "httpSessionTimeout"))
        {
            _httpSessionTimeout->propertyName = properties[i].propertyName;
            _httpSessionTimeout->defaultValue = properties[i].defaultValue;
            _httpSessionTimeout->currentValue = properties[i].defaultValue;
            _httpSessionTimeout->plannedValue = properties[i].defaultValue;
            _httpSessionTimeout->dynamic = properties[i].dynamic;
            _httpSessionTimeout->externallyVisible =
                    properties[i].externallyVisible;
        }
#endif
    }

}

struct ConfigProperty* SecurityPropertyOwner::_lookupConfigProperty(
    const String& name) const
{
    if (String::equal(_enableAuthentication->propertyName, name))
    {
        return _enableAuthentication.get();
    }
    else if (String::equal(
                 _enableNamespaceAuthorization->propertyName, name))
    {
        return _enableNamespaceAuthorization.get();
    }
    else if (String::equal(_httpAuthType->propertyName, name))
    {
        return _httpAuthType.get();
    }
    else if (String::equal(_passwordFilePath->propertyName, name))
    {
        return _passwordFilePath.get();
    }
    else if (String::equal(_certificateFilePath->propertyName, name))
    {
        return _certificateFilePath.get();
    }
    else if (String::equal(_sslBackwardCompatibility->propertyName, name))
    {
        return _sslBackwardCompatibility.get();
    }
    else if (String::equal(_keyFilePath->propertyName, name))
    {
        return _keyFilePath.get();
    }
    else if (String::equal(_trustStore->propertyName, name))
    {
        return _trustStore.get();
    }
#ifdef PEGASUS_ENABLE_SSL_CRL_VERIFICATION
    else if (String::equal(_crlStore->propertyName, name))
    {
        return _crlStore.get();
    }
#endif
    else if (String::equal(
                 _sslClientVerificationMode->propertyName, name))
    {
        return _sslClientVerificationMode.get();
    }
    else if (String::equal(
                 _sslTrustStoreUserName->propertyName, name))
    {
        return _sslTrustStoreUserName.get();
    }
    else if (String::equal(
                 _enableRemotePrivilegedUserAccess->propertyName, name))
    {
        return _enableRemotePrivilegedUserAccess.get();
    }
    else if (String::equal(
                 _enableSubscriptionsForNonprivilegedUsers->propertyName, name))
    {
        return _enableSubscriptionsForNonprivilegedUsers.get();
    }
#ifdef PEGASUS_ENABLE_USERGROUP_AUTHORIZATION
    else if (String::equal(_authorizedUserGroups->propertyName, name))
    {
        return _authorizedUserGroups.get();
    }
#endif
#ifdef PEGASUS_OS_ZOS
    else if (String::equal(_enableCFZAPPLID->propertyName, name))
    {
        return _enableCFZAPPLID.get();
    }
#endif
#ifdef PEGASUS_NEGOTIATE_AUTHENTICATION
    else if (String::equal(_mapToLocalName->propertyName, name))
    {
        return _mapToLocalName.get();
    }
#endif
    else if (String::equal(_cipherSuite->propertyName, name))
    {
        return _cipherSuite.get();
    }
#ifdef PEGASUS_ENABLE_SESSION_COOKIES
    else if (String::equal(_httpSessionTimeout->propertyName, name))
    {
        return _httpSessionTimeout.get();
    }
#endif
    else
    {
        throw UnrecognizedConfigProperty(name);
    }
}

/**
    Get information about the specified property.
*/
void SecurityPropertyOwner::getPropertyInfo(
    const String& name,
    Array<String>& propertyInfo) const
{
    struct ConfigProperty * configProperty = _lookupConfigProperty(name);

    buildPropertyInfo(name, configProperty, propertyInfo);
}


/**
    Get default value of the specified property.
*/
String SecurityPropertyOwner::getDefaultValue(const String& name) const
{
    struct ConfigProperty * configProperty = _lookupConfigProperty(name);
    return configProperty->defaultValue;
}

/**
    Get current value of the specified property.
*/
String SecurityPropertyOwner::getCurrentValue(const String& name) const
{
    struct ConfigProperty * configProperty = _lookupConfigProperty(name);
    return configProperty->currentValue;
}

/**
    Get planned value of the specified property.
*/
String SecurityPropertyOwner::getPlannedValue(const String& name) const
{
    struct ConfigProperty * configProperty = _lookupConfigProperty(name);
    return configProperty->plannedValue;
}


/**
    Init current value of the specified property to the specified value.
*/
void SecurityPropertyOwner::initCurrentValue(
    const String& name,
    const String& value)
{
    struct ConfigProperty* configProperty = _lookupConfigProperty(name);
    configProperty->currentValue = value;
}


/**
    Init planned value of the specified property to the specified value.
*/
void SecurityPropertyOwner::initPlannedValue(
    const String& name,
    const String& value)
{
    struct ConfigProperty* configProperty = _lookupConfigProperty(name);
    configProperty->plannedValue = value;
}

/**
    Update current value of the specified property to the specified value.
*/
void SecurityPropertyOwner::updateCurrentValue(
    const String& name,
    const String& value,
    const String& userName,
    Uint32 timeoutSeconds)
{
    struct ConfigProperty* configProperty =_lookupConfigProperty(name);

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
void SecurityPropertyOwner::updatePlannedValue(
    const String& name,
    const String& value)
{
    struct ConfigProperty* configProperty = _lookupConfigProperty(name);
    configProperty->plannedValue = value;
}

/**
    Checks to see if the given value is valid or not.
*/
Boolean SecurityPropertyOwner::isValid(
    const String& name,
    const String& value) const
{
    Boolean retVal = false;

    //
    // Validate the specified value
    //
    if (String::equal(_enableAuthentication->propertyName, name) ||
        String::equal(
            _enableNamespaceAuthorization->propertyName, name) ||
        String::equal(
            _enableRemotePrivilegedUserAccess->propertyName, name) ||
        String::equal(
            _enableSubscriptionsForNonprivilegedUsers->propertyName, name) ||
        String::equal(
            _sslBackwardCompatibility->propertyName, name)
#ifdef PEGASUS_OS_ZOS
        || String::equal(_enableCFZAPPLID->propertyName, name)
#endif
#ifdef PEGASUS_NEGOTIATE_AUTHENTICATION
        || String::equal(_mapToLocalName->propertyName, name)
#endif
        )
    {
        retVal = ConfigManager::isValidBooleanValue(value);
    }
    else if (String::equal(_httpAuthType->propertyName, name))
    {
        if (String::equal(value, "Basic") || String::equal(value, "Digest")
#ifdef PEGASUS_NEGOTIATE_AUTHENTICATION
            || String::equal(value, "Negotiate")
#endif
        )
        {
            retVal = true;
        }
    }
    else if (String::equal(_passwordFilePath->propertyName, name))
    {
        String fileName(value);

        //
        // Check if the file path is empty
        //
        if (fileName == String::EMPTY)
        {
            return false;
        }

        fileName = ConfigManager::getHomedPath(fileName);

        //
        // Check if the file path is a directory
        //
        FileSystem::translateSlashes(fileName);
        if (FileSystem::isDirectory(fileName))
        {
            return false;
        }

        //
        // Check if the file exists and is writable
        //
        if (FileSystem::exists(fileName))
        {
            if (!FileSystem::canWrite(fileName))
            {
                return false;
            }
            else
            {
                return true;
            }
        }
        else
        {
            //
            // Check if directory is writable
            //
            Uint32 pos = fileName.reverseFind('/');

            if (pos != PEG_NOT_FOUND)
            {
                String dirName = fileName.subString(0,pos);
                if (!FileSystem::isDirectory(dirName))
                {
                    return false;
                }
                if (!FileSystem::canWrite(dirName) )
                {
                    return false;
                }
                else
                {
                    return true;
                }
            }
            else
            {
                String currentDir;

                //
                // Check if there is permission to write in the
                // current working directory
                //
                FileSystem::getCurrentDirectory(currentDir);

                if (!FileSystem::canWrite(currentDir))
                {
                    return false;
                }
                else
                {
                    return true;
                }
            }
        }
    }
    else if (String::equal(_certificateFilePath->propertyName, name) ||
             String::equal(_keyFilePath->propertyName, name))
    {
        //
        // Check if the file path is empty
        //
        if (value == String::EMPTY)
        {
            return false;
        }

        String fileName = ConfigManager::getHomedPath(value);

        //
        // Check if the file path is a directory
        //
        FileSystem::translateSlashes(fileName);
        if (FileSystem::isDirectory(fileName))
        {
            return false;
        }

        //
        // Check if the file exists and is readable and is not empty.
        //
        if (FileSystem::exists(fileName) && FileSystem::canRead(fileName))
        {
            Uint32 size;
            if (FileSystem::getFileSize(fileName, size))
            {
                if (size > 0)
                {
                    return true;
                }
            }
        }

         return false;
    }
    else if (String::equal(_trustStore->propertyName, name)
#ifdef PEGASUS_ENABLE_SSL_CRL_VERIFICATION
             || String::equal(_crlStore->propertyName, name)
#endif
    )
    {
        //
        // Allow the file path to be empty
        //
        if (value == String::EMPTY)
        {
            return true;
        }

        String fileName = ConfigManager::getHomedPath(value);

        //
        // Check if the file path is a directory
        //
        FileSystem::translateSlashes(fileName);
        if (FileSystem::isDirectory(fileName))
        {
            //
            // Truststore can be a directory, congruent with OpenSSL standards
            // Check if the directoy has read and write permissions
            //
            if (FileSystem::canRead(fileName) && FileSystem::canWrite(fileName))
            {
                return true;
            }
        }
        //
        // Check if the file exists and is readable
        //
        else if (FileSystem::exists(fileName) && FileSystem::canRead(fileName))
        {
            return true;
        }

        return false;
    }
    else if (String::equal(
                 _sslClientVerificationMode->propertyName, name))
    {
        if (String::equal(value, "disabled") ||
            String::equal(value, "required") ||
            String::equal(value, "optional"))
        {
            retVal = true;
        }
    }
    else if (String::equal(_sslTrustStoreUserName->propertyName, name))
    {
        if (System::isSystemUser((const char*)value.getCString()))
        {
            return true;
        }
    }
#ifdef PEGASUS_ENABLE_USERGROUP_AUTHORIZATION
    else if (String::equal(_authorizedUserGroups->propertyName, name))
    {
        retVal = true;
    }
#endif
    else if (String::equal(_cipherSuite->propertyName, name))
    {
        String cipherSuite(value);

        //
        // Check if the cipher list is empty
        //
        if (cipherSuite == String::EMPTY)
        {
            retVal =  false;
        }
        else
        {
           retVal =  true;
        }
    }
#ifdef PEGASUS_ENABLE_SESSION_COOKIES
    else if (String::equal(_httpSessionTimeout->propertyName, name))
    {
        Uint64 v;
        return
            StringConversion::decimalStringToUint64(value.getCString(), v) &&
            StringConversion::checkUintBounds(v, CIMTYPE_UINT32);
    }
#endif
    else
    {
        throw UnrecognizedConfigProperty(name);
    }
    return retVal;
}

/**
    Checks to see if the specified property is dynamic or not.
*/
Boolean SecurityPropertyOwner::isDynamic(const String& name) const
{
    struct ConfigProperty* configProperty =_lookupConfigProperty(name);

    return (configProperty->dynamic == IS_DYNAMIC);
}


PEGASUS_NAMESPACE_END
