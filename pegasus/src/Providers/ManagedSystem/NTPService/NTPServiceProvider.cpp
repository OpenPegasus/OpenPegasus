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

// Based on DNSServiceProvider.cpp file

//------------------------------------------------------------------------------
// INCLUDES
//------------------------------------------------------------------------------
//Pegasus includes
#include <Pegasus/Common/Config.h>
#include <Pegasus/Provider/ProviderException.h>

#include "NTPServiceProvider.h"

//------------------------------------------------------------------------------
PEGASUS_USING_STD;

//==============================================================================
//
// Class [NTPServiceProvider] methods
//
//==============================================================================

//------------------------------------------------------------------------------
// Constructor
//------------------------------------------------------------------------------
NTPServiceProvider::NTPServiceProvider()
{
}

//------------------------------------------------------------------------------
// Destructor
//------------------------------------------------------------------------------
NTPServiceProvider::~NTPServiceProvider()
{
}

//------------------------------------------------------------------------------
// getInstance
//------------------------------------------------------------------------------
void NTPServiceProvider::getInstance(
    const OperationContext & context,
    const CIMObjectPath & ref,
    const Boolean includeQualifiers,
    const Boolean includeClassOrigin,
    const CIMPropertyList & propertyList,
    InstanceResponseHandler & handler)
{
    Array<CIMKeyBinding> keys;
    CIMInstance instance;
    CIMName className;

    //-- make sure we're working on the right class
    className = ref.getClassName();

    if (!className.equal(CLASS_NAME))
        throw CIMNotSupportedException(
            "NTPServiceProvider does not support class " +
                className.getString());

    NTPService ntp;

    if (!ntp.AccessOk(context))
        throw CIMAccessDeniedException("Access denied by NTPServiceProvider");

    //-- make sure we're the right instance
    int keyCount;
    CIMName keyName;
    String keyValue;
    String svcName;
    String systemName;

    keyCount = MAX_KEYS;
    keys = ref.getKeyBindings();

    if ((unsigned int)keys.size() != (unsigned int)keyCount)
        throw CIMInvalidParameterException("Wrong number of keys");

    // Retrieve name property value
    if (!ntp.getSystemName(systemName))
        throw CIMObjectNotFoundException("NTPServiceProvider "
            "can't create PG_NTPService instance");

    // Retrieve service name property value
    if (!ntp.getNTPName(svcName))
        throw CIMObjectNotFoundException("NTPServiceProvider "
            "can't create PG_NTPService instance - no service name");

    for (unsigned int ii = 0; ii < keys.size(); ii++) {
         keyName = keys[ii].getName();
         keyValue = keys[ii].getValue();

        if (keyName.equal (PROPERTY_CREATION_CLASS_NAME) &&
            (String::equalNoCase(keyValue, CLASS_NAME.getString()) ||
             keyValue.size() == 0))
            keyCount--;
        else if (keyName.equal (PROPERTY_NAME) &&
                 String::equalNoCase(keyValue, svcName))
            keyCount--;
        else if (keyName.equal (PROPERTY_SYSTEM_CREATION_CLASS_NAME) &&
                 ((keyValue.size() == 0) || (String::equalNoCase(keyValue,
                         SYSTEM_CREATION_CLASS_NAME.getString()))))
            keyCount--;
        else if (keyName.equal (PROPERTY_SYSTEM_NAME) &&
                 ((keyValue.size() == 0) ||
                  (String::equalNoCase(keyValue, systemName))))
            keyCount--;

    }

    if (keyCount)
        throw CIMInvalidParameterException("Wrong keys");

    handler.processing();
    instance = _build_instance(className, ref.getNameSpace(), keys, ntp);
    handler.deliver(instance);
    handler.complete();
    return;
}

//------------------------------------------------------------------------------
// enumerateInstances
//------------------------------------------------------------------------------
void NTPServiceProvider::enumerateInstances(
    const OperationContext & context,
    const CIMObjectPath & ref,
    const Boolean includeQualifiers,
    const Boolean includeClassOrigin,
    const CIMPropertyList & propertyList,
    InstanceResponseHandler & handler)
{
    CIMName className;
    CIMInstance instance;
    CIMObjectPath newref;

    className = ref.getClassName();

    NTPService ntp;

    // only return instances when enumerate on our subclass, CIMOM
    // will call us as natural part of recursing through subtree on
    // enumerate - if we return instances on enumerate of our superclass,
    // there would be dups
    if (!className.equal (CLASS_NAME))
        throw CIMNotSupportedException(
            "NTPServiceProvider does not support class " +
                className.getString());

    if (!ntp.AccessOk(context))
        throw CIMAccessDeniedException("Access denied by NTPServiceProvider");

    handler.processing();
    newref = _fill_reference(ref.getNameSpace(), className, ntp);
    instance = _build_instance(
        className,
        ref.getNameSpace(),
        ref.getKeyBindings(),
        ntp);
    instance.setPath(newref);
    handler.deliver(instance);
    handler.complete();
    return;
}

//------------------------------------------------------------------------------
// enumerateInstanceNames
//------------------------------------------------------------------------------
void NTPServiceProvider::enumerateInstanceNames(
    const OperationContext & context,
    const CIMObjectPath & ref,
    ObjectPathResponseHandler & handler)
{
    CIMObjectPath newref;
    CIMName className;

    // only return instances when enumerate on our subclass, CIMOM
    // will call us as natural part of recursing through subtree on
    // enumerate - if we return instances on enumerate of our superclass,
    // there would be dups

    className = ref.getClassName();

    NTPService ntp;

    if (!className.equal (CLASS_NAME))
        throw CIMNotSupportedException(
            "NTPServiceProvider does not support class " +
                className.getString());

    if (!ntp.AccessOk(context))
        throw CIMAccessDeniedException("Access denied by NTPServiceProvider");

    handler.processing();
    newref = _fill_reference(ref.getNameSpace(), className, ntp);
    handler.deliver(newref);
    handler.complete();
    return;
}

//------------------------------------------------------------------------------
// modifyInstance
//------------------------------------------------------------------------------
void NTPServiceProvider::modifyInstance(
    const OperationContext & context,
    const CIMObjectPath & ref,
    const CIMInstance & obj,
    const Boolean includeQualifiers,
    const CIMPropertyList & propertyList,
    ResponseHandler & handler)
{
    throw CIMNotSupportedException(
        "NTPServiceProvider does not support modifyInstance");
}

//------------------------------------------------------------------------------
// createInstance
//------------------------------------------------------------------------------
void NTPServiceProvider::createInstance(
    const OperationContext & context,
    const CIMObjectPath & ref,
    const CIMInstance & obj,
    ObjectPathResponseHandler & handler)
{
    throw CIMNotSupportedException(
        "NTPServiceProvider does not support createInstance");
}

//------------------------------------------------------------------------------
// deleteInstance
//------------------------------------------------------------------------------
void NTPServiceProvider::deleteInstance(
    const OperationContext & context,
    const CIMObjectPath & ref,
    ResponseHandler & handler)
{
    throw CIMNotSupportedException(
        "NTPServiceProvider does not support deleteInstance");
}

//------------------------------------------------------------------------------
// invokeMethod
//------------------------------------------------------------------------------
void NTPServiceProvider::invokeMethod(
    const OperationContext& context,
    const CIMObjectPath& objectReference,
    const CIMName& methodName,
    const Array<CIMParamValue>& inParameters,
    MethodResultResponseHandler& handler)
{
    throw CIMNotSupportedException(
        "NTPServiceProvider does not support invokeMethod");
}

//------------------------------------------------------------------------------
// initialize
//------------------------------------------------------------------------------
void NTPServiceProvider::initialize(CIMOMHandle& handle)
{
}

//------------------------------------------------------------------------------
// terminate
//------------------------------------------------------------------------------
void NTPServiceProvider::terminate()
{
    delete this;
}

// This method insert new properties into NTPService class.
//------------------------------------------------------------------------------
// _build_instance
//------------------------------------------------------------------------------
CIMInstance NTPServiceProvider::_build_instance(
    const CIMName & className,
    const CIMNamespaceName & nameSpace,
    const Array<CIMKeyBinding> keys,
    NTPService ntp)
{
    CIMInstance instance(className);
    String strValue;
    String hostName;
    Array<String> lst;

    if (!ntp.getLocalHostName(hostName))
        hostName.assign("localhost");

    instance.setPath(CIMObjectPath(hostName, nameSpace, className, keys));

    instance.addProperty(CIMProperty(PROPERTY_SYSTEM_CREATION_CLASS_NAME,
                                      SYSTEM_CREATION_CLASS_NAME.getString()));

    if (!ntp.getSystemName(hostName))
        hostName.assign("localhost");

    instance.addProperty(CIMProperty(PROPERTY_SYSTEM_NAME, hostName));

    instance.addProperty(CIMProperty(PROPERTY_CREATION_CLASS_NAME,
                                      CREATION_CLASS_NAME.getString()));

    if (!ntp.getNTPName(strValue))
    {
        throw CIMOperationFailedException(
            "NTPServiceProvider can't determine Name property");
    }

    instance.addProperty(CIMProperty(PROPERTY_NAME, strValue));

    if (!ntp.getCaption(strValue))
    {
        throw CIMOperationFailedException(
            "NTPServiceProvider can't determine Caption property");
    }

    instance.addProperty(CIMProperty(PROPERTY_CAPTION, strValue));

    if (!ntp.getDescription(strValue))
    {
        throw CIMOperationFailedException(
            "NTPServiceProvider can't determine Description property");
    }

    instance.addProperty(CIMProperty(PROPERTY_DESCRIPTION, strValue));

    if (!ntp.getServerAddress(lst))
    {
        throw CIMOperationFailedException(
            "NTPServiceProvider can't determine ServerAddress property");
    }

    instance.addProperty(CIMProperty(PROPERTY_SERVER_ADDRESS, lst));

    return instance;
}

// This method verify the property names.
//------------------------------------------------------------------------------
// _fill_reference
//------------------------------------------------------------------------------
CIMObjectPath NTPServiceProvider::_fill_reference(
    const CIMNamespaceName &nameSpace,
    const CIMName &className,
    NTPService ntp)
{
    Array<CIMKeyBinding> keys;
    String hostName;
    String strValue;
    String param;

    keys.append(CIMKeyBinding(PROPERTY_SYSTEM_CREATION_CLASS_NAME,
                           SYSTEM_CREATION_CLASS_NAME.getString(),
                           CIMKeyBinding::STRING));

    if (!ntp.getSystemName(hostName))
        hostName.assign("localhost");

    keys.append(CIMKeyBinding(
        PROPERTY_SYSTEM_NAME,
        hostName,
        CIMKeyBinding::STRING));

    keys.append(CIMKeyBinding(
        PROPERTY_CREATION_CLASS_NAME,
        CREATION_CLASS_NAME.getString(),
        CIMKeyBinding::STRING));

    if (!ntp.getNTPName(strValue))
        strValue.assign("unknown");

    keys.append(CIMKeyBinding(
        PROPERTY_NAME,
        strValue,
        CIMKeyBinding::STRING));

    return CIMObjectPath(hostName, nameSpace, className, keys);
}
