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

/*!
    \file datetime.c
    \brief Native CMPIDateTime implementation.

    This is the native CMPIDateTime implementation as used for remote
    providers. It reflects the well-defined interface of a regular
    CMPIDateTime, however, it works independently from the management broker.

    It is part of a native broker implementation that simulates CMPI data
    types rather than interacting with the entities in a full-grown CIMOM.

*/
#include "cmpir_common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <Pegasus/Provider/CMPI/cmpipl.h>

#ifdef PEGASUS_OS_TYPE_WINDOWS
# include <winsock2.h>
# include <winbase.h>
# include <sys/types.h>
# include <sys/timeb.h>
#else
# include <sys/time.h>
#endif


#include "mm.h"
#include "native.h"
#include "debug.h"

#ifdef PEGASUS_OS_TYPE_WINDOWS
# define UINT64_LITERAL(x)  ((CMPIUint64)x)
#else
# define UINT64_LITERAL(x)  ((CMPIUint64)x##ULL)
#endif

//! Native extension of the CMPIDateTime data type.
/*!
    This structure stores the information needed to represent time for
    CMPI providers.
*/

// Adding this to the POSIX 1970 microseconds epoch produces a 1 BCE epoch
// as used by CIMDateTime. -V 5885
// This POSIX_1970_EPOCH_OFFSET value should be same as the
// POSIX_1970_EPOCH_OFFSET value found in CIMDateTime.cpp

// ATTN: Need to find the way to sync changes made to POSIX_1970_EPOCH_OFFSET
// value in CIMDateTime.cpp here.

static const CMPIUint64 POSIX_1970_EPOCH_OFFSET  =
    UINT64_LITERAL(62167219200000000);

/* Number of microseconds in one minute.*/
static const CMPIUint64 MICROSECONDS_IN_MINUTE =  60000000;

/* Number of microseconds in one Hour */
static const CMPIUint64 MICROSECONDS_IN_HOUR = UINT64_LITERAL(3600000000);

#ifdef PEGASUS_OS_TYPE_WINDOWS
struct timezone
{
    int tz_minuteswest;
    int tz_dsttime;
};
#endif

struct native_datetime
{
    CMPIDateTime dt;    /*! < the inheriting data structure  */
    int mem_state;      /*! < states, whether this object is
                            registered within the memory mangagement or
                            represents a cloned object */

    CMPIUint64 msecs;   /*! < microseconds since 01/01/1970 00:00  */
    CMPIBoolean interval;   /*! < states if the date-time object is to be
                                treated as an interval or as absolute time */
};

static struct native_datetime * __new_datetime (
    int,
    CMPIUint64,
    CMPIBoolean,
    CMPIStatus * );

/****************************************************************************/

//! Releases a previously cloned CMPIDateTime object from memory.
/*!
    To achieve this, the object is simply added to the thread-based memory
    management to be freed later.

    \param dt the object to be released

    \return CMPI_RC_OK.
*/
static CMPIStatus __dtft_release ( CMPIDateTime * dt )
{
    struct native_datetime * ndt = (struct native_datetime *) dt;

    CMPIStatus rc = checkArgsReturnStatus(dt);

    if (rc.rc == CMPI_RC_OK && ndt->mem_state == TOOL_MM_NO_ADD)
    {
        ndt->mem_state = TOOL_MM_ADD;
        tool_mm_add ( ndt );
    }

    return rc;
}

//! Clones an existing CMPIDateTime object.
/*!
    The function simply calls __new_datetime() with the data fields
    extracted from dt.

    \param dt the object to be cloned
    \param rc return code pointer

    \return a copy of the given CMPIDateTime object that won't be freed
    from memory before calling __dtft_release().
*/
static CMPIDateTime * __dtft_clone ( CONST CMPIDateTime * dt, CMPIStatus * rc )
{
    struct native_datetime * ndt   = (struct native_datetime *) dt;
    struct native_datetime * new;

    if (!checkArgs(dt, rc))
    {
        return 0;
    }
    new = __new_datetime (
        TOOL_MM_NO_ADD,
        ndt->msecs,
        ndt->interval,
        rc );

    return(CMPIDateTime *) new;
}

//! Extracts the binary time from the encapsulated CMPIDateTime object.
/*!
    \param dt the native CMPIDateTime to be extracted.
    \param rc return code pointer

    \return an amount of microseconds.
*/
static CMPIUint64 __dtft_getBinaryFormat (
    CONST CMPIDateTime * dt,
    CMPIStatus * rc )
{
    struct native_datetime * ndt   = (struct native_datetime *) dt;

    if (!checkArgs(dt, rc))
    {
        return 0;
    }

    CMSetStatus ( rc, CMPI_RC_OK );
    return ndt->msecs;
}

//! Gives back a string representation of the time object.
/*!
    \param dt the native CMPIDateTime to be converted.
    \param rc return code pointer

    \return a string that has one of the following formats:

    - yyyymmddhhmmss.mmmmmmsutc (for absolute times)
    - ddddddddhhmmss.mmmmmm:000 (for time intervals)
*/
static CMPIString * __dtft_getStringFormat (
    CONST CMPIDateTime * dt,
    CMPIStatus * rc )
{
#ifdef PEGASUS_OS_TYPE_WINDOWS
    FILETIME FTime;
    SYSTEMTIME STime;
#endif

    struct native_datetime * ndt   = (struct native_datetime *) dt;

    time_t secs;
    unsigned long usecs;
    char str_time[26];

    if (!checkArgs(dt, rc))
    {
        return 0;
    }
    secs  = (time_t) ndt->msecs / 1000000;
    usecs = (unsigned long) ndt->msecs % 1000000;
    if (ndt->interval)
    {

        unsigned long mins, hrs, days;

        mins  = (unsigned long) secs / 60;
        secs %= 60;
        hrs   = (unsigned long) mins / 60;
        mins %= 60;
        days  = (unsigned long) hrs / 24;
        hrs  %= 24;

        sprintf ( str_time, "%8.8ld%2.2ld%2.2ld%2.2ld.%6.6ld:000",
            days, hrs, mins, secs, usecs );

    }
    else
    {
#ifndef PEGASUS_OS_TYPE_WINDOWS
        struct tm tm_time;
        char us_utc_time[11];

        if (localtime_r ( &secs, &tm_time ) == NULL)
        {

            CMSetStatus ( rc, CMPI_RC_ERR_FAILED );
            return NULL;
        }

        tzset ();
#ifdef PEGASUS_OS_ZOS
        sprintf (
            us_utc_time,
            "%6.6ld%+4.3ld",
            usecs,
            ( daylight != 0 ) * 60 - timezone / 60 );
#else
        snprintf (
            us_utc_time,
            11,
            "%6.6ld%+4.3ld",
            usecs,
            ( daylight != 0 ) * 60 - timezone / 60 );
#endif
        strftime ( str_time, 26, "%Y%m%d%H%M%S.", &tm_time );

        strcat ( str_time, us_utc_time );

#else
        FTime.dwLowDateTime = (DWORD) ndt->msecs;

        FileTimeToSystemTime(&FTime,&STime);
        sprintf(
            str_time,
            "%04u%02u%02u%02u%02u%02u.%06u+000",
            STime.wYear,
            STime.wMonth,
            STime.wDay,
            STime.wHour,
            STime.wMinute,
            STime.wSecond,
            STime.wMilliseconds);
#endif
    }
    return native_new_CMPIString ( str_time, rc );
}


//! States, whether the time object represents an interval.
/*!
    \param dt the native CMPIDateTime to be checked.
    \param rc return code pointer

    \return zero, if it is an absolute time, non-zero for intervals.
*/
static CMPIBoolean __dtft_isInterval (
    CONST CMPIDateTime * dt,
    CMPIStatus * rc )
{
    struct native_datetime * ndt   = (struct native_datetime *) dt;
    if (!checkArgs(dt, rc))
    {
        return 0;
    }
    CMSetStatus ( rc, CMPI_RC_OK );
    return ndt->interval;
}


//! Creates a new native_datetime object.
/*!
    The newly allocated object's function table is initialized to point
    to the native functions in this file.

    \param mm_add TOOL_MM_ADD for a regular object, TOOL_MM_NO_ADD for
                  cloned ones
    \param msecs the binary time to be stored
    \param interval the interval flag to be stored
    \param rc return code pointer

    \return a fully initialized native_datetime object pointer.
*/
static struct native_datetime * __new_datetime (
    int mm_add,
    CMPIUint64 msecs,
    CMPIBoolean interval,
    CMPIStatus * rc )
{
    static CMPIDateTimeFT dtft = {
        NATIVE_FT_VERSION,
        __dtft_release,
        __dtft_clone,
        __dtft_getBinaryFormat,
        __dtft_getStringFormat,
        __dtft_isInterval
    };

    static CMPIDateTime dt = {
        "CMPIDateTime",
        &dtft
    };

    struct native_datetime * ndt =
        (struct native_datetime *)
        tool_mm_alloc ( mm_add, sizeof ( struct native_datetime ) );

    ndt->dt = dt;
    ndt->mem_state = mm_add;
    ndt->msecs = msecs;
    ndt->interval = interval;

    CMSetStatus ( rc, CMPI_RC_OK );
    return ndt;
}


//! Creates a native CMPIDateTime representing the current time.
/*!
    This function calculates the current time and stores it within
    a new native_datetime object.

    \param rc return code pointer

    \return a pointer to a native CMPIDateTime.
*/

#ifdef PEGASUS_OS_TYPE_WINDOWS

int gettimeofday (struct timeval *tv, char *unUsed )
{
    struct _timeb timeBuffer;

    _ftime( &timeBuffer );
    tv->tv_sec = timeBuffer.time;
    tv->tv_usec = ( timeBuffer.millitm * 1000 );

    return 0;
}

int getCurrentTimeZone()
{
    TIME_ZONE_INFORMATION timezone;
    int currentTimeZone = 0;
    memset(&timezone, 0, sizeof(timezone));

    switch (GetTimeZoneInformation(&timezone))
    {
        case TIME_ZONE_ID_UNKNOWN:
            {
                currentTimeZone = timezone.Bias;
                break;
            }

        case TIME_ZONE_ID_STANDARD:
            {
                currentTimeZone = (timezone.Bias + timezone.StandardBias);
                break;
            }

        case TIME_ZONE_ID_DAYLIGHT:
            {
                currentTimeZone = (timezone.Bias + timezone.DaylightBias);
                break;
            }

        case TIME_ZONE_ID_INVALID:
        {
            TRACE_CRITICAL(("Invalid Time zone returned by "
                "GetTimeZoneInformation()"));
            break;
        }
        default:
            break;
    }

    // the bias used to calculate the time zone is a factor that is used to
    // determine the UTC time from the local time. to get the UTC offset from
    // the local time, use the inverse.

    if (currentTimeZone != 0)
    {
        currentTimeZone *= -1;
    }

    return currentTimeZone;
}

#endif

CMPIDateTime * native_new_CMPIDateTime ( CMPIStatus * rc )
{
    // Get sec and usec:
    time_t sec;
    CMPIUint64 usec;
    // ATTN: if this fails on your platform, use time() to obtain the
    // sec element and set usec to zero.
    struct timeval tv;

#if defined(PEGASUS_OS_VMS)
    void *tz = NULL;
#else
    struct timezone tz;
#endif
    int tzMinutesEast=0;
    struct tm* tmval;
    struct tm tmvalBuffer;
    int utcOffset;    /* UTC offset */
    int sign;   /* '-' or '+' for time stamps */
    CMPIUint64 hours;
    CMPIUint64 minutes;

    gettimeofday(&tv, &tz);
    sec = tv.tv_sec;
    usec = (CMPIUint64) tv.tv_usec;

#ifndef PEGASUS_OS_TYPE_WINDOWS
    // Get the localtime
    tmval = localtime_r(&sec, &tmvalBuffer);
#endif

    // Calculate minutes East of GMT.
    {
#if defined(PEGASUS_OS_SOLARIS)
        tzMinutesEast =
            -(int)((tmval->tm_isdst > 0 && daylight) ?
            altzone :
            timezone) / 60;
#elif defined(PEGASUS_OS_HPUX)
        tzMinutesEast = - (int) timezone / 60;
        if ((tmval->tm_isdst > 0) && daylight)
        {
            // ATTN: It is unclear how to determine the DST offset.
            // Assume 1 hour.
            tzMinutesEast += 60;
        }
#elif defined(PEGASUS_OS_LINUX) || defined(PEGASUS_OS_VMS)
        tzMinutesEast = (int) tmval->tm_gmtoff/60;
#elif defined (PEGASUS_OS_TYPE_WINDOWS)
        tzMinutesEast=getCurrentTimeZone();
#else
        tzMinutesEast = -tz.tz_minuteswest;
        if (tz.tz_dsttime > 0)
        {
            // ATTN: It is unclear how to determine the DST offset.
            // Assume 1 hour.
            tzMinutesEast += 60;
        }
#endif
    }

    usec = POSIX_1970_EPOCH_OFFSET +
        (CMPIUint64)(sec + tzMinutesEast * 60) * (CMPIUint64) 1000000 +
        (CMPIUint64) usec;

    /* Normalize the time */

    sign = tzMinutesEast < 0 ? '-' : '+';
    utcOffset = tzMinutesEast < 0 ? -tzMinutesEast : tzMinutesEast;
    hours = (utcOffset / 60) * MICROSECONDS_IN_HOUR;
    minutes = (utcOffset % 60) * MICROSECONDS_IN_MINUTE;
    if (sign == '+')
    {
        usec -= hours + minutes;
    }
    else
    {
        usec += hours + minutes;
    }

    return(CMPIDateTime *) __new_datetime (
        TOOL_MM_ADD,
        usec,
        0,
        rc );
}

//! Creates a native CMPIDateTime given a fixed binary time.
/*!
    This calls is simply passed on to __new_datetime().

    \param time fixed time-stamp in microseconds
    \param interval states, if the time-stamp is to be treated as interval
    \param rc return code pointer

    \return a pointer to a native CMPIDateTime.

    \sa __dtft_getBinaryFormat()
 */
CMPIDateTime * native_new_CMPIDateTime_fromBinary (
    CMPIUint64 time,
    CMPIBoolean interval,
    CMPIStatus * rc )
{
    return(CMPIDateTime *) __new_datetime (
        TOOL_MM_ADD,
        time,
        interval,
        rc );
}


//! Creates a native CMPIDateTime given a fixed time in string representation.
/*!
    This function assumes the given string to have one of the following formats:

    - for absolute times: yyyymmddhhmmss.mmmmmmsutc
    - for time intervals: ddddddddhhmmss.mmmmmm:000

    \param string the time to be converted into internal representation
    \param rc return code pointer

    \return a pointer to a native CMPIDateTime.

    \sa __dtft_getStringFormat()
*/
CMPIDateTime * native_new_CMPIDateTime_fromChars (
    const char * string,
    CMPIStatus * rc )
{
    CMPIUint64 msecs;
    CMPIBoolean interval;
    char * str;
    //String length must be 25.
    if (!string || strlen(string) != 25)
    {
        CMSetStatus (rc, CMPI_RC_ERR_INVALID_PARAMETER);
        return NULL;
    }
    interval = ( string[21] == ':' );
    str = strdup ( string );

    str[21] = 0;
    //msec =  atoll (str + 15) or _atoli64 (str + 15)
    msecs  = PEGASUS_CMPIR_ATOL ( str + 15 );
    str[14] = 0;
    msecs += PEGASUS_CMPIR_ATOL ( str + 12 ) * 1000000;
    str[12] = 0;
    msecs += PEGASUS_CMPIR_ATOL ( str + 10 ) * 1000000 * 60;
    str[10] = 0;
    msecs += PEGASUS_CMPIR_ATOL ( str + 8 )  * 1000000 * 60 * 60;
    str[8]  = 0;

    if (interval)
    {
        msecs += PEGASUS_CMPIR_ATOL ( str )  * 1000000 * 60 * 60 * 24;
    }
    else
    {
        struct tm tmp;

        memset ( &tmp, 0, sizeof ( struct tm ) );
        tzset ();
#ifndef PEGASUS_OS_ZOS
# ifdef PEGASUS_OS_TYPE_UNIX
        tmp.tm_gmtoff = timezone;
# endif
#endif
        tmp.tm_isdst  = daylight;
        tmp.tm_mday   = atoi ( str + 6 );
        str[6] = 0;
        tmp.tm_mon    = atoi ( str + 4 ) - 1;
        str[4] = 0;
        tmp.tm_year   = atoi ( str ) - 1900;

        msecs += (CMPIUint64) mktime ( &tmp ) * 1000000;
    }

    free ( str );

    return(CMPIDateTime *)
    __new_datetime ( TOOL_MM_ADD, msecs, interval, rc );
}


/****************************************************************************/

/*** Local Variables:  ***/
/*** mode: C           ***/
/*** c-basic-offset: 8 ***/
/*** End:              ***/
