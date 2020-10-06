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

#include <windows.h>
#include <process.h>    /* _beginthread, _endthread */
#include <tchar.h>
#include <direct.h>
#include <Pegasus/Common/MessageLoader.h>
#include <Pegasus/Common/Thread.h>
#include <Pegasus/Common/Exception.h>
#include <Pegasus/Server/CIMServer.h>
#include <Service/ServerRunStatus.h>

#include "Service.cpp"

PEGASUS_USING_PEGASUS;
PEGASUS_USING_STD;

//-------------------------------------------------------------------------
// GLOBALS
//-------------------------------------------------------------------------
static Mutex _cimserverLock;
static ServerProcess* _server_proc = 0;
static bool _shutdown = false;
static Service pegasus_service;
static HANDLE pegasus_service_event = NULL;
static LPCSTR g_cimservice_key =
    TEXT("SYSTEM\\CurrentControlSet\\Services\\%s");
static LPCSTR g_cimservice_home = TEXT("home");
static int g_argc = 0;
static char **g_argv = 0;

//  Constants representing the command line options.
static const char OPTION_INSTALL[] = "install";
static const char OPTION_REMOVE[]  = "remove";
static const char OPTION_START[]   = "start";
static const char OPTION_STOP[]    = "stop";

//-------------------------------------------------------------------------
// PROTOTYPES
//-------------------------------------------------------------------------
int cimserver_windows_main(int flag, int argc, char **argv);
static bool _getRegInfo(const char *lpchKeyword, char *lpchRetValue);
static bool _setRegInfo(const char *lpchKeyword, const char *lpchValue);

//-------------------------------------------------------------------------
// NO-OPs for windows platform
//-------------------------------------------------------------------------
int ServerProcess::cimserver_fork() { return(0); }
void ServerProcess::notify_parent(int id) { return; }
void cimserver_exitRC(int rc) {}
int ServerProcess::cimserver_initialize() { return 0; }
int ServerProcess::cimserver_wait() { return 0; }


//-------------------------------------------------------------------------
// Helper for platform specific handling
//-------------------------------------------------------------------------

ServerProcess::ServerProcess()
{
    // Be sure to call cimserver_set_process right after instantiating this in
    // order for everything to work
}

ServerProcess::~ServerProcess()
{
}

void ServerProcess::cimserver_set_process(void* p)
{
    AutoMutex am(_cimserverLock);
    _server_proc = static_cast<ServerProcess *>(p);
    if (_server_proc && _shutdown)
        _server_proc->cimserver_stop();

 #ifndef PEGASUS_FLAVOR
    pegasus_service = Service(getProcessName());
 #else
    String serviceName(PEGASUS_FLAVOR);
    serviceName.append(Char16('-'));
    serviceName.append(getProcessName());
    pegasus_service = Service(serviceName.getCString());
 #endif
}

void signal_shutdown()
{
    AutoMutex am(_cimserverLock);
    _shutdown = true;
    if (_server_proc)
        _server_proc->cimserver_stop();
}

//-------------------------------------------------------------------------
// Run main server asynchronously
//-------------------------------------------------------------------------
static unsigned __stdcall cimserver_windows_thread(void* parm)
{
    int argc = 0;
    int rc = _server_proc->cimserver_run(g_argc, g_argv, false, false);
    SetEvent(pegasus_service_event);
    _endthreadex(rc);
    return rc;
}

//-------------------------------------------------------------------------
//  Windows NT Service Control Code
//-------------------------------------------------------------------------

//-------------------------------------------------------------------------
// START/STOP handler
//-------------------------------------------------------------------------
int cimserver_windows_main(int flag, int argc, char *argv[])
{
    switch(flag)
    {
    case Service::STARTUP_FLAG:
    {
        //
        // Start up main run in a separate thread and wait for it to finish.
        //

        unsigned threadid = 0;
        g_argc = argc;
        g_argv = argv;
        HANDLE hThread = (HANDLE)_beginthreadex(
            NULL, 0, cimserver_windows_thread, NULL, 0, &threadid);
        if (hThread == NULL)
            return 1;

        WaitForSingleObject(pegasus_service_event, INFINITE);

        //
        // Shutdown the cimserver.
        //

        signal_shutdown();

        //
        // Make sure we upate the SCM that our stop is pending.
        // Wait for the main run thread to exit.
        //

        DWORD dwCheckPoint = 1; // service code should have already started at 0

        while (WaitForSingleObject(hThread, 3000) == WAIT_TIMEOUT)
        {
            pegasus_service.report_status(
                SERVICE_STOP_PENDING, NO_ERROR, dwCheckPoint++, 5000);
        }

        CloseHandle(hThread);
        break;
    }
    case Service::SHUTDOWN_FLAG:
        SetEvent(pegasus_service_event);
        break;

    default:
        break;
    }

    return 0;
}


//-------------------------------------------------------------------------
// INSTALL
//-------------------------------------------------------------------------
bool cimserver_install_nt_service(char *service_name)
{
    Service::ReturnCode status = Service::SERVICE_RETURN_SUCCESS;
    char filename[_MAX_PATH] = {0};
    char displayname[_MAX_PATH] = {0};
    char descriptionname[_MAX_PATH] = {0};

    // If service name is specified, override default
    if (service_name == NULL)
    {
        strcpy(displayname, _server_proc->getExtendedName());
    }
    else
    {
        pegasus_service.SetServiceName(service_name);
        sprintf(displayname, "%s - %s",
            _server_proc->getExtendedName(),
            service_name);
    }

    strcpy(descriptionname, _server_proc->getDescription());

    if (0 != GetModuleFileName(NULL, filename, sizeof(filename)))
    {
        status =
            pegasus_service.Install(displayname, descriptionname, filename);

        // Upon success, set home in registry
        if (status == Service::SERVICE_RETURN_SUCCESS)
        {
            char pegasus_homepath[_MAX_PATH];
            System::extract_file_path(filename, pegasus_homepath);
            pegasus_homepath[strlen(pegasus_homepath)-1] = '\0';
            strcpy(filename, pegasus_homepath);
            System::extract_file_path(filename, pegasus_homepath);
            pegasus_homepath[strlen(pegasus_homepath)-1] = '\0';
            _setRegInfo(g_cimservice_home, pegasus_homepath);
        }
    }
    else
    {
        status = (Service::ReturnCode) GetLastError();
    }

    return (status == Service::SERVICE_RETURN_SUCCESS);
}

//-------------------------------------------------------------------------
// REMOVE
//-------------------------------------------------------------------------
bool cimserver_remove_nt_service(char* service_name)
{
    Service::ReturnCode status = Service::SERVICE_RETURN_SUCCESS;

    // If service name is specified, override default
    if (service_name != NULL)
    {
        pegasus_service.SetServiceName(service_name);
    }

    status = pegasus_service.Remove();

    return (status == Service::SERVICE_RETURN_SUCCESS);
}

//-------------------------------------------------------------------------
// START
//-------------------------------------------------------------------------
bool cimserver_start_nt_service(
    char* service_name,
    int num_args,
    char** service_args)
{
    Service::ReturnCode status = Service::SERVICE_RETURN_SUCCESS;

    // If service name is specified, override default
    if (service_name != NULL)
    {
        pegasus_service.SetServiceName(service_name);
    }

    if (num_args > 0 && service_args != NULL)
    {
        pegasus_service.SetServiceArgs(num_args, service_args);
    }

    status = pegasus_service.Start(5);

    return (status == Service::SERVICE_RETURN_SUCCESS);
}

//-------------------------------------------------------------------------
// STOP
//-------------------------------------------------------------------------
bool cimserver_stop_nt_service(char* service_name)
{
    Service::ReturnCode status = Service::SERVICE_RETURN_SUCCESS;

    // If service name is specified, override default
    if (service_name != NULL)
    {
        pegasus_service.SetServiceName(service_name);
    }

    status = pegasus_service.Stop(5);

    return (status == Service::SERVICE_RETURN_SUCCESS);
}

//-------------------------------------------------------------------------
// HELPER Utilities
//-------------------------------------------------------------------------
static bool _getRegInfo(
    const char* lpchKeyword,
    char* lpchRetValue)
{
    HKEY hKey;
    DWORD dw = _MAX_PATH;
    char subKey[_MAX_PATH] = {0};

    sprintf(subKey, g_cimservice_key, pegasus_service.GetServiceName());

    if ((RegOpenKeyEx(
             HKEY_LOCAL_MACHINE,
             subKey,
             0,
             KEY_READ,
             &hKey)) != ERROR_SUCCESS)
    {
        return false;
    }

    if ((RegQueryValueEx(
             hKey,
             lpchKeyword,
             NULL,
             NULL,
             (LPBYTE)lpchRetValue,
             &dw)) != ERROR_SUCCESS)
    {
        RegCloseKey(hKey);
        return false;
    }

    RegCloseKey(hKey);

    return true;
}

static bool _setRegInfo(
    const char* lpchKeyword,
    const char* lpchValue)
{
    HKEY hKey;
    DWORD dw = _MAX_PATH;
    char home_key[_MAX_PATH] = {0};
    char subKey[_MAX_PATH] = {0};

    if (lpchKeyword == NULL || lpchValue == NULL)
        return false;

    sprintf(subKey, g_cimservice_key, pegasus_service.GetServiceName());

    if ((RegCreateKeyEx(
             HKEY_LOCAL_MACHINE,
             subKey,
             0,
             NULL,
             0,
             KEY_ALL_ACCESS,
             NULL,
             &hKey,
             NULL) != ERROR_SUCCESS))
    {
        return false;
    }

    if ((RegSetValueEx(
             hKey,
             lpchKeyword,
             0,
             REG_SZ,
             (CONST BYTE *)lpchValue,
             (DWORD)(strlen(lpchValue)+1))) != ERROR_SUCCESS)
    {
        RegCloseKey(hKey);
        return false;
    }

    RegCloseKey(hKey);

    return true;
}

//void ServerProcess::setHome(const String& home)
String ServerProcess::getHome()
{
    String home;

    // Determine the absolute path to the running program
    char exe_pathname[_MAX_PATH] = {0};
    char home_pathname[_MAX_PATH] = {0};
    if (0 != GetModuleFileName(NULL, exe_pathname, sizeof(exe_pathname)))
    {
        // Pegasus home search rules:
        // - look in registry (if set)
        // - if not found, look in PEGASUS_HOME (if set)
        // - if not found, use exe directory minus one level

        bool found_reg = _getRegInfo("home", home_pathname);
        if (found_reg)
        {
            // Make sure home matches
            String current_home(home_pathname);
            String current_exe(exe_pathname);
            current_home.toLower();
            current_exe.toLower();

            Uint32 pos = current_exe.find(current_home);
            if (pos != PEG_NOT_FOUND)
            {
                home = home_pathname;
            }
            else
            {
                found_reg = false;
            }
        }
        if (found_reg == false)
        {
            const char* tmp = getenv("PEGASUS_HOME");
            if (tmp)
            {
                home = tmp;
            }
            else
            {
                // ASSUMPTION: At a minimum, the cimserver program is running
                // from a "bin" directory
                home = FileSystem::extractFilePath(exe_pathname);
                home.remove(home.size()-1, 1);
                home = FileSystem::extractFilePath(home);
                home.remove(home.size()-1, 1);
            }
        }
    }
    return home;
}

//
// Our console control handler
//

static BOOL WINAPI ControlHandler(DWORD dwCtrlType)
{
    switch (dwCtrlType)
    {
    case CTRL_BREAK_EVENT:  // use Ctrl+C or Ctrl+Break to simulate
    case CTRL_C_EVENT:      // SERVICE_CONTROL_STOP in debug mode
    {
        signal_shutdown();
        return TRUE;
    }
    }
    return FALSE;
}

//
// Platform specific run
//

int ServerProcess::platform_run(
    int argc,
    char** argv,
    Boolean shutdownOption,
    Boolean debugOutputOption)
{
    //
    // Check for my command line options
    //

    for (int i = 1; i < argc; )
    {
        const char* arg = argv[i];

        // Check for -option
        if (*arg == '-')
        {
            // Get the option
            const char* option = arg + 1;

            if (strcmp(option, OPTION_INSTALL) == 0)
            {
                //
                // Install as a NT service
                //
                char *opt_arg = NULL;
                if (i+1 < argc)
                {
                    opt_arg = argv[i+1];

                }
                if (cimserver_install_nt_service(opt_arg))
                {
                    //l10n
                    //cout << "\nPegasus installed as NT Service";
                    MessageLoaderParms parms(
                        "src.Server.cimserver.INSTALLED_NT_SERVICE",
                        "\nPegasus installed as a Windows service");
                    cout << MessageLoader::getMessage(parms) << endl;
                    exit(0);
                }
                else
                {
                    exit(0);
                }
            }
            else if (strcmp(option, OPTION_REMOVE) == 0)
            {
                //
                // Remove Pegasus as an NT service
                //
                char *opt_arg = NULL;
                if (i+1 < argc)
                {
                    opt_arg = argv[i+1];
                }
                if (cimserver_remove_nt_service(opt_arg))
                {
                    //l10n
                    //cout << "\nPegasus removed as NT Service";
                    MessageLoaderParms parms(
                        "src.Server.cimserver.REMOVED_NT_SERVICE",
                        "\nPegasus removed as a Windows service");
                    cout << MessageLoader::getMessage(parms) << endl;
                    exit(0);
                }
                else
                {
                    exit(0);
                }

            }
            else if (strcmp(option, OPTION_START) == 0)
            {
                //
                // Start as a NT service
                //
                char *opt_arg = NULL;
                int num_args = 0;
                if (i+1 < argc)
                {
                    opt_arg = argv[i+1];
                    num_args = argc - 3;
                }
                else
                {
                    num_args = argc - 2;
                }

                char **service_args = &argv[1];
                if (cimserver_start_nt_service(opt_arg, num_args, service_args))
                {
                    MessageLoaderParms parms(
                        "src.Server.cimserver.STARTED_NT_SERVICE",
                        "\nPegasus started as a Windows service");
                    cout << MessageLoader::getMessage(parms) << endl;
                    exit(0);
                }
                else
                {
                    exit(0);
                }
            }
            else if (strcmp(option, OPTION_STOP) == 0)
            {
                //
                // Stop as a NT service
                //
                char *opt_arg = NULL;
                if (i+1 < argc)
                {
                    opt_arg = argv[i+1];
                }
                if (cimserver_stop_nt_service(opt_arg))
                {
                    MessageLoaderParms parms(
                        "src.Server.cimserver.STOPPED_NT_SERVICE",
                        "\nPegasus stopped as a Windows service");
                    cout << MessageLoader::getMessage(parms) << endl;
                    exit(0);
                }
                else
                {
                    exit(0);
                }
            }
            else
                i++;
        }
        else
            i++;
    }

    //
    // Signal ourself as running
    //

    // NOTE: This object must persist for the life of the server process
    ServerRunStatus serverRunStatus(getProcessName(), 0);

    if (!shutdownOption)
    {
        serverRunStatus.setServerRunning();
    }

    //
    // Check if already running
    //
    // Hmm, when starting as a service, should we do this here (before
    // starting the control dispatcher)?  If we do then the SCM reports
    // a dumb message to the user.  If we don't, and it in the serviceProc
    // then the service will start up then die silently.
    //

    if (!shutdownOption && serverRunStatus.isServerRunning())
    {
        MessageLoaderParms parms(
            "src.Server.cimserver.UNABLE_TO_START_SERVER_ALREADY_RUNNING",
            "Unable to start CIMServer.\nCIMServer is already running.");
        Logger::put_l(
            Logger::ERROR_LOG, "CIMServer", Logger::SEVERE,
            parms);
        PEGASUS_STD(cerr) << MessageLoader::getMessage(parms) <<
            PEGASUS_STD(endl);
        return 1;
    }

    //
    // Check if running from a console window. If so then just run
    // as a console process.
    //

    char console_title[ _MAX_PATH ] = {0};
    if (GetConsoleTitle(console_title, _MAX_PATH) > 0)
    {
        SetConsoleCtrlHandler(ControlHandler, TRUE);

        return cimserver_run(argc, argv, shutdownOption, debugOutputOption);
    }

    //
    // Run as a service
    //

    pegasus_service_event = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (pegasus_service_event == NULL)
    {
        throw Exception(MessageLoaderParms(
            "src.Server.cimserver_windows.EVENT_CREATION_FAILED",
            "Event Creation Failed : $0.",
            PEGASUS_SYSTEM_ERRORMSG_NLS));
    }

    Service::ReturnCode status;
    status = pegasus_service.Run(cimserver_windows_main);

    if (status != Service::SERVICE_RETURN_SUCCESS)
    {
        // todo: put into localized messages when messages unfreezes.
        Logger::put_l(
            Logger::ERROR_LOG, "CIMServer", Logger::SEVERE,
            MessageLoaderParms(
                "src.Server.cimserver_windows.LISTENING_ON_HTTP_PORT",
                "Error during service run: code = $0.",
                status));
        return 1;
    }

    return 0;
}
