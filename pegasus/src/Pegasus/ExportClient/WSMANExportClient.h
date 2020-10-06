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

#ifndef Pegasus_WSMANExportClient_h
#define Pegasus_WSMANExportClient_h

#include <Pegasus/ExportClient/ExportClient.h>

PEGASUS_NAMESPACE_BEGIN


/**
    This class provides the interface that a CIMOM uses to communicate
    with a client or listner.
*/
class PEGASUS_EXPORT_CLIENT_LINKAGE WSMANExportClient : public ExportClient
{
public:
      
    /**
        Constructor for a WSMAN Export Client object.
    */
    WSMANExportClient(
        HTTPConnector* httpConnector,
        Monitor* _monitor,
        Uint32 timeoutMilliseconds =
            PEGASUS_DEFAULT_CLIENT_TIMEOUT_MILLISECONDS);

    // Destructor for a WSMAN Export Client object.
    ~WSMANExportClient();
    
     /**
        Send indication message to the destination where the url input
        parameter defines the destination.

        @param url String defining the destination of the indication to be sent.
        @param instance CIMInstance is the indication instance which needs to
        be sent to the destination.
        @param contentLanguages The language of the indication
    */
    void exportIndication(
        const String& url,
        const CIMInstance& instance,
        const ContentLanguageList& contentLanguages = ContentLanguageList(),
        const String& toPath = "");

    void setDeliveryMode(deliveryMode &deliveryMode);

private:

   Boolean  _doRequest(
        WsmRequest * request,
        WsmOperationType expectedResponseMessageType);

    deliveryMode _deliveryMode;
};

PEGASUS_NAMESPACE_END

#endif /* Pegasus_WSMANExportClient_h */
