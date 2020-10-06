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

#include "CMPI_Broker.h"
#include "CMPI_Object.h"
#include "CMPI_ContextArgs.h"
#include "CMPI_Enumeration.h"
#include "CMPI_Value.h"
#include "CMPIProviderManager.h"
#include "CMPI_String.h"
#include <Pegasus/ProviderManager2/CMPI/CMPIClassCache.h>
#include <Pegasus/ProviderManager2/CMPI/CMPI_ThreadContext.h>

#include <Pegasus/Common/CIMName.h>
#include <Pegasus/Common/CIMPropertyList.h>
#include <Pegasus/Provider/CIMOMHandle.h>
#include <Pegasus/Common/CIMValue.h>
#include <Pegasus/Common/CIMType.h>
#include "CMPISCMOUtilities.h"


PEGASUS_USING_STD;
PEGASUS_NAMESPACE_BEGIN

static const CMPIUint32 MB_CAPABILITIES =
#   ifdef CMPI_VER_200
    CMPI_MB_Supports_Extended_Error |
#   endif
    CMPI_MB_BasicRead | CMPI_MB_BasicWrite | CMPI_MB_InstanceManipulation |
    CMPI_MB_AssociationTraversal | CMPI_MB_QueryNormalization |
    CMPI_MB_Indications | CMPI_MB_BasicQualifierSupport |
    CMPI_MB_OSEncapsulationSupport
#   ifndef PEGASUS_DISABLE_EXECQUERY
    | CMPI_MB_QueryExecution
#   endif
    ;

#define HandlerCatchSetStatus(rc, returnvalue) \
    catch (const CIMException &e) \
    { \
        PEG_TRACE(( \
            TRC_CMPIPROVIDERINTERFACE, \
            Tracer::LEVEL1, \
            "CIMException: %s",(const char*)e.getMessage().getCString())); \
        CMSetStatusWithString( \
            rc, \
            (CMPIrc)e.getCode(), \
            (CMPIString*)string2CMPIString(e.getMessage())); \
        PEG_METHOD_EXIT(); \
        return returnvalue; \
    } \
    catch (const Exception &e) \
    { \
        PEG_TRACE(( \
            TRC_CMPIPROVIDERINTERFACE, \
            Tracer::LEVEL2, \
            "Exception: %s", (const char *)e.getMessage().getCString())); \
        CMSetStatusWithString( \
            rc, \
            (CMPIrc)CMPI_RC_ERROR_SYSTEM, \
            (CMPIString*)string2CMPIString(e.getMessage())); \
        PEG_METHOD_EXIT(); \
        return returnvalue; \
    } \
    catch (...) \
    { \
        PEG_TRACE(( \
            TRC_CMPIPROVIDERINTERFACE, \
            Tracer::LEVEL2, \
            "Unknown exception")); \
        CMSetStatusWithString( \
            rc, \
            (CMPIrc)CMPI_RC_ERROR_SYSTEM, \
            (CMPIString*)string2CMPIString("Unknown exception")); \
        PEG_METHOD_EXIT(); \
        return returnvalue; \
    }

#define HandlerCatchReturnStatus() \
    catch (const CIMException &e) \
    { \
        PEG_TRACE(( \
            TRC_CMPIPROVIDERINTERFACE, \
            Tracer::LEVEL2, \
            "CIMException: %s",(const char*)e.getMessage().getCString())); \
        PEG_METHOD_EXIT(); \
        CMReturnWithString( \
            (CMPIrc)e.getCode(), \
            (CMPIString*)string2CMPIString(e.getMessage())); \
    } \
    catch (const Exception &e) \
    { \
        PEG_TRACE(( \
            TRC_CMPIPROVIDERINTERFACE, \
            Tracer::LEVEL2, \
            "Exception: %s", (const char *)e.getMessage().getCString())); \
        PEG_METHOD_EXIT(); \
        CMReturnWithString( \
            (CMPIrc)CMPI_RC_ERROR_SYSTEM, \
            (CMPIString*)string2CMPIString(e.getMessage())); \
    } \
    catch (...) \
    { \
        PEG_TRACE(( \
            TRC_CMPIPROVIDERINTERFACE, \
            Tracer::LEVEL2, \
            "Unknown exception")); \
        PEG_METHOD_EXIT(); \
        CMReturnWithString( \
            (CMPIrc)CMPI_RC_ERROR_SYSTEM, \
            (CMPIString*)string2CMPIString("Unknown exception")); \
    }

static CIMPropertyList getList(const char** l)
{
    CIMPropertyList pl;
    if (l)
    {
        Array<CIMName> n;
        while (*l)
        {
            n.append(*l++);
        }
        pl.set(n);
    }
    return pl;
}

SCMOClass* mbGetSCMOClass(
    const char* nameSpace,
    Uint32 nsL,
    const char* cls,
    Uint32 clsL)
{
    PEG_METHOD_ENTER(TRC_CMPIPROVIDERINTERFACE, "CMPI_Broker:mbGetSCMOClass()");

    const CMPIBroker * mb = CMPI_ThreadContext::getBroker();
    CMPI_Broker *xBroker = (CMPI_Broker*)mb;

    const char* ns=nameSpace;
    if (0 == nsL)
    {
        //If we don't have a namespace here, we use the initnamespace from
        // the thread context, since we need one to be able to lookup the class
        const CMPIContext* ctx = CMPI_ThreadContext::getContext();
        if (0!=ctx)
        {
            CMPIStatus rc;
            CMPIData nsCtxData = CMGetContextEntry(ctx, CMPIInitNameSpace,&rc);
            if (rc.rc == CMPI_RC_OK)
            {
                ns = CMGetCharsPtr(nsCtxData.value.string, 0);
                nsL = strlen(ns);
            }
        }

    }


    SCMOClass* scmoCls =
        xBroker->classCache.getSCMOClass(xBroker, ns, nsL, cls, clsL);

    PEG_METHOD_EXIT();
    return scmoCls;
}

extern "C"
{

    static CMPIInstance* mbGetInstance(
        const CMPIBroker *mb,
        const CMPIContext *ctx,
        const CMPIObjectPath *cop,
        const char **properties,
        CMPIStatus *rc)
    {
        PEG_METHOD_ENTER(
            TRC_CMPIPROVIDERINTERFACE,
            "CMPI_Broker:mbGetInstance()");

        mb = CM_BROKER;
        CMPIFlags flgs =
            ctx->ft->getEntry(ctx,CMPIInvocationFlags,NULL).value.uint32;
        const CIMPropertyList props = getList(properties);

        SCMOInstance* scmoObjPath = SCMO_ObjectPath(cop);
        CIMObjectPath qop;
        try
        {
            scmoObjPath->getCIMObjectPath(qop);

            CIMResponseData resData = CM_CIMOM(mb)->getInstance(
                *CM_Context(ctx),
                scmoObjPath->getNameSpace(),
                qop,
                CM_IncludeQualifiers(flgs),
                CM_ClassOrigin(flgs),
                props);

            // When running out of process the returned instances don't contain
            // a namespace.
            // Add the namespace from the input parameters where neccessary
            resData.completeNamespace(SCMO_ObjectPath(cop));

            SCMOInstance& scmoOrgInst = resData.getSCMO()[0];

            SCMOInstance* scmoInst = new SCMOInstance(scmoOrgInst);

            // Rebuild the objectPath
            scmoInst->buildKeyBindingsFromProperties();

            CMPIInstance* cmpiInst = reinterpret_cast<CMPIInstance*>(
                new CMPI_Object(scmoInst,CMPI_Object::ObjectTypeInstance));

            CMSetStatus(rc,CMPI_RC_OK);
            PEG_METHOD_EXIT();
            return cmpiInst;
        }
        HandlerCatchSetStatus(rc, NULL);

        // Code flow should never get here.
    }

    static CMPIObjectPath* mbCreateInstance(
        const CMPIBroker *mb,
        const CMPIContext *ctx,
        const CMPIObjectPath *cop,
        const CMPIInstance *ci,
        CMPIStatus *rc)
    {
        PEG_METHOD_ENTER(
            TRC_CMPIPROVIDERINTERFACE,
            "CMPI_Broker:mbCreateInstance()");

        mb = CM_BROKER;

        SCMOInstance* scmoInst = SCMO_Instance(ci);
        CIMInstance inst;
        try
        {
            scmoInst->getCIMInstance(inst);

            CIMObjectPath ncop = CM_CIMOM(mb)->createInstance(
                *CM_Context(ctx),
                scmoInst->getNameSpace(),
                inst);

            SCMOInstance* newScmoInst=
                CMPISCMOUtilities::getSCMOFromCIMObjectPath(
                    ncop,
                    scmoInst->getNameSpace());

            CMPIObjectPath* cmpiObjPath = reinterpret_cast<CMPIObjectPath*>(
                new CMPI_Object(newScmoInst,CMPI_Object::ObjectTypeObjectPath));

            CMSetStatus(rc,CMPI_RC_OK);
            PEG_METHOD_EXIT();
            return cmpiObjPath;
        }
        HandlerCatchSetStatus(rc, NULL);

        // Code flow should never get here.
    }

    static CMPIStatus mbModifyInstance(
        const CMPIBroker *mb,
        const CMPIContext *ctx,
        const CMPIObjectPath *cop,
        const CMPIInstance *ci,
        const char ** properties)
    {
        PEG_METHOD_ENTER(
            TRC_CMPIPROVIDERINTERFACE,
            "CMPI_Broker:mbModifyInstance()");
        mb = CM_BROKER;
        CMPIFlags flgs =
            ctx->ft->getEntry(ctx,CMPIInvocationFlags,NULL).value.uint32;
        const CIMPropertyList props = getList(properties);

        SCMOInstance* scmoInst = SCMO_Instance(ci);
        CIMInstance inst;
        try
        {
            scmoInst->getCIMInstance(inst);

            CM_CIMOM(mb)->modifyInstance(
                *CM_Context(ctx),
                SCMO_ObjectPath(cop)->getNameSpace(),
                inst,
                CM_IncludeQualifiers(flgs),
                props);
        }
        HandlerCatchReturnStatus();

        PEG_METHOD_EXIT();
        CMReturn(CMPI_RC_OK);
    }

    static CMPIStatus mbDeleteInstance(
        const CMPIBroker *mb,
        const CMPIContext *ctx,
        const CMPIObjectPath *cop)
    {
        PEG_METHOD_ENTER(
            TRC_CMPIPROVIDERINTERFACE,
            "CMPI_Broker:mbDeleteInstance()");
        mb = CM_BROKER;

        SCMOInstance* scmoObjPath = SCMO_ObjectPath(cop);
        CIMObjectPath qop;
        try
        {
            scmoObjPath->getCIMObjectPath(qop);

            CM_CIMOM(mb)->deleteInstance(
                *CM_Context(ctx),
                SCMO_ObjectPath(cop)->getNameSpace(),
                qop);
        }
        HandlerCatchReturnStatus();

        PEG_METHOD_EXIT();
        CMReturn(CMPI_RC_OK);
    }

    static CMPIEnumeration* mbExecQuery(
        const CMPIBroker *mb,
        const CMPIContext *ctx,
        const CMPIObjectPath *cop,
        const char *query, const char *lang, CMPIStatus *rc)
    {
        PEG_METHOD_ENTER(
            TRC_CMPIPROVIDERINTERFACE,
            "CMPI_Broker:mbExecQuery()");
        mb = CM_BROKER;

        try
        {
            CIMResponseData resData = CM_CIMOM(mb)->execQuery(
                *CM_Context(ctx),
                SCMO_ObjectPath(cop)->getNameSpace(),
                String(lang),
                String(query));

            // When running out of process the returned instances don't contain
            // a namespace.
            // Add the namespace from the input parameters where neccessary
            resData.completeNamespace(SCMO_ObjectPath(cop));

            Array<SCMOInstance>* aObj =
                new Array<SCMOInstance>(resData.getSCMO());


            CMPIEnumeration* cmpiEnum = reinterpret_cast<CMPIEnumeration*>(
                new CMPI_Object(new CMPI_ObjEnumeration(aObj)));

            CMSetStatus(rc,CMPI_RC_OK);
            PEG_METHOD_EXIT();
            return cmpiEnum;
        }
        HandlerCatchSetStatus(rc, NULL);

        // Code flow should never get here.
    }

    static CMPIEnumeration* mbEnumInstances(
        const CMPIBroker *mb,
        const CMPIContext *ctx,
        const CMPIObjectPath *cop,
        const char **properties,
        CMPIStatus *rc)
    {
        PEG_METHOD_ENTER(
            TRC_CMPIPROVIDERINTERFACE,
            "CMPI_Broker:mbEnumInstances()");
        mb = CM_BROKER;

        CMPIFlags flgs =
            ctx->ft->getEntry(ctx,CMPIInvocationFlags,NULL).value.uint32;
        const CIMPropertyList props = getList(properties);

        try
        {
            CIMResponseData resData =
                CM_CIMOM(mb)->enumerateInstances(
                    *CM_Context(ctx),
                    SCMO_ObjectPath(cop)->getNameSpace(),
                    SCMO_ObjectPath(cop)->getClassName(),
                    true,
                    CM_IncludeQualifiers(flgs),
                    CM_ClassOrigin(flgs),
                    props);

            // When running out of process the returned instances don't contain
            // a namespace.
            // Add the namespace from the input parameters where neccessary
            resData.completeNamespace(SCMO_ObjectPath(cop));

            Array<SCMOInstance>* aInst =
                new Array<SCMOInstance>(resData.getSCMO());

            CMPIEnumeration* cmpiEnum = reinterpret_cast<CMPIEnumeration*>(
                new CMPI_Object(new CMPI_InstEnumeration(aInst)));

            CMSetStatus(rc,CMPI_RC_OK);
            PEG_METHOD_EXIT();
            return cmpiEnum;
        }
        HandlerCatchSetStatus(rc, NULL);

        // Code flow should never get here.
    }

    static CMPIEnumeration* mbEnumInstanceNames(
        const CMPIBroker *mb,
        const CMPIContext *ctx,
        const CMPIObjectPath *cop,
        CMPIStatus *rc)
    {
        PEG_METHOD_ENTER(
            TRC_CMPIPROVIDERINTERFACE,
            "CMPI_Broker:mbEnumInstanceNames()");
        mb = CM_BROKER;

        try
        {
            CIMResponseData resData =
                CM_CIMOM(mb)->enumerateInstanceNames(
                    *CM_Context(ctx),
                    SCMO_ObjectPath(cop)->getNameSpace(),
                    SCMO_ObjectPath(cop)->getClassName());

            // When running out of process the returned instances don't contain
            // a namespace.
            // Add the namespace from the input parameters where neccessary
            resData.completeNamespace(SCMO_ObjectPath(cop));

            Array<SCMOInstance>* aRef =
                new Array<SCMOInstance>(resData.getSCMO());


            CMPIEnumeration* cmpiEnum = reinterpret_cast<CMPIEnumeration*>(
                new CMPI_Object(new CMPI_OpEnumeration(aRef)));

            CMSetStatus(rc,CMPI_RC_OK);
            PEG_METHOD_EXIT();
            return cmpiEnum;
        }
        HandlerCatchSetStatus(rc, NULL);

        // Code flow should never get here.
    }

    static CMPIEnumeration* mbAssociators(
        const CMPIBroker *mb,
        const CMPIContext *ctx,
        const CMPIObjectPath *cop,
        const char *assocClass,
        const char *resultClass,
        const char *role,
        const char *resultRole,
        const char **properties,
        CMPIStatus *rc)
    {
        PEG_METHOD_ENTER(
            TRC_CMPIPROVIDERINTERFACE,
            "CMPI_Broker:mbAssociators()");
        mb = CM_BROKER;
        // OpenPegasus does not allow singleton classes so that
        // a cop with no keys is illegal here.  The test is important
        // since it insures that the associators request function is
        // not called with zero keys since that function uses the
        // existence of keys to determine if it is a class or instance
        // operation. See BUG_3302
        if (!SCMO_ObjectPath(cop)->getKeyBindingCount())
        {
            CMSetStatus(rc, CMPI_RC_ERR_FAILED);
            PEG_METHOD_EXIT();
            return 0;
        }
        CMPIFlags flgs =
            ctx->ft->getEntry(ctx,CMPIInvocationFlags,NULL).value.uint32;
        const CIMPropertyList props = getList(properties);

        SCMOInstance* scmoObjPath = SCMO_ObjectPath(cop);
        CIMObjectPath qop;
        try
        {
            scmoObjPath->getCIMObjectPath(qop);
            // For compatibility with previous implementations have empty ns
            qop.setNameSpace(CIMNamespaceName());

            CIMResponseData resData =
                CM_CIMOM(mb)->associators(
                    *CM_Context(ctx),
                    SCMO_ObjectPath(cop)->getNameSpace(),
                    qop,
                    assocClass ? CIMName(assocClass) : CIMName(),
                    resultClass ? CIMName(resultClass) : CIMName(),
                    role ? String(role) : String::EMPTY,
                    resultRole ? String(resultRole) : String::EMPTY,
                    CM_IncludeQualifiers(flgs),
                    CM_ClassOrigin(flgs),
                    props);

            // When running out of process the returned instances don't contain
            // a namespace.
            // Add the namespace from the input parameters where neccessary
            resData.completeNamespace(scmoObjPath);

            Array<SCMOInstance>* aObj =
                new Array<SCMOInstance>(resData.getSCMO());

            CMPIEnumeration* cmpiEnum = reinterpret_cast<CMPIEnumeration*>(
                new CMPI_Object(new CMPI_ObjEnumeration(aObj)));

            CMSetStatus(rc,CMPI_RC_OK);
            PEG_METHOD_EXIT();
            return cmpiEnum;
        }
        HandlerCatchSetStatus(rc, NULL);

        // Code flow should never get here.
    }

    static CMPIEnumeration* mbAssociatorNames(
        const CMPIBroker *mb,
        const CMPIContext *ctx,
        const CMPIObjectPath *cop,
        const char *assocClass,
        const char *resultClass,
        const char *role,
        const char *resultRole,
        CMPIStatus *rc)
    {
        PEG_METHOD_ENTER(
            TRC_CMPIPROVIDERINTERFACE,
            "CMPI_Broker:mbAssociatorNames()");
        mb = CM_BROKER;
        // OpenPegasus does not allow singleton classes so that
        // a cop with no keys is illegal here.  The test is important
        // since it insures that the associatorNames function is
        // not called with zero keys since that function uses the
        // existence of keys to determine if it is a class or instance
        // operation. See BUG_3302
        if (!SCMO_ObjectPath(cop)->getKeyBindingCount())
        {
            CMSetStatus(rc, CMPI_RC_ERR_FAILED);
            PEG_METHOD_EXIT();
            return 0;
        }

        SCMOInstance* scmoObjPath = SCMO_ObjectPath(cop);
        CIMObjectPath qop;
        try
        {
            scmoObjPath->getCIMObjectPath(qop);
            // For compatibility with previous implementations have empty ns
            qop.setNameSpace(CIMNamespaceName());

            CIMResponseData resData =
                CM_CIMOM(mb)->associatorNames(
                    *CM_Context(ctx),
                    scmoObjPath->getNameSpace(),
                    qop,
                    assocClass ? CIMName(assocClass) : CIMName(),
                    resultClass ? CIMName(resultClass) : CIMName(),
                    role ? String(role) : String::EMPTY,
                    resultRole ? String(resultRole) : String::EMPTY);

            // When running out of process the returned instances don't contain
            // a namespace.
            // Add the namespace from the input parameters where neccessary
            resData.completeNamespace(scmoObjPath);

            Array<SCMOInstance>* aRef =
                new Array<SCMOInstance>(resData.getSCMO());


            CMPIEnumeration* cmpiEnum = reinterpret_cast<CMPIEnumeration*>(
                new CMPI_Object(new CMPI_OpEnumeration(aRef)));

            CMSetStatus(rc,CMPI_RC_OK);
            PEG_METHOD_EXIT();
            return cmpiEnum;
        }
        HandlerCatchSetStatus(rc, NULL);

        // Code flow should never get here.
    }

    static CMPIEnumeration* mbReferences(
        const CMPIBroker *mb,
        const CMPIContext *ctx,
        const CMPIObjectPath *cop,
        const char *resultClass,
        const char *role ,
        const char **properties,
        CMPIStatus *rc)
    {
        PEG_METHOD_ENTER(
            TRC_CMPIPROVIDERINTERFACE,
            "CMPI_Broker:mbReferences()");
        mb = CM_BROKER;
        // OpenPegasus does not allow singleton classes so that
        // a cop with no keys is illegal here.  The test is important
        // since it insures that the references function is
        // not called with zero keys since that function uses the
        // existence of keys to determine if it is a class or instance
        // operation. See BUG_3302
        if (!SCMO_ObjectPath(cop)->getKeyBindingCount())
        {
            CMSetStatus(rc, CMPI_RC_ERR_FAILED);
            PEG_METHOD_EXIT();
            return 0;
        }
        CMPIFlags flgs =
           ctx->ft->getEntry(ctx,CMPIInvocationFlags,NULL).value.uint32;
        CIMPropertyList props = getList(properties);


        SCMOInstance* scmoObjPath = SCMO_ObjectPath(cop);
        CIMObjectPath qop;
        try
        {
            scmoObjPath->getCIMObjectPath(qop);
            // For compatibility with previous implementations have empty ns
            qop.setNameSpace(CIMNamespaceName());

            CIMResponseData resData =
                CM_CIMOM(mb)->references(
                    *CM_Context(ctx),
                    scmoObjPath->getNameSpace(),
                    qop,
                    resultClass ? CIMName(resultClass) : CIMName(),
                    role ? String(role) : String::EMPTY,
                    CM_IncludeQualifiers(flgs),
                    CM_ClassOrigin(flgs),
                    props);

            CMSetStatus(rc,CMPI_RC_OK);

            // Add the namespace from the input parameters when neccessary
            resData.completeNamespace(scmoObjPath);

            Array<SCMOInstance>* aObj =
                new Array<SCMOInstance>(resData.getSCMO());

            CMPIEnumeration* cmpiEnum = reinterpret_cast<CMPIEnumeration*>(
                new CMPI_Object(new CMPI_ObjEnumeration(aObj)));
            PEG_METHOD_EXIT();
            return cmpiEnum;
        }
        HandlerCatchSetStatus(rc, NULL);

        // Code flow should never get here.
    }

    static CMPIEnumeration* mbReferenceNames(
        const CMPIBroker *mb,
        const CMPIContext *ctx,
        const CMPIObjectPath *cop,
        const char *resultClass,
        const char *role,
        CMPIStatus *rc)
    {
        PEG_METHOD_ENTER(
            TRC_CMPIPROVIDERINTERFACE,
            "CMPI_Broker:mbReferenceNames()");
        mb = CM_BROKER;
        // OpenPegasus does not allow singleton classes so that
        // a cop with no keys is illegal here.  The test is important
        // since it insures that the referenceNames function is
        // not called with zero keys since that function uses the
        // existence of keys to determine if it is a class or instance
        // operation. See BUG_3302
        if (!SCMO_ObjectPath(cop)->getKeyBindingCount())
        {
            CMSetStatus(rc, CMPI_RC_ERR_FAILED);
            PEG_METHOD_EXIT();
            return 0;
        }

        SCMOInstance* scmoObjPath = SCMO_ObjectPath(cop);
        CIMObjectPath qop;
        try
        {
            scmoObjPath->getCIMObjectPath(qop);
            // For compatibility with previous implementations have empty ns
            qop.setNameSpace(CIMNamespaceName());

            CIMResponseData resData =
                CM_CIMOM(mb)->referenceNames(
                    *CM_Context(ctx),
                    scmoObjPath->getNameSpace(),
                    qop,
                    resultClass ? CIMName(resultClass) : CIMName(),
                    role ? String(role) : String::EMPTY);

            // Add the namespace from the input parameters when neccessary
            resData.completeNamespace(scmoObjPath);

            Array<SCMOInstance>* aRef =
                new Array<SCMOInstance>(resData.getSCMO());

            CMPIEnumeration* cmpiEnum = reinterpret_cast<CMPIEnumeration*>(
                new CMPI_Object(new CMPI_OpEnumeration(aRef)));
            CMSetStatus(rc,CMPI_RC_OK);
            PEG_METHOD_EXIT();
            return cmpiEnum;
        }
        HandlerCatchSetStatus(rc, NULL);

        // Code flow should never get here.
    }

#define CM_Args(args) ((Array<CIMParamValue>*)args->hdl)

    static CMPIData mbInvokeMethod(
        const CMPIBroker *mb,
        const CMPIContext *ctx,
        const CMPIObjectPath *cop,
        const char *method,
        const CMPIArgs *in,
        CMPIArgs *out,
        CMPIStatus *rc)
    {
        PEG_METHOD_ENTER(
            TRC_CMPIPROVIDERINTERFACE,
            "CMPI_Broker:mbInvokeMethod()");
        CMPIData data = {0,CMPI_nullValue,{0}};
        mb = CM_BROKER;


        SCMOInstance* scmoObjPath = SCMO_ObjectPath(cop);
        CIMObjectPath qop;
        try
        {
            scmoObjPath->getCIMObjectPath(qop);

            CIMValue v = CM_CIMOM(mb)->invokeMethod(
                *CM_Context(ctx),
                SCMO_ObjectPath(cop)->getNameSpace(),
                qop,
                method ? String(method) : String::EMPTY,
                *CM_Args(in),
                *CM_Args(out));

            CIMType vType=v.getType();
            CMPIType t = type2CMPIType(vType,v.isArray());
            value2CMPIData(v,t,&data);

            if (rc)
            {
                CMSetStatus(rc,CMPI_RC_OK);
            }
        }
        HandlerCatchSetStatus(rc, data);

        PEG_METHOD_EXIT();
        return data; // "data" will be valid data or nullValue (in error case)
    }

    static CMPIStatus mbSetProperty(
        const CMPIBroker *mb,
        const CMPIContext *ctx,
        const CMPIObjectPath *cop,
        const char *name,
        const CMPIValue *val,
        CMPIType type)
    {
        PEG_METHOD_ENTER(
            TRC_CMPIPROVIDERINTERFACE,
            "CMPI_Broker:mbSetProperty()");
        mb = CM_BROKER;
        CMPIrc rc;
        CIMValue v = value2CIMValue(val,type,&rc);

        SCMOInstance* scmoObjPath = SCMO_ObjectPath(cop);
        CIMObjectPath qop;
        try
        {
            scmoObjPath->getCIMObjectPath(qop);

            CM_CIMOM(mb)->setProperty(
                *CM_Context(ctx),
                SCMO_ObjectPath(cop)->getNameSpace(),
                qop,
                String(name),
                v);
        }
        HandlerCatchReturnStatus();

        PEG_METHOD_EXIT();
        CMReturn(CMPI_RC_OK);
    }

    static CMPIData mbGetProperty(
        const CMPIBroker *mb,
        const CMPIContext *ctx,
        const CMPIObjectPath *cop,
        const char *name,
        CMPIStatus *rc)
    {
        PEG_METHOD_ENTER(
            TRC_CMPIPROVIDERINTERFACE,
            "CMPI_Broker:mbGetProperty()");
        mb = CM_BROKER;
        CMPIData data = {0,CMPI_nullValue,{0}};

        SCMOInstance* scmoObjPath = SCMO_ObjectPath(cop);
        CIMObjectPath qop;
        scmoObjPath->getCIMObjectPath(qop);

        try
        {
            CIMValue v = CM_CIMOM(mb)->getProperty(
                *CM_Context(ctx),
                SCMO_ObjectPath(cop)->getNameSpace(),
                qop,
                String(name));
            CIMType vType = v.getType();
            CMPIType t = type2CMPIType(vType,v.isArray());
            value2CMPIData(v,t,&data);
            CMSetStatus(rc,CMPI_RC_OK);
        }
        HandlerCatchSetStatus(rc, data);

        PEG_METHOD_EXIT();
        return data; // "data" will be valid data or nullValue (in error case)
    }

    /* With Bug#8541 the CMPI Provider Manager was changed to attach the
       complete requests operation context to the CMPI thread context.
       In future, when we have a lot more containers on the operation context,
       this might lead to an impact on memory usage here, when the entire
       operation context gets copied to the new thread.
    */
    static CMPIContext* mbPrepareAttachThread(
        const CMPIBroker* mb,
        const CMPIContext* eCtx)
    {
        PEG_METHOD_ENTER(
            TRC_CMPIPROVIDERINTERFACE,
            "CMPI_Broker:mbPrepareAttachThread()");
        mb = CM_BROKER;
        OperationContext *ctx = (OperationContext*)((CMPI_Context*)eCtx)->ctx;
        OperationContext nctx = *ctx;
        CMPIContext* neCtx = new CMPI_Context(*(new OperationContext(nctx)));
        CMPIString *name;
        for (int i=0,s=CMPI_Args_Ftab->getArgCount(
            reinterpret_cast<const CMPIArgs*>(eCtx),NULL); i<s; i++)
        {
            CMPIData data = CMPI_Args_Ftab->getArgAt(
                reinterpret_cast<const CMPIArgs*>(eCtx),i,&name,NULL);
            CMPI_Args_Ftab->addArg(
                reinterpret_cast<CMPIArgs*>(neCtx),
                CMGetCharsPtr(name,NULL),
                &data.value,data.type);
        }
        PEG_METHOD_EXIT();
        return neCtx;
    }

    static CMPIStatus mbAttachThread(
        const CMPIBroker* mb,
        const CMPIContext* eCtx)
    {
        ((CMPI_Context*)eCtx)->thr = new CMPI_ThreadContext(mb,eCtx);
        CMReturn(CMPI_RC_OK);
    }

    static CMPIStatus mbDetachThread(
        const CMPIBroker* mb,
        const CMPIContext* eCtx)
    {
        mb = CM_BROKER;
        CMPI_Context *neCtx = (CMPI_Context *)eCtx;
        delete neCtx->thr;
        // Delete also CMPIContext
        delete neCtx;
        CMReturn(CMPI_RC_OK);
    }

    static CMPIStatus mbDeliverIndication(
        const CMPIBroker* eMb,
        const CMPIContext* ctx,
        const char *ns,
        const CMPIInstance* ind)
    {
        PEG_METHOD_ENTER(
            TRC_CMPIPROVIDERINTERFACE,
            "CMPI_Broker:mbDeliverIndication()");
        // If no valid broker was passed in we try the use the broker
        // that was stored in the local thread context
        if (eMb==NULL)
        {
        eMb = CM_BROKER;
        }
        CMPI_Broker *mb = (CMPI_Broker*)eMb;
        IndProvRecord *indProvRec;
        OperationContext* context = CM_Context(ctx);

        SCMOInstance* scmoInst = SCMO_Instance(ind);
        CIMInstance indInst;
        scmoInst->getCIMInstance(indInst);

        // When an indication to be delivered comes from Remote providers,
        // the CMPIBroker contains the name of the provider in the form
        // of physical-name:logical-name. Search using logical-name. -V 5884
        String provider_name;
        CMPIUint32 n;

        if ( (n = mb->name.find(':') ) != PEG_NOT_FOUND)
        {
            provider_name = mb->name.subString (n + 1);
        }
        else
        {
            provider_name = mb->name;
        }
        ReadLock readLock(CMPIProviderManager::rwSemProvTab);
        if (CMPIProviderManager::indProvTab.lookup(provider_name, indProvRec))
        {
            if (indProvRec->isEnabled())
            {
                if (!context->contains(
                    SubscriptionInstanceNamesContainer::NAME))
                {
                    Array<CIMObjectPath> subscriptionInstanceNames;
                    context->insert(
                        SubscriptionInstanceNamesContainer(
                            subscriptionInstanceNames));
                }
                CIMIndication cimIndication(indInst);
                try
                {
                    indProvRec->getHandler()->deliver(
                        *context,
   //                   OperationContext(*CM_Context(ctx)),
                        cimIndication);
                    PEG_METHOD_EXIT();
                    CMReturn(CMPI_RC_OK);
                }
                HandlerCatchReturnStatus();
            }
        }
        PEG_METHOD_EXIT();
        CMReturn(CMPI_RC_ERR_FAILED);
    }
}

static CMPIBrokerFT broker_FT =
{
    MB_CAPABILITIES, // brokerClassification;
    CMPICurrentVersion,
    "Pegasus",
    mbPrepareAttachThread,
    mbAttachThread,
    mbDetachThread,
    mbDeliverIndication,
    mbEnumInstanceNames,
    mbGetInstance,
    mbCreateInstance,
    mbModifyInstance,
    mbDeleteInstance,
    mbExecQuery,
    mbEnumInstances,
    mbAssociators,
    mbAssociatorNames,
    mbReferences,
    mbReferenceNames,
    mbInvokeMethod,
    mbSetProperty,
    mbGetProperty,
};

CMPIBrokerFT *CMPI_Broker_Ftab = & broker_FT;

PEGASUS_NAMESPACE_END
