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
// Author: Mike Brasher (mbrasher@bmc.com)
//
// Modified By: Nitin Upasani, Hewlett-Packard Company (Nitin_Upasani@hp.com)
//              Nag Boranna, Hewlett-Packard Company (nagaraja_boranna@hp.com)
//              Jenny Yu, Hewlett-Packard Company (jenny_yu@hp.com)
//              Sushma Fernandes, Hewlett-Packard Company
//              (sushma_fernandes@hp.com)
//              Carol Ann Krug Graves, Hewlett-Packard Company
//                (carolann_graves@hp.com)
//                Jair Santos, Hewlett-Packard Company (jair.santos@hp.com)
//              Amit K Arora, IBM (amita@in.ibm.com) for PEP#101
//                Mateus Baur, Hewlett-Packard Company (mateus.baur@hp.com)
//
//%////////////////////////////////////////////////////////////////////////////

#ifndef Pegasus_CIMOperationRequestDecoder_h
#define Pegasus_CIMOperationRequestDecoder_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/MessageQueueService.h>
#include <Pegasus/Common/CIMMessage.h>
#include <Pegasus/Common/HTTPMessage.h>
#include <Pegasus/Common/AcceptLanguageList.h>
#include <Pegasus/Common/ContentLanguageList.h>
#include <Pegasus/Common/AutoPtr.h>

PEGASUS_NAMESPACE_BEGIN

class XmlParser;

/** This class decodes CIM operation requests and passes them down-stream.
 */
class CIMOperationRequestDecoder : public MessageQueueService
{
   public:
      typedef MessageQueueService Base;

      CIMOperationRequestDecoder(
     MessageQueueService* outputQueue,
     Uint32 returnQueueId);

      ~CIMOperationRequestDecoder();

      void sendResponse(
     Uint32 queueId,
     Buffer& message);

      void sendIMethodError(
     Uint32 queueId,
         HttpMethod httpMethod,
     const String& messageId,
     const String& methodName,
     const CIMException& cimException);

      void sendMethodError(
     Uint32 queueId,
         HttpMethod httpMethod,
     const String& messageId,
     const String& methodName,
     const CIMException& cimException);

      void sendHttpError(
     Uint32 queueId,
     const String& status,
     const String& cimError = String::EMPTY,
     const String& messageBody = String::EMPTY);

      virtual void handleEnqueue(Message *);

      virtual void handleEnqueue();

      void handleHTTPMessage(HTTPMessage* httpMessage);

// l10n
      void handleMethodCall(
     Uint32 queueId,
         HttpMethod httpMethod,
     char* content,
         Uint32 contentLength,
     const char* cimProtocolVersionInHeader,
     const String& cimMethodInHeader,
     const String& cimObjectInHeader,
     String authType,
     String userName,
     String password,
     const AcceptLanguageList& httpAcceptLanguages,
     const ContentLanguageList& httpContentLanguages);

      CIMCreateClassRequestMessage* decodeCreateClassRequest(
     Uint32 queueId,
     XmlParser& parser,
     const String& messageId,
     const CIMNamespaceName& nameSpace,
     const String& authType,
     const String& userName);


      CIMGetClassRequestMessage* decodeGetClassRequest(
     Uint32 queueId,
     XmlParser& parser,
     const String& messageId,
     const CIMNamespaceName& nameSpace,
     const String& authType,
     const String& userName);

      CIMModifyClassRequestMessage* decodeModifyClassRequest(
     Uint32 queueId,
     XmlParser& parser,
     const String& messageId,
     const CIMNamespaceName& nameSpace,
     const String& authType,
     const String& userName);

      CIMEnumerateClassNamesRequestMessage* decodeEnumerateClassNamesRequest(
     Uint32 queueId,
     XmlParser& parser,
     const String& messageId,
     const CIMNamespaceName& nameSpace,
     const String& authType,
     const String& userName);

      CIMEnumerateClassesRequestMessage* decodeEnumerateClassesRequest(
     Uint32 queueId,
     XmlParser& parser,
     const String& messageId,
     const CIMNamespaceName& nameSpace,
     const String& authType,
     const String& userName);

      CIMDeleteClassRequestMessage* decodeDeleteClassRequest(
     Uint32 queueId,
     XmlParser& parser,
     const String& messageId,
     const CIMNamespaceName& nameSpace,
     const String& authType,
     const String& userName);

      CIMCreateInstanceRequestMessage* decodeCreateInstanceRequest(
     Uint32 queueId,
     XmlParser& parser,
     const String& messageId,
     const CIMNamespaceName& nameSpace,
     const String& authType,
     const String& userName);

      CIMGetInstanceRequestMessage* decodeGetInstanceRequest(
     Uint32 queueId,
     XmlParser& parser,
     const String& messageId,
     const CIMNamespaceName& nameSpace,
     const String& authType,
     const String& userName);

      CIMModifyInstanceRequestMessage*
          decodeModifyInstanceRequest(
     Uint32 queueId,
     XmlParser& parser,
     const String& messageId,
     const CIMNamespaceName& nameSpace,
     const String& authType,
     const String& userName);

      CIMEnumerateInstanceNamesRequestMessage*
          decodeEnumerateInstanceNamesRequest(
     Uint32 queueId,
     XmlParser& parser,
     const String& messageId,
     const CIMNamespaceName& nameSpace,
     const String& authType,
     const String& userName);

      CIMEnumerateInstancesRequestMessage* decodeEnumerateInstancesRequest(
     Uint32 queueId,
     XmlParser& parser,
     const String& messageId,
     const CIMNamespaceName& nameSpace,
     const String& authType,
     const String& userName);

      CIMDeleteInstanceRequestMessage* decodeDeleteInstanceRequest(
     Uint32 queueId,
     XmlParser& parser,
     const String& messageId,
     const CIMNamespaceName& nameSpace,
     const String& authType,
     const String& userName);

      CIMSetQualifierRequestMessage* decodeSetQualifierRequest(
     Uint32 queueId,
     XmlParser& parser,
     const String& messageId,
     const CIMNamespaceName& nameSpace,
     const String& authType,
     const String& userName);

      CIMGetQualifierRequestMessage* decodeGetQualifierRequest(
     Uint32 queueId,
     XmlParser& parser,
     const String& messageId,
     const CIMNamespaceName& nameSpace,
     const String& authType,
     const String& userName);

      CIMEnumerateQualifiersRequestMessage* decodeEnumerateQualifiersRequest(
     Uint32 queueId,
     XmlParser& parser,
     const String& messageId,
     const CIMNamespaceName& nameSpace,
     const String& authType,
     const String& userName);

      CIMDeleteQualifierRequestMessage* decodeDeleteQualifierRequest(
     Uint32 queueId,
     XmlParser& parser,
     const String& messageId,
     const CIMNamespaceName& nameSpace,
     const String& authType,
     const String& userName);

      CIMReferenceNamesRequestMessage* decodeReferenceNamesRequest(
     Uint32 queueId,
     XmlParser& parser,
     const String& messageId,
     const CIMNamespaceName& nameSpace,
     const String& authType,
     const String& userName);

      CIMReferencesRequestMessage* decodeReferencesRequest(
     Uint32 queueId,
     XmlParser& parser,
     const String& messageId,
     const CIMNamespaceName& nameSpace,
     const String& authType,
     const String& userName);

      CIMAssociatorNamesRequestMessage* decodeAssociatorNamesRequest(
     Uint32 queueId,
     XmlParser& parser,
     const String& messageId,
     const CIMNamespaceName& nameSpace,
     const String& authType,
     const String& userName);

      CIMAssociatorsRequestMessage* decodeAssociatorsRequest(
     Uint32 queueId,
     XmlParser& parser,
     const String& messageId,
     const CIMNamespaceName& nameSpace,
     const String& authType,
     const String& userName);

      CIMGetPropertyRequestMessage* decodeGetPropertyRequest(
     Uint32 queueId,
     XmlParser& parser,
     const String& messageId,
     const CIMNamespaceName& nameSpace,
     const String& authType,
     const String& userName);

      CIMSetPropertyRequestMessage* decodeSetPropertyRequest(
     Uint32 queueId,
     XmlParser& parser,
     const String& messageId,
     const CIMNamespaceName& nameSpace,
     const String& authType,
     const String& userName);

      CIMExecQueryRequestMessage* decodeExecQueryRequest(
     Uint32 queueId,
     XmlParser& parser,
     const String& messageId,
     const CIMNamespaceName& nameSpace,
     const String& authType,
     const String& userName);

      CIMInvokeMethodRequestMessage* decodeInvokeMethodRequest(
     Uint32 queueId,
     XmlParser& parser,
     const String& messageId,
     const CIMObjectPath& reference,
     const String& cimMethodName,
     const String& authType,
     const String& userName);

      /** Sets the flag to indicate whether or not the CIMServer is
      shutting down.
      */
      void setServerTerminating(Boolean flag);

   private:

      AutoPtr<MessageQueueService> _outputQueue; //PEP101

      // Queue where responses should be enqueued.
      Uint32 _returnQueueId;

      // Flag to indicate whether or not the CIMServer is shutting down.
      Boolean _serverTerminating;
};

PEGASUS_NAMESPACE_END

#endif /* Pegasus_CIMOperationRequestDecoder_h */
