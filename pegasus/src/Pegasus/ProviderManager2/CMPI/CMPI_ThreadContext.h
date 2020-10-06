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

#ifndef _CMPITHREADCONTEXT_H_
#define _CMPITHREADCONTEXT_H_

#include <iostream>
#include <stdlib.h>
#ifdef PEGASUS_OS_HPUX
# include <pthread.h>
#endif

#include <Pegasus/Common/TSDKey.h>
#include <Pegasus/Provider/CMPI/cmpidt.h>
#include <Pegasus/Provider/CMPI/cmpift.h>
#include "CMPI_Object.h"
#include "CMPI_Enumeration.h"

PEGASUS_NAMESPACE_BEGIN

#define ENQ_BOT_LIST(i,f,l,n,p) { if (l) l->n=i; else f=i; \
                                  i->p=l; i->n=NULL; l=i;}
#define ENQ_TOP_LIST(i,f,l,n,p) { if (f) f->p=i; else l=i; \
                                   i->p=NULL; i->n=f; f=i;}
#define DEQ_FROM_LIST(i,f,l,n,p) \
                    { if (i->n) i->n->p=i->p; else l=i->p; \
                      if (i->p) i->p->n=i->n; else f=i->n;}

class CMPI_ThreadContextKey
{
public:
    CMPI_ThreadContextKey()
    {
        TSDKey::create(&contextKey);
    };
    ~CMPI_ThreadContextKey()
    {
        TSDKey::destroy(contextKey);
    };
    TSDKeyType contextKey;
};

class CMPI_ThreadContext
{
    CMPI_ThreadContext* prev;
    const CMPIBroker *broker;
    const CMPIContext *context;

    CMPI_Object *CIMfirst,*CIMlast;
    void add(CMPI_Object *o);
    void remove(CMPI_Object *o);

public:
    static void addObject(CMPI_Object*);
    static void remObject(CMPI_Object*);
    static CMPI_ThreadContext* getThreadContext();
    static const CMPIBroker* getBroker();
    static const CMPIContext* getContext();
    /**
         CMPI_ThreadContext(CMPIBroker*,CMPIContext*);
   */
    CMPI_ThreadContext(const CMPIBroker*,const CMPIContext*);
    ~CMPI_ThreadContext();

    static CMPI_ThreadContextKey globalThreadContextKey;
};

PEGASUS_NAMESPACE_END

// Most of the functions really should be inlined whereever possible
// as they do very little real work, but are called many times

PEGASUS_NAMESPACE_BEGIN

inline void CMPI_ThreadContext::add(CMPI_Object *o)
{
    ENQ_TOP_LIST(o,CIMfirst,CIMlast,next,prev);
}

inline void CMPI_ThreadContext::addObject(CMPI_Object* o)
{
    CMPI_ThreadContext* ctx=getThreadContext();
    if (ctx)
    {
        ctx->add(o);
    }
}

inline void CMPI_ThreadContext::remove(CMPI_Object *o)
{
    if( o->next!=reinterpret_cast<CMPI_Object*>((void*)-1l))
    {
        DEQ_FROM_LIST(o,CIMfirst,CIMlast,next,prev);
        o->next=reinterpret_cast<CMPI_Object*>((void*)-1l);
    }
}

inline void CMPI_ThreadContext::remObject(CMPI_Object* o)
{
    CMPI_ThreadContext* ctx=getThreadContext();
    if (ctx)
    {
        ctx->remove(o);
    }
}

inline CMPI_ThreadContext* CMPI_ThreadContext::getThreadContext()
{
    return (CMPI_ThreadContext*)
        TSDKey::get_thread_specific(globalThreadContextKey.contextKey);
}

inline const CMPIBroker* CMPI_ThreadContext::getBroker()
{
    /**
      return getThreadContext()->broker;
   */
    CMPI_ThreadContext *ctx = getThreadContext();
    if( ctx )
    {
        return ctx->broker;
    }
    return 0;
}

inline const CMPIContext* CMPI_ThreadContext::getContext()
{
    return getThreadContext()->context;
}

PEGASUS_NAMESPACE_END

#endif

