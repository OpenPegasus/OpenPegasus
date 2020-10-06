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

#include "CIMQualifier.h"
#include "CIMQualifierRep.h"
#include "CIMName.h"
#include "InternalException.h"
#include "StrLit.h"

PEGASUS_NAMESPACE_BEGIN
PEGASUS_USING_STD;

////////////////////////////////////////////////////////////////////////////////
//
// CIMQualifierRep
//
////////////////////////////////////////////////////////////////////////////////

CIMQualifierRep::CIMQualifierRep(const CIMQualifierRep& x) :
    _name(x._name),
    _value(x._value),
    _flavor(x._flavor),
    _propagated(x._propagated),
    _nameTag(x._nameTag),
    _refCounter(1),
    _ownerCount(0)
{
}

CIMQualifierRep::CIMQualifierRep(
    const CIMName& name,
    const CIMValue& value,
    const CIMFlavor & flavor,
    Boolean propagated)
    :
    _name(name),
    _value(value),
    _flavor(flavor),
    _propagated(propagated),
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
}

void CIMQualifierRep::setName(const CIMName& name)
{
    // ensure name is not null
    if (name.isNull())
    {
        throw UninitializedObjectException();
    }

    if (_ownerCount != 0 && _name != name)
    {
        MessageLoaderParms parms(
            "Common.CIMQualifierRep.CONTAINED_QUALIFIER_NAMECHANGEDEXCEPTION",
            "Attempted to change the name of a qualifier"
                " already in a container.");
        throw Exception(parms);
    }

    _name = name;

    // Set the CIM name tag.
    _nameTag = generateCIMNameTag(_name);
}

void CIMQualifierRep::resolveFlavor(const CIMFlavor & inheritedFlavor)
{
    // ATTN: KS P3 Needs more tests and expansion so we treate first different
    // from inheritance

    // if the turnoff flags set, reset the flavor bits
    if (inheritedFlavor.hasFlavor (CIMFlavor::RESTRICTED))
    {
        _flavor.removeFlavor (CIMFlavor::TOSUBCLASS);
        _flavor.removeFlavor (CIMFlavor::TOINSTANCE);
    }
    if (inheritedFlavor.hasFlavor (CIMFlavor::DISABLEOVERRIDE))
    {
        _flavor.removeFlavor (CIMFlavor::ENABLEOVERRIDE);
    }

    _flavor.addFlavor (inheritedFlavor);
}

Boolean CIMQualifierRep::identical(const CIMQualifierRep* x) const
{
    return
        this == x ||
        (_name.equal(x->_name) &&
         _value == x->_value &&
         (_flavor.equal(x->_flavor)) &&
         _propagated == x->_propagated);
}

PEGASUS_NAMESPACE_END
