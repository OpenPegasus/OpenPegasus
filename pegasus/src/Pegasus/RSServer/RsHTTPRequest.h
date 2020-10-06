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

#ifndef Pegasus_RsHTTPRequest_h
#define Pegasus_RsHTTPRequest_h

#include <Pegasus/RSServer/Linkage.h>
#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/OperationContext.h>
#include <Pegasus/Common/AcceptLanguageList.h>
#include <Pegasus/Common/ContentLanguageList.h>
#include <Pegasus/Common/ArrayInternal.h>
#include <Pegasus/Common/HTTPMessage.h>
#include <Pegasus/Common/Buffer.h>

#include "RsURI.h"
#include "RsHTTPResponse.h"

PEGASUS_NAMESPACE_BEGIN

enum RsOperationType
{
    RS_CLASS_COLLECTION_GET,
    RS_CLASS_MEMBER_GET,
    RS_INSTANCE_COLLECTION_GET,
    RS_INSTANCE_MEMBER_GET,
    RS_ASSOCIATION_GET,
    RS_REFERENCE_GET,
    RS_INSTANCE_CREATE_POST,
    RS_METHOD_POST
};


class PEGASUS_RSSERVER_LINKAGE RsHTTPRequest
{
public:

    RsHTTPRequest(HTTPMessage*);
    virtual ~RsHTTPRequest();

    RsOperationType getType();

    RsURI& getURI();

    Boolean hasRange();
    Uint32 getRangeStart();
    Uint32 getRangeEnd();
    const char *getContentStart();

    // copied from WsmRequest.h
    String method;
    String authType;
    String userName;
    AcceptLanguageList acceptLanguages;
    ContentLanguageList contentLanguages;
    Boolean httpCloseConnect;
    Uint32 queueId;
    String ipAddress;
    HttpMethod httpMethod;
    Uint32 contentLength;
    Buffer message;


    RsHTTPResponse* response;
private:
    Array<HTTPHeader> _headers;
    RsURI _uri;
    RsOperationType _type;
    Boolean _hasType;

    void _loadRangeHeader();
    // Only for Range retrieval requests
    Boolean _rangeHeaderLoaded;
    Uint32 _first;
    Uint32 _last;
    const char* _content;

};


PEGASUS_NAMESPACE_END

#endif /* Pegasus_RsRequest_h */
