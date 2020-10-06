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
#include <Pegasus/CQL/CQLRegularExpression.h>
#include <Pegasus/Common/String.h>

PEGASUS_USING_PEGASUS;
PEGASUS_USING_STD;

void test01()
{
    //result should return false
    const String p = "abc";
    const String s = "";
    CQLRegularExpression re(p);

    PEGASUS_TEST_ASSERT ( (re.match(s)) == false);
    return;
}

void test02()
{
    //result should return false
    const String p = "";
    const String s = "abc";
    CQLRegularExpression re(p);

    PEGASUS_TEST_ASSERT ( (re.match(s)) == false);
    return;
}

void test03()
{
    //result should return true
    const String p = "abc";
    const String s = "abc";
    CQLRegularExpression re(p);

    PEGASUS_TEST_ASSERT ( (re.match(s)) == true);
    return;
}


void test04()
{
    //result should return false
    const String p = "abcd";
    const String s = "abc";
    CQLRegularExpression re(p);

    PEGASUS_TEST_ASSERT ( (re.match(s)) == false);
    return;
}

void test05()
{
    //result should return false
    const String p = "abc";
    const String s = "abcd";
    CQLRegularExpression re(p);

    PEGASUS_TEST_ASSERT ( (re.match(s)) == false);
    return;
}

void test06()
{
    //result should return true
    const String p = "ab.";
    const String s = "ab?";
    CQLRegularExpression re(p);

    PEGASUS_TEST_ASSERT ( (re.match(s)) == true);
    return;
}

void test07()
{
    //result should return true
    const String p = ".a.b";
    const String s = "aa!b";
    CQLRegularExpression re(p);

    PEGASUS_TEST_ASSERT ( (re.match(s)) == true);
    return;
}

void test08()
{
    //result should return true
    const String p = "\\.ab";
    const String s = ".ab";
    CQLRegularExpression re(p);

    PEGASUS_TEST_ASSERT ( (re.match(s)) == true);
    return;
}

void test09()
{
    //result should return false
    const String p = "\\.ab";
    const String s = "\\.ab";
    CQLRegularExpression re(p);

    PEGASUS_TEST_ASSERT ( (re.match(s)) == false);
    return;
}

void test10()
{
    //result should return true
    const String p = ".*";
    const String s = "abcd";
    CQLRegularExpression re(p);

    PEGASUS_TEST_ASSERT ( (re.match(s)) == true);
    return;
}

void test11()
{
    //result should return true
    const String p = "\\.*";
    const String s = "......";
    CQLRegularExpression re(p);

    PEGASUS_TEST_ASSERT ( (re.match(s)) == true);
    return;
}

void test12()
{
    //result should return true
    const String p = "abcd*";
    const String s = "abcddddd";
    CQLRegularExpression re(p);

    PEGASUS_TEST_ASSERT ( (re.match(s)) == true);
    return;
}

void test13()
{
    //result should return false
    const String p = "abcd*";
    const String s = "abcd";
    CQLRegularExpression re(p);

    PEGASUS_TEST_ASSERT ( (re.match(s)) == false);
    return;
}

void test14()
{
    //result should return true
    const String p = "ab*cd";
    const String s = "abbbbcd";
    CQLRegularExpression re(p);

    PEGASUS_TEST_ASSERT ( (re.match(s)) == true);
    return;
}

void test15()
{
    //result should return true
    const String p = "ab.*cd";
    const String s = "ab123!cd";
    CQLRegularExpression re(p);

    PEGASUS_TEST_ASSERT ( (re.match(s)) == true);
    return;
}

void test16()
{
    //result should return true
    const String p = "\\*ab";
    const String s = "*ab";
    CQLRegularExpression re(p);

    PEGASUS_TEST_ASSERT ( (re.match(s)) == true);
    return;
}

void test17()
{
    //result should return false
    const String p = ".\\*";
    const String s = "****";
    CQLRegularExpression re(p);

    PEGASUS_TEST_ASSERT ( (re.match(s)) == false);
    return;
}

void test18()
{
    //result should return true
    Char16 utf16Chars[] =
        {
        0xD800,0x78BC, 0xDC01, 0x45A3,
        0x00};

    const String utf(utf16Chars);
    CQLRegularExpression re(utf);

    PEGASUS_TEST_ASSERT ( (re.match(utf)) == true);
    return;
}

void test19()
{
    //result should return true
    Char16 utf16CharsP[] =
        {
        0xD800,0x78BC,
        0x00};

    String utfPattern(utf16CharsP);
    utfPattern.append("*");
    Char16 utf16CharsS[] =
    {
    0xD800,0x78BC, 0xD800,0x78BC, 0xD800,0x78BC,
    0x00};

    const String utfString(utf16CharsS);
    CQLRegularExpression re(utfPattern);

    PEGASUS_TEST_ASSERT ( (re.match(utfString)) == true);
    return;
}

void test20()
{
    //result should return true
    const String pattern = ".*";

    Char16 utf16CharsS[] =
    {
    0xD800,0x78BC, 0x00};

    String utfString(utf16CharsS);
    utfString.append("*");
    CQLRegularExpression re(pattern);

    PEGASUS_TEST_ASSERT ( (re.match(utfString)) == true);
    return;
}

void test21()
{
    //result should return true
    Char16 utf16CharsP[] =
        {
        0xD800,0x78BC,
        0x00};

    String utfPattern(utf16CharsP);
    utfPattern.append(".*");

    Char16 utf16CharsS[] =
    {
    0xD800,0x78BC, 0x00};

    String utfString(utf16CharsS);
    utfString.append("an3s");
    CQLRegularExpression re(utfPattern);

    PEGASUS_TEST_ASSERT ( (re.match(utfString)) == true);
    return;
}





int main()
{
    test01();
    test02();
    test03();
    test04();
    test05();
    test06();
    test07();
    test08();
    test09();
    test10();
    test11();
    test12();
    test13();
    test14();
    test15();
    test16();
    test17();
    test18();
    test19();
    test20();
    test21();
    cout << "+++++ passed all tests" << endl;


    return 0;
}

