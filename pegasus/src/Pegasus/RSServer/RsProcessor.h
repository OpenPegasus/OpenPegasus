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

#ifndef Pegasus_RsProcessor_h
#define Pegasus_RsProcessor_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/MessageQueueService.h>
#include <Pegasus/Common/CIMMessage.h>
#include <Pegasus/Repository/CIMRepository.h>

#include "RsRequestDecoder.h"
#include "RsHTTPRequest.h"
#include "Linkage.h"

// Dummy value; HTTP responses are always
// unique to their request based on client socket
#define CIMRS_MESSAGE_ID "1000"

PEGASUS_NAMESPACE_BEGIN

/**
    Processes RsRequest messages and produces RsResponse messages.
 */
class PEGASUS_RSSERVER_LINKAGE RsProcessor :
    public MessageQueue
{
public:

    RsProcessor(
        MessageQueue* cimOperationProcessorQueue,
        CIMRepository* repository);

    ~RsProcessor();

    virtual void enqueue(Message*);
    virtual void handleEnqueue(Message *);

    virtual void handleEnqueue();

    void handleRequest(RsHTTPRequest*);
    void handleResponse(CIMResponseMessage*);

    Uint32 getRsRequestDecoderQueueId();

    // Sets the flag to indicate whether or not the CIMServer is shutting down.
    void setServerTerminating(Boolean flag)
    {
        _serverTerminating = flag;
    }

private:

    String getParamValues(CIMConstMethod& method,
                          Array<CIMParamValue>& inParms,
                          const char *content,
                          Uint32 contenSize,
                          CIMRepository* repository,
                          RsURI& reqUri);

    RsRequestDecoder _rsRequestDecoder;

    typedef HashTable<Uint32,
    RsHTTPRequest*, EqualFunc<Uint32>, HashFunc<Uint32> > RequestTable;

    RequestTable _requestTable;

    MessageQueue* _cimOperationProcessorQueue;
    CIMRepository* _repository;

    // Flag to indicate whether or not the CIMServer is shutting down.
    Boolean _serverTerminating;
};

PEGASUS_NAMESPACE_END

#endif /* Pegasus_RsProcessor_h */
