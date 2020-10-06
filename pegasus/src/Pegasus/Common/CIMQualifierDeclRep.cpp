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

#include <cstdio>
#include "CIMQualifierDecl.h"
#include "CIMQualifierDeclRep.h"
#include "CIMName.h"
#include "InternalException.h"
#include "StrLit.h"

PEGASUS_NAMESPACE_BEGIN
PEGASUS_USING_STD;
////////////////////////////////////////////////////////////////////////////////
//
// CIMQualifierDeclRep
//
////////////////////////////////////////////////////////////////////////////////

CIMQualifierDeclRep::CIMQualifierDeclRep(
    const CIMName& name,
    const CIMValue& value,
    const CIMScope & scope,
    const CIMFlavor & flavor,
    Uint32 arraySize)
    :
    _name(name),
    _value(value),
    _scope(scope),
    _flavor(flavor),
    _arraySize(arraySize),
    _refCounter(1)
{
    // ensure name is not null
    if (name.isNull())
    {
        throw UninitializedObjectException();
    }

    // Set the flavor defaults. Must actively set them in case input flavor
    // sets some but not all the defaults.  Also Make sure no conflicts.  This
    // covers the fact that we have separate flags for on and off for the
    // toelement and override functions.  Something must be set on creation
    // and the default in the .h file only covers the case where there is no
    // input.  This also assures that there are no conflicts.  Note that it
    // favors restricted and disable override
    //ATTN: This should become an exception in case conflicting entities are
    // set.
    if (!(_flavor.hasFlavor (CIMFlavor::RESTRICTED)))
        _flavor.addFlavor (CIMFlavor::TOSUBCLASS);
    else
        _flavor.removeFlavor (CIMFlavor::TOSUBCLASS);

    if (!(_flavor.hasFlavor (CIMFlavor::DISABLEOVERRIDE)))
        _flavor.addFlavor (CIMFlavor::ENABLEOVERRIDE);
    else
        _flavor.removeFlavor (CIMFlavor::ENABLEOVERRIDE);

}

void CIMQualifierDeclRep::setName(const CIMName& name)
{
    // ensure name is not null
    if (name.isNull())
    {
        throw UninitializedObjectException();
    }

    _name = name;
}

CIMQualifierDeclRep::CIMQualifierDeclRep(const CIMQualifierDeclRep& x) :
    _name(x._name),
    _value(x._value),
    _scope(x._scope),
    _flavor(x._flavor),
    _arraySize(x._arraySize),
    _refCounter(1)
{

}

Boolean CIMQualifierDeclRep::identical(const CIMQualifierDeclRep* x) const
{
    return
        this == x ||
        (_name.equal(x->_name) &&
         _value == x->_value &&
         (_scope.equal(x->_scope)) &&
         (_flavor.equal(x->_flavor)) &&
         _arraySize == x->_arraySize);
}

PEGASUS_NAMESPACE_END

