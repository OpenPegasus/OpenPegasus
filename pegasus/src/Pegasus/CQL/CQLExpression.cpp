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
// Author: Dave Rosckes (rosckes@us.ibm.com)
//
// Modified By: Vijay Eli, IBM (vijayeli@in.ibm.com) bug#3590.
//
//%/////////////////////////////////////////////////////////////////////////////

#include <Pegasus/CQL/CQLExpression.h>
#include <Pegasus/CQL/CQLExpressionRep.h>
#include <Pegasus/CQL/CQLFactory.h>
#include <Pegasus/Query/QueryCommon/QueryContext.h>

PEGASUS_NAMESPACE_BEGIN

/*
#define PEGASUS_ARRAY_T TermOpType
#include <Pegasus/Common/ArrayImpl.h>
#undef PEGASUS_ARRAY_T

#define PEGASUS_ARRAY_T CQLExpression
#include <Pegasus/Common/ArrayImpl.h>
*/

CQLExpression::CQLExpression()
{
    _rep = new CQLExpressionRep();
}

CQLExpression::CQLExpression(const CQLTerm& theTerm)
{
   _rep = new CQLExpressionRep(theTerm);

}

CQLExpression::CQLExpression(const CQLExpression& inExpress)
{
    _rep = new CQLExpressionRep(inExpress._rep);
}

CQLExpression::~CQLExpression()
{
    if(_rep)
        delete _rep;
}

CQLValue CQLExpression::resolveValue(const CIMInstance& CI,
                                     const QueryContext& QueryCtx)
{
    return _rep->resolveValue(CI,QueryCtx);
}

void CQLExpression::appendOperation(const TermOpType theTermOpType,
                                    const CQLTerm& theTerm)
{
    _rep->appendOperation(theTermOpType,theTerm);
}

String CQLExpression::toString()const
{
   return _rep->toString();
}
Boolean CQLExpression::isSimple()const
{
        return _rep->isSimple();
}
Boolean CQLExpression::isSimpleValue()const
{
    return _rep->isSimpleValue();
}

Array<CQLTerm> CQLExpression::getTerms()const
{
   return _rep->getTerms();
}

Array<TermOpType> CQLExpression::getOperators()const
{
   return _rep->getOperators();
}

void CQLExpression::applyContext(const QueryContext& inContext,
                                 const CQLChainedIdentifier& inCid)
{
  _rep->applyContext(inContext, inCid);
}

CQLExpression& CQLExpression::operator=(const CQLExpression& rhs)
{
    if(&rhs != this){
        if(_rep) delete _rep;
        _rep = new CQLExpressionRep(rhs._rep);
    }
    return *this;
}
/*
Boolean CQLExpression::operator==(const CQLExpression& expr)const
{
    return *_rep == *(expr._rep);
}

Boolean CQLExpression::operator!=(const CQLExpression& expr)const
{
    return (!operator==(expr));
}
*/
PEGASUS_NAMESPACE_END

