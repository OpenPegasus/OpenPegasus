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

#ifndef Pegasus_WebProcessor_h
#define Pegasus_WebProcessor_h

#include <Pegasus/WebServer/Linkage.h>
#include <Pegasus/WebServer/WebConfig.h>
#include <Pegasus/WebServer/WebRequest.h>

PEGASUS_USING_STD;
PEGASUS_NAMESPACE_BEGIN


class WebServer;

/**
 *   Processes WebRequest messages and produces WebResponse messages.
 */
class PEGASUS_WEBSERVER_LINKAGE WebProcessor
{

public:

    /**
     * Constructor.
     *
     * @param webServer
     *             Generated responses will be passed to it.
     */
    WebProcessor(WebServer* const webServer);

    /**
     * Destructor.
     */
    ~WebProcessor();



    /**
     * Default size for initialization.of the response message-buffer.
     */
    static const Uint32 DEFAULT_RESPONSE_BUFFER_SIZE;

    /**
     * Maximum URI length acceptable for requests.
     */
    static const Uint32 MAX_URI_LENGTH;

    /**
     * Used version of the HTTP.
     */
    static const String HTTP_VERSION;

    /**
     * Constant identifying gzip compression.
     */
    static const String GZIP;

    /**
     * Constant identifying zlib compression.
     */
    static const String DEFLATE;


    /**
     * Processes the request and generates the corresponding answer.
     *
     * @param webRequest
     *             Request to process.
     */

    void handleWebRequest(WebRequest* webRequest);

    /**
     * Returns the corresponding error-page for the given status code.
     *
     * @param statusCode
     *             HTTP status code as defined by the rfc2616.
     *             Currently there are messages for a small selection available,
     *             only.
     * @param debugMsg
     *             In case the 'PEGASUS_DEBUG' pre-compiler flag has been
     *             set the given debugMsg will be included in the error-page's.
     */
    String getErrorPage(Uint32& statusCode, String& debugMsg);


private:

    /**
     * Configuration containing the web-server's document root and more.
     */
    WebConfig _webConfig;

    /**
     * WebServer used for delivering generated responses back to the client.
     */
    WebServer* const _webServer;


    /**
     * Sends an error-page to the requester.
     *
     * @param statusCode
     *             HTTP-Statuscode
     * @param queueId
     *             QueueID required for successful delivery to the requester.
     * @param debugMsg
     *             Message will appear in trace and in PEGASUS_DEBUG mode it
     *             will be included in the error-page response.
     * @param additionalHeaderFields
     *             Will be included in the error response. In case it has
     *             several lines, each line but the last one must end with
     *             "\r\n".
     */
    void _sendError(
            int statusCode,
            Uint32& queueId,
            String debugMsg,
            String additionalHeaderFields = "");

    /**
     * Sends response.
     *
     * @param response
     *             HTTPMEssage to send.
     */
    void _sendRepsonse(HTTPMessage* response);

    /**
     * Writes text based files to the given buffer in the given destination
     * encoding.
     *
     * @param message
     *             Buffer of response-message to write the given file-stream to.
     * @param fileName
     *             Fullpath to the file to write into response buffer.
     */
    void _writeTextBody(Buffer& message, String& fileName);

    /**
     * Looks up the contentType (mime-type) for the given filename
     * in the WebConfig.
     *
     * @param fileName
     *             Name of file to look up mimeType of.
     * @param mimeType
     *             Will be cleared before writing mime-type if present, so the
     *             method call may result in an empty mimeType-String.
     * @return Whether the lookup was successful.
     */
    Boolean _getContentType(String& fileName, String& mimeType);

    /**
     * Constructs the full path name of the file requested and does furthermore
     * resolving to the real path and ensures it is located in the web-server's
     * document-root.
     *
     * @param requestURI
     *             URI requested
     * @param absPath
     *             absolute path of file, may be empty
     * @param HTTP status code: 'HTTP_STATUSCODE_OK', etc.
     */
    Uint32 _getFileNameForURI(String& requestURI, String& absPath);

    /**
     * Determines date of last file modification.
     *
     * @param fileName
     *             Name of file to determine date of last modification of.
     * @return Date as GMT
     */
    char* _getDateOfLastMod(String& fileName);

    /**
     * Returns current date.
     *
     * @return current date.
     * TODO: Deleisha check if getCurrentTimeASSCI does the same thing
     */
    char* _getCurrentDate();

    /**
     * Returns the corresponding status-message for the given status code.
     *
     * @param statusCode
     *             Status code to return message for
     * @return Message for given status code, like message 'OK' for code '200'
     */
    String _getStatusMessage(Uint32& statusCode);


    /**
     * Parses the header line and check whether prefValue is an option.
     * In case of a broken or invalid header line HTTP_STATUS_BADREQUEST will
     * be returned, in case of explicit exclude of the preferred value
     * HTTP_STATUS_NOTACCEPTABLE
     *
     * @param headerLine
     *             Line to parse.
     * @param prefValue
     *             Server-side preferred value to search for.
     * @param resValue
     *             The encoding to use, which has been determined.
     *
     * @return HTTP_STATUS_OK, HTTP_STATUS_BADREQUEST, HTTP_STATUS_NOTACCEPTABLE
     */
    Uint32 _getRequestHeaderValue(
            String& headerLine, String& prefValue, String& resValue);

    /**
     * Splits given string with given delimiter.
     * The result contains at least s.
     *
     * @param s
     *             String to split.
     * @param delimiter
     *             Delimiter to use for splitting.
     */
    Array<String> _split(String& s, const char* delimiter);
};

PEGASUS_NAMESPACE_END

#endif /* Pegasus_WebProcessor_h */
