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
//
//%/////////////////////////////////////////////////////////////////////////////

#include <Pegasus/Common/InternalException.h>
#include "CQLPredicate.h"
#include "CQLSimplePredicate.h"
#include "CQLPredicateRep.h"
#include <Pegasus/CQL/CQLFactory.h>
#include <Pegasus/Query/QueryCommon/QueryContext.h>
#include <Pegasus/Common/Tracer.h>

PEGASUS_NAMESPACE_BEGIN

CQLPredicateRep::CQLPredicateRep():
    _invert(false)
{

}

CQLPredicateRep::CQLPredicateRep(const CQLSimplePredicate& inSimplePredicate,
    Boolean inVerted) :
    _simplePredicate(inSimplePredicate), _invert(inVerted)
{

}

CQLPredicateRep::CQLPredicateRep(const CQLPredicate& inPredicate,
    Boolean inInverted) :
    _invert(inInverted)
{
    _predicates.append(inPredicate);
}

CQLPredicateRep::CQLPredicateRep(const CQLPredicateRep* rep):
    _invert(false)
{
    _predicates = rep->_predicates;
    _simplePredicate = rep->_simplePredicate;
    _operators = rep->_operators;
    _invert = rep->_invert;
}

Boolean CQLPredicateRep::evaluate(CIMInstance CI, QueryContext& QueryCtx)
{
    PEG_METHOD_ENTER(TRC_CQL, "CQLIPredicateRep::evaluate");
    Boolean result = false;

    if (isSimple())
    {
    result = _simplePredicate.evaluate(CI, QueryCtx);
    }
    else
    {
        result = _predicates[0].evaluate(CI, QueryCtx);
        for (Uint32 i = 0; i < _operators.size(); i++)
        {
            if (_operators[i] == AND)
            {
                if(result)
                {
                    result = _predicates[i+1].evaluate(CI, QueryCtx);
                }
            }
            else
            {
                if(result)
                {
                    break;
                }
                else
                {
                    result = _predicates[i+1].evaluate(CI, QueryCtx);
                }
            }
        }
    }
    PEG_METHOD_EXIT();
    return (getInverted()) ? !result : result;
}

Boolean CQLPredicateRep::getInverted()const{
    return _invert;
}

void CQLPredicateRep::setInverted(Boolean invert){
    _invert = invert;
}

void CQLPredicateRep::appendPredicate(const CQLPredicate& inPredicate){
    _predicates.append(inPredicate);
}

void CQLPredicateRep::appendPredicate(const CQLPredicate& inPredicate,
    BooleanOpType inBooleanOperator)
{
    _predicates.append(inPredicate);
    _operators.append(inBooleanOperator);
}

Array<CQLPredicate> CQLPredicateRep::getPredicates()const{
    return _predicates;
}

CQLSimplePredicate CQLPredicateRep::getSimplePredicate()const{
    return _simplePredicate;
}

Array<BooleanOpType> CQLPredicateRep::getOperators()const{
    return _operators;
}

void CQLPredicateRep::applyContext(const QueryContext& queryContext)
{
    PEG_METHOD_ENTER(TRC_CQL, "CQLPredicateRep::applyContext");
    if (isSimple())
    {
        _simplePredicate.applyContext(queryContext);
    }
    else
    {
        for (Uint32 i = 0; i <_predicates.size(); i++)
        {
            _predicates[i].applyContext(queryContext);
        }
    }
    PEG_METHOD_EXIT();
}

Boolean CQLPredicateRep::isSimple()const{
    return (_predicates.size() == 0);
}

Boolean CQLPredicateRep::isSimpleValue()const{
    return (isSimple() && _simplePredicate.isSimpleValue());
}

String CQLPredicateRep::toString()const{
    if(isSimple())
    {
        String s;
      if(_invert) s = "NOT ";
      s.append(_simplePredicate.toString());
      return s;
    }
    String s;
    if(_invert) s = "NOT ";
    for(Uint32 i = 0; i < _predicates.size(); i++)
    {
        s.append(_predicates[i].toString());
        if(i < _operators.size())
        {
            switch(_operators[i])
            {
                case AND: s.append(" AND ");
                    break;
                case OR: s.append(" OR ");
                    break;
            }
        }
    }
    return s;
}

PEGASUS_NAMESPACE_END
