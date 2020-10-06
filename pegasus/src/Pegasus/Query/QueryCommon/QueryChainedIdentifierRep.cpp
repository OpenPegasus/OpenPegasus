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

#include "QueryChainedIdentifier.h"
#include "QueryChainedIdentifierRep.h"
#include <Pegasus/Query/QueryCommon/QueryException.h>

PEGASUS_NAMESPACE_BEGIN

QueryChainedIdentifierRep::QueryChainedIdentifierRep()
{
}

QueryChainedIdentifierRep::QueryChainedIdentifierRep(const QueryIdentifier &id)
{
  _subIdentifiers.append(id);
}

QueryChainedIdentifierRep::QueryChainedIdentifierRep(
    const QueryChainedIdentifierRep* rep)
{
    _subIdentifiers = rep->_subIdentifiers;
}

QueryChainedIdentifierRep::~QueryChainedIdentifierRep()
{
}

const Array<QueryIdentifier>&
    QueryChainedIdentifierRep::getSubIdentifiers() const
{
    return _subIdentifiers;
}

QueryIdentifier QueryChainedIdentifierRep::getLastIdentifier() const
{
    if (_subIdentifiers.size() > 0)
        return _subIdentifiers[_subIdentifiers.size()-1];
    return QueryIdentifier();
}

String QueryChainedIdentifierRep::toString()const
{
    String s;
    for (Uint32 i = 0; i < _subIdentifiers.size(); i++)
    {
        s.append(_subIdentifiers[i].toString());
        if (i < _subIdentifiers.size() - 1)
            s.append(".");
    }
    return s;
}

void QueryChainedIdentifierRep::append(const QueryIdentifier & id)
{
    _subIdentifiers.append(id);
}

Boolean QueryChainedIdentifierRep::isSubChain(
    const QueryChainedIdentifier & chain) const
{
    Array<QueryIdentifier> ids = chain.getSubIdentifiers();
    for(Uint32 i = 0; i < ids.size(); i++)
    {
        if (ids[i] != _subIdentifiers[i].getName())
            return false;
    }
    return true;
}

QueryIdentifier QueryChainedIdentifierRep::operator[](Uint32 index) const
{
    return QueryIdentifier(_subIdentifiers[index]);
}

QueryChainedIdentifierRep& QueryChainedIdentifierRep::operator=(
    const QueryChainedIdentifierRep& rhs)
{
    if (&rhs != this)
    {
        _subIdentifiers = rhs._subIdentifiers;
    }
    return *this;
}

Uint32 QueryChainedIdentifierRep::size()const
{
    return _subIdentifiers.size();
}

Boolean QueryChainedIdentifierRep::prepend(const QueryIdentifier & id)
{
    /*
       Compare id against the first element in _subIdentifiers,
       if not an exact match, then prepend.  This is used to fully
       qualify the chained identifier.
    */
    if (id != _subIdentifiers[0])
    {
        _subIdentifiers.prepend(id);
        return true;
    }
    return false;
}

void QueryChainedIdentifierRep::applyContext(const QueryContext& inContext)
{
    if (_subIdentifiers.size() == 0)
        return;

    // Chained identifiers that are standalone symbolic constants
    // should have had the context applied already.  If this method
    // is called and this is still a standalone symbolic constant,
    // then that is an error.
    if (_subIdentifiers.size() == 1 &&
        _subIdentifiers[0].getName().getString().size() == 0 &&
        _subIdentifiers[0].isSymbolicConstant())
    {
      throw QueryParseException(MessageLoaderParms(
          "QueryCommon.QueryChainedIdentifierRep.STAND_ALONE_SYMBOLIC_CONSTANT",
          "The stand alone chained identifier should have had the context "
              "applied already."));
    }

    QueryIdentifier firstId = _subIdentifiers[0];

    // Process if the first identifier has some contents.
    if ((firstId.getName().getString().size() != 0) || firstId.isWildcard())
    {
        Array<QueryIdentifier> fromList = inContext.getFromList();

        if (firstId.isWildcard())
        {
            // First chain element is wildcarded.
            // Prepend the FROM class.
            _subIdentifiers.prepend(fromList[0]);
        }
        else
        {
            // Not a wildcard.
            if (firstId.isScoped())
            {
                // The first identifier is a scoped property or a
                // scoped symbolic constant.
                // Prepend the FROM class.
                // Example: SELECT * FROM F WHERE someprop = X::p#'OK'
                _subIdentifiers.prepend(fromList[0]);
            }
            else
            {
                // The first identifier is not a scoped property or a scoped
                // symbolic constant.

                // Determine if the first identifier's name is in the FROM list.
                String classContext = firstId.getName().getString();
                QueryIdentifier matchedId = inContext.findClass(classContext);
                if (matchedId.getName().getString().size() == 0)
                {
                    // Could not find the firstId in the FROM list.
                    // Assume the firstId is a property on the FROM class.
                    // Prepend the FROM class, except in the cases described
                    // below.
                    //
                    // NOTE:
                    // We need special handling for symbolic constants, because
                    // there are cases where the first id is a class that is
                    // not the FROM class.
                    // Refer to section 5.4.3 of the CQL spec.
                    //
                    // Examples:
                    // SELECT * FROM F WHERE F.someprop = prop#'OK'
                    // Prepend the FROM class because symbolic constants can
                    // only apply to properties, and properties in the first
                    // position are assumed to be on the FROM class.
                    //
                    // SELECT * FROM F WHERE F.someprop =
                    //     ClassNotInFromList.prop#'OK'
                    // We don't want to prepend the FROM class to
                    // ClassNotInFromList in this case. But we need to get the
                    // FROM class from the schema to make sure
                    // ClassNotInFromList is not a property on the FROM class.
                    //
                    // SELECT * FROM F WHERE F.someprop =
                    //     embeddedObjectOnF.X::p#'OK'
                    // In this case embeddedObjectOnF is an embedded object
                    // property on the FROM class.  We want to prepend the
                    // FROM class.  But we need to get FROM class from the
                    // schema first to make sure.
                    //
                    // SELECT * FROM F WHERE F.someprop =
                    //     embeddedObjectOnF.X::p
                    // In this case embeddedObjectOnF is an embedded object
                    // property on the FROM class.  We want to prepend the
                    // FROM class.  Since the last id is not a symbolic
                    // constant we can prepend without getting the FROM class
                    // from the schema.
                    //
                    // Note that standalone symbolic constants, like #'OK'
                    // are errors when this code is run.
                    // See above for details.
                    //

                    if (firstId.isSymbolicConstant() ||
                        !getLastIdentifier().isSymbolicConstant())
                    {
                        // Must be a property on the FROM class.
                        _subIdentifiers.prepend(fromList[0]);
                    }
                    else
                    {
                        // Get the FROM class.
                        try
                        {
                            CIMClass fromClass =
                                inContext.getClass(fromList[0].getName());
                            if (fromClass.findProperty(firstId.getName()) !=
                                    PEG_NOT_FOUND)
                            {
                                // A property on the FROM class.
                                _subIdentifiers.prepend(fromList[0]);
                            }
                        }
                        catch (const CIMException& ce)
                        {
                            if (ce.getCode() == CIM_ERR_INVALID_CLASS ||
                                ce.getCode() == CIM_ERR_NOT_FOUND)
                            {
                                throw QueryParseException(MessageLoaderParms(
                                    "QueryCommon.QueryChainedIdentifierRep."
                                        "FROM_CLASS_DOES_NOT_EXIST",
                                    "The FROM class $0 does not exist.",
                                    fromList[0].getName().getString()));
                            }
                            else
                            {
                                throw;
                            }
                        }
                    }
                }
                else
                {
                    // The firstId was found in the FROM list, but it could
                    // have been an alias
                    if (!matchedId.getName().equal(firstId.getName()))
                    {
                        // It was an alias.
                        // Replace the alias with the FROM class
                        _subIdentifiers[0] = matchedId;
                    }
                    else
                    {
                        // It was not an alias. Do nothing.
                    }
                }  //else first identifier was in the FROM list
            }  // else first identifier was not scoped
        }  // else first identifier was not wildcard
    }  // first identifier has some contents

    // Go through and replace any aliases on scoping classes
    for (Uint32 i = 0; i < _subIdentifiers.size(); i++)
    {
        if (_subIdentifiers[i].isScoped())
        {
            QueryIdentifier match =
                inContext.findClass(_subIdentifiers[i].getScope());
            if (match.getName().getString().size() != 0)
            {
                // The scoping class was either the FROM class, or an alias
                // of the FROM class.  Replace the scoping class with the
                // FROM class.
                _subIdentifiers[i].applyScope(match.getName().getString());
            }
        }
    }
}

PEGASUS_NAMESPACE_END
