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

#ifndef _CMPI_Broker_H_
#define _CMPI_Broker_H_

#include <Pegasus/Provider/CMPI/cmpidt.h>
#include <Pegasus/Provider/CMPI/cmpift.h>

#include <Pegasus/Common/String.h>
#include <Pegasus/Common/CIMClass.h>
#include <Pegasus/Provider/CIMOMHandle.h>
#include <Pegasus/Provider/CIMOMHandleRep.h>
#include <Pegasus/ProviderManager2/CMPI/CMPIClassCache.h>

PEGASUS_NAMESPACE_BEGIN

#define CM_BROKER (CMPI_ThreadContext::getBroker())
#define CM_CIMOM(mb) (CMPI_BrokerHdl::getRep(mb))

#define CM_Context(ctx) (((CMPI_Context*)ctx)->ctx)
#define CM_Instance(ci) ((CIMInstance*)ci->hdl)
#define SCMO_Instance(ci) ((SCMOInstance*)ci->hdl)
#define CM_ObjectPath(cop) ((CIMObjectPath*)cop->hdl)
#define SCMO_ObjectPath(cop) ((SCMOInstance*)cop->hdl)

#define CM_ClassOrigin(flgs) (((flgs) & CMPI_FLAG_IncludeClassOrigin)!=0)
#define CM_IncludeQualifiers(flgs) (((flgs) & CMPI_FLAG_IncludeQualifiers)!=0)

SCMOClass *mbGetSCMOClass(
    const char* ns, Uint32 nsL, const char* cls, Uint32 clsL);

class CMPI_BrokerHdl
{
public:
    static inline CIMOMHandleRep* getRep(const CMPIBroker *mb)
    {
        return ((CIMOMHandle*)mb->hdl)->_rep;
    }
};

class CMPIProvider;

struct CMPI_Broker : CMPIBroker
{
    CMPIClassCache classCache;
    String name;
    CMPIProvider *provider;
};

PEGASUS_NAMESPACE_END

#endif
