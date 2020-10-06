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


#ifndef ConfigSettingProvider_h
#define ConfigSettingProvider_h

///////////////////////////////////////////////////////////////////////////////
//  ConfigSetting Provider
///////////////////////////////////////////////////////////////////////////////

#include <Pegasus/Common/Config.h>
#include <Pegasus/ControlProviders/ConfigSettingProvider/Linkage.h>

#include <Pegasus/Common/String.h>
#include <Pegasus/Common/System.h>
#include <Pegasus/Common/ArrayInternal.h>
#include <Pegasus/Common/CIMType.h>
#include <Pegasus/Common/CIMInstance.h>
#include <Pegasus/Common/CIMObjectPath.h>
#include <Pegasus/Common/InternalException.h>
#include <Pegasus/Common/CIMStatusCode.h>
#include <Pegasus/Common/Tracer.h>
#include <Pegasus/Config/ConfigManager.h>

#include <Pegasus/Repository/CIMRepository.h>
#include <Pegasus/Provider/CIMInstanceProvider.h>
#include <Pegasus/Provider/CIMMethodProvider.h>
#include <Pegasus/Common/ResponseHandler.h>

PEGASUS_NAMESPACE_BEGIN

/**
    The ConfigSettingProvider provides responses to the CIM Operations
    defined in the DMTF document CIM Operations over HTTP (Section 2.5).

    This provider implements the following functions:
    - getInstance
    - modifyInstance
    - enumerateInstances
    - enumerateInstanceNames
    - InvokeMethod
*/

class PEGASUS_CONFIGSETTINGPROVIDER_LINKAGE ConfigSettingProvider
    : public CIMInstanceProvider, public CIMMethodProvider
{
public:

    ConfigSettingProvider()
    {
        _configManager = ConfigManager::getInstance();
    }

    virtual ~ConfigSettingProvider()
    {
    }

    // Note:  The initialize() and terminate() methods are not called for
    // Control Providers.
    void initialize(CIMOMHandle& cimomHandle) { }
    void terminate() { }

    /**
    Creates a new instance.

    @param  context
    @param  instanceReference
    @param  CIMInstance
    @param  handler
    */
    virtual void createInstance(
    const OperationContext & context,
    const CIMObjectPath & instanceReference,
        const CIMInstance& myInstance,
    ObjectPathResponseHandler & handler)
    {
        throw PEGASUS_CIM_EXCEPTION(CIM_ERR_NOT_SUPPORTED, "");
    }

    /**
    Deletes the specified instance.

    @param  context
    @param  InstanceName
    @param  handler
    */
    virtual void deleteInstance(
    const OperationContext & context,
        const CIMObjectPath& instanceName,
    ResponseHandler & handler)
    {
        throw PEGASUS_CIM_EXCEPTION(CIM_ERR_NOT_SUPPORTED, "");
    }

    /**
    Returns the instance based on instanceName.

    @param context specifies security and locale information relevant for
           the lifetime of this operation.
    @param instanceName name of the class for which instance is requested.
    @param includeQualifiers specifies whether qualifiers must be included in
    the returned instance
    @param includeClassOrigin specifies whether class origin should be
    included in each of the returned instance elements
    @param propertyList list containing the properties.
    @param handler enables providers to asynchronously return the results.
    */
    virtual void getInstance(
    const OperationContext & context,
        const CIMObjectPath& instanceName,
    const Boolean includeQualifiers,
    const Boolean includeClassOrigin,
        const CIMPropertyList& propertyList,
    InstanceResponseHandler & handler);

    /**
    Modify instance based on modifiedInstance.

    @param context specifies security and locale information relevant for
           the lifetime of this operation.
    @param instanceReference the fully qualified object path of the instance.
    @param modifiedInstance  the modified instance.
    @param includeQualifiers specifies whether qualifiers must be updated as
    specified in the modified instance
    @param propertyList list containing the properties to which the modify
                        operation must be limited.
    @param handler enables providers to asynchronously return the results.
    */
    void modifyInstance(
    const OperationContext & context,
    const CIMObjectPath & instanceReference,
        const CIMInstance& modifiedIns,
    const Boolean includeQualifiers,
        const CIMPropertyList& propertyList,
    ResponseHandler & handler);

    /**
    Enumerates all the config properties and values.

    @param context specifies security and locale information relevant for
           the lifetime of this operation.
    @param ref the fully qualified object path of the instance.
    @param includeQualifiers specifies whether qualifiers must be included in
    the returned instances
    @param includeClassOrigin specifies whether class origin should be
    included in each of the returned instance elements
    @param propertyList list containing the properties.
    @param handler enables providers to asynchronously return the results.
    */
    virtual void enumerateInstances(
    const OperationContext & context,
    const CIMObjectPath & ref,
    const Boolean includeQualifiers,
    const Boolean includeClassOrigin,
        const CIMPropertyList& propertyList,
    InstanceResponseHandler & handler);

    /**
    Enumerates all the config property names.

    @param context specifies security and locale information relevant for
           the lifetime of this operation.
    @param classReference the fully qualified object path of the instance.
    @param handler enables providers to asynchronously return the results.
    */
    virtual void enumerateInstanceNames(
    const OperationContext & context,
    const CIMObjectPath & classReference,
        ObjectPathResponseHandler & handler);

    // CIMMethodProvider interface
    virtual void invokeMethod(
        const OperationContext & context,
        const CIMObjectPath & cimObjectPath,
        const CIMName & methodName,
        const Array<CIMParamValue> & inParams,
        MethodResultResponseHandler & handler);

private:

    void _modifyInstance(
        const OperationContext & context,
        const CIMObjectPath & instanceReference,
        const CIMInstance& modifiedIns,
        const CIMPropertyList& propertyList,
        Uint32 timeoutSeconds);

    /**
        Verify User Authorization.

        @param  user                    userName to be verified

        @throw  CIM_ERR_ACCESS_DENIED   if the specified user is not a
                                        priviliged user.
    */
    void _verifyAuthorization(const String& user);

    /**
    send notify config change message to provider manager service.

    @param propertyName    The name of the property to update.
    @param propertyValue   The new value of the property.
    @param userName        The User requesting the update.
    @param currentValueModified    Specifies whether the current value or
                                   planned value is modified.
                                   true - current value modified;
                                   false - planned value modified
    */
    void _sendNotifyConfigChangeMessage(
        const String& propertyName,
        const String& newPropertyValue,
        const String& userName,
        const char *queueName,
        Boolean currentValueModified);

    //
    // Config Manager Instance variable
    //
    ConfigManager*   _configManager;

};

PEGASUS_NAMESPACE_END

#endif // ConfigSettingProvider_h
