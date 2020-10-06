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

#ifndef Pegasus_OSInfoCommand_h
#define Pegasus_OSInfoCommand_h

#include <Pegasus/Common/String.h>
#include <Pegasus/Client/CIMClient.h>
#include <Clients/cliutils/Command.h>
#include <Clients/cliutils/CommandException.h>
#include "OSInfoException.h"

PEGASUS_NAMESPACE_BEGIN

/**
   The osinfo command displays information regarding the operating
   system, gathered via WBEM using the PG_OperatingSystem class supported
   by the OperatingSystem Provider.  osinfo requires CIM Server to be
   installed and running on the target host.

    @author  Hewlett-Packard Company

 */
class OSInfoCommand : public Command
{
public:

    enum { DEFAULT_TIMEOUT_MILLISECONDS = 200000 };

    /**

        Constructs an OSInfoCommand and initializes instance variables.

     */
    OSInfoCommand ();

    /**

        Parses the command line, validates the options, and sets instance
        variables based on the option arguments.

        @param   argc  the number of command line arguments
        @param   argv  the string vector of command line arguments

        @exception  CommandFormatException  if an error is encountered in
                                            parsing the command line

     */
    void setCommand (Uint32 argc, char* argv []);

    /**

        Executes the command and writes the results to the PrintWriters.

        @param   outPrintWriter     the ostream to which output should be
                                    written
        @param   errPrintWriter     the ostream to which error output should be
                                    written

        @return  0                  if the command is successful
                 1                  if an error occurs in executing the command

     */
    Uint32 execute (ostream& outPrintWriter, ostream& errPrintWriter);

    /**
        The command name.
     */
    static const char   COMMAND_NAME [];

private:

    /**

        Connect to cimserver.

        @param   outPrintWriter     the ostream to which error output should be
                                    written

        @exception Exception        if an error is encountered in creating
                                    the connection

     */
    void _connectToServer( CIMClient& client,
               ostream& outPrintWriter );

/**

    Prompt for password.

    @param   estream             the ostream to which errors should be written

    @return  String value of the user entered password

 */
    String _promptForPassword( ostream&  eStream );

    void getOSInfo (ostream& outPrintWriter, ostream& errPrintWriter);

    void errorExit(ostream& eStream, const String &message);

    void gatherProperties(CIMInstance &inst, Boolean cimFormat);

    void displayProperties(ostream& outPrintWriter);

    /**
        The host on which the command is to be executed.  A CIM Server must be
        running on this host when the command is executed.  The default host is
        the local host.
     */
    String _hostName;

    /**
        A Boolean indicating whether a hostname was specified on the command
        line.  The default host is the local host.
     */
    Boolean _hostNameSet;

    /**

        The port to be used when the command is executed. The
        port number must be the port number on which the
        target CIM Server is running.

        If no port is specified, osinfo will attempt to locate the port
        associated with the service "wbem-http" for non-SSL connections
        and "wbem-https" for SSL connections. If not found, the DMTF
        recommended default port number will be used.

        _portNumberStr and _portNumber are not used with
        connectLocal().

        The default port for non-SSL 5988 and 5989 for SSL.

     */
    String _portNumberStr;
    Uint32 _portNumber;

    /**
        A Boolean indicating whether a port number was specified on the
    command line.
     */
    Boolean _portNumberSet;

    /**
        The timeout value to be used, in milliseconds.
        The default timeout value is DEFAULT_TIMEOUT_MILLISECONDS.
     */
    Uint32 _timeout;

    /**
        The username to be used for authentication and
        authorization of the operation.
     */
    String _userName;

    /**
        Indicates that the user name is set.
     */
    Boolean _userNameSet;

    /**
        The password to be used for authentication.
     */
    String _password;

    /**
        Indicates that the password is set.
     */
    Boolean _passwordSet;

    /**
        A Boolean indicating whether an SSL connection was specified on the
        command line.
     */
    Boolean _useSSL;

    /**
        A Boolean indicating whether CIM_DateTime values should be diplayed
        using raw format.
     */
    Boolean _useRawDateTimeFormat;

    /**
        The option character used to specify the hostname.
     */
    static const char   _OPTION_HOSTNAME;

    /**
        The option character used to specify the port number.
     */
    static const char   _OPTION_PORTNUMBER;

    /**
        The option character used to specify the timeout value.
     */
    static const char   _OPTION_TIMEOUT;

    /**
        The option character used to specify the username.
     */
    static const char   _OPTION_USERNAME;

    /**
        The option character used to specify the password.
     */
    static const char   _OPTION_PASSWORD;

    /**
        The option character used to specify whether SSL should be used.
     */
    static const char   _OPTION_SSL;

    /**
        The option character used to specify whether DATETIME values
        should be formatted.
     */
    static const char   _OPTION_RAW_DATETIME_FORMAT;

    /**
        Label for the usage string for this command.
     */
    static const char   _USAGE [];

    /**
        The minimum valid portnumber.
     */
    static const Uint32 _MIN_PORTNUMBER;

    /**
        The maximum valid portnumber.
     */
    static const Uint32 _MAX_PORTNUMBER;

    /**
        The debug option argument value used to specify that the HTTP
        encapsulation of the original XML request be included in the output.
     */
    static const char   _DEBUG_OPTION1;

    /**
        The debug option argument value used to specify that the HTTP
        encapsulation of the XML response be included in the output.
     */
    static const char   _DEBUG_OPTION2;

  private:
     String osCSName;       // the hostname
     String osName;         // the name of the operating system
     String osVersion;
     String osOtherInfo;    // for Linux, the distribution info
     String osBootUpTime;
     String osCapability;
     String osLicensedUsers;
     String osLocalDateTime;
     String osSystemUpTime;
     String osNumberOfProcesses;
     String osNumberOfUsers;

     String usage;

     // The type of operation specified on the command line.
     Uint32        _operationType;
};

PEGASUS_NAMESPACE_END

#endif /* Pegasus_OSInfoCommand_h */
