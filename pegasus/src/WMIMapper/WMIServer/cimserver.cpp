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
//%////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////
//
// Notes on deamon operation (Unix) and service operation (Win 32):
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
// The daemon config property has no effect on windows operation.
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
#include <iostream>
#include <Pegasus/Common/PegasusAssert.h>
#include <fstream>
#include <Pegasus/Common/FileSystem.h>
#include <Pegasus/Common/PegasusVersion.h>
#include <Pegasus/Common/Logger.h>
#include <Pegasus/Common/System.h>
#include <Pegasus/Common/Tracer.h>
#include <Pegasus/Common/LanguageParser.h>
#include <Pegasus/Config/ConfigManager.h>
#include <Pegasus/Client/CIMClient.h>
#include <Pegasus/Server/CIMServer.h>
#include <Service/ServerProcess.h>
#include <Service/ServerShutdownClient.h>
#include <Service/ServerRunStatus.h>

PEGASUS_USING_PEGASUS;
PEGASUS_USING_STD;

#define PEGASUS_PROCESS_NAME "WMI Mapper"

//Windows service variables are not defined elsewhere in the product
//enable ability to override these
#ifndef PEGASUS_SERVICE_NAME
#define PEGASUS_SERVICE_NAME "Pegasus WMI Mapper";
#endif
#ifndef PEGASUS_SERVICE_DESCRIPTION
#define PEGASUS_SERVICE_DESCRIPTION "Pegasus WMI Mapper Service";
#endif

class CIMServerProcess : public ServerProcess
{
public:

    CIMServerProcess(void)
    {
        cimserver_set_process(this);
    }

    virtual ~CIMServerProcess(void)
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

    void cimserver_stop(void);
};

ServerRunStatus _serverRunStatus(
    PEGASUS_PROCESS_NAME, PEGASUS_CIMSERVER_START_FILE);
AutoPtr<CIMServerProcess> _cimServerProcess(new CIMServerProcess());
static CIMServer* _cimServer = 0;

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

static const char   LONG_HELP []  = "help";

static const char   LONG_VERSION []  = "version";

#if defined(PEGASUS_OS_HPUX)
static const char OPTION_BINDVERBOSE = 'X';
#endif

/** GetOptions function - This function defines the Options Table
    and sets up the options from that table using the config manager.

    Some possible exceptions:  NoSuchFile, FileNotReadable, CannotRenameFile,
    ConfigFileSyntaxError, UnrecognizedConfigProperty, InvalidPropertyValue,
    CannotOpenFile.
*/
void GetOptions(
    ConfigManager* cm,
    int& argc,
    char** argv)
{
    cm->mergeConfigFiles();
    cm->mergeCommandLine(argc, argv);
}

/* PrintHelp - This is temporary until we expand the options manager to allow
   options help to be defined with the OptionRow entries and presented from
   those entries.
*/
void PrintHelp(const char* arg0)
{
    String usage = String (USAGE);
    usage.append (COMMAND_NAME);
    usage.append (" [ [ options ] | [ configProperty=value, ... ] ]\n");
    usage.append ("  options\n");
    usage.append("    -v, --version   - displays CIM Server version number\n");
    usage.append ("    -h, --help      - prints this help message\n");
    usage.append ("    -s              - shuts down CIM Server\n");
#if !defined(PEGASUS_USE_RELEASE_DIRS)
    usage.append ("    -D [home]       - sets pegasus home directory\n");
#endif
#if defined(PEGASUS_OS_TYPE_WINDOWS)
    usage.append ("    -install [name] - installs pegasus as a Windows "
        "Service\n");
    usage.append ("                      [name] is optional and overrides "
        "the\n");
    usage.append ("                      default CIM Server Service Name\n");
    usage.append ("    -remove [name]  - removes pegasus as a Windows "
        "Service\n");
    usage.append ("                      [name] is optional and overrides "
        "the\n");
    usage.append ("                      default CIM Server Service Name\n");
    usage.append ("    -start [name]   - starts pegasus as a Windows "
        "Service\n");
    usage.append ("                      [name] is optional and overrides "
        "the\n");
    usage.append ("                      default CIM Server Service Name\n");
    usage.append ("    -stop [name]    - stops pegasus as a Windows "
        "Service\n");
    usage.append ("                      [name] is optional and overrides "
        "the\n");
    usage.append ("                      default CIM Server Service Name\n\n");
#endif
    usage.append ("  configProperty=value\n");
    usage.append ("                    - sets CIM Server configuration "
        "property\n");

    cout << endl;
    cout << _cimServerProcess->getProductName() << " " <<
        _cimServerProcess->getVersion() << endl;
    cout << endl;

#if defined(PEGASUS_OS_TYPE_WINDOWS)
    MessageLoaderParms parms("src.Server.cimserver.MENU.WINDOWS", usage);
#elif defined(PEGASUS_USE_RELEASE_DIRS)
    MessageLoaderParms parms(
        "src.Server.cimserver.MENU.HPUXLINUXIA64GNU", usage);
#else
    MessageLoaderParms parms("src.Server.cimserver.MENU.STANDARD", usage);
#endif
    cout << MessageLoader::getMessage(parms) << endl;
}

// This needs to be called at various points in the code depending on the
// platform and error conditions. We need to delete the _cimServer
// reference on exit in order for the destructors to get called.
void deleteCIMServer()
{
    delete _cimServer;
    _cimServer = 0;
}

// l10n
//
// Dummy function for the Thread object associated with the initial thread.
// Since the initial thread is used to process CIM requests, this is
// needed to localize the exceptions thrown during CIM request processing.
// Note: This function should never be called!
//
ThreadReturnType PEGASUS_THREAD_CDECL dummyThreadFunc(void *parm)
{
   return((ThreadReturnType)0);
}


/////////////////////////////////////////////////////////////////////////
//  MAIN
//////////////////////////////////////////////////////////////////////////
int main(int argc, char** argv)
{
    String pegasusHome;
    Boolean shutdownOption = false;
    Boolean debugOutputOption = false;

//l10n
// Set Message loading to process locale
MessageLoader::_useProcessLocale = true;
//l10n

//l10n
#if defined(PEGASUS_OS_AIX) && defined(PEGASUS_HAS_MESSAGES)
setlocale(LC_ALL, "");
#endif

#ifndef PEGASUS_OS_TYPE_WINDOWS
    //
    // Get environment variables:
    //
  #if defined(PEGASUS_OS_AIX) && defined(PEGASUS_USE_RELEASE_DIRS)
    pegasusHome = AIX_RELEASE_PEGASUS_HOME;
  #elif !defined(PEGASUS_USE_RELEASE_DIRS)
    const char* tmp = getenv("PEGASUS_HOME");

    if (tmp)
    {
        pegasusHome = tmp;
    }
  #endif

    FileSystem::translateSlashes(pegasusHome);
#else

  // windows only
  //setHome(pegasusHome);
  pegasusHome = _cimServerProcess->getHome();
#endif

        // Get help, version, and shutdown options

        for (int i = 1; i < argc; )
        {
            const char* arg = argv[i];
            if(String::equal(arg,"--help"))
            {
                    PrintHelp(argv[0]);
                    exit(0);
            }
            else if(String::equal(arg,"--version"))
            {
                cout << _cimServerProcess->getVersion() << endl;
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
                    cout << _cimServerProcess->getVersion() << endl;
                    exit(0);
                }
                //
                // Check to see if user asked for help (-h option):
                //
                else if (*option == OPTION_HELP &&
                        (strlen(option) == 1))
                {
                    PrintHelp(argv[0]);
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
                        exit(0);
                    }

                    memmove(&argv[i], &argv[i + 2],
                        (argc-i-1) * sizeof(char*));
                    argc -= 2;
                }
#endif
#if defined(PEGASUS_OS_HPUX)
                //
                // Check to see if user asked for the version (-X option):
                //
                if (*option == OPTION_BINDVERBOSE &&
                        (strlen(option) == 1))
                {
                    System::bindVerbose = true;

                    MessageLoaderParms parms(
                        "src.Server.cimserver.UNSUPPORTED_DEBUG_OPTION",
                        "Unsupported debug option, BIND_VERBOSE, enabled.");
                    cout << MessageLoader::getMessage(parms) << endl;
                    // remove the option from the command line
                    memmove(&argv[i], &argv[i + 1], (argc-i) * sizeof(char*));
                    argc--;
                }
#endif
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
                        exit(0);
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
    // Do the plaform specific run
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
// Perhpas further extract out common stuff and put into main(), put
// daemon stuff into platform specific platform_run(), etc.
// Note: make sure to not put error handling stuff that platform
// specific runs may need to deal with bettter (instead of exit(), etc).
//

int CIMServerProcess::cimserver_run(
    int argc,
    char** argv,
    Boolean shutdownOption,
    Boolean debugOutputOption)
{
    String logsDirectory;
    Boolean daemonOption = false;

    //
    // Get an instance of the Config Manager.
    //
    ConfigManager* configManager = ConfigManager::getInstance();
    configManager->useConfigFiles = true;

    //
    // Get options (from command line and from configuration file); this
    // removes corresponding options and their arguments from the command
    // line.
    //
    try
    {
        GetOptions(configManager, argc, argv);
    }
    catch (Exception& e)
    {
        MessageLoaderParms parms("src.Server.cimserver.SERVER_NOT_STARTED",
            "cimserver not started: $0", e.getMessage());

        Logger::put_l(Logger::ERROR_LOG, System::CIMSERVER, Logger::SEVERE,
            parms);

        PEGASUS_STD(cerr) << argv[0] << ": " <<
            MessageLoader::getMessage(parms) << PEGASUS_STD(endl);

        return(1);
    }

// l10n
    // Set the home directory, msg sub-dir, into the MessageLoader.
    // This will be the default directory where the resource bundles
    // are found.
    MessageLoader::setPegasusMsgHome(ConfigManager::getHomedPath(
        ConfigManager::getInstance()->getCurrentValue("messageDir")));

    Boolean enableHttpConnection = ConfigManager::parseBooleanValue(
        configManager->getCurrentValue("enableHttpConnection"));
    Boolean enableHttpsConnection = ConfigManager::parseBooleanValue(
        configManager->getCurrentValue("enableHttpsConnection"));

    // Make sure at least one connection is enabled
#ifdef PEGASUS_DISABLE_LOCAL_DOMAIN_SOCKET
    if (!enableHttpConnection && !enableHttpsConnection)
    {
        MessageLoaderParms parms(
            "src.Server.cimserver.HTTP_NOT_ENABLED_SERVER_NOT_STARTING",
            "Neither HTTP nor HTTPS connection is enabled.");
        Logger::put_l(Logger::STANDARD_LOG, System::CIMSERVER, Logger::WARNING,
            parms);
        cerr << MessageLoader::getMessage(parms) << endl;
        return 1;
    }
#endif

    try
    {
        //
        // Check to see if we should Pegasus as a daemon
        //

        daemonOption = ConfigManager::parseBooleanValue(
            configManager->getCurrentValue("daemon"));

        logsDirectory =
        ConfigManager::getHomedPath(configManager->getCurrentValue("logdir"));

        // Set up the Logger. This does not open the logs
        // Might be more logical to clean before set.
        // ATTN: Need tool to completely disable logging.

#if !defined(PEGASUS_OS_HPUX) && !defined(PEGASUS_PLATFORM_LINUX_IA64_GNU)
        Logger::setHomeDirectory(logsDirectory);
#endif

        //
        // Check to see if we need to shutdown CIMOM
        //
        if (shutdownOption)
        {
            String configTimeout =
                configManager->getCurrentValue("shutdownTimeout");
            Uint32 timeoutValue = strtol(
                configTimeout.getCString(), (char **)0, 10);

            ServerShutdownClient serverShutdownClient(&_serverRunStatus);
            serverShutdownClient.shutdown(timeoutValue);

            MessageLoaderParms parms(
                "src.Server.cimserver.SERVER_STOPPED",
                "CIM Server stopped.");

            cout << MessageLoader::getMessage(parms) << endl;
            return(0);
        }

#if defined(PEGASUS_DEBUG) && !defined(PEGASUS_USE_SYSLOGS)
        // Leave this in until people get familiar with the logs.
        MessageLoaderParms parms("src.Server.cimserver.LOGS_DIRECTORY",
                                 "Logs Directory = ");
        cout << MessageLoader::getMessage(parms) << logsDirectory << endl;
#endif
    }
    catch (UnrecognizedConfigProperty& e)
    {
    //l10n
    //cout << "Error: " << e.getMessage() << endl;
    MessageLoaderParms parms("src.Server.cimserver.ERROR",
                             "Error: $0",
                             e.getMessage());
    cout << MessageLoader::getMessage(parms) << endl;
    }

    // Bug 2148 - Here is the order of operations for determining the server
    // HTTP and HTTPS ports.
    // 1) If the user explicitly specified a port, use it.
    // 2) If the user did not specify a port, get the port from the
    //    services file.
    // 3) If no value is specified in the services file, use the IANA WBEM
    //    default port.
    // Note that 2 and 3 are done within the System::lookupPort method
    // An empty string from the ConfigManager implies that the user did
    // not specify a port.

    Uint32 portNumberHttps=0;
    Uint32 portNumberHttp=0;
    Uint32 portNumberExportHttps=0;

    if (enableHttpsConnection)
    {
        String httpsPort = configManager->getCurrentValue("httpsPort");
        if (httpsPort == String::EMPTY)
        {
            //
            // Look up the WBEM-HTTPS port number
            //
            portNumberHttps = System::lookupPort(WBEM_HTTPS_SERVICE_NAME,
                WBEM_DEFAULT_HTTPS_PORT);

        } else
        {
            //
            // user-specified
            //
            CString portString = httpsPort.getCString();
            char* end = 0;
            portNumberHttps = strtol(portString, &end, 10);
            if(!(end != 0 && *end == '\0'))
            {
                InvalidPropertyValue e("httpsPort", httpsPort);
                cerr << e.getMessage() << endl;
                exit(1);
            }
        }
    }

    if (enableHttpConnection)
    {
        String httpPort = configManager->getCurrentValue("httpPort");
        if (httpPort == String::EMPTY)
        {
            //
            // Look up the WBEM-HTTP port number
            //
            portNumberHttp = System::lookupPort(WBEM_HTTP_SERVICE_NAME,
                WBEM_DEFAULT_HTTP_PORT);

        } else
        {
            //
            // user-specified
            //
            CString portString = httpPort.getCString();
            char* end = 0;
            portNumberHttp = strtol(portString, &end, 10);
            if(!(end != 0 && *end == '\0'))
            {
                InvalidPropertyValue e("httpPort", httpPort);
                cerr << e.getMessage() << endl;
                exit(1);
            }
        }
    }

#if defined(PEGASUS_DEBUG)
    // Put out startup up message.
    cout << _cimServerProcess->getProductName() << " " <<
        _cimServerProcess->getVersion() << endl;
#endif

    // reset message loading to NON-process locale
    MessageLoader::_useProcessLocale = false;

    // Get the parent's PID before forking
    _serverRunStatus.setParentPid(System::getPID());

    // do we need to run as a daemon ?
    if (daemonOption)
    {
        if(-1 == _cimServerProcess->cimserver_fork())
    {
        return(-1);
    }
    }

// l10n
    // Now we are after the fork...
    // Create a dummy Thread object that can be used to store the
    // AcceptLanguageList object for CIM requests that are serviced
    // by this thread (initial thread of server).  Need to do this
    // because this thread is not in a ThreadPool, but is used
    // to service CIM requests.
    // The run function for the dummy Thread should never be called,
    Thread *dummyInitialThread = new Thread(dummyThreadFunc, NULL, false);
    Thread::setCurrent(dummyInitialThread);
    try
    {
         Thread::setLanguages(LanguageParser::getDefaultAcceptLanguages());
    }
    catch(InvalidAcceptLanguageHeader& e)
    {
          Logger::put_l(Logger::ERROR_LOG, System::CIMSERVER, Logger::SEVERE,
              MessageLoaderParms(
                  "src.Server.cimserver.FAILED_TO_SET_PROCESS_LOCALE",
                  "Could not convert the system process locale into "
                        "a valid AcceptLanguage format."));
          Logger::put(Logger::ERROR_LOG, System::CIMSERVER, Logger::SEVERE,
                             e.getMessage());
    }

#if defined(PEGASUS_OS_HPUX) || defined(PEGASUS_PLATFORM_LINUX_GENERIC_GNU) \
|| defined(PEGASUS_OS_ZOS) || defined(PEGASUS_OS_AIX) \
|| defined(PEGASUS_OS_SOLARIS) || defined (PEGASUS_OS_VMS)
    umask(S_IWGRP|S_IWOTH);

    //
    // check if CIMServer is already running
    // if CIMServer is already running, print message and
    // notify parent process (if there is a parent process) to terminate
    //
    if (_serverRunStatus.isServerRunning())
    {
        MessageLoaderParms parms(
            "src.Server.cimserver.UNABLE_TO_START_SERVER_ALREADY_RUNNING",
            "Unable to start CIMServer.\nCIMServer is already running.");
        PEGASUS_STD(cerr) << MessageLoader::getMessage(parms) <<
            PEGASUS_STD(endl);

        //
        // notify parent process (if there is a parent process) to terminate
        //
        if (daemonOption)
        {
            _cimServerProcess->notify_parent(1);
        }

        return 1;
    }

#endif

    // try loop to bind the address, and run the server
    try
    {
        _cimServer = new CIMServer();


        if (enableHttpConnection)
        {
#ifdef PEGASUS_ENABLE_IPV6
            _cimServer->addAcceptor(HTTPAcceptor::IPV6_CONNECTION,
                portNumberHttp, false);
#endif

#if !defined (PEGASUS_ENABLE_IPV6) || defined (PEGASUS_OS_TYPE_WINDOWS)
            _cimServer->addAcceptor(HTTPAcceptor::IPV4_CONNECTION,
                portNumberHttp, false);
#endif

            Logger::put_l(
                Logger::STANDARD_LOG, System::CIMSERVER, Logger::INFORMATION,
                MessageLoaderParms(
                    "src.Server.cimserver.LISTENING_ON_HTTP_PORT",
                    "Listening on HTTP port $0.", portNumberHttp));
        }

        if (enableHttpsConnection)
        {
#ifdef PEGASUS_ENABLE_IPV6
            _cimServer->addAcceptor(HTTPAcceptor::IPV6_CONNECTION,
                portNumberHttps, true);
#endif

#if !defined (PEGASUS_ENABLE_IPV6) || defined (PEGASUS_OS_TYPE_WINDOWS)
            _cimServer->addAcceptor(HTTPAcceptor::IPV4_CONNECTION,
                portNumberHttps, true);
#endif

            Logger::put_l(
                Logger::STANDARD_LOG, System::CIMSERVER, Logger::INFORMATION,
                MessageLoaderParms(
                    "src.Server.cimserver.LISTENING_ON_HTTPS_PORT",
                    "Listening on HTTPS port $0.", portNumberHttps));
        }

#ifndef PEGASUS_DISABLE_LOCAL_DOMAIN_SOCKET
        _cimServer->addAcceptor(HTTPAcceptor::LOCAL_CONNECTION, 0, false);

        Logger::put_l(
            Logger::STANDARD_LOG, System::CIMSERVER, Logger::INFORMATION,
            MessageLoaderParms(
                "src.Server.cimserver.LISTENING_ON_LOCAL",
                "Listening on local connection socket."));
#endif

#if defined(PEGASUS_DEBUG)
        if (enableHttpConnection)
        {
            MessageLoaderParms parms(
                "src.Server.cimserver.LISTENING_ON_HTTP_PORT",
                "Listening on HTTP port $0.", portNumberHttp);
            cout << MessageLoader::getMessage(parms) << endl;
        }
        if (enableHttpsConnection)
        {
            MessageLoaderParms parms(
                "src.Server.cimserver.LISTENING_ON_HTTPS_PORT",
                "Listening on HTTPS port $0.", portNumberHttps);
            cout << MessageLoader::getMessage(parms) << endl;
        }

# ifndef PEGASUS_DISABLE_LOCAL_DOMAIN_SOCKET
        MessageLoaderParms parms(
            "src.Server.cimserver.LISTENING_ON_LOCAL",
            "Listening on local connection socket.");
        cout << MessageLoader::getMessage(parms) << endl;
# endif
#endif

        // bind throws an exception if the bind fails
        _cimServer->bind();

        // notify parent process (if there is a parent process) to terminate
        // so user knows that there is cimserver ready to serve CIM requests.
        if (daemonOption)
        {
            _cimServerProcess->notify_parent(0);
        }

#if defined(PEGASUS_OS_HPUX) || \
    defined(PEGASUS_OS_LINUX) || \
    defined(PEGASUS_OS_ZOS) || \
    defined(PEGASUS_OS_AIX) || \
    defined(PEGASUS_OS_SOLARIS) || \
    defined(PEGASUS_OS_VMS)
        //
        // create a file to indicate that the cimserver has started and
        // save the process id of the cimserver process in the file
        //
        _serverRunStatus.setServerRunning();
#endif

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
                _cimServerProcess->getVersion()));

        //
        // Loop to call CIMServer's runForever() method until CIMServer
        // has been shutdown
        //
    while( !_cimServer->terminated() )
    {

      _cimServer->runForever();

    }

        //
        // normal termination
        //

        // Put server shutdown message to the logger
        Logger::put_l(Logger::STANDARD_LOG, System::CIMSERVER,
            Logger::INFORMATION,
            MessageLoaderParms(
                "src.Server.cimserver.STOPPED",
                "$0 stopped.", _cimServerProcess->getProductName()));
    }
    catch (Exception& e)
    {
        MessageLoaderParms parms("src.Server.cimserver.SERVER_NOT_STARTED",
            "cimserver not started: $0", e.getMessage());
        Logger::put_l(Logger::ERROR_LOG, System::CIMSERVER, Logger::SEVERE,
            parms);
        cerr << MessageLoader::getMessage(parms) << endl;

        //
        // notify parent process (if there is a parent process) to terminate
        //
        if (daemonOption)
                _cimServerProcess->notify_parent(1);

        deleteCIMServer();
        return 1;
    }

    deleteCIMServer();
    return 0;
}

