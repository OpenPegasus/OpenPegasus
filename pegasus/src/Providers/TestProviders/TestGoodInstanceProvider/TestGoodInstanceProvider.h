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
//%///////////////////////////////////////////////////////////////////////

#ifndef Pegasus_TestGoodInstanceProvider_h
#define Pegasus_TestGoodInstanceProvider_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/AtomicInt.h>
#include <Pegasus/Provider/CIMInstanceProvider.h>

PEGASUS_NAMESPACE_BEGIN

class TestGoodInstanceProvider :
    public CIMInstanceProvider
{
public:
    TestGoodInstanceProvider();
    virtual ~TestGoodInstanceProvider();

    // CIMProvider interface
    virtual void initialize(CIMOMHandle& cimom);
    virtual void terminate();

    // CIMInstanceProvider interface
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

protected:
    Array<CIMInstance> _instances;
};

PEGASUS_NAMESPACE_END

#endif
