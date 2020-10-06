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

#ifndef Pegasus_Threads_h
#define Pegasus_Threads_h

#include <cstring>
#include <cstdio>
#include <cstring>
#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/Linkage.h>

// ATTN: can we consolidate these someplace?

#ifdef PEGASUS_OS_ZOS
# include <sched.h>
#endif
#if defined(PEGASUS_HAVE_PTHREADS)
# include <pthread.h>
# include <errno.h>
# include <sys/time.h>
#elif defined(PEGASUS_HAVE_WINDOWS_THREADS)
# include <windows.h>
# include <process.h>
#else
# error "<Pegasus/Common/Threads.h>: not implemented"
#endif

#if defined(PEGASUS_OS_SOLARIS)
# include <string.h>
# include <stdio.h>
#endif

PEGASUS_NAMESPACE_BEGIN

//==============================================================================
//
// PEGASUS_THREAD_CDECL
//
//==============================================================================

#if defined(PEGASUS_OS_TYPE_WINDOWS)
# define PEGASUS_THREAD_CDECL __stdcall
#else
# define PEGASUS_THREAD_CDECL /* empty */
#endif

//==============================================================================
//
// ThreadId
//
//==============================================================================

struct ThreadId
{
    // The character representation of a uint64 requires 22 bytes including the
    // null terminator.
    char buffer[22];
};

//==============================================================================
//
// ThreadType
//
//==============================================================================

#if defined(PEGASUS_HAVE_PTHREADS)
struct ThreadType
{
    ThreadType()
    {
        memset(&thread, 0, sizeof(thread));
    }

    ThreadType(pthread_t thread_) : thread(thread_)
    {
    }

    pthread_t thread;
};
#endif /* PEGASUS_HAVE_PTHREADS */

#if defined(PEGASUS_HAVE_WINDOWS_THREADS)
struct ThreadType
{
    ThreadType() : handle(NULL)
    {
    }

    HANDLE handle;
};
#endif /* PEGASUS_HAVE_WINDOWS_THREADS */

//==============================================================================
//
// ThreadReturnType
//
//==============================================================================

#if defined(PEGASUS_HAVE_PTHREADS)
typedef void* ThreadReturnType;
#endif

#if defined(PEGASUS_HAVE_WINDOWS_THREADS)
typedef unsigned ThreadReturnType;
#endif

//==============================================================================
//
// ThreadHandle
//
//==============================================================================

#if defined(PEGASUS_HAVE_PTHREADS)
struct ThreadHandle
{
    ThreadType thid;
};
#elif defined(PEGASUS_HAVE_WINDOWS_THREADS)
struct ThreadHandle
{
    ThreadType thid;
};
#endif

//==============================================================================
//
// Threads
//
//==============================================================================

class PEGASUS_COMMON_LINKAGE Threads
{
public:

    enum Type { DETACHED, JOINABLE };

    static int create(
        ThreadType& thread,
        Type type,
        void* (*start)(void*),
        void* arg);

    static ThreadType self();

    static bool equal(ThreadType x, ThreadType y);

    static void exit(ThreadReturnType rc);

    static void yield();

    static void sleep(int msec);

    static ThreadId id(const ThreadType& x = Threads::self());

    static bool null(const ThreadType& x = Threads::self());

    static void clear(ThreadType& x);
};

//==============================================================================
//
// POSIX Threads Implementation
//
//==============================================================================

#if defined(PEGASUS_HAVE_PTHREADS)

inline bool Threads::equal(ThreadType x, ThreadType y)
{
    return pthread_equal(x.thread, y.thread);
}

inline void Threads::exit(ThreadReturnType rc)
{
    // NOTE: pthread_exit exhibits unusual behavior on RHEL 3 U2, as
    // documented in Bugzilla 3836.  Where feasible, it may be advantageous
    // to avoid using this function.
    pthread_exit(rc);
}

inline void Threads::yield()
{
#ifdef PEGASUS_OS_LINUX
    pthread_yield();
#else
    sched_yield();
#endif
}

inline ThreadId Threads::id(const ThreadType& x)
{
    ThreadId tid = { { 0 } };

#if defined(PEGASUS_OS_ZOS)
    const char* s = x.thread.__;
    sprintf(tid.buffer, "%X%X%X%X%X%X%X%X",
        s[0], s[1], s[2], s[3], s[4], s[5], s[6], s[7]);
#else
    sprintf(tid.buffer, "%" PEGASUS_64BIT_CONVERSION_WIDTH "u",
        Uint64(x.thread));
#endif

    return tid;
}

inline bool Threads::null(const ThreadType& x)
{
#if defined(PEGASUS_OS_ZOS)
    Uint64 tmp;
    memcpy(&tmp, x.thread.__, sizeof(Uint64));
    return tmp == 0;
#else
    return x.thread == 0;
#endif
}

inline void Threads::clear(ThreadType& x)
{
    memset(&x, 0, sizeof(x));
}

#endif /* defined(PEGASUS_HAVE_PTHREADS) */

//==============================================================================
//
// Windows Threads Implementation
//
//==============================================================================

#if defined(PEGASUS_HAVE_WINDOWS_THREADS)

inline ThreadType Threads::self()
{
    ThreadType tt;
    tt.handle = GetCurrentThread();
    return tt;
}

inline bool Threads::equal(ThreadType x, ThreadType y)
{
    return x.handle == y.handle;
}

inline void Threads::exit(ThreadReturnType rc)
{
    _endthreadex(rc);
}

inline void Threads::yield()
{
    Sleep(0);
}

inline ThreadId Threads::id(const ThreadType& x)
{
    ThreadId tmp;

    sprintf(tmp.buffer, "%" PEGASUS_64BIT_CONVERSION_WIDTH "u",
        Uint64(x.handle));

    return tmp;
}

inline bool Threads::null(const ThreadType& x)
{
    return x.handle == NULL;
}

inline void Threads::clear(ThreadType& x)
{
    x.handle = NULL;
}

#endif /* defined(PEGASUS_HAVE_WINDOWS_THREADS) */

PEGASUS_NAMESPACE_END

#endif /* Pegasus_Threads_h */
