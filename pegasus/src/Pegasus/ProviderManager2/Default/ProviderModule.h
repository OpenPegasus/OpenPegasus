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

#ifndef Pegasus_ProviderModule_h
#define Pegasus_ProviderModule_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/AtomicInt.h>
#include <Pegasus/Common/String.h>
#include <Pegasus/Common/System.h>

#include <Pegasus/General/DynamicLibrary.h>

#include <Pegasus/Provider/CIMProvider.h>
#include <Pegasus/ProviderManager2/Default/Linkage.h>


PEGASUS_NAMESPACE_BEGIN

/** The ProviderModule class represents the physical module, as defined by the
    operating system, that contains one or more providers.
    Also this class in not reentrant.
 */
class PEGASUS_DEFPM_LINKAGE ProviderModule
{
public:
    ProviderModule(const String& fileName);
    virtual ~ProviderModule();

    const String& getFileName() const;

    CIMProvider* load(const String& providerName);
    void unloadModule();

private:
    ProviderModule();    // Unimplemented
    ProviderModule(const ProviderModule& pm);    // Unimplemented
    ProviderModule& operator=(const ProviderModule& pm);    // Unimplemented

    DynamicLibrary _library;
};

inline const String& ProviderModule::getFileName() const
{
    return _library.getFileName();
}

PEGASUS_NAMESPACE_END

#endif
