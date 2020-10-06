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
#include <Pegasus/Common/PegasusAssert.h>
#include <Pegasus/Common/XmlWriter.h>
#include <Pegasus/Common/CIMPropertyList.h>
#include <Pegasus/Common/FileSystem.h>

#include <Pegasus/General/MofWriter.h>

#include <Pegasus/Repository/CIMRepository.h>

PEGASUS_USING_PEGASUS;
PEGASUS_USING_STD;
static Boolean verbose;

String repositoryRoot;

void TestNameSpaces(Uint32 mode)
{
  CIMRepository r (repositoryRoot, mode);

    r.createNameSpace(CIMNamespaceName ("namespace0"));
    r.createNameSpace(CIMNamespaceName ("namespace1"));
    r.createNameSpace(CIMNamespaceName ("namespace2"));
    r.createNameSpace(CIMNamespaceName ("namespace2/subnamespace"));
    r.createNameSpace(CIMNamespaceName (
                "namespace2/subnamespace/subsubnamespace"));

    Array<CIMNamespaceName> nameSpaces;
    nameSpaces = r.enumerateNameSpaces();
    BubbleSort(nameSpaces);

    PEGASUS_TEST_ASSERT(nameSpaces.size() == 6);
    PEGASUS_TEST_ASSERT(nameSpaces[0] == CIMNamespaceName ("namespace0"));
    PEGASUS_TEST_ASSERT(nameSpaces[1] == CIMNamespaceName ("namespace1"));
    PEGASUS_TEST_ASSERT(nameSpaces[2] == CIMNamespaceName ("namespace2"));
    PEGASUS_TEST_ASSERT(
        nameSpaces[3] == CIMNamespaceName ("namespace2/subnamespace"));
    PEGASUS_TEST_ASSERT(nameSpaces[4] == CIMNamespaceName (
        "namespace2/subnamespace/subsubnamespace"));
    PEGASUS_TEST_ASSERT(nameSpaces[5] == CIMNamespaceName ("root"));

    r.deleteNameSpace(CIMNamespaceName ("namespace0"));
    r.deleteNameSpace(CIMNamespaceName ("namespace1"));

    nameSpaces = r.enumerateNameSpaces();
    PEGASUS_TEST_ASSERT(nameSpaces.size() == 4);
    BubbleSort(nameSpaces);
    PEGASUS_TEST_ASSERT(nameSpaces[0] == CIMNamespaceName ("namespace2"));
    PEGASUS_TEST_ASSERT(
        nameSpaces[1] == CIMNamespaceName ("namespace2/subnamespace"));
    PEGASUS_TEST_ASSERT(nameSpaces[2] == CIMNamespaceName (
        "namespace2/subnamespace/subsubnamespace"));
    PEGASUS_TEST_ASSERT(nameSpaces[3] == CIMNamespaceName ("root"));

    r.deleteNameSpace(CIMNamespaceName ("namespace2"));
    r.deleteNameSpace(CIMNamespaceName ("namespace2/subnamespace"));
    r.deleteNameSpace(
        CIMNamespaceName ("namespace2/subnamespace/subsubnamespace"));
    nameSpaces = r.enumerateNameSpaces();
    PEGASUS_TEST_ASSERT(nameSpaces.size() == 1);
    PEGASUS_TEST_ASSERT(nameSpaces[0] == CIMNamespaceName ("root"));
}



void TestCreateClass(Uint32 mode)
{
  //
  // -- Create repository and "xyz" namespace:
  //
  CIMRepository r (repositoryRoot, mode);
    const CIMNamespaceName NS = CIMNamespaceName ("TestCreateClass");

    try
    {
    r.createNameSpace(NS);
    }
    catch (AlreadyExistsException&)
    {
    // Ignore this!
    }

    //
    // -- Declare the key qualifier:
    //
    r.setQualifier(NS, CIMQualifierDecl(CIMName ("key"),true,
                   CIMScope::PROPERTY));
    r.setQualifier(NS, CIMQualifierDecl(CIMName ("description"),String(),
                   (CIMScope::PROPERTY + CIMScope::CLASS)));
    r.setQualifier(NS, CIMQualifierDecl(CIMName ("junk"),String(),
                   (CIMScope::PROPERTY + CIMScope::CLASS)));

    // -- Construct new class:"*REMOVED*"
    //  CIMQualifier d(CIMName("description"),
    //  String("Test info in SuperClass"));
    // Updated test to ensure it works with enabled PEGASUS_REMOVE_DESCRIPTIONS
    // as well as not enabled.

    CIMQualifier d(
        CIMName("description"), String("*REMOVED*"), CIMFlavor::DEFAULTS);
    CIMClass c1(CIMName ("SuperClass"));
    c1.addQualifier(d);
    c1.addProperty(CIMProperty(CIMName ("key"), Uint32(0))
        .addQualifier(CIMQualifier(CIMName("key"), true, CIMFlavor::DEFAULTS)));

    c1.addProperty(CIMProperty(CIMName ("ratio"), Real32(1.5)));
    c1.addProperty(CIMProperty(CIMName ("message"), String("Hello World")));

    //
    //-- Create the class (get it back and compare):
    //
    r.createClass(NS, c1);
    CIMConstClass cc1;
    cc1 = r.getClass(NS, CIMName("SuperClass"), true, true, false);
    PEGASUS_TEST_ASSERT(c1.identical(cc1));
    PEGASUS_TEST_ASSERT(cc1.identical(c1));

    // -- Now create a sub class (get it back and compare):
    // c22 has one additional property than c1 (junk)
    //

    CIMClass c2(CIMName ("SubClass"), CIMName ("SuperClass"));

    //
    // Add new qualifier that will be local
    //
    CIMQualifier jq(
        CIMName("junk"), String("TestQualifier"), CIMFlavor::DEFAULTS);
    c2.addQualifier(jq);

    c2.addProperty(CIMProperty(CIMName ("junk"), Real32(66.66)));
    r.createClass(NS, c2);
    CIMConstClass cc2;
    cc2 = r.getClass(NS, CIMName ("SubClass"), true, true, false);
    //XmlWriter::printClassElement(c2);
    //XmlWriter::printClassElement(cc2);

    PEGASUS_TEST_ASSERT(c2.identical(cc2));
    PEGASUS_TEST_ASSERT(cc2.identical(c2));

    //
    // -- Modify "SubClass" (add new property)
    //
    c2.addProperty(CIMProperty(CIMName ("newProperty"), Uint32(888)));
    r.modifyClass(NS, c2);
    cc2 = r.getClass(NS, CIMName ("SubClass"), true, true, false);
    PEGASUS_TEST_ASSERT(c2.identical(cc2));
    PEGASUS_TEST_ASSERT(cc2.identical(c2));
    // should test for this new property on subclass also.

    //
    // -- Enumerate the class names: expect "SuperClass", "SubClass"
    //
    Array<CIMName> classNames = r.enumerateClassNames(NS, CIMName (), true);
    if (verbose)
      {
    for (Uint32 i = 0 ; i < classNames.size(); i++)
      {
        cout << classNames[i].getString();
      }
      }
    BubbleSort(classNames);
    PEGASUS_TEST_ASSERT(classNames.size() == 2);
    PEGASUS_TEST_ASSERT(classNames[1] == CIMName ("SuperClass"));
    PEGASUS_TEST_ASSERT(classNames[0] == CIMName ("SubClass"));

    //
    // Test the getClass operation options, localonly,
    //      includeQualifiers, includeClassOrigin, propertyList
    //

    // test: localonly true, includequalifiers true, classorigin true
    cc2 = r.getClass(NS, CIMName ("SubClass"), true, true, true);
    if (verbose)
      {
    XmlWriter::printClassElement(c1);
    XmlWriter::printClassElement(cc2);
      }

    PEGASUS_TEST_ASSERT(cc2.findProperty("ratio") == PEG_NOT_FOUND);
    PEGASUS_TEST_ASSERT(cc2.findProperty("message") == PEG_NOT_FOUND);
    PEGASUS_TEST_ASSERT(cc2.findProperty("junk") != PEG_NOT_FOUND);

    //
    // Test to assure that propagated class qualifier removed and
    // local one not removed
    // The following test does not work because propagated, etc. not set.
    PEGASUS_TEST_ASSERT (cc2.findQualifier("Description") == PEG_NOT_FOUND);
    PEGASUS_TEST_ASSERT (cc2.findQualifier("junk") != PEG_NOT_FOUND);

    // test for qualifier on the junk property.
    // ATTN: TODO

    //
    // test localonly == false
    // (localonly false, includequalifiers true, classorigin true)
    //
    cc2 = r.getClass(NS, CIMName ("SubClass"), false, true, true);
    PEGASUS_TEST_ASSERT(cc2.findProperty("ratio") != PEG_NOT_FOUND);
    PEGASUS_TEST_ASSERT(cc2.findProperty("message") != PEG_NOT_FOUND);

    //
    // test includeQualifiers set true
    // (localonly true, includequalifiers true, classorigin true)
    //
    cc2 = r.getClass(NS, CIMName ("SuperClass"), true, true, true);
    PEGASUS_TEST_ASSERT(cc2.getQualifierCount() != 0);

    //
    // test includeQualifiers set false
    // (localonly true, includequalifiers false, classorigin true)
    //
    cc2 = r.getClass(NS, CIMName ("SuperClass"), true, false, true);
    PEGASUS_TEST_ASSERT(cc2.getQualifierCount() == 0);

    for (Uint32 i = 0; i < cc2.getPropertyCount(); i++)
      {
    CIMConstProperty p = cc2.getProperty(i);
    PEGASUS_TEST_ASSERT(p.getQualifierCount() == 0);
      }
    for (Uint32 i = 0; i < cc2.getMethodCount(); i++)
      {
    CIMConstMethod m = cc2.getMethod(i);
    PEGASUS_TEST_ASSERT(m.getQualifierCount() == 0);
      }

    //
    // Test for Class origin set true
    // (localonly false, includequalifiers true, classorigin true)
    //
    cc2 = r.getClass(NS, CIMName ("SubClass"), false, true, true);
    CIMConstProperty cp;
    Uint32 pos  =  cc2.findProperty("ratio");
    PEGASUS_TEST_ASSERT(pos != PEG_NOT_FOUND);
    cp = cc2.getProperty(pos);
    PEGASUS_TEST_ASSERT(cp.getClassOrigin() == CIMName("SuperClass"));

    //
    // Test for Class origin set false. Should return null CIMName.
    // (localonly false, includequalifiers true, classorigin false)
    //
    cc2 = r.getClass(NS, CIMName ("SubClass"), false, true, false);
    CIMConstProperty p1;
    Uint32 pos1  =  cc2.findProperty("ratio");
    PEGASUS_TEST_ASSERT(pos1 != PEG_NOT_FOUND);
    p1 = cc2.getProperty(pos);
    PEGASUS_TEST_ASSERT(p1.getClassOrigin() == CIMName());


    //
    // Test for propertylist set
    //

    //
    // Test with empty property in list.
    //
    Array<CIMName> pls_empty;
    CIMPropertyList pl(pls_empty);
    cc2 = r.getClass(NS, CIMName ("SuperClass"), false, true, true, pl);
    PEGASUS_TEST_ASSERT(cc2.findProperty("ratio") == PEG_NOT_FOUND);
    PEGASUS_TEST_ASSERT(cc2.findProperty("message") == PEG_NOT_FOUND);
    PEGASUS_TEST_ASSERT(cc2.getPropertyCount() == 0);

    //
    // Test with one property in list.
    //
    Array<CIMName> pls;
    pls.append(CIMName("ratio"));
    pl.clear();
    pl.set(pls);
    cc2 = r.getClass(NS, CIMName ("SuperClass"), false, true, true, pl);
    PEGASUS_TEST_ASSERT(cc2.findProperty("ratio") != PEG_NOT_FOUND);
    PEGASUS_TEST_ASSERT(cc2.findProperty("message") == PEG_NOT_FOUND);
    PEGASUS_TEST_ASSERT(cc2.getPropertyCount() == 1);

    //
    // retest with two entries in the list.
    //
    pls.append(CIMName("message"));
    pl.clear();
    pl.set(pls);
    cc2 = r.getClass(NS, CIMName ("SuperClass"), false, true, true, pl);
    PEGASUS_TEST_ASSERT(cc2.findProperty("ratio") != PEG_NOT_FOUND);
    PEGASUS_TEST_ASSERT(cc2.findProperty("message") != PEG_NOT_FOUND);
    PEGASUS_TEST_ASSERT(cc2.getPropertyCount() == 2);

    //
    // Test with an invalid property in the list. It should be ignored
    // and the results should be identical to the previous.
    //
    pls.append(CIMName("herroyalhighnessofyork"));
    pl.clear();
    pl.set(pls);
    cc2 = r.getClass(NS, CIMName ("SuperClass"), false, true, true, pl);
    PEGASUS_TEST_ASSERT(cc2.findProperty("ratio") != PEG_NOT_FOUND);
    PEGASUS_TEST_ASSERT(cc2.findProperty("message") != PEG_NOT_FOUND);
    PEGASUS_TEST_ASSERT(cc2.getPropertyCount() == 2);


    //
    // -- Create an instance of each class:
    //
    CIMInstance inst0(CIMName ("SuperClass"));
    inst0.addProperty(CIMProperty(CIMName ("key"), Uint32(111)));
    r.createInstance(NS, inst0);

    CIMInstance inst1(CIMName ("SubClass"));
    inst1.addProperty(CIMProperty(CIMName ("key"), Uint32(222)));
    r.createInstance(NS, inst1);

    //
    // -- Enumerate instances names:
    //
    Array<CIMObjectPath> instanceNames =
    r.enumerateInstanceNamesForSubtree(NS, CIMName ("SuperClass"));

    PEGASUS_TEST_ASSERT(instanceNames.size() == 2);

    PEGASUS_TEST_ASSERT(
    instanceNames[0].toString() == "SuperClass.key=111" ||
    instanceNames[0].toString() == "SubClass.key=222");

    PEGASUS_TEST_ASSERT(
    instanceNames[1].toString() == "SuperClass.key=111" ||
    instanceNames[1].toString() == "SubClass.key=222");

    inst0.setPath (CIMObjectPath ("SuperClass.key=111"));
    inst1.setPath (CIMObjectPath ("SubClass.key=222"));


    //
    // -- Enumerate instances:
    //
    Array<CIMInstance> namedInstances = r.enumerateInstancesForSubtree(
        NS, CIMName("SuperClass"));

    PEGASUS_TEST_ASSERT(namedInstances.size() == 2);

    //XmlWriter::printInstanceElement(namedInstances[0], cout);
    //XmlWriter::printInstanceElement(inst0, cout);
    //XmlWriter::printInstanceElement(inst1, cout);

    for (Uint32 i = 0; i < namedInstances.size(); i++)
    {
        if (namedInstances[i].getClassName() == CIMName("SuperClass"))
        {
            PEGASUS_TEST_ASSERT(namedInstances[i].getProperty(
                namedInstances[i].findProperty("key")).getValue() ==
                CIMValue(Uint32(111)));
        }
        else
        {
            PEGASUS_TEST_ASSERT(namedInstances[i].getClassName() ==
                CIMName("SubClass"));
            PEGASUS_TEST_ASSERT(namedInstances[i].getProperty(
                namedInstances[i].findProperty("key")).getValue() ==
                CIMValue(Uint32(222)));
        }

        // Test propagated properties
        PEGASUS_TEST_ASSERT(namedInstances[i].getProperty(
            namedInstances[i].findProperty("ratio")).getValue() ==
            CIMValue(Real32(1.5)));
        PEGASUS_TEST_ASSERT(namedInstances[i].getProperty(
            namedInstances[i].findProperty("ratio")).getPropagated());
        PEGASUS_TEST_ASSERT(namedInstances[i].getProperty(
            namedInstances[i].findProperty("message")).getValue() ==
            CIMValue(String("Hello World")));
        PEGASUS_TEST_ASSERT(namedInstances[i].getProperty(
            namedInstances[i].findProperty("message")).getPropagated());
    }

    //
    // Repeat the above tests for the enumerateInstancesForClass function
    //
    namedInstances = r.enumerateInstancesForClass(NS, CIMName("SuperClass"));

    PEGASUS_TEST_ASSERT(namedInstances.size() == 1);

    //XmlWriter::printInstanceElement(namedInstances[0], cout);
    //XmlWriter::printInstanceElement(inst0, cout);

    PEGASUS_TEST_ASSERT(namedInstances[0].getPropertyCount() == 3);
    PEGASUS_TEST_ASSERT(
        namedInstances[0].getProperty(namedInstances[0].findProperty("key"))
        .identical(CIMProperty(CIMName("key"), Uint32(111))));
    PEGASUS_TEST_ASSERT(
        namedInstances[0].getProperty(namedInstances[0].findProperty("ratio"))
        .identical(CIMProperty(CIMName("ratio"), Real32(1.5), 0, CIMName(),
            CIMName(), true)));
    PEGASUS_TEST_ASSERT(
        namedInstances[0].getProperty(namedInstances[0].findProperty("message"))
        .identical(CIMProperty(CIMName("message"), String("Hello World"), 0,
            CIMName(), CIMName(), true)));

    namedInstances = r.enumerateInstancesForClass(NS, CIMName("SubClass"));

    PEGASUS_TEST_ASSERT(namedInstances.size() == 1);

    //XmlWriter::printInstanceElement(namedInstances[0], cout);
    //XmlWriter::printInstanceElement(inst1, cout);

    PEGASUS_TEST_ASSERT(namedInstances[0].getPropertyCount() == 5);
    PEGASUS_TEST_ASSERT(
        namedInstances[0].getProperty(namedInstances[0].findProperty("key"))
        .identical(CIMProperty(CIMName("key"), Uint32(222))));
    PEGASUS_TEST_ASSERT(
        namedInstances[0].getProperty(namedInstances[0].findProperty("ratio"))
        .identical(CIMProperty(CIMName("ratio"), Real32(1.5), 0, CIMName(),
            CIMName(), true)));
    PEGASUS_TEST_ASSERT(
        namedInstances[0].getProperty(namedInstances[0].findProperty("message"))
        .identical(CIMProperty(CIMName("message"), String("Hello World"), 0,
            CIMName(), CIMName(), true)));
    PEGASUS_TEST_ASSERT(
        namedInstances[0].getProperty(namedInstances[0].findProperty("junk"))
        .identical(CIMProperty(CIMName("junk"), Real32(66.66), 0, CIMName(),
            CIMName(), true)));
    PEGASUS_TEST_ASSERT(
        namedInstances[0].getProperty(namedInstances[0].findProperty(
            "newProperty")).identical(
                CIMProperty(CIMName("newProperty"), Uint32(888), 0, CIMName(),
                    CIMName(), true)));

    //
    // Test enumerating with classOrigin false
    //

    namedInstances = r.enumerateInstancesForSubtree(NS,
        CIMName ("SuperClass"), true, false);

    PEGASUS_TEST_ASSERT(namedInstances.size() == 2);

    for (Uint32 i = 0 ; i < namedInstances.size() ; i++)
    {
        // Check all properties for classorigin
        for (Uint32 j = 0 ; j < namedInstances[i].getPropertyCount() ; j++)
        {
            CIMProperty p = namedInstances[i].getProperty(j);
        }

    }

    //
    // Repeat the above for enumerateinstancesForClass
    //
    namedInstances = r.enumerateInstancesForClass(NS,
        CIMName("SuperClass"), true, false);

    PEGASUS_TEST_ASSERT(namedInstances.size() == 1);

    for (Uint32 i = 0 ; i < namedInstances.size() ; i++)
    {
        // Check all properties for classorigin
        for (Uint32 j = 0 ; j < namedInstances[i].getPropertyCount() ; j++)
        {
            CIMProperty p = namedInstances[i].getProperty(j);
        }
    }

    //
    // Test for qualifier removal from enumerateinstances
    //
    namedInstances = r.enumerateInstancesForSubtree(NS,
        CIMName ("SuperClass"), false, false);

    PEGASUS_TEST_ASSERT(namedInstances.size() == 2);

    for (Uint32 i = 0 ; i < namedInstances.size() ; i++)
    {
        PEGASUS_TEST_ASSERT(namedInstances[i].getQualifierCount() == 0);
        // Check all properties for qualifiers
        for (Uint32 j = 0 ; j < namedInstances[i].getPropertyCount() ; j++)
        {
            CIMProperty p = namedInstances[i].getProperty(j);
            PEGASUS_TEST_ASSERT(p.getQualifierCount() == 0);
        }
    }

    //
    // Repeat the above for the enumerateinstancesFor Class
    //
    namedInstances = r.enumerateInstancesForClass(NS,
        CIMName("SuperClass"), false, false);

    PEGASUS_TEST_ASSERT(namedInstances.size() == 1);

    for (Uint32 i = 0 ; i < namedInstances.size() ; i++)
    {
        PEGASUS_TEST_ASSERT(namedInstances[i].getQualifierCount() == 0);
        // Check all properties for qualifiers
        for (Uint32 j = 0 ; j < namedInstances[i].getPropertyCount() ; j++)
        {
            CIMProperty p = namedInstances[i].getProperty(j);
            PEGASUS_TEST_ASSERT(p.getQualifierCount() == 0);
        }

    }

    // *******************************************************************
    //
    // Test for property filtering
    //
    // *******************************************************************

    //
    // test with empty property list
    //
    CIMPropertyList pl1(pls_empty);

    // deepInheritance=true,
    // includeQualifiers=false,includeClassOrigin=false,
    namedInstances = r.enumerateInstancesForSubtree(NS,
        CIMName ("SuperClass"), false, false, pl1);

    PEGASUS_TEST_ASSERT(namedInstances.size() == 2);

    for (Uint32 i = 0 ; i < namedInstances.size() ; i++)
    {
        // Check all properties for qualifiers
        for (Uint32 j = 0 ; j < namedInstances[i].getPropertyCount() ; j++)
        {
            PEGASUS_TEST_ASSERT(
                namedInstances[i].findProperty("key") == PEG_NOT_FOUND);
            PEGASUS_TEST_ASSERT(
                namedInstances[i].findProperty("ratio") == PEG_NOT_FOUND);
            PEGASUS_TEST_ASSERT(
                namedInstances[i].findProperty("message") == PEG_NOT_FOUND);
            PEGASUS_TEST_ASSERT(namedInstances[i].getPropertyCount() == 0);
        }
    }


    //
    // test with property list with property "ratio"
    //
    Array<CIMName> pls1;
    pls1.append(CIMName("ratio"));
    pl1.clear();
    pl1.set(pls1);

    namedInstances = r.enumerateInstancesForSubtree(NS,
        CIMName ("SuperClass"), false, false, pl1);

    PEGASUS_TEST_ASSERT(namedInstances.size() == 2);

    for (Uint32 i = 0 ; i < namedInstances.size() ; i++)
    {
        // Check all properties for qualifiers
        for (Uint32 j = 0 ; j < namedInstances[i].getPropertyCount() ; j++)
        {
            PEGASUS_TEST_ASSERT(
                namedInstances[i].findProperty("key") == PEG_NOT_FOUND);
            PEGASUS_TEST_ASSERT(
                namedInstances[i].findProperty("ratio") != PEG_NOT_FOUND);
            PEGASUS_TEST_ASSERT(
                namedInstances[i].findProperty("message") == PEG_NOT_FOUND);
            PEGASUS_TEST_ASSERT(
                namedInstances[i].getPropertyCount() == 1);
        }
    }


    //
    // retest with two entries in the list.
    //
    pls1.append(CIMName("message"));
    pl1.clear();
    pl1.set(pls1);
    // deepInheritance=true,
    // includeQualifiers=false,includeClassOrigin=false,
    namedInstances = r.enumerateInstancesForSubtree(NS,
        CIMName ("SuperClass"), false, false, pl1);

    PEGASUS_TEST_ASSERT(namedInstances.size() == 2);

    for (Uint32 i = 0 ; i < namedInstances.size() ; i++)
    {
        // Check all properties for qualifiers
        for (Uint32 j = 0 ; j < namedInstances[i].getPropertyCount() ; j++)
        {
            PEGASUS_TEST_ASSERT(
                namedInstances[i].findProperty("key") == PEG_NOT_FOUND);
            PEGASUS_TEST_ASSERT(
                namedInstances[i].findProperty("ratio") != PEG_NOT_FOUND);
            PEGASUS_TEST_ASSERT(
                namedInstances[i].findProperty("message") != PEG_NOT_FOUND);
            PEGASUS_TEST_ASSERT(
                namedInstances[i].getPropertyCount() == 2);
        }
    }


    //
    // Test with an invalid property in the list. It should be ignored
    // and the results should be identical to the previous.
    //
    pls1.append(CIMName("herroyalhighnessofyork"));
    pl1.clear();
    pl1.set(pls1);

    namedInstances = r.enumerateInstancesForSubtree(NS,
        CIMName ("SuperClass"), false, false, pl1);

    PEGASUS_TEST_ASSERT(namedInstances.size() == 2);

    for (Uint32 i = 0 ; i < namedInstances.size() ; i++)
    {
        // Check all properties for qualifiers
        for (Uint32 j = 0 ; j < namedInstances[i].getPropertyCount() ; j++)
        {
            PEGASUS_TEST_ASSERT(
                namedInstances[i].findProperty("key") == PEG_NOT_FOUND);
            PEGASUS_TEST_ASSERT(
                namedInstances[i].findProperty("ratio") != PEG_NOT_FOUND);
            PEGASUS_TEST_ASSERT(
                namedInstances[i].findProperty("message") != PEG_NOT_FOUND);
            PEGASUS_TEST_ASSERT(
                namedInstances[i].getPropertyCount() == 2);
        }
    }


    //
    // -- Modify one of the instances:
    //

    CIMInstance premodifyInst = r.getInstance(NS, instanceNames[0], true, true);
    premodifyInst.setPath(instanceNames[0]);

    CIMInstance modifiedInst0(CIMName ("SuperClass"));
    modifiedInst0.addProperty(CIMProperty(CIMName ("key"), Uint32(111)));
    modifiedInst0.addProperty(
        CIMProperty(CIMName ("message"), String("Goodbye World")));
    modifiedInst0.setPath (instanceNames[0]);
    r.modifyInstance(NS, modifiedInst0);

    //
    // -- Get instance back and see that it is the same as modified one:
    //
    CIMInstance tmpInstance = r.getInstance(NS, instanceNames[0], true, true);
    tmpInstance.setPath(instanceNames[0]);
    //XmlWriter::printInstanceElement(premodifyInst, cout);
    //XmlWriter::printInstanceElement(tmpInstance, cout);

    PEGASUS_TEST_ASSERT(
        tmpInstance.getProperty(tmpInstance.findProperty("message")).getValue()
            == String("Goodbye World"));
    PEGASUS_TEST_ASSERT(
        tmpInstance.getProperty(tmpInstance.findProperty("ratio")).identical(
            premodifyInst.getProperty(premodifyInst.findProperty("ratio"))));
    PEGASUS_TEST_ASSERT(
        tmpInstance.getProperty(tmpInstance.findProperty("key")).identical(
            premodifyInst.getProperty(premodifyInst.findProperty("key"))));

    //
    // -- Now modify the "message" property:
    //
    CIMValue messageValue = r.getProperty(NS, CIMObjectPath
        ("SuperClass.key=111"), CIMName ("message"));
    String message;
    messageValue.get(message);
    PEGASUS_TEST_ASSERT(message == "Goodbye World");

    r.setProperty(NS, CIMObjectPath ("SuperClass.key=111"), CIMName ("message"),
        CIMValue(String("Hello World")));

    messageValue = r.getProperty( NS, CIMObjectPath ("SuperClass.key=111"),
        CIMName ("message"));
    messageValue.get(message);
    PEGASUS_TEST_ASSERT(message == "Hello World");

    //
    // Future test -  modify key property and attempt to write
    // TODO

    //
    // -- Attempt to modify a key property:
    //
    Boolean failed = false;

    try
    {
    r.setProperty(NS, CIMObjectPath ("SuperClass.key=111"), CIMName ("key"),
            Uint32(999));
    }
    catch (CIMException& e)
    {
    PEGASUS_TEST_ASSERT(e.getCode() == CIM_ERR_FAILED);
    failed = true;
    }

    PEGASUS_TEST_ASSERT(failed);

    //
    // -- Delete the instances:
    //
    r.deleteInstance(NS, CIMObjectPath ("SuperClass.key=111"));
    r.deleteInstance(NS, CIMObjectPath ("SubClass.key=222"));

    //
    // -- Clean up classes:
    //
    r.deleteClass(NS, CIMName ("SubClass"));
    r.deleteClass(NS, CIMName ("SuperClass"));

    //
    // -- Delete the qualifier:
    //
    r.deleteQualifier(NS, CIMName ("key"));
    r.deleteQualifier(NS, CIMName ("description"));
    r.deleteQualifier(NS, CIMName ("junk"));

    r.deleteNameSpace(NS);
}

void TestModifyClass(Uint32 mode)
{
    //
    // -- Create repository and test namespace:
    //
    CIMRepository r(repositoryRoot, mode);
    const CIMNamespaceName NS = CIMNamespaceName("TestModifyClass");

    r.createNameSpace(NS);

    // Test NOT_FOUND error when modifying a non-existent class

    CIMClass c("PG_NonExistent", "PG_NonExistentParent");
    try
    {
        r.modifyClass(NS, c);
        // Should not get here.
        PEGASUS_TEST_ASSERT(false);
    }
    catch (CIMException& e)
    {
        PEGASUS_TEST_ASSERT(e.getCode() == CIM_ERR_NOT_FOUND);
    }
}

void TestQualifiers(Uint32 mode)
{
  //
  // -- Create repository and "xyz" namespace:
  //
  CIMRepository r (repositoryRoot, mode);

    const CIMNamespaceName NS = CIMNamespaceName ("TestQualifiers");

    try
    {
    r.createNameSpace(NS);
    }
    catch (AlreadyExistsException&)
    {
    // Ignore this!
    }

    //
    // -- Construct a qualifier declaration:
    //
    CIMQualifierDecl q(CIMName ("abstract"), true, CIMScope::CLASS);
    r.setQualifier(NS, q);

    CIMQualifierDecl qq = r.getQualifier(NS, CIMName ("abstract"));

    PEGASUS_TEST_ASSERT(qq.identical(q));
    PEGASUS_TEST_ASSERT(q.identical(qq));

    //
    // -- Delete the qualifier:
    //
    r.deleteQualifier(NS, CIMName ("abstract"));

    //
    // -- Delete the namespace:
    //
    r.deleteNameSpace(NS);
}


int main(int argc, char** argv)
{
    verbose = getenv("PEGASUS_TEST_VERBOSE") ? true : false;

    if (argc != 2)
    {
        cout << "Usage: " << argv[0] << " XML | BIN" << endl;
        return 1;
    }

    const char* tmpDir = getenv ("PEGASUS_TMP");
    if (tmpDir == NULL)
    {
        repositoryRoot = ".";
    }
    else
    {
        repositoryRoot = tmpDir;
    }
    repositoryRoot.append("/repository");

    FileSystem::removeDirectoryHier(repositoryRoot);

    try
    {
      Uint32 mode;
      if (!strcmp(argv[1],"XML") )
    {
      mode = CIMRepository::MODE_XML;
      if (verbose) cout << argv[0]<< ": using XML mode repository" << endl;
    }
      else if (!strcmp(argv[1],"BIN") )
    {
      mode = CIMRepository::MODE_BIN;
      if (verbose) cout << argv[0]<< ": using BIN mode repository" << endl;
    }
      else
    {
      cout << argv[0] << ": invalid argument: " << argv[1] << endl;
      return 1;
    }

    TestNameSpaces(mode);
    TestCreateClass(mode);
    TestModifyClass(mode);
    TestQualifiers(mode);

    }
    catch (Exception& e)
    {
    cout << argv[0] << " " << argv[1] << " " << e.getMessage() << endl;
    exit(1);
    }

    FileSystem::removeDirectoryHier(repositoryRoot);

    cout << argv[0] << " +++++ passed all tests" << endl;

    return 0;
}
