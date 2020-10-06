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

/* This program tests the generation and resolution of instances.  It creates
   a set of qualifiers and a class and then creates instances of the class
   and confirms both the creation characteristics and the resolution
   characteristics.
*/

#include <Pegasus/Common/PegasusAssert.h>
#include <Pegasus/Common/CIMInstance.h>
#include <Pegasus/Common/CIMClass.h>
#include <Pegasus/Common/CIMName.h>
#include <Pegasus/Common/DeclContext.h>
#include <Pegasus/Common/Resolver.h>
#include <Pegasus/Common/XmlWriter.h>
#include <Pegasus/Common/ArrayInternal.h>

PEGASUS_USING_PEGASUS;
PEGASUS_USING_STD;

static Boolean verbose;

void test01()
{
    const CIMNamespaceName NAMESPACE = CIMNamespaceName("/zzz");

    // Create and populate a declaration context:

    SimpleDeclContext* context = new SimpleDeclContext;

    context->addQualifierDecl(
        NAMESPACE, CIMQualifierDecl(CIMName("counter"), false,
        CIMScope::PROPERTY));

    context->addQualifierDecl(
        NAMESPACE, CIMQualifierDecl(CIMName("classcounter"), false,
        CIMScope::CLASS));

    context->addQualifierDecl(
        NAMESPACE, CIMQualifierDecl(CIMName("min"), String(),
        CIMScope::PROPERTY));

    context->addQualifierDecl(
        NAMESPACE, CIMQualifierDecl(CIMName("max"), String(),
        CIMScope::PROPERTY));

    context->addQualifierDecl(NAMESPACE,
        CIMQualifierDecl(CIMName("Description"), String(),
        CIMScope::PROPERTY));

    CIMClass class1(CIMName("MyClass"));

    class1
        .addProperty(CIMProperty(CIMName("count"), Uint32(55))
            .addQualifier(CIMQualifier(CIMName("counter"), true))
            .addQualifier(CIMQualifier(CIMName("min"), String("0")))
            .addQualifier(CIMQualifier(CIMName("max"), String("1"))))
        .addProperty(CIMProperty(CIMName("message"), String("Hello"))
            .addQualifier(CIMQualifier(CIMName("description"),
                String("My Message"))))
        .addProperty(CIMProperty(CIMName("ratio"), Real32(1.5)));

    // Test
    PEGASUS_TEST_ASSERT(
        class1.findProperty(CIMName("count")) != PEG_NOT_FOUND);
    PEGASUS_TEST_ASSERT(
        class1.findProperty(CIMName("message")) != PEG_NOT_FOUND);
    PEGASUS_TEST_ASSERT(
        class1.findProperty(CIMName("ratio")) != PEG_NOT_FOUND);

    Resolver::resolveClass(class1, context, NAMESPACE);
    context->addClass(NAMESPACE, class1);

    if (verbose)
    {
        XmlWriter::printClassElement(class1);
    }

    CIMInstance instance0(CIMName("MyClass"));
    PEGASUS_TEST_ASSERT(instance0.getClassName().equal(CIMName("MyClass")));
    instance0.setPath(CIMObjectPath("//localhost/root/cimv2:MyClass.Foo=1"));
    PEGASUS_TEST_ASSERT(instance0.getPath() ==
        CIMObjectPath("//localhost/root/cimv2:MyClass.Foo=1"));

    CIMInstance instance1(CIMName("MyClass"));
    instance1.addQualifier(CIMQualifier(CIMName("classcounter"), true));

    instance1.addProperty(CIMProperty(CIMName("message"), String("Goodbye")));

    PEGASUS_TEST_ASSERT(
        instance1.findProperty(CIMName("message")) != PEG_NOT_FOUND);

    PEGASUS_TEST_ASSERT(
        instance1.findProperty(CIMName("count")) == PEG_NOT_FOUND);
    PEGASUS_TEST_ASSERT(
        instance1.findProperty(CIMName("ratio")) == PEG_NOT_FOUND);
    PEGASUS_TEST_ASSERT(
        instance1.findProperty(CIMName("nuts")) == PEG_NOT_FOUND);
    PEGASUS_TEST_ASSERT(instance1.getPropertyCount() == 1);

    if (verbose)
    {
        XmlWriter::printInstanceElement(instance1);
    }

    Resolver::resolveInstance(instance1, context, NAMESPACE, true);

    if (verbose)
    {
        XmlWriter::printInstanceElement(instance1);
    }

    // Now test for properties after resolution.

    PEGASUS_TEST_ASSERT(
        instance1.findProperty(CIMName("message")) != PEG_NOT_FOUND);
    PEGASUS_TEST_ASSERT(
        instance1.findProperty(CIMName("count")) != PEG_NOT_FOUND);
    PEGASUS_TEST_ASSERT(
        instance1.findProperty(CIMName("ratio")) != PEG_NOT_FOUND);
    PEGASUS_TEST_ASSERT(
        instance1.findProperty(CIMName("nuts")) == PEG_NOT_FOUND);
    PEGASUS_TEST_ASSERT(instance1.getPropertyCount() == 3);

    // Now remove a property

    Uint32 posProperty;
    posProperty = instance1.findProperty(CIMName("count"));
    instance1.removeProperty(posProperty);

    PEGASUS_TEST_ASSERT(
        instance1.findProperty(CIMName("message")) != PEG_NOT_FOUND);
    PEGASUS_TEST_ASSERT(
        instance1.findProperty(CIMName("count")) == PEG_NOT_FOUND);
    PEGASUS_TEST_ASSERT(
        instance1.findProperty(CIMName("ratio")) != PEG_NOT_FOUND);
    PEGASUS_TEST_ASSERT(
        instance1.findProperty(CIMName("nuts")) == PEG_NOT_FOUND);

    PEGASUS_TEST_ASSERT(instance1.getPropertyCount() == 2);

    // Instance qualifier tests

    CIMQualifier cq=instance1.getQualifier(instance1.findQualifier(
        CIMName("classcounter")));

    const CIMInstance instance2 = instance1.clone();
    PEGASUS_TEST_ASSERT(instance2.identical(instance1));
    PEGASUS_TEST_ASSERT(
        instance1.findQualifier(CIMName("nuts")) == PEG_NOT_FOUND);
    PEGASUS_TEST_ASSERT(
        instance2.findQualifier(CIMName("nuts")) == PEG_NOT_FOUND);
    PEGASUS_TEST_ASSERT(instance1.getQualifierCount() != 4);
    PEGASUS_TEST_ASSERT(instance1.getQualifierCount() == 1);
    PEGASUS_TEST_ASSERT(instance2.getQualifierCount() == 1);

    if (verbose)
    {
        XmlWriter::printInstanceElement(instance2);
    }

    // Tests for CIMConstInstance
    CIMConstInstance cinstance1(CIMName("MyClass")), cinstance3;
    CIMConstInstance ccopy(cinstance1);

    cinstance1 = instance1;
    PEGASUS_TEST_ASSERT(cinstance1.identical(instance1));

    PEGASUS_TEST_ASSERT(cinstance1.getQualifierCount() == 1);
    CIMConstQualifier ccq = cinstance1.getQualifier(cinstance1.findQualifier(
        CIMName("classcounter")));
    PEGASUS_TEST_ASSERT(
        cinstance1.findProperty(CIMName("message")) != PEG_NOT_FOUND);
    CIMConstProperty ccp =
        cinstance1.getProperty(cinstance1.findProperty(CIMName("message")));

    cinstance3 = cinstance1;
    PEGASUS_TEST_ASSERT(cinstance3.identical(cinstance1));

    PEGASUS_TEST_ASSERT(cinstance1.getClassName() == CIMName("MyClass"));
    PEGASUS_TEST_ASSERT(cinstance1.getClassName().equal(CIMName("MyClass")));
    PEGASUS_TEST_ASSERT(cinstance1.getClassName().equal(CIMName("MYCLASS")));
    PEGASUS_TEST_ASSERT(cinstance1.getClassName().equal(CIMName("myclass")));
    PEGASUS_TEST_ASSERT(!cinstance1.getClassName().equal(CIMName("blob")));

    PEGASUS_TEST_ASSERT(cinstance1.getQualifierCount() != 4);
    PEGASUS_TEST_ASSERT(cinstance1.getPropertyCount() == 2);

    CIMConstInstance cinstance2 = cinstance1.clone();
    PEGASUS_TEST_ASSERT(cinstance2.identical(cinstance1));

    if (verbose)
    {
        XmlWriter::printInstanceElement(cinstance1);
    }

    cinstance1.buildPath(class1);

    PEGASUS_TEST_ASSERT( !cinstance1.isUninitialized() );

    delete context;
}

void test02()
{
    const CIMNamespaceName NAMESPACE = CIMNamespaceName("/zzz");

    CIMClass cimClass(CIMName("MyClass"));

    cimClass
        .addProperty(CIMProperty(CIMName("Last"), String())
            .addQualifier(CIMQualifier(CIMName("key"), true)))
        .addProperty(CIMProperty(CIMName("First"), String())
            .addQualifier(CIMQualifier(CIMName("key"), true)))
        .addProperty(CIMProperty(CIMName("Age"), String())
            .addQualifier(CIMQualifier(CIMName("key"), true)));

    CIMInstance cimInstance(CIMName("MyClass"));
    cimInstance.addProperty(CIMProperty(CIMName("first"), String("John")));
    cimInstance.addProperty(CIMProperty(CIMName("last"), String("Smith")));
    cimInstance.addProperty(CIMProperty(CIMName("age"), Uint8(101)));

    PEGASUS_TEST_ASSERT(
        cimInstance.findProperty(CIMName("first")) != PEG_NOT_FOUND);
    PEGASUS_TEST_ASSERT(
        cimInstance.findProperty(CIMName("last")) != PEG_NOT_FOUND);
    PEGASUS_TEST_ASSERT(
        cimInstance.findProperty(CIMName("age")) != PEG_NOT_FOUND);

    PEGASUS_TEST_ASSERT(cimInstance.getPropertyCount() == 3);


    CIMObjectPath instanceName =
        cimInstance.buildPath(CIMConstClass(cimClass));

    CIMObjectPath tmp("myclass.age=101,first=\"John\",last=\"Smith\"");

    PEGASUS_TEST_ASSERT(tmp.makeHashCode() == instanceName.makeHashCode());

    // Test CIMInstance::buildPath with incomplete keys in the instance

    Boolean caughtNoSuchPropertyException = false;

    try
    {
        CIMInstance badInstance(CIMName("MyClass"));
        badInstance.addProperty(CIMProperty(CIMName("first"), String("John")));
        badInstance.addProperty(CIMProperty(CIMName("last"), String("Smith")));
        instanceName = badInstance.buildPath(CIMConstClass(cimClass));
    }
    catch (const NoSuchProperty&)
    {
        caughtNoSuchPropertyException = true;
    }

    PEGASUS_TEST_ASSERT(caughtNoSuchPropertyException);
}

// Build the instance of an association class and test the build path functions.
void test03()
{
    const CIMNamespaceName NAMESPACE = CIMNamespaceName("/zzz");

    CIMClass myPersonClass(CIMName("MY_PersonClass"));

    myPersonClass
        .addProperty(CIMProperty(CIMName("name"), String())
            .addQualifier(CIMQualifier(CIMName("Key"), true)));

    CIMClass myAssocClass(CIMName("MY_AssocClass"));

    myAssocClass
        .addQualifier(CIMQualifier(CIMName("association"), true))
        .addProperty(CIMProperty(CIMName("parent"), CIMObjectPath(), 0,
            CIMName("MY_PersonClass"))
            .addQualifier(CIMQualifier(CIMName("key"), true)))
        .addProperty(CIMProperty(CIMName("child"), CIMObjectPath(), 0,
            CIMName("MY_PersonClass"))
            .addQualifier(CIMQualifier(CIMName("key"), true)))
        .addProperty(CIMProperty(CIMName("Age"), String()));

    CIMInstance fatherInstance(CIMName("MY_PersonClass"));

    fatherInstance
        .addProperty(CIMProperty(CIMName("name"), String("father")));

    CIMInstance daughterInstance(CIMName("My_PersonClass"));
    daughterInstance
       .addProperty(CIMProperty(CIMName("name"), String("daughter")));

    CIMObjectPath fatherInstancePath =
        fatherInstance.buildPath(CIMConstClass(myPersonClass));

    CIMObjectPath daughterInstancePath =
        daughterInstance.buildPath(CIMConstClass(myPersonClass));

    CIMInstance assocInstance(CIMName("MY_AssocClass"));

    assocInstance.addProperty(CIMProperty(CIMName("parent"),
        CIMObjectPath(fatherInstancePath),0,CIMName("MY_PersonClass")));

    assocInstance.addProperty(CIMProperty(CIMName("child"),
        CIMObjectPath(daughterInstancePath),0, CIMName("MY_PersonClass")));

    CIMObjectPath assocInstancePath =
        assocInstance.buildPath(CIMConstClass(myAssocClass));

    // Now do the asserts, etc.
    // See if the pathing works on Associations and association instances

    if (verbose)
    {
        XmlWriter::printClassElement(myPersonClass);
        XmlWriter::printClassElement(myAssocClass);
        XmlWriter::printInstanceElement(fatherInstance);
        XmlWriter::printInstanceElement(daughterInstance);
        XmlWriter::printInstanceElement(assocInstance);
    }

    if (verbose)
    {
        cout << "Paths " << endl;
        cout << "FatherInstancePath = " << fatherInstancePath.toString() <<
            endl;
        cout << "DaughterInstancePath = " << daughterInstancePath.toString() <<
            endl;
        cout << "AssocInstancePath = " << assocInstancePath.toString() << endl;
    }

    String x ="MY_AssocClass.child=\"My_PersonClass.name=\\\"daughter\\\"\","
        "parent=\"MY_PersonClass.name=\\\"father\\\"\"";
    PEGASUS_TEST_ASSERT(x == assocInstancePath.toString());
    CIMObjectPath px = x;
    PEGASUS_TEST_ASSERT(px.identical(assocInstancePath));
}

Boolean _propertyIdentical(
    const char* propertyName,
    CIMInstance& instance1,
    CIMInstance& instance2)
{
    Uint32 pos = instance1.findProperty(propertyName);
    CIMConstProperty p1 = instance1.getProperty(pos);
    pos = instance2.findProperty(propertyName);
    CIMConstProperty p2 = instance2.getProperty(pos);
    return (p1.identical(p2));
}

// Compare the class and instance provided to determine if the attributes
// and properties match.  The includeQualifier parameter, classOrigin
// parameter and pl define whether these attributes were used in creating
// or filtering the instance so they are tested to in accord with the
// parameters.
// Processing continues with error messages produced for discrepencies.
// If there are any discrepencies, both objects are displayed and false
// is returned to the caller.
bool _compare(const CIMClass& myClass,
                     const CIMInstance& inst,
                     bool includeQualifiers,
                     bool classOrigin,
                     const CIMPropertyList& pl)
{
    bool rtn = true;
    if (myClass.getClassName() != inst.getClassName())
    {
        rtn = false;
        cout << "Class and Instance Name mismatch" << endl;
    }

    if (includeQualifiers && 
        myClass.getQualifierCount() != inst.getQualifierCount())
    {
        rtn = false;
        cout << "Diff in class qualifiers" << endl;
    }
    else if (!includeQualifiers && inst.getQualifierCount() != 0)
    {
        rtn = false;
        cout << "Expected class level qualifier count == 0" << endl;
    }

    // test for properties expected in instance before executing 
    // detailed property tests
    if (pl.isNull() ||  pl.size() != 0)
    {
        Array<CIMName> plArray = pl.getPropertyNameArray();

        // Loop to test properties from class
        for (Uint32 i = 0 ; i < myClass.getPropertyCount() ; i++)
        {
            CIMConstProperty p1 = myClass.getProperty(i);
            String pName = p1.getName().getString();

            // if this property is in the property list or the
            // propertylist is NULL compare this property with
            // the instance
            if (pl.isNull() || Contains(plArray, p1.getName()))
            {
                CIMConstProperty p2;
                
                // if the property exists in the instance
                Uint32 pos;
                if ((pos = inst.findProperty( p1.getName())) != PEG_NOT_FOUND)
                {
                     p2 = inst.getProperty(pos);
                }
                else
                {
                    rtn = false;
                    cout << "Property " << pName
                         << " Not found in instance" << endl;
                    continue;
                }

                // Property exists in instance.  Compare to property in class
                if (!p1.identical(p2))
                {
                    if (p1.getName() != p2.getName())
                    {
                        rtn = false;
                        cout << "Property Name difference " << pName
                             << " " << p1.getName().getString()
                             << " 2 " << p2.getName().getString()
                            << endl;
                    }
                    if (p1.getType() != p2.getType())
                    {
                        rtn = false;
                        cout << " Property Types differ " << pName
                            << endl;
                    }
                    if (includeQualifiers &&
                         (p1.getQualifierCount() != p2.getQualifierCount()))
                    {
                        rtn = false;
                        cout << " Property QualifierCounts differ "
                             << pName
                             << endl;
                    }
                    else if (!includeQualifiers && 
                             inst.getQualifierCount() != 0)
                    {
                        rtn = false;
                        cout << "Expected QualifierCount == 0" 
                             << pName << endl;
                    }
                    
                    // test for classOrigin true and values match
                    // or expect classorigin null
                    if ( classOrigin &&
                         (p1.getClassOrigin() != p2.getClassOrigin()))
                    {
                        rtn = false;
                        cout << "ClassOrigins Differ" << endl;
                    }
                    else if (!classOrigin && !p2.getClassOrigin().isNull())
                    {
                        rtn = false;
                        cout << "Expect ClassOrigin == Null " << endl;
                    }

                    if (p1.getPropagated() != p2.getPropagated())
                    {
                        rtn = false;
                        cout << "Propagated Attributes Differ" << endl;
                    }

                    if (p1.getArraySize() != p2.getArraySize())
                    {
                        rtn = false;
                        cout << "ArraySize Attributes Differ" << endl;
                    }

                    if (p1.getValue() != p2.getValue())
                    {
                        rtn = false;
                        cout << "Propagated Attributes Differ" << endl;
                    }
                }
            }
        }
    }

    // if error, display the class and instance
    if (!rtn)
    {
        cout << "Error in class property to inst property comparison. "
                "Class" << endl;
        XmlWriter::printClassElement(myClass);
        cout << "Instance" << endl;
        XmlWriter::printInstanceElement(inst);
    }
    return rtn;
}
//
// Test of the Instance creation and instance filtering
//
void test04()
{
    if (verbose)
    {
        cout << "Test04 - Create instance from Class. " << endl;
    }

    const CIMNamespaceName NAMESPACE = CIMNamespaceName("/zzz");

    // Create and populate a declaration context:

    SimpleDeclContext* context = new SimpleDeclContext;

    context->addQualifierDecl(
        NAMESPACE, CIMQualifierDecl(CIMName("counter"), false,
        CIMScope::PROPERTY));

    context->addQualifierDecl(
        NAMESPACE, CIMQualifierDecl(CIMName("classcounter"), false,
        CIMScope::CLASS));

    context->addQualifierDecl(
        NAMESPACE, CIMQualifierDecl(CIMName("min"), String(),
        CIMScope::PROPERTY));

    context->addQualifierDecl(
        NAMESPACE, CIMQualifierDecl(CIMName("max"), String(),
        CIMScope::PROPERTY));

    context->addQualifierDecl(NAMESPACE,
        CIMQualifierDecl(CIMName("Description"), String(),
        CIMScope::PROPERTY));

    CIMClass class1(CIMName("MyClass"));
    Array<Uint32> testArray;
    testArray.append(Uint32(1));
    testArray.append(Uint32(2));
    CIMValue testArrayValue(testArray);

    class1
        .addProperty(CIMProperty(CIMName("count"), Uint32(55))
            .addQualifier(CIMQualifier(CIMName("counter"), true))
            .addQualifier(CIMQualifier(CIMName("min"), String("0")))
            .addQualifier(CIMQualifier(CIMName("max"), String("1"))))
        .addProperty(CIMProperty(CIMName("message"), String("Hello"))
            .addQualifier(CIMQualifier(CIMName("description"),
                String("My Message"))))
        .addProperty(CIMProperty(CIMName("ratio"), Real32(1.5)))
        .addProperty(CIMProperty(CIMName("array"), testArrayValue));

    Resolver::resolveClass(class1, context, NAMESPACE);
    context->addClass(NAMESPACE, class1);


    // Create a subclass to do classOrigin and propagation testing
    CIMClass mySubClass(CIMName("subclass"));
    mySubClass.setSuperClassName(CIMName("MyClass"));

    Resolver::resolveClass(mySubClass, context, NAMESPACE);
    context->addClass(NAMESPACE, mySubClass);

    if (verbose)
    {
        XmlWriter::printClassElement(class1);
        XmlWriter::printClassElement(mySubClass);
    }

    //
    // Create instance with qualifiers, classorigin, and Null propertyList
    //
    {
        CIMInstance newInstance;

        newInstance = class1.buildInstance(true, true, CIMPropertyList());

        if (verbose)
        {
            XmlWriter::printInstanceElement(newInstance);
        }

        PEGASUS_TEST_ASSERT(
            newInstance.getPropertyCount() == class1.getPropertyCount());
        PEGASUS_TEST_ASSERT(
            newInstance.getQualifierCount() == class1.getQualifierCount());
        PEGASUS_TEST_ASSERT(
            newInstance.findProperty("ratio") != PEG_NOT_FOUND);
        PEGASUS_TEST_ASSERT(
            newInstance.findProperty("message") != PEG_NOT_FOUND);
        PEGASUS_TEST_ASSERT(
            newInstance.findProperty("array") != PEG_NOT_FOUND);

        PEGASUS_TEST_ASSERT(_compare(class1, newInstance, true, true,
                                     CIMPropertyList()));
    }

    // Test with instance of subclass
    // Create instance with qualifiers, classorigin, and Null propertyList
    // Primary function is testing propagated  and classOrigin attributes.
    {
        CIMInstance newInstance;

        newInstance = mySubClass.buildInstance(true, true, CIMPropertyList());

        if (verbose)
        {
            XmlWriter::printInstanceElement(newInstance);
        }

        PEGASUS_TEST_ASSERT(
            newInstance.getPropertyCount() == class1.getPropertyCount());
        PEGASUS_TEST_ASSERT(
            newInstance.getQualifierCount() == class1.getQualifierCount());
        PEGASUS_TEST_ASSERT(
            newInstance.findProperty("ratio") != PEG_NOT_FOUND);
        PEGASUS_TEST_ASSERT(
            newInstance.findProperty("message") != PEG_NOT_FOUND);
        PEGASUS_TEST_ASSERT(
            newInstance.findProperty("array") != PEG_NOT_FOUND);

        PEGASUS_TEST_ASSERT(_compare(mySubClass, newInstance, true, true,
                                     CIMPropertyList()));
    }

    //
    // Test with include qualifiers false. Should be no qualifiers in result
    //
    {
        CIMInstance newInstance =
            class1.buildInstance(false, true, CIMPropertyList());

        PEGASUS_TEST_ASSERT(newInstance.getQualifierCount() == 0);
        PEGASUS_TEST_ASSERT(
            newInstance.getPropertyCount() == class1.getPropertyCount());
        PEGASUS_TEST_ASSERT(
            newInstance.findProperty("ratio") != PEG_NOT_FOUND);
        PEGASUS_TEST_ASSERT(
            newInstance.findProperty("message") != PEG_NOT_FOUND);
        PEGASUS_TEST_ASSERT(
            newInstance.findProperty("array") != PEG_NOT_FOUND);

        PEGASUS_TEST_ASSERT(_compare(class1, newInstance, false, true,
                                            CIMPropertyList()));
    }

    //
    // Test with empty property list.  Should have no properties.
    //
    {
        Array<CIMName> pl1Array;
        CIMPropertyList pl1(pl1Array);

        CIMInstance newInstance = class1.buildInstance(false, true, pl1);

        PEGASUS_TEST_ASSERT(newInstance.getQualifierCount() == 0);
        PEGASUS_TEST_ASSERT(newInstance.getPropertyCount() == 0);

        PEGASUS_TEST_ASSERT(_compare(class1, newInstance, false, true,
                                            pl1));
    }

    //
    // Test with a property that exists in property list.
    //
    {
        Array<CIMName> pl1Array;
        pl1Array.append("ratio");
        CIMPropertyList pl1(pl1Array);

        CIMInstance newInstance = class1.buildInstance(false, true, pl1);

        if (verbose)
        {
            cout << "Test with one property in new instance" << endl;
            XmlWriter::printInstanceElement(newInstance);
        }

        PEGASUS_TEST_ASSERT(newInstance.getPropertyCount() == 1);
        PEGASUS_TEST_ASSERT(
            newInstance.findProperty("ratio") != PEG_NOT_FOUND);
        PEGASUS_TEST_ASSERT(
            newInstance.findProperty("message") == PEG_NOT_FOUND);
        PEGASUS_TEST_ASSERT(newInstance.getQualifierCount() == 0);
        PEGASUS_TEST_ASSERT(
            newInstance.findProperty("array") == PEG_NOT_FOUND);

        PEGASUS_TEST_ASSERT(_compare(class1, newInstance, false, true,
                                            pl1));
    }

    //
    // Test with a property that does/does not exist in property list
    //
    {
        Array<CIMName> pl1Array;
        CIMPropertyList pl1(pl1Array);
        pl1.clear();
        pl1Array.append("blob");
        pl1Array.append("ratio");
        pl1.set(pl1Array);

        CIMInstance newInstance = class1.buildInstance(false, true, pl1);

        PEGASUS_TEST_ASSERT(newInstance.getPropertyCount() == 1);
        PEGASUS_TEST_ASSERT(newInstance.findProperty("ratio") != PEG_NOT_FOUND);
        PEGASUS_TEST_ASSERT(newInstance.findProperty("blob") == PEG_NOT_FOUND);
        PEGASUS_TEST_ASSERT(
            newInstance.findProperty("message") == PEG_NOT_FOUND);
        PEGASUS_TEST_ASSERT(newInstance.getQualifierCount() == 0);

        PEGASUS_TEST_ASSERT(_compare(class1, newInstance, false, true,
                                            pl1));
    }


    delete context;
}

//
// Test instance qualifier operations
//
void test05()
{
    CIMInstance instance1(CIMName("MyClass"));
    instance1.addQualifier(CIMQualifier(CIMName("Qualifier1"), true));
    instance1.addQualifier(CIMQualifier(CIMName("Qualifier2"), Uint32(1)));
    instance1.addQualifier(CIMQualifier(CIMName("Qualifier3"), String("a")));

    Boolean caughtException = false;

    try
    {
        instance1.addQualifier(CIMQualifier(CIMName("Qualifier1"), true));
    }
    catch (const AlreadyExistsException&)
    {
        caughtException = true;
    }

    PEGASUS_TEST_ASSERT(caughtException);
    PEGASUS_TEST_ASSERT(instance1.getQualifierCount() == 3);

    // Test that the correct qualifiers exist in the instance

    Uint32 qualifierIndex;

    qualifierIndex = instance1.findQualifier(CIMName("Qualifier1"));
    PEGASUS_TEST_ASSERT(qualifierIndex != PEG_NOT_FOUND);

    qualifierIndex = instance1.findQualifier(CIMName("Qualifier2"));
    PEGASUS_TEST_ASSERT(qualifierIndex != PEG_NOT_FOUND);

    qualifierIndex = instance1.findQualifier(CIMName("Qualifier3"));
    PEGASUS_TEST_ASSERT(qualifierIndex != PEG_NOT_FOUND);

    qualifierIndex = instance1.findQualifier(CIMName("Qualifier4"));
    PEGASUS_TEST_ASSERT(qualifierIndex == PEG_NOT_FOUND);

    // Clone the instance and test for the correct qualifiers

    CIMConstInstance instance2 = instance1.clone();
    PEGASUS_TEST_ASSERT(instance2.getQualifierCount() == 3);

    qualifierIndex = instance2.findQualifier(CIMName("Qualifier1"));
    PEGASUS_TEST_ASSERT(qualifierIndex != PEG_NOT_FOUND);

    qualifierIndex = instance2.findQualifier(CIMName("Qualifier2"));
    PEGASUS_TEST_ASSERT(qualifierIndex != PEG_NOT_FOUND);

    qualifierIndex = instance2.findQualifier(CIMName("Qualifier3"));
    PEGASUS_TEST_ASSERT(qualifierIndex != PEG_NOT_FOUND);

    qualifierIndex = instance2.findQualifier(CIMName("Qualifier4"));
    PEGASUS_TEST_ASSERT(qualifierIndex == PEG_NOT_FOUND);

    // Remove Qualifier2 from instance1

    qualifierIndex = instance1.findQualifier(CIMName("Qualifier2"));
    instance1.removeQualifier(qualifierIndex);
    PEGASUS_TEST_ASSERT(instance1.getQualifierCount() == 2);

    qualifierIndex = instance1.findQualifier(CIMName("Qualifier2"));
    PEGASUS_TEST_ASSERT(qualifierIndex == PEG_NOT_FOUND);

    qualifierIndex = instance1.findQualifier(CIMName("Qualifier1"));
    PEGASUS_TEST_ASSERT(qualifierIndex != PEG_NOT_FOUND);

    // Confirm that the qualifiers are still in instance2

    PEGASUS_TEST_ASSERT(instance2.getQualifierCount() == 3);
    qualifierIndex = instance2.findQualifier(CIMName("Qualifier1"));
    PEGASUS_TEST_ASSERT(qualifierIndex != PEG_NOT_FOUND);

    qualifierIndex = instance2.findQualifier(CIMName("Qualifier2"));
    PEGASUS_TEST_ASSERT(qualifierIndex != PEG_NOT_FOUND);

    // Remove a non-existant qualifier

    caughtException = false;

    try
    {
        instance1.removeQualifier(10);
    }
    catch (const IndexOutOfBoundsException&)
    {
        caughtException = true;
    }

    PEGASUS_TEST_ASSERT(caughtException);

    // Remove Qualifier1 and Qualifier3 from instance1

    qualifierIndex = instance1.findQualifier(CIMName("Qualifier1"));
    instance1.removeQualifier(qualifierIndex);
    PEGASUS_TEST_ASSERT(instance1.getQualifierCount() == 1);

    qualifierIndex = instance1.findQualifier(CIMName("Qualifier3"));
    instance1.removeQualifier(qualifierIndex);
    PEGASUS_TEST_ASSERT(instance1.getQualifierCount() == 0);

    qualifierIndex = instance1.findQualifier(CIMName("Qualifier1"));
    PEGASUS_TEST_ASSERT(qualifierIndex == PEG_NOT_FOUND);

    qualifierIndex = instance1.findQualifier(CIMName("Qualifier3"));
    PEGASUS_TEST_ASSERT(qualifierIndex == PEG_NOT_FOUND);

    // Confirm that the qualifiers are still in instance2

    PEGASUS_TEST_ASSERT(instance2.getQualifierCount() == 3);
    qualifierIndex = instance2.findQualifier(CIMName("Qualifier1"));
    PEGASUS_TEST_ASSERT(qualifierIndex != PEG_NOT_FOUND);

    qualifierIndex = instance2.findQualifier(CIMName("Qualifier2"));
    PEGASUS_TEST_ASSERT(qualifierIndex != PEG_NOT_FOUND);

    qualifierIndex = instance2.findQualifier(CIMName("Qualifier3"));
    PEGASUS_TEST_ASSERT(qualifierIndex != PEG_NOT_FOUND);
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
