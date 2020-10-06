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
#include <iostream>
#include <Pegasus/Common/Exception.h>
#include <Pegasus/WQL/WQLParser.h>
#include <Pegasus/WQL/WQLSimplePropertySource.h>

PEGASUS_USING_PEGASUS;
PEGASUS_USING_STD;

Boolean verbose;

void test01()
{
    //
    // Create a property source (a place for the evaluate to get the
    // values of properties from):
    //

    WQLSimplePropertySource source;
    PEGASUS_TEST_ASSERT(source.addValue("x",
        WQLOperand(10, WQL_INTEGER_VALUE_TAG)));
    PEGASUS_TEST_ASSERT(source.addValue("y",
        WQLOperand(20, WQL_INTEGER_VALUE_TAG)));
    PEGASUS_TEST_ASSERT(source.addValue("z",
        WQLOperand(1.5, WQL_DOUBLE_VALUE_TAG)));

    //
    // Define query:
    //

    const char TEXT[] =
        "SELECT x,y,z\n"
        "FROM MyClass\n"
        "WHERE x > 5 AND y < 25 AND z > 1.2";

    //
    //  Will test WQLParser::parse(const Buffer&, WQLSelectStatement&)
    //  and WQLParser::parse(const char*, WQLSelectStatement&) forms
    //
    Buffer text;
    text.append(TEXT, sizeof(TEXT));
    if (verbose)
    {
        cout << text.getData() << endl;
    }

    //
    // Parse the text:
    //

    WQLSelectStatement statement;

    try
    {
        WQLParser::parse(text, statement);
        if (verbose)
        {
            statement.print();
        }

        //
        //  Test WQLSelectStatement functions
        //
        PEGASUS_TEST_ASSERT (statement.getClassName().equal ("MyClass"));
        PEGASUS_TEST_ASSERT (!statement.getAllProperties());
        PEGASUS_TEST_ASSERT (statement.getSelectPropertyNameCount() == 3);
        CIMName propName = statement.getSelectPropertyName (0);
        PEGASUS_TEST_ASSERT ((propName.equal ("x")) || (propName.equal ("y"))
                || (propName.equal ("z")));
        CIMPropertyList propList = statement.getSelectPropertyList();
        PEGASUS_TEST_ASSERT (!propList.isNull());
        PEGASUS_TEST_ASSERT (propList.size() == 3);
        PEGASUS_TEST_ASSERT ((propList[0].equal ("x"))
            || (propList[0].equal ("y")) || (propList[0].equal ("z")));
        PEGASUS_TEST_ASSERT (statement.hasWhereClause());
        PEGASUS_TEST_ASSERT (statement.getWherePropertyNameCount() == 3);
        CIMName wherePropName = statement.getWherePropertyName (0);
        PEGASUS_TEST_ASSERT ((wherePropName.equal ("x"))
            || (wherePropName.equal ("y")) || (wherePropName.equal ("z")));
        CIMPropertyList wherePropList = statement.getWherePropertyList();
        PEGASUS_TEST_ASSERT (!wherePropList.isNull());
        PEGASUS_TEST_ASSERT (wherePropList.size() == 3);
        PEGASUS_TEST_ASSERT ((wherePropList[0].equal ("x")) ||
                (wherePropList[0].equal ("y")) ||
                (wherePropList[0].equal ("z")));
        PEGASUS_TEST_ASSERT (statement.evaluateWhereClause(&source));
    }
    catch (Exception& e)
    {
        cerr << "Exception: " << e.getMessage() << endl;
        exit(1);
    }
}

void test02()
{
    //
    // Create a property source (a place for the evaluate to get the
    // values of properties from):
    //

    WQLSimplePropertySource source;
    PEGASUS_TEST_ASSERT(source.addValue("a",
        WQLOperand(5, WQL_INTEGER_VALUE_TAG)));
    PEGASUS_TEST_ASSERT(source.addValue("b", WQLOperand(25,
        WQL_INTEGER_VALUE_TAG)));
    PEGASUS_TEST_ASSERT(source.addValue("c", WQLOperand(0.9,
        WQL_DOUBLE_VALUE_TAG)));
    PEGASUS_TEST_ASSERT(source.addValue("d", WQLOperand("Test",
        WQL_STRING_VALUE_TAG)));

    //
    // Define query:
    //

    const char TEXT[] =
        "SELECT a,c,d\n"
        "FROM YourClass\n"
        "WHERE a > 5 AND b < 25 AND c > 1.2 AND d = \"Pass\"";

    //
    //  Will test WQLParser::parse(const String&, WQLSelectStatement&)
    //  and WQLParser::parse(const char*, WQLSelectStatement&) forms
    //
    String text (TEXT);
    if (verbose)
    {
        cout << text << endl;
    }

    //
    // Parse the text:
    //

    WQLSelectStatement statement;

    try
    {
        WQLParser::parse(text, statement);
        if (verbose)
        {
            statement.print();
        }

        //
        //  Test WQLSelectStatement functions
        //
        PEGASUS_TEST_ASSERT (statement.getClassName().equal ("YourClass"));
        PEGASUS_TEST_ASSERT (!statement.getAllProperties());
        PEGASUS_TEST_ASSERT (statement.getSelectPropertyNameCount() == 3);
        CIMName propName = statement.getSelectPropertyName (2);
        PEGASUS_TEST_ASSERT ((propName.equal ("a")) || (propName.equal ("c"))
            || (propName.equal ("d")));
        CIMPropertyList propList = statement.getSelectPropertyList();
        PEGASUS_TEST_ASSERT (!propList.isNull());
        PEGASUS_TEST_ASSERT (propList.size() == 3);
        PEGASUS_TEST_ASSERT ((propList[2].equal ("a"))
            || (propList[2].equal ("c")) || (propList[2].equal ("d")));
        PEGASUS_TEST_ASSERT (statement.hasWhereClause());
        PEGASUS_TEST_ASSERT (statement.getWherePropertyNameCount() == 4);
        CIMName wherePropName = statement.getWherePropertyName (3);
        PEGASUS_TEST_ASSERT ((wherePropName.equal ("a"))
            || (wherePropName.equal ("b")) ||
            (wherePropName.equal ("c")) || (wherePropName.equal ("d")));
        CIMPropertyList wherePropList = statement.getWherePropertyList();
        PEGASUS_TEST_ASSERT (!wherePropList.isNull());
        PEGASUS_TEST_ASSERT (wherePropList.size() == 4);
        PEGASUS_TEST_ASSERT ((wherePropList[3].equal ("a")) ||
                (wherePropList[3].equal ("b")) ||
                (wherePropList[3].equal ("c")) ||
                (wherePropList[3].equal ("d")));
        PEGASUS_TEST_ASSERT (!statement.evaluateWhereClause(&source));
    }
    catch (Exception& e)
    {
        cerr << "Exception: " << e.getMessage() << endl;
        exit(1);
    }
}

void test03()
{
    //
    // Define query:
    //

    const char TEXT[] =
        "SELECT *\n"
        "FROM AnotherClass\n";

    //
    //  Will test WQLParser::parse(const String&, WQLSelectStatement&)
    //  and WQLParser::parse(const char*, WQLSelectStatement&) forms
    //
    String text (TEXT);
    if (verbose)
    {
        cout << text << endl;
    }

    //
    // Parse the text:
    //

    WQLSelectStatement statement;

    try
    {
        WQLParser::parse(text, statement);
        if (verbose)
        {
            statement.print();
        }

        //
        //  Test WQLSelectStatement functions
        //
        PEGASUS_TEST_ASSERT (statement.getClassName().equal ("AnotherClass"));
        PEGASUS_TEST_ASSERT (statement.getAllProperties());
        CIMPropertyList propList = statement.getSelectPropertyList();
        PEGASUS_TEST_ASSERT (propList.isNull());
        PEGASUS_TEST_ASSERT (!statement.hasWhereClause());
        PEGASUS_TEST_ASSERT (statement.getWherePropertyNameCount() == 0);
        CIMPropertyList wherePropList = statement.getWherePropertyList();
        PEGASUS_TEST_ASSERT (!wherePropList.isNull());
        PEGASUS_TEST_ASSERT (wherePropList.size() == 0);
    }
    catch (Exception& e)
    {
        cerr << "Exception: " << e.getMessage() << endl;
        exit(1);
    }
}

int main()
{
    verbose = (getenv ("PEGASUS_TEST_VERBOSE")) ? true : false;
    test01();
    test02();
    test03();

    cout << "+++++ passed all tests" << endl;
    return 0;
}
