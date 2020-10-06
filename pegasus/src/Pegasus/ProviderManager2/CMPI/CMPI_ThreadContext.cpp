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
#include "CMPI_ThreadContext.h"

#if !defined(PEGASUS_OS_TYPE_WINDOWS)
# include <pthread.h>
#endif
#include <limits.h>


PEGASUS_USING_STD;
PEGASUS_NAMESPACE_BEGIN

// this static gets initialized on load of CMPIProviderManager library
// in the constructor of CMPI_ThreadContextKey the key for the thread-specific
// storage used by CMPI is created
// This was done to ensure that the key gets created exactly once, saving the
// need to check for it on later access.
CMPI_ThreadContextKey CMPI_ThreadContext::globalThreadContextKey;

CMPI_ThreadContext::CMPI_ThreadContext(
const CMPIBroker *mb,
const CMPIContext *ctx )
{
    CIMfirst=CIMlast=NULL;
    broker=mb;
    context=ctx;
    prev=(CMPI_ThreadContext*)
        TSDKey::get_thread_specific(globalThreadContextKey.contextKey);
    TSDKey::set_thread_specific(globalThreadContextKey.contextKey,this);
    return;
}

CMPI_ThreadContext::~CMPI_ThreadContext()
{
    for( CMPI_Object *nxt,*cur=CIMfirst; cur; cur=nxt )
    {
        nxt=cur->next;
        (reinterpret_cast<CMPIInstance*>(cur))->ft->release(
        reinterpret_cast<CMPIInstance*>(cur));
    }
    TSDKey::set_thread_specific(globalThreadContextKey.contextKey,prev);
}

PEGASUS_NAMESPACE_END

