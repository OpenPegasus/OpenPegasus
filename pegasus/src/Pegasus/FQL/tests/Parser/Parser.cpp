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
#include <iostream>
#include <Pegasus/Common/Exception.h>
#include <Pegasus/FQL/FQLParser.h>
#include <Pegasus/FQL/FQLInstancePropertySource.h>
#include <Pegasus/FQL/FQLPropertySource.h>
#include <Pegasus/Common/CIMInstance.h>
#include <Pegasus/Common/Print.h>
#include <Pegasus/General/MofWriter.h>


PEGASUS_USING_PEGASUS;
PEGASUS_USING_STD;

Boolean verbose;

#define VCOUT if (verbose) cout


/***************************************************************************
**
**  Build Source object to be used in the tests
**
****************************************************************************/

// Build the single instance to be used in tests.  Setting the argument
// display to true will cause the created instance to be displayed.

CIMInstance buildCIMInstanceSourceObject(Boolean display)
{
    // Build an instance of a class that will be embedded within
    // the instance of MyClass
    CIMInstance instembed(CIMName("EmbedClassClass"));
    instembed.addProperty(CIMProperty(CIMName("embedBool"), true));
    instembed.addProperty(CIMProperty(CIMName("embedInt"), Uint32(5)));
    instembed.addProperty(CIMProperty(CIMName("embedStr"), String("Test")));

    // Build an instance that will be the primary test instance
    CIMInstance inst(CIMName("MyClass"));
    inst.addProperty(CIMProperty(CIMName("BoolScal1"), true));
    inst.addProperty(CIMProperty(CIMName("BoolScal2"), false));

    inst.addProperty(CIMProperty(CIMName("IntScal1"), Uint32(5)));
    inst.addProperty(CIMProperty(CIMName("IntScal2"), Uint32(25)));
    inst.addProperty(CIMProperty(CIMName("IntScal3"), Sint32(-25123)));
    inst.addProperty(CIMProperty(CIMName("IntScal4"), Uint32(0)));

    inst.addProperty(CIMProperty(CIMName("Int64Scal5"), Uint64(0x6fffff)));
    inst.addProperty(CIMProperty(CIMName("Int64Scal6"), Uint64(2147483647)));
    inst.addProperty(CIMProperty(CIMName("Int64Scal7"), Uint64(4067)));
    inst.addProperty(CIMProperty(CIMName("Int64Scal8"), Uint64(0)));
    //// TODO add properties max and min integers.

    // Add real properties
    inst.addProperty(CIMProperty(CIMName("DoubleScal1"), Real64(20.9)));
    inst.addProperty(CIMProperty(CIMName("DoubleScal2"), Real64(0.9)));
    inst.addProperty(CIMProperty(CIMName("DoubleScal3"), Real32(20.9)));
    inst.addProperty(CIMProperty(CIMName("DoubleScal4"), Real32(0.9)));

    // add scalar string properties
    inst.addProperty(CIMProperty(CIMName("strScal1"), String("Test")));
    inst.addProperty(CIMProperty(CIMName("strScal2"), String("noTest")));
    inst.addProperty(CIMProperty(CIMName("strScal3"), String("")));
    // add string property with special characters.
    inst.addProperty(CIMProperty(CIMName("strScal4"),
        String("./\\\"\'!@#$%^&*()")));

    // DateTime Scalar properties - TODO create test for
    // additional formats for this field (interval, other offsets.
    CIMDateTime dt1;
    dt1.set("19991224120000.000000+360");
    inst.addProperty(CIMProperty(CIMName("dateTimeScal1"),dt1));
        CIMDateTime dt2;
    dt2.set("19991224120000.0000**+360");
    inst.addProperty(CIMProperty(CIMName("dateTimeScal2"),dt2));

    // reference literal properties
    CIMObjectPath op1 = CIMObjectPath
            ("//atp:77/root/cimv25:"
            "TennisPlayer.first=\"Chris\",last=\"Evert\"");
    inst.addProperty(CIMProperty(CIMName("referenceScal1"),op1));
    CIMObjectPath op2 = CIMObjectPath
            ("root/cimv25:"
            "TennisPlayer.first=\"Chris\",last=\"Evert\"");
    inst.addProperty(CIMProperty(CIMName("referenceScal2"),op2));
    CIMObjectPath op3 = CIMObjectPath
            ("TennisPlayer.first=\"Chris\",last=\"Evert\"");
    inst.addProperty(CIMProperty(CIMName("referenceScal3"),op3));
    CIMObjectPath op4 = CIMObjectPath
            ("myclassname.p1=1");
    inst.addProperty(CIMProperty(CIMName("referenceScal4"),op4));

    // KS_TODO create a Reference array property

    // Embedded instance property
    inst.addProperty(CIMProperty(CIMName("embeddedInstance1"),instembed));

    // Array properties
    // Boolean Array
    Array<Boolean> b1; b1.append(true); b1.append(false); b1.append(true);
    inst.addProperty(CIMProperty(CIMName("BoolArrayProp1"), b1));
    Array<Boolean> b2; b2.append(true); b2.append(false); b2.append(true);
    inst.addProperty(CIMProperty(CIMName("BoolArrayProp2"), b2));
    Array<Boolean> b3; b3.append(false); b3.append(false); b3.append(false);
    inst.addProperty(CIMProperty(CIMName("BoolArrayProp3"), b1));

    // Integer Array
    Array<Sint64> a1; a1.append(4); a1.append(5); a1.append (7);
    inst.addProperty(CIMProperty(CIMName("IntArrayProp1"), a1));
    Array<Sint64> a2; a2.append(7); a2.append(8); a2.append (9);
    inst.addProperty(CIMProperty(CIMName("IntArrayProp2"), a2));
    Array<Sint64> a3; a3.append(7); a3.append(-888); a3.append (-999);
    inst.addProperty(CIMProperty(CIMName("IntArrayProp3"), a3));
    Array<Sint64> a4; a4.append(7);
    inst.addProperty(CIMProperty(CIMName("IntArrayProp4"), a4));
    Array<Sint64> a5;;
    inst.addProperty(CIMProperty(CIMName("IntArrayProp5"), a5));

    // Double Array
    Array<Real64> r1; r1.append(1011.04); r1.append(123456.8); r1.append(0.1);
    inst.addProperty(CIMProperty(CIMName("DoubleArrayProp1"), r1));
    Array<Real64> r2; r2.append(-1.04); r2.append(-123456.8); r2.append(0.1);
    inst.addProperty(CIMProperty(CIMName("DoubleArrayProp2"), r2));

    // String Array
    Array<String> s1; s1.append("zero"); s1.append("one"); s1.append("two");
        s1.append("three");
    inst.addProperty(CIMProperty(CIMName("StrArrayProp1"), s1));
    Array<String> s2; s2.append("zero"); s2.append("one"); s2.append("two");
        s2.append("three");
    inst.addProperty(CIMProperty(CIMName("StrArrayProp2"), s2));

    // DateTime Array
    Array<CIMDateTime> dta1;
    dta1.append(dt1);
    dt1.clear();dt1.set("20131224120000.000000+360"); dta1.append(dt1);
    dt1.clear();dt1.set("20011224120000.000000+360"); dta1.append(dt1);
    inst.addProperty(CIMProperty(CIMName("dateTimeArray1"),dta1));
    if (display)
    {
        VCOUT << "Instance to be used in Test" << endl;
        PrintInstance(cout, inst);
    }
    return inst;
}

// Test a single query filter defined by input against a predefined query
// based on the FQLSimplePropertySource defined in this function.
// Generates error message if result does not match the query and
// the result parameters of:
//    expectRtn Boolean - Defines if the query should match
//    expectException Boolean - True if the query should generate an exception
//
void testQuery(const FQLPropertySource& source,const String& query,
    Boolean expectedRtn,
    Boolean expectException = false)
{
    VCOUT << "testQuery. query: \"" << query << "\". Expected result = "
        << (expectedRtn? "true" : "false") << " expectException "
        << (expectException? "true" : "false")
        << endl;

    //
    // Parse and evaluate the query
    //

    FQLQueryStatement statement;

    try
    {
        FQLParser::parse(query, statement);
        if (verbose)
        {
            VCOUT << "Query Parse Successful - Query: \"" << query << "\" is "
                 << (expectedRtn? "true": "false")
                 << endl
                 << "Source values:" << endl;
            statement.print();
        }
    }
    catch (Exception& e)
    {
        if (expectException)
        {
            VCOUT << "Parse Exception: Expected Exception: "
                 << e.getMessage() << ". Query "
                 << query << endl;
            return;
        }
        cerr << "Parse Exception: Unexpected Exception: "
             << e.getMessage() << ". Query "
             << query << ". TestError. Terminating" << endl;
        exit(1);
    }

    // Parse successful, test evaluation
    try
    {
        Boolean queryRtn = statement.evaluateQuery(&source);

        if (expectedRtn)
        {
            if (!queryRtn)
            {
                cerr << "Query "
                    << query << " Failed evaluation but should have passed."
                    << endl;
            }
            PEGASUS_TEST_ASSERT(queryRtn);
        }
        else
        {
            if (queryRtn)
            {
                cerr << "Query "
                    << query << " Passed evaluation but should have failed"
                     << endl;
            }
            PEGASUS_TEST_ASSERT(!queryRtn);
        }
    }
    catch (Exception& e)
    {
        if (expectException)
        {
            VCOUT << "Expected Evaluation Exception: " << e.getMessage()
                 << ". Query " << query << endl;
            return;
        }
        cerr << "Unexpected Evaluation Exception: " << e.getMessage()
             << ". Query " << query << ". TestError. Terminating" << endl;
        exit(1);
    }
}

/*
    Executes all of the individual tests against the provided FQL
    source instance.
*/
void executeTests(FQLInstancePropertySource src)
{

    ///////////////////////////////////////////////////////////////////////
    /// Some general tests.  These should be moved to proper section
    //////////////////////////////////////////////////////////////////////


    testQuery(src, "IntArrayProp1 = {4, 5, 7}", true);
    testQuery(src, "IntArrayProp1[2] = 7", true);
    testQuery(src, "ANY IntArrayProp1 = 7",true);
    testQuery(src, "ANY IntArrayProp1 = IntScal1",true);
    testQuery(src, "EVERY IntArrayProp1 < 90",true);
    testQuery(src, "NOT ANY IntArrayProp1 = 7",false);
    testQuery(src, "NOT EVERY IntArrayProp1 = 7",true);

    /////////////////////////////////////////////////////////////////
    // Boolean property tests
    /////////////////////////////////////////////////////////////////

    // boolean-scalar-property op literal
    testQuery(src, "BoolScal1 = true", true);
    testQuery(src, "BoolScal1 <> true", false);
    testQuery(src, "BoolScal1 = true", true);
    testQuery(src, "BoolScal2 = true", false);
    testQuery(src, "BoolScal2 <> true", true);

    // boolean-scalar-property op boolean-scalar-property
    testQuery(src, "BoolScal1 = BoolScal1", true);
    testQuery(src, "BoolScal1 <> BoolScal2", true);
    testQuery(src, "BoolScal1 > BoolScal2", true, true);
    testQuery(src, "BoolScal1 < BoolScal2", true, true);
    testQuery(src, "BoolScal1 >= BoolScal2", true, true);
    testQuery(src, "BoolScal1 <= BoolScal2", true, true);

    testQuery(src, "BoolScal1 > BoolScal2", true, true);

    // Bool array to literal
    testQuery(src, "BoolArrayProp1 = {true, false, true }", true);
    testQuery(src, "BoolArrayProp2 = {true, false, true }", true);

        // test Bool array to array comparisons
    testQuery(src, "BoolArrayProp2 = BoolArrayProp2", true);
    testQuery(src, "BoolArrayProp2 <> BoolArrayProp2", false);

    testQuery(src, "BoolArrayProp1 = BoolArrayProp2", true);
    testQuery(src, "BoolArrayProp2 <>  BoolArrayProp2", false);

    testQuery(src, "BoolArrayProp1 = BoolArrayProp3", true);
    testQuery(src, "BoolArrayProp1 <>  BoolArrayProp3", false);

    // test invalid property name
    testQuery(src, "BoolArrayProp2 =  BoolArrayProp9", true, true);

    // test invalid operators
    testQuery(src, "BoolArrayProp2 < BoolArrayProp2", true, true);

    // Invalid bool array compares

    testQuery(src, "BoolArrayProp2 > {true, false, true }", true, true);
    testQuery(src, "BoolArrayProp2 < {true, false, true }", true, true);
    testQuery(src, "BoolArrayProp2 >= {true, false, true }", true, true);
    testQuery(src, "BoolArrayProp2 <= {true, false, true }", true, true);

    // bool aray[x] to literal TODO
    // bool array[x] to boolscalar TODO

    //////////////////////////////////////////////////////////
    // String property tests
    /////////////////////////////////////////////////////////

    // test String-scalar-property op  literal
    testQuery(src,"strScal1 = \'Test\'", true);
    testQuery(src,"strScal1 <> \'Test\'", false);
    testQuery(src,"strScal1 <> \'Testx\'", true);
    testQuery(src,"strScal1 <> \'test\'", true);
    testQuery(src,"strScal1 <> \'Tes\'", true);

    // test String-array-property op  array literal
    testQuery(src, "StrArrayProp1 ="
              "{\'zero\', \'one\', \'two\', \'three\'}",true);
    testQuery(src, "StrArrayProp1 <>"
                   "{\'zero\', \'one\', \'two\', \'three\'}",false);
    testQuery(src, "StrArrayProp1 =  {\'zero\', \'one\', \'two\'}",false);
    testQuery(src, "StrArrayProp1 <> {\'zero\', \'one\', \'two\'}",true);

    // String-array-Property[index] op scalar literal
    testQuery(src, "StrArrayProp1[0] = \'zero\'",true);
    testQuery(src, "StrArrayProp1[1] = \'one\'",true);
    testQuery(src, "StrArrayProp1[2] = \'two\'",true);
    testQuery(src, "StrArrayProp1[3] = \'three\'",true);
    testQuery(src, "StrArrayProp1[1] = \'One\'",false);
    testQuery(src, "StrArrayProp1[1] <> \'One\'",true);
    testQuery(src, "StrArrayProp1[1] <> \'\'", true);
    testQuery(src, "StrArrayProp1[1] = \'\'", false);

    // String-array-Property[index] op scalar property
    // TODO
    // String ANY and EVERY

    // TODO add < and > ops
    testQuery(src," ANY StrArrayProp1 = \'zero\'",true);
    testQuery(src," ANY StrArrayProp1 = \'three\'",true);
    testQuery(src," ANY StrArrayProp1 = \'notAProperty\'",false);
    testQuery(src," EVERY StrArrayProp1 = \'three\'",false);

    // Test LIKE operation
    testQuery(src,"strScal1 LIKE \'Test\'", true);
    testQuery(src,"strScal1 LIKE \'T...\'", true);
    testQuery(src,"strScal1 LIKE \'Tes.\'", true);

    testQuery(src,"strScal1 LIKE \'.est\'", true);
    testQuery(src,"strScal1 LIKE \'.*\'", true);

    testQuery(src,"strScal1 NOT LIKE \'.est\'", false);
    testQuery(src,"strScal1 NOT LIKE \'.*\'", false);

    testQuery(src, "StrArrayProp1[1] LIKE \'one\'",true);
    testQuery(src, "StrArrayProp1[1] LIKE \'.ne\'",true);

    ////////////////////////////////////////////////////////
    // Integer properties tests
    ////////////////////////////////////////////////////////
    // integer-scalar-property op literal
    testQuery(src, "IntScal1 = 5", true);
    testQuery(src, "IntScal2 = 25", true);
    testQuery(src, "IntScal1 <> 5", false);
    testQuery(src, "IntScal2 <> 25", false);
    testQuery(src, "IntScal1 <> 9", true);
    testQuery(src, "IntScal1 > 4", true);
    testQuery(src, "IntScal1 < 6", true);
    testQuery(src, "IntScal2 <> 0", true);
    testQuery(src, "IntScal3 = -25123", true);
    testQuery(src, "IntScal4 = 0", true);
    testQuery(src, "Int64Scal5 = 7340031", true);
    testQuery(src, "Int64Scal6 = 2147483647", true);
    testQuery(src, "Int64Scal7 = 4067", true);

    // tests against hex and binary scalar integer defintions
    testQuery(src, "IntScal1 = 0X5", true);
    testQuery(src, "IntScal1 = 101B", true);
    testQuery(src, "IntScal2 = 0X19", true);
    testQuery(src, "IntScal2 < 0X1A", true);
    testQuery(src, "IntScal2 = 0X1A", false);
    testQuery(src, "IntScal2 = 11001B", true);
    testQuery(src, "IntScal3 = -0x6223", true);
    testQuery(src, "IntScal3 = -110001000100011B", true);
    testQuery(src, "IntScal4 = 0X0", true);
    testQuery(src, "IntScal4 = 0B", true);
    testQuery(src, "Int64Scal5 = 0X6fffff", true);
    testQuery(src, "Int64Scal6 = 2147483647", true);

    // TODO create an array property with single entry.
    // TODO create array property that is empty

    // test integerArray op array literal
    testQuery(src, "IntArrayProp1 = {3}", false);
    testQuery(src, "IntArrayProp1 = {4, 5, 7}", true);
    testQuery(src, "IntArrayProp1 = {4, 5, 8}", false);
    testQuery(src, "IntArrayProp1 = {4, 5, 8, 9}", false);
    testQuery(src, "IntArrayProp1 = {4, 5}", false);

    // test integer Array[x] op scalar
    testQuery(src,"IntArrayProp1[2] = 7", true);
    testQuery(src,"IntArrayProp1[1] = 5", true);
    testQuery(src,"IntArrayProp1[0] = 4", true);
    testQuery(src,"IntArrayProp1[2] = 8", false);
    testQuery(src,"IntArrayProp1[2] > 7", false);
    testQuery(src,"IntArrayProp1[2] < 3", false);

    // scalar one property against another
    testQuery(src, "IntScal1 = IntScal2", false);
    testQuery(src, "IntScal1 <> IntScal2", true);
    testQuery(src, "IntScal1 < IntScal2", true);
    testQuery(src, "IntScal1 <= IntScal2", true);
    testQuery(src, "IntScal1 > IntScal2", false);
    testQuery(src, "IntScal1 >= IntScal2", false);

    testQuery(src, "IntScal1 < IntScal1", false);
    testQuery(src, "IntScal1 > IntScal1", false);

    testQuery(src, "IntScal1 < Int64Scal7", true);
    testQuery(src, "IntScal4 = Int64Scal8", true);

    testQuery(src,"ANY IntArrayProp1 < 90",true);
    testQuery(src,"NOT EVERY IntArrayProp1 < 90",false);
    testQuery(src,"EVERY IntArrayProp1 < 90",true);
    testQuery(src,"EVERY IntArrayProp1 > 90",false);
    testQuery(src,"ANY IntArrayProp1 = 7",true);
    testQuery(src,"ANY IntArrayProp1 = 5",true);
    testQuery(src,"ANY IntArrayProp1 = 4",true);
    testQuery(src,"ANY IntArrayProp1 = 3",false);
    testQuery(src,"NOT ANY IntArrayProp1 = 99999",true);

    testQuery(src," ANY IntArrayProp1 = 99",false);
    testQuery(src," EVERY IntArrayProp1 = 7",false);

    testQuery(src,"ANY IntArrayProp1 = IntArrayProp1[0]",true);
    testQuery(src,"ANY IntArrayProp1 = IntArrayProp1[1]",true);
    testQuery(src,"ANY IntArrayProp1 = IntArrayProp1[2]",true);

    testQuery(src,"ANY IntArrayProp1 = IntArrayProp2[0]",true);
    testQuery(src,"ANY IntArrayProp1 <> IntArrayProp2[2]",true);

    testQuery(src,"EVERY IntArrayProp1 >= IntArrayProp1[0]",true);
    testQuery(src,"EVERY IntArrayProp1 <= IntArrayProp1[2]",true);

    testQuery(src, "IntArrayProp1[1] = IntArrayProp1[1]", true);

    // Generate exception error because index out of bounds
    testQuery(src,"ANY IntArrayProp1 = IntArrayProp1[3]",true, true);

    // Property Index out of bounds. Fails test since that value
    // does not exist
    testQuery(src, "IntArrayProp1[20] = 7", false);

    /////////////////////////////////////////////////////////
    // Double type
    /////////////////////////////////////////////////////////
    // real-scalar-property op real-literal
    testQuery(src,"DoubleScal1 = 20.9", true);
    testQuery(src,"DoubleScal1 > 0.1", true);
    testQuery(src,"DoubleScal1 <> 0.1", true);
    testQuery(src,"DoubleScal1 < 0.1", false);
    testQuery(src,"DoubleScal1 < 1.2", false);
    testQuery(src,"DoubleScal1 > 1.2", true);
    testQuery(src,"DoubleScal1 = 0.9", false);
    testQuery(src,"DoubleScal1 = 1.2", false);
    testQuery(src,"DoubleScal1 = 20.9999", false);
    // real array
    testQuery(src,"DoubleArrayProp1 = {1011.04, 123456.8, 0.1 }", true);
    testQuery(src,"DoubleArrayProp1 <> {1011.04, 123456.8, 0.1 }", false);
    testQuery(src,"DoubleArrayProp1[0] <>1011.04", true);

    testQuery(src,"ANY DoubleArrayProp1 <> 20.9", true);
    testQuery(src,"ANY DoubleArrayProp1 > 1011.04", true);
    testQuery(src,"EVERY DoubleArrayProp1 <> 99999.0", true);
    testQuery(src,"EVERY DoubleArrayProp1 < 999999.1", true);
    testQuery(src,"ANY DoubleArrayProp1 > 0.0 AND"
                  " ANY DoubleArrayProp1 < 9.1", true );
    /////////////////////////////////////////////////////////
    //  DateTime property type tests
    ////////////////////////////////////////////////////////
    // Datetime scalar tests
    testQuery(src, "dateTimeScal1 = \'19991224120000.000000+360\'", true);
    testQuery(src, "dateTimeScal1 <> \'19991224120000.000000+360\'", false);
    testQuery(src, "dateTimeScal1 <> \'20131224120000.000000+360\'", true);
    testQuery(src, "dateTimeScal1 < \'20131224120000.000000+360\'", true);
    testQuery(src, "dateTimeScal1 > \'19981224120000.000000+360\'", true);

    // TODO add tests for various timezones, etc.
    // dateTime Indexed array tests
    testQuery(src, "dateTimeArray1[0] = \'19991224120000.000000+360\'",true);
    testQuery(src, "dateTimeArray1[1] = \'20131224120000.000000+360\'",true);
    testQuery(src, "dateTimeArray1[0] < \'20131224120000.000000+360\'",true);

    // DateTime array tests
    testQuery(src, "dateTimeArray1 = {\'19991224120000.000000+360\'}", false);
    testQuery(src, "dateTimeArray1 = {\'19991224120000.000000+360\',"
                   "\'20131224120000.000000+360\',"
                    "\'20021224120000.000000+360\'}", false);

    testQuery(src, "dateTimeArray1 = {\'19991224120000.000000+360\',"
                   "\'20131224120000.000000+360\',"
                    "\'20011224120000.000000+360\'}", true);
    testQuery(src, "dateTimeArray1 <> {\'19991224120000.000000+360\',"
                   "\'20131224120000.000000+360\',"
                    "\'20011224120000.000000+360\'}", false);
    // Repeat the above for single quote literal
    testQuery(src, "dateTimeArray1 = {\'19991224120000.000000+360\'}", false);
    testQuery(src, "dateTimeArray1 = {\'19991224120000.000000+360\',"
                   "\'20131224120000.000000+360\',"
                    "\'20021224120000.000000+360\'}", false);

    testQuery(src, "dateTimeArray1 = {\'19991224120000.000000+360\',"
                   "\'20131224120000.000000+360\',"
                    "\'20011224120000.000000+360\'}", true);
    testQuery(src, "dateTimeArray1 <> {\'19991224120000.000000+360\',"
                   "\'20131224120000.000000+360\',"
                    "\'20011224120000.000000+360\'}", false);

    testQuery(src, "ANY dateTimeArray1 = '20011224120000.000000+360\'", true);
    testQuery(src,"EVERY dateTimeArray1 = '20011224120000.000000+360\'", false);

    ///////////////////////////////////////////////////////////////////
    //  Tests where property named does not exist.  The spec defines the
    //  results of these tests to be result of treating the non-existent
    //  property as NULL and comparing it with the right side property.
    //  NOTE: There is ambiguity as to what <= >=, like would mean and
    //  we treat these so they fail the tests.
    ///////////////////////////////////////////////////////////////////
    testQuery(src, "nosuchproperty1 = 3",false);
    testQuery(src, "nosuchproperty2 <> 3", true);
    testQuery(src, "nosuchproperty3 = NULL",true);
    testQuery(src, "nosuchProperty4 <> NULL", false);
    testQuery(src, "nosuchProperty4 = \'blah\'", false);
    testQuery(src, "nosuchProperty4 <= \'blah\'", false);

    ///////////////////////////////////////////////////////////////////
    //  Embedded Instance tests
    ///////////////////////////////////////////////////////////////////

    testQuery(src, "embeddedInstance1.noSuchName = true",false);
    testQuery(src, "embeddedInstance1.embedBool.noSuchName = true",false);
    testQuery(src, "embeddedInstance1.embedBool = true",true);
    testQuery(src, "embeddedInstance1.embedInt = 5",true);

    /////////////////////////////////////////////////////////
    //          Reference property type tests
    ////////////////////////////////////////////////////////
    // KS_TODO TODO Add more tests.
    testQuery(src,"referenceScal4 = \'myclassname.p1=1\'", true);
    testQuery(src, "referenceScal4 <> referenceScal3", true);
    testQuery(src, "referenceScal4 = referenceScal3", false);

    // KS_TODO add test to be sure that < > etc. do not work

    ///////////////////////////////////////////////////////////
    // AND and OR Tests
    //////////////////////////////////////////////////////////
    testQuery(src,"IntArrayProp1[1] = 5 AND IntArrayProp1[2] = 7", true);
    testQuery(src, "IntScal1 = 5 OR IntScal1 = 26", true);

    //////////////////////////////////////////////////////////
    // Error tests
    // //////////////////////////////////////////////////////

    // Expect exception from the query, property.
    // IntArrayNOExist does not exist.
    testQuery(src, "IntArrayNoExist[1] = IntArrayProp1[1]", false);
    // test invalid characters on integer literals
    testQuery(src, "IntScal1 = 0xabcdefg", true, true);
    testQuery(src, "IntScal1 = 0xabcdefg", true, true);
    testQuery(src, "IntScal1 = 123x", true, true);

    //The following is illegal in the FQL spec
    // array-literal = "{" [scalar-literal *( "." scalar-literal ) ] "}"
    testQuery(src, "IntArrayProp1 = {}", false, true);

    // Test invalid datetime literal
    testQuery(src, "dateTimeScal1 = \'19991224120000.000000+abc\'", true, true);

    // The following generates an error it mixes types.  Note that the
    // spec does not allow an integer as real.
    // The throw statement from this causes a secondary exception which
    // kills off the test (Unexpected Exception: parse error: syntax error
    // unexpected $end)
    //testQuery(src,"DoubleArrayProp1 <> {1.00, 123457, 0.1 }", false, true);

    //////////////////////////////////////////////////////////
    // Test of complex queries
    // //////////////////////////////////////////////////////
    // Tests for OR
    testQuery(src,"IntScal1 = 5 OR intScal2= 26", true);
    testQuery(src,"IntScal1 = 6 OR intScal2= 25", true);
    testQuery(src,"IntScal1 = 6 OR intScal2= 26", false);

    // Tests for AND
    testQuery(src,"IntScal1 = 5 AND intScal2= 25", true);
    testQuery(src,"IntScal1 = 6 AND intScal2= 25", false);
    testQuery(src,"IntScal1 = 5 AND intScal2= 26", false);
    testQuery(src,"( IntScal1 = 5 AND intScal2 = 25 )", true);

    // Tests of complex definitions involving parenthesis
    testQuery(src,"(IntScal1 = 5) AND (intScal2 = 25)", true);
    testQuery(src,"(( IntScal1 = 5) AND (intScal2 = 25 ))", true);
    testQuery(src,"IntScal1 = 5 AND intScal2 = 25 AND DoubleScal1 > 1.2 "
                  "AND strScal1 = \'Test\'", true);
    testQuery(src,"(IntScal1 = 5 AND intScal2= 25) OR (DoubleScal1 < 1.2 "
                  "AND strScal1= \'Test\')", true);
}
/*
  TODOS. error test scalarProperty = arrayProperty
         error generated for != is really wierd.

*/
/*
    NOTES: the op token != returns parseException :Unexpected Exception:
    parse error: syntax error, unexpected TOK_UNEXPECTED_CHAR. This
    really needs a better exception definition.

    A double quoted string generates the same error.

    Need error tests for hex, etc.  x19, 102B.

    Exceptions not clean

    Add empty array property so we can test.
    Bad operators (== !=) generate wierd instead of clear errors.
*/

int main()
{
    verbose = (getenv ("PEGASUS_TEST_VERBOSE")) ? true : false;

    // Generate the source instance to test
    CIMInstance inst = buildCIMInstanceSourceObject(verbose);
    // generate the corresponding FQLInstancePropertySource
    FQLInstancePropertySource src(inst);

    // The following is a compilable bypass to allow testing of a single
    // testQuery.  To run it, set the following line to true and
    // put the query before the cout statement
    Boolean executeSingleTest = false;
    if (executeSingleTest)
    {

//      testQuery(src,"ANY DoubleArrayProp1 = 1011.04", true);
        // Generates an evaluation error, not a parse error
//  testQuery(src,"strScal1 NOT LIKE \'.*\'", false);
//      testQuery(src, "IntArrayProp1[20] = 7", false);
//      testQuery(src,"IntScal1 = reallybad", false, true);
        cout << "+++++ passed short tests" << endl;
        return 0;
    }
    else
    {
        executeTests(src);
    }

    cout << "+++++ passed all tests" << endl;
    return 0;
}
