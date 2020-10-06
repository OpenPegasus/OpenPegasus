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
// Author: Tony Fiorentino (fiorentino_tony@emc.com)
//
//%/////////////////////////////////////////////////////////////////////////////
#ifndef SERVICE_H
#define SERVICE_H

typedef int (*SERVICE_MAIN_T)(int flag, int argc, char **argv);

class Service
{
public:
    // default ctor
    Service();

    // ctor using service name
    Service(const char *service_name);

    // ctor using service name and event source
    Service(const char *service_name, char *event_source);

    // dtor
    ~Service();

    // State
    enum State
    {
        SERVICE_STATE_STOPPED            = SERVICE_STOPPED,
        SERVICE_STATE_START_PENDING      = SERVICE_START_PENDING,
        SERVICE_STATE_STOP_PENDING       = SERVICE_STOP_PENDING,
        SERVICE_STATE_RUNNING            = SERVICE_RUNNING,
        SERVICE_STATE_CONTINUE_PENDING   = SERVICE_CONTINUE_PENDING,
        SERVICE_STATE_PAUSE_PENDING      = SERVICE_PAUSE_PENDING,
        SERVICE_STATE_PAUSED             = SERVICE_PAUSED
    };

    // Return Codes
    enum ReturnCode
    {
        SERVICE_RETURN_SUCCESS           = 0,
        SERVICE_ERROR_MARKED_FOR_DELETE  = ERROR_SERVICE_MARKED_FOR_DELETE,
        SERVICE_ERROR_DOES_NOT_EXIST     = ERROR_SERVICE_DOES_NOT_EXIST,
        SERVICE_ERROR_REQUEST_TIMEOUT    = ERROR_SERVICE_REQUEST_TIMEOUT,
        SERVICE_ERROR_NO_THREAD          = ERROR_SERVICE_NO_THREAD,
        SERVICE_ERROR_DATABASE_LOCKED    = ERROR_SERVICE_DATABASE_LOCKED,
        SERVICE_ERROR_ALREADY_RUNNING    = ERROR_SERVICE_ALREADY_RUNNING,
        SERVICE_ERROR_DISABLED           = ERROR_SERVICE_DISABLED,
        SERVICE_ERROR_CANNOT_ACCEPT_CTRL = ERROR_SERVICE_CANNOT_ACCEPT_CTRL,
        SERVICE_ERROR_NOT_ACTIVE         = ERROR_SERVICE_NOT_ACTIVE,
        SERVICE_ERROR_SPECIFIC_ERROR     = ERROR_SERVICE_SPECIFIC_ERROR,
        SERVICE_ERROR_DEPENDENCY_FAIL    = ERROR_SERVICE_DEPENDENCY_FAIL,
        SERVICE_ERROR_LOGON_FAILED       = ERROR_SERVICE_LOGON_FAILED,
        SERVICE_ERROR_START_HANG         = ERROR_SERVICE_START_HANG,
        SERVICE_ERROR_DOES_EXISTS        = ERROR_SERVICE_EXISTS,
        SERVICE_ERROR_DEPENDENCY_DELETED = ERROR_SERVICE_DEPENDENCY_DELETED,
        SERVICE_ERROR_NEVER_STARTED      = ERROR_SERVICE_NEVER_STARTED,
        SERVICE_ERROR_NOT_FOUND          = ERROR_SERVICE_NOT_FOUND,
        SERVICE_ERROR_CONTROLLER_CONNECT =
            ERROR_FAILED_SERVICE_CONTROLLER_CONNECT,
        SERVICE_ERROR_INVALID_CONTROL    = ERROR_INVALID_SERVICE_CONTROL,
        SERVICE_ERROR_INVALID_ACCOUNT    = ERROR_INVALID_SERVICE_ACCOUNT,
        SERVICE_ERROR_INVALID_LOCK       = ERROR_INVALID_SERVICE_LOCK,
        SERVICE_ERROR_DUPLICATE_NAME     = ERROR_DUPLICATE_SERVICE_NAME,
        SERVICE_ERROR_DIFFERENT_ACCOUNT  = ERROR_DIFFERENT_SERVICE_ACCOUNT,
        SERVICE_ERROR_UNKNOWN            = -999
    };

    // Flag
    enum Flag
    {
        STARTUP_FLAG,
        SHUTDOWN_FLAG
    };

    // Methods
    ReturnCode Install(char* display_name, char* description, char* exe_name);
    ReturnCode Remove();

    ReturnCode Start(int wait_time);
    ReturnCode Stop(int wait_time);
    static bool report_status(
        DWORD current_state,
        DWORD exit_code,
        DWORD check_point,
        DWORD wait_hint);

    static ReturnCode Run(SERVICE_MAIN_T service_main, DWORD flags = 0);
    ReturnCode GetState(State *state);
    static bool LogEvent(WORD event_type, DWORD event_id, const char *string);
    static void SetServiceName(char *service_name);

    static void SetServiceArgs(int num_args, char **service_args)
    {
        g_argv = service_args;
        g_argc = num_args;
    }
    static char *GetServiceName()
    {
        return g_service_name;
    }

private:
    static int                    g_argc;
    static char                 **g_argv;
    static char                  *g_service_name;
    static char                  *g_event_source;
    static DWORD                  g_flags;
    static DWORD                  g_current_state;
    static SERVICE_STATUS_HANDLE  g_service_status_handle;
    static SERVICE_MAIN_T         g_service_main;

    static bool show_error(const char *action, const char *object, DWORD hr);
    static void WINAPI service_control_handler(DWORD control);
    static bool check_args_for_string(char *string);
    static void __stdcall real_service_main(DWORD argc, LPTSTR *argv);
    static void change_service_description(
        SC_HANDLE service,
        char* description);
    State get_state(DWORD scm_state);
    static ReturnCode get_error(
        DWORD error_status,
        const char action[] = "cimserver");
};

#endif // SERVICE_H

