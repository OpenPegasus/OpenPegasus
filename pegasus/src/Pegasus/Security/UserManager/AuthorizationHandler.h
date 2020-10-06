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
//%////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
//
// This file implements the functionality required to manage password file.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef Pegasus_AuthorizationHandler_h
#define Pegasus_AuthorizationHandler_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/String.h>
#include <Pegasus/Repository/CIMRepository.h>

#include <Pegasus/Security/UserManager/Linkage.h>

PEGASUS_NAMESPACE_BEGIN

///////////////////////////////////////////////////////////////////////////////
// Auth Table
//////////////////////////////////////////////////////////////////////////////

typedef HashTable<String, String, EqualFunc <String>, HashFunc <String> >
    AuthTable;

/**
    This class implements the functionality required to manage user
    authorizations.  It provides methods to get, set, remove and verify the
    user authorizations at namespace level.
*/
class PEGASUS_USERMANAGER_LINKAGE AuthorizationHandler
{
private:

    //
    // Authorization cache
    //
    AuthTable _authTable;

    //
    // Repository handle
    //
    CIMRepository* _repository;

protected:

    /** Load the user information from the Repository.
    */
    void _loadAllAuthorizations();

public:

    /** Constructor. */
    AuthorizationHandler(CIMRepository* repository);

    /** Destructor. */
    ~AuthorizationHandler();

    /** Verify whether the spcefied namespace is a valid namespace.
        @param nameSpace  string containing the namespace name.
        @return true if the specified name space is valid and exists, false
        otherwise.
    */
    Boolean verifyNamespace(const CIMNamespaceName& nameSpace);

    /**
        Verify whether the specified operation has authorization to be
        performed by the specified user.
        @param userName   string containing the user name.
        @param nameSpace  string containing the namespace name.
        @param cimMethodName string containing the cim method name.
        @return true if the specified user has authorizations to run the
        specified CIM operation on the specified namespace, false otherwise.
    */
    Boolean verifyAuthorization(
        const String& userName,
        const CIMNamespaceName& nameSpace,
        const CIMName& cimMethodName);

    /**
        Set the authorization to the specified user on the specified namespace.
        @param userName   string containing the user name.
        @param nameSpace  string containing the namespace name.
        @param auth string containing the authorizations.
    */
    void setAuthorization(
        const String& userName,
        const CIMNamespaceName& nameSpace,
        const String& auth);

    /**
        Remove the authorizations of the specified user on the specified
        namespace.
        @param userName   string containing the user name.
        @param nameSpace  string containing the namespace name.
    */
    void removeAuthorization(
        const String& userName,
        const CIMNamespaceName& nameSpace);

    /**
        Get the authorizations of the specified user on the specified namespace.
        @param userName   string containing the user name.
        @param nameSpace  string containing the namespace name.
        @return a string containing the authorizations.
    */
    String getAuthorization(
        const String& userName,
        const CIMNamespaceName& nameSpace);
};

PEGASUS_NAMESPACE_END

#endif /* Pegasus_AuthorizationHandler_h */

