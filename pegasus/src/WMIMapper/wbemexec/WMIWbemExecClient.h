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

#ifndef Pegasus_WMIWbemExecClient_h
#define Pegasus_WMIWbemExecClient_h
#include "WbemExecClient.h"

PEGASUS_NAMESPACE_BEGIN

/** This class extends the WbemExecClient class to use the WMI Mapper 
  for local connections on Windows.
*/
class WMIWbemExecClient : public WbemExecClient
{
public:

    ~WMIWbemExecClient();

    void disconnect();
    /** connectLocal - Sets up local connections to run through direct
        WMI Mapper library calls.  
        @return - No return defined. Failure to connect throws an exception.
        @SeeAlso connect - The exceptions are defined in connect.
    */
    void connectLocal()
        throw(AlreadyConnectedException, InvalidLocatorException,
              CannotCreateSocketException, CannotConnectException);

    /** issueRequest - Sends local request messages through WMI Mapper library 
        interfaces.  For remote requests, the base WbemExecClient::issueRequest
        is called.
        @return - Response message as Array<Sint8>.
    */
    Buffer issueRequest(
        const Buffer& request
    ) throw(NotConnectedException, ConnectionTimeoutException,
            UnauthorizedAccess);
};

PEGASUS_NAMESPACE_END

#endif /* Pegasus_WMIWbemExecClient_h */
#endif // PEGASUS_WMIMAPPER
