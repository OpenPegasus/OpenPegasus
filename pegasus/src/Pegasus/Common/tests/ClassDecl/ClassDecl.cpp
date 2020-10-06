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
//
//%/////////////////////////////////////////////////////////////////////////////

#include <Pegasus/Common/PegasusAssert.h>
#include <iostream>
#include <Pegasus/Common/CIMClass.h>
#include <Pegasus/Common/CIMName.h>
#include <Pegasus/Common/XmlWriter.h>
#include <Pegasus/General/MofWriter.h>

// ATTN-P3-KS - 20 March 2002 - Extend exception tests.
PEGASUS_USING_PEGASUS;
PEGASUS_USING_STD;
static Boolean verbose;

void test01()
{
    // class MyClass : YourClass
    // {
    //     string message = "Hello";
    // }
    try
    {
        CIMName a = "A_class1";
        CIMName b = "A_class2";
        CIMClass c0(a, b);
        CIMClass c1(a, CIMName("A_class2"));
        CIMClass c2(CIMName("A_class1"), b);
        CIMClass c3(b, a);
    }
    catch (InvalidNameException & ine)
    {
        if (verbose)
        {
        cout << "Caught unexpected exception: " << ine.getMessage() << endl;
        }
    }
    try
    {
        //
        //  Invalid class name
        //
        CIMClass class0(CIMName ("//localhost/root/cimv2:MyClass"),
            CIMName ("YourClass"));

        PEGASUS_TEST_ASSERT(class0.getPath() ==
            CIMObjectPath("//localhost/root/cimv2:MyClass"));
    }
    catch (InvalidNameException & ine)
    {
        if (verbose)
        {
        cout << "Caught expected exception: " << ine.getMessage() << endl;
        }
    }

    CIMClass class1(CIMName ("MyClass"), CIMName ("YourClass"));

    class1
    .addQualifier(CIMQualifier(CIMName ("association"), true))
    .addQualifier(CIMQualifier(CIMName ("q1"), Uint32(55)))
    .addQualifier(CIMQualifier(CIMName ("q2"), String("Hello")))
    .addProperty(CIMProperty(CIMName ("message"), String("Hello")))
    .addProperty(CIMProperty(CIMName ("count"), Uint32(77), 0, CIMName(),
            CIMName("YourClass"), true))
    .addMethod(CIMMethod(CIMName ("isActive"), CIMTYPE_BOOLEAN)
        .addParameter(CIMParameter(CIMName ("hostname"), CIMTYPE_STRING))
        .addParameter(CIMParameter(CIMName ("port"), CIMTYPE_UINT32)));

    // Test the method count function
    PEGASUS_TEST_ASSERT(class1.getClassName().equal(CIMName ("myclass")));
    PEGASUS_TEST_ASSERT(class1.getSuperClassName() == CIMName ("YourClass"));

    PEGASUS_TEST_ASSERT(class1.getMethodCount() ==1);


    // Test the findMethod and isMethod functions
    PEGASUS_TEST_ASSERT(class1.findMethod(
                CIMName ("isActive")) != PEG_NOT_FOUND);
    PEGASUS_TEST_ASSERT(class1.findMethod(
                CIMName ("DoesNotExist")) == PEG_NOT_FOUND);

    PEGASUS_TEST_ASSERT(class1.findMethod(
                CIMName ("isActive")) != PEG_NOT_FOUND);
    PEGASUS_TEST_ASSERT(class1.findMethod(
                CIMName ("DoesNotExist")) == PEG_NOT_FOUND);

    // Test the manipulation of an embeddedObjectProperty
    CIMClass embedClass(CIMName ("embedObj"), CIMName ());
    class1.addProperty(CIMProperty(CIMName ("embedObj"),
        CIMObject(embedClass), 0, CIMName(),
            CIMName(), false));

    PEGASUS_TEST_ASSERT(class1.findProperty(
                CIMName ("embedObj")) != PEG_NOT_FOUND);
    Uint32  posProp = class1.findProperty(CIMName ("embedObj"));
    CIMConstProperty constprop1 = class1.getProperty(posProp);
    PEGASUS_TEST_ASSERT(constprop1.getClassOrigin() == CIMName());
    PEGASUS_TEST_ASSERT(constprop1.getType() == CIMTYPE_OBJECT);
    class1.removeProperty(posProp);

    // Now add another method and reconfirm.

    class1.addMethod(CIMMethod(CIMName ("makeActive"), CIMTYPE_BOOLEAN)
    .addParameter(CIMParameter(CIMName ("hostname"), CIMTYPE_STRING))
    .addParameter(CIMParameter(CIMName ("port"), CIMTYPE_UINT32)));

    PEGASUS_TEST_ASSERT(class1.getMethodCount() == 2);

    // Test the findMethod and isMethod functions
    // with two methods defined
    PEGASUS_TEST_ASSERT(class1.findMethod(
                CIMName ("isActive")) != PEG_NOT_FOUND);
    PEGASUS_TEST_ASSERT(class1.findMethod(
                CIMName ("makeActive")) != PEG_NOT_FOUND);

    PEGASUS_TEST_ASSERT(class1.findMethod(
                CIMName ("DoesNotExist")) == PEG_NOT_FOUND);
    PEGASUS_TEST_ASSERT(class1.findMethod(
                CIMName ("isActive")) != PEG_NOT_FOUND);
    PEGASUS_TEST_ASSERT(class1.findMethod(
                CIMName ("makeActive")) != PEG_NOT_FOUND);

    PEGASUS_TEST_ASSERT(class1.findMethod(
                CIMName ("DoesNotExist")) == PEG_NOT_FOUND);


    // Test RemoveMethod function
    Uint32 posMethod;
    posMethod = class1.findMethod(CIMName ("isActive"));
    PEGASUS_TEST_ASSERT(posMethod != PEG_NOT_FOUND);

    class1.removeMethod(posMethod);

    PEGASUS_TEST_ASSERT(class1.findMethod(
                CIMName ("isActive")) == PEG_NOT_FOUND);
    PEGASUS_TEST_ASSERT(class1.getMethodCount() == 1);

    //ATTN: P3 TODO add tests for different case names

    //Qualifier manipulation tests  (find, remove)

    PEGASUS_TEST_ASSERT(class1.findQualifier(CIMName ("q1")) != PEG_NOT_FOUND);
    PEGASUS_TEST_ASSERT(class1.findQualifier(CIMName ("q2")) != PEG_NOT_FOUND);
    PEGASUS_TEST_ASSERT(class1.findQualifier(CIMName ("qx")) == PEG_NOT_FOUND);

    PEGASUS_TEST_ASSERT(class1.findQualifier(CIMName ("q1")) != PEG_NOT_FOUND);
    PEGASUS_TEST_ASSERT(class1.findQualifier(CIMName ("q2")) != PEG_NOT_FOUND);
    PEGASUS_TEST_ASSERT(class1.findQualifier(
                CIMName ("association")) != PEG_NOT_FOUND);
    PEGASUS_TEST_ASSERT(class1.isAssociation());

    // Remove middle Qualifier "q2"
    Uint32 posQualifier;
    posQualifier = class1.findQualifier(CIMName ("q2"));
    CIMConstQualifier qconst = class1.getQualifier(posQualifier);

    PEGASUS_TEST_ASSERT(class1.getQualifierCount() == 3);
    PEGASUS_TEST_ASSERT(posQualifier <= class1.getQualifierCount());
    class1.removeQualifier(posQualifier);
    PEGASUS_TEST_ASSERT(class1.getQualifierCount() == 2);

    PEGASUS_TEST_ASSERT(class1.findQualifier(CIMName ("q2")) == PEG_NOT_FOUND);
    PEGASUS_TEST_ASSERT(class1.findQualifier(CIMName ("q1")) != PEG_NOT_FOUND);
    PEGASUS_TEST_ASSERT(class1.isAssociation());


    // Remove the first parameter "q1"
    posQualifier = class1.findQualifier(CIMName ("q1"));

    PEGASUS_TEST_ASSERT(class1.getQualifierCount() == 2);
    CIMQualifier cq = class1.getQualifier( class1.findQualifier(
                CIMName ("q1")));
    PEGASUS_TEST_ASSERT(posQualifier <= class1.getQualifierCount());
    class1.removeQualifier(posQualifier);
    PEGASUS_TEST_ASSERT(class1.getQualifierCount() == 1);

    PEGASUS_TEST_ASSERT(class1.findQualifier(CIMName ("q1")) == PEG_NOT_FOUND);
    PEGASUS_TEST_ASSERT(class1.findQualifier(CIMName ("q2")) == PEG_NOT_FOUND);
    PEGASUS_TEST_ASSERT(class1.isAssociation());


    // ATTH: P3 Add tests for try block for outofbounds



    //The property manipulation tests.

    PEGASUS_TEST_ASSERT(class1.findProperty(
                CIMName ("count")) != PEG_NOT_FOUND);
    PEGASUS_TEST_ASSERT(class1.findProperty(
                CIMName ("message")) != PEG_NOT_FOUND);

    PEGASUS_TEST_ASSERT(class1.findProperty(
                CIMName ("isActive")) == PEG_NOT_FOUND);

    PEGASUS_TEST_ASSERT(class1.getPropertyCount() == 2);


    Uint32  posProperty;
    posProperty = class1.findProperty(CIMName ("count"));
    CIMConstProperty constprop = class1.getProperty(posProperty);
    PEGASUS_TEST_ASSERT(constprop.getClassOrigin() == CIMName("YourClass"));
    PEGASUS_TEST_ASSERT(constprop.getPropagated());
    class1.removeProperty(posProperty);
    PEGASUS_TEST_ASSERT(class1.findProperty(
                CIMName ("message")) != PEG_NOT_FOUND);
    PEGASUS_TEST_ASSERT(class1.findProperty(
                CIMName ("count")) == PEG_NOT_FOUND);

    PEGASUS_TEST_ASSERT(class1.getPropertyCount() == 1);
    CIMProperty cp = class1.getProperty( class1.findProperty
        (CIMName ("message")));
    PEGASUS_TEST_ASSERT(cp.getClassOrigin().isNull());
    PEGASUS_TEST_ASSERT(!cp.getPropagated());

    if(verbose)
    {
        XmlWriter::printClassElement(class1);
        MofWriter::printClassElement(class1);
    }

    Buffer out;
    MofWriter::appendClassElement(out, class1);
    out.clear();
    XmlWriter::appendClassElement(out, class1);

    PEGASUS_TEST_ASSERT(!class1.isAbstract());

    CIMName squal("q1");
    PEGASUS_TEST_ASSERT(class1.findQualifier(squal) == PEG_NOT_FOUND);

    PEGASUS_TEST_ASSERT(!class1.hasKeys());

    Array<CIMName> keyNames;
    class1.getKeyNames(keyNames);

    CIMClass c2(CIMName ("MyClass"));

    PEGASUS_TEST_ASSERT(c2.getClassName().equal(CIMName ("myclass")));


    // Error uninitialized handle
    c2.setSuperClassName(CIMName ("CIM_Element"));
    PEGASUS_TEST_ASSERT(c2.getSuperClassName() == CIMName ("CIM_Element"));

    CIMClass c3 = c2.clone();
    c3 = c2;


    try
    {
        CIMMethod cm = c2.getMethod(0);
    }
    catch(IndexOutOfBoundsException& e)
    {
        if(verbose)
            cout << "Exception: " << e.getMessage() << endl;
    }

    const CIMClass c4(CIMName ("MyClass"), CIMName ("YourClass"));

    CIMConstClass c5(CIMName ("MyClass"), CIMName ("YourClass"));
    CIMConstClass c6(CIMName ("MyClass"));
    CIMConstClass cc7(c6);
    CIMClass c7 = c5.clone();
    const CIMClass c8(class1);

    // Test the findMethod and isMethod functions
    PEGASUS_TEST_ASSERT(c7.findMethod(
                CIMName ("DoesNotExist")) == PEG_NOT_FOUND);

    PEGASUS_TEST_ASSERT(c7.findQualifier(CIMName ("dummy")) == PEG_NOT_FOUND);

    try
    {
        CIMConstMethod cm = c8.getMethod(0);
    }
    catch(IndexOutOfBoundsException& e)
    {
        if(verbose)
            cout << "Exception: " << e.getMessage() << endl;
    }

    try
    {
        CIMConstProperty ccp = c8.getProperty(c8.findProperty
            (CIMName ("count")));
    }
    catch(IndexOutOfBoundsException& e)
    {
        if(verbose)
            cout << "Exception: " << e.getMessage() << endl;
    }

    if(verbose)
    {
    XmlWriter::printClassElement(c5);
    }

    try
    {
        CIMConstMethod cm = cc7.getMethod(0);
    }
    catch(IndexOutOfBoundsException& e)
    {
    if(verbose)
        cout << "Exception: " << e.getMessage() << endl;
    }
    // Test the findMethod and isMethod functions
    PEGASUS_TEST_ASSERT(c4.findMethod(
                CIMName ("DoesNotExist")) == PEG_NOT_FOUND);

    //Qualifier manipulation tests  (find, remove)

    PEGASUS_TEST_ASSERT(c4.findQualifier(CIMName ("qx")) == PEG_NOT_FOUND);

    PEGASUS_TEST_ASSERT(c4.findQualifier(CIMName ("q1")) == PEG_NOT_FOUND);
    PEGASUS_TEST_ASSERT(c4.findQualifier(CIMName ("q2")) == PEG_NOT_FOUND);
    PEGASUS_TEST_ASSERT(c4.findQualifier(
                CIMName ("association")) == PEG_NOT_FOUND);

    posProperty = c4.findProperty(CIMName ("count"));

    try
    {
        CIMConstQualifier ccq = c4.getQualifier(c4.findQualifier
            (CIMName ("q1")));
    }
    catch (IndexOutOfBoundsException& e)
    {
        if(verbose)
            cout << "Exception: " << e.getMessage() << endl;
    }

    PEGASUS_TEST_ASSERT(c4.findProperty(CIMName ("count")) == PEG_NOT_FOUND);

    PEGASUS_TEST_ASSERT(c4.getClassName() == CIMName ("MyClass"));
    PEGASUS_TEST_ASSERT(c4.getClassName().equal(CIMName ("MyClass")));
    PEGASUS_TEST_ASSERT(c4.getClassName().equal(CIMName ("MYCLASS")));
    PEGASUS_TEST_ASSERT(c4.getClassName().equal(CIMName ("myclass")));
    PEGASUS_TEST_ASSERT(!c4.getClassName().equal(CIMName ("blob")));


    PEGASUS_TEST_ASSERT(c4.getSuperClassName() == CIMName ("YourClass"));

    // test the setSuperClassName function
    /* ATTN KS 29 April.  This test has problems.  Relook later.
      Think test, not code.
    c4.setSuperClassName(CIMName ("JunkClass"));
    PEGASUS_TEST_ASSERT(c4.getSuperClassName() == CIMName ("JunkClass"));
    c4.setSuperClassName(CIMName ("YourClass"));
    */
    PEGASUS_TEST_ASSERT(c5.getSuperClassName() == CIMName ("YourClass"));

    PEGASUS_TEST_ASSERT(c5.getQualifierCount() == 0);
    posQualifier = c5.findQualifier(CIMName ("q2"));

    // throws out of bounds
    try
    {
        CIMConstQualifier qconst1 = c5.getQualifier(posQualifier);
    }
    catch(IndexOutOfBoundsException& e)
    {
        if(verbose)
            cout << "Exception: " << e.getMessage() << endl;
    }
    if(verbose)
    {
        cout << "All tests" << endl;
    }
}

int main(int, char** argv)
{
    verbose = getenv("PEGASUS_TEST_VERBOSE") ? true : false;

    try
    {
    test01();
    }
    catch (Exception& e)
    {
    cout << "Exception: " << e.getMessage() << endl;
    }

    cout << argv[0] << " +++++ passed all tests" << endl;

    return 0;
}
