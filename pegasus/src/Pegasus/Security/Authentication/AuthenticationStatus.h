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

#ifndef AuthenticationStatus_h
#define AuthenticationStatus_h

#include <Pegasus/Security/Authentication/Linkage.h>
#include <Pegasus/Common/String.h>

PEGASUS_NAMESPACE_BEGIN


enum AuthenticationStatusCode
{
    AUTHSC_SUCCESS,
    AUTHSC_UNAUTHORIZED,
    AUTHSC_PASSWORD_CHG_REQUIRED,
    AUTHSC_ACCOUNT_EXPIRED,
    AUTHSC_SERVICE_UNAVAILABLE,
    AUTHSC_NO_ROLE_DEFINED_FOR_USER,
    AUTHSC_PASSWORD_EXPIRED,
    AUTHSC_AUTHTOK_LOCKED,
    AUTHSC_SERVICE_ERR,
    AUTHSC_CRED_INSUFFICIENT
};

class PEGASUS_SECURITY_LINKAGE AuthenticationStatus
{
public:

    /** Constructors  */
    AuthenticationStatus(AuthenticationStatusCode code);
    AuthenticationStatus(Boolean authenticated);

    /** Destructor  */
    ~AuthenticationStatus() { };

    bool isSuccess()
    {
        return (_code == AUTHSC_SUCCESS);
    }

    bool isPasswordExpired()
    {
        return ((_code == AUTHSC_PASSWORD_EXPIRED) ||
                (_code == AUTHSC_PASSWORD_CHG_REQUIRED));
    }

    bool doChallenge()
    {
        return _doChallenge;
    }

    String getHttpStatus()
    {
        return _httpStatus;
    }

    String getErrorDetail()
    {
        return _errorDetail;
    }
    
    AuthenticationStatusCode getMethodReturnCode() const
    {
        return _code;
    }
    
private:

    /* Prohibit usage of default constructor */
    AuthenticationStatus();

    String _httpStatus;
    String _errorDetail;
    AuthenticationStatusCode _code;
    bool _doChallenge;
};

PEGASUS_NAMESPACE_END

#endif /* AuthenticationStatus_h */
