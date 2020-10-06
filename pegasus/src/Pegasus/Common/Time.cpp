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

#include "Time.h"

#if defined(PEGASUS_OS_TYPE_WINDOWS)
# include <windows.h>
# include <sys/timeb.h>
#else
# define USE_GETTIMEOFDAY
#endif

#if defined(USE_NTP_GETTIME)
# include <sys/timex.h>
# include <errno.h>
#endif

#if defined(USE_GETTIMEOFDAY)
# include <sys/time.h>
# include <errno.h>
#endif

PEGASUS_NAMESPACE_BEGIN

int Time::gettimeofday(timeval* tv)
{
#if defined(USE_GETTIMEOFDAY)

    if (tv == NULL)
        return EINVAL;

    struct timeval tmp;

    if (::gettimeofday(&tmp, NULL) == 0)
    {
        tv->tv_sec = tmp.tv_sec;
        tv->tv_usec = tmp.tv_usec;
        return 0;
    }

    return -1;

#elif defined(USE_NTP_GETTIME)

    if (tv == NULL)
        return EINVAL;

    struct ntptimeval ntp;

    if (ntp_gettime(&ntp) == 0)
    {
        tv->tv_sec = ntp.time.tv_sec;
        tv->tv_usec = ntp.time.tv_usec;
        return 0;
    }

    return -1;

#elif defined(PEGASUS_OS_TYPE_WINDOWS)

    struct _timeb timebuffer;

    if (tv == NULL)
        return -1;

    _ftime(&timebuffer);
    tv->tv_sec = long(timebuffer.time);
    tv->tv_usec = timebuffer.millitm * 1000;

    return 0;

#endif
}

int Time::subtract(timeval* result, timeval* x, timeval* y)
{
    /* Perform the carry for the later subtraction by updating Y. */
    if (x->tv_usec < y->tv_usec)
    {
        int nsec = (y->tv_usec - x->tv_usec) / 1000000 + 1;
        y->tv_usec -= 1000000 * nsec;
        y->tv_sec += nsec;
    }
    if (x->tv_usec - y->tv_usec > 1000000)
    {
        int nsec = (x->tv_usec - y->tv_usec) / 1000000;
        y->tv_usec += 1000000 * nsec;
        y->tv_sec -= nsec;
    }

    /* Compute the time remaining to wait.
        `tv_usec' is certainly positive. */
    result->tv_sec = x->tv_sec - y->tv_sec;
    result->tv_usec = x->tv_usec - y->tv_usec;

    /* Return 1 if result is negative. */
    return x->tv_sec < y->tv_sec;
}

PEGASUS_NAMESPACE_END
