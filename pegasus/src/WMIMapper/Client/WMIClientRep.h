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
// Modified By:
//
//%////////////////////////////////////////////////////////////////////////////

#ifndef Pegasus_WMIClientRep_h
#define Pegasus_WMIClientRep_h

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
#include <Pegasus/Common/Constants.h>
#include <Pegasus/Common/FileSystem.h>
#include <Pegasus/Common/Monitor.h>
#include <Pegasus/Common/HTTPConnector.h>
#include <Pegasus/Common/CIMMessage.h>
#include <Pegasus/Common/HTTPConnection.h>
#include <Pegasus/Common/XmlWriter.h>
#include <Pegasus/Common/TimeValue.h>
#include <Pegasus/Common/PegasusVersion.h>
#include <Pegasus/Client/CIMClientException.h>
#include <Pegasus/Client/Linkage.h>
#include <Pegasus/Client/CIMClientInterface.h>

PEGASUS_NAMESPACE_BEGIN

///////////////////////////////////////////////////////////////////////////////
//
// WMIClientRep
//
///////////////////////////////////////////////////////////////////////////////
class WMIClientRep : public CIMClientInterface
{
public:

    WMIClientRep(Uint32 timeoutMilliseconds =
        PEGASUS_DEFAULT_CLIENT_TIMEOUT_MILLISECONDS);

    ~WMIClientRep();

    virtual void handleEnqueue();

    void registerClientOpPerformanceDataHandler(
        ClientOpPerformanceDataHandler & handler) {};

    void deregisterClientOpPerformanceDataHandler(){};

    Uint32 getTimeout() const
    {
        return _timeoutMilliseconds;
    }

    void setTimeout(Uint32 timeoutMilliseconds)
    {
        _timeoutMilliseconds = timeoutMilliseconds;
    }

    // l10n start
    AcceptLanguageList getRequestAcceptLanguages() const
    {
        return AcceptLanguageList();
    };

    ContentLanguageList getRequestContentLanguages() const
    {
        return ContentLanguageList();
    };

    ContentLanguageList getResponseContentLanguages() const
    {
        return ContentLanguageList();
    };

    void setRequestAcceptLanguages(const AcceptLanguageList& langs) { };
    void setRequestContentLanguages(const ContentLanguageList& langs) { };
    void setRequestDefaultLanguages() { };
    // l10n end

    void connect(
        const String& host,
        const Uint32 portNumber,
        const String& userName,
        const String& password
        ) { };

    void connect(
        const String& host,
        const Uint32 portNumber,
        const SSLContext& sslContext,
        const String& userName,
        const String& password
    ) { };

    void connectLocal();

    void disconnect();

    virtual CIMClass getClass(
    const CIMNamespaceName& nameSpace,
    const CIMName& className,
    Boolean localOnly = true,
    Boolean includeQualifiers = true,
    Boolean includeClassOrigin = false,
    const CIMPropertyList& propertyList = CIMPropertyList()
    );

    virtual CIMInstance getInstance(
    const CIMNamespaceName& nameSpace,
    const CIMObjectPath& instanceName,
    Boolean localOnly = true,
    Boolean includeQualifiers = false,
    Boolean includeClassOrigin = false,
    const CIMPropertyList& propertyList = CIMPropertyList()
    );

    virtual void deleteClass(
    const CIMNamespaceName& nameSpace,
    const CIMName& className
    );

    virtual void deleteInstance(
    const CIMNamespaceName& nameSpace,
    const CIMObjectPath& instanceName
    );

    virtual void createClass(
    const CIMNamespaceName& nameSpace,
    const CIMClass& newClass
    );

    virtual CIMObjectPath createInstance(
    const CIMNamespaceName& nameSpace,
    const CIMInstance& newInstance
    );

    virtual void modifyClass(
    const CIMNamespaceName& nameSpace,
    const CIMClass& modifiedClass
    );

    virtual void modifyInstance(
    const CIMNamespaceName& nameSpace,
    const CIMInstance& modifiedInstance,
    Boolean includeQualifiers = true,
    const CIMPropertyList& propertyList = CIMPropertyList()
    );

    virtual Array<CIMClass> enumerateClasses(
    const CIMNamespaceName& nameSpace,
    const CIMName& className = CIMName(),
    Boolean deepInheritance = false,
    Boolean localOnly = true,
    Boolean includeQualifiers = true,
    Boolean includeClassOrigin = false
    );

    virtual Array<CIMName> enumerateClassNames(
    const CIMNamespaceName& nameSpace,
    const CIMName& className = CIMName(),
    Boolean deepInheritance = false
    );

    virtual Array<CIMInstance> enumerateInstances(
    const CIMNamespaceName& nameSpace,
    const CIMName& className,
    Boolean deepInheritance = true,
    Boolean localOnly = true,
    Boolean includeQualifiers = false,
    Boolean includeClassOrigin = false,
    const CIMPropertyList& propertyList = CIMPropertyList()
    );

    virtual Array<CIMObjectPath> enumerateInstanceNames(
    const CIMNamespaceName& nameSpace,
    const CIMName& className
    );

    virtual Array<CIMObject> execQuery(
    const CIMNamespaceName& nameSpace,
    const String& queryLanguage,
    const String& query
    );

    virtual Array<CIMObject> associators(
    const CIMNamespaceName& nameSpace,
    const CIMObjectPath& objectName,
    const CIMName& assocClass = CIMName(),
    const CIMName& resultClass = CIMName(),
    const String& role = String::EMPTY,
    const String& resultRole = String::EMPTY,
    Boolean includeQualifiers = false,
    Boolean includeClassOrigin = false,
    const CIMPropertyList& propertyList = CIMPropertyList()
    );

    virtual Array<CIMObjectPath> associatorNames(
    const CIMNamespaceName& nameSpace,
    const CIMObjectPath& objectName,
    const CIMName& assocClass = CIMName(),
    const CIMName& resultClass = CIMName(),
    const String& role = String::EMPTY,
    const String& resultRole = String::EMPTY
    );

    virtual Array<CIMObject> references(
    const CIMNamespaceName& nameSpace,
    const CIMObjectPath& objectName,
    const CIMName& resultClass = CIMName(),
    const String& role = String::EMPTY,
    Boolean includeQualifiers = false,
    Boolean includeClassOrigin = false,
    const CIMPropertyList& propertyList = CIMPropertyList()
    );

    virtual Array<CIMObjectPath> referenceNames(
    const CIMNamespaceName& nameSpace,
    const CIMObjectPath& objectName,
    const CIMName& resultClass = CIMName(),
    const String& role = String::EMPTY
    );

    virtual CIMValue getProperty(
    const CIMNamespaceName& nameSpace,
    const CIMObjectPath& instanceName,
    const CIMName& propertyName
    );

    virtual void setProperty(
    const CIMNamespaceName& nameSpace,
    const CIMObjectPath& instanceName,
    const CIMName& propertyName,
    const CIMValue& newValue = CIMValue()
    );

    virtual CIMQualifierDecl getQualifier(
    const CIMNamespaceName& nameSpace,
    const CIMName& qualifierName
    );

    virtual void setQualifier(
    const CIMNamespaceName& nameSpace,
    const CIMQualifierDecl& qualifierDeclaration
    );

    virtual void deleteQualifier(
    const CIMNamespaceName& nameSpace,
    const CIMName& qualifierName
    );

    virtual Array<CIMQualifierDecl> enumerateQualifiers(
    const CIMNamespaceName& nameSpace
    );

    virtual CIMValue invokeMethod(
    const CIMNamespaceName& nameSpace,
    const CIMObjectPath& instanceName,
    const CIMName& methodName,
    const Array<CIMParamValue>& inParameters,
    Array<CIMParamValue>& outParameters
    );

private:
    Boolean _connected;
    Uint32 _timeoutMilliseconds;
};

PEGASUS_NAMESPACE_END

#endif /* Pegasus_WMIClientRep_h */
