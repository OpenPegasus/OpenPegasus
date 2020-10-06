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

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/BinaryCodec.h>
#include <iostream>
#include <Pegasus/Common/Constants.h>
#include <Pegasus/Common/XmlWriter.h>
#include <Pegasus/Common/HTTPMessage.h>
#include "CIMOperationRequestEncoder.h"
#include "CIMClientRep.h"

PEGASUS_USING_STD;

PEGASUS_NAMESPACE_BEGIN

static const String COOKIE_HEADER = "\r\nCookie: ";

// append either ClassPath or InstancePath based IParameter.
// This function local to client to clean up use of the getKeyBindings test
// in the common code.  It is used by the associators, etc. requests
// because the path can be either a class or instance.
void _appendObjectNameIParameter(
    Buffer& out,
    const char* name,
    const CIMObjectPath& objectName)
{
    //
    //  ATTN-CAKG-P2-20020726:  The following condition does not correctly
    //  distinguish instanceNames from classNames in every case
    //  The instanceName of a singleton instance of a keyless class also
    //  has no key bindings. See BUG_3302
    //
    if (objectName.getKeyBindings ().size () == 0)
    {
        XmlWriter::appendClassNameIParameter(
            out, name, objectName.getClassName());
    }
    else
    {
        XmlWriter::appendInstanceNameIParameter(
            out, name, objectName);
    }
}

CIMOperationRequestEncoder::CIMOperationRequestEncoder(
    MessageQueue* outputQueue,
    const String& hostName,
    ClientAuthenticator* authenticator,
    bool binaryRequest,
    bool binaryResponse)
    :
    MessageQueue(PEGASUS_QUEUENAME_OPREQENCODER),
    _outputQueue(outputQueue),
    _hostName(hostName.getCString()),
    _authenticator(authenticator),
    _binaryRequest(binaryRequest),
    _binaryResponse(binaryResponse)
{
    dataStore_prt=NULL;
}

CIMOperationRequestEncoder::~CIMOperationRequestEncoder()
{
}

void CIMOperationRequestEncoder::handleEnqueue()
{
    Message* message = dequeue();

    if (!message)
        return;

    _authenticator->setRequestMessage(message);

    //
    // Encode request as binary request.
    //

    if (_binaryRequest)
    {
        CIMOperationRequestMessage* msg =
            dynamic_cast<CIMOperationRequestMessage*>(message);

        if (msg)
        {
            Buffer buf;

            if (BinaryCodec::encodeRequest(buf, _hostName,
                _authenticator->buildRequestAuthHeader(), msg, _binaryResponse))
            {
                _sendRequest(buf);
                return;
            }

            // Drop through and encode as an XML request below.
        }
    }

    //
    // Encode request as an XML request.
    //

    switch (message->getType())
    {
        case CIM_CREATE_CLASS_REQUEST_MESSAGE:
            _encodeCreateClassRequest(
                (CIMCreateClassRequestMessage*)message);
            break;

        case CIM_GET_CLASS_REQUEST_MESSAGE:
            _encodeGetClassRequest((CIMGetClassRequestMessage*)message);
            break;

        case CIM_MODIFY_CLASS_REQUEST_MESSAGE:
            _encodeModifyClassRequest(
                (CIMModifyClassRequestMessage*)message);
            break;

        case CIM_ENUMERATE_CLASS_NAMES_REQUEST_MESSAGE:
            _encodeEnumerateClassNamesRequest(
                (CIMEnumerateClassNamesRequestMessage*)message);
            break;

        case CIM_ENUMERATE_CLASSES_REQUEST_MESSAGE:
            _encodeEnumerateClassesRequest(
                (CIMEnumerateClassesRequestMessage*)message);
            break;

        case CIM_DELETE_CLASS_REQUEST_MESSAGE:
            _encodeDeleteClassRequest(
                (CIMDeleteClassRequestMessage*)message);
            break;

        case CIM_CREATE_INSTANCE_REQUEST_MESSAGE:
            _encodeCreateInstanceRequest(
                (CIMCreateInstanceRequestMessage*)message);
            break;

        case CIM_GET_INSTANCE_REQUEST_MESSAGE:
            _encodeGetInstanceRequest((CIMGetInstanceRequestMessage*)message);
            break;

        case CIM_MODIFY_INSTANCE_REQUEST_MESSAGE:
            _encodeModifyInstanceRequest(
                (CIMModifyInstanceRequestMessage*)message);
            break;

        case CIM_ENUMERATE_INSTANCE_NAMES_REQUEST_MESSAGE:
            _encodeEnumerateInstanceNamesRequest(
                (CIMEnumerateInstanceNamesRequestMessage*)message);
            break;

        case CIM_ENUMERATE_INSTANCES_REQUEST_MESSAGE:
            _encodeEnumerateInstancesRequest(
                (CIMEnumerateInstancesRequestMessage*)message);
            break;

        case CIM_DELETE_INSTANCE_REQUEST_MESSAGE:
            _encodeDeleteInstanceRequest(
                (CIMDeleteInstanceRequestMessage*)message);
            break;

        case CIM_SET_QUALIFIER_REQUEST_MESSAGE:
            _encodeSetQualifierRequest(
                (CIMSetQualifierRequestMessage*)message);
            break;

        case CIM_GET_QUALIFIER_REQUEST_MESSAGE:
            _encodeGetQualifierRequest(
                (CIMGetQualifierRequestMessage*)message);
            break;

        case CIM_ENUMERATE_QUALIFIERS_REQUEST_MESSAGE:
            _encodeEnumerateQualifiersRequest(
                (CIMEnumerateQualifiersRequestMessage*)message);
            break;

        case CIM_DELETE_QUALIFIER_REQUEST_MESSAGE:
            _encodeDeleteQualifierRequest(
                (CIMDeleteQualifierRequestMessage*)message);
            break;

        case CIM_REFERENCE_NAMES_REQUEST_MESSAGE:
            _encodeReferenceNamesRequest(
                (CIMReferenceNamesRequestMessage*)message);
            break;

        case CIM_REFERENCES_REQUEST_MESSAGE:
            _encodeReferencesRequest(
                (CIMReferencesRequestMessage*)message);
            break;

        case CIM_ASSOCIATOR_NAMES_REQUEST_MESSAGE:
            _encodeAssociatorNamesRequest(
                (CIMAssociatorNamesRequestMessage*)message);
            break;

        case CIM_ASSOCIATORS_REQUEST_MESSAGE:
            _encodeAssociatorsRequest(
                (CIMAssociatorsRequestMessage*)message);
            break;

        case CIM_EXEC_QUERY_REQUEST_MESSAGE:
            _encodeExecQueryRequest(
                (CIMExecQueryRequestMessage*)message);
            break;

        case CIM_GET_PROPERTY_REQUEST_MESSAGE:
            _encodeGetPropertyRequest((CIMGetPropertyRequestMessage*)message);
            break;

        case CIM_SET_PROPERTY_REQUEST_MESSAGE:
            _encodeSetPropertyRequest((CIMSetPropertyRequestMessage*)message);
            break;

        case CIM_INVOKE_METHOD_REQUEST_MESSAGE:
            _encodeInvokeMethodRequest(
                (CIMInvokeMethodRequestMessage*)message);
            break;

// EXP_PULL_BEGIN
        case CIM_OPEN_ENUMERATE_INSTANCES_REQUEST_MESSAGE:
            _encodeOpenEnumerateInstancesRequest(
                (CIMOpenEnumerateInstancesRequestMessage*)message);
            break;

        case CIM_OPEN_ENUMERATE_INSTANCE_PATHS_REQUEST_MESSAGE:
            _encodeOpenEnumerateInstancePathsRequest(
                (CIMOpenEnumerateInstancePathsRequestMessage*)message);
            break;

        case CIM_OPEN_REFERENCE_INSTANCES_REQUEST_MESSAGE:
            _encodeOpenReferenceInstancesRequest(
                (CIMOpenReferenceInstancesRequestMessage*)message);
            break;

        case CIM_OPEN_REFERENCE_INSTANCE_PATHS_REQUEST_MESSAGE:
            _encodeOpenReferenceInstancePathsRequest(
                (CIMOpenReferenceInstancePathsRequestMessage*)message);
            break;

        case CIM_OPEN_ASSOCIATOR_INSTANCES_REQUEST_MESSAGE:
            _encodeOpenAssociatorInstancesRequest(
                (CIMOpenAssociatorInstancesRequestMessage*)message);
            break;

        case CIM_OPEN_ASSOCIATOR_INSTANCE_PATHS_REQUEST_MESSAGE:
            _encodeOpenAssociatorInstancePathsRequest(
                (CIMOpenAssociatorInstancePathsRequestMessage*)message);
            break;

        case CIM_PULL_INSTANCES_WITH_PATH_REQUEST_MESSAGE:
            _encodePullInstancesWithPathRequest(
                (CIMPullInstancesWithPathRequestMessage*)message);
            break;

        case CIM_PULL_INSTANCE_PATHS_REQUEST_MESSAGE:
            _encodePullInstancePathsRequest(
                (CIMPullInstancePathsRequestMessage*)message);
            break;

        case CIM_PULL_INSTANCES_REQUEST_MESSAGE:
            _encodePullInstancesRequest(
                (CIMPullInstancesRequestMessage*)message);
            break;

        case CIM_CLOSE_ENUMERATION_REQUEST_MESSAGE:
            _encodeCloseEnumerationRequest(
                (CIMCloseEnumerationRequestMessage*)message);
            break;

        case CIM_ENUMERATION_COUNT_REQUEST_MESSAGE:
            _encodeEnumerationCountRequest(
                (CIMEnumerationCountRequestMessage*)message);
            break;

        case CIM_OPEN_QUERY_INSTANCES_REQUEST_MESSAGE:
            _encodeOpenQueryInstancesRequest(
                (CIMOpenQueryInstancesRequestMessage*)message);
            break;
// EXP_PULL_END

        default:
            // Unexpected message type
            PEGASUS_UNREACHABLE(PEGASUS_ASSERT(0);)
            break;
    }

    //ATTN: Do not delete the message here.
    //
    // ClientAuthenticator needs this message for resending the request on
    // authentication challenge from the server. The message is deleted in
    // the decoder after receiving the valid response from thr server.
    //
    //delete message;
}

void CIMOperationRequestEncoder::setDataStorePointer(
    ClientPerfDataStore* perfDataStore_ptr)
{   dataStore_prt = perfDataStore_ptr;
}

// l10n Added accept language and content language support starting here

void CIMOperationRequestEncoder::_encodeCreateClassRequest(
    CIMCreateClassRequestMessage* message)
{
    Buffer params;
    XmlWriter::appendClassIParameter(params, "NewClass", message->newClass);

    Buffer buffer = XmlWriter::formatSimpleIMethodReqMessage(
        _hostName,
        message->nameSpace, CIMName ("CreateClass"), message->messageId,
        message->getHttpMethod(),
        _authenticator->buildRequestAuthHeader(),
        ((AcceptLanguageListContainer)message->operationContext.get(
            AcceptLanguageListContainer::NAME)).getLanguages(),
        ((ContentLanguageListContainer)message->operationContext.get(
            ContentLanguageListContainer::NAME)).getLanguages(),
        params, _binaryResponse);
    _sendRequest(buffer);
}

void CIMOperationRequestEncoder::_encodeGetClassRequest(
    CIMGetClassRequestMessage* message)
{
    Buffer params;

    XmlWriter::appendClassNameIParameter(
        params, "ClassName", message->className);

    if (message->localOnly != true)
        XmlWriter::appendBooleanIParameter(params, "LocalOnly", false);

    if (message->includeQualifiers != true)
        XmlWriter::appendBooleanIParameter(params, "IncludeQualifiers", false);

    if (message->includeClassOrigin != false)
        XmlWriter::appendBooleanIParameter(params, "IncludeClassOrigin", true);

    if (!message->propertyList.isNull())
        XmlWriter::appendPropertyListIParameter(
            params, message->propertyList);

    Buffer buffer = XmlWriter::formatSimpleIMethodReqMessage(
        _hostName, message->nameSpace, CIMName ("GetClass"), message->messageId,
        message->getHttpMethod(),
        _authenticator->buildRequestAuthHeader(),
        ((AcceptLanguageListContainer)message->operationContext.get(
            AcceptLanguageListContainer::NAME)).getLanguages(),
        ((ContentLanguageListContainer)message->operationContext.get(
            ContentLanguageListContainer::NAME)).getLanguages(),
        params, _binaryResponse);

    _sendRequest(buffer);
}

void CIMOperationRequestEncoder::_encodeModifyClassRequest(
    CIMModifyClassRequestMessage* message)
{
    Buffer params;

    XmlWriter::appendClassIParameter(
        params, "ModifiedClass", message->modifiedClass);

    Buffer buffer = XmlWriter::formatSimpleIMethodReqMessage(_hostName,
        message->nameSpace, CIMName ("ModifyClass"), message->messageId,
        message->getHttpMethod(),
        _authenticator->buildRequestAuthHeader(),
        ((AcceptLanguageListContainer)message->operationContext.get(
            AcceptLanguageListContainer::NAME)).getLanguages(),
        ((ContentLanguageListContainer)message->operationContext.get(
            ContentLanguageListContainer::NAME)).getLanguages(),
        params, _binaryResponse);

    _sendRequest(buffer);
}

void CIMOperationRequestEncoder::_encodeEnumerateClassNamesRequest(
    CIMEnumerateClassNamesRequestMessage* message)
{
    Buffer params;

    if (!message->className.isNull())
        XmlWriter::appendClassNameIParameter(
            params, "ClassName", message->className);

    if (message->deepInheritance != false)
        XmlWriter::appendBooleanIParameter(params, "DeepInheritance", true);

    Buffer buffer = XmlWriter::formatSimpleIMethodReqMessage(_hostName,
        message->nameSpace, CIMName ("EnumerateClassNames"), message->messageId,
        message->getHttpMethod(),
        _authenticator->buildRequestAuthHeader(),
        ((AcceptLanguageListContainer)message->operationContext.get(
            AcceptLanguageListContainer::NAME)).getLanguages(),
        ((ContentLanguageListContainer)message->operationContext.get(
            ContentLanguageListContainer::NAME)).getLanguages(),
        params, _binaryResponse);

    _sendRequest(buffer);
}

void CIMOperationRequestEncoder::_encodeEnumerateClassesRequest(
    CIMEnumerateClassesRequestMessage* message)
{
    Buffer params;

    if (!message->className.isNull())
        XmlWriter::appendClassNameIParameter(
            params, "ClassName", message->className);

    if (message->deepInheritance != false)
        XmlWriter::appendBooleanIParameter(params, "DeepInheritance", true);

    if (message->localOnly != true)
        XmlWriter::appendBooleanIParameter(params, "LocalOnly", false);

    if (message->includeQualifiers != true)
        XmlWriter::appendBooleanIParameter(
            params, "IncludeQualifiers", false);

    if (message->includeClassOrigin != false)
        XmlWriter::appendBooleanIParameter(
            params, "IncludeClassOrigin", true);

    Buffer buffer = XmlWriter::formatSimpleIMethodReqMessage(_hostName,
        message->nameSpace, CIMName ("EnumerateClasses"), message->messageId,
        message->getHttpMethod(),
        _authenticator->buildRequestAuthHeader(),
        ((AcceptLanguageListContainer)message->operationContext.get(
            AcceptLanguageListContainer::NAME)).getLanguages(),
        ((ContentLanguageListContainer)message->operationContext.get(
            ContentLanguageListContainer::NAME)).getLanguages(),
        params, _binaryResponse);

    _sendRequest(buffer);
}

void CIMOperationRequestEncoder::_encodeDeleteClassRequest(
    CIMDeleteClassRequestMessage* message)
{
    Buffer params;

    if (!message->className.isNull())
        XmlWriter::appendClassNameIParameter(
            params, "ClassName", message->className);

    Buffer buffer = XmlWriter::formatSimpleIMethodReqMessage(_hostName,
        message->nameSpace, CIMName ("DeleteClass"), message->messageId,
        message->getHttpMethod(),
        _authenticator->buildRequestAuthHeader(),
        ((AcceptLanguageListContainer)message->operationContext.get(
            AcceptLanguageListContainer::NAME)).getLanguages(),
        ((ContentLanguageListContainer)message->operationContext.get(
            ContentLanguageListContainer::NAME)).getLanguages(),
        params, _binaryResponse);

    _sendRequest(buffer);
}

void CIMOperationRequestEncoder::_encodeCreateInstanceRequest(
    CIMCreateInstanceRequestMessage* message)
{
    Buffer params;

    XmlWriter::appendInstanceIParameter(
        params, "NewInstance", message->newInstance);

    Buffer buffer = XmlWriter::formatSimpleIMethodReqMessage(_hostName,
        message->nameSpace, CIMName ("CreateInstance"), message->messageId,
        message->getHttpMethod(),
        _authenticator->buildRequestAuthHeader(),
        ((AcceptLanguageListContainer)message->operationContext.get(
            AcceptLanguageListContainer::NAME)).getLanguages(),
        ((ContentLanguageListContainer)message->operationContext.get(
            ContentLanguageListContainer::NAME)).getLanguages(),
        params, _binaryResponse);

    _sendRequest(buffer);
}

void CIMOperationRequestEncoder::_encodeGetInstanceRequest(
    CIMGetInstanceRequestMessage* message)
{
    Buffer params;

    XmlWriter::appendInstanceNameIParameter(
        params, "InstanceName", message->instanceName);

    if (message->localOnly != true)
        XmlWriter::appendBooleanIParameter(
            params, "LocalOnly", false);

    if (message->includeQualifiers != false)
        XmlWriter::appendBooleanIParameter(
            params, "IncludeQualifiers", true);

    if (message->includeClassOrigin != false)
        XmlWriter::appendBooleanIParameter(
            params, "IncludeClassOrigin", true);

    if (!message->propertyList.isNull())
        XmlWriter::appendPropertyListIParameter(
            params, message->propertyList);

        Buffer buffer = XmlWriter::formatSimpleIMethodReqMessage(_hostName,
        message->nameSpace, CIMName ("GetInstance"), message->messageId,
        message->getHttpMethod(),
        _authenticator->buildRequestAuthHeader(),
        ((AcceptLanguageListContainer)message->operationContext.get(
            AcceptLanguageListContainer::NAME)).getLanguages(),
        ((ContentLanguageListContainer)message->operationContext.get(
            ContentLanguageListContainer::NAME)).getLanguages(),
        params, _binaryResponse);

    _sendRequest(buffer);
}

void CIMOperationRequestEncoder::_encodeModifyInstanceRequest(
    CIMModifyInstanceRequestMessage* message)
{
    Buffer params;
    XmlWriter::appendNamedInstanceIParameter(
        params, "ModifiedInstance", message->modifiedInstance);

    if (message->includeQualifiers != true)
        XmlWriter::appendBooleanIParameter(
            params, "IncludeQualifiers", false);

    if (!message->propertyList.isNull())
        XmlWriter::appendPropertyListIParameter(
            params, message->propertyList);

    Buffer buffer = XmlWriter::formatSimpleIMethodReqMessage(_hostName,
        message->nameSpace, CIMName ("ModifyInstance"), message->messageId,
        message->getHttpMethod(),
        _authenticator->buildRequestAuthHeader(),
        ((AcceptLanguageListContainer)message->operationContext.get(
            AcceptLanguageListContainer::NAME)).getLanguages(),
        ((ContentLanguageListContainer)message->operationContext.get(
            ContentLanguageListContainer::NAME)).getLanguages(),
        params, _binaryResponse);

    _sendRequest(buffer);
}

void CIMOperationRequestEncoder::_encodeEnumerateInstanceNamesRequest(
    CIMEnumerateInstanceNamesRequestMessage* message)
{
    Buffer params;

    XmlWriter::appendClassNameIParameter(
        params, "ClassName", message->className);


    Buffer buffer = XmlWriter::formatSimpleIMethodReqMessage(_hostName,
        message->nameSpace, CIMName ("EnumerateInstanceNames"),
        message->messageId, message->getHttpMethod(),
        _authenticator->buildRequestAuthHeader(),
        ((AcceptLanguageListContainer)message->operationContext.get(
            AcceptLanguageListContainer::NAME)).getLanguages(),
        ((ContentLanguageListContainer)message->operationContext.get(
            ContentLanguageListContainer::NAME)).getLanguages(),
        params, _binaryResponse);

    _sendRequest(buffer);
}

void CIMOperationRequestEncoder::_encodeEnumerateInstancesRequest(
    CIMEnumerateInstancesRequestMessage* message)
{
    Buffer params;

    XmlWriter::appendClassNameIParameter(
        params, "ClassName", message->className);

    if (message->localOnly != true)
        XmlWriter::appendBooleanIParameter(params, "LocalOnly", false);

    if (message->deepInheritance != true)
        XmlWriter::appendBooleanIParameter(params, "DeepInheritance", false);

    if (message->includeQualifiers != false)
        XmlWriter::appendBooleanIParameter(
            params, "IncludeQualifiers", true);

    if (message->includeClassOrigin != false)
        XmlWriter::appendBooleanIParameter(
            params, "IncludeClassOrigin", true);

    if (!message->propertyList.isNull())
        XmlWriter::appendPropertyListIParameter(
            params, message->propertyList);

    Buffer buffer = XmlWriter::formatSimpleIMethodReqMessage(_hostName,
        message->nameSpace, CIMName ("EnumerateInstances"), message->messageId,
        message->getHttpMethod(),
        _authenticator->buildRequestAuthHeader(),
        ((AcceptLanguageListContainer)message->operationContext.get(
            AcceptLanguageListContainer::NAME)).getLanguages(),
        ((ContentLanguageListContainer)message->operationContext.get(
            ContentLanguageListContainer::NAME)).getLanguages(),
        params, _binaryResponse);

    _sendRequest(buffer);
}

void CIMOperationRequestEncoder::_encodeDeleteInstanceRequest(
    CIMDeleteInstanceRequestMessage* message)
{
    Buffer params;

    XmlWriter::appendInstanceNameIParameter(
        params, "InstanceName", message->instanceName);

    Buffer buffer = XmlWriter::formatSimpleIMethodReqMessage(_hostName,
        message->nameSpace, CIMName ("DeleteInstance"), message->messageId,
        message->getHttpMethod(),
        _authenticator->buildRequestAuthHeader(),
        ((AcceptLanguageListContainer)message->operationContext.get(
            AcceptLanguageListContainer::NAME)).getLanguages(),
        ((ContentLanguageListContainer)message->operationContext.get(
            ContentLanguageListContainer::NAME)).getLanguages(),
        params, _binaryResponse);

    _sendRequest(buffer);
}

void CIMOperationRequestEncoder::_encodeGetPropertyRequest(
    CIMGetPropertyRequestMessage* message)
{
    Buffer params;

    XmlWriter::appendInstanceNameIParameter(
        params, "InstanceName", message->instanceName);

    XmlWriter::appendPropertyNameIParameter(
        params, message->propertyName);

    Buffer buffer = XmlWriter::formatSimpleIMethodReqMessage(_hostName,
        message->nameSpace, CIMName ("GetProperty"), message->messageId,
        message->getHttpMethod(),
        _authenticator->buildRequestAuthHeader(),
        ((AcceptLanguageListContainer)message->operationContext.get(
            AcceptLanguageListContainer::NAME)).getLanguages(),
        ((ContentLanguageListContainer)message->operationContext.get(
            ContentLanguageListContainer::NAME)).getLanguages(),
        params, _binaryResponse);

    _sendRequest(buffer);
}

void CIMOperationRequestEncoder::_encodeSetPropertyRequest(
    CIMSetPropertyRequestMessage* message)
{
    Buffer params;

    XmlWriter::appendInstanceNameIParameter(
        params, "InstanceName", message->instanceName);

    XmlWriter::appendPropertyNameIParameter(
        params, message->propertyName);

    if (!message->newValue.isNull())
        XmlWriter::appendPropertyValueIParameter(
            params, "NewValue", message->newValue);

    Buffer buffer = XmlWriter::formatSimpleIMethodReqMessage(_hostName,
        message->nameSpace, CIMName ("SetProperty"), message->messageId,
        message->getHttpMethod(),
        _authenticator->buildRequestAuthHeader(),
        ((AcceptLanguageListContainer)message->operationContext.get(
            AcceptLanguageListContainer::NAME)).getLanguages(),
        ((ContentLanguageListContainer)message->operationContext.get(
            ContentLanguageListContainer::NAME)).getLanguages(),
        params, _binaryResponse);

    _sendRequest(buffer);
}

void CIMOperationRequestEncoder::_encodeSetQualifierRequest(
    CIMSetQualifierRequestMessage* message)
{
    Buffer params;
    XmlWriter::appendQualifierDeclarationIParameter(
        params, "QualifierDeclaration", message->qualifierDeclaration);

    Buffer buffer = XmlWriter::formatSimpleIMethodReqMessage(_hostName,
        message->nameSpace, CIMName ("SetQualifier"), message->messageId,
        message->getHttpMethod(),
        _authenticator->buildRequestAuthHeader(),
        ((AcceptLanguageListContainer)message->operationContext.get(
            AcceptLanguageListContainer::NAME)).getLanguages(),
        ((ContentLanguageListContainer)message->operationContext.get(
            ContentLanguageListContainer::NAME)).getLanguages(),
        params, _binaryResponse);

    _sendRequest(buffer);
}

void CIMOperationRequestEncoder::_encodeGetQualifierRequest(
    CIMGetQualifierRequestMessage* message)
{
    Buffer params;

    if (!message->qualifierName.isNull())
        XmlWriter::appendStringIParameter(
            params, "QualifierName", message->qualifierName.getString());

    Buffer buffer = XmlWriter::formatSimpleIMethodReqMessage(_hostName,
        message->nameSpace, CIMName ("GetQualifier"), message->messageId,
        message->getHttpMethod(),
        _authenticator->buildRequestAuthHeader(),
        ((AcceptLanguageListContainer)message->operationContext.get(
            AcceptLanguageListContainer::NAME)).getLanguages(),
        ((ContentLanguageListContainer)message->operationContext.get(
            ContentLanguageListContainer::NAME)).getLanguages(),
        params, _binaryResponse);

    _sendRequest(buffer);
}

void CIMOperationRequestEncoder::_encodeEnumerateQualifiersRequest(
    CIMEnumerateQualifiersRequestMessage* message)
{
    Buffer params;

    Buffer buffer = XmlWriter::formatSimpleIMethodReqMessage(_hostName,
        message->nameSpace, CIMName ("EnumerateQualifiers"), message->messageId,
        message->getHttpMethod(),
        _authenticator->buildRequestAuthHeader(),
        ((AcceptLanguageListContainer)message->operationContext.get(
            AcceptLanguageListContainer::NAME)).getLanguages(),
        ((ContentLanguageListContainer)message->operationContext.get(
            ContentLanguageListContainer::NAME)).getLanguages(),
        params, _binaryResponse);

    _sendRequest(buffer);
}

void CIMOperationRequestEncoder::_encodeDeleteQualifierRequest(
    CIMDeleteQualifierRequestMessage* message)
{
    Buffer params;

    if (!message->qualifierName.isNull())
        XmlWriter::appendStringIParameter(
            params, "QualifierName", message->qualifierName.getString());

    Buffer buffer = XmlWriter::formatSimpleIMethodReqMessage(_hostName,
        message->nameSpace, CIMName ("DeleteQualifier"), message->messageId,
        message->getHttpMethod(),
        _authenticator->buildRequestAuthHeader(),
        ((AcceptLanguageListContainer)message->operationContext.get(
            AcceptLanguageListContainer::NAME)).getLanguages(),
        ((ContentLanguageListContainer)message->operationContext.get(
            ContentLanguageListContainer::NAME)).getLanguages(),
        params, _binaryResponse);

    _sendRequest(buffer);
}

void CIMOperationRequestEncoder::_encodeReferenceNamesRequest(
    CIMReferenceNamesRequestMessage* message)
{
    Buffer params;

    _appendObjectNameIParameter(
        params, "ObjectName", message->objectName);

    XmlWriter::appendClassNameIParameter(
        params, "ResultClass", message->resultClass);

    //
    //  The Client API has no way to represent a NULL role;
    //  An empty string is interpreted as NULL
    //
    if (message->role.size() != 0)
    {
        XmlWriter::appendStringIParameter(params, "Role", message->role);
    }

    Buffer buffer = XmlWriter::formatSimpleIMethodReqMessage(_hostName,
        message->nameSpace, CIMName ("ReferenceNames"), message->messageId,
        message->getHttpMethod(),
        _authenticator->buildRequestAuthHeader(),
        ((AcceptLanguageListContainer)message->operationContext.get(
            AcceptLanguageListContainer::NAME)).getLanguages(),
        ((ContentLanguageListContainer)message->operationContext.get(
            ContentLanguageListContainer::NAME)).getLanguages(),
        params, _binaryResponse);

    _sendRequest(buffer);
}

void CIMOperationRequestEncoder::_encodeReferencesRequest(
    CIMReferencesRequestMessage* message)
{
    Buffer params;

    _appendObjectNameIParameter(
        params, "ObjectName", message->objectName);

    XmlWriter::appendClassNameIParameter(
        params, "ResultClass", message->resultClass);

    //
    //  The Client API has no way to represent a NULL role;
    //  An empty string is interpreted as NULL
    //
    if (message->role.size() != 0)
    {
        XmlWriter::appendStringIParameter(params, "Role", message->role);
    }

    if (message->includeQualifiers != false)
        XmlWriter::appendBooleanIParameter(params, "IncludeQualifiers", true);

    if (message->includeClassOrigin != false)
        XmlWriter::appendBooleanIParameter(params, "IncludeClassOrigin", true);

    if (!message->propertyList.isNull())
        XmlWriter::appendPropertyListIParameter(
            params, message->propertyList);

    Buffer buffer = XmlWriter::formatSimpleIMethodReqMessage(_hostName,
        message->nameSpace, CIMName ("References"), message->messageId,
        message->getHttpMethod(),
        _authenticator->buildRequestAuthHeader(),
        ((AcceptLanguageListContainer)message->operationContext.get(
            AcceptLanguageListContainer::NAME)).getLanguages(),
        ((ContentLanguageListContainer)message->operationContext.get(
            ContentLanguageListContainer::NAME)).getLanguages(),
        params, _binaryResponse);

    _sendRequest(buffer);
}

void CIMOperationRequestEncoder::_encodeAssociatorNamesRequest(
    CIMAssociatorNamesRequestMessage* message)
{
    Buffer params;

    _appendObjectNameIParameter(
        params, "ObjectName", message->objectName);

    XmlWriter::appendClassNameIParameter(
        params, "AssocClass", message->assocClass);

    XmlWriter::appendClassNameIParameter(
        params, "ResultClass", message->resultClass);

    //
    //  The Client API has no way to represent a NULL role;
    //  An empty string is interpreted as NULL
    //
    if (message->role.size() != 0)
    {
        XmlWriter::appendStringIParameter(params, "Role", message->role);
    }

    //
    //  The Client API has no way to represent a NULL resultRole;
    //  An empty string is interpreted as NULL
    //
    if (message->resultRole.size() != 0)
    {
        XmlWriter::appendStringIParameter(
            params, "ResultRole", message->resultRole);
    }

    Buffer buffer = XmlWriter::formatSimpleIMethodReqMessage(_hostName,
        message->nameSpace, CIMName ("AssociatorNames"), message->messageId,
        message->getHttpMethod(),
        _authenticator->buildRequestAuthHeader(),
        ((AcceptLanguageListContainer)message->operationContext.get(
            AcceptLanguageListContainer::NAME)).getLanguages(),
        ((ContentLanguageListContainer)message->operationContext.get(
            ContentLanguageListContainer::NAME)).getLanguages(),
        params, _binaryResponse);

    _sendRequest(buffer);
}

void CIMOperationRequestEncoder::_encodeAssociatorsRequest(
    CIMAssociatorsRequestMessage* message)
{
    Buffer params;

    _appendObjectNameIParameter(
        params, "ObjectName", message->objectName);

    XmlWriter::appendClassNameIParameter(
        params, "AssocClass", message->assocClass);

    XmlWriter::appendClassNameIParameter(
        params, "ResultClass", message->resultClass);

    //
    //  The Client API has no way to represent a NULL role;
    //  An empty string is interpreted as NULL
    //
    if (message->role.size() != 0)
    {
        XmlWriter::appendStringIParameter(params, "Role", message->role);
    }

    //
    //  The Client API has no way to represent a NULL resultRole;
    //  An empty string is interpreted as NULL
    //
    if (message->resultRole.size() != 0)
    {
        XmlWriter::appendStringIParameter(
            params, "ResultRole", message->resultRole);
    }

    if (message->includeQualifiers != false)
        XmlWriter::appendBooleanIParameter(params, "IncludeQualifiers", true);

    if (message->includeClassOrigin != false)
        XmlWriter::appendBooleanIParameter(params, "IncludeClassOrigin", true);

    if (!message->propertyList.isNull())
    {
        XmlWriter::appendPropertyListIParameter(
            params, message->propertyList);
    }

    Buffer buffer = XmlWriter::formatSimpleIMethodReqMessage(_hostName,
        message->nameSpace, CIMName ("Associators"), message->messageId,
        message->getHttpMethod(),
        _authenticator->buildRequestAuthHeader(),
        ((AcceptLanguageListContainer)message->operationContext.get(
            AcceptLanguageListContainer::NAME)).getLanguages(),
        ((ContentLanguageListContainer)message->operationContext.get(
            ContentLanguageListContainer::NAME)).getLanguages(),
        params, _binaryResponse);

    _sendRequest(buffer);
}

void CIMOperationRequestEncoder::_encodeExecQueryRequest(
    CIMExecQueryRequestMessage* message)
{
    Buffer params;

    XmlWriter::appendStringIParameter(
        params, "QueryLanguage", message->queryLanguage);

    XmlWriter::appendStringIParameter(
        params, "Query", message->query);

    Buffer buffer = XmlWriter::formatSimpleIMethodReqMessage(_hostName,
        message->nameSpace, CIMName ("ExecQuery"), message->messageId,
        message->getHttpMethod(),
        _authenticator->buildRequestAuthHeader(),
        ((AcceptLanguageListContainer)message->operationContext.get(
            AcceptLanguageListContainer::NAME)).getLanguages(),
        ((ContentLanguageListContainer)message->operationContext.get(
            ContentLanguageListContainer::NAME)).getLanguages(),
        params, _binaryResponse);

    _sendRequest(buffer);
}

void CIMOperationRequestEncoder::_encodeInvokeMethodRequest(
    CIMInvokeMethodRequestMessage* message)
{
    Buffer buffer = XmlWriter::formatSimpleMethodReqMessage(_hostName,
        message->nameSpace, message->instanceName, message->methodName,
        message->inParameters, message->messageId,
        message->getHttpMethod(), _authenticator->buildRequestAuthHeader(),
        ((AcceptLanguageListContainer)message->operationContext.get(
            AcceptLanguageListContainer::NAME)).getLanguages(),
        ((ContentLanguageListContainer)message->operationContext.get(
            ContentLanguageListContainer::NAME)).getLanguages(),
        _binaryResponse);

    _sendRequest(buffer);
}

// EXP_PULL_BEGIN
/*
    encode those parameters that are common to all open operations.
    These are defined as common to these operations in DSP200
*/
void _encodeOpenCommonParameters(
    Boolean continueOnError,
    Uint32 maxObjectCount,
    const Uint32Arg& operationTimeout,
    const String& filterQueryLanguage,
    const String& filterQuery,
    Buffer& params)
{
    XmlWriter::appendBooleanIParameter(
        params, "ContinueOnError", continueOnError);

    XmlWriter::appendUint32IParameter(
        params, "MaxObjectCount", maxObjectCount);

    XmlWriter::appendUint32ArgIParameter(
        params, "OperationTimeout", operationTimeout,false);

    XmlWriter::appendStringIParameterIfNotEmpty(
        params, "FilterQueryLanguage", filterQueryLanguage);

    XmlWriter::appendStringIParameterIfNotEmpty(
        params, "filterQuery", filterQuery);
}

void CIMOperationRequestEncoder::_encodeOpenEnumerateInstancesRequest(
    CIMOpenEnumerateInstancesRequestMessage* message)
{
    Buffer params;

    XmlWriter::appendClassNameIParameter(
        params, "ClassName", message->className);

    if (message->deepInheritance != true)
        XmlWriter::appendBooleanIParameter(
            params, "DeepInheritance", false);

    if (message->includeClassOrigin != false)
        XmlWriter::appendBooleanIParameter(
            params, "IncludeClassOrigin", true);

    if (!message->propertyList.isNull())
        XmlWriter::appendPropertyListIParameter(
            params, message->propertyList);

    _encodeOpenCommonParameters(message->continueOnError,
        message->maxObjectCount,
        message->operationTimeout,
        message->filterQueryLanguage,
        message->filterQuery,
        params);

    Buffer buffer = XmlWriter::formatSimpleIMethodReqMessage(_hostName,
        message->nameSpace, CIMName ("OpenEnumerateInstances"),
        message->messageId,
        message->getHttpMethod(),
        _authenticator->buildRequestAuthHeader(),
        ((AcceptLanguageListContainer)message->operationContext.get(
            AcceptLanguageListContainer::NAME)).getLanguages(),
        ((ContentLanguageListContainer)message->operationContext.get(
            ContentLanguageListContainer::NAME)).getLanguages(),
        params, _binaryResponse);

    _sendRequest(buffer);
}


void CIMOperationRequestEncoder::_encodeOpenEnumerateInstancePathsRequest(
    CIMOpenEnumerateInstancePathsRequestMessage* message)
{
    Buffer params;

    XmlWriter::appendClassNameIParameter(
        params, "ClassName", message->className);

    _encodeOpenCommonParameters(message->continueOnError,
        message->maxObjectCount,
        message->operationTimeout,
        message->filterQueryLanguage,
        message->filterQuery,
        params);

    Buffer buffer = XmlWriter::formatSimpleIMethodReqMessage(_hostName,
        message->nameSpace, CIMName ("OpenEnumerateInstancePaths"),
        message->messageId,
        message->getHttpMethod(),
        _authenticator->buildRequestAuthHeader(),
        ((AcceptLanguageListContainer)message->operationContext.get(
            AcceptLanguageListContainer::NAME)).getLanguages(),
        ((ContentLanguageListContainer)message->operationContext.get(
            ContentLanguageListContainer::NAME)).getLanguages(),
        params, _binaryResponse);

    _sendRequest(buffer);
}

void CIMOperationRequestEncoder::_encodeOpenReferenceInstancesRequest(
    CIMOpenReferenceInstancesRequestMessage* message)
{
    Buffer params;

    _appendObjectNameIParameter(
        params, "InstanceName", message->objectName);

    XmlWriter::appendClassNameIParameter(
        params, "ResultClass", message->resultClass);

    //
    //  The Client API has no way to represent a NULL role;
    //  An empty string is interpreted as NULL
    //
    if (message->role.size() != 0)
    {
        XmlWriter::appendStringIParameter(
            params, "Role", message->role);
    }

    if (message->includeClassOrigin != false)
        XmlWriter::appendBooleanIParameter(params, "IncludeClassOrigin", true);

    if (!message->propertyList.isNull())
        XmlWriter::appendPropertyListIParameter(
            params, message->propertyList);

    _encodeOpenCommonParameters(message->continueOnError,
        message->maxObjectCount,
        message->operationTimeout,
        message->filterQueryLanguage,
        message->filterQuery,
        params);

    Buffer buffer = XmlWriter::formatSimpleIMethodReqMessage(_hostName,
        message->nameSpace, CIMName ("OpenReferenceInstances"),
        message->messageId,
        message->getHttpMethod(),
        _authenticator->buildRequestAuthHeader(),
        ((AcceptLanguageListContainer)message->operationContext.get(
            AcceptLanguageListContainer::NAME)).getLanguages(),
        ((ContentLanguageListContainer)message->operationContext.get(
            ContentLanguageListContainer::NAME)).getLanguages(),
        params, _binaryResponse);

    _sendRequest(buffer);
}


void CIMOperationRequestEncoder::_encodeOpenReferenceInstancePathsRequest(
    CIMOpenReferenceInstancePathsRequestMessage* message)
{
    Buffer params;

    _appendObjectNameIParameter(
        params, "InstanceName", message->objectName);

    XmlWriter::appendClassNameIParameter(
        params, "ResultClass", message->resultClass);

    //
    //  The Client API has no way to represent a NULL role;
    //  An empty string is interpreted as NULL
    //
    if (message->role.size() != 0)
    {
        XmlWriter::appendStringIParameter(params, "Role", message->role);
    }

    _encodeOpenCommonParameters(message->continueOnError,
        message->maxObjectCount,
        message->operationTimeout,
        message->filterQueryLanguage,
        message->filterQuery,
        params);

    Buffer buffer = XmlWriter::formatSimpleIMethodReqMessage(_hostName,
        message->nameSpace, CIMName ("OpenReferenceInstancePaths"),
        message->messageId,
        message->getHttpMethod(),
        _authenticator->buildRequestAuthHeader(),
        ((AcceptLanguageListContainer)message->operationContext.get(
            AcceptLanguageListContainer::NAME)).getLanguages(),
        ((ContentLanguageListContainer)message->operationContext.get(
            ContentLanguageListContainer::NAME)).getLanguages(),
        params, _binaryResponse);

    _sendRequest(buffer);
}

void CIMOperationRequestEncoder::_encodeOpenAssociatorInstancesRequest(
    CIMOpenAssociatorInstancesRequestMessage* message)
{
    Buffer params;

    _appendObjectNameIParameter(
        params, "InstanceName", message->objectName);

    XmlWriter::appendClassNameIParameter(
        params, "AssocClass", message->assocClass);

    XmlWriter::appendClassNameIParameter(
        params, "ResultClass", message->resultClass);

    //
    //  The Client API has no way to represent a NULL role;
    //  An empty string is interpreted as NULL
    //
    if (message->role.size() != 0)
    {
        XmlWriter::appendStringIParameter(params, "Role", message->role);
    }
    //
    //  The Client API has no way to represent a NULL resultRole;
    //  An empty string is interpreted as NULL
    //
    if (message->resultRole.size() != 0)
    {
        XmlWriter::appendStringIParameter(
            params, "ResultRole", message->resultRole);
    }

    if (message->includeClassOrigin != false)
        XmlWriter::appendBooleanIParameter(params, "IncludeClassOrigin", true);

    if (!message->propertyList.isNull())
        XmlWriter::appendPropertyListIParameter(
            params, message->propertyList);

    _encodeOpenCommonParameters(message->continueOnError,
        message->maxObjectCount,
        message->operationTimeout,
        message->filterQueryLanguage,
        message->filterQuery,
        params);

    Buffer buffer = XmlWriter::formatSimpleIMethodReqMessage(_hostName,
        message->nameSpace, CIMName ("OpenAssociatorInstances"),
        message->messageId,
        message->getHttpMethod(),
        _authenticator->buildRequestAuthHeader(),
        ((AcceptLanguageListContainer)message->operationContext.get(
            AcceptLanguageListContainer::NAME)).getLanguages(),
        ((ContentLanguageListContainer)message->operationContext.get(
            ContentLanguageListContainer::NAME)).getLanguages(),
        params, _binaryResponse);

    _sendRequest(buffer);
}

void CIMOperationRequestEncoder::_encodeOpenAssociatorInstancePathsRequest(
    CIMOpenAssociatorInstancePathsRequestMessage* message)
{
    Buffer params;

    _appendObjectNameIParameter(
        params, "InstanceName", message->objectName);

    XmlWriter::appendClassNameIParameter(
        params, "ResultClass", message->resultClass);

    //
    //  The Client API has no way to represent a NULL role;
    //  An empty string is interpreted as NULL
    //
    if (message->role.size() != 0)
    {
        XmlWriter::appendStringIParameter(params, "Role", message->role);
    }

    _encodeOpenCommonParameters(message->continueOnError,
        message->maxObjectCount,
        message->operationTimeout,
        message->filterQueryLanguage,
        message->filterQuery,
        params);

    Buffer buffer = XmlWriter::formatSimpleIMethodReqMessage(_hostName,
        message->nameSpace, CIMName ("OpenAssociatorInstancePaths"),
        message->messageId,
        message->getHttpMethod(),
        _authenticator->buildRequestAuthHeader(),
        ((AcceptLanguageListContainer)message->operationContext.get(
            AcceptLanguageListContainer::NAME)).getLanguages(),
        ((ContentLanguageListContainer)message->operationContext.get(
            ContentLanguageListContainer::NAME)).getLanguages(),
        params, _binaryResponse);

    _sendRequest(buffer);
}
/*
    Encoder for input parameters common to all of the PullOperations
*/
void _encodeCommonPullOperationParameters(
    Uint32 maxObjectCount,
    const String enumerationContext,
    Buffer& params)
{
    // This is a required parameter.
    XmlWriter::appendUint32IParameter(
        params, "MaxObjectCount", maxObjectCount);

    XmlWriter::appendStringIParameter(
        params, "EnumerationContext", enumerationContext);
}


void CIMOperationRequestEncoder::_encodePullInstancesWithPathRequest(
    CIMPullInstancesWithPathRequestMessage* message)
{
    Buffer params;

    _encodeCommonPullOperationParameters(message->maxObjectCount,
        message->enumerationContext,
        params);

    Buffer buffer = XmlWriter::formatSimpleIMethodReqMessage(_hostName,
        message->nameSpace, CIMName ("PullInstancesWithPath"),
        message->messageId,
        message->getHttpMethod(),
        _authenticator->buildRequestAuthHeader(),
        ((AcceptLanguageListContainer)message->operationContext.get(
            AcceptLanguageListContainer::NAME)).getLanguages(),
        ((ContentLanguageListContainer)message->operationContext.get(
            ContentLanguageListContainer::NAME)).getLanguages(),
        params, _binaryResponse);

    _sendRequest(buffer);
}

void CIMOperationRequestEncoder::_encodePullInstancePathsRequest(
    CIMPullInstancePathsRequestMessage* message)
{
    Buffer params;

    _encodeCommonPullOperationParameters(message->maxObjectCount,
        message->enumerationContext,
        params);

    Buffer buffer = XmlWriter::formatSimpleIMethodReqMessage(_hostName,
        message->nameSpace, CIMName ("PullInstancePaths"), message->messageId,
        message->getHttpMethod(),
        _authenticator->buildRequestAuthHeader(),
        ((AcceptLanguageListContainer)message->operationContext.get(
            AcceptLanguageListContainer::NAME)).getLanguages(),
        ((ContentLanguageListContainer)message->operationContext.get(
            ContentLanguageListContainer::NAME)).getLanguages(),
        params, _binaryResponse);

    _sendRequest(buffer);
}

void CIMOperationRequestEncoder::_encodePullInstancesRequest(
    CIMPullInstancesRequestMessage* message)
{
    Buffer params;

    _encodeCommonPullOperationParameters(message->maxObjectCount,
        message->enumerationContext,
        params);

    Buffer buffer = XmlWriter::formatSimpleIMethodReqMessage(_hostName,
        message->nameSpace, CIMName ("PullInstances"),
        message->messageId,
        message->getHttpMethod(),
        _authenticator->buildRequestAuthHeader(),
        ((AcceptLanguageListContainer)message->operationContext.get(
            AcceptLanguageListContainer::NAME)).getLanguages(),
        ((ContentLanguageListContainer)message->operationContext.get(
            ContentLanguageListContainer::NAME)).getLanguages(),
        params, _binaryResponse);

    _sendRequest(buffer);
}void CIMOperationRequestEncoder::_encodeCloseEnumerationRequest(
    CIMCloseEnumerationRequestMessage* message)
{
    Buffer params;

    // EXP_PULL ISSUE what is default and case of empty msg here.
    // DMTF unclear if empty is legal.
    if (message->enumerationContext.size() != 0)
    {
        XmlWriter::appendStringIParameter(
            params, "EnumerationContext", message->enumerationContext);
    }

    Buffer buffer = XmlWriter::formatSimpleIMethodReqMessage(_hostName,
        message->nameSpace, CIMName ("CloseEnumeration"), message->messageId,
        message->getHttpMethod(),
        _authenticator->buildRequestAuthHeader(),
        ((AcceptLanguageListContainer)message->operationContext.get(
            AcceptLanguageListContainer::NAME)).getLanguages(),
        ((ContentLanguageListContainer)message->operationContext.get(
            ContentLanguageListContainer::NAME)).getLanguages(),
        params, _binaryResponse);

    _sendRequest(buffer);
}

void CIMOperationRequestEncoder::_encodeEnumerationCountRequest(
    CIMEnumerationCountRequestMessage* message)
{
    Buffer params;

    if (message->enumerationContext.size() != 0)
    {
        XmlWriter::appendStringIParameter(
            params, "EnumerationContext", message->enumerationContext);
    }

    Buffer buffer = XmlWriter::formatSimpleIMethodReqMessage(_hostName,
        message->nameSpace, CIMName ("EnumerationCount"), message->messageId,
        message->getHttpMethod(),
        _authenticator->buildRequestAuthHeader(),
        ((AcceptLanguageListContainer)message->operationContext.get(
            AcceptLanguageListContainer::NAME)).getLanguages(),
        ((ContentLanguageListContainer)message->operationContext.get(
            ContentLanguageListContainer::NAME)).getLanguages(),
        params, _binaryResponse);

    _sendRequest(buffer);
}

void CIMOperationRequestEncoder::_encodeOpenQueryInstancesRequest(
    CIMOpenQueryInstancesRequestMessage* message)
{
    Buffer params;

    if (message->returnQueryResultClass != true)
        XmlWriter::appendBooleanIParameter(
            params, "ReturnQueryResultClass", false);

    //// KS_TODO this treats filter Query and lang as optional
    //// here they are required. Should that matter here?
    _encodeOpenCommonParameters(
        message->continueOnError,
        message->maxObjectCount,
        message->operationTimeout,
        message->queryLanguage,
        message->query,
        params);

    Buffer buffer = XmlWriter::formatSimpleIMethodReqMessage(_hostName,
        message->nameSpace, CIMName ("OpenQueryInstances"),
        message->messageId,
        message->getHttpMethod(),
        _authenticator->buildRequestAuthHeader(),
        ((AcceptLanguageListContainer)message->operationContext.get(
            AcceptLanguageListContainer::NAME)).getLanguages(),
        ((ContentLanguageListContainer)message->operationContext.get(
            ContentLanguageListContainer::NAME)).getLanguages(),
        params, _binaryResponse);

    _sendRequest(buffer);
}
// EXP_PULL_END

//
// Enqueue the buffer to the ouptut queue with a conditional display.
// This function is only enabled if the Pegasus Client trace is enabled.
// Uses parameter to determine whether to send to console to log.
void CIMOperationRequestEncoder::_sendRequest(Buffer& buffer)
{
    if (ClientTrace::displayOutput(ClientTrace::TRACE_CON))
    {
        XmlWriter::indentedPrint(cout, buffer.getData());
        cout << endl;
    }
    if (ClientTrace::displayOutput(ClientTrace::TRACE_LOG))
    {
        Logger::put(
            Logger::STANDARD_LOG,
            "CimClient",
            Logger::INFORMATION,
            "CIMOperationRequestEncoder::SendRequest, XML content: $0",
            buffer.getData());
    }

    HTTPMessage * http_request = new HTTPMessage(buffer);

    // these variables are needed to call HTTPMessage::parse, all we need
    // is contentLength
    String startLine;
    Array<HTTPHeader> headers;
    Uint32 contentLength;

    http_request->parse(startLine, headers, contentLength);
    if (dataStore_prt)
    {
        dataStore_prt->setRequestSize(contentLength);
        dataStore_prt->setStartNetworkTime();
    }

    // Inject Cookie: header, if needed
    if (_authenticator)
    {
        String cookie = _authenticator->getCookie();
        if (cookie.size())
        {
            String hdr = COOKIE_HEADER;
            hdr.append(cookie);
            http_request->injectHeader(hdr);
        }
    }

    _outputQueue->enqueue(http_request);
}

PEGASUS_NAMESPACE_END
