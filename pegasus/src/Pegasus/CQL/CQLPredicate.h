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

#ifndef Pegasus_CQLPredicate_h
#define Pegasus_CQLPredicate_h

#ifdef PEGASUS_USE_EXPERIMENTAL_INTERFACES

#include <Pegasus/CQL/Linkage.h>
#include <Pegasus/CQL/CQLSimplePredicate.h>
#include <Pegasus/Common/ArrayInternal.h>

PEGASUS_NAMESPACE_BEGIN

enum BooleanOpType { AND, OR };

/*
#ifndef PEGASUS_ARRAY_T
#define PEGASUS_ARRAY_T BooleanOpType
#include <Pegasus/Common/ArrayInter.h>
#undef PEGASUS_ARRAY_T
#endif
*/

class CQLPredicateRep;
class CQLFactory;
class QueryContext;

/**
    The CQLPredicate class contains an expression that produces a boolean
    result.

    <PRE>
    The CQLPredicate is non-simple if it contains only CQLPredicate objects.
    Example: a CQLPredicate representing (a=1 OR b=2) would contain
    two CQLPredicates, one representing (a=1), and the other
    representing (b=2).  These CQLPredicates would in turn be simple.

    The CQLPredicate is simple if it contains a single CQLSimplePredicate,
    representing an arithmetic or string expression.
    Examples:  a = 2
               (a-b)/ 3 = c
               a || b = 'abc'
    </PRE>
*/
class PEGASUS_CQL_LINKAGE CQLPredicate
{
  public:
    /**
        Default constructor

        @param  -  None.
        @return - None.
        @throws - None.
        <I><B>Experimental Interface</B></I><BR>
    */
    CQLPredicate();

    /**
        Constructor. Using this constructor sets isSimple() to true.

        @param  -  inSimplePredicate.
        @param  -  inVerted. Defaults to false.  This is a logical NOT of
                   this predicate
        @return - None.
        @throws - None.
        <I><B>Experimental Interface</B></I><BR>
    */
    CQLPredicate(const CQLSimplePredicate & inSimplePredicate,
            Boolean inVerted = false);

    /**
        Constructor. Using this constructor sets isSimple() to false.

        @param  -  inPredicate.
        @param  -  inVerted. Defaults to false.  This is a logical NOT of
                   this predicate
        @return - None.
        @throws - None.
        <I><B>Experimental Interface</B></I><BR>
    */
    CQLPredicate(const CQLPredicate & inPredicate, Boolean inVerted = false);

    /**
        Destructor

        @param  - None.
        @return - None.
        @throws - None.
        <I><B>Experimental Interface</B></I><BR>
    */
   ~CQLPredicate();

    /**
         Evaluates this predicate, using a CIMInstance as a property source.

         <PRE>
         The CQLPredicate is non-simple if it contains only CQLPredicate
         objects.
         A non-simple CQLPredicate is evaluated by in turn evaluating the
         contained CQLPredicates and boolean operator.

         For the evaluate method on each CQLPredicate. the CQLPredicate is
         evaluated to TRUE/FALSE and the result of the evaluation is then
         applied to the appropriate boolean operator.

         The result of the evaluation is then inverted if the _invert member
         variable is set to TRUE and then returned to the caller.

         The CQLPredicate is simple if it contains only a single
         CQLSimplePredicate.
         A simple CQLPredicate is evaluated by calling evaluate on
         the CQLSimplePredicate.
         </PRE>

         @param  - CI. Instance to evaluate query against.
         @param  - QueryCtx. Query Context
         @return - Boolean.
         @throws - None.
         <I><B>Experimental Interface</B></I><BR>
       */
    Boolean evaluate(CIMInstance CI, QueryContext& QueryCtx);

    /**
        Returns true if this predicate has been inverted (NOT), false otherwise

        @param  - None.
        @return - Boolean.
        @throws - None.
        <I><B>Experimental Interface</B></I><BR>
    */
    Boolean getInverted()const;

    /**
        Sets the inverted flag.  Logically NOTting this predicate.

        @param  - None.
        @return - None.
        @throws - None.
        <I><B>Experimental Interface</B></I><BR>
    */
    void setInverted(Boolean invert);

    /**
        Appends a predicate to the predicate array. This method should only
        be called by Bison.

        @param  - inPredicate.
        @return - None.
        @throws - None.
        <I><B>Experimental Interface</B></I><BR>
    */
    void appendPredicate(const CQLPredicate& inPredicate);

    /**
        Appends a predicate to the predicate array. This method should only
        be called by Bison.

        @param  - inPredicate.
        @param  - inBooleanOperator
        @return - None.
        @throws - None.
        <I><B>Experimental Interface</B></I><BR>
    */
    void appendPredicate(const CQLPredicate& inPredicate,
            BooleanOpType inBooleanOperator);

    /**
        Gets the CQLPredicates contained within this CQLPredicate.

        @param  - None.
        @return - Array<CQLPredicate>.
        @throws - None.
        <I><B>Experimental Interface</B></I><BR>
    */
    Array<CQLPredicate> getPredicates()const;

     /**
        Gets the CQLSimplePredicate contained within this CQLPredicate.
        Only valid if this isSimple() = true.

        @param  - None.
        @return - CQLSimplePredicate.
        @throws - None.
        <I><B>Experimental Interface</B></I><BR>
    */
    CQLSimplePredicate getSimplePredicate()const;

     /**
        Gets the Operators for this CQLPredicate.
        Given P1 AND P2 OR P3, this would return [AND][OR]

        @param  - None.
        @return - Array<BooleanOpType>.
        @throws - None.
        <I><B>Experimental Interface</B></I><BR>
    */
    Array<BooleanOpType> getOperators()const;

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
        Returns true if this CQLPredicate contains no CQLPredicates,
        ie. only has one CQLSimplePredicate.

        @param  - None.
        @return - Boolean.
        @throws - None.
        <I><B>Experimental Interface</B></I><BR>
    */
    Boolean isSimple()const;

     /**
        Returns true if this CQLPredicate is simple and the contained
        CQLSimplePredicate is a simple value (isSimpleValue( ) returns
        true).

        @param  - None.
        @return - Boolean.
        @throws - None.
        <I><B>Experimental Interface</B></I><BR>
    */
    Boolean isSimpleValue()const;

    /**
       Returns this predicate in string form.

        @param  - None.
        @return - string form of predicate.
        @throws - None.
        <I><B>Experimental Interface</B></I><BR>
    */
    String toString()const;

     /**
        Operator=

        @param  - rhs. Right hand side of assignment operator.
        @return - CQLPredicate&.
        @throws - None.
        <I><B>Experimental Interface</B></I><BR>
    */
    CQLPredicate& operator=(const CQLPredicate& rhs);

    friend class CQLFactory;
  private:
        CQLPredicateRep* _rep;

};

/*
#ifndef PEGASUS_ARRAY_T
#define PEGASUS_ARRAY_T CQLPredicate
#include <Pegasus/Common/ArrayInter.h>
#undef PEGASUS_ARRAY_T
#endif
*/

PEGASUS_NAMESPACE_END
#endif
#endif
