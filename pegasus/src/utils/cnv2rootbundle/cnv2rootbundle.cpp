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

// FILE: cnv2rootbundle.cpp
// DATE: 8/20/03
// AUTHOR: John Bartucz
// PURPOSE: to convert a text library to a root library
//          use -u to uppercase

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char *argv[])
{
    char infilename[1024];
    char outfilename[1024];
    FILE* outfile;
    FILE* textfile;
    char* tmp;
    char line[1024];

    unsigned int i;
    int found_first_line = 0;
    int in_quotes;

    int do_uppercase = 0;

    if (argc != 2 && argc != 3)
    {
        fprintf(stderr, "USAGE: %s {-u if uppercase} [filename]\n", argv[0]);
        exit(1);
    }

    if (argc == 3)
    {
        if (!strcmp(argv[1], "-u"))
        {
            do_uppercase = 1;
        }
        else
        {
            fprintf(stderr,
                "USAGE: %s {-u if uppercase} [filename]\n", argv[0]);
            exit(1);
        }

        strcpy(infilename, argv[2]);
    }
    else
    {
        strcpy(infilename, argv[1]);
    }

    if ((textfile = fopen(infilename, "r")) == 0)
    {
        fprintf(stderr,
            "ERROR: could not open %s\nUSAGE: %s {-u if uppercase} "
                "[filename]\n",
            infilename,
            argv[0]);
        exit(1);
    }

    sprintf (outfilename, "%s", infilename);
    if ((tmp = strchr(outfilename, '_')) == NULL)
    {
        fprintf(stderr,
            "ERROR: could not convert %s - invalid filename\n"
                "USAGE: %s {-u if uppercase} [filename]\n",
            infilename,
            argv[0]);
        exit(1);
    }

    // append root to the outputfilename
    sprintf(tmp, "_root.rb");

    if ((outfile = fopen(outfilename, "w")) == 0)
    {
        fprintf(stderr,
            "COULD NOT open outfile %s for writing\n", outfilename);
        exit(1);
    }

    while (fgets(line, 1024, textfile))
    {
        // we're just going to translate comments
        // because they won't be seen anyway.

        if (found_first_line == 0 && strstr(line, ":table"))
        {
/*
IBM-CC. Since we are going to back to just having the language as the name
of the table, comment out this code.
            if ((tmp = strchr(line, '_')) == NULL)
            {
                fprintf(stderr,
                    "ERROR: first line of file not in proper format: "
                        "[bundlename]_[lang]:table {\nLINE: %s",
                    line);
                exit(1);
            }

            // deal with the first line (the bundle name)
            tmp[0] = '\0';

            fprintf(outfile, "%s_root:table {", line); //\n
*/
            fprintf(outfile, "root:table {"); //\n

            found_first_line = 1;
        }
        else
        {
            // have to do the first character outside the loop
            // because the loop compares character[i - 1]
            if (line[0] == '\"')
            {
                in_quotes = 1;
            }
            else
            {
                in_quotes = 0;
            }
            fprintf (outfile, "%c", line[0]);
            // end of first character special case

            for (i = 1; i < strlen(line); i++)
            {
                if (do_uppercase)
                {
                    if (in_quotes == 0)
                    {
                        if (line[i] == '\"' && line[i-1] != '\\')
                        {
                            in_quotes = 1;
                        }
                    }
                    else if (in_quotes == 1)
                    {
                        if (line[i] == '\"' && line[i-1] != '\\')
                        {
                            in_quotes = 0;
                        }

                        if (line[i] >= 'a' && line[i] <= 'z' &&
                            line[i-1] != '\\')
                        {
                            line[i] += 'A' - 'a'; // uppercase the character
                        }
                    }

                } // end if do_uppercase

                fprintf(outfile, "%c", line[i]);

            } // ends the for-loop for each character in the line

        } // ends the "if-else" statement for the first line special case

    } // ends the line reading loop

    if (found_first_line == 0)
    {
/*
IBM-CC. Since we are going to back to just having the language as the name of
the table, comment out this code.
        fprintf(stderr,
            "ERROR: did not find bundle name in proper format: "
                "[bundlename]_[lang]:table {\nLINE: %s",
            line);
*/
        fprintf(stderr,
            "ERROR: did not find table name in proper format: "
                "[lang]:table {\nLINE: %s",
            line);
    }

    fclose(outfile);
    fclose(textfile);

    return 0;
}
