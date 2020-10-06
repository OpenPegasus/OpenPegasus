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
    \file mm.h
    \brief Memory Managment system for remote providers (header file).

    \sa mm.h
    \sa native.h
*/
#ifndef _REMOTE_CMPI_TOOL_MM_H
# define _REMOTE_CMPI_TOOL_MM_H

# include "cmpir_common.h"

#ifdef PEGASUS_OS_ZOS
# include <stddef.h>
#endif

//! States cloned objects, i.e. memory that is not being tracked.
# define TOOL_MM_NO_ADD 0

//! States tracked memory objects.
# define TOOL_MM_ADD    1

//! The initial size of trackable memory pointers per thread.
/*!
    This size is increased by the same amount, once the limit is reached.
*/
# define MT_SIZE_STEP 100

typedef struct _managed_thread managed_thread;

//! Per-Thread management structure.
/*!
    This struct is returned using a global pthread_key_t and stores all
    allocated objects that are going to be freed, once the thread is flushed
    or dies.
*/
struct _managed_thread
{
    void *broker;
    void *ctx;
    unsigned size;        /*! < current maximum number of tracked
                              pointers */
    unsigned used;        /*! < currently tracked pointers */
    void **  objs;        /*! < array of tracked pointers */
};


PEGASUS_EXPORT void * PEGASUS_CMPIR_CDECL tool_mm_load_lib (
    const char * libname );

void tool_mm_flush ();
void * tool_mm_alloc ( int, size_t );
void * tool_mm_realloc ( void *, size_t );
int tool_mm_add (  void * );
PEGASUS_EXPORT void PEGASUS_CMPIR_CDECL tool_mm_set_broker (void * , void *);
int tool_mm_remove ( void * );
PEGASUS_EXPORT void * PEGASUS_CMPIR_CDECL tool_mm_get_broker(void **);

#endif
