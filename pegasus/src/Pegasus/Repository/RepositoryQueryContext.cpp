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

#include "RepositoryQueryContext.h"

PEGASUS_NAMESPACE_BEGIN

RepositoryQueryContext::RepositoryQueryContext(
    const CIMNamespaceName& inNS,
    CIMRepository* inCIMRep)
    : QueryContext(inNS),
      _CIMRep(inCIMRep)
{
}

RepositoryQueryContext::RepositoryQueryContext(
    const RepositoryQueryContext& ctx)
    : QueryContext(ctx),
      _CIMRep(ctx._CIMRep)
{
}

RepositoryQueryContext::~RepositoryQueryContext()
{
}

RepositoryQueryContext& RepositoryQueryContext::operator=(
    const RepositoryQueryContext& rhs)
{
    if (this == &rhs)
        return *this;

    QueryContext::operator=(rhs);

    _CIMRep = rhs._CIMRep;

    return *this;
}

CIMClass RepositoryQueryContext::getClass(const CIMName& inClassName) const
{
    /* Hardcoded defaults */
    Boolean localOnly = false;
    Boolean includeQualifiers = true;
    Boolean includeClassOrigin = false;
    CIMPropertyList _emptyCIMPropertyList;

    CIMClass _class = _CIMRep->getClass(
        getNamespace(),
        inClassName,
        localOnly,
        includeQualifiers,
        includeClassOrigin,
        _emptyCIMPropertyList);
    return _class;
}

Array<CIMName> RepositoryQueryContext::enumerateClassNames(
    const CIMName& inClassName) const
{
    return _CIMRep->enumerateClassNames(
        getNamespace(),
        inClassName,
        true);          // deepInheritance
}

Boolean RepositoryQueryContext::isSubClass(
    const CIMName& baseClass,
    const CIMName& derivedClass) const
{
    if (baseClass == derivedClass)
    {
        return false;
    }

    Array<CIMName> subClasses = enumerateClassNames(baseClass);
    for (Uint32 i = 0; i < subClasses.size(); i++)
    {
        if (subClasses[i] == derivedClass)
        {
            return true;
        }
    }

    return false;
}

QueryContext::ClassRelation RepositoryQueryContext::getClassRelation(
    const CIMName& anchorClass,
    const CIMName& relatedClass) const
{
    if (anchorClass == relatedClass)
    {
        return SAMECLASS;
    }

    if (isSubClass(anchorClass, relatedClass))
    {
        return SUBCLASS;
    }

    if (isSubClass(relatedClass, anchorClass))
    {
        return SUPERCLASS;
    }

    return NOTRELATED;
}

QueryContext* RepositoryQueryContext::clone() const
{
    return new RepositoryQueryContext(*this);
}

PEGASUS_NAMESPACE_END

