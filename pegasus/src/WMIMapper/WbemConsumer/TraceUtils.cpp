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

#include <windows.h>
#include <process.h>     /* _beginthread, _endthread */
#include <tchar.h>
#include <direct.h>

#include <Pegasus/Common/FileSystem.h>

#include "service.cpp"

PEGASUS_USING_PEGASUS;
PEGASUS_USING_STD;

/////////////////////////////////////////////////////////////////////////////
// DEFINES
//
/////////////////////////////////////////////////////////////////////////////
#define PEGASUS_SERVICE_NAME "wmiserver"
#define PEGASUS_DISPLAY_NAME "Pegasus WMI Mapper"
#define PEGASUS_DESCRIPTION "Pegasus WBEM to WMI Mapper Manager Service"

/////////////////////////////////////////////////////////////////////////////
// GLOBALS
//
/////////////////////////////////////////////////////////////////////////////
static HANDLE pegasus_service_event;
static LPCSTR g_cimservice_key =
    TEXT("SYSTEM\\CurrentControlSet\\Services\\%s");
static LPCSTR g_cimservice_home = TEXT("home");

/////////////////////////////////////////////////////////////////////////////
// PROTOTYPES
//
/////////////////////////////////////////////////////////////////////////////
static bool _getRegInfo(const char *lpchKeyword, char *lpchRetValue);
static bool _setRegInfo(const char *lpchKeyword, const char *lpchValue);
void setHome(String & home);

/////////////////////////////////////////////////////////////////////////////
// NO-OPs for windows platform
//
/////////////////////////////////////////////////////////////////////////////
int cimserver_fork(void) { return(0); }
int cimserver_kill(void) { return(0); }
void notify_parent(int id) { return; }

/////////////////////////////////////////////////////////////////////////////
// HELPER Utilities
//
/////////////////////////////////////////////////////////////////////////////
static bool _getRegInfo(const char *lpchKeyword, char *lpchRetValue)
{
    HKEY hKey;
    DWORD dw = _MAX_PATH;
    char subKey[_MAX_PATH] = {0};

    sprintf(subKey, g_cimservice_key, PEGASUS_SERVICE_NAME);

    if ((RegOpenKeyEx(
        HKEY_LOCAL_MACHINE,
        subKey,
        0,
        KEY_READ,
        &hKey)) != ERROR_SUCCESS)
    {
        return false;
    }

    if ((RegQueryValueEx(hKey,
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

void setHome(String & home)
{
    // Determine the absolute path to the running program
    char exe_pathname[_MAX_PATH] = {0};
    char home_pathname[_MAX_PATH] = {0};
    GetModuleFileName(NULL, exe_pathname, sizeof(exe_pathname));

    // Pegasus home search rules:
    // - look in registry (if set)
    // - if not found, look in PEGASUS_HOME (if set)
    // - if not found, use exe directory minus one level

    bool found_reg = _getRegInfo("home", home_pathname);
    if (found_reg == true)
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

