/*
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
*/

#include "Random.h"
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

/*
**==============================================================================
**
** FillRandomBytes1()
**
**     Fills *data* with *size* random bytes using the high-quality random
**     number generator device (/dev/urandom).
**
**==============================================================================
*/

static int _fillRandomBytes1(unsigned char* data, size_t size)
{
    size_t numErrors = 0;
    const size_t MAX_ERRORS_TOLERATED = 20;

    int fd = open("/dev/urandom", O_RDONLY | O_NONBLOCK);

    if (fd < 0)
        return -1;

    while (size)
    {
        ssize_t n = read(fd, data, size);

        if (n < 1)
        {
            if (++numErrors == MAX_ERRORS_TOLERATED)
                break;

            if (errno == EAGAIN)
                continue;

            break;
        }

        size -= n;
        data += n;
    }

    close(fd);
    return size == 0 ? 0 : -1;
}

/*
**==============================================================================
**
** FillRandomBytes2()
**
**     Fills *data* with *size* random bytes using the standard rand() function.
**     Note: this function uses srand(), which is generally not considered
**     "random" enough for security purposes but it is only used if
**     FillRandomBytes1() fails.
**
**==============================================================================
*/

static void _fillRandomBytes2(unsigned char* data, size_t size)
{
    struct timeval tv;

    /* Seed the random number generator. */

    gettimeofday(&tv, 0);

    /* Flawfinder: ignore */
    srand(tv.tv_usec);

    /* Fill data with random bytes. */

    while (size--)
        *data++ = rand();
}

/*
**==============================================================================
**
** FillRandomBytes()
**
**==============================================================================
*/

void FillRandomBytes(unsigned char* data, size_t size)
{
    if (_fillRandomBytes1(data, size) != 0)
        _fillRandomBytes2(data, size);
}

/*
**==============================================================================
**
** RandBytesToHexASCII()
**
**     Converts the bytes given by *data* to a hexidecimal sequence of ASCII
**     characters. The *ascii* parameter must be twice size plus one (for the
**     null terminator).
**
**==============================================================================
*/

void RandBytesToHexASCII(
    const unsigned char* data,
    size_t size,
    char* ascii)
{
    static char _hexDigits[] =
    {
        '0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'
    };
    size_t i;
    size_t j = 0;

    for (i = 0; i < size; i++)
    {
        unsigned char hi = data[i] >> 4;
        unsigned char lo = 0x0F & data[i];
        ascii[j++] = _hexDigits[hi];
        ascii[j++] = _hexDigits[lo];
    }

    ascii[j] = '\0';
}
