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

#include "DynamicLibrary.h"
#include <Pegasus/Common/MessageLoader.h>
#include <Pegasus/Common/Tracer.h>

#include <dlfcn.h>

#if defined(PEGASUS_ZOS_SECURITY)
# include <sys/stat.h>
# include "DynamicLibraryzOS_inline.h"
#endif

PEGASUS_NAMESPACE_BEGIN

Boolean DynamicLibrary::_load()
{
    CString cstr = _fileName.getCString();

#if defined(PEGASUS_ZOS_SECURITY)
    if (!hasProgramControl(cstr))
    {
        MessageLoaderParms parms(
            "General.DynamicLibrary.MISSING_PROGRAM_CONTROL_REASON.ZOS",
            "Missing program control flag.");
        _loadErrorMessage = MessageLoader::getMessage(parms);
        return false;
    }
#endif

#if defined(PEGASUS_OS_VMS)
    _handle = dlopen(cstr, RTLD_NOW);
#else
    _handle = dlopen(cstr, RTLD_LAZY | RTLD_GLOBAL);
#endif

    if (_handle == 0)
    {
        // Record the load error message
        _loadErrorMessage = dlerror();
    }

    return isLoaded();
}

void DynamicLibrary::_unload()
{
    if(dlclose(_handle))
    {
        PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL2,
            "dlclose on %s failed with \"%s\"",
            (const char*)_fileName.getCString(),
            dlerror()));
    }
}

DynamicLibrary::DynamicSymbolHandle DynamicLibrary::getSymbol(
    const String& symbolName)
{
    PEGASUS_ASSERT(isLoaded());

#if defined(PEGASUS_OS_VMS)
    String str = symbolName.subString(0, 31);
    CString cstr = str.getCString();
#else
    CString cstr = symbolName.getCString();
#endif

    DynamicSymbolHandle func =
        (DynamicSymbolHandle) dlsym(_handle, (const char *)cstr);

    return func;
}

PEGASUS_NAMESPACE_END
