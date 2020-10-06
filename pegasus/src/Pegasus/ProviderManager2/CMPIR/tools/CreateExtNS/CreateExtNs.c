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
    \file CreateExtNs.c
    \brief Creates an extended namespaces

    This program enables the creation of namespaces exploiting Shared Schema
    facilities and namespaces designated to be used as remote namespaces using
    Remote CMPI facilities.

    Usage:

         CreateExtNs -n <namespace-name>
             [ -p <parent-namespace> -l <remote-location> ]

    Where
         -n defines the name of the new nameplates.
                Examples: -n root/test
                          -n root/local

         -p Optionally define the new namespace to share schema data with an
            exciting parent namespace.
                Example: -p root/cimv2

         -l Optionally defines this namespace to be represeting a remote
            locaton with hostname <remote-location>. CMPIRDeamon must be
            running at this location.
                Example: -l localhost
                         -l hpc4711
*/

#include "../../cmpir_common.h"
#include <stdio.h>
#include <stdlib.h>
#include "../Common/getopts.h"

#define RMAXPATHLEN 4096

#if defined(PEGASUS_OS_TYPE_WINDOWS)
# define RCMPI_MKDIR(dir) mkdir(dir)
#else
# include <sys/stat.h>
# include <unistd.h>
# define RCMPI_MKDIR(dir) mkdir(dir,0775)
#endif

int main(int argc, char* argv[])
{

    char *tmp;
    char *newnamespace = 0;
    char *pnamespace = 0;
    char *remotelocation = 0;
    int  n = 0;
    char *optsarg = 0;
    char *pegasushome = 0;
    char *pathseparator = 0;
    char *repositoryname = "repository";
    char rdir[RMAXPATHLEN];
    int  rlen;


    if (0 == (pegasushome = getenv("PEGASUS_HOME")))
    {
        printf("PEGASUS_HOME environment varialble not set. ");
        return -1;
    }

    if (2 > argc)
    {
        printf(
            "Usage: CreateExtNs -n <namespace-name> [ -p <parent-namespace>"
            " -l <remote-location> ] \n");
        return 0;
    }

    while ((tmp = getopts("l:n:p:r:",&n,&optsarg,argc,argv)))
    {
        switch (*tmp)
        {
            case 'n':
                newnamespace = optsarg;
                break;
            case 'l':
                remotelocation = optsarg;
                break;
            case 'p':
                pnamespace = optsarg;
                break;
            case 'r':
                repositoryname = optsarg;
                break;
            default: // '\0'
                printf("%s\n",optsarg);
        }
    }

    pathseparator = "/";

    /* check for repository */
    strcpy(rdir,pegasushome);
    strcat(rdir,pathseparator);
    strcat(rdir,repositoryname);
    rlen = strlen(rdir);

    if (0 != access(rdir,0))
    {
        perror(rdir);
        return -1;
    }

    /* check for namespace */
    if (0 == newnamespace)
    {
        printf("Required parameter <namespace name> is missing.\n");
        return -1;
    }

    /* check for namespace existence */
    strcat(rdir,pathseparator);
    strcat(rdir,newnamespace);
    tmp = rdir + ++rlen;
    while ((tmp = strchr(tmp,*pathseparator)))
    {
        *tmp = '#';
    }
    if (0 == access(rdir,0))
    {
        printf("%s : Namespace already exisists\n",rdir);
        return 0;
    }

    rdir[rlen] = '\0'; // get back repository directory with pathseparator

    /* check for parent namespace existence */
    if (0 != pnamespace)
    {
        strcat(rdir,pnamespace);
        tmp = rdir + rlen;
        while ((tmp = strchr(tmp,*pathseparator)))
        {
            *tmp = '#';
        }
        if (0 != access(rdir,0))
        {
            printf("%s :Parent namespace does not exist",rdir);
            return -1;
        }
        rdir[rlen] = '\0';
    }

    /* create namespace */
    strcat(rdir,newnamespace);
    tmp = rdir + rlen;
    while ((tmp = strchr(tmp,*pathseparator)))
    {
        *tmp = '#';
    }
    if (0 != RCMPI_MKDIR(rdir))
    {
        printf("Create Namespace\n");
        perror(rdir);
        return -1;
    }

    strcat(rdir,pathseparator);
    rlen = strlen(rdir);

    /* classes */
    strcat(rdir,"classes");
    if (0 != RCMPI_MKDIR(rdir))
    {
        printf("Create classes\n");
        perror(rdir);
        return -1;
    }
    rdir[rlen] = '\0';

    /* qualifiers */
    strcat(rdir,"qualifiers");
    if (0 != RCMPI_MKDIR(rdir))
    {
        printf("Create qualifiers\n");
        perror(rdir);
        return -1;
    }
    rdir[rlen] ='\0';

    /* instances */
    strcat(rdir,"instances");
    if (0 != RCMPI_MKDIR(rdir))
    {
        printf("Create instances\n");
        perror(rdir);
        return -1;
    }
    rdir[rlen] ='\0';

    /* parent namespace */
    if (0 != pnamespace)
    {
        strcat(rdir,"SRS");
        strcat(rdir,pnamespace);
        tmp = rdir + rlen + 3; // 3 for SRS
        while ((tmp = strchr(tmp,*pathseparator)))
        {
            *tmp = '#';
        }
        if (0 != RCMPI_MKDIR(rdir))
        {
            printf("Create SRS parent Namespace\n");
            perror(rdir);
            return -1;
        }
        rdir[rlen] ='\0';
    }

    /* remote location */
    if (0 != remotelocation)
    {
        strcat(rdir,"r10");
        strcat(rdir,remotelocation);
        if (0 != RCMPI_MKDIR(rdir))
        {
            printf("Create remote location\n");
            perror(rdir);
            return -1;
        }
    }

    return 0;
}

