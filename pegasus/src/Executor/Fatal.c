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

#include <stdarg.h>
#include <syslog.h>
#include <stdio.h>
#include "Fatal.h"
#include "Log.h"
#include "Exit.h"
#include "Globals.h"
#include "Strlcpy.h"
#include "Strlcat.h"

/*
**==============================================================================
**
** Fatal()
**
**     Report fatal errors. The callar set fatal_file and fatal_line before
**     calling this function. Note that since this is a single threaded
**     application, there is no attempt to synchronize access to these
**     globals.
**
**==============================================================================
*/

void Fatal(const char* file, size_t line, const char* format, ...)
{
    char prefixedFormat[EXECUTOR_BUFFER_SIZE];

#ifdef PEGASUS_DEBUG
    /* Prepend "__FILE__(__LINE__): FATAL: " to format. */

    char lineStr[32];
    Strlcpy(prefixedFormat, file, sizeof(prefixedFormat));
    Strlcat(prefixedFormat, "(", sizeof(prefixedFormat));
    sprintf(lineStr, "%u", (unsigned int)line);
    Strlcat(prefixedFormat, lineStr, sizeof(prefixedFormat));
    Strlcat(prefixedFormat, "): FATAL: ", sizeof(prefixedFormat));
#endif

    Strlcat(prefixedFormat, format, sizeof(prefixedFormat));

    /* Print to syslog. */
    {
        va_list ap;
        char buffer[EXECUTOR_BUFFER_SIZE];

        va_start(ap, format);
        /* Flawfinder: ignore */
        vsprintf(buffer, prefixedFormat, ap);
        va_end(ap);

        syslog(LOG_CRIT, "%s", buffer);
    }

    /* Print to stderr. */
    {
        va_list ap;

        fprintf(stderr, "%s: ", globals.argv[0]);
        va_start(ap, format);
        /* Flawfinder: ignore */
        vfprintf(stderr, prefixedFormat, ap);
        va_end(ap);
        fputc('\n', stderr);
    }

    Exit(1);
}
