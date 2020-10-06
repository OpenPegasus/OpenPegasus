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

#include <Pegasus/CQL/CQLUtilities.h>

// Query includes
#include <Pegasus/Query/QueryCommon/QueryException.h>

// Pegasus Common includes
#include <Pegasus/Common/Tracer.h>
#include <Pegasus/Common/StringConversion.h>

// standard includes
#include <errno.h>

// symbol defines

// required for the windows compile
#ifndef _MSC_VER
#define _MSC_VER 0
#endif

PEGASUS_NAMESPACE_BEGIN

const char CQLUtilities::KEY[] = "CQL.CQLUtilities.INVALID_STRING";
const char CQLUtilities::MSG[] = "Error converting string to $0. "
    " String '$1' is invalid or causes an overflow ";

Uint64 CQLUtilities::stringToUint64(const String &stringNum)
{
    PEG_METHOD_ENTER(TRC_CQL,"CQLUtilities::stringToUint64()");
    Uint64 x = 0;
    CString cstrNum = stringNum.getCString();
    const char *p = (const char*)cstrNum;
    const char *pstart = p;
    Boolean isError = false;
    Uint32 endStr = 0;

    if (*p)
    {
        endStr = stringNum.size() - 1;
    }

    // If the string is a real number, use stringToReal, then convert to
    // a Uint64
    if (isReal(stringNum))
    {
        // Note:  the cast will rip off any non-whole number precision.
        // CQL spec is silent on whether to round up, round down, throw an
        // error, or allow the platform to round as it sees fit.
        // We chose the latter for now.
        return (Uint64) stringToReal64(stringNum);
    }

    if (*p == '+')
    {
        ++p;
    }

    // There cannot be a negative '-' sign or empty string
    if (*p == '-' || !*p)
    {
         isError = true;
    }
    else if (!((*p >= '0') && (*p <= '9')))
    {
         isError = true;
    }
    // if hexidecimal
    else  if ( ((*p == '0') && ((p[1] == 'x') || (p[1] == 'X'))))
    {
         if (!StringConversion::hexStringToUint64(p,x,true))
         {
            isError = true;
         }
    }
    else if (pstart[endStr] == 'b' || pstart[endStr] == 'B')
    {
         if (!StringConversion::binaryStringToUint64(p,x,true))
         {
            isError = true;
         }
    }
    else if (!StringConversion::decimalStringToUint64(p,x,true))
    {
         isError = true;
    }
    // return the value for the decimal string
    if(isError)
    {
       MessageLoaderParms mload(
           CQLUtilities::KEY,
           CQLUtilities::MSG,
           "Uint64",
           stringNum);
       throw CQLRuntimeException(mload);
    }
    else
    {
      PEG_METHOD_EXIT();
      return x;
    }
}

Sint64 CQLUtilities::stringToSint64(const String &stringNum)
{
    PEG_METHOD_ENTER(TRC_CQL,"CQLUtilities::stringToSint64()");
    Sint64 x = 0;
    CString cstrNum = stringNum.getCString();
    const char *p = (const char*)cstrNum;
    const char* pstart = p;
    Boolean isError = false;
    Uint32 endStr = 0;

    if (*p)
    {
        endStr = stringNum.size() - 1;
    }

    // If the string is a real number, use stringToReal, then convert to
    // a Sint64
    if (isReal(stringNum))
    {
        // Note:  the cast will rip off any non-whole number precision.
        // CQL spec is silent on whether to round up, round down, throw
        // an error, or allow the platform to round as it sees fit.
        // We chose the latter for now.
        return (Sint64) stringToReal64(stringNum);
    }

    // skip over the sign if there is one
    if ((*p == '-')||(*p == '+'))
    {
        p++;
    }

    if (!(*p >= '0') && (*p <= '9'))
    {
        isError = true;
    }

    // ********************
    // Build the Sint64 as a negative number, regardless of the
    // eventual sign (negative numbers can be bigger than positive ones)
    // ********************

    // if hexadecimal
    else if ( (*p == '0') && (p[1] == 'x' || p[1] == 'X'))
    {
        // Convert a hexadecimal string
        if (!StringConversion::stringToSint64(
            (const char *)(stringNum.getCString()),
            StringConversion::hexStringToUint64,
            x))
        {
           isError = true;
        }

        // return value from the hex string

    }  // end if hexidecimal

    // if binary

    else if ( (pstart[endStr] == 'b') || (pstart[endStr] == 'B') )
    {
    // Add on each digit, checking for overflow errors
      if (!StringConversion::stringToSint64(
          (const char *)(stringNum.getCString()),
          StringConversion::binaryStringToUint64,
          x))
       {
          isError = true;
       }

    // return value from the binary string

    }  // end if binary

    // Expect a positive decimal digit:


    else if (!StringConversion::stringToSint64(
         (const char *)(stringNum.getCString()),
         StringConversion::decimalStringToUint64,
         x))
    {
       isError = true;
    }
    if(isError)
    {
      MessageLoaderParms mload(
          CQLUtilities::KEY,
          CQLUtilities::MSG,
          "Sint64",
          stringNum);
      throw CQLRuntimeException(mload);

    }
    else
    {
      PEG_METHOD_EXIT();
      return x;
    }

}

Real64 CQLUtilities::stringToReal64(const String &stringNum)
{
    PEG_METHOD_ENTER(TRC_CQL,"CQLUtilities::stringToReal64()");

    Real64 x = 0;
    CString cstrNum = stringNum.getCString();
    const char *p = (const char*)cstrNum;
    Boolean neg = false;

    if (*p  == '-')
    {
        neg = true;
    }
    if(!isReal(stringNum))
    {
      // Check if it it is a binary or hex integer
        if (neg)
            x = stringToSint64(stringNum);
        else

// Check if the complier is MSVC 6, which does not support the conversion
//  operator from Uint64 to Real64
#if defined(PEGASUS_PLATFORM_WIN32_IX86_MSVC) && (_MSC_VER < 1300)
       {
         Uint64 num = stringToUint64(stringNum);
         Sint64 half = num / 2;
         x = half;
         x += half;
         if (num % 2)  // if odd, then add the lost remainder
             x += 1;
       }
#else
        x = stringToUint64(stringNum);
#endif
        PEG_METHOD_EXIT();
        return x;
    }

    //
    // Do the conversion
    //
    if (!StringConversion::stringToReal64(
        (const char *)(stringNum.getCString()),
        x))
    {
      MessageLoaderParms mload(
          CQLUtilities::KEY,
          CQLUtilities::MSG,
          "Real64",
          stringNum);
      throw CQLRuntimeException(mload);
    }
    PEG_METHOD_EXIT();
    //  printf("String %s = %.16e\n", (const char *)stringNum.getCString(), x);
      return x;
}

String CQLUtilities::formatRealStringExponent(const String &realString)
{
    String newString(realString);
    Uint32 expIndex = PEG_NOT_FOUND;
    Uint32 index = newString.size() - 1;

    expIndex = newString.find('E');
    if (expIndex == PEG_NOT_FOUND)
        expIndex = newString.find('e');

    if (expIndex == PEG_NOT_FOUND)
        return newString;  // no exponent symbol, so just return

    // format the exponent
    index = expIndex + 1;  // start index at next character
    if (newString[index] == '+')
        newString.remove(index, 1);  // remove the '+' symbol

    if (newString[index] == '-')
        index++;   // skip the '-' exponent sign

    while (index < newString.size() && newString[index] == '0')
    {
        newString.remove(index, 1);
    }

    // If only an 'e' is left (only 0's behind it) then strip the 'e'
    if (index >= newString.size())
        newString.remove(expIndex, 1);

    return newString;
}

Boolean CQLUtilities::isReal(const String &numString)
{
    // If there is a decimal point, we consider it to be a real.
    if (numString.find('.') == PEG_NOT_FOUND)
        return false;
    return true;
}

PEGASUS_NAMESPACE_END
