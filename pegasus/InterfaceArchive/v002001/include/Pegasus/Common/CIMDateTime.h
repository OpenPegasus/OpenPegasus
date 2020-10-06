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
    is an intrinsic CIM data type which represents the time as a formatted 
    fixed length string.

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
	s = '+' or '-' to represent the UTC sign
	utc = UTC offset (same as GMT offset)

    An interval has the following form:

	ddddddddhhmmss.mmmmmm:000

    Where

	dddddddd = days
	hh = hours (0-23)
	mm = minutes (0-59)
	ss = seconds (0-59)
	mmmmmm = microseconds
    </PRE>

    Note that intervals always end in ":000" (this is how they
    are distinguished from dates).

    CIMDateTime objects are constructed from String objects or from
    other CIMDateTime objects.  These character strings must be exactly
    twenty-five characters and conform to one of the forms identified
    above.

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
    */
    CIMDateTime(const String & str);

    /** Creates a CIMDateTime object from another CIMDateTime object.
	@param x  CIMDateTime object to be copied.
    */
    CIMDateTime(const CIMDateTime& x);

    /** CIMDateTime Destructor. */
    ~CIMDateTime();

    /** Assign one CIMDateTime object to another.
        @param x  The CIMDateTime Object to assign.
        <PRE>
            CIMDateTime d1;
            CIMDateTime d2 = "00000000000000.000000:000";
            d1 = d1;
        </PRE>
    */
    CIMDateTime& operator=(const CIMDateTime& x);

    /** Returns a string representing the DateTime value of the
        CIMDateTime Object.
        @return String representing the DateTime value.
    */
    String toString () const;

    /** Sets the date time in the CIMDateTime object from 
	the input parameter.
        @param str  String constant containing the datetime
	in the datetime format.

	    <PRE>
	    CIMDateTime dt;
	    dt.set("19991224120000.000000+360");
	    </PRE>

	@exception InvalidDateTimeFormatException on format error.
    */
    void set(const String & str);

    /** Clears the datetime class object.  The date time is set to 
        a zero interval value.
    */
    void clear();

    /** Get current time as CIMDateTime. The time returned is the local time.
        @return CIMDateTime object containing the current datetime.
    */
    static CIMDateTime getCurrentDateTime();

    /** Computes the difference in microseconds between two CIMDateTime dates or 
        two CIMDateTime intervals. 
        @param startTime  Contains the start datetime.
        @param finishTime  Contains the finish datetime.
        @return Interger containing the difference between the two datetimes 
        in microseconds.
        @exception InvalidDateTimeFormatException If one argument is a datetime
        and one is an interval.
        @exception DateTimeOutOfRangeException If datetime is outside the allowed
        range.

        NOTE: The behavior on HP-UX and Windows platform is to throw an exception
        when the dates are out of range. Red Hat Linux platform normalizes the
        dates when they are outside their legal interval and will not throw an
        exception.
    
        Allowed Date Range:
        The mktime (3C) man page on HP-UX does not document the allowed range. 
        The approximate range of dates allowed on HP-UX is between 
        1901 and 2038.
 
        On Windows platform, the approximate range is between 1970 to 2038. 

        On Red Hat Linux the approximate range of dates allowed are within the
        range of 1901 and 2038.
    */
    static Sint64 getDifference(CIMDateTime startTime, CIMDateTime finishTime);
 
    /** Checks whether the datetime is an interval.
        @return true if the datetime is an interval, false otherwise.
    */
    Boolean isInterval();

    /** Compares the CIMDateTime object to another CIMDateTime object for 
        equality.
        @param x  CIMDateTime object to be compared.
        @return true if the two CIMDateTime objects are equal, false otherwise.
    */
    Boolean equal (const CIMDateTime & x) const;

private:

    CIMDateTimeRep* _rep;

    Boolean _set(const String & dateTimeStr);
};

PEGASUS_COMMON_LINKAGE Boolean operator==(
    const CIMDateTime& x, 
    const CIMDateTime& y);

#define PEGASUS_ARRAY_T CIMDateTime
# include <Pegasus/Common/ArrayInter.h>
#undef PEGASUS_ARRAY_T

PEGASUS_NAMESPACE_END

#endif /* Pegasus_DateTime_h */
