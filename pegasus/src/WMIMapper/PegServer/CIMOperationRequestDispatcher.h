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
// Author: Mike Brasher (mbrasher@bmc.com)
//
// Modified By: Nitin Upasani, Hewlett-Packard Company (Nitin_Upasani@hp.com)
//              Mike Day (mdday@us.ibm.com)
//              Yi Zhou (yi_zhou@hp.com)
//              Carol Ann Krug Graves, Hewlett-Packard Company
//                  (carolann_graves@hp.com)
//              Roger Kumpf, Hewlett-Packard Company (roger_kumpf@hp.com)
//              Karl Schopmeyer (k.schopmeyer@opengroup.org)
//              Barbara Packard, Hewlett-Packard Company
//                  (barbara_packard@hp.com)
//              Jair Santos, Hewlett-Packard Company (jair.santos@hp.com)
//              Terry Martin, Hewlett-Packard Company (terry.martin@hp.com)
//
//%////////////////////////////////////////////////////////////////////////////

#ifndef PegasusDispatcher_Dispatcher_h
#define PegasusDispatcher_Dispatcher_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/Thread.h>
#include <Pegasus/Common/MessageQueue.h>
#include <Pegasus/Common/CIMMessage.h>
#include <Pegasus/Common/CIMObject.h>
#include <Pegasus/Common/OperationContext.h>
#include <WMIMapper/PegServer/WMIMapperUserInfoContainer.h>
#include <WMIMapper/PegServer/CIMServer.h>
#include <Pegasus/Server/Linkage.h>

PEGASUS_NAMESPACE_BEGIN

class PEGASUS_SERVER_LINKAGE CIMOperationRequestDispatcher :
    public MessageQueueService
{
public:

      typedef MessageQueueService Base;

      CIMOperationRequestDispatcher(
//     CIMRepository* repository,
/*     ProviderRegistrationManager* providerRegistrationManager */);

      virtual ~CIMOperationRequestDispatcher();

      virtual void handleEnqueue(Message *);

      virtual void handleEnqueue();

      void handleGetClassRequest(
     CIMGetClassRequestMessage* request);

      void handleGetInstanceRequest(
     CIMGetInstanceRequestMessage* request);

      void handleDeleteClassRequest(
     CIMDeleteClassRequestMessage* request);

      void handleDeleteInstanceRequest(
     CIMDeleteInstanceRequestMessage* request);

      void handleCreateClassRequest(
     CIMCreateClassRequestMessage* request);

      void handleCreateInstanceRequest(
     CIMCreateInstanceRequestMessage* request);

      void handleModifyClassRequest(
     CIMModifyClassRequestMessage* request);

      void handleModifyInstanceRequest(
     CIMModifyInstanceRequestMessage* request);

      void handleEnumerateClassesRequest(
     CIMEnumerateClassesRequestMessage* request);

      void handleEnumerateClassNamesRequest(
     CIMEnumerateClassNamesRequestMessage* request);

      void handleEnumerateInstancesRequest(
     CIMEnumerateInstancesRequestMessage* request);

      void handleEnumerateInstanceNamesRequest(
     CIMEnumerateInstanceNamesRequestMessage* request);

      void handleAssociatorsRequest(
     CIMAssociatorsRequestMessage* request);

      void handleAssociatorNamesRequest(
     CIMAssociatorNamesRequestMessage* request);

      void handleReferencesRequest(
     CIMReferencesRequestMessage* request);

      void handleReferenceNamesRequest(
     CIMReferenceNamesRequestMessage* request);

      void handleGetPropertyRequest(
     CIMGetPropertyRequestMessage* request);

      void handleSetPropertyRequest(
     CIMSetPropertyRequestMessage* request);

      void handleGetQualifierRequest(
     CIMGetQualifierRequestMessage* request);

      void handleSetQualifierRequest(
     CIMSetQualifierRequestMessage* request);

      void handleDeleteQualifierRequest(
     CIMDeleteQualifierRequestMessage* request);

      void handleEnumerateQualifiersRequest(
     CIMEnumerateQualifiersRequestMessage* request);

      void handleExecQueryRequest(
     CIMExecQueryRequestMessage* request);

      void handleInvokeMethodRequest(
     CIMInvokeMethodRequestMessage* request);

protected:

      void _enqueueResponse(
     CIMRequestMessage* request, CIMResponseMessage* response);

      CIMValue _convertValueType(const CIMValue& value, CIMType type);

      void _fixInvokeMethodParameterTypes(
          CIMInvokeMethodRequestMessage* request);

      void _fixSetPropertyValueType(CIMSetPropertyRequestMessage* request);

//      CIMRepository * _repository;

//      ProviderRegistrationManager* _providerRegistrationManager;

      AtomicInt _dying;

      Boolean _enableAssociationTraversal;
      Boolean _enableIndicationService;

   private:
      void CIMOperationRequestDispatcher::_handleOperationMessage(
          Message *request);

      static ThreadReturnType PEGASUS_THREAD_CDECL _callMethodHandler(
          void *parm);
};

PEGASUS_NAMESPACE_END

#endif /* PegasusDispatcher_Dispatcher_h */
