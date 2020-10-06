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

#include "CIMMethod.h"
#include "CIMMethodRep.h"

PEGASUS_NAMESPACE_BEGIN

#define PEGASUS_ARRAY_T CIMMethod
# include "ArrayImpl.h"
#undef PEGASUS_ARRAY_T

///////////////////////////////////////////////////////////////////////////////
//
// CIMMethod
//
///////////////////////////////////////////////////////////////////////////////

CIMMethod::CIMMethod()
    : _rep(0)
{
}

CIMMethod::CIMMethod(const CIMMethod& x)
{
    _rep = x._rep;
    if (_rep)
        _rep->Inc();
}

CIMMethod::CIMMethod(
    const CIMName& name,
    CIMType type,
    const CIMName& classOrigin,
    Boolean propagated)
{
    _rep = new CIMMethodRep(name, type, classOrigin, propagated);
}

CIMMethod::CIMMethod(CIMMethodRep* rep)
    : _rep(rep)
{
}

CIMMethod::~CIMMethod()
{
    if (_rep)
        _rep->Dec();
}

CIMMethod& CIMMethod::operator=(const CIMMethod& x)
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

const CIMName& CIMMethod::getName() const
{
    CheckRep(_rep);
    return _rep->getName();
}

void CIMMethod::setName(const CIMName& name)
{
    CheckRep(_rep);
    _rep->setName(name);
}

CIMType CIMMethod::getType() const
{
    CheckRep(_rep);
    return _rep->getType();
}

void CIMMethod::setType(CIMType type)
{
    CheckRep(_rep);
    _rep->setType(type);
}

const CIMName& CIMMethod::getClassOrigin() const
{
    CheckRep(_rep);
    return _rep->getClassOrigin();
}

void CIMMethod::setClassOrigin(const CIMName& classOrigin)
{
    CheckRep(_rep);
    _rep->setClassOrigin(classOrigin);
}

Boolean CIMMethod::getPropagated() const
{
    CheckRep(_rep);
    return _rep->getPropagated();
}

void CIMMethod::setPropagated(Boolean propagated)
{
    CheckRep(_rep);
    _rep->setPropagated(propagated);
}

CIMMethod& CIMMethod::addQualifier(const CIMQualifier& x)
{
    CheckRep(_rep);
    _rep->addQualifier(x);
    return *this;
}

Uint32 CIMMethod::findQualifier(const CIMName& name) const
{
    CheckRep(_rep);
    return _rep->findQualifier(name);
}

CIMQualifier CIMMethod::getQualifier(Uint32 index)
{
    CheckRep(_rep);
    return _rep->getQualifier(index);
}

CIMConstQualifier CIMMethod::getQualifier(Uint32 index) const
{
    CheckRep(_rep);
    return _rep->getQualifier(index);
}

void CIMMethod::removeQualifier(Uint32 index)
{
    CheckRep(_rep);
    _rep->removeQualifier(index);
}

Uint32 CIMMethod::getQualifierCount() const
{
    CheckRep(_rep);
    return _rep->getQualifierCount();
}

CIMMethod& CIMMethod::addParameter(const CIMParameter& x)
{
    CheckRep(_rep);
    _rep->addParameter(x);
    return *this;
}

Uint32 CIMMethod::findParameter(const CIMName& name) const
{
    CheckRep(_rep);
    return _rep->findParameter(name);
}

CIMParameter CIMMethod::getParameter(Uint32 index)
{
    CheckRep(_rep);
    return _rep->getParameter(index);
}

CIMConstParameter CIMMethod::getParameter(Uint32 index) const
{
    CheckRep(_rep);
    return _rep->getParameter(index);
}

void CIMMethod::removeParameter (Uint32 index)
{
    CheckRep(_rep);
    _rep->removeParameter (index);
}

Uint32 CIMMethod::getParameterCount() const
{
    CheckRep(_rep);
    return _rep->getParameterCount();
}

Boolean CIMMethod::isUninitialized() const
{
    return _rep == 0;
}

Boolean CIMMethod::identical(const CIMConstMethod& x) const
{
    CheckRep(x._rep);
    CheckRep(_rep);
    return _rep->identical(x._rep);
}

CIMMethod CIMMethod::clone() const
{
    return CIMMethod(_rep->clone());
}


///////////////////////////////////////////////////////////////////////////////
//
// CIMConstMethod
//
///////////////////////////////////////////////////////////////////////////////

CIMConstMethod::CIMConstMethod()
    : _rep(0)
{
}

CIMConstMethod::CIMConstMethod(const CIMConstMethod& x)
{
    _rep = x._rep;
    if (_rep)
        _rep->Inc();
}

CIMConstMethod::CIMConstMethod(const CIMMethod& x)
{
    _rep = x._rep;
    if (_rep)
        _rep->Inc();
}

CIMConstMethod::CIMConstMethod(
    const CIMName& name,
    CIMType type,
    const CIMName& classOrigin,
    Boolean propagated)
{
    _rep = new CIMMethodRep(name, type, classOrigin, propagated);
}

CIMConstMethod::~CIMConstMethod()
{
    if (_rep)
        _rep->Dec();
}

CIMConstMethod& CIMConstMethod::operator=(const CIMConstMethod& x)
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

CIMConstMethod& CIMConstMethod::operator=(const CIMMethod& x)
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

const CIMName& CIMConstMethod::getName() const
{
    CheckRep(_rep);
    return _rep->getName();
}

CIMType CIMConstMethod::getType() const
{
    CheckRep(_rep);
    return _rep->getType();
}

const CIMName& CIMConstMethod::getClassOrigin() const
{
    CheckRep(_rep);
    return _rep->getClassOrigin();
}

Boolean CIMConstMethod::getPropagated() const
{
    CheckRep(_rep);
    return _rep->getPropagated();
}

Uint32 CIMConstMethod::findQualifier(const CIMName& name) const
{
    CheckRep(_rep);
    return _rep->findQualifier(name);
}

CIMConstQualifier CIMConstMethod::getQualifier(Uint32 index) const
{
    CheckRep(_rep);
    return _rep->getQualifier(index);
}

Uint32 CIMConstMethod::getQualifierCount() const
{
    CheckRep(_rep);
    return _rep->getQualifierCount();
}

Uint32 CIMConstMethod::findParameter(const CIMName& name) const
{
    CheckRep(_rep);
    return _rep->findParameter(name);
}

CIMConstParameter CIMConstMethod::getParameter(Uint32 index) const
{
    CheckRep(_rep);
    return _rep->getParameter(index);
}

Uint32 CIMConstMethod::getParameterCount() const
{
    CheckRep(_rep);
    return _rep->getParameterCount();
}

Boolean CIMConstMethod::isUninitialized() const
{
    return _rep == 0;
}

Boolean CIMConstMethod::identical(const CIMConstMethod& x) const
{
    CheckRep(x._rep);
    CheckRep(_rep);
    return _rep->identical(x._rep);
}

CIMMethod CIMConstMethod::clone() const
{
    return CIMMethod(_rep->clone());
}

PEGASUS_NAMESPACE_END
