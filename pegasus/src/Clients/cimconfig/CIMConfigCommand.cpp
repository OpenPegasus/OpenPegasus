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
#include <Pegasus/Common/System.h>
#include <Pegasus/Common/FileSystem.h>
#include <Pegasus/Common/CIMProperty.h>
#include <Pegasus/Common/CIMObjectPath.h>
#include <Pegasus/Common/CIMStatusCode.h>
#include <Pegasus/Common/Exception.h>
#include <Pegasus/Common/PegasusVersion.h>
#include <Pegasus/Common/StringConversion.h>
#include <Pegasus/getoopt/getoopt.h>

#include <Pegasus/Client/CIMClient.h>
#include <Pegasus/Config/ConfigFileHandler.h>
#include <Pegasus/Config/ConfigManager.h>
#include <Pegasus/Config/ConfigExceptions.h>
#include "CIMConfigCommand.h"
#include <Pegasus/Common/ArrayInternal.h>

#ifdef PEGASUS_OS_PASE
# include <ILEWrapper/ILEUtilities.h>
#endif

#ifdef PEGASUS_OS_ZOS
#include <Pegasus/General/SetFileDescriptorToEBCDICEncoding.h>
#endif

PEGASUS_NAMESPACE_BEGIN

//l10n
/**
 * The CLI message resource name
 */

static const char MSG_PATH []               = "pegasus/pegasusCLI";

//l10n end

/**
    The command name.
*/
static const char COMMAND_NAME []              = "cimconfig";

/**
    The usage string for this command.  This string is displayed
    when an error occurs in parsing or validating the command line.
*/
static const char USAGE []                     = "Usage: ";

/**
    This constant represents the getoopt argument designator
*/
static const char GETOPT_ARGUMENT_DESIGNATOR   = ':';

/*
    These constants represent the operation modes supported by the CLI.
    Any new operation should be added here.
*/

/**
    This constant signifies that an operation option has not been recorded
*/
static const Uint32 OPERATION_TYPE_UNINITIALIZED  = 0;

/**
    This constant represents a property get operation
*/
static const Uint32 OPERATION_TYPE_GET            = 1;

/**
    This constant represents a property set operation
*/
static const Uint32 OPERATION_TYPE_SET            = 2;

/**
    This constant represents a property unset operation
*/
static const Uint32 OPERATION_TYPE_UNSET          = 3;

/**
    This constant represents a property list operation
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
    This constant represents a help operation
*/
static const Uint32 OPERATION_TYPE_CONFIG_PROPERTY_HELP    = 7;

/**
    The constants representing the string literals.
*/
static const CIMName PROPERTY_NAME              = CIMName ("PropertyName");

static const CIMName DEFAULT_VALUE              = CIMName ("DefaultValue");

static const CIMName CURRENT_VALUE              = CIMName ("CurrentValue");

static const CIMName PLANNED_VALUE              = CIMName ("PlannedValue");

static const CIMName DYNAMIC_PROPERTY           = CIMName ("DynamicProperty");

static const CIMName DESCRIPTION                = CIMName("Description");

/**
    The name of the method that implements the property value update using the
    timeout period.
*/
static const CIMName METHOD_UPDATE_PROPERTY_VALUE  =
    CIMName("UpdatePropertyValue");

/**
    The input parameter names for the UpdatePropertyValue() method.
*/
static const String PARAM_PROPERTYVALUE = String("PropertyValue");
static const String PARAM_RESETVALUE = String("ResetValue");
static const String PARAM_UPDATEPLANNEDVALUE = String("SetPlannedValue");
static const String PARAM_UPDATECURRENTVALUE = String("SetCurrentValue");
static const String PARAM_TIMEOUTPERIOD = String("TimeoutPeriod");

/**
    The constants representing the messages.
*/
static const char FILE_NOT_READABLE []         =
                        "Configuration file not readable.";

static const char FAILED_TO_GET_PROPERTY []    =
                        "Failed to get the config property.";

static const char FAILED_TO_SET_PROPERTY []    =
                        "Failed to set the config property.";

static const char FAILED_TO_UNSET_PROPERTY []  =
                        "Failed to unset the config property.";

static const char FAILED_TO_LIST_PROPERTIES [] =
                        "Failed to list the config properties. ";

static const char CONFIG_SCHEMA_NOT_LOADED []  =
    "Please restore the internal repository on the CIM Server.";

static const char PROPERTY_NOT_FOUND []        =
                        "Specified property name was not found.";

static const char INVALID_PROPERTY_VALUE []    =
                        "Specified property value is not valid.";

static const char PROPERTY_NOT_MODIFIED []     =
                        "Specified property can not be modified.";

//l10n default messages and resource keys

static const char OPTION_INVALID_CIM_RUNNING [] =
    "Option -$0 is not valid for this command when CIM server is not running.";

static const char OPTION_INVALID_CIM_RUNNING_KEY [] =
    "Clients.CIMConfig.CIMConfigCommand.OPTION_INVALID_CIM_RUNNING";

static const char FILE_NOT_READABLE_KEY [] =
    "Clients.CIMConfig.CIMConfigCommand.FILE_NOT_READABLE";

static const char FAILED_TO_GET_PROPERTY_KEY [] =
    "Clients.CIMConfig.CIMConfigCommand.FAILED_TO_GET_PROPERTY";

static const char FAILED_TO_SET_PROPERTY_KEY [] =
    "Clients.CIMConfig.CIMConfigCommand.FAILED_TO_SET_PROPERTY";

static const char FAILED_TO_UNSET_PROPERTY_KEY [] =
    "Clients.CIMConfig.CIMConfigCommand.FAILED_TO_UNSET_PROPERTY";

static const char FAILED_TO_LIST_PROPERTIES_KEY [] =
    "Clients.CIMConfig.CIMConfigCommand.FAILED_TO_LIST_PROPERTIES";

static const char CONFIG_SCHEMA_NOT_LOADED_KEY [] =
    "Clients.CIMConfig.CIMConfigCommand.CONFIG_SCHEMA_NOT_LOADED";

static const char PROPERTY_NOT_FOUND_KEY [] =
    "Clients.CIMConfig.CIMConfigCommand.PROPERTY_NOT_FOUND";

static const char INVALID_PROPERTY_VALUE_KEY [] =
    "Clients.CIMConfig.CIMConfigCommand.INVALID_PROPERTY_VALUE";

static const char PROPERTY_NOT_MODIFIED_KEY [] =
    "Clients.CIMConfig.CIMConfigCommand.PROPERTY_NOT_MODIFIED";

static const char PLANNED_VALUE_OF_PROPERTY_IS [] =
    "Planned value for the property $0 is set to \"$1\" in CIMServer.";
static const char PLANNED_VALUE_OF_PROPERTY_IS_KEY [] =
    "Clients.CIMConfig.CIMConfigCommand.PLANNED_VALUE_OF_PROPERTY_IS";

static const char CURRENT_VALUE_OF_PROPERTY_IS [] =
    "Current value for the property $0 is set to \"$1\" in CIMServer.";
static const char CURRENT_VALUE_OF_PROPERTY_IS_KEY [] =
    "Clients.CIMConfig.CIMConfigCommand.CURRENT_VALUE_OF_PROPERTY_IS";

static const char CURRENT_VALUE_OF_PROPERTY_CANNOT_BE_SET [] =
    "Current value for the property '$0' can not be set because the"
        " CIM server is not running.";
static const char CURRENT_VALUE_OF_PROPERTY_CANNOT_BE_SET_KEY [] =
    "Clients.CIMConfig.CIMConfigCommand."
        "CURRENT_VALUE_OF_PROPERTY_CANNOT_BE_SET";

static const char FAILED_UPDATE_OF_PLANNED_VALUE_IN_FILE [] =
    "Failed to update the planned value of the Property '$0' in"
        " configuration file.";
static const char FAILED_UPDATE_OF_PLANNED_VALUE_IN_FILE_KEY [] =
    "Clients.CIMConfig.CIMConfigCommand."
        "FAILED_UPDATE_OF_PLANNED_VALUE_IN_FILE";

static const char PROPERTY_UPDATED_IN_FILE [] =
    "Property '$0' updated in configuration file.";

static const char PROPERTY_UPDATED_IN_FILE_KEY [] =
    "Clients.CIMConfig.CIMConfigCommand.PROPERTY_UPDATED_IN_FILE";

static const char PLANNED_VALUE_IS [] = "Planned value: $0";
static const char PLANNED_VALUE_IS_KEY [] =
    "Clients.CIMConfig.CIMConfigCommand.PLANNED_VALUE_IS";

static const char CURRENT_VALUE_IS [] = "Current value: $0";
static const char CURRENT_VALUE_IS_KEY [] =
    "Clients.CIMConfig.CIMConfigCommand.CURRENT_VALUE_IS";

static const char DEFAULT_VALUE_IS [] = "Default value: $0";
static const char DEFAULT_VALUE_IS_KEY [] =
    "Clients.CIMConfig.CIMConfigCommand.DEFAULT_VALUE_IS";

static const char CANNOT_DETERMINE_PLANNED_VALUE_CIM_RUNNING [] =
    "Planned value can not be determined because the CIM server is not"
        " running.";
static const char CANNOT_DETERMINE_PLANNED_VALUE_CIM_RUNNING_KEY [] =
    "Clients.CIMConfig.CIMConfigCommand."
        "CANNOT_DETERMINE_PLANNED_VALUE_CIM_RUNNING";

static const char CANNOT_DETERMINE_CURRENT_VALUE_CIM_RUNNING [] =
    "Current value can not be determined because the CIM server is"
        " not running.";
static const char CANNOT_DETERMINE_CURRENT_VALUE_CIM_RUNNING_KEY [] =
    "Clients.CIMConfig.CIMConfigCommand."
        "CANNOT_DETERMINE_CURRENT_VALUE_CIM_RUNNING";

static const char PROPERTY_VALUE_ALREADY_SET_TO [] =
    "The property '$0' value is already set to '$1'.";

static const char PROPERTY_VALUE_ALREADY_SET_TO_KEY [] =
    "Clients.CIMConfig.CIMConfigCommand.PROPERTY_VALUE_ALREADY_SET_TO";

static const char CURRENT_PROPERTY_SET_TO_DEFAULT [] =
    "Current value for the property '$0' is set to default value in CIMServer.";
static const char CURRENT_PROPERTY_SET_TO_DEFAULT_KEY [] =
    "Clients.CIMConfig.CIMConfigCommand.CURRENT_PROPERTY_SET_TO_DEFAULT";

static const char PROPERTY_UNSET_IN_FILE [] =
    "Property '$0' is unset in the configuration file.";

static const char PROPERTY_UNSET_IN_FILE_KEY [] =
    "Clients.CIMConfig.CIMConfigCommand.PROPERTY_UNSET_IN_FILE";

static const char CURRENT_PROPERTY_CANNOT_BE_UNSET_CIM_NOT_RUNNING [] =
    "Current value for the property '$0' can not be unset because the"
        " CIM server is not running.";
static const char CURRENT_PROPERTY_CANNOT_BE_UNSET_CIM_NOT_RUNNING_KEY [] =
    "Clients.CIMConfig.CIMConfigCommand."
        "CURRENT_PROPERTY_CANNOT_BE_UNSET_CIM_NOT_RUNNING";

static const char PROPERTY_VALUE_ALREADY_UNSET [] =
    "The property '$0' value is already unset.";

static const char PROPERTY_VALUE_ALREADY_UNSET_KEY [] =
    "Clients.CIMConfig.CIMConfigCommand.PROPERTY_VALUE_ALREADY_UNSET";

static const char CURRENT_VALUES_CANNOT_BE_LISTED_CIM_NOT_RUNNING [] =
    "Current value of properties can not be listed because the CIM server"
        " is not running.";
static const char CURRENT_VALUES_CANNOT_BE_LISTED_CIM_NOT_RUNNING_KEY [] =
    "Clients.CIMConfig.CIMConfigCommand."
        "CURRENT_VALUES_CANNOT_BE_LISTED_CIM_NOT_RUNNING";

static const char NO_PROPERTIES_FOUND_IN_FILE [] =
    "No configuration properties found in the configuration file.";

static const char NO_PROPERTIES_FOUND_IN_FILE_KEY [] =
    "Clients.CIMConfig.CIMConfigCommand.NO_PROPERTIES_FOUND_IN_FILE";

static const char REQUIRED_ARGS_MISSING []        =
                        "Required arguments missing.";

static const char REQUIRED_ARGS_MISSING_KEY [] =
    "Clients.cimuser.CIMUserCommand.REQUIRED_ARGS_MISSING";

static const char ERR_USAGE [] =
    "Use '--help' to obtain command syntax.";

static const char ERR_USAGE_KEY [] =
    "Clients.CIMConfig.CIMConfigCommand.ERR_USAGE";

//l10n end default messages and keys

/**
    The option character used to specify get config property.
*/
static const char   OPTION_GET                 = 'g';

/**
    The option character used to specify set config property.
*/
static const char   OPTION_SET                 = 's';

#ifdef PEGASUS_OS_PASE
/**
    The option character used to specify no output to stdout or stderr.
*/
     static const char OPTION_QUIET_VALUE      = 'q';
#endif

/**
    The option character used to specify unset config property.
*/
static const char   OPTION_UNSET               = 'u';

/**
    The option character used to specify listing of config properties.
*/
static const char   OPTION_LIST                = 'l';

/**
    The option character used to specify the current config value.
*/
static const char   OPTION_CURRENT_VALUE       = 'c';

/**
    The option character used to specify the planned config value.
*/
static const char   OPTION_PLANNED_VALUE       = 'p';

/**
    The option character used to specify the default config value.
*/
static const char   OPTION_DEFAULT_VALUE       = 'd';

/**
    The option character used to specify the timeout value.
*/
static const char   OPTION_TIMEOUT_VALUE       = 't';

/**
    The option character used to display help info.
*/
static const char   OPTION_HELP                = 'h';

/**
    The option character used to display help on config properties.
*/
static const char   OPTION_CONFIG_HELP         = 'H';

/**
    The option character used to display version info.
*/
static const char   OPTION_VERSION             = 'v';

static const char   LONG_HELP []  = "help";

static const char   LONG_VERSION []  = "version";

/**
    Constructs a CIMConfigCommand and initializes instance variables.
*/
CIMConfigCommand::CIMConfigCommand ()
{
    /**
        Initialize the instance variables.
    */
    _operationType       = OPERATION_TYPE_UNINITIALIZED;
    _propertyName        = CIMName ();
    _currentValueSet     = false;
    _plannedValueSet     = false;
    _defaultValueSet     = false;
#ifdef PEGASUS_OS_PASE
    _defaultQuietSet     = false;
#endif

    /**
        Build the usage string for the config command.
    */
    usage.reserveCapacity(200);
    usage.append(USAGE);

    usage.append(COMMAND_NAME);

    usage.append(" -").append(OPTION_GET).append(" name");
    usage.append(" [ -").append(OPTION_CURRENT_VALUE);
    usage.append(" ] [ -").append(OPTION_DEFAULT_VALUE);
    usage.append(" ] [ -").append(OPTION_PLANNED_VALUE).append(" ]\n");
#ifdef PEGASUS_OS_PASE
    usage.append(" ] [ -").append(OPTION_QUIET_VALUE).append(" ]\n");
#endif

    usage.append("                 -").append(OPTION_SET).append(" name=value");
    usage.append(" [ -").append(OPTION_CURRENT_VALUE);
    usage.append(" ] [ -").append(OPTION_PLANNED_VALUE);
    usage.append(" ] [ -").append(OPTION_TIMEOUT_VALUE).append(" ]\n");
#ifdef PEGASUS_OS_PASE
    usage.append(" ] [ -").append(OPTION_QUIET_VALUE).append(" ]\n");
#endif

    usage.append("                 -").append(OPTION_UNSET).append(" name");
    usage.append(" [ -").append(OPTION_CURRENT_VALUE);
    usage.append(" ] [ -").append(OPTION_PLANNED_VALUE);
    usage.append(" ] [ -").append(OPTION_TIMEOUT_VALUE).append(" ]\n");
#ifdef PEGASUS_OS_PASE
    usage.append(" ] [ -").append(OPTION_QUIET_VALUE).append(" ]\n");
#endif

    usage.append("                 -").append(OPTION_LIST);
    usage.append(" [ -").append(OPTION_CURRENT_VALUE);
    usage.append(" | -").append(OPTION_PLANNED_VALUE).append(" ]\n");

    usage.append("                 -").
        append(OPTION_CONFIG_HELP).
        append(" name | \"All\"\n");
    usage.append("                 -").append(OPTION_HELP).append("\n");
    usage.append("                 --").append(LONG_HELP).append("\n");
    usage.append("                 --").append(LONG_VERSION).append("\n");

    usage.append("Options : \n");
    usage.append("    -c         - Use current configuration\n"
                 "                 it is the default optional option"
                                   " for options like -l\n");
    usage.append("    -d         - Use default configuration\n");
    usage.append("    -g         - Get the value of specified configuration"
                                    " property\n");
    usage.append("    -H         - Get help on specified configuration "
                                      "property\n"
                 "                 (or all configuration properties with "
                                       "keyword \"All\"\n");

    usage.append("    -h, --help - Display this help message\n");
    usage.append("    -l         - Display all the configuration properties\n");
    usage.append("    -p         - Configuration used on next CIM Server"
                                    " start\n");
#ifdef PEGASUS_OS_PASE
    usage.append("    -q         - Specify quiet mode,"
                                    "avoiding output to stdout or stderr\n");
#endif
    usage.append("    -s         - Add or Update configuration property"
                                    " value\n");
    usage.append("    -u         - Reset configuration property to its"
                                    " default value\n");
    usage.append("    -t         - Timeout value in seconds for updating the"
                                   " current or\n");
    usage.append("                 planned value\n");
    usage.append("    --version  - Display CIM Server version number\n");

    usage.append("\nUsage note: The cimconfig command can be used to update"
                                    " the next planned\n");
    usage.append("    configuration without having the CIM Server running."
                                    " All other options\n");
    usage.append("    except -h, --help and --version of the cimconfig command"
                                    " require that the\n");
    usage.append("    CIM Server is running.");

//l10n localize usage
#ifdef PEGASUS_HAS_ICU
    MessageLoaderParms menuparms(
            "Clients.CIMConfig.CIMConfigCommand.MENU.STANDARD",usage);
    menuparms.msg_src_path = MSG_PATH;
    usage = MessageLoader::getMessage(menuparms);
#endif
    setUsage (usage);
}

/**
    Parses the command line, validates the options, and sets instance
    variables based on the option arguments.
*/
void CIMConfigCommand::setCommand (Uint32 argc, char* argv [])
{
    unsigned int      i                = 0;
    Uint32            c                = 0;
    String            property         = String ();
    String            badOptionString  = String ();
    String            optString        = String ();
    Uint32            equalsIndex      = 0;

    //
    //  Construct optString
    //
    optString.append(OPTION_GET);
    optString.append(GETOPT_ARGUMENT_DESIGNATOR);

    optString.append(OPTION_SET);
    optString.append(GETOPT_ARGUMENT_DESIGNATOR);

    optString.append(OPTION_UNSET);
    optString.append(GETOPT_ARGUMENT_DESIGNATOR);

    optString.append(OPTION_TIMEOUT_VALUE);
    optString.append(GETOPT_ARGUMENT_DESIGNATOR);


    optString.append(OPTION_LIST);
    optString.append(OPTION_CURRENT_VALUE);
    optString.append(OPTION_PLANNED_VALUE);
    optString.append(OPTION_DEFAULT_VALUE);
#ifdef PEGASUS_OS_PASE
    optString.append(OPTION_QUIET_VALUE);
#endif
    optString.append(OPTION_HELP);

    optString.append(OPTION_CONFIG_HELP);
    optString.append(GETOPT_ARGUMENT_DESIGNATOR);
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
    _timeoutSeconds = 0;

    //
    //  Get options and arguments from the command line
    //
    for (i =  options.first (); i <  options.last (); i++)
    {
        // First search for long arguments since they cannot be
        // processed by case statement
        if (options[i].getType () == Optarg::LONGFLAG)
        {
            if (options[i].getopt () == LONG_HELP)
            {
                if (_operationType != OPERATION_TYPE_UNINITIALIZED)
                {
                    //
                    // More than one operation option was found
                    //
                    throw UnexpectedOptionException(String(LONG_HELP));
                }

               _operationType = OPERATION_TYPE_HELP;
            }
            else if (options[i].getopt () == LONG_VERSION)
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
        else if (options [i].getType () == Optarg::REGULAR)
        {
            //
            //  The cimconfig command has no non-option argument options
            //
            throw UnexpectedArgumentException(options[i].Value());
        }

        // process short arguments with switch
        else /* if (options [i].getType () == Optarg::FLAG) */
        {
            c = options [i].getopt () [0];

            switch (c)
        {
                case OPTION_GET:
                {
                    if (_operationType != OPERATION_TYPE_UNINITIALIZED)
                    {
                        //
                        // More than one operation option was found
                        //
                        throw UnexpectedOptionException(OPTION_GET);
                    }

                    if (options.isSet (OPTION_GET) > 1)
                    {
                        //
                        // More than one get option was found
                        //
                        throw DuplicateOptionException(OPTION_GET);
                    }

                    try
                    {
                        _propertyName = options [i].Value ();
                    }
                    catch (const InvalidNameException&)
                    {
                        throw InvalidOptionArgumentException(
                            options[i].Value(), OPTION_GET);
                    }

                    _operationType = OPERATION_TYPE_GET;

                    break;
                }

                case OPTION_SET:
                {
                    if (_operationType != OPERATION_TYPE_UNINITIALIZED)
                    {
                        //
                        // More than one operation option was found
                        //
                        throw UnexpectedOptionException(OPTION_SET);
                    }

                    if (options.isSet (OPTION_SET) > 1)
                    {
                        //
                        // More than one set option was found
                        //
                        throw DuplicateOptionException(OPTION_SET);
                    }

                    _operationType = OPERATION_TYPE_SET;

                    property = options [i].Value ();

                    equalsIndex = property.find ('=');

                    if ( equalsIndex == PEG_NOT_FOUND )
                    {
                        //
                        // The property value was not specified
                        //
                        throw InvalidOptionArgumentException(
                            property,
                            OPTION_SET);
                    }

                    try
                    {
                        _propertyName = CIMName (property.subString
                            (0, equalsIndex));
                    }
                    catch (const InvalidNameException&)
                    {
                        throw InvalidOptionArgumentException(
                            property, OPTION_SET);
                    }

                    _propertyValue = property.subString( equalsIndex + 1 );

                    break;
                }

                case OPTION_TIMEOUT_VALUE:
                {
                    if (options.isSet (OPTION_TIMEOUT_VALUE) > 1)
                    {
                        throw DuplicateOptionException(OPTION_TIMEOUT_VALUE);
                    }

                    property = options [i].Value ();
                    Uint64 value = 0;
                    if (!StringConversion::decimalStringToUint64(
                        property.getCString(), value) || !value)
                    {
                        throw InvalidOptionArgumentException(
                            property, OPTION_TIMEOUT_VALUE);
                    }
                    _timeoutSeconds = (Uint32)value;
                    break;
                }

                case OPTION_UNSET:
                {
                    if (_operationType != OPERATION_TYPE_UNINITIALIZED)
                    {
                        //
                        // More than one operation option was found
                        //
                        throw UnexpectedOptionException(OPTION_UNSET);
                    }

                    if (options.isSet (OPTION_UNSET) > 1)
                    {
                        //
                        // More than one unset option was found
                        //
                        throw DuplicateOptionException(OPTION_UNSET);
                    }

                    try
                    {
                        _propertyName = options [i].Value ();
                    }
                    catch (const InvalidNameException&)
                    {
                        throw InvalidOptionArgumentException(
                            options[i].Value(), OPTION_UNSET);
                    }

                    _operationType = OPERATION_TYPE_UNSET;

                    break;
                }

                case OPTION_CONFIG_HELP:
                {
                    if (_operationType != OPERATION_TYPE_UNINITIALIZED)
                    {
                        // The operation was already specified and
                        //config parser found a second operation
                        //
                        throw UnexpectedOptionException (OPTION_CONFIG_HELP);
                    }

                    if (options.isSet (OPTION_CONFIG_HELP) > 1)
                    {
                        //
                        // More than one help option was found
                        //
                        throw DuplicateOptionException (OPTION_CONFIG_HELP);
                    }

                    try
                    {
                        _propertyName = options [i].Value ();
                    }
                    catch (const InvalidNameException&)
                    {
                        throw InvalidOptionArgumentException(
                            options[i].Value(), OPTION_CONFIG_HELP);
                    }

                    _operationType = OPERATION_TYPE_CONFIG_PROPERTY_HELP;

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

                case OPTION_CURRENT_VALUE:
                {
                    if (options.isSet (OPTION_CURRENT_VALUE) > 1)
                    {
                        //
                        // More than one current value option was found
                        //
                        throw DuplicateOptionException(OPTION_CURRENT_VALUE);
                    }

                    _currentValueSet = true;
                    break;
                }

                case OPTION_PLANNED_VALUE:
                {
                    if (options.isSet (OPTION_PLANNED_VALUE) > 1)
                    {
                        //
                        // More than one planned value option was found
                        //
                        throw DuplicateOptionException(OPTION_PLANNED_VALUE);
                    }

                    _plannedValueSet = true;
                    break;
                }

                case OPTION_DEFAULT_VALUE:
                {
                    if (options.isSet (OPTION_DEFAULT_VALUE) > 1)
                    {
                        //
                        // More than one default value option was found
                        //
                        throw DuplicateOptionException(OPTION_DEFAULT_VALUE);
                    }

                    _defaultValueSet = true;
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

#ifdef PEGASUS_OS_PASE
                // check for quiet option
                // before processing the rest of the options
                case OPTION_QUIET_VALUE:
                {
                    _defaultQuietSet = true;
                    freopen("/dev/null","w",stdout);
                    freopen("/dev/null","w",stderr);
                    break;
                }
#endif

                default:
                    //
                    // Should never get here
                    //
                    break;
            }
        }
    }

    if ( _operationType == OPERATION_TYPE_UNINITIALIZED )
    {
        //
        // No operation type was specified; throw exception
        // so that usage can be displayed.
        //
        throw CommandFormatException(localizeMessage(
            MSG_PATH, REQUIRED_ARGS_MISSING_KEY, REQUIRED_ARGS_MISSING));
    }

    if ( ( _operationType != OPERATION_TYPE_GET ) && ( _defaultValueSet ) )
    {
        //
        // An invalid option was encountered
        //
        throw InvalidOptionException(OPTION_DEFAULT_VALUE);
    }

    if (_operationType != OPERATION_TYPE_SET &&
        _operationType != OPERATION_TYPE_UNSET &&  _timeoutSeconds != 0)
    {
        //
        // An invalid option was encountered
        //
        throw InvalidOptionException(OPTION_TIMEOUT_VALUE);
    }

    if (_operationType == OPERATION_TYPE_LIST)
    {
        if ( _currentValueSet && _plannedValueSet )
        {
            //
            // An invalid option was encountered
            //
            throw InvalidOptionException(OPTION_CURRENT_VALUE);
        }
#ifdef PEGASUS_OS_PASE
        if( _defaultQuietSet )
        {
            //
            // An invalid option was encountered
            //
            throw InvalidOptionException(OPTION_QUIET_VALUE);
        }
#endif
    }
    else
    {
        //
        // if no options specified for get, set or unset operations
        // then set option as _currentValueSet
        //
        if ( !_currentValueSet && !_plannedValueSet && !_defaultValueSet )
        {
            _currentValueSet = true;
        }
    }

}

/**
    Executes the command and writes the results to the PrintWriters.
*/
Uint32 CIMConfigCommand::execute(
    ostream& outPrintWriter,
    ostream& errPrintWriter)
{
    Boolean   connected     = false;
    String    defaultValue;
    String    currentValue;
    String    plannedValue;
    String    pegasusHome;
    Boolean   gotCurrentValue = false;
    Boolean   gotPlannedValue = false;
    ConfigManager* configManager = ConfigManager::getInstance();

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
        _client->setRequestDefaultLanguages();
    }
    catch (const Exception& e)
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

        connected = true;
    }
    catch (const Exception&)
    {
        //
        // Failed to connect, so process the request offline.
        //
        connected = false;
    }

    if (!connected)
    {
        //
        // Locate the config files
        //
        const char* env = getenv("PEGASUS_HOME");

        String currentFile =
            FileSystem::getAbsolutePath(env, PEGASUS_CURRENT_CONFIG_FILE_PATH);
        String plannedFile =
            FileSystem::getAbsolutePath(env, PEGASUS_PLANNED_CONFIG_FILE_PATH);

        try
        {
            //
            // Open default config files and load current config properties
            //
            _configFileHandler.reset(
                new ConfigFileHandler(currentFile, plannedFile, true));;
        }
        catch (const NoSuchFile&)
        {
        }
        catch (const FileNotReadable& fnr)
        {
            errPrintWriter
                << localizeMessage(
                       MSG_PATH, FILE_NOT_READABLE_KEY, FILE_NOT_READABLE)
                << fnr.getMessage() << endl;
            return RC_ERROR;
        }
        catch (const ConfigFileSyntaxError& cfse)
        {
            errPrintWriter << cfse.getMessage() << endl;
            return RC_ERROR;
        }

        //
        // When the CIM Server is not running, cimconfig only updates the
        // planned config properties.
        //
        _configFileHandler->loadPlannedConfigProperties();
    }

    //
    // Perform the requested operation
    //
    switch (_operationType)
    {
        case OPERATION_TYPE_GET:
            try
            {
                if (connected)
                {
                    Array<String> propertyValues;

                    _getPropertiesFromCIMServer( outPrintWriter,
                        errPrintWriter, _propertyName, propertyValues);

                    defaultValue = propertyValues[1];
                    currentValue = propertyValues[2];
                    gotCurrentValue = true;
                    plannedValue = propertyValues[3];
                    gotPlannedValue = true;
                }
                else
                {
                    if (_defaultValueSet)
                    {
                        errPrintWriter << localizeMessage(MSG_PATH,
                                             OPTION_INVALID_CIM_RUNNING_KEY,
                                             OPTION_INVALID_CIM_RUNNING,
                                             String(&OPTION_DEFAULT_VALUE,1))
                                       << endl;
                        return ( RC_ERROR );
                    }
                    else
                    {
                        gotCurrentValue = _configFileHandler->getCurrentValue (
                            _propertyName, currentValue );
                        gotPlannedValue = _configFileHandler->getPlannedValue (
                            _propertyName, plannedValue );
                    }
                }
            }
            catch (const CIMException& e)
            {
                CIMStatusCode code = e.getCode();

                if (code == CIM_ERR_NOT_FOUND ||
                    code == CIM_ERR_FAILED)
                {
                    outPrintWriter << localizeMessage(MSG_PATH,
                                                PROPERTY_NOT_FOUND_KEY,
                                                PROPERTY_NOT_FOUND)
                                                 << endl;

                    errPrintWriter << e.getMessage() << endl;
                }
                else if (code == CIM_ERR_INVALID_CLASS)
                {
                    outPrintWriter << localizeMessage(MSG_PATH,
                                                FAILED_TO_GET_PROPERTY_KEY,
                                                FAILED_TO_GET_PROPERTY)
                                   << endl <<
                                      localizeMessage(MSG_PATH,
                                                CONFIG_SCHEMA_NOT_LOADED_KEY,
                                                CONFIG_SCHEMA_NOT_LOADED)
                                   << endl;
                }
                else
                {
                    outPrintWriter
                        << localizeMessage(
                                MSG_PATH,
                                FAILED_TO_GET_PROPERTY_KEY,
                                FAILED_TO_GET_PROPERTY)
                        << e.getMessage() << endl;

                }
                return ( RC_ERROR );
            }
            catch (const Exception& e)
            {
                outPrintWriter
                    << localizeMessage(MSG_PATH,FAILED_TO_GET_PROPERTY_KEY,
                                       FAILED_TO_GET_PROPERTY) << endl
                    << e.getMessage() << endl;
                return ( RC_ERROR );
            }

            if( _currentValueSet || ( !_plannedValueSet && !_defaultValueSet ))
            {
                if (gotCurrentValue)
                {
                    outPrintWriter << localizeMessage(MSG_PATH,
                                                      CURRENT_VALUE_IS_KEY,
                                                      CURRENT_VALUE_IS,
                                                      currentValue) << endl;
                }
                else
                {
                    outPrintWriter
                        << localizeMessage(MSG_PATH,
                                CANNOT_DETERMINE_CURRENT_VALUE_CIM_RUNNING_KEY,
                                CANNOT_DETERMINE_CURRENT_VALUE_CIM_RUNNING)
                        << endl;
                    return RC_ERROR;
                }
            }

            if( _plannedValueSet )
            {
                if (gotPlannedValue)
                {
                    outPrintWriter << localizeMessage(MSG_PATH,
                                                      PLANNED_VALUE_IS_KEY,
                                                      PLANNED_VALUE_IS,
                                                      plannedValue) << endl;
                }
                else
                {
                    outPrintWriter
                        << localizeMessage(MSG_PATH,
                               CANNOT_DETERMINE_PLANNED_VALUE_CIM_RUNNING_KEY,
                               CANNOT_DETERMINE_PLANNED_VALUE_CIM_RUNNING)
                        << endl;
                    return RC_ERROR;
                }
            }

            if( _defaultValueSet )
            {
                outPrintWriter << localizeMessage(MSG_PATH,
                                                  DEFAULT_VALUE_IS_KEY,
                                                  DEFAULT_VALUE_IS,
                                                  defaultValue) << endl;
            }
            break;

        case OPERATION_TYPE_SET:
            //
            // send changes to CIMOM if running, else send to config file
            //
            try
            {
                if (connected)
                {
                    _updatePropertyInCIMServer( outPrintWriter,
                        errPrintWriter, _propertyName, _propertyValue, false);

                    if ( _currentValueSet )
                    {
                       outPrintWriter
                           << localizeMessage(MSG_PATH,
                                  CURRENT_VALUE_OF_PROPERTY_IS_KEY,
                                  CURRENT_VALUE_OF_PROPERTY_IS,
                                  _propertyName.getString(),
                                  _propertyValue)
                           << endl;
                    }

                    if ( _plannedValueSet )
                    {
                        outPrintWriter
                            << localizeMessage(MSG_PATH,
                                   PLANNED_VALUE_OF_PROPERTY_IS_KEY,
                                   PLANNED_VALUE_OF_PROPERTY_IS,
                                   _propertyName.getString(),
                                   _propertyValue)
                            << endl;
                    }
                }
                else
                {
                    if (_currentValueSet)
                    {
                        outPrintWriter
                            << localizeMessage(MSG_PATH,
                                  CURRENT_VALUE_OF_PROPERTY_CANNOT_BE_SET_KEY,
                                  CURRENT_VALUE_OF_PROPERTY_CANNOT_BE_SET,
                                  _propertyName.getString()) << endl;

                        return ( RC_ERROR );
                    }
                    else if (_plannedValueSet)
                    {
                        // Check if new planned value is valid by
                        // asking the ConfigManager which does the same for the
                        // server
                        ConfigManager* myCfgMgr=ConfigManager::getInstance();
                        if (!myCfgMgr->validatePropertyValue(
                                _propertyName.getString(),
                                _propertyValue))
                        {
                            outPrintWriter << localizeMessage(MSG_PATH,
                                                 INVALID_PROPERTY_VALUE_KEY,
                                                 INVALID_PROPERTY_VALUE)
                                           << endl;
                            return ( RC_ERROR );
                        }

                        if ( !_configFileHandler->updatePlannedValue(
                            _propertyName, _propertyValue, false ) )
                        {
                            outPrintWriter
                                << localizeMessage(MSG_PATH,
                                     FAILED_UPDATE_OF_PLANNED_VALUE_IN_FILE_KEY,
                                     FAILED_UPDATE_OF_PLANNED_VALUE_IN_FILE,
                                     _propertyName.getString())
                                << endl;
                            return ( RC_ERROR );
                        }
                    }
                    outPrintWriter
                        << localizeMessage(MSG_PATH,
                               PROPERTY_UPDATED_IN_FILE_KEY,
                               PROPERTY_UPDATED_IN_FILE,
                               _propertyName.getString()) << endl;
                }
            }
            catch (const CIMException& e)
            {
                CIMStatusCode code = e.getCode();

                if (code == CIM_ERR_TYPE_MISMATCH)
                {
                    outPrintWriter << localizeMessage(MSG_PATH,
                                         INVALID_PROPERTY_VALUE_KEY,
                                         INVALID_PROPERTY_VALUE) << endl;

                    errPrintWriter << e.getMessage() << endl;
                }
                else if (code == CIM_ERR_NOT_FOUND)
                {
                    outPrintWriter << localizeMessage(MSG_PATH,
                                          PROPERTY_NOT_FOUND_KEY,
                                          PROPERTY_NOT_FOUND) << endl;

                    errPrintWriter << e.getMessage() << endl;
                }
                else if (code == CIM_ERR_NOT_SUPPORTED)
                {
                    outPrintWriter << localizeMessage(MSG_PATH,
                                          PROPERTY_NOT_MODIFIED_KEY,
                                          PROPERTY_NOT_MODIFIED) << endl;

                    errPrintWriter << e.getMessage() << endl;
                }
                else if (code == CIM_ERR_FAILED)
                {
                    outPrintWriter << localizeMessage(MSG_PATH,
                                          FAILED_TO_SET_PROPERTY_KEY,
                                          FAILED_TO_SET_PROPERTY)
                                   << e.getMessage() << endl;
                }
                else if (code == CIM_ERR_ALREADY_EXISTS)
                {
                    outPrintWriter << localizeMessage(MSG_PATH,
                                          PROPERTY_VALUE_ALREADY_SET_TO_KEY,
                                          PROPERTY_VALUE_ALREADY_SET_TO,
                                          _propertyName.getString(),
                                          _propertyValue) << endl;

                    errPrintWriter << e.getMessage() << endl;
                }
                else if (code == CIM_ERR_INVALID_CLASS)
                {
                    outPrintWriter << localizeMessage(MSG_PATH,
                                          FAILED_TO_SET_PROPERTY_KEY,
                                          FAILED_TO_SET_PROPERTY)
                                          << endl <<
                                      localizeMessage(MSG_PATH,
                                          CONFIG_SCHEMA_NOT_LOADED_KEY,
                                          CONFIG_SCHEMA_NOT_LOADED)
                                          << endl;
                }
                else
                {
                    errPrintWriter << e.getMessage() << endl;
                }
                return ( RC_ERROR );
            }
            catch (const Exception& e)
            {
                outPrintWriter << localizeMessage(MSG_PATH,
                                                  FAILED_TO_SET_PROPERTY_KEY,
                                                  FAILED_TO_SET_PROPERTY)
                                                  << endl
                                                  << e.getMessage() << endl;
                return ( RC_ERROR );
            }
            break;

        case OPERATION_TYPE_UNSET:
            //
            // send changes to CIMOM if running, else send to config file
            //
            try
            {
                if (connected)
                {
                    _propertyValue = String::EMPTY;

                    _updatePropertyInCIMServer( outPrintWriter,
                        errPrintWriter, _propertyName, _propertyValue, true);

                    if ( _currentValueSet )
                    {
                        outPrintWriter << localizeMessage(MSG_PATH,
                                            CURRENT_PROPERTY_SET_TO_DEFAULT_KEY,
                                            CURRENT_PROPERTY_SET_TO_DEFAULT,
                                            _propertyName.getString())
                                            << endl;

                    }

                    if ( _plannedValueSet )
                    {
                        outPrintWriter << localizeMessage(MSG_PATH,
                                              PROPERTY_UNSET_IN_FILE_KEY,
                                              PROPERTY_UNSET_IN_FILE,
                                              _propertyName.getString())
                                              << endl;
                    }
                }
                else
                {
                    if (_currentValueSet)
                    {
                        outPrintWriter << localizeMessage(MSG_PATH,
                           CURRENT_PROPERTY_CANNOT_BE_UNSET_CIM_NOT_RUNNING_KEY,
                           CURRENT_PROPERTY_CANNOT_BE_UNSET_CIM_NOT_RUNNING,
                           _propertyName.getString())
                           << endl;


                        return ( RC_ERROR );
                    }

                    if ( !_configFileHandler->updatePlannedValue(
                        _propertyName, _propertyValue, true ) )
                    {
                        return ( RC_ERROR );
                    }
                    outPrintWriter << localizeMessage(MSG_PATH,
                                          PROPERTY_UNSET_IN_FILE_KEY,
                                          PROPERTY_UNSET_IN_FILE,
                                          _propertyName.getString())
                                          << endl;
                }
            }
            catch (const CIMException& e)
            {
                CIMStatusCode code = e.getCode();

                if (code == CIM_ERR_TYPE_MISMATCH)
                {
                    outPrintWriter << localizeMessage(MSG_PATH,
                                          INVALID_PROPERTY_VALUE_KEY,
                                          INVALID_PROPERTY_VALUE)
                                          << endl;

                    errPrintWriter << e.getMessage() << endl;
                }
                else if (code == CIM_ERR_NOT_FOUND)
                {
                    outPrintWriter << localizeMessage(MSG_PATH,
                                          PROPERTY_NOT_FOUND_KEY,
                                          PROPERTY_NOT_FOUND)
                                          << endl;

                    errPrintWriter << e.getMessage() << endl;
                }
                else if (code == CIM_ERR_NOT_SUPPORTED)
                {
                    outPrintWriter << localizeMessage(MSG_PATH,
                                          PROPERTY_NOT_MODIFIED_KEY,
                                          PROPERTY_NOT_MODIFIED)
                                          << endl;

                    errPrintWriter << e.getMessage() << endl;
                }
                else if (code == CIM_ERR_FAILED)
                {
                    outPrintWriter << localizeMessage(MSG_PATH,
                                          FAILED_TO_UNSET_PROPERTY_KEY,
                                          FAILED_TO_UNSET_PROPERTY)
                                          << e.getMessage() << endl;
                }
                else if (code == CIM_ERR_ALREADY_EXISTS)
                {
                    outPrintWriter << localizeMessage(MSG_PATH,
                                          PROPERTY_VALUE_ALREADY_UNSET_KEY,
                                          PROPERTY_VALUE_ALREADY_UNSET,
                                          _propertyName.getString())
                                          << endl;

                    errPrintWriter << e.getMessage() << endl;
                }
                else if (code == CIM_ERR_INVALID_CLASS)
                {
                    outPrintWriter << localizeMessage(MSG_PATH,
                                          FAILED_TO_UNSET_PROPERTY_KEY,
                                          FAILED_TO_UNSET_PROPERTY)
                                          << endl <<
                                      localizeMessage(MSG_PATH,
                                          CONFIG_SCHEMA_NOT_LOADED_KEY,
                                          CONFIG_SCHEMA_NOT_LOADED)
                                          << endl;
                }
                else
                {
                    errPrintWriter << e.getMessage() << endl;
                }
                return ( RC_ERROR );
            }
            catch (const Exception& e)
            {
                outPrintWriter << localizeMessage(MSG_PATH,
                                      FAILED_TO_UNSET_PROPERTY_KEY,
                                      FAILED_TO_UNSET_PROPERTY)
                                      << endl << e.getMessage() << endl;
                return ( RC_ERROR );
            }
            break;

        // Request info on single property or all properties.
        // Outputs property attributes and descriptive (formatted in
        // configManager) information
        case OPERATION_TYPE_CONFIG_PROPERTY_HELP:
            {
                Array<CIMName> propertyNames;
                // If propertyName = All get all properties.  Else
                // display for provided property.
                if ( String::equalNoCase(_propertyName.getString(),"All"))
                {
                    // Currently this gets from the config files and not
                    // what we really want, (all defined properties) when
                    // server not running. There
                    // is no funciton to get all defined properties today
                    // apparently when server not running. KS TODO.
                    if (!_getConfigPropertyNames(propertyNames, connected))
                    {
                        outPrintWriter << localizeMessage(MSG_PATH,
                            FAILED_TO_GET_PROPERTY_KEY,
                            FAILED_TO_GET_PROPERTY)
                            << "Could not get properties from Server"
                            << endl;
                        return ( RC_ERROR );
                    }
                    else
                    {
                        // Sort property names
                        BubbleSort(propertyNames);
                    }
                }
                else
                {
                    // set the single name into the list
                    propertyNames.append(_propertyName.getString());
                }

                // For all properties in the list, output help info
                for (Uint32 i = 0; i < propertyNames.size(); i++)
                {
                    if (i > 0)
                    {
                        outPrintWriter << endl;
                    }

                    try
                    {
                        // Get Description and attributes for the specified
                        // config property in configInfo String
                        String descriptionInfo;
                        String name = propertyNames[i].getString();
                        configManager->getPropertyHelp(name, descriptionInfo);

                        // Display the property name, "(" attributes ")" EOL
                        // descriptionInfo
                        outPrintWriter << name
                            << " ("
                            << configManager->getDynamicAttributeStatus(name)
                            << "," << "Default:"
                            << configManager->getDefaultValue(name)
                            << ")\n"
                            << descriptionInfo << endl;
                    }
                    catch (const CIMException& cimExp)
                    {
                        CIMStatusCode code = cimExp.getCode();
                        if (code == CIM_ERR_NOT_FOUND ||
                            code == CIM_ERR_FAILED)
                        {
                            outPrintWriter <<
                                localizeMessage(MSG_PATH,
                                    PROPERTY_NOT_FOUND_KEY,
                                    PROPERTY_NOT_FOUND)
                                << endl;
                            errPrintWriter << cimExp.getMessage() << endl;
                        }
                        else
                        {
                            outPrintWriter << localizeMessage(MSG_PATH,
                                FAILED_TO_GET_PROPERTY_KEY,
                                FAILED_TO_GET_PROPERTY)
                                << cimExp.getMessage()
                                << endl;
                        }
                        return ( RC_ERROR );
                    }
                    catch (const Exception& exception)
                    {
                        outPrintWriter << localizeMessage(MSG_PATH,
                            FAILED_TO_GET_PROPERTY_KEY,
                            FAILED_TO_GET_PROPERTY)
                            << endl
                            << exception.getMessage()
                            << endl;
                        return ( RC_ERROR );
                    }
                }
                break;

            }

        case OPERATION_TYPE_LIST:
            //
            // send request to CIMOM if running, else send to config file
            //
            try
            {
                Array<CIMName> propertyNames;
                Array<String> propertyValues;

                if (connected)
                {
                    _listAllPropertiesInCIMServer(outPrintWriter,
                         errPrintWriter, propertyNames, propertyValues);

                }
                else
                {
                    if (_plannedValueSet)
                    {
                        _configFileHandler->getAllPlannedProperties(
                            propertyNames, propertyValues);
                    }
                    else
                    {
                        outPrintWriter
                         << localizeMessage(MSG_PATH,
                            CURRENT_VALUES_CANNOT_BE_LISTED_CIM_NOT_RUNNING_KEY,
                            CURRENT_VALUES_CANNOT_BE_LISTED_CIM_NOT_RUNNING)
                            << endl;
                        return RC_ERROR;
                    }
                }

                Uint32 valuesSize = propertyValues.size();
                Uint32 namesSize  = propertyNames.size();

                if (namesSize == 0)
                {
                    outPrintWriter << localizeMessage(MSG_PATH,
                                          NO_PROPERTIES_FOUND_IN_FILE_KEY,
                                          NO_PROPERTIES_FOUND_IN_FILE)
                                          << endl;
                    break;
                }

                for ( Uint32 i = 0; i < namesSize; i++ )
                {
                    outPrintWriter << propertyNames[i].getString();
                    if ( ( _currentValueSet || _plannedValueSet ) &&
                         ( valuesSize == namesSize) )
                    {
                        outPrintWriter << "=" << propertyValues[i];
                    }
                    outPrintWriter << endl;
                }
                break;
            }
            catch (const CIMException& e)
            {
                CIMStatusCode code = e.getCode();

                if (code == CIM_ERR_NOT_FOUND || code == CIM_ERR_INVALID_CLASS)
                {
                    outPrintWriter << localizeMessage(MSG_PATH,
                                          FAILED_TO_LIST_PROPERTIES_KEY,
                                          FAILED_TO_LIST_PROPERTIES)
                                          << endl <<
                                      localizeMessage(MSG_PATH,
                                          CONFIG_SCHEMA_NOT_LOADED_KEY,
                                          CONFIG_SCHEMA_NOT_LOADED)
                                          << endl;
                }
                else if (code == CIM_ERR_FAILED)
                {
                    outPrintWriter << localizeMessage(MSG_PATH,
                                          FAILED_TO_LIST_PROPERTIES_KEY,
                                          FAILED_TO_LIST_PROPERTIES)
                                   << e.getMessage() << endl;
                }
                else
                {
                    errPrintWriter << e.getMessage() << endl;
                }

                return ( RC_ERROR );
            }
            catch (const Exception& e)
            {
                outPrintWriter << localizeMessage(MSG_PATH,
                                      FAILED_TO_LIST_PROPERTIES_KEY,
                                      FAILED_TO_LIST_PROPERTIES)
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
    Get property values for the specified property from the CIM Server.
*/
void CIMConfigCommand::_getPropertiesFromCIMServer
    (
    ostream&    outPrintWriter,
    ostream&    errPrintWriter,
    const CIMName&    propName,
    Array <String>&    propValues
    )
{
    CIMProperty prop;

    Array<CIMKeyBinding> kbArray;
    CIMKeyBinding        kb;

    kb.setName(PROPERTY_NAME);
    kb.setValue(propName.getString());
    kb.setType(CIMKeyBinding::STRING);

    kbArray.append(kb);

    CIMObjectPath reference(
        _hostName, PEGASUS_NAMESPACENAME_CONFIG,
        PEGASUS_CLASSNAME_CONFIGSETTING, kbArray);

    CIMInstance cimInstance =
        _client->getInstance(PEGASUS_NAMESPACENAME_CONFIG, reference);

    Uint32 pos = cimInstance.findProperty(PROPERTY_NAME);
    prop = (CIMProperty)cimInstance.getProperty(pos);
    propValues.append(prop.getValue().toString());

    pos = cimInstance.findProperty(DEFAULT_VALUE);
    prop = (CIMProperty)cimInstance.getProperty(pos);
    propValues.append(prop.getValue().toString());

    pos = cimInstance.findProperty(CURRENT_VALUE);
    prop = (CIMProperty)cimInstance.getProperty(pos);
    propValues.append(prop.getValue().toString());

    pos = cimInstance.findProperty(PLANNED_VALUE);
    prop = (CIMProperty)cimInstance.getProperty(pos);
    propValues.append(prop.getValue().toString());

    pos = cimInstance.findProperty(DYNAMIC_PROPERTY);
    prop = (CIMProperty)cimInstance.getProperty(pos);
    propValues.append(prop.getValue().toString());
}

/**
    Send an updated property value to the CIM Server.
 */
void CIMConfigCommand::_updatePropertyInCIMServer
    (
    ostream&    outPrintWriter,
    ostream&    errPrintWriter,
    const CIMName&   propName,
    const String&   propValue,
    Boolean     isUnsetOperation
    )
{
    Array<CIMKeyBinding> kbArray;
    CIMKeyBinding        kb;

    kb.setName(PROPERTY_NAME);
    kb.setValue(propName.getString());
    kb.setType(CIMKeyBinding::STRING);

    kbArray.append(kb);

    CIMObjectPath reference(
        _hostName, PEGASUS_NAMESPACENAME_CONFIG,
        PEGASUS_CLASSNAME_CONFIGSETTING, kbArray);

    if (_timeoutSeconds == 0)
    {
        _timeoutSeconds = (PEGASUS_DEFAULT_CLIENT_TIMEOUT_MILLISECONDS / 1000);
    }

    Array<CIMParamValue> inParams;
    Array<CIMParamValue> outParams;

    inParams.append(
        CIMParamValue(PARAM_PROPERTYVALUE, CIMValue(propValue)));

    inParams.append(
        CIMParamValue(
            PARAM_RESETVALUE,
            CIMValue(isUnsetOperation)));

    inParams.append(
        CIMParamValue(
            PARAM_UPDATEPLANNEDVALUE,
            CIMValue(_plannedValueSet)));

    inParams.append(
        CIMParamValue(
            PARAM_UPDATECURRENTVALUE,
            CIMValue(_currentValueSet)));

    inParams.append(
        CIMParamValue(
            PARAM_TIMEOUTPERIOD,
            CIMValue(_timeoutSeconds)));

    // Set timeout and add some grace time.
    _client->setTimeout( (_timeoutSeconds + 10) * 1000);

    _client->invokeMethod(
        PEGASUS_NAMESPACENAME_CONFIG,
        reference,
        METHOD_UPDATE_PROPERTY_VALUE,
        inParams,
        outParams);
}


Boolean CIMConfigCommand::_getConfigPropertyNames(
    Array<CIMName>& propertyNames,
    Boolean connected)
{
    if (connected)
    {
        //
        // get all the instances of class PG_ConfigSetting
        //

        Array<CIMInstance> configNamedInstances =
            _client->enumerateInstances(
                PEGASUS_NAMESPACENAME_CONFIG,
                PEGASUS_CLASSNAME_CONFIGSETTING);
        //
        // copy all the property names
        //
        for (Uint32 i = 0; i < configNamedInstances.size(); i++)
        {
            CIMInstance& configInstance =
                configNamedInstances[i];

            Uint32 pos = configInstance.findProperty
                (CIMName ("PropertyName"));
            CIMProperty prop = (CIMProperty)configInstance.getProperty(pos);
            propertyNames.append(prop.getValue().toString());
        }
    }

    else
    {
        if (_plannedValueSet)
        {
            _configFileHandler->getAllPlannedPropertyNames(
                propertyNames);
        }
        else
        {
            return false;
        }
    }
    return true;
}

/**
    get a list of all property names and value from the CIM Server.
 */
void CIMConfigCommand::_listAllPropertiesInCIMServer
    (
    ostream&    outPrintWriter,
    ostream&    errPrintWriter,
    Array <CIMName>&   propNames,
    Array <String>&   propValues
    )
{
    Array<CIMInstance> configNamedInstances;

    if ( _currentValueSet ||  _plannedValueSet )
    {
        //
        // get all the instances of class PG_ConfigSetting
        //
        configNamedInstances =
            _client->enumerateInstances(
                PEGASUS_NAMESPACENAME_CONFIG,
                PEGASUS_CLASSNAME_CONFIGSETTING);

        //
        // copy all the property names and values
        //
        for (Uint32 i = 0; i < configNamedInstances.size(); i++)
        {
            CIMInstance& configInstance =
                configNamedInstances[i];

            Uint32 pos = configInstance.findProperty
                (CIMName ("PropertyName"));
            CIMProperty prop = (CIMProperty)configInstance.getProperty(pos);
            propNames.append(prop.getValue().toString());

            if (_currentValueSet)
            {
                //
                // get current value
                //
                pos = configInstance.findProperty(CIMName ("CurrentValue"));
                prop = (CIMProperty)configInstance.getProperty(pos);
                propValues.append(prop.getValue().toString());
            }
            else if (_plannedValueSet)
            {
                //
                // get planned value
                //
                pos = configInstance.findProperty(CIMName ("PlannedValue"));
                prop = (CIMProperty)configInstance.getProperty(pos);
                propValues.append(prop.getValue().toString());
            }
        }
    }
    else
    {
        //
        // call enumerateInstanceNames
        //
        Array<CIMObjectPath> instanceNames =
            _client->enumerateInstanceNames(
                PEGASUS_NAMESPACENAME_CONFIG,
                PEGASUS_CLASSNAME_CONFIGSETTING);
        //
        // copy all the property names
        //
        for (Uint32 i = 0; i < instanceNames.size(); i++)
        {
            Array<CIMKeyBinding> kbArray =
                instanceNames[i].getKeyBindings();

            if (kbArray.size() > 0)
            {
                propNames.append(kbArray[0].getValue());
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

int main (int argc, char* argv [])
{
#ifdef PEGASUS_OS_PASE
    // Allow user group name larger than 8 chars in PASE environemnt
    setenv("PASE_USRGRP_LIMITED","N",1);
#endif
    AutoPtr<CIMConfigCommand> command;
    Uint32               returnCode;

    //l10n set message loading to process locale
    MessageLoader::_useProcessLocale = true;
    MessageLoader::setPegasusMsgHomeRelative(argv[0]);

#ifdef PEGASUS_OS_PASE
    // Check special authorities in PASE environment
    if (!umeCheckCmdAuthorities(false))
        return Command::RC_ERROR;
#endif

#ifdef PEGASUS_OS_ZOS
    // for z/OS set stdout and stderr to EBCDIC
    setEBCDICEncoding(STDOUT_FILENO);
    setEBCDICEncoding(STDERR_FILENO);
#endif

    command.reset(new CIMConfigCommand ());

    try
    {
        command->setCommand (argc, argv);
    }
    catch (const CommandFormatException& cfe)
    {
        cerr << COMMAND_NAME << ": " << cfe.getMessage() << endl;

        MessageLoaderParms parms(ERR_USAGE_KEY,ERR_USAGE);
        parms.msg_src_path = MSG_PATH;
        cerr << COMMAND_NAME <<
            ": " << MessageLoader::getMessage(parms) << endl;
        exit (Command::RC_ERROR);
    }

    returnCode = command->execute (cout, cerr);
    command.reset();

    exit (returnCode);
    return 0;
}
