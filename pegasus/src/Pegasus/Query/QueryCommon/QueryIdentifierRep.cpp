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

#include <iostream>
#include "QueryIdentifierRep.h"
#include "QueryException.h"
#include <ctype.h>
#include <cstdlib>
PEGASUS_NAMESPACE_BEGIN

QueryIdentifierRep::QueryIdentifierRep()
    : _isWildcard(false),
      _isSymbolicConstant(false)
{
    _name = CIMName();
}

QueryIdentifierRep::QueryIdentifierRep(const QueryIdentifierRep* rep)
{
    _symbolicConstant = rep->_symbolicConstant;
    _scope = rep->_scope;
    _indices = rep->_indices;
    _name = rep->_name;
    _isWildcard = rep->_isWildcard;
    _isSymbolicConstant = rep->_isSymbolicConstant;
}

QueryIdentifierRep::~QueryIdentifierRep()
{
}

const CIMName& QueryIdentifierRep::getName() const
{
    return _name;
}

void QueryIdentifierRep::setName(const CIMName& inName)
{
   _name = inName;
}


const String& QueryIdentifierRep::getSymbolicConstantName() const
{
    return _symbolicConstant;
}

const Array<SubRange>& QueryIdentifierRep::getSubRanges() const
{
    return _indices;
}

Boolean QueryIdentifierRep::isArray() const
{
    return(_indices.size() > 0);
}

Boolean QueryIdentifierRep::isSymbolicConstant() const
{
    return _isSymbolicConstant;
}

Boolean QueryIdentifierRep::isWildcard() const
{
    return _isWildcard;
}

const String& QueryIdentifierRep::getScope() const
{
    return _scope;
}

Boolean QueryIdentifierRep::isScoped() const
{
    return (_scope != String::EMPTY);
}

void QueryIdentifierRep::applyScope(String scope)
{
    _scope = scope;
}

QueryIdentifierRep& QueryIdentifierRep::operator=(const QueryIdentifierRep& rhs)
{
    if (&rhs != this)
    {
        _symbolicConstant = rhs._symbolicConstant;
        _scope = rhs._scope;
        _indices = rhs._indices;
        _name = rhs._name;
        _isWildcard = rhs._isWildcard;
        _isSymbolicConstant = rhs._isSymbolicConstant;
    }
    return *this;
}

Boolean QueryIdentifierRep::operator==(const CIMName &rhs) const
{
    if (_name == rhs)
        return true;
    return false;
}

Boolean QueryIdentifierRep::operator!=(const CIMName &rhs) const
{
    return !operator==(rhs);
}

Boolean QueryIdentifierRep::operator==(const QueryIdentifierRep &rhs) const
{
    if (_isWildcard && rhs.isWildcard())
        return true;
    if (getName() == rhs.getName())
    {
        if (String::equalNoCase(getScope(), rhs.getScope()))
        {
            if (String::equalNoCase(
                    getSymbolicConstantName(), rhs.getSymbolicConstantName()))
            {
                // compare _indices arrays
                Uint32 size = rhs.getSubRanges().size();
                if (_indices.size() == size)
                {
                    Array<SubRange> rhs_SubRanges = rhs.getSubRanges();
                    for (Uint32 i = 0; i < size; i++)
                    {
                        if (_indices[i] != rhs_SubRanges[i])
                            return false;
                    }
                    return true;
                }
            }
        }
    }
    return false;
}

Boolean QueryIdentifierRep::operator!=(const QueryIdentifierRep &rhs) const
{
    return !operator==(rhs);
}

String QueryIdentifierRep::toString() const
{
    if (_isWildcard)
        return "*";
    String s = getScope();
    if (s != String::EMPTY)
        s.append("::");
    s.append(_name.getString());
    if (_isSymbolicConstant)
    {
        s.append("#").append(_symbolicConstant);
        return s;
    }
    if (isArray())
    {
        s.append("[");
        for (Uint32 i = 0; i < _indices.size(); i++)
        {
            s.append(_indices[i].toString());
            if (i < _indices.size()-1)
                s.append(",");
        }
        s.append("]");
    }
    return s;
}

PEGASUS_NAMESPACE_END
