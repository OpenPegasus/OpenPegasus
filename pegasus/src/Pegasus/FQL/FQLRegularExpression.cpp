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
//
/*
    Current regex definition the same as CQL Basic LIKE Regular expressions
    See DSP0202 v 1.0, section C.1
        Regular Expression Parser.
    The CQL parser regular expression is
    * matches zero or more instances of the proceeding single character.
    . matches any single character
    \ Escape the next character (next must be must be *, . or \)
    \\ The backslash character
*/

// TODO: This was taken from CQL and we need to merge the code so that there
// is only one function for the ltwo. KS August 2014

/*
    The goal is to produce a full Regex tool in accord with DSP1001
    Annex B

    The FQL regular expressions defined in DSP1001 are a subset of
    UNIX Regular Expressions as follows

    The abnf for the regex is documented in the readme in this
    directory. NOTE: Today that is the goal.
*/

#include "FQLRegularExpression.h"
#include <Pegasus/Common/Char16.h>
#include <Pegasus/Common/CommonUTF.h>

PEGASUS_USING_STD;

// FQL_TOTRACE defined in the Makefile
#ifdef FQL_DOTRACE
#define DCOUT if (true) cout << __FILE__ << ":" << __LINE__ << " "
#define COMPILE_LOCAL_TRACE
#define DISPSTRANDPAT  DCOUT << "RegularExpression Line " \
                        << " strIndex " << strIndex \
                        << " patIndex " << patIndex << endl
// macro to conditionally display return information
#define MATCHRETURN(RTN_VALUE) \
    DCOUT << "RegularExpression rtns  " << RTN_VALUE << " " <<  __LINE__ \
        << " strIndex " << strIndex << " \'" \
        << string.subString(strIndex,1) << "\' " \
        << " string.size() " << string.size() \
        << " patIndex \'" << patIndex << string.subString(patIndex,1) \
        << "\' " << " pattern.size() " << pattern.size()<< endl; \
        return RTN_VALUE
#else
#define MATCHRETURN(rtnValue) return rtnValue
#define DISPSTRANDPAT
#endif

PEGASUS_NAMESPACE_BEGIN

PEGASUS_USING_STD;

bool testSurrogates(const String & pattern, Uint32 patIndex)
{
return (((Uint16)pattern[patIndex] >= FIRST_HIGH_SURROGATE) &&
        ((Uint16)pattern[patIndex] <= LAST_HIGH_SURROGATE)) ||
        (((Uint16)pattern[patIndex] >= FIRST_LOW_SURROGATE) &&
        ((Uint16)pattern[patIndex] <= LAST_LOW_SURROGATE));
}

bool matchChar(const String & pattern, const String & string,
                Uint32 patIndex, Uint32 strIndex)
{
#ifdef COMPILE_LOCAL_TRACE
    DCOUT << "RegularExpressionmatchChar Pattern \"" << pattern
         << " pchar " << pattern.subString(patIndex,1)
         << "\" String to match \"" << string << "\" "
         << " strChar " << string.subString(strIndex,1) <<  endl;
#endif
    if ( (pattern[patIndex] != string[strIndex]))
    {
        MATCHRETURN(false) ;
    }
    else if (pattern[patIndex+1] != string[strIndex+1])
    {
        MATCHRETURN(false) ;
    }
    return true;
}

FQLRegularExpression::FQLRegularExpression(const String& pattern):
    pattern(pattern)
{
}

FQLRegularExpression::~FQLRegularExpression()
{
}


Boolean FQLRegularExpression::match(const String& string)
{
    Uint32 patIndex = 0;
    Uint32 strIndex = 0;

#ifdef COMPILE_LOCAL_TRACE
    DCOUT << "RegularExpression Pattern \"" << pattern
         << "\" String to match \"" << string << "\"" << endl;
#endif
    // if either pattern or string are "EMPTY" you have an invalid String
    if (pattern.size() == 0 || (string.size() == 0))
    {
        MATCHRETURN(false);
    }

    while (true)
    {
#ifdef COMPILE_LOCAL_TRACE
        DCOUT << "RegularExpression while true "
              << " strIndex " << strIndex << " \'"
              << string.subString(strIndex,1) << "\' "
              << " patIndex \'" << patIndex << string.subString(strIndex,1)
              << "\' " << endl;
#endif
        if ( (string.size() == strIndex) && (pattern.size() == patIndex))
        {
            MATCHRETURN(true);
        }

        else if ((string.size() == strIndex) || (pattern.size() == patIndex))
        {
            MATCHRETURN(false);
        }

        //  Check if pattern equal to  '.'
        if (pattern[patIndex] == '.')
        {
            DISPSTRANDPAT;

            //assumes a valid multi-byte pair has been passed
            if (testSurrogates(pattern, patIndex))
            {
                patIndex ++;
                strIndex ++;
            }

            strIndex ++;
            patIndex ++;

        }
        // Check if pattern char  equal to '*'
        else if (pattern[patIndex] == '*')
        {
            DISPSTRANDPAT;

            // and if first char in pattern, illegal
            if(patIndex == 0)
            {
                MATCHRETURN(false);
            }
            // and if prev char is escape char, DOT
            if (pattern[patIndex-1] == '.')
            {
                if ((patIndex > 1) && pattern[patIndex-2] =='\\')
                {
                    if (string[strIndex] != '.')
                    {
                        MATCHRETURN(false);
                    }
                }
                else if (pattern.size()-1 == patIndex)
                {
                    MATCHRETURN(true);
                }
                else if (string.size()-1 == strIndex)
                {
                    MATCHRETURN(false);
                }
            }
            // otherwise if prev char is backslash
            else if (pattern[patIndex-1] == '\\')
            {
                if (pattern[patIndex-2] == '.')
                {
                    if (string[strIndex] != '*')
                    {
                        MATCHRETURN(false);
                    }
                }
            }
            else if(testSurrogates(pattern, patIndex-2))
            {
                if (!matchChar(pattern, string, patIndex-2, strIndex))
                {
                    MATCHRETURN(false);
                }
                else
                {
                    strIndex ++;
                }
            }
            else if (pattern[patIndex-1] != string[strIndex])
            {
                MATCHRETURN(false);
            }

            while (true)
            {
                DISPSTRANDPAT;
                strIndex ++;

                if (pattern[patIndex-1] == '.')
                {
                    if ((patIndex > 1) && (pattern[patIndex-2] =='\\'))
                    {
                        if (string[strIndex] != '.')
                        {
                            patIndex ++;
                            break;
                        }
                    }
                    else if (pattern[patIndex+1] == string[strIndex])
                    {
                        //make copies of the indexes in case you do not reach
                        //the end of the string
                        int stringOrig = strIndex;
                        int patternOrig = patIndex;
                        patIndex++;

                        if (strIndex == string.size()-1 &&
                                patIndex == pattern.size()-1)
                        {
                            MATCHRETURN(true);
                        }
                        while (true)
                        {
                            strIndex++;
                            patIndex ++;
                            if (pattern[patIndex] != string[strIndex])
                            {
                                strIndex = stringOrig + 1;
                                patIndex = patternOrig;
                                break;
                            }
                            else if (strIndex == string.size()-1 &&
                                   patIndex == pattern.size()-1)
                            {
                                break;
                            }
                            patIndex++;
                        }
                    }
                }
                else if (pattern[patIndex-1] == '\\')
                {
                    DISPSTRANDPAT;

                    if (pattern[patIndex-2] == '.')
                    {
                        if (string[strIndex] != '*')
                        {
                            patIndex ++;
                            break;
                        }
                        if (strIndex == string.size()-1 &&
                            patIndex == pattern.size()-1)
                        {
                            MATCHRETURN(true);
                        }
                        while (true)
                        {
                            strIndex ++;
                            if (string[strIndex] != '*')
                            {
                                patIndex ++;
                                break;
                            }
                            if (strIndex == string.size()-1 &&
                                patIndex == pattern.size()-1)
                            {
                                MATCHRETURN(true);
                            }
                        }
                    }
                }

                else if(testSurrogates(pattern, patIndex-2))
                {
                    DISPSTRANDPAT;

                    if (!matchChar(pattern, string, patIndex-2, strIndex))
                    {
                        patIndex++;
                        break;
                    }
                    else
                    {
                        strIndex ++;
                    }
                }
                else if (pattern[patIndex-1] != string[strIndex])
                {
                    patIndex ++;
                    break;
                }

                if (strIndex == string.size()-1 &&
                    patIndex == pattern.size()-1)
                {
                    MATCHRETURN(true);
                }
                else if (strIndex == string.size()-1)
                {
                    MATCHRETURN(false);
                }
            }
        }
        // check if pattern equal to '\', the escape char
        else if (pattern[patIndex] == '\\')
        {
            DISPSTRANDPAT;

            patIndex ++;
            if (testSurrogates(pattern, patIndex))
            {
                if (!matchChar(pattern, string, patIndex, strIndex))
                {
                    MATCHRETURN(false);
                }
                else
                {
                    patIndex ++;
                    strIndex ++;
                }
            }
            else
            {
                DISPSTRANDPAT;

                if (pattern[patIndex] != string[strIndex])
                {
                    MATCHRETURN(false);
                }
                if (strIndex == string.size()-1 &&
                    patIndex == pattern.size()-1)
                {
                    MATCHRETURN(true);
                }
                strIndex ++;
                patIndex ++;
            }

        //default, Test next character against pattern for equality
        }
        else
        {
            DISPSTRANDPAT;
            if (testSurrogates(pattern, patIndex))
            {
                if (!matchChar(pattern, string, patIndex, strIndex))
                {
                    MATCHRETURN(false);
                }
                else
                {
                    patIndex ++;
                    strIndex ++;
                }
            }
            else if (pattern[patIndex] != string[strIndex])
            {
                MATCHRETURN(false);
            }
            patIndex ++;
            strIndex ++;
        }
    }   // end of while loop
    PEGASUS_UNREACHABLE( return false; )
}

PEGASUS_NAMESPACE_END
