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

#ifndef Pegasus_Once_h
#define Pegasus_Once_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/Linkage.h>
#include <Pegasus/Common/Mutex.h>

#define PEGASUS_ONCE_INITIALIZER { PEGASUS_MUTEX_INITIALIZER, 0 }

PEGASUS_NAMESPACE_BEGIN

/** Once implements the "once" concept as introduced by POSIX threads.
    That is, it arranges for a function to be called just once in a thread
    safe manner. The following example shows how to constuct an object of
    type X the first time any thread reaches the line that calls once().

        static Once _once = PEGASUS_ONCE_INITIALIZER;
        static static X* _ptr;

        static void _create_X()
        {
            ptr = new X;
        }

        ...

        once(&_once, _create_X);

    The _create_X() function is called exactly once no matter how many times
    once() is called on it. Also, once() may be called safely from multiple
    threads.

    CAUTION: Once instances must always be defined statically.
*/
struct Once
{
    MutexType mutex;
    int initialized;
};

void PEGASUS_COMMON_LINKAGE __once(Once* once, void (*function)());

inline void once(Once* once, void (*function)())
{
    if (once->initialized == 0)
        __once(once, function);
}

PEGASUS_NAMESPACE_END

#endif /* Pegasus_Once_h */
