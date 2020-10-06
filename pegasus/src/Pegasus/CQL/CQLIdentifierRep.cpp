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

//#include "CQLIdentifier.h"
#include "CQLIdentifierRep.h"
#include <Pegasus/Query/QueryCommon/QueryIdentifierRep.h>
#include <Pegasus/Query/QueryCommon/QueryException.h>
#include <Pegasus/Common/Tracer.h>
PEGASUS_NAMESPACE_BEGIN


Char16 CQLIdentifierRep::STAR = '*';
Char16 CQLIdentifierRep::HASH = '#';
Char16 CQLIdentifierRep::RBRKT = ']';
Char16 CQLIdentifierRep::LBRKT = '[';
const char CQLIdentifierRep::SCOPE[] = "::";

CQLIdentifierRep::CQLIdentifierRep():
    QueryIdentifierRep()
{

}

CQLIdentifierRep::CQLIdentifierRep(const String& identifier):
    QueryIdentifierRep()
{
    _isWildcard = false;
    _isSymbolicConstant = false;
    parse(identifier);
}

CQLIdentifierRep::CQLIdentifierRep(const CQLIdentifierRep* rep):
    QueryIdentifierRep()
{
    _symbolicConstant = rep->_symbolicConstant;
    _scope = rep->_scope;
    _indices = rep->_indices;
    _name = rep->_name;
    _isWildcard = rep->_isWildcard;
    _isSymbolicConstant = rep->_isSymbolicConstant;
}

CQLIdentifierRep::~CQLIdentifierRep()
{

}

CQLIdentifierRep& CQLIdentifierRep::operator=(const CQLIdentifierRep& rhs)
{
    _symbolicConstant = rhs._symbolicConstant;
    _scope = rhs._scope;
    _indices = rhs._indices;
    _name = rhs._name;
    _isWildcard = rhs._isWildcard;
    _isSymbolicConstant = rhs._isSymbolicConstant;
    return *this;
}

void CQLIdentifierRep::parse(String identifier)
{
    PEG_METHOD_ENTER(TRC_CQL, "CQLIdentifierRep::parse");
    /*
     - Parse for the following:
         1. A::<scoped string>
     (a)  property name
         (b)  property[3]     e.g. an array index
         (c)  property#'OK'    e.g. a symbolic constant
         (d)  *   (wildcard)
     (e)  class name
     (f)  embedded object
     (g)  namespace
    */

    Uint32 index;
    Boolean hasCIMName = true;
    if(identifier == String::EMPTY)
    {
        _name = CIMName();
        return;
    }
    // basic error check
    if((index = identifier.find(HASH)) != PEG_NOT_FOUND)
    {
        if(((index = identifier.find(RBRKT)) != PEG_NOT_FOUND) ||
            ((index = identifier.find(LBRKT)) != PEG_NOT_FOUND))
        {
            //error
            MessageLoaderParms parms(
                "CQL.CQLIdentifier.HASH_ARRAY_SYMBOL_MISMATCH",
                "The identifier contains a mismatched symbolic"
                    " constant symbol and an array symbol: $0",
                identifier);
            throw CQLIdentifierParseException(parms);
        }
    }

    String _SCOPE(SCOPE);
    if((index = identifier.find(_SCOPE)) != PEG_NOT_FOUND)
    {
        _scope = identifier.subString(0,index);
        identifier = identifier.subString(index+2);
    }

    if((index = identifier.find(RBRKT)) != PEG_NOT_FOUND)
    {
        if((index = identifier.find(LBRKT)) != PEG_NOT_FOUND)
        {
            // found array index, parse for ','
            String range = identifier.subString(index);
            // remove left and right bracket
            range = range.subString(1,range.size()-2);
            while(index != PEG_NOT_FOUND){
            if((index = range.find(',')) != PEG_NOT_FOUND)
            {
                // Basic query error
                MessageLoaderParms parms(
                    "CQL.CQLIdentifier.TOO_MANY_ARRAY_INDICES",
                    "The identifier contains one or more commas which"
                        " is not allowed in CQL Basic query: $0",
                    identifier);
                throw CQLIdentifierParseException(parms);
                //
                // For basic query the following lines are disabled
                // An exception is thrown if we have ',' in the array range
                //
                //_indices.append(SubRange(range.subString(0,index)));
                //range = range.subString(index+1);
            }else
            {
                _indices.append(SubRange(range));
            }
          }
          // remove ranges from identifier
          identifier = identifier.subString(0,identifier.find(LBRKT));
        }
        else
        {
          // error
            MessageLoaderParms parms(
                "CQL.CQLIdentifier.ARRAY_SYMBOL_MISMATCH",
                "The identifier contains a mismatched array symbol: $0",
                identifier);
            throw CQLIdentifierParseException(parms);
        }
    }
    else if((index = identifier.find(STAR)) != PEG_NOT_FOUND)
    {
        // wildcard
        _isWildcard = true;
    }
    else if((index = identifier.find(HASH)) != PEG_NOT_FOUND)
    {
        // symbolic constant

        // check if only have a symbolic constant without a leading identifier
        if(index == 0) hasCIMName = false;

        _isSymbolicConstant = true;
        _symbolicConstant = identifier.subString(index+1);
        identifier = identifier.subString(0,index);
    }

    // name
    if(!_isWildcard)
    {
        try
        {
            if(hasCIMName)
                _name = CIMName(identifier);
        }
        catch (Exception&)
        {
            MessageLoaderParms parms(
                "CQL.CQLIdentifier.INVALID_CIMNAME",
                "The identifier contains an invalid CIMName: $0.",
                identifier);
            throw CQLIdentifierParseException(parms);
        }
    }

    PEG_METHOD_EXIT();
}

PEGASUS_NAMESPACE_END
