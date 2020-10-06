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

#include "CQLRegularExpression.h"
#include <Pegasus/Common/Char16.h>
#include <Pegasus/Common/CommonUTF.h>

PEGASUS_NAMESPACE_BEGIN

CQLRegularExpression::CQLRegularExpression(const String& pattern):
    pattern(pattern)
{
}

CQLRegularExpression::~CQLRegularExpression()
{
}


Boolean CQLRegularExpression::match(const String& string)

{
    Uint32 patIndex = 0;
    Uint32 strIndex = 0;

    // if either pattern or string are "EMPTY" you have an invalid String
    if (pattern == String::EMPTY)
    {
        return false;
    }

    if (string == String::EMPTY)
    {
        return false;
    }

    while (true)
    {
        if ( (string.size() == strIndex) && (pattern.size() == patIndex))
        {
            return true;
        }
        else if ((string.size() == strIndex) || (pattern.size() == patIndex))
        {
            return false;
        }

        //  Check if pattern equal to  '.'
        if (pattern[patIndex] == '.')
        {
            //assumes a valid multi-byte pair has been passed
            if ((((Uint16)pattern[patIndex] >= FIRST_HIGH_SURROGATE) &&
                 ((Uint16)pattern[patIndex] <= LAST_HIGH_SURROGATE)) ||
                (((Uint16)pattern[patIndex] >= FIRST_LOW_SURROGATE) &&
                 ((Uint16)pattern[patIndex] <= LAST_LOW_SURROGATE)))
            {
                patIndex ++;
                strIndex ++;
            }

            strIndex ++;
            patIndex ++;

            // Check if pattern equal to '*'
        }
        else if (pattern[patIndex] == '*')
        {
            if(patIndex == 0)
                return false;
            if (pattern[patIndex-1] == '.')
            {
                if ((patIndex > 1) && pattern[patIndex-2] =='\\')
                {
                    if (string[strIndex] != '.')
                    {
                        return false;
                    }
                }
                else if (pattern.size()-1 == patIndex)
                {
                    return true;
                }
                else if (string.size()-1 == strIndex)
                {
                    return false;
                }
            }
            else if (pattern[patIndex-1] == '\\')
            {
                if (pattern[patIndex-2] == '.')
                {
                    if (string[strIndex] != '*')
                    {
                        return false;
                    }
                }
            }
            else if (
                 (((Uint16)pattern[patIndex-2] >= FIRST_HIGH_SURROGATE) &&
                    ((Uint16)pattern[patIndex-2] <= LAST_HIGH_SURROGATE)) ||
                 (((Uint16)pattern[patIndex-2] >= FIRST_LOW_SURROGATE) &&
                    ((Uint16)pattern[patIndex-2] <= LAST_LOW_SURROGATE)))
            {

                if (pattern[patIndex-2] != string[strIndex])
                {
                    return false;
                } else if (pattern[patIndex-1] != string[strIndex+1])
                {
                    return false;
                }
                else
                {
                    strIndex ++;
                }
            }
            else if (pattern[patIndex-1] != string[strIndex])
            {
                return false;
            }
            while (true)
            {
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
                            return true;
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
                            } else if (strIndex == string.size()-1 &&
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
                            return true;
                        }
                        while (true) {
                            strIndex ++;
                            if (string[strIndex] != '*')
                            {
                                patIndex ++;
                                break;
                            }
                            if (strIndex == string.size()-1 &&
                                patIndex == pattern.size()-1)
                            {
                                return true;
                            }
                        }
                    }
                }
                else if (
                     (((Uint16)pattern[patIndex-2] >= FIRST_HIGH_SURROGATE) &&
                        ((Uint16)pattern[patIndex-2] <= LAST_HIGH_SURROGATE)) ||
                     (((Uint16)pattern[patIndex-2] >= FIRST_LOW_SURROGATE) &&
                        ((Uint16)pattern[patIndex-2] <= LAST_LOW_SURROGATE)))
                {
                    if (pattern[patIndex-2] != string[strIndex])
                    {
                        patIndex ++;
                        break;
                    }
                    else if (pattern[patIndex-1] != string[strIndex+1])
                    {
                        patIndex ++;
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
                    return true;
                } else if (strIndex == string.size()-1)
                {
                    return false;
                }

            }
            // check if pattern equal to '\'
        }
        else if (pattern[patIndex] == '\\')
        {
            patIndex ++;
            if ((((Uint16)pattern[patIndex] >= FIRST_HIGH_SURROGATE) &&
                 ((Uint16)pattern[patIndex] <= LAST_HIGH_SURROGATE)) ||
                (((Uint16)pattern[patIndex] >= FIRST_LOW_SURROGATE) &&
                 ((Uint16)pattern[patIndex] <= LAST_LOW_SURROGATE)))
            {

                if (pattern[patIndex] != string[strIndex])
                {
                    return false;
                }
                else if (pattern[patIndex+1] != string[strIndex+1])
                {
                    return false;
                }
                else
                {
                    patIndex ++;
                    strIndex ++;
                }
            }
            else
            {
                if (pattern[patIndex] != string[strIndex]) {
                    return false;
                }
                if (strIndex == string.size()-1 &&
                    patIndex == pattern.size()-1)
                {
                    return true;
                }
                strIndex ++;
                patIndex ++;

            }

            //default
        }
        else
        {
            if ((((Uint16)pattern[patIndex] >= FIRST_HIGH_SURROGATE) &&
                 ((Uint16)pattern[patIndex] <= LAST_HIGH_SURROGATE)) ||
                (((Uint16)pattern[patIndex] >= FIRST_LOW_SURROGATE) &&
                 ((Uint16)pattern[patIndex] <= LAST_LOW_SURROGATE)))
            {
                if (pattern[patIndex] != string[strIndex])
                {
                    return false;
                }
                else if (pattern[patIndex+1] != string[strIndex+1])
                {
                    return false;
                } else {
                    patIndex ++;
                    strIndex ++;
                }
            }
            else if (pattern[patIndex] != string[strIndex])
            {
                return false;
            }
            patIndex ++;
            strIndex ++;
        }
    }
    PEGASUS_UNREACHABLE( return false; )
}

PEGASUS_NAMESPACE_END



