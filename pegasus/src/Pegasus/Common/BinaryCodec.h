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

#ifndef Pegasus_BinaryCodec_h
#define Pegasus_BinaryCodec_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/Buffer.h>
#include <Pegasus/Common/CIMBuffer.h>
#include <Pegasus/Common/CIMMessage.h>
#include <Pegasus/Common/AcceptLanguageList.h>
#include <Pegasus/Common/ContentLanguageList.h>
#include <Pegasus/Common/Linkage.h>

PEGASUS_NAMESPACE_BEGIN

/** This is the coder-decoder (codec) for the OpenPegasus proprietary binary
    protocol.
*/
class PEGASUS_COMMON_LINKAGE BinaryCodec
{
public:

#if defined(PEGASUS_DEBUG)
    /**
        Peform hex dump of the given data. This is a diagnostic
        tool compiled only in debug mode.
        TODO - There are 3 hex dump functions in pegasus today,
        here, SCMO.cpp and Tracer.  We should consolidate them
    */
    static void hexDump(const void* data, size_t size);
#endif /* defined(PEGASUS_DEBUG) */

    static bool encodeRequest(
        Buffer& out,
        const char* host,
        const String& authenticationHeader,
        CIMOperationRequestMessage* msg,
        bool binaryResponse);

    static bool encodeResponseBody(
        Buffer& out,
        const CIMResponseMessage* msg,
        CIMName& name);

    /**
       Decode a serialized CIMOperationRequestMessage received in a Buffer.
    */
    static CIMOperationRequestMessage* decodeRequest(
        CIMBuffer& in,
        Uint32 queueId,
        Uint32 returnQueueId);

    /**
       Decode an input CIMBuffer containing a serialized message stream
       into a singe CIMResponseMessage.
    */
    static CIMResponseMessage* decodeResponse(
        CIMBuffer& in);

    static Buffer formatSimpleIMethodRspMessage(
        const CIMName& iMethodName,
        const String& messageId,
        HttpMethod httpMethod,
        const ContentLanguageList& httpContentLanguages,
        const Buffer& rtnParams,
        const Buffer& body,
        Uint64 serverResponseTime,
        Boolean isFirst,
        Boolean isLast);

private:


    BinaryCodec();
};

PEGASUS_NAMESPACE_END

#endif /* Pegasus_BinaryCodec_h */
