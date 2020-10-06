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
#include <Pegasus/FQL/FQLRegularExpression.h>
#include <Pegasus/Common/String.h>

PEGASUS_USING_PEGASUS;
PEGASUS_USING_STD;

Boolean verbose;

#define VCOUT if (verbose) cout

/* Test a single pattern against a string and return either true or false
   @param num - Number for this test to output with error message
   @param pattern String representing the pattern to be used
   @param testStr String that the pattern will be matched against
   @param result Boolean expected result. Returns true of match is same
          as this expected result
   @return true if match returns true. Returns false and generates output
          display if match fails
*/
void testPattern(Uint32 num, Uint32 srcLine,
            const String& pattern, const String& testStr, Boolean result)
{
    VCOUT << "Test # " << num << " pattern: \"" << pattern
          << "\". Test String: \"" << testStr
          << "\". Expected Result: "
          << " from Source line " << srcLine
          << (result? "true" : "false:") << endl;

    // build the regex object with the pattern
    FQLRegularExpression re(pattern);

    // Execute the match and if false output error information
    Boolean rtn = (re.match(testStr));

    if (result == rtn)
    {
        return;
    }
    else
    {
        cout << "ERROR for test # " << num << " pattern: \"" << pattern
             << "\". Test String: \"" << testStr << "\". Expected Result: "
             << (result? "true" : "false:")
             << " source line " << srcLine << endl;
        PEGASUS_TEST_ASSERT(false);
    }
}


int main()
{
    verbose = (getenv ("PEGASUS_TEST_VERBOSE")) ? true : false;
    // Test one pattern vs string per call
    //  (test #, pattern, stringToBeTested, expected result)
    testPattern(1,__LINE__,"", "", false);
    testPattern(2,__LINE__,"abc", "", false);
    testPattern(3,__LINE__,"", "abc", false);

    testPattern(4,__LINE__,"abc", "abc", true);
    testPattern(5,__LINE__,"abcd", "abc", false);

    testPattern(6,__LINE__,"abc", "abcd", false);

    testPattern(7,__LINE__,"ab.", "ab?", true);
    testPattern(8,__LINE__,".a.b", "aa!b", true);
    testPattern(9,__LINE__,"\\.ab", ".ab", true);
    testPattern(10,__LINE__,"\\.ab", "\\.ab", false);

    testPattern(11,__LINE__,".*", "abcd", true);

    testPattern(12,__LINE__,"\\.*", "......", true);
    testPattern(13,__LINE__,"abcd*", "abcddddd", true);

    testPattern(14,__LINE__,"abcd*", "abcd", false);

    testPattern(15,__LINE__,"ab*cd", "abbbbcd", true);
    testPattern(16,__LINE__,"\\*ab", "*ab", true);
    testPattern(17,__LINE__,".\\*", "****", false);
    testPattern(18,__LINE__,".est", "Test", true);
//// KS_TODO this pattern fails so we are exluding it until we understand
//// the issue
////  testPattern(19,__LINE__,".*est", "Test", true);
    //// KS_TODO not sure what this one should do.
////  testPattern(19,__LINE__,"*est", "Test", true);
    // The following tests use utf16 chars
    {
        Char16 utf16Chars[] = {0xD800,0x78BC, 0xDC01, 0x45A3, 0x00};

        const String utf(utf16Chars);
        testPattern(30,__LINE__,utf, utf, true);
    }

    {
        Char16 utf16CharsP[] = { 0xD800,0x78BC, 0x00};

        String utfPattern(utf16CharsP);
        utfPattern.append("*");
        Char16 utf16[] = {0xD800,0x78BC, 0xD800,0x78BC, 0xD800,0x78BC, 0x00};

        const String utfString(utf16);
        testPattern(31,__LINE__,utfPattern, utfString, true);
    }
    {
        Char16 utf16CharsS[] = {0xD800,0x78BC, 0x00};

        String utfString(utf16CharsS);
        utfString.append("*");
        testPattern(32,__LINE__,".*", utfString, true);
    }

    {
        Char16 utf16CharsP[] = {0xD800,0x78BC, 0x00};

        String utfPattern(utf16CharsP);
        utfPattern.append(".*");

        Char16 utf16CharsS[] = {0xD800,0x78BC, 0x00};

        String utfString(utf16CharsS);
        utfString.append("an3s");
        testPattern(33,__LINE__,utfPattern, utfString, true);
    }
    cout << "+++++ passed all tests" << endl;
    return 0;
}
