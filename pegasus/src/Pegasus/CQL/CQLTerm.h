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

#ifndef Pegasus_CQLTerm_h
#define Pegasus_CQLTerm_h

#ifdef PEGASUS_USE_EXPERIMENTAL_INTERFACES

#include <Pegasus/CQL/Linkage.h>
#include <Pegasus/CQL/CQLValue.h>
#include <Pegasus/Common/ArrayInternal.h>
#include <Pegasus/CQL/CQLFactor.h>

PEGASUS_NAMESPACE_BEGIN

class CQLFactory;
class CQLTermRep;
class QueryContext;

/**
    enum of multiply, divide and string concatenation operators.
*/
enum FactorOpType { /*mult, divide,*/ concat };

/*
#ifndef PEGASUS_ARRAY_T
#define PEGASUS_ARRAY_T FactorOpType
#include <Pegasus/Common/ArrayInter.h>
#undef PEGASUS_ARRAY_T
#endif
*/

/**
    The CQLTerm class encapsulates a generic CQL term to make it
    easier to break into pieces (factors) and process the term.
    A CQL term is made up of factors and operators.
    There must be exactly one more factor than there are operators.
*/

class PEGASUS_CQL_LINKAGE CQLTerm
{
 public:

  /**
      Constructs CQLTermRep default object.

      @param  - None.
      @return - None.
      @throw  - None.
      <I><B>Experimental Interface</B></I><BR>
  */
  CQLTerm();

  /**
      Constructs CQLTermRep from a CQLFactor object.

      @param  - theFactor is a CQLFactor object.
      @return - None.
      @throw  - None.
      <I><B>Experimental Interface</B></I><BR>
  */
  CQLTerm(const CQLFactor& theFactor);

  /**
      Constructs CQLTermRep from a CQLTermRep object. (copy-constructor)

      @param  - inTerm is a CQLTermRep object.
      @return - None.
      @throw  - None.
      <I><B>Experimental Interface</B></I><BR>
  */
  CQLTerm(const CQLTerm& inTerm);

  /**
      Destroys CQLTermRep object.

      @param  - None.
      @return - None.
      @throw  - None.
      <I><B>Experimental Interface</B></I><BR>
  */
  ~CQLTerm();

  /** the resolveValue method evaluates the CQL Term and returns the value.
      Any property that need to be resolved into a value is taken from the
      CIMInstance.

      @param  - CI - The CIMInstance used for the evaluate.
      @param  - QueryCtx - Reference to the Querycontext object.
      @return - CQLValue - The value of the object being resolved.
      @throw  - CQLRuntimeException
      <I><B>Experimental Interface</B></I><BR>
  */
  CQLValue resolveValue(const CIMInstance& CI, const QueryContext& QueryCtx);

  /** The function is used by the parser.
      It is invoked 0 or more times for the CQLTerm, and
      when invoked will always pass in an integer that is the Factor operation
      type and a CQLFactor object.

      @param  - inFactorOpType - next operation to be performed.
      @param  - inFactor - next CQLFactor to be operated on.
      @return - None.
      @throw  - None.
      <I><B>Experimental Interface</B></I><BR>
  */
  void appendOperation(FactorOpType inFactorOpType, const CQLFactor& inFactor);

  /**
      Converts the contents of the object to a String.

      @param  - None.
      @return - A string version of the object.
      @throw  - None.
      <I><B>Experimental Interface</B></I><BR>
  */
  String toString()const;

  /**
      Returns true if the object is simple.  Simple is defined as only having
      a one CQLFactor object. The CQLFactor object could be complex.

      @param  - None.
      @return - true or false
      @throw  - None.
      <I><B>Experimental Interface</B></I><BR>
  */
  Boolean isSimple()const;

  /**
      Returns true if the value within the object is Simple.  In this case
      the object only has one CQLFactor object.  This CQLFactor object in
      not a complex object.

      @param  - None.
      @return - true or false
      @throw  - None.
      <I><B>Experimental Interface</B></I><BR>
  */
  Boolean isSimpleValue()const;

  /**
      Will return an array of CQLFactors.

      @param  - None.
      @return - An array of CQLFactor
      @throw  - None.
      <I><B>Experimental Interface</B></I><BR>
  */
  Array<CQLFactor> getFactors()const;

  /**
      Will return an array of Operations.

      @param  - None.
      @return - an array of FactorOpType
      @throw  - None.
      <I><B>Experimental Interface</B></I><BR>
  */
  Array<FactorOpType> getOperators()const;

  /**
      Calling applyContext function for every internal object.  This
      will fully qualify the Chained Identifiers within all the CQLValue
      objects.

      @param  - inContext - Query Context used to access the repository.
      @param  - inCid - Chained Identifier used for standalone symbolic
      constants.
      @return - None.
      @throw  - None.
      <I><B>Experimental Interface</B></I><BR>
  */
  void applyContext(const QueryContext& inContext,
            const CQLChainedIdentifier& inCid);

  /**
      Assignment operator.

      @param  - rhs - a CQLTerm to be assined.
      @return - a reference of a CQLTerm.
      @throw  - None.
      <I><B>Experimental Interface</B></I><BR>
  */
  CQLTerm& operator=(const CQLTerm& rhs);

  /**
      Compare to CQLTerms for equality

      @param  - rhs - rightside value of operation
      @return - true or false.
      @throw  - None.
      <I><B>Experimental Interface</B></I><BR>
  */
  /*
  Boolean operator==(const CQLTerm& rhs)const;
  */
  /**
      Compare to CQLTerms for non-equality

      @param  - rhs - rightside value of operation
      @return - true or false.
      @throw  - None.
      <I><B>Experimental Interface</B></I><BR>
  */
  /*
  Boolean operator!=(const CQLTerm& rhs)const;
  */

  friend class CQLFactory;

 private:

  CQLTermRep *_rep;
};

/*
#ifndef PEGASUS_ARRAY_T
#define PEGASUS_ARRAY_T CQLTerm
#include <Pegasus/Common/ArrayInter.h>
#undef PEGASUS_ARRAY_T
#endif
*/

PEGASUS_NAMESPACE_END
#endif
#endif
