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

#include "AuthenticationStatus.h"
#include <Pegasus/Common/Constants.h>

PEGASUS_NAMESPACE_BEGIN

AuthenticationStatus::AuthenticationStatus(AuthenticationStatusCode code):
    _code(code),
    _doChallenge(true)
{
    switch (_code)
    {
        case AUTHSC_SUCCESS:
        {
            _httpStatus.assign(HTTP_STATUS_OK);
            _doChallenge = false;
            break;
        }
        case AUTHSC_UNAUTHORIZED:
        {
            _httpStatus.assign(HTTP_STATUS_UNAUTHORIZED);
            _errorDetail.assign("User Unauthorized");
            break;
        }
        case AUTHSC_PASSWORD_CHG_REQUIRED:
        {
            _httpStatus.assign(HTTP_STATUS_UNAUTHORIZED);
            _errorDetail.assign("Password Change Required");
            break;
        }
        case AUTHSC_ACCOUNT_EXPIRED:
        {
            _httpStatus.assign(HTTP_STATUS_UNAUTHORIZED);
            _errorDetail.assign("User Account Has Expired");
            break;
        }
        case AUTHSC_SERVICE_UNAVAILABLE:
        {
            _httpStatus.assign(HTTP_STATUS_SERVICEUNAVAILABLE);
            _doChallenge = false;
            break;
        }
        case AUTHSC_NO_ROLE_DEFINED_FOR_USER:
        {
            _httpStatus.assign(HTTP_STATUS_UNAUTHORIZED);
            _errorDetail.assign("No Role Defined For User");
            break;
        }
        case AUTHSC_PASSWORD_EXPIRED:
        {
            _httpStatus.assign(HTTP_STATUS_UNAUTHORIZED);
            _errorDetail.assign("Password Expired");
            break;
        }
        case AUTHSC_AUTHTOK_LOCKED:
        {
            _httpStatus.assign(HTTP_STATUS_UNAUTHORIZED);
            _errorDetail.assign("User Account Locked");
            break;
        }
        case AUTHSC_SERVICE_ERR:
        {
            _httpStatus.assign(HTTP_STATUS_INTERNALSERVERERROR);
            _doChallenge = false;
            break;
        }
        case AUTHSC_CRED_INSUFFICIENT:
        {
            _httpStatus.assign(HTTP_STATUS_UNAUTHORIZED);
            _errorDetail.assign("Insufficient Privilege");
            break;
        }
        default:
        {
            _httpStatus.assign(HTTP_STATUS_UNAUTHORIZED);
            break;
        }
    }
}

AuthenticationStatus::AuthenticationStatus(Boolean authenticated)
{
    if (authenticated)
    {
        _code = AUTHSC_SUCCESS;
        _httpStatus.assign(HTTP_STATUS_OK);
        _doChallenge = false;
    }
    else
    {
        _code = AUTHSC_UNAUTHORIZED;
        _httpStatus.assign(HTTP_STATUS_UNAUTHORIZED);
        _doChallenge = true;
    }
}


PEGASUS_NAMESPACE_END
