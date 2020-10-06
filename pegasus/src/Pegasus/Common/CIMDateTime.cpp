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

#include <cstring>
#include <cassert>
#include <fstream>
#include "CIMDateTime.h"
#include "CIMDateTimeRep.h"
#include "CIMDateTimeInline.h"
#include "Exception.h"
#include "System.h"
#include "AutoPtr.h"
#include "PegasusAssert.h"
#include <time.h>

#if defined(PEGASUS_OS_TYPE_UNIX) || defined(PEGASUS_OS_VMS)
# include <sys/time.h>
#elif defined(PEGASUS_OS_TYPE_WINDOWS)
# include <sstream>
# include <iomanip>
# include <windows.h>
#else
# error "unsupported platform"
#endif

PEGASUS_USING_STD;

PEGASUS_NAMESPACE_BEGIN

#define PEGASUS_ARRAY_T CIMDateTime
# include "ArrayImpl.h"
#undef PEGASUS_ARRAY_T

//==============================================================================
//
// Local constants.
//
//==============================================================================

// Number of microseconds in ten thousand years.
static const Uint64 TEN_THOUSAND_YEARS =
    PEGASUS_UINT64_LITERAL(315569520000000000);

// Number of microseconds in one million days.
static const Uint64 HUNDRED_MILLION_DAYS =
    PEGASUS_UINT64_LITERAL(8640000000000000000);

// Adding this to the POSIX 1970 microseconds epoch produces a 1 BCE epoch
// as used by this class.
static const Uint64 POSIX_1970_EPOCH_OFFSET  =
    PEGASUS_UINT64_LITERAL(62167219200000000);

//==============================================================================
//
// Local functions.
//
//==============================================================================

/** Returns true if argument is a leap year.
*/
static inline bool _isLeapYear(Uint32 year)
{
    return year % 400 == 0 || (year % 4 == 0 && year % 100 != 0);
}

/** Calculates the number of days in a given month, accounting for leap year.
*/
static Uint32 _getDaysPerMonth(Uint32 year, Uint32 month)
{
    static char _daysPerMonth[] =
    {
        31, /* JAN */
        28, /* FEB */
        31, /* MAR */
        30, /* APR */
        31, /* MAY */
        30, /* JUN */
        31, /* JUL */
        31, /* AUG */
        30, /* SEP */
        31, /* OCT */
        30, /* NOV */
        31, /* DEC */
    };

    // If February:

    if (month == 2 && _isLeapYear(year))
        return 29;

    return _daysPerMonth[month - 1];
}


/** Optimized version of _strToUint32() for n=2 case.
*/
static inline bool _strToUint32_n2(const Uint16* s, Uint32& x)
{
    Uint32 c0 = s[0] - '0';

    if (c0 > 9)
        return false;

    Uint32 c1 = s[1] - '0';

    if (c1 > 9)
        return false;

    x = 10 * c0 + c1;

    return true;
}

/** Powers of ten.
*/
static const Uint32 _tens[] =
{
    1,
    10,
    100,
    1000,
    10000,
    100000,
    1000000,
    10000000,
};

/** Convert the next n digits to integer. Return true on success. Return
    false if a non-digit was encountered in the first n characters. Don't
    call with n > 8.
*/
static inline bool _strToUint32(const Uint16* s, size_t n, Uint32& x)
{
    switch (n)
    {
        case 2:
            return _strToUint32_n2(s, x);

        default:
        {
            x = 0;

            const Uint32* m = _tens;

            for (const Uint16* p = &s[n]; n--; )
            {
                Uint16 c = *--p - '0';

                if (c > 9)
                    return false;

                x += *m++ * c;
            }

            return true;
        }
    }
}

/** Parse the integer component pointed to by s. Return WILDCARD if s consists
    entirely of '*' characters. Returns the integer if it consists entirely
    of digits. Throw exception if digits and '*' are mixed. Also throw
    exception if digits are encountered when priorWildcards parameter is true.
*/
static inline Uint32 _parseComponent(
    const Uint16*& s, size_t n, bool& priorWildcards)
{
    // Check whether all characters are '*'.

    if (*s == '*')
    {
        bool allWild = true;

        for (size_t i = 0; i < n; i++)
        {
            if (s[i] != '*')
            {
                allWild = false;
                break;
            }
        }

        if (allWild)
        {
            s += n;
            priorWildcards = true;
            return Uint32(-1);
        }
    }

    if (priorWildcards)
        throw InvalidDateTimeFormatException();

    Uint32 x;

    if (!_strToUint32(s, n, x))
        throw InvalidDateTimeFormatException();

    s += n;
    return x;
}

/** Return true if all characters of the string are asterisks.
*/
static inline bool _allAsterisks(const Uint16* s, size_t n)
{
    for (size_t i = 0; i < n; i++)
    {
        if (s[i] != '*')
            return false;
    }

    return true;
}

/** Parse the microseconds component of the given string (6 characters).
    Set numSignificantMicrosecondDigits to the number of leading significant
    digits (non-asterisks). Note that once an asterisk is encountered, all
    subsequent characters must be asterisks. Returns the number of
    microseconds. Throws an exception if priorWildcards is true and any digits
    are encountered or if digits occurs after asterisks.
*/
static Uint32 _parseMicroseconds(
    const Uint16*& s,
    bool priorWildcards,
    Uint16& numSignificantDigits)
{
    static const Uint32 _mult[] = { 100000, 10000, 1000, 100, 10, 1, };

    // If wildcards encountered in previous components, then the first
    // character must be an asterisk.

    if (priorWildcards && s[0] != '*')
        throw InvalidDateTimeFormatException();

    // Examine characters left to right.

    numSignificantDigits = 0;
    Uint32 x = 0;

    for (size_t i = 0; i < 6; i++)
    {
        Uint32 c = s[i] - '0';

        if (c < 10)
        {
            // A digit:
            x += c * _mult[i];
        }
        else if (c == Uint32('*' - '0'))
        {
            // An asterisk:
            numSignificantDigits = Uint16(i);

            // All remaining characters must be asterisks.

            if (!_allAsterisks(s + i, 6 - i))
                throw InvalidDateTimeFormatException();

            s += 6;
            return x;
        }
        else
        {
            // An illegal character.
            throw InvalidDateTimeFormatException();
        }
    }

    numSignificantDigits = 6;
    s += 6;
    return x;
}

/** Similar to strcmp() but accounts for wildcards. Compares the first twenty
    five corresponding characters of s1 and s2. Returns the first non-zero
    difference, unless one of the characters is an asterisk, in which case
    it proceeds to the next character. The return value has the following
    meaning:

          0 : s1 is lexographically equal to s2
        < 0 : s1 is lexographically less than s2
        > 0 : s1 is lexographically greather than s2
*/
static int _matchTimeStampStrings(const char* s1, const char* s2)
{
    for (size_t i = 0; i < 25; i++)
    {
        char c1 = s1[i];
        char c2 = s2[i];

        if (c1 == '*' || c2 == '*')
            continue;

        int r = c1 - c2;

        if (r)
            return r;
    }

    // Identical
    return 0;
}

/** Normalize timestamps by including the utcOffset in the usec member and
    then setting utcOffset to zero.
*/
static inline void _normalize(CIMDateTimeRep* in)
{
    if (in->sign != ':')
    {
        // DDDDDDDDHHMMSS.MMMMMM:000

        Uint64 hours = (in->utcOffset / 60) * HOUR;
        Uint64 minutes = (in->utcOffset % 60) * MINUTE;

        // If minutes not wildcarded.
        // Else if hours not wildcarded.

        if (in->numWildcards < 10)
        {
            if (in->sign == '+')
                in->usec -= hours + minutes;
            else
                in->usec += hours + minutes;
        }
        else if (in->numWildcards < 12)
        {
            if (in->sign == '+')
                in->usec -= hours;
            else
                in->usec += hours;
        }

        in->utcOffset = 0;
        in->sign = '+';
    }
}

/** Converts the representation object to microseconds. For intervals, this
    quantity is the same as usec member. Time stamps are normalized so that
    the usec component contains the UTF offset.
*/
static Uint64 _toMicroSeconds(const CIMDateTimeRep* rep)
{
    if (rep->sign == ':')
        return rep->usec;

    CIMDateTimeRep tmp = *rep;
    _normalize(&tmp);
    return tmp.usec;
}

/** This table is used to convert integers between 0 and 99 (inclusive) to
    a char16 array, with zero padding.
*/
static Uint16 _intToStrTable[][2] =
{
    { '0', '0', },
    { '0', '1', },
    { '0', '2', },
    { '0', '3', },
    { '0', '4', },
    { '0', '5', },
    { '0', '6', },
    { '0', '7', },
    { '0', '8', },
    { '0', '9', },
    { '1', '0', },
    { '1', '1', },
    { '1', '2', },
    { '1', '3', },
    { '1', '4', },
    { '1', '5', },
    { '1', '6', },
    { '1', '7', },
    { '1', '8', },
    { '1', '9', },
    { '2', '0', },
    { '2', '1', },
    { '2', '2', },
    { '2', '3', },
    { '2', '4', },
    { '2', '5', },
    { '2', '6', },
    { '2', '7', },
    { '2', '8', },
    { '2', '9', },
    { '3', '0', },
    { '3', '1', },
    { '3', '2', },
    { '3', '3', },
    { '3', '4', },
    { '3', '5', },
    { '3', '6', },
    { '3', '7', },
    { '3', '8', },
    { '3', '9', },
    { '4', '0', },
    { '4', '1', },
    { '4', '2', },
    { '4', '3', },
    { '4', '4', },
    { '4', '5', },
    { '4', '6', },
    { '4', '7', },
    { '4', '8', },
    { '4', '9', },
    { '5', '0', },
    { '5', '1', },
    { '5', '2', },
    { '5', '3', },
    { '5', '4', },
    { '5', '5', },
    { '5', '6', },
    { '5', '7', },
    { '5', '8', },
    { '5', '9', },
    { '6', '0', },
    { '6', '1', },
    { '6', '2', },
    { '6', '3', },
    { '6', '4', },
    { '6', '5', },
    { '6', '6', },
    { '6', '7', },
    { '6', '8', },
    { '6', '9', },
    { '7', '0', },
    { '7', '1', },
    { '7', '2', },
    { '7', '3', },
    { '7', '4', },
    { '7', '5', },
    { '7', '6', },
    { '7', '7', },
    { '7', '8', },
    { '7', '9', },
    { '8', '0', },
    { '8', '1', },
    { '8', '2', },
    { '8', '3', },
    { '8', '4', },
    { '8', '5', },
    { '8', '6', },
    { '8', '7', },
    { '8', '8', },
    { '8', '9', },
    { '9', '0', },
    { '9', '1', },
    { '9', '2', },
    { '9', '3', },
    { '9', '4', },
    { '9', '5', },
    { '9', '6', },
    { '9', '7', },
    { '9', '8', },
    { '9', '9', },
};

/** Convert integer x to a zero-padded char16 string. Legal for x less than
    100000000.
*/
static inline void _intToChar16String(Uint32 x, Uint16*& str, size_t numDigits)
{
    if (numDigits == 2)
    {
        str[0] = _intToStrTable[x][0];
        str[1] = _intToStrTable[x][1];
        str += 2;
        return;
    }

    while (numDigits--)
    {
        Uint32 d = _tens[numDigits];
        Uint32 n = x / d;
        x %= d;
        *str++ = n + '0';
    }
}

static void _toChar16Str(const CIMDateTimeRep* rep, Char16* data_)
{
    Uint16* data = (Uint16*)data_;

    if (rep->sign == ':')
    {
        // DDDDDDDDHHMMSS.MMMMMM:000

        Uint64 usec = rep->usec;
        Uint32 microseconds = Uint32(usec % SECOND);
        Uint32 seconds = Uint32((usec / SECOND) % 60);
        Uint32 minutes = Uint32((usec / MINUTE) % 60);
        Uint32 hours = Uint32((usec / HOUR) % 24);
        Uint32 days = Uint32((usec / DAY));

        _intToChar16String(days, data, 8);
        _intToChar16String(hours, data, 2);
        _intToChar16String(minutes, data, 2);
        _intToChar16String(seconds, data, 2);
        *data++ = '.';
        _intToChar16String(microseconds, data, 6);
        data[0] = ':';
        data[1] = '0';
        data[2] = '0';
        data[3] = '0';
    }
    else
    {
        // YYYYMMDDHHMMSS.MMMMMMSUTC

        Uint64 usec = rep->usec;
        Uint32 microseconds = Uint32(usec % SECOND);
        Uint32 seconds = Uint32((usec / SECOND) % 60);
        Uint32 minutes = Uint32((usec / MINUTE) % 60);
        Uint32 hours = Uint32((usec / HOUR) % 24);
        Uint32 days = Uint32((usec / DAY));
        Uint32 jd = Uint32(days + JULIAN_ONE_BCE);
        Uint32 year;
        Uint32 month;
        Uint32 day;
        _fromJulianDay(jd, year, month, day);

        _intToChar16String(year, data, 4);
        _intToChar16String(month, data, 2);
        _intToChar16String(day, data, 2);
        _intToChar16String(hours, data, 2);
        _intToChar16String(minutes, data, 2);
        _intToChar16String(seconds, data, 2);
        *data++ = '.';
        _intToChar16String(microseconds, data, 6);
        *data++ = rep->sign;
        _intToChar16String(rep->utcOffset, data, 3);
    }

    // Fill buffer with '*' chars (if any).
    {
        Uint16* first = (Uint16*)data_ + 20;
        Uint16* last = (Uint16*)data_ + 20 - rep->numWildcards;

        if (rep->numWildcards > 6)
            last--;

        for (; first != last; first--)
        {
            if (*first != '.')
                *first = '*';
        }
    }
}

/** Compares the two CIMDateTime representations. The return value is one of
    the following.

        0   : x is equal to y
        < 0 : x is less than y
        > 0 : x is greater than y

    This function throws TypeMismatchException if x and y are not of the
    same type (time stamps or intervals).

    Algorithm: If both representations have zero numWildcards members, then
    the comparison is simply _toMicroSeconds(x) - _toMicroSeconds(y). If either
    has a non-zero numWildcards member, then they are converted to to canonical
    string format and compared lexographically with _matchTimeStampStrings().
    If so, then time stamps must be normalized (usec must be adjusted for the
    sign and utcOffset).
*/
static int _compare(const CIMDateTimeRep* x, const CIMDateTimeRep* y)
{
    bool xIsInterval = x->sign == ':';
    bool yIsInterval = y->sign == ':';

    if (xIsInterval != yIsInterval)
    {
        MessageLoaderParms parms(
            "Common.CIMDateTime.INVALID_OPERATION_COMP_DIF",
            "Trying to compare CIMDateTime objects of differing types");
        throw TypeMismatchException(parms);
    }

    if (x->numWildcards == 0 && y->numWildcards == 0)
    {
        Uint64 xm = _toMicroSeconds(x);
        Uint64 ym = _toMicroSeconds(y);

        if (xm < ym)
            return -1;
        else if (xm > ym)
            return 1;

        return 0;
    }
    else
    {
        if (!xIsInterval)
        {
            // Normalize before comparing.

            CIMDateTimeRep x1 = *x;
            _normalize(&x1);

            CIMDateTimeRep y1 = *y;
            _normalize(&y1);

            char s1[26];
            char s2[26];
            _DateTimetoCStr(x1, s1);
            _DateTimetoCStr(y1, s2);
            return _matchTimeStampStrings(s1, s2);
        }
        else
        {
            char s1[26];
            char s2[26];
            _DateTimetoCStr(*x, s1);
            _DateTimetoCStr(*y, s2);
            return _matchTimeStampStrings(s1, s2);
        }
    }
}

//==============================================================================
//
// CIMDateTime
//
//==============================================================================

const Uint32 CIMDateTime::WILDCARD = Uint32(-1);

CIMDateTime::CIMDateTime()
{
    _rep = new CIMDateTimeRep;
    memset(_rep, 0, sizeof(CIMDateTimeRep));
    _rep->sign = ':';
}

CIMDateTime::CIMDateTime(const CIMDateTime& x)
{
    _rep = new CIMDateTimeRep;
    memcpy(_rep, x._rep, sizeof(CIMDateTimeRep));
}

CIMDateTime::CIMDateTime(const String& str)
{
    _rep = new CIMDateTimeRep;
    AutoPtr<CIMDateTimeRep> autoRep(_rep);  // Prevent memory leak on exception
    set(str);
    autoRep.release();
}

CIMDateTime::CIMDateTime(Uint64 usec, Boolean isInterval)
{
    if (!isInterval && usec >= TEN_THOUSAND_YEARS)
    {
        MessageLoaderParms parms(
            "Common.Exception.DATETIME_OUT_OF_RANGE_EXCEPTION",
            "Cannot create a CIMDateTime time stamp beyond the year 10,000");
        throw DateTimeOutOfRangeException(parms);
    }

    if (isInterval && usec >= HUNDRED_MILLION_DAYS)
    {
        MessageLoaderParms parms(
            "Common.Exception.DATETIME_OUT_OF_RANGE_EXCEPTION",
            "Cannot create a CIMDateTime interval greater than 100 million "
            "days");
        throw DateTimeOutOfRangeException(parms);
    }

    _rep = new CIMDateTimeRep;
    _rep->usec = usec;
    _rep->utcOffset = 0;
    _rep->sign = isInterval ? ':' : '+';
    _rep->numWildcards = 0;
}

CIMDateTime::CIMDateTime(
    Uint32 year,
    Uint32 month,
    Uint32 day,
    Uint32 hours,
    Uint32 minutes,
    Uint32 seconds,
    Uint32 microseconds,
    Uint32 numSignificantMicrosecondDigits,
    Sint32 utcOffset)
{
    _rep = new CIMDateTimeRep;
    AutoPtr<CIMDateTimeRep> autoRep(_rep);  // Prevent memory leak on exception
    setTimeStamp(year, month, day, hours, minutes, seconds, microseconds,
        numSignificantMicrosecondDigits, utcOffset);
    autoRep.release();
}

CIMDateTime::CIMDateTime(
    Uint32 days,
    Uint32 hours,
    Uint32 minutes,
    Uint32 seconds,
    Uint32 microseconds,
    Uint32 numSignificantMicrosecondDigits)
{
    _rep = new CIMDateTimeRep;
    AutoPtr<CIMDateTimeRep> autoRep(_rep);  // Prevent memory leak on exception
    setInterval(days, hours, minutes, seconds, microseconds,
        numSignificantMicrosecondDigits);
    autoRep.release();
}

CIMDateTime::CIMDateTime(CIMDateTimeRep* rep) : _rep(rep)
{
}

CIMDateTime::CIMDateTime(const CIMDateTimeRep* x)
{
    _rep = new CIMDateTimeRep;
    memcpy(_rep, x, sizeof(CIMDateTimeRep));
}


CIMDateTime::~CIMDateTime()
{
    delete _rep;
}

CIMDateTime& CIMDateTime::operator=(const CIMDateTime& x)
{
    if (this != &x)
        memcpy(_rep, x._rep, sizeof(CIMDateTimeRep));

    return *this;
}

void CIMDateTime::clear()
{
    memset(_rep, 0, sizeof(CIMDateTimeRep));
    _rep->sign = ':';
}

void CIMDateTime::set(const String& str)
{
    clear();

    if (str.size() != 25)
        throw InvalidDateTimeFormatException();

    const Uint16* s = (const Uint16*)str.getChar16Data();
    Uint16 sign = s[21];

    if (sign == ':')
    {
        bool priorWildcards = false;

        // It's an interval of the form "DDDDDDDDHHMMSS.MMMMMM:000"

        // Parse days:

        Uint32 days = _parseComponent(s, 8, priorWildcards);
        Uint32 hours = _parseComponent(s, 2, priorWildcards);
        Uint32 minutes = _parseComponent(s, 2, priorWildcards);
        Uint32 seconds = _parseComponent(s, 2, priorWildcards);

        // Skip over dot:

        if (*s++ != '.')
            throw InvalidDateTimeFormatException();

        // Parse microseconds:

        Uint16 numSignificantMicrosecondDigits;
        Uint32 microseconds = _parseMicroseconds(
            s, priorWildcards, numSignificantMicrosecondDigits);

        // Skip over ':'.

        s++;

        // Expect "000".

        if (!(s[0] == '0' && s[1] == '0' && s[2] == '0'))
            throw InvalidDateTimeFormatException();

        // Set representation:

        setInterval(
            days,
            hours,
            minutes,
            seconds,
            microseconds,
            numSignificantMicrosecondDigits);
    }
    else if (sign == '-' || sign == '+')
    {
        bool priorWildcards = false;

        // It's a time stamp of the form "YYYYMMDDHHMMSS.MMMMMMSUTC"

        // Parse year, month, day, hours, minutes, seconds:

        Uint32 year = _parseComponent(s, 4, priorWildcards);
        Uint32 month = _parseComponent(s, 2, priorWildcards);
        Uint32 day = _parseComponent(s, 2, priorWildcards);
        Uint32 hours = _parseComponent(s, 2, priorWildcards);
        Uint32 minutes = _parseComponent(s, 2, priorWildcards);
        Uint32 seconds = _parseComponent(s, 2, priorWildcards);

        // Skip over dot:

        if (*s++ != '.')
            throw InvalidDateTimeFormatException();

        // Parse microseconds:

        Uint16 numSignificantMicrosecondDigits;
        Uint32 microseconds = _parseMicroseconds(
            s, priorWildcards, numSignificantMicrosecondDigits);

        // Skip over sign:

        s++;

        // Parse UTF offset.

        Uint32 utcOffset;

        if (!_strToUint32(s, 3, utcOffset))
            throw InvalidDateTimeFormatException();

        // Set representation:

        setTimeStamp(
            year,
            month,
            day,
            hours,
            minutes,
            seconds,
            microseconds,
            numSignificantMicrosecondDigits,
            sign == '+' ? utcOffset : -Sint16(utcOffset));
    }
    else
    {
        throw InvalidDateTimeFormatException();
    }
}

void CIMDateTime::setTimeStamp(
    Uint32 year,
    Uint32 month,
    Uint32 day,
    Uint32 hours,
    Uint32 minutes,
    Uint32 seconds,
    Uint32 microseconds,
    Uint32 numSignificantMicrosecondDigits,
    Sint32 utcOffset)
{
    clear();

    Uint32 numWildcards = 0;

    // Check Year:


    if (year == WILDCARD)
    {
        year = 0;
        numWildcards = 20;
    }
    else if (year > 9999)
    {
        MessageLoaderParms parms(
            "Common.Exception.DATETIME_OUT_OF_RANGE_EXCEPTION",
            "year is greater than 9999");
        throw DateTimeOutOfRangeException(parms);
    }

    // Check Month:

    if (month == WILDCARD)
    {
        month = 1;

        if (!numWildcards)
            numWildcards = 16;
    }
    else if (month < 1 || month > 12)
    {
        MessageLoaderParms parms(
            "Common.Exception.DATETIME_OUT_OF_RANGE_EXCEPTION",
            "illegal month number");
        throw DateTimeOutOfRangeException(parms);
    }

    // Check day:

    if (day == WILDCARD)
    {
        day = 1;

        if (!numWildcards)
            numWildcards = 14;
    }
    else if (day < 1 || day > _getDaysPerMonth(year, month))
    {
        MessageLoaderParms parms(
            "Common.Exception.DATETIME_OUT_OF_RANGE_EXCEPTION",
            "illegal day number");
        throw DateTimeOutOfRangeException(parms);
    }

    // Check hours:

    if (hours == WILDCARD)
    {
        hours = 0;

        if (!numWildcards)
            numWildcards = 12;
    }
    else if (hours > 23)
    {
        MessageLoaderParms parms(
            "Common.Exception.DATETIME_OUT_OF_RANGE_EXCEPTION",
            "illegal hours number ");
        throw DateTimeOutOfRangeException(parms);
    }

    // Check minutes:

    if (minutes == WILDCARD)
    {
        minutes = 0;

        if (!numWildcards)
            numWildcards = 10;
    }
    else if (minutes > 59)
    {
        MessageLoaderParms parms(
            "Common.Exception.DATETIME_OUT_OF_RANGE_EXCEPTION",
            "illegal minutes number ");
        throw DateTimeOutOfRangeException(parms);
    }

    // Check seconds:

    if (seconds == WILDCARD)
    {
        seconds = 0;

        if (!numWildcards)
            numWildcards = 8;
    }
    else if (seconds > 59)
    {
        MessageLoaderParms parms(
            "Common.Exception.DATETIME_OUT_OF_RANGE_EXCEPTION",
            "illegal seconds number ");
        throw DateTimeOutOfRangeException(parms);
    }

    // Check microseconds:

    if (numSignificantMicrosecondDigits > 6)
    {
        MessageLoaderParms parms(
            "Common.Exception.DATETIME_OUT_OF_RANGE_EXCEPTION",
            "bad numSignificantMicrosecondDigits (must fall between 0 and 6)");
        throw DateTimeOutOfRangeException(parms);
    }

    if (microseconds > 999999)
    {
        MessageLoaderParms parms(
            "Common.Exception.DATETIME_OUT_OF_RANGE_EXCEPTION",
            "microseconds number must be less than 999999");
        throw DateTimeOutOfRangeException(parms);
    }

    if (!numWildcards)
        numWildcards = 6 - numSignificantMicrosecondDigits;

    // Check UTC offset:

    if (utcOffset < -999 || utcOffset > 999)
    {
        MessageLoaderParms parms(
            "Common.Exception.DATETIME_OUT_OF_RANGE_EXCEPTION",
            "illegal utcOffset");
        throw DateTimeOutOfRangeException(parms);
    }

    // Set the representation.

    Uint32 days = _toJulianDay(year, month, day) - JULIAN_ONE_BCE;

    // Multiply in 64-bit to prevent overflow.
    _rep->usec =
        Uint64(microseconds) +
        Uint64((seconds * SECOND)) +
        Uint64((minutes * MINUTE)) +
        Uint64((hours * HOUR)) +
        Uint64((days * DAY));
    _rep->sign = utcOffset < 0 ? '-' : '+';
    _rep->utcOffset = utcOffset < 0 ? -utcOffset : utcOffset;
    _rep->numWildcards = numWildcards;
}

void CIMDateTime::setInterval(
    Uint32 days,
    Uint32 hours,
    Uint32 minutes,
    Uint32 seconds,
    Uint32 microseconds,
    Uint32 numSignificantMicrosecondDigits)
{
    clear();

    Uint32 numWildcards = 0;

    // Check days:

    if (days == WILDCARD)
    {
        days = 1;

        if (!numWildcards)
            numWildcards = 20;
    }
    else if (days > 99999999)
    {
        MessageLoaderParms parms(
            "Common.Exception.DATETIME_OUT_OF_RANGE_EXCEPTION",
            "illegal days number (must be less than 100000000");
        throw DateTimeOutOfRangeException(parms);
    }

    // Check hours:

    if (hours == WILDCARD)
    {
        hours = 0;

        if (!numWildcards)
            numWildcards = 12;
    }
    else if (hours > 23)
    {
        MessageLoaderParms parms(
            "Common.Exception.DATETIME_OUT_OF_RANGE_EXCEPTION",
            "illegal hours number ");
        throw DateTimeOutOfRangeException(parms);
    }

    // Check minutes:

    if (minutes == WILDCARD)
    {
        minutes = 0;

        if (!numWildcards)
            numWildcards = 10;
    }
    else if (minutes > 59)
    {
        MessageLoaderParms parms(
            "Common.Exception.DATETIME_OUT_OF_RANGE_EXCEPTION",
            "illegal minutes number ");
        throw DateTimeOutOfRangeException(parms);
    }

    // Check seconds:

    if (seconds == WILDCARD)
    {
        seconds = 0;

        if (!numWildcards)
            numWildcards = 8;
    }
    else if (seconds > 59)
    {
        MessageLoaderParms parms(
            "Common.Exception.DATETIME_OUT_OF_RANGE_EXCEPTION",
            "illegal seconds number ");
        throw DateTimeOutOfRangeException(parms);
    }

    // Check microseconds:

    if (numSignificantMicrosecondDigits > 6)
    {
        MessageLoaderParms parms(
            "Common.Exception.DATETIME_OUT_OF_RANGE_EXCEPTION",
            "bad numSignificantMicrosecondDigits (must fall between 0 and 6)");
        throw DateTimeOutOfRangeException(parms);
    }

    if (microseconds > 999999)
    {
        MessageLoaderParms parms(
            "Common.Exception.DATETIME_OUT_OF_RANGE_EXCEPTION",
            "microseconds number must be less than 999999");
        throw DateTimeOutOfRangeException(parms);
    }

    if (!numWildcards)
        numWildcards = 6 - numSignificantMicrosecondDigits;

    // Set the representation.

    _rep->usec =
        microseconds +
        (seconds * SECOND) +
        (minutes * MINUTE) +
        (hours * HOUR) +
        (days * DAY);
    _rep->sign = ':';
    _rep->utcOffset = 0;
    _rep->numWildcards = numWildcards;
}

String CIMDateTime::toString() const
{
    Char16 str[26];
    _toChar16Str(_rep, str);
    return String(str, 25);
}

Sint64 CIMDateTime::getDifference(CIMDateTime x, CIMDateTime y)
{
    if (x.isInterval() != y.isInterval())
        throw InvalidDateTimeFormatException();

    return y.toMicroSeconds() - x.toMicroSeconds();
}

Boolean CIMDateTime::isInterval() const
{
    return _rep->sign == ':';
}

Boolean CIMDateTime::isInterval()
{
    return _rep->sign == ':';
}

Boolean CIMDateTime::isTimeStamp() const
{
    return _rep->sign != ':';
}

Uint64 CIMDateTime::toMicroSeconds() const
{
    return _toMicroSeconds(_rep);
}

Boolean CIMDateTime::equal(const CIMDateTime& x) const
{
    return _compare(_rep, x._rep) == 0;
}

CIMDateTime CIMDateTime::operator+(const CIMDateTime& x) const
{
    CIMDateTime result(*this);
    return result+=(x);
}

CIMDateTime& CIMDateTime::operator+=(const CIMDateTime& x)
{
    // ATTN: check for overflow?

    if (!x.isInterval())
        throw TypeMismatchException();

    if (isInterval())
        _rep->usec += x._rep->usec;
    else
        _rep->usec += x.toMicroSeconds();

    return *this;
}

CIMDateTime CIMDateTime::operator-(const CIMDateTime& dt) const
{
    // ATTN: check for overflow?
    // ATTN: use operator-=()?

    if (isInterval() && !dt.isInterval())
        throw TypeMismatchException();

    Uint64 x = toMicroSeconds();
    Uint64 y = dt.toMicroSeconds();

    if (x < y)
    {
        MessageLoaderParms parms(
            "Common.Exception.DATETIME_OUT_OF_RANGE_EXCEPTION",
            "Result of subtracting two CIMDateTimes would be negative.");
        throw DateTimeOutOfRangeException(parms);
    }

    if (isInterval() == dt.isInterval())
    {
        // TIMESTAMP - TIMESTAMP
        // OR
        // INTERVAL - INTERVAL
        return CIMDateTime(x - y, true);
    }
    else
    {
        // TIMESTAMP - INTERVAL (INTERVAL - TIMESTAMP eliminated above).
        CIMDateTime tmp(x - y, false);
        tmp._rep->sign = _rep->sign;
        tmp._rep->utcOffset = _rep->utcOffset;
        tmp._rep->numWildcards = _rep->numWildcards;
        return tmp;
    }
}

CIMDateTime& CIMDateTime::operator-=(const CIMDateTime& x)
{
    // ATTN: check for overflow?

    if (!x.isInterval())
        throw TypeMismatchException();

    if (_rep->usec < x._rep->usec)
    {
        MessageLoaderParms parms(
            "Common.Exception.DATETIME_OUT_OF_RANGE_EXCEPTION",
            "Result of subtracting two CIMDateTimes would be negative.");
        throw DateTimeOutOfRangeException(parms);
    }

    if (isInterval())
        _rep->usec -= x._rep->usec;
    else
        _rep->usec -= x.toMicroSeconds();

    return *this;
}

CIMDateTime CIMDateTime::operator*(Uint64 x) const
{
    CIMDateTime result(*this);
    return result*=(x);
}

CIMDateTime& CIMDateTime::operator*=(Uint64 x)
{
    if (!isInterval())
        throw TypeMismatchException();

    _rep->usec *= x;
    return *this;
}

CIMDateTime CIMDateTime::operator/(Uint64 x) const
{
    CIMDateTime result(*this);
    return result/=(x);
}

CIMDateTime& CIMDateTime::operator/=(Uint64 x)
{
    if (!isInterval())
    {
        MessageLoaderParms parms(
            "Common.CIMDateTime.INVALID_OPERATION_DIV_INT",
            "Can not divide a TimeStamp by an integer");
        throw TypeMismatchException(parms);
    }

    if (x == 0)
    {
        MessageLoaderParms parms(
            "Common.CIMDateTime.INVALID_OPERATION_DIV_ZERO",
            "Can not divide CIMDateTime by zero");
        throw Exception(parms);
    }

    _rep->usec /= x;
    return *this;
}

Uint64 CIMDateTime::operator/(const CIMDateTime& x) const
{
    if (!isInterval() || !x.isInterval())
    {
        MessageLoaderParms parms(
            "Common.CIMDateTime.INVALID_OPERATION_DIV_TS",
            "Can not divide two CIMDateTime objects if one of them is "
            "a TimeStamp");
        throw TypeMismatchException(parms);
    }

    if (x._rep->usec == 0)
    {
        MessageLoaderParms parms(
            "Common.CIMDateTime.INVALID_OPERATION_DIV_ZERO",
            "Can not divide CIMDateTime by zero");
        throw Exception(parms);
    }

    return _rep->usec / x._rep->usec;
}

Boolean CIMDateTime::operator<(const CIMDateTime& x) const
{
    return _compare(_rep, x._rep) < 0;
}

Boolean CIMDateTime::operator<=(const CIMDateTime& x) const
{
    return _compare(_rep, x._rep) <= 0;
}

Boolean CIMDateTime::operator>(const CIMDateTime& x) const
{
    return _compare(_rep, x._rep) > 0;
}

Boolean CIMDateTime::operator>=(const CIMDateTime& x) const
{
    return _compare(_rep, x._rep) >= 0;
}

Boolean CIMDateTime::operator!=(const CIMDateTime& x) const
{
    return _compare(_rep, x._rep) != 0;
}

Boolean operator==(const CIMDateTime& x, const CIMDateTime& y)
{
    return x.equal(y);
}

//==============================================================================
//
// PEGASUS_OS_TYPE_UNIX
//
//==============================================================================

#if defined(PEGASUS_OS_TYPE_UNIX) || defined(PEGASUS_OS_VMS)

CIMDateTime CIMDateTime::getCurrentDateTime()
{
    // Get sec and usec:

    time_t sec;
    Uint64 usec;
    // ATTN: if this fails on your platform, use time() to obtain the
    // sec element and set usec to zero.
    struct timeval tv;
#if defined(PEGASUS_OS_VMS)
    void *tz = NULL;
#else
    struct timezone tz;
#endif
    gettimeofday(&tv, &tz);
    sec = tv.tv_sec;
    usec = Uint64(tv.tv_usec);

    // Get the localtime

    struct tm* tmval;
    struct tm tmvalBuffer;
    tmval = localtime_r(&sec, &tmvalBuffer);
    PEGASUS_ASSERT(tmval != 0);

    // Calculate minutes East of GMT.

    int tzMinutesEast;
    {
# if defined(PEGASUS_OS_SOLARIS)
        tzMinutesEast =
            -(int)((tmval->tm_isdst > 0 && daylight) ? altzone : timezone) / 60;
# elif defined(PEGASUS_OS_HPUX)
        tzMinutesEast = - (int) timezone / 60;
        if ((tmval->tm_isdst > 0) && daylight)
        {
            // ATTN: It is unclear how to determine the DST offset.
            // Assume 1 hour.
            tzMinutesEast += 60;
        }
# elif defined(PEGASUS_OS_LINUX) || defined(PEGASUS_OS_VMS)
        tzMinutesEast = (int) tmval->tm_gmtoff/60;
# else
        tzMinutesEast = -tz.tz_minuteswest;
        if (tz.tz_dsttime > 0)
        {
            // ATTN: It is unclear how to determine the DST offset.
            // Assume 1 hour.
            tzMinutesEast += 60;
        }
# endif
    }

    // Create the representation object.

    CIMDateTimeRep* rep = new CIMDateTimeRep;
    rep->usec =
        POSIX_1970_EPOCH_OFFSET +
        Uint64(sec + tzMinutesEast * 60) * Uint64(1000000) +
        Uint64(usec);
    rep->sign = tzMinutesEast < 0 ? '-' : '+';
    rep->utcOffset = tzMinutesEast < 0 ? -tzMinutesEast : tzMinutesEast;
    rep->numWildcards = 0;

    return CIMDateTime(rep);
}

#endif /* PEGASUS_OS_TYPE_UNIX */

//==============================================================================
//
// PEGASUS_OS_TYPE_WINDOWS
//
//==============================================================================

#if defined(PEGASUS_OS_TYPE_WINDOWS)

Boolean getCurrentTimeZone(Sint16& currentTimeZone)
{
    currentTimeZone = 0;
    TIME_ZONE_INFORMATION timezone;
    ::memset(&timezone, 0, sizeof(timezone));

    switch(::GetTimeZoneInformation(&timezone))
    {
        case TIME_ZONE_ID_UNKNOWN:
        {
            currentTimeZone = static_cast<Sint16>(timezone.Bias);
            break;
        }

        case TIME_ZONE_ID_STANDARD:
        {
            currentTimeZone =
                static_cast<Sint16>(timezone.Bias + timezone.StandardBias);
            break;
        }

        case TIME_ZONE_ID_DAYLIGHT:
        {
            currentTimeZone =
                static_cast<Sint16>(timezone.Bias + timezone.DaylightBias);
            break;
        }

        case TIME_ZONE_ID_INVALID:
        {
            MessageLoaderParms parms(
                "Common.CIMDateTime.INVALID_TIME_ZONE",
                "Invalid time zone information : $0",
                PEGASUS_SYSTEM_ERRORMSG_NLS);
            throw Exception(parms);
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

    return true;
}

CIMDateTime CIMDateTime::getCurrentDateTime()
{
    // Get system time.

    SYSTEMTIME time;
    memset(&time, 0, sizeof(time));
    GetLocalTime(&time);

    // Get UTC offset.

    Sint32 utcOffset = 0;
    Sint16 currentTimeZone;

    if (getCurrentTimeZone(currentTimeZone))
        utcOffset = currentTimeZone;

    // Create the CIMDateTime object.

    return CIMDateTime(
        time.wYear,
        time.wMonth,
        time.wDay,
        time.wHour,
        time.wMinute,
        time.wSecond,
        time.wMilliseconds * 1000,
        6,
        utcOffset);
}

#endif /* PEGASUS_OS_TYPE_WINDOWS */

/*
================================================================================

Notes:

    (1) The legacy implementation added the UTC offset when it was negative and
        substracted it when it was positive. I preserved this behavior but
        suspect it may be wrong.

    (2) Evenetually change getCurrentDateTime() to use constructor that takes
        a single microseconds component.

    (4) Add overflow checking for adds and multiplies.

================================================================================
*/

PEGASUS_NAMESPACE_END
