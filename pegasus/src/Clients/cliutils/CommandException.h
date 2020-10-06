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
// Author: Carol Ann Krug Graves, Hewlett-Packard Company
//         (carolann_graves@hp.com)
//
// Modified By:
//
//%/////////////////////////////////////////////////////////////////////////////

#ifndef Pegasus_CommandException_h
#define Pegasus_CommandException_h

#include <Pegasus/Common/Exception.h>
#include <Clients/cliutils/Linkage.h>
#include <Pegasus/Common/MessageLoader.h>

PEGASUS_NAMESPACE_BEGIN

/**
    CommandException signals that an exception has occurred in executing the
    command.
 */
class PEGASUS_CLIUTILS_LINKAGE CommandException : public Exception
{
public:
    /**
        Constructs a CommandException with the specified message.

        @param  exceptionMessage  a string containing the exception message
     */
    CommandException(const String& exceptionMessage);

    CommandException(MessageLoaderParms &parms);
};

/**
    CommandFormatException signals that an exception has occurred in parsing
    the command line or validating the options and arguments.
 */
class PEGASUS_CLIUTILS_LINKAGE CommandFormatException : public CommandException
{
public:
    /**
        Constructs a CommandFormatException with the specified message.

        @param  exceptionMessage  a string containing the exception message
     */
    CommandFormatException(const String& exceptionMessage);

    CommandFormatException(MessageLoaderParms &parms);
};

/**
    DuplicateOptionException signals that the same option appeared more than
    once in the command line.
 */
class PEGASUS_CLIUTILS_LINKAGE DuplicateOptionException
    : public CommandFormatException
{
public:
    /**
        Constructs a DuplicateOptionException using the value of the duplicate
        option character.

        @param  duplicateOption  the character represnting the duplicate option
     */
    DuplicateOptionException(char duplicateOption);

private:
    /**
        First part of exception message string indicating a duplicate option
        appeared in the command line.
     */
    static const char _MESSAGE_DUPLICATE_OPTION1[];

    /**
        Second part of exception message string indicating a duplicate option
        appeared in the command line.
     */
    static const char _MESSAGE_DUPLICATE_OPTION2[];
};

/**
    InvalidOptionArgumentException signals that an invalid option argument
    appeared in the command line.
 */
class PEGASUS_CLIUTILS_LINKAGE InvalidOptionArgumentException
    : public CommandFormatException
{
public:
    /**
        Constructs an InvalidOptionArgumentException using the values of the
        invalid option argument string and the option character.

        @param  invalidArgument  the string containing the invalid option
                                 argument
        @param  option           the character representing the option
     */
    InvalidOptionArgumentException(
        const String& invalidArgument,
        char option);

private:
    /**
        First part of exception message string indicating an invalid option
        argument appeared in the command line.
     */
    static const char _MESSAGE_INVALID_ARG1[];

    /**
        Second part of exception message string indicating an invalid option
        argument appeared in the command line.
     */
    static const char _MESSAGE_INVALID_ARG2[];

    /**
        Third part of exception message string indicating an invalid option
        argument appeared in the command line.
     */
    static const char _MESSAGE_INVALID_ARG3[];
};

/**
    InvalidOptionException signals that an invalid option appeared in the
    command line.
 */
class PEGASUS_CLIUTILS_LINKAGE InvalidOptionException
    : public CommandFormatException
{
public:
    /**
        Constructs an InvalidOptionException using the value of the invalid
        option character.

        @param  invalidOption  the character representing the invalid option
     */
    InvalidOptionException(char invalidOption);

private:
    /**
        First part of exception message string indicating an invalid option
        appeared in the command line.
     */
    static const char _MESSAGE_INVALID_OPTION1[];

    /**
        Second part of exception message string indicating an invalid option
        appeared in the command line.
     */
    static const char _MESSAGE_INVALID_OPTION2[];
};

/**
    MissingOptionArgumentException signals that a required option argument is
    missing from the command line.
 */
class PEGASUS_CLIUTILS_LINKAGE MissingOptionArgumentException
    : public CommandFormatException
{
public:
    /**
        Constructs a MissingOptionArgumentException using the value of the
        option character whose argument is missing.

        @param option the character representing the option whose argument is
                      missing
     */
    MissingOptionArgumentException(char option);

private:
    /**
        First part of exception message string indicating a required option
        argument missing from the command line.
     */
    static const char _MESSAGE_MISSING_OPTARG1[];

    /**
        Second part of exception message string indicating a required option
        argument missing from the command line.
     */
    static const char _MESSAGE_MISSING_OPTARG2[];
};

/**
    MissingOptionException signals that a required option is missing from
    the command line.
 */
class PEGASUS_CLIUTILS_LINKAGE MissingOptionException
    : public CommandFormatException
{
public:

    /**
        Constructs a MissingOptionException using the value of the missing
        required option character.

        @param missingOption the character representing the missing required
                             option
     */
    MissingOptionException(char missingOption);

private:
    /**
        First part of exception message string indicating a required option
        is missing from the command line.
     */
    static const char _MESSAGE_MISSING_OPTION1[];

    /**

        Second part of exception message string indicating a required option
        is missing from the command line.

     */
    static const char _MESSAGE_MISSING_OPTION2[];
};

/**
    UnexpectedArgumentException signals that an unexpected argument value
    appeared in the command line.
 */
class PEGASUS_CLIUTILS_LINKAGE UnexpectedArgumentException
    : public CommandFormatException
{
public:
    /**
        Constructs an UnexpectedArgumentException using the value of the
        argument string.

        @param  argumentValue  the string containing the unexpected argument
     */
    UnexpectedArgumentException(const String& argumentValue);

private:
    /**
        First part of exception message string indicating an unexpected
        argument appeared in the command line.
     */
    static const char _MESSAGE_UNEXPECTED_ARG1[];

    /**
        Second part of exception message string indicating an unexpected
        argument appeared in the command line.
     */
    static const char _MESSAGE_UNEXPECTED_ARG2[];
};

/**
    UnexpectedOptionException signals that an unexpected option appeared
    in the command line.  An unexpected option is an option that is not
    consistent with other options already processed.
 */
class PEGASUS_CLIUTILS_LINKAGE UnexpectedOptionException
    : public CommandFormatException
{
public:
    /**
        Constructs an UnexpectedOptionException using the value of the
        unexpected option character.

        @param  optionValue  the character representing the option that was
                             unexpected
     */
    UnexpectedOptionException(char optionValue);

    /**
        Constructs an UnexpectedOptionException using the value of the
        unexpected option string.

        @param optionValue the string representing the option that was
                           unexpected
     */
    UnexpectedOptionException(const String& optionValue);

private:
    /**
        First part of exception message string indicating an unexpected option
        appeared in the command line.
     */
    static const char _MESSAGE_UNEXPECTED_OPT1[];

    /**
        Second part of exception message string indicating an unexpected option
        appeared in the command line.
     */
    static const char _MESSAGE_UNEXPECTED_OPT2[];
};

PEGASUS_NAMESPACE_END

#endif  /* Pegasus_CommandException_h */
