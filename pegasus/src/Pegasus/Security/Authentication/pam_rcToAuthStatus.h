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

#ifndef pam_rcToAuthStatus_h
#define pam_rcToAuthStatus_h

#include <security/pam_appl.h>
#include <Pegasus/Security/Authentication/Linkage.h>
#include <Pegasus/Security/Authentication/AuthenticationStatus.h>

PEGASUS_NAMESPACE_BEGIN

static AuthenticationStatus _getAuthStatusFromPAM_RC(int pam_rc)
{
    enum AuthenticationStatusCode authCode;

    switch (pam_rc)
    {
        case PAM_SUCCESS:
        {
            authCode = AUTHSC_SUCCESS;
            break;
        }
        case PAM_NEW_AUTHTOK_REQD:
        {
            authCode = AUTHSC_PASSWORD_CHG_REQUIRED;
            break;
        }
        case PAM_ACCT_EXPIRED:
        {
            authCode = AUTHSC_ACCOUNT_EXPIRED;
            break;
        }
        case PAM_AUTH_ERR:
        case PAM_USER_UNKNOWN:
        {
            authCode = AUTHSC_UNAUTHORIZED;
            break;
        }
        case PAM_CRED_UNAVAIL:
        {
            authCode = AUTHSC_NO_ROLE_DEFINED_FOR_USER;
            break;
        }
        case PAM_CRED_EXPIRED:
        {
            authCode = AUTHSC_PASSWORD_EXPIRED;
            break;
        }
        case PAM_AUTHTOK_LOCK_BUSY:
        {
            authCode = AUTHSC_AUTHTOK_LOCKED;
            break;
        }
        case PAM_CRED_INSUFFICIENT:
        {
            authCode = AUTHSC_CRED_INSUFFICIENT;
            break;
        }
        case PAM_SERVICE_ERR:
        case PAM_SYSTEM_ERR:
        {
            authCode = AUTHSC_SERVICE_ERR;
            break;
        }
        case PAM_AUTHINFO_UNAVAIL:
        {
            authCode = AUTHSC_SERVICE_UNAVAILABLE;
            break;
        }
        default:
        {
            authCode = AUTHSC_UNAUTHORIZED;
        }
    }
    return AuthenticationStatus(authCode);
}

PEGASUS_NAMESPACE_END

#endif /* pam_rcToAuthStatus_h */
