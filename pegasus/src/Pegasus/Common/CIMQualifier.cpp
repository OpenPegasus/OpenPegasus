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

PEGASUS_NAMESPACE_BEGIN

#define PEGASUS_ARRAY_T CIMQualifier
# include "ArrayImpl.h"
#undef PEGASUS_ARRAY_T

////////////////////////////////////////////////////////////////////////////////
//
// CIMQualifier
//
////////////////////////////////////////////////////////////////////////////////

CIMQualifier::CIMQualifier()
    : _rep(0)
{
}

CIMQualifier::CIMQualifier(const CIMQualifier& x)
{
    _rep = x._rep;
    if (_rep)
        _rep->Inc();
}

CIMQualifier::CIMQualifier(
    const CIMName& name,
    const CIMValue& value,
    const CIMFlavor & flavor,
    Boolean propagated)
{
    _rep = new CIMQualifierRep(name, value, flavor, propagated);
}

CIMQualifier::CIMQualifier(CIMQualifierRep* rep)
    : _rep(rep)
{
}

CIMQualifier::~CIMQualifier()
{
    if(_rep)
       _rep->Dec();
}

CIMQualifier& CIMQualifier::operator=(const CIMQualifier& x)
{
    if (x._rep != _rep)
    {
        if (_rep)
            _rep->Dec();
        _rep = x._rep;
        if (_rep)
            _rep->Inc();
    }

    return *this;
}

const CIMName& CIMQualifier::getName() const
{
    CheckRep(_rep);
    return _rep->getName();
}

void CIMQualifier::setName(const CIMName& name)
{
    CheckRep(_rep);
    _rep->setName(name);
}

CIMType CIMQualifier::getType() const
{
    CheckRep(_rep);
    return _rep->getType();
}

Boolean CIMQualifier::isArray() const
{
    CheckRep(_rep);
    return _rep->isArray();
}

const CIMValue& CIMQualifier::getValue() const
{
    CheckRep(_rep);
    return _rep->getValue();
}

void CIMQualifier::setValue(const CIMValue& value)
{
    CheckRep(_rep);
    _rep->setValue(value);
}

void CIMQualifier::setFlavor(const CIMFlavor & flavor)
{
    CheckRep(_rep);
    _rep->setFlavor(flavor);
}

void CIMQualifier::unsetFlavor(const CIMFlavor & flavor)
{
    CheckRep(_rep);
    _rep->unsetFlavor(flavor);
}

const CIMFlavor & CIMQualifier::getFlavor() const
{
    CheckRep(_rep);
    return _rep->getFlavor();
}

Uint32 CIMQualifier::getPropagated() const
{
    CheckRep(_rep);
    return (_rep->getPropagated()) ? 1 : 0;
}

void CIMQualifier::setPropagated(Boolean propagated)
{
    CheckRep(_rep);
    _rep->setPropagated(propagated);
}

Boolean CIMQualifier::isUninitialized() const
{
    return _rep == 0;
}

Boolean CIMQualifier::identical(const CIMConstQualifier& x) const
{
    CheckRep(x._rep);
    CheckRep(_rep);
    return _rep->identical(x._rep);
}

CIMQualifier CIMQualifier::clone() const
{
    return CIMQualifier(_rep->clone());
}


////////////////////////////////////////////////////////////////////////////////
//
// CIMConstQualifier
//
////////////////////////////////////////////////////////////////////////////////

CIMConstQualifier::CIMConstQualifier()
    : _rep(0)
{
}

CIMConstQualifier::CIMConstQualifier(const CIMConstQualifier& x)
{
       _rep = x._rep;
       if (_rep)
           _rep->Inc();
}

CIMConstQualifier::CIMConstQualifier(const CIMQualifier& x)
{
       _rep = x._rep;
       if (_rep)
           _rep->Inc();
}

CIMConstQualifier::CIMConstQualifier(
    const CIMName& name,
    const CIMValue& value,
    const CIMFlavor & flavor,
    Boolean propagated)
{
    _rep = new CIMQualifierRep(name, value, flavor, propagated);
}

CIMConstQualifier::~CIMConstQualifier()
{
    if (_rep)
        _rep->Dec();
}

CIMConstQualifier& CIMConstQualifier::operator=(const CIMConstQualifier& x)
{
    if (x._rep != _rep)
    {
        if (_rep)
            _rep->Dec();
        _rep = x._rep;
        if (_rep)
            _rep->Inc();
    }

    return *this;
}

CIMConstQualifier& CIMConstQualifier::operator=(const CIMQualifier& x)
{
    if (x._rep != _rep)
    {
        if (_rep)
            _rep->Dec();
        _rep = x._rep;
        if (_rep)
            _rep->Inc();
    }

    return *this;
}

const CIMName& CIMConstQualifier::getName() const
{
    CheckRep(_rep);
    return _rep->getName();
}

CIMType CIMConstQualifier::getType() const
{
    CheckRep(_rep);
    return _rep->getType();
}

Boolean CIMConstQualifier::isArray() const
{
    CheckRep(_rep);
    return _rep->isArray();
}

const CIMValue& CIMConstQualifier::getValue() const
{
    CheckRep(_rep);
    return _rep->getValue();
}

const CIMFlavor & CIMConstQualifier::getFlavor() const
{
    CheckRep(_rep);
    return _rep->getFlavor();
}

Uint32 CIMConstQualifier::getPropagated() const
{
    CheckRep(_rep);
    return (_rep->getPropagated()) ? 1 : 0;
}

Boolean CIMConstQualifier::isUninitialized() const
{
    return _rep == 0;
}

Boolean CIMConstQualifier::identical(const CIMConstQualifier& x) const
{
    CheckRep(x._rep);
    CheckRep(_rep);
    return _rep->identical(x._rep);
}

CIMQualifier CIMConstQualifier::clone() const
{
    return CIMQualifier(_rep->clone());
}

PEGASUS_NAMESPACE_END
