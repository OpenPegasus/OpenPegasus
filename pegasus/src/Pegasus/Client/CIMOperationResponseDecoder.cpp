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
#include <iostream>
#include <Pegasus/Common/Constants.h>
#include <Pegasus/Common/XmlParser.h>
#include <Pegasus/Common/XmlReader.h>
#include <Pegasus/Common/System.h>
#include <Pegasus/Common/XmlWriter.h>
#include <Pegasus/Common/HTTPMessage.h>
#include <Pegasus/Common/CIMMessage.h>
#include <Pegasus/Common/Exception.h>
#include <Pegasus/Common/BinaryCodec.h>
#include "CIMOperationResponseDecoder.h"
#include "CIMClientRep.h"

#include <Pegasus/Common/MessageLoader.h>

PEGASUS_USING_STD;

PEGASUS_NAMESPACE_BEGIN

CIMOperationResponseDecoder::CIMOperationResponseDecoder(
    MessageQueue* outputQueue,
    MessageQueue* encoderQueue,
    ClientAuthenticator* authenticator)
    :
    MessageQueue(PEGASUS_QUEUENAME_OPRESPDECODER),
    _outputQueue(outputQueue),
    _encoderQueue(encoderQueue),
    _authenticator(authenticator)
{
}

CIMOperationResponseDecoder::~CIMOperationResponseDecoder()
{
}

void  CIMOperationResponseDecoder::setEncoderQueue(MessageQueue* encoderQueue)
{
    _encoderQueue = encoderQueue;
}

void CIMOperationResponseDecoder::handleEnqueue()
{
    Message* message = dequeue();

    if (!message)
        return;

    switch (message->getType())
    {
        case HTTP_MESSAGE:
        {
            HTTPMessage* httpMessage = (HTTPMessage*)message;
            _handleHTTPMessage(httpMessage);
            break;
        }

        default:
            PEGASUS_UNREACHABLE(PEGASUS_ASSERT(0);)
            break;
    }

    delete message;
}

void CIMOperationResponseDecoder::setDataStorePointer(
    ClientPerfDataStore* perfDataStore_ptr)
{   dataStore = perfDataStore_ptr;
}

void CIMOperationResponseDecoder::_handleHTTPMessage(HTTPMessage* httpMessage)
{
    //
    // Parse the HTTP message:
    //
    TimeValue networkEndTime = TimeValue::getCurrentTime();

    String  startLine;
    Array<HTTPHeader> headers;
    const char* content;
    Uint32  contentLength;
    Boolean cimReconnect=false;

    if (httpMessage->message.size() == 0)
    {
        MessageLoaderParms mlParms(
            "Client.CIMOperationResponseDecoder.EMPTY_RESPONSE",
            "Connection closed by CIM Server.");
        String mlString(MessageLoader::getMessage(mlParms));

        CIMClientMalformedHTTPException* malformedHTTPException =
            new CIMClientMalformedHTTPException(mlString);

        ClientExceptionMessage * response =
            new ClientExceptionMessage(malformedHTTPException);

       //reconnect and resend next request
        response->setCloseConnect(true);

        _outputQueue->enqueue(response);
        return;
    }


    httpMessage->parse(startLine, headers, contentLength);

    //
    // Check for Connection: Close
    //
    const char* connectClose;
    if (HTTPMessage::lookupHeader(headers, "Connection", connectClose, false))
    {
        if (System::strcasecmp(connectClose, "Close") == 0)
        {
            //reconnect and then resend next request.
            cimReconnect=true;
        }
    }
    //
    // Get the status line info
    //
    String httpVersion;
    Uint32 statusCode;
    String reasonPhrase;

    Boolean parsableMessage = HTTPMessage::parseStatusLine(
        startLine, httpVersion, statusCode, reasonPhrase);
    if (!parsableMessage)
    {
        MessageLoaderParms mlParms(
            "Client.CIMOperationResponseDecoder.MALFORMED_RESPONSE",
            "Malformed HTTP response message.");
        String mlString(MessageLoader::getMessage(mlParms));

        CIMClientMalformedHTTPException* malformedHTTPException =
            new CIMClientMalformedHTTPException(mlString);

        ClientExceptionMessage * response =
            new ClientExceptionMessage(malformedHTTPException);

        response->setCloseConnect(cimReconnect);

        _outputQueue->enqueue(response);
        return;
    }

    if (ClientTrace::displayOutput(ClientTrace::TRACE_CON))    {
        cout << "CIMOperatonResponseDecoder";
        httpMessage->printAll(cout);
    }
    if (ClientTrace::displayOutput(ClientTrace::TRACE_LOG))
    {
        Logger::put(Logger::STANDARD_LOG,
            "CIMCLient",
            Logger::INFORMATION,
            "CIMOperationRequestDecoder::Response, XML content: $0",
            httpMessage->message.getData());
    }

    try
    {
        if (_authenticator->checkResponseHeaderForChallenge(headers))
        {
            // If we had a cookie, it must have been a wrong one
            _authenticator->clearCookie();

            //
            // Get the original request, put that in the encoder's queue for
            // re-sending with authentication challenge response.
            //
            Message* reqMessage = _authenticator->releaseRequestMessage();

            if (cimReconnect == true)
            {
                reqMessage->setCloseConnect(cimReconnect);
                _outputQueue->enqueue(reqMessage);
            }
            else
            {
                _encoderQueue->enqueue(reqMessage);
            }

            return;
        }
        else
        {
            //
            // Received a valid/error response from the server.
            // We do not need the original request message anymore, hence
            // delete the request message by getting the handle from the
            // ClientAuthenticator.
            //
            Message* reqMessage = _authenticator->releaseRequestMessage();
            delete reqMessage;
        }
    }
    catch (InvalidAuthHeader& e)
    {
        CIMClientMalformedHTTPException* malformedHTTPException =
            new CIMClientMalformedHTTPException(e.getMessage());
        ClientExceptionMessage * response =
            new ClientExceptionMessage(malformedHTTPException);

        response->setCloseConnect(cimReconnect);
        _outputQueue->enqueue(response);
        return;
    }

    // Store incoming cookie, if present.
    _authenticator->parseCookie(headers);

    // We have the response.  If authentication failed, we will generate a
    // CIMClientHTTPErrorException below with the "401 Unauthorized" status
    // in the (re-challenge) response.

    //
    // Check for a success (200 OK) response
    //
    if (statusCode != HTTP_STATUSCODE_OK)
    {
        String cimError;
        String pegasusError;

        HTTPMessage::lookupHeader(headers, "CIMError", cimError, true);
        HTTPMessage::lookupHeader(
            headers, PEGASUS_HTTPHEADERTAG_ERRORDETAIL, pegasusError);
        try
        {
            pegasusError = XmlReader::decodeURICharacters(pegasusError);
        }
        catch (ParseError&)
        {
            // Ignore this exception.  We're more interested in having the
            // message in encoded form than knowing that the format is invalid.
        }

        CIMClientHTTPErrorException* httpError =
            new CIMClientHTTPErrorException(statusCode, reasonPhrase,
                                            cimError, pegasusError);
        ClientExceptionMessage * response =
            new ClientExceptionMessage(httpError);

        response->setCloseConnect(cimReconnect);
        _outputQueue->enqueue(response);
        return;
    }

    //
    // Search for "CIMOperation" header:
    //
    const char* cimOperation;

    if (!HTTPMessage::lookupHeader(
        headers, "CIMOperation", cimOperation, true))
    {
        MessageLoaderParms mlParms(
            "Client.CIMOperationResponseDecoder.MISSING_CIMOP_HEADER",
            "Missing CIMOperation HTTP header");
        String mlString(MessageLoader::getMessage(mlParms));

        CIMClientMalformedHTTPException* malformedHTTPException =
            new CIMClientMalformedHTTPException(mlString);

        ClientExceptionMessage * response =
            new ClientExceptionMessage(malformedHTTPException);

        response->setCloseConnect(cimReconnect);

        _outputQueue->enqueue(response);
        return;
    }

    //
    // Search for "Content-Type" header:
    //

    // BUG 572, Use of Content-Type header and change error msg.
    // If header exists, test type.  If not, ignore. We will find
    // content type errors in text analysis.
    // content-type header  value format:
    //              type "/" subtype *( ";" parameter )
    // ex. text/xml;Charset="utf8"
    const char* cimContentType;
    bool binaryResponse = false;

    if (HTTPMessage::lookupHeader(
            headers, "Content-Type", cimContentType, true))
    {
        String type;
        String charset;

        if (!HTTPMessage::parseContentTypeHeader(
                cimContentType, type, charset) ||
            (((!String::equalNoCase(type, "application/xml") &&
              !String::equalNoCase(type, "text/xml")) ||
             !String::equalNoCase(charset, "utf-8"))
            && !(binaryResponse=String::equalNoCase(
                type, "application/x-openpegasus"))
        ))
        {
            CIMClientMalformedHTTPException* malformedHTTPException = new
                CIMClientMalformedHTTPException(
                    "Bad Content-Type HTTP header; " + String(cimContentType));
            ClientExceptionMessage * response =
                new ClientExceptionMessage(malformedHTTPException);

            response->setCloseConnect(cimReconnect);

            _outputQueue->enqueue(response);
            return;
        }
    }
    // comment out the error rejection code if the content-type header does
    //    not exist
#ifdef PEGASUS_REJECT_ON_MISSING_CONTENTTYPE_HEADER
    else
    {
        CIMClientMalformedHTTPException* malformedHTTPException = new
            CIMClientMalformedHTTPException
                ("Missing Content-Type HTTP header; ");
        ClientExceptionMessage * response =
            new ClientExceptionMessage(malformedHTTPException);

        response->setCloseConnect(cimReconnect);

        _outputQueue->enqueue(response);
        return;
    }
#endif

    // look for any cim status codes. The HTTPConnection level would have
    // added them here.

    const char* cimStatusCodeValue;
    Boolean found = HTTPMessage::lookupHeader(headers, "CIMStatusCode",
        cimStatusCodeValue, true);
    CIMStatusCode cimStatusCodeNumber = CIM_ERR_SUCCESS;

    if (found &&
        (cimStatusCodeNumber = (CIMStatusCode) atoi(cimStatusCodeValue)) !=
             CIM_ERR_SUCCESS)
    {
        String cimStatusCodeDescription;
        found = HTTPMessage::lookupHeader(headers, "CIMStatusCodeDescription",
            cimStatusCodeDescription, true);
        if (found && cimStatusCodeDescription.size() > 0)
        {
            try
            {
                cimStatusCodeDescription =
                    XmlReader::decodeURICharacters(cimStatusCodeDescription);
            }
            catch (ParseError&)
            {
            }
        } // if there is a description with the code

        CIMException* cimStatusException =
            new CIMException(cimStatusCodeNumber,cimStatusCodeDescription);
        cimStatusException->setContentLanguages(httpMessage->contentLanguages);
        ClientExceptionMessage * response =
            new ClientExceptionMessage(cimStatusException);

        response->setCloseConnect(cimReconnect);

        _outputQueue->enqueue(response);
        return;
    }

    const char* serverTime;
    if (HTTPMessage::lookupHeader(
            headers, "WBEMServerResponseTime", serverTime, true))
    {
        Uint32 sTime = (Uint32) atol(serverTime);
        dataStore->setServerTime(sTime);
    }


    // Calculate the beginning of the content from the message size and
    // the content length.
    if (binaryResponse)
    {
        // binary the "Content" also contains a few padding '\0' to align
        // data structures to 8byte boundary
        // the padding '\0' are also part of the counted contentLength
        Uint32 headerEnd = httpMessage->message.size() - contentLength;
        Uint32 binContentStart = CIMBuffer::round(headerEnd);

        contentLength = contentLength - (binContentStart - headerEnd);
        content = httpMessage->message.getData() + binContentStart;
    }
    else
    {
        content = httpMessage->message.getData() +
            httpMessage->message.size() - contentLength;
    }

    //
    // If it is a method response, then dispatch it to be handled:
    //

    if (System::strcasecmp(cimOperation, "MethodResponse") != 0)
    {
        MessageLoaderParms mlParms(
            "Client.CIMOperationResponseDecoder.EXPECTED_METHODRESPONSE",
            "Received CIMOperation HTTP header value \"$1\", expected "
                "\"MethodResponse\"",
            cimOperation);
        String mlString(MessageLoader::getMessage(mlParms));

        CIMClientMalformedHTTPException* malformedHTTPException =
            new CIMClientMalformedHTTPException(mlString);

        ClientExceptionMessage * response =
            new ClientExceptionMessage(malformedHTTPException);

        response->setCloseConnect(cimReconnect);

        _outputQueue->enqueue(response);
        return;
    }

    dataStore->setResponseSize(contentLength);
    dataStore->setEndNetworkTime(networkEndTime);
    _handleMethodResponse(content, contentLength,
        httpMessage->contentLanguages, cimReconnect, binaryResponse);
}

void CIMOperationResponseDecoder::_handleMethodResponse(
    const char* content,
    Uint32 contentLength,
    const ContentLanguageList& contentLanguages,
    Boolean cimReconnect,
    Boolean binaryResponse)
{
    Message* response = 0;

    //
    // Decode binary messages up-front and skip remainder:
    //

    if (binaryResponse)
    {
        // Note: this may throw an exception which will be caught by caller.

        CIMBuffer in((char*)content, contentLength);
        CIMBufferReleaser buf_(in);

        CIMResponseMessage* msg = BinaryCodec::decodeResponse(in);

        msg->operationContext.set(
            ContentLanguageListContainer(contentLanguages));
        msg->setCloseConnect(cimReconnect);
        _outputQueue->enqueue(msg);

        return;
    }

    //
    // Create and initialize XML parser:
    //

    XmlParser parser((char*)content);
    XmlEntry entry;

    try
    {
        //
        // Process <?xml ... >
        //

        const char* xmlVersion = 0;
        const char* xmlEncoding = 0;

        XmlReader::getXmlDeclaration(parser, xmlVersion, xmlEncoding);

        //
        // Process <CIM ... >
        //

        const char* cimVersion = 0;
        const char* dtdVersion = 0;

        // ATTN-RK-P3-20020416: Need to validate these versions?
        XmlReader::getCimStartTag(parser, cimVersion, dtdVersion);

        //
        // Expect <MESSAGE ... >
        //

        String messageId;
        String protocolVersion;

        if (!XmlReader::getMessageStartTag(parser, messageId, protocolVersion))
        {
            MessageLoaderParms mlParms(
                "Client.CIMOperationResponseDecoder.EXPECTED_MESSAGE_ELEMENT",
                "expected MESSAGE element");
            throw XmlValidationError(parser.getLine(), mlParms);
        }

        // test for valid protocolVersion
        if (!XmlReader::isSupportedProtocolVersion(protocolVersion))
        {
            MessageLoaderParms mlParms(
                "Client.CIMOperationResponseDecoder.UNSUPPORTED_PROTOCOL",
                "Received unsupported protocol version \"$0\", expected "
                    "\"$1\"",
                protocolVersion,
                "1.[0-9]+");
            String mlString(MessageLoader::getMessage(mlParms));

            CIMClientResponseException* responseException =
                new CIMClientResponseException(mlString);

            ClientExceptionMessage * clientExceptionMessage =
                new ClientExceptionMessage(responseException);

            clientExceptionMessage->setCloseConnect(cimReconnect);

            _outputQueue->enqueue(clientExceptionMessage);
            return;
        }

        //
        // Expect <SIMPLERSP ... >
        //

        XmlReader::expectStartTag(parser, entry, "SIMPLERSP");

        //
        // Expect <IMETHODRESPONSE ... >
        //

        const char* iMethodResponseName = 0;
        Boolean isEmptyTag = false;

        if (XmlReader::getIMethodResponseStartTag(
                parser, iMethodResponseName, isEmptyTag))
        {
            //
            // Dispatch the method:
            //

            if (System::strcasecmp(iMethodResponseName, "GetClass") == 0)
                response = _decodeGetClassResponse(
                    parser, messageId, isEmptyTag);
            else if (System::strcasecmp(
                         iMethodResponseName, "GetInstance") == 0)
                response = _decodeGetInstanceResponse(
                    parser, messageId, isEmptyTag);
            else if (System::strcasecmp(
                         iMethodResponseName, "EnumerateClassNames") == 0)
                response = _decodeEnumerateClassNamesResponse(
                    parser, messageId, isEmptyTag);
            else if (System::strcasecmp(
                         iMethodResponseName, "References") == 0)
                response = _decodeReferencesResponse(
                    parser, messageId, isEmptyTag);
            else if (System::strcasecmp(
                         iMethodResponseName, "ReferenceNames") == 0)
                response = _decodeReferenceNamesResponse(
                    parser, messageId, isEmptyTag);
            else if (System::strcasecmp(
                         iMethodResponseName, "AssociatorNames") == 0)
                response = _decodeAssociatorNamesResponse(
                    parser, messageId, isEmptyTag);
            else if (System::strcasecmp(
                         iMethodResponseName, "Associators") == 0)
                response = _decodeAssociatorsResponse(
                    parser, messageId, isEmptyTag);
            else if (System::strcasecmp(
                         iMethodResponseName, "CreateInstance") == 0)
                response = _decodeCreateInstanceResponse(
                    parser, messageId, isEmptyTag);
            else if (System::strcasecmp(
                         iMethodResponseName,"EnumerateInstanceNames") == 0)
                response = _decodeEnumerateInstanceNamesResponse(
                    parser, messageId, isEmptyTag);
            else if (System::strcasecmp(
                         iMethodResponseName,"EnumerateInstances") == 0)
                response = _decodeEnumerateInstancesResponse(
                    parser, messageId, isEmptyTag);
            else if (System::strcasecmp(
                         iMethodResponseName, "GetProperty") == 0)
                response = _decodeGetPropertyResponse(
                    parser, messageId, isEmptyTag);
            else if (System::strcasecmp(
                         iMethodResponseName, "SetProperty") == 0)
                response = _decodeSetPropertyResponse(
                    parser, messageId, isEmptyTag);
            else if (System::strcasecmp(
                         iMethodResponseName, "DeleteQualifier") == 0)
                response = _decodeDeleteQualifierResponse(
                    parser, messageId, isEmptyTag);
            else if (System::strcasecmp(
                         iMethodResponseName, "GetQualifier") == 0)
                response = _decodeGetQualifierResponse(
                    parser, messageId, isEmptyTag);
            else if (System::strcasecmp(
                         iMethodResponseName, "SetQualifier") == 0)
                response = _decodeSetQualifierResponse(
                    parser, messageId, isEmptyTag);
            else if (System::strcasecmp(
                         iMethodResponseName, "EnumerateQualifiers") == 0)
                response = _decodeEnumerateQualifiersResponse(
                    parser, messageId, isEmptyTag);
            else if (System::strcasecmp(
                         iMethodResponseName, "EnumerateClasses") == 0)
                response = _decodeEnumerateClassesResponse(
                    parser, messageId, isEmptyTag);
            else if (System::strcasecmp(
                         iMethodResponseName, "CreateClass") == 0)
                response = _decodeCreateClassResponse(
                    parser, messageId, isEmptyTag);
            else if (System::strcasecmp(
                         iMethodResponseName, "ModifyClass") == 0)
                response = _decodeModifyClassResponse(
                    parser, messageId, isEmptyTag);
            else if (System::strcasecmp(
                         iMethodResponseName, "ModifyInstance") == 0)
                response = _decodeModifyInstanceResponse(
                    parser, messageId, isEmptyTag);
            else if (System::strcasecmp(
                         iMethodResponseName, "DeleteClass") == 0)
                response = _decodeDeleteClassResponse(
                    parser, messageId, isEmptyTag);
            else if (System::strcasecmp(
                         iMethodResponseName, "DeleteInstance") == 0)
                response = _decodeDeleteInstanceResponse(
                    parser, messageId, isEmptyTag);
            else if (System::strcasecmp(iMethodResponseName, "ExecQuery") == 0)
                response = _decodeExecQueryResponse(
                    parser, messageId, isEmptyTag);
// EXP_PULL_BEGIN
            else if (System::strcasecmp(
                    iMethodResponseName, "OpenEnumerateInstances") == 0)
            {
                response = _decodeOpenEnumerateInstancesResponse(
                    parser, messageId, isEmptyTag);
            }
            else if (System::strcasecmp(
                    iMethodResponseName, "OpenEnumerateInstancePaths") == 0)
            {
                response = _decodeOpenEnumerateInstancePathsResponse(
                    parser, messageId, isEmptyTag);
            }
            else if (System::strcasecmp(
                    iMethodResponseName, "OpenReferenceInstances") == 0)
            {
                response = _decodeOpenReferenceInstancesResponse(
                    parser, messageId, isEmptyTag);
            }
            else if (System::strcasecmp(
                    iMethodResponseName, "OpenReferenceInstancePaths") == 0)
            {
                response = _decodeOpenReferenceInstancePathsResponse(
                    parser, messageId, isEmptyTag);
            }
            else if (System::strcasecmp(
                    iMethodResponseName, "OpenAssociatorInstances") == 0)
            {
                response = _decodeOpenAssociatorInstancesResponse(
                    parser, messageId, isEmptyTag);
            }
            else if (System::strcasecmp(
                    iMethodResponseName, "OpenAssociatorInstancePaths") == 0)
            {
                response = _decodeOpenAssociatorInstancePathsResponse(
                    parser, messageId, isEmptyTag);
            }
            else if (System::strcasecmp(
                    iMethodResponseName, "OpenQueryInstances") == 0)
            {
                response = _decodeOpenQueryInstancesResponse(
                    parser, messageId, isEmptyTag);
            }
            else if (System::strcasecmp(
                    iMethodResponseName, "PullInstancesWithPath") == 0)
            {
                response = _decodePullInstancesWithPathResponse(
                    parser, messageId, isEmptyTag);
            }
            else if (System::strcasecmp(
                    iMethodResponseName, "PullInstancePaths") == 0)
            {
                response = _decodePullInstancePathsResponse(
                    parser, messageId, isEmptyTag);
            }
            else if (System::strcasecmp(
                    iMethodResponseName, "PullInstances") == 0)
            {
                response = _decodePullInstancesResponse(
                    parser, messageId, isEmptyTag);
            }
            else if (System::strcasecmp(
                    iMethodResponseName, "CloseEnumeration") == 0)
            {
                response = _decodeCloseEnumerationResponse(
                    parser, messageId, isEmptyTag);
            }

            else if (System::strcasecmp(
                iMethodResponseName, "EnumerationCount") == 0)
            {
                response = _decodeEnumerationCountResponse(
                    parser, messageId, isEmptyTag);
            }
//EXP_PULL_END
            else
            {
                MessageLoaderParms mlParms(
                    "Client.CIMOperationResponseDecoder.UNRECOGNIZED_NAME",
                    "Unrecognized IMethodResponse name \"$0\"",
                    iMethodResponseName);
                throw XmlValidationError(parser.getLine(), mlParms);
            }

            //
            // Handle end tag:
            //

            if (!isEmptyTag)
            {
                XmlReader::expectEndTag(parser, "IMETHODRESPONSE");
            }
        }
        else if (XmlReader::getMethodResponseStartTag(
                     parser, iMethodResponseName, isEmptyTag))
        {
            response = _decodeInvokeMethodResponse(
                parser, messageId, iMethodResponseName, isEmptyTag);


            //
            // Handle end tag:
            //
            if (!isEmptyTag)
            {
                XmlReader::expectEndTag(parser, "METHODRESPONSE");
            }
        }
        else
        {
            MessageLoaderParms mlParms(
                "Client.CIMOperationResponseDecoder."
                    "EXPECTED_METHODRESPONSE_OR_IMETHODRESPONSE_ELEMENT",
                "expected METHODRESPONSE or IMETHODRESPONSE element");
            throw XmlValidationError(parser.getLine(), mlParms);
        }

        //
        // Handle end tags:
        //
        XmlReader::expectEndTag(parser, "SIMPLERSP");
        XmlReader::expectEndTag(parser, "MESSAGE");
        XmlReader::expectEndTag(parser, "CIM");
    }
    catch (XmlException& x)
    {
        if (response)
        {
            delete response;
        }

        response = new ClientExceptionMessage(
            new CIMClientXmlException(x.getMessage()));
    }
    catch (Exception& x)
    {
        // Might get MalformedObjectNameException, InvalidNameException, etc.

        if (response)
        {
            delete response;
        }

        response = new ClientExceptionMessage(
            new CIMClientResponseException(x.getMessage()));
    }

//l10n start
// l10n TODO - might want to move A-L and C-L to Message
// to make this more maintainable
    // Add the language header to the request
    CIMMessage * cimmsg = dynamic_cast<CIMMessage *>(response);
    if (cimmsg != NULL)
    {
        cimmsg->operationContext.set(
            ContentLanguageListContainer(contentLanguages));
    }
    else
    {
        ;    // l10n TODO - error back to client here
    }
// l10n end

    response->setCloseConnect(cimReconnect);


    _outputQueue->enqueue(response);
}

CIMCreateClassResponseMessage*
    CIMOperationResponseDecoder::_decodeCreateClassResponse(
        XmlParser& parser,
        const String& messageId,
        Boolean isEmptyImethodresponseTag)
{
    XmlEntry entry;
    CIMException cimException;

    if (!isEmptyImethodresponseTag)
    {
        if (XmlReader::getErrorElement(parser, cimException))
        {
            return new CIMCreateClassResponseMessage(
                messageId,
                cimException,
                QueueIdStack());
        }

        if (XmlReader::testStartTagOrEmptyTag(parser, entry, "IRETURNVALUE"))
        {
            if (entry.type != XmlEntry::EMPTY_TAG)
            {
                XmlReader::expectEndTag(parser, "IRETURNVALUE");
            }
        }
    }

    return new CIMCreateClassResponseMessage(
        messageId,
        cimException,
        QueueIdStack());
}

CIMGetClassResponseMessage*
    CIMOperationResponseDecoder::_decodeGetClassResponse(
        XmlParser& parser,
        const String& messageId,
        Boolean isEmptyImethodresponseTag)
{
    XmlEntry entry;
    CIMException cimException;

    if (isEmptyImethodresponseTag)
    {
        MessageLoaderParms mlParms("Common.XmlReader.EXPECTED_OPEN",
            "Expected open of $0 element", "IMETHODRESPONSE");
        throw XmlValidationError(parser.getLine(), mlParms);
    }
    else if (XmlReader::getErrorElement(parser, cimException))
    {
        return new CIMGetClassResponseMessage(
            messageId,
            cimException,
            QueueIdStack(),
            CIMClass());
    }
    else if (XmlReader::testStartTagOrEmptyTag(parser, entry, "IRETURNVALUE"))
    {
        CIMClass cimClass;

        if ((entry.type == XmlEntry::EMPTY_TAG) ||
            !XmlReader::getClassElement(parser, cimClass))
        {
            MessageLoaderParms mlParms(
                "Client.CIMOperationResponseDecoder.EXPECTED_CLASS_ELEMENT",
                "expected CLASS element");
            throw XmlValidationError(parser.getLine(), mlParms);
        }

        XmlReader::expectEndTag(parser, "IRETURNVALUE");

        return new CIMGetClassResponseMessage(
            messageId,
            cimException,
            QueueIdStack(),
            cimClass);
    }
    else
    {
        MessageLoaderParms mlParms(
            "Client.CIMOperationResponseDecoder."
                "EXPECTED_ERROR_OR_IRETURNVALUE_ELEMENT",
            "expected ERROR or IRETURNVALUE element");
        throw XmlValidationError(parser.getLine(), mlParms);
    }
}

CIMModifyClassResponseMessage*
    CIMOperationResponseDecoder::_decodeModifyClassResponse(
        XmlParser& parser,
        const String& messageId,
        Boolean isEmptyImethodresponseTag)
{
    XmlEntry entry;
    CIMException cimException;

    if (!isEmptyImethodresponseTag)
    {
        if (XmlReader::getErrorElement(parser, cimException))
        {
            return new CIMModifyClassResponseMessage(
                messageId,
                cimException,
                QueueIdStack());
        }

        if (XmlReader::testStartTagOrEmptyTag(parser, entry, "IRETURNVALUE"))
        {
            if (entry.type != XmlEntry::EMPTY_TAG)
            {
                XmlReader::expectEndTag(parser, "IRETURNVALUE");
            }
        }
    }

    return new CIMModifyClassResponseMessage(
        messageId,
        cimException,
        QueueIdStack());
}

CIMEnumerateClassNamesResponseMessage*
    CIMOperationResponseDecoder::_decodeEnumerateClassNamesResponse(
        XmlParser& parser,
        const String& messageId,
        Boolean isEmptyImethodresponseTag)
{
    XmlEntry entry;
    CIMException cimException;
    Array<CIMName> classNames;

    if (!isEmptyImethodresponseTag)
    {
        if (XmlReader::getErrorElement(parser, cimException))
        {
            return new CIMEnumerateClassNamesResponseMessage(
                messageId,
                cimException,
                QueueIdStack(),
                Array<CIMName>());
        }

        if (XmlReader::testStartTagOrEmptyTag(parser, entry, "IRETURNVALUE"))
        {
            if (entry.type != XmlEntry::EMPTY_TAG)
            {
                CIMName className;

                while (XmlReader::getClassNameElement(parser, className, false))
                    classNames.append(className);

                XmlReader::expectEndTag(parser, "IRETURNVALUE");
            }
        }
    }

    return new CIMEnumerateClassNamesResponseMessage(
        messageId,
        cimException,
        QueueIdStack(),
        classNames);
}

CIMEnumerateClassesResponseMessage*
    CIMOperationResponseDecoder::_decodeEnumerateClassesResponse(
        XmlParser& parser,
        const String& messageId,
        Boolean isEmptyImethodresponseTag)
{
    XmlEntry entry;
    CIMException cimException;
    Array<CIMClass> cimClasses;

    if (!isEmptyImethodresponseTag)
    {
        if (XmlReader::getErrorElement(parser, cimException))
        {
            return new CIMEnumerateClassesResponseMessage(
                messageId,
                cimException,
                QueueIdStack(),
                Array<CIMClass>());
        }

        if (XmlReader::testStartTagOrEmptyTag(parser, entry, "IRETURNVALUE"))
        {
            if (entry.type != XmlEntry::EMPTY_TAG)
            {
                CIMClass cimClass;

                while (XmlReader::getClassElement(parser, cimClass))
                    cimClasses.append(cimClass);

                XmlReader::expectEndTag(parser, "IRETURNVALUE");
            }
        }
    }

    return new CIMEnumerateClassesResponseMessage(
        messageId,
        cimException,
        QueueIdStack(),
        cimClasses);
}

CIMDeleteClassResponseMessage*
    CIMOperationResponseDecoder::_decodeDeleteClassResponse(
        XmlParser& parser,
        const String& messageId,
        Boolean isEmptyImethodresponseTag)
{
    XmlEntry entry;
    CIMException cimException;

    if (!isEmptyImethodresponseTag)
    {
        if (XmlReader::getErrorElement(parser, cimException))
        {
            return new CIMDeleteClassResponseMessage(
                messageId,
                cimException,
                QueueIdStack());
        }

        if (XmlReader::testStartTagOrEmptyTag(parser, entry, "IRETURNVALUE"))
        {
            if (entry.type != XmlEntry::EMPTY_TAG)
            {
                XmlReader::expectEndTag(parser, "IRETURNVALUE");
            }
        }
    }

    return new CIMDeleteClassResponseMessage(
        messageId,
        cimException,
        QueueIdStack());
}

CIMCreateInstanceResponseMessage*
    CIMOperationResponseDecoder::_decodeCreateInstanceResponse(
        XmlParser& parser,
        const String& messageId,
        Boolean isEmptyImethodresponseTag)
{
    XmlEntry entry;
    CIMException cimException;

    if (isEmptyImethodresponseTag)
    {
        MessageLoaderParms mlParms("Common.XmlReader.EXPECTED_OPEN",
            "Expected open of $0 element", "IMETHODRESPONSE");
        throw XmlValidationError(parser.getLine(), mlParms);
    }
    else if (XmlReader::getErrorElement(parser, cimException))
    {
        return new CIMCreateInstanceResponseMessage(
            messageId,
            cimException,
            QueueIdStack(),
            CIMObjectPath());
    }
    else if (XmlReader::testStartTag(parser, entry, "IRETURNVALUE"))
    {
        CIMObjectPath instanceName;
        XmlReader::getInstanceNameElement(parser, instanceName);

        XmlReader::expectEndTag(parser, "IRETURNVALUE");

        return new CIMCreateInstanceResponseMessage(
            messageId,
            cimException,
            QueueIdStack(),
            instanceName);
    }
    else
    {
        MessageLoaderParms mlParms(
            "Client.CIMOperationResponseDecoder."
                "EXPECTED_ERROR_OR_IRETURNVALUE_ELEMENT",
            "expected ERROR or IRETURNVALUE element");

        throw XmlValidationError(parser.getLine(), mlParms);
    }
}

CIMGetInstanceResponseMessage*
    CIMOperationResponseDecoder::_decodeGetInstanceResponse(
        XmlParser& parser,
        const String& messageId,
        Boolean isEmptyImethodresponseTag)
{
    XmlEntry entry;
    CIMException cimException;

    if (isEmptyImethodresponseTag)
    {
        MessageLoaderParms mlParms("Common.XmlReader.EXPECTED_OPEN",
            "Expected open of $0 element", "IMETHODRESPONSE");
        throw XmlValidationError(parser.getLine(), mlParms);
    }
    else if (XmlReader::getErrorElement(parser, cimException))
    {
        return new CIMGetInstanceResponseMessage(
            messageId,
            cimException,
            QueueIdStack());
    }
    else if (XmlReader::testStartTagOrEmptyTag(parser, entry, "IRETURNVALUE"))
    {
        CIMInstance cimInstance;

        if ((entry.type == XmlEntry::EMPTY_TAG) ||
            !XmlReader::getInstanceElement(parser, cimInstance))
        {
            MessageLoaderParms mlParms(
                "Client.CIMOperationResponseDecoder.EXPECTED_INSTANCE_ELEMENT",
                "expected INSTANCE element");
            throw XmlValidationError(parser.getLine(), mlParms);
        }

        XmlReader::expectEndTag(parser, "IRETURNVALUE");

        CIMGetInstanceResponseMessage* msg = new CIMGetInstanceResponseMessage(
            messageId,
            cimException,
            QueueIdStack());
        msg->getResponseData().setInstance(cimInstance);
        return msg;
    }
    else
    {
        MessageLoaderParms mlParms(
            "Client.CIMOperationResponseDecoder."
                "EXPECTED_ERROR_OR_IRETURNVALUE_ELEMENT",
            "expected ERROR or IRETURNVALUE element");
        throw XmlValidationError(parser.getLine(), mlParms);
    }
}

CIMModifyInstanceResponseMessage*
    CIMOperationResponseDecoder::_decodeModifyInstanceResponse(
        XmlParser& parser,
        const String& messageId,
        Boolean isEmptyImethodresponseTag)
{
    XmlEntry entry;
    CIMException cimException;

    if (!isEmptyImethodresponseTag)
    {
        if (XmlReader::getErrorElement(parser, cimException))
        {
            return new CIMModifyInstanceResponseMessage(
                messageId,
                cimException,
                QueueIdStack());
        }

        if (XmlReader::testStartTagOrEmptyTag(parser, entry, "IRETURNVALUE"))
        {
            if (entry.type != XmlEntry::EMPTY_TAG)
            {
                XmlReader::expectEndTag(parser, "IRETURNVALUE");
            }
        }
    }

    return new CIMModifyInstanceResponseMessage(
        messageId,
        cimException,
        QueueIdStack());
}

CIMEnumerateInstanceNamesResponseMessage*
    CIMOperationResponseDecoder::_decodeEnumerateInstanceNamesResponse(
        XmlParser& parser,
        const String& messageId,
        Boolean isEmptyImethodresponseTag)
{
    XmlEntry entry;
    CIMException cimException;
    Array<CIMObjectPath> instanceNames;

    if (!isEmptyImethodresponseTag)
    {
        if (XmlReader::getErrorElement(parser, cimException))
        {
            return new CIMEnumerateInstanceNamesResponseMessage(
                messageId,
                cimException,
                QueueIdStack());
        }

        if (XmlReader::testStartTagOrEmptyTag(parser, entry, "IRETURNVALUE"))
        {
            if (entry.type != XmlEntry::EMPTY_TAG)
            {
                String className;
                Array<CIMKeyBinding> keyBindings;

                while (XmlReader::getInstanceNameElement(
                    parser, className, keyBindings))
                {
                    CIMObjectPath r(
                        String::EMPTY,
                        CIMNamespaceName(),
                        className,
                        keyBindings);
                    instanceNames.append(r);
                }

                XmlReader::expectEndTag(parser, "IRETURNVALUE");
            }
        }
    }

    CIMEnumerateInstanceNamesResponseMessage* msg;

    msg = new CIMEnumerateInstanceNamesResponseMessage(
        messageId,
        cimException,
        QueueIdStack());

    msg->getResponseData().setInstanceNames(instanceNames);
    return msg;
}

CIMEnumerateInstancesResponseMessage*
    CIMOperationResponseDecoder::_decodeEnumerateInstancesResponse(
        XmlParser& parser,
        const String& messageId,
        Boolean isEmptyImethodresponseTag)
{
    XmlEntry entry;
    CIMException cimException;
    Array<CIMInstance> namedInstances;

    if (!isEmptyImethodresponseTag)
    {
        if (XmlReader::getErrorElement(parser, cimException))
        {
            return new CIMEnumerateInstancesResponseMessage(
                messageId,
                cimException,
                QueueIdStack());
        }

        if (XmlReader::testStartTagOrEmptyTag(parser, entry, "IRETURNVALUE"))
        {
            if (entry.type != XmlEntry::EMPTY_TAG)
            {
                CIMInstance namedInstance;

                while (XmlReader::getNamedInstanceElement(
                           parser, namedInstance))
                {
                    namedInstances.append(namedInstance);
                }

                XmlReader::expectEndTag(parser, "IRETURNVALUE");
            }
        }
    }

    CIMEnumerateInstancesResponseMessage* msg;

    msg = new CIMEnumerateInstancesResponseMessage(
        messageId,
        cimException,
        QueueIdStack());

    msg->getResponseData().setInstances(namedInstances);
    return msg;
}

CIMDeleteInstanceResponseMessage*
    CIMOperationResponseDecoder::_decodeDeleteInstanceResponse(
        XmlParser& parser,
        const String& messageId,
        Boolean isEmptyImethodresponseTag)
{
    XmlEntry entry;
    CIMException cimException;

    if (!isEmptyImethodresponseTag)
    {
        if (XmlReader::getErrorElement(parser, cimException))
        {
            return new CIMDeleteInstanceResponseMessage(
                messageId,
                cimException,
                QueueIdStack());
        }

        if (XmlReader::testStartTagOrEmptyTag(parser, entry, "IRETURNVALUE"))
        {
            if (entry.type != XmlEntry::EMPTY_TAG)
            {
                XmlReader::expectEndTag(parser, "IRETURNVALUE");
            }
        }
    }

    return new CIMDeleteInstanceResponseMessage(
        messageId,
        cimException,
        QueueIdStack());
}

CIMGetPropertyResponseMessage*
    CIMOperationResponseDecoder::_decodeGetPropertyResponse(
        XmlParser& parser,
        const String& messageId,
        Boolean isEmptyImethodresponseTag)
{
    XmlEntry entry;
    CIMException cimException;
    CIMValue cimValue(CIMTYPE_STRING, false);

    if (!isEmptyImethodresponseTag)
    {
        if (XmlReader::getErrorElement(parser, cimException))
        {
            return new CIMGetPropertyResponseMessage(
                messageId,
                cimException,
                QueueIdStack(),
                CIMValue());
        }

        if (XmlReader::testStartTagOrEmptyTag(parser, entry, "IRETURNVALUE"))
        {
            if (entry.type != XmlEntry::EMPTY_TAG)
            {
                if (!XmlReader::getPropertyValue(parser, cimValue))
                {
                    // No value given; just return a null String value
                }

                XmlReader::expectEndTag(parser, "IRETURNVALUE");
            }
        }
        else
        {
            // No value given; just return a null String value
        }
    }

    return new CIMGetPropertyResponseMessage(
        messageId,
        cimException,
        QueueIdStack(),
        cimValue);
}

CIMSetPropertyResponseMessage*
    CIMOperationResponseDecoder::_decodeSetPropertyResponse(
        XmlParser& parser,
        const String& messageId,
        Boolean isEmptyImethodresponseTag)
{
    XmlEntry entry;
    CIMException cimException;

    if (!isEmptyImethodresponseTag)
    {
        if (XmlReader::getErrorElement(parser, cimException))
        {
            return new CIMSetPropertyResponseMessage(
                messageId,
                cimException,
                QueueIdStack());
        }

        if (XmlReader::testStartTagOrEmptyTag(parser, entry, "IRETURNVALUE"))
        {
            if (entry.type != XmlEntry::EMPTY_TAG)
            {
                XmlReader::expectEndTag(parser, "IRETURNVALUE");
            }
        }
    }

    return new CIMSetPropertyResponseMessage(
        messageId,
        cimException,
        QueueIdStack());
}

CIMSetQualifierResponseMessage*
    CIMOperationResponseDecoder::_decodeSetQualifierResponse(
        XmlParser& parser,
        const String& messageId,
        Boolean isEmptyImethodresponseTag)
{
    XmlEntry entry;
    CIMException cimException;

    if (!isEmptyImethodresponseTag)
    {
        if (XmlReader::getErrorElement(parser, cimException))
        {
            return new CIMSetQualifierResponseMessage(
                messageId,
                cimException,
                QueueIdStack());
        }

        if (XmlReader::testStartTagOrEmptyTag(parser, entry, "IRETURNVALUE"))
        {
            if (entry.type != XmlEntry::EMPTY_TAG)
            {
                XmlReader::expectEndTag(parser, "IRETURNVALUE");
            }
        }
    }

    return new CIMSetQualifierResponseMessage(
        messageId,
        cimException,
        QueueIdStack());
}

CIMGetQualifierResponseMessage*
    CIMOperationResponseDecoder::_decodeGetQualifierResponse(
        XmlParser& parser,
        const String& messageId,
        Boolean isEmptyImethodresponseTag)
{
    XmlEntry entry;
    CIMException cimException;

    if (isEmptyImethodresponseTag)
    {
        MessageLoaderParms mlParms("Common.XmlReader.EXPECTED_OPEN",
            "Expected open of $0 element", "IMETHODRESPONSE");
        throw XmlValidationError(parser.getLine(), mlParms);
    }
    else if (XmlReader::getErrorElement(parser, cimException))
    {
        return new CIMGetQualifierResponseMessage(
            messageId,
            cimException,
            QueueIdStack(),
            CIMQualifierDecl());
    }
    else if (XmlReader::testStartTag(parser, entry, "IRETURNVALUE"))
    {
        CIMQualifierDecl qualifierDecl;
        XmlReader::getQualifierDeclElement(parser, qualifierDecl);

        XmlReader::expectEndTag(parser, "IRETURNVALUE");

        return new CIMGetQualifierResponseMessage(
            messageId,
            cimException,
            QueueIdStack(),
            qualifierDecl);
    }
    else
    {
        MessageLoaderParms mlParms(
            "Client.CIMOperationResponseDecoder."
                "EXPECTED_ERROR_OR_IRETURNVALUE_ELEMENT",
            "expected ERROR or IRETURNVALUE element");
        throw XmlValidationError(parser.getLine(), mlParms);
    }
}

CIMEnumerateQualifiersResponseMessage*
    CIMOperationResponseDecoder::_decodeEnumerateQualifiersResponse(
        XmlParser& parser,
        const String& messageId,
        Boolean isEmptyImethodresponseTag)
{
    XmlEntry entry;
    CIMException cimException;
    Array<CIMQualifierDecl> qualifierDecls;

    if (!isEmptyImethodresponseTag)
    {
        if (XmlReader::getErrorElement(parser, cimException))
        {
            return new CIMEnumerateQualifiersResponseMessage(
                messageId,
                cimException,
                QueueIdStack(),
                Array<CIMQualifierDecl>());
        }

        if (XmlReader::testStartTagOrEmptyTag(parser, entry, "IRETURNVALUE"))
        {
            if (entry.type != XmlEntry::EMPTY_TAG)
            {
                CIMQualifierDecl qualifierDecl;

                while (XmlReader::getQualifierDeclElement(
                           parser, qualifierDecl))
                {
                    qualifierDecls.append(qualifierDecl);
                }

                XmlReader::expectEndTag(parser, "IRETURNVALUE");
            }
        }
    }

    return new CIMEnumerateQualifiersResponseMessage(
        messageId,
        cimException,
        QueueIdStack(),
        qualifierDecls);
}

CIMDeleteQualifierResponseMessage*
    CIMOperationResponseDecoder::_decodeDeleteQualifierResponse(
        XmlParser& parser,
        const String& messageId,
        Boolean isEmptyImethodresponseTag)
{
    XmlEntry entry;
    CIMException cimException;

    if (!isEmptyImethodresponseTag)
    {
        if (XmlReader::getErrorElement(parser, cimException))
        {
            return new CIMDeleteQualifierResponseMessage(
                messageId,
                cimException,
                QueueIdStack());
        }

        if (XmlReader::testStartTagOrEmptyTag(parser, entry, "IRETURNVALUE"))
        {
            if (entry.type != XmlEntry::EMPTY_TAG)
            {
                XmlReader::expectEndTag(parser, "IRETURNVALUE");
            }
        }
    }

    return new CIMDeleteQualifierResponseMessage(
        messageId,
        cimException,
        QueueIdStack());
}

CIMReferenceNamesResponseMessage*
    CIMOperationResponseDecoder::_decodeReferenceNamesResponse(
        XmlParser& parser,
        const String& messageId,
        Boolean isEmptyImethodresponseTag)
{
    XmlEntry entry;
    CIMException cimException;
    Array<CIMObjectPath> objectPaths;

    if (!isEmptyImethodresponseTag)
    {
        if (XmlReader::getErrorElement(parser, cimException))
        {
            return new CIMReferenceNamesResponseMessage(
                messageId,
                cimException,
                QueueIdStack());
        }

        if (XmlReader::testStartTagOrEmptyTag(parser, entry, "IRETURNVALUE"))
        {
            if (entry.type != XmlEntry::EMPTY_TAG)
            {
                CIMObjectPath objectPath;

                while (XmlReader::getObjectPathElement(parser, objectPath))
                    objectPaths.append(objectPath);

                XmlReader::expectEndTag(parser, "IRETURNVALUE");
            }
        }
    }

    CIMReferenceNamesResponseMessage* msg;

    msg = new CIMReferenceNamesResponseMessage(
        messageId,
        cimException,
        QueueIdStack());

    msg->getResponseData().setInstanceNames(objectPaths);

    return msg;
}

CIMReferencesResponseMessage*
    CIMOperationResponseDecoder::_decodeReferencesResponse(
        XmlParser& parser,
        const String& messageId,
        Boolean isEmptyImethodresponseTag)
{
    XmlEntry entry;
    CIMException cimException;
    Array<CIMObject> objectWithPathArray;

    if (!isEmptyImethodresponseTag)
    {
        if (XmlReader::getErrorElement(parser, cimException))
        {
            return new CIMReferencesResponseMessage(
                messageId,
                cimException,
                QueueIdStack());
        }

        if (XmlReader::testStartTagOrEmptyTag(parser, entry, "IRETURNVALUE"))
        {
            if (entry.type != XmlEntry::EMPTY_TAG)
            {
                CIMObject objectWithPath;

                while (XmlReader::getValueObjectWithPathElement(
                           parser, objectWithPath))
                {
                    objectWithPathArray.append(objectWithPath);
                }

                XmlReader::expectEndTag(parser, "IRETURNVALUE");
            }
        }
    }

    CIMReferencesResponseMessage *msg;

    msg = new CIMReferencesResponseMessage(
        messageId,
        cimException,
        QueueIdStack());

    msg->getResponseData().setObjects(objectWithPathArray);

    return msg;
}

CIMAssociatorNamesResponseMessage*
    CIMOperationResponseDecoder::_decodeAssociatorNamesResponse(
        XmlParser& parser,
        const String& messageId,
        Boolean isEmptyImethodresponseTag)
{
    XmlEntry entry;
    CIMException cimException;
    Array<CIMObjectPath> objectPaths;

    if (!isEmptyImethodresponseTag)
    {
        if (XmlReader::getErrorElement(parser, cimException))
        {
            return new CIMAssociatorNamesResponseMessage(
                messageId,
                cimException,
                QueueIdStack());
        }

        if (XmlReader::testStartTagOrEmptyTag(parser, entry, "IRETURNVALUE"))
        {
            if (entry.type != XmlEntry::EMPTY_TAG)
            {
                CIMObjectPath objectPath;

                while (XmlReader::getObjectPathElement(parser, objectPath))
                    objectPaths.append(objectPath);

                XmlReader::expectEndTag(parser, "IRETURNVALUE");
            }
        }
    }

    CIMAssociatorNamesResponseMessage* msg;

    msg = new CIMAssociatorNamesResponseMessage(
        messageId,
        cimException,
        QueueIdStack());

    msg->getResponseData().setInstanceNames(objectPaths);

    return msg;
}

CIMAssociatorsResponseMessage*
    CIMOperationResponseDecoder::_decodeAssociatorsResponse(
        XmlParser& parser,
        const String& messageId,
        Boolean isEmptyImethodresponseTag)
{
    XmlEntry entry;
    CIMException cimException;
    Array<CIMObject> objectWithPathArray;

    if (!isEmptyImethodresponseTag)
    {
        if (XmlReader::getErrorElement(parser, cimException))
        {
            return new CIMAssociatorsResponseMessage(
                messageId,
                cimException,
                QueueIdStack());
        }

        if (XmlReader::testStartTagOrEmptyTag(parser, entry, "IRETURNVALUE"))
        {
            if (entry.type != XmlEntry::EMPTY_TAG)
            {
                CIMObject objectWithPath;

                while (XmlReader::getValueObjectWithPathElement(
                           parser, objectWithPath))
                {
                    objectWithPathArray.append(objectWithPath);
                }

                XmlReader::expectEndTag(parser, "IRETURNVALUE");
            }
        }
    }

    CIMAssociatorsResponseMessage* msg;

    msg = new CIMAssociatorsResponseMessage(
        messageId,
        cimException,
        QueueIdStack());

    msg->getResponseData().setObjects(objectWithPathArray);

    return msg;
}

CIMExecQueryResponseMessage*
    CIMOperationResponseDecoder::_decodeExecQueryResponse(
        XmlParser& parser,
        const String& messageId,
        Boolean isEmptyImethodresponseTag)
{
    XmlEntry entry;
    CIMException cimException;
    Array<CIMObject> objectWithPathArray;

    if (!isEmptyImethodresponseTag)
    {
        if (XmlReader::getErrorElement(parser, cimException))
        {
            return new CIMExecQueryResponseMessage(
                messageId,
                cimException,
                QueueIdStack());
        }

        if (XmlReader::testStartTagOrEmptyTag(parser, entry, "IRETURNVALUE"))
        {
            if (entry.type != XmlEntry::EMPTY_TAG)
            {
                XmlReader::getObjectArray(parser, objectWithPathArray);

                XmlReader::expectEndTag(parser, "IRETURNVALUE");
            }
        }
    }

    CIMExecQueryResponseMessage* msg;

    msg = new CIMExecQueryResponseMessage(
        messageId,
        cimException,
        QueueIdStack());

    msg->getResponseData().setObjects(objectWithPathArray);

    return msg;
}

CIMInvokeMethodResponseMessage*
    CIMOperationResponseDecoder::_decodeInvokeMethodResponse(
        XmlParser& parser,
        const String& messageId,
        const String& methodName,
        Boolean isEmptyMethodresponseTag)
{
    CIMException cimException;

    CIMParamValue paramValue;
    Array<CIMParamValue> outParameters;
    CIMValue returnValue;

    if (!isEmptyMethodresponseTag)
    {
        if (XmlReader::getErrorElement(parser, cimException))
        {
            return new CIMInvokeMethodResponseMessage(
                messageId,
                cimException,
                QueueIdStack(),
                returnValue,
                outParameters,
                methodName);
        }

        Boolean isReturnValue = false;
        Boolean isParamValue = false;
        Boolean gotReturnValue = false;

        while ((isReturnValue =
                    XmlReader::getReturnValueElement(parser, returnValue)) ||
               (isParamValue =
                    XmlReader::getParamValueElement(parser, paramValue)))
        {
            if (isReturnValue)
            {
                if (gotReturnValue)
                {
                    MessageLoaderParms mlParms(
                        "Client.CIMOperationResponseDecoder."
                            "EXPECTED_RETURNVALUE_ELEMENT",
                        "unexpected RETURNVALUE element");
                    throw XmlValidationError(parser.getLine(), mlParms);
                }
                gotReturnValue = true;
            }
            else    // isParamValue == true
            {
                outParameters.append(paramValue);
            }

            isReturnValue = false;
            isParamValue = false;
        }
    }

    return new CIMInvokeMethodResponseMessage(
        messageId,
        cimException,
        QueueIdStack(),
        returnValue,
        outParameters,
        methodName);
}

// EXP_PULL_BEGIN
/**************************************************************************
**
**   Common Functions to support the decode of Pull Operation Responses
**
***************************************************************************/

/*
    Decode the instancePath portion of all of the open an pull instancepaths
    operations.  This function is common to all of the pull decode operations.
*/

// KS_EXP_TBD - Can we combine what we do here with the function in
// enumerateinstancenames that uses getInstanceNameElement????
void _decodeInstancePathElements(
    XmlParser& parser,
    Array<CIMObjectPath>& instancePaths)
{
    XmlEntry entry;
    if (XmlReader::testStartTagOrEmptyTag(parser, entry, "IRETURNVALUE"))
    {
        if (entry.type != XmlEntry::EMPTY_TAG)
        {
            CIMObjectPath instancePath;

            while (XmlReader::getInstancePathElement(parser, instancePath))
            {
                instancePaths.append(instancePath);
            }
            XmlReader::expectEndTag(parser, "IRETURNVALUE");
        }
    }
}

/*
    decode returned instancesWithPathElement into an array
    of instances. This function is common to all of the Pull decoder
    operations.
*/
void _decodeGetInstancesWithPathElement(
    XmlParser& parser,
    Array<CIMInstance>& namedInstances)
{
    XmlEntry entry;
    if (XmlReader::testStartTagOrEmptyTag(parser, entry, "IRETURNVALUE"))
    {
        if (entry.type != XmlEntry::EMPTY_TAG)
        {
            CIMInstance namedInstance;

            /// KS_TBD _QUESTION. Diff of this getNameInstances Function
            while (XmlReader::getInstanceWithPathElement(
                       parser, namedInstance))
            {
                namedInstances.append(namedInstance);
            }

            XmlReader::expectEndTag(parser, "IRETURNVALUE");
        }
    }
}

/*
    Decode returned instances Element into an array
    of instances. This function is only for pullInstances.
*/
void _decodeGetInstancesElement(
    XmlParser& parser,
    Array<CIMInstance>& instances)
{
    XmlEntry entry;
    if (XmlReader::testStartTagOrEmptyTag(parser, entry, "IRETURNVALUE"))
    {
        if (entry.type != XmlEntry::EMPTY_TAG)
        {
            CIMInstance instance;

            while (XmlReader::getInstanceElement(parser, instance))
            {
                instances.append(instance);
            }

            XmlReader::expectEndTag(parser, "IRETURNVALUE");
        }
    }
}

/*  Common Function for Open, Pull Parm Value processing.
    Parse the output parameters from the  responses that
    have two parameters (endOfSequence and enumerationContext). These
    parameters are common across all of the Open* operations and the
    pull operations.
    This function returns the parsed values of these parameters or, if
    there is an error, generates an exception.
*/
void _decodeOpenResponseParamValues(XmlParser& parser,
       Boolean& endOfSequence,
       String& enumerationContext)
{
    Boolean duplicateParameter = false;
    Boolean gotEndOfSequence = false;
    Boolean gotEnumerationContext = false;

    Boolean emptyTag;
    for (const char* name;
         XmlReader::getParamValueTag(parser, name, emptyTag); )
    {
        if (System::strcasecmp(name, "EndOfSequence") == 0)
        {
            XmlReader::rejectNullParamValue(parser, emptyTag, name);
            XmlReader::getBooleanValueElement(parser, endOfSequence, true);
            duplicateParameter = gotEndOfSequence;
            gotEndOfSequence = true;
        }

        else if (System::strcasecmp(name, "EnumerationContext") == 0)
        {
            XmlReader::getStringValueElement(parser, enumerationContext,
                false);
            duplicateParameter = gotEnumerationContext;
            gotEnumerationContext = true;
        }
        else
        {
            // Ignore this as an extra tag
        }
        if (!emptyTag)
        {
            XmlReader::expectEndTag(parser, "PARAMVALUE");
        }
        // Stop on the first duplicate found
        if (duplicateParameter)
        {
            throw PEGASUS_CIM_EXCEPTION(
                CIM_ERR_INVALID_PARAMETER,
                    "Duplicate EndOfSequence or EnumerationContext received");
        }
    }

    // KS_TODO -Should the error be INVALID_PARAMETER or XmlValidation since
    // it is really MISSING but this is response so we have generally
    // used XmlValidationError.
    if (!gotEndOfSequence)
    {
        throw PEGASUS_CIM_EXCEPTION(CIM_ERR_INVALID_PARAMETER,
            "EndOfSequence is a Required Parameter");
    }

    // EnumerationContext is required parameter
    if (!gotEnumerationContext)
    {
        throw PEGASUS_CIM_EXCEPTION(CIM_ERR_INVALID_PARAMETER,
            "EnumerationContext is a Required Parameter");
    }
    // EnumerationContext must have value if not endOfSequence
    if ((!endOfSequence) && (enumerationContext.size() == 0))
    {
        throw PEGASUS_CIM_EXCEPTION(CIM_ERR_INVALID_PARAMETER,
            "Valid EnumerationContext is a Required Parameter");
    }
}

/******************************************************************************
**
** Open and Pull Operation Response Decoders
**
******************************************************************************/
CIMOpenEnumerateInstancesResponseMessage*
    CIMOperationResponseDecoder::_decodeOpenEnumerateInstancesResponse(
        XmlParser& parser,
        const String& messageId,
        Boolean isEmptyImethodresponseTag)
{
    //XmlEntry entry;
    CIMException cimException;
    Array<CIMInstance> namedInstances;
    Boolean endOfSequence = true;
    String enumerationContext = String::EMPTY;

    if (XmlReader::getErrorElement(parser, cimException))
    {
        return new CIMOpenEnumerateInstancesResponseMessage(
            messageId,
            cimException,
            QueueIdStack(),
            endOfSequence,
            enumerationContext);
    }
    // EXP_PULL should error out if response empty because either
    // enumerationContext or endOfSequence is required. Create
    // missing Parameter error here.
    if (isEmptyImethodresponseTag)
    {
        throw PEGASUS_CIM_EXCEPTION(CIM_ERR_INVALID_PARAMETER,
            "Return Parameters endOfSequence"
                "and/or enumerationContext required.");
    }

    _decodeGetInstancesWithPathElement(parser, namedInstances);

    // Get the OUT parameters (endOfSequence and enumerationContext)
    _decodeOpenResponseParamValues(parser, endOfSequence, enumerationContext);

    CIMOpenEnumerateInstancesResponseMessage* msg;

    msg = new CIMOpenEnumerateInstancesResponseMessage(
        messageId,
        cimException,
        QueueIdStack(),
        endOfSequence,
        enumerationContext);

    msg->getResponseData().setInstances(namedInstances);
    return msg;
}

CIMOpenEnumerateInstancePathsResponseMessage*
    CIMOperationResponseDecoder::_decodeOpenEnumerateInstancePathsResponse(
        XmlParser& parser,
        const String& messageId,
        Boolean isEmptyImethodresponseTag)
{
    XmlEntry entry;
    CIMException cimException;
    Array<CIMObjectPath> instancePaths;

    Boolean endOfSequence = true;
    String enumerationContext = String::EMPTY;

    if (XmlReader::getErrorElement(parser, cimException))
    {
        return new CIMOpenEnumerateInstancePathsResponseMessage(
            messageId,
            cimException,
            QueueIdStack(),
            endOfSequence,
            enumerationContext);
    }

    if (isEmptyImethodresponseTag)
    {
        throw PEGASUS_CIM_EXCEPTION(CIM_ERR_INVALID_PARAMETER,
            "Return Parameters endOfSequence"
                "and/or enumerationContext required.");

    }

    _decodeInstancePathElements(parser, instancePaths);

    // Get the OUT parameters (endOfSequence and enumerationContext)
    _decodeOpenResponseParamValues(parser, endOfSequence, enumerationContext);

    CIMOpenEnumerateInstancePathsResponseMessage* msg;

    msg = new CIMOpenEnumerateInstancePathsResponseMessage(
        messageId,
        cimException,
        QueueIdStack(),
        endOfSequence,
        enumerationContext);

    msg->getResponseData().setInstanceNames(instancePaths);
    return msg;
}


CIMOpenReferenceInstancesResponseMessage*
    CIMOperationResponseDecoder::_decodeOpenReferenceInstancesResponse(
        XmlParser& parser,
        const String& messageId,
        Boolean isEmptyImethodresponseTag)
{
    XmlEntry entry;
    CIMException cimException;

    Array<CIMInstance> namedInstances;
    Boolean endOfSequence = true;
    String enumerationContext = String::EMPTY;

    if (XmlReader::getErrorElement(parser, cimException))
    {
        return new CIMOpenReferenceInstancesResponseMessage(
            messageId,
            cimException,
            QueueIdStack(),
            endOfSequence,
            enumerationContext);
    }

    if (isEmptyImethodresponseTag)
    {
        throw PEGASUS_CIM_EXCEPTION(CIM_ERR_INVALID_PARAMETER,
            "Return Parameters endOfSequence"
                "and/or enumerationContext required.");
    }

    _decodeGetInstancesWithPathElement(parser, namedInstances);

    // Get the OUT parameters (endOfSequence and enumerationContext)
    _decodeOpenResponseParamValues(parser, endOfSequence, enumerationContext);

    CIMOpenReferenceInstancesResponseMessage* msg;

    msg = new CIMOpenReferenceInstancesResponseMessage(
        messageId,
        cimException,
        QueueIdStack(),
        endOfSequence,
        enumerationContext);

    // set response data type to Instances.  The default for this
    // message is OBJECTS since that is what we use in the server.
    msg->getResponseData().setDataType(CIMResponseData::RESP_INSTANCES);
    msg->getResponseData().setInstances(namedInstances);

    return msg;
}

CIMOpenReferenceInstancePathsResponseMessage*
    CIMOperationResponseDecoder::_decodeOpenReferenceInstancePathsResponse(
        XmlParser& parser,
        const String& messageId,
        Boolean isEmptyImethodresponseTag)
{
    XmlEntry entry;
    CIMException cimException;
    Array<CIMObjectPath> instancePaths;
    Boolean endOfSequence = true;
    String enumerationContext = String::EMPTY;

    if (XmlReader::getErrorElement(parser, cimException))
    {
        return new CIMOpenReferenceInstancePathsResponseMessage(
            messageId,
            cimException,
            QueueIdStack(),
            endOfSequence,
            enumerationContext);
    }
    if (isEmptyImethodresponseTag)
    {
        throw PEGASUS_CIM_EXCEPTION(CIM_ERR_INVALID_PARAMETER,
            "Return Parameters endOfSequence"
                "and/or enumerationContext required.");
    }

    _decodeInstancePathElements(parser, instancePaths);

    // Get the OUT parameters (endOfSequence and enumerationContext)
    _decodeOpenResponseParamValues(parser, endOfSequence, enumerationContext);

    CIMOpenReferenceInstancePathsResponseMessage* msg;

    msg = new CIMOpenReferenceInstancePathsResponseMessage(
        messageId,
        cimException,
        QueueIdStack(),
        endOfSequence,
        enumerationContext);

    msg->getResponseData().setInstanceNames(instancePaths);

    return msg;
}

CIMOpenAssociatorInstancesResponseMessage*
    CIMOperationResponseDecoder::_decodeOpenAssociatorInstancesResponse(
        XmlParser& parser,
        const String& messageId,
        Boolean isEmptyImethodresponseTag)
{
    XmlEntry entry;
    CIMException cimException;
    Array<CIMInstance> namedInstances;
    Boolean endOfSequence = true;
    String enumerationContext = String::EMPTY;

    if (XmlReader::getErrorElement(parser, cimException))
    {
        return new CIMOpenAssociatorInstancesResponseMessage(
            messageId,
            cimException,
            QueueIdStack(),
            endOfSequence,
            enumerationContext);
    }

    if (isEmptyImethodresponseTag)
    {
        throw PEGASUS_CIM_EXCEPTION(CIM_ERR_INVALID_PARAMETER,
            "Return Parameters endOfSequence"
                "and/or enumerationContext required.");
    }

    _decodeGetInstancesWithPathElement(parser, namedInstances);

    // Get the OUT parameters (endOfSequence and enumerationContext)
    _decodeOpenResponseParamValues(parser, endOfSequence, enumerationContext);

    CIMOpenAssociatorInstancesResponseMessage* msg;

    msg = new CIMOpenAssociatorInstancesResponseMessage(
        messageId,
        cimException,
        QueueIdStack(),
        endOfSequence,
        enumerationContext);

    msg->getResponseData().setDataType(CIMResponseData::RESP_INSTANCES);
    msg->getResponseData().setInstances(namedInstances);

    return msg;
}

CIMOpenAssociatorInstancePathsResponseMessage*
    CIMOperationResponseDecoder::_decodeOpenAssociatorInstancePathsResponse(
        XmlParser& parser,
        const String& messageId,
        Boolean isEmptyImethodresponseTag)
{
    XmlEntry entry;
    CIMException cimException;
    Array<CIMObjectPath> instancePaths;
    Boolean endOfSequence = true;
    String enumerationContext = String::EMPTY;

    if (XmlReader::getErrorElement(parser, cimException))
    {
        return new CIMOpenAssociatorInstancePathsResponseMessage(
            messageId,
            cimException,
            QueueIdStack(),
            endOfSequence,
            enumerationContext);
    }

    if (isEmptyImethodresponseTag)
    {
        throw PEGASUS_CIM_EXCEPTION(CIM_ERR_INVALID_PARAMETER,
            "Return Parameters endOfSequence"
                "and/or enumerationContext required.");
    }

    if (XmlReader::testStartTagOrEmptyTag(parser, entry, "IRETURNVALUE"))
    {
        if (entry.type != XmlEntry::EMPTY_TAG)
        {
            CIMObjectPath instancePath;

            while (XmlReader::getInstancePathElement(parser, instancePath))
                instancePaths.append(instancePath);
            XmlReader::expectEndTag(parser, "IRETURNVALUE");
        }
    }

    // Get the OUT parameters (endOfSequence and enumerationContext)
    //
    _decodeOpenResponseParamValues(parser, endOfSequence, enumerationContext);

    CIMOpenAssociatorInstancePathsResponseMessage* msg;

    msg = new CIMOpenAssociatorInstancePathsResponseMessage(
        messageId,
        cimException,
        QueueIdStack(),
        endOfSequence,
        enumerationContext);

    msg->getResponseData().setInstanceNames(instancePaths);

    return msg;
}

CIMOpenQueryInstancesResponseMessage*
    CIMOperationResponseDecoder::_decodeOpenQueryInstancesResponse(
        XmlParser& parser,
        const String& messageId,
        Boolean isEmptyImethodresponseTag)
{
    CIMException cimException;
    Array<CIMInstance> instances;
    CIMClass queryResultClass;
    Boolean endOfSequence = true;
    String enumerationContext = String::EMPTY;

    if (XmlReader::getErrorElement(parser, cimException))
    {
        return new CIMOpenQueryInstancesResponseMessage(
            messageId,
            cimException,
            CIMClass(),
            QueueIdStack(),
            endOfSequence,
            enumerationContext);
    }
    // EXP_PULL error out if response empty because either
    // enumerationContext or endOfSequence is required. Create
    // missing Parameter error here.
    if (isEmptyImethodresponseTag)
    {
        throw PEGASUS_CIM_EXCEPTION(CIM_ERR_INVALID_PARAMETER,
            "Return Parameters endOfSequence"
                "and/or enumerationContext required.");
    }

    _decodeGetInstancesElement(parser, instances);

    // Get the OUT parameters (endOfSequence and enumerationContext)
    _decodeOpenResponseParamValues(parser, endOfSequence, enumerationContext);

    CIMOpenQueryInstancesResponseMessage* msg =
        new CIMOpenQueryInstancesResponseMessage(
            messageId,
            cimException,
            queryResultClass,
            QueueIdStack(),
            endOfSequence,
            enumerationContext);

    msg->getResponseData().setInstances(instances);
    return msg;
}

CIMPullInstancesWithPathResponseMessage*
    CIMOperationResponseDecoder::_decodePullInstancesWithPathResponse(
        XmlParser& parser,
        const String& messageId,
        Boolean isEmptyImethodresponseTag)
{
    XmlEntry entry;
    CIMException cimException;
    Array<CIMInstance> namedInstances;
    Boolean endOfSequence = true;
    String enumerationContext = String::EMPTY;

    if (XmlReader::getErrorElement(parser, cimException))
    {
        return new CIMPullInstancesWithPathResponseMessage(
            messageId,
            cimException,
            QueueIdStack(),
            endOfSequence,
            enumerationContext);
    }

    if (isEmptyImethodresponseTag)
    {
        throw PEGASUS_CIM_EXCEPTION(CIM_ERR_INVALID_PARAMETER,
            "Return Parameters endOfSequence"
                "and/or enumerationContext required.");
    }
    _decodeGetInstancesWithPathElement(parser, namedInstances);

    _decodeOpenResponseParamValues(parser, endOfSequence, enumerationContext);

    CIMPullInstancesWithPathResponseMessage* msg;

    msg = new CIMPullInstancesWithPathResponseMessage(
        messageId,
        cimException,
        QueueIdStack(),
        endOfSequence,
        enumerationContext);

    msg->getResponseData().setInstances(namedInstances);
    return msg;
}

CIMPullInstancePathsResponseMessage*
    CIMOperationResponseDecoder::_decodePullInstancePathsResponse(
        XmlParser& parser,
        const String& messageId,
        Boolean isEmptyImethodresponseTag)
{
    XmlEntry entry;
    CIMException cimException;
    Array<CIMObjectPath> instancePaths;
    Boolean endOfSequence = true;
    String enumerationContext = String::EMPTY;

    //Boolean duplicateParameter = false;
    //Boolean gotEndOfSequence = false;
    //Boolean gotEnumerationContext = false;

    if (XmlReader::getErrorElement(parser, cimException))
    {
        return new CIMPullInstancePathsResponseMessage(
            messageId,
            cimException,
            QueueIdStack(),
            endOfSequence,
            enumerationContext);
    }

    if (isEmptyImethodresponseTag)
    {
        throw PEGASUS_CIM_EXCEPTION(CIM_ERR_INVALID_PARAMETER,
            "Return Parameters endOfSequence"
                "and/or enumerationContext required.");
    }

    _decodeInstancePathElements(parser, instancePaths);

    _decodeOpenResponseParamValues(parser, endOfSequence, enumerationContext);

    CIMPullInstancePathsResponseMessage* msg;

    msg = new CIMPullInstancePathsResponseMessage(
        messageId,
        cimException,
        QueueIdStack(),
        endOfSequence,
        enumerationContext);

    msg->getResponseData().setInstanceNames(instancePaths);
    return msg;
}

CIMPullInstancesResponseMessage*
    CIMOperationResponseDecoder::_decodePullInstancesResponse(
        XmlParser& parser,
        const String& messageId,
        Boolean isEmptyImethodresponseTag)
{
    XmlEntry entry;
    CIMException cimException;
    Array<CIMInstance> instances;
    Boolean endOfSequence = true;
    String enumerationContext = String::EMPTY;

    if (XmlReader::getErrorElement(parser, cimException))
    {
        return new CIMPullInstancesResponseMessage(
            messageId,
            cimException,
            QueueIdStack(),
            endOfSequence,
            enumerationContext);
    }

    if (isEmptyImethodresponseTag)
    {
        throw PEGASUS_CIM_EXCEPTION(CIM_ERR_INVALID_PARAMETER,
            "Return Parameters endOfSequence"
                "and/or enumerationContext required.");
    }
    _decodeGetInstancesElement(parser, instances);

    _decodeOpenResponseParamValues(parser, endOfSequence, enumerationContext);

    CIMPullInstancesResponseMessage* msg;

    msg = new CIMPullInstancesResponseMessage(
        messageId,
        cimException,
        QueueIdStack(),
        endOfSequence,
        enumerationContext);

    msg->getResponseData().setInstances(instances);
    return msg;
}

CIMCloseEnumerationResponseMessage*
    CIMOperationResponseDecoder::_decodeCloseEnumerationResponse(
        XmlParser& parser,
        const String& messageId,
        Boolean isEmptyImethodresponseTag)
{
    XmlEntry entry;
    CIMException cimException;
    Array<CIMObjectPath> instanceNames;
    String enumerationContext = String::EMPTY;

    if (XmlReader::getErrorElement(parser, cimException))
    {
        return new CIMCloseEnumerationResponseMessage(
            messageId,
            cimException,
            QueueIdStack());
    }

    return new CIMCloseEnumerationResponseMessage(
        messageId,
        cimException,
        QueueIdStack());
}

CIMEnumerationCountResponseMessage*
    CIMOperationResponseDecoder::_decodeEnumerationCountResponse(
        XmlParser& parser,
        const String& messageId,
        Boolean isEmptyImethodresponseTag)
{
    XmlEntry entry;
    CIMException cimException;
    Uint64Arg count;

    Boolean duplicateParameter = false;
    Boolean gotCount = false;

    if (XmlReader::getErrorElement(parser, cimException))
    {
        return new CIMEnumerationCountResponseMessage(
            messageId,
            cimException,
            QueueIdStack(),
            0);
    }

    if (isEmptyImethodresponseTag)
    {
        throw PEGASUS_CIM_EXCEPTION(CIM_ERR_INVALID_PARAMETER,
            "Return Parameters endOfSequence"
                "and/or enumerationContext required.");
    }

    // Extract the parameter count from the message

    Boolean emptyTag;
    for (const char* name;
         XmlReader::getIReturnValueTag(parser, name, emptyTag); )
    {
        if (System::strcasecmp(name, "Count") == 0)
        {
            XmlReader::getUint64ValueElement(parser, count, true);
            //duplicateParameter = gotCount;
            gotCount = true;
        }

        else
        {
            /// EXP_PULL_TBD
            // We probably simply want to ignore this as an extra tag
        }
        if (!emptyTag)
        {
            XmlReader::expectEndTag(parser, "IRETURNVALUE");
        }

        if (duplicateParameter)
        {
            throw PEGASUS_CIM_EXCEPTION(
                CIM_ERR_INVALID_PARAMETER, String::EMPTY);
        }

        // EXP_PULL_TBD add test here for the required parameters
        // NOT sure from the spec if the parameter is required or not.

        if (!gotCount)
        {
            throw PEGASUS_CIM_EXCEPTION(CIM_ERR_INVALID_PARAMETER,
                                        "Return value missing");
        }
    }
    return new CIMEnumerationCountResponseMessage(
        messageId,
        cimException,
        QueueIdStack(),
        count);
}

//EXP_PULL_END

PEGASUS_NAMESPACE_END
