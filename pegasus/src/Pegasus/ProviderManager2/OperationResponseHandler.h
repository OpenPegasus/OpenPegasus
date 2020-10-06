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

#ifndef Pegasus_OperationResponseHandler_h
#define Pegasus_OperationResponseHandler_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/CIMMessage.h>
#include <Pegasus/Common/MessageQueueService.h>
#include <Pegasus/Common/Constants.h>

#include <Pegasus/Common/CIMClass.h>
#include <Pegasus/Common/CIMInstance.h>
#include <Pegasus/Common/CIMIndication.h>
#include <Pegasus/Common/CIMValue.h>

#include <Pegasus/Common/OperationContext.h>
#include <Pegasus/Common/OperationContextInternal.h>

#include <Pegasus/Common/ObjectNormalizer.h>

#include <Pegasus/Common/ResponseHandler.h>
#include <Pegasus/ProviderManager2/SimpleResponseHandler.h>

#include <Pegasus/ProviderManager2/Linkage.h>

PEGASUS_NAMESPACE_BEGIN

typedef void (*PEGASUS_RESPONSE_CHUNK_CALLBACK_T)(
    CIMRequestMessage* request, CIMResponseMessage* response);

class PEGASUS_PPM_LINKAGE OperationResponseHandler
{
    friend class SimpleResponseHandler;

public:
    OperationResponseHandler(
        CIMRequestMessage* request,
        CIMResponseMessage* response,
        PEGASUS_RESPONSE_CHUNK_CALLBACK_T responseChunkCallback);

    virtual ~OperationResponseHandler();

    CIMRequestMessage* getRequest() const;

    CIMResponseMessage* getResponse() const;

    virtual void setStatus(
        const Uint32 code,
        const String& message = String::EMPTY);

    virtual void setStatus(
        const Uint32 code,
        const ContentLanguageList& langs,
        const String& message = String::EMPTY);

    virtual void setCIMException(const CIMException& cimException);

protected:
    // the default for all derived handlers. Some handlers may not apply
    // async behavior because their callers cannot handle partial responses.
    virtual Boolean isAsync() const;

    // send (deliver) asynchronously
    virtual void send(Boolean isComplete);

    // transfer any objects from handler to response. this does not clear()
    virtual void transfer();

    // validate whatever is necessary before the transfer
    virtual void validate();

    virtual String getClass() const;

    Uint32 getResponseObjectTotal() const;

    // there can be many objects per message (or none at all - i.e complete())
    Uint32 getResponseMessageTotal() const;

    Uint32 getResponseObjectThreshold() const;

    CIMRequestMessage* _request;
    CIMResponseMessage* _response;
    PEGASUS_RESPONSE_CHUNK_CALLBACK_T _responseChunkCallback;

private:
    Uint32 _responseObjectTotal;
    Uint32 _responseMessageTotal;
    Uint32 _responseObjectThreshold;

};

class PEGASUS_PPM_LINKAGE GetInstanceResponseHandler :
    public OperationResponseHandler, public SimpleInstanceResponseHandler
{
public:
    GetInstanceResponseHandler(
        CIMGetInstanceRequestMessage* request,
        CIMGetInstanceResponseMessage* response,
        PEGASUS_RESPONSE_CHUNK_CALLBACK_T responseChunkCallback);

    virtual void deliver(const CIMInstance& cimInstance);
    virtual void deliver(const Array<CIMInstance>& cimInstanceArray)
    {
        SimpleInstanceResponseHandler::deliver(cimInstanceArray);
    }
    virtual void deliver(const SCMOInstance& cimInstance);

    virtual void complete();

protected:
    virtual String getClass() const;

    virtual void transfer();

    virtual void validate();

private:
    ObjectNormalizer _normalizer;

};

class PEGASUS_PPM_LINKAGE EnumerateInstancesResponseHandler :
    public OperationResponseHandler, public SimpleInstanceResponseHandler
{
public:
    EnumerateInstancesResponseHandler(
        CIMEnumerateInstancesRequestMessage* request,
        CIMEnumerateInstancesResponseMessage* response,
        PEGASUS_RESPONSE_CHUNK_CALLBACK_T responseChunkCallback);

    virtual void deliver(const CIMInstance& cimInstance);
    virtual void deliver(const Array<CIMInstance>& cimInstanceArray)
    {
        SimpleInstanceResponseHandler::deliver(cimInstanceArray);
    }
    virtual void deliver(const SCMOInstance& scmoInstance);

protected:
    virtual String getClass() const;

    virtual void transfer();

private:
    ObjectNormalizer _normalizer;

};

class PEGASUS_PPM_LINKAGE EnumerateInstanceNamesResponseHandler :
    public OperationResponseHandler, public SimpleObjectPathResponseHandler
{
public:
    EnumerateInstanceNamesResponseHandler(
        CIMEnumerateInstanceNamesRequestMessage* request,
        CIMEnumerateInstanceNamesResponseMessage* response,
        PEGASUS_RESPONSE_CHUNK_CALLBACK_T responseChunkCallback);

    virtual void deliver(const CIMObjectPath& cimObjectPath);
    virtual void deliver(const Array<CIMObjectPath>& cimObjectPathArray)
    {
        SimpleObjectPathResponseHandler::deliver(cimObjectPathArray);
    }
    virtual void deliver(const SCMOInstance& scmoObjectPath);

protected:
    virtual String getClass() const;

    virtual void transfer();

private:
    ObjectNormalizer _normalizer;

};

class PEGASUS_PPM_LINKAGE CreateInstanceResponseHandler :
    public OperationResponseHandler, public SimpleObjectPathResponseHandler
{
public:
    CreateInstanceResponseHandler(
        CIMCreateInstanceRequestMessage* request,
        CIMCreateInstanceResponseMessage* response,
        PEGASUS_RESPONSE_CHUNK_CALLBACK_T responseChunkCallback);

    virtual void deliver(const CIMObjectPath& cimObjectPath);
    virtual void deliver(const Array<CIMObjectPath>& cimObjectPathArray)
    {
        SimpleObjectPathResponseHandler::deliver(cimObjectPathArray);
    }

    virtual void complete();

protected:
    virtual String getClass() const;

    virtual void transfer();

};

class PEGASUS_PPM_LINKAGE ModifyInstanceResponseHandler :
    public OperationResponseHandler, public SimpleResponseHandler
{
public:
    ModifyInstanceResponseHandler(
        CIMModifyInstanceRequestMessage* request,
        CIMModifyInstanceResponseMessage* response,
        PEGASUS_RESPONSE_CHUNK_CALLBACK_T responseChunkCallback);

protected:
    virtual String getClass() const;

};

class PEGASUS_PPM_LINKAGE DeleteInstanceResponseHandler :
    public OperationResponseHandler, public SimpleResponseHandler
{
public:
    DeleteInstanceResponseHandler(
        CIMDeleteInstanceRequestMessage* request,
        CIMDeleteInstanceResponseMessage* response,
        PEGASUS_RESPONSE_CHUNK_CALLBACK_T responseChunkCallback);

protected:
    virtual String getClass() const;

};

class PEGASUS_PPM_LINKAGE GetPropertyResponseHandler :
    public OperationResponseHandler, public SimpleValueResponseHandler
{
public:
    GetPropertyResponseHandler(
        CIMGetPropertyRequestMessage* request,
        CIMGetPropertyResponseMessage* response,
        PEGASUS_RESPONSE_CHUNK_CALLBACK_T responseChunkCallback);

    virtual void deliver(const CIMValue& cimValue);
    virtual void deliver(const Array<CIMValue>& cimValueArray)
    {
        SimpleValueResponseHandler::deliver(cimValueArray);
    }

protected:
    virtual String getClass() const;

    virtual void transfer();

    virtual void validate();

};

class PEGASUS_PPM_LINKAGE SetPropertyResponseHandler :
    public OperationResponseHandler, public SimpleResponseHandler
{
public:
    SetPropertyResponseHandler(
        CIMSetPropertyRequestMessage* request,
        CIMSetPropertyResponseMessage* response,
        PEGASUS_RESPONSE_CHUNK_CALLBACK_T responseChunkCallback);

protected:
    virtual String getClass() const;

};

class PEGASUS_PPM_LINKAGE ExecQueryResponseHandler :
    public OperationResponseHandler,
    public SimpleInstance2ObjectResponseHandler
{
public:
    ExecQueryResponseHandler(
        CIMExecQueryRequestMessage* request,
        CIMExecQueryResponseMessage* response,
        PEGASUS_RESPONSE_CHUNK_CALLBACK_T responseChunkCallback);

    virtual void deliver(const CIMInstance& cimInstance);
    virtual void deliver(const Array<CIMInstance>& cimInstanceArray)
    {
        SimpleInstance2ObjectResponseHandler::deliver(cimInstanceArray);
    }
    virtual void deliver(const SCMOInstance& scmoInstance);

protected:
    virtual String getClass() const;

    virtual void transfer();

    virtual Boolean isAsync() const;

    CIMClass _cimClass;

};

class PEGASUS_PPM_LINKAGE AssociatorsResponseHandler :
    public OperationResponseHandler, public SimpleObjectResponseHandler
{
public:
    AssociatorsResponseHandler(
        CIMAssociatorsRequestMessage* request,
        CIMAssociatorsResponseMessage* response,
        PEGASUS_RESPONSE_CHUNK_CALLBACK_T responseChunkCallback);

    virtual void deliver(const CIMObject& cimObject);
    virtual void deliver(const CIMInstance& cimInstance);
    virtual void deliver(const Array<CIMObject>& cimObjectArray)
    {
        SimpleObjectResponseHandler::deliver(cimObjectArray);
    }
    virtual void deliver(const SCMOInstance& scmoObject);

protected:
    virtual String getClass() const;

    virtual void transfer();

};

class PEGASUS_PPM_LINKAGE AssociatorNamesResponseHandler :
    public OperationResponseHandler, public SimpleObjectPathResponseHandler
{
public:
    AssociatorNamesResponseHandler(
        CIMAssociatorNamesRequestMessage* request,
        CIMAssociatorNamesResponseMessage* response,
        PEGASUS_RESPONSE_CHUNK_CALLBACK_T responseChunkCallback);

    virtual void deliver(const CIMObjectPath& cimObjectPath);
    virtual void deliver(const Array<CIMObjectPath>& cimObjectPathArray)
    {
        SimpleObjectPathResponseHandler::deliver(cimObjectPathArray);
    }
    virtual void deliver(const SCMOInstance& scmoObjectPath);

protected:
    virtual String getClass() const;

    virtual void transfer();

};

class PEGASUS_PPM_LINKAGE ReferencesResponseHandler :
    public OperationResponseHandler, public SimpleObjectResponseHandler
{
public:
    ReferencesResponseHandler(
        CIMReferencesRequestMessage* request,
        CIMReferencesResponseMessage* response,
        PEGASUS_RESPONSE_CHUNK_CALLBACK_T responseChunkCallback);

    virtual void deliver(const CIMObject& cimObject);
    virtual void deliver(const Array<CIMObject>& cimObjectArray)
    {
        SimpleObjectResponseHandler::deliver(cimObjectArray);
    }
    virtual void deliver(const SCMOInstance& scmoObject);

protected:
    virtual String getClass() const;

    virtual void transfer();

};

class PEGASUS_PPM_LINKAGE ReferenceNamesResponseHandler :
    public OperationResponseHandler, public SimpleObjectPathResponseHandler
{
public:
    ReferenceNamesResponseHandler(
        CIMReferenceNamesRequestMessage* request,
        CIMReferenceNamesResponseMessage* response,
        PEGASUS_RESPONSE_CHUNK_CALLBACK_T responseChunkCallback);

    virtual void deliver(const CIMObjectPath& cimObjectPath);
    virtual void deliver(const Array<CIMObjectPath>& cimObjectPathArray)
    {
        SimpleObjectPathResponseHandler::deliver(cimObjectPathArray);
    }
    virtual void deliver(const SCMOInstance& scmoObjectPath);

protected:
    virtual String getClass() const;

    virtual void transfer();

};

class PEGASUS_PPM_LINKAGE InvokeMethodResponseHandler :
    public OperationResponseHandler, public SimpleMethodResultResponseHandler
{
public:
    InvokeMethodResponseHandler(
        CIMInvokeMethodRequestMessage* request,
        CIMInvokeMethodResponseMessage* response,
        PEGASUS_RESPONSE_CHUNK_CALLBACK_T responseChunkCallback);

    virtual void deliverParamValue(const CIMParamValue& cimParamValue);
    virtual void deliverParamValue(
        const Array<CIMParamValue>& cimParamValueArray)
    {
        SimpleMethodResultResponseHandler::deliverParamValue(
            cimParamValueArray);
    }

    virtual void deliver(const CIMValue& cimValue);

protected:
    virtual String getClass() const;

    virtual void transfer();

};

typedef void (*PEGASUS_INDICATION_CALLBACK_T)(
    CIMProcessIndicationRequestMessage*);

class PEGASUS_PPM_LINKAGE EnableIndicationsResponseHandler :
    public OperationResponseHandler, public SimpleIndicationResponseHandler
{
public:
    EnableIndicationsResponseHandler(
        CIMRequestMessage* request,
        CIMResponseMessage* response,
        const CIMInstance& provider,
        PEGASUS_INDICATION_CALLBACK_T indicationCallback,
        PEGASUS_RESPONSE_CHUNK_CALLBACK_T responseChunkCallback);

    virtual void deliver(const CIMIndication& cimIndication);

    virtual void deliver(
        const OperationContext& context,
        const CIMIndication& cimIndication);

    virtual void deliver(const Array<CIMIndication>& cimIndications);

    virtual void deliver(
        const OperationContext& context,
        const Array<CIMIndication>& cimIndications);

protected:
    virtual String getClass() const;

    virtual Boolean isAsync() const;

private:
    PEGASUS_INDICATION_CALLBACK_T _indicationCallback;

};

PEGASUS_NAMESPACE_END

#endif
