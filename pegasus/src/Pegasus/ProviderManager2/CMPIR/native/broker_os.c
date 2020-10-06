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
#include "tool.h"
#include "native.h"
#include "debug.h"
#include <Pegasus/Provider/CMPI/cmpipl.h>
#include <errno.h>
#include <stdlib.h>


#if defined PEGASUS_OS_TYPE_WINDOWS
//
// PLEASE DO NOT REMOVE THE DEFINITION OF FD_SETSIZE!
//
# ifndef FD_SETSIZE
#  define FD_SETSIZE 1024
# endif
# include <winsock2.h>
# include <process.h>
# include <winbase.h>
# include <sys/types.h>
# include <sys/timeb.h>
# include <winuser.h>
#else
# include <pthread.h>
#endif

PEGASUS_EXPORT CMPI_MUTEX_TYPE pegthreadOnceMutex = NULL;

static char *resolveFileName (const char *filename)
{
    char dlName[1024];
#if defined(CMPI_PLATFORM_WIN32_IX86_MSVC)
    strcpy(dlName,filename);
    strcat(dlName,".dll");
#elif defined(CMPI_PLATFORM_HPUX_PARISC_ACC)
    strcpy(dlName,"lib");
    strcat(dlName,filename);
    strcat(dlName,".sl");
#elif defined(CMPI_OS_VMS)
    strcpy(dlName,"/wbem_lib");
    strcat(dlName,filename);
    strcat(dlName,".exe");
#elif defined(CMPI_OS_DARWIN)
    strcpy(dlName,"lib");
    strcat(dlName,filename);
    strcat(dlName,".dylib");
#else
    strcpy(dlName,"lib");
    strcat(dlName,filename);
    strcat(dlName,".so");
#endif

    return strdup(dlName);
}

/*
    We need to have wrapper for newthread. When we invoke newThread from
    current thread, new thread is not managed by memeory management and may
    not have CMPIBroker and CMPIContext assosiated with that.
    Add them in wrapper. -V 5245
*/
struct startWrapperArg
{
    void *(CMPI_THREAD_CDECL * start) (void *);
    void *arg;
    CMPIBroker *broker;
    CMPIContext *ctx;
};

typedef struct startWrapperArg startWrapperArg;

void *_start_wrapper(void *arg_)
{
    startWrapperArg *arg = (startWrapperArg *) arg_;
    void* return_value;

    return_value = (*arg->start) (arg->arg);
    free (arg);

    return return_value;
}

static CMPI_THREAD_TYPE newThread(
    CMPI_THREAD_RETURN (CMPI_THREAD_CDECL *start )(void *),
    void *parm, int detached)
{
#if defined(CMPI_PLATFORM_LINUX_GENERIC_GNU)
    pthread_t t;
    pthread_attr_t tattr;
    startWrapperArg *wparm =
        (startWrapperArg*)malloc ( sizeof ( struct startWrapperArg ) );
    wparm->start = start;
    wparm->arg = parm;
    wparm->broker = NULL;
    wparm->ctx = NULL;
    if (detached)
    {
        pthread_attr_init(&tattr);
        pthread_attr_setdetachstate(&tattr, PTHREAD_CREATE_DETACHED);
        pthread_create(&t, &tattr, (void *(*)(void *)) _start_wrapper, wparm);
    }
    else
    {
        pthread_create(&t, NULL, (void *(*)(void *)) _start_wrapper, wparm);
    }
    return(CMPI_THREAD_TYPE)t;

#elif defined(CMPI_PLATFORM_ZOS_ZSERIES_IBM)

# define PTHREAD_CREATE_DETACHED 0

    pthread_t t;
    pthread_attr_t tattr;
    startWrapperArg *wparm =
        (startWrapperArg*)malloc ( sizeof ( struct startWrapperArg ) );
    wparm->start = start;
    wparm->arg = parm;
    wparm->broker = NULL;
    wparm->ctx = NULL;
    if (detached)
    {
        pthread_attr_init(&tattr);
        pthread_attr_setdetachstate(&tattr, PTHREAD_CREATE_DETACHED);
        pthread_create(&t, &tattr, (void *(*)(void *)) _start_wrapper, wparm);
    }
    else
    {
        pthread_create(&t, NULL, (void *(*)(void *)) _start_wrapper, wparm);
    }

    return(CMPI_THREAD_TYPE) t;
#elif defined PEGASUS_OS_TYPE_WINDOWS

    unsigned threadid = 0;
    HANDLE hThread;
    startWrapperArg *wparm =
        (startWrapperArg*)malloc ( sizeof ( struct startWrapperArg ) );
    wparm->start = start;
    wparm->arg = parm;
    wparm->broker = NULL;
    wparm->ctx = NULL;
    hThread = (HANDLE)_beginthread( (void *)_start_wrapper, 0, wparm);
    return(CMPI_THREAD_TYPE) hThread;
#else
# error Platform for Remote CMPI daemon not yet supported
#endif
}

static int joinThread (
    CMPI_THREAD_TYPE hdlThread,
    CMPI_THREAD_RETURN *returnCode)
{
#if defined(PEGASUS_OS_TYPE_UNIX)
   return pthread_join((pthread_t)hdlThread, returnCode);
#elif defined(PEGASUS_OS_TYPE_WINDOWS)
   WaitForSingleObject(hdlThread,INFINITE);
   GetExitCodeThread(hdlThread,returnCode);
   return CloseHandle(hdlThread);
#else
   #error Platform for Remote CMPI daemon not yet supported
#endif
}

static int exitThread (CMPI_THREAD_RETURN returnCode)
{

#if defined(PEGASUS_OS_TYPE_UNIX)
    pthread_exit (returnCode);
    return 0;
#elif defined(PEGASUS_OS_TYPE_WINDOWS)
    returnCode=0;
    _endthread();
    return 0;  /* should never reach here */
#else
   #error Platform for Remote CMPI daemon not yet supported
#endif
}

static int cancelThread (CMPI_THREAD_TYPE hdlThread)
{
#if defined(PEGASUS_OS_TYPE_UNIX)
    return pthread_cancel ((pthread_t)hdlThread);
#elif defined(PEGASUS_OS_TYPE_WINDOWS)
    return TerminateThread(hdlThread,0);
#else
   #error Platform for Remote CMPI daemon not yet supported
#endif
}


#ifdef CMPI_PLATFORM_ZOS_ZSERIES_IBM
static int threadOnce (pthread_once_t *once, void (*init)(void))
{
    return pthread_once ( once, init );
}
#else
static int threadOnce (int *once, void (*init)(void))
{
# if defined(CMPI_PLATFORM_LINUX_GENERIC_GNU)
    return pthread_once ( once, init );
# elif defined PEGASUS_OS_TYPE_WINDOWS
    if (*once==0)
    {
        WaitForSingleObject(pegthreadOnceMutex,INFINITE);
        if ((*once)++ == 0)
        {
            (init)();
        }
        ReleaseMutex(pegthreadOnceMutex);
    }

    return *once;
# else
#  error Platform for Remote CMPI daemon not yet supported
# endif
}
#endif /* endif of #ifdef CMPI_PLATFORM_ZOS_ZSERIES_IBM */

static int createThreadKey(CMPI_THREAD_KEY_TYPE *key, void (*cleanup)(void*))
{
#if defined(CMPI_PLATFORM_LINUX_GENERIC_GNU)
    return pthread_key_create ((pthread_key_t*)key , cleanup );
#elif defined(CMPI_PLATFORM_ZOS_ZSERIES_IBM)
    return pthread_key_create (key , cleanup );
#elif defined PEGASUS_OS_TYPE_WINDOWS
    CMPI_THREAD_KEY_TYPE  key1 = TlsAlloc();
    key = (CMPI_THREAD_KEY_TYPE*) key1;
    return TlsSetValue(key1,cleanup);
#else
# error Platform for Remote CMPI daemon not yet supported
#endif
}

static int destroyThreadKey(CMPI_THREAD_KEY_TYPE key)
{
#if defined(PEGASUS_OS_TYPE_UNIX)
    return pthread_key_delete (key);
#elif defined PEGASUS_OS_TYPE_WINDOWS
    return TlsFree(key);
#else
# error Platform for Remote CMPI daemon not yet supported
#endif
}

static void *getThreadSpecific(CMPI_THREAD_KEY_TYPE key)
{
#if defined(CMPI_PLATFORM_LINUX_GENERIC_GNU)
    return pthread_getspecific(key);
#elif defined(CMPI_PLATFORM_ZOS_ZSERIES_IBM)
    return(void*) pthread_getspecific_d8_np(key);
#elif defined PEGASUS_OS_TYPE_WINDOWS
    return TlsGetValue(key);
#else
# error Platform for Remote CMPI daemon not yet supported
#endif
}

static int setThreadSpecific(CMPI_THREAD_KEY_TYPE key, void * value)
{
#if defined(CMPI_PLATFORM_LINUX_GENERIC_GNU)
    return pthread_setspecific(key,value);
#elif defined(CMPI_PLATFORM_ZOS_ZSERIES_IBM)
    return pthread_setspecific(key,value);
#elif defined PEGASUS_OS_TYPE_WINDOWS
    return TlsSetValue(key,value);
#else
# error Platform for Remote CMPI daemon not yet supported
#endif
}

static CMPI_MUTEX_TYPE newMutex (int opt)
{
#if defined(CMPI_PLATFORM_LINUX_GENERIC_GNU)
    static pthread_mutex_t tmpl=PTHREAD_MUTEX_INITIALIZER;
    pthread_mutex_t *m=calloc(1,sizeof(pthread_mutex_t));
    *m=tmpl;
    return m;
#elif defined(CMPI_PLATFORM_ZOS_ZSERIES_IBM)
    // pthread_mutex_t init_m;
    pthread_mutex_t *new_mutex =
        (pthread_mutex_t*) calloc(1,sizeof(pthread_mutex_t));
    // PTHREAD_MUTEX_INITIALIZER;
    errno = 0;

    if (pthread_mutex_init(new_mutex, NULL) != 0)
    {
        TRACE_CRITICAL(("pthread_mutex_init failed: %s",strerror(errno)));
        return NULL;
    }
    return(CMPI_MUTEX_TYPE) new_mutex;
#elif defined PEGASUS_OS_TYPE_WINDOWS
    HANDLE new_mutex;
    new_mutex = CreateMutex(NULL,FALSE,NULL);
    if (new_mutex == NULL)
    {
        TRACE_CRITICAL(("CreateMutex failed: Error code %d",
            GetLastError()));
        return NULL;
    }
    return(CMPI_MUTEX_TYPE) new_mutex;
#else
# error Platform for Remote CMPI daemon not yet supported
#endif
}

static void destroyMutex (CMPI_MUTEX_TYPE m)
{
#if defined(CMPI_PLATFORM_LINUX_GENERIC_GNU)
    free(m);
#elif defined(CMPI_PLATFORM_ZOS_ZSERIES_IBM)
    if (pthread_mutex_destroy((pthread_mutex_t *) m)!=0)
    {
        TRACE_CRITICAL(("pthread_mutex_destroy failed: %s",strerror(errno)));
    }
    free((pthread_mutex_t*) m);
#elif defined PEGASUS_OS_TYPE_WINDOWS
    WaitForSingleObject(m, INFINITE);
    CloseHandle(m);
#else
# error Platform for Remote CMPI daemon not yet supported
#endif
}

static void lockMutex (CMPI_MUTEX_TYPE m)
{
#if defined(CMPI_PLATFORM_LINUX_GENERIC_GNU)
    pthread_mutex_lock ((pthread_mutex_t *)m );
#elif defined(CMPI_PLATFORM_ZOS_ZSERIES_IBM)
    pthread_mutex_lock ((pthread_mutex_t *)m );
#elif defined PEGASUS_OS_TYPE_WINDOWS
    WaitForSingleObject(m,INFINITE);
#else
# error Platform for Remote CMPI daemon not yet supported
#endif
}

static void unlockMutex (CMPI_MUTEX_TYPE m)
{
#if defined(CMPI_PLATFORM_LINUX_GENERIC_GNU)
    pthread_mutex_unlock ((pthread_mutex_t *)m );
#elif defined(CMPI_PLATFORM_ZOS_ZSERIES_IBM)
    pthread_mutex_unlock ((pthread_mutex_t *)m );
#elif defined PEGASUS_OS_TYPE_WINDOWS
    ReleaseMutex(m);
#else
# error Platform for Remote CMPI daemon not yet supported
#endif
}

static CMPI_COND_TYPE newCondition (int opt)
{
#if defined(CMPI_PLATFORM_LINUX_GENERIC_GNU)
    pthread_cond_t tmpl=PTHREAD_COND_INITIALIZER;
    pthread_cond_t *c=calloc(1,sizeof(pthread_cond_t));
    *c=tmpl;
    return c;
#elif defined(CMPI_PLATFORM_ZOS_ZSERIES_IBM)
    pthread_cond_t *c=(pthread_cond_t*) calloc(1,sizeof(pthread_cond_t));;
    if (pthread_cond_init(c, NULL) != 0)
    {
        TRACE_CRITICAL(("pthread_cond_init failed: %s",strerror(errno)));
    }
    return(CMPI_COND_TYPE) c;
#elif defined PEGASUS_OS_TYPE_WINDOWS
    HANDLE c;
    c = CreateEvent( NULL, FALSE, FALSE, NULL );
    if (c == NULL)
    {
        TRACE_CRITICAL(("CreateEvent failed: Error code: %d",
            GetLastError()));
    }
    return c;
#else
# error Platform for Remote CMPI daemon not yet supported
#endif
}

static void destroyCondition (CMPI_COND_TYPE c)
{
#if defined(CMPI_PLATFORM_LINUX_GENERIC_GNU)
    free(c);
#elif defined(CMPI_PLATFORM_ZOS_ZSERIES_IBM)
    if (pthread_cond_destroy((pthread_cond_t*) c) != 0)
    {
        TRACE_CRITICAL(("pthread_cond_destroy failed: %s",strerror(errno)));
    }
    free((pthread_cond_t*) c);
#elif defined PEGASUS_OS_TYPE_WINDOWS
    CloseHandle(c);
#else
# error Platform for Remote CMPI daemon not yet supported
#endif
}

static int timedCondWait(
    CMPI_COND_TYPE c,
    CMPI_MUTEX_TYPE m,
    struct timespec *wait)
{
#if defined(CMPI_PLATFORM_LINUX_GENERIC_GNU)
    return pthread_cond_timedwait(
        (pthread_cond_t*)c,
        (pthread_mutex_t*)m,
        wait);
#elif defined(CMPI_PLATFORM_ZOS_ZSERIES_IBM)
    return pthread_cond_timedwait(
        (pthread_cond_t*)c,
        (pthread_mutex_t*)m,
        wait);
#elif defined PEGASUS_OS_TYPE_WINDOWS

    int rc;
    int msec;
    struct timespec next=*wait;
    struct timeval
    {
        long tv_sec;
        long tv_usec;
    }now;
    //struct _timeb timebuffer;

    /*  this is not truely mapping to pthread_timed_wait
        but will work for the time beeing
    */

    gettimeofday(&now, NULL);

    if (next.tv_nsec>1000000000)
    {
        next.tv_sec+=next.tv_nsec/1000000000;
        next.tv_nsec=next.tv_nsec%1000000000;
    }
    msec=(next.tv_sec-now.tv_sec)*1000;
    msec+=(next.tv_nsec/1000000)-(now.tv_usec/1000);

    if ((rc = SignalObjectAndWait(m,c,msec,FALSE))==WAIT_OBJECT_0)
    {
        if (WaitForSingleObject(m,INFINITE)==WAIT_OBJECT_0)
        {
            return 0;
        }
    }
    return 1;
#else
# error Platform for Remote CMPI daemon not yet supported
#endif
}

static int condWait(CMPI_COND_TYPE c, CMPI_MUTEX_TYPE m)
{
#if defined(CMPI_PLATFORM_LINUX_GENERIC_GNU)
    return pthread_cond_wait((pthread_cond_t*)c, (pthread_mutex_t*)m);
#elif defined(CMPI_PLATFORM_ZOS_ZSERIES_IBM)
    return pthread_cond_wait((pthread_cond_t*)c, (pthread_mutex_t*)m);
#elif defined PEGASUS_OS_TYPE_WINDOWS
    if (SignalObjectAndWait(m,c,INFINITE,FALSE)==WAIT_OBJECT_0)
    {
        if (WaitForSingleObject(m,INFINITE)==WAIT_OBJECT_0)
        {
            return 0;
        }
    }
    return 1;
#else
# error Platform for Remote CMPI daemon not yet supported
#endif
}

static int threadSleep(CMPIUint32 msec)
{
#if defined(PEGASUS_HAVE_NANOSLEEP)

    struct timespec wait,remwait;
    wait.tv_sec = msec / 1000;
    wait.tv_nsec = (msec % 1000) * 1000000;

    while ((nanosleep(&wait, &remwait) == -1) && (errno == EINTR))
    {
        wait.tv_sec = remwait.tv_sec;
        wait.tv_nsec = remwait.tv_nsec;
    }

#elif defined(PEGASUS_PLATFORM_WIN64_IA64_MSVC) || \
      defined(PEGASUS_PLATFORM_WIN64_X86_64_MSVC) || \
      defined(PEGASUS_PLATFORM_WIN32_IX86_MSVC)
    struct _timeb end, now;

    if (msec == 0)
    {
        Sleep(0);
        return 0;
    }


    _ftime( &end );
    end.time += (msec / 1000);
    msec -= (msec / 1000);
    end.millitm += msec;

    do
    {
        Sleep(0);
        _ftime(&now);
    }
    while (end.millitm > now.millitm && end.time >= now.time);

#elif defined(PEGASUS_OS_ZOS)
    int seconds;
    if (msec < 1000)
    {
        usleep(msec*1000);
    }
    else
    {
        // sleep for loop seconds
        sleep(msec / 1000);
        // Usleep the remaining micro seconds
        usleep( (msec*1000) % 1000000 );
    }
#elif defined(PEGASUS_OS_VMS)

    sleep(msec / 1000);

#endif
    return 0;
}

static CMPIBrokerExtFT brokerExt_FT={
    CMPICurrentVersion,
    resolveFileName,
    newThread,
    joinThread,
    exitThread,
    cancelThread,
    threadSleep,
    threadOnce,

    createThreadKey,
    destroyThreadKey,
    getThreadSpecific,
    setThreadSpecific,

    newMutex,
    destroyMutex,
    lockMutex,
    unlockMutex,

    newCondition,
    destroyCondition,
    condWait,
    timedCondWait,
    NULL                       // Signal not supported yet
};

PEGASUS_EXPORT CMPIBrokerExtFT *CMPI_BrokerExt_Ftab = &brokerExt_FT;

