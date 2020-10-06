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

#ifndef Pegasus_ExceptionProvider_h
#define Pegasus_ExceptionProvider_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Provider/CIMProvider.h>
#include <Pegasus/Provider/CIMInstanceQueryProvider.h>
#include <Pegasus/Provider/CIMAssociationProvider.h>
#include <Pegasus/Provider/CIMMethodProvider.h>
#include <Pegasus/Provider/CIMIndicationProvider.h>
#include <Pegasus/Provider/CIMIndicationConsumerProvider.h>

PEGASUS_NAMESPACE_BEGIN

/**
    The ExceptionProvider class is a test provider that throws different
    exceptions based on the class name specified in the operation.
 */
class ExceptionProvider :
    public CIMInstanceQueryProvider,
    public CIMAssociationProvider,
    public CIMMethodProvider,
    public CIMIndicationProvider,
    public CIMIndicationConsumerProvider
{
public:
    ExceptionProvider();

    virtual ~ExceptionProvider();

    // CIMProvider interface
    virtual void initialize(CIMOMHandle& cimom);
    virtual void terminate();

    // CIMInstanceProvider interface
    virtual void getInstance(
        const OperationContext& context,
        const CIMObjectPath& instanceReference,
        const Boolean includeQualifiers,
        const Boolean includeClassOrigin,
        const CIMPropertyList& propertyList,
        InstanceResponseHandler& handler);

    virtual void enumerateInstances(
        const OperationContext& context,
        const CIMObjectPath& classReference,
        const Boolean includeQualifiers,
        const Boolean includeClassOrigin,
        const CIMPropertyList& propertyList,
        InstanceResponseHandler& handler);

    virtual void enumerateInstanceNames(
        const OperationContext& context,
        const CIMObjectPath& classReference,
        ObjectPathResponseHandler& handler);

    virtual void modifyInstance(
        const OperationContext& context,
        const CIMObjectPath& instanceReference,
        const CIMInstance& instanceObject,
        const Boolean includeQualifiers,
        const CIMPropertyList& propertyList,
        ResponseHandler& handler);

    virtual void createInstance(
        const OperationContext& context,
        const CIMObjectPath& instanceReference,
        const CIMInstance& instanceObject,
        ObjectPathResponseHandler& handler);

    virtual void deleteInstance(
        const OperationContext& context,
        const CIMObjectPath& instanceReference,
        ResponseHandler& handler);

    virtual void execQuery(
        const OperationContext& context,
        const CIMObjectPath& nameSpaceAndClass,
        const QueryExpression& query,
        InstanceResponseHandler& handler);

    // CIMAssociationProvider interface
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

    // CIMMethodProvider
    virtual void invokeMethod(
        const OperationContext& context,
        const CIMObjectPath& objectReference,
        const CIMName& methodName,
        const Array<CIMParamValue>& inParameters,
        MethodResultResponseHandler& handler);

    // CIMIndicationProvider interface
    virtual void enableIndications(IndicationResponseHandler& handler);

    virtual void disableIndications();

    virtual void createSubscription(
        const OperationContext& context,
        const CIMObjectPath& subscriptionName,
        const Array<CIMObjectPath>& classNames,
        const CIMPropertyList& propertyList,
        const Uint16 repeatNotificationPolicy);

    virtual void modifySubscription(
        const OperationContext& context,
        const CIMObjectPath& subscriptionName,
        const Array<CIMObjectPath>& classNames,
        const CIMPropertyList& propertyList,
        const Uint16 repeatNotificationPolicy);

    virtual void deleteSubscription(
        const OperationContext& context,
        const CIMObjectPath& subscriptionName,
        const Array<CIMObjectPath>& classNames);

    // CIMIndicationConsumerProvider interface
    virtual void consumeIndication(
        const OperationContext& context,
        const String& destinationPath,
        const CIMInstance& indicationInstance);
};

PEGASUS_NAMESPACE_END

#endif
