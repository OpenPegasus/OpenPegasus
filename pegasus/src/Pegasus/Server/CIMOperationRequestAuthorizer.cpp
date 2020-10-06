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

#include <Pegasus/Common/Constants.h>
#ifndef PEGASUS_PAM_AUTHENTICATION
# include <Pegasus/Security/UserManager/UserManager.h>
#endif
#include <Pegasus/Common/HTTPMessage.h>
#include <Pegasus/Common/XmlWriter.h>
#include <Pegasus/Common/Tracer.h>
#include <Pegasus/Config/ConfigManager.h>
#include <Pegasus/Common/MessageLoader.h>
#include "CIMOperationRequestAuthorizer.h"

#ifdef PEGASUS_ZOS_SECURITY
// This include file will not be provided in the OpenGroup CVS for now.
// Do NOT try to include it in your compile
# include <Pegasus/Common/safCheckzOS_inline.h>
#endif

PEGASUS_NAMESPACE_BEGIN

PEGASUS_USING_STD;

//
// Set group name separator
//
const char CIMOperationRequestAuthorizer::_GROUPNAME_SEPARATOR = ',';

CIMOperationRequestAuthorizer::CIMOperationRequestAuthorizer(
    MessageQueueService* outputQueue)
    : Base(PEGASUS_QUEUENAME_OPREQAUTHORIZER),
      _outputQueue(outputQueue),
      _serverTerminating(false)
{
    PEG_METHOD_ENTER(TRC_SERVER, "CIMOperationRequestAuthorizer::"
        "CIMOperationRequestAuthorizer");

#ifdef PEGASUS_ENABLE_USERGROUP_AUTHORIZATION
    _authorizedUserGroups = _getAuthorizedUserGroups();
#endif

    PEG_METHOD_EXIT();
}

CIMOperationRequestAuthorizer::~CIMOperationRequestAuthorizer()
{
    PEG_METHOD_ENTER(TRC_SERVER, "CIMOperationRequestAuthorizer::"
        "~CIMOperationRequestAuthorizer");

    PEG_METHOD_EXIT();
}

void CIMOperationRequestAuthorizer::sendResponse(
    Uint32 queueId,
    Buffer& message)
{
    PEG_METHOD_ENTER(TRC_SERVER, "CIMOperationRequestAuthorizer::sendResponse");

    MessageQueue* queue = MessageQueue::lookup(queueId);

    if (queue)
    {
        queue->enqueue(new HTTPMessage(message));
    }

    PEG_METHOD_EXIT();
}

// Code is duplicated in CIMOperationRequestDecoder
void CIMOperationRequestAuthorizer::sendIMethodError(
    Uint32 queueId,
    HttpMethod httpMethod,
    const String& messageId,
    const CIMName& iMethodName,
    const CIMException& cimException)
{
     PEG_METHOD_ENTER(TRC_SERVER,
         "CIMOperationRequestAuthorizer::sendIMethodError");

     Buffer message;
     message = XmlWriter::formatSimpleIMethodErrorRspMessage(
         iMethodName,
         messageId,
         httpMethod,
         cimException);

     sendResponse(queueId, message);

     PEG_METHOD_EXIT();
}

// Code is duplicated in CIMOperationRequestDecoder
void CIMOperationRequestAuthorizer::sendMethodError(
    Uint32 queueId,
    HttpMethod httpMethod,
    const String& messageId,
    const CIMName& methodName,
    const CIMException& cimException)
{
     PEG_METHOD_ENTER(TRC_SERVER,
         "CIMOperationRequestAuthorizer::sendMethodError");

     Buffer message;
     message = XmlWriter::formatSimpleMethodErrorRspMessage(
         methodName,
         messageId,
         httpMethod,
         cimException);

     sendResponse(queueId, message);

     PEG_METHOD_EXIT();
}

////////////////////////////////////////////////////////////////////////////////

void CIMOperationRequestAuthorizer::handleEnqueue(Message* request)
{

    PEG_METHOD_ENTER(TRC_SERVER,
        "CIMOperationRequestAuthorizer::handleEnqueue");

    if (!request)
    {
       PEG_METHOD_EXIT();
       return;
    }

    AutoPtr<CIMOperationRequestMessage> req(
        dynamic_cast<CIMOperationRequestMessage*>(request));

    PEGASUS_ASSERT(req.get());

    //
    // Get the HTTPConnection queue id
    //
    QueueIdStack qis = req->queueIds.copyAndPop();

    Uint32 queueId = qis.top();

    // Set the client's requested language into this service thread.
    // This will allow functions in this service to return messages
    // in the correct language.
    req->updateThreadLanguages();

    //
    // If CIMOM is shutting down, return "Service Unavailable" response
    //
    if (_serverTerminating)
    {
        Buffer message;
        message = XmlWriter::formatHttpErrorRspMessage(
            HTTP_STATUS_SERVICEUNAVAILABLE,
            String::EMPTY,
            "CIM Server is shutting down.");

        sendResponse(queueId, message);
        PEG_METHOD_EXIT();
        return;
    }

    String userName = ((IdentityContainer)(req->operationContext.get(
        IdentityContainer::NAME))).getUserName();
    String authType = req->authType;
    CIMNamespaceName nameSpace = req->nameSpace;
    String cimMethodName;

    switch (req->getType())
    {
        case CIM_GET_CLASS_REQUEST_MESSAGE:
            cimMethodName = "GetClass";
            break;

        case CIM_GET_INSTANCE_REQUEST_MESSAGE:
            cimMethodName = "GetInstance";
            break;

        case CIM_DELETE_CLASS_REQUEST_MESSAGE:
            cimMethodName = "DeleteClass";
            break;

        case CIM_DELETE_INSTANCE_REQUEST_MESSAGE:
            cimMethodName = "DeleteInstance";
            break;

        case CIM_CREATE_CLASS_REQUEST_MESSAGE:
            cimMethodName = "CreateClass";
            break;

        case CIM_CREATE_INSTANCE_REQUEST_MESSAGE:
            cimMethodName = "CreateInstance";
            break;

        case CIM_MODIFY_CLASS_REQUEST_MESSAGE:
            cimMethodName = "ModifyClass";
            break;

        case CIM_MODIFY_INSTANCE_REQUEST_MESSAGE:
            cimMethodName = "ModifyInstance";
            break;

        case CIM_ENUMERATE_CLASSES_REQUEST_MESSAGE:
            cimMethodName = "EnumerateClasses";
            break;

        case CIM_ENUMERATE_CLASS_NAMES_REQUEST_MESSAGE:
            cimMethodName = "EnumerateClassNames";
            break;

        case CIM_ENUMERATE_INSTANCES_REQUEST_MESSAGE:
            cimMethodName = "EnumerateInstances";
            break;

        case CIM_ENUMERATE_INSTANCE_NAMES_REQUEST_MESSAGE:
            cimMethodName = "EnumerateInstanceNames";
            break;

        case CIM_EXEC_QUERY_REQUEST_MESSAGE:
            cimMethodName = "ExecQuery";
            break;

        case CIM_ASSOCIATORS_REQUEST_MESSAGE:
            cimMethodName = "Associators";
            break;

        case CIM_ASSOCIATOR_NAMES_REQUEST_MESSAGE:
            cimMethodName = "AssociatorNames";
            break;

        case CIM_REFERENCES_REQUEST_MESSAGE:
            cimMethodName = "References";
            break;

        case CIM_REFERENCE_NAMES_REQUEST_MESSAGE:
            cimMethodName = "ReferenceNames";
            break;

        case CIM_GET_PROPERTY_REQUEST_MESSAGE:
            cimMethodName = "GetProperty";
            break;

        case CIM_SET_PROPERTY_REQUEST_MESSAGE:
            cimMethodName = "SetProperty";
            break;

        case CIM_GET_QUALIFIER_REQUEST_MESSAGE:
            cimMethodName = "GetQualifier";
            break;

        case CIM_SET_QUALIFIER_REQUEST_MESSAGE:
            cimMethodName = "SetQualifier";
            break;

        case CIM_DELETE_QUALIFIER_REQUEST_MESSAGE:
            cimMethodName = "DeleteQualifier";
            break;

        case CIM_ENUMERATE_QUALIFIERS_REQUEST_MESSAGE:
            cimMethodName = "EnumerateQualifiers";
            break;

        case CIM_INVOKE_METHOD_REQUEST_MESSAGE:
            cimMethodName = "InvokeMethod";
            break;

        default:
            PEGASUS_UNREACHABLE(PEGASUS_ASSERT(0);)
            break;
    }

#ifdef PEGASUS_ZOS_SECURITY
    if (checkRequestTypeAuthorizationZOS(
            req->getType(), userName, nameSpace) == false)
    {
        //
        // user is not authorized, send an
        // error message to the requesting client.
        //
        if (cimMethodName == "InvokeMethod")
        {
            sendMethodError(
                queueId,
                req->getHttpMethod(),
                req->messageId,
                ((CIMInvokeMethodRequestMessage*)req.get())->methodName,
                PEGASUS_CIM_EXCEPTION_L(CIM_ERR_ACCESS_DENIED,
                    MessageLoaderParms(
                        "Server.CIMOperationRequestAuthorizer."
                            "NAMESPACE_AUTHORIZATION_FAILED",
                        "User '$0' is not authorized to run '$1' in the "
                            "namespace '$2'",
                        userName, cimMethodName, nameSpace.getString())));
        }
        else
        {
            sendIMethodError(
                queueId,
                req->getHttpMethod(),
                req->messageId,
                cimMethodName,
                PEGASUS_CIM_EXCEPTION_L(CIM_ERR_ACCESS_DENIED,
                    MessageLoaderParms(
                        "Server.CIMOperationRequestAuthorizer."
                            "NAMESPACE_AUTHORIZATION_FAILED",
                        "User '$0' is not authorized to run '$1' in the "
                            "namespace '$2'",
                        userName, cimMethodName, nameSpace.getString())));
        }
        PEG_METHOD_EXIT();
        return;
    }
#endif

#ifdef PEGASUS_ENABLE_USERGROUP_AUTHORIZATION
    //
    // If the user is not privileged and authorized user group is specified,
    // then perform the user group authorization check.
    //
    try
    {
        if ( ! System::isPrivilegedUser(userName) )
        {
            const Uint32 size = _authorizedUserGroups.size();

            if (size > 0)
            {
                Boolean authorized = false;

                //
                // Check if the user name is in the authorized user groups.
                //
                for (Uint32 i = 0; i < size; i++)
                {
                    //
                    // Check if the user is a member of the group
                    //
                    if (System::isGroupMember(userName.getCString(),
                            _authorizedUserGroups[i].getCString()))
                    {
                        authorized = true;
                        break;
                    }
                }

                //
                // If the user is not a member of any of the authorized
                // user groups then generate error response.
                //
                if (!authorized)
                {
                    PEG_TRACE((TRC_SERVER, Tracer::LEVEL1,
                        "Authorization Failed: User '%s' "
                        "is not a member of the authorized groups",
                        (const char*)userName.getCString()));

                    MessageLoaderParms msgLoaderParms(
                        "Server.CIMOperationRequestAuthorizer."
                            "NOT_IN_AUTHORIZED_GRP",
                        "User '$0' is not authorized to access CIM data.",
                        userName);

                    //
                    // user is not in the authorized user groups, send an
                    // error message to the requesting client.
                    //
                    if (cimMethodName == "InvokeMethod")
                    {
                        sendMethodError(
                            queueId,
                            req->getHttpMethod(),
                            req->messageId,
                            ((CIMInvokeMethodRequestMessage*)req.get())->
                                methodName,
                            PEGASUS_CIM_EXCEPTION_L(CIM_ERR_ACCESS_DENIED,
                                msgLoaderParms));
                        PEG_METHOD_EXIT();
                        return;
                    }
                    else
                    {
                        sendIMethodError(
                            queueId,
                            req->getHttpMethod(),
                            req->messageId,
                            cimMethodName,
                            PEGASUS_CIM_EXCEPTION_L(CIM_ERR_ACCESS_DENIED,
                                msgLoaderParms));
                        PEG_METHOD_EXIT();
                        return;
                    }
                }
            }
        }
    }
    catch (InternalSystemError& ise)
    {
        sendIMethodError(
            queueId,
            req->getHttpMethod(),
            req->messageId,
            cimMethodName,
            PEGASUS_CIM_EXCEPTION(CIM_ERR_ACCESS_DENIED, ise.getMessage()));
        PEG_METHOD_EXIT();
        return;
    }
#endif  // #ifdef PEGASUS_ENABLE_USERGROUP_AUTHORIZATION

    //
    // Get a config manager instance
    //
    ConfigManager* configManager = ConfigManager::getInstance();

    //
    // Do namespace authorization verification
    //
    if (ConfigManager::parseBooleanValue(
        configManager->getCurrentValue("enableNamespaceAuthorization")))
    {
        //
        // If the user is not privileged, perform the authorization check.
        //
        if (!System::isPrivilegedUser(userName))
        {
#ifndef PEGASUS_PAM_AUTHENTICATION
            UserManager* userManager = UserManager::getInstance();

            if (!userManager ||
                !userManager->verifyAuthorization(
                     userName, nameSpace, cimMethodName))
            {
                if (cimMethodName == "InvokeMethod")
                {
                    sendMethodError(
                      queueId,
                      req->getHttpMethod(),
                      req->messageId,
                      ((CIMInvokeMethodRequestMessage*)req.get())->methodName,
                      PEGASUS_CIM_EXCEPTION_L(CIM_ERR_ACCESS_DENIED,
                          MessageLoaderParms(
                              "Server.CIMOperationRequestAuthorizer."
                                  "NAMESPACE_AUTHORIZATION_FAILED",
                              "User '$0' is not authorized to run '$1' in the "
                                  "namespace '$2'",
                              userName, cimMethodName, nameSpace.getString())));
                }
                else
                {
                    sendIMethodError(
                        queueId,
                        req->getHttpMethod(),
                        req->messageId,
                        cimMethodName,
                        PEGASUS_CIM_EXCEPTION_L(CIM_ERR_ACCESS_DENIED,
                            MessageLoaderParms(
                                "Server.CIMOperationRequestAuthorizer."
                                    "NAMESPACE_AUTHORIZATION_FAILED",
                                "User '$0' is not authorized to run '$1' in "
                                    "the namespace '$2'",
                                userName,
                                cimMethodName,
                                nameSpace.getString())));
                }

                PEG_METHOD_EXIT();
                return;
            }
#endif
        }
    }

    //
    // Enqueue the request
    //
    _outputQueue->enqueue(req.release());

    PEG_METHOD_EXIT();
}

void CIMOperationRequestAuthorizer::handleEnqueue()
{
    PEG_METHOD_ENTER(TRC_SERVER,
        "CIMOperationRequestAuthorizer::handleEnqueue");

    Message* request = dequeue();
    if (request)
    {
        handleEnqueue(request);
    }

    PEG_METHOD_EXIT();
}

void CIMOperationRequestAuthorizer::setServerTerminating(Boolean flag)
{
    PEG_METHOD_ENTER(TRC_SERVER,
        "CIMOperationRequestAuthorizer::setServerTerminating");

    _serverTerminating = flag;

    PEG_METHOD_EXIT();
}

Array<String> CIMOperationRequestAuthorizer::_getAuthorizedUserGroups()
{
    PEG_METHOD_ENTER(TRC_SERVER,
        "CIMOperationRequestAuthorizer::getAuthorizedUserGroups");

    Array<String> authorizedGroups;

    String groupNames;

    //
    // Get a config manager instance
    //
    ConfigManager* configManager = ConfigManager::getInstance();

    groupNames = configManager->getCurrentValue("authorizedUserGroups");

    //
    // Check if the group name is empty
    //
    if (groupNames.size() == 0 )
    {
        PEG_METHOD_EXIT();
        return authorizedGroups;
    }

    //
    // Append _GROUPNAME_SEPARATOR to the end of the groups
    //
    groupNames.append(_GROUPNAME_SEPARATOR);

    Uint32 position = 0;
    String groupName;

    while (groupNames.size() != 0 )
    {
        //
        // Get a group name from user groups
        // User groups are separated by _GROUPNAME_SEPARATOR
        //
        position = groupNames.find(_GROUPNAME_SEPARATOR);
        groupName = groupNames.subString(0,(position));

        authorizedGroups.append(groupName);

        // Remove the searched group name
        groupNames.remove(0, position + 1);
    }

    PEG_METHOD_EXIT();

    return authorizedGroups;
}

PEGASUS_NAMESPACE_END
