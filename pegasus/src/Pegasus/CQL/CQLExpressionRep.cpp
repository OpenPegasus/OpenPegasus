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

#include <Pegasus/CQL/CQLExpression.h>
#include <Pegasus/CQL/CQLExpressionRep.h>
#include <Pegasus/CQL/CQLFactory.h>
#include <Pegasus/Query/QueryCommon/QueryContext.h>
#include <Pegasus/Common/Tracer.h>

PEGASUS_NAMESPACE_BEGIN

CQLExpressionRep::CQLExpressionRep(const CQLTerm& theTerm)
{
  PEG_METHOD_ENTER(TRC_CQL,
      "CQLExpressionRep::CQLExpressionRep(const CQLTerm& theTerm)");
  _CQLTerms.append(theTerm);
  PEG_METHOD_EXIT();
}

CQLExpressionRep::CQLExpressionRep(const CQLExpressionRep* rep)
{
  PEG_METHOD_ENTER(TRC_CQL,
      "CQLExpressionRep::CQLExpressionRep(const CQLExpressionRep* rep)");

  _TermOperators = rep->_TermOperators;
  _CQLTerms = rep->_CQLTerms;

  PEG_METHOD_EXIT();
}

CQLExpressionRep::~CQLExpressionRep()
{

}
CQLValue CQLExpressionRep::resolveValue(const CIMInstance& CI,
                                        const QueryContext& QueryCtx)
{
  PEG_METHOD_ENTER(TRC_CQL,"CQLExpressionRep::resolveValue()");

  CQLValue returnVal = _CQLTerms[0].resolveValue(CI,QueryCtx);

  /*
    for(Uint32 i = 0; i < _TermOperators.size(); ++i)
    {
      switch(_TermOperators[i])
        {
           case TERM_ADD:
               returnVal = returnVal +
               _CQLTerms[i+1].resolveValue(CI,QueryCtx);
           break;
               case TERM_SUBTRACT:
               returnVal = returnVal -
               _CQLTerms[i+1].resolveValue(CI,QueryCtx);
               break;
        default:
            throw(1);
        }
    }
*/

  PEG_METHOD_EXIT();
  return returnVal;
}

void CQLExpressionRep::appendOperation(const TermOpType theTermOpType,
                                       const CQLTerm& theTerm)
{
    PEG_METHOD_ENTER(TRC_CQL,"CQLExpressionRep::appendOperation()");

    _TermOperators.append(theTermOpType);
    _CQLTerms.append(theTerm);

    PEG_METHOD_EXIT();
}

String CQLExpressionRep::toString()const
{
    PEG_METHOD_ENTER(TRC_CQL,"CQLExpressionRep::toString()");

    String returnStr;

    if(_CQLTerms.size() > 0)
    {
        returnStr.append(_CQLTerms[0].toString());
        /* for(Uint32 i = 0; i < _TermOperators.size(); ++i)
          {
            returnStr.append(_TermOperators[i] ==
                TERM_ADD ? String(" + ") : String(" - "));
        returnStr.append(_CQLTerms[i+1].toString());
          }
        */
    }

    PEG_METHOD_EXIT();
    return returnStr;
    }


Boolean CQLExpressionRep::isSimple()const
{
   return (_CQLTerms.size() == 1);
}

Boolean CQLExpressionRep::isSimpleValue()const
{
  PEG_METHOD_ENTER(TRC_CQL,"CQLExpressionRep::isSimpleValue()");

  if(_CQLTerms.size() == 1)
    {
      PEG_METHOD_EXIT();
      return _CQLTerms[0].isSimpleValue();
    }

  PEG_METHOD_EXIT();

  return false;
}

Array<CQLTerm> CQLExpressionRep::getTerms()const
{
   return _CQLTerms;
}

Array<TermOpType> CQLExpressionRep::getOperators()const
{
   return _TermOperators;
}

void CQLExpressionRep::applyContext(const QueryContext& inContext,
                                    const CQLChainedIdentifier& inCid)
{
  PEG_METHOD_ENTER(TRC_CQL,"CQLExpressionRep::applyContext()");

  for(Uint32 i = 0; i < _CQLTerms.size(); ++i)
  {
    _CQLTerms[i].applyContext(inContext, inCid);
  }

  PEG_METHOD_EXIT();
}
/*
Boolean CQLExpressionRep::operator==(const CQLExpressionRep& rep)const
{
  PEG_METHOD_ENTER(TRC_CQL,"CQLExpressionRep::operator==()");

  if(_isSimple != rep._isSimple)
    {
      PEG_METHOD_EXIT();
      return false;
    }

  for(Uint32 i = 0; i < _TermOperators.size(); ++i)
    {
      if(_TermOperators[i] != rep._TermOperators[i])
    {
      PEG_METHOD_EXIT();
      return false;
    }
    }

  for(Uint32 i = 0; i < _CQLTerms.size(); ++i)
    {
      if(_CQLTerms[i] != rep._CQLTerms[i])
    {
      PEG_METHOD_EXIT();
      return false;
    }
    }

  PEG_METHOD_EXIT();
  return true;
}

Boolean CQLExpressionRep::operator!=(const CQLExpressionRep& rep)const
{
  return (!operator==(rep));
}
*/
PEGASUS_NAMESPACE_END

