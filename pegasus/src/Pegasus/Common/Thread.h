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

#ifndef Pegasus_Thread_h
#define Pegasus_Thread_h

#include <cstring>
#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/AtomicInt.h>
#include <Pegasus/Common/InternalException.h>
#include <Pegasus/Common/AcceptLanguageList.h>
#include <Pegasus/Common/Linkage.h>
#include <Pegasus/Common/AutoPtr.h>
#include <Pegasus/Common/List.h>
#include <Pegasus/Common/Mutex.h>
#include <Pegasus/Common/Semaphore.h>
#include <Pegasus/Common/TSDKey.h>
#include <Pegasus/Common/Threads.h>

#if defined(PEGASUS_HAVE_PTHREADS)
# include <signal.h>
#endif

PEGASUS_NAMESPACE_BEGIN

class PEGASUS_COMMON_LINKAGE cleanup_handler : public Linkable
{
public:
    cleanup_handler(void (*routine) (void *), void *arg):_routine(routine),
        _arg(arg)
    {
    }
    ~cleanup_handler()
    {;
    }

private:

    void execute()
    {
        _routine(_arg);
    }

    cleanup_handler();

    void (*_routine)(void*);
    void *_arg;

    friend class Thread;
};

///////////////////////////////////////////////////////////////////////////////

enum TSD_Key
{
    TSD_ACCEPT_LANGUAGES,
    TSD_SLEEP_SEM,
    TSD_LAST_ACTIVITY_TIME,
    TSD_WORK_FUNC,
    TSD_WORK_PARM,
    TSD_BLOCKING_SEM,
    TSD_CIMOM_HANDLE_CONTENT_LANGUAGES,
    TSD_RESERVED_1,
    TSD_RESERVED_2,
    TSD_RESERVED_3,
    TSD_RESERVED_4,
    TSD_RESERVED_5,
    TSD_RESERVED_6,
    TSD_RESERVED_7,
    TSD_RESERVED_8,
    // Add new TSD keys before this line.
    TSD_COUNT
};

class thread_data
{
    /**
     * This class is NOT build thread-safe.
     * The Caller(user) of this class has to ensure there is no collision
     * taking place.
     *
     * There is no mechanism in place to protect threads from manipulating
     * the same thread-specific storage at one time.
     * Make sure the possibility for a parallel access to the same
     * threads-specific data from multiple threads cannot arise.
     *
     * In OpenPegasus this class is used in the ThreadPool
     *        - on initialisation and creation of threads owned by ThreadPool
     *        - on threads that are idle inside the ThreadPool
     *        - on the ThreadPools main thread (the thread which the ThreadPool
     *          runs in)
     * In OpenPegasus this class is used in the
     * ClientCIMOMHandleRep and InternalCIMOMHandleRep
     *        - on the current active Thread which belongs to that CIMOMHandle
     *
     */
public:

    static void default_delete(void *data)
    {
        if (data)
            ::operator  delete(data);
    }

    thread_data(TSD_Key key) : _delete_func(0), _data(0), _size(0), _key(key)
    {
    }

    thread_data(TSD_Key key, size_t size) :
        _delete_func(default_delete), _size(size), _key(key)
    {
        _data = ::operator  new(_size);
    }

    thread_data(TSD_Key key, size_t size, void* data)
        : _delete_func(default_delete), _size(size), _key(key)
    {
        _data = ::operator  new(_size);
        memcpy(_data, data, size);
    }

    ~thread_data()
    {
        if (_data && _delete_func)
            (*_delete_func)(_data);
    }

    /**
     * This function is used to put data in thread space.
     *
     * Be aware that there is NOTHING in place to stop
     * other users of the thread to remove this data.
     * Or change the data.
     *
     * You, the developer has to make sure that there are
     * no situations in which this can arise (ie, have a
     * lock for the function which manipulates the TSD.
     *
     * @exception NullPointer
    */
    void put_data(void (*del)(void *), size_t size, void* data)
    {
        if (_data && _delete_func)
            (*_delete_func)(_data);

        _delete_func = del;
        _data = data;
        _size = size;
    }

    size_t get_size()
    {
        return _size;
    }

    void* get_data()
    {
        return _data;
    }

    /**
        This function is used to retrieve data from the
        TSD, the thread specific data.

        Be aware that there is NOTHING in place to stop
        other users of the thread to change the data you
        get from this function.

        You, the developer has to make sure that there are
        no situations in which this can arise (ie, have a
        lock for the function which manipulates the TSD.
     */
    void get_data(void** data, size_t* size)
    {
        if (data == 0 || size == 0)
            throw NullPointer();

        *data = _data;
        *size = _size;
    }

    // @exception NullPointer
    void copy_data(void** buf, size_t* size)
    {
        if ((buf == 0) || (size == 0))
            throw NullPointer();

        *buf = ::operator new(_size);
        *size = _size;
        memcpy(*buf, _data, _size);
    }


private:

    thread_data();
    thread_data(const thread_data& x);
    thread_data& operator=(const thread_data& x);

    void (*_delete_func)(void*);
    void* _data;
    size_t _size;
    TSD_Key _key;
};


enum ThreadStatus
{
    PEGASUS_THREAD_OK = 1,      /* No problems */
    PEGASUS_THREAD_INSUFFICIENT_RESOURCES,      /* Can't allocate a thread.
                                                   Not enough memory. Try
                                                   again later */
    PEGASUS_THREAD_SETUP_FAILURE,       /* Could not allocate into the thread
                                           specific data storage. */
    PEGASUS_THREAD_UNAVAILABLE  /* Service is being destroyed and no new
                                   threads can be provided. */
};

///////////////////////////////////////////////////////////////////////////

class PEGASUS_COMMON_LINKAGE Thread : public Linkable
{
public:

    Thread(
        ThreadReturnType(PEGASUS_THREAD_CDECL* start) (void*),
        void* parameter,
        Boolean detached);

     ~Thread();

      /** Start the thread.
          @return PEGASUS_THREAD_OK if the thread is started successfully,
          PEGASUS_THREAD_INSUFFICIENT_RESOURCES if the resources necessary
          to start the thread are not currently available.
          PEGASUS_THREAD_SETUP_FAILURE if the thread could not
          be create properly - check the 'errno' value for specific operating
          system return code.
       */
    ThreadStatus run();

    // get the user parameter
    void *get_parm()
    {
        return _thread_parm;
    }

    // cancellation must be deferred (not asynchronous)
    // for user-level threads the thread itself can decide
    // when it should die.
    void cancel();

    // for user-level threads - put the calling thread
    // to sleep and jump to the thread scheduler.
    // platforms with preemptive scheduling and native threads
    // can define this to be a no-op.
    // platforms without preemptive scheduling like NetWare
    // or gnu portable threads will have an existing
    // routine that can be mapped to this method

    void thread_switch();

    static void sleep(Uint32 msec);

    // block the calling thread until this thread terminates
    void join();

    // thread routine needs to call this function when
    // it is ready to exit
    void exit_self(ThreadReturnType return_code);

    // stack of functions to be called when thread terminates
    // will be called last in first out (LIFO)
    void cleanup_push(void (*routine) (void *), void* parm);

    void cleanup_pop(Boolean execute = true);

    // get the buffer associated with the key
    // NOTE: this call leaves the tsd LOCKED !!!!
    void* reference_tsd(TSD_Key key)
    {
        if (_tsd[key])
            return _tsd[key]->get_data();
         else
            return 0;
    }

    // release the lock held on the tsd
    // NOTE: assumes a corresponding and prior call to reference_tsd() !!!
    void dereference_tsd()
    {
    }

    // delete the tsd associated with the key
    void delete_tsd(TSD_Key key)
    {
        thread_data* tsd;

        tsd = _tsd[key];
        _tsd[key] = 0;

        if (tsd)
            delete tsd;
    }

    void empty_tsd()
    {
        thread_data* data[TSD_COUNT];

        memcpy(data, _tsd, sizeof(_tsd));
        memset(_tsd, 0, sizeof(_tsd));

        for (size_t i = 0; i < TSD_COUNT; i++)
        {
            if (data[i])
                delete data[i];
        }
    }

    // create or re-initialize tsd associated with the key
    // if the tsd already exists, delete the existing buffer
    void put_tsd(
        TSD_Key key,
        void (*delete_func)(void*),
        Uint32 size,
        void* value)
    {
        thread_data* tsd = new thread_data(key);
        tsd->put_data(delete_func, size, value);

        thread_data* old;

        old = _tsd[key];
        _tsd[key] = tsd;

        if (old)
            delete old;
    }

    ThreadReturnType get_exit()
    {
        return _exit_code;
    }

    ThreadType self()
    {
        return Threads::self();
    }

    ThreadHandle getThreadHandle()
    {
        return _handle;
    }

    Boolean isDetached()
    {
        return _is_detached;
    }

    void detach();

    //
    // Gets the Thread object associated with the caller's thread.
    // Note: this may return NULL if no Thread object is associated
    // with the caller's thread.
    //
    static Thread *getCurrent();

    //
    // Sets the Thread object associated with the caller's thread.
    // Note: the Thread object must be placed on the heap.
    //
    static void setCurrent(Thread* thrd);

    //
    // Gets the AcceptLanguageList object associated with the caller's
    // Thread.
    // Note: this may return NULL if no Thread object, or no
    // AcceptLanguageList object, is associated with the caller's thread.
    //
    static AcceptLanguageList* getLanguages();

    //
    // Sets the AcceptLanguageList object associated with the caller's
    // Thread.
    // Note: a Thread object must have been previously associated with
    // the caller's thread.
    //
    static void setLanguages(const AcceptLanguageList& langs);

    //
    // Removes the AcceptLanguageList object associated with the caller's
    // Thread.
    //
    static void clearLanguages();

private:
    Thread();

    static Sint8 initializeKey();

    ThreadHandle _handle;
    Boolean _is_detached;
    Boolean _cancelled;

    // always pass this * as the void * parameter to the thread
    // store the user parameter in _thread_parm

    ThreadReturnType(PEGASUS_THREAD_CDECL* _start) (void *);
    List<cleanup_handler, Mutex> _cleanup;
    thread_data* _tsd[TSD_COUNT];

    void* _thread_parm;
    ThreadReturnType _exit_code;
    static Boolean _signals_blocked;
    static TSDKeyType _platform_thread_key;
    static Boolean _key_initialized;
    static Boolean _key_error;
};

PEGASUS_NAMESPACE_END

#endif // Pegasus_Thread_h
