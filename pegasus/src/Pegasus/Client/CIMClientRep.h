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

#ifndef Pegasus_ClientRep_h
#define Pegasus_ClientRep_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/String.h>
#include <Pegasus/Common/CIMName.h>
#include <Pegasus/Common/SSLContext.h>
#include <Pegasus/Common/CIMObject.h>
#include <Pegasus/Common/CIMClass.h>
#include <Pegasus/Common/CIMInstance.h>
#include <Pegasus/Common/CIMObjectPath.h>
#include <Pegasus/Common/CIMValue.h>
#include <Pegasus/Common/CIMParamValue.h>
#include <Pegasus/Common/CIMPropertyList.h>
#include <Pegasus/Common/CIMQualifierDecl.h>
#include <Pegasus/Common/Exception.h>
#include <Pegasus/Common/Constants.h>
#include <Pegasus/Common/FileSystem.h>
#include <Pegasus/Common/Monitor.h>
#include <Pegasus/Common/HTTPConnector.h>
#include <Pegasus/Common/CIMMessage.h>
#include <Pegasus/Common/HTTPConnection.h>
#include <Pegasus/Common/XmlWriter.h>
#include <Pegasus/Common/TimeValue.h>
#include <Pegasus/Common/AutoPtr.h>
#include <Pegasus/Client/CIMClientException.h>
#include <Pegasus/Client/Linkage.h>
#include <Pegasus/Client/CIMClientInterface.h>
#include <Pegasus/Client/ClientPerfDataStore.h>

#include "CIMOperationResponseDecoder.h"
#include "CIMOperationRequestEncoder.h"


PEGASUS_NAMESPACE_BEGIN

///////////////////////////////////////////////////////////////////////////////
//
// CIMClientRep
//
///////////////////////////////////////////////////////////////////////////////
class PEGASUS_CLIENT_LINKAGE CIMClientRep : public CIMClientInterface
{
public:

    // Timeout value defines the time the CIMClient will wait for a response
    // to an outstanding request.  If a request times out, the connection
    // gets reset (disconnected and reconnected).
    CIMClientRep(Uint32 timeoutMilliseconds =
                 PEGASUS_DEFAULT_CLIENT_TIMEOUT_MILLISECONDS);

    ~CIMClientRep();

    virtual void handleEnqueue();

    virtual Uint32 getTimeout() const
    {
        return _timeoutMilliseconds;
    }

    virtual void setTimeout(Uint32 timeoutMilliseconds)
    {
        _timeoutMilliseconds = timeoutMilliseconds;
        if ((_connected) && (_httpConnection != 0))
           _httpConnection->setSocketWriteTimeout(_timeoutMilliseconds/1000+1);
    }

    AcceptLanguageList getRequestAcceptLanguages() const;
    ContentLanguageList getRequestContentLanguages() const;
    ContentLanguageList getResponseContentLanguages() const;
    void setRequestAcceptLanguages(const AcceptLanguageList& langs);
    void setRequestContentLanguages(const ContentLanguageList& langs);
    void setRequestDefaultLanguages();

    void connect(
        const String& host,
        const Uint32 portNumber,
        const String& userName,
        const String& password
    );

    void connect(
        const String& host,
        const Uint32 portNumber,
        const SSLContext& sslContext,
        const String& userName,
        const String& password
    );

    void connectLocal();

    void disconnect();

    Boolean isConnected() const throw();

    Boolean isLocalConnect() const throw();

    virtual CIMClass getClass(
        const CIMNamespaceName& nameSpace,
        const CIMName& className,
        Boolean localOnly = true,
        Boolean includeQualifiers = true,
        Boolean includeClassOrigin = false,
        const CIMPropertyList& propertyList = CIMPropertyList()
    );

    virtual CIMResponseData getInstance(
        const CIMNamespaceName& nameSpace,
        const CIMObjectPath& instanceName,
        Boolean localOnly = true,
        Boolean includeQualifiers = false,
        Boolean includeClassOrigin = false,
        const CIMPropertyList& propertyList = CIMPropertyList()
    );

    virtual void deleteClass(
        const CIMNamespaceName& nameSpace,
        const CIMName& className
    );

    virtual void deleteInstance(
        const CIMNamespaceName& nameSpace,
        const CIMObjectPath& instanceName
    );

    virtual void createClass(
        const CIMNamespaceName& nameSpace,
        const CIMClass& newClass
    );

    virtual CIMObjectPath createInstance(
        const CIMNamespaceName& nameSpace,
        const CIMInstance& newInstance
    );

    virtual void modifyClass(
        const CIMNamespaceName& nameSpace,
        const CIMClass& modifiedClass
    );

    virtual void modifyInstance(
        const CIMNamespaceName& nameSpace,
        const CIMInstance& modifiedInstance,
        Boolean includeQualifiers = true,
        const CIMPropertyList& propertyList = CIMPropertyList()
    );

    virtual Array<CIMClass> enumerateClasses(
        const CIMNamespaceName& nameSpace,
        const CIMName& className = CIMName(),
        Boolean deepInheritance = false,
        Boolean localOnly = true,
        Boolean includeQualifiers = true,
        Boolean includeClassOrigin = false
    );

    virtual Array<CIMName> enumerateClassNames(
        const CIMNamespaceName& nameSpace,
        const CIMName& className = CIMName(),
        Boolean deepInheritance = false
    );

    virtual CIMResponseData enumerateInstances(
        const CIMNamespaceName& nameSpace,
        const CIMName& className,
        Boolean deepInheritance = true,
        Boolean localOnly = true,
        Boolean includeQualifiers = false,
        Boolean includeClassOrigin = false,
        const CIMPropertyList& propertyList = CIMPropertyList()
    );

    virtual CIMResponseData enumerateInstanceNames(
        const CIMNamespaceName& nameSpace,
        const CIMName& className
    );

    virtual CIMResponseData execQuery(
        const CIMNamespaceName& nameSpace,
        const String& queryLanguage,
        const String& query
    );

    virtual CIMResponseData associators(
        const CIMNamespaceName& nameSpace,
        const CIMObjectPath& objectName,
        const CIMName& assocClass = CIMName(),
        const CIMName& resultClass = CIMName(),
        const String& role = String::EMPTY,
        const String& resultRole = String::EMPTY,
        Boolean includeQualifiers = false,
        Boolean includeClassOrigin = false,
        const CIMPropertyList& propertyList = CIMPropertyList()
    );

    virtual CIMResponseData associatorNames(
        const CIMNamespaceName& nameSpace,
        const CIMObjectPath& objectName,
        const CIMName& assocClass = CIMName(),
        const CIMName& resultClass = CIMName(),
        const String& role = String::EMPTY,
        const String& resultRole = String::EMPTY
    );

    virtual CIMResponseData references(
        const CIMNamespaceName& nameSpace,
        const CIMObjectPath& objectName,
        const CIMName& resultClass = CIMName(),
        const String& role = String::EMPTY,
        Boolean includeQualifiers = false,
        Boolean includeClassOrigin = false,
        const CIMPropertyList& propertyList = CIMPropertyList()
    );

    virtual CIMResponseData referenceNames(
        const CIMNamespaceName& nameSpace,
        const CIMObjectPath& objectName,
        const CIMName& resultClass = CIMName(),
        const String& role = String::EMPTY
    );

    virtual CIMValue getProperty(
        const CIMNamespaceName& nameSpace,
        const CIMObjectPath& instanceName,
        const CIMName& propertyName
    );

    virtual void setProperty(
        const CIMNamespaceName& nameSpace,
        const CIMObjectPath& instanceName,
        const CIMName& propertyName,
        const CIMValue& newValue = CIMValue()
    );

    virtual CIMQualifierDecl getQualifier(
        const CIMNamespaceName& nameSpace,
        const CIMName& qualifierName
    );

    virtual void setQualifier(
        const CIMNamespaceName& nameSpace,
        const CIMQualifierDecl& qualifierDeclaration
    );

    virtual void deleteQualifier(
        const CIMNamespaceName& nameSpace,
        const CIMName& qualifierName
    );

    virtual Array<CIMQualifierDecl> enumerateQualifiers(
        const CIMNamespaceName& nameSpace
    );

    virtual CIMValue invokeMethod(
        const CIMNamespaceName& nameSpace,
        const CIMObjectPath& instanceName,
        const CIMName& methodName,
        const Array<CIMParamValue>& inParameters,
        Array<CIMParamValue>& outParameters
    );

// EXP_PULL_BEGIN
    virtual CIMResponseData openEnumerateInstances(
        CIMEnumerationContext& enumerationContext,
        Boolean& endOfSequence,
        const CIMNamespaceName& nameSpace,
        const CIMName& className,
        Boolean deepInheritance = false,
        Boolean includeClassOrigin = false,
        const CIMPropertyList& propertyList = CIMPropertyList(),
        const String& filterQueryLanguage = String::EMPTY,
        const String& filterQuery = String::EMPTY,
        const Uint32Arg& operationTimeout = Uint32Arg(),
        Boolean continueOnError = false,
        Uint32 maxObjectCount = 0
    );

    virtual CIMResponseData openEnumerateInstancePaths(
        CIMEnumerationContext& enumerationContext,
        Boolean& endOfSequence,
        const CIMNamespaceName& nameSpace,
        const CIMName& className,
        const String& filterQueryLanguage = String(),
        const String& filterQuery = String(),
        const Uint32Arg& operationTimeout = Uint32Arg(),
        Boolean continueOnError = false,
        Uint32 maxObjectCount = 0
    );

    virtual CIMResponseData openReferenceInstances(
        CIMEnumerationContext& enumerationContext,
        Boolean& endOfSequence,
        const CIMNamespaceName& nameSpace,
        const CIMObjectPath& objectName,
        const CIMName& resultClass = CIMName(),
        const String& role = String::EMPTY,
        Boolean includeClassOrigin = false,
        const CIMPropertyList& propertyList = CIMPropertyList(),
        const String& filterQueryLanguage = String(),
        const String& filterQuery = String(),
        const Uint32Arg& operationTimeout = Uint32Arg(),
        Boolean continueOnError = false,
        Uint32 maxObjectCount = 0
    );

    virtual CIMResponseData openReferenceInstancePaths(
        CIMEnumerationContext& enumerationContext,
        Boolean& endOfSequence,
        const CIMNamespaceName& nameSpace,
        const CIMObjectPath& objectName,
        const CIMName& resultClass = CIMName(),
        const String& role = String::EMPTY,
        const String& filterQueryLanguage = String(),
        const String& filterQuery = String(),
        const Uint32Arg& operationTimeout = Uint32Arg(),
        Boolean continueOnError = false,
        Uint32 maxObjectCount = 0
    );

    virtual CIMResponseData openAssociatorInstances(
        CIMEnumerationContext& enumerationContext,
        Boolean& endOfSequence,
        const CIMNamespaceName& nameSpace,
        const CIMObjectPath& objectName,
        const CIMName& assocClass = CIMName(),
        const CIMName& resultClass = CIMName(),
        const String& role = String::EMPTY,
        const String& resultRole = String::EMPTY,
        Boolean includeClassOrigin = false,
        const CIMPropertyList& propertyList = CIMPropertyList(),
        const String& filterQueryLanguage = String(),
        const String& filterQuery = String(),
        const Uint32Arg& operationTimeout = Uint32Arg(),
        Boolean continueOnError = false,
        Uint32 maxObjectCount = 0
    );

    virtual CIMResponseData openAssociatorInstancePaths(
        CIMEnumerationContext& enumerationContext,
        Boolean& endOfSequence,
        const CIMNamespaceName& nameSpace,
        const CIMObjectPath& objectName,
        const CIMName& assocClass = CIMName(),
        const CIMName& resultClass = CIMName(),
        const String& role = String::EMPTY,
        const String& resultRole = String::EMPTY,
        const String& filterQueryLanguage = String(),
        const String& filterQuery = String(),
        const Uint32Arg& operationTimeout = Uint32Arg(),
        Boolean continueOnError = false,
        Uint32 maxObjectCount = 0
    );

    virtual CIMResponseData pullInstancesWithPath(
        CIMEnumerationContext& enumerationContext,
        Boolean& endOfSequence,
        Uint32 maxObjectCount
    );

    virtual CIMResponseData pullInstancePaths(
        CIMEnumerationContext& enumerationContext,
        Boolean& endOfSequence,
        Uint32 maxObjectCount
    );

    virtual CIMResponseData pullInstances(
        CIMEnumerationContext& enumerationContext,
        Boolean& endOfSequence,
        Uint32 maxObjectCount
    );

    virtual void closeEnumeration(
        CIMEnumerationContext& enumerationContext
    );

    virtual Uint64Arg enumerationCount(
        CIMEnumerationContext& enumerationContext
    );

    virtual CIMResponseData openQueryInstances(
        CIMEnumerationContext& enumerationContext,
        Boolean& endOfSequence,
        const CIMNamespaceName& nameSpace,
        const String& queryLanguage,
        const String& query,
        CIMClass& queryResultClass,
        Boolean returnQueryResultClass = false,
        const Uint32Arg& operationTimeout = Uint32Arg(),
        Boolean continueOnError = false,
        Uint32 maxObjectCount = 0
    );
 //EXP_PULL_END

    void registerClientOpPerformanceDataHandler(
        ClientOpPerformanceDataHandler & handler);

    void deregisterClientOpPerformanceDataHandler();

    void setBinaryResponse(bool x) { _binaryResponse = x; }

    void setBinaryRequest(bool x) { _binaryRequest = x; }

    void connectLocalBinary();

    bool _binaryResponse;

private:

    void _connect(bool binaryRequest, bool binaryResponse);
    void _disconnect(bool keepChallengeStatus = false);
    void _connectLocal(bool binary);

    Message* _doRequest(
        AutoPtr<CIMRequestMessage>& request,
        MessageType expectedResponseMessageType);

    AutoPtr<Monitor> _monitor;
    AutoPtr<HTTPConnector> _httpConnector;
    HTTPConnection* _httpConnection;

    Uint32 _timeoutMilliseconds;
    Boolean _connected;
    /**
        The CIMExportClient uses a lazy reconnect algorithm.  A reconnection
        is necessary when the server (listener) sends a Connection: Close
        header in the HTTP response or when a connection timeout occurs
        while waiting for a response.  In these cases, a disconnect is
        performed immediately and the _doReconnect flag is set.  The
        connection is re-established only when required to perform another
        operation.  Note that in the case of a connection timeout, the
        challenge status must be reset in the ClientAuthenticator to allow
        authentication to be performed properly on the new connection.
    */
    Boolean _doReconnect;

    AutoPtr<CIMOperationResponseDecoder> _responseDecoder;
    AutoPtr<CIMOperationRequestEncoder> _requestEncoder;
    ClientAuthenticator _authenticator;
    String _connectHost;
    Uint32 _connectPortNumber;
    AutoPtr<SSLContext> _connectSSLContext;
    ClientPerfDataStore perfDataStore;

    AcceptLanguageList requestAcceptLanguages;
    ContentLanguageList requestContentLanguages;
    ContentLanguageList responseContentLanguages;
    bool _binaryRequest;
    bool _localConnect;
};

/****************************************************************************
**
**   Implementation of ClientTrace class.  This allows setup of variables
**   to control display of Client network send and receive.
**
****************************************************************************/
// Tests for Display optons of the form:
// Env variable PEGASUS_CLIENT_TRACE= <intrace> : <outtrace
// intrace = "con" | "log" | "both"
// outtrace = intrace
// ex set PEGASUS_CLIENT_TRACE=BOTH:BOTH traces input and output
// to console and log
// Keywords are case insensitive.
// PEP 90
// options allowed are:
//     keyword:keyword  separately define input and output
//     keyword:         Input only
//     :keyword         Output Only
//     keyword          Input and output defined by keyword
//
class ClientTrace
{
public:
    // Bit flags, that define what is to be displayed.
    enum TraceType
    {
        TRACE_NONE = 0,
        TRACE_CON = 1,
        TRACE_LOG = 2,
        TRACE_BOTH = 3
    };

    // setup the control variables from env variable
    static void setup();

    // Called from OperationRequest and Response handlers to test for
    // particular masks set.  Return true if the TraceType mask defined by
    // tt is set in the state variable.
    static Boolean displayOutput(TraceType tt);
    static Boolean displayInput(TraceType tt);

private:
    // constructors, etc. are private and not to be used.
    ClientTrace();
    ClientTrace(ClientTrace const&);
    ClientTrace& operator=(ClientTrace const&);

    // internal function to translate input strings to TraceTypes
    static TraceType selectType(const String& str);

    // Define the display states set by setup.
    static Uint32 inputState;
    static Uint32 outputState;
};

PEGASUS_NAMESPACE_END

#endif /* Pegasus_ClientRep_h */
