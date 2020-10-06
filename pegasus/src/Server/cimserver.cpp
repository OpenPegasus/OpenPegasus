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


//////////////////////////////////////////////////////////////////////
//
// Notes on daemon operation (Unix) and service operation (Win 32):
//
// To run pegasus as a daemon on Unix platforms:
//
// cimserver
//
// To NOT run pegasus as a daemon on Unix platforms, set the daemon config
// property to false:
//
// cimserver daemon=false
//
// The daemon config property has no effect on windows operation or when
// privilege separation is enabled.
//
// To shutdown pegasus, use the -s option:
//
// cimserver -s
//
// To run pegasus as an NT service, there are FOUR  different possibilities:
//
// To INSTALL the Pegasus service,
//
// cimserver -install
//
// To REMOVE the Pegasus service,
//
// cimserver -remove
//
// To START the Pegasus service,
//
// net start cimserver
// or
// cimserver -start
//
// To STOP the Pegasus service,
//
// net stop cimserver
// or
// cimserver -stop
//
// Alternatively, you can use the windows service manager. Pegasus shows up
// in the service database as "Pegasus CIM Object Manager"
//
//////////////////////////////////////////////////////////////////////


#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/Constants.h>
#include <Pegasus/Common/PegasusAssert.h>
#include <Pegasus/Common/FileSystem.h>
#include <Pegasus/Common/PegasusVersion.h>
#include <Pegasus/Common/Logger.h>
#include <Pegasus/Common/StringConversion.h>
#include <Pegasus/Common/System.h>
#include <Pegasus/Common/Tracer.h>
#include <Pegasus/Common/LanguageParser.h>
#include <Pegasus/Config/ConfigManager.h>
#include <Pegasus/Client/CIMClient.h>
#include <Pegasus/Server/CIMServer.h>
#include <Service/ServerProcess.h>
#include <Service/ServerShutdownClient.h>
#include <Service/ServerRunStatus.h>

#if defined(PEGASUS_OS_ZOS)
#include <Pegasus/General/SetFileDescriptorToEBCDICEncoding.h>
#include <Service/ARM_zOS.h>
#include <Service/TimeZone_zOS.h>
#include <Service/WaitForTCPIP_zOS.h>
# ifdef PEGASUS_ZOS_SECURITY
// This include file will not be provided in the OpenGroup CVS for now.
// Do NOT try to include it in your compile
#  include <Pegasus/Common/safCheckzOS_inline.h>
# endif
#endif

#if defined(PEGASUS_OS_TYPE_UNIX)
# include <unistd.h>
# include <sys/types.h>
# include <sys/stat.h>
# include <fcntl.h>
#endif

#ifdef PEGASUS_ENABLE_PRIVILEGE_SEPARATION
# define PEGASUS_PROCESS_NAME "cimservermain"
#else
# define PEGASUS_PROCESS_NAME "cimserver"
#endif

#include <Pegasus/Common/Executor.h>

#ifdef PEGASUS_OS_PASE
# include <ILEWrapper/ILEUtilities2.h>
# include <ILEWrapper/qumemultiutil.h>
#endif

PEGASUS_USING_PEGASUS;
PEGASUS_USING_STD;

//Windows service variables are not defined elsewhere in the product
//enable ability to override these
#ifndef PEGASUS_FLAVOR
#ifndef PEGASUS_SERVICE_NAME
#define PEGASUS_SERVICE_NAME "Pegasus CIM Object Manager";
#endif
#ifndef PEGASUS_SERVICE_DESCRIPTION
#define PEGASUS_SERVICE_DESCRIPTION "Pegasus CIM Object Manager Service";
#endif
#else
#ifndef PEGASUS_SERVICE_NAME
#define PEGASUS_SERVICE_NAME PEGASUS_FLAVOR " Pegasus CIM Object Manager"
#endif
#ifndef PEGASUS_SERVICE_DESCRIPTION
#define PEGASUS_SERVICE_DESCRIPTION PEGASUS_FLAVOR \
            " Pegasus CIM Object Manager Service";
#endif
#endif

#ifdef PEGASUS_OS_PASE
#include <as400_protos.h> //for _SETCCSID
#endif

class CIMServerProcess : public ServerProcess
{
public:

    CIMServerProcess()
    {
        cimserver_set_process(this);
    }

    virtual ~CIMServerProcess()
    {
    }

    //defined in PegasusVersion.h
    virtual const char* getProductName() const
    {
        return PEGASUS_PRODUCT_NAME;
    }

    virtual const char* getExtendedName() const
    {
        return PEGASUS_SERVICE_NAME;
    }

    virtual const char* getDescription() const
    {
        return PEGASUS_SERVICE_DESCRIPTION;
    }

    //defined in PegasusVersion.h
    virtual const char* getCompleteVersion() const
    {
      if (*PEGASUS_PRODUCT_STATUS == '\0' )
        return PEGASUS_PRODUCT_VERSION;
      else
        return PEGASUS_PRODUCT_VERSION " " PEGASUS_PRODUCT_STATUS;
    }

    //defined in PegasusVersion.h
    virtual const char* getVersion() const
    {
        return PEGASUS_PRODUCT_VERSION;
    }

    virtual const char* getProcessName() const
    {
        return PEGASUS_PROCESS_NAME;
    }

    int cimserver_run(
        int argc,
        char** argv,
        Boolean shutdownOption,
        Boolean debugOutputOption);

    void cimserver_stop();
};

ServerRunStatus _serverRunStatus(
    PEGASUS_PROCESS_NAME, PEGASUS_CIMSERVER_START_FILE);
AutoPtr<CIMServerProcess> _cimServerProcess(new CIMServerProcess());
static CIMServer* _cimServer = 0;
static Thread* dummyInitialThread = 0;

//
//  The command name.
//
static const char COMMAND_NAME []    = "cimserver";

//
//  The constant defining usage string.
//
static const char USAGE []           = "Usage: ";

/**
Constants representing the command line options.
*/
static const char OPTION_VERSION     = 'v';

static const char OPTION_HELP        = 'h';

static const char OPTION_HOME        = 'D';

static const char OPTION_SHUTDOWN    = 's';

static const char OPTION_DEBUGOUTPUT = 'X';

/** GetOptions function - This function defines the Options Table
    and sets up the options from that table using the config manager.

    Some possible exceptions:  NoSuchFile, FileNotReadable, CannotRenameFile,
    ConfigFileSyntaxError, UnrecognizedConfigProperty, InvalidPropertyValue,
    CannotOpenFile.
*/
void GetOptions(
    ConfigManager* cm,
    int& argc,
    char** argv,
    Boolean shutdownOption)
{
    if (shutdownOption)
    {
        cm->loadConfigFiles();
    }
    else
    {
        cm->mergeConfigFiles();
    }

    // Temporarily disable updates to the current configuration
    // file if shutdownOption is true
    cm->useConfigFiles = (shutdownOption==false);

    cm->mergeCommandLine(argc, argv);

    // Enable updates again
    cm->useConfigFiles = true;
}

/* PrintHelp - This is temporary until we expand the options manager to allow
   options help to be defined with the OptionRow entries and presented from
   those entries.
*/
void PrintHelp(const char* arg0)
{
    String usage = String (USAGE);
    usage.append(COMMAND_NAME);
    usage.append(" [ [ options ] | [ configProperty=value, ... ] ]\n");
    usage.append("  options\n");
    usage.append("    -v, --version   - displays CIM Server version number\n");
    usage.append("    --status        - displays the running status of"
        " the CIM Server\n");
    usage.append("    -h, --help      - prints this help message\n");
    usage.append("    -s              - shuts down CIM Server\n");
#if !defined(PEGASUS_USE_RELEASE_DIRS)
    usage.append("    -D [home]       - sets pegasus home directory\n");
#endif
#if defined(PEGASUS_OS_TYPE_WINDOWS)
    usage.append("    -install [name] - installs pegasus as a Windows "
        "Service\n");
    usage.append("                      [name] is optional and overrides "
        "the\n");
    usage.append("                      default CIM Server Service Name\n");
    usage.append("                      by appending [name]\n");
    usage.append("    -remove [name]  - removes pegasus as a Windows "
        "Service\n");
    usage.append("                      [name] is optional and overrides "
        "the\n");
    usage.append("                      default CIM Server Service Name\n");
    usage.append("                      by appending [name]\n");
    usage.append("    -start [name]   - starts pegasus as a Windows Service\n");
    usage.append("                      [name] is optional and overrides "
        "the\n");
    usage.append("                      default CIM Server Service Name\n");
    usage.append("                      by appending [name]\n");
    usage.append("    -stop [name]    - stops pegasus as a Windows Service\n");
    usage.append("                      [name] is optional and overrides "
        "the\n");
    usage.append("                      default CIM Server Service Name\n");
    usage.append("                      by appending [name]\n\n");
#endif
    usage.append("  configProperty=value\n");
    usage.append("                    - sets CIM Server configuration "
        "property\n");

    cout << endl;
    cout << _cimServerProcess->getProductName() << " " <<
        _cimServerProcess->getCompleteVersion() << endl;
    cout << endl;

#if defined(PEGASUS_OS_TYPE_WINDOWS)
    MessageLoaderParms parms("src.Server.cimserver.MENU.WINDOWS", usage);
#elif defined(PEGASUS_USE_RELEASE_DIRS)
    MessageLoaderParms parms(
        "src.Server.cimserver.MENU.HPUXLINUXIA64GNU",
        usage);
#else
    MessageLoaderParms parms("src.Server.cimserver.MENU.STANDARD", usage);
#endif
    cout << MessageLoader::getMessage(parms) << endl;
}

// This needs to be called at various points in the code depending on the
// platform and error conditions.
// We need to delete the _cimServer reference on exit in order for the
// destructors to get called.
void deleteCIMServer()
{
    delete _cimServer;
    _cimServer = 0;

    if (dummyInitialThread)
    {
        Thread::clearLanguages();
        delete dummyInitialThread;
    }
    _serverRunStatus.setServerNotRunning();
}

//
// Dummy function for the Thread object associated with the initial thread.
// Since the initial thread is used to process CIM requests, this is
// needed to localize the exceptions thrown during CIM request processing.
// Note: This function should never be called!
//
ThreadReturnType PEGASUS_THREAD_CDECL dummyThreadFunc(void* parm)
{
    return (ThreadReturnType)0;
}

#ifdef PEGASUS_ENABLE_PRIVILEGE_SEPARATION

static int _extractExecutorSockOpt(int& argc, char**& argv)
{
    // Extract the "--executor-socket <sock>" option if any. This indicates
    // that the e[x]ecutor is running. The option argument is the socket used
    // to communicate with the executor. Remove the option from the
    // argv list and decrease argc by two.

    int sock = -1;
    const char OPT[] = "--executor-socket";

    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], OPT) == 0)
        {
            // Check for missing option argument.

            if (i + 1 == argc)
            {
                MessageLoaderParms parms(
                    "src.Server.cimserver.MISSING_OPTION_ARGUMENT",
                    "Missing argument for $0 option.",
                    OPT);
                cerr << argv[0] << ": " << MessageLoader::getMessage(parms) <<
                    endl;
                exit(1);
            }

            // Convert argument to positive integer.

            char* end;
            unsigned long x = strtoul(argv[i+1], &end, 10);

            // Check whether option argument will fit in a signed integer.

            if (*end != '\0' || x > 2147483647)
            {
                MessageLoaderParms parms(
                    "src.Server.cimserver.BAD_OPTION_ARGUMENT",
                    "Bad $0 option argument: $1.",
                    OPT,
                    argv[i+1]);
                cerr << argv[0] << ": " << MessageLoader::getMessage(parms) <<
                    endl;
                exit(1);
            }

            sock = int(x);

            // Remove "-x <sock>" from argv-argc.

            memmove(argv + i, argv + i + 2, sizeof(char*) * (argc - i - 1));
            argc -= 2;
            break;
        }
    }

    if (sock == -1)
    {
        MessageLoaderParms parms(
            "src.Server.cimserver.MISSING_OPTION",
            "Missing $0 option.",
            OPT);
        cerr << argv[0] << ": " << MessageLoader::getMessage(parms) << endl;
        exit(1);
    }

    return sock;
}

#endif /* PEGASUS_ENABLE_PRIVILEGE_SEPARATION */

static void _initConfigProperty(const String &propName, Uint32 value)
{
    char strValue[22];
    Uint32 n;
    const char *startP = Uint32ToString(
        strValue,
        value,
        n);
    ConfigManager::getInstance()->initCurrentValue(propName, String(startP, n));
}
static void _restrictListening(
    ConfigManager* configManager,
    const String &listenOn,
    const Uint32 &portNumberHttp,
    const Boolean useSSL)
{
    static Array<HostAddress> laddr = configManager ->getListenAddress( 
                                          listenOn);
    for(Uint32 i = 0, n = laddr.size(); i < n; ++i)
    {
        if(laddr[i].getAddressType() == HostAddress::AT_IPV6)
        {
#ifdef PEGASUS_ENABLE_IPV6
            _cimServer->addAcceptor(HTTPAcceptor::IPV6_CONNECTION,
                    portNumberHttp, useSSL, 
                    &laddr[i]);
#endif
        }
        else if(laddr[i].getAddressType() == HostAddress::AT_IPV4)
        {
            _cimServer->addAcceptor(HTTPAcceptor::IPV4_CONNECTION,
                    portNumberHttp, useSSL, 
                    &laddr[i]);
        }
    }
}

/////////////////////////////////////////////////////////////////////////
//  MAIN
//////////////////////////////////////////////////////////////////////////

int main(int argc, char** argv)
{
    String pegasusHome;
    Boolean shutdownOption = false;
    Boolean debugOutputOption = false;

    // Set Message loading to process locale
    MessageLoader::_useProcessLocale = true;

#ifdef PEGASUS_OS_ZOS
    // Direct standard input to /dev/null,
    close(STDIN_FILENO);
    open("/dev/null", O_RDONLY);

    if ( setEBCDICEncoding(STDOUT_FILENO)==-1 ||
         setEBCDICEncoding(STDERR_FILENO)==-1 )
    {
       PEG_TRACE_CSTRING(TRC_SERVER,Tracer::LEVEL1,
           "Coud not set stdout or stderr to EBCDIC encoding.");
    }
    // Need to initialize timezone information in the
    // initial processing thread (IPT)
    initialize_zOS_timezone();
#endif

#if defined(PEGASUS_OS_AIX) && defined(PEGASUS_HAS_MESSAGES)
    setlocale(LC_ALL, "");
#endif

#ifndef PEGASUS_OS_TYPE_WINDOWS
    //
    // Get environment variables:
    //
# if defined(PEGASUS_OS_AIX) && defined(PEGASUS_USE_RELEASE_DIRS)
    pegasusHome = AIX_RELEASE_PEGASUS_HOME;
# elif defined(PEGASUS_OS_PASE)
    const char *tmp = getenv("PEGASUS_HOME");
    pegasusHome = (tmp == 0) ? PASE_DEFAULT_PEGASUS_HOME : tmp;
# elif !defined(PEGASUS_USE_RELEASE_DIRS) || \
    defined(PEGASUS_OS_ZOS)
    const char* tmp = getenv("PEGASUS_HOME");

    if (tmp)
    {
        pegasusHome = tmp;
    }
# endif

    FileSystem::translateSlashes(pegasusHome);
#else

    // windows only
    //setHome(pegasusHome);
    pegasusHome = _cimServerProcess->getHome();
#endif

#ifdef PEGASUS_ENABLE_PRIVILEGE_SEPARATION

    // If invoked with "--executor-socket <socket>" option, then use executor.

    Executor::setSock(_extractExecutorSockOpt(argc, argv));

    // Ping executor to verify the specified socket is valid.

    if (Executor::ping() != 0)
    {
        MessageLoaderParms parms("src.Server.cimserver.EXECUTOR_PING_FAILED",
            "Failed to ping the executor on the specified socket.");
        cerr << argv[0] << ": " << MessageLoader::getMessage(parms) << endl;
        exit(1);
    }

#endif /* !defined(PEGASUS_ENABLE_PRIVILEGE_SEPARATION) */

        // Get help, version, status and shutdown options

        for (int i = 1; i < argc; )
        {
            const char* arg = argv[i];
            if (strcmp(arg, "--help") == 0)
            {
                PrintHelp(argv[0]);
                Executor::daemonizeExecutor();
                exit(0);
            }
            if (strcmp(arg, "--status") == 0)
            {
                int retValue = 0;
                if (_serverRunStatus.isServerRunning())
                {
                    MessageLoaderParms parms(
                        "src.Server.cimserver.CIMSERVER_RUNNING",
                        "The CIM Server is running.");
                    cout << MessageLoader::getMessage(parms) << endl;
                }
                else
                {
                    MessageLoaderParms parms(
                        "src.Server.cimserver.CIMSERVER_NOT_RUNNING",
                        "The CIM Server is not running.");
                    cout << MessageLoader::getMessage(parms) << endl;
                    retValue = 2;
                }
                Executor::daemonizeExecutor();
                exit(retValue);
            }
            else if (strcmp(arg, "--version") == 0)
            {
                cout << _cimServerProcess->getCompleteVersion() << endl;
                Executor::daemonizeExecutor();
                exit(0);
            }
            // Check for -option
            else if (*arg == '-')
            {
                // Get the option
                const char* option = arg + 1;

                //
                // Check to see if user asked for the version (-v option):
                //
                if (*option == OPTION_VERSION &&
                    strlen(option) == 1)
                {
                    cout << _cimServerProcess->getCompleteVersion() << endl;
                    Executor::daemonizeExecutor();
                    exit(0);
                }
                //
                // Check to see if user asked for help (-h option):
                //
                else if (*option == OPTION_HELP &&
                        (strlen(option) == 1))
                {
                    PrintHelp(argv[0]);
                    Executor::daemonizeExecutor();
                    exit(0);
                }
#if !defined(PEGASUS_USE_RELEASE_DIRS)
                else if (*option == OPTION_HOME &&
                        (strlen(option) == 1))
                {
                    if (i + 1 < argc)
                    {
                        pegasusHome.assign(argv[i + 1]);
                    }
                    else
                    {
                        String opt(option);
                        MessageLoaderParms parms(
                            "src.Server.cimserver.MISSING_ARGUMENT",
                            "Missing argument for option -$0",
                            opt);
                        cout << MessageLoader::getMessage(parms) << endl;
                        exit(1);
                    }

                    memmove(&argv[i], &argv[i + 2], (argc-i-1) * sizeof(char*));
                    argc -= 2;
                }
#endif
                //
                // Check to see if user asked for debug output (-X option):
                //
                else if (*option == OPTION_DEBUGOUTPUT &&
                        (strlen(option) == 1))
                {
                    MessageLoaderParms parms(
                        "src.Server.cimserver.UNSUPPORTED_DEBUG_OPTION",
                        "Unsupported debug output option is enabled.");
                    cout << MessageLoader::getMessage(parms) << endl;

                    debugOutputOption = true;

#if defined(PEGASUS_OS_HPUX)
                    System::bindVerbose = true;
#endif

                    // remove the option from the command line
                    memmove(&argv[i], &argv[i + 1], (argc-i) * sizeof(char*));
                    argc--;
                }
                //
                // Check to see if user asked for shutdown (-s option):
                //
                else if (*option == OPTION_SHUTDOWN &&
                        (strlen(option) == 1))
                {
                    //
                    // Check to see if shutdown has already been specified:
                    //
                    if (shutdownOption)
                    {
                        MessageLoaderParms parms(
                            "src.Server.cimserver.DUPLICATE_SHUTDOWN_OPTION",
                            "Duplicate shutdown option specified.");

                        cout << MessageLoader::getMessage(parms) << endl;
                        exit(1);
                    }

                    shutdownOption = true;

                    // remove the option from the command line
                    memmove(&argv[i], &argv[i + 1], (argc-i) * sizeof(char*));
                    argc--;
                }
                else
                    i++;
            }
            else
                i++;
        }

    //
    // Set the value for pegasusHome property
    //
    ConfigManager::setPegasusHome(pegasusHome);

    //
    // Do the platform specific run
    //

    return _cimServerProcess->platform_run(
        argc, argv, shutdownOption, debugOutputOption);
}

void CIMServerProcess::cimserver_stop()
{
    _cimServer->shutdownSignal();
}

//
// The main, common, running code
//
// NOTE: Do NOT call exit().  Use return(), otherwise some platforms
// will fail to shutdown properly/cleanly.
//
// TODO: Current change minimal for platform "service" shutdown bug fixes.
// Perhaps further extract out common stuff and put into main(), put
// daemon stuff into platform specific platform_run(), etc.
// Note: make sure to not put error handling stuff that platform
// specific runs may need to deal with better (instead of exit(), etc).
//

int CIMServerProcess::cimserver_run(
    int argc,
    char** argv,
    Boolean shutdownOption,
    Boolean debugOutputOption)
{
    Boolean daemonOption = false;

#if defined (PEGASUS_OS_PASE) && !defined (PEGASUS_DEBUG)
    // PASE have itself regular for checking privileged user
    if (!System::isPrivilegedUser("*CURRENT  "))
    {
        MessageLoaderParms parms(
                "src.Server.cimserver.NO_AUTHORITY.PEGASUS_OS_PASE",
                "The caller should be a privileged user,"
                " or the server will not run.");
        cerr << MessageLoader::getMessage(parms) << endl;
        exit (1);
    }
    char jobName[11];
    // this function only can be found in PASE environment
    umeGetJobName(jobName, false);
    if (strncmp("QUMECIMOM ", jobName, 10) != 0
            && strncmp("QUMEENDCIM", jobName, 10) != 0)
    {
        MessageLoaderParms parms(
                "src.Server.cimserver.NOT_OFFICIAL_START.PEGASUS_OS_PASE",
                "cimserver can not be started by user.\nServer will not run.");
        cerr << MessageLoader::getMessage(parms) << endl;
        exit (1);
    }

    // Direct standard input, output, and error to /dev/null,
    // PASE run this job in background, any output in not allowed
    freopen("/dev/null", "r", stdin);
    freopen("/dev/null", "w", stdout);
    freopen("/dev/null", "w", stderr);
#endif

    //
    // Get an instance of the Config Manager.
    //
    ConfigManager* configManager = ConfigManager::getInstance();
    configManager->useConfigFiles = true;

    try
    {
        //
        // Get options (from command line and from configuration file); this
        // removes corresponding options and their arguments from the command
        // line.  NOTE: If shutdownOption=true, the contents of current config
        // file are not overwritten by the planned config file.
        //
        GetOptions(configManager, argc, argv, shutdownOption);

        //
        // Initialize the message home directory in the MessageLoader.
        // This is the default directory where the resource bundles are found.
        //
        MessageLoader::setPegasusMsgHome(ConfigManager::getHomedPath(
            ConfigManager::getInstance()->getCurrentValue("messageDir")));

#if !defined(PEGASUS_USE_SYSLOGS)
        String logsDirectory = ConfigManager::getHomedPath(
            configManager->getCurrentValue("logdir"));

        // Set up the Logger.  This does not open the logs.
        // Might be more logical to clean before set.
        Logger::setHomeDirectory(logsDirectory);
#endif


#ifdef PEGASUS_OS_PASE
        /* write job log to tell where pegasus log is.*/
        if(logsDirectory.size() > 0)
            // this function only can be found in PASE environment
            logPegasusDir2joblog(logsDirectory.getCString());
        else
            logPegasusDir2joblog(".");

        // set ccsid to unicode for entire job
        // ccsid is globolization mechanism in PASE environment
        if (_SETCCSID(1208) == -1)
        {
            MessageLoaderParms parms(
                    "src.Server.cimserver.SET_CCSID_ERROR.PEGASUS_OS_PASE",
                    "Failed to set CCSID, server will stop.");
            cerr << MessageLoader::getMessage(parms) << endl;
            Logger::put_l(Logger::ERROR_LOG, System::CIMSERVER, Logger::FATAL,
                    parms);
            exit (1);
        }

        char fullJobName[29];
        umeGetJobName(fullJobName, true);
        Logger::put_l(Logger::STANDARD_LOG, System::CIMSERVER,
                Logger::INFORMATION,
                MessageLoaderParms(
                    "src.Server.cimserver.SERVER_JOB_NAME.PEGASUS_OS_PASE",
                    "CIM Server's Job Name is: $0", fullJobName));
#endif

#ifdef PEGASUS_DISABLE_LOCAL_DOMAIN_SOCKET
        // Make sure at least one connection is enabled

        Boolean enableHttpConnection = ConfigManager::parseBooleanValue(
            configManager->getCurrentValue("enableHttpConnection"));
        Boolean enableHttpsConnection = ConfigManager::parseBooleanValue(
            configManager->getCurrentValue("enableHttpsConnection"));

        if (!enableHttpConnection && !enableHttpsConnection)
        {
            MessageLoaderParms parms(
                "src.Server.cimserver.HTTP_NOT_ENABLED_SERVER_NOT_STARTING",
                "Neither HTTP nor HTTPS connection is enabled."
                    "  CIMServer will not be started.");
            Logger::put_l(
                Logger::STANDARD_LOG, System::CIMSERVER, Logger::WARNING,
                parms);
            cerr << MessageLoader::getMessage(parms) << endl;
            return 1;
        }
#endif

        //
        // Check to see if we should start Pegasus as a daemon
        //
        daemonOption = ConfigManager::parseBooleanValue(
            configManager->getCurrentValue("daemon"));

        if ((Executor::detectExecutor() == 0) && (daemonOption == false))
        {
            MessageLoaderParms parms(
                "src.Server.cimserver.PRIVSEP_REQUIRES_DAEMON",
                "Warning: The configuration setting daemon=false is ignored "
                    "with privilege separation enabled.");
            cerr << MessageLoader::getMessage(parms) << endl;
            daemonOption = true;
        }

        //
        // Check to see if we need to shutdown CIMOM
        //
        if (shutdownOption)
        {
#if defined(PEGASUS_OS_ZOS) && defined(PEGASUS_ZOS_SECURITY)
            // This checks whether user is authorized to stop the
            // CIM Server. When unauthorized a message is logged to
            // to the user and program exits.
            shutdownCheckProfileCIMSERVclassWBEM();
            // Depending on the success of the previous check we may not
            // reach this code!!!
#endif
            String configTimeout =
                configManager->getCurrentValue("shutdownTimeout");
            Uint32 timeoutValue =
                strtol(configTimeout.getCString(), (char **)0, 10);

            ServerShutdownClient serverShutdownClient(&_serverRunStatus);
            serverShutdownClient.shutdown(timeoutValue);

            MessageLoaderParms parms(
                "src.Server.cimserver.SERVER_STOPPED",
                "CIM Server stopped.");

            cout << MessageLoader::getMessage(parms) << endl;
            return 0;
        }

#if defined(PEGASUS_DEBUG) && !defined(PEGASUS_USE_SYSLOGS)
        // Leave this in until people get familiar with the logs.
        MessageLoaderParms parms("src.Server.cimserver.LOGS_DIRECTORY",
                                 "Logs Directory = ");
        cout << MessageLoader::getMessage(parms) << logsDirectory << endl;
#endif
    }
    catch (Exception& e)
    {
        MessageLoaderParms parms("src.Server.cimserver.SERVER_NOT_STARTED",
            "cimserver not started: $0", e.getMessage());
        Logger::put_l(Logger::ERROR_LOG, System::CIMSERVER, Logger::SEVERE,
            parms);
        cerr << MessageLoader::getMessage(parms) << endl;

        return 1;
    }

#if defined(PEGASUS_OS_ZOS) 
#  if defined(PEGASUS_ZOS_SECURITY)
    startupCheckBPXServer(true);
    startupCheckProfileCIMSERVclassWBEM();
    startupEnableMSC();
#  endif
    startupWaitForTCPIP();
#endif

#if defined(PEGASUS_DEBUG)
    // Put out startup up message.
    cout << _cimServerProcess->getProductName() << " " <<
        _cimServerProcess->getCompleteVersion() << endl;
#endif

    // Force initialization of hostname and fullyQualifiedHostName through
    // retrieving current value from Configuration Manager
    // - this will run getCurrentValue() in DefaultPropertyOwner.cpp
    configManager->getCurrentValue("hostname");
    configManager->getCurrentValue("fullyQualifiedHostName");

    // reset message loading to NON-process locale
    MessageLoader::_useProcessLocale = false;

    // Get the parent's PID before forking
    _serverRunStatus.setParentPid(System::getPID());

    // Do not fork when using privilege separation (executor will daemonize
    // itself later).
    if (daemonOption)
    {
        if (-1 == _cimServerProcess->cimserver_fork())
            return -1;
    }

    // Now we are after the fork...
    // Create a dummy Thread object that can be used to store the
    // AcceptLanguageList object for CIM requests that are serviced
    // by this thread (initial thread of server).  Need to do this
    // because this thread is not in a ThreadPool, but is used
    // to service CIM requests.
    // The run function for the dummy Thread should never be called,
    dummyInitialThread = new Thread(dummyThreadFunc, NULL, false);
    Thread::setCurrent(dummyInitialThread);
    try
    {
        Thread::setLanguages(LanguageParser::getDefaultAcceptLanguages());
    }
    catch (InvalidAcceptLanguageHeader& e)
    {
        Logger::put_l(Logger::ERROR_LOG, System::CIMSERVER, Logger::SEVERE,
            MessageLoaderParms(
                "src.Server.cimserver.FAILED_TO_SET_PROCESS_LOCALE",
                "Could not convert the system process locale into a valid "
                    "AcceptLanguage format."));
        Logger::put(Logger::ERROR_LOG, System::CIMSERVER, Logger::SEVERE,
            e.getMessage());
    }

#ifndef PEGASUS_OS_TYPE_WINDOWS
    umask(S_IRWXG|S_IRWXO);
#endif

    // Start up the CIM Server

#if defined(PEGASUS_OS_TYPE_UNIX)
    //
    // Lock the CIMSERVER_LOCK_FILE during CIM Server start-up to prevent
    // concurrent writes to this file by multiple cimserver processes
    // starting at the same time.
    //
    CString startupLockFileName = ConfigManager::getHomedPath(
       PEGASUS_CIMSERVER_START_LOCK_FILE).getCString();

    try
    {
    
        // Make sure the start-up lock file exists
        FILE* startupLockFile;
        if ((startupLockFile = fopen(startupLockFileName, "w")) != 0)
        {
            fclose(startupLockFile);
        }

        AutoFileLock fileLock(startupLockFileName);
#else
    try
    {
#endif

#if defined(PEGASUS_OS_TYPE_UNIX) || defined(PEGASUS_OS_VMS)
        //
        // Check if a CIM Server is already running.  If so, print an error
        // message and notify the parent process (if there is one) to terminate
        //
        if (_serverRunStatus.isServerRunning())
        {
            MessageLoaderParms parms(
                "src.Server.cimserver.UNABLE_TO_START_SERVER_ALREADY_RUNNING",
                "Unable to start CIMServer. CIMServer is already running.");
            Logger::put_l(
                Logger::ERROR_LOG, System::CIMSERVER, Logger::INFORMATION,
                parms);
            cerr << MessageLoader::getMessage(parms) << endl;

            if (daemonOption)
            {
                _cimServerProcess->notify_parent(1);
            }

            return 1;
        }

        //
        // Declare ourselves as the running CIM Server process, and write our
        // PID to the PID file.
        //
        _serverRunStatus.setServerRunning();
#endif

        // Create and initialize the CIMServer object

        _cimServer = new CIMServer();

        Boolean enableHttpConnection = ConfigManager::parseBooleanValue(
            configManager->getCurrentValue("enableHttpConnection"));
        Boolean enableHttpsConnection = ConfigManager::parseBooleanValue(
            configManager->getCurrentValue("enableHttpsConnection"));

#ifdef PEGASUS_DISABLE_LOCAL_DOMAIN_SOCKET
        // Make sure at least one connection is enabled
        if (!enableHttpConnection && !enableHttpsConnection)
        {
            MessageLoaderParms parms(
                "src.Server.cimserver.HTTP_NOT_ENABLED_SERVER_NOT_STARTING",
                "Neither HTTP nor HTTPS connection is enabled.");
            throw Exception(parms);
        }
#endif

        Boolean addIP6Acceptor = false;
        Boolean addIP4Acceptor = false;

#ifdef PEGASUS_OS_TYPE_WINDOWS
        addIP4Acceptor = true;
#endif

#ifdef PEGASUS_ENABLE_IPV6
        // If IPv6 stack is disabled swicth to IPv4 stack.
        if (System::isIPv6StackActive())
        {
            addIP6Acceptor = true;
        }
        else
        {
            PEG_TRACE_CSTRING(TRC_SERVER,Tracer::LEVEL4,
                "IPv6 stack is not active, using IPv4 socket.");
        }
#endif
        if (!addIP6Acceptor)
        {
            addIP4Acceptor = true;
        }

        // The server HTTP and HTTPS ports are determined via this algorithm:
        // 1) If the user explicitly specified a port, use it.
        // 2) If the user did not specify a port, get the port from the
        //    services file.
        // 3) If no value is specified in the services file, use the IANA WBEM
        //    default port.
        // Note that 2 and 3 are done within the System::lookupPort method
        // An empty string from the ConfigManager implies that the user did not
        // specify a port.

        if (enableHttpConnection)
        {
            Uint32 portNumberHttp = 0;
            String httpPort = configManager->getCurrentValue("httpPort");
            if (httpPort.size() == 0)
            {
                //
                // Look up the WBEM-HTTP port number
                //
                portNumberHttp = System::lookupPort(
                    WBEM_HTTP_SERVICE_NAME, WBEM_DEFAULT_HTTP_PORT);
                _initConfigProperty("httpPort", portNumberHttp);
            }
            else
            {
                Uint64 longNumber;
                // use the current value which has been checked for validity at
                // load(fct. GetOptions), see DefaultPropertyOwner::isValid()
                StringConversion::decimalStringToUint64(
                    httpPort.getCString(),
                    longNumber);
                portNumberHttp = longNumber & 0xffff;
            }

            String listenOn = configManager->getCurrentValue("listenAddress");
            if(String::equalNoCase(listenOn, "All"))
            {
                if (addIP6Acceptor)
                {
                    _cimServer->addAcceptor(HTTPAcceptor::IPV6_CONNECTION,
                        portNumberHttp, false);
                }
                if (addIP4Acceptor)
                {
                    _cimServer->addAcceptor(HTTPAcceptor::IPV4_CONNECTION,
                        portNumberHttp, false);
                }
            }
            else // Restricted listening
            {
                _restrictListening(
                    configManager, listenOn, portNumberHttp, false);
            }

            // The port number is converted to a string to avoid the
            //  addition of localized characters (e.g., "5,988").
            char scratchBuffer[22];
            Uint32 n;
            const char * portNumberHttpStr = Uint32ToString(
                scratchBuffer, portNumberHttp, n);
            MessageLoaderParms parms(
                "src.Server.cimserver.LISTENING_ON_HTTP_PORT",
                "Listening on HTTP port $0.", portNumberHttpStr);
            Logger::put_l(
                Logger::STANDARD_LOG, System::CIMSERVER, Logger::INFORMATION,
                parms);
#if defined(PEGASUS_DEBUG)
            cout << MessageLoader::getMessage(parms) << endl;
#endif
        }

        if (enableHttpsConnection)
        {
            Uint32 portNumberHttps = 0;
            String httpsPort = configManager->getCurrentValue("httpsPort");
            if (httpsPort.size() == 0)
            {
                //
                // Look up the WBEM-HTTPS port number
                //
                portNumberHttps = System::lookupPort(
                    WBEM_HTTPS_SERVICE_NAME, WBEM_DEFAULT_HTTPS_PORT);
                _initConfigProperty("httpsPort", portNumberHttps);
            }
            else
            {
                Uint64 longNumber;
                // use the current value which has been checked for validity at
                // load(fct. GetOptions), see DefaultPropertyOwner::isValid()
                StringConversion::decimalStringToUint64(
                    httpsPort.getCString(),
                    longNumber);
                portNumberHttps = longNumber & 0xffff;
            }

            String listenOn = configManager->getCurrentValue("listenAddress");
            if(String::equalNoCase(listenOn, "All"))
            {
                if (addIP6Acceptor)
                {
                    _cimServer->addAcceptor(HTTPAcceptor::IPV6_CONNECTION,
                        portNumberHttps, true);
                }
                if (addIP4Acceptor)
                {
                    _cimServer->addAcceptor(HTTPAcceptor::IPV4_CONNECTION,
                        portNumberHttps, true);
                }
            }
            else //Restricted
            {
                _restrictListening(
                    configManager, listenOn, portNumberHttps, true);
            }

            // The port number is converted to a string to avoid the
            //  addition of localized characters (e.g., "5,989").
            char scratchBuffer[22];
            Uint32 n;
            const char * portNumberHttpsStr = Uint32ToString(
                scratchBuffer, portNumberHttps, n);
            MessageLoaderParms parms(
                "src.Server.cimserver.LISTENING_ON_HTTPS_PORT",
                "Listening on HTTPS port $0.", portNumberHttpsStr);
            Logger::put_l(
                Logger::STANDARD_LOG, System::CIMSERVER, Logger::INFORMATION,
                parms);
#if defined(PEGASUS_DEBUG)
            cout << MessageLoader::getMessage(parms) << endl;
#endif
        }

#ifndef PEGASUS_DISABLE_LOCAL_DOMAIN_SOCKET
        {
            _cimServer->addAcceptor(HTTPAcceptor::LOCAL_CONNECTION, 0, false);

            MessageLoaderParms parms(
                "src.Server.cimserver.LISTENING_ON_LOCAL",
                "Listening on local connection socket.");
            Logger::put_l(
                Logger::STANDARD_LOG, System::CIMSERVER, Logger::INFORMATION,
                parms);
# if defined(PEGASUS_DEBUG)
            cout << MessageLoader::getMessage(parms) << endl;
# endif
        }
#endif

        _cimServer->bind();

        // notify parent process (if there is a parent process) to terminate
        // so user knows that there is cimserver ready to serve CIM requests.
        if (daemonOption)
        {
            _cimServerProcess->notify_parent(0);
        }

#if defined(PEGASUS_DEBUG)
        cout << "Started. " << endl;
#endif

        // Put server started message to the logger
        Logger::put_l(Logger::STANDARD_LOG, System::CIMSERVER,
            Logger::INFORMATION,
            MessageLoaderParms(
                "src.Server.cimserver.STARTED_VERSION",
                "Started $0 version $1.",
                _cimServerProcess->getProductName(),
                _cimServerProcess->getCompleteVersion()));

#if defined(PEGASUS_OS_TYPE_UNIX) && !defined(PEGASUS_OS_ZOS)
        if (daemonOption && !debugOutputOption)
        {
            // Direct standard input, output, and error to /dev/null,
            // since we are running as a daemon.
            close(STDIN_FILENO);
            open("/dev/null", O_RDONLY);
            close(STDOUT_FILENO);
            open("/dev/null", O_RDWR);
            close(STDERR_FILENO);
            open("/dev/null", O_RDWR);
        }
#endif
    }
    catch (Exception& e)
    {
        MessageLoaderParms parms("src.Server.cimserver.SERVER_NOT_STARTED",
            "cimserver not started: $0", e.getMessage());
        Logger::put_l(Logger::ERROR_LOG, System::CIMSERVER, Logger::SEVERE,
            parms);
        cerr << MessageLoader::getMessage(parms) << endl;

        deleteCIMServer();

        //
        // notify parent process (if there is a parent process) to terminate
        //
        if (daemonOption)
            _cimServerProcess->notify_parent(1);

        return 1;
    }

    // Run the main CIM Server loop

    try
    {
#if defined(PEGASUS_OS_ZOS)

        // ARM is a z/OS internal restart facility.
        // This is a z/OS specific change.

        // Instatiating the automatic restart manager for zOS
        ARM_zOS automaticRestartManager;

        // register to zOS ARM
        automaticRestartManager.Register();

#endif

#ifdef PEGASUS_ENABLE_SLP
        _cimServer->startSLPProvider();
#endif
        _cimServer->initComplete();

        //
        // Loop to call CIMServer's runForever() method until CIMServer
        // has been shutdown
        //
        while (!_cimServer->terminated())
        {
            _cimServer->runForever();
        }

        //
        // normal termination
        //

#if defined(PEGASUS_OS_ZOS)

        // ARM is a z/OS internal restart facility.
        // This is a z/OS specific change.

        // register to zOS ARM
        automaticRestartManager.DeRegister();

#endif

        // Put server shutdown message to the logger
        Logger::put_l(
            Logger::STANDARD_LOG, System::CIMSERVER, Logger::INFORMATION,
            MessageLoaderParms(
                "src.Server.cimserver.STOPPED",
                "$0 stopped.", _cimServerProcess->getProductName()));
    }
    catch (Exception& e)
    {
        MessageLoaderParms parms(
            "src.Server.cimserver.ERROR",
            "Error: $0",
            e.getMessage());
        Logger::put_l(Logger::STANDARD_LOG, System::CIMSERVER, Logger::WARNING,
            parms);
        cerr << MessageLoader::getMessage(parms) << endl;

    //delete the start up lock file
#if defined(PEGASUS_OS_TYPE_UNIX)
    System::removeFile(startupLockFileName);
#endif
        deleteCIMServer();
        return 1;
    }

    //delete the start up lock file
#if defined(PEGASUS_OS_TYPE_UNIX)
    System::removeFile(startupLockFileName);
#endif
    deleteCIMServer();
    return 0;
}
