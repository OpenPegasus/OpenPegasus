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

#ifndef _Executor_Defines_h
#define _Executor_Defines_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/Constants.h>
#include <assert.h>

/*
**==============================================================================
**
** EXECUTOR_LINKAGE
**
**     Linkage macro used to export various functions in the executor. Since the
**     executor is a self-contained program, export linkage may seem unecessary
**     but the source files in this directory are also included in a test
**     library used to test some of the executor functions.
**
**==============================================================================
*/

#ifdef EXECUTOR_PLEASE_EXPORT
# define EXECUTOR_LINKAGE PEGASUS_EXPORT
#else
# define EXECUTOR_LINKAGE PEGASUS_IMPORT
#endif

/*
**==============================================================================
**
** EXECUTOR_TRACE()
**
**     Handy macro for temporary source-level tracing and debugging.
**
**==============================================================================
*/

#define EXECUTOR_TRACE printf("TRACE: %s(%d)\n", __FILE__, __LINE__)

/*
**==============================================================================
**
** EXECUTOR_RESTART()
**
**     Macro used to repeatedly restart (retry) a system call as long as the
**     errno is EINTR.
**
**==============================================================================
*/

#define EXECUTOR_RESTART(F, X) while (((X = (F)) == -1) && (errno == EINTR))

/*
**==============================================================================
**
** EXECUTOR_BUFFER_SIZE
**
**     General purpose buffer size (large enough for any file path or user
**     name).
**
**==============================================================================
*/

#define EXECUTOR_BUFFER_SIZE 4096

/*
**==============================================================================
**
** FL
**
**     Shorthand macro for passing __FILE__ and __LINE__ arguments to a
**     function.
**
**==============================================================================
*/

#define FL __FILE__, __LINE__

/*
**==============================================================================
**
** CIMSERVERMAIN
**
**     The name of the main CIM server program.
**
**==============================================================================
*/

#define CIMSERVERMAIN "cimservermain"

/*
**==============================================================================
**
** CIMSHUTDOWN
**
**     The name of the main CIM shutdown program.
**
**==============================================================================
*/

#define CIMSHUTDOWN "cimshutdown"

/*
**==============================================================================
**
** CIMPROVAGT
**
**     The name of the provider agent program.
**
**==============================================================================
*/

#define CIMPROVAGT "cimprovagt"

/*
**==============================================================================
**
** CIMPROVAGT32
**
**     The name of the 32-bit provider agent program.
**
**==============================================================================
*/

#define CIMPROVAGT32 "cimprovagt32"

/*
**==============================================================================
**
** CIMSERVERA
**
**     The name of the external PAM authentication program.
**
**==============================================================================
*/

#define CIMSERVERA "cimservera"

/*
**==============================================================================
**
** ProviderModule bitness values
**
**==============================================================================
*/

#define BITNESS_DEFAULT 1
#define BITNESS_32 2
#define BITNESS_64 3

/*
**==============================================================================
**
** EXIT_ON_POLICY_FAILURE
**
**     If this is defined, the executor exits on any policy failure.
**
**==============================================================================
*/

#if 0
# define EXIT_ON_POLICY_FAILURE
#endif

/*
**==============================================================================
**
** HAVE_MSG_CONTROL
**
**     Define this if the platform support Berkeley msg control structures.
**
**==============================================================================
*/

#if defined(PEGASUS_OS_LINUX)
# define HAVE_MSG_CONTROL
#endif

#endif /* _Executor_Defines_h */
