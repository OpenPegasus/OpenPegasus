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
// EventDispatcher.h : Declaration of the CEventDispatcher
//
//////////////////////////////////////////////////////////////////////

#ifndef __EVENTDISPATCHER_H_
#define __EVENTDISPATCHER_H_

#include "resource.h"       // main symbols

/////////////////////////////////////////////////////////////////////////////
// CEventDispatcher
class ATL_NO_VTABLE CEventDispatcher :
    public CComObjectRootEx<CComMultiThreadModel>,
    public CComCoClass<CEventDispatcher, &CLSID_EventDispatcher>,
    public IWbemUnboundObjectSink
{

public:
    CEventDispatcher()
    {
    }

DECLARE_REGISTRY_RESOURCEID(IDR_EVENTDISPATCHER)
DECLARE_NOT_AGGREGATABLE(CEventDispatcher)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CEventDispatcher)
    COM_INTERFACE_ENTRY(IWbemUnboundObjectSink)
END_COM_MAP()

// IWbemUnboundObjectSink
public:
    STDMETHOD(IndicateToConsumer)(
        IWbemClassObject *pLogicalConsumer, 
        LONG lNumObjects, 
        IWbemClassObject **apObjects);
};

#endif //__EVENTDISPATCHER_H_
