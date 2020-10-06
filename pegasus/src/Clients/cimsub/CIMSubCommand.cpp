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
#include <fstream>
#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/Constants.h>
#include <Pegasus/Common/System.h>
#include <Pegasus/Client/CIMClient.h>
#include <Pegasus/Common/Exception.h>
#include <Pegasus/Common/PegasusVersion.h>
#include <Pegasus/Common/AutoPtr.h>
#include <Pegasus/Common/HostAddress.h>
#include <Pegasus/Common/CIMNameCast.h>
#include <Pegasus/Common/StringConversion.h>


#ifdef PEGASUS_OS_ZOS
#include <Pegasus/General/SetFileDescriptorToEBCDICEncoding.h>
#endif

#ifdef PEGASUS_OS_PASE
#include <ILEWrapper/ILEUtilities.h>
#endif

#include <Pegasus/getoopt/getoopt.h>
#include <Clients/cliutils/CommandException.h>
#include <Clients/cliutils/CsvStringParse.h>
#include "CIMSubCommand.h"

PEGASUS_NAMESPACE_BEGIN


/**
 * The CLI message resource name
 */

static const char MSG_PATH [] = "pegasus/pegasusCLI";

/**
    The command name.
 */
const char COMMAND_NAME[] = "cimsub";

/**
   The default subscription namespace
 */
static const CIMNamespaceName _DEFAULT_SUBSCRIPTION_NAMESPACE  =
    PEGASUS_NAMESPACENAME_INTEROP;

/**
   The Description of the Command.
 */

static const char DESCRIPTION[] = "Cimsub Command Line \
Interface is used to manage CIM Indication Subscriptions.";

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
const Uint32 CIMSubCommand::OPERATION_TYPE_UNINITIALIZED = 0;

/**
    This constant represents a disable subscription operation
 */
const Uint32 CIMSubCommand::OPERATION_TYPE_DISABLE = 1;

/**
    This constant represents a enable subscription operation
 */
const Uint32 CIMSubCommand::OPERATION_TYPE_ENABLE = 2;

/**
    This constant represents a list operation
 */
const Uint32 CIMSubCommand::OPERATION_TYPE_LIST = 3;

/**
    This constant represents a remove operation
 */
const Uint32 CIMSubCommand::OPERATION_TYPE_REMOVE = 4;

/**
    This constant represents a help operation
 */
const Uint32 CIMSubCommand::OPERATION_TYPE_HELP = 5;

/**
    This constant represents a verbose list operation
 */
const Uint32 CIMSubCommand::OPERATION_TYPE_VERBOSE = 6;

/**
    This constant represents a version display operation
 */
const Uint32 CIMSubCommand::OPERATION_TYPE_VERSION = 7;

/**
    This constant represents a create operation
 */
const Uint32 CIMSubCommand::OPERATION_TYPE_CREATE= 8;

/**
    This constant represents a Batch file execution operation
 */

const Uint32 CIMSubCommand::OPERATION_TYPE_BATCH = 9;

/**
    The constants representing the messages.
 */

static const char CIMOM_NOT_RUNNING[] =
    "The CIM server may not be running.";

static const char CIMOM_NOT_RUNNING_KEY[] =
    "Clients.cimsub.CIMSubCommand.CIMOM_NOT_RUNNING";

static const char SUBSCRIPTION_NOT_FOUND_FAILURE[] =
    "The requested subscription could not be found.";

static const char SUBSCRIPTION_NOT_FOUND_KEY[] =
    "Clients.cimsub.CIMSubCommand."
        "SUBSCRIPTION_NOT_FOUND_FAILURE_KEY";

static const char HANDLER_NOT_FOUND_FAILURE[] =
    "The requested handler could not be found.";

static const char HANDLER_NOT_FOUND_KEY[] =
    "Clients.cimsub.CIMSubCommand."
        "HANDLER_NOT_FOUND_FAILURE_KEY";

static const char FILTER_NOT_FOUND_FAILURE[] =
    "The requested filter could not be found.";

static const char FILTER_NOT_FOUND_KEY[] =
    "Clients.cimsub.CIMSubCommand."
        "FILTER_NOT_FOUND_FAILURE_KEY";

static const char SUBSCRIPTION_ALREADY_DISABLED[] =
    "The subscription is already disabled.";

static const char SUBSCRIPTION_ALREADY_DISABLED_KEY[] =
    "Clients.cimsub.CIMSubCommand.SUBSCRIPTION_ALREADY_DISABLED";

static const char SUBSCRIPTION_ALREADY_ENABLED[] =
    "The subscription is already enabled.";

static const char SUBSCRIPTION_ALREADY_ENABLED_KEY[] =
    "Clients.cimsub.CIMSubCommand.SUBSCRIPTION_ALREADY_ENABLED";

static const char REQUIRED_OPTION_MISSING[] =
    "Required option missing.";

static const char REQUIRED_OPTION_MISSING_KEY[] =
    "Clients.cimsub.CIMSubCommand.REQUIRED_OPTION_MISSING";

static const char ERR_USAGE_KEY[] =
    "Clients.cimsub.CIMSubCommand.ERR_USAGE";

static const char ERR_USAGE[] =
    "Use '--help' to obtain command syntax.";

static const char LONG_HELP[] = "help";

static const char LONG_VERSION[] = "version";

/**
    The option character used to specify disable a specified subscription
 */
static const char OPTION_DISABLE = 'd';

/**
    The option character used to specify remove a specified subscription
 */
static const char OPTION_REMOVE = 'r';

/**
    The option character used to specify the Filter Name of a subscription
 */
static const char OPTION_FILTER = 'F';

/**
    The option character used to specify the Query of a Filter
 */
static const char OPTION_QUERY = 'Q';

/**
    The option character used to specify the Query language of a Filter
 */
static const char OPTION_QUERYLANGUAGE = 'L';


/**
    The option character used to specify the Source Namespace of a Filter
 */
static const char OPTION_SOURCENAMESPACE = 'N';


/**
    The option character used to specify enable a specified subscription.
 */
static const char OPTION_ENABLE = 'e';

/**
    The option character used to specify the Handler Name of a subscription
 */
static const char OPTION_HANDLER = 'H';

/**
    The option character used to specify the Destination of a
    CIM_IndicationHandlerCIMXML Handler
 */
static const char OPTION_DESTINATION = 'D';

/**
    The option character used to specify the mailto of a
    PG_ListenerDestinationEmail Handler
 */
static const char OPTION_MAILTO = 'M';

/**
    The option character used to specify the mailcc of a
    PG_ListenerDestinationEmail Handler
 */
static const char OPTION_MAILCC = 'C';

/**
    The option character used to specify the mail subject of a
    PG_ListenerDestinationEmail Handler
 */
static const char OPTION_MAILSUBJECT = 'S';

/**
    The option character used to specify the snmp target host of a
    PG_IndicationHandlerSNMPMapper Handler
 */
static const char OPTION_SNMPTARGETHOST = 'T';

/**
    The option character used to specify the snmp port number of a
    PG_IndicationHandlerSNMPMapper Handler
 */
static const char OPTION_SNMPPORTNUMBER = 'P';

/**
    The option character used to specify the snmp version of a
    PG_IndicationHandlerSNMPMapper Handler
 */
static const char OPTION_SNMPVERSION = 'V';

/**
    The option character used to specify the snmp security name host of a
    PG_IndicationHandlerSNMPMapper Handler
 */
static const char OPTION_SNMPSECURITYNAME = 'U';

/**
    The option character used to specify the snmp engine id of a
    PG_IndicationHandlerSNMPMapper Handler
 */
static const char OPTION_SNMPENGINEID = 'E';

/**
    The option character used to specify creation
 */
static const char OPTION_CREATE = 'c';

/**
    The option character used to specify listing
 */
static const char OPTION_LIST = 'l';

/**
    The option charcter used to specify a file for batch file execution
 */

static const char OPTION_BATCH = 'b';

/**
    The option argument character used to specify subscriptions
 */
static String ARG_SUBSCRIPTIONS = "s";

/**
    The option argument character used to specify filters
 */
static String ARG_FILTERS = "f";

/**
    The option argument character used to specify handlers
 */
static String ARG_HANDLERS = "h";

/**
    The option argument character used to specify handlers, filters,
    and subscriptions
 */
static String ARG_ALL = "a";

/**
    The option character used to specify namespace of subscription
 */
static const char OPTION_NAMESPACE = 'n';

/**
    The option character used to display verbose info.
 */
static const char OPTION_VERBOSE = 'v';

static const char DELIMITER_NAMESPACE = ':';
static const char DELIMITER_HANDLER_CLASS = '.';

/**
    List output header values
 */

const Uint32 TITLE_SEPERATOR_LEN = 2;

static const Uint32 RC_CONNECTION_FAILED = 2;
static const Uint32 RC_CONNECTION_TIMEOUT = 3;
static const Uint32 RC_ACCESS_DENIED = 4;
static const Uint32 RC_NAMESPACE_NONEXISTENT = 5;
static const Uint32 RC_OBJECT_NOT_FOUND = 6;
static const Uint32 RC_OPERATION_NOT_SUPPORTED = 7;

//
// List column header constants
//
const Uint32 _HANDLER_LIST_NAME_COLUMN = 0;
const Uint32 _HANDLER_LIST_DESTINATION_COLUMN = 1;
const Uint32 _FILTER_LIST_NAME_COLUMN = 0;
const Uint32 _FILTER_LIST_QUERY_COLUMN = 1;
const Uint32 _SUBSCRIPTION_LIST_NS_COLUMN = 0;
const Uint32 _SUBSCRIPTION_LIST_FILTER_COLUMN = 1;
const Uint32 _SUBSCRIPTION_LIST_HANDLER_COLUMN = 2;
const Uint32 _SUBSCRIPTION_LIST_STATE_COLUMN = 3;
//
// Handler persistence display values
//
const String _PERSISTENTENCE_OTHER_STRING = "Other";
const String _PERSISTENTENCE_PERMANENT_STRING = "Permanent";
const String _PERSISTENTENCE_TRANSIENT_STRING = "Transient";
const String _PERSISTENTENCE_UNKNOWN_STRING = "Unknown";

//
// Subscription state display values
//
const String _SUBSCRIPTION_STATE_UNKNOWN_STRING = "Unknown";
const String _SUBSCRIPTION_STATE_OTHER_STRING = "Other";
const String _SUBSCRIPTION_STATE_ENABLED_STRING = "Enabled";
const String _SUBSCRIPTION_STATE_ENABLED_DEGRADED_STRING = "Enabled Degraded";
const String _SUBSCRIPTION_STATE_DISABLED_STRING = "Disabled";
const String _SUBSCRIPTION_STATE_NOT_SUPPORTED_STRING = "Not Supported";
//
// SNMP version display values
//
const String _SNMP_VERSION_SNMPV1_TRAP_STRING = "SNMPv1 Trap";
const String _SNMP_VERSION_SNMPV2C_TRAP_STRING = "SNMPv2C Trap";
const String _SNMP_VERSION_PEGASUS_RESERVED_STRING = "Pegasus Reserved";

/**

    Constructs a CIMSubCommand and initializes instance variables.
 */
CIMSubCommand::CIMSubCommand()
{
    /**
        Initialize the instance variables.
    */
    _operationType = OPERATION_TYPE_UNINITIALIZED;
    _verbose = false;
    _handlerSNMPPortNumber = 162;
    _filterQueryLanguage = "CIM:CQL";
    _isBatchNamespace = false;
    _filterNSFlag =false;
    /**
        Build the usage string for the config command.
    */

    usage.reserveCapacity(200);
    usage.append(DESCRIPTION);
    usage.append("\n\n");
    usage.append(USAGE);
    /*
        cimsub -cf [fnamespace:]filtername  -Q query
                 [-L querylanuage] [-N sourcenamespace]
    */
    usage.append(COMMAND_NAME);
    usage.append(" -").append(OPTION_CREATE);
    usage.append(" ").append(ARG_FILTERS);
    usage.append(" [fnamespace:]filtername");
    usage.append(" -").append(OPTION_QUERY).append(" query");
    usage.append(" [-").append(OPTION_QUERYLANGUAGE);
    usage.append(" querylanguage]\n");
    usage.append("                  [-").append(OPTION_SOURCENAMESPACE);
    usage.append(" sourcenamespace(s)] \n");
    /*
        cimsub -ch [hnamespace:][hclassname.]handlername [-D destination ]|
                  [-T mailto [-C mailcc] -S mailsubject] |
                  [-T snmptargethost [-P snmpportnumber] -V snmpversion
                      [-S snmpsecurityname] [-E snmpengineid]]
    */
    usage.append("       ");
    usage.append(COMMAND_NAME);
    usage.append(" -").append(OPTION_CREATE);
    usage.append(" ").append(ARG_HANDLERS);
    usage.append(" [hnamespace:][hclassname.]handlername");
    usage.append(" [-").append(OPTION_DESTINATION);
    usage.append(" destination]| \n");
    usage.append("                  [-").append(OPTION_MAILTO);
    usage.append(" mailto");
    usage.append(" [-").append(OPTION_MAILCC).append(" mailcc]");
    usage.append(" -").append(OPTION_MAILSUBJECT).append(" mailsubject]| \n");
    usage.append("                  [-").append(OPTION_SNMPTARGETHOST);
    usage.append(" snmptargethost");
    usage.append(" [-").append(OPTION_SNMPPORTNUMBER);
    usage.append(" snmpportnumber]");
    usage.append(" -").append(OPTION_SNMPVERSION).append(" snmpversion] \n");
    usage.append("                      [-").append(OPTION_SNMPSECURITYNAME);
    usage.append(" snmpsecurityname]");
    usage.append(" [-").append(OPTION_SNMPENGINEID);
    usage.append(" snmpengineid]] \n");

    /*
        cimsub -cs [-n namespace] -F [fnamespace:]filtername
                  -H [hnamespace:][hclassname.]handlername
    */

    usage.append("       ");
    usage.append(COMMAND_NAME);
    usage.append(" -").append(OPTION_CREATE);
    usage.append(" ").append(ARG_SUBSCRIPTIONS);
    usage.append(" [-").append(OPTION_NAMESPACE).append(" namespace]");
    usage.append(" -").append(OPTION_FILTER).append
        (" [fnamespace:]filtername \n");
    usage.append("                  -").append(OPTION_HANDLER).append
        (" [hnamespace:][hclassname.]handlername \n");
    usage.append("       ");
    usage.append(COMMAND_NAME);
    usage.append(" -").append(OPTION_LIST);
    usage.append(" ").append(ARG_SUBSCRIPTIONS).append("|");
    usage.append(ARG_FILTERS).append("|");
    usage.append(ARG_HANDLERS);
    usage.append(" [-").append(OPTION_VERBOSE).append("]");
    usage.append(" [-").append(OPTION_NAMESPACE).append(" namespace]");
    usage.append(" [-").append(OPTION_FILTER).append
        (" [fnamespace:]filtername] \n");
    usage.append("                  [-").append(OPTION_HANDLER).append
        (" [hnamespace:][hclassname.]handlername] \n");

    usage.append("       ");
    usage.append(COMMAND_NAME);
    usage.append(" -").append(OPTION_ENABLE);
    usage.append(" [-").append(OPTION_NAMESPACE).append(" namespace]");
    usage.append(" -").append(OPTION_FILTER).append
        (" [fnamespace:]filtername \n");
    usage.append("                  -").append(OPTION_HANDLER).append
        (" [hnamespace:][hclassname.]handlername \n");

    usage.append("       ");
    usage.append(COMMAND_NAME);
    usage.append(" -").append(OPTION_DISABLE);
    usage.append(" [-").append(OPTION_NAMESPACE).append(" namespace]");
    usage.append(" -").append(OPTION_FILTER).append
        (" [fnamespace:]filtername\n");
    usage.append("                  -").append(OPTION_HANDLER).append
        (" [hnamespace:][hclassname.]handlername \n");

    usage.append("       ");
    usage.append(COMMAND_NAME);
    usage.append(" -").append(OPTION_REMOVE);
    usage.append(" ").append (ARG_SUBSCRIPTIONS).append("|");
    usage.append(ARG_FILTERS).append("|");
    usage.append(ARG_HANDLERS).append("|");
    usage.append(ARG_ALL);
    usage.append(" [-").append(OPTION_NAMESPACE).append(" namespace]");
    usage.append(" [-").append(OPTION_FILTER).append
        (" [fnamespace:]filtername] \n");
    usage.append("                  [-").append(OPTION_HANDLER).append
        (" [hnamespace:][hclassname.]handlername]\n");
    usage.append("       ");
    usage.append(COMMAND_NAME);
    usage.append(" -").append(OPTION_BATCH);
    usage.append(" ").append("batchfile");
    usage.append(" [-").append(OPTION_NAMESPACE).append(" namespace]\n");
    usage.append("       ");
    usage.append(COMMAND_NAME);
    usage.append(" --").append(LONG_HELP).append("\n");

    usage.append("       ");
    usage.append(COMMAND_NAME);
    usage.append(" --").append(LONG_VERSION).append("\n");

    usage.append("Options : \n");
    usage.append("    -c         - Create specified subscription, filter, \n"
                 "                   handler(CIM_ListenerDestinationCIMXML, \n"
                 "                       if hclassname is not specified)\n");
    usage.append("    -l         - List and display information\n");
    usage.append("    -e         - Enable specified subscription\n");
    usage.append("                   (set SubscriptionState to Enabled) \n");
    usage.append("    -d         - Disable specified subscription \n");
    usage.append("                   (set SubscriptionState to Disabled) \n");
    usage.append("    -r         - Remove specified subscription, handler,"
                                " filter \n");
    usage.append("    -b filename\n");
    usage.append("               - Execute cimsub batch file. Filename is path"
                                     " to the batch file \n");
    usage.append("    -v         - Include verbose information \n");
    usage.append("    -F         - Specify Filter Name of subscription for"
                                " the operation\n");
    usage.append("    -Q         - Specify Query Expression of a Filter \n");
    usage.append("    -L         - Specify Query Language of a Filter \n");
    usage.append("    -N         - To Use SourceNamespaces property,Specify");
    usage.append(" multiple SourceNamespaces\n");
    usage.append("                 with coma separated or append comma to");
    usage.append(" the single SourceNamespace.\n");
    usage.append("                 By default SourceNamespace property is ");
    usage.append(" populated if the single\n");
    usage.append("                 SourceNamespace is specified.\n");
    usage.append("    -H         - Specify Handler Name of subscription for"
                                " the operation\n");
    usage.append("    -D         - Specify Destination of a "
                 "CIM_IndicationHandlerCIMXML Handler.\n"
                 "                   Required option for "
                 "CIM_IndicationHandlerCIMXML or \n"
                 "                   CIM_ListenerDestinationCIMXML Handler\n");
    usage.append("    -M         - Specify Mailto of a "
                 " PG_ListenerDestinationEmail Handler\n");
    usage.append("    -C         - Specify Mailcc of a "
                 "PG_ListenerDestinationEmail Handler\n");
    usage.append("    -S         - Specify Subject of a "
                 "PG_ListenerDestinationEmail Handler\n");
    usage.append("    -T         - Specify Target Host of "
                 "PG_IndicationHandlerSNMPMapper Handler.\n"
                 "                 Required option for SNMPMapper handler\n");
    usage.append("    -U         - Specify Security Name of "
                 "PG_IndicationHandlerSNMPMapper Handler\n");
    usage.append("    -P         - Specify Port Number of a "
                 "PG_IndicationHandlerSNMPMapper Handler\n"
                 "                   (default 162) \n");
    usage.append("    -V         - Specify SNMPVersion of a "
                 "PG_IndicationHandlerSNMPMapper Handler\n"
                 "                   2 : SNMPv1 Trap\n"
                 "                   3 : SNMPv2C Trap\n"
                 "                   4 : SNMPv2C Inform\n"
                 "                   5 : SNMPv3 Trap\n"
                 "                   6 : SNMPv3 Inform\n"
                 "                   Required option for SNMPMapper Handler");
    usage.append("\n");
    usage.append("    -E         - Specify Engine ID of a "
                 "PG_IndicationHandlerSNMPMapper Handler\n");
    usage.append("    -n         - Specify namespace of subscription\n");
    usage.append("                   (interop namespace, if not specified)\n");
    usage.append("    --help     - Display this help message\n");
    usage.append("    --version  - Display CIM Server version\n");
    usage.append("\n");
    usage.append("Usage note: The cimsub command requires that the CIM Server"
                                " is running.\n");
    usage.append("\n");
    usage.append("Batch File Format\n");
    usage.append("-----------------\n\n");
    usage.append("A cimsub batch file contains multiple cimsub ");
    usage.append("commands, one per line.");
    usage.append("Lines\nthat start with the '#' character and blank lines");
    usage.append(" are ignored.The Execution\n");
    usage.append("of Batch file will stop when it encounters any syntax error");
    usage.append(" or an exception\nin the command.\n\n");
    usage.append("The namespace option (-n) when used with batch execution");
    usage.append(" (-b) uses the\n");
    usage.append("namespace to populate the namespace values of filter, ");
    usage.append("handler and\nsubscription. Any namespace specified for ");
    usage.append("any or all of these is specified\ninside the batch");
    usage.append("command, they will be overridden.\n");

#ifdef PEGASUS_HAS_ICU
    MessageLoaderParms menuparms(
            "Clients.cimsub.CIMSubCommand.MENU.STANDARD",
            usage);
    menuparms.msg_src_path = MSG_PATH;
    usage = MessageLoader::getMessage(menuparms);
#endif

    setUsage(usage);
}



/**
    Parses the command line, validates the options, and sets instance
    variables based on the option arguments.
*/
void CIMSubCommand::setCommand(
    ostream& outPrintWriter,
    ostream& errPrintWriter,
    Uint32 argc,
    char* argv[])
{
    Uint32 i = 0;
    Uint64 c = 0;
    //char c;
    String badOptionString;
    String optString;
    String filterNameString;
    String handlerNameString;
    Boolean filterSet = false;
    Boolean handlerSet = false;
    Boolean filterQuerySet = false;
    Boolean filterQueryLanguageSet = false;
    Boolean filterSourceNamespaceSet = false;
    Boolean handlerDestinationSet = false;
    Boolean handlerMailToSet = false;
    Boolean handlerMailCcSet = false;
    Boolean handlerMailSubjectSet = false;
    Boolean handlerSnmpTargetHostSet = false;
    Boolean handlerSnmpPortNumberSet = false;
    Boolean handlerSnmpVersionSet = false;
    Boolean handlerSnmpSecurityNameSet = false;
    Boolean handlerSnmpEngineIdSet = false;

    //
    //  Construct optString
    //
    optString.append(OPTION_CREATE);
    optString.append(getoopt::GETOPT_ARGUMENT_DESIGNATOR);
    optString.append(OPTION_LIST);
    optString.append(getoopt::GETOPT_ARGUMENT_DESIGNATOR);
    optString.append(OPTION_DISABLE);
    optString.append(OPTION_ENABLE);
    optString.append(OPTION_REMOVE);
    optString.append(getoopt::GETOPT_ARGUMENT_DESIGNATOR);
    optString.append(OPTION_VERBOSE);
    optString.append(OPTION_FILTER);
    optString.append(getoopt::GETOPT_ARGUMENT_DESIGNATOR);
    optString.append(OPTION_HANDLER);
    optString.append(getoopt::GETOPT_ARGUMENT_DESIGNATOR);
    optString.append(OPTION_NAMESPACE);
    optString.append(getoopt::GETOPT_ARGUMENT_DESIGNATOR);
    optString.append(OPTION_QUERY);
    optString.append(getoopt::GETOPT_ARGUMENT_DESIGNATOR);
    optString.append(OPTION_QUERYLANGUAGE);
    optString.append(getoopt::GETOPT_ARGUMENT_DESIGNATOR);
    optString.append(OPTION_SOURCENAMESPACE);
    optString.append(getoopt::GETOPT_ARGUMENT_DESIGNATOR);
    optString.append(OPTION_DESTINATION);
    optString.append(getoopt::GETOPT_ARGUMENT_DESIGNATOR);
    optString.append(OPTION_MAILTO); //OPTION_MAILTO & OPTION_SNMPTARGETHOST
    optString.append(getoopt::GETOPT_ARGUMENT_DESIGNATOR);
    optString.append(OPTION_MAILCC);
    optString.append(getoopt::GETOPT_ARGUMENT_DESIGNATOR);
    optString.append(OPTION_MAILSUBJECT);
              //OPTION_MAILSUBJECT & OPTION_SNMPSECURITYNAME
    optString.append(getoopt::GETOPT_ARGUMENT_DESIGNATOR);
    optString.append(OPTION_SNMPTARGETHOST);
    optString.append(getoopt::GETOPT_ARGUMENT_DESIGNATOR);
    optString.append(OPTION_SNMPSECURITYNAME);
    optString.append(getoopt::GETOPT_ARGUMENT_DESIGNATOR);
    optString.append(OPTION_SNMPPORTNUMBER);
    optString.append(getoopt::GETOPT_ARGUMENT_DESIGNATOR);
    optString.append(OPTION_SNMPVERSION);
    optString.append(getoopt::GETOPT_ARGUMENT_DESIGNATOR);
    optString.append(OPTION_SNMPENGINEID);
    optString.append(getoopt::GETOPT_ARGUMENT_DESIGNATOR);
    optString.append(OPTION_BATCH);
    optString.append(getoopt::GETOPT_ARGUMENT_DESIGNATOR);
    //
    //  Initialize and parse options
    //
    getoopt options("");
    options.addFlagspec(optString);

    options.addLongFlagspec(LONG_HELP,getoopt::NOARG);
    options.addLongFlagspec(LONG_VERSION,getoopt::NOARG);

    options.parse(argc, argv);

    if (options.hasErrors())
    {
        throw CommandFormatException(options.getErrorStrings()[0]);
    }
    _operationType = OPERATION_TYPE_UNINITIALIZED;


    //
    //  Get options and arguments from the command line
    //
    for (i = options.first(); i <  options.last(); i++)
    {
        if (options[i].getType () == Optarg::LONGFLAG)
        {
            if (options[i].getopt() == LONG_HELP)
            {
                if (_operationType != OPERATION_TYPE_UNINITIALIZED)
                {
                    //
                    // More than one operation option was found
                    //
                    throw UnexpectedOptionException(LONG_HELP);
                }

               _operationType = OPERATION_TYPE_HELP;
            }
            else if (options[i].getopt() == LONG_VERSION)
            {
                if (_operationType != OPERATION_TYPE_UNINITIALIZED)
                {
                    //
                    // More than one operation option was found
                    //
                    throw UnexpectedOptionException(LONG_VERSION);
                }

                _operationType = OPERATION_TYPE_VERSION;
            }
        }
        else if (options [i].getType() == Optarg::REGULAR)
        {
            if (options.isSet(OPTION_CREATE))
            {
                if(ARG_FILTERS == _operationArg)
                {
                    if(filterSet)
                    {
                        throw UnexpectedArgumentException(options[i].Value());
                    }
                    filterNameString = options[i].Value();
                    filterSet = true;
                }
                else if(ARG_HANDLERS == _operationArg)
                {
                    if(handlerSet)
                    {
                        throw UnexpectedArgumentException(options[i].Value());
                    }
                    handlerNameString = options[i].Value();
                    handlerSet = true;
                }
                else
                {
                    //
                    //  The cimsub command has no non-option argument
                    //  options except "c"
                    //
                    throw UnexpectedArgumentException(options[i].Value());
                }
            }
            else
            {
                //
                //  The cimsub command has no non-option argument
                //  options except "c"
                //
                throw UnexpectedArgumentException(options[i].Value());
            }
        }
        else
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
                        // More than one disable subscription option was found
                        //
                        throw DuplicateOptionException(OPTION_DISABLE);
                    }

                    _operationType = OPERATION_TYPE_DISABLE;

                    break;
                }

                case OPTION_ENABLE:
                {
                    if (_operationType != OPERATION_TYPE_UNINITIALIZED)
                    {
                        //
                        // More than one operation option was found
                        //
                        throw UnexpectedOptionException (OPTION_ENABLE);
                    }

                    if (options.isSet(OPTION_ENABLE) > 1)
                    {
                        //
                        // More than one enable option was found
                        //
                        throw DuplicateOptionException (OPTION_ENABLE);
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
                        throw DuplicateOptionException (OPTION_LIST);
                    }
                    _operationType = OPERATION_TYPE_LIST;
                    _operationArg = options[i].Value();
                    break;
                }

                case OPTION_CREATE:
                {
                    if (_operationType != OPERATION_TYPE_UNINITIALIZED)
                    {
                        //
                        // More than one operation option was found
                        //
                        throw UnexpectedOptionException(OPTION_LIST);
                    }

                    if (options.isSet(OPTION_CREATE) > 1)
                    {
                        //
                        // More than two create option was found
                        //
                        throw DuplicateOptionException (OPTION_CREATE);
                    }
                    _operationType = OPERATION_TYPE_CREATE;
                    _operationArg = options[i].Value();
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
                        // More than one remove option was found
                        //
                        throw DuplicateOptionException(OPTION_REMOVE);
                    }
                    _operationType = OPERATION_TYPE_REMOVE;
                    _operationArg = options[i].Value();
                    break;
                }

                case OPTION_VERBOSE:
                {
                   if (_operationType != OPERATION_TYPE_LIST)
                    {
                        //
                        // Unexpected verbose option was found
                        //
                        throw UnexpectedOptionException(OPTION_VERBOSE);
                    }
                    if (options.isSet(OPTION_VERBOSE) > 1)
                    {
                        //
                        // More than one verbose option was found
                        //
                        throw DuplicateOptionException(OPTION_VERBOSE);
                    }

                    _verbose = true;

                    break;
                }

                case OPTION_FILTER:
                {
                    if ((_operationType == OPERATION_TYPE_HELP) ||
                        (_operationType == OPERATION_TYPE_VERSION) ||
                        (_operationType == OPERATION_TYPE_CREATE
                            && ARG_SUBSCRIPTIONS!= _operationArg))
                    {
                        //
                        // Help and version take no options.
                        //
                        throw UnexpectedOptionException(OPTION_FILTER);
                    }
                    if (options.isSet(OPTION_FILTER) > 1)
                    {
                        //
                        // More than one filter option was found
                        //
                        throw DuplicateOptionException(OPTION_FILTER);
                    }
                    filterNameString = options[i].Value();
                    filterSet = true;
                    break;
                }

                case OPTION_HANDLER:
                {
                    if ((_operationType == OPERATION_TYPE_HELP) ||
                        (_operationType == OPERATION_TYPE_VERSION) ||
                        (_operationType == OPERATION_TYPE_CREATE
                            && ARG_SUBSCRIPTIONS!= _operationArg))
                    {
                        //
                        // Help and version take no options.
                        //
                        throw UnexpectedOptionException(OPTION_HANDLER);
                    }
                    if (options.isSet(OPTION_HANDLER) > 1)
                    {
                        //
                        // More than one handler option was found
                        //
                        throw DuplicateOptionException(OPTION_HANDLER);
                    }

                    handlerNameString = options[i].Value();
                    handlerSet = true;
                    break;
                }

                case OPTION_NAMESPACE:
                {
                    if ((_operationType == OPERATION_TYPE_HELP) ||
                        (_operationType == OPERATION_TYPE_VERSION) ||
                        (_operationType == OPERATION_TYPE_CREATE
                            && ARG_SUBSCRIPTIONS!= _operationArg))
                    {
                        //
                        // Help and version take no options.
                        //
                        throw UnexpectedOptionException(OPTION_NAMESPACE);
                    }
                    if (options.isSet(OPTION_NAMESPACE) > 1)
                    {
                        //
                        // More than one namespace option was found
                        //
                        throw DuplicateOptionException(OPTION_NAMESPACE);
                    }

                    if (_operationType == OPERATION_TYPE_BATCH )
                    {
                        _batchNamespace = options[i].Value();
                        _isBatchNamespace = true;
                    }
                    else
                    {
                        String nsNameValue = options[i].Value();
                        _subscriptionNamespace = nsNameValue;
                    }
                    break;
                }
                case OPTION_BATCH:
                {
                   if (_operationType != OPERATION_TYPE_UNINITIALIZED)
                    {
                        // More than one operation option was found
                        throw UnexpectedOptionException(OPTION_BATCH);
                    }
                    if (options.isSet(OPTION_BATCH) > 1)
                    {
                        throw DuplicateOptionException(OPTION_BATCH);
                    }
                    _operationType = OPERATION_TYPE_BATCH;
                    _batchFileName = options[i].Value();
                    break;
                 }
                case OPTION_QUERY:
                {
                    if (_operationType != OPERATION_TYPE_CREATE
                        || ARG_FILTERS != _operationArg)
                    {
                        //
                        // Help and version take no options.
                        //
                        throw UnexpectedOptionException(OPTION_QUERY);
                    }
                    if (options.isSet(OPTION_QUERY) > 1)
                    {
                        //
                        // More than one query option was found
                        //
                        throw DuplicateOptionException(OPTION_QUERY);
                    }

                    String filterQueryValue = options[i].Value();
                    _filterQuery = filterQueryValue;

                    filterQuerySet = true;
                    break;
                }
                case OPTION_QUERYLANGUAGE:
                {
                    if (_operationType != OPERATION_TYPE_CREATE
                        || ARG_FILTERS != _operationArg)
                    {
                        //
                        // Help and version take no options.
                        //
                        throw UnexpectedOptionException(OPTION_QUERYLANGUAGE);
                    }
                    if (options.isSet(OPTION_QUERYLANGUAGE) > 1)
                    {
                        //
                        // More than one query language option was found
                        //
                        throw DuplicateOptionException(OPTION_QUERYLANGUAGE);
                    }

                    String filterQueryLanguageValue = options[i].Value();
                    _filterQueryLanguage = filterQueryLanguageValue;

                    filterQueryLanguageSet = true;
                    break;
                }
                case OPTION_SOURCENAMESPACE:
                {
                    if (_operationType != OPERATION_TYPE_CREATE
                        || ARG_FILTERS != _operationArg)
                    {
                        //
                        // Help and version take no options.
                        //
                        throw UnexpectedOptionException(OPTION_SOURCENAMESPACE);
                    }
                    if (options.isSet(OPTION_SOURCENAMESPACE) > 1)
                    {
                        //
                        // More than one query language option was found
                        //
                        throw DuplicateOptionException(OPTION_SOURCENAMESPACE);
                    }
                    _filterNSFlag = false;
                    String filterSourceNamespaceValue = options[i].Value();
                    String filterNS = filterSourceNamespaceValue;
                    Boolean sourceNamespacesProperty = false;
                    csvStringParse strSNS (filterNS, ',');
                    while (strSNS.more())
                    {
                        _filterSourceNamespaces.append(strSNS.next());
                    }
                    if ( _filterSourceNamespaces.size() > 1 ||
                          filterNS[filterNS.size()-1] == ',')
                    {
                         sourceNamespacesProperty = true;
                    }
                    _filterNSFlag = sourceNamespacesProperty;
                    filterSourceNamespaceSet = true;
                    break;
                }
                case OPTION_DESTINATION:
                {
                    if (_operationType != OPERATION_TYPE_CREATE
                        || ARG_HANDLERS != _operationArg)
                    {
                        //
                        // Help and version take no options.
                        //
                        throw UnexpectedOptionException(OPTION_DESTINATION);
                    }
                    if (options.isSet(OPTION_DESTINATION) > 1)
                    {
                        //
                        // More than one query language option was found
                        //
                        throw DuplicateOptionException(OPTION_DESTINATION);
                    }

                    String handlerDestinationValue = options[i].Value();
                    _handlerDestination = handlerDestinationValue;

                    handlerDestinationSet = true;
                    break;
                }
                case OPTION_MAILCC:
                {
                    if (_operationType != OPERATION_TYPE_CREATE
                        || ARG_HANDLERS != _operationArg)
                    {
                        //
                        // Help and version take no options.
                        //
                        throw UnexpectedOptionException(OPTION_MAILCC);
                    }
                    if (options.isSet(OPTION_MAILCC) > 1)
                    {
                        //
                        // More than one query language option was found
                        //
                        throw DuplicateOptionException(OPTION_MAILCC);
                    }

                    String handlerMailCcValue = options[i].Value();
                    _handlerMailCc = handlerMailCcValue;

                    handlerMailCcSet = true;
                    break;
                }
                case OPTION_MAILTO: //OPTION_MAILTO && OPTION_SNMPTARGETHOST
                {
                    if (_operationType != OPERATION_TYPE_CREATE
                        || ARG_HANDLERS != _operationArg)
                    {
                        //
                        // Help and version take no options.
                        //
                        throw UnexpectedOptionException(OPTION_MAILTO);
                    }
                    if (options.isSet(OPTION_MAILTO) > 1)
                    {
                        //
                        // More than one query language option was found
                        //
                        throw DuplicateOptionException(OPTION_MAILTO);
                    }

                    String tmpValue = options[i].Value();
                    _handlerMailTo= tmpValue;
                    handlerMailToSet = true;
                    _handlerSNMPTartgetHost = tmpValue;
                    handlerSnmpTargetHostSet = true;
                    break;
                }
                case OPTION_MAILSUBJECT:
                {
                  //OPTION_MAILSUBJECT && OPTION_SNMPSECURITYNAME
                    if (_operationType != OPERATION_TYPE_CREATE
                        || ARG_HANDLERS != _operationArg)
                    {
                        //
                        // Help and version take no options.
                        //
                        throw UnexpectedOptionException(OPTION_MAILSUBJECT);
                    }
                    if (options.isSet(OPTION_MAILSUBJECT) > 1)
                    {
                        //
                        // More than one query language option was found
                        //
                        throw DuplicateOptionException(OPTION_MAILSUBJECT);
                    }

                    String tmpValue = options[i].Value();
                    _handlerMailSubject = tmpValue;
                    handlerMailSubjectSet = true;
                    _handlerSNMPSecurityName = tmpValue;
                    handlerSnmpSecurityNameSet = true;
                    break;
                }
                case OPTION_SNMPTARGETHOST:
                {
                    if (_operationType != OPERATION_TYPE_CREATE
                        || ARG_HANDLERS != _operationArg)
                    {
                        //
                        // Help and version take no options.
                        //
                        throw UnexpectedOptionException(OPTION_SNMPTARGETHOST);
                    }
                    if (options.isSet(OPTION_SNMPTARGETHOST) > 1)
                    {
                        //
                        // More than one query language option was found
                        //
                        throw DuplicateOptionException(OPTION_SNMPTARGETHOST);
                    }

                    String tmpValue = options[i].Value();
                    _handlerSNMPTartgetHost = tmpValue;
                    handlerSnmpTargetHostSet = true;
                    break;
                }
                case OPTION_SNMPSECURITYNAME:
                {
                    if (_operationType != OPERATION_TYPE_CREATE
                        || ARG_HANDLERS != _operationArg)
                    {
                        //
                        // Help and version take no options.
                        //
                        throw UnexpectedOptionException(
                             OPTION_SNMPSECURITYNAME);
                    }
                    if (options.isSet(OPTION_SNMPSECURITYNAME) > 1)
                    {
                        //
                        // More than one query language option was found
                        //
                        throw DuplicateOptionException(OPTION_SNMPSECURITYNAME);
                    }

                    String tmpValue = options[i].Value();

                    _handlerSNMPSecurityName = tmpValue;
                    handlerSnmpSecurityNameSet = true;
                    break;
                }
                case OPTION_SNMPPORTNUMBER:
                {
                    if (_operationType != OPERATION_TYPE_CREATE
                        || ARG_HANDLERS != _operationArg)
                    {
                        //
                        // Help and version take no options.
                        //
                        throw UnexpectedOptionException(OPTION_SNMPPORTNUMBER);
                    }
                    if (options.isSet(OPTION_SNMPPORTNUMBER) > 1)
                    {
                        //
                        // More than one query language option was found
                        //
                        throw DuplicateOptionException(OPTION_SNMPPORTNUMBER);
                    }

                    String handlerPortNumberValue = options[i].Value();

                    for( Uint32 i = 0; i < handlerPortNumberValue.size(); i++)
                    {
                        if(!isdigit(handlerPortNumberValue[i]))
                        {
                            throw InvalidOptionArgumentException(
                                handlerPortNumberValue,
                                OPTION_SNMPPORTNUMBER);
                        }
                    }
                    StringConversion::stringToUnsignedInteger(
                        handlerPortNumberValue.getCString(),
                        _handlerSNMPPortNumber);
                    handlerSnmpPortNumberSet = true;
                    break;
                }
                case OPTION_SNMPVERSION:
                {
                    if (_operationType != OPERATION_TYPE_CREATE
                        || ARG_HANDLERS != _operationArg)
                    {
                        //
                        // Help and version take no options.
                        //
                        throw UnexpectedOptionException(OPTION_SNMPVERSION);
                    }
                    if (options.isSet(OPTION_SNMPVERSION) > 1)
                    {
                        //
                        // More than one query language option was found
                        //
                        throw DuplicateOptionException(OPTION_SNMPVERSION);
                    }

                    String handlerSnmpVersionValue = options[i].Value();

                    for( Uint32 i = 0; i < handlerSnmpVersionValue.size(); i++)
                    {
                        if(!isdigit(handlerSnmpVersionValue[i]))
                        {
                            throw InvalidOptionArgumentException(
                                handlerSnmpVersionValue,
                                OPTION_SNMPPORTNUMBER);
                        }
                    }
                    StringConversion::stringToUnsignedInteger(
                        handlerSnmpVersionValue.getCString(),
                        _handlerSNMPVersion);
                    handlerSnmpVersionSet = true;
                    break;
                }
                case OPTION_SNMPENGINEID:
                {
                    if (_operationType != OPERATION_TYPE_CREATE
                        || ARG_HANDLERS != _operationArg)
                    {
                        //
                        // Help and version take no options.
                        //
                        throw UnexpectedOptionException(OPTION_SNMPENGINEID);
                    }
                    if (options.isSet(OPTION_SNMPENGINEID) > 1)
                    {
                        //
                        // More than one query language option was found
                        //
                        throw DuplicateOptionException(OPTION_SNMPENGINEID);
                    }

                    String handlerSnmpEngineIdValue = options[i].Value();
                    _handlerSNMPEngineID = handlerSnmpEngineIdValue;

                    handlerSnmpEngineIdSet = true;
                    break;
                }
                default:
                {
                    throw UnexpectedOptionException (c);
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
            MSG_PATH, REQUIRED_OPTION_MISSING_KEY, REQUIRED_OPTION_MISSING));
    }

    if (_operationType == OPERATION_TYPE_LIST)
    {
        if (_operationArg == ARG_FILTERS)
        {
            if (handlerSet)
            {
                //
                // Wrong option for this operation
                // was found
                //
                throw UnexpectedOptionException(OPTION_HANDLER);
            }
        }
        else if (_operationArg == ARG_HANDLERS)
        {
            if (filterSet)
            {
                //
                // Wrong option for this operation was found
                //
                throw UnexpectedOptionException
                    (OPTION_FILTER);
            }
        }
        else if (_operationArg != ARG_SUBSCRIPTIONS)
        {
            //
            // A wrong option argument for this
            // operation was found
            //
            throw InvalidOptionArgumentException(
                _operationArg, OPTION_LIST);
        }
    }

    if (_operationType == OPERATION_TYPE_DISABLE)
    {
        if (!filterSet)
        {
            throw MissingOptionException(OPTION_FILTER);
        }

        if (!handlerSet)
        {
            throw MissingOptionException(OPTION_HANDLER);
        }
    }

    if (_operationType == OPERATION_TYPE_ENABLE)
    {
        if (!filterSet)
        {
            throw MissingOptionException(OPTION_FILTER);
        }

        if (!handlerSet)
        {
            throw MissingOptionException(OPTION_HANDLER);
        }
    }

    if (_operationType == OPERATION_TYPE_REMOVE)
    {
        if (_operationArg == ARG_FILTERS)
        {
            if (handlerSet)
            {
                //
                // Wrong option for this
                // operation was found
                //
                throw UnexpectedOptionException(
                    OPTION_HANDLER);
            }
        }
        else
        {
            if (_operationArg == ARG_HANDLERS)
            {
                if (filterSet)
                {
                    //
                    // Wrong option for this operation was found
                    //
                    throw UnexpectedOptionException(OPTION_FILTER);
                }
            }
            else
            {
                if ((_operationArg != ARG_SUBSCRIPTIONS) &&
                    (_operationArg != ARG_ALL))
                {
                    //
                    // A wrong option argument for this operation
                    // was found
                    //
                    throw InvalidOptionArgumentException
                        (_operationArg, OPTION_REMOVE);
                }
            }
        }
        if ((_operationArg == ARG_SUBSCRIPTIONS) ||
            (_operationArg == ARG_ALL) ||
            (_operationArg == ARG_FILTERS))
        {
          if (!filterSet)
            {
                throw MissingOptionException(OPTION_FILTER);
            }
        }
        if ((_operationArg == ARG_SUBSCRIPTIONS) ||
            (_operationArg == ARG_ALL) ||
            (_operationArg == ARG_HANDLERS))
        {
            if (!handlerSet)
            {
                throw MissingOptionException(OPTION_HANDLER);
            }
        }
    }

    if (_operationType == OPERATION_TYPE_CREATE)
    {
        if(ARG_FILTERS == _operationArg)
        {
            if (!filterQuerySet)
            {
                throw MissingOptionException(OPTION_QUERY);
            }
            if (!filterSet)
            {
                throw CommandFormatException(
                    localizeMessage(
                         MSG_PATH,
                         REQUIRED_OPTION_MISSING_KEY,
                         REQUIRED_OPTION_MISSING)
                    );
            }
        }
        else if(ARG_HANDLERS == _operationArg)
        {
            if (!handlerSet)
            {
                throw CommandFormatException(
                    localizeMessage(
                        MSG_PATH,
                        REQUIRED_OPTION_MISSING_KEY,
                        REQUIRED_OPTION_MISSING)
                    );
            }
            String handlerName;
            String handlerNamespace;
            _handlerCreationClass =
                 PEGASUS_CLASSNAME_LSTNRDST_CIMXML.getString();
            _parseHandlerName(handlerNameString, handlerName, handlerNamespace,
                 _handlerCreationClass);
            CIMName handlerCreationClass(_handlerCreationClass);

            if(handlerCreationClass != PEGASUS_CLASSNAME_LSTNRDST_CIMXML &&
                handlerCreationClass != PEGASUS_CLASSNAME_INDHANDLER_CIMXML &&
                handlerCreationClass != PEGASUS_CLASSNAME_LSTNRDST_SYSTEM_LOG &&
                handlerCreationClass != PEGASUS_CLASSNAME_LSTNRDST_EMAIL &&
                handlerCreationClass != PEGASUS_CLASSNAME_INDHANDLER_SNMP)
            {
                throw UnexpectedArgumentException(handlerNameString);
            }

            if (handlerDestinationSet)
            {
                if (handlerCreationClass != PEGASUS_CLASSNAME_LSTNRDST_CIMXML &&
                    handlerCreationClass != PEGASUS_CLASSNAME_INDHANDLER_CIMXML)
                {
                    //
                    // Wrong option for this operation was found
                    //
                    throw UnexpectedOptionException(OPTION_DESTINATION);
                }
            }
            else if (handlerCreationClass ==
                 PEGASUS_CLASSNAME_LSTNRDST_CIMXML ||
                 handlerCreationClass == PEGASUS_CLASSNAME_INDHANDLER_CIMXML)
            {
                throw MissingOptionException(OPTION_DESTINATION);
            }

            if (
                handlerMailToSet ||
                handlerSnmpTargetHostSet)
            {
                if (
                    handlerCreationClass != PEGASUS_CLASSNAME_LSTNRDST_EMAIL &&
                    handlerCreationClass != PEGASUS_CLASSNAME_INDHANDLER_SNMP)
                {
                    //
                    // Wrong option for this operation was found
                    //
                    throw UnexpectedOptionException(OPTION_SNMPTARGETHOST);
                }
            }
            else if(
                handlerCreationClass == PEGASUS_CLASSNAME_LSTNRDST_EMAIL ||
                handlerCreationClass == PEGASUS_CLASSNAME_INDHANDLER_SNMP)
            {
                throw MissingOptionException(OPTION_SNMPTARGETHOST);
            }
            if (handlerMailCcSet)
            {
                if (handlerCreationClass != PEGASUS_CLASSNAME_LSTNRDST_EMAIL)
                {
                    //
                    // Wrong option for this operation was found
                    //
                    throw UnexpectedOptionException(OPTION_MAILCC);
                }
            }
            if (
                handlerMailSubjectSet ||
                handlerSnmpSecurityNameSet)
            {
                if (
                    handlerCreationClass != PEGASUS_CLASSNAME_LSTNRDST_EMAIL &&
                    handlerCreationClass != PEGASUS_CLASSNAME_INDHANDLER_SNMP)
                {
                    //
                    // Wrong option for this operation was found
                    //
                    throw UnexpectedOptionException(OPTION_SNMPSECURITYNAME);
               }
            }
          else if(handlerCreationClass == PEGASUS_CLASSNAME_LSTNRDST_EMAIL)
            {
                throw MissingOptionException(OPTION_MAILSUBJECT);
            }
            if (handlerSnmpPortNumberSet)
            {
                if (handlerCreationClass != PEGASUS_CLASSNAME_INDHANDLER_SNMP)
                {
                    //
                    // Wrong option for this operation was found
                    //
                    throw UnexpectedOptionException(OPTION_SNMPPORTNUMBER);
                }
            }

            if (handlerSnmpVersionSet)
            {
                if (handlerCreationClass != PEGASUS_CLASSNAME_INDHANDLER_SNMP)
                {
                    //
                    // Wrong option for this operation was found
                    //
                    throw UnexpectedOptionException(OPTION_SNMPVERSION);
                }
            }
            else if (handlerCreationClass == PEGASUS_CLASSNAME_INDHANDLER_SNMP)
            {
                throw MissingOptionException(OPTION_SNMPVERSION);
            }

            if (handlerSnmpEngineIdSet)
            {
                if (handlerCreationClass != PEGASUS_CLASSNAME_INDHANDLER_SNMP)
                {
                    //
                    // Wrong option for this operation was found
                    //
                    throw UnexpectedOptionException(OPTION_SNMPENGINEID);
                }
            }
        }
        else if(ARG_SUBSCRIPTIONS== _operationArg)
        {
            if (!handlerSet)
            {
                throw MissingOptionException(OPTION_HANDLER);
            }
            if (!filterSet)
            {
                throw MissingOptionException(OPTION_FILTER);
            }
        }
        else
        {
            //
            // A wrong option argument for this operation
            // was found
            //
            throw InvalidOptionArgumentException
                (_operationArg, OPTION_CREATE);
        }
    }

    if (filterSet)
    {
        _parseFilterName(filterNameString, _filterName, _filterNamespace);
    }
    if (handlerSet)
    {
        _parseHandlerName(handlerNameString, _handlerName, _handlerNamespace,
            _handlerCreationClass);
    }
}

/**
    Executes the command and writes the results to the PrintWriters.
*/
Uint32 CIMSubCommand::execute(
    ostream& outPrintWriter,
    ostream& errPrintWriter)
{
    Array<CIMNamespaceName> namespaceNames;
    //
    // The CIM Client reference
    //

    if (_operationType == OPERATION_TYPE_UNINITIALIZED)
    {
        //
        // The command was not initialized
        //
        return 1;
    }
    else if (_operationType == OPERATION_TYPE_HELP)
    {
        errPrintWriter << usage << endl;
        return (RC_SUCCESS);
    }
    else if (_operationType == OPERATION_TYPE_VERSION)
    {
         errPrintWriter << "Version " << PEGASUS_PRODUCT_VERSION << endl;
        return (RC_SUCCESS);
    }

    try
    {
        // Construct the CIMClient and set to request server messages
        // in the default language of this client process.
        _client.reset(new CIMClient);
        _client->setRequestDefaultLanguages();
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
    catch (const Exception&)
    {
        errPrintWriter << localizeMessage(MSG_PATH,
            CIMOM_NOT_RUNNING_KEY,
            CIMOM_NOT_RUNNING) << endl;
        return (RC_CONNECTION_FAILED);
    }
    //
    // Perform the requested operation
    //
    try
    {
        CIMNamespaceName subscriptionNS;
        CIMNamespaceName filterNS = CIMNamespaceName();
        CIMNamespaceName handlerNS = CIMNamespaceName();
        if (_subscriptionNamespace != String::EMPTY)
        {
            subscriptionNS = _subscriptionNamespace;
            if(OPERATION_TYPE_LIST == _operationType &&
                _operationArg != ARG_SUBSCRIPTIONS)
            {
                filterNS = subscriptionNS;
                handlerNS = subscriptionNS;
            }
        }

        if (_filterNamespace != String::EMPTY)
        {
            filterNS = _filterNamespace;
        }

        if (_handlerNamespace != String::EMPTY)
        {
            handlerNS = _handlerNamespace;
        }

        switch (_operationType)
        {
            case OPERATION_TYPE_ENABLE:
                return(_findAndModifyState(STATE_ENABLED,
                    subscriptionNS, _filterName, filterNS,
                    _handlerName, handlerNS, _handlerCreationClass,
                    outPrintWriter));

            case OPERATION_TYPE_DISABLE:
                return (_findAndModifyState(STATE_DISABLED,
                    subscriptionNS, _filterName, filterNS,
                    _handlerName, handlerNS, _handlerCreationClass,
                    outPrintWriter));

            case OPERATION_TYPE_LIST:
                if (_operationArg == ARG_SUBSCRIPTIONS)
                {
                    if (subscriptionNS.isNull())
                    {
                        _getAllNamespaces(namespaceNames);
                    }
                    else
                    {
                        namespaceNames.append(subscriptionNS);
                    }
                    _listSubscriptions(namespaceNames, _filterName,
                        filterNS, _handlerName, handlerNS,
                        _handlerCreationClass, _verbose, outPrintWriter,
                        errPrintWriter);
                }
                else if (_operationArg == ARG_FILTERS)
                {
                    if (filterNS.isNull())
                    {
                        _getAllNamespaces(namespaceNames);
                    }
                    else
                    {
                        namespaceNames.append(filterNS);
                    }
                    _listFilters(_filterName, _verbose,
                        namespaceNames, outPrintWriter,
                        errPrintWriter);
                }
                else if (_operationArg == ARG_HANDLERS)
                {
                     if (handlerNS.isNull())
                     {
                          _getAllNamespaces(namespaceNames);
                     }
                     else
                     {
                          namespaceNames.append(handlerNS);
                     }
                          _listHandlers(_handlerName, namespaceNames,
                               _handlerCreationClass, _verbose,
                               outPrintWriter, errPrintWriter);
                }
            break;

        case OPERATION_TYPE_REMOVE:
            if ((_operationArg == ARG_SUBSCRIPTIONS) || (_operationArg ==
                ARG_ALL))
            {
                Boolean removeAll = false;
                if (_operationArg == ARG_ALL)
                {
                    removeAll = true;
                }
                return _removeSubscription(subscriptionNS,
                    _filterName, filterNS, _handlerName, handlerNS,
                    _handlerCreationClass, removeAll, outPrintWriter,
                    errPrintWriter);
            }
            else if (_operationArg == ARG_FILTERS)
            {
                return (_removeFilter(_filterName, filterNS,
                    outPrintWriter, errPrintWriter));
            }
            else
            {
                PEGASUS_ASSERT (_operationArg == ARG_HANDLERS);
                return _removeHandler(_handlerName,
                    handlerNS, _handlerCreationClass, outPrintWriter,
                    errPrintWriter);
            }
            break;
        case OPERATION_TYPE_CREATE:
            if ((_operationArg == ARG_SUBSCRIPTIONS) )
            {
                return _createSubscription(subscriptionNS,
                    _filterName, filterNS, _handlerName, handlerNS,
                    _handlerCreationClass, outPrintWriter,
                    errPrintWriter);
            }
            else if (_operationArg == ARG_FILTERS)
            {
                return (_createFilter(_filterName, filterNS,
                    _filterQuery,_filterQueryLanguage,_filterSourceNamespaces,
                    outPrintWriter, errPrintWriter));
            }
            else
            {
                PEGASUS_ASSERT (_operationArg == ARG_HANDLERS);
                CIMName handlerClass(_handlerCreationClass);

                if (handlerClass == PEGASUS_CLASSNAME_LSTNRDST_CIMXML
                    || handlerClass == PEGASUS_CLASSNAME_INDHANDLER_CIMXML)
                {
                    return _createCimXmlHandler(_handlerName,
                        handlerNS, _handlerCreationClass, _handlerDestination,
                        outPrintWriter, errPrintWriter);
                }
               else if (handlerClass == PEGASUS_CLASSNAME_LSTNRDST_SYSTEM_LOG)
                {
                    return _createSystemLogHandler(_handlerName,
                        handlerNS, _handlerCreationClass,
                        outPrintWriter, errPrintWriter);
                }
                else if (handlerClass == PEGASUS_CLASSNAME_LSTNRDST_EMAIL)
                {
                    Array<String> mailToList;
                    Array<String> mailCcList;
                    {
                        csvStringParse strl(_handlerMailTo, ',');
                        while(strl.more())
                        {
                            mailToList.append(strl.next());
                        }
                    }
                    {
                        csvStringParse strl(_handlerMailCc, ',');
                        while(strl.more())
                        {
                            mailCcList.append(strl.next());
                        }
                    }
                    return _createEmailHandler(_handlerName,
                        handlerNS, _handlerCreationClass,
                        mailToList,
                        mailCcList,
                        _handlerMailSubject,
                        outPrintWriter, errPrintWriter);
                }
                else if(handlerClass == PEGASUS_CLASSNAME_INDHANDLER_SNMP)
                {
                    return _createSnmpMapperHandler(_handlerName,
                        handlerNS, _handlerCreationClass,
                        _handlerSNMPTartgetHost,
                        _handlerSNMPPortNumber,
                        _handlerSNMPVersion,
                        _handlerSNMPSecurityName,
                        _handlerSNMPEngineID,
                        outPrintWriter, errPrintWriter);
                }
                else
                {
                    PEGASUS_UNREACHABLE(PEGASUS_ASSERT(0);)
                }
            }
            break;
        case OPERATION_TYPE_BATCH:
        {
            ifstream batchFile(_batchFileName.getCString());
            char buffer[1024];
            char* argv[128];
            String tempString;
            Uint32 argc = 0;
            String bLine;
            if (!batchFile)
            {
                throw CannotOpenFile(_batchFileName);
            }
            // parsing batch file line by line
            while (batchFile.getline(buffer, sizeof(buffer)))
            {
                 bLine = buffer;
                 Uint32 start = 0;
                 // ignore whitespaces
                 while (((bLine[start]) == ' ') &&
                     (start < bLine.size()))
                 {
                     start++;
                 }
                 // ignore comment and empty line
                 if (bLine[start] != '#' &&  bLine[start] != '\000')
                 {
                        String batchLine = bLine.subString(start,bLine.size());
                        csvStringParse BatchCmd(batchLine,' ');
                        while (BatchCmd.more())
                        {
                           tempString = BatchCmd.next();
                           if( tempString.size() != 0 )
                           {
                              argv[argc] = strdup(tempString.getCString());
                              argc++;
                           }
                        }
                        try
                        {
                            // get all options
                            setCommand(outPrintWriter,errPrintWriter,argc,argv);
                        }
                        catch(CommandFormatException& e)
                        {
                           errPrintWriter << e.getMessage()<<"\n";
                        }
                        catch(CIMException& e)
                        {
                           errPrintWriter << e.getMessage()<<"\n";
                        }
                        catch(...)
                        {

                        }
                        try
                        {
                            // execute the command
                            execute(outPrintWriter,errPrintWriter);
                        }
                        catch(CIMException& e)
                        {
                           errPrintWriter << e.getMessage()<<"\n";
                        }
                        catch(...)
                        {

                        }
                        for (Uint32 ac = 0; ac < argc ;ac++ )
                        {
                            free(argv[ac]);
                        }
                        argc = 0;
                        if (_verbose)
                        {
                            _verbose = false;
                        }
                        // reset values before next command
                        _subscriptionNamespace.clear();
                        _filterNamespace.clear();
                        _handlerNamespace.clear();
                        _filterName.clear();
                        _handlerName.clear();
                        outPrintWriter << "\n";
                  }
             }
             _isBatchNamespace = false;
          break;
        }
        default:
            PEGASUS_UNREACHABLE(PEGASUS_ASSERT(0);)
            break;
        }
    }

    catch (CIMException& e)
    {
        CIMStatusCode code = e.getCode();
        if (code == CIM_ERR_NOT_FOUND)
        {
            errPrintWriter << e.getMessage() << endl;
            return RC_OBJECT_NOT_FOUND;
        }
        else if (code == CIM_ERR_INVALID_NAMESPACE)
        {
            errPrintWriter << e.getMessage() << endl;
            return RC_NAMESPACE_NONEXISTENT;
        }
        else if (code == CIM_ERR_NOT_SUPPORTED)
        {
            errPrintWriter << e.getMessage() << endl;
            return RC_OPERATION_NOT_SUPPORTED;
        }
        else if (code == CIM_ERR_ACCESS_DENIED)
        {
            errPrintWriter << e.getMessage() << endl;
            return RC_ACCESS_DENIED;
        }
        else
        {
            errPrintWriter << e.getMessage() << endl;
        }
        return (RC_ERROR);
    }
    catch (ConnectionTimeoutException& e)
    {
        errPrintWriter << e.getMessage() << endl;
        return (RC_CONNECTION_TIMEOUT);
    }
    catch (Exception& e)
    {
        errPrintWriter << e.getMessage() << endl;
        return (RC_ERROR);
    }
    return (RC_SUCCESS);
}

//
// parse the filter option string
//
void CIMSubCommand::_parseFilterName(
    const String& filterNameString,
    String& filterName,
    String& filterNamespace)
{
    Uint32 nsDelimiterIndex = filterNameString.find(
        DELIMITER_NAMESPACE);
    if (nsDelimiterIndex == PEG_NOT_FOUND)
    {
        filterName = filterNameString;
        filterNamespace.clear();
    }
    else
    {
        if((nsDelimiterIndex == 0 ) ||
            ((nsDelimiterIndex + 1) ==
            filterNameString.size()))
        {
            // Invalid - either no name or no class
            throw InvalidOptionArgumentException(
                filterNameString, OPTION_FILTER);
        }
        // Parse the filter namespace and filter name
        filterNamespace = filterNameString.subString(0,
            nsDelimiterIndex);
        filterName = filterNameString.subString(
            nsDelimiterIndex+1);
    }
}

//
// parse the handler option string
//
void CIMSubCommand::_parseHandlerName(
    const String& handlerString,
    String& handlerName,
    String& handlerNamespace,
    String& handlerCreationClass)
{
    Uint32 nsDelimiterIndex = handlerString.find (
        DELIMITER_NAMESPACE);
    if (nsDelimiterIndex == PEG_NOT_FOUND)
    {
        //
        // handler namespace was not found
        //
        handlerNamespace.clear();
        //
        // Check for handler class
        //
        Uint32 classDelimiterIndex = handlerString.find (
        DELIMITER_HANDLER_CLASS);
        if (classDelimiterIndex == PEG_NOT_FOUND)
        {
            handlerName = handlerString;
        }
        else
        {
            //
            // Parse creation class and handler name
            //
            if ((classDelimiterIndex == 0) ||
                ((classDelimiterIndex + 1) ==
                handlerString.size()))
            {
                // Invalid - either no name or no class
                throw InvalidOptionArgumentException(
                    handlerString, OPTION_HANDLER);
            }
            handlerCreationClass =
                handlerString.subString (0,
                classDelimiterIndex);
            handlerName = handlerString.subString(
                classDelimiterIndex+1);
        }
    }
    else
    {
        //
        // handler namespace was found
        //

        // Parse the handler namespace and handler name
        handlerNamespace = handlerString.subString(0,
            nsDelimiterIndex);
        if ((nsDelimiterIndex == 0) ||
            ((nsDelimiterIndex + 1) ==
            handlerString.size()))
        {
            // Invalid - either no name or no class
            throw InvalidOptionArgumentException(
                handlerString, OPTION_HANDLER);
        }
        Uint32 classDelimiterIndex = handlerString.find (
            DELIMITER_HANDLER_CLASS);
        if (classDelimiterIndex == PEG_NOT_FOUND)
        {

            // No creation class specified, just the handler name

            handlerName = handlerString.subString(nsDelimiterIndex+1);
        }
        else
        {
            if ((nsDelimiterIndex + 1 ) == classDelimiterIndex)
            {
                // Invalid - no class
                throw InvalidOptionArgumentException(
                    handlerString, OPTION_HANDLER);
            }

            if ((classDelimiterIndex + 1) ==
                handlerString.size())
            {
                // Invalid - no name
                throw InvalidOptionArgumentException(
                    handlerString, OPTION_HANDLER);
            }

            // Parse the handler class and name

            Uint32 slen = classDelimiterIndex - nsDelimiterIndex - 1;
            handlerCreationClass =
                handlerString.subString(nsDelimiterIndex+1, slen);
            handlerName = handlerString.subString(classDelimiterIndex+1);
        }
    }
}


Uint32 CIMSubCommand::_createSubscription(
    const CIMNamespaceName& subscriptionNamespace,
    const String& filterName,
    const CIMNamespaceName& filterNamespace,
    const String& handlerName,
    const CIMNamespaceName& handlerNamespace,
    const String& handlerCreationClass,
    ostream& outPrintWriter,
    ostream& errPrintWriter)
{
    Array<CIMObjectPath> allSubPathFound;
    CIMNamespaceName filterNS = _DEFAULT_SUBSCRIPTION_NAMESPACE;
    CIMNamespaceName handlerNS = _DEFAULT_SUBSCRIPTION_NAMESPACE;
    CIMNamespaceName subscriptionNS = _DEFAULT_SUBSCRIPTION_NAMESPACE;
    CIMObjectPath filterPath;
    CIMObjectPath handlerPath;
    String handlerCreationCls = handlerCreationClass;
    if (_isBatchNamespace )
    {
        subscriptionNS = _batchNamespace;
    }
    else if (!subscriptionNamespace.isNull())
    {
        subscriptionNS = subscriptionNamespace;
    }
    if (_isBatchNamespace )
    {
        filterNS = _batchNamespace;
    }
    else if (!filterNamespace.isNull())
    {
        filterNS = filterNamespace;
    }
    if (_isBatchNamespace )
    {
        handlerNS = _batchNamespace;
    }
    else if (!handlerNamespace.isNull())
    {
        handlerNS = handlerNamespace;
    }

    if (handlerCreationCls == String::EMPTY)
    {
        handlerCreationCls = PEGASUS_CLASSNAME_LSTNRDST_CIMXML.getString();
    }

    if (!_findFilter(filterName, filterNS, errPrintWriter, filterPath))
    {
        outPrintWriter << localizeMessage(MSG_PATH,
            FILTER_NOT_FOUND_KEY,
            FILTER_NOT_FOUND_FAILURE) << endl;
        return (RC_OBJECT_NOT_FOUND);
    }
    filterPath.setNameSpace(filterNS);
    if (!_findHandler(handlerName, handlerNS, handlerCreationCls,
        errPrintWriter, handlerPath))
    {
        outPrintWriter << localizeMessage(MSG_PATH,
            HANDLER_NOT_FOUND_KEY,
            HANDLER_NOT_FOUND_FAILURE) << endl;
        return RC_OBJECT_NOT_FOUND;
    }
    handlerPath.setNameSpace(handlerNS);

    CIMInstance subscriptionInstance (PEGASUS_CLASSNAME_INDSUBSCRIPTION);
    subscriptionInstance.addProperty (CIMProperty (CIMName ("Filter"),
        filterPath, 0, PEGASUS_CLASSNAME_INDFILTER));
    subscriptionInstance.addProperty (CIMProperty (CIMName ("Handler"),
        handlerPath, 0, PEGASUS_CLASSNAME_INDHANDLER_CIMXML));
    subscriptionInstance.addProperty (CIMProperty
        (CIMName ("SubscriptionState"), CIMValue ((Uint16) 2)));

    _client->createInstance (subscriptionNS,
        subscriptionInstance);

    return (RC_SUCCESS);
}

//
// remove an existing subscription instance
//
Uint32 CIMSubCommand::_removeSubscription(
    const CIMNamespaceName& subscriptionNamespace,
    const String& filterName,
    const CIMNamespaceName& filterNamespace,
    const String& handlerName,
    const CIMNamespaceName& handlerNamespace,
    const String& handlerCreationClass,
    const Boolean removeAll,
    ostream& outPrintWriter,
    ostream& errPrintWriter)
{
    Array<CIMObjectPath> allSubPathFound;
    CIMNamespaceName filterNS;
    CIMNamespaceName handlerNS;
    CIMNamespaceName subscriptionNS = _DEFAULT_SUBSCRIPTION_NAMESPACE;
    if (_isBatchNamespace )
    {
        subscriptionNS = _batchNamespace;
    }
    else if (!subscriptionNamespace.isNull())
    {
        subscriptionNS = subscriptionNamespace;
    }
    if (_isBatchNamespace )
    {
        filterNS = _batchNamespace;
    }
    else if (!filterNamespace.isNull())
    {
        filterNS = filterNamespace;
    }
    if (_isBatchNamespace )
    {
        handlerNS = _batchNamespace;
    }
    else if (!handlerNamespace.isNull())
    {
        handlerNS = handlerNamespace;
    }

    if (_findSubscription(subscriptionNS, filterName, filterNS,
        handlerName, handlerNS, handlerCreationClass, allSubPathFound))
    {
        for(Uint32 i = 0;i<allSubPathFound.size();i++)
        {
                CIMObjectPath subPathFound = allSubPathFound[i];
                if (!removeAll)
                {
                    try
                    {
                        _client->deleteInstance(subscriptionNS, subPathFound);
                    }
                    catch(const Exception& e)
                    {
                        errPrintWriter << e.getMessage() << endl;
                    }
                }
                else
                {
                    // Delete subscription, filter and handler
                    CIMObjectPath filterRef, handlerRef;
                    //
                    //  Get the subscription Filter and Handler ObjectPaths
                    //
                    Array<CIMKeyBinding> keys = subPathFound.getKeyBindings();
                    for( Uint32 j=0; j < keys.size(); j++)
                    {
                        if (keys[j].getName().equal(
                            PEGASUS_PROPERTYNAME_FILTER))
                        {
                            filterRef = keys[j].getValue();
                        }
                        if (keys[j].getName().equal(
                            PEGASUS_PROPERTYNAME_HANDLER))
                        {
                            handlerRef = keys[j].getValue();
                        }
                    }
                    try
                    {
                        _client->deleteInstance(subscriptionNS, subPathFound);
                    }
                    catch(const Exception& e)
                    {
                        errPrintWriter << e.getMessage() << endl;
                    }
                    try
                    {
                        CIMNamespaceName tmpFilterNS = filterNS;
                        if(tmpFilterNS.isNull())
                        {
                            tmpFilterNS = filterRef.getNameSpace();
                        }
                        _client->deleteInstance(tmpFilterNS, filterRef);
                    }
                    catch(const Exception& e)
                    {
                        errPrintWriter << e.getMessage() << endl;
                    }
                    try
                    {
                        CIMNamespaceName tmpFilterNS = handlerNS;
                        if(tmpFilterNS.isNull())
                        {
                            tmpFilterNS = handlerRef.getNameSpace();
                        }
                        _client->deleteInstance(tmpFilterNS, handlerRef);
                    }
                    catch(const Exception& e)
                    {
                        errPrintWriter << e.getMessage() << endl;
                    }
                }
        }
        return (RC_SUCCESS);
    }
    else
    {
        outPrintWriter << localizeMessage(MSG_PATH,
            SUBSCRIPTION_NOT_FOUND_KEY,
            SUBSCRIPTION_NOT_FOUND_FAILURE) << endl;
        return (RC_OBJECT_NOT_FOUND);
    }
}

//
//  create an specify filter instance
//
Uint32 CIMSubCommand::_createFilter
(
    const String& filterName,
    const CIMNamespaceName& filterNamespace,
    const String& filterQuery,
    const String& filterQueryLanguage,
    const Array<String>& filterSourceNamespaces,
    ostream& outPrintWriter,
    ostream& errPrintWriter
)
{
    CIMObjectPath filterPath;
    CIMNamespaceName filterNS = _DEFAULT_SUBSCRIPTION_NAMESPACE;
    Array<String> sourceNamespaces;
    sourceNamespaces = filterSourceNamespaces;
    String queryLang = "CIM:CQL";
    if (_isBatchNamespace )
    {
        filterNS = _batchNamespace;
    }
    else if (!filterNamespace.isNull())
    {
        filterNS = filterNamespace;
    }
    if (sourceNamespaces.size() == 0)
    {
        sourceNamespaces.append(filterNS.getString());
    }
    if (filterQueryLanguage != String::EMPTY)
    {
        queryLang = filterQueryLanguage;
    }

    CIMInstance filterInstance (PEGASUS_CLASSNAME_INDFILTER);
    filterInstance.addProperty (CIMProperty (CIMName ("Name"), filterName));
    filterInstance.addProperty (CIMProperty (CIMName ("Query"), filterQuery));
    filterInstance.addProperty (CIMProperty (CIMName ("QueryLanguage"),
        filterQueryLanguage));
    if (_filterNSFlag == false && sourceNamespaces.size() == 1)
    {
         filterInstance.addProperty (
             CIMProperty (CIMName ("SourceNamespace"),sourceNamespaces[0]));
    }
    else
    {
         filterInstance.addProperty (
             CIMProperty (CIMName ("SourceNamespaces"),sourceNamespaces));
    }
    _client->createInstance(filterNS, filterInstance);
    return (RC_SUCCESS);
}

//
//  remove an existing filter instance
//
Uint32 CIMSubCommand::_removeFilter
(
    const String& filterName,
    const CIMNamespaceName& filterNamespace,
    ostream& outPrintWriter,
    ostream& errPrintWriter
)
{
    CIMObjectPath filterPath;
    CIMNamespaceName filterNS = _DEFAULT_SUBSCRIPTION_NAMESPACE;
    if (_isBatchNamespace )
    {
        filterNS = _batchNamespace;
    }
    else if (!filterNamespace.isNull())
    {
        filterNS = filterNamespace;
    }

    if (_findFilter(filterName, filterNS, errPrintWriter, filterPath))
    {
        _client->deleteInstance(filterNS, filterPath);
        return (RC_SUCCESS);
    }
    else
    {
        outPrintWriter << localizeMessage(MSG_PATH,
            FILTER_NOT_FOUND_KEY,
            FILTER_NOT_FOUND_FAILURE) << endl;
        return (RC_OBJECT_NOT_FOUND);
    }
}

//
//  find a filter
//
Boolean CIMSubCommand::_findFilter(
    const String& filterName,
    const CIMNamespaceName& filterNamespace,
    ostream& errPrintWriter,
    CIMObjectPath& filterPath)
{
    Array<CIMObjectPath> filterPaths;
    Boolean status = false;

    try
    {
        filterPaths = _client->enumerateInstanceNames(
            filterNamespace,
            PEGASUS_CLASSNAME_INDFILTER);
    }
    catch (CIMException& e)
    {
        if (e.getCode() == CIM_ERR_INVALID_CLASS)
        {
            return false;
        }
        else
        {
            throw;
        }
    }

    Uint32 filterCount = filterPaths.size();
    if (filterCount > 0)
    {

        // find matching indication filter
        for (Uint32 i = 0; i < filterCount; i++)
        {
            CIMObjectPath fPath = filterPaths[i];
            Array<CIMKeyBinding> keys = fPath.getKeyBindings();
            for(Uint32 j=0; j < keys.size(); j++)
            {
                String filterNameValue;
                if(keys[j].getName().equal(PEGASUS_PROPERTYNAME_NAME))
                {
                    filterNameValue = keys[j].getValue();
                }
                if (filterNameValue == filterName)
                {
                    status = true;
                    filterPath = fPath;
                    break;
                }
          }
        }
    }
    return status;
}

////  create a specify CIMXML handler instance
//
Uint32 CIMSubCommand::_createCimXmlHandler(
    const String& handlerName,
    const CIMNamespaceName& handlerNamespace,
    const String& handlerCreationClass,
    const String& handlerDestination,
    ostream& outPrintWriter,
    ostream& errPrintWriter)
{
    CIMObjectPath handlerPath;
    CIMNamespaceName handlerNS = _DEFAULT_SUBSCRIPTION_NAMESPACE;
    CIMName creationClass = PEGASUS_CLASSNAME_LSTNRDST_CIMXML;
    if (_isBatchNamespace )
    {
        handlerNS = _batchNamespace;
    }
    else if (!handlerNamespace.isNull())
    {
        handlerNS = handlerNamespace;
    }

    if (handlerCreationClass != String::EMPTY)
    {
        creationClass = handlerCreationClass;
    }

    CIMInstance handlerInstance(creationClass);
    handlerInstance.addProperty(CIMProperty(CIMName("Name"), handlerName));
    handlerInstance.addProperty(CIMProperty(
        CIMName("Destination"),
        handlerDestination));

    _client->createInstance(
        handlerNS,
        handlerInstance);
        return (RC_SUCCESS);
}

////  create a specify Syslog handler instance
//
Uint32 CIMSubCommand::_createSystemLogHandler(
    const String& handlerName,
    const CIMNamespaceName& handlerNamespace,
    const String& handlerCreationClass,
    ostream& outPrintWriter,
    ostream& errPrintWriter)
{
    CIMObjectPath handlerPath;
    CIMNamespaceName handlerNS = _DEFAULT_SUBSCRIPTION_NAMESPACE;
    CIMName creationClass = PEGASUS_CLASSNAME_LSTNRDST_SYSTEM_LOG;
    if (_isBatchNamespace )
    {
        handlerNS = _batchNamespace;
    }
    else if (!handlerNamespace.isNull())
    {
        handlerNS = handlerNamespace;
    }

    if (handlerCreationClass != String::EMPTY)
    {
        creationClass = handlerCreationClass;
    }

    CIMInstance handlerInstance(creationClass);
    handlerInstance.addProperty(CIMProperty(CIMName("Name"), handlerName));

    _client->createInstance(
        handlerNS,
        handlerInstance);
        return (RC_SUCCESS);
}


////  create a specify Email handler instance
//
Uint32 CIMSubCommand::_createEmailHandler(
        const String& handlerName,
        const CIMNamespaceName& handlerNamespace,
        const String& handlerCreationClass,
        const Array<String>& mailTo,
        const Array<String>& mailCc,
        const String& mailSubject,
        ostream& outPrintWriter,
        ostream& errPrintWriter)
{
    CIMObjectPath handlerPath;
    CIMNamespaceName handlerNS = _DEFAULT_SUBSCRIPTION_NAMESPACE;
    CIMName creationClass = PEGASUS_CLASSNAME_LSTNRDST_EMAIL;
    if (_isBatchNamespace )
    {
        handlerNS = _batchNamespace;
    }
    else if (!handlerNamespace.isNull())
    {
        handlerNS = handlerNamespace;
    }

    if (handlerCreationClass != String::EMPTY)
    {
        creationClass = handlerCreationClass;
    }

    CIMInstance handlerInstance(creationClass);
    handlerInstance.addProperty(CIMProperty(CIMName("Name"), handlerName));

    handlerInstance.addProperty(CIMProperty(
        PEGASUS_PROPERTYNAME_LSTNRDST_MAILTO,
        mailTo));

    handlerInstance.addProperty(CIMProperty(
        PEGASUS_PROPERTYNAME_LSTNRDST_MAILCC,
        mailCc));

    handlerInstance.addProperty(CIMProperty(
        PEGASUS_PROPERTYNAME_LSTNRDST_MAILSUBJECT,
        mailSubject));

    _client->createInstance(
        handlerNS,
        handlerInstance);
        return (RC_SUCCESS);
}

////  create a specify Snmp Mapper handler instance
//
Uint32 CIMSubCommand::_createSnmpMapperHandler(
        const String& handlerName,
        const CIMNamespaceName& handlerNamespace,
        const String& handlerCreationClass,
        const String& targetHost,
        Uint32 snmpPort,
        Uint32 snmpVersion,
        const String& securityName,
        const String& engineId,
        ostream& outPrintWriter,
        ostream& errPrintWriter)
{
    CIMObjectPath handlerPath;
    CIMNamespaceName handlerNS = _DEFAULT_SUBSCRIPTION_NAMESPACE;
    CIMName creationClass = PEGASUS_CLASSNAME_INDHANDLER_SNMP;
    Uint16 targetHostFormat = 2; //Host Name
    if (_isBatchNamespace )
    {
        handlerNS = _batchNamespace;
    }
    else if (!handlerNamespace.isNull())
    {
        handlerNS = handlerNamespace;
    }

    if (handlerCreationClass != String::EMPTY)
    {
        creationClass = handlerCreationClass;
    }

    CIMInstance handlerInstance(creationClass);
    handlerInstance.addProperty(CIMProperty(CIMName("Name"), handlerName));

    {
        HostAddress tgtHost;
        tgtHost.setHostAddress(targetHost);
        if (tgtHost.getAddressType() == HostAddress::AT_IPV4)
        {
              targetHostFormat = 3; //Ipv4
        }
        else if(tgtHost.getAddressType() == HostAddress::AT_IPV6)
        {
            targetHostFormat = 4; //Ipv6
        }
    }
    handlerInstance.addProperty(CIMProperty(
        PEGASUS_PROPERTYNAME_LSTNRDST_TARGETHOST,
        targetHost));

    handlerInstance.addProperty(CIMProperty(
        "TargetHostFormat",
        (Uint16)targetHostFormat));

    handlerInstance.addProperty(CIMProperty(
        "PortNumber",
        snmpPort));

    handlerInstance.addProperty(CIMProperty(
        "SNMPVersion",
        (Uint16)snmpVersion));

    if (securityName != String::EMPTY)
    {
        handlerInstance.addProperty(CIMProperty(
            "SNMPSecurityName",
            securityName));
    }

    if (engineId != String::EMPTY)
    {
        handlerInstance.addProperty(CIMProperty(
            "SNMPEngineID",
            engineId));
    }
    _client->createInstance(
        handlerNS,
        handlerInstance);
        return (RC_SUCCESS);
}
////  remove an existing handler instance
//
Uint32 CIMSubCommand::_removeHandler(
    const String& handlerName,
    const CIMNamespaceName& handlerNamespace,
    const String& handlerCreationClass,
    ostream& outPrintWriter,
    ostream& errPrintWriter)
{
    CIMObjectPath handlerPath;
    CIMNamespaceName handlerNS = _DEFAULT_SUBSCRIPTION_NAMESPACE;
    if (_isBatchNamespace )
    {
        handlerNS = _batchNamespace;
    }
    else if (!handlerNamespace.isNull())
    {
        handlerNS = handlerNamespace;
    }

    if (_findHandler(handlerName, handlerNS, handlerCreationClass,
        errPrintWriter, handlerPath))
    {
        _client->deleteInstance(handlerNS, handlerPath);
        return (RC_SUCCESS);
    }
    else
    {
        outPrintWriter << localizeMessage(MSG_PATH,
            HANDLER_NOT_FOUND_KEY,
            HANDLER_NOT_FOUND_FAILURE) << endl;
        return RC_OBJECT_NOT_FOUND;
    }
}

//
//  find a matching handler
//
Boolean CIMSubCommand::_findHandler(
    const String& handlerName,
    const CIMNamespaceName& handlerNamespace,
    const String& handlerCreationClass,
    ostream& errPrintWriter,
    CIMObjectPath& handlerPath)
{
    Array<CIMObjectPath> handlerPaths;
    String handlerCC = PEGASUS_CLASSNAME_LSTNRDST_CIMXML.getString();
    Boolean status = false;
    if (handlerCreationClass != String::EMPTY)
    {
        handlerCC = handlerCreationClass;
    }
    try
    {
        handlerPaths = _client->enumerateInstanceNames(
            handlerNamespace,
            handlerCC);
    }
    catch (CIMException& e)
    {
        if (e.getCode() == CIM_ERR_INVALID_CLASS)
        {
            return false;
        }
        else
        {
            throw;
        }
    }

    Uint32 handlerCount = handlerPaths.size();
    if (handlerCount > 0)
    {
        // find matching indication handler
        for (Uint32 i = 0; i < handlerCount; i++)
        {
            Boolean nameFound = false;
            CIMObjectPath hPath = handlerPaths[i];
            Array<CIMKeyBinding> keys = hPath.getKeyBindings();
            for( Uint32 j=0; j < keys.size(); j++)
            {
                if (keys[j].getName().equal(PEGASUS_PROPERTYNAME_NAME))
                {
                    String handlerNameValue = keys[j].getValue();
                    if (handlerNameValue == handlerName )
                    {
                        nameFound = true;
                        break;
                    }
                }
            }
            if (nameFound)
            {
                status = true;
                handlerPath = hPath;
                break;
            }
        }
    }
    return status;
}

//
//  Modify a subscription state
//
void CIMSubCommand::_modifySubscriptionState(
    const CIMNamespaceName& subscriptionNS,
    const CIMObjectPath& targetPath,
    const Uint16 newState,
    ostream& outPrintWriter)
{
    Boolean alreadySet = false;
    CIMInstance targetInstance = _client->getInstance(subscriptionNS,
        targetPath);
    Uint32 pos = targetInstance.findProperty (
        PEGASUS_PROPERTYNAME_SUBSCRIPTION_STATE);
    if (pos != PEG_NOT_FOUND)
    {
        Uint16 subscriptionState;
        if (targetInstance.getProperty(pos).getValue().isNull())
        {
            subscriptionState = STATE_UNKNOWN;
        }
        else
        {
            targetInstance.getProperty(pos).getValue().get
                (subscriptionState);
            if (subscriptionState == newState)
            {
                if (newState == STATE_ENABLED )
                {
                    outPrintWriter << localizeMessage(MSG_PATH,
                        SUBSCRIPTION_ALREADY_ENABLED_KEY,
                        SUBSCRIPTION_ALREADY_ENABLED) << endl;
                }
                else
                {
                    outPrintWriter << localizeMessage(MSG_PATH,
                        SUBSCRIPTION_ALREADY_DISABLED_KEY,
                        SUBSCRIPTION_ALREADY_DISABLED) << endl;
                }
                alreadySet = true;
            }
        }
        if (!alreadySet)
        {
            targetInstance.getProperty(pos).setValue(newState);
            Array <CIMName> propertyNames;
            propertyNames.append(PEGASUS_PROPERTYNAME_SUBSCRIPTION_STATE);
            CIMPropertyList properties(propertyNames);
            targetInstance.setPath(targetPath);
            _client->modifyInstance(subscriptionNS, targetInstance, false,
                properties);
        }
    }
}

//
//  find a subscription
//
Boolean CIMSubCommand::_findSubscription(
    const CIMNamespaceName& subscriptionNamespace,
    const String& filterName,
    const CIMNamespaceName& filterNamespace,
    const String& handlerName,
    const CIMNamespaceName& handlerNamespace,
    const String& handlerCC,
    Array<CIMObjectPath>& subscriptionFound)
{
    Array<CIMObjectPath> subscriptionPaths;
    String handlerCreationClass;
    if (handlerCC != String::EMPTY)
    {
        handlerCreationClass = handlerCC;
    }
    try
    {
        subscriptionPaths = _client->enumerateInstanceNames(
            subscriptionNamespace, PEGASUS_CLASSNAME_INDSUBSCRIPTION);
    }
    catch (CIMException& e)
    {
        if (e.getCode() == CIM_ERR_INVALID_CLASS)
        {
            return false;
        }
        else
        {
            throw;
        }
    }

    Uint32 subscriptionCount = subscriptionPaths.size();
    if (subscriptionCount > 0)
    {
        String handlerNameString, filterNameString;
        CIMNamespaceName handlerNS, filterNS;

        // Search the indication subscriptions instances
        for (Uint32 i = 0; i < subscriptionCount; i++)
        {
            CIMObjectPath subPath = subscriptionPaths[i];
            CIMObjectPath filterRef;
            if (_filterMatches(subPath, subscriptionNamespace,
                filterName, filterNamespace, filterNS, filterRef))
            {
                CIMObjectPath handlerRef;
                if(_handlerMatches(subPath, subscriptionNamespace,
                    handlerName, handlerNamespace, handlerCreationClass,
                    handlerNS, handlerRef))
                {
                    subscriptionFound.append(subPath);
                }
            }
        }
    }
    return subscriptionFound.size() > 0;
}

//
// Find a subscription and modify it's state
//
Uint32 CIMSubCommand::_findAndModifyState(
    const Uint16 newState,
    const CIMNamespaceName& subscriptionNamespace,
    const String& filterName,
    const CIMNamespaceName& filterNamespace,
    const String& handlerName,
    const CIMNamespaceName& handlerNamespace,
    const String& handlerCreationClass,
    ostream& outPrintWriter)
{
    Array<CIMObjectPath> allSubscriptionFound;
    CIMNamespaceName filterNS;
    CIMNamespaceName handlerNS;
    CIMNamespaceName subscriptionNS = _DEFAULT_SUBSCRIPTION_NAMESPACE;

    if (_isBatchNamespace )
    {
        subscriptionNS = _batchNamespace;
    }

    else if (!subscriptionNamespace.isNull())
    {
        subscriptionNS = subscriptionNamespace;
    }

    if (_isBatchNamespace )
    {
        filterNS = _batchNamespace;
    }

    else if (!filterNamespace.isNull())
    {
        filterNS = filterNamespace;
    }
    else
    {
        filterNS = subscriptionNS;
    }
    if (_isBatchNamespace )
    {
        handlerNS = _batchNamespace;
    }

    else if (!handlerNamespace.isNull())
    {
        handlerNS = handlerNamespace;
    }
    else
    {
        handlerNS = subscriptionNS;
    }
    // Find subscriptions in the namespace specified by the user
    if (_findSubscription(subscriptionNS, filterName, filterNS,
        handlerName, handlerNS, handlerCreationClass, allSubscriptionFound))
    {
        for(Uint32 i = 0;i<allSubscriptionFound.size();i++)
        {
                _modifySubscriptionState(
                     subscriptionNS,
                     allSubscriptionFound[i],
                     newState,
                     outPrintWriter);
        }
        return(RC_SUCCESS);
    }
    else
    {
        outPrintWriter << localizeMessage(MSG_PATH,
            SUBSCRIPTION_NOT_FOUND_KEY,
            SUBSCRIPTION_NOT_FOUND_FAILURE) << endl;
        return(RC_OBJECT_NOT_FOUND);
    }
}

//
// Get the name from a CIMObjectPath
//
String CIMSubCommand::_getNameInKey(const CIMObjectPath& r)
{
    String nameValue;
    Array<CIMKeyBinding> keys = r.getKeyBindings();
    for (Uint32 j=0; j < keys.size(); j++)
    {
        if (keys[j].getName().equal(PEGASUS_PROPERTYNAME_NAME))
        {
            nameValue = keys[j].getValue();
        }
    }
    return (nameValue);
}

//
// Get all namespaces
//
//
void CIMSubCommand::_getAllNamespaces(
    Array<CIMNamespaceName>& namespaceNames)
{
    Array<CIMObjectPath> instanceNames = _client->enumerateInstanceNames(
        PEGASUS_VIRTUAL_TOPLEVEL_NAMESPACE,
        PEGASUS_CLASSNAME___NAMESPACE);

    // for all new elements in the output array
    for (Uint32 i = 0; i < instanceNames.size(); i++)
    {
        Array<CIMKeyBinding> keys = instanceNames[i].getKeyBindings();
        for (Uint32 j=0; j < keys.size(); j++)
        {
            if (keys[j].getName().equal(PEGASUS_PROPERTYNAME_NAME))
            {
                namespaceNames.append(keys[j].getValue());
            }
        }
    }
}

//
//  get a list of all Handlers in the specified namespace(s)
//
void CIMSubCommand::_listHandlers(
    const String& handlerName,
    const Array<CIMNamespaceName>& namespaceNames,
    const String& handlerCreationClass,
    const Boolean verbose,
    ostream& outPrintWriter,
    ostream& errPrintWriter)
{
    Array <Uint32> maxColumnWidth;
    Array <ListColumnEntry> listOutputTable;
    Array<String> handlersFound;
    Array<String> destinationsFound;
    Array<CIMInstance> instancesFound;
    Array<Uint32> handlerTypesFound;
    const String handlerTitle = "HANDLER";
    const String destinationTitle = "DESTINATION";
    if (!verbose)
    {
        handlersFound.append(handlerTitle);
        maxColumnWidth.append(handlerTitle.size());
        destinationsFound.append(destinationTitle);
        maxColumnWidth.append(destinationTitle.size());
    }
    listOutputTable.append(handlersFound);
    listOutputTable.append(destinationsFound);
    //
    //  Find handlers in namespaces
    //
    if (_isBatchNamespace )
    {
        _getHandlerList(
                handlerName,
                _batchNamespace,
                handlerCreationClass,
                verbose,
                instancesFound,
                handlerTypesFound,
                maxColumnWidth,
                listOutputTable,
                outPrintWriter,
                errPrintWriter);
    }
    else
    {
       for (Uint32 i = 0 ; i < namespaceNames.size() ; i++)
       {
           _getHandlerList(
                handlerName,
                namespaceNames[i],
                handlerCreationClass,
                verbose,
                instancesFound,
                handlerTypesFound,
                maxColumnWidth,
                listOutputTable,
                outPrintWriter,
                errPrintWriter);
       }
    }
    if (verbose)
    {
        if (listOutputTable[_HANDLER_LIST_NAME_COLUMN].size() > 0 )
        {
            _printHandlersVerbose(instancesFound, handlerTypesFound,
                 listOutputTable, outPrintWriter);
        }
    }
    else
    {
        if (listOutputTable[_HANDLER_LIST_NAME_COLUMN].size() > 1 )
        {
            _printColumns(maxColumnWidth, listOutputTable, outPrintWriter);
        }
    }
}

//
//  get a list of all handlers in a specified namespace
//
void CIMSubCommand::_getHandlerList(
    const String& handlerName,
    const CIMNamespaceName& handlerNamespace,
    const String& handlerCreationClass,
    const Boolean verbose,
    Array<CIMInstance>& instancesFound,
    Array<Uint32>& handlerTypesFound,
    Array <Uint32>& maxColumnWidth,
    Array <ListColumnEntry>& listOutputTable,
    ostream& outPrintWriter,
    ostream& errPrintWriter)
{
    Array<CIMObjectPath> handlerPaths;
    try
    {
        handlerPaths = _client->enumerateInstanceNames(
            handlerNamespace,
            PEGASUS_CLASSNAME_LSTNRDST);
    }
    catch(CIMException& e)
    {
        if (e.getCode() == CIM_ERR_INVALID_CLASS)
        {
            return;
        }
        else
        {
            throw;
        }
    }

    Uint32 handlerCount = handlerPaths.size();
    if (handlerCount > 0)
    {
        String handlerNameValue;
        String destination;
        String creationClassValue;

        // List all the indication handlers
        for (Uint32 i = 0; i < handlerCount; i++)
        {
            Boolean isMatch = true;
            CIMObjectPath handlerPath;
            CIMObjectPath hPath = handlerPaths[i];
            Array<CIMKeyBinding> keys = hPath.getKeyBindings();
            for(Uint32 j=0; j < keys.size(); j++)
            {
                if(keys[j].getName().equal(PEGASUS_PROPERTYNAME_NAME))
                {
                    handlerNameValue = keys[j].getValue();
                }
                if(keys[j].getName().equal(
                            PEGASUS_PROPERTYNAME_CREATIONCLASSNAME))
                {
                    creationClassValue = keys[j].getValue();
                }
            }
            if (handlerName != String::EMPTY)
            {
                if (handlerNameValue == handlerName)
                {
                    if (handlerCreationClass != String::EMPTY)
                    {
                        if (handlerCreationClass !=
                            creationClassValue)
                        {
                            isMatch = false;
                        }
                    }
                }
                else
                {
                    isMatch = false;
                }
            }
            if (isMatch)
            {
                handlerPath = hPath;
                CIMInstance handlerInstance = _client->getInstance(
                    handlerNamespace, handlerPath);
                Uint32 handlerType = _HANDLER_CIMXML;
                _getHandlerDestination(handlerInstance, creationClassValue,
                    handlerType, destination);
                String handlerString = handlerNamespace.getString();
                handlerString.append(DELIMITER_NAMESPACE);
                handlerString.append(creationClassValue);
                handlerString.append(DELIMITER_HANDLER_CLASS);
                handlerString.append(handlerNameValue);
                listOutputTable[_HANDLER_LIST_NAME_COLUMN].append(
                        handlerString);
                listOutputTable[_HANDLER_LIST_DESTINATION_COLUMN].append(
                        destination);
                handlerTypesFound.append(handlerType);
                if (verbose)
                {
                    instancesFound.append(handlerInstance);
                }
                else
                {
                    if (handlerString.size() >
                        maxColumnWidth[_HANDLER_LIST_NAME_COLUMN])
                    {
                        maxColumnWidth[_HANDLER_LIST_NAME_COLUMN] =
                            handlerString.size();
                    }
                    if (destination.size() >
                        maxColumnWidth[_HANDLER_LIST_DESTINATION_COLUMN])
                    {
                        maxColumnWidth[_HANDLER_LIST_DESTINATION_COLUMN] =
                            destination.size();
                    }
                }
            }
        }
    }
}

//
//  print a verbose list of Handlers
//
void CIMSubCommand::_printHandlersVerbose(
    const Array<CIMInstance>& instancesFound,
    const Array<Uint32>& handlerTypesFound,
    const Array <ListColumnEntry>& listOutputTable,
    ostream& outPrintWriter)
{
    Uint32 maxEntries = listOutputTable[_HANDLER_LIST_NAME_COLUMN].size();
    Array <Uint32> indexes;
    for (Uint32 i = 0; i < maxEntries; i++)
    {
       indexes.append (i);
    }
    _bubbleIndexSort (listOutputTable[_HANDLER_LIST_NAME_COLUMN], 0, indexes);
    for (Uint32 i = 0; i < maxEntries; i++)
    {
        Uint32 pos;
        CIMInstance handlerInstance = instancesFound[indexes[i]];
        outPrintWriter << "Handler:           " <<
           (listOutputTable[_HANDLER_LIST_NAME_COLUMN])[indexes[i]] << endl;
        switch (handlerTypesFound[indexes[i]])
        {
            case _HANDLER_SNMP:
            {
                String targetHost;
                pos = handlerInstance.findProperty(
                    PEGASUS_PROPERTYNAME_LSTNRDST_TARGETHOST);
                if (pos != PEG_NOT_FOUND)
                {
                    handlerInstance.getProperty(pos).getValue().get
                        (targetHost);
                }
                outPrintWriter << "TargetHost:        " << targetHost
                    << endl;
                outPrintWriter << "SNMPVersion:       " <<
                    _getSnmpVersion(handlerInstance) << endl;
                break;
            }

            case _HANDLER_EMAIL:
            {
                String mailCc;
                String mailTo;
                String mailSubject;
                _getEmailInfo(handlerInstance, mailCc,
                    mailTo, mailSubject );
                outPrintWriter << "MailTo:            " <<
                    mailTo << endl;
                if (mailCc.size() > 0 )
                {
                    outPrintWriter << "MailCc:            " <<
                        mailCc << endl;
                }
                if (mailSubject.size() > 0 )
                {
                    outPrintWriter << "MailSubject:       " <<
                        mailSubject << endl;
                }
                break;
            }

            case _HANDLER_SYSLOG:
            {
                break;
            }

            case _HANDLER_CIMXML:
            {
                outPrintWriter << "Destination:       " <<
                    (listOutputTable[_HANDLER_LIST_DESTINATION_COLUMN])
                        [indexes[i]]
                    << endl;
            }
        }
        outPrintWriter << "PersistenceType:   " <<
            _getPersistenceType(handlerInstance) << endl;
        outPrintWriter << "-----------------------------------------"
            << endl;
    }
}

//
//  get a list of all filters in the specified namespace(s)
//
void CIMSubCommand::_listFilters(
    const String& filterName,
    const Boolean verbose,
    const Array<CIMNamespaceName>& namespaceNames,
    ostream& outPrintWriter,
    ostream& errPrintWriter)
{
    Array <Uint32> maxColumnWidth;
    Array <ListColumnEntry> listOutputTable;
    Array<String> filtersFound;
    Array<String> querysFound;
    Array<String> queryLangsFound;
    Array<String> filterSourceNamespaces;
    const String filterTitle = "FILTER";
    const String queryTitle = "QUERY";
    if (!verbose)
    {
        filtersFound.append(filterTitle);
        maxColumnWidth.append(filterTitle.size());
        querysFound.append(queryTitle);
        maxColumnWidth.append(queryTitle.size());
    }
    listOutputTable.append(filtersFound);
    listOutputTable.append(querysFound);

    //  Find filters in namespaces
    if (_isBatchNamespace )
    {
         _getFilterList(
             filterName,
             _batchNamespace,
             verbose,
             maxColumnWidth,
             listOutputTable,
             queryLangsFound,
             filterSourceNamespaces,
             outPrintWriter,
             errPrintWriter);
    }
    else
    {
       for (Uint32 i = 0 ; i < namespaceNames.size(); i++)
       {
        _getFilterList(
             filterName,
             namespaceNames[i],
             verbose,
             maxColumnWidth,
             listOutputTable,
             queryLangsFound,
             filterSourceNamespaces,
             outPrintWriter,
             errPrintWriter);
       }
    }
    if (verbose)
    {
        if (listOutputTable[_FILTER_LIST_NAME_COLUMN].size() > 0)
        {
           _printFiltersVerbose(
                listOutputTable,
                queryLangsFound,
                filterSourceNamespaces,
                outPrintWriter);
        }
    }
    else
    {
        if (listOutputTable[_FILTER_LIST_NAME_COLUMN].size() > 1)
        {
            _printColumns(maxColumnWidth, listOutputTable, outPrintWriter);
        }
    }
}

//
//  get a list of all filters in the specified namespace(s)
//
void CIMSubCommand::_printFiltersVerbose(
    const Array <ListColumnEntry>& listOutputTable,
    const Array <String>& queryLangs,
    const Array<String>& filterSourceNamespaces,
    ostream& outPrintWriter)
{
    Uint32 maxEntries = listOutputTable[_FILTER_LIST_NAME_COLUMN].size();
    Array <Uint32> indexes;
    for (Uint32 i = 0; i < maxEntries; i++)
    {
       indexes.append(i);
    }
    _bubbleIndexSort (listOutputTable[_FILTER_LIST_NAME_COLUMN], 0, indexes);
    for (Uint32 i = 0; i < maxEntries; i++)
    {
        outPrintWriter << "Filter:           " <<
            (listOutputTable[_FILTER_LIST_NAME_COLUMN])[indexes[i]] << endl;
        outPrintWriter << "Query:            " <<
            (listOutputTable[_FILTER_LIST_QUERY_COLUMN])[indexes[i]] << endl;
        outPrintWriter << "Query Language:   " <<
            queryLangs[indexes[i]] << endl;
        outPrintWriter << "Source Namespace: ";
        for (Uint32 j = 0; j < filterSourceNamespaces.size(); j++)
        {
            if ( j < filterSourceNamespaces.size() - 1)
            {
                outPrintWriter<< filterSourceNamespaces[j]<<",";
            }
            else
            {
                outPrintWriter<< filterSourceNamespaces[j]<<endl;
            }
        }
        outPrintWriter <<
            "-----------------------------------------" << endl;
   }
}

//
//  get a list of all filters in the specified namespace
//
void CIMSubCommand::_getFilterList(
    const String& filterName,
    const CIMNamespaceName& filterNamespace,
    const Boolean verbose,
    Array <Uint32>& maxColumnWidth,
    Array <ListColumnEntry>& listOutputTable,
    Array <String>& queryLangsFound,
    Array<String>& filterSourceNamespaces,
    ostream& outPrintWriter,
    ostream& errPrintWriter)
{
    Array<CIMObjectPath> filterPaths;
    try
    {
        filterPaths = _client->enumerateInstanceNames(
            filterNamespace,
            PEGASUS_CLASSNAME_INDFILTER);
    }
    catch(CIMException& e)
    {
        if (e.getCode() == CIM_ERR_INVALID_CLASS)
        {
            return;
        }
        else
        {
            throw;
        }
    }

    Uint32 filterCount = filterPaths.size();
    if (filterCount > 0)
    {
        CIMObjectPath filterPath;
        String filterNameValue;

        // List all the indication filters
        for (Uint32 i = 0; i < filterCount; i++)
        {
            Boolean isMatch = true;
            CIMObjectPath fPath = filterPaths[i];
            Array<CIMKeyBinding> keys = fPath.getKeyBindings();
            for(Uint32 j=0; j < keys.size(); j++)
            {
                if(keys[j].getName().equal(PEGASUS_PROPERTYNAME_NAME))
                {
                    filterNameValue = keys[j].getValue();
                    filterPath = fPath;
                    if (filterName != String::EMPTY)
                    {
                        if (filterNameValue == filterName)
                        {
                            break;
                        }
                        else
                        {
                            isMatch = false;
                            break;
                        }
                    }
                    else
                    {
                        break;
                    }
                }
            }
            if (isMatch)
            {
                String queryString,queryLanguageString;
                String filterString = filterNamespace.getString();
                String sourceNamespaceString = filterNamespace.getString();
                filterString.append(DELIMITER_NAMESPACE);
                filterString.append(filterNameValue);
                _getFilterInfo(
                     filterNamespace,
                     filterPath,
                     queryString,
                     queryLanguageString,
                     filterSourceNamespaces);
                listOutputTable[_FILTER_LIST_NAME_COLUMN].append(filterString);
                listOutputTable[_FILTER_LIST_QUERY_COLUMN].append(queryString);
                if (verbose)
                {
                    queryLangsFound.append(queryLanguageString);
                }
                else
                {
                    if (filterString.size () >
                        maxColumnWidth[_FILTER_LIST_NAME_COLUMN])
                    {
                        maxColumnWidth[_FILTER_LIST_NAME_COLUMN] =
                            filterString.size();
                    }
                    if (queryString.size() >
                        maxColumnWidth[_FILTER_LIST_QUERY_COLUMN])
                    {
                        maxColumnWidth[_FILTER_LIST_QUERY_COLUMN] =
                            queryString.size();
                    }
                }
           }
        }
    }
}

//
//  get the query string for a filter
//
void CIMSubCommand::_getFilterInfo(
    const CIMNamespaceName& filterNamespace,
    const CIMObjectPath& filterPath,
    String& queryString,
    String& queryLangString,
    Array<String>& filterSourceNamespaces)
{
    String query;
    String filterSourceNamespace;
    queryString = "\"";
    CIMInstance filterInstance = _client->getInstance(
        filterNamespace, filterPath);
    Uint32 pos = filterInstance.findProperty(
        PEGASUS_PROPERTYNAME_QUERY);
    if (pos != PEG_NOT_FOUND)
    {
        filterInstance.getProperty(pos).getValue().get(query);
        queryString.append(query);
    }
    queryString.append("\"");
    pos = filterInstance.findProperty(
        PEGASUS_PROPERTYNAME_QUERYLANGUAGE);
    if (pos != PEG_NOT_FOUND)
    {
        filterInstance.getProperty(pos).getValue().get(queryLangString);
    }
    pos = filterInstance.findProperty(
        CIMNameCast("SourceNamespace"));
    if (pos != PEG_NOT_FOUND)
    {
        filterInstance.getProperty(pos).getValue().get(filterSourceNamespace);
        filterSourceNamespaces.append(filterSourceNamespace);
    }
    pos = filterInstance.findProperty(
              CIMNameCast("SourceNamespaces"));
    if (pos != PEG_NOT_FOUND)
    {
        filterSourceNamespaces.clear();
        filterInstance.getProperty(pos).getValue().get(
              filterSourceNamespaces);
    }
    if (filterSourceNamespaces.size() == 0)
    {
        filterSourceNamespaces.append(filterSourceNamespace);
    }
}

//
//  list  all subscriptions is the specified namespace(s)
//
void CIMSubCommand::_listSubscriptions(
    const Array<CIMNamespaceName>& namespaceNames,
    const String& filterName,
    const CIMNamespaceName& filterNamespace,
    const String& handlerName,
    const CIMNamespaceName& handlerNamespace,
    const String& handlerCreationClass,
    const Boolean verbose,
    ostream& outPrintWriter,
    ostream& errPrintWriter)
{
    Array <Uint32> maxColumnWidth;
    Array <ListColumnEntry> listOutputTable;
    Array<String> namespacesFound;
    Array<String> filtersFound;
    Array<String> handlersFound;
    Array<String> statesFound;
    Array<CIMInstance> handlerInstancesFound;
    Array<Uint32> handlerTypesFound;
    Array<String> querysFound;
    const String namespaceTitle = "NAMESPACE";
    const String filterTitle = "FILTER";
    const String handlerTitle = "HANDLER";
    const String stateTitle = "STATE";
    if (!verbose)
    {
        namespacesFound.append(namespaceTitle);
        maxColumnWidth.append(namespaceTitle.size());
        filtersFound.append(filterTitle);
        maxColumnWidth.append(filterTitle.size());
        handlersFound.append(handlerTitle);
        maxColumnWidth.append(handlerTitle.size());
        statesFound.append(stateTitle);
        maxColumnWidth.append(stateTitle.size());
    }

    listOutputTable.append(namespacesFound);
    listOutputTable.append(filtersFound);
    listOutputTable.append(handlersFound);
    listOutputTable.append(statesFound);
    if (_isBatchNamespace)
    {
        _getSubscriptionList(
             _batchNamespace,
             filterName,
             _batchNamespace,
             handlerName,
             _batchNamespace,
             handlerCreationClass,
             verbose,
             handlerInstancesFound,
             handlerTypesFound,
             querysFound,
             maxColumnWidth,
             listOutputTable);
    }
    else
    {
        for (Uint32 i = 0 ; i < namespaceNames.size() ; i++)
        {
            _getSubscriptionList(
                namespaceNames[i],
                filterName,
                filterNamespace,
                handlerName,
                handlerNamespace,
                handlerCreationClass,
                verbose,
                handlerInstancesFound,
                handlerTypesFound,
                querysFound,
                maxColumnWidth,
                listOutputTable);
        }
    }

    if (verbose)
    {
        if (listOutputTable[_SUBSCRIPTION_LIST_NS_COLUMN].size() > 0)
        {
            _printSubscriptionsVerbose(handlerInstancesFound,
               handlerTypesFound, querysFound, listOutputTable,
               outPrintWriter);
        }
    }
    else
    {
        if (listOutputTable[_SUBSCRIPTION_LIST_NS_COLUMN].size() > 1)
        {
            _printColumns(maxColumnWidth, listOutputTable, outPrintWriter);
        }
    }
}

//
//  get a list of subscriptions in the specifed namespace
//
void CIMSubCommand::_getSubscriptionList(
    const CIMNamespaceName& subscriptionNSIn,
    const String& filterName,
    const CIMNamespaceName& filterNSIn,
    const String& handlerName,
    const CIMNamespaceName& handlerNSIn,
    const String& handlerCCIn,
    const Boolean verbose,
    Array<CIMInstance>& handlerInstancesFound,
    Array<Uint32>& handlerTypesFound,
    Array<String>& querysFound,
    Array <Uint32>& maxColumnWidth,
    Array <ListColumnEntry>& listOutputTable
)
{
    Array<CIMObjectPath> subscriptionPaths;
    String query;
    String destination;
    CIMNamespaceName filterNamespace;
    CIMNamespaceName handlerNamespace;
    CIMNamespaceName subscriptionNamespace = _DEFAULT_SUBSCRIPTION_NAMESPACE;
    String handlerCreationClass;
    if (!subscriptionNSIn.isNull())
    {
        subscriptionNamespace = subscriptionNSIn;
    }
    if (!filterNSIn.isNull())
    {
        filterNamespace = filterNSIn;
    }

    if (!handlerNSIn.isNull())
    {
        handlerNamespace = handlerNSIn;
    }

    if (handlerCCIn != String::EMPTY)
    {
        handlerCreationClass = handlerCCIn;
    }

    try
    {
        subscriptionPaths = _client->enumerateInstanceNames(
            subscriptionNamespace,
            PEGASUS_CLASSNAME_INDSUBSCRIPTION);
    }
    catch(CIMException& e)
    {
        if (e.getCode() == CIM_ERR_INVALID_CLASS)
        {
            return;
        }
        else
        {
            throw;
        }
    }
    Uint32 subscriptionCount = subscriptionPaths.size();
    if (subscriptionCount > 0)
    {
        String handlerNameString, filterNameString;
        CIMObjectPath handlerPath, filterPath;
        CIMNamespaceName handlerNS, filterNS;
        // List all the indication subscriptions
        for (Uint32 i = 0; i < subscriptionCount; i++)
        {
            CIMObjectPath subPath = subscriptionPaths[i];
            CIMObjectPath filterRef, handlerRef;
            Boolean filterMatch = true;
            Boolean handlerMatch = true;
            String creationClassName;

            //
            //  Get the subscription Filter
            //
            filterMatch = _filterMatches(subPath, subscriptionNamespace,
                filterName, filterNamespace, filterNS, filterRef);
            if (filterMatch)
            {
                filterNameString = _getNameInKey(filterRef);
                handlerMatch = _handlerMatches(subPath, subscriptionNamespace,
                    handlerName, handlerNamespace, handlerCreationClass,
                    handlerNS, handlerRef);
                if (handlerMatch)
                {
                    handlerNameString = _getNameInKey(handlerRef);
                }
            }
            if ((filterMatch) && (handlerMatch))
            {
                // Get the destination for this handler.
                CIMInstance handlerInstance = _client->getInstance(
                    handlerNS, handlerRef);
                Uint32 handlerType;
                _getHandlerDestination(handlerInstance, creationClassName,
                    handlerType, destination);
                String handlerString = handlerNS.getString();
                handlerString.append(DELIMITER_NAMESPACE);
                handlerString.append(creationClassName);
                handlerString.append(DELIMITER_HANDLER_CLASS);
                handlerString.append(handlerNameString);
                String statusValue = _getSubscriptionState(
                    subscriptionNamespace, subPath);

                // Save for columnar listing
                listOutputTable[_SUBSCRIPTION_LIST_NS_COLUMN].append
                    (subscriptionNamespace.getString());
                String filterString = filterNS.getString();
                filterString.append(DELIMITER_NAMESPACE);
                filterString.append(filterNameString);
                listOutputTable[_SUBSCRIPTION_LIST_FILTER_COLUMN].append
                    (filterString);
                listOutputTable[_SUBSCRIPTION_LIST_HANDLER_COLUMN].append
                    (handlerString);
                listOutputTable[_SUBSCRIPTION_LIST_STATE_COLUMN].append
                    (statusValue);
                if (verbose)
                {
                    String queryString, queryLangString, sourceNamespace;
                    Array <String> sourceNamespaces;
                    handlerInstancesFound.append (handlerInstance);
                    handlerTypesFound.append (handlerType);
                    _getFilterInfo(filterNS, filterRef, queryString,
                       queryLangString,sourceNamespaces);
                    querysFound.append(queryString);
                }
                else
                {
                    if (subscriptionNamespace.getString().size() >
                        maxColumnWidth[_SUBSCRIPTION_LIST_NS_COLUMN])
                    {
                        maxColumnWidth[_SUBSCRIPTION_LIST_NS_COLUMN] =
                            subscriptionNamespace.getString().size();
                    }
                    if (filterString.size() >
                        maxColumnWidth[_SUBSCRIPTION_LIST_FILTER_COLUMN])
                    {
                        maxColumnWidth[_SUBSCRIPTION_LIST_FILTER_COLUMN] =
                            filterString.size();
                    }
                    if (handlerString.size() >
                        maxColumnWidth[_SUBSCRIPTION_LIST_HANDLER_COLUMN])
                    {
                        maxColumnWidth[_SUBSCRIPTION_LIST_HANDLER_COLUMN] =
                            handlerString.size();
                    }
                    if (statusValue.size() >
                        maxColumnWidth[_SUBSCRIPTION_LIST_STATE_COLUMN])
                    {
                        maxColumnWidth[_SUBSCRIPTION_LIST_STATE_COLUMN] =
                            statusValue.size();
                    }
                }
            }
        }
    }
}

//
//  get the handler destination and type
//
void CIMSubCommand::_getHandlerDestination(
    const CIMInstance& handlerInstance,
    String& creationClassName,
    Uint32&  handlerTypeFound,
    String& destination)
{
    Uint32 pos;
    pos = handlerInstance.findProperty(
        PEGASUS_PROPERTYNAME_CREATIONCLASSNAME);
    if (pos != PEG_NOT_FOUND)
    {
        handlerInstance.getProperty(pos).getValue().get
            (creationClassName);
    }
    handlerTypeFound = _HANDLER_CIMXML;
    destination = String::EMPTY;
    if (handlerInstance.getClassName() ==
        PEGASUS_CLASSNAME_INDHANDLER_SNMP)
    {
        handlerTypeFound = _HANDLER_SNMP;
        String targetHost;
        pos = handlerInstance.findProperty(
            PEGASUS_PROPERTYNAME_LSTNRDST_TARGETHOST);
        if (pos != PEG_NOT_FOUND)
        {
            handlerInstance.getProperty(pos).getValue().get
                (targetHost);
        }
        destination = targetHost;
    }
    else
    {
        if (handlerInstance.getClassName() ==
            PEGASUS_CLASSNAME_LSTNRDST_EMAIL)
        {
            handlerTypeFound = _HANDLER_EMAIL;
            Array <String> mailTo;
            pos = handlerInstance.findProperty
                (PEGASUS_PROPERTYNAME_LSTNRDST_MAILTO);
            if (pos != PEG_NOT_FOUND)
            {
                handlerInstance.getProperty(pos).getValue().get
                    (mailTo);
            }
            for (Uint32 eIndex=0; eIndex < mailTo.size();
                eIndex++)
            {
                destination.append (mailTo[eIndex]);
                destination.append (" ");
            }
         }
        else
        {
            if (creationClassName ==
                PEGASUS_CLASSNAME_LSTNRDST_SYSTEM_LOG)
            {
                handlerTypeFound = _HANDLER_SYSLOG;
            }
            else
            {
                pos = handlerInstance.findProperty(
                    PEGASUS_PROPERTYNAME_LSTNRDST_DESTINATION);
                if (pos != PEG_NOT_FOUND)
                {
                    handlerInstance.getProperty (pos).getValue ().get
                        (destination);
                 }
            }
        }
    }
}

//
//  print a verbose list of subscriptions
//
void CIMSubCommand::_printSubscriptionsVerbose(
    const Array<CIMInstance>& handlerInstancesFound,
    const Array<Uint32>& handlerTypesFound,
    const Array<String>& querysFound,
    const Array<ListColumnEntry>& listOutputTable,
    ostream& outPrintWriter)
{
    Uint32 maxEntries = listOutputTable[_SUBSCRIPTION_LIST_NS_COLUMN].size();
    Array <Uint32> indexes;
    for (Uint32 i = 0; i < maxEntries; i++)
    {
       indexes.append (i);
    }
    _bubbleIndexSort(listOutputTable[_SUBSCRIPTION_LIST_HANDLER_COLUMN], 0,
            indexes);
    _bubbleIndexSort(listOutputTable[_SUBSCRIPTION_LIST_FILTER_COLUMN], 0,
            indexes);
    _bubbleIndexSort(listOutputTable[_SUBSCRIPTION_LIST_NS_COLUMN], 0, indexes);
    for (Uint32 i = 0; i < maxEntries; i++)
    {
        outPrintWriter << "Namespace:         " <<
            (listOutputTable[_SUBSCRIPTION_LIST_NS_COLUMN])[indexes[i]] << endl;
        outPrintWriter << "Filter:            " <<
            (listOutputTable[_SUBSCRIPTION_LIST_FILTER_COLUMN])[indexes[i]]
            << endl;
        outPrintWriter << "Handler:           " <<
            (listOutputTable[_SUBSCRIPTION_LIST_HANDLER_COLUMN])[indexes[i]]
            << endl;
        outPrintWriter << "Query:             " << querysFound[indexes[i]]
                << endl;
        CIMInstance handlerInstance = handlerInstancesFound[indexes[i]];
        switch (handlerTypesFound[indexes[i]])
        {
            case _HANDLER_SNMP:
            {
                String targetHost;
                Uint32 pos = handlerInstance.findProperty(
                    PEGASUS_PROPERTYNAME_LSTNRDST_TARGETHOST);
                if (pos != PEG_NOT_FOUND)
                {
                    handlerInstance.getProperty(pos).getValue().get
                        (targetHost);
                }
                outPrintWriter << "TargetHost:        " << targetHost
                    << endl;
                outPrintWriter << "SNMPVersion:       " <<
                    _getSnmpVersion(handlerInstance) << endl;
                break;
            }
            case _HANDLER_EMAIL:
            {
                String mailCc;
                String mailTo;
                String mailSubject;
                _getEmailInfo(handlerInstance, mailCc,
                    mailTo, mailSubject );
                outPrintWriter << "MailTo:            " <<
                    mailTo << endl;
                if (mailCc.size() > 0 )
                {
                    outPrintWriter << "MailCc:            " <<
                        mailCc << endl;
                }
                if (mailSubject.size() > 0 )
                {
                    outPrintWriter << "MailSubject:       " <<
                        mailSubject << endl;
                }
                break;
            }
            case _HANDLER_SYSLOG:
            {
                break;
            }
            case _HANDLER_CIMXML:
            {
                String destination;
                Uint32 pos = handlerInstance.findProperty(
                    PEGASUS_PROPERTYNAME_LSTNRDST_DESTINATION);
                if (pos != PEG_NOT_FOUND)
                {
                    handlerInstance.getProperty(pos).getValue().get
                        (destination);
                }
                outPrintWriter << "Destination:       " << destination << endl;
            }
        }
        outPrintWriter << "SubscriptionState: " <<
            (listOutputTable[_SUBSCRIPTION_LIST_STATE_COLUMN])[indexes[i]] <<
            endl;
        outPrintWriter <<
            "-----------------------------------------" << endl;
    }
}

//
//  check a subscription for a filter match
//
Boolean CIMSubCommand::_filterMatches(
    const CIMObjectPath& subPath,
    const CIMNamespaceName& subscriptionNS,
    const String& filterName,
    const CIMNamespaceName& filterNamespace,
    CIMNamespaceName& filterNS,
    CIMObjectPath& filterRef)
{
    Boolean filterMatch = false;
    String filterNameString;

    //
    //  Get the subscription Filter
    //
    Array<CIMKeyBinding> keys = subPath.getKeyBindings();
    for( Uint32 j=0; j < keys.size(); j++)
    {
        if (keys[j].getName().equal(PEGASUS_PROPERTYNAME_FILTER))
        {
            filterRef = keys[j].getValue();
        }
    }

    filterNameString = _getNameInKey(filterRef);
    CIMNamespaceName instanceNS = filterRef.getNameSpace();
    if (filterName != String::EMPTY)
    {
        if (filterNameString == filterName)
        {
            if (!filterNamespace.isNull())
            {
                //
                //  If the Filter reference property value includes
                //  namespace, check if it is the namespace of the Filter.
                //  If the Filter reference property value does not
                //  include namespace, check if the current subscription
                //  namespace is the namespace of the Filter.
                //
                if(instanceNS.isNull()
                    || filterNamespace == instanceNS)
                {
                    filterNS = filterNamespace;
                    filterMatch = true;
                }
            }
            else
            {
                // No namespace was specified and the filter name matches
                filterMatch = true;
                if (instanceNS.isNull())
                {
                    filterNS = subscriptionNS;
                }
                else
                {
                    filterNS = instanceNS;
                }
            }
        }
        else
        {
            // The filter name does not match
            filterMatch = false;
        }
    }
    else
    {
        filterMatch = true;
        // No filter name was specified.
        // Use the filter namespace if specified in the reference.
        //
        if (instanceNS.isNull())
        {
            filterNS = subscriptionNS;
        }
        else
        {
            filterNS = instanceNS;
        }
    }
    return filterMatch;
}

//
//  check a subscription for a handler match
//
Boolean CIMSubCommand::_handlerMatches(
    const CIMObjectPath& subPath,
    const CIMNamespaceName& subscriptionNS,
    const String& handlerName,
    const CIMNamespaceName& handlerNamespace,
    const String& handlerCreationClass,
    CIMNamespaceName& handlerNS,
    CIMObjectPath& handlerRef)
{
    Boolean handlerMatch = false;
    String handlerNameString;
    String creationClassName;

    //
    //  Get the subscription Handler
    //
    Array<CIMKeyBinding> keys = subPath.getKeyBindings();
    for( Uint32 j=0; j < keys.size(); j++)
    {
        if (keys[j].getName().equal(PEGASUS_PROPERTYNAME_HANDLER))
        {
            handlerRef = keys[j].getValue();
        }
    }
    handlerNameString = _getNameInKey(handlerRef);
    if (handlerName != String::EMPTY)
    {
        CIMNamespaceName instanceNS = handlerRef.getNameSpace();
        if (handlerNameString == handlerName)
        {
            if (!handlerNamespace.isNull())
            {
                //
                //  If the Handler reference property value includes
                //  namespace, check if it is the namespace of the Handler.
                //  If the Handler reference property value does not
                //  include namespace, check if the current subscription
                //  namespace is the namespace of the Handler.
                //
                if(instanceNS.isNull()
                    || handlerNamespace == instanceNS)
                {
                    handlerNS = handlerNamespace;
                    handlerMatch = true;
                }
            }
            else
            {
                // Handler namespace is not set and handler name matches
                handlerMatch = true;
                if (instanceNS.isNull())
                {
                    handlerNS = subscriptionNS;
                }
                else
                {
                    handlerNS = instanceNS;
                }
            }
        }
        else
        {
            // Handler name does not match
            handlerMatch = false;
        }
        if (handlerMatch)
        {
            if(handlerCreationClass != String::EMPTY)
            {
                CIMInstance handlerInstance = _client->getInstance(
                    handlerNS, handlerRef);
                Uint32 pos = handlerInstance.findProperty(
                    PEGASUS_PROPERTYNAME_CREATIONCLASSNAME);
                if (pos != PEG_NOT_FOUND)
                {
                    handlerInstance.getProperty(pos).getValue().get
                        (creationClassName);
                }
                if (handlerCreationClass != creationClassName)
                {
                    handlerMatch = false;
                }
            }
        }
    }
    else
    {
        handlerMatch = true;
        //
        // The handler was not specified.
        // Use the handler namespace if specified in the reference.
        //
        CIMNamespaceName instanceNS = handlerRef.getNameSpace();
        if (!instanceNS.isNull())
        {
            handlerNS = instanceNS;
        }
        else
        {
            handlerNS = subscriptionNS;
        }
    }
    return handlerMatch;
}

//
//  Get the subscription state string from a subscription instance
//
void CIMSubCommand::_getEmailInfo(
    const CIMInstance& handlerInstance,
    String& ccString,
    String& toString,
    String& subjectString)
{
    Array <String> mailCc, mailTo;
    subjectString = String::EMPTY;
    mailTo.append(String::EMPTY);
    Uint32 pos =
        handlerInstance.findProperty(PEGASUS_PROPERTYNAME_LSTNRDST_MAILTO);
    if( pos != PEG_NOT_FOUND)
    {
        handlerInstance.getProperty(pos).getValue().get(mailTo);
    }
    for (Uint32 eIndex=0; eIndex < mailTo.size();
        eIndex++)
    {
        toString.append(mailTo[eIndex]);
        toString.append(" ");
    }
    pos = handlerInstance.findProperty(PEGASUS_PROPERTYNAME_LSTNRDST_MAILCC);
    if (pos != PEG_NOT_FOUND)
    {
        handlerInstance.getProperty(pos).getValue().get(mailCc);
    }
    if (mailCc.size() > 0)
    {
        for (Uint32 eIndex=0; eIndex < mailCc.size();
            eIndex++)
        {
            ccString.append (mailCc[eIndex]);
            ccString.append (" ");
        }
    }
    pos = handlerInstance.findProperty(
            PEGASUS_PROPERTYNAME_LSTNRDST_MAILSUBJECT);
    if (pos != PEG_NOT_FOUND)
    {
        handlerInstance.getProperty(pos).getValue().get(subjectString);
    }
}

//
//  Get the persistence value from the handler instance
//
String CIMSubCommand::_getPersistenceType(const CIMInstance& handlerInstance)
{
    Uint16 persistenceType = 1;
    Uint32 pos =
        handlerInstance.findProperty(PEGASUS_PROPERTYNAME_PERSISTENCETYPE);
    if (pos != PEG_NOT_FOUND)
    {
        handlerInstance.getProperty(pos).getValue().get(persistenceType);
    }
    String persistenceString;
    switch (persistenceType)
    {
        case PERSISTENCE_OTHER:
        {
            persistenceString = _PERSISTENTENCE_OTHER_STRING;
            break;
        }
        case PERSISTENCE_PERMANENT:
        {
            persistenceString = _PERSISTENTENCE_PERMANENT_STRING;
            break;
        }
        case PERSISTENCE_TRANSIENT:
        {
            persistenceString = _PERSISTENTENCE_TRANSIENT_STRING;
            break;
        }
        default:
            persistenceString = _PERSISTENTENCE_UNKNOWN_STRING;
      }
      return persistenceString;
}

//
//    Get the subscription state string from a subscription instance
//
String CIMSubCommand::_getSubscriptionState(
    const CIMNamespaceName& subscriptionNamespace,
    const CIMObjectPath& subPath)
{
    CIMInstance subInstance = _client->getInstance(subscriptionNamespace,
        subPath);
    Uint32 pos = subInstance.findProperty(
            PEGASUS_PROPERTYNAME_SUBSCRIPTION_STATE);
    Uint16 subscriptionState = STATE_UNKNOWN;
    if (pos != PEG_NOT_FOUND)
    {
        if (!subInstance.getProperty(pos).getValue().isNull() )
        {
            subInstance.getProperty(pos).getValue().get(subscriptionState);
        }
    }
    String statusString;
    switch (subscriptionState)
    {
        case STATE_UNKNOWN:
        {
            statusString = _SUBSCRIPTION_STATE_UNKNOWN_STRING;
            break;
        }
        case STATE_OTHER:
        {
            statusString = _SUBSCRIPTION_STATE_UNKNOWN_STRING;
            break;
        }
        case STATE_ENABLED:
        {
            statusString = _SUBSCRIPTION_STATE_ENABLED_STRING;
            break;
        }
        case STATE_ENABLEDDEGRADED:
        {
            statusString = _SUBSCRIPTION_STATE_ENABLED_DEGRADED_STRING;
            break;
        }
        case STATE_DISABLED:
        {
            statusString = _SUBSCRIPTION_STATE_DISABLED_STRING;
            break;
        }
        default:
            statusString = _SUBSCRIPTION_STATE_NOT_SUPPORTED_STRING;
    }
    return statusString;
}

//
//  Get the SNMP version string from a handler instance
//
String CIMSubCommand::_getSnmpVersion (const CIMInstance& handlerInstance)
{
    Uint16 snmpVersion = 0;
    Uint32 pos = handlerInstance.findProperty(PEGASUS_PROPERTYNAME_SNMPVERSION);
    if (pos != PEG_NOT_FOUND)
    {
        if (!handlerInstance.getProperty(pos).getValue().isNull() )
        {
            handlerInstance.getProperty(pos).getValue().get(snmpVersion);
        }
    }

    String snmpVersionString;
    switch (snmpVersion)
    {
        case SNMPV1_TRAP:
            snmpVersionString = _SNMP_VERSION_SNMPV1_TRAP_STRING;
            break;
        case SNMPV2C_TRAP:
            snmpVersionString = _SNMP_VERSION_SNMPV2C_TRAP_STRING;
            break;
        default:
            snmpVersionString = _SNMP_VERSION_PEGASUS_RESERVED_STRING;
    }
    return snmpVersionString;
}

//
//    print data in a columnar form
//
void CIMSubCommand::_printColumns(
    const Array <Uint32>& maxColumnWidth,
    const Array <ListColumnEntry>& listOutputTable,
    ostream& outPrintWriter)
{
    Uint32 maxColumns = maxColumnWidth.size();
    Uint32 maxEntries = listOutputTable[0].size();
    Array <Uint32> indexes;
    for (Uint32 i = 0; i < maxEntries; i++)
    {
       indexes.append(i);
    }
    for (int column = maxColumns-1; column >= 0; column--)
    {
      _bubbleIndexSort(listOutputTable[column], 1, indexes);
    }
    for (Uint32 i = 0; i < maxEntries; i++)
    {
        for (Uint32 column = 0; column < maxColumns-1; column++)
        {
            Uint32 outputItemSize =
                (listOutputTable[column])[indexes[i]].size();
            Uint32 fillerLen = maxColumnWidth[column] + TITLE_SEPERATOR_LEN -
                outputItemSize;
            outPrintWriter << (listOutputTable[column])[indexes[i]];
            for (Uint32 j = 0; j < fillerLen; j++)
            {
                outPrintWriter << ' ';
            }
        }
        outPrintWriter << (listOutputTable[maxColumns-1])[indexes[i]] << endl;
    }
}

//
//    Sort a string array by indexes
//
void CIMSubCommand::_bubbleIndexSort(
    const Array<String>& x,
    const Uint32 startIndex,
    Array<Uint32>& index)
{
    Uint32 n = x.size();

    if (n < 3)
        return;

    for (Uint32 i = startIndex; i < (n-1); i++)
    {
        for (Uint32 j = startIndex; j < (n-1); j++)
        {
            if (String::compareNoCase(x[index[j]],
                                      x[index[j+1]]) > 0)
            {
                Uint32 t = index[j];
                index[j] = index[j+1];
                index[j+1] = t;
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
    AutoPtr<CIMSubCommand> command;
    Uint32 retCode;

#ifdef PEGASUS_OS_PASE
    Uint32 ret = 0;
    ret = umeCheckCmdAuthorities(COMMAND_NAME, true);
    if (ret)
    {
        return Command::RC_ERROR;
    }
#endif
#ifdef PEGASUS_OS_ZOS
    // for z/OS set stdout and stderr to EBCDIC
    setEBCDICEncoding(STDOUT_FILENO);
    setEBCDICEncoding(STDERR_FILENO);
#endif

    MessageLoader::_useProcessLocale = true;
    MessageLoader::setPegasusMsgHomeRelative(argv[0]);

    command.reset(new CIMSubCommand());
    try
    {
        command->setCommand (cout, cerr, argc, argv);
    }
    catch (CommandFormatException& cfe)
    {
        cerr << COMMAND_NAME << ": " << cfe.getMessage() << endl;

        MessageLoaderParms parms(ERR_USAGE_KEY,ERR_USAGE);
        parms.msg_src_path = MSG_PATH;
        cerr << COMMAND_NAME <<
            ": " << MessageLoader::getMessage(parms) << endl;

        exit (Command::RC_ERROR);
    }
    retCode = command->execute(cout, cerr);
    return (retCode);
}
