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

#ifndef Pegasus_benchmarkTestException_h
#define Pegasus_benchmarkTestException_h

#include <Clients/cliutils/CommandException.h>

PEGASUS_NAMESPACE_BEGIN

/**
    benchmarkTestException signals that an exception has occurred in executing
    the benchmarkTest command.
 */
class benchmarkTestException : public CommandException
{
public:
    /**

        Constructs an benchmarkTestException with a message corresponding to the
        specified exception ID.

        @param  ID                the integer exception identifier

     */
    benchmarkTestException (Uint32 ID);

    /**

        Constructs an benchmarkTestException with a message corresponding to the
        specified ID, appended with the specified String.

        @param  ID                the integer exception identifier
        @param  appendString      the string to append to the exception message

     */
    benchmarkTestException (Uint32 ID, const String& appendString);

    /**

        Constructs an benchmarkTestException with the specified message.

        @param  exceptionMessage  a string containing the exception message

     */
    benchmarkTestException (const String& exceptionMessage);

    //
    //  Exception identifiers
    //
    //  These are used to index into the messageStrings array to retrieve the
    //  appropriate exception message string.  When a new identifier is added,
    //  the messageStrings array and MAX_ID must be updated appropriately.
    //

    /**

        Default exception identifier.  This identifier is used if the specified
        identifier is out of range.  This identifier corresponds to a default
        (generic) message in the array of exception message strings.

     */
    static const Uint32 DEFAULT_ID;

    /**

        Minimum valid exception identifier.

     */
    static const Uint32 MIN_ID;

    /**

        Exception identifier indicating a connection failure.

     */

    static const Uint32 CONNECT_FAIL;

    /**

        Exception identifier indicating timed out waiting for response.

     */
    static const Uint32 TIMED_OUT;

    /**

        Exception identifier indicating invalid input.

     */
    static const Uint32 INVALID_INPUT;

    /**

        Maximum valid exception identifier.  This value must be updated when
        a new exception identifier and message are added.

     */
    static const Uint32 MAX_ID;

private:
    /**

        Exception message strings.  The exception identifier is used as an
        index into this array to retrieve the appropriate exception message
        string.  When a new identifier is added, this array must be updated
        appropriately.

     */
    static const char*  _messageStrings [];
};

PEGASUS_NAMESPACE_END

#endif  /* Pegasus_benchmarkTestException_h */
