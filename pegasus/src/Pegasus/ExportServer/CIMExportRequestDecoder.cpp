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
#include <Pegasus/Common/Tracer.h>
#include <cctype>
#include <cstdio>
#include <Pegasus/Common/Constants.h>
#include <Pegasus/Common/XmlParser.h>
#include <Pegasus/Common/XmlReader.h>
#include <Pegasus/Common/XmlWriter.h>
#include <Pegasus/Common/System.h>
#include <Pegasus/Common/Logger.h>
#include "CIMExportRequestDecoder.h"
#include <Pegasus/Common/CommonUTF.h>
#include <Pegasus/Common/MessageLoader.h>
#include <Pegasus/Common/LanguageParser.h>

PEGASUS_USING_STD;

PEGASUS_NAMESPACE_BEGIN

CIMExportRequestDecoder::CIMExportRequestDecoder(
    MessageQueueService* outputQueue,
    Uint32 returnQueueId)
    : Base(PEGASUS_QUEUENAME_EXPORTREQDECODER),
      _outputQueue(outputQueue),
      _returnQueueId(returnQueueId),
      _serverTerminating(false)
{
}

CIMExportRequestDecoder::~CIMExportRequestDecoder()
{
}

void CIMExportRequestDecoder::sendResponse(
    Uint32 queueId,
    Buffer& message,
    Boolean closeConnect)
{
    MessageQueue* queue = MessageQueue::lookup(queueId);

    if (queue)
    {
        HTTPMessage* httpMessage = new HTTPMessage(message);
        httpMessage->setCloseConnect(closeConnect);
        queue->enqueue(httpMessage);
    }
}

void CIMExportRequestDecoder::sendEMethodError(
    Uint32 queueId,
    HttpMethod httpMethod,
    const String& messageId,
    const String& eMethodName,
    const CIMException& cimException,
    Boolean closeConnect)
{
    Buffer message;
    message = XmlWriter::formatSimpleEMethodErrorRspMessage(
        eMethodName,
        messageId,
        httpMethod,
        cimException);

    sendResponse(queueId, message,closeConnect);
}

void CIMExportRequestDecoder::sendHttpError(
    Uint32 queueId,
    const String& status,
    const String& cimError,
    const String& messageBody,
    Boolean closeConnect)
{
    Buffer message;
    message = XmlWriter::formatHttpErrorRspMessage(
        status,
        cimError,
        messageBody);

    sendResponse(queueId, message,closeConnect);
}

void CIMExportRequestDecoder::handleEnqueue(Message* message)
{
    PEGASUS_ASSERT(message != 0);

    switch (message->getType())
    {
        case HTTP_MESSAGE:
            handleHTTPMessage((HTTPMessage*)message);
            break;

        default:
            PEGASUS_UNREACHABLE(PEGASUS_ASSERT(0);)
            break;
    }

    delete message;
}


void CIMExportRequestDecoder::handleEnqueue()
{
    Message* message = dequeue();
    if (message)
        handleEnqueue(message);
}

//------------------------------------------------------------------------------
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
//     73-CIMExport: MethodRequest
//     73-CIMExportMethod: ExportIndication
//     73-CIMObject: root/cimv2
//
//------------------------------------------------------------------------------

void CIMExportRequestDecoder::handleHTTPMessage(HTTPMessage* httpMessage)
{
    PEGASUS_ASSERT(httpMessage->message.size() != 0);

    // Save queueId and userName

    Uint32 queueId = httpMessage->queueId;
    String userName = httpMessage->authInfo->getAuthenticatedUser();

    Boolean closeConnect = httpMessage->getCloseConnect();
    PEG_TRACE((
        TRC_HTTP,
        Tracer::LEVEL4,
        "CIMOperationRequestDecoder::handleHTTPMessage() -"
             " httpMessage->getCloseConnect() returned %d",
        httpMessage->getCloseConnect()));

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
    HttpMethod httpMethod = HTTP_METHOD__POST;

    // ATTN-RK-P3-20020404: The requestUri may need to be pruned of the host
    // name.  All we care about at this point is the path.
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
    //<Bug #351>
    //PEGASUS_ASSERT(methodName == "M-POST" || methodName == "POST");
    if (methodName != "M-POST" && methodName != "POST")
    {
        sendHttpError(
            queueId,
            HTTP_STATUS_NOTIMPLEMENTED,
            "Only POST and M-POST are implemented",
            String::EMPTY,
            closeConnect);
        return;
    }
    //</bug>
    //
    // Not true: "Mismatch of method and version is caught in
    // HTTPAuthenticatorDelegator", bug #351 fixes this:
    //
    //PEGASUS_ASSERT (!((httpMethod == HTTP_METHOD_M_POST) &&
    //                  (httpVersion == "HTTP/1.0")));
    if ((httpMethod == HTTP_METHOD_M_POST) &&
         (httpVersion == "HTTP/1.0"))
    {
        sendHttpError(
            queueId,
            HTTP_STATUS_BADREQUEST,
            "M-POST method is not valid with version 1.0",
            String::EMPTY,
            closeConnect);
        return;
    }
    //</bug>

    // Process M-POST and POST messages:

    if (httpVersion == "HTTP/1.1")
    {
        // Validate the presence of a "Host" header.  The HTTP/1.1 specification
        // says this in section 14.23 regarding the Host header field:
        //
        //     All Internet-based HTTP/1.1 servers MUST respond with a 400 (Bad
        //     Request) status code to any HTTP/1.1 request message which lacks
        //     a Host header field.
        //
        // Note:  The Host header value is not validated.

        const char* hostHeader;
        Boolean hostHeaderFound = HTTPMessage::lookupHeader(
            headers, "Host", hostHeader, false);

        if (!hostHeaderFound)
        {
            MessageLoaderParms parms(
                "ExportServer.CIMExportRequestDecoder.MISSING_HOST_HEADER",
                "HTTP request message lacks a Host header field.");
            String msg(MessageLoader::getMessage(parms));
            sendHttpError(
                queueId,
                HTTP_STATUS_BADREQUEST,
                "",
                msg,
                closeConnect);
            return;
        }
    }

    // Validate the "CIMExport" header:

    const char* cimExport;
    Boolean exportHeaderFound = HTTPMessage::lookupHeader(
        headers, "CIMExport", cimExport, true);
    // If the CIMExport header was missing, the HTTPAuthenticatorDelegator
    // would not have passed the message to us.

    // <bug #351>
    // PEGASUS_ASSERT(exportHeaderFound);
    if (!exportHeaderFound)
    {
        sendHttpError(
             queueId,
             HTTP_STATUS_BADREQUEST,
             "Export header not found",
             String::EMPTY,
             closeConnect);
         return;
    }
    // </bug>

    if (System::strcasecmp(cimExport, "MethodRequest") != 0)
    {
        // The Specification for CIM Operations over HTTP reads:
        //     3.3.5. CIMExport
        //     If a CIM Listener receives CIM Export request with this
        //     header, but with a missing value or a value that is not
        //     "MethodRequest", then it MUST fail the request with
        //     status "400 Bad Request". The CIM Server MUST include a
        //     CIMError header in the response with a value of
        //     unsupported-operation.
        sendHttpError(
            queueId,
            HTTP_STATUS_BADREQUEST,
            "unsupported-operation",
            String::EMPTY,
            closeConnect);
        return;
    }

    // Validate the "CIMExportBatch" header:

    const char* cimExportBatch;
    if (HTTPMessage::lookupHeader(
            headers, "CIMExportBatch", cimExportBatch, true))
    {
        // The Specification for CIM Operations over HTTP reads:
        //     3.3.10. CIMExportBatch
        //     If a CIM Listener receives CIM Export Request for which the
        //     CIMExportBatch header is present, but the Listener does not
        //     support Multiple Exports, then it MUST fail the request and
        //     return a status of "501 Not Implemented".
        sendHttpError(
            queueId,
            HTTP_STATUS_NOTIMPLEMENTED,
            "multiple-requests-unsupported",
            String::EMPTY,
            closeConnect);
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

    const char* cimExportMethod;
    if (!HTTPMessage::lookupHeader(
            headers, "CIMExportMethod", cimExportMethod, true))
    {
        // The CIMExportMethod header is not present and we already know the
        // Export Request Message is a Simple Export Request.
        sendHttpError(
            queueId,
            HTTP_STATUS_BADREQUEST,
            "header-mismatch",
            String::EMPTY,
            closeConnect);
        return;
    }

    AcceptLanguageList acceptLanguages;
    ContentLanguageList contentLanguages;
    try
    {
        if (httpMessage->acceptLanguagesDecoded)
        {
            acceptLanguages = httpMessage->acceptLanguages;
        }
        else
        {
            // Get and validate the Accept-Language header, if set
            String acceptLanguageHeader;
            if (HTTPMessage::lookupHeader(
                    headers,
                    "Accept-Language",
                    acceptLanguageHeader,
                    false))
            {
                acceptLanguages = LanguageParser::parseAcceptLanguageHeader(
                    acceptLanguageHeader);
            }
        }

        if (httpMessage->contentLanguagesDecoded)
        {
            contentLanguages = httpMessage->contentLanguages;
        }
        else
        {
            // Get and validate the Content-Language header, if set
            String contentLanguageHeader;
            if (HTTPMessage::lookupHeader(
                    headers,
                    "Content-Language",
                    contentLanguageHeader,
                    false))
            {
                contentLanguages = LanguageParser::parseContentLanguageHeader(
                    contentLanguageHeader);
            }
        }
    }
    catch (Exception& e)
    {
        Thread::clearLanguages();
        MessageLoaderParms msgParms(
            "ExportServer.CIMExportRequestDecoder.REQUEST_NOT_VALID",
            "request-not-valid");
        String msg(MessageLoader::getMessage(msgParms));
        sendHttpError(
            queueId,
            HTTP_STATUS_BADREQUEST,
            msg,
            e.getMessage(),
            closeConnect);
        return;
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
        sendHttpError(
            queueId,
            HTTP_STATUS_BADREQUEST,
            "",
            "HTTP Content-Type header error.",
            closeConnect);
        return;
    }
    else
    {
        // Validating content falls within UTF8 (required to be complaint
        // with section C12 of Unicode 4.0 spec, chapter 3.)
        Uint32 count = 0;
        while(count<contentLength)
        {
            if (!(isUTF8((char *)&content[count])))
            {
                sendHttpError(
                    queueId,
                    HTTP_STATUS_BADREQUEST,
                    "request-not-valid",
                    "Invalid UTF-8 character detected.",
                    closeConnect);
                return;
            }
            UTF8_NEXT(content,count);
        }
    }

    // If it is a method call, then dispatch it to be handled:

    handleMethodRequest(
        queueId,
        httpMethod,
        content,
        requestUri,
        cimProtocolVersion,
        cimExportMethod,
        userName,
        httpMessage->ipAddress,
        acceptLanguages,
        contentLanguages,
        closeConnect);
}

void CIMExportRequestDecoder::handleMethodRequest(
    Uint32 queueId,
    HttpMethod httpMethod,
    char* content,
    const String& requestUri,
    const char* cimProtocolVersionInHeader,
    const char* cimExportMethodInHeader,
    const String& userName,
    const String& ipAddress,
    const AcceptLanguageList& httpAcceptLanguages,
    const ContentLanguageList& httpContentLanguages,
    Boolean closeConnect)
{
    // Set the Accept-Language into the thread for this service.
    // This will allow all code in this thread to get
    // the languages for the messages returned to the client.
    Thread::setLanguages(httpAcceptLanguages);

    //
    // If CIM Listener is shutting down, return error response
    //
    if (_serverTerminating)
    {
        sendHttpError(
            queueId,
            HTTP_STATUS_SERVICEUNAVAILABLE,
            String::EMPTY,
            "CIM Listener is shutting down.",
            closeConnect);
        return;
    }

    // Create a parser:

    XmlParser parser(content);
    XmlEntry entry;
    String messageId;
    const char* cimExportMethodName = "";
    AutoPtr<CIMExportIndicationRequestMessage> request;

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

        if (!XmlReader::isSupportedCIMVersion(cimVersion))
        {
            sendHttpError(
                queueId,
                HTTP_STATUS_NOTIMPLEMENTED,
                "unsupported-cim-version",
                String::EMPTY,
                closeConnect);
            return;
        }

        if (!XmlReader::isSupportedDTDVersion(dtdVersion))
        {
            sendHttpError(
                queueId,
                HTTP_STATUS_NOTIMPLEMENTED,
                "unsupported-dtd-version",
                String::EMPTY,
                closeConnect);
            return;
        }

        // Expect <MESSAGE ...>

        String protocolVersion;
        if (!XmlReader::getMessageStartTag(
                parser, messageId, protocolVersion))
        {
            MessageLoaderParms mlParms(
                "ExportServer.CIMExportRequestDecoder.EXPECTED_MESSAGE_ELEMENT",
                "expected MESSAGE element");

            throw XmlValidationError(parser.getLine(), mlParms);
        }

        // Validate that the protocol version in the header matches the XML

        if (!String::equalNoCase(protocolVersion, cimProtocolVersionInHeader))
        {
            sendHttpError(
                queueId,
                HTTP_STATUS_BADREQUEST,
                "header-mismatch",
                String::EMPTY,
                closeConnect);
            return;
        }

        if (!XmlReader::isSupportedProtocolVersion(protocolVersion))
        {
            // See Specification for CIM Operations over HTTP section 4.3
            sendHttpError(
                queueId,
                HTTP_STATUS_NOTIMPLEMENTED,
                "unsupported-protocol-version",
                String::EMPTY,
                closeConnect);
            return;
        }

        if (XmlReader::testStartTag(parser, entry, "MULTIEXPREQ"))
        {
            // We wouldn't have gotten here if CIMExportBatch header was
            // specified, so this must be indicative of a header mismatch
            sendHttpError(
                queueId,
                HTTP_STATUS_BADREQUEST,
                "header-mismatch",
                String::EMPTY,
                closeConnect);
            return;
            // Future: When MULTIEXPREQ is supported, must ensure
            // CIMExportMethod header is absent, and CIMExportBatch header
            // is present.
        }

        // Expect <SIMPLEEXPREQ ...>

        XmlReader::expectStartTag(parser, entry, "SIMPLEEXPREQ");

        // Expect <EXPMETHODCALL ...>

        if (!XmlReader::getEMethodCallStartTag(parser, cimExportMethodName))
        {
            MessageLoaderParms mlParms(
                "ExportServer.CIMExportRequestDecoder."
                    "EXPECTED_EXPMETHODCALL_ELEMENT",
                "expected EXPMETHODCALL element");

            throw XmlValidationError(parser.getLine(), mlParms);
        }

        // The Specification for CIM Operations over HTTP reads:
        //     3.3.9. CIMExportMethod
        //
        //     This header MUST be present in any CIM Export Request
        //     message that contains a Simple Export Request.
        //
        //     It MUST NOT be present in any CIM Export Response message,
        //     nor in any CIM Export Request message that is not a
        //     Simple Export Request. It MUST NOT be present in any CIM
        //     Operation Request or Response message.
        //
        //     The name of the CIM export method within a Simple Export
        //     Request is defined to be the value of the NAME attribute
        //     of the <EXPMETHODCALL> element.
        //
        //     If a CIM Listener receives a CIM Export Request for which
        //     either:
        //
        //     - The CIMExportMethod header is present but has an invalid
        //       value, or;
        //     - The CIMExportMethod header is not present but the Export
        //       Request Message is a Simple Export Request, or;
        //     - The CIMExportMethod header is present but the Export
        //       Request Message is not a Simple Export Request, or;
        //     - The CIMExportMethod header is present, the Export Request
        //       Message is a Simple Export Request, but the CIMIdentifier
        //       value (when unencoded) does not match the unique method
        //       name within the Simple Export Request,
        //
        //     then it MUST fail the request and return a status of
        //     "400 Bad Request" (and MUST include a CIMError header in the
        //     response with a value of header-mismatch), subject to the
        //     considerations specified in Errors.
        if (System::strcasecmp(
                cimExportMethodName, cimExportMethodInHeader) != 0)
        {
            // ATTN-RK-P3-20020404: How to decode cimExportMethodInHeader?
            sendHttpError(
                queueId,
                HTTP_STATUS_BADREQUEST,
                "header-mismatch",
                String::EMPTY,
                closeConnect);
            return;
        }

        // This try block only catches CIMExceptions, because they must be
        // responded to with a proper EMETHODRESPONSE.  Other exceptions are
        // caught in the outer try block.
        try
        {
            // Delegate to appropriate method to handle:

            if (System::strcasecmp(
                    cimExportMethodName, "ExportIndication") == 0)
            {
               request.reset(decodeExportIndicationRequest(
                   queueId, parser, messageId, requestUri));
            }
            else
            {
                throw PEGASUS_CIM_EXCEPTION_L(CIM_ERR_NOT_SUPPORTED,
                    MessageLoaderParms(
                        "ExportServer.CIMExportRequestDecoder."
                            "UNRECOGNIZED_EXPORT_METHOD",
                        "Unrecognized export method: $0",
                        cimExportMethodName));
            }
        }
        catch (CIMException& e)
        {
            sendEMethodError(
                queueId,
                httpMethod,
                messageId,
                cimExportMethodName,
                e,
                closeConnect);

            return;
        }

        // Expect </EXPMETHODCALL>

        XmlReader::expectEndTag(parser, "EXPMETHODCALL");

        // Expect </SIMPLEEXPREQ>

        XmlReader::expectEndTag(parser, "SIMPLEEXPREQ");

        // Expect </MESSAGE>

        XmlReader::expectEndTag(parser, "MESSAGE");

        // Expect </CIM>

        XmlReader::expectEndTag(parser, "CIM");
    }
    catch (XmlValidationError& e)
    {
        PEG_TRACE((TRC_XML,Tracer::LEVEL1,
            "CIMExportRequestDecoder::handleMethodRequest - "
            "XmlValidationError exception has occurred. Message: %s",
            (const char*) e.getMessage().getCString()));

        sendHttpError(
            queueId,
            HTTP_STATUS_BADREQUEST,
            "request-not-valid",
            e.getMessage(),
            closeConnect);
        return;
    }
    catch (XmlSemanticError& e)
    {
        PEG_TRACE((TRC_XML,Tracer::LEVEL1,
            "CIMExportRequestDecoder::handleMethodRequest - "
            "XmlSemanticError exception has occurred. Message: %s",
            (const char*) e.getMessage().getCString()));
        // ATTN-RK-P2-20020404: Is this the correct response for these errors?
        sendHttpError(
            queueId,
            HTTP_STATUS_BADREQUEST,
            "request-not-valid",
            e.getMessage(),
            closeConnect);
        return;
    }
    catch (XmlException& e)
    {
        PEG_TRACE((TRC_XML,Tracer::LEVEL1,
            "CIMExportRequestDecoder::handleMethodRequest - "
            "XmlException has occurred. Message: %s",
            (const char*) e.getMessage().getCString()));

        sendHttpError(
            queueId,
            HTTP_STATUS_BADREQUEST,
            "request-not-well-formed",
            e.getMessage(),
            closeConnect);
        return;
    }
    catch (Exception& e)
    {
        // Don't know why I got this exception.  Seems like a bad thing.
        // Any exceptions we're expecting should be caught separately and
        // dealt with appropriately.  This is a last resort.
        sendHttpError(
            queueId,
            HTTP_STATUS_INTERNALSERVERERROR,
            String::EMPTY,
            e.getMessage(),
            closeConnect);
        return;
    }
    catch (...)
    {
        // Don't know why I got whatever this is.  Seems like a bad thing.
        // Any exceptions we're expecting should be caught separately and
        // dealt with appropriately.  This is a last resort.
        sendHttpError(
            queueId,
            HTTP_STATUS_INTERNALSERVERERROR,
            String::EMPTY,
            String::EMPTY,
            closeConnect);
        return;
    }

// l10n TODO - might want to move A-L and C-L to Message
// to make this more maintainable
    // Add the language headers to the request.
    // Note: Since the text of an export error response will be ignored
    // by the export client, ignore Accept-Language in the export request.
    // This will cause any export error response message to be sent in the
    // default language.
    request->operationContext.insert(IdentityContainer(userName));
    request->operationContext.set(
        ContentLanguageListContainer(httpContentLanguages));
    request->operationContext.set(
        AcceptLanguageListContainer(AcceptLanguageList()));

    request->ipAddress = ipAddress;

    request->setCloseConnect(closeConnect);

    _outputQueue->enqueue(request.release());
}

CIMExportIndicationRequestMessage*
CIMExportRequestDecoder::decodeExportIndicationRequest(
    Uint32 queueId,
    XmlParser& parser,
    const String& messageId,
    const String& requestUri)
{
    CIMInstance instanceName;

    String destStr = requestUri.subString(
        requestUri.find("/CIMListener") + 12, PEG_NOT_FOUND);

    for (const char* name; XmlReader::getEParamValueTag(parser, name);)
    {
        if (System::strcasecmp(name, "NewIndication") == 0)
        {
            XmlReader::getInstanceElement(parser, instanceName);
        }
        else
        {
            MessageLoaderParms mlParms(
                "ExportServer.CIMExportRequestDecoder."
                    "UNRECOGNIZED_EXPPARAMVALUE_NAME",
                "Unrecognized EXPPARAMVALUE Name $0", name);
            throw PEGASUS_CIM_EXCEPTION(CIM_ERR_NOT_SUPPORTED, mlParms);
        }

        XmlReader::expectEndTag(parser, "EXPPARAMVALUE");
    }

    CIMExportIndicationRequestMessage* request =
        new CIMExportIndicationRequestMessage(
            messageId,
            destStr,
            instanceName,
            QueueIdStack(queueId, _returnQueueId));

    return request;
}

void CIMExportRequestDecoder::setServerTerminating(Boolean flag)
{
    _serverTerminating = flag;
}

PEGASUS_NAMESPACE_END
