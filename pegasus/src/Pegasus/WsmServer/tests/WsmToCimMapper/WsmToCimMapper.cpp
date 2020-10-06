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
#include <Pegasus/Common/FileSystem.h>
#include <Pegasus/WsmServer/WsmConstants.h>
#include <Pegasus/WsmServer/WsmUtils.h>
#include <Pegasus/WsmServer/WsmReader.h>
#include <Pegasus/WsmServer/WsmWriter.h>
#include <Pegasus/WsmServer/WsmValue.h>
#include <Pegasus/WsmServer/WsmEndpointReference.h>
#include <Pegasus/WsmServer/WsmInstance.h>
#include <Pegasus/WsmServer/WsmSelectorSet.h>
#include <Pegasus/WsmServer/WsmToCimRequestMapper.h>

PEGASUS_USING_PEGASUS;
PEGASUS_USING_STD;

#ifdef PEGASUS_OS_VMS
# define PEGASUS_NAN "NaNQ"
# define PEGASUS_INF "Infinity"
# define PEGASUS_NEG_INF "-Infinity"
#else
# define PEGASUS_NAN "nan"
# define PEGASUS_INF "inf"
# define PEGASUS_NEG_INF "-inf"
#endif

static Boolean verbose;
static String repositoryRoot;

/* This template provides a set of tests of simple CIMValues
   (excluding reference and embedded instance types). */
template<class T>
void testSimpleType(const T& x)
{
    WsmToCimRequestMapper mapper((CIMRepository*) 0);

    // Create a NULL CIMValue of the appropriate type. Normally type
    // info is retrieved from the repository.
    CIMValue tmp(x);
    CIMValue cimValue(tmp.getType(), tmp.isArray());
    String str = tmp.toString();
    if (tmp.getType() == CIMTYPE_BOOLEAN)
    {
        str.toLower();
    }
    WsmValue wsmValue(str);
    mapper.convertWsmToCimValue(wsmValue, CIMNamespaceName(), cimValue);
    PEGASUS_TEST_ASSERT(tmp == cimValue);
}


/* This template provides a set of tests of array CIMValues (excluding
   reference and embedded instance types). */
template<class T>
void testArrayType(const Array<T>& x)
{
    WsmToCimRequestMapper mapper((CIMRepository*) 0);

    // Create a NULL CIMValue of the appropriate type. Normally type
    // info is retrieved from the repository.
    CIMValue tmp(x);
    CIMValue cimValue(tmp.getType(), tmp.isArray());

    // Create WsmValue out of the given array
    Array<String> arr;
    for (Uint32 i = 0; i < x.size(); i++)
    {
        String str = CIMValue(x[i]).toString();
        if (tmp.getType() == CIMTYPE_BOOLEAN)
        {
            str.toLower();
        }
        arr.append(str);
    }

    WsmValue wsmValue(arr);
    mapper.convertWsmToCimValue(wsmValue, CIMNamespaceName(), cimValue);
    PEGASUS_TEST_ASSERT(tmp == cimValue);
}

static void _testValues(void)
{
    WsmToCimRequestMapper mapper((CIMRepository*) 0);

    // Test simple data types
    testSimpleType(Boolean(true));
    testSimpleType(Boolean(false));
    testSimpleType((Sint8)-4);
    testSimpleType((Sint16)-44);
    testSimpleType((Sint32)-444);
    testSimpleType((Sint64)-4444);
    testSimpleType((Uint8)4);
    testSimpleType((Uint16)44);
    testSimpleType((Uint32)444);
    testSimpleType((Uint64)4444);
    testSimpleType(Char16('Z'));
    testSimpleType(Real32(1.5));
    testSimpleType(Real64(55.5));
    testSimpleType(Uint64(123456789));
    testSimpleType(Sint64(-123456789));
    testSimpleType(String("Hello world"));

    // Test special floating point values: NaN, INF, -INF
    WsmValue wsmf1("NaN");
    CIMValue cimf1((Real32)0.0);
    mapper.convertWsmToCimValue(wsmf1, CIMNamespaceName(), cimf1);
    PEGASUS_TEST_ASSERT(cimf1.toString() == PEGASUS_NAN);

    WsmValue wsmf2("INF");
    CIMValue cimf2((Real32)0.0);
    mapper.convertWsmToCimValue(wsmf2, CIMNamespaceName(), cimf2);
    PEGASUS_TEST_ASSERT(cimf2.toString() == PEGASUS_INF);

    WsmValue wsmf3("-INF");
    CIMValue cimf3((Real32)0.0);
    mapper.convertWsmToCimValue(wsmf3, CIMNamespaceName(), cimf3);
    PEGASUS_TEST_ASSERT(cimf3.toString() == PEGASUS_NEG_INF);

    WsmValue wsmd1("NaN");
    CIMValue cimd1((Real64)0.0);
    mapper.convertWsmToCimValue(wsmd1, CIMNamespaceName(), cimd1);
    PEGASUS_TEST_ASSERT(cimd1.toString() == PEGASUS_NAN);

    WsmValue wsmd2("INF");
    CIMValue cimd2((Real64)0.0);
    mapper.convertWsmToCimValue(wsmd2, CIMNamespaceName(), cimd2);
    PEGASUS_TEST_ASSERT(cimd2.toString() == PEGASUS_INF);

    WsmValue wsmd3("-INF");
    CIMValue cimd3((Real64)0.0);
    mapper.convertWsmToCimValue(wsmd3, CIMNamespaceName(), cimd3);
    PEGASUS_TEST_ASSERT(cimd3.toString() == PEGASUS_NEG_INF);

    // Test datetime
    CIMDateTime cimDT;

    mapper.convertWsmToCimDatetime("P1Y1M1DT10H5M44.0012345678901234S", cimDT);
    PEGASUS_TEST_ASSERT(cimDT == CIMDateTime("00000396100544.001234:000"));
    mapper.convertWsmToCimDatetime("P1Y1M1DT10H5M44.12345678901234S", cimDT);
    PEGASUS_TEST_ASSERT(cimDT == CIMDateTime("00000396100544.123456:000"));
    mapper.convertWsmToCimDatetime("P1Y1M1DT10H5M44.0055555555S", cimDT);
    PEGASUS_TEST_ASSERT(cimDT == CIMDateTime("00000396100544.005555:000"));
    mapper.convertWsmToCimDatetime("P1Y1M1DT10H5M44.00500000S", cimDT);
    PEGASUS_TEST_ASSERT(cimDT == CIMDateTime("00000396100544.005000:000"));
    mapper.convertWsmToCimDatetime("P1Y1M1DT10H5M44.9999999S", cimDT);
    PEGASUS_TEST_ASSERT(cimDT == CIMDateTime("00000396100544.999999:000"));
    mapper.convertWsmToCimDatetime("P1Y1M1DT10H5M44S", cimDT);
    PEGASUS_TEST_ASSERT(cimDT == CIMDateTime("00000396100544.000000:000"));
    mapper.convertWsmToCimDatetime("P1Y1M1DT10H5M44.0055S", cimDT);
    PEGASUS_TEST_ASSERT(cimDT == CIMDateTime("00000396100544.005500:000"));
    mapper.convertWsmToCimDatetime("PT10H5M44.0055S", cimDT);
    PEGASUS_TEST_ASSERT(cimDT == CIMDateTime("00000000100544.005500:000"));
    mapper.convertWsmToCimDatetime("P10Y", cimDT);
    PEGASUS_TEST_ASSERT(cimDT == CIMDateTime("00003652000000.000000:000"));
    mapper.convertWsmToCimDatetime("P10Y18M", cimDT);
    PEGASUS_TEST_ASSERT(cimDT == CIMDateTime("00004201000000.000000:000"));
    mapper.convertWsmToCimDatetime("P10Y18M40D", cimDT);
    PEGASUS_TEST_ASSERT(cimDT == CIMDateTime("00004241000000.000000:000"));
    mapper.convertWsmToCimDatetime("P10Y18M40DT34H", cimDT);
    PEGASUS_TEST_ASSERT(cimDT == CIMDateTime("00004242100000.000000:000"));
    mapper.convertWsmToCimDatetime("P10Y18M40DT34H70M", cimDT);
    PEGASUS_TEST_ASSERT(cimDT == CIMDateTime("00004242111000.000000:000"));
    mapper.convertWsmToCimDatetime("P10Y18M40DT34H70M140S", cimDT);
    PEGASUS_TEST_ASSERT(cimDT == CIMDateTime("00004242111220.000000:000"));
    mapper.convertWsmToCimDatetime("PT70M140S", cimDT);
    PEGASUS_TEST_ASSERT(cimDT == CIMDateTime("00000000011220.000000:000"));
    mapper.convertWsmToCimDatetime("PT140S", cimDT);
    PEGASUS_TEST_ASSERT(cimDT == CIMDateTime("00000000000220.000000:000"));
    mapper.convertWsmToCimDatetime("PT5M44.0055S", cimDT);
    PEGASUS_TEST_ASSERT(cimDT == CIMDateTime("00000000000544.005500:000"));

    mapper.convertWsmToCimDatetime("2004-12-01", cimDT);
    PEGASUS_TEST_ASSERT(cimDT == CIMDateTime("20041201******.******+000"));
    mapper.convertWsmToCimDatetime("2004-12-01Z", cimDT);
    PEGASUS_TEST_ASSERT(cimDT == CIMDateTime("20041201******.******+000"));
    mapper.convertWsmToCimDatetime("2004-12-01+02:00", cimDT);
    PEGASUS_TEST_ASSERT(cimDT == CIMDateTime("20041201******.******+120"));
    mapper.convertWsmToCimDatetime("2004-12-01-11:30", cimDT);
    PEGASUS_TEST_ASSERT(cimDT == CIMDateTime("20041201******.******-690"));

    mapper.convertWsmToCimDatetime("2004-12-01T12:23:34+02:00", cimDT);
    PEGASUS_TEST_ASSERT(cimDT == CIMDateTime("20041201122334.000000+120"));
    mapper.convertWsmToCimDatetime("2004-12-01T12:23:34.0012+02:00", cimDT);
    PEGASUS_TEST_ASSERT(cimDT == CIMDateTime("20041201122334.001200+120"));
    mapper.convertWsmToCimDatetime("2004-12-01T12:23:34.0012-04:15", cimDT);
    PEGASUS_TEST_ASSERT(cimDT == CIMDateTime("20041201122334.001200-255"));
    mapper.convertWsmToCimDatetime("2004-12-01T12:23:34Z", cimDT);
    PEGASUS_TEST_ASSERT(cimDT == CIMDateTime("20041201122334.000000+000"));
    mapper.convertWsmToCimDatetime("2004-12-01T12:23:34.0012Z", cimDT);
    PEGASUS_TEST_ASSERT(cimDT == CIMDateTime("20041201122334.001200+000"));
    mapper.convertWsmToCimDatetime("2004-12-01T12:23:34.00+02:00", cimDT);
    PEGASUS_TEST_ASSERT(cimDT == CIMDateTime("20041201122334.000000+120"));
    mapper.convertWsmToCimDatetime("2004-12-01T12:23:34.0000000+02:00", cimDT);
    PEGASUS_TEST_ASSERT(cimDT == CIMDateTime("20041201122334.000000+120"));
    mapper.convertWsmToCimDatetime("2004-12-01T12:23:34.0000009+02:00", cimDT);
    PEGASUS_TEST_ASSERT(cimDT == CIMDateTime("20041201122334.000000+120"));
    mapper.convertWsmToCimDatetime("2004-12-01T12:23:34.000001+02:00", cimDT);
    PEGASUS_TEST_ASSERT(cimDT == CIMDateTime("20041201122334.000001+120"));
    mapper.convertWsmToCimDatetime("2004-12-01T12:23:34.1+02:00", cimDT);
    PEGASUS_TEST_ASSERT(cimDT == CIMDateTime("20041201122334.100000+120"));

    mapper.convertWsmToCimDatetime("20041201122334.001200+360", cimDT);
    PEGASUS_TEST_ASSERT(cimDT == CIMDateTime("20041201122334.001200+360"));


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

    Array<CIMDateTime> dt_arr;
    dt_arr.append(CIMDateTime("19991224120000.000000+360"));
    dt_arr.append(CIMDateTime("20001224120000.000000+360"));
    testArrayType(dt_arr);

    Array<String> str_arr;
    str_arr.append("Test string 1");
    str_arr.append("Test string 2");
    testArrayType(str_arr);

    // Test class URI to class name conversion
    String classURI = String(WSM_RESOURCEURI_CIMSCHEMAV2) + "/MyClass";
    CIMName cimName = mapper.convertResourceUriToClassName(classURI);
    PEGASUS_TEST_ASSERT(cimName.getString() == "MyClass");
}

#define ASSERT_FAULT(statement, subcode)        \
    do                                          \
    {                                           \
        try                                     \
        {                                       \
            statement;                          \
            PEGASUS_TEST_ASSERT(0);             \
        }                                       \
        catch (WsmFault& fault)                 \
        {                                       \
            PEGASUS_TEST_ASSERT(                \
                fault.getSubcode() == subcode); \
        }                                       \
    } while (0)

static void _testConversionErrors(void)
{
    WsmToCimRequestMapper mapper((CIMRepository*) 0);

    // Invalid boolean
    {
        CIMValue cimValue(CIMTYPE_BOOLEAN, false);
        WsmValue wsmValue("test");
        ASSERT_FAULT(
            mapper.convertWsmToCimValue(wsmValue, CIMNamespaceName(), cimValue),
            "wxf:InvalidRepresentation");
    }

    // Invalid uint/sint
    {
        CIMValue cimValue(CIMTYPE_UINT8, false);
        WsmValue wsmValue("test");
        ASSERT_FAULT(
            mapper.convertWsmToCimValue(wsmValue, CIMNamespaceName(), cimValue),
            "wxf:InvalidRepresentation");
    }

    {
        CIMValue cimValue(CIMTYPE_SINT8, false);
        WsmValue wsmValue("test");
        ASSERT_FAULT(
            mapper.convertWsmToCimValue(wsmValue, CIMNamespaceName(), cimValue),
            "wxf:InvalidRepresentation");
    }

    // Uint/sint out of bounds errors
    {
        CIMValue cimValue(CIMTYPE_SINT8, false);
        WsmValue wsmValue("-222");
        ASSERT_FAULT(
            mapper.convertWsmToCimValue(wsmValue, CIMNamespaceName(), cimValue),
            "wxf:InvalidRepresentation");
    }

    {
        CIMValue cimValue(CIMTYPE_SINT16, false);
        WsmValue wsmValue("-777777");
        ASSERT_FAULT(
            mapper.convertWsmToCimValue(wsmValue, CIMNamespaceName(), cimValue),
            "wxf:InvalidRepresentation");
    }

    {
        CIMValue cimValue(CIMTYPE_SINT32, false);
        WsmValue wsmValue("-4444444444");
        ASSERT_FAULT(
            mapper.convertWsmToCimValue(wsmValue, CIMNamespaceName(), cimValue),
            "wxf:InvalidRepresentation");
    }

    {
        CIMValue cimValue(CIMTYPE_UINT8, false);
        WsmValue wsmValue("333");
        ASSERT_FAULT(
            mapper.convertWsmToCimValue(wsmValue, CIMNamespaceName(), cimValue),
            "wxf:InvalidRepresentation");
    }

    {
        CIMValue cimValue(CIMTYPE_UINT16, false);
        WsmValue wsmValue("777777");
        ASSERT_FAULT(
            mapper.convertWsmToCimValue(wsmValue, CIMNamespaceName(), cimValue),
            "wxf:InvalidRepresentation");
    }

    {
        CIMValue cimValue(CIMTYPE_UINT32, false);
        WsmValue wsmValue("4444444444");
        ASSERT_FAULT(
            mapper.convertWsmToCimValue(wsmValue, CIMNamespaceName(), cimValue),
            "wxf:InvalidRepresentation");
    }

    // Invalid real values
    {
        CIMValue cimValue(CIMTYPE_REAL32, false);
        WsmValue wsmValue("35.54.32");
        ASSERT_FAULT(
            mapper.convertWsmToCimValue(wsmValue, CIMNamespaceName(), cimValue),
            "wxf:InvalidRepresentation");
    }

    {
        CIMValue cimValue(CIMTYPE_REAL64, false);
        WsmValue wsmValue("35.54.32");
        ASSERT_FAULT(
            mapper.convertWsmToCimValue(wsmValue, CIMNamespaceName(), cimValue),
            "wxf:InvalidRepresentation");
    }

    // Test special floating point values. The only special values allowed
    // are INF, -INF and NaN
    {
        CIMValue cimValue(CIMTYPE_REAL32, false);
        WsmValue wsmValue("+INF");
        ASSERT_FAULT(
            mapper.convertWsmToCimValue(wsmValue, CIMNamespaceName(), cimValue),
            "wxf:InvalidRepresentation");
    }

    {
        CIMValue cimValue(CIMTYPE_REAL32, false);
        WsmValue wsmValue("0x2");
        ASSERT_FAULT(
            mapper.convertWsmToCimValue(wsmValue, CIMNamespaceName(), cimValue),
            "wxf:InvalidRepresentation");
    }

    {
        CIMValue cimValue(CIMTYPE_REAL32, false);
        WsmValue wsmValue("nan");
        ASSERT_FAULT(
            mapper.convertWsmToCimValue(wsmValue, CIMNamespaceName(), cimValue),
            "wxf:InvalidRepresentation");
    }

    {
        CIMValue cimValue(CIMTYPE_REAL32, false);
        WsmValue wsmValue("inf");
        ASSERT_FAULT(
            mapper.convertWsmToCimValue(wsmValue, CIMNamespaceName(), cimValue),
            "wxf:InvalidRepresentation");
    }

    {
        CIMValue cimValue(CIMTYPE_REAL32, false);
        WsmValue wsmValue("i");
        ASSERT_FAULT(
            mapper.convertWsmToCimValue(wsmValue, CIMNamespaceName(), cimValue),
            "wxf:InvalidRepresentation");
    }

    // Invalid char16
    {
        CIMValue cimValue(CIMTYPE_CHAR16, false);
        WsmValue wsmValue("35.54.32");
        ASSERT_FAULT(
            mapper.convertWsmToCimValue(wsmValue, CIMNamespaceName(), cimValue),
            "wxf:InvalidRepresentation");
    }

    // Invalid class URI
    {
        ASSERT_FAULT(
            mapper.convertResourceUriToClassName("garbage"),
            "wsa:DestinationUnreachable");
    }

    {
        String classURI = String(WSM_RESOURCEURI_CIMSCHEMAV2) + "/My{}Class";
        ASSERT_FAULT(
            mapper.convertResourceUriToClassName(classURI),
            "wsa:DestinationUnreachable");
    }

    // Invalid date/time
    {
        CIMDateTime cimDT;

        // Invalid dates
        ASSERT_FAULT(
            mapper.convertWsmToCimDatetime("-2004-12-01", cimDT),
            "wxf:InvalidRepresentation");
        ASSERT_FAULT(
            mapper.convertWsmToCimDatetime("+2004-12-01", cimDT),
            "wxf:InvalidRepresentation");
        ASSERT_FAULT(
            mapper.convertWsmToCimDatetime("2004-+2-01", cimDT),
            "wxf:InvalidRepresentation");
        ASSERT_FAULT(
            mapper.convertWsmToCimDatetime("2004-12- 1", cimDT),
            "wxf:InvalidRepresentation");
        ASSERT_FAULT(
            mapper.convertWsmToCimDatetime("322004-12-01", cimDT),
            "wxf:InvalidRepresentation");
        ASSERT_FAULT(
            mapper.convertWsmToCimDatetime("2004-112-01", cimDT),
            "wxf:InvalidRepresentation");
        ASSERT_FAULT(
            mapper.convertWsmToCimDatetime("2004-12-00", cimDT),
            "wxf:InvalidRepresentation");
        ASSERT_FAULT(
            mapper.convertWsmToCimDatetime("2004-12:01", cimDT),
            "wxf:InvalidRepresentation");
        ASSERT_FAULT(
            mapper.convertWsmToCimDatetime("2004-12-01ZX", cimDT),
            "wxf:InvalidRepresentation");
        ASSERT_FAULT(
            mapper.convertWsmToCimDatetime("2004-12-01Z+01:30", cimDT),
            "wxf:InvalidRepresentation");
        ASSERT_FAULT(
            mapper.convertWsmToCimDatetime("2004-12-01+001:30", cimDT),
            "wxf:InvalidRepresentation");
        ASSERT_FAULT(
            mapper.convertWsmToCimDatetime("2004-12-01+01:030", cimDT),
            "wxf:InvalidRepresentation");
        ASSERT_FAULT(
            mapper.convertWsmToCimDatetime("2004-12-0101:30", cimDT),
            "wxf:InvalidRepresentation");
        ASSERT_FAULT(
            mapper.convertWsmToCimDatetime("2004-12-01+25:30", cimDT),
            "wxf:InvalidRepresentation");
         ASSERT_FAULT(
            mapper.convertWsmToCimDatetime("2004-12-01+22:66", cimDT),
            "wxf:InvalidRepresentation");
       ASSERT_FAULT(
            mapper.convertWsmToCimDatetime("2004-12-01+++1:3", cimDT),
            "wxf:InvalidRepresentation");
        ASSERT_FAULT(
            mapper.convertWsmToCimDatetime("2004-12-01-01:-6", cimDT),
            "wxf:InvalidRepresentation");
        ASSERT_FAULT(
            mapper.convertWsmToCimDatetime("2004-12-01-01:66", cimDT),
            "wxf:InvalidRepresentation");
        ASSERT_FAULT(
            mapper.convertWsmToCimDatetime("2004-12-01Z+:.", cimDT),
            "wxf:InvalidRepresentation");
        ASSERT_FAULT(
            mapper.convertWsmToCimDatetime("4324g432", cimDT),
            "wxf:InvalidRepresentation");
        ASSERT_FAULT(
            mapper.convertWsmToCimDatetime("a-e-3Z", cimDT),
            "wxf:InvalidRepresentation");

        // Invalid Datetimes
        ASSERT_FAULT(
            mapper.convertWsmToCimDatetime("2004-12-01T", cimDT),
            "wxf:InvalidRepresentation");
        ASSERT_FAULT(
            mapper.convertWsmToCimDatetime("2004-12-01T11:22:33X", cimDT),
            "wxf:InvalidRepresentation");
        ASSERT_FAULT(
            mapper.convertWsmToCimDatetime("2004-12-01T11:22:33.Z", cimDT),
            "wxf:InvalidRepresentation");
        ASSERT_FAULT(
            mapper.convertWsmToCimDatetime("2004-12-01T11:22:33.44Z0", cimDT),
            "wxf:InvalidRepresentation");
        ASSERT_FAULT(
            mapper.convertWsmToCimDatetime("2004-12-01T11:22:33.44Z+0", cimDT),
            "wxf:InvalidRepresentation");
        ASSERT_FAULT(
            mapper.convertWsmToCimDatetime("004-12-01T11:22:33.44Z", cimDT),
            "wxf:InvalidRepresentation");
        ASSERT_FAULT(
            mapper.convertWsmToCimDatetime("2004-12-01T11Z22:33.44Z", cimDT),
            "wxf:InvalidRepresentation");
        ASSERT_FAULT(
            mapper.convertWsmToCimDatetime("2004-12-01T11:22:133.44Z", cimDT),
            "wxf:InvalidRepresentation");
        ASSERT_FAULT(
            mapper.convertWsmToCimDatetime("2004-12-01T11:22.133Z", cimDT),
            "wxf:InvalidRepresentation");
        ASSERT_FAULT(
            mapper.convertWsmToCimDatetime("--T::", cimDT),
            "wxf:InvalidRepresentation");
        ASSERT_FAULT(
            mapper.convertWsmToCimDatetime("1-1-1T1:1:1", cimDT),
            "wxf:InvalidRepresentation");
        ASSERT_FAULT(
            mapper.convertWsmToCimDatetime(" 004-12-01T11:22:33Z", cimDT),
            "wxf:InvalidRepresentation");
        ASSERT_FAULT(
            mapper.convertWsmToCimDatetime("2004-+2-01T11:22:33Z", cimDT),
            "wxf:InvalidRepresentation");
        ASSERT_FAULT(
            mapper.convertWsmToCimDatetime("2004-12- 1T11:22:33Z", cimDT),
            "wxf:InvalidRepresentation");
        ASSERT_FAULT(
            mapper.convertWsmToCimDatetime("2004-12-1T11:22:33Z", cimDT),
            "wxf:InvalidRepresentation");
        ASSERT_FAULT(
            mapper.convertWsmToCimDatetime("2004-12-11T-1:22:33Z", cimDT),
            "wxf:InvalidRepresentation");
        ASSERT_FAULT(
            mapper.convertWsmToCimDatetime("2004-12-11T11:+2:33Z", cimDT),
            "wxf:InvalidRepresentation");
        ASSERT_FAULT(
            mapper.convertWsmToCimDatetime("2004-12-11T11:22: 3Z", cimDT),
            "wxf:InvalidRepresentation");
        ASSERT_FAULT(
            mapper.convertWsmToCimDatetime("2004-12-11T11:22:33.X", cimDT),
            "wxf:InvalidRepresentation");
        ASSERT_FAULT(
            mapper.convertWsmToCimDatetime("2004-12-11T11:22:33.+1", cimDT),
            "wxf:InvalidRepresentation");
        ASSERT_FAULT(
            mapper.convertWsmToCimDatetime("2004-12-11T11:22:33.+1S", cimDT),
            "wxf:InvalidRepresentation");
        ASSERT_FAULT(
            mapper.convertWsmToCimDatetime("2004-12-11T11:22:33+25:00", cimDT),
            "wxf:InvalidRepresentation");
        ASSERT_FAULT(
            mapper.convertWsmToCimDatetime("2004-12-11T11:22:33+22:66", cimDT),
            "wxf:InvalidRepresentation");


        // Invalid intervals
        ASSERT_FAULT(
            mapper.convertWsmToCimDatetime("P", cimDT),
            "wxf:InvalidRepresentation");
        ASSERT_FAULT(
            mapper.convertWsmToCimDatetime("PT", cimDT),
            "wxf:InvalidRepresentation");
        ASSERT_FAULT(
            mapper.convertWsmToCimDatetime("P1YT", cimDT),
            "wxf:InvalidRepresentation");
        ASSERT_FAULT(
            mapper.convertWsmToCimDatetime("P100", cimDT),
            "wxf:InvalidRepresentation");
        ASSERT_FAULT(
            mapper.convertWsmToCimDatetime("PT100", cimDT),
            "wxf:InvalidRepresentation");
        ASSERT_FAULT(
            mapper.convertWsmToCimDatetime("PT100K", cimDT),
            "wxf:InvalidRepresentation");
        ASSERT_FAULT(
            mapper.convertWsmToCimDatetime("PT100Y", cimDT),
            "wxf:InvalidRepresentation");
        ASSERT_FAULT(
            mapper.convertWsmToCimDatetime("PT100D", cimDT),
            "wxf:InvalidRepresentation");
        ASSERT_FAULT(
            mapper.convertWsmToCimDatetime("P1M1Y", cimDT),
            "wxf:InvalidRepresentation");
        ASSERT_FAULT(
            mapper.convertWsmToCimDatetime("P1D1M", cimDT),
            "wxf:InvalidRepresentation");
        ASSERT_FAULT(
            mapper.convertWsmToCimDatetime("P1DT1M1H", cimDT),
            "wxf:InvalidRepresentation");
        ASSERT_FAULT(
            mapper.convertWsmToCimDatetime("P1DTM", cimDT),
            "wxf:InvalidRepresentation");
        ASSERT_FAULT(
            mapper.convertWsmToCimDatetime("P1YT1Y", cimDT),
            "wxf:InvalidRepresentation");
        ASSERT_FAULT(
            mapper.convertWsmToCimDatetime("P1YT1M1.S", cimDT),
            "wxf:InvalidRepresentation");
        ASSERT_FAULT(
            mapper.convertWsmToCimDatetime("P1YT1M1S.0", cimDT),
            "wxf:InvalidRepresentation");
        ASSERT_FAULT(
            mapper.convertWsmToCimDatetime("P1YT1H1.2M", cimDT),
            "wxf:InvalidRepresentation");
        ASSERT_FAULT(
            mapper.convertWsmToCimDatetime("P1YT1HT1M", cimDT),
            "wxf:InvalidRepresentation");
        ASSERT_FAULT(
            mapper.convertWsmToCimDatetime("P-1Y", cimDT),
            "wxf:InvalidRepresentation");
        ASSERT_FAULT(
            mapper.convertWsmToCimDatetime("P-1M", cimDT),
            "wxf:InvalidRepresentation");
        ASSERT_FAULT(
            mapper.convertWsmToCimDatetime("P-1D", cimDT),
            "wxf:InvalidRepresentation");
        ASSERT_FAULT(
            mapper.convertWsmToCimDatetime("PT-1H", cimDT),
            "wxf:InvalidRepresentation");
        ASSERT_FAULT(
            mapper.convertWsmToCimDatetime("PT-1M", cimDT),
            "wxf:InvalidRepresentation");
        ASSERT_FAULT(
            mapper.convertWsmToCimDatetime("PT-1S", cimDT),
            "wxf:InvalidRepresentation");
        ASSERT_FAULT(
            mapper.convertWsmToCimDatetime("P 1Y", cimDT),
            "wxf:InvalidRepresentation");
        ASSERT_FAULT(
            mapper.convertWsmToCimDatetime("P~Y", cimDT),
            "wxf:InvalidRepresentation");
        ASSERT_FAULT(
            mapper.convertWsmToCimDatetime("P22.33S", cimDT),
            "wxf:InvalidRepresentation");
        ASSERT_FAULT(
            mapper.convertWsmToCimDatetime("PT22.33S1Y", cimDT),
            "wxf:InvalidRepresentation");
        ASSERT_FAULT(
            mapper.convertWsmToCimDatetime("P999999999Y", cimDT),
            "wxf:InvalidRepresentation");
        ASSERT_FAULT(
            mapper.convertWsmToCimDatetime("P9999999999Y", cimDT),
            "wxf:InvalidRepresentation");
        ASSERT_FAULT(
            mapper.convertWsmToCimDatetime("P1Y9999999999S", cimDT),
            "wxf:InvalidRepresentation");
    }
}

static void _testInstances(void)
{
    const String NAMESPACE = "aa/bb";
    const String CLASSNAME = "MyClass";

    CIMRepository r(repositoryRoot, CIMRepository::MODE_XML);
    WsmToCimRequestMapper mapper(&r);

    // Create a repository entry
    r.createNameSpace(NAMESPACE);
    CIMClass cimClass(CLASSNAME);
    cimClass.addProperty(CIMProperty(CIMName("prop1"), String::EMPTY));
    cimClass.addProperty(CIMProperty(CIMName("prop2"), String::EMPTY));
    r.createClass(NAMESPACE, cimClass);

    CIMInstance cimInst;
    WsmInstance wsmInst(CLASSNAME);
    WsmValue val1("value1");
    WsmValue val2("value2");

    // Test mapping of instances
    wsmInst.addProperty(WsmProperty(String("prop1"), val1));
    wsmInst.addProperty(WsmProperty(String("prop2"), val2));
    mapper.convertWsmToCimInstance(wsmInst, NAMESPACE, cimInst);

    String str1, str2;
    PEGASUS_TEST_ASSERT(cimInst.getClassName().getString() == CLASSNAME);
    PEGASUS_TEST_ASSERT(
        cimInst.getProperty(0).getName().getString() == "prop1");
    PEGASUS_TEST_ASSERT(cimInst.getProperty(0).getType() == CIMTYPE_STRING);
    cimInst.getProperty(0).getValue().get(str1);
    PEGASUS_TEST_ASSERT(str1 == "value1");
    PEGASUS_TEST_ASSERT(
        cimInst.getProperty(1).getName().getString() == "prop2");
    PEGASUS_TEST_ASSERT(cimInst.getProperty(1).getType() == CIMTYPE_STRING);
    cimInst.getProperty(1).getValue().get(str2);
    PEGASUS_TEST_ASSERT(str2 == "value2");

    // Test mapping of instance values
    WsmValue wsmInstValue(wsmInst);
    CIMValue cimInstValue(CIMTYPE_INSTANCE, false);
    mapper.convertWsmToCimValue(wsmInstValue, NAMESPACE, cimInstValue);
    CIMInstance cimInst1;
    cimInstValue.get(cimInst1);
    PEGASUS_TEST_ASSERT(cimInst1.getClassName().getString() == CLASSNAME);
    PEGASUS_TEST_ASSERT(
        cimInst1.getProperty(0).getName().getString() == "prop1");
    PEGASUS_TEST_ASSERT(cimInst1.getProperty(0).getType() == CIMTYPE_STRING);
    cimInst1.getProperty(0).getValue().get(str1);
    PEGASUS_TEST_ASSERT(str1 == "value1");
    PEGASUS_TEST_ASSERT(
        cimInst1.getProperty(1).getName().getString() == "prop2");
    PEGASUS_TEST_ASSERT(cimInst1.getProperty(1).getType() == CIMTYPE_STRING);
    cimInst1.getProperty(1).getValue().get(str2);
    PEGASUS_TEST_ASSERT(str2 == "value2");

    // Test mapping of instance array values
    WsmInstance wsmInst1(CLASSNAME);
    WsmValue val3("value3");
    WsmValue val4("value4");
    wsmInst1.addProperty(WsmProperty(String("prop1"), val3));
    wsmInst1.addProperty(WsmProperty(String("prop2"), val4));
    Array<WsmInstance> wsmInstArray;
    wsmInstArray.append(wsmInst);
    wsmInstArray.append(wsmInst1);
    WsmValue wsmInstArrayValue(wsmInstArray);
    CIMValue cimInstArrayValue(CIMTYPE_INSTANCE, true);
    mapper.convertWsmToCimValue(
        wsmInstArrayValue, NAMESPACE, cimInstArrayValue);
    Array<CIMInstance> cimInstArray;
    cimInstArrayValue.get(cimInstArray);
    PEGASUS_TEST_ASSERT(cimInstArray.size() == 2);

    PEGASUS_TEST_ASSERT(
        cimInstArray[0].getClassName().getString() == CLASSNAME);
    PEGASUS_TEST_ASSERT(
        cimInstArray[0].getProperty(0).getName().getString() == "prop1");
    PEGASUS_TEST_ASSERT(
        cimInstArray[0].getProperty(0).getType() == CIMTYPE_STRING);
    cimInstArray[0].getProperty(0).getValue().get(str1);
    PEGASUS_TEST_ASSERT(str1 == "value1");
    PEGASUS_TEST_ASSERT(
        cimInstArray[0].getProperty(1).getName().getString() == "prop2");
    PEGASUS_TEST_ASSERT(
        cimInstArray[0].getProperty(1).getType() == CIMTYPE_STRING);
    cimInstArray[0].getProperty(1).getValue().get(str2);
    PEGASUS_TEST_ASSERT(str2 == "value2");

    PEGASUS_TEST_ASSERT(
        cimInstArray[1].getClassName().getString() == CLASSNAME);
    PEGASUS_TEST_ASSERT(
        cimInstArray[1].getProperty(0).getName().getString() == "prop1");
    PEGASUS_TEST_ASSERT(
        cimInstArray[1].getProperty(0).getType() == CIMTYPE_STRING);
    cimInstArray[1].getProperty(0).getValue().get(str1);
    PEGASUS_TEST_ASSERT(str1 == "value3");
    PEGASUS_TEST_ASSERT(
        cimInstArray[1].getProperty(1).getName().getString() == "prop2");
    PEGASUS_TEST_ASSERT(
        cimInstArray[1].getProperty(1).getType() == CIMTYPE_STRING);
    cimInstArray[1].getProperty(1).getValue().get(str2);
    PEGASUS_TEST_ASSERT(str2 == "value4");

    // Test non-existent class
    try
    {
        wsmInst.setClassName("garbage");
        mapper.convertWsmToCimInstance(wsmInst, NAMESPACE, cimInst);
        PEGASUS_TEST_ASSERT(0);
    }
    catch (CIMException& e)
    {
        PEGASUS_TEST_ASSERT(e.getCode() == CIM_ERR_NOT_FOUND);
    }

    // Test non-existent property
    {
        WsmValue val("value3");
        wsmInst.setClassName(CLASSNAME);
        wsmInst.addProperty(WsmProperty(String("prop3"), val));
        ASSERT_FAULT(
            mapper.convertWsmToCimInstance(wsmInst, NAMESPACE, cimInst),
            "wsman:SchemaValidationError");
    }
}

static void _testEPRs(void)
{
    const String NAMESPACE = "aa/bb";
    const String CLASSNAME = "MyClass";

    CIMRepository r(repositoryRoot, CIMRepository::MODE_XML);
    WsmToCimRequestMapper mapper(&r);

    // Test mapping of EPRs
    CIMObjectPath objectPath;
    WsmEndpointReference epr;
    epr.address = "http://www.acme.com:5988/wsman";
    epr.resourceUri = String(WSM_RESOURCEURI_CIMSCHEMAV2) + "/MyClass";
    epr.selectorSet->selectors.
        append(WsmSelector("prop1", "value1"));
    epr.selectorSet->selectors.
        append(WsmSelector("__cimnamespace", NAMESPACE));
    mapper.convertEPRToObjectPath(epr, objectPath);
    PEGASUS_TEST_ASSERT(objectPath.toString() ==
        "//www.acme.com/aa/bb:MyClass.prop1=\"value1\"");

    // Test mapping of EPR values
    WsmValue wsmEprValue(epr);
    CIMValue cimObjpathValue(CIMTYPE_REFERENCE, false);
    mapper.convertWsmToCimValue(wsmEprValue, NAMESPACE, cimObjpathValue);
    CIMObjectPath objectPath1;
    cimObjpathValue.get(objectPath1);
    PEGASUS_TEST_ASSERT(objectPath1.toString() ==
        "//www.acme.com/aa/bb:MyClass.prop1=\"value1\"");

    // Test mapping of EPR array values
    WsmEndpointReference epr1;
    epr1.address = "http://www.acme1.com:5988/wsman";
    epr1.resourceUri = String(WSM_RESOURCEURI_CIMSCHEMAV2) + "/MyClass";
    epr1.selectorSet->selectors.
        append(WsmSelector("prop1", "value2"));
    epr1.selectorSet->selectors.
        append(WsmSelector("__cimnamespace", NAMESPACE));
    Array<WsmEndpointReference> eprArray;
    eprArray.append(epr);
    eprArray.append(epr1);
    WsmValue wsmEprArrayValue(eprArray);
    CIMValue cimObjpathArrayValue(CIMTYPE_REFERENCE, true);
    mapper.convertWsmToCimValue(
        wsmEprArrayValue, NAMESPACE, cimObjpathArrayValue);
    Array<CIMObjectPath> objectPathArray;
    cimObjpathArrayValue.get(objectPathArray);
    PEGASUS_TEST_ASSERT(objectPathArray[0].toString() ==
        "//www.acme.com/aa/bb:MyClass.prop1=\"value1\"");
    PEGASUS_TEST_ASSERT(objectPathArray[1].toString() ==
        "//www.acme1.com/aa/bb:MyClass.prop1=\"value2\"");

    // Test invalid __cimnamespace selector type
    WsmEndpointReference epr2;
    epr2.address = "http://www.acme.com:5988/wsman";
    epr2.resourceUri = String(WSM_RESOURCEURI_CIMSCHEMAV2) + "/MyClass";
    epr2.selectorSet->selectors.
        append(WsmSelector("__cimnamespace", epr1));
    ASSERT_FAULT(
        mapper.convertEPRToObjectPath(epr2, objectPath),
        "wsman:InvalidSelectors");

    // Test illegal __cimnamespace name
    WsmEndpointReference epr3;
    epr3.address = "http://www.acme.com:5988/wsman";
    epr3.resourceUri = String(WSM_RESOURCEURI_CIMSCHEMAV2) + "/MyClass";
    epr3.selectorSet->selectors.
        append(WsmSelector("__cimnamespace", "garbage namespace #@!"));
    ASSERT_FAULT(
        mapper.convertEPRToObjectPath(epr3, objectPath),
        "wsman:InvalidSelectors");

    // Test illegal property name
    WsmEndpointReference epr4;
    epr4.address = "http://www.acme.com:5988/wsman";
    epr4.resourceUri = String(WSM_RESOURCEURI_CIMSCHEMAV2) + "/MyClass";
    epr4.selectorSet->selectors.
        append(WsmSelector("__cimnamespace", NAMESPACE));
    epr4.selectorSet->selectors.
        append(WsmSelector("prop 1", "value"));
    ASSERT_FAULT(
        mapper.convertEPRToObjectPath(epr4, objectPath),
        "wsman:InvalidSelectors");

    // Test non-existent property name
    WsmEndpointReference epr5;
    epr5.address = "http://www.acme.com:5988/wsman";
    epr5.resourceUri = String(WSM_RESOURCEURI_CIMSCHEMAV2) + "/MyClass";
    epr5.selectorSet->selectors.
        append(WsmSelector("__cimnamespace", NAMESPACE));
    epr5.selectorSet->selectors.
        append(WsmSelector("prop3", "value"));
    ASSERT_FAULT(
        mapper.convertEPRToObjectPath(epr5, objectPath),
        "wsman:InvalidSelectors");

    // Test type mismatch in key types
    WsmEndpointReference epr6;
    epr6.address = "https://www.acme.com:5988/wsman";
    epr6.resourceUri = String(WSM_RESOURCEURI_CIMSCHEMAV2) + "/MyClass";
    epr6.selectorSet->selectors.
        append(WsmSelector("__cimnamespace", NAMESPACE));
    epr6.selectorSet->selectors.
        append(WsmSelector("prop3", epr1));
    ASSERT_FAULT(
        mapper.convertEPRToObjectPath(epr6, objectPath),
        "wsman:InvalidSelectors");

    // Test anonymous address in EPR
    String addr1 = mapper.convertEPRAddressToHostname(WSM_ADDRESS_ANONYMOUS);
    PEGASUS_TEST_ASSERT(addr1 == String::EMPTY);

    // Test malformed EPR addresses
    ASSERT_FAULT(
        mapper.convertEPRAddressToHostname("garbage"),
        "wsa:InvalidMessageInformationHeader");

    ASSERT_FAULT(
        mapper.convertEPRAddressToHostname("http://blah"),
        "wsa:InvalidMessageInformationHeader");

    ASSERT_FAULT(
        mapper.convertEPRAddressToHostname("http://bsa#@^&sa/wsman"),
        "wsa:InvalidMessageInformationHeader");
}

int main(int argc, char** argv)
{
    verbose = getenv("PEGASUS_TEST_VERBOSE") ? true : false;

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
        if (verbose)
        {
            cout << "Testing simple values and arrays." << endl;
        }
        _testValues();

        if (verbose)
        {
            cout << "Testing instances." << endl;
        }
        _testInstances();

        if (verbose)
        {
            cout << "Testing EPRs." << endl;
        }
        _testEPRs();

        if (verbose)
        {
            cout << "Testing conversion errors." << endl;
        }
        _testConversionErrors();
    }
    catch (Exception& e)
    {
        cerr << "Error: " << e.getMessage() << endl;
        exit(1);
    }
    catch (WsmFault& f)
    {
        cerr << "Error: " << f.getSubcode() << " " << f.getReason() << endl;
        exit(1);
    }

    FileSystem::removeDirectoryHier(repositoryRoot);

    cout << argv[0] << " +++++ passed all tests" << endl;

    return 0;
}
