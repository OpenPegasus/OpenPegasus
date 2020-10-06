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

/*!
    \file tokenizer.h
    \brief Defines gettoken function.

*/

#ifndef _REMOTE_CMPI_TOKENIZER_H
#define _REMOTE_CMPI_TOKENIZER_H

#include <ctype.h>

#define EOL   0
#define EQUAL 100
#define LB    101
#define RB    102
#define COMMA 103
#define ALNUM 104

#define BUFFLEN  4096
#define TOKENLEN 1024


int gettoken(char **buff,char *token)
{
    int tokentype;

    while (**buff && isspace(**buff))
    {
        ++*buff;
    }

    if (!**buff)
    {
        return *token = EOL;
    }

    *token = *(*buff)++;
    switch (*token++)
    {
        case '=':
            tokentype = EQUAL;
            break;
        case '{':
            tokentype = LB;
            break;
        case '}':
            tokentype = RB;
            break;
        case ',':
            tokentype = COMMA;
            break;
        default:
            while (**buff && !isspace(**buff) &&
                (isalnum(**buff)|| **buff=='_'))
            {
                *token++ = *(*buff)++;
            }
            tokentype = ALNUM;
    }
    *token ='\0';

    return tokentype;
}

#endif
