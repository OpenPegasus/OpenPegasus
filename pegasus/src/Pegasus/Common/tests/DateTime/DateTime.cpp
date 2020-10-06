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
#include <Pegasus/Common/CIMDateTime.h>
#include <Pegasus/Common/CIMValue.h>
#include <Pegasus/Common/Exception.h>

PEGASUS_USING_PEGASUS;
PEGASUS_USING_STD;

void put(const char *msg, const CIMDateTime & x)
{
    cout << msg << "[" << x.toString() << ":" <<
        CIMValue(x.toMicroSeconds()).toString() << "]";
    cout << endl;
}

int main(int, char **argv)
{
    try
    {
        // ATTN-P2-KS 20 Mar 2002 - Needs expansion of tests.
        // get the output display flag.
        Boolean verbose = (getenv("PEGASUS_TEST_VERBOSE")) ? true : false;

        CIMDateTime dt;
        dt.set("19991224120000.000000+360");

        dt.clear();
        PEGASUS_TEST_ASSERT(dt.equal(CIMDateTime("00000000000000.000000:000")));

        {
            Boolean bad = false;

            try
            {
                dt.set("too short");
            }
            catch(InvalidDateTimeFormatException &)
            {
                bad = true;
            }

            PEGASUS_TEST_ASSERT(bad);
        }

        {
            Boolean bad = false;

            try
            {
                dt.set("too short");
                dt.set("19990132120000.000000+360");
            }
            catch(InvalidDateTimeFormatException &)
            {
                bad = true;
            }

            PEGASUS_TEST_ASSERT(bad);
        }
        if (verbose)
            cout << dt.toString() << endl;

        CIMDateTime dtcopy;
        dtcopy = dt;


    /****************************************************************
              testing function added in PEP 192
    *******************************************************************/


        Boolean te1 = false;


        CIMDateTime te;
        try
        {
            te.set("2000122412****.002000+360");        // this is not leagal
        }
        catch(const InvalidDateTimeFormatException &)
        {
            te1 = true;
        }
        PEGASUS_TEST_ASSERT(te1);
        te1 = false;


        String stri = "20001224122***.******+360";
        try
        {
            CIMDateTime st(stri);
        }
        catch(const InvalidDateTimeFormatException &)
        {
            te1 = true;
        }
        PEGASUS_TEST_ASSERT(te1);
        te1 = false;

        /* makes sure the fields has splats starting form least significant
           place and going up */
        String str_test2 = "20001224120000.002*00+360";
        try
        {
            CIMDateTime cdt_test2(str_test2);
        }
        catch(const InvalidDateTimeFormatException &)
        {
            te1 = true;
        }
        PEGASUS_TEST_ASSERT(te1);
        te1 = false;

        try
        {
            CIMDateTime stt("200012*4120000.002000+360");
        }
        catch(const InvalidDateTimeFormatException &)
        {
            te1 = true;
        }
        PEGASUS_TEST_ASSERT(te1);
        te1 = false;


        /* check for UTC */
        String str_test1 = "20001012010920.002000+3*0";
        try
        {
            CIMDateTime cdt_test1(str_test1);
        }
        catch(const InvalidDateTimeFormatException &)
        {
            te1 = true;
        }
        PEGASUS_TEST_ASSERT(te1);


        /* check days and UTC field when object is an interval */

        String str_test3 = "20001012010920.002000:000";
        try
        {
            CIMDateTime cdt_test3(str_test3);
        }
        catch(const InvalidDateTimeFormatException &)
        {
            te1 = false;
        }
        PEGASUS_TEST_ASSERT(te1);
        te1 = false;


        String str_test4 = "20001012010920.002000:100";
        try
        {
            CIMDateTime cdt_test4(str_test4);
        }
        catch(const InvalidDateTimeFormatException &)
        {
            te1 = true;
        }
        PEGASUS_TEST_ASSERT(te1);
        te1 = false;

        // Check for invalid non-ASCII character
        try
        {
            String str = "20001012010920.002000:000";
            str[7] = Char16(0x0132);
            CIMDateTime cdt(str);
        }
        catch(const InvalidDateTimeFormatException &)
        {
            te1 = true;
        }
        PEGASUS_TEST_ASSERT(te1);
        te1 = false;

        // verify CIMDateTime::CIMDateTime(const Uint64 microSec, Boolean)
        // and toMicroSeconds()
        try
        {
            Uint64 co = 1;
            while (co < 20)
            {
                Uint64 num_day = PEGASUS_UINT64_LITERAL(86400000000) * co;
                CIMDateTime cdt1(num_day, true);
                PEGASUS_TEST_ASSERT(cdt1.toMicroSeconds() ==
                                    PEGASUS_UINT64_LITERAL(86400000000) * co);
                co = co + 2;
            }
        }
        catch(...)
        {
            PEGASUS_TEST_ASSERT(false);
        }


        {
            CIMDateTime cdt("00010101000000.000000+000");
            Uint64 us = cdt.toMicroSeconds();
            PEGASUS_TEST_ASSERT(us == (Uint64) 366 * 24 * 60 * 60 * 1000000);
        }

        {
            CIMDateTime cdt("00000001000000.123456:000");
            Uint64 us = cdt.toMicroSeconds();
            PEGASUS_TEST_ASSERT(us == PEGASUS_UINT64_LITERAL(86400123456));
        }

        {
            CIMDateTime cdt("00000101000000.123456+000");
            Uint64 us = cdt.toMicroSeconds();
            PEGASUS_TEST_ASSERT(us == PEGASUS_UINT64_LITERAL(123456));
        }

        {
            CIMDateTime cdt("00000000030000.123456:000");
            Uint64 us = cdt.toMicroSeconds();
            PEGASUS_TEST_ASSERT(us == (Uint64) 3 * 60 * 60 * 1000000 + 123456);
        }


        /* CIMDateTime con1(864003660000000,false); //CIMDateTime
           con_comp("00270519010100.000000+000"); //if (!con1.equal(con_comp))
           { PEGASUS_TEST_ASSERT(false); } const Uint64 conr1 =
           con1.toMicroSeconds(); if (conr1 != 864003660000000) {
           PEGASUS_TEST_ASSERT(false); } */


        {
            Uint64 n = PEGASUS_UINT64_LITERAL(8640000000000000);
            PEGASUS_TEST_ASSERT(n == CIMDateTime(n, false).toMicroSeconds());
        }

        {
            Uint64 n = PEGASUS_UINT64_LITERAL(8637408000000000);
            PEGASUS_TEST_ASSERT(n == CIMDateTime(n, false).toMicroSeconds());
        }


        CIMDateTime date1("20041014002133.123456+000");
        Uint64 mic1 = date1.toMicroSeconds();
        CIMDateTime result1(mic1, false);
        if (!date1.equal(result1))
        {
            cout << result1.
                toString() << " was returned for constructor and should equal "
                << date1.toString() << endl;
            PEGASUS_TEST_ASSERT(false);
        }


        CIMDateTime date2("20000714000233.123456+000");
        Uint64 mic2 = date2.toMicroSeconds();
        CIMDateTime result2(mic2, false);
        if (!date2.equal(result2))
        {
            cout << result2.
                toString() << " was returned for constructor and should equal "
                << date2.toString() << endl;
            PEGASUS_TEST_ASSERT(false);
        }


        CIMDateTime date3("90001014100000.000456+000");
        Uint64 mic3 = date3.toMicroSeconds();
        CIMDateTime result3(mic3, false);
        if (!date3.equal(result3))
        {
            cout << result3.
                toString() << " was returned for constructor and should equal "
                << date3.toString() << endl;
            PEGASUS_TEST_ASSERT(false);
        }


        CIMDateTime date4("01234567123456.123456:000");
        Uint64 mic4 = date4.toMicroSeconds();
        CIMDateTime result4(mic4, true);
        if (!date4.equal(result4))
        {
            cout << result4.
                toString() << " was returned for constructor and should equal "
                << date4.toString() << endl;
            PEGASUS_TEST_ASSERT(false);
        }


        CIMDateTime date5("81234567123456.123456:000");
        Uint64 mic5 = date5.toMicroSeconds();
        CIMDateTime result5(mic5, true);
        if (!date5.equal(result5))
        {
            cout << result5.
                toString() << " was returned for constructor and should equal "
                << date5.toString() << endl;
            PEGASUS_TEST_ASSERT(false);
        }


        CIMDateTime date6("00000000123456.123456:000");
        Uint64 mic6 = date6.toMicroSeconds();
        CIMDateTime result6(mic6, true);
        if (!date6.equal(result6))
        {
            cout << result6.
                toString() << " was returned for constructor and should equal "
                << date6.toString() << endl;
            PEGASUS_TEST_ASSERT(false);
        }

        // Test overflow on timestamp construction from microseconds
        {
            CIMDateTime dto("99991231235959.999999-000");
            Uint64 microseconds = dto.toMicroSeconds();
            Boolean gotException = false;
            try
            {
                CIMDateTime x(microseconds + 1, false);
            }
            catch(const DateTimeOutOfRangeException &)
            {
                gotException = true;
            }
            PEGASUS_TEST_ASSERT(gotException);
        }

        // Test overflow on timestamp construction from microseconds from UTC
        {
            Boolean gotException = false;
            try
            {
                CIMDateTime dto("99991231235959.999999-001");
                CIMDateTime x(dto.toMicroSeconds(), false);
            }
            catch(const DateTimeOutOfRangeException &)
            {
                gotException = true;
            }
            PEGASUS_TEST_ASSERT(gotException);
        }

        // Test overflow on interval construction from microseconds
        {
            CIMDateTime dto("99999999235959.999999:000");
            Uint64 microseconds = dto.toMicroSeconds();
            Boolean gotException = false;
            try
            {
                CIMDateTime x(microseconds + 1, true);
            }
            catch(const DateTimeOutOfRangeException &)
            {
                gotException = true;
            }
            PEGASUS_TEST_ASSERT(gotException);
        }

        // Test overflow on timestamp construction from large, valid interval
        {
            CIMDateTime dto("99999999235959.999999:000");
            Uint64 microseconds = dto.toMicroSeconds();
            Boolean gotException = false;
            try
            {
                CIMDateTime x(microseconds, false);
            }
            catch(const DateTimeOutOfRangeException &)
            {
                gotException = true;
            }
            PEGASUS_TEST_ASSERT(gotException);
        }

        // testing toMicroSeconds when object has wild cards

        CIMDateTime tms_cdt1("00000000000000.12****:000");
        Uint64 ms1 = tms_cdt1.toMicroSeconds();
        if (ms1 != 120000)
        {
            cout << "problem converting " << tms_cdt1.
                toString() << " to MicroSeconds" << endl;
            PEGASUS_TEST_ASSERT(false);
        }

        CIMDateTime tms_cdt2("00000101000000.1234**+000");
        Uint64 ms2 = tms_cdt2.toMicroSeconds();
        if (ms2 != 123400)
        {
            cout << "problem converting " << tms_cdt2.
                toString() << " to MicroSeconds" << endl;
            PEGASUS_TEST_ASSERT(false);
        }

        CIMDateTime tms_cdt3("000000000001**.******:000");
        Uint64 ms3 = tms_cdt3.toMicroSeconds();
        if (ms3 != 60000000)
        {
            cout << "problem converting " << tms_cdt3.
                toString() << " to MicroSeconds" << endl;
            PEGASUS_TEST_ASSERT(false);
        }

        CIMDateTime tms_cdt4("**************.******+000");
        Uint64 ms4 = tms_cdt4.toMicroSeconds();
        if (ms4 != PEGASUS_UINT64_LITERAL(0))
        {
            cout << "problem converting " << tms_cdt4.
                toString() << " to MicroSeconds" << endl;
            PEGASUS_TEST_ASSERT(false);
        }

        /* verifying CIMDateTime::operator= */


        CIMDateTime org_vCo1("**************.******+000");
        CIMDateTime se_vCo1("99931231230200.000000-127");
        org_vCo1 = se_vCo1;
        if (String::compare(org_vCo1.toString(), se_vCo1.toString()))
        {
            PEGASUS_TEST_ASSERT(false);
        }

        CIMDateTime org_vCo2("12340101010101.******+000");
        CIMDateTime se_vCo2("00001231230200.000000-127");
        org_vCo2 = se_vCo2;
        if (String::compare(org_vCo2.toString(), se_vCo2.toString()))
        {
            PEGASUS_TEST_ASSERT(false);
        }



        /* testing equals() */

        if (!org_vCo1.equal(se_vCo1))
        {
            PEGASUS_TEST_ASSERT(false);
        }


        if (!org_vCo2.equal(se_vCo2))
        {
            PEGASUS_TEST_ASSERT(false);
        }

        CIMDateTime teeq3("12340101010101.******+000");
        CIMDateTime teeq3_comp("12340101010101.123456+000");
        if (!teeq3.equal(teeq3_comp))
        {
            PEGASUS_TEST_ASSERT(false);
        }

        CIMDateTime teeq4("12340101010101.212345+000");
        CIMDateTime teeq4_comp("12340101010101.2*****+000");
        if (!teeq4.equal(teeq4_comp))
        {
            PEGASUS_TEST_ASSERT(false);
        }

        CIMDateTime teeq5("12340101010101.2*****+000");
        CIMDateTime teeq5_comp("12340101010101.2234**+000");
        if (!teeq5.equal(teeq5_comp))
        {
            PEGASUS_TEST_ASSERT(false);
        }

        CIMDateTime teeq6("12340101010101.2*****+000");
        CIMDateTime teeq6_comp("123401010101**.******+000");
        if (!teeq6.equal(teeq6_comp))
        {
            PEGASUS_TEST_ASSERT(false);
        }

        CIMDateTime teeq7("12340101010101.2*****+000");
        CIMDateTime teeq7_comp("1234010101****.******+030");
        if (!teeq7.equal(teeq7_comp))
        {
            PEGASUS_TEST_ASSERT(false);
        }


        //
        // Tests for getDifference.
        //

        // One second difference
        {
            CIMDateTime dt1("20040520041400.000000+000");
            CIMDateTime dt2("20040520041401.000000+000");
            Sint64 diff = CIMDateTime::getDifference(dt1, dt2);
            PEGASUS_TEST_ASSERT(diff == 1000000);
        }

        // One day difference, with wildcards
        {
            CIMDateTime dt1("20040520041400.000000+050");
            CIMDateTime dt2("20040521041400.00****+050");
            Sint64 diff = CIMDateTime::getDifference(dt1, dt2);
            PEGASUS_TEST_ASSERT(diff == PEGASUS_SINT64_LITERAL(86400000000));
        }

        // UTC difference, with wildcards
        {
            CIMDateTime dt1("20040520041400.000***+000");
            CIMDateTime dt2("20040520041400.000000+120");
            Sint64 diff = CIMDateTime::getDifference(dt1, dt2);
            PEGASUS_TEST_ASSERT(diff == -PEGASUS_SINT64_LITERAL(7200000000));
        }

        // Century non-leap year
        {
            CIMDateTime dt1("19000228041400.000000+000");
            CIMDateTime dt2("19000301041400.000000+000");
            Sint64 diff = CIMDateTime::getDifference(dt1, dt2);
            PEGASUS_TEST_ASSERT(diff == PEGASUS_SINT64_LITERAL(86400000000));
        }

        // Century leap year
        {
            CIMDateTime dt1("20000229041400.000000+000");
            CIMDateTime dt2("20000301041400.000000+000");
            Sint64 diff = CIMDateTime::getDifference(dt1, dt2);
            PEGASUS_TEST_ASSERT(diff == PEGASUS_SINT64_LITERAL(86400000000));
        }

        // Non-leap year
        {
            CIMDateTime dt1("20030228041400.000000+000");
            CIMDateTime dt2("20030301041400.000000+000");
            Sint64 diff = CIMDateTime::getDifference(dt1, dt2);
            PEGASUS_TEST_ASSERT(diff == PEGASUS_SINT64_LITERAL(86400000000));
        }

        // Regular leap year
        {
            CIMDateTime dt1("20040229041400.000000+000");
            CIMDateTime dt2("20040301041400.000000+000");
            Sint64 diff = CIMDateTime::getDifference(dt1, dt2);
            PEGASUS_TEST_ASSERT(diff == PEGASUS_SINT64_LITERAL(86400000000));
        }

        // Non-leap year
        {
            CIMDateTime dt1("20050228041400.000000+000");
            CIMDateTime dt2("20050301041400.000000+000");
            Sint64 diff = CIMDateTime::getDifference(dt1, dt2);
            PEGASUS_TEST_ASSERT(diff == PEGASUS_SINT64_LITERAL(86400000000));
        }

        // UTC difference
        {
            CIMDateTime dt1("20020507170000.000000-300");
            CIMDateTime dt2("20020507170000.000000-480");
            Sint64 diff = CIMDateTime::getDifference(dt1, dt2);
            PEGASUS_TEST_ASSERT(diff == PEGASUS_SINT64_LITERAL(10800000000));
        }

        // Negative UTC difference
        {
            CIMDateTime dt1("20020507170000.000000-480");
            CIMDateTime dt2("20020507170000.000000-300");
            Sint64 diff = CIMDateTime::getDifference(dt1, dt2);
            PEGASUS_TEST_ASSERT(diff == -PEGASUS_SINT64_LITERAL(10800000000));
        }

        // Another UTC difference
        {
            CIMDateTime dt1("20020507170000.000000+330");
            CIMDateTime dt2("20020507170000.000000-480");
            Sint64 diff = CIMDateTime::getDifference(dt1, dt2);
            PEGASUS_TEST_ASSERT(diff == PEGASUS_SINT64_LITERAL(48600000000));
        }

        // Another negative UTC difference
        {
            CIMDateTime dt1("20020507170000.000000-480");
            CIMDateTime dt2("20020507170000.000000+330");
            Sint64 diff = CIMDateTime::getDifference(dt1, dt2);
            PEGASUS_TEST_ASSERT(diff == -PEGASUS_SINT64_LITERAL(48600000000));
        }

        // Microseconds difference
        {
            CIMDateTime dt1("20020507170000.000000-300");
            CIMDateTime dt2("20020507170000.000003-480");
            Sint64 diff = CIMDateTime::getDifference(dt1, dt2);
            PEGASUS_TEST_ASSERT(diff == PEGASUS_SINT64_LITERAL(10800000003));
        }

        // Microseconds difference
        {
            CIMDateTime dt1("20020507170000.000003-300");
            CIMDateTime dt2("20020507170000.000000-480");
            Sint64 diff = CIMDateTime::getDifference(dt1, dt2);
            PEGASUS_TEST_ASSERT(diff == PEGASUS_SINT64_LITERAL(10799999997));
        }

        // One day difference a long time ago
        {
            CIMDateTime dt1("19011214000000.000000-000");
            CIMDateTime dt2("19011215000000.000000-000");
            Sint64 diff = CIMDateTime::getDifference(dt1, dt2);
            PEGASUS_TEST_ASSERT(diff == PEGASUS_SINT64_LITERAL(86400000000));
        }

        // One day difference a long time into the future
        {
            CIMDateTime dt1("20370101000000.000000-000");
            CIMDateTime dt2("20370102000000.000000-000");
            Sint64 diff = CIMDateTime::getDifference(dt1, dt2);
            PEGASUS_TEST_ASSERT(diff == PEGASUS_SINT64_LITERAL(86400000000));
        }

        // One minute interval difference
        {
            CIMDateTime dt1("00000001010100.000000:000");
            CIMDateTime dt2("00000001010200.000000:000");
            PEGASUS_TEST_ASSERT(dt1.isInterval());
            PEGASUS_TEST_ASSERT(dt2.isInterval());
            Sint64 diff = CIMDateTime::getDifference(dt1, dt2);
            PEGASUS_TEST_ASSERT(diff == 60000000);
        }

        // Largest interval difference
        {
            CIMDateTime dt1("00000000000000.000000:000");
            CIMDateTime dt2("99999999235959.999999:000");
            PEGASUS_TEST_ASSERT(dt1.isInterval());
            PEGASUS_TEST_ASSERT(dt2.isInterval());
            Sint64 diff = CIMDateTime::getDifference(dt1, dt2);
            PEGASUS_TEST_ASSERT(diff ==
                                PEGASUS_SINT64_LITERAL(8639999999999999999));
        }

        // Mismatched interval and timestamp
        {
            CIMDateTime dt1("00000001010100.000000:000");
            CIMDateTime dt2("20020507170000.000000-480");
            PEGASUS_TEST_ASSERT(dt1.isInterval());
            PEGASUS_TEST_ASSERT(!dt2.isInterval());
            Boolean gotException = false;
            try
            {
                CIMDateTime::getDifference(dt1, dt2);
            }
            catch(const InvalidDateTimeFormatException &)
            {
                gotException = true;
            }
            PEGASUS_TEST_ASSERT(gotException);
        }

        // Mismatched timestamp and interval
        {
            CIMDateTime dt1("20040229041400.000000+000");
            CIMDateTime dt2("99999999235959.999999:000");
            PEGASUS_TEST_ASSERT(!dt1.isInterval());
            PEGASUS_TEST_ASSERT(dt2.isInterval());
            Boolean gotException = false;
            try
            {
                CIMDateTime::getDifference(dt1, dt2);
            }
            catch(const InvalidDateTimeFormatException &)
            {
                gotException = true;
            }
            PEGASUS_TEST_ASSERT(gotException);
        }


        //
        // Tests for getCurrentDateTime
        //
        {
            CIMDateTime currentTime1;
            CIMDateTime currentTime2;
            Boolean gotException = false;

            try
            {
                currentTime1 = CIMDateTime::getCurrentDateTime();
                currentTime2 = CIMDateTime::getCurrentDateTime();
            }
            catch(...)
            {
                gotException = true;
            }

            PEGASUS_TEST_ASSERT(!gotException);

            // The literal value is approximately February 1, 2006.
            PEGASUS_TEST_ASSERT(currentTime1.toMicroSeconds() >
                                PEGASUS_UINT64_LITERAL(63306000000000000));

            // We don't expect the two getCurrentDateTime calls to happen more
            // than a second apart.
            PEGASUS_TEST_ASSERT(CIMDateTime::
                                getDifference(currentTime1,
                                              currentTime2) <= 1000000);
        }


        /*********************************************************
                testing operators added in PEP 192
     ************************************************************/

        // testing for operator+

        Uint64 tfo_n = PEGASUS_UINT64_LITERAL(123456732445);
        Uint64 tfo_nA = PEGASUS_UINT64_LITERAL(6789143423);
        CIMDateTime tfo_nCDT(tfo_n, true);
        CIMDateTime tfo_nACDT(tfo_nA, true);
        Uint64 sum_n = tfo_n + tfo_nA;
        CIMDateTime sum_CDT(sum_n, true);
        CIMDateTime CDT_sum = tfo_nCDT + tfo_nACDT;
        if (String::compare(CDT_sum.toString(), sum_CDT.toString()))
        {
            PEGASUS_TEST_ASSERT(false);
        }


        Uint64 tfo_n2 = PEGASUS_UINT64_LITERAL(1235764732445);
        Uint64 tfo_nA2 = PEGASUS_UINT64_LITERAL(6789143423);
        CIMDateTime tfo_nCDT2(tfo_n2, false);
        CIMDateTime tfo_nACDT2(tfo_nA2, true);
        Uint64 sum_n2 = tfo_n2 + tfo_nA2;
        CIMDateTime sum_CDT2(sum_n2, false);
        CIMDateTime CDT_sum2 = tfo_nCDT2 + tfo_nACDT2;
        if (String::compare(CDT_sum2.toString(), sum_CDT2.toString()))
        {
            PEGASUS_TEST_ASSERT(false);
        }


        Uint64 tfo_n3 = PEGASUS_UINT64_LITERAL(123456732445134);
        Uint64 tfo_nA3 = PEGASUS_UINT64_LITERAL(678947563423);
        CIMDateTime tfo_nCDT3(tfo_n3, false);
        CIMDateTime tfo_nACDT3(tfo_nA3, true);
        Uint64 sum_n3 = tfo_n3 + tfo_nA3;
        CIMDateTime sum_CDT3(sum_n3, false);
        CIMDateTime CDT_sum3 = tfo_nCDT3 + tfo_nACDT3;
        if (String::compare(CDT_sum3.toString(), sum_CDT3.toString()))
        {
            PEGASUS_TEST_ASSERT(false);
        }


        Uint64 tfo_n4 = PEGASUS_UINT64_LITERAL(123456732445134);
        Uint64 tfo_nA4 = PEGASUS_UINT64_LITERAL(678947563423);
        CIMDateTime tfo_nCDT4(tfo_n4, false);
        CIMDateTime tfo_nACDT4(tfo_nA4, true);
        Uint64 sum_n4 = tfo_n4 + tfo_nA4;
        CIMDateTime sum_CDT4(sum_n4, false);
        CIMDateTime CDT_sum4 = tfo_nCDT4 + tfo_nACDT4;
        if (String::compare(CDT_sum4.toString(), sum_CDT4.toString()))
        {
            PEGASUS_TEST_ASSERT(false);
        }


        CIMDateTime tfo_n5("20040520041400.0*****+450");
        CIMDateTime tfo_nA5("000405200414**.******:000");
        Uint64 num_n5 = tfo_n5.toMicroSeconds();
        Uint64 num_nA5 = tfo_nA5.toMicroSeconds();
        Uint64 sum5 = num_n5 + num_nA5;
        CIMDateTime sum_cdt5(sum5, false);
        CIMDateTime cdt_sum5 = tfo_n5 + tfo_nA5;
        if (String::compare(sum_cdt5.toString(), sum_cdt5.toString()))
        {
            PEGASUS_TEST_ASSERT(false);
        }

/*************************************************************************/

        // testing operator+=


        CIMDateTime top_cdt0("000405200414**.******:000");
        CIMDateTime topA_cdt0("00000520041452.******:000");
        Uint64 top_num0 = top_cdt0.toMicroSeconds();
        Uint64 topA_num0 = topA_cdt0.toMicroSeconds();
        Uint64 tot_num0 = top_num0 + topA_num0;
        top_cdt0 += topA_cdt0;
        CIMDateTime topA0(tot_num0, true);
        if (top_cdt0 != topA0)
        {
            PEGASUS_TEST_ASSERT(false);
        }


        CIMDateTime top_cdt2("00040520041412.1234**+000");
        CIMDateTime topA_cdt2("00000520041452.******:000");
        Uint64 top_num2 = top_cdt2.toMicroSeconds();
        Uint64 topA_num2 = topA_cdt2.toMicroSeconds();
        Uint64 tot_num2 = top_num2 + topA_num2;
        top_cdt2 += topA_cdt2;
        CIMDateTime topA2(tot_num2, false);
        if (top_cdt2 != topA2)
        {
            PEGASUS_TEST_ASSERT(false);
        }



        CIMDateTime top_cdt3("00020220******.******+230");
        CIMDateTime topA_cdt3("00000220041452.******:000");
        Uint64 top_num3 = top_cdt3.toMicroSeconds();
        Uint64 topA_num3 = topA_cdt3.toMicroSeconds();
        Uint64 tot_num3 = top_num3 + topA_num3;
        top_cdt3 += topA_cdt3;
        Uint64 top_3n = top_cdt3.toMicroSeconds();
        PEGASUS_TEST_ASSERT(0 != top_3n);
        CIMDateTime tA(tot_num3, false);
        if (tA != top_cdt3)
        {
            PEGASUS_TEST_ASSERT(false);
        }



        CIMDateTime top_cdt4("00040520041412.123435+000");
        CIMDateTime topA_cdt4("00000520041452.123456:000");
        Uint64 top_num4 = top_cdt4.toMicroSeconds();
        Uint64 topA_num4 = topA_cdt4.toMicroSeconds();
        Uint64 tot_num4 = top_num4 + topA_num4;
        CIMDateTime tot_cdt4(tot_num4, false);
        // tot_cdt4 = tot_cdt4.setUctOffSet(-100);
        top_cdt4 += topA_cdt4;
        if (String::compare(top_cdt4.toString(), tot_cdt4.toString()))
        {
            PEGASUS_TEST_ASSERT(false);
        }


/*************************************************************************/

        // "testing the operator-


        Uint64 tfo_m = PEGASUS_UINT64_LITERAL(123456732445);
        Uint64 tfo_mA = PEGASUS_UINT64_LITERAL(6789143423);
        CIMDateTime tfo_mCDT(tfo_m, true);
        CIMDateTime tfo_mACDT(tfo_mA, true);
        Uint64 sum_m = tfo_m - tfo_mA;
        CIMDateTime dif_CDT(sum_m, true);
        CIMDateTime CDT_dif = tfo_mCDT - tfo_mACDT;
        if (String::compare(dif_CDT.toString(), CDT_dif.toString()))
        {
            PEGASUS_TEST_ASSERT(false);
        }


        Uint64 tfo_m2 = PEGASUS_UINT64_LITERAL(12357647324458);
        Uint64 tfo_mA2 = PEGASUS_UINT64_LITERAL(6789143423);
        CIMDateTime tfo_mCDT2(tfo_m2, false);
        CIMDateTime tfo_mACDT2(tfo_mA2, true);
        Uint64 dif_m2 = tfo_m2 - tfo_mA2;
        CIMDateTime dif_CDT2(dif_m2, false);
        CIMDateTime CDT_dif2 = tfo_mCDT2 - tfo_mACDT2;
        if (String::compare(dif_CDT2.toString(), CDT_dif2.toString()))
        {
            PEGASUS_TEST_ASSERT(false);
        }


        Uint64 tfo_m3 = PEGASUS_UINT64_LITERAL(12357647324458);
        Uint64 tfo_mA3 = PEGASUS_UINT64_LITERAL(6789143423);
        CIMDateTime tfo_mCDT3(tfo_m3, false);
        CIMDateTime tfo_mACDT3(tfo_mA3, false);
        Uint64 dif_m3 = tfo_m3 - tfo_mA3;
        CIMDateTime dif_CDT3(dif_m3, true);
        CIMDateTime CDT_dif3 = tfo_mCDT3 - tfo_mACDT3;
        if (String::compare(dif_CDT3.toString(), CDT_dif3.toString()))
        {
            PEGASUS_TEST_ASSERT(false);
        }



        Uint64 tfo_m4 = PEGASUS_UINT64_LITERAL(123576473244426265);
        Uint64 tfo_mA4 = PEGASUS_UINT64_LITERAL(6789143423435);
        CIMDateTime mCDT4(tfo_m4, false);
        CIMDateTime tfo_mACDT4(tfo_mA4, false);
        Uint64 dif_m4 = tfo_m4 - tfo_mA4;
        CIMDateTime CDT_dif4 = mCDT4 - tfo_mACDT4;
        Uint64 CDT_num = CDT_dif4.toMicroSeconds();
        if (CDT_num != dif_m4)
        {
            PEGASUS_TEST_ASSERT(false);
        }

/*************************************************************************/

        // testing operator-=


        CIMDateTime top_tko4("00040520041412.123435-000");
        CIMDateTime topA_tko4("00000520041452.123456:000");
        Uint64 top_yup4 = top_tko4.toMicroSeconds();
        Uint64 topA_yup4 = topA_tko4.toMicroSeconds();
        Uint64 ot_yup4 = top_yup4 - topA_yup4;
        top_tko4 -= topA_tko4;
        CIMDateTime tko4(ot_yup4, false);
        if (top_tko4 != tko4)
        {
            PEGASUS_TEST_ASSERT(false);
        }



        CIMDateTime top_tko2("00040520041412.123435:000");
        CIMDateTime topA_tko2("00000520041452.123456:000");
        Uint64 top_yup2 = top_tko2.toMicroSeconds();
        Uint64 topA_yup2 = topA_tko2.toMicroSeconds();
        Uint64 ot_yup2 = top_yup2 - topA_yup2;
        top_tko2 -= topA_tko2;
        Uint64 tko2 = top_tko2.toMicroSeconds();
        if (ot_yup2 != tko2)
        {
            PEGASUS_TEST_ASSERT(false);
        }



        CIMDateTime top_tko3("00040520041412.123435+000");
        CIMDateTime topA_tko3("00000520041452.123***:000");
        Uint64 top_yup3 = top_tko3.toMicroSeconds();
        Uint64 topA_yup3 = topA_tko3.toMicroSeconds();
        Uint64 ot_yup3 = top_yup3 - topA_yup3;
        top_tko3 -= topA_tko3;
        CIMDateTime tko3(ot_yup3, false);
        if (top_tko3 != tko3)
        {
            PEGASUS_TEST_ASSERT(false);
        }


/*************************************************************************/

//test operator*


        CIMDateTime mul1("00000020041452.123456:000");
        Uint64 m_num1 = 12;
        CIMDateTime pr_c1 = mul1 * m_num1;
        Uint64 mul_cn1 = mul1.toMicroSeconds() * m_num1;
        CIMDateTime c_pr1(mul_cn1, true);
        if (String::compare(c_pr1.toString(), pr_c1.toString()))
        {
            PEGASUS_TEST_ASSERT(false);
        }


        CIMDateTime mul2("00000020041452.12****:000");
        Uint64 m_num2 = 2;
        CIMDateTime pr_c2 = mul2 * m_num2;
        Uint64 mul_cn2 = mul2.toMicroSeconds() * m_num2;
        Uint64 pr_num2 = pr_c2.toMicroSeconds();
        if (pr_num2 != mul_cn2)
        {
            PEGASUS_TEST_ASSERT(false);
        }


        CIMDateTime mul3("000123200414**.******:000");
        Uint64 m_num3 = 2;
        CIMDateTime pr_c3 = mul3 * m_num3;
        Uint64 mul_cn3 = mul3.toMicroSeconds() * m_num2;
        Uint64 pr_num3 = pr_c3.toMicroSeconds();
        if (pr_num3 != mul_cn3)
        {
            PEGASUS_TEST_ASSERT(false);
        }



/*************************************************************************/

        // test operator*=


        CIMDateTime ul1("00000020041452.123456:000");
        m_num1 = 12;
        Uint64 ul_cn1 = ul1.toMicroSeconds() * m_num1;
        ul1 *= m_num1;
        Uint64 ul_num1 = ul1.toMicroSeconds();
        if (ul_num1 != ul_cn1)
        {
            PEGASUS_TEST_ASSERT(false);
        }


        CIMDateTime ul2("00000020041452.123***:000");
        m_num2 = 12;
        Uint64 ul_cn2 = ul2.toMicroSeconds() * m_num2;
        ul2 *= m_num2;
        Uint64 ul_num2 = ul2.toMicroSeconds();
        if (ul_num2 != ul_cn2)
        {
            PEGASUS_TEST_ASSERT(false);
        }


/*************************************************************************/

        // testing operator


        CIMDateTime dul1("00000020041452.123456:000");
        Uint64 d_num1 = 12;
        CIMDateTime r_c1 = dul1 / d_num1;
        Uint64 dul_cn1 = dul1.toMicroSeconds() / d_num1;
        Uint64 r_num1 = r_c1.toMicroSeconds();
        if (r_num1 != dul_cn1)
        {
            PEGASUS_TEST_ASSERT(false);
        }


        CIMDateTime dul2("00023020041452.12****:000");
        Uint64 d_num2 = 12;
        CIMDateTime r_c2 = dul2 / d_num2;
        Uint64 dul_cn2 = dul2.toMicroSeconds() / d_num2;
        CIMDateTime dulc(dul_cn2, true);
        Uint64 r_num2 = r_c2.toMicroSeconds();
        PEGASUS_TEST_ASSERT(0 != r_num2);
        if (dulc != r_c2)
        {
            PEGASUS_TEST_ASSERT(false);
        }


/*************************************************************************/

        // testing operator/=

        d_num1 = 50;


        CIMDateTime dull1("00000520041452.123456:000");
        Uint64 dull_cn1 = dull1.toMicroSeconds() / d_num1;
        dull1 /= d_num1;
        Uint64 dul_num1 = dull1.toMicroSeconds();
        if (dul_num1 != dull_cn1)
        {
            PEGASUS_TEST_ASSERT(false);
        }

/*************************************************************************/

        // "testing I/I operator/ "


        CIMDateTime tii1("00040520041412.123435:000");
        CIMDateTime tii_a1("00000520041452.123456:000");
        Uint64 tii_n1 = tii1.toMicroSeconds();
        Uint64 tii_an1 = tii_a1.toMicroSeconds();
        Uint64 div_n = tii_n1 / tii_an1;
        Uint64 div_c = tii1 / tii_a1;
        if (div_n != div_c)
        {
            PEGASUS_TEST_ASSERT(false);
        }


        CIMDateTime tii2("000405200414**.******:000");
        CIMDateTime tii_a2("00000520041452.1234**:000");
        Uint64 tii_n2 = tii2.toMicroSeconds();
        Uint64 tii_an2 = tii_a2.toMicroSeconds();
        Uint64 div_n2 = tii_n2 / tii_an2;
        Uint64 div_c2 = tii2 / tii_a2;
        PEGASUS_TEST_ASSERT(0 != div_n2);
        PEGASUS_TEST_ASSERT(0 != div_c2);
        if (div_n != div_c)
        {
            PEGASUS_TEST_ASSERT(false);
        }


    /********************************************************/

        // testing of comparison

        CIMDateTime las("000405200414**.******:000");
        CIMDateTime last("00040520041435.74****:000");

        if (las > last)
        {
            PEGASUS_TEST_ASSERT(false);
        }
        if (las < last)
        {
            PEGASUS_TEST_ASSERT(false);
        }
        if (!(las >= last))
        {
            PEGASUS_TEST_ASSERT(false);
        }
        if (!(las <= last))
        {
            PEGASUS_TEST_ASSERT(false);
        }
        if (las != last)
        {
            PEGASUS_TEST_ASSERT(false);
        }

        CIMDateTime las2("000405200414**.******+000");
        CIMDateTime last2("00040520041435.74****+000");

        if (las2 > last2)
        {
            PEGASUS_TEST_ASSERT(false);
        }
        if (las2 < last2)
        {
            PEGASUS_TEST_ASSERT(false);
        }
        if (!(las2 >= last2))
        {
            PEGASUS_TEST_ASSERT(false);
        }
        if (!(las2 <= last2))
        {
            PEGASUS_TEST_ASSERT(false);
        }
        if (las2 != last2)
        {
            PEGASUS_TEST_ASSERT(false);
        }

        CIMDateTime las3("0004052004****.******+050");
        CIMDateTime last3("00040520041435.74****+000");

        if (las3 > last3)
        {
            PEGASUS_TEST_ASSERT(false);
        }
        if (las3 < last3)
        {
            PEGASUS_TEST_ASSERT(false);
        }
        if (!(las3 >= last3))
        {
            PEGASUS_TEST_ASSERT(false);
        }
        if (!(las3 <= last3))
        {
            PEGASUS_TEST_ASSERT(false);
        }
        if (las3 != last3)
        {
            PEGASUS_TEST_ASSERT(false);
        }


        CIMDateTime las4("000405200444**.******+030");
        CIMDateTime last4("00040520041435.74****+000");

        if (las4 > last4)
        {
            PEGASUS_TEST_ASSERT(false);
        }
        if (las4 < last4)
        {
            PEGASUS_TEST_ASSERT(false);
        }
        if (!(las4 >= last4))
        {
            PEGASUS_TEST_ASSERT(false);
        }
        if (!(las4 <= last4))
        {
            PEGASUS_TEST_ASSERT(false);
        }
        if (las4 != last4)
        {
            PEGASUS_TEST_ASSERT(false);
        }


    }
    catch(Exception & e)
    {
        cout << "Exception: " << e.getMessage() << endl;
        cout << "Exception caught at the end of test file" << endl;
        exit(1);
    }

    {
        // 20040811165625.000000+000
        CIMDateTime x(PEGASUS_UINT64_LITERAL(63259462585000000), false);
        CIMDateTime y("20060908113026.000000-300");
        PEGASUS_TEST_ASSERT(y > x);
    }

    cout << argv[0] << " +++++ passed all tests" << endl;

    return 0;
}
