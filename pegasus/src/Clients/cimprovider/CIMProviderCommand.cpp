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

#include <iostream>

#include <Clients/cliutils/Command.h>
#include <Clients/cliutils/CommandException.h>

#include <Pegasus/getoopt/getoopt.h>

#include <Pegasus/Client/CIMClient.h>
#include <Pegasus/Common/CIMProperty.h>
#include <Pegasus/Common/CIMObjectPath.h>
#include <Pegasus/Common/CIMStatusCode.h>
#include <Pegasus/Common/Exception.h>
#include <Pegasus/Common/String.h>
#include <Pegasus/Common/System.h>
#include <Pegasus/Common/Constants.h>
#include <Pegasus/Common/PegasusVersion.h>
#include <Pegasus/Common/AutoPtr.h>
#include <Pegasus/Common/PegasusAssert.h>

#ifdef PEGASUS_OS_ZOS
#include <Pegasus/General/SetFileDescriptorToEBCDICEncoding.h>
#endif

#define CIMPROVIDERCOMMAND_CLIENT_DEFAULTTIMEOUT 120000

PEGASUS_USING_STD;

PEGASUS_NAMESPACE_BEGIN

/**
    The CLI message resource name
*/
static const char MSG_PATH[] = "pegasus/pegasusCLI";

/**
    The command name.
*/
static const char COMMAND_NAME[]              = "cimprovider";

/**
   The name of the provider module name  property for PG_Provider class
*/
static const CIMName _PROPERTY_PROVIDERMODULENAME =
    CIMName("ProviderModuleName");

/**
   The name of the Name property for PG_Provider class
*/
static const CIMName _PROPERTY_PROVIDER_NAME = CIMName("Name");

/**
    The usage string for this command.  This string is displayed
    when an error occurs in parsing or validating the command line.
*/
static const char USAGE[] = "Usage: ";

/*
    These constants represent the operation modes supported by the CLI.
    Any new operation should be added here.
*/

/**
    This constant signifies that an operation option has not been recorded
*/
static const Uint32 OPERATION_TYPE_UNINITIALIZED = 0;

/**
    This constant represents a disable provider operation
*/
static const Uint32 OPERATION_TYPE_DISABLE = 1;

/**
    This constant represents a remove provider operation
*/
static const Uint32 OPERATION_TYPE_REMOVE  = 2;

/**
    This constant represents a enable provider operation
*/
static const Uint32 OPERATION_TYPE_ENABLE  = 3;

/**
    This constant represents a list operation
*/
static const Uint32 OPERATION_TYPE_LIST    = 4;

/**
    This constant represents a help operation
*/
static const Uint32 OPERATION_TYPE_HELP    = 5;

/**
    This constant represents a version display operation
*/
static const Uint32 OPERATION_TYPE_VERSION = 6;

/**
    This constant represents provider module group set operation.
*/
static const Uint32 OPERATION_TYPE_GROUP = 7;


/**
    The constants representing the messages.
*/

static const char CIMOM_NOT_RUNNING[] =
    "The CIM server may not be running.";

static const char CIMOM_NOT_RUNNING_KEY[] =
    "Clients.cimprovider.CIMProviderCommand.CIMOM_NOT_RUNNING";

static const char DELETE_PROVIDER_FAILURE[] =
    "Failed to delete...";

static const char DELETE_PROVIDER_FAILURE_KEY[] =
    "Clients.cimprovider.CIMProviderCommand.DELETE_PROVIDER_FAILURE";


static const char START_PROVIDER_FAILURE[] =
    "Failed to enable provider module.";

static const char START_PROVIDER_FAILURE_KEY[] =
    "Clients.cimprovider.CIMProviderCommand.START_PROVIDER_FAILURE";

static const char STOP_PROVIDER_FAILURE[] =
    "Failed to disable provider module.";

static const char STOP_PROVIDER_FAILURE_KEY[] =
    "Clients.cimprovider.CIMProviderCommand.STOP_PROVIDER_FAILURE";

static const char PENDING_REQUESTS_STOP_PROVIDER_FAILURE_KEY[] =
    "Clients.cimprovider.CIMProviderCommand."
        "PENDING_REQUESTS_STOP_PROVIDER_FAILURE";

static const char PENDING_REQUESTS_STOP_PROVIDER_FAILURE[] =
    "Failed to disable provider module: Provider is busy.";

static const char LIST_PROVIDERS_FAILURE[] =
    "Failed to list... ";

static const char LIST_PROVIDERS_FAILURE_KEY[] =
    "Clients.cimprovider.CIMProviderCommand.LIST_PROVIDERS_FAILURE";

static const char DELETE_PROVIDER_MODULE_SUCCESS[] =
    "Provider module deleted successfully.";

static const char DELETE_PROVIDER_MODULE_SUCCESS_KEY[] =
    "Clients.cimprovider.CIMProviderCommand.DELETE_PROVIDER_SUCCESS";

static const char DELETE_PROVIDER_SUCCESS[] =
    "Provider deleted successfully.";

static const char DELETE_PROVIDER_SUCCESS_KEY[] =
    "Clients.cimprovider.CIMProviderCommand.DELETE_PROVIDER_SUCCESS";

static const char START_PROVIDER_SUCCESS[] =
    "Provider module enabled successfully.";

static const char START_PROVIDER_SUCCESS_KEY[] =
    "Clients.cimprovider.CIMProviderCommand.START_PROVIDER_SUCCESS";

static const char STOP_PROVIDER_SUCCESS[] =
    "Provider module disabled successfully.";

static const char STOP_PROVIDER_SUCCESS_KEY[] =
    "Clients.cimprovider.CIMProviderCommand.STOP_PROVIDER_SUCCESS";

static const char PROVIDER_ALREADY_STOPPED[] =
    "Provider module already disabled.";

static const char PROVIDER_ALREADY_STOPPED_KEY[] =
    "Clients.cimprovider.CIMProviderCommand.PROVIDER_ALREADY_STOPPED";

static const char PROVIDER_ALREADY_STARTED[] =
    "Provider module already enabled.";

static const char PROVIDER_ALREADY_STARTED_KEY[] =
    "Clients.cimprovider.CIMProviderCommand.PROVIDER_ALREADY_STARTED";

static const char CANNOT_START_PROVIDER[] =
    "Provider module can not be enabled since it is disabling.";

static const char CANNOT_START_PROVIDER_KEY[] =
    "Clients.cimprovider.CIMProviderCommand.CANNOT_START_PROVIDER";

static const char DELETEING_PROVIDER_MODULE[] =
    "Deleting provider module...";

static const char DELETEING_PROVIDER_MODULE_KEY[] =
    "Clients.cimprovider.CIMProviderCommand.DELETEING_PROVIDER_MODULE";

static const char DELETEING_PROVIDER[] =
    "Deleting provider...";

static const char DELETEING_PROVIDER_KEY[] =
    "Clients.cimprovider.CIMProviderCommand.DELETEING_PROVIDER";

static const char STARTING_PROVIDER_MODULE[] =
    "Enabling provider module...";

static const char STARTING_PROVIDER_MODULE_KEY[] =
    "Clients.cimprovider.CIMProviderCommand.STARTING_PROVIDER_MODULE";

static const char STOPING_PROVIDER_MODULE[] =
    "Disabling provider module...";

static const char STOPING_PROVIDER_MODULE_KEY[] =
    "Clients.cimprovider.CIMProviderCommand.STOPING_PROVIDER_MODULE";

static const char ERR_MODULE_NOT_REGISTERED[] =
    "Specified provider module was not registered.";

static const char ERR_MODULE_NOT_REGISTERED_KEY[] =
    "Clients.cimprovider.CIMProviderCommand.ERR_MODULE_NOT_REGISTERED";

static const char ERR_PROVIDER_NOT_REGISTERED[] =
    "Specified provider was not registered.";

static const char ERR_PROVIDER_NOT_REGISTERED_KEY[] =
    "Clients.cimprovider.CIMProviderCommand.ERR_PROVIDER_NOT_REGISTERED";

static const char PG_PROVIDER_SCHEMA_NOT_LOADED[] =
    "Please make sure that the registration schema is loaded on the CIMOM.";

static const char PG_PROVIDER_SCHEMA_NOT_LOADED_KEY[] =
    "Clients.cimprovider.CIMProviderCommand.PG_PROVIDER_SCHEMA_NOT_LOADED";

static const char REQUIRED_ARGS_MISSING[] =
    "Required arguments missing.";

static const char REQUIRED_ARGS_MISSING_KEY[] =
    "Clients.cimprovider.CIMProviderCommand.REQUIRED_ARGS_MISSING";

static const char UNEXPECTED_OPTION[] = "Unexpected Option.";

static const char UNEXPECTED_OPTION_KEY[] =
    "Clients.cimprovider.CIMProviderCommand.UNEXPECTED_OPTION";

static const char ERR_USAGE_KEY[] =
    "Clients.cimprovider.CIMProviderCommand.ERR_USAGE";

static const char ERR_USAGE[] =
    "Use '--help' to obtain command syntax.";

static const char SETTING_PROVIDERMODULE_GROUP_KEY[] =
    "Clients.cimprovider.CIMProviderCommand.SETTING_PROVIDERMODULE_GROUP";

static const char SETTING_PROVIDERMODULE_GROUP[] =
    "Setting the provider module group...";

static const char SET_PROVIDERMODULE_GROUP_SUCCESS_KEY[] =
    "Clients.cimprovider.CIMProviderCommand."
        "SET_PROVIDERMODULE_GROUP_SUCCESS";

static const char SET_PROVIDERMODULE_GROUP_SUCCESS[] =
    "Provider module group set successfully.";

static const char SET_PROVIDERMODULE_GROUP_FAILURE_KEY[] =
    "Clients.cimprovider.CIMProviderCommand."
        "SET_PROVIDERMODULE_GROUP_FAILURE";

static const char SET_PROVIDERMODULE_GROUP_FAILURE[] =
    "Failed to set the provider module group.";


static const char LONG_HELP[] = "help";

static const char LONG_VERSION[] = "version";

/**
    The option character used to specify disable a provider module.
*/
static const char OPTION_DISABLE     = 'd';

/**
    The option character used to specify remove a provider or module.
*/
static const char OPTION_REMOVE      = 'r';

/**
    The option character used to specify enable a provider module.
*/
static const char OPTION_ENABLE      = 'e';

/**
    The option character used to specify a module.
*/
static const char OPTION_MODULE      = 'm';

/**
    The option character used to specify a provider.
*/
static const char OPTION_PROVIDER    = 'p';

/**
    The option character used to specify listing of providers or provider
    modules.
*/
static const char OPTION_LIST        = 'l';

/**
    The option character used to specify get module status.
*/
static const char OPTION_STATUS      = 's';

/**
    The option character used to specify get full module status.
*/
static const char OPTION_FULLSTATUS      = 'f';

/**
    The option character used to display help info.
*/
static const char OPTION_HELP        = 'h';

/**
    The option character used to display version info.
*/
static const char OPTION_VERSION     = 'v';

/**
    The option character used to specify the module group name.
*/
static const char OPTION_GROUP     = 'g';

/**
    The name of the Method that implements stop provider or module
*/
static const CIMName STOP_METHOD = CIMName("stop");

/**
    The name of the Method that implements start provider or module
*/
static const CIMName START_METHOD = CIMName("start");

/**
    The name of the Method that implements the provider module group setting.
*/
static const CIMName SETMODULEGROUPNAME_METHOD = CIMName("SetModuleGroupName");

/**
    ModuleGroupName param or property.
*/
static const String MODULEGROUPNAME = "ModuleGroupName";

/**
    This is a CLI used to update providers of the CIM Server.  This
    command supports operations to stop, start, list and delete provider
    or module.
*/
class CIMProviderCommand : public Command
{
public:

    /**
        Constructs a CIMProviderCommand and initializes instance variables.
    */
    CIMProviderCommand();

    //
    // Overrides the virtual function setCommand from Command class
    // This is defined as an empty function.
    //
    void setCommand(Uint32, char**)
    {
        // Empty function
    }

    /**
        Parses the command line, validates the options, and sets instance
        variables based on the option arguments. This implementation of
        setCommand includes the parameters for output and error stream.

        @param outPrintWriter The stream to which command output is written.
        @param errPrintWriter The stream to which command errors are written.
        @param argc An integer count of the command line arguments
        @param argv A character string array containing the command line
                    arguments

        @throws CommandFormatException if an error is encountered in parsing
                                       the command line
    */
    void setCommand(
        ostream& outPrintWriter,
        ostream& errPrintWriter,
        Uint32 argc,
        char* argv[]);

    /**
        Executes the command and writes the results to the output streams.

        @param outPrintWriter The stream to which command output is written.
        @param errPrintWriter The stream to which command errors are written.
        @return 0 if the command is successful,
                1 if an error occurs in executing the command.
    */
    Uint32 execute(
        ostream& outPrintWriter,
        ostream& errPrintWriter);

private:

    //
    // Delete a provider or module
    //
    // @param outPrintWriter The stream to which command output is written.
    // @param errPrintWriter The stream to which command errors are written.
    //
    // @exception CIMException  if failed to delete provider module
    //
    void _deleteProvider(
        ostream& outPrintWriter,
        ostream& errPrintWriter);

    //
    // Start a provider or module
    //
    // @param outPrintWriter The stream to which command output is written.
    // @param errPrintWriter The stream to which command errors are written.
    //
    // @return Uint32 command result
    // @exception CIMException  if the start operation generates an exception.
    //
    Uint32 _startProvider(
        ostream& outPrintWriter,
        ostream& errPrintWriter);

    //
    // Stop a provider or module
    //
    // @param outPrintWriter The stream to which command output is written.
    // @param errPrintWriter The stream to which command errors are written.
    //
    // @return Uint32 command result
    // @exception CIMException  if the stop operation generates an exception.
    //
    Uint32 _stopProvider(
        ostream& outPrintWriter,
        ostream& errPrintWriter);

    //
    // Sets the provider module group
    //
    // @param outPrintWriter The stream to which command output is written.
    // @param errPrintWriter The stream to which command errors are written.
    //
    // @return Uint32 command result
    // @exception CIMException  if the setModuleGroupName operation generates
    // an exception.
    //
   Uint32 _setProviderModuleGroupName(
        ostream& outPrintWriter,
        ostream& errPrintWriter);


    //
    // List all the registered providers or modules.
    //
    // @param outPrintWriter The stream to which command output is written.
    // @param errPrintWriter The stream to which command errors are written.
    //
    void _listProviders(
        ostream& outPrintWriter,
        ostream& errPrintWriter);

    // Get namedInstance for the provider module
    CIMInstance _getModuleInstance();

    // Get namedInstance for the provider
    CIMInstance _getProviderInstance();

    // Print out registered modules and status
    void _printList(
        Array<String>& moduleNames,
        Array<CIMInstance>& instances,
        ostream& outPrintWriter,
        ostream& errPrintWriter);

    //
    // The CIM Client reference
    //
    AutoPtr<CIMClient> _client;

    //
    // The host name.
    //
    String _hostName;

    //
    // The name of the provider module.
    //
    String _moduleName;

    //
    // The name of the provider module group.
    //
    String _moduleGroupName;

    //
    // The name of the provider.
    //
    String _providerName;

    //
    // The type of operation specified on the command line.
    //
    Uint32 _operationType;

    //
    // The flag to indicate whether the provider module is set or not
    //
    Boolean _moduleSet;

    //
    // The flag to indicate whether the provider is set or not
    //
    Boolean _providerSet;

    //
    // The flag to indicate whether the status is set or not
    //
    Boolean _statusSet;

    //
    // The flag to indicate whether the full status is set or not
    //
    Boolean _fullStatusSet;

    //
    // The flag to indicate whether the group is set or not
    //
    Boolean _moduleGroupSet;

    String usage;
};

/**
    Constructs a CIMProviderCommand and initializes instance variables.
*/
CIMProviderCommand::CIMProviderCommand()
{
    /**
        Initialize the instance variables.
    */
    _operationType      = OPERATION_TYPE_UNINITIALIZED;
    _moduleSet          = false;
    _providerSet        = false;
    _statusSet          = false;
    _fullStatusSet          = false;
    _moduleGroupSet          = false;

    /**
        Build the usage string for the config command.
    */

    usage.reserveCapacity(200);
    usage.append(USAGE);
    usage.append(COMMAND_NAME);

    usage.append(" -").append(OPTION_DISABLE);
    usage.append(" -").append(OPTION_MODULE).append(" module \n");

    usage.append("                   -").append(OPTION_ENABLE);
    usage.append(" -").append(OPTION_MODULE).append(" module \n");

    usage.append("                   -").append(OPTION_GROUP).append(" group");
    usage.append(" -").append(OPTION_MODULE).append(" module \n");

    usage.append("                   -").append(OPTION_REMOVE);
    usage.append(" -").append(OPTION_MODULE).append(" module");
    usage.append(" [ -").append(OPTION_PROVIDER).append(" provider ] \n");

    usage.append("                   -").append(OPTION_LIST);
    usage.append(" [ -").append(OPTION_STATUS);
    usage.append(" | -").append(OPTION_FULLSTATUS);
    usage.append(" | -").append(OPTION_MODULE).append(" module ] \n");

    usage.append("                   -").append(OPTION_HELP).append("\n");
    usage.append("                   --").append(LONG_HELP).append("\n");
    usage.append("                   --").append(LONG_VERSION).append("\n");

    usage.append("Options:\n");
    usage.append("    -d         - "
        "Disable the specified CIM provider module\n");
    usage.append("    -e         - "
        "Enable the specified CIM provider module\n");
    usage.append("    -g         - "
        "Sets the CIM provider module group. Specify\n"
        "                 empty string to remove from grouping.\n"
        "                 If the provider module is  active, provider\n"
        "                 module is disabled first, group is set and\n"
        "                 enabled again. All provider modules with the same\n"
        "                 group name are loaded into a single agent process\n"
        "                 except when overridden by specific UserContext\n"
        "                 values. If group name is CIMServer, provider module\n"
        "                 is loaded into CIMServer process depending on\n"
        "                 UserContext value\n");
    usage.append("    -h, --help - Display this help message\n");
    usage.append("    -l         - "
        "Display all the registered provider modules\n");
    usage.append("    -m         - "
        "Specify the provider module for the operation\n");
    usage.append("    -p         - Specify the provider for the operation\n");

    usage.append("    -r         - "
        "Remove the registration for a provider (if specified)\n"
        "                 or for the specified provider module and all the "
            "providers\n"
        "                 it contains\n");
    usage.append("    -s         - "
        "Display the status of registered provider modules\n");
    usage.append("    -f         - "
        "Full status, display the status of registered\n"
        "                 provider modules and module group name\n");
    usage.append("    --version  - Display CIM Server version number\n");

// Localize the usage text
#ifdef PEGASUS_HAS_ICU
    MessageLoaderParms menuparms(
        "Clients.cimprovider.CIMProviderCommand.MENU.STANDARD", usage);
    menuparms.msg_src_path = MSG_PATH;
    usage = MessageLoader::getMessage(menuparms);
#endif

    setUsage(usage);
}



/**
    Parses the command line, validates the options, and sets instance
    variables based on the option arguments.
*/
void CIMProviderCommand::setCommand(
    ostream& outPrintWriter,
    ostream& errPrintWriter,
    Uint32 argc,
    char* argv[])
{
    Uint32 i = 0;
    Uint32 c = 0;
    String badOptionString;
    String optString;

    //
    //  Construct optString
    //
    optString.append(OPTION_DISABLE);
    optString.append(OPTION_ENABLE);
    optString.append(OPTION_GROUP);
    optString.append(getoopt::GETOPT_ARGUMENT_DESIGNATOR);
    optString.append(OPTION_HELP);
    optString.append(OPTION_LIST);
    optString.append(OPTION_MODULE);
    optString.append(getoopt::GETOPT_ARGUMENT_DESIGNATOR);
    optString.append(OPTION_PROVIDER);
    optString.append(getoopt::GETOPT_ARGUMENT_DESIGNATOR);
    optString.append(OPTION_REMOVE);
    optString.append(OPTION_STATUS);
    optString.append(OPTION_FULLSTATUS);

    //
    //  Initialize and parse options
    //
    getoopt options("");
    options.addFlagspec(optString);

    options.addLongFlagspec(LONG_HELP, getoopt::NOARG);
    options.addLongFlagspec(LONG_VERSION, getoopt::NOARG);

    options.parse(argc, argv);

    if (options.hasErrors())
    {
        throw CommandFormatException(options.getErrorStrings()[0]);
    }

    _operationType = OPERATION_TYPE_UNINITIALIZED;

    //
    //  Get options and arguments from the command line
    //
    for (i = options.first(); i < options.last(); i++)
    {
        if (options[i].getType() == Optarg::LONGFLAG)
        {
            if (options[i].getopt() == LONG_HELP)
            {
                if (_operationType != OPERATION_TYPE_UNINITIALIZED)
                {
                    String param = String(LONG_HELP);
                    //
                    // More than one operation option was found
                    //
                    throw UnexpectedOptionException(param);
                }

               _operationType = OPERATION_TYPE_HELP;
            }
            else if (options[i].getopt() == LONG_VERSION)
            {
                if (_operationType != OPERATION_TYPE_UNINITIALIZED)
                {
                    String param = String(LONG_VERSION);
                    //
                    // More than one operation option was found
                    //
                    throw UnexpectedOptionException(param);
                }

               _operationType = OPERATION_TYPE_VERSION;
            }
        }
        else if (options[i].getType() == Optarg::REGULAR)
        {
            //
            //  The cimprovider command has no non-option argument options
            //
            throw UnexpectedArgumentException(options[i].Value());
        }
        else /* if (options[i].getType() == Optarg::FLAG) */
        {
            c = options[i].getopt()[0];

            switch (c)
            {
                case OPTION_DISABLE:
                {
                    if (_operationType != OPERATION_TYPE_UNINITIALIZED)
                    {
                        //
                        // More than one operation option was found
                        //
                        throw UnexpectedOptionException(OPTION_DISABLE);
                    }

                    if (options.isSet(OPTION_DISABLE) > 1)
                    {
                        //
                        // More than one disable provider option was found
                        //
                        throw DuplicateOptionException(OPTION_DISABLE);
                    }

                    _operationType = OPERATION_TYPE_DISABLE;

                    break;
                }
                case OPTION_GROUP:
                {
                    if (_operationType != OPERATION_TYPE_UNINITIALIZED)
                    {
                        //
                        // More than one operation option was found
                        //
                        throw UnexpectedOptionException(OPTION_GROUP);
                    }

                    if (options.isSet(OPTION_GROUP) > 1)
                    {
                        //
                        // More than one group option was found
                        //
                        throw DuplicateOptionException(OPTION_GROUP);
                    }

                    _operationType = OPERATION_TYPE_GROUP;
                    _moduleGroupName = options[i].Value();
                    _moduleGroupSet = true;

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

                    if (options.isSet(OPTION_REMOVE) > 1)
                    {
                        //
                        // More than one remove provider option was found
                        //
                        throw DuplicateOptionException(OPTION_REMOVE);
                    }

                    _operationType = OPERATION_TYPE_REMOVE;

                    break;
                }
                case OPTION_ENABLE:
                {
                    if (_operationType != OPERATION_TYPE_UNINITIALIZED)
                    {
                        //
                        // More than one operation option was found
                        //
                        throw UnexpectedOptionException(OPTION_ENABLE);
                    }

                    if (options.isSet(OPTION_ENABLE) > 1)
                    {
                        //
                        // More than one stop option was found
                        //
                        throw DuplicateOptionException(OPTION_ENABLE);
                    }

                    _operationType = OPERATION_TYPE_ENABLE;

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

                    if (options.isSet(OPTION_LIST) > 1)
                    {
                        //
                        // More than one list option was found
                        //
                        throw DuplicateOptionException(OPTION_LIST);
                    }
                    _operationType = OPERATION_TYPE_LIST;
                    break;
                }

                case OPTION_MODULE:
                {
                    if (options.isSet(OPTION_MODULE) > 1)
                    {
                        //
                        // More than one module option was found
                        //
                        throw DuplicateOptionException(OPTION_MODULE);
                    }

                    _moduleName = options[i].Value();
                    _moduleSet = true;

                    break;
                }

                case OPTION_PROVIDER:
                {
                    if (options.isSet(OPTION_PROVIDER) > 1)
                    {
                        //
                        // More than one provider option was found
                        //
                        throw DuplicateOptionException(OPTION_PROVIDER);
                    }

                    _providerName = options[i].Value();
                    _providerSet = true;

                    break;
                }

                case OPTION_STATUS:
                {
                    if (options.isSet(OPTION_STATUS) > 1)
                    {
                        //
                        // More than one status option was found
                        //
                        throw DuplicateOptionException(OPTION_STATUS);
                    }

                    _statusSet = true;

                    break;
                }
                case OPTION_FULLSTATUS:
                {
                    if (options.isSet(OPTION_FULLSTATUS) > 1)
                    {
                        //
                        // More than one status option was found
                        //
                        throw DuplicateOptionException(OPTION_FULLSTATUS);
                    }

                    _fullStatusSet = true;

                    break;
                }

                case OPTION_HELP:
                {
                    if (_operationType != OPERATION_TYPE_UNINITIALIZED)
                    {
                        //
                        // More than one operation option was found
                        //
                        throw UnexpectedOptionException(OPTION_HELP);
                    }

                    if (options.isSet(OPTION_HELP) > 1)
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

                    if (options.isSet(OPTION_VERSION) > 1)
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
        throw CommandFormatException(localizeMessage(
            MSG_PATH, REQUIRED_ARGS_MISSING_KEY, REQUIRED_ARGS_MISSING));
    }

    if (_operationType == OPERATION_TYPE_DISABLE
        || _operationType == OPERATION_TYPE_ENABLE
        || _operationType == OPERATION_TYPE_GROUP)
    {
        if (_providerSet)
        {
            throw CommandFormatException(localizeMessage(MSG_PATH,
                UNEXPECTED_OPTION_KEY,
                UNEXPECTED_OPTION));
        }

        if (!_moduleSet)
        {
            throw MissingOptionException(OPTION_MODULE);
        }
    }

    if (_operationType == OPERATION_TYPE_GROUP)
    {
        if (!_moduleGroupSet)
        {
            throw MissingOptionException(OPTION_GROUP);
        }
    }


    if (_operationType == OPERATION_TYPE_REMOVE && !_moduleSet)
    {
        throw MissingOptionException(OPTION_MODULE);
    }

    if (_operationType == OPERATION_TYPE_LIST && _providerSet)
    {
        throw CommandFormatException(localizeMessage(MSG_PATH,
            UNEXPECTED_OPTION_KEY,
            UNEXPECTED_OPTION));
    }

    if (_operationType == OPERATION_TYPE_LIST &&
        ( (_statusSet && _moduleSet) || (_fullStatusSet && _moduleSet) ||
            (_fullStatusSet && _statusSet)))
    {
        throw CommandFormatException(localizeMessage(MSG_PATH,
            UNEXPECTED_OPTION_KEY,
            UNEXPECTED_OPTION));
    }
}

/**
    Executes the command and writes the results to the PrintWriters.
*/
Uint32 CIMProviderCommand::execute(
    ostream& outPrintWriter,
    ostream& errPrintWriter)
{
    if (_operationType == OPERATION_TYPE_UNINITIALIZED)
    {
        //
        // The command was not initialized
        //
        return RC_ERROR;
    }
    else if (_operationType == OPERATION_TYPE_HELP)
    {
        cerr << usage << endl;
        return RC_SUCCESS;
    }
    else if (_operationType == OPERATION_TYPE_VERSION)
    {
        cerr << "Version " << PEGASUS_PRODUCT_VERSION << endl;
        return RC_SUCCESS;
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
        _client->setTimeout(CIMPROVIDERCOMMAND_CLIENT_DEFAULTTIMEOUT);
        _client->setRequestDefaultLanguages();
    }
    catch (Exception& e)
    {
        errPrintWriter << e.getMessage() << endl;
        return RC_ERROR;
    }

    try
    {
        //
        // Open connection with CIMSever
        //
        _client->connectLocal();

    }
    catch (const Exception&)
    {
        outPrintWriter << localizeMessage(MSG_PATH,
            CIMOM_NOT_RUNNING_KEY,
            CIMOM_NOT_RUNNING) << endl;
        return RC_ERROR;
    }


    //
    // Perform the requested operation
    //
    switch (_operationType)
    {
        case OPERATION_TYPE_REMOVE:
            try
            {
                _deleteProvider(outPrintWriter, errPrintWriter);
            }
            catch (CIMException& e)
            {
                CIMStatusCode code = e.getCode();

                if (code == CIM_ERR_FAILED)
                {
                    outPrintWriter << localizeMessage(MSG_PATH,
                        DELETE_PROVIDER_FAILURE_KEY,
                        DELETE_PROVIDER_FAILURE) << endl;
                    errPrintWriter << e.getMessage() << endl;
                }
                else if (code == CIM_ERR_INVALID_CLASS)
                {
                    outPrintWriter << localizeMessage(MSG_PATH,
                        DELETE_PROVIDER_FAILURE_KEY,
                        DELETE_PROVIDER_FAILURE) << endl;
                    outPrintWriter << localizeMessage(MSG_PATH,
                        PG_PROVIDER_SCHEMA_NOT_LOADED_KEY,
                        PG_PROVIDER_SCHEMA_NOT_LOADED) << endl;
                }
                else
                {
                    errPrintWriter << e.getMessage() << endl;
                }
                return RC_ERROR;
            }
            catch (Exception& e)
            {
                errPrintWriter << e.getMessage() << endl;
                return RC_ERROR;
            }

            break;

        case OPERATION_TYPE_ENABLE:
            try
            {
                return _startProvider(outPrintWriter, errPrintWriter);
            }
            catch (CIMException& e)
            {
                CIMStatusCode code = e.getCode();
                if (code == CIM_ERR_FAILED)
                {
                    outPrintWriter << localizeMessage(MSG_PATH,
                        START_PROVIDER_FAILURE_KEY,
                        START_PROVIDER_FAILURE) << endl;
                    errPrintWriter << e.getMessage() << endl;
                }
                else if (code == CIM_ERR_INVALID_CLASS)
                {
                    outPrintWriter << localizeMessage(MSG_PATH,
                        START_PROVIDER_FAILURE_KEY,
                        START_PROVIDER_FAILURE) << endl;
                    outPrintWriter << localizeMessage(MSG_PATH,
                        PG_PROVIDER_SCHEMA_NOT_LOADED_KEY,
                        PG_PROVIDER_SCHEMA_NOT_LOADED) << endl;
                }
                else
                {
                    errPrintWriter << e.getMessage() << endl;
                }
                return RC_ERROR;
            }
            catch (Exception& e)
            {
                errPrintWriter << e.getMessage() << endl;
                return RC_ERROR;
            }

            break;

        case OPERATION_TYPE_DISABLE:
            try
            {
                return _stopProvider(outPrintWriter, errPrintWriter);
            }
            catch (CIMException& e)
            {
                CIMStatusCode code = e.getCode();
                if (code == CIM_ERR_FAILED)
                {
                    outPrintWriter << localizeMessage(MSG_PATH,
                        STOP_PROVIDER_FAILURE_KEY,
                        STOP_PROVIDER_FAILURE) << endl;
                    errPrintWriter << e.getMessage() << endl;
                }
                else if (code == CIM_ERR_INVALID_CLASS)
                {
                    outPrintWriter << localizeMessage(MSG_PATH,
                        STOP_PROVIDER_FAILURE_KEY,
                        STOP_PROVIDER_FAILURE) << endl;
                    outPrintWriter << localizeMessage(MSG_PATH,
                        PG_PROVIDER_SCHEMA_NOT_LOADED,
                        PG_PROVIDER_SCHEMA_NOT_LOADED) << endl;
                }
                else
                {
                    errPrintWriter << e.getMessage() << endl;
                }
                return RC_ERROR;
            }
            catch (Exception& e)
            {
                errPrintWriter << e.getMessage() << endl;
                return RC_ERROR;
            }
            break;

        case OPERATION_TYPE_GROUP:
            try
            {
                return _setProviderModuleGroupName(
                    outPrintWriter, errPrintWriter);
            }
            catch (CIMException& e)
            {
                CIMStatusCode code = e.getCode();
                if (code == CIM_ERR_FAILED)
                {
                    outPrintWriter << localizeMessage(MSG_PATH,
                        SET_PROVIDERMODULE_GROUP_FAILURE_KEY,
                        SET_PROVIDERMODULE_GROUP_FAILURE) << endl;
                    errPrintWriter << e.getMessage() << endl;
                }
                else if (code == CIM_ERR_INVALID_CLASS)
                {
                    outPrintWriter << localizeMessage(MSG_PATH,
                        SET_PROVIDERMODULE_GROUP_FAILURE_KEY,
                        SET_PROVIDERMODULE_GROUP_FAILURE) << endl;
                    outPrintWriter << localizeMessage(MSG_PATH,
                        PG_PROVIDER_SCHEMA_NOT_LOADED,
                        PG_PROVIDER_SCHEMA_NOT_LOADED) << endl;
                }
                else
                {
                    errPrintWriter << e.getMessage() << endl;
                }
                return RC_ERROR;
            }
            catch (Exception& e)
            {
                errPrintWriter << e.getMessage() << endl;
                return RC_ERROR;
            }
            break;

        case OPERATION_TYPE_LIST:
            try
            {
                _listProviders(outPrintWriter, errPrintWriter);
            }
            catch (CIMException& e)
            {
                CIMStatusCode code = e.getCode();
                if (code == CIM_ERR_FAILED)
                {
                    outPrintWriter << localizeMessage(MSG_PATH,
                        LIST_PROVIDERS_FAILURE_KEY,
                        LIST_PROVIDERS_FAILURE) << endl;
                    errPrintWriter << e.getMessage() << endl;
                }
                else if (code == CIM_ERR_INVALID_CLASS)
                {
                    outPrintWriter << localizeMessage(MSG_PATH,
                        LIST_PROVIDERS_FAILURE_KEY,
                        LIST_PROVIDERS_FAILURE) << endl;
                    outPrintWriter << localizeMessage(MSG_PATH,
                        PG_PROVIDER_SCHEMA_NOT_LOADED_KEY,
                        PG_PROVIDER_SCHEMA_NOT_LOADED) << endl;

                }
                else
                {
                    errPrintWriter << e.getMessage() << endl;
                }
                return RC_ERROR;
            }
            catch (Exception& e)
            {
                errPrintWriter << e.getMessage() << endl;
                return RC_ERROR;
            }
            break;

        default:
            //
            // Should never get here
            //
            break;
    }

    return RC_SUCCESS;
}

/**
    Delete the provider module or providers in a module
*/
void CIMProviderCommand::_deleteProvider(
    ostream& outPrintWriter,
    ostream& errPrintWriter)
{
    PEGASUS_ASSERT(_moduleSet);

    if (_providerSet)
    {
        // Delete provider with specified module name and provider name

        CIMInstance providerInstance = _getProviderInstance();

        CIMObjectPath providerRef = providerInstance.getPath();
        providerRef.setNameSpace(PEGASUS_NAMESPACENAME_PROVIDERREG);
        providerRef.setClassName(PEGASUS_CLASSNAME_PROVIDER);

        CIMKeyBinding kb1(
            _PROPERTY_PROVIDERMODULENAME,
            _moduleName,
            CIMKeyBinding::STRING);
        CIMKeyBinding kb2(
            _PROPERTY_PROVIDER_NAME,
            _providerName,
            CIMKeyBinding::STRING);
        Array<CIMKeyBinding> keys;
        keys.append(kb1);
        keys.append(kb2);
        providerRef.setKeyBindings(keys);

        outPrintWriter << localizeMessage(MSG_PATH,
            DELETEING_PROVIDER_KEY,
            DELETEING_PROVIDER) << endl;

        _client->deleteInstance(
            PEGASUS_NAMESPACENAME_PROVIDERREG,
            providerRef);

        outPrintWriter << localizeMessage(MSG_PATH,
            DELETE_PROVIDER_SUCCESS_KEY,
            DELETE_PROVIDER_SUCCESS) << endl;
    }
    else
    {
        // Delete all the registered provider modules

        CIMInstance moduleInstance = _getModuleInstance();

        CIMObjectPath moduleRef = moduleInstance.getPath();
        moduleRef.setNameSpace(PEGASUS_NAMESPACENAME_PROVIDERREG);
        moduleRef.setClassName(PEGASUS_CLASSNAME_PROVIDERMODULE);

        CIMKeyBinding kb1(
            CIMName("Name"), _moduleName, CIMKeyBinding::STRING);
        Array<CIMKeyBinding> keys;
        keys.append(kb1);

        moduleRef.setKeyBindings(keys);
        outPrintWriter << localizeMessage(MSG_PATH,
            DELETEING_PROVIDER_MODULE_KEY,
            DELETEING_PROVIDER_MODULE) << endl;

        _client->deleteInstance(
            PEGASUS_NAMESPACENAME_PROVIDERREG,
            moduleRef);
        outPrintWriter << localizeMessage(MSG_PATH,
            DELETE_PROVIDER_MODULE_SUCCESS_KEY,
            DELETE_PROVIDER_MODULE_SUCCESS) << endl;
    }
}

//
// Set the  provider module group name
//
Uint32 CIMProviderCommand::_setProviderModuleGroupName(
    ostream& outPrintWriter,
    ostream& errPrintWriter)
{
    PEGASUS_ASSERT(_moduleSet);
    PEGASUS_ASSERT(_moduleGroupSet);

    // get the module instance
    CIMInstance moduleInstance = _getModuleInstance();

    CIMObjectPath moduleRef = moduleInstance.getPath();
    moduleRef.setNameSpace(PEGASUS_NAMESPACENAME_PROVIDERREG);
    moduleRef.setClassName(PEGASUS_CLASSNAME_PROVIDERMODULE);

    CIMKeyBinding kb1(CIMName("Name"), _moduleName, CIMKeyBinding::STRING);
    Array<CIMKeyBinding> keys;
    keys.append(kb1);

    moduleRef.setKeyBindings(keys);

    outPrintWriter << localizeMessage(MSG_PATH,
        SETTING_PROVIDERMODULE_GROUP_KEY,
        SETTING_PROVIDERMODULE_GROUP) << endl;

    Array<CIMParamValue> inParams;
    Array<CIMParamValue> outParams;

    inParams.append(CIMParamValue(MODULEGROUPNAME, _moduleGroupName));

    CIMValue ret_value = _client->invokeMethod(
        PEGASUS_NAMESPACENAME_PROVIDERREG,
        moduleRef,
        SETMODULEGROUPNAME_METHOD,
        inParams,
        outParams);

    Sint16 retValue;
    ret_value.get(retValue);
    if (retValue == 0)
    {
        outPrintWriter << localizeMessage(MSG_PATH,
            SET_PROVIDERMODULE_GROUP_SUCCESS_KEY,
            SET_PROVIDERMODULE_GROUP_SUCCESS) << endl;
        return RC_SUCCESS;
    }
    else
    {
        outPrintWriter << localizeMessage(MSG_PATH,
            SET_PROVIDERMODULE_GROUP_FAILURE_KEY,
            SET_PROVIDERMODULE_GROUP_FAILURE) << endl;
        return RC_ERROR;
    }
}

// Enable the provider module
//
Uint32 CIMProviderCommand::_startProvider(
    ostream& outPrintWriter,
    ostream& errPrintWriter)
{
    PEGASUS_ASSERT(_moduleSet);

    // get the module instance
    CIMInstance moduleInstance = _getModuleInstance();

    CIMObjectPath moduleRef = moduleInstance.getPath();
    moduleRef.setNameSpace(PEGASUS_NAMESPACENAME_PROVIDERREG);
    moduleRef.setClassName(PEGASUS_CLASSNAME_PROVIDERMODULE);

    CIMKeyBinding kb1(CIMName("Name"), _moduleName, CIMKeyBinding::STRING);
    Array<CIMKeyBinding> keys;
    keys.append(kb1);

    moduleRef.setKeyBindings(keys);

    //
    // enable module
    //

    outPrintWriter << localizeMessage(MSG_PATH,
        STARTING_PROVIDER_MODULE_KEY,
        STARTING_PROVIDER_MODULE) << endl;

    Array<CIMParamValue> inParams;
    Array<CIMParamValue> outParams;

    CIMValue ret_value = _client->invokeMethod(
        PEGASUS_NAMESPACENAME_PROVIDERREG,
        moduleRef,
        START_METHOD,
        inParams,
        outParams);

    Sint16 retValue;
    ret_value.get(retValue);
    if (retValue == 0)
    {
        outPrintWriter << localizeMessage(MSG_PATH,
            START_PROVIDER_SUCCESS_KEY,
            START_PROVIDER_SUCCESS) << endl;
        return RC_SUCCESS;
    }
    else if (retValue == 1)
    {
        outPrintWriter << localizeMessage(MSG_PATH,
            PROVIDER_ALREADY_STARTED_KEY,
            PROVIDER_ALREADY_STARTED) << endl;
        return RC_SUCCESS;
    }
    else if (retValue == 2)
    {
        outPrintWriter << localizeMessage(MSG_PATH,
            CANNOT_START_PROVIDER_KEY,
            CANNOT_START_PROVIDER) << endl;
        return RC_ERROR;
    }
    else
    {
        outPrintWriter << localizeMessage(MSG_PATH,
            START_PROVIDER_FAILURE_KEY,
            START_PROVIDER_FAILURE) << endl;
        return RC_ERROR;
    }
}

//
// Disable the provider module
//
Uint32 CIMProviderCommand::_stopProvider(
    ostream& outPrintWriter,
    ostream& errPrintWriter)
{
    PEGASUS_ASSERT(_moduleSet);

    // get the module instance
    CIMInstance moduleInstance = _getModuleInstance();

    CIMObjectPath moduleRef = moduleInstance.getPath();
    moduleRef.setNameSpace(PEGASUS_NAMESPACENAME_PROVIDERREG);
    moduleRef.setClassName(PEGASUS_CLASSNAME_PROVIDERMODULE);

    CIMKeyBinding kb1(CIMName("Name"), _moduleName, CIMKeyBinding::STRING);
    Array<CIMKeyBinding> keys;
    keys.append(kb1);

    moduleRef.setKeyBindings(keys);

    //
    // disable the module
    //

    outPrintWriter << localizeMessage(MSG_PATH,
        STOPING_PROVIDER_MODULE_KEY,
        STOPING_PROVIDER_MODULE) << endl;

    Array<CIMParamValue> inParams;
    Array<CIMParamValue> outParams;

    CIMValue ret_value = _client->invokeMethod(
        PEGASUS_NAMESPACENAME_PROVIDERREG,
        moduleRef,
        STOP_METHOD,
        inParams,
        outParams);

    Sint16 retValue;
    ret_value.get(retValue);
    if (retValue == 0)
    {
        outPrintWriter << localizeMessage(MSG_PATH,
            STOP_PROVIDER_SUCCESS_KEY,
            STOP_PROVIDER_SUCCESS) << endl;
        return RC_SUCCESS;
    }
    else if (retValue == 1)
    {
        outPrintWriter << localizeMessage(MSG_PATH,
            PROVIDER_ALREADY_STOPPED_KEY,
            PROVIDER_ALREADY_STOPPED) << endl;
        return RC_SUCCESS;
    }
    else if (retValue == -2)
    {
        outPrintWriter << localizeMessage(MSG_PATH,
            PENDING_REQUESTS_STOP_PROVIDER_FAILURE_KEY,
            PENDING_REQUESTS_STOP_PROVIDER_FAILURE) << endl;
        return RC_ERROR;
    }
    else
    {
        outPrintWriter << localizeMessage(MSG_PATH,
            STOP_PROVIDER_FAILURE_KEY,
            STOP_PROVIDER_FAILURE) << endl;
        return RC_ERROR;
    }
}


/**
    get a list of all registered provider modules or their status or providers.
 */
void CIMProviderCommand::_listProviders(
    ostream& outPrintWriter,
    ostream& errPrintWriter)
{
    Array<CIMInstance> moduleInstances;
    Array<CIMObjectPath> instanceNames;
    String moduleName;
    String providerName;
    Array<String> moduleNames;
    Array<CIMInstance> instances;
    Boolean moduleExist = false;

    if (_moduleSet)
    {
        // get registered providers which are in the specified module
        instanceNames = _client->enumerateInstanceNames(
            PEGASUS_NAMESPACENAME_PROVIDERREG,
            PEGASUS_CLASSNAME_PROVIDER);

        if (instanceNames.size() == 0)
        {
            cerr << localizeMessage(MSG_PATH,
                ERR_PROVIDER_NOT_REGISTERED_KEY,
                ERR_PROVIDER_NOT_REGISTERED) << endl;
            exit(RC_ERROR);
        }
        else
        {
            // List all the registered providers which have specified module
            for (Uint32 i = 0; i < instanceNames.size(); i++)
            {
                Array<CIMKeyBinding> keys = instanceNames[i].getKeyBindings();
                for (Uint32 j=0; j < keys.size(); j++)
                {
                    if (keys[j].getName().equal(_PROPERTY_PROVIDERMODULENAME))
                    {
                        moduleName = keys[j].getValue();
                    }
                    if (keys[j].getName().equal(_PROPERTY_PROVIDER_NAME))
                    {
                        providerName = keys[j].getValue();
                    }
                }
                if (String::equalNoCase(moduleName, _moduleName))
                {
                    moduleExist = true;
                        outPrintWriter << providerName << endl;;
                }
            }

            if (!moduleExist)
            {
                cerr << localizeMessage(MSG_PATH,
                    ERR_PROVIDER_NOT_REGISTERED_KEY,
                    ERR_PROVIDER_NOT_REGISTERED) << endl;
                exit(RC_ERROR);
            }
        }
    }
    else
    {
        // Get all registered provider modules
        moduleInstances = _client->enumerateInstances(
            PEGASUS_NAMESPACENAME_PROVIDERREG,
            PEGASUS_CLASSNAME_PROVIDERMODULE);
        if (moduleInstances.size() != 0)
        {
            // List all the registered provider modules
            for (Uint32 i = 0; i < moduleInstances.size(); i++)
            {
                CIMInstance& instance = moduleInstances[i];
                instance.getProperty(
                instance.findProperty(
                    _PROPERTY_PROVIDERMODULE_NAME)).getValue().get(moduleName);
                moduleNames.append(moduleName);
                instances.append(instance);
            }

            _printList(moduleNames, instances, outPrintWriter, errPrintWriter);
        }
    }
}

// Get namedInstance for a provider module
CIMInstance CIMProviderCommand::_getModuleInstance()
{

    CIMKeyBinding kb(
        _PROPERTY_PROVIDERMODULE_NAME, _moduleName, CIMKeyBinding::STRING);

    Array<CIMKeyBinding> kbArray;
    kbArray.append(kb);

    CIMObjectPath moduleRef(
        "", PEGASUS_NAMESPACENAME_PROVIDERREG,
        PEGASUS_CLASSNAME_PROVIDERMODULE,
        kbArray);

    CIMInstance instance;

    try
    {
        instance = _client->getInstance(
            PEGASUS_NAMESPACENAME_PROVIDERREG, moduleRef);
    }
    catch (const Exception&)
    {
        // Provider module was not registered yet
        cerr << localizeMessage(MSG_PATH,
            ERR_MODULE_NOT_REGISTERED_KEY,
            ERR_MODULE_NOT_REGISTERED) << endl;
        exit(RC_ERROR);
    }

    return instance;
}

// Get namedInstance for a provider
CIMInstance CIMProviderCommand::_getProviderInstance()
{
    CIMKeyBinding kb(
        _PROPERTY_PROVIDERMODULENAME, _moduleName, CIMKeyBinding::STRING);

    CIMKeyBinding kb2(
        _PROPERTY_PROVIDER_NAME, _providerName, CIMKeyBinding::STRING);

    Array<CIMKeyBinding> kbArray;
    kbArray.append(kb);
    kbArray.append(kb2);

    CIMObjectPath providerRef(
        "", PEGASUS_NAMESPACENAME_PROVIDERREG,
        PEGASUS_CLASSNAME_PROVIDER,
        kbArray);

    CIMInstance instance;

    try
    {
        instance = _client->getInstance(
            PEGASUS_NAMESPACENAME_PROVIDERREG, providerRef);
    }
    catch (const Exception&)
    {
        // Provider was not registered yet
        cerr << localizeMessage(MSG_PATH,
            ERR_PROVIDER_NOT_REGISTERED_KEY,
            ERR_PROVIDER_NOT_REGISTERED) << endl;
        exit(RC_ERROR);
    }

    return instance;
}

// Print out registered modules and status
void CIMProviderCommand::_printList(
    Array<String>& moduleNames,
    Array<CIMInstance>& instances,
    ostream& outPrintWriter,
    ostream& errPrintWriter)
{
    Uint32 maxModuleLength=0;
    Uint32 maxStatusLength=0;

    Array<Uint16> _status;
    String output;
    String statusValue;

    Array<String> modulesStatus;

    if (_statusSet || _fullStatusSet)
    {
        // get max length of module name
        for (Uint32 i=0; i < moduleNames.size(); i++)
        {
            if (maxModuleLength < moduleNames[i].size())
            {
                maxModuleLength = moduleNames[i].size();
            }
        }

        for (Uint32 i =0; i < instances.size(); i++)
        {
            Uint32 pos = instances[i].findProperty(_PROPERTY_OPERATIONALSTATUS);
            if (pos == PEG_NOT_FOUND)
            {
                _status.append(0);
            }
            else
            {
                if (instances[i].getProperty(pos).getValue().isNull())
                {
                    if (_status.size() == 0)
                    {
                        _status.append(0);
                    }
                }
                else
                {
                    instances[i].getProperty(pos).getValue().get(_status);
                }
            }

            for (Uint32 j=0; j < _status.size(); j++)
            {
                switch (_status[j])
                {
                    case 0:
                    {
                        statusValue = "Unknown";
                        break;
                    }

                    case 1:
                    {
                        statusValue = "Other";
                        break;
                    }

                    case 2:
                    {
                        statusValue = "OK";
                        break;
                    }

                    case 3:
                    {
                        statusValue = "Degraded";
                        break;
                    }

                    case 4:
                    {
                        statusValue = "Stressed";
                        break;
                    }

                    case 5:
                    {
                        statusValue = "Predictive Failure";
                        break;
                    }

                    case 6:
                    {
                        statusValue = "Error";
                        break;
                    }

                    case 7:
                    {
                        statusValue = "Non-Recoverable Error";
                        break;
                    }

                    case 8:
                    {
                        statusValue = "Starting";
                        break;
                    }

                    case 9:
                    {
                        statusValue = "Stopping";
                        break;
                    }

                    case 10:
                    {
                        statusValue = "Stopped";
                        break;
                    }

                    case 11:
                    {
                        statusValue = "In Service";
                        break;
                    }

                    case 12:
                    {
                        statusValue = "No Contact";
                        break;
                    }

                    case 13:
                    {
                        statusValue = "Lost Communication";
                        break;
                    }

                    default:
                        statusValue = "Not Support";
                        break;
                }
                output.append(statusValue);
                output.append(" ");
            }
            modulesStatus.append(output);
            if (output.size() > maxStatusLength)
            {
                maxStatusLength = output.size();
            }
            output.clear();
        }

        output.clear();
        output = "MODULE";

        for (Uint32 x =0; x < maxModuleLength; ++x)
        {
            output.append(" ");
        }

        output.append("STATUS");

        if (_fullStatusSet)
        {
            for (Uint32 x =0; x < maxStatusLength; ++x)
            {
                output.append(" ");
            }
            output.append("GROUP");
        }

#ifdef PEGASUS_OS_VMS
            //
            // When outputing to a file using outPrintWriter,
            // characters appear one per line.
            // Use printf instead.
            //
            printf("%s\n", (const char *) output.getCString());
#else
            outPrintWriter << output << endl;
#endif

        for (Uint32 i = 0, n = moduleNames.size(); i < n ; ++i)
        {
            output.clear();
            output.append(moduleNames[i]);
            for (Uint32 x =0,
                m = maxModuleLength - moduleNames[i].size() + 6;
                x < m;  ++x)
            {
                output.append(" ");
            }

            output.append(modulesStatus[i]);

            if (_fullStatusSet)
            {

                String groupName;
                Uint32 pos = instances[i].findProperty(MODULEGROUPNAME);
                if (pos != PEG_NOT_FOUND)
                {
                    CIMValue value = instances[i].getProperty(pos).getValue();
                    value.get(groupName);
                    for (Uint32 x =0,
                        m = maxStatusLength - modulesStatus[i].size() + 6;
                        x < m;  ++x)
                    {
                        output.append(" ");
                    }
                    output.append(groupName);
                }
            }

#ifdef PEGASUS_OS_VMS
            //
            // When outputing to a file using outPrintWriter,
            // characters appear one per line.
            // Use printf instead.
            //
            printf("%s\n", (const char *) output.getCString());
#else
            outPrintWriter << output << endl;
#endif
        }
    }
    else
    {
        for (Uint32 i=0; i < moduleNames.size(); i++)
        {
#ifdef PEGASUS_OS_VMS
            //
            // When outputing to a file using outPrintWriter,
            // characters appear one per line.
            // Use printf instead.
            //
            printf("%s\n", (const char *) moduleNames[i].getCString());
#else
            outPrintWriter << moduleNames[i] << endl;
#endif
        }
    }
}

PEGASUS_NAMESPACE_END

//
// exclude main from the Pegasus Namespace
//
PEGASUS_USING_PEGASUS;

///////////////////////////////////////////////////////////////////////////////
/**
    Parses the command line, and execute the command.

    @param   args  the string array containing the command line arguments
*/
///////////////////////////////////////////////////////////////////////////////

int main(int argc, char* argv[])
{
    AutoPtr<CIMProviderCommand>  command;
    Uint32               retCode;

    MessageLoader::_useProcessLocale = true;
    MessageLoader::setPegasusMsgHomeRelative(argv[0]);

#ifdef PEGASUS_OS_ZOS
    // for z/OS set stdout and stderr to EBCDIC
    setEBCDICEncoding(STDOUT_FILENO);
    setEBCDICEncoding(STDERR_FILENO);
#endif

    command.reset(new CIMProviderCommand());

    try
    {
        command->setCommand(cout, cerr, argc, argv);
    }
    catch (CommandFormatException& cfe)
    {
        cerr << COMMAND_NAME << ": " << cfe.getMessage() <<  endl;

        MessageLoaderParms parms(ERR_USAGE_KEY, ERR_USAGE);
        parms.msg_src_path = MSG_PATH;
        cerr << COMMAND_NAME <<
            ": " << MessageLoader::getMessage(parms) << endl;

        return 1;
    }

    retCode = command->execute(cout, cerr);

    return retCode;
}
