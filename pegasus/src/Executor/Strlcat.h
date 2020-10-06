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

#ifndef _Executor_Strlcat_h
#define _Executor_Strlcat_h

#include <stdlib.h>

/*
**==============================================================================
**
** Strlcat()
**
**     This is an original implementation of the strlcat() function as described
**     by Todd C. Miller in his popular security paper entitled "strlcpy and
**     strlcat - consistent, safe, string copy and concatenation".
**
**     Note that this implementation favors readability over efficiency. More
**     efficient implementations are possible but would be too complicated
**     to verify in a security audit.
**
**==============================================================================
*/

static size_t Strlcat(char* dest, const char* src, size_t size)
{
    size_t i;
    size_t j;

    /* Find dest null terminator. */

    for (i = 0; i < size && dest[i]; i++)
        ;

    /* If no-null terminator found, return size. */

    if (i == size)
    {
        int k = 0;
        while (src[k])
        {
            k++;
        }
        return size + k;
    }

    /* Copy src characters to dest. */

    for (j = 0; src[j] && i + 1 < size; i++, j++)
        dest[i] = src[j];

    /* Null terminate the destination.  We are guaranteed that size is
     * non-zero, because the (i == size) condition above is always true
     * when size is zero.
     */

    dest[i] = '\0';

    while (src[j])
    {
        j++;
        i++;
    }

    return i;
}

#endif /* _Executor_Strlcat_h */
