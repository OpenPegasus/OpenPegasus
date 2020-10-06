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

#include <iostream>
#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/Constants.h>
#include <Pegasus/Common/System.h>
#include <Pegasus/Common/PegasusVersion.h>
#include <Pegasus/Common/SSLContext.h>
#include <Pegasus/getoopt/getoopt.h>
#include <Pegasus/Common/String.h>
#include <Pegasus/Client/CIMClient.h>
#include <Pegasus/Common/FileSystem.h>
#include <Pegasus/Common/TimeValue.h>
#include <time.h>
#include <signal.h>
#include "StressTestController.h"
#include "StressTestControllerException.h"
//
//Windows
//
#ifdef PEGASUS_OS_TYPE_WINDOWS
 // for DWORD etc.
# include <windows.h>
 // getpid() and others
 typedef DWORD pid_t;
# include <process.h>
#else
# include <unistd.h>
#endif

//#define STRESSTEST_DEBUG

#define SIXTYSECONDS 60
#define MILLISECONDS 1000
#define CHECKUP_INTERVAL 1
#define STOP_DELAY 1
#define SHUTDOWN_DELAY 5
#define RUN_DELAY 1
#define DEFAULT_INSTANCE "5"

#define convertmin2millisecs(x) (x * SIXTYSECONDS * MILLISECONDS)
#define getToleranceInPercent(x,y) (100 - (((y-x)/y) * 100))



static void endAllTests(int signum);

static void cleanupProcess();

static String convertUint64toString(Uint64 x);


PEGASUS_NAMESPACE_BEGIN
PEGASUS_USING_PEGASUS;
PEGASUS_USING_STD;



/**
  Log file descripter
*/

/**
   variable for Signal handler
*/
static Boolean Quit = false;

/**
    The command name.
 */
const char StressTestControllerCommand::COMMAND_NAME [] =
              "TestStressTestController";


/**
   StressTest Configuration file details
*/
char StressTestControllerCommand::FILENAME[] = "default_stresstest.conf";
char StressTestControllerCommand::TESTDIR[] = "/test/";
char StressTestControllerCommand::STRESSTESTDIR[] = "StressTestController/";
char StressTestControllerCommand::LOGDIR[] = "log/";
char StressTestControllerCommand::BINDIR[] = "/bin/";
char StressTestControllerCommand::DEFAULT_CFGDIR[] =
      STRESSTEST_DEFAULTCFGDIR;
char StressTestControllerCommand::DEFAULT_LOGDIR[] =
         "/test/StressTestController/log/";
char StressTestControllerCommand::DEFAULT_TMPDIR[] =
         "/test/StressTestController/tmp/";

String DEFAULT_BINDIR = String::EMPTY;

static Uint32 DEFAULT_CLIENTS = 2;
static Uint32 Total_Clients = DEFAULT_CLIENTS;
static Uint32 NEW_CLIENTS = 5;

static char MODELWALK_CLIENT[] = "TestModelWalkStressClient";
static char WRAPPER_CLIENT[] = "TestWrapperStressClient";

/**
  StressTest Client Status types
*/
enum CStatus{
    VALID_RESPONSE,
    INVALID_RESPONSE,
    NO_RESPONSE};


/**
  Temporary arrays to store client information
*/

/**
  Client PID's
*/
static pid_t *clientPIDs;

/**
  Client Status
*/
static int *clientStatus;


/**
  Client Status
*/
static int *prev_clientStatus;

/**
  Client Instance
*/
static Uint32* clientInstance;

/**
  Indicates if client is Active
*/
static Boolean *clientActive;

/**
  Client status time stamp
*/
static Uint64 *clientTimeStamp;

/**
  Previous client status time stamp
*/
static Uint64 *prev_clientTimeStamp;

/**
    DEFAULT VALUES:
*/

/**
    Default duration for the stress tests
*/
double StressTestControllerCommand::_duration = 180;

/**
    Label for the usage string for this command.
 */
const char StressTestControllerCommand::_USAGE [] = "Usage: ";

/**
    The option character used to specify the hostname.
 */
const char StressTestControllerCommand::_OPTION_HOSTNAME = 'h';

/**
    The option character used to specify the port number.
 */
const char StressTestControllerCommand::_OPTION_PORTNUMBER = 'p';

/**
    The option character used to specify SSL usage.
 */
const char StressTestControllerCommand::_OPTION_SSL = 's';

/**
    The option character used to specify the username.
 */
const char StressTestControllerCommand::_OPTION_USERNAME = 'u';

/**
    The option character used to specify the password.
 */
const char StressTestControllerCommand::_OPTION_PASSWORD = 'w';

/**
    The minimum valid portnumber.
 */
const Uint32 StressTestControllerCommand::_MIN_PORTNUMBER = 0;

/**
    The maximum valid portnumber.
 */
const Uint32 StressTestControllerCommand::_MAX_PORTNUMBER = 65535;

/**
    The minimum Duration.
 */
const Uint32 StressTestControllerCommand::_MIN_DURATION = 0;

/**
    The minimum valid Tolerance Level.
 */
const Uint32 StressTestControllerCommand::_MIN_TOLERANCE = 0;

/**
    The maximum valid Tolerance Level.
 */
const Uint32 StressTestControllerCommand::_MAX_TOLERANCE = 100;

/**
    The variable used to specify the hostname.
 */
static const char HOSTNAME[] = "hostname";

/**
    The variable used to specify the port number.
 */
static const char PORTNUMBER[] = "port";

/**
    The variable used to specify SSL usage.
 */
static const char SSL [] = "ssl";

/**
    The variable used to specify the username.
 */
static const char USERNAME[] = "username";

/**
    The variable used to specify the password.
 */
static const char PASSWORD[] = "password";

/**
    The variable used to specify the duration of the tests.
 */
static const char DURATION[] = "duration";

/**
    The variable used to specify the duration of the Client tests.
 */
static const char CLIENTDURATION[] = "ClientDuration";

/**
    The variable used to specify the ToleranceLevel for the tests.
 */
static const char TOLERANCELEVEL[] = "TOLERANCELEVEL";

/**
    The variable used to specify the NameSpace for the tests.
 */
static const char NAMESPACE[] = "namespace";

/**
    The variable used to specify the ClassName for the tests.
 */
static const char CLASSNAME[] = "classname";

/**
    The variable used to specify the Name for the tests.
 */
static const char NAME[] = "NAME";

/**
    The variable used to specify the Clientname for the tests.
 */
static const char CLIENTNAME[] = "clientname";

/**
    The variable used to specify the Clientname for the tests.
 */
static const char OPTIONS[] = "options";

/**
    The variable used to specify the Clientname for the tests.
 */
static const char INSTANCE[] = "INSTANCE";

/**
    The variable used to specify the Clientname for the tests.
 */
static const char CLIENTWAIT[] = "CLIENTWAIT";



/**
 * Message resource name
 */
static const char LONG_HELP[] = "help";
static const char LONG_VERSION[] = "version";
static const char LONG_VERBOSE[] = "verbose";

static Boolean IsAClient = false;

static Boolean IsClientOptions = false;

static Boolean IgnoreLine = false;

/**

    Constructs a StressTestControllerCommand and initializes instance variables.

 */
StressTestControllerCommand::StressTestControllerCommand ()
{

    _hostName = String ();
    _hostNameSpecified = false;
    _portNumber = WBEM_DEFAULT_HTTP_PORT;
    _portNumberSpecified = false;

    char buffer[32];
    sprintf(buffer, "%lu", (unsigned long) _portNumber);

    _portNumberStr = buffer;

    _timeout = DEFAULT_TIMEOUT_MILLISECONDS;
    _userName = String ();
    _userNameSpecified = false;
    _password = String ();
    _passwordSpecified = false;
    _useSSL = false;

    //
    // initialize
    //
    _clientCount = 0;
    _currClientCount = 0;

    //
    // Set up tables for client properties.
    //
    _clientTable.reset(new Table[Total_Clients]);

    //
    // Get environment variables:
    //
    pegasusHome = getenv("PEGASUS_HOME");

    DEFAULT_BINDIR = String(pegasusHome);
    DEFAULT_BINDIR.append(BINDIR);

    _stressTestLogFile = String::EMPTY;

    _stressTestClientPIDFile = String::EMPTY;

    _stressTestClientLogFile = String::EMPTY;

    _tmpStressTestClientPIDFile = String::EMPTY;



    _usage = String (_USAGE);

    _usage.append (COMMAND_NAME);
#ifndef DISABLE_SUPPORT_FOR_REMOTE_CONNECTIONS
    _usage.append (" [ -");
    _usage.append (_OPTION_SSL);
    _usage.append (" ] [ -");
    _usage.append (_OPTION_HOSTNAME);
    _usage.append (" hostname ] [ -");
    _usage.append (_OPTION_PORTNUMBER);
    _usage.append (" portnumber ]\n                            [ -");
    _usage.append (_OPTION_USERNAME);
    _usage.append (" username ] [ -");
    _usage.append (_OPTION_PASSWORD);
    _usage.append (" password ]");
#endif
    _usage.append (" [ --");
    _usage.append (LONG_HELP);
    _usage.append(" ]\n");
    _usage.append("                            ");
    _usage.append("[ --").append(LONG_VERSION).append(" ]");
    _usage.append(" [ --").append(LONG_VERBOSE).append(" ]").append(\
        " [<config_filename>] \n");

    _usage.append("Options : \n");
    _usage.append("    -h         - Connect to CIM Server on specified ");
    _usage.append("hostname. \n");
    _usage.append("    --help     - Display this help message.\n");
    _usage.append("    -p         - Connect to CIM Server on specified ");
    _usage.append("portnumber.\n");
    _usage.append("    -s         - Use SSL protocol between Stress Test ");
    _usage.append("Client\n");
    _usage.append("                 and the CIM Server\n");
    _usage.append("    -u         - Connect to CIM Server using the specified");
    _usage.append(" username\n");
    _usage.append("    --version  - Display CIM Server version number\n");
    _usage.append("    --verbose  - Display verbose information\n");
    _usage.append("    -w         - Connect to CIM Server using the specified");
    _usage.append(" password\n");
    _usage.append("\nOperands : \n");
    _usage.append("   <config_filename>\n");
    _usage.append("               - Specifies the name of the configuration ");
    _usage.append("file that is to be used \n");
    _usage.append("                 for the tests.\n");

    setUsage(_usage);

} /* StressTestControllerCommand  */

/**

    Parses the command line, validates the options, and sets instance
    variables based on the option arguments.

    @param   argc  the number of command line arguments
    @param   argv  the string vector of command line arguments

    @exception  CommandFormatException  if an error is encountered in parsing
                                        the command line

 */
void StressTestControllerCommand::setCommand (Uint32 argc, char* argv [])
{
    Uint32 i = 0;
    Uint32 c = 0;
    String GetOptString = String ();
    getoopt getOpts;
    _toleranceLevel = 0;
    _configFilePath = String ();
    _configFilePathSpecified = false;

    _operationType = OPERATION_TYPE_UNINITIALIZED;


    ofstream log_file;

    //
    // opens  the log file
    //
    OpenAppend(log_file,_stressTestLogFile);

    if (!log_file)
    {
        if(verboseEnabled)
        {
            cout<<StressTestControllerCommand::COMMAND_NAME<<
                "::Cannot get file "<<_stressTestLogFile<<endl;
        }

    }
    log_file<<StressTestControllerCommand::COMMAND_NAME<<
        ":: Preparing to set up parameters: "<<endl;

    //
    //  Construct GetOptString
    //
    GetOptString.append (_OPTION_HOSTNAME);
    GetOptString.append (getoopt::GETOPT_ARGUMENT_DESIGNATOR);
    GetOptString.append (_OPTION_PORTNUMBER);
    GetOptString.append (getoopt::GETOPT_ARGUMENT_DESIGNATOR);
    GetOptString.append (_OPTION_SSL);
    GetOptString.append (_OPTION_USERNAME);
    GetOptString.append (getoopt::GETOPT_ARGUMENT_DESIGNATOR);
    GetOptString.append (_OPTION_PASSWORD);
    GetOptString.append (getoopt::GETOPT_ARGUMENT_DESIGNATOR);

    //
    //  Initialize and parse getOpts
    //
    getOpts = getoopt ();
    getOpts.addFlagspec (GetOptString);

    //
    // per PEP#167
    //
    getOpts.addLongFlagspec(LONG_HELP,getoopt::NOARG);
    getOpts.addLongFlagspec(LONG_VERSION,getoopt::NOARG);
    getOpts.addLongFlagspec(LONG_VERBOSE,getoopt::NOARG);

    getOpts.parse (argc, argv);

    if (getOpts.hasErrors ())
    {
        log_file.close();
        CommandFormatException e(getOpts.getErrorStrings () [0]);
        throw e;
    }

    //
    //  Get options and arguments from the command line
    //
    for (i = getOpts.first(); i <  getOpts.last (); i++)
    {
        if (getOpts[i].getType() == Optarg::LONGFLAG)
        {
            if (getOpts[i].getopt() == LONG_HELP)
            {
               _operationType = OPERATION_TYPE_HELP;
            }
            else if (getOpts[i].getopt() == LONG_VERSION)
            {
                _operationType = OPERATION_TYPE_VERSION;
            }
            if (getOpts[i].getopt() == LONG_VERBOSE)
            {
                verboseEnabled = true;
            }
        }
        else if (getOpts [i].getType() == Optarg::REGULAR)
        {
            //
            // _configFilePath is the only non-option argument
            //
            if (_configFilePathSpecified)
            {
                //
                // more than one _configFilePath argument was found
                //
                log_file<<StressTestControllerCommand::COMMAND_NAME<<
                    "::More than one arguement was found "<<endl;
                log_file.close();
                UnexpectedArgumentException e(getOpts[i].Value());
                throw e;
            }
            _configFilePath = getOpts[i].Value();
            _configFilePathSpecified = true;
        }
        else /* getOpts[i].getType() == FLAG */
        {
            c = getOpts[i].getopt()[0];
            switch(c)
            {
                case _OPTION_HOSTNAME:
                {
                    if (getOpts.isSet(_OPTION_HOSTNAME) > 1)
                    {
                        //
                        // More than one hostname option was found
                        //
                        log_file.close();
                        DuplicateOptionException e(_OPTION_HOSTNAME);
                        throw e;
                    }
                    _hostName = getOpts [i].Value ();
                    _hostNameSpecified = true;
                    if (!_propertyTable.insert("hostname", _hostName))
                    {
                         // shouldn't get here
                         if(verboseEnabled)
                         {
                             cout<<StressTestControllerCommand::COMMAND_NAME;
                             cout<<"::Property Name name already saved: "<<
                                 "hostname"<<endl;
                         }
                    }
                    break;
                }
                case _OPTION_PORTNUMBER:
                {
                    if (getOpts.isSet(_OPTION_PORTNUMBER) > 1)
                    {
                        //
                        // More than one portNumber option was found
                        //
                        log_file.close();
                        DuplicateOptionException e(_OPTION_PORTNUMBER);
                        throw e;
                    }
                    _portNumberStr = getOpts [i].Value ();
                    try
                    {
                        getOpts[i].Value(_portNumber);
                    }
                    catch (const TypeMismatchException&)
                    {
                        log_file.close();
                        InvalidOptionArgumentException e(
                            _portNumberStr,
                            _OPTION_PORTNUMBER);
                        throw e;
                    }
                    _portNumberSpecified = true;
                    if (!_propertyTable.insert("port", _portNumberStr))
                    {
                        if(verboseEnabled)
                        {
                           cout<<StressTestControllerCommand::COMMAND_NAME;
                           cout<<"::Property Name:duplicate name already saved:"
                               <<"port"<<endl;
                        }
                    }
                    break;
                }
                case _OPTION_SSL:
                {
                    //
                    // Use port 5989 as the default port for SSL
                    //
                    _useSSL = true;
                    if (!_portNumberSpecified)
                    {
                       _portNumber = 5989;
                       _portNumberStr = "5989";
                       if (!_propertyTable.insert("port", _portNumberStr))
                       {
                           if(verboseEnabled)
                           {
                              cout<<StressTestControllerCommand::COMMAND_NAME;
                              cout<<"::Property Name already saved: "<<"port"<<
                                  endl;
                           }
                       }
                    }
                    if (!_propertyTable.insert("ssl", ""))
                    {
                        if(verboseEnabled)
                        {
                            cout<<StressTestControllerCommand::COMMAND_NAME;
                            cout<<"::Property Name already saved: "<<"ssl"<<
                                endl;
                        }
                    }
                    break;
                }
                case _OPTION_USERNAME:
                {
                    if (getOpts.isSet(_OPTION_USERNAME) > 1)
                    {
                        //
                        // More than one username option was found
                        //
                        log_file.close();
                        DuplicateOptionException e(_OPTION_USERNAME);
                        throw e;
                    }
                    _userName = getOpts[i].Value();
                    _userNameSpecified = true;
                    if (!_propertyTable.insert("username", _userName))
                    {
                        if(verboseEnabled)
                        {
                            cout<<StressTestControllerCommand::COMMAND_NAME;
                            cout<< "::Property Name already saved: "<<
                                "username"<<endl;
                        }
                    }
                    break;
                }
                case _OPTION_PASSWORD:
                {
                    if (getOpts.isSet(_OPTION_PASSWORD) > 1)
                    {
                        //
                        // More than one password option was found
                        //
                        log_file.close();
                        DuplicateOptionException e(_OPTION_PASSWORD);
                        throw e;
                    }
                    _password = getOpts[i].Value();
                    _passwordSpecified = true;
                    if (!_propertyTable.insert("password", _password))
                    {
                        if(verboseEnabled)
                        {
                            cout<<StressTestControllerCommand::COMMAND_NAME;
                            cout<<"::Property Name already saved: "<<
                                "password"<<endl;
                        }
                    }
                    break;
                }

                default:
                {
                    //
                    //  This path should not be hit
                    //  PEP#167 unless an empty '-' is specified
                    //
                    log_file.close();
                    String ErrReport =
                        String(StressTestControllerCommand::COMMAND_NAME);
                    ErrReport.append("::Invalid or unknown option specified");
                    throw StressTestControllerException(ErrReport);

                    break;
               }
            }
        }
    }
    //
    //  More validation:
    //    No portNumber specified
    //    Default to WBEM_DEFAULT_PORT
    //    Already done in constructor
    //
    if (getOpts.isSet(_OPTION_PORTNUMBER) == 1)
    {
        if (_portNumber > _MAX_PORTNUMBER)
        {
            //
            //  Portnumber out of valid range
            //
            log_file.close();
            InvalidOptionArgumentException e(_portNumberStr,
                _OPTION_PORTNUMBER);
            throw e;
        }
    }
    log_file.close();
} /* setCommand  */

/**

    Generates commands and its options for each of the clients.
    The client table is traversed to generate each of the client commands.
    The Commands, Duration and Delays for each client are saved in
    the following array's respectively:
       _clientCommands
       _clientDurations
       _clientDelays

    @param   log_file           The log file.

    @return  0                  if the command is successfully generated
             1                  if the command cannot be generated.
 */
Boolean StressTestControllerCommand::generateClientCommands(ostream& log_file)
{

    String client_command = String::EMPTY;
    double duration = _duration;
    double delay = 0;

    //
    // Array's to store client specific information
    //
    _clientCommands.reset(new String[_clientCount]);
    _clientDurations.reset(new Uint64[_clientCount]);
    _clientDelays.reset(new Uint64[_clientCount]);

    //
    // Retrieve all the client options from the client table
    // and build commands for respective clients.
    // Add appropriate options to command string  as required
    //
    for (Uint32 j=0; j< _clientCount; j++)
    {
        delay = 0;
        String clientName = String::EMPTY;
        String clientInst = String::EMPTY;
        //
        //  Stress Client Name must exist for each client/client table
        //
        if (!_clientTable.get()[j].lookup(NAME, clientName))
        {
            log_file<<StressTestControllerCommand::COMMAND_NAME<<
                 "::Required property NAME not found."<<endl;
            return false;
        }
        //
        // Start the command string with the client name.
        //
        client_command = clientName;

        //
        // Generate the commands for each client from each client table.
        //
        for (Table::Iterator i = _clientTable.get()[j].start(); i; i++)
        {
            if (String::equalNoCase(i.key(),HOSTNAME))
            {
                client_command.append(" -hostname ");
                if (_hostNameSpecified)
                {
                    client_command.append(_hostName);
                }
                else
                {
                    client_command.append(i.value());
                }
            }
            else if (String::equalNoCase(i.key(),NAME))
            {
                //
                // should be ignored - already saved using clientName
                //
            }
            else if (String::equalNoCase(i.key(),PORTNUMBER))
            {
                client_command.append(" -");
                client_command.append(PORTNUMBER);
                client_command.append(" ");
                if (_portNumberSpecified)
                {
                    client_command.append(_portNumberStr);
                }
                else
                {
                    client_command.append(i.value());
                }
            }
            else if (String::equalNoCase(i.key(),SSL))
            {
                client_command.append(" -");
                client_command.append(SSL);
            }
            else if (String::equalNoCase(i.key(),USERNAME))
            {
                client_command.append(" -");
                client_command.append(USERNAME);
                client_command.append(" ");
                client_command.append(i.value());
            }
            else if (String::equalNoCase(i.key(),PASSWORD))
            {
                client_command.append(" -");
                client_command.append(PASSWORD);
                client_command.append(" ");
                client_command.append(i.value());
            }
            else if (String::equalNoCase(i.key(),CLIENTNAME))
            {
                client_command.append(" -");
                client_command.append(CLIENTNAME);
                client_command.append(" ");
                client_command.append(i.value());
            }
            else if (String::equalNoCase(i.key(),OPTIONS))
            {
                client_command.append(" -");
                client_command.append(OPTIONS);
                client_command.append(" ");
                client_command.append(i.value());
            }
            else if (String::equalNoCase(i.key(),NAMESPACE))
            {
                client_command.append(" -");
                client_command.append(NAMESPACE);
                client_command.append(" ");
                client_command.append(i.value());
            }
            else if (String::equalNoCase(i.key(),CLASSNAME))
            {
                client_command.append(" -");
                client_command.append(CLASSNAME);
                client_command.append(" ");
                client_command.append(i.value());
            }
            else if ((String::equalNoCase(i.key(),INSTANCE))
                      ||(String::equalNoCase(i.key(),CLIENTWAIT))
                      ||(String::equalNoCase(i.key(),CLIENTDURATION)))
            {
                //
                // do nothing here
                //   will be utilized to run the clients later.
                //
            }
            else
            {
                //
                // Save all other options for the commands
                //
                client_command.append(" -");
                client_command.append(i.key());
                client_command.append(" ");
                client_command.append(i.value());
            }
        }
        //
        // Include verbose if enabled to clients
        //
        if (verboseEnabled)
        {
            client_command.append(" -verbose ");
        }

        //
        // Acquire all the common properties listed in the property table
        // from config file and include it as part of the client command
        // as required.
        //
        for (Table::Iterator k = _propertyTable.start(); k; k++)
        {
            String propertyValue = String::EMPTY;
            //
            // Only include the common properties that are not already
            // listed for the clients.
            //
            if (!_clientTable.get()[j].lookup(k.key(), propertyValue))
            {
                //
                // Include options other than ToleranceLevel
                // clientDuration,clientwait and Duration
                // in the command string for the clients.
                //
                if ((!String::equalNoCase(k.key(),TOLERANCELEVEL))
                     && (!String::equalNoCase(k.key(),CLIENTDURATION))
                     && (!String::equalNoCase(k.key(),CLIENTWAIT))
                     && (!String::equalNoCase(k.key(),DURATION)))
                {
                    client_command.append(" -");
                    client_command.append(k.key());
                    //
                    // No values required for SSL
                    //
                    if (!String::equalNoCase(k.key(),SSL))
                    {
                        client_command.append(" ");
                        client_command.append(k.value());
                    }
                }
            }
            //
            // Use default duration if one was not specified in the Config file.
            //
            if (String::equalNoCase(k.key(),DURATION))
            {
                duration = atof(k.value().getCString());
            }
            else
            {
                duration = _duration;
            }
        } /* for (Table::Iterator k = _propertyTable.start(); k; k++) */

        //
        // Looking up table while ignoring cases for Client Duration/Wait.
        //
        for (Table::Iterator k = _clientTable.get()[j].start(); k; k++)
        {
            //
            // Overwrite duration if client duration set
            //
            if (String::equalNoCase(k.key(),CLIENTDURATION))
            {
                duration = atof(k.value().getCString());
            }
            if (String::equalNoCase(k.key(),CLIENTWAIT))
            {
                delay = atof(k.value().getCString());
            }
        }

        //
        // Save the generated command to corresponding element in the
        // clientCommand array.
        //
        _clientCommands.get()[j] = client_command;

        //
        // Converting minutes to milliseconds
        //
        _clientDurations.get()[j] = (Uint64)convertmin2millisecs(duration);
        _clientDelays.get()[j] = (Uint64)convertmin2millisecs(delay);

        //
        // Saving logs
        //
        log_file << StressTestControllerCommand::COMMAND_NAME <<
            "::Client Command[" << j << "]" << endl;
        log_file << "  " << _clientCommands.get()[j] << endl;
        log_file << "   Client Duration: " <<
            convertUint64toString(_clientDurations.get()[j]) << endl;
        log_file << "   Client Delay: " <<
            convertUint64toString(_clientDelays.get()[j]) << endl;

        //
        // Verbose
        //
        if (verboseEnabled)
        {
            cout << StressTestControllerCommand::COMMAND_NAME <<
                "::Client Command[" << j << "]" << endl;
            cout << "  " << _clientCommands.get()[j] << endl;
            cout << "   Client Duration: " <<
                convertUint64toString(_clientDurations.get()[j]) << endl;
            cout << "   Client Delay: " <<
                convertUint64toString(_clientDelays.get()[j]) << endl;
        }
    } /* for(Uint32 j=0; j< _clientCount; j++) */
    return true;
} /* generateClientCommands */

/*
    Executes the command and writes the results to the PrintWriters.
    This method is where the clients are intiated.
    The clients are kept running until its duration is met
    or the controller is interrupted by a SIGINT or SIGABRT signal
    or if the controller failed to acquire the clientPIDs when needed.

    Clients with clientWait,  will be stopped when its duration is met and
    then restarted after its wait period is completed. This will continue
    until the end of the overall duration.
    When the overall duration has ended or the controller is interupted then
    the controller will read the PID file to update its clients PID and
    request all the clients to end its processes.

    @param   outPrintWriter     the ostream to which output should be
                                written
    @param   errPrintWriter     the ostream to which error output should be
                                written

    @return  0                  if the command is successful
             1                  if an error occurs in executing the command

 */
Uint32 StressTestControllerCommand::execute (
    ostream& outPrintWriter,
    ostream& errPrintWriter)
{
    int actual_client = 0;
    Uint64 startMilliseconds = 0;
    Uint64 nowMilliseconds = 0;
    Uint64 stopMilliseconds = 0;
    Uint64 timeoutMilliseconds = 0;
    AutoArrayPtr<Uint64> clientStartMilliseconds;
    AutoArrayPtr<Uint64> clientStopMilliseconds;
    AutoArrayPtr<Uint64> clientDelayMilliseconds;
    AutoArrayPtr<Boolean> clientStopped;
    AutoArrayPtr<Boolean> clientDelayed;
    String act_command;
    Boolean TestFailed = false;
    char str[15];
    char strTime[256];
    struct tm tmTime;

    //
    // log file
    //
    ofstream log_file;

    //
    // open the file
    //
    OpenAppend(log_file,_stressTestLogFile);

    //
    // Failed to read log file.
    //
    if (!log_file)
    {
        log_file.close();
        if (verboseEnabled)
        {
            outPrintWriter<<StressTestControllerCommand::COMMAND_NAME;
            outPrintWriter<<"Cannot read file "<<_stressTestLogFile<<endl;
        }
        return RC_ERROR;
    }

    //
    // Display usage message if help was specified
    //
    if ( _operationType == OPERATION_TYPE_HELP )
    {
        outPrintWriter << _usage << endl;
        log_file<<StressTestControllerCommand::COMMAND_NAME<<
            "::Listing usage information "<<endl;
        log_file.close();
        //
        //  No need for the client pid and log file.
        //
        FileSystem::removeFile(_stressTestClientPIDFile);
        FileSystem::removeFile(_stressTestClientLogFile);
        return (RC_SUCCESS);
    }
    //
    // Display PEGASUS version if version was specified
    //
    else if ( _operationType == OPERATION_TYPE_VERSION )
    {
        outPrintWriter << "Version " << PEGASUS_PRODUCT_VERSION << endl;
        log_file<<StressTestControllerCommand::COMMAND_NAME<<
            "::Listing version information "<<endl;
        log_file<<"Version " << PEGASUS_PRODUCT_VERSION << endl;
        log_file.close();
        //
        //  No need for the client pid and log file.
        //
        FileSystem::removeFile(_stressTestClientPIDFile);
        FileSystem::removeFile(_stressTestClientLogFile);
        return (RC_SUCCESS);
    }

    //
    // gracefully shutdown when interrupted
    //
    signal(SIGABRT, endAllTests);
    signal(SIGINT, endAllTests);

    //
    // Allocate variables necessary to run clients.
    //
    if(_clientCount > 0)
    {
        clientInstance = new Uint32[_clientCount];
        clientStartMilliseconds.reset(new Uint64[_clientCount]);
        clientStopMilliseconds.reset(new Uint64[_clientCount]);
        clientDelayMilliseconds.reset(new Uint64[_clientCount]);
        clientStopped.reset(new Boolean[_clientCount]);
        clientDelayed.reset(new Boolean[_clientCount]);
    }
    else
    {
        errPrintWriter << "Stress Tests must have at least one Client." << endl;
        log_file<<StressTestControllerCommand::COMMAND_NAME<<
            "::Stress Tests must have at least one Client."<<endl;
        log_file.close();
        return (RC_ERROR);
    }

    try
    {
        //
        // Initializing the Boolean array's to false.
        //
        for (Uint32 i=0;i<_clientCount;i++)
        {
            clientStopped.get()[i] = false;
            clientDelayed.get()[i] = false;
        }
        //
        // Set up duration of the tests
        //
        startMilliseconds   = TimeValue::getCurrentTime().toMilliseconds();
        nowMilliseconds     = startMilliseconds;
        timeoutMilliseconds = (Uint64)convertmin2millisecs(_duration);
        stopMilliseconds    = nowMilliseconds + timeoutMilliseconds;

        log_file<<StressTestControllerCommand::COMMAND_NAME<<
            ":: Test Duration information "<<endl;
        log_file<<"  Start Time in milliseconds: "<<
            convertUint64toString(startMilliseconds)<<endl;
        log_file<<"  Total duration in milliseconds: "<<
            convertUint64toString(timeoutMilliseconds)<<endl;
        log_file<<"  Actual Stop Time in milliseconds: "<<
            convertUint64toString(stopMilliseconds)<<endl;
        //
        //  Verbose details for Stress Test duration
        //
        if (verboseEnabled)
        {
            outPrintWriter<<StressTestControllerCommand::COMMAND_NAME<<
                ":: Test Duration information "<<endl;
            outPrintWriter<<"  Start Time in milliseconds: "<<
                convertUint64toString(startMilliseconds)<<endl;
            outPrintWriter<<"  Total duration in milliseconds: "<<
                convertUint64toString(timeoutMilliseconds)<<endl;
            outPrintWriter<<
                "  Actual Stop Time in milliseconds: "<<
                convertUint64toString(stopMilliseconds)<<endl;
        }

        //
        //  First Tolerance check up interval is set up to be twice
        //  the CHECKUP_INTERVAL. This should give the clients enough time
        //  to update its PID, status  etc.
        //
        Uint64 nextCheckupInMillisecs =
           (Uint64)convertmin2millisecs(2 * CHECKUP_INTERVAL) + nowMilliseconds;
        //
        //  Main "while" loop where the clients are initiated.
        //
        while(stopMilliseconds > nowMilliseconds)
        {

            //
            // Small delay in the while loop seemed to reduce the CPU usage
            // considerably  in Windows. (From 80% to 1%)
            //
#ifndef PEGASUS_OS_TYPE_WINDOWS
            sleep(RUN_DELAY);
#else
            Sleep(RUN_DELAY * 1000);
#endif

            //
            // Quit if SIGINT, SIGABRT is caught
            // So the clients can be gracefully shutdown.
            //
            if(Quit)
            {
                log_file<<
                    "Test interrupted by either SIGINT or SIGABORT."<<endl;
                TestFailed = true;
                break;
            }
            //
            // The following block will be where clients are executed initially.
            //
            if(!actual_client)
            {
                log_file<<StressTestControllerCommand::COMMAND_NAME<<
                    "::Running the following tests: "<<endl;
                outPrintWriter<<StressTestControllerCommand::COMMAND_NAME<<
                    "::Running the following tests: "<<endl;
                for (Uint32 j=0; j< _clientCount; j++)
                {
                    String clientInst = String::EMPTY;
                    //
                    // Acquire the client Instance for each clients
                    //
                    if (!_clientTable.get()[j].lookup(INSTANCE, clientInst))
                    {
                        String ErrReport = String("Invalid Property Value: ");
                        ErrReport.append(INSTANCE);
                        ErrReport.append("=");
                        ErrReport.append(clientInst);
                        throw StressTestControllerException(ErrReport);
                    }
                    clientInstance[j] = (Uint32)atoi(clientInst.getCString());

                    //
                    // Acquire and set client specific duration
                    //
                    clientStartMilliseconds.get()[j] =
                        TimeValue::getCurrentTime().toMilliseconds();
                    clientStopMilliseconds.get()[j] =
                        clientStartMilliseconds.get()[j] +
                            _clientDurations.get()[j];

                    //
                    // for verbose only
                    //
                    if (verboseEnabled)
                    {
                        outPrintWriter << "Client:" << "[" << j<< "]" << endl;
                        log_file << "Client:" << "[" << j << "]" << endl;
                        outPrintWriter << "ClientStart:" <<
                            convertUint64toString(
                                clientStartMilliseconds.get()[j]) <<
                            endl;
                        outPrintWriter << "ClientStop:" <<
                            convertUint64toString(
                                clientStopMilliseconds.get()[j]) <<
                            endl;
                        outPrintWriter << "ClientDuration:" <<
                            convertUint64toString(_clientDurations.get()[j]) <<
                            endl;
                        log_file << "ClientStart:" <<
                            convertUint64toString(
                                clientStartMilliseconds.get()[j]) <<
                            endl;
                        log_file << "ClientStop:" <<
                            convertUint64toString(
                                clientStopMilliseconds.get()[j]) <<
                            endl;
                        log_file << "ClientDuration:" <<
                            convertUint64toString(_clientDurations.get()[j]) <<
                            endl;
                    }
                    log_file<<
                        "Number of instances of this client: "<<
                        clientInstance[j]<<endl;
                    if(verboseEnabled)
                    {
                        outPrintWriter<<
                            "Number of instances of this client:"<<
                            clientInstance[j]<<endl;
                    }
                    //
                    // Execute each instance of the client.
                    //   - Additional required parameters are added to the
                    //     commands.
                    //     like, -clientid, -pidfile, -clientlog
                    //
                    for (Uint32 instanceID = 0; instanceID<clientInstance[j];
                         instanceID++)
                    {
                        outPrintWriter<<
                            "Running Client("<<actual_client<<")"<<endl;
                        log_file<<
                            "Running Client("<<actual_client<<")"<<endl;
                        act_command=String::EMPTY;
#ifdef PEGASUS_OS_TYPE_WINDOWS
                        act_command.append("start ");
#endif
                        //
                        // Adding all the required parameters for the command.
                        //
                        act_command.append(_clientCommands.get()[j]);
                        act_command.append(" -clientid ");
                        sprintf(str,"%d",actual_client);
                        act_command.append(str);
                        act_command.append(" -pidfile ");
                        act_command.append(" \"");
                        act_command.append(_stressTestClientPIDFile);
                        act_command.append("\"");
                        act_command.append(" -clientlog");
                        act_command.append(" \"");
                        act_command.append(_stressTestClientLogFile);
                        act_command.append("\"");
                        act_command.append("&");
                        if (verboseEnabled)
                        {
                            outPrintWriter<<"  "<<act_command<<endl;
                            tmTime = getCurrentActualTime();
                            strftime(
                                strTime,
                                256,
                                "%d/%m/%Y at %H:%M:%S\n",
                                &tmTime);
                            log_file<<
                                StressTestControllerCommand::COMMAND_NAME<<
                                "::Running following command on "<<
                                strTime<<endl;
                            log_file<<"     ("<<actual_client<<") \n"<<
                                act_command<<endl;
                        }

                        //
                        // Executing the Client
                        //
                        int commandRc = system(act_command.getCString());
                        //
                        // Continue even if the client failed to Execute
                        // This failure is validated with Tolerance level later
                        //
                        if (commandRc)
                        {
                            log_file<<"Command failed to Execute."<<endl;
                            if (verboseEnabled)
                            {
                                outPrintWriter<<
                                    "Command failed to Execute."<<endl;
                            }
                        }
                        //
                        // Increment the actual number of clients
                        //
                        ++actual_client;
                    } /* for(int instanceID =0;instanceID<clientInstance[j]...*/

                }/* for(Uint32 j=0; j< _clientCount; j++) */

                //
                //retrieve all PIDs and status;
                //
                clientPIDs = new pid_t[actual_client];
                clientStatus = new int[actual_client];
                prev_clientStatus = new int[actual_client];
                clientTimeStamp = new Uint64[actual_client];
                prev_clientTimeStamp = new Uint64[actual_client];
                clientActive = new Boolean[actual_client];
                nowMilliseconds = TimeValue::getCurrentTime().toMilliseconds();
                for (int i=0;i<actual_client;i++)
                {
                    clientPIDs[i] = 9999999;
                    clientStatus[i] = NO_RESPONSE;
                    clientActive[i] = true;
                    clientTimeStamp[i] = nowMilliseconds;
                    prev_clientTimeStamp[i] = nowMilliseconds;
                }
                log_file<<StressTestControllerCommand::COMMAND_NAME<<
                    "::Getting client PID's and status. "<<endl;
                if (verboseEnabled)
                {
                    outPrintWriter<<StressTestControllerCommand::COMMAND_NAME<<
                        "::Getting client PID's and status. "<<endl;
                }
                int getClientPidRc = _getClientPIDs(actual_client,log_file);
                if (!getClientPidRc)
                {
                    outPrintWriter<<
                        "Failed to communicate with clients."<<endl;
                    log_file<<StressTestControllerCommand::COMMAND_NAME<<
                        "::Failed to communicate with clients. "<<endl;
                    log_file<<
                    "                    ::Failed to get client PID & status. "
                        <<endl;
                    TestFailed = true;
                    break;
                }
            }/* if (!actual_client) */
            else
            {
                /**
                 Every CHECKUP_INTERVAL minutes check to see if tests are
                 within tolerance. Tests will cease to run if they
                 are not within tolerance. The controller will stop
                 all the clients and then exit.
                */

                //
                // Retreive all the client PIDs
                //
                int getClientPidRc = _getClientPIDs(actual_client,log_file);

                //
                // Get Current Time
                //
                nowMilliseconds = TimeValue::getCurrentTime().toMilliseconds();

                //
                // Check tolerance level if its time
                //
                if (nowMilliseconds >= nextCheckupInMillisecs)
                {
                    //
                    //  Set up the next tolerance time
                    //
                    nextCheckupInMillisecs =
                           (Uint64)convertmin2millisecs(CHECKUP_INTERVAL) +
                           nowMilliseconds;
                    //
                    //  End tests when failed to acquire the Client PID  or
                    //  status.
                    //
                    if (!getClientPidRc)
                    {
                        outPrintWriter<<
                            "Failed to communicate with clients."<<endl;
                        log_file<<StressTestControllerCommand::COMMAND_NAME<<
                            "::Failed to communicate with clients. "<<endl;
                        log_file<<StressTestControllerCommand::COMMAND_NAME<<
                            "::Get Client PID FAILED. "<<endl;
                        TestFailed = true;
                        break;
                    }
                    log_file<<StressTestControllerCommand::COMMAND_NAME<<
                        "::Checking current tolerance level. "<<endl;
                    //
                    // Output Client info if verbose is enabled.
                    //
                    if (verboseEnabled)
                    {
                        outPrintWriter<<
                            StressTestControllerCommand::COMMAND_NAME<<
                            "::Checking current tolerance level. "<<endl;
                        for (int i=0;i< actual_client; i++)
                        {
                            outPrintWriter <<" Client: "<<i;
                            outPrintWriter <<" PID: "<<clientPIDs[i]<<", ";
                            outPrintWriter <<" Status: "<<clientStatus[i]<<endl;
                            log_file <<" Client: "<<i;
                            log_file <<" PID: "<<clientPIDs[i]<<", ";
                            log_file <<" Status: "<<clientStatus[i]<<", ";
                            log_file<<"   TimeStamp: "<<
                                convertUint64toString(clientTimeStamp[i])<<
                                endl;
                        }
                    }
                    //
                    // Check the actual tolerance level
                    //
                    Boolean withinTolerance = _checkToleranceLevel(
                                                  actual_client,
                                                  nowMilliseconds,
                                                  log_file);
                    //
                    //  End tests if not within tolerance
                    //
                    if (!withinTolerance)
                    {
                        log_file<<"FAILED::Tests NOT within tolerance."<<endl;
                        errPrintWriter<<"FAILED::Tests NOT within tolerance."
                                      <<endl;
                        TestFailed = true;
                        break;
                    }
                    //
                    //  Within Tolerance - Continue tests.
                    //
                    log_file<<"********Tests are within tolerance.********* "<<
                            endl;
                    if (verboseEnabled)
                    {
                        outPrintWriter<<
                           " ********Tests are within tolerance.**********"<<
                           endl;
                    }
                } /* if (nowMilliseconds >= nextCheckupInMillisecs)*/
                //
                // Stop clients with delay
                //
                for (Uint32 clientID=0; clientID < _clientCount; clientID++)
                {
                    //
                    // Get Current time
                    //
                    nowMilliseconds =
                        TimeValue::getCurrentTime().toMilliseconds();

                    //
                    // Stop only running clients as required.
                    //
                    if (!clientStopped.get()[clientID])
                    {
                        //
                        // If Client's duration is up
                        //
                        if (clientStopMilliseconds.get()[clientID] <=
                                nowMilliseconds)
                        {
                            //
                            // Stop all the instances of this client
                            //
                            for (Uint32 instanceID = 0;
                                 instanceID<clientInstance[clientID];
                                 instanceID++)
                            {
                                log_file<<"Stopping client:("<<
                                    clientID+instanceID<<")"<<endl;
                                log_file<<"  with PID = "<<
                                    clientPIDs[clientID+instanceID]<<endl;
                                outPrintWriter<<"Stopping client:("<<
                                    clientID+instanceID<<")"<<endl;
                                outPrintWriter<<"  with PID = "<<
                                    clientPIDs[clientID+instanceID]<<endl;
                                if (verboseEnabled)
                                {
                                    tmTime = getCurrentActualTime();
                                    strftime(
                                        strTime,
                                        256,
                                        "%d/%m/%Y at %H:%M:%S\n",
                                        &tmTime);
                                    log_file<<"    Stopped on "<<strTime<<endl;
                                }
                                String stopClientFile = String::EMPTY;
                                stopClientFile.append(pegasusHome);
                                stopClientFile.append(DEFAULT_TMPDIR);
                                stopClientFile.append("STOP_");
                                sprintf(
                                    str,
                                    "%d",
                                    clientPIDs[clientID+instanceID]);
                                stopClientFile.append(str);
                                //
                                // Required for Windows
                                //
                                ofstream stop_file(
                                    stopClientFile.getCString(),
                                    ios::out);
                                stop_file << "Stop Client PID : "<<
                                          clientPIDs[clientID + instanceID]<<
                                          endl;
                                stop_file.close();
#ifndef PEGASUS_OS_TYPE_WINDOWS
                                // one more way to stop the clients.
                                int killRc =
                                  kill(clientPIDs[clientID+instanceID], SIGINT);
                                if (killRc)
                                {
                                    outPrintWriter<<"FAILED to stop client:("<<
                                        clientID+instanceID<<")"<<endl;
                                    log_file<<"FAILED to stop client:("<<
                                        clientID + instanceID<<")"<<endl;
                                }
#endif
                                //
                                // Set the client as inactive.
                                //
                                clientActive[clientID + instanceID] = false;
                            }/* for (int instanceID =0;instanceID<clientInst..*/
                            //
                            // indicate that the client was stopped.
                            //
                            clientStopped.get()[clientID] = true;
                            //
                            // If the Client has a Wait time
                            //
                            if (_clientDelays.get()[clientID] !=0)
                            {
                                clientDelayMilliseconds.get()[clientID] =
                                    nowMilliseconds +
                                        _clientDelays.get()[clientID];
                                clientDelayed.get()[clientID] = true;
                            }
                        } /* if (clientStopMilliseconds[clientID]<= nowMilli..*/
                    }  /*  if (!clientStopped.get()[clientID]) */
                    else
                    {
                        //
                        // Only restart clients that are waiting.
                        //
                        if (clientDelayed.get()[clientID])
                        {
                            //
                            // When waiting period is consumed.
                            //
                            if (clientDelayMilliseconds.get()[clientID] <=
                                nowMilliseconds)
                            {
                                //
                                //  Restart all the instances of the client.
                                //
                                for (Uint32 instanceID = 0;
                                    instanceID<clientInstance[clientID];
                                    instanceID++)
                                {
                                    act_command=String::EMPTY;
#ifdef PEGASUS_OS_TYPE_WINDOWS
                                    act_command.append("start ");
#endif
                                    act_command.append(
                                        _clientCommands.get()[clientID]);
                                    act_command.append(" -clientid ");
                                    sprintf(str,"%u",clientID+instanceID);
                                    act_command.append(str);
                                    act_command.append(" -pidfile ");
                                    act_command.append(" \"");
                                    act_command.append(
                                        _stressTestClientPIDFile);
                                    act_command.append("\"");
                                    act_command.append(" -clientlog");
                                    act_command.append(" \"");
                                    act_command.append(
                                        _stressTestClientLogFile);
                                    act_command.append("\"");
                                    act_command.append("&");
                                    log_file<<"Restarting client:("<<
                                        clientID+instanceID<<")"<<endl;
                                    outPrintWriter<<"Restarting client:("<<
                                        clientID+instanceID<<")"<<endl;
                                    if (verboseEnabled)
                                    {
                                        outPrintWriter<<"  "<<act_command<<endl;
                                        log_file<<"     ("<<
                                            clientID+instanceID<<
                                            ") \n"<<act_command<<endl;
                                        tmTime = getCurrentActualTime();
                                        strftime(
                                            strTime,
                                            256,
                                            "%d/%m/%Y at %H:%M:%S\n",
                                            &tmTime);
                                        log_file<<"   Restarted on "<<
                                            strTime<<endl;
                                    }
                                    int commandRc =
                                        system(act_command.getCString());
                                    if (commandRc)
                                    {
                                        log_file<<"Command failed to Execute."<<
                                            endl;
                                        if (verboseEnabled)
                                        {
                                            outPrintWriter<<act_command<<
                                                "Command failed to Execute."<<
                                                endl;
                                        }
                                    }
                                    clientActive[clientID+instanceID] = true;
                                } /* for (Uint32 instanceID =0;instanceID .. */
                                clientStopMilliseconds.get()[clientID] =
                                    nowMilliseconds +
                                        _clientDurations.get()[clientID];
                                clientStopped.get()[clientID] = false;
                                clientDelayed.get()[clientID] = false;
                            }/* if(clientDelayMilliseconds[clientID]<=nowMi.. */
                        } /* if(clientDelayed.get()[clientID]) */
                    } /* else ..*/
                } /* for(Uint32 clientID=0;clientID < _clientCount;clientID++)*/
            } /* else for if(!actual_client) */
            //
            // Get Current time
            //
            nowMilliseconds = TimeValue::getCurrentTime().toMilliseconds();

        } /* while(stopMilliseconds > nowMilliseconds) */

    }//try

    catch (const StressTestControllerException& e)
    {
        errPrintWriter << StressTestControllerCommand::COMMAND_NAME <<
            ": " << e.getMessage () << endl;
        return (RC_ERROR);
    }

    //
    //  Stress Tests should be stopped.
    //
    outPrintWriter<<"Ending tests::Preparing to stop all the clients."<<endl;
    log_file<<"Ending tests::Preparing to stop all the clients."<<endl;

    // Waiting to allow any clients that might have been re-started
    // just before the tests were ended to add
    // its pid to the pid file.

#ifndef PEGASUS_OS_TYPE_WINDOWS
    sleep(STOP_DELAY);
#else
    Sleep(STOP_DELAY * 1000);
#endif

    //
    // get all the clientPIDs before it is stopped.
    //
    int getClientPidRc = _getClientPIDs(actual_client,log_file);
    if (!getClientPidRc)
    {
        outPrintWriter<<"Failed to communicate with clients."<<endl;
        log_file<<StressTestControllerCommand::COMMAND_NAME<<
            "::Failed to communicate with clients. "<<endl;
        log_file<<"                    ::Failed to get client PID & status. "<<
            endl;
        TestFailed = true;
    }
    tmTime = getCurrentActualTime();
    strftime(strTime,256,"%d/%m/%Y at %H:%M:%S\n",&tmTime);
    log_file<<"Ending tests:: Stopping all the clients on "<<strTime <<endl;
    for (int i=0;i<actual_client;i++)
    {
        if(verboseEnabled)
        {
            outPrintWriter<<"Stopping Client("<<i<<") with PID:"<<
                clientPIDs[i]<<endl;
        }
        log_file<<"Stopping Client("<<i<<") with PID:"<<clientPIDs[i]<<endl;
        //
        // Required for Windows
        //
        String stopClientFile = String::EMPTY;
        stopClientFile.append(pegasusHome);
        stopClientFile.append(DEFAULT_TMPDIR);
        stopClientFile.append("STOP_");
        sprintf(str,"%d",clientPIDs[i]);
        stopClientFile.append(str);
        ofstream stop_file(stopClientFile.getCString(),ios::out);
        stop_file << "Stop Client PID : "<<clientPIDs[i]<<endl;
        stop_file.close();
#ifndef PEGASUS_OS_TYPE_WINDOWS
        // Another way to stop the client
        int killRc = kill(clientPIDs[i], SIGINT);
        if (killRc)
        {
           if (verboseEnabled)
           {
               outPrintWriter<<"Failed to stop client:("<<i<<")"<<endl;
           }
           log_file<<"Failed to stop client:("<<i<<")"<<endl;
        }
#endif
    }
    if (verboseEnabled)
    {
      outPrintWriter<<"Cleaning all resources"<<endl;
    }
    log_file<<"Cleaning all resources."<<endl;
    cleanupProcess();

    //
    // Waiting to allow clients to shutdown
    //
#ifndef PEGASUS_OS_TYPE_WINDOWS
    sleep(SHUTDOWN_DELAY);
#else
    Sleep(SHUTDOWN_DELAY * 1000);
#endif
    //
    // If the test did not run to completition
    //
    if (TestFailed)
        return(RC_ERROR);

    return (RC_SUCCESS);
} /* execute */


/*
    Retrieves the contents of the config file if specified or uses default
    values from either the default config file

    @param   fileName           The specified or default config file for the
                                tests.
    @param   log_file           The log file.

    @return  true               if the file was read successfully
             false              if file was not read successfully.

 */
Boolean StressTestControllerCommand::getFileContent(
    String fileName,
    ostream& log_file)
{
    String configData = String::EMPTY;
    String line;
    int lineNumber = 0;
    Boolean isSuccess = true;
    String name = String::EMPTY;
    String value = String::EMPTY;
    String ErrReports = String::EMPTY;

    ifstream ifs;

    //
    // Open the config file and read the stress test configuration data
    //
    Open(ifs,fileName);
    if (!ifs)
    {
        log_file<<StressTestControllerCommand::COMMAND_NAME<<
            "::Cannot read config file: "<<fileName<<endl;
        throw NoSuchFile(fileName);
    }

    log_file<<StressTestControllerCommand::COMMAND_NAME <<
        "::Storing client details. "<<endl;
    if (verboseEnabled)
    {
        cout<<StressTestControllerCommand::COMMAND_NAME<<
            "::Storing config details. "<<endl;
    }
    //
    // Get each line of the file.
    //
    while (GetLine(ifs, line))
    {
        ++lineNumber;
        IsAClient = false;
        name = String::EMPTY;
        value = String::EMPTY;
        try
        {
            //
            // Parse each line of Config file
            //
            _parseLine(line,lineNumber,name,value,log_file);
        }
        //
        // catch all the exceptions if any thrown from parseLine
        //   - Accumulate all the errors from the config file
        //   - Report all errors if found when complete.
        //
        catch (Exception& e)
        {
            char line_num[10];
            sprintf(line_num, "%d", lineNumber);
            String msg(e.getMessage());
            ErrReports.append("\n    ");
            ErrReports.append("line#");
            ErrReports.append(line_num);
            ErrReports.append(":: ");
            ErrReports.append(msg.getCString());
            log_file<<StressTestControllerCommand::COMMAND_NAME<<"::"
                <<msg<<endl;
            isSuccess = false;
        }
        catch(...)
        {
            char line_num[10];
            sprintf(line_num, "%d", lineNumber);
            ErrReports.append("\n    ");
            ErrReports.append("line#");
            ErrReports.append(line_num);
            ErrReports.append(":: ");
            ErrReports.append("Unknown exception caught.");
            log_file<<StressTestControllerCommand::COMMAND_NAME<<
                ":Unknown exception caught when parsing line."<<endl;
            cerr<<StressTestControllerCommand::COMMAND_NAME <<
               ":Unknown exception caught when parsing line."<<endl;
            return false;
        }

        if ((IsClientOptions)||(IgnoreLine))
        {
            IsClientOptions = false;
            IgnoreLine = false;
            continue;
        }
        //
        // If its a client Update the table
        //
        if (IsAClient)
        {
            _currClient=name;
            _currClientCount=_clientCount;
            //
            // save the client details in a table
            //
            if (!_storeClientDetails(name,value))
            {
                log_file<<StressTestControllerCommand::COMMAND_NAME<<
                    "::Syntax error found in line#"<<lineNumber<<
                    " of config file: "<<fileName<<endl;
                isSuccess = false;
            }
        }
        else
        {
            //
            // Common properties are updated only if they are valid.
            //
            if (isSuccess)
            {
                //
                // Store the property name and value in the table
                //
                if (verboseEnabled)
                {
                    cout<<"     "<<name<<"\t= "<<value<<endl;
                }
                if (!_propertyTable.insert(name, value))
                {
                    //
                    // Duplicate property, ignore the new property value.
                    //
#ifdef STRESSTEST_DEBUG
                    cout<< "Property Name:duplicate name already saved: "<<
                        name<<endl;
#endif
                    log_file<<StressTestControllerCommand::COMMAND_NAME<<
                        "::Duplicate property name "<<name<<" already saved."<<
                        endl;
                }
            }
        }

    }
    ifs.close();
    //
    // If a client is not read from the config file
    //
    if ((lineNumber==0)||(!_clientCount))
    {
        ErrReports.append("\n    ");
        ErrReports.append(StressTestControllerCommand::COMMAND_NAME);
        ErrReports.append("::No clients found.");
        log_file<<StressTestControllerCommand::COMMAND_NAME <<
            "::No clients found in"<<" config file: "<<fileName<<endl;
        isSuccess = false;
    }
    //
    // Error was found.
    //
    if (!isSuccess)
    {
       //
       // cleanup allocated memory
       //
       cleanupProcess();
       throw StressTestControllerException(ErrReports);
    }

    log_file<<StressTestControllerCommand::COMMAND_NAME<<
        "::Common Properties:"<<endl;
    if (verboseEnabled)
    {
        cout<<StressTestControllerCommand::COMMAND_NAME<<
            "::Common Properties:"<<endl;
    }
    for (Table::Iterator i = _propertyTable.start(); i; i++)
    {
        log_file<<"      "<<i.key()<<"\t= "<<i.value()<<endl;
        if (verboseEnabled)
        {
            cout<<"      "<<i.key()<<"\t= "<<i.value()<<endl;
        }
    }
    if (verboseEnabled)
    {
        cout<<"Total clients found:"<<_clientCount<<endl;
        cout<<"CLIENT TABLE CONTENTS:"<<endl;
    }
    log_file<<StressTestControllerCommand::COMMAND_NAME<<
        "::Client Properties:"<<endl;
    for (Uint32 j = 0; j < _clientCount; j++)
    {
        log_file<<"Client information #"<<j+1<<" from config file:"<<endl;
        if (verboseEnabled)
        {
            cout<<"Client("<<j<<")"<<endl;
        }
        for (Table::Iterator i = _clientTable.get()[j].start(); i; i++)
        {
            log_file<<"    "<<i.key()<<"    = "<<i.value()<<endl;
            if (verboseEnabled)
            {
                cout<<"   "<< i.key() << "=" << i.value() << endl;
            }
        }
    }


    if (isSuccess)
    {
       log_file<<StressTestControllerCommand::COMMAND_NAME<<
           "::Successfully retreived config values from" <<
           " config file: "<<fileName<<endl;
    }
    return isSuccess;
}/* getFileContent */


/*
    Validates the configuration data found in the specified config file or
    the default config file.
    Will validate known common/client specific properties in configuration.
    Will validate valid client names.
      (Clients are excepted to  exist in the $PEGASUS_HOME/bin directory)

    @param   vars               The property name to be validated
    @param   value              The property value associated to the above name

    @return  true               if the property was validated successfully
             false              if the property failed validation.

 */


Boolean StressTestControllerCommand::_validateConfiguration(
    String & vars,
    const String & value,
    ostream& log_file)
{
    Boolean IsValid = false;


    if (String::equalNoCase(vars,HOSTNAME))
    {
        vars=String::EMPTY;
        vars.append(HOSTNAME);
        if(!IsAClient)
        {
            if (!_hostNameSpecified)
            {
                _hostName = value;
            }
        }
        else
        {

            if(_hostName != String::EMPTY)
            {
                if (!String::equalNoCase(value,_hostName))
                {
                    log_file<<StressTestControllerCommand::COMMAND_NAME<<
                        "::Multiple hostnames were found. "<<endl;
                    if (verboseEnabled)
                    {
                        cout<<StressTestControllerCommand::COMMAND_NAME <<
                            "::Multiple hostnames were found. "<<endl;
                    }
                    return false;
                }
            }
            else
            {
                _hostName = value;
            }
        }
    }
    else if (String::equalNoCase(vars,PORTNUMBER))
    {
        vars=String::EMPTY;
        vars.append(PORTNUMBER);
        Uint32 vPortNumber = atoi(value.getCString());

        log_file<<StressTestControllerCommand::COMMAND_NAME <<
            "::Portnumber  specified in config = "<<vPortNumber<<endl;
        if (verboseEnabled)
        {
            cout<<StressTestControllerCommand::COMMAND_NAME <<
                "::Portnumber  specified in config = "<<vPortNumber<<endl;
        }
        if (vPortNumber > _MAX_PORTNUMBER)
        {
            //
            //  Portnumber out of valid range
            //
            if (verboseEnabled)
            {
                cout<<StressTestControllerCommand::COMMAND_NAME <<
                    "::Invalid portnumber  was found. "<<endl;
            }
            log_file<<StressTestControllerCommand::COMMAND_NAME<<
                "::Invalid portnumber  was found. "<<endl;
            return false;
        }
        if (!_portNumberSpecified)
        {
            _portNumber = atoi(value.getCString());
        }

    }/* else if (String::equalNoCase ... */
    else if (String::equalNoCase(vars,SSL))
    {
        log_file<<StressTestControllerCommand::COMMAND_NAME<<
            "::SSL enabled in config. "<<endl;
        if (!_useSSL)
        {
            _useSSL = true;
        }
    }
    else if (String::equalNoCase(vars,USERNAME))
    {
        vars=String::EMPTY;
        vars.append(USERNAME);
        log_file<<StressTestControllerCommand::COMMAND_NAME<<
            "::UserName  specified in config = "<<value<<endl;
        if (!_userNameSpecified)
        {
           _userName = value;
        }
    }
    else if (String::equalNoCase(vars,PASSWORD))
    {
        vars=String::EMPTY;
        vars.append(PASSWORD);
        if(!_passwordSpecified)
        {
            _password = value;
        }
        log_file<<StressTestControllerCommand::COMMAND_NAME<<
            "::Password  specified in config = "<<value<<endl;
    }
    else if (String::equalNoCase(vars,DURATION))
    {
        vars=String::EMPTY;
        vars.append(DURATION);
        //
        // converting to a double
        //
        if (!IsAClient)
        {
            _duration = atof(value.getCString());
            if (_duration > _MIN_DURATION)
            {
                log_file<<StressTestControllerCommand::COMMAND_NAME<<
                    "::Duration  specified in config = "<<value<<endl;
            }
            else
            {
                log_file<<StressTestControllerCommand::COMMAND_NAME<<
                    "::Invalid Duration  was specified. "<<endl;
                return false;
            }
        }
    }
    else if (String::equalNoCase(vars,TOLERANCELEVEL))
    {
        _toleranceLevel =  atoi(value.getCString());
        log_file<<StressTestControllerCommand::COMMAND_NAME<<
            "::ToleranceLevel  specified in config = "<<value<<endl;
        if (_toleranceLevel > _MAX_TOLERANCE)
        {
            if(verboseEnabled)
            {
                cout<<StressTestControllerCommand::COMMAND_NAME <<
                    "::Invalid Tolerance level  was specified. "<<endl;
            }
            log_file<<StressTestControllerCommand::COMMAND_NAME<<
                "::Invalid Tolerance level  was specified. "<<endl;

            return false;
        }
    }
    else if (String::equalNoCase(vars,NAMESPACE))
    {
        _nameSpace = value;
        log_file<<StressTestControllerCommand::COMMAND_NAME <<
            "::NameSpace  specified in config = "<<value<<endl;
    }
    else if (String::equalNoCase(vars,CLASSNAME))
    {
        _className = value;
        log_file<<StressTestControllerCommand::COMMAND_NAME<<
            "::Class name  specified in config = "<<value<<endl;
    }
    else
    {
       if (!IsAClient)
       {
           IsAClient=true;
           int instance = atoi(value.getCString());
           //
           // Check if the instances are set correctly
           // Must be greater than 0
           //
           if (instance <=0)
           {
              //
              // Invalid Instance value
              //
              return false;
           }
           //
           // Check if client exists or is valid.
           // Clients are expected to be in the $PEGASUS_HOME/bin directory
           //
           String clientName = String(DEFAULT_BINDIR);
           clientName.append(vars.getCString());
#if defined(PEGASUS_OS_TYPE_WINDOWS) || defined(PEGASUS_OS_VMS)
           clientName.append(".exe");
#endif
           if (!FileSystem::exists(clientName))
           {
               String testString = String::EMPTY;
               testString.append("Test");
               Uint32 Index = vars.find(testString);
               if (Index != 0)
               {
                   clientName = String(DEFAULT_BINDIR);
                   testString.append(vars.getCString());
                   clientName.append(testString.getCString());
#if defined(PEGASUS_OS_TYPE_WINDOWS) || defined(PEGASUS_OS_VMS)
                   clientName.append(".exe");
#endif
                   if (!FileSystem::exists(clientName))
                   {
                       //
                       // Invalid client name
                       //
                       IsValid = false;
                   }
                   else
                   {
                       IsValid = true;
                       vars=String::EMPTY;
                       vars.append(testString.getCString());
                   }
               }
               else
               {

                   //
                   // Invalid client name
                   //
                   IsValid = false;
               }
           }
           else
           {
               IsValid = true;
           }
           if (!IsValid)
           {
              if (verboseEnabled)
              {
                  cout<<StressTestControllerCommand::COMMAND_NAME <<
                      "::Invalid Client Name = "<<vars<<endl;
              }
              String ErrReport = String("Invalid Client Name:");
              ErrReport.append(vars.getCString());
              throw StressTestControllerException(ErrReport);
           }
           return IsValid;
       }
        log_file<<StressTestControllerCommand::COMMAND_NAME<<"::"<<vars<<
            " = "<<value<<endl;
       //
       // otherwise accept the properties listed with the clients.
       //
    }
    return true;

} /* _validateConfiguration */

/*
    Retrieves the client specific options from the config file.
    Will check for syntax errors with the client options.
    - Will retrieve all the client options in the line until ']'
    - Client options/properties in Config file are
      represented as follows:
         -   "[" indicates start of client options.
         -   "]" indicates end of client options.
         -   Client properties and values are seperated by commas.
         Example:
              [clientName=cimcli,Options=niall]
    - This method will throw appropriate exceptions.

    @param   p               The pointer to the char in the concerned line


 */
void StressTestControllerCommand::_getClientOptions(
    const Char16* p,
    ostream& log_file)
{
    //
    // Get the property name
    //
    String name = String::EMPTY;
    String value = String::EMPTY;

    while (*p != ']')
    {
        //
        // Skip whitespace after property name
        //
        while (*p && isspace(*p))
        {
            p++;
        }
        if (!(isalpha(*p) || *p == '_'))
        {
            throw StressTestControllerException(
                StressTestControllerException::INVALID_OPTION);
        }

        name.append(*p++);


        while (isalnum(*p) || *p == '_')
        {
            name.append(*p++);
        }

        //
        // Skip whitespace after property name
        //
        while (*p && isspace(*p))
        {
            p++;
        }

        //
        // Expect an equal sign
        //
        if (*p != '=')
        {
            throw StressTestControllerException(
                StressTestControllerException::INVALID_OPERATOR);
        }

        p++;

        //
        // Skip whitespace after equal sign
        //
        while (*p && isspace(*p))
        {
            p++;
        }

        //
        // Get the value
        //

        while (*p && *p != ']' && *p != ',')
        {
            value.append(*p++);
        }
        //
        // Skip whitespace after value
        //
        while (*p && isspace(*p))
        {
            cout << "got space after value\n";
            p++;
        }

        if(*p !=']' && *p != ',')
        {
            throw StressTestControllerException(
                StressTestControllerException::MISSING_BRACE);
        }
        if(value == String::EMPTY)
        {
            throw StressTestControllerException(
                StressTestControllerException::MISSING_VALUE);
        }

#ifdef STRESSTEST_DEBUG
        cout<<"name="<<name<<endl;
        cout<<"Before validate config: value="<<value<<endl;
#endif
        //
        // validate client property
        //
        Boolean IsValid=_validateConfiguration(name,value,log_file);
        if(!IsValid)
        {
            String ErrReport = String("Invalid Client property value: ");
            ErrReport.append(name);
            ErrReport.append("=");
            ErrReport.append(value);
            throw StressTestControllerException(ErrReport);
        }
        //
        // Save client property in client table if valid.
        //
        if (!_clientTable.get()[_currClientCount].insert(name,value))
        {
            //
            // Duplicate property, ignore the new property value.
            // Log this message in a log file.
            //
           log_file<< "duplicate name already saved: "<<name<<endl;
           if (verboseEnabled)
           {
              cout<< "duplicate name already saved: "<<name<<endl;
           }
        }
        if (*p ==',')
        {
            name = String::EMPTY;
            value = String::EMPTY;
            p++;
            continue;
        }
    }

    if ((name == String::EMPTY)||(value == String::EMPTY))
    {
        String ErrReport = String("Missing Name & Value for client option:");
        throw StressTestControllerException(ErrReport);
    }
} /* _getClientOptions */


/*
    Retrieves the Client PIDs and the corresponding status of the
    clients started by the Controller from the PID file.
    Each line in the PID file if not a comment is expected to
    have the following format:
       <clientid>::<client_pid>::<client_status>::<timeStampInMillisec>
    Example:
         1::7582::0::4119329327

    Client PID, status and Time Stamp from the PID file will be saved
    in the following global array's for each client.
      clientPIDs
      clientStatus
      clientTimeStamp


    @param   actual_clients  The actual number of clients executed by the
                             Controller.
    @param   log_file        The log file.

    @return  true            if the status and PIDs were read successfully
             false           Failed to read the status & PIDs of clients.

 */
Boolean StressTestControllerCommand::_getClientPIDs(
    int actual_clients,
    ostream& log_file)
{

    ifstream ifs;

    //
    // Make a temp copy of the file
    //
    Boolean cTempFile = FileSystem::copyFile(
        _stressTestClientPIDFile,
        _tmpStressTestClientPIDFile);

    if(!cTempFile)
    {
        cout<<"Cannot copy file "<<_stressTestClientPIDFile<<endl;
        log_file<<StressTestControllerCommand::COMMAND_NAME<<
            "::Cannot copy PID file: "<<_stressTestClientPIDFile<<endl;
        return (false);
    }
    //
    // Open the temp PID file and retreive all the client PIDs and status
    //
    Open(ifs,_tmpStressTestClientPIDFile);

    String line;

    const Char16* p;
    int lineNumber= 0;
    Boolean isSuccess=false;
    //
    // get each line until end of file.
    //
    while (GetLine(ifs, line))
    {
        ++lineNumber;
#ifdef STRESSTEST_DEBUG
        log_file<<" Line number:"<<lineNumber<<endl;
        log_file<<"      "<<line<<endl;
#endif
        p = line.getChar16Data();

        while (*p && isspace(*p))
        {
            p++;
        }

        //
        // Skip comment lines
        //
        if ((!*p)||(*p == '#'))
        {
            continue;
        }

        //
        // Get the client ID
        //
        String client = String::EMPTY;
        while (isalnum(*p) || *p == '_')
        {
            client.append(*p++);
        }

        //
        // Skip whitespace after property name
        //
        while (*p && isspace(*p))
        {
            p++;
        }

        //
        // Expecting a colon
        //
        if (*p != ':')
        {
            ifs.close();
            log_file<<StressTestControllerCommand::COMMAND_NAME<<
                "::Syntax Error in PID file line number:"<<lineNumber<<endl;
            FileSystem::removeFile(_tmpStressTestClientPIDFile);
            return(isSuccess = false);
        }

        //
        // point to next character in line.
        //
        p++;

        //
        // Expecting a colon
        //
        if (*p != ':')
        {
            ifs.close();
            log_file<<StressTestControllerCommand::COMMAND_NAME<<
                "::Syntax Error in PID file line number:"<<lineNumber<<endl;
            FileSystem::removeFile(_tmpStressTestClientPIDFile);
            return(isSuccess = false);
        }

        //
        // point to next character in line.
        //
        p++;

        //
        // Skip whitespace after colon
        //
        while (*p && isspace(*p))
        {
            p++;
        }

        //
        // Get the client PID
        //
        String clntPID = String::EMPTY;
        while (isalnum(*p) || *p == '_')
        {
            clntPID.append(*p++);
        }

        //
        // Skip whitespace after property name
        //
        while (*p && isspace(*p))
        {
            p++;
        }

        //
        // Expecting a colon
        //
        if (*p != ':')
        {
            ifs.close();
            log_file<<StressTestControllerCommand::COMMAND_NAME <<
                "::Syntax Error in PID file line number:"<<lineNumber<<endl;
            FileSystem::removeFile(_tmpStressTestClientPIDFile);
            return(isSuccess = false);
        }

        //
        // point to next character in line.
        //
        p++;

        //
        // Expecting a colon
        //
        if (*p != ':')
        {
            ifs.close();
            log_file<<StressTestControllerCommand::COMMAND_NAME <<
                "::Syntax Error in PID file line number:"<<lineNumber<<endl;
            FileSystem::removeFile(_tmpStressTestClientPIDFile);
            return(isSuccess = false);
        }
        p++;

        //
        // Skip whitespace after the colon if any
        //
        while (*p && isspace(*p))
        {
            p++;
        }

        String clntStatus = String::EMPTY;
        while (isalnum(*p) || *p == '_')
        {
            clntStatus.append(*p++);
        }

        //
        // Skip whitespace after property name
        //
        while (*p && isspace(*p))
        {
            p++;
        }

        //
        // Expecting a colon
        //
        if (*p != ':')
        {
            ifs.close();
            log_file<<StressTestControllerCommand::COMMAND_NAME <<
                "::Syntax Error in PID file line number:"<<lineNumber<<endl;
            FileSystem::removeFile(_tmpStressTestClientPIDFile);
            return(isSuccess = false);
        }

        //
        // next character in line.
        //
        p++;

        if (*p != ':')
        {
            ifs.close();
            log_file<<StressTestControllerCommand::COMMAND_NAME<<
                "::Syntax Error in PID file line number:"<<lineNumber<<endl;
            FileSystem::removeFile(_tmpStressTestClientPIDFile);
            return(isSuccess = false);
        }
        //
        // next character in line.
        //
        p++;
        //
        // Skip whitespace after the colon if any
        //
        while (*p && isspace(*p))
        {
            p++;
        }

        //
        // Get the client timestamp
        //
        String clntTmStmp = String::EMPTY;
        while (isalnum(*p))
        {
            clntTmStmp.append(*p++);
        }

        //
        // Store the PID, Status and TimeStamp for each client
        //
        if(atoi(client.getCString()) <= actual_clients)
        {
            clientPIDs[atoi(client.getCString())] =
                (pid_t)atoi(clntPID.getCString());
            clientStatus[atoi(client.getCString())] =
                (pid_t)atoi(clntStatus.getCString());
            sscanf(
                (const char*)clntTmStmp.getCString(),
                "%" PEGASUS_64BIT_CONVERSION_WIDTH "u",
                &clientTimeStamp[atoi(client.getCString())]);
        }
        else
        {
            if (verboseEnabled)
            {
                log_file<<StressTestControllerCommand::COMMAND_NAME<<
                    "::Unknown client PID for client#"<<
                    atoi(client.getCString())<<
                    " read at line number:"<<lineNumber<<endl;
                cout<<"Unknown Client PID recieved"<<endl;
            }
        }
    }
    //
    // remove the temporary file.
    //
    FileSystem::removeFile(_tmpStressTestClientPIDFile);
    return(isSuccess = true);
}/* _getClientPIDs */

/*
   Parses specified line to retrieve valid config data for the stress tests.
   - Identifies client specific properties from common properties in config file
   - Saves all the client specific data from the config file into appropriate
     client tables.
   - Saves all the common properties in the property table
   - This method will throw appropriate exceptions.
   Config File Format:
     - All comments begin with "#"
     - Properties in Config file are represented as follows:
          <property> = <property value>
     - Client options/properties in Config file are
       represented as follows:
         -   "[" indicates start of client options.
         -   Client properties and values are seperated by commas.
         Example:
              [clientName=cimcli,Options=niall]

    @param   line            The line that will be parsed.
    @parm    lineNumber      The line number of the line.
    @parm    name            The property name that will be retrieved.
    @parm    value           The property value of the name.
    @parm    log_file        The log file.

    @return  true            Succesfully parsed the line.
             false           Failed to parse the lines successfully.
 */
Boolean StressTestControllerCommand::_parseLine(
    const String & line,
    int lineNumber,
    String &name,
    String &value,
    ostream& log_file)
{

    const Char16* p;
    p = line.getChar16Data();

    //
    // Skip whitespace
    //
    while (*p && isspace(*p))
    {
       p++;
    }

    //
    //  Ignore empty lines
    //
    if (!*p)
    {
       IgnoreLine = true;
       return IgnoreLine;
    }

    //
    // Skip comment lines
    //
    if (*p == '#')
    {
       IgnoreLine = true;
       return IgnoreLine;
    }

    //
    // Retreive all the Client Options
    //   "[" indicates start of client options.
    //
    if (*p == '[')
    {
       IsAClient = true;
       IsClientOptions = true;
       p++;
       //
       // Ignore spaces before client property
       //
       while (*p && isspace(*p))
       {
         p++;
       }
       //
       // Invalid Client property name
       //
       if (!(isalpha(*p) || *p == '_'))
       {
            String ErrReport = String("Syntax Error with client options:");
            ErrReport.append(line.getCString());
            throw StressTestControllerException(ErrReport);
       }
       //
       // Retrieve client options
       //
       try
       {
           //
           //  get and validate client options
           //
           _getClientOptions(p,log_file);
       }
       catch (Exception& e)
       {
           String msg(e.getMessage());
           if ((name == String::EMPTY)
              ||(value == String::EMPTY))
           {
               msg.append(" in ");
               msg.append(line.getCString());
           }
           throw StressTestControllerException(msg);
       }
       catch (...)
       {
           String msg = String(
               "Unknown exception caught when geting client options.");
           log_file<<StressTestControllerCommand::COMMAND_NAME <<
               ":Unknown exception caught when geting client options."<<endl;
           cerr<<StressTestControllerCommand::COMMAND_NAME <<
               ":Unknown exception caught when geting client options."<<endl;
           throw StressTestControllerException(msg);
        }

        //
        // Successfully retrieved all the client options.
        //
        return true;
    }

    //
    // Get Common Properties
    //
    name = String::EMPTY;

    //
    // Invalid Common Property name
    //
    if (!(isalpha(*p) || *p == '_'))
    {
       String ErrReport = String("Invalid Property Value: ");
       ErrReport.append(name);
       ErrReport.append("=");
       ErrReport.append(value);
       throw StressTestControllerException(ErrReport);
    }

    //
    // Save the property Name
    //
    name.append(*p++);
    while (isalnum(*p) || *p == '_')
    {
        name.append(*p++);
    }

    //
    // Skip whitespace after property name
    //
    while (*p && isspace(*p))
    {
        p++;
    }

    //
    // Expect an equal sign
    //
    if (*p != '=')
    {
       String ErrReport = String("Invalid Property Value: ");
       ErrReport.append(name);
       ErrReport.append("=");
       ErrReport.append(value);
       throw StressTestControllerException(ErrReport);
    }

    //
    // go to next
    //
    p++;

    //
    // Retrive the property value
    //   Skip whitespace after equal sign
    //
    while (*p && isspace(*p))
    {
        p++;
    }

    //
    // Get the value
    //
    value = String::EMPTY;
    while (*p)
    {
        value.append(*p++);
    }
#ifdef STRESSTEST_DEBUG
    cout<<"name="<<name<<endl;
    cout<<"value="<<value<<endl;
#endif
    IsAClient = false;
    Boolean IsValid = false;
    //
    // Validate property and its value
    //
    try
    {
        IsValid=_validateConfiguration(name,value,log_file);
    }
    catch (Exception& e)
    {
        String msg(e.getMessage());
        throw StressTestControllerException(msg);
    }
    if (!IsValid)
    {
       String ErrReport = String("Invalid Property Value: ");
       ErrReport.append(name);
       ErrReport.append("=");
       ErrReport.append(value);
       throw StressTestControllerException(ErrReport);
    }
    return true;
} /* _parseLine */

/*
    Storing client details in a table.
    - Stores the Client name  and instance for specific clients in their
      respective client table for later use.

    @parm    name            The client name that will be stored.
    @parm    value           The number of instances of the client.

    @return  true            Succesfully stored the name or instance.
             false           Failed to store the name or instance.
 */
Boolean StressTestControllerCommand::_storeClientDetails(
    String name,
    String value)
{

    //
    // Expand the client table as required.
    //
    if (_clientCount >= Total_Clients)
    {
        Total_Clients += NEW_CLIENTS;
        Table* tempClientTable = new Table[Total_Clients];
        for (Uint32 i = 0; i < _clientCount; i++)
        {
            tempClientTable[i] = _clientTable.get()[i];
        }
        _clientTable.reset(tempClientTable);
    }

    //
    // Store the client Name in the table
    //
    if (!_clientTable.get()[_clientCount].insert(NAME, name))
    {
        //
        // Duplicate property, ignore the new property value.
        //
        if (verboseEnabled)
        {
            cout<< "Duplicate Client already saved: "<<endl;
        }
        return false;
    }

    //
    // Store the number of instances for the client in the table
    //
    if (!_clientTable.get()[_clientCount].insert(INSTANCE, value))
    {

       //
       // Duplicate property, ignore the new property value.
       //
       if(verboseEnabled)
       {
          cout<< "Duplicate Instance already saved: "<<endl;
       }
       return false;
    }
    ++_clientCount;
    return true;
} /* _storeClientDetails */

/*
    Will check the current tolerance level of the running clients with
    respect to the expected tolerance level.
    @parm    actual_client     The total number of executed clients.
    @parm    nowMilliseconds   The current time in milliseconds.
    @parm    log_file          The log_file.

    @return  true            Clients with tolerance.
             false           Clients failed tolerance.
 */
Boolean StressTestControllerCommand::_checkToleranceLevel(
    int actual_client,
    Uint64 nowMilliseconds,
    ostream& log_file)
{
    int count = 0;
    int failed_count = 0;
    Uint64 lastUpdateinMilliSec =0;
    Boolean withinTolerance = false;

    for (int i=0;i<actual_client;i++)
    {
        //
        //Observe only the status of running clients
        //
        if (clientActive[i])
        {
            ++count;
            //
            //  Validate the timestamps:
            //  The timestamps on the status is compared to the previous
            //  timestamp to ensure that the status has been updated within
            //  the previous 2 updates.
            //
            if(clientStatus[i]== VALID_RESPONSE)
            {
                //
                //check with the last timestamp
                //
                lastUpdateinMilliSec = nowMilliseconds - clientTimeStamp[i];
                //
                // Assume failure if status update is
                // longer than 2 * checkup interval
                //
                if ((clientTimeStamp[i] == prev_clientTimeStamp[i])
                   && (lastUpdateinMilliSec >=
                   (2 * (Uint64)convertmin2millisecs(CHECKUP_INTERVAL))))
                {
                    if (verboseEnabled)
                    {
                        log_file <<" Status not updated for client (" <<i<<
                            ")pid :"<<clientPIDs[i]<<endl;
                        log_file << "        for the past " <<
                            2*(CHECKUP_INTERVAL) << " minutes." << endl;
                        cout<<" Status not updated for client ("<<i<<")pid :"<<
                            clientPIDs[i]<<endl;
                        cout<<"        for the past " << 2*(CHECKUP_INTERVAL)<<
                            " minutes." << endl;
                    }
                    ++failed_count;
                }
            }
            //
            // If unknown status - server or client may be hung.
            // Two consective failures on the same client will be counted
            // as a failed client.
            //
            if (((clientStatus[i]== NO_RESPONSE)
               ||(clientStatus[i]== INVALID_RESPONSE))
               &&((prev_clientStatus[i]== NO_RESPONSE)
                 ||(prev_clientStatus[i]== INVALID_RESPONSE)))
            {
                if (verboseEnabled)
                {
                    if (clientStatus[i]== INVALID_RESPONSE)
                    {
                        log_file<<
                            "Recieved an Invalid response Status from client("<<
                            i <<") pid :"<<clientPIDs[i]<<endl;
                    }
                    else
                    {
                        log_file<<"Recieved a no response Status from client("<<
                            i <<") pid :"<<clientPIDs[i]<<endl;
                    }
                }
                ++failed_count;
            } /* if (((clientStatus[i]== NO_RESPONSE) ... */
            //
            // Save previous time stamp of client
            //
            prev_clientTimeStamp[i] = clientTimeStamp[i];
            prev_clientStatus[i] = clientStatus[i];
        } /* if (clientActive[i]) */
    }
    //
    // check actual tolerance
    //
    if(count > 0)
    {
        double curr_tolerancePercent = getToleranceInPercent(
                                           failed_count,
                                           (double)count);
        if (verboseEnabled)
        {
            cout<<" total running clients ="<<count<<endl;
            cout<<" failed clients ="<<failed_count<<endl;
            cout<<"Actual Tolerance % ="<<curr_tolerancePercent<<endl;
            cout<<"Expected Tolerance % ="<<_toleranceLevel<<endl;
            log_file<<"Total Running clients:"<<count<<endl;
            log_file<<"Actual Failed clients:"<<failed_count<<endl;
            log_file<<"::Expected Tolerance:"<<_toleranceLevel<<endl;
            log_file<<"::Actual Tolerance:"<<curr_tolerancePercent<<endl;
        }
        if ((double)_toleranceLevel >= curr_tolerancePercent)
        {
            withinTolerance = true;
        }
        return(withinTolerance);
    }
    //
    // All process are stopped.
    //
    return(withinTolerance = true);
} /* _checkToleranceLevel */


/*
    This will populate the client table with the hard coded
    values for the stress tests.
    This method is only used if the default configuration
    file does not exist.
    Default clients are 5 instances of
    "TestWrapperStressClient" and "TestModelWalkStressClient".

    @parm    log_file      The log_file.

 */
void StressTestControllerCommand::getDefaultClients(ostream& log_file)
{
    //
    // Setting the client count to default client value
    //
    _clientCount = DEFAULT_CLIENTS;

    log_file << "Populating default configuration for stress Tests." << endl;
    if (verboseEnabled)
    {
        cout << "Populating default configuration for stress Tests." << endl;
    }
    //
    // Populating default client attributes
    //
    for (Uint32 i=0;i<_clientCount; i++)
    {
        //
        // Adding the default instance value to each client table
        //
        if (!_clientTable.get()[i].insert(INSTANCE, DEFAULT_INSTANCE))
        {
            log_file <<  "Duplicate name already saved: "<<INSTANCE<<endl;
            if (verboseEnabled)
            {
                cout<< "duplicate name already saved: "<<INSTANCE<<endl;
            }
        }
        switch(i)
        {
            case 0:
            {
                if (!_clientTable.get()[i].insert(NAME, MODELWALK_CLIENT))
                {
                    log_file <<  "Duplicate name already saved: "<<NAME<<endl;
                    if (verboseEnabled)
                    {
                        cout<< "Duplicate name already saved: "<<NAME<<endl;
                    }
                }
                log_file << "Stress Test Client Name:" << MODELWALK_CLIENT <<
                    endl;
                if (verboseEnabled)
                {
                    cout<< "Stress Test Client Name:"<<MODELWALK_CLIENT<< endl;
                }

                break;
            }
            case 1:
            {
                if (!_clientTable.get()[i].insert(NAME, WRAPPER_CLIENT))
                {
                    log_file <<  "Duplicate name already saved: "<<NAME<<endl;
                    if (verboseEnabled)
                    {
                        cout<< "Duplicate name already saved: "<<NAME<<endl;
                    }
                }
                log_file << "Stress Test Client Name:" <<WRAPPER_CLIENT<< endl;
                if (verboseEnabled)
                {
                    cout << "Stress Test Client Name:" <<WRAPPER_CLIENT<< endl;
                }
                if (!_clientTable.get()[i].insert(CLIENTNAME, "cimcli"))
                {
                    log_file<< "Duplicate name already saved: "<<
                        CLIENTNAME<<endl;
                    if (verboseEnabled)
                    {
                        cout<< "Duplicate name already saved: "<<
                            CLIENTNAME<<endl;
                    }
                }
                if (!_clientTable.get()[i].insert(OPTIONS, "niall"))
                {
                    log_file<< "Duplicate name already saved: "<<OPTIONS<<endl;
                    if (verboseEnabled)
                    {
                        cout<< "Duplicate name already saved: "<<OPTIONS<<endl;
                    }
                }
                log_file <<
                    "            Client Command &  options: cimcli niall" <<
                    endl;
                if (verboseEnabled)
                {
                    cout <<
                        "            Client Command &  options: cimcli niall" <<
                        endl;
                }
                break;
            } /* case 1: */
        } /* switch(i) */
    } /* for(Uint32 i=0;i<_clientCount; i++) */

} /* getDefaultClients */




/**
    Will generate or create all the required files for the tests.
    - Required log files, pid files, client log file are created here.

    @parm    strTime         The time stamp for the tests.
                             This is used in the naming of the log file.

    @return  true            The files were successfully created.
             false           Failed to create one or more of the required
                             files.

 */
Boolean StressTestControllerCommand::generateRequiredFileNames(char *strTime)
{
    char pid_str[15];
    ofstream log_file;
    ofstream pid_file;
    ofstream clntlog_file;

    sprintf(pid_str, "%d", getpid());

    //
    // Stress Controller Log file
    //
    _stressTestLogFile.append(pegasusHome);
    _stressTestLogFile.append(TESTDIR);
    FileSystem::makeDirectory(_stressTestLogFile);
    _stressTestLogFile.append(STRESSTESTDIR);
    FileSystem::makeDirectory(_stressTestLogFile);
    _stressTestLogFile.append(LOGDIR);
    FileSystem::makeDirectory(_stressTestLogFile);
    _stressTestLogFile.append(pid_str);
    _stressTestLogFile.append("_stressTest_");
    _stressTestLogFile.append(strTime);
    _stressTestLogFile.append("log");

    //
    // StressClient PID  file
    //
    _stressTestClientPIDFile.append(pegasusHome);
    _stressTestClientPIDFile.append(DEFAULT_TMPDIR);
    FileSystem::makeDirectory(_stressTestClientPIDFile);
    _stressTestClientPIDFile.append(pid_str);
    _stressTestClientPIDFile.append("_StressTestClients");
    _stressTestClientPIDFile.append(strTime);
    _stressTestClientPIDFile.append("pid");

    //
    // StressClient Log  file
    //
    _stressTestClientLogFile.append(pegasusHome);
    _stressTestClientLogFile.append(DEFAULT_LOGDIR);
    _stressTestClientLogFile.append(pid_str);
    _stressTestClientLogFile.append("_StressTestClients");
    _stressTestClientLogFile.append(".log");

    //
    // Temporary StressClient PID/status  file
    //
    _tmpStressTestClientPIDFile.append(pegasusHome);
    _tmpStressTestClientPIDFile.append(DEFAULT_TMPDIR);
    _tmpStressTestClientPIDFile.append(pid_str);
    _tmpStressTestClientPIDFile.append("TEMP");
    _tmpStressTestClientPIDFile.append("_Clients");
    _tmpStressTestClientPIDFile.append(".pid");

    //
    // Translate slashed for appropriate OS
    //
    FileSystem::translateSlashes(_stressTestClientPIDFile);
    FileSystem::translateSlashes(_stressTestClientLogFile);
    FileSystem::translateSlashes(_stressTestLogFile);
    FileSystem::translateSlashes(_tmpStressTestClientPIDFile);

    //
    // open the file
    //
    OpenAppend(log_file,_stressTestLogFile);
    Open(pid_file,_stressTestClientPIDFile);
    Open(clntlog_file,_stressTestClientLogFile);

    //
    //  Failed to open the log file
    //
    if (!log_file)
    {
       log_file.close();
       pid_file.close();
       clntlog_file.close();
       cout<<"Cannot get file "<<_stressTestLogFile<<endl;
       return false;

    }
    //
    //  Failed to open the pid file
    //
    if (!pid_file)
    {
       cout<<"Cannot get file "<<_stressTestClientPIDFile<<endl;
       log_file<<StressTestControllerCommand::COMMAND_NAME<<
           "Cannot read file "<<_stressTestClientPIDFile<<endl;
       log_file.close();
       clntlog_file.close();
       return false;
    }
    //
    //  Failed to open the clntlog file
    //
    if (!clntlog_file)
    {
       log_file<<StressTestControllerCommand::COMMAND_NAME<<
           "Cannot read file "<<_stressTestClientLogFile<<endl;
       log_file.close();
       pid_file.close();
       return false;
    }

    //
    // Successfully opened all the files.
    //
    pid_file<<"#"<<StressTestControllerCommand::COMMAND_NAME<<
        " has the following clients:: \n";
    clntlog_file<<"#"<<StressTestControllerCommand::COMMAND_NAME<<
        "::Process ID:"<<getpid()<<endl;
    clntlog_file.close();
    pid_file.close();
    return true;

} /* generateRequiredFileNames */

/**
    Will remove all the unused files for the tests.
    - Unused log files & pid files are removed here.

 */
void StressTestControllerCommand::removeUnusedFiles()
{
    FileSystem::removeFile(_stressTestClientPIDFile);
    FileSystem::removeFile(_stressTestClientLogFile);
}

PEGASUS_NAMESPACE_END

/**
    Cleanup function for stressTestController to free allocated
    memory used to execute clients.
 */
void cleanupProcess()
{

    delete [] clientPIDs;
    delete [] clientStatus;
    delete [] clientInstance;
    delete [] clientActive;
    delete [] clientTimeStamp;
    delete [] prev_clientTimeStamp;
    delete [] prev_clientStatus;
}

/*
    This will generate the current time.
 */
struct tm getCurrentActualTime()
{
    struct tm tmTime;
    time_t inTime=time(NULL);
#ifdef PEGASUS_OS_TYPE_WINDOWS
    tmTime=*localtime(&inTime);
#else
    localtime_r(&inTime,&tmTime);
#endif
    return tmTime;
}

/**
    Signal handler for SIGINT, SIGABRT.

    @param   signum  the signal identifier
*/
void endAllTests(int signum)
{
    if (verboseEnabled)
    {
        switch(signum)
        {
            case SIGINT:
            {
                cout<<StressTestControllerCommand::COMMAND_NAME<<
                    "::Recieved interupt signal SIGINT!"<<endl;
                break;
            }
            case SIGABRT:
            {
                cout<<StressTestControllerCommand::COMMAND_NAME<<
                    "::Recieved signal SIGABRT!"<<endl;
                break;
            }
            default:
            {
                cout<<StressTestControllerCommand::COMMAND_NAME<<
                    "::Recieved Signal ( "<<signum<<"!" <<endl;
                break;
            }
        }
    }
    //
    // Sets the variable that will interupt stress tests
    //
    Quit = true;
} /* endAllTests */

/**
    This function will convert a Uint64
    to a string.

    @param   x       The Uint64 integer

    @return  String  Returns the converted string.
*/
String convertUint64toString(Uint64 x)
{
    char buffer[32];
    sprintf(buffer, "%" PEGASUS_64BIT_CONVERSION_WIDTH "u", x);
    return(String(buffer));
}/* convertUint64toString(..) */
