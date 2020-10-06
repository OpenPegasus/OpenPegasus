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

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/Constants.h>
#include <Pegasus/Common/AutoPtr.h>

#include <iostream>

#include <Clients/cliutils/Command.h>
#include <Clients/cliutils/CommandException.h>

#include <Pegasus/getoopt/getoopt.h>

#include <Pegasus/Client/CIMClient.h>
#include <Pegasus/Common/CIMProperty.h>
#include <Pegasus/Common/CIMObjectPath.h>
#include <Pegasus/Common/CIMStatusCode.h>
#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/String.h>
#include <Pegasus/Common/System.h>
#include <Pegasus/Common/MessageLoader.h> //l10n

#include <Pegasus/Common/PegasusVersion.h>

PEGASUS_USING_STD;

PEGASUS_NAMESPACE_BEGIN

//l10n
/**
 * The CLI message resource name
 */

static const char MSG_PATH [] = "pegasus/pegasusCLI";

//l10n end

/**
    The command name.
*/
static const char COMMAND_NAME [] = "cimauth";

/**
    This constant represents the name of the User name property in the schema
*/
static const CIMName PROPERTY_NAME_USERNAME        = CIMName ("Username");

/**
    This constant represents the name of the Namespace property in the schema
*/
static const CIMName PROPERTY_NAME_NAMESPACE       = CIMName ("Namespace");

/**
    This constant represents the name of the authorizations property in
    the schema
*/
static const CIMName PROPERTY_NAME_AUTHORIZATION   = CIMName ("Authorization");

/**
    The usage string for this command.  This string is displayed
    when an error occurs in parsing or validating the command line.
*/
static const char USAGE []                        = "Usage: ";

/*
    These constants represent the operation modes supported by the CLI.
    Any new operation should be added here.
*/

/**
    This constant signifies that an operation option has not been recorded
*/
static const Uint32 OPERATION_TYPE_UNINITIALIZED  = 0;

/**
    This constant represents a add user operation
*/
static const Uint32 OPERATION_TYPE_ADD            = 1;

/**
    This constant represents a add user operation
*/
static const Uint32 OPERATION_TYPE_MODIFY         = 2;

/**
    This constant represents a remove user operation
*/
static const Uint32 OPERATION_TYPE_REMOVE         = 3;

/**
    This constant represents a list operation
*/
static const Uint32 OPERATION_TYPE_LIST           = 4;

/**
    This constant represents a help operation
*/
static const Uint32 OPERATION_TYPE_HELP           = 5;

/**
    This constant represents a version display operation
*/
static const Uint32 OPERATION_TYPE_VERSION        = 6;



/**
    The constants representing the messages.
*/

//l10n default messages and corresponding resource keys

static const char CIMOM_NOT_RUNNING [] =
    "CIM Server may not be running.";

static const char CIMOM_NOT_RUNNING_KEY [] =
    "Clients.cimauth.CIMAuthCommand.CIMOM_NOT_RUNNING";

static const char ADD_AUTH_FAILURE [] =
    "Failed to add authorizations.";

static const char ADD_AUTH_FAILURE_KEY [] =
    "Clients.cimauth.CIMAuthCommand.ADD_AUTH_FAILURE";

static const char MODIFY_AUTH_FAILURE [] =
    "Failed to modify authorizations.";

static const char MODIFY_AUTH_FAILURE_KEY [] =
    "Clients.cimauth.CIMAuthCommand.MODIFY_AUTH_FAILURE";

static const char REMOVE_AUTH_FAILURE [] =
    "Failed to remove authorizations.";

static const char REMOVE_AUTH_FAILURE_KEY [] =
    "Clients.cimauth.CIMAuthCommand.REMOVE_AUTH_FAILURE";

static const char LIST_AUTH_FAILURE [] =
    "Failed to list the users. ";

static const char LIST_AUTH_FAILURE_KEY [] =
    "Clients.cimauth.CIMAuthCommand.LIST_AUTH_FAILURE";

static const char ADD_AUTH_SUCCESS [] =
    "Authorizations added successfully.";

static const char ADD_AUTH_SUCCESS_KEY [] =
    "Clients.cimauth.CIMAuthCommand.ADD_AUTH_SUCCESS";

static const char MODIFY_AUTH_SUCCESS [] =
    "Authorizations modified successfully.";

static const char MODIFY_AUTH_SUCCESS_KEY [] =
    "Clients.cimauth.CIMAuthCommand.MODIFY_AUTH_SUCCESS";

static const char REMOVE_AUTH_SUCCESS [] =
    "Authorizations removed successfully.";

static const char REMOVE_AUTH_SUCCESS_KEY [] =
    "Clients.cimauth.CIMAuthCommand.REMOVE_AUTH_SUCCESS";

static const char AUTH_SCHEMA_NOT_LOADED [] =
    "Please restore the internal repository on the CIM Server.";

static const char AUTH_SCHEMA_NOT_LOADED_KEY [] =
    "Clients.cimauth.CIMAuthCommand.AUTH_SCHEMA_NOT_LOADED";

static const char REQUIRED_ARGS_MISSING [] =
    "Required arguments missing.";

static const char REQUIRED_ARGS_MISSING_KEY [] =
    "Clients.cimauth.CIMAuthCommand.REQUIRED_ARGS_MISSING";

static const char AUTH_ALREADY_EXISTS [] =
    "Specified user authorizations already exist.";

static const char AUTH_ALREADY_EXISTS_KEY [] =
    "Clients.cimauth.CIMAuthCommand.AUTH_ALREADY_EXISTS";

static const char AUTH_NOT_FOUND [] =
    "Specified user authorizations were not found.";

static const char AUTH_NOT_FOUND_KEY [] =
    "Clients.cimauth.CIMAuthCommand.AUTH_NOT_FOUND";

static const char ERR_USAGE_KEY [] =
    "Clients.cimauth.CIMAuthCommand.ERR_USAGE";

static const char ERR_USAGE [] =
    "Use '--help' to obtain command syntax.";
//l10n end default messages and resource keys


static const char   LONG_HELP []  = "help";

static const char   LONG_VERSION []  = "version";

/**
    The option character used to specify add user.
*/
static const char   OPTION_ADD                 = 'a';

/**
    The option character used to specify modify user.
*/
static const char   OPTION_MODIFY              = 'm';

/**
    The option character used to specify user name.
*/
static const char   OPTION_USER_NAME           = 'u';

/**
    The option character used to specify password.
*/
static const char   OPTION_NAMESPACE           = 'n';

/**
    The option character used to specify read permision.
*/
static const char   OPTION_READ                = 'R';

/**
    The option character used to specify write permision.
*/
static const char   OPTION_WRITE               = 'W';

/**
    The option character used to specify remove user.
*/
static const char   OPTION_REMOVE              = 'r';

/**
    The option character used to specify listing of users.
*/
static const char   OPTION_LIST                = 'l';

/**
    The option character used to display help info.
*/
static const char   OPTION_HELP                = 'h';

/**
    The option character used to display version info.
*/
static const char   OPTION_VERSION             = 'v';

/**
This is a CLI used to manage users of the CIM Server.  This command supports
operations to add, modify, list and remove users.

@author Nag Boranna, Hewlett-Packard Company
*/

class CIMAuthCommand : public Command
{

public:

    /**
        Constructs a CIMAuthCommand and initializes instance variables.
    */
    CIMAuthCommand ();

    /**
    Parses the command line, validates the options, and sets instance
    variables based on the option arguments.

    @param  args       The string array containing the command line arguments
    @param  argc       The int containing the arguments count

    @throws  CommandFormatException  if an error is encountered in parsing
                                     the command line
    */
    void setCommand (Uint32 argc, char* argv []);

    /**
    Executes the command and writes the results to the output streams.

    @param ostream    The stream to which command output is written.
    @param ostream    The stream to which command errors are written.
    @return  0        if the command is successful
             1        if an error occurs in executing the command
    */
    Uint32 execute (
        PEGASUS_STD(ostream)& outPrintWriter,
        PEGASUS_STD(ostream)& errPrintWriter);


private:

    //
    // Add a new user to the CIM Server
    //
    // @param ostream        The stream to which command output is written.
    // @param ostream        The stream to which command errors are written.
    //
    // @exception Exception  if failed to add user
    //
    void _AddAuthorization
        (
        PEGASUS_STD(ostream)& outPrintWriter,
        PEGASUS_STD(ostream)& errPrintWriter
        );

    //
    // Modify an existing user's password.
    //
    // @param ostream          The stream to which command output is written.
    // @param ostream          The stream to which command errors are written.
    //
    // @exception Exception  if failed to modify password
    //
    void _ModifyAuthorization
        (
        PEGASUS_STD(ostream)& outPrintWriter,
        PEGASUS_STD(ostream)& errPrintWriter
        );

    //
    // Remove an existing user from the CIM Server
    //
    // @param ostream          The stream to which command output is written.
    // @param ostream          The stream to which command errors are written.
    //
    // @exception Exception  if failed to remove user
    //
    void _RemoveAuthorization
        (
        PEGASUS_STD(ostream)& outPrintWriter,
        PEGASUS_STD(ostream)& errPrintWriter
        );

    //
    // List all users.
    //
    // @param ostream        The stream to which command output is written.
    // @param ostream        The stream to which command errors are written.
    //
    void _ListAuthorization
    (
        PEGASUS_STD(ostream)& outPrintWriter,
        PEGASUS_STD(ostream)& errPrintWriter
    );

    //
    // The CIM Client reference
    //
    AutoPtr<CIMClient>    _client;

    //
    // The host name.
    //
    String        _hostName;

    //
    // The name of the user.
    //
    String        _userName;

    //
    // The namespace.
    //
    String        _namespace;

    //
    // The authorization capability.
    //
    String        _authorizations;

    //
    // The type of operation specified on the command line.
    //
    Uint32        _operationType;

    //
    // Flags for command options
    //
    Boolean       _userNameSet;
    Boolean       _namespaceSet;
    Boolean       _readFlagSet;
    Boolean       _writeFlagSet;

    //String for usage information
    String usage;

};

/**
    Constructs a CIMAuthCommand and initializes instance variables.
*/
CIMAuthCommand::CIMAuthCommand ()
{
    /**
        Initialize the instance variables.
    */
    _operationType       = OPERATION_TYPE_UNINITIALIZED;
    _namespaceSet        = false;
    _userNameSet         = false;
    _readFlagSet         = false;
    _writeFlagSet        = false;

    /**
        Build the usage string for the config command.
    */

    usage.reserveCapacity(200);
    usage.append(USAGE);
    usage.append(COMMAND_NAME);

    usage.append(" -").append(OPTION_ADD);
    usage.append(" -").append(OPTION_USER_NAME).append(" username");
    usage.append(" -").append(OPTION_NAMESPACE).append(" namespace");
    usage.append(" [ -").append(OPTION_READ).append(" ]");
    usage.append(" [ -").append(OPTION_WRITE).append(" ] \n");

    usage.append("               -").append(OPTION_MODIFY);
    usage.append(" -").append(OPTION_USER_NAME).append(" username");
    usage.append(" -").append(OPTION_NAMESPACE).append(" namespace");
    usage.append(" [ -").append(OPTION_READ).append(" ]");
    usage.append(" [ -").append(OPTION_WRITE).append(" ] \n");

    usage.append("               -").append(OPTION_REMOVE);
    usage.append(" -").append(OPTION_USER_NAME).append(" username");
    usage.append(" [ -").append(OPTION_NAMESPACE).append(" namespace ]\n");

    usage.append("               -").append(OPTION_LIST).append(" \n");

    usage.append("               -").append(OPTION_HELP).append(" \n");
    usage.append("               --").append(LONG_HELP).append(" \n");
    usage.append("               --").append(LONG_VERSION).append(" \n");

    usage.append("Options : \n");
    usage.append("    -a         - Add authorizations for a user on a"
                    " namespace\n");
    usage.append("    -h, --help - Display this help message\n");
    usage.append("    -l         - Display the authorizations of all the"
                    " authorized users\n");
    usage.append("    -m         - Modify authorizations for a user on a"
                 "   namespace\n");
    usage.append("    -n         - Use the specified namespace\n");
    usage.append("    -r         - Remove authorizations for a user on a"
                    " namespace\n");
    usage.append("    -R         - Specify a READ authorization (Default)\n");
    usage.append("    -u         - Perform operations for the specified user"
                    " name\n");
    usage.append("    --version  - Display CIM Server version number\n");
    usage.append("    -W         - Specify a WRITE authorization\n");

    usage.append("\nUsage note: The cimauth command requires that the"
                    " CIM Server is running.\n");

    setUsage (usage);
}


/**
    Parses the command line, validates the options, and sets instance
    variables based on the option arguments.

    @exception CommandFormatException
*/
void CIMAuthCommand::setCommand (Uint32 argc, char* argv [])
{
    Uint32            i                = 0;
    Uint32            c                = 0;
    String            badOptionString  = String ();
    String            optString        = String ();

    //
    //  Construct optString
    //
    optString.append(OPTION_ADD);
    optString.append(OPTION_USER_NAME);
    optString.append(getoopt::GETOPT_ARGUMENT_DESIGNATOR);
    optString.append(OPTION_NAMESPACE);
    optString.append(getoopt::GETOPT_ARGUMENT_DESIGNATOR);

    optString.append(OPTION_MODIFY);
    optString.append(OPTION_USER_NAME);
    optString.append(getoopt::GETOPT_ARGUMENT_DESIGNATOR);
    optString.append(OPTION_NAMESPACE);
    optString.append(getoopt::GETOPT_ARGUMENT_DESIGNATOR);

    optString.append(OPTION_REMOVE);
    optString.append(OPTION_USER_NAME);
    optString.append(getoopt::GETOPT_ARGUMENT_DESIGNATOR);
    optString.append(OPTION_NAMESPACE);
    optString.append(getoopt::GETOPT_ARGUMENT_DESIGNATOR);

    optString.append(OPTION_LIST);
    optString.append(OPTION_READ);
    optString.append(OPTION_WRITE);
    optString.append(OPTION_HELP);

    //
    //  Initialize and parse options
    //
    getoopt options ("");
    options.addFlagspec(optString);

    //PEP#167 - adding long flag for options : 'help' and 'version'
    options.addLongFlagspec(LONG_HELP,getoopt::NOARG);
    options.addLongFlagspec(LONG_VERSION,getoopt::NOARG);


    options.parse (argc, argv);

    if (options.hasErrors ())
    {
        throw CommandFormatException(options.getErrorStrings()[0]);
    }
    _operationType = OPERATION_TYPE_UNINITIALIZED;

    //
    //  Get options and arguments from the command line
    //
    for (i =  options.first (); i <  options.last (); i++)
    {
        if (options[i].getType () == Optarg::LONGFLAG)
        {
            if (options[i].getopt () == LONG_HELP)
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
            else if (options[i].getopt () == LONG_VERSION)
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
        else if (options [i].getType () == Optarg::REGULAR)
        {
            //
            //  The cimauth command has no non-option argument options
            //
            throw UnexpectedArgumentException(options[i].Value());
        }
        else /* if (options [i].getType () == Optarg::FLAG) */
        {

            c = options [i].getopt () [0];

            switch (c)
            {
                case OPTION_ADD:
                {
                    if (_operationType != OPERATION_TYPE_UNINITIALIZED)
                    {
                        //
                        // More than one operation option was found
                        //
                        throw UnexpectedOptionException(OPTION_ADD);
                    }

                    if (options.isSet (OPTION_ADD) > 1)
                    {
                        //
                        // More than one add user option was found
                        //
                        throw DuplicateOptionException(OPTION_ADD);
                    }

                    _operationType = OPERATION_TYPE_ADD;

                    break;
                }
                case OPTION_MODIFY:
                {
                    if (_operationType != OPERATION_TYPE_UNINITIALIZED)
                    {
                        //
                        // More than one operation option was found
                        //
                        throw UnexpectedOptionException(OPTION_MODIFY);
                    }

                    if (options.isSet (OPTION_MODIFY) > 1)
                    {
                        //
                        // More than one modify user option was found
                        //
                        throw DuplicateOptionException(OPTION_MODIFY);
                    }

                    _operationType = OPERATION_TYPE_MODIFY;

                    break;
                }
                case OPTION_REMOVE:
                {
                    if (_operationType != OPERATION_TYPE_UNINITIALIZED)
                    {
                        //
                        // More than one operation option was found
                        //
                        throw UnexpectedOptionException(OPTION_REMOVE);
                    }

                    if (options.isSet (OPTION_REMOVE) > 1)
                    {
                        //
                        // More than one remove user option was found
                        //
                        throw DuplicateOptionException(OPTION_REMOVE);
                    }

                    _operationType = OPERATION_TYPE_REMOVE;

                    break;
                }
                case OPTION_USER_NAME:
                {
                    if (options.isSet (OPTION_USER_NAME) > 1)
                    {
                        //
                        // More than one username option was found
                        //
                        throw DuplicateOptionException(OPTION_USER_NAME);
                    }

                    _userName = options [i].Value ();

                    _userNameSet = true;

                    break;
                }
                case OPTION_NAMESPACE:
                {
                    if (options.isSet (OPTION_NAMESPACE) > 1)
                    {
                        //
                        // More than one password option was found
                        //
                        throw DuplicateOptionException(OPTION_NAMESPACE);
                    }

                    _namespace = options [i].Value ();

                    _namespaceSet = true;

                    break;
                }
                case OPTION_LIST:
                {
                    if (_operationType != OPERATION_TYPE_UNINITIALIZED)
                    {
                        //
                        // More than one operation option was found
                        //
                        throw UnexpectedOptionException(OPTION_LIST);
                    }

                    if (options.isSet (OPTION_LIST) > 1)
                    {
                        //
                        // More than one list option was found
                        //
                        throw DuplicateOptionException(OPTION_LIST);
                    }
                    _operationType = OPERATION_TYPE_LIST;
                    break;
                }
                case OPTION_READ:
                {
                    if (options.isSet (OPTION_READ) > 1)
                    {
                        //
                        // More than one read option was found
                        //
                        throw DuplicateOptionException(OPTION_READ);
                    }
                    _authorizations.append("r");
                    _readFlagSet = true;
                    break;
                }
                case OPTION_WRITE:
                {
                    if (options.isSet (OPTION_WRITE) > 1)
                    {
                        //
                        // More than one write option was found
                        //
                        throw DuplicateOptionException(OPTION_WRITE);
                    }
                    _authorizations.append("w");
                    _writeFlagSet = true;
                    break;
                }

                //PEP#167 - 2 new cases added below for HELP and VERSION
                case OPTION_HELP:
                {
                    if (_operationType != OPERATION_TYPE_UNINITIALIZED)
                    {
                        //
                        // More than one operation option was found
                        //
                        throw UnexpectedOptionException(OPTION_HELP);
                    }

                    if (options.isSet (OPTION_HELP) > 1)
                    {
                        //
                        // More than one list option was found
                        //
                        throw DuplicateOptionException(OPTION_HELP);
                    }
                    _operationType = OPERATION_TYPE_HELP;
                    break;
                }
                case OPTION_VERSION:
                {
                    if (_operationType != OPERATION_TYPE_UNINITIALIZED)
                    {
                        //
                        // More than one operation option was found
                        //
                        throw UnexpectedOptionException(OPTION_VERSION);
                    }

                    if (options.isSet (OPTION_VERSION) > 1)
                    {
                        //
                        // More than one list option was found
                        //
                        throw DuplicateOptionException(OPTION_VERSION);
                    }
                    _operationType = OPERATION_TYPE_VERSION;
                    break;
                }

                default:
                {
                    //
                    // Should never get here
                    //
                    break;
                }
            }
        }
    }

    if ( _operationType == OPERATION_TYPE_UNINITIALIZED )
    {
        //
        // No operation type was specified
        // Show the usage
        //
        throw CommandFormatException(localizeMessage(
            MSG_PATH, REQUIRED_ARGS_MISSING_KEY, REQUIRED_ARGS_MISSING));
    }

    if ( _operationType == OPERATION_TYPE_ADD ||
         _operationType == OPERATION_TYPE_MODIFY )
    {
        if ( !_userNameSet )
        {
            //
            // A required option is missing
            //
            throw MissingOptionException(OPTION_USER_NAME);
        }
        if ( !_namespaceSet )
        {
            //
            // A required option is missing
            //
            throw MissingOptionException(OPTION_NAMESPACE);
        }
        if ( !_readFlagSet && !_writeFlagSet )
        {
            //
            // Authorization flags were not specified,
            // set default to read only
            //
            _authorizations.append("r");
            _readFlagSet = true;
        }
    }
    else if ( _operationType == OPERATION_TYPE_REMOVE )
    {
        if ( !_userNameSet )
        {
            //
            // A required option is missing
            //
            throw MissingOptionException(OPTION_USER_NAME);
        }
    }
    else if (_operationType != OPERATION_TYPE_ADD &&
        _operationType != OPERATION_TYPE_MODIFY)
    {
        //
        // Unexpected option was specified
        //
        if ( _readFlagSet )
        {
            throw UnexpectedOptionException(OPTION_READ);
        }
        else if ( _writeFlagSet )
        {
            throw UnexpectedOptionException(OPTION_WRITE);
        }
    }
}

/**
    Executes the command and writes the results to the PrintWriters.
*/
Uint32 CIMAuthCommand::execute (
    ostream& outPrintWriter,
    ostream& errPrintWriter)
{
    if ( _operationType == OPERATION_TYPE_UNINITIALIZED )
    {
        //
        // The command was not initialized
        //
        return ( RC_ERROR );
    }
    //PEP#167 - Added Options HELP and VERSION
    //PEP#167 - CIMServer need not be running for these to work
    else if (_operationType == OPERATION_TYPE_HELP)
    {
        cerr << usage << endl;
        return (RC_SUCCESS);
    }
    else if(_operationType == OPERATION_TYPE_VERSION)
    {
        cerr << "Version " << PEGASUS_PRODUCT_VERSION << endl;
        return (RC_SUCCESS);
    }

    //
    // Get local host name
    //
    _hostName.assign(System::getHostName());

    try
    {
        // Construct the CIMClient and set to request server messages
        // in the default language of this client process.
        _client.reset(new CIMClient);
        _client->setRequestDefaultLanguages(); //l10n
    }
    catch (Exception & e)
    {
        errPrintWriter << e.getMessage() << endl;
        return ( RC_ERROR );
    }

    try
    {
        //
        // Open connection with CIMSever
        //
        _client->connectLocal();
    }
    catch(Exception&)
    {
        outPrintWriter << localizeMessage(MSG_PATH,
            CIMOM_NOT_RUNNING_KEY,
            CIMOM_NOT_RUNNING)
            << endl;
        return 1;
    }

    //
    // Perform the requested operation
    //
    switch ( _operationType )
    {
        case OPERATION_TYPE_ADD:
            try
            {
                _AddAuthorization( outPrintWriter, errPrintWriter );
            }
            catch (CIMException& e)
            {
                CIMStatusCode code = e.getCode();

                if (code == CIM_ERR_FAILED)
                {
                    outPrintWriter << localizeMessage(MSG_PATH,
                        ADD_AUTH_FAILURE_KEY,
                        ADD_AUTH_FAILURE)
                        << endl;
                    errPrintWriter << e.getMessage() << endl;
                }
                else if (code == CIM_ERR_NOT_SUPPORTED)
                {
                    outPrintWriter << localizeMessage(MSG_PATH,
                        ADD_AUTH_FAILURE_KEY,
                        ADD_AUTH_FAILURE)
                        << endl;
                    errPrintWriter << e.getMessage()  << endl;
                }
                else if (code == CIM_ERR_ALREADY_EXISTS)
                {
                    outPrintWriter << localizeMessage(MSG_PATH,
                        ADD_AUTH_FAILURE_KEY,
                        ADD_AUTH_FAILURE)
                        << endl;
                    outPrintWriter << localizeMessage(MSG_PATH,
                        AUTH_ALREADY_EXISTS_KEY,
                        AUTH_ALREADY_EXISTS)
                        << endl;

                    errPrintWriter << e.getMessage()  << endl;
                }
                else if (code == CIM_ERR_INVALID_CLASS)
                {
                    outPrintWriter << localizeMessage(MSG_PATH,
                        ADD_AUTH_FAILURE_KEY,
                        ADD_AUTH_FAILURE)
                        << endl;
                    outPrintWriter << localizeMessage(MSG_PATH,
                        AUTH_SCHEMA_NOT_LOADED_KEY,
                        AUTH_SCHEMA_NOT_LOADED)
                        << endl;
                }
                else
                {
                    errPrintWriter << e.getMessage() << endl;
                }
                return ( RC_ERROR );
            }
            catch (Exception& e)
            {
                //l10n
                //outPrintWriter << ADD_AUTH_FAILURE << endl <<
                    //e.getMessage() << endl;
                outPrintWriter << localizeMessage(MSG_PATH,
                    ADD_AUTH_FAILURE_KEY,
                    ADD_AUTH_FAILURE)
                    << endl << e.getMessage() << endl;
                return ( RC_ERROR );
            }
            break;

        case OPERATION_TYPE_MODIFY:
            try
            {
                _ModifyAuthorization( outPrintWriter, errPrintWriter );
            }
            catch (CIMException& e)
            {
                CIMStatusCode code = e.getCode();
                if (code == CIM_ERR_FAILED)
                {
                    //l10n
                    //outPrintWriter << MODIFY_AUTH_FAILURE << endl;
                    outPrintWriter << localizeMessage(MSG_PATH,
                        MODIFY_AUTH_FAILURE_KEY,
                        MODIFY_AUTH_FAILURE)
                        << endl;
                    errPrintWriter << e.getMessage() << endl;
                }
                else if (code == CIM_ERR_NOT_SUPPORTED)
                {
                    //l10n
                    //outPrintWriter << MODIFY_AUTH_FAILURE << endl;
                    outPrintWriter << localizeMessage(MSG_PATH,
                        MODIFY_AUTH_FAILURE_KEY,
                        MODIFY_AUTH_FAILURE)
                        << endl;
                    errPrintWriter << e.getMessage()  << endl;
                }
                else if (code == CIM_ERR_NOT_FOUND)
                {
                    //l10n
                    //outPrintWriter << MODIFY_AUTH_FAILURE << endl;
                    //outPrintWriter << AUTH_NOT_FOUND          << endl;
                    outPrintWriter << localizeMessage(MSG_PATH,
                        MODIFY_AUTH_FAILURE_KEY,
                        MODIFY_AUTH_FAILURE)
                        << endl;
                    outPrintWriter << localizeMessage(MSG_PATH,
                        AUTH_NOT_FOUND_KEY,
                        AUTH_NOT_FOUND)
                        << endl;
                    errPrintWriter << e.getMessage()  << endl;
                }
                else if (code == CIM_ERR_INVALID_CLASS)
                {
                    //l10n
                    //outPrintWriter << MODIFY_AUTH_FAILURE << endl;
                    //outPrintWriter << AUTH_SCHEMA_NOT_LOADED << endl;
                    outPrintWriter << localizeMessage(MSG_PATH,
                        MODIFY_AUTH_FAILURE_KEY,
                        MODIFY_AUTH_FAILURE)
                        << endl;
                    outPrintWriter << localizeMessage(MSG_PATH,
                        AUTH_SCHEMA_NOT_LOADED_KEY,
                        AUTH_SCHEMA_NOT_LOADED)
                        << endl;
                }
                else
                {
                    errPrintWriter << e.getMessage() << endl;
                }
                return ( RC_ERROR );
            }
            catch (Exception& e)
            {
                //l10n
                //outPrintWriter << MODIFY_AUTH_FAILURE << endl <<
                    //e.getMessage() << endl;
                outPrintWriter << localizeMessage(MSG_PATH,
                    MODIFY_AUTH_FAILURE_KEY,
                    MODIFY_AUTH_FAILURE)
                    << endl << e.getMessage() << endl;
                return ( RC_ERROR );
            }
            break;

        case OPERATION_TYPE_REMOVE:
            try
            {
                _RemoveAuthorization( outPrintWriter, errPrintWriter );
            }
            catch (CIMException& e)
            {
                CIMStatusCode code = e.getCode();
                if (code == CIM_ERR_FAILED)
                {
                    outPrintWriter << localizeMessage(MSG_PATH,
                        REMOVE_AUTH_FAILURE_KEY,
                        REMOVE_AUTH_FAILURE)
                        << endl;
                    errPrintWriter << e.getMessage() << endl;
                }
                else if (code == CIM_ERR_NOT_SUPPORTED)
                {
                    outPrintWriter << localizeMessage(MSG_PATH,
                        REMOVE_AUTH_FAILURE_KEY,
                        REMOVE_AUTH_FAILURE)
                        << endl;
                    errPrintWriter << e.getMessage()  << endl;
                }
                else if (code == CIM_ERR_NOT_FOUND)
                {
                    outPrintWriter << localizeMessage(MSG_PATH,
                        REMOVE_AUTH_FAILURE_KEY,
                        REMOVE_AUTH_FAILURE)
                        << endl;
                    outPrintWriter << localizeMessage(MSG_PATH,
                        AUTH_NOT_FOUND_KEY,
                        AUTH_NOT_FOUND)
                        << endl;
                    errPrintWriter << e.getMessage()  << endl;
                }
                else if (code == CIM_ERR_INVALID_CLASS)
                {
                    outPrintWriter << localizeMessage(MSG_PATH,
                        REMOVE_AUTH_FAILURE_KEY,
                        REMOVE_AUTH_FAILURE)
                        << endl;
                    outPrintWriter << localizeMessage(MSG_PATH,
                        AUTH_SCHEMA_NOT_LOADED_KEY,
                        AUTH_SCHEMA_NOT_LOADED)
                        << endl;
                }
                else
                {
                    errPrintWriter << e.getMessage() << endl;
                }
                return ( RC_ERROR );
            }
            catch (Exception& e)
            {
                outPrintWriter << localizeMessage(MSG_PATH,
                    REMOVE_AUTH_FAILURE_KEY,
                    REMOVE_AUTH_FAILURE)
                    << endl << e.getMessage() << endl;
                return ( RC_ERROR );
            }
            break;

        case OPERATION_TYPE_LIST:
            try
            {
                _ListAuthorization( outPrintWriter, errPrintWriter );
            }
            catch (CIMException& e)
            {
                CIMStatusCode code = e.getCode();
                if (code == CIM_ERR_FAILED)
                {
                    outPrintWriter << localizeMessage(MSG_PATH,
                        LIST_AUTH_FAILURE_KEY,
                        LIST_AUTH_FAILURE)
                        << endl;
                    errPrintWriter << e.getMessage() << endl;
                }
                else if (code == CIM_ERR_NOT_SUPPORTED)
                {
                    outPrintWriter << localizeMessage(MSG_PATH,
                        LIST_AUTH_FAILURE_KEY,
                        LIST_AUTH_FAILURE)
                        << endl;
                    errPrintWriter << e.getMessage()  << endl;
                }
                else if (code == CIM_ERR_ALREADY_EXISTS)
                {
                    outPrintWriter << localizeMessage(MSG_PATH,
                        LIST_AUTH_FAILURE_KEY,
                        LIST_AUTH_FAILURE)
                        << endl;
                    errPrintWriter << e.getMessage()  << endl;
                }
                else if (code == CIM_ERR_INVALID_CLASS)
                {
                    outPrintWriter << localizeMessage(MSG_PATH,
                        LIST_AUTH_FAILURE_KEY,
                        LIST_AUTH_FAILURE)
                        << endl;
                    outPrintWriter << localizeMessage(MSG_PATH,
                        AUTH_SCHEMA_NOT_LOADED_KEY,
                        AUTH_SCHEMA_NOT_LOADED)
                        << endl;
                }
                else
                {
                    errPrintWriter << e.getMessage() << endl;
                }
                return ( RC_ERROR );
            }
            catch (Exception& e)
            {
                outPrintWriter << localizeMessage(MSG_PATH,
                    LIST_AUTH_FAILURE_KEY,
                    LIST_AUTH_FAILURE)
                    << endl << e.getMessage() << endl;
                return ( RC_ERROR );
            }
            break;

        default:
            //
            // Should never get here
            //
            break;
    }

    return (RC_SUCCESS);
}

/**
    Add the user to the CIM Server.
*/
void CIMAuthCommand::_AddAuthorization
    (
    ostream& outPrintWriter,
    ostream& errPrintWriter
    )
{
    CIMProperty prop;

    CIMInstance newInstance( PEGASUS_CLASSNAME_AUTHORIZATION );
    newInstance.addProperty (
        CIMProperty( PROPERTY_NAME_USERNAME, _userName ) );
    newInstance.addProperty (
        CIMProperty( PROPERTY_NAME_NAMESPACE , _namespace ) );
    newInstance.addProperty (
        CIMProperty( PROPERTY_NAME_AUTHORIZATION, _authorizations ) );

    _client->createInstance(
        PEGASUS_NAMESPACENAME_AUTHORIZATION,
        newInstance);

    outPrintWriter << localizeMessage(MSG_PATH,
        ADD_AUTH_SUCCESS_KEY,
        ADD_AUTH_SUCCESS)
        << endl;
}

//
// Modify the password for a user
//
void CIMAuthCommand::_ModifyAuthorization
    (
    ostream& outPrintWriter,
    ostream& errPrintWriter
    )
{
    Array<CIMKeyBinding>      kbArray;
    CIMKeyBinding             kb;

    //
    // Build the input params
    //
    kb.setName(PROPERTY_NAME_USERNAME);
    kb.setValue(_userName);
    kb.setType(CIMKeyBinding::STRING);

    kbArray.append(kb);

    kb.setName(PROPERTY_NAME_NAMESPACE);
    kb.setValue(_namespace);
    kb.setType(CIMKeyBinding::STRING);

    kbArray.append(kb);

    CIMObjectPath reference(
        _hostName, PEGASUS_NAMESPACENAME_AUTHORIZATION,
        PEGASUS_CLASSNAME_AUTHORIZATION, kbArray);

    CIMInstance modifiedInst( PEGASUS_CLASSNAME_AUTHORIZATION );
    modifiedInst.addProperty(
        CIMProperty( PROPERTY_NAME_USERNAME, _userName ) );
    modifiedInst.addProperty(
        CIMProperty( PROPERTY_NAME_NAMESPACE , _namespace ) );
    modifiedInst.addProperty(
        CIMProperty( PROPERTY_NAME_AUTHORIZATION, _authorizations ) );

    CIMInstance namedInstance (modifiedInst);
    namedInstance.setPath (reference);
    _client->modifyInstance(
        PEGASUS_NAMESPACENAME_AUTHORIZATION,
        namedInstance);

    outPrintWriter << localizeMessage(MSG_PATH,
        MODIFY_AUTH_SUCCESS_KEY,
        MODIFY_AUTH_SUCCESS)
        << endl;
}

//
// Remove a user
//
void CIMAuthCommand::_RemoveAuthorization
    (
    ostream& outPrintWriter,
    ostream& errPrintWriter
    )
{
    Array<CIMKeyBinding> kbArray;
    CIMKeyBinding        kb;

    //
    // If namespace is specified
    //
    if ( _namespaceSet )
    {
        //
        // Build the input params
        //
        kb.setName(PROPERTY_NAME_USERNAME);
        kb.setValue(_userName);
        kb.setType(CIMKeyBinding::STRING);

        kbArray.append(kb);

        kb.setName(PROPERTY_NAME_NAMESPACE);
        kb.setValue(_namespace);
        kb.setType(CIMKeyBinding::STRING);

        kbArray.append(kb);

        CIMObjectPath reference(
            _hostName, PEGASUS_NAMESPACENAME_AUTHORIZATION,
            PEGASUS_CLASSNAME_AUTHORIZATION, kbArray);

        _client->deleteInstance(
            PEGASUS_NAMESPACENAME_AUTHORIZATION,
            reference);
    }
    else
    {
        //
        // ATTN: If namespace is not specified, do enumerate instance
        // names to get all the namespaces and call delete instance for
        // each of the namespaces.
        //
        Array<CIMObjectPath> instanceNames =
            _client->enumerateInstanceNames(
                PEGASUS_NAMESPACENAME_AUTHORIZATION,
                PEGASUS_CLASSNAME_AUTHORIZATION);
        //
        //
        //
        for (Uint32 i = 0; i < instanceNames.size(); i++)
        {
            String user;

            kbArray = instanceNames[i].getKeyBindings();

            for (Uint32 j = 0; j < kbArray.size(); j++)
            {
                if (kbArray[j].getName().equal (PROPERTY_NAME_USERNAME))
                {
                    user = kbArray[j].getValue();
                }
            }

            if ( String::equal(user, _userName) )
            {
                _client->deleteInstance(
                    PEGASUS_NAMESPACENAME_AUTHORIZATION, instanceNames[i]);
            }
        }
    }

    outPrintWriter << localizeMessage(MSG_PATH,
        REMOVE_AUTH_SUCCESS_KEY,
        REMOVE_AUTH_SUCCESS)
        << endl;
}


/**
    get a list of all user names from the CIM Server.
 */
void CIMAuthCommand::_ListAuthorization
    (
    ostream& outPrintWriter,
    ostream& errPrintWriter
    )
{
    Array<CIMInstance> authNamedInstances;

    //
    // get all the instances of class PG_Authorization
    //
    authNamedInstances =
        _client->enumerateInstances(
            PEGASUS_NAMESPACENAME_AUTHORIZATION,
            PEGASUS_CLASSNAME_AUTHORIZATION);

    //
    // display all the user names, namespaces, and authorizations
    //
    for (Uint32 i = 0; i < authNamedInstances.size(); i++)
    {
        CIMInstance& authInstance =
            authNamedInstances[i];

        //
        // get user name
        //
        Uint32 pos = authInstance.findProperty(PROPERTY_NAME_USERNAME);
        CIMProperty prop = (CIMProperty)authInstance.getProperty(pos);
        String name = prop.getValue().toString();

        //
        // get namespace name
        //
        pos = authInstance.findProperty(PROPERTY_NAME_NAMESPACE);
        prop = (CIMProperty)authInstance.getProperty(pos);
        String ns = prop.getValue().toString();

        //
        // get authorizations
        //
        pos = authInstance.findProperty(PROPERTY_NAME_AUTHORIZATION);
        prop = (CIMProperty)authInstance.getProperty(pos);
        String auth = prop.getValue().toString();

        outPrintWriter << name << ", " << ns << ", \"" ;
        outPrintWriter << auth << "\"" << endl;
    }
}

PEGASUS_NAMESPACE_END

//
// exclude main from the Pegasus Namespace
//
PEGASUS_USING_PEGASUS;

PEGASUS_USING_STD;

///////////////////////////////////////////////////////////////////////////////
/**
    Parses the command line, and execute the command.

    @param   args  the string array containing the command line arguments
*/
///////////////////////////////////////////////////////////////////////////////

int main (int argc, char* argv [])
{
    AutoPtr<CIMAuthCommand> command;
    Uint32               retCode;
    // l10n set messageloading to process locale
    MessageLoader::_useProcessLocale = true;
    MessageLoader::setPegasusMsgHomeRelative(argv[0]);

    command.reset(new CIMAuthCommand ());

    try
    {
        command->setCommand (argc, argv);
    }
    catch (CommandFormatException& cfe)
    {
        cerr << COMMAND_NAME << ": " << cfe.getMessage() <<  endl;

        MessageLoaderParms parms(ERR_USAGE_KEY,ERR_USAGE);
        parms.msg_src_path = MSG_PATH;
        cerr << COMMAND_NAME <<
            ": " << MessageLoader::getMessage(parms) << endl;
        exit (Command::RC_ERROR);
    }

    retCode = command->execute (cout, cerr);

    exit (retCode);
    return 0;
}

