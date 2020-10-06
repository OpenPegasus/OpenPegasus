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
// Author: Roger Kumpf, Hewlett-Packard Company (roger_kumpf@hp.com)
//
// Modified By:
//
//%/////////////////////////////////////////////////////////////////////////////

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/System.h>

#include "UserContextTestProvider.h"

PEGASUS_NAMESPACE_BEGIN

#define NAMESPACE "test/TestProvider"

extern "C" PEGASUS_EXPORT CIMProvider* PegasusCreateProvider(
    const String & providerName)
{
    if (String::equalNoCase(providerName, "UserContextRequestorProvider") ||
        String::equalNoCase(providerName, "UserContextPrivilegedProvider") ||
        String::equalNoCase(providerName, "UserContextDesignatedProvider") ||
        String::equalNoCase(providerName, "UserContextCIMServerProvider"))
    {
        return(new UserContextTestProvider());
    }
    return (0);
}


CIMInstance UserContextTestProvider::_buildInstance(const CIMName& className)
{
    if ((!className.equal(CIMName("TST_UserContextRequestor"))) &&
        (!className.equal(CIMName("TST_UserContextPrivileged"))) &&
        (!className.equal(CIMName("TST_UserContextDesignated"))) &&
        (!className.equal(CIMName("TST_UserContextCIMServer"))))
    {
        throw CIMNotSupportedException(className.getString());
    }

    CIMInstance instance(className);
    instance.addProperty(CIMProperty(CIMName("Id"), Uint16(1)));
    instance.addProperty(
        CIMProperty(CIMName("UserContext"), System::getEffectiveUserName()));

    Array<CIMKeyBinding> kb;
    kb.append(CIMKeyBinding(CIMName("Id"), "1", CIMKeyBinding::NUMERIC));
    CIMObjectPath path(String::EMPTY, CIMNamespaceName(), className, kb);
    instance.setPath(path);

    return instance;
}

UserContextTestProvider::UserContextTestProvider()
{
}

UserContextTestProvider::~UserContextTestProvider()
{
}

void UserContextTestProvider::initialize(CIMOMHandle& cimom)
{
    // save cimom handle
    _cimom = cimom;
}

void UserContextTestProvider::terminate()
{
    delete this;
}

void UserContextTestProvider::getInstance(
    const OperationContext & context,
    const CIMObjectPath & instanceReference,
    const Boolean includeQualifiers,
    const Boolean includeClassOrigin,
    const CIMPropertyList & propertyList,
    InstanceResponseHandler & handler)
{
    // ensure the Id key is valid
    Array<CIMKeyBinding> keys = instanceReference.getKeyBindings();
    if ((keys.size() != 1) || !keys[0].getName().equal("Id"))
    {
        throw CIMException(CIM_ERR_INVALID_PARAMETER);
    }

    // ensure the Namespace is valid
    if (!instanceReference.getNameSpace().equal(NAMESPACE))
    {
        throw CIMException(CIM_ERR_INVALID_NAMESPACE);
    }

    // begin processing the request
    handler.processing();

    handler.deliver(_buildInstance(instanceReference.getClassName()));

    // complete processing the request
    handler.complete();
}

void UserContextTestProvider::enumerateInstances(
    const OperationContext & context,
    const CIMObjectPath & ref,
    const Boolean includeQualifiers,
    const Boolean includeClassOrigin,
    const CIMPropertyList & propertyList,
    InstanceResponseHandler & handler)
{
    // ensure the Namespace is valid
    if (!ref.getNameSpace().equal(NAMESPACE))
    {
        throw CIMException(CIM_ERR_INVALID_NAMESPACE);
    }

    // begin processing the request
    handler.processing();

    handler.deliver(_buildInstance(ref.getClassName()));

    // complete processing the request
    handler.complete();
}

void UserContextTestProvider::enumerateInstanceNames(
    const OperationContext & context,
    const CIMObjectPath & classReference,
    ObjectPathResponseHandler & handler)
{
    // ensure the Namespace is valid
    if (!classReference.getNameSpace().equal(NAMESPACE))
    {
        throw CIMException(CIM_ERR_INVALID_NAMESPACE);
    }

    // begin processing the request
    handler.processing();

    handler.deliver(_buildInstance(classReference.getClassName()).getPath());

    // complete processing the request
    handler.complete();
}

void UserContextTestProvider::modifyInstance(
    const OperationContext & context,
    const CIMObjectPath & instanceReference,
    const CIMInstance & instanceObject,
    const Boolean includeQualifiers,
    const CIMPropertyList & propertyList,
    ResponseHandler & handler)
{
    // This provider does not support instance modification
    throw CIMNotSupportedException("modifyInstance");
}

void UserContextTestProvider::createInstance(
    const OperationContext & context,
    const CIMObjectPath & instanceReference,
    const CIMInstance & instanceObject,
    ObjectPathResponseHandler & handler)
{
    // This provider does not support instance creation
    throw CIMNotSupportedException("modifyInstance");
}

void UserContextTestProvider::deleteInstance(
    const OperationContext & context,
    const CIMObjectPath & instanceReference,
    ResponseHandler & handler)
{
    // This provider does not support instance deletion
    throw CIMNotSupportedException("modifyInstance");
}

PEGASUS_NAMESPACE_END
