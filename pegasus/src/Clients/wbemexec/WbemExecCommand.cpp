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

// define asprintf used to implement ultostr on Linux
#if defined(PEGASUS_LINUX_GENERIC_GNU)
#define _GNU_SOURCE
#include <features.h>
#include <stdio.h>
#endif

#include <iostream>
#include <Pegasus/Common/Signal.h>
#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/Constants.h>
#include <Pegasus/Common/System.h>
#include <Pegasus/Common/FileSystem.h>
#include <Pegasus/Common/String.h>
#include <Pegasus/Common/XmlWriter.h>
#include <Pegasus/Common/SSLContext.h>
#include <Pegasus/Common/AutoPtr.h>
#include <Pegasus/Common/PegasusVersion.h>

#include <Pegasus/getoopt/getoopt.h>
#include <Clients/cliutils/CommandException.h>
#include "HttpConstants.h"
#include "XMLProcess.h"
#include "WbemExecCommand.h"
#ifdef PEGASUS_WMIMAPPER
#include <WMIMapper/wbemexec/WMIWbemExecClient.h>
#else
#include "WbemExecClient.h"
#endif

#ifdef PEGASUS_OS_ZOS
#include <Pegasus/General/SetFileDescriptorToEBCDICEncoding.h>
#endif

PEGASUS_NAMESPACE_BEGIN

/**
    The command name.
 */
const char   WbemExecCommand::COMMAND_NAME []      = "wbemexec";

/**
    Label for the usage string for this command.
 */
const char   WbemExecCommand::_USAGE []            = "Usage: ";

/**
    The option character used to specify the hostname.
 */
const char   WbemExecCommand::_OPTION_HOSTNAME     = 'h';

/**
    The option character used to specify the port number.
 */
const char   WbemExecCommand::_OPTION_PORTNUMBER   = 'p';

/**
    The option character used to specify the HTTP version for the request.
 */
const char   WbemExecCommand::_OPTION_HTTPVERSION  = 'v';

/**
    The option character used to specify the HTTP method for the request.
 */
const char   WbemExecCommand::_OPTION_HTTPMETHOD   = 'm';

#ifdef PEGASUS_HAS_SSL
/**
    The option character used to specify SSL usage.
 */
const char   WbemExecCommand::_OPTION_SSL          = 's';
#endif

/**
    The option character used to specify the timeout value.
 */
const char   WbemExecCommand::_OPTION_TIMEOUT      = 't';

/**
    The option character used to specify that debug output is requested.
 */
const char   WbemExecCommand::_OPTION_DEBUG        = 'd';

/**
    The option character used to specify the username.
 */
const char   WbemExecCommand::_OPTION_USERNAME     = 'u';

/**
    The option character used to specify the password.
 */
const char   WbemExecCommand::_OPTION_PASSWORD     = 'w';

/**
    The minimum valid portnumber.
 */
const Uint32 WbemExecCommand::_MIN_PORTNUMBER      = 0;

/**
    The maximum valid portnumber.
 */
const Uint32 WbemExecCommand::_MAX_PORTNUMBER      = 65535;

/**
    The debug option argument value used to specify that the HTTP
    encapsulation of the original XML request be included in the output.
*/
const char   WbemExecCommand::_DEBUG_OPTION1       = '1';

/**
    The debug option argument value used to specify that the HTTP
    encapsulation of the XML response be included in the output.
*/
const char   WbemExecCommand::_DEBUG_OPTION2       = '2';

static const char   LONG_HELP []  = "help";

static const char   LONG_VERSION []  = "version";

static const char MSG_PATH []               = "pegasus/pegasusCLI";

static const char ERR_USAGE [] =
    "Use '--help' to obtain command syntax.";

static const char ERR_USAGE_KEY [] =
    "Clients.CIMConfig.CIMConfigCommand.ERR_USAGE";

/**
    This constant signifies that an operation option has not been recorded
*/

static const Uint32 OPERATION_TYPE_UNINITIALIZED  = 0;
/**
    This constant represents a help operation
*/
static const Uint32 OPERATION_TYPE_HELP           = 1;

/**
    This constant represents a version display operation
*/
static const Uint32 OPERATION_TYPE_VERSION        = 2;

static Boolean verifyCertificate(SSLCertificateInfo &certInfo)
{
    //
    // If server certificate was found in CA trust store and validated, then
    // return 'true' to accept the certificate, otherwise return 'false'.
    //
    if (certInfo.getResponseCode() == 1)
    {
        return true;
    }
    else
    {
        return false;
    }
}

/**

    Constructs a WbemExecCommand and initializes instance variables.

 */
WbemExecCommand::WbemExecCommand ()
{

    _hostName            = String ();
    _hostNameSet         = false;
    _portNumber          = WBEM_DEFAULT_HTTP_PORT;
    _portNumberSet       = false;

    char buffer[32];
    sprintf(buffer, "%lu", (unsigned long) _portNumber);
    _portNumberStr       = buffer;

    _useHTTP11           = true;
    _useMPost            = true;
    _timeout             = PEGASUS_DEFAULT_CLIENT_TIMEOUT_MILLISECONDS;
    _debugOutput1        = false;
    _debugOutput2        = false;
    _userName            = String ();
    _userNameSet         = false;
    _password            = String ();
    _passwordSet         = false;
    _inputFilePath       = String ();
    _inputFilePathSet    = false;
    _useSSL              = false;

    //
    //  Note: debug option is not shown in usage string.
    //  The debug option is not included in end-user documentation.
    //
    usage = String (_USAGE);
    usage.append (COMMAND_NAME);
    usage.append (" [ -");
    usage.append (_OPTION_HOSTNAME);
    usage.append (" hostname ] [ -");
    usage.append (_OPTION_PORTNUMBER);
    usage.append (" portnumber ] [ -");
    usage.append (_OPTION_HTTPVERSION);
    usage.append (" httpversion ]\n                [ -");
    usage.append (_OPTION_HTTPMETHOD);
    usage.append (" httpmethod ] [ -");
    usage.append (_OPTION_TIMEOUT);
    usage.append (" timeout ] [ -");
    usage.append (_OPTION_USERNAME);
    usage.append (" username ]\n                [ -");
    usage.append (_OPTION_PASSWORD);
    usage.append (" password ] [ ");
#ifdef PEGASUS_HAS_SSL
    usage.append ("-");
    usage.append (_OPTION_SSL);
    usage.append (" ] [ ");
#endif
    usage.append ("--");
    usage.append (LONG_HELP);
    usage.append (" ] [ --");
    usage.append (LONG_VERSION);
    usage.append (" ]\n                [ inputfilepath ]\n");

    usage.append("Options : \n");
    usage.append(
        "    -h         - Connect to CIM Server on specified hostname\n");
    usage.append("    --help     - Display this help message\n");
    usage.append(
        "    -m         - Use the specified HTTP method for the request\n");
    usage.append(
        "    -p         - Connect to CIM Server on specified portnumber\n");
#ifdef PEGASUS_HAS_SSL
    usage.append("    -s         - Use SSL protocol between 'wbemexec' client"
                    " and the CIM Server\n");
#endif
    usage.append(
        "    -t         - Specify response timeout value in milliseconds\n");
    usage.append("    -u         - Authorize the operation using the"
                    " specified username\n");
    usage.append("    -v         - Use the specified HTTP version for the"
                    " request\n");
    usage.append("    --version  - Display CIM Server version number\n");
    usage.append("    -w         - Authorize the operation using the"
                    " specified password\n");

    usage.append("\nUsage note: The wbemexec command requires that the"
                    " CIM Server is running.\n");

    setUsage (usage);
}

/**

    Connects to cimserver.

    @param   outPrintWriter     the ostream to which error output should be
                                written

    @return  the Channel created

    @exception       Exception  if an error is encountered in creating
                               the connection

 */
 void WbemExecCommand::_connectToServer( WbemExecClient& client,
                         ostream& outPrintWriter )
{
    String                 host                  = String ();
    Uint32                 portNumber            = 0;
    Boolean                connectToLocal        = false;

    //
    //  Construct host address
    //
    if ((!_hostNameSet) && (!_portNumberSet))
      {
        connectToLocal = true;
      }
#ifndef PEGASUS_DISABLE_LOCAL_DOMAIN_SOCKET
    if ((_hostNameSet) || (_portNumberSet))
    {
#endif
      host = _hostName;
      portNumber = _portNumber;
#ifndef PEGASUS_DISABLE_LOCAL_DOMAIN_SOCKET
    }
#endif

    if( _useSSL )
    {
#ifdef PEGASUS_HAS_SSL
        if( connectToLocal )
        {
            client.connectLocal();
        }
        else
        {
            //
            // Get environment variables:
            //
            const char* pegasusHome = getenv("PEGASUS_HOME");

            String certpath = FileSystem::getAbsolutePath(
                    pegasusHome, PEGASUS_SSLCLIENT_CERTIFICATEFILE);

            String randFile;

#ifdef PEGASUS_SSL_RANDOMFILE
            randFile = FileSystem::getAbsolutePath(
                    pegasusHome, PEGASUS_SSLCLIENT_RANDOMFILE);
#endif
            SSLContext sslcontext(certpath, verifyCertificate, randFile);
            client.connect(host, portNumber, &sslcontext, _userName, _password);
        }
#else
        PEGASUS_UNREACHABLE(PEGASUS_ASSERT(false);)
#endif
    }
    else
    {
        if( connectToLocal )
        {
            client.connectLocal();
        }
        else
        {
            client.connect(host, portNumber, _userName, _password );
        }
    }
}

/**
    Prints the response to the specified output stream.
 */
void WbemExecCommand::_printContent(
    ostream& oStream,
    Buffer& responseMessage,
    Uint32 contentOffset)
{
    //
    //  Get HTTP header
    //
    if (contentOffset < responseMessage.size())
    {
        //
        //  Print XML response to the ostream
        //
        const char* content = responseMessage.getData () + contentOffset;

#if defined(PEGASUS_DEBUG) && defined(PEGASUS_ENABLE_PROTOCOL_WSMAN)
        //
        // The response contains a unique Message ID. To allow
        // predictable message IDs for static comparison tests, replace the
        // response message Id with 0.
        //
        AutoArrayPtr<char> contentCopy;

        if (const char* uuidStart = strstr(content, "<wsa:MessageID>"))
        {
            if (const char* uuidEnd = strstr(uuidStart, "</wsa:MessageID>"))
            {
                contentCopy.reset((
                    strcpy(new char [strlen(content)+1],content)));

                // The message ID starts after the last ':' char. (See,
                // DSP0226 R5.4.4-1.). Position to the last ':' char.
                const char* colonPos = uuidEnd;
                for ( ; colonPos >= uuidStart && *colonPos != ':'; colonPos--)
                {
                }

                char* beginPtr = contentCopy.get() + (colonPos+1 - content);
                char* endPtr   = contentCopy.get() + (uuidEnd - content);

                // Replace the response messageID with 0.
                for (; beginPtr < endPtr; beginPtr++)
                {
                    if (*beginPtr != '-')
                    {
                        *beginPtr = '0';
                    }
                }

                content = contentCopy.get();
            }
        }
#endif
        XmlWriter::indentedPrint (oStream, content, 0);
    }
}

/**

    Process HTTP response message from cimserver

    @param   httpResponse        Buffer reply from cimserver

    @param   ostream             the ostream to which output should be written

    @param   estream             the ostream to which errors should be written

    @return  true  = wait for data from challenge response
    @return  false = client response has been received

 */
void WbemExecCommand::_handleResponse( Buffer           responseMessage,
                                          ostream&           oStream,
                                          ostream&           eStream
                                       )
{
    String                       startLine;
    Array<HTTPHeader>            headers;
    Uint32                       contentLength;
    Uint32                       contentOffset       = 0;
    HTTPMessage                  httpMessage(responseMessage, 0);

    httpMessage.parse(startLine, headers, contentLength);
    if( contentLength > 0 )
    {
        contentOffset = responseMessage.size() - contentLength;
    }
    else
    {
        contentOffset = responseMessage.size();
    }

    String httpVersion;
    Uint32 statusCode;
    String reasonPhrase;

    Boolean parsableMessage = HTTPMessage::parseStatusLine(
        startLine, httpVersion, statusCode, reasonPhrase);
    if (!parsableMessage || (statusCode != HTTP_STATUSCODE_OK))
    {
        // Received an HTTP error response
        // Output the HTTP error message and exit
        for (Uint32 i = 0; i < contentOffset; i++)
        {
            oStream << responseMessage[i];
        }
        oStream.flush();
        if( contentLength > 0 )
        {
            _printContent( oStream, responseMessage, contentOffset );
        }
        exit( 1 );
    }

    //
    // Received a valid HTTP response from the server.
    //
    if (_debugOutput2)
    {
        for (Uint32 i = 0; i < contentOffset; i++)
        {
            oStream << responseMessage[i];
        }
        oStream.flush();
    }
    _printContent( oStream, responseMessage, contentOffset );
}

/**

    Executes the command using HTTP.  A CIM request encoded in XML is read
    from the input, and encapsulated in an HTTP request message.  A channel
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
void WbemExecCommand::_executeHttp (ostream& outPrintWriter,
                                    ostream& errPrintWriter)
{
    Uint32                       size;
    Buffer                    content;
    Buffer                    contentCopy;
    Buffer                    message;
    Buffer                    httpHeaders;
    Buffer                    httpResponse;
#ifdef PEGASUS_WMIMAPPER
    WMIWbemExecClient client;
#else
    WbemExecClient client;
#endif

    client.setTimeout( _timeout );

    //
    //  Check for invalid combination of options
    //  The M-POST method may not be used with HTTP/1.0
    //
    if ((!_useHTTP11) && (_useMPost))
    {
        throw WbemExecException(WbemExecException::MPOST_HTTP10_INVALID);
    }

    //
    //  If no hostName specified
    //  Default to local host
    //
    if (!_hostNameSet)
    {
      _hostName = System::getHostName();
    }
    if( !_portNumberSet )
    {
        if( _useSSL )
        {
            _portNumber = System::lookupPort( WBEM_HTTPS_SERVICE_NAME,
                              WBEM_DEFAULT_HTTPS_PORT );
        }
        else
        {
            _portNumber = System::lookupPort( WBEM_HTTP_SERVICE_NAME,
                              WBEM_DEFAULT_HTTP_PORT );
        }
        char buffer[32];
        sprintf( buffer, "%lu", (unsigned long) _portNumber );
        _portNumberStr = buffer;
    }

    //
    //  Get XML request from input file
    //
    if (_inputFilePathSet)
    {
        //
        //  Check that input file exists
        //
        if (!FileSystem::exists (_inputFilePath))
        {
            throw WbemExecException(WbemExecException::INPUT_FILE_NONEXISTENT);
        }

        //
        //  Check that input file is readable
        //
        if (!FileSystem::canRead (_inputFilePath))
        {
            throw WbemExecException(WbemExecException::INPUT_FILE_NOT_READABLE);
        }

        //
        //  Check that file is not empty
        //
        FileSystem::getFileSize (_inputFilePath, size);
        if (size == 0)
        {
            throw WbemExecException(WbemExecException::NO_INPUT);
        }

        //
        //  Read from input file
        //
        try
        {
            FileSystem::loadFileToMemory (content, _inputFilePath);
        }
        catch (const CannotOpenFile&)
        {
            throw WbemExecException(WbemExecException::INPUT_FILE_CANNOT_OPEN);
        }
    }
    else
    {
        //
        //  Read from cin
        //
        //  (GetLine is defined in Pegasus/Common/String.[h,cpp], but is
        //  not a class member.)
        //
        String line;

        while (GetLine (cin, line))
        {
            content << line << '\n';
        }

        if (content.size () == 0)
        {
            //
            //  No input
            //
            throw WbemExecException(WbemExecException::NO_INPUT);
        }
    }

    //
    //  Make a copy of the content because the XmlParser constructor
    //  modifies the text
    //
    contentCopy << content;

    XmlParser parser ((char*) contentCopy.getData ());

    try
    {
        _connectToServer( client, outPrintWriter );

        //
        //  Encapsulate XML request in an HTTP request
        //

        String hostName;
        if (_hostNameSet && _hostName.size())
        {
            hostName = _hostName + String(":") + _portNumberStr;
        }

        message = XMLProcess::encapsulate( parser, hostName,
                                           _useMPost, _useHTTP11,
                                           content, httpHeaders );

        if (_debugOutput1)
        {
            outPrintWriter << message.getData () << endl;
        }
    }
    catch (const XmlException& xe)
    {
        throw WbemExecException(
            WbemExecException::INVALID_XML, xe.getMessage());
    }
    catch (const WbemExecException&)
    {
        throw;
    }
    catch (const Exception& ex)
    {
        throw WbemExecException(
            WbemExecException::CONNECT_FAIL, ex.getMessage());
    }

    try
    {
        httpResponse = client.issueRequest( message );
    }
    catch (const ConnectionTimeoutException&)
    {
        throw WbemExecException(WbemExecException::TIMED_OUT);
    }
    catch (const UnauthorizedAccess& ex)
    {
        throw WbemExecException(
            WbemExecException::CONNECT_FAIL, ex.getMessage());
    }
    catch (const Exception& ex)
    {
        throw WbemExecException(
            WbemExecException::CONNECT_FAIL, ex.getMessage());
    }

    //
    // Process the response message
    //
    _handleResponse( httpResponse, outPrintWriter, errPrintWriter );
}

/**

    Parses the command line, validates the options, and sets instance
    variables based on the option arguments.

    @param   argc  the number of command line arguments
    @param   argv  the string vector of command line arguments

    @exception  CommandFormatException  if an error is encountered in parsing
                                        the command line

 */
void WbemExecCommand::setCommand (Uint32 argc, char* argv [])
{
    Uint32         i              = 0;
    Uint32         c              = 0;
    String         httpVersion    = String ();
    String         httpMethod     = String ();
    String         timeoutStr     = String ();
    String         GetOptString   = String ();
    getoopt        getOpts;

    _operationType = OPERATION_TYPE_UNINITIALIZED;

    //
    //  Construct GetOptString
    //
    GetOptString.append (_OPTION_HOSTNAME);
    GetOptString.append (getoopt::GETOPT_ARGUMENT_DESIGNATOR);
    GetOptString.append (_OPTION_PORTNUMBER);
    GetOptString.append (getoopt::GETOPT_ARGUMENT_DESIGNATOR);
    GetOptString.append (_OPTION_HTTPVERSION);
    GetOptString.append (getoopt::GETOPT_ARGUMENT_DESIGNATOR);
    GetOptString.append (_OPTION_HTTPMETHOD);
    GetOptString.append (getoopt::GETOPT_ARGUMENT_DESIGNATOR);
#ifdef PEGASUS_HAS_SSL
    GetOptString.append (_OPTION_SSL);
#endif
    GetOptString.append (_OPTION_TIMEOUT);
    GetOptString.append (getoopt::GETOPT_ARGUMENT_DESIGNATOR);
    GetOptString.append (_OPTION_USERNAME);
    GetOptString.append (getoopt::GETOPT_ARGUMENT_DESIGNATOR);
    GetOptString.append (_OPTION_PASSWORD);
    GetOptString.append (getoopt::GETOPT_ARGUMENT_DESIGNATOR);
    GetOptString.append (_OPTION_DEBUG);
    GetOptString.append (getoopt::GETOPT_ARGUMENT_DESIGNATOR);

    //
    //  Initialize and parse getOpts
    //
    getOpts = getoopt ();
    getOpts.addFlagspec (GetOptString);

    //PEP#167 - adding long flag for options : 'help' and 'version'
    getOpts.addLongFlagspec(LONG_HELP,getoopt::NOARG);
    getOpts.addLongFlagspec(LONG_VERSION,getoopt::NOARG);

    getOpts.parse (argc, argv);

    if (getOpts.hasErrors ())
    {
        throw CommandFormatException(getOpts.getErrorStrings()[0]);
    }

    //
    //  Get options and arguments from the command line
    //
    for (i =  getOpts.first (); i <  getOpts.last (); i++)
    {
        if (getOpts[i].getType () == Optarg::LONGFLAG)
        {
            if (getOpts[i].getopt () == LONG_HELP)
            {
                if (_operationType != OPERATION_TYPE_UNINITIALIZED)
                {
                    String param = String (LONG_HELP);
                    //
                    // More than one operation option was found
                    //
                    throw UnexpectedOptionException(param);
                }

               _operationType = OPERATION_TYPE_HELP;
            }
            else if (getOpts[i].getopt () == LONG_VERSION)
            {
                if (_operationType != OPERATION_TYPE_UNINITIALIZED)
                {
                    String param = String (LONG_VERSION);
                    //
                    // More than one operation option was found
                    //
                    throw UnexpectedOptionException(param);
                }

               _operationType = OPERATION_TYPE_VERSION;
            }
        }
        else if (getOpts [i].getType () == Optarg::REGULAR)
        {
            //
            // _inputFilePath is the only non-option argument
            //
            if (_inputFilePathSet)
            {
                //
                // more than one _inputFilePath argument was found
                //
                throw UnexpectedArgumentException(getOpts[i].Value());
            }
            _inputFilePath = getOpts [i].Value ();
            _inputFilePathSet = true;
        }
        else /* getOpts [i].getType () == FLAG */
        {
            c = getOpts [i].getopt () [0];

            switch (c)
            {
                case _OPTION_HOSTNAME:
                {
                    if (getOpts.isSet (_OPTION_HOSTNAME) > 1)
                    {
                        //
                        // More than one hostname option was found
                        //
                        throw DuplicateOptionException(_OPTION_HOSTNAME);
                    }
                    _hostName = getOpts [i].Value ();
                    _hostNameSet = true;
                    break;
                }

                case _OPTION_PORTNUMBER:
                {
                    if (getOpts.isSet (_OPTION_PORTNUMBER) > 1)
                    {
                        //
                        // More than one portNumber option was found
                        //
                        throw DuplicateOptionException(_OPTION_PORTNUMBER);
                    }

                    _portNumberStr = getOpts [i].Value ();

                    try
                    {
                        getOpts [i].Value (_portNumber);
                    }
                    catch (const TypeMismatchException&)
                    {
                        throw InvalidOptionArgumentException(_portNumberStr,
                            _OPTION_PORTNUMBER);
                    }
                    _portNumberSet = true;
                    break;
                }

                case _OPTION_HTTPVERSION:
                {
                    if (getOpts.isSet (_OPTION_HTTPVERSION) > 1)
                    {
                        //
                        // More than one httpVersion option was found
                        //
                        throw DuplicateOptionException(_OPTION_HTTPVERSION);
                    }
                    httpVersion = getOpts [i].Value ();
                    break;
                }

#ifdef PEGASUS_HAS_SSL
                case _OPTION_SSL:
                {
                    _useSSL = true;
                    break;
                }
#endif

                case _OPTION_HTTPMETHOD:
                {
                    if (getOpts.isSet (_OPTION_HTTPMETHOD) > 1)
                    {
                        //
                        // More than one httpMethod option was found
                        //
                        throw DuplicateOptionException(_OPTION_HTTPMETHOD);
                    }
                    httpMethod = getOpts [i].Value ();
                    break;
                }

                case _OPTION_TIMEOUT:
                {
                    if (getOpts.isSet (_OPTION_TIMEOUT) > 1)
                    {
                        //
                        // More than one timeout option was found
                        //
                        throw DuplicateOptionException(_OPTION_TIMEOUT);
                    }

                    timeoutStr = getOpts [i].Value ();

                    try
                    {
                        getOpts [i].Value (_timeout);
                    }
                    catch (const TypeMismatchException&)
                    {
                        throw InvalidOptionArgumentException(
                            timeoutStr, _OPTION_TIMEOUT);
                    }
                    break;
                }

                case _OPTION_USERNAME:
                {
                    if (getOpts.isSet (_OPTION_USERNAME) > 1)
                    {
                        //
                        // More than one username option was found
                        //
                        throw DuplicateOptionException(_OPTION_USERNAME);
                    }
                    _userName = getOpts [i].Value ();
                    _userNameSet = true;
                    break;
                }

                case _OPTION_PASSWORD:
                {
                    if (getOpts.isSet (_OPTION_PASSWORD) > 1)
                    {
                        //
                        // More than one password option was found
                        //
                        throw DuplicateOptionException(_OPTION_PASSWORD);
                    }
                    _password = getOpts [i].Value ();
                    _passwordSet = true;
                    break;
                }

                case _OPTION_DEBUG:
                {
                    String debugOptionStr;

                    debugOptionStr = getOpts [i].Value ();

                    if (debugOptionStr.size () != 1)
                    {
                        //
                        //  Invalid debug option
                        //
                        throw InvalidOptionArgumentException(
                            debugOptionStr, _OPTION_DEBUG);
                    }

                    if (debugOptionStr [0] == _DEBUG_OPTION1)
                    {
                        _debugOutput1 = true;
                    }
                    else if (debugOptionStr [0] == _DEBUG_OPTION2)
                    {
                        _debugOutput2 = true;
                    }
                    else
                    {
                        //
                        //  Invalid debug option
                        //
                        throw InvalidOptionArgumentException(
                            debugOptionStr, _OPTION_DEBUG);
                    }
                    break;
                }

                default:
                    //
                    //  This path should not be hit
                    //
                    break;
            }
        }
    }

    if (!_userNameSet)
    {
        _userName = System::getEffectiveUserName();
    }
/*
    //
    // Some more validations
    //
    if ( _operationType == OPERATION_TYPE_UNINITIALIZED )
    {
        //
        // No operation type was specified
        // Show the usage
        //
        //l10n
        //CommandFormatException e (REQUIRED_ARGS_MISSING);
        throw CommandFormatException(localizeMessage(
            MSG_PATH, REQUIRED_ARGS_MISSING_KEY, REQUIRED_ARGS_MISSING));
    }
*/

    if (getOpts.isSet (_OPTION_PORTNUMBER) < 1)
    {
        //
        //  No portNumber specified
        //  Default to WBEM_DEFAULT_PORT
        //  Already done in constructor
        //
    }
    else
    {
        if (_portNumber > _MAX_PORTNUMBER)
        {
            //
            //  Portnumber out of valid range
            //
            throw InvalidOptionArgumentException(
                _portNumberStr, _OPTION_PORTNUMBER);
        }
    }

    if (getOpts.isSet (_OPTION_HTTPVERSION) < 1)
    {
        //
        //  No httpVersion specified
        //  Default is to use HTTP/1.1
        //
        _useHTTP11 = true;
    }
    else
    {
        if (httpVersion == HTTP_VERSION_10)
        {
            _useHTTP11 = false;
        }

        //
        //  If version specified is "1.1", use HTTP/1.1
        //
        else if (httpVersion == HTTP_VERSION_11)
        {
            _useHTTP11 = true;
        }

        //
        //  Invalid (unsupported) HTTP version specified
        //
        else
        {
            throw InvalidOptionArgumentException(
                httpVersion, _OPTION_HTTPVERSION);
        }
    }

    if (getOpts.isSet (_OPTION_HTTPMETHOD) < 1)
    {
        //
        //  No httpMethod specified
        //  Default is to use POST
        //
        _useMPost = false;
    }
    else
    {
        //
        //  Use HTTP POST method
        //
        if (httpMethod == HTTP_METHOD_POST)
        {
            _useMPost = false;
        }
        //
        //  Use HTTP M-POST method
        //
        else if (httpMethod == HTTP_METHOD_MPOST)
        {
            _useMPost = true;
        }

        //
        //  Invalid HTTP method specified
        //
        else
        {
            throw InvalidOptionArgumentException(
                httpMethod, _OPTION_HTTPMETHOD);
        }
    }

    if (getOpts.isSet (_OPTION_TIMEOUT) < 1)
    {
        //
        //  No timeout specified
        //  Default to WbemExecClient::DEFAULT_TIMEOUT_MILLISECONDS
        //  Already done in constructor
        //
    }
    else
    {
        if (_timeout == 0)
        {
            //
            //  Timeout out of valid range
            //
            throw InvalidOptionArgumentException(timeoutStr, _OPTION_TIMEOUT);
        }
    }
}


/**

    Executes the command and writes the results to the PrintWriters.

    @param   outPrintWriter     the ostream to which output should be
                                written
    @param   errPrintWriter     the ostream to which error output should be
                                written

    @return  0                  if the command is successful
             1                  if an error occurs in executing the command

 */
Uint32 WbemExecCommand::execute (ostream& outPrintWriter,
                                 ostream& errPrintWriter)
{
    if ( _operationType == OPERATION_TYPE_HELP )
    {
        cerr << usage << endl;
        return (RC_SUCCESS);
    }
    else if ( _operationType == OPERATION_TYPE_VERSION )
    {
        cerr << "Version " << PEGASUS_PRODUCT_VERSION << endl;
        return (RC_SUCCESS);
    }
    try
    {
        _executeHttp (outPrintWriter, errPrintWriter);
    }
    catch (const WbemExecException& e)
    {
        errPrintWriter << WbemExecCommand::COMMAND_NAME << ": " <<
            e.getMessage () << endl;
        return (RC_ERROR);
    }
    catch (const Exception& e)
    {
        errPrintWriter << WbemExecCommand::COMMAND_NAME << ": " <<
            e.getMessage() << endl;
        return (RC_ERROR);
    }
    catch (const exception& e)
    {
        errPrintWriter << WbemExecCommand::COMMAND_NAME << ": " <<
            e.what() << endl;
        return (RC_ERROR);
    }
    catch (...)
    {
        errPrintWriter << WbemExecCommand::COMMAND_NAME << ": " <<
            "Unknown error thrown" << endl;
        return (RC_ERROR);
    }
    return (RC_SUCCESS);
}

/**

    Parses the command line, and executes the command.

    @param   argc  the number of command line arguments
    @param   argv  the string vector of command line arguments

    @return  0                  if the command is successful
             1                  if an error occurs in executing the command

 */
PEGASUS_NAMESPACE_END

// exclude main from the Pegasus Namespace
PEGASUS_USING_PEGASUS;
PEGASUS_USING_STD;

int main (int argc, char* argv [])
{
    WbemExecCommand    command = WbemExecCommand ();
    int                rc;
    MessageLoader::setPegasusMsgHomeRelative(argv[0]);

#ifdef PEGASUS_OS_ZOS
    // for z/OS set stdout and stderr to EBCDIC
    setEBCDICEncoding(STDOUT_FILENO);
    setEBCDICEncoding(STDERR_FILENO);
#endif

    try
    {
        command.setCommand (argc, argv);
    }
    catch (const CommandFormatException& cfe)
    {
        cerr << WbemExecCommand::COMMAND_NAME << ": " << cfe.getMessage()
            << endl;

        MessageLoaderParms parms(ERR_USAGE_KEY,ERR_USAGE);
        parms.msg_src_path = MSG_PATH;
        cerr << WbemExecCommand::COMMAND_NAME <<
            ": " << MessageLoader::getMessage(parms) << endl;

        exit (Command::RC_ERROR);
    }

    rc = command.execute (cout, cerr);
    exit (rc);
    return 0;
}
