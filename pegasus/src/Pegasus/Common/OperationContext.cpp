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

#include "OperationContext.h"
#include "ArrayInternal.h"
#include <Pegasus/Common/MessageLoader.h>

#if defined(PEGASUS_PLATFORM_LINUX_GENERIC_GNU) || \
    defined(PEGASUS_PLATFORM_DARWIN_PPC_GNU) || \
    defined(PEGASUS_PLATFORM_DARWIN_IX86_GNU)
# define PEGASUS_INCLUDE_SUPERCLASS_INITIALIZER
#endif

PEGASUS_NAMESPACE_BEGIN

//
// OperationContext
//

class OperationContextRep
{
public:
    Array<OperationContext::Container*> containers;
};

OperationContext::OperationContext()
{
    _rep = new OperationContextRep;
}

OperationContext::OperationContext(const OperationContext& context)
{
    _rep = new OperationContextRep;
    *this = context;
}

OperationContext::~OperationContext()
{
    clear();
    delete _rep;
}

OperationContext& OperationContext::operator=(const OperationContext& context)
{
    if (this == &context)
    {
        return *this;
    }

    clear();

    for (Uint32 i = 0, n = context._rep->containers.size(); i < n; i++)
    {
        _rep->containers.append(context._rep->containers[i]->clone());
    }

    return *this;
}

void OperationContext::clear()
{
    for (Uint32 i = 0, n = _rep->containers.size(); i < n; i++)
    {
        _rep->containers[i]->destroy();
    }

    _rep->containers.clear();
}

const OperationContext::Container& OperationContext::get(
    const String& containerName) const
{
    Uint32 size = _rep->containers.size();
    Container** data = (Container**)_rep->containers.getData();

    for (; size--; data++)
        if (data[0]->getName() == containerName)
            return *(data[0]);

    static Exception _exception(MessageLoaderParms(
        "Common.OperationContext.OBJECT_NOT_FOUND", "object not found"));

    throw Exception(_exception);
}

Boolean OperationContext::contains(
    const String& containerName) const
{
    Uint32 size = _rep->containers.size();
    Container** data = (Container**)_rep->containers.getData();

    for (; size--; data++)
    {
        if (data[0]->getName() == containerName)
            return true;
    }

    return false;
}

void OperationContext::set(const OperationContext::Container& container)
{
    for (Uint32 i = 0, n = _rep->containers.size(); i < n; i++)
    {
        if (container.getName() == _rep->containers[i]->getName())
        {
            // delete previous container
            _rep->containers[i]->destroy();
            _rep->containers.remove(i);

            // append current container
            _rep->containers.append(container.clone());

            return;
        }
    }

    MessageLoaderParms parms(
        "Common.OperationContext.OBJECT_NOT_FOUND",
        "object not found");
    throw Exception(parms);
}

void OperationContext::insert(const OperationContext::Container& container)
{
    for (Uint32 i = 0, n = _rep->containers.size(); i < n; i++)
    {
        if (container.getName() == _rep->containers[i]->getName())
        {
            MessageLoaderParms parms(
                "Common.OperationContext.OBJECT_ALREADY_EXISTS",
                "object already exists.");
            throw Exception(parms);
        }
    }

    _rep->containers.append(container.clone());
}

void OperationContext::remove(const String& containerName)
{
    for (Uint32 i = 0, n = _rep->containers.size(); i < n; i++)
    {
        if (containerName == _rep->containers[i]->getName())
        {
            _rep->containers[i]->destroy();
            _rep->containers.remove(i);

            return;
        }
    }

    MessageLoaderParms parms(
        "Common.OperationContext.OBJECT_NOT_FOUND",
        "object not found");
    throw Exception(parms);
}


//
// OperationContext::Container
//

OperationContext::Container::~Container()
{
}


//
// IdentityContainer
//

class IdentityContainerRep
{
public:
    String userName;
};

const String IdentityContainer::NAME = "IdentityContainer";

IdentityContainer::IdentityContainer(
    const OperationContext::Container& container)
{
    const IdentityContainer* p =
        dynamic_cast<const IdentityContainer*>(&container);

    if (p == 0)
    {
        throw DynamicCastFailedException();
    }

    _rep = new IdentityContainerRep();
    _rep->userName = p->_rep->userName;
}

IdentityContainer::IdentityContainer(const IdentityContainer& container)
#ifdef PEGASUS_INCLUDE_SUPERCLASS_INITIALIZER
    : OperationContext::Container()
#endif
{
    _rep = new IdentityContainerRep();
    _rep->userName = container._rep->userName;
}

IdentityContainer::IdentityContainer(const String& userName)
{
    _rep = new IdentityContainerRep();
    _rep->userName = userName;
}

IdentityContainer::~IdentityContainer()
{
    delete _rep;
}

IdentityContainer& IdentityContainer::operator=(
    const IdentityContainer& container)
{
    if (this == &container)
    {
        return *this;
    }

    _rep->userName = container._rep->userName;

    return *this;
}

String IdentityContainer::getName() const
{
    return NAME;
}

OperationContext::Container* IdentityContainer::clone() const
{
    return new IdentityContainer(_rep->userName);
}

void IdentityContainer::destroy()
{
    delete this;
}

String IdentityContainer::getUserName() const
{
    return _rep->userName;
}


//
// SubscriptionInstanceContainer
//

class SubscriptionInstanceContainerRep
{
public:
    CIMInstance subscriptionInstance;
};

const String SubscriptionInstanceContainer::NAME =
    "SubscriptionInstanceContainer";

SubscriptionInstanceContainer::SubscriptionInstanceContainer(
    const OperationContext::Container& container)
{
    const SubscriptionInstanceContainer* p =
        dynamic_cast<const SubscriptionInstanceContainer*>(&container);

    if (p == 0)
    {
        throw DynamicCastFailedException();
    }

    _rep = new SubscriptionInstanceContainerRep();
    _rep->subscriptionInstance = p->_rep->subscriptionInstance;
}

SubscriptionInstanceContainer::SubscriptionInstanceContainer(
    const SubscriptionInstanceContainer& container)
#ifdef PEGASUS_INCLUDE_SUPERCLASS_INITIALIZER
    : OperationContext::Container()
#endif
{
    _rep = new SubscriptionInstanceContainerRep();
    _rep->subscriptionInstance = container._rep->subscriptionInstance;
}

SubscriptionInstanceContainer::SubscriptionInstanceContainer(
    const CIMInstance& subscriptionInstance)
{
    _rep = new SubscriptionInstanceContainerRep();
    _rep->subscriptionInstance = subscriptionInstance;
}

SubscriptionInstanceContainer::~SubscriptionInstanceContainer()
{
    delete _rep;
}

SubscriptionInstanceContainer& SubscriptionInstanceContainer::operator=(
    const SubscriptionInstanceContainer& container)
{
    if (this == &container)
    {
        return *this;
    }

    _rep->subscriptionInstance = container._rep->subscriptionInstance;

    return *this;
}

String SubscriptionInstanceContainer::getName() const
{
    return NAME;
}

OperationContext::Container* SubscriptionInstanceContainer::clone() const
{
    return new SubscriptionInstanceContainer(_rep->subscriptionInstance);
}

void SubscriptionInstanceContainer::destroy()
{
    delete this;
}

CIMInstance SubscriptionInstanceContainer::getInstance() const
{
    return _rep->subscriptionInstance;
}


//
// SubscriptionInstanceNamesContainer
//

class SubscriptionInstanceNamesContainerRep
{
public:
    Array<CIMObjectPath> subscriptionInstanceNames;
};

const String SubscriptionInstanceNamesContainer::NAME =
    "SubscriptionInstanceNamesContainer";

SubscriptionInstanceNamesContainer::SubscriptionInstanceNamesContainer(
    const OperationContext::Container& container)
{
    const SubscriptionInstanceNamesContainer* p =
        dynamic_cast<const SubscriptionInstanceNamesContainer*>(&container);

    if (p == 0)
    {
        throw DynamicCastFailedException();
    }

    _rep = new SubscriptionInstanceNamesContainerRep();
    _rep->subscriptionInstanceNames = p->_rep->subscriptionInstanceNames;
}

SubscriptionInstanceNamesContainer::SubscriptionInstanceNamesContainer(
    const SubscriptionInstanceNamesContainer& container)
#ifdef PEGASUS_INCLUDE_SUPERCLASS_INITIALIZER
    : OperationContext::Container()
#endif
{
    _rep = new SubscriptionInstanceNamesContainerRep();
    _rep->subscriptionInstanceNames = container._rep->subscriptionInstanceNames;
}

SubscriptionInstanceNamesContainer::SubscriptionInstanceNamesContainer(
    const Array<CIMObjectPath>& subscriptionInstanceNames)
{
    _rep = new SubscriptionInstanceNamesContainerRep();
    _rep->subscriptionInstanceNames = subscriptionInstanceNames;
}

SubscriptionInstanceNamesContainer::~SubscriptionInstanceNamesContainer()
{
    delete _rep;
}

SubscriptionInstanceNamesContainer&
    SubscriptionInstanceNamesContainer::operator=(
        const SubscriptionInstanceNamesContainer& container)
{
    if (this == &container)
    {
        return *this;
    }

    _rep->subscriptionInstanceNames = container._rep->subscriptionInstanceNames;

    return *this;
}

String SubscriptionInstanceNamesContainer::getName() const
{
    return NAME;
}

OperationContext::Container*
    SubscriptionInstanceNamesContainer::clone() const
{
    return new SubscriptionInstanceNamesContainer(
        _rep->subscriptionInstanceNames);
}

void SubscriptionInstanceNamesContainer::destroy()
{
    delete this;
}

Array<CIMObjectPath>
    SubscriptionInstanceNamesContainer::getInstanceNames() const
{
    return _rep->subscriptionInstanceNames;
}


//
// SubscriptionFilterConditionContainer
//

class SubscriptionFilterConditionContainerRep
{
public:
    String filterCondition;
    String queryLanguage;
};

const String SubscriptionFilterConditionContainer::NAME =
    "SubscriptionFilterConditionContainer";

SubscriptionFilterConditionContainer::SubscriptionFilterConditionContainer(
    const OperationContext::Container& container)
{
    const SubscriptionFilterConditionContainer* p =
        dynamic_cast<const SubscriptionFilterConditionContainer*>(&container);

    if (p == 0)
    {
        throw DynamicCastFailedException();
    }

    _rep = new SubscriptionFilterConditionContainerRep();
    _rep->filterCondition = p->_rep->filterCondition;
    _rep->queryLanguage = p->_rep->queryLanguage;
}

SubscriptionFilterConditionContainer::SubscriptionFilterConditionContainer(
    const SubscriptionFilterConditionContainer& container)
#ifdef PEGASUS_INCLUDE_SUPERCLASS_INITIALIZER
    : OperationContext::Container()
#endif
{
    _rep = new SubscriptionFilterConditionContainerRep();
    _rep->filterCondition = container._rep->filterCondition;
    _rep->queryLanguage = container._rep->queryLanguage;
}

SubscriptionFilterConditionContainer::SubscriptionFilterConditionContainer(
    const String& filterCondition,
    const String& queryLanguage)
{
    _rep = new SubscriptionFilterConditionContainerRep();
    _rep->filterCondition = filterCondition;
    _rep->queryLanguage = queryLanguage;
}

SubscriptionFilterConditionContainer::~SubscriptionFilterConditionContainer()
{
    delete _rep;
}

SubscriptionFilterConditionContainer&
    SubscriptionFilterConditionContainer::operator=(
        const SubscriptionFilterConditionContainer& container)
{
    if (this == &container)
    {
        return *this;
    }

    _rep->filterCondition = container._rep->filterCondition;
    _rep->queryLanguage = container._rep->queryLanguage;

    return *this;
}

String SubscriptionFilterConditionContainer::getName() const
{
    return NAME;
}

OperationContext::Container* SubscriptionFilterConditionContainer::clone() const
{
    return new SubscriptionFilterConditionContainer(
        _rep->filterCondition, _rep->queryLanguage);
}

void SubscriptionFilterConditionContainer::destroy()
{
    delete this;
}

String SubscriptionFilterConditionContainer::getFilterCondition() const
{
    return _rep->filterCondition;
}

String SubscriptionFilterConditionContainer::getQueryLanguage() const
{
    return _rep->queryLanguage;
}


//
// SubscriptionFilterQueryContainer
//

class SubscriptionFilterQueryContainerRep
{
public:
  String filterQuery;
  String queryLanguage;
  CIMNamespaceName sourceNameSpace;
};

const String SubscriptionFilterQueryContainer::NAME =
    "SubscriptionFilterQueryContainer";

SubscriptionFilterQueryContainer::SubscriptionFilterQueryContainer(
    const OperationContext::Container& container)
{
    const SubscriptionFilterQueryContainer* p =
        dynamic_cast<const SubscriptionFilterQueryContainer*>(&container);

    if (p == 0)
    {
        throw DynamicCastFailedException();
    }

    _rep = new SubscriptionFilterQueryContainerRep();
    _rep->filterQuery = p->_rep->filterQuery;
    _rep->queryLanguage = p->_rep->queryLanguage;
    _rep->sourceNameSpace = p->_rep->sourceNameSpace;
}

SubscriptionFilterQueryContainer::SubscriptionFilterQueryContainer(
    const SubscriptionFilterQueryContainer& container)
#ifdef PEGASUS_INCLUDE_SUPERCLASS_INITIALIZER
    : OperationContext::Container()
#endif
{
    _rep = new SubscriptionFilterQueryContainerRep();
    _rep->filterQuery = container._rep->filterQuery;
    _rep->queryLanguage = container._rep->queryLanguage;
    _rep->sourceNameSpace = container._rep->sourceNameSpace;
}

SubscriptionFilterQueryContainer::SubscriptionFilterQueryContainer(
    const String& filterQuery,
    const String& queryLanguage,
    const CIMNamespaceName& sourceNameSpace)
{
    _rep = new SubscriptionFilterQueryContainerRep();
    _rep->filterQuery = filterQuery;
    _rep->queryLanguage = queryLanguage;
    _rep->sourceNameSpace = sourceNameSpace;
}

SubscriptionFilterQueryContainer::~SubscriptionFilterQueryContainer()
{
    delete _rep;
}

SubscriptionFilterQueryContainer&
    SubscriptionFilterQueryContainer::operator=(
        const SubscriptionFilterQueryContainer& container)
{
    if (this == &container)
    {
        return *this;
    }

    _rep->filterQuery = container._rep->filterQuery;
    _rep->queryLanguage = container._rep->queryLanguage;
    _rep->sourceNameSpace = container._rep->sourceNameSpace;

    return *this;
}

String SubscriptionFilterQueryContainer::getName() const
{
    return NAME;
}

OperationContext::Container* SubscriptionFilterQueryContainer::clone() const
{
    return new SubscriptionFilterQueryContainer(
        _rep->filterQuery,
        _rep->queryLanguage,
        _rep->sourceNameSpace);
}

void SubscriptionFilterQueryContainer::destroy()
{
    delete this;
}

String SubscriptionFilterQueryContainer::getFilterQuery() const
{
    return _rep->filterQuery;
}

String SubscriptionFilterQueryContainer::getQueryLanguage() const
{
    return _rep->queryLanguage;
}

CIMNamespaceName SubscriptionFilterQueryContainer::getSourceNameSpace() const
{
    return _rep->sourceNameSpace;
}


//
// TimeoutContainer
//

const String TimeoutContainer::NAME = "TimeoutContainer";

TimeoutContainer::TimeoutContainer(const OperationContext::Container& container)
{
    const TimeoutContainer* p =
        dynamic_cast<const TimeoutContainer*>(&container);
    if (p == 0)
    {
        throw DynamicCastFailedException();
    }
    _value = p->_value;
}

TimeoutContainer::TimeoutContainer(Uint32 timeout)
{
    _value = timeout;
}

String TimeoutContainer::getName() const
{
    return NAME;
}

OperationContext::Container* TimeoutContainer::clone() const
{
    return new TimeoutContainer(_value);
}

void TimeoutContainer::destroy()
{
    delete this;
}

Uint32 TimeoutContainer::getTimeOut() const
{
    return _value;
}


//
// AcceptLanguageListContainer
//

class AcceptLanguageListContainerRep
{
public:
    AcceptLanguageList languages;
};

const String AcceptLanguageListContainer::NAME =
    "AcceptLanguageListContainer";

AcceptLanguageListContainer::AcceptLanguageListContainer(
    const OperationContext::Container& container)
{
    const AcceptLanguageListContainer* p =
        dynamic_cast<const AcceptLanguageListContainer*>(&container);

    if (p == 0)
    {
        throw DynamicCastFailedException();
    }

    _rep = new AcceptLanguageListContainerRep();
    _rep->languages = p->_rep->languages;
}

AcceptLanguageListContainer::AcceptLanguageListContainer(
    const AcceptLanguageListContainer& container)
#ifdef PEGASUS_INCLUDE_SUPERCLASS_INITIALIZER
    : OperationContext::Container()
#endif
{
    _rep = new AcceptLanguageListContainerRep();
    _rep->languages = container._rep->languages;
}

AcceptLanguageListContainer::AcceptLanguageListContainer(
    const AcceptLanguageList& languages)
{
    _rep = new AcceptLanguageListContainerRep();
    _rep->languages = languages;
}

AcceptLanguageListContainer::~AcceptLanguageListContainer()
{
    delete _rep;
}

AcceptLanguageListContainer& AcceptLanguageListContainer::operator=(
    const AcceptLanguageListContainer& container)
{
    if (this == &container)
    {
        return *this;
    }

    _rep->languages = container._rep->languages;

    return *this;
}

String AcceptLanguageListContainer::getName() const
{
    return NAME;
}

OperationContext::Container* AcceptLanguageListContainer::clone() const
{
    return new AcceptLanguageListContainer(_rep->languages);
}

void AcceptLanguageListContainer::destroy()
{
    delete this;
}

AcceptLanguageList AcceptLanguageListContainer::getLanguages() const
{
    return _rep->languages;
}


//
// ContentLanguageListContainer
//

class ContentLanguageListContainerRep
{
public:
    ContentLanguageList languages;
};

const String ContentLanguageListContainer::NAME =
    "ContentLanguageListContainer";

ContentLanguageListContainer::ContentLanguageListContainer(
    const OperationContext::Container& container)
{
    const ContentLanguageListContainer* p =
        dynamic_cast<const ContentLanguageListContainer*>(&container);

    if (p == 0)
    {
        throw DynamicCastFailedException();
    }

    _rep = new ContentLanguageListContainerRep();
    _rep->languages = p->_rep->languages;
}

ContentLanguageListContainer::ContentLanguageListContainer(
    const ContentLanguageListContainer& container)
#ifdef PEGASUS_INCLUDE_SUPERCLASS_INITIALIZER
    : OperationContext::Container()
#endif
{
    _rep = new ContentLanguageListContainerRep();
    _rep->languages = container._rep->languages;
}

ContentLanguageListContainer::ContentLanguageListContainer(
    const ContentLanguageList& languages)
{
    _rep = new ContentLanguageListContainerRep();
    _rep->languages = languages;
}

ContentLanguageListContainer::~ContentLanguageListContainer()
{
    delete _rep;
}

ContentLanguageListContainer& ContentLanguageListContainer::operator=(
    const ContentLanguageListContainer& container)
{
    if (this == &container)
    {
        return *this;
    }

    _rep->languages = container._rep->languages;

    return *this;
}

String ContentLanguageListContainer::getName() const
{
    return NAME;
}

OperationContext::Container* ContentLanguageListContainer::clone() const
{
    return new ContentLanguageListContainer(_rep->languages);
}

void ContentLanguageListContainer::destroy()
{
    delete this;
}

ContentLanguageList ContentLanguageListContainer::getLanguages() const
{
    return _rep->languages;
}


//
// SnmpTrapOidContainer
//

class SnmpTrapOidContainerRep
{
public:
    String snmpTrapOid;
};

const String SnmpTrapOidContainer::NAME =
    "SnmpTrapOidContainer";

SnmpTrapOidContainer::SnmpTrapOidContainer(
    const OperationContext::Container& container)
{
    const SnmpTrapOidContainer* p =
        dynamic_cast<const SnmpTrapOidContainer*>(&container);

    if (p == 0)
    {
        throw DynamicCastFailedException();
    }

    _rep = new SnmpTrapOidContainerRep();
    _rep->snmpTrapOid = p->_rep->snmpTrapOid;
}

SnmpTrapOidContainer::SnmpTrapOidContainer(
    const SnmpTrapOidContainer& container)
#ifdef PEGASUS_INCLUDE_SUPERCLASS_INITIALIZER
    : OperationContext::Container()
#endif
{
    _rep = new SnmpTrapOidContainerRep();
    _rep->snmpTrapOid = container._rep->snmpTrapOid;
}

SnmpTrapOidContainer::SnmpTrapOidContainer(const String& snmpTrapOid)
{
    _rep = new SnmpTrapOidContainerRep();
    _rep->snmpTrapOid = snmpTrapOid;
}

SnmpTrapOidContainer::~SnmpTrapOidContainer()
{
    delete _rep;
}

SnmpTrapOidContainer& SnmpTrapOidContainer::operator=(
    const SnmpTrapOidContainer& container)
{
    if (this == &container)
    {
        return *this;
    }

    _rep->snmpTrapOid = container._rep->snmpTrapOid;

    return *this;
}

String SnmpTrapOidContainer::getName() const
{
    return NAME;
}

OperationContext::Container* SnmpTrapOidContainer::clone() const
{
    return new SnmpTrapOidContainer(_rep->snmpTrapOid);
}

void SnmpTrapOidContainer::destroy()
{
    delete this;
}

String SnmpTrapOidContainer::getSnmpTrapOid() const
{
    return _rep->snmpTrapOid;
}

PEGASUS_NAMESPACE_END
