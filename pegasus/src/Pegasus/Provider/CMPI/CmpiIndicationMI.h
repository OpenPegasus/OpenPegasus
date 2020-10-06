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
//%///////////////////////////////////////////////////////////////////////////


#ifndef _CmpiIndicationMI_h_
#define _CmpiIndicationMI_h_

#include <iostream>

#include "cmpidt.h"
#include "cmpift.h"

#include "CmpiBaseMI.h"
#include "CmpiStatus.h"
#include "CmpiObjectPath.h"
#include "CmpiResult.h"
#include "CmpiContext.h"
#include "CmpiSelectExp.h"
#include "Linkage.h"


class PEGASUS_CMPI_PROVIDER_LINKAGE CmpiIndicationMI :
    virtual public CmpiBaseMI
{
public:
    CmpiIndicationMI(const CmpiBroker &mbp, const CmpiContext& ctx);

    static CMPIStatus driveAuthorizeFilter(
        CMPIIndicationMI* mi,
        const CMPIContext* eCtx,
        const CMPISelectExp* se,
        const char* ns,
        const CMPIObjectPath* op,
        const char* user);
    static CMPIStatus driveMustPoll(
        CMPIIndicationMI* mi,
        const CMPIContext* eCtx,
        const CMPISelectExp* se,
        const char* ns,
        const CMPIObjectPath* op);
    static CMPIStatus driveActivateFilter(
        CMPIIndicationMI* mi,
        const CMPIContext* eCtx,
        const CMPISelectExp* se,
        const char* clsName,
        const CMPIObjectPath* op,
        CMPIBoolean first);
    static CMPIStatus driveDeActivateFilter(
        CMPIIndicationMI* mi,
        const CMPIContext* eCtx,
        const CMPISelectExp* se,
        const char* clsName,
        const CMPIObjectPath* op,
        CMPIBoolean last);
    static CMPIStatus driveEnableIndications(
        CMPIIndicationMI* mi,
         const CMPIContext* eCtx);
    static CMPIStatus  driveDisableIndications(
        CMPIIndicationMI* mi,
        const CMPIContext* eCtx);

    virtual CmpiStatus authorizeFilter(
        const CmpiContext& ctx,
        const CmpiSelectExp& se,
        const char* ns,
        const CmpiObjectPath& op,
        const char* user);
    virtual CmpiStatus mustPoll(
        const CmpiContext& ctx,
        const CmpiSelectExp& se,
        const char* ns,
        const CmpiObjectPath& op);
    virtual CmpiStatus activateFilter(
        const CmpiContext& ctx,
        const CmpiSelectExp& se,
        const char* ns,
        const CmpiObjectPath& op,
        CmpiBoolean first);
    virtual CmpiStatus deActivateFilter(
        const CmpiContext& ctx,
        const CmpiSelectExp& se,
        const char* ns,
        const CmpiObjectPath& op,
        CmpiBoolean last);
    virtual CmpiStatus enableIndications(
        const CmpiContext& ctx);
   virtual CmpiStatus disableIndications(
        const CmpiContext& ctx);
};

#endif

