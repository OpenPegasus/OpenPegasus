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

/*
    This test module tests the functions associated with CIMvalue.
*/

#include <Pegasus/Common/PegasusAssert.h>
#include <Pegasus/Common/CIMValue.h>
#include <Pegasus/Common/DeclContext.h>
#include <Pegasus/Common/Resolver.h>
#include <Pegasus/Common/XmlWriter.h>
#include <Pegasus/General/MofWriter.h>

PEGASUS_USING_PEGASUS;
PEGASUS_USING_STD;

/* The following define is used to control displays of output from the
    test.  Comment out the define to turn off the large quantity of prinout.
    Added the verbose variable which comes from an environment variable.
    IO occurs only when the environment variable is set.
*/
#define IO 1

static Boolean verbose;

/* This template provides a complete set of tests of simple CIMValues for
    the different possible data types.  It tests creating, assigning,
    the equals, creating XML and MOF, and the functions associated with
    clearing testing for the null attribute. It also tests that there
    is not value returned from get of CIMValue which is NULL
*/
template<class T>
void test01(const T& x)
{
    CIMValue v(x);
    CIMValue v2(v);
    CIMValue v3;
    // Create a null constructor
    CIMType type = v.getType();            // get the type of v
    CIMValue v4(type,false);               // isArray false, isNULL
    v3 = v2;
    CIMValue v5;
    v5 = v4;
#ifdef IO
    if (verbose)
    {
    cout << "\n----------------------\n";
    XmlWriter::printValueElement(v3, cout);
    }
#endif
    try
    {
        T t;
        T t1 = x;                           // create instances of value
        v3.get(t);                          // get valid value
        v4.get(t1);                         // attempt to get from NULL
        PEGASUS_TEST_ASSERT (!v.isNull());
        PEGASUS_TEST_ASSERT (!v.isArray());
        PEGASUS_TEST_ASSERT (!v2.isNull());
        PEGASUS_TEST_ASSERT(t == x);
        // confirm v4.get from NULL CIMValue did not change target variable
        PEGASUS_TEST_ASSERT(t1 == x);      // should be value set in t1 = x
        PEGASUS_TEST_ASSERT (v3.typeCompatible(v2));
        // Confirm that the constructor created Null, not array and
        // correct type
        PEGASUS_TEST_ASSERT (v4.isNull());
        PEGASUS_TEST_ASSERT (!v4.isArray());
        PEGASUS_TEST_ASSERT (v4.typeCompatible(v));
        PEGASUS_TEST_ASSERT (v5.isNull());
        PEGASUS_TEST_ASSERT (!v5.isArray());
        PEGASUS_TEST_ASSERT (v5.typeCompatible(v));

        // Test toMof
        Buffer mofout;
        mofout.clear();
        MofWriter::appendValueElement(mofout, v);

        // Test toXml
        Buffer out;
        XmlWriter::appendValueElement(out, v);
        XmlWriter::appendValueElement(out, v);

        // Test toString
        String valueString = v.toString();
#ifdef IO
        if (verbose)
        {
            cout << "MOF = [" << mofout.getData() << "]" << endl;
            cout << "toString Output [" << valueString << "]" << endl;
        }
#endif

    // There should be no exceptions to this point in the test.
    }
    catch(Exception& e)
    {
        cerr << "Error: " << e.getMessage() << endl;
        exit(1);
    }

    // From here forward, in the future we may have exceptions because
    // of the isNull which should generate an exception on the getdata.
    // Currently NO exceptions should be generated.
    try
    {
        // Test for isNull and the setNullValue function
        v.setNullValue(v.getType(), false);
        PEGASUS_TEST_ASSERT(v.isNull());

        // get the String and XML outputs for v
        String valueString2 = v.toString();
        Buffer xmlBuffer;
        XmlWriter::appendValueElement(xmlBuffer, v);

        Buffer mofOutput2;
        MofWriter::appendValueElement(mofOutput2, v);
#ifdef IO
        if (verbose)
        {
            cout << "MOF NULL = [" << mofOutput2.getData() << "]" << endl;
            cout << "toString NULL Output [" << valueString2 << "]" << endl;
            cout << " XML NULL = [" << xmlBuffer.getData() << "]" << endl;
        }
#endif
        v.clear();
        PEGASUS_TEST_ASSERT(v.isNull());
        v2.clear();
        PEGASUS_TEST_ASSERT(v2.isNull());

    }
    catch(Exception& e)
    {
        cerr << "Error: " << e.getMessage() << endl;
        PEGASUS_TEST_ASSERT(false);
    }

}
/* This template defines the set of tests used for arrays. Includes tests
   for isNULL not changing output variable.
*/

template<class T>
void test02(const Array<T>& x)
{
    CIMValue va(x);
    CIMValue va2(va);
    CIMValue va3;
    va3 = va2;
    // Create a null constructor
    CIMType type = va.getType();            // get the type of v
    CIMValue va4(type,true);                // Create CIMValue array isNULL
    CIMValue va5;
    va5 = va4;
#ifdef IO
    if (verbose)
    {
        cout << "\n----------------------\n";
        XmlWriter::printValueElement(va3, cout);
    }
#endif

    try
    {
        Array<T> t;
        Array<T> t1;
        PEGASUS_TEST_ASSERT(t1.size() == 0);
        va3.get(t);
        va4.get(t1);                     // get from NULL CIMValue
        PEGASUS_TEST_ASSERT(t == x);
        PEGASUS_TEST_ASSERT(t1.size() == 0); // array should still be empty
        PEGASUS_TEST_ASSERT (va3.typeCompatible(va2));
        PEGASUS_TEST_ASSERT (!va.isNull());
        PEGASUS_TEST_ASSERT (va.isArray());
        PEGASUS_TEST_ASSERT (!va2.isNull());
        PEGASUS_TEST_ASSERT (va2.isArray());
        PEGASUS_TEST_ASSERT (!va3.isNull());
        PEGASUS_TEST_ASSERT (va3.isArray());

        // Note that this test depends on what is built.  Everything has
        // 2 entries.
        PEGASUS_TEST_ASSERT (va.getArraySize() == 3);

        // Confirm that va4 (and va5) is Null, and array and zero length
        PEGASUS_TEST_ASSERT (va4.isNull());
        PEGASUS_TEST_ASSERT (va4.isArray());
        PEGASUS_TEST_ASSERT (va4.getArraySize() == 0);
        PEGASUS_TEST_ASSERT (va4.typeCompatible(va));
        PEGASUS_TEST_ASSERT (va5.isNull());
        PEGASUS_TEST_ASSERT (va5.isArray());
        PEGASUS_TEST_ASSERT (va5.getArraySize() == 0);
        PEGASUS_TEST_ASSERT (va5.typeCompatible(va));

        // Test toMof
        Buffer mofOutput;
        MofWriter::appendValueElement(mofOutput, va);

        // Test toXml
        Buffer out;
        XmlWriter::appendValueElement(out, va);
        XmlWriter::appendValueElement(out, va);
        // Test toString
        String valueString = va.toString();
#ifdef IO
        if (verbose)
        {
            cout << "MOF = [" << mofOutput.getData() << "]" << endl;
            cout << "toString Output [" << valueString << "]" << endl;
        }
#endif
        // There should be no exceptions to this point so the
        // catch simply terminates.
    }
    catch(Exception& e)
    {
        cerr << "Error: " << e.getMessage() << endl;
        PEGASUS_TEST_ASSERT(false);
    }

    // Test the Null Characteristics
    try
    {
        // Set the initial one to Null
        va.setNullValue(va.getType(), true, 0);
        PEGASUS_TEST_ASSERT(va.isNull());
        PEGASUS_TEST_ASSERT(va.isArray());
        PEGASUS_TEST_ASSERT(va.getArraySize() == 0);
        va.setNullValue(va.getType(), false);
        PEGASUS_TEST_ASSERT(va.isNull());
        PEGASUS_TEST_ASSERT(!va.isArray());

        // get the String and XML outputs for v
        String valueString2 = va.toString();
        Buffer xmlBuffer;
        XmlWriter::appendValueElement(xmlBuffer, va);

        Buffer mofOutput2;
        MofWriter::appendValueElement(mofOutput2, va);
#ifdef IO
        if (verbose)
        {
            cout << "MOF NULL = [" << mofOutput2.getData() << "]" << endl;
            cout << "toString NULL Output [" << valueString2 << "]" << endl;
            cout << " XML NULL = [" << xmlBuffer.getData() << "]" << endl;
        }
#endif
        va.clear();
        PEGASUS_TEST_ASSERT(va.isNull());
    }
    catch(Exception& e)
    {
        cerr << "Error: " << e.getMessage() << endl;
        PEGASUS_TEST_ASSERT(false);
    }

}

template<class T1, class T2, class T3>
void test03( Array<T1>& arrObj1, Array<T1>& arrObj2, T2 obj, T3 val1, T3 val2)
{
    PEGASUS_TEST_ASSERT( 10 == arrObj1.size() && arrObj1[5] == val1);
    PEGASUS_TEST_ASSERT( 1 == arrObj2.size() && arrObj2[0] == val1);
    *obj = val2;
    arrObj2.append(obj,1);
    PEGASUS_TEST_ASSERT( 2 == arrObj2.size() && arrObj2[1] == val2 );
    arrObj1.appendArray(arrObj2);
    PEGASUS_TEST_ASSERT( 12 == arrObj1.size()
            && arrObj1[10] == val1 && arrObj1[11] == val2);
    arrObj2.clear();
    PEGASUS_TEST_ASSERT( 0 == arrObj2.size() );
    PEGASUS_TEST_ASSERT( 16 == arrObj1.getCapacity()
            && 8 == arrObj2.getCapacity() );
    arrObj2.grow(10,val1);
    PEGASUS_TEST_ASSERT( 10 == arrObj2.size() && arrObj2[5] == val1);
    arrObj2.insert(10,val2);
    PEGASUS_TEST_ASSERT( 11 == arrObj2.size() && arrObj2[10] == val2);
    arrObj2.insert(10, obj, 1);
    PEGASUS_TEST_ASSERT( 12 == arrObj2.size() );
    arrObj2.prepend(val2);
    PEGASUS_TEST_ASSERT( 13 == arrObj2.size() && arrObj2[12] == val2);
    *obj = val1;
    arrObj2.prepend(obj,1);
    PEGASUS_TEST_ASSERT( 14 == arrObj2.size() && arrObj2[0] == val1);
    arrObj1.swap(arrObj2);
    PEGASUS_TEST_ASSERT( 14 == arrObj1.size() && 12 == arrObj2.size() &&
                        arrObj2[10] == val1 && arrObj1[12] == val2 );
    arrObj1.remove(1);
    PEGASUS_TEST_ASSERT( 13 == arrObj1.size() );
    arrObj1.remove(1,1);
    PEGASUS_TEST_ASSERT( 12 == arrObj1.size() );
}

template<class EmbeddedType>
void testEmbeddedValue(const CIMInstance & instance)
{
    CIMInstance instance1 = instance.clone();
    // Specific test to verify the cloning of CIMObjects/CIMInstances when set
    // and gotten from a CIMValue.
    CIMValue v1;
    // Create CIMValue v1 of type CIMTYPE_OBJECT/CIMTYPE_INSTANCE
    v1.set(EmbeddedType(instance1));
    // Change the "count" property of instance1, and then verify
    // that the CIMValue v1, that was set from instance1, is
    // not affected (ie. tests clone() on CIMValue::set() ).
    Uint32 propIx = instance1.findProperty(CIMName("count"));
    PEGASUS_TEST_ASSERT(propIx != PEG_NOT_FOUND);
    CIMValue v2 = instance1.getProperty(propIx).getValue();
    Uint32 propCount;
    v2.get(propCount);
    PEGASUS_TEST_ASSERT(propCount == 55);
    instance1.removeProperty(propIx);
    instance1.addProperty(CIMProperty(CIMName ("count"), Uint32(65))
        .addQualifier(CIMQualifier(CIMName ("counter"), true))
        .addQualifier(CIMQualifier(CIMName ("min"), String("0")))
        .addQualifier(CIMQualifier(CIMName ("max"), String("1"))));
    EmbeddedType object2;
    v1.get(object2);
    CIMInstance instance1a(object2);
    propIx = instance1a.findProperty(CIMName("count"));
    PEGASUS_TEST_ASSERT(propIx != PEG_NOT_FOUND);
    CIMValue v3 = instance1a.getProperty(propIx).getValue();
    v3.get(propCount);
    PEGASUS_TEST_ASSERT(propCount == 55);
    // Now change the "count" property of instance1a, which was obtained
    // from a get of CIMValue v1. Again, the underlying CIMValue should
    // not be affected (ie. tests clone() on CIMValue::get() ).
    instance1a.removeProperty(propIx);
    instance1a.addProperty(CIMProperty(CIMName ("count"), Uint32(65))
        .addQualifier(CIMQualifier(CIMName ("counter"), true))
        .addQualifier(CIMQualifier(CIMName ("min"), String("0")))
        .addQualifier(CIMQualifier(CIMName ("max"), String("1"))));
    EmbeddedType object3;
    v1.get(object3);
    CIMInstance instance1b(object3);
    propIx = instance1b.findProperty(CIMName("count"));
    PEGASUS_TEST_ASSERT(propIx != PEG_NOT_FOUND);
    CIMValue v4 = instance1b.getProperty(propIx).getValue();
    v4.get(propCount);
    PEGASUS_TEST_ASSERT(propCount == 55);

    // Specific test for setting value as a null CIMObject/CIMInstance
    // (see bug 3373).
    // Confirm that CIMValue() with an uninitialized CIMObject/CIMInstance will
    // throw exception.
    Boolean caught_exception = false;
    try
    {
        EmbeddedType obj = EmbeddedType();
        CIMValue y(obj);
    }
    catch(UninitializedObjectException&)
    {
        caught_exception = true;
    }
    PEGASUS_TEST_ASSERT (caught_exception == true);
    // Confirm that set() with an uninitialized CIMObject/CIMInstance will
    // throw exception.
    caught_exception = false;
    try
    {
        CIMValue y;
        y.set(EmbeddedType());
    }
    catch(UninitializedObjectException&)
    {
        caught_exception = true;
    }
    PEGASUS_TEST_ASSERT (caught_exception == true);
}

template<class EmbeddedType>
void testEmbeddedValueArray(const CIMInstance & startInstance,
                            const CIMNamespaceName & NAMESPACE,
                            SimpleDeclContext * context)
{
    CIMInstance instance1(startInstance.clone());
    // Test an array of CIMObjects that are CIMInstances
    CIMInstance instance2(CIMName ("MyClass"));
    instance2.addQualifier(CIMQualifier(CIMName ("classcounter"), true));
    instance2.addProperty(CIMProperty(CIMName ("message"), String("Adios")));
    Resolver::resolveInstance (instance2, context, NAMESPACE, true);

    CIMInstance instance3(CIMName ("MyClass"));
    instance3.addQualifier(CIMQualifier(CIMName ("classcounter"), false));
    instance3.addProperty(CIMProperty(CIMName ("message"),
                String("Au Revoir")));
    Resolver::resolveInstance (instance3, context, NAMESPACE, true);

    Array<EmbeddedType> arr16;
    arr16.append(EmbeddedType(instance1));
    arr16.append(EmbeddedType(instance2));
    arr16.append(EmbeddedType(instance3));
    test02(arr16);

    // Specific test to verify the cloning of CIMObjects when set as and
    // gotten from a CIMValue.
    CIMValue v1array;
    // Create CIMValue v1 of type CIMTYPE_OBJECT (ie. CIMObject)
    v1array.set(arr16);
    // Change the "count" property of arr16[1], and then verify
    // that the CIMValue v1array, that was set from arr16, is
    // not affected (ie. tests clone() on CIMValue::set() ).
    Uint32 propIx = arr16[1].findProperty(CIMName("count"));
    PEGASUS_TEST_ASSERT(propIx != PEG_NOT_FOUND);
    CIMValue v2 = arr16[1].getProperty(propIx).getValue();
    Uint32 propCount;
    v2.get(propCount);
    PEGASUS_TEST_ASSERT(propCount == 55);
    arr16[1].removeProperty(propIx);
    arr16[1].addProperty(CIMProperty(CIMName ("count"), Uint32(65))
        .addQualifier(CIMQualifier(CIMName ("counter"), true))
        .addQualifier(CIMQualifier(CIMName ("min"), String("0")))
        .addQualifier(CIMQualifier(CIMName ("max"), String("1"))));
    Array<EmbeddedType> object2array;
    v1array.get(object2array);
    CIMInstance instance2a(object2array[1]);
    propIx = instance2a.findProperty(CIMName("count"));
    PEGASUS_TEST_ASSERT(propIx != PEG_NOT_FOUND);
    CIMValue v3 = instance2a.getProperty(propIx).getValue();
    v3.get(propCount);
    PEGASUS_TEST_ASSERT(propCount == 55);
    // Now change the "count" property of instance2a, which was obtained
    // from a get of CIMValue v1array. Again, the underlying CIMValue should
    // not be affected (ie. tests clone() on CIMValue::get() ).
    instance2a.removeProperty(propIx);
    instance2a.addProperty(CIMProperty(CIMName ("count"), Uint32(65))
       .addQualifier(CIMQualifier(CIMName ("counter"), true))
       .addQualifier(CIMQualifier(CIMName ("min"), String("0")))
       .addQualifier(CIMQualifier(CIMName ("max"), String("1"))));
    Array<EmbeddedType> object3array;
    v1array.get(object3array);
    CIMInstance instance2b(object3array[1]);
    propIx = instance2b.findProperty(CIMName("count"));
    PEGASUS_TEST_ASSERT(propIx != PEG_NOT_FOUND);
    CIMValue v4 = instance2b.getProperty(propIx).getValue();
    v4.get(propCount);
    PEGASUS_TEST_ASSERT(propCount == 55);

    // Specific test for setting value as a null CIMObject() (see bug 3373).
    // Confirm that CIMValue() with an uninitialized CIMObject in the input
    // array will throw exception.
    arr16.append(EmbeddedType());
    bool caught_exception = false;
    try
    {
        CIMValue y(arr16);
    }
    catch(UninitializedObjectException&)
    {
        caught_exception = true;
    }
    PEGASUS_TEST_ASSERT (caught_exception == true);
    // Confirm that set() with an uninitialized CIMObject in the input
    // array will throw exception.
    caught_exception = false;
    try
    {
        CIMValue y;
        y.set(arr16);
    }
    catch(UninitializedObjectException&)
    {
        caught_exception = true;
    }
    PEGASUS_TEST_ASSERT (caught_exception == true);
}

int main(int, char** argv)
{
#ifdef IO
    verbose = getenv("PEGASUS_TEST_VERBOSE") ? true : false;
    if (verbose)
    cout << "Test CIMValue. To turn off display, compile with IO undefined\n";
#endif
    // Test the primitive CIMValue types with test01
    test01(Boolean(true));
    test01(Boolean(false));
    test01(Char16('Z'));
    test01(Uint8(77));
    test01(Sint8(-77));
    test01(Sint16(77));
    test01(Uint16(-77));
    test01(Sint32(77));
    test01(Uint32(-77));
    test01(Sint64(77));
    test01(Uint64(-77));
    test01(Real32(1.5));
    test01(Real64(55.5));
    test01(Uint64(123456789));
    test01(Sint64(-123456789));
    test01(String("Hello world"));
    test01(CIMDateTime("19991224120000.000000+360"));
    test01(CIMObjectPath(
        "//host1:77/root/test:Class1.key1=\"key1Value\",key2=\"key2Value\""));

    // Create and populate a declaration context:

    const CIMNamespaceName NAMESPACE = CIMNamespaceName ("/zzz");

    SimpleDeclContext* context = new SimpleDeclContext;

    context->addQualifierDecl(
    NAMESPACE, CIMQualifierDecl(CIMName ("counter"), false,
        CIMScope::PROPERTY));

    context->addQualifierDecl(
    NAMESPACE, CIMQualifierDecl(CIMName ("classcounter"), false,
        CIMScope::CLASS));

    context->addQualifierDecl(
    NAMESPACE, CIMQualifierDecl(CIMName ("min"), String(),
        CIMScope::PROPERTY));

    context->addQualifierDecl(
    NAMESPACE, CIMQualifierDecl(CIMName ("max"), String(),
        CIMScope::PROPERTY));

    context->addQualifierDecl(NAMESPACE,
    CIMQualifierDecl(CIMName ("Description"), String(),
        CIMScope::PROPERTY));

    CIMClass class1(CIMName ("MyClass"));

    class1
    .addProperty(CIMProperty(CIMName ("count"), Uint32(55))
        .addQualifier(CIMQualifier(CIMName ("counter"), true))
        .addQualifier(CIMQualifier(CIMName ("min"), String("0")))
        .addQualifier(CIMQualifier(CIMName ("max"), String("1"))))
    .addProperty(CIMProperty(CIMName ("message"), String("Hello"))
        .addQualifier(CIMQualifier(CIMName ("description"),
                String("My Message"))))
    .addProperty(CIMProperty(CIMName ("ratio"), Real32(1.5)));

    Resolver::resolveClass (class1, context, NAMESPACE);
    context->addClass(NAMESPACE, class1);

    // Test a CIMObject that is a CIMClass
    test01(CIMObject(class1));

    // Test a CIMObject that is a CIMInstance
    CIMInstance instance1(CIMName ("MyClass"));
    instance1.addQualifier(CIMQualifier(CIMName ("classcounter"), true));
    instance1.addProperty(CIMProperty(CIMName ("message"), String("Goodbye")));

    Resolver::resolveInstance (instance1, context, NAMESPACE, true);

    test01(CIMObject(instance1));

    testEmbeddedValue<CIMObject>(instance1);
    testEmbeddedValue<CIMInstance>(instance1);

    // Test CIMValue arrays

    Array<Uint8> arr1;
    arr1.append(11);
    arr1.append(22);
    arr1.append(23);
    test02(arr1);

    Array<Uint16> arr2;
    arr2.append(333);
    arr2.append(444);
    arr2.append(445);
    test02(arr2);

    Array<Uint32> arr3;
    arr3.append(5555);
    arr3.append(6666);
    arr3.append(6667);
    test02(arr3);

    Array<Uint64> arr4;
    arr4.append(123456789);
    arr4.append(987654321);
    arr4.append(987654322);
    test02(arr4);

    Array<Sint8> arr5;
    arr5.append(-11);
    arr5.append(-22);
    arr5.append(-23);
    test02(arr5);

    Array<Sint16> arr6;
    arr6.append(333);
    arr6.append(444);
    arr6.append(555);
    test02(arr6);

    Array<Sint32> arr7;
    arr7.append(555);
    arr7.append(666);
    arr7.append(777);
    test02(arr7);

    Array<Sint64> arr8;
    arr8.append(-123456789);
    arr8.append(-987654321);
    arr8.append(-987654321);
    test02(arr8);

    Array<Boolean> arr9;
    arr9.append(true);
    arr9.append(false);
    arr9.append(false);
    test02(arr9);

    Array<Real32> arr10;
    arr10.append(1.55F);
    arr10.append(2.66F);
    arr10.append(3.77F);
    test02(arr10);

    Array<Real64> arr11;
    arr11.append(55.55);
    arr11.append(66.66);
    arr11.append(77.77);
    test02(arr11);

    Array<Char16> arr12;
    arr12.append('X');
    arr12.append('Y');
    arr12.append('Z');
    test02(arr12);

    Array<String> arr13;
    arr13.append("One");
    arr13.append("Two");
    arr13.append("Three");
    test02(arr13);

    Array<CIMDateTime> arr14;
    arr14.append(CIMDateTime ("20020130120000.000000+360"));
    arr14.append(CIMDateTime ("20020201120000.000000+360"));
    arr14.append(CIMDateTime ("20020202120000.000000+360"));
    test02(arr14);

    Array<CIMObjectPath> arr15;
    arr15.append(CIMObjectPath(
        "//host1:77/root/test:Class1.key1=\"key1Value\",key2=\"key2Value\""));
    arr15.append(CIMObjectPath(
        "//host2:88/root/static:Class2.keyA=\"keyAValue\",keyB="
                "\"keyBValue\""));
    arr15.append(CIMObjectPath(
        "//host3:99/root/test/static:Class3.keyX=\"keyXValue\","
                "keyY=\"keyYValue\""));
    test02(arr15);

    testEmbeddedValueArray<CIMObject>(instance1, NAMESPACE, context);
    testEmbeddedValueArray<CIMInstance>(instance1, NAMESPACE, context);

    // Calling remaining  Array tests..
    CIMDateTime D1("19991224120000.000000+100");
    Array<CIMDateTime> arrD1(10,D1);
    CIMDateTime *D2 = new CIMDateTime("19991224120000.000000+100");
    Array<CIMDateTime> arrD2(D2,1);
    test03(arrD1, arrD2, D2, CIMDateTime("19991224120000.000000+100"),
                     CIMDateTime("29991224120000.000000+100"));
    delete D2;

    CIMName cimname1("yourName");
    Array<CIMName> arrcimname1(10,cimname1);
    CIMName *cimname2 = new CIMName("yourName");
    Array<CIMName> arrcimname2(cimname2,1);
    test03(arrcimname1, arrcimname2, cimname2, CIMName("yourName"),
            CIMName("myName"));
    delete cimname2;

    CIMKeyBinding cimbind1(cimname1, "myKey", CIMKeyBinding::STRING);
    CIMKeyBinding cimbind2(cimname1, "yourKey", CIMKeyBinding::STRING);
    Array<CIMKeyBinding> arrcimbind1(10,cimbind1);
    CIMKeyBinding *cimbind3 =
        new CIMKeyBinding(cimname1, "myKey", CIMKeyBinding::STRING);
    Array<CIMKeyBinding> arrcimbind2(cimbind3,1);
    test03(arrcimbind1, arrcimbind2, cimbind3, cimbind1, cimbind2 );
    delete cimbind3;

    CIMNamespaceName cimnamespace1("root/SampleProvider");
    Array<CIMNamespaceName> arrcimnamespace1(10,cimnamespace1);
    CIMNamespaceName *cimnamespace2 = new CIMNamespaceName(
            "root/SampleProvider");
    Array<CIMNamespaceName> arrcimnamespace2(cimnamespace2,1);
    test03(arrcimnamespace1, arrcimnamespace2, cimnamespace2,
            CIMNamespaceName("root/SampleProvider"),
            CIMNamespaceName("root/SampleProvider2"));
    delete cimnamespace2;

    Array<Boolean> arrB1(10,true);
    Boolean *b = new Boolean(true);
    Array<Boolean> arrB2(b,1);
    Array<Boolean> arrB3(2);
    test03(arrB1, arrB2, b, Boolean(true),Boolean(false));
    delete b;

    Array<Real32> arrreal321(10);
    Real32 creal321(2.5);
    Array<Real32> arrreal322(10, creal321);
    Real32 *creal322 = new Real32(2.5);
    Array<Real32> arrreal323(creal322,1);
    Array<Real32> arrreal324(arrreal321);
    test03(arrreal322, arrreal323, creal322,Real32(2.5),Real32(3.5));
    delete creal322;

    Array<Real64> arrreal641(10);
    Real64 creal641(20000.54321);
    Array<Real64> arrreal642(10, creal641);
    Real64 *creal642 = new Real64(20000.54321);
    Array<Real64> arrreal643(creal642,1);
    Array<Real64> arrreal644(arrreal641);
    test03(arrreal642, arrreal643, creal642,Real64(20000.54321),
            Real64(30000.54321));
    delete creal642;

    Array<Sint16> arrSint161(10);
    Sint16 cSint161(-2000);
    Array<Sint16> arrSint162(10, cSint161);
    Sint16 *cSint162 = new Sint16(-2000);
    Array<Sint16> arrSint163(cSint162,1);
    Array<Sint16> arrSint164(arrSint161);
    test03(arrSint162, arrSint163, cSint162, Sint16(-2000), Sint16(-3000));
    delete cSint162;

    Array<Sint32> arrSint321(10);
    Sint32 cSint321(-200000000);
    Array<Sint32> arrSint322(10, cSint321);
    Sint32 *cSint322 = new Sint32(-200000000);
    Array<Sint32> arrSint323(cSint322,1);
    Array<Sint32> arrSint324(arrSint321);
    test03(arrSint322, arrSint323, cSint322, Sint32(-200000000),
            Sint32(-300000000));
    delete cSint322;

    Array<Sint64> arrSint641(10);
    Sint64 cSint641(Sint64(-2000000)*Sint64(10000000) );
    Array<Sint64> arrSint642(10, cSint641);
    Sint64 *cSint642 = new Sint64(Sint64(-2000000)*Sint64(10000000));
    Array<Sint64> arrSint643(cSint642,1);
    Array<Sint64> arrSint644(arrSint641);
    test03(arrSint642, arrSint643, cSint642,Sint64(-2000000)*Sint64(10000000),
                           Sint64(-3000000)*Sint64(10000000));
    delete cSint642;

    Array<Sint8> arrSint81(10);
    Sint8 cSint81(-20);
    Array<Sint8> arrSint82(10, cSint81);
    Sint8 *cSint82 = new Sint8(-20);
    Array<Sint8> arrSint83(cSint82,1);
    Array<Sint8> arrSint84(arrSint81);
    test03(arrSint82, arrSint83, cSint82, Sint8(-20), Sint8(-22));
    delete cSint82;

    Array<Uint16> arrUint161(10);
    Uint16 cUint161(200);
    Array<Uint16> arrUint162(10, cUint161);
    Uint16 *cUint162 = new Uint16(200);
    Array<Uint16> arrUint163(cUint162,1);
    Array<Uint16> arrUint164(arrUint161);
    test03(arrUint162, arrUint163, cUint162, Uint16(200), Uint16(255));
    delete cUint162;

    Array<Uint32> arrUint321(10);
    Uint32 cUint321(2000);
    Array<Uint32> arrUint322(10, cUint321);
    Uint32 *cUint322 = new Uint32(2000);
    Array<Uint32> arrUint323(cUint322,1);
    Array<Uint32> arrUint324(arrUint321);
    test03(arrUint322, arrUint323, cUint322, Uint32(2000), Uint32(3000));
    delete cUint322;

    Array<Uint64> arrUint641(10);
    Uint64 cUint641(Uint64(2000000)*Uint64(10000000));
    Array<Uint64> arrUint642(10, cUint641);
    Uint64 *cUint642 = new Uint64(Uint64(2000000)*Uint64(10000000));
    Array<Uint64> arrUint643(cUint642,1);
    Array<Uint64> arrUint644(arrUint641);
    test03(arrUint642, arrUint643, cUint642,Uint64(2000000)*Uint64(10000000),
                           Uint64(255000)*Uint64(10000000));
    delete cUint642;

    Array<Uint8> arrUint81(10);
    Uint8 cUint81(200);
    Array<Uint8> arrUint82(10, cUint81);
    Uint8 *cUint82 = new Uint8(200);
    Array<Uint8> arrUint83(cUint82,1);
    Array<Uint8> arrUint84(arrUint81);
    test03(arrUint82, arrUint83, cUint82, Uint8(200), Uint8(255));
    delete cUint82;

    Array<Char16> arrChar161(10);
    Char16 cChar161('Z');
    Array<Char16> arrChar162(10, cChar161);
    Char16 *cChar162 = new Char16('Z');
    Array<Char16> arrChar163(cChar162,1);
    Array<Char16> arrChar164(arrChar161);
    test03(arrChar162, arrChar163, cChar162, Char16('Z'), Char16('z'));
    delete cChar162;
    delete context;

    cout << argv[0] << " +++++ passed all tests" << endl;

    return 0;
}

