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

#ifndef Pegasus_JMPIProviderModule_h
#define Pegasus_JMPIProviderModule_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/String.h>
#include <Pegasus/Common/System.h>

#include <Pegasus/Provider/CIMProvider.h>
#include <Pegasus/ProviderManager2/JMPI/JMPIProvider.h>
#include <Pegasus/ProviderManager2/JMPI/JMPIImpl.h>

#include <Pegasus/Server/Linkage.h>
#include <Pegasus/ProviderManager2/JMPI/Linkage.h>

PEGASUS_NAMESPACE_BEGIN

// The JMPIProviderModule class represents the physical module, as defined by
// the operation, that contains a provider. This class effectively encapsulates
// the "physical" portion of a provider.


class PEGASUS_JMPIPM_LINKAGE JMPIProviderModule
{

    friend class JMPILocalProviderManager;

public:
    virtual ~JMPIProviderModule(void);
    const String & getFileName(void) const;
    ProviderVector load(const String & providerName);
    void unloadModule(void);

protected:
    String _fileName;
    String _className;
    String _interfaceName;
    void *jProviderClass;
    void *jProvider;

private:
    JMPIProviderModule(const String & fileName, const String & interfaceName);
    const String & getProviderName(void) const;
    const String & getInterfaceName(void) const ;
    virtual CIMProvider * getProvider(void) const;

    String _providerName;
    CIMProvider * _provider;
};

inline const String & JMPIProviderModule::getFileName(void) const
{
    return(_fileName);
}

inline const String & JMPIProviderModule::getInterfaceName(void) const
{
    return(_interfaceName);
}

inline const String & JMPIProviderModule::getProviderName(void) const
{
    return(_providerName);
}

inline CIMProvider * JMPIProviderModule::getProvider(void) const
{
    return(_provider);
}

PEGASUS_NAMESPACE_END

#endif
