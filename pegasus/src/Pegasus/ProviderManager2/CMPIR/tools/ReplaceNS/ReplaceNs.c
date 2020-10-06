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
    \file ReplaceNs.c
    \brief Replaces Namespaces property in provider registration MOF fies.

    This program helps in modifying existing registration MOF files,  it
    replaces existing namespace name(s) of the Namespaces property.

    Usage:
         ReplaceNs <new-namespace-name> [ ... ] <mof-file-name>

    Where
        new-namespace-name -- is the namespace name to replace the existing
                              Namespaces specification. Multiple new namespace
                              names can be specified, separated by blanks.

        mof-file-name -- is the name of the mof registration file to used.

        Output of the script is routed to std out.

    Example:
        The following command replaces the existing Namespaces specification:

    ReplaceNs root/local root/node* Linux_baseR.mof >Linux_baseCMPIR.mof

    On Linux root/node* is used to define the provider supports all namespaces
    starting with root/node.

    But on windows it does not work because command shell does not expand
    wildcards.
*/

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "../Common/tokenizer.h"

int main(int argc,char *argv[])
{
    FILE *fp;
    char buff[BUFFLEN];
    char token[TOKENLEN];
    char *tmp;
    int  i;

    if (3 > argc)
    {
        printf(
            "%s\n",
            "Usage: AppendeNs <additional-namespace-name> [ ... ] "
            "<mof-file-name>");
        exit(1);
    }

    fp = fopen(argv[argc-1],"r");
    if (0 == fp)
    {
        perror(argv[argc-1]);
        exit(1);
    }

    while (0 != fgets(buff,BUFFLEN,fp))
    {
        tmp = buff;
        gettoken(&tmp,token);
        if (0 != strcmp("Namespaces",token) || 0 != strcmp("namespaces",token))
        {
            fputs(buff,stdout);
            continue;
        }

        // we completly rely on correctness of registration MOF file.
        assert(EQUAL == gettoken(&tmp,token));
        assert(LB == gettoken(&tmp,token));

        *tmp = '\0';

        /* Append new namespaces */
        for (i = 1; i< argc -1 ;++i)
        {
            if (1 != i)
            {
                strcat(buff,", ");
            }
            strcat(buff,"\"");
            strcat(buff,argv[i]);
            strcat(buff,"\"");
        }
        strcat(buff," };\n");
        fputs(buff,stdout);
    }
    fclose(fp);
    return 0;
}

