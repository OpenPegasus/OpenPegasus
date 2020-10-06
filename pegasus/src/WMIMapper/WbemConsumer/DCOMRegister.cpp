
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

#include "stdafx.h"
#include "DCOMRegister.h"

DWORD adjustDCOMPermissions()
{
    SID_IDENTIFIER_AUTHORITY AdministratorsAuthority = SECURITY_NT_AUTHORITY;
    SID_IDENTIFIER_AUTHORITY AnonymousAuthority = SECURITY_NT_AUTHORITY;
    SID_IDENTIFIER_AUTHORITY EveryoneAuthority = SECURITY_WORLD_SID_AUTHORITY;
    PSID sidAdministrators = NULL;
    PSID sidAnonymous = NULL;
    PSID sidEveryone = NULL;
    DWORD dwRet = ERROR_SUCCESS;

    //get administrators SID
    if (!AllocateAndInitializeSid(
            &AdministratorsAuthority,
            2,
            SECURITY_BUILTIN_DOMAIN_RID,
            DOMAIN_ALIAS_RID_ADMINS,
            0, 0, 0, 0, 0, 0,
            &sidAdministrators
            ))
    {
        return GetLastError();
    }

    //get anonymous SID
    if (!AllocateAndInitializeSid(
            &AnonymousAuthority,
            1,
            SECURITY_ANONYMOUS_LOGON_RID,
            0, 0, 0, 0, 0, 0, 0,
            &sidAnonymous))
    {
        FreeSid(sidAdministrators);
        return GetLastError();
    }

    //get everyone SID
    if (!AllocateAndInitializeSid(
            &EveryoneAuthority,
            1,
            SECURITY_WORLD_RID,
            0, 0, 0, 0, 0, 0, 0,
            &sidEveryone))
    {
        FreeSid(sidAdministrators);
        FreeSid(sidAnonymous);
        return GetLastError();
    }

    //adjust DCOM Limits permissions
    if (dwRet = setDCOMAccessLimits(
            sidAnonymous,
            CNS_COM_RIGHTS_EXECUTE | 
            CNS_COM_RIGHTS_ACCESS_LOCAL | 
            CNS_COM_RIGHTS_ACCESS_REMOTE))
    {                        
        return dwRet;
    } 

    if (dwRet = setDCOMLaunchLimits(
            sidAnonymous,
            CNS_COM_RIGHTS_EXECUTE |
            CNS_COM_RIGHTS_LAUNCH_LOCAL |
            CNS_COM_RIGHTS_ACTIVATE_LOCAL |
            CNS_COM_RIGHTS_ACTIVATE_REMOTE))
    {        
        return dwRet;
    }

    if (dwRet = setDCOMAccessLimits(
            sidEveryone,
            CNS_COM_RIGHTS_EXECUTE | 
            CNS_COM_RIGHTS_ACCESS_LOCAL |
            CNS_COM_RIGHTS_ACCESS_REMOTE))
    {
        return dwRet;
    }

    if (dwRet = setDCOMLaunchLimits(
            sidEveryone,
            CNS_COM_RIGHTS_EXECUTE |
            CNS_COM_RIGHTS_LAUNCH_LOCAL |
            CNS_COM_RIGHTS_ACTIVATE_LOCAL |
            CNS_COM_RIGHTS_ACTIVATE_REMOTE))
    {            
        return dwRet;
    }        

    //adjust WBEMConsumer DCOM component permissions
    if (dwRet = setDCOMAppAccess(
            sidAnonymous,
            COM_RIGHTS_EXECUTE,
            WBEMCONSUMER_APPID))
    {    
        return dwRet;
    }
    
    if (dwRet = setDCOMAppLaunch(
            sidAnonymous,
            COM_RIGHTS_EXECUTE,
            WBEMCONSUMER_APPID))
    {
        return dwRet;
    }

    if (dwRet = setDCOMAppAccess(
            sidEveryone,
            COM_RIGHTS_EXECUTE,
            WBEMCONSUMER_APPID))
    {
        return dwRet;
    }

    if (dwRet = setDCOMAppLaunch(
            sidEveryone,
            COM_RIGHTS_EXECUTE,
            WBEMCONSUMER_APPID))
    {
        return dwRet;
    }

    if (dwRet = setDCOMAppAccess(
            sidAdministrators,
            COM_RIGHTS_EXECUTE,
            WBEMCONSUMER_APPID))
    {
        return dwRet;
    }

    if (dwRet = setDCOMAppLaunch(
            sidAdministrators,
            COM_RIGHTS_EXECUTE,
            WBEMCONSUMER_APPID))
    {
        return dwRet;
    }

    //free allocated SIDs
    FreeSid(sidAdministrators);
    FreeSid(sidAnonymous);
    FreeSid(sidEveryone);

    return ERROR_SUCCESS;
}

DWORD setDCOMAppAccess(
    PSID sid,
    DWORD dwPermissionMask,
    LPTSTR AppID)
{
    TCHAR   keyName [256];
    wsprintf (keyName, TEXT("APPID\\%s"), AppID);
    
    return setDCOMPermission (
        HKEY_CLASSES_ROOT,
        keyName,
        sid, 
        TEXT("AccessPermission"),
        dwPermissionMask,
        FALSE);
}

DWORD setDCOMAppLaunch(
    PSID sid,
    DWORD dwPermissionMask,
    LPTSTR AppID)
{
    TCHAR   keyName [256];
    wsprintf (keyName, TEXT("APPID\\%s"), AppID);

    return setDCOMPermission(
        HKEY_CLASSES_ROOT,
        keyName,
        sid,
        TEXT("LaunchPermission"),
        dwPermissionMask, 
        FALSE);
}


DWORD setDCOMAccessLimits (
    PSID sid,
    DWORD dwPermissionMask)
{
    TCHAR keyName [256];

    wsprintf (keyName, TEXT("SOFTWARE\\Microsoft\\Ole"));

    return setDCOMPermission(
        HKEY_LOCAL_MACHINE,
        keyName,
        sid, 
        TEXT("MachineAccessRestriction"),
        dwPermissionMask);
}

DWORD setDCOMLaunchLimits (
    PSID sid,
    DWORD dwPermissionMask)
{
    TCHAR keyName [256];
    wsprintf (keyName, TEXT("SOFTWARE\\Microsoft\\Ole"));

    return setDCOMPermission(
        HKEY_LOCAL_MACHINE,
        keyName,
        sid,
        TEXT("MachineLaunchRestriction"),
        dwPermissionMask);
}

DWORD setDCOMPermission(
    HKEY RootKey, 
    LPTSTR KeyName, 
    PSID sid,
    LPTSTR ValueName,
    DWORD dwPermissionMask,
    BOOL bLimits)
{
    DWORD returnValue;
    SECURITY_DESCRIPTOR *sd = NULL;
    SECURITY_DESCRIPTOR *sdSelfRelative = NULL;
    SECURITY_DESCRIPTOR *sdAbsolute = NULL;
    DWORD secDescSize;
    BOOL present;
    BOOL defaultDACL;
    PACL dacl;
    BOOL newSD = FALSE;

    returnValue = GetNamedValueSD (RootKey, KeyName, ValueName, &sd, &newSD);

    //
    // Get security descriptor from registry or create a new one
    //

    if (returnValue != ERROR_SUCCESS)
    {
        return returnValue;
    }

    if (!GetSecurityDescriptorDacl (sd, &present, &dacl, &defaultDACL))
    {
        return GetLastError();
    }

    if ((newSD) && (!bLimits))
    {
        AddAccessAllowedACEToACL(
            &dacl, 
            COM_RIGHTS_EXECUTE,
            TEXT("SYSTEM"));
            
        AddAccessAllowedACEToACL(
            &dacl,
            COM_RIGHTS_EXECUTE,
            TEXT("INTERACTIVE"));
    }

    //get account according to the SID
    TCHAR userName[256];
    TCHAR acctName[256];
    TCHAR domainName[256];
    DWORD dwAcctName = 256;
    DWORD dwDomainName = 256;
    SID_NAME_USE eUse = SidTypeUnknown;

    if (LookupAccountSid(
        NULL,
        sid,
        acctName,
        (LPDWORD)&dwAcctName,
        domainName,
        &dwDomainName,
        &eUse)) 
    {
        sprintf(userName, "%s\\%s", domainName, acctName);
    }
    else
    {
        return GetLastError();
    }

    returnValue = AddAccessAllowedACEToACL (
        &dacl, 
        dwPermissionMask, 
        userName); 
    

    if (returnValue != ERROR_SUCCESS)
    {
        free (sd);
        return returnValue;
    }

    //
    // Make the security descriptor absolute if it isn't new
    //

    if (!newSD)
    {
        MakeSDAbsolute (
            (PSECURITY_DESCRIPTOR) sd, 
            (PSECURITY_DESCRIPTOR *) &sdAbsolute);
    } 
    else 
    {
        sdAbsolute = sd;
    }

    //
    // Set the discretionary ACL on the security descriptor
    //

    if (!SetSecurityDescriptorDacl (sdAbsolute, TRUE, dacl, FALSE))
        return GetLastError();

    //
    // Make the security descriptor self-relative so that we can
    // store it in the registry
    //

    secDescSize = 0;
    MakeSelfRelativeSD (sdAbsolute, sdSelfRelative, &secDescSize);
    sdSelfRelative = (SECURITY_DESCRIPTOR *) malloc (secDescSize);
    
    if (!MakeSelfRelativeSD (sdAbsolute, sdSelfRelative, &secDescSize))
    {
        return GetLastError();
    }

    //
    // Store the security descriptor in the registry
    //

    SetNamedValueSD (RootKey, KeyName, ValueName, sdSelfRelative);

    free (sd);
    free (sdSelfRelative);
    
    if (sd != sdAbsolute) 
    {
        free (sdAbsolute);
    }

    return ERROR_SUCCESS;
}

DWORD CreateNewSD (
    SECURITY_DESCRIPTOR **SD)
{
    PACL dacl;
    DWORD sidLength;
    PSID sid;
    PSID groupSID;
    PSID ownerSID;
    DWORD returnValue;

    *SD = NULL;

    returnValue = GetCurrentUserSID (&sid);
    if (returnValue != ERROR_SUCCESS)
    {
        return returnValue;
    }

    sidLength = GetLengthSid (sid);

    *SD = (SECURITY_DESCRIPTOR *) malloc (
        (sizeof (ACL)+sizeof (ACCESS_ALLOWED_ACE)+sidLength) +
        (2 * sidLength) +
        sizeof (SECURITY_DESCRIPTOR));

    groupSID = (SID *) (*SD + 1);
    ownerSID = (SID *) (((BYTE *) groupSID) + sidLength);
    dacl = (ACL *) (((BYTE *) ownerSID) + sidLength);

    if (!InitializeSecurityDescriptor (*SD, SECURITY_DESCRIPTOR_REVISION))
    {
        free (*SD);
        free (sid);
        return GetLastError();
    }

    if (!InitializeAcl (dacl,
        sizeof (ACL)+sizeof (ACCESS_ALLOWED_ACE)+sidLength,
        ACL_REVISION2))
    {
        free (*SD);
        free (sid);
        return GetLastError();
    }

    if (!AddAccessAllowedAce (dacl,
        ACL_REVISION2,
        COM_RIGHTS_EXECUTE,
        sid))
    {
        free (*SD);
        free (sid);
        return GetLastError();
    }

    if (!SetSecurityDescriptorDacl (*SD, TRUE, dacl, FALSE))
    {
        free (*SD);
        free (sid);
        return GetLastError();
    }

    memcpy (groupSID, sid, sidLength);
    if (!SetSecurityDescriptorGroup (*SD, groupSID, FALSE))
    {
        free (*SD);
        free (sid);
        return GetLastError();
    }

    memcpy (ownerSID, sid, sidLength);
    if (!SetSecurityDescriptorOwner (*SD, ownerSID, FALSE))
    {
        free (*SD);
        free (sid);
        return GetLastError();
    }
    return ERROR_SUCCESS;
}


DWORD MakeSDAbsolute (
    PSECURITY_DESCRIPTOR OldSD,
    PSECURITY_DESCRIPTOR *NewSD)
{
    PSECURITY_DESCRIPTOR sd = NULL;
    DWORD descriptorSize;
    DWORD daclSize;
    DWORD saclSize;
    DWORD ownerSIDSize;
    DWORD groupSIDSize;
    PACL dacl;
    PACL sacl;
    PSID ownerSID;
    PSID groupSID;
    BOOL present;
    BOOL systemDefault;

    //
    // Get SACL
    //

    if (!GetSecurityDescriptorSacl (OldSD, &present, &sacl, &systemDefault))
    {
        return GetLastError();
    }       

    if (sacl && present)
    {
        saclSize = sacl->AclSize;
    } 
    else 
    {
        saclSize = 0;
    }

    //
    // Get DACL
    //

    if (!GetSecurityDescriptorDacl (OldSD, &present, &dacl, &systemDefault))
    {
        return GetLastError();
    }

    if (dacl && present)
    {
        daclSize = dacl->AclSize;
    }
    else 
    {
        daclSize = 0;
    }

    //
    // Get Owner
    //

    if (!GetSecurityDescriptorOwner (OldSD, &ownerSID, &systemDefault))
    {
        return GetLastError();
    }

    ownerSIDSize = GetLengthSid (ownerSID);

    //
    // Get Group
    //

    if (!GetSecurityDescriptorGroup (OldSD, &groupSID, &systemDefault))
    {        
        return GetLastError();
    }

    groupSIDSize = GetLengthSid (groupSID);

    //
    // Do the conversion
    //

    descriptorSize = 0;

    MakeAbsoluteSD(
        OldSD, 
        sd,
        &descriptorSize,
        dacl,
        &daclSize,
        sacl,
        &saclSize,
        ownerSID,
        &ownerSIDSize,
        groupSID,
        &groupSIDSize);

    sd = (PSECURITY_DESCRIPTOR) new BYTE [SECURITY_DESCRIPTOR_MIN_LENGTH];

    if (!InitializeSecurityDescriptor (sd, SECURITY_DESCRIPTOR_REVISION))
    {
        return GetLastError();
    }

    if (!MakeAbsoluteSD(
            OldSD,
            sd,
            &descriptorSize,
            dacl,
            &daclSize,
            sacl,
            &saclSize,
            ownerSID,
            &ownerSIDSize,
            groupSID,
            &groupSIDSize))
    {
        return GetLastError();
    }

    *NewSD = sd;
    return ERROR_SUCCESS;
}

DWORD SetNamedValueSD (
    HKEY RootKey,
    LPTSTR KeyName,
    LPTSTR ValueName,
    SECURITY_DESCRIPTOR *SD)
{
    DWORD returnValue;
    DWORD disposition;
    HKEY registryKey;

    //
    // Create new key or open existing key
    //

    returnValue = RegCreateKeyEx(
        RootKey, 
        KeyName,
        0,
        TEXT(""),
        0,
        KEY_ALL_ACCESS,
        NULL,
        &registryKey,
        &disposition);
            
    if (returnValue != ERROR_SUCCESS)
    {
        return returnValue;
    }

    //
    // Write the security descriptor
    //

    returnValue = RegSetValueEx(
        registryKey,
        ValueName,
        0,
        REG_BINARY,
        (LPBYTE) SD,
        GetSecurityDescriptorLength (SD));
            
    if (returnValue != ERROR_SUCCESS)
    {
        return returnValue;
    }
    
    RegCloseKey (registryKey);
    return ERROR_SUCCESS;
}

DWORD GetNamedValueSD (
    HKEY RootKey,
    LPTSTR KeyName,
    LPTSTR ValueName,
    SECURITY_DESCRIPTOR **SD,
    BOOL *NewSD)
{
    DWORD returnValue;
    HKEY registryKey;
    DWORD valueType;
    DWORD valueSize;

    *NewSD = FALSE;

    //
    // Get the security descriptor from the named value. If it doesn't
    // exist, create a fresh one.
    //

    returnValue = RegOpenKeyEx(
        RootKey,
        KeyName,
        0,
        KEY_ALL_ACCESS,
        &registryKey);

    if (returnValue != ERROR_SUCCESS)
    {
        if (returnValue == ERROR_FILE_NOT_FOUND)
        {
            *SD = NULL;
            returnValue = CreateNewSD (SD);
            if (returnValue != ERROR_SUCCESS)
            {
                return returnValue;
            }

            *NewSD = TRUE;
            return ERROR_SUCCESS;
        } 
        else
        {
            return returnValue;
        }
    }

    returnValue = RegQueryValueEx(
        registryKey, 
        ValueName,
        NULL,
        &valueType,
        NULL,
        &valueSize);

    if (returnValue && returnValue != ERROR_INSUFFICIENT_BUFFER)
    {
        *SD = NULL;
        returnValue = CreateNewSD (SD);
        if (returnValue != ERROR_SUCCESS)
        {
            return returnValue;
        }

        *NewSD = TRUE;
    } 
    else
    {
        *SD = (SECURITY_DESCRIPTOR *) malloc (valueSize);

        returnValue = RegQueryValueEx(
            registryKey,
            ValueName,
            NULL,
            &valueType,
            (LPBYTE) *SD,
            &valueSize);
            
        if (returnValue)
        {
            free (*SD);

            *SD = NULL;
            returnValue = CreateNewSD (SD);
            if (returnValue != ERROR_SUCCESS)
            {
                return returnValue;
            }

            *NewSD = TRUE;
        }
    }

    RegCloseKey (registryKey);

    return ERROR_SUCCESS;
}


DWORD GetCurrentUserSID (
    PSID *Sid)
{
    TOKEN_USER *tokenUser = NULL;
    HANDLE tokenHandle;
    DWORD tokenSize;
    DWORD sidLength;

    if (OpenProcessToken (GetCurrentProcess(), TOKEN_QUERY, &tokenHandle))
    {
        GetTokenInformation (tokenHandle,
            TokenUser,
            tokenUser,
            0,
            &tokenSize);

        tokenUser = (TOKEN_USER *) malloc (tokenSize);

        if (GetTokenInformation(tokenHandle,
            TokenUser,
            tokenUser,
            tokenSize,
            &tokenSize))
        {
            sidLength = GetLengthSid (tokenUser->User.Sid);
            *Sid = (PSID) malloc (sidLength);

            memcpy (*Sid, tokenUser->User.Sid, sidLength);
            CloseHandle (tokenHandle);
        } 
        else
        {
            free (tokenUser);
            return GetLastError();
        }
    } 
    else
    {
        free (tokenUser);
        return GetLastError();
    }

    free (tokenUser);
    return ERROR_SUCCESS;
}

DWORD GetPrincipalSID (
    LPTSTR Principal,
    PSID *Sid)
{
    DWORD sidSize;
    TCHAR refDomain [256];
    DWORD refDomainSize;
    DWORD returnValue;
    SID_NAME_USE snu;

    sidSize = 0;
    refDomainSize = 255;

    LookupAccountName(
        NULL,
        Principal,
        *Sid,
        &sidSize,
        refDomain,
        &refDomainSize,
        &snu);

    returnValue = GetLastError();
    if (returnValue != ERROR_INSUFFICIENT_BUFFER)
    {
        return returnValue;
    }

    *Sid = (PSID) malloc (sidSize);
    refDomainSize = 255;

    if (!LookupAccountName (
            NULL,
            Principal,
            *Sid,
            &sidSize,
            refDomain,
            &refDomainSize,
            &snu))
    {
        return GetLastError();
    }

    return ERROR_SUCCESS;
}

DWORD CopyACL(
    PACL OldACL,
    PACL NewACL)
{
    ACL_SIZE_INFORMATION aclSizeInfo;
    LPVOID ace;
    ACE_HEADER *aceHeader;
    ULONG i;

    GetAclInformation(
        OldACL,
        (LPVOID) &aclSizeInfo,
        (DWORD) sizeof (aclSizeInfo),
        AclSizeInformation);

    //
    // Copy all of the ACEs to the new ACL
    //

    for (i = 0; i < aclSizeInfo.AceCount; i++)
    {
        //
        // Get the ACE and header info
        //

        if (!GetAce (OldACL, i, &ace))
        {
            return GetLastError();
        }

        aceHeader = (ACE_HEADER *) ace;

        //
        // Add the ACE to the new list
        //

        if (!AddAce(
            NewACL,
            ACL_REVISION,
            0xffffffff,
            ace,
            aceHeader->AceSize))
        {
            return GetLastError();
        }
    }

    return ERROR_SUCCESS;
}

DWORD AddAccessAllowedACEToACL(
    PACL *Acl,
    DWORD PermissionMask,
    LPTSTR Principal)
{
    ACL_SIZE_INFORMATION aclSizeInfo;
    int aclSize;
    DWORD returnValue;
    PSID principalSID;
    PACL oldACL;
    PACL newACL;

    oldACL = *Acl;

    returnValue = GetPrincipalSID (Principal, &principalSID);
    if (returnValue != ERROR_SUCCESS)
    {
        return returnValue;
    }

    GetAclInformation(
        oldACL,
        (LPVOID) &aclSizeInfo,
        (DWORD) sizeof (ACL_SIZE_INFORMATION),
        AclSizeInformation);

    aclSize = aclSizeInfo.AclBytesInUse +
        sizeof (ACL) + 
        sizeof (ACCESS_ALLOWED_ACE) +
        GetLengthSid (principalSID) - 
        sizeof (DWORD);

    newACL = (PACL) new BYTE [aclSize];

    if (!InitializeAcl (newACL, aclSize, ACL_REVISION))
    {
        free (principalSID);
        return GetLastError();
    }

    returnValue = CopyACL (oldACL, newACL);
    if (returnValue != ERROR_SUCCESS)
    {
        free (principalSID);
        return returnValue;
    }

    if (!AddAccessAllowedAce(
        newACL,
        ACL_REVISION2,
        PermissionMask,
        principalSID))
    {
        free (principalSID);
        return GetLastError();
    }

    *Acl = newACL;

    free (principalSID);
    return ERROR_SUCCESS;
}

