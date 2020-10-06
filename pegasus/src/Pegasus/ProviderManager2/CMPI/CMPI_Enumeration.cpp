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

#include "CMPI_Enumeration.h"
#include "CMPI_Object.h"
#include "CMPI_Ftabs.h"
#include <Pegasus/Common/Tracer.h>

PEGASUS_USING_STD;
PEGASUS_NAMESPACE_BEGIN

extern "C"
{

    static CMPIStatus enumRelease(CMPIEnumeration* eObj)
    {
        if ((void*)eObj->ft == (void*)CMPI_InstEnumeration_Ftab)
        {
            CMPI_InstEnumeration* ie = (CMPI_InstEnumeration*)eObj->hdl;
            if (ie)
            {
                Array<SCMOInstance>* enm = (Array<SCMOInstance>*)ie->hdl;
                if (enm)
                {
                    delete enm;
                }
                delete ie;
            }
            (reinterpret_cast<CMPI_Object*>(eObj))->unlinkAndDelete();
        }
        else if ((void*)eObj->ft == (void*)CMPI_ObjEnumeration_Ftab)
        {
            CMPI_ObjEnumeration* ie = (CMPI_ObjEnumeration*)eObj->hdl;
            if (ie)
            {
                Array<SCMOInstance>* enm = (Array<SCMOInstance>*)ie->hdl;
                if (enm)
                {
                    delete enm;
                }
                delete ie;
            }
            (reinterpret_cast<CMPI_Object*>(eObj))->unlinkAndDelete();
        }
        else if ((void*)eObj->ft == (void*)CMPI_OpEnumeration_Ftab)
        {
            CMPI_OpEnumeration* ie = (CMPI_OpEnumeration*)eObj->hdl;
            if (ie)
            {
                Array<SCMOInstance>* enm = (Array<SCMOInstance>*)ie->hdl;
                if (enm)
                {
                    delete enm;
                }
                delete ie;
            }
            (reinterpret_cast<CMPI_Object*>(eObj))->unlinkAndDelete();
        }
        CMReturn(CMPI_RC_OK);
    }

    static CMPIEnumeration* enumClone(
        const CMPIEnumeration* eEnumObj,
        CMPIStatus* rc)
    {
        PEG_METHOD_ENTER(
            TRC_CMPIPROVIDERINTERFACE,
            "CMPI_Enumeration:enumClone()");
        const CMPIEnumeration* eEnum = (CMPIEnumeration*)eEnumObj->hdl;

        CMSetStatus(rc, CMPI_RC_OK);
        if (eEnum->hdl)
        {
            if ((void*)eEnum->ft == (void*)CMPI_InstEnumeration_Ftab)
            {
                Array<SCMOInstance>* enm = (Array<SCMOInstance>*)eEnum->hdl;
                CMPI_Object *obj = new CMPI_Object(
                    new CMPI_InstEnumeration(new Array<SCMOInstance>(*enm)));
                obj->unlink(); // remove from current thread context.
                CMPIEnumeration* cmpiEnum =
                    reinterpret_cast<CMPIEnumeration*>(obj);
                PEG_METHOD_EXIT();
                return cmpiEnum;
            }
            else if ((void*)eEnum->ft == (void*)CMPI_ObjEnumeration_Ftab)
            {
                Array<SCMOInstance>* enm = (Array<SCMOInstance>*)eEnum->hdl;
                CMPI_Object *obj = new CMPI_Object(
                    new CMPI_ObjEnumeration(new Array<SCMOInstance>(*enm)));
                obj->unlink(); // remove from current thread context.
                CMPIEnumeration* cmpiEnum =
                    reinterpret_cast<CMPIEnumeration*>(obj);
                PEG_METHOD_EXIT();
                return cmpiEnum;
            }
            else if ((void*)eEnum->ft == (void*)CMPI_OpEnumeration_Ftab)
            {
                Array<SCMOInstance>* enm = (Array<SCMOInstance>*)eEnum->hdl;
                CMPI_Object *obj = new CMPI_Object(
                    new CMPI_OpEnumeration(new Array<SCMOInstance>(*enm)));
                obj->unlink(); // remove from current thread context.
                CMPIEnumeration* cmpiEnum =
                    reinterpret_cast<CMPIEnumeration*>(obj);
                PEG_METHOD_EXIT();
                return cmpiEnum;
            }
        }
        PEG_TRACE_CSTRING(
            TRC_CMPIPROVIDERINTERFACE,
            Tracer::LEVEL1,
            "Received invalid Handle - eEnum->hdl...");
        CMSetStatus(rc, CMPI_RC_ERR_INVALID_HANDLE);
        PEG_METHOD_EXIT();
        return NULL;
    }

    static CMPIData enumGetNext(
        const CMPIEnumeration* eEnumObj,
        CMPIStatus* rc)
    {
        CMPIData data = {0,CMPI_nullValue,{0}};
        const CMPIEnumeration* eEnum = (CMPIEnumeration*)eEnumObj->hdl;

        if (!eEnum || !eEnum->hdl)
        {
            CMSetStatus(rc, CMPI_RC_ERR_INVALID_HANDLE);
            return data;
        }
        if ((void*)eEnum->ft == (void*)CMPI_ObjEnumeration_Ftab)
        {
            CMPI_ObjEnumeration* ie = (CMPI_ObjEnumeration*)eEnum;
            data.type = CMPI_instance;
            Array<SCMOInstance>* ia = (Array<SCMOInstance>*)ie->hdl;
            if (ie->cursor<ie->max)
            {
                data.value.inst = reinterpret_cast<CMPIInstance*>
                    (new CMPI_Object(
                        new SCMOInstance((*ia)[ie->cursor++]),
                        CMPI_Object::ObjectTypeInstance));
                CMSetStatus(rc, CMPI_RC_OK);
            }
            else
            {
                CMSetStatus(rc, CMPI_RC_NO_MORE_ELEMENTS);
            }
        }
        else if ((void*)eEnum->ft == (void*)CMPI_InstEnumeration_Ftab)
        {
            CMPI_InstEnumeration* ie = (CMPI_InstEnumeration*)eEnum;
            data.type = CMPI_instance;
            Array<SCMOInstance>* ia = (Array<SCMOInstance>*)ie->hdl;
            if (ie->cursor<ie->max)
            {
                data.value.inst = reinterpret_cast<CMPIInstance*>
                    (new CMPI_Object(
                        new SCMOInstance((*ia)[ie->cursor++]),
                        CMPI_Object::ObjectTypeInstance));
                CMSetStatus(rc, CMPI_RC_OK);
            }
            else
            {
                CMSetStatus(rc, CMPI_RC_NO_MORE_ELEMENTS);
            }
        }
        else
        {
            CMPI_OpEnumeration* oe = (CMPI_OpEnumeration*)eEnum;
            data.type = CMPI_ref;
            Array<SCMOInstance>* opa = (Array<SCMOInstance>*)oe->hdl;
            if (oe->cursor<oe->max)
            {
                data.value.ref = reinterpret_cast<CMPIObjectPath*>
                    (new CMPI_Object(
                        new SCMOInstance((*opa)[oe->cursor++]),
                        CMPI_Object::ObjectTypeObjectPath));
                CMSetStatus(rc, CMPI_RC_OK);
            }
            else
            {
                CMSetStatus(rc, CMPI_RC_NO_MORE_ELEMENTS);
            }
        }
        return data;
    }

    static CMPIBoolean enumHasNext(
        const CMPIEnumeration* eEnumObj,
        CMPIStatus* rc)
    {
        const CMPIEnumeration* eEnum = (CMPIEnumeration*)eEnumObj->hdl;

        if (!eEnum || !eEnum->hdl)
        {
            PEG_TRACE_CSTRING(
                TRC_CMPIPROVIDERINTERFACE,
                Tracer::LEVEL1,
                "Received invalid Handle - eEnum || eEnum->hdl...");
            CMSetStatus(rc, CMPI_RC_ERR_INVALID_HANDLE);
            return false;
        }
        CMSetStatus(rc, CMPI_RC_OK);
        if ((void*)eEnum->ft == (void*)CMPI_ObjEnumeration_Ftab)
        {
            CMPI_ObjEnumeration* ie = (CMPI_ObjEnumeration*)eEnum;
            if (ie->cursor<ie->max)
            {
                return true;
            }
        }
        else if ((void*)eEnum->ft == (void*)CMPI_InstEnumeration_Ftab)
        {
            CMPI_InstEnumeration* ie = (CMPI_InstEnumeration*)eEnum;
            if (ie->cursor<ie->max)
            {
                return true;
            }
        }
        else
        {
            CMPI_OpEnumeration* oe = (CMPI_OpEnumeration*)eEnum;
            if (oe->cursor<oe->max)
            {
                return true;
            }
        }
        return false;
    }

    extern CMPIArray* mbEncNewArray(
        const CMPIBroker* mb,
        CMPICount count,
        CMPIType type,
        CMPIStatus *rc);

    extern CMPIStatus arraySetElementAt(
        CMPIArray* eArray,
        CMPICount pos,
        const CMPIValue *val,
        CMPIType type);

    static CMPIArray* enumToArray(
        const CMPIEnumeration* eEnumObj,
        CMPIStatus* rc)
    {
        PEG_METHOD_ENTER(
            TRC_CMPIPROVIDERINTERFACE,
            "CMPI_Enumeration:enumToArray()");
        Uint32 size;
        CMPI_Object* obj;
        CMPIArray *nar = NULL;
        const CMPIEnumeration* eEnum = (CMPIEnumeration*)eEnumObj->hdl;

        if (!eEnum || !eEnum->hdl)
        {
            PEG_TRACE_CSTRING(
                TRC_CMPIPROVIDERINTERFACE,
                Tracer::LEVEL1,
                "Received invalid Handle - eEnum || eEnum->hdl...");
            CMSetStatus(rc, CMPI_RC_ERR_INVALID_HANDLE);
            PEG_METHOD_EXIT();
            return NULL;
        }
        if ((void*)eEnum->ft == (void*)CMPI_ObjEnumeration_Ftab ||
            (void*)eEnum->ft == (void*)CMPI_InstEnumeration_Ftab)
        {
            Array<SCMOInstance>* ia;
            if ((void*)eEnum->ft == (void*)CMPI_ObjEnumeration_Ftab)
            {
                CMPI_ObjEnumeration* ie = (CMPI_ObjEnumeration*)eEnum;
                ia = (Array<SCMOInstance>*)ie->hdl;
            }
            else
            {
                CMPI_InstEnumeration* ie = (CMPI_InstEnumeration*)eEnum;
                ia = (Array<SCMOInstance>*)ie->hdl;
            }
            size = ia->size();
            nar = mbEncNewArray(NULL,size,CMPI_instance,NULL);
            for (Uint32 i=0; i<size; i++)
            {
                SCMOInstance &inst = (*ia)[i];
                obj = new CMPI_Object(new SCMOInstance(inst),
                                      CMPI_Object::ObjectTypeInstance);
                arraySetElementAt(nar,i,(CMPIValue*)&obj,CMPI_instance);
            }
        }
        else
        {
            CMPI_OpEnumeration* oe = (CMPI_OpEnumeration*)eEnum;
            Array<SCMOInstance>* opa = (Array<SCMOInstance>*)oe->hdl;
            size = opa->size();
            nar = mbEncNewArray(NULL,size,CMPI_ref,NULL);
            for (Uint32 i=0; i<size; i++)
            {
                SCMOInstance &op = (*opa)[i];
                obj = new CMPI_Object(new SCMOInstance(op),
                                      CMPI_Object::ObjectTypeObjectPath);
                arraySetElementAt(nar,i,(CMPIValue*)&obj,CMPI_ref);
            }
        }
        PEG_METHOD_EXIT();
        return nar;
    }
}

static CMPIEnumerationFT objEnumeration_FT =
{
    CMPICurrentVersion,
    enumRelease,
    enumClone,
    enumGetNext,
    enumHasNext,
    enumToArray,
};

static CMPIEnumerationFT instEnumeration_FT =
{
    CMPICurrentVersion,
    enumRelease,
    enumClone,
    enumGetNext,
    enumHasNext,
    enumToArray,
};

static CMPIEnumerationFT opEnumeration_FT =
{
    CMPICurrentVersion,
    enumRelease,
    enumClone,
    enumGetNext,
    enumHasNext,
    enumToArray,
};

CMPIEnumerationFT *CMPI_ObjEnumeration_Ftab = &objEnumeration_FT;
CMPIEnumerationFT *CMPI_InstEnumeration_Ftab = &instEnumeration_FT;
CMPIEnumerationFT *CMPI_OpEnumeration_Ftab = &opEnumeration_FT;

CMPI_ObjEnumeration::CMPI_ObjEnumeration(Array<SCMOInstance>* oa)
{
    PEG_METHOD_ENTER(
        TRC_CMPIPROVIDERINTERFACE,
        "CMPI_ObjEnumeration::CMPI_ObjEnumeration()");
    cursor = 0;
    max = oa->size();
    hdl = (void*)oa;
    ft = CMPI_ObjEnumeration_Ftab;
    PEG_METHOD_EXIT();
}
CMPI_InstEnumeration::CMPI_InstEnumeration(Array<SCMOInstance>* ia)
{
    PEG_METHOD_ENTER(
        TRC_CMPIPROVIDERINTERFACE,
        "CMPI_InstEnumeration::CMPI_InstEnumeration()");
    cursor = 0;
    max = ia->size();
    hdl = (void*)ia;
    ft = CMPI_InstEnumeration_Ftab;
    PEG_METHOD_EXIT();
}

CMPI_OpEnumeration::CMPI_OpEnumeration(Array<SCMOInstance>* opa)
{
    PEG_METHOD_ENTER(
        TRC_CMPIPROVIDERINTERFACE,
        "CMPI_OpEnumeration::CMPI_OpEnumeration()");
    cursor = 0;
    max = opa->size();
    hdl = (void*)opa;
    ft = CMPI_OpEnumeration_Ftab;
    PEG_METHOD_EXIT();
}

PEGASUS_NAMESPACE_END













