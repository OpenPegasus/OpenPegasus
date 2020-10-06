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

#include "CMPI_SelectCond.h"
#include "CMPI_Ftabs.h"
#include "CMPI_Value.h"
#include "CMPI_String.h"
#include <Pegasus/Common/Tracer.h>

PEGASUS_USING_STD;
PEGASUS_NAMESPACE_BEGIN

extern "C"
{

    CMPIStatus scndRelease(CMPISelectCond* eSc)
    {
        PEG_METHOD_ENTER(
            TRC_CMPIPROVIDERINTERFACE,
            "CMPI_SelectCond:scndRelease()");
         CMPI_SelectCond *sc = (CMPI_SelectCond*)eSc->hdl;
        if (sc)
        {
            CMPI_SelectCondData *data = (CMPI_SelectCondData *)sc->priv;
            if (data)
            {
                delete data;
            }
            delete sc;
            reinterpret_cast<CMPI_Object*>(eSc)->unlinkAndDelete();
            PEG_METHOD_EXIT();
            CMReturn(CMPI_RC_OK);
        }
        else
        {
            PEG_METHOD_EXIT();
            CMReturn(CMPI_RC_ERR_INVALID_HANDLE);
        }
    }

    CMPISelectCond* scndClone(const CMPISelectCond* eSc, CMPIStatus* rc)
    {
        CMSetStatus(rc,CMPI_RC_ERR_NOT_SUPPORTED);
        return NULL;
    }

    CMPICount scndGetCountAndType(
        const CMPISelectCond* eSc,
        int* type,
        CMPIStatus* rc)
    {
        PEG_METHOD_ENTER(
            TRC_CMPIPROVIDERINTERFACE,
            "CMPI_SelectCond:scndGetCountAndType()");
        CMPI_SelectCond *sc=(CMPI_SelectCond*)eSc->hdl;
        if (!sc)
        {
            PEG_TRACE_CSTRING(
                TRC_CMPIPROVIDERINTERFACE,
                Tracer::LEVEL1,
                "Invalid hanle in \
                CMPI_SelectCond:scndGetCountAndType");
            CMSetStatus (rc, CMPI_RC_ERR_INVALID_HANDLE);
            PEG_METHOD_EXIT();
            return 0;
        }
        CMPI_SelectCondData *data = (CMPI_SelectCondData *)sc->priv;

        if (data)
        {
            if (type!=NULL) *type=data->type;
            CMSetStatus(rc,CMPI_RC_OK);
            PEG_METHOD_EXIT();
            return data->tableau->size();
        }
        PEG_METHOD_EXIT();
        return 0;
    }

    CMPISubCond* scndGetSubCondAt(
        const CMPISelectCond* eSc,
        unsigned int index,
        CMPIStatus* rc)
    {
        PEG_METHOD_ENTER(
            TRC_CMPIPROVIDERINTERFACE,
            "CMPI_SelectCond:scndGetSubCondAt()");
        CMPI_SelectCond *sc=(CMPI_SelectCond*)eSc->hdl;
        if (!sc)
        {
            PEG_TRACE_CSTRING(
                TRC_CMPIPROVIDERINTERFACE,
                Tracer::LEVEL1,
                "Invalid hanle in \
                CMPI_SelectCond:scndGetSubCondAt");
            CMSetStatus (rc, CMPI_RC_ERR_INVALID_HANDLE);
            return 0;
        }
        CMPI_SelectCondData *data = (CMPI_SelectCondData *)sc->priv;
        if (data)
        {
            if (index<=data->tableau->size())
            {
                const CMPI_TableauRow *row=(data->tableau->getData())+index;

                CMPISubCond *sbc=(CMPISubCond*)new CMPI_SubCond(row);
                CMPI_Object *obj = new CMPI_Object(sbc);
                CMSetStatus(rc,CMPI_RC_OK);
                CMPISubCond* cmpiSubCond =
                    reinterpret_cast<CMPISubCond *>(obj);
                PEG_METHOD_EXIT();
                return cmpiSubCond;
            }
        }
        else
        {
            PEG_TRACE_CSTRING(
                TRC_CMPIPROVIDERINTERFACE,
                Tracer::LEVEL1,
                "Property Not Found in \
                CMPI_SelectCond:scndGetSubCondAt");
            CMSetStatus(rc,CMPI_RC_ERR_NO_SUCH_PROPERTY);
        }
        PEG_METHOD_EXIT();
        return NULL;
    }

}

static CMPISelectCondFT scnd_FT=
{
    CMPICurrentVersion,
    scndRelease,
    scndClone,
    scndGetCountAndType,
    scndGetSubCondAt,
};

CMPISelectCondFT *CMPI_SelectCond_Ftab=&scnd_FT;

CMPI_SelectCondData::CMPI_SelectCondData(
    CMPI_Tableau *tblo,
    int t): tableau(tblo), type(t)
{
}

CMPI_SelectCond::CMPI_SelectCond(CMPI_Tableau* tblo, int t)
{
    priv = new CMPI_SelectCondData(tblo, t);
    ft=CMPI_SelectCond_Ftab;
}

PEGASUS_NAMESPACE_END
