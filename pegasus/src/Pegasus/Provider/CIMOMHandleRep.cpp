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

#include <string.h>    // for memcpy()
#include <Pegasus/Common/Tracer.h>

#include "CIMOMHandleRep.h"

PEGASUS_NAMESPACE_BEGIN

CIMOMHandleRep::CIMOMHandleRep()
    : _providerUnloadProtect(0)
{
}

CIMOMHandleRep::~CIMOMHandleRep()
{
}

void CIMOMHandleRep::disallowProviderUnload()
{
    PEG_METHOD_ENTER(TRC_CIMOM_HANDLE,
        "CIMOMHandleRep::disallowProviderUnload");

    try
    {
        AutoMutex lock(_providerUnloadProtectMutex);
        _providerUnloadProtect++;
    }
    catch (...)
    {
        // There's not much a provider could do with this exception.  Since
        // this is just a hint, our best bet is to just ignore it.
        PEG_TRACE_CSTRING(TRC_CIMOM_HANDLE, Tracer::LEVEL2,
            "Caught unexpected exception");
    }

    PEG_METHOD_EXIT();
}

void CIMOMHandleRep::allowProviderUnload()
{
    PEG_METHOD_ENTER(TRC_CIMOM_HANDLE,
        "CIMOMHandleRep::allowProviderUnload");

    try
    {
        AutoMutex lock(_providerUnloadProtectMutex);
        if (_providerUnloadProtect > 0)
        {
            _providerUnloadProtect--;
        }
    }
    catch (...)
    {
        // There's not much a provider could do with this exception.  Since
        // this is just a hint, our best bet is to just ignore it.
        PEG_TRACE_CSTRING(TRC_CIMOM_HANDLE, Tracer::LEVEL2,
            "Caught unexpected exception");
    }

    PEG_METHOD_EXIT();
}

Boolean CIMOMHandleRep::unload_ok()
{
    Boolean unloadable = true;

    AutoMutex lock(_providerUnloadProtectMutex);
    if (_providerUnloadProtect > 0)
    {
        unloadable = false;
    }

    return unloadable;
}

PEGASUS_NAMESPACE_END
