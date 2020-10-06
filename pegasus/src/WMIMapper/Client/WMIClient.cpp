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
// Author: Jair Santos, Hewlett-Packard Company (jair.santos@hp.com)
//
// Modified By: Terry Martin, Hewlett-Packard Company (terry.martin@hp.com)
//
//%////////////////////////////////////////////////////////////////////////////

#include <Pegasus/Client/CIMClient.h>
#include <Pegasus/Client/CIMClientRep.h>
#include "WMIClientRep.h"

#include <iostream>
#include <fstream>

PEGASUS_USING_STD;

PEGASUS_NAMESPACE_BEGIN

///////////////////////////////////////////////////////////////////////////////
//
// WMIClient
//
///////////////////////////////////////////////////////////////////////////////
CIMClient::CIMClient()
{
    // The WMI Mapper has its own implementation of CIMClient, in order to
    // handle localConnect()'s on Windows (for localConnect's on Windows,
    // we simply call functions in the WMI Provider .dll directly, rather than
    // through using a domain socket/local authentication as is done on Unix).
    //
    // For this, we handle 2 separate cccClientRep classes: CIMClientRep (the
    // standard Pegasus client rep) is used for remote connections, and our
    // custom WMI rep (WMIClientRep) is used for local connections. So, the
    // basic idea is to instantiate the appropriate _rep, depending on the
    // type of connection being used (local or remote).
    //
    // A problem arises in the storing of the timeout value: The user must be
    // able to set this value independently of the current connection state
    // (e.g., construct CIMClient, set timeout, then connect). In the standard
    // Pegasus implementation of CIMClient, the timeout value is store in the
    // CIMClientRep object, not in the CIMClient object. The problem is, we
    // need to persist this value while potentially deleting and constructing
    // new _rep objects -- and we cannot add the timeout as a member of our
    // version of CIMClient, or this will create memory corruption problems for
    // client apps (since they #include <Pegasus/Client/CIMClient.h>, but link
    // to the WMI Mapper implementation of this class -- at least to utilize
    // the Mapper's localConnect feature (found this out the hard way)!
    //
    // So, in order to handle this, we must construct a WMIClientRep object
    // here, if only as a container for the timeout setting, then in the
    // connect() methods, we must save the timeout, delete the current _rep,
    // create a new _rep using the current timeout, then finally connect()!
    //
    // Not pretty, but the best solution so far (also, there is very little
    // overhead in constructing a WMIClientRep object here -- even if its only
    // purpose is to be a container for the timeout value!

    _rep = new WMIClientRep();
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
    const String& password
)
{
    // See the comment in the constructor for why we need to get the
    // current timeout, delete and create a new _rep object here:
    Uint32 timeout = _rep->getTimeout();
    delete _rep;
    _rep = new CIMClientRep(timeout);

    _rep->connect(host, portNumber, userName, password);
}

void CIMClient::connect(
    const String& host,
    const Uint32 portNumber,
    const SSLContext& sslContext,
    const String& userName,
    const String& password
)
{
    // See the comment in the constructor for why we need to get the
    // current timeout, delete and create a new _rep object here:
    Uint32 timeout = _rep->getTimeout();
    delete _rep;
    _rep = new CIMClientRep(timeout);

    _rep->connect(host, portNumber, sslContext, userName, password);
}

void CIMClient::connectLocal()
{
    // See the comment in the constructor for why we need to get the
    // current timeout, delete and create a new _rep object here:
    Uint32 timeout = _rep->getTimeout();
    delete _rep;
    // For connectLocal, we use WMIClientRep, rather than CIMClientRep:
    _rep = new WMIClientRep(timeout);

    _rep->connectLocal();
}

void CIMClient::disconnect()
{
    _rep->disconnect();
}

// l10n start
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
// l10n end

CIMClass CIMClient::getClass(
    const CIMNamespaceName& nameSpace,
    const CIMName& className,
    Boolean localOnly,
    Boolean includeQualifiers,
    Boolean includeClassOrigin,
    const CIMPropertyList& propertyList
)
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
    const CIMPropertyList& propertyList
)
{
    return _rep->getInstance(
        nameSpace,
        instanceName,
        localOnly,
        includeQualifiers,
        includeClassOrigin,
        propertyList);
}

void CIMClient::deleteClass(
    const CIMNamespaceName& nameSpace,
    const CIMName& className
)
{
    _rep->deleteClass(
        nameSpace,
        className);
}

void CIMClient::deleteInstance(
    const CIMNamespaceName& nameSpace,
    const CIMObjectPath& instanceName
)
{
    _rep->deleteInstance(
        nameSpace,
        instanceName);
}

void CIMClient::createClass(
    const CIMNamespaceName& nameSpace,
    const CIMClass& newClass
)
{
    _rep->createClass(
        nameSpace,
        newClass);
}

CIMObjectPath CIMClient::createInstance(
    const CIMNamespaceName& nameSpace,
    const CIMInstance& newInstance
)
{
    return _rep->createInstance(
        nameSpace,
        newInstance);
}

void CIMClient::modifyClass(
    const CIMNamespaceName& nameSpace,
    const CIMClass& modifiedClass
)
{
    _rep->modifyClass(
        nameSpace,
        modifiedClass);
}

void CIMClient::modifyInstance(
    const CIMNamespaceName& nameSpace,
    const CIMInstance& modifiedInstance,
    Boolean includeQualifiers,
    const CIMPropertyList& propertyList
)
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
    Boolean includeClassOrigin
)
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
    Boolean deepInheritance
)
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
    const CIMPropertyList& propertyList
)
{
    return _rep->enumerateInstances(
        nameSpace,
        className,
        deepInheritance,
        localOnly,
        includeQualifiers,
        includeClassOrigin,
        propertyList);
}

Array<CIMObjectPath> CIMClient::enumerateInstanceNames(
    const CIMNamespaceName& nameSpace,
    const CIMName& className
)
{
    return _rep->enumerateInstanceNames(
        nameSpace,
        className);
}

Array<CIMObject> CIMClient::execQuery(
    const CIMNamespaceName& nameSpace,
    const String& queryLanguage,
    const String& query
)
{
    return _rep->execQuery(
        nameSpace,
        queryLanguage,
        query);
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
    const CIMPropertyList& propertyList
)
{
    return _rep->associators(
        nameSpace,
        objectName,
        assocClass,
        resultClass,
        role,
        resultRole,
        includeQualifiers,
        includeClassOrigin,
        propertyList);
}

Array<CIMObjectPath> CIMClient::associatorNames(
    const CIMNamespaceName& nameSpace,
    const CIMObjectPath& objectName,
    const CIMName& assocClass,
    const CIMName& resultClass,
    const String& role,
    const String& resultRole
)
{
    return _rep->associatorNames(
        nameSpace,
        objectName,
        assocClass,
        resultClass,
        role,
        resultRole);
}

Array<CIMObject> CIMClient::references(
    const CIMNamespaceName& nameSpace,
    const CIMObjectPath& objectName,
    const CIMName& resultClass,
    const String& role,
    Boolean includeQualifiers,
    Boolean includeClassOrigin,
    const CIMPropertyList& propertyList
)
{
    return _rep->references(
        nameSpace,
        objectName,
        resultClass,
        role,
        includeQualifiers,
        includeClassOrigin,
        propertyList);
}

Array<CIMObjectPath> CIMClient::referenceNames(
    const CIMNamespaceName& nameSpace,
    const CIMObjectPath& objectName,
    const CIMName& resultClass,
    const String& role
)
{
    return _rep->referenceNames(
        nameSpace,
        objectName,
        resultClass,
        role);
}

CIMValue CIMClient::getProperty(
    const CIMNamespaceName& nameSpace,
    const CIMObjectPath& instanceName,
    const CIMName& propertyName
)
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
    const CIMValue& newValue
)
{
    _rep->setProperty(
        nameSpace,
        instanceName,
        propertyName,
        newValue);
}

CIMQualifierDecl CIMClient::getQualifier(
    const CIMNamespaceName& nameSpace,
    const CIMName& qualifierName
)
{
    return _rep->getQualifier(
        nameSpace,
        qualifierName);
}

void CIMClient::setQualifier(
    const CIMNamespaceName& nameSpace,
    const CIMQualifierDecl& qualifierDeclaration
)
{
    _rep->setQualifier(
        nameSpace,
        qualifierDeclaration);
}

void CIMClient::deleteQualifier(
    const CIMNamespaceName& nameSpace,
    const CIMName& qualifierName
)
{
    _rep->deleteQualifier(
        nameSpace,
        qualifierName);
}

Array<CIMQualifierDecl> CIMClient::enumerateQualifiers(
    const CIMNamespaceName& nameSpace
)
{
    return _rep->enumerateQualifiers(
        nameSpace);
}

CIMValue CIMClient::invokeMethod(
    const CIMNamespaceName& nameSpace,
    const CIMObjectPath& instanceName,
    const CIMName& methodName,
    const Array<CIMParamValue>& inParameters,
    Array<CIMParamValue>& outParameters
)
{
    return _rep->invokeMethod(
        nameSpace,
        instanceName,
        methodName,
        inParameters,
        outParameters);
}

PEGASUS_NAMESPACE_END
