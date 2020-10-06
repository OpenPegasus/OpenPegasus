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

#include "CIMClassRep.h"
#include "CIMInstanceRep.h"
#include "CIMObjectRep.h"
#include "CIMObject.h"
#include "CIMClass.h"
#include "CIMInstance.h"
#include "XmlWriter.h"

PEGASUS_NAMESPACE_BEGIN

#define PEGASUS_ARRAY_T CIMObject
# include "ArrayImpl.h"
#undef PEGASUS_ARRAY_T

////////////////////////////////////////////////////////////////////////////////
//
// CIMObject
//
////////////////////////////////////////////////////////////////////////////////

CIMObject::CIMObject()
    : _rep(0)
{
}

CIMObject::CIMObject(const CIMObject& x)
{
    _rep = x._rep;
    if (_rep)
        _rep->Inc();
}

CIMObject::CIMObject(const CIMClass& x)
{
    _rep = x._rep;
    if (_rep)
        _rep->Inc();
}

CIMObject::CIMObject(const CIMInstance& x)
{
    _rep = x._rep;
    if (_rep)
        _rep->Inc();
}

CIMObject::CIMObject(CIMObjectRep* rep)
    : _rep(rep)
{
}

CIMObject& CIMObject::operator=(const CIMObject& x)
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

CIMObject::~CIMObject()
{
    if (_rep)
        _rep->Dec();
}

const CIMName& CIMObject::getClassName() const
{
    CheckRep(_rep);
    return _rep->getClassName();
}

const CIMObjectPath& CIMObject::getPath() const
{
    CheckRep(_rep);
    return _rep->getPath();
}

void CIMObject::setPath(const CIMObjectPath & path)
{
    CheckRep(_rep);
    _rep->setPath(path);
}

CIMObject& CIMObject::addQualifier(const CIMQualifier& qualifier)
{
    CheckRep(_rep);
    _rep->addQualifier(qualifier);
    return *this;
}

Uint32 CIMObject::findQualifier(const CIMName& name) const
{
    CheckRep(_rep);
    return _rep->findQualifier(name);
}

CIMQualifier CIMObject::getQualifier(Uint32 index)
{
    CheckRep(_rep);
    return _rep->getQualifier(index);
}

CIMConstQualifier CIMObject::getQualifier(Uint32 index) const
{
    CheckRep(_rep);
    return _rep->getQualifier(index);
}

void CIMObject::removeQualifier(Uint32 index)
{
    CheckRep(_rep);
    _rep->removeQualifier(index);
}

Uint32 CIMObject::getQualifierCount() const
{
    CheckRep(_rep);
    return _rep->getQualifierCount();
}

CIMObject& CIMObject::addProperty(const CIMProperty& x)
{
    CheckRep(_rep);
    _rep->addProperty(x);
    return *this;
}

Uint32 CIMObject::findProperty(const CIMName& name) const
{
    CheckRep(_rep);
    return _rep->findProperty(name);
}

CIMProperty CIMObject::getProperty(Uint32 index)
{
    CheckRep(_rep);
    return _rep->getProperty(index);
}

CIMConstProperty CIMObject::getProperty(Uint32 index) const
{
    CheckRep(_rep);
    return _rep->getProperty(index);
}

void CIMObject::removeProperty(Uint32 index)
{
    CheckRep(_rep);
    _rep->removeProperty(index);
}

Uint32 CIMObject::getPropertyCount() const
{
    CheckRep(_rep);
    return _rep->getPropertyCount();
}

Boolean CIMObject::isUninitialized() const
{
    return _rep == 0;
}

String CIMObject::toString() const
{
    CheckRep(_rep);
    Buffer out;

    XmlWriter::appendObjectElement(out, *this);

    return out.getData();
}

Boolean CIMObject::isClass() const
{
    return Boolean(dynamic_cast<CIMClassRep*>(this->_rep));
}

Boolean CIMObject::isInstance() const
{
    return Boolean(dynamic_cast<CIMInstanceRep*>(this->_rep));
}

Boolean CIMObject::identical(const CIMConstObject& x) const
{
    CheckRep(x._rep);
    CheckRep(_rep);
    return _rep->identical(x._rep);
}

CIMObject CIMObject::clone() const
{
    CheckRep(_rep);
    return CIMObject(_rep->clone());
}

void CIMObject::instanceFilter(
    Boolean includeQualifiers,
    Boolean includeClassOrigin,
    const CIMPropertyList & propertyList)
{
    CheckRep(_rep);
    _rep->instanceFilter(includeQualifiers, includeClassOrigin, propertyList);
}


////////////////////////////////////////////////////////////////////////////////
//
// CIMConstObject
//
////////////////////////////////////////////////////////////////////////////////

CIMConstObject::CIMConstObject()
    : _rep(0)
{
}

CIMConstObject::CIMConstObject(const CIMConstObject& x)
{
    _rep = x._rep;
    if (_rep)
        _rep->Inc();
}

CIMConstObject::CIMConstObject(const CIMObject& x)
{
    _rep = x._rep;
    if (_rep)
        _rep->Inc();
}

CIMConstObject::CIMConstObject(const CIMClass& x)
{
    _rep = x._rep;
    if (_rep)
        _rep->Inc();
}

CIMConstObject::CIMConstObject(const CIMConstClass& x)
{
    _rep = x._rep;
    if (_rep)
        _rep->Inc();
}

CIMConstObject::CIMConstObject(const CIMInstance& x)
{
    _rep = x._rep;
    if (_rep)
        _rep->Inc();
}

CIMConstObject::CIMConstObject(const CIMConstInstance& x)
{
    _rep = x._rep;
    if (_rep)
        _rep->Inc();
}

CIMConstObject& CIMConstObject::operator=(const CIMConstObject& x)
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

CIMConstObject::~CIMConstObject()
{
    if (_rep)
        _rep->Dec();
}

const CIMName& CIMConstObject::getClassName() const
{
    CheckRep(_rep);
    return _rep->getClassName();
}

const CIMObjectPath& CIMConstObject::getPath() const
{
    CheckRep(_rep);
    return _rep->getPath();
}

Uint32 CIMConstObject::findQualifier(const CIMName& name) const
{
    CheckRep(_rep);
    return _rep->findQualifier(name);
}

CIMConstQualifier CIMConstObject::getQualifier(Uint32 index) const
{
    CheckRep(_rep);
    return _rep->getQualifier(index);
}

Uint32 CIMConstObject::getQualifierCount() const
{
    CheckRep(_rep);
    return _rep->getQualifierCount();
}

Uint32 CIMConstObject::findProperty(const CIMName& name) const
{
    CheckRep(_rep);
    return _rep->findProperty(name);
}

CIMConstProperty CIMConstObject::getProperty(Uint32 index) const
{
    CheckRep(_rep);
    return _rep->getProperty(index);
}

Uint32 CIMConstObject::getPropertyCount() const
{
    CheckRep(_rep);
    return _rep->getPropertyCount();
}

Boolean CIMConstObject::isUninitialized() const
{
    return _rep == 0;
}

String CIMConstObject::toString() const
{
    CheckRep(_rep);
    Buffer out;

    XmlWriter::appendObjectElement(out, *this);

    return out.getData();
}

Boolean CIMConstObject::isClass() const
{
    return Boolean(dynamic_cast<CIMClassRep*>(this->_rep));
}

Boolean CIMConstObject::isInstance() const
{
    return Boolean(dynamic_cast<CIMInstanceRep*>(this->_rep));
}

Boolean CIMConstObject::identical(const CIMConstObject& x) const
{
    CheckRep(x._rep);
    CheckRep(_rep);
    return _rep->identical(x._rep);
}

CIMObject CIMConstObject::clone() const
{
    return CIMObject(_rep->clone());
}

PEGASUS_NAMESPACE_END
