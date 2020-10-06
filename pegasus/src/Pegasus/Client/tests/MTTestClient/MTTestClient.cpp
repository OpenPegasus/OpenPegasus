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
// Author : Sushma Fernandes, Hewlett-Packard Company
//              (sushma_fernandes@hp.com)
//
// Modified By: David Dillard, VERITAS Software Corp.
//                  (david.dillard@veritas.com)
//
//%/////////////////////////////////////////////////////////////////////////////


#include <iostream>
#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/Constants.h>
#include <Pegasus/Common/System.h>
#include <Pegasus/Common/FileSystem.h>
#include <Pegasus/Common/String.h>
#include <Pegasus/Common/PegasusVersion.h>
#include <Pegasus/Common/SSLContext.h>

#include <Pegasus/getoopt/getoopt.h>
#include <Clients/cliutils/CommandException.h>
#include "MTTestClient.h"

PEGASUS_NAMESPACE_BEGIN

#define NAMESPACE CIMNamespaceName ("root/cimv2")

/**
    The command name.
 */
const char   MTTestClient::COMMAND_NAME []      = "MTTestClient";

/**
    Label for the usage string for this command.
 */
const char   MTTestClient::_USAGE []            = "usage: ";

/**
    The option character used to specify the hostname.
 */
const char   MTTestClient::_OPTION_HOSTNAME     = 'h';

/**
    The option character used to specify the port number.
 */
const char   MTTestClient::_OPTION_PORTNUMBER   = 'p';

/**
    The option character used to specify SSL usage.
 */
const char   MTTestClient::_OPTION_SSL          = 's';

/**
    The option character used to specify the timeout value.
 */
const char   MTTestClient::_OPTION_TIMEOUT      = 't';

/**
    The option character used to specify the username.
 */
const char   MTTestClient::_OPTION_USERNAME     = 'u';

/**
    The option character used to specify the password.
 */
const char   MTTestClient::_OPTION_PASSWORD     = 'w';

/**
    The minimum valid portnumber.
 */
const Uint32 MTTestClient::_MIN_PORTNUMBER      = 0;

/**
    The maximum valid portnumber.
 */
const Uint32 MTTestClient::_MAX_PORTNUMBER      = 65535;

static const char PASSWORD_PROMPT []  =
                     "Please enter your password: ";

static const char PASSWORD_BLANK []  =
                     "Password cannot be blank. Please re-enter your password.";

static const Uint32 MAX_PW_RETRIES = 3;

String MTTestClient::_portNumberStr;
String MTTestClient::_password;
String MTTestClient::_userName;
String MTTestClient::_hostName;
Uint32 MTTestClient::_portNumber;
Boolean MTTestClient::_useSSL;
Boolean MTTestClient::_hostNameSet;
Boolean MTTestClient::_userNameSet;
Boolean MTTestClient::_passwordSet;
Boolean MTTestClient::_portNumberSet;

static Boolean verifyCertificate(SSLCertificateInfo &certInfo)
{

#ifdef DEBUG
    PEGASUS_STD(cout) << certInfo.getSubjectName() << endl;
#endif
    //ATTN-NB-03-05132002: Add code to handle server certificate verification.
    return true;
}

ThreadReturnType PEGASUS_THREAD_CDECL test_client(void *parm)
{
    CIMClient              client;
    Thread*                myHandle    = (Thread *)parm;
    String                 host                  = String ();
    Uint32                 portNumber            = 0;
    Boolean                connectToLocal        = false;

    //
    //  Construct host address
    //

    try
    {
        if ((!MTTestClient::_hostNameSet) && (!MTTestClient::_portNumberSet) &&
            (!MTTestClient::_userNameSet) && (!MTTestClient::_passwordSet))
        {
            connectToLocal = true;
        }
        else
        {
            if (!MTTestClient::_hostNameSet)
            {
               MTTestClient::_hostName = System::getHostName();
            }
            if( !MTTestClient::_portNumberSet )
            {
               if( MTTestClient::_useSSL )
               {
                   MTTestClient::_portNumber = System::lookupPort(
                                          WBEM_HTTPS_SERVICE_NAME,
                                          WBEM_DEFAULT_HTTPS_PORT );
               }
               else
               {
                   MTTestClient::_portNumber = System::lookupPort(
                                          WBEM_HTTP_SERVICE_NAME,
                                          WBEM_DEFAULT_HTTP_PORT );
               }
               char buffer[32];
               sprintf( buffer, "%lu",
                        (unsigned long) MTTestClient::_portNumber );
               MTTestClient::_portNumberStr = buffer;
            }
        }
        host = MTTestClient::_hostName;
        portNumber = MTTestClient::_portNumber;

        if( connectToLocal )
        {
            client.connectLocal();
        }
        else if( MTTestClient::_useSSL )
        {
            //
            // Get environment variables:
            //
            const char* pegasusHome = getenv("PEGASUS_HOME");

            String certpath = FileSystem::getAbsolutePath(
                               pegasusHome, PEGASUS_SSLCLIENT_CERTIFICATEFILE);

            String randFile;

            randFile = FileSystem::getAbsolutePath(
                        pegasusHome, PEGASUS_SSLCLIENT_RANDOMFILE);
            SSLContext  sslcontext (certpath, verifyCertificate, randFile);

            if (!MTTestClient::_userNameSet)
            {
               MTTestClient::_userName = System::getEffectiveUserName();
            }

            if (!MTTestClient::_passwordSet)
            {
                MTTestClient::_password =
                    MTTestClient::_promptForPassword( cout );
            }
            client.connect(host, portNumber, sslcontext,
                    MTTestClient::_userName, MTTestClient::_password );
       }
       else
       {
           if (!MTTestClient::_passwordSet)
           {
               MTTestClient::_password =
                    MTTestClient::_promptForPassword( cout );
           }
           client.connect(host, portNumber,
                   MTTestClient::_userName, MTTestClient::_password );
        }

        // Enumerate Instances.
        Array<CIMObjectPath> instanceNames = client.enumerateInstanceNames(
                                             NAMESPACE,
                                             "CIM_ManagedElement");

#ifdef DEBUG
        if ( instanceNames.size() == 0 )
        {
             PEGASUS_STD(cout) <<
                "<<<<<<<<<<<<< No Instances Found >>>>>>>>>>>" << endl;
        }
        else
        {
             PEGASUS_STD(cout) <<
                "<<<<<<<<<<<<< Instances Found : " <<
                instanceNames.size() << ">>>>>>>>>>>" << endl;
        }
        PEGASUS_STD(cout) << endl <<
            "++++++++ Completed Operation +++++++++ " << endl;
#endif
        client.disconnect();
#ifdef DEBUG
         PEGASUS_STD(cout) << endl <<
             "++++++++ Completed Disconnect +++++++++ " << endl;
#endif
    }
    catch(const Exception& e)
    {
        PEGASUS_STD(cout) << "Error: " << e.getMessage() << endl;
    }

    return ThreadReturnType(0);
}

/**

    Constructs a MTTestClient and initializes instance variables.

 */
MTTestClient::MTTestClient ()
{

    _hostName            = String ();
    _hostNameSet         = false;
    _portNumber          = WBEM_DEFAULT_HTTP_PORT;
    _portNumberSet       = false;

    char buffer[32];
    sprintf(buffer, "%lu", (unsigned long) _portNumber);
    _portNumberStr       = buffer;

    _timeout             = DEFAULT_TIMEOUT_MILLISECONDS;
    _userName            = String ();
    _userNameSet         = false;
    _password            = String ();
    _passwordSet         = false;
    _useSSL              = false;
    _enableDebug         = false;

    String usage = String (_USAGE);
    usage.append (COMMAND_NAME);
    usage.append (" [ -");
    usage.append (_OPTION_SSL);
    usage.append (" ] [ -");
    usage.append (_OPTION_HOSTNAME);
    usage.append (" hostname ] [ -");
    usage.append (_OPTION_PORTNUMBER);
    usage.append (" portnumber ] [ -");
    usage.append (_OPTION_USERNAME);
    usage.append (" username ] [ -");
    usage.append (_OPTION_PASSWORD);
    usage.append (" password ] [ -");
    usage.append (_OPTION_TIMEOUT);
    usage.append (" timeout ] [ -");
    usage.append (" ]");
    setUsage (usage);
}

String MTTestClient::_promptForPassword( ostream& outPrintWriter )
{
    //
    // Password is not set, prompt for non-blank password
    //
    String pw;
    Uint32 retries = 1;
    do
    {
        pw = System::getPassword(PASSWORD_PROMPT);

        if (pw == String::EMPTY)
        {
            if( retries < MAX_PW_RETRIES )
            {
                retries++;
            }
            else
            {
                break;
            }
            outPrintWriter << PASSWORD_BLANK << endl;
            continue;
        }
    }
    while (pw == String::EMPTY);
    return pw;
}

/**

    Parses the command line, validates the options, and sets instance
    variables based on the option arguments.

    @param   argc  the number of command line arguments
    @param   argv  the string vector of command line arguments

    @exception  CommandFormatException  if an error is encountered in parsing
                                        the command line

 */
void MTTestClient::setCommand (Uint32 argc, char* argv [])
{
    Uint32         i              = 0;
    Uint32         c              = 0;
    String         httpVersion    = String ();
    String         httpMethod     = String ();
    String         timeoutStr     = String ();
    String         GetOptString   = String ();
    getoopt        getOpts;

    //
    //  Construct GetOptString
    //
    GetOptString.append (_OPTION_HOSTNAME);
    GetOptString.append (getoopt::GETOPT_ARGUMENT_DESIGNATOR);
    GetOptString.append (_OPTION_PORTNUMBER);
    GetOptString.append (getoopt::GETOPT_ARGUMENT_DESIGNATOR);
    GetOptString.append (_OPTION_SSL);
    GetOptString.append (_OPTION_TIMEOUT);
    GetOptString.append (getoopt::GETOPT_ARGUMENT_DESIGNATOR);
    GetOptString.append (_OPTION_USERNAME);
    GetOptString.append (getoopt::GETOPT_ARGUMENT_DESIGNATOR);
    GetOptString.append (_OPTION_PASSWORD);
    GetOptString.append (getoopt::GETOPT_ARGUMENT_DESIGNATOR);

    //
    //  Initialize and parse getOpts
    //
    getOpts = getoopt ();
    getOpts.addFlagspec (GetOptString);
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
        if (getOpts [i].getType () == Optarg::LONGFLAG)
        {
            throw UnexpectedArgumentException(getOpts[i].Value());
        }
        else if (getOpts [i].getType () == Optarg::REGULAR)
        {
            throw UnexpectedArgumentException(getOpts[i].Value());
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
                        throw InvalidOptionArgumentException(
                            _portNumberStr,
                            _OPTION_PORTNUMBER);
                    }
                    _portNumberSet = true;
                    break;
                }

                case _OPTION_SSL:
                {
                    //
                    // Use port 5989 as the default port for SSL
                    //
                    _useSSL = true;
                    if (!_portNumberSet)
                       _portNumber = 5989;
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
                            timeoutStr,
                            _OPTION_TIMEOUT);
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

                default:
                    //
                    //  This path should not be hit
                    //
                    break;
            }
        }
    }

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
                _portNumberStr,
                _OPTION_PORTNUMBER);
        }
    }

    if (getOpts.isSet (_OPTION_TIMEOUT) < 1)
    {
        //
        //  No timeout specified
        //  Default to DEFAULT_TIMEOUT_MILLISECONDS
        //  Already done in constructor
        //
    }
    else
    {
        if (_timeout <= 0)
        {
            //
            //  Timeout out of valid range
            //
            throw InvalidOptionArgumentException(timeoutStr, _OPTION_TIMEOUT);
        }
    }
}

void MTTestClient::getMTTestClient(ostream& outPrintWriter,
                              ostream& errPrintWriter)
{
    const Uint32 THREAD_COUNT = 3;
    Thread *test_clients[THREAD_COUNT];

       for(Uint32 i = 0; i < THREAD_COUNT; i++)
       {
           test_clients[i] = new Thread(test_client, 0, false);
           test_clients[i]->run();
       }

       for(Uint32 i = 0; i < THREAD_COUNT; i++)
       {
           test_clients[i]->join();
           delete test_clients[i];
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
Uint32 MTTestClient::execute (ostream& outPrintWriter,
                                 ostream& errPrintWriter)
{
    try
    {
        getMTTestClient( outPrintWriter, errPrintWriter );
    }
    catch (const Exception& e)
    {
        errPrintWriter << MTTestClient::COMMAND_NAME << ": " <<
            e.getMessage () << endl;
        return (RC_ERROR);
    }
    catch (...)
    {
        errPrintWriter << MTTestClient::COMMAND_NAME << ": " <<
            "Unknown error" << endl;
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
    MTTestClient    command = MTTestClient ();
    int                rc;

    try
    {
        command.setCommand (argc, argv);
    }
    catch (const CommandFormatException& cfe)
    {
        cerr << MTTestClient::COMMAND_NAME << ": " << cfe.getMessage ()
             << endl;
        cerr << command.getUsage () << endl;
        exit (Command::RC_ERROR);
    }
    catch (const Exception& e)
    {
        cerr << MTTestClient::COMMAND_NAME << ": " << e.getMessage ()
             << endl;
    }

    rc = command.execute (cout, cerr);

    PEGASUS_STD(cout) <<  "+++++ passed all tests" << PEGASUS_STD(endl);
    exit (rc);
    return 0;
}
