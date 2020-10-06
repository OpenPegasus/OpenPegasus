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

#include "ProviderModule.h"

#include <Pegasus/Common/FileSystem.h>
#include <Pegasus/Common/MessageLoader.h>

PEGASUS_NAMESPACE_BEGIN

ProviderModule::ProviderModule(const String& fileName)
    : _library(fileName)
{
}

ProviderModule::~ProviderModule()
{
}

CIMProvider* ProviderModule::load(const String& providerName)
{
    // dynamically load the provider library
    if (!_library.load())
    {
        throw Exception(MessageLoaderParms(
            "ProviderManager.ProviderModule.CANNOT_LOAD_LIBRARY",
            "ProviderLoadFailure ($0:$1):Cannot load library, error: $2",
            _library.getFileName(),
            providerName,
            _library.getLoadErrorMessage()));
    }

    // find library entry point
    CIMProvider * (*createProvider)(const String&) =
        (CIMProvider* (*)(const String&))
            _library.getSymbol("PegasusCreateProvider");

    if (createProvider == 0)
    {
        _library.unload();
        throw Exception(MessageLoaderParms(
            "ProviderManager.ProviderModule.ENTRY_POINT_NOT_FOUND",
            "ProviderLoadFailure ($0:$1):entry point not found.",
            _library.getFileName(),
            providerName));
    }

    // invoke the provider entry point
    CIMProvider* provider = createProvider(providerName);

    // test for the appropriate interface
    if (dynamic_cast<CIMProvider *>(provider) == 0)
    {
        _library.unload();
        throw Exception(MessageLoaderParms(
            "ProviderManager.ProviderModule.PROVIDER_IS_NOT_A",
            "ProviderLoadFailure ($0:$1):provider is not a CIMProvider.",
            _library.getFileName(),
            providerName));
    }

    return provider;
}

void ProviderModule::unloadModule()
{
    _library.unload();
}

PEGASUS_NAMESPACE_END
