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
// Author: Paulo F. Borges (pfborges@wowmail.com)
//
// Modified By:  Jenny Yu, Hewlett-Packard Company (jenny.yu@hp.com)
//
//%////////////////////////////////////////////////////////////////////////////

//------------------------------------------------------------------------------
// INCLUDES
//------------------------------------------------------------------------------
//Pegasus includes
#include <Pegasus/Common/Config.h>
#include <Pegasus/Provider/ProviderException.h>
#include "NISServerServiceProvider.h"


//------------------------------------------------------------------------------
PEGASUS_USING_PEGASUS;
PEGASUS_USING_STD;

//==============================================================================
//
// Class [NISServerServiceProvider] methods
//
//==============================================================================

//------------------------------------------------------------------------------
// Constructor
//------------------------------------------------------------------------------
NISServerServiceProvider::NISServerServiceProvider(void)
{
}

//------------------------------------------------------------------------------
// Destructor
//------------------------------------------------------------------------------
NISServerServiceProvider::~NISServerServiceProvider(void)
{
}

//------------------------------------------------------------------------------
// getInstance method
//------------------------------------------------------------------------------
void
NISServerServiceProvider::getInstance(
                const OperationContext & context,
                const CIMObjectPath & ref,
                const Boolean includeQualifiers,
                const Boolean includeClassOrigin,
                const CIMPropertyList & propertyList,
                InstanceResponseHandler & handler)
{
    CIMName className;
    Array<CIMKeyBinding> keys;
    CIMInstance instance;
    String name;
    String systemName;
    String keyValue;
    NISServerService nis;

    //-- make sure we're working on the right class
    className = ref.getClassName();
    if (!className.equal(CLASS_NAME))
        throw CIMNotSupportedException(
                "NISServerServiceProvider does not support class " +
                className.getString());

    if(!nis.AccessOk(context))
       throw CIMAccessDeniedException("Access denied by NISProvider");

    // Retrieve system name property value
    if(!nis.getSystemName(systemName))
        throw CIMObjectNotFoundException("NISServerServiceProvider "
            "can't determine system name");

    // Retrieve name property value
    if(!nis.getName(name))
        throw CIMObjectNotFoundException("NISServerServiceProvider "
            "can't determine NIS domain name");

    //-- make sure we're the right instance
    int keyCount;
    CIMName keyName;

    keyCount = MAX_KEYS;
    keys = ref.getKeyBindings();

    if ((unsigned int)keys.size() != (unsigned int)keyCount)
        throw CIMInvalidParameterException("Wrong number of keys");

    for (unsigned int ii = 0; ii < keys.size(); ii++) {
         keyName = keys[ii].getName();
         keyValue = keys[ii].getValue();

        if (keyName.equal(PROPERTY_CREATION_CLASS_NAME) &&
            (String::equalNoCase(keyValue, CLASS_NAME) || keyValue.size() == 0))
            keyCount--;
        else if (keyName.equal(PROPERTY_NAME) &&
                (String::equalNoCase(keyValue, name) || keyValue.size() == 0 &&
                 name.size() == 0))
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

    if (keyCount > 0)
        throw CIMInvalidParameterException("Wrong keys");

    handler.processing();
    instance = _build_instance(className,
                               ref.getNameSpace(),
                               ref.getKeyBindings(),
                               nis);
    handler.deliver(instance);
    handler.complete();
    return;
}

//------------------------------------------------------------------------------
// enumerateInstances method
//------------------------------------------------------------------------------
void
NISServerServiceProvider::enumerateInstances(
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
    Array<CIMKeyBinding> keys;
    NISServerService nis;

    className = ref.getClassName();

    // only return instances when enumerate on our subclass, CIMOM
    // will call us as natural part of recursing through subtree on
    // enumerate - if we return instances on enumerate of our superclass,
    // there would be dups
    if (!className.equal(CLASS_NAME))
        throw CIMNotSupportedException("NISServerServiceProvider "
                "does not support class " + className.getString());

    if(!nis.AccessOk(context))
       throw CIMAccessDeniedException("Access denied by NISProvider");

    keys = ref.getKeyBindings();
    handler.processing();
    newref = _fill_reference(ref.getNameSpace(), className, nis);
    instance = _build_instance(className,
                               ref.getNameSpace(),
                               ref.getKeyBindings(),
                               nis);

    instance.setPath(newref);
    handler.deliver(instance);
    handler.complete();
    return;
}

//------------------------------------------------------------------------------
// enumerateInstanceNames method
//------------------------------------------------------------------------------
void
NISServerServiceProvider::enumerateInstanceNames(
                const OperationContext & context,
                const CIMObjectPath & ref,
                ObjectPathResponseHandler & handler)
{
    CIMObjectPath newref;
    CIMName className;
    NISServerService nis;

    // only return instances when enumerate on our subclass, CIMOM
    // will call us as natural part of recursing through subtree on
    // enumerate - if we return instances on enumerate of our superclass,
    // there would be dups

    className = ref.getClassName();

    if (!className.equal(CLASS_NAME))
        throw CIMNotSupportedException("NISServerServiceProvider "
                "does not support class " + className.getString());

    if(!nis.AccessOk(context))
       throw CIMAccessDeniedException("Access denied by NISProvider");

    handler.processing();
    newref = _fill_reference(ref.getNameSpace(), className, nis);
    handler.deliver(newref);
    handler.complete();
    return;
}

//------------------------------------------------------------------------------
// modifyInstance method
//------------------------------------------------------------------------------
void
NISServerServiceProvider::modifyInstance(
                const OperationContext & context,
                const CIMObjectPath & instanceReference,
                const CIMInstance & instanceObject,
                const Boolean includeQualifiers,
                const CIMPropertyList & propertyList,
                ResponseHandler & handler)
{
    throw CIMNotSupportedException("NISServerServiceProvider "
                       "does not support modifyInstance");
}

//------------------------------------------------------------------------------
// createInstance method
//------------------------------------------------------------------------------
void
NISServerServiceProvider::createInstance(
                const OperationContext & context,
                const CIMObjectPath & instanceReference,
                const CIMInstance & instanceObject,
                ObjectPathResponseHandler & handler)
{
    throw CIMNotSupportedException("NISServerServiceProvider "
                       "does not support createInstance");
}

//------------------------------------------------------------------------------
// deleteInstance method
//------------------------------------------------------------------------------
void
NISServerServiceProvider::deleteInstance(
                const OperationContext & context,
                const CIMObjectPath & instanceReference,
                ResponseHandler & handler)
{
    throw CIMNotSupportedException("NISServerServiceProvider "
                       "does not support deleteInstance");
}

//------------------------------------------------------------------------------
// initialize method
//------------------------------------------------------------------------------
void
NISServerServiceProvider::initialize(CIMOMHandle& handle)
{
}

//------------------------------------------------------------------------------
// terminate method
//------------------------------------------------------------------------------
void
NISServerServiceProvider::terminate(void)
{
    delete this;
}

// This method inserts new properties into NISServerService class.
//------------------------------------------------------------------------------
// _build_instance method
//------------------------------------------------------------------------------
CIMInstance
NISServerServiceProvider::_build_instance(const CIMName & className,
                                          const CIMNamespaceName & nameSpace,
                                          const Array<CIMKeyBinding> keys,
                                          NISServerService nis)
{
    CIMInstance instance(className);

    String strValue;
    String hostName;
    Array<String> lst;
    Uint16 Uint16Value;

    if(!nis.getLocalHostName(hostName))
        hostName.assign("localhost");

    instance.setPath(CIMObjectPath(hostName,
                                   nameSpace,
                                   className,
                                   keys));

    instance.addProperty(CIMProperty(PROPERTY_SYSTEM_CREATION_CLASS_NAME,
                                      SYSTEM_CREATION_CLASS_NAME.getString()));

    if(!nis.getSystemName(hostName))
        hostName.assign("localhost");

    instance.addProperty(CIMProperty(PROPERTY_SYSTEM_NAME, hostName));

    // Setting the CreationClassName property value
    instance.addProperty(CIMProperty(PROPERTY_CREATION_CLASS_NAME,
                                      CREATION_CLASS_NAME.getString()));

    // Verify if Name property is ok
    nis.getName(strValue);

    // Setting the Name property value
    instance.addProperty(CIMProperty(PROPERTY_NAME, strValue));

    // Verify if Caption property is ok
    if(!nis.getCaption(strValue)) {
        throw CIMOperationFailedException("NISServerServiceProvider "
              "can't determine Caption property");
    }

    // Setting the Caption property value
    instance.addProperty(CIMProperty(PROPERTY_CAPTION, strValue));

    // Verify if Description property is ok
    if(!nis.getDescription(strValue)) {
        throw CIMOperationFailedException("NISServerServiceProvider "
              "can't determine Description property");
    }

    // Setting the Description property value
    instance.addProperty(CIMProperty(PROPERTY_DESCRIPTION, strValue));

    // Verify if ServerWaitFlag property is ok
    if(!nis.getServerWaitFlag(Uint16Value)) {
        throw CIMOperationFailedException("NISServerServiceProvider "
              "can't determine ServerWaitFlag property");
    }

    // Setting the ServerWaitFlag property value
    instance.addProperty(CIMProperty(PROPERTY_SERVER_WAIT_FLAG, Uint16Value));

    // Verify if ServerType property is ok
    if(!nis.getServerType(Uint16Value)) {
        throw CIMOperationFailedException("NISServerServiceProvider "
              "can't determine ServerType property");
    }

    // Setting the ServerType property value
    instance.addProperty(CIMProperty(PROPERTY_SERVER_TYPE, Uint16Value));

    return instance;
}

// This method verifies the property names.
//------------------------------------------------------------------------------
// _fill_reference method
//------------------------------------------------------------------------------
CIMObjectPath
NISServerServiceProvider::_fill_reference(const CIMNamespaceName &nameSpace,
                                          const CIMName &className,
                                          NISServerService nis)
{
    Array<CIMKeyBinding> keys;
    String hostName;
    String param;


    keys.append(CIMKeyBinding(PROPERTY_SYSTEM_CREATION_CLASS_NAME,
                           SYSTEM_CREATION_CLASS_NAME.getString(),
                           CIMKeyBinding::STRING));

    if(!nis.getSystemName(hostName))
        hostName.assign("localhost");

    keys.append(CIMKeyBinding(PROPERTY_SYSTEM_NAME, hostName,
                           CIMKeyBinding::STRING));

    keys.append(CIMKeyBinding(PROPERTY_CREATION_CLASS_NAME,
                           CREATION_CLASS_NAME.getString(),
                           CIMKeyBinding::STRING));

    nis.getName(param);

    keys.append(CIMKeyBinding(PROPERTY_NAME, param, CIMKeyBinding::STRING));

    return CIMObjectPath(hostName, nameSpace, className, keys);
}

