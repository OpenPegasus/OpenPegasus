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

#ifndef Pegasus_CIMExportRequestEncoder_h
#define Pegasus_CIMExportRequestEncoder_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/MessageQueue.h>
#include <Pegasus/Common/CIMMessage.h>
#include <Pegasus/Common/AutoPtr.h>
#include <Pegasus/Client/ClientAuthenticator.h>
#include <Pegasus/ExportClient/Linkage.h>

PEGASUS_NAMESPACE_BEGIN

/** This class receives CIM Operation Request messages on its input queue
    and encodes them into HTTP messages which it places on its output queue.
*/
class PEGASUS_EXPORT_CLIENT_LINKAGE CIMExportRequestEncoder
   : public MessageQueue
{
   public:
      /** Constuctor.
          @param outputQueue queue to receive encoded HTTP messages.
          @param hostName Name of the target host for the encoded requests.
                          I.e., the value of the HTTP Host header.
          @param authenticator client authenticator.
      */
      CIMExportRequestEncoder(
         MessageQueue* outputQueue,
         const String& hostName,
         ClientAuthenticator* authenticator);

      /** Destructor. */
      ~CIMExportRequestEncoder();

      /** This method is called when a message is enqueued on this queue. */
      virtual void handleEnqueue();

   private:

      void _encodeExportIndicationRequest(
            CIMExportIndicationRequestMessage* message);

      MessageQueue* _outputQueue;
      CString _hostName;
      AutoPtr<ClientAuthenticator> _authenticator; //PEP101
};

PEGASUS_NAMESPACE_END

#endif /* Pegasus_CIMExportRequestEncoder_h */
