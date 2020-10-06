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

#include "CMPI_ContextArgs.h"
#include "CMPI_Ftabs.h"
#include "CMPI_Value.h"
#include "CMPI_String.h"
#include <Pegasus/Common/CIMNameCast.h>
#include <Pegasus/Common/Tracer.h>

#include <string.h>

PEGASUS_USING_STD;
PEGASUS_NAMESPACE_BEGIN

// CMPIArgs section

extern "C"
{

    static CMPIStatus argsRelease(CMPIArgs* eArg)
    {
        PEG_METHOD_ENTER(
            TRC_CMPIPROVIDERINTERFACE,
            "CMPI_ContextArgs:argsRelease()");
        Array<CIMParamValue>* arg = (Array<CIMParamValue>*)eArg->hdl;
        if (arg)
        {
            delete arg;
            (reinterpret_cast<CMPI_Object*>(eArg))->unlinkAndDelete();
            PEG_METHOD_EXIT();
            CMReturn(CMPI_RC_OK);
        }
        PEG_METHOD_EXIT();
        CMReturn(CMPI_RC_ERR_INVALID_HANDLE);
    }

    static CMPIStatus argsReleaseNop(CMPIArgs* eArg)
    {
        CMReturn(CMPI_RC_OK);
    }

    static CMPIArgs* argsClone(const CMPIArgs* eArg, CMPIStatus* rc)
    {
        PEG_METHOD_ENTER(
            TRC_CMPIPROVIDERINTERFACE,
            "CMPI_ContextArgs:argsClone()");
        Array<CIMParamValue>* arg = (Array<CIMParamValue>*)eArg->hdl;
        if (!arg)
        {
            PEG_TRACE_CSTRING(
                TRC_CMPIPROVIDERINTERFACE,
                Tracer::LEVEL1,
                "Invalid Handle - eArg->hdl in \
                CMPI_ContextArgs:argsClone");
            CMSetStatus(rc, CMPI_RC_ERR_INVALID_HANDLE);
            PEG_METHOD_EXIT();
            return 0;
        }
        Array<CIMParamValue>* cArg = new Array<CIMParamValue>();
        for (long i=0,s=arg->size(); i<s; i++)
        {
            const CIMParamValue &v = (*arg)[i];
            cArg->append(v.clone());
        }
        CMPI_Object* obj = new CMPI_Object(cArg);
        obj->unlink();
        CMPIArgs* neArg = reinterpret_cast<CMPIArgs*>(obj);
        CMSetStatus(rc,CMPI_RC_OK);
        PEG_METHOD_EXIT();
        return neArg;
    }

    static long locateArg(
        const Array<CIMParamValue> &a,
        const CIMName &eName)
    {
        for (long i=0,s=a.size(); i<s; i++)
        {
            const String &n = a[i].getParameterName();
            if (String::equalNoCase(n,eName.getString()))
            {
                return i;
            }
        }
        return -1;
    }

    static CMPIStatus argsAddArg(
        const CMPIArgs* eArg,
        const char *name,
        const CMPIValue* data,
        const CMPIType type)
    {
        PEG_METHOD_ENTER(
            TRC_CMPIPROVIDERINTERFACE,
            "CMPI_ContextArgs:argsAddArg()");
        Array<CIMParamValue>* arg = (Array<CIMParamValue>*)eArg->hdl;
        if (!arg)
        {
            PEG_TRACE_CSTRING(
                TRC_CMPIPROVIDERINTERFACE,
                Tracer::LEVEL1,
                "Invalid Handle - eArg->hdl in \
                CMPI_ContextArgs:argsAddArg");
            PEG_METHOD_EXIT();
            CMReturn(CMPI_RC_ERR_INVALID_HANDLE);
        }
        if (!name)
        {
            PEG_TRACE_CSTRING(
                TRC_CMPIPROVIDERINTERFACE,
                Tracer::LEVEL1,
                "Invalid Parameter - name in \
                CMPI_ContextArgs:argsAddArg");
            PEG_METHOD_EXIT();
            CMReturn(CMPI_RC_ERR_INVALID_PARAMETER);
        }
        CMPIrc rc;
        CIMValue v = value2CIMValue(data,type,&rc);
        CIMName sName(name);

        long i = locateArg(*arg,sName);
        if (i>=0)
        {
            arg->remove(i);
        }

        arg->append(CIMParamValue(sName.getString(), v));
        PEG_METHOD_EXIT();
        CMReturn(CMPI_RC_OK);
    }

    static CMPIData argsGetArgAt(
        const CMPIArgs* eArg,
        CMPICount pos,
        CMPIString** name,
        CMPIStatus* rc)
    {
        Array<CIMParamValue>* arg = (Array<CIMParamValue>*)eArg->hdl;
        CMPIData data = {0,CMPI_nullValue | CMPI_notFound,{0}};
        if (!arg)
        {
            PEG_TRACE_CSTRING(
                TRC_CMPIPROVIDERINTERFACE,
                Tracer::LEVEL1,
                "Invalid handle eArg->hdl in \
                CMPI_ContextArgs:argsGetArgAt");
            CMSetStatus(rc, CMPI_RC_ERR_INVALID_HANDLE);
            return data;
        }

        if (pos > arg->size())
        {
            PEG_TRACE_CSTRING(
                TRC_CMPIPROVIDERINTERFACE,
                Tracer::LEVEL1,
                "Property Not Found in CMPI_ContextArgs:argsGetArgAt");
            CMSetStatus(rc, CMPI_RC_ERR_NO_SUCH_PROPERTY);
            return data;
        }

        CIMValue v = (*arg)[pos].getValue();
        CIMType pType = v.getType();
        CMPIType t = type2CMPIType(pType,v.isArray());

        value2CMPIData(v,t,&data);

        if (name)
        {
            String n = (*arg)[pos].getParameterName();
            *name = (CMPIString*)string2CMPIString(n);
        }

        CMSetStatus(rc,CMPI_RC_OK);
        return data;
    }

    static CMPIData argsGetArg(
        const CMPIArgs* eArg,
        const char *name,
        CMPIStatus* rc)
    {
        Array<CIMParamValue>* arg = (Array<CIMParamValue>*)eArg->hdl;
        CMPIData data = {0,CMPI_nullValue | CMPI_notFound,{0}};
        if (!arg)
        {
            PEG_TRACE_CSTRING(
                TRC_CMPIPROVIDERINTERFACE,
                Tracer::LEVEL1,
                "Invalid Handle - eArg->hdl in CMPI_ContextArgs:argsGetArg");
            CMSetStatus(rc, CMPI_RC_ERR_INVALID_HANDLE);
            return data;
        }
        if (!name)
        {
            PEG_TRACE_CSTRING(
                TRC_CMPIPROVIDERINTERFACE,
                Tracer::LEVEL1,
                "Invalid Parameter - name in \
                CMPI_ContextArgs:argsGetArg");
            CMSetStatus(rc, CMPI_RC_ERR_INVALID_PARAMETER);
            return data;
        }
        long i = locateArg(*arg, CIMNameCast(name));
        if (i>=0)
        {
            return argsGetArgAt(eArg, i, NULL, rc);
        }

        CMSetStatus(rc, CMPI_RC_ERR_NO_SUCH_PROPERTY);
        return data;
    }

    static CMPICount argsGetArgCount(const CMPIArgs* eArg, CMPIStatus* rc)
    {
        Array<CIMParamValue>* arg = (Array<CIMParamValue>*)eArg->hdl;
        if (!arg)
        {
            PEG_TRACE_CSTRING(
                TRC_CMPIPROVIDERINTERFACE,
                Tracer::LEVEL1,
                "Invalid Handle - eArg->hdl in \
                CMPI_ContextArgs:argsGetArgCount");
            CMSetStatus(rc, CMPI_RC_ERR_INVALID_HANDLE);
            return 0;
        }
        CMSetStatus(rc,CMPI_RC_OK);
        return arg->size();
    }

}

static CMPIArgsFT args_FT =
{
    CMPICurrentVersion,
    argsRelease,
    argsClone,
    argsAddArg,
    argsGetArg,
    argsGetArgAt,
    argsGetArgCount,
};

CMPIArgsFT *CMPI_Args_Ftab = &args_FT;

static CMPIArgsFT argsOnStack_FT =
{
    CMPICurrentVersion,
    argsReleaseNop,
    argsClone,
    argsAddArg,
    argsGetArg,
    argsGetArgAt,
    argsGetArgCount,
};

CMPIArgsFT *CMPI_ArgsOnStack_Ftab = &argsOnStack_FT;



// CMPIContext Session

extern "C"
{

    static CMPIStatus contextReleaseNop(CMPIContext* eCtx)
    {
        CMReturn(CMPI_RC_OK);
    }

    static CMPIData contextGetEntry(
        const CMPIContext* eCtx,
        const char *name,
        CMPIStatus* rc)
    {
        return argsGetArg(reinterpret_cast<const CMPIArgs*>(eCtx), name, rc);
    }

    CMPIData contextGetEntryAt(
        const CMPIContext* eCtx,
        CMPICount pos,
        CMPIString** name,
        CMPIStatus* rc)
    {
        return (argsGetArgAt(
            reinterpret_cast<const CMPIArgs*>(eCtx), pos, name, rc));
    }

    static CMPICount contextGetEntryCount(
        const CMPIContext* eCtx,
        CMPIStatus* rc)
    {
        return argsGetArgCount(reinterpret_cast<const CMPIArgs*>(eCtx), rc);
    }

    static CMPIStatus contextAddEntry(
        const CMPIContext* eCtx,
        const char *name,
        const CMPIValue* data,
        const CMPIType type)
    {
        PEG_METHOD_ENTER(
            TRC_CMPIPROVIDERINTERFACE,
            "CMPI_ContextArgs:contextAddEntry()");

        Boolean invalidType = false;
        Boolean invalidContext = false;

        if (!name || !data)
        {
            PEG_TRACE_CSTRING(
                TRC_CMPIPROVIDERINTERFACE,
                Tracer::LEVEL1,
                "Invalid Parameter - name || data in \
                CMPI_ContextArgs:contextAddEntry");
            PEG_METHOD_EXIT();
            CMReturn(CMPI_RC_ERR_INVALID_PARAMETER);
        }

        if (strcmp(name,SnmpTrapOidContainer::NAME.getCString()) == 0)
        {
            OperationContext *ctx = ((CMPI_Context*)eCtx)->ctx;
            if (!ctx)
            {
                invalidContext = true;
            }
            else if (type == CMPI_chars)
            {
                if (ctx->contains(SnmpTrapOidContainer::NAME))
                {
                    ctx->set(SnmpTrapOidContainer((char*)data));
                }
                else
                {
                    ctx->insert(SnmpTrapOidContainer((char*)data));
                }
                PEG_METHOD_EXIT();
                CMReturn(CMPI_RC_OK);
            }
            else if (type == CMPI_string)
            {
                if (ctx->contains(SnmpTrapOidContainer::NAME))
                {
                    ctx->set(SnmpTrapOidContainer((char*)data->string->hdl));
                }
                else
                {
                    ctx->insert(SnmpTrapOidContainer((char*)data->string->hdl));
                }
                PEG_METHOD_EXIT();
                CMReturn(CMPI_RC_OK);
            }
            else
            {
                invalidType = true;
            }
        }
        else if (!strcmp(name,
            SubscriptionInstanceNamesContainer::NAME.getCString()))
        {
            OperationContext *ctx = ((CMPI_Context*)eCtx)->ctx;
            if (!ctx)
            {
                invalidContext = true;
            }
            else if (type == CMPI_refA)
            {
                Array<CIMObjectPath> names;
                CMPI_Array *arr = (CMPI_Array*)data->array->hdl;
                CMPIData* dta=(CMPIData*)arr->hdl;
                for (Uint32 i = 1 ; i <= dta->value.uint32 ; i++)
                {
                    SCMOInstance *ref = (SCMOInstance*)(dta[i].value.ref->hdl);
                    CIMObjectPath path;
                    ref->getCIMObjectPath(path);
                    names.append(path);
                }
                if (ctx->contains(SubscriptionInstanceNamesContainer::NAME))
                {
                    ctx->set(SubscriptionInstanceNamesContainer(names));
                }
                else
                {
                    ctx->insert(SubscriptionInstanceNamesContainer(names));
                }
                PEG_METHOD_EXIT();
                CMReturn(CMPI_RC_OK);
            }
            else
            {
                invalidType = true;
            }
        }

        if (invalidType)
        {
            PEG_TRACE_CSTRING(
                TRC_CMPIPROVIDERINTERFACE,
                Tracer::LEVEL1,
                "Received Invalid Data Type in"
                    " CMPI_COntextArgs:contextAddEntry");
                PEG_METHOD_EXIT();
                CMReturn(CMPI_RC_ERR_INVALID_DATA_TYPE);
        }
        else if (invalidContext)
        {
            PEG_TRACE_CSTRING(
               TRC_CMPIPROVIDERINTERFACE,
               Tracer::LEVEL1,
               "Invalid Handle - eCtx->ctx in"
                   " CMPI_ContextArgs:contextAddEntry");
            PEG_METHOD_EXIT();
            CMReturn(CMPI_RC_ERR_INVALID_HANDLE);
        }

        CMPIStatus stat = argsAddArg(
            reinterpret_cast<const CMPIArgs*>(eCtx), name, data, type);

        PEG_METHOD_EXIT();
        return stat;
    }
}

static CMPIContextFT context_FT =
{
    CMPICurrentVersion,
    contextReleaseNop,
    NULL,
    contextGetEntry,
    contextGetEntryAt,
    contextGetEntryCount,
    contextAddEntry,
};

CMPIContextFT *CMPI_Context_Ftab = &context_FT;

static CMPIContextFT contextOnStack_FT =
{
    CMPICurrentVersion,
    contextReleaseNop,
    NULL,
    contextGetEntry,
    contextGetEntryAt,
    contextGetEntryCount,
    contextAddEntry,
};

CMPIContextFT *CMPI_ContextOnStack_Ftab = &contextOnStack_FT;


CMPI_Context::CMPI_Context(const OperationContext& ct)
{
    PEG_METHOD_ENTER(
        TRC_CMPIPROVIDERINTERFACE,
        "CMPI_Context::CMPI_Context()");
    ctx = (OperationContext*)&ct;
    thr = NULL;
    hdl = (void*)new Array<CIMParamValue>();
    ft = CMPI_Context_Ftab;
    PEG_METHOD_EXIT();
}

CMPI_Context::~CMPI_Context()
{
    PEG_METHOD_ENTER(
        TRC_CMPIPROVIDERINTERFACE,
        "CMPI_Context::~CMPI_Context()");
    delete (Array<CIMParamValue>*)hdl;
    delete ctx;
    PEG_METHOD_EXIT();
}
CMPI_ContextOnStack::CMPI_ContextOnStack(const OperationContext& ct)
{
    PEG_METHOD_ENTER(
        TRC_CMPIPROVIDERINTERFACE,
        "CMPI_ContextOnStack::CMPI_ContextOnStack()");
    ctx = (OperationContext*)&ct;
    hdl = (void*)new Array<CIMParamValue>();
    ft = CMPI_ContextOnStack_Ftab;
    PEG_METHOD_EXIT();
}

CMPI_ContextOnStack::~CMPI_ContextOnStack()
{
    PEG_METHOD_ENTER(
        TRC_CMPIPROVIDERINTERFACE,
        "CMPI_ContextOnStack::~CMPI_ContextOnStack()");
    delete (Array<CIMParamValue>*)hdl;
    PEG_METHOD_EXIT();
}

CMPI_ArgsOnStack::CMPI_ArgsOnStack(const Array<CIMParamValue>& args)
{
    PEG_METHOD_ENTER(
        TRC_CMPIPROVIDERINTERFACE,
        "CMPI_ArgsOnStack::CMPI_ArgsOnStack");
    hdl = (void*)&args;
    ft = CMPI_ArgsOnStack_Ftab;
    PEG_METHOD_EXIT();
}

PEGASUS_NAMESPACE_END







