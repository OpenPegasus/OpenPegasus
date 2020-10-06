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

#ifndef Pegasus_SpinLock_h
#define Pegasus_SpinLock_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/Linkage.h>

//==============================================================================
//
// PEGASUS_PLATFORM_HPUX_PARISC_ACC || PEGASUS_PLATFORM_HPUX_PARISC_GNU
//
// NOTE:  This spinlock implementation is based on the paper "Implementing
// Spinlocks on the Intel(R) Itanium(R) Architecture an PA-RISC" by Tor
// Ekqvist and David Graves.
//
// WARNING:  This implementation has subtle complexities.  Seemingly
// innocuous changes could have unexpected side effects.  Please use
// diligence when modifying the implementation.
//
//==============================================================================

#if defined(PEGASUS_PLATFORM_HPUX_PARISC_ACC) || \
    defined(PEGASUS_PLATFORM_HPUX_PARISC_GNU)
# define PEGASUS_SPIN_LOCK_DEFINED

# include <sys/time.h>    // For select()

#define PEGASUS_SPINLOCK_USE_PTHREADS

extern "C" int LoadAndClearWord(volatile int* addr);
extern "C" void _flush_globals();

PEGASUS_NAMESPACE_BEGIN

// This type implements a spinlock. It is deliberately not a class since we
// wish to avoid automatic construction/destruction.
struct SpinLock
{
    unsigned int initialized;

    /**
        Points to a 16-byte aligned lock word (which lies somewhere within
        the region). The lock word is zero when locked and 1 when unlocked.
     */
    volatile int* lock;

    /**
        Points to a lock region (which contains the lock).  The LDCWS
        instruction requires that the lock word be aligned on a 16-byte
        boundary.  So we allocate 32 bytes and adjust lock so that it falls
        on the first such boundary within this region. We make the region
        large to keep the spin locks from getting too close together, which
        would put them on the same cache line, creating contention.
     */
    char region[32];

    /**
        Extends the size of the struct to match the 64-byte cache line size.
        NOTE: This does not ensure that the struct will align with a cache
        line.  Doing so could benefit performance.
     */
    char unused[24];
};

inline void SpinLockCreate(SpinLock& x)
{
    // Set x.lock to first 16-byte boundary within region.
#ifdef __LP64__
    x.lock = (volatile int*)(((long)x.region + 15) & ~0xF);
#else
    x.lock = (volatile int*)(((int)x.region + 15) & ~0xF);
#endif

    // Set to unlocked
    *x.lock = 1;

    x.initialized = 1;
}

inline void SpinLockDestroy(SpinLock& x)
{
}

inline void SpinLockLock(SpinLock& x)
{
    // Spin until we obtain the lock.
    while (1)
    {
        for (Uint32 spins = 0; spins < 200; spins++)
        {
            if (*x.lock == 1)    // pre-check
            {
                if (LoadAndClearWord(x.lock) == 1)
                {
                    return;
                }
            }
        }

        // Didn't get the lock after 200 spins, so sleep for 5 ms
        struct timeval sleeptime = { 0, 5000 };
        select(0, 0, 0, 0, &sleeptime);
    }
}

inline void SpinLockUnlock(SpinLock& x)
{
    // Ensure that the compiler doesn't hold any externally-visible values in
    // registers across the lock release.
#ifndef PEGASUS_PLATFORM_HPUX_PARISC_GNU
     _flush_globals();
#endif

    // Set to unlocked
    *x.lock = 1;
}

PEGASUS_NAMESPACE_END

#endif /* PEGASUS_PLATFORM_HPUX_PARISC_ACC || 
     PEGASUS_PLATFORM_HPUX_PARISC_GNU */

//==============================================================================
//
// PEGASUS_PLATFORM_SOLARIS_SPARC_GNU
//
//==============================================================================

#if defined(PEGASUS_PLATFORM_SOLARIS_SPARC_GNU)
# define PEGASUS_SPIN_LOCK_DEFINED

# include <new>

#define PEGASUS_SPINLOCK_USE_PTHREADS

PEGASUS_NAMESPACE_BEGIN

// This type implements a spinlock. It is deliberately not a class since we
// wish to avoid automatic construction/destruction.
struct SpinLock
{
    volatile unsigned char lock;
};

inline void SpinLockCreate(SpinLock& x)
{
    x.lock = 0;
}

inline void SpinLockDestroy(SpinLock& x)
{
}

inline void SpinLockLock(SpinLock& x)
{
    Uint32 value;

    // Loop until lock becomes zero.
    do
    {
        // Load and store unsigned byte (LDSTUB). Load the lock argument
        // into value and set lock to 0xFF (atomically).
        asm("ldstub %1, %0"
            : "=r" (value),
              "=m" (x.lock)
            : "m" (x.lock));
    }
    while (value);
}

inline void SpinLockUnlock(SpinLock& x)
{
    x.lock = 0;
}

PEGASUS_NAMESPACE_END

#endif /* PEGASUS_PLATFORM_SOLARIS_SPARC_GNU */

//==============================================================================
//
// Generic SpinLock Implementation (for other platforms).
//
//==============================================================================

#if !defined(PEGASUS_SPIN_LOCK_DEFINED)
# define PEGASUS_SPIN_LOCK_DEFINED

# include <new>
# include <Pegasus/Common/Mutex.h>

PEGASUS_NAMESPACE_BEGIN

// This type implements a spinlock. It is deliberately not a class since we
// wish to avoid automatic construction/destruction.
struct SpinLock
{
    union
    {
        char mutex[sizeof(Mutex)];
        Uint64 alignment8;
    };
};

inline void SpinLockCreate(SpinLock& x)
{
    new(&x.mutex) Mutex;
}

inline void SpinLockDestroy(SpinLock& x)
{
    ((Mutex*)&x.mutex)->~Mutex();
}

inline void SpinLockLock(SpinLock& x)
{
    ((Mutex*)&x.mutex)->lock();
}

inline void SpinLockUnlock(SpinLock& x)
{
    ((Mutex*)&x.mutex)->unlock();
}

PEGASUS_NAMESPACE_END

#endif /* !PEGASUS_SPIN_LOCK_DEFINED */

//==============================================================================
//
// SpinLock Pool Definitions
//
//==============================================================================

//  If PEGASUS_NUM_SHARED_SPIN_LOCKS size need to be changed,
//  ensure size is power of two to simplify moudulus calculation
#define PEGASUS_NUM_SHARED_SPIN_LOCKS 64

PEGASUS_NAMESPACE_BEGIN

// This array defines spin locks shared across the system. These are
// initialized by calling SpinLockCreatePool().
PEGASUS_COMMON_LINKAGE
    extern SpinLock spinLockPool[PEGASUS_NUM_SHARED_SPIN_LOCKS];

// This flag is 0 until SpinLockCreatePool() is called, which sets it
// to 1.
PEGASUS_COMMON_LINKAGE extern int spinLockPoolInitialized;

// Initializes the global pool of mutexes.
PEGASUS_COMMON_LINKAGE void SpinLockCreatePool();

// Maps an address into the spinLockPool[] array defined above. This is used
// to assign objects (by their addresses) to a shared lock. Collisions are
// okay.
inline size_t SpinLockIndex(const void* x)
{
    // Throw away the lower two bits since they are almost always zero
    // anyway due to alignment properties.
    return ((size_t)x >> 2) & ( PEGASUS_NUM_SHARED_SPIN_LOCKS -1);
}

// Call this function before forking to unlock the spinlocks in the global
// spinlock pool (spinLockPool).
void SpinLockInit();

PEGASUS_NAMESPACE_END

#endif /* Pegasus_SpinLock_h */
