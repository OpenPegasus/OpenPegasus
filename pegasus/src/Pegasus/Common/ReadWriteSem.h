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

#ifndef Pegasus_ReadWriteSem_h
#define Pegasus_ReadWriteSem_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/Linkage.h>
#include <Pegasus/Common/Semaphore.h>
#include <Pegasus/Common/Mutex.h>
#include <Pegasus/Common/AtomicInt.h>

PEGASUS_NAMESPACE_BEGIN

/**
    Every platform should decide which implementation for read/write locks
    should be used in OpenPegasus by defining one of the following:
    1.) PEGASUS_USE_POSIX_RWLOCK - POSIX standard based implementation
    2.) PEGASUS_USE_SEMAPHORE_RWLOCK - mutex based implementation

    The definition for each platform can be found in the according platform
    header file: pegasus/src/Pegasus/Common/Platform_<Platform>.h
*/

#if !defined(PEGASUS_USE_POSIX_RWLOCK) && !defined(PEGASUS_USE_SEMAPHORE_RWLOCK)
# error "Unsupported platform: ReadWriteSem.h implementation type missing"

#endif

//==============================================================================
//
// ReadWriteSemRep
//
//==============================================================================

#ifdef PEGASUS_USE_POSIX_RWLOCK
struct ReadWriteSemRep
{
    pthread_rwlock_t rwlock;
};
#endif /* PEGASUS_USE_POSIX_RWLOCK */

#ifdef PEGASUS_USE_SEMAPHORE_RWLOCK
struct ReadWriteSemRep
{
    Semaphore _rlock;
    Mutex _wlock;
    Mutex _internal_lock;
    AtomicInt _readers;
    AtomicInt _writers;
    ReadWriteSemRep() :
        _rlock(10), _wlock(), _internal_lock(), _readers(0), _writers(0)
    {
    }
};
#endif /* PEGASUS_USE_POSIX_RWLOCK */

//==============================================================================
//
// ReadWriteSem
//
//==============================================================================

class PEGASUS_COMMON_LINKAGE ReadWriteSem
{
public:

    ReadWriteSem();

    ~ReadWriteSem();

    void waitRead();

    void waitWrite();

    void unlockRead();

    void unlockWrite();

private:

    ReadWriteSemRep _rwlock;
};

//==============================================================================
//
// ReadLock
//
//==============================================================================

class ReadLock
{
public:

    ReadLock(ReadWriteSem& rwsem) : _rwsem(rwsem)
    {
        _rwsem.waitRead();
    }

    ~ReadLock()
    {
        _rwsem.unlockRead();
    }

private:
    ReadWriteSem & _rwsem;
};

//==============================================================================
//
// WriteLock
//
//==============================================================================

class WriteLock
{
public:

    WriteLock(ReadWriteSem& rwsem) : _rwsem(rwsem)
    {
        _rwsem.waitWrite();
    }

    ~WriteLock()
    {
        _rwsem.unlockWrite();
    }

private:
    ReadWriteSem & _rwsem;
};

PEGASUS_NAMESPACE_END

#endif /* Pegasus_ReadWriteSem_h */
