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

#ifndef CQLFACTOR_H_HEADER
#define CQLFACTOR_H_HEADER

#ifdef PEGASUS_USE_EXPERIMENTAL_INTERFACES

#include <Pegasus/CQL/Linkage.h>
#include <Pegasus/Common/CIMInstance.h>
#include <Pegasus/CQL/CQLChainedIdentifier.h>
#include <Pegasus/Common/ArrayInternal.h>

PEGASUS_NAMESPACE_BEGIN
class CQLFactory;
class CQLExpression;
class CQLFunction;
class CQLFactorRep;
class CQLValue;
class QueryContext;

/**  The CQLFactor could contain a CQLValue, CQLExpression or CQLFunction.
     This class evaluates the object to get a CQLValue.
     If it contains a CQLValue, it will resolve the CQLValue.
*/

class PEGASUS_CQL_LINKAGE CQLFactor
{
 public:

  /**
      Contructs CQLFactor default object.

      @param  - None.
      @return - None.
      @throw  - None.
      <I><B>Experimental Interface</B></I><BR>
  */
  CQLFactor();

  /**
      Contructs CQLFactor from a CQLValue.

      @param  - inCQLVal - The CQLValue used to construct the CQLFactor.
      @return - None.
      @throw  - None.
      <I><B>Experimental Interface</B></I><BR>
  */
  CQLFactor(const CQLValue& inCQLVal);

  /**
      Contructs CQLFactor from a CQLExpression object.

      @param  - inCQLExp - The CQLExpression used to construct the CQLFactor.
      @return - None.
      @throw  - None.
      <I><B>Experimental Interface</B></I><BR>
  */
  CQLFactor(const CQLExpression& inCQLExp);

  /**
      Contructs CQLFactor from a CQLFunction object.

      @param  - inCQLFunc - The CQLFunction used to construct the CQLFactor.
      @return - None.
      @throw  - None.
      <I><B>Experimental Interface</B></I><BR>
  */
  CQLFactor(const CQLFunction& inCQLFunc);

  /**
      Contructs CQLFactor from a CQLFactor object (copy-constructor).

      @param  - inCQLFact - The CQLFactor used to construct the CQLFactor.
      @return - None.
      @throw  - None.
      <I><B>Experimental Interface</B></I><BR>
  */
  CQLFactor(const CQLFactor& inCQLFact);

  /**
      Destroyes the CQLFactor object.

      @param  - None.
      @return - None.
      @throw  - None.
      <I><B>Experimental Interface</B></I><BR>
  */
  ~CQLFactor();

  /**
      The CQLFactor could contain a CQLValue, CQLExpression or CQLFunction.
      This method evaluates the object to get a CQLValue.
      If it contains a CQLValue, it will resolve the CQLValue by calling the
      resolve method
      on the CQLValue object.

      If it contains a CQLFunction this method will invoke the getValue method
      on
      CQLFunction, and the returned value will already be resolved.

      If it contains a CQLExpression this method will invoke the getValue
      method on
      CQLExpression, and the returned value will already be resolved.

      @param  - CI - The CIMInstance to be evaluated.
      @param  - QueryCtx - The QueryContext contains specific information
                           about the query.
      @return - The primitive CQLValue.
      @throw  - CQLSyntaxErrorException.
      <I><B>Experimental Interface</B></I><BR>
  */
  CQLValue resolveValue(const CIMInstance& CI, const QueryContext& QueryCtx);

  /**
      Will return true if the object has only one CQLValue object within it..

      @param  - None.
      @return - true or false.
      @throw  - None.
      <I><B>Experimental Interface</B></I><BR>
  */
  Boolean isSimple()const;

  /**
      Will return true if the object has only one CQLValue object within it.

      @param  - None.
      @return - true or false.
      @throw  - None.
      <I><B>Experimental Interface</B></I><BR>
  */
  Boolean isSimpleValue()const;

  /**
      Retrieves the CQLValue from the CQLFactor object.
      NOTE: If CQLValue has not been set for this object, a
      default object will be returned.

      @param  - None.
      @return - The CQLValue.
      @throw  - None.
      <I><B>Experimental Interface</B></I><BR>
  */
  CQLValue getValue()const;

  /**
      Retrieves the CQLFunciton from the CQLFactor object.
      NOTE: If CQLFunction has not been set for this object, a
      default object will be returned.

      @param  - None.
      @return - The CQLFucntion
      @throw  - None.
      <I><B>Experimental Interface</B></I><BR>
  */
  CQLFunction getCQLFunction()const;

  /**
      Retrieves the CQLExpression from the CQLFactor object.
      NOTE: If CQLExpression has not been set for this object, a
      default object will be returned.

      @param  - None.
      @return - The CQLFucntion
      @throw  - None.
      <I><B>Experimental Interface</B></I><BR>
  */
  CQLExpression getCQLExpression()const;

  /**
      Returns a string representation of the object.

      @param  - None.
      @return - A string
      @throw  - None.
      <I><B>Experimental Interface</B></I><BR>
  */
  String toString()const;

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
      Assignment operation

      @param  - rhs - CQLFactor to be assigned
      @return - the assigned value.
      @throw  - None.
      <I><B>Experimental Interface</B></I><BR>
  */
  CQLFactor& operator=(const CQLFactor& rhs);

  /**
      Compare to CQLFactors for equality

      @param  - expr - rightside value of operation
      @return - true or false.
      @throw  - None.
      <I><B>Experimental Interface</B></I><BR>
  */
  /*
  Boolean operator==(const CQLFactor& factor)const;
  */
  /**
      Compare to CQLFactors for non-equality

      @param  - expr - rightside value of operation
      @return - true or false.
      @throw  - None.
      <I><B>Experimental Interface</B></I><BR>
  */
  /*
  Boolean operator!=(const CQLFactor& factor)const;
  */
  friend class CQLFactory;
 private:

  CQLFactorRep *_rep;
};

/*
#ifndef PEGASUS_ARRAY_T
#define PEGASUS_ARRAY_T CQLFactor
#include <Pegasus/Common/ArrayInter.h>
#undef PEGASUS_ARRAY_T
#endif
*/

PEGASUS_NAMESPACE_END
#endif
#endif /* CQLFACTOR_H_HEADER */
