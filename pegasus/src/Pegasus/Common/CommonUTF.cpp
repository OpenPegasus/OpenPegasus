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

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/Array.h>
#include <Pegasus/Common/Logger.h>
#include "CommonUTF.h"
#include <Pegasus/Common/String.h>
#include <cstdio>
#include <cstring>
#include <cctype>

#ifdef PEGASUS_HAS_ICU
#include <unicode/uclean.h>
#endif

PEGASUS_NAMESPACE_BEGIN

const Uint32 halfBase = 0x0010000UL;
const Uint32 halfMask = 0x3FFUL;
const int halfShift  = 10;
const Uint8 firstByteMark[7] = { 0x00, 0x00, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC };

const Uint32 offsetsFromUTF8[6] = { 0x00000000UL, 0x00003080UL, 0x000E2080UL,
             0x03C82080UL, 0xFA082080UL, 0x82082080UL };

const char trailingBytesForUTF8[256] = {
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2, 3,3,3,3,3,3,3,3,4,4,4,4,5,5,5,5
};
inline Uint8 _hexCharToNumeric(Char16 c)
{
    Uint8 n;

    if (isdigit(c))
        n = (c - '0');
    else if (isupper(c))
        n = (c - 'A' + 10);
    else // if (islower(c))
        n = (c - 'a' + 10);

    return n;
}

// Note: Caller must ensure that "src" contains "size" bytes.
Boolean isValid_U8(const Uint8 *src, int size)
{
    Uint8 U8_char;
    const Uint8 *srcptr = src+size;
    switch (size)
    {
    case 4:
        if ((U8_char = (*--srcptr)) < 0x80 || U8_char > 0xBF)
        {
        return false;
        }
    case 3:
        if ((U8_char = (*--srcptr)) < 0x80 || U8_char > 0xBF)
        {
        return false;
        }
    case 2:
        if ((U8_char = (*--srcptr)) > 0xBF)
        {
        return false;
        }
        switch (*src)
        {
        case 0xE0:
            if (U8_char < 0xA0)
            {
            return false;
            }
            break;
        case 0xF0:
            if (U8_char < 0x90)
            {
            return false;
            }
            break;
        case 0xF4:
            if (U8_char > 0x8F)
            {
            return false;
            }
            break;
        default:
            if (U8_char < 0x80)
            {
            return false;
            }
        }
    case 1:
        if (*src >= 0x80 && *src < 0xC2)
        {
        return false;
        }
        if (*src > 0xF4)
        {
        return false;
        }
        break;
        default:
        {
        return false;
            }

    }
    return true;
}

int UTF16toUTF8(const Uint16** srcHead,
        const Uint16* srcEnd,
        Uint8** tgtHead,
        Uint8* tgtEnd)
{
    int returnCode = 0;
    const Uint16* src = *srcHead;
    Uint8* tgt = *tgtHead;
    while (src < srcEnd)
    {
        if (*src < 128)
        {
            if (tgt == tgtEnd)
            {
                returnCode = -1;
                break;
            }

            *tgt++ = (Uint8)*src++;
            continue;
        }

    Uint32 tempchar;
    Uint16 numberOfBytes = 0;
    const Uint16* oldsrc = src;
    tempchar = *src++;
    if (tempchar >= FIRST_HIGH_SURROGATE
        && tempchar <= LAST_HIGH_SURROGATE)
    {
        if (src < srcEnd)
        {
        Uint32 tempchar2 = *src;
        if (tempchar2 >= FIRST_LOW_SURROGATE &&
            tempchar2 <= LAST_LOW_SURROGATE)
        {
            tempchar = ((tempchar - FIRST_HIGH_SURROGATE) << halfShift)
              + (tempchar2 - FIRST_LOW_SURROGATE) + halfBase;
            ++src;
        }
        }
        else
        {
        --src;
        returnCode = -1;
        break;
        }
    }
    if (tempchar < (Uint32)0x80)
    {
        numberOfBytes = 1;
    }
    else if (tempchar < (Uint32)0x800)
    {
        numberOfBytes = 2;
    }
    else if (tempchar < (Uint32)0x10000)
    {
        numberOfBytes = 3;
    }
    else if (tempchar < (Uint32)0x200000)
    {
        numberOfBytes = 4;
    }
    else
    {
        numberOfBytes = 2;
        tempchar = REPLACEMENT_CHARACTER;
    }

    tgt += numberOfBytes;
    if (tgt > tgtEnd)
    {
        src = oldsrc;
        tgt -= numberOfBytes;
        returnCode = -1;
        break;
    }

    switch (numberOfBytes)
    {
        case 4:
        *--tgt = (Uint8)((tempchar | 0x80) & 0xBF);
        tempchar >>= 6;
        case 3:
        *--tgt = (Uint8)((tempchar | 0x80) & 0xBF);
        tempchar >>= 6;
        case 2:
        *--tgt = (Uint8)((tempchar | 0x80) & 0xBF);
        tempchar >>= 6;
        case 1:
        *--tgt =  (Uint8)(tempchar | firstByteMark[numberOfBytes]);
    }
    tgt += numberOfBytes;
    }
    *srcHead = src;
    *tgtHead = tgt;
    return returnCode;
}

int UTF8toUTF16 (const Uint8** srcHead,
         const Uint8* srcEnd,
         Uint16** tgtHead,
         Uint16* tgtEnd)
{
    int returnCode = 0;
    const Uint8* src = *srcHead;
    Uint16* tgt = *tgtHead;
    while (src < srcEnd)
    {
    Uint32 tempchar = 0;
    Uint16 moreBytes = trailingBytesForUTF8[*src];
    if (src + moreBytes >= srcEnd)
    {
        returnCode = -1;
        break;
    }
    switch (moreBytes)
    {
        case 3:
        tempchar += *src++;
        tempchar <<= 6;
        case 2:
        tempchar += *src++;
        tempchar <<= 6;
        case 1:
        tempchar += *src++;
        tempchar <<= 6;
        case 0:
        tempchar += *src++;
    }
    tempchar -= offsetsFromUTF8[moreBytes];

    if (tgt >= tgtEnd)
    {
        src -= (moreBytes+1);
        returnCode = -1; break;
    }
    if (tempchar <= MAX_BYTE)
    {
        if ((tempchar >= FIRST_HIGH_SURROGATE &&
         tempchar <= LAST_LOW_SURROGATE) ||
        ((tempchar & 0xFFFE) == 0xFFFE))
        {
        *tgt++ = REPLACEMENT_CHARACTER;
        }
        else
        {
        *tgt++ = (Uint16)tempchar;
        }
    }
    else if (tempchar > MAX_UTF16)
    {
        *tgt++ = REPLACEMENT_CHARACTER;
    }
    else
    {
        if (tgt + 1 >= tgtEnd)
        {
        src -= (moreBytes+1);
        returnCode = -1;
        break;
        }
        tempchar -= halfBase;
        *tgt++ = (Uint16)((tempchar >> halfShift) + FIRST_HIGH_SURROGATE);
        *tgt++ = (Uint16)((tempchar & halfMask) + FIRST_LOW_SURROGATE);
    }
    }
    *srcHead = src;
    *tgtHead = tgt;
    return returnCode;
}

Boolean isUTF8Aux(const char *legal)
{
    unsigned char numBytes = UTF_8_COUNT_TRAIL_BYTES(*legal)+1;

    // Validate that the string is long enough to hold all the expected bytes.
    // Note that if legal[0] == 0, numBytes will be 1.
    for (unsigned char i=1; i<numBytes; i++)
    {
        if (legal[i] == 0)
        {
            return false;
        }
    }

    return isValid_U8((const Uint8 *)legal, numBytes);
}

Boolean isUTF8Str(const char *legal)
{
    /*char tmp[] = {0xCE,0x99,0xCE,0xBF,0xCF,0x8D,0xCE,0xBD,0xCE,
                      0xB9,0xCE,0xBA,0xCE,0xBF,0xCE,0xBD,0xCF,0x84,
                      0x00};*/
//  char tmp_[] = "class";
//  char * tmp = legal;
    size_t count = 0;
    const size_t size = strlen(legal);
//  printf("size = %d\n",size);
    while (count<size)
    {
//      printf("count = %d\n",count);
        if (isUTF8(&legal[count]) == true)
        {
            UTF8_NEXT(legal,count);
        }
        else
        {
//          printf("bad string\n");
            return false;
        }
    }
//  printf("good string\n");
    return true;
/*
    printf("legal = %s\n\n", legal);
    Uint32 count = 0;
    Uint32 trailingBytes = 0;
    Uint32 size = strlen(legal);
    printf("size of legal is %d\n",size);
    while (count<size-1)
    {
        printf("count = %d\n", count);
        if (isUTF8((char*)&legal[count]) == true)
        {
            UTF8_NEXT(legal,trailingBytes);
            count += trailingBytes;
        }
        else
        {
            printf("CommonUTF8:: returning false; position[%d]",count);
            return false;
        }
    }
    printf("CommonUTF8:: returning false; position[%d]",count);
    return true;*/
}

String escapeStringEncoder(const String& Str)
{
    String escapeStr;
    Uint16 escChar;
    char hexencoding[6];

    for (Uint32 i = 0; i < Str.size(); ++i)
    {
        escChar = Str[i];
        if (escChar <= 0x7F)
        {
            escapeStr.append(escChar);
        }
        else
        {
            memset(hexencoding,0x00,sizeof(hexencoding));
            sprintf(hexencoding, "%%%03X%X", escChar/16, escChar%16);
            escapeStr.append(hexencoding);
        }
    }
    return escapeStr;
}

String escapeStringDecoder(const String& Str)
{
    Uint32 i;

    Array<Uint16> utf16Chars;

    for (i=0; i< Str.size(); ++i)
    {
        if (Str[i] == '%')
        {
            Uint8 digit1 = _hexCharToNumeric((Str[++i]));
            Uint8 digit2 = _hexCharToNumeric((Str[++i]));
            Uint8 digit3 = _hexCharToNumeric((Str[++i]));
            Uint8 digit4 = _hexCharToNumeric((Str[++i]));

        Uint16 decodedChar = (digit1<<12) + (digit2<<8) +
                                 (digit3<< 4) + (digit4);

            utf16Chars.append(decodedChar);
        }
        else
        {
            utf16Chars.append((Uint16)Str[i]);
        }
    }

    // If there was a string to decode...
    if (Str.size() > 0)
    {
        utf16Chars.append('\0');
        return String((Char16 *)utf16Chars.getData());
    }
    else
    {
        return String();
    }
}

#ifdef PEGASUS_HAS_ICU

Boolean InitializeICU::_initAttempted = false;
Boolean InitializeICU::_initSuccessful = false;
Mutex InitializeICU::_initMutex;

Boolean InitializeICU::initICUSuccessful()
{
    if (!_initAttempted)
    {
        {
            AutoMutex lock(_initMutex);

            if (!_initAttempted)
            {
                UErrorCode _status = U_ZERO_ERROR;

                // Initialize ICU
                u_init(&_status);

                if (U_FAILURE(_status))
                {
                    _initSuccessful = false;
                    Logger::put(
                        Logger::STANDARD_LOG , System::CIMSERVER,
                        Logger::WARNING,
                        "ICU initialization failed with error: $0.",
                        _status);
                }
                else
                {
                    _initSuccessful = true;
                }
                _initAttempted = true;
            }
        }
    }

    return _initSuccessful;
}

#endif

PEGASUS_NAMESPACE_END
