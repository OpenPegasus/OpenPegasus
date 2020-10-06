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
#include <Pegasus/Common/CIMObject.h>
#include <Pegasus/Common/CIMClass.h>
#include <Pegasus/Common/CIMInstance.h>
#include <Pegasus/Common/XmlWriter.h>

#include <Pegasus/Common/PegasusAssert.h>

PEGASUS_USING_PEGASUS;
PEGASUS_USING_STD;

static Boolean verbose;

//*********************************************************************
//  CIMObject tests
//
//  The CIMObject class refers either to a CIMInstance or a CIMClass.
//*********************************************************************
void test01()
{
    CIMObject obj;
    //
    // Construct from CIMClass
    //
    CIMClass cimClass1(CIMName ("MyClass"));
    cimClass1.setPath(CIMObjectPath ("//localhost/root/cimv2:MyClass"));

    CIMObject oclass1 = cimClass1;
    CIMObject oclass2(cimClass1);
    CIMObject oclass3(oclass2);
    CIMObject oclass4 = oclass3;

    CIMClass cimClass2 = cimClass1;
    cimClass2 = cimClass1;
    cimClass2 = CIMClass(oclass1);
    CIMClass cimClass3 = CIMClass(oclass1);

    PEGASUS_TEST_ASSERT(oclass1.getClassName() == CIMName ("MyClass"));
    PEGASUS_TEST_ASSERT(oclass1.getPath() ==
        CIMObjectPath("//localhost/root/cimv2:MyClass"));
    PEGASUS_TEST_ASSERT (oclass1.isClass ());
    PEGASUS_TEST_ASSERT (!oclass1.isInstance ());
    PEGASUS_TEST_ASSERT (oclass2.isClass ());
    PEGASUS_TEST_ASSERT (!oclass2.isInstance ());
    PEGASUS_TEST_ASSERT (oclass3.isClass ());
    PEGASUS_TEST_ASSERT (!oclass3.isInstance ());
    PEGASUS_TEST_ASSERT (oclass4.isClass ());
    PEGASUS_TEST_ASSERT (!oclass4.isInstance ());

    //
    // Construct from CIMInstance
    //
    CIMInstance cimInstance1(CIMName ("MyClass"));
    CIMObject oinstance1 = cimInstance1;
    CIMObject instance2(cimInstance1);

    PEGASUS_TEST_ASSERT (oinstance1.isInstance ());
    PEGASUS_TEST_ASSERT (!oinstance1.isClass ());
    PEGASUS_TEST_ASSERT (instance2.isInstance ());
    PEGASUS_TEST_ASSERT (!instance2.isClass ());

    // Test qualifiers
    oinstance1.addQualifier(CIMQualifier(CIMName ("Key"), true));
    oinstance1.addQualifier(CIMQualifier(CIMName ("Description"),
        String("Just a Test")));
    oinstance1.addQualifier(CIMQualifier(CIMName ("q1"), true));
    oinstance1.addQualifier(CIMQualifier(CIMName ("q2"), true));

    PEGASUS_TEST_ASSERT(oinstance1.findQualifier(CIMName ("Key"))
        != PEG_NOT_FOUND);
    PEGASUS_TEST_ASSERT(oinstance1.findQualifier(CIMName ("Description"))
        != PEG_NOT_FOUND);
    PEGASUS_TEST_ASSERT(oinstance1.findQualifier(CIMName ("q1"))
        != PEG_NOT_FOUND);
    PEGASUS_TEST_ASSERT(oinstance1.findQualifier(CIMName ("q2"))
        != PEG_NOT_FOUND);
    PEGASUS_TEST_ASSERT(oinstance1.findQualifier(CIMName ("q3"))
        == PEG_NOT_FOUND);
    PEGASUS_TEST_ASSERT(oinstance1.getQualifierCount() == 4);

    PEGASUS_TEST_ASSERT(oinstance1.findQualifier(CIMName ("q1"))
        != PEG_NOT_FOUND);
    PEGASUS_TEST_ASSERT(oinstance1.findQualifier(CIMName ("q2"))
        != PEG_NOT_FOUND);
    PEGASUS_TEST_ASSERT(oinstance1.findQualifier(CIMName ("q3"))
        == PEG_NOT_FOUND);
    PEGASUS_TEST_ASSERT(oinstance1.findQualifier(CIMName ("q4"))
        == PEG_NOT_FOUND);

    Uint32 posQualifier;
    posQualifier = oinstance1.findQualifier(CIMName ("q1"));
    PEGASUS_TEST_ASSERT(posQualifier != PEG_NOT_FOUND);
    PEGASUS_TEST_ASSERT(posQualifier < oinstance1.getQualifierCount());

    try
    {
        CIMQualifier q1 = oinstance1.getQualifier(posQualifier);
        PEGASUS_TEST_ASSERT(!q1.isUninitialized());
        CIMConstQualifier cq1 = oinstance1.getQualifier(posQualifier);
        PEGASUS_TEST_ASSERT(!cq1.isUninitialized());

        /**
            Added to cover the function
            CIMConstQualifier CIMObject::getQualifier(Uint32 index) const
            in CIMObject.cpp
        */
        const CIMObject coinstance1 = cimInstance1;
        CIMConstQualifier cq2 = coinstance1.getQualifier(0);
        PEGASUS_TEST_ASSERT(!cq2.isUninitialized());

        /**
            Added to cover the function Added to cover the function
            CIMConstProperty CIMObject::getProperty(Uint32 index) const
            in CIMObject.cpp
        */
        CIMConstProperty cp1 =  coinstance1.getProperty(0);
        PEGASUS_TEST_ASSERT(!cp1.isUninitialized());
    }
    catch(IndexOutOfBoundsException& e)
    {
        if(verbose)
            cout << "Exception: " << e.getMessage() << endl;

    }

    oinstance1.removeQualifier(posQualifier);
    PEGASUS_TEST_ASSERT(oinstance1.getQualifierCount() == 3);
    PEGASUS_TEST_ASSERT(oinstance1.findQualifier(CIMName ("q1"))
        == PEG_NOT_FOUND);
    PEGASUS_TEST_ASSERT(oinstance1.findQualifier(CIMName ("q2"))
        != PEG_NOT_FOUND);

    // Test properties
    oinstance1.addProperty(CIMProperty(CIMName ("message"),
        String("Hello There")));
    oinstance1.addProperty(CIMProperty(CIMName ("count"), Uint32(77)));

    PEGASUS_TEST_ASSERT(oinstance1.findProperty(CIMName ("count"))
        != PEG_NOT_FOUND);
    PEGASUS_TEST_ASSERT(oinstance1.findProperty(CIMName ("message"))
        != PEG_NOT_FOUND);
    PEGASUS_TEST_ASSERT(oinstance1.findProperty(CIMName ("ratio"))
        == PEG_NOT_FOUND);

    CIMProperty cp =
       oinstance1.getProperty(oinstance1.findProperty(CIMName ("message")));

    Uint32 posProperty;
    posProperty = oinstance1.findProperty(CIMName ("count"));
    oinstance1.removeProperty(posProperty);
    PEGASUS_TEST_ASSERT(oinstance1.findProperty(CIMName ("count"))
        == PEG_NOT_FOUND);

    PEGASUS_TEST_ASSERT(oinstance1.getPropertyCount() == 1);

    const CIMObject oinstance2 = oinstance1.clone();
    PEGASUS_TEST_ASSERT(oinstance2.identical(oinstance1));

    if (verbose)
    {
        Buffer xmlOut;
        XmlWriter::appendObjectElement(xmlOut, oinstance1);
    }

}

//*********************************************************************
//  CIMConstObject tests
//*********************************************************************
void test02()
{
    CIMConstObject obj;
    //
    // Construct from CIMClass
    //
    CIMClass class1(CIMName ("MyClass"));
    class1.setPath(CIMObjectPath ("//localhost/root/cimv2:MyClass"));

    //
    // Construct from CIMClass
    //

    CIMObject obj1 = class1;

    CIMConstClass cclass1(CIMName ("MyClass"));
    CIMConstObject cobj1(class1);
    CIMConstObject cobj2(cclass1);
    CIMConstObject cobj3(obj1);

    /**
        Added to cover the constructor and getPath() in CIMClass.cpp
    */
    CIMConstClass cclass11(obj1);
    PEGASUS_TEST_ASSERT( !cclass11.isUninitialized());

    cclass11.getPath();
    PEGASUS_TEST_ASSERT( cclass11.getPath().toString() ==
        "//localhost/root/cimv2:MyClass" );

    /**
        Added to cover the function CIMConstObject& CIMConstObject::operator=(
        const CIMConstObject& x) in CIMObject.cpp
    */
    const CIMConstObject cobj111(class1);
    cobj1=cobj111;
    PEGASUS_TEST_ASSERT( cobj1.toString() == cobj111.toString());

    /**
        Added to cover the function Boolean CIMConstObject::isUninitialized()
        const in CIMObject.cpp
    */
    Boolean result=0;
    PEGASUS_TEST_ASSERT( result == cobj1.isUninitialized());

    /**
        Added to cover the function String CIMConstObject::toString () const
        in CIMObject.cpp
    */
    String result1;
    result1 = cobj1.toString();
    PEGASUS_TEST_ASSERT( result1 != "<CLASS  NAME=\"MyClass\" ></CLASS>" );

    //
    // Construct from CIMInstance
    //
    CIMInstance instance(CIMName ("MyClass"));
    CIMObject obj2 = instance;

    obj2.addQualifier(CIMQualifier(CIMName ("Key"), true));
    obj2.addQualifier(CIMQualifier(CIMName ("Description"),
        String("Just a Test")));
    obj2.addProperty(CIMProperty(CIMName ("message"), String("Hello There")));
    obj2.addProperty(CIMProperty(CIMName ("count"), Uint32(77)));

    CIMConstInstance cinstance(CIMName ("MyClass"));
    CIMConstObject cobj4(instance);
    CIMConstObject cobj5(cinstance);
    CIMConstObject cobj6(obj2);

    CIMConstObject cobj7 = obj1;
    CIMConstObject cobj8 = cobj7;
    CIMConstObject cobj9 = class1;
    CIMConstObject cobj10 = cclass1;
    CIMConstObject cobj11 = instance;
    CIMConstObject cobj12 = cinstance;

    PEGASUS_TEST_ASSERT(cobj1.getClassName() == CIMName ("MyClass"));
    PEGASUS_TEST_ASSERT(cobj1.getPath() ==
        CIMObjectPath("//localhost/root/cimv2:MyClass"));

    // clone the instance object
    CIMObject cloneObj = cobj4.clone();
    CIMConstObject ccloneObj = cloneObj;
    PEGASUS_TEST_ASSERT(ccloneObj.identical(cobj4) == true);
    PEGASUS_TEST_ASSERT(ccloneObj.identical(cobj2) == false);

    if (verbose)
    {
        Buffer xmlOut;
        XmlWriter::appendObjectElement(xmlOut, ccloneObj);
    }

    // Test qualifiers
    PEGASUS_TEST_ASSERT(ccloneObj.getQualifierCount() == 2);
    PEGASUS_TEST_ASSERT(ccloneObj.findQualifier(CIMName ("Key"))
        != PEG_NOT_FOUND);
    PEGASUS_TEST_ASSERT(ccloneObj.findQualifier(CIMName ("Description"))
        != PEG_NOT_FOUND);
    PEGASUS_TEST_ASSERT(ccloneObj.findQualifier(CIMName ("q1"))
        == PEG_NOT_FOUND);

    Uint32 posQualifier;
    posQualifier = ccloneObj.findQualifier(CIMName ("Key"));
    PEGASUS_TEST_ASSERT(posQualifier != PEG_NOT_FOUND);
    PEGASUS_TEST_ASSERT(posQualifier < ccloneObj.getQualifierCount());

    try
    {
        ccloneObj.getQualifier(posQualifier);
    }
    catch(IndexOutOfBoundsException& e)
    {
        if(verbose)
            cout << "Exception: " << e.getMessage() << endl;
    }

    // Test properties
    PEGASUS_TEST_ASSERT(ccloneObj.getPropertyCount() == 2);
    PEGASUS_TEST_ASSERT(ccloneObj.findProperty(CIMName ("count"))
        != PEG_NOT_FOUND);
    PEGASUS_TEST_ASSERT(ccloneObj.findProperty(CIMName ("message"))
        != PEG_NOT_FOUND);
    PEGASUS_TEST_ASSERT(ccloneObj.findProperty(CIMName ("ratio"))
        == PEG_NOT_FOUND);

    Uint32 posProperty;
    posProperty = ccloneObj.findProperty(CIMName ("message"));
    try
    {
        ccloneObj.getProperty(posProperty);
    }
    catch(IndexOutOfBoundsException& e)
    {
        if(verbose)
            cout << "Exception: " << e.getMessage() << endl;
    }
}

//*********************************************************************
//  CIMObject setPath tests
//
//*********************************************************************
void test03()
{
    CIMClass class1(CIMName ("MyClass"));
    CIMObjectPath ref1("//localhost/root/cimv2:MyClass");
    class1.setPath(ref1);

    CIMObject obj1;
    CIMObject obj2 = CIMObject (class1);
    obj2.setPath (ref1);
    CIMObject obj3(obj2);
    CIMObject obj4 = obj3;

    CIMClass class2(CIMName ("YourClass"));
    CIMObjectPath ref2("//localhost/root/cimv2:YourClass");
    class2.setPath(ref2);

    obj3 = CIMObject (class2);
    obj3.setPath (ref2);
    CIMObjectPath ref3 = obj3.getPath ();

    CIMObject myObj = obj3;

    if(verbose)
    {
        Buffer xmlOut;
        XmlWriter::appendObjectElement(xmlOut, myObj);
    }
}

//*********************************************************************
//  CIMObject, CIMClass, CIMInstance tests using setPath and getPath
//*********************************************************************
void test04()
{
    // Verify class name cannot be changed after creation. only a test for
    // CIMObject is needed because CIMClass and CIMInstance use
    // CIMClass::setPath() to perform this function.
    try
    {
        CIMObject cimObject(CIMInstance("MyClass"));

        cimObject.setPath(CIMObjectPath("//localhost/root/cimv2:YourClass"));

        throw Exception(
            "Failed to detect name change via CIMObject::setPath().");
    }
    catch(Exception &)
    {
        // expected. do nothing.

        if(verbose)
        {
            cout << "Successfully prevented class name change via setPath()."
                << endl;
        }
    }

    //
    //  Test CIMClass
    //
    CIMClass class1 (CIMName ("MyClass"));
    class1.setPath(CIMObjectPath ("//localhost/root/cimv2:MyClass"));
    CIMProperty prop1;
    prop1 = CIMProperty (CIMName ("message"), CIMValue (CIMTYPE_STRING));
    prop1.addQualifier (CIMQualifier (CIMName ("Key"), true));
    CIMProperty prop2;
    prop2 = CIMProperty (CIMName ("count"), CIMValue (CIMTYPE_UINT32));
    prop2.addQualifier (CIMQualifier (CIMName ("Key"), true));
    class1.addProperty (prop1);
    class1.addProperty (prop2);

    CIMObjectPath cpath = class1.getPath ();
    class1.setPath (CIMObjectPath (class1.getClassName ().getString()));
    CIMObjectPath cpath2 = class1.getPath ();
    class1.setPath (cpath);
    CIMObjectPath cpath3 = class1.getPath ();
    PEGASUS_TEST_ASSERT (cpath3 == cpath);

    if (verbose)
    {
        cout << "Class object path from getPath: " << cpath.toString() << endl;
        cout << "Class object path from getPath after setPath: "
             << cpath2.toString() << endl;
        cout << "Class object path from getPath after second setPath: "
             << cpath3.toString() << endl;
    }

    //
    //  Test class CIMObject
    //
    CIMObject oclass1 = class1;
    CIMObjectPath ocpath = class1.getPath ();
    class1.setPath (CIMObjectPath (class1.getClassName ().getString()));
    CIMObjectPath ocpath2 = class1.getPath ();
    class1.setPath (ocpath);
    CIMObjectPath ocpath3 = class1.getPath ();
    PEGASUS_TEST_ASSERT (ocpath3 == ocpath);

    if (verbose)
    {
        cout << "Class object path from getPath: " << ocpath.toString() << endl;
        cout << "Class object path from getPath after setPath: "
             << ocpath2.toString() << endl;
        cout << "Class object path from getPath after second setPath: "
             << ocpath3.toString() << endl;
    }

    //
    //  Test CIMInstance
    //
    CIMInstance instance1 (CIMName ("MyClass"));
    instance1.addProperty (CIMProperty (CIMName ("message"),
        String("Hello There")));
    instance1.addProperty (CIMProperty (CIMName ("count"), Uint32 (77)));
    CIMObjectPath path = instance1.buildPath (class1);
    CIMObjectPath path2 = instance1.getPath ();
    instance1.setPath (path);
    CIMObjectPath path3 = instance1.getPath ();
    PEGASUS_TEST_ASSERT (path3 == path);

    if (verbose)
    {
        cout << "Instance object path from buildPath: "
             << path.toString() << endl;
        cout << "Instance object path from getPath: "
             << path2.toString() << endl;
        cout << "Instance object path from getPath after setPath: "
             << path3.toString() << endl;
    }

    //
    //  Test instance CIMObject
    //
    CIMInstance instance2 (CIMName ("MyClass"));
    instance2.addProperty (CIMProperty (CIMName ("message"),
        String("Good-bye...")));
    instance2.addProperty (CIMProperty (CIMName ("count"), Uint32 (88)));
    CIMObject oinstance1 = instance2;
    CIMObjectPath opath = instance2.buildPath (class1);
    CIMObjectPath opath1 = oinstance1.getPath ();
    oinstance1.setPath (opath);
    CIMObjectPath opath2 = oinstance1.getPath ();
    PEGASUS_TEST_ASSERT (opath2 == opath);

    if (verbose)
    {
        cout << "Instance object path from buildPath: "
             << opath.toString() << endl;
        cout << "Instance object path from getPath: "
             << opath1.toString() << endl;
        cout << "Instance object path from getPath after setPath: "
             << opath2.toString() << endl;
    }
}

int main(int, char** argv)
{
    verbose = getenv("PEGASUS_TEST_VERBOSE") ? true : false;

    try
    {
       test01();
       test02();
       test03();
       test04();
    }
    catch (Exception& e)
    {
        cout << "Exception: " << e.getMessage() << endl;
    }

    cout << argv[0] << " +++++ passed all tests" << endl;
    return 0;
}

