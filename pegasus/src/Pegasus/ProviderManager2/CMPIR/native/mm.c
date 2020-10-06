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

#include "cmpir_common.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#ifdef PEGASUS_OS_TYPE_UNIX
# include <dlfcn.h>
# ifndef PEGASUS_OS_ZOS
#  include <error.h>
# endif
#elif defined PEGASUS_OS_TYPE_WINDOWS
# include <windows.h>
#endif

#include "mm.h"
#include "native.h"
#include "debug.h"

/**
    Exits the program with an error message in case the given
    condition holds.
*/
#define __ALLOC_ERROR(cond) \
    if (cond) \
    { \
        error_at_line ( -1, errno, __FILE__, __LINE__, \
            "unable to allocate requested memory." ); \
    }

/**
    flag to ensure MM is initialized only once
*/
#ifndef PEGASUS_PLATFORM_ZOS_ZSERIES
static int __once = 0;
#else
pthread_once_t __once = PTHREAD_ONCE_INIT;
#endif

/**
    global key to get access to thread-specific memory
    management data
*/
static CMPI_THREAD_KEY_TYPE __mm_key;

/****************************************************************************/

PEGASUS_EXPORT void * PEGASUS_CMPIR_CDECL tool_mm_load_lib (
    const char * libname )
{

    char filename[255];

    //PEGASUS_CMPIR_LIBTYPE appends libname with the lib extn based on os.
    sprintf ( filename, PEGASUS_CMPIR_LIBTYPE, libname );
    //invoke dlopen under unix and LoadLibrary under windows OS.
    return PEGASUS_CMPIR_LOADLIBRARY(filename, RTLD_LAZY );

}

/**
    Initializes the current thread by adding it to the memory
    management sytem.
*/
static managed_thread * __init_mt ()
{
    managed_thread * mt =
        (managed_thread *) calloc ( 1, sizeof ( managed_thread ) );

    __ALLOC_ERROR(!mt);

    mt->size   = MT_SIZE_STEP;
    mt->objs   = (void **) malloc ( MT_SIZE_STEP * sizeof ( void * ) );

    CMPI_BrokerExt_Ftab->setThreadSpecific( __mm_key, mt );

    return mt;
}

static void __flush_mt ( managed_thread * mt )
{
    TRACE_VERBOSE(("entered function."));
    TRACE_INFO(("freeing %d pointer(s).", mt->used ));

    while (mt->used)
    {
        free ( mt->objs[--mt->used] );
        mt->objs[mt->used] = NULL;
    };

    TRACE_VERBOSE(("leaving function."));
}

/**
    Cleans up a previously initialized thread once it
    dies/exits.
*/
static void __cleanup_mt ( void * ptr )
{
    managed_thread * mt = (managed_thread *) ptr;

    __flush_mt ( mt );

    free ( mt->objs );
    free ( mt );
}

/**
    Initializes the memory mangement system.
*/
static void __init_mm ()
{
    CMPI_BrokerExt_Ftab->createThreadKey( &__mm_key, __cleanup_mt );
}

/**
    Allocates zeroed memory and eventually puts it under memory
    mangement.

    Description:
        Calls calloc to get the requested block size, then adds
        it to the control system depending on add, defined as
        TOOL_MM_ADD and TOOL_MM_NO_ADD.
*/
void * tool_mm_alloc ( int add, size_t size )
{
    void * result = calloc ( 1, size );

    __ALLOC_ERROR(!result);

    if (add != TOOL_MM_NO_ADD)
    {
        tool_mm_add ( result );
    }
    return result;
}

/**
    Reallocates memory.

    Description:

        Reallocates oldptr to the new size, then checks if the
        new and old pointer are equal. If not and the old one is
        successfully removed from the managed_thread, this means
        that the new one has to be added as well, before
        returning it as result.

    The newly allocated memory is being returned as from the
    realloc() sys-call, no zeroing is performed as compared to
    tool_mm_alloc().
*/
void * tool_mm_realloc ( void * oldptr, size_t size )
{
    void * new = realloc ( oldptr, size );

    __ALLOC_ERROR(!new);

    if (oldptr != NULL &&
        tool_mm_remove ( oldptr ))
    {
        tool_mm_add ( new );
    }

    return new;
}

/**
    Adds ptr to the list of managed objects for the current
    thread.

    Description:

        First checks if the current thread is already under
        memory management control, eventually adds it. Then
        checks if ptr is already stored, if not finally adds it.
        Additionally the array size for stored void pointers may
        have to be enlarged by MT_SIZE_STEP.
*/
int tool_mm_add (  void * ptr )
{
    managed_thread * mt;

    CMPI_BrokerExt_Ftab->threadOnce( &__once, (void*)__init_mm );

    mt = (managed_thread *)
    CMPI_BrokerExt_Ftab->getThreadSpecific( __mm_key);

    if (mt == NULL)
    {
        mt = __init_mt ();
    }

    mt->objs[mt->used++] = ptr;

    if (mt->used == mt->size)
    {
        mt->size += MT_SIZE_STEP;
        mt->objs  = (void **) realloc ( mt->objs,
            mt->size * sizeof ( void * ) );

        __ALLOC_ERROR(!mt->objs);
    }

    return 1;
}
PEGASUS_EXPORT void PEGASUS_CMPIR_CDECL tool_mm_set_broker (
    void * broker,
    void * ctx )
{
    managed_thread * mt;

    CMPI_BrokerExt_Ftab->threadOnce( &__once, (void*)__init_mm );

    mt = (managed_thread *)
    CMPI_BrokerExt_Ftab->getThreadSpecific( __mm_key);
    if (mt == NULL)
    {
        mt = __init_mt ();
    }
    mt->broker=broker;
    mt->ctx=ctx;
}

PEGASUS_EXPORT void * PEGASUS_CMPIR_CDECL tool_mm_get_broker ( void **ctx )
{
    managed_thread * mt;

    CMPI_BrokerExt_Ftab->threadOnce( &__once, (void*)__init_mm );

    mt = (managed_thread *)
    CMPI_BrokerExt_Ftab->getThreadSpecific( __mm_key);

    if (mt)
    {
        if (ctx)
        {
            *ctx=mt->ctx;
        }
        return mt->broker;
    }

    return NULL;
}

/**
    Removes ptr from the list of managed objects for the current
    thread.

    Description:

        The removal is achieved by replacing the stored pointer
        with NULL, once found, as this does not disturb a later
        free() call.
*/
int tool_mm_remove ( void * ptr )
{
    managed_thread * mt;

    CMPI_BrokerExt_Ftab->threadOnce( &__once, (void*)__init_mm );

    mt = (managed_thread *)
    CMPI_BrokerExt_Ftab->getThreadSpecific( __mm_key );

    if (mt != NULL)
    {
        int i = mt->used;

        while (i--)
        {
            if (mt->objs[i] == ptr)
            {
                mt->objs[i] = NULL;
                return 1;
            }
        }
    }
    return 0;
}

void tool_mm_flush ()
{
    managed_thread * mt;

    CMPI_BrokerExt_Ftab->threadOnce( &__once, (void*)__init_mm );

    mt = (managed_thread *)
    CMPI_BrokerExt_Ftab->getThreadSpecific( __mm_key );

    if (mt != NULL)
    {
        __flush_mt ( mt );
    }
}

/*** Local Variables:  ***/
/*** mode: C           ***/
/*** c-basic-offset: 8 ***/
/*** End:              ***/
