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
// NOTE: this code is intended to be used only with the WMI Mapper 
// on WIN32_IX86_MSVC platforms.
//
#ifdef PEGASUS_WMIMAPPER

#include "WMIWbemExecClient.h"
#include <WMIMapper/PegServer/LocalCIMServer.h>

PEGASUS_USING_STD;

PEGASUS_NAMESPACE_BEGIN


WMIWbemExecClient::~WMIWbemExecClient()
{
    disconnect();
}

void WMIWbemExecClient::connectLocal()
    throw(AlreadyConnectedException, InvalidLocatorException,
          CannotCreateSocketException, CannotConnectException)
{
    //
    // If already connected, bail out!
    //
    if (_connected)
        throw AlreadyConnectedException();

    // As local connections will use direct WMI Mapper function calls,
    // no setup for connectiity is required.
    _connected = true;
    _isRemote = false;
}

void WMIWbemExecClient::disconnect()
{
    if (_connected)
    {
        _connected = false;
    }
}

Buffer WMIWbemExecClient::issueRequest(const Buffer& request) 
    throw(NotConnectedException, ConnectionTimeoutException, UnauthorizedAccess)
{
    if (!_connected)
    {
        throw NotConnectedException();
    }

    // Use WMI Mapper interfaces for local transactions
    if (_isRemote) 
    {
        //
        // For remote operations, use the standard WbemExecClient functionality
        //
        return WbemExecClient::issueRequest(request);    
    }
    else
    {
        //
        // For local operations, use a local WMI Mapper object:
        //
        LocalCIMServer localMapper;

        return localMapper.handleHTTPRequest(request);
    }
}

PEGASUS_NAMESPACE_END

#endif // PEGASUS_WMIMAPPER
