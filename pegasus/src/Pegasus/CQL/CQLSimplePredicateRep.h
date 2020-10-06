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

#ifndef Pegasus_CQLSimplePredicateRep_h
#define Pegasus_CQLSimplePredicateRep_h

#include <Pegasus/CQL/Linkage.h>
#include <Pegasus/CQL/CQLExpression.h>

//#include <Pegasus/Common/CIMInstance.h>
//#include <Pegasus/CQL/QueryContext.h>

PEGASUS_NAMESPACE_BEGIN

class CQLExpression;
class CQLFactory;
class QueryContext;

class CQLSimplePredicateRep
{
  public:
    CQLSimplePredicateRep();

    CQLSimplePredicateRep(const CQLExpression& inExpression);

    CQLSimplePredicateRep(const CQLExpression& inExpression,
            ExpressionOpType inOperator);

    CQLSimplePredicateRep(const CQLExpression& leftSideExpression,
                          const CQLExpression& rightSideExpression,
                          ExpressionOpType inOperator);

    CQLSimplePredicateRep(const CQLSimplePredicateRep* rep);

    ~CQLSimplePredicateRep(){}

    /**
      CQLExpressions:
          For an expression, CQLExpression::getValue is called and will
          return a CQLValue.
          The appropriate operator is then invoked on CQLValue and that
          operator function will enforce the type restrictions as documented
          in the DMTF CQL Specification.
          That operator then determines whether the predicate is TRUE / FALSE.


       CQLPredicates:
          The CQLPredicate is non-terminal if it contains only CQLPredicate
          objects.
          A non-terminal CQLPredicate is evaluated by in turn evaluating the
          contained CQLPredicates and boolean operator.
          Valid operators are:
                  AND, OR

          For the evaluate method on each CQLPredicate. the CQLPredicate is
          evaluated to TRUE/FALSE and the result of the evaluation is then
          applied to the appropriate boolean operator.

          The result of the evaluation is and then inverted if the _invert
          member variable is set to TRUE and then returned to the caller.

      */
    Boolean evaluate(CIMInstance CI, QueryContext& QueryCtx);

    CQLExpression getLeftExpression()const;

    CQLExpression getRightExpression()const;

    ExpressionOpType getOperation()const;

    void applyContext(const QueryContext& queryContext);

    String toString()const;

    Boolean isSimple()const;

    Boolean isSimpleValue()const;

    void setOperation(ExpressionOpType op);

    friend class CQLFactory;

private:
    CQLExpression _leftSide;

    CQLExpression _rightSide;

    ExpressionOpType _operator;
    Boolean _isSimple;
};
PEGASUS_NAMESPACE_END

#endif
