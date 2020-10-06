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
// This file implements the functionality required to manage auth table.
//
///////////////////////////////////////////////////////////////////////////////

#include <Pegasus/Common/FileSystem.h>
#include <Pegasus/Common/HashTable.h>
#include <Pegasus/Common/Logger.h>
#include <Pegasus/Common/System.h>
#include <Pegasus/Common/Tracer.h>
#include <Pegasus/Common/CIMInstance.h>
#include <Pegasus/Common/Constants.h>
#include <Pegasus/Common/XmlWriter.h>
#include <Pegasus/Common/CIMNameCast.h>

#ifdef PEGASUS_OS_PASE
# include <ILEWrapper/qumemultiutil.h>
# include <ILEWrapper/ILEUtilities2.h>
#endif

#include "AuthorizationHandler.h"
#include "UserExceptions.h"

PEGASUS_USING_STD;

PEGASUS_NAMESPACE_BEGIN


//
// This constant represents the  User name property in the schema
//
static const CIMName PROPERTY_NAME_USERNAME = CIMNameCast("Username");

//
// This constant represents the Namespace property in the schema
//
static const CIMName PROPERTY_NAME_NAMESPACE = CIMNameCast("Namespace");

//
// This constant represents the Authorizations property in the schema
//
static const CIMName PROPERTY_NAME_AUTHORIZATION = CIMNameCast("Authorization");


//
// List of all the CIM Operations
//
// Note: The following tables contain all the existing CIM Operations.
//       Any new CIM Operations created must be included in one of these tables,
//       otherwise no CIM requests will have authorization to execute those
//       new operations.
//

//
// List of read only CIM Operations
//
static const CIMName READ_OPERATIONS [] =
{
    CIMName("GetClass"),
    CIMName("GetInstance"),
    CIMName("EnumerateClassNames"),
    CIMName("References"),
    CIMName("ReferenceNames"),
    CIMName("AssociatorNames"),
    CIMName("Associators"),
    CIMName("EnumerateInstanceNames"),
    CIMName("GetQualifier"),
    CIMName("EnumerateQualifiers"),
    CIMName("EnumerateClasses"),
    CIMName("EnumerateInstances"),
    CIMName("ExecQuery"),
    CIMName("GetProperty")
};

//
// List of write CIM Operations
//
static const CIMName WRITE_OPERATIONS [] =
{
    CIMName("CreateClass"),
    CIMName("CreateInstance"),
    CIMName("DeleteQualifier"),
    CIMName("SetQualifier"),
    CIMName("ModifyClass"),
    CIMName("ModifyInstance"),
    CIMName("DeleteClass"),
    CIMName("DeleteInstance"),
    CIMName("SetProperty"),
    CIMName("InvokeMethod"),
    CIMName("EnableIndicationSubscription"),
    CIMName("ModifyIndicationSubscription"),
    CIMName("DisableIndicationSubscription")
};

static String _getAuthKey(const String &userName, const String &nameSpace)
{
    //Separator ":" is used to distinguish "a" +"bc"
    // and "ab"+"c" scenarios
    String key = nameSpace;
    key.append(Char16(':'));
#ifdef PEGASUS_OS_TYPE_WINDOWS
    key.append(userName);
    key.toLower();
#else
    key.toLower();
    key.append(userName);
#endif

    return key;
}

//
// Constructor
//
AuthorizationHandler::AuthorizationHandler(CIMRepository* repository)
{
    PEG_METHOD_ENTER(
        TRC_AUTHORIZATION, "AuthorizationHandler::AuthorizationHandler()");

    _repository = repository;

    _loadAllAuthorizations();

    PEG_METHOD_EXIT();
}

//
// Destructor.
//
AuthorizationHandler::~AuthorizationHandler()
{
    PEG_METHOD_ENTER(
        TRC_AUTHORIZATION, "AuthorizationHandler::~AuthorizationHandler()");

    PEG_METHOD_EXIT();
}

//
// Check if a given namespace exists
//
Boolean AuthorizationHandler::verifyNamespace(
    const CIMNamespaceName& nameSpace)
{
    PEG_METHOD_ENTER(
        TRC_AUTHORIZATION, "AuthorizationHandler::verifyNamespace()");

    try
    {
        //
        // call enumerateNameSpaces to get all the namespaces
        // in the repository
        //
        Array<CIMNamespaceName> namespaceNames =
            _repository->enumerateNameSpaces();

        //
        // check for the given namespace
        //
        Uint32 size = namespaceNames.size();

        for (Uint32 i = 0; i < size; i++)
        {
             if (nameSpace.equal (namespaceNames[i]))
             {
                 PEG_METHOD_EXIT();
                 return true;
             }
        }
    }
    catch (Exception& e)
    {
        PEG_METHOD_EXIT();
        throw InvalidNamespace(nameSpace.getString() + e.getMessage());
    }

    PEG_METHOD_EXIT();

    return false;
}

//
// Load all user names and password
//
void AuthorizationHandler::_loadAllAuthorizations()
{
    PEG_METHOD_ENTER(
        TRC_AUTHORIZATION, "AuthorizationHandler::_loadAllAuthorizations()");

    Array<CIMInstance> namedInstances;

    try
    {
        //
        // call enumerateInstancesForClass of the repository
        //
        namedInstances = _repository->enumerateInstancesForClass(
            PEGASUS_NAMESPACENAME_AUTHORIZATION,
            PEGASUS_CLASSNAME_AUTHORIZATION);

        //
        // get all the user names, namespaces, and authorizations
        //
        for (Uint32 i = 0; i < namedInstances.size(); i++)
        {
            CIMInstance& authInstance = namedInstances[i];

            //
            // get user name
            //
            Uint32 pos = authInstance.findProperty(PROPERTY_NAME_USERNAME);
            CIMProperty prop = (CIMProperty)authInstance.getProperty(pos);
            String userName = prop.getValue().toString();

            //
            // get namespace name
            //
            pos = authInstance.findProperty(PROPERTY_NAME_NAMESPACE);
            prop = (CIMProperty)authInstance.getProperty(pos);
            String nameSpace = prop.getValue().toString();

            //
            // get authorizations
            //
            pos = authInstance.findProperty(PROPERTY_NAME_AUTHORIZATION);
            prop = (CIMProperty)authInstance.getProperty(pos);
            String auth = prop.getValue().toString();

            //
            // Add authorization to the table
            //
            if (!_authTable.insert(_getAuthKey(userName, nameSpace), auth))
            {
                throw AuthorizationCacheError();
            }
        }

    }
    catch (const CIMException& e)
    {
        // Allow initialization to succeed with an empty repository
        if (e.getCode() != CIM_ERR_INVALID_NAMESPACE)
        {
            PEG_METHOD_EXIT();
            throw;
        }
    }

    PEG_METHOD_EXIT();
}

void AuthorizationHandler::setAuthorization(
    const String& userName,
    const CIMNamespaceName& nameSpace,
    const String& auth)
{
    PEG_METHOD_ENTER(
        TRC_AUTHORIZATION, "AuthorizationHandler::setAuthorization()");

    String key = _getAuthKey(userName, nameSpace.getString());
    //
    // Remove auth if it already exists
    //
    _authTable.remove(key);

    //
    // Insert the specified authorization
    //
    if (!_authTable.insert(key, auth))
    {
        PEG_METHOD_EXIT();
        throw AuthorizationCacheError();
    }

    PEG_METHOD_EXIT();
}

void AuthorizationHandler::removeAuthorization(
    const String& userName,
    const CIMNamespaceName& nameSpace)
{
    PEG_METHOD_ENTER(
        TRC_AUTHORIZATION, "AuthorizationHandler::removeAuthorization()");

    //
    // Remove the specified authorization
    //
    if (!_authTable.remove(_getAuthKey(userName, nameSpace.getString())))
    {
        PEG_METHOD_EXIT();
        throw AuthorizationEntryNotFound(userName, nameSpace.getString());
    }
    PEG_METHOD_EXIT();
}

String AuthorizationHandler::getAuthorization(
    const String& userName,
    const CIMNamespaceName& nameSpace)
{
    PEG_METHOD_ENTER(
        TRC_AUTHORIZATION, "AuthorizationHandler::getAuthorization()");

    String auth;

    //
    // Get authorization for the specified userName and nameSpace
    //
    if (!_authTable.lookup(_getAuthKey(userName, nameSpace.getString()), auth))
    {
        PEG_METHOD_EXIT();
        throw AuthorizationEntryNotFound(userName, nameSpace.getString());
    }

    PEG_METHOD_EXIT();

    return auth;
}

//
// Verify whether the specified operation has authorization
// to be performed by the specified user.
//
Boolean AuthorizationHandler::verifyAuthorization(
    const String& userName,
    const CIMNamespaceName& nameSpace,
    const CIMName& cimMethodName)
{
    PEG_METHOD_ENTER(
        TRC_AUTHORIZATION, "AuthorizationHandler::verifyAuthorization()");

    Boolean authorized = false;
    Boolean readOperation = false;
    Boolean writeOperation = false;

    Uint32 readOpSize = sizeof(READ_OPERATIONS) / sizeof(READ_OPERATIONS[0]);

    Uint32 writeOpSize = sizeof(WRITE_OPERATIONS) / sizeof(WRITE_OPERATIONS[0]);

    for (Uint32 i = 0; i < readOpSize; i++)
    {
        if (cimMethodName.equal(READ_OPERATIONS[i]))
        {
            readOperation = true;
            break;
        }
    }
    if (!readOperation)
    {
        for (Uint32 i = 0; i < writeOpSize; i++ )
        {
            if (cimMethodName.equal(WRITE_OPERATIONS[i]))
            {
                writeOperation = true;
                break;
            }
        }
    }

#ifdef PEGASUS_OS_PASE
    if (readOperation || writeOperation)
    {
        //Use OS/400 Application Administration to do cim operation verification
        CString userCStr = userName.getCString();
        const char * user = (const char *)userCStr;
        CString cimMethCStr = cimMethodName.getString().getCString();
        const char * cimMeth = (const char *)cimMethCStr;

        CString nameSpaceCStr = nameSpace.getString().getCString();
        const char * nameSpChar = (const char *)nameSpaceCStr;

        int PaseAuth =
            umeVerifyFunctionAuthorization(user,
                    cimMeth);

        if (PaseAuth == TRUE)
            authorized = true;

        /* read operation needn't verify priviledUser */
        if(authorized && writeOperation)
        {
            /*
               The Application Admin checks
               we have now cover all class/qualifier
               operations to all namespaces.
               But maybe this is not enough protection
               for the private Pegasus namespaces.
               We should call isPrivilegedUser
               in this case instead of App Admin
               */
            if (strcasecmp(nameSpChar,"root/PG_Internal") == 0
                    ||strcasecmp(nameSpChar,"root/PG_InterOp") == 0
                    ||strcasecmp(nameSpChar,"PG_Internal") == 0
                    ||strcasecmp(nameSpChar,"PG_InterOp") == 0 
                    ||strcasecmp(nameSpChar,"root/interop") == 0
                    ||strcasecmp(nameSpChar,"interop") == 0 )
            {
                if(!System::isPrivilegedUser(userName))
                    authorized = false;
            }
        }
    }
#else
    //
    // Get the authorization of the specified user and namespace
    //
    String auth;
    try
    {
        auth = getAuthorization(userName, nameSpace);
    }
    catch (Exception&)
    {
        PEG_METHOD_EXIT();
        return authorized;
    }

    if ((String::equal(auth, "rw") || String::equal(auth, "wr")) &&
        (readOperation || writeOperation))
    {
        authorized = true;
    }
    else if (String::equal(auth, "r") && readOperation)
    {
        authorized = true;
    }
    else if (String::equal(auth, "w") && writeOperation)
    {
        authorized = true;
    }
#endif

    PEG_METHOD_EXIT();

    return authorized;
}

PEGASUS_NAMESPACE_END


