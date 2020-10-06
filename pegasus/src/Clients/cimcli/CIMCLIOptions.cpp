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

/*******************************************************************************
*
* Command line options processing.  All option processing is based on the
* Pegasus OptionManager and the definition of options contained in this
* file. This file also contains the funtions to process input options.
*
* Once processed, the input options are placed into the options structure
* to be passed to the operation processors.
*
*******************************************************************************/
#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/Constants.h>
#include <Pegasus/Common/PegasusAssert.h>
#include <Pegasus/Common/FileSystem.h>
#include <Pegasus/Common/XmlWriter.h>
#include <Pegasus/General/MofWriter.h>
#include <Pegasus/Common/Tracer.h>
#include <Pegasus/Common/StringConversion.h>
#include <Pegasus/Common/ArrayInternal.h>
#include <Pegasus/Common/PegasusVersion.h>
#include <Pegasus/Common/Pegasus_inl.h>

#include "CIMCLIOptions.h"
#include "CIMCLIClient.h"
#include "CIMCLIHelp.h"
#include "CIMCLICommon.h"
#include "CIMCLIClient.h"

PEGASUS_USING_STD;
PEGASUS_NAMESPACE_BEGIN

/*******************************************************************************
**
**  BuildOptionsTable
**
*******************************************************************************/
/*
    This function builds the complete options table from the entries defined
    below.  It then merges in the options from any config file and from any
    command line parameters. The command line parameters will override
    any config file parameters.
*/
void BuildOptionsTable(
    OptionManager& om,
    int& argc,
    char** argv,
    const String& testHome)
{
    // Table of available options to be used by cimcli.  Each of the possible
    // options is defined in one entry in the table below.

    static OptionRowWithMsg optionsTable[] =
        //optionname defaultvalue rqd  type domain domainsize clname msgkey
        // hlpmsg
    {
        // FUTURE TODO.  This strange number was a way to tell option parser
        // that the option was not found on the command line.  That concept
        //  does not exist today in the OptionManager. The error mechanism
        // tells you whether the option name is defined not whether there was
        // an option supplied.  Need to fix OptionManager
        {"count", "29346", false, Option::WHOLE_NUMBER, 0, 0, "count",
        "Clients.cimcli.CIMCLIClient.COUNT_OPTION_HELP",
        "Expected count of objects returned if summary set.\n"
            "    Tests this count and displays difference.\n"
            "    Return nonzero status code if test fails"},

        {"debug", "false", false, Option::BOOLEAN, 0, 0, "d",
        "Clients.cimcli.CIMCLIClient.DEBUG_OPTION_HELP",
        "More detailed debug messages"},

        {"delay", "0", false, Option::WHOLE_NUMBER, 0, 0, "delay",
        "Clients.cimcli.CIMCLIClient.DELAY_OPTION_HELP",
        "Delay between connection and request"},

        {"Password", "", false, Option::STRING, 0, 0, "p",
        "Clients.cimcli.CIMCLIClient.PASSWORD_OPTION_HELP",
        "Defines password for authentication" },

        {"location", "", false, Option::STRING, 0, 0, "l",
        "Clients.cimcli.CIMCLIClient.LOCATION_OPTION_HELP",
        "Specifies system and port (HostName:port).\n"
            "    Port is optional" },

#ifdef PEGASUS_HAS_SSL
        {"ssl", "false", false, Option::BOOLEAN, 0, 0, "s",
        "Clients.cimcli.CIMCLIClient.SSL_OPTION_HELP",
        "Specifies to connect over HTTPS" },

        {"clientCert", "", false, Option::STRING, 0, 0, "-cert",
        "Clients.cimcli.CIMCLIClient.CLIENTCERT_OPTION_HELP",
        "Specifies a client certificate file path to present to the server.\n"
            "    This is optional and only has an effect on connections\n"
            "    made over HTTPS using -s. If this option specified the\n"
            "    clientKey must also exist" },

        {"clientKey", "", false, Option::STRING, 0, 0, "-key",
        "Clients.cimcli.CIMCLIClient.CLIENTKEY_OPTION_HELP",
        "Specifies a client private key file path.\n"
            "    This is optional and only has an effect on connections\n"
            "    made over HTTPS using -s. If this option specified the\n"
            "    clientCert must also exist" },

        {"clientTruststore", "", false, Option::STRING, 0, 0, "-truststore",
        "Clients.cimcli.CIMCLIClient.CLIENTKEY_OPTION_HELP",
        "Specifies a path to a client trust store cused to verify server\n"
            "    certificates. This is optional and only has an effect"
            "     on connections made over HTTPS using -s\n"},
#endif
        {"User", "", false, Option::STRING, 0, 0, "u",
        "Clients.cimcli.CIMCLIClient.USER_OPTION_HELP",
        "Defines User Name for authentication" },

        {"namespace", "root/cimv2", false, Option::STRING, 0, 0, "n",
        "Clients.cimcli.CIMCLIClient.NAMESPACE_OPTION_HELP",
        "Specifies namespace to use for operation" },

        {"includeClassOrigin", "false", false, Option::BOOLEAN, 0, 0, "ic",
        "Clients.cimcli.CIMCLIClient.INCLUDECLASSORIGIN_OPTION_HELP",
        "If set includeClassOrigin  parameter\n"
            "    set true on requests that honor this parameter"},

        {"deepInheritance", "false", false, Option::BOOLEAN, 0, 0, "di",
        "Clients.cimcli.CIMCLIClient.DEEPINHERITANCE_OPTION_HELP",
        "If set deepInheritance parameter\n"
            "    set true"},

        // FUTURE - Drop this option completely
        {"localOnly", "true", false, Option::BOOLEAN, 0, 0, "lo",
        "Clients.cimcli.CIMCLIClient.LOCALONLY_OPTION_HELP",
        "DEPRECATED. This was used to set LocalOnly.\n"
            "    However, default should be true and we cannot use True\n"
            "    as default. See -nlo"},

        {"notLocalOnly", "false", false, Option::BOOLEAN, 0, 0, "nlo",
        "Clients.cimcli.CIMCLIClient.NOTLOCALONLY_OPTION_HELP",
        "When set, sets LocalOnly = false on\n"
            "    operations"},

        {"includeQualifiers", "false", false, Option::BOOLEAN, 0, 0, "iq",
        "Clients.cimcli.CIMCLIClient.INCLUDEQUALIFIERS_OPTION_HELP",
        "DEPRECATED. Sets includeQualifiers = True.\n"
            "    Useful for instance operations where default is false"},

        {"notIncludeQualifiers", "false", false, Option::BOOLEAN, 0, 0, "niq",
        "Clients.cimcli.CIMCLIClient.NOTINCLUDEQUALIFIERS_OPTION_HELP",
        "Sets includeQualifiers = false\n"
            "    on operations. Useful for class operations where \n"
            "    the default is true."},

        // Uses a magic string as shown below to indicate never used.
        {"propertyList", "###!###", false, Option::STRING, 0, 0, "pl",
        "Clients.cimcli.CIMCLIClient.PROPERTYLIST_OPTION_HELP",
        "Defines a propertyNameList. Format is p1,p2,p3\n"
            "    (without spaces). Use \"\" for empty"},

        {"assocClass", "", false, Option::STRING, 0, 0, "ac",
        "Clients.cimcli.CIMCLIClient.ASSOCCLASS_OPTION_HELP",
        "Defines a assocation Class string for Associator calls"},

        {"assocRole", "", false, Option::STRING, 0, 0, "ar",
        "Clients.cimcli.CIMCLIClient.ASSOCROLE_OPTION_HELP",
        "Defines a role string for Associators. AssocRole\n"
            "    parameter"},

        {"role", "", false, Option::STRING, 0, 0, "r",
        "Clients.cimcli.CIMCLIClient.ROLE_OPTION_HELP",
        "Defines a role string for reference role parameter"},

        {"resultClass", "", false, Option::STRING, 0, 0, "rc",
        "Clients.cimcli.CIMCLIClient.RESULTCLASS_OPTION_HELP",
        "Defines a resultClass string for References and\n"
            "    Associatiors"},

        {"resultRole", "", false, Option::STRING, 0, 0, "rr",
        "Clients.cimcli.CIMCLIClient.RESULTROLE_OPTION_HELP",
        "Defines a role string for associators operation resultRole\n"
            "    parameter"},

        // This options has been deprecated and its functionality removed
        // Keeping it simply as means to explain the issue to users in case
        // they try to use it.
        {"inputParameters", "", false, Option::STRING, 0, 0, "ip",
        "Clients.cimcli.CIMCLIClient.INPUTPARAMETERS_OPTION_HELP",
        "This option deprecated and removed. Replaced by use of\n"
            "    the same name/value pair syntax as properties in create\n"
            "    and modify instance."},

        {"filter", "", false, Option::STRING, 0, 0, "f",
        "Clients.cimcli.CIMCLIClient.FILTER_OPTION_HELP",
        "Defines a filter to use for query. Single String input"},

        {"queryLanguage", "WQL", false, Option::STRING, 0, 0, "ql",
        "Clients.cimcli.CIMCLIClient.QUERYLANGUAGE_OPTION_HELP",
        "Defines a Query Language to be used with a query filter.\n"},

        // Defines the multiple output formats. List of possible options
        // defined in CIMCLIOptionStruct files. default mof.
        // FUTURE - Add simple option that uses the enum features of
        // options manager.  However. we cannot remove this option.
        {"outputformats", "mof", false, Option::STRING, 0,0,
        "o",
        "Clients.cimcli.CIMCLIClient.OUTPUTFORMATS_OPTION_HELP",
        "Output in xml, mof, txt, table"},

        {"xmlOutput", "false", false, Option::BOOLEAN, 0,0, "x",
        "Clients.cimcli.CIMCLIClient.XMLOUTPUT_OPTION_HELP",
        "Output objects in xml format"},

        {"version", "false", false, Option::BOOLEAN, 0, 0, "-version",
        "Clients.cimcli.CIMCLIClient.VERSION_OPTION_HELP",
        "Displays software Version"},

        {"verbose", "false", false, Option::BOOLEAN, 0, 0, "v",
        "Clients.cimcli.CIMCLIClient.VERBOSE_OPTION_HELP",
        "Verbose Display. Outputs detailed parameter input\n"
            "    display and other request processing information"},

        {"summary", "false", false, Option::BOOLEAN, 0, 0, "-sum",
        "Clients.cimcli.CIMCLIClient.SUMMARY_OPTION_HELP",
        "Displays only summary count for enumerations,\n"
            "    associators, etc."},

        {"help", "false", false, Option::BOOLEAN, 0, 0, "h",
        "Clients.cimcli.CIMCLIClient.HELP_OPTION_HELP",
        "Prints help usage message"},

        {"full help", "false", false, Option::BOOLEAN, 0, 0, "-help",
        "Clients.cimcli.CIMCLIClient.FULLHELP_OPTION_HELP",
        "Prints full help message with commands, options,\n"
            "    examples"},

        {"help options", "false", false, Option::BOOLEAN, 0, 0, "ho",
        "Clients.cimcli.CIMCLIClient.HELPOPTIONS_OPTION_HELP",
        "Prints list of options"},

        {"help commands", "false", false, Option::BOOLEAN, 0, 0, "hc",
        "Clients.cimcli.CIMCLIClient.HELPCOMMANDS_OPTION_HELP",
        "Prints CIM Operation command list"},

        {"connecttimeout", "0", false, Option::WHOLE_NUMBER, 0, 0, "-timeout",
        "Clients.cimcli.CIMCLIClient.CONNECTIONTIMEOUT_OPTION_HELP",
        "Set the connection timeout in seconds."},

        {"interactive", "false", false, Option::BOOLEAN, 0, 0, "i",
        "Clients.cimcli.CIMCLIClient.INTERACTIVE_OPTION_HELP",
        "Interactively ask user to select instances.\n"
            "    Used with associator and reference operations"},

        {"setRtnHostNames", "", false, Option::STRING, 0, 0,
             "-setRtnHostNames",
            "Clients.cimcli.CIMCLIClient.SETRTNHOSTNAMES_OPTION_HELP",
            "Set namespace component of reference and path outputs parameter.\n"
            "    Used to allow comparison of paths and instances without"
            " involving the variable of host namespaces."},

        {"trace", "0", false, Option::WHOLE_NUMBER, 0, 0, "trace",
        "Clients.cimcli.CIMCLIClient.TRACE_OPTION_HELP",
        "Set Pegasus Common Components Trace. Sets the Trace level.\n"
            "    0 is off"},

        {"repeat", "0", false, Option::WHOLE_NUMBER, 0, 0, "-r",
        "Clients.cimcli.CIMCLIClient.REPEAT_OPTION_HELP",
        "Number of times to repeat the operation.\n"
            "    Zero means one time"},

        {"time", "false", false, Option::BOOLEAN, 0, 0, "-t",
        "Clients.cimcli.CIMCLIClient.TIME_OPTION_HELP",
        "Measure time for the operation and present results"},

//EXP_PULL_BEGIN
        {"pullTimeout", "10", false, Option::WHOLE_NUMBER, 0, 0, "pt",
            "Clients.cimcli.CIMCLIClient.PULLTIMEOUT",
            "Pull interoperation timeout in seconds. "},

        {"maxObjectCount", "100", false, Option::WHOLE_NUMBER, 0, 0, "mo",
            "Clients.cimcli.CIMCLIClient.MAXOBJCNT",
            "Maximum number of objects in a single Pull "},

        {"maxObjectsToReceive", "0", false, Option::WHOLE_NUMBER, 0, 0, "mr",
        "Clients.cimcli.CIMCLIClient.MAXOBJRCV",
            "Maximum objects to Receive in a pull sequence. "
            "Will Close when this number received."},

        {"pullDelay", "0", false, Option::WHOLE_NUMBER, 0, 0, "pullDelay",
        "Clients.cimcli.CIMCLIClient.PULLDELAY",
            "Delay in Seconds between pull Operations. "
            "Default zero, no delay."},
// EXP_PULL_END

        {"sort", "false", false, Option::BOOLEAN, 0, 0, "-sort",
        "Clients.cimcli.CIMCLIClient.SORT_OPTION_HELP",
        "Sort the returned entities for multi-object responses "
        "(ex. enumerations, associators, etc.)"},

        {"expectedExitCode", "0", false, Option::WHOLE_NUMBER, 0, 0, "-expExit",
        "Clients.cimcli.CIMCLIClient.EXPEXIT_OPTION_HELP",
        "Set the exit code that cimcli expects for this operation. cimcli"
        " exits code = 0 if expectedExitCode matches pgm exit code."}
    };
    const Uint32 NUM_OPTIONS = sizeof(optionsTable) / sizeof(optionsTable[0]);

    // Register all of the options in the table above
    om.registerOptions(optionsTable, NUM_OPTIONS);

    // Merge any options from the config file if it exists
    String configFile = "cimcli.conf";

    if (FileSystem::exists(configFile))
    {
        om.mergeFile(configFile);
    }

    // Merge options from the command line
    om.mergeCommandLine(argc, argv);

    om.checkRequiredOptions();
}

/*
    Execute lookup on option.  This used because the OptionTable functions
    generally return error when the option is not found.  In cimcli this is
    really a developer error (i.e option not in the table) so we isolate it
    from the general lookup functionality. This function exits if the
    lookup fails, that means that there is a discrepency between the table
    of options and the names in the lookup functions. Fix the table.
    The error exit in this function should NEVER happen in a released
    version of cimcli.
*/
static const Option* _lookupOption(OptionManager& om, const char* optionName)
{
    const Option* op = om.lookupOption(optionName);

    // if option does not exist in table, terminate.  All options must exist
    // in the table. This is a programming error between the input
    // parsing code and the table defining options.
    if (op == 0)
    {
               cerr << "Parse Error in " << optionName
                   << " Name not valid cimcli option. Fix options table"
                    << endl;
               cimcliExit(CIMCLI_INPUT_ERR);
    }
    return op;
}

// Get the value of the option if it was resolved.  Note that
// resolved means that it was input on either the command line or
// a configuration file.
// @param opts - reference to the options structure
// @param om -reference to the OptionManger where the data is saved.
// @param optionName - Name of the option for which we want the value
// @param resolvedStateVariable Boolean that is set true if the option has
//     been resolved (i.e. provided by either the config file or command line)
// @param optsTarget String containing the value of the parameter if it
//    was resolved.
// @return Boolean indicating whether the parameter was found in the table


Boolean lookupStringResolvedOption(Options& opts,
    OptionManager& om,
    const char* optionName,
    Boolean& resolvedStateVariable,
    String& optsTarget)
{
    // test for existing option
    const Option* op = _lookupOption(om, optionName);

    if (op->isResolved())
    {
        resolvedStateVariable = true;
        optsTarget = op->getValue();

        if (opts.verboseTest && opts.debug)
        {
            cout << optionName << " = " << optsTarget << endl;
        }
    }
    else
    {
        resolvedStateVariable = false;
    }
    return resolvedStateVariable;
}

/*
    Lookup string option and insert into target property
    Uses the table defined default directly if nothing input from
    either command line or config file.
*/
void lookupStringOption(Options& opts,
    OptionManager& om,
    const char* optionName,
    String& optsTarget)
{
    // Test for existing option. If nothing found, this
    // function exits cimcli with error status
    _lookupOption(om, optionName);

    if (om.lookupValue(optionName, optsTarget))
    {
        if (opts.verboseTest && opts.debug)
            cout << optionName << " = " << optsTarget << endl;
    }
}

// Looks up a String option by name.  If the returned value is empty
// insures that it is value by setting to String::EMPTY
void lookupStringOptionEMPTY(Options& opts,
    OptionManager& om,
    const char* optionName,
    String& optsTarget)
{
    // Test for Existing Option
    _lookupOption(om, optionName);

    String temp;
    if (om.lookupValue(optionName, temp))
    {
        optsTarget = (temp.size() == 0)? String::EMPTY : temp;

        if (opts.verboseTest && opts.debug)
        {
            cout << optionName << " = " << optsTarget << endl;
        }
    }
}

Boolean lookupCIMNameOption(Options& opts,
    OptionManager& om,
    const char* optionName,
    CIMName& optsTarget,
    const CIMName& defaultValue)
{
    // Test for existing option
    _lookupOption(om, optionName);

    String temp;
    if (om.lookupValue(optionName, temp))
    {
       if (temp != "")
       {
           //Assigning to CIMName can cause exception.
           try
           {
               optsTarget = temp;
           }
           catch(Exception& e)
           {
               cerr << "Parse Error in " << optionName << " Class. Exception "
                    << e.getMessage()
                    << endl;
               return false;
           }
       }
       else
           optsTarget = defaultValue;

       if (opts.verboseTest && opts.debug && temp != "")
       {
           cout << optionName << " = " << optsTarget.getString() << endl;
       }
    }
    return true;
}

// Lookup a single Uint32 option.  NOTE: The issue here is with detecting
// whether the option exists or we should use the internal default.
// Return from the option manager is the defined default which is itself
// an integer.
void lookupUint32Option(Options& opts,
    OptionManager& om,
    const char* optionName,
    Uint32& optsTarget,
    Uint32 defaultValue,
    const char* units = "")
{
    // Test for existing option
    _lookupOption(om, optionName);

    optsTarget = 0;
    if (!om.lookupIntegerValue(optionName, optsTarget))
    {
        optsTarget = 0;
    }

    if (opts.verboseTest && opts.debug && optsTarget != 0)
    {
        cout << optionName << " = "
            << optsTarget << units
            << endl;
    }
}

//EXP_PULL_BEGIN
// Lookup a single Uint32 option.  NOTE: The issue here is with detecting
// whether the option exists or we should use the internal default.
// Return from the option manager is the defined default which is itself
// an integer.
void lookupUint32ArgOption(Options& opts,
                   OptionManager& om,
                   const char* optionName,
                   Uint32Arg& optsTarget,
                   Uint32 defaultValue,
                   const char* units = "")
{
    // KS_TBD - Expand this to allow the NULL options which is part of
    // Uint32Arg

    optsTarget.setNullValue();
    Uint32 temp;
    if (!om.lookupIntegerValue(optionName, temp))
    {
        optsTarget.setValue(0);
    }

    // KS_TBD - This is temporary until we expand option manager to support
    // the numeric arg concept.
    optsTarget.setValue(temp);

    if (opts.verboseTest && opts.debug &&
        (optsTarget.getValue() != 0 || !optsTarget.isNull() ) )
    {
        cout << optionName << " = " << optsTarget.toString()
        << " " << units
            << endl;
    }
}
//EXP_PULL_END

// Lookup value only if the option was resolved. Ignores default values.
Boolean lookupUint32ResolvedOption(Options& opts,
    OptionManager& om,
    const char* optionName,
    Boolean& resolvedStateVariable,
    Uint32& optsTarget,
    Uint32 defaultValue,
    const char* units = "")
{
    // Test for existing Option
    const Option* op = _lookupOption(om, optionName);

    optsTarget = 0;
    if (op->isResolved())
    {
        resolvedStateVariable = true;
        const String value = op->getValue();
        Uint64 u64 = strToUint(value.getCString(), CIMTYPE_UINT32);
        optsTarget = (Uint32)u64;
    }
    else
    {
        resolvedStateVariable = false;
    }
    return resolvedStateVariable;
}

void lookupBooleanOption(Options& opts,
                   OptionManager& om,
                   const char* optionName,
                   Boolean& optsTarget)
{
    // Test for existing option
    _lookupOption(om, optionName);

    optsTarget = om.isTrue(optionName);
    if (optsTarget  && opts.verboseTest && opts.debug)
    {
        cout << optionName << " = " << boolToString(optsTarget) << endl;
    }
}

void lookupBooleanOptionNegate(Options& opts,
                   OptionManager& om,
                   const char* optionName,
                   Boolean& optsTarget)
{
    // Test for existing option
    _lookupOption(om, optionName);

    optsTarget = !om.isTrue(optionName);
    if (optsTarget  && opts.verboseTest && opts.debug)
    {
        cout << optionName << " = " << boolToString(optsTarget) << endl;
    }
}

Boolean CheckCommonOptionValues(OptionManager& om, char** argv, Options& opts)
{
    Uint32 lineLength = 79;
    // Catch the verbose and debug options first so they can control other
    // processing
    Boolean verboseTest = (om.valueEquals("verbose", "true")) ? true :false;
    Boolean debug = (om.valueEquals("debug", "true")) ? true :false;

    if (verboseTest)
    {
        opts.verboseTest = verboseTest;
    }

    if (debug)
    {
        opts.debug= debug;
    }

    if (om.isTrue("full help"))
    {
        showFullHelpMsg(argv[0], om, lineLength);
        return false;   // signal cimcli should terminate after display
    }

    // show usage for a single operation and exit
    if (om.isTrue("help"))
    {
        if (!showOperationUsage(argv[1], om, lineLength))
        {
            opts.termCondition = CIMCLI_INPUT_ERR;
            return false;
        }
        return false;
    }

    // show version number
    if (om.isTrue("version"))
    {
        showVersion(argv[0], om);
        return false;
    }

    // show all help options
    if (om.isTrue("help options"))
    {
        showOptions(argv[0], om);
        return false;
    }

    // show help Operation list
    if (om.isTrue("help commands"))
    {
        showOperations(argv[0], lineLength);
        return false;
    }

    lookupStringOption(opts, om, "namespace", opts.nameSpace);

    lookupStringOptionEMPTY(opts, om, "role", opts.role);

    lookupStringOptionEMPTY(opts, om, "resultRole", opts.resultRole);

    lookupStringOption(opts, om, "location", opts.location);

#ifdef PEGASUS_HAS_SSL
    // Determine whether to connect over HTTPS
    opts.ssl = om.isTrue("ssl");

    // Get value for client certificate
    om.lookupValue("clientCert", opts.clientCert);

    // Get value for client key
    om.lookupValue("clientKey", opts.clientKey);

    // Get value for client key
    om.lookupValue("clientTruststore", opts.clientTruststore);

    if (verboseTest && debug && opts.ssl)
    {
        cout << "ssl = true" << endl;
        if (opts.clientCert != "" && opts.clientKey != "")
        {
            cout << "clientCert = " << opts.clientCert << endl;
            cout << "clientKey = " << opts.clientKey << endl;
            if (opts.clientTruststore.size() != 0)
            {
                cout << "clientTruststore path = "
                     << opts.clientTruststore << endl;
            }
        }
    }
#endif

    // Assign the result class
    if (!lookupCIMNameOption(opts, om, "resultClass",
                             opts.resultClass, CIMName()))
    {
        opts.termCondition = CIMCLI_INPUT_ERR;
        return false;
    }

    if (!lookupCIMNameOption(opts, om, "assocClass",
                        opts.assocClass, CIMName()))
    {
        opts.termCondition = CIMCLI_INPUT_ERR;
        return false;
    }

    // Evaluate connectiontimeout option.
    lookupUint32Option(opts, om, "connecttimeout", opts.connectionTimeout, 0,
        "seconds");

    lookupUint32Option(opts, om, "delay", opts.delay, 0, "seconds");

//EXP_PULL_BEGIN
    // Options to support parameters on pull operations
    lookupUint32ArgOption(opts, om, "pullTimeout",
                          opts.pullOperationTimeout, 0, "seconds");
    lookupUint32Option(opts, om, "maxObjectCount",
                       opts.maxObjectCount, 0, "max rtn");

    lookupUint32Option(opts, om, "pullDelay", opts.pullDelay, 0, "seconds");
    lookupUint32Option(opts, om, "maxObjectsToReceive",
                       opts.maxObjectsToReceive, 0, "seconds");
//EXP_PULL_END

    // Set the interactive request flag based on input
    lookupBooleanOption(opts, om,"interactive", opts.interactive);

    // Set the sort request flag based on input
    lookupBooleanOption(opts, om,"sort", opts.sort);

    // set the deepInheritance flag based on input
    lookupBooleanOption(opts, om,"deepInheritance", opts.deepInheritance);

    // only use this one if there was an input from the command line
    // or config file. sets the Boolean setRtnHostNames based on whether
    // there was an option input (i.e. resolved).
    lookupStringResolvedOption(opts, om,
                                   "setRtnHostNames",
                                   opts.setRtnHostNames,
                                   opts.rtnHostSubstituteName);

    lookupStringOption(opts, om, "filter", opts.query);

    lookupStringOption(opts, om, "queryLanguage", opts.queryLanguage);

    // Test for existence of input parameter option.  If found, put out
    // warning and exit with syntax error message.  This parameter was
    // deprecated and removed in favor of direct input of name/value pairs
    // as separate input entities in CIM Version 2.10
    Boolean inputParametersResolved = false;
    String inputParameters;
    lookupStringResolvedOption(opts, om,
                                   "inputParameters",
                                   inputParametersResolved,
                                   inputParameters);
    if (inputParametersResolved)
    {
        cerr << "The -ip option has been deprecated and removed.\n"
              "    parameters can be directly input as name/value pairs\n"
              "    in the same manner properties are input to the\n"
              "    createInstance and other operations\n" << endl;
        opts.termCondition = CIMCLI_INPUT_ERR;
        return false;
    }

    // process localOnly and notlocalOnly parameters
    opts.localOnly = om.isTrue("localOnly");
    if (om.isTrue("notLocalOnly"))
    {
        opts.localOnly = false;
    }

    // Used the not version because the DMTF and pegasus default is true
    if (verboseTest && debug && om.isTrue("notLocalOnly"))
    {
        cout << "localOnly= " << boolToString(opts.localOnly) << endl;;
    }

    // Process includeQualifiers and notIncludeQualifiers
    // These are particular in that there are two parameters each
    // of which may be useful. Also, the CIM/XML default is different
    // for class operations and instance operations.  In class operations
    // the default is true while, in instance operations, the default
    // is false and further, the whole use of the parameter is deprecated for
    // instance operations.
    // For each of these parameters we want to specifically confirm if
    // the command line input parameter is supplied. Thus, for class operations
    // the user would use the niq to tell the environment to not include
    // qualifiers whereas for instance operations the user would use iq
    // to specifically request qualifiers with instances.

    lookupBooleanOption(opts, om, "includeQualifiers",
        opts.includeQualifiersRequested);

    lookupBooleanOption(opts, om, "notIncludeQualifiers",
        opts.notIncludeQualifiersRequested);

    lookupBooleanOption(opts, om,"includeClassOrigin",
        opts.includeClassOrigin );

    lookupBooleanOption(opts, om,"time", opts.time);

    if (!om.lookupIntegerValue("trace", opts.trace))
    {
            opts.trace = 0;
    }
    else
    {
        Uint32 traceLevel = 0;
        switch (opts.trace)
        {
            case 0:             // This covers the default.
                break;
            case 1 :
                traceLevel = Tracer::LEVEL1;
                break;
            case 2 :
                traceLevel = Tracer::LEVEL2;
                break;
            case 3 :
                traceLevel = Tracer::LEVEL3;
                break;
            case 4 :
                traceLevel = Tracer::LEVEL4;
                break;
            default:
                cerr << "Illegal value for Trace. Max = 4" << endl;
        }
        opts.trace = traceLevel;
    }
    if (verboseTest && debug && opts.trace != 0)
    {
        cout << "Pegasus Trace set to  Level  " << opts.trace << endl;
    }

    lookupBooleanOption(opts, om,"summary", opts.summary);

    // get User name and password if set.
    lookupStringOptionEMPTY(opts, om, "User", opts.user);
    lookupStringOptionEMPTY(opts, om, "Password", opts.password);

    // Test for outputFormats parameter with valid type string
    if (om.lookupValue("outputformats", opts.outputTypeParamStr))
     {
        if (debug && verboseTest)
        {
            cout << "Output Format = " << opts.outputTypeParamStr << endl;
        }
     }

    // test for valid string on output type.
    if ((opts.outputType = OutputTypeStruct::getOutputType(
        opts.outputTypeParamStr)) == OUTPUT_TYPE_ILLEGAL )
    {
        cerr << "Error: Invalid Output Type " << opts.outputTypeParamStr
             << ". Valid types are: "<< OutputTypeStruct::listOutputTypes()
             << endl;
            opts.termCondition = CIMCLI_INPUT_ERR;
            return false;    }

    // Test for special output option -x
    // Note that this is after the general format definition so that it
    // overrides any choice made with -o

    Boolean xmlTest = om.isTrue("xmlOutput");
    if (xmlTest)
    {
        opts.outputType = OUTPUT_XML;
        if (debug && verboseTest)
        {
            cout << "xmlOutput set" << endl;
        }
    }

    lookupUint32Option(opts, om, "repeat", opts.repeat, 0, "times");

    Uint32 tempCode = 0;
    lookupUint32Option(opts, om, "expectedExitCode", tempCode, 0, "");
    if (tempCode != 0)
    {
        setExpectedExitCode(tempCode);
        if (debug && verboseTest)
        {
            cout << "expectedExitCode="
                << tempCode
                << endl;
        }
    }

    lookupUint32ResolvedOption(opts, om, "count", opts.executeCountTest,
                               opts.expectedCount,
                               0, "Comparison Count");

    /*  Property List parameter.
        Separate an input stream into an array of Strings
        Two special situations, empty list and NULL list
        Use NULL when there is no list. This means return all
        Use empty if if you want no properties in the response
        NOTE: We use the ###!### to represent no input of parameter
    */
    {
        String properties;
        if (om.lookupValue("propertyList", properties))
        {
            // om default.  No property list input
            if (properties == "###!###")
            {
                opts.propertyList.clear();
            }
            // propertylist input empty.
            // Account for inputter error where they try to input string
            // representing two quotes
            else if (properties.size() == 0 || properties == "\"\"")
            {
                Array<CIMName> pList;
                opts.propertyList = pList;
            }
            else
            {
                Array<CIMName> pList;
                // tokenize everything separated by commas
                Array<String> pListString =  _tokenize(properties, ',', true);

                for (Uint32 i = 0 ; i < pListString.size(); i++)
                {
                    try
                    {
                        pList.append(CIMName(pListString[i]));
                    }
                    catch (InvalidNameException&)
                    {
                        throw OMInvalidOptionValue("propertyList", properties);
                    }
                }
                opts.propertyList.set(pList);
            }
            if (debug && verboseTest && properties != "###!###")
            {
                cout << "PropertyList= " << opts.propertyList.toString()
                     << endl;
            }
        }
    }
    return true;
}

PEGASUS_NAMESPACE_END
// END_OF_FILE
