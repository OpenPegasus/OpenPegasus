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
#ifndef Pegasus_WSMANExportRequestEncoder_h
#define Pegasus_WSMANExportRequestEncoder_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/MessageQueue.h>
#include <Pegasus/Common/CIMMessage.h>
#include <Pegasus/Common/AutoPtr.h>
#include <Pegasus/Client/ClientAuthenticator.h>
#include <Pegasus/ExportClient/Linkage.h>
#include <Pegasus/WsmServer/WsmRequest.h>

PEGASUS_NAMESPACE_BEGIN

class PEGASUS_EXPORT_CLIENT_LINKAGE WSMANExportRequestEncoder
   : public MessageQueue
{
    public:
        /** Constuctor.
          @param outputQueue queue to receive encoded HTTP messages.
          @param hostName Name of the target host for the encoded requests.
                          I.e., the value of the HTTP Host header.
          @param authenticator client authenticator.
        */
        WSMANExportRequestEncoder(
            MessageQueue* outputQueue,
            const String& hostName,
            const String& portId,
            ClientAuthenticator* authenticator);        

        /** Destructor. */
        ~WSMANExportRequestEncoder();
        
        /** This method is called when a message is enqueued on this queue. */
        virtual void handleEnqueue();
       
        void setDeliveryMode(deliveryMode &deliveryMode);

    private:
        
        void _encodeExportIndicationRequest(
            WsExportIndicationRequest * message);
        void _encodeWSMANIndication( 
            WsExportIndicationRequest * message,
            Buffer &out);
       
        MessageQueue* _outputQueue;
        CString _hostName;
        CString _portNumber;
        AutoPtr<ClientAuthenticator> _authenticator; 
        deliveryMode _deliveryMode;
        friend class TestRequestEncoder;
};

PEGASUS_NAMESPACE_END

#endif /* Pegasus_WSMANExportRequestEncoder_h */



