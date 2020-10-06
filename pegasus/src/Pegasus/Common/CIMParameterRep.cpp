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
#include <cstdio>
#include "CIMParameter.h"
#include "CIMParameterRep.h"
#include "CIMName.h"
#include "CIMScope.h"
#include "StrLit.h"

PEGASUS_NAMESPACE_BEGIN

CIMParameterRep::CIMParameterRep(const CIMParameterRep& x) :
    _name(x._name),
    _type(x._type),
    _isArray(x._isArray),
    _arraySize(x._arraySize),
    _referenceClassName(x._referenceClassName),
    _refCounter(1),
    _ownerCount(0)
{
    x._qualifiers.cloneTo(_qualifiers);
    // Set the CIM name tag.
    _nameTag = generateCIMNameTag(_name);
}

CIMParameterRep::CIMParameterRep(
    const CIMName& name,
    CIMType type,
    Boolean isArray,
    Uint32 arraySize,
    const CIMName& referenceClassName)
    : _name(name), _type(type),
    _isArray(isArray), _arraySize(arraySize),
    _referenceClassName(referenceClassName),
    _refCounter(1),
    _ownerCount(0)
{
    // ensure name is not null
    if (name.isNull())
    {
        throw UninitializedObjectException();
    }
    // Set the CIM name tag.
    _nameTag = generateCIMNameTag(_name);

    if ((_arraySize != 0) && !_isArray)
    {
        throw TypeMismatchException();
    }

    if (!referenceClassName.isNull())
    {
        if (_type != CIMTYPE_REFERENCE)
        {
            throw TypeMismatchException();
        }
    }
    else
    {
        if (_type == CIMTYPE_REFERENCE)
        {
            throw TypeMismatchException();
        }
    }
}

void CIMParameterRep::setName(const CIMName& name)
{
    // ensure name is not null
    if (name.isNull())
    {
        throw UninitializedObjectException();
    }
    if (_ownerCount != 0 && _name != name)
    {
        MessageLoaderParms parms(
            "Common.CIMParameterRep.CONTAINED_PARAMETER_NAMECHANGEDEXCEPTION",
            "Attempted to change the name of a parameter"
                " already in a container.");
        throw Exception(parms);
    }
    _name = name;
    // Set the CIM name tag.
    _nameTag = generateCIMNameTag(_name);
}

void CIMParameterRep::resolve(
    DeclContext* declContext,
    const CIMNamespaceName& nameSpace)
{
    // Validate the qualifiers of the method (according to
    // superClass's method with the same name). This method
    // will throw an exception if the validation fails.

    CIMQualifierList dummy;

    _qualifiers.resolve(
        declContext,
        nameSpace,
        CIMScope::PARAMETER,
        false,
        dummy,
        true);
}

Boolean CIMParameterRep::identical(const CIMParameterRep* x) const
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

    if (!_referenceClassName.equal (x->_referenceClassName))
        return false;

    if (!_qualifiers.identical(x->_qualifiers))
        return false;

    return true;
}

PEGASUS_NAMESPACE_END
