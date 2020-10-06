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
// Modified By: David Dillard, VERITAS Software Corp.
//                  (david.dillard@veritas.com)
//              Vijay Eli, IBM (vijayeli@in.ibm.com) bug#3590
//
//%/////////////////////////////////////////////////////////////////////////////

#ifndef Pegasus_CQLFactorRep_h
#define Pegasus_CQLFactorRep_h

#include <Pegasus/CQL/Linkage.h>
#include <Pegasus/Common/CIMInstance.h>
#include <Pegasus/CQL/CQLExpression.h>
#include <Pegasus/CQL/CQLFunction.h>

PEGASUS_NAMESPACE_BEGIN
class CQLFactory;
class CQLValue;
class QueryContext;

class CQLFactorRep
{
 public:

  CQLFactorRep();

  CQLFactorRep(const CQLValue& inCQLVal);

  CQLFactorRep(const CQLExpression& inCQLExp);

  CQLFactorRep(const CQLFunction& inCQLFunc);

  CQLFactorRep(const CQLFactorRep* rep);

  ~CQLFactorRep(){}

  CQLValue resolveValue(const CIMInstance& CI, const QueryContext& QueryCtx);

  Boolean isSimple()const;

  Boolean isSimpleValue()const;

  CQLValue getValue()const;

  CQLFunction getCQLFunction()const;

  CQLExpression getCQLExpression()const;

  String toString()const;

  void applyContext(const QueryContext& inContext,
            const CQLChainedIdentifier& inCid);
/*
  Boolean operator==(const CQLFactorRep& rep)const;

  Boolean operator!=(const CQLFactorRep& rep)const;
  */
  friend class CQLFactory;

 private:

  enum ContainedObjectType {EXPRESSION, FUNCTION, VALUE};

  CQLExpression _CQLExp;

  CQLValue _CQLVal;

  CQLFunction _CQLFunct;

  Boolean _invert;

  Boolean _simpleValue;

  ContainedObjectType _containedType;
};

PEGASUS_NAMESPACE_END

#endif /* CQLFACTORREP_H_HEADER */
