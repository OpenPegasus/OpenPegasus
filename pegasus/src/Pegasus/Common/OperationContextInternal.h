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

#ifndef Pegasus_OperationContextInternal_h
#define Pegasus_OperationContextInternal_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/Linkage.h>
#include <Pegasus/Common/OperationContext.h>
#include <Pegasus/Common/CIMClass.h>
#include <Pegasus/Common/CIMInstance.h>
#include <Pegasus/Common/AutoPtr.h>
#include <Pegasus/Common/ObjectNormalizer.h>

PEGASUS_NAMESPACE_BEGIN

// This class should be moved to the OperationContext module when localization
// support is added to Pegasus and this class definition is firm.
class PEGASUS_COMMON_LINKAGE LocaleContainer
    : virtual public OperationContext::Container
{
public:
    static const String NAME;

    LocaleContainer(const OperationContext::Container& container);
    LocaleContainer(const String& languageId);
    virtual ~LocaleContainer();

    // NOTE: The compiler default implementation of the copy constructor
    // is used for this class.

    LocaleContainer& operator=(const LocaleContainer& container);

    virtual String getName() const;
    virtual OperationContext::Container* clone() const;
    virtual void destroy();

    String getLanguageId() const;

protected:
    String _languageId;

};

class PEGASUS_COMMON_LINKAGE ProviderIdContainer
    : virtual public OperationContext::Container
{
public:
    static const String NAME;

    ProviderIdContainer(const OperationContext::Container& container);
    ProviderIdContainer(
        const CIMInstance& module,
        const CIMInstance& provider,
        Boolean isRemoteNameSpace = false,
        const String& remoteInfo = String::EMPTY);
    virtual ~ProviderIdContainer();

    // NOTE: The compiler default implementation of the copy constructor
    // is used for this class.

    ProviderIdContainer& operator=(const ProviderIdContainer& container);

    virtual String getName() const;
    virtual OperationContext::Container* clone() const;
    virtual void destroy();

    CIMInstance getModule() const;
    CIMInstance getProvider() const;
    Boolean isRemoteNameSpace() const;
    String getRemoteInfo() const;
    String getProvMgrPath() const;
    void setProvMgrPath(const String &path);

protected:
    CIMInstance _module;
    CIMInstance _provider;
    Boolean _isRemoteNameSpace;
    String _remoteInfo;
    String _provMgrPath;

};

class PEGASUS_COMMON_LINKAGE CachedClassDefinitionContainer
    : virtual public OperationContext::Container
{
public:
    static const String NAME;

    CachedClassDefinitionContainer(
        const OperationContext::Container& container);
    CachedClassDefinitionContainer(const CIMConstClass& cimClass);
    virtual ~CachedClassDefinitionContainer();

    // NOTE: The compiler default implementation of the copy constructor
    // is used for this class.

    CachedClassDefinitionContainer& operator=(
        const CachedClassDefinitionContainer& container);

    virtual String getName() const;
    virtual OperationContext::Container* clone() const;
    virtual void destroy();

    CIMConstClass getClass() const;

protected:
    CIMConstClass _cimClass;

};

class PEGASUS_COMMON_LINKAGE NormalizerContextContainer
    : virtual public OperationContext::Container
{
public:
    static const String NAME;

    NormalizerContextContainer(const OperationContext::Container& container);
    NormalizerContextContainer(AutoPtr<NormalizerContext>& context);
    NormalizerContextContainer(const NormalizerContextContainer& container);
    virtual ~NormalizerContextContainer();

    NormalizerContextContainer& operator=(
        const NormalizerContextContainer& container);

    virtual String getName() const;
    virtual OperationContext::Container* clone() const;
    virtual void destroy();

    NormalizerContext* getContext() const;

protected:
    AutoPtr<NormalizerContext> normalizerContext;

private:
    NormalizerContextContainer();
};

class PEGASUS_COMMON_LINKAGE UserRoleContainer
    : virtual public OperationContext::Container
{
public:
    static const String NAME;

    UserRoleContainer(const OperationContext::Container& container);
    UserRoleContainer(const String& userRole);
    
    virtual ~UserRoleContainer();

    UserRoleContainer& operator=(
        const UserRoleContainer& container);

    virtual String getName() const;
    virtual OperationContext::Container* clone() const;
    virtual void destroy();

    String getUserRole() const;

protected:
    String _userRole;

private:
    UserRoleContainer();
};



PEGASUS_NAMESPACE_END

#endif
