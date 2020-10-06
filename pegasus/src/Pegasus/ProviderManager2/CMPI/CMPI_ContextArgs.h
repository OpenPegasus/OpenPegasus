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

#ifndef _CMPI_ContextArgs_H_
#define _CMPI_ContextArgs_H_

#include "CMPI_Object.h"

#include <Pegasus/Provider/CMPI/cmpidt.h>
#include <Pegasus/Provider/CMPI/cmpift.h>

#include <Pegasus/Common/OperationContext.h>

PEGASUS_NAMESPACE_BEGIN

extern CMPIArgsFT *CMPI_Args_Ftab;

class CMPI_ThreadContext;

struct CMPI_Context : CMPIContext
{
    CMPI_Object *next,*prev;
    OperationContext* ctx;
    CMPI_ThreadContext *thr;
    CMPI_Context(const OperationContext& ct);
    ~CMPI_Context();
};

struct CMPI_ArgsOnStack : CMPIArgs
{
    CMPI_Object *next,*prev;
    CMPI_ArgsOnStack(const Array<CIMParamValue>& args);
};

struct CMPI_ContextOnStack : CMPIContext
{
    CMPI_Object *next,*prev;
    OperationContext* ctx;
    CMPI_ContextOnStack(const OperationContext& ct);
    ~CMPI_ContextOnStack();
};

PEGASUS_NAMESPACE_END

#endif
