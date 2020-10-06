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

#ifndef Pegasus_WsmResponseEncoder_h
#define Pegasus_WsmResponseEncoder_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/WsmServer/WsmResponse.h>

PEGASUS_NAMESPACE_BEGIN

class WsmResponse;
class SoapResponse;

/** This class encodes WS-Man operation requests and passes them up-stream.
 */
class WsmResponseEncoder
{
public:

    WsmResponseEncoder();
    ~WsmResponseEncoder();

    void enqueue(WsmResponse* response);

    // The encoding of enumeration responses must be separated from the
    // sending of the responses to allow proper handling of MaxEnvelopeSize.
    SoapResponse* encodeWsenEnumerateResponse(
        WsenEnumerateResponse* response,
        Uint32& numDataItemsEncoded);
    SoapResponse* encodeWsenPullResponse(
        WsenPullResponse* response,
        Uint32& numDataItemsEncoded);
    void sendResponse(SoapResponse* response)
    {
        _sendResponse(response);
    }

private:

    void _sendResponse(SoapResponse* response);
    void _sendUnreportableSuccess(WsmResponse* response);
    SoapResponse* _buildEncodingLimitFault(WsmResponse* response);

    void _encodeWxfGetResponse(WxfGetResponse* response);
    void _encodeWxfPutResponse(WxfPutResponse* response);
    void _encodeWxfCreateResponse(WxfCreateResponse* response);
    void _encodeWxfSubCreateResponse(WxfSubCreateResponse* response);
    void _encodeWxfDeleteResponse(WxfDeleteResponse* response);
    void _encodeWxfSubDeleteResponse(WxfSubDeleteResponse* response);
    void _encodeWsenReleaseResponse(WsenReleaseResponse* response);
    void _encodeWsmFaultResponse(WsmFaultResponse* response);
    void _encodeSoapFaultResponse(SoapFaultResponse* response);
    void _encodeWsInvokeResponse(WsInvokeResponse* response);

    Boolean _encodeEnumerationData(
        SoapResponse& soapResponse,
        Buffer& headers,
        WsmOperationType operation,
        Uint64 contextId,
        Boolean isComplete,
        WsenEnumerationData& data,
        Uint32& numDataItemsEncoded,
        const String& resourceUri);
};

PEGASUS_NAMESPACE_END

#endif /* Pegasus_WsmResponseEncoder_h */
