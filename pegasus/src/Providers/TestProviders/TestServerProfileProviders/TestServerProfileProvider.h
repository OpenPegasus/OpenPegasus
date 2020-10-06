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

#ifndef Pegasus_TestStorageSystemProvider_h
#define Pegasus_TestStorageSystemProvider_h

#include <Pegasus/Provider/CIMInstanceProvider.h>
#include <Pegasus/Provider/CIMAssociationProvider.h>
#include <Pegasus/Common/Constants.h>

PEGASUS_USING_PEGASUS;

class TestServerProfileProvider :
    public CIMInstanceProvider,
    public CIMAssociationProvider
{
public:
    TestServerProfileProvider(const String&);
    virtual ~TestServerProfileProvider();

    virtual void initialize(CIMOMHandle& cimom);
    virtual void terminate();

    // Instance Provider interface
    virtual void getInstance(
        const OperationContext& context,
        const CIMObjectPath& ref,
        const Boolean includeQualifiers,
        const Boolean includeClassOrigin,
        const CIMPropertyList& propertyList,
        InstanceResponseHandler& handler);

    virtual void enumerateInstances(
        const OperationContext& context,
        const CIMObjectPath& ref,
        const Boolean includeQualifiers,
        const Boolean includeClassOrigin,
        const CIMPropertyList& propertyList,
        InstanceResponseHandler& handler);

    virtual void enumerateInstanceNames(
        const OperationContext& context,
        const CIMObjectPath& ref,
        ObjectPathResponseHandler& handler);

    virtual void modifyInstance(
        const OperationContext& context,
        const CIMObjectPath& ref,
        const CIMInstance& obj,
        const Boolean includeQualifiers,
        const CIMPropertyList& propertyList,
        ResponseHandler& handler);

    virtual void createInstance(
        const OperationContext& context,
        const CIMObjectPath& ref,
        const CIMInstance& obj,
        ObjectPathResponseHandler& handler);

    virtual void deleteInstance(
        const OperationContext& context,
        const CIMObjectPath& ref,
        ResponseHandler& handler);

    //
    // Association Provider Interface
    //
    virtual void associators(
        const OperationContext& context,
        const CIMObjectPath& objectName,
        const CIMName& associationClass,
        const CIMName& resultClass,
        const String& role,
        const String& resultRole,
        const Boolean includeQualifiers,
        const Boolean includeClassOrigin,
        const CIMPropertyList& propertyList,
        ObjectResponseHandler& handler);

    virtual void associatorNames(
        const OperationContext& context,
        const CIMObjectPath& objectName,
        const CIMName& associationClass,
        const CIMName& resultClass,
        const String& role,
        const String& resultRole,
        ObjectPathResponseHandler& handler);

    virtual void references(
        const OperationContext& context,
        const CIMObjectPath& objectName,
        const CIMName& resultClass,
        const String& role,
        const Boolean includeQualifiers,
        const Boolean includeClassOrigin,
        const CIMPropertyList& propertyList,
        ObjectResponseHandler& handler);

    virtual void referenceNames(
        const OperationContext& context,
        const CIMObjectPath& objectName,
        const CIMName& resultClass,
        const String& role,
        ObjectPathResponseHandler& handler);

private:
    Array<CIMInstance> localEnumerateInstances(
        const CIMName& className,
        Boolean includeQualifiers,
        Boolean includeClassOrigin,
        const CIMPropertyList& propList);

    CIMOMHandle cimomHandle;
    CIMName testClassName;
    CIMClass testClass;
    Array<String> names;
};

#endif
