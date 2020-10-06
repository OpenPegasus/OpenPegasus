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

#include "OperationContextInternal.h"

#if defined(PEGASUS_PLATFORM_LINUX_GENERIC_GNU) || \
    defined(PEGASUS_PLATFORM_DARWIN_PPC_GNU) || \
    defined(PEGASUS_PLATFORM_DARWIN_IX86_GNU)
# define PEGASUS_INCLUDE_SUPERCLASS_INITIALIZER
#endif

PEGASUS_NAMESPACE_BEGIN

//
// LocaleContainer
//

const String LocaleContainer::NAME = "LocaleContainer";

LocaleContainer::LocaleContainer(const OperationContext::Container& container)
{
    const LocaleContainer* p = dynamic_cast<const LocaleContainer*>(&container);

    if (p == 0)
    {
        throw DynamicCastFailedException();
    }

    *this = *p;
}

LocaleContainer::LocaleContainer(const String& languageId)
{
    _languageId = languageId;
}

LocaleContainer::~LocaleContainer()
{
}

LocaleContainer& LocaleContainer::operator=(const LocaleContainer&container)
{
    if (this == &container)
    {
        return *this;
    }

    _languageId = container._languageId;

    return *this;
}

String LocaleContainer::getName() const
{
    return NAME;
}

OperationContext::Container* LocaleContainer::clone() const
{
    return new LocaleContainer(*this);
}

void LocaleContainer::destroy()
{
    delete this;
}

String LocaleContainer::getLanguageId() const
{
    return _languageId;
}

//
// ProviderIdContainer
//

const String ProviderIdContainer::NAME = "ProviderIdContainer";

ProviderIdContainer::ProviderIdContainer(
    const OperationContext::Container& container)
{
    const ProviderIdContainer* p =
        dynamic_cast<const ProviderIdContainer*>(&container);

    if (p == 0)
    {
        throw DynamicCastFailedException();
    }

    *this = *p;
}

ProviderIdContainer::ProviderIdContainer(
    const CIMInstance& module,
    const CIMInstance& provider,
    Boolean isRemoteNameSpace,
    const String& remoteInfo)
    : _module(module),
    _provider(provider),
    _isRemoteNameSpace(isRemoteNameSpace),
    _remoteInfo(remoteInfo)
{
}

ProviderIdContainer::~ProviderIdContainer()
{
}

ProviderIdContainer& ProviderIdContainer::operator=(
    const ProviderIdContainer& container)
{
    if (this == &container)
    {
        return *this;
    }

    _module = container._module;
    _provider = container._provider;
    _isRemoteNameSpace = container._isRemoteNameSpace;
    _remoteInfo = container._remoteInfo;
    _provMgrPath = container._provMgrPath;

    return *this;
}

String ProviderIdContainer::getName() const
{
    return NAME;
}

OperationContext::Container* ProviderIdContainer::clone() const
{
    return new ProviderIdContainer(*this);
}

void ProviderIdContainer::destroy()
{
    delete this;
}

CIMInstance ProviderIdContainer::getModule() const
{
    return _module;
}

CIMInstance ProviderIdContainer::getProvider() const
{
    return _provider;
}

Boolean ProviderIdContainer::isRemoteNameSpace() const
{
    return _isRemoteNameSpace;
}

String ProviderIdContainer::getRemoteInfo() const
{
    return _remoteInfo;
}

String ProviderIdContainer::getProvMgrPath() const
{
    return _provMgrPath;
}

void ProviderIdContainer::setProvMgrPath(const String &path)
{
    _provMgrPath = path;
}

//
// CachedClassDefinitionContainer
//

const String CachedClassDefinitionContainer::NAME =
    "CachedClassDefinitionContainer";

CachedClassDefinitionContainer::CachedClassDefinitionContainer(
    const OperationContext::Container& container)
{
    const CachedClassDefinitionContainer* p =
        dynamic_cast<const CachedClassDefinitionContainer*>(&container);

    if (p == 0)
    {
        throw DynamicCastFailedException();
    }

    *this = *p;
}


CachedClassDefinitionContainer::CachedClassDefinitionContainer(
    const CIMConstClass& cimClass)
    : _cimClass(cimClass)
{
}

CachedClassDefinitionContainer::~CachedClassDefinitionContainer()
{
}

CachedClassDefinitionContainer& CachedClassDefinitionContainer::operator=(
    const CachedClassDefinitionContainer& container)
{
    if (this == &container)
    {
        return *this;
    }

    _cimClass = container._cimClass;

    return *this;
}

String CachedClassDefinitionContainer::getName() const
{
    return NAME;
}

OperationContext::Container* CachedClassDefinitionContainer::clone() const
{
    return new CachedClassDefinitionContainer(*this);
}

void CachedClassDefinitionContainer::destroy()
{
    delete this;
}

CIMConstClass CachedClassDefinitionContainer::getClass() const
{
    return _cimClass;
}

//
// NormalizerContextContainer
//

const String NormalizerContextContainer::NAME = "NormalizerContextContainer";

NormalizerContextContainer::NormalizerContextContainer(
    const OperationContext::Container& container)
{
    const NormalizerContextContainer* p =
        dynamic_cast<const NormalizerContextContainer*>(&container);

    if (p == 0)
    {
        throw DynamicCastFailedException();
    }

    *this = *p;
}


NormalizerContextContainer::NormalizerContextContainer(
    AutoPtr<NormalizerContext>& context) : normalizerContext(context.get())
{
  context.release();
}

NormalizerContextContainer::NormalizerContextContainer(
    const NormalizerContextContainer& container)
#ifdef PEGASUS_INCLUDE_SUPERCLASS_INITIALIZER
    : OperationContext::Container()
#endif
{
    if (this != &container)
    {
        normalizerContext.reset(container.normalizerContext->clone());
    }
}

NormalizerContextContainer::~NormalizerContextContainer()
{
}

NormalizerContextContainer& NormalizerContextContainer::operator=(
    const NormalizerContextContainer& container)
{
    if (this == &container)
    {
        return *this;
    }

    normalizerContext.reset(container.normalizerContext->clone());

    return *this;
}

String NormalizerContextContainer::getName() const
{
    return NAME;
}

OperationContext::Container* NormalizerContextContainer::clone() const
{
    return new NormalizerContextContainer(*this);
}

void NormalizerContextContainer::destroy()
{
    delete this;
}

NormalizerContext* NormalizerContextContainer::getContext() const
{
    return normalizerContext.get();
}

//
// UserRoleContainer
//

const String UserRoleContainer::NAME = "UserRoleContainer";

UserRoleContainer::UserRoleContainer(
    const OperationContext::Container& container)
{
    const UserRoleContainer* p =
        dynamic_cast<const UserRoleContainer*>(&container);

    if (p == 0)
    {
        throw DynamicCastFailedException();
    }

    *this = *p;
}

UserRoleContainer::UserRoleContainer(const String& userRole)
{
    _userRole = userRole;
}

UserRoleContainer::~UserRoleContainer()
{
}

UserRoleContainer& UserRoleContainer::operator=(
    const UserRoleContainer&container)
{
    if (this == &container)
    {
        return *this;
    }

    _userRole = container._userRole;

    return *this;
}

String UserRoleContainer::getName() const
{
    return NAME;
}

OperationContext::Container* UserRoleContainer::clone() const
{
    return new UserRoleContainer(*this);
}

void UserRoleContainer::destroy()
{
    delete this;
}

String UserRoleContainer::getUserRole() const
{
    return _userRole;
}


PEGASUS_NAMESPACE_END
