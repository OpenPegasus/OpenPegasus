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
#include "CIMOperationRequestDecoder.h"
#include <Pegasus/Common/CommonUTF.h>
#include <Pegasus/Common/MessageLoader.h>
#include <Pegasus/Common/BinaryCodec.h>
#include <Pegasus/Common/OperationContextInternal.h>
#include <Pegasus/General/CIMError.h>

#ifdef PEGASUS_PAM_SESSION_SECURITY
#include <Pegasus/Security/Authentication/PAMSessionBasicAuthenticator.h>
#include <Pegasus/Security/Authentication/AuthenticationStatus.h>
#endif


PEGASUS_USING_STD;

PEGASUS_NAMESPACE_BEGIN

/******************************************************************************
**
**             Local Functions
**
******************************************************************************/


/******************************************************************************
**
**            Common Exception Messages. Reduce the
**            number of throws to reduce code size.
**
******************************************************************************/

//
// throw CIM_ERR_NOT_SUPPORTED with optional added text
void _throwCIMExceptionCIMErrNotSupported(const String& param = String::EMPTY)
{
    throw PEGASUS_CIM_EXCEPTION(CIM_ERR_NOT_SUPPORTED, param);
}

// throw internationalized PEGASUS_CIM_EXCEPTION_L
void _throwCIMExceptionCIMErrNotSupported(const MessageLoaderParms& mlp)
{
    throw PEGASUS_CIM_EXCEPTION_L(CIM_ERR_NOT_SUPPORTED, mlp);
}

// Throw CIM_ERR_INVALID_PARAMETER with optional parameter name(s)
void _throwCIMExceptionInvalidParameter(const String& param = String::EMPTY)
{
    throw PEGASUS_CIM_EXCEPTION(CIM_ERR_INVALID_PARAMETER, param);
}

// Throw CIM_ERR_INVALID_PARAMETER with optional parameter name(s)
void _throwCIMExceptionInvalidParameter(const MessageLoaderParms& mlp)
{
    throw PEGASUS_CIM_EXCEPTION_L(CIM_ERR_INVALID_PARAMETER, mlp);
}

// Common call for all cases where duplicate Input parameter Values recieved
// in a single operation. Throw InvalidParameter exception with optional
// additional data.
void _throwCIMExceptionDuplicateParameter(const String& name = String::EMPTY)
{
    _throwCIMExceptionInvalidParameter(MessageLoaderParms(
        "Server.CIMOperationRequestDecoder."
                "DUPLICATE_PARAMETER",
        "Duplicated request input parameter."));
}

// Common call for cases where Invalid IParameterValue names recived
void _throwCIMExceptionInvalidIParamName(const String& name)
{
    _throwCIMExceptionCIMErrNotSupported(MessageLoaderParms(
        "Server.CIMOperationRequestDecoder."
            "INVALID_PARAMETER",
        "Unrecognized or invalid request input parameter \"$0\"", name));
}

// Throw CIM_ERR_ENUMERATION_CONTEXT_REQUIRED
void _throwCIMExceptionEnumerationContextRequired()
{
    _throwCIMExceptionInvalidParameter(MessageLoaderParms(
        "Server.CIMOperationRequestDecoder."
                "ENUMERATION_CONTEXT_REQUIRED",
        "The EnumerationContext input parameter is required."));
}

// test if Required parameters exist (i.e. the got variable is
// true. Generates exception if exist == false

void _throwCIMExceptionRequiredDoesNotExist(const String& name)
{
    _throwCIMExceptionInvalidParameter(MessageLoaderParms(
    "Server.CIMOperationRequestDecoder."
        "REQUIRED_PARAMETER_MISSING",
    "Required parameter \"$0\" missing from request.", name));
}

// FUTURE - Issues message with no name attached.
// We want to get rid of this one completely but there are some calls
void _testRequiredParametersExist(Boolean exist)
{
    if (!exist)
    {
        _throwCIMExceptionInvalidParameter();
    }
}

void _testRequiredParametersExist(const char* name, Boolean exist)
{
    if (!exist)
    {
        _throwCIMExceptionRequiredDoesNotExist(String(name));
    }
}

void _testRequiredParametersExist(const String& name, Boolean exist)
{
    if (!exist)
    {
        _throwCIMExceptionRequiredDoesNotExist(name);
    }
}
/******************************************************************************
**
**            CIMOperationRequestDecoder Class
**
******************************************************************************/

#ifdef PEGASUS_PAM_SESSION_SECURITY
void CIMOperationRequestDecoder::_updateExpiredPassword(
    Uint32 queueId,
    HttpMethod httpMethod,
    const String& messageId,
    Boolean closeConnect,
    const ContentLanguageList& httpContentLanguages,
    CIMMessage * request,
    const String& userName,
    const String& oldPass,
    const String& ipAddress)
{
    static CIMName meth = CIMName("UpdateExpiredPassword");
    static CIMName clName = CIMName("PG_Account");


    // this has to be an invokeMethod and
    if (CIM_INVOKE_METHOD_REQUEST_MESSAGE != request->getType())
    {
        sendHttpError(
            queueId,
            HTTP_STATUS_BADREQUEST,
            "invalid header",
            "Header \'Pragma: UpdateExpiredPassword\' not valid for this "
                "CIMMethod.",
            closeConnect);

        return;
    }

    CIMInvokeMethodRequestMessage* msg =
        dynamic_cast<CIMInvokeMethodRequestMessage*>(request);

    // class PG_Account
    // method UpdateExpiredPassword
    // InterOp namespace
    if ((!clName.equal(msg->className)) ||
        (!(meth.equal(msg->methodName))) ||
        (!msg->nameSpace.equal(PEGASUS_NAMESPACENAME_INTEROP.getString())))
    {
        // not of interest for us, chicken out
        sendHttpError(
            queueId,
            HTTP_STATUS_BADREQUEST,
            "invalid header",
            "Header \'Pragma: UpdateExpiredPassword\' not valid for this "
                "class, method or namespace.",
            closeConnect);
        return;
    }

    String newPass;
    Boolean found = false;

    try
    {
        // Get new password from request - String-type Parameter UserPassword
        Array<CIMParamValue> inParm = msg->inParameters;
        for (Uint32 i=0; i < inParm.size(); i++)
        {
            CIMParamValue x = inParm[i];
            if (String::equalNoCase(x.getParameterName(),"UserPassword"))
            {
                CIMValue passValue = x.getValue();
                passValue.get(newPass);
                found = true;
                break;
            }
        }
    } catch(Exception &e)
    {
        // already know it is an invokeMethod, see checks above
        sendMethodError(
            queueId,
            httpMethod,
            messageId,
            meth.getString(),
            PEGASUS_CIM_EXCEPTION(
                CIM_ERR_INVALID_PARAMETER, e.getMessage()),
            closeConnect);
        return;
    }
    if (!found)
    {
        sendMethodError(
            queueId,
            httpMethod,
            messageId,
            meth.getString(),
            PEGASUS_CIM_EXCEPTION(
                CIM_ERR_INVALID_PARAMETER, "Missing Parameter UserPassword"),
            closeConnect);
        return;
    }
    // Call password update function from PAMSession.h
    AuthenticationStatus authStat =
        PAMSessionBasicAuthenticator::updateExpiredPassword(
            userName,
            oldPass,
            newPass,
            ipAddress);

    if (authStat.isSuccess())
    {
        // Send success message
        Buffer message;
        Buffer emptyBody;

        XmlWriter::appendReturnValueElement(
            emptyBody,
            CIMValue((Uint8)authStat.getMethodReturnCode()));

        message = XmlWriter::formatSimpleMethodRspMessage(
            meth,
            messageId,
            httpMethod,
            httpContentLanguages,
            emptyBody,
            0,
            true,
            true);

        sendResponse(queueId, message,closeConnect);
    }
    else
    {
        sendHttpError(
            queueId,
            authStat.getHttpStatus(),
            String::EMPTY,
            authStat.getErrorDetail(),
            closeConnect);
    }
}
#endif

CIMOperationRequestDecoder::CIMOperationRequestDecoder(
    MessageQueue* outputQueue,
    Uint32 returnQueueId)
    : Base(PEGASUS_QUEUENAME_OPREQDECODER),
      _outputQueue(outputQueue),
      _returnQueueId(returnQueueId),
      _serverTerminating(false)
{
}

CIMOperationRequestDecoder::~CIMOperationRequestDecoder()
{
}

void CIMOperationRequestDecoder::sendResponse(
    Uint32 queueId,
    Buffer& message,
    Boolean closeConnect)
{
    MessageQueue* queue = MessageQueue::lookup(queueId);

    if (queue)
    {
        AutoPtr<HTTPMessage> httpMessage(new HTTPMessage(message));
        httpMessage->setCloseConnect(closeConnect);
        queue->enqueue(httpMessage.release());
    }
}

void CIMOperationRequestDecoder::sendIMethodError(
    Uint32 queueId,
    HttpMethod httpMethod,
    const String& messageId,
    const String& iMethodName,
    const CIMException& cimException,
    Boolean closeConnect)
{
    Buffer message;
    message = XmlWriter::formatSimpleIMethodErrorRspMessage(
        iMethodName,
        messageId,
        httpMethod,
        cimException);

    sendResponse(queueId, message,closeConnect);
}

void CIMOperationRequestDecoder::sendMethodError(
    Uint32 queueId,
    HttpMethod httpMethod,
    const String& messageId,
    const String& methodName,
    const CIMException& cimException,
    Boolean closeConnect)
{
    Buffer message;
    message = XmlWriter::formatSimpleMethodErrorRspMessage(
        methodName,
        messageId,
        httpMethod,
        cimException);

    sendResponse(queueId, message,closeConnect);
}

void CIMOperationRequestDecoder::sendUserAccountExpired(
    Uint32 queueId,
    HttpMethod httpMethod,
    const String& messageId,
    const String& methodName,
    Boolean closeConnect,
    Boolean isIMethod)
{
    Buffer message;

    CIMError errorInst;
    errorInst.setErrorType(CIMError::ERROR_TYPE_OTHER);
    errorInst.setOtherErrorType("Expired Password");
    errorInst.setProbableCause(CIMError::PROBABLE_CAUSE_AUTHENTICATION_FAILURE);

    CIMException myExc(
        CIM_ERR_ACCESS_DENIED,
        "User Account Expired",
        errorInst.getInstance());

    if (isIMethod)
    {
        message = XmlWriter::formatSimpleIMethodErrorRspMessage(
            methodName,
            messageId,
            httpMethod,
            myExc);
    }
    else
    {
        message = XmlWriter::formatSimpleMethodErrorRspMessage(
            methodName,
            messageId,
            httpMethod,
            myExc);
    }

    sendResponse(queueId, message,closeConnect);
}

void CIMOperationRequestDecoder::sendHttpError(
    Uint32 queueId,
    const String& status,
    const String& cimError,
    const String& pegasusError,
    Boolean closeConnect)
{
    Buffer message;
    message = XmlWriter::formatHttpErrorRspMessage(
        status,
        cimError,
        pegasusError);

    sendResponse(queueId, message,closeConnect);
}

void CIMOperationRequestDecoder::handleEnqueue(Message* message)
{
    if (!message)
        return;

    switch (message->getType())
    {
        case HTTP_MESSAGE:
             handleHTTPMessage((HTTPMessage*)message);
             break;

        default:
            // Unexpected message type
            PEGASUS_UNREACHABLE(PEGASUS_ASSERT(0);)
            break;
    }

    delete message;
}


void CIMOperationRequestDecoder::handleEnqueue()
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
//     73-CIMOperation: MethodCall
//     73-CIMMethod: EnumerateInstances
//     73-CIMObject: root/cimv2
//
//------------------------------------------------------------------------------

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

    // Save userName, userRole, userPass and authType:

    String userName;
    String userRole;
    String userPass;
    Boolean isExpiredPassword = false;
    Boolean updateExpiredPassword;
    String authType;
    Boolean closeConnect = httpMessage->getCloseConnect();

    PEG_TRACE((
        TRC_HTTP,
        Tracer::LEVEL4,
        "CIMOperationRequestDecoder::handleHTTPMessage()- "
        "httpMessage->getCloseConnect() returned %d",
        closeConnect));

    userName = httpMessage->authInfo->getAuthenticatedUser();
    authType = httpMessage->authInfo->getAuthType();
    userRole = httpMessage->authInfo->getUserRole();
    userPass = httpMessage->authInfo->getAuthenticatedPassword();

#ifdef PEGASUS_PAM_SESSION_SECURITY
    isExpiredPassword = httpMessage->authInfo->isExpiredPassword();
#endif

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
    PEGASUS_ASSERT(!((httpMethod == HTTP_METHOD_M_POST) &&
                     (httpVersion == "HTTP/1.0")));

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
                "Server.CIMOperationRequestDecoder.MISSING_HOST_HEADER",
                "HTTP request message lacks a Host header field.");
            sendHttpError(
                queueId,
                HTTP_STATUS_BADREQUEST,
                "",
                MessageLoader::getMessage(parms),
                closeConnect);
            PEG_METHOD_EXIT();
            return;
        }
    }

    // Validate the "CIMOperation" header:

    const char* cimOperation;

    // If the CIMOperation header was missing, the HTTPAuthenticatorDelegator
    // would not have passed the message to us.
    PEGASUS_FCT_EXECUTE_AND_ASSERT(
        true,
        HTTPMessage::lookupHeader(headers, "CIMOperation", cimOperation, true));

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
            "CIMOperation value \"$0\" is not supported.",cimOperation);
        sendHttpError(
            queueId,
            HTTP_STATUS_BADREQUEST,
            "unsupported-operation",
            MessageLoader::getMessage(parms),
            closeConnect);
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
        sendHttpError(
            queueId,
            HTTP_STATUS_NOTIMPLEMENTED,
            "multiple-requests-unsupported",
            String::EMPTY,
            closeConnect);
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

    // Validate if Pragma: UpdateExpiredPassword is set
    updateExpiredPassword = false;

    String pragmaValue;
    if(HTTPMessage::lookupHeader(headers,"Pragma",pragmaValue, true))
    {
        updateExpiredPassword =
            (PEG_NOT_FOUND != pragmaValue.find("UpdateExpiredPassword"));
    }

    String cimMethod;
    if (HTTPMessage::lookupHeader(headers, "CIMMethod", cimMethod, true))
    {
        if (cimMethod.size() == 0)
        {
            // This is not a valid value, and we use EMPTY to mean "absent"
            MessageLoaderParms parms(
                "Server.CIMOperationRequestDecoder.EMPTY_CIMMETHOD_VALUE",
                "Empty CIMMethod value.");
            sendHttpError(
                queueId,
                HTTP_STATUS_BADREQUEST,
                "header-mismatch",
                MessageLoader::getMessage(parms),
                closeConnect);
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
                "Server.CIMOperationRequestDecoder."
                    "CIMMETHOD_VALUE_SYNTAX_ERROR",
                "CIMMethod value syntax error.");
            sendHttpError(
                queueId,
                HTTP_STATUS_BADREQUEST,
                "header-mismatch",
                MessageLoader::getMessage(parms),
                closeConnect);
            PEG_METHOD_EXIT();
            return;
        }
    }

    String cimObject;
    if (HTTPMessage::lookupHeader(headers, "CIMObject", cimObject, true))
    {
        if (cimObject.size() == 0)
        {
            // This is not a valid value, and we use EMPTY to mean "absent"
            MessageLoaderParms parms(
                "Server.CIMOperationRequestDecoder.EMPTY_CIMOBJECT_VALUE",
                "Empty CIMObject value.");
            sendHttpError(
                queueId,
                HTTP_STATUS_BADREQUEST,
                "header-mismatch",
                MessageLoader::getMessage(parms),
                closeConnect);
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
            MessageLoaderParms parms(
                "Server.CIMOperationRequestDecoder."
                    "CIMOBJECT_VALUE_SYNTAX_ERROR",
                "CIMObject value syntax error.");
            sendHttpError(
                queueId,
                HTTP_STATUS_BADREQUEST,
                "header-mismatch",
                MessageLoader::getMessage(parms),
                closeConnect);
            PEG_METHOD_EXIT();
            return;
        }
    }

    // Validate the "Content-Type" header:

    const char* cimContentType;
    Boolean contentTypeHeaderFound = HTTPMessage::lookupHeader(
        headers, "Content-Type", cimContentType, true);
    String type;
    String charset;
    Boolean binaryRequest = false;

    if (!contentTypeHeaderFound ||
        !HTTPMessage::parseContentTypeHeader(cimContentType, type, charset) ||
        (((!String::equalNoCase(type, "application/xml") &&
         !String::equalNoCase(type, "text/xml")) ||
        !String::equalNoCase(charset, "utf-8"))
        && !(binaryRequest = String::equalNoCase(type,
            "application/x-openpegasus"))
        ))
    {
        MessageLoaderParms parms(
            "Server.CIMOperationRequestDecoder.CIMCONTENTTYPE_SYNTAX_ERROR",
            "HTTP Content-Type header error.");
        sendHttpError(
            queueId,
            HTTP_STATUS_BADREQUEST,
            "",
            MessageLoader::getMessage(parms),
            closeConnect);
        PEG_METHOD_EXIT();
        return;
    }
    // Calculate the beginning of the content from the message size and
    // the content length.
    if (binaryRequest)
    {
        // binary the "Content" also contains a few padding '\0' to align
        // data structures to 8byte boundary
        // the padding '\0' are also part of the counted contentLength
        Uint32 headerEnd = httpMessage->message.size() - contentLength;
        Uint32 binContentStart = CIMBuffer::round(headerEnd);

        contentLength = contentLength - (binContentStart - headerEnd);
        content = (char*) httpMessage->message.getData() + binContentStart;
    }
    else
    {
        content = (char*) httpMessage->message.getData() +
            httpMessage->message.size() - contentLength;
    }

    // Validating content falls within UTF8
    // (required to be complaint with section C12 of Unicode 4.0 spec,
    // chapter 3.)
    if (!binaryRequest)
    {
        Uint32 count = 0;
        while(count<contentLength)
        {
            if (!(isUTF8((char*)&content[count])))
            {
                MessageLoaderParms parms(
                    "Server.CIMOperationRequestDecoder.INVALID_UTF8_CHARACTER",
                    "Invalid UTF-8 character detected.");
                sendHttpError(
                    queueId,
                    HTTP_STATUS_BADREQUEST,
                    "request-not-valid",
                    MessageLoader::getMessage(parms),
                    closeConnect);

                PEG_METHOD_EXIT();
                return;
            }
            UTF8_NEXT(content,count);
        }
    }

    // Check for "Accept: application/x-openpegasus" HTTP header to see if
    // client can accept binary responses.

    bool binaryResponse;

    if (HTTPMessage::lookupHeader(headers, "Accept", type, true) &&
        String::equalNoCase(type, "application/x-openpegasus"))
    {
        binaryResponse = true;
    }
    else
    {
        binaryResponse = false;
    }
    httpMessage->binaryResponse=binaryResponse;

    // If it is a method call, then dispatch it to be handled:

    handleMethodCall(
        queueId,
        httpMethod,
        content,
        contentLength,
        cimProtocolVersion,
        cimMethod,
        cimObject,
        authType,
        userName,
        userRole,
        userPass,
        isExpiredPassword,
        updateExpiredPassword,
        httpMessage->ipAddress,
        httpMessage->acceptLanguages,
        httpMessage->contentLanguages,
        closeConnect,
        binaryRequest,
        binaryResponse);

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
    Boolean binaryResponse)
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
        sendHttpError(
            queueId,
            HTTP_STATUS_SERVICEUNAVAILABLE,
            String::EMPTY,
            MessageLoader::getMessage(parms),
            closeConnect);
        PEG_METHOD_EXIT();
        return;
    }

    PEG_TRACE((TRC_XML,Tracer::LEVEL4,
        "CIMOperationRequestdecoder - XML content: %s",
        content));

    //
    // Handle binary messages:
    //

    AutoPtr<CIMOperationRequestMessage> request;
    String messageId;
    Boolean isIMethodCall = true;

    if (binaryRequest)
    {
        CIMBuffer buf(content, contentLength);
        CIMBufferReleaser buf_(buf);

        request.reset(BinaryCodec::decodeRequest(buf, queueId, _returnQueueId));

        if (!request.get())
        {
            sendHttpError(
                queueId,
                HTTP_STATUS_BADREQUEST,
                "Corrupt binary request message",
                String::EMPTY,
                closeConnect);
            PEG_METHOD_EXIT();
            return;
        }
    }
    else try
    {
        XmlParser parser(content);
        XmlEntry entry;
        const char* cimMethodName = "";

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
            MessageLoaderParms parms(
                "Server.CIMOperationRequestDecoder.CIM_VERSION_NOT_SUPPORTED",
                "CIM version \"$0\" is not supported.",
                 cimVersion);
            sendHttpError(
                queueId,
                HTTP_STATUS_NOTIMPLEMENTED,
                "unsupported-cim-version",
                MessageLoader::getMessage(parms),
                closeConnect);
            PEG_METHOD_EXIT();
            return;
        }

        if (!XmlReader::isSupportedDTDVersion(dtdVersion))
        {
            MessageLoaderParms parms(
                "Server.CIMOperationRequestDecoder.DTD_VERSION_NOT_SUPPORTED",
                "DTD version \"$0\" is not supported.",
                dtdVersion);
            sendHttpError(
                queueId,
                HTTP_STATUS_NOTIMPLEMENTED,
                "unsupported-dtd-version",
                MessageLoader::getMessage(parms),
                closeConnect);
            PEG_METHOD_EXIT();
            return;
        }

        // Expect <MESSAGE ...>

        String protocolVersion;

        if (!XmlReader::getMessageStartTag(
                 parser, messageId, protocolVersion))
        {
            MessageLoaderParms mlParms(
                "Server.CIMOperationRequestDecoder.EXPECTED_MESSAGE_ELEMENT",
                "expected MESSAGE element");

            throw XmlValidationError(parser.getLine(), mlParms);
        }

        // Validate that the protocol version in the header matches the XML
        if (!String::equalNoCase(protocolVersion, cimProtocolVersionInHeader))
        {
            MessageLoaderParms parms(
                "Server.CIMOperationRequestDecoder."
                    "CIMPROTOCOL_VERSION_MISMATCH",
                "CIMProtocolVersion value \"$0\" does not match CIM request "
                    "protocol version \"$1\".",
                cimProtocolVersionInHeader,
                protocolVersion);
            sendHttpError(
                queueId,
                HTTP_STATUS_BADREQUEST,
                "header-mismatch",
                MessageLoader::getMessage(parms),
                closeConnect);
            PEG_METHOD_EXIT();
            return;
        }

        // Accept protocol version 1.x (see Bugzilla 1556)
        if (!XmlReader::isSupportedProtocolVersion(protocolVersion))
        {
            // See Specification for CIM Operations over HTTP section 4.3
            MessageLoaderParms parms(
                "Server.CIMOperationRequestDecoder."
                    "CIMPROTOCOL_VERSION_NOT_SUPPORTED",
                "CIMProtocolVersion \"$0\" is not supported.",
                     protocolVersion);
            sendHttpError(
                queueId,
                HTTP_STATUS_NOTIMPLEMENTED,
                "unsupported-protocol-version",
                MessageLoader::getMessage(parms),
                closeConnect);
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
            sendHttpError(
                queueId,
                HTTP_STATUS_BADREQUEST,
                "header-mismatch",
                MessageLoader::getMessage(parms),
                closeConnect);
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
            isIMethodCall = true;
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
            //     - The CIMMethod header is present but has an invalid value,
            //       or;
            //     - The CIMMethod header is not present but the Operation
            //       Request Message is a Simple Operation Request, or;
            //     - The CIMMethod header is present but the Operation Request
            //       Message is not a Simple Operation Request, or;
            //     - The CIMMethod header is present, the Operation Request
            //       Message is a Simple Operation Request, but the
            //       CIMIdentifier value (when unencoded) does not match the
            //       unique method name within the Simple Operation Request,
            //
            //     then it MUST fail the request and return a status of
            //     "400 Bad Request" (and MUST include a CIMError header in the
            //     response with a value of header-mismatch), subject to the
            //     considerations specified in Errors.
            if (!String::equalNoCase(cimMethodName, cimMethodInHeader))
            {
                // ATTN-RK-P3-20020304: How to decode cimMethodInHeader?
                if (cimMethodInHeader.size() == 0)
                {
                    MessageLoaderParms parms(
                        "Server.CIMOperationRequestDecoder."
                            "MISSING_CIMMETHOD_HTTP_HEADER",
                        "Missing CIMMethod HTTP header.");
                    sendHttpError(
                        queueId,
                        HTTP_STATUS_BADREQUEST,
                        "header-mismatch",
                        MessageLoader::getMessage(parms),
                        closeConnect);
                }
                else
                {
                    MessageLoaderParms parms(
                        "Server.CIMOperationRequestDecoder."
                             "CIMMETHOD_VALUE_DOES_NOT_MATCH_REQUEST_METHOD",
                        "CIMMethod value \"$0\" does not match CIM request "
                             "method \"$1\".",
                        cimMethodInHeader,
                        cimMethodName);
                    sendHttpError(
                        queueId,
                        HTTP_STATUS_BADREQUEST,
                        "header-mismatch",
                        MessageLoader::getMessage(parms),
                        closeConnect);
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
                throw XmlValidationError(parser.getLine(), mlParms);
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
            //     - The CIMObject header is present but has an invalid value,
            //       or;
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
                if (cimObjectInHeader.size() == 0)
                {
                    MessageLoaderParms parms(
                        "Server.CIMOperationRequestDecoder."
                            "MISSING_CIMOBJECT_HTTP_HEADER",
                        "Missing CIMObject HTTP header.");
                    sendHttpError(
                        queueId,
                        HTTP_STATUS_BADREQUEST,
                        "header-mismatch",
                        MessageLoader::getMessage(parms),
                        closeConnect);
                }
                else
                {
                    MessageLoaderParms parms(
                        "Server.CIMOperationRequestDecoder."
                            "CIMOBJECT_VALUE_DOES_NOT_MATCH_REQUEST_OBJECT",
                        "CIMObject value \"$0\" does not match CIM request "
                            "object \"$1\".",
                        cimObjectInHeader,
                        nameSpace);
                    sendHttpError(
                        queueId,
                        HTTP_STATUS_BADREQUEST,
                        "header-mismatch",
                        MessageLoader::getMessage(parms),
                        closeConnect);
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
                        queueId, parser, messageId, nameSpace));
                else if (System::strcasecmp(cimMethodName, "GetInstance") == 0)
                    request.reset(decodeGetInstanceRequest(
                        queueId, parser, messageId, nameSpace));
                else if (System::strcasecmp(
                             cimMethodName, "EnumerateClassNames") == 0)
                    request.reset(decodeEnumerateClassNamesRequest(
                        queueId, parser, messageId, nameSpace));
                else if (System::strcasecmp(cimMethodName, "References") == 0)
                    request.reset(decodeReferencesRequest(
                        queueId, parser, messageId, nameSpace));
                else if (System::strcasecmp(
                             cimMethodName, "ReferenceNames") == 0)
                    request.reset(decodeReferenceNamesRequest(
                        queueId, parser, messageId, nameSpace));
                else if (System::strcasecmp(
                             cimMethodName, "AssociatorNames") == 0)
                    request.reset(decodeAssociatorNamesRequest(
                        queueId, parser, messageId, nameSpace));
                else if (System::strcasecmp(cimMethodName, "Associators") == 0)
                    request.reset(decodeAssociatorsRequest(
                        queueId, parser, messageId, nameSpace));
                else if (System::strcasecmp(
                             cimMethodName, "CreateInstance") == 0)
                    request.reset(decodeCreateInstanceRequest(
                        queueId, parser, messageId, nameSpace));
                else if (System::strcasecmp(
                             cimMethodName, "EnumerateInstanceNames")==0)
                    request.reset(decodeEnumerateInstanceNamesRequest(
                        queueId, parser, messageId, nameSpace));
                else if (System::strcasecmp(
                             cimMethodName, "DeleteQualifier") == 0)
                    request.reset(decodeDeleteQualifierRequest(
                        queueId, parser, messageId, nameSpace));
                else if (System::strcasecmp(cimMethodName, "GetQualifier") == 0)
                    request.reset(decodeGetQualifierRequest(
                        queueId, parser, messageId, nameSpace));
                else if (System::strcasecmp(cimMethodName, "SetQualifier") == 0)
                    request.reset(decodeSetQualifierRequest(
                        queueId, parser, messageId, nameSpace));
                else if (System::strcasecmp(
                             cimMethodName, "EnumerateQualifiers") == 0)
                    request.reset(decodeEnumerateQualifiersRequest(
                        queueId, parser, messageId, nameSpace));
                else if (System::strcasecmp(
                             cimMethodName, "EnumerateClasses") == 0)
                    request.reset(decodeEnumerateClassesRequest(
                        queueId, parser, messageId, nameSpace));
                else if (System::strcasecmp(
                             cimMethodName, "EnumerateInstances") == 0)
                    request.reset(decodeEnumerateInstancesRequest(
                        queueId, parser, messageId, nameSpace));
                else if (System::strcasecmp(cimMethodName, "CreateClass") == 0)
                    request.reset(decodeCreateClassRequest(
                        queueId, parser, messageId, nameSpace));
                else if (System::strcasecmp(cimMethodName, "ModifyClass") == 0)
                    request.reset(decodeModifyClassRequest(
                        queueId, parser, messageId, nameSpace));
                else if (System::strcasecmp(
                             cimMethodName, "ModifyInstance") == 0)
                    request.reset(decodeModifyInstanceRequest(
                        queueId, parser, messageId, nameSpace));
                else if (System::strcasecmp(cimMethodName, "DeleteClass") == 0)
                    request.reset(decodeDeleteClassRequest(
                        queueId, parser, messageId, nameSpace));
                else if (System::strcasecmp(
                             cimMethodName, "DeleteInstance") == 0)
                    request.reset(decodeDeleteInstanceRequest(
                        queueId, parser, messageId, nameSpace));
                else if (System::strcasecmp(cimMethodName, "GetProperty") == 0)
                    request.reset(decodeGetPropertyRequest(
                        queueId, parser, messageId, nameSpace));
                else if (System::strcasecmp(cimMethodName, "SetProperty") == 0)
                    request.reset(decodeSetPropertyRequest(
                        queueId, parser, messageId, nameSpace));
                else if (System::strcasecmp(cimMethodName, "ExecQuery") == 0)
                    request.reset(decodeExecQueryRequest(
                        queueId, parser, messageId, nameSpace));
                // EXP_PULL_BEGIN
                else if (System::strcasecmp(
                             cimMethodName, "OpenEnumerateInstances") == 0)
                    request.reset(decodeOpenEnumerateInstancesRequest(
                        queueId, parser, messageId, nameSpace));
                else if (System::strcasecmp(
                             cimMethodName, "OpenEnumerateInstancePaths") == 0)
                    request.reset(decodeOpenEnumerateInstancePathsRequest(
                        queueId, parser, messageId, nameSpace));
                else if (System::strcasecmp(
                             cimMethodName, "OpenReferenceInstances") == 0)
                    request.reset(decodeOpenReferenceInstancesRequest(
                        queueId, parser, messageId, nameSpace));
                else if (System::strcasecmp(
                             cimMethodName, "OpenReferenceInstancePaths") == 0)
                    request.reset(decodeOpenReferenceInstancePathsRequest(
                        queueId, parser, messageId, nameSpace));
                else if (System::strcasecmp(
                             cimMethodName, "OpenAssociatorInstances") == 0)
                    request.reset(decodeOpenAssociatorInstancesRequest(
                        queueId, parser, messageId, nameSpace));
                else if (System::strcasecmp(
                             cimMethodName, "OpenAssociatorInstancePaths") == 0)
                    request.reset(decodeOpenAssociatorInstancePathsRequest(
                        queueId, parser, messageId, nameSpace));
                else if (System::strcasecmp(
                             cimMethodName, "PullInstancesWithPath") == 0)
                    request.reset(decodePullInstancesWithPathRequest(
                        queueId, parser, messageId, nameSpace));
                else if (System::strcasecmp(
                             cimMethodName, "PullInstancePaths") == 0)
                    request.reset(decodePullInstancePathsRequest(
                        queueId, parser, messageId, nameSpace));
                else if (System::strcasecmp(
                             cimMethodName, "PullInstances") == 0)
                    request.reset(decodePullInstancesRequest(
                        queueId, parser, messageId, nameSpace));
                else if (System::strcasecmp(
                             cimMethodName, "CloseEnumeration") == 0)
                    request.reset(decodeCloseEnumerationRequest(
                        queueId, parser, messageId, nameSpace));
                else if (System::strcasecmp(
                             cimMethodName, "EnumerationCount") == 0)
                    request.reset(decodeEnumerationCountRequest(
                        queueId, parser, messageId, nameSpace));
                else if (System::strcasecmp(
                             cimMethodName, "OpenQueryInstances") == 0)
                    request.reset(decodeOpenQueryInstancesRequest(
                        queueId, parser, messageId, nameSpace));
                // EXP_PULL_END
                else
                {
                    _throwCIMExceptionCIMErrNotSupported(MessageLoaderParms(
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
                    e,
                    closeConnect);

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
                // have had a problem reconstructing a CIM object, we'll treat
                // it as an invalid parameter
                sendIMethodError(
                    queueId,
                    httpMethod,
                    messageId,
                    cimMethodName,
                    PEGASUS_CIM_EXCEPTION(
                        CIM_ERR_INVALID_PARAMETER, e.getMessage()),
                    closeConnect);

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
            isIMethodCall = false;

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
            //     - The CIMMethod header is present but has an invalid value,
            //       or;
            //     - The CIMMethod header is not present but the Operation
            //       Request Message is a Simple Operation Request, or;
            //     - The CIMMethod header is present but the Operation Request
            //       Message is not a Simple Operation Request, or;
            //     - The CIMMethod header is present, the Operation Request
            //       Message is a Simple Operation Request, but the
            //       CIMIdentifier value (when unencoded) does not match the
            //       unique method name within the Simple Operation Request,
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
                if (cimMethodInHeader.size() == 0)
                {
                    MessageLoaderParms parms(
                        "Server.CIMOperationRequestDecoder."
                            "MISSING_CIMMETHOD_HTTP_HEADER",
                        "Missing CIMMethod HTTP header.");
                    sendHttpError(
                        queueId,
                        HTTP_STATUS_BADREQUEST,
                        "header-mismatch",
                        MessageLoader::getMessage(parms),
                        closeConnect);
                }
                else
                {
                    MessageLoaderParms parms(
                        "Server.CIMOperationRequestDecoder."
                            "CIMMETHOD_VALUE_DOES_NOT_MATCH_REQUEST_METHOD",
                        "CIMMethod value \"$0\" does not match CIM request "
                            "method \"$1\".",
                        (const char*)cimMethodInHeader.getCString(),
                        cimMethodName);
                    sendHttpError(
                        queueId,
                        HTTP_STATUS_BADREQUEST,
                        "header-mismatch",
                        MessageLoader::getMessage(parms),
                        closeConnect);
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
                MessageLoaderParms parms(
                    "Common.XmlConstants.MISSING_ELEMENT_LOCALPATH",
                    MISSING_ELEMENT_LOCALPATH);
                // this throw is not updated with MLP because
                // MISSING_ELEMENT_LOCALPATH is a hardcoded variable,
                // not a message
                throw XmlValidationError(parser.getLine(), parms);
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
            //     - The CIMObject header is present but has an invalid value,
            //       or;
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
            if (cimObjectInHeader.size() == 0)
            {
                MessageLoaderParms parms(
                    "Server.CIMOperationRequestDecoder."
                        "MISSING_CIMOBJECT_HTTP_HEADER",
                    "Missing CIMObject HTTP header.");
                sendHttpError(
                    queueId,
                    HTTP_STATUS_BADREQUEST,
                    "header-mismatch",
                    MessageLoader::getMessage(parms),
                    closeConnect);
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
                sendHttpError(
                    queueId,
                    HTTP_STATUS_BADREQUEST,
                    "header-mismatch",
                    MessageLoader::getMessage(parms),
                    closeConnect);
                PEG_METHOD_EXIT();
                return;
            }

            if (!reference.identical(headerObjectReference))
            {
                MessageLoaderParms parms(
                    "Server.CIMOperationRequestDecoder."
                        "CIMOBJECT_VALUE_DOES_NOT_MATCH_REQUEST_OBJECT",
                    "CIMObject value \"$0\" does not match CIM request "
                        "object \"$1\".",
                    cimObjectInHeader,
                    reference.toString());
                sendHttpError(
                    queueId,
                    HTTP_STATUS_BADREQUEST,
                    "header-mismatch",
                    MessageLoader::getMessage(parms),
                    closeConnect);
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
                   cimMethodNameUTF16)); // contains UTF-16 converted from UTF-8
            }
            catch (CIMException& e)
            {
                sendMethodError(
                    queueId,
                    httpMethod,
                    messageId,
                    cimMethodNameUTF16, // contains UTF-16 converted from UTF-8
                    e,
                    closeConnect);

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
                // have had a problem reconstructing a CIM object, we'll treata
                // it as an invalid parameter
                sendMethodError(
                    queueId,
                    httpMethod,
                    messageId,
                    cimMethodNameUTF16, // contains UTF-16 converted from UTF-8
                    PEGASUS_CIM_EXCEPTION(
                        CIM_ERR_INVALID_PARAMETER, e.getMessage()),
                    closeConnect);

                PEG_METHOD_EXIT();
                return;
            }

            // Expect </METHODCALL>

            XmlReader::expectEndTag(parser, "METHODCALL");
        }
        else
        {
            MessageLoaderParms mlParms(
               "Server.CIMOperationRequestDecoder.EXPECTED_IMETHODCALL_ELEMENT",
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

        sendHttpError(
            queueId,
            HTTP_STATUS_BADREQUEST,
            "request-not-valid",
            e.getMessage(),
            closeConnect);
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
        sendHttpError(
            queueId,
            HTTP_STATUS_BADREQUEST,
            "request-not-valid",
            e.getMessage(),
            closeConnect);
        PEG_METHOD_EXIT();
        return;
    }
    catch (XmlException& e)
    {
        PEG_TRACE((TRC_XML,Tracer::LEVEL1,
            "CIMOperationRequestDecoder::handleMethodCall - "
                "XmlException has occurred. Message: %s",
            (const char*) e.getMessage().getCString()));

        sendHttpError(
            queueId,
            HTTP_STATUS_BADREQUEST,
            "request-not-well-formed",
            e.getMessage(),
            closeConnect);
        PEG_METHOD_EXIT();
        return;
    }
    catch (TooManyElementsException& e)
    {
        PEG_TRACE((TRC_XML,Tracer::LEVEL1,
            "CIMOperationRequestDecoder::handleMethodCall - "
                "TooManyElementsException has occurred. Message: %s",
            (const char*) e.getMessage().getCString()));

        sendHttpError(
            queueId,
            HTTP_STATUS_BADREQUEST,
            "request-with-too-many-elements",
            e.getMessage(),
            closeConnect);
        PEG_METHOD_EXIT();
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
        PEG_METHOD_EXIT();
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
        PEG_METHOD_EXIT();
        return;
    }

    STAT_BYTESREAD

    request->authType = authType;
    request->userName = userName;
    request->ipAddress = ipAddress;
    request->setHttpMethod (httpMethod);
    request->binaryResponse = binaryResponse;

//l10n start
// l10n TODO - might want to move A-L and C-L to Message
// to make this more maintainable
    // Add the language headers to the request
    CIMMessage* cimmsg = dynamic_cast<CIMMessage*>(request.get());
    if (cimmsg != NULL)
    {
        cimmsg->operationContext.insert(IdentityContainer(userName));
        cimmsg->operationContext.insert(UserRoleContainer(userRole));
        cimmsg->operationContext.set(
            AcceptLanguageListContainer(httpAcceptLanguages));
        cimmsg->operationContext.set(
            ContentLanguageListContainer(httpContentLanguages));
    }
    else
    {
        ;    // l10n TODO - error back to client here
    }
// l10n end

#ifdef PEGASUS_PAM_SESSION_SECURITY

    // Whatever happens on the authentication, we need to check for
    // a change of an expired password
    // Since the definition for password updates is not completely
    // defined in DMTF yet, keep this feature PAM_SESSION only
    // This also only works with CIM-XML right now.
    if (isExpiredPassword)
    {
        // only try password update if req. Pragma is set
        if (updateExpiredPassword)
        {
            // update expired password
            // fct. _updateExpiredPassword returns false
            //        if the request was NOT for PG_Account etc.
            _updateExpiredPassword(
                queueId,
                httpMethod,
                messageId,
                closeConnect,
                httpContentLanguages,
                cimmsg,
                userName,
                userPass,
                ipAddress);
        }
        else
        {
            sendUserAccountExpired(
                queueId,
                httpMethod,
                messageId,
                cimMethodInHeader,
                closeConnect,
                isIMethodCall);
        }
        PEG_METHOD_EXIT();
        return;
    }
#endif

    request->setCloseConnect(closeConnect);
    _outputQueue->enqueue(request.release());

    PEG_METHOD_EXIT();
}
/**************************************************************************
**
**  Decode CIM Operation Type Common IParameter types.
**  Each class defines processing for a particular IPARAM type
**  (ex. boolean)or parameter name (i.e. propertyListIParam)
**  Each struct defines:
**      got - Boolean defines whether this parameter has been found
**      value - The value container for this type
**      found(...) - function sets the duplicate flag and the got flag
**      get - Function to get the defined parameter from the
**            parser
**      NOTE: at least one has multiple get.. functions.
** NOTE: Some of these are defined for a particular attribute (ex.
**      propertyListIparam) so the attribute name is integrated into the
**      methods and others for a whole class of attributes (Boolean,
**      String,ClassName etc.) so the attribute name is defined as part
**      of the constructor.
***************************************************************************/

/* base class for common elements of all of the IParam classes
*/
class baseIParam
{
public:

    // Constructor with defaulted Name. Name set by subclass
    baseIParam():
        got(false) {}

    // Constructor with Name.
    baseIParam(const char* name):
        got(false),
        iParamName(name) {}

    // Set the flag to indicate that this IParam has been gotten and also
    // set the flag defined by the duplicate parameter
    // @param duplicate Boolean that is set to previous value of the got
    // variable indicating whether this is second call to this IParam
    void found(Boolean& duplicate)
    {
        duplicate = got;
        got = true;
    }

    String& name()
    {
        return iParamName;
    }

    // Tests if the parameter exists in the request
    // and if it does not, issues an exception
    void rejectIfNotExist()
    {
        if (!got)
        {
            _throwCIMExceptionRequiredDoesNotExist(iParamName);
        }
    }

    // Initial false.  Set true by found function.
    Boolean got;

protected:
    String iParamName;

private:
        // hide unused default constructor and assign, copy constructors
    baseIParam(const baseIParam&);
    baseIParam& operator = (const baseIParam&);
};

// Common xml attribute accessor for all boolean attributes.   The
// attribute name is defined in the constructor.
// The usage pattern is:
//    Boolean duplicate;     // Flag to indicate multiple calls
//
//    booleanIParam xyz("xyz"); default is false for attribute xyz
//
//    if(xyz.get(parser, name, emptyTag)   // parses to test if name == xyz
//        found(duplicate);          // set flag to indicate exists etc.
class booleanIParam : public baseIParam
{
public:
    Boolean value;

    // constructor with default value = false
    booleanIParam(const char* name): baseIParam(name), value(false) {}

    // constructor with initial value specifically set from the input

    booleanIParam(const char* name, Boolean _value): baseIParam(name),
        value(_value)
    {
    }

    // get the value of the parameter if the parameter if it exists.
    // Note that the attribute name is defined in the constructor
    // Value is required.
    // @param parser
    // @param testName attribute name from parse.
    // @emptyTag returns true if emptyTag returned true from parser
    Boolean get(XmlParser& parser, const char * testName,  Boolean& emptyTag)
    {
        if (System::strcasecmp(iParamName.getCString(), testName) == 0)
        {
            XmlReader::rejectNullIParamValue(parser, emptyTag,
                iParamName.getCString());
            XmlReader::getBooleanValueElement(parser, value, true);
            return true;
        }
        return false;
    }

private:
    // hide unused default constructor and assign, copy constructors
    booleanIParam();
    booleanIParam(const booleanIParam&);
    booleanIParam& operator = (const booleanIParam&);
};

// decode Iparam to CIMName representing class names.  This struct
// has two get functions:
//     get - parse where the parameter value is required
//     getOptional - parse where the parameter value is optional

class classNameIParam : public baseIParam
{
public:
    CIMName value;

    // construct an IParam definition with name.
    // @param name const char* defining name of IParam to match
    // @return true if IParam found with _attrName

    classNameIParam(const char* name): baseIParam(name), value(CIMName()) {}

    // Get Required value element.Test for name parameter as IParam with
    // name and if found, if value not NULL, parse the className and
    // set into value
    Boolean get(XmlParser& parser, const char* name, Boolean& emptyTag)
    {
        if (System::strcasecmp(name,iParamName.getCString()) == 0)
        {
            XmlReader::rejectNullIParamValue(parser, emptyTag, name);
            XmlReader::getClassNameElement(parser, value, true);
            return true;
        }
        return false;
    }
    // Get Iparam with optional value.
    Boolean getOptional(XmlParser& parser, const char* name,
                        Boolean& emptyTag)
    {
        if (System::strcasecmp(name, iParamName.getCString()) == 0)
        {
            //  value may be NULL
            if (!emptyTag)
            {
                XmlReader::getClassNameElement(parser, value, false);
            }
            return true;
        }
        return false;
    }
private:
    // hide unused default constructor and assign, copy constructors
    classNameIParam();
    classNameIParam(const classNameIParam&);
    classNameIParam& operator = (const classNameIParam&);
};

// test for "InstanceName" iParam and if found, return CIMObjectPath
// in value
class instanceNameIParam : public baseIParam
{
public:
    CIMObjectPath value;

    instanceNameIParam(const char* name): baseIParam(name),
        value(CIMObjectPath())
    {
    }

    Boolean get(XmlParser& parser, const char * name, Boolean& emptyTag)
    {
        if (System::strcasecmp(name, iParamName.getCString()) == 0)
        {
            XmlReader::rejectNullIParamValue(parser, emptyTag, name);
            XmlReader::getInstanceNameElement(parser, value);
            return true;
        }
        return false;
    }
private:
    // hide unused assign, copy constructors
    instanceNameIParam();
    instanceNameIParam(const instanceNameIParam&);
    instanceNameIParam& operator = (const instanceNameIParam&);
};

// test for "ObjectName" attribute and if found, return CIMObjectPath
// This struct has an extra attribute, the flag isClassNameElement which
// returns true if the objectName was a classPath and not an instance
// path.
// If Xmlreader returns true, this is class only element, no
//  key bindings. That state must be set into the request
//  message (ex. objectName.isClassElement)
// @param (Optional) Name of IParam.  Default is ObjectName.  Note
// that pull operations use InstanceName as IParamName.
class objectNameIParam: public baseIParam
{
public:
    CIMObjectPath value;
    bool isClassNameElement;

    // Constructor with default parameter name = "ObjectName"
    objectNameIParam(): baseIParam("ObjectName"),
        value(CIMObjectPath()), isClassNameElement(false)
    {
    }

    objectNameIParam(const char* name): baseIParam(name),
        value(CIMObjectPath()), isClassNameElement(false)
    {
    }

    Boolean get(XmlParser& parser, const char * name, Boolean& emptyTag)
    {
        if (System::strcasecmp(name, iParamName.getCString()) == 0)
        {
            XmlReader::rejectNullIParamValue(parser, emptyTag, name);
            isClassNameElement =
                XmlReader::getObjectNameElement(parser, value);
            return true;
        }
        return false;
    }
private:
    // hide unused assign, copy constructors
    objectNameIParam(const objectNameIParam&);
    objectNameIParam& operator = (const objectNameIParam&);
};

// test for "PropertyList" attribute and, if found, return property list
// in the value element.
class propertyListIParam : public baseIParam
{
public:
    CIMPropertyList value;

    // construct a propertyListIParam object
    propertyListIParam(): baseIParam(){}

    ~propertyListIParam(){}

    Boolean get(XmlParser& parser, const char* name, Boolean& emptyTag)
    {
        if (System::strcasecmp(name, "PropertyList") == 0)
        {
            if (!emptyTag)
            {
                CIMValue pl;
                if (XmlReader::getValueArrayElement(parser, CIMTYPE_STRING, pl))
                {
                    Array<String> propertyListArray;
                    pl.get(propertyListArray);
                    // NOTE: Cannot use the propertyList.set(...) method
                    // here because set does not create propertyList tags
                    value.append(propertyListArray);
                }
            }
            return true;
        }
        return false;
    }

    // This version of the get function uses the propertyList set function
    // to set the property list array into the propertyList object.  It
    // can only be used for those Operations where the propertylist is NOT
    // used by the Server in the response (i.e. getClass and modifyInstance).
    //
    Boolean getSpecial(XmlParser& parser, const char* name, Boolean& emptyTag)
    {
        if (System::strcasecmp(name, "PropertyList") == 0)
        {
            if (!emptyTag)
            {
                CIMValue pl;
                if (XmlReader::getValueArrayElement(parser, CIMTYPE_STRING, pl))
                {
                    Array<String> propertyListArray;
                    pl.get(propertyListArray);
                    Array<CIMName> cimNameArray;
                    // Map the strings to CIMNames.
                    for (Uint32 i = 0; i < propertyListArray.size(); i++)
                    {
                        cimNameArray.append(propertyListArray[i]);
                    }
                    // use set to put list into property list without
                    // setting propertyList tags.
                    value.set(cimNameArray);
                }
            }
            return true;
        }
        return false;
    }
private:
    // hide unused default assign, copy constructors
    propertyListIParam(const propertyListIParam&);
    propertyListIParam& operator = (const propertyListIParam&);
};

// Attribute decoder for String Parameters
// The constructor MUST include the attribute name.
// The second defines whether a value is required.
// If true and there is no value, the XmlReader does an exception.

class stringIParam : public baseIParam
{
public:
    String value;

    // constructor with definition of attribute and  required flag.
    // @param name const char* with name of IParam to match
    // @param valueRequired Boolean that defines whether value is required

    stringIParam(const char* name, Boolean valueRequired): baseIParam(name),
        _valueRequired(valueRequired) {}

    ~stringIParam(){}

    // get the attribute if it exists. The attribute name is defined in
    // the constructor
    // @param parser
    // @param testName attribute name from parse.
    // @emptyTag returns true if emptyTag returned true from parser
    // @return Returns true if testName matches the IParam defined by current
    // position in the parser
    Boolean get(XmlParser& parser, const char * testName,  Boolean& emptyTag)
    {
        if (System::strcasecmp(iParamName.getCString(), testName) == 0)
        {
            if (!emptyTag)
            {
                XmlReader::getStringValueElement(parser, value, _valueRequired);
            }
            return true;
        }
        return false;
    }

private:
    Boolean _valueRequired;
    stringIParam();
    stringIParam(const stringIParam&);
    stringIParam& operator = (const stringIParam&);
};

//EXP_PULL_BEGIN
// Attribute decoder for Uint32Arg Parameters
// The constructor MUST include the attribute name.
// The second defines whether a value is required.
// If true and there is no value, the XmlReader does an exception.
//
class uint32ArgIParam : public baseIParam
{
public:
    // Initally set to NULL. (Server sets timeout time)
    Uint32Arg value;

    // constructor with definition of iParam name and default for the
    // required flag (false). Default value of parameter is NULL if
    // no value is supplied.
    // @param name const char* with name of IParam to match
    // The default for Uint32Args in general is NULL. If you want
    // anything else, set it specifically

    uint32ArgIParam(const char* name): baseIParam(name),
        _valueRequired(false) {}

    // constructor with definition of iParam name and default for the
    // required flag (false). Default value of parameter is integer defined
    // by supplied value.
    // @param name const char* with name of IParam to match
    // @param valueArg Uint32 value to to which the uint32ArgIParam is
    // set.

    uint32ArgIParam(const char* name, Uint32 valueArg): baseIParam(name),
        value(valueArg), _valueRequired(false) {}

    ~uint32ArgIParam(){}

    // get the attribute if it exists. The attribute name is defined in
    // the constructor
    // @param parser
    // @param testName attribute name from parse.
    // @emptyTag returns true if emptyTag returned true from parser
    // @return Returns true if testName matches the IParam defined by current
    // position in the parser
    Boolean get(XmlParser& parser, const char * testName,  Boolean& emptyTag)
    {
        if (_valueRequired)
        {
            XmlReader::rejectNullIParamValue(parser, emptyTag, testName);
        }
        if (System::strcasecmp(iParamName.getCString(), testName) == 0)
        {
            XmlReader::getUint32ArgValueElement(parser, value, true);
            return true;
        }
        return false;
    }
private:
    Boolean _valueRequired;
    uint32ArgIParam();
    uint32ArgIParam(const uint32ArgIParam&);
    uint32ArgIParam& operator = (const uint32ArgIParam&);
};

// Attribute decoder for Uint32 Parameters
// The constructor MUST include the attribute name.
// The second defines whether a value is required.
// If true and there is no value, the XmlReader does an exception.

class uint32IParam : public baseIParam
{
public:
    Uint32 value;

    // constructor with definition of iParam name and default for the
    // required flag (false). Default value of parameter is NULL if
    // no value is supplied. This is for paramaters that are not required but
    // where the default value is NULL.
    // @param name const char* with name of IParam to match
    uint32IParam(const char* name)
        : baseIParam(name),
        value(0),
        _valueRequired(false)
    {
    }

    // constructor with definition of iParam name and default for the
    // required flag (false). Default value of parameter is integer defined
    // by supplied value. This is for parameters that are not required but
    // for which there is a nonNull default value if the parameter is not
    // supplied.
    // @param name const char* with name of IParam to match
    // @param uint32Value Uint32 value that is the default if the paramter
    // is not  found.
    // @param rqd Boolean (optional) that defines whether the parameter is
    // required on input.  If it is required, the iomt32Value is not used

    uint32IParam(const char* name, Uint32 _value, Boolean rqd = false)
        : baseIParam(name), value(0), _valueRequired(rqd) {}

    // get the attribute if it exists. The attribute name is defined in
    // the constructor
    // @param parser
    // @param testName attribute name from parse.
    // @emptyTag returns true if emptyTag returned true from parser
    // @return Returns true if testName matches the IParam defined by current
    // position in the parser
    Boolean get(XmlParser& parser, const char * testName,  Boolean& emptyTag)
    {
        if (_valueRequired)
        {
            XmlReader::rejectNullIParamValue(parser, emptyTag, testName);
        }
        if (System::strcasecmp(iParamName.getCString(), testName) == 0)
        {
            XmlReader::getUint32ValueElement(parser, value, true);
            return true;
        }
        return false;
    }

private:
    Boolean _valueRequired;
    uint32IParam();
    uint32IParam(const uint32IParam&);
    uint32IParam& operator = (const uint32IParam&);
};
// EXP_PULL_END

/************************************************************************
**
**      Common functions used by the decoders to avoid duplicate code.
**
**************************************************************************/
// test for valid end of XML and duplicate parameters on input
// This function returns if OK or executes appropriate exceptions if there
// is either a duplicate (duplicateParameter == true) or the
// end tag IPARAMVALUE is not found.
void _checkMissingEndTagOrDuplicateParamValue(
    XmlParser& parser, Boolean duplicateParameter, Boolean emptyTag)
{
    if (!emptyTag)
    {
        XmlReader::expectEndTag(parser, "IPARAMVALUE");
    }

    if (duplicateParameter)
    {
        _throwCIMExceptionDuplicateParameter();
    }
}

/**************************************************************************
**
**  Decode each CIMOperation type, processing the parameters for that type
**  and producing either a CIMMessage of the appropriate type or
**  an exception.
**
***************************************************************************/

CIMCreateClassRequestMessage*
    CIMOperationRequestDecoder::decodeCreateClassRequest(
        Uint32 queueId,
        XmlParser& parser,
        const String& messageId,
        const CIMNamespaceName& nameSpace)
{
    STAT_GETSTARTTIME

    CIMClass newClass;
    Boolean gotNewClass = false;

    Boolean emptyTag;
    Boolean duplicateParameter = false;

    for (const char* name;
         XmlReader::getIParamValueTag(parser, name, emptyTag); )
    {
        if (System::strcasecmp(name, "NewClass") == 0)
        {
            XmlReader::rejectNullIParamValue(parser, emptyTag, name);
            if (!XmlReader::getClassElement(parser, newClass))
            {
                _throwCIMExceptionInvalidParameter("NewClass");
            }
            duplicateParameter = gotNewClass;
            gotNewClass = true;
        }
        else
        {
            _throwCIMExceptionInvalidIParamName(name);
        }

        // generate exception if endtag error or duplicate attributes
        _checkMissingEndTagOrDuplicateParamValue(
            parser, duplicateParameter, emptyTag);
    }

    _testRequiredParametersExist(gotNewClass);

    AutoPtr<CIMCreateClassRequestMessage> request(
        new CIMCreateClassRequestMessage(
            messageId,
            nameSpace,
            newClass,
            QueueIdStack(queueId, _returnQueueId)));

    STAT_SERVERSTART

    return request.release();
}

CIMGetClassRequestMessage* CIMOperationRequestDecoder::decodeGetClassRequest(
    Uint32 queueId,
    XmlParser& parser,
    const String& messageId,
    const CIMNamespaceName& nameSpace)
{
    STAT_GETSTARTTIME

    // GetClass Parameters
    classNameIParam className("ClassName");
    booleanIParam localOnly("localOnly",true);
    booleanIParam includeQualifiers("IncludeQualifiers", true);
    booleanIParam includeClassOrigin("IncludeClassOrigin");
    propertyListIParam propertyList;

    Boolean duplicateParameter = false;
    Boolean emptyTag;

    for (const char* name;
         XmlReader::getIParamValueTag(parser, name, emptyTag); )
    {
        if(className.get(parser, name, emptyTag))
        {
            className.found(duplicateParameter);
        }
        else if(localOnly.get(parser, name, emptyTag))
        {
            localOnly.found(duplicateParameter);
        }
        else if(includeQualifiers.get(parser, name, emptyTag))
        {
            includeQualifiers.found(duplicateParameter);
        }
        else if(includeClassOrigin.get(parser, name,  emptyTag))
        {
            includeClassOrigin.found(duplicateParameter);
        }
        else if(propertyList.getSpecial(parser, name, emptyTag))
        {
            propertyList.found(duplicateParameter);
        }
        else
        {
            _throwCIMExceptionInvalidIParamName(name);
        }

        // generate exception if endtag error or duplicate attributes
        _checkMissingEndTagOrDuplicateParamValue(
            parser, duplicateParameter, emptyTag);
    }

    // reject if required parameter does not exist
    className.rejectIfNotExist();

    // Build message
    AutoPtr<CIMGetClassRequestMessage> request(new CIMGetClassRequestMessage(
        messageId,
        nameSpace,
        className.value,
        localOnly.value,
        includeQualifiers.value,
        includeClassOrigin.value,
        propertyList.value,
        QueueIdStack(queueId, _returnQueueId)));

    STAT_SERVERSTART

    return request.release();
}

CIMModifyClassRequestMessage*
    CIMOperationRequestDecoder::decodeModifyClassRequest(
        Uint32 queueId,
        XmlParser& parser,
        const String& messageId,
        const CIMNamespaceName& nameSpace)
{
    STAT_GETSTARTTIME

    CIMClass modifiedClass;
    Boolean gotClass = false;

    Boolean emptyTag;
    Boolean duplicateParameter = false;

    for (const char* name;
         XmlReader::getIParamValueTag(parser, name, emptyTag); )
    {
        if (System::strcasecmp(name, "ModifiedClass") == 0)
        {
            XmlReader::rejectNullIParamValue(parser, emptyTag, name);
            if (!XmlReader::getClassElement(parser, modifiedClass))
            {
                _throwCIMExceptionInvalidParameter("ModifiedClass");
            }
            duplicateParameter = gotClass;
            gotClass = true;
        }
        else
        {
            _throwCIMExceptionInvalidIParamName(name);
        }

        // generate exception if endtag error or duplicate attributes
        _checkMissingEndTagOrDuplicateParamValue(
            parser, duplicateParameter, emptyTag);
    }

    _testRequiredParametersExist("ModifiedClass", gotClass);

    AutoPtr<CIMModifyClassRequestMessage> request(
        new CIMModifyClassRequestMessage(
            messageId,
            nameSpace,
            modifiedClass,
            QueueIdStack(queueId, _returnQueueId)));

    STAT_SERVERSTART

    return request.release();
}

CIMEnumerateClassNamesRequestMessage*
    CIMOperationRequestDecoder::decodeEnumerateClassNamesRequest(
        Uint32 queueId,
        XmlParser& parser,
        const String& messageId,
        const CIMNamespaceName& nameSpace)
{
    STAT_GETSTARTTIME

    classNameIParam className("ClassName");
    booleanIParam deepInheritance("DeepInheritance");

    Boolean duplicateParameter = false;
    Boolean emptyTag;

    for (const char* name;
         XmlReader::getIParamValueTag(parser, name, emptyTag); )
    {
        if(className.getOptional(parser, name, emptyTag))
        {
            className.found(duplicateParameter);
        }
        else if(deepInheritance.get(parser, name, emptyTag))
        {
            deepInheritance.found(duplicateParameter);
        }
        else
        {
            _throwCIMExceptionInvalidIParamName(name);
        }

        // generate exception if endtag error or duplicate attributes
        _checkMissingEndTagOrDuplicateParamValue(
            parser, duplicateParameter, emptyTag);
    }

    // NOTE: className not required for this operation

    AutoPtr<CIMEnumerateClassNamesRequestMessage> request(
        new CIMEnumerateClassNamesRequestMessage(
            messageId,
            nameSpace,
            className.value,
            deepInheritance.value,
            QueueIdStack(queueId, _returnQueueId)));

    STAT_SERVERSTART

    return request.release();
}

CIMEnumerateClassesRequestMessage*
    CIMOperationRequestDecoder::decodeEnumerateClassesRequest(
        Uint32 queueId,
        XmlParser& parser,
        const String& messageId,
        const CIMNamespaceName& nameSpace)
{
    STAT_GETSTARTTIME

    // EnumerateClasses Parameters
    classNameIParam className("ClassName");
    booleanIParam deepInheritance("deepInheritance");
    booleanIParam localOnly("localOnly",true);
    booleanIParam includeQualifiers("IncludeQualifiers", true);
    booleanIParam includeClassOrigin("IncludeClassOrigin");

    Boolean duplicateParameter = false;
    Boolean emptyTag;

    for (const char* name;
         XmlReader::getIParamValueTag(parser, name, emptyTag); )
    {
        if(className.getOptional(parser, name, emptyTag))
        {
            className.found(duplicateParameter);
        }
        else if(deepInheritance.get(parser, name, emptyTag))
        {
            deepInheritance.found(duplicateParameter);
        }
        else if(localOnly.get(parser, name, emptyTag))
        {
            localOnly.found(duplicateParameter);
        }
        else if(includeQualifiers.get(parser, name, emptyTag))
        {
            includeQualifiers.found(duplicateParameter);
        }
        else if(includeClassOrigin.get(parser, name,  emptyTag))
        {
            includeClassOrigin.found(duplicateParameter);
        }
        else
        {
            _throwCIMExceptionInvalidIParamName(name);
        }

        // generate exception if endtag error or duplicate attributes
        _checkMissingEndTagOrDuplicateParamValue(
            parser, duplicateParameter, emptyTag);
    }

    // NOTE: Class name not required for this enumerate.

    AutoPtr<CIMEnumerateClassesRequestMessage> request(
        new CIMEnumerateClassesRequestMessage(
            messageId,
            nameSpace,
            className.value,
            deepInheritance.value,
            localOnly.value,
            includeQualifiers.value,
            includeClassOrigin.value,
            QueueIdStack(queueId, _returnQueueId)));

    STAT_SERVERSTART

    return request.release();
}

CIMDeleteClassRequestMessage*
    CIMOperationRequestDecoder::decodeDeleteClassRequest(
        Uint32 queueId,
        XmlParser& parser,
        const String& messageId,
        const CIMNamespaceName& nameSpace)
{
    STAT_GETSTARTTIME

    classNameIParam className("ClassName");

    Boolean duplicateParameter = false;
    Boolean emptyTag;

    for (const char* name;
         XmlReader::getIParamValueTag(parser, name, emptyTag); )
    {
        if(className.get(parser, name, emptyTag))
        {
            className.found(duplicateParameter);
        }
        else
        {
            _throwCIMExceptionInvalidIParamName(name);
        }

        // generate exception if endtag error or duplicate attributes
        _checkMissingEndTagOrDuplicateParamValue(
            parser, duplicateParameter, emptyTag);
    }

    _testRequiredParametersExist(className.name(),className.got);

    AutoPtr<CIMDeleteClassRequestMessage> request(
        new CIMDeleteClassRequestMessage(
            messageId,
            nameSpace,
            className.value,
            QueueIdStack(queueId, _returnQueueId)));

    STAT_SERVERSTART

    return request.release();
}

CIMCreateInstanceRequestMessage*
    CIMOperationRequestDecoder::decodeCreateInstanceRequest(
        Uint32 queueId,
        XmlParser& parser,
        const String& messageId,
        const CIMNamespaceName& nameSpace)
{
    STAT_GETSTARTTIME

    CIMInstance newInstance;
    Boolean gotInstance = false;

    Boolean emptyTag;
    Boolean duplicateParameter = false;

    for (const char* name;
         XmlReader::getIParamValueTag(parser, name, emptyTag); )
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
            _throwCIMExceptionInvalidIParamName(name);
        }

        // generate exception if endtag error or duplicate attributes
        _checkMissingEndTagOrDuplicateParamValue(
            parser, duplicateParameter, emptyTag);
    }

    _testRequiredParametersExist("NewInstance", gotInstance);

    AutoPtr<CIMCreateInstanceRequestMessage> request(
        new CIMCreateInstanceRequestMessage(
            messageId,
            nameSpace,
            newInstance,
            QueueIdStack(queueId, _returnQueueId)));

    STAT_SERVERSTART

    return request.release();
}

CIMGetInstanceRequestMessage*
    CIMOperationRequestDecoder::decodeGetInstanceRequest(
        Uint32 queueId,
        XmlParser& parser,
        const String& messageId,
        const CIMNamespaceName& nameSpace)
{
    STAT_GETSTARTTIME

    instanceNameIParam instanceName("InstanceName");
    // This attribute is accepted for compatibility reasons, but is
    // not honored because it is deprecated.
    booleanIParam localOnly("localOnly",true);
    booleanIParam includeQualifiers("IncludeQualifiers");
    booleanIParam includeClassOrigin("IncludeClassOrigin");
    propertyListIParam propertyList;

    Boolean duplicateParameter = false;
    Boolean emptyTag;

    for (const char* name;
         XmlReader::getIParamValueTag(parser, name, emptyTag); )
    {
        if(instanceName.get(parser, name, emptyTag))
        {
            instanceName.found(duplicateParameter);
        }
        // localOnly is accepted for compatibility reasons, but is
        // not honored because it is deprecated.
        else if(localOnly.get(parser, name, emptyTag))
        {
            localOnly.found(duplicateParameter);
        }
        else if(includeQualifiers.get(parser, name, emptyTag))
        {
            includeQualifiers.found(duplicateParameter);
        }
        else if(includeClassOrigin.get(parser, name,  emptyTag))
        {
            includeClassOrigin.found(duplicateParameter);
        }
        else if(propertyList.get(parser, name, emptyTag))
        {
            propertyList.found(duplicateParameter);
        }
        else
        {
            _throwCIMExceptionInvalidIParamName(name);
        }

        // generate exception if endtag error or duplicate attributes
        _checkMissingEndTagOrDuplicateParamValue(
            parser, duplicateParameter, emptyTag);
    }

    _testRequiredParametersExist(instanceName.got);

    AutoPtr<CIMGetInstanceRequestMessage> request(
        new CIMGetInstanceRequestMessage(
            messageId,
            nameSpace,
            instanceName.value,
#ifdef PEGASUS_DISABLE_INSTANCE_QUALIFIERS
            false,
#else
            includeQualifiers.value,
#endif
            includeClassOrigin.value,
            propertyList.value,
            QueueIdStack(queueId, _returnQueueId)));

    STAT_SERVERSTART

    return request.release();
}


CIMModifyInstanceRequestMessage*
    CIMOperationRequestDecoder::decodeModifyInstanceRequest(
        Uint32 queueId,
        XmlParser& parser,
        const String& messageId,
        const CIMNamespaceName& nameSpace)
{
    STAT_GETSTARTTIME

    CIMInstance modifiedInstance;
    Boolean gotInstance = false;

    booleanIParam includeQualifiers("IncludeQualifiers", true);
    propertyListIParam propertyList;

    Boolean emptyTag;
    Boolean duplicateParameter = false;

    for (const char* name;
         XmlReader::getIParamValueTag(parser, name, emptyTag); )
    {
        if (System::strcasecmp(name, "ModifiedInstance") == 0)
        {
            XmlReader::rejectNullIParamValue(parser, emptyTag, name);
            XmlReader::getNamedInstanceElement(parser, modifiedInstance);
            duplicateParameter = gotInstance;
            gotInstance = true;
        }
        else if(includeQualifiers.get(parser, name, emptyTag))
        {
            includeQualifiers.found(duplicateParameter);
        }
        else if(propertyList.getSpecial(parser, name, emptyTag))
        {
            propertyList.found(duplicateParameter);
        }
        else
        {
            _throwCIMExceptionInvalidIParamName(name);
        }

        _checkMissingEndTagOrDuplicateParamValue(
            parser, duplicateParameter, emptyTag);
    }

    _testRequiredParametersExist("ModifiedInstance", gotInstance);

    AutoPtr<CIMModifyInstanceRequestMessage> request(
        new CIMModifyInstanceRequestMessage(
            messageId,
            nameSpace,
            modifiedInstance,
            includeQualifiers.value,
            propertyList.value,
            QueueIdStack(queueId, _returnQueueId)));

    STAT_SERVERSTART

    return request.release();
}

CIMEnumerateInstancesRequestMessage*
    CIMOperationRequestDecoder::decodeEnumerateInstancesRequest(
        Uint32 queueId,
        XmlParser& parser,
        const String& messageId,
        const CIMNamespaceName& nameSpace)
{
    STAT_GETSTARTTIME

    // EnumerateInstance Parameters
    classNameIParam className("ClassName");
    booleanIParam deepInheritance("DeepInheritance", true);
    // localOnly is accepted for compatibility reasons, but is
    // not honored because it is deprecated.
    booleanIParam localOnly("localOnly", true);
    booleanIParam includeQualifiers("IncludeQualifiers");
    booleanIParam includeClassOrigin("IncludeClassOrigin");
    propertyListIParam propertyList;

    Boolean duplicateParameter = false;
    Boolean emptyTag;

    for (const char* name;
         XmlReader::getIParamValueTag(parser, name, emptyTag); )
    {
        if(className.get(parser, name, emptyTag))
        {
            className.found(duplicateParameter);
        }
        else if(deepInheritance.get(parser, name, emptyTag))
        {
            deepInheritance.found(duplicateParameter);
        }
        // This attribute is accepted for compatibility reasons, but is
        // not honored because it is deprecated.
        else if(localOnly.get(parser, name, emptyTag))
        {
            localOnly.found(duplicateParameter);
        }
        else if(includeQualifiers.get(parser, name, emptyTag))
        {
            includeQualifiers.found(duplicateParameter);
        }
        else if(includeClassOrigin.get(parser, name,  emptyTag))
        {
            includeClassOrigin.found(duplicateParameter);
        }
        else if(propertyList.get(parser, name, emptyTag))
        {
            propertyList.found(duplicateParameter);
        }
        else
        {
            _throwCIMExceptionInvalidIParamName(name);
        }

        // generate exception if endtag error or duplicate attributes
        _checkMissingEndTagOrDuplicateParamValue(
            parser, duplicateParameter, emptyTag);
    }

    // Reject if required parameter does not exist in request
    className.rejectIfNotExist();

    AutoPtr<CIMEnumerateInstancesRequestMessage> request(
        new CIMEnumerateInstancesRequestMessage(
            messageId,
            nameSpace,
            className.value,
            deepInheritance.value,
#ifdef PEGASUS_DISABLE_INSTANCE_QUALIFIERS
            false,
#else
            includeQualifiers.value,
#endif
            includeClassOrigin.value,
            propertyList.value,
            QueueIdStack(queueId, _returnQueueId)));

    STAT_SERVERSTART

    return request.release();
}

CIMEnumerateInstanceNamesRequestMessage*
    CIMOperationRequestDecoder::decodeEnumerateInstanceNamesRequest(
        Uint32 queueId,
        XmlParser& parser,
        const String& messageId,
        const CIMNamespaceName& nameSpace)
{
    STAT_GETSTARTTIME

    classNameIParam className("ClassName");

    Boolean duplicateParameter = false;
    Boolean emptyTag;

    for (const char* name;
         XmlReader::getIParamValueTag(parser, name, emptyTag); )
    {
        if(className.get(parser, name, emptyTag))
        {
            className.found(duplicateParameter);
        }
        else
        {
            _throwCIMExceptionInvalidIParamName(name);
        }

        // generate exception if endtag error or duplicate attributes
        _checkMissingEndTagOrDuplicateParamValue(
            parser, duplicateParameter, emptyTag);
    }

    // Reject if required parameter does not exist in request
    className.rejectIfNotExist();

    AutoPtr<CIMEnumerateInstanceNamesRequestMessage> request(
        new CIMEnumerateInstanceNamesRequestMessage(
            messageId,
            nameSpace,
            className.value,
            QueueIdStack(queueId, _returnQueueId)));

    STAT_SERVERSTART

    return request.release();
}

CIMDeleteInstanceRequestMessage*
    CIMOperationRequestDecoder::decodeDeleteInstanceRequest(
        Uint32 queueId,
        XmlParser& parser,
        const String& messageId,
        const CIMNamespaceName& nameSpace)
{
    STAT_GETSTARTTIME

    instanceNameIParam instanceName("InstanceName");

    Boolean duplicateParameter = false;
    Boolean emptyTag;

    for (const char* name;
         XmlReader::getIParamValueTag(parser, name, emptyTag); )
    {
        if(instanceName.get(parser, name, emptyTag))
        {
            instanceName.found(duplicateParameter);
        }
        else
        {
            _throwCIMExceptionInvalidIParamName(name);
        }

        // generate exception if endtag error or duplicate attributes
        _checkMissingEndTagOrDuplicateParamValue(
            parser, duplicateParameter, emptyTag);
    }

    _testRequiredParametersExist(instanceName.got);

    AutoPtr<CIMDeleteInstanceRequestMessage> request(
        new CIMDeleteInstanceRequestMessage(
            messageId,
            nameSpace,
            instanceName.value,
            QueueIdStack(queueId, _returnQueueId)));

    STAT_SERVERSTART

    return request.release();
}

CIMSetQualifierRequestMessage*
    CIMOperationRequestDecoder::decodeSetQualifierRequest(
        Uint32 queueId,
        XmlParser& parser,
        const String& messageId,
        const CIMNamespaceName& nameSpace)
{
    STAT_GETSTARTTIME

    CIMQualifierDecl qualifierDeclaration;
    Boolean duplicateParameter = false;
    Boolean gotQualifierDeclaration = false;
    Boolean emptyTag;

    for (const char* name;
         XmlReader::getIParamValueTag(parser, name, emptyTag); )
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
            _throwCIMExceptionInvalidIParamName(name);
        }

        // generate exception if endtag error or duplicate attributes
        _checkMissingEndTagOrDuplicateParamValue(
            parser, duplicateParameter, emptyTag);
    }

    _testRequiredParametersExist(gotQualifierDeclaration);

    AutoPtr<CIMSetQualifierRequestMessage> request(
        new CIMSetQualifierRequestMessage(
            messageId,
            nameSpace,
            qualifierDeclaration,
            QueueIdStack(queueId, _returnQueueId)));

    STAT_SERVERSTART

    return request.release();
}

CIMGetQualifierRequestMessage*
    CIMOperationRequestDecoder::decodeGetQualifierRequest(
        Uint32 queueId,
        XmlParser& parser,
        const String& messageId,
        const CIMNamespaceName& nameSpace)
{
    STAT_GETSTARTTIME

    String qualifierNameString;
    CIMName qualifierName;
    Boolean duplicateParameter = false;
    Boolean gotQualifierName = false;
    Boolean emptyTag;

    for (const char* name;
         XmlReader::getIParamValueTag(parser, name, emptyTag); )
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
            _throwCIMExceptionInvalidIParamName(name);
        }

        // generate exception if endtag error or duplicate attributes
        _checkMissingEndTagOrDuplicateParamValue(
            parser, duplicateParameter, emptyTag);
    }

    _testRequiredParametersExist(gotQualifierName);

    AutoPtr<CIMGetQualifierRequestMessage> request(
        new CIMGetQualifierRequestMessage(
            messageId,
            nameSpace,
            qualifierName,
            QueueIdStack(queueId, _returnQueueId)));

    STAT_SERVERSTART

    return request.release();
}

CIMEnumerateQualifiersRequestMessage*
    CIMOperationRequestDecoder::decodeEnumerateQualifiersRequest(
        Uint32 queueId,
        XmlParser& parser,
        const String& messageId,
        const CIMNamespaceName& nameSpace)
{
    STAT_GETSTARTTIME
    Boolean emptyTag;

    for (const char* name;
         XmlReader::getIParamValueTag(parser, name, emptyTag); )
    {
        // No IPARAMVALUEs are defined for this operation
        _throwCIMExceptionInvalidIParamName(name);
    }

    AutoPtr<CIMEnumerateQualifiersRequestMessage> request(
        new CIMEnumerateQualifiersRequestMessage(
            messageId,
            nameSpace,
            QueueIdStack(queueId, _returnQueueId)));

    STAT_SERVERSTART

    return request.release();
}

CIMDeleteQualifierRequestMessage*
    CIMOperationRequestDecoder::decodeDeleteQualifierRequest(
        Uint32 queueId,
        XmlParser& parser,
        const String& messageId,
        const CIMNamespaceName& nameSpace)
{
    STAT_GETSTARTTIME

    String qualifierNameString;
    CIMName qualifierName;
    Boolean gotQualifierName = false;

    Boolean emptyTag;
    Boolean duplicateParameter = false;

    for (const char* name;
         XmlReader::getIParamValueTag(parser, name, emptyTag); )
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
            _throwCIMExceptionInvalidIParamName(name);
        }


        // generate exception if endtag error or duplicate attributes
        _checkMissingEndTagOrDuplicateParamValue(
            parser, duplicateParameter, emptyTag);
    }

    _testRequiredParametersExist(gotQualifierName);

    AutoPtr<CIMDeleteQualifierRequestMessage> request(
        new CIMDeleteQualifierRequestMessage(
            messageId,
            nameSpace,
            qualifierName,
            QueueIdStack(queueId, _returnQueueId)));

    STAT_SERVERSTART

    return request.release();
}

CIMReferenceNamesRequestMessage*
    CIMOperationRequestDecoder::decodeReferenceNamesRequest(
        Uint32 queueId,
        XmlParser& parser,
        const String& messageId,
        const CIMNamespaceName& nameSpace)
{
    STAT_GETSTARTTIME
    objectNameIParam objectName;
    classNameIParam resultClass("ResultClass");
    stringIParam role("role", false);

    Boolean duplicateParameter = false;
    Boolean emptyTag;

    for (const char* name;
        XmlReader::getIParamValueTag(parser, name, emptyTag); )
    {
        if(objectName.get(parser, name, emptyTag))
        {
            objectName.found(duplicateParameter);
        }
        else if (resultClass.getOptional(parser, name, emptyTag))
        {
            resultClass.found(duplicateParameter);
        }
        else if(role.get(parser, name, emptyTag))
        {
            role.found(duplicateParameter);
        }
        else
        {
            _throwCIMExceptionInvalidIParamName(name);
        }

        // generate exception if endtag error or duplicate attributes
        _checkMissingEndTagOrDuplicateParamValue(
            parser, duplicateParameter, emptyTag);
    }

    // Reject if required parameter does not exist in request
    objectName.rejectIfNotExist();

    AutoPtr<CIMReferenceNamesRequestMessage> request(
        new CIMReferenceNamesRequestMessage(
            messageId,
            nameSpace,
            objectName.value,
            resultClass.value,
            role.value,
            QueueIdStack(queueId, _returnQueueId),
            objectName.isClassNameElement));

    STAT_SERVERSTART

    return request.release();
}

CIMReferencesRequestMessage*
    CIMOperationRequestDecoder::decodeReferencesRequest(
        Uint32 queueId,
        XmlParser& parser,
        const String& messageId,
        const CIMNamespaceName& nameSpace)
{
    STAT_GETSTARTTIME

    objectNameIParam objectName;
    classNameIParam resultClass("ResultClass");
    stringIParam role("role", false);
    booleanIParam includeQualifiers("IncludeQualifiers");
    booleanIParam includeClassOrigin("IncludeClassOrigin");
    propertyListIParam propertyList;

    Boolean duplicateParameter = false;
    Boolean emptyTag;

    for (const char* name;
         XmlReader::getIParamValueTag(parser, name, emptyTag); )
    {
        if(objectName.get(parser, name, emptyTag))
        {
            objectName.found(duplicateParameter);
        }
        else if(role.get(parser, name, emptyTag))
        {
            role.found(duplicateParameter);
        }
        else if (resultClass.getOptional(parser, name, emptyTag))
        {
            resultClass.found(duplicateParameter);
        }
        else if(includeQualifiers.get(parser, name, emptyTag))
        {
            includeQualifiers.found(duplicateParameter);
        }
        else if(includeClassOrigin.get(parser, name,  emptyTag))
        {
            includeClassOrigin.found(duplicateParameter);
        }
        else if(propertyList.get(parser, name, emptyTag))
        {
            propertyList.found(duplicateParameter);
        }
        else
        {
            _throwCIMExceptionInvalidIParamName(name);
        }

        // generate exception if endtag error or duplicate attributes
        _checkMissingEndTagOrDuplicateParamValue(
            parser, duplicateParameter, emptyTag);
    }

    // Reject if required parameter does not exist in request
    objectName.rejectIfNotExist();

    AutoPtr<CIMReferencesRequestMessage> request(
        new CIMReferencesRequestMessage(
            messageId,
            nameSpace,
            objectName.value,
            resultClass.value,
            role.value,
            includeQualifiers.value,
            includeClassOrigin.value,
            propertyList.value,
            QueueIdStack(queueId, _returnQueueId),
            objectName.isClassNameElement));

    STAT_SERVERSTART

    return request.release();
}

CIMAssociatorNamesRequestMessage*
    CIMOperationRequestDecoder::decodeAssociatorNamesRequest(
        Uint32 queueId,
        XmlParser& parser,
        const String& messageId,
        const CIMNamespaceName& nameSpace)
{
    STAT_GETSTARTTIME

    objectNameIParam objectName;
    classNameIParam assocClass("AssocClass");
    classNameIParam resultClass("ResultClass");
    stringIParam role("role", false);
    stringIParam resultRole("Resultrole", false);

    Boolean emptyTag;
    Boolean duplicateParameter = false;

    for (const char* name;
         XmlReader::getIParamValueTag(parser, name, emptyTag); )
    {
        if(objectName.get(parser, name, emptyTag))
        {
            objectName.found(duplicateParameter);
        }
        else if (assocClass.getOptional(parser, name, emptyTag))
        {
            assocClass.found(duplicateParameter);
        }
        else if (resultClass.getOptional(parser, name, emptyTag))
        {
            resultClass.found(duplicateParameter);
        }
        else if(role.get(parser, name, emptyTag))
        {
            role.found(duplicateParameter);
        }
        else if(resultRole.get(parser, name, emptyTag))
        {
            resultRole.found(duplicateParameter);
        }
        else
        {
            _throwCIMExceptionInvalidIParamName(name);

        }

        // generate exception if endtag error or duplicate attributes
        _checkMissingEndTagOrDuplicateParamValue(
            parser, duplicateParameter, emptyTag);
    }

    // Reject if required parameter does not exist in request
    objectName.rejectIfNotExist();

    AutoPtr<CIMAssociatorNamesRequestMessage> request(
        new CIMAssociatorNamesRequestMessage(
            messageId,
            nameSpace,
            objectName.value,
            assocClass.value,
            resultClass.value,
            role.value,
            resultRole.value,
            QueueIdStack(queueId, _returnQueueId),
            objectName.isClassNameElement));

    STAT_SERVERSTART

    return request.release();
}

CIMAssociatorsRequestMessage*
    CIMOperationRequestDecoder::decodeAssociatorsRequest(
        Uint32 queueId,
        XmlParser& parser,
        const String& messageId,
        const CIMNamespaceName& nameSpace)
{
    STAT_GETSTARTTIME

    // Associator Operation Parameter Declarations
    objectNameIParam objectName;
    classNameIParam assocClass("AssocClass");
    classNameIParam resultClass("ResultClass");
    stringIParam resultRole("Resultrole", false);
    stringIParam role("role", false);
    booleanIParam includeQualifiers("IncludeQualifiers");
    booleanIParam includeClassOrigin("IncludeClassOrigin");
    propertyListIParam propertyList;

    Boolean duplicateParameter = false;
    Boolean emptyTag;

    for (const char* name;
         XmlReader::getIParamValueTag(parser, name, emptyTag); )
    {
        if(objectName.get(parser, name, emptyTag))
        {
            objectName.found(duplicateParameter);
        }
        else if (assocClass.getOptional(parser, name, emptyTag))
        {
            assocClass.found(duplicateParameter);
        }
        else if (resultClass.getOptional(parser, name, emptyTag))
        {
            resultClass.found(duplicateParameter);
        }
        else if(role.get(parser, name, emptyTag))
        {
            role.found(duplicateParameter);
        }
        else if(resultRole.get(parser, name, emptyTag))
        {
            resultRole.found(duplicateParameter);
        }
        else if(includeQualifiers.get(parser, name, emptyTag))
        {
            includeQualifiers.found(duplicateParameter);
        }
        else if(includeClassOrigin.get(parser, name,  emptyTag))
        {
            includeClassOrigin.found(duplicateParameter);
        }
        else if(propertyList.get(parser, name, emptyTag))
        {
            propertyList.found(duplicateParameter);
        }
        else
        {
            _throwCIMExceptionInvalidIParamName(name);
        }

        // generate exception if endtag error or duplicate attributes
        _checkMissingEndTagOrDuplicateParamValue(
            parser, duplicateParameter, emptyTag);
    }

    // Reject if required parameter does not exist in request
    objectName.rejectIfNotExist();

    AutoPtr<CIMAssociatorsRequestMessage> request(
        new CIMAssociatorsRequestMessage(
            messageId,
            nameSpace,
            objectName.value,
            assocClass.value,
            resultClass.value,
            role.value,
            resultRole.value,
            includeQualifiers.value,
            includeClassOrigin.value,
            propertyList.value,
            QueueIdStack(queueId, _returnQueueId),
            objectName.isClassNameElement));

    STAT_SERVERSTART

    return request.release();
}

CIMGetPropertyRequestMessage*
    CIMOperationRequestDecoder::decodeGetPropertyRequest(
        Uint32 queueId,
        XmlParser& parser,
        const String& messageId,
        const CIMNamespaceName& nameSpace)
{
    STAT_GETSTARTTIME

    CIMObjectPath instanceName;
    String propertyName;
    Boolean duplicateParameter = false;
    Boolean gotInstanceName = false;
    Boolean gotPropertyName = false;
    Boolean emptyTag;

    for (const char* name;
         XmlReader::getIParamValueTag(parser, name, emptyTag); )
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
            _throwCIMExceptionInvalidIParamName(name);
        }

        // generate exception if endtag error or duplicate attributes
        _checkMissingEndTagOrDuplicateParamValue(
            parser, duplicateParameter, emptyTag);
    }

    _testRequiredParametersExist(gotInstanceName && gotPropertyName);

    AutoPtr<CIMGetPropertyRequestMessage> request(
        new CIMGetPropertyRequestMessage(
            messageId,
            nameSpace,
            instanceName,
            propertyName,
            QueueIdStack(queueId, _returnQueueId)));

    STAT_SERVERSTART

    return request.release();
}

CIMSetPropertyRequestMessage*
    CIMOperationRequestDecoder::decodeSetPropertyRequest(
        Uint32 queueId,
        XmlParser& parser,
        const String& messageId,
        const CIMNamespaceName& nameSpace)
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
         XmlReader::getIParamValueTag(parser, name, emptyTag); )
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
            _throwCIMExceptionInvalidIParamName(name);
        }

        // generate exception if endtag error or duplicate attributes
        _checkMissingEndTagOrDuplicateParamValue(
            parser, duplicateParameter, emptyTag);
    }

    _testRequiredParametersExist(gotInstanceName && gotPropertyName);

    AutoPtr<CIMSetPropertyRequestMessage> request(
        new CIMSetPropertyRequestMessage(
            messageId,
            nameSpace,
            instanceName,
            propertyName,
            propertyValue,
            QueueIdStack(queueId, _returnQueueId)));

    STAT_SERVERSTART

    return request.release();
}

CIMExecQueryRequestMessage* CIMOperationRequestDecoder::decodeExecQueryRequest(
    Uint32 queueId,
    XmlParser& parser,
    const String& messageId,
    const CIMNamespaceName& nameSpace)
{
    STAT_GETSTARTTIME

    // define execQuery parameters.  Values are required parameters exist.
    stringIParam queryLanguage("QueryLanguage", true);
    stringIParam query("Query", true);

    Boolean emptyTag;
    Boolean duplicateParameter = false;

    for (const char* name;
         XmlReader::getIParamValueTag(parser, name, emptyTag); )
    {
        if(queryLanguage.get(parser, name, emptyTag))
        {
            queryLanguage.found(duplicateParameter);
        }
        else if(query.get(parser, name, emptyTag))
        {
            query.found(duplicateParameter);
        }
        else
        {
            _throwCIMExceptionInvalidIParamName(name);
        }

        // generate exception if endtag error or duplicate attributes
        _checkMissingEndTagOrDuplicateParamValue(parser,
            duplicateParameter, emptyTag);
    }

    _testRequiredParametersExist(queryLanguage.got && query.got);

    AutoPtr<CIMExecQueryRequestMessage> request(
        new CIMExecQueryRequestMessage(
            messageId,
            nameSpace,
            queryLanguage.value,
            query.value,
            QueueIdStack(queueId, _returnQueueId)));

    STAT_SERVERSTART

    return request.release();
}

CIMInvokeMethodRequestMessage*
    CIMOperationRequestDecoder::decodeInvokeMethodRequest(
        Uint32 queueId,
        XmlParser& parser,
        const String& messageId,
        const CIMObjectPath& reference,
        const String& cimMethodName)
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
            QueueIdStack(queueId, _returnQueueId)));

    STAT_SERVERSTART

    return request.release();
}

// EXP_PULL_BEGIN
CIMOpenEnumerateInstancesRequestMessage*
    CIMOperationRequestDecoder::decodeOpenEnumerateInstancesRequest(
        Uint32 queueId,
        XmlParser& parser,
        const String& messageId,
        const CIMNamespaceName& nameSpace)
{
    STAT_GETSTARTTIME

    // EnumerateInstance Parameters
    classNameIParam className("ClassName");
    booleanIParam deepInheritance("DeepInheritance", true);
    booleanIParam includeQualifiers("IncludeQualifiers");
    booleanIParam includeClassOrigin("IncludeClassOrigin");
    propertyListIParam propertyList;
    // the following are optional parameters for all of the Open requests
    // [IN,OPTIONAL,NULL] string FilterQueryLanguage = NULL,
    // [IN,OPTIONAL,NULL] string FilterQuery = NULL,
    stringIParam filterQueryLanguage("FilterQueryLanguage",false);
    stringIParam filterQuery("FilterQuery", false);
    //[IN,OPTIONAL] Boolean ContinueOnError = false,
    booleanIParam continueOnError("ContinueOnError");
    // [IN,OPTIONAL] uint32 MaxObjectCount = 0
    uint32IParam maxObjectCount("MaxObjectCount", 0);
    // [IN,OPTIONAL,NULL] uint32 OperationTimeout = NULL
    // The default for this parameter is NULL (Server; server sets timeout)
    uint32ArgIParam operationTimeout("OperationTimeout");

    Boolean duplicateParameter = false;

    Boolean emptyTag;

    for (const char* name;
         XmlReader::getIParamValueTag(parser, name, emptyTag); )
    {
        if(className.get(parser, name, emptyTag))
        {
            className.found(duplicateParameter);
        }
        else if(deepInheritance.get(parser, name, emptyTag))
        {
            deepInheritance.found(duplicateParameter);
        }
        else if(includeQualifiers.get(parser, name, emptyTag))
        {
            includeQualifiers.found(duplicateParameter);
        }
        else if(includeClassOrigin.get(parser, name,  emptyTag))
        {
            includeClassOrigin.found(duplicateParameter);
        }
        // [IN,OPTIONAL,NULL] string FilterQueryLanguage = NULL,
        else if(filterQueryLanguage.get(parser, name, emptyTag))
        {
            filterQueryLanguage.found(duplicateParameter);
        }
        // [IN,OPTIONAL,NULL] string FilterQuery = NULL,
        else if(filterQuery.get(parser, name, emptyTag))
        {
            filterQuery.found(duplicateParameter);
        }
        // [IN,OPTIONAL] Boolean ContinueOnError = false,
        else if (continueOnError.get(parser, name, emptyTag))
        {
            continueOnError.found(duplicateParameter);
        }
        // [IN,OPTIONAL,NULL] uint32 OperationTimeout = NULL,
        else if (operationTimeout.get(parser, name, emptyTag))
        {
            operationTimeout.found(duplicateParameter);
        }
        // [IN,OPTIONAL] uint32 MaxObjectCount = 0
        else if (maxObjectCount.get(parser, name, emptyTag))
        {
            maxObjectCount.found(duplicateParameter);
        }
        else if(propertyList.get(parser, name, emptyTag))
        {
            propertyList.found(duplicateParameter);
        }
        else
        {
            _throwCIMExceptionInvalidIParamName(name);
        }

        // generate exception if endtag error or duplicate attributes
        _checkMissingEndTagOrDuplicateParamValue(
            parser, duplicateParameter, emptyTag);
    }

    // Reject if required parameter does not exist in request
    className.rejectIfNotExist();

    AutoPtr<CIMOpenEnumerateInstancesRequestMessage> request(
        new CIMOpenEnumerateInstancesRequestMessage(
            messageId,
            nameSpace,
            className.value,
            deepInheritance.value,
            includeClassOrigin.value,
            propertyList.value,
            filterQueryLanguage.value,
            filterQuery.value,
            operationTimeout.value,
            continueOnError.value,
            maxObjectCount.value,
            QueueIdStack(queueId, _returnQueueId)));

    STAT_SERVERSTART

    return request.release();
}


CIMOpenEnumerateInstancePathsRequestMessage*
    CIMOperationRequestDecoder::decodeOpenEnumerateInstancePathsRequest(
        Uint32 queueId,
        XmlParser& parser,
        const String& messageId,
        const CIMNamespaceName& nameSpace)
{
    STAT_GETSTARTTIME

    // EnumerateInstance Parameters
    classNameIParam className("ClassName");
    stringIParam filterQueryLanguage("FilterQueryLanguage",false);
    stringIParam filterQuery("FilterQuery", false);
    booleanIParam continueOnError("ContinueOnError");
    // [IN,OPTIONAL] uint32 MaxObjectCount = 0
    uint32IParam maxObjectCount("MaxObjectCount", 0);
    // [IN,OPTIONAL,NULL] uint32 OperationTimeout = NULL,
    uint32ArgIParam operationTimeout("OperationTimeout");

    Boolean duplicateParameter = false;
    Boolean emptyTag;

    for (const char* name;
         XmlReader::getIParamValueTag(parser, name, emptyTag); )
    {
        if(className.get(parser, name, emptyTag))
        {
            className.found(duplicateParameter);
        }
        // [IN,OPTIONAL,NULL] string FilterQueryLanguage = NULL,
        else if(filterQueryLanguage.get(parser, name, emptyTag))
        {
            filterQueryLanguage.found(duplicateParameter);
        }
        // [IN,OPTIONAL,NULL] string FilterQuery = NULL,
        else if(filterQuery.get(parser, name, emptyTag))
        {
            filterQuery.found(duplicateParameter);
        }
        // [IN,OPTIONAL] Boolean ContinueOnError = false,
        else if (continueOnError.get(parser, name, emptyTag))
        {
            continueOnError.found(duplicateParameter);
        }
        // [IN,OPTIONAL,NULL] uint32 OperationTimeout = NULL,
        else if (operationTimeout.get(parser, name, emptyTag))
        {
            operationTimeout.found(duplicateParameter);
        }
        // [IN,OPTIONAL] uint32 MaxObjectCount = 0
        else if (maxObjectCount.get(parser, name, emptyTag))
        {
            maxObjectCount.found(duplicateParameter);
        }
        else
        {
            _throwCIMExceptionInvalidIParamName(name);
        }

        // generate exception if endtag error or duplicate attributes
        _checkMissingEndTagOrDuplicateParamValue(
            parser, duplicateParameter, emptyTag);
    }

    // Reject if required parameter does not exist in request
    className.rejectIfNotExist();

    AutoPtr<CIMOpenEnumerateInstancePathsRequestMessage> request(
        new CIMOpenEnumerateInstancePathsRequestMessage(
            messageId,
            nameSpace,
            className.value,
            filterQueryLanguage.value,
            filterQuery.value,
            operationTimeout.value,
            continueOnError.value,
            maxObjectCount.value,
            QueueIdStack(queueId, _returnQueueId)));

    STAT_SERVERSTART

    return request.release();
}

CIMOpenReferenceInstancesRequestMessage*
    CIMOperationRequestDecoder::decodeOpenReferenceInstancesRequest(
        Uint32 queueId,
        XmlParser& parser,
        const String& messageId,
        const CIMNamespaceName& nameSpace)
{
    STAT_GETSTARTTIME

    objectNameIParam objectName("InstanceName");
    classNameIParam resultClass("ResultClass");
    stringIParam role("role", false);
    booleanIParam includeClassOrigin("IncludeClassOrigin");
    propertyListIParam propertyList;

    stringIParam filterQueryLanguage("FilterQueryLanguage",false);
    stringIParam filterQuery("FilterQuery", false);
    booleanIParam continueOnError("ContinueOnError");
    // [IN,OPTIONAL] uint32 MaxObjectCount = 0
    uint32IParam maxObjectCount("MaxObjectCount", 0);
    uint32ArgIParam operationTimeout("OperationTimeout");

    Boolean duplicateParameter = false;
    Boolean emptyTag;


    for (const char* name;
         XmlReader::getIParamValueTag(parser, name, emptyTag); )
    {
        if(objectName.get(parser, name, emptyTag))
        {
            objectName.found(duplicateParameter);
        }
        else if(role.get(parser, name, emptyTag))
        {
            role.found(duplicateParameter);
        }
        else if (resultClass.getOptional(parser, name, emptyTag))
        {
            resultClass.found(duplicateParameter);
        }
        else if(includeClassOrigin.get(parser, name,  emptyTag))
        {
            includeClassOrigin.found(duplicateParameter);
        }
        else if(propertyList.get(parser, name, emptyTag))
        {
            propertyList.found(duplicateParameter);
        }
        // [IN,OPTIONAL,NULL] string FilterQueryLanguage = NULL,
        else if(filterQueryLanguage.get(parser, name, emptyTag))
        {
            filterQueryLanguage.found(duplicateParameter);
        }
        // [IN,OPTIONAL,NULL] string FilterQuery = NULL,
        else if(filterQuery.get(parser, name, emptyTag))
        {
            filterQuery.found(duplicateParameter);
        }
        // [IN,OPTIONAL] Boolean ContinueOnError = false,
        else if (continueOnError.get(parser, name, emptyTag))
        {
            continueOnError.found(duplicateParameter);
        }
        // [IN,OPTIONAL,NULL] uint32 OperationTimeout = NULL,
        else if (operationTimeout.get(parser, name, emptyTag))
        {
            operationTimeout.found(duplicateParameter);
        }
        // [IN,OPTIONAL] uint32 MaxObjectCount = 0
        else if (maxObjectCount.get(parser, name, emptyTag))
        {
            maxObjectCount.found(duplicateParameter);
        }
        else
        {
            _throwCIMExceptionInvalidIParamName(name);
        }

        // generate exception if endtag error or duplicate attributes
        _checkMissingEndTagOrDuplicateParamValue(
            parser, duplicateParameter, emptyTag);
    }

    objectName.rejectIfNotExist();

    AutoPtr<CIMOpenReferenceInstancesRequestMessage> request(
        new CIMOpenReferenceInstancesRequestMessage(
            messageId,
            nameSpace,
            objectName.value,
            resultClass.value,
            role.value,
            includeClassOrigin.value,
            propertyList.value,
            filterQueryLanguage.value,
            filterQuery.value,
            operationTimeout.value,
            continueOnError.value,
            maxObjectCount.value,
            QueueIdStack(queueId, _returnQueueId)));

    STAT_SERVERSTART

    return request.release();
}


CIMOpenReferenceInstancePathsRequestMessage*
    CIMOperationRequestDecoder::decodeOpenReferenceInstancePathsRequest(
        Uint32 queueId,
        XmlParser& parser,
        const String& messageId,
        const CIMNamespaceName& nameSpace)
{
    STAT_GETSTARTTIME

    objectNameIParam objectName("InstanceName");
    classNameIParam resultClass("ResultClass");
    stringIParam role("role", false);

    stringIParam filterQueryLanguage("FilterQueryLanguage",false);
    stringIParam filterQuery("FilterQuery", false);
    booleanIParam continueOnError("ContinueOnError");
    // [IN,OPTIONAL] uint32 MaxObjectCount = 0
    uint32IParam maxObjectCount("MaxObjectCount", 0);
    uint32ArgIParam operationTimeout("OperationTimeout");

    Boolean duplicateParameter = false;
    Boolean emptyTag;

    for (const char* name;
         XmlReader::getIParamValueTag(parser, name, emptyTag); )
    {
        if(objectName.get(parser, name, emptyTag))
        {
            objectName.found(duplicateParameter);
        }
        else if(role.get(parser, name, emptyTag))
        {
            role.found(duplicateParameter);
        }
        else if (resultClass.getOptional(parser, name, emptyTag))
        {
            resultClass.found(duplicateParameter);
        }
        else if(filterQueryLanguage.get(parser, name, emptyTag))
        {
            filterQueryLanguage.found(duplicateParameter);
        }
        else if(filterQuery.get(parser, name, emptyTag))
        {
            filterQuery.found(duplicateParameter);
        }
        else if (continueOnError.get(parser, name, emptyTag))
        {
            continueOnError.found(duplicateParameter);
        }
        else if (operationTimeout.get(parser, name, emptyTag))
        {
            operationTimeout.found(duplicateParameter);
        }
        else if (maxObjectCount.get(parser, name, emptyTag))
        {
            maxObjectCount.found(duplicateParameter);
        }
        else
        {
            _throwCIMExceptionInvalidIParamName(name);
        }

        // generate exception if endtag error or duplicate attributes
        _checkMissingEndTagOrDuplicateParamValue(
            parser, duplicateParameter, emptyTag);
    }

    objectName.rejectIfNotExist();

    AutoPtr<CIMOpenReferenceInstancePathsRequestMessage> request(
        new CIMOpenReferenceInstancePathsRequestMessage(
            messageId,
            nameSpace,
            objectName.value,
            resultClass.value,
            role.value,
            filterQueryLanguage.value,
            filterQuery.value,
            operationTimeout.value,
            continueOnError.value,
            maxObjectCount.value,
            QueueIdStack(queueId, _returnQueueId)));

    STAT_SERVERSTART

    return request.release();
}

CIMOpenAssociatorInstancesRequestMessage*
    CIMOperationRequestDecoder::decodeOpenAssociatorInstancesRequest(
        Uint32 queueId,
        XmlParser& parser,
        const String& messageId,
        const CIMNamespaceName& nameSpace)
{
    STAT_GETSTARTTIME

    objectNameIParam objectName("InstanceName");
    classNameIParam assocClass("AssocClass");
    classNameIParam resultClass("ResultClass");
    stringIParam resultRole("Resultrole", false);
    stringIParam role("role", false);
    booleanIParam includeClassOrigin("IncludeClassOrigin");
    propertyListIParam propertyList;

    stringIParam filterQueryLanguage("FilterQueryLanguage",false);
    stringIParam filterQuery("FilterQuery", false);
    booleanIParam continueOnError("ContinueOnError");
    // [IN,OPTIONAL] uint32 MaxObjectCount = 0
    uint32IParam maxObjectCount("MaxObjectCount", 0);
    uint32ArgIParam operationTimeout("OperationTimeout");

    Boolean duplicateParameter = false;
    Boolean emptyTag;

    for (const char* name;
         XmlReader::getIParamValueTag(parser, name, emptyTag); )
    {
        if(objectName.get(parser, name, emptyTag))
        {
            objectName.found(duplicateParameter);
        }
        else if (assocClass.getOptional(parser, name, emptyTag))
        {
            assocClass.found(duplicateParameter);
        }
        else if (resultClass.getOptional(parser, name, emptyTag))
        {
            resultClass.found(duplicateParameter);
        }
        else if(role.get(parser, name, emptyTag))
        {
            role.found(duplicateParameter);
        }
        else if(resultRole.get(parser, name, emptyTag))
        {
            resultRole.found(duplicateParameter);
        }
        else if(includeClassOrigin.get(parser, name,  emptyTag))
        {
            includeClassOrigin.found(duplicateParameter);
        }
        else if(propertyList.get(parser, name, emptyTag))
        {
            propertyList.found(duplicateParameter);
        }
        else if(filterQueryLanguage.get(parser, name, emptyTag))
        {
            filterQueryLanguage.found(duplicateParameter);
        }
        else if(filterQuery.get(parser, name, emptyTag))
        {
            filterQuery.found(duplicateParameter);
        }
        else if (continueOnError.get(parser, name, emptyTag))
        {
            continueOnError.found(duplicateParameter);
        }
        else if (operationTimeout.get(parser, name, emptyTag))
        {
            operationTimeout.found(duplicateParameter);
        }
        else if (maxObjectCount.get(parser, name, emptyTag))
        {
            maxObjectCount.found(duplicateParameter);
        }
        else
        {
            _throwCIMExceptionInvalidIParamName(name);
        }

        // generate exception if endtag error or duplicate attributes
        _checkMissingEndTagOrDuplicateParamValue(
            parser, duplicateParameter, emptyTag);
    }

    objectName.rejectIfNotExist();

    AutoPtr<CIMOpenAssociatorInstancesRequestMessage> request(
        new CIMOpenAssociatorInstancesRequestMessage(
            messageId,
            nameSpace,
            objectName.value,
            assocClass.value,
            resultClass.value,
            role.value,
            resultRole.value,
            includeClassOrigin.value,
            propertyList.value,
            filterQueryLanguage.value,
            filterQuery.value,
            operationTimeout.value,
            continueOnError.value,
            maxObjectCount.value,
            QueueIdStack(queueId, _returnQueueId)));

    STAT_SERVERSTART

    return request.release();
}

CIMOpenAssociatorInstancePathsRequestMessage*
    CIMOperationRequestDecoder::decodeOpenAssociatorInstancePathsRequest(
        Uint32 queueId,
        XmlParser& parser,
        const String& messageId,
        const CIMNamespaceName& nameSpace)
{
    STAT_GETSTARTTIME

    objectNameIParam objectName("InstanceName");
    classNameIParam assocClass("AssocClass");
    classNameIParam resultClass("ResultClass");
    stringIParam resultRole("Resultrole", false);
    stringIParam role("role", false);

    stringIParam filterQueryLanguage("FilterQueryLanguage",false);
    stringIParam filterQuery("FilterQuery", false);
    booleanIParam continueOnError("ContinueOnError");
    // [IN,OPTIONAL] uint32 MaxObjectCount = 0
    uint32IParam maxObjectCount("MaxObjectCount", 0);
    uint32ArgIParam operationTimeout("OperationTimeout");

    Boolean duplicateParameter = false;
    Boolean emptyTag;

    for (const char* name;
         XmlReader::getIParamValueTag(parser, name, emptyTag); )
    {
        if(objectName.get(parser, name, emptyTag))
        {
            objectName.found(duplicateParameter);
        }
        else if (assocClass.getOptional(parser, name, emptyTag))
        {
            assocClass.found(duplicateParameter);
        }
        else if (resultClass.getOptional(parser, name, emptyTag))
        {
            resultClass.found(duplicateParameter);
        }
        else if(role.get(parser, name, emptyTag))
        {
            role.found(duplicateParameter);
        }
        else if(resultRole.get(parser, name, emptyTag))
        {
            resultRole.found(duplicateParameter);
        }
        else if(filterQueryLanguage.get(parser, name, emptyTag))
        {
            filterQueryLanguage.found(duplicateParameter);
        }
        else if(filterQuery.get(parser, name, emptyTag))
        {
            filterQuery.found(duplicateParameter);
        }
        else if (continueOnError.get(parser, name, emptyTag))
        {
            continueOnError.found(duplicateParameter);
        }
        else if (operationTimeout.get(parser, name, emptyTag))
        {
            operationTimeout.found(duplicateParameter);
        }
        else if (maxObjectCount.get(parser, name, emptyTag))
        {
            maxObjectCount.found(duplicateParameter);
        }
        else
        {
            _throwCIMExceptionInvalidIParamName(name);
        }

        // generate exception if endtag error or duplicate attributes
        _checkMissingEndTagOrDuplicateParamValue(
            parser, duplicateParameter, emptyTag);
    }

    objectName.rejectIfNotExist();

    AutoPtr<CIMOpenAssociatorInstancePathsRequestMessage> request(
        new CIMOpenAssociatorInstancePathsRequestMessage(
            messageId,
            nameSpace,
            objectName.value,
            assocClass.value,
            resultClass.value,
            role.value,
            resultRole.value,
            filterQueryLanguage.value,
            filterQuery.value,
            operationTimeout.value,
            continueOnError.value,
            maxObjectCount.value,
            QueueIdStack(queueId, _returnQueueId)));

    STAT_SERVERSTART

    return request.release();
}

CIMPullInstancesWithPathRequestMessage*
    CIMOperationRequestDecoder::decodePullInstancesWithPathRequest(
        Uint32 queueId,
        XmlParser& parser,
        const String& messageId,
        const CIMNamespaceName& nameSpace)
{
    STAT_GETSTARTTIME

    // enumerationContext parameter. Value Required.
    //[IN,OUT] <enumerationContext> EnumerationContext,
    stringIParam enumerationContext("EnumerationContext",  true);

    // maxObjectCount Parameter, Value Required. The default value is ignored.
    // [IN] uint32 MaxObjectCount
    uint32IParam maxObjectCount("MaxObjectCount",0, true);

    Boolean duplicateParameter = false;
    Boolean emptyTag;

    for (const char* name;
         XmlReader::getIParamValueTag(parser, name, emptyTag); )
    {
       if(enumerationContext.get(parser, name, emptyTag))
        {
            enumerationContext.found(duplicateParameter);
        }
        else if(maxObjectCount.get(parser, name, emptyTag))
        {
            maxObjectCount.found(duplicateParameter);
        }
        else
        {
            _throwCIMExceptionInvalidIParamName(name);
        }

        // generate exception if endtag error or duplicate attributes
        _checkMissingEndTagOrDuplicateParamValue(
            parser, duplicateParameter, emptyTag);
    }

    // test to be sure required parameters exist.
    enumerationContext.rejectIfNotExist();
    maxObjectCount.rejectIfNotExist();

    AutoPtr<CIMPullInstancesWithPathRequestMessage> request(
        new CIMPullInstancesWithPathRequestMessage(
            messageId,
            nameSpace,
            enumerationContext.value,
            maxObjectCount.value,
            QueueIdStack(queueId, _returnQueueId)));

    STAT_SERVERSTART

    return request.release();
}

CIMPullInstancePathsRequestMessage*
    CIMOperationRequestDecoder::decodePullInstancePathsRequest(
        Uint32 queueId,
        XmlParser& parser,
        const String& messageId,
        const CIMNamespaceName& nameSpace)
{
    STAT_GETSTARTTIME

    // enumerationContext parameter. Value Required.
    //[IN,OUT] <enumerationContext> EnumerationContext,
    stringIParam enumerationContext("EnumerationContext",  true);

    // maxObjectCount Parameter, Value Required. The default value is ignored.
    // [IN] uint32 MaxObjectCount
    uint32IParam maxObjectCount("MaxObjectCount",0, true);

    Boolean duplicateParameter = false;
    Boolean emptyTag;

    for (const char* name;
         XmlReader::getIParamValueTag(parser, name, emptyTag); )
    {
       if(enumerationContext.get(parser, name, emptyTag))
        {
            enumerationContext.found(duplicateParameter);
        }
        else if(maxObjectCount.get(parser, name, emptyTag))
        {
            maxObjectCount.found(duplicateParameter);
        }
        else
        {
            _throwCIMExceptionInvalidIParamName(name);
        }

        // generate exception if endtag error or duplicate attributes
        _checkMissingEndTagOrDuplicateParamValue(
            parser, duplicateParameter, emptyTag);
    }

    // test to be sure required parameters exist.
    enumerationContext.rejectIfNotExist();
    maxObjectCount.rejectIfNotExist();

    AutoPtr<CIMPullInstancePathsRequestMessage> request(
        new CIMPullInstancePathsRequestMessage(
            messageId,
            nameSpace,
            enumerationContext.value,
            maxObjectCount.value,
            QueueIdStack(queueId, _returnQueueId)));

    STAT_SERVERSTART

    return request.release();
}

CIMPullInstancesRequestMessage*
    CIMOperationRequestDecoder::decodePullInstancesRequest(
        Uint32 queueId,
        XmlParser& parser,
        const String& messageId,
        const CIMNamespaceName& nameSpace)
{
    STAT_GETSTARTTIME

    // enumerationContext parameter. Value Required.
    //[IN,OUT] <enumerationContext> EnumerationContext,
    stringIParam enumerationContext("EnumerationContext",  true);
    // maxObjectCount Parameter, Value Required. The default value is ignored.
    // [IN] uint32 MaxObjectCount
    uint32IParam maxObjectCount("MaxObjectCount",0, true);

    Boolean duplicateParameter = false;
    Boolean emptyTag;

    for (const char* name;
         XmlReader::getIParamValueTag(parser, name, emptyTag); )
    {
       if(enumerationContext.get(parser, name, emptyTag))
        {
            enumerationContext.found(duplicateParameter);
        }
        else if(maxObjectCount.get(parser, name, emptyTag))
        {
            maxObjectCount.found(duplicateParameter);
        }
        else
        {
            _throwCIMExceptionInvalidIParamName(name);
        }

        // generate exception if endtag error or duplicate attributes
        _checkMissingEndTagOrDuplicateParamValue(
            parser, duplicateParameter, emptyTag);
    }

    // test to be sure required parameters exist.
    enumerationContext.rejectIfNotExist();
    maxObjectCount.rejectIfNotExist();

    AutoPtr<CIMPullInstancesRequestMessage> request(
        new CIMPullInstancesRequestMessage(
            messageId,
            nameSpace,
            enumerationContext.value,
            maxObjectCount.value,
            QueueIdStack(queueId, _returnQueueId)));

    STAT_SERVERSTART

    return request.release();
}
CIMCloseEnumerationRequestMessage*
    CIMOperationRequestDecoder::decodeCloseEnumerationRequest(
        Uint32 queueId,
        XmlParser& parser,
        const String& messageId,
        const CIMNamespaceName& nameSpace)
{
    STAT_GETSTARTTIME

    String enumerationContext;
    Boolean duplicateParameter = false;
    Boolean gotEnumerationContext = false;

    Boolean emptyTag;
    for (const char* name;
         XmlReader::getIParamValueTag(parser, name, emptyTag); )
    {
        if (System::strcasecmp(name, "EnumerationContext") == 0)
        {
            XmlReader::rejectNullIParamValue(parser, emptyTag, name);
            XmlReader::getStringValueElement(parser, enumerationContext, true);
            duplicateParameter = gotEnumerationContext;
            gotEnumerationContext = true;
        }
        else
        {
            _throwCIMExceptionInvalidIParamName(name);
        }

        // generate exception if endtag error or duplicate attributes
        _checkMissingEndTagOrDuplicateParamValue(
            parser, duplicateParameter, emptyTag);
    }

    if (!gotEnumerationContext)
    {
        _throwCIMExceptionEnumerationContextRequired();
    }

    AutoPtr<CIMCloseEnumerationRequestMessage> request(
        new CIMCloseEnumerationRequestMessage(
            messageId,
            nameSpace,
            enumerationContext,
            QueueIdStack(queueId, _returnQueueId)));

    STAT_SERVERSTART

    return request.release();
}

// NOTE: We did not update this message to use the new classes because
// it is deprecated, and not supported by pegasus.
CIMEnumerationCountRequestMessage*
    CIMOperationRequestDecoder::decodeEnumerationCountRequest(
        Uint32 queueId,
        XmlParser& parser,
        const String& messageId,
        const CIMNamespaceName& nameSpace)
{
    STAT_GETSTARTTIME

    String enumerationContext;
    Boolean duplicateParameter = false;
    Boolean gotEnumerationContext = false;

    Boolean emptyTag;

    for (const char* name;
         XmlReader::getIParamValueTag(parser, name, emptyTag); )
    {
        if (System::strcasecmp(name, "EnumerationContext") == 0)
        {
            XmlReader::rejectNullIParamValue(parser, emptyTag, name);
            XmlReader::getStringValueElement(parser, enumerationContext, true);
            duplicateParameter = gotEnumerationContext;
            gotEnumerationContext = true;
        }
        else
        {
            _throwCIMExceptionInvalidIParamName(name);
        }

        // generate exception if endtag error or duplicate attributes
        _checkMissingEndTagOrDuplicateParamValue(
            parser, duplicateParameter, emptyTag);
    }

    if (!gotEnumerationContext)
    {

        _throwCIMExceptionEnumerationContextRequired();
    }


    AutoPtr<CIMEnumerationCountRequestMessage> request(
        new CIMEnumerationCountRequestMessage(
            messageId,
            nameSpace,
            enumerationContext,
            QueueIdStack(queueId, _returnQueueId)));

    STAT_SERVERSTART

    return request.release();
}

CIMOpenQueryInstancesRequestMessage*
    CIMOperationRequestDecoder::decodeOpenQueryInstancesRequest(
        Uint32 queueId,
        XmlParser& parser,
        const String& messageId,
        const CIMNamespaceName& nameSpace)
{
    STAT_GETSTARTTIME

    stringIParam filterQueryLanguage("FilterQueryLanguage",true);
    stringIParam filterQuery("FilterQuery", true);

    booleanIParam returnQueryResultClass("ReturnQueryResultClass");
    booleanIParam continueOnError("ContinueOnError");
    // [IN,OPTIONAL] uint32 MaxObjectCount = 0
    uint32IParam maxObjectCount("MaxObjectCount", 0);
    // [IN,OPTIONAL,NULL] uint32 OperationTimeout = NULL,
    uint32ArgIParam operationTimeout("OperationTimeout");

    Boolean duplicateParameter = false;
    Boolean emptyTag;

    for (const char* name;
         XmlReader::getIParamValueTag(parser, name, emptyTag); )
    {
        // [IN,OPTIONAL,NULL] string FilterQueryLanguage = NULL,
        if(filterQueryLanguage.get(parser, name, emptyTag))
        {
            filterQueryLanguage.found(duplicateParameter);
        }
        // [IN,OPTIONAL,NULL] string FilterQuery = NULL,
        else if(filterQuery.get(parser, name, emptyTag))
        {
            filterQuery.found(duplicateParameter);
        }
        else if (returnQueryResultClass.get(parser, name, emptyTag))
        {
            returnQueryResultClass.found(duplicateParameter);
        }
        // [IN,OPTIONAL] Boolean ContinueOnError = false,
        else if (continueOnError.get(parser, name, emptyTag))
        {
            continueOnError.found(duplicateParameter);
        }
        // [IN,OPTIONAL,NULL] uint32 OperationTimeout = NULL,
        else if (operationTimeout.get(parser, name, emptyTag))
        {
            operationTimeout.found(duplicateParameter);
        }
        // [IN,OPTIONAL] uint32 MaxObjectCount = 0
        else if (maxObjectCount.get(parser, name, emptyTag))
        {
            maxObjectCount.found(duplicateParameter);
        }
        else
        {
            _throwCIMExceptionInvalidIParamName(name);
        }

        // generate exception if endtag error or duplicate attributes
        _checkMissingEndTagOrDuplicateParamValue(
            parser, duplicateParameter, emptyTag);
    }

    _testRequiredParametersExist(filterQuery.value,filterQuery.got);
    _testRequiredParametersExist(filterQuery.value,filterQueryLanguage.got);

    AutoPtr<CIMOpenQueryInstancesRequestMessage> request(
        new CIMOpenQueryInstancesRequestMessage(
            messageId,
            nameSpace,
            filterQueryLanguage.value,
            filterQuery.value,
            returnQueryResultClass.value,
            operationTimeout.value,
            continueOnError.value,
            maxObjectCount.value,
            QueueIdStack(queueId, _returnQueueId)));

    STAT_SERVERSTART

    return request.release();
}
//EXP_PULL_END

void CIMOperationRequestDecoder::setServerTerminating(Boolean flag)
{
    _serverTerminating = flag;
}

PEGASUS_NAMESPACE_END
