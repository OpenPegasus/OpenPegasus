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
//%////////////////////////////////////////////////////////////////////////////

#ifndef PG_REMOTE_SERVICE_ACCESS_POINT_PROVIDER_H
#define PG_REMOTE_SERVICE_ACCESS_POINT_PROVIDER_H

#include <Pegasus/Provider/CIMInstanceProvider.h>
#include <Providers/ManagedSystem/IP/IPPlatform.h>
#include <Pegasus/Common/Tracer.h>

PEGASUS_USING_STD;
PEGASUS_USING_PEGASUS;

class RSApProvider : public CIMInstanceProvider
{
public:

    RSApProvider();

    ~RSApProvider();

    void createInstance(
        const OperationContext &context,
        const CIMObjectPath &instanceName,
        const CIMInstance &instanceObject,
        ObjectPathResponseHandler &handler);

    void deleteInstance(
        const OperationContext &context,
        const CIMObjectPath &instanceReference,
        ResponseHandler &handler);

    void enumerateInstances(
        const OperationContext & context,
        const CIMObjectPath & classReference,
        const Boolean includeQualifiers,
        const Boolean includeClassOrigin,
        const CIMPropertyList & propertyList,
        InstanceResponseHandler & handler);

    void enumerateInstanceNames(
        const OperationContext &context,
        const CIMObjectPath &ref,
        ObjectPathResponseHandler &handler);

    void getInstance(
        const OperationContext &context,
        const CIMObjectPath &instanceName,
        const Boolean includeQualifiers,
        const Boolean includeClassOrigin,
        const CIMPropertyList &propertyList,
        InstanceResponseHandler &handler);

    void modifyInstance(
        const OperationContext &context,
        const CIMObjectPath &instanceName,
        const CIMInstance &instanceObject,
        const Boolean includeQualifiers,
        const CIMPropertyList &propertyList,
        ResponseHandler &handler);

    void initialize(CIMOMHandle&);

    void terminate();

private:

    // Private member to store handle passed by initialize().
    CIMOMHandle _cimomHandle;

    Array<CIMKeyBinding> _constructKeyBindings(const RSAp&);

    // Used to add properties to an instance.
    CIMInstance _constructInstance(
        const CIMName&,
        const CIMNamespaceName&,
        const RSAp&);

    // Checks the class passed by the cimom, and throws
    // an exception if it's not supported by this provider.
    void _checkClass(CIMName&);

};


#endif  /* #ifndef PG_REMOTE_SERVICE_ACCESS_POINT_PROVIDER_H */
