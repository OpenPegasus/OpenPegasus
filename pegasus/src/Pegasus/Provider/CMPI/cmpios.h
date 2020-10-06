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
//%////////////////////////////////////////////////////////////////////////////

#ifndef _CMPIOS_H_
#define _CMPIOS_H_

#include "cmpipl.h"
#include <stdlib.h>          // To get the size_t
#ifndef CMPI_PLATFORM_WIN32_IX86_MSVC
#define CMPI_THREAD_RETURN      void*
#else
# define CMPI_THREAD_RETURN     unsigned
#endif
#define CMPI_THREAD_TYPE        void*
#define CMPI_MUTEX_TYPE         void*
#define CMPI_COND_TYPE          void*

#if defined(CMPI_PLATFORM_WIN32_IX86_MSVC)
# define CMPI_THREAD_CDECL    __stdcall

/*# define CMPI_THREAD_KEY_TYPE unsigned long int */
/* CMPI_THREAD_KEY_TYPE is defined as void* in CMPI spec. Defining
   CMPI_THREAD_KEY_TYPE as 'unsigned long int' causing failure on 64-bit
   platforms. We make it as 'unsigned int' which is type compatible with
   pthread_key_t. This will not be problem for 32-bit platforms.
*/
# define CMPI_THREAD_KEY_TYPE unsigned int
# ifndef HAVE_STRUCT_TIMESPEC
#  define HAVE_STRUCT_TIMESPEC
   struct timespec
   {
       long tv_sec;
       long tv_nsec;
   };
# endif /* HAVE_STRUCT_TIMESPEC */

#elif defined(CMPI_PLATFORM_ZOS_ZSERIES_IBM)
# include <pthread.h>
# ifndef __cplusplus
#  define CMPI_THREAD_CDECL
# else
#  define CMPI_THREAD_CDECL    __cdecl
# endif
# define CMPI_THREAD_KEY_TYPE  pthread_key_t
#else
# define CMPI_THREAD_CDECL
/*# define CMPI_THREAD_KEY_TYPE unsigned long int */
# define CMPI_THREAD_KEY_TYPE unsigned int
#endif

/* Define CMPI_EXPORT and CMPI_IMPORT */
#if defined(CMPI_PLATFORM_WIN32_IX86_MSVC)
# define CMPI_EXPORT __declspec(dllexport)
# define CMPI_IMPORT __declspec(dllimport)
#elif defined(CMPI_PLATFORM_LINUX_GENERIC_GNU) && (__GNUC__ >= 4)
# define CMPI_EXPORT __attribute__((visibility("default")))
# define CMPI_IMPORT __attribute__((visibility("default")))
#else
# define CMPI_EXPORT /* empty */
# define CMPI_IMPORT /* empty */
#endif

/* Define CMPI_EXTERN_C */
#ifdef __cplusplus
# define CMPI_EXTERN_C extern "C" CMPI_EXPORT
#else
# define CMPI_EXTERN_C CMPI_EXPORT
#endif

#endif
