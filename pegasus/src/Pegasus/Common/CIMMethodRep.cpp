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

#include <Pegasus/Common/Config.h>
#include "CIMMethod.h"
#include "CIMMethodRep.h"
#include "Resolver.h"
#include "CIMName.h"
#include "CIMScope.h"
#include <Pegasus/Common/MessageLoader.h>
#include "StrLit.h"

PEGASUS_NAMESPACE_BEGIN

CIMMethodRep::CIMMethodRep(const CIMMethodRep& x) :
    _name(x._name),
    _type(x._type),
    _classOrigin(x._classOrigin),
    _propagated(x._propagated),
    _ownerCount(0),
    _refCounter(1)
{
    x._qualifiers.cloneTo(_qualifiers);
    // Set the CIM name tag.
    _nameTag = generateCIMNameTag(_name);

    _parameters.reserveCapacity(x._parameters.size());

    for (Uint32 i = 0, n = x._parameters.size(); i < n; i++)
    {
        _parameters.append(x._parameters[i].clone());
    }
}

CIMMethodRep::CIMMethodRep(
    const CIMName& name,
    CIMType type,
    const CIMName& classOrigin,
    Boolean propagated)
    : _name(name), _type(type),
    _classOrigin(classOrigin),
    _propagated(propagated),
    _ownerCount(0),
    _refCounter(1)
{
    // ensure name is not null
    if (name.isNull())
    {
        throw UninitializedObjectException();
    }
    // Set the CIM name tag.
    _nameTag = generateCIMNameTag(_name);
}

void CIMMethodRep::setName(const CIMName& name)
{
    // ensure name is not null
    if (name.isNull())
    {
        throw UninitializedObjectException();
    }
    if (_ownerCount != 0 && _name != name)
    {
        MessageLoaderParms parms(
            "Common.CIMMethodRep.CONTAINED_METHOD_NAMECHANGEDEXCEPTION",
            "Attempted to change the name of a method"
                " already in a container.");
        throw Exception(parms);
    }
    _name = name;
    // Set the CIM name tag.
    _nameTag = generateCIMNameTag(_name);
}

void CIMMethodRep::addParameter(const CIMParameter& x)
{
    if (x.isUninitialized())
        throw UninitializedObjectException();

    if (findParameter(x.getName()) != PEG_NOT_FOUND)
    {
        MessageLoaderParms parms("Common.CIMMethodRep.PARAMETER",
            "parameter \"$0\"",
            x.getName().getString());
        throw AlreadyExistsException(parms);
    }

    _parameters.append(x);
}

void CIMMethodRep::resolve(
    DeclContext* declContext,
    const CIMNamespaceName& nameSpace,
    const CIMConstMethod& inheritedMethod)
{
    // ATTN: Check to see if this method has same signature as
    // inherited one.

    // Check for type mismatch between return types.

    PEGASUS_ASSERT(!inheritedMethod.isUninitialized());

    // Validate the qualifiers of the method (according to
    // superClass's method with the same name). This method
    // will throw an exception if the validation fails.

    _qualifiers.resolve(
        declContext,
        nameSpace,
        CIMScope::METHOD,
        false,
        inheritedMethod._rep->_qualifiers,
        true);

    // Validate each of the parameters:

    for (Uint32 i = 0; i < _parameters.size(); i++)
        Resolver::resolveParameter (_parameters[i], declContext, nameSpace);

    _classOrigin = inheritedMethod.getClassOrigin();
}

void CIMMethodRep::resolve(
    DeclContext* declContext,
    const CIMNamespaceName& nameSpace)
{
    // Validate the qualifiers:

    CIMQualifierList dummy;

    _qualifiers.resolve(
        declContext,
        nameSpace,
        CIMScope::METHOD,
        false,
        dummy,
        true);

    // Validate each of the parameters:

    for (Uint32 i = 0; i < _parameters.size(); i++)
        Resolver::resolveParameter (_parameters[i], declContext, nameSpace);
}

Boolean CIMMethodRep::identical(const CIMMethodRep* x) const
{
    // If the pointers are the same, the objects must be identical
    if (this == x)
    {
        return true;
    }

    if (!_name.equal (x->_name))
        return false;

    if (_type != x->_type)
        return false;

    if (!_qualifiers.identical(x->_qualifiers))
        return false;

    if (_parameters.size() != x->_parameters.size())
        return false;

    for (Uint32 i = 0, n = _parameters.size(); i < n; i++)
    {
        if (!_parameters[i].identical(x->_parameters[i]))
            return false;
    }

    return true;
}

PEGASUS_NAMESPACE_END
