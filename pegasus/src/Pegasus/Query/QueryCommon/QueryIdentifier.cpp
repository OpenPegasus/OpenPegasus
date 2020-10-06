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

#include "QueryIdentifier.h"
#include "QueryIdentifierRep.h"
#include <iostream>
#include <ctype.h>
#include <cstdlib>
PEGASUS_NAMESPACE_BEGIN

/*
Char16 STAR = '*';
Char16 HASH = '#';
Char16 RBRKT = ']';
Char16 LBRKT = '[';
String SCOPE = "::";
*/
QueryIdentifier::QueryIdentifier()
{
    _rep = new QueryIdentifierRep();
}

QueryIdentifier::QueryIdentifier(const QueryIdentifier& id)
{
    _rep = new QueryIdentifierRep(id._rep);
}

QueryIdentifier::~QueryIdentifier()
{
    delete _rep;
}

const CIMName& QueryIdentifier::getName() const
{
    return _rep->getName();
}

void QueryIdentifier::setName(const CIMName& inName)
{
   _rep->setName(inName);
}

const String& QueryIdentifier::getSymbolicConstantName() const
{
    return _rep->getSymbolicConstantName();
}

const  Array<SubRange>& QueryIdentifier::getSubRanges() const
{
    return _rep->getSubRanges();
}

Boolean QueryIdentifier::isArray() const
{
    return _rep->isArray();
}

Boolean QueryIdentifier::isSymbolicConstant() const
{
    return _rep->isSymbolicConstant();
}

Boolean QueryIdentifier::isWildcard() const
{
    return _rep->isWildcard();
}

const String& QueryIdentifier::getScope() const
{
    return _rep->getScope();
}

Boolean QueryIdentifier::isScoped() const
{
    return _rep->isScoped();
}

void QueryIdentifier::applyScope(String scope)
{
    _rep->applyScope(scope);
}


QueryIdentifier& QueryIdentifier::operator=(const QueryIdentifier& rhs)
{
    if (&rhs != this)
    {
        delete _rep;
        _rep = new QueryIdentifierRep(rhs._rep);
    }
    return *this;
}

Boolean QueryIdentifier::operator==(const CIMName &rhs) const
{
    return _rep->operator==(rhs);
}

Boolean QueryIdentifier::operator!=(const CIMName &rhs) const
{
    return(!operator==(rhs));
}

Boolean QueryIdentifier::operator==(const QueryIdentifier &rhs) const
{
    return _rep->operator==(rhs._rep);
}

Boolean QueryIdentifier::operator!=(const QueryIdentifier &rhs) const
{
    return(!operator==(rhs));
}

String QueryIdentifier::toString() const
{
    return _rep->toString();
}

PEGASUS_NAMESPACE_END
