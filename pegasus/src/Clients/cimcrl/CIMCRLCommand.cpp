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
#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/Constants.h>
#include <Pegasus/Common/System.h>
#include <Pegasus/Common/FileSystem.h>
#include <Pegasus/Common/String.h>
#include <Pegasus/Common/InternalException.h>
#include <Pegasus/Common/PegasusVersion.h>

#include <Pegasus/getoopt/getoopt.h>
#include "CIMCRLCommand.h"

PEGASUS_NAMESPACE_BEGIN

/**
 * The CLI message resource name
 */
static const char MSG_PATH []                  = "pegasus/pegasusCLI";

/**
    The command name.
 */
const char   CIMCRLCommand::COMMAND_NAME []      = "cimcrl";

/**
    Label for the usage string for this command.
 */
const char   CIMCRLCommand::_USAGE []            = "Usage: ";

/**
    The option character used to specify add CRL.
*/
const char   CIMCRLCommand::_OPTION_ADD          = 'a';

/**
    The option character used to specify remove CRL.
*/
const char   CIMCRLCommand::_OPTION_REMOVE       = 'r';

/**
    The option character used to specify list CRLs.
*/
const char   CIMCRLCommand::_OPTION_LIST         = 'l';

/**
    The option character used to specify a CRL file path.
 */
const char   CIMCRLCommand::_OPTION_CRLFILE     = 'f';

/**
    The option character used to specify the issuer name.
 */
const char   CIMCRLCommand::_OPTION_ISSUERNAME   = 'i';

static const char   LONG_HELP []  = "help";

static const char   LONG_VERSION []  = "version";

/**
    This constant signifies that an operation option has not
    been recorded
*/
const Uint32 CIMCRLCommand::_OPERATION_TYPE_UNINITIALIZED = 0;

/**
    The constant representing an add operation
*/
const Uint32 CIMCRLCommand::_OPERATION_TYPE_ADD = 1;

/**
    The constant representing a remove operation
*/
const Uint32 CIMCRLCommand::_OPERATION_TYPE_REMOVE = 2;

/**
    The constant representing a list operation
*/
const Uint32 CIMCRLCommand::_OPERATION_TYPE_LIST = 3;

/**
    The constant representing a help operation
*/
const Uint32 CIMCRLCommand::_OPERATION_TYPE_HELP = 4;

/**
    The constant representing a version display operation
*/
const Uint32 CIMCRLCommand::_OPERATION_TYPE_VERSION = 5;

/**
    This constant represents the name of the Issuer name
    property in the schema
*/
static const CIMName PROPERTY_NAME_ISSUERNAME = CIMName ("IssuerName");

/**
    This constant represents the name of the lastUpdate
    property in the schema
*/
static const CIMName PROPERTY_NAME_LASTUPDATE   = CIMName ("LastUpdate");

/**
    This constant represents the name of the nextUpdate
    property in the schema
*/
static const CIMName PROPERTY_NAME_NEXTUPDATE   = CIMName ("NextUpdate");

/**
    This constant represents the name of the revokedSerialNumbers
    property in the schema
*/
static const CIMName PROPERTY_NAME_REVOKED_SERIAL_NUMBERS =
                                      CIMName ("RevokedSerialNumbers");

/**
    This constant represents the name of the revocationDates
    property in the schema
*/
static const CIMName PROPERTY_NAME_REVOCATION_DATES =
                                      CIMName ("RevocationDates");

/**
    The name of the Method that implements add revoked certificate to
    the CRL store.
*/
static const CIMName ADD_CRL_METHOD = CIMName ("addCertificateRevocationList");

/**
    The input parameter name for CRL content.
*/
static const char   CRL_CONTENTS []     = "CRLContents";

/**
    The constants representing the display messages.
*/
static const char REQUIRED_ARGS_MISSING [] =
                "Required arguments missing.";

static const char REQUIRED_ARGS_MISSING_KEY [] =
                "Clients.cimcrl.CIMCRLCommand.REQUIRED_ARGS_MISSING";

static const char CANNOT_CONNECT_CIMSERVER_NOT_RUNNING [] =
                "Cannot connect to CIM Server, CIM Server may not be running.";

static const char CANNOT_CONNECT_CIMSERVER_NOT_RUNNING_KEY [] =
            "Clients.cimcrl.CIMCRLCommand.CANNOT_CONNECT_CIMSERVER_NOT_RUNNING";

static const char CONNECTION_TIMEOUT [] =
                "Connection timed out.";

static const char CONNECTION_TIMEOUT_KEY [] =
                "Clients.cimcrl.CIMCRLCommand.CONNECTION_TIMEOUT";

static const char ADD_CRL_SUCCESS [] =
                "CRL added successfully.";

static const char ADD_CRL_SUCCESS_KEY [] =
                "Clients.cimcrl.CIMCRLCommand.ADD_CRL_SUCCESS";

static const char REMOVE_CRL_SUCCESS [] =
                "CRL removed successfully.";

static const char REMOVE_CRL_SUCCESS_KEY [] =
                "Clients.cimcrl.CIMCRLCommand.REMOVE_CRL_SUCCESS";

static const char CRL_NOT_FOUND [] =
                "Specified CRL does not exist.";

static const char CRL_NOT_FOUND_KEY [] =
                "Clients.cimcrl.CIMCRLCommand.CRL_NOT_FOUND";

static const char CRL_SCHEMA_NOT_LOADED []  =
                "Please restore the internal repository on the CIM Server.";

static const char CRL_SCHEMA_NOT_LOADED_KEY []  =
                "Clients.cimcrl.CIMCRLCommand.CRL_SCHEMA_NOT_LOADED";

// Return codes

static const Uint32 RC_CONNECTION_FAILED = 2;

static const Uint32 RC_CONNECTION_TIMEOUT = 3;

static const Uint32 RC_CRL_DOES_NOT_EXIST = 4;

/**

    Constructs a CIMCRLCommand and initializes instance variables.

 */
CIMCRLCommand::CIMCRLCommand ()
{
    //
    // Initialize the instance variables.
    //
    _operationType       = _OPERATION_TYPE_UNINITIALIZED;

    _crlFileSet          = false;
    _issuerSet           = false;;

    _timeout             = DEFAULT_TIMEOUT_MILLISECONDS;

    //
    // build Usage string.
    //
    _usage = String (_USAGE);
    _usage.append (COMMAND_NAME);

    //
    // Add option
    //
    _usage.append (" -").append (_OPTION_ADD);
    _usage.append (" -").append (_OPTION_CRLFILE).append (" crlfile");
    _usage.append ("\n");

    //
    // Remove option
    //
    //_usage.append ("                   -").append (_OPTION_REMOVE);
    _usage.append("       ").append (COMMAND_NAME);
    _usage.append (" -").append (_OPTION_REMOVE);
    _usage.append (" -").append (_OPTION_ISSUERNAME).append (" issuername");
    _usage.append ("\n");

    //
    // List option
    //
    //_usage.append ("                   -").append (_OPTION_LIST);
    _usage.append("       ").append (COMMAND_NAME);
    _usage.append (" -").append (_OPTION_LIST);
    _usage.append (" [ -").append (_OPTION_ISSUERNAME);
    _usage.append (" issuername").append (" ]");
    _usage.append ("\n");

    //
    // Version options
    //
    _usage.append("       ").append (COMMAND_NAME);
    _usage.append (" --").append (LONG_VERSION)
         .append("\n");

    //
    // Help option
    //
    _usage.append("       ").append (COMMAND_NAME);
    _usage.append (" --").append (LONG_HELP)
         .append("\n");

    //
    // Options description
    //
    _usage.append("Options : \n");
    _usage.append("    -a              ");
    _usage.append("- Adds the specified crl to the CRL store\n");

    _usage.append("    -r              ");
    _usage.append("- Removes the specified crl from the CRL store\n");

    _usage.append("    -l              ");
    _usage.append("- Displays the crls in the CRL store\n");

    _usage.append("    -f crlfile      ");
    _usage.append("- Specifies the PEM format file containing ");
    _usage.append("an X509 CRL\n");

    _usage.append("    -i issuername   ");
    _usage.append("- Specifies the CRL issuer name\n");

    _usage.append("    --help          - Displays this help message\n");
    _usage.append("    --version       - Displays CIM Server version number\n");

    _usage.append("\nUsage note: The cimcrl command requires that ");
    _usage.append("the CIM Server is running.\n");

#ifdef PEGASUS_HAS_ICU
    MessageLoaderParms menuparms(
        "Clients.cimcrl.CIMCRLCommand.MENU.STANDARD", _usage);
    menuparms.msg_src_path = MSG_PATH;
    _usage = MessageLoader::getMessage(menuparms);
#endif

    setUsage (_usage);
}

/**
    Convert CIMDateTime to user-readable string of the format
    month day-of-month, year  hour:minute:second (value-hrs-GMT-offset)

    @param  cimDateTimeStr  CIM formated DateTime String

    @return  String user-readable date time string.
 */
String CIMCRLCommand::_formatCIMDateTime(const String& cimDateTimeStr)
{
    Uint32 year = 0;
    Uint32 month = 0;
    Uint32 day = 0;
    Uint32 hour = 0;
    Uint32 minute = 0;
    Uint32 second = 0;
    Uint32 microsecond = 0;
    Sint32 timezone = 0;

    sscanf(cimDateTimeStr.getCString(), "%04u%02u%02u%02u%02u%02u.%06u%04d",
           &year, &month, &day, &hour, &minute, &second,
           &microsecond, &timezone);

    char monthString[5];

    switch (month)
    {
       case 1 :
           sprintf(monthString, "Jan");
           break;
       case 2 :
           sprintf(monthString, "Feb");
           break;
       case 3 :
           sprintf(monthString, "Mar");
           break;
      case 4 :
          sprintf(monthString, "Apr");
          break;
      case 5 :
          sprintf(monthString, "May");
          break;
      case 6 :
          sprintf(monthString, "Jun");
          break;
      case 7 :
          sprintf(monthString, "Jul");
          break;
      case 8 :
          sprintf(monthString, "Aug");
          break;
      case 9 :
          sprintf(monthString, "Sep");
          break;
      case 10 :
          sprintf(monthString, "Oct");
          break;
      case 11 :
          sprintf(monthString, "Nov");
          break;
      case 12 :
          sprintf(monthString, "Dec");
          break;
      default:
          PEGASUS_UNREACHABLE(PEGASUS_ASSERT(false);)

      return (cimDateTimeStr);
   }

   char dateTimeStr[80];
   sprintf(dateTimeStr, "%s %u, %u  %u:%02u:%02u (%+03d%02u)",
           monthString, day, year, hour, minute, second,
           timezone/60, timezone%60);

   return String(dateTimeStr);
}


/**
    Read CRL content.

    @param  crlFilePath     the CRL file path

    @return  Buffer containing the CRL content

    @exception  Exception    if an error is encountered
                             in reading the CRL file
 */
Buffer CIMCRLCommand::_readCRLContent(const String &certFilePath)
{
    Buffer content;

    //
    //  Check that cert file exists
    //
    if (!FileSystem::exists (certFilePath))
    {
        throw NoSuchFile(certFilePath);
    }

    //
    //  Load file context to memory
    //
    FileSystem::loadFileToMemory (content, certFilePath);

    return content;
}


/**
   Add a CRL to the CRL store

   @param  client           the handle to CIMClient object

   @param  outPrintWriter   the ostream to which output should be written
 */
void CIMCRLCommand::_addCRL(
    CIMClient&   client,
    ostream&     outPrintWriter)
{
    Buffer              content;
    Array<CIMKeyBinding>   kbArray;
    CIMKeyBinding          kb;
    Array<CIMParamValue>   inParams;
    Array<CIMParamValue>   outParams;

    content = _readCRLContent(_crlFile);

    String contentStr = String(content.getData());

    //
    // Build the input params
    //
    inParams.append ( CIMParamValue ( CRL_CONTENTS,
                                      CIMValue (contentStr)));

    CIMObjectPath reference(
        String::EMPTY, PEGASUS_NAMESPACENAME_CERTIFICATE,
        PEGASUS_CLASSNAME_CRL, kbArray);

    //
    // Call the invokeMethod with the input parameters
    //

    client.invokeMethod (
                   PEGASUS_NAMESPACENAME_CERTIFICATE,
                   reference,
                   ADD_CRL_METHOD,
                   inParams,
                   outParams );

    outPrintWriter << localizeMessage(MSG_PATH, ADD_CRL_SUCCESS_KEY,
        ADD_CRL_SUCCESS) << endl;
}

/**
   Remove an existing CRL from the CRL store

   @param  client           the handle to CIMClient object

   @param  outPrintWriter   the ostream to which output should be written
*/
void CIMCRLCommand::_removeCRL (
    CIMClient&   client,
    ostream&     outPrintWriter)
{
    Array<CIMKeyBinding> kbArray;
    CIMKeyBinding        kb;

    //
    // Build the input params
    //
    kb.setName(PROPERTY_NAME_ISSUERNAME);
    kb.setValue(_issuer);
    kb.setType(CIMKeyBinding::STRING);

    kbArray.append(kb);

    CIMObjectPath reference(
        String::EMPTY, PEGASUS_NAMESPACENAME_CERTIFICATE,
        PEGASUS_CLASSNAME_CRL, kbArray);

    client.deleteInstance(
        PEGASUS_NAMESPACENAME_CERTIFICATE,
        reference);

    outPrintWriter << localizeMessage(MSG_PATH, REMOVE_CRL_SUCCESS_KEY,
        REMOVE_CRL_SUCCESS) << endl;
}

/**
   List CRL's in the CRL store

   @param  client           the handle to CIMClient object

   @param  outPrintWriter   the ostream to which output should be written
 */
void CIMCRLCommand::_listCRL (
    CIMClient&   client,
    ostream&     outPrintWriter)
{
    Array<CIMInstance> crlNamedInstances;
    Boolean issuerFound = false;

    //
    // get all the instances of class PG_SSLCertificateRevocationList
    //
    crlNamedInstances =
        client.enumerateInstances(
            PEGASUS_NAMESPACENAME_CERTIFICATE,
            PEGASUS_CLASSNAME_CRL);

    //
    // copy the CRL content
    //
    Uint32 numberInstances = crlNamedInstances.size();

    for (Uint32 i = 0; i < numberInstances; i++)
    {
        CIMInstance& crlInstance = crlNamedInstances[i];

        String issuer;
        CIMDateTime lastUpdate;
        CIMDateTime nextUpdate;

        //
        // Check if issuer name is specified
        //
        Uint32 pos = crlInstance.findProperty(PROPERTY_NAME_ISSUERNAME);
        CIMConstProperty prop = crlInstance.getProperty(pos);
        prop.getValue().get(issuer);

        if ( _issuerSet && !String::equal(_issuer, issuer) )
        {
            continue;
        }
        else
        {
            issuerFound = true;
        }

        //
        // Get the remaining properties and display them.
        //
        pos = crlInstance.findProperty(PROPERTY_NAME_LASTUPDATE);
        prop = crlInstance.getProperty(pos);
        prop.getValue().get(lastUpdate);

        String lastUpdateStr = _formatCIMDateTime(lastUpdate.toString());

        pos = crlInstance.findProperty(PROPERTY_NAME_NEXTUPDATE);
        prop = crlInstance.getProperty(pos);
        prop.getValue().get(nextUpdate);

        String nextUpdateStr = _formatCIMDateTime(nextUpdate.toString());

        //
        // Display the CRL issuer name and update dates
        //
        outPrintWriter << "Issuer:  " << issuer << endl;
        outPrintWriter << "Last update: " << lastUpdateStr << endl;
        outPrintWriter << "Next update: " << nextUpdateStr << endl;

        Array<String> revokedSerialNumbers;
        Array<CIMDateTime> revocationDates;

        pos = crlInstance.findProperty(PROPERTY_NAME_REVOKED_SERIAL_NUMBERS);
        prop = crlInstance.getProperty(pos);
        prop.getValue().get(revokedSerialNumbers);

        pos = crlInstance.findProperty(PROPERTY_NAME_REVOCATION_DATES);
        prop = crlInstance.getProperty(pos);
        prop.getValue().get(revocationDates);

        outPrintWriter << "Revoked Certificates:" << endl;

        for (Uint32 j = 0; j < revokedSerialNumbers.size(); j++)
        {
            String revocationDateStr =
                _formatCIMDateTime(revocationDates[j].toString());

            //
            // Display the revoked serial numbers and the revocation dates
            //
            outPrintWriter <<
                "    Serial Number: " << revokedSerialNumbers[j] << endl;
            outPrintWriter <<
                "        Revocation Date: " << revocationDateStr << endl;
            outPrintWriter << endl;
        }
        outPrintWriter <<
            "---------------------------------------------"<< endl;
    }

    if ( _issuerSet && !issuerFound)
    {
         CIMException ce( CIM_ERR_NOT_FOUND );
         throw ce;
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
void CIMCRLCommand::setCommand (Uint32 argc, char* argv [])
{
    Uint32         i               = 0;
    Uint32         c               = 0;
    String         timeoutStr;
    String         serialNumberStr;
    String         optString;
    getoopt        getOpts;

    //
    //  Construct optString
    //
    optString.append (_OPTION_ADD);
    optString.append (getoopt::NOARG);
    optString.append (_OPTION_CRLFILE);
    optString.append (getoopt::GETOPT_ARGUMENT_DESIGNATOR);

    optString.append (_OPTION_REMOVE);
    optString.append (getoopt::NOARG);
    optString.append (_OPTION_ISSUERNAME);
    optString.append (getoopt::GETOPT_ARGUMENT_DESIGNATOR);

    optString.append (_OPTION_LIST);
    optString.append (getoopt::NOARG);
    optString.append (_OPTION_ISSUERNAME);
    optString.append (getoopt::GETOPT_ARGUMENT_DESIGNATOR);

    //
    //  Initialize and parse getOpts
    //
    getOpts = getoopt ();
    getOpts.addFlagspec (optString);

    //
    // Add long flag options for 'help' and 'version'
    //
    getOpts.addLongFlagspec (LONG_HELP, getoopt::NOARG);
    getOpts.addLongFlagspec (LONG_VERSION, getoopt::NOARG);

    getOpts.parse (argc, argv);

    if (getOpts.hasErrors ())
    {
        throw CommandFormatException(getOpts.getErrorStrings()[0]);
    }
    _operationType = _OPERATION_TYPE_UNINITIALIZED;

    //
    //  Get options and arguments from the command line
    //
    for (i =  getOpts.first (); i <  getOpts.last (); i++)
    {
        if (getOpts [i].getType () == Optarg::LONGFLAG)
        {
            if (getOpts [i].getopt () == LONG_HELP)
            {
                if (_operationType != _OPERATION_TYPE_UNINITIALIZED)
                {
                    //
                    // More than one operation option was found
                    //
                    throw UnexpectedOptionException(String(LONG_HELP));
                }

               _operationType = _OPERATION_TYPE_HELP;
            }
            else if (getOpts [i].getopt () == LONG_VERSION)
            {
                if (_operationType != _OPERATION_TYPE_UNINITIALIZED)
                {
                    //
                    // More than one operation option was found
                    //
                    throw UnexpectedOptionException(String(LONG_VERSION));
                }

               _operationType = _OPERATION_TYPE_VERSION;
            }
        }
        else if (getOpts [i].getType () == Optarg::REGULAR)
        {
            //
            //  The cimcrl command has no non-option argument options
            //
            throw UnexpectedArgumentException(getOpts[i].Value());
        }
        else /* getOpts [i].getType () == FLAG */
        {
            c = getOpts [i].getopt () [0];

            switch (c)
            {
                case _OPTION_ADD:
                {
                    if (_operationType != _OPERATION_TYPE_UNINITIALIZED)
                    {
                        //
                        // More than one operation option was found
                        //
                        throw UnexpectedOptionException(_OPTION_ADD);
                    }

                    if (getOpts.isSet (_OPTION_ADD) > 1)
                    {
                        //
                        // More than one add option was found
                        //
                        throw DuplicateOptionException(_OPTION_ADD);
                    }

                    _operationType = _OPERATION_TYPE_ADD;
                    break;
                }

                case _OPTION_REMOVE:
                {
                    if (_operationType != _OPERATION_TYPE_UNINITIALIZED)
                    {
                        //
                        // More than one operation option was found
                        //
                        throw UnexpectedOptionException(_OPTION_REMOVE);
                    }

                    if (getOpts.isSet (_OPTION_REMOVE) > 1)
                    {
                        //
                        // More than one remove option was found
                        //
                        throw DuplicateOptionException(_OPTION_REMOVE);
                    }

                    _operationType = _OPERATION_TYPE_REMOVE;
                    break;
                }

                case _OPTION_LIST:
                {
                    if (_operationType != _OPERATION_TYPE_UNINITIALIZED)
                    {
                        //
                        // More than one operation option was found
                        //
                        throw UnexpectedOptionException(_OPTION_LIST);
                    }

                    if (getOpts.isSet (_OPTION_LIST) > 1)
                    {
                        //
                        // More than one list option was found
                        //
                        throw DuplicateOptionException(_OPTION_LIST);
                    }

                    _operationType = _OPERATION_TYPE_LIST;
                    break;
                }

                case _OPTION_CRLFILE:
                {
                    if (getOpts.isSet (_OPTION_CRLFILE) > 1)
                    {
                        //
                        // More than one cert file option was found
                        //
                        throw DuplicateOptionException(_OPTION_CRLFILE);
                    }

                    _crlFile = getOpts [i].Value ();
                    _crlFileSet = true;
                    break;
                }

                case _OPTION_ISSUERNAME:
                {
                    if (getOpts.isSet (_OPTION_ISSUERNAME) > 1)
                    {
                        //
                        // More than one issuer name option was found
                        //
                        throw DuplicateOptionException(_OPTION_ISSUERNAME);
                    }

                    _issuer = getOpts [i].Value ();
                    _issuerSet = true;
                    break;
                }

                default:
                {
                    PEGASUS_UNREACHABLE(PEGASUS_ASSERT(false);)
                }
            }
        }
    }

    if ( _operationType == _OPERATION_TYPE_UNINITIALIZED )
    {
        //
        // No operation type was specified
        // Show the usage
        //
        throw CommandFormatException(localizeMessage(
            MSG_PATH, REQUIRED_ARGS_MISSING_KEY, REQUIRED_ARGS_MISSING));
    }

    if ( _operationType == _OPERATION_TYPE_ADD )
    {
        //
        // For -a option, the required option is -f,
        // make sure it is set.
        //
        if ( !_crlFileSet )
        {
            //
            // A required option is missing
            //
            throw MissingOptionException(_OPTION_CRLFILE);
        }

    }

    if ( _operationType == _OPERATION_TYPE_REMOVE )
    {
        //
        // For -r option, the required option is -i,
        // make sure it is set.
        //
        if ( !_issuerSet )
        {
            //
            // A required option is missing
            //
            throw MissingOptionException(_OPTION_ISSUERNAME);
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
Uint32 CIMCRLCommand::execute (
    ostream& outPrintWriter,
    ostream& errPrintWriter)
{
    //
    // Options HELP and VERSION
    // CIMServer need not be running for these options to work
    //
    if (_operationType == _OPERATION_TYPE_HELP)
    {
        errPrintWriter << _usage << endl;
        return (RC_SUCCESS);
    }

    if(_operationType == _OPERATION_TYPE_VERSION)
    {
        errPrintWriter << "Version " << PEGASUS_PRODUCT_VERSION << endl;
        return (RC_SUCCESS);
    }

    CIMClient client;

    client.setTimeout( _timeout );
    client.setRequestDefaultLanguages(); //l10n

    try
    {
        client.connectLocal();
    }
    catch (const CannotConnectException&)
    {
        errPrintWriter << localizeMessage(MSG_PATH,
            CANNOT_CONNECT_CIMSERVER_NOT_RUNNING_KEY,
            CANNOT_CONNECT_CIMSERVER_NOT_RUNNING) << endl;
        return RC_CONNECTION_FAILED;
    }
    catch (const Exception& e)
    {
        errPrintWriter << e.getMessage() << endl;
        return RC_ERROR;
    }

    //
    // Perform the requested operation
    //
    switch ( _operationType )
    {
        case _OPERATION_TYPE_ADD:
            try
            {
                _addCRL( client, outPrintWriter );
            }
            catch (const CIMException& e)
            {
                CIMStatusCode code = e.getCode();

                if (code == CIM_ERR_INVALID_CLASS)
                {
                    errPrintWriter << localizeMessage(MSG_PATH,
                       CRL_SCHEMA_NOT_LOADED_KEY,
                       CRL_SCHEMA_NOT_LOADED) << endl;
                }
                else
                {
                    errPrintWriter << e.getMessage() << endl;
                }

                return RC_ERROR;
            }
            catch (const ConnectionTimeoutException&)
            {
                errPrintWriter << localizeMessage(MSG_PATH,
                    CONNECTION_TIMEOUT_KEY,
                    CONNECTION_TIMEOUT);
                return RC_CONNECTION_TIMEOUT;
            }
            catch (const Exception& e)
            {
                errPrintWriter << e.getMessage() << endl;
                return RC_ERROR;
            }
            break;

        case _OPERATION_TYPE_REMOVE:
            try
            {
                _removeCRL ( client, outPrintWriter );
            }
            catch (const CIMException& e)
            {
                CIMStatusCode code = e.getCode();

                if (code == CIM_ERR_NOT_FOUND)
                {
                    errPrintWriter << localizeMessage(MSG_PATH,
                            CRL_NOT_FOUND_KEY, CRL_NOT_FOUND) << endl;
                    errPrintWriter << e.getMessage()  << endl;
                    return ( RC_CRL_DOES_NOT_EXIST );
                }
                else if (code == CIM_ERR_INVALID_CLASS)
                {
                    errPrintWriter << localizeMessage(MSG_PATH,
                        CRL_SCHEMA_NOT_LOADED_KEY,
                        CRL_SCHEMA_NOT_LOADED) << endl;
                }
                else
                {
                    errPrintWriter << e.getMessage() << endl;
                }
                return RC_ERROR;
            }
            catch (const ConnectionTimeoutException&)
            {
                errPrintWriter << localizeMessage(MSG_PATH,
                    CONNECTION_TIMEOUT_KEY,
                    CONNECTION_TIMEOUT);
                return RC_CONNECTION_TIMEOUT;
            }
            catch (const Exception& e)
            {
                errPrintWriter << e.getMessage() << endl;
                return RC_ERROR;
            }
            break;

        case _OPERATION_TYPE_LIST:
            try
            {
                _listCRL ( client, outPrintWriter );
            }
            catch (const CIMException& e)
            {
                CIMStatusCode code = e.getCode();

                if (code == CIM_ERR_NOT_FOUND)
                {
                    errPrintWriter << localizeMessage(MSG_PATH,
                            CRL_NOT_FOUND_KEY, CRL_NOT_FOUND) << endl;
                    errPrintWriter << e.getMessage()  << endl;
                    return ( RC_CRL_DOES_NOT_EXIST );
                }
                else if (code == CIM_ERR_INVALID_CLASS)
                {
                    errPrintWriter << localizeMessage(MSG_PATH,
                        CRL_SCHEMA_NOT_LOADED_KEY,
                        CRL_SCHEMA_NOT_LOADED) << endl;
                }
                else
                {
                    errPrintWriter << e.getMessage() << endl;
                }
                return RC_ERROR;
            }
            catch (const ConnectionTimeoutException&)
            {
                errPrintWriter << localizeMessage(MSG_PATH,
                    CONNECTION_TIMEOUT_KEY,
                    CONNECTION_TIMEOUT);
                return RC_CONNECTION_TIMEOUT;
            }
            catch (const Exception& e)
            {
                errPrintWriter << e.getMessage() << endl;
                return RC_ERROR;
            }
            break;
    }

    return (RC_SUCCESS);
}

PEGASUS_NAMESPACE_END

// exclude main from the Pegasus Namespace


PEGASUS_USING_PEGASUS;

PEGASUS_USING_STD;

///////////////////////////////////////////////////////////////////////////
/**

    Parses the command line, and executes the command.

    @param   argc  the number of command line arguments
    @param   argv  the string vector of command line arguments

    @return  0                  if the command is successful
             1                  if an error occurs in executing the command

 */
///////////////////////////////////////////////////////////////////////////

int main (int argc, char* argv [])
{
    //l10n set message loading to process locale
    MessageLoader::_useProcessLocale = true;

    CIMCRLCommand    command = CIMCRLCommand();
    int            retCode;

    try
    {
        command.setCommand (argc, argv);
    }
    catch (const CommandFormatException& cfe)
    {
        cerr << CIMCRLCommand::COMMAND_NAME << ": " << cfe.getMessage ()
             << endl;
        cerr << "Use '--help' to obtain command syntax." << endl;
        exit (Command::RC_ERROR);
    }

    retCode = command.execute (cout, cerr);

    exit (retCode);
    return 0;
}

