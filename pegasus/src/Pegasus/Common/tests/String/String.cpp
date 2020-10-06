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
#include <cstring>
#include <Pegasus/Common/String.h>
#include <Pegasus/Common/Exception.h>
#include <Pegasus/Common/CommonUTF.h>
#include <stdcxx/stream/strstream>

PEGASUS_USING_PEGASUS;
PEGASUS_USING_STD;

#define VCOUT if (verbose) cout

static Boolean verbose;

void test1()
{
    String s1 = "Hello World";
    String s2 = s1;
    String s3(s2);

    PEGASUS_TEST_ASSERT(String::equal(s1, s3));

    // Test append characters to String
    String s4 = "Hello";
    s4.append(Char16(0x0000));
    s4.append(Char16(0x1234));
    s4.append(Char16(0x5678));
    s4.append(Char16(0x9cde));
    s4.append(Char16(0xffff));

    {
#ifdef HAVE_SSTREAM
        stringstream os;
#endif
#ifdef HAVE_STRSTREAM
        ostrstream os;
#endif
        os << s4;
#ifdef HAVE_STRSTREAM
        os.put('\0');
#endif
#ifndef PEGASUS_HAS_ICU
        const char EXPECTED[] = "Hello\\x0000\\x1234\\x5678\\x9CDE\\xFFFF";
#else
        CString cstr = s4.getCString();
        const char * EXPECTED = (const char *)cstr;
#endif

#ifdef HAVE_SSTREAM
    string os_str = os.str();
    const char* tmp = os_str.c_str();
#endif
#ifdef HAVE_STRSTREAM
        char *tmp = os.str();
#endif

        PEGASUS_TEST_ASSERT(strcmp(EXPECTED, tmp) == 0);
#ifdef PEGASUS_PLATFORM_AIX_RS_IBMCXX
        os.freeze(false);
#else

#ifdef HAVE_STRSTREAM
        delete tmp;
#endif

#endif
    }

    {
        // Test getCString
        const char STR0[] = "one two three four";
        String s = STR0;
        PEGASUS_TEST_ASSERT(strcmp(s.getCString(), STR0) == 0);
    }

    {
        // Test remove
        String s = "abcdefg";
        s.remove(3, 3);
        PEGASUS_TEST_ASSERT(String::equal(s, "abcg"));
        PEGASUS_TEST_ASSERT(s.size() == 4);

        s = "abcdefg";
        s.remove(3, 4);
        PEGASUS_TEST_ASSERT(String::equal(s, "abc"));
        PEGASUS_TEST_ASSERT(s.size() == 3);

        s = "abcdefg";
        s.remove(3);
        PEGASUS_TEST_ASSERT(String::equal(s, "abc"));
        PEGASUS_TEST_ASSERT(s.size() == 3);

        s = "abc";
        s.remove(3);
        PEGASUS_TEST_ASSERT(String::equal(s, "abc"));
        PEGASUS_TEST_ASSERT(s.size() == 3);

        s = "abc";
        s.remove(0);
        PEGASUS_TEST_ASSERT(String::equal(s, ""));
        PEGASUS_TEST_ASSERT(s.size() == 0);

        s = "abc";
        s.remove(0, 1);
        PEGASUS_TEST_ASSERT(String::equal(s, "bc"));
        PEGASUS_TEST_ASSERT(s.size() == 2);

        String t1 = "HELLO";
        String t2 = t1;
        t2.toLower();
        PEGASUS_TEST_ASSERT(String::equal(t1, "HELLO"));
        PEGASUS_TEST_ASSERT(String::equal(t2, "hello"));
    }

    {
        // another test of the append method
        String t1 = "one";
        t1.append(" two");
        PEGASUS_TEST_ASSERT(String::equal(t1, "one two"));
        t1.append(' ');
        t1.append('t');
        t1.append('h');
        t1.append('r');
        t1.append("ee");
        PEGASUS_TEST_ASSERT(String::equal(t1,"one two three"));

        // used as example in Doc.
        String test = "abc";
        test.append("def");
        PEGASUS_TEST_ASSERT(test == "abcdef");
    }

    // Test of the different overload operators
    {
        // Test the == overload operator
        String t1 = "one";
        String t2 = "one";
        PEGASUS_TEST_ASSERT(t1 == "one");
        PEGASUS_TEST_ASSERT("one" == t1);
        PEGASUS_TEST_ASSERT(t1 == t2);
        PEGASUS_TEST_ASSERT(t2 == t1);
        PEGASUS_TEST_ASSERT(String("one") == "one");

        const char STR0[] = "one two three four";
        String s = STR0;
        CString tmp = s.getCString();
        PEGASUS_TEST_ASSERT(tmp == s);
        PEGASUS_TEST_ASSERT(s == tmp);
    }

    {
        // Tests of the + Overload operator
        String t1 = "abc";
        String t2 = t1 + t1;
        PEGASUS_TEST_ASSERT(t2 == "abcabc");
        t1 = "abc";
        t2 = t1 + "def";
        PEGASUS_TEST_ASSERT(t2 == "abcdef");

        t1 = "ghi";
        PEGASUS_TEST_ASSERT(t1 == "ghi");

        // ATTN: the following fails because there
        // is no single character overload operator
        // KS: Apr 2001
        // t2 = t1 + 'k' + 'l' + 'm' + "nop";
        t2 = t1 + "k" + "l" + "m" + "nop";
        PEGASUS_TEST_ASSERT(t2 == "ghiklmnop");
        PEGASUS_TEST_ASSERT(String::equal(t2,"ghiklmnop"));

        // add tests for != operator.

        t1 = "abc";
        PEGASUS_TEST_ASSERT(t1 != "ghi");
        PEGASUS_TEST_ASSERT(t1 != t2);

        // add tests for other compare operators

        // Operater <
        t1 = "ab";
        t2 = "cde";
        PEGASUS_TEST_ASSERT(t1 < t2);
        PEGASUS_TEST_ASSERT(t1 <= t2);
        PEGASUS_TEST_ASSERT(t2 > t1);
        PEGASUS_TEST_ASSERT(t2 >=t1);
        PEGASUS_TEST_ASSERT(String::compare(t1,t2) < 0);
        PEGASUS_TEST_ASSERT(String::compare(t2,t1) > 0);
        PEGASUS_TEST_ASSERT(String::compare(t1, t2, 1) < 0);
        PEGASUS_TEST_ASSERT(String::compare(t2, t1, 1) > 0);
        PEGASUS_TEST_ASSERT(String::compare(t1, t2, 10) < 0);
        PEGASUS_TEST_ASSERT(String::compare(t2, t1, 10) > 0);
        PEGASUS_TEST_ASSERT(String::compare(t1, t2, 0) == 0);
        t2 = t1;
        PEGASUS_TEST_ASSERT(t1 <= t2);
        PEGASUS_TEST_ASSERT(t1 >= t2);
        PEGASUS_TEST_ASSERT(String::compare(t1, t2) == 0);
        PEGASUS_TEST_ASSERT(String::compare(t1, t2, 0) == 0);
        PEGASUS_TEST_ASSERT(String::compare(t1, t2, 1) == 0);
        PEGASUS_TEST_ASSERT(String::compare(t1, t2, 10) == 0);


        // Tests for compare with same length
        t1 = "abc";
        t2 = "def";
        PEGASUS_TEST_ASSERT(t1 < t2);
        PEGASUS_TEST_ASSERT(String::compare(t1, t2) < 0);
        PEGASUS_TEST_ASSERT(String::compare(t2, t1) > 0);
        PEGASUS_TEST_ASSERT(String::compare(t1, t2, 10) < 0);
        PEGASUS_TEST_ASSERT(String::compare(t1, t2, 0) == 0);

        t1 = "abc";
        t2 = "ABC";
        PEGASUS_TEST_ASSERT(String::equalNoCase(t1,t2));
        PEGASUS_TEST_ASSERT(!String::equal(t1,t2));
        PEGASUS_TEST_ASSERT(String::compareNoCase(t1,t2) == 0);
        t1.toUpper();
        t2.toLower();
        PEGASUS_TEST_ASSERT(String::equal(t1, "ABC"));
        PEGASUS_TEST_ASSERT(String::equal(t2, "abc"));

        t1 = "1000";
        t2 = "1001";
        PEGASUS_TEST_ASSERT(String::compareNoCase(t1,t2) < 0);
        PEGASUS_TEST_ASSERT(String::compare(t1, t2) < 0);
        PEGASUS_TEST_ASSERT(String::compare(t1, t2, 3) == 0);
        PEGASUS_TEST_ASSERT(String::compare(t1, t2, 4) < 0);

#ifdef PEGASUS_HAS_ICU
        //
        // Strings used to test non-ascii case mappings
        // Tests context sensitve mappings (eg. greek)
        // Tests expansion after mapping (eg german)
        //

        // Lower case german and greek
        // --latin small letter sharp s (german) (2 of these to cause
        //  ICU overflow error inside of String)
        // --greek small letter sigma (followed by another letter)
        // --latin small a
        // --greek small letter sigma (NOT followed by another letter)
        const Char16 lowermap[] = {
                                   0xdf,
                                   0xdf,
                                   0x3c3,
                                   'a',
                                   0x3c2,
                                   0x00};
        String degkLow(lowermap);

        // Needed because the german char does not round trip
        // after an uppercase followed by lower case.
        // --latin small letters 's' 's' (4 of these due to expansion)
        // --greek small letter sigma (followed by another letter)
        // --latin small a
        // --greek small letter sigma (NOT followed by another letter)
        const Char16 lowermap2[] = {
                                   's', 's', 's', 's',
                                   0x3c3,
                                   'a',
                                   0x3c2,
                                   0x00};
        String degkLow2(lowermap2);

        // Upper case greek and german
        // latin cap letter sharp s (german) (4 of these due to expansion)
        // greek cap letter sigma (followed by another letter)
        // latin cap A
        // greek cap letter sigma (NOT followed by another letter)
        const Char16 uppermap[] = {
                                  'S', 'S', 'S', 'S',
                                   0x3a3,
                                   'A',
                                   0x3a3,
                                   0x00};
        String degkUp(uppermap);


        PEGASUS_TEST_ASSERT(String::compareNoCase(degkLow,degkUp) == 0);
        // does a binary compare, so lower > upper
        PEGASUS_TEST_ASSERT(String::compare(degkLow,degkUp) > 0);
        PEGASUS_TEST_ASSERT(String::equalNoCase(degkLow,degkUp));

        String mapTest(degkLow);
        mapTest.toUpper();
        PEGASUS_TEST_ASSERT(String::equal(mapTest, degkUp));

        // Note that the German char does not round trip
        mapTest.toLower();
        PEGASUS_TEST_ASSERT(String::equal(mapTest, degkLow2));
#endif
    }

    {
        // Test of the [] operator
        String t1 = "abc";
        Char16 c = t1[1];
        // note c is Char16
        PEGASUS_TEST_ASSERT(c == 'b');

        //ATTN: test for outofbounds exception
        try
        {
            c = t1[200];
        }
        catch (IndexOutOfBoundsException&)
        {
        PEGASUS_TEST_ASSERT(true);
        }
    }

    {
        // Test the find function
        String t1 = "abcdef";
        String t2 = "cde";
        String t3 = "xyz";
        String t4 = "abc";
        String t5 = "abd";
        String t6 = "defg";
        PEGASUS_TEST_ASSERT(t1.find('c') == 2);
        PEGASUS_TEST_ASSERT(t1.find(t2)==2);
        PEGASUS_TEST_ASSERT(t1.find(t3)==PEG_NOT_FOUND);
        PEGASUS_TEST_ASSERT(t1.find(t4)==0);
        PEGASUS_TEST_ASSERT(t1.find(t5)==PEG_NOT_FOUND);
        PEGASUS_TEST_ASSERT(t1.find(t6)==PEG_NOT_FOUND);
        PEGASUS_TEST_ASSERT(t1.find("cde")==2);
        PEGASUS_TEST_ASSERT(t1.find("def")==3);
        PEGASUS_TEST_ASSERT(t1.find("xyz")==PEG_NOT_FOUND);
        PEGASUS_TEST_ASSERT(t1.find("a") ==0);

        // test for the case where string
        // partly occurs and then later
        // completely occurs
        String s = "this is an apple";
        PEGASUS_TEST_ASSERT(s.find("apple")==11);
        PEGASUS_TEST_ASSERT(s.find("appld")==PEG_NOT_FOUND);
        PEGASUS_TEST_ASSERT(s.find("this")==0);
        PEGASUS_TEST_ASSERT(s.find("t")==0);
        PEGASUS_TEST_ASSERT(s.find("e")==15);
        s = "a";
        PEGASUS_TEST_ASSERT(s.find("b")==PEG_NOT_FOUND);
        PEGASUS_TEST_ASSERT(s.find("a")==0);
        PEGASUS_TEST_ASSERT(s.find(s)==0);
        s = "aaaapple";
        PEGASUS_TEST_ASSERT(s.find("apple")==3);

        // 20020715-RK This method was removed from the String class
        //{
        //    String nameSpace = "a#b#c";
        //    nameSpace.translate('#', '/');
        //    PEGASUS_TEST_ASSERT(nameSpace == "a/b/c");
        //}
    }

    {
        //
        // Test String unicode enablement
        //

        char utf8chr[]    = {
                              '\xCE', '\x99', '\xCE', '\xBF', '\xCF', '\x8D',
                              '\xCE', '\xBD', '\xCE', '\xB9', '\xCE', '\xBA',
                              '\xCE', '\xBF', '\xCE', '\xBD', '\xCF', '\x84',
                              '\0'
                            }; // utf8 string with mutliple byte characters

        Char16 utf16chr[] = {
                              0x0399,0x03BF,0x03CD,0x03BD,0x03B9,
                              0x03BA,0x03BF,0x03BD,0x03C4,0x00
                            };  // utf16 representation of the utf8 string

        String utf16string(utf16chr);
        String utf8string(utf8chr);
        String utf16merge(utf8string.getChar16Data());

        CString temp = utf8string.getCString();
        CString temp2 = utf16string.getCString();

        PEGASUS_TEST_ASSERT(utf16string == utf8string);
        PEGASUS_TEST_ASSERT(utf16string == utf16merge);
        PEGASUS_TEST_ASSERT(utf16string == utf16chr);
        PEGASUS_TEST_ASSERT(utf8string  == utf16chr);

        PEGASUS_TEST_ASSERT(memcmp(utf8string.getChar16Data(),
                            utf16string.getChar16Data(),sizeof(utf16chr)) == 0);
        PEGASUS_TEST_ASSERT(strcmp(utf8string.getCString(),utf8chr) == 0);
        PEGASUS_TEST_ASSERT(strcmp(utf16string.getCString(),utf8chr) == 0);
        PEGASUS_TEST_ASSERT(strcmp(temp,utf8chr) == 0);
        PEGASUS_TEST_ASSERT(strcmp(temp2,utf8chr) == 0);

        Uint32 count = 0;
        Uint32 size = sizeof(utf8chr);
        while(count<size)
        {
                PEGASUS_TEST_ASSERT(isUTF8(&utf8chr[count]) == true);
                UTF8_NEXT(utf8chr,count);
        }

        // utf8 string with mutliple byte characters
        char utf8bad[] =
        {
            '\xFF','\xFF', '\xFF', '\0', '\0', '\0'
        };

        count = 0;
        size = 3;
        while(count<size)
        {
            PEGASUS_TEST_ASSERT(isUTF8(&utf8bad[count]) == false);
            UTF8_NEXT(utf8bad,count);
        }

        Char16 utf16Chars[] =
        {
        0x6A19, 0x6E96, 0x842C, 0x570B, 0x78BC,
        0x042E, 0x043D, 0x0438, 0x043A, 0x043E, 0x0434,
        0x110B, 0x1172, 0x1102, 0x1165, 0x110F, 0x1169, 0x11AE,
        0x10E3, 0x10DC, 0x10D8, 0x10D9, 0x10DD, 0x10D3, 0x10D8,
        0xdbc0, 0xdc01,
        0x05D9, 0x05D5, 0x05E0, 0x05D9, 0x05E7, 0x05D0, 0x05B8, 0x05D3,
        0x064A, 0x0648, 0x0646, 0x0650, 0x0643, 0x0648, 0x062F,
        0x092F, 0x0942, 0x0928, 0x093F, 0x0915, 0x094B, 0x0921,
        0x016A, 0x006E, 0x012D, 0x0063, 0x014D, 0x0064, 0x0065, 0x033D,
        0x00E0, 0x248B, 0x0061, 0x2173, 0x0062, 0x1EA6, 0xFF21, 0x00AA,
            0x0325, 0x2173, 0x249C, 0x0063,
        0x02C8, 0x006A, 0x0075, 0x006E, 0x026A, 0x02CC, 0x006B, 0x006F,
            0x02D0, 0x0064,
        0x30E6, 0x30CB, 0x30B3, 0x30FC, 0x30C9,
        0xFF95, 0xFF86, 0xFF7A, 0xFF70, 0xFF84, 0xFF9E,
        0xC720, 0xB2C8, 0xCF5B, 0x7D71, 0x4E00, 0x78BC,
        0xdbc0, 0xdc01,
        0x00};

        String ugly(utf16Chars);
        PEGASUS_TEST_ASSERT(ugly == utf16Chars);

        //
        // Test passing bad utf-8 into String
        //

        // A utf-8 sequence with a byte zeroed out in a bad spot
        char utf8bad1[]    = {
                              '\xCE', '\x99', '\xCE', '\xBF', '\xCF', '\x8D',
                              '\xCE', '\xBD', '\xCE', '\0', '\xCE', '\xBA',
                              '\xCE', '\xBF', '\xCE', '\xBD', '\xCF', '\x84',
                              '\0'
                            }; // utf8 string with mutliple byte characters

        // Test String(char *)
        try
        {
          // the first terminator causes invalid utf-8
          String tmp(utf8bad1);
          PEGASUS_TEST_ASSERT(false);
        }
        catch (Exception &)
        {
          // expect an error
        }

        // Test String(char *, Uint32)
        try
        {
          // bogus utf-8 char in the middle
          String tmp(utf8bad1, sizeof(utf8bad1)-1);
          PEGASUS_TEST_ASSERT(false);
        }
        catch (Exception &)
        {
          // expect an error
        }

        // Test String(char *, Uint32)
        try
        {
          // good, but the last utf-8 char extends past the last byte
          String tmp(utf8chr, sizeof(utf8chr) - 2);
          PEGASUS_TEST_ASSERT(false);
        }
        catch (Exception &)
        {
          // expect an error
        }

        // Test String::assign(char *)
        String assigntest(utf8chr);  // good so far
        try
        {
          // the first terminator causes invalid utf-8
          assigntest.assign(utf8bad1);  // bad
          PEGASUS_TEST_ASSERT(false);
        }
        catch (Exception &)
        {
          // expect an error
        }

        // Test String::assign(char *, Uint32)
        try
        {
          // bogus utf-8 char in the middle
          assigntest.assign(utf8bad1, sizeof(utf8bad1) - 1);  // bad
          PEGASUS_TEST_ASSERT(false);
        }
        catch (Exception &)
        {
          // expect an error
        }

        // Test String::assign(char *, Uint32)
        try
        {
          // good, but the last utf-8 char extends past the end
          assigntest.assign(utf8chr, sizeof(utf8chr) - 2);  // bad
          PEGASUS_TEST_ASSERT(false);
        }
        catch (Exception &)
        {
          // expect an error
        }

        //
        // Test passing in good utf-8 with an embedded terminator
        //

        // A utf-8 sequence with a byte zeroed out in an ok spot
        char utf8good1[]    = {
                              '\xCE', '\x99', '\xCE', '\xBF', '\xCF', '\x8D',
                              '\xCE', '\xBD', 'A', '\0', '\xCE', '\xBA',
                              '\xCE', '\xBF', '\xCE', '\xBD', '\xCF', '\x84',
                              '\0'
                            }; // utf8 string with mutliple byte characters

        // Test String(char *)
        try
        {
          // terminator after 5 chars
          String tmp(utf8good1);
          PEGASUS_TEST_ASSERT (tmp.size() == 5);
        }
        catch (Exception &)
        {
          // didn't see that one coming
          PEGASUS_TEST_ASSERT(false);
        }

        // Test String(char *, Uint32)
        try
        {
          // embedded terminator counts as 1 char
          String tmp(utf8good1, sizeof(utf8good1) - 1);
          PEGASUS_TEST_ASSERT (tmp.size() == 10);
        }
        catch (Exception &)
        {
          // didn't see that one coming
          PEGASUS_TEST_ASSERT(false);
        }

        assigntest.clear();

        // Test String::assign(char *)
        try
        {
          // terminator after 5 chars
          assigntest.assign(utf8good1);
          PEGASUS_TEST_ASSERT (assigntest.size() == 5);
        }
        catch (Exception &)
        {
          // didn't see that one coming
          PEGASUS_TEST_ASSERT(false);
        }

        assigntest.clear();

        // Test String::assign(char *, Uint32)
        try
        {
          // embedded terminator counts as 1 char
          assigntest.assign(utf8good1, sizeof(utf8good1) - 1);
          PEGASUS_TEST_ASSERT (assigntest.size() == 10);
        }
        catch (Exception &)
        {
          // didn't see that one coming
          PEGASUS_TEST_ASSERT(false);
        }


        //
        // Casing tests
        //

        String little("the quick brown fox jumped over the lazy dog");
        String    big("THE QUICK BROWN FOX JUMPED OVER THE LAZY DOG");

        String tmpBig = big;
        String tmpLittle = little;

        tmpBig.toLower();
        PEGASUS_TEST_ASSERT(tmpBig == little);

        tmpBig.toUpper();
        PEGASUS_TEST_ASSERT(tmpBig == big);
    }

#if 0
    // The match code has been removed from the String class
    // Test the string match functions
    {
        String abc = "abc";
        String ABC = "ABC";
        PEGASUS_TEST_ASSERT(String::match(abc, "abc"));
        PEGASUS_TEST_ASSERT(String::match(ABC, "ABC"));
        PEGASUS_TEST_ASSERT(!String::match(abc, "ABC"));
        PEGASUS_TEST_ASSERT(!String::match(ABC, "abc"));

        PEGASUS_TEST_ASSERT(String::matchNoCase(abc, "abc"));
        PEGASUS_TEST_ASSERT(String::matchNoCase(ABC, "abc"));
        PEGASUS_TEST_ASSERT(String::matchNoCase(abc, "ABC"));
        PEGASUS_TEST_ASSERT(String::matchNoCase(ABC, "ABc"));

        PEGASUS_TEST_ASSERT(String::match(abc, "???"));
        PEGASUS_TEST_ASSERT(String::match(ABC, "???"));
        PEGASUS_TEST_ASSERT(String::match(abc, "*"));
        PEGASUS_TEST_ASSERT(String::match(ABC, "*"));

        PEGASUS_TEST_ASSERT(String::match(abc, "?bc"));
        PEGASUS_TEST_ASSERT(String::match(abc, "?b?"));
        PEGASUS_TEST_ASSERT(String::match(abc, "??c"));
        PEGASUS_TEST_ASSERT(String::matchNoCase(ABC, "?bc"));
        PEGASUS_TEST_ASSERT(String::matchNoCase(ABC, "?b?"));
        PEGASUS_TEST_ASSERT(String::matchNoCase(ABC, "??c"));


        PEGASUS_TEST_ASSERT(String::match(abc, "*bc"));
        PEGASUS_TEST_ASSERT(String::match(abc, "a*c"));
        PEGASUS_TEST_ASSERT(String::match(abc, "ab*"));
        PEGASUS_TEST_ASSERT(String::match(abc, "a*"));
        // ATTN-RK-P3-20020603: This match code is broken
        //PEGASUS_TEST_ASSERT(String::match(abc, "[axy]bc"));
        PEGASUS_TEST_ASSERT(!String::match(abc, "[xyz]bc"));

        PEGASUS_TEST_ASSERT(!String::match(abc, "def"));
        PEGASUS_TEST_ASSERT(!String::match(abc, "[de]bc"));
        // ATTN-RK-P3-20020603: This match code is broken
        //PEGASUS_TEST_ASSERT(String::match(abc, "a[a-c]c"));
        PEGASUS_TEST_ASSERT(!String::match(abc, "a[d-x]c"));
        // ATTN-RK-P3-20020603: This match code does not yet handle escape chars
        //PEGASUS_TEST_ASSERT(String::match("*test", "\\*test"));

        PEGASUS_TEST_ASSERT(String::match("abcdef123", "*[0-9]"));

        PEGASUS_TEST_ASSERT(String::match("This is a test", "*is*"));
        PEGASUS_TEST_ASSERT(String::matchNoCase("This is a test", "*IS*"));

        PEGASUS_TEST_ASSERT(String::match("Hello", "Hello"));
        PEGASUS_TEST_ASSERT(String::matchNoCase("HELLO", "hello"));
        PEGASUS_TEST_ASSERT(String::match("This is a test", "This is *"));
        PEGASUS_TEST_ASSERT(String::match("This is a test", "* is a test"));
        PEGASUS_TEST_ASSERT(!String::match("Hello", "Goodbye"));

        String tPattern =
            "When in the * of human*e??nts it be?ome[sS] [0-9] nec*";


        try
        {
            String x(reinterpret_cast<const char *>(0));
            cerr <<
                 "Error: Exception not thrown on NULL passed to"
                 " constructor(const char *)"
                << endl;
        }

        catch ( const NullPointer & )
        {
            // This is the exception that should be thrown.
        }

        catch ( ... )
        {
            cerr <<
                "Error: Wrong exception thrown on NULL passed to"
                " constructor(const char *)"
                << endl;
        }


        try
        {
            String x(reinterpret_cast<const Char16 *>(0));
            cerr <<
                "Error: Exception not thrown on NULL passed to"
                " constructor(const Char16 *)"
                << endl;
        }

        catch ( const NullPointer & )
        {
            // This is the exception that should be thrown.
        }

        catch ( ... )
        {
            cerr <<
                "Error: Wrong exception thrown on NULL passed to"
                " constructor(const Char16 *)"
                << endl;
        }


        try
        {
            String x;

            x.assign(reinterpret_cast<const char *>(0));
            cerr <<
                "Error: Exception not thrown on NULL passed to"
                " assign(const char *)"
                << endl;
        }

        catch ( const NullPointer & )
        {
            // This is the exception that should be thrown.
        }

        catch ( ... )
        {
            cerr <<
                "Error: Wrong exception thrown on NULL passed to"
                " assign(const char *)"
                << endl;
        }


        try
        {
            String x;

            x.assign(reinterpret_cast<const Char16 *>(0));
            cerr <<
                "Error: Exception not thrown on NULL passed to"
                " assign(const Char16 *)"
                << endl;
        }

        catch ( const NullPointer & )
        {
            // This is the exception that should be thrown.
        }

        catch ( ... )
        {
            cerr <<
                "Error: Wrong exception thrown on NULL passed to"
                " assign(const Char16 *)"
                << endl;
        }


        try
        {
            String x;

            x.append(reinterpret_cast<const char *>(0));
            cerr <<
                "Error: Exception not thrown on NULL passed to"
                " append(const char *)"
                << endl;
        }

        catch ( const NullPointer & )
        {
            // This is the exception that should be thrown.
        }

        catch ( ... )
        {
            cerr <<
                "Error: Wrong exception thrown on NULL passed to"
                " append(const char *)"
                << endl;
        }


        try
        {
            String x;

            x.append(reinterpret_cast<const Char16 *>(0));
            cerr <<
                "Error: Exception not thrown on NULL passed to"
                " append(const Char16 *)"
                << endl;
        }

        catch ( const NullPointer & )
        {
            // This is the exception that should be thrown.
        }

        catch ( ... )
        {
            cerr <<
                "Error: Wrong exception thrown on NULL passed"
                " to append(const Char16 *)"
                << endl;
        }

        // ATTN-RK-P3-20020603: This match code is broken
        //PEGASUS_TEST_ASSERT(String::match(
        //    "When in the course of human events it becomes 0 necessary",
        //    tPattern));
        //PEGASUS_TEST_ASSERT(String::match(
        //    "When in the xyz of human events it becomes 9 necessary",
        //    tPattern));
        //PEGASUS_TEST_ASSERT(String::match(
        //    "When in the  of human events it becomes 3 necessary",
        //    tPattern));
    }
#endif

    // string()
    {
    String s;
    PEGASUS_TEST_ASSERT(s.size() == 0);
    PEGASUS_TEST_ASSERT(s[0] == '\0');
    }

    // String(const String& s)
    {
    const String s("hello");
    const String t = s;
    PEGASUS_TEST_ASSERT(s.size() == strlen("hello"));
    PEGASUS_TEST_ASSERT(s == "hello");
    PEGASUS_TEST_ASSERT(t.size() == strlen("hello"));
    PEGASUS_TEST_ASSERT(t == "hello");
    }

    // String(const char*)
    {
    const String s("hello");
    PEGASUS_TEST_ASSERT(s.size() == strlen("hello"));
    PEGASUS_TEST_ASSERT(s == "hello");
    }

    // reserve()
    {
    String s;
    s.reserveCapacity(100);
    PEGASUS_TEST_ASSERT(s.size() == 0);
    // PEGASUS_TEST_ASSERT(s.getCapacity() >= 100);

    String t("hello world");
    PEGASUS_TEST_ASSERT(t.size() == strlen("hello world"));
    t.reserveCapacity(500);
    PEGASUS_TEST_ASSERT(t.size() == strlen("hello world"));
    PEGASUS_TEST_ASSERT(t == "hello world");
    }

    // assign(const String&)
    {
    String s("this is a test");
    String t;

    t = s;
    PEGASUS_TEST_ASSERT(s.size() == strlen("this is a test"));
    PEGASUS_TEST_ASSERT(s == "this is a test");
    PEGASUS_TEST_ASSERT(t.size() == strlen("this is a test"));
    PEGASUS_TEST_ASSERT(t == "this is a test");

    s = t;
    PEGASUS_TEST_ASSERT(s.size() == strlen("this is a test"));
    PEGASUS_TEST_ASSERT(s == "this is a test");
    PEGASUS_TEST_ASSERT(t.size() == strlen("this is a test"));
    PEGASUS_TEST_ASSERT(t == "this is a test");
    }

    // assign(const char*, size_t)
    {
    const char MESSAGE[] = "x";
    const size_t LENGTH = sizeof(MESSAGE) - 1;
    String s;
    s.assign(MESSAGE, LENGTH);
    PEGASUS_TEST_ASSERT(s.size() == LENGTH);
    PEGASUS_TEST_ASSERT(s == MESSAGE);

    String t("dummy", 5);
    t.assign(MESSAGE, LENGTH);
    PEGASUS_TEST_ASSERT(t.size() == LENGTH);
    PEGASUS_TEST_ASSERT(t == MESSAGE);
    }

    // assign(const char*)
    {
    const char MESSAGE[] = "x";
    const size_t LENGTH = sizeof(MESSAGE) - 1;
    String s;
    s.assign(MESSAGE);
    PEGASUS_TEST_ASSERT(s.size() == LENGTH);
    PEGASUS_TEST_ASSERT(s == MESSAGE);

    String t("dummy", 5);
    t.assign(MESSAGE);
    PEGASUS_TEST_ASSERT(t.size() == LENGTH);
    PEGASUS_TEST_ASSERT(t == MESSAGE);
    }

    // append(const String&)
    {
    String s;

    s.append(String("xxx"));
    PEGASUS_TEST_ASSERT(s.size() == 3);
    PEGASUS_TEST_ASSERT(s == "xxx");

    s.append(String("yyy"));
    PEGASUS_TEST_ASSERT(s.size() == 6);
    PEGASUS_TEST_ASSERT(s == "xxxyyy");

    s.append(String("zzz"));
    PEGASUS_TEST_ASSERT(s.size() == 9);
    PEGASUS_TEST_ASSERT(s == "xxxyyyzzz");
    }

    // append(const char*)
    {
    String s;

    s.append("xxx");
    PEGASUS_TEST_ASSERT(s.size() == 3);
    PEGASUS_TEST_ASSERT(s == "xxx");

    s.append("yyy");
    PEGASUS_TEST_ASSERT(s.size() == 6);
    PEGASUS_TEST_ASSERT(s == "xxxyyy");

    s.append("zzz");
    PEGASUS_TEST_ASSERT(s.size() == 9);
    PEGASUS_TEST_ASSERT(s == "xxxyyyzzz");
    }

    // append(const char*)
    {
    String s;

    s.append("xxx");
    PEGASUS_TEST_ASSERT(s.size() == 3);
    PEGASUS_TEST_ASSERT(s == "xxx");

    s.append("yyy");
    PEGASUS_TEST_ASSERT(s.size() == 6);
    PEGASUS_TEST_ASSERT(s == "xxxyyy");

    s.append("zzz");
    PEGASUS_TEST_ASSERT(s.size() == 9);
    PEGASUS_TEST_ASSERT(s == "xxxyyyzzz");
    }

    // append(char)
    {
    String s;

    for (int i = 'a'; i <= 'z'; i++)
    {
        Char16 c = i;
        s.append(c);
    }

    PEGASUS_TEST_ASSERT(s.size() == 26);
    PEGASUS_TEST_ASSERT(s == "abcdefghijklmnopqrstuvwxyz");
    }

    // clear()
    {
    String s("abc");
    String t = s;
    String u = s;

    s.clear();
    PEGASUS_TEST_ASSERT(t.size() == 3);
    PEGASUS_TEST_ASSERT(t == "abc");
    PEGASUS_TEST_ASSERT(t[0] == 'a');
    PEGASUS_TEST_ASSERT(u.size() == 3);
    PEGASUS_TEST_ASSERT(u == "abc");
    PEGASUS_TEST_ASSERT(u[0] == 'a');
    PEGASUS_TEST_ASSERT(s.size() == 0);
    PEGASUS_TEST_ASSERT(s[0] == '\0');

    t.clear();
    PEGASUS_TEST_ASSERT(t.size() == 0);
    PEGASUS_TEST_ASSERT(t[0] == '\0');
    PEGASUS_TEST_ASSERT(t == "");
    PEGASUS_TEST_ASSERT(u.size() == 3);
    PEGASUS_TEST_ASSERT(u == "abc");
    PEGASUS_TEST_ASSERT(u[0] == 'a');
    PEGASUS_TEST_ASSERT(s.size() == 0);
    PEGASUS_TEST_ASSERT(s == "");
    PEGASUS_TEST_ASSERT(s[0] == '\0');

    u.clear();
    PEGASUS_TEST_ASSERT(t.size() == 0);
    PEGASUS_TEST_ASSERT(t == "");
    PEGASUS_TEST_ASSERT(t[0] == '\0');
    PEGASUS_TEST_ASSERT(u.size() == 0);
    PEGASUS_TEST_ASSERT(u == "");
    PEGASUS_TEST_ASSERT(u[0] == '\0');
    PEGASUS_TEST_ASSERT(s.size() == 0);
    PEGASUS_TEST_ASSERT(s == "");
    PEGASUS_TEST_ASSERT(s[0] == '\0');
    }

    // c_str()
    {
    String s("abc");
    String t("abc");
    String u("def");
    String v;
    String w("");

    PEGASUS_TEST_ASSERT(s == "abc");
    PEGASUS_TEST_ASSERT(t == "abc");
    PEGASUS_TEST_ASSERT(u == "def");
    PEGASUS_TEST_ASSERT(s == t);
    PEGASUS_TEST_ASSERT(s != u);
    PEGASUS_TEST_ASSERT(v == "");
    PEGASUS_TEST_ASSERT(v[0] == '\0');
    PEGASUS_TEST_ASSERT(v[0] == '\0');
    PEGASUS_TEST_ASSERT(w.size() == 0);
    PEGASUS_TEST_ASSERT(w[0] == '\0');
    PEGASUS_TEST_ASSERT(w[0] == '\0');
    }

    // set(size_t, char)
    {
    String s("abcdefghijklmnopqrstuvwxyz");

    for (int i = 0; i < 26; i++)
        s[i] = toupper(s[i]);

    PEGASUS_TEST_ASSERT(s == "ABCDEFGHIJKLMNOPQRSTUVWXYZ");
    }

    // equal(const String&)
    {
    String t("abc");
    String u("abc");
    String v("def");
    String w("defg");
    String x("");
    String y("");

    PEGASUS_TEST_ASSERT(String::equal(t, t));
    PEGASUS_TEST_ASSERT(String::equal(u, u));
    PEGASUS_TEST_ASSERT(String::equal(v, v));
    PEGASUS_TEST_ASSERT(String::equal(w, w));
    PEGASUS_TEST_ASSERT(String::equal(x, x));
    PEGASUS_TEST_ASSERT(String::equal(y, y));

    PEGASUS_TEST_ASSERT(String::equal(t, u));
    PEGASUS_TEST_ASSERT(String::equal(u, t));

    PEGASUS_TEST_ASSERT(!String::equal(t, v));
    PEGASUS_TEST_ASSERT(!String::equal(t, w));
    PEGASUS_TEST_ASSERT(!String::equal(t, x));
    PEGASUS_TEST_ASSERT(!String::equal(t, y));
    PEGASUS_TEST_ASSERT(!String::equal(v, t));
    PEGASUS_TEST_ASSERT(!String::equal(w, t));
    PEGASUS_TEST_ASSERT(!String::equal(x, t));
    PEGASUS_TEST_ASSERT(!String::equal(y, t));

    PEGASUS_TEST_ASSERT(!String::equal(v, w));
    PEGASUS_TEST_ASSERT(!String::equal(w, v));
    PEGASUS_TEST_ASSERT(String::equal(x, y));
    PEGASUS_TEST_ASSERT(String::equal(y, x));
    }

    // equal(const char*)
    {
    String t("abc");
    String u("abc");
    String v("def");
    String w("defg");
    String x("");
    String y("");

    PEGASUS_TEST_ASSERT(String::equal(t, "abc"));
    PEGASUS_TEST_ASSERT(String::equal(u, "abc"));
    PEGASUS_TEST_ASSERT(String::equal(v, "def"));
    PEGASUS_TEST_ASSERT(String::equal(w, "defg"));
    PEGASUS_TEST_ASSERT(String::equal(x, ""));
    PEGASUS_TEST_ASSERT(String::equal(y, ""));

    PEGASUS_TEST_ASSERT(String::equal(t, "abc"));
    PEGASUS_TEST_ASSERT(String::equal(u, "abc"));

    PEGASUS_TEST_ASSERT(!String::equal(t, "def"));
    PEGASUS_TEST_ASSERT(!String::equal(t, "defg"));
    PEGASUS_TEST_ASSERT(!String::equal(t, ""));
    PEGASUS_TEST_ASSERT(!String::equal(t, ""));
    PEGASUS_TEST_ASSERT(!String::equal(v, "abc"));
    PEGASUS_TEST_ASSERT(!String::equal(w, "abc"));
    PEGASUS_TEST_ASSERT(!String::equal(x, "abc"));
    PEGASUS_TEST_ASSERT(!String::equal(y, "abc"));

    PEGASUS_TEST_ASSERT(!String::equal(v, "defg"));
    PEGASUS_TEST_ASSERT(!String::equal(w, "def"));
    PEGASUS_TEST_ASSERT(String::equal(x, ""));
    PEGASUS_TEST_ASSERT(String::equal(y, ""));
    }

    // equali()
    {
    String s("abc");
    String t("abC");
    String u("ABC");
    String v("xyz");
    String w("");
    String x("");
    PEGASUS_TEST_ASSERT(String::equalNoCase(s, t));
    PEGASUS_TEST_ASSERT(String::equalNoCase(s, u));
    PEGASUS_TEST_ASSERT(!String::equalNoCase(s, v));
    PEGASUS_TEST_ASSERT(String::equalNoCase(w, x));
    PEGASUS_TEST_ASSERT(!String::equalNoCase(w, s));
    PEGASUS_TEST_ASSERT(!String::equalNoCase(w, t));
    PEGASUS_TEST_ASSERT(!String::equalNoCase(w, v));
    }

    {

    String t;
    const char MESSAGE[] = "hello";
    const size_t LENGTH = sizeof(MESSAGE) - 1;
    String s = String(MESSAGE);
    t = s;
    String u = String(t);

    PEGASUS_TEST_ASSERT(t.size() == LENGTH);
    PEGASUS_TEST_ASSERT(t == MESSAGE);
    PEGASUS_TEST_ASSERT(s.size() == LENGTH);
    PEGASUS_TEST_ASSERT(s == MESSAGE);
    PEGASUS_TEST_ASSERT(u.size() == LENGTH);
    PEGASUS_TEST_ASSERT(u == MESSAGE);

    PEGASUS_TEST_ASSERT(t[0] == 'h');
    PEGASUS_TEST_ASSERT(t[1] == 'e');
    PEGASUS_TEST_ASSERT(t[2] == 'l');
    PEGASUS_TEST_ASSERT(t[3] == 'l');
    PEGASUS_TEST_ASSERT(t[4] == 'o');
    PEGASUS_TEST_ASSERT(t[5] == '\0');

    t.append(" world");
    PEGASUS_TEST_ASSERT(t.size() == strlen("hello world"));
    PEGASUS_TEST_ASSERT(t == "hello world");
    PEGASUS_TEST_ASSERT(s != "hello world");
    PEGASUS_TEST_ASSERT(s == "hello");
    PEGASUS_TEST_ASSERT(s.size() == strlen("hello"));

    t[0] = 'x';
    PEGASUS_TEST_ASSERT(t == "xello world");
    }

    // remove()
    {
    String s("abcXYZdefLMNOP");

    s.remove(0,0);
    PEGASUS_TEST_ASSERT(s.size() == 14);
    PEGASUS_TEST_ASSERT(s == "abcXYZdefLMNOP");

    s.remove(0, 3);
    PEGASUS_TEST_ASSERT(s.size() == 11);
    PEGASUS_TEST_ASSERT(s == "XYZdefLMNOP");

    s.remove(3, 3);
    PEGASUS_TEST_ASSERT(s.size() == 8);
    PEGASUS_TEST_ASSERT(s == "XYZLMNOP");

    s.remove(7, 1);
    PEGASUS_TEST_ASSERT(s.size() == 7);
    PEGASUS_TEST_ASSERT(s == "XYZLMNO");

    s.remove(0, 1);
    PEGASUS_TEST_ASSERT(s.size() == 6);
    PEGASUS_TEST_ASSERT(s == "YZLMNO");

    s.remove(2, PEG_NOT_FOUND);
    PEGASUS_TEST_ASSERT(s.size() == 2);
    PEGASUS_TEST_ASSERT(s == "YZ");

    s.remove(2, 0);
    PEGASUS_TEST_ASSERT(s.size() == 2);
    PEGASUS_TEST_ASSERT(s == "YZ");

    s.remove(1, 1);
    PEGASUS_TEST_ASSERT(s.size() == 1);
    PEGASUS_TEST_ASSERT(s == "Y");

    s.remove(0, 1);
    PEGASUS_TEST_ASSERT(s.size() == 0);
    PEGASUS_TEST_ASSERT(s == "");
    PEGASUS_TEST_ASSERT(s[0] == '\0');

    s.remove(0,0);
    PEGASUS_TEST_ASSERT(s.size() == 0);
    PEGASUS_TEST_ASSERT(s == "");
    PEGASUS_TEST_ASSERT(s[0] == '\0');
    }

    // subString()
    {
    String s("one two three");
    PEGASUS_TEST_ASSERT(s.subString(0) == "one two three");
    PEGASUS_TEST_ASSERT(s.subString(0, 3) == "one");
    PEGASUS_TEST_ASSERT(s.subString(4, 3) == "two");
    PEGASUS_TEST_ASSERT(s.subString(8, 5) == "three");
    PEGASUS_TEST_ASSERT(s.subString(0, 0) == "");
    PEGASUS_TEST_ASSERT(s.subString(13, 0) == "");
    }

    // Overflow
    bool caught_bad_alloc = false;
    try
    {
    String s("junk", Uint32(0xFFFFFFFF));
    }
    catch(...)
    {
    caught_bad_alloc = true;
    }
    PEGASUS_TEST_ASSERT(caught_bad_alloc);

    // Added to test funtionality of String(const String& str, Uint32 n)
    // for memory overflow.
    Boolean caughtBadAlloc = false;
    try
    {
        String s("abc", 0xFFFF0000);   //to check for alloc
    }
    catch(const PEGASUS_STD(bad_alloc)&)
    {
        caughtBadAlloc = true;
    }

    // Added to test funtionality of
    // void reserveCapacity(Uint32 capacity) for memory overflow.
    caughtBadAlloc = false;
    try
    {
        String s;
        s.reserveCapacity(0xFFFF0000);   //to check for _reserve
    }
    catch (const PEGASUS_STD(bad_alloc)&)
    {
        caughtBadAlloc = true;
    }
    PEGASUS_TEST_ASSERT(caughtBadAlloc);

    // Added to test funtionality of
    // String& append(const char* str, Uint32 size) for memory overflow.
    caughtBadAlloc = false;
    try
    {
        String s;
        s.append("xxx", 0xFFFF0000);         //to check for _reserve
    }
    catch (const PEGASUS_STD(bad_alloc)&)
    {
        caughtBadAlloc = true;
    }
    PEGASUS_TEST_ASSERT(caughtBadAlloc);

    // Added to test funtionality of
    // String& append(const Char16* str, Uint32 n) for memory overflow.
    caughtBadAlloc = false;
    try
    {
        String s;
        s.append((Char16 *)"xxx", 0xFFFF0000); //to check for _reserve
    }
    catch (const PEGASUS_STD(bad_alloc)&)
    {
        caughtBadAlloc = true;
    }
    PEGASUS_TEST_ASSERT(caughtBadAlloc);

    // Added to test funtionality of
    // String& assign(const char* str, Uint32 n) for memory overflow.
    caughtBadAlloc = false;
    try
    {
        String s;
        s.assign("xxx", 0xFFFF0000);               //to check for alloc
    }
    catch (const PEGASUS_STD(bad_alloc)&)
    {
    caughtBadAlloc = true;
    }
    PEGASUS_TEST_ASSERT(caughtBadAlloc);

    // Added to test funtionality of
    // String& assign(const Char16* str, Uint32 n) for memory overflow.
    caughtBadAlloc = false;
    try
    {
        String s;
        s.assign((Char16 *)"xxx", 0xFFFF0000);   //to check for alloc
    }
    catch (const PEGASUS_STD(bad_alloc)&)
    {
        caughtBadAlloc = true;
    }
    PEGASUS_TEST_ASSERT(caughtBadAlloc);

    // Tests for the dump of a bad strings.

    // 40 good chars, bad char, 10 trailing chars.
    //    Testing the full range of printed data.

    caughtBadAlloc = false;
    try
    {
        String s1("1234567890123456789012345678901234567890""\xFF""1234567890");
    }
    catch ( Exception& ex )
    {
        caughtBadAlloc = true;
        PEGASUS_TEST_ASSERT(
            strstr((const char*)ex.getMessage().getCString(),
                ": 1234567890123456789012345678901234567890"
                    " 0xFF 0x31 0x32 0x33 0x34 0x35 0x36 0x37 0x38 0x39 0x30")
            != 0);

    }
    PEGASUS_TEST_ASSERT(caughtBadAlloc);

    // 7 good chars, bad char, 7 trailing chars.
    // Testing reduced number of chars arround the bad char.

    caughtBadAlloc = false;
    try
    {
        String s1("0123456""\xAA""0123456");
    }
    catch ( Exception& ex )
    {
        caughtBadAlloc = true;
        PEGASUS_TEST_ASSERT(
            strstr((const char*)ex.getMessage().getCString(),
                ": 0123456 0xAA 0x30 0x31 0x32 0x33 0x34 0x35 0x36")
            != 0);

    }
    PEGASUS_TEST_ASSERT(caughtBadAlloc);

    // No good chars, bad char , 20 trailing chars
    // Testing no good chars, bad char at beginning,
    // more trailing chars available then printed.

    caughtBadAlloc = false;
    try
    {
        String s1("\xDD""0123456789012345679");
    }
    catch ( Exception& ex )
    {
        caughtBadAlloc = true;
        PEGASUS_TEST_ASSERT(
            strstr((const char*)ex.getMessage().getCString(),
                ":  0xDD 0x30 0x31 0x32 0x33 0x34 0x35 0x36 0x37 0x38 0x39")
            != 0);

    }
    PEGASUS_TEST_ASSERT(caughtBadAlloc);

    // 50 good chars ( more the printed ), bad char as last char,
    // no trailing chars.

    caughtBadAlloc = false;
    try
    {
        String s1("AAAAAAAAAA0123456789012345678901234567890123456789""\xBB");
    }
    catch ( Exception& ex )
    {
        caughtBadAlloc = true;
        PEGASUS_TEST_ASSERT(
            strstr((const char*)ex.getMessage().getCString(),
                ": 0123456789012345678901234567890123456789 0xBB")
            != 0);

    }
    PEGASUS_TEST_ASSERT(caughtBadAlloc);

    // no good chars, bad char as the only char, no trailing chars.

    caughtBadAlloc = false;
    try
    {
        String s1("\x80");
    }
    catch ( Exception& ex )
    {
        caughtBadAlloc = true;
        PEGASUS_TEST_ASSERT(
            strstr((const char*)ex.getMessage().getCString(),":  0x80") != 0);
    }
    PEGASUS_TEST_ASSERT(caughtBadAlloc);

    char* p = (char*)operator new(88888);
    operator delete(p);
}

// Test appendPrintf
void testappendPrintf()
{
    // Test the various basic C++ basic integer  and string types
    {
        String prtf;
        prtf.appendPrintf("%u", 1234);
        PEGASUS_TEST_ASSERT(prtf == "1234");
    }

    {
        String prtf;
        const char* teststr = "1234";
        prtf.appendPrintf("%s", teststr);
        VCOUT << __LINE__ << " " << prtf << endl;
        PEGASUS_TEST_ASSERT(prtf == teststr);
    }

    {
        String prtf;
        const char* teststr = "123456789abcdefghighklmnopqrstuvwxyz!@#$%^&*()";
        prtf.appendPrintf("%s", teststr);
        VCOUT << __LINE__ << " " << prtf << endl;
        PEGASUS_TEST_ASSERT(prtf == teststr);
    }

    {
        String prtf;
        unsigned int usi = 948;
        prtf.appendPrintf("%u", usi);
        VCOUT << __LINE__ << " " << prtf << endl;
        PEGASUS_TEST_ASSERT(prtf == "948");
    }
    {
        String prtf;
        unsigned int si = -948;
        prtf.appendPrintf("%d", si);
        VCOUT << __LINE__ << " " << prtf << endl;
        PEGASUS_TEST_ASSERT(prtf == "-948");
    }
    {
        String prtf;
        unsigned long int  uli= 888888;
        prtf.appendPrintf("%lu", uli);
        VCOUT << __LINE__ << " " << prtf << endl;
        PEGASUS_TEST_ASSERT(prtf == "888888");

    }

    // test against the Pegasus types cast
    {
        String prtf;
        Uint64 u64 = 99999;
        prtf.appendPrintf("%lu", (unsigned long int) u64);
        VCOUT << __LINE__ << " " << prtf << endl;
        PEGASUS_TEST_ASSERT(prtf == "99999");
    }

    {
        String prtf;
        Uint32 u32 = 32512;
        prtf.appendPrintf("%u", (unsigned int)u32);
        VCOUT << __LINE__ << " " << prtf << endl;
        PEGASUS_TEST_ASSERT(prtf == "32512");
    }

    {
        String prtf;
        Uint32 u32 = 32;
        Uint64 u64 = 99999;
        Sint32 s32 = -12;
        signed long int  sli = 12345678;
        char str1[] = "abcd";
        String str2 = "cdef";
        const char* str3 = "defg";
        prtf.appendPrintf(" Test %u %lu %d %ld %s%s%s%s ", (unsigned int)u32,
               (unsigned long)u64, (int)s32, sli,
               str1, (const char *)str2.getCString(), str3, "hijk");

        String result = " Test 32 99999 -12 12345678 abcdcdefdefghijk ";
        VCOUT << "prtf    " << prtf << "\nresult  " << result << endl;
        PEGASUS_TEST_ASSERT(prtf == result);
    }

    {
        String prtf = "abcd";
        prtf.appendPrintf("%s", "efgh");
        prtf.appendPrintf("%s", "ijkl");
        String result = "abcdefghijkl";
        PEGASUS_TEST_ASSERT(prtf == result);
    }

    {
        Uint32 a = 1;
        Uint32 b = 2;
        Uint32 c = 3;
        Uint32 d = 4;
        Uint64 e = 99999;

        String prtf = "abcd";
        prtf.appendPrintf("%s", "efgh");
        prtf.appendPrintf("%s", "ijkl");
        prtf.appendPrintf("this is going to be a big string so we realloc%u",a);
        prtf.appendPrintf("this is going to be a big string so we realloc%u",b);
        prtf.appendPrintf("this is going to be a big string so we realloc%u",c);
        prtf.appendPrintf("this is going to be a big string so we realloc%u",d);
        prtf.appendPrintf("more big string so we realloc%llu",e);
        prtf.appendPrintf("%s", "efgh");
        prtf.appendPrintf("%s", "ijkl");
        prtf.appendPrintf("this is going to be a big string so we realloc%u",a);
        prtf.appendPrintf("this is going to be a big string so we realloc%u",b);
        prtf.appendPrintf("this is going to be a big string so we realloc%u",c);
        prtf.appendPrintf("this is going to be a big string so we realloc%u",d);
        prtf.appendPrintf("more big string so we realloc%llu",e);
        prtf.appendPrintf("%s", "efgh");
        prtf.appendPrintf("%s", "ijkl");
        prtf.appendPrintf("this is going to be a big string so we realloc%u",a);
        prtf.appendPrintf("this is going to be a big string so we realloc%u",b);
        prtf.appendPrintf("this is going to be a big string so we realloc%u",c);
        prtf.appendPrintf("this is going to be a big string so we realloc%u",d);
        prtf.appendPrintf("more big string so we realloc%llu",e);
        prtf.appendPrintf("%s", "efgh");
        prtf.appendPrintf("%s", "ijkl");
        prtf.appendPrintf("this is going to be a big string so we realloc%u",a);
        prtf.appendPrintf("this is going to be a big string so we realloc%u",b);
        prtf.appendPrintf("this is going to be a big string so we realloc%u",c);
        prtf.appendPrintf("this is going to be a big string so we realloc%u",d);
        prtf.appendPrintf("more big string so we realloc%llu",e);
        prtf.appendPrintf("%s", "efgh");
        prtf.appendPrintf("%s", "ijkl");
        prtf.appendPrintf("this is going to be a big string so we realloc%u",a);
        prtf.appendPrintf("this is going to be a big string so we realloc%u",b);
        prtf.appendPrintf("this is going to be a big string so we realloc%u",c);
        prtf.appendPrintf("this is going to be a big string so we realloc%u",d);
        prtf.appendPrintf("more big string so we realloc%llu",e);
        prtf.appendPrintf("%s", "efgh");
        prtf.appendPrintf("%s", "ijkl");
        prtf.appendPrintf("this is going to be a big string so we realloc%u",a);
        prtf.appendPrintf("this is going to be a big string so we realloc%u",b);
        prtf.appendPrintf("this is going to be a big string so we realloc%u",c);
        prtf.appendPrintf("this is going to be a big string so we realloc%u",d);
        prtf.appendPrintf("more big string so we realloc%llu",e);
        prtf.appendPrintf("%s", "efgh");
        prtf.appendPrintf("%s", "ijkl");
        prtf.appendPrintf("this is going to be a big string so we realloc%u",a);
        prtf.appendPrintf("this is going to be a big string so we realloc%u",b);
        prtf.appendPrintf("this is going to be a big string so we realloc%u",c);
        prtf.appendPrintf("this is going to be a big string so we realloc%u",d);
        prtf.appendPrintf("more big string so we realloc%llu",e);
        prtf.appendPrintf("%s", "efgh");
        prtf.appendPrintf("%s", "ijkl");
        prtf.appendPrintf("this is going to be a big string so we realloc%u",a);
        prtf.appendPrintf("this is going to be a big string so we realloc%u",b);
        prtf.appendPrintf("this is going to be a big string so we realloc%u",c);
        prtf.appendPrintf("this is going to be a big string so we realloc%u",d);
        prtf.appendPrintf("more big string so we realloc%llu",e);
        String result = "abcdefghijklthis is going to be a big string so we"
                        " realloc1this is going to be a big string so we "
                        "realloc2this is going to be a big string so we "
                        "realloc3this is going to be a big string so we "
                        "realloc4more big string so we realloc99999efghijklthis"
                        " is going to be a big string so we realloc1this is"
                        " going to be a big string so we realloc2this is going"
                        " to be a big string so we realloc3this is going to be"
                        " a big string so we realloc4more big string so we"
                        " realloc99999efghijklthis is going to be a big string"
                        " so we realloc1this is going to be a big string so"
                        " we realloc2this is going to be a big string so we"
                        " realloc3this is going to be a big string so we"
                        " realloc4more big string so we realloc99999efgh"
                        "ijklthis is going to be a big string so we realloc"
                        "1this is going to be a big string so we realloc2th"
                        "is is going to be a big string so we realloc3this "
                        "is going to be a big string so we realloc4more big"
                        " string so we realloc99999efghijklthis is going to"
                        " be a big string so we realloc1this is going to be"
                        " a big string so we realloc2this is going to be a big"
                        " string so we realloc3this is going to be a big"
                        " string so we realloc4more big string so we "
                        "realloc99999efghijklthis is going to be a big string"
                        " so we realloc1this is going to be a big string so we"
                        " realloc2this is going to be a big string so we"
                        " realloc3this is going to be a big string so we reall"
                        "oc4more big string so we realloc99999efghijklthis is"
                        " going to be a big string so we realloc1this is going"
                        " to be a big string so we realloc2this is going to be"
                        " a big string so we realloc3this is going to be a big"
                        " string so we realloc4more big string so we realloc"
                        "99999efghijklthis is going to be a big string so we"
                        " realloc1this is going to be a big string so we rea"
                        "lloc2this is going to be a big string so we realloc"
                        "3this is going to be a big string so we realloc4mor"
                        "e big string so we realloc99999";
        VCOUT  << prtf << endl;

        PEGASUS_TEST_ASSERT(prtf == result);

    }
// The following are not normally compiled.  They test that compile
// errors are generated on at least the supported compilers when
// there are differences between the format string and the input arguments
// Today this is only the gcc compiler

// Enable following define to test compile warning generation.
//#define COMPILER_ERROR_TESTS
#ifdef COMPILER_ERROR_TESTS
    // Test of compiler generating errors in printf specification
    {
        String prtf;
        prtf.appendPrintf("%u incompatible type", "1234");
        VCOUT << __LINE__ << prtf << endl;
        PEGASUS_TEST_ASSERT(prtf == "1234");
    }
    {
        String prtf;
        prtf.appendPrintf("%s incompatible type", 1234);
        VCOUT << __LINE__ << " " << prtf << endl;
        PEGASUS_TEST_ASSERT(prtf == "1234");
    }
    {
        String prtf;
        Uint64 u64 = 99999;
        prtf.appendPrintf("%lu no cast", u64);
        VCOUT << __LINE__ << " " << prtf << endl;
        PEGASUS_TEST_ASSERT(prtf == "99999");
    }
    {
        String prtf;
        Uint32 u32 = 32512;
        prtf.appendPrintf("%lu incompatible type", (unsigned int)u32);
        VCOUT << __LINE__ << " " << prtf << endl;
        PEGASUS_TEST_ASSERT(prtf == "32512");
    }
    {
        String prtf;
        Uint32 u32 = 32512;
        prtf.appendPrintf("No format cmds", (unsigned int)u32);
        VCOUT << __LINE__ << " " << prtf << endl;
        PEGASUS_TEST_ASSERT(prtf == "32512");
    }
    {
        String prtf;
        Uint32 u32 = 32512;
        prtf.appendPrintf("%u using Uint32", u32);
        VCOUT << __LINE__ << " " << prtf << endl;
        PEGASUS_TEST_ASSERT(prtf == "32512");
    }
    {
        String prtf;
        Uint32 u32 = 32512;
        prtf.appendPrintf("%u extra arg",(unsigned int)u32, "arg 2");
        VCOUT << __LINE__ << " " << prtf << endl;
        PEGASUS_TEST_ASSERT(prtf == "32512");
    }
#endif

}

int main(int, char** argv)
{
    verbose = (getenv("PEGASUS_TEST_VERBOSE")) ? true : false;

    test1();
    testappendPrintf();

    cout << argv[0] << " +++++ passed all tests" << endl;
    return 0;
}
