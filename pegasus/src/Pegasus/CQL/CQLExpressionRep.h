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
// Authors: David Rosckes (rosckes@us.ibm.com)
//          Bert Rivero (hurivero@us.ibm.com)
//          Chuck Carmack (carmack@us.ibm.com)
//          Brian Lucier (lucier@us.ibm.com)
//
// Modified By: David Dillard, VERITAS Software Corp.
//                  (david.dillard@veritas.com)
//              Vijay Eli, IBM (vijayeli@in.ibm.com) bug#3590
//
//%/////////////////////////////////////////////////////////////////////////////

#ifndef Pegasus_CQLExpressionRep_h
#define Pegasus_CQLExpressionRep_h

#include <Pegasus/CQL/Linkage.h>
#include <Pegasus/CQL/CQLValue.h>
#include <Pegasus/CQL/CQLTerm.h>

PEGASUS_NAMESPACE_BEGIN

class CQLFactory;
class QueryContext;

class CQLExpressionRep
{
  public:
  CQLExpressionRep(){}

  CQLExpressionRep(const CQLTerm& theTerm);
  CQLExpressionRep(const CQLExpressionRep* rep);

  ~CQLExpressionRep();

  CQLValue resolveValue(const CIMInstance& CI, const QueryContext& QueryCtx);

  void appendOperation(const TermOpType theTermOpType, const CQLTerm& theTerm);

  String toString()const;

  Boolean isSimple()const;

  Boolean isSimpleValue()const;

  Array<CQLTerm> getTerms()const;

  Array<TermOpType> getOperators()const;

  void applyContext(const QueryContext& inContext,
            const CQLChainedIdentifier& inCid);
/*
  Boolean operator==(const CQLExpressionRep& rep)const;

  Boolean operator!=(const CQLExpressionRep& rep)const;
  */
  friend class CQLFactory;

 private:

  /**  The _TermOperators member variable is an
       array of operators that are valid to operate on Terms in a CQL
       expression.
       Valid operators include concatentation, add and subtract.

       The array is ordered according to the operation from left to right.
  */
  Array<TermOpType> _TermOperators;

  /**  The _CQLTerms member variable is an
       array of operands that are valid in a CQL expression.

       The array is ordered according to the operation from left to right.
  */
  Array<CQLTerm> _CQLTerms;

};

PEGASUS_NAMESPACE_END


#endif /* CQLEXPRESSION_H_HEADER_INCLUDED_BEE5929F */
