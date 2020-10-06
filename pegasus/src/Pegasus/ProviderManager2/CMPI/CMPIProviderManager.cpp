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

#include "CMPI_Version.h"

#include "CMPIProviderManager.h"

#include "CMPI_Object.h"
#include "CMPI_Instance.h"
#include "CMPI_ObjectPath.h"
#include "CMPI_Result.h"
#include "CMPI_SelectExp.h"
#include "CMPISCMOUtilities.h"
#include "CMPI_Value.h"
#include "CMPIMsgHandleManager.h"


#include <Pegasus/Common/CIMMessage.h>
#include <Pegasus/Common/OperationContext.h>
#include <Pegasus/Common/OperationContextInternal.h>
#include <Pegasus/Common/Tracer.h>
#include <Pegasus/Common/StatisticalData.h>
#include <Pegasus/Common/Logger.h>
#include <Pegasus/Common/LanguageParser.h>
#include <Pegasus/Common/MessageLoader.h> //l10n
#include <Pegasus/Common/Constants.h>
#include <Pegasus/Common/FileSystem.h>

#include <Pegasus/Config/ConfigManager.h>

#include <Pegasus/Provider/CIMOMHandleQueryContext.h>
#include <Pegasus/ProviderManager2/CIMOMHandleContext.h>
#include <Pegasus/ProviderManager2/ProviderName.h>
#include <Pegasus/ProviderManager2/AutoPThreadSecurity.h>
#include <Pegasus/ProviderManager2/CMPI/CMPIProviderModule.h>
#include <Pegasus/ProviderManager2/CMPI/CMPIProvider.h>
#include <Pegasus/ProviderManager2/CMPI/CMPI_ThreadContext.h>
#include <Pegasus/Query/QueryExpression/QueryExpression.h>
#include <Pegasus/Query/QueryCommon/QueryException.h>

PEGASUS_USING_STD;

PEGASUS_NAMESPACE_BEGIN


ReadWriteSem CMPIProviderManager::rwSemProvTab;
CMPIProviderManager::IndProvTab CMPIProviderManager::indProvTab;

class CMPIPropertyList
{
    char **props;
    int pCount;
public:
    CMPIPropertyList(CIMPropertyList &propertyList) : props(0), pCount(0)
    {
        PEG_METHOD_ENTER(
            TRC_PROVIDERMANAGER,
            "CMPIPropertyList::CMPIPropertyList()");
        if (!propertyList.isNull())
        {
            Array<CIMName> p=propertyList.getPropertyNameArray();
            pCount=p.size();
            props = new char*[1+pCount];
            for (int i=0; i<pCount; i++)
            {
                props[i]=strdup(p[i].getString().getCString());
            }
            props[pCount]=NULL;
        }
        else props=NULL;
        PEG_METHOD_EXIT();
    }
    ~CMPIPropertyList()
    {
        PEG_METHOD_ENTER(
            TRC_PROVIDERMANAGER,
            "CMPIPropertyList::~CMPIPropertyList()");
        if (props)
        {
            for (int i=0; i<pCount; i++)
                free(props[i]);
            delete [] props;
        }
        PEG_METHOD_EXIT();
    }
    char  **getList()
    {
        return props;
    }
};

CMPIProviderManager::CMPIProviderManager()
{
    PEG_METHOD_ENTER(
        TRC_PROVIDERMANAGER,
        "CMPIProviderManager::CMPIProviderManager()");

    _subscriptionInitComplete = false;
    PEG_TRACE_CSTRING (
        TRC_PROVIDERMANAGER,
        Tracer::LEVEL2,
        "-- CMPI Provider Manager activated");
    PEG_METHOD_EXIT();
}

CMPIProviderManager::~CMPIProviderManager()
{
    PEG_METHOD_ENTER(
        TRC_PROVIDERMANAGER,
        "CMPIProviderManager::~CMPIProviderManager()");

    IndProvRecord *indProvRec = 0;

    WriteLock lock(rwSemProvTab);

    for (IndProvTab::Iterator i = indProvTab.start(); i; i++)
    {
        indProvTab.lookup(i.key(), indProvRec);
        delete indProvRec;
    }

    PEG_METHOD_EXIT();
}


SCMOInstance* CMPIProviderManager::getSCMOClassFromRequest(
    CString& nameSpace,
    CString& className )
{
    SCMOClass* scmoClass = mbGetSCMOClass(
        (const char*)nameSpace,
        strlen((const char*)nameSpace),
        (const char*)className,
        strlen((const char*)className));

    if (0 == scmoClass)
    {
        // This indicates a severe error, since we should't have come
        // here at all, if the class is invalid
        PEG_TRACE((TRC_PROVIDERMANAGER,Tracer::LEVEL2,
            "CMPIProviderManager::getSCMOClassFromRequest - "
            "Failed to obtain SCMOClass for Namespace: %s  Classname: %s",
            (const char*) nameSpace,
            (const char*) className));

        CIMException cimException(CIM_ERR_NOT_FOUND);
        throw cimException;
    }

    SCMOInstance *classPath = new SCMOInstance(*scmoClass);
    classPath->setHostName(
        (const char*)System::getHostName().getCString());

    // Clear the KeyBindings to make this instance as class path only.
    classPath->clearKeyBindings();
    
    return classPath;
}

SCMOInstance* CMPIProviderManager::getSCMOObjectPathFromRequest(
    CString& nameSpace,
    CString& className,
    CIMObjectPath& cimObjPath )
{
    SCMOClass* scmoClass = mbGetSCMOClass(
        (const char*)nameSpace,
        strlen((const char*)nameSpace),
        (const char*)className,
        strlen((const char*)className));

    if (0 == scmoClass)
    {
        // This indicates a severe error, since we should't have come
        // here at all, if the class is invalid
        PEG_TRACE((TRC_PROVIDERMANAGER,Tracer::LEVEL1,
            "CMPIProviderManager::getSCMOObjectPathFromRequest - "
            "Failed to obtain CIMClass for Namespace: %s  Classname: %s",
            (const char*) nameSpace,
            (const char*) className));

        CIMException cimException(CIM_ERR_NOT_FOUND);
        throw cimException;
    }

    SCMOInstance * objectPath = new SCMOInstance(*scmoClass,cimObjPath);
    objectPath->setHostName((const char*)System::getHostName().getCString());
    return objectPath;
}

SCMOInstance* CMPIProviderManager::getSCMOInstanceFromRequest(
    CString& nameSpace,
    CString& className,
    CIMInstance& cimInstance )
{
    SCMOClass* scmoClass = mbGetSCMOClass(
        (const char*)nameSpace,
        strlen((const char*)nameSpace),
        (const char*)className,
        strlen((const char*)className));

    if (0 == scmoClass)
    {
        // This indicates a severe error, since we should't have come
        // here at all, if the class is invalid
        PEG_TRACE((TRC_PROVIDERMANAGER,Tracer::LEVEL1,
            "CMPIProviderManager::getSCMOInstanceFromRequest - "
            "Failed to obtain CIMClass for Namespace: %s  Classname: %s",
            (const char*) nameSpace,
            (const char*) className));

        CIMException cimException(CIM_ERR_NOT_FOUND);
        throw cimException;
    }

    SCMOInstance * newInstance = new SCMOInstance(*scmoClass, cimInstance);
    newInstance->setHostName((const char*)System::getHostName().getCString());

    return newInstance;
}



Message * CMPIProviderManager::processMessage(Message * request)
{
    PEG_METHOD_ENTER(
        TRC_PROVIDERMANAGER,
        "CMPIProviderManager::processMessage()");

    Message * response = 0;

    // pass the request message to a handler method based on message type
    switch (request->getType())
    {
        case CIM_GET_INSTANCE_REQUEST_MESSAGE:
            response = handleGetInstanceRequest(request);

            break;
        case CIM_ENUMERATE_INSTANCES_REQUEST_MESSAGE:
            response = handleEnumerateInstancesRequest(request);

            break;
        case CIM_ENUMERATE_INSTANCE_NAMES_REQUEST_MESSAGE:
            response = handleEnumerateInstanceNamesRequest(request);

            break;
        case CIM_CREATE_INSTANCE_REQUEST_MESSAGE:
            response = handleCreateInstanceRequest(request);

            break;
        case CIM_MODIFY_INSTANCE_REQUEST_MESSAGE:
            response = handleModifyInstanceRequest(request);

            break;
        case CIM_DELETE_INSTANCE_REQUEST_MESSAGE:
            response = handleDeleteInstanceRequest(request);

            break;
        case CIM_EXEC_QUERY_REQUEST_MESSAGE:
            response = handleExecQueryRequest(request);

            break;
        case CIM_ASSOCIATORS_REQUEST_MESSAGE:
            response = handleAssociatorsRequest(request);

            break;
        case CIM_ASSOCIATOR_NAMES_REQUEST_MESSAGE:
            response = handleAssociatorNamesRequest(request);

            break;
        case CIM_REFERENCES_REQUEST_MESSAGE:
            response = handleReferencesRequest(request);

            break;
        case CIM_REFERENCE_NAMES_REQUEST_MESSAGE:
            response = handleReferenceNamesRequest(request);

            break;
        case CIM_INVOKE_METHOD_REQUEST_MESSAGE:
            response = handleInvokeMethodRequest(request);

            break;
        case CIM_CREATE_SUBSCRIPTION_REQUEST_MESSAGE:
            response = handleCreateSubscriptionRequest(request);

            break;
/*    case CIM_MODIFY_SUBSCRIPTION_REQUEST_MESSAGE:
        response = handleModifySubscriptionRequest(request);

        break;
*/
        case CIM_DELETE_SUBSCRIPTION_REQUEST_MESSAGE:
            response = handleDeleteSubscriptionRequest(request);

            break;
/*    case CIM_EXPORT_INDICATION_REQUEST_MESSAGE:
        response = handleExportIndicationRequest(request);
        break;
*/
        case CIM_DISABLE_MODULE_REQUEST_MESSAGE:
            response = handleDisableModuleRequest(request);

            break;
        case CIM_ENABLE_MODULE_REQUEST_MESSAGE:
            response = handleEnableModuleRequest(request);

            break;
        case CIM_STOP_ALL_PROVIDERS_REQUEST_MESSAGE:
            response = handleStopAllProvidersRequest(request);

            break;
        case CIM_SUBSCRIPTION_INIT_COMPLETE_REQUEST_MESSAGE:
            response = handleSubscriptionInitCompleteRequest (request);

            break;
        case CIM_INDICATION_SERVICE_DISABLED_REQUEST_MESSAGE:
            response = handleIndicationServiceDisabledRequest (request);
            break;
        case CIM_GET_PROPERTY_REQUEST_MESSAGE:
            response = handleGetPropertyRequest(request);

            break;
        case CIM_SET_PROPERTY_REQUEST_MESSAGE:
            response = handleSetPropertyRequest(request);

            break;
        default:
            response = handleUnsupportedRequest(request);

            break;
    }

    PEG_METHOD_EXIT();

    return(response);
}

Boolean CMPIProviderManager::hasActiveProviders()
{
    return providerManager.hasActiveProviders();
}

void CMPIProviderManager::unloadIdleProviders()
{
    providerManager.unloadIdleProviders();
}


#define CHARS(cstring) (char*)(strlen(cstring)?(const char*)cstring:NULL)


#define HandlerIntroBase(type,type1,message,request,response,handler) \
    CIM##type##RequestMessage * request = \
        dynamic_cast<CIM##type##RequestMessage *>(const_cast<Message *> \
        (message)); \
    PEGASUS_ASSERT(request != 0); \
    CIM##type##ResponseMessage * response = \
        dynamic_cast<CIM##type##ResponseMessage*>(request->buildResponse()); \
    PEGASUS_ASSERT(response != 0); \
    type1##ResponseHandler handler(request, response, _responseChunkCallback);

#define HandlerIntroInd(type,message,request,response,handler) \
     HandlerIntroBase(type,Operation,message,request,response,handler)

#define HandlerIntroInit(type,message,request,response,handler) \
     HandlerIntroBase(type,Operation,message,request,response,handler)

#define HandlerIntro(type,message,request,response,handler) \
     HandlerIntroBase(type,type,message,request,response,handler)

#define HandlerCatch(handler) \
    catch(const CIMException & e)  \
    { PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL1, \
          "CIMException: %s",(const char*)e.getMessage().getCString())); \
        handler.setCIMException(e); \
    } \
    catch(const Exception & e) \
    { PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL1, \
          "Exception: %s",(const char*)e.getMessage().getCString())); \
        handler.setStatus(CIM_ERR_FAILED, e.getContentLanguages(), \
        e.getMessage()); \
    } \
    catch(...) \
    { PEG_TRACE_CSTRING(TRC_PROVIDERMANAGER, Tracer::LEVEL1, \
                "Exception: Unknown"); \
        handler.setStatus(CIM_ERR_FAILED, "Unknown error."); \
    }

/* setup the CMPI context based on the requests OperationContext
   the OperationContext
   nameSpace and remoteInfo are used by pointer instead of by reference to
   avoid copies being generated, both CStrings are anchored on the stack in the
   scope of the calling function to keep them valid across the lifetime of the
   CMPI processing of a request
*/
void CMPIProviderManager::_setupCMPIContexts(
    CMPI_ContextOnStack * eCtx,
    OperationContext * context,
    const CString * nameSpace,
    const CString * remoteInfo,
    Boolean remote,
    Boolean includeQualifiers,
    Boolean includeClassOrigin,
    Boolean setFlags)
{
    if (setFlags)
    {
        // set CMPI invocation flags
        CMPIValue value;
        value.uint32 = 0;
        if (includeQualifiers) value.uint32 |= CMPI_FLAG_IncludeQualifiers;
        if (includeClassOrigin) value.uint32 |= CMPI_FLAG_IncludeClassOrigin;
        eCtx->ft->addEntry(
            eCtx,
            CMPIInvocationFlags,
            &value,
            CMPI_uint32);
    }

    // add identity context
    const IdentityContainer container =
    context->get(IdentityContainer::NAME);
    eCtx->ft->addEntry(
        eCtx,
        CMPIPrincipal,
        (CMPIValue*)(const char*)container.getUserName().getCString(),
        CMPI_chars);

    // add AcceptLanguages to CMPI context
    const AcceptLanguageListContainer accept_language=
    context->get(AcceptLanguageListContainer::NAME);
    const AcceptLanguageList acceptLangs = accept_language.getLanguages();

    eCtx->ft->addEntry(
        eCtx,
        CMPIAcceptLanguage,
        (CMPIValue*)(const char*)
            LanguageParser::buildAcceptLanguageHeader(acceptLangs).getCString(),
        CMPI_chars);

    // add initial namespace to context
    eCtx->ft->addEntry(
        eCtx,
        CMPIInitNameSpace,
        (CMPIValue*)(const char*)(*nameSpace),
        CMPI_chars);

    // add remote info to context
    if (remote)
    {
        eCtx->ft->addEntry(
            eCtx,
            "CMPIRRemoteInfo",(CMPIValue*)(const char*)(*remoteInfo),
            CMPI_chars);
    }

    // add User Role from OperationContext to CMPIRole

    if (context->contains(UserRoleContainer::NAME))
    {
        const UserRoleContainer container=context->get(UserRoleContainer::NAME);

        CString userRole = container.getUserRole().getCString();

        eCtx->ft->addEntry(
            eCtx,
            CMPIRole,
            (CMPIValue*)(const char*) userRole,
            CMPI_chars);
    }
}

/*
   Function resolves the provider name and gets the cached or loads new
   provider module, also returns if operation is remote and the remote
   information
*/
CMPIProvider & CMPIProviderManager::_resolveAndGetProvider(
    OperationContext * context,
    OpProviderHolder * ph,
    CString * remoteInfo,
    Boolean & isRemote)
{
        isRemote=false;

        // resolve provider name
        ProviderIdContainer pidc =
            context->get(ProviderIdContainer::NAME);

        ProviderName name = _resolveProviderName(pidc);

        if ((isRemote=pidc.isRemoteNameSpace()))
        {
            *ph = providerManager.getRemoteProvider(
                name.getLocation(),
                name.getLogicalName(),
                name.getModuleName());
        }
        else
        {
            // get cached or load new provider module
            *ph = providerManager.getProvider(
                name.getPhysicalName(),
                name.getLogicalName(),
                name.getModuleName());
        }
        *remoteInfo = pidc.getRemoteInfo().getCString();

        // forward request
        return ph->GetProvider();
}

Message * CMPIProviderManager::handleGetInstanceRequest(
    const Message * message)
{
    PEG_METHOD_ENTER(
        TRC_PROVIDERMANAGER,
        "CMPIProviderManager::handleGetInstanceRequest()");

    HandlerIntro(GetInstance,message,request,response,handler);

    try
    {
        PEG_TRACE((TRC_PROVIDERMANAGER,Tracer::LEVEL3,
            "CMPIProviderManager::handleGetInstanceRequest - Host name:"
            " %s  Name space: %s  Class name: %s",
            (const char*) System::getHostName().getCString(),
            (const char*) request->nameSpace.getString().getCString(),
            (const char*)
                request->instanceName.getClassName().getString().getCString()));

        Boolean remote=false;
        OpProviderHolder ph;
        CString remoteInfo;

        CMPIProvider & pr = _resolveAndGetProvider(
            &(request->operationContext),
            &ph,
            &remoteInfo,
            remote);

        CMPIStatus rc={CMPI_RC_OK,NULL};
        CMPI_ContextOnStack eCtx(request->operationContext);
        CMPI_ResultOnStack eRes(handler,pr.getBroker());
        CMPI_ThreadContext thr(pr.getBroker(),&eCtx);

        CMPIPropertyList props(request->propertyList);
        CString nameSpace = request->nameSpace.getString().getCString();
        CString className = request->className.getString().getCString();

        _setupCMPIContexts(
            &eCtx,
            &(request->operationContext),
            &nameSpace,
            &remoteInfo,
            remote,
            request->includeQualifiers,
            request->includeClassOrigin,
            true);


        SCMOInstance * objectPath = getSCMOObjectPathFromRequest(
            nameSpace, className, request->instanceName);

        CMPI_ObjectPathOnStack eRef(objectPath);

        CMPIProvider::pm_service_op_lock op_lock(&pr);

        PEG_TRACE((
            TRC_PROVIDERMANAGER,
            Tracer::LEVEL2,
            "Calling provider.getInstance: %s",
            (const char*)pr.getName().getCString()));

        {
            AutoPThreadSecurity threadLevelSecurity(request->operationContext);

            StatProviderTimeMeasurement providerTime(response);

            rc = pr.getInstMI()->ft->getInstance(
                pr.getInstMI(),
                &eCtx,
                &eRes,
                &eRef,
                (const char **)props.getList());
        }

        PEG_TRACE((
            TRC_PROVIDERMANAGER,
            Tracer::LEVEL2,
            "Returning from provider.getInstance: %s",
            (const char*)pr.getName().getCString()));

//      Need to save ContentLanguage value into operation context of response
//      Do this before checking rc from provider to throw exception in case
//      rc.msg is also localized.
        CMPIStatus tmprc={CMPI_RC_OK,NULL};
        CMPIData cldata =
            eCtx.ft->getEntry (&eCtx, CMPIContentLanguage, &tmprc);
        if (tmprc.rc == CMPI_RC_OK)
        {
            response->operationContext.set(
                ContentLanguageListContainer(
                ContentLanguageList(
                LanguageParser::parseContentLanguageHeader(
                CMGetCharsPtr(cldata.value.string, NULL)))));
            handler.setContext(response->operationContext);
        }

        _throwCIMException(rc, eRes.resError);
    }
    HandlerCatch(handler);

    PEG_METHOD_EXIT();

    return(response);
}

Message * CMPIProviderManager::handleEnumerateInstancesRequest(
    const Message * message)
{
    PEG_METHOD_ENTER(
        TRC_PROVIDERMANAGER,
        "CMPIProviderManager::handleEnumerateInstanceRequest()");

    HandlerIntro(EnumerateInstances,message,request,response,handler);
    try
    {
        PEG_TRACE((TRC_PROVIDERMANAGER,Tracer::LEVEL3,
            "CMPIProviderManager::handleEnumerateInstancesRequest - Host name:"
            " %s  Name space: %s  Class name: %s",
            (const char*) System::getHostName().getCString(),
            (const char*) request->nameSpace.getString().getCString(),
            (const char*) request->className.getString().getCString()));


        Boolean remote=false;
        OpProviderHolder ph;
        CString remoteInfo;

        CMPIProvider & pr = _resolveAndGetProvider(
            &(request->operationContext),
            &ph,
            &remoteInfo,
            remote);

        CIMPropertyList propertyList(request->propertyList);

        CMPIStatus rc={CMPI_RC_OK,NULL};
        CMPI_ContextOnStack eCtx(request->operationContext);
        CMPI_ResultOnStack eRes(handler,pr.getBroker());
        CMPI_ThreadContext thr(pr.getBroker(),&eCtx);
        CMPIPropertyList props(propertyList);
        CString nameSpace = request->nameSpace.getString().getCString();
        CString className = request->className.getString().getCString();

        _setupCMPIContexts(
            &eCtx,
            &(request->operationContext),
            &nameSpace,
            &remoteInfo,
            remote,
            request->includeQualifiers,
            request->includeClassOrigin,
            true);

        SCMOInstance * objectPath =
            getSCMOClassFromRequest(nameSpace, className);

        CMPI_ObjectPathOnStack eRef(objectPath);

        CMPIProvider::pm_service_op_lock op_lock(&pr);

        PEG_TRACE((
            TRC_PROVIDERMANAGER,
            Tracer::LEVEL2,
            "Calling provider.enumerateInstances: %s",
             (const char*)pr.getName().getCString()));

        {
            AutoPThreadSecurity threadLevelSecurity(request->operationContext);

            StatProviderTimeMeasurement providerTime(response);

            rc = pr.getInstMI()->ft->enumerateInstances(
                pr.getInstMI(),
                &eCtx,
                &eRes,
                &eRef,
                (const char **)props.getList());
        }

        PEG_TRACE((
            TRC_PROVIDERMANAGER,
            Tracer::LEVEL2,
            "Returning from provider.enumerateInstances: %s",
             (const char*)pr.getName().getCString()));

//      Need to save ContentLanguage value into operation context of response
//      Do this before checking rc from provider to throw exception in case
//      rc.msg is also localized.
        CMPIStatus tmprc={CMPI_RC_OK,NULL};
        CMPIData cldata =
            eCtx.ft->getEntry(&eCtx, CMPIContentLanguage, &tmprc);
        if (tmprc.rc == CMPI_RC_OK)
        {
            response->operationContext.set(
                ContentLanguageListContainer(
                ContentLanguageList(
                LanguageParser::parseContentLanguageHeader(
                CMGetCharsPtr(cldata.value.string, NULL)))));
            handler.setContext(response->operationContext);
        }

        _throwCIMException(rc, eRes.resError);
    }
    HandlerCatch(handler);

    PEG_METHOD_EXIT();

    return(response);
}

Message * CMPIProviderManager::handleEnumerateInstanceNamesRequest(
    const Message * message)
{
    PEG_METHOD_ENTER(
        TRC_PROVIDERMANAGER,
        "CMPIProviderManager::handleEnumerateInstanceNamesRequest()");

    HandlerIntro(EnumerateInstanceNames,message,request,response,handler);
    try
    {
        PEG_TRACE((TRC_PROVIDERMANAGER,Tracer::LEVEL3,
            "CMPIProviderManager::handleEnumerateInstanceNamesRequest"
            " - Host name: %s  Name space: %s  Class name: %s",
            (const char*) System::getHostName().getCString(),
            (const char*) request->nameSpace.getString().getCString(),
            (const char*) request->className.getString().getCString()));

        Boolean remote=false;
        OpProviderHolder ph;
        CString remoteInfo;

        CMPIProvider & pr = _resolveAndGetProvider(
            &(request->operationContext),
            &ph,
            &remoteInfo,
            remote);

        CMPIStatus rc={CMPI_RC_OK,NULL};
        CMPI_ContextOnStack eCtx(request->operationContext);
        CMPI_ResultOnStack eRes(handler,pr.getBroker());
        CMPI_ThreadContext thr(pr.getBroker(),&eCtx);

        CString nameSpace = request->nameSpace.getString().getCString();
        CString className = request->className.getString().getCString();

        _setupCMPIContexts(
            &eCtx,
            &(request->operationContext),
            &nameSpace,
            &remoteInfo,
            remote,
            false,
            false,
            true);

        SCMOInstance * objectPath =
            getSCMOClassFromRequest(nameSpace, className);

        CMPI_ObjectPathOnStack eRef(objectPath);

        CMPIProvider::pm_service_op_lock op_lock(&pr);

        PEG_TRACE((
            TRC_PROVIDERMANAGER,
            Tracer::LEVEL2,
            "Calling provider.enumerateInstanceNames: %s",
            (const char*)pr.getName().getCString()));

        {
            AutoPThreadSecurity threadLevelSecurity(request->operationContext);

            StatProviderTimeMeasurement providerTime(response);

            rc = pr.getInstMI()->ft->enumerateInstanceNames(
                pr.getInstMI(),
                &eCtx,
                &eRes,
                &eRef);
        }

        PEG_TRACE((
            TRC_PROVIDERMANAGER,
            Tracer::LEVEL2,
            "Returning from provider.enumerateInstanceNames: %s",
            (const char*)pr.getName().getCString()));

//      Need to save ContentLanguage value into operation context of response
//      Do this before checking rc from provider to throw exception in case
//      rc.msg is also localized.
        CMPIStatus tmprc={CMPI_RC_OK,NULL};
        CMPIData cldata =
            eCtx.ft->getEntry (&eCtx, CMPIContentLanguage, &tmprc);
        if (tmprc.rc == CMPI_RC_OK)
        {
            response->operationContext.set(
                ContentLanguageListContainer(
                ContentLanguageList(
                LanguageParser::parseContentLanguageHeader(
                CMGetCharsPtr(cldata.value.string, NULL)))));
            handler.setContext(response->operationContext);
        }
        _throwCIMException(rc, eRes.resError);
    }
    HandlerCatch(handler);

    PEG_METHOD_EXIT();

    return(response);
}

Message * CMPIProviderManager::handleCreateInstanceRequest(
    const Message * message)
{
    PEG_METHOD_ENTER(
        TRC_PROVIDERMANAGER,
        "CMPIProviderManager::handleCreateInstanceRequest()");

    HandlerIntro(CreateInstance,message,request,response,handler);
    try
    {
        PEG_TRACE((TRC_PROVIDERMANAGER,Tracer::LEVEL3,
            "CMPIProviderManager::handleCreateInstanceRequest"
            " - Host name: %s  Name space: %s  Class name: %s",
            (const char*) System::getHostName().getCString(),
            (const char*) request->nameSpace.getString().getCString(),
            (const char*)
          request->newInstance.getPath().getClassName().getString().getCString()
        ));

        Boolean remote=false;
        OpProviderHolder ph;
        CString remoteInfo;

        CMPIProvider & pr = _resolveAndGetProvider(
            &(request->operationContext),
            &ph,
            &remoteInfo,
            remote);

        CMPIStatus rc={CMPI_RC_OK,NULL};
        CMPI_ContextOnStack eCtx(request->operationContext);
        CMPI_ResultOnStack eRes(handler,pr.getBroker());
        CMPI_ThreadContext thr(pr.getBroker(),&eCtx);

        CString nameSpace = request->nameSpace.getString().getCString();
        CString className = request->newInstance.getPath().getClassName().
            getString().getCString();

        _setupCMPIContexts(
            &eCtx,
            &(request->operationContext),
            &nameSpace,
            &remoteInfo,
            remote,
            false,
            false,
            true);

        SCMOInstance * newInstance = getSCMOInstanceFromRequest(
            nameSpace, className, request->newInstance);
        CMPI_InstanceOnStack eInst(newInstance);

        // This will create a second reference for the same SCMOInstance
        CMPI_ObjectPathOnStack eRef(*newInstance);

        CMPIProvider::pm_service_op_lock op_lock(&pr);

        PEG_TRACE((
            TRC_PROVIDERMANAGER,
            Tracer::LEVEL2,
            "Calling provider.createInstance: %s",
            (const char*)ph.GetProvider().getName().getCString()));

        {
            AutoPThreadSecurity threadLevelSecurity(request->operationContext);

            StatProviderTimeMeasurement providerTime(response);

            rc = pr.getInstMI()->ft->createInstance(
                pr.getInstMI(),
                &eCtx,
                &eRes,
                &eRef,
                &eInst);
        }

        PEG_TRACE((
            TRC_PROVIDERMANAGER,
            Tracer::LEVEL2,
            "Returning from provider.createInstance: %s",
            (const char*)ph.GetProvider().getName().getCString()));

//      Need to save ContentLanguage value into operation context of response
//      Do this before checking rc from provider to throw exception in case
//      rc.msg is also localized.
        CMPIStatus tmprc={CMPI_RC_OK,NULL};
        CMPIData cldata =
            eCtx.ft->getEntry (&eCtx, CMPIContentLanguage, &tmprc);
        if (tmprc.rc == CMPI_RC_OK)
        {
            response->operationContext.set(
                ContentLanguageListContainer(
                ContentLanguageList(
                LanguageParser::parseContentLanguageHeader(
                CMGetCharsPtr(cldata.value.string, NULL)))));
            handler.setContext(response->operationContext);
        }
        _throwCIMException(rc, eRes.resError);
    }
    HandlerCatch(handler);

    PEG_METHOD_EXIT();

    return(response);
}

Message * CMPIProviderManager::handleModifyInstanceRequest(
    const Message * message)
{
    PEG_METHOD_ENTER(
        TRC_PROVIDERMANAGER,
        "CMPIProviderManager::handleModifyInstanceRequest()");

    HandlerIntro(ModifyInstance,message,request,response,handler);
    try
    {
        PEG_TRACE((TRC_PROVIDERMANAGER,Tracer::LEVEL3,
            "CMPIProviderManager::handleModifyInstanceRequest"
            " - Host name: %s  Name space: %s  Class name: %s",
            (const char*) System::getHostName().getCString(),
            (const char*) request->nameSpace.getString().getCString(),
            (const char*) request->modifiedInstance.\
                getPath().getClassName().getString().getCString()));

        Boolean remote=false;
        OpProviderHolder ph;
        CString remoteInfo;

        CMPIProvider & pr = _resolveAndGetProvider(
            &(request->operationContext),
            &ph,
            &remoteInfo,
            remote);

        CMPIStatus rc={CMPI_RC_OK,NULL};
        CMPI_ContextOnStack eCtx(request->operationContext);
        CMPI_ResultOnStack eRes(handler,pr.getBroker());
        CMPI_ThreadContext thr(pr.getBroker(),&eCtx);

        // Note: During the CIMInstance to SCMOInstance convertion all schema
        // defined properties are added with null/default values. This causes
        // the provider to set null values to the properties those does not
        // exist in the original CIMInstance. Build the property if the client
        // specified list is empty.
        if (request->propertyList.isNull())
        {
            Array<CIMName> propArry;
            for (Uint32 i = 0, n = request->modifiedInstance.getPropertyCount();
                i < n ; ++i)
            {
                CIMConstProperty prop =
                    request->modifiedInstance.getProperty(i);
                propArry.append(prop.getName());
            }
            request->propertyList = CIMPropertyList(propArry);
        }

        CMPIPropertyList props(request->propertyList);

        CString nameSpace = request->nameSpace.getString().getCString();
        CString className = request->modifiedInstance.getPath().getClassName().
            getString().getCString();

        _setupCMPIContexts(
            &eCtx,
            &(request->operationContext),
            &nameSpace,
            &remoteInfo,
            remote,
            request->includeQualifiers,
            false,
            true);


        SCMOInstance * modInstance = getSCMOInstanceFromRequest(
            nameSpace, className, request->modifiedInstance);
        CMPI_InstanceOnStack eInst(modInstance);

        // This will create a second reference for the same SCMOInstance
        CMPI_ObjectPathOnStack eRef(*modInstance);

        CMPIProvider::pm_service_op_lock op_lock(&pr);

        PEG_TRACE((
            TRC_PROVIDERMANAGER,
            Tracer::LEVEL2,
            "Calling provider.modifyInstance: %s",
            (const char*)pr.getName().getCString()));

        {
            AutoPThreadSecurity threadLevelSecurity(request->operationContext);

            StatProviderTimeMeasurement providerTime(response);

            rc = pr.getInstMI()->ft->modifyInstance(
                pr.getInstMI(),
                &eCtx,
                &eRes,
                &eRef,
                &eInst,
                (const char **)props.getList());
        }

        PEG_TRACE((
            TRC_PROVIDERMANAGER,
            Tracer::LEVEL2,
            "Returning from provider.modifyInstance: %s",
            (const char*)pr.getName().getCString()));

//      Need to save ContentLanguage value into operation context of response
//      Do this before checking rc from provider to throw exception in case
//      rc.msg is also localized.
        CMPIStatus tmprc={CMPI_RC_OK,NULL};
        CMPIData cldata =
            eCtx.ft->getEntry (&eCtx, CMPIContentLanguage, &tmprc);
        if (tmprc.rc == CMPI_RC_OK)
        {
            response->operationContext.set(
                ContentLanguageListContainer(
                ContentLanguageList(
                LanguageParser::parseContentLanguageHeader(
                CMGetCharsPtr(cldata.value.string, NULL)))));
            handler.setContext(response->operationContext);
        }
        _throwCIMException(rc, eRes.resError);
    }
    HandlerCatch(handler);

    PEG_METHOD_EXIT();

    return(response);
}

Message * CMPIProviderManager::handleDeleteInstanceRequest(
    const Message * message)
{
    PEG_METHOD_ENTER(
        TRC_PROVIDERMANAGER,
        "CMPIProviderManager::handleDeleteInstanceRequest()");

    HandlerIntro(DeleteInstance,message,request,response,handler);
    try
    {
        PEG_TRACE((TRC_PROVIDERMANAGER,Tracer::LEVEL3,
            "CMPIProviderManager::handleDeleteInstanceRequest"
            " - Host name: %s  Name space: %s  Class name: %s",
            (const char*) System::getHostName().getCString(),
            (const char*) request->nameSpace.getString().getCString(),
            (const char*)
                request->instanceName.getClassName().getString().getCString()));

        Boolean remote=false;
        OpProviderHolder ph;
        CString remoteInfo;

        CMPIProvider & pr = _resolveAndGetProvider(
            &(request->operationContext),
            &ph,
            &remoteInfo,
            remote);

        CMPIStatus rc={CMPI_RC_OK,NULL};
        CMPI_ContextOnStack eCtx(request->operationContext);
        CMPI_ResultOnStack eRes(handler,pr.getBroker());
        CMPI_ThreadContext thr(pr.getBroker(),&eCtx);

        CString nameSpace = request->nameSpace.getString().getCString();
        CString className = request->instanceName.getClassName().
            getString().getCString();

        _setupCMPIContexts(
            &eCtx,
            &(request->operationContext),
            &nameSpace,
            &remoteInfo,
            remote,
            false,
            false,
            true);

        SCMOInstance * objectPath = getSCMOObjectPathFromRequest(
            nameSpace, className, request->instanceName);

        CMPI_ObjectPathOnStack eRef(objectPath);

        CMPIProvider::pm_service_op_lock op_lock(&pr);

        PEG_TRACE((
            TRC_PROVIDERMANAGER,
            Tracer::LEVEL2,
            "Calling provider.deleteInstance: %s",
            (const char*)pr.getName().getCString()));

        {
            AutoPThreadSecurity threadLevelSecurity(request->operationContext);

            StatProviderTimeMeasurement providerTime(response);

            rc = pr.getInstMI()->ft->deleteInstance(
                pr.getInstMI(),
                &eCtx,
                &eRes,
                &eRef);
        }

        PEG_TRACE((
            TRC_PROVIDERMANAGER,
            Tracer::LEVEL2,
            "Returning from provider.deleteInstance: %s",
            (const char*)pr.getName().getCString()));

//      Need to save ContentLanguage value into operation context of response
//      Do this before checking rc from provider to throw exception in case
//      rc.msg is also localized.
        CMPIStatus tmprc={CMPI_RC_OK,NULL};
        CMPIData cldata =
            eCtx.ft->getEntry (&eCtx, CMPIContentLanguage, &tmprc);
        if (tmprc.rc == CMPI_RC_OK)
        {
            response->operationContext.set(
                ContentLanguageListContainer(
                ContentLanguageList(
                LanguageParser::parseContentLanguageHeader(
                CMGetCharsPtr(cldata.value.string, NULL)))));
            handler.setContext(response->operationContext);
        }
        _throwCIMException(rc, eRes.resError);
    }
    HandlerCatch(handler);

    PEG_METHOD_EXIT();

    return(response);
}

Message * CMPIProviderManager::handleExecQueryRequest(const Message * message)
{
    PEG_METHOD_ENTER(
        TRC_PROVIDERMANAGER,
        "CMPIProviderManager::handleExecQueryRequest()");

    HandlerIntro(ExecQuery,message,request,response,handler);

    try
    {
        PEG_TRACE((TRC_PROVIDERMANAGER,Tracer::LEVEL3,
            "CMPIProviderManager::ExecQueryRequest"
            " - Host name: %s  Name space: %s  Class name: %s",
            (const char*) System::getHostName().getCString(),
            (const char*) request->nameSpace.getString().getCString(),
            (const char*) request->className.getString().getCString()));

        Boolean remote=false;
        OpProviderHolder ph;
        CString remoteInfo;

        CMPIProvider & pr = _resolveAndGetProvider(
            &(request->operationContext),
            &ph,
            &remoteInfo,
            remote);

        CMPIStatus rc={CMPI_RC_OK,NULL};
        CMPI_ContextOnStack eCtx(request->operationContext);
        CMPI_ResultOnStack eRes(handler,pr.getBroker());
        CMPI_ThreadContext thr(pr.getBroker(),&eCtx);

        const CString queryLan=request->queryLanguage.getCString();
        const CString query=request->query.getCString();
        CString nameSpace = request->nameSpace.getString().getCString();
        CString className = request->className.getString().getCString();

        _setupCMPIContexts(
            &eCtx,
            &(request->operationContext),
            &nameSpace,
            &remoteInfo,
            remote,
            false,
            false,
            true);

        SCMOInstance * classPath = getSCMOClassFromRequest(nameSpace,className);
        CMPI_ObjectPathOnStack eRef(classPath);

        CMPIProvider::pm_service_op_lock op_lock(&pr);

        PEG_TRACE((
            TRC_PROVIDERMANAGER,
            Tracer::LEVEL2,
            "Calling provider.execQuery: %s",
            (const char*)pr.getName().getCString()));

        {
            AutoPThreadSecurity threadLevelSecurity(request->operationContext);

            StatProviderTimeMeasurement providerTime(response);

            rc = pr.getInstMI()->ft->execQuery(
                pr.getInstMI(),
                &eCtx,
                &eRes,
                &eRef,
                CHARS(queryLan),
                CHARS(query));
        }

        PEG_TRACE((
            TRC_PROVIDERMANAGER,
            Tracer::LEVEL2,
            "Returning from provider.execQuery: %s",
            (const char*)pr.getName().getCString()));

//      Need to save ContentLanguage value into operation context of response
//      Do this before checking rc from provider to throw exception in case
//      rc.msg is also localized.
        CMPIStatus tmprc={CMPI_RC_OK,NULL};
        CMPIData cldata =
            eCtx.ft->getEntry (&eCtx, CMPIContentLanguage, &tmprc);
        if (tmprc.rc == CMPI_RC_OK)
        {
            response->operationContext.set(
                ContentLanguageListContainer(
                ContentLanguageList(
                LanguageParser::parseContentLanguageHeader(
                CMGetCharsPtr(cldata.value.string, NULL)))));
            handler.setContext(response->operationContext);
        }
        _throwCIMException(rc, eRes.resError);

    }
    HandlerCatch(handler);

    PEG_METHOD_EXIT();

    return(response);
}

Message * CMPIProviderManager::handleAssociatorsRequest(const Message * message)
{
    PEG_METHOD_ENTER(
        TRC_PROVIDERMANAGER,
        "CMPIProviderManager::handleAssociatorsRequest()");

    HandlerIntro(Associators,message,request,response,handler);
    try
    {
        PEG_TRACE((TRC_PROVIDERMANAGER,Tracer::LEVEL3,
            "CMPIProviderManager::handleAssociatorsRequest"
            " - Host name: %s  Name space: %s  Class name: %s",
            (const char*) System::getHostName().getCString(),
            (const char*) request->nameSpace.getString().getCString(),
            (const char*)
                request->objectName.getClassName().getString().getCString()));

        Boolean remote=false;
        OpProviderHolder ph;
        CString remoteInfo;

        CMPIProvider & pr = _resolveAndGetProvider(
            &(request->operationContext),
            &ph,
            &remoteInfo,
            remote);

        PEG_TRACE((TRC_PROVIDERMANAGER,Tracer::LEVEL4,
            "--- CMPIProviderManager::associators < role: > %s%s",
            (const char*)request->role.getCString(),
            (const char*)request->assocClass.getString().getCString()));

        CMPIStatus rc={CMPI_RC_OK,NULL};
        CMPI_ContextOnStack eCtx(request->operationContext);
        CMPI_ResultOnStack eRes(handler,pr.getBroker());
        CMPI_ThreadContext thr(pr.getBroker(),&eCtx);
        const CString aClass=request->assocClass.getString().getCString();
        const CString rClass=request->resultClass.getString().getCString();
        const CString rRole=request->role.getCString();
        const CString resRole=request->resultRole.getCString();
        CString nameSpace = request->nameSpace.getString().getCString();
        CString className =
            request->objectName.getClassName().getString().getCString();

        CMPIPropertyList props(request->propertyList);

        _setupCMPIContexts(
            &eCtx,
            &(request->operationContext),
            &nameSpace,
            &remoteInfo,
            remote,
            request->includeQualifiers,
            request->includeClassOrigin,
            true);

        SCMOInstance * objectPath = getSCMOObjectPathFromRequest(
            nameSpace, className, request->objectName);

        CMPI_ObjectPathOnStack eRef(objectPath);

        CMPIProvider::pm_service_op_lock op_lock(&pr);

        PEG_TRACE((
            TRC_PROVIDERMANAGER,
            Tracer::LEVEL2,
            "Calling provider.associators: %s",
            (const char*)pr.getName().getCString()));

        {
            AutoPThreadSecurity threadLevelSecurity(request->operationContext);

            StatProviderTimeMeasurement providerTime(response);

            rc = pr.getAssocMI()->ft->associators(
                pr.getAssocMI(),
                &eCtx,
                &eRes,
                &eRef,
                CHARS(aClass),
                CHARS(rClass),
                CHARS(rRole),
                CHARS(resRole),
                (const char **)props.getList());
        }

        PEG_TRACE((
            TRC_PROVIDERMANAGER,
            Tracer::LEVEL2,
            "Returning from provider.associators: %s",
            (const char*)pr.getName().getCString()));

//      Need to save ContentLanguage value into operation context of response
//      Do this before checking rc from provider to throw exception in case
//      rc.msg is also localized.
        CMPIStatus tmprc={CMPI_RC_OK,NULL};
        CMPIData cldata = eCtx.ft->getEntry (
            &eCtx,
            CMPIContentLanguage,
            &tmprc);
        if (tmprc.rc == CMPI_RC_OK)
        {
            response->operationContext.set(
                ContentLanguageListContainer(
                ContentLanguageList(
                LanguageParser::parseContentLanguageHeader(
                CMGetCharsPtr(cldata.value.string, NULL)))));
            handler.setContext(response->operationContext);
        }
        _throwCIMException(rc, eRes.resError);
    }
    HandlerCatch(handler);

    PEG_METHOD_EXIT();

    return(response);
}

Message * CMPIProviderManager::handleAssociatorNamesRequest(
    const Message * message)
{
    PEG_METHOD_ENTER(
        TRC_PROVIDERMANAGER,
        "CMPIProviderManager::handleAssociatorNamesRequest()");

    HandlerIntro(AssociatorNames,message,request,response,handler);
    try
    {
        PEG_TRACE((TRC_PROVIDERMANAGER,Tracer::LEVEL3,
            "CMPIProviderManager::handleAssociatorNamesRequest"
            " - Host name: %s  Name space: %s  Class name: %s",
            (const char*) System::getHostName().getCString(),
            (const char*) request->nameSpace.getString().getCString(),
            (const char*)
                request->objectName.getClassName().getString().getCString()));

        Boolean remote=false;
        OpProviderHolder ph;
        CString remoteInfo;

        CMPIProvider & pr = _resolveAndGetProvider(
            &(request->operationContext),
            &ph,
            &remoteInfo,
            remote);

        PEG_TRACE((TRC_PROVIDERMANAGER,Tracer::LEVEL4,
            "--- CMPIProviderManager::associatorNames --  role: %s< aCls %s",
            (const char*)request->role.getCString(),
            (const char*)request->assocClass.getString().getCString()));

        CMPIStatus rc={CMPI_RC_OK,NULL};
        CMPI_ContextOnStack eCtx(request->operationContext);
        CMPI_ResultOnStack eRes(handler,pr.getBroker());
        CMPI_ThreadContext thr(pr.getBroker(),&eCtx);
        const CString aClass=request->assocClass.getString().getCString();
        const CString rClass=request->resultClass.getString().getCString();
        const CString rRole=request->role.getCString();
        const CString resRole=request->resultRole.getCString();
        CString nameSpace = request->nameSpace.getString().getCString();
        CString className =
            request->objectName.getClassName().getString().getCString();

        _setupCMPIContexts(
            &eCtx,
            &(request->operationContext),
            &nameSpace,
            &remoteInfo,
            remote,
            false,
            false,
            true);

        SCMOInstance * objectPath = getSCMOObjectPathFromRequest(
            nameSpace, className, request->objectName);

        CMPI_ObjectPathOnStack eRef(objectPath);

        CMPIProvider::pm_service_op_lock op_lock(&pr);

        PEG_TRACE((
            TRC_PROVIDERMANAGER,
            Tracer::LEVEL2,
            "Calling provider.associatorNames: %s",
            (const char*)pr.getName().getCString()));

        {
            AutoPThreadSecurity threadLevelSecurity(request->operationContext);

            StatProviderTimeMeasurement providerTime(response);

            rc = pr.getAssocMI()->ft->associatorNames(
                pr.getAssocMI(),
                &eCtx,
                &eRes,
                &eRef,
                CHARS(aClass),
                CHARS(rClass),
                CHARS(rRole),
                CHARS(resRole));
        }

        PEG_TRACE((
            TRC_PROVIDERMANAGER,
            Tracer::LEVEL2,
            "Returning from provider.associatorNames: %s",
            (const char*)pr.getName().getCString()));

//      Need to save ContentLanguage value into operation context of response
//      Do this before checking rc from provider to throw exception in case
//      rc.msg is also localized.
        CMPIStatus tmprc={CMPI_RC_OK,NULL};
        CMPIData cldata =
            eCtx.ft->getEntry (&eCtx, CMPIContentLanguage, &tmprc);
        if (tmprc.rc == CMPI_RC_OK)
        {
            response->operationContext.set(
                ContentLanguageListContainer(
                ContentLanguageList(
                LanguageParser::parseContentLanguageHeader(
                CMGetCharsPtr(cldata.value.string, NULL)))));
            handler.setContext(response->operationContext);
        }
        _throwCIMException(rc, eRes.resError);
    }
    HandlerCatch(handler);

    PEG_METHOD_EXIT();

    return(response);
}

Message * CMPIProviderManager::handleReferencesRequest(const Message * message)
{
    PEG_METHOD_ENTER(
        TRC_PROVIDERMANAGER,
        "CMPIProviderManager::handleReferencesRequest()");

    HandlerIntro(References,message,request,response,handler);
    try
    {
        PEG_TRACE((TRC_PROVIDERMANAGER,Tracer::LEVEL3,
            "CMPIProviderManager::handleReferencesRequest"
            " - Host name: %s  Name space: %s  Class name: %s",
            (const char*) System::getHostName().getCString(),
            (const char*) request->nameSpace.getString().getCString(),
            (const char*)
                request->objectName.getClassName().getString().getCString()));

        Boolean remote=false;
        OpProviderHolder ph;
        CString remoteInfo;

        CMPIProvider & pr = _resolveAndGetProvider(
            &(request->operationContext),
            &ph,
            &remoteInfo,
            remote);

        PEG_TRACE((TRC_PROVIDERMANAGER,Tracer::LEVEL4,
            "--- CMPIProviderManager::references -- role:%s< aCls %s",
            (const char*)request->role.getCString(),
            (const char*)request->resultClass.getString().getCString()));

        CMPIStatus rc={CMPI_RC_OK,NULL};
        CMPI_ContextOnStack eCtx(request->operationContext);
        CMPI_ResultOnStack eRes(handler,pr.getBroker());
        CMPI_ThreadContext thr(pr.getBroker(),&eCtx);
        const CString rClass=request->resultClass.getString().getCString();
        const CString rRole=request->role.getCString();
        CString nameSpace = request->nameSpace.getString().getCString();
        CString className =
            request->objectName.getClassName().getString().getCString();

        CMPIPropertyList props(request->propertyList);

        _setupCMPIContexts(
            &eCtx,
            &(request->operationContext),
            &nameSpace,
            &remoteInfo,
            remote,
            request->includeQualifiers,
            request->includeClassOrigin,
            true);

        SCMOInstance * objectPath = getSCMOObjectPathFromRequest(
            nameSpace, className, request->objectName);

        CMPI_ObjectPathOnStack eRef(objectPath);

        CMPIProvider::pm_service_op_lock op_lock(&pr);

        PEG_TRACE((
            TRC_PROVIDERMANAGER,
            Tracer::LEVEL2,
            "Calling provider.references: %s",
            (const char*)pr.getName().getCString()));

        {
            AutoPThreadSecurity threadLevelSecurity(request->operationContext);

            StatProviderTimeMeasurement providerTime(response);

            rc = pr.getAssocMI()->ft->references(
                pr.getAssocMI(),
                &eCtx,
                &eRes,
                &eRef,
                CHARS(rClass),
                CHARS(rRole),
                (const char **)props.getList());
        }

        PEG_TRACE((
            TRC_PROVIDERMANAGER,
            Tracer::LEVEL2,
            "Returning from provider.references: %s",
            (const char*)pr.getName().getCString()));

//      Need to save ContentLanguage value into operation context of response
//      Do this before checking rc from provider to throw exception in case
//      rc.msg is also localized.
        CMPIStatus tmprc={CMPI_RC_OK,NULL};
        CMPIData cldata =
            eCtx.ft->getEntry (&eCtx, CMPIContentLanguage, &tmprc);
        if (tmprc.rc == CMPI_RC_OK)
        {
            response->operationContext.set(
                ContentLanguageListContainer(
                ContentLanguageList(
                LanguageParser::parseContentLanguageHeader(
                CMGetCharsPtr(cldata.value.string, NULL)))));
            handler.setContext(response->operationContext);
        }
        _throwCIMException(rc, eRes.resError);
    }
    HandlerCatch(handler);

    PEG_METHOD_EXIT();

    return(response);
}

Message * CMPIProviderManager::handleReferenceNamesRequest(
    const Message * message)
{
    PEG_METHOD_ENTER(
        TRC_PROVIDERMANAGER,
        "CMPIProviderManager::handleReferenceNamesRequest()");

    HandlerIntro(ReferenceNames,message,request,response,handler);
    try
    {
        PEG_TRACE((TRC_PROVIDERMANAGER,Tracer::LEVEL3,
            "CMPIProviderManager::handleReferenceNamesRequest"
            " - Host name: %s  Name space: %s  Class name: %s",
            (const char*) System::getHostName().getCString(),
            (const char*) request->nameSpace.getString().getCString(),
            (const char*)
                request->objectName.getClassName().getString().getCString()));

        Boolean remote=false;
        OpProviderHolder ph;
        CString remoteInfo;

        CMPIProvider & pr = _resolveAndGetProvider(
            &(request->operationContext),
            &ph,
            &remoteInfo,
            remote);

        PEG_TRACE((TRC_PROVIDERMANAGER,Tracer::LEVEL4,
            "--- CMPIProviderManager::referenceNames -- role: %s< aCls %s",
            (const char*)request->role.getCString(),
            (const char*)request->resultClass.getString().getCString()));

        CMPIStatus rc={CMPI_RC_OK,NULL};
        CMPI_ContextOnStack eCtx(request->operationContext);
        CMPI_ResultOnStack eRes(handler,pr.getBroker());
        CMPI_ThreadContext thr(pr.getBroker(),&eCtx);
        const CString rClass=request->resultClass.getString().getCString();
        const CString rRole=request->role.getCString();

        CString nameSpace = request->nameSpace.getString().getCString();
        CString className =
            request->objectName.getClassName().getString().getCString();

        _setupCMPIContexts(
            &eCtx,
            &(request->operationContext),
            &nameSpace,
            &remoteInfo,
            remote,
            false,
            false,
            true);

        SCMOInstance * objectPath = getSCMOObjectPathFromRequest(
            nameSpace, className, request->objectName);

        CMPI_ObjectPathOnStack eRef(objectPath);

        CMPIProvider::pm_service_op_lock op_lock(&pr);

        PEG_TRACE((
            TRC_PROVIDERMANAGER,
            Tracer::LEVEL2,
            "Calling provider.referenceNames: %s",
            (const char*)pr.getName().getCString()));

        {
            AutoPThreadSecurity threadLevelSecurity(request->operationContext);

            StatProviderTimeMeasurement providerTime(response);

            rc = pr.getAssocMI()->ft->referenceNames(
                pr.getAssocMI(),
                &eCtx,
                &eRes,
                &eRef,
                CHARS(rClass),
                CHARS(rRole));
        }

        PEG_TRACE((
            TRC_PROVIDERMANAGER,
            Tracer::LEVEL2,
            "Returning from provider.referenceNames: %s",
            (const char*)pr.getName().getCString()));

//      Need to save ContentLanguage value into operation context of response
//      Do this before checking rc from provider to throw exception in case
//      rc.msg is also localized.
        CMPIStatus tmprc={CMPI_RC_OK,NULL};
        CMPIData cldata =
             eCtx.ft->getEntry (&eCtx, CMPIContentLanguage, &tmprc);
        if (tmprc.rc == CMPI_RC_OK)
        {
            response->operationContext.set(
                ContentLanguageListContainer(
                ContentLanguageList(
                LanguageParser::parseContentLanguageHeader(
                CMGetCharsPtr(cldata.value.string, NULL)))));
            handler.setContext(response->operationContext);
        }
        _throwCIMException(rc, eRes.resError);
    }
    HandlerCatch(handler);

    PEG_METHOD_EXIT();

    return(response);
}

Message * CMPIProviderManager::handleInvokeMethodRequest(
    const Message * message)
{
    PEG_METHOD_ENTER(
        TRC_PROVIDERMANAGER,
        "CMPIProviderManager::handleInvokeMethodRequest()");

    HandlerIntro(InvokeMethod,message,request,response,handler);
    try
    {
        PEG_TRACE((TRC_PROVIDERMANAGER,Tracer::LEVEL3,
            "CMPIProviderManager::handleInvokeMethodRequest"
            " - Host name: %s  Name space: %s  Class name: %s",
            (const char*) System::getHostName().getCString(),
            (const char*) request->nameSpace.getString().getCString(),
            (const char*)
                request->instanceName.getClassName().getString().getCString()));

        Boolean remote=false;
        OpProviderHolder ph;
        CString remoteInfo;

        CMPIProvider & pr = _resolveAndGetProvider(
            &(request->operationContext),
            &ph,
            &remoteInfo,
            remote);

        CMPIStatus rc={CMPI_RC_OK,NULL};
        CMPI_ContextOnStack eCtx(request->operationContext);
        CMPI_ResultOnStack eRes(handler,pr.getBroker());
        CMPI_ThreadContext thr(pr.getBroker(),&eCtx);
        CMPI_ArgsOnStack eArgsIn(request->inParameters);
        Array<CIMParamValue> outArgs;
        CMPI_ArgsOnStack eArgsOut(outArgs);
        CString mName=request->methodName.getString().getCString();
        CString nameSpace = request->nameSpace.getString().getCString();
        CString className = request->className.getString().getCString();

        _setupCMPIContexts(
            &eCtx,
            &(request->operationContext),
            &nameSpace,
            &remoteInfo,
            remote,
            false,
            false,
            true);


        SCMOInstance * objectPath = getSCMOObjectPathFromRequest(
            nameSpace, className, request->instanceName);

        CMPI_ObjectPathOnStack eRef(objectPath);

        CMPIProvider::pm_service_op_lock op_lock(&pr);

        PEG_TRACE((
            TRC_PROVIDERMANAGER,
            Tracer::LEVEL2,
            "Calling provider.invokeMethod: %s",
            (const char*)pr.getName().getCString()));

        {
            AutoPThreadSecurity threadLevelSecurity(request->operationContext);

            StatProviderTimeMeasurement providerTime(response);

            rc = pr.getMethMI()->ft->invokeMethod(
                pr.getMethMI(),
                &eCtx,
                &eRes,
                &eRef,
                CHARS(mName),
                &eArgsIn,
                &eArgsOut);
        }

        PEG_TRACE((
            TRC_PROVIDERMANAGER,
            Tracer::LEVEL2,
            "Returning from provider.invokeMethod: %s",
            (const char*)pr.getName().getCString()));

//      Need to save ContentLanguage value into operation context of response
//      Do this before checking rc from provider to throw exception in case
//      rc.msg is also localized.
        CMPIStatus tmprc={CMPI_RC_OK,NULL};
        CMPIData cldata =
            eCtx.ft->getEntry (&eCtx, CMPIContentLanguage, &tmprc);
        if (tmprc.rc == CMPI_RC_OK)
        {
            response->operationContext.set(
                ContentLanguageListContainer(
                ContentLanguageList(
                LanguageParser::parseContentLanguageHeader(
                CMGetCharsPtr(cldata.value.string, NULL)))));
            handler.setContext(response->operationContext);
        }
        _throwCIMException(rc, eRes.resError);

        // Even if external normalization is enabled we don't normalize the
        // Embedded instances present in output args. Normalize them here.
        {
            // There is no try catch here because if there is no external
            // normalization, then these containers were added by this method.
            const CachedClassDefinitionContainer * classCont =
                dynamic_cast<const CachedClassDefinitionContainer *>(
                &request->operationContext.get(
                CachedClassDefinitionContainer::NAME));
            PEGASUS_ASSERT(classCont != 0);

            CIMConstClass classDef(classCont->getClass());
            Uint32 methodIndex = classDef.findMethod(request->methodName);
            CIMConstMethod methodDef;
            if (methodIndex != PEG_NOT_FOUND)
            {
                methodDef = classDef.getMethod(methodIndex);
            }
            for (unsigned int i = 0, n = outArgs.size(); i < n; ++i)
            {
                CIMParamValue currentParam(outArgs[i]);
                CIMValue paramValue(currentParam.getValue());
                // If the parameter value is an EmbeddedObject type, we have
                // to check against the type of the parameter definition.
                // CMPI does not distinguish between EmbeddedObjects and
                // EmbeddedInstances, so if the parameter definition has a type
                // of EmbeddedInstance, the type of the output parameter must
                // be changed.
                if (paramValue.getType() == CIMTYPE_OBJECT &&
                    methodIndex != PEG_NOT_FOUND)
                {
                    String currentParamName(currentParam.getParameterName());
                    Uint32 paramIndex = methodDef.findParameter(
                        CIMName(currentParamName));
                    if (paramIndex == PEG_NOT_FOUND)
                    {
                        MessageLoaderParms msg("ProviderManager.CMPI."
                            "CMPIProviderManager.PARAMETER_NOT_FOUND",
                            "Parameter $0 not found in definition for "
                            "method $1.", currentParamName,
                            request->methodName.getString());

                        PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL1,
                            "Parameter %s not found in definition for "
                            "method %s.",
                            (const char*)currentParamName.getCString(),
                            (const char*)
                                request->methodName.getString().getCString()));

                        handler.setStatus(CIM_ERR_FAILED,
                            MessageLoader::getMessage(msg));
                    }
                    else
                    {
                        CIMConstParameter paramDef(
                            methodDef.getParameter(paramIndex));
                        if (paramDef.findQualifier(
                                PEGASUS_QUALIFIERNAME_EMBEDDEDINSTANCE)
                            != PEG_NOT_FOUND)
                        {
                            if (paramValue.isArray())
                            {
                                Array<CIMInstance> paramInstArr;
                                Array<CIMObject> paramObjectArr;
                                paramValue.get(paramObjectArr);
                                for (Uint32 j = 0 ;
                                    j < paramObjectArr.size() ; ++j)
                                {
                                    paramInstArr.append(
                                        CIMInstance(paramObjectArr[j]));
                                }
                                currentParam = CIMParamValue(currentParamName,
                                    CIMValue(paramInstArr));
                            }
                            else
                            {
                                CIMObject paramObject;
                                paramValue.get(paramObject);
                                CIMInstance paramInst(paramObject);
                                currentParam = CIMParamValue(currentParamName,
                                    CIMValue(paramInst));
                            }
                        }
                        else
                        {
                            currentParam = CIMParamValue(currentParamName,
                                paramValue);
                        }

                        handler.deliverParamValue(currentParam);
                    }
                }
                else
                {
                    handler.deliverParamValue(currentParam);
                }
            }
        }
        handler.complete();
    }
    HandlerCatch(handler);

    PEG_METHOD_EXIT();

    return(response);
}

int LocateIndicationProviderNames(
    const CIMInstance& pInstance,
    const CIMInstance& pmInstance,
    String& providerName,
    String& location)
{
   PEG_METHOD_ENTER(
        TRC_PROVIDERMANAGER,
        "CMPIProviderManager:LocateIndicationProviderNames()");
    Uint32 pos = pInstance.findProperty(PEGASUS_PROPERTYNAME_NAME);
    pInstance.getProperty(pos).getValue().get(providerName);

    pos = pmInstance.findProperty(CIMName ("Location"));
    pmInstance.getProperty(pos).getValue().get(location);
    PEG_METHOD_EXIT();
    return 0;
}

String CMPIProviderManager::_getClassNameFromQuery(
    CIMOMHandleQueryContext *context,
    String &query,
    String &lang)
{
    String className;

    try
    {
        QueryExpression qe(lang, query, *context);
        // Neither WQL nor CQL support joins, we should get only
        // one class path here.
        PEGASUS_ASSERT(qe.getClassPathList().size() == 1);
        className = qe.getClassPathList()[0].getClassName().getString();
    }
    catch(QueryException&)
    {
        // We should never get query parsing exceptions, IndicationService
        // already performed this checking.
        PEGASUS_ASSERT(0);
    }
    return className;
}

void CMPIProviderManager::_throwCIMException(
    CMPIStatus rc,
    CMPI_Error* cmpiError)
{
    if (rc.rc!=CMPI_RC_OK)
    {
        CIMException cimException(
            (CIMStatusCode)rc.rc,
            rc.msg ? CMGetCharsPtr(rc.msg, NULL) : String::EMPTY);
        if (cmpiError)
        {
            for (CMPI_Error* currErr=cmpiError;
                currErr!=NULL;
                currErr=currErr->nextError)
            {
                cimException.addError(
                    ((CIMError*)currErr->hdl)->getInstance());
            }
        }
        throw cimException;
    }

}

Message * CMPIProviderManager::handleCreateSubscriptionRequest(
    const Message * message)
{
    PEG_METHOD_ENTER(
        TRC_PROVIDERMANAGER,
        "CMPIProviderManager::handleCreateSubscriptionRequest()");

    HandlerIntroInd(CreateSubscription,message,request,response,
        handler);
    try
    {
        CIMInstance req_provider, req_providerModule;
        ProviderIdContainer pidc =
            (ProviderIdContainer)request->operationContext.get(
            ProviderIdContainer::NAME);
        req_provider = pidc.getProvider();
        req_providerModule = pidc.getModule();

        String providerName,providerLocation;
        LocateIndicationProviderNames(req_provider, req_providerModule,
            providerName,providerLocation);

        PEG_TRACE((TRC_PROVIDERMANAGER,Tracer::LEVEL3,
            "CMPIProviderManager::handleCreateSubscriptionRequest"
            " - Host name: %s  Name space: %s  Provider name(s): %s",
            (const char*) System::getHostName().getCString(),
            (const char*) request->nameSpace.getString().getCString(),
            (const char*) providerName.getCString()));

        Boolean remote=false;
        OpProviderHolder ph;
        CString remoteInfo;

        CMPIProvider & pr = _resolveAndGetProvider(
            &(request->operationContext),
            &ph,
            &remoteInfo,
            remote);

        //
        //  Save the provider instance from the request
        //
        pr.setProviderInstance (req_provider);

        CIMObjectPath subscriptionName =
            request->subscriptionInstance.getPath();

        CMPIStatus rc={CMPI_RC_OK,NULL};
        CMPI_ContextOnStack eCtx(request->operationContext);
        SubscriptionFilterConditionContainer sub_cntr =
            request->operationContext.get(
            SubscriptionFilterConditionContainer::NAME);

        CIMOMHandleQueryContext _context(
            CIMNamespaceName(
            request->nameSpace.getString()),
            *pr.getCIMOMHandle());

        CMPI_SelectExp *eSelx=new CMPI_SelectExp(
            request->operationContext,
            &_context,
            request->query,
            sub_cntr.getQueryLanguage());

        IndProvRecord *indProvRec=NULL;
        const CIMObjectPath &sPath=request->subscriptionInstance.getPath();

        {
            WriteLock lock(rwSemProvTab);
            indProvTab.lookup(pr.getName(), indProvRec);
            if (indProvRec == NULL)
            {
                indProvRec = new IndProvRecord();
#ifdef PEGASUS_ENABLE_REMOTE_CMPI
                if (remote)
                {
                    indProvRec->setRemoteInfo((const char*)remoteInfo);
                }
#endif
                indProvTab.insert(pr.getName(), indProvRec);                
            }
            // Note that per provider subscription path - namespace
            // MUST be unique.
            PEGASUS_FCT_EXECUTE_AND_ASSERT(
                true,
                indProvRec->addSelectExp(sPath, request->nameSpace, eSelx));
        }


        CMPI_ThreadContext thr(pr.getBroker(),&eCtx);

        String lang(sub_cntr.getQueryLanguage());
        CString className = _getClassNameFromQuery(
            &_context,
            request->query,
            lang).getCString();
        CString nameSpace = request->nameSpace.getString().getCString();

        CIMObjectPath indClassPath(
            System::getHostName(),
            request->nameSpace,
            (const char*)className);

        eSelx->classNames.append(indClassPath);

        SCMOInstance * indClassPathSCMO =
            getSCMOClassFromRequest(nameSpace, className);
        eSelx->classNamesSCMO.append(*indClassPathSCMO);
        delete indClassPathSCMO;

        CIMPropertyList propertyList = request->propertyList;
        if (!propertyList.isNull())
        {
            Array<CIMName> p=propertyList.getPropertyNameArray();
            int pCount=p.size();
            eSelx->props = new const char*[1+pCount];
            for (int i=0; i<pCount; i++)
            {
                eSelx->props[i]=strdup(p[i].getString().getCString());
            }
            eSelx->props[pCount]=NULL;
        }

        // includeQualifiers and includeClassOrigin not of interest for
        // this type of request
        _setupCMPIContexts(
            &eCtx,
            &(request->operationContext),
            &nameSpace,
            &remoteInfo,
            remote);

        CMPIProvider::pm_service_op_lock op_lock(&pr);

        PEG_TRACE((
            TRC_PROVIDERMANAGER,
            Tracer::LEVEL2,
            "Calling provider.createSubscriptionRequest: %s",
            (const char*)pr.getName().getCString()));

        Boolean filterActivated = false;
        {
            AutoPThreadSecurity threadLevelSecurity(request->operationContext);

            StatProviderTimeMeasurement providerTime(response);

            // Call activateFilter() on each subclass name, Check if atleast one
            // filter can be activated for any of the subclasses.
            for (Uint32 i = 0, n = request->classNames.size(); i < n; i++)
            {
                CString subClassName =
                    request->classNames[i].getString().getCString();

                // make target object paths
                CIMObjectPath classPath(
                    System::getHostName(),
                    request->nameSpace,
                    request->classNames[i]);

                SCMOInstance * classPathSCMO =
                    getSCMOClassFromRequest(nameSpace, subClassName);
                CMPI_ObjectPathOnStack eRef(classPathSCMO);

                if (pr.getIndMI()->ft->ftVersion >= 100)
                {
                    rc = pr.getIndMI()->ft->activateFilter(
                        pr.getIndMI(),
                        &eCtx,
                        eSelx,
                        CHARS(className),
                        &eRef,
                        i == 0);
                }
                else
                {
                    // Older version of (pre 1.00) also pass in a CMPIResult

                    rc = ((CMPIStatus (*)(
                             CMPIIndicationMI*,
                             CMPIContext*,
                             CMPIResult*,
                             CMPISelectExp*,
                             const char *,
                             CMPIObjectPath*,
                             CMPIBoolean))pr.getIndMI()->ft->activateFilter)
                                 (pr.getIndMI(),
                                  &eCtx,
                                  NULL,
                                  eSelx,
                                  CHARS(className),
                                  &eRef,
                                  i == 0);
                }

                if (rc.rc == CMPI_RC_OK)
                {
                    filterActivated = true;
                    eSelx->classNames.append(classPath);
                    eSelx->classNamesSCMO.append(*classPathSCMO);
                }
                else
                {
                    PEG_TRACE((
                        TRC_PROVIDERMANAGER,
                        Tracer::LEVEL2,
                        "activateFilter() for class %s in namespace %s "
                            "failed. Error : %s",
                        CHARS(classPath.getClassName().
                            getString().getCString()),
                        CHARS(request->nameSpace.getString().getCString()),
                        rc.msg ? CMGetCharsPtr(rc.msg, NULL) : "Unknown"));
                }
            }
        }

        PEG_TRACE((
            TRC_PROVIDERMANAGER,
            Tracer::LEVEL2,
            "Returning from provider.createSubscriptionRequest: %s",
            (const char*)pr.getName().getCString()));

//      Need to save ContentLanguage value into operation context of response
//      Do this before checking rc from provider to throw exception in case
//      rc.msg is also localized.
        CMPIStatus tmprc={CMPI_RC_OK,NULL};
        CMPIData cldata =
            eCtx.ft->getEntry (&eCtx, CMPIContentLanguage, &tmprc);
        if (tmprc.rc == CMPI_RC_OK)
        {
            response->operationContext.set(
                ContentLanguageListContainer(
                ContentLanguageList(
                LanguageParser::parseContentLanguageHeader(
                CMGetCharsPtr(cldata.value.string, NULL)))));
        }

        if (!filterActivated)
        {
            //  Remove the select expression from the cache
            WriteLock lock(rwSemProvTab);
            PEGASUS_FCT_EXECUTE_AND_ASSERT(
                true,
                indProvRec->deleteSelectExp(sPath, request->nameSpace));
            
            delete eSelx;
            throw CIMException((CIMStatusCode)rc.rc,
                rc.msg ? CMGetCharsPtr(rc.msg, NULL) : String::EMPTY);
        }
        else
        {
            //
            //  Increment count of current subscriptions for this provider
            //
            if (ph.GetProvider ().testIfZeroAndIncrementSubscriptions ())
            {
                //
                //  If there were no current subscriptions before the increment,
                //  the first subscription has been created
                //  Call the provider's enableIndications method
                //
                if (_subscriptionInitComplete)
                {
                    _callEnableIndications (req_provider, _indicationCallback,
                        ph, (const char*)pidc.getRemoteInfo().getCString());
                }
            }
        }
    }
    HandlerCatch(handler);

    PEG_METHOD_EXIT();

    return(response);
}

Message * CMPIProviderManager::handleDeleteSubscriptionRequest(
    const Message * message)
{
    PEG_METHOD_ENTER(
        TRC_PROVIDERMANAGER,
        "CMPIProviderManager::handleDeleteSubscriptionRequest()");

    HandlerIntroInd(DeleteSubscription,message,request,response,
        handler);
    try
    {
        String providerName,providerLocation;

        CIMInstance req_provider, req_providerModule;
        ProviderIdContainer pidc =
            (ProviderIdContainer)request->operationContext.get(
            ProviderIdContainer::NAME);
        req_provider = pidc.getProvider();
        req_providerModule = pidc.getModule();

        LocateIndicationProviderNames(req_provider, req_providerModule,
            providerName,providerLocation);

        PEG_TRACE((TRC_PROVIDERMANAGER,Tracer::LEVEL3,
            "CMPIProviderManager::handleDeleteSubscriptionRequest"
            " - Host name: %s  Name space: %s  Provider name(s): %s",
            (const char*) System::getHostName().getCString(),
            (const char*) request->nameSpace.getString().getCString(),
            (const char*) providerName.getCString()));

        Boolean remote=false;
        OpProviderHolder ph;
        CString remoteInfo;

        CMPIProvider & pr = _resolveAndGetProvider(
            &(request->operationContext),
            &ph,
            &remoteInfo,
            remote);

        const CIMObjectPath &sPath=request->subscriptionInstance.getPath();
        IndProvRecord *indProvRec=NULL;
        CMPI_SelectExp *eSelx=NULL;
        {
            WriteLock lock(rwSemProvTab);
            indProvTab.lookup(pr.getName(),indProvRec);
            if (!indProvRec->lookupSelectExp(sPath, request->nameSpace, eSelx))
            {
                MessageLoaderParms parms(
                    "ProviderManager.CMPI.CMPIProviderManager."
                        "FAILED_LOCATE_SUBSCRIPTION_FILTER",
                    "Failed to locate the subscription filter.");
                // failed to get select expression from hash table
                throw CIMException(CIM_ERR_FAILED, parms);
            }
            PEGASUS_FCT_EXECUTE_AND_ASSERT(
                true,
                indProvRec->deleteSelectExp(sPath, request->nameSpace));
        }

        CString className = eSelx->classNames[0].getClassName().
            getString().getCString();

        CIMObjectPath subscriptionName =
            request->subscriptionInstance.getPath();

        CMPIStatus rc={CMPI_RC_OK,NULL};
        CMPI_ContextOnStack eCtx(request->operationContext);
        CMPI_ThreadContext thr(pr.getBroker(),&eCtx);

        CString nameSpace = request->nameSpace.getString().getCString();

        // includeQualifiers and includeClassOrigin not of interest for
        // this type of request
        _setupCMPIContexts(
            &eCtx,
            &(request->operationContext),
            &nameSpace,
            &remoteInfo,
            remote);

        CMPIProvider::pm_service_op_lock op_lock(&pr);

        PEG_TRACE((
            TRC_PROVIDERMANAGER,
            Tracer::LEVEL2,
            "Calling provider.deleteSubscriptionRequest: %s",
            (const char*)pr.getName().getCString()));

        {
            AutoPThreadSecurity threadLevelSecurity(request->operationContext);

            StatProviderTimeMeasurement providerTime(response);

            // Call deactivateFilter() for each subclass name those were
            // activated previously using activateFilter().
            // Note: Start from Index 1, first name is actual class name in
            // the FROM clause of filter query.
            for (Uint32 i = 1, n = eSelx->classNames.size(); i < n ; ++i)
            {
                CMPI_ObjectPathOnStack eRef(eSelx->classNamesSCMO[i]);
                if (pr.getIndMI()->ft->ftVersion >= 100)
                {
                    rc = pr.getIndMI()->ft->deActivateFilter(
                        pr.getIndMI(),
                        &eCtx,
                        eSelx,
                        CHARS(className),
                        &eRef,
                        i == n - 1);
                }
                else
                {
                    // Older version of (pre 1.00) also pass in a CMPIResult

                    rc = ((CMPIStatus (*)(
                        CMPIIndicationMI*,
                        CMPIContext*,
                        CMPIResult*,
                        CMPISelectExp*,
                        const char *,
                        CMPIObjectPath*,
                        CMPIBoolean)) pr.getIndMI()->ft->deActivateFilter)(
                            pr.getIndMI(),
                            &eCtx,
                            NULL,
                            eSelx,
                            CHARS(className),
                            &eRef,
                            i == n - 1);
                }
                if (rc.rc != CMPI_RC_OK)
                {
                    PEG_TRACE((
                        TRC_PROVIDERMANAGER,
                        Tracer::LEVEL2,
                        "deactivateFilter() for class %s in namespace %s"
                            "failed. Error : %s",
                        CHARS(eSelx->classNames[i].getClassName().
                            getString().getCString()),
                        CHARS(request->nameSpace.getString().getCString()),
                        rc.msg ? CMGetCharsPtr(rc.msg, NULL) : "Unknown"));
                }
            }
        }

        PEG_TRACE((
            TRC_PROVIDERMANAGER,
            Tracer::LEVEL2,
            "Returning from provider.deleteSubscriptionRequest: %s",
            (const char*)pr.getName().getCString()));

        delete eSelx;

//      Need to save ContentLanguage value into operation context of response
//      Do this before checking rc from provider to throw exception in case
//      rc.msg is also localized.
        CMPIStatus tmprc={CMPI_RC_OK,NULL};
        CMPIData cldata =
            eCtx.ft->getEntry (&eCtx, CMPIContentLanguage, &tmprc);
        if (tmprc.rc == CMPI_RC_OK)
        {
            response->operationContext.set(
                ContentLanguageListContainer(
                ContentLanguageList(
                LanguageParser::parseContentLanguageHeader(
                CMGetCharsPtr(cldata.value.string, NULL)))));
        }

        if (rc.rc!=CMPI_RC_OK)
        {
            throw CIMException((CIMStatusCode)rc.rc,
                rc.msg ? CMGetCharsPtr(rc.msg, NULL) : String::EMPTY);
        }
        else
        {
            //
            //  Decrement count of current subscriptions for this provider
            //
            if (pr.decrementSubscriptionsAndTestIfZero ())
            {
                Boolean callDisable = false;
                {
                    WriteLock lock(rwSemProvTab);
                    if (!indProvRec->getSelectExpCount())
                    {
                        indProvTab.remove(pr.getName());
                        delete indProvRec;
                        callDisable = true;
                    }
                }

                //
                //  If there are no current subscriptions after the decrement,
                //  the last subscription has been deleted
                //  Call the provider's disableIndications method
                //
                if (_subscriptionInitComplete && callDisable)
                {
                    _callDisableIndications(
                        ph,
                        (const char*)pidc.getRemoteInfo().getCString());
                }
            }
        }
    }
    HandlerCatch(handler);

    PEG_METHOD_EXIT();

    return(response);
}

Message * CMPIProviderManager::handleDisableModuleRequest(
    const Message * message)
{
    PEG_METHOD_ENTER(
        TRC_PROVIDERMANAGER,
        "CMPIProviderManager::handleDisableModuleRequest()");

    CIMDisableModuleRequestMessage * request =
        dynamic_cast<CIMDisableModuleRequestMessage *>(
        const_cast<Message *>(message));

    PEGASUS_ASSERT(request != 0);

    //Set to false when provider refused to unload due to pending operations.
    Boolean disableModuleOk = true;

    //
    // Unload providers
    //
    Array<CIMInstance> _pInstances = request->providers;
    Array <Boolean> _indicationProviders = request->indicationProviders;
    /* The CIMInstances on request->providers array is completly _different_
       than the request->providerModule CIMInstance. Hence  */

    String physicalName=(request->providerModule.getProperty(
        request->
        providerModule.findProperty("Location")).getValue().toString());

    String moduleName= request->providerModule.getProperty(
        request->providerModule.findProperty("Name")).getValue().toString();

    for (Uint32 i = 0, n = _pInstances.size(); i < n; i++)
    {
        String providerName;

        Uint32 pos = _pInstances[i].findProperty(PEGASUS_PROPERTYNAME_NAME);

        _pInstances[i].getProperty(pos).getValue().get(providerName);


        if (!providerManager.isProviderActive(providerName, moduleName))
        {
            continue;
        }

        Boolean unloadOk = providerManager.unloadProvider(
            physicalName,
            _pInstances[i].getProperty(pos).getValue().toString(),
            moduleName);

        if (!unloadOk)
        {
            disableModuleOk = false;
            continue;
        }
        //
        //  Reset the indication provider's count of current
        //  subscriptions since it has been disabled
        //
        if (_indicationProviders [i])
        {
            // Remove from IndProvRecord table
            IndProvRecord *rec = 0;
            WriteLock lock(rwSemProvTab);
            if (indProvTab.lookup(providerName, rec))
            {
                delete rec;
                indProvTab.remove(providerName);
            }
        }
    }

    CIMDisableModuleResponseMessage* response =
        dynamic_cast<CIMDisableModuleResponseMessage*>(
            request->buildResponse());
    PEGASUS_ASSERT(response != 0);

    if (disableModuleOk)
    {
        response->operationalStatus.append(
            CIM_MSE_OPSTATUS_VALUE_STOPPED);
    }
    else
    {
        response->operationalStatus.append(
            CIM_MSE_OPSTATUS_VALUE_OK);
    }

    PEG_METHOD_EXIT();

    return(response);
}

Message * CMPIProviderManager::handleEnableModuleRequest(
    const Message * message)
{
    PEG_METHOD_ENTER(
        TRC_PROVIDERMANAGER,
        "CMPIProviderManager::handleEnableModuleRequest()");

    CIMEnableModuleRequestMessage * request =
        dynamic_cast<CIMEnableModuleRequestMessage *>(
        const_cast<Message *>(message));

    PEGASUS_ASSERT(request != 0);

    Array<Uint16> operationalStatus;
    operationalStatus.append(CIM_MSE_OPSTATUS_VALUE_OK);

    CIMEnableModuleResponseMessage* response =
        dynamic_cast<CIMEnableModuleResponseMessage*>(
            request->buildResponse());
    PEGASUS_ASSERT(response != 0);
    response->operationalStatus = operationalStatus;

    PEG_METHOD_EXIT();

    return(response);
}

Message * CMPIProviderManager::handleStopAllProvidersRequest(
    const Message * message)
{
    PEG_METHOD_ENTER(
        TRC_PROVIDERMANAGER,
        "CMPIProviderManager::handleStopAllProvidersRequest()");

    CIMStopAllProvidersRequestMessage * request =
        dynamic_cast<CIMStopAllProvidersRequestMessage *>(
        const_cast<Message *>(message));

    PEGASUS_ASSERT(request != 0);

    CIMStopAllProvidersResponseMessage* response =
        dynamic_cast<CIMStopAllProvidersResponseMessage*>(
            request->buildResponse());
    PEGASUS_ASSERT(response != 0);

    // tell the provider manager to shutdown all the providers
    providerManager.shutdownAllProviders();

    PEG_METHOD_EXIT();

    return(response);
}

Message* CMPIProviderManager::handleIndicationServiceDisabledRequest(
    Message* message)
{
    PEG_METHOD_ENTER(TRC_PROVIDERMANAGER,
        "CMPIProviderManager::_handleIndicationServiceDisabledRequest");

    CIMIndicationServiceDisabledRequestMessage* request =
        dynamic_cast<CIMIndicationServiceDisabledRequestMessage*>(message);
    PEGASUS_ASSERT(request != 0);

    CIMIndicationServiceDisabledResponseMessage* response =
        dynamic_cast<CIMIndicationServiceDisabledResponseMessage*>(
            request->buildResponse());
    PEGASUS_ASSERT(response != 0);

    _subscriptionInitComplete = false;

    PEG_METHOD_EXIT ();
    return response;
}

Message * CMPIProviderManager::handleSubscriptionInitCompleteRequest(
    const Message * message)
{
    PEG_METHOD_ENTER(
        TRC_PROVIDERMANAGER,
        "CMPIProviderManager::handleSubscriptionInitCompleteRequest()");

    CIMSubscriptionInitCompleteRequestMessage * request =
        dynamic_cast <CIMSubscriptionInitCompleteRequestMessage *>
        (const_cast <Message *> (message));

    PEGASUS_ASSERT (request != 0);

    CIMSubscriptionInitCompleteResponseMessage * response =
        dynamic_cast <CIMSubscriptionInitCompleteResponseMessage *>
        (request->buildResponse ());

    PEGASUS_ASSERT (response != 0);

    //
    //  Set indicator
    //
    _subscriptionInitComplete = true;

    //
    //  For each provider that has at least one subscription, call
    //  provider's enableIndications method
    //
    Array <CMPIProvider *> enableProviders;
    enableProviders = providerManager.getIndicationProvidersToEnable ();

    Uint32 numProviders = enableProviders.size ();
    for (Uint32 i = 0; i < numProviders; i++)
    {
        try
        {
            CIMInstance provider;
            provider = enableProviders [i]->getProviderInstance ();

            CString info;
#ifdef PEGASUS_ENABLE_REMOTE_CMPI
            IndProvRecord *provRec = 0;
            if (indProvTab.lookup (enableProviders [i]->getName(), provRec))
            {
                if (provRec->getRemoteInfo() != String::EMPTY)
                {
                    info = provRec->getRemoteInfo().getCString();
                }
            }
#endif
            //
            //  Get cached or load new provider module
            //
            OpProviderHolder ph;
            if ((const char*)info)
            {
                ph = providerManager.getRemoteProvider
                    (enableProviders [i]->getModule ()->getFileName (),
                    enableProviders [i]->getName (),
                    enableProviders[i]->getModuleName());
            }
            else
            {
                ph = providerManager.getProvider
                    (enableProviders [i]->getModule ()->getFileName (),
                    enableProviders [i]->getName (),
                    enableProviders[i]->getModuleName());
            }
            _callEnableIndications(
                provider,
                _indicationCallback,
                ph,
                (const char*)info);
        }
        catch (const CIMException & e)
        {
            PEG_TRACE((TRC_PROVIDERMANAGER,Tracer::LEVEL1,
                "CIMException: %s",(const char*)e.getMessage().getCString()));
        }
        catch (const Exception & e)
        {
            PEG_TRACE((TRC_PROVIDERMANAGER,Tracer::LEVEL1,
                "Exception: %s",(const char*)e.getMessage().getCString()));
        }
        catch (...)
        {
            PEG_TRACE_CSTRING(
                TRC_PROVIDERMANAGER,
                Tracer::LEVEL1,
                "Unknown error in handleSubscriptionInitCompleteRequest");
        }
    }

    PEG_METHOD_EXIT ();
    return(response);
}

Message * CMPIProviderManager::handleGetPropertyRequest(
    const Message * message)
{
    PEG_METHOD_ENTER(
        TRC_PROVIDERMANAGER,
        "CMPIProviderManager::handleGetPropertyRequest()");

    HandlerIntro(GetProperty,message,request,response,handler);

    // We're only going to be interested in the specific property from this
    // instance.
    Array<CIMName> localPropertyListArray;
    localPropertyListArray.append(request->propertyName);
    CIMPropertyList localPropertyList(localPropertyListArray);

    // NOTE: GetProperty will use the CIMInstanceProvider interface, so we
    // must manually define a request, response, and handler (emulate
    // HandlerIntro macro)
    CIMGetInstanceRequestMessage * GI_request =
        new CIMGetInstanceRequestMessage(
        request->messageId,
        request->nameSpace,
        request->instanceName,
        false,
        false,
        localPropertyList,
        request->queueIds,
        request->authType,
        request->userName
        );

    PEGASUS_ASSERT(GI_request != 0);

    CIMGetInstanceResponseMessage * GI_response =
        dynamic_cast<CIMGetInstanceResponseMessage*>
        (GI_request->buildResponse());

    PEGASUS_ASSERT(GI_response != 0);

    GetInstanceResponseHandler GI_handler(
        GI_request,
        GI_response,
        _responseChunkCallback);

    try
    {
        PEG_TRACE((TRC_PROVIDERMANAGER,Tracer::LEVEL3,
            "CMPIProviderManager::handleGetPropertyRequest"
            " - Host name: %s  Name space: %s  "
                "Class name: %s  Property name: %s",
            (const char*) System::getHostName().getCString(),
            (const char*) request->nameSpace.getString().getCString(),
            (const char*)
                request->instanceName.getClassName().getString().getCString(),
            (const char*) request->propertyName.getString().getCString()));

        Boolean remote=false;
        OpProviderHolder ph;
        CString remoteInfo;

        CMPIProvider & pr = _resolveAndGetProvider(
            &(request->operationContext),
            &ph,
            &remoteInfo,
            remote);

        AutoPtr<NormalizerContext> tmpNormalizerContext(
            new CIMOMHandleContext(*pr.getCIMOMHandle()));
        request->operationContext.insert(
            NormalizerContextContainer(tmpNormalizerContext));

        CMPIStatus rc={CMPI_RC_OK,NULL};
        CMPI_ContextOnStack eCtx(request->operationContext);
        CMPI_ResultOnStack eRes(GI_handler,pr.getBroker());
        CMPI_ThreadContext thr(pr.getBroker(),&eCtx);

        // For the getInstance provider call, use the property list that we
        // created containing the single property from the getProperty call.
        CMPIPropertyList props(localPropertyList);

        CString nameSpace = request->nameSpace.getString().getCString();
        CString className =
            request->instanceName.getClassName().getString().getCString();

        // Leave includeQualifiers and includeClassOrigin as false for this
        // call to getInstance
        _setupCMPIContexts(
            &eCtx,
            &(request->operationContext),
            &nameSpace,
            &remoteInfo,
            remote,
            false,
            false,
            true);

        SCMOInstance * objectPath = getSCMOObjectPathFromRequest(
            nameSpace, className, request->instanceName);
        CMPI_ObjectPathOnStack eRef(objectPath);

        CMPIProvider::pm_service_op_lock op_lock(&pr);

        PEG_TRACE((
            TRC_PROVIDERMANAGER,
            Tracer::LEVEL2,
            "Calling provider.getInstance via getProperty: %s",
            (const char*)pr.getName().getCString()));

        {
            AutoPThreadSecurity threadLevelSecurity(request->operationContext);

            StatProviderTimeMeasurement providerTime(response);

            rc = pr.getInstMI()->ft->getInstance(
                pr.getInstMI(),
                &eCtx,
                &eRes,
                &eRef,
                (const char **)props.getList());
        }

        PEG_TRACE((
            TRC_PROVIDERMANAGER,
            Tracer::LEVEL2,
            "Returning from provider.getInstance via getProperty: %s",
            (const char*)pr.getName().getCString()));

//      Need to save ContentLanguage value into operation context of response
//      Do this before checking rc from provider to throw exception in case
//      rc.msg is also localized.
        CMPIStatus tmprc={CMPI_RC_OK,NULL};
        CMPIData cldata =
            eCtx.ft->getEntry (&eCtx, CMPIContentLanguage, &tmprc);
        if (tmprc.rc == CMPI_RC_OK)
        {
            response->operationContext.set(
                ContentLanguageListContainer(
                ContentLanguageList(
                LanguageParser::parseContentLanguageHeader(
                CMGetCharsPtr(cldata.value.string, NULL)))));
            handler.setContext(response->operationContext);
        }
        _throwCIMException(rc, eRes.resError);

        // Copy property value from instance to getProperty response
        Array<SCMOInstance>& arInstance =
            GI_response->getResponseData().getSCMO();
        if (arInstance.size() > 0)
        {
            SCMOInstance& instance = arInstance[0];
            if(!(instance.isUninitialized()))
            {
                CString pName =
                    request->propertyName.getString().getCString();

                // Construct a temporary CMPI Instance object, on which we
                // can use the encapsulation functions to retrieve the property.
                CMPI_InstanceOnStack tmpInst(instance);

                CMPIStatus trc;
                CMPIData data =
                    CMGetProperty(&tmpInst, (const char*)pName, &trc);

                if (trc.rc == CMPI_RC_OK)
                {
                    // Convert the CMPIData to a CIMValue
                    CIMValue val =
                        value2CIMValue(&(data.value), data.type, &(trc.rc));

                    response->value = val;
                }
                // Else property not found. Return CIM_ERR_NO_SUCH_PROPERTY.
                else
                {
                    throw PEGASUS_CIM_EXCEPTION(
                        CIM_ERR_NO_SUCH_PROPERTY,
                        request->propertyName.getString());
                }
            }
        }
    }
    HandlerCatch(handler);

    delete GI_request;
    delete GI_response;

    PEG_METHOD_EXIT();

    return(response);
}

Message * CMPIProviderManager::handleSetPropertyRequest(
    const Message * message)
{
    PEG_METHOD_ENTER(
        TRC_PROVIDERMANAGER,
        "CMPIProviderManager::handleSetPropertyRequest()");

    HandlerIntro(SetProperty,message,request,response,handler);

    // We're only going to be interested in the specific property from this
    // instance.
    Array<CIMName> localPropertyListArray;
    localPropertyListArray.append(request->propertyName);
    CIMPropertyList localPropertyList(localPropertyListArray);

    // Build a modified instance with just the specific property and its
    // new value.
    CIMInstance localModifiedInstance(request->instanceName.getClassName());
    localModifiedInstance.setPath(request->instanceName);
    localModifiedInstance.addProperty(
        CIMProperty(request->propertyName, request->newValue));

    // NOTE: SetProperty will use the CIMInstanceProvider interface, so we must
    // manually define a request, response, and handler.
    CIMModifyInstanceRequestMessage * MI_request =
        new CIMModifyInstanceRequestMessage(
        request->messageId,
        request->nameSpace,
        localModifiedInstance,
        false,
        localPropertyList,
        request->queueIds,
        request->authType,
        request->userName
        );

    PEGASUS_ASSERT(MI_request != 0);

    CIMModifyInstanceResponseMessage * MI_response =
        dynamic_cast<CIMModifyInstanceResponseMessage*>(
        MI_request->buildResponse());

    PEGASUS_ASSERT(MI_response != 0);

    ModifyInstanceResponseHandler MI_handler(
        MI_request,
        MI_response,
        _responseChunkCallback);

    try
    {
        CString nameSpace = request->nameSpace.getString().getCString();
        CString className =
            request->instanceName.getClassName().getString().getCString();

        PEG_TRACE((TRC_PROVIDERMANAGER,Tracer::LEVEL3,
            "CMPIProviderManager::handleSetPropertyRequest"
            " - Host name: %s  Name space: %s  "
                "Class name: %s  Property name: %s",
            (const char*) System::getHostName().getCString(),
            (const char*) nameSpace,
            (const char*) className,
            (const char*) request->propertyName.getString().getCString()));

        Boolean remote=false;
        OpProviderHolder ph;
        CString remoteInfo;

        CMPIProvider & pr = _resolveAndGetProvider(
            &(request->operationContext),
            &ph,
            &remoteInfo,
            remote);

        CMPIStatus rc={CMPI_RC_OK,NULL};
        CMPI_ContextOnStack eCtx(request->operationContext);
        CMPI_ResultOnStack eRes(MI_handler,pr.getBroker());
        CMPI_ThreadContext thr(pr.getBroker(),&eCtx);

        CMPIPropertyList props(localPropertyList);

        // Leave includeQualifiers as false for this call to modifyInstance
        _setupCMPIContexts(
            &eCtx,
            &(request->operationContext),
            &nameSpace,
            &remoteInfo,
            remote,
            false,
            false,
            true);


        SCMOInstance * modInst = getSCMOInstanceFromRequest(
            nameSpace, className, localModifiedInstance);
        CMPI_InstanceOnStack eInst(modInst);

        // This will create a second reference for the same SCMOInstance
        CMPI_ObjectPathOnStack eRef(*modInst);

        CMPIProvider::pm_service_op_lock op_lock(&pr);

        PEG_TRACE((
            TRC_PROVIDERMANAGER,
            Tracer::LEVEL2,
            "Calling provider.modifyInstance via setProperty: %s",
            (const char*)pr.getName().getCString()));

        {
            AutoPThreadSecurity threadLevelSecurity(request->operationContext);

            StatProviderTimeMeasurement providerTime(response);

            rc = pr.getInstMI()->ft->modifyInstance(
                pr.getInstMI(),
                &eCtx,
                &eRes,
                &eRef,
                &eInst,
                (const char **)props.getList());
        }

        PEG_TRACE((
            TRC_PROVIDERMANAGER,
            Tracer::LEVEL2,
            "Returning from provider.modifyInstance via setProperty: %s",
            (const char*)pr.getName().getCString()));

//      Need to save ContentLanguage value into operation context of response
//      Do this before checking rc from provider to throw exception in case
//      rc.msg is also localized.
        CMPIStatus tmprc={CMPI_RC_OK,NULL};
        CMPIData cldata =
            eCtx.ft->getEntry (&eCtx, CMPIContentLanguage, &tmprc);
        if (tmprc.rc == CMPI_RC_OK)
        {
            response->operationContext.set(
                ContentLanguageListContainer(
                ContentLanguageList(
                LanguageParser::parseContentLanguageHeader(
                CMGetCharsPtr(cldata.value.string, NULL)))));
            handler.setContext(response->operationContext);
        }
        _throwCIMException(rc, eRes.resError);
    }
    HandlerCatch(handler);

    delete MI_request;
    delete MI_response;

    PEG_METHOD_EXIT();

    return(response);
}

Message * CMPIProviderManager::handleUnsupportedRequest(
    const Message * message)
{
    PEG_METHOD_ENTER(
        TRC_PROVIDERMANAGER,
        "CMPIProviderManager::handleUnsupportedRequest()");
    CIMRequestMessage* request =
        dynamic_cast<CIMRequestMessage *>(const_cast<Message *>(message));
    PEGASUS_ASSERT(request != 0 );

    CIMResponseMessage* response = request->buildResponse();
    response->cimException =
        PEGASUS_CIM_EXCEPTION(CIM_ERR_NOT_SUPPORTED, String::EMPTY);

    PEG_METHOD_EXIT();
    return response;
}

ProviderName CMPIProviderManager::_resolveProviderName(
    const ProviderIdContainer & providerId)
{
    String providerName;
    String fileName;
    String location;
    String moduleName;
    CIMValue genericValue;

    PEG_METHOD_ENTER(
        TRC_PROVIDERMANAGER,
        "CMPIProviderManager::_resolveProviderName()");

    genericValue = providerId.getModule().getProperty(
        providerId.getModule().findProperty(
            PEGASUS_PROPERTYNAME_NAME)).getValue();
    genericValue.get(moduleName);

    genericValue = providerId.getProvider().getProperty(
        providerId.getProvider().findProperty(
            PEGASUS_PROPERTYNAME_NAME)).getValue();
    genericValue.get(providerName);

    genericValue = providerId.getModule().getProperty(
        providerId.getModule().findProperty("Location")).getValue();
    genericValue.get(location);
    fileName = _resolvePhysicalName(location);

    // An empty file name is only for interest if we are in the
    // local name space. So the message is only issued if not
    // in the remote Name Space.
    if (fileName == String::EMPTY && (!providerId.isRemoteNameSpace()))
    {
        genericValue.get(location);
        String fullName = FileSystem::buildLibraryFileName(location);
        Logger::put_l(Logger::ERROR_LOG, System::CIMSERVER, Logger::SEVERE,
            MessageLoaderParms(
                "ProviderManager.CMPI.CMPIProviderManager.CANNOT_FIND_LIBRARY",
                "For provider $0 library $1 was not found.",
                providerName, fullName));

    }
    ProviderName name(moduleName, providerName, fileName);
    name.setLocation(location);
    PEG_METHOD_EXIT();
    return name;
}

void CMPIProviderManager::_callEnableIndications
    (CIMInstance & req_provider,
    PEGASUS_INDICATION_CALLBACK_T _indicationCallback,
    OpProviderHolder & ph,
    const char* remoteInfo)
{
    PEG_METHOD_ENTER(
        TRC_PROVIDERMANAGER,
        "CMPIProviderManager::_callEnableIndications()");

    try
    {
        IndProvRecord *indProvRec =0;
        {
            WriteLock lock(rwSemProvTab);

            if (indProvTab.lookup (ph.GetProvider ().getName (),indProvRec))
            {
                CIMRequestMessage * request = 0;
                CIMResponseMessage * response = 0;
                indProvRec->setHandler(new EnableIndicationsResponseHandler(
                    request,
                    response,
                    req_provider,
                    _indicationCallback,
                    _responseChunkCallback));
            }
        }

        CMPIProvider & pr=ph.GetProvider();

        //
        //  Versions prior to 86 did not include enableIndications routine
        //
        if (pr.getIndMI()->ft->ftVersion >= 86)
        {
            OperationContext context;
#ifdef PEGASUS_ZOS_THREADLEVEL_SECURITY
            // For the z/OS security model we always need an Identity container
            // in the operation context. Since we don't have a client request
            // ID here we have to use the cim servers identity for the time
            // being.
            IdentityContainer idContainer(System::getEffectiveUserName());
            context.insert(idContainer);
#endif

            CMPI_ContextOnStack eCtx(context);
            CMPI_ThreadContext thr(pr.getBroker(),&eCtx);

            // Add RemoteInformation -V 5245
            if (remoteInfo)
            {
                eCtx.ft->addEntry(&eCtx,"CMPIRRemoteInfo",
                    (CMPIValue*)(const char*)remoteInfo,CMPI_chars);
            }

            PEG_TRACE((
                TRC_PROVIDERMANAGER,
                Tracer::LEVEL2,
                "Calling provider.enableIndications: %s",
                (const char*)pr.getName().getCString()));

            pr.protect();

            // enableIndications() is defined by the CMPI standard as
            // returning a CMPIStatus return value. Unfortunately, Pegasus
            // originally implemented enableIndications() with a void
            // return type, and this incompatibility was not detected for
            // some time. Since exceptions thrown from enableIndications()
            // are not reported (other than via logging), it was decided to
            // discard the returned CMPIStatus here. This will prevent us from
            // breaking existing CMPI Indication providers. This is ok since
            // there really isn't a user to which the problem should be
            // reported.
            pr.getIndMI()->ft->enableIndications(pr.getIndMI(),&eCtx);

            PEG_TRACE((
                TRC_PROVIDERMANAGER,
                Tracer::LEVEL2,
                "Returning from provider.enableIndications: %s",
                (const char*)pr.getName().getCString()));

        }
        else
        {
            PEG_TRACE((
                TRC_PROVIDERMANAGER,
                Tracer::LEVEL2,
                "Not calling provider.enableIndications: %s routine as it is "
                "an earlier version that does not support this function",
                 (const char*)pr.getName().getCString()));
        }
    }
    catch (const Exception & e)
    {
        Logger::put_l(Logger::ERROR_LOG, System::CIMSERVER, Logger::WARNING,
            MessageLoaderParms(
                "ProviderManager.CMPI.CMPIProviderManager."
                    "ENABLE_INDICATIONS_FAILED",
                "Failed to enable indications for provider $0: $1.",
                ph.GetProvider().getName(), e.getMessage()));
    }
    catch (...)
    {
        Logger::put_l(Logger::ERROR_LOG, System::CIMSERVER, Logger::WARNING,
            MessageLoaderParms(
                "ProviderManager.CMPI.CMPIProviderManager."
                    "ENABLE_INDICATIONS_FAILED_UNKNOWN",
                "Failed to enable indications for provider $0.",
                ph.GetProvider().getName()));
    }

    PEG_METHOD_EXIT ();
}

void CMPIProviderManager::_callDisableIndications
    (OpProviderHolder & ph, const char *remoteInfo)
{
    PEG_METHOD_ENTER(
        TRC_PROVIDERMANAGER,
        "CMPIProviderManager::_callDisableIndications()");

    try
    {
        CMPIProvider & pr=ph.GetProvider();

        //
        //  Versions prior to 86 did not include disableIndications routine
        //
        if (pr.getIndMI()->ft->ftVersion >= 86)
        {
            OperationContext context;
            CMPI_ContextOnStack eCtx(context);

            if (remoteInfo)
            {
                eCtx.ft->addEntry(&eCtx,"CMPIRRemoteInfo",
                    (CMPIValue*)(const char*)remoteInfo,CMPI_chars);
            }
            CMPI_ThreadContext thr(pr.getBroker(),&eCtx);

            PEG_TRACE((TRC_PROVIDERMANAGER,Tracer::LEVEL2,
                "Calling provider.disableIndications: %s",
                (const char*)pr.getName().getCString()));

            // disableIndications() is defined by the CMPI standard as
            // returning a CMPIStatus return value. Unfortunately, Pegasus
            // originally implemented disableIndications() with a void
            // return type, and this incompatibility was not detected for
            // some time. For consistency with the enableIndications()
            // interface, it was decided to discard the returned CMPIStatus
            // here. This will prevent us from breaking existing CMPI
            // Indication providers. This is ok since there really isn't a
            // user to which the problem should be reported.
            pr.getIndMI()->ft->disableIndications(
                pr.getIndMI(),
                &eCtx);

            pr.unprotect();

            PEG_TRACE((TRC_PROVIDERMANAGER,Tracer::LEVEL2,
                "Returning from provider.disableIndications: %s",
                (const char*)pr.getName().getCString()));

        }
        else
        {
            PEG_TRACE((TRC_PROVIDERMANAGER,Tracer::LEVEL2,
                "Not calling provider.disableIndications: %s routine as it is "
                "an earlier version that does not support this function",
                (const char*)pr.getName().getCString()));
        }
    }
    catch (const Exception & e)
    {
        Logger::put_l(Logger::ERROR_LOG, System::CIMSERVER, Logger::WARNING,
            MessageLoaderParms(
                "ProviderManager.CMPI.CMPIProviderManager."
                    "DISABLE_INDICATIONS_FAILED",
                "Failed to disable indications for provider $0: $1.",
                ph.GetProvider().getName(), e.getMessage()));
    }
    catch (...)
    {
        Logger::put_l(Logger::ERROR_LOG, System::CIMSERVER, Logger::WARNING,
            MessageLoaderParms(
                "ProviderManager.CMPI.CMPIProviderManager."
                    "DISABLE_INDICATIONS_FAILED_UNKNOWN",
                "Failed to disable indications for provider $0.",
                ph.GetProvider().getName()));
    }

    PEG_METHOD_EXIT ();
}

PEGASUS_NAMESPACE_END

