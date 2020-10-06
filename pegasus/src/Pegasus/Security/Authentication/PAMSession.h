/*
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
*/

#ifndef PAMSession_h
#define PAMSession_h

#include <security/pam_appl.h>
#include <Pegasus/Security/Authentication/Linkage.h>
#include <Pegasus/Common/Logger.h>
#include <Pegasus/Common/AuthenticationInfo.h>


#ifdef PEGASUS_FLAVOR
# define PAM_CONFIG_FILE "wbem" PEGASUS_FLAVOR
#else
# define PAM_CONFIG_FILE "wbem"
#endif

PEGASUS_NAMESPACE_BEGIN

/*
**==============================================================================
**
** struct PAMData
**
**     Client data passed to PAM routines.
**
**==============================================================================
*/

typedef struct PAMDataStruct
{
    const char* password;
    const char* newpassword;
}
PAMData;

/*
**==============================================================================
**
** PAMAuthenticateCallback()
**
**     Callback used by PAMAuthenticate().
**
**==============================================================================
*/

static void _freePAMResponse(int numMsg, struct pam_response* rsp)
{
    // rsp is initialized to zero's, can call free() since it does check for
    // Null Pointer and a failed malloc does not change pointer value
    for (int i = 0; i < numMsg; i++)
    {
        if (rsp[i].resp != NULL)
        {
            memset(rsp[i].resp, 0, PAM_MAX_MSG_SIZE);
            free(rsp[i].resp);
        }
    }
    free(rsp);
}

static int PAMAuthenticateCallback(
    int num_msg,
#if defined(PEGASUS_OS_LINUX)
    const struct pam_message** msg,
#else
    struct pam_message** msg,
#endif
    struct pam_response** response,
    void* appdata_ptr)
{
    PAMData* data = (PAMData*)appdata_ptr;
    struct pam_response *reply;
    int i;

    if (num_msg > 0)
    {
        reply = (struct pam_response*)calloc(
            num_msg, sizeof(struct pam_response));

        if (reply == NULL)
            return PAM_BUF_ERR;
    }
    else
    {
        return PAM_CONV_ERR;
    }

    for (i = 0; i < num_msg; i++)
    {
        switch (msg[i]->msg_style)
        {
            case PAM_PROMPT_ECHO_OFF:
            {
                reply[i].resp = (char*)malloc(PAM_MAX_MSG_SIZE);
                if (reply[i].resp == NULL)
                {
                    _freePAMResponse(num_msg, reply);
                    return PAM_BUF_ERR;
                }
                strncpy(reply[i].resp, data->password, PAM_MAX_MSG_SIZE);
                reply[i].resp_retcode = 0;
                break;
            }

            default:
            {
                _freePAMResponse(num_msg, reply);
                return PAM_CONV_ERR;
            }
        }
    }
    *response = reply; 
    return PAM_SUCCESS;  
}

/*
**==============================================================================
**
** PAMUpdateExpiredPasswordCallback()
**
**     Callback used by _PAMUpdateExpiredPassword().
**
**==============================================================================
*/

static int PAMUpdateExpiredPasswordCallback(
    int num_msg,
#if defined(PEGASUS_OS_LINUX)
    const struct pam_message** msg,
#else
    struct pam_message** msg,
#endif
    struct pam_response** response,
    void* appdata_ptr)
{
    PAMData* data = (PAMData*)appdata_ptr;
    struct pam_response *reply;
    int i;

    if (num_msg > 0)
    {
        reply = (struct pam_response*)calloc(
            num_msg, sizeof(struct pam_response));

        if (reply == NULL)
            return PAM_BUF_ERR;
    }
    else
    {
        return PAM_CONV_ERR;
    }

    for (i = 0; i < num_msg; i++)
    {
        switch (msg[i]->msg_style)
        {
            case PAM_PROMPT_ECHO_OFF:
            {
                reply[i].resp = (char*)malloc(PAM_MAX_MSG_SIZE);
                if (reply[i].resp == NULL)
                {
                    _freePAMResponse(num_msg, reply);
                    return PAM_BUF_ERR;
                }
                if (i > 0)
                {
                    strncpy(reply[i].resp, data->newpassword, PAM_MAX_MSG_SIZE);
                }
                else
                {
                    strncpy(reply[0].resp, data->password, PAM_MAX_MSG_SIZE);
                }
                reply[i].resp_retcode = 0;
                break;
            }

            default:
            {
                _freePAMResponse(num_msg, reply);
                return PAM_CONV_ERR;
            }
        }
    }
    *response = reply; 
    return PAM_SUCCESS;
}

/*
**==============================================================================
**
** PAMValidateUserCallback()
**
**     Callback used by PAMValidateUser().
**
**==============================================================================
*/

static int PAMValidateUserCallback(
    int num_msg,
#if defined(PEGASUS_OS_LINUX)
    const struct pam_message** msg,
#else
    struct pam_message** msg,
#endif
    struct pam_response** response,
    void* appdata_ptr)
{
 
    struct pam_response* reply;
    msg = 0;

    appdata_ptr = 0;

    if (num_msg > 0)
    {
        reply = (struct pam_response*)calloc(
            num_msg, sizeof(struct pam_response));

        if (reply == NULL)
            return PAM_BUF_ERR;
    }
    else
    {
        return PAM_CONV_ERR;
    }
    *response = reply;
    return PAM_SUCCESS;
}

static void _logPAMError(
    pam_handle_t* handle,
    const char* functionName,
    int pamrc)
{
    const char * errorText;
    if (0 == handle)
    {
        errorText = "";
    }
    else
    {
        errorText = pam_strerror(handle, pamrc);
    }
    Logger::put_l(Logger::STANDARD_LOG, System::CIMSERVER, Logger::WARNING,
        MessageLoaderParms(
            "Security.Authentication.PAMSession.PAM_FUNCTION_FAILED",
            "Error in PAM function $0 (errorCode=$1, errorText=\'$2\')",
            functionName,
            pamrc,
            errorText));
}

static int _preparePAM(
    bool authenticate,
    pam_handle_t** handle,
    PAMData* data,
    const char* username,
    const char* password,
    AuthenticationInfo * authInfo)
{
    struct pam_conv pconv;
    int pam_rc;

    if (authenticate)
    {
        pconv.conv = PAMAuthenticateCallback;
        data->password = password;
    }
    else
    {
        pconv.conv = PAMValidateUserCallback;
    }
    pconv.appdata_ptr = data;

    pam_rc = pam_start(PAM_CONFIG_FILE,username,&pconv,handle);

    if (pam_rc != PAM_SUCCESS)
    {
        _logPAMError(0, "pam_start", pam_rc);
        return pam_rc;
    }

    CString ipAddress = authInfo->getIpAddress().getCString();
    
    PEG_TRACE((TRC_AUTHENTICATION,Tracer::LEVEL3,
        "Setting PAM_RHOST for user %s to %s",
        username,
        (const char*) ipAddress));

    pam_rc = pam_set_item(
        *handle,
        PAM_RHOST,
        (const char*) ipAddress);

    if (pam_rc !=PAM_SUCCESS)
    {
        _logPAMError(*handle, "pam_set_item(PAM_RHOST)", pam_rc);
        pam_end(*handle, 0);
        return pam_rc;
    }
    return PAM_SUCCESS;
}

/*
**==============================================================================
**
** _PAMAuthenticate()
**
**     Performs basic PAM authentication on the given username and password.
**
**==============================================================================
*/
static int _PAMAuthenticate(
    const char* username,
    const char* password,
    AuthenticationInfo * authInfo)
{
    pam_handle_t* handle;
    int pam_rc;
    PAMData data;

    /* commented out statement in place to allow triggering a Http 500 Error */
    /* intentionally for testing purposes */
    // return PAM_SERVICE_ERR;

    // commented out statement in place to allow triggering a Password Expired
    // intentionally for testing purposes
    // return PAM_CRED_EXPIRED;

    pam_rc = _preparePAM(true, &handle, &data, username, password, authInfo);
    if (pam_rc != PAM_SUCCESS)
    {
        return pam_rc;
    }

    pam_rc = pam_authenticate(handle, 0);
    if (pam_rc != PAM_SUCCESS)
    {
        _logPAMError(handle, "pam_authenticate", pam_rc);
        pam_end(handle, 0);
        return pam_rc;
    }

    // uncomment the following line for testing purposes
    // pam_putenv(handle, "CMPIRole=UserTestRole4711");

    String userRole;
    const char* role = pam_getenv(handle, "CMPIRole");
    if (NULL != role)
    {
        userRole.assign(role);
    }

    pam_rc = pam_acct_mgmt(handle, 0);
    if (pam_rc != PAM_SUCCESS)
    {
        _logPAMError(handle, "pam_acct_mgmt", pam_rc);
        pam_end(handle, 0);
        return pam_rc;
    }

    // Delayed on success: -> pam_end(handle, 0);
    // Cleanup will happen through destroy() function of AuthHandle in
    // HTTPConnection
    AuthHandle myAuth;
    myAuth.hdl = handle;
    authInfo->setAuthHandle(myAuth);
    authInfo->setUserRole(userRole);

    return pam_rc;
}

/*
**==============================================================================
**
** _PAMValidateUser()
**
**     Validate that the *username* refers to a valid PAM user.
**
**==============================================================================
*/

static int _PAMValidateUser(
    const char* username,
    AuthenticationInfo * authInfo)
{
    pam_handle_t* handle;
    PAMData data;
    int pam_rc;

    pam_rc = _preparePAM(false, &handle, &data, username, 0, authInfo);
    if (pam_rc != PAM_SUCCESS)
    {
        return pam_rc;
    }

    pam_rc = pam_acct_mgmt(handle, 0);
    if (pam_rc != PAM_SUCCESS)
    {
        _logPAMError(handle, "pam_acct_mgmt", pam_rc);
        pam_end(handle, 0);
        return pam_rc;
    }

    pam_end(handle, 0);

    return pam_rc;
}

/*
**==============================================================================
**
** _PAMUpdateExpiredPassword()
**
**     Update the password for user: *username*
**
**==============================================================================
*/

static int _PAMUpdateExpiredPassword(
    const char* username,
    const char* oldpass,
    const char* newpass,
    const char* ipaddress)
{
    pam_handle_t* handle;
    PAMData data;
    struct pam_conv pconv;
    int pam_rc;

    pconv.conv = PAMUpdateExpiredPasswordCallback;
    data.password = oldpass;
    data.newpassword = newpass;

    pconv.appdata_ptr = &data;

    pam_rc = pam_start(PAM_CONFIG_FILE,username,&pconv,&handle);

    if (pam_rc != PAM_SUCCESS)
    {
        _logPAMError(0, "pam_start", pam_rc);
        return pam_rc;
    }
  
    // set the PAM_RHOST so product can audit log the ip  
    // address of the remote host that is changing the password
 
    pam_rc = pam_set_item(
        handle,
        PAM_RHOST,
        ipaddress);  

    if (pam_rc !=PAM_SUCCESS)
    {
        _logPAMError(handle, "pam_set_item(PAM_RHOST)", pam_rc);
        pam_end(handle, 0);
        return pam_rc;
    }
    pam_rc = pam_authenticate(handle, 0);
    if (pam_rc != PAM_SUCCESS)
    {
        _logPAMError(handle, "pam_authenticate", pam_rc);
        pam_end(handle, 0);
        return pam_rc;
    }
    
    pam_rc = pam_acct_mgmt(handle, 0);
    if ( (pam_rc != PAM_CRED_EXPIRED) &&
         (pam_rc != PAM_NEW_AUTHTOK_REQD))
    {
        _logPAMError(handle, "pam_acct_mgmt", pam_rc);
        pam_end(handle, 0);
        return pam_rc;
    }

    pam_rc = pam_chauthtok(handle, PAM_CHANGE_EXPIRED_AUTHTOK);
    if (pam_rc != PAM_SUCCESS)
    {
        _logPAMError(handle, "pam_chauthtok", pam_rc);
    }

    pam_end(handle, 0);

    return pam_rc;
}


PEGASUS_NAMESPACE_END

#endif /* PAMSession_h */
