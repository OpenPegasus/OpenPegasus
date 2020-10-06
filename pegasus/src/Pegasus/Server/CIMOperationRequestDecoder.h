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

#ifndef Pegasus_CIMOperationRequestDecoder_h
#define Pegasus_CIMOperationRequestDecoder_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/MessageQueue.h>
#include <Pegasus/Common/CIMMessage.h>
#include <Pegasus/Common/HTTPMessage.h>
#include <Pegasus/Common/AcceptLanguageList.h>
#include <Pegasus/Common/ContentLanguageList.h>
#include <Pegasus/Common/AutoPtr.h>

PEGASUS_NAMESPACE_BEGIN

class XmlParser;

/** This class decodes CIM operation requests and passes them down-stream.
 */
class CIMOperationRequestDecoder : public MessageQueue
{
public:
    typedef MessageQueue Base;

    CIMOperationRequestDecoder(
        MessageQueue* outputQueue,
        Uint32 returnQueueId);

   ~CIMOperationRequestDecoder();

   void sendResponse(
        Uint32 queueId,
        Buffer& message,
        Boolean closeConnect = false);

    void sendIMethodError(
        Uint32 queueId,
        HttpMethod httpMethod,
        const String& messageId,
        const String& methodName,
        const CIMException& cimException,
        Boolean closeConnect = false);

    void sendMethodError(
        Uint32 queueId,
        HttpMethod httpMethod,
        const String& messageId,
        const String& methodName,
        const CIMException& cimException,
        Boolean closeConnect = false);

    void sendUserAccountExpired(
        Uint32 queueId,
        HttpMethod httpMethod,
        const String& messageId,
        const String& methodName,
        Boolean closeConnect,
        Boolean isIMethod);


    void sendHttpError(
        Uint32 queueId,
        const String& status,
        const String& cimError = String::EMPTY,
        const String& messageBody = String::EMPTY,
        Boolean closeConnect = false);

    virtual void handleEnqueue(Message *);

    virtual void handleEnqueue();

    void handleHTTPMessage(HTTPMessage* httpMessage);

    void handleMethodCall(
        Uint32 queueId,
        HttpMethod httpMethod,
        char* content,
        Uint32 contentLength,
        const char* cimProtocolVersionInHeader,
        const String& cimMethodInHeader,
        const String& cimObjectInHeader,
        const String& authType,
        const String& userName,
        const String& userRole,
        const String& userPass,
        Boolean isExpiredPassword,
        Boolean updateExpiredPassword,
        const String& ipAddress,
        const AcceptLanguageList& httpAcceptLanguages,
        const ContentLanguageList& httpContentLanguages,
        Boolean closeConnect,
        Boolean binaryRequest,
        Boolean binaryResponse);

    CIMCreateClassRequestMessage* decodeCreateClassRequest(
        Uint32 queueId,
        XmlParser& parser,
        const String& messageId,
        const CIMNamespaceName& nameSpace);

    CIMGetClassRequestMessage* decodeGetClassRequest(
        Uint32 queueId,
        XmlParser& parser,
        const String& messageId,
        const CIMNamespaceName& nameSpace);

    CIMModifyClassRequestMessage* decodeModifyClassRequest(
        Uint32 queueId,
        XmlParser& parser,
        const String& messageId,
        const CIMNamespaceName& nameSpace);

    CIMEnumerateClassNamesRequestMessage* decodeEnumerateClassNamesRequest(
        Uint32 queueId,
        XmlParser& parser,
        const String& messageId,
        const CIMNamespaceName& nameSpace);

    CIMEnumerateClassesRequestMessage* decodeEnumerateClassesRequest(
        Uint32 queueId,
        XmlParser& parser,
        const String& messageId,
        const CIMNamespaceName& nameSpace);

    CIMDeleteClassRequestMessage* decodeDeleteClassRequest(
        Uint32 queueId,
        XmlParser& parser,
        const String& messageId,
        const CIMNamespaceName& nameSpace);

    CIMCreateInstanceRequestMessage* decodeCreateInstanceRequest(
        Uint32 queueId,
        XmlParser& parser,
        const String& messageId,
        const CIMNamespaceName& nameSpace);

    CIMGetInstanceRequestMessage* decodeGetInstanceRequest(
        Uint32 queueId,
        XmlParser& parser,
        const String& messageId,
        const CIMNamespaceName& nameSpace);

    CIMModifyInstanceRequestMessage* decodeModifyInstanceRequest(
        Uint32 queueId,
        XmlParser& parser,
        const String& messageId,
        const CIMNamespaceName& nameSpace);

    CIMEnumerateInstanceNamesRequestMessage*
        decodeEnumerateInstanceNamesRequest(
            Uint32 queueId,
            XmlParser& parser,
            const String& messageId,
            const CIMNamespaceName& nameSpace);

    CIMEnumerateInstancesRequestMessage* decodeEnumerateInstancesRequest(
        Uint32 queueId,
        XmlParser& parser,
        const String& messageId,
        const CIMNamespaceName& nameSpace);

    CIMDeleteInstanceRequestMessage* decodeDeleteInstanceRequest(
        Uint32 queueId,
        XmlParser& parser,
        const String& messageId,
        const CIMNamespaceName& nameSpace);

    CIMSetQualifierRequestMessage* decodeSetQualifierRequest(
        Uint32 queueId,
        XmlParser& parser,
        const String& messageId,
        const CIMNamespaceName& nameSpace);

    CIMGetQualifierRequestMessage* decodeGetQualifierRequest(
        Uint32 queueId,
        XmlParser& parser,
        const String& messageId,
        const CIMNamespaceName& nameSpace);

    CIMEnumerateQualifiersRequestMessage* decodeEnumerateQualifiersRequest(
        Uint32 queueId,
        XmlParser& parser,
        const String& messageId,
        const CIMNamespaceName& nameSpace);

    CIMDeleteQualifierRequestMessage* decodeDeleteQualifierRequest(
        Uint32 queueId,
        XmlParser& parser,
        const String& messageId,
        const CIMNamespaceName& nameSpace);

    CIMReferenceNamesRequestMessage* decodeReferenceNamesRequest(
        Uint32 queueId,
        XmlParser& parser,
        const String& messageId,
        const CIMNamespaceName& nameSpace);

    CIMReferencesRequestMessage* decodeReferencesRequest(
        Uint32 queueId,
        XmlParser& parser,
        const String& messageId,
        const CIMNamespaceName& nameSpace);

    CIMAssociatorNamesRequestMessage* decodeAssociatorNamesRequest(
        Uint32 queueId,
        XmlParser& parser,
        const String& messageId,
        const CIMNamespaceName& nameSpace);

    CIMAssociatorsRequestMessage* decodeAssociatorsRequest(
        Uint32 queueId,
        XmlParser& parser,
        const String& messageId,
        const CIMNamespaceName& nameSpace);

    CIMGetPropertyRequestMessage* decodeGetPropertyRequest(
        Uint32 queueId,
        XmlParser& parser,
        const String& messageId,
        const CIMNamespaceName& nameSpace);

    CIMSetPropertyRequestMessage* decodeSetPropertyRequest(
        Uint32 queueId,
        XmlParser& parser,
        const String& messageId,
        const CIMNamespaceName& nameSpace);

    CIMExecQueryRequestMessage* decodeExecQueryRequest(
        Uint32 queueId,
        XmlParser& parser,
        const String& messageId,
        const CIMNamespaceName& nameSpace);

    CIMInvokeMethodRequestMessage* decodeInvokeMethodRequest(
        Uint32 queueId,
        XmlParser& parser,
        const String& messageId,
        const CIMObjectPath& reference,
        const String& cimMethodName);

// EXP_PULL_BEGIN
    CIMOpenEnumerateInstancesRequestMessage*
        decodeOpenEnumerateInstancesRequest(
            Uint32 queueId,
            XmlParser& parser,
            const String& messageId,
            const CIMNamespaceName& nameSpace);

    CIMOpenReferenceInstancePathsRequestMessage*
        decodeOpenReferenceInstancePathsRequest(
            Uint32 queueId,
            XmlParser& parser,
            const String& messageId,
            const CIMNamespaceName& nameSpace);

    CIMOpenAssociatorInstancesRequestMessage*
        decodeOpenAssociatorInstancesRequest(
            Uint32 queueId,
            XmlParser& parser,
            const String& messageId,
            const CIMNamespaceName& nameSpace);

    CIMOpenAssociatorInstancePathsRequestMessage*
        decodeOpenAssociatorInstancePathsRequest(
            Uint32 queueId,
            XmlParser& parser,
            const String& messageId,
            const CIMNamespaceName& nameSpace);

    CIMOpenReferenceInstancesRequestMessage*
        decodeOpenReferenceInstancesRequest(
            Uint32 queueId,
            XmlParser& parser,
            const String& messageId,
            const CIMNamespaceName& nameSpace);

    CIMOpenEnumerateInstancePathsRequestMessage*
        decodeOpenEnumerateInstancePathsRequest(
            Uint32 queueId,
            XmlParser& parser,
            const String& messageId,
            const CIMNamespaceName& nameSpace);

    CIMPullInstancesWithPathRequestMessage* decodePullInstancesWithPathRequest(
        Uint32 queueId,
        XmlParser& parser,
        const String& messageId,
        const CIMNamespaceName& nameSpace);

    CIMPullInstancePathsRequestMessage* decodePullInstancePathsRequest(
        Uint32 queueId,
        XmlParser& parser,
        const String& messageId,
        const CIMNamespaceName& nameSpace);

    CIMPullInstancesRequestMessage* decodePullInstancesRequest(
        Uint32 queueId,
        XmlParser& parser,
        const String& messageId,
        const CIMNamespaceName& nameSpace);

    CIMCloseEnumerationRequestMessage* decodeCloseEnumerationRequest(
        Uint32 queueId,
        XmlParser& parser,
        const String& messageId,
        const CIMNamespaceName& nameSpace);

    CIMEnumerationCountRequestMessage* decodeEnumerationCountRequest(
        Uint32 queueId,
        XmlParser& parser,
        const String& messageId,
        const CIMNamespaceName& nameSpace);

    CIMOpenQueryInstancesRequestMessage*  decodeOpenQueryInstancesRequest(
        Uint32 queueId,
        XmlParser& parser,
        const String& messageId,
        const CIMNamespaceName& nameSpace);
// EXP_PULL_END

    /** Sets the flag to indicate whether or not the CIMServer is
        shutting down.
    */
    void setServerTerminating(Boolean flag);

private:

#ifdef PEGASUS_PAM_SESSION_SECURITY
    void _updateExpiredPassword(
        Uint32 queueId,
        HttpMethod httpMethod,
        const String& messageId,
        Boolean closeConnect,
        const ContentLanguageList& httpContentLanguages,
        CIMMessage* request,
        const String& userName,
        const String& oldPass,
        const String& ipAddress);
#endif

    // Do not make _outputQueue an AutoPtr.
    MessageQueue* _outputQueue;

    // Queue where responses should be enqueued.
    Uint32 _returnQueueId;

    // Flag to indicate whether or not the CIMServer is shutting down.
    Boolean _serverTerminating;
};

PEGASUS_NAMESPACE_END

#endif /* Pegasus_CIMOperationRequestDecoder_h */
