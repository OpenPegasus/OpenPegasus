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

#include "OperationResponseHandler.h"
#include "CIMOMHandleContext.h"

#include <Pegasus/Common/Tracer.h>
#include <Pegasus/Common/SharedPtr.h>
#include <Pegasus/Provider/CIMOMHandle.h>
#include <Pegasus/Config/ConfigManager.h>
#include <Pegasus/Common/SCMOClassCache.h>

PEGASUS_NAMESPACE_BEGIN

static void _initializeNormalizer(
    CIMOperationRequestMessage *request,
    Boolean includeQualifiers,
    Boolean includeClassOrigin,
    ObjectNormalizer &normalizer)
{
    // Attempt to get the cached class definition, normalization is disabled
    // if it does not exist.
    if (request->operationContext.contains(
            CachedClassDefinitionContainer::NAME))
    {
        CachedClassDefinitionContainer container =
            request->operationContext.get(
                CachedClassDefinitionContainer::NAME);
        CIMClass cimClass = container.getClass().clone();
        container = CachedClassDefinitionContainer(cimClass);
        SharedPtr<NormalizerContext> tmpContext(new CIMOMHandleContext());
        ObjectNormalizer tmpNormalizer(
            cimClass,
            includeQualifiers,
            includeClassOrigin,
            request->nameSpace,
            tmpContext);
        normalizer = tmpNormalizer;
    }
}

//
// OperationResponseHandler
//

OperationResponseHandler::OperationResponseHandler(
    CIMRequestMessage* request,
    CIMResponseMessage* response,
    PEGASUS_RESPONSE_CHUNK_CALLBACK_T responseChunkCallback)
    : _request(request),
    _response(response),
    _responseChunkCallback(responseChunkCallback),
    _responseObjectTotal(0),
    _responseMessageTotal(0),
    _responseObjectThreshold(0)
{
#ifndef PEGASUS_RESPONSE_OBJECT_COUNT_THRESHOLD
# define PEGASUS_RESPONSE_OBJECT_COUNT_THRESHOLD 100
#elif PEGASUS_RESPONSE_OBJECT_COUNT_THRESHOLD  == 0
# undef PEGASUS_RESPONSE_OBJECT_COUNT_THRESHOLD
# define PEGASUS_RESPONSE_OBJECT_COUNT_THRESHOLD  ~0
#endif

    if (!request)
    {
        _responseObjectThreshold = ~0;
    }
    else
    {
        _responseObjectThreshold = PEGASUS_RESPONSE_OBJECT_COUNT_THRESHOLD;

#ifdef PEGASUS_DEBUG
        static const char* responseObjectThreshold =
            getenv("PEGASUS_RESPONSE_OBJECT_COUNT_THRESHOLD");

        if (responseObjectThreshold)
        {
            Uint32 i = (Uint32)atoi(responseObjectThreshold);

            if (i > 0)
            {
                _responseObjectThreshold = i;
            }
        }
#endif
    }
}

OperationResponseHandler::~OperationResponseHandler()
{
    _request = 0;
    _response = 0;
}

CIMRequestMessage* OperationResponseHandler::getRequest() const
{
    return _request;
}

CIMResponseMessage* OperationResponseHandler::getResponse() const
{
    return _response;
}

void OperationResponseHandler::setStatus(
    const Uint32 code,
    const String& message)
{
    _response->cimException =
        PEGASUS_CIM_EXCEPTION(CIMStatusCode(code), message);
}

void OperationResponseHandler::setStatus(
    const Uint32 code,
    const ContentLanguageList& langs,
    const String& message)
{
    _response->cimException =
        PEGASUS_CIM_EXCEPTION_LANG(
        langs,
        CIMStatusCode(code),
        message);
}

void OperationResponseHandler::setCIMException(
    const CIMException& cimException)
{
    // Assign the cimException argument to _response->cimException. Note that
    // there is no need to use the PEGASUS_CIM_EXCEPTION_LANG() macro to create
    // a TraceableCIMException since both _response->cimException and
    // cimException are of type CIMException and the TraceableCIMException
    // constructor has no side effects.
    _response->cimException = cimException;
}

Boolean OperationResponseHandler::isAsync() const
{
    return _responseChunkCallback != 0;
}

// This is only called from SimpleResponseHandler.deliver() but lives in this
// class because all asyncronous response must have a "response" pointer
// to go through. Only operation classes have a response pointer
void OperationResponseHandler::send(Boolean isComplete)
{
    // It is possible to instantiate this class directly (not a derived
    // class, which would also inherit from SimpleResponseHandler).
    // The caller would do this only if the operation does not have any
    // data to be returned.

    SimpleResponseHandler* simpleP =
        dynamic_cast<SimpleResponseHandler*>(this);
    if (simpleP == 0)
    {
        // if there is no data to be returned, then the message should NEVER be
        // incomplete (even on an error)
        PEGASUS_ASSERT(isComplete);
        return;
    }

    // some handlers do not send async because their callers cannot handle
    // partial responses. If this is the case, stop here.

    if (!isAsync())
    {
        // preserve traditional behavior
        if (isComplete)
        {
            if (_response != 0)
            {
                _response->operationContext.set(
                    ContentLanguageListContainer(simpleP->getLanguages()));
            }
            transfer();
        }

        return;
    }

    SimpleResponseHandler& simple = *simpleP;
    PEGASUS_ASSERT(_response);
    Uint32 objectCount = simple.size();

    // have not reached threshold yet
    if ((isComplete == false) && (objectCount < _responseObjectThreshold))
    {
        return;
    }

    CIMResponseMessage* response = _response;

    // for complete responses, just use the one handed down from caller
    // otherwise, create our own that the caller never sees but is
    // utilized for async responses underneath

    if (isComplete == false)
    {
        _response = _request->buildResponse();
    }

    _response->setComplete(isComplete);
    _responseObjectTotal += objectCount;

    // since we are reusing response for every chunk, keep track of original
    // count
    _response->setIndex(_responseMessageTotal++);

    // set the originally allocated response to one more than the current.
    // The reason for doing this is proactive in case of an exception. This
    // allows the last response to be set as it may not re-enter this code.

    if (isComplete == false)
    {
        response->setIndex(_responseMessageTotal);
    }

    validate();

    if (_response->cimException.getCode() != CIM_ERR_SUCCESS)
    {
        simple.clear();
    }

    PEG_TRACE((
        TRC_PROVIDERMANAGER,
        Tracer::LEVEL4,
        "%s::transfer",
        (const char*) getClass().getCString()));

    transfer();
    simple.clear();

    _response->operationContext.set(
        ContentLanguageListContainer(simple.getLanguages()));

    // call thru ProviderManager to get externally declared entry point

    if (isComplete == false)
    {
        _responseChunkCallback(_request, _response);
    }

    // put caller's allocated response back in place. Note that _response
    // is INVALID after sending because it has been deleted externally

    _response = response;
}

void OperationResponseHandler::transfer()
{
}

void OperationResponseHandler::validate()
{
}

String OperationResponseHandler::getClass() const
{
    return String("OperationResponseHandler");
}

Uint32 OperationResponseHandler::getResponseObjectTotal() const
{
    return _responseObjectTotal;
}

Uint32 OperationResponseHandler::getResponseMessageTotal() const
{
    return _responseMessageTotal;
}

Uint32 OperationResponseHandler::getResponseObjectThreshold() const
{
    return _responseObjectThreshold;
}

//
// GetInstanceResponseHandler
//

GetInstanceResponseHandler::GetInstanceResponseHandler(
    CIMGetInstanceRequestMessage* request,
    CIMGetInstanceResponseMessage* response,
    PEGASUS_RESPONSE_CHUNK_CALLBACK_T responseChunkCallback)
    : OperationResponseHandler(request, response, responseChunkCallback)
{
    _initializeNormalizer(
        request,
        request->includeQualifiers,
        request->includeClassOrigin,
        _normalizer);
}

void GetInstanceResponseHandler::deliver(const CIMInstance& cimInstance)
{
    if (cimInstance.isUninitialized())
    {
        MessageLoaderParms message(
            "Common.Exception.UNINITIALIZED_OBJECT_EXCEPTION",
            "The object is not initialized.");

        throw CIMException(CIM_ERR_FAILED, message);
    }

    if (SimpleInstanceResponseHandler::size() != 0)
    {
        MessageLoaderParms message(
            "Server.OperationResponseHandler.TOO_MANY_OBJECTS_DELIVERED",
            "Too many objects delivered.");

        throw CIMException(CIM_ERR_FAILED, message);
    }

    CIMInstance localInstance(cimInstance);
#ifdef PEGASUS_ENABLE_OBJECT_NORMALIZATION
    // The normalizer expects an object path embedded in instances even
    // though it is not required by this operation. Use the requested
    // object path is missing from the instance.
    if (localInstance.getPath().getKeyBindings().size() == 0)
    {
        // ATTN: should clone before modification
        localInstance.setPath(static_cast<CIMGetInstanceRequestMessage*>(
            getRequest())->instanceName);
    }
#endif
    SimpleInstanceResponseHandler::deliver(
        _normalizer.processInstance(localInstance));
}

void GetInstanceResponseHandler::deliver(const SCMOInstance& cimInstance)
{
    if (cimInstance.isUninitialized())
    {
        MessageLoaderParms message(
            "Common.Exception.UNINITIALIZED_OBJECT_EXCEPTION",
            "The object is not initialized.");

        throw CIMException(CIM_ERR_FAILED, message);
    }

    if (SimpleInstanceResponseHandler::size() != 0)
    {
        MessageLoaderParms message(
            "Server.OperationResponseHandler.TOO_MANY_OBJECTS_DELIVERED",
            "Too many objects delivered.");

        throw CIMException(CIM_ERR_FAILED, message);
    }

    SimpleInstanceResponseHandler::deliver(cimInstance);
}

void GetInstanceResponseHandler::complete()
{
    if (SimpleInstanceResponseHandler::size() == 0)
    {
        MessageLoaderParms message(
            "Server.OperationResponseHandler.TOO_FEW_OBJECTS_DELIVERED",
            "Too few objects delivered.");
        // Provider claims success, no instances returned. -V see Bug #4104
        setStatus(CIM_ERR_NOT_FOUND);
        throw CIMException(CIM_ERR_FAILED, message);
    }

    SimpleInstanceResponseHandler::complete();
}

String GetInstanceResponseHandler::getClass() const
{
    return String("GetInstanceResponseHandler");
}

void GetInstanceResponseHandler::transfer()
{
    if (size() > 0)
    {
        CIMGetInstanceResponseMessage& msg =
            *static_cast<CIMGetInstanceResponseMessage*>(getResponse());
        Array<CIMInstance> cimObjs= getObjects();
        Array<SCMOInstance> scmoObjs= getSCMOObjects();
        if (cimObjs.size() != 0)
        {
            msg.getResponseData().setInstance(cimObjs[0]);
        }
        else
        {
            msg.getResponseData().setSCMO(scmoObjs);
        }
    }
}

void GetInstanceResponseHandler::validate()
{
    if (getResponseObjectTotal() == 0)
    {
        // error? provider claims success,
        // but did not deliver an instance.
        setStatus(CIM_ERR_NOT_FOUND);
    }
}

//
// EnumerateInstancesResponseHandler
//

EnumerateInstancesResponseHandler::EnumerateInstancesResponseHandler(
    CIMEnumerateInstancesRequestMessage* request,
    CIMEnumerateInstancesResponseMessage* response,
    PEGASUS_RESPONSE_CHUNK_CALLBACK_T responseChunkCallback)
    : OperationResponseHandler(request, response, responseChunkCallback)
{
    _initializeNormalizer(
        request,
        request->includeQualifiers,
        request->includeClassOrigin,
        _normalizer);
}

void EnumerateInstancesResponseHandler::deliver(const CIMInstance& cimInstance)
{
    if (cimInstance.isUninitialized())
    {
        MessageLoaderParms message(
            "Common.Exception.UNINITIALIZED_OBJECT_EXCEPTION",
            "The object is not initialized.");

        throw CIMException(CIM_ERR_FAILED, message);
    }

    SimpleInstanceResponseHandler::deliver(
        _normalizer.processInstance(cimInstance));
}

void EnumerateInstancesResponseHandler::deliver(
    const SCMOInstance& scmoInstance)
{
    if (scmoInstance.isUninitialized())
    {
        MessageLoaderParms message(
            "Common.Exception.UNINITIALIZED_OBJECT_EXCEPTION",
            "The object is not initialized.");

        throw CIMException(CIM_ERR_FAILED, message);
    }
    SimpleInstanceResponseHandler::deliver(scmoInstance);
}

String EnumerateInstancesResponseHandler::getClass() const
{
    return String("EnumerateInstancesResponseHandler");
}

void EnumerateInstancesResponseHandler::transfer()
{
    CIMEnumerateInstancesResponseMessage& msg =
        *static_cast<CIMEnumerateInstancesResponseMessage*>(getResponse());

    Array<CIMInstance> cimObjs= getObjects();
    Array<SCMOInstance> scmoObjs= getSCMOObjects();
    if (cimObjs.size() != 0)
    {
        msg.getResponseData().setInstances(cimObjs);
    }
    if (scmoObjs.size() != 0)
    {
        msg.getResponseData().setSCMO(scmoObjs);
    }
}

//
// EnumerateInstanceNamesResponseHandler
//

EnumerateInstanceNamesResponseHandler::EnumerateInstanceNamesResponseHandler(
    CIMEnumerateInstanceNamesRequestMessage* request,
    CIMEnumerateInstanceNamesResponseMessage* response,
    PEGASUS_RESPONSE_CHUNK_CALLBACK_T responseChunkCallback)
    : OperationResponseHandler(request, response, responseChunkCallback)
{
    _initializeNormalizer(
        request,
        false,
        false,
        _normalizer);
}

void EnumerateInstanceNamesResponseHandler::deliver(
    const CIMObjectPath& cimObjectPath)
{
    if (cimObjectPath.getClassName().isNull())
    {
        MessageLoaderParms message(
            "Common.Exception.UNINITIALIZED_OBJECT_EXCEPTION",
            "The object is not initialized.");

        throw CIMException(CIM_ERR_FAILED, message);
    }

    SimpleObjectPathResponseHandler::deliver(
        _normalizer.processInstanceObjectPath(cimObjectPath));
}

void EnumerateInstanceNamesResponseHandler::deliver(
    const SCMOInstance& scmoObjectPath)
{
    if (scmoObjectPath.getClassName()==NULL)
    {
        MessageLoaderParms message(
            "Common.Exception.UNINITIALIZED_OBJECT_EXCEPTION",
            "The object is not initialized.");

        throw CIMException(CIM_ERR_FAILED, message);
    }

    SimpleObjectPathResponseHandler::deliver(scmoObjectPath);
}

String EnumerateInstanceNamesResponseHandler::getClass() const
{
    return String("EnumerateInstanceNamesResponseHandler");
}

void EnumerateInstanceNamesResponseHandler::transfer()
{
    CIMEnumerateInstanceNamesResponseMessage& msg =
        *static_cast<CIMEnumerateInstanceNamesResponseMessage*>(getResponse());

    Array<CIMObjectPath> cimObjs= getObjects();
    Array<SCMOInstance> scmoObjs= getSCMOObjects();
    if (cimObjs.size() != 0)
    {
        msg.getResponseData().setInstanceNames(cimObjs);
    }
    if (scmoObjs.size() != 0)
    {
        msg.getResponseData().setSCMO(scmoObjs);
    }
}

//
// CreateInstanceResponseHandler
//

CreateInstanceResponseHandler::CreateInstanceResponseHandler(
    CIMCreateInstanceRequestMessage* request,
    CIMCreateInstanceResponseMessage* response,
    PEGASUS_RESPONSE_CHUNK_CALLBACK_T responseChunkCallback)
    : OperationResponseHandler(request, response, responseChunkCallback)
{
}

void CreateInstanceResponseHandler::deliver(const CIMObjectPath& cimObjectPath)
{
    if (cimObjectPath.getClassName().isNull())
    {
        MessageLoaderParms message(
            "Common.Exception.UNINITIALIZED_OBJECT_EXCEPTION",
            "The object is not initialized.");

        throw CIMException(CIM_ERR_FAILED, message);
    }

    if (SimpleObjectPathResponseHandler::size() != 0)
    {
        MessageLoaderParms message(
            "Server.OperationResponseHandler.TOO_MANY_OBJECTS_DELIVERED",
            "Too many objects delivered.");

        throw CIMException(CIM_ERR_FAILED, message);
    }

    SimpleObjectPathResponseHandler::deliver(cimObjectPath);
}

void CreateInstanceResponseHandler::complete()
{
    if (SimpleObjectPathResponseHandler::size() == 0)
    {
        MessageLoaderParms message(
            "Server.OperationResponseHandler.TOO_FEW_OBJECTS_DELIVERED",
            "Too few objects delivered.");

        throw CIMException(CIM_ERR_FAILED, message);
    }

    SimpleObjectPathResponseHandler::complete();
}

String CreateInstanceResponseHandler::getClass() const
{
    return String("CreateInstanceResponseHandler");
}

#if 0
// ATTN: is it an error to not return instance name?
void CreateInstanceResponseHandler::validate()
{
    if (getResponseObjectTotal() == 0)
    {
        setStatus(CIM_ERR_NOT_FOUND);
    }
}
#endif

void CreateInstanceResponseHandler::transfer()
{
    if (size() > 0)
    {
        CIMCreateInstanceResponseMessage& msg =
            *static_cast<CIMCreateInstanceResponseMessage*>(getResponse());

        Array<CIMObjectPath> cimObjs= getObjects();
        Array<SCMOInstance> scmoObjs= getSCMOObjects();
        if (cimObjs.size() != 0)
        {
            msg.instanceName = cimObjs[0];
        }
        else
        {
            scmoObjs[0].getCIMObjectPath(msg.instanceName);
        }
    }
}

//
// ModifyInstanceResponseHandler
//

ModifyInstanceResponseHandler::ModifyInstanceResponseHandler(
    CIMModifyInstanceRequestMessage* request,
    CIMModifyInstanceResponseMessage* response,
    PEGASUS_RESPONSE_CHUNK_CALLBACK_T responseChunkCallback)
    : OperationResponseHandler(request, response, responseChunkCallback)
{
}

String ModifyInstanceResponseHandler::getClass() const
{
    return String("ModifyInstanceResponseHandler");
}

//
// DeleteInstanceResponseHandler
//

DeleteInstanceResponseHandler::DeleteInstanceResponseHandler(
    CIMDeleteInstanceRequestMessage* request,
    CIMDeleteInstanceResponseMessage* response,
    PEGASUS_RESPONSE_CHUNK_CALLBACK_T responseChunkCallback)
    : OperationResponseHandler(request, response, responseChunkCallback)
{
}

String DeleteInstanceResponseHandler::getClass() const
{
    return String("DeleteInstanceResponseHandler");
}

//
// GetPropertyResponseHandler
//

GetPropertyResponseHandler::GetPropertyResponseHandler(
    CIMGetPropertyRequestMessage* request,
    CIMGetPropertyResponseMessage* response,
    PEGASUS_RESPONSE_CHUNK_CALLBACK_T responseChunkCallback)
    : OperationResponseHandler(request, response, responseChunkCallback)
{
}

void GetPropertyResponseHandler::deliver(const CIMValue& cimValue)
{
    if (cimValue.isNull())
    {
        MessageLoaderParms message(
            "Common.Exception.UNINITIALIZED_OBJECT_EXCEPTION",
            "The object is not initialized.");

        throw CIMException(CIM_ERR_FAILED, message);
    }

    SimpleValueResponseHandler::deliver(cimValue);
}

String GetPropertyResponseHandler::getClass() const
{
    return String("GetPropertyResponseHandler");
}

void GetPropertyResponseHandler::transfer()
{
    if (size() > 0)
    {
        CIMGetPropertyResponseMessage& msg =
            *static_cast<CIMGetPropertyResponseMessage*>(getResponse());

        msg.value = getObjects()[0];
    }
}

void GetPropertyResponseHandler::validate()
{
    // error? provider claims success,
    // but did not deliver an instance.
    if (getResponseObjectTotal() == 0)
    {
        setStatus(CIM_ERR_NOT_FOUND);
    }
}

//
// SetPropertyResponseHandler
//

SetPropertyResponseHandler::SetPropertyResponseHandler(
    CIMSetPropertyRequestMessage* request,
    CIMSetPropertyResponseMessage* response,
    PEGASUS_RESPONSE_CHUNK_CALLBACK_T responseChunkCallback)
    : OperationResponseHandler(request, response, responseChunkCallback)
{
}

String SetPropertyResponseHandler::getClass() const
{
    return String("SetPropertyResponseHandler");
}

//
// ExecQueryResponseHandler
//

ExecQueryResponseHandler::ExecQueryResponseHandler(
    CIMExecQueryRequestMessage* request,
    CIMExecQueryResponseMessage* response,
    PEGASUS_RESPONSE_CHUNK_CALLBACK_T responseChunkCallback)
    : OperationResponseHandler(request, response, responseChunkCallback)
{
}

void ExecQueryResponseHandler::deliver(const CIMInstance& cimInstance)
{
    if (cimInstance.isUninitialized())
    {
        MessageLoaderParms message(
            "Common.Exception.UNINITIALIZED_OBJECT_EXCEPTION",
            "The object is not initialized.");

        throw CIMException(CIM_ERR_FAILED, message);
    }

    SimpleInstance2ObjectResponseHandler::deliver(cimInstance);
}

void ExecQueryResponseHandler::deliver(const SCMOInstance& scmoInstance)
{
    if (scmoInstance.isUninitialized())
    {
        MessageLoaderParms message(
            "Common.Exception.UNINITIALIZED_OBJECT_EXCEPTION",
            "The object is not initialized.");

        throw CIMException(CIM_ERR_FAILED, message);
    }

    SimpleInstance2ObjectResponseHandler::deliver(scmoInstance);
}

String ExecQueryResponseHandler::getClass() const
{
    return String("ExecQueryResponseHandler");
}

void ExecQueryResponseHandler::transfer()
{
    CIMExecQueryResponseMessage& msg =
        *static_cast<CIMExecQueryResponseMessage*>(getResponse());

    Array<CIMObject> cimObjs= getObjects();
    Array<SCMOInstance> scmoObjs= getSCMOObjects();
    if (cimObjs.size() != 0)
    {
        // complete keybindings based on set property values
        CIMOperationRequestMessage * reqMsg =
            (CIMOperationRequestMessage*) _request;

        Boolean clsRead=false;
        for (Uint32 j = 0, m = cimObjs.size(); j < m; j++)
        {
            CIMObject & co=cimObjs[j];
            CIMObjectPath op=co.getPath();
            const Array<CIMKeyBinding>& kbs=op.getKeyBindings();
            if (kbs.size()==0)
            {     // no path set why ?
                if (clsRead==false || _cimClass.isUninitialized())
                {
                    SCMOClassCache * classCache = SCMOClassCache::getInstance();
                    CString nsName = reqMsg->nameSpace.getString().getCString();
                    CString clName = reqMsg->className.getString().getCString();
                    SCMOClass theClass = classCache->getSCMOClass(
                        nsName,
                        strlen(nsName),
                        clName,
                        strlen(clName));
                    theClass.getCIMClass(_cimClass);
                    clsRead=true;
                }
                op = CIMInstance(co).buildPath(_cimClass);
            }
            op.setNameSpace(reqMsg->nameSpace);
            op.setHost(System::getHostName());
            co.setPath(op);
        }
        msg.getResponseData().setObjects(cimObjs);
    }
    if (scmoObjs.size() != 0)
    {
        msg.getResponseData().setSCMO(scmoObjs);
    }
}

Boolean ExecQueryResponseHandler::isAsync() const
{
    return false;
}

//
// AssociatorsResponseHandler
//

AssociatorsResponseHandler::AssociatorsResponseHandler(
    CIMAssociatorsRequestMessage* request,
    CIMAssociatorsResponseMessage* response,
    PEGASUS_RESPONSE_CHUNK_CALLBACK_T responseChunkCallback)
    : OperationResponseHandler(request, response, responseChunkCallback)
{
}

void AssociatorsResponseHandler::deliver(const CIMObject& cimObject)
{
    if (cimObject.isUninitialized())
    {
        MessageLoaderParms message(
            "Common.Exception.UNINITIALIZED_OBJECT_EXCEPTION",
            "The object is not initialized.");

        throw CIMException(CIM_ERR_FAILED, message);
    }

    SimpleObjectResponseHandler::deliver(cimObject);
}

void AssociatorsResponseHandler::deliver(const CIMInstance& cimInstance)
{
    if (cimInstance.isUninitialized())
    {
        MessageLoaderParms message(
            "Common.Exception.UNINITIALIZED_OBJECT_EXCEPTION",
            "The object is not initialized.");

        throw CIMException(CIM_ERR_FAILED, message);
    }

    SimpleObjectResponseHandler::deliver(cimInstance);
}

void AssociatorsResponseHandler::deliver(const SCMOInstance& scmoObject)
{
    if (scmoObject.isUninitialized())
    {
        MessageLoaderParms message(
            "Common.Exception.UNINITIALIZED_OBJECT_EXCEPTION",
            "The object is not initialized.");

        throw CIMException(CIM_ERR_FAILED, message);
    }
    SimpleObjectResponseHandler::deliver(scmoObject);
}

String AssociatorsResponseHandler::getClass() const
{
    return String("AssociatorsResponseHandler");
}

void AssociatorsResponseHandler::transfer()
{
    CIMAssociatorsResponseMessage& msg =
        *static_cast<CIMAssociatorsResponseMessage*>(getResponse());

    Array<CIMObject> cimObjs= getObjects();
    Array<SCMOInstance> scmoObjs= getSCMOObjects();
    if (cimObjs.size() != 0)
    {
        msg.getResponseData().setObjects(cimObjs);
    }
    if (scmoObjs.size() != 0)
    {
        msg.getResponseData().setSCMO(scmoObjs);
    }
}

//
// AssociatorNamesResponseHandler
//

AssociatorNamesResponseHandler::AssociatorNamesResponseHandler(
    CIMAssociatorNamesRequestMessage* request,
    CIMAssociatorNamesResponseMessage* response,
    PEGASUS_RESPONSE_CHUNK_CALLBACK_T responseChunkCallback)
    : OperationResponseHandler(request, response, responseChunkCallback)
{
}

void AssociatorNamesResponseHandler::deliver(const CIMObjectPath& cimObjectPath)
{
    if (cimObjectPath.getClassName().isNull())
    {
        MessageLoaderParms message(
            "Common.Exception.UNINITIALIZED_OBJECT_EXCEPTION",
            "The object is not initialized.");

        throw CIMException(CIM_ERR_FAILED, message);
    }

    SimpleObjectPathResponseHandler::deliver(cimObjectPath);
}

void AssociatorNamesResponseHandler::deliver(const SCMOInstance& scmoObjectPath)
{
    SimpleObjectPathResponseHandler::deliver(scmoObjectPath);
}

String AssociatorNamesResponseHandler::getClass() const
{
    return String("AssociatorNamesResponseHandler");
}

void AssociatorNamesResponseHandler::transfer()
{
    CIMAssociatorNamesResponseMessage& msg =
        *static_cast<CIMAssociatorNamesResponseMessage*>(getResponse());

    Array<CIMObjectPath> cimObjs= getObjects();
    Array<SCMOInstance> scmoObjs= getSCMOObjects();
    if (cimObjs.size() != 0)
    {
        msg.getResponseData().setInstanceNames(cimObjs);
    }
    if (scmoObjs.size() != 0)
    {
        msg.getResponseData().setSCMO(scmoObjs);
    }
}

//
// ReferencesResponseHandler
//

ReferencesResponseHandler::ReferencesResponseHandler(
    CIMReferencesRequestMessage* request,
    CIMReferencesResponseMessage* response,
    PEGASUS_RESPONSE_CHUNK_CALLBACK_T responseChunkCallback)
    : OperationResponseHandler(request, response, responseChunkCallback)
{
}

void ReferencesResponseHandler::deliver(const CIMObject& cimObject)
{
    if (cimObject.isUninitialized())
    {
        MessageLoaderParms message(
            "Common.Exception.UNINITIALIZED_OBJECT_EXCEPTION",
            "The object is not initialized.");

        throw CIMException(CIM_ERR_FAILED, message);
    }

    SimpleObjectResponseHandler::deliver(cimObject);
}

void ReferencesResponseHandler::deliver(const SCMOInstance& scmoObject)
{
    if (scmoObject.isUninitialized())
    {
        MessageLoaderParms message(
            "Common.Exception.UNINITIALIZED_OBJECT_EXCEPTION",
            "The object is not initialized.");

        throw CIMException(CIM_ERR_FAILED, message);
    }
    SimpleObjectResponseHandler::deliver(scmoObject);
}

String ReferencesResponseHandler::getClass() const
{
    return String("ReferencesResponseHandler");
}

void ReferencesResponseHandler::transfer()
{
    CIMReferencesResponseMessage& msg =
        *static_cast<CIMReferencesResponseMessage*>(getResponse());

    Array<CIMObject> cimObjs= getObjects();
    Array<SCMOInstance> scmoObjs= getSCMOObjects();
    if (cimObjs.size() != 0)
    {
        msg.getResponseData().setObjects(cimObjs);
    }
    if (scmoObjs.size() != 0)
    {
        msg.getResponseData().setSCMO(scmoObjs);
    }
}

//
// ReferenceNamesResponseHandler
//

ReferenceNamesResponseHandler::ReferenceNamesResponseHandler(
    CIMReferenceNamesRequestMessage* request,
    CIMReferenceNamesResponseMessage* response,
    PEGASUS_RESPONSE_CHUNK_CALLBACK_T responseChunkCallback)
    : OperationResponseHandler(request, response, responseChunkCallback)
{
}

void ReferenceNamesResponseHandler::deliver(const CIMObjectPath& cimObjectPath)
{
    if (cimObjectPath.getClassName().isNull())
    {
        MessageLoaderParms message(
            "Common.Exception.UNINITIALIZED_OBJECT_EXCEPTION",
            "The object is not initialized.");

        throw CIMException(CIM_ERR_FAILED, message);
    }

    SimpleObjectPathResponseHandler::deliver(cimObjectPath);
}

void ReferenceNamesResponseHandler::deliver(const SCMOInstance& scmoObjectPath)
{
    SimpleObjectPathResponseHandler::deliver(scmoObjectPath);
}

String ReferenceNamesResponseHandler::getClass() const
{
    return String("ReferenceNamesResponseHandler");
}

void ReferenceNamesResponseHandler::transfer()
{
    CIMReferenceNamesResponseMessage& msg =
        *static_cast<CIMReferenceNamesResponseMessage*>(getResponse());

    Array<CIMObjectPath> cimObjs= getObjects();
    Array<SCMOInstance> scmoObjs= getSCMOObjects();
    if (cimObjs.size() != 0)
    {
        msg.getResponseData().setInstanceNames(cimObjs);
    }
    if (scmoObjs.size() != 0)
    {
        msg.getResponseData().setSCMO(scmoObjs);
    }
}

//
// InvokeMethodResponseHandler
//

InvokeMethodResponseHandler::InvokeMethodResponseHandler(
    CIMInvokeMethodRequestMessage* request,
    CIMInvokeMethodResponseMessage* response,
    PEGASUS_RESPONSE_CHUNK_CALLBACK_T responseChunkCallback)
    : OperationResponseHandler(request, response, responseChunkCallback)
{
}

void InvokeMethodResponseHandler::deliverParamValue(
    const CIMParamValue& cimParamValue)
{
    if (cimParamValue.isUninitialized())
    {
        MessageLoaderParms message(
            "Common.Exception.UNINITIALIZED_OBJECT_EXCEPTION",
            "The object is not initialized.");

        throw CIMException(CIM_ERR_FAILED, message);
    }

    SimpleMethodResultResponseHandler::deliverParamValue(cimParamValue);
}

void InvokeMethodResponseHandler::deliver(const CIMValue& cimValue)
{
    if (cimValue.isNull())
    {
        MessageLoaderParms message(
            "Common.Exception.UNINITIALIZED_OBJECT_EXCEPTION",
            "The object is not initialized.");

        throw CIMException(CIM_ERR_FAILED, message);
    }

    SimpleMethodResultResponseHandler::deliver(cimValue);
}

String InvokeMethodResponseHandler::getClass() const
{
    return String("InvokeMethodResponseHandler");
}

void InvokeMethodResponseHandler::transfer()
{
    CIMInvokeMethodResponseMessage& msg =
        *static_cast<CIMInvokeMethodResponseMessage*>(getResponse());

    msg.outParameters = getParamValues();

    // ATTN-RK-20020903: Is it legal for the return value to be null?
    // if not, then the check must be done here since deliver() works off the
    // virtual size, which refers to out parameters!
    msg.retValue = getReturnValue();
}

//
// EnableIndicationsResponseHandler
//

EnableIndicationsResponseHandler::EnableIndicationsResponseHandler(
    CIMRequestMessage* request,
    CIMResponseMessage* response,
    const CIMInstance& provider,
    PEGASUS_INDICATION_CALLBACK_T indicationCallback,
    PEGASUS_RESPONSE_CHUNK_CALLBACK_T responseChunkCallback)
    : OperationResponseHandler(request, response, responseChunkCallback),
      _indicationCallback(indicationCallback)
{
    _provider = provider;
}

void EnableIndicationsResponseHandler::deliver(
    const CIMIndication& cimIndication)
{
    OperationContext context;

    Array<CIMObjectPath> subscriptionInstanceNames;

    context.insert(
        SubscriptionInstanceNamesContainer(subscriptionInstanceNames));

    deliver(context, cimIndication);
}

void EnableIndicationsResponseHandler::deliver(
    const OperationContext& context,
    const CIMIndication& cimIndication)
{
    if (cimIndication.isUninitialized())
    {
        MessageLoaderParms message(
            "Common.Exception.UNINITIALIZED_OBJECT_EXCEPTION",
            "The object is not initialized.");

        throw CIMException(CIM_ERR_FAILED, message);
    }

    // ATTN: temporarily convert indication to instance
    CIMInstance cimInstance(cimIndication);

    //  Get list of subscription instance names from context
    Array<CIMObjectPath> subscriptionInstanceNames;

    if (context.contains(SubscriptionInstanceNamesContainer::NAME))
    {
        SubscriptionInstanceNamesContainer container =
            context.get(SubscriptionInstanceNamesContainer::NAME);

        subscriptionInstanceNames = container.getInstanceNames();
    }
    else
    {
        subscriptionInstanceNames.clear();
    }

    ContentLanguageList contentLangs;

    if (context.contains(ContentLanguageListContainer::NAME))
    {
        // Get the Content-Language for this indication.  The provider
        // does not have to add specify a language for the indication.
        ContentLanguageListContainer langContainer =
            context.get(ContentLanguageListContainer::NAME);

        contentLangs = langContainer.getLanguages();
    }
    else
    {
        // The provider did not explicitly set a Content-Language for
        // the indication.  Fall back to the lang set in this object.
        contentLangs = getLanguages();
    }

    Uint32 timeoutMilliSec = 0;
    if (context.contains(TimeoutContainer::NAME))
    {
        TimeoutContainer timeoutContainer =
            context.get(TimeoutContainer::NAME);
        timeoutMilliSec = timeoutContainer.getTimeOut();
    }

    // create message
    CIMProcessIndicationRequestMessage* request =
        new CIMProcessIndicationRequestMessage(
        XmlWriter::getNextMessageId(),
        cimInstance.getPath().getNameSpace(),
        cimInstance,
        subscriptionInstanceNames,
        _provider,
        QueueIdStack(),  // Must be filled in by the callback function
        timeoutMilliSec);

    request->operationContext = context;

    if (request->operationContext.contains(ContentLanguageListContainer::NAME))
    {
        request->operationContext.set(
            ContentLanguageListContainer(contentLangs));
    }
    else
    {
        request->operationContext.insert(
            ContentLanguageListContainer(contentLangs));
    }

    _indicationCallback(request);
}

void EnableIndicationsResponseHandler::deliver(
    const Array<CIMIndication>& cimIndications)
{
    OperationContext context;

    deliver(context, cimIndications);
}

void EnableIndicationsResponseHandler::deliver(
    const OperationContext& context,
    const Array<CIMIndication>& cimIndications)
{
    for (Uint32 i = 0, n = cimIndications.size(); i < n; i++)
    {
        deliver(context, cimIndications[i]);
    }
}

String EnableIndicationsResponseHandler::getClass() const
{
    return String("EnableIndicationsResponseHandler");
}

Boolean EnableIndicationsResponseHandler::isAsync() const
{
    return false;
}

PEGASUS_NAMESPACE_END
