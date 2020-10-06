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

#include "CMPI_SubCond.h"
#include "CMPI_Ftabs.h"
#include "CMPI_Value.h"
#include "CMPI_String.h"
#include <Pegasus/Common/Tracer.h>

PEGASUS_USING_STD;
PEGASUS_NAMESPACE_BEGIN

extern "C"
{

    CMPIStatus sbcRelease(CMPISubCond* sc)
    {
        PEG_METHOD_ENTER(
            TRC_CMPIPROVIDERINTERFACE,
            "CMPI_SubCond:sbcRelease()");
        CMPI_SubCond *sbc = (CMPI_SubCond*)sc->hdl;
        if( sbc )
        {
            delete sbc;
            reinterpret_cast<CMPI_Object*>(sc)->unlinkAndDelete();
            PEG_METHOD_EXIT();
            CMReturn(CMPI_RC_OK);
        }
        else
        {
            PEG_TRACE_CSTRING(
                TRC_CMPIPROVIDERINTERFACE,
                Tracer::LEVEL1,
                "Invalid handle in CMPI_SubCond:sbcRelease");
            PEG_METHOD_EXIT();
            CMReturn(CMPI_RC_ERR_INVALID_HANDLE);
        }
    }

    CMPISubCond* sbcClone(const CMPISubCond* eSc, CMPIStatus* rc)
    {
        CMSetStatus(rc,CMPI_RC_ERR_NOT_SUPPORTED);
        return NULL;
    }

    CMPICount sbcGetCount(const CMPISubCond* eSbc, CMPIStatus* rc)
    {
        PEG_METHOD_ENTER(
            TRC_CMPIPROVIDERINTERFACE,
            "CMPI_SubCond:sbcGetCount()");
        const CMPI_SubCond *sbc = (CMPI_SubCond*)eSbc->hdl;
        if( !sbc )
        {
            PEG_TRACE_CSTRING(
                TRC_CMPIPROVIDERINTERFACE,
                Tracer::LEVEL1,
                "Invalid handle in CMPI_SubCond:sbcGetCount");
            CMSetStatus(rc, CMPI_RC_ERR_INVALID_HANDLE);
            PEG_METHOD_EXIT();
            return 0;
        }
        CMPI_TableauRow* row = (CMPI_TableauRow* )sbc->priv;
        CMSetStatus(rc,CMPI_RC_OK);
        if( row )
        {
            PEG_METHOD_EXIT();
            return row->size();
        }
        PEG_METHOD_EXIT();
        return 0;
    }

    CMPIPredicate* sbcGetPredicateAt(const CMPISubCond* eSbc,
    unsigned int index, CMPIStatus* rc)
    {
        PEG_METHOD_ENTER(
            TRC_CMPIPROVIDERINTERFACE,
            "CMPI_SubCond:sbcGetPredicateAt()");
        const CMPI_SubCond *sbc = (CMPI_SubCond*)eSbc->hdl;
        if( !sbc )
        {
            PEG_TRACE_CSTRING(
                TRC_CMPIPROVIDERINTERFACE,
                Tracer::LEVEL1,
                "Invalid handle in CMPI_SubCond:sbcGetPredicateAt");
            CMSetStatus(rc, CMPI_RC_ERR_INVALID_HANDLE);
            PEG_METHOD_EXIT();
            return 0;
        }
        CMPI_TableauRow* row = (CMPI_TableauRow* )sbc->priv;

        if( row )
        {
            if( index<=row->size() )
            {
                const CMPI_term_el *term=(row->getData())+index;

                CMPIPredicate *prd=(CMPIPredicate*)new CMPI_Predicate(term);
                /**
                   CMPI_Object puts in the hdl the pointer to the
                   CMPI_Predicate The sbcRelease will use that pointer to
                   de-allocate the CMPI_Predicate
                   structure.
                 */
                CMPI_Object *obj = new CMPI_Object(prd);

                CMSetStatus(rc,CMPI_RC_OK);
                CMPIPredicate* cmpiPredicate =
                    reinterpret_cast<CMPIPredicate*>(obj);
                PEG_METHOD_EXIT();
                return cmpiPredicate;
            }
        }
        CMSetStatus(rc, CMPI_RC_ERR_NO_SUCH_PROPERTY);
        PEG_METHOD_EXIT();
        return NULL;
    }

    CMPIPredicate* sbcGetPredicate(
    const CMPISubCond* eSbc,
    const char *name, CMPIStatus* rc)
    {
        /**
           CMPI_SubCond *sc=(CMPI_SubCond*)eSbc;
       */
        return NULL;
    }

}

static CMPISubCondFT scnd_FT=
{
    CMPICurrentVersion,
    sbcRelease,
    sbcClone,
    sbcGetCount,
    sbcGetPredicateAt,
    sbcGetPredicate
};

CMPISubCondFT *CMPI_SubCond_Ftab=&scnd_FT;

CMPI_SubCond::CMPI_SubCond(const CMPI_TableauRow* tblor)
: priv((void*)tblor)
{
    ft=CMPI_SubCond_Ftab;
}


PEGASUS_NAMESPACE_END
