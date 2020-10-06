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

#ifndef Pegasus_SoapResponse_h
#define Pegasus_SoapResponse_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/ContentLanguageList.h>
#include <Pegasus/WsmServer/WsmResponse.h>
#include <Pegasus/WsmServer/WsmUtils.h>

PEGASUS_NAMESPACE_BEGIN

class PEGASUS_WSMSERVER_LINKAGE SoapResponse
{
public:

    SoapResponse(WsmResponse* response);

    Uint32 getEnvelopeSize()
    {
        return _envStart.size() + _envEnd.size() +
            _hdrStart.size() + _hdrEnd.size() + _hdrContent.size() +
            _bodyStart.size() + _bodyEnd.size() + _bodyHeader.size() +
            _bodyContent.size() + _bodyTrailer.size();
    }

    Boolean getHttpCloseConnect()
    {
        return _httpCloseConnect;
    }

    Uint32 getQueueId()
    {
        return _queueId;
    }

    Buffer getResponseContent();

    Boolean appendHeader(Buffer& buf);
    Boolean appendBodyContent(Buffer& buf);
    Boolean appendBodyHeader(Buffer& buf);
    Boolean appendBodyTrailer(Buffer& buf);

    Buffer& getBodyHeader()
    {
        return _bodyHeader;
    }

    Buffer& getBodyTrailer()
    {
        return _bodyTrailer;
    }

private:

    Uint32 _maxEnvelopeSize;
    Boolean _httpCloseConnect;
    Uint32 _queueId;

    Buffer _httpHeader;
    Buffer _envStart, _envEnd;
    Buffer _hdrStart, _hdrEnd, _hdrContent;
    Buffer _bodyStart, _bodyEnd, _bodyHeader, _bodyContent, _bodyTrailer;
};

PEGASUS_NAMESPACE_END

#endif /* Pegasus_SoapResponse_h */
