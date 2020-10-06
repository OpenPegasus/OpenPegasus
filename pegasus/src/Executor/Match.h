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

#ifndef _Executor_Match_h
#define _Executor_Match_h

#include "Defines.h"

static int Match(const char* pattern, const char* str)
{
    const char* p;
    const char* q;

    /* Now match expression to str. */

    for (p = pattern, q = str; *p && *q; )
    {
        if (*p == '*')
        {
            const char* r;

            p++;

            /* Recursively call to find the shortest match. */

            for (r = q; *r; r++)
            {
                if (Match(p, r) == 0)
                    break;
            }

            q = r;

        }
        else if (*p == *q)
        {
            p++;
            q++;
        }
        else
            return -1;
    }

    /* If src was exhausted but pattern has a single '*' remaining charcters,
     * then match the result.
     */

    if (p[0] == '*' && p[1] == '\0')
        return 0;

    /* If anything left over, then they do not match. */

    if (*p || *q)
        return -1;

    return 0;
}

#endif /* _Executor_Match_h */
