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
    \file debug.h
    \brief Bug tracing facility.

    This header file defines macros for tracing output
    using different debug levels, which can be defined during runtime as
    environment variable.

    Modules including this header file have to be compiled used -DDEBUG to
    enable debug support.

*/

#ifndef _REMOTE_CMPI_DEBUG_H
#define _REMOTE_CMPI_DEBUG_H

#include "cmpir_common.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <sys/types.h>


#ifdef PEGASUS_OS_TYPE_UNIX
# include <unistd.h>
# if defined PEGASUS_OS_ZOS
#  include <strings.h>
# else
#  include <string.h>
# endif
#elif defined PEGASUS_OS_TYPE_WINDOWS
# include <process.h>
# include <string.h>
#endif

#define DEBUG_VERBOSE  3
#define DEBUG_INFO     2
#define DEBUG_NORMAL   1
#define DEBUG_CRITICAL 0

#define TRACE(level,args) \
    if ( trace_level ( (level) ) ) \
    { \
        char * __msg = trace_format args; \
        trace_this ( \
            (level), \
            __FILE__, __LINE__, \
            __msg ); \
        }

#if defined PEGASUS_DEBUG
 #define TRACE_VERBOSE(args)  TRACE(DEBUG_VERBOSE,args)
#else
 #define TRACE_VERBOSE(args)
#endif

#if defined PEGASUS_DEBUG
 #define TRACE_INFO(args)     TRACE(DEBUG_INFO,args)
#else
 #define TRACE_INFO(args)
#endif

#if defined PEGASUS_DEBUG
 #define TRACE_NORMAL(args)   TRACE(DEBUG_NORMAL,args)
#else
 #define TRACE_NORMAL(args)
#endif

#if defined PEGASUS_DEBUG
 #define TRACE_CRITICAL(args) TRACE(DEBUG_CRITICAL,args)
#else
 #define TRACE_CRITICAL(args)
#endif

#if defined PEGASUS_DEBUG
# define START_DEBUGGER start_debugger ()
#else
# define START_DEBUGGER
#endif

#if defined (PEGASUS_CMPI_PROXY_INTERNAL) || \
    defined (PEGASUS_CMPI_NATIVE_INTERNAL)
# define PEGASUS_RCMPI_DEBUG_VISIBILITY PEGASUS_EXPORT
#else
# define PEGASUS_RCMPI_DEBUG_VISIBILITY PEGASUS_IMPORT
#endif

#ifdef PEGASUS_DEBUG

PEGASUS_RCMPI_DEBUG_VISIBILITY int trace_level(int);
PEGASUS_RCMPI_DEBUG_VISIBILITY char* trace_format(const char *fmt, ...);
PEGASUS_RCMPI_DEBUG_VISIBILITY void trace_this(int, const char *, int, char *);
PEGASUS_RCMPI_DEBUG_VISIBILITY void start_debugger();

#endif /* PEGASUS_DEBUG */

#ifndef PEGASUS_PLATFORM_LINUX_GENERIC_GNU
PEGASUS_RCMPI_DEBUG_VISIBILITY void error_at_line(int a_num, int error,
    char* filename, int line, char* message, ...);
#endif

#endif /* _REMOTE_CMPI_DEBUG_H */
