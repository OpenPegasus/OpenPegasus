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

#ifndef Pegasus_WebRequest_h
#define Pegasus_WebRequest_h

#include <Pegasus/Common/HTTPMessage.h>

PEGASUS_NAMESPACE_BEGIN


/**
 * Container for the request values of interest for the WebProcessor.
 */
class PEGASUS_WEBSERVER_LINKAGE WebRequest
{

public:

    /**
     * Constructor.
     *
     * @param queueId
     *      Request's queueId required for generation of the response
     *             for a successful delivery back to the requester.
     */
    WebRequest(Uint32 queueId)
        :_queueId(queueId)
    {
    }


    /**
     * Destructor.
     */
    ~WebRequest()
    {
    }

    Uint32 getQueueId() const
    {
        return _queueId;
    }


    /**
     * URI of the request.
     */
    String requestURI;

    /**
     * HTTP version.
     */
    String httpVersion;

    /**
     * Method of the request.
     */
    HttpMethod httpMethod;

    /*
     * HTTP-header: Accept
     */
    String mimeTypes;

    /*
     * HTTP-header: Accept-Charset
     */
    String charSets;

    /*
     * HTTP-header: Accept-Encoding
     */
    String encodings;

    /*
     * Type of user-authentication, currently not used.
     */
    String authType;

    /*
     * Username/requester, currently not used.
     */
    String userName;

    /*
     * List of languages acceptable for a response, currently not used
     */
    AcceptLanguageList acceptLanguages;

    /**
     * IP-Address, currently not used.
     */
    String ipAddress;



private:

    /**
     * QueueID of request, required for a successful delivery of the response.
     */
    Uint32 _queueId;
};

PEGASUS_NAMESPACE_END

#endif /* Pegasus_WebRequest_h */
