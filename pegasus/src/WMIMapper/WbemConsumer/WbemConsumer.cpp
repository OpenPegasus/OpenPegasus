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
// WbemConsumer.cpp : Implementation of WinMain
//
//////////////////////////////////////////////////////////////////////

// Note: Proxy/Stub Information
//      To build a separate proxy/stub DLL, 
//      run nmake -f WbemConsumerps.mk in the project directory.

#include "stdafx.h"
#include "resource.h"
#include <initguid.h>

#include "WbemConsumer.h"
#include "WbemConsumer_i.cpp"
#include "EventConsumer.h"
#include "EventDispatcher.h"
#include "DCOMRegister.h"

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/Tracer.h>
#include <Pegasus/Config/ConfigManager.h>

#include "TraceUtils.cpp"

#include <stdio.h>

CServiceModule _Module;

BEGIN_OBJECT_MAP(ObjectMap)
OBJECT_ENTRY(CLSID_EventConsumer, CEventConsumer)
OBJECT_ENTRY(CLSID_EventDispatcher, CEventDispatcher)
END_OBJECT_MAP()


LPCTSTR FindOneOf(LPCTSTR p1, LPCTSTR p2)
{
    while (p1 != NULL && *p1 != NULL)
    {
        LPCTSTR p = p2;
        while (p != NULL && *p != NULL)
        {
            if (*p1 == *p)
            {
                return CharNext(p1);
            }
            p = CharNext(p);
        }
        p1 = CharNext(p1);
    }
    return NULL;
}

// Although some of these functions are big they are declared inline since 
// they are only used once

inline HRESULT CServiceModule::RegisterServer(BOOL bRegTypeLib, BOOL bService)
{
    HRESULT hr = CoInitialize(NULL);
    if (FAILED(hr))
    {
        return hr;
    }

    // Remove any previous service since it may point to
    // the incorrect file
    Uninstall();

    // Add service entries
    UpdateRegistryFromResource(IDR_WbemConsumer, TRUE);

    // Adjust the AppID for Local Server or Service
    CRegKey keyAppID;
    LONG lRes = keyAppID.Open(HKEY_CLASSES_ROOT, _T("AppID"), KEY_WRITE);
    if (lRes != ERROR_SUCCESS)
    {
        return lRes;
    }

    CRegKey key;
    lRes = key.Open(
        keyAppID, 
        _T("{BA48CF5D-8012-4680-8AD9-12D688B3BB3D}"), 
        KEY_WRITE);
        
    if (lRes != ERROR_SUCCESS)
    {
        return lRes;
    }
    
    key.DeleteValue(_T("LocalService"));
    
    if (bService)
    {
        key.SetValue(_T("WbemConsumer"), _T("LocalService"));
        key.SetValue(_T("-Service"), _T("ServiceParameters"));
        
        // Create service
        Install();
    }

    // Add object entries
    hr = CComModule::RegisterServer(bRegTypeLib);

    if(adjustDCOMPermissions()) 
    {
        MessageBox(
            NULL, 
            _T("Couldn't adjust DCOM permissions."), 
            m_szServiceName, 
            MB_OK);
    }
    CoUninitialize();
    return hr;
}

inline HRESULT CServiceModule::UnregisterServer()
{
    HRESULT hr = CoInitialize(NULL);
    if (FAILED(hr))
    {
        return hr;
    }

    // Remove service entries
    UpdateRegistryFromResource(IDR_WbemConsumer, FALSE);
    // Remove service
    Uninstall();
    // Remove object entries
    CComModule::UnregisterServer(TRUE);
    CoUninitialize();
    return S_OK;
}

inline void CServiceModule::Init(
    _ATL_OBJMAP_ENTRY* p, 
    HINSTANCE h, 
    UINT nServiceNameID, 
    const GUID* plibid)
{
    CComModule::Init(p, h, plibid);

    m_bService = TRUE;

    LoadString(
        h, 
        nServiceNameID, 
        m_szServiceName, 
        sizeof(m_szServiceName) / sizeof(TCHAR));

    // set up the initial service status 
    m_hServiceStatus = NULL;
    m_status.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
    m_status.dwCurrentState = SERVICE_STOPPED;
    m_status.dwControlsAccepted = SERVICE_ACCEPT_STOP;
    m_status.dwWin32ExitCode = 0;
    m_status.dwServiceSpecificExitCode = 0;
    m_status.dwCheckPoint = 0;
    m_status.dwWaitHint = 0;
}

LONG CServiceModule::Unlock()
{
    LONG l = CComModule::Unlock();
    if (l == 0 && !m_bService)
    {
        PostThreadMessage(dwThreadID, WM_QUIT, 0, 0);
    }
    return l;
}

BOOL CServiceModule::IsInstalled()
{
    BOOL bResult = FALSE;

    SC_HANDLE hSCM = ::OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);

    if (hSCM != NULL)
    {
        SC_HANDLE hService = ::OpenService(
            hSCM, 
            m_szServiceName, 
            SERVICE_QUERY_CONFIG);
            
        if (hService != NULL)
        {
            bResult = TRUE;
            ::CloseServiceHandle(hService);
        }
        ::CloseServiceHandle(hSCM);
    }
    return bResult;
}

inline BOOL CServiceModule::Install()
{
    if (IsInstalled())
    {
        return TRUE;
    }

    SC_HANDLE hSCM = ::OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if (hSCM == NULL)
    {
        MessageBox(
            NULL, 
            _T("Couldn't open service manager"), 
            m_szServiceName, 
            MB_OK);
            
        return FALSE;
    }

    // Get the executable file path
    TCHAR szFilePath[_MAX_PATH];
    ::GetModuleFileName(NULL, szFilePath, _MAX_PATH);

    SC_HANDLE hService = ::CreateService(
        hSCM,
        m_szServiceName,
        _T("Pegasus WMIMapper Indications Service"),
        SERVICE_ALL_ACCESS,
        SERVICE_WIN32_OWN_PROCESS,
        SERVICE_AUTO_START,
        SERVICE_ERROR_NORMAL,
        szFilePath,
        NULL,
        NULL,
        _T("RPCSS\0"),
        NULL,
        NULL);

    if (hService == NULL)
    {
        ::CloseServiceHandle(hSCM);
        MessageBox(
            NULL,
            _T("Couldn't create service"),
            m_szServiceName,
            MB_OK);
            
        return FALSE;
    }

    SERVICE_DESCRIPTION ServiceDesc;
    ServiceDesc.lpDescription = "Converts and Delivers WMI indications \
        as CIM indications";
        
    ChangeServiceConfig2(
        hService,
        SERVICE_CONFIG_DESCRIPTION,
        (void *) &ServiceDesc);

    ::CloseServiceHandle(hService);
    ::CloseServiceHandle(hSCM);
    
    return TRUE;
}

inline BOOL CServiceModule::Uninstall()
{
    if (!IsInstalled())
    {
        return TRUE;
    }

    SC_HANDLE hSCM = ::OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);

    if (hSCM == NULL)
    {
        MessageBox(
            NULL,
            _T("Couldn't open service manager"),
            m_szServiceName,
            MB_OK);
            
        return FALSE;
    }

    SC_HANDLE hService = ::OpenService(
        hSCM,
        m_szServiceName,
        SERVICE_STOP | DELETE);

    if (hService == NULL)
    {
        ::CloseServiceHandle(hSCM);
        MessageBox(NULL, _T("Couldn't open service"), m_szServiceName, MB_OK);
        return FALSE;
    }
    SERVICE_STATUS status;
    ::ControlService(hService, SERVICE_CONTROL_STOP, &status);

    BOOL bDelete = ::DeleteService(hService);
    ::CloseServiceHandle(hService);
    ::CloseServiceHandle(hSCM);

    if (bDelete)
    {
        return TRUE;
    }

    MessageBox(
        NULL,
        _T("Service could not be deleted"),
        m_szServiceName,
        MB_OK);
        
    return FALSE;
}

/////////////////////////////////////////////////////////////////////////////
// Logging functions

void CServiceModule::LogEvent(LPCTSTR pFormat, ...)
{
    TCHAR chMsg[256];
    HANDLE hEventSource;
    LPTSTR lpszStrings[1];
    va_list pArg;

    va_start(pArg, pFormat);
    _vstprintf(chMsg, pFormat, pArg);
    va_end(pArg);

    lpszStrings[0] = chMsg;

    if (m_bService)
    {
        /* Get a handle to use with ReportEvent(). */
        hEventSource = RegisterEventSource(NULL, m_szServiceName);
        if (hEventSource != NULL)
        {
            /* Write to event log. */
            ReportEvent(
                hEventSource,
                EVENTLOG_INFORMATION_TYPE,
                0,
                0,
                NULL,
                1,
                0,
                (LPCTSTR*) &lpszStrings[0],
                NULL);
                
            DeregisterEventSource(hEventSource);
        }
    }
    else
    {
        // As we are not running as a service, just write the error to the
        // console.
        _putts(chMsg);
    }
}

/////////////////////////////////////////////////////////////////////////////
// Service startup and registration

inline void CServiceModule::Start()
{
    SERVICE_TABLE_ENTRY st[] =
    {
        { m_szServiceName, _ServiceMain },
        { NULL, NULL }
    };
    if (m_bService && !::StartServiceCtrlDispatcher(st))
    {
        m_bService = FALSE;
    }
    if (m_bService == FALSE)
    {
        Run();
    }
}

inline void CServiceModule::ServiceMain(
    DWORD /* dwArgc */, 
    LPTSTR* /* lpszArgv */)
{
    // Register the control request handler
    m_status.dwCurrentState = SERVICE_START_PENDING;
    m_hServiceStatus = RegisterServiceCtrlHandler(m_szServiceName, _Handler);
    if (m_hServiceStatus == NULL)
    {
        LogEvent(_T("Handler not installed"));
        return;
    }
    SetServiceStatus(SERVICE_START_PENDING);

    m_status.dwWin32ExitCode = S_OK;
    m_status.dwCheckPoint = 0;
    m_status.dwWaitHint = 0;

    // When the Run function returns, the service has stopped.
    Run();

    SetServiceStatus(SERVICE_STOPPED);
    LogEvent(_T("Service stopped"));
}

inline void CServiceModule::Handler(DWORD dwOpcode)
{
    switch (dwOpcode)
    {
    case SERVICE_CONTROL_STOP:
        SetServiceStatus(SERVICE_STOP_PENDING);
        PostThreadMessage(dwThreadID, WM_QUIT, 0, 0);
        break;
    case SERVICE_CONTROL_PAUSE:
        break;
    case SERVICE_CONTROL_CONTINUE:
        break;
    case SERVICE_CONTROL_INTERROGATE:
        break;
    case SERVICE_CONTROL_SHUTDOWN:
        break;
    default:
        LogEvent(_T("Bad service request"));
    }
}

void WINAPI CServiceModule::_ServiceMain(DWORD dwArgc, LPTSTR* lpszArgv)
{
    _Module.ServiceMain(dwArgc, lpszArgv);
}
void WINAPI CServiceModule::_Handler(DWORD dwOpcode)
{
    _Module.Handler(dwOpcode); 
}

void CServiceModule::SetServiceStatus(DWORD dwState)
{
    m_status.dwCurrentState = dwState;
    ::SetServiceStatus(m_hServiceStatus, &m_status);
}

void CServiceModule::Run()
{
    _Module.dwThreadID = GetCurrentThreadId();

    HRESULT hr = CoInitialize(NULL);
//  If you are running on NT 4.0 or higher you can use the following call
//  instead to make the EXE free threaded.
//  This means that calls come in on a random RPC thread
//  HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);

    _ASSERTE(SUCCEEDED(hr));

    // This provides a NULL DACL which will allow access to everyone.
    CSecurityDescriptor sd;
    sd.InitializeFromThreadToken();

    hr = CoInitializeSecurity(
        NULL,
        -1,
        NULL,
        NULL, 
        RPC_C_AUTHN_LEVEL_NONE,
        RPC_C_IMP_LEVEL_IMPERSONATE,
        NULL,
        EOAC_NONE,
        NULL);

    _ASSERTE(SUCCEEDED(hr));

    hr = _Module.RegisterClassObjects(
        CLSCTX_LOCAL_SERVER | CLSCTX_REMOTE_SERVER, 
        REGCLS_MULTIPLEUSE);
        
    _ASSERTE(SUCCEEDED(hr));

    LogEvent(_T("Service started"));
    if (m_bService)
    {
        SetServiceStatus(SERVICE_RUNNING);
    }

    MSG msg;
    while (GetMessage(&msg, 0, 0, 0))
    {
        DispatchMessage(&msg);
    }

    _Module.RevokeClassObjects();

    CoUninitialize();
}

/////////////////////////////////////////////////////////////////////////////
//
extern "C" int WINAPI _tWinMain(
    HINSTANCE hInstance, 
    HINSTANCE /*hPrevInstance*/, 
    LPTSTR lpCmdLine, 
    int /*nShowCmd*/)
{
    lpCmdLine = GetCommandLine(); //this line necessary for _ATL_MIN_CRT
    _Module.Init(
        ObjectMap,
        hInstance,
        IDS_SERVICENAME,
        &LIBID_WBEMCONSUMERLib);
        
    _Module.m_bService = TRUE;

    TCHAR szTokens[] = _T("-/");

    ///////////////////////////////////
    // Sets the path for WbemConsumer trace file 

    Pegasus::ConfigManager* configManager;

    //
    // Set the value for pegasusHome property
    //
    Pegasus::String pegasusHome  = Pegasus::String::EMPTY;

    setHome(pegasusHome);

    Pegasus::ConfigManager::setPegasusHome(pegasusHome);

    //
    // Get an instance of the Config Manager.
    //
    configManager = Pegasus::ConfigManager::getInstance();
    configManager->useConfigFiles = true;

    configManager->mergeConfigFiles();

    char traceFile[1024];
    int paramLen;
    int tracePathLen;

    tracePathLen = _snprintf(traceFile, 
        1024, 
        "%s\\%s", 
        pegasusHome.getCString(),
        "trace\\wbemcons.trc");

    /* 
        Only write in trace file if the Path was not trucated inside
        string traceFile
    */
    if (tracePathLen < 1024)
    {
        Pegasus::Tracer::setTraceFile(traceFile);
    }

    Pegasus::String traceComponents;
    Pegasus::String traceLevel;

    try {
        traceLevel = configManager->getCurrentValue("TraceLevel");
        Pegasus::Tracer::setTraceLevel(atoi(traceLevel.getCString()));
    }
    catch (...)
    {
    }

    try {
        traceComponents = configManager->getCurrentValue("TraceComponents");
        Pegasus::Tracer::setTraceComponents(traceComponents);
    }
    catch (...)
    {
    }
    
    ///////////////////////////////////

    LPCTSTR lpszToken = FindOneOf(lpCmdLine, szTokens);
    while (lpszToken != NULL)
    {
        if (lstrcmpi(lpszToken, _T("UnregServer"))==0)
        {
            return _Module.UnregisterServer();
        }

        // Register as Local Server
        if (lstrcmpi(lpszToken, _T("RegServer"))==0)
        {
            return _Module.RegisterServer(TRUE, FALSE);
        }
        
        // Register as Service
        if (lstrcmpi(lpszToken, _T("Service"))==0)
        {
            return _Module.RegisterServer(TRUE, TRUE);
        }
        
        lpszToken = FindOneOf(lpszToken, szTokens);
    }

    // Are we Service or Local Server
    CRegKey keyAppID;
    LONG lRes = keyAppID.Open(HKEY_CLASSES_ROOT, _T("AppID"), KEY_READ);
    if (lRes != ERROR_SUCCESS)
    {
        return lRes;
    }

    CRegKey key;
    lRes = key.Open(
        keyAppID,
        _T("{BA48CF5D-8012-4680-8AD9-12D688B3BB3D}"),
        KEY_READ);
        
    if (lRes != ERROR_SUCCESS)
    {
        return lRes;
    }

    TCHAR szValue[_MAX_PATH];
    DWORD dwLen = _MAX_PATH;
    lRes = key.QueryValue(szValue, _T("LocalService"), &dwLen);

    _Module.m_bService = FALSE;
    if (lRes == ERROR_SUCCESS)
    {
        _Module.m_bService = TRUE;
    }

    _Module.Start();

    // When we get here, the service has been stopped
    return _Module.m_status.dwWin32ExitCode;
}
