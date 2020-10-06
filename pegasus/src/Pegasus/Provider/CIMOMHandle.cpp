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


#include <Pegasus/Common/Constants.h>
#include <Pegasus/Common/MessageQueue.h>
#include "CIMOMHandleRep.h"
#include "InternalCIMOMHandleRep.h"
#include "ClientCIMOMHandleRep.h"

#include "CIMOMHandle.h"


PEGASUS_NAMESPACE_BEGIN

CIMOMHandle::CIMOMHandle()
{
    // The existence of a RequestDispatcher determines which Rep to use
    MessageQueue* dispatcher =
        MessageQueue::lookup(PEGASUS_QUEUENAME_OPREQDISPATCHER);
    if (dispatcher != 0)
    {
        // RequestDispatcher exists.  We can use InternalCIMOMHandleRep
        _rep = new InternalCIMOMHandleRep();
    }
    else
    {
        // No RequestDispatcher exists.  We must use ClientCIMOMHandleRep
        _rep = new ClientCIMOMHandleRep();
    }
}

CIMOMHandle::CIMOMHandle(const CIMOMHandle & handle)
{
    Inc(this->_rep = handle._rep);
}

CIMOMHandle::~CIMOMHandle()
{
    Dec(_rep);
}

CIMOMHandle & CIMOMHandle::operator =(const CIMOMHandle & handle)
{
    if (this != &handle)
    {
        Dec(_rep);
        Inc(_rep = handle._rep);
    }
    return *this;
}


CIMClass CIMOMHandle::getClass(
    const OperationContext& context,
    const CIMNamespaceName& nameSpace,
    const CIMName& className,
    Boolean localOnly,
    Boolean includeQualifiers,
    Boolean includeClassOrigin,
    const CIMPropertyList& propertyList)
{
    return _rep->getClass(
        context,
        nameSpace,
        className,
        localOnly,
        includeQualifiers,
        includeClassOrigin,
        propertyList);
}

Array<CIMClass> CIMOMHandle::enumerateClasses(
    const OperationContext& context,
    const CIMNamespaceName& nameSpace,
    const CIMName& className,
    Boolean deepInheritance,
    Boolean localOnly,
    Boolean includeQualifiers,
    Boolean includeClassOrigin)
{
    return _rep->enumerateClasses(
        context,
        nameSpace,
        className,
        deepInheritance,
        localOnly,
        includeQualifiers,
        includeClassOrigin);
}

Array<CIMName> CIMOMHandle::enumerateClassNames(
    const OperationContext& context,
    const CIMNamespaceName& nameSpace,
    const CIMName& className,
    Boolean deepInheritance)
{
    return _rep->enumerateClassNames(
        context,
        nameSpace,
        className,
        deepInheritance);
}

void CIMOMHandle::createClass(
    const OperationContext& context,
    const CIMNamespaceName& nameSpace,
    const CIMClass& newClass)
{
    _rep->createClass(
        context,
        nameSpace,
        newClass);
}

void CIMOMHandle::modifyClass(
    const OperationContext& context,
    const CIMNamespaceName& nameSpace,
    const CIMClass& modifiedClass)
{
    _rep->modifyClass(
        context,
        nameSpace,
        modifiedClass);
}

void CIMOMHandle::deleteClass(
    const OperationContext& context,
    const CIMNamespaceName& nameSpace,
    const CIMName& className)
{
    _rep->deleteClass(
        context,
        nameSpace,
        className);
}

CIMInstance CIMOMHandle::getInstance(
    const OperationContext& context,
    const CIMNamespaceName& nameSpace,
    const CIMObjectPath& instanceName,
    Boolean localOnly,
    Boolean includeQualifiers,
    Boolean includeClassOrigin,
    const CIMPropertyList& propertyList)
{
    return _rep->getInstance(
        context,
        nameSpace,
        instanceName,
        includeQualifiers,
        includeClassOrigin,
        propertyList).getInstance();
}

Array<CIMInstance> CIMOMHandle::enumerateInstances(
    const OperationContext& context,
    const CIMNamespaceName& nameSpace,
    const CIMName& className,
    Boolean deepInheritance,
    Boolean localOnly,
    Boolean includeQualifiers,
    Boolean includeClassOrigin,
    const CIMPropertyList& propertyList)
{
    return _rep->enumerateInstances(
        context,
        nameSpace,
        className,
        deepInheritance,
        includeQualifiers,
        includeClassOrigin,
        propertyList).getInstances();
}

Array<CIMObjectPath> CIMOMHandle::enumerateInstanceNames(
    const OperationContext& context,
    const CIMNamespaceName& nameSpace,
    const CIMName& className)
{
    return _rep->enumerateInstanceNames(
        context,
        nameSpace,
        className).getInstanceNames();
}

CIMObjectPath CIMOMHandle::createInstance(
    const OperationContext& context,
    const CIMNamespaceName& nameSpace,
    const CIMInstance& newInstance)
{
    return _rep->createInstance(
        context,
        nameSpace,
        newInstance);
}

void CIMOMHandle::modifyInstance(
    const OperationContext& context,
    const CIMNamespaceName& nameSpace,
    const CIMInstance& modifiedInstance,
    Boolean includeQualifiers,
    const CIMPropertyList& propertyList)
{
    _rep->modifyInstance(
        context,
        nameSpace,
        modifiedInstance,
        includeQualifiers,
        propertyList);
}

void CIMOMHandle::deleteInstance(
    const OperationContext& context,
    const CIMNamespaceName& nameSpace,
    const CIMObjectPath& instanceName)
{
    _rep->deleteInstance(
        context,
        nameSpace,
        instanceName);
}

Array<CIMObject> CIMOMHandle::execQuery(
    const OperationContext& context,
    const CIMNamespaceName& nameSpace,
    const String& queryLanguage,
    const String& query)
{
    return _rep->execQuery(
        context,
        nameSpace,
        queryLanguage,
        query).getObjects();
}

Array<CIMObject> CIMOMHandle::associators(
    const OperationContext& context,
    const CIMNamespaceName& nameSpace,
    const CIMObjectPath& objectName,
    const CIMName& assocClass,
    const CIMName& resultClass,
    const String& role,
    const String& resultRole,
    Boolean includeQualifiers,
    Boolean includeClassOrigin,
    const CIMPropertyList& propertyList)
{
    return _rep->associators(
        context,
        nameSpace,
        objectName,
        assocClass,
        resultClass,
        role,
        resultRole,
        includeQualifiers,
        includeClassOrigin,
        propertyList).getObjects();
}

Array<CIMObjectPath> CIMOMHandle::associatorNames(
    const OperationContext& context,
    const CIMNamespaceName& nameSpace,
    const CIMObjectPath& objectName,
    const CIMName& assocClass,
    const CIMName& resultClass,
    const String& role,
    const String& resultRole)
{
    return _rep->associatorNames(
        context,
        nameSpace,
        objectName,
        assocClass,
        resultClass,
        role,
        resultRole).getInstanceNames();
}

Array<CIMObject> CIMOMHandle::references(
    const OperationContext& context,
    const CIMNamespaceName& nameSpace,
    const CIMObjectPath& objectName,
    const CIMName& resultClass,
    const String& role,
    Boolean includeQualifiers,
    Boolean includeClassOrigin,
    const CIMPropertyList& propertyList)
{
    return _rep->references(
        context,
        nameSpace,
        objectName,
        resultClass,
        role,
        includeQualifiers,
        includeClassOrigin,
        propertyList).getObjects();
}

Array<CIMObjectPath> CIMOMHandle::referenceNames(
    const OperationContext& context,
    const CIMNamespaceName& nameSpace,
    const CIMObjectPath& objectName,
    const CIMName& resultClass,
    const String& role)
{
    return _rep->referenceNames(
        context,
        nameSpace,
        objectName,
        resultClass,
        role).getInstanceNames();
}

CIMValue CIMOMHandle::getProperty(
    const OperationContext& context,
    const CIMNamespaceName& nameSpace,
    const CIMObjectPath& instanceName,
    const CIMName& propertyName)
{
    return _rep->getProperty(
        context,
        nameSpace,
        instanceName,
        propertyName);
}

void CIMOMHandle::setProperty(
    const OperationContext& context,
    const CIMNamespaceName& nameSpace,
    const CIMObjectPath& instanceName,
    const CIMName& propertyName,
    const CIMValue& newValue)
{
    _rep->setProperty(
        context,
        nameSpace,
        instanceName,
        propertyName,
        newValue);
}

CIMValue CIMOMHandle::invokeMethod(
    const OperationContext& context,
    const CIMNamespaceName& nameSpace,
    const CIMObjectPath& instanceName,
    const CIMName& methodName,
    const Array<CIMParamValue>& inParameters,
    Array<CIMParamValue>& outParameters)
{
    return _rep->invokeMethod(
        context,
        nameSpace,
        instanceName,
        methodName,
        inParameters,
        outParameters);
}


void CIMOMHandle::disallowProviderUnload()
{
    _rep->disallowProviderUnload();
}

void CIMOMHandle::allowProviderUnload()
{
    _rep->allowProviderUnload();
}

Boolean CIMOMHandle::unload_ok()
{
    return _rep->unload_ok();
}

#ifdef PEGASUS_USE_EXPERIMENTAL_INTERFACES
OperationContext CIMOMHandle::getResponseContext()
{
    return _rep->getResponseContext();
}
#endif

PEGASUS_NAMESPACE_END
