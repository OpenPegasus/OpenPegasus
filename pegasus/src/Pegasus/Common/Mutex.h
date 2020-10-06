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

#ifndef Pegasus_Mutex_h
#define Pegasus_Mutex_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/Linkage.h>
#include <Pegasus/Common/Magic.h>
#include <Pegasus/Common/Threads.h>

PEGASUS_NAMESPACE_BEGIN

//==============================================================================
//
// MutexRep
//
//==============================================================================

#if defined(PEGASUS_HAVE_PTHREADS)
typedef pthread_mutex_t MutexType;
inline void mutex_lock(MutexType* mutex) { pthread_mutex_lock(mutex); }
inline void mutex_unlock(MutexType* mutex) { pthread_mutex_unlock(mutex); }
struct MutexRep
{
    pthread_mutex_t mutex;
    int count;
};
# define PEGASUS_MUTEX_INITIALIZER PTHREAD_MUTEX_INITIALIZER
#endif

#if defined(PEGASUS_HAVE_WINDOWS_THREADS)
typedef HANDLE MutexType;
inline void mutex_lock(MutexType* m) { WaitForSingleObject(*m, INFINITE); }
inline void mutex_unlock(MutexType* m) { ReleaseMutex(*m); }
struct MutexRep
{
    MutexType handle;
    size_t count;
};
# define PEGASUS_MUTEX_INITIALIZER (CreateMutex(NULL, FALSE, NULL))
#endif

//==============================================================================
//
// Mutex
//
//==============================================================================

class PEGASUS_COMMON_LINKAGE Mutex
{
public:

    enum RecursiveTag { RECURSIVE };
    enum NonRecursiveTag { NON_RECURSIVE };

    /** Default constructor creates a recursive mutex.
    */
    Mutex();

    /** Call as Mutex(Mutex::RECURSIVE) to create a recursive mutex.
    */
    Mutex(RecursiveTag);

    /** Call as Mutex(Mutex::NON_RECURSIVE) to create a non-recursive mutex.
    */
    Mutex(NonRecursiveTag);

    ~Mutex();

    void lock();

    /**
        Attempts to lock the mutex without blocking.
        @return A Boolean indicating whether the lock was acquired.
    */
    Boolean try_lock();

    /**
        Attempts to lock the mutex within the specified time.
        @param milliseconds The maximum time to block while attempting to
            acquire the lock.
        @return A Boolean indicating whether the lock was acquired.
    */
    Boolean timed_lock(Uint32 milliseconds);

    void unlock();

#if defined(PEGASUS_OS_LINUX)
    /**
        This method must only be called after a fork() to reset the mutex
        lock status in the new process.  Any other use of this method is
        unsafe.
    */
    void reinitialize();
#endif

private:
    Mutex(const Mutex&);
    Mutex& operator=(const Mutex&);

    MutexRep _rep;
    Magic<0x57D11485> _magic;

    friend class Condition;
};

//==============================================================================
//
// AutoMutex
//
//==============================================================================

class PEGASUS_COMMON_LINKAGE AutoMutex
{
public:

    AutoMutex(Mutex& mutex) : _mutex(mutex)
    {
        _mutex.lock();
    }

    ~AutoMutex()
    {
        _mutex.unlock();
    }

private:
    AutoMutex(); // Unimplemented
    AutoMutex(const AutoMutex& x); // Unimplemented
    AutoMutex& operator=(const AutoMutex& x); // Unimplemented

    Mutex& _mutex;
};

//==============================================================================
//
// PEGASUS_FORK_SAFE_MUTEX
//
//==============================================================================

// Use of this macro ensures that a static Mutex is not locked across a fork().

#if !defined(PEGASUS_HAVE_PTHREADS) || \
    defined(PEGASUS_OS_ZOS) || \
    defined(PEGASUS_OS_VMS)

# define PEGASUS_FORK_SAFE_MUTEX(mutex)

#elif defined(PEGASUS_OS_LINUX)

# define PEGASUS_FORK_SAFE_MUTEX(mutex)  \
    class ForkSafeMutex ## mutex         \
    {                                    \
    public:                              \
        ForkSafeMutex ## mutex()         \
        {                                \
            pthread_atfork(              \
                0,                       \
                0,                       \
                _reinitializeMutex);     \
        }                                \
                                         \
    private:                             \
        static void _reinitializeMutex() \
        {                                \
            mutex.reinitialize();        \
        }                                \
    };                                   \
                                         \
    static ForkSafeMutex ## mutex __forkSafeMutex ## mutex;

#else

# define PEGASUS_FORK_SAFE_MUTEX(mutex)  \
    class ForkSafeMutex ## mutex         \
    {                                    \
    public:                              \
        ForkSafeMutex ## mutex()         \
        {                                \
            pthread_atfork(              \
                _lockMutex,              \
                _unlockMutex,            \
                _unlockMutex);           \
        }                                \
                                         \
    private:                             \
        static void _lockMutex()         \
        {                                \
            mutex.lock();                \
        }                                \
                                         \
        static void _unlockMutex()       \
        {                                \
            mutex.unlock();              \
        }                                \
    };                                   \
                                         \
    static ForkSafeMutex ## mutex __forkSafeMutex ## mutex;

#endif

PEGASUS_NAMESPACE_END

#endif /* Pegasus_Mutex_h */
