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

#include "TestOperationsProvider.h"

PEGASUS_NAMESPACE_BEGIN

#define CIMOMHANDLE_TEST_ASSERT(expr)                              \
    if (!(expr))                                                   \
    {                                                              \
        char msg[256];                                             \
        sprintf(msg, "Assertion (%s) failed in file %s, line %d.", \
            #expr, __FILE__, __LINE__);                            \
        throw CIMOperationFailedException(msg);                    \
    }

static void testCIMOMHandle(CIMOMHandle& cimomHandle)
{
    OperationContext context;
    const CIMNamespaceName opNamespace("test/TestProvider");
    const CIMName opBaseClassName("TST_OperationsBase");
    const CIMName op1ClassName("TST_Operations1");
    const CIMName op2ClassName("TST_Operations2");
    const CIMName opAssocClassName("TST_OperationsAssoc");
    const CIMName opTempClassName("TST_OperationsTemp");
    const CIMPropertyList plNull;
    const CIMObjectPath instance1Name("TST_Operations1.key=1");
    const CIMObjectPath instance2Name("TST_Operations1.key=2");
    const CIMObjectPath instance3Name("TST_Operations2.key=3");
    const CIMObjectPath assocInstance1Name(
        "TST_OperationsAssoc.a=\"TST_Operations1.key=1\","
            "b=\"TST_Operations2.key=3\"");
    const CIMObjectPath assocInstance2Name(
        "TST_OperationsAssoc.a=\"TST_Operations1.key=2\","
            "b=\"TST_Operations2.key=3\"");

    // Test GetClass operation
    {
        CIMClass c = cimomHandle.getClass(
            context, opNamespace, op1ClassName, false, true, true, plNull);
        CIMOMHANDLE_TEST_ASSERT(c.getPropertyCount() == 2);
    }

    // Test EnumerateClasses operation
    {
        Array<CIMClass> c = cimomHandle.enumerateClasses(
            context, opNamespace, opBaseClassName, true, true, true, true);
        CIMOMHANDLE_TEST_ASSERT(c.size() == 4);
    }

    // Test EnumerateClassNames operation
    {
        Array<CIMName> c = cimomHandle.enumerateClassNames(
            context, opNamespace, opBaseClassName, true);
        CIMOMHANDLE_TEST_ASSERT(c.size() == 4);
    }

    // Test CreateClass operation
    {
        CIMClass c(opTempClassName, opBaseClassName);
        CIMProperty p("key1", CIMValue(Uint32(0)));
        p.addQualifier(CIMQualifier("Key", CIMValue(true)));
        c.addProperty(p);
        cimomHandle.createClass(context, opNamespace, c);
    }

    // Test ModifyClass operation
    {
        CIMClass c = cimomHandle.getClass(
            context, opNamespace, opTempClassName, true, true, true, plNull);
        CIMOMHANDLE_TEST_ASSERT(c.getPropertyCount() == 1);
        c.addProperty(CIMProperty("p2", CIMValue(Boolean(false))));
        cimomHandle.modifyClass(context, opNamespace, c);
    }

    // Test DeleteClass operation
    {
        CIMClass c = cimomHandle.getClass(
            context, opNamespace, opTempClassName, false, true, true, plNull);
        CIMOMHANDLE_TEST_ASSERT(c.getPropertyCount() == 3);
        cimomHandle.deleteClass(context, opNamespace, c.getClassName());

        Boolean gotException = false;
        try
        {
            CIMClass deletedClass = cimomHandle.getClass(
                context, opNamespace, opTempClassName, false, true, true,
                plNull);
        }
        catch (const CIMException&)
        {
            gotException = true;
        }
        CIMOMHANDLE_TEST_ASSERT(gotException);
    }

    // Test GetInstance operation
    {
        CIMInstance i = cimomHandle.getInstance(
            context, opNamespace, instance1Name, false, true, true, plNull);
        CIMOMHANDLE_TEST_ASSERT(i.getPropertyCount() == 2);
    }

    // Test EnumerateInstances operation
    {
        Array<CIMInstance> i = cimomHandle.enumerateInstances(
            context, opNamespace, op1ClassName, true, false, true, true,
            plNull);
        CIMOMHANDLE_TEST_ASSERT(i.size() == 2);
    }

    // Test EnumerateInstanceNames operation
    {
        Array<CIMObjectPath> i = cimomHandle.enumerateInstanceNames(
            context, opNamespace, op2ClassName);
        CIMOMHANDLE_TEST_ASSERT(i.size() == 1);
    }

    // Test CreateInstance operation
    {
        CIMInstance i(op1ClassName);
        i.addProperty(CIMProperty(
            CIMName("Description"), CIMValue(String("Temporary instance"))));
        i.addProperty(CIMProperty(CIMName("key"), CIMValue(Uint32(3))));
        cimomHandle.createInstance(context, opNamespace, i);
    }

    // Test ModifyInstance operation
    {
        CIMInstance i = cimomHandle.getInstance(
            context, opNamespace, CIMObjectPath("TST_Operations1.key=3"),
            false, true, true, plNull);
        i.setPath(CIMObjectPath("TST_Operations1.key=3"));
        i.removeProperty(i.findProperty("Description"));
        i.addProperty(CIMProperty(
            CIMName("Description"), CIMValue(String("Temporary"))));
        cimomHandle.modifyInstance(context, opNamespace, i, true, plNull);

        i = cimomHandle.getInstance(
            context, opNamespace, CIMObjectPath("TST_Operations1.key=3"),
            false, true, true, plNull);
        String s;
        i.getProperty(i.findProperty("Description")).getValue().get(s);
        CIMOMHANDLE_TEST_ASSERT(s == "Temporary");
    }

    // Test DeleteInstance operation
    {
        CIMInstance i = cimomHandle.getInstance(
            context, opNamespace, CIMObjectPath("TST_Operations1.key=3"),
            false, true, true, plNull);
        cimomHandle.deleteInstance(
            context, opNamespace, CIMObjectPath("TST_Operations1.key=3"));

        Boolean gotException = false;
        try
        {
            CIMInstance deletedInstance = cimomHandle.getInstance(
                context, opNamespace, CIMObjectPath("TST_Operations1.key=3"),
                false, true, true, plNull);
        }
        catch (const CIMException&)
        {
            gotException = true;
        }
        CIMOMHANDLE_TEST_ASSERT(gotException);
    }

    // Test ExecQuery operation
    {
        try
        {
            Array<CIMObject> o = cimomHandle.execQuery(
                context, opNamespace, "WQL", "SELECT * FROM TST_Operations1");
        }
        catch (const CIMException&)
        {
        }
        // Results are dependent on CIM Server query support
    }

    // Test Associators operation
    {
        Array<CIMObject> o = cimomHandle.associators(
            context, opNamespace, instance1Name, CIMName(), CIMName(),
            String::EMPTY, String::EMPTY, true, true, plNull);
        CIMOMHANDLE_TEST_ASSERT(o.size() == 1);
    }

    // Test AssociatorNames operation
    {
        Array<CIMObjectPath> o = cimomHandle.associatorNames(
            context, opNamespace, instance3Name, CIMName(), CIMName(),
            String::EMPTY, String::EMPTY);
        CIMOMHANDLE_TEST_ASSERT(o.size() == 2);
    }

    // Test References operation
    {
        Array<CIMObject> o = cimomHandle.references(
            context, opNamespace, instance1Name, CIMName(), String::EMPTY,
            true, true, plNull);
        CIMOMHANDLE_TEST_ASSERT(o.size() == 1);
    }

    // Test ReferenceNames operation
    {
        Array<CIMObjectPath> o = cimomHandle.referenceNames(
            context, opNamespace, instance3Name, CIMName(), String::EMPTY);
        CIMOMHANDLE_TEST_ASSERT(o.size() == 2);
    }

    // Test GetProperty operation
    {
        CIMValue v = cimomHandle.getProperty(
            context, opNamespace, instance1Name, CIMName("key"));

        CIMOMHANDLE_TEST_ASSERT (v.getType() == CIMTYPE_STRING);
        String keyValue;
        v.get(keyValue);
        CIMOMHANDLE_TEST_ASSERT(keyValue == "1");
    }

    // Test SetProperty operation
    {
        // Create a temporary instance
        CIMInstance i(op1ClassName);
        i.addProperty(CIMProperty(
            CIMName("Description"), CIMValue(String("Temporary instance"))));
        i.addProperty(CIMProperty(CIMName("key"), CIMValue(Uint32(3))));
        const CIMObjectPath tempInstanceName("TST_Operations1.key=3");
        cimomHandle.createInstance(context, opNamespace, i);
        CIMInstance i1 = cimomHandle.getInstance(
            context, opNamespace, tempInstanceName, false, true, true, plNull);

        // Set the Description property to a new value
        cimomHandle.setProperty(
            context, opNamespace, tempInstanceName, CIMName("Description"),
            String("Different value"));
        CIMInstance i2 = cimomHandle.getInstance(
            context, opNamespace, tempInstanceName, false, true, true, plNull);
        CIMOMHANDLE_TEST_ASSERT(
            !i1.getProperty(i1.findProperty(CIMName("Description"))).identical(
                i2.getProperty(i2.findProperty(CIMName("Description")))));

        // Set the Description property back to the original value
        cimomHandle.setProperty(
            context, opNamespace, tempInstanceName, CIMName("Description"),
            String("Temporary instance"));
        CIMInstance i3 = cimomHandle.getInstance(
            context, opNamespace, tempInstanceName, false, true, true, plNull);
        CIMOMHANDLE_TEST_ASSERT(
            i1.getProperty(i1.findProperty(CIMName("Description"))).identical(
                i3.getProperty(i3.findProperty(CIMName("Description")))));

        // Delete the temporary instance
        cimomHandle.deleteInstance(context, opNamespace, tempInstanceName);
    }

    // Test InvokeMethod operation
    {
        Array<CIMParamValue> in;
        in.append(CIMParamValue("param1", String("InParam")));
        in.append(CIMParamValue("param2", Uint32(1)));
        Array<CIMParamValue> out;
        CIMValue v = cimomHandle.invokeMethod(
            context, opNamespace, CIMObjectPath("TST_OperationsDriver"),
            CIMName("testMethod"), in, out);
        Uint32 intValue;
        v.get(intValue);
        CIMOMHANDLE_TEST_ASSERT(intValue == 10);
        CIMOMHANDLE_TEST_ASSERT(out.size() == 1);
        CIMOMHANDLE_TEST_ASSERT(out[0].getParameterName() == "param2");
        out[0].getValue().get(intValue);
        CIMOMHANDLE_TEST_ASSERT(intValue == 2);

        OperationContext responseContext = cimomHandle.getResponseContext();
        ContentLanguageListContainer contentLanguagesContainer =
            (ContentLanguageListContainer)
                responseContext.get(ContentLanguageListContainer::NAME);
        ContentLanguageList contentLanguages =
            contentLanguagesContainer.getLanguages();
        CIMOMHANDLE_TEST_ASSERT(contentLanguages.size() == 1);
        CIMOMHANDLE_TEST_ASSERT(
            contentLanguages.getLanguageTag(0) == LanguageTag("en-US"));
    }
}

TestOperationsProvider::TestOperationsProvider()
{
}

TestOperationsProvider::~TestOperationsProvider()
{
}

void TestOperationsProvider::initialize(CIMOMHandle & cimom)
{
    _cimom = cimom;
}

void TestOperationsProvider::terminate()
{
    delete this;
}

void TestOperationsProvider::invokeMethod(
    const OperationContext & context,
    const CIMObjectPath & objectReference,
    const CIMName & methodName,
    const Array<CIMParamValue> & inParameters,
    MethodResultResponseHandler & handler)
{
    handler.processing();

    if (methodName == "testCIMOMHandle")
    {
        testCIMOMHandle(_cimom);
        handler.deliver(Uint32(0));
    }
    else if (methodName == "testMethod")
    {
        OperationContext responseContext;
        ContentLanguageList contentLanguages;
        contentLanguages.append(LanguageTag("en-US"));
        responseContext.insert(ContentLanguageListContainer(contentLanguages));
        handler.setContext(responseContext);

        CIMOMHANDLE_TEST_ASSERT(inParameters.size() == 2);
        String param1;
        Uint32 param2;
        if (inParameters[0].getParameterName() == "param1")
        {
            inParameters[0].getValue().get(param1);
            inParameters[1].getValue().get(param2);
        }
        else
        {
            inParameters[1].getValue().get(param1);
            inParameters[0].getValue().get(param2);
        }
        CIMOMHANDLE_TEST_ASSERT(param1 == "InParam");
        CIMOMHANDLE_TEST_ASSERT(param2 == 1);
        handler.deliverParamValue(CIMParamValue("param2", Uint32(2)));
        handler.deliver(CIMValue(Uint32(10)));
    }
    else
    {
        throw CIMOperationFailedException(
            String("Unrecognized method name: ") + methodName.getString());
    }

    handler.complete();
}

PEGASUS_NAMESPACE_END

