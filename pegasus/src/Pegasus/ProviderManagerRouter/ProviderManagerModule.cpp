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

#include "ProviderManagerModule.h"

PEGASUS_NAMESPACE_BEGIN

ProviderManagerModule::ProviderManagerModule()
    : DynamicLibrary(),
      _createProviderManager(0)
{
}

ProviderManagerModule::ProviderManagerModule(
    const ProviderManagerModule& module)
    : DynamicLibrary(module),
      _createProviderManager(module._createProviderManager)
{
}

ProviderManagerModule::ProviderManagerModule(
    const String& fileName)
    : DynamicLibrary(fileName),
      _createProviderManager(0)
{
}

ProviderManagerModule::~ProviderManagerModule()
{
}

ProviderManagerModule& ProviderManagerModule::operator=(
    const ProviderManagerModule& module)
{
    if (this == &module)
    {
        return *this;
    }

    DynamicLibrary::operator=(module);

    _createProviderManager = module._createProviderManager;

    return *this;
}

Boolean ProviderManagerModule::load()
{
    if (DynamicLibrary::load())
    {
        // export entry points
        _createProviderManager = (CREATE_PROVIDER_MANAGER_FUNCTION)
            getSymbol("PegasusCreateProviderManager");

        if (_createProviderManager != 0)
        {
            return true;
        }

        DynamicLibrary::unload();
    }

    return false;
}

void ProviderManagerModule::unload()
{
    DynamicLibrary::unload();
}

ProviderManager* ProviderManagerModule::getProviderManager(
    const String& s) const
{
    if (!isLoaded())
    {
        return 0;
    }

    return _createProviderManager(s);
}

PEGASUS_NAMESPACE_END
