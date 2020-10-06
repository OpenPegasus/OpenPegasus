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

#include "CQLSelectStatement.h"
#include "CQLSelectStatementRep.h"
#include <Pegasus/Common/InternalException.h>
#include <Pegasus/CQL/CQLPredicate.h>

PEGASUS_NAMESPACE_BEGIN

CQLSelectStatement::CQLSelectStatement()
    :SelectStatement()
{
    _rep = new CQLSelectStatementRep();

    // Set the _rep into the base class also
    SelectStatement::_rep = _rep;
}

CQLSelectStatement::CQLSelectStatement(
    const String& inQlang,
    const String& inQuery,
    const QueryContext& inCtx)
    :SelectStatement()
{
    _rep = new CQLSelectStatementRep(inQlang,inQuery,inCtx);

    // Set the _rep into the base class also
    SelectStatement::_rep = _rep;
}

CQLSelectStatement::CQLSelectStatement(
    const String& inQlang,
    const String& inQuery)
    :SelectStatement()
{
    _rep = new CQLSelectStatementRep(inQlang,inQuery);

    // Set the _rep into the base class also
    SelectStatement::_rep = _rep;
}

CQLSelectStatement::CQLSelectStatement(const CQLSelectStatement& statement)
    :SelectStatement()
{
    _rep = new CQLSelectStatementRep(*statement._rep);

    // Set the _rep into the base class also
    SelectStatement::_rep = _rep;
}

CQLSelectStatement::~CQLSelectStatement()
{
    if(_rep)
        delete _rep;

    // Note - no need to delete the rep in the base class
}

CQLSelectStatement& CQLSelectStatement::operator=(const CQLSelectStatement& rhs)
{
    if(&rhs != this)
    {
        if(_rep)
            delete _rep;
        _rep = new CQLSelectStatementRep(*rhs._rep);

        // Set the _rep into the base class also
        SelectStatement::_rep = _rep;
    }
    return *this;
}

Boolean CQLSelectStatement::evaluate(const CIMInstance& inCI)
{
    PEGASUS_ASSERT(_rep != NULL);

    return _rep->evaluate(inCI);
}

void CQLSelectStatement::applyProjection(
    CIMInstance& inCI,
    Boolean allowMissing)
{
    PEGASUS_ASSERT(_rep != NULL);

    _rep->applyProjection(inCI, allowMissing);
}

void CQLSelectStatement::validate()
{
    PEGASUS_ASSERT(_rep != NULL);

    _rep->validate();
}

Array<CIMObjectPath> CQLSelectStatement::getClassPathList() const
{
    PEGASUS_ASSERT(_rep != NULL);

    return _rep->getClassPathList();
}

CIMPropertyList CQLSelectStatement::getPropertyList(
    const CIMObjectPath& inClassName)
{
    // Should be set by the concrete sub-classes
    PEGASUS_ASSERT(_rep != NULL);

    return _rep->getPropertyList(inClassName);
}

CIMPropertyList CQLSelectStatement::getSelectPropertyList(
    const CIMObjectPath& inClassName)
{
    // Should be set by the concrete sub-classes
    PEGASUS_ASSERT(_rep != NULL);

    return _rep->getSelectPropertyList(inClassName);
}

CIMPropertyList CQLSelectStatement::getWherePropertyList(
    const CIMObjectPath& inClassName)
{
    // Should be set by the concrete sub-classes
    PEGASUS_ASSERT(_rep != NULL);

    return _rep->getWherePropertyList(inClassName);
}

Array<CQLChainedIdentifier> CQLSelectStatement::getSelectChainedIdentifiers()
{
    // Should be set by the concrete sub-classes
    PEGASUS_ASSERT(_rep != NULL);

    return _rep->getSelectChainedIdentifiers();
}

Array<CQLChainedIdentifier> CQLSelectStatement::getWhereChainedIdentifiers()
{
    // Should be set by the concrete sub-classes
    PEGASUS_ASSERT(_rep != NULL);

    return _rep->getWhereChainedIdentifiers();
}

void CQLSelectStatement::appendClassPath(const CQLIdentifier& inIdentifier)
{
    PEGASUS_ASSERT(_rep != NULL);

    _rep->appendClassPath(inIdentifier);
}

void CQLSelectStatement::setPredicate(const CQLPredicate& inPredicate)
{
    PEGASUS_ASSERT(_rep != NULL);

    _rep->setPredicate(inPredicate);
}

CQLPredicate CQLSelectStatement::getPredicate() const
{
    PEGASUS_ASSERT(_rep != NULL);

    return _rep->getPredicate();
}

void CQLSelectStatement::insertClassPathAlias(
    const CQLIdentifier& inIdentifier,
    const String& inAlias)
{
    PEGASUS_ASSERT(_rep != NULL);

    _rep->insertClassPathAlias(inIdentifier,inAlias);
}

void CQLSelectStatement::appendSelectIdentifier(const CQLChainedIdentifier& x)
{
    PEGASUS_ASSERT(_rep != NULL);

    _rep->appendSelectIdentifier(x);
}

void CQLSelectStatement::applyContext()
{
    PEGASUS_ASSERT(_rep != NULL);

    _rep->applyContext();
}

void CQLSelectStatement::normalizeToDOC()
{
    PEGASUS_ASSERT(_rep != NULL);

    _rep->normalizeToDOC();
}

void CQLSelectStatement::setHasWhereClause()
{
    PEGASUS_ASSERT(_rep != NULL);

    _rep->setHasWhereClause();
}

Boolean CQLSelectStatement::hasWhereClause() const
{
    PEGASUS_ASSERT(_rep != NULL);

    return _rep->hasWhereClause();
}

String CQLSelectStatement::toString() const
{
    PEGASUS_ASSERT(_rep != NULL);

    return _rep->toString();
}

void CQLSelectStatement::clear()
{
    PEGASUS_ASSERT(_rep != NULL);

    _rep->clear();
}

PEGASUS_NAMESPACE_END
