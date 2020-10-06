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

#include "CIMClient.h"
#include <Pegasus/Client/CIMClientRep.h>
#include <Pegasus/Common/PegasusVersion.h>

#include <iostream>
#include <fstream>

PEGASUS_USING_STD;

PEGASUS_NAMESPACE_BEGIN

///////////////////////////////////////////////////////////////////////////////
//
// CIMClient
//
///////////////////////////////////////////////////////////////////////////////

CIMClient::CIMClient()
{
    _rep = new CIMClientRep();
}

CIMClient::~CIMClient()
{
    delete _rep;
}

Uint32 CIMClient::getTimeout() const
{
    return _rep->getTimeout();
}

void CIMClient::setTimeout(Uint32 timeoutMilliseconds)
{
    _rep->setTimeout(timeoutMilliseconds);
}

void CIMClient::connect(
    const String& host,
    const Uint32 portNumber,
    const String& userName,
    const String& password)
{
    _rep->connect(host, portNumber, userName, password);
}

void CIMClient::connect(
    const String& host,
    const Uint32 portNumber,
    const SSLContext& sslContext,
    const String& userName,
    const String& password)
{
    _rep->connect(host, portNumber, sslContext, userName, password);
}

void CIMClient::connectLocal()
{
    _rep->connectLocal();
}

void CIMClient::disconnect()
{
    _rep->disconnect();
}

void CIMClient::setRequestAcceptLanguages(const AcceptLanguageList& langs)
{
    _rep->setRequestAcceptLanguages(langs);
}

AcceptLanguageList CIMClient::getRequestAcceptLanguages() const
{
    return _rep->getRequestAcceptLanguages();
}

void CIMClient::setRequestContentLanguages(const ContentLanguageList& langs)
{
    _rep->setRequestContentLanguages(langs);
}

ContentLanguageList CIMClient::getRequestContentLanguages() const
{
    return _rep->getRequestContentLanguages();
}

ContentLanguageList CIMClient::getResponseContentLanguages() const
{
    return _rep->getResponseContentLanguages();
}

void CIMClient::setRequestDefaultLanguages()
{
    _rep->setRequestDefaultLanguages();
}

CIMClass CIMClient::getClass(
    const CIMNamespaceName& nameSpace,
    const CIMName& className,
    Boolean localOnly,
    Boolean includeQualifiers,
    Boolean includeClassOrigin,
    const CIMPropertyList& propertyList)
{
    return _rep->getClass(
        nameSpace,
        className,
        localOnly,
        includeQualifiers,
        includeClassOrigin,
        propertyList);
}

CIMInstance CIMClient::getInstance(
    const CIMNamespaceName& nameSpace,
    const CIMObjectPath& instanceName,
    Boolean localOnly,
    Boolean includeQualifiers,
    Boolean includeClassOrigin,
    const CIMPropertyList& propertyList)
{
    CIMInstance inst = _rep->getInstance(
        nameSpace,
        instanceName,
        localOnly,
        includeQualifiers,
        includeClassOrigin,
        propertyList).getInstance();

    if (!inst.isUninitialized())
    {
        // remove key bindings, name space and host name form object path.
        CIMObjectPath& p =
            const_cast<CIMObjectPath&>(inst.getPath());

        CIMName cls = p.getClassName();
        p.clear();
        p.setClassName(cls);

    }
#ifdef PEGASUS_ENABLE_PROTOCOL_BINARY
    CIMClientRep * rep = static_cast<CIMClientRep*>(_rep);
    if (rep->_binaryResponse)
    {
        inst.instanceFilter(includeQualifiers,includeClassOrigin,propertyList);
    }
#endif

    return inst;
}

void CIMClient::deleteClass(
    const CIMNamespaceName& nameSpace,
    const CIMName& className)
{
    _rep->deleteClass(
        nameSpace,
        className);
}

void CIMClient::deleteInstance(
    const CIMNamespaceName& nameSpace,
    const CIMObjectPath& instanceName)
{
    _rep->deleteInstance(
        nameSpace,
        instanceName);
}

void CIMClient::createClass(
    const CIMNamespaceName& nameSpace,
    const CIMClass& newClass)
{
    _rep->createClass(
        nameSpace,
        newClass);
}

CIMObjectPath CIMClient::createInstance(
    const CIMNamespaceName& nameSpace,
    const CIMInstance& newInstance)
{
    return _rep->createInstance(
        nameSpace,
        newInstance);
}

void CIMClient::modifyClass(
    const CIMNamespaceName& nameSpace,
    const CIMClass& modifiedClass)
{
    _rep->modifyClass(
        nameSpace,
        modifiedClass);
}

void CIMClient::modifyInstance(
    const CIMNamespaceName& nameSpace,
    const CIMInstance& modifiedInstance,
    Boolean includeQualifiers,
    const CIMPropertyList& propertyList)
{
    _rep->modifyInstance(
        nameSpace,
        modifiedInstance,
        includeQualifiers,
        propertyList);
}

Array<CIMClass> CIMClient::enumerateClasses(
    const CIMNamespaceName& nameSpace,
    const CIMName& className,
    Boolean deepInheritance,
    Boolean localOnly,
    Boolean includeQualifiers,
    Boolean includeClassOrigin)
{
    return _rep->enumerateClasses(
        nameSpace,
        className,
        deepInheritance,
        localOnly,
        includeQualifiers,
        includeClassOrigin);
}

Array<CIMName> CIMClient::enumerateClassNames(
    const CIMNamespaceName& nameSpace,
    const CIMName& className,
    Boolean deepInheritance)
{
    return _rep->enumerateClassNames(
        nameSpace,
        className,
        deepInheritance);
}

Array<CIMInstance> CIMClient::enumerateInstances(
    const CIMNamespaceName& nameSpace,
    const CIMName& className,
    Boolean deepInheritance,
    Boolean localOnly,
    Boolean includeQualifiers,
    Boolean includeClassOrigin,
    const CIMPropertyList& propertyList)
{
#ifndef PEGASUS_ENABLE_PROTOCOL_BINARY
    Array<CIMInstance> a = _rep->enumerateInstances(
            nameSpace,
            className,
            deepInheritance,
            localOnly,
            includeQualifiers,
            includeClassOrigin,
            propertyList).getInstances();
#else
    CIMResponseData respData = _rep->enumerateInstances(
        nameSpace,
        className,
        deepInheritance,
        localOnly,
        includeQualifiers,
        includeClassOrigin,
        propertyList);

    Array<CIMInstance> a = respData.getInstances();
    CIMClientRep * rep = static_cast<CIMClientRep*>(_rep);

    if (rep->_binaryResponse)
    {
        CIMPropertyList returnedPropList = respData.getPropertyList();
        for (Uint32 i = 0, n = a.size(); i < n ; i++)
        {
            CIMInstance & inst = a[i];
            inst.instanceFilter(
                includeQualifiers,
                includeClassOrigin,
                returnedPropList);
        }
    }
#endif
    // remove name space and host name to be instance names
    for (Uint32 i = 0, n = a.size(); i < n ; i++)
    {
        if (!a[i].isUninitialized())
        {
            CIMObjectPath& p = const_cast<CIMObjectPath&>(a[i].getPath());
            p.setNameSpace(CIMNamespaceName());
            p.setHost(String());
        }
    }

    return a;
}

Array<CIMObjectPath> CIMClient::enumerateInstanceNames(
    const CIMNamespaceName& nameSpace,
    const CIMName& className)
{

    Array<CIMObjectPath> p = _rep->enumerateInstanceNames(
        nameSpace,
        className).getInstanceNames();

    // remove name space and host name from object paths to be
    // instance names.
    for (Uint32 i = 0, n = p.size(); i < n ; i++)
    {
        p[i].setNameSpace(CIMNamespaceName());
        p[i].setHost(String());
    }

    return p;

}

Array<CIMObject> CIMClient::execQuery(
    const CIMNamespaceName& nameSpace,
    const String& queryLanguage,
    const String& query)
{
    return _rep->execQuery(
        nameSpace,
        queryLanguage,
        query).getObjects();
}

Array<CIMObject> CIMClient::associators(
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
    CIMResponseData respData = _rep->associators(
        nameSpace,
        objectName,
        assocClass,
        resultClass,
        role,
        resultRole,
        includeQualifiers,
        includeClassOrigin,
        propertyList);

    Array<CIMObject> a = respData.getObjects();
#ifdef PEGASUS_ENABLE_PROTOCOL_BINARY
    CIMClientRep * rep = static_cast<CIMClientRep*>(_rep);
    if (rep->_binaryResponse)
    {
        CIMPropertyList returnedPropList = respData.getPropertyList();
        if ((a.size() > 0) && (a[0].isInstance()))
        {
            for (Uint32 i = 0, n = a.size(); i < n ; i++)
            {
                CIMObject & obj = a[i];
                obj.instanceFilter(
                    includeQualifiers,
                    includeClassOrigin,
                    propertyList);
            }
        }
    }
#endif
    return a;
}

Array<CIMObjectPath> CIMClient::associatorNames(
    const CIMNamespaceName& nameSpace,
    const CIMObjectPath& objectName,
    const CIMName& assocClass,
    const CIMName& resultClass,
    const String& role,
    const String& resultRole)
{
    return _rep->associatorNames(
        nameSpace,
        objectName,
        assocClass,
        resultClass,
        role,
        resultRole).getInstanceNames();
}

Array<CIMObject> CIMClient::references(
    const CIMNamespaceName& nameSpace,
    const CIMObjectPath& objectName,
    const CIMName& resultClass,
    const String& role,
    Boolean includeQualifiers,
    Boolean includeClassOrigin,
    const CIMPropertyList& propertyList)
{
    CIMResponseData respData = _rep->references(
        nameSpace,
        objectName,
        resultClass,
        role,
        includeQualifiers,
        includeClassOrigin,
        propertyList);
    Array<CIMObject> a = respData.getObjects();

#ifdef PEGASUS_ENABLE_PROTOCOL_BINARY
    CIMClientRep * rep = static_cast<CIMClientRep*>(_rep);
    if (rep->_binaryResponse)
    {
        CIMPropertyList returnedPropList = respData.getPropertyList();
        if ((a.size() > 0) && (a[0].isInstance()))
        {
            for (Uint32 i = 0, n = a.size(); i < n ; i++)
            {
                CIMObject & obj = a[i];
                obj.instanceFilter(
                    includeQualifiers,
                    includeClassOrigin,
                    propertyList);
            }
        }
    }
#endif
    return a;
}

Array<CIMObjectPath> CIMClient::referenceNames(
    const CIMNamespaceName& nameSpace,
    const CIMObjectPath& objectName,
    const CIMName& resultClass,
    const String& role)
{
    return _rep->referenceNames(
        nameSpace,
        objectName,
        resultClass,
        role).getInstanceNames();
}

CIMValue CIMClient::getProperty(
    const CIMNamespaceName& nameSpace,
    const CIMObjectPath& instanceName,
    const CIMName& propertyName)
{
    return _rep->getProperty(
        nameSpace,
        instanceName,
        propertyName);
}

void CIMClient::setProperty(
    const CIMNamespaceName& nameSpace,
    const CIMObjectPath& instanceName,
    const CIMName& propertyName,
    const CIMValue& newValue)
{
    _rep->setProperty(
        nameSpace,
        instanceName,
        propertyName,
        newValue);
}

CIMQualifierDecl CIMClient::getQualifier(
    const CIMNamespaceName& nameSpace,
    const CIMName& qualifierName)
{
    return _rep->getQualifier(
        nameSpace,
        qualifierName);
}

void CIMClient::setQualifier(
    const CIMNamespaceName& nameSpace,
    const CIMQualifierDecl& qualifierDeclaration)
{
    _rep->setQualifier(
        nameSpace,
        qualifierDeclaration);
}

void CIMClient::deleteQualifier(
    const CIMNamespaceName& nameSpace,
    const CIMName& qualifierName)
{
    _rep->deleteQualifier(
        nameSpace,
        qualifierName);
}

Array<CIMQualifierDecl> CIMClient::enumerateQualifiers(
    const CIMNamespaceName& nameSpace)
{
    return _rep->enumerateQualifiers(
        nameSpace);
}

CIMValue CIMClient::invokeMethod(
    const CIMNamespaceName& nameSpace,
    const CIMObjectPath& instanceName,
    const CIMName& methodName,
    const Array<CIMParamValue>& inParameters,
    Array<CIMParamValue>& outParameters)
{
    return _rep->invokeMethod(
        nameSpace,
        instanceName,
        methodName,
        inParameters,
        outParameters);
}

// KS_PULL_BEGIN
Array<CIMInstance> CIMClient::openEnumerateInstances(
    CIMEnumerationContext& enumerationContext,
    Boolean& endOfSequence,
    const CIMNamespaceName& nameSpace,
    const CIMName& className,
    Boolean deepInheritance,
    Boolean includeClassOrigin,
    const CIMPropertyList& propertyList,
    const String& filterQueryLanguage,
    const String& filterQuery,
    const Uint32Arg& operationTimeout,
    Boolean continueOnError,
    Uint32 maxObjectCount)
{
    return _rep->openEnumerateInstances(
        enumerationContext,
        endOfSequence,
        nameSpace,
        className,
        deepInheritance,
        includeClassOrigin,
        propertyList,
        filterQueryLanguage,
        filterQuery,
        operationTimeout,
        continueOnError,
        maxObjectCount).getInstances();
}

Array<CIMObjectPath> CIMClient::openEnumerateInstancePaths(
    CIMEnumerationContext& enumerationContext,
    Boolean& endOfSequence,
    const CIMNamespaceName& nameSpace,
    const CIMName& className,
    const String& filterQueryLanguage,
    const String& filterQuery,
    const Uint32Arg& operationTimeout,
    Boolean continueOnError,
    Uint32 maxObjectCount)
{

    return _rep->openEnumerateInstancePaths(
        enumerationContext,
        endOfSequence,
        nameSpace,
        className,
        filterQueryLanguage,
        filterQuery,
        operationTimeout,
        continueOnError,
        maxObjectCount).getInstanceNames();
}

Array<CIMInstance> CIMClient::openReferenceInstances(
    CIMEnumerationContext& enumerationContext,
    Boolean& endOfSequence,
    const CIMNamespaceName& nameSpace,
    const CIMObjectPath& objectName,
    const CIMName& resultClass,
    const String& role,
    const Boolean includeClassOrigin,
    const CIMPropertyList& propertyList,
    const String& filterQueryLanguage,
    const String& filterQuery,
    const Uint32Arg& operationTimeout,
    Boolean continueOnError,
    Uint32 maxObjectCount
    )
{
    return _rep->openReferenceInstances(
        enumerationContext,
        endOfSequence,
        nameSpace,
        objectName,
        resultClass,
        role,
        includeClassOrigin,
        propertyList,
        filterQueryLanguage,
        filterQuery,
        operationTimeout,
        continueOnError,
        maxObjectCount).getInstancesFromInstancesOrObjects();
}

Array<CIMObjectPath> CIMClient::openReferenceInstancePaths(
    CIMEnumerationContext& enumerationContext,
    Boolean& endOfSequence,
    const CIMNamespaceName& nameSpace,
    const CIMObjectPath& objectName,
    const CIMName& resultClass,
    const String& role,
    const String& filterQueryLanguage,
    const String& filterQuery,
    const Uint32Arg& operationTimeout,
    Boolean continueOnError,
    Uint32 maxObjectCount
    )
{
    return _rep->openReferenceInstancePaths(
        enumerationContext,
        endOfSequence,
        nameSpace,
        objectName,
        resultClass,
        role,
        filterQueryLanguage,
        filterQuery,
        operationTimeout,
        continueOnError,
        maxObjectCount).getInstanceNames();
}

Array<CIMInstance> CIMClient::openAssociatorInstances(
    CIMEnumerationContext& enumerationContext,
    Boolean& endOfSequence,
    const CIMNamespaceName& nameSpace,
    const CIMObjectPath& objectName,
    const CIMName& assocClass,
    const CIMName& resultClass,
    const String& role,
    const String& resultRole,
    Boolean includeClassOrigin,
    const CIMPropertyList& propertyList,
    const String& filterQueryLanguage,
    const String& filterQuery,
    const Uint32Arg& operationTimeout,
    Boolean continueOnError,
    Uint32 maxObjectCount
    )
{
    return _rep->openAssociatorInstances(
        enumerationContext,
        endOfSequence,
        nameSpace,
        objectName,
        assocClass,
        resultClass,
        role,
        resultRole,
        includeClassOrigin,
        propertyList,
        filterQueryLanguage,
        filterQuery,
        operationTimeout,
        continueOnError,
        maxObjectCount).getInstancesFromInstancesOrObjects();
}

Array<CIMObjectPath> CIMClient::openAssociatorInstancePaths(
    CIMEnumerationContext& enumerationContext,
    Boolean& endOfSequence,
    const CIMNamespaceName& nameSpace,
    const CIMObjectPath& objectName,
    const CIMName& assocClass,
    const CIMName& resultClass,
    const String& role,
    const String& resultRole,
    const String& filterQueryLanguage,
    const String& filterQuery,
    const Uint32Arg& operationTimeout,
    Boolean continueOnError,
    Uint32 maxObjectCount
    )
{
    return _rep->openAssociatorInstancePaths(
        enumerationContext,
        endOfSequence,
        nameSpace,
        objectName,
        assocClass,
        resultClass,
        role,
        resultRole,
        filterQueryLanguage,
        filterQuery,
        operationTimeout,
        continueOnError,
        maxObjectCount).getInstanceNames();
}


Array<CIMInstance> CIMClient::openQueryInstances(
        CIMEnumerationContext& enumerationContext,
        Boolean& endOfSequence,
        const CIMNamespaceName& nameSpace,
        const String& queryLanguage,
        const String& query,
        CIMClass& queryResultClass,
        Boolean returnQueryResultClass,
        const Uint32Arg& operationTimeout,
        Boolean continueOnError,
        Uint32 maxObjectCount)
{
    return _rep->openQueryInstances(
        enumerationContext,
        endOfSequence,
        nameSpace,
        queryLanguage,
        query,
        queryResultClass,
        returnQueryResultClass,
        operationTimeout,
        continueOnError,
        maxObjectCount).getInstances();
}

Array<CIMInstance> CIMClient::pullInstancesWithPath(
    CIMEnumerationContext& enumerationContext,
    Boolean& endOfSequence,
    Uint32 maxObjectCount)
{
    return _rep->pullInstancesWithPath(
        enumerationContext,
        endOfSequence,
        maxObjectCount).getInstancesFromInstancesOrObjects();
}

Array<CIMObjectPath> CIMClient::pullInstancePaths(
    CIMEnumerationContext& enumerationContext,
    Boolean& endOfSequence,
    Uint32 maxObjectCount)
{

    return _rep->pullInstancePaths(
        enumerationContext,
        endOfSequence,
        maxObjectCount).getInstanceNames();
}

Array<CIMInstance> CIMClient::pullInstances(
    CIMEnumerationContext& enumerationContext,
    Boolean& endOfSequence,
    Uint32 maxObjectCount)
{
    return _rep->pullInstances(
        enumerationContext,
        endOfSequence,
        maxObjectCount).getInstancesFromInstancesOrObjects();
}

void CIMClient::closeEnumeration(
    CIMEnumerationContext& enumerationContext)
{
    return _rep->closeEnumeration(
        enumerationContext);
}

Uint64Arg CIMClient::enumerationCount(
    CIMEnumerationContext& enumerationContext)
{
    return _rep->enumerationCount(
        enumerationContext);
}

// KS_PULL_END

void CIMClient::registerClientOpPerformanceDataHandler(
    ClientOpPerformanceDataHandler& handler)
{
    _rep->registerClientOpPerformanceDataHandler(handler);
}

void CIMClient::deregisterClientOpPerformanceDataHandler()
{
    _rep->deregisterClientOpPerformanceDataHandler();
}

PEGASUS_NAMESPACE_END
