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

#ifndef Pegasus_DynamicIndicationProvider_h
#define Pegasus_DynamicIndicationProvider_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/AtomicInt.h>
#include <Pegasus/Provider/CIMMethodProvider.h>
#include <Pegasus/Provider/CIMInstanceProvider.h>
#include <Pegasus/Provider/CIMIndicationProvider.h>

PEGASUS_NAMESPACE_BEGIN

class DynamicIndicationProvider :
    public CIMMethodProvider,
    public CIMInstanceProvider,
    public CIMIndicationProvider
{

public:
    DynamicIndicationProvider();
    virtual ~DynamicIndicationProvider();

    // CIMProvider interface
    virtual void initialize(CIMOMHandle& cimom);
    virtual void terminate();

    // CIMMethodProvider Interface
    void invokeMethod(
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

    virtual void createInstance(
        const OperationContext& context,
        const CIMObjectPath& instanceReference,
        const CIMInstance& instanceObject,
        ObjectPathResponseHandler& handler);

    virtual void modifyInstance(
        const OperationContext& context,
        const CIMObjectPath& instanceReference,
        const CIMInstance& instanceObject,
        const Boolean includeQualifiers,
        const CIMPropertyList& propertyList,
        ResponseHandler& handler);

    virtual void deleteInstance(
        const OperationContext& context,
        const CIMObjectPath& instanceReference,
        ResponseHandler& handler);

public:
    virtual void sendIndication(
        int severity,
        String classname,
        String description);

protected:
    IndicationResponseHandler* _pHandler;

private:
    static AtomicInt _refCnt;

};

PEGASUS_NAMESPACE_END

#endif
