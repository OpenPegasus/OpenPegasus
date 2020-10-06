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

#include "Base64.h"
#include <Pegasus/Common/ArrayInternal.h>

PEGASUS_NAMESPACE_BEGIN
PEGASUS_USING_STD;

#ifdef PEGASUS_PLATFORM_AIX_RS_IBMCXX
#define inline
#endif


//**********************************************************
/*  Encode thanslates one six-bit pattern into a base-64 character.
    Unsigned char is used to represent a six-bit stream of date.
*/
inline PEGASUS_COMMON_LINKAGE char Base64::_Encode(Uint8 uc)
{
    if (uc < 26)
        return 'A'+uc;

    if (uc < 52)
        return 'a'+(uc-26);

    if (uc < 62)
        return '0'+(uc-52);

    if (uc == 62)
        return '+';

    return '/';
}

// Helper function returns true is a character is a valid base-64 character
// and false otherwise.

inline Boolean Base64::_IsBase64(char c)
{

    if (c >= 'A' && c <= 'Z')
        return true;

    if (c >= 'a' && c <= 'z')
        return true;

    if (c >= '0' && c <= '9')
        return true;

    if (c == '+')
        return true;

    if (c == '/')
        return true;

    if (c == '=')
        return true;

    return false;
}

 // Translate one base-64 character into a six bit pattern
inline Uint8 Base64::_Decode(char c)
{
    if (c >= 'A' && c <= 'Z')
        return c - 'A';
    if (c >= 'a' && c <= 'z')
        return c - 'a' + 26;

    if (c >= '0' && c <= '9')
        return c - '0' + 52;

    if (c == '+')
        return 62;

    return 63;
}


//*************************************************************
/*  Encode static method takes an array of 8-bit values and
    returns a base-64 stream.
    ATTN: KS feb 2002 - This is probably a very slow an inefficient
    implementation and could be improved if it is required for
    production.  Today it is only for test programs.
*/
Buffer Base64::encode(const Buffer& vby)
{
    Buffer retArray;
    // If nothing in input string, return empty string
    if (vby.size() == 0)
        return retArray;
    // for every character in the input array taken 3 bytes at a time
    for (Uint32 i=0; i < vby.size(); i+=3)
    {
        // Create from 3 8 bit values to 4 6 bit values
        Uint8 by1=0,by2=0,by3=0;
        by1 = vby[i];
        if (i+1<vby.size())
        {
            by2 = vby[i+1];
        };
        if (i+2<vby.size())
        {
            by3 = vby[i+2];
        }

        Uint8 by4=0,by5=0,by6=0,by7=0;
        by4 = by1>>2;
        by5 = ((by1&0x3)<<4)|(by2>>4);
        by6 = ((by2&0xf)<<2)|(by3>>6);
        by7 = by3&0x3f;

        retArray.append(_Encode(by4));
        retArray.append(_Encode(by5));

        if (i+1<vby.size())
            retArray.append( _Encode(by6));
        else
            retArray.append('=');


        if (i+2<vby.size())
            retArray.append( _Encode(by7));
        else
            retArray.append('=');

        /* ATTN: Need to fix this. It adds unwanted cr-lf after 4 chars.

        if (i % (76/4*3) == 0)
        {
            retArray.append( '\r');
            retArray.append( '\n');
        }
        */
    };

    return retArray;
}

/*
    I checked for the zero length. The algorithm would also work for zero
    length input stream, but I’m pretty adamant about handling border
    conditions. They are often the culprits of run-time production failures.
    The algorithm goes thru each three bytes of data at a time. The first
    thing I do is to shift the bits around from three 8-bit values to four
    6-bit values. Then I encode the 6-bit values and add then one at a time
    to the output stream. This is actually quite inefficient. The STL
    character array is being allocated one byte at a time. The algorithm
    would be much faster, if I pre-allocated that array. I’ll leave that as
    an optimization practical exercise for the reader.
*/

/*  The decode static method takes a base-64 stream and converts it
    to an array of 8-bit values.
*/
Buffer Base64::decode(const Buffer& strInput)
{
    //Strip any non-base64 characters from the input
    Buffer str;
    for (Uint32 j=0;j<strInput.size();j++)
    {
        if (_IsBase64(strInput[j]))
            str.append(strInput[j]);
    }

    Buffer retArray;

    // Return if the input is zero length
    if (str.size() == 0)
        return retArray;

    //  comment
    for (Uint32 i=0; i < str.size(); i+=4)
    {
        char c1='A',c2='A',c3='A',c4='A';

        c1 = str[i];
        if (i+1<str.size())
            c2 = str[i+1];
        if (i+2<str.size())
            c3 = str[i+2];
        if (i+3<str.size())
            c4 = str[i+3];

        Uint8 by1=0,by2=0,by3=0,by4=0;
        by1 = _Decode(c1);
        by2 = _Decode(c2);
        by3 = _Decode(c3);
        by4 = _Decode(c4);
        //cout << "base::64decode bytes" <<
        //      " 1 " << c1 << " " << by1 <<
        //      " 2 " << c2 << " " << by2 <<
        //      " 3 " << c3 << " " << by3 <<
        //      " 4 " << c4 << " " << by4 << endl;

        // append first byte by shifting
        retArray.append( static_cast<char>((by1<<2)|(by2>>4)) );

        // append second byte if not padding
        if (c3 != '=')
            retArray.append( static_cast<char>(((by2&0xf)<<4)|(by3>>2)) );

        if (c4 != '=')
            retArray.append( static_cast<char>(((by3&0x3)<<6)|by4) );
    }


    return retArray;
}

PEGASUS_NAMESPACE_END
