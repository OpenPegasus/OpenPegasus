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
#include <Pegasus/Common/Tracer.h>
#include <Pegasus/Common/MessageLoader.h>
#include <Pegasus/Common/StringConversion.h>
#include <Pegasus/Common/StrLit.h>
#include <Pegasus/Common/CIMNameCast.h>
#include <Pegasus/Common/Constants.h>
#include <cctype>
#include <cstdio>
#include <time.h>

#include "RsHTTPResponse.h"

PEGASUS_USING_STD;

PEGASUS_NAMESPACE_BEGIN


RsHTTPResponse::RsHTTPResponse():
    _message(4096),
    _hasWriter(false)
{
}


RsHTTPResponse::~RsHTTPResponse()
{
    if (_hasWriter)
    {
        delete _writer;
    }
}

void RsHTTPResponse::setStatus(const char* status, Uint32 length)
{
    PEG_METHOD_ENTER(TRC_RSSERVER, "RsHTTPResponse::setStatus()");
    if (_message.size() > 0)
    {
        _message.clear();
    }

    _message.append(STRLIT_ARGS("HTTP/1.1 "));
    _message.append(status, length);
    _message.append(STRLIT_ARGS("\r\n"));

    PEG_TRACE((TRC_RSSERVER, Tracer::LEVEL4,
        "RsHTTPResponse::setStatus() Status Line: [%s]",
        _message.getData()));

    // Date is MUST per rfc 2616
    // for format see section 3.3.3.1 / rfc 1123, e.g.
    // Sun, 06 Nov 1994 08:49:37 GMT
    time_t currentTime;
    struct tm* gmtTime;
    char timeValue[30];
    time(&currentTime);
    gmtTime = gmtime(&currentTime);
    strftime(timeValue,30,"%a, %d %b %Y %H:%M:%S GMT",gmtTime);
    _message.append(STRLIT_ARGS("Date: "));
    _message.append(timeValue, 29);

    _message.append(STRLIT_ARGS("\r\n" "Content-Length: "));
    _contentLengthPos = _message.size();
    _message.append(STRLIT_ARGS("\r\n"));

    PEG_METHOD_EXIT();
}



void RsHTTPResponse::setRange(Uint32 start, Uint32 end, Uint32 total)
{
    char startc[22];
    Uint32 startSize;
    char endc[22];
    Uint32 endSize;
    char totalc[22];
    Uint32 totalSize;

    const char* startRange = Uint32ToString(startc, start, startSize);
    const char* endRange = Uint32ToString(endc, end, endSize);
    const char* totalRange = Uint32ToString(totalc, total, totalSize);

    Buffer contentRange(2048);
    contentRange.append(STRLIT_ARGS("Content-Range: items "));
    contentRange.append(startRange, startSize);
    contentRange.append('-');
    contentRange.append(endRange, endSize);
    contentRange.append('/');
    contentRange.append(totalRange, totalSize);
    contentRange.append(STRLIT_ARGS("\r\n"));

    _message.insert(_headerLength, contentRange.getData(),
        contentRange.size());

    PEGASUS_ASSERT(_headerLength > 0);
    _headerLength += contentRange.size();
}



void RsHTTPResponse::setCloseConnect(Boolean closeConnect)
{
    _closeConnect = closeConnect;
}


JSONWriter* RsHTTPResponse::getJSONWriter()
{
    if (!_hasWriter)
    {
        _headerLength = _message.size();

        _message.append(STRLIT_ARGS("\r\n"));

        _writer = new JSONWriter(_message);
        PEG_TRACE((TRC_RSSERVER, Tracer::LEVEL4,
                "RsHTTPResponse::getJSONWriter() Created Writer"));
        _hasWriter = true;
    }

    return _writer;
}

HTTPMessage* RsHTTPResponse::getHTTPMessage()
{
    char scratch[22];
    Uint32 contentLengthSize = 0;

    const char* contentLength = Uint32ToString( scratch,
            _message.size() - (_headerLength + 2),
            contentLengthSize);

    // Carve in body length
    _message.insert(_contentLengthPos, contentLength, contentLengthSize);

    return new HTTPMessage(_message);
}

void RsHTTPResponse::setContentType(const char* type, Uint32 length)
{
    _message.append(STRLIT_ARGS("Content-Type: "));
    _message.append(type, length);
    _message.append(STRLIT_ARGS("\r\n"));
}


PEGASUS_NAMESPACE_END
