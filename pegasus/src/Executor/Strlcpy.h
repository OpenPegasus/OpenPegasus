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

#ifndef _Executor_Strlcpy_h
#define _Executor_Strlcpy_h

#include <stddef.h>

/*
**==============================================================================
**
** Strlcpy()
**
**     This is an original implementation of the strlcpy() function as described
**     by Todd C. Miller in his popular security paper entitled "strlcpy and
**     strlcat - consistent, safe, string copy and concatenation".
**
**     Note that this implementation favors readability over efficiency. More
**     efficient implementations are possible but would be too complicated
**     to verify in a security audit.
**
**==============================================================================
*/

static size_t Strlcpy(char* dest, const char* src, size_t size)
{
    size_t i;

    for (i = 0; src[i] && i + 1 < size; i++)
        dest[i] = src[i];

    if (size > 0)
        dest[i] = '\0';

    while (src[i])
        i++;

    return i;
}

#endif /* _Executor_Strlcpy_h */
