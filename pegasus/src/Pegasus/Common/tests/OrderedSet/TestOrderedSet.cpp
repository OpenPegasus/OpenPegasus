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
#include <Pegasus/Common/CIMName.h>
#include <Pegasus/Common/CharSet.h>
#include <Pegasus/Common/OrderedSet.h>
#include <Pegasus/Common/CIMProperty.h>
#include <Pegasus/Common/CIMPropertyRep.h>
#include <Pegasus/Common/CIMQualifier.h>
#include <Pegasus/Common/CIMQualifierRep.h>
#include <Pegasus/Common/CIMParameter.h>
#include <Pegasus/Common/CIMParameterRep.h>
#include <Pegasus/Common/CIMMethod.h>
#include <Pegasus/Common/CIMMethodRep.h>

#include <Pegasus/Common/PegasusAssert.h>

/* needs to be included here as the arrays are invisible externally */
/* this include should not be copied */
#include <Pegasus/Common/CharSet.cpp>

PEGASUS_USING_PEGASUS;
PEGASUS_USING_STD;

static Boolean verbose;

#define VCOUT if (verbose) cout

void testOrderedSetCIMProperty()
{
    VCOUT << "Test OrderedSet<CIMProperty, CIMPropertyRep, "
              << PEGASUS_PROPERTY_ORDEREDSET_HASHSIZE << "> ...";

    typedef OrderedSet<CIMProperty,
                       CIMPropertyRep,
                       PEGASUS_PROPERTY_ORDEREDSET_HASHSIZE> Set;

    // Create a few test properties and test CIMNames
    CIMName cimNameSearchProperty("myLookUpProperty");

    // 31E3 -> manually calculated, entangled 'm' and 'y'
    // after transform to upper case
    // M = 4D = 0100 1101
    // Y = 59 = 0101 1001
    // entangled:
    // 00 11 00 01  11 10 00 11
    //   3     1      E     3
    Uint32 cimNameSearchPropertyTag=0x31E3;

    CIMName cimNameAProperty1("AProperty1");
    CIMName cimNameAProperty2("AProperty2");
    CIMName cimNameAProperty3("AProperty3");
    CIMName cimNameAProperty4("AProperty4");
    CIMName cimNameBProperty1("BProperty1");
    CIMName cimNameBProperty2("BProperty2");
    CIMName cimNameBProperty3("BProperty3");
    CIMName cimNameBProperty4("BProperty4");

    CIMProperty blankProperty;
    CIMProperty searchProperty(cimNameSearchProperty, CIMValue(Uint32(1)));
    CIMProperty aProperty1(cimNameAProperty1, CIMValue(Boolean(true)));
    CIMProperty aProperty2(cimNameAProperty2, CIMValue(Boolean(true)));
    CIMProperty aProperty3(cimNameAProperty3, CIMValue(Boolean(true)));
    CIMProperty aProperty4(cimNameAProperty4, CIMValue(Boolean(true)));
    CIMProperty bProperty1(cimNameBProperty1, CIMValue(Boolean(false)));
    CIMProperty bProperty2(cimNameBProperty2, CIMValue(Boolean(false)));
    CIMProperty bProperty3(cimNameBProperty3, CIMValue(Boolean(false)));
    CIMProperty bProperty4(cimNameBProperty4, CIMValue(Boolean(false)));

    // Test default constructor
    Set properties;
    PEGASUS_TEST_ASSERT(0 == properties.size());

    // reserve capacity for a few properties
    properties.reserveCapacity(16);

    // append all our test properties (test append)
    properties.append(searchProperty);
    properties.append(aProperty2);
    properties.append(bProperty2);
    properties.append(aProperty1);
    properties.append(bProperty4);
    properties.append(aProperty3);
    properties.append(aProperty4);
    properties.append(bProperty1);
    properties.append(bProperty3);

    PEGASUS_TEST_ASSERT(9 == properties.size());

    // clear()
    properties.clear();
    PEGASUS_TEST_ASSERT(0 == properties.size());
    // reappend all our test properties (test append)
    properties.append(aProperty1);
    properties.append(aProperty2);
    properties.append(aProperty3);
    properties.append(aProperty4);
    properties.append(searchProperty);
    properties.append(bProperty1);
    properties.append(bProperty2);
    properties.append(bProperty3);
    properties.append(bProperty4);

    PEGASUS_TEST_ASSERT(9 == properties.size());


    // lookup the test property
    // this also tests the inline generateCIMNameTag() function in OrderedSet.h
    Uint32 pos = properties.find(cimNameSearchProperty,
                                 cimNameSearchPropertyTag);
    PEGASUS_TEST_ASSERT(4 == pos);

    // operator[] test
    CIMProperty bProp1 = properties[5];
    PEGASUS_TEST_ASSERT(bProp1.identical(bProperty1));

    // const operator[] test
    const CIMProperty bConstProp1 = properties[5];
    PEGASUS_TEST_ASSERT(bConstProp1.identical(bProperty1));

    // remove() test
    properties.remove(0);
    PEGASUS_TEST_ASSERT(8 == properties.size());
    properties.remove(2);
    PEGASUS_TEST_ASSERT(7 == properties.size());
    properties.remove(6);
    PEGASUS_TEST_ASSERT(6 == properties.size());

    // insert() test
    properties.insert(6, bProperty4);
    PEGASUS_TEST_ASSERT(7 == properties.size());
    properties.insert(2, aProperty4);
    PEGASUS_TEST_ASSERT(8 == properties.size());
    properties.insert(0, aProperty1);
    PEGASUS_TEST_ASSERT(9 == properties.size());


    // Out of Bounds test for remove and insert
    try
    {
        properties.remove(100);
    }
    catch(IndexOutOfBoundsException&)
    {
        PEGASUS_TEST_ASSERT(9 == properties.size());
        try
        {
            properties.insert(100, blankProperty);
        }
        catch(IndexOutOfBoundsException&)
        {
            PEGASUS_TEST_ASSERT(9 == properties.size());
            VCOUT << " +++++ passed" << endl;
            return;
        }
    }
    // test failed if not caught an out of bounds exception
    PEGASUS_TEST_ASSERT(false);
}

void testOrderedSetCIMQualifier()
{
    VCOUT << "Test OrderedSet<CIMQualifier, CIMQualifierRep, "
              << PEGASUS_QUALIFIER_ORDEREDSET_HASHSIZE << "> ...";

    typedef OrderedSet<CIMQualifier,
                       CIMQualifierRep,
                       PEGASUS_QUALIFIER_ORDEREDSET_HASHSIZE> Set;

    // Create a few test properties and test CIMNames
    CIMName cimNameSearchQualifier("myLookUpQualifier");

    // 31A6 -> manually calculated, entangled 'm' and 'r'
    // after transform to upper case
    // M = 4D = 0100 1101
    // R = 52 = 0101 0010
    // entangled:
    // 00 11 00 01  10 10 01 10
    //   3     1      A     6
    Uint32 cimNameSearchQualifierTag=0x31A6;

    CIMName cimNameAQualifier1("AQualifier1");
    CIMName cimNameAQualifier2("AQualifier2");
    CIMName cimNameAQualifier3("AQualifier3");
    CIMName cimNameAQualifier4("AQualifier4");
    CIMName cimNameBQualifier1("BQualifier1");
    CIMName cimNameBQualifier2("BQualifier2");
    CIMName cimNameBQualifier3("BQualifier3");
    CIMName cimNameBQualifier4("BQualifier4");

    CIMQualifier blankQualifier;
    CIMQualifier searchQualifier(cimNameSearchQualifier, CIMValue(Uint32(1)));
    CIMQualifier aQualifier1(cimNameAQualifier1, CIMValue(Boolean(true)));
    CIMQualifier aQualifier2(cimNameAQualifier2, CIMValue(Boolean(true)));
    CIMQualifier aQualifier3(cimNameAQualifier3, CIMValue(Boolean(true)));
    CIMQualifier aQualifier4(cimNameAQualifier4, CIMValue(Boolean(true)));
    CIMQualifier bQualifier1(cimNameBQualifier1, CIMValue(Boolean(false)));
    CIMQualifier bQualifier2(cimNameBQualifier2, CIMValue(Boolean(false)));
    CIMQualifier bQualifier3(cimNameBQualifier3, CIMValue(Boolean(false)));
    CIMQualifier bQualifier4(cimNameBQualifier4, CIMValue(Boolean(false)));

    // Test default constructor
    Set qualifiers;
    PEGASUS_TEST_ASSERT(0 == qualifiers.size());

    // reserve capacity for a few qualifiers
    qualifiers.reserveCapacity(16);

    // append all our test qualifiers (test append)
    qualifiers.append(aQualifier1);
    qualifiers.append(aQualifier2);
    qualifiers.append(aQualifier3);
    qualifiers.append(aQualifier4);
    qualifiers.append(searchQualifier);
    qualifiers.append(bQualifier1);
    qualifiers.append(bQualifier2);
    qualifiers.append(bQualifier3);
    qualifiers.append(bQualifier4);

    PEGASUS_TEST_ASSERT(9 == qualifiers.size());

    // clear()
    qualifiers.clear();
    PEGASUS_TEST_ASSERT(0 == qualifiers.size());
    // reappend all our test qualifiers (test append)
    qualifiers.append(aQualifier1);
    qualifiers.append(aQualifier2);
    qualifiers.append(aQualifier3);
    qualifiers.append(aQualifier4);
    qualifiers.append(searchQualifier);
    qualifiers.append(bQualifier1);
    qualifiers.append(bQualifier2);
    qualifiers.append(bQualifier3);
    qualifiers.append(bQualifier4);

    PEGASUS_TEST_ASSERT(9 == qualifiers.size());

    // lookup the test qualifier
    // this also tests the inline generateCIMNameTag()
    // function in CIMNameInline.h
    Uint32 pos = qualifiers.find(cimNameSearchQualifier,
                                 cimNameSearchQualifierTag);

    PEGASUS_TEST_ASSERT(4 == pos);

    // operator[] test
    CIMQualifier bQual1 = qualifiers[5];
    PEGASUS_TEST_ASSERT(bQual1.identical(bQualifier1));

    // const operator[] test
    const CIMQualifier bConstQual1 = qualifiers[5];
    PEGASUS_TEST_ASSERT(bConstQual1.identical(bQualifier1));

    // remove() test
    qualifiers.remove(0);
    PEGASUS_TEST_ASSERT(8 == qualifiers.size());
    qualifiers.remove(2);
    PEGASUS_TEST_ASSERT(7 == qualifiers.size());
    qualifiers.remove(6);
    PEGASUS_TEST_ASSERT(6 == qualifiers.size());

    // insert() test
    qualifiers.insert(6, bQualifier4);
    PEGASUS_TEST_ASSERT(7 == qualifiers.size());
    qualifiers.insert(2, aQualifier4);
    PEGASUS_TEST_ASSERT(8 == qualifiers.size());
    qualifiers.insert(0, aQualifier1);
    PEGASUS_TEST_ASSERT(9 == qualifiers.size());

    // add a key qualifier
    CIMQualifier myKeyQualifier("keyboard", CIMValue(Boolean(true)));
    qualifiers.append(myKeyQualifier);
    PEGASUS_TEST_ASSERT(10 == qualifiers.size());

    // need some more qualifiers for the next tests

    CIMName cimNameCQualifier1("CQualifier1");
    CIMName cimNameCQualifier2("CQualifier2");
    CIMName cimNameCQualifier3("CQualifier3");
    CIMName cimNameCQualifier4("CQualifier4");
    CIMName cimNameDQualifier1("DQualifier1");
    CIMName cimNameDQualifier2("DQualifier2");
    CIMName cimNameDQualifier3("DQualifier3");
    CIMName cimNameDQualifier4("DQualifier4");
    CIMName cimNameCQualifier5("CQualfer1");
    CIMName cimNameCQualifier6("CQualfer2");
    CIMName cimNameCQualifier7("CQualfer3");
    CIMName cimNameCQualifier8("CQualfer4");
    CIMName cimNameDQualifier5("DQualfer1");
    CIMName cimNameDQualifier6("DQualfer2");
    CIMName cimNameDQualifier7("DQualfer3");
    CIMName cimNameDQualifier8("DQualfer4");

    CIMQualifier cQualifier1(cimNameCQualifier1, CIMValue(Uint32(1)));
    CIMQualifier cQualifier2(cimNameCQualifier2, CIMValue(Uint32(2)));
    CIMQualifier cQualifier3(cimNameCQualifier3, CIMValue(Uint32(3)));
    CIMQualifier cQualifier4(cimNameCQualifier4, CIMValue(Uint32(4)));
    CIMQualifier dQualifier1(cimNameDQualifier1, CIMValue(Uint32(5)));
    CIMQualifier dQualifier2(cimNameDQualifier2, CIMValue(Uint32(6)));
    CIMQualifier dQualifier3(cimNameDQualifier3, CIMValue(Uint32(7)));
    CIMQualifier dQualifier4(cimNameDQualifier4, CIMValue(Uint32(8)));

    CIMQualifier cQualifier5(cimNameCQualifier5, CIMValue(Uint32(10)));
    CIMQualifier cQualifier6(cimNameCQualifier6, CIMValue(Uint32(20)));
    CIMQualifier cQualifier7(cimNameCQualifier7, CIMValue(Uint32(30)));
    CIMQualifier cQualifier8(cimNameCQualifier8, CIMValue(Uint32(40)));
    CIMQualifier dQualifier5(cimNameDQualifier5, CIMValue(Uint32(50)));
    CIMQualifier dQualifier6(cimNameDQualifier6, CIMValue(Uint32(60)));
    CIMQualifier dQualifier7(cimNameDQualifier7, CIMValue(Uint32(70)));
    CIMQualifier dQualifier8(cimNameDQualifier8, CIMValue(Uint32(80)));

    // try to break the OrderedSet by adding more than 16 qualifiers to it
    qualifiers.append(cQualifier1);
    qualifiers.append(cQualifier2);
    qualifiers.append(cQualifier3);
    qualifiers.append(cQualifier4);
    qualifiers.append(dQualifier1);
    qualifiers.append(dQualifier2);
    qualifiers.append(dQualifier3);
    qualifiers.append(dQualifier4);

    PEGASUS_TEST_ASSERT(18 == qualifiers.size());

    qualifiers.append(cQualifier5);
    qualifiers.append(cQualifier6);
    qualifiers.append(cQualifier7);
    qualifiers.append(cQualifier8);
    qualifiers.append(dQualifier5);
    qualifiers.append(dQualifier6);
    qualifiers.append(dQualifier7);
    qualifiers.append(dQualifier8);

    PEGASUS_TEST_ASSERT(26 == qualifiers.size());
    // ensure secondary names are found
    // (meaning names with same generateCIMNameTag)
    //
    // 2521 -> manually calculated, entangled 'D' and '1'
    // after transform to upper case
    // D = 44 = 0100 0100
    // 1 = 31 = 0011 0001
    // entangled:
    // 00 10 01 01  00 10 00 01
    //   2     5      2     1
    Uint32 secPos = qualifiers.find(cimNameDQualifier5, 0x2521);

    PEGASUS_TEST_ASSERT(22 == secPos);

    // Out of Bounds test for remove and insert
    try
    {
        qualifiers.remove(100);
    }
    catch(IndexOutOfBoundsException&)
    {
        PEGASUS_TEST_ASSERT(26 == qualifiers.size());
        try
        {
            qualifiers.insert(100, blankQualifier);
        }
        catch(IndexOutOfBoundsException&)
        {
            PEGASUS_TEST_ASSERT(26 == qualifiers.size());
            VCOUT << " +++++ passed" << endl;
            return;
        }
    }
    // test failed if not caught an out of bounds exception
    PEGASUS_TEST_ASSERT(false);
}

void testOrderedSetCIMParameter()
{
    VCOUT << "Test OrderedSet<CIMParameter, CIMParameterRep, "
              << PEGASUS_PARAMETER_ORDEREDSET_HASHSIZE << "> ...";

    typedef OrderedSet<CIMParameter,
                       CIMParameterRep,
                       PEGASUS_PARAMETER_ORDEREDSET_HASHSIZE> Set;

    // Create a few test properties and test CIMNames
    CIMName cimNameSearchParameter("myLookUpParameter");

    // 31A6 -> manually calculated, entangled 'm' and 'r'
    // after transform to upper case
    // M = 4D = 0100 1101
    // R = 52 = 0101 0010
    // entangled:
    // 00 11 00 01  10 10 01 10
    //   3     1      A     6
    Uint32 cimNameSearchParameterTag=0x31A6;

    CIMName cimNameAParameter1("AParameter1");
    CIMName cimNameAParameter2("AParameter2");
    CIMName cimNameAParameter3("AParameter3");
    CIMName cimNameAParameter4("AParameter4");
    CIMName cimNameBParameter1("BParameter1");
    CIMName cimNameBParameter2("BParameter2");
    CIMName cimNameBParameter3("BParameter3");
    CIMName cimNameBParameter4("BParameter4");

    CIMParameter blankParameter;
    CIMParameter searchParameter(cimNameSearchParameter, CIMTYPE_UINT32);
    CIMParameter aParameter1(cimNameAParameter1, CIMTYPE_BOOLEAN);
    CIMParameter aParameter2(cimNameAParameter2, CIMTYPE_BOOLEAN);
    CIMParameter aParameter3(cimNameAParameter3, CIMTYPE_BOOLEAN);
    CIMParameter aParameter4(cimNameAParameter4, CIMTYPE_BOOLEAN);
    CIMParameter bParameter1(cimNameBParameter1, CIMTYPE_BOOLEAN);
    CIMParameter bParameter2(cimNameBParameter2, CIMTYPE_BOOLEAN);
    CIMParameter bParameter3(cimNameBParameter3, CIMTYPE_BOOLEAN);
    CIMParameter bParameter4(cimNameBParameter4, CIMTYPE_BOOLEAN);

    // Test default constructor
    Set parameters;
    PEGASUS_TEST_ASSERT(0 == parameters.size());

    // reserve capacity for a few parameters
    parameters.reserveCapacity(16);

    // append all our test parameters (test append)
    parameters.append(aParameter1);
    parameters.append(aParameter2);
    parameters.append(aParameter3);
    parameters.append(aParameter4);
    parameters.append(searchParameter);
    parameters.append(bParameter1);
    parameters.append(bParameter2);
    parameters.append(bParameter3);
    parameters.append(bParameter4);

    PEGASUS_TEST_ASSERT(9 == parameters.size());

    // clear()
    parameters.clear();
    PEGASUS_TEST_ASSERT(0 == parameters.size());
    // reappend all our test parameters (test append)
    parameters.append(aParameter1);
    parameters.append(aParameter2);
    parameters.append(aParameter3);
    parameters.append(aParameter4);
    parameters.append(searchParameter);
    parameters.append(bParameter1);
    parameters.append(bParameter2);
    parameters.append(bParameter3);
    parameters.append(bParameter4);

    PEGASUS_TEST_ASSERT(9 == parameters.size());

    // lookup the test parameter
    // this also tests the inline generateCIMNameTag()
    // function in CIMNameInline.h
    Uint32 pos = parameters.find(cimNameSearchParameter,
                                 cimNameSearchParameterTag);

    PEGASUS_TEST_ASSERT(4 == pos);

    // operator[] test
    CIMParameter bParam1 = parameters[5];
    PEGASUS_TEST_ASSERT(bParam1.identical(bParameter1));

    // const operator[] test
    const CIMParameter bConstParam1 = parameters[5];
    PEGASUS_TEST_ASSERT(bConstParam1.identical(bParameter1));

    // remove() test
    parameters.remove(0);
    PEGASUS_TEST_ASSERT(8 == parameters.size());
    parameters.remove(2);
    PEGASUS_TEST_ASSERT(7 == parameters.size());
    parameters.remove(6);
    PEGASUS_TEST_ASSERT(6 == parameters.size());

    // insert() test
    parameters.insert(6, bParameter4);
    PEGASUS_TEST_ASSERT(7 == parameters.size());
    parameters.insert(2, aParameter4);
    PEGASUS_TEST_ASSERT(8 == parameters.size());
    parameters.insert(0, aParameter1);
    PEGASUS_TEST_ASSERT(9 == parameters.size());

    // add a keyboard
    CIMParameter myKeyParameter("keyboard", CIMTYPE_BOOLEAN);
    parameters.append(myKeyParameter);
    PEGASUS_TEST_ASSERT(10 == parameters.size());

    // need some more parameters for the next tests

    CIMName cimNameCParameter1("CParameter1");
    CIMName cimNameCParameter2("CParameter2");
    CIMName cimNameCParameter3("CParameter3");
    CIMName cimNameCParameter4("CParameter4");
    CIMName cimNameDParameter1("DParameter1");
    CIMName cimNameDParameter2("DParameter2");
    CIMName cimNameDParameter3("DParameter3");
    CIMName cimNameDParameter4("DParameter4");
    CIMName cimNameCParameter5("CParamer1");
    CIMName cimNameCParameter6("CParamer2");
    CIMName cimNameCParameter7("CParamer3");
    CIMName cimNameCParameter8("CParamer4");
    CIMName cimNameDParameter5("DParamer1");
    CIMName cimNameDParameter6("DParamer2");
    CIMName cimNameDParameter7("DParamer3");
    CIMName cimNameDParameter8("DParamer4");

    CIMParameter cParameter1(cimNameCParameter1, CIMTYPE_UINT32);
    CIMParameter cParameter2(cimNameCParameter2, CIMTYPE_UINT32);
    CIMParameter cParameter3(cimNameCParameter3, CIMTYPE_UINT32);
    CIMParameter cParameter4(cimNameCParameter4, CIMTYPE_UINT32);
    CIMParameter dParameter1(cimNameDParameter1, CIMTYPE_UINT32);
    CIMParameter dParameter2(cimNameDParameter2, CIMTYPE_UINT32);
    CIMParameter dParameter3(cimNameDParameter3, CIMTYPE_UINT32);
    CIMParameter dParameter4(cimNameDParameter4, CIMTYPE_UINT32);

    CIMParameter cParameter5(cimNameCParameter5, CIMTYPE_UINT32);
    CIMParameter cParameter6(cimNameCParameter6, CIMTYPE_UINT32);
    CIMParameter cParameter7(cimNameCParameter7, CIMTYPE_UINT32);
    CIMParameter cParameter8(cimNameCParameter8, CIMTYPE_UINT32);
    CIMParameter dParameter5(cimNameDParameter5, CIMTYPE_UINT32);
    CIMParameter dParameter6(cimNameDParameter6, CIMTYPE_UINT32);
    CIMParameter dParameter7(cimNameDParameter7, CIMTYPE_UINT32);
    CIMParameter dParameter8(cimNameDParameter8, CIMTYPE_UINT32);

    // try to break the OrderedSet by adding more than 16 parameters to it
    parameters.append(cParameter1);
    parameters.append(cParameter2);
    parameters.append(cParameter3);
    parameters.append(cParameter4);
    parameters.append(dParameter1);
    parameters.append(dParameter2);
    parameters.append(dParameter3);
    parameters.append(dParameter4);

    PEGASUS_TEST_ASSERT(18 == parameters.size());

    parameters.append(cParameter5);
    parameters.append(cParameter6);
    parameters.append(cParameter7);
    parameters.append(cParameter8);
    parameters.append(dParameter5);
    parameters.append(dParameter6);
    parameters.append(dParameter7);
    parameters.append(dParameter8);

    PEGASUS_TEST_ASSERT(26 == parameters.size());
    // ensure secondary names are found
    // (meaning names with same generateCIMNameTag)
    //
    // 2521 -> manually calculated, entangled 'D' and '1'
    // after transform to upper case
    // D = 44 = 0100 0100
    // 1 = 31 = 0011 0001
    // entangled:
    // 00 10 01 01  00 10 00 01
    //   2     5      2     1
    Uint32 secPos = parameters.find(cimNameDParameter5, 0x2521);

    PEGASUS_TEST_ASSERT(22 == secPos);

    // Out of Bounds test for remove and insert
    try
    {
        parameters.remove(100);
    }
    catch(IndexOutOfBoundsException&)
    {
        PEGASUS_TEST_ASSERT(26 == parameters.size());
        try
        {
            parameters.insert(100, blankParameter);
        }
        catch(IndexOutOfBoundsException&)
        {
            PEGASUS_TEST_ASSERT(26 == parameters.size());
            VCOUT << " +++++ passed" << endl;
            return;
        }
    }
    // test failed if not caught an out of bounds exception
    PEGASUS_TEST_ASSERT(false);
}

void testOrderedSetCIMMethod()
{
    VCOUT << "Test OrderedSet<CIMMethod, CIMMethodRep, "
              << PEGASUS_METHOD_ORDEREDSET_HASHSIZE << "> ...";

    typedef OrderedSet<CIMMethod,
                       CIMMethodRep,
                       PEGASUS_METHOD_ORDEREDSET_HASHSIZE> Set;

    // Create a few test properties and test CIMNames
    CIMName cimNameSearchMethod("myLookUpMethod");

    // 30B2 -> manually calculated, entangled 'm' and 'r'
    // after transform to upper case
    // M = 4D = 0100 1101
    // R = 44 = 0100 0100
    // entangled:
    // 00 11 00 00  10 11 00 10
    //   3     0      B     6
    Uint32 cimNameSearchMethodTag=0x30B2;

    CIMName cimNameAMethod1("AMethod1");
    CIMName cimNameAMethod2("AMethod2");
    CIMName cimNameAMethod3("AMethod3");
    CIMName cimNameAMethod4("AMethod4");
    CIMName cimNameBMethod1("BMethod1");
    CIMName cimNameBMethod2("BMethod2");
    CIMName cimNameBMethod3("BMethod3");
    CIMName cimNameBMethod4("BMethod4");

    CIMMethod blankMethod;
    CIMMethod searchMethod(cimNameSearchMethod, CIMTYPE_UINT32);
    CIMMethod aMethod1(cimNameAMethod1, CIMTYPE_BOOLEAN);
    CIMMethod aMethod2(cimNameAMethod2, CIMTYPE_BOOLEAN);
    CIMMethod aMethod3(cimNameAMethod3, CIMTYPE_BOOLEAN);
    CIMMethod aMethod4(cimNameAMethod4, CIMTYPE_BOOLEAN);
    CIMMethod bMethod1(cimNameBMethod1, CIMTYPE_BOOLEAN);
    CIMMethod bMethod2(cimNameBMethod2, CIMTYPE_BOOLEAN);
    CIMMethod bMethod3(cimNameBMethod3, CIMTYPE_BOOLEAN);
    CIMMethod bMethod4(cimNameBMethod4, CIMTYPE_BOOLEAN);

    // Test default constructor
    Set methods;
    PEGASUS_TEST_ASSERT(0 == methods.size());

    // reserve capacity for a few methods
    methods.reserveCapacity(16);

    // append all our test methods (test append)
    methods.append(aMethod1);
    methods.append(aMethod2);
    methods.append(aMethod3);
    methods.append(aMethod4);
    methods.append(searchMethod);
    methods.append(bMethod1);
    methods.append(bMethod2);
    methods.append(bMethod3);
    methods.append(bMethod4);

    PEGASUS_TEST_ASSERT(9 == methods.size());

    // clear()
    methods.clear();
    PEGASUS_TEST_ASSERT(0 == methods.size());
    // reappend all our test methods (test append)
    methods.append(aMethod1);
    methods.append(aMethod2);
    methods.append(aMethod3);
    methods.append(aMethod4);
    methods.append(searchMethod);
    methods.append(bMethod1);
    methods.append(bMethod2);
    methods.append(bMethod3);
    methods.append(bMethod4);

    PEGASUS_TEST_ASSERT(9 == methods.size());

    // lookup the test method
    // this also tests the inline generateCIMNameTag()
    // function in CIMNameInline.h
    Uint32 pos = methods.find(cimNameSearchMethod,
                                 cimNameSearchMethodTag);

    PEGASUS_TEST_ASSERT(4 == pos);

    // operator[] test
    CIMMethod bMeth1 = methods[5];
    PEGASUS_TEST_ASSERT(bMeth1.identical(bMethod1));

    // const operator[] test
    const CIMMethod bConstMeth1 = methods[5];
    PEGASUS_TEST_ASSERT(bConstMeth1.identical(bMethod1));

    // remove() test
    methods.remove(0);
    PEGASUS_TEST_ASSERT(8 == methods.size());
    methods.remove(2);
    PEGASUS_TEST_ASSERT(7 == methods.size());
    methods.remove(6);
    PEGASUS_TEST_ASSERT(6 == methods.size());

    // insert() test
    methods.insert(6, bMethod4);
    PEGASUS_TEST_ASSERT(7 == methods.size());
    methods.insert(2, aMethod4);
    PEGASUS_TEST_ASSERT(8 == methods.size());
    methods.insert(0, aMethod1);
    PEGASUS_TEST_ASSERT(9 == methods.size());

    // add a keyboard
    CIMMethod myKeyMethod("keyboard", CIMTYPE_BOOLEAN);
    methods.append(myKeyMethod);
    PEGASUS_TEST_ASSERT(10 == methods.size());

    // need some more methods for the next tests

    CIMName cimNameCMethod1("CMethod1");
    CIMName cimNameCMethod2("CMethod2");
    CIMName cimNameCMethod3("CMethod3");
    CIMName cimNameCMethod4("CMethod4");
    CIMName cimNameDMethod1("DMethod1");
    CIMName cimNameDMethod2("DMethod2");
    CIMName cimNameDMethod3("DMethod3");
    CIMName cimNameDMethod4("DMethod4");
    CIMName cimNameCMethod5("CMeth1");
    CIMName cimNameCMethod6("CMeth2");
    CIMName cimNameCMethod7("CMeth3");
    CIMName cimNameCMethod8("CMeth4");
    CIMName cimNameDMethod5("DMeth1");
    CIMName cimNameDMethod6("DMeth2");
    CIMName cimNameDMethod7("DMeth3");
    CIMName cimNameDMethod8("DMeth4");

    CIMMethod cMethod1(cimNameCMethod1, CIMTYPE_UINT32);
    CIMMethod cMethod2(cimNameCMethod2, CIMTYPE_UINT32);
    CIMMethod cMethod3(cimNameCMethod3, CIMTYPE_UINT32);
    CIMMethod cMethod4(cimNameCMethod4, CIMTYPE_UINT32);
    CIMMethod dMethod1(cimNameDMethod1, CIMTYPE_UINT32);
    CIMMethod dMethod2(cimNameDMethod2, CIMTYPE_UINT32);
    CIMMethod dMethod3(cimNameDMethod3, CIMTYPE_UINT32);
    CIMMethod dMethod4(cimNameDMethod4, CIMTYPE_UINT32);

    CIMMethod cMethod5(cimNameCMethod5, CIMTYPE_UINT32);
    CIMMethod cMethod6(cimNameCMethod6, CIMTYPE_UINT32);
    CIMMethod cMethod7(cimNameCMethod7, CIMTYPE_UINT32);
    CIMMethod cMethod8(cimNameCMethod8, CIMTYPE_UINT32);
    CIMMethod dMethod5(cimNameDMethod5, CIMTYPE_UINT32);
    CIMMethod dMethod6(cimNameDMethod6, CIMTYPE_UINT32);
    CIMMethod dMethod7(cimNameDMethod7, CIMTYPE_UINT32);
    CIMMethod dMethod8(cimNameDMethod8, CIMTYPE_UINT32);

    // try to break the OrderedSet by adding more than 16 methods to it
    methods.append(cMethod1);
    methods.append(cMethod2);
    methods.append(cMethod3);
    methods.append(cMethod4);
    methods.append(dMethod1);
    methods.append(dMethod2);
    methods.append(dMethod3);
    methods.append(dMethod4);

    PEGASUS_TEST_ASSERT(18 == methods.size());

    methods.append(cMethod5);
    methods.append(cMethod6);
    methods.append(cMethod7);
    methods.append(cMethod8);
    methods.append(dMethod5);
    methods.append(dMethod6);
    methods.append(dMethod7);
    methods.append(dMethod8);

    PEGASUS_TEST_ASSERT(26 == methods.size());
    // ensure secondary names are found
    // (meaning names with same generateCIMNameTag)
    //
    // 2521 -> manually calculated, entangled 'D' and '1'
    // after transform to upper case
    // D = 44 = 0100 0100
    // 1 = 31 = 0011 0001
    // entangled:
    // 00 10 01 01  00 10 00 01
    //   2     5      2     1
    Uint32 secPos = methods.find(cimNameDMethod5, 0x2521);

    PEGASUS_TEST_ASSERT(22 == secPos);

    // Out of Bounds test for remove and insert
    try
    {
        methods.remove(100);
    }
    catch(IndexOutOfBoundsException&)
    {
        PEGASUS_TEST_ASSERT(26 == methods.size());
        try
        {
            methods.insert(100, blankMethod);
        }
        catch(IndexOutOfBoundsException&)
        {
            PEGASUS_TEST_ASSERT(26 == methods.size());
            VCOUT << " +++++ passed" << endl;
            return;
        }
    }
    // test failed if not caught an out of bounds exception
    PEGASUS_TEST_ASSERT(false);
}

void testOrderedSetPropertyOverFlow()
{
    VCOUT << "Test OrderedSet with 521 Properties";

    typedef OrderedSet<CIMProperty,
                       CIMPropertyRep,
                       PEGASUS_PROPERTY_ORDEREDSET_HASHSIZE> Set;
    Set properties;

    char letter[26] =
        { 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',
          'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z'
        };

    CIMValue oneFitsAll(Uint32(42));

    CIMName cimNameSearchProperty("myLookUpProperty");
    CIMProperty searchProperty(cimNameSearchProperty, oneFitsAll);

    // Add 10 times 26 properties
    for (Uint32 i=0; i<26; i++)
    {
        for (Uint32 j=0; j<10; j++)
        {
            char buf[20];
            sprintf(buf, "%cproperty%u", letter[i], j);
            CIMName newNumPropertyName(buf);
            CIMProperty newNumProperty(newNumPropertyName, oneFitsAll);
            properties.append(newNumProperty);
        }
    }

    // this is the property we are going to lookup later on
    // (index position 260 since index starts at 0)
    properties.append(searchProperty);

    // Add 10 times 26 properties
    for (Uint32 i=0; i<26; i++)
    {
        for (Uint32 j=0; j<10; j++)
        {
            char buf[20];
            sprintf(buf, "%cproperty%u", letter[i], j);
            CIMName newNumPropertyName(buf);
            CIMProperty newNumProperty(newNumPropertyName, oneFitsAll);
            properties.append(newNumProperty);
        }
    }

    // lookup the test property
    // this also tests the inline generateCIMNameTag() function in OrderedSet.h
    Uint32 pos = properties.find(cimNameSearchProperty, 0x31E3);
    PEGASUS_TEST_ASSERT(260 == pos);
    VCOUT << " +++++ passed" << endl;
    return;
}


void testSetPropertyNameContainerException()
{
    VCOUT << "Test Exception on setName() of contained CIMProperty ...";
    typedef OrderedSet<CIMProperty,
                       CIMPropertyRep,
                       PEGASUS_PROPERTY_ORDEREDSET_HASHSIZE> PropertySet;
    PropertySet propertyContainer;
    // Create a test property add to container and change the name
    CIMName nameBefore("before");
    CIMName nameAfter("after");
    CIMProperty testProperty(nameBefore, CIMValue(Uint32(1)));
    propertyContainer.append(testProperty);
    try
    {
        testProperty.setName(nameAfter);
    } catch(Exception &)
    {
        VCOUT << " +++++ passed" << endl;
        return;
    }
    // test failed if not caught an Exception
    PEGASUS_TEST_ASSERT(false);
}

void testSetQualifierNameContainerException()
{
    VCOUT << "Test Exception on setName() of contained CIMQualifier ...";
    typedef OrderedSet<CIMQualifier,
                       CIMQualifierRep,
                       PEGASUS_QUALIFIER_ORDEREDSET_HASHSIZE> QualifierSet;
    QualifierSet qualifierContainer;
    // Create a test qualifier add to container and change the name
    CIMName nameBefore("before");
    CIMName nameAfter("after");
    CIMQualifier testQualifier(nameBefore, CIMValue(Uint32(1)));
    qualifierContainer.append(testQualifier);
    try
    {
        testQualifier.setName(nameAfter);
    } catch(Exception &)
    {
        VCOUT << " +++++ passed" << endl;
        return;
    }
    // test failed if not caught an Exception
    PEGASUS_TEST_ASSERT(false);
}

void testSetParameterNameContainerException()
{
    VCOUT << "Test Exception on setName() of contained CIMParameter ...";
    typedef OrderedSet<CIMParameter,
                       CIMParameterRep,
                       PEGASUS_PARAMETER_ORDEREDSET_HASHSIZE> ParameterSet;
    ParameterSet parameterContainer;
    // Create a test parameter add to container and change the name
    CIMName nameBefore("before");
    CIMName nameAfter("after");
    CIMParameter testParameter(nameBefore, CIMTYPE_UINT32);
    parameterContainer.append(testParameter);
    try
    {
        testParameter.setName(nameAfter);
    } catch(Exception &)
    {
        VCOUT << " +++++ passed" << endl;
        return;
    }
    // test failed if not caught an Exception
    PEGASUS_TEST_ASSERT(false);
}

void testSetMethodNameContainerException()
{
    VCOUT << "Test Exception on setName() of contained CIMMethod ...";
    typedef OrderedSet<CIMMethod,
                       CIMMethodRep,
                       PEGASUS_METHOD_ORDEREDSET_HASHSIZE> MethodSet;
    MethodSet methodContainer;
    // Create a test method add to container and change the name
    CIMName nameBefore("before");
    CIMName nameAfter("after");
    CIMMethod testMethod(nameBefore, CIMTYPE_UINT32);
    methodContainer.append(testMethod);
    try
    {
        testMethod.setName(nameAfter);
    } catch(Exception &)
    {
        VCOUT << " +++++ passed" << endl;
        return;
    }
    // test failed if not caught an Exception
    PEGASUS_TEST_ASSERT(false);
}

void testCharSetToUpperHash()
{
    VCOUT << "Test CharSet::toUpperHash() ...";
    Uint8 c=0;
    for (int i=0; i<256; i++, c++)
    {
        Uint16 hash = CharSet::toUpperHash(c);
        Uint16 algorithHash = 0;
        Uint8  code = CharSet::toUpper(c);
        if ((code & 0x01) > 0)
            algorithHash |= 0x0001;
        if ((code & 0x02) > 0)
            algorithHash |= 0x0004;
        if ((code & 0x04) > 0)
            algorithHash |= 0x0010;
        if ((code & 0x08) > 0)
            algorithHash |= 0x0040;
        if ((code & 0x10) > 0)
            algorithHash |= 0x0100;
        if ((code & 0x20) > 0)
            algorithHash |= 0x0400;
        if ((code & 0x40) > 0)
            algorithHash |= 0x1000;
        if ((code & 0x80) > 0)
            algorithHash |= 0x4000;

        if (algorithHash != hash)
        {
            VCOUT << "_toUpperHash[" << i << "] is " << hash \
                  << ", but should be " << algorithHash << endl;
            PEGASUS_TEST_ASSERT(algorithHash == hash);
        }
    }
    VCOUT << " +++++ passed" << endl;
}

int main(int, char** argv)
{
    verbose = getenv("PEGASUS_TEST_VERBOSE") ? true : false;

    try
    {
        // check interfaces to OrderedSet implementation
        testOrderedSetCIMProperty();
        testOrderedSetCIMQualifier();
        testOrderedSetCIMParameter();
        testOrderedSetCIMMethod();

        // check _array and _table overruns
        testOrderedSetPropertyOverFlow();

        // check setName exception for contained objects
        testSetPropertyNameContainerException();
        testSetQualifierNameContainerException();
        testSetParameterNameContainerException();
        testSetMethodNameContainerException();

        // check CharSet::ToUpperHash table for correctness
        testCharSetToUpperHash();
    }
    catch (Exception& e)
    {
        cout << "Exception: " << e.getMessage() << endl;
        exit(1);
    }
    catch (...)
    {
        cout << "Unexpected exception. Type unknown." << endl;
        exit(1);
    }

    cout << argv[0] << " +++++ passed all tests" << endl;
    return 0;
}

