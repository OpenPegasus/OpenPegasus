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
#include <Pegasus/Common/Char16.h>
#include <Pegasus/Common/CIMName.h>
#include <Pegasus/Common/CIMNameCast.h>

#include <Pegasus/Common/PegasusAssert.h>

PEGASUS_USING_PEGASUS;
PEGASUS_USING_STD;

static Boolean verbose;

#define VCOUT if (verbose) cout

// ASCII only
// 6+1+5+1+4+1+7 = 25
const char asciiOnly[26] = "simple_ASCII_only_CIMName";
const Char16 asciiOnlyChar16[26]= {
    0x73, 0x69, 0x6D, 0x70, 0x6c, 0x65,         // "simple"
    0x5F,
    0x41, 0x53, 0x43, 0x49, 0x49,               // "ASCII"
    0x5F,
    0x6F, 0x6E, 0x6C, 0x79,                     // "only"
    0x5F,
    0x43, 0x49, 0x4D, 0x4E, 0x61, 0x6D, 0x65,   // "CIMName"
    0x00                                        // null termination
};

// UTF-8 characters only
// 1+2+2+3
const char utf8Only[9] = {
    '\x79',                  // letter y
    '\xC3', '\xA4',          // umlaut ae
    '\xC2', '\xAE',          // sign for registered trademark
    '\xE2', '\x82', '\xAC',  // euro sign
    '\0'                     // null termination
};
const Char16 utf8OnlyChar16[5] = {
    0x0079,                  // letter y
    0x00E4,                  // sign for registered trademark
    0x00AE,                  // euro sign
    0x20AC,                  // null termination
    0x0000
};

// mixed ASCII and UTF-8 characters
const char utf8Mixed[14] = {
    'a',
    '\x79',                  // letter y
    'b',
    '\xC3', '\xA4',          // umlaut ae
    'c',
    '\xC2', '\xAE',          // sign for registered trademark
    'd',
    '\xE2', '\x82', '\xAC',  // euro sign
    'e',
    '\0'                     // null termination
};

const Char16 utf8MixedChar16[10] = {
    'a',
    0x0079,                  // letter y
    'b',
    0x00E4,                  // umlaut ae
    'c',
    0x00AE,                  // sign for registered trademark
    'd',
    0x20AC,                  // euro sign
    'e',
    0x0000                   // null termination
};


// ASCII with invalid UTF-8, error in start byte
const char utf8BrokenStartByte[6] = {
    'a', 'b', 'c',
    '\xFF',                   // invalid start byte
    '\x83',                   // sequence byte for 2 byte UTF-8
    '\0'                      // null termination
};

// ASCII with invalid UTF-8, error in sequence bytes
const char utf8BrokenSequenceByte[6] = {
    'a', 'b', 'c',
    '\xC2',                   // valid start byte for 2 byte UTF-8
    '\x00',                   // invalid sequence byte, valid binary='10xx xxxx'
    '\0'                      // null termination
};

const char invalidWhiteSpacesName[23]="CIM Invalid Class Name";

// length should be the number of 2byte characters
Boolean equalChar16(const Char16* x, const Char16* y, int length)
{
    for (int i=0; i <length; i++)
    {
        if (x[i] != y[i])
        {
            //return false;
        }
    }
    return true;
}

void test01()
{
    VCOUT << "Test CIMName(char*) with valid ASCII...";

    CIMName tempCIMName(asciiOnly);

    // check size
    PEGASUS_TEST_ASSERT(25 == tempCIMName.getString().size());

    // check if what we get back equals what we placed into it
    char* backTransformation;
    backTransformation = strdup(tempCIMName.getString().getCString());
    PEGASUS_TEST_ASSERT(backTransformation);
    PEGASUS_TEST_ASSERT(25 == strlen(backTransformation));
    PEGASUS_TEST_ASSERT(0 == strcmp(backTransformation, asciiOnly));
    free(backTransformation);

    // for safety, check the UTF-16 representation is correct
    const Char16* tempChar16= tempCIMName.getString().getChar16Data();
    PEGASUS_TEST_ASSERT(equalChar16(tempChar16, asciiOnlyChar16, 26));
    VCOUT << " +++++ passed" << endl;
}

void test02()
{
    VCOUT << "Test CIMName(char*) with valid utf-8 only...";

    CIMName tempCIMName(utf8Only);

    // check size
    PEGASUS_TEST_ASSERT(4 == tempCIMName.getString().size());

    // check if what we get back equals what we placed into it
    char* backTransformation;
    backTransformation = strdup(tempCIMName.getString().getCString());
    PEGASUS_TEST_ASSERT(backTransformation);
    PEGASUS_TEST_ASSERT(8 == strlen(backTransformation));
    PEGASUS_TEST_ASSERT(0 == strcmp(backTransformation, utf8Only));
    free(backTransformation);

    // for safety, check the UTF-16 representation is correct
    const Char16* tempChar16= tempCIMName.getString().getChar16Data();
    PEGASUS_TEST_ASSERT(equalChar16(tempChar16, utf8OnlyChar16, 5));
    VCOUT << " +++++ passed" << endl;
}

void test03()
{
    VCOUT << "Test CIMName(char*) with mixed valid utf-8 and ASCII...";

    CIMName tempCIMName(utf8Mixed);
    // check size
    PEGASUS_TEST_ASSERT(9 == tempCIMName.getString().size());

    // check if what we get back equals what we placed into it
    char* backTransformation;
    backTransformation = strdup(tempCIMName.getString().getCString());
    PEGASUS_TEST_ASSERT(backTransformation);
    PEGASUS_TEST_ASSERT(13 == strlen(backTransformation));
    PEGASUS_TEST_ASSERT(0 == strcmp(backTransformation, utf8Mixed));
    free(backTransformation);

    // for safety, check the UTF-16 representation is correct
    const Char16* tempChar16= tempCIMName.getString().getChar16Data();
    PEGASUS_TEST_ASSERT(equalChar16(tempChar16, utf8MixedChar16, 10));
    VCOUT << " +++++ passed" << endl;
}

void test04()
{
    VCOUT << "Test CIMName(char*) ASCII with invalid UTF-8. "
                 "Error in start byte at index 3...";
    try
    {
        CIMName tempCIMName(utf8BrokenStartByte);
    }
    catch(Exception& e)
    {
        String message=e.getMessage();
        String expectedErrorMessage(
            "The byte sequence starting at index 3 "
            "is not valid UTF-8 encoding: abc 0xFF 0x83");
        // check if this is the expected exception
        if (String::equalNoCase(message, expectedErrorMessage))
        {
            VCOUT << " +++++ passed" << endl;
            return;
        }
        else
        {
            throw;
        }
    }
    PEGASUS_TEST_ASSERT(false);
}

void test05()
{
    VCOUT << "Test CIMName(char*) ASCII with invalid UTF-8. "
                 "Error in sequence bytes starts at byte index 3...";
    try
    {
        CIMName tempCIMName(utf8BrokenSequenceByte);
    }
    catch(Exception& e)
    {
        String message=e.getMessage();
        String expectedErrorMessage(
            "The byte sequence starting at index 3 "
            "is not valid UTF-8 encoding: abc 0xC2");
        // check if this is the expected exception
        if (String::equalNoCase(message, expectedErrorMessage))
        {
            VCOUT << " +++++ passed" << endl;
            return;
        }
        else
        {
            throw;
        }
    }
    PEGASUS_TEST_ASSERT(false);
}

void test06()
{
    VCOUT << "Test CIMName(char*) with invalid whitespaces...";
    try
    {
        CIMName tempCIMName(invalidWhiteSpacesName);
    }
    catch(InvalidNameException&)
    {
        VCOUT << " +++++ passed" << endl;
        return;
    }
    PEGASUS_TEST_ASSERT(false);
}

void test11()
{
    VCOUT << "Test assignment CIMName=(char*) with valid ASCII...";

    CIMName tempCIMName = asciiOnly;

    // check size
    PEGASUS_TEST_ASSERT(25 == tempCIMName.getString().size());

    // check if what we get back equals what we placed into it
    char* backTransformation;
    backTransformation = strdup(tempCIMName.getString().getCString());
    PEGASUS_TEST_ASSERT(backTransformation);
    PEGASUS_TEST_ASSERT(25 == strlen(backTransformation));
    PEGASUS_TEST_ASSERT(0 == strcmp(backTransformation, asciiOnly));
    free(backTransformation);

    // for safety, check the UTF-16 representation is correct
    const Char16* tempChar16= tempCIMName.getString().getChar16Data();
    PEGASUS_TEST_ASSERT(equalChar16(tempChar16, asciiOnlyChar16, 26));
    VCOUT << " +++++ passed" << endl;
}

void test12()
{
    VCOUT << "Test assignment CIMName=(char*)"
                 " with valid utf-8 only...";

    CIMName tempCIMName = utf8Only;

    // check size
    PEGASUS_TEST_ASSERT(4 == tempCIMName.getString().size());

    // check if what we get back equals what we placed into it
    char* backTransformation;
    backTransformation = strdup(tempCIMName.getString().getCString());
    PEGASUS_TEST_ASSERT(backTransformation);
    PEGASUS_TEST_ASSERT(8 == strlen(backTransformation));
    PEGASUS_TEST_ASSERT(0 == strcmp(backTransformation, utf8Only));
    free(backTransformation);

    // for safety, check the UTF-16 representation is correct
    const Char16* tempChar16= tempCIMName.getString().getChar16Data();
    PEGASUS_TEST_ASSERT(equalChar16(tempChar16, utf8OnlyChar16, 5));
    VCOUT << " +++++ passed" << endl;
}

void test13()
{
    VCOUT << "Test assignment CIMName=(char*)"
                 " with mixed valid utf-8 and ASCII...";

    CIMName tempCIMName = utf8Mixed;
    // check size
    PEGASUS_TEST_ASSERT(9 == tempCIMName.getString().size());

    // check if what we get back equals what we placed into it
    char* backTransformation;
    backTransformation = strdup(tempCIMName.getString().getCString());
    PEGASUS_TEST_ASSERT(backTransformation);
    PEGASUS_TEST_ASSERT(13 == strlen(backTransformation));
    PEGASUS_TEST_ASSERT(0 == strcmp(backTransformation, utf8Mixed));
    free(backTransformation);

    // for safety, check the UTF-16 representation is correct
    const Char16* tempChar16= tempCIMName.getString().getChar16Data();
    PEGASUS_TEST_ASSERT(equalChar16(tempChar16, utf8MixedChar16, 10));
    VCOUT << " +++++ passed" << endl;
}

void test14()
{
    VCOUT << "Test assignment CIMName=(char*) ASCII with invalid UTF-8. "
                 "Error in start byte at index 3...";
    try
    {
        CIMName tempCIMName=utf8BrokenStartByte;
    }
    catch(Exception& e)
    {
        String message=e.getMessage();
        String expectedErrorMessage(
            "The byte sequence starting at index 3 "
            "is not valid UTF-8 encoding: abc 0xFF 0x83");
        // check if this is the expected exception
        if (String::equalNoCase(message, expectedErrorMessage))
        {
            VCOUT << " +++++ passed" << endl;
            return;
        }
        else
        {
            throw;
        }
    }
    PEGASUS_TEST_ASSERT(false);
}

void test15()
{
    VCOUT << "Test assignment CIMName=(char*) ASCII with invalid UTF-8. "
                 "Error in sequence bytes starts at byte index 3...";
    try
    {
        CIMName tempCIMName = utf8BrokenSequenceByte;
    }
    catch(Exception& e)
    {
        String message=e.getMessage();
        String expectedErrorMessage(
            "The byte sequence starting at index 3 "
            "is not valid UTF-8 encoding: abc 0xC2");
        // check if this is the expected exception
        if (String::equalNoCase(message, expectedErrorMessage))
        {
            VCOUT << " +++++ passed" << endl;
            return;
        }
        else
        {
            throw;
        }
    }
    PEGASUS_TEST_ASSERT(false);
}

void test16()
{
    VCOUT << "Test assignment CIMName=(char*)"
                 " with invalid whitespaces...";
    try
    {
        CIMName tempCIMName=invalidWhiteSpacesName;
    }
    catch(InvalidNameException&)
    {
        VCOUT << " +++++ passed" << endl;
        return;
    }
    PEGASUS_TEST_ASSERT(false);
}

void runCIMNameConstructorTests()
{
    // valid constructor calls
    test01();
    // UTF-8 related tests, circumvent if UTF-8 support disabled
    test02();
    test03();
    // invalid constructor calls
    test04();
    test05();
    test06();
}

// Test the same things as for constructor
// now using experimental assignment operator
void runCIMNameAssignmentTests()
{
    // this is part of the experimental interface
    // valid assignment calls
    test11();
    // UTF-8 related tests, circumvent if UTF-8 support disabled
    test12();
    test13();
    // invalid assignment calls
    test14();
    test15();
    test16();
}

void runCIMNameCastTests()
{
    try
    {
        CIMName name(CIMNameCast("Okay"));
    }
    catch (...)
    {
        PEGASUS_TEST_ASSERT(false);
    }

    Boolean caught = false;

    try
    {
        CIMName name(CIMNameCast("Not Okay"));
    }
    catch (InvalidNameException& e)
    {
        caught = true;
    }
    catch (...)
    {
        PEGASUS_TEST_ASSERT(false);
    }

#if defined(PEGASUS_DEBUG)
    PEGASUS_TEST_ASSERT(caught);
#else
    PEGASUS_TEST_ASSERT(!caught);
#endif
}

int main(int, char** argv)
{
    verbose = getenv("PEGASUS_TEST_VERBOSE") ? true : false;

    try
    {
        runCIMNameConstructorTests();
        runCIMNameAssignmentTests();
        runCIMNameCastTests();
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

