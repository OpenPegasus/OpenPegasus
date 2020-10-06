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

#include <Pegasus/Query/QueryCommon/SubRange.h>
#include <Pegasus/Query/QueryCommon/QueryException.h>
#include <Pegasus/Common/InternalException.h>

PEGASUS_NAMESPACE_BEGIN

SubRange::SubRange()
    : start(0),
      end(0)
{
}

SubRange::SubRange(String range)
{
    parse(range);
}

Boolean SubRange::operator==(const SubRange &rhs) const
{
    return ((start == rhs.start) && (end == rhs.end));
}

Boolean SubRange::operator!=(const SubRange &rhs) const
{
    return !operator==(rhs);
}

String SubRange::toString() const
{
    char start_[15];
    char end_[15];
    sprintf(start_,"%d",start);
    sprintf(end_,"%d",end);
    String s(start_);
    s.append("-");
    s.append(end_);
    return s;
}

void SubRange::parse(String range)
{
    /*
       Look for the following possibilities:
       *** NOTE: 2-7 are currently not supported ***
       1. "3"   start is set to 3, end is set to 3
       2. "3-5" start is set to 3, end is set to 5
       3. "3.." start is set to 3, end is set to END_OF_ARRAY
       4. "..3" start is set to 0, end is set to 3
       5. ".."  start is set to 0, end is set to END_OF_ARRAY
       6. ""    start is set to NO_INDEX
       7. "*"   start is set to 0, end is set to END_OF_ARRAY
    */
    //Uint32 index;
    CString _cstr;
/*
    if (range == String::EMPTY)
    {
        start = SUBRANGE_NO_INDEX;
        end = SUBRANGE_NO_INDEX;
        return;
    }
    if (range == "*" || range == "..")
    {
        start = 0;
        end = SUBRANGE_END_OF_ARRAY;
        return;
    }
*/

    if (range.size() == 1)
    {
        _cstr = range.getCString();
        if (isNum(_cstr))
        {
            start = atoi(_cstr);
            end = start;
        }
        else
        {
            // error
            MessageLoaderParms parms(
                "QueryCommon.SubRange.INDEX_NOT_NUMBER",
                "An array index string cannot be converted to a numeric form.");
            throw QueryParseException(parms);
        }
        return;
    }
    else
    {
        // error
        MessageLoaderParms parms(
            "QueryCommon.SubRange.INDEX_NOT_SINGLE",
            "An array index is not a single unsigned integer.");
        throw QueryParseException(parms);
    }

/*
    if ((index = range.find('-')) != PEG_NOT_FOUND)
    {
        String s = range.subString(0,index);
        String e = range.subString(index+1);
        _cstr = s.getCString();
        if (isNum(_cstr))
        {
            start = atoi(_cstr);
        }
        else
        {
            // error
            printf("SubRange::parse() error\n");
        }
        _cstr = e.getCString();
        if (isNum(_cstr))
        {
            end = atoi(_cstr);
        }
        else
        {
            // error
            printf("SubRange::parse() error\n");
        }
        return;
    }
    if ((index = range.find("..") == 0))
    {
        start = 0;
        String e = range.subString(2);
        _cstr = e.getCString();
        if (isNum(_cstr))
        {
            end = atoi(_cstr);
        }
        else
        {
            // error
            printf("SubRange::parse() error\n");
        }
        return;
    }
    else if ((index = range.find("..")) != PEG_NOT_FOUND)
    {
        // make sure nothing comes after the ..
        if (range.size() > (index+2)){
            // error
            printf("SubRange::parse() error\n");
        }
        printf("index = %d\n", index);
        String s = range.subString(0,index);
        _cstr = s.getCString();
        printf("%s\n", (const char*)_cstr);
        if (isNum(_cstr))
        {
            start = atoi(_cstr);
        }
        else
        {
            // error
            printf("SubRange::parse() error\n");
        }
        end = SUBRANGE_END_OF_ARRAY;
        return;
    }
*/
}

Boolean SubRange::isNum(CString cstr)
{
    Uint32 size = (Uint32)strlen(cstr);
    for (Uint32 i = 0; i < size; i++)
    {
        if (!isdigit(cstr[i]))
            return false;
    }
    return true;
}

PEGASUS_NAMESPACE_END
