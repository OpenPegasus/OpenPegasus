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

#ifndef Pegasus_ClientInterface_h
#define Pegasus_ClientInterface_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/String.h>
#include <Pegasus/Common/CIMName.h>
#include <Pegasus/Common/SSLContext.h>
#include <Pegasus/Common/CIMObject.h>
#include <Pegasus/Common/CIMClass.h>
#include <Pegasus/Common/CIMInstance.h>
#include <Pegasus/Common/CIMObjectPath.h>
#include <Pegasus/Common/CIMValue.h>
#include <Pegasus/Common/CIMParamValue.h>
#include <Pegasus/Common/CIMPropertyList.h>
#include <Pegasus/Common/CIMQualifierDecl.h>
#include <Pegasus/Common/Exception.h>
#include <Pegasus/Common/MessageQueue.h>
#include <Pegasus/Common/Constants.h>
#include <Pegasus/Common/ContentLanguageList.h>
#include <Pegasus/Common/UintArgs.h>                     // PEP 317
#include <Pegasus/Client/CIMEnumerationContext.h>
#include <Pegasus/Common/CIMResponseData.h>
#include <Pegasus/Client/ClientOpPerformanceDataHandler.h> //PEP# 128

PEGASUS_NAMESPACE_BEGIN

///////////////////////////////////////////////////////////////////////////////
//
// CIMClientInterface
//
///////////////////////////////////////////////////////////////////////////////

class CIMClientInterface : virtual public MessageQueue
{
public:


    CIMClientInterface() :
        MessageQueue(PEGASUS_QUEUENAME_CLIENT)
    { };

    ~CIMClientInterface() { };

    virtual void handleEnqueue() = 0;

    virtual Uint32 getTimeout() const = 0;

    virtual void setTimeout(Uint32 timeoutMilliseconds) = 0;

    virtual AcceptLanguageList getRequestAcceptLanguages() const = 0;
    virtual ContentLanguageList getRequestContentLanguages() const = 0;
    virtual ContentLanguageList getResponseContentLanguages() const = 0;
    virtual void setRequestAcceptLanguages(const AcceptLanguageList& langs) = 0;
    virtual void setRequestContentLanguages(
        const ContentLanguageList& langs) = 0;
    virtual void setRequestDefaultLanguages() = 0;

    virtual void registerClientOpPerformanceDataHandler(
        ClientOpPerformanceDataHandler& handler) = 0;
    virtual void deregisterClientOpPerformanceDataHandler() = 0;

    virtual void connect(
        const String& host,
        const Uint32 portNumber,
        const String& userName,
        const String& password) = 0;

    virtual void connect(
        const String& host,
        const Uint32 portNumber,
        const SSLContext& sslContext,
        const String& userName,
        const String& password) = 0;

    virtual void connectLocal() = 0;

    virtual void disconnect() = 0;

    virtual CIMClass getClass(
        const CIMNamespaceName& nameSpace,
        const CIMName& className,
        Boolean localOnly = true,
        Boolean includeQualifiers = true,
        Boolean includeClassOrigin = false,
        const CIMPropertyList& propertyList = CIMPropertyList()) = 0;

    virtual CIMResponseData getInstance(
        const CIMNamespaceName& nameSpace,
        const CIMObjectPath& instanceName,
        Boolean localOnly = true,
        Boolean includeQualifiers = false,
        Boolean includeClassOrigin = false,
        const CIMPropertyList& propertyList = CIMPropertyList()) = 0;

    virtual void deleteClass(
        const CIMNamespaceName& nameSpace,
        const CIMName& className) = 0;

    virtual void deleteInstance(
        const CIMNamespaceName& nameSpace,
        const CIMObjectPath& instanceName) = 0;

    virtual void createClass(
        const CIMNamespaceName& nameSpace,
        const CIMClass& newClass) = 0;

    virtual CIMObjectPath createInstance(
        const CIMNamespaceName& nameSpace,
        const CIMInstance& newInstance) = 0;

    virtual void modifyClass(
        const CIMNamespaceName& nameSpace,
        const CIMClass& modifiedClass) = 0;

    virtual void modifyInstance(
        const CIMNamespaceName& nameSpace,
        const CIMInstance& modifiedInstance,
        Boolean includeQualifiers = true,
        const CIMPropertyList& propertyList = CIMPropertyList()) = 0;

    virtual Array<CIMClass> enumerateClasses(
        const CIMNamespaceName& nameSpace,
        const CIMName& className = CIMName(),
        Boolean deepInheritance = false,
        Boolean localOnly = true,
        Boolean includeQualifiers = true,
        Boolean includeClassOrigin = false) = 0;

    virtual Array<CIMName> enumerateClassNames(
        const CIMNamespaceName& nameSpace,
        const CIMName& className = CIMName(),
        Boolean deepInheritance = false) = 0;

    virtual CIMResponseData enumerateInstances(
        const CIMNamespaceName& nameSpace,
        const CIMName& className,
        Boolean deepInheritance = true,
        Boolean localOnly = true,
        Boolean includeQualifiers = false,
        Boolean includeClassOrigin = false,
        const CIMPropertyList& propertyList = CIMPropertyList()) = 0;

    virtual CIMResponseData enumerateInstanceNames(
        const CIMNamespaceName& nameSpace,
        const CIMName& className) = 0;

    virtual CIMResponseData execQuery(
        const CIMNamespaceName& nameSpace,
        const String& queryLanguage,
        const String& query) = 0;

    virtual CIMResponseData associators(
        const CIMNamespaceName& nameSpace,
        const CIMObjectPath& objectName,
        const CIMName& assocClass = CIMName(),
        const CIMName& resultClass = CIMName(),
        const String& role = String::EMPTY,
        const String& resultRole = String::EMPTY,
        Boolean includeQualifiers = false,
        Boolean includeClassOrigin = false,
        const CIMPropertyList& propertyList = CIMPropertyList()) = 0;

    virtual CIMResponseData associatorNames(
        const CIMNamespaceName& nameSpace,
        const CIMObjectPath& objectName,
        const CIMName& assocClass = CIMName(),
        const CIMName& resultClass = CIMName(),
        const String& role = String::EMPTY,
        const String& resultRole = String::EMPTY) = 0;

    virtual CIMResponseData references(
        const CIMNamespaceName& nameSpace,
        const CIMObjectPath& objectName,
        const CIMName& resultClass = CIMName(),
        const String& role = String::EMPTY,
        Boolean includeQualifiers = false,
        Boolean includeClassOrigin = false,
        const CIMPropertyList& propertyList = CIMPropertyList()) = 0;

    virtual CIMResponseData referenceNames(
        const CIMNamespaceName& nameSpace,
        const CIMObjectPath& objectName,
        const CIMName& resultClass = CIMName(),
        const String& role = String::EMPTY) = 0;

    virtual CIMValue getProperty(
        const CIMNamespaceName& nameSpace,
        const CIMObjectPath& instanceName,
        const CIMName& propertyName) = 0;

    virtual void setProperty(
        const CIMNamespaceName& nameSpace,
        const CIMObjectPath& instanceName,
        const CIMName& propertyName,
        const CIMValue& newValue = CIMValue()) = 0;

    virtual CIMQualifierDecl getQualifier(
        const CIMNamespaceName& nameSpace,
        const CIMName& qualifierName) = 0;

    virtual void setQualifier(
        const CIMNamespaceName& nameSpace,
        const CIMQualifierDecl& qualifierDeclaration) = 0;

    virtual void deleteQualifier(
        const CIMNamespaceName& nameSpace,
        const CIMName& qualifierName) = 0;

    virtual Array<CIMQualifierDecl> enumerateQualifiers(
        const CIMNamespaceName& nameSpace) = 0;

    virtual CIMValue invokeMethod(
        const CIMNamespaceName& nameSpace,
        const CIMObjectPath& instanceName,
        const CIMName& methodName,
        const Array<CIMParamValue>& inParameters,
        Array<CIMParamValue>& outParameters) = 0;

// EXP_PULL_BEGIN
    virtual CIMResponseData openEnumerateInstances(
        CIMEnumerationContext& enumerationContext,
        Boolean& EndOfSequence,
        const CIMNamespaceName& nameSpace,
        const CIMName& className,
        const Boolean deepInheritance,
        const Boolean includeClassOrigin,
        const CIMPropertyList& propertyList = CIMPropertyList(),
        const String& filterQueryLanguage = String::EMPTY,
        const String& filterQuery = String::EMPTY,
        const Uint32Arg& operationTimeout = Uint32Arg(),    // Default is NULL
        Boolean continueOnError = false,
        Uint32 maxObjectCount = 0) = 0;

    virtual CIMResponseData openEnumerateInstancePaths(
        CIMEnumerationContext& enumerationContext,
        Boolean& EndOfSequence,
        const CIMNamespaceName& nameSpace,
        const CIMName& className,
        const String& filterQueryLanguage = String(),
        const String& filterQuery = String(),
        const Uint32Arg& operationTimeout = Uint32Arg(),    // Default is NULL
        Boolean continueOnError = false,
        Uint32 maxObjectCount = 0) = 0;

    virtual CIMResponseData openReferenceInstances(
        CIMEnumerationContext& enumerationContext,
        Boolean& EndOfSequence,
        const CIMNamespaceName& nameSpace,
        const CIMObjectPath& objectName,
        const CIMName& resultClass = CIMName(),
        const String& role = String::EMPTY,
        Boolean includeClassOrigin = false,
        const CIMPropertyList& propertyList = CIMPropertyList(),
        const String& filterQueryLanguage = String(),
        const String& filterQuery = String(),
        const Uint32Arg& operationTimeout = Uint32Arg(),   // Default is NULL
        Boolean continueOnError = false,
        Uint32 maxObjectCount = 0
        ) = 0;

    virtual CIMResponseData openReferenceInstancePaths(
        CIMEnumerationContext& enumerationContext,
        Boolean& EndOfSequence,
        const CIMNamespaceName& nameSpace,
        const CIMObjectPath& objectName,
        const CIMName& resultClass = CIMName(),
        const String& role = String::EMPTY,
        const String& filterQueryLanguage = String(),
        const String& filterQuery = String(),
        const Uint32Arg& operationTimeout = Uint32Arg(),   // Default is NULL
        Boolean continueOnError = false,
        Uint32 maxObjectCount = 0
        ) = 0;

    virtual CIMResponseData openAssociatorInstances(
        CIMEnumerationContext& enumerationContext,
        Boolean& EndOfSequence,
        const CIMNamespaceName& nameSpace,
        const CIMObjectPath& objectName,
        const CIMName& assocClass = CIMName(),
        const CIMName& resultClass = CIMName(),
        const String& role = String::EMPTY,
        const String& resultRole = String::EMPTY,
        Boolean includeClassOrigin = false,
        const CIMPropertyList& propertyList = CIMPropertyList(),
        const String& filterQueryLanguage = String(),
        const String& filterQuery = String(),
        const Uint32Arg& operationTimeout = Uint32Arg(),   // Default is NULL
        Boolean continueOnError = false,
        Uint32 maxObjectCount = 0
        ) = 0;

    virtual CIMResponseData openAssociatorInstancePaths(
        CIMEnumerationContext& enumerationContext,
        Boolean& EndOfSequence,
        const CIMNamespaceName& nameSpace,
        const CIMObjectPath& objectName,
        const CIMName& assocClass = CIMName(),
        const CIMName& resultClass = CIMName(),
        const String& role = String::EMPTY,
        const String& resultRole = String::EMPTY,
        const String& filterQueryLanguage = String(),
        const String& filterQuery = String(),
        const Uint32Arg& operationTimeout = Uint32Arg(),   // Default is NULL
        Boolean continueOnError = false,
        Uint32 maxObjectCount = 0     // Default is 0
        ) = 0;

    virtual CIMResponseData openQueryInstances(
        CIMEnumerationContext& enumerationContext,
        Boolean& endOfSequence,
        const CIMNamespaceName& nameSpace,
        const String& queryLanguage,
        const String& query,
        CIMClass& queryResultClass,
        Boolean returnQueryResultClass = false,
        const Uint32Arg& operationTimeout = Uint32Arg(),
        Boolean continueOnError = false,
        Uint32 maxObjectCount = 0
        ) = 0;

    virtual CIMResponseData pullInstancesWithPath(
        CIMEnumerationContext& enumerationContext,
        Boolean& endOfSequence,
        Uint32 MaxObjectCount) = 0;

    virtual CIMResponseData pullInstancePaths(
        CIMEnumerationContext& enumerationContext,
        Boolean& endOfSequence,
        Uint32 MaxObjectCount ) = 0;

    virtual CIMResponseData pullInstances(
        CIMEnumerationContext& enumerationContext,
        Boolean& endOfSequence,
        Uint32 MaxObjectCount) = 0;

    virtual void closeEnumeration(
        CIMEnumerationContext& enumerationContext ) = 0;

    virtual Uint64Arg enumerationCount(
        CIMEnumerationContext& enumerationContext ) = 0;

// EXP_PULL_END

};

PEGASUS_NAMESPACE_END

#endif /* Pegasus_Client_h */
