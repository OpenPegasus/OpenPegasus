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
//
//L10N TODO: Internal benchmark utility. Although this module contains
//messages, there is currently no expectation that it will be
//localized.

#include <iostream>
#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/Constants.h>
#include <Pegasus/Common/System.h>
#include <Pegasus/Common/FileSystem.h>
#include <Pegasus/Common/String.h>
#include <Pegasus/Common/PegasusVersion.h>
#include <Pegasus/Common/SSLContext.h>
#include <Pegasus/General/Stopwatch.h>

#include <Pegasus/Common/Tracer.h>

#include <Pegasus/getoopt/getoopt.h>
#include <Clients/cliutils/CommandException.h>
#include "benchmarkTest.h"
#include "../benchmarkDefinition/benchmarkDefinition.h"

FILE *_resultsFileHandle = NULL;
String _startTime;

#ifndef PLATFORM_PRODUCT_VERSION
# define PLATFORM_PRODUCT_VERSION "1.0.0"
#endif

PEGASUS_NAMESPACE_BEGIN

#define NAMESPACE CIMNamespaceName ("root/benchmark")
#define MODULENAME String ("benchmarkProviderModule")

benchmarkDefinition test;

/**
    The command name.
 */
const char   benchmarkTestCommand::COMMAND_NAME []      = "benchmarkTest";

//    Label for the usage string for this command.
const char   benchmarkTestCommand::_USAGE []            = "usage: ";

//    The option character used to specify the hostname.
const char   benchmarkTestCommand::_OPTION_HOSTNAME     = 'h';

//    The option character used to specify the port number.
const char   benchmarkTestCommand::_OPTION_PORTNUMBER   = 'p';

//    The option character used to specify SSL usage.
const char   benchmarkTestCommand::_OPTION_SSL          = 's';

//    The option character to request that the version be displayed.
const char   benchmarkTestCommand::_OPTION_VERSION      = 'v';

//    The option character used to specify the timeout value.
const char   benchmarkTestCommand::_OPTION_TIMEOUT      = 't';

//    The option character used to specify the username.
const char   benchmarkTestCommand::_OPTION_USERNAME     = 'u';

//    The option character used to specify the password.
const char   benchmarkTestCommand::_OPTION_PASSWORD     = 'w';

//    The option character used to specify the iteration parameter.
const char benchmarkTestCommand::_OPTION_ITERATIONS     = 'i';

//    The option character used to specify the TESTID parameter.
const char benchmarkTestCommand::_OPTION_TESTID         = 'n';

//    The option character used to specify the TESTID parameter.
const char benchmarkTestCommand::_OPTION_RESULTS_DIRECTORY  = 'D';

//    The option character used to specify that a debug option is requested.
const char   benchmarkTestCommand::_OPTION_DEBUG        = 'd';

//    The minimum valid portnumber.
const Uint32 benchmarkTestCommand::_MIN_PORTNUMBER      = 0;

//    The maximum valid portnumber.
const Uint32 benchmarkTestCommand::_MAX_PORTNUMBER      = 65535;

static const char PASSWORD_PROMPT []  =
                     "Please enter your password: ";

static const char PASSWORD_BLANK []  =
                     "Password cannot be blank. Please re-enter your password.";

//    The debug option argument value used to enable client-side tracing.
const char   benchmarkTestCommand::_DEBUG_OPTION1       = '1';

//    The debug option argument value used to disable printing verbose report.
const char   benchmarkTestCommand::_DEBUG_OPTION2       = '2';


static const Uint32 MAX_PW_RETRIES = 3;

static Boolean verifyCertificate(SSLCertificateInfo &certInfo)
{

#if 0
    outPrintWriter << certInfo.getSubjectName() << endl;
#endif
    return true;
}

/**

    Constructs a benchmarkTestCommand and initializes instance variables.

 */
benchmarkTestCommand::benchmarkTestCommand ()
{

    _hostName            = String ();
    _resultsDirectory        = String ();
    _hostNameSet         = false;
    _portNumber          = WBEM_DEFAULT_HTTP_PORT;
    _portNumberSet       = false;

    char buffer[32];
    sprintf(buffer, "%lu", (unsigned long) _portNumber);
    _portNumberStr       = buffer;

    _testID              = 0;
    _testIDSet           = false;

    _timeout             = DEFAULT_TIMEOUT_MILLISECONDS;
    _userName            = String ();
    _userNameSet         = false;
    _password            = String ();
    _passwordSet         = false;
    _useSSL              = false;
    _displayVersion      = false;
    _iterations          = DEFAULT_NUMBER_OF_ITERATIONS;
    _iterationsSet       = false;

    _debugOption1        = false;
    _generateReport      = true;

    String usage = String (_USAGE);
    usage.append (COMMAND_NAME);
    usage.append (" [ -");
#ifdef PEGASUS_HAS_SSL
    usage.append (_OPTION_SSL);
    usage.append (" ] [ -");
#endif
    usage.append (_OPTION_VERSION);
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
    usage.append (_OPTION_RESULTS_DIRECTORY);
    usage.append (" resultsDirectory ] [ -");
    usage.append (_OPTION_ITERATIONS);
    usage.append (" iterations ] [ -");
    usage.append (_OPTION_TESTID);
    usage.append (" testNumber ] [ -");
    usage.append (_OPTION_DEBUG);
    usage.append (_DEBUG_OPTION1);
    usage.append (" [ -");
    usage.append (_OPTION_DEBUG);
    usage.append (_DEBUG_OPTION2);
    usage.append (" ]");
    setUsage (usage);
}

String benchmarkTestCommand::_promptForPassword( ostream& outPrintWriter )
{
  //
  // Password is not set, prompt for non-blank password
  //
  String pw = String::EMPTY;
  Uint32 retries = 1;
  do
    {
      pw = System::getPassword( PASSWORD_PROMPT );

      if ( pw == String::EMPTY || pw == "" )
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
          pw = String::EMPTY;
          continue;
        }
    }
  while ( pw == String::EMPTY );
  return( pw );
}

/**

    Connects to cimserver.

    @param   outPrintWriter     the ostream to which error output should be
                                written

    @exception       Exception  if an error is encountered in creating
                               the connection

 */
 void benchmarkTestCommand::_connectToServer( CIMClient& client,
                         ostream& outPrintWriter )
{
    String                 host                  = String ();
    Uint32                 portNumber            = 0;
    Boolean                connectToLocal        = false;

    //
    //  Construct host address
    //

    if ((!_hostNameSet) && (!_portNumberSet) && \
        (!_userNameSet) && (!_passwordSet))
    {
        connectToLocal = true;
    }
    else
    {
        if (!_hostNameSet)
        {
           _hostName = System::getHostName();
        }
        if( !_portNumberSet )
        {
           if( _useSSL )
           {
#ifdef PEGASUS_HAS_SSL
               _portNumber = System::lookupPort( WBEM_HTTPS_SERVICE_NAME,
                                          WBEM_DEFAULT_HTTPS_PORT );
#else
               PEGASUS_TEST_ASSERT(false);
#endif
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
    }
    host = _hostName;
    portNumber = _portNumber;

    if( connectToLocal )
    {
        client.connectLocal();
    }
    else if( _useSSL )
    {
#ifdef PEGASUS_HAS_SSL
        //
        // Get environment variables:
        //
        const char* pegasusHome = getenv("PEGASUS_HOME");

        String certpath = FileSystem::getAbsolutePath(
            pegasusHome, PEGASUS_SSLCLIENT_CERTIFICATEFILE);

        String randFile = String::EMPTY;

#ifdef PEGASUS_SSL_RANDOMFILE
        randFile = FileSystem::getAbsolutePath(
             pegasusHome, PEGASUS_SSLCLIENT_RANDOMFILE);
#endif
        SSLContext  sslcontext (certpath, verifyCertificate, randFile);

        if (!_userNameSet)
        {
           _userName = System::getEffectiveUserName();
        }

        if (!_passwordSet)
        {
            _password = _promptForPassword( outPrintWriter );
        }
        client.connect(host, portNumber, sslcontext,  _userName, _password );
#else
        PEGASUS_TEST_ASSERT(false);
#endif
    }
    else
    {
        if (!_passwordSet)
        {
            _password = _promptForPassword( outPrintWriter );
        }
        client.connect(host, portNumber, _userName, _password );
     }
}

/**

    Parses the command line, validates the options, and sets instance
    variables based on the option arguments.

    @param   argc  the number of command line arguments
    @param   argv  the string vector of command line arguments

    @exception  CommandFormatException  if an error is encountered in parsing
                                        the command line

 */
void benchmarkTestCommand::setCommand (Uint32 argc, char* argv [])
{
    String httpVersion;
    String httpMethod;
    String timeoutStr;
    String GetOptString;
    getoopt getOpts;

    //
    //  Construct GetOptString
    //
    GetOptString.append (_OPTION_HOSTNAME);
    GetOptString.append (getoopt::GETOPT_ARGUMENT_DESIGNATOR);
    GetOptString.append (_OPTION_PORTNUMBER);
    GetOptString.append (getoopt::GETOPT_ARGUMENT_DESIGNATOR);
    GetOptString.append (_OPTION_VERSION);
#ifdef PEGASUS_HAS_SSL
    GetOptString.append (_OPTION_SSL);
#endif
    GetOptString.append (_OPTION_TIMEOUT);
    GetOptString.append (getoopt::GETOPT_ARGUMENT_DESIGNATOR);
    GetOptString.append (_OPTION_USERNAME);
    GetOptString.append (getoopt::GETOPT_ARGUMENT_DESIGNATOR);
    GetOptString.append (_OPTION_PASSWORD);
    GetOptString.append (getoopt::GETOPT_ARGUMENT_DESIGNATOR);
    GetOptString.append (_OPTION_RESULTS_DIRECTORY);
    GetOptString.append (getoopt::GETOPT_ARGUMENT_DESIGNATOR);
    GetOptString.append (_OPTION_ITERATIONS);
    GetOptString.append (getoopt::GETOPT_ARGUMENT_DESIGNATOR);
    GetOptString.append (_OPTION_TESTID);
    GetOptString.append (getoopt::GETOPT_ARGUMENT_DESIGNATOR);
    GetOptString.append (_OPTION_DEBUG);
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
    for (Uint32 i = getOpts.first(); i < getOpts.last(); i++)
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
            Uint32 c = getOpts [i].getopt () [0];

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
#ifdef PEGASUS_HAS_SSL
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
#endif
                case _OPTION_VERSION:
                {
            _displayVersion = true;
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

                case _OPTION_RESULTS_DIRECTORY:
                {
                    if (getOpts.isSet (_OPTION_RESULTS_DIRECTORY) > 1)
                    {
                        //
                        // More than one log directory option was found
                        //
                        throw DuplicateOptionException(
                            _OPTION_RESULTS_DIRECTORY);
                    }
                    _resultsDirectory = getOpts [i].Value ();
                    break;
                }

                case _OPTION_ITERATIONS:
                {
                    if (getOpts.isSet (_OPTION_ITERATIONS) > 1)
                    {
                        //
                        // More than one iteration option was found
                        //
                        throw DuplicateOptionException(_OPTION_ITERATIONS);
                    }

                    _iterationsStr = getOpts [i].Value ();

                    try
                    {
                        getOpts [i].Value (_iterations);
                    }
                    catch (const TypeMismatchException&)
                    {
                        throw InvalidOptionArgumentException(
                            _iterationsStr,
                            _OPTION_ITERATIONS);
                    }
                    _iterationsSet = true;
                    break;
                }

                case _OPTION_TESTID:
                {
                    if (getOpts.isSet (_OPTION_TESTID) > 1)
                    {
                        //
                        // More than one TESTID option was found
                        //
                        throw DuplicateOptionException(_OPTION_TESTID);
                    }

                    _testIDStr = getOpts [i].Value ();

                    try
                    {
                        getOpts [i].Value (_testID);
                    }
                    catch (const TypeMismatchException&)
                    {
                        throw InvalidOptionArgumentException(
                            _testIDStr,
                            _OPTION_TESTID);
                    }
                    _testIDSet = true;
                    break;
                }

                case _OPTION_DEBUG:
                {
                    //
                    //
                    String debugOptionStr;

                    debugOptionStr = getOpts [i].Value ();

                    if (debugOptionStr.size () != 1)
                    {
                        //
                        //  Invalid debug option
                        //
                        throw InvalidOptionArgumentException(
                            debugOptionStr,
                            _OPTION_DEBUG);
                    }

                    if (debugOptionStr [0] == _DEBUG_OPTION1)
                    {
                        _debugOption1 = true;
                    }
                    else if (debugOptionStr [0] == _DEBUG_OPTION2)
                    {
                        _generateReport = false;
                    }
                    else
                    {
                        //
                        //  Invalid debug option
                        //
                        throw InvalidOptionArgumentException(
                            debugOptionStr,
                            _OPTION_DEBUG);
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
        if (_timeout == 0)
        {
            //
            //  Timeout out of valid range
            //
            throw InvalidOptionArgumentException(timeoutStr, _OPTION_TIMEOUT);
        }
    }

    if (_displayVersion)
    {
        cout << PLATFORM_PRODUCT_VERSION << __DATE__ << endl;
    }

    if (_generateReport)
    {
        for (Uint32 i = 0; i < argc; i++)
        {
            cout << argv[i] << " ";
        }
        cout << endl <<  endl;
    }

}

/** ErrorExit - Print out the error message and exits.
    @param   errPrintWriter     The ostream to which error output should be
                                written
    @param   message            Text for error message
    @return - None, Terminates the program
    @exception - This function terminates the program
*/
void benchmarkTestCommand::errorExit( ostream& errPrintWriter,
                               const String& message)
{
    errPrintWriter << "benchmarkTest error: " << message << endl;
    exit(1);
}

void benchmarkTestCommand::_getCSInfo(ostream& outPrintWriter,
                                      ostream& errPrintWriter)
{

#define CSINFO_NAMESPACE CIMNamespaceName ("root/cimv2")
#define CSINFO_CLASSNAME CIMName ("PG_ComputerSystem")

    Boolean isConnected = false;
    CIMClient client;
    client.setTimeout( _timeout );

    try
    {
        _connectToServer( client, outPrintWriter);
        isConnected = true;

        Boolean deepInheritance = true;
        Boolean localOnly = false;
        Boolean includeQualifiers = false;
        Boolean includeClassOrigin = false;
        Array<CIMInstance> cimNInstances =
               client.enumerateInstances(CSINFO_NAMESPACE, CSINFO_CLASSNAME,
                                         deepInheritance,
                                         localOnly, includeQualifiers,
                                         includeClassOrigin );

        if (cimNInstances.size() != 1)
        {
           client.disconnect();
           return;
        }

        CIMObjectPath instanceRef = cimNInstances[0].getPath();
        if ( !(instanceRef.getClassName().equal(CSINFO_CLASSNAME)))
        {
           errPrintWriter << "Returned ClassName = "
               << instanceRef.getClassName().getString() << endl;
           errPrintWriter << "Expected ClassName = "
               << CSINFO_CLASSNAME.getString() << endl;
           errorExit(errPrintWriter,
               "EnumerateInstances failed.  Incorrect class name returned.");
        }

        for (Uint32 j = 0; j < cimNInstances[0].getPropertyCount(); j++)
        {
           CIMName propertyName = cimNInstances[0].getProperty(j).getName();
           if (propertyName.equal(CIMName("OtherIdentifyingInfo")))
           {
               outPrintWriter << "Model = "
                   << cimNInstances[0].getProperty(j).getValue().toString()
                   << endl;
           }
        }

    }  // end try

    catch(...)
    {
       if (isConnected)
          client.disconnect();
       return;
    }

    client.disconnect();
}


void benchmarkTestCommand::_getOSInfo(ostream& outPrintWriter,
                                      ostream& errPrintWriter)
{

#define OSINFO_NAMESPACE CIMNamespaceName ("root/cimv2")
#define OSINFO_CLASSNAME CIMName ("PG_OperatingSystem")

    Boolean isConnected = false;
    CIMClient client;
    client.setTimeout( _timeout );

    try
    {
        _connectToServer( client, outPrintWriter);
        isConnected = true;

        Boolean deepInheritance = true;
        Boolean localOnly = false;
        Boolean includeQualifiers = false;
        Boolean includeClassOrigin = false;
        Array<CIMInstance> cimNInstances =
               client.enumerateInstances(OSINFO_NAMESPACE, OSINFO_CLASSNAME,
                                         deepInheritance,
                                         localOnly, includeQualifiers,
                                         includeClassOrigin );

        // while we only have one instance (the running OS), we can take the
        // first instance.  When the OSProvider supports installed OSs as well,
        // will need to select the runningOS instance

        if (cimNInstances.size() != 1)
        {
           client.disconnect();
           return;
        }

        CIMObjectPath instanceRef = cimNInstances[0].getPath();
        if ( !(instanceRef.getClassName().equal(OSINFO_CLASSNAME)) )
        {
           errPrintWriter << "Returned ClassName = "
               << instanceRef.getClassName().getString() << endl;
           errPrintWriter << "Expected ClassName = "
               << OSINFO_CLASSNAME.getString() << endl;
           errorExit(errPrintWriter,
               "enumerateInstances failed.  Incorrect class name returned.");
        }

        for (Uint32 j = 0; j < cimNInstances[0].getPropertyCount(); j++)
        {
           CIMName propertyName = cimNInstances[0].getProperty(j).getName();
           if (propertyName.equal(CIMName("CSName")))
           {
               outPrintWriter << "Computer System Name = "
                   << cimNInstances[0].getProperty(j).getValue().toString()
                   << endl;
           }

           if (propertyName.equal(CIMName("Version")))
           {
               outPrintWriter << "Version = "
                   << cimNInstances[0].getProperty(j).getValue().toString()
                   << endl;
           }
        }

    }  // end try

    catch(...)
    {
       if (isConnected)
          client.disconnect();
       return;
    }

    client.disconnect();
}

void benchmarkTestCommand::_getSystemConfiguration(
                                      ostream& outPrintWriter,
                                      ostream& errPrintWriter)
{
   benchmarkTestCommand::_getOSInfo(outPrintWriter, errPrintWriter);
   benchmarkTestCommand::_getCSInfo(outPrintWriter, errPrintWriter);
   outPrintWriter << endl << endl;
}

void benchmarkTestCommand::_getTestConfiguration(
                      ostream& outPrintWriter,
                      ostream& errPrintWriter)
{
    Boolean isConnected = false;
    CIMClient client;
    client.setTimeout( _timeout );

    try
    {
        _connectToServer( client, outPrintWriter);
        isConnected = true;

        Boolean deepInheritance = true;

        Array<CIMName> classNames = client.enumerateClassNames(
                  NAMESPACE, CIMName(), deepInheritance);


        Uint32 numberOfProperties;
        Uint32 sizeOfPropertyValue;
        Uint32 numberOfInstances;

        for (Uint32 i = 0, n = classNames.size(); i < n; i++)
        {
            if (CIM_ERR_SUCCESS == test.getConfiguration(classNames[i],
                 numberOfProperties, sizeOfPropertyValue, numberOfInstances))
            {
               _testClassNames.append (classNames[i]);
            }
        }

    }  // end try

    catch (const Exception&)
    {
        if (isConnected)
           client.disconnect();
        throw;
    }

    catch (const exception&)
    {
        if (isConnected)
           client.disconnect();
        throw;
    }

    client.disconnect();
}


CIMObjectPath benchmarkTestCommand::_buildObjectPath(
                         const CIMName& className,
                         CIMValue Identifier)
{
    Array<CIMKeyBinding> keys;

    keys.append(CIMKeyBinding("Identifier", Identifier.toString(),
        CIMKeyBinding::NUMERIC));

    return CIMObjectPath(String(),CIMNamespaceName(NAMESPACE),className,keys);
}

Boolean benchmarkTestCommand::_invokeProviderModuleMethod(
                                         const String& moduleName,
                                         const CIMName& methodName,
                                         ostream&    outPrintWriter,
                                         ostream&    errPrintWriter)
{
    CIMClient client;
    Boolean isConnected = false;
    Sint16 retValue = -1;
    client.setTimeout( _timeout );
    CIMObjectPath moduleRef;
    try
    {
        _connectToServer( client, outPrintWriter);
        isConnected = true;

        //
        // disable the module
        //

        moduleRef.setNameSpace(PEGASUS_NAMESPACENAME_PROVIDERREG);
        moduleRef.setClassName(PEGASUS_CLASSNAME_PROVIDERMODULE);
        CIMKeyBinding kb1(CIMName ("Name"), moduleName,
            CIMKeyBinding::STRING);
        Array<CIMKeyBinding> keys;
        keys.append(kb1);

        moduleRef.setKeyBindings(keys);

        Array<CIMParamValue> inParams;
        Array<CIMParamValue> outParams;

        CIMValue ret_value = client.invokeMethod(
                PEGASUS_NAMESPACENAME_PROVIDERREG,
                moduleRef,
                methodName,
                inParams,
                outParams);

        ret_value.get(retValue);
        client.disconnect();
    }

    catch (const Exception&)
    {
        if (isConnected)
           client.disconnect();
        throw;
    }

    if ((retValue == 0) || (retValue == 1))
        return true;
    else return false;
}

void benchmarkTestCommand::dobenchmarkTest1(
                              Uint32 testID,
                              ostream& outPrintWriter,
                              ostream& errPrintWriter)
{
    CIMClient client;
    client.setTimeout( _timeout );

    try
    {
        Stopwatch stopwatchTime;

        stopwatchTime.start();

        for (Uint32 i = 0; i < _iterations; i++)
        {
           _connectToServer( client, outPrintWriter);
           client.disconnect();
        }

        stopwatchTime.stop();

        double elapsedTime = stopwatchTime.getElapsed();

        outPrintWriter << testID;
        outPrintWriter <<  ": Benchmark Test #1: Connect/Disconnect Test"
            << endl;
        outPrintWriter << _iterations << " requests processed in "
            << stopwatchTime.getElapsed() << " Seconds "
            << "(Average Elapse Time = " << elapsedTime/_iterations
            << ")" << endl << endl;

        if (_resultsFileHandle != NULL)
        {
            fprintf(_resultsFileHandle,
            "Connect/Disconnect\t%f\t", elapsedTime);
        }

    }  // end try

    catch(const Exception& e)
    {
      errorExit(errPrintWriter, e.getMessage());
    }
}

void benchmarkTestCommand::dobenchmarkTest2(
                              Uint32 testID,
                              const CIMName& className,
                              ostream& outPrintWriter,
                              ostream& errPrintWriter)
{
    CIMClient client;
    client.setTimeout( _timeout );

    try
    {
        Stopwatch stopwatchTime;

        stopwatchTime.start();

        _connectToServer(client, outPrintWriter);

        stopwatchTime.stop();

        double connectTime = stopwatchTime.getElapsed();

        stopwatchTime.reset();
        stopwatchTime.start();

        if (_invokeProviderModuleMethod(MODULENAME, CIMName("stop"),
                                        outPrintWriter, errPrintWriter))
        {
            _invokeProviderModuleMethod(MODULENAME, CIMName("start"),
                                        outPrintWriter, errPrintWriter);
        }

        stopwatchTime.stop();

        double _unloadModuleTime = stopwatchTime.getElapsed();

        stopwatchTime.reset();
        stopwatchTime.start();

        CIMObjectPath reference =
           benchmarkTestCommand::_buildObjectPath(className, CIMValue(99));
        CIMInstance cimInstance = client.getInstance(NAMESPACE, reference);

        CIMObjectPath instanceRef = cimInstance.getPath();
        if ( !(instanceRef.getClassName().equal(className)))
        {
            outPrintWriter << "Returned ClassName = "
                << instanceRef.getClassName().getString() << endl;
            outPrintWriter << "Expected ClassName = "
                << className.getString() << endl;
            errorExit(errPrintWriter,
                "getInstance failed.  Incorrect class name returned.");
        }

        stopwatchTime.stop();

        double elapsedTime = stopwatchTime.getElapsed();

        outPrintWriter << testID
            << ": Benchmark Test #2: Load Provider Test on class "
            << className.getString() << endl;
        outPrintWriter << "Connect time = " << connectTime << endl;
        outPrintWriter << "Unload Module time = " << _unloadModuleTime << endl;
        outPrintWriter << "First getInstance request processed in "
            << elapsedTime << " Seconds "
            << endl << endl;

        if (_resultsFileHandle != NULL)
        {
            fprintf(_resultsFileHandle,
            "Load/Unload\t%f\t", elapsedTime);
        }

        client.disconnect();

    }  // end try

    catch(const Exception& e)
    {
         errorExit(errPrintWriter, e.getMessage());
    }
}

void benchmarkTestCommand::dobenchmarkTest3(
                              Uint32 testID,
                              const CIMName& className,
                              ostream& outPrintWriter,
                              ostream& errPrintWriter)
{
    CIMClient client;
    client.setTimeout( _timeout );

    try
    {
        Stopwatch stopwatchTime;

        stopwatchTime.reset();
        stopwatchTime.start();

        _connectToServer( client, outPrintWriter);

        stopwatchTime.stop();

        double connectTime = stopwatchTime.getElapsed();

        Uint32 numberOfProperties;
        Uint32 sizeOfPropertyValue;
        Uint32 expectedNumberOfInstances;

        test.getConfiguration(className, numberOfProperties,
                 sizeOfPropertyValue, expectedNumberOfInstances);

        stopwatchTime.reset();
        stopwatchTime.start();

        CIMObjectPath reference =
            benchmarkTestCommand::_buildObjectPath(className, CIMValue(99));

        for (Uint32 i = 0; i < _iterations; i++)
        {
            CIMInstance cimInstance = client.getInstance(NAMESPACE, reference);
            CIMObjectPath instanceRef = cimInstance.getPath();
            if ( !(instanceRef.getClassName().equal(className)))
            {
                outPrintWriter << "Returned ClassName = "
                    << instanceRef.getClassName().getString() << endl;
                outPrintWriter << "Expected ClassName = "
                    << className.getString() << endl;
                errorExit(errPrintWriter,
                    "getInstance failed. Incorrect class name returned.");
            }

            if ( cimInstance.getPropertyCount() != numberOfProperties+1)
            {
                errorExit(errPrintWriter, "getInstance failed. "
                   "Incorrect number of properties returned.");
            }
        }

        stopwatchTime.stop();

        double elapsedTime = stopwatchTime.getElapsed();

        outPrintWriter << testID << ": Benchmark Test #3: getInstance Test on "
            << className.getString() << endl;
        outPrintWriter << "Connect time = " << connectTime << endl;
        outPrintWriter << "Number of Non-Key Properties Returned  = "
            << numberOfProperties << endl
            << "Size of Each Non-Key Property Returned  = "
            << sizeOfPropertyValue << endl
            << "Number of Instances Returned = " << 1 << endl;
        outPrintWriter << _iterations << " requests processed in "
            << elapsedTime << " Seconds "
            << "(Average Elapse Time = " << elapsedTime/_iterations
            << ")" << endl << endl;

        if (_resultsFileHandle != NULL)
        {
            fprintf(_resultsFileHandle,
                "getInstance\t%s\t%f\t",
                (const char *)className.getString().getCString(),
                elapsedTime/_iterations);
        }

        client.disconnect();

    }  // end try

    catch(const Exception& e)
    {
      errorExit(errPrintWriter, e.getMessage());
    }
}

void benchmarkTestCommand::dobenchmarkTest4(
                              Uint32 testID,
                              const CIMName& className,
                              ostream& outPrintWriter,
                              ostream& errPrintWriter)
{

    CIMClient client;
    client.setTimeout( _timeout );

    try
    {
        Stopwatch stopwatchTime;

        stopwatchTime.reset();
        stopwatchTime.start();

        _connectToServer( client, outPrintWriter);

        stopwatchTime.stop();

        double connectTime = stopwatchTime.getElapsed();

        Uint32 numberOfProperties;
        Uint32 sizeOfPropertyValue;
        Uint32 expectedNumberOfInstances;

        test.getConfiguration(className, numberOfProperties,
                 sizeOfPropertyValue, expectedNumberOfInstances);

        Uint32 numberInstances;

        stopwatchTime.reset();
        stopwatchTime.start();

        for (Uint32 i = 0; i < _iterations; i++)
        {
           Array<CIMObjectPath> cimInstanceNames =
                  client.enumerateInstanceNames(NAMESPACE, className);

           numberInstances = cimInstanceNames.size();
           if (numberInstances != expectedNumberOfInstances)
           {
              errorExit(errPrintWriter,
                  "enumerateInstanceNames failed. "
                  "Incorrect number of instances returned.");
           }

           for (Uint32 j = 0; j < numberInstances; j++)
           {
              if ( !(cimInstanceNames[j].getClassName().equal(className)))
              {
                 errorExit(errPrintWriter, "enumerateInstanceNames failed. "
                    "Incorrect class name returned.");
              }

            }   // end for looping through instances
        }

        stopwatchTime.stop();

        double elapsedTime = stopwatchTime.getElapsed();

        outPrintWriter << testID
            << ": Benchmark Test #4: enumerateInstanceNames Test on class "
            << className.getString() << endl;
        outPrintWriter << "Connect time = " << connectTime << endl;
        outPrintWriter << "Number of Non-Key Properties Returned  = "
            << 0 << endl
            << "Number of Instances Returned = "
            << expectedNumberOfInstances << endl;
        outPrintWriter << _iterations << " requests processed in "
            << elapsedTime << " Seconds "
            << "(Average Elapse Time = " << elapsedTime/_iterations
            << ")" << endl << endl;

        if (_resultsFileHandle != NULL)
        {
            fprintf(_resultsFileHandle,
                "enumerateInstanceNames\t%s\t%f\t",
                (const char *)className.getString().getCString(),
                elapsedTime/_iterations);
        }

        client.disconnect();
    }  // end try

    catch(const Exception& e)
    {
      errorExit(errPrintWriter, e.getMessage());
    }

}

void benchmarkTestCommand::dobenchmarkTest5(
                              Uint32 testID,
                              const CIMName& className,
                              ostream& outPrintWriter,
                              ostream& errPrintWriter)
{

    CIMClient client;
    client.setTimeout( _timeout );

    try
    {
        Stopwatch stopwatchTime;

        stopwatchTime.reset();
        stopwatchTime.start();

        _connectToServer(client, outPrintWriter);

        stopwatchTime.stop();

        double connectTime = stopwatchTime.getElapsed();

        Uint32 numberOfProperties;
        Uint32 sizeOfPropertyValue;
        Uint32 expectedNumberOfInstances;

        test.getConfiguration(className, numberOfProperties,
                 sizeOfPropertyValue, expectedNumberOfInstances);

        Uint32 numberInstances;

        stopwatchTime.reset();
        stopwatchTime.start();

        Boolean deepInheritance = true;
        Boolean localOnly = false;
        Boolean includeQualifiers = false;
        Boolean includeClassOrigin = false;

        for (Uint32 i = 0; i < _iterations; i++)
        {
           Array<CIMInstance> cimNInstances =
                  client.enumerateInstances(NAMESPACE, className,
                                         deepInheritance,
                                         localOnly,  includeQualifiers,
                                         includeClassOrigin );

           numberInstances = cimNInstances.size();
           if (numberInstances != expectedNumberOfInstances)
           {
              errorExit(errPrintWriter,
                  "enumerateInstances failed. "
                  "Incorrect number of instances returned.");
           }

           for (Uint32 j = 0; j < numberInstances; j++)
           {
              CIMObjectPath instanceRef = cimNInstances[j].getPath ();
              if ( !(instanceRef.getClassName().equal(className)))
              {
                 errorExit(errPrintWriter, "enumerateInstances failed. "
                    "Incorrect class name returned.");
              }

              if ( cimNInstances[j].getPropertyCount() != numberOfProperties+1)
              {
                 errorExit(errPrintWriter, "enumerateInstances failed. "
                    "Incorrect number of properties returned.");
              }

            }   // end for looping through instances
        }

        stopwatchTime.stop();

        double elapsedTime = stopwatchTime.getElapsed();

        outPrintWriter << testID
            << ": Benchmark Test #5: enumerateInstances Test on class "
            << className.getString() << endl;
        outPrintWriter << "Connect time = " << connectTime << endl;
        outPrintWriter << "Number of Non-Key Properties Returned  = "
            << numberOfProperties << endl
            << "Size of Each Non-Key Property Returned  = "
            << sizeOfPropertyValue << endl
            << "Number of Instances Returned = "
            << expectedNumberOfInstances << endl;
        outPrintWriter << _iterations << " requests processed in "
            << elapsedTime << " Seconds "
            << "(Average Elapse Time = " << elapsedTime/_iterations
            << ")" << endl << endl;

        if (_resultsFileHandle != NULL)
        {
            fprintf(_resultsFileHandle,
                "enumerateInstances\t%s\t%f\t",
                (const char *)className.getString().getCString(),
                elapsedTime/_iterations);
        }

        client.disconnect();
    }  // end try

    catch(const Exception& e)
    {
      errorExit(errPrintWriter, e.getMessage());
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
Uint32 benchmarkTestCommand::execute (ostream& outPrintWriter,
                                      ostream& errPrintWriter)
{

    benchmarkDefinition test;

    if (_resultsDirectory != String::EMPTY)
    {
        String resultsFile = _resultsDirectory;
        resultsFile.append("/benchmarkTestResults0001.txt");
        _resultsFileHandle = fopen(resultsFile.getCString(), "a+");
    }
    else
    {
        _resultsFileHandle = NULL;
    }

    if (_resultsFileHandle != NULL)
    {
        _startTime = System::getCurrentASCIITime().getCString();
        fprintf(_resultsFileHandle, "%s\t",
            (const char *)_startTime.getCString());
    }

    try
    {
        if (_debugOption1)
        {
          Tracer::setTraceFile("benchmarkTest.trc");
          Tracer::setTraceComponents("ALL");
          Tracer::setTraceLevel(Tracer::LEVEL4);
        }

        Uint32 testID = 0;

        if (_generateReport)
        {
             benchmarkTestCommand::_getSystemConfiguration(
                 outPrintWriter, errPrintWriter);
        }

        benchmarkTestCommand::_getTestConfiguration(outPrintWriter,
            errPrintWriter);

        testID++;
        if (!_testIDSet || (testID == _testID))
        {
             benchmarkTestCommand::dobenchmarkTest1(
                 testID, outPrintWriter, errPrintWriter);
        }

        testID++;
        if (!_testIDSet || (testID == _testID))
        {
             benchmarkTestCommand::dobenchmarkTest2(
                 testID, _testClassNames[0], outPrintWriter, errPrintWriter);
        }

        for (Uint32 i = 0, n = _testClassNames.size(); i < n; i++)

        {
            if ((_resultsFileHandle != NULL) && ((i+1) % 10) == 0)
            {
               fclose(_resultsFileHandle);
               String resultsFile = _resultsDirectory;
               char s[40];
               sprintf(s, "/benchmarkTestResults%.4u.txt", (i/10)+1);
               resultsFile.append(s);
               _resultsFileHandle = fopen(resultsFile.getCString(), "a+");
               if (_resultsFileHandle != NULL)
               {
                   fprintf(_resultsFileHandle, "%s\t",
                       (const char *)_startTime.getCString());
               }
            }

            testID++;
            if (!_testIDSet || (testID == _testID))
            {
                 benchmarkTestCommand::dobenchmarkTest3(testID,
                     _testClassNames[i], outPrintWriter, errPrintWriter);
            }

            testID++;
            if (!_testIDSet || (testID == _testID))
            {
                 benchmarkTestCommand::dobenchmarkTest4(testID,
                     _testClassNames[i], outPrintWriter, errPrintWriter);
            }

            testID++;
            if (!_testIDSet || (testID == _testID))
            {
                  benchmarkTestCommand::dobenchmarkTest5(testID,
                      _testClassNames[i], outPrintWriter, errPrintWriter);
            }
        }
    }
    catch (const benchmarkTestException& e)
    {
      errPrintWriter << benchmarkTestCommand::COMMAND_NAME << ": " <<
            e.getMessage () << endl;
      return (RC_ERROR);
    }

    catch (const Exception& e)
    {
      errPrintWriter << benchmarkTestCommand::COMMAND_NAME << ": " <<
            e.getMessage () << endl;
      return (RC_ERROR);
    }

    catch (const exception& e)
    {
      errPrintWriter << benchmarkTestCommand::COMMAND_NAME << ": " <<
            e.what () << endl;
      return (RC_ERROR);
     }

    if (_resultsFileHandle != NULL)
    {
        fprintf(_resultsFileHandle, "\n");
        fclose(_resultsFileHandle);
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
    benchmarkTestCommand    command = benchmarkTestCommand ();
    int                rc;

    try
    {
        command.setCommand (argc, argv);
    }
    catch (const CommandFormatException& cfe)
    {
        cerr << benchmarkTestCommand::COMMAND_NAME << ": " << cfe.getMessage ()
             << endl;
        cerr << command.getUsage () << endl;
        exit (Command::RC_ERROR);
    }

    rc = command.execute (cout, cerr);
    return (rc);
}
