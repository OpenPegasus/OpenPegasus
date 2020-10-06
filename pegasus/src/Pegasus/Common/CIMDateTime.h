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

#ifndef Pegasus_CIMDateTime_h
#define Pegasus_CIMDateTime_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/Array.h>
#include <Pegasus/Common/Linkage.h>

PEGASUS_NAMESPACE_BEGIN

struct CIMDateTimeRep;
class CMPISCMOUtilities;

/**
    The CIMDateTime class represents the CIM datetime data type as a C++ class
    CIMDateTime.  A CIM datetime may contain a time stamp or an interval.
    CIMDateTime is an intrinsic CIM data type that represents the time as a
    string with a fixed length.

    <PRE>
    A time stamp has the following form:
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

    Note:  Intervals always end in ":000".  This distinguishes intervals from
    time stamps.

    CIMDateTime objects are constructed from String objects or from
    other CIMDateTime objects.  Character strings must be exactly
    twenty-five characters in length and conform to either the time stamp
    or interval format.

    CIMDateTime objects that are not explicitly initialized will be
    implicitly initialized with a zero time interval:

    00000000000000.000000:000


    The following table shows what arithmetic operations are allowed
    between CIMDateTime types. The entries in the last four columns define
    the type of the result when the operation, specified in the column header,
    is performed on operands, of the types specified in the first two columns.

    <PRE>
    LHS - left hand side    TS - time stamp  int - integer
    RHS - right hand side   IV - interval
    X - operation not allowed between types

    LHS     RHS    +       -       *        /
    _____________________________________________
    TS      TS     X       IV      X       X
    TS      IV     TS      TS      X       X
    TS      int    X       X       X       X
    IV      IV     IV      IV      X       int
    IV      TS     X       X       X       X
    IV      int    X       X       IV      IV
    int     TS     X       X       X       X
    int     IV     X       X       X       X
    </PRE>

    The relational operators may only operate on two operands of the same type,
    i.e. two time stamps or two intervals.
*/
class PEGASUS_COMMON_LINKAGE CIMDateTime
{
public:

#ifdef PEGASUS_USE_EXPERIMENTAL_INTERFACES

    /** Wildcard parameter for component-based initializer member functions.
    */
    static const Uint32 WILDCARD;

#endif /* PEGASUS_USE_EXPERIMENTAL_INTERFACES */

    /** Creates a new CIMDateTime object with a zero interval value.
    */
    CIMDateTime();

    /** Creates a CIMDateTime object from another CIMDateTime object.
        @param x  Specifies the name of the CIMDateTime object to copy.
    */
    CIMDateTime(const CIMDateTime& x);

    /** Creates a new CIMDateTime object from a string constant representing
        the CIM DateTime formatted datetime.
        See the class documentation for CIMDateTime for the definition of the
        input string for absolute and interval datetime.
        @param str String object containing the CIMDateTime formatted string.
        This must contain twenty-five characters.
        @exception InvalidDateTimeFormatException If the input string is not
        formatted correctly.
    */
    CIMDateTime(const String& str);

    /** Creates a CIMDateTime object from an integer.
        @param usec For a time stamp, the number of microseconds since
        the epoch 0/0/0000 (12 am Jan 1, 1BCE); For an interval, the number
        of microseconds in the interval.
        @param isInterval Specifies whether the CIMDateTime object is to be
        created as an interval value (true) or a time stamp (false).
        @exception DateTimeOutOfRangeException If the microSec value is too
        large (greater than 317,455,200,000,000,000 for a time stamps or
        8,640,000,000,000,000,000 for an interval).
        @exception InvalidDateTimeFormatException If the CIMDateTime object is
        not formed correctly.
    */
    CIMDateTime(Uint64 usec, Boolean isInterval);

#ifdef PEGASUS_USE_EXPERIMENTAL_INTERFACES

    /** Create datetime time stamp from components.
        @param year zero-based year number (or CIMDateTime::WILDCARD)
        @param month number from 1 to 12 (or CIMDateTime::WILDCARD)
        @param day one-based day of the month (or CIMDateTime::WILDCARD)
        @param hours a number from 0 to 23 (or CIMDateTime::WILDCARD)
        @param minutes a number from 0 to 59 (or CIMDateTime::WILDCARD)
        @param seconds a number from 0 to 59 (or CIMDateTime::WILDCARD)
        @param microseconds a number from 0 to 999999
        @param numSignificantMicrosecondDigits the number of decimal digits of
            the microseconds parameter (from left to right) that are
            significant (all others are wildcarded) or six if they are all
            significant.
        @param UTF offset in minutes (negative or positive).
        @exception DateTimeOutOfRangeException.
    */
    CIMDateTime(
        Uint32 year,
        Uint32 month,
        Uint32 day,
        Uint32 hours,
        Uint32 minutes,
        Uint32 seconds,
        Uint32 microseconds,
        Uint32 numSignificantMicrosecondDigits,
        Sint32 utcOffset);

    /** Create datetime interval from components.
        @param days a number from 0 to 99999999 (or CIMDateTime::WILDCARD)
        @param hours a number from 0 to 23 (or CIMDateTime::WILDCARD)
        @param minutes a number from 0 to 59 (or CIMDateTime::WILDCARD)
        @param seconds a number from 0 to 59 (or CIMDateTime::WILDCARD)
        @param microseconds a number from 0 to 999999
        @param numSignificantMicrosecondDigits the number of decimal digits of
            the microseconds parameter (from left to right) that are
            significant (all others are wildcarded) or six if they are all
            significant.
        @exception DateTimeOutOfRangeException.
    */
    CIMDateTime(
        Uint32 days,
        Uint32 hours,
        Uint32 minutes,
        Uint32 seconds,
        Uint32 microseconds,
        Uint32 numSignificantMicrosecondDigits);

#endif /* PEGASUS_USE_EXPERIMENTAL_INTERFACES */

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
        Therefore, d1 is assigned the same "00000000000000.000000:000" value
        as d2.
    */
    CIMDateTime& operator=(const CIMDateTime& x);

    /** Returns a string representing the DateTime value of the
        CIMDateTime object.
        @return String representing the DateTime value.
    */
    String toString() const;

    /** Sets the datetime value from the input parameter.
        @param str String containing the new value in the datetime format
        (specified in the CIMDateTime class description).  For example, the
        following sets the date to December 24, 1999 and time to 12:00 P.M.
        EST.

        <PRE>
        CIMDateTime dt;
        dt.set("19991224120000.000000-300");
        </PRE>

        @exception InvalidDateTimeFormatException If the datetime String is not
        formatted correctly.
    */
    void set(const String & str);

#ifdef PEGASUS_USE_EXPERIMENTAL_INTERFACES

    /** Sets the datetime timestamp from individual components.
        @param year zero-based year number (or CIMDateTime::WILDCARD)
        @param month number from 1 to 12 (or CIMDateTime::WILDCARD)
        @param day one-based day of the month (or CIMDateTime::WILDCARD)
        @param hours a number from 0 to 23 (or CIMDateTime::WILDCARD)
        @param minutes a number from 0 to 59 (or CIMDateTime::WILDCARD)
        @param seconds a number from 0 to 59 (or CIMDateTime::WILDCARD)
        @param microseconds a number from 0 to 999999
        @param UTF offset in minutes (negative or positive).
        @param numSignificantMicrosecondDigits the number of decimal digits of
            the microseconds parameter (from left to right) that are
            significant (all others are wildcarded) or six if they are all
            significant.
        @exception DateTimeOutOfRangeException.
    */
    void setTimeStamp(
        Uint32 year,
        Uint32 month,
        Uint32 day,
        Uint32 hours,
        Uint32 minutes,
        Uint32 seconds,
        Uint32 microseconds,
        Uint32 numSignificantMicrosecondDigits,
        Sint32 utcOffset);

    /** Create datetime interval from components.
        @param days a number from 0 to 99999999
        @param hours a number from 0 to 23 (or CIMDateTime::WILDCARD)
        @param minutes a number from 0 to 59 (or CIMDateTime::WILDCARD)
        @param seconds a number from 0 to 59 (or CIMDateTime::WILDCARD)
        @param microseconds a number from 0 to 999999
        @param numSignificantMicrosecondDigits the number of decimal digits of
            the microseconds parameter (from left to right) that are
            significant (all others are wildcarded) or six if they are all
            significant.
        @exception DateTimeOutOfRangeException.
    */
    void setInterval(
        Uint32 days,
        Uint32 hours,
        Uint32 minutes,
        Uint32 seconds,
        Uint32 microseconds,
        Uint32 numSignificantMicrosecondDigits);

#endif /* PEGASUS_USE_EXPERIMENTAL_INTERFACES */

    /** Clears the datetime class object.  The date time is set to
        a zero interval value.
    */
    void clear();

    /** Returns the current local time in a CIMDateTime object.
        @return CIMDateTime object containing the current local date and time.
    */
    static CIMDateTime getCurrentDateTime();

    /** Computes the difference in microseconds between two CIMDateTime time
        stamps or two CIMDateTime intervals.
        @param startTime  Contains the start datetime.
        @param finishTime  Contains the finish datetime.
        @return An integer that contains the difference between the two
        datetime values (in microseconds).
        @exception InvalidDateTimeFormatException If arguments are not the same
        type of CIMDateTime.
    */
    static Sint64 getDifference(CIMDateTime startTime, CIMDateTime finishTime);

    /** Checks whether the datetime is an interval.
        @return True if the datetime is an interval value, false otherwise.
    */
    Boolean isInterval() const;

    /** Checks whether the datetime is an interval.  (This non-const form is
        maintained for compatibility.)
        @return True if the datetime is an interval value, false otherwise.
    */
    Boolean isInterval();

#ifdef PEGASUS_USE_EXPERIMENTAL_INTERFACES

    /** Checks whether the datetime is a timestamp.
        @return True if so.
    */
    Boolean isTimeStamp() const;

#endif /* PEGASUS_USE_EXPERIMENTAL_INTERFACES */

    /** Compares the CIMDateTime object to another CIMDateTime object for
        equality.
        @param x  CIMDateTime object to be compared.
        @return true if the two CIMDateTime objects are equal, false otherwise
        @exception TypeMismatchException If arguments are of different types.
    */
    Boolean equal(const CIMDateTime& x) const;

    /** Converts a CIMDateTime object to its microsecond representation.
        @return Number of microseconds since the epoch (for time stamps) or
        in a span of time (for intervals).
        @exception DateTimeOutOfRangeException If conversion to UTC (an
        internal operation) causes an overflow condition.
    */
    Uint64 toMicroSeconds() const;

    /** Adds two CIMDateTime objects and returns a CIMDateTime object that
        represents the sum.
        @param x operand on the RHS of the operator
        @return A CIMDateTime object that is the result of adding the calling
        object to the RHS operand
        @exception DateTimeOutOfRangeException If the operation causes an
        overflow condition.
        @exception TypeMismatchException If the operands are not type
        compatible (see table of operations).
    */
    CIMDateTime operator+(const CIMDateTime& x) const;

    /** Adds two CIMDateTime objects, returns the sum and changes
        the value of the calling CIMDateTime object to match the return value.
        @param x operand on the RHS of the operator
        @return A CIMDateTime object that is the result of adding the calling
        object to the RHS operand
        @exception DateTimeOutOfRangeException If the operation causes an
        overflow condition.
        @exception TypeMismatchException If the operands are not type
        compatible (see table of operations).
    */
    CIMDateTime & operator+=(const CIMDateTime& x);

    /** Subtracts one CIMDateTime object from another and returns a
        CIMDateTime object that represents the difference.
        @param x operand on the RHS of the operator
        @return A CIMDateTime object that is the result of subtracting the
        the RHS object from the calling.
        @exception DateTimeOutOfRangeException If the operation causes an
        underflow condition or conversion to UTC (an internal operation)
        causes an overflow condition.
        @exception TypeMismatchException If the operands are not type
        compatible (see table of operations).
    */
    CIMDateTime operator-(const CIMDateTime& x) const;

    /** Subtracts one CIMDateTime object from another, returns the difference
        and changes the value of the calling CIMDateTime object to match the
        return value.
        @param x operand on the RHS of the operator
        @return A CIMDateTime object that is the result of subtracting the
        object on the RHS from the calling object.
        @exception DateTimeOutOfRangeException If the operation causes an
        underflow condition or conversion to UTC (an internal operation)
        causes an overflow condition.
        @exception TypeMismatchException If the operands are not type
        compatible (see table of operations).
    */
    CIMDateTime & operator-=(const CIMDateTime& x);

    /** Multiplies a CIMDateTime object by an integer and returns a CIMDateTime
        object that represents the product.
        @param x integer operand on the RHS of the operator
        @return A CIMDateTime object that is the result of multiplying the
        calling object by the RHS operand.
        @exception DateTimeOutOfRangeException If the operation causes an
        overflow condition.
        @exception TypeMismatchException If the operands are not type
        compatible (see table of operations).
    */
    CIMDateTime operator*(Uint64 x) const;

    /** Multiplies a CIMDateTime object by an integer, returns the product
        and changes the value of the calling object to match the returned
        product.
        @param x integer operand on the RHS of the operator
        @return A CIMDateTime object that is the result of multiplying the
        calling object by the RHS operand.
        @exception DateTimeOutOfRangeException If the operation causes an
        overflow condition.
        @exception TypeMismatchException If the operands are not type
        compatible (see table of operations).
    */
    CIMDateTime & operator*=(Uint64 x);

    /** Divides a CIMDateTime object by an integer and returns a CIMDateTime
        object that represents the quotient.
        @param num integer operand on the RHS of the operator
        @return A CIMDateTime object that is the result of dividing the calling
        object by the RHS operand.
        @exception DateTimeOutOfRangeException If conversion to UTC (an
        internal operation) causes an overflow condition.
        @exception TypeMismatchException If the CIMDateTime object does not
        hold an interval value (see table of operations).
        @exception Exception if param num is zero.
    */
    CIMDateTime operator/(Uint64 num) const;

    /** Divides a CIMDateTime object by an integer, returns the quotient
        and changes the value of the calling object to match the returned
        quotient.
        @param num integer operand on the RHS of the operator
        @return A CIMDateTime object that is the result of dividing the calling
        object by the RHS operand.
        @exception DateTimeOutOfRangeException If conversion to UTC (an
        internal operation) causes an overflow condition.
        @exception TypeMismatchException If the CIMDateTime object does not
        hold an interval value (see table of operations).
        @exception Exception if param num is zero.
    */
    CIMDateTime & operator/=(Uint64 num);

    /** Divides a CIMDateTime object by another CIMDateTime object and returns
        an integer quotient.
        @param cdt CIMDateTime object on the RHS of the operator
        @return An integer that is the result of dividing the number of
        microseconds represented by the calling CIMDateTime object by the
        number of microseconds represented by the CIMDateTime object on the
        RHS of the operator.
        @exception DateTimeOutOfRangeException If conversion to UTC (an
        internal operation) causes an overflow condition.
        @exception TypeMismatchException If the operands are not type
        compatible (see table of operations).
    */
    Uint64 operator/(const CIMDateTime& cdt) const;

    /** Compare two CIMDateTime objects and returns true if the LHS is
        less than the RHS.
        @param x operand on the RHS of the operator
        @return true if the LHS is less than the RHS, false otherwise.
        @exception DateTimeOutOfRangeException If conversion to UTC (an
        internal operation) causes an overflow condition.
        @exception TypeMismatchException if operands are not of the same
        type.
     */
    Boolean operator<(const CIMDateTime& x) const;

    /** Compare two CIMDateTime objects and returns true if the LHS is
        less than or equal to the RHS.
        @param x operand on the RHS of the operator
        @return true if the LHS is less than or equal to the RHS, false
        otherwise.
        @exception DateTimeOutOfRangeException If conversion to UTC (an
        internal operation) causes an overflow condition.
        @exception TypeMismatchException if operands are not of the same
        type.
    */
    Boolean operator<=(const CIMDateTime& x) const;

    /** Compare two CIMDateTime objects and returns true if the LHS is
        greater than the RHS.
        @param x operand on the RHS of the operator
        @return true if the LHS is greater than the RHS, false otherwise.
        @exception DateTimeOutOfRangeException If conversion to UTC (an
        internal operation) causes an overflow condition.
        @exception TypeMismatchException if operands are not of the same
        type.
    */
    Boolean operator>(const CIMDateTime & x) const;

    /** Compare two CIMDateTime objects and returns true if the LHS is
        greater than or equal to the RHS.
        @param x operand on the RHS of the operator
        @return true if the LHS is greater than or equal to the RHS, false
        otherwise.
        @exception DateTimeOutOfRangeException If conversion to UTC (an
        internal operation) causes an overflow condition.
        @exception TypeMismatchException if operands are not of the same
        type.
    */
    Boolean operator>=(const CIMDateTime & x) const;

    /** Compare two CIMDateTime objects and returns true if the LHS is
        not equal to the RHS.
        @param x operand on the RHS of the operator
        @return true if the LHS is not equal to RHS, false otherwise.
        @exception DateTimeOutOfRangeException If conversion to UTC (an
        internal operation) causes an overflow condition.
        @exception TypeMismatchException if operands are not of the same
        type.
    */
    Boolean operator!=(const CIMDateTime & x) const;

private:
    CIMDateTimeRep* _rep;
    CIMDateTime(CIMDateTimeRep*);
    CIMDateTime(const CIMDateTimeRep*);

    friend class CIMBuffer;
    friend class SCMOClass;
    friend class SCMODump;
    friend class SCMOInstance;
    friend class CMPISCMOUtilities;
};

/** Compares two CIMDateTime objects and returns true if they represent the
    same time or length of time.
    @param x one of the CIMDateTime objects to be compared
    @param y one of the CIMDateTime objects to be compared
    @return true if the two objects passed in represent the same time or
    length of time, false otherwise.
*/
PEGASUS_COMMON_LINKAGE Boolean operator==(
    const CIMDateTime& x,
    const CIMDateTime& y);

#define PEGASUS_ARRAY_T CIMDateTime
# include <Pegasus/Common/ArrayInter.h>
#undef PEGASUS_ARRAY_T

PEGASUS_NAMESPACE_END

#endif /* Pegasus_CIMDateTime_h */
