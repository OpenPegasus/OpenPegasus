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

#include "Thread.h"
#include <errno.h>
#include <exception>
#include <Pegasus/Common/Tracer.h>
#include <Pegasus/Common/AutoPtr.h>
#include "Time.h"

PEGASUS_USING_STD;

PEGASUS_NAMESPACE_BEGIN

//==============================================================================
//
// POSIX Threads Implementation:
//
//==============================================================================

#if defined(PEGASUS_HAVE_PTHREADS)

struct StartWrapperArg
{
    void *(PEGASUS_THREAD_CDECL * start) (void *);
    void *arg;
};

extern "C" void *_start_wrapper(void *arg_)
{
    // Clean up dynamic memory now to prevent a leak if the thread is canceled.
    StartWrapperArg arg;
    arg.start = ((StartWrapperArg *) arg_)->start;
    arg.arg = ((StartWrapperArg *) arg_)->arg;
    delete (StartWrapperArg *) arg_;

    // establish cancelability of the thread
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);

    void *return_value = (*arg.start) (arg.arg);

    return return_value;
}

void Thread::cancel()
{
    pthread_cancel(_handle.thid.thread);
}

void Thread::thread_switch()
{
#if defined(PEGASUS_OS_ZOS)
    pthread_yield(NULL);
#else
    sched_yield();
#endif
}

void Thread::sleep(Uint32 msec)
{
    Threads::sleep(msec);
}

void Thread::join()
{
    if (!_is_detached && !Threads::null(_handle.thid))
        pthread_join(_handle.thid.thread, &_exit_code);

    Threads::clear(_handle.thid);
}

void Thread::detach()
{
    _is_detached = true;
#if defined(PEGASUS_OS_ZOS)
    pthread_t  thread_id=_handle.thid.thread;
    pthread_detach(&thread_id);
#else
    pthread_detach(_handle.thid.thread);
#endif
}

ThreadStatus Thread::run()
{
    StartWrapperArg *arg = new StartWrapperArg;
    arg->start = _start;
    arg->arg = this;

    Threads::Type type = _is_detached ? Threads::DETACHED : Threads::JOINABLE;
    int rc = Threads::create(_handle.thid, type, _start_wrapper, arg);

    // On Linux distributions released prior 2005, the implementation of
    // Native POSIX Thread Library returns ENOMEM instead of EAGAIN when
    // there
    // are no insufficient memory.  Hence we are checking for both.  See bug
    // 386.

    if (rc == -1)
        rc = errno;
    if ((rc == EAGAIN) || (rc == ENOMEM))
    {
        Threads::clear(_handle.thid);
        delete arg;
        return PEGASUS_THREAD_INSUFFICIENT_RESOURCES;
    }
    else if (rc != 0)
    {
        Threads::clear(_handle.thid);
        delete arg;
        return PEGASUS_THREAD_SETUP_FAILURE;
    }
    return PEGASUS_THREAD_OK;
}

Thread::Thread(
    ThreadReturnType(PEGASUS_THREAD_CDECL* start) (void*),
    void* parameter,
    Boolean detached)
    : _is_detached(detached),
      _start(start),
      _cleanup(),
      _thread_parm(parameter),
      _exit_code(0)
{
    Threads::clear(_handle.thid);
    memset(_tsd, 0, sizeof(_tsd));
}

Thread::~Thread()
{
    try
    {
        join();
        empty_tsd();
    }
    catch (...)
    {
        // Do not allow the destructor to throw an exception
    }
}

#endif /* PEGASUS_HAVE_PTHREADS */

//==============================================================================
//
// Windows Threads Implementation:
//
//==============================================================================

#if defined(PEGASUS_HAVE_WINDOWS_THREADS)

ThreadStatus Thread::run()
{
    // Note: A Win32 thread ID is not the same thing as a pthread ID.
    // Win32 threads have both a thread ID and a handle.  The handle
    // is used in the wait functions, etc.
    // So _handle.thid is actually the thread handle.

    unsigned threadid = 0;

    ThreadType tt;
    tt.handle = (HANDLE) _beginthreadex(NULL, 0, _start, this, 0, &threadid);
    _handle.thid = tt;

    if (Threads::null(_handle.thid))
    {
        if (errno == EAGAIN)
        {
            return PEGASUS_THREAD_INSUFFICIENT_RESOURCES;
        }
        else
        {
            return PEGASUS_THREAD_SETUP_FAILURE;
        }
    }
    return PEGASUS_THREAD_OK;
}

void Thread::cancel()
{
    _cancelled = true;
}

void Thread::thread_switch()
{
    Sleep(0);
}

void Thread::sleep(Uint32 milliseconds)
{
    Sleep(milliseconds);
}

void Thread::join()
{
    if (!Threads::null(_handle.thid))
    {
        if (!_is_detached)
        {
            if (!_cancelled)
            {
                // Emulate the unix join api. Caller sleeps until thread is
                // done.
                WaitForSingleObject(_handle.thid.handle, INFINITE);
            }
            else
            {
                // Currently this is the only way to ensure this code does
                // not
                // hang forever.
                if (WaitForSingleObject(_handle.thid.handle, 10000) ==
                    WAIT_TIMEOUT)
                {
                    TerminateThread(_handle.thid.handle, 0);
                }
            }

            DWORD exit_code = 0;
            GetExitCodeThread(_handle.thid.handle, &exit_code);
            _exit_code = (ThreadReturnType) exit_code;
        }

        CloseHandle(_handle.thid.handle);
        Threads::clear(_handle.thid);
    }
}

void Thread::detach()
{
    _is_detached = true;
}

Thread::Thread(
    ThreadReturnType(PEGASUS_THREAD_CDECL* start)(void*),
    void *parameter,
    Boolean detached) :_is_detached(detached),
    _cancelled(false),
    _start(start),
    _cleanup(),
    _thread_parm(parameter),
    _exit_code(0)
{
    Threads::clear(_handle.thid);
    memset(_tsd, 0, sizeof(_tsd));
}

Thread::~Thread()
{
    try
    {
        join();
        empty_tsd();
    }
    catch (...)
    {
    }
}

#endif /* PEGASUS_HAVE_WINDOWS_THREADS */

//==============================================================================
//
// Common implementation:
//
//==============================================================================

void language_delete(void *data)
{
    if (data != NULL)
    {
        AutoPtr < AcceptLanguageList > al(static_cast <
                                          AcceptLanguageList * >(data));
    }
}

Boolean Thread::_signals_blocked = false;
#ifndef PEGASUS_OS_ZOS
TSDKeyType Thread::_platform_thread_key = TSDKeyType(-1);
#else
TSDKeyType Thread::_platform_thread_key;
#endif
Boolean Thread::_key_initialized = false;
Boolean Thread::_key_error = false;

void Thread::cleanup_push(void (*routine) (void *), void *parm)
{
    AutoPtr < cleanup_handler > cu(new cleanup_handler(routine, parm));
    _cleanup.insert_front(cu.get());
    cu.release();
    return;
}

void Thread::cleanup_pop(Boolean execute)
{
    AutoPtr < cleanup_handler > cu;
    try
    {
        cu.reset(_cleanup.remove_front());
    }
    catch (...)
    {
        PEGASUS_ASSERT(0);
    }
    if (execute == true)
    {
        cu->execute();
    }
}


void Thread::exit_self(ThreadReturnType exit_code)
{
#if !defined(PEGASUS_PLATFORM_AIX_RS_IBMCXX) \
    && !defined(PEGASUS_PLATFORM_PASE_ISERIES_IBMCXX)
    Threads::exit(exit_code);
#else
    // execute the cleanup stack and then return
    while (_cleanup.size())
    {
        try
        {
            cleanup_pop(true);
        }
        catch (...)
        {
            PEGASUS_ASSERT(0);
            break;
        }
    }
    _exit_code = exit_code;
    Threads::exit(exit_code);
    Threads::clear(_handle.thid);
#endif
}

Sint8 Thread::initializeKey()
{
    PEG_METHOD_ENTER(TRC_THREAD, "Thread::initializeKey");
    if (!Thread::_key_initialized)
    {
        if (Thread::_key_error)
        {
            PEG_TRACE_CSTRING(TRC_THREAD, Tracer::LEVEL1,
                          "Thread: ERROR - thread key error");
            return -1;
        }

        if (TSDKey::create(&Thread::_platform_thread_key) == 0)
        {
            PEG_TRACE_CSTRING(TRC_THREAD, Tracer::LEVEL4,
                          "Thread: able to create a thread key");
            Thread::_key_initialized = true;
        }
        else
        {
            PEG_TRACE_CSTRING(TRC_THREAD, Tracer::LEVEL1,
                          "Thread: ERROR - unable to create a thread key");
            Thread::_key_error = true;
            return -1;
        }
    }

    PEG_METHOD_EXIT();
    return 0;
}

Thread *Thread::getCurrent()
{
    PEG_METHOD_ENTER(TRC_THREAD, "Thread::getCurrent");
    if (Thread::initializeKey() != 0)
    {
        return NULL;
    }
    PEG_METHOD_EXIT();
    return (Thread *) TSDKey::get_thread_specific(_platform_thread_key);
}

void Thread::setCurrent(Thread * thrd)
{
    PEG_METHOD_ENTER(TRC_THREAD, "Thread::setCurrent");
    if (Thread::initializeKey() == 0)
    {
        if (TSDKey::
            set_thread_specific(Thread::_platform_thread_key,
                                (void *) thrd) == 0)
        {
            PEG_TRACE_CSTRING(TRC_THREAD, Tracer::LEVEL4,
                "Successful set Thread * into thread specific storage");
        }
        else
        {
            PEG_TRACE_CSTRING(TRC_THREAD, Tracer::LEVEL1,
                "ERROR: error setting Thread * into thread specific storage");
        }
    }
    PEG_METHOD_EXIT();
}

AcceptLanguageList *Thread::getLanguages()
{
    PEG_METHOD_ENTER(TRC_THREAD, "Thread::getLanguages");

    Thread *curThrd = Thread::getCurrent();
    if (curThrd == NULL)
        return NULL;
    AcceptLanguageList *acceptLangs =
        (AcceptLanguageList *) curThrd->reference_tsd(TSD_ACCEPT_LANGUAGES);
    curThrd->dereference_tsd();
    PEG_METHOD_EXIT();
    return acceptLangs;
}

void Thread::setLanguages(const AcceptLanguageList& langs)
{
    PEG_METHOD_ENTER(TRC_THREAD, "Thread::setLanguages");

    Thread *currentThrd = Thread::getCurrent();
    if (currentThrd != NULL)
    {
        AutoPtr<AcceptLanguageList> langsCopy(new AcceptLanguageList(langs));

        // deletes the old tsd and creates a new one
        currentThrd->put_tsd(
            TSD_ACCEPT_LANGUAGES,
            language_delete,
            sizeof (AcceptLanguageList *),
            langsCopy.get());

        langsCopy.release();
    }

    PEG_METHOD_EXIT();
}

void Thread::clearLanguages()
{
    PEG_METHOD_ENTER(TRC_THREAD, "Thread::clearLanguages");

    Thread *currentThrd = Thread::getCurrent();
    if (currentThrd != NULL)
    {
        // deletes the old tsd
        currentThrd->delete_tsd(TSD_ACCEPT_LANGUAGES);
    }

    PEG_METHOD_EXIT();
}

// ATTN: not sure where to put this!
#ifdef PEGASUS_ZOS_SECURITY
bool isEnhancedSecurity = 99;
#endif

PEGASUS_NAMESPACE_END
