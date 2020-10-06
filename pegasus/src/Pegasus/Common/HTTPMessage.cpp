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
#include <iostream>
#include <cstring>
#include "HTTPMessage.h"
#include "System.h"
#include "ArrayIterator.h"

PEGASUS_USING_STD;

PEGASUS_NAMESPACE_BEGIN

//------------------------------------------------------------------------------
//
// Implementation notes:
//
//     According to the HTTP specification:
//
//         1.  Method names are case-sensitive.
//         2.  Field names are case-insensitive.
//         3.  The first line of a message is known as the "start-line".
//         4.  Subsequent lines are known as headers.
//         5.  Headers have a field-name and field-value.
//         6.  Start-lines may be request-lines or status-lines. Request lines
//             have this form:
//
//             Request-Line = Method SP Request-URI SP HTTP-Version CRLF
//
//             Status-lines have this form:
//
//             Status-Line = HTTP-Version SP Status-Code SP Reason-Phrase CRLF
//
//------------------------------------------------------------------------------

char* HTTPMessage::findSeparator(const char* data)
{
    // [^\0\r\n]
    static const unsigned char _skip[256] =
    {
        0,1,1,1,1,1,1,1,1,1,0,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
        1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
        1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
        1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
        1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
        1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
        1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
        1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    };

    // Note that data is null-terminated.
    const unsigned char* p = (const unsigned char*)data;

    for (;;)
    {
        // Search for a '\0', '\r', or '\n'.
        while (_skip[*p])
            p++;

        if (!p[0])
            break;
        if (p[0] == '\r' && p[1] == '\n')
            return (char*)p;
        if (p[0] == '\n')
            return (char*)p;

        p++;
    }

    return 0;
}

void HTTPMessage::skipHeaderWhitespace(const char*& str)
{
    while (*str && (*str == ' ' || *str == '\t'))
    {
        ++str;
    }
}

Boolean HTTPMessage::expectHeaderToken(const char*& str, const char *token)
{
    PEGASUS_ASSERT(token);

    skipHeaderWhitespace(str);
    for ( ; *token ; ++str, ++token)
    {
        if (!*str || tolower(*str) != tolower(*token))
        {
            return false;
        }
    }
    return true;
}

HTTPMessage::HTTPMessage(
    const Buffer& message_,
    Uint32 queueId_, const CIMException *cimException_)
    :
    Message(HTTP_MESSAGE),
    message(message_),
    queueId(queueId_),
    authInfo(0),
    acceptLanguagesDecoded(false),
    contentLanguagesDecoded(false),
    binaryResponse(false)
{
    if (cimException_)
        cimException = *cimException_;
}


HTTPMessage::HTTPMessage(const HTTPMessage & msg)
    : Base(msg)
{
    message = msg.message;
    queueId = msg.queueId;
    authInfo = msg.authInfo;
    acceptLanguages = msg.acceptLanguages;
    contentLanguages = msg.contentLanguages;
    acceptLanguagesDecoded = msg.acceptLanguagesDecoded;
    contentLanguagesDecoded = msg.contentLanguagesDecoded;
    cimException = msg.cimException;
    binaryResponse = msg.binaryResponse;
}


Boolean HTTPMessage::parse(
    String& startLine,
    Array<HTTPHeader>& headers,
    Uint32& contentLength) const
{
    startLine.clear();
    headers.clear();
    contentLength = 0;

    char* data = (char*)message.getData();
    const Uint32 size = message.size();
    char* line = data;
    char* sep;
    Boolean firstTime = true;
    Uint32 headersFound = 0;

    while ((sep = findSeparator(line)))
    {
        // Look for double separator which terminates the header?

        if (line == sep)
        {
            // Establish pointer to content (account for "\n" and "\r\n").

            char* content = line + ((*sep == '\r') ? 2 : 1);

            // Determine length of content:

            contentLength = (Uint32)(size - (content - data));
            break;
        }

        Uint32 lineLength = (Uint32)(sep - line);

        if (firstTime)
        {
            startLine.assign(line, lineLength);
            firstTime = false;
        }
        else
        {
            // Find the colon:

            char* colon = 0;

            for (Uint32 i = 0; i < lineLength; i++)
            {
                if (line[i] == ':')
                {
                    colon = &line[i];
                    break;
                }
            }

            // This should always be true:

            if (colon)
            {
                // Get the name part:

                char* end;

                for (end = colon - 1; end > line && isspace(*end); end--)
                    ;

                end++;

                // Get the value part:

                char* start;

                for (start = colon + 1; start < sep && isspace(*start); start++)
                    ;

                HTTPHeader header(
                    Buffer(line, (Uint32)(end - line), 20),
                    Buffer(start, (Uint32)(sep - start), 50));

                headersFound++;
                if (headersFound >= PEGASUS_MAXELEMENTS_NUM)
                {
                    return false;
                }

                // From the HTTP/1.1 specification (RFC 2616) section 4.2
                // Message Headers:
                //
                // Multiple message-header fields with the same field-name
                // MAY be present in a message if and only if the entire
                // field-value for that header field is defined as a
                // comma-separated list [i.e., #(values)]. It MUST be
                // possible to combine the multiple header fields into one
                // "field-name: field-value" pair, without changing the
                // semantics of the message, by appending each subsequent
                // field-value to the first, each separated by a comma.  The
                // order in which header fields with the same field-name are
                // received is therefore significant to the interpretation
                // of the combined field value, and thus a proxy MUST NOT
                // change the order of these field values when a message is
                // forwarded.

                // This implementation concatenates duplicate header values,
                // with a comma separator.  If the resulting value is invalid,
                // that should be detected when the value is used.

                Uint32 headerIndex = 0;
                for (; headerIndex < headers.size(); headerIndex++)
                {
                    if (System::strcasecmp(
                            headers[headerIndex].first.getData(),
                            header.first.getData()) == 0)
                    {
                        break;
                    }
                }

                if (headerIndex == headers.size())
                {
                    headers.append(header);
                }
                else
                {
                    headers[headerIndex].second.append(", ", 2);
                    headers[headerIndex].second.append(
                        header.second.getData(), header.second.size());
                }
            }
        }

        line = sep + ((*sep == '\r') ? 2 : 1);
    }
    return true;
}



void HTTPMessage::printAll(ostream& os) const
{
    static const char* _HTTP_HEADER_CONTENT_TYPE = "content-type";
    Message::print(os);

    String startLine;
    Array<HTTPHeader> headers;
    Uint32 contentLength;
    parse(startLine, headers, contentLength);

    // get pointer to start of data.
    const char* content = message.getData() + message.size() - contentLength;
    // Print the first line:

    os << endl << startLine << endl;

    // Print the headers:

    Boolean image = false;

    for (Uint32 i = 0; i < headers.size(); i++)
    {
        cout << headers[i].first.getData() << ": " <<
            headers[i].second.getData() << endl;

        if (System::strcasecmp(
                headers[i].first.getData(), _HTTP_HEADER_CONTENT_TYPE) == 0)
        {
            if (strncmp(headers[i].second.getData(), "image/", 6) == 0)
                image = true;
        }
    }

    os << endl;

    // Print the content:

    for (Uint32 i = 0; i < contentLength; i++)
    {
        //char c = content[i];

        if (image)
        {
            if ((i % 60) == 0)
                os << endl;

            char c = content[i];

            if (c >= ' ' && c < '~')
                os << c;
            else
                os << '.';
        }
        else
            cout << content[i];
    }

    os << endl;
}


/*
 * Find the header prefix (i.e 2-digit number in front of cim keyword) if any.
 * If a fieldName is given it will use that, otherwise the FIRST field
 * starting with the standard keyword will be used. Given field names that do
 * not start with the standard keyword will never match.
 * if there is a keyword match, the prefix will be populated, else set to empty
 */

void HTTPMessage::lookupHeaderPrefix(
    Array<HTTPHeader>& headers_,
    const char* fieldName,
    String& prefix)
{
    ArrayIterator<HTTPHeader> headers(headers_);

    static const char keyword[] = "CIM";
    prefix.clear();

    for (Uint32 i = 0, n = headers.size(); i < n; i++)
    {
        const char* h = headers[i].first.getData();

        if ((headers[i].first.size() >= 3) &&
            (h[0] >= '0') && (h[0] <= '9') &&
            (h[1] >= '0') && (h[1] <= '9') &&
            (h[2] == '-'))
        {
            const char* fieldNameCurrent = h + 3;

            // ONLY fields starting with keyword can have prefixed according
            // to spec
            if (!String::equalNoCase(String(fieldNameCurrent, 3), keyword))
                continue;

            prefix = String(h, 3);

            // no field name given, just return the first prefix encountered
            if (!fieldName)
                break;

            if (System::strcasecmp(fieldNameCurrent, fieldName) != 0)
                prefix.clear();
            else break;
        }
    }
}

Boolean HTTPMessage::_lookupHeaderIndex(
    Array<HTTPHeader>& headers_,
    const char* fieldName,
    Uint32& headerIndex,
    Boolean allowNamespacePrefix)
{
    ArrayIterator<HTTPHeader> headers(headers_);

    for (Uint32 i = 0, n = headers.size(); i < n; i++)
    {
        if ((System::strcasecmp(headers[i].first.getData(), fieldName) == 0) ||
            (allowNamespacePrefix && (headers[i].first.size() >= 3) &&
             (headers[i].first[0] >= '0') && (headers[i].first[0] <= '9') &&
             (headers[i].first[1] >= '0') && (headers[i].first[1] <= '9') &&
             (headers[i].first[2] == '-') &&
             (System::strcasecmp(
                  headers[i].first.getData() + 3, fieldName) == 0)))
        {
            headerIndex = i;
            return true;
        }
    }

    // Not found:
    return false;
}

Boolean HTTPMessage::lookupHeader(
    Array<HTTPHeader>& headers,
    const char* fieldName,
    String& fieldValue,
    Boolean allowNamespacePrefix)
{
    Uint32 index = PEG_NOT_FOUND;

    if (_lookupHeaderIndex(headers, fieldName, index, allowNamespacePrefix))
    {
        fieldValue = String(
            headers[index].second.getData(),
            headers[index].second.size());
        return true;
    }

    return false;
}

Boolean HTTPMessage::lookupHeader(
    Array<HTTPHeader>& headers,
    const char* fieldName,
    const char*& fieldValue,
    Boolean allowNamespacePrefix)
{
    Uint32 index = PEG_NOT_FOUND;

    if (_lookupHeaderIndex(headers, fieldName, index, allowNamespacePrefix))
    {
        fieldValue = headers[index].second.getData();
        return true;
    }

    return false;
}

Boolean HTTPMessage::parseRequestLine(
    const String& startLine,
    String& methodName,
    String& requestUri,
    String& httpVersion)
{
    // Request-Line = Method SP Request-URI SP HTTP-Version CRLF

    // Extract the method-name:

    Uint32 space1 = startLine.find(' ');

    if (space1 == PEG_NOT_FOUND)
        return false;

    methodName = startLine.subString(0, space1);

    // Extract the request-URI:

    Uint32 space2 = startLine.find(space1 + 1, ' ');

    if (space2 == PEG_NOT_FOUND)
        return false;

    Uint32 uriPos = space1 + 1;

    requestUri = startLine.subString(uriPos, space2 - uriPos);

    // Extract the HTTP version:

    httpVersion = startLine.subString(space2 + 1);

    return true;
}

Boolean HTTPMessage::parseStatusLine(
    const String& statusLine,
    String& httpVersion,
    Uint32& statusCode,
    String& reasonPhrase)
{
    // Request-Line = Method SP Request-URI SP HTTP-Version CRLF
    // Status-Line = HTTP-Version SP Status-Code SP Reason-Phrase CRLF

    // Extract the HTTP version:

    Uint32 space1 = statusLine.find(' ');

    if (space1 == PEG_NOT_FOUND)
        return false;

    httpVersion = statusLine.subString(0, space1);

    // Extract the status code:

    Uint32 space2 = statusLine.find(space1 + 1, ' ');

    if (space2 == PEG_NOT_FOUND)
        return false;

    Uint32 statusCodePos = space1 + 1;
    String statusCodeStr;
    statusCodeStr = statusLine.subString(statusCodePos, space2 - statusCodePos);
    if (!sscanf(statusCodeStr.getCString(), "%u", &statusCode))
        return false;

    // Extract the reason phrase:

    reasonPhrase = statusLine.subString(space2 + 1);

    return true;
}

Boolean HTTPMessage::parseContentTypeHeader(
    const char* contentTypeHeader,
    String& type,
    String& charset)
{
    const char* str = contentTypeHeader;
    skipHeaderWhitespace(str);

    // Get the type string

    const char* end = str;
    while (*end && (*end != ' ') && (*end != '\t') && (*end != ';'))
    {
        end++;
    }

    type.assign(str, end-str);
    str = end;
    skipHeaderWhitespace(str);

    // Get the charset

    if (*str == ';')
    {
        str++;
        if (!expectHeaderToken(str, "charset") ||
            !expectHeaderToken(str, "="))
        {
            return false;
        }
        skipHeaderWhitespace(str);

        // The value may optionally be enclosed in quotes
        if (*str == '"')
        {
            str++;
            end = strchr(str, '"');
            if (!end)
            {
                return false;
            }
            charset.assign(str, end-str);
            str = end + 1;
        }
        else
        {
            end = str;
            while (*end && (*end != ' ') && (*end != '\t'))
            {
                end++;
            }
            charset.assign(str, end-str);
            str = end;
        }
    }
    else
    {
        // No charset specified; assume UTF-8.
        charset = "utf-8";
    }

    skipHeaderWhitespace(str);

    // Check for unexpected characters at the end of the value
    return !*str;
}

//
// parse the local authentication header
//
Boolean HTTPMessage::parseLocalAuthHeader(
    const String& authHeader,
    String& authType,
    String& userName,
    String& cookie)
{
    PEG_METHOD_ENTER(TRC_HTTP, "HTTPMessage::parseLocalAuthHeader()");

    //
    // Extract the authentication type:
    //
    Uint32 space = authHeader.find(' ');

    if ( space == PEG_NOT_FOUND )
    {
        PEG_METHOD_EXIT();
        return false;
    }

    authType = authHeader.subString(0, space);

    Uint32 startQuote = authHeader.find(space, '"');

    if ( startQuote == PEG_NOT_FOUND )
    {
        PEG_METHOD_EXIT();
        return false;
    }

    Uint32 endQuote = authHeader.find(startQuote + 1, '"');

    if ( endQuote == PEG_NOT_FOUND )
    {
        PEG_METHOD_EXIT();
        return false;
    }

    String temp = authHeader.subString(
        startQuote + 1, (endQuote - startQuote - 1));

    //
    // Extract the user name and cookie:
    //
    Uint32 colon = temp.find(0, ':');

    if ( colon == PEG_NOT_FOUND )
    {
        userName = temp;
    }
    else
    {
        userName = temp.subString(0, colon);
        cookie = temp;
    }

    PEG_METHOD_EXIT();

    return true;
}

//
// parse the HTTP authentication header
//
Boolean HTTPMessage::parseHttpAuthHeader(
    const String& authHeader, String& authTypeString, String& cookie)
{
    PEG_METHOD_ENTER(TRC_HTTP, "HTTPMessage::parseHttpAuthHeader()");

    //
    // Extract the authentication type:
    //
    Uint32 space = authHeader.find(' ');

    if ( space == PEG_NOT_FOUND )
    {
        PEG_METHOD_EXIT();
        return false;
    }

    authTypeString = authHeader.subString(0, space);

    //
    // Extract the cookie:
    //
    cookie = authHeader.subString(space + 1);

    PEG_METHOD_EXIT();

    return true;
}

void HTTPMessage::injectHeader(const String &header)
{
    char* data = (char*)message.getData();

    // find where the request / status line ends and inject just after it
    char* sep = findSeparator(data);
    PEGASUS_ASSERT(sep);

    int len = sep - data;
    message.insert(len, (const char*) header.getCString(), header.size());
}


Boolean HTTPMessage::parseCookieHeader(
    const String& cookieHeader,
    const String& name,
    String &value)
{
    // Cookie header syntax: Cookie: <name>=<value>;<name>=<value>...
    Uint32 i = 0;
    Uint32 size = cookieHeader.size();

    while(i < size)
    {
        // find name=value;
        Uint32 namesep = cookieHeader.find(i, '=');

        if (namesep == PEG_NOT_FOUND)
            return false;

        Uint32 valsep = cookieHeader.find(namesep, ';');
        if (valsep == PEG_NOT_FOUND)
        {
            // there is no ';', it must be the last value
            valsep = size - 1;
        }

        String cookieName = cookieHeader.subString(i, namesep-i);
        String cookieValue = cookieHeader.subString(namesep+1, valsep-namesep);
        if (name == cookieName)
        {
            value = cookieValue;
            return true;
        }
        i = valsep + 1;
    }
    return false;
};

PEGASUS_NAMESPACE_END
