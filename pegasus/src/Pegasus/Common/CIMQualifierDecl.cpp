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

#include "CIMQualifierDecl.h"
#include "CIMQualifierDeclRep.h"

PEGASUS_NAMESPACE_BEGIN

#define PEGASUS_ARRAY_T CIMQualifierDecl
# include "ArrayImpl.h"
#undef PEGASUS_ARRAY_T

////////////////////////////////////////////////////////////////////////////////
//
// CIMQualifierDecl
//
////////////////////////////////////////////////////////////////////////////////

CIMQualifierDecl::CIMQualifierDecl()
    : _rep(0)
{
}

CIMQualifierDecl::CIMQualifierDecl(const CIMQualifierDecl& x)
{
    _rep = x._rep;
     if (_rep)
         _rep->Inc();
}

CIMQualifierDecl::CIMQualifierDecl(
    const CIMName& name,
    const CIMValue& value,
    const CIMScope & scope,
    const CIMFlavor & flavor,
    Uint32 arraySize)
{
    _rep = new CIMQualifierDeclRep(name, value, scope, flavor, arraySize);
}

CIMQualifierDecl::CIMQualifierDecl(CIMQualifierDeclRep* rep)
    : _rep(rep)
{
}

CIMQualifierDecl::~CIMQualifierDecl()
{
    if (_rep)
        _rep->Dec();
}

CIMQualifierDecl& CIMQualifierDecl::operator=(const CIMQualifierDecl& x)
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

const CIMName& CIMQualifierDecl::getName() const
{
    CheckRep(_rep);
    return _rep->getName();
}

void CIMQualifierDecl::setName(const CIMName& name)
{
    CheckRep(_rep);
    _rep->setName(name);
}

CIMType CIMQualifierDecl::getType() const
{
    CheckRep(_rep);
    return _rep->getType();
}

Boolean CIMQualifierDecl::isArray() const
{
    CheckRep(_rep);
    return _rep->isArray();
}

const CIMValue& CIMQualifierDecl::getValue() const
{
    CheckRep(_rep);
    return _rep->getValue();
}

void CIMQualifierDecl::setValue(const CIMValue& value)
{
    CheckRep(_rep);
    _rep->setValue(value);
}

const CIMScope & CIMQualifierDecl::getScope() const
{
    CheckRep(_rep);
    return _rep->getScope();
}

const CIMFlavor & CIMQualifierDecl::getFlavor() const
{
    CheckRep(_rep);
    return _rep->getFlavor();
}

Uint32 CIMQualifierDecl::getArraySize() const
{
    CheckRep(_rep);
    return _rep->getArraySize();
}

Boolean CIMQualifierDecl::isUninitialized() const
{
    return _rep == 0;
}

Boolean CIMQualifierDecl::identical(const CIMConstQualifierDecl& x) const
{
    CheckRep(x._rep);
    CheckRep(_rep);
    return _rep->identical(x._rep);
}

CIMQualifierDecl CIMQualifierDecl::clone() const
{
    return CIMQualifierDecl(_rep->clone());
}


////////////////////////////////////////////////////////////////////////////////
//
// CIMConstQualifierDecl
//
////////////////////////////////////////////////////////////////////////////////

CIMConstQualifierDecl::CIMConstQualifierDecl()
    : _rep(0)
{
}

CIMConstQualifierDecl::CIMConstQualifierDecl(const CIMConstQualifierDecl& x)
{
    _rep = x._rep;
     if (_rep)
         _rep->Inc();
}

CIMConstQualifierDecl::CIMConstQualifierDecl(const CIMQualifierDecl& x)
{
    _rep = x._rep;
     if (_rep)
         _rep->Inc();
}

CIMConstQualifierDecl::CIMConstQualifierDecl(
    const CIMName& name,
    const CIMValue& value,
    const CIMScope & scope,
    const CIMFlavor & flavor,
    Uint32 arraySize)
{
    _rep = new CIMQualifierDeclRep(name, value, scope, flavor, arraySize);
}

CIMConstQualifierDecl::~CIMConstQualifierDecl()
{
    if (_rep)
        _rep->Dec();
}

CIMConstQualifierDecl& CIMConstQualifierDecl::operator=(
    const CIMConstQualifierDecl& x)
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

CIMConstQualifierDecl& CIMConstQualifierDecl::operator=(
    const CIMQualifierDecl& x)
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

const CIMName& CIMConstQualifierDecl::getName() const
{
    CheckRep(_rep);
    return _rep->getName();
}

CIMType CIMConstQualifierDecl::getType() const
{
    CheckRep(_rep);
    return _rep->getType();
}

Boolean CIMConstQualifierDecl::isArray() const
{
    CheckRep(_rep);
    return _rep->isArray();
}

const CIMValue& CIMConstQualifierDecl::getValue() const
{
    CheckRep(_rep);
    return _rep->getValue();
}

const CIMScope & CIMConstQualifierDecl::getScope() const
{
    CheckRep(_rep);
    return _rep->getScope();
}

const CIMFlavor & CIMConstQualifierDecl::getFlavor() const
{
    CheckRep(_rep);
    return _rep->getFlavor();
}

Uint32 CIMConstQualifierDecl::getArraySize() const
{
    CheckRep(_rep);
    return _rep->getArraySize();
}

Boolean CIMConstQualifierDecl::isUninitialized() const
{
    return _rep == 0;
}

Boolean CIMConstQualifierDecl::identical(const CIMConstQualifierDecl& x) const
{
    CheckRep(x._rep);
    CheckRep(_rep);
    return _rep->identical(x._rep);
}

CIMQualifierDecl CIMConstQualifierDecl::clone() const
{
    return CIMQualifierDecl(_rep->clone());
}

PEGASUS_NAMESPACE_END
