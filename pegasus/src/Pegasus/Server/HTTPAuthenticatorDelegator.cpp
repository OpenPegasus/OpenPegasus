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

#include <Pegasus/Common/AuditLogger.h>
#include <Pegasus/Common/Constants.h>
#include <Pegasus/Common/XmlWriter.h>
#include <Pegasus/Common/Thread.h>
#include <Pegasus/Common/MessageLoader.h>
#include <Pegasus/Common/FileSystem.h>
#include <Pegasus/Common/LanguageParser.h>
#include <Pegasus/Config/ConfigManager.h>
#include "HTTPAuthenticatorDelegator.h"

#ifdef PEGASUS_ZOS_SECURITY
// This include file will not be provided in the OpenGroup CVS for now.
// Do NOT try to include it in your compile
# include <Pegasus/Common/safCheckzOS_inline.h>
#endif

#ifdef PEGASUS_OS_ZOS
# include <sys/ps.h>
#endif

PEGASUS_USING_STD;

PEGASUS_NAMESPACE_BEGIN

static const String _HTTP_VERSION_1_0 = "HTTP/1.0";

static const String _HTTP_METHOD_MPOST = "M-POST";
static const String _HTTP_METHOD = "POST";
#ifdef PEGASUS_ENABLE_PROTOCOL_WEB
static const String _HTTP_METHOD_GET = "GET";
static const String _HTTP_METHOD_HEAD = "HEAD";
#endif /* #ifdef PEGASUS_ENABLE_PROTOCOL_WEB */
static const char* _HTTP_HEADER_CIMEXPORT = "CIMExport";
static const char* _HTTP_HEADER_CONNECTION = "Connection";
static const char* _HTTP_HEADER_CIMOPERATION = "CIMOperation";
static const char* _HTTP_HEADER_ACCEPT_LANGUAGE = "Accept-Language";
static const char* _HTTP_HEADER_CONTENT_LANGUAGE = "Content-Language";
static const char* _HTTP_HEADER_AUTHORIZATION = "Authorization";
static const char* _HTTP_HEADER_PEGASUSAUTHORIZATION = "PegasusAuthorization";

static const String _CONFIG_PARAM_ENABLEAUTHENTICATION = "enableAuthentication";

static const char _COOKIE_NAME[] = "PEGASUS_SID";

HTTPAuthenticatorDelegator::HTTPAuthenticatorDelegator(
    Uint32 cimOperationMessageQueueId,
    Uint32 cimExportMessageQueueId,
    CIMRepository* repository)
    : Base(PEGASUS_QUEUENAME_HTTPAUTHDELEGATOR),
      _cimOperationMessageQueueId(cimOperationMessageQueueId),
      _cimExportMessageQueueId(cimExportMessageQueueId),
      _wsmanOperationMessageQueueId(PEG_NOT_FOUND),
      _rsOperationMessageQueueId(PEG_NOT_FOUND),
#ifdef PEGASUS_ENABLE_PROTOCOL_WEB
      _webOperationMessageQueueId(PEG_NOT_FOUND),
#endif
      _repository(repository)
{
    PEG_METHOD_ENTER(TRC_HTTP,
        "HTTPAuthenticatorDelegator::HTTPAuthenticatorDelegator");

    _authenticationManager.reset(new AuthenticationManager());
#ifdef PEGASUS_ENABLE_SESSION_COOKIES
    _sessions.reset(new HTTPSessionList());
#endif

    PEG_METHOD_EXIT();
}

HTTPAuthenticatorDelegator::~HTTPAuthenticatorDelegator()
{
    PEG_METHOD_ENTER(TRC_HTTP,
        "HTTPAuthenticatorDelegator::~HTTPAuthenticatorDelegator");

    PEG_METHOD_EXIT();
}

void HTTPAuthenticatorDelegator::enqueue(Message* message)
{
    handleEnqueue(message);
}

void HTTPAuthenticatorDelegator::_sendResponse(
    Uint32 queueId,
    Buffer& message,
    Boolean closeConnect)
{
    PEG_METHOD_ENTER(TRC_HTTP,
        "HTTPAuthenticatorDelegator::_sendResponse");

    MessageQueue* queue = MessageQueue::lookup(queueId);

    if (queue)
    {
        HTTPMessage* httpMessage = new HTTPMessage(message);
        httpMessage->dest = queue->getQueueId();

        httpMessage->setCloseConnect(closeConnect);

        queue->enqueue(httpMessage);
    }

    PEG_METHOD_EXIT();
}


void HTTPAuthenticatorDelegator::_sendChallenge(
    Uint32 queueId,
    const String& errorDetail,
    const String& authResponse,
    Boolean closeConnect)
{
    PEG_METHOD_ENTER(TRC_HTTP,
        "HTTPAuthenticatorDelegator::_sendChallenge");

    //
    // build unauthorized (401) response message
    //

    Buffer message;
    XmlWriter::appendUnauthorizedResponseHeader(
        message,
        errorDetail,
        authResponse);

    _sendResponse(queueId, message,closeConnect);

    PEG_METHOD_EXIT();
}


void HTTPAuthenticatorDelegator::_sendHttpError(
    Uint32 queueId,
    const String& status,
    const String& cimError,
    const String& pegasusError,
    Boolean closeConnect)
{
    PEG_METHOD_ENTER(TRC_HTTP,
        "HTTPAuthenticatorDelegator::_sendHttpError");

    //
    // build error response message
    //

    Buffer message;
    message = XmlWriter::formatHttpErrorRspMessage(
        status,
        cimError,
        pegasusError);

    _sendResponse(queueId, message,closeConnect);

    PEG_METHOD_EXIT();
}


void HTTPAuthenticatorDelegator::handleEnqueue(Message *message)
{
    PEG_METHOD_ENTER(TRC_HTTP,
        "HTTPAuthenticatorDelegator::handleEnqueue");

    if (!message)
    {
        PEG_METHOD_EXIT();
        return;
    }

    // Flag indicating whether the message should be deleted after handling.
    // This should be set to false by handleHTTPMessage when the message is
    // passed as is to another queue.
    Boolean deleteMessage = true;

    try
    {
        if (message->getType() == HTTP_MESSAGE)
        {
            handleHTTPMessage((HTTPMessage*)message, deleteMessage);
        }
    }
    catch (...)
    {
        if (deleteMessage)
        {
            PEG_TRACE_CSTRING(TRC_HTTP, Tracer::LEVEL4,
                    "Exception caught, deleting Message in "
                    "HTTPAuthenticator::handleEnqueue");

            delete message;
        }
        throw;
    }

    if (deleteMessage)
    {
        PEG_TRACE_CSTRING(TRC_HTTP, Tracer::LEVEL4,
                    "Deleting Message in HTTPAuthenticator::handleEnqueue");

        delete message;
    }

    PEG_METHOD_EXIT();
}

void HTTPAuthenticatorDelegator::handleEnqueue()
{
    PEG_METHOD_ENTER(TRC_HTTP,
        "HTTPAuthenticatorDelegator::handleEnqueue");

    Message* message = dequeue();
    if (message)
    {
        handleEnqueue(message);
    }

    PEG_METHOD_EXIT();
}

void HTTPAuthenticatorDelegator::handleHTTPMessage(
    HTTPMessage* httpMessage,
    Boolean& deleteMessage)
{
    PEG_METHOD_ENTER(TRC_HTTP,
        "HTTPAuthenticatorDelegator::handleHTTPMessage");

    PEGASUS_ASSERT(httpMessage->message.size() != 0);

    deleteMessage = true;

    //
    // Save queueId:
    //
    Uint32 queueId = httpMessage->queueId;

    //
    // Parse the HTTP message:
    //
    String startLine;
    Array<HTTPHeader> headers;
    Uint32 contentLength;
    Boolean closeConnect = false;

    //
    // Process M-POST and POST messages:
    //

    PEG_TRACE_CSTRING(
        TRC_HTTP,
        Tracer::LEVEL3,
        "HTTPAuthenticatorDelegator - HTTP processing start");

    // parse the received HTTPMessage
    // parse function will return false if more than PEGASUS_MAXELEMENTS_NUM
    // headers were detected in the message
    if (!httpMessage->parse(startLine, headers, contentLength))
    {
        throw TooManyHTTPHeadersException();
    }

    //
    // Check for Connection: Close
    //
    const char* connectClose;
    if (HTTPMessage::lookupHeader(
            headers, _HTTP_HEADER_CONNECTION, connectClose, false))
    {
        if (System::strcasecmp(connectClose, "Close") == 0)
        {
            PEG_TRACE_CSTRING(TRC_HTTP, Tracer::LEVEL3,
                "Header in HTTP Message Contains a Connection: Close");
            closeConnect = true;
            httpMessage->setCloseConnect(closeConnect);
        }
    }

    //
    // Check and set languages
    //
    AcceptLanguageList acceptLanguages;
    ContentLanguageList contentLanguages;
    try
    {
        // Get and validate the Accept-Language header, if set
        String acceptLanguageHeader;
        if (HTTPMessage::lookupHeader(
                headers,
                _HTTP_HEADER_ACCEPT_LANGUAGE,
                acceptLanguageHeader,
                false))
        {
            acceptLanguages = LanguageParser::parseAcceptLanguageHeader(
                acceptLanguageHeader);
            httpMessage->acceptLanguagesDecoded = true;
        }

        // Get and validate the Content-Language header, if set
        String contentLanguageHeader;
        if (HTTPMessage::lookupHeader(
                headers,
                _HTTP_HEADER_CONTENT_LANGUAGE,
                contentLanguageHeader,
                false))
        {
            contentLanguages = LanguageParser::parseContentLanguageHeader(
                contentLanguageHeader);
            httpMessage->contentLanguagesDecoded = true;
        }
    }
    catch (Exception& e)
    {
        // clear any existing languages to force messages to come from the
        // root bundle
        Thread::clearLanguages();
        MessageLoaderParms msgParms(
            "Pegasus.Server.HTTPAuthenticatorDelegator.REQUEST_NOT_VALID",
            "request-not-valid");
        String msg(MessageLoader::getMessage(msgParms));

        _sendHttpError(
            queueId,
            HTTP_STATUS_BADREQUEST,
            msg,
            e.getMessage(),
            closeConnect);
        PEG_METHOD_EXIT();
        return;
    }

    Thread::setLanguages(acceptLanguages);
    httpMessage->acceptLanguages = acceptLanguages;
    httpMessage->contentLanguages = contentLanguages;

    //
    // Parse the request line:
    //
    String methodName;
    String requestUri;
    String httpVersion;

    HTTPMessage::parseRequestLine(
        startLine, methodName, requestUri, httpVersion);

    //
    //  Set HTTP method for the request
    //
    HttpMethod httpMethod = HTTP_METHOD__POST;
    if (methodName == _HTTP_METHOD_MPOST)
    {
        httpMethod = HTTP_METHOD_M_POST;
    }
#ifdef PEGASUS_ENABLE_PROTOCOL_WEB
    else if (methodName == _HTTP_METHOD_GET)
    {
        httpMethod = HTTP_METHOD_GET;
    }
    else if (methodName == _HTTP_METHOD_HEAD)
    {
        httpMethod = HTTP_METHOD_HEAD;
    }
#endif

    if (httpMethod != HTTP_METHOD__POST && httpMethod != HTTP_METHOD_M_POST
#ifdef PEGASUS_ENABLE_PROTOCOL_WEB
        && httpMethod != HTTP_METHOD_GET && httpMethod != HTTP_METHOD_HEAD
#endif
       )
    {
        //
        //  M-POST method is not valid with version 1.0
        //
        _sendHttpError(
            queueId,
            HTTP_STATUS_NOTIMPLEMENTED,
            String::EMPTY,
            String::EMPTY,
            closeConnect);
        PEG_METHOD_EXIT();
        return;
    }
    
    if ((httpMethod == HTTP_METHOD_M_POST) &&
             (httpVersion == _HTTP_VERSION_1_0))
    {
        //
        //  M-POST method is not valid with version 1.0
        //
        _sendHttpError(
            queueId,
            HTTP_STATUS_BADREQUEST,
            String::EMPTY,
            String::EMPTY,
            closeConnect);

        PEG_METHOD_EXIT();
        return;
    }

    PEG_TRACE_CSTRING(
        TRC_AUTHENTICATION,
        Tracer::LEVEL3,
        "HTTPAuthenticatorDelegator - Authentication processing start");

    //
    // Handle authentication:
    //
    ConfigManager* configManager = ConfigManager::getInstance();
    Boolean enableAuthentication =
        ConfigManager::parseBooleanValue(configManager->getCurrentValue(
            _CONFIG_PARAM_ENABLEAUTHENTICATION));

    AuthenticationStatus authStatus(AUTHSC_UNAUTHORIZED);
    if (httpMessage->authInfo->isConnectionAuthenticated())
    {
        authStatus = AuthenticationStatus(AUTHSC_SUCCESS);
    }

    if (enableAuthentication)
    {

#ifdef PEGASUS_ENABLE_SESSION_COOKIES
        // Find cookie and check if we know it
        String cookieHdr;
        if (!authStatus.isSuccess() && _sessions->cookiesEnabled()
                    && httpMessage->lookupHeader(headers, "Cookie", cookieHdr))
        {
            String userName;
            String cookie;
            if (httpMessage->parseCookieHeader(cookieHdr, _COOKIE_NAME, cookie)
                        && _sessions->isAuthenticated(cookie,
                                    httpMessage->ipAddress, userName))
            {
                authStatus = AuthenticationStatus(AUTHSC_SUCCESS);
                httpMessage->authInfo->setAuthenticatedUser(userName);
                httpMessage->authInfo->setAuthType(
                        AuthenticationInfoRep::AUTH_TYPE_COOKIE);
            }
        }
#endif

        if (authStatus.isSuccess())
        {
            if (httpMessage->authInfo->getAuthType()==
                    AuthenticationInfoRep::AUTH_TYPE_SSL)
            {
                // Get the user name associated with the certificate (using the
                // certificate chain, if necessary).

                String certUserName;
                String issuerName;
                String subjectName;
                char serialNumber[32];

                PEG_TRACE_CSTRING(TRC_HTTP, Tracer::LEVEL3,
                    "Client was authenticated via trusted SSL certificate.");

                String trustStore =
                    configManager->getCurrentValue("sslTrustStore");

                if (FileSystem::isDirectory(
                        ConfigManager::getHomedPath(trustStore)))
                {
                    PEG_TRACE_CSTRING(TRC_HTTP, Tracer::LEVEL4,
                        "Truststore is a directory, lookup username");

                    // Get the client certificate chain to determine the
                    // correct username mapping.  Starting with the peer
                    // certificate, work your way up the chain towards the
                    // root certificate until a match is found in the
                    // repository.
                    Array<SSLCertificateInfo*> clientCertificateChain =
                        httpMessage->authInfo->getClientCertificateChain();
                    SSLCertificateInfo* clientCertificate = NULL;

                    PEG_TRACE((TRC_HTTP, Tracer::LEVEL4,
                        "Client certificate chain length: %d.",
                        clientCertificateChain.size()));

                    Uint32 loopCount = clientCertificateChain.size() - 1;
                    for (Uint32 i = 0; i <= loopCount ; i++)
                    {
                        clientCertificate = clientCertificateChain[i];
                        if (clientCertificate == NULL)
                        {
                            MessageLoaderParms msgParms(
                                "Pegasus.Server.HTTPAuthenticatorDelegator."
                                    "BAD_CERTIFICATE",
                                "The certificate used for authentication is "
                                    "not valid.");
                            _sendHttpError(
                                queueId,
                                HTTP_STATUS_UNAUTHORIZED,
                                String::EMPTY,
                                MessageLoader::getMessage(msgParms),
                                closeConnect);
                            PEG_METHOD_EXIT();
                            return;
                        }
                        PEG_TRACE((TRC_HTTP, Tracer::LEVEL4,
                            "Certificate toString %s",
                            (const char*)
                                clientCertificate->toString().getCString()));

                        // Get certificate properties
                        issuerName = clientCertificate->getIssuerName();
                        sprintf(serialNumber, "%lu",
                            (unsigned long)
                                clientCertificate->getSerialNumber());
                        subjectName = clientCertificate->getSubjectName();

                        //
                        // The truststore type key property is deprecated. To
                        // retain backward compatibility, add the truststore
                        // type property to the key bindings and set it to
                        // cimserver truststore.
                        //

                        // Construct the corresponding PG_SSLCertificate
                        // instance
                        Array<CIMKeyBinding> keyBindings;
                        keyBindings.append(CIMKeyBinding(
                            "IssuerName", issuerName, CIMKeyBinding::STRING));
                        keyBindings.append(CIMKeyBinding(
                            "SerialNumber",
                            serialNumber,
                            CIMKeyBinding::STRING));
                        keyBindings.append(CIMKeyBinding("TruststoreType",
                            PG_SSLCERTIFICATE_TSTYPE_VALUE_SERVER));

                        CIMObjectPath cimObjectPath(
                            "localhost",
                            PEGASUS_NAMESPACENAME_CERTIFICATE,
                            PEGASUS_CLASSNAME_CERTIFICATE,
                            keyBindings);

                        PEG_TRACE((TRC_HTTP, Tracer::LEVEL4,
                            "Client Certificate COP: %s",
                            (const char*)
                                cimObjectPath.toString().getCString()));

                        CIMInstance cimInstance;
                        CIMValue value;
                        Uint32 pos;
                        String userName;

                        // Attempt to get the username registered to the
                        // certificate
                        try
                        {
                            cimInstance = _repository->getInstance(
                                PEGASUS_NAMESPACENAME_CERTIFICATE,
                                cimObjectPath);

                            pos =
                                cimInstance.findProperty("RegisteredUserName");

                            if (pos != PEG_NOT_FOUND &&
                                !(value = cimInstance.getProperty(pos).
                                      getValue()).isNull())
                            {
                                value.get(userName);

                                //
                                // If a user name is specified, our search is
                                // complete
                                //
                                if (userName.size())
                                {
                                    PEG_TRACE((TRC_HTTP, Tracer::LEVEL3,
                                        "User name for certificate is %s",
                                        (const char*)userName.getCString()));
                                    certUserName = userName;
                                    break;
                                }

                                // No user name is specified; continue up the
                                // chain
                                PEG_TRACE((TRC_HTTP, Tracer::LEVEL4,
                                    "The certificate at level %u has no "
                                        "associated username; moving up the "
                                        "chain",
                                    i));
                            }
                            else
                            {
                                PEG_TRACE_CSTRING(
                                    TRC_HTTP,
                                    Tracer::LEVEL3,
                                    "HTTPAuthenticatorDelegator - Bailing, no "
                                        "username is registered to this "
                                        "certificate.");
                            }
                        }
                        catch (CIMException& e)
                        {
                            // this certificate did not have a registration
                            // associated with it; continue up the chain
                            if (e.getCode() == CIM_ERR_NOT_FOUND)
                            {
                                PEG_TRACE_CSTRING(TRC_HTTP, Tracer::LEVEL4,
                                    "No registration for this certificate; "
                                        "try next certificate in chain");
                                continue;
                            }
                            else
                            {
                                PEG_TRACE((TRC_HTTP,Tracer::LEVEL1,
                                    "HTTPAuthenticatorDelegator- Bailing,the "
                                        "certificate used for authentication "
                                        "is not valid for client IP address "
                                        "%s.",
                                    (const char*)
                                        httpMessage->ipAddress.getCString())
                                    );

                                MessageLoaderParms msgParms(
                                    "Pegasus.Server.HTTPAuthenticatorDelegator."
                                        "BAD_CERTIFICATE",
                                    "The certificate used for authentication "
                                        "is not valid.");
                                String msg(MessageLoader::getMessage(msgParms));
                                _sendHttpError(
                                    queueId,
                                    HTTP_STATUS_UNAUTHORIZED,
                                    String::EMPTY,
                                    msg,
                                    closeConnect);
                                PEG_METHOD_EXIT();
                                return;
                            }
                        }
                        catch (...)
                        {
                            // This scenario can occur if a certificate cached
                            // on the server was deleted openssl would not pick
                            // up the deletion but we would pick it up here
                            // when we went to look it up in the repository

                            PEG_TRACE((TRC_HTTP,Tracer::LEVEL1,
                                "HTTPAuthenticatorDelegator- Bailing,the "
                                    "certificate used for authentication is "
                                    "not valid for client IP address %s.",
                                (const char*)
                                    httpMessage->ipAddress.getCString()));
                            
                            MessageLoaderParms msgParms(
                                "Pegasus.Server.HTTPAuthenticatorDelegator."
                                    "BAD_CERTIFICATE",
                                "The certificate used for authentication is "
                                    "not valid.");
                            String msg(MessageLoader::getMessage(msgParms));
                            _sendHttpError(
                                queueId,
                                HTTP_STATUS_UNAUTHORIZED,
                                String::EMPTY,
                                msg,
                                closeConnect);
                            PEG_METHOD_EXIT();
                            return;
                        }
                    } //end for clientcertificatechain
                } //end sslTrustStore directory
                else
                {
                    // trustStore is a single CA file, lookup username
                    // user was already verified as a valid system user during
                    // server startup
                    certUserName =
                        configManager->getCurrentValue("sslTrustStoreUserName");
                }

                //
                // Validate user information
                //

                if (certUserName == String::EMPTY)
                {
                    PEG_TRACE((TRC_HTTP,Tracer::LEVEL1,
                        "HTTPAuthenticatorDelegator-No username is registered "
                            "to this certificate for client IP address %s.",
                        (const char*)httpMessage->ipAddress.getCString()));

                    MessageLoaderParms msgParms(
                        "Pegasus.Server.HTTPAuthenticatorDelegator."
                            "BAD_CERTIFICATE_USERNAME",
                        "No username is registered to this certificate.");
                    _sendHttpError(
                        queueId,
                        HTTP_STATUS_UNAUTHORIZED,
                        String::EMPTY,
                        MessageLoader::getMessage(msgParms),
                        closeConnect);
                    PEG_METHOD_EXIT();
                    return;
                }

                authStatus =
                    _authenticationManager->validateUserForHttpAuth(
                        certUserName,
                        httpMessage->authInfo);

                if (!authStatus.isSuccess())
                {
                    PEG_AUDIT_LOG(logCertificateBasedUserValidation(
                        certUserName,
                        issuerName,
                        subjectName,
                        serialNumber,
                        httpMessage->ipAddress,
                        false));
                    MessageLoaderParms msgParms(
                        "Pegasus.Server.HTTPAuthenticatorDelegator."
                            "CERTIFICATE_USER_NOT_VALID",
                        "User '$0' registered to this certificate is not a "
                            "valid user.",
                        certUserName);

                    _sendHttpError(
                        queueId,
                        HTTP_STATUS_UNAUTHORIZED,
                        String::EMPTY,
                        MessageLoader::getMessage(msgParms),
                        closeConnect);
                    PEG_METHOD_EXIT();
                    return;
                }

                PEG_AUDIT_LOG(logCertificateBasedUserValidation(
                    certUserName,
                    issuerName,
                    subjectName,
                    serialNumber,
                    httpMessage->ipAddress,
                    true));

                httpMessage->authInfo->setAuthenticatedUser(certUserName);

                PEG_TRACE((
                    TRC_HTTP,
                    Tracer::LEVEL4,
                    "HTTPAuthenticatorDelegator - The trusted client "
                        "certificate is registered to %s.",
                    (const char*) certUserName.getCString()));
            } // end AuthenticationInfoRep::AUTH_TYPE_SSL

#ifdef PEGASUS_OS_ZOS
            if (httpMessage->authInfo->getAuthType()==
                    AuthenticationInfoRep::AUTH_TYPE_ZOS_ATTLS)
            {
                String connectionUserName =
                    httpMessage->authInfo->getConnectionUser();

                // If authenticated user not the connected user
                // then check CIMSERV profile.
                if (!String::equalNoCase(connectionUserName,
                        httpMessage->authInfo->getAuthenticatedUser()))
                {

#ifdef PEGASUS_ZOS_SECURITY
                   if ( !CheckProfileCIMSERVclassWBEM(connectionUserName,
                             __READ_RESOURCE))
                   {
                       Logger::put_l(Logger::STANDARD_LOG, ZOS_SECURITY_NAME,
                           Logger::WARNING,
                           MessageLoaderParms(
                               "Pegasus.Server.HTTPAuthenticatorDelegator."
                                   "ATTLS_NOREAD_CIMSERV_ACCESS.PEGASUS_OS_ZOS",
                               "Request UserID $0 doesn't have READ permission"
                               " to profile CIMSERV CL(WBEM).",
                               connectionUserName));

                       PEG_AUDIT_LOG(logCertificateBasedUserValidation(
                           connectionUserName,
                           String::EMPTY,
                           String::EMPTY,
                           String::EMPTY,
                           httpMessage->ipAddress,
                           false));

                       _sendHttpError(
                           queueId,
                           HTTP_STATUS_UNAUTHORIZED,
                           String::EMPTY,
                           String::EMPTY,
                           closeConnect);

                       PEG_METHOD_EXIT();
                       return;
                   }
#endif
                   PEG_TRACE((TRC_HTTP, Tracer::LEVEL4,
                       "Client UserID '%s' was authenticated via AT-TLS.",
                       (const char*)connectionUserName.getCString()));

                   httpMessage->authInfo->setAuthenticatedUser(
                       connectionUserName);

                   // For audit loging, only the mapping of the client IP to
                   // the resolved user ID is from interest.
                   // The SAF facility logs the certificate validation and
                   // the mapping of certificate subject to a local userID.
                   PEG_AUDIT_LOG(logCertificateBasedUserValidation(
                                    connectionUserName,
                                    String::EMPTY,
                                    String::EMPTY,
                                    String::EMPTY,
                                    httpMessage->ipAddress,
                                    true));

                }// end is authenticated ?

            } // end AuthenticationInfoRep::AUTH_TYPE_ZOS_ATTLS

            if (httpMessage->authInfo->getAuthType()==
                    AuthenticationInfoRep::AUTH_TYPE_ZOS_LOCAL_DOMIAN_SOCKET)
            {
                String connectionUserName =
                    httpMessage->authInfo->getConnectionUser();

                String requestUserName;
                String authHeader;
                String authHttpType;
                String cookie;

                // if lookupHeader() is not successfull parseLocalAuthHeader()
                // must not be called !!
                if ( HTTPMessage::lookupHeader(headers,
                        _HTTP_HEADER_PEGASUSAUTHORIZATION, authHeader, false)&&
                     HTTPMessage::parseLocalAuthHeader(authHeader,
                         authHttpType, requestUserName, cookie))
                {
                    String cimServerUserName = System::getEffectiveUserName();

                    PEG_TRACE((TRC_HTTP, Tracer::LEVEL4,
                               "CIM server UserID = '%s', "
                               "Request UserID = '%s', "
                               "Local authenticated UserID = '%s'.",
                               (const char*) cimServerUserName.getCString(),
                               (const char*) requestUserName.getCString(),
                               (const char*) connectionUserName.getCString()
                             ));

                    // if the request name and the user connected to the socket
                    // are the same, or if the currnet user running the
                    // cim server and the connected user are the same then
                    // assign the request user id as authenticated user id.
                    if( String::equalNoCase(
                            requestUserName,connectionUserName) ||
                        String::equalNoCase(
                            cimServerUserName,connectionUserName))
                    {
                        // If the designate new authenticated user, the user of
                        // the request, is not already the authenticated user
                        // then set the authenticated user and check CIMSERV.
                        if (!String::equalNoCase(requestUserName,
                             httpMessage->authInfo->getAuthenticatedUser()))
                        {

#ifdef PEGASUS_ZOS_SECURITY
                           if ( !CheckProfileCIMSERVclassWBEM(requestUserName,
                                     __READ_RESOURCE))
                           {
                               Logger::put_l(Logger::STANDARD_LOG,
                                   ZOS_SECURITY_NAME,
                                   Logger::WARNING,
                                   MessageLoaderParms(
                                   "Pegasus.Server.HTTPAuthenticatorDelegator."
                                       "UNIXSOCKET_NOREAD_CIMSERV_ACCESS."
                                       "PEGASUS_OS_ZOS",
                                   "Request UserID $0 doesn't have READ "
                                       "permission to profile "
                                       "CIMSERV CL(WBEM).",
                                   requestUserName));

                               _sendHttpError(
                                   queueId,
                                   HTTP_STATUS_UNAUTHORIZED,
                                   String::EMPTY,
                                   String::EMPTY,
                                   closeConnect);

                               PEG_METHOD_EXIT();
                               return;
                           }
#endif
                           // It is not necessary to check remote privileged
                           // user access for local connections;
                           // set the flag to "check done"
                           httpMessage->authInfo->
                               setRemotePrivilegedUserAccessChecked();

                           httpMessage->authInfo->setAuthenticatedUser(
                               requestUserName);

                           PEG_TRACE((TRC_HTTP, Tracer::LEVEL4,
                                "New authenticated User = '%s'.",
                                (const char*)requestUserName.getCString()
                                ));

                        } // end changed authenticated user

                    PEG_TRACE((TRC_HTTP, Tracer::LEVEL4,
                         "User authenticated for request = '%s'.",
                         (const char*)httpMessage->authInfo->
                               getAuthenticatedUser().getCString()
                         ));

                        // Write local authentication audit record.
                        PEG_AUDIT_LOG(logLocalAuthentication(
                             requestUserName,true));

                    } // end select authenticated user
                    else
                    {
                        PEG_AUDIT_LOG(logLocalAuthentication(
                             requestUserName,false));

                        PEG_TRACE((TRC_HTTP, Tracer::LEVEL4,
                             "User '%s' not authorized for request",
                             (const char*)requestUserName.getCString()));

                        _sendHttpError(
                            queueId,
                            HTTP_STATUS_UNAUTHORIZED,
                            String::EMPTY,
                            String::EMPTY,
                            closeConnect);

                        PEG_METHOD_EXIT();
                        return;
                    }
                } // end lookup header
                else
                {
                    MessageLoaderParms msgParms(
                        "Pegasus.Server.HTTPAuthenticatorDelegator."
                            "AUTHORIZATION_HEADER_ERROR",
                        "Authorization header error");
                    String msg(MessageLoader::getMessage(msgParms));
                    _sendHttpError(
                        queueId,
                        HTTP_STATUS_BADREQUEST,
                        String::EMPTY,
                        msg,
                        closeConnect);

                    PEG_METHOD_EXIT();
                    return;
                }
            } // end AuthenticationInfoRep::AUTH_TYPE_ZOS_LOCAL_DOMIAN_SOCKET
#endif
        } // end isRequestAuthenticated
        else
        { // !isRequestAuthenticated

            String authorization;

            //
            // do Local/Pegasus authentication
            //
            if (HTTPMessage::lookupHeader(headers,
                    _HTTP_HEADER_PEGASUSAUTHORIZATION, authorization, false))
            {
                try
                {
                    //
                    // Do pegasus/local authentication
                    //
                    authStatus =
                        _authenticationManager->performPegasusAuthentication(
                            authorization,
                            httpMessage->authInfo);

                    if (authStatus.isSuccess())
                    {
#ifdef PEGASUS_ENABLE_SESSION_COOKIES
                        _createCookie(httpMessage);
#endif
                    }
                    else
                    {
                        String authResp;

                        authResp = _authenticationManager->
                            getPegasusAuthResponseHeader(
                                authorization,
                                httpMessage->authInfo);

                        if (authResp.size() > 0)
                        {
                            if (authStatus.doChallenge())
                            {
                                _sendChallenge(
                                    queueId,
                                    authStatus.getErrorDetail(),
                                    authResp,
                                    closeConnect);
                            }
                            else
                            {
                                _sendHttpError(
                                    queueId,
                                    authStatus.getHttpStatus(),
                                    String::EMPTY,
                                    authStatus.getErrorDetail(),
                                    closeConnect);
                            }
                        }
                        else
                        {
                            MessageLoaderParms msgParms(
                                "Pegasus.Server.HTTPAuthenticatorDelegator."
                                    "AUTHORIZATION_HEADER_ERROR",
                                "Authorization header error");
                            String msg(MessageLoader::getMessage(msgParms));
                            _sendHttpError(
                                queueId,
                                HTTP_STATUS_BADREQUEST,
                                String::EMPTY,
                                msg,
                                closeConnect);
                        }

                        PEG_METHOD_EXIT();
                        return;
                    }
                }
                catch (const CannotOpenFile&)
                {
                    _sendHttpError(
                        queueId,
                        HTTP_STATUS_INTERNALSERVERERROR,
                        String::EMPTY,
                        String::EMPTY,
                        closeConnect);
                    PEG_METHOD_EXIT();
                    return;
                }
            } // end PEGASUS/LOCAL authentication

            //
            // do HTTP authentication
            //
            if (HTTPMessage::lookupHeader(
                    headers, _HTTP_HEADER_AUTHORIZATION,
                    authorization, false))
            {
                authStatus =
                    _authenticationManager->performHttpAuthentication(
                        authorization,
                        httpMessage->authInfo);

#ifdef PEGASUS_PAM_SESSION_SECURITY
                if (authStatus.isPasswordExpired())
                {
                    // if this is CIM-XML and Password Expired treat as success
                    // expired password state is already stored in
                    // AuthenticationInfo
                    const char* cimOperation;

                    if (HTTPMessage::lookupHeader(
                        headers,
                        _HTTP_HEADER_CIMOPERATION,
                        cimOperation,
                        true))
                    {
                        authStatus = AuthenticationStatus(true);
                    }                    
                }
#endif
                if (authStatus.isSuccess())
                {
#ifdef PEGASUS_ENABLE_SESSION_COOKIES
                    _createCookie(httpMessage);
#endif
                }
                else
                {
                    //ATTN: the number of challenges get sent for a
                    //      request on a connection can be pre-set.
#ifdef PEGASUS_NEGOTIATE_AUTHENTICATION
                    // Kerberos authentication needs access to the
                    // AuthenticationInfo object for this session in
                    // order to set up the reference to the
                    // CIMKerberosSecurityAssociation object for this
                    // session.

                    String authResp =
                        _authenticationManager->getHttpAuthResponseHeader(
                            httpMessage->authInfo);
#else
                    String authResp =
                        _authenticationManager->getHttpAuthResponseHeader();
#endif
                    if (authResp.size() > 0)
                    {
                        if (authStatus.doChallenge())
                        {
                            _sendChallenge(
                                queueId,
                                authStatus.getErrorDetail(),
                                authResp,
                                closeConnect);
                        }
                        else
                        {
                            _sendHttpError(
                                queueId,
                                authStatus.getHttpStatus(),
                                String::EMPTY,
                                authStatus.getErrorDetail(),
                                closeConnect);
                        }
                    }
                    else
                    {
                        MessageLoaderParms msgParms(
                            "Pegasus.Server.HTTPAuthenticatorDelegator."
                                "AUTHORIZATION_HEADER_ERROR",
                            "Authorization header error");
                        String msg(MessageLoader::getMessage(msgParms));
                        _sendHttpError(
                            queueId,
                            HTTP_STATUS_BADREQUEST,
                            String::EMPTY,
                            msg,
                            closeConnect);
                    }

                    PEG_METHOD_EXIT();
                    return;
                }
            }  // End if HTTP Authorization
        } //end if (!isRequestAuthenticated)

    } //end enableAuthentication

    PEG_TRACE_CSTRING(
        TRC_AUTHENTICATION,
        Tracer::LEVEL3,
        "HTTPAuthenticatorDelegator - Authentication processing ended");


    if (authStatus.isSuccess() || !enableAuthentication)
    {
        // Final bastion to ensure the remote privileged user access
        // check is done as it should be
        // check for remote privileged User Access
        if (!httpMessage->authInfo->getRemotePrivilegedUserAccessChecked())
        {
            // the AuthenticationHandler did not process the
            // enableRemotePrivilegedUserAccess check
            // time to do it ourselves
            String userName = httpMessage->authInfo->getAuthenticatedUser();
            if (!AuthenticationManager::isRemotePrivilegedUserAccessAllowed(
                    userName))
            {
                // Send client a message that we can't proceed to talk
                // to him
                // HTTP 401 ?
                MessageLoaderParms msgParms(
                    "Server.CIMOperationRequestAuthorizer."
                        "REMOTE_NOT_ENABLED",
                    "Remote privileged user access is not enabled.");
                String msg(MessageLoader::getMessage(msgParms));
                _sendHttpError(
                    queueId,
                    HTTP_STATUS_UNAUTHORIZED,
                    String::EMPTY,
                    msg,
                    closeConnect);
                PEG_METHOD_EXIT();
                return;
            }
            httpMessage->authInfo->setRemotePrivilegedUserAccessChecked();
        }

        //
        // Determine the type of this request:
        //
        //   - A "CIMOperation" header indicates a CIM operation request
        //   - A "CIMExport" header indicates a CIM export request
        //   - A "/wsman" path in the start message indicates a WS-Man request
        //   - The requestUri starting with "/cimrs" indicates a CIM-RS request
        CString uri = requestUri.getCString();

        const char* cimOperation;

        if (HTTPMessage::lookupHeader(
                headers, _HTTP_HEADER_CIMOPERATION, cimOperation, true))
        {
            PEG_TRACE((
                TRC_HTTP,
                Tracer::LEVEL3,
                "HTTPAuthenticatorDelegator - CIMOperation: %s",
                cimOperation));

            MessageQueue* queue =
                MessageQueue::lookup(_cimOperationMessageQueueId);

            if (queue)
            {
                httpMessage->dest = queue->getQueueId();

                try
                {
                    queue->enqueue(httpMessage);
                }
                catch (const bad_alloc&)
                {
                    delete httpMessage;
                    HTTPConnection *httpQueue =
                        dynamic_cast<HTTPConnection*>(
                             MessageQueue::lookup(queueId));
                    if (httpQueue)
                    {
                        httpQueue->handleInternalServerError(0, true);
                    }
                    PEG_METHOD_EXIT();
                    deleteMessage = false;
                    return;
                }
                deleteMessage = false;
            }
        }
        else if (HTTPMessage::lookupHeader(
                     headers, _HTTP_HEADER_CIMEXPORT, cimOperation, true))
        {
            PEG_TRACE((
                TRC_AUTHENTICATION,
                Tracer::LEVEL3,
                "HTTPAuthenticatorDelegator - CIMExport: %s",
                cimOperation));

            MessageQueue* queue =
                MessageQueue::lookup(_cimExportMessageQueueId);

            if (queue)
            {
                httpMessage->dest = queue->getQueueId();

                queue->enqueue(httpMessage);
                deleteMessage = false;
            }
        }
        else if ((_wsmanOperationMessageQueueId != PEG_NOT_FOUND) &&
                 ((requestUri == "/wsman") ||
                  ((requestUri == "/wsman-anon") && !enableAuthentication)))
        {
            // Note: DSP0226 R5.3-1 specifies if /wsman is used,
            // unauthenticated access should not be allowed.  This "should"
            // requirement is not implemented here, because it is difficult
            // for a client to determine whether enableAuthentication=true.

            // DSP0226 R5.3-2 specifies if /wsman-anon is used, authenticated
            // access shall not be required.  Unauthenticated access is
            // currently not allowed if enableAuthentication=true.  When
            // support for wsmid:Identify is added, it will be necessary to
            // respond to that request without requiring authentication,
            // regardless of the CIM Server configuration.

            MessageQueue* queue =
                MessageQueue::lookup(_wsmanOperationMessageQueueId);

            if (queue)
            {
                httpMessage->dest = queue->getQueueId();

                try
                {
                    queue->enqueue(httpMessage);
                }
                catch (const bad_alloc&)
                {
                    delete httpMessage;
                    HTTPConnection *httpQueue =
                        dynamic_cast<HTTPConnection*>(
                             MessageQueue::lookup(queueId));
                    if (httpQueue)
                    {
                        httpQueue->handleInternalServerError(0, true);
                    }
                    PEG_METHOD_EXIT();
                    deleteMessage = false;
                    return;
                }
                deleteMessage = false;
            }
        }
        else if (
            (_rsOperationMessageQueueId != PEG_NOT_FOUND) &&
            (strncmp((const char*)uri, "/cimrs", 6) == 0))
        {
            MessageQueue* queue = MessageQueue::lookup(
                                  _rsOperationMessageQueueId);
            if (queue)
            {
                httpMessage->dest = queue->getQueueId();
                try
                {
                    PEG_TRACE((TRC_HTTP, Tracer::LEVEL4,
                            "HTTPAuthenticatorDelegator - "
                            "CIM-RS request enqueued [%d]",
                            queue->getQueueId()));
                    queue->enqueue(httpMessage);
                }
                catch (const bad_alloc&)
                {
                    delete httpMessage;
                    _sendHttpError(
                        queueId,
                        HTTP_STATUS_REQUEST_TOO_LARGE,
                        String::EMPTY,
                        String::EMPTY,
                        closeConnect);
                    PEG_METHOD_EXIT();
                    deleteMessage = false;
                    return;
                }
                deleteMessage = false;
            }
            else
            {
                PEG_TRACE((TRC_HTTP, Tracer::LEVEL3,
                        "HTTPAuthenticatorDelegator - "
                        "Queue not found for URI: %s\n",
                        (const char*)uri));
            }
        }
#ifdef PEGASUS_ENABLE_PROTOCOL_WEB
        //Unlike Other protocol, Web server is an pegasus extension
        //and uses method name to identify it as it we don't have any
        //like /cimrs and /wsman yet
        //Instead, We deduce operation request to Webserver through it's
        //method name GET and HEAD which is HACKISH.
        else if ((_webOperationMessageQueueId != PEG_NOT_FOUND) &&
            (httpMethod == HTTP_METHOD_GET || httpMethod == HTTP_METHOD_HEAD ))
        {
            MessageQueue* queue = MessageQueue::lookup(
                                  _webOperationMessageQueueId);
            if (queue)
            {
                httpMessage->dest = queue->getQueueId();
                try
                {
                PEG_TRACE((TRC_HTTP, Tracer::LEVEL4,
                            "HTTPAuthenticatorDelegator - "
                            "WebServer request enqueued [%d]",
                            queue->getQueueId()));
                queue->enqueue(httpMessage);
                }
                catch (const bad_alloc&)
                {
                    delete httpMessage;
                    HTTPConnection *httpQueue =
                        dynamic_cast<HTTPConnection*>(
                             MessageQueue::lookup(queueId));
                    if (httpQueue)
                    {
                        httpQueue->handleInternalServerError(0, true);
                    }
                    PEG_METHOD_EXIT();
                    deleteMessage = false;
                    return;
                }
                catch (Exception& e)
                {
                    PEG_TRACE((TRC_HTTP, Tracer::LEVEL4,
                                "HTTPAuthenticatorDelegator - "
                                "WebServer has thrown an exception: %s",
                                (const char*)e.getMessage().getCString()));
                    delete httpMessage;
                    _sendHttpError(
                        queueId,
                        HTTP_STATUS_INTERNALSERVERERROR,
                        String::EMPTY,
                        String::EMPTY,
                        true);
                    PEG_METHOD_EXIT();
                    deleteMessage = false;
                    return;
                }
                catch (...)
                {
                    delete httpMessage;
                    HTTPConnection *httpQueue =
                        dynamic_cast<HTTPConnection*>(
                             MessageQueue::lookup(queueId));
                    if (httpQueue)
                    {
                        httpQueue->handleInternalServerError(0, true);
                    }
                    PEG_METHOD_EXIT();
                    deleteMessage = false;
                    return;
                }
                    deleteMessage = false;
            }
            else
            {
               PEG_TRACE((TRC_HTTP, Tracer::LEVEL3,
                        "HTTPAuthenticatorDelegator - "
                        "Queue not found for URI: %s\n",
                        (const char*)requestUri.getCString()));
            }
        }
#endif /* PEGAUS_ENABLE_PROTOCOL_WEB */
        else
        {
            // We don't recognize this request message type

            // The Specification for CIM Operations over HTTP reads:
            //
            //     3.3.4. CIMOperation
            //
            //     If a CIM Server receives a CIM Operation request without
            //     this [CIMOperation] header, it MUST NOT process it as if
            //     it were a CIM Operation Request.  The status code
            //     returned by the CIM Server in response to such a request
            //     is outside of the scope of this specification.
            //
            //     3.3.5. CIMExport
            //
            //     If a CIM Listener receives a CIM Export request without
            //     this [CIMExport] header, it MUST NOT process it.  The
            //     status code returned by the CIM Listener in response to
            //     such a request is outside of the scope of this
            //     specification.
            //
            // The author has chosen to send a 400 Bad Request error, but
            // without the CIMError header since this request must not be
            // processed as a CIM request.

            _sendHttpError(
                queueId,
                HTTP_STATUS_BADREQUEST,
                String::EMPTY,
                String::EMPTY,
                closeConnect);
            PEG_METHOD_EXIT();
            return;
        } // bad request
    } // isRequestAuthenticated and enableAuthentication check
    else
    {  // client not authenticated; send challenge
#ifdef PEGASUS_NEGOTIATE_AUTHENTICATION
        String authResp =
            _authenticationManager->getHttpAuthResponseHeader(
                httpMessage->authInfo);
#else
        String authResp =
            _authenticationManager->getHttpAuthResponseHeader();
#endif
        if (authResp.size() > 0)
        {
            if (authStatus.doChallenge())
            {
                _sendChallenge(
                    queueId,
                    authStatus.getErrorDetail(),
                    authResp,
                    closeConnect);
            }
            else
            {
                _sendHttpError(
                    queueId,
                    authStatus.getHttpStatus(),
                    String::EMPTY,
                    authStatus.getErrorDetail(),
                    closeConnect);
            }
        }
        else
        {
            MessageLoaderParms msgParms(
                "Pegasus.Server.HTTPAuthenticatorDelegator."
                    "AUTHORIZATION_HEADER_ERROR",
                "Authorization header error");
            String msg(MessageLoader::getMessage(msgParms));
            _sendHttpError(
                queueId,
                HTTP_STATUS_BADREQUEST,
                String::EMPTY,
                msg,
                closeConnect);
        }
    }
PEG_METHOD_EXIT();
}

#ifdef PEGASUS_ENABLE_SESSION_COOKIES
void HTTPAuthenticatorDelegator::_createCookie(
        HTTPMessage *httpMessage)
{
    PEG_METHOD_ENTER(TRC_HTTP,
        "HTTPAuthenticatorDelegator::_createCookie");

    if (!_sessions->cookiesEnabled())
    {
        PEG_METHOD_EXIT();
        return;
    }

    // The client passed authentication, give it a cookie
    String sessionID = _sessions->addNewSession(
            httpMessage->authInfo->getAuthenticatedUser(),
            httpMessage->ipAddress);
    const char attributes[] = ";secure;httpOnly;maxAge=";
    ConfigManager *configManager = ConfigManager::getInstance();
    String strTimeout = configManager->getCurrentValue("httpSessionTimeout");

    String cookie;
    cookie.reserveCapacity(sizeof(_COOKIE_NAME) + 1 + sessionID.size()
            + sizeof(attributes) + strTimeout.size() + 1);
    cookie.append(_COOKIE_NAME);
    cookie.append("=");
    cookie.append(sessionID);
    cookie.append(attributes);
    cookie.append(strTimeout);

    // Schedule the cookie to be added in the next response
    httpMessage->authInfo->setCookie(cookie);

    PEG_METHOD_EXIT();
}
#endif

void HTTPAuthenticatorDelegator::idleTimeCleanup()
{
    PEG_METHOD_ENTER(TRC_HTTP, "HTTPAuthenticatorDelegator::idleTimeCleanup");
#ifdef PEGASUS_ENABLE_SESSION_COOKIES
    _sessions->clearExpired();
#endif
    PEG_METHOD_EXIT();
}

PEGASUS_NAMESPACE_END
