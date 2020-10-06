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

#include <Pegasus/Common/Tracer.h>
#include <Pegasus/Common/MessageLoader.h>
#include <Pegasus/Common/StringConversion.h>
#include <Pegasus/Common/StrLit.h>
#include <cctype>
#include <cstdio>
#include <time.h>

#include "RsHTTPRequest.h"

PEGASUS_USING_STD;

PEGASUS_NAMESPACE_BEGIN


RsHTTPRequest::RsHTTPRequest(HTTPMessage* httpMessage) :
    _hasType(false),
    _rangeHeaderLoaded(false),
    _first(PEG_NOT_FOUND),
    _last(PEG_NOT_FOUND),
    _content(0)
    {

    // Parse the HTTP message:
    String startLine;

    httpMessage->parse(startLine, _headers, contentLength);

    // Parse the request line:
    String methodName;
    String requestUri;
    String httpVersion;

    HTTPMessage::parseRequestLine(
        startLine, methodName, requestUri, httpVersion);

    PEG_TRACE((TRC_RSSERVER, Tracer::LEVEL4,
        "RsHTTPRequest::RsHTTPRequest(HTTPMessage* httpMessage)- Queue %d: "
            "methodName [%s], requestUri [%s], httpVersion [%s]",
        httpMessage->queueId,
        (const char*)methodName.getCString(),
        (const char*)requestUri.getCString(),
        (const char*)httpVersion.getCString()));


    _uri = RsURI(requestUri);
    if (!RsURI::hasDefaultNamespace())
    {
        RsURI::setDefaultNamespace(_uri.getNamespaceName());
        RsURI::setDefaultNamespaceEncoded(_uri.getNamespaceName(true));
    }

    response = new RsHTTPResponse();

    // Save userName and authType:
    httpCloseConnect = httpMessage->getCloseConnect();
    userName = httpMessage->authInfo->getAuthenticatedUser();
    authType = httpMessage->authInfo->getAuthType();

    method = methodName;
    queueId = httpMessage->queueId;
    ipAddress = httpMessage->ipAddress;
    acceptLanguages = httpMessage->acceptLanguages;
    contentLanguages = httpMessage->contentLanguages;
    message.swap(httpMessage->message);

    // set the pointer to start of data.
    _content = message.getData() + message.size() - contentLength;

}

RsHTTPRequest::~RsHTTPRequest()
{
    delete response;
}

const char *RsHTTPRequest::getContentStart()
{
    return _content;
}

RsURI& RsHTTPRequest::getURI()
{
    return _uri;
}

RsOperationType RsHTTPRequest::getType()
{
    PEG_METHOD_ENTER(TRC_RSSERVER,
        "RsHTTPRequest::getType()");
    if (_hasType)
    {
        return _type;
    }

    if (_uri.hasAssociationPath())
    {
        _type = RS_ASSOCIATION_GET;
    }
    else if (_uri.hasReferencesPath())
    {
        _type = RS_REFERENCE_GET;
    }
    else if (_uri.hasMethodPath())
    {
        _type = RS_METHOD_POST;
    }
    else if (_uri.hasInstancePath())
    {
        _type = RS_INSTANCE_MEMBER_GET;
    }
    else if (_uri.hasEnum())
    {
        _type = RS_INSTANCE_COLLECTION_GET;
    }
    else if (_uri.hasCreate())
    {
        _type = RS_INSTANCE_CREATE_POST;
    }
    else if (_uri.hasClassPath())
    {
        _type = RS_CLASS_MEMBER_GET;
    }
    else if (_uri.hasClassesPath())
    {
        _type = RS_CLASS_COLLECTION_GET;
    }
    else
    {
        _type = RS_CLASS_COLLECTION_GET;
    }

    _hasType = true;
    PEG_METHOD_EXIT();
    return _type;
}


// RFC 2616, Section 14.35.2:
// Range = "Range" ":" ranges-specifier
Boolean RsHTTPRequest::hasRange()
{
    PEG_METHOD_ENTER(TRC_RSSERVER, "RsHTTPRequest::hasRange()");

    if (!_rangeHeaderLoaded)
    {
        _loadRangeHeader();
    }

    PEG_METHOD_EXIT();
    return (_first != PEG_NOT_FOUND && _last != PEG_NOT_FOUND);

}

Uint32 RsHTTPRequest::getRangeStart()
{
    if (!_rangeHeaderLoaded)
    {
        _loadRangeHeader();
    }

    return _first;
}

Uint32 RsHTTPRequest::getRangeEnd()
{
    if (!_rangeHeaderLoaded)
    {
        _loadRangeHeader();
    }

    return _last;
}


void RsHTTPRequest::_loadRangeHeader()
{
    PEG_METHOD_ENTER(TRC_RSSERVER, "RsHTTPRequest::_loadRangeHeader()");

    Boolean found = false;
    for (Uint32 i = 0; i < _headers.size(); i++)
    {
        if (!strcmp("Range", _headers[i].first.getData()))
        {
            // ranges-specifier = "items" "=" item-first "-" item-last
            String range = String(_headers[i].second.getData());
            Uint32 len = range.size();
            Uint32 delimPos = range.find('-');

            if (delimPos != PEG_NOT_FOUND)
            {
                _first = atoi((const char*)range.subString(
                            6, delimPos).getCString());

                _last = atoi((const char*)range.subString(
                            delimPos + 1, len - delimPos + 1).getCString());

                found = true;

                PEG_TRACE((TRC_RSSERVER, Tracer::LEVEL4,
                    "RsHTTPRequest::_loadRangeHeader() found in header %d, "
                    "first: %d, second: %d",
                    i, _first, _last));
            }

            break;
        } // if
    }

    _rangeHeaderLoaded = true;
    PEG_METHOD_EXIT();
}

PEGASUS_NAMESPACE_END
