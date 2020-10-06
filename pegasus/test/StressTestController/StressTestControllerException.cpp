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
// Author: John Alex, IBM         (johnalex@us.ibm.com)
//
//
//%/////////////////////////////////////////////////////////////////////////////

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/Exception.h>
#include "StressTestControllerException.h"

PEGASUS_NAMESPACE_BEGIN


/**

    Default exception identifier.  This identifier is used if the specified
    identifier is out of range.  This identifier corresponds to a default
    (generic) message in the array of exception message strings.

 */
const Uint32 StressTestControllerException::DEFAULT_ID = 0;

/**

    Minimum valid exception identifier.

 */
const Uint32 StressTestControllerException::MIN_ID = DEFAULT_ID;

/**

    Exception identifier indicating "Syntax Error" with client option.

 */

const Uint32 StressTestControllerException::INVALID_OPTION = 1;

/**

    Exception identifier indicating "Syntax Error" with client option operator.

 */
const Uint32 StressTestControllerException::INVALID_OPERATOR = 2;

/**

    Exception identifier indicating "Missing closing square brace".

 */
const Uint32 StressTestControllerException::MISSING_BRACE = 3;

/**

    Exception identifier indicating "Missing value for client option".

 */
const Uint32 StressTestControllerException::MISSING_VALUE = 4;

/**

    Maximum valid exception identifier.  This value must be updated when
    a new exception identifier and message are added.

 */
const Uint32 StressTestControllerException::MAX_ID =
    StressTestControllerException::MISSING_VALUE;

/**

    Exception message strings.  The exception identifier is used as an
    index into this array to retrieve the appropriate exception message
    string.  When a new identifier is added, this array must be updated
    appropriately.

 */
const char*  StressTestControllerException::_messageStrings [] =
{
    "Error in TestStressTestController command: ",
    "Syntax Error Client option name: ",
    "Syntax Error Client option operator: ",
    "Missing closing square brace: ",
    "Missing value for client option: "
};

/**

    Constructs a StressTestControllerException with a message corresponding to
    the specified exception ID.

    @param  ID                the integer exception identifier

 */
StressTestControllerException::StressTestControllerException (
    Uint32 ID) : CommandException
                     (_messageStrings [(ID > MAX_ID) ? DEFAULT_ID : ID])
{
}

/**

    Constructs a StressTestControllerException with the specified message.

    @param  exceptionMessage  a string containing the exception message

 */
StressTestControllerException::StressTestControllerException (
    const String& exceptionMessage) : CommandException (exceptionMessage)
{
}

PEGASUS_NAMESPACE_END
