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

#ifndef Pegasus_SecureBasicAuthenticator_h
#define Pegasus_SecureBasicAuthenticator_h

#ifndef PEGASUS_PAM_AUTHENTICATION
# include <Pegasus/Security/UserManager/UserManager.h>
#endif

#include "BasicAuthenticator.h"

#include <Pegasus/Security/Authentication/Linkage.h>

PEGASUS_NAMESPACE_BEGIN

/** This class provides Secure basic authentication implementation by extending
    the BasicAuthenticator.
*/
class PEGASUS_SECURITY_LINKAGE SecureBasicAuthenticator
    : public BasicAuthenticator
{
public:

    /** constructor. */
    SecureBasicAuthenticator();

    /** destructor. */
    ~SecureBasicAuthenticator();

    /** Verify the authentication of the requesting user.
        @param userName String containing the user name
        @param password String containing the user password
        @param authInfo AuthenticationInfo holding ALL request specific
               authentication information
        @return AuthenticationStatus holding http status code and error detail
    */
    AuthenticationStatus authenticate(
        const String& userName,
        const String& password,
        AuthenticationInfo* authInfo);

    /**
        Verify whether the user is valid.
        @param userName String containing the user name
        @param authInfo reference to AuthenticationInfo object that holds the
        authentication information for the given connection.
        @return AuthenticationStatus holding http status code and error detail
    */
    AuthenticationStatus validateUser(
        const String& userName,
        AuthenticationInfo* authInfo);

    /** Construct and return the HTTP Basic authentication challenge header
        @return A string containing the authentication challenge header.
    */
    String getAuthResponseHeader();

private:

#ifdef PEGASUS_OS_ZOS
#if (__TARGET_LIB__ >= 0x410A0000)

    String        _zosAPPLID;

#elif defined(PEGASUS_PLATFORM_ZOS_ZSERIES_IBM)

    /** Set the applicatoin ID to CFZAPPL for validatition of passtickes.
        This function is only needed if the compile target system is z/OS R9
        or earlier and 32 Bit !
         @return true on success.
    */
    Boolean set_ZOS_ApplicationID( void );
#endif // end __TARGET_LIB__
#endif // end PEGASUS_OS_ZOS

    String        _realm;
#ifndef PEGASUS_PAM_AUTHENTICATION
    UserManager*  _userManager;
#endif
};

PEGASUS_NAMESPACE_END

#endif /* Pegasus_SecureBasicAuthenticator_h */
