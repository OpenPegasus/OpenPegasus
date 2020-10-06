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
//  User/Auth Provider
///////////////////////////////////////////////////////////////////////////////

#include <Pegasus/Common/Config.h>

#include <cctype>
#include <iostream>

#include <Pegasus/Common/String.h>
#include <Pegasus/Common/CIMNameCast.h>
#include <Pegasus/Common/System.h>
#include <Pegasus/Common/ArrayInternal.h>
#include <Pegasus/Common/CIMType.h>
#include <Pegasus/Common/CIMInstance.h>
#include <Pegasus/Common/CIMObjectPath.h>
#include <Pegasus/Common/InternalException.h>
#include <Pegasus/Common/CIMStatusCode.h>
#include <Pegasus/Common/Tracer.h>
#include <Pegasus/Common/PegasusVersion.h>

#include <Pegasus/Repository/CIMRepository.h>
#include <Pegasus/Provider/CIMInstanceProvider.h>
#include <Pegasus/Provider/CIMMethodProvider.h>
#include <Pegasus/Common/ResponseHandler.h>
#include <Pegasus/Security/UserManager/UserManager.h>
#include <Pegasus/Security/UserManager/UserExceptions.h>
#include <Pegasus/Common/MessageLoader.h> //l10n

#include "UserAuthProvider.h"

PEGASUS_USING_STD;

PEGASUS_NAMESPACE_BEGIN

/**
    The constants representing the Username and Password properties in the
    schema.
*/
static const CIMName PROPERTY_NAME_USERNAME = CIMNameCast("Username");

static const CIMName PROPERTY_NAME_PASSWORD = CIMNameCast("Password");

static const char OLD_PASSWORD[] = "OldPassword";

static const char NEW_PASSWORD[] = "NewPassword";

/**
    The constants representing the namespace and authorization
    in the schema.
*/
static const CIMName PROPERTY_NAME_NAMESPACE       =
    CIMNameCast("Namespace");

static const CIMName PROPERTY_NAME_AUTHORIZATION   =
    CIMNameCast("Authorization");

/**
    The constant representing the User class name.
*/
static const CIMName CLASS_NAME_PG_USER            =
    CIMNameCast("PG_User");

static const CIMName METHOD_NAME_MODIFY_PASSWORD   =
    CIMNameCast("modifyPassword");

/**
    The constant representing the authorization class name
*/
static const CIMName CLASS_NAME_PG_AUTHORIZATION =
    CIMNameCast("PG_Authorization");

//
// Verify user authorization
//
void UserAuthProvider::_verifyAuthorization(const String& user)
{
    PEG_METHOD_ENTER(TRC_CONFIG,
        "UserAuthProvider::_verifyAuthorization()");

    if ( user.size() && !System::isPrivilegedUser(user) )
    {
        MessageLoaderParms parms(
            "ControlProviders.UserAuthProvider."
            "MUST_BE_PRIVILEGED_USER",
            "Superuser authority is required to run this CIM operation.");
        PEG_METHOD_EXIT();
        throw PEGASUS_CIM_EXCEPTION_L(CIM_ERR_ACCESS_DENIED,parms);
    }

    PEG_METHOD_EXIT();
}

//
// Creates a new instance.
//
void UserAuthProvider::createInstance(
    const OperationContext & context,
    const CIMObjectPath & instanceReference,
    const CIMInstance & myInstance,
    ObjectPathResponseHandler & handler)
{
    PEG_METHOD_ENTER(TRC_USER_MANAGER,"UserAuthProvider::createInstance");

    CIMValue    userName;
    CIMValue    password;
    String      userNameStr;
    String      passwordStr;
    String      namespaceStr;
    String      authorizationStr;
    Boolean     authAlreadyExists = false;
    //
    // get userName
    //
    String user;
    try
    {
        const IdentityContainer container =
            context.get(IdentityContainer::NAME);
        user= container.getUserName();
    }
    catch (...)
    {
        user= String::EMPTY;
    }

    //
    // verify user authorizations
    //
    if (user != String::EMPTY)
    {
        _verifyAuthorization(user);
    }

    CIMInstance          modifiedInst = myInstance;

    // begin processing the request
    handler.processing();

#ifndef PEGASUS_NO_PASSWORDFILE
    //
    // check if the class name requested is PG_User
    //
    if (CLASS_NAME_PG_USER.equal(instanceReference.getClassName()))
    {
        //
        // Get the user name from the instance
        //
        Uint32 pos = myInstance.findProperty(PROPERTY_NAME_USERNAME);
        CIMProperty prop = (CIMProperty)modifiedInst.getProperty(pos);
        userName = prop.getValue();
        userName.get(userNameStr);

        //
        // Get the password from the instance
        //
        pos = myInstance.findProperty (PROPERTY_NAME_PASSWORD);
        prop = (CIMProperty) modifiedInst.getProperty(pos);
        password = prop.getValue();
        password.get(passwordStr);

        //
        // Add the user to the User Manager
        //
        _userManager->addUser(userNameStr, passwordStr);
    }
    //
    // check if the class name requested is PG_Authorization
    //
    else if (instanceReference.getClassName().equal
        (CLASS_NAME_PG_AUTHORIZATION))
#else
    if (instanceReference.getClassName().equal (CLASS_NAME_PG_AUTHORIZATION))
#endif
    {
        try
        {
            //
            // Get the user name from the instance
            //
            Uint32 pos = myInstance.findProperty ( PROPERTY_NAME_USERNAME );
            CIMProperty prop = (CIMProperty)modifiedInst.getProperty(pos);
            prop.getValue().get(userNameStr);

            //
            // Get the namespace from the instance
            //
            pos = myInstance.findProperty ( PROPERTY_NAME_NAMESPACE );
            prop = (CIMProperty)modifiedInst.getProperty(pos);
            prop.getValue().get(namespaceStr);

            //
            // Get the authorization from the instance
            //
            pos = myInstance.findProperty ( PROPERTY_NAME_AUTHORIZATION );
            prop = (CIMProperty)modifiedInst.getProperty(pos);
            prop.getValue().get(authorizationStr);

            //
            // Check if the user is a valid system user
            //
            if ( !System::isSystemUser( userNameStr.getCString() ) )
            {
                InvalidSystemUser isu(userNameStr);
                throw isu;
            }

#ifndef PEGASUS_NO_PASSWORDFILE
            //
            // check if the user is a valid CIM user
            //
            if ( !_userManager->verifyCIMUser( userNameStr ) )
            {
                InvalidUser iu(userNameStr);
                throw iu;
            }
#endif

            try
            {
                _userManager->getAuthorization(userNameStr, namespaceStr);
                authAlreadyExists = true;
            }
            catch(...)
            {
                //if authorization does not exist, It will be created 
                //If exist, exception is thrown later
            }
            if (!authAlreadyExists)
            {
            _repository->createInstance(
                instanceReference.getNameSpace(), myInstance);

            //
            // set authorization in the UserManager
            //
            _userManager->setAuthorization(
                userNameStr, namespaceStr, authorizationStr );
        }
        }
        catch ( InvalidUser &iu )
        {
            PEG_METHOD_EXIT();
            throw PEGASUS_CIM_EXCEPTION(
                CIM_ERR_INVALID_PARAMETER, iu.getMessage());
        }
        catch ( InvalidSystemUser &isu )
        {
            PEG_METHOD_EXIT();
            throw PEGASUS_CIM_EXCEPTION(
                CIM_ERR_INVALID_PARAMETER, isu.getMessage());
        }
        catch ( InvalidNamespace &ins )
        {
            PEG_METHOD_EXIT();
            throw PEGASUS_CIM_EXCEPTION(
                CIM_ERR_INVALID_PARAMETER, ins.getMessage());
        }
        catch ( CIMException &e )
        {
            PEG_METHOD_EXIT();
            throw PEGASUS_CIM_EXCEPTION(CIM_ERR_FAILED, e.getMessage());
        }
        catch ( Exception &e )
        {
            PEG_METHOD_EXIT();
            throw PEGASUS_CIM_EXCEPTION(CIM_ERR_FAILED, e.getMessage());
        }
    }
    else
    {
        PEG_METHOD_EXIT();
        throw PEGASUS_CIM_EXCEPTION (CIM_ERR_NOT_SUPPORTED,
                                 instanceReference.getClassName().getString());
    }
    if (authAlreadyExists)
    {
        throw PEGASUS_CIM_EXCEPTION(
            CIM_ERR_ALREADY_EXISTS,
            userNameStr);
    }

    handler.deliver(instanceReference);

    // complete processing the request
    handler.complete();

    PEG_METHOD_EXIT();
    return;
}

//
// Deletes the specified instance.
//
void UserAuthProvider::deleteInstance(
    const OperationContext & context,
    const CIMObjectPath& myInstance,
    ResponseHandler & handler)
{
    CIMValue                userName ;
    String                  userNameStr;
    String                  namespaceStr;
    Array<CIMKeyBinding>       kbArray;

    PEG_METHOD_ENTER(TRC_USER_MANAGER,"UserAuthProvider::deleteInstance");

    //
    // get userName
    //
    String user;
    try
    {
        IdentityContainer container = context.get(IdentityContainer::NAME);
        user= container.getUserName();
    }
    catch (...)
    {
        user= String::EMPTY;
    }

    //
    // verify user authorizations
    //
    if (user != String::EMPTY)
    {
        _verifyAuthorization(user);
    }

    // begin processing the request
    handler.processing();

#ifndef PEGASUS_NO_PASSWORDFILE
    //
    // check if the class name requested is PG_User
    //
    if (myInstance.getClassName().equal (CLASS_NAME_PG_USER))
    {
        //
        // Get the user name from the instance
        //
        kbArray = myInstance.getKeyBindings();
        if (!kbArray.size())
        {
            MessageLoaderParms parms(
                "ControlProviders.UserAuthProvider."
                    "UNABLE_TO_FIND_KEY_PROPERTY_USERNAME",
                "Unable to find Key Property Username");
            throw PEGASUS_CIM_EXCEPTION_L( CIM_ERR_INVALID_PARAMETER,parms);
        }
        if (kbArray[0].getName() == PROPERTY_NAME_USERNAME)
        {
            userNameStr = kbArray[0].getValue();
        }
        else
        {
            MessageLoaderParms parms(
                "ControlProviders.UserAuthProvider.UNEXPECTED_KEY_PROPERTY",
                "Unexpected Key property");
            throw PEGASUS_CIM_EXCEPTION_L(CIM_ERR_INVALID_PARAMETER, parms);
        }

        //
        // Remove the user from User Manager
        //
        _userManager->removeUser(userNameStr);
    }
    //
    // check if the class name requested is PG_Authorization
    //
    else if (myInstance.getClassName().equal (CLASS_NAME_PG_AUTHORIZATION))
#else
    if (myInstance.getClassName().equal (CLASS_NAME_PG_AUTHORIZATION))
#endif
    {
        //
        // Get the user name and namespace from the instance
        //
        kbArray = myInstance.getKeyBindings();
        for (Uint32 i = 0; i < kbArray.size(); i++)
        {
            if ( kbArray[i].getName() == PROPERTY_NAME_USERNAME )
            {
                userNameStr = kbArray[i].getValue();
            }
            else if ( kbArray[i].getName() == PROPERTY_NAME_NAMESPACE )
            {
                namespaceStr = kbArray[i].getValue();
            }
        }

        if ( !userNameStr.size() )
        {
            PEG_METHOD_EXIT();
            MessageLoaderParms parms("ControlProviders.UserAuthProvider."
                                        "USERNAME_PROPERTY_CANNOT_BE_EMPTY",
                                        "Username property can not be empty.");
            throw PEGASUS_CIM_EXCEPTION_L(CIM_ERR_INVALID_PARAMETER,parms);
        }
        else if ( !namespaceStr.size() )
        {
            PEG_METHOD_EXIT();
            MessageLoaderParms parms("ControlProviders.UserAuthProvider."
                                         "NAMESPACE_PROPERTY_CANNOT_BE_EMPTY",
                                     "Namespace property can not be empty.");
            throw PEGASUS_CIM_EXCEPTION_L(CIM_ERR_INVALID_PARAMETER,parms);
        }

        //
        // ATTN: Note that the following is a hack, because
        // deleteInstance() in repository does not like
        // the hostname and namespace included in the CIMObjectPath
        // passed to it as a parameter.
        //
        CIMObjectPath ref("", CIMNamespaceName (),
            myInstance.getClassName(), myInstance.getKeyBindings());

        _repository->deleteInstance(myInstance.getNameSpace(), ref);

        //
        // remove authorization in the UserManager
        //
        _userManager->removeAuthorization(userNameStr, namespaceStr);
    }
    else
    {
        PEG_METHOD_EXIT();
        throw PEGASUS_CIM_EXCEPTION(
            CIM_ERR_NOT_FOUND, myInstance.getClassName().getString());
    }


    // complete processing the request
    handler.complete();

    PEG_METHOD_EXIT();
    return;
}

//
// Modify instance based on modifiedInstance.
//
void UserAuthProvider::modifyInstance(
    const OperationContext & context,
    const CIMObjectPath & instanceReference,
    const CIMInstance& modifiedIns,
    const Boolean includeQualifiers,
    const CIMPropertyList & propertyList,
    ResponseHandler & handler)
{
    PEG_METHOD_ENTER(TRC_USER_MANAGER,"UserAuthProvider::modifyInstance");

    //
    // get userName
    //
    String user;
    try
    {
        IdentityContainer container = context.get(IdentityContainer::NAME);
        user= container.getUserName();
    }
    catch (...)
    {
        user= String::EMPTY;
    }

    //
    // verify user authorizations
    //
    if (user != String::EMPTY)
    {
        _verifyAuthorization(user);
    }

    //
    // check if the class name requested is PG_Authorization
    //
    if (!instanceReference.getClassName().equal (CLASS_NAME_PG_AUTHORIZATION))
    {
        PEG_METHOD_EXIT();
        throw PEGASUS_CIM_EXCEPTION (
            CIM_ERR_NOT_SUPPORTED,
            instanceReference.getClassName().getString());
    }

    CIMInstance newInstance = modifiedIns;

    // begin processing the request
    handler.processing();

    //
    // Get the user name from the instance
    //
    String userNameStr;
    String namespaceStr;
    String authorizationStr;

    Uint32 pos = modifiedIns.findProperty ( PROPERTY_NAME_USERNAME );
    CIMProperty prop = (CIMProperty)newInstance.getProperty(pos);
    prop.getValue().get(userNameStr);

    //
    // Get the namespace from the instance
    //
    pos = modifiedIns.findProperty ( PROPERTY_NAME_NAMESPACE );
    prop = (CIMProperty)newInstance.getProperty(pos);
    prop.getValue().get(namespaceStr);

    //
    // Get the authorization from the instance
    //
    pos = modifiedIns.findProperty ( PROPERTY_NAME_AUTHORIZATION );
    prop = (CIMProperty)newInstance.getProperty(pos);
    prop.getValue().get(authorizationStr);

    //
    // ATTN: Note that the following is a hack, because
    // modifyInstance() in repository does not like
    // the hostname and namespace included in the CIMObjectPath
    // passed to it as a parameter.
    //
    CIMObjectPath ref("", CIMNamespaceName (),
        modifiedIns.getClassName(), instanceReference.getKeyBindings());

    CIMInstance newModifiedIns = modifiedIns.clone ();
    newModifiedIns.setPath (ref);

    //
    // call modifyInstances of the repository
    //
    _repository->modifyInstance(
        instanceReference.getNameSpace(), newModifiedIns);

    //
    // set authorization in the UserManager
    //
    _userManager->setAuthorization(
        userNameStr, namespaceStr, authorizationStr);

    // complete processing the request
    handler.complete();

    PEG_METHOD_EXIT();
}

//
// Enumerates instances.
//
void UserAuthProvider::enumerateInstances(
    const OperationContext & context,
    const CIMObjectPath & ref,
    const Boolean includeQualifiers,
    const Boolean includeClassOrigin,
    const CIMPropertyList& propertyList,
    InstanceResponseHandler & handler)
{
    PEG_METHOD_ENTER(TRC_USER_MANAGER,"UserAuthProvider::enumerateInstances");

    Array<CIMInstance> instanceArray;
    Array<CIMInstance> namedInstances;

    //
    // get userName
    //
    String user;
    try
    {
        IdentityContainer container = context.get(IdentityContainer::NAME);
        user= container.getUserName();
    }
    catch (...)
    {
        user= String::EMPTY;
    }

    //
    // verify user authorizations
    //
    if (user != String::EMPTY)
    {
        _verifyAuthorization(user);
    }

    //
    // check if the class name requested is PG_Authorization
    //
    if (!ref.getClassName().equal (CLASS_NAME_PG_AUTHORIZATION))
    {
        PEG_METHOD_EXIT();
        throw PEGASUS_CIM_EXCEPTION (CIM_ERR_NOT_SUPPORTED,
            ref.getClassName().getString());
    }

    // begin processing the request
    handler.processing();

    //
    // call enumerateInstancesForClass of the repository
    //
    namedInstances = _repository->enumerateInstancesForClass(
        ref.getNameSpace(), ref.getClassName());

    for(Uint32 i = 0, n = namedInstances.size(); i < n; i++)
    {
        handler.deliver(namedInstances[i]);
    }

    // complete processing the request
    handler.complete();

    PEG_METHOD_EXIT();
    return;
}

//
// Enumerates all the user names.
//
void UserAuthProvider::enumerateInstanceNames(
    const OperationContext & context,
    const CIMObjectPath & classReference,
    ObjectPathResponseHandler & handler)
{
    PEG_METHOD_ENTER(TRC_USER_MANAGER,
                     "UserAuthProvider::enumerateInstanceNames");

    Array<CIMObjectPath> instanceRefs;
    Array<String>       userNames;
    Array<CIMKeyBinding>   keyBindings;
    CIMKeyBinding          kb;
    String            hostName;

    //
    // get userName
    //
    String user;
    try
    {
        IdentityContainer container = context.get(IdentityContainer::NAME);
        user= container.getUserName();
    }
    catch (...)
    {
        user= String::EMPTY;
    }

    //
    // verify user authorizations
    //
    if (user != String::EMPTY)
    {
        _verifyAuthorization(user);
    }

    const CIMName& className = classReference.getClassName();
    const CIMNamespaceName& nameSpace = classReference.getNameSpace();

    // begin processing the request
    handler.processing();

#ifndef PEGASUS_NO_PASSWORDFILE
    //
    // check if the class name requested is PG_User
    //
    if (className.equal(CLASS_NAME_PG_USER))
    {
        hostName.assign(System::getHostName());

        _userManager->getAllUserNames(userNames);

        Uint32 size = userNames.size();

        for (Uint32 i = 0; i < size; i++)
        {
            keyBindings.append(CIMKeyBinding(PROPERTY_NAME_USERNAME,
                userNames[i],
                CIMKeyBinding::STRING));

            //
            // Convert instance names to References
            //
            CIMObjectPath ref(hostName, nameSpace, className, keyBindings);

            handler.deliver(ref);

            keyBindings.clear();
        }
    }
    //
    // check if the class name requested is PG_Authorization
    //
    else if (className.equal (CLASS_NAME_PG_AUTHORIZATION))
#else
    if (className.equal (CLASS_NAME_PG_AUTHORIZATION))
#endif
    {
        //
        // call enumerateInstanceNamesForClass of the repository
        //
        instanceRefs = _repository->enumerateInstanceNamesForClass(
            nameSpace, className);

        handler.deliver(instanceRefs);
    }
    else
    {
        handler.complete();

        PEG_METHOD_EXIT();
        throw PEGASUS_CIM_EXCEPTION (CIM_ERR_NOT_SUPPORTED,
            className.getString());
    }

    // complete processing the request
    handler.complete();

    PEG_METHOD_EXIT();
    return;
}

//
// Invoke Method, used to modify user's password
//
void UserAuthProvider::invokeMethod(
    const OperationContext & context,
    const CIMObjectPath & ref,
    const CIMName & methodName,
    const Array<CIMParamValue> & inParams,
    MethodResultResponseHandler & handler)
{
    PEG_METHOD_ENTER(TRC_USER_MANAGER,"UserAuthProvider::invokeMethod");

    //
    // get userName
    //
    String user;
    try
    {
        IdentityContainer container = context.get(IdentityContainer::NAME);
        user= container.getUserName();
    }
    catch (...)
    {
        user= String::EMPTY;
    }
    //
    // verify user authorizations
    //
    if (user != String::EMPTY)
    {
        _verifyAuthorization(user);
    }

#ifndef PEGASUS_NO_PASSWORDFILE
    String            userName;
    String            password;
    String            newPassword;
    Array<CIMKeyBinding>     kbArray;

    // Begin processing the request
    handler.processing();

    // Check if the class name is PG_USER
    if (!ref.getClassName().equal (CLASS_NAME_PG_USER))
    {
        handler.complete();
            throw PEGASUS_CIM_EXCEPTION (CIM_ERR_NOT_SUPPORTED,
                ref.getClassName().getString());
    }

    // Check if the method name is correct
    if (!methodName.equal (METHOD_NAME_MODIFY_PASSWORD))
    {
        handler.complete();
        PEG_METHOD_EXIT();
        MessageLoaderParms parms("ControlProviders.UserAuthProvider."
                                     "UNSUPPORTED_METHOD_NAME",
                                 "Unsupported method name, $0",
                                 methodName.getString());
        throw PEGASUS_CIM_EXCEPTION_L(CIM_ERR_FAILED,parms);
    }

    // Check if all the input parameters are passed.
    if (inParams.size() < 2)
    {
        handler.complete();
        PEG_METHOD_EXIT();
        MessageLoaderParms parms(
            "ControlProviders.UserAuthProvider.INPUT_PARAMETERS_NOT_VALID",
            "Input parameters are not valid.");
        throw PEGASUS_CIM_EXCEPTION_L(CIM_ERR_INVALID_PARAMETER, parms);
    }

    kbArray = ref.getKeyBindings();

    if (!kbArray.size())
    {
        PEG_METHOD_EXIT();
        MessageLoaderParms parms(
            "ControlProviders.UserAuthProvider."
                "UNABLE_TO_FIND_KEY_PROPERTY_USERNAME",
            "Unable to find Key Property Username");
        throw PEGASUS_CIM_EXCEPTION_L(CIM_ERR_INVALID_PARAMETER, parms);
    }

    //
    // Get the user name
    //
    if (kbArray[0].getName() == PROPERTY_NAME_USERNAME)
    {
        userName = kbArray[0].getValue();
    }
    else
    {
        PEG_METHOD_EXIT();
        MessageLoaderParms parms(
            "ControlProviders.UserAuthProvider.UNEXPECTED_KEY_PROPERTY",
            "Unexpected key property");
        throw PEGASUS_CIM_EXCEPTION_L(CIM_ERR_INVALID_PARAMETER, parms);
    }

    //
    // Get the old and the new password params
    //
    for ( Uint32 i=0; i < 2; i++)
    {
        //
        // check the param name
        //
        if (inParams[i].getParameterName() == OLD_PASSWORD)
        {
            inParams[i].getValue().get(password);
        }
        if (inParams[i].getParameterName() == NEW_PASSWORD)
        {
            inParams[i].getValue().get(newPassword);
        }
    }

    // Modify the user's password in User Manager
    _userManager->modifyUser(
        userName,
        password,
        newPassword);

    // Return zero as there is no error
    Uint32 retCode = 0;
    handler.deliver(CIMValue(retCode));

    // complete processing the request
    handler.complete();

    PEG_METHOD_EXIT();
    return;
#else
    PEG_METHOD_EXIT();
    throw PEGASUS_CIM_EXCEPTION (CIM_ERR_NOT_SUPPORTED,
        ref.getClassName().getString());
#endif
}

PEGASUS_NAMESPACE_END
