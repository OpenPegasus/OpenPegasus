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

#ifndef TimeZone_zOS_h
#define TimeZone_zOS_h

#include <env.h>
#include <time.h>

PEGASUS_NAMESPACE_BEGIN

/* Defines */
#define MICRO_PER_SEC (1000*1000)
#define STCK_SHIFT    12
#define STCK_MASK     0x000FFFFFFFFFFFFFull

/**
 Get the GMT offset (including daylight saving time)
*/

int getUtcOffset_zOS( void )
{
   int utcOffset = 0;
   char *pC;
   Sint64 cvtldto;

   pC = (char*) *((int*)(0x10));            // Address(CVT)
   pC = (char*) *((int*)(pC+0x148));        // Address(CVTEXT2)
   pC = pC + 0x38;                          // Address(CVTLDTO)
   cvtldto =   *((Uint64 *) pC);            // CVTLDTO
   cvtldto >>= STCK_SHIFT;                  // Now its in microseconds
   cvtldto =   cvtldto / MICRO_PER_SEC;     // Now it is in seconds
   utcOffset = cvtldto;
   return utcOffset;
};

/**
 Initialize the timezone information (TZ)

 When environment variable TZ is not set, the UTC offset is
 determined from the CVTLDTO control block and the value for
 TZ is set accordingly.
 Calls tzset() to initialize timezone information.

*/
void initialize_zOS_timezone()
{
    // Check if the TZ environment variable is already set,
    // indicating which timezone information we should use.
    const char* tzValue = getenv("TZ");
    if (!tzValue)
    {
        int utcOffset = getUtcOffset_zOS();

        char sign = (utcOffset>0) ? '-' : '+';
        utcOffset = abs(utcOffset);
        int hours = utcOffset / 3600;
        utcOffset = utcOffset - (hours*3600);
        int minutes = utcOffset / 60;
        int seconds = utcOffset - (minutes*60);

        char newTzValue[20]; // e.g "GMT+02:00:00"
        sprintf(newTzValue,
                "GMT%c%02d:%02d:%02d",
                sign, hours, minutes, seconds);

        setenv("TZ", newTzValue, 1);
    }
    // To activate the value of TZ for all threads, we need to call
    // tzset() once in the applications main thread.
    tzset();
}

PEGASUS_NAMESPACE_END

#endif


