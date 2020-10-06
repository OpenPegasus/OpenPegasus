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

#include "CMPI_Result.h"
#include "CMPI_Ftabs.h"
#include "CMPI_Value.h"
#include "CMPI_String.h"
#include <typeinfo>

#include <Pegasus/ProviderManager2/SimpleResponseHandler.h>
#include <Pegasus/Common/System.h>
#include <Pegasus/Common/CIMType.h>
#include <Pegasus/Common/Mutex.h>
#include <string.h>
#include <Pegasus/Common/Tracer.h>
#include <Pegasus/ProviderManager2/CMPI/CMPI_ThreadContext.h>

PEGASUS_USING_STD;
PEGASUS_NAMESPACE_BEGIN

Mutex errorChainMutex;

extern "C"
{

    // Gets the invaction flags from the thread context and sets them
    // on an SCMOInstance object
    PEGASUS_STATIC inline void appendInvocationFlags(SCMOInstance& inst)
    {
        const CMPIContext *ctx = CMPI_ThreadContext::getContext();
        if (0!=ctx)
        {
            CMPIFlags flgs = ctx->ft->getEntry(
                ctx,CMPIInvocationFlags,NULL).value.uint32;
            if (flgs & CMPI_FLAG_IncludeQualifiers)
            {
                inst.includeQualifiers();
            }
            if (flgs & CMPI_FLAG_IncludeClassOrigin)
            {
                inst.includeClassOrigins();
            }
        }
    }

    PEGASUS_STATIC CMPIStatus resultReturnData(
        const CMPIResult* eRes,
        const CMPIValue* data,
        const CMPIType type)
    {
        PEG_METHOD_ENTER(
            TRC_CMPIPROVIDERINTERFACE,
            "CMPI_Result:resultReturnData()");
        CMPIrc rc;
        if (eRes->hdl == NULL)
        {
            PEG_TRACE_CSTRING(
                TRC_CMPIPROVIDERINTERFACE,
                Tracer::LEVEL1,
                "Received invalid handle in CMPI_Result:resultReturnData");
            PEG_METHOD_EXIT();
            CMReturn(CMPI_RC_ERR_INVALID_HANDLE);
        }
        if (data == NULL)
        {
            PEG_TRACE_CSTRING(
                TRC_CMPIPROVIDERINTERFACE,
                Tracer::LEVEL1,
                "Received Invalid Parameter in CMPI_Result:resultReturnData");
            PEG_METHOD_EXIT();
            CMReturn(CMPI_RC_ERR_INVALID_PARAMETER);
        }

        try
        {
            CIMValue v=value2CIMValue((CMPIValue*)data,type,&rc);
            if (eRes->ft==CMPI_ResultMethOnStack_Ftab)
            {
                MethodResultResponseHandler* res=
                    (MethodResultResponseHandler*)eRes->hdl;
                if ((((CMPI_Result*)eRes)->flags & RESULT_set)==0)
                {
                    res->processing();
                    ((CMPI_Result*)eRes)->flags|=RESULT_set;
                }
              /**
                 If the CMPI type is CMPI_instance, then the CIM return type
                 could be either an EmbeddedObject or EmbeddedInstance. We
                 must find the method signature in the class definition to find
                 out what the return type is.
             */
                if (type == CMPI_instance)
                {
                    try
                    {
                        InvokeMethodResponseHandler * opRes =
                            dynamic_cast<InvokeMethodResponseHandler *>(res);
                        PEGASUS_ASSERT(opRes != 0);
                        CIMInvokeMethodRequestMessage * request =
                            dynamic_cast<CIMInvokeMethodRequestMessage *>(
                            opRes->getRequest());
                        PEGASUS_ASSERT(request != 0);

                        const CachedClassDefinitionContainer * classContainer =
                           dynamic_cast<const CachedClassDefinitionContainer *>
                            (&(request->operationContext.get(
                            CachedClassDefinitionContainer::NAME)));
                        PEGASUS_ASSERT(classContainer != 0);

                        CIMConstClass classDef(classContainer->getClass());
                        Uint32 methodIndex = classDef.findMethod(
                            request->methodName);
                        if (methodIndex == PEG_NOT_FOUND)
                        {
                            String message(
                                "Method not found in class definition");
                            PEG_METHOD_EXIT();
                            CMReturnWithString(CMPI_RC_ERR_FAILED,
                                (CMPIString*)string2CMPIString(message));
                        }

                        CIMConstMethod methodDef(
                            classDef.getMethod(methodIndex));
                        if (methodDef.findQualifier(
                               PEGASUS_QUALIFIERNAME_EMBEDDEDINSTANCE)
                            != PEG_NOT_FOUND)
                        {
                            PEGASUS_ASSERT(v.getType() == CIMTYPE_OBJECT);
                            CIMObject tmpObject;
                            v.get(tmpObject);
                            v = CIMValue(CIMInstance(tmpObject));
                        }
                    }
                    catch (Exception & e)
                    {
                        PEG_METHOD_EXIT();
                        CMReturnWithString(CMPI_RC_ERR_FAILED,
                            (CMPIString*)string2CMPIString(e.getMessage()));
                    }
                }
                res->deliver(v);
            }
            else
            {
                ValueResponseHandler* res=(ValueResponseHandler*)eRes->hdl;
                if ((((CMPI_Result*)eRes)->flags & RESULT_set)==0)
                {
                    res->processing();
                    ((CMPI_Result*)eRes)->flags|=RESULT_set;
                }
                res->deliver(v);
            }
        }
        catch (const CIMException &e)
        {
            PEG_TRACE((TRC_CMPIPROVIDERINTERFACE,Tracer::LEVEL1,
                "CIMException: resultReturnData - msg: %s",
                (const char*)e.getMessage().getCString()));
            PEG_METHOD_EXIT();
            CMReturnWithString(
                CMPI_RC_ERR_FAILED,
                (CMPIString*)string2CMPIString(e.getMessage()));
        }
        PEG_METHOD_EXIT();
        CMReturn(CMPI_RC_OK);
    }

    PEGASUS_STATIC CMPIStatus resultReturnInstance(
        const CMPIResult* eRes,
        const CMPIInstance* eInst)
    {
        PEG_METHOD_ENTER(
            TRC_CMPIPROVIDERINTERFACE,
            "CMPI_Result:resultReturnInstance()");
        SimpleInstanceResponseHandler* res=
            (SimpleInstanceResponseHandler*)eRes->hdl;
        if ((res == NULL) || (eInst == NULL))
        {
            PEG_TRACE((
                TRC_CMPIPROVIDERINTERFACE,
                ((res==0) ? (Tracer::LEVEL1) : (Tracer::LEVEL3)),
                "Invalid parameter res (%p) || eInst (%p) "
                    "in CMPI_Result:resultReturnInstance",
                res,
                eInst));
            PEG_METHOD_EXIT();
            CMReturn(CMPI_RC_ERR_INVALID_PARAMETER);
        }

        if (!eInst->hdl)
        {
            PEG_TRACE_CSTRING(
                TRC_CMPIPROVIDERINTERFACE,
                Tracer::LEVEL1,
                "Invalid parameter eInst->hdl in \
                CMPI_Result:resultReturnInstance");
            PEG_METHOD_EXIT();
            CMReturn(CMPI_RC_ERR_INVALID_PARAMETER);
        }

        try
        {
            if ((((CMPI_Result*)eRes)->flags & RESULT_set)==0)
            {
                res->processing();
                ((CMPI_Result*)eRes)->flags|=RESULT_set;
            }
            SCMOInstance& inst=*(SCMOInstance*)(eInst->hdl);

            appendInvocationFlags(inst);

            // Ensure that the instance includes a valid ObjectPath with
            // all key properties set, for which the according property
            // has been set on the instance.
            inst.buildKeyBindingsFromProperties();

            res->deliver(inst);
        }
        catch (const CIMException &e)
        {
            PEG_TRACE((TRC_CMPIPROVIDERINTERFACE,Tracer::LEVEL1,
                "CIMException: resultReturnInstance - msg: %s",
                (const char*)e.getMessage().getCString()));
            PEG_METHOD_EXIT();
            CMReturnWithString(
                CMPI_RC_ERR_FAILED,
                (CMPIString*)string2CMPIString(e.getMessage()));
        }
        PEG_METHOD_EXIT();
        CMReturn(CMPI_RC_OK);
    }

    PEGASUS_STATIC CMPIStatus resultReturnObject(
        const CMPIResult* eRes,
        const CMPIInstance* eInst)
    {
        PEG_METHOD_ENTER(
            TRC_CMPIPROVIDERINTERFACE,
            "CMPI_Result:resultReturnObject()");
        SimpleObjectResponseHandler* res=
            (SimpleObjectResponseHandler*)eRes->hdl;

        if ((res == NULL) || (eInst == NULL))
        {
            PEG_TRACE((
                TRC_CMPIPROVIDERINTERFACE,
                ((res==0) ? (Tracer::LEVEL1) : (Tracer::LEVEL3)),
                "Invalid parameter res (%p) || eInst (%p) "
                    "in CMPI_Result:resultReturnObject",
                res,
                eInst));
            PEG_METHOD_EXIT();
            CMReturn(CMPI_RC_ERR_INVALID_PARAMETER);
        }

        if (!eInst->hdl)
        {
            PEG_TRACE_CSTRING(
                TRC_CMPIPROVIDERINTERFACE,
                Tracer::LEVEL1,
                "Invalid parameter eInst->hdl in \
                CMPI_Result:resultReturnObject");
            PEG_METHOD_EXIT();
            CMReturn(CMPI_RC_ERR_INVALID_PARAMETER);
        }

        try
        {
            if ((((CMPI_Result*)eRes)->flags & RESULT_set)==0)
            {
                res->processing();
                ((CMPI_Result*)eRes)->flags|=RESULT_set;
            }
            SCMOInstance& inst=*(SCMOInstance*)(eInst->hdl);

            appendInvocationFlags(inst);

            // Ensure that the instance includes a valid ObjectPath with
            // all key properties set, for which the according property
            // has been set on the instance.
            inst.buildKeyBindingsFromProperties();

            res->deliver(inst);
        }
        catch (const CIMException &e)
        {
            PEG_TRACE((TRC_CMPIPROVIDERINTERFACE,Tracer::LEVEL1,
                "CIMException: resultReturnObject - msg: %s",
                (const char*)e.getMessage().getCString()));

            PEG_METHOD_EXIT();
            CMReturnWithString(
                CMPI_RC_ERR_FAILED,
                (CMPIString*)string2CMPIString(e.getMessage()));
        }
        PEG_METHOD_EXIT();
        CMReturn(CMPI_RC_OK);
    }

    PEGASUS_STATIC CMPIStatus resultReturnExecQuery(
        const CMPIResult* eRes,
        const CMPIInstance* eInst)
    {
        PEG_METHOD_ENTER(
            TRC_CMPIPROVIDERINTERFACE,
            "CMPI_Result:resultReturnExecQuery()");
        ExecQueryResponseHandler* res=(ExecQueryResponseHandler*)eRes->hdl;
        if ((res == NULL) || (eInst == NULL))
        {
            PEG_TRACE((
                TRC_CMPIPROVIDERINTERFACE,
                ((res==0) ? (Tracer::LEVEL1) : (Tracer::LEVEL3)),
                "Invalid parameter res (%p) || eInst (%p) "
                    "in CMPI_Result:resultReturnExecQuery",
                res,
                eInst));
            PEG_METHOD_EXIT();
            CMReturn(CMPI_RC_ERR_INVALID_PARAMETER);
        }

        if (!eInst->hdl)
        {
            PEG_TRACE_CSTRING(
                TRC_CMPIPROVIDERINTERFACE,
                Tracer::LEVEL1,
                "Invalid parameter eInst->hdl in \
                CMPI_Result:resultReturnExecQuery");
            PEG_METHOD_EXIT();
            CMReturn(CMPI_RC_ERR_INVALID_PARAMETER);
        }

        try
        {
            if ((((CMPI_Result*)eRes)->flags & RESULT_set)==0)
            {
                res->processing();
                ((CMPI_Result*)eRes)->flags|=RESULT_set;
            }
            SCMOInstance& inst=*(SCMOInstance*)(eInst->hdl);
            
            appendInvocationFlags(inst);

            // Ensure that the instance includes a valid ObjectPath with
            // all key properties set, for which the according property
            // has been set on the instance.
            inst.buildKeyBindingsFromProperties();

            res->deliver(inst);
        }
        catch (const CIMException &e)
        {
            PEG_TRACE((TRC_CMPIPROVIDERINTERFACE,Tracer::LEVEL1,
                "CIMException: resultReturnExecQuery - msg: %s",
                (const char*)e.getMessage().getCString()));
            PEG_METHOD_EXIT();
            CMReturnWithString(
                CMPI_RC_ERR_FAILED,
                (CMPIString*)string2CMPIString(e.getMessage()));
        }
        PEG_METHOD_EXIT();
        CMReturn(CMPI_RC_OK);
    }
    PEGASUS_STATIC CMPIStatus resultReturnObjectPath(
        const CMPIResult* eRes,
        const CMPIObjectPath* eRef)
    {
        PEG_METHOD_ENTER(
            TRC_CMPIPROVIDERINTERFACE,
            "CMPI_Result:resultReturnObjectPath()");
        SimpleObjectPathResponseHandler* res=
            (SimpleObjectPathResponseHandler*)eRes->hdl;

        if ((res == NULL) || (eRef == NULL))
        {
            PEG_TRACE((
                TRC_CMPIPROVIDERINTERFACE,
                ((res==0) ? (Tracer::LEVEL1) : (Tracer::LEVEL3)),
                "Invalid parameter res (%p) || eRef (%p) "
                    "in CMPI_Result:resultReturnObjectPath",
                res,
                eRef));
            PEG_METHOD_EXIT();
            CMReturn(CMPI_RC_ERR_INVALID_PARAMETER);
        }
        if (!eRef->hdl)
        {
            PEG_TRACE_CSTRING(
                TRC_CMPIPROVIDERINTERFACE,
                Tracer::LEVEL1,
                "Invalid parameter eRef->hdl in \
                CMPI_Result:resultReturnObjectPath");
            PEG_METHOD_EXIT();
            CMReturn(CMPI_RC_ERR_INVALID_PARAMETER);
        }

        try
        {
            if ((((CMPI_Result*)eRes)->flags & RESULT_set)==0)
            {
                res->processing();
                ((CMPI_Result*)eRes)->flags|=RESULT_set;
            }
            SCMOInstance& ref=*(SCMOInstance*)(eRef->hdl);
            res->deliver(ref);
        }
        catch (const CIMException &e)
        {
            PEG_TRACE((TRC_CMPIPROVIDERINTERFACE,Tracer::LEVEL1,
                "CIMException: resultReturnObjectPath - msg: %s",
                (const char*)e.getMessage().getCString()));

            PEG_METHOD_EXIT();
            CMReturnWithString(
                CMPI_RC_ERR_FAILED,
                (CMPIString*)string2CMPIString(e.getMessage()));
        }
        PEG_METHOD_EXIT();
        CMReturn(CMPI_RC_OK);
    }

    PEGASUS_STATIC CMPIStatus resultReturnInstDone(const CMPIResult* eRes)
    {
        PEG_METHOD_ENTER(
            TRC_CMPIPROVIDERINTERFACE,
            "CMPI_Result:resultReturnInstDone()");
        InstanceResponseHandler* res=(InstanceResponseHandler*)eRes->hdl;
        if (!res)
        {
            PEG_TRACE_CSTRING(
                TRC_CMPIPROVIDERINTERFACE,
                Tracer::LEVEL1,
                "Invalid parameter in \
                CMPI_Result:resultReturnInstDone");
            PEG_METHOD_EXIT();
            CMReturn(CMPI_RC_ERR_INVALID_PARAMETER);
        }

        try
        {
            if ((((CMPI_Result*)
               eRes)->flags & RESULT_set)==0) res->processing();
            res->complete();
            ((CMPI_Result*)eRes)->flags|=(RESULT_done | RESULT_set);
        }
        catch (const CIMException &e)
        {
            PEG_TRACE((TRC_CMPIPROVIDERINTERFACE,Tracer::LEVEL1,
                "CIMException: resultReturnInstDone - msg: %s",
                (const char*)e.getMessage().getCString()));

            PEG_METHOD_EXIT();
            CMReturnWithString(
                CMPI_RC_ERR_FAILED,
                (CMPIString*)string2CMPIString(e.getMessage()));
        }
        PEG_METHOD_EXIT();
        CMReturn(CMPI_RC_OK);
    }

    PEGASUS_STATIC CMPIStatus resultReturnRefDone(const CMPIResult* eRes)
    {
        PEG_METHOD_ENTER(
            TRC_CMPIPROVIDERINTERFACE,
            "CMPI_Result:resultReturnRefDone()");
        ObjectPathResponseHandler* res=(ObjectPathResponseHandler*)eRes->hdl;
        if (!res)
        {
            PEG_TRACE_CSTRING(
                TRC_CMPIPROVIDERINTERFACE,
                Tracer::LEVEL1,
                "Invalid parameter in \
                CMPI_Result:resultReturnRefDone");
            PEG_METHOD_EXIT();
            CMReturn(CMPI_RC_ERR_INVALID_PARAMETER);
        }

        try
        {
            if ((((CMPI_Result*)eRes)->flags & RESULT_set)==0)
                res->processing();
            res->complete();
            ((CMPI_Result*)eRes)->flags|=(RESULT_done | RESULT_set);
        }
        catch (const CIMException &e)
        {
            PEG_TRACE((TRC_CMPIPROVIDERINTERFACE,Tracer::LEVEL1,
                "CIMException: resultReturnRefDone - msg: %s",
                (const char*)e.getMessage().getCString()));

            PEG_METHOD_EXIT();
            CMReturnWithString(
                CMPI_RC_ERR_FAILED,
                (CMPIString*)string2CMPIString(e.getMessage()));
        }
        PEG_METHOD_EXIT();
        CMReturn(CMPI_RC_OK);
    }

    PEGASUS_STATIC CMPIStatus resultReturnDataDone(const CMPIResult* eRes)
    {
        PEG_METHOD_ENTER(
            TRC_CMPIPROVIDERINTERFACE,
            "CMPI_Result:resultReturnDataDone()");
        ResponseHandler* res=(ResponseHandler*)eRes->hdl;
        if (!res)
        {
            PEG_TRACE_CSTRING(
                TRC_CMPIPROVIDERINTERFACE,
                Tracer::LEVEL1,
                "Invalid parameter in \
                CMPI_Result:resultReturnDataDone");
            PEG_METHOD_EXIT();
            CMReturn(CMPI_RC_ERR_INVALID_PARAMETER);
        }

        try
        {
            if ((((CMPI_Result*)
               eRes)->flags & RESULT_set)==0) res->processing();
            res->complete();
            ((CMPI_Result*)eRes)->flags|=(RESULT_done | RESULT_set);
        }
        catch (const CIMException &e)
        {
            PEG_TRACE((TRC_CMPIPROVIDERINTERFACE,Tracer::LEVEL1,
                "CIMException: resultReturnDataDone - msg: %s",
                (const char*)e.getMessage().getCString()));

            PEG_METHOD_EXIT();
            CMReturnWithString(
                CMPI_RC_ERR_FAILED,
                (CMPIString*)string2CMPIString(e.getMessage()));
        }
        PEG_METHOD_EXIT();
        CMReturn(CMPI_RC_OK);
    }

    PEGASUS_STATIC CMPIStatus resultReturnMethDone(const CMPIResult* eRes)
    {
        PEG_METHOD_ENTER(
            TRC_CMPIPROVIDERINTERFACE,
            "CMPI_Result:resultReturnMethDone()");
        MethodResultResponseHandler* res=
            (MethodResultResponseHandler*)eRes->hdl;
        if (!res)
        {
            PEG_TRACE_CSTRING(
                TRC_CMPIPROVIDERINTERFACE,
                Tracer::LEVEL1,
                "Invalid parameter in \
                CMPI_Result:resultReturnMethDone");
            PEG_METHOD_EXIT();
            CMReturn(CMPI_RC_ERR_INVALID_PARAMETER);
        }

        try
        {
            if ((((CMPI_Result*)eRes)->flags & RESULT_set)==0)
                 res->processing();
            //   res->complete();    // Do not close the handle
            ((CMPI_Result*)eRes)->flags|=(RESULT_done | RESULT_set);
        }
        catch (const CIMException &e)
        {
            PEG_TRACE((TRC_CMPIPROVIDERINTERFACE,Tracer::LEVEL1,
                "CIMException: resultReturnMethDone - msg: %s",
                (const char*)e.getMessage().getCString()));

            PEG_METHOD_EXIT();
            CMReturnWithString(
                CMPI_RC_ERR_FAILED,
                (CMPIString*)string2CMPIString(e.getMessage()));
        }
        PEG_METHOD_EXIT();
        CMReturn(CMPI_RC_OK);
    }

    PEGASUS_STATIC CMPIStatus resultReturnObjDone(const CMPIResult* eRes)
    {
        PEG_METHOD_ENTER(
            TRC_CMPIPROVIDERINTERFACE,
            "CMPI_Result:resultReturnObjDone()");
        ObjectResponseHandler* res=(ObjectResponseHandler*)eRes->hdl;
        if (!res)
        {
            PEG_TRACE_CSTRING(
                TRC_CMPIPROVIDERINTERFACE,
                Tracer::LEVEL1,
                "Invalid parameter in \
                CMPI_Result:resultReturnObjDone");
            PEG_METHOD_EXIT();
            CMReturn(CMPI_RC_ERR_INVALID_PARAMETER);
        }

        try
        {
            if ((((CMPI_Result*)
               eRes)->flags & RESULT_set)==0) res->processing();
            res->complete();
            ((CMPI_Result*)eRes)->flags|=(RESULT_done | RESULT_set);
        }
        catch (const CIMException &e)
        {
            PEG_TRACE((TRC_CMPIPROVIDERINTERFACE,Tracer::LEVEL1,
                "CIMException: resultReturnObjDone - msg: %s",
                (const char*)e.getMessage().getCString()));

            PEG_METHOD_EXIT();
            CMReturnWithString(
                CMPI_RC_ERR_FAILED,
               (CMPIString*)string2CMPIString(e.getMessage()));
        }
        PEG_METHOD_EXIT();
        CMReturn(CMPI_RC_OK);
    }

    PEGASUS_STATIC CMPIStatus resultReturnExecQueryDone(const CMPIResult* eRes)
    {
        PEG_METHOD_ENTER(
            TRC_CMPIPROVIDERINTERFACE,
            "CMPI_Result:resultReturnExecQueryDone()");
        ExecQueryResponseHandler* res=(ExecQueryResponseHandler*)eRes->hdl;
        if (!res)
        {
            PEG_TRACE_CSTRING(
                TRC_CMPIPROVIDERINTERFACE,
                Tracer::LEVEL1,
                "Invalid parameter in \
                CMPI_Result:resultReturnExecQueryDone");
            PEG_METHOD_EXIT();
            CMReturn(CMPI_RC_ERR_INVALID_PARAMETER);
        }

        try
        {
            if ((((CMPI_Result*)eRes)->flags & RESULT_set)==0)
                res->processing();
            res->complete();
            ((CMPI_Result*)eRes)->flags|=(RESULT_done | RESULT_set);
        }
        catch (const CIMException &e)
        {
            PEG_TRACE((TRC_CMPIPROVIDERINTERFACE,Tracer::LEVEL1,
                "CIMException: resultReturnExecQueryDone - msg: %s",
                (const char*)e.getMessage().getCString()));

            PEG_METHOD_EXIT();
            CMReturnWithString(
                CMPI_RC_ERR_FAILED,
                (CMPIString*)string2CMPIString(e.getMessage()));
        }
        PEG_METHOD_EXIT();
        CMReturn(CMPI_RC_OK);
    }

    PEGASUS_STATIC CMPIStatus resultReturnError(
        const CMPIResult* eRes,
        const CMPIError* er)
    {
        PEG_METHOD_ENTER(
            TRC_CMPIPROVIDERINTERFACE,
            "CMPI_Result:resultReturnError()");
        CMPIStatus rrc={CMPI_RC_OK,NULL};

        if (eRes->hdl == NULL)
        {
            PEG_TRACE_CSTRING(
                TRC_CMPIPROVIDERINTERFACE,
                Tracer::LEVEL1,
                "Invalid parameter eRes->hdl in \
                CMPI_Result:resultReturnError");
            PEG_METHOD_EXIT();
            CMReturn(CMPI_RC_ERR_INVALID_PARAMETER);
        }

        if (er == NULL)
        {
            PEG_TRACE_CSTRING(
                TRC_CMPIPROVIDERINTERFACE,
                Tracer::LEVEL1,
                "Invalid handle in \
                CMPI_Result:resultReturnError");
            PEG_METHOD_EXIT();
            CMReturn(CMPI_RC_ERR_INVALID_HANDLE);
        }

        CMPIError *clonedError = er->ft->clone(er,&rrc);
        if (rrc.rc != CMPI_RC_OK)
        {
            PEG_METHOD_EXIT();
            return rrc;
        }

        AutoMutex mtx(errorChainMutex);
        ((CMPI_Error*)clonedError)->nextError = ((CMPI_Result*)eRes)->resError;
        ((CMPI_Result*)eRes)->resError = (CMPI_Error*)clonedError;
        PEG_METHOD_EXIT();
        CMReturn(CMPI_RC_OK);
    }

    PEGASUS_STATIC CMPIStatus resultBadReturnData(
        const CMPIResult* eRes,
        const CMPIValue* data,
        const CMPIType type)
    {
        CMReturn(CMPI_RC_ERR_NOT_SUPPORTED);
    }

    PEGASUS_STATIC CMPIStatus resultBadReturnInstance(
       const CMPIResult* eRes,
       const CMPIInstance* eInst)
    {
        CMReturn(CMPI_RC_ERR_NOT_SUPPORTED);
    }

    PEGASUS_STATIC CMPIStatus resultBadReturnObjectPath(
        const CMPIResult* eRes,
        const CMPIObjectPath* eRef)
    {
        CMReturn(CMPI_RC_ERR_NOT_SUPPORTED);
    }

}

static CMPIResultFT resultMethOnStack_FT=
{
    CMPICurrentVersion,
    NULL,
    NULL,
    resultReturnData,
    resultBadReturnInstance,
    resultBadReturnObjectPath,
    resultReturnMethDone,
    resultReturnError
};

static CMPIResultFT resultObjOnStack_FT=
{
    CMPICurrentVersion,
    NULL,
    NULL,
    resultBadReturnData,
    resultReturnObject,
    resultBadReturnObjectPath,
    resultReturnObjDone,
    resultReturnError
};

static CMPIResultFT resultExecQueryOnStack_FT=
{
    CMPICurrentVersion,
    NULL,
    NULL,
    resultBadReturnData,
    resultReturnExecQuery,
    resultBadReturnObjectPath,
    resultReturnExecQueryDone,
    resultReturnError
};

static CMPIResultFT resultData_FT=
{
    CMPICurrentVersion,
    NULL,
    NULL,
    resultReturnData,
    resultBadReturnInstance,
    resultBadReturnObjectPath,
    resultReturnDataDone,
    resultReturnError
};

static CMPIResultFT resultInstOnStack_FT=
{
    CMPICurrentVersion,
    NULL,
    NULL,
    resultBadReturnData,
    resultReturnInstance,
    resultBadReturnObjectPath,
    resultReturnInstDone,
    resultReturnError
};

static CMPIResultFT resultRefOnStack_FT=
{
    CMPICurrentVersion,
    NULL,
    NULL,
    resultBadReturnData,
    resultBadReturnInstance,
    resultReturnObjectPath,
    resultReturnRefDone,
    resultReturnError
};

static CMPIResultFT resultResponseOnStack_FT=
{
    CMPICurrentVersion,
    NULL,
    NULL,
    resultBadReturnData,
    resultBadReturnInstance,
    resultBadReturnObjectPath,
    resultReturnDataDone,
    resultReturnError
};

CMPIResultFT *CMPI_ResultMeth_Ftab=&resultMethOnStack_FT;
CMPIResultFT *CMPI_ResultData_Ftab=&resultData_FT;
CMPIResultFT *CMPI_ResultMethOnStack_Ftab=&resultMethOnStack_FT;
CMPIResultFT *CMPI_ResultInstOnStack_Ftab=&resultInstOnStack_FT;
CMPIResultFT *CMPI_ResultObjOnStack_Ftab=&resultObjOnStack_FT;
CMPIResultFT *CMPI_ResultRefOnStack_Ftab=&resultRefOnStack_FT;
CMPIResultFT *CMPI_ResultResponseOnStack_Ftab=&resultResponseOnStack_FT;
CMPIResultFT *CMPI_ResultExecQueryOnStack_Ftab=&resultExecQueryOnStack_FT;

CMPI_ResultOnStack::CMPI_ResultOnStack(
    const SimpleObjectPathResponseHandler & handler,
    CMPI_Broker *xMb)
{
    hdl=(void*)&handler;
    xBroker=xMb;
    ft=CMPI_ResultRefOnStack_Ftab;
    flags=RESULT_ObjectPath;
    resError=NULL;
}

CMPI_ResultOnStack::CMPI_ResultOnStack(
    const SimpleInstanceResponseHandler& handler,
    CMPI_Broker *xMb)
{
    hdl=(void*)&handler;
    xBroker=xMb;
    ft=CMPI_ResultInstOnStack_Ftab;
    flags=RESULT_Instance;
    resError=NULL;
}

CMPI_ResultOnStack::CMPI_ResultOnStack(
    const SimpleObjectResponseHandler& handler,
    CMPI_Broker *xMb)
{
    hdl=(void*)&handler;
    xBroker=xMb;
    ft=CMPI_ResultObjOnStack_Ftab;
    flags=RESULT_Object;
    resError=NULL;
}

CMPI_ResultOnStack::CMPI_ResultOnStack(
    const MethodResultResponseHandler& handler,
    CMPI_Broker *xMb)
{
    hdl=(void*)&handler;
    xBroker=xMb;
    ft=CMPI_ResultMethOnStack_Ftab;
    flags=RESULT_Method;
    resError=NULL;
}

CMPI_ResultOnStack::CMPI_ResultOnStack(
    const ResponseHandler& handler,
    CMPI_Broker *xMb)
{
    hdl=(void*)&handler;
    xBroker=xMb;
    ft=CMPI_ResultResponseOnStack_Ftab;
    flags=RESULT_Response;
    resError=NULL;
}

CMPI_ResultOnStack::CMPI_ResultOnStack(
    const ExecQueryResponseHandler& handler,
    CMPI_Broker *xMb)
{
    hdl=(void*)&handler;
    xBroker=xMb;
    ft=CMPI_ResultExecQueryOnStack_Ftab;
    flags=RESULT_Object;
    resError=NULL;
}

CMPI_ResultOnStack::~CMPI_ResultOnStack()
{
    try
    {
        if (resError)
        {
            CMPI_Error* nextErr = NULL;
            for (CMPI_Error* currErr=resError;
                currErr!=NULL;
                currErr=nextErr)
            {
                nextErr = currErr->nextError;
                ((CMPIError*)currErr)->ft->release(currErr);
            }
        }
        if ((flags & RESULT_set)==0)
        {
            if (ft==CMPI_ResultRefOnStack_Ftab)
               ((SimpleObjectPathResponseHandler*)hdl)->processing();
            else
            if (ft==CMPI_ResultInstOnStack_Ftab)
                ((SimpleInstanceResponseHandler*)hdl)->processing();
            else
            if (ft==CMPI_ResultObjOnStack_Ftab)
               ((SimpleObjectResponseHandler*)hdl)->processing();
            else
            if (ft==CMPI_ResultMethOnStack_Ftab)
               ((MethodResultResponseHandler*)hdl)->processing();
            else
            if (ft==CMPI_ResultResponseOnStack_Ftab)
               ((ResponseHandler*)hdl)->processing();
            else
            if (ft==CMPI_ResultExecQueryOnStack_Ftab)
               ((ExecQueryResponseHandler*)hdl)->processing();
            else
               ((ResponseHandler*)hdl)->processing();  // shoul not get here
        }
        if ((flags & RESULT_done)==0)
        {
            if (ft==CMPI_ResultRefOnStack_Ftab)
               ((SimpleObjectPathResponseHandler*)hdl)->complete();
            else
            if (ft==CMPI_ResultInstOnStack_Ftab)
               ((SimpleInstanceResponseHandler*)hdl)->complete();
            else
            if (ft==CMPI_ResultObjOnStack_Ftab)
               ((SimpleObjectResponseHandler*)hdl)->complete();
            else
            if (ft==CMPI_ResultMethOnStack_Ftab)
               ((MethodResultResponseHandler*)hdl)->complete();
            else
            if (ft==CMPI_ResultResponseOnStack_Ftab)
               ((ResponseHandler*)hdl)->complete();
            else
            if (ft==CMPI_ResultExecQueryOnStack_Ftab)
               ((ExecQueryResponseHandler*)hdl)->complete();
            else ((ResponseHandler*)hdl)->complete();  // shoul not get here
        }
    }
    catch (const CIMException &)
    {
        // Ignore the exception
    }
}

PEGASUS_NAMESPACE_END
