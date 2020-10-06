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

#ifndef Pegasus_TSDKey_h
#define Pegasus_TSDKey_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/Linkage.h>
#include <Pegasus/Common/Threads.h>

PEGASUS_NAMESPACE_BEGIN

//==============================================================================
//
// TSDKeyType
//
//==============================================================================

#if defined(PEGASUS_HAVE_PTHREADS)
typedef pthread_key_t TSDKeyType;
#endif

#if defined(PEGASUS_HAVE_WINDOWS_THREADS)
typedef DWORD TSDKeyType;
#endif

//==============================================================================
//
// TSDKey
//
//==============================================================================

class PEGASUS_COMMON_LINKAGE TSDKey
{
public:

    static Uint32 create(TSDKeyType * key);

    static Uint32 destroy(TSDKeyType key);

    static void* get_thread_specific(TSDKeyType key);

    static Uint32 set_thread_specific(TSDKeyType key, void* value);
};

//==============================================================================
//
// PEGASUS_HAVE_PTHREADS
//
//==============================================================================

#if defined(PEGASUS_HAVE_PTHREADS)

inline Uint32 TSDKey::create(TSDKeyType* key)
{
    return pthread_key_create(key, NULL);
}

inline Uint32 TSDKey::destroy(TSDKeyType key)
{
    return pthread_key_delete(key);
}

inline void * TSDKey::get_thread_specific(TSDKeyType key)
{
#ifdef PEGASUS_OS_ZOS
    return pthread_getspecific_d8_np(key);
#else
    return pthread_getspecific(key);
#endif
}

inline Uint32 TSDKey::set_thread_specific(TSDKeyType key, void* value)
{
    return pthread_setspecific(key, value);
}

#endif /* defined(PEGASUS_HAVE_PTHREADS) */

//==============================================================================
//
// PEGASUS_HAVE_WINDOWS_THREADS
//
//==============================================================================

#if defined(PEGASUS_HAVE_WINDOWS_THREADS)

inline Uint32 TSDKey::create(TSDKeyType* key)
{
    *key = TlsAlloc();

    if (*key == -1)
        return 1;
    else
        return 0;
}

inline Uint32 TSDKey::destroy(TSDKeyType key)
{
    if (TlsFree(key))
        return 0;
    else
        return 1;
}

inline void * TSDKey::get_thread_specific(TSDKeyType key)
{
    return TlsGetValue(key);
}

inline Uint32 TSDKey::set_thread_specific(TSDKeyType key, void * value)
{
    if (TlsSetValue(key, value))
        return 0;
    else
        return 1;
}

#endif /* defined(PEGASUS_HAVE_WINDOWS_THREADS) */

PEGASUS_NAMESPACE_END

#endif /* Pegasus_TSDKey_h */
