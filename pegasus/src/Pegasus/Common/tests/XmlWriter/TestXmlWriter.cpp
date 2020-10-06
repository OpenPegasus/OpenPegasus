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

#include <cstdio>
#include <Pegasus/Common/PegasusAssert.h>
#include <Pegasus/Common/XmlWriter.h>
#include <Pegasus/Common/System.h>

PEGASUS_USING_PEGASUS;
PEGASUS_USING_STD;

static Boolean verbose = false;

static const char* xmlPropertyA =
    "<PROPERTY NAME=\"JustaName\"  TYPE=\"string\" EmbeddedObject=\"instance\""
        " EMBEDDEDOBJECT=\"instance\">\n"
            "<VALUE>&lt;INSTANCE CLASSNAME=&quot;className&quot; &gt;&#10;&lt;"
                "/INSTANCE&gt;&#10;</VALUE>\n"
                    "</PROPERTY>\n";

static const char* xmlPropertyAwithClassOrigin =
    "<PROPERTY NAME=\"JustaName\"  CLASSORIGIN=\"origin\" TYPE=\"string\""
        " EmbeddedObject=\"instance\" EMBEDDEDOBJECT=\"instance\">\n"
            "<VALUE>&lt;INSTANCE CLASSNAME=&quot;className&quot; &gt;&#10;&lt;"
                "/INSTANCE&gt;&#10;</VALUE>\n"
                    "</PROPERTY>\n";


static const char* xmlPropertyB =
    "<PROPERTY.REFERENCE NAME=\"JustaName\" >\n<VALUE.REFERENCE>\n"
        "<INSTANCENAME CLASSNAME=\"MyClass\">\n<KEYBINDING NAME=\"x\">\n"
            "<KEYVALUE VALUETYPE=\"numeric\">1</KEYVALUE>\n</KEYBINDING>\n"
                "</INSTANCENAME>\n</VALUE.REFERENCE>\n</PROPERTY.REFERENCE>\n";

static const char* xmlPropertyBwithClassOrigin =
    "<PROPERTY.REFERENCE NAME=\"JustaName\"  CLASSORIGIN=\"origin\">\n"
        "<VALUE.REFERENCE>\n<INSTANCENAME CLASSNAME=\"MyClass\">\n"
            "<KEYBINDING NAME=\"x\">\n"
                "<KEYVALUE VALUETYPE=\"numeric\">1</KEYVALUE>\n</KEYBINDING>\n"
                    "</INSTANCENAME>\n</VALUE.REFERENCE>\n"
                        "</PROPERTY.REFERENCE>\n";

static const char* xmlPropertyC =
    "<PROPERTY NAME=\"JustaName\"  TYPE=\"string\">\n"
        "<VALUE>stringValue</VALUE>\n"
            "</PROPERTY>\n";

static const char* xmlPropertyCwithClassOrigin =
    "<PROPERTY NAME=\"JustaName\"  CLASSORIGIN=\"origin\" TYPE=\"string\">\n"
        "<VALUE>stringValue</VALUE>\n"
            "</PROPERTY>\n";

/* function shall check that the XmlWriter does not generate an empty
   ClassOrigin for instance type properties */
void testClassOriginA()
{
    Buffer outputBuffer;

    const CIMName myPropertyName("JustaName");

    // create a tiny, little CIM instance
    const CIMName instanceClassName("className");
    const CIMInstance x(instanceClassName);

    const CIMValue propertyAValue(x);
    CIMProperty testPropertyA(myPropertyName, propertyAValue);
    XmlWriter::appendPropertyElement(outputBuffer, testPropertyA);

    if (verbose) cout << outputBuffer.getData() << endl;

    PEGASUS_TEST_ASSERT(
        System::strcasecmp(outputBuffer.getData(),xmlPropertyA) == 0);
    // cross-check that classOrigin is written if it has a value
    const CIMName classOriginName("origin");
    testPropertyA.setClassOrigin(classOriginName);
    outputBuffer.clear();
    XmlWriter::appendPropertyElement(outputBuffer, testPropertyA);

    if (verbose) cout << outputBuffer.getData() << endl;
    PEGASUS_TEST_ASSERT(
        System::strcasecmp(
            outputBuffer.getData(),
            xmlPropertyAwithClassOrigin) == 0);
    return;
}

/* function shall check that the XmlWriter does not generate an empty
   ClassOrigin for reference type properties */
void testClassOriginB()
{
    Buffer outputBuffer;

    const CIMName myPropertyName("JustaName");

    // create a tiny, little CIM object path
    const CIMObjectPath x("MyClass.x=1");

    const CIMValue propertyBValue(x);
    CIMProperty testPropertyB(myPropertyName, propertyBValue);
    XmlWriter::appendPropertyElement(outputBuffer, testPropertyB);

    if (verbose) cout << outputBuffer.getData() << endl;
    PEGASUS_TEST_ASSERT(
        System::strcasecmp(outputBuffer.getData(),xmlPropertyB) == 0);
    // cross-check that classOrigin is written if it has a value
    const CIMName classOriginName("origin");
    testPropertyB.setClassOrigin(classOriginName);
    outputBuffer.clear();
    XmlWriter::appendPropertyElement(outputBuffer, testPropertyB);

    if (verbose) cout << outputBuffer.getData() << endl;
    PEGASUS_TEST_ASSERT(
        System::strcasecmp(
            outputBuffer.getData(),
            xmlPropertyBwithClassOrigin) == 0);

    return;
}

/* function shall check that the XmlWriter does not generate an empty
   ClassOrigin for basic type properties */
void testClassOriginC()
{
    Buffer outputBuffer;

    const CIMName myPropertyName("JustaName");
    const String x("stringValue");
    const CIMValue propertyCValue(x);
    CIMProperty testPropertyC(myPropertyName, propertyCValue);
    XmlWriter::appendPropertyElement(outputBuffer, testPropertyC);

    if (verbose) cout << outputBuffer.getData() << endl;
    PEGASUS_TEST_ASSERT(
        System::strcasecmp(outputBuffer.getData(),xmlPropertyC) == 0);

    // cross-check that classOrigin is written if it has a value
    const CIMName classOriginName("origin");
    testPropertyC.setClassOrigin(classOriginName);
    outputBuffer.clear();
    XmlWriter::appendPropertyElement(outputBuffer, testPropertyC);

    if (verbose) cout << outputBuffer.getData() << endl;
    PEGASUS_TEST_ASSERT(
        System::strcasecmp(
            outputBuffer.getData(),
            xmlPropertyCwithClassOrigin) == 0);
    return;
}

int main(int, char** argv)
{
    verbose = getenv("PEGASUS_TEST_VERBOSE") ? true : false;

    // create a CIMProperty with an empty ClassOrigin and feed that to the
    // XmlWriter
    // three cases need to be considered:
    // property is

    // a) an instance
    testClassOriginA();

    // b) a reference
    testClassOriginB();

    // c) a basic type property
    testClassOriginC();

    cout << argv[0] << " +++++ passed all tests" << endl;
    return 0;
}
