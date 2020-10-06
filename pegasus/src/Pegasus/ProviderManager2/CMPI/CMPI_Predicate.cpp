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

#include "CMPI_Predicate.h"
#include "CMPI_Ftabs.h"
#include "CMPI_Value.h"
#include "CMPI_String.h"
#include <Pegasus/Common/Tracer.h>

PEGASUS_USING_STD;
PEGASUS_NAMESPACE_BEGIN

extern "C"
{
    CMPIStatus prdRelease(CMPIPredicate* sc)
    {
        CMPI_Predicate *pred = (CMPI_Predicate*)sc->hdl;
        if (pred)
        {
            delete pred;
            reinterpret_cast<CMPI_Object*>(sc)->unlinkAndDelete();
            CMReturn(CMPI_RC_OK);
        }
        else
        {
            CMReturn(CMPI_RC_ERR_INVALID_HANDLE);
        }
    }

    CMPIPredicate* prdClone(const CMPIPredicate* ePrd, CMPIStatus* rc)
    {
        CMSetStatus(rc, CMPI_RC_ERR_NOT_SUPPORTED);
        return NULL;
    }

    CMPIStatus prdGetData(
        const CMPIPredicate* ePrd,
        CMPIType* type,
        CMPIPredOp* op,
        CMPIString** lhs,
        CMPIString** rhs)
    {
        const CMPI_Predicate *prd = (CMPI_Predicate*)ePrd->hdl;
        if (!prd)
        {
            PEG_TRACE_CSTRING(
                TRC_CMPIPROVIDERINTERFACE,
                Tracer::LEVEL1,
                "Received invalid handle in CMPI_Predicate:prdGetData");
            CMReturn(CMPI_RC_ERR_INVALID_HANDLE);
        }
        CMPI_term_el *term =  (CMPI_term_el *)prd->priv;

        if (term)
        {
            String o1,o2;
            CMPIPredOp o;
            CMPIType t;
            term->toStrings(t,o,o1,o2);

            if (type)
            {
                *type=t;
            }
            if (op)
            {
                *op=o;
            }
            if (lhs)
            {
                *lhs = string2CMPIString(o1);
            }
            if (rhs)
            {
                *rhs = string2CMPIString(o2);
            }
            CMReturn(CMPI_RC_OK);
        }
        else
        {
            PEG_TRACE_CSTRING(
                TRC_CMPIPROVIDERINTERFACE,
                Tracer::LEVEL1,
                "Operation not Supported in CMPI_Predicate:prdGetData");
            CMReturn(CMPI_RC_ERR_NOT_SUPPORTED);
        }
    }

#if defined (CMPI_VER_87) && !defined(CMPI_VER_100)
    int prdEvaluate(
        CMPIPredicate* pr,
        CMPIValue* value,
        CMPIType type,
        CMPIStatus* rc)
    {
        PEG_TRACE_CSTRING(
            TRC_CMPIPROVIDERINTERFACE,
            Tracer::LEVEL1,
            "Operation not Supported in CMPI_Predicate:prdEvaluate");
        CMSetStatus(rc, CMPI_RC_ERR_NOT_SUPPORTED);
        return 0;
    }
#endif

    CMPIBoolean prdEvaluateUsingAccessor(
        const CMPIPredicate*,
        CMPIAccessor *f,
        void *p,
        CMPIStatus *rc)
    {
        PEG_TRACE_CSTRING(
            TRC_CMPIPROVIDERINTERFACE,
            Tracer::LEVEL1,
            "Operation not Supported in \
            CMPI_Predicate:prdEvaluateUsingAccessor");
        CMSetStatus(rc, CMPI_RC_ERR_NOT_SUPPORTED);
        return 0;
    }
}

static CMPIPredicateFT prd_FT =
{
    CMPICurrentVersion,
    prdRelease,
    prdClone,
    prdGetData,
#if defined (CMPI_VER_87) && !defined(CMPI_VER_100)
    prdEvaluate,
#endif
#if defined(CMPI_VER_100)
    prdEvaluateUsingAccessor
#endif
};

CMPIPredicateFT *CMPI_Predicate_Ftab = &prd_FT;

CMPI_Predicate::CMPI_Predicate(const CMPI_term_el* t) : priv((void*)t)
{
    ft = CMPI_Predicate_Ftab;
}


PEGASUS_NAMESPACE_END
