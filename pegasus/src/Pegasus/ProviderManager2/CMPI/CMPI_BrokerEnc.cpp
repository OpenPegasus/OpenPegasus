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

#include "CMPI_Object.h"
#include "CMPI_ThreadContext.h"
#include "CMPI_Broker.h"
#include "CMPI_Ftabs.h"
#include "CMPI_String.h"
#include "CMPI_SelectExp.h"
#include "CMPI_Array.h"
#include "CMPIMsgHandleManager.h"
#include "CMPISCMOUtilities.h"

#include <Pegasus/Common/CIMName.h>
#include <Pegasus/Common/CIMNameCast.h>
#include <Pegasus/Common/CIMPropertyList.h>
#include <Pegasus/Common/SCMO.h>
#if defined (CMPI_VER_85)
#include <Pegasus/Common/MessageLoader.h>
#include <Pegasus/Common/LanguageParser.h>
#endif
#if defined(CMPI_VER_100)
#include <Pegasus/Common/Logger.h>
#include <Pegasus/Common/Tracer.h>
#endif

#include <Pegasus/Provider/CIMOMHandle.h>
#include <Pegasus/WQL/WQLSelectStatement.h>
#include <Pegasus/WQL/WQLParser.h>
#include <Pegasus/Provider/CIMOMHandleQueryContext.h>

#ifdef PEGASUS_ENABLE_CQL
#include <Pegasus/CQL/CQLSelectStatement.h>
#include <Pegasus/CQL/CQLParser.h>
#include <Pegasus/CQL/CQLChainedIdentifier.h>
#endif

#include <stdarg.h>
#include <string.h>

PEGASUS_USING_STD;
PEGASUS_NAMESPACE_BEGIN

#if defined(PEGASUS_OS_TYPE_WINDOWS)
# define lloonngg __int64
#else
# define lloonngg long long int
#endif

// Factory section

static String typeToString(CIMType t)
{
    switch (t)
    {
        case CIMTYPE_BOOLEAN:
            return "boolean";
        case CIMTYPE_UINT8:
            return "uint8";
        case CIMTYPE_SINT8:
            return "sint8";
        case CIMTYPE_UINT16:
            return "uint16";
        case CIMTYPE_SINT16:
            return "sint16";
        case CIMTYPE_UINT32:
            return "uint32";
        case CIMTYPE_SINT32:
            return "sint32";
        case CIMTYPE_UINT64:
            return "sint64";
        case CIMTYPE_SINT64:
            return "sint64";
        case CIMTYPE_REAL32:
            return "real32";
        case CIMTYPE_REAL64:
            return "real64";
        case CIMTYPE_CHAR16:
            return "char16";
        case CIMTYPE_STRING:
            return "string";
        case CIMTYPE_DATETIME:
            return "datetime";
        case CIMTYPE_REFERENCE:
            return "reference";
        case CIMTYPE_OBJECT:
            return "object";
        case CIMTYPE_INSTANCE:
            return "instance";
        default:
            return "???";
    }
}

#define CMPIInstance_str "CMPIInstance"
#define CMPIInstance_str_l 13

#define CMPIObjectPath_str "CMPIObjectPath"
#define CMPIObjectPath_str_l 16

#define CMPIArgs_str "CMPIArgs"
#define CMPIArgs_str_l 8

#define CMPIContext_str "CMPIContext"
#define CMPIContext_str_l 11

#define CMPIResult_str "CMPIResult"
#define CMPIResult_str_l 12

#define CMPIDateTime_str "CMPIDateTime"
#define CMPIDateTime_str_l 12

#define CMPIArray_str "CMPIArray"
#define CMPIArray_str_l 9

#define CMPIString_str "CMPIString"
#define CMPIString_str_l 10

#define CMPISelectExp_str "CMPISelectExp"
#define CMPISelectExp_str_l 13

#define CMPISelectCond_str "CMPISelectCond"
#define CMPISelectCond_str_l 14

#define CMPISubCond_str "CMPISubCond"
#define CMPISubCond_str_l 11

#define CMPIPredicate_str "CMPIPredicate"
#define CMPIPredicate_str_l 13

#define CMPIBroker_str "CMPIBroker"
#define CMPIBroker_str_l 10

#define CMPIEnumeration_str "CMPIEnumeration"
#define CMPIEnumeration_str_l 15


static Formatter::Arg formatValue(va_list *argptr, CMPIStatus *rc, int *err)
{

    CMPIType type = va_arg(*argptr,int);
    CMSetStatus(rc,CMPI_RC_OK);

    if (*err)
    {
        return Formatter::Arg("*failed*");
    }

    switch (type)
    {
        case CMPI_sint8:
        case CMPI_sint16:
        case CMPI_sint32:
            return Formatter::Arg((int)va_arg(*argptr, int));
        case CMPI_uint8:
        case CMPI_uint16:
        case CMPI_uint32:
            return Formatter::Arg((unsigned int)va_arg(*argptr, unsigned int));
        case CMPI_boolean:
            return Formatter::Arg((Boolean)va_arg(*argptr, int));
        case CMPI_real32:
        case CMPI_real64:
            return Formatter::Arg((double)va_arg(*argptr, double));
        case CMPI_sint64:
            return Formatter::Arg((lloonngg)va_arg(*argptr, lloonngg));
        case CMPI_uint64:
            return Formatter::Arg(
                (unsigned lloonngg)va_arg(*argptr, unsigned lloonngg));
        case CMPI_chars:
            return Formatter::Arg((char*)va_arg(*argptr, char*));
        case CMPI_string:
            {
                CMPIString *s = va_arg(*argptr, CMPIString*);
                return Formatter::Arg((char*)CMGetCharsPtr(s,NULL));
            }
        default:
            *err=1;
            if (rc)
            {
                rc->rc=CMPI_RC_ERR_INVALID_PARAMETER;
            }
            return Formatter::Arg("*bad value type*");
    }
}


static inline CIMNamespaceName NameSpaceName(const char *ns)
{
    CIMNamespaceName n;
    if (ns==NULL)
    {
        return n;
    }
    try
    {
        n = CIMNamespaceName(ns);
    }
    catch (...)
    {
        PEG_TRACE_CSTRING(
            TRC_PROVIDERMANAGER,
            Tracer::LEVEL2,
            "Exception: Unknown Exception thrown...");
        // n won't be assigned to anything yet, so it is safe
        // to send it off.
    }
    return n;
}

static inline CIMName Name(const char *n)
{
    CIMName name;
    if (n==NULL)
    {
        return name;
    }
    try
    {
        name = CIMNameCast(n);
    }
    catch ( ...)
    {
        PEG_TRACE_CSTRING(
            TRC_PROVIDERMANAGER,
            Tracer::LEVEL2,
            "Exception: Unknown Exception thrown...");
    }
    return name;
}


extern "C"
{

    static CMPIString* mbEncToString(
        const CMPIBroker*,
        const void *o,
        CMPIStatus *rc);

    static CMPIInstance* mbEncNewInstance(
        const CMPIBroker* mb,
        const CMPIObjectPath* eCop,
        CMPIStatus *rc)
    {
        PEG_METHOD_ENTER(
            TRC_CMPIPROVIDERINTERFACE,
            "CMPI_BrokerEnc:mbEncNewInstance()");
        if (!eCop || !eCop->hdl)
        {
            PEG_TRACE_CSTRING(
                TRC_CMPIPROVIDERINTERFACE,
                Tracer::LEVEL1,
                "Received inv. parameter in CMPI_BrokerEnc:mbEncNewInstance");
            CMSetStatus(rc, CMPI_RC_ERR_INVALID_PARAMETER);
            PEG_METHOD_EXIT();
            return NULL;
        }

        // A CMPIObjectPath is already represented through a SCMOInstance.
        SCMOInstance* scmoOp = (SCMOInstance*)eCop->hdl;
        SCMOInstance* newScmoInst;

        if (scmoOp->isCompromised())
        {
            Uint32 nsL;
            const char* ns = scmoOp->getNameSpace_l(nsL);
            Uint32 cnL;
            const char* cn = scmoOp->getClassName_l(cnL);
            SCMOClass* scmoClass = mbGetSCMOClass(ns,nsL,cn,cnL);

            // If return with no class, create instance but mark with
            // flag indicating no valid class in repository
            if (0 == scmoClass)
            {
                // Create the new instance based on the objectPath.
                newScmoInst = new SCMOInstance(scmoOp->clone());

                newScmoInst->markNoClassForInstance(true);
                CMPIInstance* neInst = reinterpret_cast<CMPIInstance*>(
                new CMPI_Object(newScmoInst, CMPI_Object::ObjectTypeInstance));

                CMSetStatus(rc, CMPI_RC_OK);
                PEG_METHOD_EXIT();
                return neInst;
            }
            // The following is the old code before we changed to
            // allow use of instances that have no class in repository.
            // Get Rid of this when we are satisfied that the new
            // logic covers all issues (See bug 9721)
            // No Class found for this classname. Error
            //if (0 == scmoClass)
            //{
            //    CMSetStatus(rc, CMPI_RC_ERR_NOT_FOUND);
            //    PEG_METHOD_EXIT();
            //    return NULL;
            //}
            //else
            {
                // Create a new clean objectPath from the retrieved instance
                SCMOInstance scmoNewOp(*scmoClass);

                // ... copy the key properties from the dirty one ...
                CMPIrc cmpirc = CMPISCMOUtilities::copySCMOKeyProperties(
                    scmoOp,     // source
                    &scmoNewOp); // target
                if (cmpirc != CMPI_RC_OK)
                {
                    PEG_TRACE_CSTRING(
                        TRC_CMPIPROVIDERINTERFACE,
                        Tracer::LEVEL1,
                        "Failed to copy key bindings");
                    CMSetStatus(rc, CMPI_RC_ERR_FAILED);
                    PEG_METHOD_EXIT();
                    return NULL;
                }

                // ... and finally use the new clean ObjectPath
                newScmoInst = new SCMOInstance(scmoNewOp);
                newScmoInst->markNoClassForInstance(false);
            }
        }
        else   // not compromised
        {
            newScmoInst = new SCMOInstance(scmoOp->clone());
            newScmoInst->markNoClassForInstance(false);
        }
        CMPIInstance* neInst = reinterpret_cast<CMPIInstance*>(
            new CMPI_Object(newScmoInst, CMPI_Object::ObjectTypeInstance));

        CMSetStatus(rc, CMPI_RC_OK);
        PEG_METHOD_EXIT();
        return neInst;
    }

    static CMPIObjectPath* mbEncNewObjectPath(
        const CMPIBroker* mb,
        const char *ns,
        const char *cls,
        CMPIStatus *rc)
    {
        PEG_METHOD_ENTER(
            TRC_CMPIPROVIDERINTERFACE,
            "CMPI_BrokerEnc:mbEncNewObjectPath()");

        SCMOInstance* scmoInst;

        SCMOClass* scmoClass =
            mbGetSCMOClass(
                ns,
                ns ? strlen(ns) : 0,
                cls,
                cls ? strlen(cls) : 0);
        if (0 == scmoClass)
        {
            // Though it is not desirable to let providers create objectPaths
            // for non-existant classes, this has to be allowed for backwards
            // compatibility with previous CMPI implementation :-(
            // So we simply create a 'dirty' SCMOClass object here.
            if (!ns)
            {
                ns="";
            }
            if (!cls)
            {
                cls="";
            }
            SCMOClass localDirtyClass(cls,ns);
            scmoInst = new SCMOInstance(localDirtyClass);
            scmoInst->markAsCompromised();
            scmoInst->markNoClassForInstance(true);

            PEG_TRACE((
                TRC_CMPIPROVIDERINTERFACE,
                Tracer::LEVEL1,
                "Created invalid ObjectPath for non-existant class %s/%s",
                ns,cls));
        }
        else
        {
            scmoInst = new SCMOInstance(*scmoClass);
        }

        CMPIObjectPath *nePath = reinterpret_cast<CMPIObjectPath*>(
            new CMPI_Object(scmoInst, CMPI_Object::ObjectTypeObjectPath));
        CMSetStatus(rc, CMPI_RC_OK);
        PEG_METHOD_EXIT();
        return nePath;
    }

    static CMPIArgs* mbEncNewArgs(const CMPIBroker* mb, CMPIStatus *rc)
    {
        PEG_METHOD_ENTER(
            TRC_CMPIPROVIDERINTERFACE,
            "CMPI_BrokerEnc:mbEncNewArgs()");
        CMSetStatus(rc,CMPI_RC_OK);
        CMPIArgs* cmpiArgs = reinterpret_cast<CMPIArgs*>(
            new CMPI_Object(new Array<CIMParamValue>()));
        PEG_METHOD_EXIT();
        return cmpiArgs;
    }

    static CMPIString* mbEncNewString(
        const CMPIBroker* mb,
        const char *cStr,
        CMPIStatus *rc)
    {
        PEG_METHOD_ENTER(
            TRC_CMPIPROVIDERINTERFACE,
            "CMPI_Broker:mbEncNewString()");
        CMSetStatus(rc,CMPI_RC_OK);
        if (cStr == NULL)
        {
            PEG_TRACE_CSTRING(
                TRC_CMPIPROVIDERINTERFACE,
                Tracer::LEVEL1,
                "Received Invalid Parameter in CMPI_BrokerEnc:mbEncNewString");
            CMSetStatus(rc, CMPI_RC_ERR_INVALID_PARAMETER);
            PEG_METHOD_EXIT();
            return NULL;
        }
        CMPIString* cmpiString =
            reinterpret_cast<CMPIString*>(new CMPI_Object(cStr));
        PEG_METHOD_EXIT();
        return cmpiString;
    }

    CMPIArray* mbEncNewArray(
        const CMPIBroker* mb,
        CMPICount count,
        CMPIType type,
        CMPIStatus *rc)
    {
        PEG_METHOD_ENTER(
            TRC_CMPIPROVIDERINTERFACE,
            "CMPI_BrokerEnc:mbEncNewArray()");
        CMSetStatus(rc,CMPI_RC_OK);
        CMPIData *dta=new CMPIData[count+1];
        dta->type=type;
        dta->value.uint32=count;
        for (unsigned int i=1; i<=count; i++)
        {
            dta[i].type=(type&~CMPI_ARRAY);
            dta[i].state=CMPI_nullValue;
            dta[i].value.uint64=0;
        }
        CMPI_Array *arr = new CMPI_Array(dta);
        CMPIArray* cmpiArray =
            reinterpret_cast<CMPIArray*>(new CMPI_Object(arr));
        PEG_METHOD_EXIT();
        return cmpiArray;
    }

    extern CMPIDateTime *newDateTime();

    static CMPIDateTime* mbEncNewDateTime(const CMPIBroker* mb, CMPIStatus *rc)
    {
        CMSetStatus(rc,CMPI_RC_OK);
        return newDateTime();
    }

    extern CMPIDateTime *newDateTimeBin(CMPIUint64,CMPIBoolean);

    static CMPIDateTime* mbEncNewDateTimeFromBinary(
        const CMPIBroker* mb,
        CMPIUint64 time,
        CMPIBoolean interval,
        CMPIStatus *rc)
    {
        CMSetStatus(rc,CMPI_RC_OK);
        return newDateTimeBin(time, interval);
    }

    extern CMPIDateTime *newDateTimeChar(const char*);

    static CMPIDateTime* mbEncNewDateTimeFromString(
        const CMPIBroker* mb,
        const char *t,
        CMPIStatus *rc)
    {
        PEG_METHOD_ENTER(
            TRC_CMPIPROVIDERINTERFACE,
            "CMPI_BrokerEnc:mbEncNewDateTimeFromString()");
        CMPIDateTime *date = NULL;
        CMSetStatus(rc,CMPI_RC_OK);
        date=newDateTimeChar(t);
        if (!date)
        {
            PEG_TRACE_CSTRING(
                TRC_CMPIPROVIDERINTERFACE,
                Tracer::LEVEL1,
                "Received Invalid Parameter in CMPI_BrokerEnc:newDateTimeChar");
            CMSetStatus(rc, CMPI_RC_ERR_INVALID_PARAMETER);
        }
        PEG_METHOD_EXIT();
        return date;
    }

#if defined(CMPI_VER_200)
    extern CMPIError *newCMPIError(
        const char*,
        const char*,
        const char*,
        const CMPIErrorSeverity,
        const CMPIErrorProbableCause,
        const CMPIrc);

    static CMPIError* mbEncNewCMPIError(
        const CMPIBroker* mb,
        const char* owner,
        const char* msgID,
        const char* msg,
        const CMPIErrorSeverity sev,
        const CMPIErrorProbableCause pc,
        const CMPIrc cimStatusCode, CMPIStatus* rc)
    {
        CMPIError* cmpiError;
        if (rc)
        {
            CMSetStatus(rc, CMPI_RC_OK);
        }
        cmpiError = newCMPIError(owner, msgID, msg, sev, pc, cimStatusCode);
        if (!cmpiError)
        {
            CMSetStatus(rc, CMPI_RC_ERR_INVALID_PARAMETER);
        }
        return cmpiError;
    }
#endif

    static CMPIString* mbEncToString(
        const CMPIBroker*,
        const void *o,
        CMPIStatus *rc)
    {
        PEG_METHOD_ENTER(
            TRC_CMPIPROVIDERINTERFACE,
            "CMPI_BrokerEnc:mbEncToString()");
        CMPI_Object *obj = (CMPI_Object*)o;
        String str;
        char msg[128];

        CMSetStatus(rc,CMPI_RC_OK);
        if (obj==NULL)
        {
            sprintf(msg,"** Null object ptr (%p) **",o);
            CMSetStatus(rc,CMPI_RC_ERR_INVALID_PARAMETER);
            CMPIString* cmpiString =
                reinterpret_cast<CMPIString*>(new CMPI_Object(msg));
            PEG_METHOD_EXIT();
            return cmpiString;
        }

        if (obj->getHdl()==NULL)
        {
            sprintf(msg,"** Null object hdl (%p) **",o);
            CMSetStatus(rc,CMPI_RC_ERR_INVALID_PARAMETER);
            CMPIString* cmpiString =
                reinterpret_cast<CMPIString*>(new CMPI_Object(msg));
            PEG_METHOD_EXIT();
            return cmpiString;
        }

        if (obj->getFtab() == (void*)CMPI_Instance_Ftab ||
            obj->getFtab() == (void*)CMPI_InstanceOnStack_Ftab)
        {
            SCMOInstance *scmoInst=(SCMOInstance*)obj->getHdl();
            CIMInstance ci;
            SCMO_RC src=scmoInst->getCIMInstance(ci);
            if (SCMO_OK==src)
            {
                str="Instance of "+ci.getClassName().getString()+" {\n";
                for (int i=0,m=ci.getPropertyCount(); i<m; i++)
                {
                    CIMConstProperty p = ci.getProperty(i);
                    str.append("  "+typeToString(p.getType())+
                        " "+p.getName().getString()+
                        " = "+p.getValue().toString()+";\n");
                }
                str.append("};\n");
            }
            else
            {
                str.append("Failed to convert instance to string");
            }
        }
        else if (obj->getFtab() == (void*)CMPI_ObjectPath_Ftab ||
            obj->getFtab() == (void*)CMPI_ObjectPathOnStack_Ftab)
        {
            SCMOInstance * scmoObj = (SCMOInstance*)obj->getHdl();
            CIMObjectPath obj;
            scmoObj->getCIMObjectPath(obj);
            str = obj.toString();
        }
        else if (obj->getFtab()==(void*)CMPI_String_Ftab)
        {
            str = String((const char*)obj->getHdl());
        }
        else if (obj->getFtab() == (void*)CMPI_Args_Ftab ||
            obj->getFtab() == (void*)CMPI_ArgsOnStack_Ftab)
        {
            const Array<CIMParamValue>* arg =
                (Array<CIMParamValue>*)obj->getHdl();
            for (int i=0,m=arg->size(); i < m; i++)
            {
                const CIMParamValue &p = (*arg)[i];
                str.append(
                    p.getParameterName()+":"+p.getValue().toString()+"\n");
            }
        }
        else
        {
            sprintf(msg,"** Object not recognized (%p) **",o);
            CMSetStatus(rc,CMPI_RC_ERR_INVALID_PARAMETER);
            CMPIString* cmpiString =
                reinterpret_cast<CMPIString*>(new CMPI_Object(msg));
            PEG_METHOD_EXIT();
            return cmpiString;
        }

        sprintf(msg,"%p: ",o);
        CMPIString* cmpiString =
            reinterpret_cast<CMPIString*>(new CMPI_Object(String(msg)+str));
        PEG_METHOD_EXIT();
        return cmpiString;
    }

    static CMPIBoolean mbEncClassPathIsA(
        const CMPIBroker *mb,
        const CMPIObjectPath *eCp,
        const char *type,
        CMPIStatus *rc)
    {
        PEG_METHOD_ENTER(
            TRC_CMPIPROVIDERINTERFACE,
            "CMPI_BrokerEnc:mbEncClassPathIsA()");

        CMSetStatus(rc,CMPI_RC_OK);
        if ((eCp==NULL) || (type==NULL))
        {
            PEG_TRACE_CSTRING(
                TRC_CMPIPROVIDERINTERFACE,
                Tracer::LEVEL1,
                "Received Invalid Parameter - eCp || type in \
                CMPI_BrokerEnc:mbEncClassPathIsA");
            CMSetStatus(rc, CMPI_RC_ERR_INVALID_PARAMETER);
            PEG_METHOD_EXIT();
            return false;
        }
        if (CIMName::legal(type) == false)
        {
            PEG_TRACE_CSTRING(
                TRC_CMPIPROVIDERINTERFACE,
                Tracer::LEVEL1,
                "Received Invalid Class in \
                CMPI_BrokerEnc:mbEncClassPathIsA");
            CMSetStatus(rc,CMPI_RC_ERR_INVALID_CLASS);
            PEG_METHOD_EXIT();
            return 0;
        }
        SCMOInstance* cop = (SCMOInstance*)eCp->hdl;
        Uint32 nsL;
        const char *ns = cop->getNameSpace_l(nsL);
        Uint32 clsL;
        const char *cls = cop->getClassName_l(clsL);
        Uint32 typeL=strlen(type);

        if (System::strncasecmp(type, typeL, cls, clsL))
        {
            PEG_METHOD_EXIT();
            return 1;
        }

        SCMOClass *cc = mbGetSCMOClass(ns, nsL, cls, clsL);
        if (cc == NULL)
        {
            PEG_METHOD_EXIT();
            return 0;
        }
        cls = cc->getSuperClassName_l(clsL);

        while(NULL!=cls)
        {
            cc = mbGetSCMOClass(ns, nsL, cls, clsL);
            if (cc == NULL)
            {
                PEG_METHOD_EXIT();
                return 0;
            }
            if (System::strncasecmp(cls,clsL,type,typeL))
            {
                PEG_METHOD_EXIT();
                return 1;
            }
            cls = cc->getSuperClassName_l(clsL);
        };
        PEG_METHOD_EXIT();
        return 0;
    }

    static CMPIBoolean mbEncIsOfType(
        const CMPIBroker *mb,
        const void *o,
        const char *type,
        CMPIStatus *rc)
    {
        PEG_METHOD_ENTER(
            TRC_CMPIPROVIDERINTERFACE,
            "CMPI_BrokerEnc:mbEncIsOfType()");
        CMPI_Object *obj = (CMPI_Object*)o;
        char msg[128];
        void *Ftab = NULL;

        if (obj==NULL)
        {
            sprintf(msg,"** Null object ptr (%p) **",o);
            if (rc)
            {
                CMSetStatusWithChars(mb, rc, CMPI_RC_ERR_FAILED, msg);
            }
            PEG_METHOD_EXIT();
            return 0;
        }

        CMSetStatus(rc,CMPI_RC_OK);
        Ftab = obj->getFtab();

        if (((Ftab == (void*)CMPI_Instance_Ftab) ||
            (Ftab == (void*)CMPI_InstanceOnStack_Ftab)) &&
            strncmp(type, CMPIInstance_str, CMPIInstance_str_l) == 0)
        {
            PEG_METHOD_EXIT();
            return 1;
        }
        if (((Ftab == (void*)CMPI_ObjectPath_Ftab) ||
            (Ftab == (void*)CMPI_ObjectPathOnStack_Ftab)) &&
            strncmp(type, CMPIObjectPath_str, CMPIObjectPath_str_l) == 0)
        {
            PEG_METHOD_EXIT();
            return 1;
        }
        if (((Ftab == (void*)CMPI_Args_Ftab) ||
            (Ftab == (void*)CMPI_ArgsOnStack_Ftab)) &&
            strncmp(type, CMPIArgs_str, CMPIArgs_str_l) == 0)
        {
            PEG_METHOD_EXIT();
            return 1;
        }
        if (((Ftab == (void*)CMPI_Context_Ftab) ||
            (Ftab == (void*)CMPI_ContextOnStack_Ftab)) &&
            strncmp(type, CMPIContext_str, CMPIContext_str_l) == 0)
        {
            PEG_METHOD_EXIT();
            return 1;
        }
        if (((Ftab == (void*)CMPI_ResultRefOnStack_Ftab) ||
            (Ftab == (void*)CMPI_ResultInstOnStack_Ftab) ||
            (Ftab == (void*)CMPI_ResultData_Ftab) ||
            (Ftab == (void*)CMPI_ResultMethOnStack_Ftab) ||
            (Ftab == (void*)CMPI_ResultResponseOnStack_Ftab) ||
            (Ftab == (void*)CMPI_ResultExecQueryOnStack_Ftab)) &&
            strncmp(type, CMPIResult_str, CMPIResult_str_l) == 0)
        {
            PEG_METHOD_EXIT();
            return 1;
        }
        if (Ftab == (void*)CMPI_DateTime_Ftab &&
            strncmp(type, CMPIDateTime_str, CMPIDateTime_str_l) == 0)
        {
            PEG_METHOD_EXIT();
            return 1;
        }
        if (Ftab == (void*)CMPI_Array_Ftab &&
            strncmp(type, CMPIArray_str, CMPIArray_str_l) == 0)
        {
            PEG_METHOD_EXIT();
            return 1;
        }
        if (Ftab == (void*)CMPI_String_Ftab &&
            strncmp(type, CMPIString_str, CMPIString_str_l) == 0)
        {
            PEG_METHOD_EXIT();
            return 1;
        }
        if (Ftab == (void*)CMPI_SelectExp_Ftab &&
            strncmp(type, CMPISelectExp_str, CMPISelectExp_str_l) == 0)
        {
            PEG_METHOD_EXIT();
            return 1;
        }
        if (Ftab == (void*)CMPI_SelectCond_Ftab &&
            strncmp(type, CMPISelectCond_str, CMPISelectCond_str_l) == 0)
        {
            PEG_METHOD_EXIT();
            return 1;
        }
        if (Ftab == (void*)CMPI_SubCond_Ftab &&
            strncmp(type, CMPISubCond_str, CMPISubCond_str_l) == 0)
        {
            PEG_METHOD_EXIT();
            return 1;
        }
        if (Ftab == (void*)CMPI_Predicate_Ftab &&
            strncmp(type, CMPIPredicate_str, CMPIPredicate_str_l) == 0)
        {
            PEG_METHOD_EXIT();
            return 1;
        }
        if (Ftab == (void*)CMPI_Broker_Ftab &&
            strncmp(type, CMPIBroker_str, CMPIBroker_str_l) == 0)
        {
            PEG_METHOD_EXIT();
            return 1;
        }
        if (((Ftab == (void*)CMPI_ObjEnumeration_Ftab) ||
            (Ftab == (void*)CMPI_InstEnumeration_Ftab) ||
            (Ftab == (void*)CMPI_OpEnumeration_Ftab)) &&
            strncmp(type, CMPIEnumeration_str, CMPIEnumeration_str_l) == 0)
        {
            PEG_METHOD_EXIT();
            return 1;
        }

        sprintf(msg, "** Object not recognized (%p) **", o);
        if (rc)
        {
            CMSetStatusWithChars(mb, rc, CMPI_RC_ERR_FAILED, msg);
        }
        PEG_METHOD_EXIT();
        return 0;
    }

    static CMPIString* mbEncGetType(
        const CMPIBroker *mb,
        const  void* o,
        CMPIStatus *rc)
    {
        PEG_METHOD_ENTER(
            TRC_CMPIPROVIDERINTERFACE,
            "CMPI_BrokerEnc:mbEncGetType()");
        CMPI_Object *obj = (CMPI_Object*)o;
        char msg[128];
        void *Ftab= NULL;

        if (obj==NULL)
        {
            sprintf(msg, "** Null object ptr (%p) **", o);
            if (rc)
            {
                CMSetStatusWithChars(mb, rc, CMPI_RC_ERR_FAILED, msg);
            }
            PEG_METHOD_EXIT();
            return 0;
        }

        CMSetStatus(rc, CMPI_RC_OK);
        Ftab = obj->getFtab();

        if ((Ftab == (void*)CMPI_Instance_Ftab) ||
            (Ftab == (void*)CMPI_InstanceOnStack_Ftab))
        {
            PEG_METHOD_EXIT();
            return mb->eft->newString(mb, CMPIInstance_str, rc);
        }
        if ((Ftab == (void*)CMPI_ObjectPath_Ftab) ||
            (Ftab == (void*)CMPI_ObjectPathOnStack_Ftab))
        {
            PEG_METHOD_EXIT();
            return mb->eft->newString(mb, CMPIObjectPath_str, rc);
        }
        if ((Ftab == (void*)CMPI_Args_Ftab) ||
            (Ftab == (void*)CMPI_ArgsOnStack_Ftab))
        {
            PEG_METHOD_EXIT();
            return mb->eft->newString(mb, CMPIArgs_str, rc);
        }
        if ((Ftab == (void*)CMPI_Context_Ftab) ||
            (Ftab == (void*)CMPI_ContextOnStack_Ftab))
        {
            PEG_METHOD_EXIT();
            return mb->eft->newString(mb, CMPIContext_str, rc);
        }
        if ((Ftab == (void*)CMPI_ResultRefOnStack_Ftab) ||
            (Ftab == (void*)CMPI_ResultInstOnStack_Ftab) ||
            (Ftab == (void*)CMPI_ResultData_Ftab) ||
            (Ftab == (void*)CMPI_ResultMethOnStack_Ftab) ||
            (Ftab == (void*)CMPI_ResultResponseOnStack_Ftab) ||
            (Ftab == (void*)CMPI_ResultExecQueryOnStack_Ftab))
        {
            PEG_METHOD_EXIT();
            return mb->eft->newString(mb, CMPIResult_str, rc);
        }
        if (Ftab == (void*)CMPI_DateTime_Ftab)
        {
            PEG_METHOD_EXIT();
            return mb->eft->newString(mb, CMPIDateTime_str, rc);
        }
        if (Ftab == (void*)CMPI_Array_Ftab)
        {
            PEG_METHOD_EXIT();
            return mb->eft->newString(mb, CMPIArray_str, rc);
        }
        if (Ftab == (void*)CMPI_String_Ftab)
        {
            PEG_METHOD_EXIT();
            return mb->eft->newString(mb, CMPIString_str, rc);
        }
        if (Ftab == (void*)CMPI_SelectExp_Ftab)
        {
            PEG_METHOD_EXIT();
            return mb->eft->newString(mb, CMPISelectExp_str, rc);
        }
        if (Ftab == (void*)CMPI_SelectCond_Ftab)
        {
            PEG_METHOD_EXIT();
            return mb->eft->newString(mb, CMPISelectCond_str, rc);
        }
        if (Ftab == (void*)CMPI_SubCond_Ftab)
        {
            PEG_METHOD_EXIT();
            return mb->eft->newString(mb, CMPISubCond_str, rc);
        }
        if (Ftab == (void*)CMPI_Predicate_Ftab)
        {
            PEG_METHOD_EXIT();
            return mb->eft->newString(mb, CMPIPredicate_str, rc);
        }
        if (Ftab == (void*)CMPI_Broker_Ftab)
        {
            PEG_METHOD_EXIT();
            return mb->eft->newString(mb, CMPIBroker_str, rc);
        }
        if ((Ftab == (void*)CMPI_ObjEnumeration_Ftab) ||
            (Ftab == (void*)CMPI_InstEnumeration_Ftab) ||
            (Ftab == (void*)CMPI_OpEnumeration_Ftab))
        {
            PEG_METHOD_EXIT();
            return mb->eft->newString(mb, CMPIEnumeration_str, rc);
        }
        sprintf(msg, "** Object not recognized (%p) **", o);
        if (rc)
        {
            CMSetStatusWithChars(mb, rc, CMPI_RC_ERR_FAILED, msg);
        }
        PEG_METHOD_EXIT();
        return 0;
    }

#if defined (CMPI_VER_85)

    static CMPIString* mbEncGetMessage(
        const CMPIBroker *mb,
        const char *msgId,
        const char *defMsg,
        CMPIStatus* rc,
        CMPICount count,
        ...)
    {
        PEG_METHOD_ENTER(
            TRC_CMPIPROVIDERINTERFACE,
            "CMPI_BrokerEnc:mbEncGetMessage()");
        MessageLoaderParms parms(msgId, defMsg);
        int err = 0;
        if (rc)
        {
            rc->rc = CMPI_RC_OK;
        }
        if (count > 0)
        {
            va_list argptr;
            va_start(argptr, count);
            for (;;)
            {
                if (count > 0)
                {
                    parms.arg0 = formatValue(&argptr, rc, &err);
                }
                else
                {
                    break;
                }
                if (count > 1)
                {
                    parms.arg1 = formatValue(&argptr, rc, &err);
                }
                else
                {
                    break;
                }
                if (count > 2)
                {
                    parms.arg2 = formatValue(&argptr, rc, &err);
                }
                else
                {
                    break;
                }
                if (count > 3)
                {
                    parms.arg3 = formatValue(&argptr, rc, &err);
                }
                else
                {
                    break;
                }
                if (count > 4)
                {
                    parms.arg4 = formatValue(&argptr, rc, &err);
                }
                else
                {
                    break;
                }
                if (count > 5)
                {
                    parms.arg5 = formatValue(&argptr, rc, &err);
                }
                else
                {
                    break;
                }
                if (count > 6)
                {
                    parms.arg6 = formatValue(&argptr, rc, &err);
                }
                else
                {
                    break;
                }
                if (count > 7)
                {
                    parms.arg7 = formatValue(&argptr, rc, &err);
                }
                else
                {
                    break;
                }
                if (count > 8)
                {
                    parms.arg8 = formatValue(&argptr, rc, &err);
                }
                else
                {
                    break;
                }
                if (count > 9)
                {
                    parms.arg9 = formatValue(&argptr, rc, &err);
                }
                break;
            }
            va_end(argptr);
        }
        String nMsg = MessageLoader::getMessage(parms);
        PEG_METHOD_EXIT();
        return string2CMPIString(nMsg);
    }
#endif

#ifdef CMPI_VER_200
    static CMPIStatus mbEncOpenMessageFile(
        const CMPIBroker* mb,
        const char* msgFile,
        CMPIMsgFileHandle* msgFileHandle)
    {
        PEG_METHOD_ENTER(
            TRC_CMPIPROVIDERINTERFACE,
            "CMPI_BrokerEnc:mbEncOpenMessageFile()");
        CMPIStatus rc = { CMPI_RC_OK, NULL };
        AutoPtr<MessageLoaderParms> parms(new MessageLoaderParms());
        parms->msg_src_path = msgFile;

        // Initialize the msgFileHandle to NULL, so it is easier to
        // detect an uninitialized handle in related functions.
        *msgFileHandle = NULL;

        // Get the AcceptLanguage entry
        const CMPIContext *ctx = CMPI_ThreadContext::getContext();
        CMPIData data = ctx->ft->getEntry(ctx, CMPIAcceptLanguage, &rc);
        if (rc.rc != CMPI_RC_ERR_NO_SUCH_PROPERTY)
        {
            if (rc.rc == CMPI_RC_OK)
            {
                const char* accLangs = CMGetCharsPtr(data.value.string, NULL);
                if ((accLangs != NULL) && (accLangs[0] != '\0'))
                {
                    parms->acceptlanguages =
                        LanguageParser::parseAcceptLanguageHeader(accLangs);
                }
            }
            else
            {
                PEG_TRACE(
                    (TRC_PROVIDERMANAGER,
                     Tracer::LEVEL1,
                     "Failed to get CMPIAcceptLanguage from CMPIContext. RC=%d",
                     rc.rc));
                PEG_METHOD_EXIT();
                return rc; // should be CMPI_RC_ERR_INVALID_HANDLE
            }
        }

        MessageLoader::openMessageFile(*parms.get());

        ContentLanguageList cll = parms->contentlanguages;
        // Check that we have at least one content language (ie. matching
        // resource bundle was found) before adding to Invocation Context.
        if (cll.size() > 0)
        {
            ctx->ft->addEntry(
                ctx,
                CMPIContentLanguage,
                (CMPIValue*)(const char*)
                    LanguageParser::
                        buildContentLanguageHeader(cll).getCString(),
                CMPI_chars);
        }

        CMPIMsgHandleManager* handleMgr =
            CMPIMsgHandleManager::getCMPIMsgHandleManager();
        *msgFileHandle = handleMgr->getNewHandle(parms.release());

        PEG_METHOD_EXIT();
        CMReturn(CMPI_RC_OK);
    }

    static CMPIStatus mbEncCloseMessageFile(
        const CMPIBroker *mb,
        const CMPIMsgFileHandle msgFileHandle)
    {
        PEG_METHOD_ENTER(
            TRC_CMPIPROVIDERINTERFACE,
            "CMPI_BrokerEnc:mbEncCloseMessageFile()");

        CMPIMsgHandleManager* handleMgr =
            CMPIMsgHandleManager::getCMPIMsgHandleManager();
        MessageLoaderParms* parms;

        try
        {
            parms = handleMgr->releaseHandle(msgFileHandle);
        }
        catch( IndexOutOfBoundsException&)
        {
            PEG_METHOD_EXIT();
            CMReturn(CMPI_RC_ERR_INVALID_HANDLE);
        }

        MessageLoader::closeMessageFile(*parms);

        delete parms;

        PEG_METHOD_EXIT();
        CMReturn(CMPI_RC_OK);
    }

    static CMPIString* mbEncGetMessage2(
        const CMPIBroker *mb,
        const char *msgId,
        const CMPIMsgFileHandle msgFileHandle,
        const char *defMsg,
        CMPIStatus* rc,
        CMPICount count,
        ...)
    {
        PEG_METHOD_ENTER(
            TRC_CMPIPROVIDERINTERFACE,
            "CMPI_BrokerEnc:mbEncGetMessage2()");

        CMPIMsgHandleManager* handleMgr =
            CMPIMsgHandleManager::getCMPIMsgHandleManager();
        MessageLoaderParms* parms;

        try
        {
            parms = handleMgr->getDataForHandle(msgFileHandle);
        }
        catch( IndexOutOfBoundsException&)
        {
            if (rc)
            {
                rc->rc=CMPI_RC_ERR_INVALID_HANDLE;
            }
            PEG_METHOD_EXIT();
            return NULL;
        }


        if (msgId != NULL)
        {
            parms->msg_id = msgId;
        }

        if ( defMsg != NULL )
        {
            parms->default_msg.assign(defMsg);
        }

        int err = 0;
        if (rc)
        {
            rc->rc=CMPI_RC_OK;
        }

        if (count > 0)
        {
            va_list argptr;
            va_start(argptr, count);
            for (;;)
            {
                if (count > 0)
                {
                     parms->arg0 = formatValue(&argptr, rc, &err);
                }
                else
                {
                    break;
                }
                if (count > 1)
                {
                    parms->arg1 = formatValue(&argptr, rc, &err);
                }
                else
                {
                    break;
                }
                if (count > 2)
                {
                    parms->arg2 = formatValue(&argptr, rc, &err);
                }
                else
                {
                    break;
                }
                if (count > 3)
                {
                    parms->arg3 = formatValue(&argptr, rc, &err);
                }
                else
                {
                    break;
                }
                if (count > 4)
                {
                    parms->arg4 = formatValue(&argptr, rc, &err);
                }
                else
                {
                    break;
                }
                if (count > 5)
                {
                    parms->arg5 = formatValue(&argptr, rc, &err);
                }
                else
                {
                    break;
                }
                if (count > 6)
                {
                    parms->arg6 = formatValue(&argptr, rc, &err);
                }
                else
                {
                    break;
                }
                if (count > 7)
                {
                    parms->arg7 = formatValue(&argptr, rc, &err);
                }
                else
                {
                    break;
                }
                if (count > 8)
                {
                    parms->arg8 = formatValue(&argptr, rc, &err);
                }
                else
                {
                    break;
                }
                if (count > 9)
                {
                    parms->arg9 = formatValue(&argptr, rc, &err);
                }
                break;
            }
            va_end(argptr);
        }
        String nMsg = MessageLoader::getMessage2(*parms);
        PEG_METHOD_EXIT();
        return string2CMPIString(nMsg);
    }
#endif

#if defined(CMPI_VER_100)
    CMPIStatus mbEncLogMessage(
        const CMPIBroker*,
        CMPISeverity severity,
        const char *id,
        const char *text,
        const CMPIString *string)
    {
        if (!(text || string))
        {
            CMReturn(CMPI_RC_ERR_INVALID_PARAMETER);
        }

        String logString;
        if (id != NULL)
        {
            logString.assign(id);
        logString.append(":");
        }

        if (string)
        {
            logString.append(( char*)CMGetCharsPtr(string,NULL));
        }
        else
        {
            logString.append(text);
        }
        /*
            CMPI Severity Codes from Section 4.8
            CMPI_SEV_ERROR = 1,
            CMPI_SEV_INFO = 2,
            CMPI_SEV_WARNING = 3,
            CMPI_DEV_DEBUG = 4
            Here we try to match CMPI Severity codes to the Pegauss Log levels.
        */

        Uint32 logSeverity;
        Logger::LogFileType logFileType;

        switch (severity)
        {
            case CMPI_DEV_DEBUG:
                logSeverity = Logger::TRACE;
                logFileType = Logger::STANDARD_LOG;
                break;
            case  CMPI_SEV_INFO:
                logSeverity = Logger::INFORMATION;
                logFileType = Logger::STANDARD_LOG;
                break;
            case CMPI_SEV_WARNING:
                logSeverity = Logger::WARNING;
                logFileType = Logger::STANDARD_LOG;
                break;
            case CMPI_SEV_ERROR:
                logSeverity = Logger::SEVERE;
                logFileType = Logger::ERROR_LOG;
                break;
            default:
                logSeverity = Logger::INFORMATION;
                logFileType = Logger::STANDARD_LOG;
        }

        Logger::put(
            logFileType,
            System::CIMSERVER,
            logSeverity,
            logString);
        CMReturn ( CMPI_RC_OK);
    }

    inline Uint32 mapTraceLevel(CMPILevel level)
    {
        // There is no notion in CMPI spec about what 'level' means.
        // So we are going to try to map.
        switch (level)
        {
            case CMPI_LEV_INFO:
                return Tracer::LEVEL3;
                break;
            case CMPI_LEV_WARNING:
                return Tracer::LEVEL2;
                break;
            case CMPI_LEV_VERBOSE:
                return Tracer::LEVEL4;
                break;
            default:
                return Tracer::LEVEL4;
        }
    }

    inline const char* decideTraceString(
        const char *text,
        const CMPIString *string)
    {
        if (text)
        {
            return text;
        }
        return (const char*) CMGetCharsPtr(string, NULL);
    }

    CMPIStatus mbEncTracer(
        const CMPIBroker*,
        CMPILevel level,
        const char *component,
        const char *text,
        const CMPIString *string)
    {
        if (!component || !(text || string))
        {
            CMReturn(CMPI_RC_ERR_INVALID_PARAMETER);
        }
        // emptiness of component checked at begin of mbEncTracer
        // don't have to expect a null pointer here
        // through the first check in mbEncTracer
        // we know that return from decideTraceString is not null
        PEG_TRACE(
            (TRC_CMPIPROVIDER,
                mapTraceLevel(level),
                "%s: %s",
                component,
                decideTraceString(text, string)));

        CMReturn ( CMPI_RC_OK);
    }
#endif

    static CMPISelectExp *mbEncNewSelectExp(
        const CMPIBroker *mb,
        const char *query,
        const char *lang,
        CMPIArray ** projection,
        CMPIStatus * st)
    {
        PEG_METHOD_ENTER(
            TRC_CMPIPROVIDERINTERFACE,
            "CMPI_BrokerEnc:mbEncNewSelectExp()");
        int exception = 1;
        CMPIStatus rc = { CMPI_RC_OK, NULL };

        if (strncmp (lang, CALL_SIGN_WQL, CALL_SIGN_WQL_SIZE) == 0)
        {
            // Get the namespace.
            const CMPIContext *ctx = CMPI_ThreadContext::getContext ();

            CMPIData data = ctx->ft->getEntry (ctx, CMPIInitNameSpace, &rc);
            if (rc.rc != CMPI_RC_OK)
            {
                CMSetStatus (st, CMPI_RC_ERR_FAILED);
                PEG_METHOD_EXIT();
                return NULL;
            }

            // Create the CIMOMHandle wrapper.
            CIMOMHandle *cm_handle = (CIMOMHandle*)mb->hdl;
            CIMOMHandleQueryContext qcontext(
                CIMNamespaceName(CMGetCharsPtr(data.value.string,NULL)),
                *cm_handle);

            String sLang (lang);
            String sQuery (query);

            WQLSelectStatement *stmt = new WQLSelectStatement (sLang,
                                                               sQuery,
                                                               qcontext);
            try
            {
                WQLParser::parse(query, *stmt);
                exception = 0;
            }
            catch (const ParseError &)
            {
                if (st)
                {
                    CMSetStatus (st, CMPI_RC_ERR_INVALID_QUERY);
                }
            }
            catch (const MissingNullTerminator &)
            {
                if (st)
                {
                    CMSetStatus (st, CMPI_RC_ERR_INVALID_QUERY);
                }
            }

            if (exception)
            {
                delete stmt;
                if (projection)
                {
                    *projection = NULL;
                }
                PEG_METHOD_EXIT();
                return NULL;
            }
            if (projection)
            {
                if (stmt->getAllProperties ())
                {
                    *projection = NULL;
                }
                else
                {
                    *projection =
                        mbEncNewArray(
                            mb,
                            stmt->getSelectPropertyNameCount(),
                            CMPI_string,
                            NULL);
                    for (int i = 0, m = stmt->getSelectPropertyNameCount();
                        i < m; i++)
                    {
                        const CIMName &n = stmt->getSelectPropertyName(i);
                        /* cerr << "Property: " << n.getString() << endl;
                           Since the array and the CIMName disappear when this
                           function exits we use CMPI data storage - the
                           CMPI_Object keeps a list of data and cleans it up
                           when the provider API function is exited.
                        */
                        CMPIString *str_data =
                            reinterpret_cast<CMPIString *>(
                                new CMPI_Object(n.getString()));
                        CMPIValue value;
                        value.string = str_data;

                        rc = CMSetArrayElementAt(
                            *projection,
                            i,
                            &value,
                            CMPI_string);
                        if (rc.rc != CMPI_RC_OK)
                        {
                            if (st)
                            {
                                CMSetStatus (st, rc.rc);
                            }
                            PEG_METHOD_EXIT();
                            return NULL;
                        }
                    }
                }
            }
            stmt->hasWhereClause ();
            if (st)
            {
                CMSetStatus (st, CMPI_RC_OK);
            }
            CMPISelectExp* cmpiSelectExp = new CMPI_SelectExp(stmt);

            PEG_METHOD_EXIT();
            return (cmpiSelectExp);
        }
#ifdef PEGASUS_ENABLE_CQL
        if ((strncmp (lang, CALL_SIGN_CQL, CALL_SIGN_CQL_SIZE) == 0) ||
            (strncmp (lang, "CIMxCQL", 7) == 0) ||
            (strncmp (lang, "CIM:CQL", 7) == 0))
        {
            int useShortNames = 0;
            /* IBMKR: This will have to be removed when the CMPI spec is
               updated with a clear explanation of what properties array
               can have as strings. For right now, if useShortNames is set
               to true, _only_ the last chained identifier is used.
            */
            if (strncmp (lang, CALL_SIGN_CQL, CALL_SIGN_CQL_SIZE) == 0)
            {
                useShortNames = 1;
            }
            // Get the namespace.
            const CMPIContext *ctx = CMPI_ThreadContext::getContext ();

            CMPIData data = ctx->ft->getEntry (ctx, CMPIInitNameSpace, &rc);
            if (rc.rc != CMPI_RC_OK)
            {
                if (st)
                {
                    CMSetStatus (st, CMPI_RC_ERR_FAILED);
                }
                PEG_METHOD_EXIT();
                return NULL;
            }

            // Create the CIMOMHandle wrapper.
            CIMOMHandle *cm_handle = (CIMOMHandle*)mb->hdl;
            CIMOMHandleQueryContext qcontext(
                CIMNamespaceName(CMGetCharsPtr(data.value.string, NULL)),
                *cm_handle);

            String sLang (lang);
            String sQuery (query);

            CQLSelectStatement *selectStatement =
                new CQLSelectStatement(sLang, sQuery, qcontext);
            try
            {
                CQLParser::parse (query, *selectStatement);
                selectStatement->validate ();
                exception = 0;
            }
            catch (...)
            {
                PEG_TRACE_CSTRING(
                    TRC_CMPIPROVIDERINTERFACE,
                    Tracer::LEVEL1,
                    "Exception: Unknown Exception received...");
                if (st)
                {
                    PEG_TRACE_CSTRING(
                        TRC_CMPIPROVIDERINTERFACE,
                        Tracer::LEVEL1,
                        "Exception: Invalid Query Exception received...");
                    CMSetStatus (st, CMPI_RC_ERR_INVALID_QUERY);
                }
            }

            if (exception)
            {
                delete selectStatement;
                if (projection)
                {
                    *projection = NULL;
                }
                PEG_METHOD_EXIT();
                return NULL;
            }
            else
            {
                if (projection)
                {
                    Array <CQLChainedIdentifier> select_Array =
                        selectStatement->getSelectChainedIdentifiers();

                    // Special check. Remove it when useShortNames is not
                    // neccessary
                    if ((select_Array.size() == 1) &&
                        (useShortNames) &&
                        (select_Array[0].getLastIdentifier().getName() \
                         .getString() == String::EMPTY))
                    {
                        *projection= NULL;

                    }
                    else
                    {
                        *projection =
                            mbEncNewArray(
                                mb,
                                select_Array.size(),
                                CMPI_string,
                                NULL);

                        CQLIdentifier identifier;
                        String name;

                        for (Uint32 i = 0; i < select_Array.size (); i++)
                        {
                            if (useShortNames)
                            {
                                identifier =
                                    select_Array[i].getLastIdentifier();
                                name = identifier.getName ().getString ();
                            }
                            else
                            {
                                name = select_Array[i].toString ();
                            }
                            /* Since the array and the CIMName disappear when
                               this function exits we use CMPI data storage -
                               the CMPI_Object keeps a list of data and cleans
                               it up when the provider API function is exited.
                               cerr << "Property: " << name << endl;
                            */
                            CMPIString *str_data =
                                reinterpret_cast<CMPIString *>(
                                    new CMPI_Object (name));
                            CMPIValue value;
                            value.string = str_data;

                            rc = CMSetArrayElementAt(
                                *projection,
                                i,
                                &value,
                                CMPI_string);

                            if (rc.rc != CMPI_RC_OK)
                            {
                                if (st)
                                {
                                    CMSetStatus (st, rc.rc);
                                }
                                PEG_METHOD_EXIT();
                                return NULL;
                            }
                        }
                    }
                }
            }
            if (st)
            {
                CMSetStatus (st, CMPI_RC_OK);
            }
            CMPI_SelectExp* cmpiSelectExp = new CMPI_SelectExp(
                selectStatement, false, (QueryContext*) &qcontext);
            PEG_METHOD_EXIT();
            return (cmpiSelectExp);
        }
#endif
        if (st)
        {
            PEG_TRACE_CSTRING(
                TRC_CMPIPROVIDERINTERFACE,
                Tracer::LEVEL1,
                "Received Query Language is currently NOT SUPPORTED");
            CMSetStatus (st, CMPI_RC_ERR_QUERY_LANGUAGE_NOT_SUPPORTED);
        }
        return NULL;
    }

#if defined (CMPI_VER_90) && !defined(CMPI_VER_100)

    static CMPIArray *mbEncGetKeyList(
        CMPIBroker *mb,
        CMPIContext *ctx,
        CMPIObjectPath *cop,
        CMPIStatus *rc)
    {
        PEG_METHOD_ENTER(
            TRC_CMPIPROVIDERINTERFACE,
            "CMPI_BrokerEnc:mbEncGetKeyList()");
        if ((cop==NULL) || (ctx==NULL))
        {
            PEG_TRACE_CSTRING(
                TRC_CMPIPROVIDERINTERFACE,
                Tracer::LEVEL1,
                "Invalid Parameter cop || ctx in \
                CMPI_BrokerEnc:mbEncGetKeyList");
            CMSetStatus(rc, CMPI_RC_ERR_INVALID_PARAMETER);
            PEG_METHOD_EXIT();
            return NULL;
        }
        SCMOInstance *op = (SCMOInstance*)cop->hdl;

        if (!op)
        {
            PEG_TRACE_CSTRING(
                TRC_CMPIPROVIDERINTERFACE,
                Tracer::LEVEL1,
                "Received Invalid Handle cop->handle in \
                CMPI_BrokerEnc:mbEncGetKeyList");
            CMSetStatus(rc, CMPI_RC_ERR_INVALID_HANDLE);
            PEG_METHOD_EXIT();
            return NULL;
        }
        Uint32 nsL;
        const char* ns = op->getNamespace(nsL);
        Uint32 cnL;
        const char* cn = op->getClassName_L(cnL);
        SCMOClass *cls = mbGetSCMOClass(ns,nsL,cn,cnL);
        if (0==cls)
        {
            PEG_TRACE_CSTRING(
                TRC_CMPIPROVIDERINTERFACE,
                Tracer::LEVEL1,
                "Invalid Parameter cop (class not found) in \
                CMPI_BrokerEnc:mbEncGetKeyList");
            CMSetStatus(rc, CMPI_RC_ERR_INVALID_PARAMETER);
            PEG_METHOD_EXIT();
            return NULL;
        }

        //TODO: Eventually implement getKeyNamesAsString using native C-strings
        Array<String> keys;
        cls->getKeyNamesAsString(keys);
        CMPIArray *ar = mb->eft->newArray(mb, keys.size(), CMPI_string, NULL);
        for (Uint32 i=0,m=keys.size(); i<m; i++)
        {
            String s = keys[i];
            CMPIString *str = string2CMPIString(s);
            ar->ft->setElementAt(ar, i, (CMPIValue*)&str, CMPI_string);
        }
        CMSetStatus(rc, CMPI_RC_OK);
        PEG_METHOD_EXIT();
        return ar;
    }


#endif

}

static CMPIBrokerEncFT brokerEnc_FT =
{
    CMPICurrentVersion,
    mbEncNewInstance,
    mbEncNewObjectPath,
    mbEncNewArgs,
    mbEncNewString,
    mbEncNewArray,
    mbEncNewDateTime,
    mbEncNewDateTimeFromBinary,
    mbEncNewDateTimeFromString,
    mbEncNewSelectExp,
    mbEncClassPathIsA,
    mbEncToString,
    mbEncIsOfType,
    mbEncGetType,
#if defined (CMPI_VER_85)
    mbEncGetMessage,
#endif
#if defined (CMPI_VER_90) && !defined(CMPI_VER_100)
    mbEncGetKeyList,
#endif
#if defined (CMPI_VER_100)
    mbEncLogMessage,
    mbEncTracer,
#endif
#if defined (CMPI_VER_200)
    mbEncNewCMPIError,
    mbEncOpenMessageFile,
    mbEncCloseMessageFile,
    mbEncGetMessage2,
#endif
};

CMPIBrokerEncFT *CMPI_BrokerEnc_Ftab = &brokerEnc_FT;


PEGASUS_NAMESPACE_END



