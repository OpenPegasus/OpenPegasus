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


#ifndef Pegasus_CIMOMHandle_h
#define Pegasus_CIMOMHandle_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/OperationContext.h>
#include <Pegasus/Common/CIMObject.h>
#include <Pegasus/Common/CIMObjectPath.h>
#include <Pegasus/Common/CIMClass.h>
#include <Pegasus/Common/CIMInstance.h>
#include <Pegasus/Common/CIMPropertyList.h>
#include <Pegasus/Common/ResponseHandler.h>
#include <Pegasus/Provider/Linkage.h>

PEGASUS_NAMESPACE_BEGIN

class CIMOMHandleRep;

/**
    Provides an interface through which a provider may communicate with the
    CIM Server.  Most of the methods of this class mirror the CIMClient
    interface and allow a provider to perform CIM operations.  The semantics
    of these methods are the same as the CIMClient interface, and their
    documentation is not repeated here.  Please refer to the CIMClient
    documentation for further details.  The interface differs slightly by
    the use of an OperationContext parameter.  This parameter is used to
    communicate context information such as language data, for which the
    CIMClient uses state settings.

    The CIMOMHandle may serialize requests from different threads, and
    therefore may not be a good choice for multi-threaded access.  Use of
    the CIMClient interface by a provider is permitted and may be considered
    when multiple threads need to perform client operations concurrently.
*/
class PEGASUS_PROVIDER_LINKAGE CIMOMHandle
{
public:

    /**
        Constructs a default CIMOMHandle object.
    */
    CIMOMHandle();

    CIMOMHandle(const CIMOMHandle&);

    /**
        Destructs a CIMOMHandle object.
    */
    ~CIMOMHandle();

    /**
        Assigns the CIMOMHandle from a specified CIMOMHandle object.
        @param handle The CIMOMHandle object to copy.
        @return A reference to this CIMOMHandle object with the new assignment.
    */
    CIMOMHandle& operator=(const CIMOMHandle& handle);

    CIMClass getClass(
        const OperationContext& context,
        const CIMNamespaceName& nameSpace,
        const CIMName& className,
        Boolean localOnly,
        Boolean includeQualifiers,
        Boolean includeClassOrigin,
        const CIMPropertyList& propertyList);

    Array<CIMClass> enumerateClasses(
        const OperationContext& context,
        const CIMNamespaceName& nameSpace,
        const CIMName& className,
        Boolean deepInheritance,
        Boolean localOnly,
        Boolean includeQualifiers,
        Boolean includeClassOrigin);

    Array<CIMName> enumerateClassNames(
        const OperationContext& context,
        const CIMNamespaceName& nameSpace,
        const CIMName& className,
        Boolean deepInheritance);

    void createClass(
        const OperationContext& context,
        const CIMNamespaceName& nameSpace,
        const CIMClass& newClass);

    void modifyClass(
        const OperationContext& context,
        const CIMNamespaceName& nameSpace,
        const CIMClass& modifiedClass);

    void deleteClass(
        const OperationContext& context,
        const CIMNamespaceName& nameSpace,
        const CIMName& className);

    CIMInstance getInstance(
        const OperationContext& context,
        const CIMNamespaceName& nameSpace,
        const CIMObjectPath& instanceName,
        Boolean localOnly,
        Boolean includeQualifiers,
        Boolean includeClassOrigin,
        const CIMPropertyList& propertyList);

    Array<CIMInstance> enumerateInstances(
        const OperationContext& context,
        const CIMNamespaceName& nameSpace,
        const CIMName& className,
        Boolean deepInheritance,
        Boolean localOnly,
        Boolean includeQualifiers,
        Boolean includeClassOrigin,
        const CIMPropertyList& propertyList);

    Array<CIMObjectPath> enumerateInstanceNames(
        const OperationContext& context,
        const CIMNamespaceName& nameSpace,
        const CIMName& className);

    CIMObjectPath createInstance(
        const OperationContext& context,
        const CIMNamespaceName& nameSpace,
        const CIMInstance& newInstance);

    void modifyInstance(
        const OperationContext& context,
        const CIMNamespaceName& nameSpace,
        const CIMInstance& modifiedInstance,
        Boolean includeQualifiers,
        const CIMPropertyList& propertyList);

    void deleteInstance(
        const OperationContext& context,
        const CIMNamespaceName& nameSpace,
        const CIMObjectPath& instanceName);

    Array<CIMObject> execQuery(
        const OperationContext& context,
        const CIMNamespaceName& nameSpace,
        const String& queryLanguage,
        const String& query);

    Array<CIMObject> associators(
        const OperationContext& context,
        const CIMNamespaceName& nameSpace,
        const CIMObjectPath& objectName,
        const CIMName& assocClass,
        const CIMName& resultClass,
        const String& role,
        const String& resultRole,
        Boolean includeQualifiers,
        Boolean includeClassOrigin,
        const CIMPropertyList& propertyList);

    Array<CIMObjectPath> associatorNames(
        const OperationContext& context,
        const CIMNamespaceName& nameSpace,
        const CIMObjectPath& objectName,
        const CIMName& assocClass,
        const CIMName& resultClass,
        const String& role,
        const String& resultRole);

    Array<CIMObject> references(
        const OperationContext& context,
        const CIMNamespaceName& nameSpace,
        const CIMObjectPath& objectName,
        const CIMName& resultClass,
        const String& role,
        Boolean includeQualifiers,
        Boolean includeClassOrigin,
        const CIMPropertyList& propertyList);

    Array<CIMObjectPath> referenceNames(
        const OperationContext& context,
        const CIMNamespaceName& nameSpace,
        const CIMObjectPath& objectName,
        const CIMName& resultClass,
        const String& role);

    // property operations
    CIMValue getProperty(
        const OperationContext& context,
        const CIMNamespaceName& nameSpace,
        const CIMObjectPath& instanceName,
        const CIMName& propertyName);

    void setProperty(
        const OperationContext& context,
        const CIMNamespaceName& nameSpace,
        const CIMObjectPath& instanceName,
        const CIMName& propertyName,
        const CIMValue& newValue);

    CIMValue invokeMethod(
        const OperationContext& context,
        const CIMNamespaceName& nameSpace,
        const CIMObjectPath& instanceName,
        const CIMName& methodName,
        const Array<CIMParamValue>& inParameters,
        Array<CIMParamValue>& outParameters);

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
    void disallowProviderUnload();

    /**
        Provides a hint to the CIM Server that the provider calling this
        method no longer prefers not to be unloaded.  This hint applies in
        situations where a provider unload is not necessary, such as
        when the CIM Server unloads idle providers for efficiency reasons.
        This method is used to rescind a hint that was given using the
        disallowProviderUnload method.  Note that each allowProviderUnload
        call should be preceded by a disallowProviderUnload call.
    */
    void allowProviderUnload();

#ifdef PEGASUS_USE_EXPERIMENTAL_INTERFACES
    /**
        Returns the context of the response to the last request.
        Currently, the context only contains the ContentLanguageListContainer
        from the response.
        Note: this method should be called directly after the call to the
        CIM request method (getClass, etc), and can only be called once
        per request.

        @return An OperationContext object containing the context of the most
            recent response.
    */
    OperationContext getResponseContext();
#endif

private:
    CIMOMHandleRep* _rep;

    friend class ProviderStatus;
    friend class CMPIProvider;
    friend class CMPI_BrokerHdl;

    Boolean unload_ok();
};

PEGASUS_NAMESPACE_END

#endif
