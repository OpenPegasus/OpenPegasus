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

#ifndef Pegasus_Stopwatch_h
#define Pegasus_Stopwatch_h

#include <Pegasus/Common/Config.h>

#include <Pegasus/General/Linkage.h>

PEGASUS_NAMESPACE_BEGIN

/** Stopwatch - A class for measuring elapsed time
    Stopwatch is a class for measuring time intervals within the environment.
    It is intended to be a developers tool primarily.
*/
class PEGASUS_GENERAL_LINKAGE Stopwatch
{
public:

    /** constructor. The constructor creates the object. Start
    must be called to start the timer.
    @example Stopwatch time;
    */
    Stopwatch();

    /** start - Starts accumulating time and continues until stop is called.
    The object can be started and stopped multiple times to measure the
    sum of several intervals, but each start must have a corresponding start.
    */
    void start();

    /** stop - Stops the accumlation of time for an interval. The object
    should only stopped if it has been started.
    */
    void stop();

    /** reset - Effectively clears the time values stored by a Stopwatch.
    */
    void reset();

    /** getElapsed - Get the elapsed time for the defined stopwatch. This
    method should only be called if it is currently stopped.
    @return Returns the elapsed time value as a double (in seconds).
    */
    double getElapsed() const;

    /** Get number of elapsed microseconds
    */
    Uint64 getElapsedUsec() const;

    /** printElapsed method gets the current value of the timer and
    sends it to standard out as a string with the word seconds attached
    */
    void printElapsed();

private:
    Uint64 _start;
    Uint64 _stop;
    Uint64 _total;
};

PEGASUS_NAMESPACE_END

#endif /* Pegasus_Stopwatch_h */
