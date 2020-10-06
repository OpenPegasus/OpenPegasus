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
/* NOCHKSRC */

//
// Author: Mike Brasher (mbrasher@bmc.com)
//
// Modified By: Karl Schopmeyer(k.schopmeyer@opengroup.org)
//              Sushma Fernandes, Hewlett Packard Company
//                  (sushma_fernandes@hp.com)
//              Roger Kumpf, Hewlett Packard Company (roger_kumpf@hp.com)
//              Carol Ann Krug Graves, Hewlett-Packard Company
//                (carolann_graves@hp.com)
//
//%/////////////////////////////////////////////////////////////////////////////

#ifndef Pegasus_DateTime_h
#define Pegasus_DateTime_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/Array.h>
#include <Pegasus/Common/Linkage.h>

PEGASUS_NAMESPACE_BEGIN

class CIMDateTimeRep;

/**
    The CIMDateTime class represents the CIM datetime data type as a C++ class
    CIMDateTime. A CIM datetime may contain a date or an interval. CIMDateTime
    is an intrinsic CIM data type that represents the time as a string with a
    fixed length.

    <PRE>
    A date has the following form:
    yyyymmddhhmmss.mmmmmmsutc

    Where

    yyyy = year (1-9999)
    mm = month (1-12)
    dd = day (1-31)
    hh = hour (0-23)
    mm = minute (0-59)
    ss = second (0-59)
    mmmmmm = microseconds
    s = '+' or '-' to represent the Coordinated Universal Time (UTC) sign
    utc = offset from Coordinated Universal Time (UTC)
        (same as Greenwich Mean Time(GMT) offset)

    An interval has the following form:

    ddddddddhhmmss.mmmmmm:000

    Where

    dddddddd = days
    hh = hours (0-23)
    mm = minutes (0-59)
    ss = seconds (0-59)
    mmmmmm = microseconds
    </PRE>

    Note: Intervals always end in ":000". This distinguishes intervals from dates.

    CIMDateTime objects are constructed from String objects or from
    other CIMDateTime objects.  Character strings must be exactly
    twenty-five characters in length and conform to either the date or interval
    format.

    CIMDateTime objects that are not explicitly initialized will be
    implicitly initialized with a zero time interval:

    00000000000000.000000:000

*/
class PEGASUS_COMMON_LINKAGE CIMDateTime
{
public:

    /** Creates a new CIMDateTime object with a zero interval value.
    */
    CIMDateTime();

    /** Creates a new CIMDateTime object from a string constant representing
        the CIM DateTime-formatted datetime.
        See the class documentation for CIMDateTime for the definition of the
        input string for absolute and interval datetime.
    @param str  String object containing the CIM DateTime-formatted string.
        This must contain twenty-five characters.
    */
    CIMDateTime(const String & str);

    /** Creates a CIMDateTime object from another CIMDateTime object.
    @param x  Specifies the name of the CIMDateTime object to copy.
    */
    CIMDateTime(const CIMDateTime& x);

    /** CIMDateTime destructor. */
    ~CIMDateTime();

    /** Assigns one instance of the CIMDateTime object to another.
        @param x  The CIMDateTime Object to assign to the CIMDateTime object.
        For example, you can assign the d1 CIMDateTime instance to the d2
        CIMDateTime instance.
        <PRE>
            CIMDateTime d1;
            CIMDateTime d2 = "00000000000000.000000:000";
            d1 = d2;
        </PRE>
        Therefore, d1 is assigned the same "00000000000000.000000:000" value as d2.
    */
    CIMDateTime& operator=(const CIMDateTime& x);

    /** Returns a string representing the DateTime value of the
        CIMDateTime object.
        @return String representing the DateTime value.
    */
    String toString () const;

    /** Sets the date and time in the CIMDateTime object from
    the input parameter.
        @param str  String constant containing the datetime
    in the datetime format. This must conform the to formatting rules specified
        in the CIMDateTime class description.  For example, the following sets
        the date to December 24, 1999 and time to 12:00 P.M.

        <PRE>
        CIMDateTime dt;
        dt.set("19991224120000.000000+360");
        </PRE>

    @exception InvalidDateTimeFormatException because the date and time is not
        formatted correctly.  See the CIMDateTime class decscription for the
        formatting rules.
    */
    void set(const String & str);

    /** Clears the datetime class object.  The date time is set to
        a zero interval value.
    */
    void clear();

    /** Receives the current time as CIMDateTime. The time is returned as the local time.
        @return CIMDateTime object containing the current date and time.
    */
    static CIMDateTime getCurrentDateTime();

    /** Computes the difference in microseconds between two CIMDateTime dates or
        two CIMDateTime intervals.
        @param startTime  Contains the start datetime.
        @param finishTime  Contains the finish datetime.
        @return Integer that contains the difference between the two datetime values
        in microseconds.
        @exception InvalidDateTimeFormatException If one argument uses the interval
        format and the other uses the string format.
        @exception DateTimeOutOfRangeException If datetime is outside the allowed
        range.

        Note: The behavior on HP-UX and Windows platform is to throw an exception
        when the dates are out of range. Red Hat Linux platform normalizes the
        dates when they are outside their legal interval and will not throw an
        exception.

        Allowed Date Range:
        The mktime (3C) man page on HP-UX does not document the allowed range.
        The approximate range of dates allowed on HP-UX is between
        1901 and 2038.

        On Windows platform, the approximate range is between 1970 to 2038.

        On Red Hat Linux platform the approximate range is between 1901 and 2038.
    */
    static Sint64 getDifference(CIMDateTime startTime, CIMDateTime finishTime);

    /** Checks whether the datetime is an interval.
        @return True if the datetime is an interval; otherwise, false.
    */
    Boolean isInterval();

    /** Compares the CIMDateTime object to another CIMDateTime object for
        equality.
        @param x  CIMDateTime object to be compared.
        @return true if the two CIMDateTime objects are equal; otherwise, false.
    */
    Boolean equal (const CIMDateTime & x) const;

private:

    CIMDateTimeRep* _rep;
    Boolean _set(const String & dateTimeStr);
};
/**
        Add documentation here.
    */
PEGASUS_COMMON_LINKAGE Boolean operator==(
    /**
        Add documentation here.
    */
    const CIMDateTime& x,
    /**
        Add documentation here.
    */
    const CIMDateTime& y);

#define PEGASUS_ARRAY_T CIMDateTime
# include <Pegasus/Common/ArrayInter.h>
#undef PEGASUS_ARRAY_T

PEGASUS_NAMESPACE_END

#endif /* Pegasus_DateTime_h */
