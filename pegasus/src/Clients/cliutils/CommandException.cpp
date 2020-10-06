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
#include "CommandException.h"

PEGASUS_NAMESPACE_BEGIN

/**

    Constructs a CommandException with the specified message.

    @param  exceptionMessage  a string containing the exception message

 */
CommandException::CommandException (const String& exceptionMessage) : Exception
    (exceptionMessage)
{
}
//l10n
CommandException::CommandException (MessageLoaderParms &parms) : Exception
    (parms)
{
}
/**

    Constructs a CommandFormatException with the specified message.

    @param  exceptionMessage  a string containing the exception message

 */
CommandFormatException::CommandFormatException (const String& exceptionMessage)
   : CommandException (exceptionMessage)
{
}
//l10n
CommandFormatException::CommandFormatException (MessageLoaderParms &parms)
   : CommandException (parms)
{
}
/**

    First part of exception message string indicating a duplicate option
    appeared in the command line.

 */
const char DuplicateOptionException::_MESSAGE_DUPLICATE_OPTION1 [] =
    "duplicate \"-";

/**

    Second part of exception message string indicating a duplicate option
    appeared in the command line.

 */
const char DuplicateOptionException::_MESSAGE_DUPLICATE_OPTION2 [] =
    "\" option";

/**

    Constructs a DuplicateOptionException using the value of the duplicate
    option character.

    @param  duplicateOption  the character representing the duplicate option

 */
DuplicateOptionException::DuplicateOptionException (char duplicateOption) :
    CommandFormatException (String ())
{
    //l10n
    //_rep->message = _MESSAGE_DUPLICATE_OPTION1;
    //_rep->message.append (duplicateOption);
    //_rep->message.append (_MESSAGE_DUPLICATE_OPTION2);
    MessageLoaderParms parms(
            "Clients.cliutils.CommandException.DUPLICATE_OPTION",
            "duplicate \"-$0\" option",
            String().append(duplicateOption));
    _rep->message.append(MessageLoader::getMessage(parms));
}

/**

    First part of exception message string indicating an invalid option
    argument appeared in the command line.

 */
const char InvalidOptionArgumentException::_MESSAGE_INVALID_ARG1 [] =
    "argument \"";

/**

    Second part of exception message string indicating an invalid option
    argument appeared in the command line.

 */
const char InvalidOptionArgumentException::_MESSAGE_INVALID_ARG2 [] =
    "\" is not valid for option \"-";

/**

    Third part of exception message string indicating an invalid option
    argument appeared in the command line.

 */
const char InvalidOptionArgumentException::_MESSAGE_INVALID_ARG3 [] = "\"";

/**

    Constructs an InvalidOptionArgumentException using the values of the
    invalid option argument string and the option character.

    @param  invalidArgument  the string containing the invalid option
                             argument

    @param  option           the character representing the option

 */
InvalidOptionArgumentException::InvalidOptionArgumentException
    (const String& invalidArgument, char option) : CommandFormatException
        (String ())
{
    //l10n
    //_rep->message = _MESSAGE_INVALID_ARG1;
    //_rep->message.append (invalidArgument);
    //_rep->message.append (_MESSAGE_INVALID_ARG2);
    //_rep->message.append (option);
    //_rep->message.append (_MESSAGE_INVALID_ARG3);
    MessageLoaderParms parms("Clients.cliutils.CommandException.INVALID_ARG",
                             "argument \"$0\" is not valid for option \"-$1\"",
                             invalidArgument,
                             String().append(option));
    _rep->message.append(MessageLoader::getMessage(parms));


}

/**

    First part of exception message string indicating an invalid option
    appeared in the command line.

 */
const char InvalidOptionException::_MESSAGE_INVALID_OPTION1 [] = "option \"-";

/**

    Second part of exception message string indicating an invalid option
    appeared in the command line.

 */
const char InvalidOptionException::_MESSAGE_INVALID_OPTION2 [] =
    "\" is not valid for this command";

/**

    Constructs an InvalidOptionException using the value of the invalid
    option character.

    @param  invalidOption  the character representing the invalid option

 */
InvalidOptionException::InvalidOptionException (char invalidOption) :
    CommandFormatException (String ())
{
    //l10n
    //_rep->message = _MESSAGE_INVALID_OPTION1;
    //_rep->message.append (invalidOption);
    //_rep->message.append (_MESSAGE_INVALID_OPTION2);
    MessageLoaderParms parms("Clients.cliutils.CommandException.INVALID_OPTION",
                             "option \"-$0\" is not valid for this command",
                             String().append(invalidOption));
    _rep->message.append(MessageLoader::getMessage(parms));
}

/**

    First part of exception message string indicating a required option
    argument missing from the command line.

 */
const char MissingOptionArgumentException::_MESSAGE_MISSING_OPTARG1 [] =
    "missing argument value for \"-";

/**

    Second part of exception message string indicating a required option
    argument missing from the command line.

 */
const char MissingOptionArgumentException::_MESSAGE_MISSING_OPTARG2 [] =
    "\" option";

/**

    Constructs a MissingOptionArgumentException using the value of the
    option character whose argument is missing.

    @param  option  the character representing the option whose argument is
                    missing

 */
MissingOptionArgumentException::MissingOptionArgumentException (char option) :
    CommandFormatException (String ())
{
    //l10n
    //_rep->message = _MESSAGE_MISSING_OPTARG1;
    //_rep->message.append (option);
    //_rep->message.append (_MESSAGE_MISSING_OPTARG2);
    MessageLoaderParms parms(
        "Clients.cliutils.CommandException.MISSING_OPTION_ARG",
        "missing argument value for \"-$0\" option",
        String().append(option));
    _rep->message.append(MessageLoader::getMessage(parms));
}

/**

    First part of exception message string indicating a required option
    is missing from the command line.

 */
const char MissingOptionException::_MESSAGE_MISSING_OPTION1 [] = "the \"-";

/**

    Second part of exception message string indicating a required option
    is missing from the command line.

 */
const char MissingOptionException::_MESSAGE_MISSING_OPTION2 [] =
    "\" option is required";

/**

    Constructs a MissingOptionException using the value of the missing
    required option character.

    @param  missingOption  the character representing the missing required
                           option

 */
MissingOptionException::MissingOptionException (char missingOption) :
    CommandFormatException (String ())
{
    //l10n
    //_rep->message = _MESSAGE_MISSING_OPTION1;
    //_rep->message.append (missingOption);
    //_rep->message.append (_MESSAGE_MISSING_OPTION2);
    MessageLoaderParms parms("Clients.cliutils.CommandException.MISSING_OPTION",
                             "the \"-$0\" option is required",
                             String().append(missingOption));
    _rep->message.append(MessageLoader::getMessage(parms));
}

/**

    First part of exception message string indicating an unexpected
    argument appeared in the command line.

 */
const char UnexpectedArgumentException::_MESSAGE_UNEXPECTED_ARG1 [] =
    "argument \"";

/**

    Second part of exception message string indicating an unexpected
    argument appeared in the command line.

 */
const char UnexpectedArgumentException::_MESSAGE_UNEXPECTED_ARG2 [] =
    "\" was unexpected";

/**

    Constructs an UnexpectedArgumentException using the value of the
    argument string.

    @param  argumentValue  the string containing the unexpected argument

 */
UnexpectedArgumentException::UnexpectedArgumentException
    (const String& argumentValue) : CommandFormatException (String ())
{
    //l10n
    //_rep->message = _MESSAGE_UNEXPECTED_ARG1;
    //_rep->message.append (argumentValue);
    //_rep->message.append (_MESSAGE_UNEXPECTED_ARG2);
    MessageLoaderParms parms("Clients.cliutils.CommandException.UNEXPECTED_ARG",
                             "argument \"$0\" was unexpected",
                             argumentValue);
    _rep->message.append(MessageLoader::getMessage(parms));
}

/**

    First part of exception message string indicating an unexpected option
    appeared in the command line.

 */
const char UnexpectedOptionException::_MESSAGE_UNEXPECTED_OPT1 [] =
    "option \"-";

/**

    Second part of exception message string indicating an unexpected option
    appeared in the command line.

 */
const char UnexpectedOptionException::_MESSAGE_UNEXPECTED_OPT2 [] =
    "\" was unexpected";

/**

    Constructs an UnexpectedOptionException using the value of the
    unexpected option character.

    @param  optionValue  the character representing the option that was
                         unexpected

 */
UnexpectedOptionException::UnexpectedOptionException (char optionValue) :
    CommandFormatException (String ())
{
    //l10n
    //_rep->message = _MESSAGE_UNEXPECTED_OPT1;
    //_rep->message.append (optionValue);
    //_rep->message.append (_MESSAGE_UNEXPECTED_OPT2);
    MessageLoaderParms parms(
        "Clients.cliutils.CommandException.UNEXPECTED_OPTION",
        "option \"-$0\" was unexpected",
        String().append(optionValue));
    _rep->message.append(MessageLoader::getMessage(parms));
}

/**

    Constructs an UnexpectedOptionException using the value of the
    unexpected option string.

    @param  optionValue  the string representing the long option that was
                         unexpected

 */
UnexpectedOptionException::UnexpectedOptionException(const String& optionValue)
    :
    CommandFormatException (String ())
{
    MessageLoaderParms parms(
        "Clients.cliutils.CommandException.UNEXPECTED_OPTION",
        "option \"-$0\" was unexpected",
        optionValue);
    _rep->message.append(MessageLoader::getMessage(parms));
}

PEGASUS_NAMESPACE_END
