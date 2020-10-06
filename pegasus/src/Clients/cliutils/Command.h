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
//              (carolann_graves@hp.com)
//
// Modified By: David Dillard, VERITAS Software Corp.
//                  (david.dillard@veritas.com)
//
//%/////////////////////////////////////////////////////////////////////////////

#ifndef Pegasus_Command_h
#define Pegasus_Command_h

#include <iostream>
#include <Pegasus/Common/String.h>
#include <Clients/cliutils/Linkage.h>
#include <Clients/cliutils/CommandException.h>
#include <Pegasus/Common/MessageLoader.h> //l10n
#include <Pegasus/Common/Formatter.h> //l10n

PEGASUS_NAMESPACE_BEGIN

/**

    Command is the abstract base class for WBEM and CIM commands.

    @author  Hewlett-Packard Company

 */
class PEGASUS_CLIUTILS_LINKAGE Command
{
public:

    virtual ~Command();

    /**
        Gets command usage string.  Usage string may be retrieved and displayed
        when a CommandFormatException is caught.

        @return  String containing the command usage message
     */
    String& getUsage();

    /**
        Parses the command line, validates the options, and sets instance
        variables based on the options and arguments.

        @param   argc  the number of command line arguments
        @param   argv  the string vector of command line arguments

        @exception     CommandFormatException    Indicates the command line was
                                                 improperly formed
     */
    virtual void setCommand(Uint32 argc, char *argv []) = 0;

    /**
        Executes the command and writes the results to the output streams.

        @param  outPrintWriter  the ostream to which output should be
                                written

        @param  errPrintWriter  the ostream to which error output
                                should be written

        @return  0              if the command is successful
                 1              if an error occurs in executing the command
     */
    virtual Uint32 execute(
        PEGASUS_STD(ostream)& outPrintWriter,
        PEGASUS_STD(ostream)& errPrintWriter) = 0;

    //
    // Return codes
    //
    /**
        Return value from the execute method indicating the command executed
        successfully.
    */
    static const Uint32 RC_SUCCESS;

    /**
        Return value from the execute method indicating an error occurred in
        executing the command.
    */
    static const Uint32 RC_ERROR;

    String localizeMessage(
        const char *resource_path,
        const char * key,
        const char * default_msg,
        Formatter::Arg arg0 = Formatter::Arg(),
        Formatter::Arg arg1 = Formatter::Arg(),
        Formatter::Arg arg2 = Formatter::Arg(),
        Formatter::Arg arg3 = Formatter::Arg(),
        Formatter::Arg arg4 = Formatter::Arg(),
        Formatter::Arg arg5 = Formatter::Arg(),
        Formatter::Arg arg6 = Formatter::Arg(),
        Formatter::Arg arg7 = Formatter::Arg(),
        Formatter::Arg arg8 = Formatter::Arg(),
        Formatter::Arg arg9 = Formatter::Arg());

protected:
    /**
        Sets command usage string.  Usage string should be set by Command
        subclass constructor.

        @param    usageMessage    the command usage message
     */
    void setUsage(String& usageMessage);

private:

    /**
        The usage string for this command.  The value of the string should be
        set in the command subclass constructor.  The string may be retrieved
        and displayed when a CommandFormatException is caught.
     */
    String _usage;
};

PEGASUS_NAMESPACE_END

#endif  /* Pegasus_Command_h */
