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

#include <time.h>
#include <stdio.h>
#include <windows.h>
#include <process.h>

#include "service.h"

/////////////////////////////////////////////////////////////////////////////
// G L O B A L S
/////////////////////////////////////////////////////////////////////////////
int Service::g_argc = 0;
char **Service::g_argv = NULL;
char *Service::g_service_name = NULL;
char *Service::g_event_source = NULL;
DWORD Service::g_flags = 0;
DWORD Service::g_current_state = 0;
SERVICE_STATUS_HANDLE Service::g_service_status_handle = 0;
SERVICE_MAIN_T Service::g_service_main = NULL;

/////////////////////////////////////////////////////////////////////////////
// P U B L I C
/////////////////////////////////////////////////////////////////////////////
Service::Service(void)
{
}

Service::Service(char *service_name)
{
    g_service_name = service_name;
}

Service::Service(char *service_name, char *event_source)
{
    g_event_source = event_source;
    g_service_name = service_name;
}

Service::~Service(void)
{
}

/////////////////////////////////////////////////////////////////////////////
// Method: Install
//
// Args:
//   display_name
//     The service's display name (hopefully more descriptive, will show
//     up in the Win32 Services MMC snapin as "Service Name").
//
//   exe_name:
//     The service's executable name (full path, please)
//
//   flags:
//     Reserved.  Currently unused.
//
// Description:
//     This function creates the service.
//
// NOTE: If the process is successfully launched as a Win32 service, this
//       function never returns, but calls exit() instead.
//
/////////////////////////////////////////////////////////////////////////////
Service::ReturnCode Service::Install(
    char  *display_name,
    char  *description,
    char  *exe_name)
{
    ReturnCode status = SERVICE_RETURN_SUCCESS;
    SC_HANDLE sch;

    if (g_service_name == NULL || display_name == NULL || exe_name == NULL)
    {
        return SERVICE_ERROR_NOT_FOUND; // SERVICE_ERROR_NOT_FOUND
    }
    else if 
        ((sch = OpenSCManager(NULL, NULL, SC_MANAGER_CREATE_SERVICE)) == NULL)
    {
        status = get_error(GetLastError(), "open");
    }
    else
    {
        SC_HANDLE service = CreateService(
            sch,                       // SCManager database
            g_service_name,            // name of service
            display_name,              // service name to display
            SERVICE_ALL_ACCESS,        // desired access
            SERVICE_WIN32_OWN_PROCESS, // service type
            SERVICE_DEMAND_START,      // start type
            SERVICE_ERROR_NORMAL,      // error control type
            exe_name,                  // service's binary
            NULL,                      // no load ordering group
            NULL,                      // no tag identifier
            NULL,                      // no dependencies
            NULL,                      // LocalSystem account
            NULL);                     // no password
 
        if (service == NULL) 
        {
            status = get_error(GetLastError(), "create");
            return status;
        }
        else 
        {
            change_service_description(service, description);
            CloseServiceHandle(service);
        }
  
        CloseServiceHandle(sch);
    }

    return status;
}

/////////////////////////////////////////////////////////////////////////////
// Method: Remove
//
// Description:
//     Removes the service.
//
/////////////////////////////////////////////////////////////////////////////
Service::ReturnCode Service::Remove(void)
{
    ReturnCode status = SERVICE_RETURN_SUCCESS;
    SC_HANDLE sch;

    if (g_service_name == NULL)
    {
        return SERVICE_ERROR_NOT_FOUND; /* SERVICE_ERROR_NOT_FOUND */
    }
    else if 
        ((sch = OpenSCManager(NULL, NULL, SC_MANAGER_CREATE_SERVICE)) == NULL)
    {
        status = get_error(GetLastError(), "open");
    }
    else
    {
        SC_HANDLE service = OpenService(sch, g_service_name, DELETE);
 
        if (service == NULL) 
        {
            status = get_error(GetLastError(), "open");
        }
        else 
        {
            if (!DeleteService(service))
            {
                status = get_error(GetLastError(), "remove");
            }

            CloseServiceHandle(service);
        }
  
        CloseServiceHandle(sch);
    }
    return status;
}

/////////////////////////////////////////////////////////////////////////////
// Method: Start
//
// Args:
//   wait_time:
//     The user supplied wait (~1 second per QueryServiceStatus() attempt)
//
// Description:
//     Attempt to start the service.
//
/////////////////////////////////////////////////////////////////////////////
Service::ReturnCode Service::Start(int wait_time)
{
    ReturnCode status = SERVICE_RETURN_SUCCESS;
    SERVICE_STATUS service_status;
    SC_HANDLE sch;

    if (g_service_name == NULL)
    {
        return SERVICE_ERROR_NOT_FOUND; // SERVICE_ERROR_NOT_FOUND
    }
    else if ((sch = OpenSCManager(NULL, NULL, GENERIC_READ)) == NULL)
    {
        status = get_error(GetLastError(), "open");
    }
    else
    {
        SC_HANDLE service = OpenService(
            sch, g_service_name, 
            SERVICE_START | SERVICE_QUERY_STATUS);
 
        if (service == NULL) 
        { 
            status = get_error(GetLastError(), "open");
        }
        else if (!StartService(service, 0, NULL))
        {
            status = get_error(GetLastError(), "start");
        }
        else 
        {
            int i, max = (wait_time > 0) ? wait_time : 5;

            // Loop up to max times waiting for the service 
            // state to change to RUNNING

            for (i = 0; i < max; i++)
            {
                if (!QueryServiceStatus(service, &service_status))
                {
                    status = get_error(GetLastError(), "query");
                    return status; // QUERY_FAIL
                }

                if (service_status.dwCurrentState == SERVICE_RUNNING)
                {
                    break;
                }

                Sleep(1 * CLOCKS_PER_SEC);
            }
          
            status = (i < max) ? 
                SERVICE_RETURN_SUCCESS : 
                SERVICE_ERROR_REQUEST_TIMEOUT;

            CloseServiceHandle(service);
        }
 
        CloseServiceHandle(sch); 
    }

    return status;
} 

/////////////////////////////////////////////////////////////////////////////
// Method: Stop
//
// Args:
//   wait_time:
//     The user supplied wait (~1 second per QueryServiceStatus() attempt)
//
// Description:
//     Attempt to stop the service.
//
/////////////////////////////////////////////////////////////////////////////   
Service::ReturnCode Service::Stop(int wait_time)
{
    ReturnCode status = SERVICE_RETURN_SUCCESS;
    SERVICE_STATUS service_status;
    SC_HANDLE sch;

    if (g_service_name == NULL)
    {
        return SERVICE_ERROR_NOT_FOUND; // SERVICE_ERROR_NOT_FOUND
    }
    else if ((sch = OpenSCManager(NULL, NULL, GENERIC_READ)) == NULL)
    {
        status = get_error(GetLastError(), "open");
    }
    // show_error("OpenSCMManager", "service", GetLastError());
    else
    {
        SC_HANDLE service = OpenService(
            sch, 
            g_service_name, 
            SERVICE_STOP | SERVICE_QUERY_STATUS);
 
        if (service == NULL)
        { 
            status = get_error(GetLastError(), "open");
        }
        else if 
            (!ControlService(service, SERVICE_CONTROL_STOP, &service_status))
        {
            status = get_error(GetLastError(), "stop");
        }
        else 
        {
            int i, max = (wait_time > 0) ? wait_time : 5;
            
            // Loop up to max times waiting for the service
            // state to change to STOPPED

            for (i = 0; i < max; i++)
            {
                if (!QueryServiceStatus(service, &service_status))
                {
                    status = get_error(GetLastError(), "query");
                    return status; // QUERY_FAIL
                }

                if (service_status.dwCurrentState == SERVICE_STOPPED)
                {
                    break;
                }

                Sleep(1 * CLOCKS_PER_SEC);
            }          
            status = (i < max) ?
                SERVICE_RETURN_SUCCESS :
                SERVICE_ERROR_REQUEST_TIMEOUT;

            CloseServiceHandle(service);
        } 
        CloseServiceHandle(sch); 
    }
    return status;
}

/////////////////////////////////////////////////////////////////////////////
// Method: Run
//
// Args:
//   service_main:
//     The user supplied service_main function (not to be confused with
//     real_service_main above)
//
//   flags:
//     Reserved.  Currently unused.
//
// Description:
//     This function interacts with the SCM to run the current process
//     as a Win32 service.
//
// NOTE: If the process is successfully launched as a Win32 service, this
//       function never returns, but calls exit() instead.
/////////////////////////////////////////////////////////////////////////////
Service::ReturnCode Service::Run(SERVICE_MAIN_T service_main, DWORD flags)
{
    ReturnCode status = SERVICE_RETURN_SUCCESS;

    SERVICE_TABLE_ENTRY dispatchTable[] = 
    { 
        { g_service_name, real_service_main },
        { NULL, NULL }
    };

    // Validate the arguments as best we can

    if (g_service_name == NULL || service_main == NULL)
        return SERVICE_ERROR_NOT_FOUND; // SERVICE_ERROR_NOT_FOUND

    // Save parameters in global variables

    g_flags        = flags;
    g_service_main = service_main;

    // Kick off the service 

    if (!StartServiceCtrlDispatcher(dispatchTable))
    {
        status = get_error(GetLastError(), "start");
        return status; // FAIL
    }

    // Don't call exit()
    return status;
}

/////////////////////////////////////////////////////////////////////////////
// Method: GetState
//
// Description:
//     Returns the state of the service into "state".
//
/////////////////////////////////////////////////////////////////////////////
Service::ReturnCode Service::GetState(State *state)
{
    ReturnCode status = SERVICE_RETURN_SUCCESS;
    SERVICE_STATUS service_status;
    SC_HANDLE sch;

    if (g_service_name == NULL)
    {
        return SERVICE_ERROR_NOT_FOUND; // SERVICE_ERROR_NOT_FOUND
    }
    else if ((sch = OpenSCManager(NULL, NULL, GENERIC_READ)) == NULL)
    {
        status = get_error(GetLastError(), "open");
    }
    else
    {
        SC_HANDLE service = OpenService(
            sch, 
            g_service_name, 
            SERVICE_QUERY_STATUS);
 
        if (service == NULL)
        { 
            status = get_error(GetLastError(), "open");
        }
        else if (!QueryServiceStatus(service, &service_status))
        {
            status = get_error(GetLastError(), "query");
        }
        else
        {
            *state = get_state(service_status.dwCurrentState);
            CloseServiceHandle(service);
        }  
        CloseServiceHandle(sch);
    }
    return status;
}

/////////////////////////////////////////////////////////////////////////////
// Method: LogEvent
//
// Args:
//   event_type:
//     The Win32 event type. Valid event types are:
//      EVENTLOG_SUCCESS          : Success event
//      EVENTLOG_ERROR_TYPE       : Error event
//      EVENTLOG_WARNING_TYPE     : Warning event
//      EVENTLOG_INFORMATION_TYPE : Information event
//      EVENTLOG_AUDIT_SUCCESS    : Success audit event
//      EVENTLOG_AUDIT_FAILURE    : Failure audit event
//
//   event_id:
//     A fancy name for error code or error number.
//
//   string:
//     String to be logged or merged in with the error string in the
//     message DLL.
//
// Description:
//     This function provides a simple layer over the Win32 error logging
//     API's.
//
// Returns:
//     true  if event was successfully logged
//     false if the event could not be logged
//
////////////////////////////////////////////////////////////////////////////
bool
Service::LogEvent(WORD event_type, DWORD event_id, const char *string)
{
    BOOL   status;
    HANDLE h_event_source = RegisterEventSource(NULL, g_event_source);

    if (h_event_source == NULL)
        FALSE;

    status = ReportEvent(
        h_event_source,
        event_type,
        0,
        event_id,
        NULL,
        1,
        0,
        &string,
        NULL);

    DeregisterEventSource(h_event_source);

    return (status == TRUE) ? true : false;
}

////////////////////////////////////////////////////////////////////////////
// P R I V A T E
////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////
// Routine: real_service_main
//
// Args:
//   argc:
//     The number of arguments in the argv array
//   argv:
//     An array of strings representing the command line arguments
//
// Description:
//     This function is the real service main (as opposed to the user
//     supplied service main, which is called service_main).
//
// Returns:
//     nothing
////////////////////////////////////////////////////////////////////////////
void __stdcall Service::real_service_main(DWORD argc, LPTSTR *argv)
{
    // Let the SCM know we're alive and kicking
    
    report_status(SERVICE_START_PENDING, NO_ERROR, 0, 5000);

    // If the command line arguments include the string "-debug" then
    // invoke the debugger

    if (check_args_for_string("-debug"))
    {
        DebugBreak();
    }

    // Save copy of argc and argc in global variables
    g_argc = argc;
    g_argv = argv;

    // Start service actions

    g_service_status_handle = RegisterServiceCtrlHandler(
        g_service_name,
        service_control_handler);

    if (g_service_status_handle == 0)
    {
        show_error("register", "Service Control Handler", GetLastError());
        report_status(SERVICE_STOPPED, NO_ERROR, 0, 5000);
        return;
    }

    // Change our state to RUNNING, then invoke the user supplied
    // service_main function. After the user's service_main exits,
    // change the service state to STOPPED.

    report_status(SERVICE_RUNNING, NO_ERROR, 0, 5000);
    g_service_main(STARTUP_FLAG, argc, argv);
    report_status(SERVICE_STOPPED, NO_ERROR, 0, 5000);

    return;
}

////////////////////////////////////////////////////////////////////////////
// Routine: check_args_for_string
//
// Args:
//   string:
//     The string to match.
//
// Description:
//     This function iterates through the command line arguments searching
//     for the string specified by the string parameter.
//
// Returns:
//     true  if the string was found
//     false if the string was not found
//
////////////////////////////////////////////////////////////////////////////
bool Service::check_args_for_string(char *string)
{
    int i;

    for (i = 1; i < g_argc; i++)
    {
        if (strcmp(g_argv[i], string) == 0)
        {
            return true;
        }
    }
    return false;
}

////////////////////////////////////////////////////////////////////////////
// Method: service_control_handler
//
// Args:
//   control:
//     The control sent from the SCM telling us what action to take:
//     start, stop, pause, continue, etc.
//
// Description:
//     This function handles control messages sent from the SCM. Currently
//     the only message that is handled differently is the STOP message,
//     which invokes the user's service main function passing to it the
//     SHUTDOWN_FLAG. The user is then responsible to perform all shutdown
//     related tasks.
//
// Returns:
//     Nothing
//
////////////////////////////////////////////////////////////////////////////
void WINAPI Service::service_control_handler(DWORD control)
{
    /* Currently, only the stop contol requires special handling */

    if (control == SERVICE_CONTROL_STOP)
    {
        report_status(SERVICE_STOP_PENDING, NO_ERROR, 0, 5000);
        g_service_main(SHUTDOWN_FLAG, g_argc, g_argv);
        return;
    }

    /* For every other control, just send back our current state */

    report_status(g_current_state, NO_ERROR, 0, 5000);
}

////////////////////////////////////////////////////////////////////////////
// Method: report_status
//
// Args:
//   current_state:
//     The service's new state. May be any valid Win32 service state.
//
//   exit_code:
//    This must be a Win32 exit code. If the server is exiting due to
//     an error returned from the Win32 API, then this is the place to
//     report the error status. Most of the time, this will be NO_ERROR.
//
//   check_point:
//     An integer value that should start at zero and increment as each
//     discrete step within a phase is completed. For example, if startup
//     consists of 3 steps, then the startup will issue its first check-
//     point of zero, then increment up through and including 2 as each
//     step is finished.
//
//   wait_hint:
//     Tells the SCM how long to expect to wait for the next status
//     update.
//
// Description:
//     This function provides an even higher level of abstraction over
//     the Win32 event logging API's.
//
// Returns:
//     true  if the status was successfully reported
//     false if the status could not be reported
////////////////////////////////////////////////////////////////////////////
bool Service::report_status(
    DWORD current_state,
    DWORD exit_code,
    DWORD check_point,
    DWORD wait_hint)
{
    SERVICE_STATUS current_status =
    {
        SERVICE_WIN32,
        current_state,
        SERVICE_CONTROL_INTERROGATE,
        exit_code,
        0,
        check_point,
        wait_hint
    };

    /* Wait until we're started before we accept a stop control */

    if (current_state == SERVICE_RUNNING)
    {
        current_status.dwControlsAccepted += SERVICE_ACCEPT_STOP;
    }

    /* Save new state */

    g_current_state = current_state;
  
    return (SetServiceStatus(
        g_service_status_handle, &current_status) == TRUE) ? true : false;
}

////////////////////////////////////////////////////////////////////////////
// Method: change_service_description
//
// Args:
//   service:
//     Handle to the service
//
//   description:
//     The service's description
//
// Description:
//     This function sets the description for the service. The description
//     text shows up in the Services mmc snapin. The description is added
//     as a benefit to users, but does not affect the service's operation.
//     Therefore, if this function should fail for any reason, there is
//     no need to stop. That's why this function has no return value.
//
// Returns:
//     Nothing
//
// Notes:
//     This function uses the ChangeServiceConfig2() API function which
//     first appeared in Windows2000. Therefore, this code checks the
//     OS version, before calling ChangeServiceConfig2().
//                  
////////////////////////////////////////////////////////////////////////////
void Service::change_service_description(SC_HANDLE service, char *description)
{
    OSVERSIONINFO osvi;

    /*
    *  Test for Windows 2000 or greater
    */

    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

    if (GetVersionEx(&osvi) != 0 && 
        osvi.dwPlatformId == VER_PLATFORM_WIN32_NT && 
        osvi.dwMajorVersion >= 5)
    {
        typedef BOOL 
            (WINAPI *CHANGE_SERVICE_CONFIG2_T)(SC_HANDLE, DWORD, LPVOID);

        HINSTANCE hdll = LoadLibrary("advapi32.dll");

        if (hdll != NULL)
        {
            SERVICE_DESCRIPTION sd;
            CHANGE_SERVICE_CONFIG2_T csc;

            csc = (CHANGE_SERVICE_CONFIG2_T) GetProcAddress(
                hdll, 
                "ChangeServiceConfig2A");

            if (csc)
            {
                sd.lpDescription = description;
                csc(service, SERVICE_CONFIG_DESCRIPTION, &sd);
            }

            FreeLibrary(hdll);
        }
    }
}

////////////////////////////////////////////////////////////////////////////
// Method: show_error
//
// Args:
//   action:
//     A single verb decribing the action going on when the error
//     occurred. For example, "opening", "creating", etc.
//
//   object:
//     Description of the object on which was action occurred. Examples
//     are "file", "service", etc.
//
//   hr:
//     The error status. Can be an hresult, or Win32 error status.
//
// Description:
//     This function provides an even higher level of abstraction over
//     the Win32 event logging API's.
//
// Returns:
//     true  if event was successfully logged
//     false if the event could not be logged
//                              
////////////////////////////////////////////////////////////////////////////
bool Service::show_error(const char *action, const char *object, DWORD hr)
{
    char console_title[_MAX_PATH] = {0};
    char txt[_MAX_PATH] = "";
    char msg[_MAX_PATH] = "";
    DWORD nchars;

    nchars = FormatMessage(
        FORMAT_MESSAGE_FROM_SYSTEM,
        NULL,
        hr,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        msg,
        sizeof(msg),
        NULL);

    if (nchars == 0)
    { 
        sprintf(msg, "Unknown error code - %%X%x", hr);
    }
    else if (nchars > 1)
    {
        if (msg[nchars - 1] == '\n') msg[nchars - 1] = '\0';
        if (msg[nchars - 2] == '\r') msg[nchars - 2] = '\0';
    }

    sprintf(
        txt, 
        "Failed to %s %s %s. Reason: %s", 
        action, 
        object, 
        g_service_name, 
        msg);

    // Running from a console window
    // send courtesy message txt to stderr
    if (GetConsoleTitle(console_title, _MAX_PATH) > 0)
    {
        PEGASUS_STD(cerr) << txt << PEGASUS_STD(endl);
    }

    return LogEvent(EVENTLOG_ERROR_TYPE, 1, txt);
}

Service::State Service::get_state(DWORD scm_state)
{
    return (State)scm_state;
}

Service::ReturnCode Service::get_error(DWORD error_status, const char *action)
{
    switch (error_status)
    {
    /*
        // INFO: Could add cases to suppress error message.
        case ERROR_SERVICE_DOES_NOT_EXIST:
        case ERROR_SERVICE_CANNOT_ACCEPT_CTRL:
        break;
    */
        case SERVICE_RETURN_SUCCESS:
        break;

        default:
            show_error(action, "service", error_status); 
        break;
    }
    return (ReturnCode)error_status;
}

