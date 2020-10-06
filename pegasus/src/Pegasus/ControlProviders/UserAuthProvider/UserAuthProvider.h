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

#ifndef UserAuthProvider_h
#define UserAuthProvider_h

///////////////////////////////////////////////////////////////////////////////
//  User/Auth Provider
///////////////////////////////////////////////////////////////////////////////

#include <Pegasus/Common/Config.h>

#include <cctype>
#include <iostream>

#include <Pegasus/ControlProviders/UserAuthProvider/Linkage.h>
#include <Pegasus/Common/String.h>
#include <Pegasus/Common/System.h>
#include <Pegasus/Common/ArrayInternal.h>
#include <Pegasus/Common/CIMType.h>
#include <Pegasus/Common/CIMInstance.h>
#include <Pegasus/Common/CIMObjectPath.h>
#include <Pegasus/Common/InternalException.h>
#include <Pegasus/Common/CIMStatusCode.h>
#include <Pegasus/Common/Tracer.h>
#include <Pegasus/Common/OperationContext.h>

#include <Pegasus/Repository/CIMRepository.h>
#include <Pegasus/Provider/CIMInstanceProvider.h>
#include <Pegasus/Provider/CIMMethodProvider.h>
#include <Pegasus/Common/ResponseHandler.h>
#include <Pegasus/Security/UserManager/UserManager.h>
#include <Pegasus/Security/UserManager/UserExceptions.h>

PEGASUS_USING_STD;

PEGASUS_NAMESPACE_BEGIN

/**
    The UserAuthProvider provides responses to the CIM Operations defined in
    the DMTF docuument CIM Operations over HTTP (Section 2.5).

    This provider implements the following functions:
    - createInstance            ( Adds a user or user authorization )
    - invokeMethod              ( Modifies a user's password )
    - enumerateInstanceNames    ( Lists all users )
    - enumerateInstances        ( Lists all user authorizations )
    - modifyInstance            ( Modifies a user authorization )
    - deleteInstance            ( Removes a user or user authorization )
*/

class PEGASUS_USERAUTHPROVIDER_LINKAGE UserAuthProvider
    : public CIMInstanceProvider, public CIMMethodProvider
{
public:

    UserAuthProvider(CIMRepository* repository)
    {
    PEG_METHOD_ENTER(TRC_USER_MANAGER,"UserAuthProvider::UserAuthProvider");

        //
        // get an instance of User Manager and initialize
        // the instance variable.
        //
        _userManager = UserManager::getInstance();

        _repository = repository;

    PEG_METHOD_EXIT();
    }

    virtual ~UserAuthProvider()
    {
    PEG_METHOD_ENTER(TRC_USER_MANAGER,"UserAuthProvider::~UserAuthProvider");

    PEG_METHOD_EXIT();
    }

    // Note:  The initialize() and terminate() methods are not called for
    // Control Providers.
    void initialize(CIMOMHandle& cimomHandle) { }
    void terminate() { }

    /**
    Returns the instance based on instanceName.
    */
    virtual void getInstance(
    const OperationContext & context,
        const CIMObjectPath& instanceName,
        const Boolean includeQualifiers,
        const Boolean includeClassOrigin,
        const CIMPropertyList& propertyList,
    InstanceResponseHandler & handler)
    {
        throw PEGASUS_CIM_EXCEPTION (CIM_ERR_NOT_SUPPORTED, "");
    }

    /**
    Creates a new instance.

    @param  context
    @param  instanceReference
    @param  myInstance
    @param  handler
    */
    virtual void createInstance(
        const OperationContext& context,
        const CIMObjectPath& instanceReference,
        const CIMInstance& myInstance,
        ObjectPathResponseHandler & handler);

    /**
    Deletes the specified instance.

    @param  context
    @param  myInstance
    @param  handler
    */
    virtual void deleteInstance(
        const OperationContext& context,
        const CIMObjectPath& myInstance,
        ResponseHandler& handler);


    /**
    Modify instance based on modifiedInstance.

    @param context specifies security and locale information relevant for
                   the lifetime of this operation.
    @param instanceReference the fully qualified object path of the instance.
    @param includeQualifiers Indicates whether the qualifiers should be
        modified as specified in modifiedIns.
    @param propertyList If not null, only the properties in this list should
        be modified in the instance.
    @param modifiedIns the modified instance.
    @param handler enables providers to asynchronously return the results.
    */
    virtual void modifyInstance(
        const OperationContext & context,
        const CIMObjectPath & instanceReference,
        const CIMInstance& modifiedIns,
        const Boolean includeQualifiers,
        const CIMPropertyList & propertyList,
        ResponseHandler & handler);

    /**
    Enumerates instances.

    @param context specifies security and locale information relevant for
                   the lifetime of this operation.
    @param ref the fully qualified object path of the instance.
    @param includeQualifiers specifies whether qualifiers should be included
        inthe instances.
    @param includeClassOrigin specifies whether class origin information
        should be included inthe instances.
    @param propertyList list containing the properties.
    @param handler enables providers to asynchronously return the results.
    */
    virtual void enumerateInstances(
        const OperationContext & context,
        const CIMObjectPath & ref,
        const Boolean includeQualifiers,
        const Boolean includeClassOrigin,
        const CIMPropertyList& propertyList,
        InstanceResponseHandler & handler);

    /**
    Enumerates all the user names.
    */
    virtual void enumerateInstanceNames(
    const OperationContext & context,
    const CIMObjectPath & classReference,
        ObjectPathResponseHandler & handler);

    /**
       Invoke Method, used to modify user's password
    */
    virtual void invokeMethod(
        const OperationContext & context,
        const CIMObjectPath & ref,
        const CIMName & methodName,
        const Array<CIMParamValue> & inParams,
        MethodResultResponseHandler & handler);

private:
    /**
        Verify User Authorization.

        @param  user                    userName to be verified

        @throw  CIM_ERR_ACCESS_DENIED   if the specified user is not a
                                        priviliged user.
    */
    void _verifyAuthorization(const String& user);

    //
    // User Manager Instance variable
    //
    UserManager*   _userManager;

    //
    // The repository pointer for use by the provider.
    //
    CIMRepository* _repository;


};

PEGASUS_NAMESPACE_END

#endif // UserAuthProvider_h
