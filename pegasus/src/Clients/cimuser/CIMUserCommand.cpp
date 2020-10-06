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
#include <iostream>
#include <Clients/cliutils/Command.h>
#include <Clients/cliutils/CommandException.h>
#include <Pegasus/getoopt/getoopt.h>
#include <Pegasus/Client/CIMClient.h>
#include <Pegasus/Common/CIMProperty.h>
#include <Pegasus/Common/CIMObjectPath.h>
#include <Pegasus/Common/CIMStatusCode.h>
#include <Pegasus/Common/String.h>
#include <Pegasus/Common/System.h>
#include <Pegasus/Common/PegasusVersion.h>
#include <Pegasus/Common/AutoPtr.h>

PEGASUS_USING_STD;

PEGASUS_NAMESPACE_BEGIN

//l10n
/**
 * The CLI message resource name
 */

static const char MSG_PATH [] = "pegasus/pegasusCLI";

/**
    The command name.
*/
static const char COMMAND_NAME []              = "cimuser";

/**
    This constant represents the name of the User name property in the schema
*/
static const CIMName PROPERTY_NAME_USER_NAME            = CIMName ("Username");

/**
    This constant represents the name of the Password property in the schema
*/
static const CIMName PROPERTY_NAME_PASSWORD             = CIMName ("Password");

/**
    The usage string for this command.  This string is displayed
    when an error occurs in parsing or validating the command line.
*/
static const char USAGE []                     = "Usage: ";

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

//l10n

static const char CIMOM_NOT_RUNNING [] =
    "CIM Server may not be running.";

static const char CIMOM_NOT_RUNNING_KEY [] =
    "Clients.cimuser.CIMUserCommand.CIMOM_NOT_RUNNING";

static const char ADD_USER_FAILURE [] =
    "Failed to add user.";

static const char ADD_USER_FAILURE_KEY [] =
    "Clients.cimuser.CIMUserCommand.ADD_USER_FAILURE";

static const char REMOVE_USER_FAILURE [] =
    "Failed to remove user.";

static const char REMOVE_USER_FAILURE_KEY [] =
    "Clients.cimuser.CIMUserCommand.REMOVE_USER_FAILURE";

static const char CHANGE_PASSWORD_FAILURE [] =
    "Failed to change password.";

static const char CHANGE_PASSWORD_FAILURE_KEY [] =
    "Clients.cimuser.CIMUserCommand.CHANGE_PASSWORD_FAILURE";

static const char LIST_USERS_FAILURE [] =
    "Failed to list the users. ";

static const char LIST_USERS_FAILURE_KEY [] =
    "Clients.cimuser.CIMUserCommand.LIST_USERS_FAILURE";

static const char ADD_USER_SUCCESS [] =
    "User added successfully.";

static const char ADD_USER_SUCCESS_KEY [] =
    "Clients.cimuser.CIMUserCommand.ADD_USER_SUCCESS";

static const char REMOVE_USER_SUCCESS[] =
    "User removed successfully.";

static const char REMOVE_USER_SUCCESS_KEY[] =
    "Clients.cimuser.CIMUserCommand.REMOVE_USER_SUCCESS";

static const char CHANGE_PASSWORD_SUCCESS [] =
    "Password changed successfully.";

static const char CHANGE_PASSWORD_SUCCESS_KEY [] =
    "Clients.cimuser.CIMUserCommand.CHANGE_PASSWORD_SUCCESS";

static const char PASSWORD_BLANK [] =
    "Password cannot be blank. Please re-enter your password.";

static const char PASSWORD_BLANK_KEY [] =
    "Clients.cimuser.CIMUserCommand.PASSWORD_BLANK";

static const char NO_USERS_FOUND[] =
    "No users found for listing.";

static const char NO_USERS_FOUND_KEY[] =
    "Clients.cimuser.CIMUserCommand.NO_USERS_FOUND";

static const char AUTH_SCHEMA_NOT_LOADED [] =
    "Please restore the internal repository on the CIM Server.";

static const char AUTH_SCHEMA_NOT_LOADED_KEY [] =
    "Clients.cimuser.CIMUserCommand.AUTH_SCHEMA_NOT_LOADED";

static const char REQUIRED_ARGS_MISSING [] =
    "Required arguments missing.";

static const char REQUIRED_ARGS_MISSING_KEY [] =
    "Clients.cimuser.CIMUserCommand.REQUIRED_ARGS_MISSING";

static const char USER_ALREADY_EXISTS [] =
    "Specified user name already exist.";

static const char USER_ALREADY_EXISTS_KEY [] =
    "Clients.cimuser.CIMUserCommand.USER_ALREADY_EXISTS";

static const char USER_NOT_FOUND [] =
    "Specified user name was not found.";

static const char USER_NOT_FOUND_KEY [] =
    "Clients.cimuser.CIMUserCommand.USER_NOT_FOUND";

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
static const char   OPTION_PASSWORD            = 'w';

/**
    The option character used to specify new password.
*/
static const char   OPTION_NEW_PASSWORD        = 'n';

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
    The name of the Method that implements modify password
*/
static const CIMName   MODIFY_METHOD            = CIMName ("modifyPassword");

/**
    The input parameter name for old password
*/
static const char   OLD_PASS_PARAM[]             = "OldPassword";

/**
    The input parameter name for new password
*/
static const char   NEW_PASS_PARAM[]             = "NewPassword";


static const char   PASSWORD_PROMPT []  =
                        "Please enter your password: ";

static const char   OLD_PASSWORD_PROMPT []  =
    "Please enter your old password: ";

static const char   RE_ENTER_PROMPT []  =
    "Please re-enter your password: ";

static const char   NEW_PASSWORD_PROMPT []  =
    "Please enter your new password: ";

static const char   PASSWORD_DOES_NOT_MATCH []  =
    "Passwords do not match. Please Re-enter.";

static const char   PASSWORD_DOES_NOT_MATCH_KEY []  =
    "Clients.cimuser.CIMUserCommand.PASSWORD_DOES_NOT_MATCH";

static const char   PASSWORD_SAME_ERROR []  =
    "Error, new and old passwords cannot be same.";

static const char   PASSWORD_SAME_ERROR_KEY []  =
    "Clients.cimuser.CIMUserCommand.PASSWORD_SAME_ERROR";

static const char ERR_USAGE [] =
    "Use '--help' to obtain command syntax.";

static const char ERR_USAGE_KEY [] =
    "Clients.cimuser.CIMUserCommand.ERR_USAGE";

static const char   LONG_HELP []  = "help";

static const char   LONG_VERSION []  = "version";
/**
This is a CLI used to manage users of the CIM Server.  This command supports
operations to add, modify, list and remove users.
*/

class CIMUserCommand : public Command
{

public:

    /**
        Constructs a CIMUserCommand and initializes instance variables.
    */
    CIMUserCommand ();


    //
    // Overrides the virtual function setCommand from Command class
    // This is defined as an empty function.
    //
    void setCommand (Uint32, char**)
    {
        // Empty function
    }

    /**
    Parses the command line, validates the options, and sets instance
    variables based on the option arguments. This implementation of
    setCommand includes the parameters for output and error stream.

    @param  ostream    The stream to which command output is written.
    @param  ostream    The stream to which command errors are written.
    @param  args       The string array containing the command line arguments
    @param  argc       The int containing the arguments count

    @throws  CommandFormatException  if an error is encountered in parsing
                                     the command line
    */
    void setCommand (
      ostream& outPrintWriter,
      ostream& errPrintWriter,
      Uint32 argc,
      char* argv[]);

    /**
    Executes the command and writes the results to the output streams.

    @param ostream    The stream to which command output is written.
    @param ostream    The stream to which command errors are written.
    @return  0        if the command is successful
             1        if an error occurs in executing the command
    */
    Uint32 execute (
        ostream& outPrintWriter,
        ostream& errPrintWriter);


private:

    //
    // Add a new user to the CIM Server
    //
    // @param ostream        The stream to which command output is written.
    // @param ostream        The stream to which command errors are written.
    //
    // @exception Exception  if failed to add user
    //
    void _AddUser
        (
        ostream& outPrintWriter,
        ostream& errPrintWriter
        );

    //
    // Modify an existing user's password.
    //
    // @param ostream          The stream to which command output is written.
    // @param ostream          The stream to which command errors are written.
    //
    // @exception Exception  if failed to modify password
    //
    void _ModifyUser
        (
            ostream&                 outPrintWriter,
            ostream&                 errPrintWriter
        );

    //
    // Remove an existing user from the CIM Server
    //
    // @param ostream          The stream to which command output is written.
    // @param ostream          The stream to which command errors are written.
    //
    // @exception Exception  if failed to remove user
    //
    void _RemoveUser
        (
            ostream& outPrintWriter,
            ostream& errPrintWriter
        );

    //
    // List all users.
    //
    // @param ostream        The stream to which command output is written.
    // @param ostream        The stream to which command errors are written.
    //
    void _ListUsers
    (
        ostream& outPrintWriter,
        ostream& errPrintWriter
    );

    //
    // The CIM Client reference
    //

    AutoPtr<CIMClient> _client; //PEP101

    //
    // The host name.
    //
    String _hostName;

    //
    // The name of the user.
    //
    String _userName;

    //
    // The password of the user.
    //
    String _password;

    //
    // The new password of the user.
    //
    String _newpassword;

    //
    // The type of operation specified on the command line.
    //
    Uint32 _operationType;

    //
    // Flags for command options
    //
    Boolean _userNameSet;
    Boolean _passwordSet;
    Boolean _newpasswordSet;

    String usage;

};

/**
    Constructs a CIMUserCommand and initializes instance variables.
*/
CIMUserCommand::CIMUserCommand ()
{
    /**
        Initialize the instance variables.
    */
    _operationType       = OPERATION_TYPE_UNINITIALIZED;
    _passwordSet         = false;
    _newpasswordSet      = false;
    _userNameSet         = false;

    /**
        Build the usage string for the config command.
    */
    usage.reserveCapacity(200);
    usage.append(USAGE);
    usage.append(COMMAND_NAME);

    usage.append(" -").append(OPTION_ADD);
    usage.append(" -").append(OPTION_USER_NAME).append(" username");
    usage.append(" [ -").append(OPTION_PASSWORD).append(" password")
         .append(" ] \n");

    usage.append("               -").append(OPTION_MODIFY);
    usage.append(" -").append(OPTION_USER_NAME).append(" username");
    usage.append(" [ -").append(OPTION_PASSWORD).append(" old password")
         .append(" ]");
    usage.append(" [ -").append(OPTION_NEW_PASSWORD).append(" new password")
         .append(" ] \n");

    usage.append("               -").append(OPTION_REMOVE);
    usage.append(" -").append(OPTION_USER_NAME).append(" username \n");

    usage.append("               -").append(OPTION_LIST).append(" \n");
    usage.append("               -").append(OPTION_HELP).append(" \n");
    usage.append("               --").append(LONG_HELP).append(" \n");
    usage.append("               --").append(LONG_VERSION).append(" \n");


    usage.append("Options : \n");
    usage.append("    -a         - Add a new CIM user\n");
    usage.append("    -h, --help - Display this help message\n");
    usage.append("    -l         - List the names of CIM users\n");
    usage.append("    -m         - Modify a CIM user's password\n");
    usage.append("    -n         - Supply a new password for the specified"
                    " user name\n");
    usage.append("    -r         - Remove the specified CIM user\n");
    usage.append("    -u         - Specify a CIM user name\n");
    usage.append("    --version  - Display CIM Server version number\n");
    usage.append("    -w         - Supply a password for the specified user"
                    " name\n");

    usage.append("\nUsage note: The cimuser command requires that the"
                    " CIM Server is running.\n");
    setUsage (usage);
}

/**
    Parses the command line, validates the options, and sets instance
    variables based on the option arguments.
*/
void CIMUserCommand::setCommand (
    ostream& outPrintWriter,
    ostream& errPrintWriter,
    Uint32 argc,
    char* argv[])
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
    optString.append(OPTION_PASSWORD);
    optString.append(getoopt::GETOPT_ARGUMENT_DESIGNATOR);
    optString.append(OPTION_MODIFY);
    optString.append(OPTION_USER_NAME);
    optString.append(getoopt::GETOPT_ARGUMENT_DESIGNATOR);
    optString.append(OPTION_PASSWORD);
    optString.append(getoopt::GETOPT_ARGUMENT_DESIGNATOR);
    optString.append(OPTION_NEW_PASSWORD);
    optString.append(getoopt::GETOPT_ARGUMENT_DESIGNATOR);
    optString.append(OPTION_REMOVE);
    optString.append(OPTION_USER_NAME);
    optString.append(getoopt::GETOPT_ARGUMENT_DESIGNATOR);
    optString.append(OPTION_LIST);
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
            //  The cimuser command has no non-option argument options
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
                case OPTION_PASSWORD:
                {
                    if (options.isSet (OPTION_PASSWORD) > 1)
                    {
                        //
                        // More than one password option was found
                        //
                        throw DuplicateOptionException(OPTION_PASSWORD);
                    }

                    String password = options [i].Value ();
                    _password = password.subString(0,8);

                    _passwordSet = true;

                    break;
                }
                case OPTION_NEW_PASSWORD:
                {
                    if (options.isSet (OPTION_NEW_PASSWORD) > 1)
                    {
                        //
                        // More than one new password option was found
                        //
                        throw DuplicateOptionException(OPTION_NEW_PASSWORD);
                    }

                    String newpassword = options [i].Value ();
                    _newpassword = newpassword.subString(0,8);

                    _newpasswordSet = true;

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

    //
    // Some more validations
    //
    if (_operationType == OPERATION_TYPE_UNINITIALIZED)
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

    if (_operationType == OPERATION_TYPE_LIST &&
        (_userNameSet || _passwordSet || _newpasswordSet))
    {
        MessageLoaderParms parms(
            "Clients.cimuser.CIMUserCommand.UNEXPECTED_OPTION",
            "Unexpected Option.");
        //CommandFormatException e("Unexpected Option.");
        throw CommandFormatException(parms);
    }

    if (_operationType == OPERATION_TYPE_ADD)
    {
        if (_newpasswordSet)
        {
            //
            // An invalid option was encountered
            //
            throw UnexpectedOptionException(OPTION_NEW_PASSWORD);
        }
        if (!_userNameSet)
        {
            //
            // An invalid option was encountered
            //
            throw MissingOptionException(OPTION_USER_NAME);
        }
        if (!_passwordSet)
        {
            //
            // Password is not set, prompt for the password
            //
            String pw;
            do
            {
                pw = System::getPassword(PASSWORD_PROMPT);

                if (pw == String::EMPTY)
                {
                    errPrintWriter << localizeMessage(MSG_PATH,
                                                      PASSWORD_BLANK_KEY,
                                                      PASSWORD_BLANK)
                                   << endl;
                    continue;
                }
                if (pw != System::getPassword(RE_ENTER_PROMPT))
                {
                    errPrintWriter << localizeMessage(MSG_PATH,
                                                 PASSWORD_DOES_NOT_MATCH_KEY,
                                                 PASSWORD_DOES_NOT_MATCH_KEY)
                                   << endl;
                    pw = String::EMPTY;
                }
            }
            while (pw == String::EMPTY);

            _password = pw ;
        }
    }

    if (_operationType == OPERATION_TYPE_MODIFY)
    {
        if (!_userNameSet)
        {
            //
            // An invalid option was encountered
            //
            throw MissingOptionException(OPTION_USER_NAME);
        }
        if (_passwordSet && _newpasswordSet)
        {
            if (_newpassword == _password)
            {
                cerr << localizeMessage(MSG_PATH,
                                        PASSWORD_SAME_ERROR_KEY,
                                        PASSWORD_SAME_ERROR)
                    << endl;
                exit (1);
            }
        }
        if (!_passwordSet)
        {
            //
            // Password is not set, prompt for the old password once
            //
            String pw;
            do
            {
                pw = System::getPassword(OLD_PASSWORD_PROMPT);
                if (pw == String::EMPTY)
                {
                    errPrintWriter << localizeMessage(MSG_PATH,
                                                      PASSWORD_BLANK_KEY,
                                                      PASSWORD_BLANK)
                                   << endl;
                    continue;
                }
            }
            while (pw == String::EMPTY);
            _password = pw ;
        }
        if (!_newpasswordSet)
        {
            //
            // Password is not set, prompt for the new password twice
            //
            String newPw;
            do
            {
                newPw = System::getPassword(NEW_PASSWORD_PROMPT);
                if (newPw == String::EMPTY)
                {
                    errPrintWriter << localizeMessage(
                        MSG_PATH, PASSWORD_BLANK_KEY, PASSWORD_BLANK) << endl;
                    continue;
                }

                if (newPw != System::getPassword(RE_ENTER_PROMPT))
                {
                    errPrintWriter << localizeMessage(MSG_PATH,
                                                    PASSWORD_DOES_NOT_MATCH_KEY,
                                                    PASSWORD_DOES_NOT_MATCH)
                                   << endl;
                    newPw = String::EMPTY;
                }
            }
            while (newPw == String::EMPTY);
            _newpassword = newPw ;
            if (_newpassword == _password)
            {
                cerr << localizeMessage(MSG_PATH,
                                        PASSWORD_SAME_ERROR_KEY,
                                        PASSWORD_SAME_ERROR)
                    << endl;
                exit (-1);
            }
        }
    }

    if (_operationType == OPERATION_TYPE_REMOVE)
    {
        if (!_userNameSet)
        {
            //
            // An invalid option was encountered
            //
            throw MissingOptionException(OPTION_USER_NAME);
        }
        if (_passwordSet)
        {
            //
            // An invalid option was encountered
            //
            throw UnexpectedOptionException(OPTION_PASSWORD);
        }
        if (_newpasswordSet)
        {
            //
            // An invalid option was encountered
            //
            throw UnexpectedOptionException(OPTION_NEW_PASSWORD);
        }
    }
}

/**
    Executes the command and writes the results to the PrintWriters.
*/
Uint32 CIMUserCommand::execute (
    ostream& outPrintWriter,
    ostream& errPrintWriter)
{
    if (_operationType == OPERATION_TYPE_UNINITIALIZED)
    {
        //
        // The command was not initialized
        //
        return 1;
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
        return (RC_ERROR);
    }

    try
    {
        //
        // Open connection with CIMSever
        //
        _client->connectLocal();
    }
    catch(const Exception&)
    {
        outPrintWriter << localizeMessage(MSG_PATH,CIMOM_NOT_RUNNING_KEY,
                                          CIMOM_NOT_RUNNING)
                      << endl;
        return 1;
    }

    //
    // Perform the requested operation
    //
    switch (_operationType)
    {
        case OPERATION_TYPE_ADD:
            try
            {
                _AddUser(outPrintWriter, errPrintWriter);
            }
            catch (CIMException& e)
            {
                CIMStatusCode code = e.getCode();

                if (code == CIM_ERR_FAILED)
                {
                    outPrintWriter
                        << localizeMessage(MSG_PATH, ADD_USER_FAILURE_KEY,
                                           ADD_USER_FAILURE)
                        << endl;
                    errPrintWriter << e.getMessage() << endl;
                }
                else if (code == CIM_ERR_NOT_SUPPORTED)
                {
                    outPrintWriter
                        << localizeMessage(MSG_PATH, ADD_USER_FAILURE_KEY,
                                           ADD_USER_FAILURE)
                        << endl;
                    errPrintWriter << e.getMessage()  << endl;
                }
                else if (code == CIM_ERR_ALREADY_EXISTS)
                {
                    outPrintWriter << localizeMessage(MSG_PATH,
                                                    ADD_USER_FAILURE_KEY,
                                                    ADD_USER_FAILURE)
                        << endl;
                    outPrintWriter << localizeMessage(MSG_PATH,
                                                    USER_ALREADY_EXISTS_KEY,
                                                    USER_ALREADY_EXISTS)
                        << endl;
                    errPrintWriter << e.getMessage()  << endl;
                }
                else if (code == CIM_ERR_INVALID_CLASS)
                {
                    outPrintWriter << localizeMessage(MSG_PATH,
                                                      ADD_USER_FAILURE_KEY,
                                                      ADD_USER_FAILURE)
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
                return (RC_ERROR);
            }
            catch (Exception& e)
            {
                outPrintWriter << localizeMessage(MSG_PATH,
                                                  ADD_USER_FAILURE_KEY,
                                                  ADD_USER_FAILURE)
                    << endl << e.getMessage() << endl;
                return (RC_ERROR);
            }
            break;

            case OPERATION_TYPE_MODIFY:
            try
            {
                _ModifyUser(outPrintWriter, errPrintWriter);
            }
            catch (CIMException& e)
            {
                CIMStatusCode code = e.getCode();
                if (code == CIM_ERR_FAILED)
                {
                    outPrintWriter << localizeMessage(MSG_PATH,
                                                CHANGE_PASSWORD_FAILURE_KEY,
                                                CHANGE_PASSWORD_FAILURE)
                                   << endl;
                    errPrintWriter << e.getMessage() << endl;
                }
                else if (code == CIM_ERR_NOT_SUPPORTED)
                {
                    outPrintWriter << localizeMessage(MSG_PATH,
                                                    CHANGE_PASSWORD_FAILURE_KEY,
                                                    CHANGE_PASSWORD_FAILURE)
                                  << endl;
                    errPrintWriter << e.getMessage()  << endl;
                }
                else if (code == CIM_ERR_NOT_FOUND)
                {
                    outPrintWriter << localizeMessage(MSG_PATH,
                            CHANGE_PASSWORD_FAILURE_KEY,
                            CHANGE_PASSWORD_FAILURE) << endl;
                    outPrintWriter << localizeMessage(MSG_PATH,
                            USER_NOT_FOUND_KEY,USER_NOT_FOUND) << endl;
                    errPrintWriter << e.getMessage()  << endl;
                }
                else if (code == CIM_ERR_INVALID_CLASS)
                {
                    outPrintWriter << localizeMessage(MSG_PATH,
                            CHANGE_PASSWORD_FAILURE_KEY,
                            CHANGE_PASSWORD_FAILURE) << endl;
                    outPrintWriter << localizeMessage(MSG_PATH,
                            AUTH_SCHEMA_NOT_LOADED_KEY,
                            AUTH_SCHEMA_NOT_LOADED) << endl;
                }
                else
                {
                    errPrintWriter << e.getMessage() << endl;
                }
                return (RC_ERROR);
            }
            catch (Exception& e)
            {
                outPrintWriter << localizeMessage(MSG_PATH,
                        CHANGE_PASSWORD_FAILURE_KEY,
                        CHANGE_PASSWORD_FAILURE) <<
                    endl << e.getMessage() << endl;
                return (RC_ERROR);
            }
            break;

        case OPERATION_TYPE_REMOVE:
            try
            {
                _RemoveUser(outPrintWriter, errPrintWriter);
            }
            catch (CIMException& e)
            {
                CIMStatusCode code = e.getCode();
                if (code == CIM_ERR_FAILED)
                {
                    outPrintWriter << localizeMessage(MSG_PATH,
                            REMOVE_USER_FAILURE_KEY,
                            REMOVE_USER_FAILURE) << endl;
                    errPrintWriter << e.getMessage() << endl;
                }
                else if (code == CIM_ERR_NOT_SUPPORTED)
                {
                    outPrintWriter << localizeMessage(MSG_PATH,
                            REMOVE_USER_FAILURE_KEY,
                            REMOVE_USER_FAILURE) << endl;
                    errPrintWriter << e.getMessage()  << endl;
                }
                else if (code == CIM_ERR_NOT_FOUND)
                {
                    outPrintWriter << localizeMessage(MSG_PATH,
                            REMOVE_USER_FAILURE_KEY,
                            REMOVE_USER_FAILURE) << endl;
                    outPrintWriter << localizeMessage(MSG_PATH,
                            USER_NOT_FOUND_KEY,USER_NOT_FOUND) << endl;
                    errPrintWriter << e.getMessage()  << endl;
                }
                else if (code == CIM_ERR_INVALID_CLASS)
                {
                    outPrintWriter << localizeMessage(MSG_PATH,
                            REMOVE_USER_FAILURE_KEY,
                            REMOVE_USER_FAILURE) << endl;
                    outPrintWriter << localizeMessage(MSG_PATH,
                            AUTH_SCHEMA_NOT_LOADED_KEY,
                            AUTH_SCHEMA_NOT_LOADED) << endl;
                }
                else
                {
                    errPrintWriter << e.getMessage() << endl;
                }
                return (RC_ERROR);
            }
            catch (Exception& e)
            {
                outPrintWriter << localizeMessage(MSG_PATH,
                        REMOVE_USER_FAILURE_KEY,REMOVE_USER_FAILURE) <<
                endl << e.getMessage() << endl;
                return (RC_ERROR);
            }
            break;

        case OPERATION_TYPE_LIST:
            try
            {
                _ListUsers(outPrintWriter, errPrintWriter);
            }
            catch (CIMException& e)
            {
                CIMStatusCode code = e.getCode();
                if (code == CIM_ERR_FAILED)
                {
                    outPrintWriter << localizeMessage(MSG_PATH,
                            LIST_USERS_FAILURE_KEY,LIST_USERS_FAILURE) << endl;
                    errPrintWriter << e.getMessage() << endl;
                }
                else if (code == CIM_ERR_NOT_SUPPORTED)
                {
                    outPrintWriter << localizeMessage(MSG_PATH,
                            LIST_USERS_FAILURE_KEY,LIST_USERS_FAILURE) << endl;
                    errPrintWriter << e.getMessage()  << endl;
                }
                else if (code == CIM_ERR_ALREADY_EXISTS)
                {
                    outPrintWriter << localizeMessage(MSG_PATH,
                            LIST_USERS_FAILURE_KEY,LIST_USERS_FAILURE) << endl;
                    errPrintWriter << e.getMessage()  << endl;
                }
                else if (code == CIM_ERR_INVALID_CLASS)
                {
                    outPrintWriter << localizeMessage(MSG_PATH,
                            LIST_USERS_FAILURE_KEY,LIST_USERS_FAILURE) << endl;
                    outPrintWriter << localizeMessage(MSG_PATH,
                            AUTH_SCHEMA_NOT_LOADED_KEY,
                            AUTH_SCHEMA_NOT_LOADED) << endl;
                }
                else
                {
                    errPrintWriter << e.getMessage() << endl;
                }
                return (RC_ERROR);
            }
            catch (Exception& e)
            {
                outPrintWriter << localizeMessage(MSG_PATH,
                        LIST_USERS_FAILURE_KEY,LIST_USERS_FAILURE) <<
                    endl << e.getMessage() << endl;
                return (RC_ERROR);
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
void CIMUserCommand::_AddUser
    (
    ostream& outPrintWriter,
    ostream& errPrintWriter
    )
{
    CIMProperty prop;

    CIMInstance newInstance(PEGASUS_CLASSNAME_USER);
    newInstance.addProperty(
        CIMProperty(PROPERTY_NAME_USER_NAME, _userName));
    newInstance.addProperty(
        CIMProperty(PROPERTY_NAME_PASSWORD , _password));

    _client->createInstance(PEGASUS_NAMESPACENAME_USER, newInstance);
    outPrintWriter <<localizeMessage(MSG_PATH,
            ADD_USER_SUCCESS_KEY,ADD_USER_SUCCESS) << endl;
}

//
// Modify the password for a user
//
void CIMUserCommand::_ModifyUser
    (
    ostream& outPrintWriter,
    ostream& errPrintWriter
    )
{
    Array<CIMKeyBinding>      kbArray;
    CIMKeyBinding             kb;
    Array<CIMParamValue>   inParams;
    Array<CIMParamValue>   outParams;

    //
    // Build the input params
    //
    inParams.append(CIMParamValue(OLD_PASS_PARAM,
        CIMValue(_password)));
    inParams.append(CIMParamValue(NEW_PASS_PARAM,
        CIMValue(_newpassword)));

    kb.setName(CIMName ("Username"));
    kb.setValue(_userName);
    kb.setType(CIMKeyBinding::STRING);

    kbArray.append(kb);

    CIMObjectPath reference(
        _hostName, PEGASUS_NAMESPACENAME_USER,
        PEGASUS_CLASSNAME_USER, kbArray);

    //
    // Call the invokeMethod with the input parameters
    //

    //
    // Not checking for return code as all error conditions will
    // throw exceptions and will be handled by the catch block. If new
    // return codes are added in future, they need to be handled here.
    //
    _client->invokeMethod(
        PEGASUS_NAMESPACENAME_USER,
        reference,
        MODIFY_METHOD,
        inParams,
        outParams);
    outPrintWriter << localizeMessage(MSG_PATH,
            CHANGE_PASSWORD_SUCCESS_KEY,CHANGE_PASSWORD_SUCCESS) << endl;
}

//
// Remove a user
//
void CIMUserCommand::_RemoveUser
    (
    ostream& outPrintWriter,
    ostream& errPrintWriter
    )
{
    Array<CIMKeyBinding> kbArray;
    CIMKeyBinding kb;

    kb.setName(PROPERTY_NAME_USER_NAME);
    kb.setValue(_userName);
    kb.setType(CIMKeyBinding::STRING);

    kbArray.append(kb);

    CIMObjectPath reference(
        _hostName, PEGASUS_NAMESPACENAME_USER,
        PEGASUS_CLASSNAME_USER, kbArray);

    _client->deleteInstance(PEGASUS_NAMESPACENAME_USER, reference);
    outPrintWriter << localizeMessage(MSG_PATH,
            REMOVE_USER_SUCCESS_KEY,REMOVE_USER_SUCCESS) << endl;
}


/**
    get a list of all user names from the CIM Server.
 */
void CIMUserCommand::_ListUsers
    (
    ostream& outPrintWriter,
    ostream& errPrintWriter
    )
{
    //
    // get all the instances of class PG_User
    //
    Array<CIMObjectPath> instanceNames =
        _client->enumerateInstanceNames(
            PEGASUS_NAMESPACENAME_USER,
            PEGASUS_CLASSNAME_USER);

    if (instanceNames.size() == 0)
    {
         outPrintWriter <<
             localizeMessage(MSG_PATH,NO_USERS_FOUND_KEY,NO_USERS_FOUND) <<
             endl;
    }
    else
    {
        //
        // List all the users.
        //
        for (Uint32 i = 0; i < instanceNames.size(); i++)
        {
            Array<CIMKeyBinding> kbArray = instanceNames[i].getKeyBindings();

            if (kbArray.size() > 0)
            {
                outPrintWriter << kbArray[0].getValue() << endl;
            }
        }
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

int main (int argc, char* argv[])
{
    AutoPtr<CIMUserCommand>  command;
    Uint32               retCode;

    MessageLoader::_useProcessLocale = true;
    MessageLoader::setPegasusMsgHomeRelative(argv[0]);

    command.reset(new CIMUserCommand ());

    try
    {
        command->setCommand ( cout, cerr, argc, argv);
    }
    catch (CommandFormatException& cfe)
    {
        cerr << COMMAND_NAME << ": " << cfe.getMessage() << endl;

        MessageLoaderParms parms(ERR_USAGE_KEY,ERR_USAGE);
        parms.msg_src_path = MSG_PATH;
        cerr << COMMAND_NAME <<
            ": " << MessageLoader::getMessage(parms) << endl;

        return 1;
    }

    retCode = command->execute (cout, cerr);


    return (retCode);
}

