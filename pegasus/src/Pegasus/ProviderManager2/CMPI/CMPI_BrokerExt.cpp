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

#include "CMPI_Version.h"

#include <Pegasus/ProviderManager2/CMPI/CMPIProvider.h>
#include "CMPI_Object.h"
#include "CMPI_ThreadContext.h"
#include "CMPI_Broker.h"
#include "CMPI_Ftabs.h"
#include "CMPI_String.h"
#include "CMPI_SelectExp.h"

#include <Pegasus/Common/CIMName.h>
#include <Pegasus/Common/Time.h>
#include <Pegasus/Common/CIMPropertyList.h>
#include <Pegasus/ProviderManager2/ProviderManager.h>
#include <Pegasus/Common/Thread.h>
#include <Pegasus/Common/Tracer.h>
#include <Pegasus/Common/AutoPtr.h>
#include <Pegasus/Common/Condition.h>

#if defined (CMPI_VER_85)
# include <Pegasus/Common/MessageLoader.h>
#endif

#include <stdarg.h>
#include <string.h>

#if defined(CMPI_PLATFORM_WIN32_IX86_MSVC)
# include <sys/timeb.h>
#endif

PEGASUS_NAMESPACE_BEGIN

class ConditionWithMutex
{
public:
    ConditionWithMutex() : _mutex(Mutex::NON_RECURSIVE)
    {
    }
    ~ConditionWithMutex()
    {
    }
    void signal()
    {
        _cond.signal();
    }
    void wait()
    {
        _cond.wait(_mutex);
    }

private:
    Mutex _mutex;
    Condition _cond;
};

extern "C"
{
    struct thrd_data
    {
        CMPI_THREAD_RETURN(CMPI_THREAD_CDECL*pgm)(void*);
        void *parm;
        CMPIProvider *provider;
    };
}

static ThreadReturnType PEGASUS_THREAD_CDECL start_driver(void *parm)
{
    PEG_METHOD_ENTER(
        TRC_CMPIPROVIDERINTERFACE,
        "CMPI_BrokerExt:start_driver()");
    ThreadReturnType rc;
    Thread* my_thread = (Thread*)parm;
    thrd_data *pp = (thrd_data*)my_thread->get_parm();
    thrd_data data=*pp;
    Thread::setCurrent(my_thread);

    delete pp;
    rc = (ThreadReturnType)(data.pgm)(data.parm);

    // Remove the thread from the watch-list (and clean it up) if this (self)
    // was created in detached mode. Don't delete the thread if this thread
    // was not created in detached mode because it is possible that join()
    // my be called on this thread later. This thread object is deleted when
    // joinThread() is called later. If joinThread() is not called memory is
    // leaked which is true as per Pthread semantics and as defined by CMPI.
    if (!my_thread->isDetached())
    {
        PEG_TRACE((TRC_PROVIDERMANAGER,Tracer::LEVEL4,
            "Thread %s is not detached, not removed from provider watch-list",
            Threads::id().buffer));
    }
    else
    {
        data.provider->removeThreadFromWatch(my_thread);
        PEG_TRACE((TRC_PROVIDERMANAGER,Tracer::LEVEL4,
            "Thread %s is detached and removed from provider watch-list",
            Threads::id().buffer));
    }
    PEG_METHOD_EXIT();
    return rc;
}

extern "C"
{
    static char *resolveFileName(const char *filename)
    {
        String pn = ProviderManager::_resolvePhysicalName(filename);
        CString n = pn.getCString();
        return(strdup((const char*)n));
    }

    static CMPI_THREAD_TYPE newThread
        (CMPI_THREAD_RETURN(CMPI_THREAD_CDECL *start)(void *),
        void *parm,
        int detached)
    {
        PEG_METHOD_ENTER(
            TRC_CMPIPROVIDERINTERFACE,
            "CMPI_BrokerExt:newThread()");
        const CMPIBroker *brk = CM_BROKER;
        const CMPI_Broker *broker = (CMPI_Broker*)brk;

        AutoPtr<thrd_data> data(new thrd_data());
        data->pgm = (CMPI_THREAD_RETURN (CMPI_THREAD_CDECL *)(void*))start;
        data->parm = parm;
        data->provider = broker->provider;
        Thread *t = new Thread(start_driver, data.get(), detached == 1);

        broker->provider->addThreadToWatch(t);
        data.release();

        if (t->run() != PEGASUS_THREAD_OK)
        {
            PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL1, \
                "Could not allocate provider thread (%p) for %s provider.",
                t, (const char *)broker->name.getCString()));
            broker->provider->removeThreadFromWatch(t);
            t = NULL;
        }
        else 
        {
            PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL3,
                "Started provider thread (%p) for %s.",
                t, (const char *)broker->name.getCString()));
        }
        PEG_METHOD_EXIT();
        return (CMPI_THREAD_TYPE)t;
    }

    static int joinThread(
        CMPI_THREAD_TYPE thread,
        CMPI_THREAD_RETURN *returnCode)
    {
        const CMPIBroker *brk = CM_BROKER;
        const CMPI_Broker *broker = (CMPI_Broker*)brk;
        ((Thread*)thread)->join();
        if (returnCode)
        {
            *returnCode = (CMPI_THREAD_RETURN)((Thread*)thread)->get_exit();
        }
        broker->provider->removeThreadFromWatch((Thread*)thread);
        return 0;
    }

    static int exitThread(CMPI_THREAD_RETURN return_code)
    {
        Thread::getCurrent()->exit_self((ThreadReturnType)return_code);
        return 0;
    }

    static int cancelThread(CMPI_THREAD_TYPE thread)
    {
        ((Thread*)thread)->cancel();
        return 0;
    }

    static int threadSleep(CMPIUint32 msec)
    {
        Threads::sleep(msec);
        return 0;
    }

    static int threadOnce (int *once, void (*init)(void))
    {
        PEG_METHOD_ENTER(
            TRC_CMPIPROVIDERINTERFACE,
            "CMPI_BrokerExt:threadOnce()");
        if (*once==0)
        {
            *once = 1;
            (init)();
        }
        PEG_METHOD_EXIT();
        return *once;
    }

    static int createThreadKey(
        CMPI_THREAD_KEY_TYPE *key,
        void (*cleanup)(void*))
    {
        return TSDKey::create((TSDKeyType*)key);
    }

    static int destroyThreadKey(CMPI_THREAD_KEY_TYPE key)
    {
        return TSDKey::destroy(key);
    }

    static void *getThreadSpecific(CMPI_THREAD_KEY_TYPE key)
    {
        return TSDKey::get_thread_specific(key);
    }

    static  int setThreadSpecific(CMPI_THREAD_KEY_TYPE key, void *value)
    {
        return TSDKey::set_thread_specific(key,value);
    }

    static CMPI_MUTEX_TYPE newMutex(int opt)
    {
        Mutex *m = new Mutex();
        return m;
    }

    static void destroyMutex(CMPI_MUTEX_TYPE m)
    {
        delete ((Mutex*)m);
    }

    static void lockMutex(CMPI_MUTEX_TYPE m)
    {
        ((Mutex*)m)->lock();
    }

    static void unlockMutex(CMPI_MUTEX_TYPE m)
    {
        ((Mutex*)m)->unlock();
    }

    static CMPI_COND_TYPE newCondition(int opt)
    {
        PEG_METHOD_ENTER(
            TRC_CMPIPROVIDERINTERFACE,
            "CMPI_BrokerExt:newCondition()");
        ConditionWithMutex *c = new ConditionWithMutex();
        PEG_METHOD_EXIT();
        return c;
    }

    static void destroyCondition(CMPI_COND_TYPE c)
    {
        delete (ConditionWithMutex*)c;
    }

    static int condWait(CMPI_COND_TYPE c, CMPI_MUTEX_TYPE m)
    {
        // need to take care of mutex
        ((ConditionWithMutex*)c)->wait();
        return 0;
    }

    static int timedCondWait(
        CMPI_COND_TYPE c,
        CMPI_MUTEX_TYPE m,
        struct timespec *wait)
    {
        PEG_METHOD_ENTER(
            TRC_CMPIPROVIDERINTERFACE,
            "CMPI_BrokerExt:timedCondWait()");
        int msec;
        struct timespec next = *wait;
#if defined CMPI_PLATFORM_WIN32_IX86_MSVC
        struct timeval
        {
            long tv_sec;
            long tv_usec;
        }now;
        struct _timeb timebuffer;
#endif

        /* this is not truely mapping to pthread_timed_wait
           but will work for the time beeing
        */
#if defined CMPI_PLATFORM_WIN32_IX86_MSVC
        _ftime(&timebuffer);
        now.tv_sec = (long)timebuffer.time;
        now.tv_usec = timebuffer.millitm*1000;
#else
        struct timeval now;
        Time::gettimeofday(&now);
#endif

        if (next.tv_nsec>1000000000)
        {
            next.tv_sec+=next.tv_nsec/1000000000;
            next.tv_nsec=next.tv_nsec%1000000000;
        }
        msec = (next.tv_sec-now.tv_sec)*1000;
        msec += (next.tv_nsec/1000000)-(now.tv_usec/1000);

        Threads::sleep(msec);
        PEG_METHOD_EXIT();
        return 0;
    }

    static int signalCondition(CMPI_COND_TYPE cond)
    {
        ((ConditionWithMutex*)cond)->signal();
        return 0;
    }

}

static CMPIBrokerExtFT brokerExt_FT =
{
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
    signalCondition                       // Signal not supported yet
};

CMPIBrokerExtFT *CMPI_BrokerExt_Ftab = &brokerExt_FT;


PEGASUS_NAMESPACE_END



