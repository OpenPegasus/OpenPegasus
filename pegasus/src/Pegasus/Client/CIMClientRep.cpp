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

#include "CIMClientRep.h"

#include <Pegasus/Common/MessageLoader.h>
#include <Pegasus/Common/System.h>
#include <Pegasus/Common/LanguageParser.h>

#include <iostream>
#include <fstream>
#include <Pegasus/Common/Network.h>
#include <Pegasus/Common/Logger.h>

PEGASUS_USING_STD;

PEGASUS_NAMESPACE_BEGIN


///////////////////////////////////////////////////////////////////////////////
//
// CIMClientRep
//
///////////////////////////////////////////////////////////////////////////////

CIMClientRep::CIMClientRep(Uint32 timeoutMilliseconds)
    :
    MessageQueue(PEGASUS_QUEUENAME_CLIENT),
    _binaryResponse(false),
    _timeoutMilliseconds(timeoutMilliseconds),
    _connected(false),
    _doReconnect(false),
    _binaryRequest(false),
    _localConnect(false)
{
    //
    // Create Monitor and HTTPConnector
    //
    _monitor.reset(new Monitor());
    _httpConnector.reset(new HTTPConnector(_monitor.get()));

    requestAcceptLanguages.clear();
    requestContentLanguages.clear();
}

CIMClientRep::~CIMClientRep()
{
   disconnect();
}

void CIMClientRep::handleEnqueue()
{
}

void CIMClientRep::_connect(bool binaryRequest, bool binaryResponse)
{
    ClientTrace::setup();

    _authenticator.setHost(_connectHost);

    //
    // Create response decoder:
    //
    AutoPtr<CIMOperationResponseDecoder> responseDecoder(
        new CIMOperationResponseDecoder(
            this, _requestEncoder.get(), &_authenticator ));

    //
    // Attempt to establish a connection:
    //
    AutoPtr<HTTPConnection> httpConnection(_httpConnector->connect(
        _connectHost,
        _connectPortNumber,
        _connectSSLContext.get(),
        _timeoutMilliseconds,
        responseDecoder.get()));

    //
    // Create request encoder:
    //
    String connectHost = _connectHost;
    if (connectHost.size())
    {
        char portStr[32];
        sprintf(portStr, ":%u", _connectPortNumber);
        connectHost.append(portStr);
    }

    AutoPtr<CIMOperationRequestEncoder> requestEncoder(
        new CIMOperationRequestEncoder(
            httpConnection.get(), connectHost, &_authenticator,
            binaryRequest,
            binaryResponse));

    _responseDecoder.reset(responseDecoder.release());
    _httpConnection = httpConnection.release();
    _requestEncoder.reset(requestEncoder.release());
    _responseDecoder->setEncoderQueue(_requestEncoder.get());

    //pass encoder and decoder a pointer to CIMClientRep::perfDataStore
    _requestEncoder->setDataStorePointer(&perfDataStore);
    _responseDecoder->setDataStorePointer(&perfDataStore);

    _doReconnect = false;
    _connected = true;
    _binaryRequest = binaryRequest;
    _binaryResponse = binaryResponse;
    _httpConnection->setSocketWriteTimeout(_timeoutMilliseconds/1000+1);
}

void CIMClientRep::_disconnect(bool keepChallengeStatus)
{
    if (_connected)
    {
        //
        // destroy response decoder
        //
        _responseDecoder.reset();


        //
        // Close the connection
        //
        if (_httpConnector.get())
        {
            _httpConnector->disconnect(_httpConnection);
            _httpConnection = 0;
        }

        //
        // destroy request encoder
        //
        _requestEncoder.reset();

        _connected = false;
    }

    // Reconnect no longer applies
    _doReconnect = false;

    // Let go of the cached request message if we have one
    _authenticator.setRequestMessage(0);

    if (keepChallengeStatus == false)
    {
    // Reset the challenge status
    _authenticator.resetChallengeStatus();
}
}

void CIMClientRep::connect(
    const String& host,
    const Uint32 portNumber,
    const String& userName,
    const String& password)
{
    //
    // If already connected, bail out!
    //
    if (_connected)
        throw AlreadyConnectedException();

    //
    // If the host is empty and port is valid, set hostName to "localhost"
    // Otherwise, HTTPConnector will use the unix domain socket.
    //
    String hostName = host;
    if (!host.size() && (portNumber != 0))
    {
        hostName = "localhost";
    }

    //
    // Set authentication information
    //
    _authenticator.clear();

    if (userName.size())
    {
        _authenticator.setUserName(userName);
    }

    if (password.size())
    {
        _authenticator.setPassword(password);
    }

    _connectSSLContext.reset();
    _connectHost = hostName;
    _connectPortNumber = portNumber;
    _connect(_binaryRequest, _binaryResponse);
}


void CIMClientRep::connect(
    const String& host,
    const Uint32 portNumber,
    const SSLContext& sslContext,
    const String& userName,
    const String& password)
{
    //
    // If already connected, bail out!
    //
    if (_connected)
        throw AlreadyConnectedException();

    //
    // If the host is empty, set hostName to "localhost"
    //
    String hostName = host;
    if (host == String::EMPTY)
    {
        hostName = "localhost";
    }

    //
    // Set authentication information
    //
    _authenticator.clear();

    if (userName.size())
    {
        _authenticator.setUserName(userName);
    }

    if (password.size())
    {
        _authenticator.setPassword(password);
    }

    _connectHost = hostName;
    _connectPortNumber = portNumber;

    _connectSSLContext.reset(new SSLContext(sslContext));
    _connect(_binaryRequest, _binaryResponse);
}


void CIMClientRep::connectLocal()
{
#if defined(PEGASUS_ENABLE_PROTOCOL_BINARY)
    _connectLocal(true);
#else
    _connectLocal(false);
#endif
}

void CIMClientRep::connectLocalBinary()
{
    _connectLocal(true);
}

void CIMClientRep::_connectLocal(bool binary)
{
    bool binaryRequest = binary;
    bool binaryResponse = binary;

    //
    // If already connected, bail out!
    //
    if (_connected)
        throw AlreadyConnectedException();

    //
    // Set authentication type
    //
    _authenticator.clear();
    _authenticator.setAuthType(ClientAuthenticator::LOCAL);

    _localConnect=true;

#ifndef PEGASUS_DISABLE_LOCAL_DOMAIN_SOCKET
    _connectSSLContext.reset();
    _connectHost = String::EMPTY;
    _connectPortNumber = 0;
    _connect(binaryRequest, binaryResponse);
#else

    try
    {
        //
        // Look up the WBEM HTTP port number for the local system
        //
        _connectPortNumber = System::lookupPort (WBEM_HTTP_SERVICE_NAME,
            WBEM_DEFAULT_HTTP_PORT);

        //
        //  Assign host
        //
        _connectHost.assign(System::getHostName());

        _connectSSLContext.reset();

        _connect(binaryRequest, binaryResponse);
    }
    catch (const CannotConnectException &)
    {
        //
        // Look up the WBEM HTTPS port number for the local system
        //
        _connectPortNumber = System::lookupPort (WBEM_HTTPS_SERVICE_NAME,
            WBEM_DEFAULT_HTTPS_PORT);

        //
        //  Assign host
        //
        _connectHost.assign(System::getHostName());

        //
        // Create SSLContext
        //
        const char* pegasusHome = getenv("PEGASUS_HOME");

        String randFile;

# ifdef PEGASUS_SSL_RANDOMFILE
        randFile = FileSystem::getAbsolutePath(
            pegasusHome, PEGASUS_SSLCLIENT_RANDOMFILE);
# endif

        // May throw SSLException
        _connectSSLContext.reset(
            new SSLContext(String::EMPTY, NULL, randFile));

        _connect(binaryRequest, binaryResponse);
    }
#endif
}

void CIMClientRep::disconnect()
{
    _disconnect();
    _authenticator.clear();
    _connectSSLContext.reset();
    _localConnect=false;
}

Boolean CIMClientRep::isConnected() const throw()
{
    return _connected;
}

Boolean CIMClientRep::isLocalConnect() const throw()
{
    return _localConnect;
}

AcceptLanguageList CIMClientRep::getRequestAcceptLanguages() const
{
    return requestAcceptLanguages;
}

ContentLanguageList CIMClientRep::getRequestContentLanguages() const
{
    return requestContentLanguages;
}

ContentLanguageList CIMClientRep::getResponseContentLanguages() const
{
    return responseContentLanguages;
}

void CIMClientRep::setRequestAcceptLanguages(const AcceptLanguageList& langs)
{
    requestAcceptLanguages = langs;
}

void CIMClientRep::setRequestContentLanguages(const ContentLanguageList& langs)
{
    requestContentLanguages = langs;
}

void CIMClientRep::setRequestDefaultLanguages()
{
    requestAcceptLanguages = LanguageParser::getDefaultAcceptLanguages();
}

CIMClass CIMClientRep::getClass(
    const CIMNamespaceName& nameSpace,
    const CIMName& className,
    Boolean localOnly,
    Boolean includeQualifiers,
    Boolean includeClassOrigin,
    const CIMPropertyList& propertyList)
{
    AutoPtr<CIMRequestMessage> request(new CIMGetClassRequestMessage(
        String::EMPTY,
        nameSpace,
        className,
        localOnly,
        includeQualifiers,
        includeClassOrigin,
        propertyList,
        QueueIdStack()));

    Message* message = _doRequest(request, CIM_GET_CLASS_RESPONSE_MESSAGE);

    CIMGetClassResponseMessage* response =
        (CIMGetClassResponseMessage*)message;

    AutoPtr<CIMGetClassResponseMessage> destroyer(response);

    return response->cimClass;
}

CIMResponseData CIMClientRep::getInstance(
    const CIMNamespaceName& nameSpace,
    const CIMObjectPath& instanceName,
    Boolean localOnly,
    Boolean includeQualifiers,
    Boolean includeClassOrigin,
    const CIMPropertyList& propertyList)
{
    AutoPtr<CIMRequestMessage> request(new CIMGetInstanceRequestMessage(
        String::EMPTY,
        nameSpace,
        instanceName,
        includeQualifiers,
        includeClassOrigin,
        propertyList,
        QueueIdStack()));
    dynamic_cast<CIMGetInstanceRequestMessage*>(request.get())->localOnly =
        localOnly;

    Message* message = _doRequest(request, CIM_GET_INSTANCE_RESPONSE_MESSAGE);

    CIMGetInstanceResponseMessage* response =
        (CIMGetInstanceResponseMessage*)message;

    AutoPtr<CIMGetInstanceResponseMessage> destroyer(response);

    return response->getResponseData();
}

void CIMClientRep::deleteClass(
    const CIMNamespaceName& nameSpace,
    const CIMName& className)
{
    AutoPtr<CIMRequestMessage> request(new CIMDeleteClassRequestMessage(
        String::EMPTY,
        nameSpace,
        className,
        QueueIdStack()));

    Message* message = _doRequest(request, CIM_DELETE_CLASS_RESPONSE_MESSAGE);

    CIMDeleteClassResponseMessage* response =
        (CIMDeleteClassResponseMessage*)message;

    AutoPtr<CIMDeleteClassResponseMessage> destroyer(response);
}

void CIMClientRep::deleteInstance(
    const CIMNamespaceName& nameSpace,
    const CIMObjectPath& instanceName)
{
    AutoPtr<CIMRequestMessage> request(new CIMDeleteInstanceRequestMessage(
        String::EMPTY,
        nameSpace,
        instanceName,
        QueueIdStack()));

    Message* message =
        _doRequest(request, CIM_DELETE_INSTANCE_RESPONSE_MESSAGE);

    CIMDeleteInstanceResponseMessage* response =
        (CIMDeleteInstanceResponseMessage*)message;

    AutoPtr<CIMDeleteInstanceResponseMessage> destroyer(response);
}

void CIMClientRep::createClass(
    const CIMNamespaceName& nameSpace,
    const CIMClass& newClass)
{
    AutoPtr<CIMRequestMessage> request(new CIMCreateClassRequestMessage(
        String::EMPTY,
        nameSpace,
        newClass,
        QueueIdStack()));

    Message* message = _doRequest(request, CIM_CREATE_CLASS_RESPONSE_MESSAGE);

    CIMCreateClassResponseMessage* response =
        (CIMCreateClassResponseMessage*)message;

    AutoPtr<CIMCreateClassResponseMessage> destroyer(response);
}

CIMObjectPath CIMClientRep::createInstance(
    const CIMNamespaceName& nameSpace,
    const CIMInstance& newInstance)
{
    AutoPtr<CIMRequestMessage> request(new CIMCreateInstanceRequestMessage(
        String::EMPTY,
        nameSpace,
        newInstance,
        QueueIdStack()));

    Message* message =
        _doRequest(request, CIM_CREATE_INSTANCE_RESPONSE_MESSAGE);

    CIMCreateInstanceResponseMessage* response =
        (CIMCreateInstanceResponseMessage*)message;

    AutoPtr<CIMCreateInstanceResponseMessage> destroyer(response);

    return response->instanceName;
}

void CIMClientRep::modifyClass(
    const CIMNamespaceName& nameSpace,
    const CIMClass& modifiedClass)
{
    AutoPtr<CIMRequestMessage> request(new CIMModifyClassRequestMessage(
        String::EMPTY,
        nameSpace,
        modifiedClass,
        QueueIdStack()));

    Message* message = _doRequest(request, CIM_MODIFY_CLASS_RESPONSE_MESSAGE);

    CIMModifyClassResponseMessage* response =
        (CIMModifyClassResponseMessage*)message;

    AutoPtr<CIMModifyClassResponseMessage> destroyer(response);
}

void CIMClientRep::modifyInstance(
    const CIMNamespaceName& nameSpace,
    const CIMInstance& modifiedInstance,
    Boolean includeQualifiers,
    const CIMPropertyList& propertyList)
{
    AutoPtr<CIMRequestMessage> request(new CIMModifyInstanceRequestMessage(
        String::EMPTY,
        nameSpace,
        modifiedInstance,
        includeQualifiers,
        propertyList,
        QueueIdStack()));

    Message* message =
        _doRequest(request, CIM_MODIFY_INSTANCE_RESPONSE_MESSAGE);

    CIMModifyInstanceResponseMessage* response =
        (CIMModifyInstanceResponseMessage*)message;

    AutoPtr<CIMModifyInstanceResponseMessage> destroyer(response);
}

Array<CIMClass> CIMClientRep::enumerateClasses(
    const CIMNamespaceName& nameSpace,
    const CIMName& className,
    Boolean deepInheritance,
    Boolean localOnly,
    Boolean includeQualifiers,
    Boolean includeClassOrigin)
{
    AutoPtr<CIMRequestMessage> request(new CIMEnumerateClassesRequestMessage(
        String::EMPTY,
        nameSpace,
        className,
        deepInheritance,
        localOnly,
        includeQualifiers,
        includeClassOrigin,
        QueueIdStack()));

    Message* message =
        _doRequest(request, CIM_ENUMERATE_CLASSES_RESPONSE_MESSAGE);

    CIMEnumerateClassesResponseMessage* response =
        (CIMEnumerateClassesResponseMessage*)message;

    AutoPtr<CIMEnumerateClassesResponseMessage> destroyer(response);

    return response->cimClasses;
}

Array<CIMName> CIMClientRep::enumerateClassNames(
    const CIMNamespaceName& nameSpace,
    const CIMName& className,
    Boolean deepInheritance)
{
    AutoPtr<CIMRequestMessage> request(new CIMEnumerateClassNamesRequestMessage(
        String::EMPTY,
        nameSpace,
        className,
        deepInheritance,
        QueueIdStack()));

    Message* message =
        _doRequest(request, CIM_ENUMERATE_CLASS_NAMES_RESPONSE_MESSAGE);

    CIMEnumerateClassNamesResponseMessage* response =
        (CIMEnumerateClassNamesResponseMessage*)message;

    AutoPtr<CIMEnumerateClassNamesResponseMessage> destroyer(response);

    // Temporary code until internal structures use CIMName instead of String
    Array<CIMName> classNameArray;
    classNameArray.reserveCapacity(response->classNames.size());
    for (Uint32 i=0; i<response->classNames.size(); i++)
    {
        classNameArray.append(response->classNames[i]);
    }
    return classNameArray;
}

CIMResponseData CIMClientRep::enumerateInstances(
    const CIMNamespaceName& nameSpace,
    const CIMName& className,
    Boolean deepInheritance,
    Boolean localOnly,
    Boolean includeQualifiers,
    Boolean includeClassOrigin,
    const CIMPropertyList& propertyList)
{
    AutoPtr<CIMRequestMessage> request(new CIMEnumerateInstancesRequestMessage(
        String::EMPTY,
        nameSpace,
        className,
        deepInheritance,
        includeQualifiers,
        includeClassOrigin,
        propertyList,
        QueueIdStack()));
    dynamic_cast<CIMEnumerateInstancesRequestMessage*>(
        request.get())->localOnly = localOnly;

    Message* message =
        _doRequest(request, CIM_ENUMERATE_INSTANCES_RESPONSE_MESSAGE);

    CIMEnumerateInstancesResponseMessage* response =
        (CIMEnumerateInstancesResponseMessage*)message;

    AutoPtr<CIMEnumerateInstancesResponseMessage> destroyer(response);

    return response->getResponseData();
}

CIMResponseData CIMClientRep::enumerateInstanceNames(
    const CIMNamespaceName& nameSpace,
    const CIMName& className)
{
    AutoPtr<CIMRequestMessage> request(
        new CIMEnumerateInstanceNamesRequestMessage(
            String::EMPTY,
            nameSpace,
            className,
            QueueIdStack()));

    Message* message =
        _doRequest(request, CIM_ENUMERATE_INSTANCE_NAMES_RESPONSE_MESSAGE);

    CIMEnumerateInstanceNamesResponseMessage* response =
        (CIMEnumerateInstanceNamesResponseMessage*)message;

    AutoPtr<CIMEnumerateInstanceNamesResponseMessage> destroyer(response);

    return response->getResponseData();
}

CIMResponseData CIMClientRep::execQuery(
    const CIMNamespaceName& nameSpace,
    const String& queryLanguage,
    const String& query)
{
    AutoPtr<CIMRequestMessage> request(new CIMExecQueryRequestMessage(
        String::EMPTY,
        nameSpace,
        queryLanguage,
        query,
        QueueIdStack()));

    Message* message = _doRequest(request, CIM_EXEC_QUERY_RESPONSE_MESSAGE);

    CIMExecQueryResponseMessage* response =
        (CIMExecQueryResponseMessage*)message;

    AutoPtr<CIMExecQueryResponseMessage> destroyer(response);

    return response->getResponseData();
}

CIMResponseData CIMClientRep::associators(
    const CIMNamespaceName& nameSpace,
    const CIMObjectPath& objectName,
    const CIMName& assocClass,
    const CIMName& resultClass,
    const String& role,
    const String& resultRole,
    Boolean includeQualifiers,
    Boolean includeClassOrigin,
    const CIMPropertyList& propertyList)
{
    AutoPtr<CIMRequestMessage> request(new CIMAssociatorsRequestMessage(
        String::EMPTY,
        nameSpace,
        objectName,
        assocClass,
        resultClass,
        role,
        resultRole,
        includeQualifiers,
        includeClassOrigin,
        propertyList,
        QueueIdStack()));

    Message* message = _doRequest(request, CIM_ASSOCIATORS_RESPONSE_MESSAGE);

    CIMAssociatorsResponseMessage* response =
        (CIMAssociatorsResponseMessage*)message;

    AutoPtr<CIMAssociatorsResponseMessage> destroyer(response);

    return response->getResponseData();
}

CIMResponseData CIMClientRep::associatorNames(
    const CIMNamespaceName& nameSpace,
    const CIMObjectPath& objectName,
    const CIMName& assocClass,
    const CIMName& resultClass,
    const String& role,
    const String& resultRole)
{
    AutoPtr<CIMRequestMessage> request(new CIMAssociatorNamesRequestMessage(
        String::EMPTY,
        nameSpace,
        objectName,
        assocClass,
        resultClass,
        role,
        resultRole,
        QueueIdStack()));

    Message* message =
        _doRequest(request, CIM_ASSOCIATOR_NAMES_RESPONSE_MESSAGE);

    CIMAssociatorNamesResponseMessage* response =
        (CIMAssociatorNamesResponseMessage*)message;

    AutoPtr<CIMAssociatorNamesResponseMessage> destroyer(response);

    return response->getResponseData();
}

CIMResponseData CIMClientRep::references(
    const CIMNamespaceName& nameSpace,
    const CIMObjectPath& objectName,
    const CIMName& resultClass,
    const String& role,
    Boolean includeQualifiers,
    Boolean includeClassOrigin,
    const CIMPropertyList& propertyList)
{
    AutoPtr<CIMRequestMessage> request(new CIMReferencesRequestMessage(
        String::EMPTY,
        nameSpace,
        objectName,
        resultClass,
        role,
        includeQualifiers,
        includeClassOrigin,
        propertyList,
        QueueIdStack()));

    Message* message = _doRequest(request, CIM_REFERENCES_RESPONSE_MESSAGE);

    CIMReferencesResponseMessage* response =
        (CIMReferencesResponseMessage*)message;

    AutoPtr<CIMReferencesResponseMessage> destroyer(response);

    return response->getResponseData();
}

CIMResponseData CIMClientRep::referenceNames(
    const CIMNamespaceName& nameSpace,
    const CIMObjectPath& objectName,
    const CIMName& resultClass,
    const String& role)
{
    AutoPtr<CIMRequestMessage> request(new CIMReferenceNamesRequestMessage(
        String::EMPTY,
        nameSpace,
        objectName,
        resultClass,
        role,
        QueueIdStack()));

    Message* message =
        _doRequest(request, CIM_REFERENCE_NAMES_RESPONSE_MESSAGE);

    CIMReferenceNamesResponseMessage* response =
        (CIMReferenceNamesResponseMessage*)message;

    AutoPtr<CIMReferenceNamesResponseMessage> destroyer(response);

    return response->getResponseData();
}

CIMValue CIMClientRep::getProperty(
    const CIMNamespaceName& nameSpace,
    const CIMObjectPath& instanceName,
    const CIMName& propertyName)
{
    AutoPtr<CIMRequestMessage> request(new CIMGetPropertyRequestMessage(
        String::EMPTY,
        nameSpace,
        instanceName,
        propertyName,
        QueueIdStack()));

    Message* message = _doRequest(request, CIM_GET_PROPERTY_RESPONSE_MESSAGE);

    CIMGetPropertyResponseMessage* response =
        (CIMGetPropertyResponseMessage*)message;

    AutoPtr<CIMGetPropertyResponseMessage> destroyer(response);

    return response->value;
}

void CIMClientRep::setProperty(
    const CIMNamespaceName& nameSpace,
    const CIMObjectPath& instanceName,
    const CIMName& propertyName,
    const CIMValue& newValue)
{
    AutoPtr<CIMRequestMessage> request(new CIMSetPropertyRequestMessage(
        String::EMPTY,
        nameSpace,
        instanceName,
        propertyName,
        newValue,
        QueueIdStack()));

    Message* message = _doRequest(request, CIM_SET_PROPERTY_RESPONSE_MESSAGE);

    CIMSetPropertyResponseMessage* response =
        (CIMSetPropertyResponseMessage*)message;

    AutoPtr<CIMSetPropertyResponseMessage> destroyer(response);
}

CIMQualifierDecl CIMClientRep::getQualifier(
    const CIMNamespaceName& nameSpace,
    const CIMName& qualifierName)
{
    AutoPtr<CIMRequestMessage> request(new CIMGetQualifierRequestMessage(
        String::EMPTY,
        nameSpace,
        qualifierName,
        QueueIdStack()));

    Message* message = _doRequest(request, CIM_GET_QUALIFIER_RESPONSE_MESSAGE);

    CIMGetQualifierResponseMessage* response =
        (CIMGetQualifierResponseMessage*)message;

    AutoPtr<CIMGetQualifierResponseMessage> destroyer(response);

    return response->cimQualifierDecl;
}

void CIMClientRep::setQualifier(
    const CIMNamespaceName& nameSpace,
    const CIMQualifierDecl& qualifierDeclaration)
{
    AutoPtr<CIMRequestMessage> request(new CIMSetQualifierRequestMessage(
        String::EMPTY,
        nameSpace,
        qualifierDeclaration,
        QueueIdStack()));

    Message* message = _doRequest(request, CIM_SET_QUALIFIER_RESPONSE_MESSAGE);

    CIMSetQualifierResponseMessage* response =
        (CIMSetQualifierResponseMessage*)message;

    AutoPtr<CIMSetQualifierResponseMessage> destroyer(response);
}

void CIMClientRep::deleteQualifier(
    const CIMNamespaceName& nameSpace,
    const CIMName& qualifierName)
{
    AutoPtr<CIMRequestMessage> request(new CIMDeleteQualifierRequestMessage(
        String::EMPTY,
        nameSpace,
        qualifierName,
        QueueIdStack()));

    Message* message =
        _doRequest(request, CIM_DELETE_QUALIFIER_RESPONSE_MESSAGE);

    CIMDeleteQualifierResponseMessage* response =
        (CIMDeleteQualifierResponseMessage*)message;

    AutoPtr<CIMDeleteQualifierResponseMessage> destroyer(response);
}

Array<CIMQualifierDecl> CIMClientRep::enumerateQualifiers(
    const CIMNamespaceName& nameSpace)
{
    AutoPtr<CIMRequestMessage> request(new CIMEnumerateQualifiersRequestMessage(
        String::EMPTY,
        nameSpace,
        QueueIdStack()));

    Message* message =
        _doRequest(request, CIM_ENUMERATE_QUALIFIERS_RESPONSE_MESSAGE);

    CIMEnumerateQualifiersResponseMessage* response =
        (CIMEnumerateQualifiersResponseMessage*)message;

    AutoPtr<CIMEnumerateQualifiersResponseMessage> destroyer(response);

    return response->qualifierDeclarations;
}

CIMValue CIMClientRep::invokeMethod(
    const CIMNamespaceName& nameSpace,
    const CIMObjectPath& instanceName,
    const CIMName& methodName,
    const Array<CIMParamValue>& inParameters,
    Array<CIMParamValue>& outParameters)
{
    // ATTN-RK-P2-20020301: Does it make sense to have a nameSpace parameter
    // when the namespace should already be included in the instanceName?
    // ATTN-RK-P3-20020301: Do we need to make sure the caller didn't specify
    // a host name in the instanceName?

    // solved with PEP#139 Stage1 as other CIMOMs contained in the object path
    // will cause a TypeMisMatchException

    AutoPtr<CIMRequestMessage> request(new CIMInvokeMethodRequestMessage(
        String::EMPTY,
        nameSpace,
        instanceName,
        methodName,
        inParameters,
        QueueIdStack()));

    Message* message = _doRequest(request, CIM_INVOKE_METHOD_RESPONSE_MESSAGE);

    CIMInvokeMethodResponseMessage* response =
        (CIMInvokeMethodResponseMessage*)message;

    AutoPtr<CIMInvokeMethodResponseMessage> destroyer(response);

    outParameters = response->outParameters;

    return response->retValue;
}

// EXP_PULL_BEGIN
CIMResponseData CIMClientRep::openEnumerateInstances(
    CIMEnumerationContext& enumerationContext,
    Boolean& endOfSequence,
    const CIMNamespaceName& nameSpace,
    const CIMName& className,
    Boolean deepInheritance,
    Boolean includeClassOrigin,
    const CIMPropertyList& propertyList,
    const String& filterQueryLanguage,
    const String& filterQuery,
    const Uint32Arg& operationTimeout,
    Boolean continueOnError,
    Uint32 maxObjectCount)
{
    // Save requied information in enumerationContext
    enumerationContext.setNameSpace(nameSpace);

    // Create/send the request message
    AutoPtr<CIMRequestMessage> request(
        new CIMOpenEnumerateInstancesRequestMessage(
            String::EMPTY,                  // messageId_ param
            nameSpace,
            className,
            deepInheritance,
            includeClassOrigin,
            propertyList,
            filterQueryLanguage,
            filterQuery,
            operationTimeout,
            continueOnError,
            maxObjectCount,
            QueueIdStack()));

    Message* message =
        _doRequest(request, CIM_OPEN_ENUMERATE_INSTANCES_RESPONSE_MESSAGE);

    // When Response received, get response message, get output parameters
    // and return data
    CIMOpenEnumerateInstancesResponseMessage* response =
        (CIMOpenEnumerateInstancesResponseMessage*)message;

    AutoPtr<CIMOpenEnumerateInstancesResponseMessage> destroyer(response);

    // set endOfSequence and context return parameters
    endOfSequence = response->endOfSequence;
    enumerationContext.setContextString(response->enumerationContext);

    return response->getResponseData();
}


CIMResponseData CIMClientRep::openEnumerateInstancePaths(
    CIMEnumerationContext& enumerationContext,
    Boolean& endOfSequence,
    const CIMNamespaceName& nameSpace,
    const CIMName& className,
    const String& filterQueryLanguage,
    const String& filterQuery,
    const Uint32Arg& operationTimeout,
    Boolean continueOnError,
    Uint32 maxObjectCount)
{
    AutoPtr<CIMRequestMessage> request(
        new CIMOpenEnumerateInstancePathsRequestMessage(
            String::EMPTY,                  // messageId_ param
            nameSpace,
            className,
            filterQueryLanguage,
            filterQuery,
            operationTimeout,
            continueOnError,
            maxObjectCount,
            QueueIdStack()));

    enumerationContext.setNameSpace(nameSpace);
    Message* message =
        _doRequest(request, CIM_OPEN_ENUMERATE_INSTANCE_PATHS_RESPONSE_MESSAGE);

    CIMOpenEnumerateInstancePathsResponseMessage* response =
        (CIMOpenEnumerateInstancePathsResponseMessage*)message;

    AutoPtr<CIMOpenEnumerateInstancePathsResponseMessage> destroyer(response);

    // set paramters to be returned to caller
    endOfSequence = response->endOfSequence;
    enumerationContext.setContextString(response->enumerationContext);

    return response->getResponseData();
}

CIMResponseData  CIMClientRep::openReferenceInstances(
    CIMEnumerationContext& enumerationContext,
    Boolean& endOfSequence,
    const CIMNamespaceName& nameSpace,
    const CIMObjectPath& objectName,
    const CIMName& resultClass,
    const String& role,
    Boolean includeClassOrigin,
    const CIMPropertyList& propertyList,
    const String& filterQueryLanguage,
    const String& filterQuery,
    const Uint32Arg& operationTimeout,
    Boolean continueOnError,
    Uint32 maxObjectCount)
{
    AutoPtr<CIMRequestMessage> request(
        new CIMOpenReferenceInstancesRequestMessage(
            String::EMPTY,                  // messageId_ param
            nameSpace,
            objectName,
            resultClass,
            role,
            includeClassOrigin,
            propertyList,
            filterQueryLanguage,
            filterQuery,
            operationTimeout,
            continueOnError,
            maxObjectCount,
            QueueIdStack()));

    enumerationContext.setNameSpace(nameSpace);
    Message* message =
        _doRequest(request, CIM_OPEN_REFERENCE_INSTANCES_RESPONSE_MESSAGE);

    CIMOpenReferenceInstancesResponseMessage* response =
        (CIMOpenReferenceInstancesResponseMessage*)message;

    AutoPtr<CIMOpenReferenceInstancesResponseMessage> destroyer(response);

    // set paramters to be returned to caller
    endOfSequence = response->endOfSequence;
    enumerationContext.setContextString(response->enumerationContext);

    return response->getResponseData();
}


CIMResponseData CIMClientRep::openReferenceInstancePaths(
    CIMEnumerationContext& enumerationContext,
    Boolean& endOfSequence,
    const CIMNamespaceName& nameSpace,
    const CIMObjectPath& objectName,
    const CIMName& resultClass,
    const String& role,
    const String& filterQueryLanguage,
    const String& filterQuery,
    const Uint32Arg& operationTimeout,
    Boolean continueOnError,
    Uint32 maxObjectCount)
{
    AutoPtr<CIMRequestMessage> request(
        new CIMOpenReferenceInstancePathsRequestMessage(
            String::EMPTY,                  // messageId_ param
            nameSpace,
            objectName,
            resultClass,
            role,
            filterQueryLanguage,
            filterQuery,
            operationTimeout,
            continueOnError,
            maxObjectCount,
            QueueIdStack()));

    enumerationContext.setNameSpace(nameSpace);
    Message* message =
        _doRequest(request, CIM_OPEN_REFERENCE_INSTANCE_PATHS_RESPONSE_MESSAGE);

    CIMOpenReferenceInstancePathsResponseMessage* response =
        (CIMOpenReferenceInstancePathsResponseMessage*)message;

    AutoPtr<CIMOpenReferenceInstancePathsResponseMessage> destroyer(response);

    // set paramters to be returned to caller
    endOfSequence = response->endOfSequence;
    enumerationContext.setContextString(response->enumerationContext);

    return response->getResponseData();
}

CIMResponseData CIMClientRep::openAssociatorInstances(
    CIMEnumerationContext& enumerationContext,
    Boolean& endOfSequence,
    const CIMNamespaceName& nameSpace,
    const CIMObjectPath& objectName,
    const CIMName& assocClass,
    const CIMName& resultClass,
    const String& role,
    const String& resultRole,
    Boolean includeClassOrigin,
    const CIMPropertyList& propertyList,
    const String& filterQueryLanguage,
    const String& filterQuery,
    const Uint32Arg& operationTimeout,
    Boolean continueOnError,
    Uint32 maxObjectCount )
{
    AutoPtr<CIMRequestMessage> request(
        new CIMOpenAssociatorInstancesRequestMessage(
            String::EMPTY,                  // messageId_ param
            nameSpace,
            objectName,
            assocClass,
            resultClass,
            role,
            resultRole,
            includeClassOrigin,
            propertyList,
            filterQueryLanguage,
            filterQuery,
            operationTimeout,
            continueOnError,
            maxObjectCount,
            QueueIdStack()));

    enumerationContext.setNameSpace(nameSpace);

    Message* message =
        _doRequest(request, CIM_OPEN_ASSOCIATOR_INSTANCES_RESPONSE_MESSAGE);

    CIMOpenAssociatorInstancesResponseMessage* response =
        (CIMOpenAssociatorInstancesResponseMessage*)message;

    AutoPtr<CIMOpenAssociatorInstancesResponseMessage> destroyer(response);

    // set parameters to be returned to caller
    endOfSequence = response->endOfSequence;
    enumerationContext.setContextString(response->enumerationContext);

    return response->getResponseData();
}

CIMResponseData CIMClientRep::openAssociatorInstancePaths(
    CIMEnumerationContext& enumerationContext,
    Boolean& endOfSequence,
    const CIMNamespaceName& nameSpace,
    const CIMObjectPath& objectName,
    const CIMName& assocClass,
    const CIMName& resultClass,
    const String& role,
    const String& resultRole,
    const String& filterQueryLanguage,
    const String& filterQuery,
    const Uint32Arg& operationTimeout,
    Boolean continueOnError,
    Uint32 maxObjectCount)
{
    AutoPtr<CIMRequestMessage> request(
        new CIMOpenAssociatorInstancePathsRequestMessage(
            String::EMPTY,                  // messageId_ param
            nameSpace,
            objectName,
            assocClass,
            resultClass,
            role,
            resultRole,
            filterQueryLanguage,
            filterQuery,
            operationTimeout,
            continueOnError,
            maxObjectCount,
            QueueIdStack()));

    enumerationContext.setNameSpace(nameSpace);
    Message* message =
        _doRequest(request,
                   CIM_OPEN_ASSOCIATOR_INSTANCE_PATHS_RESPONSE_MESSAGE);

    CIMOpenAssociatorInstancePathsResponseMessage* response =
        (CIMOpenAssociatorInstancePathsResponseMessage*)message;

    AutoPtr<CIMOpenAssociatorInstancePathsResponseMessage> destroyer(response);

    // set paramters to be returned to caller
    endOfSequence = response->endOfSequence;
    enumerationContext.setContextString(response->enumerationContext);

    return response->getResponseData();
}

CIMResponseData CIMClientRep::pullInstancesWithPath(
    CIMEnumerationContext& enumerationContext,
    Boolean& endOfSequence,
    Uint32 maxObjectCount)
{
    AutoPtr<CIMRequestMessage> request(
        new CIMPullInstancesWithPathRequestMessage(
            String::EMPTY,                  // messageId_ param
            enumerationContext.getNameSpace(),
            enumerationContext.getContextString(),
            maxObjectCount,
            QueueIdStack() ));

        Message* message =
            _doRequest(request, CIM_PULL_INSTANCES_WITH_PATH_RESPONSE_MESSAGE);

        CIMPullInstancesWithPathResponseMessage* response =
            (CIMPullInstancesWithPathResponseMessage*)message;

        AutoPtr<CIMPullInstancesWithPathResponseMessage> destroyer(response);

        // set paramters to be returned to caller
        endOfSequence = response->endOfSequence;
        enumerationContext.setContextString(response->enumerationContext);

        return response->getResponseData();
}

CIMResponseData CIMClientRep::pullInstancePaths(
    CIMEnumerationContext& enumerationContext,
    Boolean& endOfSequence,
    Uint32 maxObjectCount)
{
    AutoPtr<CIMRequestMessage> request(
        new CIMPullInstancePathsRequestMessage(
            String::EMPTY,                  // messageId_ param
            enumerationContext.getNameSpace(),
            enumerationContext.getContextString(),
            maxObjectCount,
            QueueIdStack()
            ));

        Message* message =
            _doRequest(request, CIM_PULL_INSTANCE_PATHS_RESPONSE_MESSAGE);

        CIMPullInstancePathsResponseMessage* response =
            (CIMPullInstancePathsResponseMessage*)message;

        AutoPtr<CIMPullInstancePathsResponseMessage> destroyer(response);

        // set paramters to be returned to caller
        endOfSequence = response->endOfSequence;
        enumerationContext.setContextString(response->enumerationContext);

        return response->getResponseData();
}

CIMResponseData CIMClientRep::pullInstances(
    CIMEnumerationContext& enumerationContext,
    Boolean& endOfSequence,
    Uint32 maxObjectCount)
{
    AutoPtr<CIMRequestMessage> request(
        new CIMPullInstancesRequestMessage(
            String::EMPTY,                  // messageId_ param
            enumerationContext.getNameSpace(),
            enumerationContext.getContextString(),
            maxObjectCount,
            QueueIdStack() ));

        Message* message =
            _doRequest(request, CIM_PULL_INSTANCES_RESPONSE_MESSAGE);

        CIMPullInstancesResponseMessage* response =
            (CIMPullInstancesResponseMessage*)message;

        AutoPtr<CIMPullInstancesResponseMessage> destroyer(response);

        // set paramters to be returned to caller
        endOfSequence = response->endOfSequence;
        enumerationContext.setContextString(response->enumerationContext);

        return response->getResponseData();
}

void CIMClientRep::closeEnumeration(
    CIMEnumerationContext& enumerationContext)
{
    AutoPtr<CIMRequestMessage> request(
        new CIMCloseEnumerationRequestMessage(
            String::EMPTY,                  // messageId_ param
            enumerationContext.getNameSpace(),
            enumerationContext.getContextString(),
            QueueIdStack()
            ));

        Message* message =
            _doRequest(request, CIM_CLOSE_ENUMERATION_RESPONSE_MESSAGE);

        CIMCloseEnumerationResponseMessage* response =
            (CIMCloseEnumerationResponseMessage*)message;

        AutoPtr<CIMCloseEnumerationResponseMessage> destroyer(response);

        return;
}

Uint64Arg CIMClientRep::enumerationCount(
    CIMEnumerationContext& enumerationContext)
{
    if (enumerationContext.getContextString().size())
    {
        throw InvalidEnumerationContextException();
    }
    AutoPtr<CIMRequestMessage> request(
        new CIMEnumerationCountRequestMessage(
            String::EMPTY,                  // messageId_ param
            enumerationContext.getNameSpace(),
            enumerationContext.getContextString(),
            QueueIdStack()
            ));

        Message* message =
            _doRequest(request, CIM_ENUMERATION_COUNT_RESPONSE_MESSAGE);

        CIMEnumerationCountResponseMessage* response =
            (CIMEnumerationCountResponseMessage*)message;

        AutoPtr<CIMEnumerationCountResponseMessage> destroyer(response);

        return response->count ;
}

CIMResponseData CIMClientRep::openQueryInstances(
    CIMEnumerationContext& enumerationContext,
    Boolean& endOfSequence,
    const CIMNamespaceName& nameSpace,
    const String& queryLanguage,
    const String& query,
    CIMClass& queryResultClass,
    Boolean returnQueryResultClass,
    const Uint32Arg& operationTimeout,
    Boolean continueOnError,
    Uint32 maxObjectCount)
{
    // Save requied information in enumerationContext
    enumerationContext.setNameSpace(nameSpace);

    // Create/send the request message
    AutoPtr<CIMRequestMessage> request(
        new CIMOpenQueryInstancesRequestMessage(
            String::EMPTY,                  // messageId_ param
            nameSpace,
            queryLanguage,
            query,
            returnQueryResultClass,
            operationTimeout,
            continueOnError,
            maxObjectCount,
            QueueIdStack()));

    Message* message =
        _doRequest(request, CIM_OPEN_QUERY_INSTANCES_RESPONSE_MESSAGE);

    // When Response received, get response message, get output parameters
    // and return data
    CIMOpenQueryInstancesResponseMessage* response =
        (CIMOpenQueryInstancesResponseMessage*)message;

    AutoPtr<CIMOpenQueryInstancesResponseMessage> destroyer(response);

    // set endOfSequence and context return parameters
    endOfSequence = response->endOfSequence;
    enumerationContext.setContextString(response->enumerationContext);

    return response->getResponseData();
}
//EXP_PULL_END

Message* CIMClientRep::_doRequest(
    AutoPtr<CIMRequestMessage>& request,
    MessageType expectedResponseMessageType)
{
    if (!_connected && !_doReconnect)
    {
        throw NotConnectedException();
    }

    // Check if the connection has to be re-established
    if ( _connected && _httpConnection->needsReconnect() )
    {
        _disconnect();
        _doReconnect = true;
    }

    if (_doReconnect)
    {
        _connect(_binaryRequest, _binaryResponse);
        _doReconnect = false;
    }

    String messageId = XmlWriter::getNextMessageId();
    const_cast<String &>(request->messageId) = messageId;

    _authenticator.setRequestMessage(0);

    // ATTN-RK-P2-20020416: We should probably clear out the queue first.
    PEGASUS_ASSERT(getCount() == 0);  // Shouldn't be any messages in our queue

    //
    //  Set HTTP method in request to POST
    //
    //Bug 478/418 - Change this to do post call, not mpost
    request->setHttpMethod (HTTP_METHOD__POST);

    // Set the Accept-Languages and Content-Languages into
    // the request message

    request->operationContext.set(
        AcceptLanguageListContainer(requestAcceptLanguages));
    request->operationContext.set(
        ContentLanguageListContainer(requestContentLanguages));


    //gathering statistical information about client operation
    perfDataStore.reset();
    perfDataStore.setOperationType(request->getType());
    perfDataStore.setMessageID(request->messageId);

    // Sending a new request, so clear out the response Content-Languages
    responseContentLanguages.clear();

    _requestEncoder->enqueue(request.get());
    request.release();

    Uint64 startMilliseconds = TimeValue::getCurrentTime().toMilliseconds();
    Uint64 nowMilliseconds = startMilliseconds;
#ifdef PEGASUS_DISABLE_CLIENT_TIMEOUT
    Uint64 stopMilliseconds = (Uint64) -1;
#else
    Uint64 stopMilliseconds = nowMilliseconds + _timeoutMilliseconds;
#endif

    while (nowMilliseconds < stopMilliseconds)
    {
        //
        // Wait until the timeout expires or an event occurs:
        //
        _monitor->run(Uint32(stopMilliseconds - nowMilliseconds));

        //
        // Check to see if incoming queue has a message
        //

        AutoPtr<Message> response(dequeue());

        if (response.get())
        {
            // Shouldn't be any more messages in our queue
            PEGASUS_ASSERT(getCount() == 0);

            //
            // Close the connection if response contained a "Connection: Close"
            // header (e.g. at authentication challenge)
            //
            if (response->getCloseConnect() == true)
            {
                _disconnect(true);
                _doReconnect = true;
                response->setCloseConnect(false);
            }

            //
            //  Future:  If M-POST is used and HTTP response is 501 Not
            //  Implemented or 510 Not Extended, retry with POST method
            //

            if (response->getType() == CLIENT_EXCEPTION_MESSAGE)
            {
                Exception* clientException =
                    ((ClientExceptionMessage*)response.get())->clientException;

                AutoPtr<Exception> d(clientException);

                // Make the ContentLanguage of the exception available through
                // the CIMClient API (its also available in the exception).
                responseContentLanguages =
                    clientException->getContentLanguages();

                //
                // Determine and throw the specific class of client exception
                //

                CIMClientMalformedHTTPException* malformedHTTPException =
                    dynamic_cast<CIMClientMalformedHTTPException*>(
                        clientException);
                if (malformedHTTPException)
                {
                    throw *malformedHTTPException;
                }

                CIMClientHTTPErrorException* httpErrorException =
                    dynamic_cast<CIMClientHTTPErrorException*>(
                        clientException);
                if (httpErrorException)
                {
                    throw *httpErrorException;
                }

                CIMClientXmlException* xmlException =
                    dynamic_cast<CIMClientXmlException*>(clientException);
                if (xmlException)
                {
                    throw *xmlException;
                }

                CIMClientResponseException* responseException =
                    dynamic_cast<CIMClientResponseException*>(clientException);
                if (responseException)
                {
                    throw *responseException;
                }

                CIMException* cimException =
                    dynamic_cast<CIMException*>(clientException);
                if (cimException)
                {
                    throw *cimException;
                }

                throw *clientException;
            }
            else if (response->getType() == expectedResponseMessageType)
            {
                CIMResponseMessage* cimResponse =
                    (CIMResponseMessage*)response.get();

                if (cimResponse->messageId != messageId)
                {
                    MessageLoaderParms mlParms(
                        "Client.CIMClient.MISMATCHED_RESPONSE",
                        "Mismatched response message ID:  Got \"$0\", "
                            "expected \"$1\".",
                        cimResponse->messageId, messageId);
                    String mlString(MessageLoader::getMessage(mlParms));

                    CIMClientResponseException responseException(mlString);

                    throw responseException;
                }

                // Get the Content-Languages from the response's
                // operationContext and make available through the
                // CIMClient API
                responseContentLanguages = ((ContentLanguageListContainer)
                    cimResponse->operationContext.get(
                        ContentLanguageListContainer::NAME)).getLanguages();

                if (cimResponse->cimException.getCode() != CIM_ERR_SUCCESS)
                {
                    CIMException cimException(cimResponse->cimException);

                    cimException.setContentLanguages(responseContentLanguages);
                    throw cimException;
                }

                // if execution gets here everytihng is working correctly
                // and a proper response was generated and received.

                // Check that client side statistics are valid before
                // handing them to the client application via a call back
                Boolean re_check = perfDataStore.checkMessageIDandType(
                    cimResponse->messageId,
                    cimResponse->getType());

                if (re_check &&
                    !perfDataStore.getStatError() &&
                    perfDataStore.isClassRegistered())
                {
                   // If callback method throws an exception it will be seen
                   // by the client no try/catch block is used here
                   // intentionaly - because exceptions come from the client
                   // application so client app. should handle them
                   ClientOpPerformanceData item =
                       perfDataStore.createPerfDataStruct();
                   perfDataStore.handler_prt->handleClientOpPerformanceData(
                       item);

                }//end of if statmet that call the callback method
                return response.release();
            }
            else if (dynamic_cast<CIMRequestMessage*>(response.get()) != 0)
            {
                //
                // Respond to an authentication challenge.
                // Reconnect if the connection was closed.
                //
                if (_doReconnect)
                {
                    _connect(_binaryRequest, _binaryResponse);
                }

                _requestEncoder->enqueue(response.release());
                nowMilliseconds = TimeValue::getCurrentTime().toMilliseconds();
                stopMilliseconds = nowMilliseconds + _timeoutMilliseconds;
                continue;
            }
            else
            {
                MessageLoaderParms mlParms(
                    "Client.CIMOperationResponseDecoder."
                        "MISMATCHED_RESPONSE_TYPE",
                    "Mismatched response message type.");
                String mlString(MessageLoader::getMessage(mlParms));

                CIMClientResponseException responseException(mlString);

                throw responseException;
            }
        }

        nowMilliseconds = TimeValue::getCurrentTime().toMilliseconds();
    }

    //
    // Reconnect to reset the connection (disregard late response)
    //

    _disconnect();
    _doReconnect = true;

    //
    // Throw timed out exception:
    //
    throw ConnectionTimeoutException();
}

void CIMClientRep::registerClientOpPerformanceDataHandler(
    ClientOpPerformanceDataHandler& handler)
{
    perfDataStore.handler_prt = &handler;
    perfDataStore.setClassRegistered(true);
}

void CIMClientRep::deregisterClientOpPerformanceDataHandler()
{
    perfDataStore.handler_prt = NULL;
    perfDataStore.setClassRegistered(false);
}


/*
    Implementation of the Trace mechanism
*/

// static variables to store the display state for input and output.
Uint32 ClientTrace::inputState;
Uint32 ClientTrace::outputState;

ClientTrace::TraceType ClientTrace::selectType(const String& str)
{
    if (str == "con")
    {
        return TRACE_CON;
    }
    if (str == "log")
    {
        return TRACE_LOG;
    }
    if (str == "both")
    {
        return TRACE_BOTH;
    }
    return TRACE_NONE;
}

Boolean ClientTrace::displayOutput(TraceType tt)
{
    return (tt & outputState);
}

Boolean ClientTrace::displayInput(TraceType tt)
{
    return (tt & inputState);
}

// Set up the input and output state variables from the input
// environment variable.
void ClientTrace::setup()
{
    String input;
    if (char * envVar = getenv("PEGASUS_CLIENT_TRACE"))
    {
        input = envVar;
        input.toLower();
        String in;
        String out;
        Uint32 pos = input.find(':');

        // if no colon found, input and output have same mask
        if (pos == PEG_NOT_FOUND)
        {
            in = input;
            out = input;
        }
        else
        {
            // if string starts with colon, input empty, else
            // either both or output empty
            if (input[0] == ':')
            {
                in = "";
                out = input.subString(1);
            }
            else
            {
                in = input.subString(0,pos);
                if (pos == (input.size() - 1))
                {
                    out = "";
                }
                else
                {
                    out =input.subString(pos + 1);
                }
            }
        }

        // set the state variables
        outputState = ClientTrace::selectType(out);
        inputState = ClientTrace::selectType(in);

        // Test for logging requested and if so set log parameters
        if (((outputState| inputState) & TRACE_LOG) != 0)
        {
            Logger::setlogLevelMask("");
        }
    }
}
PEGASUS_NAMESPACE_END
