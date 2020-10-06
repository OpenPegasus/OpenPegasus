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

#ifndef Pegasus_AtomicInt_h
#define Pegasus_AtomicInt_h

#include <Pegasus/Common/Config.h>

//==============================================================================
//
// class AtomicIntTemplate<>
//
//==============================================================================

PEGASUS_NAMESPACE_BEGIN

template<class ATOMIC_TYPE>
class AtomicIntTemplate
{
public:

    // Constructor.
    AtomicIntTemplate(Uint32 n = 0);

    // Destructor.
    ~AtomicIntTemplate();

    // Sets value.
    void set(Uint32 n);

    // Gets value.
    Uint32 get() const;

    // Increment.
    void inc();

    // Decrement.
    void dec();

    // Decrements and returns true if it is zero.
    bool decAndTestIfZero();

    // Assignment.
    AtomicIntTemplate& operator=(Uint32 n) { set(n); return* this; }

    // Post-increment.
    void operator++(int) { inc(); }

    // Post-decrement.
    void operator--(int) { dec(); }

private:

    // Note: These methods are intentionally hidden (and should not be called).
    // The implementation is much easier without having to implement these for
    // every platform.
    AtomicIntTemplate(const AtomicIntTemplate&);
    AtomicIntTemplate& operator=(const AtomicIntTemplate&);
    Boolean operator==(Uint32) const;
    void operator++();
    void operator--();

    typedef AtomicIntTemplate<ATOMIC_TYPE> This;

    ATOMIC_TYPE _rep;
};

PEGASUS_NAMESPACE_END

// if GNU GCC version >= 4.7.0, use the built-in atomic operations
// Clang uses libstdc++ and Atmic operations in clang appreared in clang 3.1
#if defined(GCC_VERSION) && GCC_VERSION >= 40700 || \
   defined (__clang__ ) && ( __clang_major__ >= 3 && __clang_minor__ >= 1)
# define PEGASUS_ATOMIC_INT_DEFINED

PEGASUS_NAMESPACE_BEGIN

struct AtomicType
{
    volatile int n;
};

PEGASUS_TEMPLATE_SPECIALIZATION
inline AtomicIntTemplate<AtomicType>::AtomicIntTemplate(Uint32 n)
{
    __atomic_store_n (&_rep.n, n, __ATOMIC_SEQ_CST);
}

PEGASUS_TEMPLATE_SPECIALIZATION
inline AtomicIntTemplate<AtomicType>::~AtomicIntTemplate()
{
}

PEGASUS_TEMPLATE_SPECIALIZATION
inline Uint32 AtomicIntTemplate<AtomicType>::get() const
{
    return __atomic_load_n (&_rep.n, __ATOMIC_SEQ_CST);
}

PEGASUS_TEMPLATE_SPECIALIZATION
inline void AtomicIntTemplate<AtomicType>::set(Uint32 n)
{
    __atomic_store_n (&_rep.n, n, __ATOMIC_SEQ_CST);
}

PEGASUS_TEMPLATE_SPECIALIZATION
inline void AtomicIntTemplate<AtomicType>::inc()
{
    __atomic_fetch_add (&_rep.n, 1, __ATOMIC_SEQ_CST);
}

PEGASUS_TEMPLATE_SPECIALIZATION
inline void AtomicIntTemplate<AtomicType>::dec()
{
    __atomic_fetch_sub (&_rep.n, 1, __ATOMIC_SEQ_CST);
}

PEGASUS_TEMPLATE_SPECIALIZATION
inline bool AtomicIntTemplate<AtomicType>::decAndTestIfZero()
{
    return (__atomic_fetch_sub (&_rep.n, 1, __ATOMIC_SEQ_CST) == 1);
}

typedef AtomicIntTemplate<AtomicType> AtomicInt;

PEGASUS_NAMESPACE_END

#else //!(GCC_VERSION && GCC_VERSION >= 40700), use platform specific atomics

//==============================================================================
//
// PEGASUS_PLATFORM_LINUX_IX86_GNU
// PEGASUS_PLATFORM_LINUX_IX86_CLANG
// PEGASUS_PLATFORM_DARWIN_IX86_GNU
// PEGASUS_PLATFORM_LINUX_X86_64_GNU
// PEGASUS_PLATFORM_LINUX_X86_64_CLANG
//
//==============================================================================

#if defined(PEGASUS_PLATFORM_LINUX_IX86_GNU) || \
    defined(PEGASUS_PLATFORM_LINUX_IX86_CLANG) || \
    defined(PEGASUS_PLATFORM_DARWIN_IX86_GNU) || \
    defined(PEGASUS_PLATFORM_LINUX_X86_64_GNU) || \
    defined(PEGASUS_PLATFORM_LINUX_X86_64_CLANG)
# define PEGASUS_ATOMIC_INT_DEFINED

// Note: this lock can be eliminated for single processor systems.
# define PEGASUS_ATOMIC_LOCK "lock ; "

PEGASUS_NAMESPACE_BEGIN

struct AtomicType
{
    volatile int n;
};

PEGASUS_TEMPLATE_SPECIALIZATION
inline AtomicIntTemplate<AtomicType>::AtomicIntTemplate(Uint32 n)
{
    _rep.n = n;
}

PEGASUS_TEMPLATE_SPECIALIZATION
inline AtomicIntTemplate<AtomicType>::~AtomicIntTemplate()
{
}

PEGASUS_TEMPLATE_SPECIALIZATION
inline Uint32 AtomicIntTemplate<AtomicType>::get() const
{
    return _rep.n;
}

PEGASUS_TEMPLATE_SPECIALIZATION
inline void AtomicIntTemplate<AtomicType>::set(Uint32 n)
{
    _rep.n = n;
}

PEGASUS_TEMPLATE_SPECIALIZATION
inline void AtomicIntTemplate<AtomicType>::inc()
{
    asm volatile(
        PEGASUS_ATOMIC_LOCK "incl %0"
        :"=m" (_rep.n)
        :"m" (_rep.n));
}

PEGASUS_TEMPLATE_SPECIALIZATION
inline void AtomicIntTemplate<AtomicType>::dec()
{
    asm volatile(
        PEGASUS_ATOMIC_LOCK "decl %0"
        :"=m" (_rep.n)
        :"m" (_rep.n));
}

PEGASUS_TEMPLATE_SPECIALIZATION
inline bool AtomicIntTemplate<AtomicType>::decAndTestIfZero()
{
    unsigned char c;

    asm volatile(
        PEGASUS_ATOMIC_LOCK "decl %0; sete %1"
        :"=m" (_rep.n), "=qm" (c)
        :"m" (_rep.n) : "memory");

    return c != 0;
}

typedef AtomicIntTemplate<AtomicType> AtomicInt;

PEGASUS_NAMESPACE_END

#endif /* PEGASUS_PLATFORM_LINUX_IX86_GNU */

//==============================================================================
//
// PEGASUS_PLATFORM_LINUX_IA64_GNU
//
//==============================================================================

#if defined(PEGASUS_PLATFORM_LINUX_IA64_GNU)
# define PEGASUS_ATOMIC_INT_DEFINED

PEGASUS_NAMESPACE_BEGIN

struct AtomicType
{
    volatile int n;
};

PEGASUS_TEMPLATE_SPECIALIZATION
inline AtomicIntTemplate<AtomicType>::AtomicIntTemplate(Uint32 n)
{
    _rep.n = (int)n;
}

PEGASUS_TEMPLATE_SPECIALIZATION
inline AtomicIntTemplate<AtomicType>::~AtomicIntTemplate()
{
    // Nothing to do!
}

PEGASUS_TEMPLATE_SPECIALIZATION
inline Uint32 AtomicIntTemplate<AtomicType>::get() const
{
    return (Uint32)_rep.n;
}

PEGASUS_TEMPLATE_SPECIALIZATION
inline void AtomicIntTemplate<AtomicType>::set(Uint32 n)
{
    _rep.n = (int)n;
}

PEGASUS_TEMPLATE_SPECIALIZATION
inline void AtomicIntTemplate<AtomicType>::inc()
{
    unsigned long tmp;
    volatile int* v = &_rep.n;

    asm volatile(
        "fetchadd4.rel %0=[%1],%2"
        : "=r"(tmp)
        : "r"(v), "i"(1)
        : "memory");
}

PEGASUS_TEMPLATE_SPECIALIZATION
inline void AtomicIntTemplate<AtomicType>::dec()
{
    unsigned long tmp;
    volatile int* v = &_rep.n;

    asm volatile(
        "fetchadd4.rel %0=[%1],%2"
        : "=r"(tmp)
        : "r"(v), "i"(-1)
        : "memory");
}

PEGASUS_TEMPLATE_SPECIALIZATION
inline bool AtomicIntTemplate<AtomicType>::decAndTestIfZero()
{
    unsigned long tmp;
    volatile int* v = &_rep.n;

    asm volatile(
        "fetchadd4.rel %0=[%1],%2"
        : "=r"(tmp)
        : "r"(v), "i"(-1)
        : "memory");

    return tmp == 1;
}

typedef AtomicIntTemplate<AtomicType> AtomicInt;

PEGASUS_NAMESPACE_END

#endif /* PEGASUS_PLATFORM_LINUX_IA64_GNU */

//==============================================================================
//
// PEGASUS_PLATFORM_LINUX_PPC_GNU
// PEGASUS_PLATFORM_LINUX_PPC64_GNU
//
//==============================================================================

#if defined(PEGASUS_PLATFORM_LINUX_PPC_GNU) || \
    defined(PEGASUS_PLATFORM_LINUX_PPC64_GNU)
# define PEGASUS_ATOMIC_INT_DEFINED

PEGASUS_NAMESPACE_BEGIN

struct AtomicType
{
    volatile Uint32 n;
};

PEGASUS_TEMPLATE_SPECIALIZATION
inline AtomicIntTemplate<AtomicType>::AtomicIntTemplate(Uint32 n)
{
    _rep.n = n;
}

PEGASUS_TEMPLATE_SPECIALIZATION
inline AtomicIntTemplate<AtomicType>::~AtomicIntTemplate()
{
}

PEGASUS_TEMPLATE_SPECIALIZATION
inline Uint32 AtomicIntTemplate<AtomicType>::get() const
{
    return _rep.n;
}

PEGASUS_TEMPLATE_SPECIALIZATION
inline void AtomicIntTemplate<AtomicType>::set(Uint32 n)
{
    // sync is required for SMP machines.
    asm volatile("sync" : : :); 
    _rep.n = n;
}

PEGASUS_TEMPLATE_SPECIALIZATION
inline void AtomicIntTemplate<AtomicType>::inc()
{
    int t;

    asm volatile(
        "lwsync\n"
        "1: lwarx %0,0,%1\n"
        "addic %0,%0,1\n"
        "stwcx. %0,0,%1\n"
        "bne- 1b\n"
        "isync"
        : "=&r" (t)
        : "r" (&_rep.n)
        : "cc", "memory");
}

PEGASUS_TEMPLATE_SPECIALIZATION
inline void AtomicIntTemplate<AtomicType>::dec()
{
    int c;

    asm volatile(
        "lwsync\n"
        "1: lwarx %0,0,%1\n"
        "addic %0,%0,-1\n"
        "stwcx. %0,0,%1\n"
        "bne- 1b\n"
        "isync"
        : "=&r" (c)
        : "r" (&_rep.n)
        : "cc", "memory");
}

PEGASUS_TEMPLATE_SPECIALIZATION
inline bool AtomicIntTemplate<AtomicType>::decAndTestIfZero()
{
    int c;

    asm volatile(
        "lwsync\n"
        "1: lwarx %0,0,%1\n"
        "addic %0,%0,-1\n"
        "stwcx. %0,0,%1\n"
        "bne- 1b\n"
        "isync"
        : "=&r" (c)
        : "r" (&_rep.n)
        : "cc", "memory");

    return c == 0;
}

typedef AtomicIntTemplate<AtomicType> AtomicInt;

PEGASUS_NAMESPACE_END

#endif /* PEGASUS_PLATFORM_LINUX_PPC_GNU */

//==============================================================================
//
// PEGASUS_PLATFORM_LINUX_PPC_E500_GNU
//
//==============================================================================

#if defined(PEGASUS_PLATFORM_LINUX_PPC_E500_GNU)
# define PEGASUS_ATOMIC_INT_DEFINED

PEGASUS_NAMESPACE_BEGIN

struct AtomicType
{
    volatile Uint32 n;
};

PEGASUS_TEMPLATE_SPECIALIZATION
inline AtomicIntTemplate<AtomicType>::AtomicIntTemplate(Uint32 n)
{
    _rep.n = n;
}

PEGASUS_TEMPLATE_SPECIALIZATION
inline AtomicIntTemplate<AtomicType>::~AtomicIntTemplate()
{
}

PEGASUS_TEMPLATE_SPECIALIZATION
inline Uint32 AtomicIntTemplate<AtomicType>::get() const
{
    return _rep.n;
}

PEGASUS_TEMPLATE_SPECIALIZATION
inline void AtomicIntTemplate<AtomicType>::set(Uint32 n)
{
    // sync is required for SMP machines.
    asm volatile("sync" : : :); 
    _rep.n = n;
}

PEGASUS_TEMPLATE_SPECIALIZATION
inline void AtomicIntTemplate<AtomicType>::inc()
{
    int t;

    asm volatile(
        "1: lwarx %0,0,%1\n"
        "addic %0,%0,1\n"
        "stwcx. %0,0,%1\n"
        "bne- 1b\n"
        "isync"
        : "=&r" (t)
        : "r" (&_rep.n)
        : "cc", "memory");
}

PEGASUS_TEMPLATE_SPECIALIZATION
inline void AtomicIntTemplate<AtomicType>::dec()
{
    int c;

    asm volatile(
        "1: lwarx %0,0,%1\n"
        "addic %0,%0,-1\n"
        "stwcx. %0,0,%1\n"
        "bne- 1b\n"
        "isync"
        : "=&r" (c)
        : "r" (&_rep.n)
        : "cc", "memory");
}

PEGASUS_TEMPLATE_SPECIALIZATION
inline bool AtomicIntTemplate<AtomicType>::decAndTestIfZero()
{
    int c;

    asm volatile(
        "1: lwarx %0,0,%1\n"
        "addic %0,%0,-1\n"
        "stwcx. %0,0,%1\n"
        "bne- 1b\n"
        "isync"
        : "=&r" (c)
        : "r" (&_rep.n)
        : "cc", "memory");

    return c == 0;
}

typedef AtomicIntTemplate<AtomicType> AtomicInt;

PEGASUS_NAMESPACE_END

#endif /* PEGASUS_PLATFORM_LINUX_PPC_E500_GNU */

//==============================================================================
//
// PEGASUS_PLATFORM_LINUX_XSCALE_GNU
//
//==============================================================================

#if defined (PEGASUS_PLATFORM_LINUX_XSCALE_GNU)
# define PEGASUS_ATOMIC_INT_DEFINED

PEGASUS_NAMESPACE_BEGIN

inline void AtomicIntDisableIRQs(unsigned long& flags)
{
    unsigned long temp;
    unsigned long x;

    asm volatile(
        "mrs %0, cpsr\n"
        "orr %1, %0, #128\n"
        "msr cpsr_c, %1\n"
        : "=r" (x), "=r" (temp)
        :
        : "memory");

    flags = x;
}

inline void AtomicIntEnableIRQs(unsigned long x)
{
    unsigned long temp;

    asm volatile(
        "mrs %0, cpsr\n"
        "orr %1, %0, #128\n"
        "msr cpsr_c, %1\n"
        : "=r" (x), "=r" (temp)
        :
        : "memory");
}

struct AtomicType
{
    volatile Uint32 n;
};

PEGASUS_TEMPLATE_SPECIALIZATION
inline AtomicIntTemplate<AtomicType>::AtomicIntTemplate(Uint32 n)
{
    _rep.n = n;
}

PEGASUS_TEMPLATE_SPECIALIZATION
inline AtomicIntTemplate<AtomicType>::~AtomicIntTemplate()
{
}

PEGASUS_TEMPLATE_SPECIALIZATION
inline Uint32 AtomicIntTemplate<AtomicType>::get() const
{
    return _rep.n;
}

PEGASUS_TEMPLATE_SPECIALIZATION
inline void AtomicIntTemplate<AtomicType>::set(Uint32 n)
{
    _rep.n = n;
}

PEGASUS_TEMPLATE_SPECIALIZATION
inline void AtomicIntTemplate<AtomicType>::inc()
{
    unsigned long flags;
    AtomicIntDisableIRQs(flags);
    _rep.n++;
    AtomicIntEnableIRQs(flags);
}

PEGASUS_TEMPLATE_SPECIALIZATION
inline void AtomicIntTemplate<AtomicType>::dec()
{
    unsigned long flags;
    AtomicIntDisableIRQs(flags);
    _rep.n--;
    AtomicIntEnableIRQs(flags);
}

PEGASUS_TEMPLATE_SPECIALIZATION
inline bool AtomicIntTemplate<AtomicType>::decAndTestIfZero()
{
    Uint32 tmp;
    unsigned long flags;
    AtomicIntDisableIRQs(flags);
    tmp = --_rep.n;
    AtomicIntEnableIRQs(flags);
    return tmp == 0;
}

typedef AtomicIntTemplate<AtomicType> AtomicInt;

PEGASUS_NAMESPACE_END

#endif /* PEGASUS_PLATFORM_LINUX_XSCALE_GNU */


#endif /* GCC_VERSION && GCC_VERSION >= 40700 */

//==============================================================================
//
// PEGASUS_OS_TYPE_WINDOWS
//
//==============================================================================

#if defined(PEGASUS_OS_TYPE_WINDOWS)
# define PEGASUS_ATOMIC_INT_DEFINED

#include <Pegasus/Common/Network.h>
#include <windows.h>

PEGASUS_NAMESPACE_BEGIN

typedef LONG AtomicType;

PEGASUS_TEMPLATE_SPECIALIZATION
inline AtomicIntTemplate<AtomicType>::AtomicIntTemplate(Uint32 n)
{
    _rep = LONG(n);
}

PEGASUS_TEMPLATE_SPECIALIZATION
inline AtomicIntTemplate<AtomicType>::~AtomicIntTemplate()
{
}

PEGASUS_TEMPLATE_SPECIALIZATION
inline Uint32 AtomicIntTemplate<AtomicType>::get() const
{
    return Uint32(_rep);
}

PEGASUS_TEMPLATE_SPECIALIZATION
inline void AtomicIntTemplate<AtomicType>::set(Uint32 n)
{
    _rep = LONG(n);
}

PEGASUS_TEMPLATE_SPECIALIZATION
inline void AtomicIntTemplate<AtomicType>::inc()
{
    InterlockedIncrement(&_rep);
}

PEGASUS_TEMPLATE_SPECIALIZATION
inline void AtomicIntTemplate<AtomicType>::dec()
{
    InterlockedDecrement(&_rep);
}

PEGASUS_TEMPLATE_SPECIALIZATION
inline bool AtomicIntTemplate<AtomicType>::decAndTestIfZero()
{
    return InterlockedDecrement(&_rep) == 0;
}

typedef AtomicIntTemplate<AtomicType> AtomicInt;

PEGASUS_NAMESPACE_END

#endif /* PEGASUS_OS_TYPE_WINDOWS */

//==============================================================================
//
// PEGASUS_OS_ZOS
//
//==============================================================================

#if defined(PEGASUS_OS_ZOS)
# define PEGASUS_ATOMIC_INT_DEFINED

PEGASUS_NAMESPACE_BEGIN

struct AtomicType
{
    cs_t n;
};

PEGASUS_TEMPLATE_SPECIALIZATION
inline AtomicIntTemplate<AtomicType>::AtomicIntTemplate(Uint32 n)
{
    _rep.n = (cs_t)n;
}

PEGASUS_TEMPLATE_SPECIALIZATION
inline AtomicIntTemplate<AtomicType>::~AtomicIntTemplate()
{
}

PEGASUS_TEMPLATE_SPECIALIZATION
inline Uint32 AtomicIntTemplate<AtomicType>::get() const
{
    return (Uint32)_rep.n;
}

PEGASUS_TEMPLATE_SPECIALIZATION
inline void AtomicIntTemplate<AtomicType>::set(Uint32 n)
{
    _rep.n = (cs_t)n;
}

PEGASUS_TEMPLATE_SPECIALIZATION
inline void AtomicIntTemplate<AtomicType>::inc()
{
    Uint32 x = (Uint32)_rep.n;
    Uint32 old = x;
    x++;
    while ( cs( (cs_t*)&old, &(_rep.n), (cs_t)x) )
    {
       x = (Uint32)_rep.n;
       old = x;
       x++;
    }
}

PEGASUS_TEMPLATE_SPECIALIZATION
inline void AtomicIntTemplate<AtomicType>::dec()
{
    Uint32 x = (Uint32)_rep.n;
    Uint32 old = x;
    x--;
    while ( cs( (cs_t*)&old, &(_rep.n), (cs_t) x) )
    {
       x = (Uint32) _rep.n;
       old = x;
       x--;
    }
}

PEGASUS_TEMPLATE_SPECIALIZATION
inline bool AtomicIntTemplate<AtomicType>::decAndTestIfZero()
{
    Uint32 x = (Uint32)_rep.n;
    Uint32 old = x;
    x--;
    while ( cs( (cs_t*)&old, &(_rep.n), (cs_t) x) )
    {
       x = (Uint32) _rep.n;
       old = x;
       x--;
    }
    return x==0;
}

typedef AtomicIntTemplate<AtomicType> AtomicInt;

PEGASUS_NAMESPACE_END

#endif /* PEGASUS_OS_ZOS */

//==============================================================================
//
// PEGASUS_PLATFORM_HPUX_IA64_ACC
//
//==============================================================================

#if defined (PEGASUS_PLATFORM_HPUX_IA64_ACC)
# define PEGASUS_ATOMIC_INT_DEFINED

#include <machine/sys/inline.h>

PEGASUS_NAMESPACE_BEGIN

struct AtomicType
{
    volatile Uint32 n;
};

PEGASUS_TEMPLATE_SPECIALIZATION
inline AtomicIntTemplate<AtomicType>::AtomicIntTemplate(Uint32 n)
{
    _rep.n = n;
}

PEGASUS_TEMPLATE_SPECIALIZATION
inline AtomicIntTemplate<AtomicType>::~AtomicIntTemplate()
{
    // Nothing to do!
}

PEGASUS_TEMPLATE_SPECIALIZATION
inline Uint32 AtomicIntTemplate<AtomicType>::get() const
{
    return _rep.n;
}

PEGASUS_TEMPLATE_SPECIALIZATION
inline void AtomicIntTemplate<AtomicType>::set(Uint32 n)
{
    _rep.n = n;
}

PEGASUS_TEMPLATE_SPECIALIZATION
inline void AtomicIntTemplate<AtomicType>::inc()
{
    _Asm_fetchadd(
        (_Asm_fasz)_FASZ_W,
        (_Asm_sem)_SEM_ACQ,
        (volatile Uint32*)&_rep.n,
        (int)1,
        (_Asm_ldhint)_LDHINT_NONE);
}

PEGASUS_TEMPLATE_SPECIALIZATION
inline void AtomicIntTemplate<AtomicType>::dec()
{
    _Asm_fetchadd(
        (_Asm_fasz)_FASZ_W,
        (_Asm_sem)_SEM_ACQ,
        (volatile Uint32*)&_rep.n,
        (int)-1,
        (_Asm_ldhint)_LDHINT_NONE);
}

PEGASUS_TEMPLATE_SPECIALIZATION
inline bool AtomicIntTemplate<AtomicType>::decAndTestIfZero()
{
    Uint32 x = _Asm_fetchadd(
        (_Asm_fasz)_FASZ_W,
        (_Asm_sem)_SEM_ACQ,
        (volatile Uint32*)&_rep.n,
        (int)-1,
        (_Asm_ldhint)_LDHINT_NONE);

    return x == 1;
}

typedef AtomicIntTemplate<AtomicType> AtomicInt;

PEGASUS_NAMESPACE_END

#endif /* PEGASUS_PLATFORM_HPUX_IA64_ACC */

#if defined (PEGASUS_PLATFORM_HPUX_IA64_GNU)
#include <atomic>

PEGASUS_NAMESPACE_BEGIN

struct AtomicType
{
        volatile uint32_t n;
};

PEGASUS_TEMPLATE_SPECIALIZATION
inline AtomicIntTemplate<AtomicType>::AtomicIntTemplate(Uint32 n)
{
        _rep.n = n;
}

PEGASUS_TEMPLATE_SPECIALIZATION
inline AtomicIntTemplate<AtomicType>::~AtomicIntTemplate()
{
}

PEGASUS_TEMPLATE_SPECIALIZATION
inline Uint32 AtomicIntTemplate<AtomicType>::get() const
{
        return _rep.n;
}

PEGASUS_TEMPLATE_SPECIALIZATION
inline void AtomicIntTemplate<AtomicType>::set(Uint32 n)
{
        _rep.n = n;
}

PEGASUS_TEMPLATE_SPECIALIZATION
inline void AtomicIntTemplate<AtomicType>::inc()
{
        atomic_inc_32(&_rep.n);
}

PEGASUS_TEMPLATE_SPECIALIZATION
inline void AtomicIntTemplate<AtomicType>::dec()
{
        atomic_dec_32(&_rep.n);
}

PEGASUS_TEMPLATE_SPECIALIZATION
inline bool AtomicIntTemplate<AtomicType>::decAndTestIfZero()
{
        return atomic_dec_32_nv(&_rep.n) == 0;
}
typedef AtomicIntTemplate<AtomicType> AtomicInt;

PEGASUS_NAMESPACE_END

#endif //PEGASUS_PLATFORM_HPUX_IA64_ACC

//==============================================================================
//
// PEGASUS_PLATFORM_VMS_ALPHA_DECCXX
// PEGASUS_PLATFORM_VMS_IA64_DECCXX
//
//==============================================================================

#if defined (PEGASUS_PLATFORM_VMS_ALPHA_DECCXX) || \
    defined (PEGASUS_PLATFORM_VMS_IA64_DECCXX)
# define PEGASUS_ATOMIC_INT_DEFINED

# include <sys/machine/builtins.h>

PEGASUS_NAMESPACE_BEGIN

struct AtomicType
{
    volatile int n;
};

PEGASUS_TEMPLATE_SPECIALIZATION
inline AtomicIntTemplate<AtomicType>::AtomicIntTemplate(Uint32 n)
{
    _rep.n = n;
}

PEGASUS_TEMPLATE_SPECIALIZATION
inline AtomicIntTemplate<AtomicType>::~AtomicIntTemplate()
{
}

PEGASUS_TEMPLATE_SPECIALIZATION
inline Uint32 AtomicIntTemplate<AtomicType>::get() const
{
    return _rep.n;
}

PEGASUS_TEMPLATE_SPECIALIZATION
inline void AtomicIntTemplate<AtomicType>::set(Uint32 n)
{
    _rep.n = n;
}

PEGASUS_TEMPLATE_SPECIALIZATION
inline void AtomicIntTemplate<AtomicType>::inc()
{
    __ATOMIC_INCREMENT_LONG(&_rep.n);
}

PEGASUS_TEMPLATE_SPECIALIZATION
inline void AtomicIntTemplate<AtomicType>::dec()
{
    __ATOMIC_DECREMENT_LONG(&_rep.n);
}

PEGASUS_TEMPLATE_SPECIALIZATION
inline bool AtomicIntTemplate<AtomicType>::decAndTestIfZero()
{
    return __ATOMIC_DECREMENT_LONG(&_rep.n) == 1;
}

typedef AtomicIntTemplate<AtomicType> AtomicInt;

PEGASUS_NAMESPACE_END

#endif /* _Pegasus_Common_AtomicInt_VMS_h */

//==============================================================================
//
// PEGASUS_OS_SOLARIS
//
//==============================================================================

#if defined(PEGASUS_OS_SOLARIS)
# define PEGASUS_ATOMIC_INT_DEFINED

#include <atomic.h>

PEGASUS_NAMESPACE_BEGIN

struct AtomicType
{
    volatile uint32_t n;
};

PEGASUS_TEMPLATE_SPECIALIZATION
inline AtomicIntTemplate<AtomicType>::AtomicIntTemplate(Uint32 n)
{
    _rep.n = n;
}

PEGASUS_TEMPLATE_SPECIALIZATION
inline AtomicIntTemplate<AtomicType>::~AtomicIntTemplate()
{
}

PEGASUS_TEMPLATE_SPECIALIZATION
inline Uint32 AtomicIntTemplate<AtomicType>::get() const
{
    return _rep.n;
}

PEGASUS_TEMPLATE_SPECIALIZATION
inline void AtomicIntTemplate<AtomicType>::set(Uint32 n)
{
    _rep.n = n;
}

PEGASUS_TEMPLATE_SPECIALIZATION
inline void AtomicIntTemplate<AtomicType>::inc()
{
    atomic_inc_32(&_rep.n);
}

PEGASUS_TEMPLATE_SPECIALIZATION
inline void AtomicIntTemplate<AtomicType>::dec()
{
    atomic_dec_32(&_rep.n);
}

PEGASUS_TEMPLATE_SPECIALIZATION
inline bool AtomicIntTemplate<AtomicType>::decAndTestIfZero()
{
    return atomic_dec_32_nv(&_rep.n) == 0;
}

typedef AtomicIntTemplate<AtomicType> AtomicInt;

PEGASUS_NAMESPACE_END

#endif

//==============================================================================
//
// PEGASUS_PLATFORM_AIX_RS_IBMCXX, PEGASUS_PLATFORM_PASE_ISERIES_IBMCXX
//
//==============================================================================

#if defined (PEGASUS_PLATFORM_AIX_RS_IBMCXX)|| \
    defined (PEGASUS_PLATFORM_PASE_ISERIES_IBMCXX)
# define PEGASUS_ATOMIC_INT_DEFINED

# include <sys/atomic_op.h>

PEGASUS_NAMESPACE_BEGIN

struct AtomicType
{
    volatile Uint32 n;
};

PEGASUS_TEMPLATE_SPECIALIZATION
inline AtomicIntTemplate<AtomicType>::AtomicIntTemplate(Uint32 n)
{
    _rep.n = n;
}

PEGASUS_TEMPLATE_SPECIALIZATION
inline AtomicIntTemplate<AtomicType>::~AtomicIntTemplate()
{
}

PEGASUS_TEMPLATE_SPECIALIZATION
inline Uint32 AtomicIntTemplate<AtomicType>::get() const
{
    return _rep.n;
}

PEGASUS_TEMPLATE_SPECIALIZATION
inline void AtomicIntTemplate<AtomicType>::set(Uint32 n)
{
    _rep.n = n;
}

PEGASUS_TEMPLATE_SPECIALIZATION
inline void AtomicIntTemplate<AtomicType>::inc()
{
    fetch_and_add((atomic_p)&_rep.n, 1);
}

PEGASUS_TEMPLATE_SPECIALIZATION
inline void AtomicIntTemplate<AtomicType>::dec()
{
    fetch_and_add((atomic_p)&_rep.n, -1);
}

PEGASUS_TEMPLATE_SPECIALIZATION
inline bool AtomicIntTemplate<AtomicType>::decAndTestIfZero()
{
    return fetch_and_add((atomic_p)&_rep.n, -1) == 1;
}

typedef AtomicIntTemplate<AtomicType> AtomicInt;

PEGASUS_NAMESPACE_END

#endif /* PEGASUS_PLATFORM_AIX_RS_IBMCXX, \
        PEGASUS_PLATFORM_PASE_ISERIES_IBMCXX */


//==============================================================================
//
// Generic Implementation
//
//==============================================================================

#if !defined(PEGASUS_ATOMIC_INT_DEFINED)
# define PEGASUS_ATOMIC_INT_DEFINED

# include <Pegasus/Common/SpinLock.h>

PEGASUS_NAMESPACE_BEGIN

// Represents the atomic type counter.
struct AtomicType
{
    Uint32 n;
};

PEGASUS_TEMPLATE_SPECIALIZATION
inline AtomicIntTemplate<AtomicType>::AtomicIntTemplate(Uint32 n)
{
    _rep.n = n;

    if (spinLockPoolInitialized == 0)
        SpinLockCreatePool();
}

PEGASUS_TEMPLATE_SPECIALIZATION
inline AtomicIntTemplate<AtomicType>::~AtomicIntTemplate()
{
    // Nothing to do.
}

PEGASUS_TEMPLATE_SPECIALIZATION
inline Uint32 AtomicIntTemplate<AtomicType>::get() const
{
    Uint32 tmp;
    size_t i = SpinLockIndex(&_rep);
    SpinLockLock(spinLockPool[i]);
    tmp = _rep.n;
    SpinLockUnlock(spinLockPool[i]);
    return tmp;
}

PEGASUS_TEMPLATE_SPECIALIZATION
inline void AtomicIntTemplate<AtomicType>::set(Uint32 n)
{
    size_t i = SpinLockIndex(&_rep);
    SpinLockLock(spinLockPool[i]);
    _rep.n = n;
    SpinLockUnlock(spinLockPool[i]);
}

PEGASUS_TEMPLATE_SPECIALIZATION
inline void AtomicIntTemplate<AtomicType>::inc()
{
    size_t i = SpinLockIndex(&_rep);
    SpinLockLock(spinLockPool[i]);
    _rep.n++;
    SpinLockUnlock(spinLockPool[i]);
}

PEGASUS_TEMPLATE_SPECIALIZATION
inline void AtomicIntTemplate<AtomicType>::dec()
{
    size_t i = SpinLockIndex(&_rep);
    SpinLockLock(spinLockPool[i]);
    _rep.n--;
    SpinLockUnlock(spinLockPool[i]);
}

PEGASUS_TEMPLATE_SPECIALIZATION
inline bool AtomicIntTemplate<AtomicType>::decAndTestIfZero()
{
    Uint32 tmp;
    size_t i = SpinLockIndex(&_rep);
    SpinLockLock(spinLockPool[i]);
    tmp = --_rep.n;
    SpinLockUnlock(spinLockPool[i]);
    return tmp == 0;
}

typedef AtomicIntTemplate<AtomicType> AtomicInt;

PEGASUS_NAMESPACE_END

#endif /* !PEGASUS_ATOMIC_INT_DEFINED */

#endif /* Pegasus_AtomicInt_h */
