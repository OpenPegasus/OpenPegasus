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

// This is a sample provider that provides the instance and association
// operations associated with the AssociationTest MOF for the dynamic versions
// of the classes.

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/String.h>

#include "AssociationTestProvider.h"

PEGASUS_NAMESPACE_BEGIN

// This is the dynamic entry point into this dynamic module. The name of
// this provider is "AssociationTestProvider" which is appened to
// "PegasusCreateProvider_" to form a symbol name. This function is called
// by the ProviderModule to load this provider.
//
// NOTE: The name of the provider must be correct to be loadable.

extern "C" PEGASUS_EXPORT CIMProvider* PegasusCreateProvider(const String& name)
{
    CDEBUG("Create AssociationProvider " << name);
    if (String::equalNoCase(name, "associationtestprovider") ||
        String::equalNoCase(name, "associationtestprovider (PROVIDER)"))
    {
        return new AssociationTestProvider();
    }

    return 0;
}

PEGASUS_NAMESPACE_END
