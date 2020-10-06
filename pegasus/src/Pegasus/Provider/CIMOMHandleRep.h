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


#ifndef Pegasus_CIMOMHandleRep_h
#define Pegasus_CIMOMHandleRep_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/Mutex.h>
#include <Pegasus/Common/OperationContext.h>
#include <Pegasus/Common/CIMObject.h>
#include <Pegasus/Common/CIMObjectPath.h>
#include <Pegasus/Common/CIMClass.h>
#include <Pegasus/Common/CIMInstance.h>
#include <Pegasus/Common/CIMPropertyList.h>
#include <Pegasus/Common/CIMParamValue.h>
#include <Pegasus/Common/Sharable.h>
#include <Pegasus/Common/InternalException.h>
#include <Pegasus/Common/CIMResponseData.h>
#include <Pegasus/Provider/Linkage.h>

PEGASUS_NAMESPACE_BEGIN

class PEGASUS_PROVIDER_LINKAGE CIMOMHandleRep : public Sharable
{
public:
    /** */
    CIMOMHandleRep();

    /** */
    virtual ~CIMOMHandleRep();

    virtual CIMClass getClass(
        const OperationContext & context,
        const CIMNamespaceName& nameSpace,
        const CIMName& className,
        Boolean localOnly,
        Boolean includeQualifiers,
        Boolean includeClassOrigin,
        const CIMPropertyList& propertyList) = 0;

    virtual Array<CIMClass> enumerateClasses(
        const OperationContext & context,
        const CIMNamespaceName& nameSpace,
        const CIMName& className,
        Boolean deepInheritance,
        Boolean localOnly,
        Boolean includeQualifiers,
        Boolean includeClassOrigin) = 0;

    virtual Array<CIMName> enumerateClassNames(
        const OperationContext & context,
        const CIMNamespaceName& nameSpace,
        const CIMName& className,
        Boolean deepInheritance) = 0;

    virtual void createClass(
        const OperationContext & context,
        const CIMNamespaceName& nameSpace,
        const CIMClass& newClass) = 0;

    virtual void modifyClass(
        const OperationContext & context,
        const CIMNamespaceName& nameSpace,
        const CIMClass& modifiedClass) = 0;

    virtual void deleteClass(
        const OperationContext & context,
        const CIMNamespaceName& nameSpace,
        const CIMName& className) = 0;

    virtual CIMResponseData getInstance(
        const OperationContext & context,
        const CIMNamespaceName& nameSpace,
        const CIMObjectPath& instanceName,
        Boolean includeQualifiers,
        Boolean includeClassOrigin,
        const CIMPropertyList& propertyList) = 0;

    virtual CIMResponseData enumerateInstances(
        const OperationContext & context,
        const CIMNamespaceName& nameSpace,
        const CIMName& className,
        Boolean deepInheritance,
        Boolean includeQualifiers,
        Boolean includeClassOrigin,
        const CIMPropertyList& propertyList) = 0;

    virtual CIMResponseData enumerateInstanceNames(
        const OperationContext & context,
        const CIMNamespaceName& nameSpace,
        const CIMName& className) = 0;

    virtual CIMObjectPath createInstance(
        const OperationContext & context,
        const CIMNamespaceName& nameSpace,
        const CIMInstance& newInstance) = 0;

    virtual void modifyInstance(
        const OperationContext & context,
        const CIMNamespaceName& nameSpace,
        const CIMInstance& modifiedInstance,
        Boolean includeQualifiers,
        const CIMPropertyList& propertyList) = 0;

    virtual void deleteInstance(
        const OperationContext & context,
        const CIMNamespaceName& nameSpace,
        const CIMObjectPath& instanceName) = 0;

    virtual CIMResponseData execQuery(
        const OperationContext & context,
        const CIMNamespaceName& nameSpace,
        const String& queryLanguage,
        const String& query) = 0;

    virtual CIMResponseData associators(
        const OperationContext & context,
        const CIMNamespaceName& nameSpace,
        const CIMObjectPath& objectName,
        const CIMName& assocClass,
        const CIMName& resultClass,
        const String& role,
        const String& resultRole,
        Boolean includeQualifiers,
        Boolean includeClassOrigin,
        const CIMPropertyList& propertyList) = 0;

    virtual CIMResponseData associatorNames(
        const OperationContext & context,
        const CIMNamespaceName& nameSpace,
        const CIMObjectPath& objectName,
        const CIMName& assocClass,
        const CIMName& resultClass,
        const String& role,
        const String& resultRole) = 0;

    virtual CIMResponseData references(
        const OperationContext & context,
        const CIMNamespaceName& nameSpace,
        const CIMObjectPath& objectName,
        const CIMName& resultClass,
        const String& role,
        Boolean includeQualifiers,
        Boolean includeClassOrigin,
        const CIMPropertyList& propertyList) = 0;

    virtual CIMResponseData referenceNames(
        const OperationContext & context,
        const CIMNamespaceName& nameSpace,
        const CIMObjectPath& objectName,
        const CIMName& resultClass,
        const String& role) = 0;

    // property operations
    virtual CIMValue getProperty(
        const OperationContext & context,
        const CIMNamespaceName& nameSpace,
        const CIMObjectPath& instanceName,
        const CIMName& propertyName) = 0;

    virtual void setProperty(
        const OperationContext & context,
        const CIMNamespaceName& nameSpace,
        const CIMObjectPath& instanceName,
        const CIMName& propertyName,
        const CIMValue& newValue) = 0;

    virtual CIMValue invokeMethod(
        const OperationContext & context,
        const CIMNamespaceName& nameSpace,
        const CIMObjectPath& instanceName,
        const CIMName& methodName,
        const Array<CIMParamValue>& inParameters,
        Array<CIMParamValue>& outParameters) = 0;

    /**
        Provides a hint to the CIM Server that the provider calling this
        method prefers not to be unloaded.  This hint applies in
        situations where a provider unload is not necessary, such as
        when the CIM Server unloads idle providers for efficiency reasons.
        A provider may rescind this hint by using the allowProviderUnload
        method.  Note that disallowProviderUnload is cumulative, such that
        each call to disallowProviderUnload must be matched with a call to
        allowProviderUnload.
     */
    virtual void disallowProviderUnload();

    /**
        Provides a hint to the CIM Server that the provider calling this
        method no longer prefers not to be unloaded.  This hint applies in
        situations where a provider unload is not necessary, such as
        when the CIM Server unloads idle providers for efficiency reasons.
        This method is used to rescind a hint that was given using the
        disallowProviderUnload method.  Note that each allowProviderUnload
        call should be preceded by a disallowProviderUnload call.
     */
    virtual void allowProviderUnload();

#ifdef PEGASUS_USE_EXPERIMENTAL_INTERFACES
    /**
        Returns the context of the response to the last request.
        Currently, the context only contains the ContentLanguageListContainer
        from the response.
        Note: this method should be called directly after the call to the
        CIM request method (getClass, etc), and can only be called once
        per request.
     */
    virtual OperationContext getResponseContext() = 0;
#endif

    virtual Boolean unload_ok();

private:
    Uint32 _providerUnloadProtect;
    Mutex _providerUnloadProtectMutex;
};

PEGASUS_NAMESPACE_END

#endif
