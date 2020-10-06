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

///////////////////////////////////////////////////////////////////////////
// Based on MSDN sample 
//
//////////////////////////////////////////////////////////////////////////

#ifndef DCOM_Register_h
#define DCOM_Register_h

#include <windows.h>
#include <stdio.h>
#include <conio.h>
#include <tchar.h>
#include "ntsecapi.h"

#include <lm.h>

#define GUIDSTR_MAX 38

#ifndef STR2UNI

#define STR2UNI(unistr, regstr) \
    mbstowcs (unistr, regstr, strlen (regstr)+1);

#define UNI2STR(regstr, unistr) \
    wcstombs (regstr, unistr, wcslen (unistr)+1);

#endif

#define WBEMCONSUMER_APPID "{BA48CF5D-8012-4680-8AD9-12D688B3BB3D}"

#define CNS_COM_RIGHTS_EXECUTE            1
#define CNS_COM_RIGHTS_ACCESS_LOCAL       2
#define CNS_COM_RIGHTS_ACCESS_REMOTE      4
#define CNS_COM_RIGHTS_LAUNCH_LOCAL       2
#define CNS_COM_RIGHTS_LAUNCH_REMOTE      4
#define CNS_COM_RIGHTS_ACTIVATE_LOCAL     8
#define CNS_COM_RIGHTS_ACTIVATE_REMOTE   16

DWORD adjustDCOMPermissions();

DWORD setDCOMPermission(
    HKEY RootKey, 
    LPTSTR KeyName, 
    PSID sid,
    LPTSTR ValueName,
    DWORD dwPermissionMask,
    BOOL bLimits = TRUE);

DWORD setDCOMAccessLimits(
    PSID sid,
    DWORD dwPermissionMask);

DWORD setDCOMLaunchLimits(
    PSID sid,
    DWORD dwPermissionMask);

DWORD setDCOMAppLaunch(
    PSID sid,
    DWORD dwPermissionMask,
    LPTSTR AppID);

DWORD setDCOMAppAccess(
    PSID sid,
    DWORD dwPermissionMask,
    LPTSTR AppID);

DWORD CreateNewSD(
    SECURITY_DESCRIPTOR **SD);

DWORD MakeSDAbsolute(
    PSECURITY_DESCRIPTOR OldSD,
    PSECURITY_DESCRIPTOR *NewSD);

DWORD SetNamedValueSD(
    HKEY RootKey,
    LPTSTR KeyName,
    LPTSTR ValueName,
    SECURITY_DESCRIPTOR *SD
    );

DWORD GetNamedValueSD(
    HKEY RootKey,
    LPTSTR KeyName,
    LPTSTR ValueName,
    SECURITY_DESCRIPTOR **SD,
    BOOL *NewSD);

DWORD CopyACL(
    PACL OldACL,
    PACL NewACL);

DWORD AddAccessAllowedACEToACL(
    PACL *Acl,
    DWORD PermissionMask,
    LPTSTR Principal);

DWORD GetCurrentUserSID(
    PSID *Sid);

DWORD GetPrincipalSID(
    LPTSTR Principal,
    PSID *Sid);

#endif /* DCOM_Register_h */
