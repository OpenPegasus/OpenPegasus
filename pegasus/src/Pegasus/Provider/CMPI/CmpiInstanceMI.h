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
//%////////////////////////////////////////////////////////////////////////////

#ifndef _CmpiInstanceMI_h_
#define _CmpiInstanceMI_h_

#include <iostream>

#include "cmpidt.h"
#include "cmpift.h"

#include "CmpiBaseMI.h"
#include "CmpiStatus.h"
#include "CmpiString.h"
#include "CmpiData.h"
#include "CmpiObjectPath.h"
#include "CmpiInstance.h"
#include "CmpiResult.h"
#include "CmpiContext.h"
#include "Linkage.h"

class PEGASUS_CMPI_PROVIDER_LINKAGE CmpiInstanceMI : virtual public CmpiBaseMI
{
public:
    CmpiInstanceMI(const CmpiBroker &mbp, const CmpiContext& ctx);

    static CMPIStatus driveEnumInstanceNames(
        CMPIInstanceMI* mi,
        const CMPIContext* eCtx,
        const CMPIResult* eRslt,
        const CMPIObjectPath* eCop);
    static CMPIStatus driveEnumInstances(
        CMPIInstanceMI* mi,
        const CMPIContext* eCtx,
        const CMPIResult* eRslt,
        const CMPIObjectPath* eCop,
        const char* *properties);
    static CMPIStatus driveGetInstance(
        CMPIInstanceMI* mi,
        const CMPIContext* eCtx,
        const CMPIResult* eRslt,
        const CMPIObjectPath* eCop,
        const char* *properties);
    static CMPIStatus driveCreateInstance(
        CMPIInstanceMI* mi,
        const CMPIContext* eCtx,
        const CMPIResult* eRslt,
        const CMPIObjectPath* eCop,
        const CMPIInstance* eInst);
    static CMPIStatus driveSetInstance(
        CMPIInstanceMI* mi,
        const CMPIContext* eCtx,
        const CMPIResult* eRslt,
        const CMPIObjectPath* eCop,
        const CMPIInstance* eInst,
        const char* *properties);
    static CMPIStatus driveDeleteInstance(
        CMPIInstanceMI* mi,
        const CMPIContext* eCtx,
        const CMPIResult* eRslt,
        const CMPIObjectPath* eCop);
    static CMPIStatus driveExecQuery(
        CMPIInstanceMI* mi,
        const CMPIContext* eCtx,
        const CMPIResult* eRslt,
        const CMPIObjectPath* eCop,
        const char* language,
        const char* query);
    virtual CmpiStatus enumInstanceNames(
        const CmpiContext& ctx,
        CmpiResult& rslt,
        const CmpiObjectPath& cop);
    virtual CmpiStatus enumInstances(
        const CmpiContext& ctx,
        CmpiResult& rslt,
        const CmpiObjectPath& cop,
        const char* *properties);
    virtual CmpiStatus getInstance(
        const CmpiContext& ctx,
        CmpiResult& rslt,
        const CmpiObjectPath& cop,
        const char* *properties);
    virtual CmpiStatus createInstance(
        const CmpiContext& ctx,
        CmpiResult& rslt,
        const CmpiObjectPath& cop,
        const CmpiInstance& inst);
    virtual CmpiStatus setInstance(
        const CmpiContext& ctx,
        CmpiResult& rslt,
        const CmpiObjectPath& cop,
        const CmpiInstance& inst,
        const char* *properties);
    virtual CmpiStatus deleteInstance(
        const CmpiContext& ctx,
        CmpiResult& rslt,
        const CmpiObjectPath& cop);
    virtual CmpiStatus execQuery(
        const CmpiContext& ctx,
        CmpiResult& rslt,
        const CmpiObjectPath& cop,
        const char* language,
        const char* query);
};

#endif
