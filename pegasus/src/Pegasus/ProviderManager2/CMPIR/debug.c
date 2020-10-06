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

#include <Pegasus/ProviderManager2/CMPIR/debug.h>

#ifdef PEGASUS_DEBUG

static char * DebugLevels[] = { "critical", "normal", "info", "verbose" };


int trace_level(int level)
{
    char * l = getenv("RCMPI_DEBUG");
    int i = sizeof ( DebugLevels ) / sizeof ( char * );

    if (l == NULL)
    {
        return 0;
    }

    while (i--)
    {
        if (PEGASUS_CMPIR_STRCASECMP( l, DebugLevels[i] ) == 0)
        {
            return(level <= i);
        }
    }
    return 0;
}

char * trace_format(const char * fmt, ...)
{
    va_list ap;
    char * msg = (char *) malloc(512);

    va_start(ap, fmt);

    PEGASUS_CMPIR_VSPRINTF(msg, 512, fmt, ap );

    return msg;
}


void trace_this(int level,
    const char * file,
    int line,
    char * msg)
{
    fprintf(stderr,
        "--rcmpi(%s)--[%d(%d,%d)]:%s:(%d): %s\n",
        DebugLevels[level],
        PEGASUS_CMPIR_GETPID(), PEGASUS_CMPIR_GETUID(), PEGASUS_CMPIR_GETGID(),
        file, line,
        msg);
    free(msg);
}

void start_debugger()
{

#ifdef PEGASUS_OS_TYPE_UNIX
    int ch;
    char * debugger = getenv("RCMPI_DEBUGGER");

    if (debugger != NULL)
    {
        if ((ch = fork()))
        {
            sleep(20); /* wait until debugger traces us */
        }
        else
        {
            char pid[10];
            char * argv[] = { debugger,
                "OOP-Provider",
                pid,
                NULL};
            sprintf(pid, "%d", getppid());
            execv(debugger, argv);

            TRACE_CRITICAL(("could not start debugger \"%s\", "
                "check RCMPI_DEBUGGER environment "
                "variable.",
                debugger));
            exit(-1);
        }
    }
#endif
}
#endif  /* PEGASUS_DEBUG */

#ifndef PEGASUS_PLATFORM_LINUX_GENERIC_GNU
void error_at_line(
    int a_num,
    int error,
    char* filename,
    int line,
    char* message, ...)
{
    va_list ap;

    va_start(ap, message);
    fprintf(stderr, "Error in line %d of file %s: %s -", line, filename,
        strerror(error));
    vfprintf(stderr, message, ap);
    fprintf(stderr, "\n");
    va_end(ap);

    if (a_num < 0)
    {
        exit(a_num);
    }
}
#endif
