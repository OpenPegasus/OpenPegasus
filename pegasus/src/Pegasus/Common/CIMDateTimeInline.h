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

#ifndef Pegasus_CIMDateTimeInline_h
#define Pegasus_CIMDateTimeInline_h

#include <cstring>
#include <cassert>
#include "CIMDateTime.h"
#include "CIMDateTimeRep.h"

PEGASUS_USING_STD;

PEGASUS_NAMESPACE_BEGIN

// Julian day of "1 BCE January 1".
static const Uint32 JULIAN_ONE_BCE = 1721060;

// Number of microseconds in one second.
static const Uint64 SECOND = 1000000;

// Number of microseconds in one minute.
static const Uint64 MINUTE = 60 * SECOND;

// Number of microseconds in one hour.
static const Uint64 HOUR = 60 * MINUTE;

// Number of microseconds in one day.
static const Uint64 DAY = 24 * HOUR;

/** Convert month, day, and year to a Julian day (in the Gregorian calendar).
    Return julian day.
*/
static inline Uint32 _toJulianDay(Uint32 year, Uint32 month, Uint32 day)
{
    // Formula adapted from "FREQUENTLY ASKED QUESTIONS ABOUT CALENDARS"
    // (see http://www.tondering.dk/claus/calendar.html).

    int a = (14 - month)/12;
    int y = year+4800-a;
    int m = month + 12*a - 3;
    return day + (153*m+2)/5 + y*365 + y/4 - y/100 + y/400 - 32045;
}

/** Convert a Julian day number (in the Gregorian calendar) to year, month,
    and day.
*/
static inline void _fromJulianDay(
    Uint32 jd, Uint32& year, Uint32& month, Uint32& day)
{
    // Formula adapted from "FREQUENTLY ASKED QUESTIONS ABOUT CALENDARS"
    // (see http://www.tondering.dk/claus/calendar.html).

    int a = jd + 32044;
    int b = (4*a+3)/146097;
    int c = a - (b*146097)/4;
    int d = (4*c+3)/1461;
    int e = c - (1461*d)/4;
    int m = (5*e+2)/153;
    day   = e - (153*m+2)/5 + 1;
    month = m + 3 - 12*(m/10);
    year  = b*100 + d - 4800 + m/10;
}

static char _intToNumStrTable[][2] =
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
};


// generated string format: "%04u%02u%02u%02u%02u%02u.%06u%c%03u"
// Function is eight times faster than using sprintf()
static inline void _fullYearFormat(
    char buffer[26],
    Uint32 year,
    Uint32 month,
    Uint32 day,
    Uint32 hours,
    Uint32 minutes,
    Uint32 seconds,
    Uint32 microseconds,
    char sign,
    Uint32 utcOffset)
{
    // 48 = ASCII '0'
    buffer[0] = year/1000 + 48;
    buffer[1] = (year%1000)/100 + 48;
    buffer[2] = (year%100)/10 + 48;
    buffer[3] = (year%10) + 48;

    buffer[4] = _intToNumStrTable[month][0];
    buffer[5] = _intToNumStrTable[month][1];

    buffer[6] = _intToNumStrTable[day][0];
    buffer[7] = _intToNumStrTable[day][1];

    buffer[8] = _intToNumStrTable[hours][0];
    buffer[9] = _intToNumStrTable[hours][1];

    buffer[10] = _intToNumStrTable[minutes][0];
    buffer[11] = _intToNumStrTable[minutes][1];

    buffer[12] = _intToNumStrTable[seconds][0];
    buffer[13] = _intToNumStrTable[seconds][1];

    buffer[14] = '.';

    buffer[15] = (microseconds/100000) + 48;
    buffer[16] = (microseconds%100000)/10000 + 48;
    buffer[17] = (microseconds%10000)/1000 + 48;
    buffer[18] = (microseconds%1000)/100 + 48;
    buffer[19] = (microseconds%100)/10 + 48;
    buffer[20] = (microseconds%10) + 48;

    buffer[21] = sign;

    buffer[22] = (utcOffset/100) + 48;
    buffer[23] = (utcOffset%100)/10 + 48;
    buffer[24] = (utcOffset%10) + 48;

    buffer[25] = '\0';
}

// generated string format: ""%08u%02u%02u%02u.%06u:000"
// Function is eight times faster than using sprintf()
static inline void _fullDayFormat(
    char buffer[26],
    Uint32 day,
    Uint32 hours,
    Uint32 minutes,
    Uint32 seconds,
    Uint32 microseconds)
{
    // 48 = ASCII '0'
    buffer[0] = (day/10000000) + 48;
    buffer[1] = (day%10000000)/1000000 + 48;
    buffer[2] = (day%1000000)/100000 + 48;
    buffer[3] = (day%100000)/10000 + 48;
    buffer[4] = (day%10000)/1000 + 48;
    buffer[5] = (day%1000)/100 + 48;
    buffer[6] = (day%100)/10 + 48;
    buffer[7] = (day%10) + 48;

    buffer[8] = _intToNumStrTable[hours][0];
    buffer[9] = _intToNumStrTable[hours][1];

    buffer[10] = _intToNumStrTable[minutes][0];
    buffer[11] = _intToNumStrTable[minutes][1];

    buffer[12] = _intToNumStrTable[seconds][0];
    buffer[13] = _intToNumStrTable[seconds][1];

    buffer[14] = '.';

    buffer[15] = (microseconds/100000) + 48;
    buffer[16] = (microseconds%100000)/10000 + 48;
    buffer[17] = (microseconds%10000)/1000 + 48;
    buffer[18] = (microseconds%1000)/100 + 48;
    buffer[19] = (microseconds%100)/10 + 48;
    buffer[20] = (microseconds%10) + 48;

    buffer[21] = ':';
    buffer[22] = '0';
    buffer[23] = '0';
    buffer[24] = '0';
    buffer[25] = '\0';
}

/** Converts a CIMDateTimeRep representation to its canonical string
    representation as defined in the "CIM infrastructure Specification".
    Note that this implementation preserves any wildcard characters used
    to initially create the CIMDateTime object.
*/
static inline void _DateTimetoCStr(const CIMDateTimeRep& rep, char buffer[26])
{
    if (rep.sign == ':')
    {
        // Extract components:

        Uint64 usec = rep.usec;
        Uint32 microseconds = Uint32(usec % SECOND);
        Uint32 seconds = Uint32((usec / SECOND) % 60);
        Uint32 minutes = Uint32((usec / MINUTE) % 60);
        Uint32 hours = Uint32((usec / HOUR) % 24);
        Uint32 days = Uint32((usec / DAY));

        // Format the string.
        _fullDayFormat(
            buffer,
            days,
            hours,
            minutes,
            seconds,
            microseconds);
    }
    else
    {
        // Extract components:
        Uint64 usec = rep.usec;
        Uint32 microseconds = Uint32(usec % SECOND);
        Uint32 seconds = Uint32((usec / SECOND) % 60);
        Uint32 minutes = Uint32((usec / MINUTE) % 60);
        Uint32 hours = Uint32((usec / HOUR) % 24);
        Uint32 days = Uint32((usec / DAY));
        Uint32 jd = Uint32(days + JULIAN_ONE_BCE);

        // Convert back from julian to year/month/day:

        Uint32 year;
        Uint32 month;
        Uint32 day;
        _fromJulianDay(jd, year, month, day);

        // Format the string.
        _fullYearFormat(
            buffer,
            year,
            month,
            day,
            hours,
            minutes,
            seconds,
            microseconds,
            char(rep.sign),
            rep.utcOffset);
    }

    // Fill buffer with '*' chars (if any).
    if (rep.numWildcards > 0)
    {
        char* first = buffer + 20;
        char* last = buffer + 20 - rep.numWildcards;

        if (rep.numWildcards > 6)
            last--;

        for (; first != last; first--)
        {
            if (*first != '.')
                *first = '*';
        }
    }
}

PEGASUS_NAMESPACE_END

#endif /* Pegasus_CIMDateTimeInline_h */
