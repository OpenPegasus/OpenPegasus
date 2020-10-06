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

#include <Pegasus/Common/PegasusAssert.h>
#include <Pegasus/Common/CIMProperty.h>
#include <Pegasus/Common/CIMPropertyList.h>
#include <Pegasus/Common/CIMPropertyInternal.h>
#include <Pegasus/Common/XmlWriter.h>
#include <Pegasus/Common/CIMObjectPath.h>

#include <Pegasus/General/MofWriter.h>

PEGASUS_USING_PEGASUS;
PEGASUS_USING_STD;
static Boolean verbose;         // controls test IO

#define VCOUT if (verbose) cout

void test01()
{
    CIMProperty pnull;

    PEGASUS_TEST_ASSERT(pnull.isUninitialized());

    CIMProperty p1(CIMName ("message"), String("Hi There"));
    p1.addQualifier(CIMQualifier(CIMName ("Key"), true));
    p1.addQualifier(CIMQualifier(CIMName ("stuff"), true));
    p1.addQualifier(CIMQualifier(CIMName ("stuff2"), true));
    p1.addQualifier(CIMQualifier(CIMName ("Description"), String("Blah Blah")));
    CIMConstProperty p2 = p1;

    // Test clone
    CIMProperty p1clone = p1.clone();
    CIMProperty p2clone = p2.clone();

    // Test print

    if(verbose)
    {
       XmlWriter::printPropertyElement(p1, cout);
       XmlWriter::printPropertyElement(p2, cout);
       XmlWriter::printPropertyElement(p1clone, cout);
       XmlWriter::printPropertyElement(p2clone, cout);
    }

    // Test toMof
       Buffer mofOut;
       MofWriter::appendPropertyElement(true, mofOut, p1);
       MofWriter::appendPropertyElement(true, mofOut, p2);

    // Test toXml
       Buffer xmlOut;
       XmlWriter::appendPropertyElement(xmlOut, p1);
       XmlWriter::appendPropertyElement(xmlOut, p2);

    // Test name
        CIMName name;
        name = p1.getName();
        PEGASUS_TEST_ASSERT(name == CIMName ("message"));
        name = p2.getName();
        PEGASUS_TEST_ASSERT(name == CIMName ("message"));

    // Test type
        PEGASUS_TEST_ASSERT(p1.getType() == CIMTYPE_STRING);
        PEGASUS_TEST_ASSERT(p2.getType() == CIMTYPE_STRING);

    // Test for key qualifier
        Uint32 pos;
        Boolean isKey = false;
        if ((pos = p1.findQualifier (CIMName ("key"))) != PEG_NOT_FOUND)
        {
            CIMValue value;
            value = p1.getQualifier (pos).getValue ();
            if (!value.isNull ())
            {
                value.get (isKey);
            }
        }
        PEGASUS_TEST_ASSERT (isKey);
        isKey = false;
        if ((pos = p2.findQualifier (CIMName ("key"))) != PEG_NOT_FOUND)
        {
            CIMValue value;
            value = p2.getQualifier (pos).getValue ();
            if (!value.isNull ())
            {
                value.get (isKey);
            }
        }
        PEGASUS_TEST_ASSERT (isKey);
    // Test for key property using CIMPropertyInternal
        PEGASUS_TEST_ASSERT (CIMPropertyInternal::isKeyProperty(p1));
        PEGASUS_TEST_ASSERT (CIMPropertyInternal::isKeyProperty(p2));

    // Test getArraySize
        PEGASUS_TEST_ASSERT(p1.getArraySize() == 0);
        PEGASUS_TEST_ASSERT(p2.getArraySize() == 0);

    // Test getPropagated
        PEGASUS_TEST_ASSERT(p1.getPropagated() == false);
        PEGASUS_TEST_ASSERT(p2.getPropagated() == false);

    // Tests for Qualifiers
    PEGASUS_TEST_ASSERT(p1.findQualifier(CIMName ("stuff")) != PEG_NOT_FOUND);
    PEGASUS_TEST_ASSERT(p1.findQualifier(CIMName ("stuff2")) != PEG_NOT_FOUND);
    PEGASUS_TEST_ASSERT(p1.findQualifier(CIMName ("stuff21")) == PEG_NOT_FOUND);
    PEGASUS_TEST_ASSERT(p1.findQualifier(CIMName ("stuf")) == PEG_NOT_FOUND);
    PEGASUS_TEST_ASSERT(p1.getQualifierCount() == 4);

    PEGASUS_TEST_ASSERT(p2.findQualifier(CIMName ("stuff")) != PEG_NOT_FOUND);
    PEGASUS_TEST_ASSERT(p2.findQualifier(CIMName ("stuff2")) != PEG_NOT_FOUND);
    PEGASUS_TEST_ASSERT(p2.findQualifier(CIMName ("stuff21")) == PEG_NOT_FOUND);
    PEGASUS_TEST_ASSERT(p2.findQualifier(CIMName ("stuf")) == PEG_NOT_FOUND);
    PEGASUS_TEST_ASSERT(p2.getQualifierCount() == 4);

    PEGASUS_TEST_ASSERT(p1.findQualifier(CIMName ("stuff")) != PEG_NOT_FOUND);
    PEGASUS_TEST_ASSERT(p1.findQualifier(CIMName ("stuff2")) != PEG_NOT_FOUND);

    PEGASUS_TEST_ASSERT(p1.findQualifier(CIMName ("stuff21")) == PEG_NOT_FOUND);
    PEGASUS_TEST_ASSERT(p1.findQualifier(CIMName ("stuf")) == PEG_NOT_FOUND);

    Uint32 posQualifier;
    posQualifier = p1.findQualifier(CIMName ("stuff"));
    PEGASUS_TEST_ASSERT(posQualifier != PEG_NOT_FOUND);
    PEGASUS_TEST_ASSERT(posQualifier < p1.getQualifierCount());

    p1.removeQualifier(posQualifier);
    PEGASUS_TEST_ASSERT(p1.getQualifierCount() == 3);
    PEGASUS_TEST_ASSERT(p1.findQualifier(CIMName ("stuff")) == PEG_NOT_FOUND);
    PEGASUS_TEST_ASSERT(p1.findQualifier(CIMName ("stuff2")) != PEG_NOT_FOUND);

    // Tests for value insertion.
    {
        CIMProperty px(CIMName ("p1"), String("Hi There"));
        // test for CIMValue and type
        CIMProperty py(CIMName ("p2"), Uint32(999));
        // test for CIMValue and type

        //Test getName and setName
        PEGASUS_TEST_ASSERT(px.getName() == CIMName ("p1"));
        px.setName(CIMName ("px"));
        PEGASUS_TEST_ASSERT(px.getName() == CIMName ("px"));

        PEGASUS_TEST_ASSERT(py.getName() == CIMName ("p2"));
        py.setName(CIMName ("py"));
        PEGASUS_TEST_ASSERT(py.getName() == CIMName ("py"));

        // ATTN: Test setValue and getValue
    }
}

void test02()
{
    // Tests for CIMConstProperty methods
        CIMProperty p1(CIMName ("message"), String("Hi There"));
        p1.addQualifier(CIMQualifier(CIMName ("Key"), true));
        p1.addQualifier(CIMQualifier(CIMName ("stuff"), true));
        p1.addQualifier(CIMQualifier(CIMName ("stuff2"), true));
        p1.addQualifier(CIMQualifier(CIMName ("Description"),
                                     String("Blah Blah")));
        CIMConstProperty p2 = p1;

        CIMConstProperty cp1 = p1;
        CIMConstProperty cp2 = p2;
        CIMConstProperty cp3(CIMName ("message3"), String("hello"));
        CIMConstProperty cp1clone = cp1.clone();

        if(verbose)
        XmlWriter::printPropertyElement(cp1, cout);

        Buffer mofOut;
        MofWriter::appendPropertyElement(true,mofOut, cp1);
        Buffer xmlOut;
        XmlWriter::appendPropertyElement(xmlOut, cp1);

        PEGASUS_TEST_ASSERT(cp1.getName() == CIMName ("message"));
        PEGASUS_TEST_ASSERT(cp1.getType() == CIMTYPE_STRING);
        Uint32 pos;
        Boolean isKey = false;
        if ((pos = cp1.findQualifier (CIMName ("key"))) != PEG_NOT_FOUND)
        {
            CIMValue value;
            value = cp1.getQualifier (pos).getValue ();
            if (!value.isNull ())
            {
                value.get (isKey);
            }
        }
        PEGASUS_TEST_ASSERT (isKey);
        PEGASUS_TEST_ASSERT(cp1.getArraySize() == 0);
        PEGASUS_TEST_ASSERT(cp1.getPropagated() == false);

    PEGASUS_TEST_ASSERT(cp1.findQualifier(
        CIMName ("stuff")) != PEG_NOT_FOUND);
    PEGASUS_TEST_ASSERT(cp1.findQualifier(
        CIMName ("stuff2")) != PEG_NOT_FOUND);
    PEGASUS_TEST_ASSERT(cp1.findQualifier(
        CIMName ("stuff21")) == PEG_NOT_FOUND);
    PEGASUS_TEST_ASSERT(cp1.findQualifier(
        CIMName ("stuf")) == PEG_NOT_FOUND);
    PEGASUS_TEST_ASSERT(cp1.getQualifierCount() == 4);

        try
        {
            p1.getQualifier(0);
        }
        catch(IndexOutOfBoundsException& e)
        {
            if(verbose)
                cout << "Exception: " << e.getMessage() << endl;
        }
}
//
// Test propertyList class
//
void test03()
{
    CIMPropertyList list1;
    CIMPropertyList list2;

    Array<CIMName> names;
    names.append(CIMName ("property1"));
    names.append(CIMName ("property2"));
    names.append(CIMName ("property3"));
    list1.set(names);
    list2 = list1;

    VCOUT << list1.toString() << endl;
    VCOUT << list2.toString() << endl;
    PEGASUS_TEST_ASSERT(list1.toString() == "property1, property2, property3");
    PEGASUS_TEST_ASSERT(list2.toString() == "property1, property2, property3");

    Array<CIMName> names1a = list1.getPropertyNameArray();
    PEGASUS_TEST_ASSERT(names == names1a);
    PEGASUS_TEST_ASSERT(list2[0] == CIMName("property1"));
    PEGASUS_TEST_ASSERT(list2[1] == CIMName("property2"));
    PEGASUS_TEST_ASSERT(list2[2] == CIMName("property3"));

    list1.clear();
    list2.clear();

    VCOUT << list1.toString() << endl;
    VCOUT << list2.toString() << endl;
    PEGASUS_TEST_ASSERT(list1.toString() == "NULL");
    PEGASUS_TEST_ASSERT(list2.toString() == "NULL");

    // Test use of empty list.  Note that the requirement for
    // property lists assumes that we must be able to distinguish
    // a NULL property list from an empty property list.  The usages
    // are very different.  NULL means ignore.  Empty means use list
    // but no properties in list.
    Array<CIMName> names2;
    list1.set(names2);
    PEGASUS_TEST_ASSERT(!list1.isNull());
    PEGASUS_TEST_ASSERT(list1.size() == 0);
    Array<CIMName> names3 = list1.getPropertyNameArray();
    PEGASUS_TEST_ASSERT(names3.size() == 0);
    VCOUT << list1.toString() << endl;
    PEGASUS_TEST_ASSERT(list1.toString() == "EMPTY");

    // test contains(...) function, Pegasus 2.14
    list1.clear();
    names.append(CIMName ("property1"));
    names.append(CIMName ("property2"));
    names.append(CIMName ("property3"));
    list1.set(names);
    PEGASUS_TEST_ASSERT(list1.contains("property1"));
    PEGASUS_TEST_ASSERT(list1.contains("property2"));
    PEGASUS_TEST_ASSERT(list1.contains("property3"));
    PEGASUS_TEST_ASSERT(!list1.contains("propertyx"));

    // retest with list cleared to be sure that an
    // empty list returns false
    list1.clear();
    PEGASUS_TEST_ASSERT(!list1.contains("property1"));
    PEGASUS_TEST_ASSERT(!list1.contains("property2"));
    PEGASUS_TEST_ASSERT(!list1.contains("property3"));
    PEGASUS_TEST_ASSERT(!list1.contains("propertyx"));

    // test with a list created with the Array<CIMName>
    Array<CIMName> names4;
    names4.append(CIMName ("property1"));
    names4.append(CIMName ("property2"));
    names4.append(CIMName ("property3"));
    CIMPropertyList list3(names4);
    PEGASUS_TEST_ASSERT(list3.contains("property1"));
    PEGASUS_TEST_ASSERT(list3.contains("property2"));
    PEGASUS_TEST_ASSERT(list3.contains("property3"));
    PEGASUS_TEST_ASSERT(!list3.contains("propertyx"));

    // test of useThisProperty
    list1.clear();
    names.append(CIMName ("property1"));
    names.append(CIMName ("property2"));
    names.append(CIMName ("property3"));
    list1.set(names);
    PEGASUS_TEST_ASSERT(list1.useThisProperty("property1"));
    PEGASUS_TEST_ASSERT(list1.useThisProperty("property2"));
    PEGASUS_TEST_ASSERT(list1.useThisProperty("property3"));
    PEGASUS_TEST_ASSERT(!list1.useThisProperty("propertyx"));

    // retest with list cleared to be sure that an
    // cleared property list returns true for any property
    list1.clear();
    PEGASUS_TEST_ASSERT(list1.useThisProperty("property1"));
    PEGASUS_TEST_ASSERT(list1.useThisProperty("property2"));
    PEGASUS_TEST_ASSERT(list1.useThisProperty("property3"));
    PEGASUS_TEST_ASSERT(list1.useThisProperty("propertyx"));

    // test for correct response with empty list
    Array<CIMName> emptyArray;
    list1.set(emptyArray);
    PEGASUS_TEST_ASSERT(!list1.useThisProperty("property1"));
    PEGASUS_TEST_ASSERT(!list1.useThisProperty("property2"));
    PEGASUS_TEST_ASSERT(!list1.useThisProperty("property3"));
    PEGASUS_TEST_ASSERT(!list1.useThisProperty("propertyx"));

}

//Test reference type properties
void test04()
{
    // Create reference type property from string input and reference class name
    String p =  "//localhost/root/SampleProvider:"
                "TST_PersonDynamic.Name=\"Father\"";
    CIMObjectPath path = p;

    String referenceClassName = "TST_Person";
    CIMProperty p1(CIMName ("message"), path, 0,
                   CIMName(referenceClassName));
    PEGASUS_TEST_ASSERT(!p1.isArray());
    PEGASUS_TEST_ASSERT(p1.getReferenceClassName()==
                        CIMName(referenceClassName));
    PEGASUS_TEST_ASSERT(p1.getType() == CIMTYPE_REFERENCE);

    CIMValue v1;
    v1 = p1.getValue();
    PEGASUS_TEST_ASSERT(v1.getType() ==  CIMTYPE_REFERENCE);
    PEGASUS_TEST_ASSERT(!v1.isNull());
    CIMObjectPath pathout;
    v1.get(pathout);
    PEGASUS_TEST_ASSERT( pathout == path );

    if(verbose)
        XmlWriter::printPropertyElement(p1, cout);

    // Now create an empty property, one used in class declaration
    // for a reference
    CIMProperty p2(CIMName ("parent"), CIMObjectPath(),
                   0, CIMName(referenceClassName));
    PEGASUS_TEST_ASSERT(!p2.isArray());
    PEGASUS_TEST_ASSERT(p2.getReferenceClassName() ==
                         CIMName(referenceClassName));
    PEGASUS_TEST_ASSERT(p2.getType() == CIMTYPE_REFERENCE);

    CIMValue v2;
    v2 = p2.getValue();
    PEGASUS_TEST_ASSERT(v2.getType() ==  CIMTYPE_REFERENCE);

    // ATTN: P3 KS 27 Feb 2003. Why does the following test not work.
    // I assume that the value should
    // be null in this case.
    //PEGASUS_TEST_ASSERT(v2.isNull());

    CIMObjectPath pathout2;
    v1.get(pathout2);
    // Now compare the paths

    if(verbose)
        XmlWriter::printPropertyElement(p2, cout);
}

//Test reference array type properties
void test05()
{
    Array<CIMObjectPath> oa;
    oa.append(CIMObjectPath("/root/cimv2:My_Class.a=1"));
    oa.append(CIMObjectPath("/root/cimv2:My_Class.a=2"));
    CIMProperty p1;

    Boolean gotException = false;
    try
    {
        p1 = CIMProperty(CIMName("property1"), oa, 0, CIMName("refclass"));
    }
    catch (TypeMismatchException&)
    {
        gotException = true;
    }
    PEGASUS_TEST_ASSERT(gotException);

    p1 = CIMProperty(CIMName("property1"), oa[0], 0, CIMName("refclass"));
    gotException = false;
    try
    {
        p1.setValue(oa);
    }
    catch (TypeMismatchException&)
    {
        gotException = true;
    }
    PEGASUS_TEST_ASSERT(gotException);
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
        test05();
    }
    catch (Exception& e)
    {
        cout << "Exception: " << e.getMessage() << endl;
        exit(1);
    }

    cout << argv[0] << " +++++ passed all tests" << endl;

    return 0;
}
