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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char* arg0;
/* Check one file
*/
void chksrc(const char* path,
            int checktab,
            int checklen,
            int checkbadcr,
            int checktrailbl,
            int summarize)
{
    char buf[4096];
    FILE* is = fopen(path, "rb");

    int reject = 0;
    int line = 1;
    int notest = 0;
    int tabcount = 0;
    int longlinecount = 0;
    int badcrcount = 0;
    int trailingblankscount = 0;

    if (!is)
    {
        const char* cvsroot;

        /*
           Check to see whether chksrc was run by an ordinary user or by the
           CVS server. In the latter case, CVSROOT is "/cvs/MSB" rather than
           ":pserver:<username>@cvs.opengroup.org:/cvs/MSB".
        */

        if ((cvsroot = getenv("CVSROOT")) && strcmp(cvsroot, "/cvs/MSB") == 0)
            return;

        fprintf(stderr, "%s: failed to open: %s\n", arg0, path);
        exit(1);
    }

    for (; fgets(buf, sizeof(buf), is) != NULL; line++)
    {
        /* Look for NOCHKSRC tag */

        if (strstr(buf, "NOCHKSRC"))
        {
            notest = 1;
        }

        if (strstr(buf, "DOCHKSRC"))
        {
            notest = 0;
        }

        if (notest == 1)
        {
            continue;
        }

        /* Check for tabs characters anywhere in the line*/

        if ( checktab && (strchr(buf, '\t')))
        {
            if (!summarize)
            {
                fprintf(stderr, "%s:%d: illegal tab character\n", path, line);
            }
            reject = 1;
            tabcount++;
        }

        /* check for trailing blank on the line if not zero length*/

        if (checktrailbl)
        {
            /* Error if /n or /r  or no more characters after last blank or tab
            */

            /* start at end of buf */
            char *pos = strchr(buf,'\0');

            /* back up past any eol characters */
            while (pos > buf)
            {
                pos--;
                /* if we have backed up past any eol characters*/
                if (*pos != '\n' && *pos != '\r')
                {
                    /* and if current character is blank or tab, we gen error*/
                    if( *pos == ' ' || *pos == '\t')
                    {
                        if (!summarize)
                        {
                            fprintf(stderr, "%s:%d: trailing blanks\n",
                                path, line);
                        }
                        reject = 1;
                        trailingblankscount++;
                    }
                    break;
                }
            }
        }

        /* Check for Ctrl-M characters in non-windows platforms */

#if !defined(PEGASUS_PLATFORM_WIN64_IA64_MSVC) && \
    !defined(PEGASUS_PLATFORM_WIN64_X86_64_MSVC) && \
    !defined(PEGASUS_PLATFORM_WIN32_IX86_MSVC)

        if (checkbadcr && (strchr(buf, '\r')))
        {
            if (!summarize)
            {
                fprintf(stderr, "%s:%d: illegal carriage return character\n",
                    path, line);
            }
            reject = 1;
            badcrcount++;
        }

#endif /* PEGASUS_OS_TYPE_WINDOWS */

        /* Check for lines longer than 80 characters
           Note: This is actually testing for 81 characters because
           it includes the EOL in the test.
           ISSUE: We must confirm that this works for windows.
        */

        if ( checklen && (strlen(buf) > 81))
        {
            if (!summarize)
            {
                fprintf(stderr, "%s:%d: line longer than 80 characters\n",
                    path, line);
            }
            reject = 1;
            longlinecount++;
        }
    }

    fclose(is);
    /* If rejected, generate message.  if also summarize set, generate
     * message with details of counts. The intention is that the
     * error messages above are formatted like normal compiler errors so can
     * be used in an editor to go directly to errors and the
     * non-suuamize message below is simply a warning.
    */

    if (reject)
    {
        if (summarize)
        {
            fprintf(stderr, "%s", path);
            if (checktab)
                fprintf(stderr, " tabs = %d", tabcount);
            if (checklen)
                fprintf(stderr, " long lines = %d", longlinecount);
            if (checktrailbl)
                fprintf(stderr, " trailing blanks = %d", trailingblankscount);
#if !defined(PEGASUS_PLATFORM_WIN64_IA64_MSVC) && \
    !defined(PEGASUS_PLATFORM_WIN64_X86_64_MSVC) && \
    !defined(PEGASUS_PLATFORM_WIN32_IX86_MSVC)
            /* we don't print this one unless there were errors
             * because it is so infrequent
             */
            if (checkbadcr && (badcrcount != 0))
            {
                fprintf(stderr, " Bad CRs = %d",badcrcount);
            }
#endif /* PEGASUS_OS_TYPE_WINDOWS */
            fprintf(stderr,"\n");
        }
        else
        {
            fprintf(stderr, "Rejected source file %s\n", path);
            exit(1);
        }
    }
}

int isSourceFile(const char* path)
{
    const char* suffixes[] =
    {
        ".l",
        ".y",
        ".h",
        ".c",
        ".cpp",
    };
    size_t num_suffixes = sizeof(suffixes) / sizeof(suffixes[0]);
    size_t n = strlen(path);
    size_t i;

    for (i = 0; i < num_suffixes; i++)
    {
        size_t m = strlen(suffixes[i]);

        if (n >= m && strcmp(path + n - m, suffixes[i]) == 0)
        {
            return 1;
        }
    }

    /* Not a source file. */
    return 0;
}


void usage()
{
    printf("Usage: %s [options] source-files...\n", arg0);
    printf("    Checks file list for tabs characters,\n");
    printf("        lines longer than 80 characters,");
    printf("\n        trailing blanks");

    /* show the following if not windows */
#if !defined(PEGASUS_PLATFORM_WIN64_IA64_MSVC) && \
    !defined(PEGASUS_PLATFORM_WIN64_X86_64_MSVC) && \
    !defined(PEGASUS_PLATFORM_WIN32_IX86_MSVC)
    printf(",\n        and windows Ctrl-M returns in non-windows platforms");
#endif
    printf(".\n");
    printf("    Disable with keyword NOCHKSRC in source code.\n");
    printf("    Reenable with keyword DOCHKSRC in source code.\n");
    printf("Options:\n");
    printf("    The t, l, and m options are on by default. If you set \n"
           "    any of the options, any not set are turned off. Thus \n"
           "    -t tests only for tabs and -tl tests for tabs and length.\n"
           "    WARNING: -b is set off by default and only enabled by"
           "    setting the option.\n");
    printf("    -t : Test only for tabs\n");
    printf("    -l : Test only for length\n");
    printf("    -b : Test only for trailing blanks\n");
    printf("    -m : Test only Ctrl-M (useful only on linux platforms)\n");
    printf("    -s : Generate only a summary file path list\n");
    printf("    -h : help\n");
    printf("Only the following file suffixes are tested: .l .y .h .c .cpp\n");
}

int main(int argc, char** argv)
{
    int i;

    int checktab = 0;
    int checklen = 0;
    int summarize = 0;
    int checkbadcr = 0;
    int checktrailbl = 0;

    char c;

    arg0 = argv[0];

    if (argc < 2)
    {
        usage();
        exit(1);
    }
    /* Get options from first parameter */
    if( argc > 1 && argv[1][0] == '-' )
    {
        for( i=1; (c=argv[1][i]) != '\0'; i++ )
        {
            if( c =='t' )
                checktab++;
            else if( c == 'l' )
                checklen++;
            else if( c == 'm' )
                checkbadcr++;
            else if( c == 's' )
                summarize++;
            else if( c == 'b' )
                checktrailbl++;
            else if( c =='h' )
                {usage(); exit(1);}
            else
               printf("Error Option %c?\n", c);
        }
           --argc;
           ++argv;
    }
    /* default is to test all  if none optioned*/
    if (checktab == 0 && checklen == 0 && checktrailbl == 0)
    {
        checktab = 1;
        checklen = 1;
        checkbadcr = 1;
        /* Defaults to off for now since there are so many
         * cased of trailing blanks in the environment today
         */
        checktrailbl = 0;
    }

    /* retest after argument removal */
    if (argc < 2)
    {
        usage();
        exit(1);
    }
    for (i = 1; i < argc; i++)
    {
        if (isSourceFile(argv[i]))
            chksrc(argv[i], checktab, checklen, checkbadcr,
                checktrailbl,summarize);
    }

    return 0;
}
