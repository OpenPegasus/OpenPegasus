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
//%////////////////////////////////////////////////////////////////////////////

#include <cstdlib>
#include <iostream>

#include <Pegasus/Common/PegasusAssert.h>
#include <Pegasus/Common/AcceptLanguageList.h>
#include <Pegasus/Common/ContentLanguageList.h>
#include <Pegasus/Common/LanguageParser.h>
#include <Pegasus/Common/MessageLoader.h>
#include <Pegasus/Common/ThreadPool.h>
#include <Pegasus/Common/String.h>
#include <Pegasus/Common/Array.h>
#include <Pegasus/Common/InternalException.h>
#include <Pegasus/Common/Tracer.h>

#ifdef PEGASUS_HAS_ICU
# include <unicode/uloc.h>
#endif

PEGASUS_USING_PEGASUS;
PEGASUS_USING_STD;

static bool verbose = true;
static const char* defaultResourceBundle = "de";
static bool testRootBundle = false;
static Semaphore threadSync(0);

void testLanguageParser()
{
    // Test the LanguageTag parser
    {
        String tag1("en-US-mn-blah-blah");
        String language;
        String country;
        String variant;

        LanguageParser::parseLanguageTag(tag1, language, country, variant);

        PEGASUS_TEST_ASSERT(language == "en");
        PEGASUS_TEST_ASSERT(country == "US");
        PEGASUS_TEST_ASSERT(variant == "mn-blah-blah");
    }

    // Test handling of Accept-Languages whitespace and comments
    {
        AcceptLanguageList al = LanguageParser::parseAcceptLanguageHeader(
            "    en-US-mn (should not appear)  ,"
            "(and)en-US-ca   (!!!)  ;(less) q(uality) = (just) 0.5 (half)  ");
        PEGASUS_TEST_ASSERT(al.size() == 2);
        PEGASUS_TEST_ASSERT(al.getLanguageTag(0).toString() == "en-US-mn");
        PEGASUS_TEST_ASSERT(al.getQualityValue(0) == 1.0);
        PEGASUS_TEST_ASSERT(al.getLanguageTag(1).toString() == "en-US-ca");
        PEGASUS_TEST_ASSERT(al.getQualityValue(1) == 0.5);
        PEGASUS_TEST_ASSERT(LanguageParser::buildAcceptLanguageHeader(al) ==
            "en-US-mn,en-US-ca;q=0.500");
    }

    // Test handling of Content-Languages whitespace and comments
    {
        ContentLanguageList cl = LanguageParser::parseContentLanguageHeader(
            "    en-US-mn (should not appear)  ,"
            "(and)en-US-ca   (if you can imagine) (!!!)  ");
        PEGASUS_TEST_ASSERT(cl.size() == 2);
        PEGASUS_TEST_ASSERT(cl.getLanguageTag(0).toString() == "en-US-mn");
        PEGASUS_TEST_ASSERT(cl.getLanguageTag(1).toString() == "en-US-ca");
        PEGASUS_TEST_ASSERT(LanguageParser::buildContentLanguageHeader(cl) ==
            "en-US-mn,en-US-ca");
    }

    // Test handling of non-ASCII characters in Content-Languages comment
    {
        String headerValue = "en-US (will add non-ASCII character in comment)";
        headerValue[14] = 132;
        ContentLanguageList cl =
            LanguageParser::parseContentLanguageHeader(headerValue);
        PEGASUS_TEST_ASSERT(cl.size() == 1);
        PEGASUS_TEST_ASSERT(cl.getLanguageTag(0).toString() == "en-US");
        PEGASUS_TEST_ASSERT(
            LanguageParser::buildContentLanguageHeader(cl) == "en-US");
    }

    // Test handling of non-ASCII characters in Content-Languages value
    {
        String headerValue = "en-US-ca (will add non-ASCII character in tag)";
        headerValue[4] = 132;
        Boolean gotException = false;
        try
        {
            ContentLanguageList cl =
                LanguageParser::parseContentLanguageHeader(headerValue);
        }
        catch (const InvalidContentLanguageHeader&)
        {
            gotException = true;
        }
        PEGASUS_TEST_ASSERT(gotException);
    }

    // Test handling of trailing escape character in Content-Languages value
    {
        String headerValue = "en-US-ca (trailing escape character) \\";
        Boolean gotException = false;
        try
        {
            ContentLanguageList cl =
                LanguageParser::parseContentLanguageHeader(headerValue);
        }
        catch (const InvalidContentLanguageHeader&)
        {
            gotException = true;
        }
        PEGASUS_TEST_ASSERT(gotException);
    }
}


void testLanguageTag()
{
    // Test string value constructor and accessor methods

    {
        String tag1("en-US-mn");

        PEGASUS_TEST_ASSERT(LanguageTag(tag1).toString() == "en-US-mn");
        PEGASUS_TEST_ASSERT(LanguageTag(tag1).getLanguage() == "en");
        PEGASUS_TEST_ASSERT(LanguageTag(tag1).getCountry() == "US");
        PEGASUS_TEST_ASSERT(LanguageTag(tag1).getVariant() == "mn");
    }

    {
        String tag1("en-US-123");

        PEGASUS_TEST_ASSERT(LanguageTag(tag1).toString() == "en-US-123");
        PEGASUS_TEST_ASSERT(LanguageTag(tag1).getLanguage() == "en");
        PEGASUS_TEST_ASSERT(LanguageTag(tag1).getCountry() == "US");
        PEGASUS_TEST_ASSERT(LanguageTag(tag1).getVariant() == "123");
    }

    {
        String tag1("eng-1a-C3P0");

        PEGASUS_TEST_ASSERT(LanguageTag(tag1).toString() == "eng-1a-C3P0");
        PEGASUS_TEST_ASSERT(LanguageTag(tag1).getLanguage() == "eng");
        PEGASUS_TEST_ASSERT(LanguageTag(tag1).getCountry() == "1a");
        PEGASUS_TEST_ASSERT(LanguageTag(tag1).getVariant() == "C3P0");
    }

    {
        String tag1("en-my-weird-dialect");

        PEGASUS_TEST_ASSERT(
            LanguageTag(tag1).toString() == "en-my-weird-dialect");
        PEGASUS_TEST_ASSERT(LanguageTag(tag1).getLanguage() == "en");
        PEGASUS_TEST_ASSERT(LanguageTag(tag1).getCountry() == "my");
        PEGASUS_TEST_ASSERT(
            LanguageTag(tag1).getVariant() == "weird-dialect");
    }

    {
        String tag1("en-quite-a-weird-dialect");

        PEGASUS_TEST_ASSERT(
            LanguageTag(tag1).toString() == "en-quite-a-weird-dialect");
        PEGASUS_TEST_ASSERT(LanguageTag(tag1).getLanguage() == "en");
        PEGASUS_TEST_ASSERT(LanguageTag(tag1).getCountry() == "");
        PEGASUS_TEST_ASSERT(
            LanguageTag(tag1).getVariant() == "quite-a-weird-dialect");
    }

    {
        String tag1("x-pig-latin");

        PEGASUS_TEST_ASSERT(LanguageTag(tag1).toString() == "x-pig-latin");
        PEGASUS_TEST_ASSERT(LanguageTag(tag1).getLanguage() == "");
        PEGASUS_TEST_ASSERT(LanguageTag(tag1).getCountry() == "");
        PEGASUS_TEST_ASSERT(LanguageTag(tag1).getVariant() == "");
    }

    {
        String tag1("i-latin-for-pigs");

        PEGASUS_TEST_ASSERT(
            LanguageTag(tag1).toString() == "i-latin-for-pigs");
        PEGASUS_TEST_ASSERT(LanguageTag(tag1).getLanguage() == "");
        PEGASUS_TEST_ASSERT(LanguageTag(tag1).getCountry() == "");
        PEGASUS_TEST_ASSERT(LanguageTag(tag1).getVariant() == "");
    }

        // Test copy constructor, assignment operator, and equality operator

    {
        LanguageTag lt1("en-US-ca");
        LanguageTag lt2(lt1);
        LanguageTag lt3 = lt2;
        LanguageTag lt4("EN-us-Ca");
        LanguageTag lt5("en-US-mn");
        LanguageTag lt6;
        LanguageTag lt7 = lt6;
        LanguageTag lt8 = lt1;
        lt7 = lt1;
        lt7 = lt7;
        lt8 = lt6;
        lt8 = lt1;

        PEGASUS_TEST_ASSERT(lt1 == lt2);
        PEGASUS_TEST_ASSERT(lt1 == lt3);
        PEGASUS_TEST_ASSERT(lt2 == lt3);
        PEGASUS_TEST_ASSERT(lt1 == lt4);
        PEGASUS_TEST_ASSERT(lt1 != lt5);
        PEGASUS_TEST_ASSERT(lt3 != lt5);
        PEGASUS_TEST_ASSERT(lt1 == lt8);
        PEGASUS_TEST_ASSERT(lt7 == lt8);
    }

    // Test invalid language tag:  Empty string
    {
        Boolean gotException = false;

        try
        {
            LanguageTag lt("");
        }
        catch (Exception&)
        {
            gotException = true;
        }

        PEGASUS_TEST_ASSERT(gotException);
    }

    // Test invalid language tag:  Digit in primary subtag
    {
        Boolean gotException = false;

        try
        {
            LanguageTag lt("e4-US-ca");
        }
        catch (Exception&)
        {
            gotException = true;
        }

        PEGASUS_TEST_ASSERT(gotException);
    }

    // Test invalid language tag:  Primary subtag too short
    {
        Boolean gotException = false;

        try
        {
            LanguageTag lt("e-US-ca");
        }
        catch (Exception&)
        {
            gotException = true;
        }

        PEGASUS_TEST_ASSERT(gotException);
    }

    // Test invalid language tag:  Primary subtag too long
    {
        Boolean gotException = false;

        try
        {
            LanguageTag lt("engl-US-ca");
        }
        catch (Exception&)
        {
            gotException = true;
        }

        PEGASUS_TEST_ASSERT(gotException);
    }

    // Test invalid language tag:  Primary subtag too long
    {
        Boolean gotException = false;

        try
        {
            LanguageTag lt("englishman-US-ca");
        }
        catch (Exception&)
        {
            gotException = true;
        }

        PEGASUS_TEST_ASSERT(gotException);
    }

    // Test invalid language tag:  Single character second subtag
    {
        Boolean gotException = false;

        try
        {
            LanguageTag lt("en-U-ca");
        }
        catch (Exception&)
        {
            gotException = true;
        }

        PEGASUS_TEST_ASSERT(gotException);
    }

    // Test invalid language tag:  Second subtag too long
    {
        Boolean gotException = false;

        try
        {
            LanguageTag lt("en-UnitedStates-ca");
        }
        catch (Exception&)
        {
            gotException = true;
        }

        PEGASUS_TEST_ASSERT(gotException);
    }

    // Test invalid language tag:  Third subtag too long
    {
        Boolean gotException = false;

        try
        {
            LanguageTag lt("en-US-california");
        }
        catch (Exception&)
        {
            gotException = true;
        }

        PEGASUS_TEST_ASSERT(gotException);
    }

    // Test invalid language tag:  Empty subtag
    {
        Boolean gotException = false;

        try
        {
            LanguageTag lt("en--ca");
        }
        catch (Exception&)
        {
            gotException = true;
        }

        PEGASUS_TEST_ASSERT(gotException);
    }

    // Test invalid language tag:  Non-ASCII primary tag
    {
        Boolean gotException = false;

        try
        {
            String tag = "en-US-ca";
            tag[1] = 132;
            LanguageTag lt(tag);
        }
        catch (Exception&)
        {
            gotException = true;
        }

        PEGASUS_TEST_ASSERT(gotException);
    }

    // Test invalid language tag:  Non-ASCII subtag
    {
        Boolean gotException = false;

        try
        {
            String tag = "en-US-ca";
            tag[4] = 132;
            LanguageTag lt(tag);
        }
        catch (Exception&)
        {
            gotException = true;
        }

        PEGASUS_TEST_ASSERT(gotException);
    }

    // Test uninitialized object:  getLanguage() method
    {
        Boolean gotException = false;
        LanguageTag lt;

        try
        {
            String language = lt.getLanguage();
        }
        catch (UninitializedObjectException&)
        {
            gotException = true;
        }

        PEGASUS_TEST_ASSERT(gotException);
    }

    // Test uninitialized object:  getCountry() method
    {
        Boolean gotException = false;
        LanguageTag lt;

        try
        {
            String country = lt.getCountry();
        }
        catch (UninitializedObjectException&)
        {
            gotException = true;
        }

        PEGASUS_TEST_ASSERT(gotException);
    }

    // Test uninitialized object:  getVariant() method
    {
        Boolean gotException = false;
        LanguageTag lt;

        try
        {
            String variant = lt.getVariant();
        }
        catch (UninitializedObjectException&)
        {
            gotException = true;
        }

        PEGASUS_TEST_ASSERT(gotException);
    }

    // Test uninitialized object:  toString() method
    {
        Boolean gotException = false;
        LanguageTag lt;

        try
        {
            String languageString = lt.toString();
        }
        catch (UninitializedObjectException&)
        {
            gotException = true;
        }

        PEGASUS_TEST_ASSERT(gotException);
    }

    // Test uninitialized object:  equality operator
    {
        Boolean gotException = false;
        LanguageTag lt1;
        LanguageTag lt2("en-US-ca");

        try
        {
            (lt1 == lt2);
        }
        catch (UninitializedObjectException&)
        {
            gotException = true;
        }

        PEGASUS_TEST_ASSERT(gotException);
    }

    // Test uninitialized object:  assignment
    {
        LanguageTag lt1;
        LanguageTag lt2("en-US-ca");

        lt1 = lt2;

        PEGASUS_TEST_ASSERT(lt1.toString() == "en-US-ca");
    }

    // Test uninitialized object:  unassignment
    {
        Boolean gotException = false;
        LanguageTag lt1("en-US-ca");
        LanguageTag lt2;

        lt1 = lt2;

        try
        {
            String languageTag = lt1.toString();
        }
        catch (UninitializedObjectException&)
        {
            gotException = true;
        }

        PEGASUS_TEST_ASSERT(gotException);
    }
}

#ifdef PEGASUS_HAS_ICU

void testSUCCESSMessage(
    MessageLoaderParms& mlp,
    const char* expectedLanguage,
    const char* expectedText,
    const char* expectedNum,
    const char* testSeq,
    Uint32 testNum)
{
    const Uint32 MESSAGE_SIZE = 100;
    char messageText[MESSAGE_SIZE];

    sprintf(messageText,
        ((strcmp(expectedLanguage, "ROOT") == 0) ?
            "CIM_ERR_SUCCESS: SUCCESSFUL %s %s, NUMBER = %s" :
            "CIM_ERR_SUCCESS: SUCCESSFUL %s %s, number = %s"),
            expectedLanguage,
            ((expectedText == NULL) ? "testMessageLoaderCL" : expectedText),
            ((expectedNum == NULL) ? "1" : expectedNum));
    if (verbose)
    {
        cout << "TestID: " << testSeq <<  testNum << endl;
        cout << "Expected: " << messageText << endl;
        cout << "Message:  " << MessageLoader::getMessage(mlp) << endl;
        cout << "CL: "
             << LanguageParser::buildContentLanguageHeader(
                 mlp.contentlanguages) << endl;
    }
    PEGASUS_TEST_ASSERT(MessageLoader::getMessage(mlp) == messageText);
    PEGASUS_TEST_ASSERT(LanguageParser::buildContentLanguageHeader(
        mlp.contentlanguages) ==
            ((strcmp(expectedLanguage,"ROOT") == 0) ? "" : expectedLanguage));
}

void testFAILEDMessage(MessageLoaderParms &mlp, const char * expectedLanguage,
    char * testSeq, Uint32 testNum)
{
    const char *messageText = "CIM_ERR_FAILED: A general error occurred that "
       "is not covered by a more specific error code";

    if (verbose)
    {
        cout << "TestID: " << testSeq <<  testNum << endl;
        cout << "Expected: " << messageText << endl;
        cout << "Message:  " << MessageLoader::getMessage(mlp) << endl;
        cout << "CL: "
             << LanguageParser::buildContentLanguageHeader(
                 mlp.contentlanguages) << endl;
    }
    PEGASUS_TEST_ASSERT(MessageLoader::getMessage(mlp) == messageText);
    PEGASUS_TEST_ASSERT(LanguageParser::buildContentLanguageHeader(
        mlp.contentlanguages) ==
            ((strcmp(expectedLanguage,"ROOT") == 0) ? "" : expectedLanguage));
}


#endif


void testAcceptLanguageList()
{
    {
        AcceptLanguageList al = LanguageParser::parseAcceptLanguageHeader(
            "en-US-mn;q=.9,fr-FR;q=.1,en, fr;q=.2,la-SP-bal;q=.7,*;q=.01");

        PEGASUS_TEST_ASSERT(al.size() == 6);

        PEGASUS_TEST_ASSERT(al.getLanguageTag(0).toString() == "en");
        PEGASUS_TEST_ASSERT(al.getLanguageTag(1).toString() == "en-US-mn");
        PEGASUS_TEST_ASSERT(al.getLanguageTag(2).toString() == "la-SP-bal");
        PEGASUS_TEST_ASSERT(al.getLanguageTag(3).toString() == "fr");
        PEGASUS_TEST_ASSERT(al.getLanguageTag(4).toString() == "fr-FR");
        PEGASUS_TEST_ASSERT(al.getLanguageTag(5).toString() == "*");

        PEGASUS_TEST_ASSERT(LanguageParser::buildAcceptLanguageHeader(al) ==
            "en,en-US-mn;q=0.900,la-SP-bal;q=0.700,fr;q=0.200,fr-FR;q=0.100,"
                "*;q=0.010");

        // Test insert() method

        al.insert(LanguageTag("en-XX-xx"), 1.0);
        PEGASUS_TEST_ASSERT(al.size() == 7);
        PEGASUS_TEST_ASSERT(al.find(LanguageTag("en-XX-xx")) != PEG_NOT_FOUND);
        PEGASUS_TEST_ASSERT(
            al.getQualityValue(al.find(LanguageTag("en-XX-xx"))) == 1.0);

        // Test remove() method

        Uint32 index = al.find(LanguageTag("en-XX-xx"));
        al.remove(index);
        PEGASUS_TEST_ASSERT(al.find(LanguageTag("en-XX-xx")) == PEG_NOT_FOUND);
        PEGASUS_TEST_ASSERT(al.size() == 6);

        // Test assignment operator and equality operator

        AcceptLanguageList al1;

        al1 = al;
        PEGASUS_TEST_ASSERT(al1 == al);

        al1 = al1;
        PEGASUS_TEST_ASSERT(al1 == al);

        al1.remove(0);
        PEGASUS_TEST_ASSERT(al1 != al);
    }

    // Test inequality operator
    {
        AcceptLanguageList list1;
        AcceptLanguageList list2;

        list1.insert(LanguageTag("en-US"), 1);
        list1.insert(LanguageTag("fr"), Real32(0.8));
        list2 = list1;
        PEGASUS_TEST_ASSERT(list1 == list2);

        list2.remove(1);
        PEGASUS_TEST_ASSERT(list1 != list2);

        list2.insert(LanguageTag("fr"), Real32(0.7));
        PEGASUS_TEST_ASSERT(list1 != list2);

        list2.remove(1);
        list2.insert(LanguageTag("de"), Real32(0.8));
        PEGASUS_TEST_ASSERT(list1 != list2);
    }

    // Test clear() method

    {
        AcceptLanguageList al = LanguageParser::parseAcceptLanguageHeader(
            "en-US-mn;q=.9,fr-FR;q=.1,en, fr;q=.2,la-SP-bal;q=.7,*;q=.01");
        PEGASUS_TEST_ASSERT(al.size() == 6);
        al.clear();
        PEGASUS_TEST_ASSERT(al.size() == 0);
    }

    // Test sorting of quality values
    {
        AcceptLanguageList al = LanguageParser::parseAcceptLanguageHeader(
            "de;q=0.000,it;q=0.50,*;q=0.25,en-US-ca;q=1.00");

        PEGASUS_TEST_ASSERT(al.getLanguageTag(0).toString() == "en-US-ca");
        PEGASUS_TEST_ASSERT(al.getQualityValue(0) == 1.0);
        PEGASUS_TEST_ASSERT(al.getLanguageTag(1).toString() == "it");
        PEGASUS_TEST_ASSERT(al.getQualityValue(1) == 0.5);
        PEGASUS_TEST_ASSERT(al.getLanguageTag(2).toString() == "*");
        PEGASUS_TEST_ASSERT(al.getQualityValue(2) == 0.25);
        PEGASUS_TEST_ASSERT(al.getLanguageTag(3).toString() == "de");
        PEGASUS_TEST_ASSERT(al.getQualityValue(3) == 0.0);

        PEGASUS_TEST_ASSERT(LanguageParser::buildAcceptLanguageHeader(al) ==
            "en-US-ca,it;q=0.500,*;q=0.250,de;q=0.000");
    }

    // Test invalid quality value syntax:  Missing "q"
    {
        Boolean gotException = false;

        try
        {
            AcceptLanguageList al =
                LanguageParser::parseAcceptLanguageHeader("en-US-ca;");
        }
        catch (InvalidAcceptLanguageHeader&)
        {
            gotException = true;
        }

        PEGASUS_TEST_ASSERT(gotException);
    }

    // Test invalid quality value syntax:  Missing "="
    {
        Boolean gotException = false;

        try
        {
            AcceptLanguageList al =
                LanguageParser::parseAcceptLanguageHeader("en-US-ca;q");
        }
        catch (InvalidAcceptLanguageHeader&)
        {
            gotException = true;
        }

        PEGASUS_TEST_ASSERT(gotException);
    }

    // Test invalid quality value syntax:  Unexpected character at "q"
    {
        Boolean gotException = false;

        try
        {
            AcceptLanguageList al =
                LanguageParser::parseAcceptLanguageHeader("en-US-ca;r=.9");
        }
        catch (InvalidAcceptLanguageHeader&)
        {
            gotException = true;
        }

        PEGASUS_TEST_ASSERT(gotException);
    }

    // Test invalid quality value syntax:  Unexpected character at "="
    {
        Boolean gotException = false;

        try
        {
            AcceptLanguageList al =
                LanguageParser::parseAcceptLanguageHeader(
                    "en-US-ca;q+0.1");
        }
        catch (InvalidAcceptLanguageHeader&)
        {
            gotException = true;
        }

        PEGASUS_TEST_ASSERT(gotException);
    }

    // Test invalid quality value syntax:  Extra semicolon
    {
        Boolean gotException = false;

        try
        {
            AcceptLanguageList al =
                LanguageParser::parseAcceptLanguageHeader(
                    "en-US-ca;;q=0.1");
        }
        catch (InvalidAcceptLanguageHeader&)
        {
            gotException = true;
        }

        PEGASUS_TEST_ASSERT(gotException);
    }

    // Test invalid quality value syntax:  Negative quality value
    {
        Boolean gotException = false;

        try
        {
            AcceptLanguageList al =
                LanguageParser::parseAcceptLanguageHeader(
                    "en-US-ca;q=-0.1");
        }
        catch (InvalidAcceptLanguageHeader&)
        {
            gotException = true;
        }

        PEGASUS_TEST_ASSERT(gotException);
    }

    // Test invalid quality value syntax:  Quality value too large
    {
        Boolean gotException = false;

        try
        {
            AcceptLanguageList al =
                LanguageParser::parseAcceptLanguageHeader(
                    "en-US-ca;q=1.1");
        }
        catch (InvalidAcceptLanguageHeader&)
        {
            gotException = true;
        }

        PEGASUS_TEST_ASSERT(gotException);
    }

    // Test invalid quality value syntax:  Invalid trailing characters
    {
        Boolean gotException = false;

        try
        {
            AcceptLanguageList al =
                LanguageParser::parseAcceptLanguageHeader(
                    "en-US-ca;q=0.1a");
        }
        catch (InvalidAcceptLanguageHeader&)
        {
            gotException = true;
        }

        PEGASUS_TEST_ASSERT(gotException);
    }

    // Test invalid quality value syntax:  Quality value too long
    {
        Boolean gotException = false;

        try
        {
            AcceptLanguageList al =
                LanguageParser::parseAcceptLanguageHeader(
                    "en-US-ca;q=0.1110");
        }
        catch (InvalidAcceptLanguageHeader&)
        {
            gotException = true;
        }

        PEGASUS_TEST_ASSERT(gotException);
    }

    // Test invalid comment syntax:  Missing closing parenthesis
    {
        Boolean gotException = false;

        try
        {
            AcceptLanguageList al =
                LanguageParser::parseAcceptLanguageHeader(
                    "en-US-ca(;q=0.1111");
        }
        catch (InvalidAcceptLanguageHeader&)
        {
            gotException = true;
        }

        PEGASUS_TEST_ASSERT(gotException);
    }

    // Test valid comment syntax
    {
        AcceptLanguageList al1 = LanguageParser::parseAcceptLanguageHeader(
            "en(english)-(\\(USA\\))US-\\c\\a;q(quality)=0.1(not much)");
        AcceptLanguageList al2 = LanguageParser::parseAcceptLanguageHeader(
            "en-US-ca;q=0.1");
        PEGASUS_TEST_ASSERT(al1 == al2);
    }

    // Test valid comment and whitespace syntax
    {
        AcceptLanguageList al1 = LanguageParser::parseAcceptLanguageHeader(
            "en (english)-(\\( USA \\))US-\\c \\a   ;q(quality) =0.1  "
                "(not much) ");
        AcceptLanguageList al2 = LanguageParser::parseAcceptLanguageHeader(
            "en-US-ca;q=0.1");
        PEGASUS_TEST_ASSERT(al1 == al2);
    }

    // Test invalid whitespace syntax
    {
        Boolean gotException = false;

        try
        {
            AcceptLanguageList al =
                LanguageParser::parseAcceptLanguageHeader(
                    "en-US-ca\\ ;q=0.1");
        }
        catch (InvalidAcceptLanguageHeader&)
        {
            gotException = true;
        }

        PEGASUS_TEST_ASSERT(gotException);
    }

    // Test invalid language tag:  Trailing '-'
    {
        Boolean gotException = false;

        try
        {
            AcceptLanguageList al =
                LanguageParser::parseAcceptLanguageHeader(
                    "en-US-ca-;q=0.1");
        }
        catch (InvalidAcceptLanguageHeader&)
        {
            gotException = true;
        }

        PEGASUS_TEST_ASSERT(gotException);
    }

    // Test invalid Accept-Language value:  Empty string
    {
        Boolean gotException = false;

        try
        {
            AcceptLanguageList al =
                LanguageParser::parseAcceptLanguageHeader("");
        }
        catch (InvalidAcceptLanguageHeader&)
        {
            gotException = true;
        }

        PEGASUS_TEST_ASSERT(gotException);
    }

    // Test invalid Accept-Language value:  Comment and whitespace only
    {
        Boolean gotException = false;

        try
        {
            AcceptLanguageList al =
                LanguageParser::parseAcceptLanguageHeader(
                    " (comment only)");
        }
        catch (InvalidAcceptLanguageHeader&)
        {
            gotException = true;
        }

        PEGASUS_TEST_ASSERT(gotException);
    }
}


void testContentLanguageList()
{
    {
        ContentLanguageList cl = LanguageParser::parseContentLanguageHeader(
            "en-US-mn,fr-FR,en, fr(oh you french), la-SP-bal");

        for (Uint32 index = 0; index < cl.size(); index++)
        {
            LanguageTag lt = cl.getLanguageTag(index);

            if (index == 3)
            {
                PEGASUS_TEST_ASSERT(String::equal(lt.toString(), "fr"));
            }
        }

        PEGASUS_TEST_ASSERT(cl.size() == 5);

        // Add LanguageTag

        cl.append(LanguageTag("en-XX-xx"));
        PEGASUS_TEST_ASSERT(cl.size() == 6);
        PEGASUS_TEST_ASSERT(cl.find(LanguageTag("en-XX-xx")) != PEG_NOT_FOUND);

        // Remove LanguageTag

        Uint32 index = cl.find(LanguageTag("en-XX-xx"));
        cl.remove(index);
        PEGASUS_TEST_ASSERT(cl.find(LanguageTag("en-XX-xx")) == PEG_NOT_FOUND);
        PEGASUS_TEST_ASSERT(cl.size() == 5);

        // Test assignment operator and equality operator

        ContentLanguageList cl1;
        cl1 = cl;
        PEGASUS_TEST_ASSERT(cl1 == cl);

        cl1 = cl1;
        PEGASUS_TEST_ASSERT(cl1 == cl);

        cl1.remove(0);
        PEGASUS_TEST_ASSERT(cl1 != cl);

        // Test clear() method

        cl1.clear();
        PEGASUS_TEST_ASSERT(cl1.size() == 0);
    }

    // Test invalid Content-Language value:  Invalid character
    {
        Boolean gotException = false;

        try
        {
            ContentLanguageList cl =
                LanguageParser::parseContentLanguageHeader("en-4%5US-mn");
        }
        catch(InvalidContentLanguageHeader&)
        {
            gotException = true;
        }

        PEGASUS_TEST_ASSERT(gotException);
    }

    // Test invalid Content-Language value:  Empty string
    {
        Boolean gotException = false;

        try
        {
            ContentLanguageList cl =
                LanguageParser::parseContentLanguageHeader("");
        }
        catch (InvalidContentLanguageHeader&)
        {
            gotException = true;
        }

        PEGASUS_TEST_ASSERT(gotException);
    }

    // Test invalid Content-Language value:  Comment and whitespace only
    {
        Boolean gotException = false;

        try
        {
            ContentLanguageList cl =
                LanguageParser::parseContentLanguageHeader(
                    " (comment only)");
        }
        catch (InvalidContentLanguageHeader&)
        {
            gotException = true;
        }

        PEGASUS_TEST_ASSERT(gotException);
    }

    // Test invalid Content-Language value:  "*" language tag
    {
        Boolean gotException = false;

        try
        {
            ContentLanguageList cl =
                LanguageParser::parseContentLanguageHeader("en, *, es");
        }
        catch (InvalidContentLanguageHeader&)
        {
            gotException = true;
        }

        PEGASUS_TEST_ASSERT(gotException);
    }
}

void testMessageLoader()
{
    MessageLoaderParms mlp(
        "CIMStatusCode.CIM_ERR_SUCCESS",
        "Default CIMStatusCode, $0 $1",
        "rab oof is foo bar backwards",
        64000);

    mlp.msg_src_path = "test/pegasusTest";
    mlp.acceptlanguages = LanguageParser::parseAcceptLanguageHeader("en-US");

#ifdef PEGASUS_HAS_ICU

    PEGASUS_TEST_ASSERT(MessageLoader::getMessage(mlp) ==
        "CIM_ERR_SUCCESS: SUCCESSFUL en-us rab oof is foo bar backwards, "
            "number = 64,000");

    // test for return content languages

    PEGASUS_TEST_ASSERT(LanguageParser::buildContentLanguageHeader(
        mlp.contentlanguages) == "en-US");

#else

    PEGASUS_TEST_ASSERT(MessageLoader::getMessage(mlp) ==
        "Default CIMStatusCode, rab oof is foo bar backwards 64000");

#endif

    //
    // Should load default resource bundle.
    //

    mlp.acceptlanguages.clear();
    mlp.acceptlanguages.insert(LanguageTag("en-US-mn"), 1.0);

#ifdef PEGASUS_HAS_ICU

    testSUCCESSMessage(mlp, defaultResourceBundle,
        "rab oof is foo bar backwards",
        ((strcmp(defaultResourceBundle,"de") == 0) ?
            "64.000" : "64,000"), "testML", 1);
#else

    PEGASUS_TEST_ASSERT(MessageLoader::getMessage(mlp) ==
        "Default CIMStatusCode, rab oof is foo bar backwards 64000");

#endif

    //
    // No exact matches found, return default resource bundle
    //

    MessageLoaderParms mlp1("CIMStatusCode.CIM_ERR_SUCCESS",
                            "Default CIMStatusCode, $0 $1",
                            "rab oof is foo bar backwards","fr");

    mlp1.msg_src_path = "test/pegasusTest";
    mlp1.acceptlanguages.clear();
    mlp1.acceptlanguages.insert(LanguageTag("fr-FR"), 1.0);
    mlp1.acceptlanguages.insert(LanguageTag("bl-ow"), 1.0);

#ifdef PEGASUS_HAS_ICU

    testSUCCESSMessage(mlp, defaultResourceBundle,
        "rab oof is foo bar backwards",
        ((strcmp(defaultResourceBundle,"de") == 0) ?
            "64.000" : "64,000"), "testML", 2);

#else

    PEGASUS_TEST_ASSERT(MessageLoader::getMessage(mlp) ==
        "Default CIMStatusCode, rab oof is foo bar backwards 64000");

#endif

    //
    // use gobal default message switch for messageloading
    //

    MessageLoader::_useDefaultMsg = true;

    mlp.acceptlanguages.clear();
    mlp.acceptlanguages.insert(LanguageTag("en-US"), 1.0);

    PEGASUS_TEST_ASSERT(MessageLoader::getMessage(mlp) ==
        "Default CIMStatusCode, rab oof is foo bar backwards 64000");

    //
    // set static AcceptLanguageList in message loader
    //

    MessageLoader::_useDefaultMsg = false;
    MessageLoader::_acceptlanguages.insert(LanguageTag("st-at-ic"), 1.0);

    MessageLoaderParms mlp_static(
        "CIMStatusCode.CIM_ERR_SUCCESS","Default CIMStatusCode, $0",
        "rab oof is foo bar backwards static");
    mlp_static.msg_src_path = "test/pegasusTest";

#ifdef PEGASUS_HAS_ICU

    PEGASUS_TEST_ASSERT(MessageLoader::getMessage(mlp_static) ==
        "CIM_ERR_SUCCESS: SUCCESSFUL st_at_ic rab oof is foo bar backwards "
            "static");

#else

    PEGASUS_TEST_ASSERT(MessageLoader::getMessage(mlp_static) ==
        "Default CIMStatusCode, rab oof is foo bar backwards static");

#endif
}

#ifdef PEGASUS_HAS_ICU

ThreadReturnType PEGASUS_THREAD_CDECL alTestThread(void* parm)
{
    AcceptLanguageList alThread;
    alThread.clear();
    alThread.insert(LanguageTag("el"), 1.0);
    Thread::setLanguages(alThread);

    MessageLoaderParms mlp(
        "CIMStatusCode.CIM_ERR_SUCCESS",
            "Default CIM_ERR_SUCCESS Message $0 $1",
            "testMessageLoaderCL", 1);

    mlp.msg_src_path = "test/pegasusTest";
    mlp.useThreadLocale = true;

    // Test threadTest1 - Use Thread Locale
    mlp.acceptlanguages.clear();
    testSUCCESSMessage(mlp, "el", NULL, NULL, "threadTest", 1);

    // Test threadTest2 - Passed accept language list has precedence.
    mlp.acceptlanguages.clear();
    mlp.acceptlanguages.insert(LanguageTag("en-us"), 1.0);
    mlp.acceptlanguages.insert(LanguageTag("fr"), 0.7);
    testSUCCESSMessage(mlp, "en-us", NULL, NULL, "threadTest", 2);

    // Test threadTest3 - Process locale has precedence.
    mlp.useProcessLocale = true;
    mlp.acceptlanguages.clear();
    testSUCCESSMessage(mlp, defaultResourceBundle, NULL, NULL, "threadTest", 3);


    threadSync.signal();
    return ThreadReturnType(0);
}

void testICUMessageLoaderOrdering()
{
    String localeStr;

    MessageLoaderParms mlp(
        "CIMStatusCode.CIM_ERR_SUCCESS",
            "Default CIM_ERR_SUCCESS Message $0 $1",
            "testMessageLoaderCL", 1);
    mlp.msg_src_path = "test/pegasusTest";

    // Note: The following message bundles are available:
    //       pegasusTest_de.res
    //       pegasusTest_en.res
    //       pegasusTest_en_US.res
    //       pegasusTest_fr.res
    //       pegasusTest_fu_nk.res
    //       pegasusTest_st_AT_IC.res
    //       pegasusTest_root.res

    // Test A: Test basic functionality.

    // Test A1 - Exact Match - First Tag
    mlp.acceptlanguages.clear();
    mlp.acceptlanguages.insert(LanguageTag("en-us"), 1.0);
    mlp.acceptlanguages.insert(LanguageTag("fr"), 0.7);
    testSUCCESSMessage(mlp, "en-us", NULL, NULL, "A" , 1);

    // Test A2 - Exact Match - Second Tag
    mlp.acceptlanguages.clear();
    mlp.acceptlanguages.insert(LanguageTag("en-gb"), 1.0);
    mlp.acceptlanguages.insert(LanguageTag("fr"), 0.7);
    testSUCCESSMessage(mlp, "fr", NULL, NULL, "A", 2);

    // Test A3 - Exact Match - Fourth Tag
    mlp.acceptlanguages.clear();
    mlp.acceptlanguages.insert(LanguageTag("da"), 1.0);
    mlp.acceptlanguages.insert(LanguageTag("fr-ca"), 0.9);
    mlp.acceptlanguages.insert(LanguageTag("en-gb"), 0.8);
    mlp.acceptlanguages.insert(LanguageTag("en"), 0.7);
    testSUCCESSMessage(mlp, "en", NULL, NULL, "A", 3);

    // Test A4 - No Exact Match - Fallback Match - First Tag
    mlp.acceptlanguages.clear();
    mlp.acceptlanguages.insert(LanguageTag("en-gb"), 1.0);
    mlp.acceptlanguages.insert(LanguageTag("fr-ca"), 0.7);
    testSUCCESSMessage(mlp, defaultResourceBundle, NULL, NULL, "A", 4);

    // Test A5 - No Exact Match - Fallback Match - Second Tag
    mlp.acceptlanguages.clear();
    mlp.acceptlanguages.insert(LanguageTag("pt-br"), 1.0);
    mlp.acceptlanguages.insert(LanguageTag("fr-ca"), 0.7);
    testSUCCESSMessage(mlp, defaultResourceBundle, NULL, NULL, "A", 5);

    // Test A6 - No Exact Match - No Fallback Match
    mlp.acceptlanguages.clear();
    mlp.acceptlanguages.insert(LanguageTag("pt-br"), 1.0);
    mlp.acceptlanguages.insert(LanguageTag("zh-cn"), 0.7);
    testSUCCESSMessage(mlp, defaultResourceBundle, NULL, NULL, "A", 6);

    // Test A7 - Invalid Language Tag
    mlp.acceptlanguages.clear();
    mlp.acceptlanguages.insert(LanguageTag("*"), 1.0);
    testSUCCESSMessage(mlp, defaultResourceBundle, NULL, NULL, "A", 7);

    // Test A8 - Single Accept Language - Exact Match
    mlp.acceptlanguages.clear();
    mlp.acceptlanguages.insert(LanguageTag("en-us"), 1.0);
    testSUCCESSMessage(mlp, "en-us", NULL, NULL, "A", 8);

    // Test A9 - Single Accept Language - Fallback Match
    mlp.acceptlanguages.clear();
    mlp.acceptlanguages.insert(LanguageTag("en-gb"), 1.0);
    testSUCCESSMessage(mlp, defaultResourceBundle, NULL, NULL, "A", 9);

    // Test A10 - Single Accept Language - No Match
    mlp.acceptlanguages.clear();
    mlp.acceptlanguages.insert(LanguageTag("pt-br"), 1.0);
    testSUCCESSMessage(mlp, defaultResourceBundle, NULL, NULL, "A", 10);

    // Test B: Verify that the accept languages
    // defined in MessageLoader::_acceptlanguages
    // take precedence over the languages passed
    // to getMessage.
    MessageLoader::_acceptlanguages.clear();
    MessageLoader::_acceptlanguages.insert(LanguageTag("es-mx"), 1.0);

    mlp.acceptlanguages.clear();
    mlp.acceptlanguages.insert(LanguageTag("en-us"), 1.0);
    mlp.acceptlanguages.insert(LanguageTag("fr"), 0.7);
    testSUCCESSMessage(mlp, "es-mx", NULL, NULL, "B", 1);

    MessageLoader::_acceptlanguages.clear();

    // Test C: Verify that setting
    // MessageLoader::_useProcessLocale to true
    // overrides the options passed to getMessage.
    MessageLoader::_useProcessLocale = true;

    mlp.acceptlanguages.clear();
    mlp.acceptlanguages.insert(LanguageTag("en-us"), 1.0);
    mlp.acceptlanguages.insert(LanguageTag("fr"), 0.7);
    testSUCCESSMessage(mlp, defaultResourceBundle, NULL, NULL, "C", 1);

    MessageLoader::_useProcessLocale = false;

    // Test D: If no accept languages are defined and
    // useProtocolLocale is true, then the default
    // locale should be used.
    mlp.acceptlanguages.clear();
    mlp.useProcessLocale = true;
    testSUCCESSMessage(mlp, defaultResourceBundle, NULL, NULL, "D", 1);

    mlp.acceptlanguages.clear();
    mlp.useProcessLocale = false;
    testSUCCESSMessage(mlp, defaultResourceBundle, NULL, NULL, "D", 2);

    // This value is ignored if accept languages
    // are defined.
    mlp.acceptlanguages.clear();
    mlp.acceptlanguages.insert(LanguageTag("en-us"), 1.0);
    mlp.acceptlanguages.insert(LanguageTag("fr"), 0.7);
    mlp.useProcessLocale = true;
    testSUCCESSMessage(mlp, "en-us", NULL, NULL, "D", 3);

    // By default, useProcessLocale is false.
    mlp.useProcessLocale = false;


    // Test threadTest: If no accept languages are defined and
    // useThreadLocale is true, then the thread accept
    // languages should be used.

    struct timeval deallocateWait = { 0, 1 };
    ThreadPool threadPool(0, "AcceptLanguageTestPool", 0, 1, deallocateWait);
    threadPool.allocate_and_awaken(NULL, alTestThread);
    threadSync.wait();
}

#endif

//
// Tests the substitutions into the message
//
void testMessageLoaderSubs()
{
    MessageLoader::_acceptlanguages.clear();

    //
    // Test Uint64 support.  ICU does not support Uint64, so there
    // is special handling for it in MessageLoader.
    //

    //
    // Uint64 Substitution is the biggest positive to fit in int64_t.
    // This does not test the special code in MessageLoader.
    //
    MessageLoaderParms mlp1(
        "CIMStatusCode.CIM_ERR_SUCCESS",
        "Default CIMStatusCode, $0 $1",
        String("rab oof is foo bar backwards"),
        PEGASUS_UINT64_LITERAL(0x7fffffffffffffff));
    mlp1.msg_src_path = "test/pegasusTest";
    mlp1.acceptlanguages = LanguageParser::parseAcceptLanguageHeader("en-US");

#ifdef PEGASUS_HAS_ICU
    PEGASUS_TEST_ASSERT(MessageLoader::getMessage(mlp1) ==
        "CIM_ERR_SUCCESS: SUCCESSFUL en-us rab oof is foo bar backwards, "
            "number = 9,223,372,036,854,775,807");
#else
    PEGASUS_TEST_ASSERT(MessageLoader::getMessage(mlp1) ==
        "Default CIMStatusCode, rab oof is foo bar backwards "
            "9223372036854775807");
#endif

    //
    // Uint64 substitution is too big to fit int64_t.
    // Tests the special MessageLoader
    // code for this.  Expect the number to be unformatted.
    //
    mlp1.arg1 = PEGASUS_UINT64_LITERAL(0x8000000000000000);

#ifdef PEGASUS_HAS_ICU
    PEGASUS_TEST_ASSERT(MessageLoader::getMessage(mlp1) ==
        "CIM_ERR_SUCCESS: SUCCESSFUL en-us rab oof is foo bar backwards, "
            "number = 9223372036854775808");
#else
    PEGASUS_TEST_ASSERT(MessageLoader::getMessage(mlp1) ==
        "Default CIMStatusCode, rab oof is foo bar backwards "
            "9223372036854775808");
#endif

    //
    // Sint64 substitution - biggest negative.
    //
    mlp1.arg1 = PEGASUS_SINT64_LITERAL(0x8000000000000000);

#ifdef PEGASUS_HAS_ICU
    PEGASUS_TEST_ASSERT(MessageLoader::getMessage(mlp1) ==
        "CIM_ERR_SUCCESS: SUCCESSFUL en-us rab oof is foo bar backwards, "
            "number = -9,223,372,036,854,775,808");
#else
    PEGASUS_TEST_ASSERT(MessageLoader::getMessage(mlp1) ==
        "Default CIMStatusCode, rab oof is foo bar backwards "
            "-9223372036854775808" );
#endif

    //
    // Uint32 substitution - biggest possible
    //
    mlp1.arg1 = (Uint32)(0xffffffff);

#ifdef PEGASUS_HAS_ICU
    PEGASUS_TEST_ASSERT(MessageLoader::getMessage(mlp1) ==
        "CIM_ERR_SUCCESS: SUCCESSFUL en-us rab oof is foo bar backwards, "
            "number = 4,294,967,295");
#else
    PEGASUS_TEST_ASSERT(MessageLoader::getMessage(mlp1) ==
        "Default CIMStatusCode, rab oof is foo bar backwards 4294967295");
#endif

    //
    // Sint32 substitution - biggest negative
    //
    mlp1.arg1 = (Sint32)(0x80000000);

#ifdef PEGASUS_HAS_ICU
    PEGASUS_TEST_ASSERT(MessageLoader::getMessage(mlp1) ==
        "CIM_ERR_SUCCESS: SUCCESSFUL en-us rab oof is foo bar backwards, "
            "number = -2,147,483,648");
#else
    PEGASUS_TEST_ASSERT(MessageLoader::getMessage(mlp1) ==
        "Default CIMStatusCode, rab oof is foo bar backwards -2147483648");
#endif

    //
    // Real64 substitution
    //
    mlp1.arg1 = (Real64)-64000.125;

#ifdef PEGASUS_HAS_ICU
    PEGASUS_TEST_ASSERT(MessageLoader::getMessage(mlp1) ==
        "CIM_ERR_SUCCESS: SUCCESSFUL en-us rab oof is foo bar backwards, "
            "number = -64,000.125");
#else
    // Commenting out due to platform differences
    // The main purpose of this tests is ICU substitution.
    // cout << MessageLoader::getMessage(mlp1) << endl;
    // PEGASUS_TEST_ASSERT(MessageLoader::getMessage(mlp1) ==
    //     "Default CIMStatusCode, rab oof is foo bar backwards -64000.125");
#endif

    //
    // Boolean substitution = true
    //
    mlp1.arg1 = true;

#ifdef PEGASUS_HAS_ICU
    PEGASUS_TEST_ASSERT(MessageLoader::getMessage(mlp1) ==
        "CIM_ERR_SUCCESS: SUCCESSFUL en-us rab oof is foo bar backwards, "
            "number = true");
#else
    PEGASUS_TEST_ASSERT(MessageLoader::getMessage(mlp1) ==
        "Default CIMStatusCode, rab oof is foo bar backwards true");
#endif

    //
    // Boolean substitution = false
    //
    mlp1.arg1 = false;

#ifdef PEGASUS_HAS_ICU
    PEGASUS_TEST_ASSERT(MessageLoader::getMessage(mlp1) ==
        "CIM_ERR_SUCCESS: SUCCESSFUL en-us rab oof is foo bar backwards, "
            "number = false");
#else
    PEGASUS_TEST_ASSERT(MessageLoader::getMessage(mlp1) ==
        "Default CIMStatusCode, rab oof is foo bar backwards false");
#endif
}

int main(int argc, char* argv[])
{
    verbose = getenv ("PEGASUS_TEST_VERBOSE") ? true : false;
    for (Sint32 index = 1; index < argc; index++)
    {
        if (!strcmp(argv[index], "testRootBundle"))
        {
            testRootBundle = true;
            defaultResourceBundle = "ROOT";

        }
        else if (!strcmp(argv[index], "enableTrace"))
        {
            Tracer::setTraceLevel(Tracer::LEVEL4);
            Tracer::setTraceComponents("L10N");
            Tracer::setTraceFile("l10n.trc");
        }
        else
        {
            cerr << "Usage: " << argv[0]
                << " [testRootBundle] | [enableTrace]" << endl;
            return(1);
        }
    }

#ifdef PEGASUS_HAS_ICU

    UErrorCode status = U_ZERO_ERROR;
    uloc_setDefault(testRootBundle ? "zz" : "de", &status);

    // If PEGASUS_MSG_HOME is set then use that as the message
    // home for this test.
    // This will ignore any msg home defined for the platform (Constants.h)
    // since this is a test environment, not a production environment.
    const char* env = getenv("PEGASUS_MSG_HOME");
    if (env != NULL)
    {
        MessageLoader::setPegasusMsgHome(env);
    }
    else
    {
        // PEGASUS_MSG_HOME is not set.  Since we need the test messages,
        // use PEGASUS_HOME as the message home.
        env = getenv("PEGASUS_HOME");
        if (env != NULL)
        {
            String msghome(env);
            msghome.append("/msg");
            MessageLoader::setPegasusMsgHome(msghome);
        }
        else
        {
            PEGASUS_STD(cout) << "Either PEGASUS_MSG_HOME or PEGASUS_HOME "
                "needs to be set for this test!" << PEGASUS_STD(endl);
            exit(-1);
        }
    }

    // If PEGASUS_USE_DEFAULT_MESSAGES env var is set then we need
    // to make sure that it doesn't break this test.
    // Reset _useDefaultMsg to make sure PEGASUS_USE_DEFAULT_MESSAGES
    // is ignored.
    MessageLoader::_useDefaultMsg = false;

#endif

    try
    {
        testLanguageParser();
        testLanguageTag();
        testAcceptLanguageList();
        testContentLanguageList();
        testMessageLoader();
        testMessageLoaderSubs();
#ifdef PEGASUS_HAS_ICU
        testICUMessageLoaderOrdering();
#endif
    }
    catch (Exception& e)
    {
        cout << "Unexpected exception: " << e.getMessage() << endl;
        exit(1);
    }

    cout << argv[0] << " +++++ passed all tests" << endl;
    return 0;
}
