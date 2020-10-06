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

#include "CQLChainedIdentifier.h"
#include "CQLChainedIdentifierRep.h"
#include <Pegasus/CQL/CQLFactory.h>
#include <Pegasus/Common/Tracer.h>
#include <Pegasus/Query/QueryCommon/QueryContext.h>
#include <Pegasus/Query/QueryCommon/QueryIdentifier.h>
#include <Pegasus/Query/QueryCommon/QueryException.h>

PEGASUS_NAMESPACE_BEGIN

CQLChainedIdentifierRep::CQLChainedIdentifierRep():
  QueryChainedIdentifierRep()
{
}

CQLChainedIdentifierRep::CQLChainedIdentifierRep(const String& inString):
  QueryChainedIdentifierRep()
{
    parse(inString);
}

CQLChainedIdentifierRep::CQLChainedIdentifierRep(const CQLIdentifier &id):
  QueryChainedIdentifierRep()
{
  _subIdentifiers.append(id);
}

CQLChainedIdentifierRep::CQLChainedIdentifierRep(
  const CQLChainedIdentifierRep* rep)
  :
  QueryChainedIdentifierRep()
{
    _subIdentifiers = rep->_subIdentifiers;
}

CQLChainedIdentifierRep::~CQLChainedIdentifierRep(){

}

Array<CQLIdentifier> CQLChainedIdentifierRep::getSubIdentifiers()const
{
  Array<CQLIdentifier> result;

  for (Uint32 i = 0; i < _subIdentifiers.size(); i++)
  {
    result.append(CQLIdentifier(_subIdentifiers[i]));
  }

    return result;
}

CQLIdentifier CQLChainedIdentifierRep::getLastIdentifier()const
{
    if(_subIdentifiers.size() > 0)
        return CQLIdentifier(_subIdentifiers[_subIdentifiers.size()-1]);
    return CQLIdentifier();
}

CQLIdentifier CQLChainedIdentifierRep::operator[](Uint32 index)const
{
    return CQLIdentifier(_subIdentifiers[index]);
}

void CQLChainedIdentifierRep::parse(const String & string)
{
    PEG_METHOD_ENTER(TRC_CQL, "CQLChainedIdentifierRep::parse");
    /*
      - parse string on "."
      - start from the end of string
      - if more than one substring found,
        -- store first found string then
        -- prepend remaining substrings
    */
    Char16 delim('.');
    Uint32 index;
    String range;

    index = string.reverseFind(delim);
    if(index == PEG_NOT_FOUND){
        _subIdentifiers.append(CQLIdentifier(string));
    }else{
        String tmp = string.subString(index+1);
        _subIdentifiers.append(CQLIdentifier(tmp));

        while(index != PEG_NOT_FOUND){
            tmp = string.subString(0,index);
            index = tmp.reverseFind(delim);
            if(index == PEG_NOT_FOUND){
                _subIdentifiers.prepend(CQLIdentifier(tmp));
            }
            else{
                _subIdentifiers.prepend(CQLIdentifier(tmp.subString(index+1)));
            }
        }
    }
    PEG_METHOD_EXIT();
}

PEGASUS_NAMESPACE_END
