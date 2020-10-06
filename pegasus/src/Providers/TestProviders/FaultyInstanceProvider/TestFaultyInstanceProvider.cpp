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

#include <cstring>
#include <Pegasus/Common/Config.h>
#include <Pegasus/Provider/CIMInstanceProvider.h>

#include <Pegasus/Common/CIMDateTime.h>
#include <Pegasus/Common/Threads.h>

#include "TestFaultyInstanceProvider.h"

PEGASUS_USING_STD;
PEGASUS_NAMESPACE_BEGIN

#define CIMSERVER_CLEANUP_TIME 600
#define CONN_TIME_OUT 70
#define DEFAULT_CLIENT_TIMEOUT 20 // default timeout

TestFaultyInstanceProvider::TestFaultyInstanceProvider()
{
}

TestFaultyInstanceProvider::~TestFaultyInstanceProvider()
{
}

void TestFaultyInstanceProvider::initialize(CIMOMHandle& cimom)
{
    // save cimom handle
    //_cimom = cimom;

    char namebuf[20];
    char pathbuf[45];
    memset(namebuf, 0x00, sizeof(namebuf));
    memset(pathbuf, 0x00, sizeof(pathbuf));

    // create default instances
    for (Uint32 i = 1; i <= 2; i++)
    {
        sprintf(namebuf, "%u", i);
        sprintf(pathbuf, "TST_FaultyInstanceInstance.Name=\"%u\"", i);
        {
            CIMInstance instance("TST_FaultyInstanceInstance");

            instance.addProperty(CIMProperty("name", String(namebuf)));
            instance.addProperty(CIMProperty("s", String("specified")));
            instance.addProperty(CIMProperty("n", Uint64(i)));
            instance.addProperty(CIMProperty("f", Real64(Real64(i)+0.001)));
            instance.addProperty(
                CIMProperty("d", CIMDateTime::getCurrentDateTime()));

            instance.setPath(CIMObjectPath(pathbuf));

            _instances.append(instance);
        }
    }
}

void TestFaultyInstanceProvider::terminate()
{
    delete this;
}

void TestFaultyInstanceProvider::getInstance(
    const OperationContext& context,
    const CIMObjectPath& instanceReference,
    const Boolean includeQualifiers,
    const Boolean includeClassOrigin,
    const CIMPropertyList& propertyList,
    InstanceResponseHandler& handler)
{
    throw CIMException(CIM_ERR_NOT_SUPPORTED);
}

void TestFaultyInstanceProvider::enumerateInstances(
    const OperationContext& context,
    const CIMObjectPath& ref,
    const Boolean includeQualifiers,
    const Boolean includeClassOrigin,
    const CIMPropertyList& propertyList,
    InstanceResponseHandler& handler)
{
    throw CIMException(CIM_ERR_NOT_SUPPORTED);
}

void TestFaultyInstanceProvider::enumerateInstanceNames(
    const OperationContext& context,
    const CIMObjectPath& classReference,
    ObjectPathResponseHandler& handler)
{
    _requestCount.inc(); 
 
    Uint32 sleepDuration=0;  
    if(_requestCount.get() % 2)
    {
        //few requets will respond after cimserver  cleanup time.
        sleepDuration = CIMSERVER_CLEANUP_TIME * 1000 ; 
    }
    else if(_requestCount.get() % 16)
    {
        // respond correctly.
    }
    else
    {
        //few requests will respond after client connections have timed out
        //but before cimserver cleanup time.
        sleepDuration = CONN_TIME_OUT *1000;
    }

    handler.processing();
  
    Threads::sleep(sleepDuration);

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

void TestFaultyInstanceProvider::modifyInstance(
    const OperationContext& context,
    const CIMObjectPath& instanceReference,
    const CIMInstance& instanceObject,
    const Boolean includeQualifiers,
    const CIMPropertyList& propertyList,
    ResponseHandler& handler)
{
    throw CIMException(CIM_ERR_NOT_SUPPORTED);
}

void TestFaultyInstanceProvider::createInstance(
    const OperationContext& context,
    const CIMObjectPath& instanceReference,
    const CIMInstance& instanceObject,
    ObjectPathResponseHandler& handler)
{
    throw CIMException(CIM_ERR_NOT_SUPPORTED);
}

void TestFaultyInstanceProvider::deleteInstance(
    const OperationContext& context,
    const CIMObjectPath& instanceReference,
    ResponseHandler& handler)
{
    throw CIMException(CIM_ERR_NOT_SUPPORTED);
}

PEGASUS_NAMESPACE_END
