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

#ifndef Pegasus_CQLSimplePredicate_h
#define Pegasus_CQLSimplePredicate_h

#include <Pegasus/CQL/Linkage.h>
#include <Pegasus/Common/CIMInstance.h>
#include <Pegasus/Query/QueryCommon/QueryContext.h>

#ifdef PEGASUS_USE_EXPERIMENTAL_INTERFACES

PEGASUS_NAMESPACE_BEGIN

class CQLExpression;
class CQLFactory;
class CQLSimplePredicateRep;

enum ExpressionOpType { LT, GT, EQ, LE, GE, NE, IS_NULL,
                        IS_NOT_NULL, ISA, LIKE, NOOP };


/**
    The CQLSimplePredicate class contains an arithmetic or string
    expression that produces a boolean result.

    <PRE>
    The CQLSimplePredicate is non-simple if it contains a
    left-side and right-side CQLExpression.
    Example: a CQLSimplePredicate representing a < b would contain
    a left-side CQLExpression representing (a), a < operator,
    and a right-side CQLExpression representing (b).

    The CQLSimplePredicate is simple if it contains just a left-side
    CQLExpression and an operator.
    Example:  a IS NULL
    </PRE>
*/

class PEGASUS_CQL_LINKAGE CQLSimplePredicate
{
  public:
    /**
        Default constructor

        @param  -  None.
        @return - None.
        @throws - None.
        <I><B>Experimental Interface</B></I><BR>
    */
    CQLSimplePredicate();

    /**
        Constructor. Using this constructor sets isSimple() to true.

        @param  -  inExpression.
        @return - None.
        @throws - None.
        <I><B>Experimental Interface</B></I><BR>
    */
    CQLSimplePredicate(const CQLExpression& inExpression);

    /**
        Constructor. Using this constructor sets isSimple() to true.

        @param  -  inExpression.
        @param  -  inOperator The unary operator.
        @return - None.
        @throws - None.
        <I><B>Experimental Interface</B></I><BR>
    */
    CQLSimplePredicate(const CQLExpression& inExpression,
                       ExpressionOpType inOperator);

    /**
        Constructor.

        @param  -  leftSideExpression.
        @param  -  rightSideExpression.
        @param  -  inOperator The binary operator.
        @return - None.
        @throws - None.
        <I><B>Experimental Interface</B></I><BR>
    */
    CQLSimplePredicate(const CQLExpression& leftSideExpression,
                       const CQLExpression& rightSideExpression,
                       ExpressionOpType inOperator);

    /**
        Copy Constructor.

        @param  -  inSimplePredicate. Object to copy.
        @return - None.
        @throws - None.
        <I><B>Experimental Interface</B></I><BR>
    */
     CQLSimplePredicate(const CQLSimplePredicate& inSimplePredicate);

    /**
        Destructor.

        <I><B>Experimental Interface</B></I><BR>
    */
    ~CQLSimplePredicate();

    /**
         Evaluates this predicate, using a CIMInstance as a property source.

         <PRE>
         The CQLSimplePredicate is non-simple if it contains left and
         right side expressions. A non-simple CQLSimplePredicate is evaluated
         by in turn evaluating the contained CQLExpressions and then applying
         the operator.

         For the evaluate method on each CQLExpression, the CQLExpression is
         resolved to a CQLValue and the value is then applied to the operator.

         The CQLSimplePredicate is simple if it contains only a left-side
         CQLExpression.
         A simple CQLSimplePredicate is evaluated by resolving the
         CQLExpression into a CQLValue and then applying the operator.
         </PRE>

         @param  - CI. Instance to evaluate query against.
         @param  - QueryCtx. Query Context
         @return - Boolean.
         @throws - None.
         <I><B>Experimental Interface</B></I><BR>
    */
    Boolean evaluate(CIMInstance CI, QueryContext& QueryCtx);

    /**
        Returns the left-side expression.

        @return - left-side expresson
        @throws - None.
        <I><B>Experimental Interface</B></I><BR>
    */
    CQLExpression getLeftExpression()const;

    /**
        Returns the right-side expression.

        @return - right-side expresson
        @throws - None.
        <I><B>Experimental Interface</B></I><BR>
    */
    CQLExpression getRightExpression()const;

    /**
        Returns the operator.

        @return - operator for the expressions
        @throws - None.
        <I><B>Experimental Interface</B></I><BR>
    */
    ExpressionOpType getOperation()const;

    /**
        Sets the operator.

        @param  - op The operator to set.
        @return - None
        @throws - None.
        <I><B>Experimental Interface</B></I><BR>
    */
    void setOperation(ExpressionOpType op);

     /**
        This method normalizes the CQLChainedIdentifier so that properties
        that require scoping are scoped.

        @param  - queryContext.
        @return - None.
        @throws - None.
        <I><B>Experimental Interface</B></I><BR>
    */
    void applyContext(const QueryContext& queryContext);

    /**
       Returns this predicate in string form.

        @param  - None.
        @return - string form of predicate.
        @throws - None.
        <I><B>Experimental Interface</B></I><BR>
    */
    String toString()const;

    /**
       Returns true if this CQLSimplePredicate only has a left-side
       CQLExpression.

       @param  - None.
       @return - Boolean.
       @throws - None.
       I><B>Experimental Interface</B></I><BR>
    */
    Boolean isSimple()const;

    /**
       Returns true if this CQLSimplePredicate is simple, and
       the left-side CQLExpression only contains a CQLValue.

       @param  - None.
       return - Boolean.
       @throws - None.
       <I><B>Experimental Interface</B></I><BR>
    */
    Boolean isSimpleValue()const;

    /**
       Assignment operator.

       @param  - rhs. Object to assign to this object.
       return - This object after assignment.
       @throws - None.
       <I><B>Experimental Interface</B></I><BR>
    */
    CQLSimplePredicate& operator=(const CQLSimplePredicate& rhs);

    friend class CQLFactory;
  private:
    CQLSimplePredicateRep *_rep;

};
PEGASUS_NAMESPACE_END
#endif
#endif
