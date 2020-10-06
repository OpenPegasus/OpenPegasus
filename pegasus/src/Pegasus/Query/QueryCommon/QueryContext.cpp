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

#include "QueryContext.h"
#include <Pegasus/Common/PegasusVersion.h>
#include <Pegasus/Common/System.h>
#include <iostream>

PEGASUS_NAMESPACE_BEGIN

QueryContext::QueryContext(const CIMNamespaceName& inNS)
    : _NS(inNS)
{
}

QueryContext::QueryContext(const QueryContext& ctx)
    : _NS(ctx._NS),
      _AliasClassTable(ctx._AliasClassTable),
      _fromList(ctx._fromList),
      _whereList(ctx._whereList)
{
}

QueryContext::~QueryContext()
{
}

QueryContext& QueryContext::operator=(const QueryContext& rhs)
{
    if (this == &rhs)
        return *this;

    _NS = rhs._NS;
    _fromList = rhs._fromList;
    _AliasClassTable = rhs._AliasClassTable;
    _whereList = rhs._whereList;

    return *this;
}

String QueryContext::getHost(Boolean fullyQualified)
{
    if (fullyQualified)
        return System::getFullyQualifiedHostName();
    return System::getHostName();
}

CIMNamespaceName QueryContext::getNamespace() const
{
    return _NS;
}

void QueryContext::insertClassPath(
    const QueryIdentifier& inIdentifier,
    String inAlias)
{
    QueryIdentifier _class;

    if (inIdentifier.getName().getString() == String::EMPTY)
    {
        MessageLoaderParms parms(
            "QueryCommon.QueryContext.EMPTY_CLASSNAME",
            "Empty classname is not allowed");
        throw QueryParseException(parms);
    }

    if ((inAlias != String::EMPTY) &&
        (String::equalNoCase(inAlias, inIdentifier.getName().getString())))
    {
        inAlias = String::EMPTY;
    }

    if (_AliasClassTable.contains(inIdentifier.getName().getString()))
    {
        MessageLoaderParms parms(
            "QueryCommon.QueryContext.CLASS_ALREADY_ALIAS",
            "A class name in the FROM list is already used as an alias.");
        throw QueryParseException(parms);
    }

    Boolean found = false;
    for (Uint32 i = 0; i < _fromList.size(); i++)
    {
        if ((inAlias != String::EMPTY) &&
            (String::equalNoCase(inAlias, _fromList[i].getName().getString())))
        {
            MessageLoaderParms parms(
                "QueryCommon.QueryContext.ALIAS_ALREADY_CLASS",
                "An alias name in the FROM list is already used as a class "
                    "name.");
            throw QueryParseException(parms);
        }

        if (_fromList[i].getName() == inIdentifier.getName())
        {
            found = true;
        }
    }

    if (inAlias != String::EMPTY)
    {
        if (!_AliasClassTable.insert(inAlias, inIdentifier))
        {
            // Alias already exists
            if (_AliasClassTable.lookup(inAlias, _class))
            {
                if (!_class.getName().equal(inIdentifier.getName()))
                {
                    MessageLoaderParms parms(
                        "QueryCommon.QueryContext.ALIAS_CONFLICT",
                        "An alias is being used for different class names.");
                    throw QueryParseException(parms);
                }
            }
        }
    }

    if (!found)
    {
        _fromList.append(inIdentifier);
    }
}

void QueryContext::addWhereIdentifier(
    const QueryChainedIdentifier& inIdentifier) const
{
    // If the identifier is already in the list then don't append.
    for (Uint32 i = 0; i < _whereList.size(); i++)
    {
        if (inIdentifier.size() == _whereList[i].size())
        {
            Array<QueryIdentifier> subsWhere =
                _whereList[i].getSubIdentifiers();
            Array<QueryIdentifier> subsIn = inIdentifier.getSubIdentifiers();
            Boolean match = true;
            for (Uint32 j = 0; j < subsWhere.size(); j++)
            {
                if (subsIn[j] != subsWhere[j])
                {
                    match = false;
                }
            }

            if (match)
                return;
        }
    }

    _whereList.append(inIdentifier);
}

Array<QueryChainedIdentifier> QueryContext::getWhereList() const
{
    return _whereList;
}

QueryIdentifier QueryContext::findClass(const String& inAlias) const
{
    // look for alias match
    QueryIdentifier _class;
    if (_AliasClassTable.lookup(inAlias, _class)) return _class;

    // look if inAlias is really a class name
    CIMName _aliasName(inAlias);
    Array<QueryIdentifier> _identifiers = getFromList();
    for (Uint32 i = 0; i < _identifiers.size(); i++)
    {
        if (_aliasName == _identifiers[i].getName())
            return _identifiers[i];
    }

    // could not find inAlias
    return QueryIdentifier();
}

Array<QueryIdentifier> QueryContext::getFromList() const
{
    return _fromList;
}

String QueryContext::getFromString() const
{
    Array<QueryIdentifier> aliasedClasses;
    Array<String> aliases;
    for (HT_Alias_Class::Iterator i = _AliasClassTable.start(); i; i++)
    {
        aliases.append(i.key());
        aliasedClasses.append(i.value());
    }

    String result("FROM ");
    for (Uint32 i = 0; i < _fromList.size(); i++)
    {
        Boolean hasAlias = false;
        for (Uint32 j = 0; j < aliasedClasses.size(); j++)
        {
            if (_fromList[i].getName() == aliasedClasses[j].getName())
            {
                result.append(_fromList[i].getName().getString());
                result.append(" AS ");
                result.append(aliases[j]);
                hasAlias = true;
            }
        }

        if (!hasAlias)
        {
            result.append(_fromList[i].getName().getString());
        }

        if (i < _fromList.size() - 1)
        {
            result.append(" , ");
        }
        else
        {
            result.append(" ");
        }
    }

    return result;
}

void QueryContext::clear()
{
    _fromList.clear();
    _whereList.clear();
    _AliasClassTable.clear();
}

PEGASUS_NAMESPACE_END

