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


#include <Pegasus/Common/System.h>
#include <Pegasus/Common/Tracer.h>
#include <Pegasus/Config/ConfigManager.h>
#include <Pegasus/Security/UserManager/UserExceptions.h>
#include "SecureBasicAuthenticator.h"
#include <Pegasus/Common/Executor.h>

#ifdef PEGASUS_OS_ZOS
#include <pwd.h>
#ifdef PEGASUS_ZOS_SECURITY
// This include file will not be provided in the OpenGroup CVS for now.
// Do NOT try to include it in your compile
#include <Pegasus/Common/safCheckzOS_inline.h>
#endif
#endif

#ifdef PEGASUS_OS_PASE
#include <ILEWrapper/ILEUtilities2.h>
#endif

PEGASUS_USING_STD;

PEGASUS_NAMESPACE_BEGIN


/**
    Constant representing the Basic authentication challenge header.
*/
static const String BASIC_CHALLENGE_HEADER = "WWW-Authenticate: Basic ";

#ifdef PEGASUS_PLATFORM_ZOS_ZSERIES_IBM
# if (__TARGET_LIB__ < 0x410A0000)

#define ZOS_PEGASUS_APPLID_LEN 7
static const char* ZOS_PEGASUS_APPLID =
                      "\xC3\xC6\xE9\xC1\xD7\xD7\xD3\x40"; // "CFZAPPL "
static const char* ZOS_PEGASUS_THLI_EYE_CATCHER = "\xE3\xC8\xD3\xC9"; // "THLI"
#endif // end __TARGET_LIB__
#endif // end PEGASUS_PLATFORM_ZOS_ZSERIES_IBM

/* constructor. */
SecureBasicAuthenticator::SecureBasicAuthenticator()
{
    PEG_METHOD_ENTER(TRC_AUTHENTICATION,
        "SecureBasicAuthenticator::SecureBasicAuthenticator()");

    // Build Authentication parameter realm required for Basic Challenge
    // e.g. realm="HostName"

    _realm.assign("realm=");
    _realm.append(Char16('"'));
    _realm.append(System::getHostName());
    _realm.append(Char16('"'));

    // Get a user manager instance handler
#ifndef PEGASUS_PAM_AUTHENTICATION
    _userManager = UserManager::getInstance();
#endif

#ifdef PEGASUS_OS_ZOS
    ConfigManager* configManager = ConfigManager::getInstance();

    if (ConfigManager::parseBooleanValue(
        configManager->getCurrentValue("enableCFZAPPLID")))
# if (__TARGET_LIB__ < 0x410A0000)
#ifdef PEGASUS_PLATFORM_ZOS_ZSERIES_IBM
    {

        //
        // Enable __passwd() for passticket validation
        // for APPLID CFZAPPL in this thread.
        //
        set_ZOS_ApplicationID();
    }
    else
    {
        Logger::put_l(Logger::STANDARD_LOG, ZOS_SECURITY_NAME, Logger::WARNING,
            MessageLoaderParms(
                "Security.Authentication.SecureBasicAuthenticator."
                     "APPLID_OMVSAPPL.PEGASUS_OS_ZOS",
                "CIM server authentication is using application ID OMVSAPPL."));
    }

    pthread_security_np(0,__USERID_IDENTITY,0,NULL,NULL,0);
#else
#error APPLID support is not available in 64-bit compilation mode before V1R10
#endif //PEGASUS_PLATFORM_ZOS_ZSERIES_IBM
#else
    {
        _zosAPPLID = "CFZAPPL";
    }
    else
    {
        _zosAPPLID = "OMVSAPPL";
        Logger::put_l(Logger::STANDARD_LOG, ZOS_SECURITY_NAME, Logger::WARNING,
            MessageLoaderParms(
                "Security.Authentication.SecureBasicAuthenticator."
                     "APPLID_OMVSAPPL.PEGASUS_OS_ZOS",
                "CIM server authentication is using application ID OMVSAPPL."));
    }
#endif // end __TARGET_LIB__
#endif // end PEGASUS_OS_ZOS

    PEG_METHOD_EXIT();
}

/* destructor. */
SecureBasicAuthenticator::~SecureBasicAuthenticator()
{
    PEG_METHOD_ENTER(TRC_AUTHENTICATION,
        "SecureBasicAuthenticator::~SecureBasicAuthenticator()");

    PEG_METHOD_EXIT();
}

AuthenticationStatus SecureBasicAuthenticator::authenticate(
    const String & userName,
    const String & password,
    AuthenticationInfo* authInfo)
{
    PEG_METHOD_ENTER(TRC_AUTHENTICATION,
        "SecureBasicAuthenticator::authenticate()");

    Boolean authenticated = false;

#ifdef PEGASUS_OS_ZOS
    if ( password.size() == 0 )
    {
         Logger::put_l(Logger::STANDARD_LOG, ZOS_SECURITY_NAME, Logger::WARNING,
             MessageLoaderParms(
                 "Security.Authentication.SecureBasicAuthenticator."
                     "EMPTY_PASSWD.PEGASUS_OS_ZOS",
                 "Request UserID $0 misses password.", userName));
        PEG_METHOD_EXIT();
        return (false);
     }

#if (__TARGET_LIB__ >= 0x410A0000)
    if (__passwd_applid((const char*) userName.getCString(),
             (const char*) password.getCString(), NULL,
             (const char*) _zosAPPLID.getCString()) == 0)
#else
    if (__passwd((const char*) userName.getCString(),
             (const char*) password.getCString(), NULL) == 0)
#endif
    {
#ifdef PEGASUS_ZOS_SECURITY
        if (CheckProfileCIMSERVclassWBEM( userName , __READ_RESOURCE))
        {
            authenticated = true;
        }
        else
        {
            authenticated = false;
            // no READ access to security resource profile CIMSERV CL(WBEM)
            Logger::put_l(
                Logger::STANDARD_LOG, ZOS_SECURITY_NAME, Logger::WARNING,
                MessageLoaderParms(
                    "Security.Authentication.SecureBasicAuthenticator."
                        "NOREAD_CIMSERV_ACCESS.PEGASUS_OS_ZOS",
                    "Request UserID $0 misses READ permission "
                        "to profile CIMSERV CL(WBEM).",
                    userName));
        }
#else
        authenticated = true;
#endif
    }
    else
    {
        authenticated = false;
        PEG_TRACE((TRC_AUTHENTICATION, Tracer::LEVEL1,
            "UserID %s not authenticated: %s (errno %d, reason code 0x%08X)",
             (const char*) userName.getCString(),
              strerror(errno),errno,__errno2()));
    }
#elif defined(PEGASUS_OS_PASE)
    CString userCStr = userName.getCString();
    CString pwCStr = password.getCString();

    if ((strlen(userCStr) == 0) || (strlen(pwCStr) == 0))
    {
        authenticated = false;
    }
    else
    {
        // this function only can be found in PASE environment
        authenticated = umeUserAuthenticate((const char *)userCStr,
                                            (const char *)pwCStr);
    }

#else /* DEFAULT (!PEGASUS_OS_ZOS) */

    // Check whether valid system user.

    if (!System::isSystemUser(userName.getCString()))
    {
        PEG_METHOD_EXIT();
        return AuthenticationStatus(authenticated);
    }

    try
    {
        // Verify the CIM user password. Use executor if present. Else, use
        // conventional method.

        if (Executor::detectExecutor() == 0)
        {
            if (Executor::authenticatePassword(
                userName.getCString(), password.getCString()) == 0)
            {
                authenticated = true;
            }
        }
        else
        {
#ifndef PEGASUS_PAM_AUTHENTICATION
            if (_userManager->verifyCIMUserPassword(userName, password))
                authenticated = true;
#endif
        }
    }
    catch(InvalidUser &)
    {
        PEG_METHOD_EXIT();
        return AuthenticationStatus(authenticated);
    }
    catch(Exception & e)
    {
        PEG_METHOD_EXIT();
        throw e;
    }

#endif /* DEFAULT (!PEGASUS_OS_ZOS) */

    PEG_METHOD_EXIT();

    return AuthenticationStatus(authenticated);
}

AuthenticationStatus SecureBasicAuthenticator::validateUser(
    const String& userName,
    AuthenticationInfo* authInfo)
{
    PEG_METHOD_ENTER(TRC_AUTHENTICATION,
        "SecureBasicAuthenticator::validateUser()");

    Boolean authenticated = false;

    if ( System::isSystemUser(userName.getCString()))
    {
#if defined (PEGASUS_OS_PASE)
        authenticated = true;
#else
        if (Executor::detectExecutor() == 0)
        {
            if (Executor::validateUser(userName.getCString()) != 0)
                authenticated = true;
        }
#ifndef PEGASUS_PAM_AUTHENTICATION
        else if (_userManager->verifyCIMUser(userName))
        {
            authenticated = true;
        }
#endif
#endif
    }

    PEG_METHOD_EXIT();
    return AuthenticationStatus(authenticated);
}

//
// Create authentication response header
//
String SecureBasicAuthenticator::getAuthResponseHeader()
{
    PEG_METHOD_ENTER(TRC_AUTHENTICATION,
        "SecureBasicAuthenticator::getAuthResponseHeader()");

    //
    // build response header using realm
    //
    String responseHeader = BASIC_CHALLENGE_HEADER;
    responseHeader.append(_realm);

    PEG_METHOD_EXIT();
    return responseHeader;
}

#ifdef PEGASUS_PLATFORM_ZOS_ZSERIES_IBM
# if (__TARGET_LIB__ < 0x410A0000)
// This function is only needed if the compile target system
// is z/OS R9 or earlier and 32 Bit !
Boolean SecureBasicAuthenticator::set_ZOS_ApplicationID( void )
{

    Boolean applIDset = false;

    char* psa  = 0;
    char* tcb  = *(char **)(psa  + 0x21C); // pas  + PSATOLD
    char* stcb = *(char **)(tcb  + 0x138); // tcb  + TCBSTCB
    char* otcb = *(char **)(stcb + 0x0D8); // stcb + STCBOTCB
    char* thli = *(char **)(otcb + 0x0BC); // otcb + OTCBTHLI

    // if I find the BPXYTHILI eye catcher everything went fine.
    if ( memcmp(ZOS_PEGASUS_THLI_EYE_CATCHER,thli,4) != 0 )
    {

        PEG_TRACE_CSTRING(TRC_AUTHENTICATION, Tracer::LEVEL1,
            "Could not set application ID CFZAPPL. "
            " BPXYTHLI control block not found.");

        applIDset = false;
    }
    else
    {
        // The size of applId: BPXYTHLI.THLIAPPLIDLEN
        char* thliApplIDLen = (char *)(thli + 0x052);
        // The applId: BPXYTHLI.THLIAPPLID
        char* thliApplID = (char *)(thli + 0x070);

        // The APPLID definition contains a padding space: + 1
        memcpy(thliApplID,ZOS_PEGASUS_APPLID,ZOS_PEGASUS_APPLID_LEN+1);
        *thliApplIDLen = ZOS_PEGASUS_APPLID_LEN;

        applIDset = true;
    }

    return applIDset;
}
#endif // end __TARGET_LIB__
#endif // end PEGASUS_PLATFORM_ZOS_ZSERIES_IBM

PEGASUS_NAMESPACE_END
