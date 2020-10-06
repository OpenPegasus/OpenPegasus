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
#include "WQLSelectStatement.h"
#include "WQLSelectStatementRep.h"
#include <iostream>
#include <Pegasus/Common/Stack.h>

PEGASUS_USING_STD;

PEGASUS_NAMESPACE_BEGIN

WQLSelectStatement::WQLSelectStatement(
    const String& queryLang,
    const String& query)
    : SelectStatement()
{
    _rep = new WQLSelectStatementRep(queryLang,query);
    // Set the _rep into the base class also
    SelectStatement::_rep = _rep;
}

WQLSelectStatement::WQLSelectStatement(
    const String& queryLang,
    const String& query,
    const QueryContext& inCtx)
    : SelectStatement()
{
    _rep = new WQLSelectStatementRep(queryLang,query,inCtx);
    // Set the _rep into the base class also
    SelectStatement::_rep = _rep;
}

WQLSelectStatement::WQLSelectStatement()
   :SelectStatement()
{
    _rep = new WQLSelectStatementRep();
    // Set the _rep into the base class also
    SelectStatement::_rep = _rep;
}

WQLSelectStatement::WQLSelectStatement(const WQLSelectStatement& statement)
  :SelectStatement()
{
    _rep = new WQLSelectStatementRep(*statement._rep);

    // Set the _rep into the base class also
    SelectStatement::_rep = _rep;
}

WQLSelectStatement& WQLSelectStatement::operator=(const WQLSelectStatement& rhs)
{
    if(&rhs != this)
    {
        if(_rep) delete _rep;
        _rep = new WQLSelectStatementRep(*rhs._rep);

        // Set the _rep into the base class also
        SelectStatement::_rep = _rep;
    }

  return *this;
}


WQLSelectStatement::~WQLSelectStatement()
{
    delete _rep;
}

void WQLSelectStatement::clear()
{
    _rep->clear();
}

Boolean WQLSelectStatement::getAllProperties() const
{
    return _rep->getAllProperties();
}

void WQLSelectStatement::setAllProperties(const Boolean allProperties)
{
    _rep->setAllProperties(allProperties);
}

CIMPropertyList WQLSelectStatement::getSelectPropertyList(
    const CIMObjectPath& inClassName)
{
    return _rep->getSelectPropertyList(inClassName);
}

CIMPropertyList WQLSelectStatement::getWherePropertyList(
    const CIMObjectPath& inClassName)
{
    return _rep->getWherePropertyList(inClassName);
}

Boolean WQLSelectStatement::appendWherePropertyName(const CIMName& x)
{
    return _rep->appendWherePropertyName(x);
}

Boolean WQLSelectStatement::evaluateWhereClause(
    const WQLPropertySource* source) const
{
    return _rep->evaluateWhereClause(source);
}

void WQLSelectStatement::applyProjection(CIMInstance& ci,
    Boolean allowMissing)
{
    _rep->applyProjection(ci, allowMissing);
}

void WQLSelectStatement::applyProjection(CIMObject& ci,
    Boolean allowMissing)
{
    _rep->applyProjection(ci, allowMissing);
}

void WQLSelectStatement::print() const
{
    _rep->print();
}

Boolean WQLSelectStatement::evaluate(const CIMInstance& inCI)
{
    return _rep->evaluate(inCI);
}

void WQLSelectStatement::validate()
{
    _rep->validate();
}

CIMPropertyList WQLSelectStatement::getPropertyList(
        const CIMObjectPath& inClassName)
{
    return _rep->getPropertyList(inClassName);
}

Array<CIMObjectPath> WQLSelectStatement::getClassPathList() const
{
    return _rep->getClassPathList();
}

Uint32 WQLSelectStatement::getSelectPropertyNameCount() const
{
        return _rep->getSelectPropertyNameCount();
}

/** Gets the i-th selected property name in the list.
    This function should only be used if getAllProperties() returns false.
*/
const CIMName& WQLSelectStatement::getSelectPropertyName(Uint32 i) const
{
     return _rep->getSelectPropertyName(i);
}

const CIMName& WQLSelectStatement::getClassName() const
{
    return _rep->getClassName();
}

/** Modifier. This method should not be called by the user (only by the
    parser).
*/
void WQLSelectStatement::setClassName(const CIMName& className)
{
    _rep->setClassName(className);
}

void WQLSelectStatement::appendSelectPropertyName(const CIMName& x)
{
    _rep->appendSelectPropertyName(x);
}

/** Returns the number of unique property names from the where clause.
*/
Uint32 WQLSelectStatement::getWherePropertyNameCount() const
{
    return _rep->getWherePropertyNameCount();
}

/** Gets the i-th unique property appearing in the where clause.
*/
const CIMName& WQLSelectStatement::getWherePropertyName(Uint32 i) const
{
    return _rep->getWherePropertyName(i);
}

void WQLSelectStatement::appendOperation(WQLOperation x)
{
    _rep->appendOperation(x);
}

/** Appends an operand to the operation array. This method should only
    be called by the parser itself.
*/
void WQLSelectStatement::appendOperand(const WQLOperand& x)
{
    _rep->appendOperand(x);
}

/** Returns true if this class has a where clause.
*/
Boolean WQLSelectStatement::hasWhereClause() const
{
    return _rep->hasWhereClause();
}

PEGASUS_NAMESPACE_END
