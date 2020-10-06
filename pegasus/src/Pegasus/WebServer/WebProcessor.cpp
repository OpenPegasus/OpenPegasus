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

#include <time.h>
#include <Pegasus/Common/AutoPtr.h>
#include <Pegasus/Common/HTTPConnection.h>
#include <Pegasus/Common/Logger.h>
#include <Pegasus/Common/Tracer.h>
#include <Pegasus/Common/XmlWriter.h>
#include <Pegasus/Common/FileSystem.h>
#include <Pegasus/Common/Constants.h>

#include <Pegasus/WebServer/WebProcessor.h>
#include <Pegasus/WebServer/WebServer.h>


PEGASUS_USING_STD;
PEGASUS_NAMESPACE_BEGIN


const Uint32 WebProcessor::DEFAULT_RESPONSE_BUFFER_SIZE = 8192;// 8 KB
const Uint32 WebProcessor::MAX_URI_LENGTH = 256;
const String WebProcessor::HTTP_VERSION = "1.1";
const String WebProcessor::GZIP = "gzip";
const String WebProcessor::DEFLATE = "deflate";


WebProcessor::WebProcessor(WebServer* const webServer)
    : _webConfig(), _webServer(webServer)
{
}

WebProcessor::~WebProcessor()
{
}

void WebProcessor::handleWebRequest(WebRequest* request)
{
    PEG_METHOD_ENTER(TRC_WEBSERVER, "WebProcessor::handleWebRequest()");

    if (!request)
    {
        PEG_METHOD_EXIT();
        return;
    }

    Uint32 queueId = request->getQueueId();


    // check protocol version
    if( String::equal(request->httpVersion, "") ||
            PEG_NOT_FOUND == request->httpVersion.find("HTTP/"))
    {// bad request
        _sendError( HTTP_STATUSCODE_BADREQUEST, queueId,
            "Malformed Http, http version string not found!");
        PEG_METHOD_EXIT();
        return;
    }
    else if( !String::equal(request->httpVersion, "HTTP/" + HTTP_VERSION))
    {// protocol-version not supported
        _sendError(
            HTTP_STATUSCODE_VERSIONNOTSUPPORTED,
            queueId,
            "The requested HTTP version '" + request->httpVersion
                 + "' is not supported by this server!");
        PEG_METHOD_EXIT();
        return;
    }
    PEG_TRACE((TRC_WEBSERVER,Tracer::LEVEL3,"WebServer::handleWebRequest"
        "(WebRequest* webRequest) - httpVersion='%s'",
        (const char*)request->httpVersion.getCString()));

    const char* method = NULL;
    // check HTTPmethod
    if (request->httpMethod == HTTP_METHOD_GET)
    {
        method = "GET";
    }
    else if (request->httpMethod == HTTP_METHOD_HEAD)
    {
        method = "HEAD";
    }
    else
    {// handle bad request (method not allowed)
        _sendError(
            HTTP_STATUSCODE_METHODNOTALLOWED,
            queueId,
            "The requested HTTP method is not supported by this server,"
                " 'GET' and 'HEAD' only!",
                "Allow: HEAD, GET");
        PEG_METHOD_EXIT();
        return;
    }
    PEG_TRACE((TRC_WEBSERVER,Tracer::LEVEL4,"Requested httpMethod= %s",method));

    // get absolute filename from URI
    String fileName;
    Uint32 statusCode = _getFileNameForURI(request->requestURI, fileName);
    if (statusCode != HTTP_STATUSCODE_OK)
    {
        _sendError(
            statusCode,
            queueId,
            "<p>Request-URI: <b>'" + request->requestURI
                + "'</b><br/>"
                + "Current web-root: <b>'" + _webConfig.getWebRoot()
                + "'</b>"
                + ((statusCode == HTTP_STATUSCODE_FORBIDDEN)?
                    ("<br/><br/>Reason: It points to a directory or "
                     "the requested file's real path is not located "
                     "in the webRoot!</p>")
                    :("</p>")));
        PEG_METHOD_EXIT();
        return;
    }
    PEG_TRACE((TRC_WEBSERVER,Tracer::LEVEL4,"URI maps to valid/allowed "
        "fileName='%s'",
        (const char*)fileName.getCString()));

    // get and check contentType for requested file
    /* RFC 2616, section 10.4.7 '406 Not Acceptable'
     *
     * Note: HTTP/1.1 servers are allowed to return responses which are
     * not acceptable according to the accept headers sent in the
     * request. In some cases, this may even be preferable to sending a
     * 406 response. User agents are encouraged to inspect the headers of
     * an incoming response to determine if it is acceptable.
     *
     * => So there's no need to check whether the client is willing to accept 
     *    the delivered file with a header line in the response including a 
     *    mime-type definition it does not know.
     */
    String contentType;
    if (!_getContentType(fileName, contentType))
    {
        // no contentType known for requested file(-extension)
        _sendError(
            HTTP_STATUSCODE_FORBIDDEN, queueId,
            "The requested file '" + fileName
                 + "' has an undefined content type. It will not be served!");
        PEG_METHOD_EXIT();
        return;
    }
    PEG_TRACE((TRC_WEBSERVER,Tracer::LEVEL4,
        " Requested response contentType='%s'",
            (const char*)contentType.getCString()));

    /* is it a text based mime-type ?
     *
     * Notice:
     * The mimeTypes-file mapping could be extended to an additional value
     * indicating whether the file is to treat as binary or text.
     * There are text based mime-types which do not start with 'text/'.
     */
    Boolean isBinFile = String::compare(contentType.subString(0, 4), "text");

    PEG_TRACE((TRC_WEBSERVER,Tracer::LEVEL3,"WebServer::handleWebRequest"
        "(WebRequest* webRequest) - contentType is binary='%s'",
        (isBinFile ? "true" : "false" )));

    /*
     * check requested encodings
     * (for ex. 'gzip, defalte')
     */
    String enc;// determined encoding to use
    statusCode = _getRequestHeaderValue(
                    request->encodings,
                    const_cast<String&>(GZIP),
                    enc);

    if (statusCode != HTTP_STATUSCODE_OK)
    {
        statusCode = _getRequestHeaderValue( request->encodings,
                        const_cast<String&>(DEFLATE),
                        enc);

        if (statusCode == HTTP_STATUSCODE_BADREQUEST)
        {
           _sendError(
                statusCode,
                queueId,
                "Bad Syntax in header-parameter 'Accept-Encoding' !");

            PEG_METHOD_EXIT();
            return;
        }
    }
    //compress files bigger than 10KB, only
    Boolean compressionFlag = String::equal(GZIP, enc) 
                                  || String::equal(DEFLATE, enc);

    PEG_TRACE((TRC_WEBSERVER,Tracer::LEVEL4,"WebServer::handleWebRequest"
        "(WebRequest* webRequest) - response encoding='%s', "
        "compressionFlag='%s'",
        (const char*)enc.getCString(),
        (compressionFlag) ? "true" : "false"));

    PEG_TRACE_CSTRING(TRC_WEBSERVER,Tracer::LEVEL4,"WebServer::handleWebRequest"
        "(WebRequest* webRequest) - All Headers have been parsed and "
            "successfully validated.");


    /*
     * check access to file/if exists
     */
    if (!FileSystem::canRead(fileName))
    {// file not accessible
        _sendError(
            HTTP_STATUSCODE_FORBIDDEN,
            queueId,
            "The requested file '" + fileName + "' is not accessible!");

        PEG_METHOD_EXIT();
        return;
    }

    PEG_TRACE((TRC_WEBSERVER, Tracer::LEVEL4,
        "WebServer::handleWebRequest(WebRequest* webRequest) - "
            "File accessible, creating response. HTTP-statusCode: %d ",
        statusCode));

    // initialize response-buffer
    Buffer tmp = Buffer(DEFAULT_RESPONSE_BUFFER_SIZE);
    Buffer& _message = tmp;

    /*
     * create response
     */
    _message << "HTTP/" << HTTP_VERSION << " " << statusCode << " "
                << _getStatusMessage(statusCode).getCString() << "\r\n";
    // response Date
    _message << "Date: " << _getCurrentDate() << "\r\n";
    _message << "Last-Modified: " << _getDateOfLastMod(fileName) << "\r\n";

    if (isBinFile)// || compressionFlag) TODO: enable for compression
    {
        _message << "Accept-Ranges: bytes\r\n";
    }
    else
    {
        _message << "Accept-Ranges: text/plain\r\n";
    }

    // set content-type of file to deliver
    _message << "Content-Type: " << contentType.getCString();
    if (!isBinFile && !compressionFlag)
    {// no charset for bin-files
        _message << ";charset=utf-8";
    }
    _message << "\r\n";

    // compress the file before delivery ?
    if (compressionFlag)
    {
        /*TODO
         * compress the file and set the new filename
         * for further processing
         */
//        _message << "Content-Encoding: " << enc << "\r\n";
    }

    Uint32 fileSize;
    if (!FileSystem::getFileSize(fileName, fileSize))
    {
        _sendError(
            HTTP_STATUSCODE_INTERNALSERVERERROR,
            queueId,
            "The size for requested file '" + fileName
             + "' could not be determined!");
        PEG_METHOD_EXIT();
        return;
    }
    // get file's size
    if (isBinFile)// || compressionFlag) TODO: enable for compression
    {/*
      * plain text-files cause trouble due to the "\r\n" which is not counted,
      * but added for each line
      */
        _message << "Content-Length: " << fileSize << "\r\n";
    }
    _message << "Connection: keep-alive \r\n";
    // end of headers
    _message << "\r\n";

    PEG_TRACE_CSTRING(TRC_WEBSERVER,Tracer::LEVEL4,"WebServer::handleWebRequest"
        "(WebRequest* webRequest) - All response headers have been written.");

    /*
     * Write Body
     */
    if (request->httpMethod != HTTP_METHOD_HEAD)
    {
        if (fileSize != PEG_NOT_FOUND)
        {
            _message.reserveCapacity(fileSize);
        }
        if (isBinFile)// || compressionFlag) TODO: enable for compression
        {
            FileSystem::loadFileToMemory(_message, fileName);
        }
        else
        {
            _writeTextBody(_message, fileName);
        }
    }

    PEG_TRACE((TRC_WEBSERVER,Tracer::LEVEL4,"WebServer::handleWebRequest"
        "(WebRequest* webRequest) - response:\n%s\n",
        Tracer::getHTTPRequestMessage(_message).get()));

    // create response message
    HTTPMessage* response = new HTTPMessage(_message, queueId);

    // set close connection flag
    response->setCloseConnect(true);

    // request-object is not needed any longer, free memory
    delete request;

    /*
     * send response
     */
    _sendRepsonse(response);
    PEG_METHOD_EXIT();
}

void WebProcessor::_sendError(
    int statusCode,
    Uint32& queueId,
    String debugMsg,
    String additionalHeaderFields)
{
    PEG_METHOD_ENTER(TRC_WEBSERVER, "WebProcessor::_sendError()");

    Uint32 statusC = Uint32(statusCode);
    PEG_TRACE((TRC_WEBSERVER,Tracer::LEVEL1,"WebServer::_sendError"
        "(Uint32 statusCode, Uint32 queueId, String debugMsg) - "
            "statusCode: %d, QueueId: %d, debugMsg: %s",
        statusCode, queueId, (const char*)debugMsg.getCString()));

    // initialize response-buffer
    Buffer tmp = Buffer(DEFAULT_RESPONSE_BUFFER_SIZE);
    Buffer& _message = tmp;

    /*
     * create response
     */
    _message << "HTTP/" << HTTP_VERSION << " " << statusC << " "
                << _getStatusMessage(statusC) << "\r\n";
    // response Date
    _message << "Date: " << _getCurrentDate();
    /*
     * additional header-fields required for certain errors
     */
    if (additionalHeaderFields.size() > Uint32(0))
    {
       _message << additionalHeaderFields << "\r\n";
    }
    /*
     * handleError
     */
    _message << "Content-Type: text/html;charset=UTF-8\r\n";//UTF-8 response
    _message << "\r\n\r\n"; // end of headers
    _message << getErrorPage(statusC, debugMsg);

     PEG_TRACE((TRC_WEBSERVER,Tracer::LEVEL1,"WebServer::_sendError"
        "(Uint32 statusCode, Uint32 queueId, String debugMsg) - "
            "response:\n%s\n",
        Tracer::getHTTPRequestMessage(_message).get()));

    // create response message
    HTTPMessage* response = new HTTPMessage(_message, queueId);

    // set close connection flag
    response->setCloseConnect(true);
    _sendRepsonse(response);
    PEG_METHOD_EXIT();
}

void WebProcessor::_sendRepsonse(HTTPMessage* response)
{
    PEG_METHOD_ENTER(TRC_WEBSERVER, "WebProcessor::_sendRepsonse()");
    _webServer->handleResponse(response);
    PEG_METHOD_EXIT();
}

String WebProcessor::getErrorPage(Uint32& statusCode, String& debugMsg)
{
    PEG_METHOD_ENTER(TRC_WEBSERVER, "WebProcessor::getErrorPage()");
    String page = String("<html><head><title>");
    page.reserveCapacity(debugMsg.size() + 512);// 512 for the html-strings
    page.append("Error");
    page.append("</title></head><body><br/><h1>");
    page.append(_getStatusMessage(statusCode));
    page.append("</h1>");
#ifdef PEGASUS_DEBUG
    page.append(debugMsg);
#endif /* PEGASUS_DEBUG */
    page.append("</body></html>\r\n");

    PEG_METHOD_EXIT();
    return page;
}


Uint32 WebProcessor::_getFileNameForURI(String& requestURI, String& absPath)
{
    PEG_METHOD_ENTER(TRC_WEBSERVER, "WebProcessor::_getFileNameForURI()");

    if (requestURI.size() == Uint32(0))
    {
        // bad request, no uri supplied
        PEG_METHOD_EXIT();
        return HTTP_STATUSCODE_BADREQUEST;
    }
    else if (requestURI.size() > MAX_URI_LENGTH)
    {
        // request uri too long
        PEG_METHOD_EXIT();
        return HTTP_STATUSCODE_REQUESTURITOOLONG;
    }


    String fileName;
    Uint32 index = requestURI.find("?");
    if (index != PEG_NOT_FOUND)
    {// cut parameters from URL ex: '?id=...&pb=..'
        fileName = requestURI.subString(0, index);
    }
    else
    {
        fileName = requestURI;
    }

    if (fileName == "/")
    {// map requests pointing to web-server's document-root to the index file
        fileName.append(_webConfig.getIndexFile());
    }

    /*
     * construct fileName and validate
     */
    String webRoot = _webConfig.getWebRoot();
    String fullPath = (webRoot + fileName);
    String resolvedPath = FileSystem::getAbsoluteFileName(webRoot,fileName);

    if (FileSystem::isDirectory((const String&)resolvedPath))
    {// it is not allowed to access directories, error 403
        PEG_TRACE((TRC_WEBSERVER,Tracer::LEVEL1,
            "Directory access to %s is not allowed!",
            (const char*)resolvedPath.getCString()));
        PEG_METHOD_EXIT();
        return HTTP_STATUSCODE_FORBIDDEN;
    }

    if (resolvedPath == "")
    {
        PEG_TRACE((TRC_WEBSERVER,Tracer::LEVEL1,
            "Cannot resolve file path %s",
            (const char*)fullPath.getCString()));
        PEG_METHOD_EXIT();
        return HTTP_STATUSCODE_NOTFOUND;
    }
    /*
     * further checks can be placed here, for example to
     * prevent delivery of any config-files in case they are placed
     * in the webRoot.
     */
    // clear first before appending
    absPath.clear();

    /*
     * Ensure that the realpath starts with the web-root
     */
    if (!resolvedPath.equalNoCase(
            (resolvedPath.subString(0, webRoot.size())), webRoot))
    {// directory traversal attack
        PEG_TRACE((TRC_WEBSERVER,Tracer::LEVEL1,
            "Mismatch in resolved path: %s resolves to %s",
            (const char*)fullPath.getCString(),
            (const char*)resolvedPath.getCString()));
        PEG_METHOD_EXIT();
        return HTTP_STATUSCODE_FORBIDDEN;
    }

    // append result
    absPath.append(resolvedPath);

    PEG_METHOD_EXIT();
    return HTTP_STATUSCODE_OK;
}

void WebProcessor::_writeTextBody(Buffer& _message, String& fileName)
{
    PEG_METHOD_ENTER(TRC_WEBSERVER, "WebProcessor::_writeTextBody()");
    String line;
    ifstream infile(fileName.getCString());

    if (!infile)
    {
        return;
    }
    while (GetLine(infile, line)) {
        // write UTF-8
        _message << (const char*)line.getCString() << "\r\n";
    }
    infile.close();
    PEG_METHOD_EXIT();
}


Boolean WebProcessor::_getContentType(String& fileName, String& contentType)
{
    PEG_METHOD_ENTER(TRC_WEBSERVER, "WebProcessor::_getContentType()");

    //find last dot in filename
    int found = fileName.reverseFind('.');
    if (found <= 0) {// security
        //error-case 'hidden file or file with no extension'
        return false;
    }
    String fileExtension = String(fileName.subString(found+1));
    contentType.clear();
    Boolean rslt = _webConfig.getMimeTypes().lookup(fileExtension, contentType);

    PEG_TRACE((TRC_WEBSERVER,Tracer::LEVEL4,
        "File extension is %s, contentType is %s",
        (const char*)fileExtension.getCString(),
        (const char*)contentType.getCString()));

    PEG_METHOD_EXIT();
    return rslt;
}


char* WebProcessor::_getCurrentDate()
{
//    time_t t;
//    time(&t);
//    return asctime(gmtime(&t));

    time_t currentTime;
    struct tm* gmtTime;
    char* timeValue = (char*) malloc(30);
    time(&currentTime);
    gmtTime = gmtime(&currentTime);
    strftime(timeValue,30,"%a, %d %b %Y %H:%M:%S GMT",gmtTime);
    return timeValue;
}


char* WebProcessor::_getDateOfLastMod(String& fileName)
{
    // last modified
    struct tm* modTime;            // create a time structure
    struct stat attrib;            // create a file attribute structure
    stat(fileName.getCString(), &attrib);    // get the attributes
    modTime = gmtime(&(attrib.st_mtime));    // get the last modified time
    char* timeValue = (char*) malloc(30);
    strftime(timeValue,30,"%a, %d %b %Y %H:%M:%S GMT",modTime);
    return timeValue;//asctime(modTime);
}

String WebProcessor::_getStatusMessage(Uint32& statusCode)
{
    String statusMsg;

    switch(statusCode)
    {

        /*
         * 2xx
         */
        case HTTP_STATUSCODE_OK:
            statusMsg = HTTP_REASONPHRASE_OK;
            break;

        /*
         * 4xx
         */
        case HTTP_STATUSCODE_BADREQUEST:
            statusMsg = HTTP_REASONPHRASE_BADREQUEST;
            break;
        case HTTP_STATUSCODE_FORBIDDEN:
            statusMsg = HTTP_REASONPHRASE_FORBIDDEN;
            break;
        case HTTP_STATUSCODE_NOTFOUND:
            statusMsg = HTTP_REASONPHRASE_NOTFOUND;
            break;
        case HTTP_STATUSCODE_METHODNOTALLOWED:
            statusMsg = HTTP_REASONPHRASE_METHODNOTALLOWED;
            break;
        case HTTP_STATUSCODE_NOTACCEPTABLE:
            statusMsg = HTTP_REASONPHRASE_NOTACCEPTABLE;
            break;
        case HTTP_STATUSCODE_REQUESTURITOOLONG:
            statusMsg = HTTP_REASONPHRASE_REQUESTURITOOLONG;
            break;
        /*
         * 5xx
         */
        case HTTP_STATUSCODE_VERSIONNOTSUPPORTED:
            statusMsg = HTTP_REASONPHRASE_VERSIONNOTSUPPORTED;
            break;
    }

    return statusMsg;
}



Uint32 WebProcessor::_getRequestHeaderValue(
        String& headerLine, String& prefValue, String& resValue)
{

    resValue.clear();
    // default
    resValue.append(prefValue);

    if (headerLine.size() > Uint32(0))
    {

        Array<String> values = _split(headerLine, ",");
        Uint32 any = PEG_NOT_FOUND;

        // header line available ?
        if (values.size() == Uint32(0))
        {// no, free choice
            any = 0;
        }
        else
        {
            // check requested mime types
            for (int i = 0; Uint32(i) < values.size(); i++)
            {
                Array<String> valueDef = _split(values[i], ";");
                // quality value present ?
                if (valueDef.size() > Uint32(1))
                {// yes, get it
                    String qFactorStr = valueDef[1];
                    // does the length make any sense ? (ex. q=0 & q=0.4)
                    if (qFactorStr.size() > Uint32(2)
                            && qFactorStr.size() < Uint32(5))
                    {// cast the value
                        double qFactor;
                        try
                        {
                            qFactor = atof(// remove 'q='
                                        qFactorStr.subString(
                                          2,qFactorStr.size()-1).getCString());
                        }
                        catch(...)
                        {
                            return HTTP_STATUSCODE_BADREQUEST;
                        }
                        if (String::equal(valueDef[0], prefValue)
                            && qFactor == 0)
                        {// preferred value is explicitly excluded
                            /*
                             * TODO figure out if there's one of the other
                             *         request values acceptable for processing
                             */
                            return HTTP_STATUSCODE_NOTACCEPTABLE;
                        }
                    }
                    else
                    {// invalid length of quality value
                        return HTTP_STATUSCODE_BADREQUEST;
                    }
                }
                else
                {// no quality value present means it is '1'
                    if (String::equal(prefValue, valueDef[0]))
                    {// the server-side preferred value is acceptable
                        return HTTP_STATUSCODE_OK;
                    }
                }
            }
        }
    }
    return HTTP_STATUSCODE_OK;
}

Array<String> WebProcessor::_split(String& s, const char* delimiter)
{
    Array<String> result;
    Uint32 delimLength = strlen(delimiter);
    Boolean run = true;

    while(run)
    {
        Uint32 pos = s.find(delimiter);
        if (pos == PEG_NOT_FOUND)
        {
            pos = s.size();
            run = false;
        }
        String res = s.subString(0, pos);
        result.append(res);
        s = s.subString(pos + delimLength);
        // remove leading whitespace
        while (s.size() > Uint32(0) && s[0] == " ")
        {
            s = s.subString(1);
        }
    }
    return result;
}
/* END */

PEGASUS_NAMESPACE_END
