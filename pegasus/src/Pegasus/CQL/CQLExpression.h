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

#ifndef Pegasus_CQLExpression_h
#define Pegasus_CQLExpression_h

#ifdef PEGASUS_USE_EXPERIMENTAL_INTERFACES

#include <Pegasus/CQL/Linkage.h>
#include <Pegasus/CQL/CQLValue.h>
#include <Pegasus/Common/ArrayInternal.h>
#include <Pegasus/CQL/CQLTerm.h>

PEGASUS_NAMESPACE_BEGIN

class CQLFactory;
class CQLExpressionRep;
class QueryContext;

/**
    The enum is private, but the definition is public.
*/
enum TermOpType {TERM_ADD,TERM_SUBTRACT};

/*
#ifndef PEGASUS_ARRAY_T
#define PEGASUS_ARRAY_T TermOpType
#include <Pegasus/Common/ArrayInter.h>
#undef PEGASUS_ARRAY_T
#endif
*/

/**
    The CQLExpression class encapsulates a generic CQL expression to make it
    easier to break into components and process the expression.
*/

class PEGASUS_CQL_LINKAGE CQLExpression
{
 public:

  /**
      Contructs CQLExpression default object.

      @param  - None.
      @return - None.
      @throw  - None.
      <I><B>Experimental Interface</B></I><BR>
  */
  CQLExpression();

  /**
      Contructs CQLExpression with a CQLTerm.

      @param  - theTerm - A CQLTerm used to create CQLExpression.
      @return - None.
      @throw  - None.
      <I><B>Experimental Interface</B></I><BR>
  */
  CQLExpression(const CQLTerm& theTerm);

  /**
      Contructs CQLExpression with a CQLExpression object. (copy-constructor)

      @param  - inExpress - Object to be copied.
      @return - None.
      @throw  - None.
      <I><B>Experimental Interface</B></I><BR>
  */
  CQLExpression(const CQLExpression& inExpress);

  /**
      Destructs the CQLExpression object.

      @param  - None.
      @return - None.
      @throw  - None.
      <I><B>Experimental Interface</B></I><BR>
  */
  ~CQLExpression();

  /** the resolveValue method evaluates the expression and returns the value.
      Any property that need to be resolved into a value is taken from the
      CIMInstance.

      @param  - CI - the CIMInstance to be evaluated.
      @param  - QueryCtx - The QueryContext used to access the repository.
      @return - The resolved CQLValue.
      @throw  - CQLSyntaxErrorException.
      <I><B>Experimental Interface</B></I><BR>
  */
  CQLValue resolveValue(const CIMInstance& CI, const QueryContext& QueryCtx);

  /** The appendOperation is used by Bison.
      It is invoked 0 or more times for the CQLExpression, and
      when invoked will always pass in an integer that is the Term operation
      type and a CQLTerm object.

      @param  - theTermOpType - the type of operation being appended.
      @param  - theTerm - the term to be operated on.
      @return - None.
      @throw  - None.
      <I><B>Experimental Interface</B></I><BR>
  */
  void appendOperation(const TermOpType theTermOpType, const CQLTerm& theTerm);

  /**
      Returns a string representation of the object.

      @param  - None.
      @return - A string
      @throw  - None.
      <I><B>Experimental Interface</B></I><BR>
  */
  String toString()const;

  /**
      Will return true if the object has only one CQLTerm object within it.

      @param  - None.
      @return - true or false
      @throw  - None.
      <I><B>Experimental Interface</B></I><BR>
  */
  Boolean isSimple()const;

  /**
      Will return true if the CQLExpression resolves to a simple value.

      @param  - None.
      @return - None.
      @throw  - None.
      <I><B>Experimental Interface</B></I><BR>
  */
  Boolean isSimpleValue()const;

  /**
      Retrieve an array of all the CQLTerms for this expression.

      @param  - None.
      @return - An array of CQLTerm.
      @throw  - None.
      <I><B>Experimental Interface</B></I><BR>
  */
  Array<CQLTerm> getTerms()const;

  /**
      Retrieve an array of all the TermOpType for this expression.

      @param  - None.
      @return - None.
      @throw  - None.
      <I><B>Experimental Interface</B></I><BR>
  */
  Array<TermOpType> getOperators()const;

  /**
      Calling applyContext function for every internal object.  This
      will fully qualify the Chained Identifiers within all the CQLValueobjects.

      @param  - inContext - Query Context used to access the repository.
      @param  - inCid - Chained Identifier used for standalone symbolic
                        constants.
      @return - None.
      @throw  - None.
      <I><B>Experimental Interface</B></I><BR>
  */
  void applyContext(const QueryContext& inContext,
       const CQLChainedIdentifier& inCid = CQLChainedIdentifier());

  /**
      Assignment operation.

      @param  - rhs - CQLExpression to be assigned.
      @return - Assigned value.
      @throw  - None.
      <I><B>Experimental Interface</B></I><BR>
  */
  CQLExpression& operator=(const CQLExpression& rhs);

  /**
      Compare to CQLExpressions for equality

      @param  - expr - rightside value of operation
      @return - true or false.
      @throw  - None.
      <I><B>Experimental Interface</B></I><BR>
  */
  /*
  Boolean operator==(const CQLExpression& expr)const;
   */
  /**
      Compare to CQLExpressions for non-equality

      @param  - expr - rightside value of operation
      @return - true or false.
      @throw  - None.
      <I><B>Experimental Interface</B></I><BR>
  */
  /*
  Boolean operator!=(const CQLExpression& expr)const;
  */
  friend class CQLFactory;

 private:

  CQLExpressionRep *_rep;
};

/*
#ifndef PEGASUS_ARRAY_T
#define PEGASUS_ARRAY_T CQLExpression
#include <Pegasus/Common/ArrayInter.h>
#undef PEGASUS_ARRAY_T
#endif
*/

PEGASUS_NAMESPACE_END

#endif
#endif
