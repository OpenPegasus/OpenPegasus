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
// Author: Nitin Upasani, Hewlett-Packard Company (Nitin_Upasani@hp.com)
//
// Modified By: Jenny Yu, Hewlett-Packard Company (jenny_yu@hp.com)
//              Roger Kumpf, Hewlett-Packard Company (roger_kumpf@hp.com)
//              Carol Ann Krug Graves, Hewlett-Packard Company
//                  (carolann_graves@hp.com)
//              David Dillard, VERITAS Software Corp.
//                  (david.dillard@veritas.com)
//
//%/////////////////////////////////////////////////////////////////////////////

#include <Pegasus/Common/PegasusAssert.h>
#include <Pegasus/Common/CIMParamValue.h>
#include <Pegasus/Common/XmlWriter.h>

PEGASUS_USING_PEGASUS;
PEGASUS_USING_STD;
static Boolean verbose;

void test01()
{
    CIMParamValue pv;
    PEGASUS_TEST_ASSERT(pv.isUninitialized());

    String p1("message");
    CIMValue v1(String("argument_Test"));
    CIMParamValue a1(p1, v1);

//  Test call to CIMParamValue::setValue(CIMValue& value)
//  this test uses the above values for a1, p1 and v1.
    CIMParamValue a7(p1, v1);
    CIMValue v7(String("argument_Test7"));
    a7.setValue( v7);
    PEGASUS_TEST_ASSERT(a1.getValue().toString() != a7.getValue().toString());
    PEGASUS_TEST_ASSERT(a1.getParameterName() == a7.getParameterName());

//  Test call to CIMParamValue::setParameterName(String& parameterName)
//  this test uses the above values for a1, p1 and v1.
    CIMParamValue a8(p1, v1);
    String p8("message8");
    a8.setParameterName( p8);
    PEGASUS_TEST_ASSERT(a1.getValue().toString() == a8.getValue().toString());
    PEGASUS_TEST_ASSERT(a1.getParameterName() != a8.getParameterName());
    PEGASUS_TEST_ASSERT(a8.isTyped());
    a8.setIsTyped(false);
    PEGASUS_TEST_ASSERT(!a8.isTyped());

    String p2("message2");
    CIMValue v2(String("argument_Test2"));
    CIMParamValue a2(p2, v2);

    String p3("message3");
    CIMValue v3(String("argument_Test3"));
    CIMParamValue a3(p3, v3);

    String p4("message4");
    CIMValue v4(String("argument_Test4"));
    CIMParamValue a4(p4, v4);
    CIMParamValue a5 = a4;

    String p6("message6");
    CIMValue v6(String("argument_Test6"));
    CIMParamValue a6(p6, v6);

    Array<CIMParamValue> aa;
    aa.append(a1);
    aa.append(a2);

    aa.append(CIMParamValue("message3", CIMValue(200000)));

    aa.append(CIMParamValue("message4", CIMValue(String("test4"))));

    //
    // clone
    //
    CIMParamValue a4clone = a4.clone();
    aa.append(a4clone);

    if (verbose)
    {
        for (Uint32 i=0; i< aa.size(); i++)
        {
            XmlWriter::printParamValueElement(aa[i], cout);
        }
    }

    //
    // toXml
    //
    Buffer xmlOut;
    XmlWriter::appendParamValueElement(xmlOut, a4clone);
}

#define TEST_UNINITIALIZED_OBJECT_EXCEPTION(statement) \
    {                                                  \
        Boolean gotException = false;                  \
        try                                            \
        {                                              \
            statement;                                 \
        }                                              \
        catch (const UninitializedObjectException&)    \
        {                                              \
            gotException = true;                       \
        }                                              \
        PEGASUS_TEST_ASSERT(gotException);             \
    }

void testUninitializedObject()
{
    CIMParamValue uninitializedParamValue;
    String name("name");
    CIMValue value(String("value"));

    PEGASUS_TEST_ASSERT(uninitializedParamValue.isUninitialized());

    // Copy constructor allows an uninitialized object
    {
        CIMParamValue initializedParamValue(uninitializedParamValue);
    }

    // Assignment operator allows an uninitialized object
    {
        CIMParamValue initializedParamValue(name, value);
        initializedParamValue = uninitializedParamValue;
    }

    TEST_UNINITIALIZED_OBJECT_EXCEPTION(
        uninitializedParamValue.getParameterName());
    TEST_UNINITIALIZED_OBJECT_EXCEPTION(
        uninitializedParamValue.getValue());
    TEST_UNINITIALIZED_OBJECT_EXCEPTION(
        uninitializedParamValue.isTyped());
    TEST_UNINITIALIZED_OBJECT_EXCEPTION(
        uninitializedParamValue.setParameterName(name));
    TEST_UNINITIALIZED_OBJECT_EXCEPTION(
        uninitializedParamValue.setValue(value));
    TEST_UNINITIALIZED_OBJECT_EXCEPTION(
        uninitializedParamValue.setIsTyped(true));
    TEST_UNINITIALIZED_OBJECT_EXCEPTION(
        uninitializedParamValue.clone());
}

int main(int, char** argv)
{
    verbose = getenv("PEGASUS_TEST_VERBOSE") ? true : false;

    try
    {
        test01();
        testUninitializedObject();
    }
    catch (Exception& e)
    {
        cout << "Exception: " << e.getMessage() << endl;
        return 1;
    }

    cout << argv[0] << " +++++ passed all tests" << endl;
    return 0;
}
