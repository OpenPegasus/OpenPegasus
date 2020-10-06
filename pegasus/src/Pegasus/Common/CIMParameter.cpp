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

#include "CIMParameter.h"
#include "CIMParameterRep.h"

PEGASUS_NAMESPACE_BEGIN

#define PEGASUS_ARRAY_T CIMParameter
# include "ArrayImpl.h"
#undef PEGASUS_ARRAY_T

////////////////////////////////////////////////////////////////////////////////
//
// CIMParameter
//
////////////////////////////////////////////////////////////////////////////////

CIMParameter::CIMParameter()
    : _rep(0)
{
}

CIMParameter::CIMParameter(const CIMParameter& x)
{
    _rep = x._rep;
    if (_rep)
        _rep->Inc();
}

CIMParameter::CIMParameter(
    const CIMName& name,
    CIMType type,
    Boolean isArray,
    Uint32 arraySize,
    const CIMName& referenceClassName)
{
    _rep = new CIMParameterRep(
        name, type, isArray, arraySize, referenceClassName);
}

CIMParameter::CIMParameter(CIMParameterRep* rep)
    : _rep(rep)
{
}

CIMParameter::~CIMParameter()
{
    if (_rep)
        _rep->Dec();
}

CIMParameter& CIMParameter::operator=(const CIMParameter& x)
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

const CIMName& CIMParameter::getName() const
{
    CheckRep(_rep);
    return _rep->getName();
}

void CIMParameter::setName(const CIMName& name)
{
    CheckRep(_rep);
    _rep->setName(name);
}

Boolean CIMParameter::isArray() const
{
    CheckRep(_rep);
    return _rep->isArray();
}

Uint32 CIMParameter::getArraySize() const
{
    CheckRep(_rep);
    return _rep->getArraySize();
}

const CIMName& CIMParameter::getReferenceClassName() const
{
    CheckRep(_rep);
    return _rep->getReferenceClassName();
}

CIMType CIMParameter::getType() const
{
    CheckRep(_rep);
    return _rep->getType();
}

CIMParameter& CIMParameter::addQualifier(const CIMQualifier& x)
{
    CheckRep(_rep);
    _rep->addQualifier(x);
    return *this;
}

Uint32 CIMParameter::findQualifier(const CIMName& name) const
{
    CheckRep(_rep);
    return _rep->findQualifier(name);
}

CIMQualifier CIMParameter::getQualifier(Uint32 index)
{
    CheckRep(_rep);
    return _rep->getQualifier(index);
}

CIMConstQualifier CIMParameter::getQualifier(Uint32 index) const
{
    CheckRep(_rep);
    return _rep->getQualifier(index);
}

void CIMParameter::removeQualifier (Uint32 index)
{
    CheckRep(_rep);
    _rep->removeQualifier (index);
}

Uint32 CIMParameter::getQualifierCount() const
{
    CheckRep(_rep);
    return _rep->getQualifierCount();
}

Boolean CIMParameter::isUninitialized() const
{
    return _rep == 0;
}

Boolean CIMParameter::identical(const CIMConstParameter& x) const
{
    CheckRep(x._rep);
    CheckRep(_rep);
    return _rep->identical(x._rep);
}

CIMParameter CIMParameter::clone() const
{
    return CIMParameter(_rep->clone());
}


////////////////////////////////////////////////////////////////////////////////
//
// CIMConstParameter
//
////////////////////////////////////////////////////////////////////////////////

CIMConstParameter::CIMConstParameter()
    : _rep(0)
{
}

CIMConstParameter::CIMConstParameter(const CIMConstParameter& x)
{
    _rep = x._rep;
    if (_rep)
        _rep->Inc();
}

CIMConstParameter::CIMConstParameter(const CIMParameter& x)
{
    _rep = x._rep;
    if (_rep)
        _rep->Inc();
}

CIMConstParameter::CIMConstParameter(
    const CIMName& name,
    CIMType type,
    Boolean isArray,
    Uint32 arraySize,
    const CIMName& referenceClassName)
{
    _rep = new CIMParameterRep(
        name, type, isArray, arraySize, referenceClassName);
}

CIMConstParameter::~CIMConstParameter()
{
    if (_rep)
        _rep->Dec();
}

CIMConstParameter& CIMConstParameter::operator=(const CIMConstParameter& x)
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

CIMConstParameter& CIMConstParameter::operator=(const CIMParameter& x)
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

const CIMName& CIMConstParameter::getName() const
{
    CheckRep(_rep);
    return _rep->getName();
}

Boolean CIMConstParameter::isArray() const
{
    CheckRep(_rep);
    return _rep->isArray();
}

Uint32 CIMConstParameter::getArraySize() const
{
    CheckRep(_rep);
    return _rep->getArraySize();
}

const CIMName& CIMConstParameter::getReferenceClassName() const
{
    CheckRep(_rep);
    return _rep->getReferenceClassName();
}

CIMType CIMConstParameter::getType() const
{
    CheckRep(_rep);
    return _rep->getType();
}

Uint32 CIMConstParameter::findQualifier(const CIMName& name) const
{
    CheckRep(_rep);
    return _rep->findQualifier(name);
}

CIMConstQualifier CIMConstParameter::getQualifier(Uint32 index) const
{
    CheckRep(_rep);
    return _rep->getQualifier(index);
}

Uint32 CIMConstParameter::getQualifierCount() const
{
    CheckRep(_rep);
    return _rep->getQualifierCount();
}

Boolean CIMConstParameter::isUninitialized() const
{
    return _rep == 0;
}

Boolean CIMConstParameter::identical(const CIMConstParameter& x) const
{
    CheckRep(x._rep);
    CheckRep(_rep);
    return _rep->identical(x._rep);
}

CIMParameter CIMConstParameter::clone() const
{
    return CIMParameter(_rep->clone());
}

PEGASUS_NAMESPACE_END
