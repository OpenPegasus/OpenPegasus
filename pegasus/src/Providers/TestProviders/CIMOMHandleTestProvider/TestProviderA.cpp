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

#include "TestProviderA.h"

PEGASUS_NAMESPACE_BEGIN

static CIMObjectPath _convertObjectPathAtoZ(const CIMObjectPath& objectPath)
{
    CIMObjectPath temp = objectPath;

    temp.setClassName("TST_InstanceZ");

    return temp;
}

static CIMObjectPath _convertObjectPathZtoA(const CIMObjectPath& objectPath)
{
    CIMObjectPath temp = objectPath;

    temp.setClassName("TST_InstanceA");

    return temp;
}

static CIMInstance _convertInstanceZtoA(const CIMInstance& instance)
{
    CIMInstance temp("TST_InstanceA");

    temp.addProperty(CIMProperty(
        "Name",
        instance.getProperty(instance.findProperty("Name")).getValue()));
    temp.addProperty(CIMProperty("s", instance.getPath().toString()));

    temp.setPath(_convertObjectPathZtoA(instance.getPath()));

    return temp;
}

TestProviderA::TestProviderA()
{
}

TestProviderA::~TestProviderA()
{
}

void TestProviderA::initialize(CIMOMHandle& cimom)
{
    _cimom = cimom;
}

void TestProviderA::terminate()
{
    delete this;
}

void TestProviderA::getInstance(
    const OperationContext& context,
    const CIMObjectPath& instanceReference,
    const Boolean includeQualifiers,
    const Boolean includeClassOrigin,
    const CIMPropertyList& propertyList,
    InstanceResponseHandler& handler)
{
    handler.processing();

    CIMInstance cimInstance =
        _cimom.getInstance(
            context,
            instanceReference.getNameSpace(),
            _convertObjectPathAtoZ(instanceReference),
            false,
            includeQualifiers,
            includeClassOrigin,
            propertyList);

    handler.deliver(_convertInstanceZtoA(cimInstance));

    handler.complete();
}

void TestProviderA::enumerateInstances(
    const OperationContext& context,
    const CIMObjectPath& classReference,
    const Boolean includeQualifiers,
    const Boolean includeClassOrigin,
    const CIMPropertyList& propertyList,
    InstanceResponseHandler& handler)
{
    handler.processing();

    Array<CIMInstance> cimInstances =
        _cimom.enumerateInstances(
            context,
            classReference.getNameSpace(),
            _convertObjectPathAtoZ(classReference).getClassName(),
            true,
            false,
            includeQualifiers,
            includeClassOrigin,
            propertyList);

    for (Uint32 i = 0, n = cimInstances.size(); i < n; i++)
    {
        handler.deliver(_convertInstanceZtoA(cimInstances[i]));
    }

    handler.complete();
}

void TestProviderA::enumerateInstanceNames(
    const OperationContext& context,
    const CIMObjectPath& classReference,
    ObjectPathResponseHandler& handler)
{
    handler.processing();

    Array<CIMObjectPath> cimInstanceNames =
        _cimom.enumerateInstanceNames(
            context,
            classReference.getNameSpace(),
            _convertObjectPathAtoZ(classReference).getClassName());

    for (Uint32 i = 0, n = cimInstanceNames.size(); i < n; i++)
    {
        handler.deliver(_convertObjectPathZtoA(cimInstanceNames[i]));
    }

    handler.complete();
}

void TestProviderA::modifyInstance(
    const OperationContext& context,
    const CIMObjectPath& instanceReference,
    const CIMInstance& instanceObject,
    const Boolean includeQualifiers,
    const CIMPropertyList& propertyList,
    ResponseHandler& handler)
{
    throw CIMException(CIM_ERR_NOT_SUPPORTED);
}

void TestProviderA::createInstance(
    const OperationContext& context,
    const CIMObjectPath& instanceReference,
    const CIMInstance& instanceObject,
    ObjectPathResponseHandler& handler)
{
    throw CIMException(CIM_ERR_NOT_SUPPORTED);
}

void TestProviderA::deleteInstance(
    const OperationContext& context,
    const CIMObjectPath& instanceReference,
    ResponseHandler& handler)
{
    throw CIMException(CIM_ERR_NOT_SUPPORTED);
}

PEGASUS_NAMESPACE_END

