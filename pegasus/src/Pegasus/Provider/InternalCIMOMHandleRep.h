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

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/CIMMessage.h>
#include <Pegasus/Common/OperationContext.h>
#include <Pegasus/Common/Semaphore.h>
#include <Pegasus/Common/MessageQueueService.h>

#include "CIMOMHandleRep.h"

PEGASUS_NAMESPACE_BEGIN

class InternalCIMOMHandleMessageQueue : public MessageQueue
{
public:
    InternalCIMOMHandleMessageQueue();
    virtual ~InternalCIMOMHandleMessageQueue();

    virtual void handleEnqueue();

    CIMResponseMessage * sendRequest(CIMRequestMessage * request);

private:
    Uint32 _output_qid;
    Uint32 _return_qid;

    Mutex _mutex;
    Semaphore _responseReady;
    Message* _response;

};

/**
    InternalCIMOMHandleRep is the default, in-process CIMOMHandle
    implementation.  The code was extracted from CIMOMHandle.cpp to allow
    for polymorphic CIMOMHandleRep implementations.
 */
class InternalCIMOMHandleRep : public CIMOMHandleRep
{
public:
    InternalCIMOMHandleRep();
    virtual ~InternalCIMOMHandleRep();

    virtual CIMClass getClass(
        const OperationContext & context,
        const CIMNamespaceName& nameSpace,
        const CIMName& className,
        Boolean localOnly,
        Boolean includeQualifiers,
        Boolean includeClassOrigin,
        const CIMPropertyList& propertyList);

    virtual Array<CIMClass> enumerateClasses(
        const OperationContext & context,
        const CIMNamespaceName& nameSpace,
        const CIMName& className,
        Boolean deepInheritance,
        Boolean localOnly,
        Boolean includeQualifiers,
        Boolean includeClassOrigin);

    virtual Array<CIMName> enumerateClassNames(
        const OperationContext & context,
        const CIMNamespaceName& nameSpace,
        const CIMName& className,
        Boolean deepInheritance);

    virtual void createClass(
        const OperationContext & context,
        const CIMNamespaceName& nameSpace,
        const CIMClass& newClass);

    virtual void modifyClass(
        const OperationContext & context,
        const CIMNamespaceName& nameSpace,
        const CIMClass& modifiedClass);

    virtual void deleteClass(
        const OperationContext & context,
        const CIMNamespaceName& nameSpace,
        const CIMName& className);

    virtual CIMResponseData getInstance(
        const OperationContext & context,
        const CIMNamespaceName& nameSpace,
        const CIMObjectPath& instanceName,
        Boolean includeQualifiers,
        Boolean includeClassOrigin,
        const CIMPropertyList& propertyList);

    virtual CIMResponseData enumerateInstances(
        const OperationContext & context,
        const CIMNamespaceName& nameSpace,
        const CIMName& className,
        Boolean deepInheritance,
        Boolean includeQualifiers,
        Boolean includeClassOrigin,
        const CIMPropertyList& propertyList);

    virtual CIMResponseData enumerateInstanceNames(
        const OperationContext & context,
        const CIMNamespaceName& nameSpace,
        const CIMName& className);

    virtual CIMObjectPath createInstance(
        const OperationContext & context,
        const CIMNamespaceName& nameSpace,
        const CIMInstance& newInstance);

    virtual void modifyInstance(
        const OperationContext & context,
        const CIMNamespaceName& nameSpace,
        const CIMInstance& modifiedInstance,
        Boolean includeQualifiers,
        const CIMPropertyList& propertyList);

    virtual void deleteInstance(
        const OperationContext & context,
        const CIMNamespaceName& nameSpace,
        const CIMObjectPath& instanceName);

    virtual CIMResponseData execQuery(
        const OperationContext & context,
        const CIMNamespaceName& nameSpace,
        const String& queryLanguage,
        const String& query);

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
        const CIMPropertyList& propertyList);

    virtual CIMResponseData associatorNames(
        const OperationContext & context,
        const CIMNamespaceName& nameSpace,
        const CIMObjectPath& objectName,
        const CIMName& assocClass,
        const CIMName& resultClass,
        const String& role,
        const String& resultRole);

    virtual CIMResponseData references(
        const OperationContext & context,
        const CIMNamespaceName& nameSpace,
        const CIMObjectPath& objectName,
        const CIMName& resultClass,
        const String& role,
        Boolean includeQualifiers,
        Boolean includeClassOrigin,
        const CIMPropertyList& propertyList);

    virtual CIMResponseData referenceNames(
        const OperationContext & context,
        const CIMNamespaceName& nameSpace,
        const CIMObjectPath& objectName,
        const CIMName& resultClass,
        const String& role);

    // property operations
    virtual CIMValue getProperty(
        const OperationContext & context,
        const CIMNamespaceName& nameSpace,
        const CIMObjectPath& instanceName,
        const CIMName& propertyName);

    virtual void setProperty(
        const OperationContext & context,
        const CIMNamespaceName& nameSpace,
        const CIMObjectPath& instanceName,
        const CIMName& propertyName,
        const CIMValue& newValue);

    virtual CIMValue invokeMethod(
        const OperationContext & context,
        const CIMNamespaceName& nameSpace,
        const CIMObjectPath& instanceName,
        const CIMName& methodName,
        const Array<CIMParamValue>& inParameters,
        Array<CIMParamValue>& outParameters);

    virtual void disallowProviderUnload();  // Overload for OS/400
    virtual void allowProviderUnload();  // Overload for OS/400
    virtual OperationContext getResponseContext();

private:
    InternalCIMOMHandleRep(const InternalCIMOMHandleRep& rep);
    InternalCIMOMHandleRep& operator=(const InternalCIMOMHandleRep& rep);

    CIMResponseMessage* do_request(CIMRequestMessage* request);

private:
    InternalCIMOMHandleMessageQueue _queue;

};

PEGASUS_NAMESPACE_END
