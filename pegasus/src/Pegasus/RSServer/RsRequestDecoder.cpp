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

#include <Pegasus/Common/Constants.h>
#include <cctype>
#include <cstdio>
#include <Pegasus/Common/System.h>
#include <Pegasus/Common/Logger.h>
#include <Pegasus/Common/Tracer.h>
#include <Pegasus/Common/StatisticalData.h>
#include <Pegasus/Common/CommonUTF.h>
#include <Pegasus/Common/MessageLoader.h>


#include "RsProcessor.h"
#include "RsRequestDecoder.h"
#include "RsURI.h"

PEGASUS_USING_STD;

PEGASUS_NAMESPACE_BEGIN

RsRequestDecoder::RsRequestDecoder(RsProcessor* rsProcessor)
    : MessageQueueService("RsRequestDecoder"),
    _rsProcessor(rsProcessor)
{
}

RsRequestDecoder::~RsRequestDecoder()
{
}

void RsRequestDecoder::handleEnqueue(Message* message)
{
    PEG_METHOD_ENTER(TRC_RSSERVER,"RsOperationRequestDecoder::handleEnqueue()");
    if (!message)
    {
        PEG_METHOD_EXIT();
        return;
    }

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
    PEG_METHOD_EXIT();
}


void RsRequestDecoder::handleEnqueue()
{
    PEG_METHOD_ENTER(TRC_RSSERVER,"RsOperationRequestDecoder::handleEnqueue()");
    Message* message = dequeue();
    if (message)
    {
        handleEnqueue(message);
    }
    PEG_METHOD_EXIT();
}

void RsRequestDecoder::handleHTTPMessage(HTTPMessage* httpMessage)
{
    PEG_METHOD_ENTER(TRC_DISPATCHER,
        "RsOperationRequestDecoder::handleHTTPMessage()");

    _rsProcessor->handleRequest(new RsHTTPRequest(httpMessage));

    PEG_METHOD_EXIT();
}

PEGASUS_NAMESPACE_END
