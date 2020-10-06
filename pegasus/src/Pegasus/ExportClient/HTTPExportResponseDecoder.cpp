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
#include <Pegasus/Common/Constants.h>
#include <Pegasus/Common/XmlReader.h>
#include <Pegasus/Common/System.h>
#include <Pegasus/Common/CIMMessage.h>
#include <Pegasus/Common/MessageLoader.h>
#include <Pegasus/Common/Tracer.h>
#include <Pegasus/Client/CIMClientException.h>
#include "HTTPExportResponseDecoder.h"
#ifdef PEGASUS_ENABLE_PROTOCOL_WSMAN
#include <Pegasus/WsmServer/WsmConstants.h>
#include <Pegasus/WsmServer/WsmReader.h>
#include <Pegasus/WsmServer/WsmFault.h>
#include <Pegasus/WsmServer/WsmRequestDecoder.h>
#include <Pegasus/WsmServer/WsmEndpointReference.h>
#endif

PEGASUS_USING_STD;
PEGASUS_NAMESPACE_BEGIN

void HTTPExportResponseDecoder::parseHTTPHeaders(
    HTTPMessage* httpMessage,
    ClientExceptionMessage*& exceptionMessage,
    Array<HTTPHeader>& headers,
    Uint32& contentLength,
    Uint32& statusCode,
    String& reasonPhrase,
    Boolean& cimReconnect,
    Boolean& valid)
{
    PEG_METHOD_ENTER(TRC_EXPORT_CLIENT,
        "HTTPExportResponseDecoder::parseHTTPHeaders()");

    exceptionMessage = 0;
    headers.clear();
    contentLength = 0;
    statusCode = 0;
    reasonPhrase = String::EMPTY;
    cimReconnect = false;
    valid = true;

    String startLine;

    //
    //  Check for empty HTTP response message
    //
    if (httpMessage->message.size() == 0)
    {
        MessageLoaderParms mlParms(
            "ExportClient.HTTPExportResponseDecoder.EMPTY_RESPONSE",
            "Connection closed by CIM Server.");
        String mlString(MessageLoader::getMessage(mlParms));
        AutoPtr<CIMClientMalformedHTTPException> malformedHTTPException(
            new CIMClientMalformedHTTPException(mlString));
        AutoPtr<ClientExceptionMessage> response(
            new ClientExceptionMessage(malformedHTTPException.get()));
        malformedHTTPException.release();
        exceptionMessage = response.release();
        valid = false;

        PEG_METHOD_EXIT();
        return;
    }

    //
    // Parse the HTTP message headers and get content length
    //
    httpMessage->parse(startLine, headers, contentLength);

    //
    // Check for Connection: Close
    //
    const char* connectClose;
    if (HTTPMessage::lookupHeader(headers, "Connection", connectClose, false))
    {
        if (System::strcasecmp(connectClose, "Close") == 0)
        {
            // reconnect and then resend next request.
            cimReconnect=true;
        }
    }

    PEG_TRACE_CSTRING(TRC_XML_IO, Tracer::LEVEL4,
                      httpMessage->message.getData());

    //
    //  Parse HTTP message status line
    //
    String httpVersion;

    Boolean parsableMessage = HTTPMessage::parseStatusLine(
        startLine, httpVersion, statusCode, reasonPhrase);
    if (!parsableMessage)
    {
        MessageLoaderParms mlParms(
            "ExportClient.HTTPExportResponseDecoder.MALFORMED_RESPONSE",
            "Malformed HTTP response message.");
        String mlString(MessageLoader::getMessage(mlParms));
        AutoPtr<CIMClientMalformedHTTPException> malformedHTTPException(
            new CIMClientMalformedHTTPException(mlString));
        AutoPtr<ClientExceptionMessage> response(
            new ClientExceptionMessage(malformedHTTPException.get()));
        malformedHTTPException.release();
        response->setCloseConnect(cimReconnect);
        exceptionMessage = response.release();
        valid = false;

        PEG_METHOD_EXIT();
        return;
    }

    PEG_METHOD_EXIT();
}

void HTTPExportResponseDecoder::validateHTTPHeaders(
    HTTPMessage* httpMessage,
    Array<HTTPHeader>& headers,
    Uint32 contentLength,
    Uint32 statusCode,
    Boolean cimReconnect,
    const String& reasonPhrase,
    char*& content,
    ClientExceptionMessage*& exceptionMessage,
    Boolean& valid,
    Boolean wsmanFlag)
{
    PEG_METHOD_ENTER(TRC_EXPORT_CLIENT,
        "HTTPExportResponseDecoder::validateHTTPHeaders()");

    content = 0;
    exceptionMessage = 0;
    valid = true;

    //
    // If authentication failed, a CIMClientHTTPErrorException will be
    // generated with the "401 Unauthorized" status in the (re-challenge)
    // response
    //
    // Check for a non-success (200 OK) response
    //
    if (statusCode != HTTP_STATUSCODE_OK)
    {
        String cimError;
        String pegasusError;

        HTTPMessage::lookupHeader(headers, "CIMError", cimError);
        HTTPMessage::lookupHeader(headers, PEGASUS_HTTPHEADERTAG_ERRORDETAIL,
            pegasusError);
        try
        {
            pegasusError = XmlReader::decodeURICharacters(pegasusError);
        }
        catch (const ParseError&)
        {
            // Ignore this exception.  We're more interested in having the
            // message in encoded form than knowing that the format is invalid.
        }

        AutoPtr<CIMClientHTTPErrorException> httpError(
            new CIMClientHTTPErrorException(statusCode, reasonPhrase, cimError,
                pegasusError));
        AutoPtr<ClientExceptionMessage> response(
            new ClientExceptionMessage(httpError.get()));

        httpError.release();
        response->setCloseConnect(cimReconnect);
        exceptionMessage = response.release();
        valid = false;

        PEG_METHOD_EXIT();
        return;
    }
    const char* cimExport;
    //
    // Check for missing "CIMExport" header
    //
    if (!wsmanFlag && 
        !HTTPMessage::lookupHeader(headers,"CIMExport",cimExport,true))
    {
        MessageLoaderParms mlParms(
            "ExportClient.CIMExportResponseDecoder.MISSING_CIMEXP_HEADER",
            "Missing CIMExport HTTP header");
        String mlString(MessageLoader::getMessage(mlParms));

        AutoPtr<CIMClientMalformedHTTPException> malformedHTTPException(new
            CIMClientMalformedHTTPException(mlString));

        AutoPtr<ClientExceptionMessage> response(
            new ClientExceptionMessage(malformedHTTPException.get()));

        malformedHTTPException.release();
        response->setCloseConnect(cimReconnect);
        exceptionMessage = response.release();
        valid = false;

        PEG_METHOD_EXIT();
        return;
    }

    //
    // Check for missing "Content-Type" header
    //
    const char* cimContentType;
    if (!HTTPMessage::lookupHeader(
            headers, "Content-Type", cimContentType, true))
    {
        AutoPtr<CIMClientMalformedHTTPException> malformedHTTPException(new
            CIMClientMalformedHTTPException(
                "Missing CIMContentType HTTP header"));
        AutoPtr<ClientExceptionMessage> response(
            new ClientExceptionMessage(malformedHTTPException.get()));

        malformedHTTPException.release();
        response->setCloseConnect(cimReconnect);
        exceptionMessage = response.release();
        valid = false;

        PEG_METHOD_EXIT();
        return;
    }

    //
    // Calculate the beginning of the content from the message size and
    // the content length
    //
    content = (char *) httpMessage->message.getData() +
        httpMessage->message.size() - contentLength;

    //
    // Expect CIMExport HTTP header value MethodResponse
    //

    if ( !wsmanFlag && System::strcasecmp(cimExport, "MethodResponse") != 0)
    {

        MessageLoaderParms mlParms(
            "ExportClient.CIMExportResponseDecoder.EXPECTED_METHODRESPONSE",
            "Received CIMExport HTTP header value \"$0\", "
            "expected \"MethodResponse\"", cimExport);
        String mlString(MessageLoader::getMessage(mlParms));

        AutoPtr<CIMClientMalformedHTTPException> malformedHTTPException(
            new CIMClientMalformedHTTPException(mlString));

        AutoPtr<ClientExceptionMessage> response(
            new ClientExceptionMessage(malformedHTTPException.get()));

        malformedHTTPException.release();
        response->setCloseConnect(cimReconnect);
        exceptionMessage = response.release();
        valid = false;

        PEG_METHOD_EXIT();
        return;
    } 

    PEG_METHOD_EXIT();
}

void HTTPExportResponseDecoder::decodeExportResponse(
    char* content,
    Boolean cimReconnect,
    Message*& responseMessage)
{
    PEG_METHOD_ENTER (TRC_EXPORT_CLIENT,
        "HTTPExportResponseDecoder::decodeExportResponse()");

    AutoPtr<Message> response;

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
        XmlReader::getCimStartTag(parser, cimVersion, dtdVersion);

        //
        // Expect <MESSAGE ... >
        //
        String messageId;
        String protocolVersion;
        if (!XmlReader::getMessageStartTag(parser, messageId, protocolVersion))
        {
            MessageLoaderParms mlParms(
                "ExportClient.CIMExportResponseDecoder."
                    "EXPECTED_MESSAGE_ELEMENT",
                "expected MESSAGE element");
            String mlString(MessageLoader::getMessage(mlParms));

            PEG_METHOD_EXIT();
            throw XmlValidationError(parser.getLine(), mlString);
        }

        //
        // Check for unsupported protocol version
        //

        if (!XmlReader::isSupportedProtocolVersion(protocolVersion))
        {
            MessageLoaderParms mlParms(
                "ExportClient.CIMExportResponseDecoder.UNSUPPORTED_PROTOCOL",
                "Received unsupported protocol version \"$0\", expected \"$1\"",
                protocolVersion, "1.0");
                String mlString(MessageLoader::getMessage(mlParms));

            AutoPtr<CIMClientResponseException> responseException(
                new CIMClientResponseException(mlString));

            AutoPtr<ClientExceptionMessage> clientExceptionMessage(
                new ClientExceptionMessage(responseException.get()));

            responseException.release();
            clientExceptionMessage->setCloseConnect(cimReconnect);
            responseMessage = clientExceptionMessage.release();

            PEG_METHOD_EXIT();
            return;
        }

        //
        // Expect <SIMPLEEXPRSP ... >
        //
        XmlReader::expectStartTag(parser, entry, "SIMPLEEXPRSP");

        //
        // Expect <EXPMETHODRESPONSE ... >
        //
        const char* expMethodResponseName = 0;
        Boolean isEmptyTag = false;

        if (XmlReader::getEMethodResponseStartTag(
            parser, expMethodResponseName, isEmptyTag))
        {
            if (System::strcasecmp(expMethodResponseName, "ExportIndication")
                == 0)
            {
                response.reset(_decodeExportIndicationResponse(
                    parser, messageId, isEmptyTag));
            }
            else
            {
                //
                //  Unrecognized ExpMethodResponse name attribute
                //
                MessageLoaderParms mlParms(
                    "ExportClient.CIMExportResponseDecoder."
                        "UNRECOGNIZED_EXPMETHRSP",
                    "Unrecognized ExpMethodResponse name \"$0\"",
                    expMethodResponseName);
                String mlString(MessageLoader::getMessage(mlParms));

                PEG_METHOD_EXIT();
                throw XmlValidationError(parser.getLine(), mlString);
            }

            //
            // Handle end tag:
            //
            if (!isEmptyTag)
            {
                XmlReader::expectEndTag(parser, "EXPMETHODRESPONSE");
            }
        }
        else
        {
            //
            //  Expected ExpMethodResponse element
            //
            MessageLoaderParms mlParms(
                "ExportClient.CIMExportResponseDecoder."
                    "EXPECTED_EXPMETHODRESPONSE_ELEMENT",
                "expected EXPMETHODRESPONSE element");
            String mlString(MessageLoader::getMessage(mlParms));

            PEG_METHOD_EXIT();
            throw XmlValidationError(parser.getLine(), mlString);
        }

        //
        // Handle end tags:
        //
        XmlReader::expectEndTag(parser, "SIMPLEEXPRSP");
        XmlReader::expectEndTag(parser, "MESSAGE");
        XmlReader::expectEndTag(parser, "CIM");
    }
    catch (XmlException& x)
    {
        response.reset(new ClientExceptionMessage(
            new CIMClientXmlException(x.getMessage())));
    }
    catch (Exception& x)
    {
        response.reset(new ClientExceptionMessage(
            new CIMClientResponseException(x.getMessage())));
    }

    //
    //  Note: Ignore any ContentLanguage set in the export response
    //

    response->setCloseConnect(cimReconnect);
    responseMessage = response.release();

    PEG_METHOD_EXIT();
}

CIMExportIndicationResponseMessage*
HTTPExportResponseDecoder::_decodeExportIndicationResponse(
    XmlParser& parser,
    const String& messageId,
    Boolean isEmptyExpMethodResponseTag)
{
    PEG_METHOD_ENTER (TRC_EXPORT_CLIENT,
        "HTTPExportResponseDecoder::_decodeExportIndicationResponse()");

    XmlEntry entry;
    CIMException cimException;

    if (!isEmptyExpMethodResponseTag)
    {
        if (XmlReader::getErrorElement(parser, cimException))
        {
            PEG_METHOD_EXIT();
            return(new CIMExportIndicationResponseMessage(
                messageId,
                cimException,
                QueueIdStack()));
        }

        if (XmlReader::testStartTagOrEmptyTag(parser, entry, "IRETURNVALUE"))
        {
            if (entry.type != XmlEntry::EMPTY_TAG)
            {
                XmlReader::expectEndTag(parser, "IRETURNVALUE");
            }
        }
    }

    PEG_METHOD_EXIT();
    return(new CIMExportIndicationResponseMessage(
        messageId,
        cimException,
        QueueIdStack()));
}

#ifdef PEGASUS_ENABLE_PROTOCOL_WSMAN
inline void checkRequiredHeader(
    const char* headerName,
    Boolean headerSpecified)
{
    if (!headerSpecified)
    {
        throw WsmFault(
            WsmFault::wsa_MessageInformationHeaderRequired,
            MessageLoaderParms(
                "ExportClient.HTTPExportResponseDecoder.MISSING_HEADER",
                "Required SOAP header \"$0\" was not specified.",
                headerName));
    }
}


void HTTPExportResponseDecoder::decodeWSMANExportResponse(
    char* content,
    Boolean reconnect,
    Message*& responseMessage,
    ContentLanguageList & contentLanguages,
    WsmRequest * request)
{
    PEG_METHOD_ENTER (TRC_EXPORT_CLIENT,
        "HTTPExportResponseDecoder::decodeWSMANExportResponse()");
    AutoPtr<Message> response;
    // Create and initialize XML parser:
    XmlParser parser((char*)content);
    WsmReader wsmReader(content);
    XmlEntry entry;
    String wsaAction;
    String wsaTo;
    String wsaRelatesTo,messageId;
    WsmEndpointReference epr;
    try
    {
        // Process <?xml ... >
        const char* xmlVersion = 0;
        const char* xmlEncoding = 0;
        wsmReader.getXmlDeclaration(xmlVersion, xmlEncoding);
        // Decode the SOAP envelope
        wsmReader.expectStartTag(
            entry, WsmNamespaces::SOAP_ENVELOPE, "Envelope");
        Boolean gotEntry;
        String wsaMessageId;
        wsmReader.setHideEmptyTags(true);
        wsmReader.expectStartTag(entry, WsmNamespaces::SOAP_ENVELOPE, "Header");
        wsmReader.setHideEmptyTags(false);

        while ((gotEntry = wsmReader.next(entry)) &&
            ((entry.type == XmlEntry::START_TAG) ||
            (entry.type == XmlEntry::EMPTY_TAG)))
        {
            int nsType = entry.nsType;
            const char* elementName = entry.localName;
            Boolean needEndTag = (entry.type == XmlEntry::START_TAG);

            if ((nsType == WsmNamespaces::WS_ADDRESSING) &&
                (strcmp(elementName, "To") == 0))
            {
                wsmReader.checkDuplicateHeader(entry.text, wsaTo.size());
                wsaTo = wsmReader.getElementContent(entry);
            }
            else if ((nsType == WsmNamespaces::WS_ADDRESSING) &&
                (strcmp(elementName, "Action") == 0))
            {
                wsmReader.checkDuplicateHeader(entry.text, wsaAction.size());
                wsaAction = wsmReader.getElementContent(entry);
            }
            else if ((nsType == WsmNamespaces::WS_ADDRESSING) &&
                (strcmp(elementName, "MessageID") == 0))
            {
                wsmReader.checkDuplicateHeader(entry.text, messageId.size());
                messageId = wsmReader.getElementContent(entry);         
            }
            else if ((nsType == WsmNamespaces::WS_ADDRESSING) &&
                (strcmp(elementName, "RelatesTo") == 0))
            {
                wsmReader.checkDuplicateHeader(entry.text, wsaMessageId.size());
                wsaRelatesTo = wsmReader.getElementContent(entry);
            }
            else
            {
                wsmReader.skipElement(entry);
                // The end tag, if any, has already been consumed.
                needEndTag = false;
            }

            if (needEndTag)
            {
                wsmReader.expectEndTag(nsType, elementName);
            }
        }
        if (gotEntry)
        {
            wsmReader.getParser().putBack(entry);
        }
        wsmReader.expectEndTag(WsmNamespaces::SOAP_ENVELOPE, "Header");
    }
    catch (XmlException&)
    {
        // Do not treat this as an InvalidMessageInformationHeader fault.
        throw;
    }
    catch (Exception& e)
    {
        throw WsmFault(
            WsmFault::wsa_InvalidMessageInformationHeader,
            e.getMessage(),
            e.getContentLanguages());
    }
    checkRequiredHeader("wsa:To", wsaTo.size());
    checkRequiredHeader("wsa:RelatesTo", wsaRelatesTo.size());
    checkRequiredHeader("wsa:Action", wsaAction.size());
    if (wsaAction == WSM_ACTION_ACK)
    {
        response.reset(new WSMANExportIndicationResponseMessage(
        messageId,
        request,
        contentLanguages));   
    }   
    response->setCloseConnect(reconnect);
    responseMessage = response.release();
    PEG_METHOD_EXIT();

}
#endif
PEGASUS_NAMESPACE_END
