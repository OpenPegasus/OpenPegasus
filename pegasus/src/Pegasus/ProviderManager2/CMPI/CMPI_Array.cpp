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
#include "CMPI_Ftabs.h"
#include "CMPI_Array.h"
#include <Pegasus/Common/Tracer.h>
#include <string.h>

PEGASUS_USING_STD;
PEGASUS_NAMESPACE_BEGIN

extern "C"
{

    PEGASUS_STATIC CMPIStatus arrayRelease(CMPIArray* eArray)
    {
        PEG_METHOD_ENTER(
            TRC_CMPIPROVIDERINTERFACE,
            "CMPI_Array:arrayRelease()");
        CMPIStatus rrc = {CMPI_RC_OK,NULL};
        CMPI_Array *arr = (CMPI_Array*)eArray->hdl;
        if (!arr)
        {
            PEG_METHOD_EXIT();
            CMReturn(CMPI_RC_ERR_INVALID_HANDLE);
        }
        CMPIData* dta=(CMPIData*)arr->hdl;
        if (!dta)
        {
            PEG_METHOD_EXIT();
            CMReturn(CMPI_RC_ERR_INVALID_HANDLE);
        }

        if (arr->isCloned != 0)
        {
            for (unsigned int i=1; i<=dta->value.uint32; i++)
            {
                if (dta->type & CMPI_ENC && dta[i].state==CMPI_goodValue)
                {
                    switch (dta[i].type)
                    {
                        case CMPI_instance:
                            if (dta[i].value.inst)
                            {
                                rrc = (dta[i].value.inst)->ft->release(
                                    dta[i].value.inst);
                            }
                            break;
                        case CMPI_ref:
                            if (dta[i].value.ref)
                            {
                                rrc = (dta[i].value.ref)->ft->release(
                                    dta[i].value.ref);
                            }
                            break;
                        case CMPI_args:
                            if (dta[i].value.args)
                            {
                                rrc = (dta[i].value.args)->ft->release(
                                    dta[i].value.args);
                            }
                            break;
                        case CMPI_dateTime:
                            if (dta[i].value.dateTime)
                            {
                                rrc = (dta[i].value.dateTime)->ft->release(
                                    dta[i].value.dateTime);
                            }
                            break;
                        case CMPI_enumeration:
                            if (dta[i].value.Enum)
                            {
                                rrc = (dta[i].value.Enum)->ft->release(
                                    dta[i].value.Enum);
                            }
                            break;
                        case CMPI_filter:
                            if (dta[i].value.filter)
                            {
                                rrc = (dta[i].value.filter)->ft->release(
                                    dta[i].value.filter);
                            }
                            break;
                        case CMPI_charsptr:
                            if (dta[i].value.dataPtr.ptr)
                            {
                                free(dta[i].value.dataPtr.ptr);
                            }
                            break;
                        case CMPI_string:
                            if (dta[i].value.string)
                            {
                                rrc = (dta[i].value.string)->ft->release(
                                    dta[i].value.string);
                            }
                            break;
                    }
                }
            }
        }
        delete[] dta;
        delete arr;
        reinterpret_cast<CMPI_Object*>(eArray)->unlinkAndDelete();
        PEG_METHOD_EXIT();
        CMReturn(CMPI_RC_OK);
    }

    PEGASUS_STATIC CMPIArray* arrayClone(
        const CMPIArray* eArray,
        CMPIStatus* rc)
    {
        PEG_METHOD_ENTER(
            TRC_CMPIPROVIDERINTERFACE,
            "CMPI_Array:arrayClone()");
        CMPI_Array *arr = (CMPI_Array*)eArray->hdl;
        if (!arr)
        {
            CMSetStatus(rc, CMPI_RC_ERR_INVALID_HANDLE);
            PEG_METHOD_EXIT();
            return NULL;
        }
        CMPIData* dta=(CMPIData*)arr->hdl;
        if (!dta)
        {
            CMSetStatus(rc, CMPI_RC_ERR_INVALID_HANDLE);
            PEG_METHOD_EXIT();
            return NULL;
        }
        CMPIData* nDta = new CMPIData[dta->value.uint32+1];
        CMPI_Array* nArr = new CMPI_Array(nDta, true);
        CMPI_Object* obj = new CMPI_Object(nArr);
        obj->unlink();
        CMPIArray* nArray = reinterpret_cast<CMPIArray*>(obj);
        CMPIStatus rrc = {CMPI_RC_OK,NULL};

        if (dta->type & CMPI_ENC)
        {
            for (unsigned int i=1; i<=dta->value.uint32; i++)
            {
                nDta[i].state=CMPI_nullValue;
            }
        }

        for (unsigned int i=0; i<=dta->value.uint32; i++)
        {
            nDta[i]=dta[i];
            if (i == 0)
            {
                continue;
            }
            if (dta->type & CMPI_ENC && dta[i].state==CMPI_goodValue)
            {
                switch (dta[i].type)
                {
                    case CMPI_instance:
                        if (dta[i].value.inst)
                        {
                            nDta[i].value.inst =
                                (dta[i].value.inst)->ft->clone(
                                    dta[i].value.inst,
                                    &rrc);
                        }
                        break;

                    case CMPI_ref:
                        if (dta[i].value.ref)
                        {
                            nDta[i].value.ref = (dta[i].value.ref)->ft->clone(
                                dta[i].value.ref,
                                &rrc);
                        }
                        break;

                    case CMPI_args:
                        if (dta[i].value.args)
                        {
                            nDta[i].value.args =
                                (dta[i].value.args)->ft->clone(
                                    dta[i].value.args,
                                    &rrc);
                        }
                        break;

                    case CMPI_dateTime:
                        if (dta[i].value.dateTime)
                        {
                            nDta[i].value.dateTime =
                                (dta[i].value.dateTime)->ft->clone(
                                    dta[i].value.dateTime,
                                    &rrc);
                        }
                        break;

                    case CMPI_enumeration:
                        if (dta[i].value.Enum)
                        {
                            nDta[i].value.Enum =
                                (dta[i].value.Enum)->ft->clone(
                                    dta[i].value.Enum,
                                    &rrc);
                        }
                        break;

                    case CMPI_filter:
                        if (dta[i].value.filter)
                        {
                            nDta[i].value.filter =
                                (dta[i].value.filter)->ft->clone(
                                    dta[i].value.filter,
                                    &rrc);
                        }
                        break;

                    case CMPI_charsptr:
                        if (dta[i].value.dataPtr.length>0)
                        {
                            nDta[i].value.dataPtr.length =
                                dta[i].value.dataPtr.length;
                            nDta[i].value.dataPtr.ptr =
                                malloc(nDta[i].value.dataPtr.length);
                            if (nDta[i].value.dataPtr.ptr == NULL)
                            {
                                arrayRelease(nArray);
                                if (rc)
                                {
                                    *rc=rrc;
                                }
                                return NULL;
                            }
                            memcpy(
                                nDta[i].value.dataPtr.ptr,
                                dta[i].value.dataPtr.ptr,
                                dta[i].value.dataPtr.length);
                        }
                        break;

                    case CMPI_string:
                        if (dta[i].value.string)
                        {
                            nDta[i].value.string =
                                (dta[i].value.string)->ft->clone(
                                    dta[i].value.string,
                                    &rrc);
                        }
                        break;
                }
                if (rrc.rc)
                {
                    arrayRelease(nArray);
                    if (rc)
                    {
                        *rc=rrc;
                    }
                    PEG_METHOD_EXIT();
                    return NULL;
                }
            }
        }

        CMSetStatus(rc,CMPI_RC_OK);
        PEG_METHOD_EXIT();
        return nArray;
    }

    PEGASUS_STATIC CMPIData arrayGetElementAt(
        const CMPIArray* eArray,
        CMPICount pos,
        CMPIStatus* rc)
    {
        CMPIData data = {0,CMPI_nullValue,{0}};
        CMPI_Array *arr = (CMPI_Array*)eArray->hdl;
        if (!arr)
        {
            CMSetStatus(rc, CMPI_RC_ERR_INVALID_HANDLE);
            return data;
        }

        CMPIData *dta = (CMPIData*)arr->hdl;
        if (!dta)
        {
            CMSetStatus(rc, CMPI_RC_ERR_INVALID_HANDLE);
            return data;
        }
        CMSetStatus(rc,CMPI_RC_OK);
        if (pos < dta->value.uint32)
        {
            if (dta->type == CMPI_chars && dta[pos+1].state == CMPI_goodValue)
            {
                data.type = CMPI_chars;
                data.state = CMPI_goodValue;
                data.value.chars = (char*)CMGetCharsPtr(
                    dta[pos+1].value.string,
                    NULL);
                return data;
            }
            else
            {
                return dta[pos+1];
            }
        }
        CMSetStatus(rc,CMPI_RC_ERR_NO_SUCH_PROPERTY);
        return data;
    }

    CMPIStatus arraySetElementAt(
        CMPIArray* eArray,
        CMPICount pos,
        const CMPIValue *val,
        CMPIType type)
    {
        PEG_METHOD_ENTER(
            TRC_CMPIPROVIDERINTERFACE,
            "CMPI_Array:arraySetElementAt()");
        CMPI_Array *arr = (CMPI_Array*)eArray->hdl;
        if (!arr)
        {
            PEG_METHOD_EXIT();
            CMReturn(CMPI_RC_ERR_INVALID_HANDLE);
        }

        CMPIData *dta = (CMPIData*)arr->hdl;
        if (!dta)
        {
            PEG_METHOD_EXIT();
            CMReturn(CMPI_RC_ERR_INVALID_HANDLE);
        }
        if (!val)
        {
            PEG_METHOD_EXIT();
            CMReturn( CMPI_RC_ERR_INVALID_PARAMETER);
        }
        if (pos<dta->value.uint32)
        {
            if (type == CMPI_null)
            {
                dta[pos+1].state=CMPI_nullValue;
                PEG_METHOD_EXIT();
                CMReturn(CMPI_RC_OK);
            }
            else if ((dta->type&~CMPI_ARRAY)==type)
            {
                dta[pos+1].state=CMPI_goodValue;
                if (type == CMPI_chars)
                {
                    // Store char* as CMPIString internally, this frees us from
                    // doing explicit memory management for char*.
                    dta[pos+1].value.string = reinterpret_cast<CMPIString*>(
                        new CMPI_Object((const char*) val));
                    dta[pos+1].type = CMPI_string;
                }
                else
                {
                    dta[pos+1].value = *val;
                }
                PEG_METHOD_EXIT();
                CMReturn(CMPI_RC_OK);
            }
            else
            {
                char msg[512];
                sprintf(msg,"arraySetElementAt(): CMPI_RC_ERR_TYPE_MISMATCH."
                    " Is %u - should be %u",
                    (unsigned int)type, (unsigned int)dta->type);
                PEG_METHOD_EXIT();
                CMReturnWithString(
                    CMPI_RC_ERR_TYPE_MISMATCH,
                    reinterpret_cast<CMPIString*>(new CMPI_Object(msg)));
            }
        }
        PEG_METHOD_EXIT();
        CMReturn(CMPI_RC_ERR_NO_SUCH_PROPERTY);
    }

    PEGASUS_STATIC CMPICount arrayGetSize(
        const CMPIArray* eArray,
        CMPIStatus* rc)
    {
        CMPI_Array *arr = (CMPI_Array*)eArray->hdl;
        if (!arr)
        {
            CMSetStatus(rc, CMPI_RC_ERR_INVALID_HANDLE);
            return 0;
        }
        CMPIData *dta = (CMPIData*)arr->hdl;
        if (!dta)
        {
            CMSetStatus(rc, CMPI_RC_ERR_INVALID_HANDLE);
            return 0;
        }
        CMSetStatus(rc,CMPI_RC_OK);
        return dta->value.uint32;
    }

    PEGASUS_STATIC CMPIType arrayGetType(
        const CMPIArray* eArray,
        CMPIStatus* rc)
    {
        CMPI_Array *arr = (CMPI_Array*)eArray->hdl;
        if (!arr)
        {
            CMSetStatus(rc, CMPI_RC_ERR_INVALID_HANDLE);
            return 0;
        }
        CMPIData *dta = (CMPIData*)arr->hdl;
        if (!dta)
        {
            CMSetStatus(rc, CMPI_RC_ERR_INVALID_HANDLE);
            return CMPI_null;
        }
        CMSetStatus(rc,CMPI_RC_OK);
        return dta->type;
    }
}

static CMPIArrayFT array_FT =
{
    CMPICurrentVersion,
    arrayRelease,
    arrayClone,
    arrayGetSize,
    arrayGetType,
    arrayGetElementAt,
    arraySetElementAt,
};

CMPIArrayFT *CMPI_Array_Ftab = &array_FT;

PEGASUS_NAMESPACE_END
