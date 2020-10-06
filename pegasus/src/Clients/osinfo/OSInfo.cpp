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
#include <Pegasus/Common/FileSystem.h>
#include <Pegasus/Common/String.h>
#include <Pegasus/Common/PegasusVersion.h>
#include <Pegasus/Common/SSLContext.h>
#include <Pegasus/Common/HostAddress.h>

#include <Pegasus/getoopt/getoopt.h>
#include <Clients/cliutils/CommandException.h>
#include "OSInfo.h"

// To build a version of osinfo that does not
// support remote connections the
// DISABLE_SUPPORT_FOR_REMOTE_CONNECTIONS
// flag can be enabled.
//#define DISABLE_SUPPORT_FOR_REMOTE_CONNECTIONS

//#define DEBUG

PEGASUS_NAMESPACE_BEGIN

#define NAMESPACE CIMNamespaceName ("root/cimv2")
#define CLASSNAME CIMName ("PG_OperatingSystem")

/**
    The command name.
 */
const char   OSInfoCommand::COMMAND_NAME []      = "osinfo";

/**
    Label for the usage string for this command.
 */
const char   OSInfoCommand::_USAGE []            = "Usage: ";

/**
    The option character used to specify the hostname.
 */
const char   OSInfoCommand::_OPTION_HOSTNAME     = 'h';

/**
    The option character used to specify the port number.
 */
const char   OSInfoCommand::_OPTION_PORTNUMBER   = 'p';

/**
    The option character used to specify SSL usage.
 */
const char   OSInfoCommand::_OPTION_SSL          = 's';

/**
    The option character used to specify the timeout value.
 */
const char   OSInfoCommand::_OPTION_TIMEOUT      = 't';

/**
    The option character used to specify the username.
 */
const char   OSInfoCommand::_OPTION_USERNAME     = 'u';

/**
    The option character used to specify the password.
 */
const char   OSInfoCommand::_OPTION_PASSWORD     = 'w';

/**
    The option character used to specify the password.
 */
const char   OSInfoCommand::_OPTION_RAW_DATETIME_FORMAT     = 'c';

/**
    The minimum valid portnumber.
 */
const Uint32 OSInfoCommand::_MIN_PORTNUMBER      = 0;

/**
    The maximum valid portnumber.
 */
const Uint32 OSInfoCommand::_MAX_PORTNUMBER      = 65535;

//l10n
/**
 * The CLI message resource name
 */

static const char MSG_PATH [] = "pegasus/pegasusCLI";
static const char PASSWORD_PROMPT [] =
    "Please enter your password: ";

static const char PASSWORD_BLANK [] =
    "Password cannot be blank. Please re-enter your password.";

static const char LONG_HELP [] = "help";

static const char LONG_VERSION [] = "version";

static const char ERR_USAGE [] =
    "Use '--help' to obtain command syntax.";

static const char ERR_USAGE_KEY [] =
    "Clients.cimuser.CIMUserCommand.ERR_USAGE";

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


static const Uint32 MAX_PW_RETRIES = 3;

static Boolean verifyCertificate(SSLCertificateInfo &certInfo)
{

#ifdef DEBUG
    cout << certInfo.getSubjectName() << endl;
#endif
    //
    // If server certificate was found in trust store and validated, then
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

    Constructs a OSInfoCommand and initializes instance variables.

 */
OSInfoCommand::OSInfoCommand ()
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
    _useRawDateTimeFormat   = false;

    usage = String (_USAGE);
    usage.append (COMMAND_NAME);
    usage.append (" [ -");
#ifndef DISABLE_SUPPORT_FOR_REMOTE_CONNECTIONS
#ifdef PEGASUS_HAS_SSL
    usage.append (_OPTION_SSL);
    usage.append (" ] [ -");
#endif
    usage.append (_OPTION_HOSTNAME);
    usage.append (" hostname ] [ -");
    usage.append (_OPTION_PORTNUMBER);
    usage.append (" portnumber ] [ -");
    usage.append (_OPTION_USERNAME);
    usage.append (" username ]\n              [ -");
    usage.append (_OPTION_PASSWORD);
    usage.append (" password ] [ -");
    usage.append (_OPTION_TIMEOUT);
    usage.append (" timeout ] [ -");
#endif
    usage.append (_OPTION_RAW_DATETIME_FORMAT);
    usage.append (" ]");
    usage.append (" [ --");
    usage.append (LONG_HELP);
    usage.append(" ] [ --").append(LONG_VERSION).append(" ] \n");

    usage.append("Options : \n");
    usage.append("    -c         - Use CIM format for date and time\n");
    usage.append("    -h         - Connect to CIM Server on specified"
                                    " hostname\n");
    usage.append("    --help     - Display this help message\n");
    usage.append("    -p         - Connect to CIM Server on specified"
                                    " portnumber\n");
#ifdef PEGASUS_HAS_SSL
    usage.append("    -s         - Use SSL protocol between 'osinfo' client"
                                    " and the CIM Server\n");
#endif
    usage.append("    -t         - Specify response timeout value in"
                                    " milliseconds\n");
    usage.append("    -u         - Connect to CIM Server using the specified"
                                    " username\n");
    usage.append("    --version  - Display CIM Server version number\n");
    usage.append("    -w         - Connect to CIM Server using the specified"
                                    " password\n");

    usage.append("\nUsage note: The osinfo command requires that the"
                    " CIM Server is running.\n");

    setUsage (usage);
}

String OSInfoCommand::_promptForPassword( ostream& outPrintWriter )
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
            if (retries < MAX_PW_RETRIES)
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

    Connects to cimserver.

    @param   outPrintWriter     the ostream to which error output should be
                                written

    @exception       Exception  if an error is encountered in creating
                               the connection

 */
 void OSInfoCommand::_connectToServer( CIMClient& client,
                                       ostream& outPrintWriter )
{
    String                 host                  = String ();
    Uint32                 portNumber            = 0;
    Boolean                connectToLocal        = false;

    //
    //  Construct host address
    //

    if ((!_hostNameSet) && (!_portNumberSet)
            && (!_userNameSet) && (!_passwordSet))
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
               PEGASUS_UNREACHABLE(PEGASUS_ASSERT(false);)
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
    else
    {
        if (!_userNameSet)
        {
           _userName = System::getEffectiveUserName();
        }

        if (!_passwordSet)
        {
            _password = _promptForPassword( outPrintWriter );
        }
        if( _useSSL )
        {
#ifdef PEGASUS_HAS_SSL
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
           SSLContext  sslcontext (certpath, verifyCertificate, randFile);

           client.connect(host, portNumber, sslcontext,  _userName, _password );
#else
           PEGASUS_UNREACHABLE(PEGASUS_ASSERT(false);)
#endif
        }
        else
        {
           client.connect(host, portNumber, _userName, _password );
        }
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
void OSInfoCommand::setCommand (Uint32 argc, char* argv [])
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
#ifndef DISABLE_SUPPORT_FOR_REMOTE_CONNECTIONS
    GetOptString.append (_OPTION_HOSTNAME);
    GetOptString.append (getoopt::GETOPT_ARGUMENT_DESIGNATOR);
    GetOptString.append (_OPTION_PORTNUMBER);
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
#endif
    GetOptString.append (_OPTION_RAW_DATETIME_FORMAT);

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
                    //
                    // More than one operation option was found
                    //
                    throw UnexpectedOptionException(String(LONG_VERSION));
                }

               _operationType = OPERATION_TYPE_VERSION;
            }
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
                    HostAddress addr;
                    if (!addr.setHostAddress(_hostName))
                    {
                        throw InvalidLocatorException (_hostName);
                    }
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

                case _OPTION_RAW_DATETIME_FORMAT:
                {
                    //
                    // Display "raw" CIM_DateTime format.
                    //
                    _useRawDateTimeFormat = true;
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
                    //  PEP#167 unless an empty '-' is specified
                    //_operationType = OPERATION_TYPE_UNINITIALIZED;
                    break;
            }
        }
    }

    //
    // Some more validations
    //
    /*if ( _operationType == OPERATION_TYPE_UNINITIALIZED )
    {
        //
        // No operation type was specified
        // Show the usage
        //
        throw CommandFormatException(localizeMessage(
            MSG_PATH,
            REQUIRED_ARGS_MISSING_KEY,
            REQUIRED_ARGS_MISSING));
    }*/
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
}

/** ErrorExit - Print out the error message and exits.
    @param   errPrintWriter     The ostream to which error output should be
                                written
    @param   message            Text for error message
    @return - None, Terminates the program
    @exception - This function terminates the program
*/
void OSInfoCommand::errorExit( ostream& errPrintWriter,
                               const String& message)
{
    errPrintWriter << "osinfo error: " << message << endl;
    exit(1);
}


/**
   formatCIMDateTime method takes a string with CIM formatted
   DateTime and returns a user-readable string of the format
   month day-of-month, year  hour:minute:second (value-hrs-GMT-offset)
   */
static void formatCIMDateTime (const char* cimString, char* dateTime)
{
   int year = 0;
   int month = 0;
   int day = 0;
   int hour = 0;
   int minute = 0;
   int second = 0;
   int microsecond = 0;
   int timeZone = 0;
   sscanf(cimString, "%04d%02d%02d%02d%02d%02d.%06d%04d",
          &year, &month, &day, &hour, &minute, &second,
          &microsecond, &timeZone);
   char monthString[5];
   switch (month)
   {
      case 1 : { sprintf(monthString, "Jan"); break; }
      case 2 : { sprintf(monthString, "Feb"); break; }
      case 3 : { sprintf(monthString, "Mar"); break; }
      case 4 : { sprintf(monthString, "Apr"); break; }
      case 5 : { sprintf(monthString, "May"); break; }
      case 6 : { sprintf(monthString, "Jun"); break; }
      case 7 : { sprintf(monthString, "Jul"); break; }
      case 8 : { sprintf(monthString, "Aug"); break; }
      case 9 : { sprintf(monthString, "Sep"); break; }
      case 10 : { sprintf(monthString, "Oct"); break; }
      case 11 : { sprintf(monthString, "Nov"); break; }
      case 12 : { sprintf(monthString, "Dec"); break; }
      // covered all known cases, if get to default, just
      // return the input string as received.
      default : { strcpy(dateTime, cimString); return; }
   }

   sprintf(dateTime, "%s %d, %d  %d:%02d:%02d (%+03d%02d)",
           monthString, day, year, hour, minute, second,
           timeZone/60, timeZone%60);
}


/**
   gatherProperties method of the osinfo Test Client
*/

void OSInfoCommand::gatherProperties(CIMInstance &inst, Boolean cimFormat)
{
   // don't have a try here - want it to be caught by caller

   // loop through the properties
   for (Uint32 j=0; j < inst.getPropertyCount(); j++)
   {
      CIMName propertyName = inst.getProperty(j).getName();

      // only pull out those properties of interest
      if (propertyName.equal (CIMName ("CSName")))
      {
         inst.getProperty(j).getValue().get(osCSName);
      }  // end if CSName

      else if (propertyName.equal (CIMName ("Name")))
      {
         inst.getProperty(j).getValue().get(osName);
      }  // end if Name

      else if (propertyName.equal (CIMName ("NumberOfProcesses")))
      {
         Uint32 propertyValue;
         inst.getProperty(j).getValue().get(propertyValue);
         char tmpString[80];
         sprintf(tmpString, "%u processes", propertyValue);
         osNumberOfProcesses.assign(tmpString);
      }  // end if NumberOfProcesses

      else if (propertyName.equal (CIMName ("NumberOfUsers")))
      {
         Uint32 propertyValue;
         inst.getProperty(j).getValue().get(propertyValue);
         char tmpString[80];
         sprintf(tmpString, "%u users", propertyValue);
         osNumberOfUsers.assign(tmpString);
      }  // end if NumberOfUsers

      if (propertyName.equal (CIMName ("Version")))
      {
         inst.getProperty(j).getValue().get(osVersion);
      }  // end if Version

      else if (propertyName.equal (CIMName ("OperatingSystemCapability")))
      {
         inst.getProperty(j).getValue().get(osCapability);
      }   // end if OSCapability

      else if (propertyName.equal (CIMName ("OtherTypeDescription")))
      {
         inst.getProperty(j).getValue().get(osOtherInfo);
      }   // end if OtherTypeDescription
      else if (propertyName.equal (CIMName ("NumberOfLicensedUsers")))
      {
         Uint32 propertyValue;
         inst.getProperty(j).getValue().get(propertyValue);
         // special consideration for HP-UX
         if (propertyValue == 0)
         {
            if (String::equalNoCase(osVersion,"HP-UX"))
            {
               osLicensedUsers.assign("128, 256, or unlimited users");
            }
            else
            {
               osLicensedUsers.assign("Unlimited user license");
            }
         }  // end if 0 as number of licensed users
         else  // standard number of users
         {
            char users[80];
            sprintf(users, "%u users", propertyValue);
            osLicensedUsers.assign(users);
         }
      }   // end if NumberOfLicensedUsers

      else if (propertyName.equal (CIMName ("LastBootUpTime")))
      {
         CIMDateTime bdate;
         char bdateString[80];

         inst.getProperty(j).getValue().get(bdate);
         CString dtStr = bdate.toString().getCString();
         if (!cimFormat)
         { // else leave in raw CIM
            formatCIMDateTime(dtStr, bdateString);
         }
         else
         {
            sprintf(bdateString,"%s",(const char*)dtStr);
         }
         osBootUpTime.assign(bdateString);
      }   // end if LastBootUpTime

      else if (propertyName.equal (CIMName ("LocalDateTime")))
      {
         CIMDateTime ldate;
         char ldateString[80];

         inst.getProperty(j).getValue().get(ldate);
         CString dtStr = ldate.toString().getCString();
         if (!cimFormat)
         { // else leave in raw CIM
            formatCIMDateTime(dtStr, ldateString);
         }
         else
         {
            sprintf(ldateString,"%s",(const char*)dtStr);
         }
         osLocalDateTime.assign(ldateString);
      }   // end if LocalDateTime

      else if (propertyName.equal (CIMName ("SystemUpTime")))
      {
         Uint64 total;
         char   uptime[80];
         inst.getProperty(j).getValue().get(total);

         if (!cimFormat)
         { // else leave in raw CIM
            // let's make things a bit easier for our user to read
            Uint64 days = 0;
            Uint32 hours = 0;
            Uint32 minutes = 0;
            Uint32 seconds = 0;
            Uint64 totalSeconds = total;
            seconds = Uint32(total%60);
            total = total/60;
            minutes = Uint32(total%60);
            total = total/60;
            hours = Uint32(total%24);
            total = total/24;
            days = total;

            // now deal with the proper singular/plural
            char dayString[20];
            char hourString[20];
            char minuteString[20];
            char secondString[20];

            if(!days)
            {
                sprintf(dayString," " );
            }
            else
            {
                sprintf(dayString, (days == 1 ? 
                    "%" PEGASUS_64BIT_CONVERSION_WIDTH "u day," :
                    "%" PEGASUS_64BIT_CONVERSION_WIDTH "u days," ), days);

            }

            // for other values, want to display the 0s
            sprintf(hourString, (hours == 1 ? "%u hr," : "%u hrs,"), hours);

            sprintf(minuteString,
                (minutes == 1 ? "%u min," : "%u mins,"), minutes);

            sprintf(secondString,
                (seconds == 1 ? "%u sec" : "%u secs"), seconds);

            sprintf(uptime,
                "%" PEGASUS_64BIT_CONVERSION_WIDTH "u seconds = %s %s %s %s",
                totalSeconds,
                dayString,
                hourString,
                minuteString,
                secondString);
            osSystemUpTime.assign(uptime);
         }  // end of if wanted nicely formatted vs. raw CIM
         else
         {
            sprintf(uptime, "%" PEGASUS_64BIT_CONVERSION_WIDTH "u", total);
         }

         osSystemUpTime.assign(uptime);

      }   // end if SystemUpTime

   }  // end of for looping through properties
}



/**
   displayProperties method of the osinfo Test Client
  */
void OSInfoCommand::displayProperties(ostream& outPrintWriter)
{
   // interesting properties are stored off in class variables

   outPrintWriter << "OperatingSystem Information" << endl;

   // expect to have values for the keys (even if Unknown)
   outPrintWriter << "  Host: " << osCSName << endl;
   outPrintWriter << "  Name: " << osName << endl;

   // on Linux, the OtherTypeDescription field had distribution info
   // wrote to display this info whenever it's present (any OS)
   if (osOtherInfo != String::EMPTY)
   {
      // put in parens after the name
      outPrintWriter << "   ( " << osOtherInfo << " ) " << endl;
   }

   if (osVersion != String::EMPTY)
      outPrintWriter << "  Version: " << osVersion << endl;
   else
      outPrintWriter << "  Version: Unknown" << endl;

   if (osLicensedUsers != String::EMPTY)
      outPrintWriter << "  UserLicense: " << osLicensedUsers << endl;
   else
      outPrintWriter << "  UserLicense: Unknown" << endl;

   if (osNumberOfUsers != String::EMPTY)
      outPrintWriter << "  Number of Users: " << osNumberOfUsers << endl;
   else
      outPrintWriter << "  Number of Users: Unknown" << endl;

   if (osNumberOfProcesses != String::EMPTY)
      outPrintWriter << "  Number of Processes: " << osNumberOfProcesses
                     << endl;
   else
      outPrintWriter << "  Number of Processes: Unknown" << endl;

   if (osCapability != String::EMPTY)
      outPrintWriter << "  OSCapability: " << osCapability << endl;
   else
      outPrintWriter << "  OSCapability: Unknown" << endl;

   if (osBootUpTime != String::EMPTY)
      outPrintWriter << "  LastBootTime: " << osBootUpTime << endl;
   else
      outPrintWriter << "  LastBootTime: Unknown" << endl;

   if (osLocalDateTime != String::EMPTY)
      outPrintWriter << "  LocalDateTime: " << osLocalDateTime << endl;
   else
      outPrintWriter << "  LocalDateTime: Unknown" << endl;

   if (osSystemUpTime != String::EMPTY)
      outPrintWriter << "  SystemUpTime: " << osSystemUpTime << endl;
   else
      outPrintWriter << "  SystemUpTime: Unknown" << endl;
}




void OSInfoCommand::getOSInfo(ostream& outPrintWriter,
                              ostream& errPrintWriter)
{

    CIMClient client;
    client.setTimeout( _timeout );

    try
    {
        _connectToServer( client, outPrintWriter);

        Boolean deepInheritance = true;
        Boolean localOnly = false;
        Boolean includeQualifiers = false;
        Boolean includeClassOrigin = false;
        Uint32 numberInstances;
        Array<CIMInstance> cimNInstances =
               client.enumerateInstances(NAMESPACE, CLASSNAME,
                                         deepInheritance,
                                         localOnly,  includeQualifiers,
                                         includeClassOrigin );

        numberInstances = cimNInstances.size();

        // while we only have one instance (the running OS), we can take the
        // first instance.  When the OSProvider supports installed OSs as well,
        // will need to select the runningOS instance

        for (Uint32 i = 0; i < cimNInstances.size(); i++)
        {
           CIMObjectPath instanceRef = cimNInstances[i].getPath ();
           if ( !(instanceRef.getClassName().equal (CIMName (CLASSNAME))))
           {
              errorExit(errPrintWriter, "EnumerateInstances failed");
           }

           // first gather the interesting properties
           gatherProperties(cimNInstances[i], _useRawDateTimeFormat);
           // then display them
           displayProperties(outPrintWriter);

      }   // end for looping through instances

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
Uint32 OSInfoCommand::execute (ostream& outPrintWriter,
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
        OSInfoCommand::getOSInfo( outPrintWriter, errPrintWriter );
    }
    catch (const OSInfoException& e)
    {
        errPrintWriter << OSInfoCommand::COMMAND_NAME << ": " <<
            e.getMessage () << endl;
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
    OSInfoCommand    command = OSInfoCommand ();
    int                rc;

    MessageLoader::setPegasusMsgHomeRelative(argv[0]);
    try
    {
        command.setCommand (argc, argv);
    }
    catch (const CommandFormatException& cfe)
    {
        cerr << OSInfoCommand::COMMAND_NAME << ": " << cfe.getMessage() << endl;

        MessageLoaderParms parms(ERR_USAGE_KEY,ERR_USAGE);
        parms.msg_src_path = MSG_PATH;
        cerr << OSInfoCommand::COMMAND_NAME <<
            ": " << MessageLoader::getMessage(parms) << endl;

        exit (Command::RC_ERROR);
    }
    catch (const InvalidLocatorException &ile)
    {
        cerr << OSInfoCommand::COMMAND_NAME << ": " << ile.getMessage() << endl;
        exit (Command::RC_ERROR);
    }

    rc = command.execute (cout, cerr);
    exit (rc);
    return 0;
}
