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
#include <Pegasus/Provider/CIMInstanceProvider.h>

#include <Pegasus/Common/CIMDateTime.h>

#include "TestInstanceProvider.h"

PEGASUS_NAMESPACE_BEGIN

TestInstanceProvider::TestInstanceProvider()
{
}

TestInstanceProvider::~TestInstanceProvider()
{
}

void TestInstanceProvider::initialize(CIMOMHandle& cimom)
{
    // save cimom handle
    //_cimom = cimom;

    // create default instances
    {
        CIMInstance instance("tst_instance1");

        instance.addProperty(CIMProperty("name", String("001")));

        instance.addProperty(CIMProperty("s", String("specified")));
        instance.addProperty(CIMProperty("n", Uint64(001)));
        instance.addProperty(CIMProperty("f", Real64(1.001)));
        instance.addProperty(
            CIMProperty("d", CIMDateTime::getCurrentDateTime()));

        instance.setPath(CIMObjectPath("TST_Instance1.Name=\"001\""));

        _instances.append(instance);
    }

    {
        CIMInstance instance("tst_instance1");

        instance.addProperty(CIMProperty("name", String("002")));

        instance.addProperty(CIMProperty("s", String("specified")));
        instance.addProperty(CIMProperty("n", Uint64(002)));
        instance.addProperty(CIMProperty("f", Real64(1.002)));
        instance.addProperty(
            CIMProperty("d", CIMDateTime::getCurrentDateTime()));

        instance.addProperty(CIMProperty("e", String("extra property")));

        instance.setPath(CIMObjectPath("TST_Instance1.Name=\"002\""));

        _instances.append(instance);
    }

    {
        CIMInstance instance("tst_instance1");

        instance.addProperty(CIMProperty("name", String("003")));

        //instance.addProperty(CIMProperty("s", String("specified")));
        instance.addProperty(CIMProperty("n", Uint64(003)));
        instance.addProperty(CIMProperty("f", Real64(1.003)));
        instance.addProperty(
            CIMProperty("d", CIMDateTime::getCurrentDateTime()));

        instance.setPath(CIMObjectPath("TST_Instance1.Name=\"003\""));

        _instances.append(instance);
    }

    // create instance without object path (key property not in instance)
    {
        CIMInstance instance("tst_instance1");

        //instance.addProperty(CIMProperty("name", String("004")));

        instance.addProperty(CIMProperty("s", String("specified")));
        instance.addProperty(CIMProperty("n", Uint64(004)));
        instance.addProperty(CIMProperty("f", Real64(1.004)));
        instance.addProperty(
            CIMProperty("d", CIMDateTime::getCurrentDateTime()));

        //instance.setPath(CIMObjectPath("TST_Instance1.Name=\"004\""));

        _instances.append(instance);
    }

    // create instance with incorrect class name
    {
        CIMInstance instance("tst_instanceBAD");

        instance.addProperty(CIMProperty("name", String("005")));

        instance.addProperty(CIMProperty("s", String("specified")));
        instance.addProperty(CIMProperty("n", Uint64(005)));
        instance.addProperty(CIMProperty("f", Real64(1.005)));
        instance.addProperty(
            CIMProperty("d", CIMDateTime::getCurrentDateTime()));

        instance.setPath(CIMObjectPath("TST_InstanceBAD.Name=\"005\""));

        _instances.append(instance);
    }

    // wrong property type
    {
        CIMInstance instance("tst_instance1");

        instance.addProperty(CIMProperty("name", Uint16(006)));

        instance.addProperty(CIMProperty("s", String("specified")));
        instance.addProperty(CIMProperty("n", Uint64(006)));
        instance.addProperty(CIMProperty("f", Real64(1.006)));
        instance.addProperty(
            CIMProperty("d", CIMDateTime::getCurrentDateTime()));

        instance.setPath(CIMObjectPath("TST_Instance1.Name=\"006\""));

        _instances.append(instance);
    }
}

void TestInstanceProvider::terminate()
{
    delete this;
}

void TestInstanceProvider::getInstance(
    const OperationContext& context,
    const CIMObjectPath& instanceReference,
    const Boolean includeQualifiers,
    const Boolean includeClassOrigin,
    const CIMPropertyList& propertyList,
    InstanceResponseHandler& handler)
{
    handler.processing();

    // create relative object path for comparison
    CIMObjectPath cimObjectPath(instanceReference);

    cimObjectPath.setHost(String());
    cimObjectPath.setNameSpace(CIMNamespaceName());

    for (Uint32 i = 0, n = _instances.size(); i < n; i++)
    {
        if (cimObjectPath == _instances[i].getPath())
        {
            try
            {
                handler.deliver(_instances[i]);
            }
            catch (CIMException&)
            {
                // suppress error
            }

            break;
        }
    }

    handler.complete();
}

void TestInstanceProvider::enumerateInstances(
    const OperationContext& context,
    const CIMObjectPath& ref,
    const Boolean includeQualifiers,
    const Boolean includeClassOrigin,
    const CIMPropertyList& propertyList,
    InstanceResponseHandler& handler)
{
    handler.processing();

    for (Uint32 i = 0, n = _instances.size(); i < n; i++)
    {
        try
        {
            handler.deliver(_instances[i]);
        }
        catch (CIMException&)
        {
            // suppress error
        }
    }

    handler.complete();
}

void TestInstanceProvider::enumerateInstanceNames(
    const OperationContext& context,
    const CIMObjectPath& classReference,
    ObjectPathResponseHandler& handler)
{
    handler.processing();

    for (Uint32 i = 0, n = _instances.size(); i < n; i++)
    {
        try
        {
            handler.deliver(_instances[i].getPath());
        }
        catch (CIMException&)
        {
            // suppress error
        }
    }

    handler.complete();
}

void TestInstanceProvider::modifyInstance(
    const OperationContext& context,
    const CIMObjectPath& instanceReference,
    const CIMInstance& instanceObject,
    const Boolean includeQualifiers,
    const CIMPropertyList& propertyList,
    ResponseHandler& handler)
{
    throw CIMException(CIM_ERR_NOT_SUPPORTED);
}

void TestInstanceProvider::createInstance(
    const OperationContext& context,
    const CIMObjectPath& instanceReference,
    const CIMInstance& instanceObject,
    ObjectPathResponseHandler& handler)
{
    throw CIMException(CIM_ERR_NOT_SUPPORTED);
}

void TestInstanceProvider::deleteInstance(
    const OperationContext& context,
    const CIMObjectPath& instanceReference,
    ResponseHandler& handler)
{
    throw CIMException(CIM_ERR_NOT_SUPPORTED);
}

PEGASUS_NAMESPACE_END
