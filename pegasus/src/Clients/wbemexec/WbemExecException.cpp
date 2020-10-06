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

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/ExceptionRep.h>
#include "WbemExecException.h"

PEGASUS_NAMESPACE_BEGIN


/**

    Default exception identifier.  This identifier is used if the specified
    identifier is out of range.  This identifier corresponds to a default
    (generic) message in the array of exception message strings.

 */
const Uint32 WbemExecException::DEFAULT_ID = 0;

/**

    Minimum valid exception identifier.

 */
const Uint32 WbemExecException::MIN_ID = DEFAULT_ID;

/**

    Exception identifier indicating the specified input file does not
    exist.

 */
const Uint32 WbemExecException::MPOST_HTTP10_INVALID= 1;

/**

    Exception identifier indicating the specified input file does not
    exist.

 */
const Uint32 WbemExecException::INPUT_FILE_NONEXISTENT = 2;

/**

    Exception identifier indicating the specified input file cannot be
    read.

 */
const Uint32 WbemExecException::INPUT_FILE_NOT_READABLE = 3;

/**

    Exception identifier indicating the specified input file cannot be
    opened.

 */
const Uint32 WbemExecException::INPUT_FILE_CANNOT_OPEN = 4;

/**

    Exception identifier indicating that no input was provided.

 */
const Uint32 WbemExecException::NO_INPUT = 5;

/**

    Exception identifier indicating failure to make an HTTP connection.

 */
const Uint32 WbemExecException::CONNECT_FAIL = 6;

/**

    Exception identifier indicating timed out waiting for response.

 */
const Uint32 WbemExecException::TIMED_OUT = 7;

/**

    Exception identifier indicating invalid XML input.

 */
const Uint32 WbemExecException::INVALID_XML = 8;

/**

    Exception identifier indicating invalid input.

 */
const Uint32 WbemExecException::INVALID_INPUT = 9;

/**

    Maximum valid exception identifier.  This value must be updated when
    a new exception identifier and message are added.

 */
const Uint32 WbemExecException::MAX_ID = WbemExecException::INVALID_INPUT;

/**

    Exception message strings.  The exception identifier is used as an
    index into this array to retrieve the appropriate exception message
    string.  When a new identifier is added, this array must be updated
    appropriately.

 */
const char*  WbemExecException::_messageStrings [] =
{
    "Error in wbemexec command ",
    "M-POST method invalid with HTTP version 1.0 ",
    "Unable to use requested input file: file does not exist ",
    "Unable to use requested input file: file is not readable ",
    "Unable to use requested input file: file cannot be opened ",
    "No input ",
    "Failed to connect to CIM server: ",
    "Timed out waiting for response ",
    "Invalid XML request: ",
    "Invalid input: expected XML request"
};

/**

    Constructs a WbemExecException with a message corresponding to the
    specified exception ID.

    @param  ID                the integer exception identifier

 */
WbemExecException::WbemExecException (Uint32 ID) : CommandException
    (_messageStrings [(ID > MAX_ID) ? DEFAULT_ID : ID])
{
}

/**

    Constructs a WbemExecException with a message corresponding to the
    specified ID, appended with the specified String.

    @param  ID                the integer exception identifier
    @param  appendString      the string to append to the exception message

 */
WbemExecException::WbemExecException (Uint32 ID, const String& appendString) :
    CommandException (_messageStrings
        [(ID > MAX_ID) ? DEFAULT_ID : ID])
{
    _rep->message.append (appendString);
}

/**

    Constructs a WbemExecException with the specified message.

    @param  exceptionMessage  a string containing the exception message

 */
WbemExecException::WbemExecException (const String& exceptionMessage) :
    CommandException (exceptionMessage)
{
}

PEGASUS_NAMESPACE_END
