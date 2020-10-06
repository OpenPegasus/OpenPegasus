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

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/Constants.h>
#include <cctype>
#include <cstdio>
#include <Pegasus/Common/XmlParser.h>
#include <Pegasus/Common/XmlReader.h>
#include <Pegasus/Common/XmlWriter.h>
#include <Pegasus/Common/XmlConstants.h>
#include <Pegasus/Common/System.h>
#include <Pegasus/Common/Logger.h>
#include <Pegasus/Common/Tracer.h>
#include <Pegasus/Common/StatisticalData.h>
#include <Pegasus/Common/OperationContext.h>
#include <Pegasus/Common/CommonUTF.h>
#include "CIMOperationRequestDecoder.h"
#include "WMIMapperUserInfoContainer.h"

// l10n
#include <Pegasus/Common/MessageLoader.h>

PEGASUS_USING_STD;

PEGASUS_NAMESPACE_BEGIN

CIMOperationRequestDecoder::CIMOperationRequestDecoder(
   MessageQueueService* outputQueue,
   Uint32 returnQueueId)
   :  Base(PEGASUS_QUEUENAME_OPREQDECODER),
      _outputQueue(outputQueue),
      _returnQueueId(returnQueueId),
      _serverTerminating(false)

{
}

CIMOperationRequestDecoder::~CIMOperationRequestDecoder()
{
    _outputQueue.release();
}

void CIMOperationRequestDecoder::sendResponse(
   Uint32 queueId,
   Buffer& message)
{
   MessageQueue* queue = MessageQueue::lookup(queueId);

   if (queue)
   {
      AutoPtr<HTTPMessage> httpMessage(new HTTPMessage(message));
      queue->enqueue(httpMessage.release());
   }
}

void CIMOperationRequestDecoder::sendIMethodError(
   Uint32 queueId,
   HttpMethod httpMethod,
   const String& messageId,
   const String& iMethodName,
   const CIMException& cimException)
{
    Buffer message;
    message = XmlWriter::formatSimpleIMethodErrorRspMessage(
        iMethodName,
        messageId,
        httpMethod,
        cimException);

    sendResponse(queueId, message);
}

void CIMOperationRequestDecoder::sendMethodError(
   Uint32 queueId,
   HttpMethod httpMethod,
   const String& messageId,
   const String& methodName,
   const CIMException& cimException)
{
    Buffer message;
    message = XmlWriter::formatSimpleMethodErrorRspMessage(
        methodName,
        messageId,
        httpMethod,
        cimException);

    sendResponse(queueId, message);
}

void CIMOperationRequestDecoder::sendHttpError(
   Uint32 queueId,
   const String& status,
   const String& cimError,
   const String& pegasusError)
{
    Buffer message;
    message = XmlWriter::formatHttpErrorRspMessage(
        status,
        cimError,
        pegasusError);

    sendResponse(queueId, message);
}

void CIMOperationRequestDecoder::handleEnqueue(Message *message)
{

   if (!message)
      return;

   switch (message->getType())
   {
      case HTTP_MESSAGE:
     handleHTTPMessage((HTTPMessage*)message);
     break;
   }

   delete message;
}


void CIMOperationRequestDecoder::handleEnqueue()
{
   Message* message = dequeue();
   if(message)
      handleEnqueue(message);
}

//-----------------------------------------------------------------------------
//
// From the HTTP/1.1 Specification (RFC 2626):
//
// Both types of message consist of a start-line, zero or more header fields
// (also known as "headers"), an empty line (i.e., a line with nothing
// preceding the CRLF) indicating the end of the header fields, and possibly
// a message-body.
//
// Example CIM request:
//
//     M-POST /cimom HTTP/1.1
//     HOST: www.erewhon.com
//     Content-Type: application/xml; charset=utf-8
//     Content-Length: xxxx
//     Man: http://www.dmtf.org/cim/operation ; ns=73
//     73-CIMOperation: MethodCall
//     73-CIMMethod: EnumerateInstances
//     73-CIMObject: root/cimv2
//
//-----------------------------------------------------------------------------

void CIMOperationRequestDecoder::handleHTTPMessage(HTTPMessage* httpMessage)
{
   PEG_METHOD_ENTER(TRC_DISPATCHER,
      "CIMOperationRequestDecoder::handleHTTPMessage()");

   // Set the Accept-Language into the thread for this service.
   // This will allow all code in this thread to get
   // the languages for the messages returned to the client.
   Thread::setLanguages(httpMessage->acceptLanguages);

   // Save queueId:

   Uint32 queueId = httpMessage->queueId;

   // Save userName and authType:

   String userName;
   String password;
   String authType;

   userName = httpMessage->authInfo->getAuthenticatedUser();
   password = httpMessage->authInfo->getAuthenticatedPassword();
   authType = httpMessage->authInfo->getAuthType();

   // Parse the HTTP message:

   String startLine;
   Array<HTTPHeader> headers;
   char* content;
   Uint32 contentLength;

   httpMessage->parse(startLine, headers, contentLength);

   // Parse the request line:

   String methodName;
   String requestUri;
   String httpVersion;
   HttpMethod httpMethod  = HTTP_METHOD__POST;

   PEG_TRACE_CSTRING(TRC_XML_IO, Tracer::LEVEL4,
         httpMessage->message.getData());

   HTTPMessage::parseRequestLine(
      startLine, methodName, requestUri, httpVersion);

   //
   //  Set HTTP method for the request
   //
   if (methodName == "M-POST")
   {
       httpMethod = HTTP_METHOD_M_POST;
   }

   // Unsupported methods are caught in the HTTPAuthenticatorDelegator
   PEGASUS_ASSERT(methodName == "M-POST" || methodName == "POST");

   //
   //  Mismatch of method and version is caught in HTTPAuthenticatorDelegator
   //
   PEGASUS_ASSERT (!((httpMethod == HTTP_METHOD_M_POST) &&
                     (httpVersion == "HTTP/1.0")));

   // Process M-POST and POST messages:

   // Validate the "CIMOperation" header:

   const char* cimOperation;
   Boolean operationHeaderFound = HTTPMessage::lookupHeader(
      headers, "CIMOperation", cimOperation, true);
   // If the CIMOperation header was missing, the HTTPAuthenticatorDelegator
   // would not have passed the message to us.
   PEGASUS_ASSERT(operationHeaderFound);

   if (System::strcasecmp(cimOperation, "MethodCall") != 0)
   {
      // The Specification for CIM Operations over HTTP reads:
      //     3.3.4. CIMOperation
      //     If a CIM Server receives CIM Operation request with this
      //     [CIMOperation] header, but with a missing value or a value
      //     that is not "MethodCall", then it MUST fail the request with
      //     status "400 Bad Request". The CIM Server MUST include a
      //     CIMError header in the response with a value of
      //     unsupported-operation.
      MessageLoaderParms parms(
          "Server.CIMOperationRequestDecoder."
            "CIMOPERATION_VALUE_NOT_SUPPORTED",
          "CIMOperation value \"$0\" is not supported.",
          cimOperation);

      sendHttpError(queueId, HTTP_STATUS_BADREQUEST,
          "unsupported-operation", MessageLoader::getMessage(parms));

      PEG_METHOD_EXIT();

      return;
   }

   // Validate the "CIMBatch" header:

   const char* cimBatch;
   if (HTTPMessage::lookupHeader(headers, "CIMBatch", cimBatch, true))
   {
      // The Specification for CIM Operations over HTTP reads:
      //     3.3.9. CIMBatch
      //     If a CIM Server receives CIM Operation Request for which the
      //     CIMBatch header is present, but the Server does not support
      //     Multiple Operations, then it MUST fail the request and
      //     return a status of "501 Not Implemented".
      sendHttpError(queueId,
                    HTTP_STATUS_NOTIMPLEMENTED,
                    "multiple-requests-unsupported");
      PEG_METHOD_EXIT();
      return;
   }

   // Save these headers for later checking

   const char* cimProtocolVersion;
   if (!HTTPMessage::lookupHeader(
           headers, "CIMProtocolVersion", cimProtocolVersion, true))
   {
      // Mandated by the Specification for CIM Operations over HTTP
      cimProtocolVersion = "1.0";
   }

   String cimMethod;
   if (HTTPMessage::lookupHeader(headers, "CIMMethod", cimMethod, true))
   {
      if (cimMethod == String::EMPTY)
      {
         // This is not a valid value, and we use
         // EMPTY to mean "absent"
         MessageLoaderParms parms(
             "Server.CIMOperationRequestDecoder.EMPTY_CIMMETHOD_VALUE",
             "Empty CIMMethod value.");

         sendHttpError(queueId, HTTP_STATUS_BADREQUEST, "header-mismatch",
            MessageLoader::getMessage(parms));

         PEG_METHOD_EXIT();

         return;
      }

      try
      {
         cimMethod = XmlReader::decodeURICharacters(cimMethod);
      }
      catch (const ParseError&)
      {
         // The CIMMethod header value could not be decoded
         MessageLoaderParms parms(
             "Server.CIMOperationRequestDecoder.CIMMETHOD_VALUE_SYNTAX_ERROR",
             "CIMMethod value syntax error.");

         sendHttpError(queueId, HTTP_STATUS_BADREQUEST,
             "header-mismatch", MessageLoader::getMessage(parms));

         PEG_METHOD_EXIT();

         return;
      }
   }

   String cimObject;
   if (HTTPMessage::lookupHeader(headers, "CIMObject", cimObject, true))
   {
      if (cimObject == String::EMPTY)
      {
         // This is not a valid value, and we use EMPTY
         // to mean "absent"
         MessageLoaderParms parms(
             "Server.CIMOperationRequestDecoder.EMPTY_CIMOBJECT_VALUE",
             "Empty CIMObject value.");

         sendHttpError(queueId, HTTP_STATUS_BADREQUEST,
             "header-mismatch", MessageLoader::getMessage(parms));

         PEG_METHOD_EXIT();

         return;
      }

      try
      {
         cimObject = XmlReader::decodeURICharacters(cimObject);
      }
      catch (const ParseError&)
      {
         // The CIMObject header value could not be decoded
         //sendHttpError(queueId, HTTP_STATUS_BADREQUEST, "header-mismatch",
                       //"CIMObject value syntax error.");
         MessageLoaderParms parms(
             "Server.CIMOperationRequestDecoder.CIMOBJECT_VALUE_SYNTAX_ERROR",
             "CIMObject value syntax error.");

         sendHttpError(queueId, HTTP_STATUS_BADREQUEST, "header-mismatch",
                           MessageLoader::getMessage(parms));

         PEG_METHOD_EXIT();

         return;
      }
   }

   // Calculate the beginning of the content from the message size and
   // the content length.

   content = (char *) httpMessage->message.getData() +
       httpMessage->message.size() - contentLength;

   // Validate the "Content-Type" header:

   const char* cimContentType;
   Boolean contentTypeHeaderFound = HTTPMessage::lookupHeader(
       headers, "Content-Type", cimContentType, true);
   String type;
   String charset;

   if (!contentTypeHeaderFound ||
       !HTTPMessage::parseContentTypeHeader(cimContentType, type, charset) ||
       (!String::equalNoCase(type, "application/xml") &&
        !String::equalNoCase(type, "text/xml")) ||
       !String::equalNoCase(charset, "utf-8"))
   {
       MessageLoaderParms parms(
           "Server.CIMOperationRequestDecoder.CIMCONTENTTYPE_SYNTAX_ERROR",
           "CIMContentType value syntax error.");

       sendHttpError(queueId, HTTP_STATUS_BADREQUEST, "header-mismatch",
           MessageLoader::getMessage(parms));

       PEG_METHOD_EXIT();

       return;
   }
   // Validating content falls within UTF8
   // (required to be complaint with section C12
   // of Unicode 4.0 spec, chapter 3.)
   else
   {
       Uint32 count = 0;
       while(count<contentLength)
       {
       if (!(isUTF8((char *)&content[count])))
       {
            MessageLoaderParms parms(
                "Server.CIMOperationRequestDecoder.INVALID_UTF8_CHARACTER",
                "Invalid UTF-8 character detected.");

           sendHttpError(queueId, HTTP_STATUS_BADREQUEST, "request-not-valid",
                       MessageLoader::getMessage(parms));

           PEG_METHOD_EXIT();
           return;
       }
       UTF8_NEXT(content,count);
       }
   }

   // If it is a method call, then dispatch it to be handled:

// l10n
   handleMethodCall(queueId, httpMethod, content, contentLength,
                    cimProtocolVersion, cimMethod, cimObject, authType,
                    userName, password, httpMessage->acceptLanguages,
                    httpMessage->contentLanguages);

   PEG_METHOD_EXIT();
}


void CIMOperationRequestDecoder::handleMethodCall(
   Uint32 queueId,
   HttpMethod httpMethod,
   char* content,
   Uint32 contentLength,    // used for statistics only
   const char* cimProtocolVersionInHeader,
   const String& cimMethodInHeader,
   const String& cimObjectInHeader,
   String authType,
   String userName,
   String password,
   const AcceptLanguageList& httpAcceptLanguages,  // l10n
   const ContentLanguageList& httpContentLanguages)
{
   PEG_METHOD_ENTER(TRC_DISPATCHER,
      "CIMOperationRequestDecoder::handleMethodCall()");

   //
   // If CIMOM is shutting down, return "Service Unavailable" response
   //
   if (_serverTerminating)
   {
       MessageLoaderParms parms(
           "Server.CIMOperationRequestDecoder.CIMSERVER_SHUTTING_DOWN",
           "CIM Server is shutting down.");

       sendHttpError(queueId, HTTP_STATUS_SERVICEUNAVAILABLE, String::EMPTY,
                       MessageLoader::getMessage(parms));

       PEG_METHOD_EXIT();

       return;
   }

   // Create a parser:

   XmlParser parser(content);
   XmlEntry entry;
   String messageId;
   const char* cimMethodName = "";
   AutoPtr<Message> request;

   try
   {
      //
      // Process <?xml ... >
      //

      // These values are currently unused
      const char* xmlVersion = 0;
      const char* xmlEncoding = 0;

      XmlReader::getXmlDeclaration(parser, xmlVersion, xmlEncoding);

      // Expect <CIM ...>

      const char* cimVersion = 0;
      const char* dtdVersion = 0;

      XmlReader::getCimStartTag(parser, cimVersion, dtdVersion);

      if (strcmp(cimVersion, "2.0") != 0)
      {
            MessageLoaderParms parms(
                "Server.CIMOperationRequestDecoder.CIM_VERSION_NOT_SUPPORTED",
                "CIM version \"$0\" is not supported.",
                cimVersion);

            sendHttpError(queueId, HTTP_STATUS_NOTIMPLEMENTED,
               "unsupported-cim-version", MessageLoader::getMessage(parms));

            PEG_METHOD_EXIT();

            return;
      }

      // We accept DTD version 2.x (see Bugzilla 1556)

      Boolean dtdVersionAccepted = false;

      if ((dtdVersion[0] == '2') &&
          (dtdVersion[1] == '.') &&
          (dtdVersion[2] != 0))
      {
         // Verify that all characters after the '.' are digits
         Uint32 index = 2;
         while (isdigit(dtdVersion[index]))
         {
            index++;
         }

         if (dtdVersion[index] == 0)
         {
            dtdVersionAccepted = true;
         }
      }

      if (!dtdVersionAccepted)
      {
            MessageLoaderParms parms(
                "Server.CIMOperationRequestDecoder.DTD_VERSION_NOT_SUPPORTED",
                "DTD version \"$0\" is not supported.",
                dtdVersion);

            sendHttpError(queueId, HTTP_STATUS_NOTIMPLEMENTED,
               "unsupported-dtd-version", MessageLoader::getMessage(parms));

            PEG_METHOD_EXIT();

            return;
      }

      // Expect <MESSAGE ...>

      String protocolVersion;

      if (!XmlReader::getMessageStartTag(
         parser, messageId, protocolVersion))
      {
            MessageLoaderParms mlParms(
                "Server.CIMOperationRequestDecoder."
                    "EXPECTED_MESSAGE_ELEMENT",
                "expected MESSAGE element");

            throw XmlValidationError(parser.getLine(), mlParms);
      }

      // Validate that the protocol version in the header matches the XML

      if (!String::equalNoCase(protocolVersion, cimProtocolVersionInHeader))
      {
            MessageLoaderParms parms(
                "Server.CIMOperationRequestDecoder."
                    "CIMPROTOCOL_VERSION_MISMATCH",
                "CIMProtocolVersion value \"$0\" does not match "
                    "CIM request protocol version \"$1\".",
                cimProtocolVersionInHeader,
                protocolVersion);

            sendHttpError(queueId, HTTP_STATUS_BADREQUEST,
                "header-mismatch", MessageLoader::getMessage(parms));

            PEG_METHOD_EXIT();
            return;
      }

      // We accept protocol version 1.x (see Bugzilla 1556)

      Boolean protocolVersionAccepted = false;

      if ((protocolVersion.size() >= 3) &&
          (protocolVersion[0] == '1') &&
          (protocolVersion[1] == '.'))
      {
         // Verify that all characters after the '.' are digits
         Uint32 index = 2;
         while ((index < protocolVersion.size()) &&
                (protocolVersion[index] >= '0') &&
                (protocolVersion[index] <= '9'))
         {
            index++;
         }

         if (index == protocolVersion.size())
         {
            protocolVersionAccepted = true;
         }
      }

      if (!protocolVersionAccepted)
      {
         // See Specification for CIM Operations over HTTP section 4.3
         MessageLoaderParms parms(
             "Server.CIMOperationRequestDecoder."
                "CIMPROTOCOL_VERSION_NOT_SUPPORTED",
             "CIMProtocolVersion \"$0\" is not supported.",
             protocolVersion);

         sendHttpError(queueId, HTTP_STATUS_NOTIMPLEMENTED,
             "unsupported-protocol-version",
             MessageLoader::getMessage(parms));

         PEG_METHOD_EXIT();
         return;
      }

      if (XmlReader::testStartTag(parser, entry, "MULTIREQ"))
      {
         // We wouldn't have gotten here if CIMBatch header was specified,
         // so this must be indicative of a header mismatch

         MessageLoaderParms parms(
                "Server.CIMOperationRequestDecoder."
                    "MULTI_REQUEST_MISSING_CIMBATCH_HTTP_HEADER",
                "Multi-request is missing CIMBatch HTTP header");

         sendHttpError(queueId, HTTP_STATUS_BADREQUEST,
           "header-mismatch", MessageLoader::getMessage(parms));

         PEG_METHOD_EXIT();
         return;
         // Future: When MULTIREQ is supported, must ensure CIMMethod and
         // CIMObject headers are absent, and CIMBatch header is present.
      }

      // Expect <SIMPLEREQ ...>

      XmlReader::expectStartTag(parser, entry, "SIMPLEREQ");

      // Check for <IMETHODCALL ...>

      if (XmlReader::getIMethodCallStartTag(parser, cimMethodName))
      {
         // The Specification for CIM Operations over HTTP reads:
         //     3.3.6. CIMMethod
         //
         //     This header MUST be present in any CIM Operation Request
         //     message that contains a Simple Operation Request.
         //
         //     It MUST NOT be present in any CIM Operation Response message,
         //     nor in any CIM Operation Request message that is not a
         //     Simple Operation Request.
         //
         //     The name of the CIM method within a Simple Operation Request
         //     is defined to be the value of the NAME attribute of the
         //     <METHODCALL> or <IMETHODCALL> element.
         //
         //     If a CIM Server receives a CIM Operation Request for which
         //     either:
         //
         //     - The CIMMethod header is present but has an invalid value, or;
         //     - The CIMMethod header is not present but the Operation
         //       Request Message is a Simple Operation Request, or;
         //     - The CIMMethod header is present but the Operation Request
         //       Message is not a Simple Operation Request, or;
         //     - The CIMMethod header is present, the Operation Request
         //       Message is a Simple Operation Request, but the CIMIdentifier
         //       value (when unencoded) does not match the unique method name
         //       within the Simple Operation Request,
         //
         //     then it MUST fail the request and return a status of
         //     "400 Bad Request" (and MUST include a CIMError header in the
         //     response with a value of header-mismatch), subject to the
         //     considerations specified in Errors.
         if (!String::equalNoCase(cimMethodName, cimMethodInHeader))
         {
            // ATTN-RK-P3-20020304: How to decode cimMethodInHeader?
            if (cimMethodInHeader == String::EMPTY)
            {
               MessageLoaderParms parms(
                    "Server.CIMOperationRequestDecoder."
                        "MISSING_CIMMETHOD_HTTP_HEADER",
                    "Missing CIMMethod HTTP header.");

               sendHttpError(queueId, HTTP_STATUS_BADREQUEST,
                   "header-mismatch", MessageLoader::getMessage(parms));
            }
            else
            {
               MessageLoaderParms parms(
                    "Server.CIMOperationRequestDecoder."
                        "CIMMETHOD_VALUE_DOES_NOT_MATCH_REQUEST_METHOD",
                    "CIMMethod value \"$0\" does not match "
                        "CIM request method \"$1\".",
                    cimMethodInHeader,
                    cimMethodName);

               sendHttpError(queueId, HTTP_STATUS_BADREQUEST,
                   "header-mismatch", MessageLoader::getMessage(parms));
            }
            PEG_METHOD_EXIT();
            return;
         }

     // Expect <LOCALNAMESPACEPATH ...>

     String nameSpace;

     if (!XmlReader::getLocalNameSpacePathElement(parser, nameSpace))
     {
        MessageLoaderParms mlParms(
            "Server.CIMOperationRequestDecoder."
                "EXPECTED_LOCALNAMESPACEPATH_ELEMENT",
            "expected LOCALNAMESPACEPATH element");

       throw XmlValidationError(parser.getLine(),mlParms);
     }

         // The Specification for CIM Operations over HTTP reads:
         //     3.3.7. CIMObject
         //
         //     This header MUST be present in any CIM Operation Request
         //     message that contains a Simple Operation Request.
         //
         //     It MUST NOT be present in any CIM Operation Response message,
         //     nor in any CIM Operation Request message that is not a
         //     Simple Operation Request.
         //
         //     The header identifies the CIM object (which MUST be a Class
         //     or Instance for an extrinsic method, or a Namespace for an
         //     intrinsic method) on which the method is to be invoked, using
         //     a CIM object path encoded in an HTTP-safe representation.
         //
         //     If a CIM Server receives a CIM Operation Request for which
         //     either:
         //
         //     - The CIMObject header is present but has an invalid value, or;
         //     - The CIMObject header is not present but the Operation
         //       Request Message is a Simple Operation Request, or;
         //     - The CIMObject header is present but the Operation Request
         //       Message is not a Simple Operation Request, or;
         //     - The CIMObject header is present, the Operation Request
         //       Message is a Simple Operation Request, but the ObjectPath
         //       value does not match (where match is defined in the section
         //       section on Encoding CIM Object Paths) the Operation Request
         //       Message,
         //
         //     then it MUST fail the request and return a status of
         //     "400 Bad Request" (and MUST include a CIMError header in the
         //     response with a value of header-mismatch), subject to the
         //     considerations specified in Errors.
         if (!String::equalNoCase(nameSpace, cimObjectInHeader))
         {
            if (cimObjectInHeader == String::EMPTY)
            {
               MessageLoaderParms parms(
                    "Server.CIMOperationRequestDecoder."
                        "MISSING_CIMOBJECT_HTTP_HEADER",
                    "Missing CIMObject HTTP header.");

               sendHttpError(queueId, HTTP_STATUS_BADREQUEST,
                   "header-mismatch", MessageLoader::getMessage(parms));
            }
            else
            {
               MessageLoaderParms parms(
                    "Server.CIMOperationRequestDecoder.Server."
                        "CIMOperationRequestDecoder."
                        "CIMOBJECT_VALUE_DOES_NOT_MATCH_REQUEST_OBJECT:",
                    "CIMObject value \"$0\" does not match CIM "
                        "request object \"$1\".",
                    cimObjectInHeader,
                    nameSpace);
               sendHttpError(queueId, HTTP_STATUS_BADREQUEST,
                   "header-mismatch", MessageLoader::getMessage(parms));
            }
            PEG_METHOD_EXIT();
            return;
         }

         // This try block only catches CIMExceptions, because they must be
         // responded to with a proper IMETHODRESPONSE.  Other exceptions are
         // caught in the outer try block.
         try
         {
        // Delegate to appropriate method to handle:

            if (System::strcasecmp(cimMethodName, "GetClass") == 0)
               request.reset(decodeGetClassRequest(
                  queueId, parser, messageId, nameSpace, authType, userName));
            else if (System::strcasecmp(cimMethodName, "GetInstance") == 0)
               request.reset(decodeGetInstanceRequest(
                  queueId, parser, messageId, nameSpace, authType, userName));
            else if (System::strcasecmp(
                        cimMethodName, "EnumerateClassNames") == 0)
               request.reset(decodeEnumerateClassNamesRequest(
                  queueId, parser, messageId, nameSpace, authType, userName));
            else if (System::strcasecmp(cimMethodName, "References") == 0)
               request.reset(decodeReferencesRequest(
                  queueId, parser, messageId, nameSpace, authType, userName));
            else if (System::strcasecmp(cimMethodName, "ReferenceNames") == 0)
               request.reset(decodeReferenceNamesRequest(
                  queueId, parser, messageId, nameSpace, authType, userName));
            else if (System::strcasecmp(cimMethodName, "AssociatorNames") == 0)
               request.reset(decodeAssociatorNamesRequest(
                  queueId, parser, messageId, nameSpace, authType, userName));
            else if (System::strcasecmp(cimMethodName, "Associators") == 0)
               request.reset(decodeAssociatorsRequest(
                  queueId, parser, messageId, nameSpace, authType, userName));
            else if (System::strcasecmp(cimMethodName, "CreateInstance") == 0)
               request.reset(decodeCreateInstanceRequest(
                  queueId, parser, messageId, nameSpace, authType, userName));
            else if (System::strcasecmp(
                        cimMethodName, "EnumerateInstanceNames")==0)
               request.reset(decodeEnumerateInstanceNamesRequest(
                  queueId, parser, messageId, nameSpace, authType, userName));
            else if (System::strcasecmp(cimMethodName, "DeleteQualifier") == 0)
               request.reset(decodeDeleteQualifierRequest(
                  queueId, parser, messageId, nameSpace, authType, userName));
            else if (System::strcasecmp(cimMethodName, "GetQualifier") == 0)
               request.reset(decodeGetQualifierRequest(
                  queueId, parser, messageId, nameSpace, authType, userName));
            else if (System::strcasecmp(cimMethodName, "SetQualifier") == 0)
               request.reset(decodeSetQualifierRequest(
                  queueId, parser, messageId, nameSpace, authType, userName));
            else if (System::strcasecmp(
                        cimMethodName, "EnumerateQualifiers") == 0)
               request.reset(decodeEnumerateQualifiersRequest(
                  queueId, parser, messageId, nameSpace, authType, userName));
            else if (System::strcasecmp(
                        cimMethodName, "EnumerateClasses") == 0)
               request.reset(decodeEnumerateClassesRequest(
                  queueId, parser, messageId, nameSpace, authType, userName));
            else if (System::strcasecmp(
                        cimMethodName, "EnumerateInstances") == 0)
               request.reset(decodeEnumerateInstancesRequest(
                  queueId, parser, messageId, nameSpace, authType, userName));
            else if (System::strcasecmp(cimMethodName, "CreateClass") == 0)
               request.reset(decodeCreateClassRequest(
                  queueId, parser, messageId, nameSpace, authType, userName));
            else if (System::strcasecmp(cimMethodName, "ModifyClass") == 0)
               request.reset(decodeModifyClassRequest(
                  queueId, parser, messageId, nameSpace, authType, userName));
            else if (System::strcasecmp(cimMethodName, "ModifyInstance") == 0)
               request.reset(decodeModifyInstanceRequest(
                  queueId, parser, messageId, nameSpace, authType, userName));
            else if (System::strcasecmp(cimMethodName, "DeleteClass") == 0)
               request.reset(decodeDeleteClassRequest(
                  queueId, parser, messageId, nameSpace, authType, userName));
            else if (System::strcasecmp(cimMethodName, "DeleteInstance") == 0)
               request.reset(decodeDeleteInstanceRequest(
                  queueId, parser, messageId, nameSpace, authType, userName));
            else if (System::strcasecmp(cimMethodName, "GetProperty") == 0)
               request.reset(decodeGetPropertyRequest(
                  queueId, parser, messageId, nameSpace, authType, userName));
            else if (System::strcasecmp(cimMethodName, "SetProperty") == 0)
               request.reset(decodeSetPropertyRequest(
                  queueId, parser, messageId, nameSpace, authType, userName));
            else if (System::strcasecmp(cimMethodName, "ExecQuery") == 0)
               request.reset(decodeExecQueryRequest(
                  queueId, parser, messageId, nameSpace, authType, userName));
            else
            {
                throw PEGASUS_CIM_EXCEPTION_L(CIM_ERR_NOT_SUPPORTED,
                          MessageLoaderParms(
                            "Server.CIMOperationRequestDecoder."
                                "UNRECOGNIZED_INTRINSIC_METHOD",
                            "Unrecognized intrinsic method: $0",
                            cimMethodName));

            }
         }
         catch (CIMException& e)
         {
            sendIMethodError(
               queueId,
               httpMethod,
               messageId,
               cimMethodName,
               e);

            PEG_METHOD_EXIT();
            return;
         }
         catch (XmlException&)
         {
            // XmlExceptions are handled below
            throw;
         }
         catch (Exception& e)
         {
            // Caught an unexpected exception from decoding.  Since we must
            // have had a problem reconstructing a CIM object, we'll treat it
            // as an invalid parameter
            sendIMethodError(
               queueId,
               httpMethod,
               messageId,
               cimMethodName,
               PEGASUS_CIM_EXCEPTION(
                  CIM_ERR_INVALID_PARAMETER, e.getMessage()));

            PEG_METHOD_EXIT();
            return;
         }

     // Expect </IMETHODCALL>

     XmlReader::expectEndTag(parser, "IMETHODCALL");
      }
      // Expect <METHODCALL ...>
      else if (XmlReader::getMethodCallStartTag(parser, cimMethodName))
      {
     CIMObjectPath reference;

         // The Specification for CIM Operations over HTTP reads:
         //     3.3.6. CIMMethod
         //
         //     This header MUST be present in any CIM Operation Request
         //     message that contains a Simple Operation Request.
         //
         //     It MUST NOT be present in any CIM Operation Response message,
         //     nor in any CIM Operation Request message that is not a
         //     Simple Operation Request.
         //
         //     The name of the CIM method within a Simple Operation Request
         //     is defined to be the value of the NAME attribute of the
         //     <METHODCALL> or <IMETHODCALL> element.
         //
         //     If a CIM Server receives a CIM Operation Request for which
         //     either:
         //
         //     - The CIMMethod header is present but has an invalid value, or;
         //     - The CIMMethod header is not present but the Operation
         //       Request Message is a Simple Operation Request, or;
         //     - The CIMMethod header is present but the Operation Request
         //       Message is not a Simple Operation Request, or;
         //     - The CIMMethod header is present, the Operation Request
         //       Message is a Simple Operation Request, but the CIMIdentifier
         //       value (when unencoded) does not match the unique method name
         //       within the Simple Operation Request,
         //
         //     then it MUST fail the request and return a status of
         //     "400 Bad Request" (and MUST include a CIMError header in the
         //     response with a value of header-mismatch), subject to the
         //     considerations specified in Errors.

         // Extrinic methods can have UTF-8!
         String cimMethodNameUTF16(cimMethodName);
         if (cimMethodNameUTF16 != cimMethodInHeader)
         {
            // ATTN-RK-P3-20020304: How to decode cimMethodInHeader?
            if (cimMethodInHeader == String::EMPTY)
            {
               MessageLoaderParms parms(
                    "Server.CIMOperationRequestDecoder."
                        "MISSING_CIMMETHOD_HTTP_HEADER",
                    "Missing CIMMethod HTTP header.");

               sendHttpError(queueId, HTTP_STATUS_BADREQUEST,
                   "header-mismatch", MessageLoader::getMessage(parms));
            }
            else
            {
               MessageLoaderParms parms(
                    "Server.CIMOperationRequestDecoder."
                        "CIMMETHOD_VALUE_DOES_NOT_MATCH_REQUEST_METHOD",
                    "CIMMethod value \"$0\" does not match CIM request "
                        "method \"$1\".",
                    ( const char *)cimMethodInHeader.getCString(),
                    cimMethodName);

               sendHttpError(queueId, HTTP_STATUS_BADREQUEST,
                   "header-mismatch", MessageLoader::getMessage(parms));
            }
            PEG_METHOD_EXIT();
            return;
         }

     //
     // Check for <LOCALINSTANCEPATHELEMENT> or <LOCALCLASSPATHELEMENT>
     //
         if (!(XmlReader::getLocalInstancePathElement(parser, reference) ||
               XmlReader::getLocalClassPathElement(parser, reference)))
     {
           // l10n TODO Done
           MessageLoaderParms parms("Common.XmlConstants."
               "MISSING_ELEMENT_LOCALPATH", MISSING_ELEMENT_LOCALPATH);
           throw XmlValidationError(parser.getLine(), parms);
       //throw XmlValidationError(parser.getLine(), MISSING_ELEMENT_LOCALPATH);

       // this throw is not updated with MLP because MISSING_ELEMENT_LOCALPATH
       // is a hardcoded variable, not a message
     }

         // The Specification for CIM Operations over HTTP reads:
         //     3.3.7. CIMObject
         //
         //     This header MUST be present in any CIM Operation Request
         //     message that contains a Simple Operation Request.
         //
         //     It MUST NOT be present in any CIM Operation Response message,
         //     nor in any CIM Operation Request message that is not a
         //     Simple Operation Request.
         //
         //     The header identifies the CIM object (which MUST be a Class
         //     or Instance for an extrinsic method, or a Namespace for an
         //     intrinsic method) on which the method is to be invoked, using
         //     a CIM object path encoded in an HTTP-safe representation.
         //
         //     If a CIM Server receives a CIM Operation Request for which
         //     either:
         //
         //     - The CIMObject header is present but has an invalid value, or;
         //     - The CIMObject header is not present but the Operation
         //       Request Message is a Simple Operation Request, or;
         //     - The CIMObject header is present but the Operation Request
         //       Message is not a Simple Operation Request, or;
         //     - The CIMObject header is present, the Operation Request
         //       Message is a Simple Operation Request, but the ObjectPath
         //       value does not match (where match is defined in the section
         //       section on Encoding CIM Object Paths) the Operation Request
         //       Message,
         //
         //     then it MUST fail the request and return a status of
         //     "400 Bad Request" (and MUST include a CIMError header in the
         //     response with a value of header-mismatch), subject to the
         //     considerations specified in Errors.
         if (cimObjectInHeader == String::EMPTY)
         {
            MessageLoaderParms parms("Server.CIMOperationRequestDecoder."
                                    "MISSING_CIMOBJECT_HTTP_HEADER",
                                    "Missing CIMObject HTTP header.");
            sendHttpError(queueId, HTTP_STATUS_BADREQUEST, "header-mismatch",
                           MessageLoader::getMessage(parms));
            PEG_METHOD_EXIT();
            return;
         }

         CIMObjectPath headerObjectReference;
         try
         {
             headerObjectReference.set(cimObjectInHeader);
         }
         catch (Exception&)
         {
            MessageLoaderParms parms(
                "Server.CIMOperationRequestDecoder."
                    "COULD_NOT_PARSE_CIMOBJECT_VALUE",
                "Could not parse CIMObject value \"$0\".",
                cimObjectInHeader);
            sendHttpError(queueId, HTTP_STATUS_BADREQUEST,
                "header-mismatch", MessageLoader::getMessage(parms));
            PEG_METHOD_EXIT();
            return;
         }

         if (!reference.identical(headerObjectReference))
         {
            MessageLoaderParms parms(
                "Server.CIMOperationRequestDecoder.Server."
                    "CIMOperationRequestDecoder."
                    "CIMOBJECT_VALUE_DOES_NOT_MATCH_REQUEST_OBJECT:",
                "CIMObject value \"$0\" does not match CIM "
                    "request object \"$1\".",
                cimObjectInHeader,
                reference.toString());
           sendHttpError(queueId, HTTP_STATUS_BADREQUEST, "header-mismatch",
                           MessageLoader::getMessage(parms));
            PEG_METHOD_EXIT();
            return;
         }

         // This try block only catches CIMExceptions, because they must be
         // responded to with a proper METHODRESPONSE.  Other exceptions are
         // caught in the outer try block.
         try
         {
        // Delegate to appropriate method to handle:

            request.reset(decodeInvokeMethodRequest(
               queueId,
               parser,
               messageId,
               reference,
               cimMethodNameUTF16,   // contains UTF-16 converted from UTF-8
               authType,
               userName));
         }
         catch (CIMException& e)
         {
            sendMethodError(
               queueId,
               httpMethod,
               messageId,
               cimMethodNameUTF16,   // contains UTF-16 converted from UTF-8
               e);

            PEG_METHOD_EXIT();
            return;
         }
         catch (XmlException&)
         {
            // XmlExceptions are handled below
            throw;
         }
         catch (Exception& e)
         {
            // Caught an unexpected exception from decoding.  Since we must
            // have had a problem reconstructing a CIM object, we'll treat it
            // as an invalid parameter
            sendMethodError(
               queueId,
               httpMethod,
               messageId,
               cimMethodNameUTF16,   // contains UTF-16 converted from UTF-8
               PEGASUS_CIM_EXCEPTION(
                  CIM_ERR_INVALID_PARAMETER, e.getMessage()));

            PEG_METHOD_EXIT();
            return;
         }

         // Expect </METHODCALL>

         XmlReader::expectEndTag(parser, "METHODCALL");
      }
      else
      {
            MessageLoaderParms mlParms(
                "Server.CIMOperationRequestDecoder."
                    "EXPECTED_IMETHODCALL_ELEMENT",
                "expected IMETHODCALL or METHODCALL element");

            throw XmlValidationError(parser.getLine(),mlParms);

      }

      // Expect </SIMPLEREQ>

      XmlReader::expectEndTag(parser, "SIMPLEREQ");

      // Expect </MESSAGE>

      XmlReader::expectEndTag(parser, "MESSAGE");

      // Expect </CIM>

      XmlReader::expectEndTag(parser, "CIM");
   }
   catch (XmlValidationError& e)
   {
       PEG_TRACE((TRC_XML,Tracer::LEVEL1,
           "CIMOperationRequestDecoder::handleMethodCall - "
               "XmlValidationError exception has occurred. Message: %s",
           (const char*) e.getMessage().getCString()));

      sendHttpError(queueId, HTTP_STATUS_BADREQUEST, "request-not-valid",
                    e.getMessage());
      PEG_METHOD_EXIT();
      return;
   }
   catch (XmlSemanticError& e)
   {
       PEG_TRACE((TRC_XML,Tracer::LEVEL1,
           "CIMOperationRequestDecoder::handleMethodCall - "
               "XmlSemanticError exception has occurred. Message: %s",
           (const char*) e.getMessage().getCString()));

      // ATTN-RK-P2-20020404: Is this the correct response for these errors?
      sendHttpError(queueId, HTTP_STATUS_BADREQUEST, "request-not-valid",
                    e.getMessage());
      PEG_METHOD_EXIT();
      return;
   }
   catch (XmlException& e)
   {
       PEG_TRACE((TRC_XML,Tracer::LEVEL1,
           "CIMOperationRequestDecoder::handleMethodCall - "
               "XmlException has occurred. Message: %s",
           (const char*) e.getMessage().getCString()));

      sendHttpError(queueId, HTTP_STATUS_BADREQUEST, "request-not-well-formed",
                    e.getMessage());
      PEG_METHOD_EXIT();
      return;
   }
   catch (Exception& e)
   {
      // Don't know why I got this exception.  Seems like a bad thing.
      // Any exceptions we're expecting should be caught separately and
      // dealt with appropriately.  This is a last resort.
      sendHttpError(queueId, HTTP_STATUS_INTERNALSERVERERROR, String::EMPTY,
                    e.getMessage());
      PEG_METHOD_EXIT();
      return;
   }
   catch (...)
   {
      // Don't know why I got whatever this is.  Seems like a bad thing.
      // Any exceptions we're expecting should be caught separately and
      // dealt with appropriately.  This is a last resort.
      sendHttpError(queueId, HTTP_STATUS_INTERNALSERVERERROR);
      PEG_METHOD_EXIT();
      return;
   }

   STAT_BYTESREAD

   request->setHttpMethod (httpMethod);

   ((CIMRequestMessage *) (request.get()))->
       operationContext.insert(WMIMapperUserInfoContainer(password));

//l10n start
// l10n TODO - might want to move A-L and C-L to Message
// to make this more maintainable
    // Add the language headers to the request
    CIMMessage * cimmsg = dynamic_cast<CIMMessage *>(request.get());
    if (cimmsg != NULL)
    {
        cimmsg->operationContext.set(
            AcceptLanguageListContainer(httpAcceptLanguages));
        cimmsg->operationContext.set(
            ContentLanguageListContainer(httpContentLanguages));
        cimmsg->operationContext.insert(IdentityContainer(userName));
    }
    else
    {
        ;    // l10n TODO - error back to client here
    }
// l10n end

   _outputQueue->enqueue(request.release());

   PEG_METHOD_EXIT();
}

CIMCreateClassRequestMessage*
CIMOperationRequestDecoder::decodeCreateClassRequest(
   Uint32 queueId,
   XmlParser& parser,
   const String& messageId,
   const CIMNamespaceName& nameSpace,
   const String& authType,
   const String& userName)
{
   PEG_METHOD_ENTER(TRC_DISPATCHER,
      "CIMOperationRequestDecoder::decodeCreateClassRequest()");

   STAT_GETSTARTTIME

   CIMClass newClass;
   Boolean duplicateParameter = false;
   Boolean gotClass = false;
   Boolean emptyTag;

   for (const char* name;
       XmlReader::getIParamValueTag(parser, name, emptyTag);)
   {
      if (System::strcasecmp(name, "NewClass") == 0)
      {
         XmlReader::rejectNullIParamValue(parser, emptyTag, name);
         XmlReader::getClassElement(parser, newClass);
         duplicateParameter = gotClass;
         gotClass = true;
      }
      else
      {
         PEG_METHOD_EXIT();
     throw PEGASUS_CIM_EXCEPTION(CIM_ERR_NOT_SUPPORTED, String::EMPTY);
      }

      if (!emptyTag)
      {
         XmlReader::expectEndTag(parser, "IPARAMVALUE");
      }

      if (duplicateParameter)
      {
         PEG_METHOD_EXIT();
     throw PEGASUS_CIM_EXCEPTION(CIM_ERR_INVALID_PARAMETER, String::EMPTY);
      }
   }

   if (!gotClass)
   {
      PEG_METHOD_EXIT();
      throw PEGASUS_CIM_EXCEPTION(CIM_ERR_INVALID_PARAMETER, String::EMPTY);
   }

   AutoPtr<CIMCreateClassRequestMessage> request(
       new CIMCreateClassRequestMessage(
          messageId,
          nameSpace,
          newClass,
          QueueIdStack(queueId, _returnQueueId),
          authType,
          userName));

   STAT_SERVERSTART

   PEG_METHOD_EXIT();
   return(request.release());
}

CIMGetClassRequestMessage* CIMOperationRequestDecoder::decodeGetClassRequest(
   Uint32 queueId,
   XmlParser& parser,
   const String& messageId,
   const CIMNamespaceName& nameSpace,
   const String& authType,
   const String& userName)
{
   PEG_METHOD_ENTER(TRC_DISPATCHER,
      "CIMOperationRequestDecoder::decodeGetClassRequest()");

   STAT_GETSTARTTIME

   CIMName className;
   Boolean localOnly = true;
   Boolean includeQualifiers = true;
   Boolean includeClassOrigin = false;
   CIMPropertyList propertyList;
   Boolean duplicateParameter = false;
   Boolean gotClassName = false;
   Boolean gotLocalOnly = false;
   Boolean gotIncludeQualifiers = false;
   Boolean gotIncludeClassOrigin = false;
   Boolean gotPropertyList = false;
   Boolean emptyTag;

   for (const char* name;
       XmlReader::getIParamValueTag(parser, name, emptyTag);)
   {
      if (System::strcasecmp(name, "ClassName") == 0)
      {
         XmlReader::rejectNullIParamValue(parser, emptyTag, name);
         XmlReader::getClassNameElement(parser, className, true);
         duplicateParameter = gotClassName;
         gotClassName = true;
      }
      else if (System::strcasecmp(name, "LocalOnly") == 0)
      {
         XmlReader::rejectNullIParamValue(parser, emptyTag, name);
         XmlReader::getBooleanValueElement(parser, localOnly, true);
         duplicateParameter = gotLocalOnly;
         gotLocalOnly = true;
      }
      else if (System::strcasecmp(name, "IncludeQualifiers") == 0)
      {
         XmlReader::rejectNullIParamValue(parser, emptyTag, name);
         XmlReader::getBooleanValueElement(parser, includeQualifiers, true);
         duplicateParameter = gotIncludeQualifiers;
         gotIncludeQualifiers = true;
      }
      else if (System::strcasecmp(name, "IncludeClassOrigin") == 0)
      {
         XmlReader::rejectNullIParamValue(parser, emptyTag, name);
         XmlReader::getBooleanValueElement(parser, includeClassOrigin, true);
         duplicateParameter = gotIncludeClassOrigin;
         gotIncludeClassOrigin = true;
      }
      else if (System::strcasecmp(name, "PropertyList") == 0)
      {
         if (!emptyTag)
         {
            CIMValue pl;
            if (XmlReader::getValueArrayElement(parser, CIMTYPE_STRING, pl))
            {
               Array<String> propertyListArray;
               pl.get(propertyListArray);
               Array<CIMName> cimNameArray;
               for (Uint32 i = 0; i < propertyListArray.size(); i++)
               {
                   cimNameArray.append(propertyListArray[i]);
               }
               propertyList.set(cimNameArray);
            }
         }
     duplicateParameter = gotPropertyList;
     gotPropertyList = true;
      }
      else
      {
         PEG_METHOD_EXIT();
     throw PEGASUS_CIM_EXCEPTION(CIM_ERR_NOT_SUPPORTED, String::EMPTY);
      }

      if (!emptyTag)
      {
         XmlReader::expectEndTag(parser, "IPARAMVALUE");
      }

      if (duplicateParameter)
      {
         PEG_METHOD_EXIT();
     throw PEGASUS_CIM_EXCEPTION(CIM_ERR_INVALID_PARAMETER, String::EMPTY);
      }
   }

   if (!gotClassName)
   {
      PEG_METHOD_EXIT();
      throw PEGASUS_CIM_EXCEPTION(CIM_ERR_INVALID_PARAMETER, String::EMPTY);
   }

   AutoPtr<CIMGetClassRequestMessage> request(
       new CIMGetClassRequestMessage(
          messageId,
          nameSpace,
          className,
          localOnly,
          includeQualifiers,
          includeClassOrigin,
          propertyList,
          QueueIdStack(queueId, _returnQueueId),
          authType,
          userName));

   STAT_SERVERSTART

   PEG_METHOD_EXIT();
   return(request.release());
}

CIMModifyClassRequestMessage*
CIMOperationRequestDecoder::decodeModifyClassRequest(
   Uint32 queueId,
   XmlParser& parser,
   const String& messageId,
   const CIMNamespaceName& nameSpace,
   const String& authType,
   const String& userName)
{
   STAT_GETSTARTTIME

   CIMClass modifiedClass;
   Boolean duplicateParameter = false;
   Boolean gotClass = false;
   Boolean emptyTag;

   for (const char* name;
       XmlReader::getIParamValueTag(parser, name, emptyTag);)
   {
      if (System::strcasecmp(name, "ModifiedClass") == 0)
      {
         XmlReader::rejectNullIParamValue(parser, emptyTag, name);
     XmlReader::getClassElement(parser, modifiedClass);
     duplicateParameter = gotClass;
     gotClass = true;
      }
      else
      {
     throw PEGASUS_CIM_EXCEPTION(CIM_ERR_NOT_SUPPORTED, String::EMPTY);
      }

      if (!emptyTag)
      {
         XmlReader::expectEndTag(parser, "IPARAMVALUE");
      }

      if (duplicateParameter)
      {
     throw PEGASUS_CIM_EXCEPTION(CIM_ERR_INVALID_PARAMETER, String::EMPTY);
      }
   }

   if (!gotClass)
   {
      throw PEGASUS_CIM_EXCEPTION(CIM_ERR_INVALID_PARAMETER, String::EMPTY);
   }

   AutoPtr<CIMModifyClassRequestMessage> request(
      new CIMModifyClassRequestMessage(
     messageId,
     nameSpace,
     modifiedClass,
     QueueIdStack(queueId, _returnQueueId),
     authType,
     userName));

   STAT_SERVERSTART

   return(request.release());
}

CIMEnumerateClassNamesRequestMessage*
CIMOperationRequestDecoder::decodeEnumerateClassNamesRequest(
   Uint32 queueId,
   XmlParser& parser,
   const String& messageId,
   const CIMNamespaceName& nameSpace,
   const String& authType,
   const String& userName)
{
   STAT_GETSTARTTIME

   CIMName className;
   Boolean deepInheritance = false;
   Boolean duplicateParameter = false;
   Boolean gotClassName = false;
   Boolean gotDeepInheritance = false;
   Boolean emptyTag;

   for (const char* name;
       XmlReader::getIParamValueTag(parser, name, emptyTag);)
   {
      if (System::strcasecmp(name, "ClassName") == 0)
      {
         //
         //  ClassName may be NULL
         //
         if (!emptyTag)
         {
             XmlReader::getClassNameElement(parser, className, false);
         }
     duplicateParameter = gotClassName;
     gotClassName = true;
      }
      else if (System::strcasecmp(name, "DeepInheritance") == 0)
      {
     XmlReader::rejectNullIParamValue(parser, emptyTag, name);
     XmlReader::getBooleanValueElement(parser, deepInheritance, true);
     duplicateParameter = gotDeepInheritance;
     gotDeepInheritance = true;
      }
      else
      {
     throw PEGASUS_CIM_EXCEPTION(CIM_ERR_NOT_SUPPORTED, String::EMPTY);
      }

      if (!emptyTag)
      {
         XmlReader::expectEndTag(parser, "IPARAMVALUE");
      }

      if (duplicateParameter)
      {
     throw PEGASUS_CIM_EXCEPTION(CIM_ERR_INVALID_PARAMETER, String::EMPTY);
      }
   }

   AutoPtr<CIMEnumerateClassNamesRequestMessage> request(
      new CIMEnumerateClassNamesRequestMessage(
     messageId,
     nameSpace,
     className,
     deepInheritance,
     QueueIdStack(queueId, _returnQueueId),
     authType,
     userName));

   STAT_SERVERSTART

   return(request.release());
}

CIMEnumerateClassesRequestMessage*
CIMOperationRequestDecoder::decodeEnumerateClassesRequest(
   Uint32 queueId,
   XmlParser& parser,
   const String& messageId,
   const CIMNamespaceName& nameSpace,
   const String& authType,
   const String& userName)
{
   STAT_GETSTARTTIME

   CIMName className;
   Boolean deepInheritance = false;
   Boolean localOnly = true;
   Boolean includeQualifiers = true;
   Boolean includeClassOrigin = false;
   Boolean duplicateParameter = false;
   Boolean gotClassName = false;
   Boolean gotDeepInheritance = false;
   Boolean gotLocalOnly = false;
   Boolean gotIncludeQualifiers = false;
   Boolean gotIncludeClassOrigin = false;
   Boolean emptyTag;

   for (const char* name;
       XmlReader::getIParamValueTag(parser, name, emptyTag);)
   {
      if (System::strcasecmp(name, "ClassName") == 0)
      {
         //
         //  ClassName may be NULL
         //
         if (!emptyTag)
         {
         XmlReader::getClassNameElement(parser, className, false);
         }
     duplicateParameter = gotClassName;
     gotClassName = true;
      }
      else if (System::strcasecmp(name, "DeepInheritance") == 0)
      {
         XmlReader::rejectNullIParamValue(parser, emptyTag, name);
     XmlReader::getBooleanValueElement(parser, deepInheritance, true);
     duplicateParameter = gotDeepInheritance;
     gotDeepInheritance = true;
      }
      else if (System::strcasecmp(name, "LocalOnly") == 0)
      {
         XmlReader::rejectNullIParamValue(parser, emptyTag, name);
     XmlReader::getBooleanValueElement(parser, localOnly, true);
     duplicateParameter = gotLocalOnly;
     gotLocalOnly = true;
      }
      else if (System::strcasecmp(name, "IncludeQualifiers") == 0)
      {
         XmlReader::rejectNullIParamValue(parser, emptyTag, name);
     XmlReader::getBooleanValueElement(parser, includeQualifiers, true);
     duplicateParameter = gotIncludeQualifiers;
     gotIncludeQualifiers = true;
      }
      else if (System::strcasecmp(name, "IncludeClassOrigin") == 0)
      {
         XmlReader::rejectNullIParamValue(parser, emptyTag, name);
     XmlReader::getBooleanValueElement(parser, includeClassOrigin, true);
     duplicateParameter = gotIncludeClassOrigin;
     gotIncludeClassOrigin = true;
      }
      else
      {
     throw PEGASUS_CIM_EXCEPTION(CIM_ERR_NOT_SUPPORTED, String::EMPTY);
      }

      if (!emptyTag)
      {
         XmlReader::expectEndTag(parser, "IPARAMVALUE");
      }

      if (duplicateParameter)
      {
     throw PEGASUS_CIM_EXCEPTION(CIM_ERR_INVALID_PARAMETER, String::EMPTY);
      }
   }

   AutoPtr<CIMEnumerateClassesRequestMessage> request(
      new CIMEnumerateClassesRequestMessage(
     messageId,
     nameSpace,
     className,
     deepInheritance,
     localOnly,
     includeQualifiers,
     includeClassOrigin,
     QueueIdStack(queueId, _returnQueueId),
     authType,
     userName));

   STAT_SERVERSTART

   return(request.release());
}

CIMDeleteClassRequestMessage*
CIMOperationRequestDecoder::decodeDeleteClassRequest(
   Uint32 queueId,
   XmlParser& parser,
   const String& messageId,
   const CIMNamespaceName& nameSpace,
   const String& authType,
   const String& userName)
{
   STAT_GETSTARTTIME

   CIMName className;
   Boolean duplicateParameter = false;
   Boolean gotClassName = false;
   Boolean emptyTag;

   for (const char* name;
       XmlReader::getIParamValueTag(parser, name, emptyTag);)
   {
      if (System::strcasecmp(name, "ClassName") == 0)
      {
         XmlReader::rejectNullIParamValue(parser, emptyTag, name);
     XmlReader::getClassNameElement(parser, className);
     duplicateParameter = gotClassName;
     gotClassName = true;
      }
      else
      {
     throw PEGASUS_CIM_EXCEPTION(CIM_ERR_NOT_SUPPORTED, String::EMPTY);
      }

      if (!emptyTag)
      {
         XmlReader::expectEndTag(parser, "IPARAMVALUE");
      }

      if (duplicateParameter)
      {
     throw PEGASUS_CIM_EXCEPTION(CIM_ERR_INVALID_PARAMETER, String::EMPTY);
      }
   }

   if (!gotClassName)
   {
      throw PEGASUS_CIM_EXCEPTION(CIM_ERR_INVALID_PARAMETER, String::EMPTY);
   }

   AutoPtr<CIMDeleteClassRequestMessage> request(
       new CIMDeleteClassRequestMessage(
          messageId,
          nameSpace,
          className,
          QueueIdStack(queueId, _returnQueueId),
          authType,
          userName));

   STAT_SERVERSTART

   return(request.release());
}

CIMCreateInstanceRequestMessage*
CIMOperationRequestDecoder::decodeCreateInstanceRequest(
   Uint32 queueId,
   XmlParser& parser,
   const String& messageId,
   const CIMNamespaceName& nameSpace,
   const String& authType,
   const String& userName)
{
   STAT_GETSTARTTIME

   CIMInstance newInstance;
   Boolean duplicateParameter = false;
   Boolean gotInstance = false;
   Boolean emptyTag;

   for (const char* name;
       XmlReader::getIParamValueTag(parser, name, emptyTag);)
   {
      if (System::strcasecmp(name, "NewInstance") == 0)
      {
         XmlReader::rejectNullIParamValue(parser, emptyTag, name);
     XmlReader::getInstanceElement(parser, newInstance);
     duplicateParameter = gotInstance;
     gotInstance = true;
      }
      else
      {
     throw PEGASUS_CIM_EXCEPTION(CIM_ERR_NOT_SUPPORTED, String::EMPTY);
      }

      if (!emptyTag)
      {
         XmlReader::expectEndTag(parser, "IPARAMVALUE");
      }

      if (duplicateParameter)
      {
     throw PEGASUS_CIM_EXCEPTION(CIM_ERR_INVALID_PARAMETER, String::EMPTY);
      }
   }

   if (!gotInstance)
   {
      throw PEGASUS_CIM_EXCEPTION(CIM_ERR_INVALID_PARAMETER, String::EMPTY);
   }

   AutoPtr<CIMCreateInstanceRequestMessage> request(
      new CIMCreateInstanceRequestMessage(
     messageId,
     nameSpace,
     newInstance,
     QueueIdStack(queueId, _returnQueueId),
     authType,
     userName));

   STAT_SERVERSTART

   return(request.release());
}

CIMGetInstanceRequestMessage*
CIMOperationRequestDecoder::decodeGetInstanceRequest(
   Uint32 queueId,
   XmlParser& parser,
   const String& messageId,
   const CIMNamespaceName& nameSpace,
   const String& authType,
   const String& userName)
{
   STAT_GETSTARTTIME

   CIMObjectPath instanceName;
   Boolean localOnly = true;
   Boolean includeQualifiers = false;
   Boolean includeClassOrigin = false;
   CIMPropertyList propertyList;
   Boolean duplicateParameter = false;
   Boolean gotInstanceName = false;
   Boolean gotLocalOnly = false;
   Boolean gotIncludeQualifiers = false;
   Boolean gotIncludeClassOrigin = false;
   Boolean gotPropertyList = false;
   Boolean emptyTag;

   for (const char* name;
       XmlReader::getIParamValueTag(parser, name, emptyTag);)
   {
      if (System::strcasecmp(name, "InstanceName") == 0)
      {
         XmlReader::rejectNullIParamValue(parser, emptyTag, name);
     XmlReader::getInstanceNameElement(parser, instanceName);
     duplicateParameter = gotInstanceName;
     gotInstanceName = true;
      }
      else if (System::strcasecmp(name, "LocalOnly") == 0)
      {
         XmlReader::rejectNullIParamValue(parser, emptyTag, name);
     XmlReader::getBooleanValueElement(parser, localOnly, true);
     duplicateParameter = gotLocalOnly;
     gotLocalOnly = true;
      }
      else if (System::strcasecmp(name, "IncludeQualifiers") == 0)
      {
         XmlReader::rejectNullIParamValue(parser, emptyTag, name);
     XmlReader::getBooleanValueElement(parser, includeQualifiers, true);
     duplicateParameter = gotIncludeQualifiers;
     gotIncludeQualifiers = true;
      }
      else if (System::strcasecmp(name, "IncludeClassOrigin") == 0)
      {
         XmlReader::rejectNullIParamValue(parser, emptyTag, name);
     XmlReader::getBooleanValueElement(parser, includeClassOrigin, true);
     duplicateParameter = gotIncludeClassOrigin;
     gotIncludeClassOrigin = true;
      }
      else if (System::strcasecmp(name, "PropertyList") == 0)
      {
         if (!emptyTag)
         {
            CIMValue pl;
            if (XmlReader::getValueArrayElement(parser, CIMTYPE_STRING, pl))
            {
               Array<String> propertyListArray;
               pl.get(propertyListArray);
               Array<CIMName> cimNameArray;
               for (Uint32 i = 0; i < propertyListArray.size(); i++)
               {
                   cimNameArray.append(propertyListArray[i]);
               }
               propertyList.set(cimNameArray);
            }
         }
     duplicateParameter = gotPropertyList;
     gotPropertyList = true;
      }
      else
      {
     throw PEGASUS_CIM_EXCEPTION(CIM_ERR_NOT_SUPPORTED, String::EMPTY);
      }

      if (!emptyTag)
      {
         XmlReader::expectEndTag(parser, "IPARAMVALUE");
      }

      if (duplicateParameter)
      {
     throw PEGASUS_CIM_EXCEPTION(CIM_ERR_INVALID_PARAMETER, String::EMPTY);
      }
   }

   if (!gotInstanceName)
   {
      throw PEGASUS_CIM_EXCEPTION(CIM_ERR_INVALID_PARAMETER, String::EMPTY);
   }

   AutoPtr<CIMGetInstanceRequestMessage> request(
       new CIMGetInstanceRequestMessage(
          messageId,
          nameSpace,
          instanceName,
          includeQualifiers,
          includeClassOrigin,
          propertyList,
          QueueIdStack(queueId, _returnQueueId),
          authType,
          userName));
   request->localOnly = localOnly;

   STAT_SERVERSTART

   return(request.release());
}

CIMModifyInstanceRequestMessage*
CIMOperationRequestDecoder::decodeModifyInstanceRequest(
   Uint32 queueId,
   XmlParser& parser,
   const String& messageId,
   const CIMNamespaceName& nameSpace,
   const String& authType,
   const String& userName)
{
   STAT_GETSTARTTIME

   CIMInstance modifiedInstance;
   Boolean includeQualifiers = true;
   CIMPropertyList propertyList;
   Boolean duplicateParameter = false;
   Boolean gotInstance = false;
   Boolean gotIncludeQualifiers = false;
   Boolean gotPropertyList = false;
   Boolean emptyTag;

   for (const char* name;
       XmlReader::getIParamValueTag(parser, name, emptyTag);)
   {
      if (System::strcasecmp(name, "ModifiedInstance") == 0)
      {
         XmlReader::rejectNullIParamValue(parser, emptyTag, name);
     XmlReader::getNamedInstanceElement(parser, modifiedInstance);
     duplicateParameter = gotInstance;
     gotInstance = true;
      }
      else if (System::strcasecmp(name, "IncludeQualifiers") == 0)
      {
         XmlReader::rejectNullIParamValue(parser, emptyTag, name);
     XmlReader::getBooleanValueElement(parser, includeQualifiers, true);
     duplicateParameter = gotIncludeQualifiers;
     gotIncludeQualifiers = true;
      }
      else if (System::strcasecmp(name, "PropertyList") == 0)
      {
         if (!emptyTag)
         {
            CIMValue pl;
            if (XmlReader::getValueArrayElement(parser, CIMTYPE_STRING, pl))
            {
               Array<String> propertyListArray;
               pl.get(propertyListArray);
               Array<CIMName> cimNameArray;
               for (Uint32 i = 0; i < propertyListArray.size(); i++)
               {
                   cimNameArray.append(propertyListArray[i]);
               }
               propertyList.set(cimNameArray);
            }
         }
     duplicateParameter = gotPropertyList;
     gotPropertyList = true;
      }
      else
      {
     throw PEGASUS_CIM_EXCEPTION(CIM_ERR_NOT_SUPPORTED, String::EMPTY);
      }

      if (!emptyTag)
      {
         XmlReader::expectEndTag(parser, "IPARAMVALUE");
      }

      if (duplicateParameter)
      {
     throw PEGASUS_CIM_EXCEPTION(CIM_ERR_INVALID_PARAMETER, String::EMPTY);
      }
   }

   if (!gotInstance)
   {
      throw PEGASUS_CIM_EXCEPTION(CIM_ERR_INVALID_PARAMETER, String::EMPTY);
   }

   AutoPtr<CIMModifyInstanceRequestMessage> request(
      new CIMModifyInstanceRequestMessage(
     messageId,
     nameSpace,
     modifiedInstance,
     includeQualifiers,
     propertyList,
     QueueIdStack(queueId, _returnQueueId),
     authType,
     userName));

   STAT_SERVERSTART

   return(request.release());
}

CIMEnumerateInstancesRequestMessage*
CIMOperationRequestDecoder::decodeEnumerateInstancesRequest(
   Uint32 queueId,
   XmlParser& parser,
   const String& messageId,
   const CIMNamespaceName& nameSpace,
   const String& authType,
   const String& userName)
{
   STAT_GETSTARTTIME

   CIMName className;
   Boolean deepInheritance = true;
   Boolean localOnly = true;
   Boolean includeQualifiers = false;
   Boolean includeClassOrigin = false;
   CIMPropertyList propertyList;
   Boolean duplicateParameter = false;
   Boolean gotClassName = false;
   Boolean gotDeepInheritance = false;
   Boolean gotLocalOnly = false;
   Boolean gotIncludeQualifiers = false;
   Boolean gotIncludeClassOrigin = false;
   Boolean gotPropertyList = false;
   Boolean emptyTag;

   for (const char* name;
       XmlReader::getIParamValueTag(parser, name, emptyTag);)
   {
      if (System::strcasecmp(name, "ClassName") == 0)
      {
         XmlReader::rejectNullIParamValue(parser, emptyTag, name);
     XmlReader::getClassNameElement(parser, className, true);
     duplicateParameter = gotClassName;
     gotClassName = true;
      }
      else if (System::strcasecmp(name, "DeepInheritance") == 0)
      {
         XmlReader::rejectNullIParamValue(parser, emptyTag, name);
     XmlReader::getBooleanValueElement(parser, deepInheritance, true);
     duplicateParameter = gotDeepInheritance;
     gotDeepInheritance = true;
      }
      else if (System::strcasecmp(name, "LocalOnly") == 0)
      {
         XmlReader::rejectNullIParamValue(parser, emptyTag, name);
     XmlReader::getBooleanValueElement(parser, localOnly, true);
     duplicateParameter = gotLocalOnly;
     gotLocalOnly = true;
      }
      else if (System::strcasecmp(name, "IncludeQualifiers") == 0)
      {
         XmlReader::rejectNullIParamValue(parser, emptyTag, name);
     XmlReader::getBooleanValueElement(parser, includeQualifiers, true);
     duplicateParameter = gotIncludeQualifiers;
     gotIncludeQualifiers = true;
      }
      else if (System::strcasecmp(name, "IncludeClassOrigin") == 0)
      {
         XmlReader::rejectNullIParamValue(parser, emptyTag, name);
     XmlReader::getBooleanValueElement(parser, includeClassOrigin, true);
     duplicateParameter = gotIncludeClassOrigin;
     gotIncludeClassOrigin = true;
      }
      else if (System::strcasecmp(name, "PropertyList") == 0)
      {
         if (!emptyTag)
         {
            CIMValue pl;
            if (XmlReader::getValueArrayElement(parser, CIMTYPE_STRING, pl))
            {
               Array<String> propertyListArray;
               pl.get(propertyListArray);
               Array<CIMName> cimNameArray;
               for (Uint32 i = 0; i < propertyListArray.size(); i++)
               {
                   cimNameArray.append(propertyListArray[i]);
               }
               propertyList.set(cimNameArray);
            }
         }
     duplicateParameter = gotPropertyList;
     gotPropertyList = true;
      }
      else
      {
     throw PEGASUS_CIM_EXCEPTION(CIM_ERR_NOT_SUPPORTED, String::EMPTY);
      }

      if (!emptyTag)
      {
         XmlReader::expectEndTag(parser, "IPARAMVALUE");
      }

      if (duplicateParameter)
      {
     throw PEGASUS_CIM_EXCEPTION(CIM_ERR_INVALID_PARAMETER, String::EMPTY);
      }
   }

   if (!gotClassName)
   {
      throw PEGASUS_CIM_EXCEPTION(CIM_ERR_INVALID_PARAMETER, String::EMPTY);
   }

   AutoPtr<CIMEnumerateInstancesRequestMessage> request(
      new CIMEnumerateInstancesRequestMessage(
     messageId,
     nameSpace,
     className,
     deepInheritance,
     includeQualifiers,
     includeClassOrigin,
     propertyList,
     QueueIdStack(queueId, _returnQueueId),
     authType,
     userName));
   request->localOnly = localOnly;

   STAT_SERVERSTART

   return(request.release());
}

CIMEnumerateInstanceNamesRequestMessage*
CIMOperationRequestDecoder::decodeEnumerateInstanceNamesRequest(
   Uint32 queueId,
   XmlParser& parser,
   const String& messageId,
   const CIMNamespaceName& nameSpace,
   const String& authType,
   const String& userName)
{
   STAT_GETSTARTTIME

   CIMName className;
   Boolean duplicateParameter = false;
   Boolean gotClassName = false;
   Boolean emptyTag;

   for (const char* name;
       XmlReader::getIParamValueTag(parser, name, emptyTag);)
   {
      if (System::strcasecmp(name, "ClassName") == 0)
      {
         XmlReader::rejectNullIParamValue(parser, emptyTag, name);
     XmlReader::getClassNameElement(parser, className, true);
     duplicateParameter = gotClassName;
     gotClassName = true;
      }
      else
      {
     throw PEGASUS_CIM_EXCEPTION(CIM_ERR_NOT_SUPPORTED, String::EMPTY);
      }

      if (!emptyTag)
      {
         XmlReader::expectEndTag(parser, "IPARAMVALUE");
      }

      if (duplicateParameter)
      {
     throw PEGASUS_CIM_EXCEPTION(CIM_ERR_INVALID_PARAMETER, String::EMPTY);
      }
   }

   if (!gotClassName)
   {
      throw PEGASUS_CIM_EXCEPTION(CIM_ERR_INVALID_PARAMETER, String::EMPTY);
   }

   AutoPtr<CIMEnumerateInstanceNamesRequestMessage> request(
      new CIMEnumerateInstanceNamesRequestMessage(
     messageId,
     nameSpace,
     className,
     QueueIdStack(queueId, _returnQueueId),
     authType,
     userName));

   STAT_SERVERSTART

   return(request.release());
}

CIMDeleteInstanceRequestMessage*
CIMOperationRequestDecoder::decodeDeleteInstanceRequest(
   Uint32 queueId,
   XmlParser& parser,
   const String& messageId,
   const CIMNamespaceName& nameSpace,
   const String& authType,
   const String& userName)
{
   STAT_GETSTARTTIME

   CIMObjectPath instanceName;
   Boolean duplicateParameter = false;
   Boolean gotInstanceName = false;
   Boolean emptyTag;

   for (const char* name;
       XmlReader::getIParamValueTag(parser, name, emptyTag);)
   {
      if (System::strcasecmp(name, "InstanceName") == 0)
      {
         XmlReader::rejectNullIParamValue(parser, emptyTag, name);
     XmlReader::getInstanceNameElement(parser, instanceName);
     duplicateParameter = gotInstanceName;
     gotInstanceName = true;
      }
      else
      {
     throw PEGASUS_CIM_EXCEPTION(CIM_ERR_NOT_SUPPORTED, String::EMPTY);
      }

      if (!emptyTag)
      {
         XmlReader::expectEndTag(parser, "IPARAMVALUE");
      }

      if (duplicateParameter)
      {
     throw PEGASUS_CIM_EXCEPTION(CIM_ERR_INVALID_PARAMETER, String::EMPTY);
      }
   }

   if (!gotInstanceName)
   {
      throw PEGASUS_CIM_EXCEPTION(CIM_ERR_INVALID_PARAMETER, String::EMPTY);
   }

   AutoPtr<CIMDeleteInstanceRequestMessage> request(
       new CIMDeleteInstanceRequestMessage(
          messageId,
          nameSpace,
          instanceName,
          QueueIdStack(queueId, _returnQueueId),
          authType,
          userName));

   STAT_SERVERSTART

   return(request.release());
}

CIMSetQualifierRequestMessage*
CIMOperationRequestDecoder::decodeSetQualifierRequest(
   Uint32 queueId,
   XmlParser& parser,
   const String& messageId,
   const CIMNamespaceName& nameSpace,
   const String& authType,
   const String& userName)
{
   STAT_GETSTARTTIME

   CIMQualifierDecl qualifierDeclaration;
   Boolean duplicateParameter = false;
   Boolean gotQualifierDeclaration = false;
   Boolean emptyTag;

   for (const char* name;
       XmlReader::getIParamValueTag(parser, name, emptyTag);)
   {
      if (System::strcasecmp(name, "QualifierDeclaration") == 0)
      {
         XmlReader::rejectNullIParamValue(parser, emptyTag, name);
     XmlReader::getQualifierDeclElement(parser, qualifierDeclaration);
     duplicateParameter = gotQualifierDeclaration;
     gotQualifierDeclaration = true;
      }
      else
      {
     throw PEGASUS_CIM_EXCEPTION(CIM_ERR_NOT_SUPPORTED, String::EMPTY);
      }

      if (!emptyTag)
      {
         XmlReader::expectEndTag(parser, "IPARAMVALUE");
      }

      if (duplicateParameter)
      {
     throw PEGASUS_CIM_EXCEPTION(CIM_ERR_INVALID_PARAMETER, String::EMPTY);
      }
   }

   if (!gotQualifierDeclaration)
   {
      throw PEGASUS_CIM_EXCEPTION(CIM_ERR_INVALID_PARAMETER, String::EMPTY);
   }

   AutoPtr<CIMSetQualifierRequestMessage> request(
      new CIMSetQualifierRequestMessage(
     messageId,
     nameSpace,
     qualifierDeclaration,
     QueueIdStack(queueId, _returnQueueId),
     authType,
     userName));

   STAT_SERVERSTART

   return(request.release());
}

CIMGetQualifierRequestMessage*
CIMOperationRequestDecoder::decodeGetQualifierRequest(
   Uint32 queueId,
   XmlParser& parser,
   const String& messageId,
   const CIMNamespaceName& nameSpace,
   const String& authType,
   const String& userName)
{
   STAT_GETSTARTTIME

   String qualifierNameString;
   CIMName qualifierName;
   Boolean duplicateParameter = false;
   Boolean gotQualifierName = false;
   Boolean emptyTag;

   for (const char* name;
       XmlReader::getIParamValueTag(parser, name, emptyTag);)
   {
      if (System::strcasecmp(name, "QualifierName") == 0)
      {
         XmlReader::rejectNullIParamValue(parser, emptyTag, name);
     XmlReader::getStringValueElement(parser, qualifierNameString, true);
     qualifierName = qualifierNameString;
     duplicateParameter = gotQualifierName;
     gotQualifierName = true;
      }
      else
      {
     throw PEGASUS_CIM_EXCEPTION(CIM_ERR_NOT_SUPPORTED, String::EMPTY);
      }

      if (!emptyTag)
      {
         XmlReader::expectEndTag(parser, "IPARAMVALUE");
      }

      if (duplicateParameter)
      {
     throw PEGASUS_CIM_EXCEPTION(CIM_ERR_INVALID_PARAMETER, String::EMPTY);
      }
   }

   if (!gotQualifierName)
   {
      throw PEGASUS_CIM_EXCEPTION(CIM_ERR_INVALID_PARAMETER, String::EMPTY);
   }

   AutoPtr<CIMGetQualifierRequestMessage> request(
      new CIMGetQualifierRequestMessage(
     messageId,
     nameSpace,
     qualifierName,
     QueueIdStack(queueId, _returnQueueId),
     authType,
     userName));

   STAT_SERVERSTART

   return(request.release());
}

CIMEnumerateQualifiersRequestMessage*
CIMOperationRequestDecoder::decodeEnumerateQualifiersRequest(
   Uint32 queueId,
   XmlParser& parser,
   const String& messageId,
   const CIMNamespaceName& nameSpace,
   const String& authType,
   const String& userName)
{
   STAT_GETSTARTTIME
   Boolean emptyTag;

   for (const char* name;
       XmlReader::getIParamValueTag(parser, name, emptyTag);)
   {
      // No IPARAMVALUEs are defined for this operation
      throw PEGASUS_CIM_EXCEPTION(CIM_ERR_NOT_SUPPORTED, String::EMPTY);
   }

   AutoPtr<CIMEnumerateQualifiersRequestMessage> request(
      new CIMEnumerateQualifiersRequestMessage(
     messageId,
     nameSpace,
     QueueIdStack(queueId, _returnQueueId),
     authType,
     userName));

   STAT_SERVERSTART

   return(request.release());
}

CIMDeleteQualifierRequestMessage*
CIMOperationRequestDecoder::decodeDeleteQualifierRequest(
   Uint32 queueId,
   XmlParser& parser,
   const String& messageId,
   const CIMNamespaceName& nameSpace,
   const String& authType,
   const String& userName)
{
   STAT_GETSTARTTIME

   String qualifierNameString;
   CIMName qualifierName;
   Boolean duplicateParameter = false;
   Boolean gotQualifierName = false;
   Boolean emptyTag;

   for (const char* name;
       XmlReader::getIParamValueTag(parser, name, emptyTag);)
   {
      if (System::strcasecmp(name, "QualifierName") == 0)
      {
         XmlReader::rejectNullIParamValue(parser, emptyTag, name);
     XmlReader::getStringValueElement(parser, qualifierNameString, true);
     qualifierName = qualifierNameString;
     duplicateParameter = gotQualifierName;
     gotQualifierName = true;
      }
      else
      {
     throw PEGASUS_CIM_EXCEPTION(CIM_ERR_NOT_SUPPORTED, String::EMPTY);
      }

      if (!emptyTag)
      {
         XmlReader::expectEndTag(parser, "IPARAMVALUE");
      }

      if (duplicateParameter)
      {
     throw PEGASUS_CIM_EXCEPTION(CIM_ERR_INVALID_PARAMETER, String::EMPTY);
      }
   }

   if (!gotQualifierName)
   {
      throw PEGASUS_CIM_EXCEPTION(CIM_ERR_INVALID_PARAMETER, String::EMPTY);
   }

   AutoPtr<CIMDeleteQualifierRequestMessage> request(
      new CIMDeleteQualifierRequestMessage(
     messageId,
     nameSpace,
     qualifierName,
     QueueIdStack(queueId, _returnQueueId),
     authType,
     userName));

   STAT_SERVERSTART

   return(request.release());
}

CIMReferenceNamesRequestMessage*
CIMOperationRequestDecoder::decodeReferenceNamesRequest(
   Uint32 queueId,
   XmlParser& parser,
   const String& messageId,
   const CIMNamespaceName& nameSpace,
   const String& authType,
   const String& userName)
{
   STAT_GETSTARTTIME

   CIMObjectPath objectName;
   CIMName resultClass;
   String role;
   Boolean duplicateParameter = false;
   Boolean gotObjectName = false;
   Boolean gotResultClass = false;
   Boolean gotRole = false;
   Boolean emptyTag;
   Boolean isClassNameElement;

   for (const char* name;
       XmlReader::getIParamValueTag(parser, name, emptyTag);)
   {
      if (System::strcasecmp(name, "ObjectName") == 0)
      {
         XmlReader::rejectNullIParamValue(parser, emptyTag, name);
         isClassNameElement = XmlReader::getObjectNameElement(parser,
             objectName);
         duplicateParameter = gotObjectName;
         gotObjectName = true;
      }
      else if (System::strcasecmp(name, "ResultClass") == 0)
      {
         //
         //  ResultClass may be NULL
         //
         if (!emptyTag)
         {
            XmlReader::getClassNameElement(parser, resultClass, false);
         }
         duplicateParameter = gotResultClass;
         gotResultClass = true;
      }
      else if (System::strcasecmp(name, "Role") == 0)
      {
         //
         //  Role may be NULL
         //
         if (!emptyTag)
         {
            XmlReader::getStringValueElement(parser, role, false);
         }
         duplicateParameter = gotRole;
         gotRole = true;
      }
      else
      {
         throw PEGASUS_CIM_EXCEPTION(CIM_ERR_NOT_SUPPORTED, String::EMPTY);
      }

      if (!emptyTag)
      {
         XmlReader::expectEndTag(parser, "IPARAMVALUE");
      }

      if (duplicateParameter)
      {
         throw PEGASUS_CIM_EXCEPTION(CIM_ERR_INVALID_PARAMETER, String::EMPTY);
      }
   }

   if (!gotObjectName)
   {
      throw PEGASUS_CIM_EXCEPTION(CIM_ERR_INVALID_PARAMETER, String::EMPTY);
   }

   AutoPtr<CIMReferenceNamesRequestMessage> request(
      new CIMReferenceNamesRequestMessage(
         messageId,
         nameSpace,
         objectName,
         resultClass,
         role,
         QueueIdStack(queueId, _returnQueueId),
         isClassNameElement,
         authType,
         userName));

   STAT_SERVERSTART

   return(request.release());
}

CIMReferencesRequestMessage*
CIMOperationRequestDecoder::decodeReferencesRequest(
   Uint32 queueId,
   XmlParser& parser,
   const String& messageId,
   const CIMNamespaceName& nameSpace,
   const String& authType,
   const String& userName)
{
   STAT_GETSTARTTIME

   CIMObjectPath objectName;
   CIMName resultClass;
   String role;
   Boolean includeQualifiers = false;
   Boolean includeClassOrigin = false;
   CIMPropertyList propertyList;
   Boolean duplicateParameter = false;
   Boolean gotObjectName = false;
   Boolean gotResultClass = false;
   Boolean gotRole = false;
   Boolean gotIncludeQualifiers = false;
   Boolean gotIncludeClassOrigin = false;
   Boolean gotPropertyList = false;
   Boolean emptyTag;
   Boolean isClassNameElement;

   for (const char* name;
       XmlReader::getIParamValueTag(parser, name, emptyTag);)
   {
      if (System::strcasecmp(name, "ObjectName") == 0)
      {
         XmlReader::rejectNullIParamValue(parser, emptyTag, name);
         isClassNameElement = XmlReader::getObjectNameElement(parser,
             objectName);
         duplicateParameter = gotObjectName;
         gotObjectName = true;
      }
      else if (System::strcasecmp(name, "ResultClass") == 0)
      {
         //
         //  ResultClass may be NULL
         //
         if (!emptyTag)
         {
            XmlReader::getClassNameElement(parser, resultClass, false);
         }
      duplicateParameter = gotResultClass;
      gotResultClass = true;
      }
      else if (System::strcasecmp(name, "Role") == 0)
      {
         //
         //  Role may be NULL
         //
         if (!emptyTag)
         {
            XmlReader::getStringValueElement(parser, role, false);
         }
     duplicateParameter = gotRole;
     gotRole = true;
      }
      else if (System::strcasecmp(name, "IncludeQualifiers") == 0)
      {
         XmlReader::rejectNullIParamValue(parser, emptyTag, name);
     XmlReader::getBooleanValueElement(parser, includeQualifiers, true);
     duplicateParameter = gotIncludeQualifiers;
     gotIncludeQualifiers = true;
      }
      else if (System::strcasecmp(name, "IncludeClassOrigin") == 0)
      {
         XmlReader::rejectNullIParamValue(parser, emptyTag, name);
     XmlReader::getBooleanValueElement(parser, includeClassOrigin, true);
     duplicateParameter = gotIncludeClassOrigin;
     gotIncludeClassOrigin = true;
      }
      else if (System::strcasecmp(name, "PropertyList") == 0)
      {
         if (!emptyTag)
         {
            CIMValue pl;
            if (XmlReader::getValueArrayElement(parser, CIMTYPE_STRING, pl))
            {
               Array<String> propertyListArray;
               pl.get(propertyListArray);
               Array<CIMName> cimNameArray;
               for (Uint32 i = 0; i < propertyListArray.size(); i++)
               {
                   cimNameArray.append(propertyListArray[i]);
               }
               propertyList.set(cimNameArray);
            }
         }
     duplicateParameter = gotPropertyList;
     gotPropertyList = true;
      }
      else
      {
     throw PEGASUS_CIM_EXCEPTION(CIM_ERR_NOT_SUPPORTED, String::EMPTY);
      }

      if (!emptyTag)
      {
         XmlReader::expectEndTag(parser, "IPARAMVALUE");
      }

      if (duplicateParameter)
      {
     throw PEGASUS_CIM_EXCEPTION(CIM_ERR_INVALID_PARAMETER, String::EMPTY);
      }
   }

   if (!gotObjectName)
   {
      throw PEGASUS_CIM_EXCEPTION(CIM_ERR_INVALID_PARAMETER, String::EMPTY);
   }

   AutoPtr<CIMReferencesRequestMessage> request(
      new CIMReferencesRequestMessage(
         messageId,
         nameSpace,
         objectName,
         resultClass,
         role,
         includeQualifiers,
         includeClassOrigin,
         propertyList,
         QueueIdStack(queueId, _returnQueueId),
         isClassNameElement,
         authType,
         userName));

   STAT_SERVERSTART

   return(request.release());
}

CIMAssociatorNamesRequestMessage*
CIMOperationRequestDecoder::decodeAssociatorNamesRequest(
   Uint32 queueId,
   XmlParser& parser,
   const String& messageId,
   const CIMNamespaceName& nameSpace,
   const String& authType,
   const String& userName)
{
   STAT_GETSTARTTIME

   CIMObjectPath objectName;
   CIMName assocClass;
   CIMName resultClass;
   String role;
   String resultRole;
   Boolean duplicateParameter = false;
   Boolean gotObjectName = false;
   Boolean gotAssocClass = false;
   Boolean gotResultClass = false;
   Boolean gotRole = false;
   Boolean gotResultRole = false;
   Boolean emptyTag;
   Boolean isClassNameElement;

   for (const char* name;
       XmlReader::getIParamValueTag(parser, name, emptyTag);)
   {
      if (System::strcasecmp(name, "ObjectName") == 0)
      {
         XmlReader::rejectNullIParamValue(parser, emptyTag, name);
         isClassNameElement = XmlReader::getObjectNameElement(parser,
             objectName);
         duplicateParameter = gotObjectName;
         gotObjectName = true;
      }
      else if (System::strcasecmp(name, "AssocClass") == 0)
      {
         //
         //  AssocClass may be NULL
         //
         if (!emptyTag)
         {
            XmlReader::getClassNameElement(parser, assocClass, false);
         }
         duplicateParameter = gotAssocClass;
         gotAssocClass = true;
      }
      else if (System::strcasecmp(name, "ResultClass") == 0)
      {
         //
         //  ResultClass may be NULL
         //
         if (!emptyTag)
         {
            XmlReader::getClassNameElement(parser, resultClass, false);
         }
         duplicateParameter = gotResultClass;
         gotResultClass = true;
      }
      else if (System::strcasecmp(name, "Role") == 0)
      {
         //
         //  Role may be NULL
         //
         if (!emptyTag)
         {
            XmlReader::getStringValueElement(parser, role, false);
         }
         duplicateParameter = gotRole;
         gotRole = true;
      }
      else if (System::strcasecmp(name, "ResultRole") == 0)
      {
         //
         //  ResultRole may be NULL
         //
         if (!emptyTag)
         {
            XmlReader::getStringValueElement(parser, resultRole, false);
         }
         duplicateParameter = gotResultRole;
         gotResultRole = true;
      }
      else
      {
          throw PEGASUS_CIM_EXCEPTION(CIM_ERR_NOT_SUPPORTED, String::EMPTY);
      }

      if (!emptyTag)
      {
         XmlReader::expectEndTag(parser, "IPARAMVALUE");
      }

      if (duplicateParameter)
      {
          throw PEGASUS_CIM_EXCEPTION(CIM_ERR_INVALID_PARAMETER, String::EMPTY);
      }
   }

   if (!gotObjectName)
   {
      throw PEGASUS_CIM_EXCEPTION(CIM_ERR_INVALID_PARAMETER, String::EMPTY);
   }

   AutoPtr<CIMAssociatorNamesRequestMessage> request(
      new CIMAssociatorNamesRequestMessage(
     messageId,
     nameSpace,
     objectName,
     assocClass,
     resultClass,
     role,
     resultRole,
     QueueIdStack(queueId, _returnQueueId),
     isClassNameElement,
     authType,
     userName));

   STAT_SERVERSTART

   return(request.release());
}

CIMAssociatorsRequestMessage*
CIMOperationRequestDecoder::decodeAssociatorsRequest(
   Uint32 queueId,
   XmlParser& parser,
   const String& messageId,
   const CIMNamespaceName& nameSpace,
   const String& authType,
   const String& userName)
{
   STAT_GETSTARTTIME

   CIMObjectPath objectName;
   CIMName assocClass;
   CIMName resultClass;
   String role;
   String resultRole;
   Boolean includeQualifiers = false;
   Boolean includeClassOrigin = false;
   CIMPropertyList propertyList;
   Boolean duplicateParameter = false;
   Boolean gotObjectName = false;
   Boolean gotAssocClass = false;
   Boolean gotResultClass = false;
   Boolean gotRole = false;
   Boolean gotResultRole = false;
   Boolean gotIncludeQualifiers = false;
   Boolean gotIncludeClassOrigin = false;
   Boolean gotPropertyList = false;
   Boolean emptyTag;
   Boolean isClassNameElement;

   for (const char* name;
       XmlReader::getIParamValueTag(parser, name, emptyTag);)
   {
      if (System::strcasecmp(name, "ObjectName") == 0)
      {
         XmlReader::rejectNullIParamValue(parser, emptyTag, name);
         isClassNameElement =XmlReader::getObjectNameElement(parser,
             objectName);
         duplicateParameter = gotObjectName;
         gotObjectName = true;
      }
      else if (System::strcasecmp(name, "AssocClass") == 0)
      {
         //
         //  AssocClass may be NULL
         //
         if (!emptyTag)
         {
            XmlReader::getClassNameElement(parser, assocClass, false);
         }
     duplicateParameter = gotAssocClass;
     gotAssocClass = true;
      }
      else if (System::strcasecmp(name, "ResultClass") == 0)
      {
         //
         //  ResultClass may be NULL
         //
         if (!emptyTag)
         {
            XmlReader::getClassNameElement(parser, resultClass, false);
         }
     duplicateParameter = gotResultClass;
     gotResultClass = true;
      }
      else if (System::strcasecmp(name, "Role") == 0)
      {
         //
         //  Role may be NULL
         //
         if (!emptyTag)
         {
            XmlReader::getStringValueElement(parser, role, false);
         }
     duplicateParameter = gotRole;
     gotRole = true;
      }
      else if (System::strcasecmp(name, "ResultRole") == 0)
      {
         //
         //  ResultRole may be NULL
         //
         if (!emptyTag)
         {
            XmlReader::getStringValueElement(parser, resultRole, false);
         }
     duplicateParameter = gotResultRole;
     gotResultRole = true;
      }
      else if (System::strcasecmp(name, "IncludeQualifiers") == 0)
      {
         XmlReader::rejectNullIParamValue(parser, emptyTag, name);
     XmlReader::getBooleanValueElement(parser, includeQualifiers, true);
     duplicateParameter = gotIncludeQualifiers;
     gotIncludeQualifiers = true;
      }
      else if (System::strcasecmp(name, "IncludeClassOrigin") == 0)
      {
         XmlReader::rejectNullIParamValue(parser, emptyTag, name);
     XmlReader::getBooleanValueElement(parser, includeClassOrigin, true);
     duplicateParameter = gotIncludeClassOrigin;
     gotIncludeClassOrigin = true;
      }
      else if (System::strcasecmp(name, "PropertyList") == 0)
      {
         if (!emptyTag)
         {
            CIMValue pl;
            if (XmlReader::getValueArrayElement(parser, CIMTYPE_STRING, pl))
            {
               Array<String> propertyListArray;
               pl.get(propertyListArray);
               Array<CIMName> cimNameArray;
               for (Uint32 i = 0; i < propertyListArray.size(); i++)
               {
                   cimNameArray.append(propertyListArray[i]);
               }
               propertyList.set(cimNameArray);
            }
         }
     duplicateParameter = gotPropertyList;
     gotPropertyList = true;
      }
      else
      {
     throw PEGASUS_CIM_EXCEPTION(CIM_ERR_NOT_SUPPORTED, String::EMPTY);
      }

      if (!emptyTag)
      {
         XmlReader::expectEndTag(parser, "IPARAMVALUE");
      }

      if (duplicateParameter)
      {
     throw PEGASUS_CIM_EXCEPTION(CIM_ERR_INVALID_PARAMETER, String::EMPTY);
      }
   }

   if (!gotObjectName)
   {
      throw PEGASUS_CIM_EXCEPTION(CIM_ERR_INVALID_PARAMETER, String::EMPTY);
   }

   AutoPtr<CIMAssociatorsRequestMessage> request(
      new CIMAssociatorsRequestMessage(
     messageId,
     nameSpace,
     objectName,
     assocClass,
     resultClass,
     role,
     resultRole,
     includeQualifiers,
     includeClassOrigin,
     propertyList,
     QueueIdStack(queueId, _returnQueueId),
     isClassNameElement,
     authType,
     userName));

   STAT_SERVERSTART

   return(request.release());
}

CIMGetPropertyRequestMessage*
CIMOperationRequestDecoder::decodeGetPropertyRequest(
   Uint32 queueId,
   XmlParser& parser,
   const String& messageId,
   const CIMNamespaceName& nameSpace,
   const String& authType,
   const String& userName)
{
   STAT_GETSTARTTIME

   CIMObjectPath instanceName;
   String propertyName;
   Boolean duplicateParameter = false;
   Boolean gotInstanceName = false;
   Boolean gotPropertyName = false;
   Boolean emptyTag;

   for (const char* name;
       XmlReader::getIParamValueTag(parser, name, emptyTag);)
   {
      if (System::strcasecmp(name, "InstanceName") == 0)
      {
         XmlReader::rejectNullIParamValue(parser, emptyTag, name);
     XmlReader::getInstanceNameElement(parser, instanceName);
     duplicateParameter = gotInstanceName;
     gotInstanceName = true;
      }
      else if (System::strcasecmp(name, "PropertyName") == 0)
      {
         XmlReader::rejectNullIParamValue(parser, emptyTag, name);
     XmlReader::getStringValueElement(parser, propertyName, true);
     duplicateParameter = gotPropertyName;
     gotPropertyName = true;
      }
      else
      {
     throw PEGASUS_CIM_EXCEPTION(CIM_ERR_NOT_SUPPORTED, String::EMPTY);
      }

      if (!emptyTag)
      {
         XmlReader::expectEndTag(parser, "IPARAMVALUE");
      }

      if (duplicateParameter)
      {
     throw PEGASUS_CIM_EXCEPTION(CIM_ERR_INVALID_PARAMETER, String::EMPTY);
      }
   }

   if (!gotInstanceName || !gotPropertyName)
   {
      throw PEGASUS_CIM_EXCEPTION(CIM_ERR_INVALID_PARAMETER, String::EMPTY);
   }

   AutoPtr<CIMGetPropertyRequestMessage> request(
       new CIMGetPropertyRequestMessage(
          messageId,
          nameSpace,
          instanceName,
          propertyName,
          QueueIdStack(queueId, _returnQueueId),
          authType,
          userName));

   STAT_SERVERSTART

   return(request.release());
}

CIMSetPropertyRequestMessage*
CIMOperationRequestDecoder::decodeSetPropertyRequest(
   Uint32 queueId,
   XmlParser& parser,
   const String& messageId,
   const CIMNamespaceName& nameSpace,
   const String& authType,
   const String& userName)
{
   STAT_GETSTARTTIME

   CIMObjectPath instanceName;
   String propertyName;
   CIMValue propertyValue;
   Boolean duplicateParameter = false;
   Boolean gotInstanceName = false;
   Boolean gotPropertyName = false;
   Boolean gotNewValue = false;
   Boolean emptyTag;

   for (const char* name;
       XmlReader::getIParamValueTag(parser, name, emptyTag);)
   {
      if (System::strcasecmp(name, "InstanceName") == 0)
      {
         XmlReader::rejectNullIParamValue(parser, emptyTag, name);
     XmlReader::getInstanceNameElement(parser, instanceName);
     duplicateParameter = gotInstanceName;
     gotInstanceName = true;
      }
      else if (System::strcasecmp(name, "PropertyName") == 0)
      {
         XmlReader::rejectNullIParamValue(parser, emptyTag, name);
     XmlReader::getStringValueElement(parser, propertyName, true);
     duplicateParameter = gotPropertyName;
     gotPropertyName = true;
      }
      else if (System::strcasecmp(name, "NewValue") == 0)
      {
         if (emptyTag || !XmlReader::getPropertyValue(parser, propertyValue))
         {
        propertyValue.setNullValue(CIMTYPE_STRING, false);
         }
     duplicateParameter = gotNewValue;
     gotNewValue = true;
      }
      else
      {
     throw PEGASUS_CIM_EXCEPTION(CIM_ERR_NOT_SUPPORTED, String::EMPTY);
      }

      if (!emptyTag)
      {
         XmlReader::expectEndTag(parser, "IPARAMVALUE");
      }

      if (duplicateParameter)
      {
     throw PEGASUS_CIM_EXCEPTION(CIM_ERR_INVALID_PARAMETER, String::EMPTY);
      }
   }

   if (!gotInstanceName || !gotPropertyName)
   {
      throw PEGASUS_CIM_EXCEPTION(CIM_ERR_INVALID_PARAMETER, String::EMPTY);
   }

   AutoPtr<CIMSetPropertyRequestMessage> request(
       new CIMSetPropertyRequestMessage(
          messageId,
          nameSpace,
          instanceName,
          propertyName,
          propertyValue,
          QueueIdStack(queueId, _returnQueueId),
          authType,
          userName));

   STAT_SERVERSTART

   return(request.release());
}

CIMExecQueryRequestMessage* CIMOperationRequestDecoder::decodeExecQueryRequest(
   Uint32 queueId,
   XmlParser& parser,
   const String& messageId,
   const CIMNamespaceName& nameSpace,
   const String& authType,
   const String& userName)
{
   STAT_GETSTARTTIME

   String queryLanguage;
   String query;
   Boolean duplicateParameter = false;
   Boolean gotQueryLanguage = false;
   Boolean gotQuery = false;
   Boolean emptyTag;

   for (const char* name;
       XmlReader::getIParamValueTag(parser, name, emptyTag);)
   {
      if (System::strcasecmp(name, "QueryLanguage") == 0)
      {
         XmlReader::rejectNullIParamValue(parser, emptyTag, name);
     XmlReader::getStringValueElement(parser, queryLanguage, true);
     duplicateParameter = gotQueryLanguage;
     gotQueryLanguage = true;
      }
      else if (System::strcasecmp(name, "Query") == 0)
      {
         XmlReader::rejectNullIParamValue(parser, emptyTag, name);
     XmlReader::getStringValueElement(parser, query, true);
     duplicateParameter = gotQuery;
     gotQuery = true;
      }
      else
      {
     throw PEGASUS_CIM_EXCEPTION(CIM_ERR_NOT_SUPPORTED, String::EMPTY);
      }

      if (!emptyTag)
      {
         XmlReader::expectEndTag(parser, "IPARAMVALUE");
      }

      if (duplicateParameter)
      {
     throw PEGASUS_CIM_EXCEPTION(CIM_ERR_INVALID_PARAMETER, String::EMPTY);
      }
   }

   if (!gotQueryLanguage || !gotQuery)
   {
      throw PEGASUS_CIM_EXCEPTION(CIM_ERR_INVALID_PARAMETER, String::EMPTY);
   }

   AutoPtr<CIMExecQueryRequestMessage> request(
      new CIMExecQueryRequestMessage(
     messageId,
     nameSpace,
     queryLanguage,
     query,
     QueueIdStack(queueId, _returnQueueId),
     authType,
     userName));

   STAT_SERVERSTART

   return(request.release());
}

CIMInvokeMethodRequestMessage*
CIMOperationRequestDecoder::decodeInvokeMethodRequest(
   Uint32 queueId,
   XmlParser& parser,
   const String& messageId,
   const CIMObjectPath& reference,
   const String& cimMethodName,
   const String& authType,
   const String& userName)
{
   STAT_GETSTARTTIME

   CIMParamValue paramValue;
   Array<CIMParamValue> inParameters;

   while (XmlReader::getParamValueElement(parser, paramValue))
   {
      inParameters.append(paramValue);
   }

   AutoPtr<CIMInvokeMethodRequestMessage> request(
      new CIMInvokeMethodRequestMessage(
     messageId,
     reference.getNameSpace(),
     reference,
     cimMethodName,
     inParameters,
     QueueIdStack(queueId, _returnQueueId),
     authType,
     userName));

   STAT_SERVERSTART

   return(request.release());
}

void CIMOperationRequestDecoder::setServerTerminating(Boolean flag)
{
   _serverTerminating = flag;
}

PEGASUS_NAMESPACE_END
