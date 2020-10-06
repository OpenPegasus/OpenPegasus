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
// Notes on deamon operation (Unix) and service operation (Win 32):
//
// To run pegasus listener as a daemon on Unix platforms:
//
// cimlistener
//
// To NOT run pegasus listener as a daemon on Unix platforms:
//
// cimlistener --nodaemon
//
// The daemon setting has no effect on windows operation.
//
// To shutdown pegasus listener, use the -s option:
//
// cimlistener -s
//
// To run pegasus listener as an NT service,
// there are FOUR  different possibilities:
//
// To INSTALL the Pegasus service,
//
// cimlistener -install
//
// To REMOVE the Pegasus service,
//
// cimlistener -remove
//
// To START the Pegasus service,
//
// net start cimlistener
// or
// cimlistener -start
//
// To STOP the Pegasus service,
//
// net stop cimlistener
// or
// cimlistener -stop
//
// Alternatively, you can use the windows service manager.
// Pegasus listener shows up
// in the service database as "Pegasus CIM Listener"
//
//////////////////////////////////////////////////////////////////////


#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/Constants.h>
#include <Pegasus/Common/PegasusAssert.h>
#include <Pegasus/Common/FileSystem.h>
#include <Pegasus/Common/Logger.h>
#include <Pegasus/Common/System.h>
#include <Pegasus/Common/Tracer.h>
#include <Pegasus/Common/Thread.h>
#include <Pegasus/Common/LanguageParser.h>
#include <Pegasus/Common/PegasusVersion.h>
#include <Pegasus/Common/Signal.h>
#include <Pegasus/DynListener/DynamicListener.h>
#include <Pegasus/DynListener/DynamicListenerConfig.h>
#include <Service/ServerProcess.h>
#include <Service/ServerRunStatus.h>
#include <Service/PidFile.h>
#include <Pegasus/Config/ConfigExceptions.h>

#if defined(PEGASUS_OS_TYPE_UNIX)
# include <unistd.h>
# include <sys/types.h>
# include <sys/stat.h>
# include <fcntl.h>
#endif

PEGASUS_USING_PEGASUS;
PEGASUS_USING_STD;

#define PEGASUS_LISTENER_PROCESS_NAME "cimlistener"

//Windows service variables are not defined elsewhere in the product
//enable ability to override these
#ifndef PEGASUS_LISTENER_PRODUCT_NAME
#define PEGASUS_LISTENER_PRODUCT_NAME "CIM Listener"
#endif
#ifndef PEGASUS_LISTENER_SERVICE_NAME
#define PEGASUS_LISTENER_SERVICE_NAME "Pegasus CIM Listener"
#endif
#ifndef PEGASUS_LISTENER_SERVICE_DESCRIPTION
#define PEGASUS_LISTENER_SERVICE_DESCRIPTION "Pegasus CIM Listener Service"
#endif

//The start file is created as soon the child forks and notifies the parent.
//This file is used to detect whether the cimserver is running and thus,
//it must exist until right before the child process exits.
#ifndef LISTENER_START_FILE
#define LISTENER_START_FILE "/tmp/cimlistener_start.conf"
#endif
//The stop file is created to signal the cimserver to shutdown.
//We cannot use the earlier file and delete it to "signal shutdown"
//because then another cimserver process could be started while we
//were still shutting down.  This is a workaround for the fact that
//signals are not implemented on all platforms.  Theoretically,
//we should be blocking until the calling process signalled the child
//to shutdown.
#ifndef LISTENER_STOP_FILE
#define LISTENER_STOP_FILE "/tmp/cimlistener_stop.conf"
#endif

class CIMListenerProcess : public ServerProcess
{
public:

    CIMListenerProcess(void)
    {
        cimserver_set_process(this);
    }

    virtual ~CIMListenerProcess(void)
    {
    }

    virtual const char* getProductName() const
    {
        return PEGASUS_LISTENER_PRODUCT_NAME;
    }

    virtual const char* getExtendedName() const
    {
        return PEGASUS_LISTENER_SERVICE_NAME;
    }

    virtual const char* getDescription() const
    {
        return PEGASUS_LISTENER_SERVICE_DESCRIPTION;
    }

    //defined in PegasusVersion.h
    virtual const char* getVersion() const
    {
        return PEGASUS_PRODUCT_VERSION;
    }

    virtual const char* getProcessName() const
    {
        return PEGASUS_LISTENER_PROCESS_NAME;
    }

    int cimserver_run(
        int argc,
        char** argv,
        Boolean shutdownOption,
        Boolean debugOutputOption);

    void cimserver_stop(void);
};

ServerRunStatus _serverRunStatus(
    PEGASUS_LISTENER_PROCESS_NAME, LISTENER_START_FILE);
AutoPtr<CIMListenerProcess> _cimListenerProcess(new CIMListenerProcess());
AutoPtr<DynamicListenerConfig> configManager(
                                 DynamicListenerConfig::getInstance());
static DynamicListener* _cimListener = 0;


//
//  The command name.
//
static const char COMMAND_NAME []    = "cimlistener";

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

static const char OPTION_NO_DAEMON [] = "--nodaemon";

static const char OPTION_DEBUGOUTPUT = 'X';

static const String PROPERTY_TIMEOUT = "shutdownTimeout";

static const String DEFAULT_CONFIG_FILE = "cimlistener.conf";


/* PrintHelp - This is temporary until we expand the options manager to allow
   options help to be defined with the OptionRow entries and presented from
   those entries.
*/
void PrintHelp(const char* arg0)
{
    String usage = String (USAGE);
    usage.append (COMMAND_NAME);
    usage.append (" [ [ options ] ]\n");
    usage.append ("  options\n");
    usage.append (
            "    -v, --version   - displays CIM Listener version number\n");
    usage.append ("    -h, --help      - prints this help message\n");
    usage.append ("    -s              - shuts down CIM Listener\n");
    usage.append ("    -D [home]       - sets pegasus home directory\n");
#if defined(PEGASUS_OS_TYPE_WINDOWS)
    usage.append (
            "    -install [name] - installs pegasus as a Windows Service\n");
    usage.append (
            "                      [name] is optional and overrides the\n");
    usage.append ("                      default CIM Listener Service Name\n");
    usage.append (
            "    -remove [name]  - removes pegasus as a Windows Service\n");
    usage.append (
            "                      [name] is optional and overrides the\n");
    usage.append ("                      default CIM Listener Service Name\n");
    usage.append (
            "    -start [name]   - starts pegasus as a Windows Service\n");
    usage.append (
            "                      [name] is optional and overrides the\n");
    usage.append ("                      default CIM Listener Service Name\n");
    usage.append (
            "    -stop [name]    - stops pegasus as a Windows Service\n");
    usage.append (
            "                      [name] is optional and overrides the\n");
    usage.append (
            "                      default CIM Listener Service Name\n\n");
#endif
#if !defined(PEGASUS_OS_TYPE_WINDOWS)
    usage.append ("    --nodaemon       - do NOT run as a daemon\n");
#endif

    cout << endl;
    cout << _cimListenerProcess->getProductName() << " "
         << _cimListenerProcess->getVersion() << endl;
    cout << endl;

//ATTN: Add menu items to Server bundle for listener
/*#if defined(PEGASUS_OS_TYPE_WINDOWS)
    MessageLoaderParms parms("src.Server.cimserver.MENU.WINDOWS", usage);
#elif defined(PEGASUS_OS_USE_RELEASE_DIRS)
    MessageLoaderParms parms(
      "src.Server.cimserver.MENU.HPUXLINUXIA64GNU", usage);
#else
    MessageLoaderParms parms("src.Server.cimserver.MENU.STANDARD", usage);
#endif
    cout << MessageLoader::getMessage(parms) << endl;
*/
    cout << usage;
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
#elif defined(PEGASUS_OS_PASE) && defined(PEGASUS_USE_RELEASE_DIRS)
    pegasusHome = PASE_PROD_HOME;
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
    char exeDir[MAX_PATH];
    HMODULE hExe = GetModuleHandle(NULL);
    GetModuleFileName(hExe, exeDir, sizeof(exeDir));
    *strrchr(exeDir, '\\') = '\0';
    pegasusHome = String(exeDir);

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
                cout << _cimListenerProcess->getVersion() << endl;
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
                    cout << _cimListenerProcess->getVersion() << endl;
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
                           "DynListener.cimlistener.DUPLICATE_SHUTDOWN_OPTION",
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
    DynamicListenerConfig::setListenerHome(pegasusHome);

    //
    // Do the plaform specific run
    //

    return _cimListenerProcess->platform_run(
        argc, argv, shutdownOption, debugOutputOption);
}

void CIMListenerProcess::cimserver_stop()
{
    _cimListener->stop();
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

int CIMListenerProcess::cimserver_run(
    int argc,
    char** argv,
    Boolean shutdownOption,
    Boolean debugOutputOption)
{
    String logsDirectory;
    String homeDir = configManager->getListenerHome();

    //
    // Get an instance of the Config Manager.
    //
    //configManager = DynamicListenerConfig::getInstance();

    //
    // Check to see if we should Pegasus as a daemon
    //
    Boolean daemonOption;
#if defined(PEGASUS_OS_TYPE_WINDOWS)
    daemonOption = false;
#else
    daemonOption = true;
    for (int i=1; i < argc; i++)
    {
        if (strcmp(argv[i], OPTION_NO_DAEMON) == 0)
        {
            daemonOption = false;
            break;
        }
    }
#endif

    //
    // Get options (from command line and from configuration file); this
    // removes corresponding options and their arguments from the command
    // line.
    //
    try
    {
        String configFilePath = homeDir + "/" + DEFAULT_CONFIG_FILE;
        FileSystem::translateSlashes(configFilePath);
        configManager->initOptions(configFilePath);
    }
    catch (Exception& e)
    {
        MessageLoaderParms parms("DynListener.cimlistener.LISTENER_NOT_STARTED",
                "CIM Listener not started: $0", e.getMessage());

        Logger::put_l(Logger::ERROR_LOG, System::CIMLISTENER, Logger::SEVERE,
            parms);

        PEGASUS_STD(cerr) << argv[0] << ": " << MessageLoader::getMessage(parms)
            << PEGASUS_STD(endl);

        return(1);
    }

// l10n
    // Set the home directory, msg sub-dir, into the MessageLoader.
    // This will be the default directory where the resource bundles
    // are found.
    String msgHome = homeDir + "/msg";
    FileSystem::translateSlashes(msgHome);
    MessageLoader::setPegasusMsgHome(msgHome);

    //
    // Check to see if we need to shutdown CIMOM
    //
    if (shutdownOption)
    {
        //gracefully exit
        //Uncomment the following line when signals are implemented on all
        //platforms.
        //The workaround is to use a file.
#ifdef PEGASUS_HAS_SIGNALS
        PidFile pidFile(LISTENER_START_FILE);
        PEGASUS_PID_T pid = (PEGASUS_PID_T) pidFile.getPid();

        if (pid == 0)
        {
           pidFile.remove();
           return (-1);
        }

        int rc = kill(pid, PEGASUS_SIGTERM);
        // check for success or failure of the kill operation
        if (rc == -1)
        {

            if (errno == EPERM)
            {
                MessageLoaderParms parms(
                  "DynListener.cimlistener.KILL_PERM_DENIED",
                  "Permission denied: Not permitted to"
                  "shutdown cimlistener process.");
                cout << MessageLoader::getMessage(parms) << endl;
            } else
            {
                MessageLoaderParms parms(
                    "DynListener.cimlistener.KILL_FAILED",
                    "Failed to shutdown cimlistener process. Error: \"$1\"",
                    strerror(errno));
                cout << MessageLoader::getMessage(parms) << endl;
            }
            return(rc);
        }

        //cimserver_kill(1);
#else
#if defined(PEGASUS_OS_HPUX) || defined(PEGASUS_OS_LINUX) \
        || defined(PEGASUS_OS_ZOS) \
        || defined(PEGASUS_OS_AIX) || defined(PEGASUS_OS_PASE) \
        || defined(PEGASUS_OS_SOLARIS) \
        || defined(PEGASUS_OS_VMS)

        // Check to see if the CIM Listener is running.
        // No need to stop if not running.
        if (_serverRunStatus.isServerRunning())
        {
            PidFile pidFile(LISTENER_STOP_FILE);
            pidFile.setPid(System::getPID());
        }
        else
        {
            printf("CIM Listener may not be running.\n");
            return(0);
        }
#endif

#endif //PEGASUS_HAS_SIGNALS

        //l10n
        //cout << "CIM Server stopped." << endl;
        MessageLoaderParms parms("DynListener.cimlistener.LISTENER_STOPPED",
                                 "CIM Listener stopped.");
        cout << MessageLoader::getMessage(parms) << endl;
        return(0);
    }

    //get config options.
    //note that the paths will be converted to homedPaths in the lookup calls.
    Uint32 listenerPort;
    Boolean httpsConnection;
    String sslKeyFilePath;
    String sslCertificateFilePath;
    String sslCipherSuite;
    Boolean sslCompatibility;
    String consumerDir;
    String consumerConfigDir;
    Boolean enableConsumerUnload;
    Uint32 consumerIdleTimeout;
    Uint32 shutdownTimeout;
    String traceFile;
    Uint32 traceLevel;
    String traceComponents;

    try
    {
    configManager->lookupIntegerValue("listenerPort", listenerPort);
    httpsConnection = configManager->isTrue("enableHttpsListenerConnection");
    configManager->lookupValue("sslKeyFilePath", sslKeyFilePath);
    configManager->lookupValue("sslCertificateFilePath",
                               sslCertificateFilePath);
    if(!configManager->lookupValue("sslCipherSuite",sslCipherSuite))
    {
        throw InvalidPropertyValue("sslCipherSuite",sslCipherSuite);
    }
    sslCompatibility = configManager->isTrue("sslBackwardCompatibility");
    configManager->lookupValue("consumerDir", consumerDir);
    configManager->lookupValue("consumerConfigDir", consumerConfigDir);
    enableConsumerUnload = configManager->isTrue("enableConsumerUnload");
    configManager->lookupIntegerValue("consumerIdleTimeout",
                                       consumerIdleTimeout);
    configManager->lookupIntegerValue("consumerIdleTimeout",
                                      consumerIdleTimeout);
    configManager->lookupIntegerValue("shutdownTimeout", shutdownTimeout);
    configManager->lookupValue("traceFilePath", traceFile);
    configManager->lookupIntegerValue("traceLevel", traceLevel);
    configManager->lookupValue("traceComponents", traceComponents);

    }
    catch (Exception& ex)
    {
        cout << ex.getMessage() << endl;
        exit(0);
    }

    //Check listener port validity
    //ATTN: Do we need this?
    /*CString portString = listenerPort.getCString();
    char* end = 0;
    Uint32 port = strtol(portString, &end, 10);
    if(!(end != 0 && *end == '\0'))
    {
        PEGASUS_STD(cerr) << "Bad HTTP/HTTPS Port Value" << PEGASUS_STD(endl);
        exit(1);
    }*/

    //Configure trace options
    if (traceLevel > 0)
    {
        Uint32 traceLevelArg = 0;

        switch (traceLevel)
        {
        case 1: traceLevelArg = Tracer::LEVEL1; break;
        case 2: traceLevelArg = Tracer::LEVEL2; break;
        case 3: traceLevelArg = Tracer::LEVEL3; break;
        case 4: traceLevelArg = Tracer::LEVEL4; break;
        default: break;
        }

        Tracer::setTraceFile((const char*)traceFile.getCString());
        Tracer::setTraceComponents(traceComponents);
        Tracer::setTraceLevel(traceLevelArg);
    }

#if defined(PEGASUS_DEBUG)
    {
        // Put out startup up message.
        cout << _cimListenerProcess->getProductName() << " "
             << _cimListenerProcess->getVersion() << endl;
        MessageLoaderParms parms("DynListener.cimlistener.STARTUP_MESSAGE",
            "CIM Listener built $0 $1\nCIM Listener starting...",
            __DATE__,
            __TIME__);
    }
#endif

//l10n
// reset message loading to NON-process locale
MessageLoader::_useProcessLocale = false;
//l10n

    // Get the parent's PID before forking
    _serverRunStatus.setParentPid(System::getPID());

    // do we need to run as a daemon ?
    if (daemonOption)
    {
        if(-1 == _cimListenerProcess->cimserver_fork())
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
          Logger::put_l(Logger::ERROR_LOG, System::CIMLISTENER, Logger::SEVERE,
              MessageLoaderParms(
                  "src.Server.cimserver.FAILED_TO_SET_PROCESS_LOCALE",
                  "Could not convert the system process"
                  "locale into a valid AcceptLanguage format."));
          Logger::put(Logger::ERROR_LOG, System::CIMLISTENER, Logger::SEVERE,
                             e.getMessage());
    }

#if defined(PEGASUS_OS_HPUX) || defined(PEGASUS_PLATFORM_LINUX_GENERIC_GNU) \
|| defined(PEGASUS_OS_ZOS) || defined(PEGASUS_OS_AIX) \
|| defined(PEGASUS_OS_PASE) \
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
            "DynListener.cimlistener.UNABLE_TO_START_LISTENER_ALREADY_RUNNING",
            "Unable to start CIM Listener.\nCIM Listener is already running.");
        PEGASUS_STD(cerr) << MessageLoader::getMessage(parms) <<
            PEGASUS_STD(endl);

        //
        // notify parent process (if there is a parent process) to terminate
        //
        if (daemonOption)
        {
            _cimListenerProcess->notify_parent(1);
        }

        return 1;
    }

#endif

    // try loop to bind the address, and run the server
    try
    {
#ifdef PEGASUS_HAS_SSL
        //create DynListener
        if (httpsConnection)
        {
            _cimListener = new DynamicListener(
                listenerPort,
                consumerDir,
                consumerConfigDir,
                true,
                sslKeyFilePath,
                sslCertificateFilePath,
                enableConsumerUnload,
                consumerIdleTimeout,
                shutdownTimeout,
                sslCipherSuite,
                sslCompatibility);
        }
        else
#endif
        {
            _cimListener = new DynamicListener(
                listenerPort,
                consumerDir,
                consumerConfigDir,
                enableConsumerUnload,
                consumerIdleTimeout,
                shutdownTimeout);
        }
        _cimListener->start();

        Logger::put_l(Logger::STANDARD_LOG,
            System::CIMLISTENER, Logger::INFORMATION,
            MessageLoaderParms(
                "DynListener.cimlistener.LISTENING_ON_PORT",
                "The CIM listener is listening on port $0.",
                listenerPort));


#if defined(PEGASUS_DEBUG)
        //Log startup options
        printf("Starting CIMListener with the following options\n");
        printf("\tlistenerPort %u\n", listenerPort);
        printf("\thttpsConnection %d\n", httpsConnection);
        printf("\tsslKeyFilePath %s\n",
                (const char*)sslKeyFilePath.getCString());
        printf("\tsslCertificateFilePath %s\n",
                (const char*)sslCertificateFilePath.getCString());
        printf("\tsslCipherSuite %s\n",
            (const char*)sslCipherSuite.getCString());
        printf("\tsslBackwardCompatibility %d\n",sslCompatibility);
        printf("\tconsumerDir %s\n", (const char*)consumerDir.getCString());
        printf("\tconsumerConfigDir %s\n",
                (const char*)consumerConfigDir.getCString());
        printf("\tenableConsumerUnload %d\n", enableConsumerUnload);
        printf("\tconsumerIdleTimeout %u\n", consumerIdleTimeout);
        printf("\tshutdownTimeout %u\n", shutdownTimeout);
        printf("\ttraceFilePath %s\n", (const char*)traceFile.getCString());
        printf("\ttraceLevel %u\n", traceLevel);
        printf("\ttraceComponents %s\n",
                (const char*)traceComponents.getCString());
        printf("\tMessage home is %s\n", (const char*)msgHome.getCString());
#endif

    // notify parent process (if there is a parent process) to terminate
        // so user knows that there is cimserver ready to serve CIM requests.
    if (daemonOption)
        _cimListenerProcess->notify_parent(0);

#if defined(PEGASUS_OS_HPUX) || defined(PEGASUS_OS_LINUX) || \
    defined(PEGASUS_OS_ZOS) || defined(PEGASUS_OS_AIX) || \
    defined(PEGASUS_OS_PASE) || \
    defined(PEGASUS_OS_SOLARIS) || defined(PEGASUS_OS_VMS)
        //
        // create a file to indicate that the cimserver has started and
        // save the process id of the cimserver process in the file
        //
        _serverRunStatus.setServerRunning();
#endif

//#if defined(PEGASUS_DEBUG)
    cout << "Started. " << endl;
//#endif

        // Put server started message to the logger
        Logger::put_l(Logger::STANDARD_LOG, System::CIMLISTENER,
            Logger::INFORMATION,
            MessageLoaderParms(
                "src.Server.cimserver.STARTED_VERSION",
                "Started $0 version $1.",
                _cimListenerProcess->getProductName(),
                _cimListenerProcess->getVersion()));

#if defined(PEGASUS_OS_TYPE_UNIX)
        if (daemonOption && !debugOutputOption)
        {
            // Direct standard input, output, and error to /dev/null,
            // since we are running as a daemon.
            close(0);
            open("/dev/null", O_RDONLY);
            close(1);
            open("/dev/null", O_RDWR);
            close(2);
            open("/dev/null", O_RDWR);
        }
#endif

#if !defined(PEGASUS_OS_TYPE_WINDOWS)

// if signals are defined, do not use old file creation mechanism
#ifndef PEGASUS_HAS_SIGNALS
#if defined(PEGASUS_DEBUG)
        printf("Blocking until shutdown signal\n");
#endif
        while (true)
        {
            if (FileSystem::exists(LISTENER_STOP_FILE))
            {
                break;
            }

            Threads::sleep(500);
        }

#if defined(PEGASUS_DEBUG)
        printf("Received signal to shutdown\n");
#endif
        FileSystem::removeFile(LISTENER_STOP_FILE);
        _cimListener->stop();
#else // defined(PEGASUS_HAS_SIGNALS)

        //Temporary workaround is to use a file, as specified above.
        //wait until signalled to terminate
        int sig = _cimListenerProcess->cimserver_wait();
#if defined(PEGASUS_DEBUG)
        printf("Returned from sigwait %d\n", sig);
#endif
        if ((sig == PEGASUS_SIGTERM) || (sig == PEGASUS_SIGHUP))
        {
#if defined(PEGASUS_DEBUG)
            printf("Graceful shutdown\n");
#endif
            _cimListener->stop();
        }
#endif
#else
        //ATTN: Implement cimserver_wait for windows
        //so we don't have to loop here
        //The listener is stopped in the cimserver_stop method
        //by the service control manager
        while (true)
        {
            if (!_cimListener->isAlive())
            {
                break;
            }

            Sleep(500);
        }
#endif

#if defined(PEGASUS_DEBUG)
    PEGASUS_STD(cout) << "Stopped\n";
#endif

        //
        // normal termination
        //

        // Put server shutdown message to the logger
        Logger::put_l(Logger::STANDARD_LOG, System::CIMLISTENER,
            Logger::INFORMATION,
            MessageLoaderParms(
                "src.Server.cimserver.STOPPED",
                "$0 stopped.", _cimListenerProcess->getProductName()));
    }
    catch (Exception& e)
    {
        Logger::put_l(
            Logger::STANDARD_LOG, System::CIMLISTENER, Logger::WARNING,
            MessageLoaderParms(
                "src.Server.cimserver.ERROR",
                "Error: $0", e.getMessage()));

        MessageLoaderParms parms("DynListener.cimlistener.ERROR",
            "CIM Listener error: $0");
        PEGASUS_STD(cerr) << MessageLoader::getMessage(parms) <<
            PEGASUS_STD(endl);

        //
        // notify parent process (if there is a parent process) to terminate
        //
        if (daemonOption)
                _cimListenerProcess->notify_parent(1);

        delete _cimListener;
        _cimListener = 0;

        return 1;
    }

    delete _cimListener;
    _cimListener = 0;

#if defined(PEGASUS_DEBUG)
    printf("Exiting child process\n");
#endif

    return 0;
}
