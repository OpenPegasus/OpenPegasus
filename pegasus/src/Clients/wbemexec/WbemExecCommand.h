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

#ifndef Pegasus_WbemExecCommand_h
#define Pegasus_WbemExecCommand_h

#include <Pegasus/Common/String.h>
#include <Clients/cliutils/Command.h>
#include <Clients/cliutils/CommandException.h>
#include "WbemExecException.h"
#include "WbemExecClient.h"

PEGASUS_NAMESPACE_BEGIN

/**

    WbemExecCommand provides a command line interface to the CIM Server.
    The input to the command consists of a CIM request encoded in XML.  The
    request is submitted to the CIM Server for execution.  The result of the
    operation is returned to stdout, and consists of the CIM response encoded
    in XML.

    @author  Hewlett-Packard Company

 */
class WbemExecCommand : public Command
{
public:

    /**

        Constructs a WbemExecCommand and initializes instance variables.

     */
    WbemExecCommand ();

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

        @return  the Channel created

        @exception Exception        if an error is encountered in creating
                                    the connection

     */
    void _connectToServer(
        WbemExecClient& client,
        ostream& outPrintWriter);

    void _printContent(
        ostream& oStream,
        Buffer& responseMessage,
        Uint32 contentOffset);

/**

    Prompt for password.

    @param   estream             the ostream to which errors should be written

    @return  String value of the user entered password

 */
    String _promptForPassword( ostream&  eStream );
//NOCHKSRC
/**

    Check the HTTP response message for authentication challenge or data.

     Sample Local authentication request/response

    Challenge:
    ----------
    HTTP/1.1 401 Unauthorized
    WWW-Authenticate: LocalPrivileged "filePath"


    Challenge response:
    --------------------
     
    M-POST /cimom HTTP/1.1
    HOST: cascades
    Content-CIMType: application/xml; charset=utf-8
    Content-Length: 508
    Man: http://www.dmtf.org/cim/mapping/http/v1.0; ns=35
    35-CIMOperation: MethodCall
    35-CIMMethod: GetInstance
    35-CIMObject: root/cimv2
    PegasusAuthorization: LocalPrivileged "root:/tmp/cimclient_root_476:root131006297810"

    <?xml version="1.0" encoding="utf-8" ?>
    <CIM CIMVERSION="2.0" DTDVERSION="2.0">
    <MESSAGE ID="1001" PROTOCOLVERSION="1.0">
    <SIMPLEREQ>
    <IMETHODCALL NAME="GetInstance">
    <LOCALNAMESPACEPATH>
    <NAMESPACE NAME="root"/>
    <NAMESPACE NAME="cimv2"/>
    </LOCALNAMESPACEPATH>
    <IPARAMVALUE NAME="InstanceName">
    <INSTANCENAME CLASSNAME="PG_ConfigSetting">
    <KEYBINDING NAME="PropertyName">
    <KEYVALUE VALUETYPE="string">port</KEYVALUE>
    </KEYBINDING>
    </INSTANCENAME>
    </IPARAMVALUE>
    </IMETHODCALL>
    </SIMPLEREQ>
    </MESSAGE>
    </CIM>

// DOCHKSRC
    Final response:
    --------------------
    HTTP/1.1 200 OK
 
    @param   httpResponse        Buffer containing the reply from cimserver

    @param   ostream             the ostream to which output should be written

    @param   estream             the ostream to which errors should be written

    @return  true = wait for data from challenge response
    @return  false = client response has been received

    */
    void _handleResponse( Buffer          httpResponse,
              ostream&               oStream,
              ostream&               eStream
                        );

    /**

        Executes the command using HTTP.  A CIM request encoded in XML is read
        from the input, and encapsualted in an HTTP request message.  A channel
        is obtained for an HTTP connection, and the message is written to the
        channel.  The response is written to the specified outPrintWriter, and
        consists of the CIM response encoded in XML.

        @param   outPrintWriter     the ostream to which output should be
                                    written
        @param   errPrintWriter     the ostream to which error output should be
                                    written

        @exception  WbemExecException  if an error is encountered in executing
                                       the command

     */
    void _executeHttp (ostream& outPrintWriter, ostream& errPrintWriter);

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
        The port to be used when the command is executed, specified as a
        string.  The port number must be the port number on which the CIM
        Server is running on the specified host.  The default port number
        is 5988.
     */
    String _portNumberStr;

    /**
        The port to be used when the command is executed.  The port number must
        be the port number on which the CIM Server is running on the specified
        host.  The default port is the wbem local port number specified in
    /etc/services. If the wbem local port number is not defined in
    /etc/services, use the default port number 5988.
     */
    Uint32 _portNumber;

    /**
        A Boolean indicating whether a port number was specified on the
    command line.
     */
    Boolean _portNumberSet;

    /**
        Indicates that an HTTP/1.1 version request should be generated.  If
        true, an HTTP/1.1 request is generated.  Otherwise, an HTTP/1.0 request
        is generated.
     */
    Boolean _useHTTP11;

    /**
        Indicates that the HTTP M-POST method should be used for the request.
        If true, an HTTP M-POST request is generated.  Otherwise, an HTTP POST
        request is generated.
     */
    Boolean _useMPost;

    /**
        The timeout value to be used in milliseconds.
        The default timeout value is 20000.
     */
    Uint32 _timeout;

    /**
        Indicates that debug output is requested.  If true, the output of
        wbemexec includes the HTTP encapsulation of the original XML request.
     */
    Boolean _debugOutput1;

    /**
        Indicates that debug output is requested.  If true, the output of
        wbemexec includes the HTTP encapsulation of the XML response.
        Otherwise, only the XML response (if any) is included in the output of
        wbemexec.
     */
    Boolean _debugOutput2;

    /**
        The username to be used for authorization of the operation.
     */
    String _userName;

    /**
        Indicates that the user name is set.
     */
    Boolean _userNameSet;

    /**
        The password to be used for authorization of the operation.
     */
    String _password;

    /**
        Indicates that the password is set.
     */
    Boolean _passwordSet;

    /**
        The path of the input file containing a CIM request encoded in XML.
        By default, stdin is used as the input.
     */
    String _inputFilePath;

    /**
        A Boolean indicating whether an input file path was specified on the
        command line.
     */
    Boolean _inputFilePathSet;

    /**
        A Boolean indicating whether an SSL connection was specified on the
        command line.
     */
    Boolean _useSSL;

    /**
        The option character used to specify the hostname.
     */
    static const char   _OPTION_HOSTNAME;

    /**
        The option character used to specify the port number.
     */
    static const char   _OPTION_PORTNUMBER;

    /**
        The option character used to specify the HTTP version for the request.
     */
    static const char   _OPTION_HTTPVERSION;

    /**
        The option character used to specify the HTTP method for the request.
     */
    static const char   _OPTION_HTTPMETHOD;

    /**
        The option character used to specify the timeout value.
     */
    static const char   _OPTION_TIMEOUT;

    /**
        The option character used to specify that debug output is requested.
     */
    static const char   _OPTION_DEBUG;

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

    String usage;

    // The type of operation specified on the command line.
    Uint32        _operationType;


};

PEGASUS_NAMESPACE_END

#endif /* Pegasus_WbemExecCommand_h */
