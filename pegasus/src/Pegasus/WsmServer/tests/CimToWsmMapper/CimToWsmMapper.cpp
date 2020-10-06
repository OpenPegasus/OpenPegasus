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

#include <Pegasus/Common/PegasusAssert.h>
#include <Pegasus/WsmServer/WsmConstants.h>
#include <Pegasus/WsmServer/WsmUtils.h>
#include <Pegasus/WsmServer/WsmReader.h>
#include <Pegasus/WsmServer/WsmWriter.h>
#include <Pegasus/WsmServer/WsmValue.h>
#include <Pegasus/WsmServer/WsmEndpointReference.h>
#include <Pegasus/WsmServer/WsmInstance.h>
#include <Pegasus/WsmServer/WsmSelectorSet.h>
#include <Pegasus/WsmServer/CimToWsmResponseMapper.h>

PEGASUS_USING_PEGASUS;
PEGASUS_USING_STD;

static Boolean verbose;

static const String& ruri = WSM_RESOURCEURI_CIMSCHEMAV2;

/* This template provides a set of tests of simple CIMValues (excluding
   reference and embedded instance types). */
template<class T>
void testSimpleType(const T& x)
{
    CimToWsmResponseMapper mapper;

    WsmValue wsmValue;
    String wsmStr, cimStr;
    CIMValue cimValue(x);
    cimStr = cimValue.toString();
    if (cimValue.getType() == CIMTYPE_BOOLEAN)
    {
        cimStr.toLower();
    }
    mapper.convertCimToWsmValue(ruri, cimValue, wsmValue, String::EMPTY);
    wsmValue.get(wsmStr);
    if (wsmValue.getType() != WSMTYPE_OTHER || wsmValue.isArray() ||
        wsmValue.isNull() || wsmStr != cimStr)
    {
        throw Exception(String("Unvalid ") +
            String(cimTypeToString(cimValue.getType())) +
            String(" conversion"));
    }
}

static void _testValues(void)
{
    // Test simple data types
    testSimpleType((Sint8)-4);
    testSimpleType((Sint16)-44);
    testSimpleType((Sint32)-444);
    testSimpleType((Sint64)-4444);
    testSimpleType((Uint8)4);
    testSimpleType((Uint16)44);
    testSimpleType((Uint32)444);
    testSimpleType((Uint64)4444);
    testSimpleType(Boolean(true));
    testSimpleType(Boolean(false));
    testSimpleType(Char16('Z'));
    testSimpleType(Real32(1.5));
    testSimpleType(Real64(55.5));
    testSimpleType(Uint64(123456789));
    testSimpleType(Sint64(-123456789));
    testSimpleType(String("Hello world"));

    CimToWsmResponseMapper mapper;
    String str;

    // Test special floating point values: NaN, INF, -INF
    Real32 f1 = strtod("nan", 0);
    CIMValue cimf1(f1);
    WsmValue wsmf1;
    mapper.convertCimToWsmValue(ruri, cimf1, wsmf1, String::EMPTY);
    wsmf1.get(str);
    PEGASUS_TEST_ASSERT(str == "NaN");

    Real32 f2 = strtod("inf", 0);
    CIMValue cimf2(f2);
    WsmValue wsmf2;
    mapper.convertCimToWsmValue(ruri, cimf2, wsmf2, String::EMPTY);
    wsmf2.get(str);
    PEGASUS_TEST_ASSERT(str == "INF");

    Real32 f3 = strtod("-inf", 0);
    CIMValue cimf3(f3);
    WsmValue wsmf3;
    mapper.convertCimToWsmValue(ruri, cimf3, wsmf3, String::EMPTY);
    wsmf3.get(str);
    PEGASUS_TEST_ASSERT(str == "-INF");

    Real64 d1 = strtod("nan", 0);
    CIMValue cimd1(d1);
    WsmValue wsmd1;
    mapper.convertCimToWsmValue(ruri, cimd1, wsmd1, String::EMPTY);
    wsmd1.get(str);
    PEGASUS_TEST_ASSERT(str == "NaN");

    Real64 d2 = strtod("inf", 0);
    CIMValue cimd2(d2);
    WsmValue wsmd2;
    mapper.convertCimToWsmValue(ruri, cimd2, wsmd2, String::EMPTY);
    wsmd2.get(str);
    PEGASUS_TEST_ASSERT(str == "INF");

    Real64 d3 = strtod("-inf", 0);
    CIMValue cimd3(d3);
    WsmValue wsmd3;
    mapper.convertCimToWsmValue(ruri, cimd3, wsmd3, String::EMPTY);
    wsmd3.get(str);
    PEGASUS_TEST_ASSERT(str == "-INF");

    // Test datetime conversions
    String wsmDT;
    mapper.convertCimToWsmDatetime(
        CIMDateTime("19991224120000.000000+060"), wsmDT);
    PEGASUS_TEST_ASSERT(wsmDT == "1999-12-24T12:00:00+01:00");
    mapper.convertCimToWsmDatetime(
        CIMDateTime("09990204020105.000000+060"), wsmDT);
    PEGASUS_TEST_ASSERT(wsmDT == "0999-02-04T02:01:05+01:00");
    mapper.convertCimToWsmDatetime(
        CIMDateTime("19991224120000.000123+360"), wsmDT);
    PEGASUS_TEST_ASSERT(wsmDT == "1999-12-24T12:00:00.000123+06:00");
    mapper.convertCimToWsmDatetime(
        CIMDateTime("19991224120000.000000-090"), wsmDT);
    PEGASUS_TEST_ASSERT(wsmDT == "1999-12-24T12:00:00-01:30");
    mapper.convertCimToWsmDatetime(
        CIMDateTime("19991224120000.000000-665"), wsmDT);
    PEGASUS_TEST_ASSERT(wsmDT == "1999-12-24T12:00:00-11:05");
    mapper.convertCimToWsmDatetime(
        CIMDateTime("19991224120000.123000+000"), wsmDT);
    PEGASUS_TEST_ASSERT(wsmDT == "1999-12-24T12:00:00.123000Z");
    mapper.convertCimToWsmDatetime(
        CIMDateTime("19991224120000.123000-000"), wsmDT);
    PEGASUS_TEST_ASSERT(wsmDT == "1999-12-24T12:00:00.123000Z");
    mapper.convertCimToWsmDatetime(
        CIMDateTime("19991224******.******+000"), wsmDT);
    PEGASUS_TEST_ASSERT(wsmDT == "1999-12-24Z");
    mapper.convertCimToWsmDatetime(
        CIMDateTime("19991224******.******+360"), wsmDT);
    PEGASUS_TEST_ASSERT(wsmDT == "1999-12-24+06:00");
    mapper.convertCimToWsmDatetime(
        CIMDateTime("199912********.******+000"), wsmDT);
    PEGASUS_TEST_ASSERT(wsmDT == "199912********.******+000");
    mapper.convertCimToWsmDatetime(
        CIMDateTime("11111111223344.555***:000"), wsmDT);
    PEGASUS_TEST_ASSERT(wsmDT == "P11111111DT22H33M44.555S");
    mapper.convertCimToWsmDatetime(
        CIMDateTime("111111112233**.******:000"), wsmDT);
    PEGASUS_TEST_ASSERT(wsmDT == "P11111111DT22H33M");
    mapper.convertCimToWsmDatetime(
        CIMDateTime("11111111******.******:000"), wsmDT);
    PEGASUS_TEST_ASSERT(wsmDT == "P11111111D");
    mapper.convertCimToWsmDatetime(
        CIMDateTime("0000000011****.******:000"), wsmDT);
    PEGASUS_TEST_ASSERT(wsmDT == "PT11H");

    // Test embedded instances
    {
        WsmInstance wsmInst;
        WsmValue wsmValue;
        CIMInstance cimInst(CIMName("MyClass"));
        cimInst.
            addProperty(CIMProperty(CIMName("message"), String("Goodbye")));
        cimInst.addProperty(CIMProperty(CIMName("count"), Uint32(55)));
        cimInst.addProperty(CIMProperty(CIMName("flag"), Boolean(true)));
        CIMValue cimValue(cimInst);
        mapper.convertCimToWsmValue(ruri, cimValue, wsmValue, String::EMPTY);
        wsmValue.get(wsmInst);
        if (wsmInst.getClassName() != "MyClass" ||
            wsmInst.getPropertyCount() != 3)
            throw Exception("Invalid instance conversion");

        String str1, str2, str3;
        wsmInst.getProperty(0).getValue().get(str1);
        wsmInst.getProperty(1).getValue().get(str2);
        wsmInst.getProperty(2).getValue().get(str3);
        if (wsmInst.getProperty(0).getName() != "message" ||
            wsmInst.getProperty(1).getName() != "count" ||
            wsmInst.getProperty(2).getName() != "flag" ||
            str1 != "Goodbye" || str2 != "55" || str3 != "true")
            throw Exception("Invalid instance conversion");
    }

    // Test embedded objects
    {
        WsmInstance wsmInst;
        WsmValue wsmValue;
        CIMInstance cimInst(CIMName("MyClass"));
        cimInst.
            addProperty(CIMProperty(CIMName("message"), String("Goodbye")));
        cimInst.addProperty(CIMProperty(CIMName("count"), Uint32(55)));
        cimInst.addProperty(CIMProperty(CIMName("flag"), Boolean(true)));
        CIMObject cimObj(cimInst);
        CIMValue cimValue(cimObj);
        mapper.convertCimToWsmValue(ruri, cimValue, wsmValue, String::EMPTY);
        wsmValue.get(wsmInst);
        if (wsmInst.getClassName() != "MyClass" ||
            wsmInst.getPropertyCount() != 3)
            throw Exception("Invalid instance conversion");

        String str1, str2, str3;
        wsmInst.getProperty(0).getValue().get(str1);
        wsmInst.getProperty(1).getValue().get(str2);
        wsmInst.getProperty(2).getValue().get(str3);
        if (wsmInst.getProperty(0).getName() != "message" ||
            wsmInst.getProperty(1).getName() != "count" ||
            wsmInst.getProperty(2).getName() != "flag" ||
            str1 != "Goodbye" || str2 != "55" || str3 != "true")
            throw Exception("Invalid instance conversion");
    }

    // Tests references
    {
        WsmEndpointReference epr;
        WsmValue wsmValue;
        CIMObjectPath op("//atp:77/root/cimv25:TennisPlayer.last=\"Rafter\"");
        CIMValue cimValue(op);
        mapper.convertCimToWsmValue(ruri, cimValue, wsmValue, String::EMPTY);
        wsmValue.get(epr);
        if (epr.address != "http://atp:77/wsman" ||
            epr.resourceUri !=
            "http://schemas.dmtf.org/wbem/wscim/1/cim-schema/2/TennisPlayer" ||
            epr.selectorSet->selectors.size() != 2 ||
            epr.selectorSet->selectors[0].name != "__cimnamespace" ||
            epr.selectorSet->selectors[0].value != "root/cimv25" ||
            epr.selectorSet->selectors[1].name != "last" ||
            epr.selectorSet->selectors[1].value != "Rafter")
            throw Exception("Invalid EPR conversion");
    }
}

/* This template provides a set of tests of array CIMValues (excluding
   reference and embedded instance types). */
template<class T>
void testArrayType(const Array<T>& x)
{
    CimToWsmResponseMapper mapper;

    WsmValue wsmValue;
    Array<String> arr;
    CIMValue cimValue(x);
    mapper.convertCimToWsmValue(ruri, cimValue, wsmValue, String::EMPTY);
    wsmValue.get(arr);

    if (wsmValue.getType() != WSMTYPE_OTHER || !wsmValue.isArray() ||
        wsmValue.isNull() || x.size() != arr.size())
        throw Exception(String("Unvalid ") +
            String(cimTypeToString(cimValue.getType())) +
            String(" conversion"));

    for (Uint32 i = 0; i < x.size(); i++)
    {
        String cimStr;
        CIMValue val(x[i]);
        cimStr = val.toString();
        if (val.getType() == CIMTYPE_BOOLEAN)
        {
            cimStr.toLower();
        }
        if (cimStr != arr[i])
            throw Exception(String("Unvalid ") +
                String(cimTypeToString(cimValue.getType())) +
                String(" conversion"));
    }
}

static void _testArrayValues(void)
{
    // Test arrays of simple types
    Array<Sint8> s8_arr;
    s8_arr.append(-11);
    s8_arr.append(-22);
    testArrayType(s8_arr);

    Array<Sint16> s16_arr;
    s16_arr.append(-111);
    s16_arr.append(-222);
    testArrayType(s16_arr);

    Array<Sint32> s32_arr;
    s32_arr.append(-1111);
    s32_arr.append(-2222);
    testArrayType(s32_arr);

    Array<Sint64> s64_arr;
    s64_arr.append(-11111);
    s64_arr.append(-22222);
    testArrayType(s64_arr);

    Array<Uint8> u8_arr;
    u8_arr.append(11);
    u8_arr.append(22);
    testArrayType(u8_arr);

    Array<Uint16> u16_arr;
    u16_arr.append(111);
    u16_arr.append(222);
    testArrayType(u16_arr);

    Array<Uint32> u32_arr;
    u32_arr.append(1111);
    u32_arr.append(2222);
    testArrayType(u32_arr);

    Array<Uint64> u64_arr;
    u64_arr.append(11111);
    u64_arr.append(22222);
    testArrayType(u64_arr);

    Array<Boolean> b_arr;
    b_arr.append(true);
    b_arr.append(false);
    testArrayType(b_arr);

    Array<Real32> r32_arr;
    r32_arr.append(Real32(1.5));
    r32_arr.append(Real32(2.5));
    testArrayType(r32_arr);

    Array<Real64> r64_arr;
    r64_arr.append(Real64(11.5));
    r64_arr.append(Real64(12.5));
    testArrayType(r64_arr);

    Array<Char16> c16_arr;
    c16_arr.append(Char16('Z'));
    c16_arr.append(Char16('X'));
    testArrayType(c16_arr);

    Array<String> str_arr;
    str_arr.append("Test string 1");
    str_arr.append("Test string 2");
    testArrayType(str_arr);

    CimToWsmResponseMapper mapper;

    // Test arrays of datetimes
    {
        Array<String> wsmDTs;
        Array<CIMDateTime> cimDTs;
        cimDTs.append(CIMDateTime("19991224120000.000000+360"));
        cimDTs.append(CIMDateTime("20001224120000.000000+360"));
        CIMValue cimValue(cimDTs);
        WsmValue wsmValue;
        mapper.convertCimToWsmValue(ruri, cimValue, wsmValue, String::EMPTY);
        wsmValue.get(wsmDTs);
        PEGASUS_TEST_ASSERT(wsmDTs.size() == 2 &&
            wsmDTs[0] == "1999-12-24T12:00:00+06:00" &&
            wsmDTs[1] == "2000-12-24T12:00:00+06:00");
    }

    // Test arrays of instances
    {
        Array<WsmInstance> wsmInstArray;
        Array<CIMInstance> cimInstArray;
        WsmValue wsmValue;

        CIMInstance cimInst1(CIMName("MyClass1"));
        cimInst1.addProperty(CIMProperty(CIMName("prop1"), String("value1")));
        cimInst1.addProperty(CIMProperty(CIMName("prop2"), String("value2")));
        cimInstArray.append(cimInst1);
        CIMInstance cimInst2(CIMName("MyClass2"));
        cimInst2.addProperty(CIMProperty(CIMName("prop3"), String("value3")));
        cimInst2.addProperty(CIMProperty(CIMName("prop4"), String("value4")));
        cimInstArray.append(cimInst2);
        CIMValue cimValue(cimInstArray);
        mapper.convertCimToWsmValue(ruri, cimValue, wsmValue, String::EMPTY);
        wsmValue.get(wsmInstArray);

        if (wsmInstArray.size() != 2 ||
            wsmInstArray[0].getClassName() != "MyClass1" ||
            wsmInstArray[0].getPropertyCount() != 2 ||
            wsmInstArray[1].getClassName() != "MyClass2" ||
            wsmInstArray[1].getPropertyCount() != 2)
            throw Exception("Invalid instance array conversion");

        String str1, str2, str3, str4;
        wsmInstArray[0].getProperty(0).getValue().get(str1);
        wsmInstArray[0].getProperty(1).getValue().get(str2);
        wsmInstArray[1].getProperty(0).getValue().get(str3);
        wsmInstArray[1].getProperty(1).getValue().get(str4);

        if (wsmInstArray[0].getProperty(0).getName() != "prop1" ||
            wsmInstArray[0].getProperty(1).getName() != "prop2" ||
            wsmInstArray[1].getProperty(0).getName() != "prop3" ||
            wsmInstArray[1].getProperty(1).getName() != "prop4" ||
            str1 != "value1" || str2 != "value2" ||
            str3 != "value3" || str4 != "value4")
            throw Exception("Invalid instance conversion");
    }

    // Test arrays of objects
    {
        Array<WsmInstance> wsmInstArray;
        Array<CIMObject> cimObjArray;
        WsmValue wsmValue;

        CIMInstance cimInst1(CIMName("MyClass1"));
        cimInst1.addProperty(CIMProperty(CIMName("prop1"), String("value1")));
        cimInst1.addProperty(CIMProperty(CIMName("prop2"), String("value2")));
        cimObjArray.append(CIMObject(cimInst1));
        CIMInstance cimInst2(CIMName("MyClass2"));
        cimInst2.addProperty(CIMProperty(CIMName("prop3"), String("value3")));
        cimInst2.addProperty(CIMProperty(CIMName("prop4"), String("value4")));
        cimObjArray.append(CIMObject(cimInst2));
        CIMValue cimValue(cimObjArray);
        mapper.convertCimToWsmValue(ruri, cimValue, wsmValue, String::EMPTY);
        wsmValue.get(wsmInstArray);

        if (wsmInstArray.size() != 2 ||
            wsmInstArray[0].getClassName() != "MyClass1" ||
            wsmInstArray[0].getPropertyCount() != 2 ||
            wsmInstArray[1].getClassName() != "MyClass2" ||
            wsmInstArray[1].getPropertyCount() != 2)
            throw Exception("Invalid instance array conversion");

        String str1, str2, str3, str4;
        wsmInstArray[0].getProperty(0).getValue().get(str1);
        wsmInstArray[0].getProperty(1).getValue().get(str2);
        wsmInstArray[1].getProperty(0).getValue().get(str3);
        wsmInstArray[1].getProperty(1).getValue().get(str4);

        if (wsmInstArray[0].getProperty(0).getName() != "prop1" ||
            wsmInstArray[0].getProperty(1).getName() != "prop2" ||
            wsmInstArray[1].getProperty(0).getName() != "prop3" ||
            wsmInstArray[1].getProperty(1).getName() != "prop4" ||
            str1 != "value1" || str2 != "value2" ||
            str3 != "value3" || str4 != "value4")
            throw Exception("Invalid instance conversion");
    }

    // Test arrays of EPRs
    {
        Array<WsmEndpointReference> eprArray;
        Array<CIMObjectPath> opArray;
        WsmValue wsmValue;

        CIMObjectPath op1("//atp:11/root/cimv1:Player.last=\"Marleau\"");
        CIMObjectPath op2("//atp:22/root/cimv2:Player.last=\"Thornton\"");
        opArray.append(op1);
        opArray.append(op2);
        CIMValue cimValue(opArray);
        mapper.convertCimToWsmValue(ruri, cimValue, wsmValue, String::EMPTY);
        wsmValue.get(eprArray);

        if (eprArray[0].address != "http://atp:11/wsman" ||
            eprArray[0].resourceUri !=
                "http://schemas.dmtf.org/wbem/wscim/1/cim-schema/2/Player" ||
            eprArray[0].selectorSet->selectors.size() != 2 ||
            eprArray[0].selectorSet->selectors[0].name != "__cimnamespace" ||
            eprArray[0].selectorSet->selectors[0].value != "root/cimv1" ||
            eprArray[0].selectorSet->selectors[1].name != "last" ||
            eprArray[0].selectorSet->selectors[1].value != "Marleau" ||
            eprArray[1].address != "http://atp:22/wsman" ||
            eprArray[1].resourceUri !=
                "http://schemas.dmtf.org/wbem/wscim/1/cim-schema/2/Player" ||
            eprArray[1].selectorSet->selectors.size() != 2 ||
            eprArray[1].selectorSet->selectors[0].name != "__cimnamespace" ||
            eprArray[1].selectorSet->selectors[0].value != "root/cimv2" ||
            eprArray[1].selectorSet->selectors[1].name != "last" ||
            eprArray[1].selectorSet->selectors[1].value != "Thornton")
            throw Exception("Invalid EPR conversion");
    }
}

static void _testExceptions(void)
{
    CimToWsmResponseMapper mapper;

    CIMException e1(CIM_ERR_FAILED, "failed");
    WsmFault f1 = mapper.mapCimExceptionToWsmFault(e1);
    PEGASUS_TEST_ASSERT(f1.getCode() == "SOAP-ENV:Receiver");
    PEGASUS_TEST_ASSERT(f1.getSubcode() == "wsman:InternalError");
    PEGASUS_TEST_ASSERT(f1.getReason() == "CIM_ERR_FAILED: failed");

    CIMException e2(CIM_ERR_CLASS_HAS_CHILDREN, "class has children");
    WsmFault f2 = mapper.mapCimExceptionToWsmFault(e2);
    PEGASUS_TEST_ASSERT(f2.getCode() == "SOAP-ENV:Receiver");
    PEGASUS_TEST_ASSERT(f2.getSubcode() == "wsman:InternalError");
    PEGASUS_TEST_ASSERT(f2.getReason() ==
        "CIM_ERR_CLASS_HAS_CHILDREN: class has children");

    CIMException e3(CIM_ERR_CLASS_HAS_INSTANCES, "class has instances");
    WsmFault f3 = mapper.mapCimExceptionToWsmFault(e3);
    PEGASUS_TEST_ASSERT(f3.getCode() == "SOAP-ENV:Receiver");
    PEGASUS_TEST_ASSERT(f3.getSubcode() == "wsman:InternalError");
    PEGASUS_TEST_ASSERT(f3.getReason() ==
        "CIM_ERR_CLASS_HAS_INSTANCES: class has instances");

    CIMException e4(CIM_ERR_INVALID_SUPERCLASS, "invalid superclass");
    WsmFault f4 = mapper.mapCimExceptionToWsmFault(e4);
    PEGASUS_TEST_ASSERT(f4.getCode() == "SOAP-ENV:Receiver");
    PEGASUS_TEST_ASSERT(f4.getSubcode() == "wsman:InternalError");
    PEGASUS_TEST_ASSERT(f4.getReason() ==
        "CIM_ERR_INVALID_SUPERCLASS: invalid superclass");

    CIMException e5(CIM_ERR_METHOD_NOT_FOUND, "method not found");
    WsmFault f5 = mapper.mapCimExceptionToWsmFault(e5);
    PEGASUS_TEST_ASSERT(f5.getCode() == "SOAP-ENV:Receiver");
    PEGASUS_TEST_ASSERT(f5.getSubcode() == "wsman:InternalError");
    PEGASUS_TEST_ASSERT(f5.getReason() ==
        "CIM_ERR_METHOD_NOT_FOUND: method not found");

    CIMException e6(CIM_ERR_METHOD_NOT_AVAILABLE, "method not available");
    WsmFault f6 = mapper.mapCimExceptionToWsmFault(e6);
    PEGASUS_TEST_ASSERT(f6.getCode() == "SOAP-ENV:Receiver");
    PEGASUS_TEST_ASSERT(f6.getSubcode() == "wsman:InternalError");
    PEGASUS_TEST_ASSERT(f6.getReason() ==
        "CIM_ERR_METHOD_NOT_AVAILABLE: method not available");

    CIMException e7(CIM_ERR_NO_SUCH_PROPERTY, "no such property");
    WsmFault f7 = mapper.mapCimExceptionToWsmFault(e7);
    PEGASUS_TEST_ASSERT(f7.getCode() == "SOAP-ENV:Receiver");
    PEGASUS_TEST_ASSERT(f7.getSubcode() == "wsman:InternalError");
    PEGASUS_TEST_ASSERT(f7.getReason() ==
        "CIM_ERR_NO_SUCH_PROPERTY: no such property");

    CIMException e8(CIM_ERR_TYPE_MISMATCH, "type mismatch");
    WsmFault f8 = mapper.mapCimExceptionToWsmFault(e8);
    PEGASUS_TEST_ASSERT(f8.getCode() == "SOAP-ENV:Receiver");
    PEGASUS_TEST_ASSERT(f8.getSubcode() == "wsman:InternalError");
    PEGASUS_TEST_ASSERT(f8.getReason() ==
        "CIM_ERR_TYPE_MISMATCH: type mismatch");

    CIMException e9(CIM_ERR_ACCESS_DENIED, "access denied");
    WsmFault f9 = mapper.mapCimExceptionToWsmFault(e9);
    PEGASUS_TEST_ASSERT(f9.getCode() == "SOAP-ENV:Sender");
    PEGASUS_TEST_ASSERT(f9.getSubcode() == "wsman:AccessDenied");
    PEGASUS_TEST_ASSERT(f9.getReason() ==
        "CIM_ERR_ACCESS_DENIED: access denied");

    CIMException e10(CIM_ERR_ALREADY_EXISTS, "already exists");
    WsmFault f10 = mapper.mapCimExceptionToWsmFault(e10);
    PEGASUS_TEST_ASSERT(f10.getCode() == "SOAP-ENV:Sender");
    PEGASUS_TEST_ASSERT(f10.getSubcode() == "wsman:AlreadyExists");
    PEGASUS_TEST_ASSERT(f10.getReason() ==
        "CIM_ERR_ALREADY_EXISTS: already exists");

    CIMException e11(CIM_ERR_INVALID_CLASS, "invalid class");
    WsmFault f11 = mapper.mapCimExceptionToWsmFault(e11);
    PEGASUS_TEST_ASSERT(f11.getCode() == "SOAP-ENV:Sender");
    PEGASUS_TEST_ASSERT(f11.getSubcode() == "wsa:DestinationUnreachable");
    PEGASUS_TEST_ASSERT(f11.getReason() ==
        "CIM_ERR_INVALID_CLASS: invalid class");
    PEGASUS_TEST_ASSERT(f11.getFaultDetail() ==
        WSMAN_FAULTDETAIL_INVALIDRESOURCEURI);

    CIMException e12(CIM_ERR_INVALID_NAMESPACE, "invalid namespace");
    WsmFault f12 = mapper.mapCimExceptionToWsmFault(e12);
    PEGASUS_TEST_ASSERT(f12.getCode() == "SOAP-ENV:Sender");
    PEGASUS_TEST_ASSERT(f12.getSubcode() == "wsa:DestinationUnreachable");
    PEGASUS_TEST_ASSERT(f12.getReason() ==
        "CIM_ERR_INVALID_NAMESPACE: invalid namespace");

    CIMException e13(CIM_ERR_INVALID_PARAMETER, "invalid parameter");
    WsmFault f13 = mapper.mapCimExceptionToWsmFault(e13);
    PEGASUS_TEST_ASSERT(f13.getCode() == "SOAP-ENV:Sender");
    PEGASUS_TEST_ASSERT(f13.getSubcode() == "wsman:InvalidParameter");
    PEGASUS_TEST_ASSERT(f13.getReason() ==
        "CIM_ERR_INVALID_PARAMETER: invalid parameter");

    CIMException e14(CIM_ERR_INVALID_QUERY, "invalid query");
    WsmFault f14 = mapper.mapCimExceptionToWsmFault(e14);
    PEGASUS_TEST_ASSERT(f14.getCode() == "SOAP-ENV:Sender");
    PEGASUS_TEST_ASSERT(f14.getSubcode() == "wsen:CannotProcessFilter");
    PEGASUS_TEST_ASSERT(f14.getReason() ==
        "CIM_ERR_INVALID_QUERY: invalid query");

    CIMException e15(CIM_ERR_NOT_FOUND, "not found");
    WsmFault f15 = mapper.mapCimExceptionToWsmFault(e15);
    PEGASUS_TEST_ASSERT(f15.getCode() == "SOAP-ENV:Sender");
    PEGASUS_TEST_ASSERT(f15.getSubcode() == "wsa:DestinationUnreachable");
    PEGASUS_TEST_ASSERT(f15.getReason() == "CIM_ERR_NOT_FOUND: not found");

    CIMException e16(CIM_ERR_NOT_SUPPORTED, "not supported");
    WsmFault f16 = mapper.mapCimExceptionToWsmFault(e16);
    PEGASUS_TEST_ASSERT(f16.getCode() == "SOAP-ENV:Sender");
    PEGASUS_TEST_ASSERT(f16.getSubcode() == "wsa:ActionNotSupported");
    PEGASUS_TEST_ASSERT(f16.getReason() ==
        "CIM_ERR_NOT_SUPPORTED: not supported");
    PEGASUS_TEST_ASSERT(f16.getFaultDetail() ==
        WSMAN_FAULTDETAIL_ACTIONMISMATCH);

    CIMException e17(CIM_ERR_QUERY_LANGUAGE_NOT_SUPPORTED,
        "query language not supported");
    WsmFault f17 = mapper.mapCimExceptionToWsmFault(e17);
    PEGASUS_TEST_ASSERT(f17.getCode() == "SOAP-ENV:Sender");
    PEGASUS_TEST_ASSERT(f17.getSubcode() == "wsen:FilteringNotSupported");
    PEGASUS_TEST_ASSERT(f17.getReason() ==
        "CIM_ERR_QUERY_LANGUAGE_NOT_SUPPORTED: query language not supported");
}

int main(int argc, char** argv)
{
    verbose = getenv("PEGASUS_TEST_VERBOSE") ? true : false;

    try
    {
        if (verbose)
            cout << "Testing simple values." << endl;
        _testValues();

        if (verbose)
            cout << "Testing array values." << endl;
        _testArrayValues();


        if (verbose)
            cout << "Testing exceptions." << endl;
        _testExceptions();
    }

    catch(Exception& e)
    {
        cerr << "Error: " << e.getMessage() << endl;
        exit(1);
    }

    cout << argv[0] << " +++++ passed all tests" << endl;

    return 0;
}
