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
#include <Pegasus/ProviderManager2/Default/ProviderMessageHandler.h>
#include <Pegasus/Common/OperationContextInternal.h>
#include <Pegasus/Common/PegasusAssert.h>
#include "ExceptionProvider.h"

#include <iostream>

PEGASUS_USING_PEGASUS;
PEGASUS_USING_STD;

static Boolean verbose;

class NotARealProvider : public CIMProvider
{
public:
    void initialize(CIMOMHandle& cimom) { }
    void terminate() { }
};

class GetPropertyErrorProvider : public CIMInstanceProvider
{
public:
    void initialize(CIMOMHandle& cimom) { }
    void terminate() { }

    void getInstance(
        const OperationContext& context,
        const CIMObjectPath& instanceReference,
        const Boolean includeQualifiers,
        const Boolean includeClassOrigin,
        const CIMPropertyList& propertyList,
        InstanceResponseHandler& handler)
    {
        handler.processing();

        if (instanceReference.getClassName() == "No_Properties")
        {
            CIMInstance instance(instanceReference.getClassName());
            handler.deliver(instance);
        }

        handler.complete();
    }

    void enumerateInstances(
        const OperationContext& context,
        const CIMObjectPath& classReference,
        const Boolean includeQualifiers,
        const Boolean includeClassOrigin,
        const CIMPropertyList& propertyList,
        InstanceResponseHandler& handler)
    {
    }

    void enumerateInstanceNames(
        const OperationContext& context,
        const CIMObjectPath& classReference,
        ObjectPathResponseHandler& handler)
    {
    }

    void modifyInstance(
        const OperationContext& context,
        const CIMObjectPath& instanceReference,
        const CIMInstance& instanceObject,
        const Boolean includeQualifiers,
        const CIMPropertyList& propertyList,
        ResponseHandler& handler)
    {
    }

    void createInstance(
        const OperationContext& context,
        const CIMObjectPath& instanceReference,
        const CIMInstance& instanceObject,
        ObjectPathResponseHandler& handler)
    {
    }

    void deleteInstance(
        const OperationContext& context,
        const CIMObjectPath& instanceReference,
        ResponseHandler& handler)
    {
    }
};

void testExceptionResponse(
    ProviderMessageHandler* pmh,
    CIMRequestMessage* request,
    const String& className)
{
    CIMResponseMessage* response = pmh->processMessage(request);
    CIMException e = response->cimException;

    if (className == "CIM_Exception")
    {
        PEGASUS_TEST_ASSERT(e.getCode() == CIM_ERR_NOT_FOUND);
        PEGASUS_TEST_ASSERT(e.getMessage() == "test cim exception");
    }
    else if (className == "Regular_Exception")
    {
        PEGASUS_TEST_ASSERT(e.getCode() == CIM_ERR_FAILED);
        PEGASUS_TEST_ASSERT(e.getMessage() == "test regular exception");
    }
    else if (className == "Cxx_Exception")
    {
        PEGASUS_TEST_ASSERT(e.getCode() == CIM_ERR_FAILED);
        PEGASUS_TEST_ASSERT(e.getMessage() == "test C++ exception");
    }
    else if (className == "Other_Exception")
    {
        PEGASUS_TEST_ASSERT(e.getCode() == CIM_ERR_FAILED);
        PEGASUS_TEST_ASSERT(e.getMessage() == "Unknown error.");
    }
    else if (className == "Not_Provider")
    {
        PEGASUS_TEST_ASSERT(e.getCode() == CIM_ERR_NOT_SUPPORTED);
        PEGASUS_TEST_ASSERT(e.getMessage() == "Invalid provider interface.");
    }
    else
    {
        PEGASUS_TEST_ASSERT(0);
    }

    delete response;
}

void testExceptions(
    ProviderMessageHandler* pmh,
    const String& className)
{
    String mid("1234");

    QueueIdStack qids;
    qids.push(10);
    qids.push(5);

    CIMNamespaceName ns("test/cimv2");

    Array<CIMKeyBinding> kbs;
    kbs.append(CIMKeyBinding("Index", 1));
    CIMObjectPath objectPath(String::EMPTY, ns, className, kbs);

    CIMInstance instance(className);
    CIMPropertyList pl;

    Array<CIMName> subClasses;
    subClasses.append(className);
    subClasses.append(className);
    subClasses.append(className);

    OperationContext oc;
    oc.insert(IdentityContainer("test user"));
    oc.insert(AcceptLanguageListContainer(AcceptLanguageList()));
    oc.insert(ContentLanguageListContainer(ContentLanguageList()));
    oc.insert(SubscriptionInstanceContainer(instance));
    oc.insert(SubscriptionFilterConditionContainer("cond", "WQL"));
    oc.insert(SubscriptionFilterQueryContainer("cond", "WQL", ns));
    oc.insert(ProviderIdContainer(instance, instance));

    {
        CIMGetInstanceRequestMessage request(
            mid, ns, objectPath, false, false, pl, qids);
        request.operationContext = oc;
        testExceptionResponse(pmh, &request, className);
    }

    {
        CIMEnumerateInstancesRequestMessage request(
            mid, ns, className, false, false, false, pl, qids);
        request.operationContext = oc;
        testExceptionResponse(pmh, &request, className);
    }

    {
        CIMEnumerateInstanceNamesRequestMessage request(
            mid, ns, className, qids);
        request.operationContext = oc;
        testExceptionResponse(pmh, &request, className);
    }

    {
        CIMCreateInstanceRequestMessage request(
            mid, ns, instance, qids);
        request.operationContext = oc;
        testExceptionResponse(pmh, &request, className);
    }

    {
        CIMModifyInstanceRequestMessage request(
            mid, ns, instance, false, pl, qids);
        request.operationContext = oc;
        testExceptionResponse(pmh, &request, className);
    }

    {
        CIMDeleteInstanceRequestMessage request(
            mid, ns, objectPath, qids);
        request.operationContext = oc;
        testExceptionResponse(pmh, &request, className);
    }

    {
        CIMExecQueryRequestMessage request(
            mid, ns, "WQL", String("SELECT * FROM ") + className, qids);
        request.className = className;
        request.operationContext = oc;
        testExceptionResponse(pmh, &request, className);
    }

    {
        CIMAssociatorsRequestMessage request(
            mid, ns, objectPath, className, className, "role1", "role2",
            false, false, pl, qids);
        request.operationContext = oc;
        testExceptionResponse(pmh, &request, className);
    }

    {
        CIMAssociatorNamesRequestMessage request(
            mid, ns, objectPath, className, className, "role1", "role2", qids);
        request.operationContext = oc;
        testExceptionResponse(pmh, &request, className);
    }

    {
        CIMReferencesRequestMessage request(
            mid, ns, objectPath, className, "role1", false, false, pl, qids);
        request.operationContext = oc;
        testExceptionResponse(pmh, &request, className);
    }

    {
        CIMReferenceNamesRequestMessage request(
            mid, ns, objectPath, className, "role1", qids);
        request.operationContext = oc;
        testExceptionResponse(pmh, &request, className);
    }

    {
        CIMGetPropertyRequestMessage request(
            mid, ns, objectPath, className, qids);
        request.operationContext = oc;
        testExceptionResponse(pmh, &request, className);
    }

    {
        CIMSetPropertyRequestMessage request(
            mid, ns, objectPath, className, CIMValue(Uint32(10)), qids);
        request.operationContext = oc;
        testExceptionResponse(pmh, &request, className);
    }

    {
        CIMInvokeMethodRequestMessage request(
            mid, ns, objectPath, className, Array<CIMParamValue>(), qids);
        request.operationContext = oc;
        testExceptionResponse(pmh, &request, className);
    }

    {
        CIMCreateSubscriptionRequestMessage request(
            mid, ns, instance, subClasses, pl, Uint16(1), "q", qids);
        request.operationContext = oc;
        testExceptionResponse(pmh, &request, className);
    }

    {
        CIMModifySubscriptionRequestMessage request(
            mid, ns, instance, subClasses, pl, Uint16(1), "q", qids);
        request.operationContext = oc;
        testExceptionResponse(pmh, &request, className);
    }

    {
        CIMDeleteSubscriptionRequestMessage request(
            mid, ns, instance, subClasses, qids);
        request.operationContext = oc;
        testExceptionResponse(pmh, &request, className);
    }

    {
        CIMExportIndicationRequestMessage request(
            mid, className, instance, qids);
        request.operationContext = oc;
        testExceptionResponse(pmh, &request, className);
    }
}

void testGetPropertyError()
{
    QueueIdStack qids;
    qids.push(10);
    qids.push(5);

    CIMNamespaceName ns("test/sample");

    OperationContext oc;
    oc.insert(IdentityContainer("test user"));
    oc.insert(AcceptLanguageListContainer(AcceptLanguageList()));
    oc.insert(ContentLanguageListContainer(ContentLanguageList()));

    GetPropertyErrorProvider np;
    ProviderMessageHandler pmh("GetPropertyErrorModule",
        "GetPropertyErrorProvider", &np, 0, 0, false);

    // Test GetProperty where the requested property is not contained in the
    // instance delivered.
    {
        CIMName className("No_Properties");

        Array<CIMKeyBinding> kbs;
        kbs.append(CIMKeyBinding("Index", 1));
        CIMObjectPath objectPath(String::EMPTY, ns, className, kbs);

        CIMGetPropertyRequestMessage request(
            "mid1", ns, objectPath, "theProperty", qids);
        request.operationContext = oc;

        AutoPtr<CIMResponseMessage> response(pmh.processMessage(&request));
        CIMException e = response->cimException;

        PEGASUS_TEST_ASSERT(e.getCode() == CIM_ERR_NO_SUCH_PROPERTY);
        PEGASUS_TEST_ASSERT(e.getMessage() == "theProperty");
    }
}

int main(int, char** argv)
{
    verbose = getenv ("PEGASUS_TEST_VERBOSE") ? true : false;

    try
    {
        ExceptionProvider ep;
        ProviderMessageHandler pmh("TestModule", "Test", &ep, 0, 0, false);
        testExceptions(&pmh, "CIM_Exception");
        testExceptions(&pmh, "Regular_Exception");
        testExceptions(&pmh, "Cxx_Exception");
        testExceptions(&pmh, "Other_Exception");

        NotARealProvider np;
        ProviderMessageHandler pmh2("BadModule", "BadProvider",
            &np, 0, 0, false);
        testExceptions(&pmh2, "Not_Provider");

        testGetPropertyError();
    }
    catch (Exception& e)
    {
        cout << "Caught unexpected exception: " << e.getMessage() << endl;
        return 1;
    }
    catch (...)
    {
        cout << "Caught unexpected exception" << endl;
        return 1;
    }

    cout << argv[0] << " +++++ passed all tests" << endl;

    return 0;
}
