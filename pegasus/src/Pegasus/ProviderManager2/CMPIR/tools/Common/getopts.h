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
    \file getopts.h
    \brief defines getopts function
*/

#ifndef _REMOTE_CMPI_GETOPTS_H
#define _REMOTE_CMPI_GETOPTS_H

#include <string.h>

/*
    Retrives the options like UNIX getopts command
    opts (in) - option list.
    n    (in) - Should be initialized to zero when calling this function for
                the first time. This value should not be changed elsewhere in
                the program until getopts function finishes parsing the
                arguements. Using this value getopts knows the next argument
                to be parsed.
    optsarg (out) - A pointer to the argument of the option is stored in this.
    argc (in) - number of arguments.
    argv (in) - Actual arguments list.
*/

char* getopts(char *opts,int *n,char **optsarg,int argc, char *argv[])
{
    int i;
    char *arg,*tmp;

    if (*n + 1 >= argc)
    {
        return 0;
    }

    ++*n;
    arg = argv[*n];
    *optsarg = "Unknown option.";
    if (*arg++ == '-')
    {
        while ((tmp = strchr(opts,':')))
        {
            i = tmp-opts;
            if (!strncmp(arg,opts,i))
            {
                arg += i;
                if (*arg)
                {
                    *optsarg = arg;
                }
                else if (*n + 1 <argc)
                {
                    *optsarg = argv[++*n];
                }
                else
                {
                    *optsarg = "Option requires value.";
                    break;
                }
                return(char*)opts;
            }
            else
            {
                opts += i+1;
            }
        }
    }

    return "";   /* return an empty string, an error has occured */
}

#endif
