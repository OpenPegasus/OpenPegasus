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
#include <Pegasus/Common/CIMName.h>
#include <Pegasus/Common/CIMNameCast.h>
#include <Pegasus/Common/Constants.h>
#include <Pegasus/Common/String.h>
#include <Pegasus/Common/PegasusVersion.h>
#include <Pegasus/Common/XmlWriter.h>
#include <Pegasus/Common/TimeValue.h>
#include <Pegasus/Common/Exception.h>
#include <Pegasus/Common/FileSystem.h>
#include <Pegasus/Common/System.h>
#include <Pegasus/General/VersionUtil.h>
#include <Pegasus/getoopt/getoopt.h>
#include <Clients/cliutils/CommandException.h>
#include "RepositoryUpgrade.h"

#if defined(PEGASUS_USE_RELEASE_DIRS) \
    && defined (PEGASUS_OVERRIDE_DEFAULT_RELEASE_DIRS)
# include <Pegasus/Config/ProductDirectoryStructure.h>
#endif

#ifdef PEGASUS_OS_ZOS
#include <Pegasus/General/SetFileDescriptorToEBCDICEncoding.h>
#endif

// Enables debug information.
// #define REPUPGRADE_DEBUG 1

PEGASUS_NAMESPACE_BEGIN

// Constant initializations.

//l10n
/**
 * The CLI message resource name
 */
static const char MSG_PATH []                  = "pegasus/pegasusCLI";

/**
    Constant for SystemName property
*/
const CIMName _PROPERTY_SYSTEMNAME = CIMNameCast("SystemName");
/**
    The command name.
 */
const char   RepositoryUpgrade::COMMAND_NAME []      = "repupgrade";

/**
    Label for the usage string for this command.
 */
const char   RepositoryUpgrade::_USAGE []            = "Usage: ";

/**
    This constant signifies that an option has not
    been recorded
*/
const Uint32 RepositoryUpgrade::_OPTION_TYPE_UNINITIALIZED = 0;

/**
    This constant signifies that an old repository path option has been
    been recorded
*/
const Uint32 RepositoryUpgrade::_OPTION_TYPE_OLD_REPOSITORY_PATH = 1;

/**
    This constant signifies that an new repository path option has been
    been recorded
*/
const Uint32 RepositoryUpgrade::_OPTION_TYPE_NEW_REPOSITORY_PATH = 2;

/**
    The constant representing a help operation
*/
const Uint32 RepositoryUpgrade::_OPTION_TYPE_HELP = 3;

/**
    The constant representing a version display operation
*/
const Uint32 RepositoryUpgrade::_OPTION_TYPE_VERSION = 4;

/**
    The option character used to specify the old Repository path.
 */
const char   RepositoryUpgrade::_OPTION_OLD_REPOSITORY_PATH     = 'o';

/**
    The option character used to specify the new Repository path.
 */
const char   RepositoryUpgrade::_OPTION_NEW_REPOSITORY_PATH     = 'n';

/**
    The option character used to display help info.
*/
const char   RepositoryUpgrade::_OPTION_HELP         = 'h';

#ifdef NS_INTEROP
/**
    The constant representing that interop option has been specified
*/
const Uint32 RepositoryUpgrade::_OPTION_TYPE_INTEROP = 5;

/**
    The option character used to change root/PG_Interop to interop.
*/
const char   RepositoryUpgrade::_OPTION_INTEROP         = 'i';
#endif

static const char   LONG_HELP []  = "help";

/**
    The option character used to display version info.
*/
const char   RepositoryUpgrade::_OPTION_VERSION      = 'v';

static const char   LONG_VERSION []  = "version";

/**
    Display messages.
*/
static const char REQUIRED_ARGS_MISSING [] =
                "Required arguments missing.";

static const char REQUIRED_ARGS_MISSING_KEY [] =
                "Clients.repupgrade.RepositoryUpgrade.REQUIRED_ARGS_MISSING";

static const char REPOSITORY_UPGRADE_FAILURE []    =
                "Failed to upgrade repository. ";

static const char REPOSITORY_UPGRADE_FAILURE_KEY []    =
            "Clients.repupgrade.RepositoryUpgrade.REPOSITORY_UPGRADE_FAILURE";

static const char REPOSITORY_UPGRADE_UNKNOWN_ERROR []    =
                "Unknown error encountered during upgrade. ";

static const char REPOSITORY_UPGRADE_UNKNOWN_ERROR_KEY []    =
   "Clients.repupgrade.RepositoryUpgrade.REPOSITORY_UPGRADE_UNKNOWN_ERROR";

static const char REPOSITORY_DOES_NOT_EXIST [] =
                "The specified location $0 does not exist. ";

static const char REPOSITORY_DOES_NOT_EXIST_KEY [] =
          "Clients.repupgrade.RepositoryUpgrade.REPOSITORY_DOES_NOT_EXIST";

static const char NAMESPACE_CREATION_ERROR [] =
                "Error creating namespace $0.";

static const char NAMESPACE_CREATION_ERROR_KEY [] =
           "Clients.repupgrade.RepositoryUpgrade.NAMESPACE_CREATION_ERROR";

static const char CLASS_CREATION_ERROR [] =
                "Error creating class $0 in namespace $1.";

static const char CLASS_CREATION_ERROR_KEY [] =
                "Clients.repupgrade.RepositoryUpgrade.CLASS_CREATION_ERROR";

static const char INSTANCE_CREATION_ERROR [] =
                "Error creating instance in namespace $0. ";

static const char INSTANCE_CREATION_ERROR_KEY [] =
             "Clients.repupgrade.RepositoryUpgrade.INSTANCE_CREATION_ERROR";

static const char INSTANCE_DELETION_ERROR [] =
                "Error deleting instance in namespace $0. ";

static const char INSTANCE_DELETION_ERROR_KEY [] =
             "Clients.repupgrade.RepositoryUpgrade.INSTANCE_DELETION_ERROR";

static const char QUALIFIER_CREATION_ERROR [] =
                "Error creating qualifier $0 in namespace $1.";

static const char QUALIFIER_CREATION_ERROR_KEY [] =
            "Clients.repupgrade.RepositoryUpgrade.QUALIFIER_CREATION_ERROR";

static const char OLD_CLASS_RETRIEVAL_ERROR [] =
                "Error reading old repository class $0 in namespace $1.";

static const char OLD_CLASS_RETRIEVAL_ERROR_KEY [] =
            "Clients.repupgrade.RepositoryUpgrade.OLD_CLASS_RETRIEVAL_ERROR";

static const char OLD_DEPENDENT_CLASS_RETRIEVAL_ERROR [] =
  "Error reading old repository dependent class $0 in namespace $1.";

static const char OLD_DEPENDENT_CLASS_RETRIEVAL_ERROR_KEY [] =
  "Clients.repupgrade.RepositoryUpgrade.OLD_DEPENDENT_CLASS_RETRIEVAL_ERROR";

static const char NEW_CLASS_RETRIEVAL_ERROR [] =
                "Error reading new repository class $0 in namespace $1.";

static const char NEW_CLASS_RETRIEVAL_ERROR_KEY [] =
           "Clients.repupgrade.RepositoryUpgrade.NEW_CLASS_RETRIEVAL_ERROR";

static const char CLASS_XML_OUTPUT_FILE [] =
              "CIM/XML request for this class has been logged to file $0 ";

static const char CLASS_XML_OUTPUT_FILE_KEY [] =
              "Clients.repupgrade.RepositoryUpgrade.CLASS_XML_OUTPUT_FILE";

static const char INSTANCE_XML_OUTPUT_FILE [] =
             "CIM/XML request for this instance has been logged to file $0.";

static const char INSTANCE_XML_OUTPUT_FILE_KEY [] =
          "Clients.repupgrade.RepositoryUpgrade.INSTANCE_XML_OUTPUT_FILE";

static const char QUALIFIER_XML_OUTPUT_FILE [] =
            "CIM/XML request for this qualifier has been logged to file $0.";

static const char QUALIFIER_XML_OUTPUT_FILE_KEY [] =
          "Clients.repupgrade.RepositoryUpgrade.QUALIFIER_XML_OUTPUT_FILE";

static const char HIGHER_VERSION_OLD_CLASS [] =
"Warning: The old repository contains a class $0 in namespace $1 that has"
    " a higher version number than the new repository class."
    " This class may have to be manually imported.";


static const char HIGHER_VERSION_OLD_CLASS_KEY [] =
         "Clients.repupgrade.RepositoryUpgrade.HIGHER_VERSION_OLD_CLASS";

const String RepositoryUpgrade::_FILE_EXTENSION
                                              = ".xml";

/**
    Constant representing the name of the Version Qualifier.
 */
const String   RepositoryUpgrade::_VERSION_QUALIFIER_NAME = "VERSION";

#ifdef ENABLE_MODULE_PROCESSING
/**
   Defines that the Special Processing Module is interested in
   processing classes.
*/
const char* RepositoryUpgrade::_CLASS_ONLY = "c";

/**
   Defines that the Special Processing Module is interested in
   processing instances.
*/
const char* RepositoryUpgrade::_INSTANCE_ONLY = "i";

/**
   Defines that the Special Processing Module is interested in
   processing qualifiers.
*/
const char* RepositoryUpgrade::_QUALIFIER_ONLY = "q";

/**
   Defines that the Special Processing Module is interested in
   processing all types (includes class, instance and qualifier).
*/
const char* RepositoryUpgrade::_ALL = "a";
#endif

//
// Make the repository paths fixed for HPUX, if PEGASUS_USE_RELEASE_DIRS is set.
// Also defines the directory path to store CIMXML file for a failed request.
//
#ifdef PEGASUS_USE_RELEASE_DIRS
# define REPUPGRADE_USE_RELEASE_DIRS true
# ifdef PEGASUS_OVERRIDE_DEFAULT_RELEASE_DIRS
    const String OLD_REPOSITORY_PATH = PEGASUS_PREV_REPOSITORY_DIR;
    const String NEW_REPOSITORY_PATH = PEGASUS_REPOSITORY_DIR;
    const String RepositoryUpgrade::_LOG_PATH  = PEGASUS_LOG_DIR"/upgrade";
# elif defined(PEGASUS_OS_HPUX)
    const String OLD_REPOSITORY_PATH = "/var/opt/wbem/prev_repository";
    const String NEW_REPOSITORY_PATH = "/var/opt/wbem/repository";
    const String RepositoryUpgrade::_LOG_PATH  = "/var/opt/wbem/upgrade";
# elif defined(PEGASUS_OS_VMS)
    const String OLD_REPOSITORY_PATH = "/wbem_var/opt/wbem/prev_repository";
    const String NEW_REPOSITORY_PATH = "/wbem_var/opt/wbem/repository";
    const String RepositoryUpgrade::_LOG_PATH  = "/wbem_var/opt/wbem/upgrade";
# elif defined(PEGASUS_OS_LINUX)
    const String OLD_REPOSITORY_PATH = "/var/opt/tog-pegasus/prev_repository";
    const String NEW_REPOSITORY_PATH = "/var/opt/tog-pegasus/repository";
    const String RepositoryUpgrade::_LOG_PATH  =
        "/var/opt/tog-pegasus/log/upgrade";
# else
#   undef REPUPGRADE_USE_RELEASE_DIRS
    const String RepositoryUpgrade::_LOG_PATH = "./";
# endif
#else
    const String RepositoryUpgrade::_LOG_PATH
                                              = "./";
#endif

RepositoryUpgrade::RepositoryUpgrade ()
           : MessageQueue(PEGASUS_QUEUENAME_INTERNALCLIENT), _requestEncoder(0)
{
    //
    // Initialize data members.
    //
    _authenticator.clear();
    _authenticator.setAuthType(ClientAuthenticator::NONE);

    //
    // Create request encoder:
    //
    _requestEncoder = new CIMOperationRequestEncoder(
        this, "localhost", &_authenticator, 0);

    instanceCount=0;
    qualifierCount=0;
    _modulesInitialized=false;

    //
    // Get environment variable PEGASUS_HOME
    //
    const char* tmp = getenv("PEGASUS_HOME");

    if (tmp)
    {
        _pegasusHome = tmp;
    }

    FileSystem::translateSlashes(_pegasusHome);

#ifdef REPUPGRADE_DEBUG
   cout << "Pegasus HOME : " << _pegasusHome << endl;
#endif

    //
    // build Usage string.
    //
    _usage = String (_USAGE);
    _usage.append (COMMAND_NAME);

#if !(defined(REPUPGRADE_USE_RELEASE_DIRS))
    _usage.append (" -").append (_OPTION_OLD_REPOSITORY_PATH);
    _usage.append (" old_repository_path");
    _usage.append (" -").append (_OPTION_NEW_REPOSITORY_PATH);
    _usage.append (" new_repository_path");
#ifdef NS_INTEROP
    _usage.append (" [-").append (_OPTION_INTEROP).append("]");
#endif
#endif

    _usage.append("\n");

    //
    // Version options
    //
    _usage.append("       ").append (COMMAND_NAME);
    _usage.append (" -").append (_OPTION_VERSION)
         .append(" |");
    _usage.append (" --").append (LONG_VERSION)
         .append("\n");

    //
    // Help option
    //
    _usage.append("       ").append (COMMAND_NAME);
    _usage.append (" -").append (_OPTION_HELP)
         .append(" |");
    _usage.append (" --").append (LONG_HELP)
         .append("\n");

    //
    // Options description
    //
    _usage.append("Options : \n");
#if !(defined(REPUPGRADE_USE_RELEASE_DIRS))
    _usage.append("    -o              ");
    _usage.append("- Specify the fully qualified path of the old Repository\n");

    _usage.append("    -n              ");
    _usage.append("- Specify the fully qualified path of the new Repository\n");

#ifdef NS_INTEROP
    _usage.append("    -i              - Change root/PG_InterOp to interop\n");
#endif
#endif

    _usage.append("    -h, --help      - Display this help message\n");
    _usage.append("    -v, --version   - Display CIM Server version number\n");

    //l10n localize usage
#if (defined(REPUPGRADE_USE_RELEASE_DIRS))
    MessageLoaderParms menuparms(
        "Clients.repupgrade.RepositoryUpgradeRelease.MENU.STANDARD", _usage);
# else

    MessageLoaderParms menuparms(
        "Clients.repupgrade.RepositoryUpgrade.MENU.STANDARD", _usage);
#endif

    menuparms.msg_src_path = MSG_PATH;
    _usage = MessageLoader::getMessage(menuparms);

    setUsage (_usage);

   _oldRepositoryPathSet = false;
   _newRepositoryPathSet = false;

   //
   // If the PEGASUS_USE_RELEASE_DIRS is set make the old and new
   // repository paths fixed.
   //
#if (defined(REPUPGRADE_USE_RELEASE_DIRS))
       _oldRepositoryPath = OLD_REPOSITORY_PATH;
       _newRepositoryPath = NEW_REPOSITORY_PATH;
       _oldRepositoryPathSet = true;
       _newRepositoryPathSet = true;
#endif

   _oldRepository = 0;
   _newRepository = 0;

    // Construct the ignore class list.
    _interopIgnoreClasses.append ( "PG_IndicationFilter" );
    _interopIgnoreClasses.append ( "PG_IndicationHandler" );
    _interopIgnoreClasses.append ( "PG_IndicationHandlerCIMXML" );
    _interopIgnoreClasses.append ( "PG_IndicationHandlerSNMPMapper" );
    _interopIgnoreClasses.append ( "PG_IndicationSubscription" );

}

RepositoryUpgrade::~RepositoryUpgrade ()
{
    delete _requestEncoder;
    delete _oldRepository;
    delete _newRepository;

#ifdef ENABLE_MODULE_PROCESSING
    if (_modulesInitialized)
    {
        _cleanupSSPModule();
    }
#endif
}

/**
    Parses the command line, validates the options, and sets instance
    variables based on the option arguments.

    @param   argc  the number of command line arguments

    @param   argv  the string vector of command line arguments

    @exception  CommandFormatException  if an error is encountered in parsing
                                        the command line
 */
void RepositoryUpgrade::setCommand (Uint32 argc, char* argv [])
{
    Uint32         i               = 0;
    Uint32         c               = 0;
    String         optString       = String ();
    getoopt        getOpts;

    //
    //  Construct optString
    //
#if !(defined(REPUPGRADE_USE_RELEASE_DIRS))
    optString.append (_OPTION_OLD_REPOSITORY_PATH);
    optString.append (getoopt::GETOPT_ARGUMENT_DESIGNATOR);
    optString.append (_OPTION_NEW_REPOSITORY_PATH);
    optString.append (getoopt::GETOPT_ARGUMENT_DESIGNATOR);
#ifdef NS_INTEROP
    optString.append (_OPTION_INTEROP);
    optString.append (getoopt::NOARG);
#endif
#endif

    optString.append (_OPTION_HELP);
    optString.append (getoopt::NOARG);
    optString.append (_OPTION_VERSION);
    optString.append (getoopt::NOARG);

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
    _optionType = _OPTION_TYPE_UNINITIALIZED;

    //
    //  Get options and arguments from the command line
    //
    for (i =  getOpts.first (); i <  getOpts.last (); i++)
    {
        if (getOpts [i].getType () == Optarg::LONGFLAG)
        {
            if (getOpts [i].getopt () == LONG_HELP)
            {
                if (_optionType != _OPTION_TYPE_UNINITIALIZED)
                {
                    //
                    // More than one operation option was found
                    //
                    throw UnexpectedOptionException(String(LONG_HELP));
                }

               _optionType = _OPTION_TYPE_HELP;
            }
            else if (getOpts [i].getopt () == LONG_VERSION)
            {
                if (_optionType != _OPTION_TYPE_UNINITIALIZED)
                {
                    //
                    // More than one operation option was found
                    //
                    throw UnexpectedOptionException(String(LONG_VERSION));
                }

               _optionType = _OPTION_TYPE_VERSION;
            }
        }
        else if (getOpts [i].getType () == Optarg::REGULAR)
        {
            //
            //  The repupgrade command has no non-option argument options
            //
            throw UnexpectedArgumentException(getOpts[i].Value());
        }
        else /* getOpts [i].getType () == FLAG */
        {
            c = getOpts [i].getopt () [0];

            switch (c)
            {
#if !(defined(REPUPGRADE_USE_RELEASE_DIRS))
                case _OPTION_OLD_REPOSITORY_PATH:
                {
                    if (getOpts.isSet (_OPTION_OLD_REPOSITORY_PATH) > 1)
                    {
                        //
                        // More than one old repository path option was found
                        //
                        throw DuplicateOptionException(
                            _OPTION_OLD_REPOSITORY_PATH);
                    }

                    if (_optionType != _OPTION_TYPE_UNINITIALIZED &&
                        _optionType != _OPTION_TYPE_NEW_REPOSITORY_PATH)
                    {
                        //
                        // More than one operation option was found
                        //
                        throw UnexpectedOptionException(
                            _OPTION_OLD_REPOSITORY_PATH);
                    }

                    _optionType = _OPTION_TYPE_OLD_REPOSITORY_PATH;
                    _oldRepositoryPath = getOpts [i].Value ();
                    _oldRepositoryPathSet = true;
                    break;
                }
                case _OPTION_NEW_REPOSITORY_PATH:
                {

                    if (getOpts.isSet (_OPTION_NEW_REPOSITORY_PATH) > 1)
                    {
                        //
                        // More than one new repository option was found
                        //
                        throw DuplicateOptionException(
                            _OPTION_NEW_REPOSITORY_PATH);
                    }

                    if (_optionType != _OPTION_TYPE_UNINITIALIZED &&
                        _optionType != _OPTION_TYPE_OLD_REPOSITORY_PATH)
                    {
                        //
                        // More than one operation option was found
                        //
                        throw UnexpectedOptionException(
                            _OPTION_NEW_REPOSITORY_PATH);
                    }

                    _optionType = _OPTION_TYPE_NEW_REPOSITORY_PATH;
                    _newRepositoryPath = getOpts [i].Value ();
                    _newRepositoryPathSet = true;
                    break;
                }
#ifdef NS_INTEROP
                case _OPTION_INTEROP:
                {
                    if (getOpts.isSet (_OPTION_INTEROP) > 1)
                    {
                        //
                        // More than one version option was found
                        //
                        throw DuplicateOptionException(_OPTION_INTEROP);
                    }

                    _optionType = _OPTION_TYPE_INTEROP;
                    _optionInterop = true;
                    break;
                }
#endif
#endif
                case _OPTION_HELP:
                {
                    if (getOpts.isSet (_OPTION_HELP) > 1)
                    {
                        //
                        // More than one help option was found
                        //
                        throw DuplicateOptionException(_OPTION_HELP);
                    }

                    if (_optionType != _OPTION_TYPE_UNINITIALIZED)
                    {
                        //
                        // More than one operation option was found
                        //
                        throw UnexpectedOptionException(_OPTION_HELP);
                    }

                    _optionType = _OPTION_TYPE_HELP;
                    break;
                }

                case _OPTION_VERSION:
                {
                    if (getOpts.isSet (_OPTION_VERSION) > 1)
                    {
                        //
                        // More than one version option was found
                        //
                        throw DuplicateOptionException(_OPTION_VERSION);
                    }

                    if (_optionType != _OPTION_TYPE_UNINITIALIZED)
                    {
                        //
                        // More than one operation option was found
                        //
                        throw UnexpectedOptionException(_OPTION_VERSION);
                    }

                    _optionType = _OPTION_TYPE_VERSION;
                    break;
                }
                default:
                {
                    //
                    //  This path should not be hit
                    //
                    break;
                }
            }
        }
    }

    // Check if an operation type was not specified.
#if !(defined(REPUPGRADE_USE_RELEASE_DIRS))
    if ( _optionType == _OPTION_TYPE_UNINITIALIZED )
    {
        //
        // No operation type was specified
        // Show the usage
        //
        throw CommandFormatException(localizeMessage(
            MSG_PATH, REQUIRED_ARGS_MISSING_KEY, REQUIRED_ARGS_MISSING));
    }
#endif
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
Uint32 RepositoryUpgrade::execute (
    ostream& outPrintWriter,
    ostream& errPrintWriter)
{
    //
    // Options HELP and VERSION
    //
    if (_optionType == _OPTION_TYPE_HELP)
    {
        outPrintWriter << _usage << endl;
        return (RC_SUCCESS);
    }
    else if (_optionType == _OPTION_TYPE_VERSION)
    {
        outPrintWriter << "Version " << PEGASUS_PRODUCT_VERSION << endl;
        return (RC_SUCCESS);
    }

    //
    // Check if the old and new repository paths exist.
    //
    if ( _oldRepositoryPathSet && !FileSystem::exists (_oldRepositoryPath))
    {
        cerr << localizeMessage ( MSG_PATH,
                                  REPOSITORY_DOES_NOT_EXIST_KEY,
                                  REPOSITORY_DOES_NOT_EXIST,
                                  _oldRepositoryPath ) << endl;
        return 1;
    }

    if ( _newRepositoryPathSet && !FileSystem::exists (_newRepositoryPath))
    {
        cerr << localizeMessage (MSG_PATH,
                                 REPOSITORY_DOES_NOT_EXIST_KEY,
                                 REPOSITORY_DOES_NOT_EXIST,
                                 _newRepositoryPath ) << endl;
        return 1;
    }

    try
    {
#ifdef ENABLE_MODULE_PROCESSING
        //
        // Load the Special Processing Modules.
        //
        _initSSPModule();
#endif

        _modulesInitialized = true;

        //
        // Upgrade the Repository.
        //
        upgradeRepository();
    }
    catch (RepositoryUpgradeException& rue)
    {
        errPrintWriter << rue.getMessage() << endl;
        return 1;
    }
    catch (Exception &e)
    {
        errPrintWriter << e.getMessage() << endl;
        return 1;
    }

    return 0;
}

Boolean RepositoryUpgrade::_updateFilterHandlerReference(
    CIMInstance& instance,
    const CIMName& propertyName)
{
    Boolean changed = false;
    CIMObjectPath objPath = instance.getPath();
    CIMObjectPath ref;
    String systemName;
    Uint32 pos = instance.findProperty(propertyName);
    PEGASUS_ASSERT(pos != PEG_NOT_FOUND);

    instance.getProperty(pos).getValue().get(ref);
    Array<CIMKeyBinding> keyBindings = ref.getKeyBindings();
    Uint32 nk = keyBindings.size();
    for (Uint32 j = 0; j < nk ; j++)
    {
        if (keyBindings[j].getName().equal(_PROPERTY_SYSTEMNAME))
        {
            if (keyBindings[j].getValue() != systemName)
            {
                changed = true;
                keyBindings[j].setValue(systemName);
            }
            break;
        }
    }
    // Remove hostname from Filter or Handler reference if set
    if (ref.getHost().size() != 0)
    {
        ref.setHost(String());
        changed = true;
    }

    if (changed)
    {
        ref.setKeyBindings(keyBindings);
        CIMProperty currentProp = instance.getProperty(pos);
        currentProp.setValue(ref);
        instance.removeProperty(pos);
        instance.addProperty(currentProp);
    }
    return changed;
}

void RepositoryUpgrade::_updateSubscriptionInstancesInRepository(
    const CIMNamespaceName& nameSpace,
    const CIMName& className)
{
    Array<CIMInstance> instances;
    String systemName = System::getFullyQualifiedHostName();
    instances = _newRepository->enumerateInstancesForClass(
        nameSpace,
        className);
    for (Uint32 i = 0, n = instances.size(); i < n; i++)
    {
        CIMObjectPath objPath = instances[i].getPath();
        CIMObjectPath objPathMod;
        Boolean filterUpdated = _updateFilterHandlerReference(
            instances[i],
            PEGASUS_PROPERTYNAME_FILTER);
        Boolean handlerUpdated = _updateFilterHandlerReference(
            instances[i],
            PEGASUS_PROPERTYNAME_HANDLER);

        if (filterUpdated || handlerUpdated)
        {
            try
            {
                _newRepository->deleteInstance(nameSpace, objPath);
            }
            catch (CIMException &e)
            {
                _logDeleteInstanceError(
                    nameSpace,
                    objPath,
                    e.getMessage());
            }
            try
            {
                objPathMod = _newRepository->createInstance(
                    nameSpace,
                    instances[i]);
            }
            catch (CIMException &e)
            {
                _logCreateInstanceError(
                    nameSpace,
                    instances[i],
                    e.getMessage());
            }
#ifdef REPUPGRADE_DEBUG
            cout << "Updated handler/filter references in subscription instance"
                << " from " << (const char*)objPath.toString().getCString()
                << " to " << (const char*)objPathMod.toString().getCString()
                << endl;
#endif
        }
    }
}

void RepositoryUpgrade::_updateSystemNameKeyPropertyOfInstancesForClass(
    const CIMNamespaceName& nameSpace,
    const CIMName& className)
{
    Array<CIMInstance> instances;
    String systemName;

    instances = _newRepository->enumerateInstancesForClass(
        nameSpace,
        className);

    for (Uint32 i = 0, n = instances.size(); i < n; i++)
    {
        Boolean changed = false;
        CIMObjectPath objPath = instances[i].getPath();
        Uint32 pos = instances[i].findProperty(_PROPERTY_SYSTEMNAME);
        PEGASUS_ASSERT(pos != PEG_NOT_FOUND);
        String sysNameProp;
        CIMProperty currentProp = instances[i].getProperty(pos);
        currentProp.getValue().get(sysNameProp);

        if (sysNameProp != systemName)
        {
            changed = true;
            currentProp.setValue(systemName);
            instances[i].removeProperty(pos);
            instances[i].addProperty(currentProp);
        }

        if (changed)
        {
            CIMObjectPath objPathMod;
            try
            {
                _newRepository->deleteInstance(nameSpace, objPath);
            }
            catch (CIMException &e)
            {
                _logDeleteInstanceError(
                    nameSpace,
                    objPath,
                    e.getMessage());
            }
            try
            {
                objPathMod = _newRepository->createInstance(
                    nameSpace,
                    instances[i]);
            }
            catch (CIMException &e)
            {
                _logCreateInstanceError(
                    nameSpace,
                    instances[i],
                    e.getMessage());
            }
#ifdef REPUPGRADE_DEBUG
            cout << "Updated the SystemName key property in Filter/Handler"
                << "/ObjectManager instance"
                << " from " << (const char*)objPath.toString().getCString()
                << " to " << (const char*)objPathMod.toString().getCString()
                << endl;
#endif
        }
    }
}

void RepositoryUpgrade::_updateSystemNameKeyProperty()
{
    //
    //  Get list of namespaces in repository
    //
    Array <CIMNamespaceName> nameSpaceNames;
    nameSpaceNames = _newRepository->enumerateNameSpaces();

    Array <CIMName> filterHandlerClassNameArray;

    filterHandlerClassNameArray.append(PEGASUS_CLASSNAME_INDHANDLER);
    filterHandlerClassNameArray.append(PEGASUS_CLASSNAME_INDHANDLER_CIMXML);
#ifdef  PEGASUS_ENABLE_PROTOCOL_WSMAN
    filterHandlerClassNameArray.append(PEGASUS_CLASSNAME_INDHANDLER_WSMAN);
#endif
    filterHandlerClassNameArray.append(PEGASUS_CLASSNAME_LSTNRDST_CIMXML);
#if defined(PEGASUS_ENABLE_SYSTEM_LOG_HANDLER)
    filterHandlerClassNameArray.append(PEGASUS_CLASSNAME_LSTNRDST_SYSTEM_LOG);
#endif
#if defined(PEGASUS_ENABLE_EMAIL_HANDLER)
    filterHandlerClassNameArray.append(PEGASUS_CLASSNAME_LSTNRDST_EMAIL);
#endif
    filterHandlerClassNameArray.append(PEGASUS_CLASSNAME_LSTNRDST_FILE);

    filterHandlerClassNameArray.append(PEGASUS_CLASSNAME_INDHANDLER_SNMP);
    filterHandlerClassNameArray.append(PEGASUS_CLASSNAME_INDFILTER);

    Array<CIMName> subscriptionClassNameArray;
    subscriptionClassNameArray.append(PEGASUS_CLASSNAME_INDSUBSCRIPTION);
    subscriptionClassNameArray.append(
        PEGASUS_CLASSNAME_FORMATTEDINDSUBSCRIPTION);

    for (Uint32 i = 0, ni = nameSpaceNames.size(); i < ni; i++)
    {
        for (Uint32 j = 0, n = filterHandlerClassNameArray.size(); j < n ; j++)
        {
            try
            {
                _updateSystemNameKeyPropertyOfInstancesForClass(
                    nameSpaceNames[i],
                    filterHandlerClassNameArray[j]);
            }
            catch (const CIMException& e)
            {
                //
                //  Some namespaces may not include the filter/handler class
                //
                if (e.getCode () != CIM_ERR_INVALID_CLASS)
                {
#ifdef REPUPGRADE_DEBUG
                    cout << "Exception caught in attempting to update filter"
                        << "/handler instances in new repository "
                        << (const char*)e.getMessage().getCString()
                        << endl;
#endif

                    throw;
                }
            }
        }

        // Now update subscriptions.
        for (Uint32 j = 0, n = subscriptionClassNameArray.size(); j < n ; j++)
        {
            try
            {
                _updateSubscriptionInstancesInRepository(
                    nameSpaceNames[i],
                    subscriptionClassNameArray[j]);
            }
            catch (const CIMException& e)
            {
                //
                //  Some namespaces may not include the subscription class
                //  In that case, just return no subscriptions
                //  Any other exception is an error
                //
                if (e.getCode () != CIM_ERR_INVALID_CLASS)
                {
#ifdef REPUPGRADE_DEBUG
                    cout << "Exception caught in attempting to update "
                        << "subscription instances in new repository "
                        << (const char*)e.getMessage().getCString()
                        << endl;
#endif
                    throw;
                }
            }
        }
    }
}

void RepositoryUpgrade::upgradeRepository()
{
    Array<CIMNamespaceName>         oldNamespaces;
    Array<CIMNamespaceName>         newNamespaces;
    Array<CIMNamespaceName>         missingNamespaces;
    CIMName             className;

    //
    // Create the old and new Repository instances.
    //
    _oldRepository = new CIMRepository (_oldRepositoryPath);
    _newRepository = new CIMRepository (_newRepositoryPath);

    //
    // Get Namespace information for old Repository.
    //
    oldNamespaces = _oldRepository->enumerateNameSpaces();
    //
    // Get Namespace information for new Repository.
    //
    newNamespaces = _newRepository->enumerateNameSpaces();

    //
    // Check if the namespaces in the old repository exist in the new.
    // If they don't exist, create them and populate.
    //
    missingNamespaces = _compareNamespaces( oldNamespaces, newNamespaces );

    if ( missingNamespaces.size() > 0 )
    {
        //
        // Need to add the missing namespaces (includes classes &
        // qualifiers) to the new repository.
        //
        _addNamespaces(missingNamespaces);

        //
        // Since the missing namespaces have been processed, remove
        // them from the old Namespaces list. That way we will process only
        // the pre-existing namespaces that existed in both the repositories
        // when we look for qualifiers and classes to be added.
        //
        for (Uint32 i=0; i < missingNamespaces.size(); i++)
        {
            // find the namespace and remove it from list of old namespaces.
            Uint32 n = oldNamespaces.size();

            for (Uint32 j = 0; j < n; j++)
            {
                if (oldNamespaces[j] == missingNamespaces[i])
                {
                    oldNamespaces.remove(j);
                    break;
                }
            }
        }
    }

    //
    // Get the class information for pre-existing namespaces
    // in the old and new repositories.
    //

    Uint32 n = oldNamespaces.size();

    Array<CIMName>          oldClassNames;
    Array<CIMName>          newClassNames;

    for ( Uint32 i=0; i < n;  i++)
    {

        oldClassNames.clear();
        newClassNames.clear();

        // Add qualifiers.
        _addQualifiers (oldNamespaces[i]);

#ifdef REPUPGRADE_DEBUG
        cout << "Now processing namespace : "
            << (const char*)oldNamespaces[i].getString().getCString()
            << " i=" << i << endl;
#endif

        //
        // Gather class information for each namespace.
        //
        oldClassNames = _oldRepository->enumerateClassNames(
                           oldNamespaces[i],
                           className,
                           true);

        newClassNames = _newRepository->enumerateClassNames(
                                 oldNamespaces[i],
                                 className,
                                 true);

        //
        // Process the class list.
        //
        if ( oldClassNames.size() > 0 )
        {
            _processClasses(
            oldNamespaces[i],
            oldClassNames,
            newClassNames);
        }
    }

    // Create Instances.
#ifdef REPUPGRADE_DEBUG
    cout << "Checking for instances..." << endl;
#endif
    _addInstances ();
    _updateSystemNameKeyProperty();
    _removeInstances();

#ifdef NS_INTEROP
    // Change "root/PG_InterOp" to "interop" in the new repository
    if ( (_optionInterop == true ) && 
        (_oldRepository->nameSpaceExists(PEGASUS_NAMESPACE_PGINTEROP)))
    {
#ifdef REPUPGRADE_DEBUG
        cout << "Changing namespace name from " <<
             PEGASUS_NAMESPACE_PGINTEROP.getString() << " to " 
             << PEGASUS_NAMESPACENAME_INTEROP.getString() << endl;
#endif
        _newRepository->modifyNameSpaceName(PEGASUS_NAMESPACE_PGINTEROP,
            PEGASUS_NAMESPACENAME_INTEROP);
    }
#endif
}

Array<CIMNamespaceName> RepositoryUpgrade::_compareNamespaces(
                           const Array<CIMNamespaceName>& oldNamespaces,
                           const Array<CIMNamespaceName>& newNamespaces)
{
    Array<CIMNamespaceName>     namespaceNames;

    Uint32 n = oldNamespaces.size();
    for ( Uint32 i=0; i < n; i++ )
    {
        if (! Contains (newNamespaces,oldNamespaces[i]))
        {
            namespaceNames.append( oldNamespaces[i] );
        }
    }
    return namespaceNames;
}

void RepositoryUpgrade::_addQualifiers (const CIMNamespaceName &namespaceName)
{

    //
    // Retrieve qualifiers.
    //
    Array<CIMQualifierDecl> qualifiers =
                  _oldRepository->enumerateQualifiers(namespaceName);

    Uint32 n = qualifiers.size();

    for (Uint32 j=0 ; j<n ; j++)
    {
        try
        {
            // Check if the qualifier exists in the new repository.
            CIMQualifierDecl qual =
               _newRepository->getQualifier(namespaceName,
                               qualifiers[j].getName());

            if ( qual.getName() == qualifiers[j].getName() )
            {
                 // Qualifier already exists, do not create.
                 continue;
            }
        }
        catch (CIMException& ce)
        {
            //
            // Check if the error returned was that the qualifier does not
            // exist. If not propagate the error.
            //
            if (ce.getCode() != CIM_ERR_NOT_FOUND)
            {
                throw;
            }
        }

        //
        // Check if the qualifier has to be created. If true, use the
        // processed qualifier. If an SSP Module have chosen to
        // to ignore the qualifier then skip the qualifier creation.
        //
        CIMQualifierDecl processedQual = qualifiers[j].clone();

#ifdef ENABLE_MODULE_PROCESSING
        if (!_invokeModules (qualifiers[j], processedQual))
        {
#ifdef REPUPGRADE_DEBUG
            cerr << "Ignoring qualifier creation : "
               << (const char*)qualifiers[j].getName().getString().getCString()
               << endl;
#endif
            continue;
        }
#endif

        try
        {
#ifdef REPUPGRADE_DEBUG
        cout << "Now creating qualifier :"
            << (const char*)processedQual.getName().getString().getCString()
            << endl;
#endif
            _newRepository->setQualifier (namespaceName, processedQual);
#ifdef REPUPGRADE_DEBUG
        cout << "Qualifier created:"
            << (const char*)processedQual.getName().getString().getCString()
            << endl;
#endif

        }
        catch (Exception& e)
        {
            _logSetQualifierError (namespaceName,
                                           qualifiers[j],
                                           (e.getMessage()+". "));
        }
        catch (...)
        {
            _logSetQualifierError (namespaceName,
                                           qualifiers[j],
                                           String::EMPTY);
        }
    }
}

void RepositoryUpgrade::_addNamespaces(
                         const Array<CIMNamespaceName>& namespaces)
{

    Array<CIMName>          oldClassNames;
    Array<CIMName>          newClassNames;
    CIMName             className;

    Uint32 count = namespaces.size();
    for ( Uint32 i=0; i < count;  i++)
    {
#ifdef REPUPGRADE_DEBUG
        cout << "Now creating namespace : "
            << (const char*)namespaces[i].getString().getCString() << endl;
#endif
        try
        {
            _newRepository->createNameSpace(namespaces[i]);
        }
        catch (Exception& e)
        {
            String message = localizeMessage (MSG_PATH,
                               REPOSITORY_UPGRADE_FAILURE_KEY,
                               REPOSITORY_UPGRADE_FAILURE) +

                             e.getMessage() +

                             localizeMessage (MSG_PATH,
                               NAMESPACE_CREATION_ERROR_KEY,
                               NAMESPACE_CREATION_ERROR,
                               namespaces[i].getString());

            throw RepositoryUpgradeException(message);
        }
        catch (...)
        {
            String message = localizeMessage (MSG_PATH,
                               REPOSITORY_UPGRADE_FAILURE_KEY,
                               REPOSITORY_UPGRADE_FAILURE) +

                             localizeMessage (MSG_PATH,
                               NAMESPACE_CREATION_ERROR_KEY,
                               NAMESPACE_CREATION_ERROR,
                               namespaces[i].getString());

            throw RepositoryUpgradeException(message);
        }

        //
        // 1. Add qualifiers
        //

        _addQualifiers (namespaces[i]);

        //
        // 2. Add classes
        //

        // Get classnames from old repository.

        oldClassNames = _oldRepository->enumerateClassNames(
                           namespaces[i],
                           className,
                           true);

        if (oldClassNames.size() > 0)
        {
            _processNewClasses (namespaces[i],
                            oldClassNames,
                            newClassNames);
        }

        oldClassNames.clear();
    newClassNames.clear();

    }
}

void RepositoryUpgrade::_processClasses(
                      const CIMNamespaceName& namespaceName,
                      const Array<CIMName>&   oldClasses ,
                      Array<CIMName>&   newClasses)
{
    Array<CIMName>      missingClasses;
    Array<CIMName>      existingClasses;

    //
    // Separate the existing and missing classes.
    //

    Uint32 oldCount = oldClasses.size();

    for ( Uint32 oldclasses = 0; oldclasses < oldCount ; oldclasses++)
    {
#ifdef REPUPGRADE_DEBUG
        cout << "Checking for : "
            << (const char*)oldClasses[oldclasses].getString().getCString()
            << endl;
#endif
        if ( !Contains(newClasses, oldClasses[oldclasses]) )
        {

            /** This check is required to ignore PG_Ind* classes
                in the interop namespace. These classes were
                replaced with CIM_Ind* in CIM 2.7
            */
            if ( namespaceName == PEGASUS_NAMESPACENAME_INTEROP &&
                Contains( _interopIgnoreClasses, oldClasses[oldclasses] ) )
            {
#ifdef REPUPGRADE_DEBUG
             cout << "Now ignoring: "
                 << (const char*)oldClasses[oldclasses].getString().getCString()
                 << endl;
#endif
                continue;
            }

#ifdef REPUPGRADE_DEBUG
            cout << "Now appending to missing: "
                << (const char*)oldClasses[oldclasses].getString().getCString()
                << endl;
#endif
            missingClasses.append(oldClasses[oldclasses]);
        }
        else
        {
#ifdef REPUPGRADE_DEBUG
        cout << "Now appending to existing: "
            << (const char*)oldClasses[oldclasses].getString().getCString()
            << endl;
#endif
            existingClasses.append(oldClasses[oldclasses]);
        }
    }

    // Check if any missing classes existed and add them.
    if (missingClasses.size() > 0)
    {
        _processNewClasses (namespaceName, missingClasses, newClasses);
    }

    // Check for existing classes and process them.
    if (existingClasses.size() > 0)
    {
        _processExistingClasses (namespaceName, existingClasses);
    }

    newClasses.clear();
}

void RepositoryUpgrade::_processExistingClasses(
                      const CIMNamespaceName& namespaceName,
                      const Array<CIMName>&   existingClasses)
{
    Uint32          idx;
    Uint32          existingClassCount = existingClasses.size();

    for ( Uint32 i=0; i < existingClassCount; i++)
    {
        CIMClass        oldClass;
    CIMClass        newClass;
    String          oldVersion;
    String          newVersion;
    Boolean         oldVersionFound=false;
    Boolean         newVersionFound=false;

#ifdef REPUPGRADE_DEBUG
        cout << "In Namespace: "
            << (const char*)namespaceName.getString().getCString()
            << "Existing Classname: "
            << (const char*)existingClasses[i].getString().getCString()
            << endl;
#endif

        try
        {
            oldClass = _oldRepository->getClass(namespaceName,
                                            existingClasses[i],
                                            true,
                                            true,
                                            true);
        }
        catch (Exception& e)
        {
            String message = localizeMessage (MSG_PATH,
                               REPOSITORY_UPGRADE_FAILURE_KEY,
                               REPOSITORY_UPGRADE_FAILURE) +

                             e.getMessage() +

                             localizeMessage (MSG_PATH,
                               OLD_CLASS_RETRIEVAL_ERROR_KEY,
                               OLD_CLASS_RETRIEVAL_ERROR,
                               existingClasses[i].getString(),
                               namespaceName.getString());
            throw RepositoryUpgradeException(message);
        }
        catch (...)
        {
            String errMsg = localizeMessage ( MSG_PATH,
                                              REPOSITORY_UPGRADE_FAILURE_KEY,
                                              REPOSITORY_UPGRADE_FAILURE )
                            + localizeMessage ( MSG_PATH,
                                                OLD_CLASS_RETRIEVAL_ERROR_KEY,
                                                OLD_CLASS_RETRIEVAL_ERROR,
                                                existingClasses[i].getString(),
                                                namespaceName.getString());

            throw RepositoryUpgradeException (errMsg);
        }

        try
        {
            newClass = _newRepository->getClass(namespaceName,
                                            existingClasses[i],
                                            true,
                                            true,
                                            true);
        }
        catch (Exception& e)
        {
            String message = localizeMessage (MSG_PATH,
                               REPOSITORY_UPGRADE_FAILURE_KEY,
                               REPOSITORY_UPGRADE_FAILURE) +
                             e.getMessage() +
                             localizeMessage (MSG_PATH,
                               NEW_CLASS_RETRIEVAL_ERROR_KEY,
                               NEW_CLASS_RETRIEVAL_ERROR,
                               existingClasses[i].getString(),
                               namespaceName.getString());
            throw RepositoryUpgradeException(message);
        }
        catch (...)
        {
            String errMsg = localizeMessage ( MSG_PATH,
                                              REPOSITORY_UPGRADE_FAILURE_KEY,
                                              REPOSITORY_UPGRADE_FAILURE )
                            + localizeMessage ( MSG_PATH,
                                                NEW_CLASS_RETRIEVAL_ERROR_KEY,
                                                NEW_CLASS_RETRIEVAL_ERROR,
                                                existingClasses[i].getString(),
                                                namespaceName.getString());

            throw RepositoryUpgradeException (errMsg);
        }

        // Get the version qualifier from the old Class.
        idx = oldClass.findQualifier(_VERSION_QUALIFIER_NAME);
        if (idx != PEG_NOT_FOUND)
        {
            CIMQualifier rVer = oldClass.getQualifier(idx);
            CIMValue rVal = rVer.getValue();
            rVal.get(oldVersion);
            oldVersionFound = true;
        }

        // Get the version qualifier from the new Class.
        idx = newClass.findQualifier(_VERSION_QUALIFIER_NAME);
        if (idx != PEG_NOT_FOUND)
        {
            CIMQualifier rVer = newClass.getQualifier(idx);
            CIMValue rVal = rVer.getValue();
            rVal.get(newVersion);
            newVersionFound = true;
        }

        //
        // 1. If the class in old Repository contains a version number
        //    and the new Repository class does not have a version number then
        //    print a warning message
        // 2. If the class in old Repository contains a higher version number
        //    than the new Repository class then print a warning message
        //
        if ( oldVersionFound && !newVersionFound )
        {
            //
            // The old Repository has a higher version of the class.
            // Log a warning message and ignore the class.
            //
            cerr << localizeMessage (MSG_PATH,
                                     HIGHER_VERSION_OLD_CLASS_KEY,
                                     HIGHER_VERSION_OLD_CLASS,
                                     oldClass.getClassName().getString(),
                                     namespaceName.getString()) << endl;
        }
        else if ( oldVersionFound && newVersionFound )
        {
            // Compare the version
            if (_compareVersion (oldVersion, newVersion) == true)
            {
                //
                // The old Repository has a higher version of the class.
                // Log a warning message and ignore the class.
                //
                cerr << localizeMessage (MSG_PATH,
                                         HIGHER_VERSION_OLD_CLASS_KEY,
                                         HIGHER_VERSION_OLD_CLASS,
                                         oldClass.getClassName().getString(),
                                         namespaceName.getString()) << endl;

            }
        }
    }
}

//
// Return true if old repository class has a higher version, else return false.
//
Boolean RepositoryUpgrade::_compareVersion(const String& oldVersion,
                                           const String& newVersion)
{
    Sint32          iMinorOld = -1;
    Sint32          iMajorOld = -1;
    Sint32          iUpdateOld = -1;
    Sint32          iMinorNew = -1;
    Sint32          iMajorNew = -1;
    Sint32          iUpdateNew = -1;
    Boolean         retVal = false;

    retVal = VersionUtil::parseVersion(
        oldVersion,
        iMajorOld,
        iMinorOld,
        iUpdateOld);

    // cimmof compiler rejects invalid versions.
    // ATTN-SF-P3-20050209: Need to identify invalid version formats and assert
    // here.
    // PEGASUS_ASSERT(retVal != false);

    retVal = VersionUtil::parseVersion(
        newVersion,
        iMajorNew,
        iMinorNew,
        iUpdateNew);

    // cimmof compiler rejects invalid versions.
    // ATTN-SF-P3-20050209: Need to identify invalid version formats and assert
    // here.
    // PEGASUS_ASSERT(retVal != false);

#ifdef REPUPGRADE_DEBUG
    cout << "old Version : " << iMajorOld << "."
                             << iMinorOld << "."
                             << iUpdateOld << endl;
    cout << "new Version : " << iMajorNew << "."
                             << iMinorNew << "."
                             << iUpdateNew << endl;
#endif

    retVal = false;
    if ( iMajorOld > iMajorNew )
    {
        retVal = true;
    }
    else if ( iMajorOld == iMajorNew )
    {
        if ( iMinorOld > iMinorNew )
        {
            retVal = true;
        }
        else if ( iMinorOld == iMinorNew )
        {
            if ( iUpdateOld > iUpdateNew )
            {
                retVal = true;
            }
        }
    }
    return retVal;
}

void RepositoryUpgrade::_processNewClasses(
                          const CIMNamespaceName& namespaceName,
                          Array<CIMName>&   missingClasses,
                          Array<CIMName>&   currClasses)
{
    Boolean     allSuperClassesExist = true;
    Uint32      missingCount         = missingClasses.size();
    Uint32      superClassCount      = 0;
    Uint32      saveCount            = 0;
    Array<CIMName>      superClassList;

    while (missingCount > 0)
    {
        for ( Uint32 i=0; i < missingCount; i++)
        {
            allSuperClassesExist    = true;

            //
            // Check if the class already exists in the new repository.
            //
            if (!Contains (currClasses,missingClasses[i]))
            {
                //
                // Check if this class has Super Classes.
                //
                _oldRepository->getSuperClassNames(
                    namespaceName,
                    missingClasses[i],
                    superClassList);

                superClassCount = superClassList.size();
                saveCount       = superClassCount;

                while ( superClassCount > 0 && allSuperClassesExist)
                {
                    //
                    // Check if all the super classes exist in the
                    // new repository.
                    //
                    if (!Contains(currClasses,
                                     superClassList[superClassCount-1]))
                    {
                        allSuperClassesExist = false;
                    }

                    --superClassCount;
                }

                //
                // If all the super classes existed or if the class did not have
                // any super classes then try to add the class to the
                // new Repository.
                //
                if (allSuperClassesExist || (saveCount==0))
                {
                    Uint32 retCode =
                          _addClassToRepository (namespaceName,
                                             missingClasses[i],
                                             currClasses);

                    //
                    // Check if the class was added successfully.
                    //
                    if (retCode == 0)
                    {
                         currClasses.append(missingClasses[i]);
                         missingClasses.remove(i);
                    }

                }
            }

            missingCount = missingClasses.size();
            superClassList.clear();

        }
    }
}

Uint32 RepositoryUpgrade::_addClassToRepository (
                      const CIMNamespaceName& namespaceName,
                      const CIMName&          className,
                      const Array<CIMName>    existingClasses)
{
    Uint32 retCode = 0;

    CIMClass                oldClass;

    // Get the missing class info from the old repository.
    try
    {
        oldClass = _oldRepository->getClass(
                          namespaceName,
                          className,
                          true,
                          true,
                          true);
    }
    catch (Exception& e)
    {
        String message = localizeMessage (MSG_PATH,
                           REPOSITORY_UPGRADE_FAILURE_KEY,
                           REPOSITORY_UPGRADE_FAILURE) +

                         e.getMessage() +

                         localizeMessage (MSG_PATH,
                           OLD_CLASS_RETRIEVAL_ERROR_KEY,
                           OLD_CLASS_RETRIEVAL_ERROR,
                           className.getString(),
                           namespaceName.getString());
        throw RepositoryUpgradeException(message);
    }
    catch (...)
    {
        String message = localizeMessage ( MSG_PATH,
                                           REPOSITORY_UPGRADE_FAILURE_KEY,
                                           REPOSITORY_UPGRADE_FAILURE ) +
                         localizeMessage ( MSG_PATH,
                                           OLD_CLASS_RETRIEVAL_ERROR_KEY,
                                           OLD_CLASS_RETRIEVAL_ERROR,
                                           className.getString(),
                                           namespaceName.getString());
        throw RepositoryUpgradeException (message);
    }

    //
    // Invoke the Special Processing modules.
    //
    //
    // Check if the class must be created. If true, use the
    // processed class. If the SSP Modules have chosen to
    // to ignore the class then skip the class creation.
    //
    CIMClass processedClass = oldClass.clone();

#ifdef ENABLE_MODULE_PROCESSING
    if (!_invokeModules (oldClass, processedClass))
    {
#ifdef REPUPGRADE_DEBUG
        cerr << "Ignoring class creation : "
        << oldClass.getClassName() << endl;
#endif
    }
    else
    {
#endif
        try
        {
#ifdef REPUPGRADE_DEBUG
            cout << "Creating class : " << className.getString() << endl;
#endif
            _newRepository->createClass (namespaceName, processedClass);
#ifdef REPUPGRADE_DEBUG
            cout << "Class created : " << className.getString() << endl;
#endif

        }
        catch(BadQualifierType& bqe)
        {
            if(bqe.getQualifierName() == "EmbeddedInstance" )
            {
                CIMName dependentClassName(bqe.getClassName());
                if (! Contains( existingClasses, dependentClassName))
                {
                    oldClass = _checkIfDependentClassExists(namespaceName,
                        className,
                        dependentClassName);

                    retCode = 1;
                }
                else
                {
                    _logCreateClassError (namespaceName,
                                          oldClass,
                                          (bqe.getMessage()+". "));
                }
            }
            //BadQualifierType exception occured not because of
            //embedded instance repupgrade
            else
            {
                 _logCreateClassError (namespaceName,
                                       oldClass,
                                       (bqe.getMessage()+". "));
            }
        }
        catch (CIMException& ce)
        {
            if (ce.getCode() == CIM_ERR_INVALID_PARAMETER)
            {
                CIMName dependentClassName(ce.getMessage());

                //
                // Check if the error was due to a non-existent class, if so
                // will try to create this class later. This error could be
                // because a dependent class of an Association class
                // has not yet been created.
                //
                if (! Contains( existingClasses, dependentClassName))
                {
                    oldClass = _checkIfDependentClassExists(namespaceName,
                        className,
                        dependentClassName);

                    retCode = 1;
                }
                else
                {
                    _logCreateClassError (namespaceName,
                                          oldClass,
                                          (ce.getMessage()+". "));
                }
            }
            else
            {
                _logCreateClassError (namespaceName,
                                      oldClass,
                                      (ce.getMessage()+". "));
            }
        }
        catch (Exception& e)
        {
           _logCreateClassError (namespaceName,
                                 oldClass,
                                 (e.getMessage()+". "));
        }
        catch (...)
        {
           _logCreateClassError (namespaceName,
                                 oldClass,
                                 String::EMPTY);
        }
#ifdef ENABLE_MODULE_PROCESSING
    }
#endif

    return retCode;
}

CIMClass RepositoryUpgrade::_checkIfDependentClassExists(
                      const CIMNamespaceName& namespaceName,
                      const CIMName&          className,
                      const CIMName&          dependentClassName)
{
    // Check if the class that we depend on exists in
    // the old repository.
    // If it doesn't exist, error out.
    try
    {
        CIMClass oldClass = _oldRepository->getClass(
            namespaceName,
            dependentClassName,
            true,
            true,
            true);

#ifdef REPUPGRADE_DEBUG
        cout << "Adding to retry list Class name : "
            << (const char *) className.getString().getCString()<< endl;
#endif
        return oldClass;

    }
    //Embedded instance is not in old repository
    catch (const CIMException& c)
    {
        // We have an exception case here.
        String message = localizeMessage (MSG_PATH,
                             REPOSITORY_UPGRADE_FAILURE_KEY,
                             REPOSITORY_UPGRADE_FAILURE) + " " +

                         localizeMessage ( MSG_PATH,
                             CLASS_CREATION_ERROR_KEY,
                             CLASS_CREATION_ERROR,
                             className.getString(),
                             namespaceName.getString()) + " " +

                         localizeMessage (MSG_PATH,
                             OLD_DEPENDENT_CLASS_RETRIEVAL_ERROR_KEY,
                             OLD_DEPENDENT_CLASS_RETRIEVAL_ERROR,
                             dependentClassName.getString(),
                             namespaceName.getString());

        throw RepositoryUpgradeException(message);
    }
}


void RepositoryUpgrade::_addInstances(void)
{
    Array<CIMNamespaceName>                  oldNamespaces;

    oldNamespaces = _oldRepository->enumerateNameSpaces();

    if (oldNamespaces.size() > 0)
    {
        CIMName         className;

        for ( Uint32 i = 0; i < oldNamespaces.size(); i++)
        {
            //
            // Get the list of class names.
            //
            Array<CIMName>                  oldClassNames;

            //
            // Gather class information for each namespace.
            //
            oldClassNames = _oldRepository->enumerateClassNames(
                       oldNamespaces[i],
                       className,
                       true);

            //
            // Enumerate the instances for each class.
            //
            if (oldClassNames.size() > 0)
            {
                for ( Uint32 ctr=0; ctr < oldClassNames.size(); ctr++)
                {
#ifdef REPUPGRADE_DEBUG
            cout << "Processing namespace : "
                << (const char*)oldNamespaces[i].getString().getCString()
                << " class name : "
                << (const char*)oldClassNames[ctr].getString().getCString()
                << endl;
#endif

#ifdef NS_INTEROP
                    CIMClass oldClass = _oldRepository->getClass(
                                            oldNamespaces[i],
                                            oldClassNames[ctr]);
                    Uint32 pCnt = oldClass.getPropertyCount();
                    Boolean hasReference = false;
                    Array<CIMName> refProp;
                    for ( Uint32 j = 0; j < pCnt; j++)
                    {
                        CIMProperty prop = oldClass.getProperty(j);
                        if(prop.getType() == CIMTYPE_REFERENCE)
                        {
                            refProp.append(prop.getName());
                            hasReference = true;
                        }
                    } 
#endif

                    Array<CIMInstance>         instances;
                    Uint32                     ictr = 0;

                        instances = _oldRepository->enumerateInstancesForClass(
                                            oldNamespaces[i],
                                            oldClassNames[ctr],
                                            true,
                                            true);

                        if (instances.size() > 0)
                        {
#ifdef REPUPGRADE_DEBUG
                            cout << "Found instances : "
                                 << instances.size() << endl;
#endif
                            for ( ictr=0; ictr<instances.size(); ictr++)
                            {
                                try
                                {
                                    //
                                    // Check if the instance must be created.
                                    // If true, use the processed instance.
                                    // If an SSP Module has chosen to ignore
                                    // the instance then skip the
                                    // instance creation.
                                    //
                                    CIMInstance processedInstance =
                                               instances[ictr].clone();

#ifdef ENABLE_MODULE_PROCESSING
                                    if (!_invokeModules(instances[ictr],
                                              processedInstance))
                                    {
#ifdef REPUPGRADE_DEBUG
                                        cerr << "Ignoring instance creation : "
                                        << instances[ictr].getPath().toString()
                                        << endl;
#endif
                                        continue;
                                    }
#endif

                                    //
                                    // Create the instance.
                                    //
#ifdef REPUPGRADE_DEBUG
                                cout << "Creating instance" << endl;
#endif

#ifdef NS_INTEROP
                                if( hasReference == true)
                                {
#ifdef REPUPGRADE_DEBUG
                                   cout << "Calling _processInstance()" << endl;
#endif
                                   _processInstance(processedInstance, refProp);
                                } 
#endif
                                    _newRepository->createInstance(
                                                         oldNamespaces[i],
                                                         processedInstance);
#ifdef REPUPGRADE_DEBUG
                                cout << "Instance created" << endl;
#endif
                                }
                                catch (CIMException &ce)
                                {
                                    if (ce.getCode() == CIM_ERR_ALREADY_EXISTS)
                                    {
#ifdef REPUPGRADE_DEBUG
                                       cout <<
                                       "Instance already exists." << endl;
#endif
                                    }
                                    else
                                    {
                                        _logCreateInstanceError(
                                         oldNamespaces[i],
                                                        instances[ictr],
                                         (ce.getMessage()+". "));
                                    }
                                }
                                catch (Exception& e)
                                {
                                    _logCreateInstanceError(oldNamespaces[i],
                                                   instances[ictr],
                                                   (e.getMessage()+". "));

                                }
                                catch (...)
                                {
                                    _logCreateInstanceError(oldNamespaces[i],
                                                    instances[ictr],
                                                    String::EMPTY);
                                }
                            }
                        }
                   }
              }
         }
    }
}

#ifdef NS_INTEROP
void RepositoryUpgrade::_processInstance(CIMInstance& instance,
                       Array<CIMName> a)
{
    for(Uint32 k = 0; k < a.size(); k++)
    {
        try
        {
            Uint32 propIndex = instance.findProperty(a[k]);
            CIMProperty prop = instance.getProperty(propIndex);
            CIMObjectPath objPath;
            prop.getValue().get(objPath);
            if ( objPath.getNameSpace() == PEGASUS_NAMESPACE_PGINTEROP )
            { 
                objPath.setNameSpace(PEGASUS_NAMESPACENAME_INTEROP);
                instance.removeProperty(propIndex);
                prop.setValue(objPath);
                instance.addProperty(prop);
            }
        }
        catch(...)
        {
#ifdef REPUPGRADE_DEBUG
            cout << "Execption occured in _processInstance()" << endl;
#endif
            throw;
        }
    } 
}
#endif

void RepositoryUpgrade::_removeInstances(void)
{
    Array<CIMInstance> instances;
    //
    // Remove persistence CIM_ObjectManager instances from new repository.
    // It is no longer persistence.
    //
    try
    {
        instances = _newRepository->enumerateInstancesForClass(
            PEGASUS_NAMESPACENAME_INTEROP,
            PEGASUS_CLASSNAME_PG_OBJECTMANAGER);
    } 
    catch (const CIMException& e)
    {
        //
        // Interop namespace or object manager class may not exist
        //
        if (e.getCode() != CIM_ERR_INVALID_CLASS &&
            e.getCode() != CIM_ERR_INVALID_NAMESPACE)
        {
#ifdef REPUPGRADE_DEBUG
            cout << "Exception caught in attempting to remove "
                << "object manager instances in the new repository "
                << (const char*)e.getMessage().getCString()
                << endl;
#endif
            throw;
        }
        return;
    }

    for (Uint32 i = 0, n = instances.size(); i < n; i++)
    {
        CIMObjectPath objPath = instances[i].getPath();

        try
        {
            _newRepository->deleteInstance(
                PEGASUS_NAMESPACENAME_INTEROP,
                objPath);
        }
        catch (CIMException &e)
        {
            _logDeleteInstanceError(
                PEGASUS_NAMESPACENAME_INTEROP,
                objPath,
                e.getMessage());
        }
#ifdef REPUPGRADE_DEBUG
        cout << "Delete "
            << (const char*)
                   PEGASUS_CLASSNAME_PG_OBJECTMANAGER.getString().getCString()
            << " instance: "
            << (const char*)objPath.toString().getCString()
            << endl;
#endif
    }
}
////////////////////////////////////////////////////////////////////////////////
//
// _namespaceNameToDirName()
// Converts any "/"s in the Namespace name to "#".
//
////////////////////////////////////////////////////////////////////////////////

static String _namespaceNameToDirName(const CIMNamespaceName& namespaceName)
{
    String dirName = namespaceName.getString();

    for (Uint32 i=0; i<dirName.size(); i++)
    {
        if (dirName[i] == '/')
        {
            dirName[i] = '#';
        }
    }

    return dirName;
}

void RepositoryUpgrade::_logRequestToFile(
                              const String&   fileName)
{
    Uint64              timeoutMilliseconds = 20;
    Uint64              startMilliseconds = 0;
    Uint64              nowMilliseconds = 0;
    Uint64              stopMilliseconds = 0;
    String                          startLine;
    Array<HTTPHeader>               headers;
    Uint32                              contentLength = 0;
    Uint32                              contentOffset = 0;
    HTTPMessage*            httpMessage;
    Buffer          data;

    startMilliseconds = TimeValue::getCurrentTime().toMilliseconds();
    nowMilliseconds = startMilliseconds;
    stopMilliseconds = nowMilliseconds + timeoutMilliseconds;

    // Loop till we get response or timeout.
    while (nowMilliseconds < stopMilliseconds)
    {
        //
        // Check to see if incoming queue has a message
        //

        Message* response = dequeue();

        if (response)
        {
            httpMessage = (HTTPMessage*)response;
            data = httpMessage->message;
            httpMessage->parse( startLine, headers, contentLength );

            if( contentLength > 0 )
            {
                contentOffset = data.size() - contentLength;
            }
            else
            {
                contentOffset = data.size();
            }

            //
            //  Get HTTP header
            //
            if (contentOffset < data.size())
            {
                //
                //  Print XML response to the ostream
                //
                const char* content = data.getData() + contentOffset;

                ofstream outFile(fileName.getCString(), ios::app);
                if (!outFile)
                {
#ifdef REPUPGRADE_DEBUG
                    cerr << "Unable to open output file : "
                         << fileName << endl;
#endif
                    return;
                }
                XmlWriter::indentedPrint (outFile, content, 0);
                outFile.close();
            }

            delete response;
        }
        nowMilliseconds = TimeValue::getCurrentTime().toMilliseconds();
    }
}

void RepositoryUpgrade::_logCreateClassError(
               const CIMNamespaceName& namespaceName,
               const CIMClass& oldClass,
               const String&   message)
{
    CIMRequestMessage* request;

    // Create a CreateClass message.
    // NOTE: The allocated memory is freed once the response is dequeued
    //       by the method _logRequestToFile.
    request = new CIMCreateClassRequestMessage(
                                         String::EMPTY,
                                         namespaceName,
                                         oldClass,
                                         QueueIdStack());

    // Enqueue the request.
    _requestEncoder->enqueue(request);

    // Build the output filepath.
    String outputFileName = String(_LOG_PATH +
                                   _namespaceNameToDirName(namespaceName) +
                                   "."+
                                   oldClass.getClassName().getString() +
                                   _FILE_EXTENSION);

    // Log the request to output filepath.
    _logRequestToFile (outputFileName);

    String errMsg = localizeMessage ( MSG_PATH,
                              REPOSITORY_UPGRADE_FAILURE_KEY,
                              REPOSITORY_UPGRADE_FAILURE) +
                    message +
                    localizeMessage ( MSG_PATH,
                              CLASS_CREATION_ERROR_KEY,
                              CLASS_CREATION_ERROR,
                              oldClass.getClassName().getString(),
                              namespaceName.getString()) +
                    localizeMessage ( MSG_PATH,
                                      CLASS_XML_OUTPUT_FILE_KEY,
                                      CLASS_XML_OUTPUT_FILE,
                                      outputFileName);

    throw RepositoryUpgradeException (errMsg);
}

void RepositoryUpgrade::_logCreateInstanceError(
             const CIMNamespaceName& namespaceName,
             const CIMInstance& instance,
             const String&      message)
{
    CIMRequestMessage* request;

    instanceCount++;

    // Create a CreateInstance message.
    // NOTE: The allocated memory is freed once the response is dequeued
    //       by the method _logRequestToFile.
    request = new CIMCreateInstanceRequestMessage(
                                         String::EMPTY,
                                         namespaceName,
                                         instance,
                                         QueueIdStack());

    // Enqueue the request.
    _requestEncoder->enqueue(request);

    char buffer[34];
    sprintf( buffer, "%u", instanceCount );
    // Build the output filepath.
    String outputFileName = String(_LOG_PATH +
                                   _namespaceNameToDirName(namespaceName) +
                                   "."+
                                   "instance." + buffer +
                                   _FILE_EXTENSION);

    // Log the request to output filepath.
    _logRequestToFile (outputFileName);

    String errMsg = localizeMessage ( MSG_PATH, REPOSITORY_UPGRADE_FAILURE_KEY,
                                       REPOSITORY_UPGRADE_FAILURE)
                     + message
                     + localizeMessage ( MSG_PATH, INSTANCE_CREATION_ERROR_KEY,
                                         INSTANCE_CREATION_ERROR,
                                         namespaceName.getString())
                     + localizeMessage ( MSG_PATH, INSTANCE_XML_OUTPUT_FILE_KEY,
                                         INSTANCE_XML_OUTPUT_FILE,
                                         outputFileName);

    throw RepositoryUpgradeException (errMsg);
}

void RepositoryUpgrade::_logDeleteInstanceError(
    const CIMNamespaceName& namespaceName,
    const CIMObjectPath& instanceName,
    const String& message)
{
    CIMRequestMessage* request;

    instanceCount++;

    // Create a DeleteInstance message.
    // NOTE: The allocated memory is freed once the response is dequeued
    //       by the method _logRequestToFile.
    request = new CIMDeleteInstanceRequestMessage(
        String::EMPTY,
        namespaceName,
        instanceName,
        QueueIdStack());

    // Enqueue the request.
    _requestEncoder->enqueue(request);

    char buffer[34];
    sprintf( buffer, "%u", instanceCount );
    // Build the output filepath.
    String outputFileName = String(_LOG_PATH +
        _namespaceNameToDirName(namespaceName) +
        "."+
        "instance." + buffer +
        _FILE_EXTENSION);

    // Log the request to output filepath.
    _logRequestToFile (outputFileName);

    String errMsg =
        localizeMessage(
            MSG_PATH,
            REPOSITORY_UPGRADE_FAILURE_KEY,
            REPOSITORY_UPGRADE_FAILURE)
        + message
        + localizeMessage(
            MSG_PATH,
            INSTANCE_DELETION_ERROR_KEY,
            INSTANCE_DELETION_ERROR,
            namespaceName.getString())
        + localizeMessage(
             MSG_PATH,
             INSTANCE_XML_OUTPUT_FILE_KEY,
             INSTANCE_XML_OUTPUT_FILE,
             outputFileName);

    throw RepositoryUpgradeException (errMsg);
}

void RepositoryUpgrade::_logSetQualifierError(
          const CIMNamespaceName& namespaceName,
          const CIMQualifierDecl& qualifier,
          const String&           message)
{
    CIMRequestMessage* request;

    qualifierCount++;

    // Create a SetQualifier message.
    // NOTE: The allocated memory is freed once the response is dequeued
    //       by the method _logRequestToFile.
    request = new CIMSetQualifierRequestMessage(
                                         String::EMPTY,
                                         namespaceName,
                                         qualifier,
                                         QueueIdStack());

    // Enqueue the request.
    _requestEncoder->enqueue(request);

    char buffer[34];
    sprintf( buffer, "%u", qualifierCount );
    // Build the output filepath.
    String outputFileName = String(_LOG_PATH +
                                   _namespaceNameToDirName(namespaceName) +
                                   "." +
                                   "qualifier." + buffer +
                                   _FILE_EXTENSION );

    // Log the request to output filepath.
    _logRequestToFile (outputFileName);

    String errMsg =  localizeMessage ( MSG_PATH, REPOSITORY_UPGRADE_FAILURE_KEY,
                                       REPOSITORY_UPGRADE_FAILURE)
                     + message
                     + localizeMessage ( MSG_PATH, QUALIFIER_CREATION_ERROR_KEY,
                                         QUALIFIER_CREATION_ERROR,
                                         namespaceName.getString(),
                                         qualifier.getName().getString())
                     + localizeMessage (MSG_PATH, QUALIFIER_XML_OUTPUT_FILE_KEY,
                                          QUALIFIER_XML_OUTPUT_FILE,
                                          outputFileName);

    throw RepositoryUpgradeException (errMsg);
}

#ifdef ENABLE_MODULE_PROCESSING
DynamicLibrary RepositoryUpgrade::_loadSSPModule(const String& moduleName)
{
    String fileName;

//
// Get the Special Process Module location.
//
// ATTN-SF-P3-20050209: Platforms that use a different directory path other than
//                      $PEGASUS_HOME/lib must add to the code below.
//                      Also RELEASE_DIRS configurations must be appropriately
//                      updated.
//

#if defined (PEGASUS_OS_TYPE_WINDOWS)
    fileName = _pegasusHome + "/bin/" +
                  FileSystem::buildLibraryFileName(moduleName);
#else
    fileName = _pegasusHome + "/lib/" +
                  FileSystem::buildLibraryFileName(moduleName);
#endif

    DynamicLibrary dl(fileName);

    if (!dl.load())
    {
#ifdef REPUPGRADE_DEBUG
        cout << "Error is : " << strerror(errno) << endl;
#endif
        String message =  localizeMessage ( MSG_PATH,
                                            LIBRARY_LOAD_ERROR_KEY,
                                            LIBRARY_LOAD_ERROR,
                                            moduleName);

        throw RepositoryUpgradeException(message);
    }

    return dl;
}

SchemaSpecialProcessModule *  RepositoryUpgrade::_createSSPModule(
                                   DynamicLibrary& library)
{
    CREATE_SSPMODULE_FUNCTION sspModule;

    sspModule = (CREATE_SSPMODULE_FUNCTION)
                     library.getSymbol(
                     SchemaSpecialProcessModule::CREATE_SSPMODULE_ENTRY_POINT);

    if (!sspModule)
    {
        String message =  localizeMessage ( MSG_PATH,
                                            LIBRARY_ENTRY_POINT_ERROR_KEY,
                                            LIBRARY_ENTRY_POINT_ERROR,
                                            library.getFileName());

        throw RepositoryUpgradeException(message);
    }

    return(sspModule());
}

void RepositoryUpgrade::_initSSPModule()
{
    for (Uint32 counter=0; counter < SSPModuleTable::NUM_MODULES; counter++)
    {
        //
        // Load the SSPModule library.
        //
#ifdef REPUPGRADE_DEBUG
        cout << "Loading library name : "
             << schemaProcessingModules[counter].moduleName << endl;
#endif
        DynamicLibrary tmpLibrary
                 = _loadSSPModule(schemaProcessingModules[counter].moduleName);

        _library[counter] = tmpLibrary;

        //
        // Create the SSPModule object.
        //
        _sspModule[counter] = _createSSPModule (_library[counter]);
   }
}

void RepositoryUpgrade::_cleanupSSPModule()
{
    for (Uint32 counter=0; counter < SSPModuleTable::NUM_MODULES; counter++)
    {
#ifdef REPUPGRADE_DEBUG
        cout << "Unloading library name : "
             << schemaProcessingModules[counter].moduleName << endl;
#endif
        if (_library->isLoaded())
        {
            // Unload the library.
            _library->unload();
        }
    }
}

Boolean RepositoryUpgrade::_invokeModules(CIMQualifierDecl& inputQualifier,
                                          CIMQualifierDecl& outputQualifier)
{
    Boolean createQualifier = true;

    for (Uint32 counter=0; counter < SSPModuleTable::NUM_MODULES
         && createQualifier==true ; counter++)
    {
        if ((strcmp(schemaProcessingModules[counter].moduleType, _ALL) == 0) ||
             (strcmp(schemaProcessingModules[counter].moduleType,
                _QUALIFIER_ONLY ) == 0 ))
        {
#ifdef REPUPGRADE_DEBUG
            cout << "Invoking library name : "
             << schemaProcessingModules[counter].moduleName << endl;
#endif
            createQualifier =
                      _sspModule[counter]->processQualifier(inputQualifier,
                                                           outputQualifier);
            inputQualifier = outputQualifier.clone();
        }
    }

    return createQualifier;
}

Boolean RepositoryUpgrade::_invokeModules(CIMClass& inputClass,
                                          CIMClass& outputClass)
{
   Boolean createClass = true;

   for (Uint32 counter=0; counter < SSPModuleTable::NUM_MODULES
        && createClass==true ; counter++)
   {
       if ((strcmp(schemaProcessingModules[counter].moduleType, _ALL) == 0) ||
             (strcmp(schemaProcessingModules[counter].moduleType,
                _CLASS_ONLY ) == 0 ))
       {
#ifdef REPUPGRADE_DEBUG
           cout << "Invoking library name : "
                << schemaProcessingModules[counter].moduleName << endl;
#endif
           createClass =
               _sspModule[counter]->processClass(inputClass, outputClass);
           inputClass = outputClass.clone();
       }
   }

   return createClass;
}

Boolean RepositoryUpgrade::_invokeModules(CIMInstance& inputInstance,
                                          CIMInstance& outputInstance)
{
    Boolean createInstance = true;

    for (Uint32 counter=0; counter < SSPModuleTable::NUM_MODULES
          && createInstance==true; counter++)
    {
        if ((strcmp(schemaProcessingModules[counter].moduleType, _ALL) == 0) ||
             (strcmp(schemaProcessingModules[counter].moduleType,
                _INSTANCE_ONLY ) == 0 ))
        {
#ifdef REPUPGRADE_DEBUG
            cout << "Invoking library name : "
             << schemaProcessingModules[counter].moduleName << endl;
#endif
            createInstance =
            _sspModule[counter]->processInstance(inputInstance, outputInstance);

            inputInstance = outputInstance.clone();
        }
    }

    return createInstance;
}
#endif

/**

    Parses the command line for old repository path, creates RepositoryUpgrade
    instance and invokes upgradeRepository method.

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

#ifdef PEGASUS_OS_ZOS
    // for z/OS set stdout and stderr to EBCDIC
    setEBCDICEncoding(STDOUT_FILENO);
    setEBCDICEncoding(STDERR_FILENO);
#endif

    try
    {
        RepositoryUpgrade   command;
        Uint32          retCode;

        //l10n set message loading to process locale
        MessageLoader::_useProcessLocale = true;

        try
        {
            command.setCommand (argc, argv);
        }
        catch (CommandFormatException& cfe)
        {
            cerr << RepositoryUpgrade::COMMAND_NAME << ": " << cfe.getMessage ()
                 << endl;
            cerr << "Use '-h' or '--help' to obtain command syntax." << endl;
            return (Command::RC_ERROR);
        }

        retCode = command.execute (cout, cerr);

        return (retCode);

    }
    catch (...)
    {
        MessageLoaderParms msgParms( REPOSITORY_UPGRADE_UNKNOWN_ERROR_KEY,
                                     REPOSITORY_UPGRADE_UNKNOWN_ERROR );
        msgParms.msg_src_path = MSG_PATH;
        cerr << MessageLoader::getMessage(msgParms) << endl;

        MessageLoaderParms msgParmsAdd( REPOSITORY_UPGRADE_FAILURE_KEY,
                                        REPOSITORY_UPGRADE_FAILURE );
        msgParmsAdd.msg_src_path = MSG_PATH;
        cerr << MessageLoader::getMessage(msgParmsAdd) << endl;

        return 1;
    }
}

